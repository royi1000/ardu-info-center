#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <Wire.h>
#include <SPI.h>
#include <EEPROM.h>
#include <DS1307RTC.h>
#include <Time.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "led.h"
#include "rf_io.h"
/*********   PINS     *******
 *******    RF24    *********
 CE-9
 CSN-10
 MOSI-11
 MISO-12
 SCK-13
 ******   Ethernet  ********
SCK 	Pin 13
SO 	Pin 12
SI 	Pin 11
CS 	Pin 8
***************************/

#define PAYLOAD_SIZE 20
#define DEVICE_TYPE 0x10
#define MAX_STR_LEN (12*6)
#define MAX_SCREENS 5
#define MAX_SENSOR_NAME 5
#define MAX_SENSORS 10
#define HISTORY_SIZE 24

#define SCREEN_TIME 4000 //time to view screen in milli
#define UPDATE_INTERVAL 15000
#define WD_TIMEOUT_INTERVAL (10 * 60 * 1000)
const uint16_t MAGIC_CODE = 0xDE12;
// Set up nRF24L01 radio on SPI bus plus pins 9 & 10
typedef unsigned long time_t;

RF24 radio(9,10);

EthernetUDP Udp; // New from IDE 1.0
const int NTP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets 
//DS1307RTC RTC;

// Radio pipe addresses for the 2 nodes to communicate.
uint64_t pipes[2] = {
  0xF0F0F0F0D2LL,  0xF0F0F0F0E1LL};

typedef enum{
  command_init=0xF0, command_init_response=0xF1, sensor_data=0x30} 
command_type_e;
typedef enum{
  date=0x1, string=0x2, bitmap=0x3, color_string=0x4, sound=0x5, sensor=0x6, remove_id=0x10, end_tx=0x20} 
data_type_e;
typedef enum{
  c_red=1, c_green=2, c_blue=3, c_purple=4, c_yellow=5, c_aqua=6} 
color_type_e;

typedef enum{
    temp=1, rh=2, baro=3,soil_rh=4, light=5
} sensor_type_e;

uint8_t sensor_digits[] = {0,0xf2,0xf2,0xf4,4,5};

typedef struct sensor_data_s {
	uint8_t command;
	uint8_t type;
	uint8_t name[MAX_SENSOR_NAME];
	uint8_t hist[HISTORY_SIZE];
	float min;
	float max;
	float val;
} sensor_data_t;

uint8_t radio_buf[32];
uint8_t data[100];
sensor_data_t sensors[MAX_SENSORS];

uint8_t sound_data[100];
unsigned int data_len = 0;
unsigned int sound_len = 0;
unsigned long last_packet_recived = 0;
// byte timeServer[] = {192, 43, 244, 18}; // time.nist.gov NTP server
//byte timeServer[] = {178, 79, 160, 57};    // 0.uk.pool.ntp.org
IPAddress ip(178, 79, 160, 57);

typedef struct cmd_message {
  uint8_t  command;
  uint8_t dev_type;
  uint64_t addr;
} cmd_message_t;

typedef struct sensor_msg {
  uint8_t   command;
  uint8_t   sensor_type;
  uint64_t  addr;
  float     val;
} 
sensor_message_t;

typedef struct date_cmd {
  uint8_t  command;
  time_t   time;
} 
date_cmd_t;

bool is_timeouted(unsigned long last, unsigned long timeout_interval /*millis*/)
{
  if (((millis() - last) < timeout_interval))
    return false;
  return true;
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:         
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer,NTP_PACKET_SIZE);
  Udp.endPacket();
}

void get_udp_ntp_packet()
{
#if ARDUINO >= 100
    Udp.read(packetBuffer, NTP_PACKET_SIZE);      // New from IDE 1.0,
#else
    Udp.readPacket(packetBuffer, NTP_PACKET_SIZE);
#endif 

    // NTP contains four timestamps with an integer part and a fraction part
    // we only use the integer part here
    unsigned long t1, t2, t3, t4;
    t1 = t2 = t3 = t4 = 0;
    for (int i=0; i< 4; i++)
    {
      t1 = t1 << 8 | packetBuffer[16+i];      
      t2 = t2 << 8 | packetBuffer[24+i];      
      t3 = t3 << 8 | packetBuffer[32+i];      
      t4 = t4 << 8 | packetBuffer[40+i];
    }

    // part of the fractional part
    // could be 4 bytes but this is more precise than the 1307 RTC
    // which has a precision of ONE second
    // in fact one byte is sufficient for 1307
    float f1,f2,f3,f4;
    f1 = ((long)packetBuffer[20] * 256 + packetBuffer[21]) / 65536.0;      
    f2 = ((long)packetBuffer[28] * 256 + packetBuffer[29]) / 65536.0;      
    f3 = ((long)packetBuffer[36] * 256 + packetBuffer[37]) / 65536.0;      
    f4 = ((long)packetBuffer[44] * 256 + packetBuffer[45]) / 65536.0;

    // NOTE:
    // one could use the fractional part to set the RTC more precise
    // 1) at the right (calculated) moment to the NEXT second!
    //    t4++;
    //    delay(1000 - f4*1000);
    //    RTC.adjust(DateTime(t4));
    //    keep in mind that the time in the packet was the time at
    //    the NTP server at sending time so one should take into account
    //    the network latency (try ping!) and the processing of the data
    //    ==> delay (850 - f4*1000);
    // 2) simply use it to round up the second
    //    f > 0.5 => add 1 to the second before adjusting the RTC
    //   (or lower threshold eg 0.4 if one keeps network latency etc in mind)
    // 3) a SW RTC might be more precise, => ardomic clock :)


    // convert NTP to UNIX time, differs seventy years = 2208988800 seconds
    // NTP starts Jan 1, 1900
    // Unix time starts on Jan 1 1970.
    const unsigned long seventyYears = 2208988800UL;
    t1 -= seventyYears;
    t2 -= seventyYears;
    t3 -= seventyYears;
    t4 -= seventyYears;

    // Adjust timezone and DST... in my case substract 4 hours for Chile Time
    // or work in UTC?
    t4 += (3 * 3600L);     // Notice the L for long calculations!!
    t4 += 1;               // adjust the delay(1000) at begin of loop!
    if (f4 > 0.4) t4++;    // adjust fractional part, see above
    RTC.set(t4);
}

void update_ntp_time() {
  sendNTPpacket(ip);
  delay(1000);
  if ( Udp.available() ) {
    get_udp_ntp_packet();
  }
}

void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.println(); 
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}


void send_data_to_client(struct cmd_message* msg)
{
    radio.stopListening();
    radio.openWritingPipe(msg->addr);
    msg->command = command_init_response;
    handle_write(&radio, (void *)&msg, sizeof(msg), 5);
    date_cmd_t cmd;
    cmd.command = date;
    cmd.time = RTC.get();
    handle_write(&radio, (void *)&cmd, sizeof(cmd), 5);
    for(int i=0;i<MAX_SENSORS;i++) {
      if(sensors[i].type) {
	uint8_t msg_buf[sizeof(sensor_data_t)+2];
	msg_buf[0] = sensor;
	msg_buf[1] = i;
	memcpy(&msg_buf[2], &sensors[i], sizeof(sensor_data_t));
	handle_write(&radio, (void *)&sensors[i], sizeof(msg_buf), 5);
      }
    }
}

bool handle_message() {
  bool ret = false;
  uint8_t cmd = data[0];
  if(cmd==command_init) {
    Serial.println("got init command");
    // need delay?
    cmd_message_t* msg = (cmd_message_t*)data;
    send_data_to_client(msg);
  }
  return ret;
}

void setup()
{
  Serial.begin(9600);
  Serial.println("start radio");
  radio.begin();
  Wire.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);

  //        radio.enableDynamicPayloads();
  radio.setPayloadSize(PAYLOAD_SIZE);

  radio.openWritingPipe( 0xF0F0000000LL );
  radio.openReadingPipe(1, 0xF0F0F0F0E1LL);

  radio.startListening();
  radio.printDetails();
  
  setTime(RTC.get());
  digitalClockDisplay();
}

void get_data_from_clients()
{
  unsigned long start_wait_time = millis();
  while (millis() - start_wait_time < UPDATE_INTERVAL) {
    if(radio.available()) {
      uint8_t payload_size = radio.getPayloadSize();
      radio.read(radio_buf, payload_size);
      if(handle_read(&radio, radio_buf,  payload_size, data, &data_len)) {
	handle_message();
      }
    }
  }
}

void get_data_from_net()
{
  update_ntp_time();
}
void loop()
{
  get_data_from_clients();
  get_data_from_net();
}

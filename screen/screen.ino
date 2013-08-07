#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <SPI.h>
#include <EEPROM.h>
#include <Time.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "led.h"
#include "lcd.h"
#include "sound.h"
#include "DHT.h"
#include "rf_io.h"

/*********   PINS     *******
 *******    RF24    *********
 CE-9
 CSN-10
 MOSI-11
 MISO-12
 SCK-13
 ***** SCREEN   ********
 Pin 2 (SCLK)	 Arduino digital pin 3
Pin 3 (SDIN/MOSI)	 Arduino digital pin 4
Pin 4 (D/C)	 Arduino digital pin 5
Pin 5 (SCE)	 Arduino digital pin 7
Pin 8 (RST)	 Arduino digital pin 6
***************************/

#define DHT22_PIN 2
#define PAYLOAD_SIZE 20
#define SENSOR_RH_TEMP 0x50
#define DEVICE_TYPE 0x10
#define MAX_STR_LEN (12*6)
#define MAX_SCREENS 5
#define MAX_SENSOR_NAME 5
#define MAX_SENSORS 10
#define HISTORY_SIZE 24

#define SCREEN_TIME 4000 //time to view screen in milli
#define UPDATE_INTERVAL 5000
#define WD_TIMEOUT_INTERVAL (1 * 60 * 1000)
const uint16_t MAGIC_CODE = 0xDE12;
// Set up nRF24L01 radio on SPI bus plus pins 9 & 10

RF24 radio(9,10);
DHT dht;


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

typedef struct led_s{
  uint8_t red1;
  uint8_t green1;
  uint8_t blue1;
  uint8_t red2;
  uint8_t green2;
  uint8_t blue2;
}
led_t;

typedef struct sensor_data_s {
	uint8_t type;
	uint8_t name[MAX_SENSOR_NAME];
	uint8_t hist[HISTORY_SIZE];
	float min;
	float max;
	float val;
} sensor_data_t;

uint8_t radio_buf[32];
uint8_t data[100];
uint8_t screens[MAX_SCREENS][MAX_STR_LEN+1];
uint8_t screens_size[MAX_SCREENS];
led_t screens_color[MAX_SCREENS];
sensor_data_t sensors[MAX_SENSORS];

uint8_t sound_data[100];
unsigned int data_len = 0;
unsigned int last_screen_id = 0;
unsigned long last_screen_time = 0;
unsigned long last_update_time = 0;
unsigned int sound_len = 0;
uint8_t current_color = c_blue;
unsigned long last_packet_recived = 0;
typedef struct store {
  uint16_t magic_code;
  uint64_t rx_addr;
  uint64_t tx_addr;
} 
store_t;

typedef struct cmd_message {
  uint8_t  command;
  uint8_t dev_type;
  uint64_t addr;
} 
cmd_message_t;

typedef struct temp_rh_sensor {
  uint8_t    command;
  uint8_t   sensor_type;
  uint64_t addr;
  float       rh;
  float       temp;
} 
temp_rh_sensor_message_t;

typedef struct date_cmd {
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
} 
date_cmd_t;
store_t config_settings;

void color_out(){
  fade(screens_color[last_screen_id].red1,
  screens_color[last_screen_id].green1,
  screens_color[last_screen_id].blue1,
  screens_color[last_screen_id].red2,
  screens_color[last_screen_id].green2,
  screens_color[last_screen_id].blue2,
  5);
  fade(screens_color[last_screen_id].red2,
  screens_color[last_screen_id].green2,
  screens_color[last_screen_id].blue2,
  screens_color[last_screen_id].red1,
  screens_color[last_screen_id].green1,
  screens_color[last_screen_id].blue1,
  5);
}

void digitalClockDisplay(){
  LcdClear();
  drawLine();
  gotoXY(14,1);
  LcdString(dayStr(weekday()));
  gotoXY(22,3);
  printDigits(hour(), 2, ':');
  printDigits(minute(),2,0);
  gotoXY(7,4);
  printDigits(day(),2,'/');
  printDigits(month(),2,'/');
  printDigits(year(),4,0);
  if(now() - last_packet_recived > 600) {
    gotoXY(4,1);
    LcdCharacter('~');
  }
}

bool is_timeouted(unsigned long last, unsigned long timeout_interval /*millis*/)
{
  if (((millis() - last) < timeout_interval))
    return false;
  return true;
}

bool handle_message() {
  bool ret = false;
  uint8_t cmd = data[0];
  if(cmd==command_init_response) {
    cmd_message_t* msg = (cmd_message_t*)data;
    if(config_settings.magic_code != MAGIC_CODE || msg->addr != config_settings.rx_addr) {
      red(100);
      green(100);
      blue(100);
      config_settings.rx_addr = msg->addr;
      config_settings.magic_code = MAGIC_CODE;
      eeprom_write_block((const void*)&config_settings, (void*)0, sizeof(config_settings));
    }
    last_screen_time = 0;
  }
  else if (cmd==date) {
    date_cmd_t* dct = (date_cmd_t*)(data+1) ;
    setTime(dct->hour,dct->minute,dct->second,dct->day,dct->month,dct->year);
    last_packet_recived = now();
  }
  else if (cmd==string) {
    uint8_t id = data[1];
    if ((id>10) || ((data_len - 2) > MAX_STR_LEN)) {
      //support only 10 id's
      data_len = 0;
      return false;
    }
    memcpy(screens[id-1], data+2, data_len-2);
    screens_size[id-1] = data_len - 2;
    screens[id-1][data_len-2] = 0; //verify null termination
    memset(&screens_color[id-1], 0, sizeof(led_t)) ;
  }
  else if (cmd==sensor) {
    uint8_t id = data[1];
    if ((id>MAX_SENSORS)||data_len-2 != sizeof(sensor_data_t)) {
      Serial.print("invalid sensor packet id:");
	  Serial.print(id);
	  Serial.print(" len:");
	  Serial.print(data_len-2);
	  Serial.print(" data size:");
	  Serial.println(sizeof(sensor_data_t));
      return false;
    }
    memcpy(&sensors[id], data+2, data_len-2);
  }
  else if (cmd==sound) {
    memcpy(sound_data, data+1, data_len-1);
    sound_len = data_len - 1;
  }
  else if (cmd==color_string) {
    uint8_t id = data[1];
    if ((id>10) || ((data_len - 3) > MAX_STR_LEN)) {
      //support only 10 id's
      data_len = 0;
      return false;
    }
    memcpy(screens[id-1], data+2+sizeof(led_t), data_len-2-sizeof(led_t));
    screens_size[id-1] = data_len - 3;
    memcpy(&screens_color[id-1], data+2, sizeof(led_t));
    screens[id-1][data_len-2-sizeof(led_t)] = 0; //verify null termination
  }
  else if (cmd==remove_id) {
    uint8_t id = data[1];
    if(id>0 && id <= 10){
      screens_size[id-1] = 0;
    }
  }
  else if (cmd==end_tx) {
    green(100);
    ret = true;
  }
  data_len = 0;
  return ret;
}

void send_sensor_data() {
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();

  Serial.print(dht.getStatusString());
  Serial.print("\t");
  Serial.print(humidity, 1);
  Serial.print("\t\t");
  Serial.println(temperature, 1);

  temp_rh_sensor_message_t sensor_message;
  sensor_message.command = sensor_data;
  sensor_message.sensor_type = SENSOR_RH_TEMP;
  sensor_message.addr = config_settings.rx_addr;
  sensor_message.rh = humidity;
  sensor_message.temp = temperature;
  radio.stopListening();
  handle_write(&radio, (void *)&sensor_message, sizeof(sensor_message), 5);
  radio.startListening();
}

void send_init_packet() {
  cmd_message_t message;
  message.command = command_init;
  message.dev_type = DEVICE_TYPE;
  message.addr = config_settings.rx_addr;
  radio.stopListening();
  handle_write(&radio, (void *)&message, sizeof(message), 5);
  radio.startListening();
}

void setup()
{
  Serial.begin(9600);
  Serial.println("start setup");

  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(SOUND_PIN, OUTPUT);
  digitalWrite(A0, HIGH);
  digitalWrite(A2, HIGH);
  digitalWrite(A1, HIGH);
  LcdInitialise();
  LcdClear();
  dht.setup(DHT22_PIN  ); // data pin 2
  //printf_begin();
  //printf("\n\rNokia screen receiver\n\r");
  randomSeed(analogRead(0));
  eeprom_read_block((void*) &config_settings, (void*) 0, sizeof(config_settings));
  // Setup and configure rf radio
  if (0xDE12 != config_settings.magic_code) {
    config_settings.tx_addr = 0xF0F0F0F0E1LL;//0xF0F0F00000LL | random(0xFFFFLL);
    config_settings.rx_addr = 0xF0F0000000LL | random(0xFFFFFFLL);
  }
  Serial.println("start radio");
  radio.begin();


  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);

  //        radio.enableDynamicPayloads();
  radio.setPayloadSize(PAYLOAD_SIZE);

  radio.openWritingPipe(config_settings.tx_addr);
  radio.openReadingPipe(1, config_settings.rx_addr);

  radio.startListening();
  radio.printDetails();
  last_packet_recived = now();
}

void next_screen()
{
  Serial.println("time");
    LcdClear();
    digitalClockDisplay();
	delay(SCREEN_TIME);
	for(last_screen_id=0; last_screen_id< MAX_SCREENS;last_screen_id++) {
		if(screens_size[last_screen_id]) {
            Serial.println("screen");
			color_out();
			LcdClear();
			LcdString((char*)screens[last_screen_id]);
			delay(SCREEN_TIME);
		}
	}
}

void get_data_from_master()
{
  if(!is_timeouted(last_update_time, UPDATE_INTERVAL))
  {
    return;
  }
  //yellow(100);
  bool end_transmission = false;
  int i=0;
  while (!end_transmission && i++<3){
	  delay(100);
	  send_init_packet();
	  unsigned long start_wait_time = millis();
	  bool timeout = false;
	  while(!timeout && !end_transmission) {
		if ((millis() - start_wait_time > 3000)) {
		  red(10);
		  timeout = true;
		}
		else if(radio.available()){
		  uint8_t payload_size = radio.getPayloadSize();
		  radio.read(radio_buf, payload_size);
		  if(handle_read(&radio, radio_buf,  payload_size, data, &data_len)){
			end_transmission = handle_message();
		  }
		  start_wait_time = millis();
		}
	  }
	  last_update_time = millis();
  }
  
  if(sound_len) {
    LcdClear();
    LcdString(("Playing..."));
    play_tones(sound_data, sound_len);
    sound_len = 0;
  }
}

void loop()
{
  // READ DATA
  send_sensor_data();
  get_data_from_master();
  next_screen();
}


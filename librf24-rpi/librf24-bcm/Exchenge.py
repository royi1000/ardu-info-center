import rf_prot
import shelve
import time
import struct
import urllib2
from datetime import datetime
from xml.dom.minidom import parse
from exceptions import *

def enum(**enums):
    return type('Enum', (), enums)

COMMAND_TYPE = enum(device_init=0xf0, device_init_response=0xf1, sensor_data=0x30, screen_data=0x31)
DEVICE_TYPE = enum(screen=0x10, sensor=0x20)
DATA_TYPE = enum(date=0x1, string=0x2, bitmap=0x3, color_string=0x4, sound=0x5, remove_id=0x10, end_tx=0x20)
COLOR_TYPE = enum(c_red=1, c_green=2, c_blue=3,c_purple=4,c_yellow=5,c_aqua=6)
SENSOR_TYPE = enum(rh_temp=0x50, light=0x51, presure=0x52, moisture=0x53)

TONE_TYPE = enum (c_note_b0=0, c_note_c1=1, c_note_cs1=2, c_note_d1=3, c_note_ds1=4, c_note_e1=5, c_note_f1=6, c_note_fs1=7, c_note_g1=8,
c_note_gs1=9, c_note_a1=10, c_note_as1=11, c_note_b1=12, c_note_c2=13, c_note_cs2=14, c_note_d2=15, c_note_ds2=16, c_note_e2=17,
c_note_f2=18, c_note_fs2=19, c_note_g2=20, c_note_gs2=21, c_note_a2=22, c_note_as2=23, c_note_b2=24, c_note_c3=25, c_note_cs3=26,
c_note_d3=27, c_note_ds3=28, c_note_e3=29, c_note_f3=30, c_note_fs3=31, c_note_g3=32, c_note_gs3=33, c_note_a3=34, c_note_as3=35,
c_note_b3=36, c_note_c4=37, c_note_cs4=38, c_note_d4=39, c_note_ds4=40, c_note_e4=41, c_note_f4=42, c_note_fs4=43, c_note_g4=44,
c_note_gs4=45, c_note_a4=46, c_note_as4=47, c_note_b4=48, c_note_c5=49, c_note_cs5=50, c_note_d5=51, c_note_ds5=52, c_note_e5=53,
c_note_f5=54, c_note_fs5=55, c_note_g5=56, c_note_gs5=57, c_note_a5=58, c_note_as5=59, c_note_b5=60, c_note_c6=61, c_note_cs6=62,
c_note_d6=63, c_note_ds6=64, c_note_e6=65, c_note_f6=66, c_note_fs6=67, c_note_g6=68, c_note_gs6=69, c_note_a6=70, c_note_as6=71,
c_note_b6=72, c_note_c7=73, c_note_cs7=74, c_note_d7=75, c_note_ds7=76, c_note_e7=77, c_note_f7=78, c_note_fs7=79, c_note_g7=80,
c_note_gs7=81, c_note_a7=82, c_note_as7=83, c_note_b7=84, c_note_c8=85, c_note_cs8=86, c_note_d8=87, c_note_ds8=88)


colors = {'red':(200,0,0),
          'green':(0,200,0),
          'blue':(0,0,200),
          'purple':(200,0,200),
          'aqua':(0,200,200),
          'yellow':(200,200,0),
          'orange':(200,100,0),
         }

def set_rgb(red1,green1,blue1,red2,green2,blue2):
    return chr(red1)+chr(green1)+chr(blue1)+chr(red2)+chr(green2)+chr(blue2)

def seconds(): return int(round(time.time()))

class App(object):
    APP_NAME = 'undef'
    def __init__(self, interval=60):
        self.last_time = 0
        self.interval = interval
        self.last_data = ''

    def update_data(self, _id):
        self.last_time = seconds()
                    
    def get_data(self, _id):
        return self.data

    def should_run(self):
        if self.last_time + self.interval > seconds():
            return False
        return True

    def valid(self):
        if (self.last_time + (self.interval*10)) < seconds():
            return False
        return True

class DT(App):
    APP_NAME = 'time'
    def valid(self):
        return True

    def get_data(self, _id):
        self.update_data(_id)
        return self.data
        
    def update_data(self, _id):
        self.last_time = seconds()
        dt = datetime.now().timetuple()[0:6]
        self.data = chr(DATA_TYPE.date) + struct.pack('HBBBBB', *dt)
        
class ShortTest(App):
    APP_NAME='shorttest'
    def valid(self):
        return True
    
    def get_data(self, _id):
        return chr(DATA_TYPE.color_string) + chr(_id) + set_rgb(*(colors['red'] + colors['orange'])) +  ''.join([chr(i+220) for i in range(34)])
    
class LongTest(App):
    APP_NAME='longtest'
    def valid(self):
        return True
        
    def get_data(self,_id):
        return  chr(DATA_TYPE.color_string) + chr(_id) +  set_rgb(*(colors['purple'] + colors['aqua'])) +  'long test ' * 4

class SoundTest(App):
    APP_NAME='soundtest'
    play=True
    def valid(self):
        if self.play:
            self.play = False
        return True
        
    def get_data(self,_id):
        return  chr(DATA_TYPE.sound) + "".join([chr(i) for i in [TONE_TYPE.c_note_c8, TONE_TYPE.c_note_d8, TONE_TYPE.c_note_e4, TONE_TYPE.c_note_f4, TONE_TYPE.c_note_g4 ]])

class Gmail(App):
    APP_NAME='gmail'
    def update_data(self, _id):
        user,passwd = open('gmail.txt').readline().split(',')
        try:
            self.count = self.get_unread_msgs(user,passwd)
            self.last_time = seconds()
        except:
            print "get gmail count failed"

    def get_data(self,_id):
        if self.count:
            return chr(DATA_TYPE.color_string) + chr(_id) +  set_rgb(*(colors['purple'] + colors['green'])) +  'Gmail '.ljust(12,' ') + 'unread:{}'.format(self.count)
        return ''
            
    def valid(self):
        return self.count
        
    def get_unread_msgs(self, user, passwd):
        auth_handler = urllib2.HTTPBasicAuthHandler()
        auth_handler.add_password(
        realm='New mail feed',
        uri='https://mail.google.com',
        user='%s@gmail.com' % user,
        passwd=passwd)
        opener = urllib2.build_opener(auth_handler)
        urllib2.install_opener(opener)
        feed = urllib2.urlopen('https://mail.google.com/mail/feed/atom')
        dom=parse(feed)
        count_obj = dom.getElementsByTagName("fullcount")[0]
        # get its text and convert it to an integer
        return int(count_obj.firstChild.wholeText)

class Sensor(object):
    def __init__(self, _id, name, interval = 600):
        self.interval = interval
        self._id = _id
        self.last_data = ()
        self.last_time = 0
        self.new_data = False
        self.name = name

    def update(self, data):
        self.last_data = data
        self.last_time = seconds()
        self.new_data = True

    def get_data(self,_id):
        raise NotImplementedError

    def is_new_data(self):
        if self.new_data:
            self.new_data = False
            return True
        return False
 
    def valid(self):
        if (self.last_time + (self.interval*2)) < seconds():
            return False
        return True

class RTSensor(Sensor):
    def get_data(self, _id):
        red = blue = green1 = green2 = 0
        try:
            if self.temp < 25:
                green1 = 200
                red = 0
                blue = int(-5 * (self.temp - 40))
            else:
                green2 = 200
                red = int(self.temp) * 5 
                if red > 255: red = 255
                blue = 0
        except:
            pass
        name =''
        if self.name:
            name = self.name.ljust(12,' ')
        return chr(DATA_TYPE.color_string) + chr(_id) + set_rgb(red,green1,0,0,green2,blue) + "{0:}RH:   {1:2.2f} TEMP: {2:2.2f}".format(name, self.rh, self.temp)

    def update(self, data):
        self.last_data = data
        self.rh, self.temp = data
        self.last_time = seconds()
        self.new_data = True

class LightSensor(Sensor):
    def get_data(self, _id):
        name =''
        if self.name:
            name = self.name.ljust(12,' ')
        return chr(DATA_TYPE.string) + chr(_id) +  "{0:}LUX: {1:2.2f} ".format(name, self.last_data)
 
class MoistureSensor(Sensor):
    def get_data(self, _id):
        name =''
        if self.name:
            name = self.name.ljust(12,' ')
        pre_str = chr(DATA_TYPE.string) + chr(_id)
        if self.last_data < 300:
            pre_str = chr(DATA_TYPE.color_string) + chr(_id) + set_rgb(255,0,0,0,0,0)
        return pre_str +  "{0:}Soil RH:{1:} ".format(name, self.last_data)

class PresureSensor(Sensor):
    def get_data(self, _id):
        temp, presure = self.last_data
        name =''
        if self.name:
            name = self.name.ljust(12,' ')
        return chr(DATA_TYPE.string) + chr(_id) +  name + "Pha: {0:2.2f}".format(presure).ljust(12,' ') + "Temp:  {0:2.2f} ".format(temp)
            
class RFExchange(object):
    def __init__(self, apps):
        #rx_addrs = [0xF0F0F0F01F, 0xF0F0F0F02F, 0xF0F0F0F03F, 0xF0F0F0F04F, 0xF0F0F0F05F]
        self._rf = rf_prot.RF24_Wrapper()
        self._db = shelve.open('rf_exchange_db')
        self.apps = apps
        self.sensors = dict()
        if not self._db.has_key('init'):
            self._db['init'] = True
            self._db['current_pipe'] = 0
            self._db['in_devices'] = []
            self._db['out_devices'] = []
            self._db['app_data'] = {}
            self._db['sensors'] = {}

    def get_sensor_name(self, _id):
        try:
            f=open('sensor_names.py', 'r+').read()
            x=eval(f)
        except:
            open('sensor_names.py', 'w')
            x={}
        if x.has_key(_id):
            return x[_id]
        x[_id] = ''
        open('sensor_names.py', 'w').write(str(x))

    def handle_rx_data(self, data):
        cmd = struct.unpack('B', data[0])[0]
        if cmd == COMMAND_TYPE.sensor_data:
            addr = struct.unpack('Q', data[2:10])[0]
            sensor_type = struct.unpack('B', data[1])[0]
            _id = (addr, sensor_type)
            if sensor_type == SENSOR_TYPE.rh_temp:
                rh, temp = struct.unpack('ff', data[10:18])
                if not addr in self.sensors.keys():
                    name = self.get_sensor_name(_id)
                    self.sensors[_id] = RTSensor(_id, name)
                self.sensors[_id].update((rh,temp))
                print "rh: {} temp: {}".format(rh,temp)
            if sensor_type == SENSOR_TYPE.light:
                lux = struct.unpack('H', data[10:12])[0]
                if not addr in self.sensors.keys():
                    name = self.get_sensor_name(_id)
                    self.sensors[_id] = LightSensor(_id, name)
                self.sensors[_id].update(lux)
                print "light lux: {}".format(lux)
            if sensor_type == SENSOR_TYPE.moisture:
                rh = struct.unpack('H', data[10:12])[0]
                if not addr in self.sensors.keys():
                    name = self.get_sensor_name(_id)
                    self.sensors[_id] = MoistureSensor(_id, name)
                self.sensors[_id].update(rh)
                print "soil moisture: {}".format(rh)
            if sensor_type == SENSOR_TYPE.presure:
                temp,presure = struct.unpack('ff', data[10:18])
                if not addr in self.sensors.keys():
                    name = self.get_sensor_name(_id)
                    self.sensors[_id] = PresureSensor(_id, name)
                self.sensors[_id].update((temp,presure))
                print "presure: {} temp: {}".format(presure, temp)

        if cmd == COMMAND_TYPE.device_init:
            time.sleep(0.05)
            dev_type = struct.unpack('B', data[1])[0]
            addr = struct.unpack('Q', data[2:10])[0]
            print "got device init cmd: {}, dev type: {}, addr: {}".format(hex(cmd), hex(dev_type), hex(addr))
            if dev_type==DEVICE_TYPE.screen:
                if not addr in self._db['out_devices']:
                    print repr(addr)
                    x=self._db['out_devices']
                    x.append(addr)
                    self._db['out_devices'] = x
                data=chr(COMMAND_TYPE.device_init_response)+data[1:]
                self._rf.write(addr, data[:10])
                for _id, app in enumerate(self.apps + self.sensors.values()):
                    if app.valid():
                        data = app.get_data(_id)
                        print repr(data)
                        self._rf.write(addr, data)
                    else:
                        data = chr(DATA_TYPE.remove_id) + chr(_id)
                        self._rf.write(addr, data)
                self.send_end_tx_msg(addr)
            elif dev_type==DEVICE_TYPE.sensor:
                if not addr in self._db['in_devices']:
                    print repr(addr)
                    x=self._db['in_devices']
                    x.append(addr)
                    self._db['in_devices'] = x
                data=chr(COMMAND_TYPE.device_init_response)+data[1:]
                self._rf.write(addr, data[:10])
 
    def send_end_tx_msg(self, addr):
        data = chr(DATA_TYPE.end_tx)
        self._rf.write(addr, data)
                        
    def run(self):
        try:
            while True:
                changed = []
                for _id, app in enumerate(self.apps):
                    if app.should_run():
                        app.update_data(_id)
                for _id, sensor in self._db['sensors'].items():
                    if sensor.is_new_data:
                        changed.append(_id)
                start_time = seconds()
                data = '1'
                while data:
                    (pipe, data) = self._rf.read(3000)
                    if data:
                        self.handle_rx_data(data)
                        start_time = seconds()
                
        finally:
            self._db.close()


RFExchange([DT(),Gmail(),ShortTest()]).run()

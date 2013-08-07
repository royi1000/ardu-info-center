import time
import struct
from rf24 import *
from ctypes import *
from exceptions import *


class Byte(object):
    def __init__(self, size):
        self.ptr=new_byteArray(size+32)
        self.size=size
    def __del__(self):
        delete_byteArray(self.ptr)
    def write(self, iteritem):
        if len(iteritem)>self.size:
            raise IndexError('Item size: {} bigger than array size: {}'.format(len(iteritem), self.size))
        for i in range(len(iteritem)):
            byteArray_setitem(self.ptr, i, ord(iteritem[i]))
    def read(self, size=0):
        if not size:
            size = self.size
        if size>self.size:
            raise IndexError('Item size: {} bigger than array size: {}'.format(len(iteritem), self.size))
        res = ''
        for i in range(size):
            res += chr(byteArray_getitem(self.ptr, i))
        return res


def millis(): return int(round(time.time() * 1000))%0xffffffff

def ping():
    radio=RF24(RPI_V2_GPIO_P1_15, RPI_V2_GPIO_P1_26, BCM2835_SPI_SPEED_8MHZ)
    pipes = [ 0xF0F0F0F0E1, 0xF0F0F0F0D2 ]
    radio.begin()
    radio.setRetries(15,15)
    radio.setChannel(0x4c)
    radio.setPALevel(RF24_PA_HIGH)
    radio.openWritingPipe(pipes[0])
    radio.openReadingPipe(1,pipes[1])
    radio.startListening()
    radio.printDetails()
    while True:
        radio.stopListening()
        t=millis()
        buf=struct.pack('L',t)
        w_buf=Byte(len(buf))
        w_buf.write(buf)
        ok=radio.write(w_buf.ptr,w_buf.size)
        if ok:
            print "ok"
        else:
            print "failed"
        radio.startListening()
        w=millis()
        timeout=False
        while not radio.available() and not timeout:
            time.sleep(0.005)
            if millis()-w>200:
                timeout = True
        if timeout:
            print "Failed on timeout"
        else:
            r_buf=Byte(4)
            radio.read(r_buf.ptr,r_buf.size)
            buf = r_buf.read(4)
            res = struct.unpack('L',buf)[0] 
            print "response {}, round-trip delay: {}".format(res, millis()-res)
        time.sleep(1)

if __name__ == "__main__":
    ping()

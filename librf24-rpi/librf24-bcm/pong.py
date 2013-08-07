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

def pong():
    radio=RF24(RPI_V2_GPIO_P1_15, RPI_V2_GPIO_P1_26, BCM2835_SPI_SPEED_8MHZ)
    pipes = [ 0xF0F0F0F0E1, 0xF0F0F0F0D2 ]
    radio.begin()
    radio.setRetries(15,15)
    radio.setChannel(0x4c)
    radio.setPALevel(RF24_PA_HIGH)
    radio.openWritingPipe(pipes[1])
    radio.openReadingPipe(1,pipes[0])
    radio.startListening()
    radio.printDetails()
    while True:
        while radio.available():
            done = False
            r_buf = Byte(4)
            while not done:
                done = radio.read(r_buf.ptr,r_buf.size)
                buf = r_buf.read(4)
                res = struct.unpack('L',buf)[0]
                print "Got payload({}) {}...".format(r_buf.size, res)
                time.sleep(0.03)
            radio.stopListening()
            ok=radio.write(r_buf.ptr,r_buf.size)
            if ok:
                print "sent back ok"
            else:
                print "sent back failed"
            radio.startListening()

if __name__ == "__main__":
    pong()

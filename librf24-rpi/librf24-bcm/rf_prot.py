from rf24 import *
import struct
from exceptions import *
import time

_DEBUG=True

def enum(**enums):
    return type('Enum', (), enums)

PACKET_TYPE = enum(stand_alone=0, first_packet=1, continued_packet=2, last_packet=3)
MAX_PAYLOAD_SIZE = 32
PAYLOAD_SIZE = 20

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

def print_debug(line):
    if _DEBUG:
        print line
    
class RF24_Wrapper(object):
    def __init__(self, tx_addr=0xF0F0F0F0D2, rx_addrs=[0xF0F0F0F0E1], channel=0x4c, level=RF24_PA_HIGH):
        self.radio=RF24(RPI_V2_GPIO_P1_15, RPI_V2_GPIO_P1_26, BCM2835_SPI_SPEED_8MHZ)
        self.radio.begin()
        self.debug=_DEBUG
        self.radio.setRetries(15,15)
        #self.radio.setChannel(channel)
        #self.radio.setPALevel(level)
        #self.radio.enableDynamicPayloads()
        #self.radio.enableAckPayload()
        self.radio.setPayloadSize(PAYLOAD_SIZE)
        self.radio.setAutoAck(True)
        self.channel = channel
        self.level = level
        self.tx_addr = tx_addr
        self.rx_addrs = rx_addrs
        self._buffer = Byte(32)
        self._pipe_num = Byte(1)
        self.rx_len = len(rx_addrs)
        self.radio.openWritingPipe(tx_addr)
        self._pipes_bufs = [[0,''], [0,''], [0,''], [0,''], [0,''], [0,''], ]
        if self.rx_len>5:
            raise IndexError('too much rx addresses (max 5)')
        for i in range(self.rx_len):
            if (0xFFFFFFFF00 & rx_addrs[0]) != (0xFFFFFFFF00 & rx_addrs[i]):
                raise ValueError('only last adddress byte must be diffrent to all rx address')
            self.radio.openReadingPipe(i+1, rx_addrs[i])
        #print "PRD (noise on channel) {}".format(self.radio.testRPD())
        self.radio.startListening()
        self.radio.printDetails()
        
    def read(self, timeout=1000):
        start=millis()
        while millis()<start+timeout:
            if self.radio.available(self._pipe_num.ptr):
                pipe = struct.unpack('B',self._pipe_num.read(1))[0]
                #print "radio available, pipe:{}".format(pipe)
                last_size = self.radio.getPayloadSize()
                self.radio.read(self._buffer.ptr, last_size)
                buf = self._buffer.read(last_size)
                ctrl_byte = ord(buf[0])
                packet_type = ctrl_byte >> 6
                payload_len = ctrl_byte & 0b111111
                if payload_len > MAX_PAYLOAD_SIZE or payload_len > last_size:
                    raise ValueError('payload size too big: {}'.format(payload_len))
                buf = buf[:payload_len+1]
                if packet_type == PACKET_TYPE.stand_alone:
                    if self._pipes_bufs[pipe][0]:
                        print "payload stream error on pipe: {}".format(pipe)
                    self._pipes_bufs[pipe] = [0, '']
                    print_debug('got stand alone packet, len: {}'.format(payload_len-1))
                    return (pipe, buf[1:payload_len])
                        
                elif packet_type == PACKET_TYPE.first_packet:
                    if self._pipes_bufs[pipe][0]:
                        print "payload stream error on pipe: {}".format(pipe)
                    tot_len = struct.unpack('H', buf[1:3])[0]
                    self._pipes_bufs[pipe] = [tot_len, buf[3:payload_len+1]]

                elif packet_type == PACKET_TYPE.continued_packet:
                    if not self._pipes_bufs[pipe][0]:
                        print "payload stream error on pipe: {} got continued packet without start".format(pipe)
                        self._pipes_bufs[pipe] = [0, '']
                    else:
                        self._pipes_bufs[pipe][1] += buf[1:payload_len+1]
                    
                elif packet_type == PACKET_TYPE.last_packet:
                    if not self._pipes_bufs[pipe][0]:
                        print "payload stream error on pipe: {} got last packet without start".format(pipe)
                        self._pipes_bufs[pipe] = [0, '']
                    else:
                        self._pipes_bufs[pipe][1] += buf[1:payload_len+1]
                        if len(self._pipes_bufs[pipe][1]) !=  self._pipes_bufs[pipe][0]:
                            print "invalid size: {} on pipe: {} expected size: {}".format(len(self._pipes_bufs[pipe][1]),
                                                                                          pipe, self._pipes_bufs[pipe][0])
                            self._pipes_bufs[pipe] = [0, '']
                        else:
                            res = self._pipes_bufs[pipe]
                            self._pipes_bufs[pipe] = [0, '']
                            return res                            
        return (None, None)

    def write(self, tx_addr, tx_buf):
        self.radio.stopListening()
        self.radio.openWritingPipe(tx_addr)
        buf_len = len(tx_buf)
        print buf_len
        max_payload_size = self.radio.getPayloadSize()
        if buf_len < max_payload_size:
            first_packet = PACKET_TYPE.stand_alone << 6
            first_packet |= buf_len+1
            self._buffer.write(chr(first_packet)+tx_buf)
            print_debug('sending single packet: {} len: {}'.format(repr(self._buffer.read(buf_len+1)),buf_len+1))
            res = self.radio.write(self._buffer.ptr, buf_len+1)
            self.radio.startListening()
            return res
         
        first_packet = PACKET_TYPE.first_packet << 6
        first_packet |= max_payload_size
        start = chr(first_packet) + struct.pack('H', buf_len)
        self._buffer.write(start+tx_buf[:max_payload_size-3])
        print_debug('sending first packet: {} len: {}'.format(repr(start+tx_buf[:max_payload_size-3]),max_payload_size))
        ret = self.radio.write(self._buffer.ptr, max_payload_size)
#        if not ret:
#            self.radio.startListening()
#            return False
        remain_buf = tx_buf[max_payload_size-3:]
        while len(remain_buf) > max_payload_size-1:
            time.sleep(0.1)
            first_packet = PACKET_TYPE.continued_packet << 6
            first_packet |= max_payload_size
            s_buf = chr(first_packet)+remain_buf[:max_payload_size-1]
            self._buffer.write(s_buf)
            print_debug('sending continued packet: {} len: {}'.format(repr(s_buf),len(s_buf)))
            ret = self.radio.write(self._buffer.ptr, len(s_buf))
#            if not ret:
#                self.radio.startListening()
#                return False
            remain_buf = remain_buf[max_payload_size-1:]
                
        time.sleep(0.1)
        first_packet = PACKET_TYPE.last_packet << 6
        first_packet |= len(remain_buf)+1
        s_buf = chr(first_packet)+remain_buf
        self._buffer.write(s_buf)
        print_debug('sending last packet: {} len: {}'.format(repr(s_buf),len(s_buf)))
        ret = self.radio.write(self._buffer.ptr, len(s_buf))
        self.radio.startListening()
#        if not ret:
#            return False
        return True

        

# Python wrapper for Raspberry PI driver for nRF24L01 2.4GHz Wireless Transceiver

cd librf24-rpi/librf24-bcm/
sudo python ping.py //sometime you need to use T and R in the arduino serial to start receiving packets
or
import rf24

# To rebuild the lib:
* make
* swig -python -c++ RF24.i
* gcc -fPIC -c RF24_wrap.cxx -I/usr/include/python2.7
* gcc -lstdc++ -shared bcm2835.o RF24.o RF24_wrap.o -o _rf24.so

you will also need to install the following packages (once):
* sudo apt-get install python-dev
* sudo apt-get install swig


look at ping.py for an example

Design Goals: This library is designed to be...

* Maximally compliant with the intended operation of the chip
* Easy for beginners to use
* Consumed with a public interface that's similiar to other Arduino standard libraries
* Built against the standard SPI library. 

Please refer to:

* [Documentation Main Page](http://maniacbug.github.com/RF24)
* [RF24 Class Documentation](http://maniacbug.github.com/RF24/classRF24.html)
* [Source Code](https://github.com/maniacbug/RF24)
* [Downloads](https://github.com/maniacbug/RF24/archives/master)
* [Chip Datasheet](http://www.nordicsemi.com/files/Product/data_sheet/nRF24L01_Product_Specification_v2_0.pdf)

This chip uses the SPI bus, plus two chip control pins.  Remember that pin 10 must still remain an output, or
the SPI hardware will go into 'slave' mode.


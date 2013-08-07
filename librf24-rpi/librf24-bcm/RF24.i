%module rf24
%{
#include "RF24.h"
%}

%include "stdint.i"
%include "RF24.h"
//%include "bcm2835.h"
%include "carrays.i"
%include "pointer.i"

 //%array_class(char, byteArray);
%array_functions(uint8_t, byteArray);
//%pointer_class(int,intp)
//%pointer_class(long,longp)
//%pointer_class(char,charp)


typedef enum
{
    RPI_GPIO_P1_03        =  0,  ///< Version 1, Pin P1-03
    RPI_GPIO_P1_05        =  1,  ///< Version 1, Pin P1-05
    RPI_GPIO_P1_07        =  4,  ///< Version 1, Pin P1-07
    RPI_GPIO_P1_08        = 14,  ///< Version 1, Pin P1-08, defaults to alt function 0 UART0_TXD
    RPI_GPIO_P1_10        = 15,  ///< Version 1, Pin P1-10, defaults to alt function 0 UART0_RXD
    RPI_GPIO_P1_11        = 17,  ///< Version 1, Pin P1-11
    RPI_GPIO_P1_12        = 18,  ///< Version 1, Pin P1-12
    RPI_GPIO_P1_13        = 21,  ///< Version 1, Pin P1-13
    RPI_GPIO_P1_15        = 22,  ///< Version 1, Pin P1-15
    RPI_GPIO_P1_16        = 23,  ///< Version 1, Pin P1-16
    RPI_GPIO_P1_18        = 24,  ///< Version 1, Pin P1-18
    RPI_GPIO_P1_19        = 10,  ///< Version 1, Pin P1-19, MOSI when SPI0 in use
    RPI_GPIO_P1_21        =  9,  ///< Version 1, Pin P1-21, MISO when SPI0 in use
    RPI_GPIO_P1_22        = 25,  ///< Version 1, Pin P1-22
    RPI_GPIO_P1_23        = 11,  ///< Version 1, Pin P1-23, CLK when SPI0 in use
    RPI_GPIO_P1_24        =  8,  ///< Version 1, Pin P1-24, CE0 when SPI0 in use
    RPI_GPIO_P1_26        =  7,  ///< Version 1, Pin P1-26, CE1 when SPI0 in use

    // RPi Version 2
    RPI_V2_GPIO_P1_03     =  2,  ///< Version 2, Pin P1-03
    RPI_V2_GPIO_P1_05     =  3,  ///< Version 2, Pin P1-05
    RPI_V2_GPIO_P1_07     =  4,  ///< Version 2, Pin P1-07
    RPI_V2_GPIO_P1_08     = 14,  ///< Version 2, Pin P1-08, defaults to alt function 0 UART0_TXD
    RPI_V2_GPIO_P1_10     = 15,  ///< Version 2, Pin P1-10, defaults to alt function 0 UART0_RXD
    RPI_V2_GPIO_P1_11     = 17,  ///< Version 2, Pin P1-11
    RPI_V2_GPIO_P1_12     = 18,  ///< Version 2, Pin P1-12
    RPI_V2_GPIO_P1_13     = 27,  ///< Version 2, Pin P1-13
    RPI_V2_GPIO_P1_15     = 22,  ///< Version 2, Pin P1-15
    RPI_V2_GPIO_P1_16     = 23,  ///< Version 2, Pin P1-16
    RPI_V2_GPIO_P1_18     = 24,  ///< Version 2, Pin P1-18
    RPI_V2_GPIO_P1_19     = 10,  ///< Version 2, Pin P1-19, MOSI when SPI0 in use
    RPI_V2_GPIO_P1_21     =  9,  ///< Version 2, Pin P1-21, MISO when SPI0 in use
    RPI_V2_GPIO_P1_22     = 25,  ///< Version 2, Pin P1-22
    RPI_V2_GPIO_P1_23     = 11,  ///< Version 2, Pin P1-23, CLK when SPI0 in use
    RPI_V2_GPIO_P1_24     =  8,  ///< Version 2, Pin P1-24, CE0 when SPI0 in use
    RPI_V2_GPIO_P1_26     =  7,  ///< Version 2, Pin P1-26, CE1 when SPI0 in use

    // RPi Version 2, new plug P5
    RPI_V2_GPIO_P5_03     = 28,  ///< Version 2, Pin P5-03
    RPI_V2_GPIO_P5_04     = 29,  ///< Version 2, Pin P5-04
    RPI_V2_GPIO_P5_05     = 30,  ///< Version 2, Pin P5-05
    RPI_V2_GPIO_P5_06     = 31,  ///< Version 2, Pin P5-06

} RPiGPIOPin;

typedef enum
{
    BCM2835_SPI_CLOCK_DIVIDER_65536 = 0,       ///< 65536 = 262.144us = 3.814697260kHz
    BCM2835_SPI_CLOCK_DIVIDER_32768 = 32768,   ///< 32768 = 131.072us = 7.629394531kHz
    BCM2835_SPI_CLOCK_DIVIDER_16384 = 16384,   ///< 16384 = 65.536us = 15.25878906kHz
    BCM2835_SPI_CLOCK_DIVIDER_8192  = 8192,    ///< 8192 = 32.768us = 30/51757813kHz
    BCM2835_SPI_CLOCK_DIVIDER_4096  = 4096,    ///< 4096 = 16.384us = 61.03515625kHz
    BCM2835_SPI_CLOCK_DIVIDER_2048  = 2048,    ///< 2048 = 8.192us = 122.0703125kHz
    BCM2835_SPI_CLOCK_DIVIDER_1024  = 1024,    ///< 1024 = 4.096us = 244.140625kHz
    BCM2835_SPI_CLOCK_DIVIDER_512   = 512,     ///< 512 = 2.048us = 488.28125kHz
    BCM2835_SPI_CLOCK_DIVIDER_256   = 256,     ///< 256 = 1.024us = 976.5625MHz
    BCM2835_SPI_CLOCK_DIVIDER_128   = 128,     ///< 128 = 512ns = = 1.953125MHz
    BCM2835_SPI_CLOCK_DIVIDER_64    = 64,      ///< 64 = 256ns = 3.90625MHz
    BCM2835_SPI_CLOCK_DIVIDER_32    = 32,      ///< 32 = 128ns = 7.8125MHz
    BCM2835_SPI_CLOCK_DIVIDER_16    = 16,      ///< 16 = 64ns = 15.625MHz
    BCM2835_SPI_CLOCK_DIVIDER_8     = 8,       ///< 8 = 32ns = 31.25MHz
    BCM2835_SPI_CLOCK_DIVIDER_4     = 4,       ///< 4 = 16ns = 62.5MHz
    BCM2835_SPI_CLOCK_DIVIDER_2     = 2,       ///< 2 = 8ns = 125MHz, fastest you can get
    BCM2835_SPI_CLOCK_DIVIDER_1     = 1,       ///< 0 = 262.144us = 3.814697260kHz, same as 0/65536
} bcm2835SPIClockDivider;

typedef enum
{
    BCM2835_SPI_SPEED_8KHZ = BCM2835_SPI_CLOCK_DIVIDER_32768 ,   ///< 32768 = 131.072us = 7.629394531kHz
    BCM2835_SPI_SPEED_16KHZ = BCM2835_SPI_CLOCK_DIVIDER_16384 ,   ///< 16384 = 65.536us = 15.25878906kHz
    BCM2835_SPI_SPEED_32KHZ = BCM2835_SPI_CLOCK_DIVIDER_8192  ,    ///< 8192 = 32.768us = 30/51757813kHz
    BCM2835_SPI_SPEED_64KHZ = BCM2835_SPI_CLOCK_DIVIDER_4096  ,    ///< 4096 = 16.384us = 61.03515625kHz
    BCM2835_SPI_SPEED_128KHZ = BCM2835_SPI_CLOCK_DIVIDER_2048  ,    ///< 2048 = 8.192us = 122.0703125kHz
    BCM2835_SPI_SPEED_256KHZ = BCM2835_SPI_CLOCK_DIVIDER_1024  ,    ///< 1024 = 4.096us = 244.140625kHz
    BCM2835_SPI_SPEED_512KHZ = BCM2835_SPI_CLOCK_DIVIDER_512   ,     ///< 512 = 2.048us = 488.28125kHz
    BCM2835_SPI_SPEED_1MHZ = BCM2835_SPI_CLOCK_DIVIDER_256   ,     ///< 256 = 1.024us = 976.5625MHz
    BCM2835_SPI_SPEED_2MHZ = BCM2835_SPI_CLOCK_DIVIDER_128   ,     ///< 128 = 512ns = = 1.953125MHz
    BCM2835_SPI_SPEED_4MHZ = BCM2835_SPI_CLOCK_DIVIDER_64    ,      ///< 64 = 256ns = 3.90625MHz
    BCM2835_SPI_SPEED_8MHZ = BCM2835_SPI_CLOCK_DIVIDER_32    ,      ///< 32 = 128ns = 7.8125MHz
    BCM2835_SPI_SPEED_16MHZ = BCM2835_SPI_CLOCK_DIVIDER_16    ,      ///< 16 = 64ns = 15.625MHz
    BCM2835_SPI_SPEED_32MHZ = BCM2835_SPI_CLOCK_DIVIDER_8,       ///< 8 = 32ns = 31.25MHz
    BCM2835_SPI_SPEED_64MHZ = BCM2835_SPI_CLOCK_DIVIDER_4,       ///< 4 = 16ns = 62.5MHz
} bcm2835SPISpeed;


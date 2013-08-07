#ifndef __LCD_H__
#define __LCD_H__

#include <SPI.h>

/* 
  *** PCD8544 Software SPI support ***
  This Code has extra features
  including a XY positioning function on Display
  and a Line Draw function on Nokia 3310 LCD
  It is modded from the original
  http://playground.arduino.cc/Code/PCD8544
*/
// Mods by Jim Park
// jim(^dOt^)buzz(^aT^)gmail(^dOt^)com
// hope it works for you
#define PIN_SCE   7  // LCD CS  .... Pin 3
#define PIN_RESET 6  // LCD RST .... Pin 1
#define PIN_DC    5  // LCD Dat/Com. Pin 5
#define PIN_SDIN  4  // LCD SPIDat . Pin 6
#define PIN_SCLK  3  // LCD SPIClk . Pin 4
                     // LCD Gnd .... Pin 2
                     // LCD Vcc .... Pin 8
                     // LCD Vlcd ... Pin 7

#define LCD_C     LOW
#define LCD_D     HIGH

#define LCD_X     84
#define LCD_Y     48
#define LCD_CMD   0

void LcdCharacter(unsigned char character);
void LcdClear(void);
void LcdInitialise(void);
void LcdString(char *characters);
void LcdWrite(byte dc, byte data);
void Lcdpixel(int x, int y);
void gotoXY(int x, int y);
void drawLine(void);
void printDigits(unsigned long digits, int num_of_digits, char delimiter);

#endif
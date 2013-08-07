#ifndef __LED_H__
#define __LED_H__
#define RED_PIN A0
#define BLUE_PIN A2
#define GREEN_PIN A1

#define RED color(100,0,0,10)
#define GREEN color(0,100,0,10)
#define BLUE color(0,0,100,10)
#define PURPLE color(100,0,100,10)
#define AQUA color(0,100,100,10)
#define YELLOW color(100,100,0,10)

void color(int red, int green, int blue, int time) ;
void fade(int fromred, int fromgreen, int fromblue,
             int tored,  int togreen, int toblue,
             int time) ;

void red(int time);
void green(int time);
void blue(int time);
void purple(int time) ;
void aqua(int time) ;
void yellow(int time) ;
#endif
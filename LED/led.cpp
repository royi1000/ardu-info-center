#include "led.h"
#include <SPI.h>

void color(int red, int green, int blue, int time) {
    if (red){
        digitalWrite(RED_PIN, LOW);    // turn the LED off by making the voltage LOW
        delay(4 * red / 100);
        digitalWrite(RED_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    }
    if (green){
        digitalWrite(GREEN_PIN, LOW);    // turn the LED off by making the voltage LOW
        delay(6 *  green/ 100);
        digitalWrite(GREEN_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    }
    if (blue){
        digitalWrite(BLUE_PIN, LOW);    // turn the LED off by making the voltage LOW
        delay(6 *  blue /100);
        digitalWrite(BLUE_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    }
    //delay(time);
}

void fade(int fromred, int fromgreen, int fromblue,
             int tored,  int togreen, int toblue,
             int time) 
{
    int redint = 0;
    int blueint = 0;
    int greenint = 0;
    if (!time) return;
    for(int i=0; i<time;i++){
        redint = fromred + ((tored - fromred) * i/time);
        greenint = fromgreen + ((togreen - fromgreen) * i/time);
        blueint = fromblue + ((toblue - fromblue) * i/time);
        for(int j=0;j<10;j++){
            color(redint, greenint, blueint,1);
        }
    }
}

void red(int time){
    unsigned long  start = millis();
    while(millis() < start + 1000){
        RED;
    };
}

void green(int time){
    unsigned long  start = millis();
    while(millis() < start + 1000){
        GREEN;
    };
}

void blue(int time){
    unsigned long  start = millis();
    while(millis() < start + 1000){
        BLUE;
    };
}

void purple(int time) {
    unsigned long  start = millis();
    while(millis() < start + 1000){
        PURPLE;
    };
}

void aqua(int time) {
    unsigned long  start = millis();
    while(millis() < start + 1000){
        AQUA;
    };
}

void yellow(int time) {
    unsigned long  start = millis();
    while(millis() < start + 1000){
        YELLOW;
    };
}

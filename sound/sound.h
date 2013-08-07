#ifndef __SOUND_H__
#define __SOUND_H__
/*************************************************
 * Public Constants
 *************************************************/
#include <SPI.h>
#define SOUND_PIN 8

typedef enum {c_note_b0=0, c_note_c1=1, c_note_cs1=2, c_note_d1=3, c_note_ds1=4, c_note_e1=5, c_note_f1=6, c_note_fs1=7, c_note_g1=8,
c_note_gs1=9, c_note_a1=10, c_note_as1=11, c_note_b1=12, c_note_c2=13, c_note_cs2=14, c_note_d2=15, c_note_ds2=16, c_note_e2=17,
c_note_f2=18, c_note_fs2=19, c_note_g2=20, c_note_gs2=21, c_note_a2=22, c_note_as2=23, c_note_b2=24, c_note_c3=25, c_note_cs3=26,
c_note_d3=27, c_note_ds3=28, c_note_e3=29, c_note_f3=30, c_note_fs3=31, c_note_g3=32, c_note_gs3=33, c_note_a3=34, c_note_as3=35,
c_note_b3=36, c_note_c4=37, c_note_cs4=38, c_note_d4=39, c_note_ds4=40, c_note_e4=41, c_note_f4=42, c_note_fs4=43, c_note_g4=44,
c_note_gs4=45, c_note_a4=46, c_note_as4=47, c_note_b4=48, c_note_c5=49, c_note_cs5=50, c_note_d5=51, c_note_ds5=52, c_note_e5=53,
c_note_f5=54, c_note_fs5=55, c_note_g5=56, c_note_gs5=57, c_note_a5=58, c_note_as5=59, c_note_b5=60, c_note_c6=61, c_note_cs6=62,
c_note_d6=63, c_note_ds6=64, c_note_e6=65, c_note_f6=66, c_note_fs6=67, c_note_g6=68, c_note_gs6=69, c_note_a6=70, c_note_as6=71,
c_note_b6=72, c_note_c7=73, c_note_cs7=74, c_note_d7=75, c_note_ds7=76, c_note_e7=77, c_note_f7=78, c_note_fs7=79, c_note_g7=80,
c_note_gs7=81, c_note_a7=82, c_note_as7=83, c_note_b7=84, c_note_c8=85, c_note_cs8=86, c_note_d8=87, c_note_ds8=88} tone_type_e;

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978


void play_tones(uint8_t* melody, unsigned int len);

#endif

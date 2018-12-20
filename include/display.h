/* ========================================================================
 * [PROJECT]    SIR100
 * [MODULE]     Display
 * [TITLE]      display header file
 * [FILE]       display.h
 * [VSN]        1.0
 * [CREATED]    030414
 * [LASTCHNGD]  030414
 * [COPYRIGHT]  Copyright (C) STREAMIT BV 2010
 * [PURPOSE]    API and gobal defines for display module
 * ======================================================================== */

#ifndef _Display_H
#define _Display_H

/*-------------------------------------------------------------------------*/
/* global defines                                                          */
/*-------------------------------------------------------------------------*/
#define DISPLAY_SIZE                16
#define NROF_LINES                  2
#define MAX_SCREEN_CHARS            (NROF_LINES*DISPLAY_SIZE)

#define LINE_0                      0
#define LINE_1                      1

#define FIRSTPOS_LINE_0             0
#define FIRSTPOS_LINE_1             0x40


#define LCD_BACKLIGHT_ON            1
#define LCD_BACKLIGHT_OFF           0

#define ALL_ZERO          			0x00      // 0000 0000 B
#define WRITE_COMMAND     			0x02      // 0000 0010 B
#define WRITE_DATA        			0x03      // 0000 0011 B
#define READ_COMMAND      			0x04      // 0000 0100 B
#define READ_DATA         			0x06      // 0000 0110 B


/*-------------------------------------------------------------------------*/
/* typedefs & structs                                                      */
/*-------------------------------------------------------------------------*/
typedef struct CustomIcon
{
	int pixelData[8];
} CustomIcon_t;

#define CUSTOMICON_ARRAY_SIZE 7
static CustomIcon_t CustomIconsData[] = 
{
	{{0x0E, 0x1B, 0x11, 0x00, 0x0E, 0x0A, 0x00, 0x04}}, // Wifi
	{{0x11, 0x13, 0x17, 0x11, 0x11, 0x1D, 0x19, 0x11}},	// Sync
	{{0x00, 0x04, 0x0E, 0x0E, 0x0E, 0x1F, 0x04, 0x00}}, // Alarm
	{{0x04, 0x0E, 0x1F, 0x00, 0x00, 0x1F, 0x0E, 0x04}}, // Arrow up/down
	//{{0b00000,0b00000,0b00000,0b00000,0b00011,0b00111,0b01101,0b01010}}, //dino back
	{{0b00000,0b00000,0b00000,0b00000,0b00001,0b00011,0b00110,0b00101}}, //dino back
	{{0b01111,0b01011,0b01111,0b01100,0b11010,0b11100,0b10000,0b11000}}, //dino front
	{{0b00100,0b00100,0b00101,0b10101,0b10111,0b11100,0b00100,0b00100}} // cactus
};

/*--------------------------------------------------------------------------*/
/*  Global variables                                                        */
/*--------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* export global routines (interface)                                      */
/*-------------------------------------------------------------------------*/
extern void LcdChar(char);
extern void LcdBackLight(u_char);
extern void LcdInit(void);
extern void LcdLowLevelInit(void);

void LcdWriteLine(char*);
void LcdClear(void);
void LcdMoveCursor(u_char);
void LcdAddCustomPixel(CustomIcon_t, int);
void LcdDrawCustomIcon(u_char);
void LcdWriteInfoOnScreen(char*, int);
void LcdClearLine(int);
void lcdAddText(char*, int);
void lcdChangeLastText(char*, int);
void lcdListNextMessage(void);
void lcdUpdateInfo(void);
void lcdForceDisplayText(char*, int);

#endif /* _Display_H */






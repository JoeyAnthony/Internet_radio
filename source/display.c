/* ========================================================================
 * [PROJECT]    SIR100
 * [MODULE]     Display
 * [TITLE]      display source file
 * [FILE]       display.c
 * [VSN]        1.0
 * [CREATED]    26092003
 * [LASTCHNGD]  06102006
 * [COPYRIGHT]  Copyright (C) STREAMIT BV
 * [PURPOSE]    contains all interface- and low-level routines to
 *              control the LCD and write characters or strings (menu-items)
 * ======================================================================== */

#define LOG_MODULE  LOG_DISPLAY_MODULE

#define TICK_SPEED 2
#define INDEX_DEFAULT (-6 * TICK_SPEED) -1
#define INDEX_FORCED_TEXT (-12 * TICK_SPEED) -1

#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/timer.h>
#include <sys/event.h>
#include <sys/thread.h>
#include <sys/heap.h>

#include "system.h"
#include "portio.h"
#include "display.h"
#include "log.h"
#include "llist.h"

/*-------------------------------------------------------------------------*/
/* local defines                                                           */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* local variable definitions                                              */
/*-------------------------------------------------------------------------*/
TextNode* textList = NULL;

char *currentText;
int currentSize;

int index;
int maxIndex = 0;

/*-------------------------------------------------------------------------*/
/* local routines (prototyping)                                            */
/*-------------------------------------------------------------------------*/
static void LcdWriteByte(u_char, u_char);
static void LcdWriteNibble(u_char, u_char);
static void LcdWaitBusy(void);
static void lcdInitTextList(void);


/*!
 * \addtogroup Display
 */

/*@{*/

/*-------------------------------------------------------------------------*/
/*                         start of code                                   */
/*-------------------------------------------------------------------------*/

/* ����������������������������������������������������������������������� */
/*!
 * \brief control backlight
 */
/* ����������������������������������������������������������������������� */
void LcdBackLight(u_char Mode)
{
    if (Mode==LCD_BACKLIGHT_ON)
    {
        sbi(LCD_BL_PORT, LCD_BL_BIT);   // Turn on backlight
    }

    if (Mode==LCD_BACKLIGHT_OFF)
    {
        cbi(LCD_BL_PORT, LCD_BL_BIT);   // Turn off backlight
    }
}

/*
* Writes an array of characters to the screen
*/
void LcdWriteLine(char* text)
{
	int index = 0;
	while(text[index] != '\0')
	{
		LcdChar(text[index]);
		index++;
	}
}

/*
*	Clears the screen
*/
void LcdClear()
{
	LcdWriteByte(WRITE_COMMAND, 0x01);          // display clear
    NutDelay(5);
}

/*
*	Moves cursor to position specified
*/
void LcdMoveCursor(u_char cursorPos)
{
	LcdWriteByte(WRITE_COMMAND, (cursorPos | 0x80));          // DD-RAM address counter (cursor pos) to '0'
}

/*
*	Adds your handmade icon to the register
*/
void LcdAddCustomPixel(CustomIcon_t icon, int location)
{
	u_char saveLocation = 0x0;
	int i;
	for(i = 0; i < location; i++)
	{
		saveLocation += 0x08;
	}
	
	LcdWriteByte(WRITE_COMMAND, 0x40 + saveLocation); // adres 0 van CG-RAM
	
	int cnt;
	for (cnt = 0; cnt < 8; cnt++)
	{
		LcdWriteByte(WRITE_DATA, icon.pixelData[cnt]); // schrijf data in CG-RAM
	}
}

/*
*	Draw your custom icon
*   u_char symbol : give 0,1,2,... for the specified symbol you want
*/
void LcdDrawCustomIcon(u_char symbol)
{
	LcdWriteByte(WRITE_DATA, symbol);
}

/* ����������������������������������������������������������������������� */
/*!
 * \brief Write a single character on the LCD
 *
 * Writes a single character on the LCD on the current cursor position
 *
 * \param LcdChar character to write
 */
/* ����������������������������������������������������������������������� */
void LcdChar(char MyChar)
{
    LcdWriteByte(WRITE_DATA, MyChar);
}

/* ����������������������������������������������������������������������� */
/*!
 * \brief Low-level initialisation function of the LCD-controller
 *
 * Initialise the controller and send the User-Defined Characters to CG-RAM
 * settings: 4-bit interface, cursor invisible and NOT blinking
 *           1 line dislay, 10 dots high characters
 *
 */
/* ����������������������������������������������������������������������� */
 void LcdLowLevelInit()
 {
    u_char i;
    NutDelay(140);                               // wait for more than 140 ms after Vdd rises to 2.7 V

    for (i=0; i<3; ++i)
    {
        LcdWriteNibble(WRITE_COMMAND, 0x33);      // function set: 8-bit mode; necessary to guarantee that
        NutDelay(4);                              // SIR starts up always in 5x10 dot mode
    }

    LcdWriteNibble(WRITE_COMMAND, 0x22);        // function set: 4-bit mode; necessary because KS0070 doesn't
    NutDelay(1);                                // accept combined 4-bit mode & 5x10 dot mode programming

    //LcdWriteByte(WRITE_COMMAND, 0x24);        // function set: 4-bit mode, 5x10 dot mode, 1-line
    LcdWriteByte(WRITE_COMMAND, 0x28);          // function set: 4-bit mode, 5x7 dot mode, 2-lines
    NutDelay(5);

    LcdWriteByte(WRITE_COMMAND, 0x0C);          // display ON/OFF: display ON, cursor OFF, blink OFF
    NutDelay(5);

    LcdWriteByte(WRITE_COMMAND, 0x01);          // display clear
    NutDelay(5);

    LcdWriteByte(WRITE_COMMAND, 0x06);          // entry mode set: increment mode, entire shift OFF


    LcdWriteByte(WRITE_COMMAND, 0x80);          // DD-RAM address counter (cursor pos) to '0'
	NutDelay(5);
	int cnt;
	for(cnt = 0; cnt < CUSTOMICON_ARRAY_SIZE; cnt++)
	{
		LcdAddCustomPixel(CustomIconsData[cnt], cnt);
		NutDelay(10);
	}

	lcdInitTextList();
}


/* ����������������������������������������������������������������������� */
/*!
 * \brief Low-level routine to write a byte to LCD-controller
 *
 * Writes one byte to the LCD-controller (by  calling LcdWriteNibble twice)
 * CtrlState determines if the byte is written to the instruction register
 * or to the data register.
 *
 * \param CtrlState destination: instruction or data
 * \param LcdByte byte to write
 *
 */
/* ����������������������������������������������������������������������� */
static void LcdWriteByte(u_char CtrlState, u_char LcdByte)
{
    LcdWaitBusy();                      // see if the controller is ready to receive next byte
    LcdWriteNibble(CtrlState, LcdByte & 0xF0);
    LcdWriteNibble(CtrlState, LcdByte << 4);
}

/* ����������������������������������������������������������������������� */
/*!
 * \brief Low-level routine to write a nibble to LCD-controller
 *
 * Writes a nibble to the LCD-controller (interface is a 4-bit databus, so
 * only 4 databits can be send at once).
 * The nibble to write is in the upper 4 bits of LcdNibble
 *
 * \param CtrlState destination: instruction or data
 * \param LcdNibble nibble to write (upper 4 bits in this byte
 *
 */
/* ����������������������������������������������������������������������� */
static void LcdWriteNibble(u_char CtrlState, u_char LcdNibble)
{
    outp((inp(LCD_DATA_DDR) & 0x0F) | 0xF0, LCD_DATA_DDR);  // set data-port to output again

    outp((inp(LCD_DATA_PORT) & 0x0F) | (LcdNibble & 0xF0), LCD_DATA_PORT); // prepare databus with nibble to write

    if (CtrlState == WRITE_COMMAND)
    {
        cbi(LCD_RS_PORT, LCD_RS);     // command: RS low
    }
    else
    {
        sbi(LCD_RS_PORT, LCD_RS);     // data: RS high
    }

    sbi(LCD_EN_PORT, LCD_EN);

    asm("nop\n\tnop");                    // small delay

    cbi(LCD_EN_PORT, LCD_EN);
    cbi(LCD_RS_PORT, LCD_RS);
    outp((inp(LCD_DATA_DDR) & 0x0F), LCD_DATA_DDR);           // set upper 4-bits of data-port to input
    outp((inp(LCD_DATA_PORT) & 0x0F) | 0xF0, LCD_DATA_PORT);  // enable pull-ups in data-port
}

/* ����������������������������������������������������������������������� */
/*!
 * \brief Low-level routine to see if the controller is ready to receive
 *
 * This routine repeatetly reads the databus and checks if the highest bit (bit 7)
 * has become '0'. If a '0' is detected on bit 7 the function returns.
 *
 */
/* ����������������������������������������������������������������������� */
static void LcdWaitBusy()
{
    u_char Busy = 1;
	u_char LcdStatus = 0;

    cbi (LCD_RS_PORT, LCD_RS);              // select instruction register

    sbi (LCD_RW_PORT, LCD_RW);              // we are going to read

    while (Busy)
    {
        sbi (LCD_EN_PORT, LCD_EN);          // set 'enable' to catch 'Ready'

        asm("nop\n\tnop");                  // small delay
        LcdStatus =  inp(LCD_IN_PORT);      // LcdStatus is used elsewhere in this module as well
        Busy = LcdStatus & 0x80;            // break out of while-loop cause we are ready (b7='0')
    }

    cbi (LCD_EN_PORT, LCD_EN);              // all ctrlpins low
    cbi (LCD_RS_PORT, LCD_RS);
    cbi (LCD_RW_PORT, LCD_RW);              // we are going to write
}

/**
*   Initialize the message/text list
*/
void lcdInitTextList(void) {
    index = INDEX_DEFAULT;
	textList =  malloc(sizeof(TextNode));
    textList->next = NULL;
    textList->data = NULL;
	textList->size = NULL;
}
/**
*   Add a message/text tot the list
*/
void lcdAddText(char *text, int size) {
	if(textList != NULL)
		ListInsertText(&textList, text, size);
	else{
		textList->next = NULL;
		textList->data = text;
		textList->size = size;
		
		lcdListNextMessage();
		return;
	}
}

void lcdChangeLastText(char *text, int size){
	if(textList !=NULL)
		ListChangeLastText(textList, text,size);
	else
		lcdAddText(text,size);
}

/**
*   update the text wich is moving on the second line of the display
*/
void lcdUpdateInfo(void) {
	if(currentText == NULL){
		lcdListNextMessage();
	}
    if(index % TICK_SPEED == 0){
		if(index / TICK_SPEED < -6){
			
		}else if(index <0){
			LcdWriteInfoOnScreen(currentText, 0);
		}else if(index / TICK_SPEED == (maxIndex + 6)){
			//reset the index so that next time the char will go the next one
			lcdListNextMessage();
		}else if(index / TICK_SPEED >= (maxIndex)){
			LcdWriteInfoOnScreen(currentText, maxIndex);
		}else {
			LcdWriteInfoOnScreen(currentText, index / TICK_SPEED);    
		}
    }
    
    index++;
}

/**
*   Force the display to start writting the next message in the list.
*/
void lcdForceNextInfo(void) {
    index = INDEX_DEFAULT;
    lcdListNextMessage();
}


void lcdForceDisplayText(char *text, int size) {
    LcdClearLine(1);
    NutDelay(5);
    LcdWriteInfoOnScreen(text, 0);
    currentText = text;
    currentSize = size;
    maxIndex = size - 16  + 2 ;
    index = INDEX_DEFAULT;
}

/**
*  write text on the second line of the display  
*
*/
void LcdWriteInfoOnScreen(char *text, int index){
	char substr[17];
	strncpy(&substr,text + index , 16);
	substr[16] = '\0';
	LcdMoveCursor(0x40);
	LcdWriteLine(substr);
}

/**
*   Clear one of the two lines on the display
*   input: 0, 1;
*/
void LcdClearLine(int line){
	if(line == 0)
		LcdMoveCursor(0x00);
	else if(line == 1)
		LcdMoveCursor(0x40);
		
	LcdWriteLine("                "); // 16 chars			
}


/**
* move to the next element in the list by moving the head;
* 
*/
void lcdListNextMessage() {
	LcdClearLine(1);
	index = INDEX_DEFAULT;
	currentText = textList->data;
	currentSize = textList->size;
    maxIndex = currentSize - 16;
	
	
	if(textList->next->data != NULL){
        //free(textList);
		textList = textList->next;	
	}
}




/* ---------- end of module ------------------------------------------------ */

/*@}*/

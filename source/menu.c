

#define MAX_MENU_KEY 8			// Amounth of other menu's
#define LCD_MAX_LINES 11


/* Define indices for the keys in the array of new IDs */
#define MENU_KEY_ESC 0
#define MENU_KEY_OK 1
#define MENU_KEY_UP 2
#define MENU_KEY_DOWN 3
#define MENU_KEY_LEFT 4
#define MENU_KEY_RIGHT 5

// All menu IDs
#define MENU_NO_MENU -1
#define MENU_MAIN_0_ID 0
#define MENU_TIME_SETTING_1_ID 1
#define MENU_TIMEZONE_SETTING_2_ID 2
#define MENU_ALARM_ADD_3_ID 3
#define MENU_All_ALARMS_4_ID 4
#define MENU_ALARM_EDIT_5_ID 5
#define MENU_SET_TIMER_6_ID 6
#define MAIN_MENU_ITEMS 5				// Amount of items in main menu starting from 1
#define ADD_ALARM_ITEMS 10

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "rtc.h"
#include "alarmcontroller.h"
#include "system.h"
#include "display.h"
#include "keyboard.h"
#include "menu.h"
#include "clock.h"


/* STRUCTS  */
typedef struct {
	unsigned int id;						// ID of current item
	unsigned int newId[MAX_MENU_KEY];		// ID's to jump to when certain buttons are pressed
	char *text[LCD_MAX_LINES];				// Text of current item
	void (*fpOnKey[MAX_MENU_KEY])(void);	/* Function pointer for each key */
	void (*fpOnEntry)(void);				/* Function called on entry */
	void (*fpOnExit)(void);					/* Function called on exit */
} MENU_ITEM_STRUCT;
/* STRUCTS  */
/*local prototypes*/

void exitItem(void);
void enterItem(void);
void timeChangeButtonLeft(void);
void timeChangeButtonRight(void);
void timeChangeButtonUp(void);
void timeChangeButtonDown(void);
void timeChangeButtonOK(void);
void timeZoneChangeButtonUp(void);
void timeZoneChangeButtonDown(void);
void timeZoneChangeButtonOK(void);
void mainMenuUp(void);
void mainMenuDown(void);
void mainMenuOk(void);
void subMenuEsc(void);
void mainMenuEsc(void);
void AllAlarmButtonUp(void);
void AllAlarmButtonDown(void);
void AllAlarmButtonOk(void);
void editAlarmButtonOk(void);
void addAlarmButtonUp(void);
void addAlarmButtonDown(void);
void addAlarmButtonLeft(void);
void addAlarmButtonRight(void);
void addAlarmButtonOk(void);

void addTimerButtonUp(void);
void addTimerButtonDown(void);
void addTimerButtonLeft(void);
void addTimerButtonRight(void);
void addTimerButtonOk(void);
void displayRunningTimer(void);


/*GLOBAL VARIABLES*/
int menuFinished = 0;
uint32_t inputDelay = 1;
//uint32_t inputStart = NutGetSeconds();

static unsigned int currentMenuIndex = 1;
static unsigned int currentItem = 0;
static unsigned int currentId;
static struct _tm gmt;
static unsigned int subMenuIndex = 0;
static int tempTimeZone = 0;

static unsigned int listSize = 0; // size of the alarmlist

static unsigned int AddAlarmMenuSelected = 0;

//variables for the temp alarm
tm *tempAlarmTime;
static unsigned int snoozeTime = 0;
static unsigned int alarmEnabled = 1;
static unsigned int alarmGame = 0;
static unsigned int alarmSchedular = 0;
static unsigned int alarmTone = 0;
static unsigned int alarmIpStream = 0;
static unsigned int alarmDays = 0b0000000;


Alarm* tempAlarm;

static unsigned int currentAlarmIndex;

Alarm* alarmForPrinting;

char timeToWrite[10];

//set timer variables
int timerMinute, timerSecond; //lenth of timer
int timerStartSec = -1; //timestamp of the begin of the timer. It is -1 if the timer is not running
int timerStream = 0; //end the stream when timer finishes

//variables for display purposes
int currentSeconds;
int secondsLeft;
int minutesLeft;

/* Creating all menu items */
MENU_ITEM_STRUCT menu[] ={
	{
	/* Main menu item */
	MENU_MAIN_0_ID,
	{	/* New item id (Esc, Ok, Up, Down, Left, Right) Where to jump on keypress*/
	
		MENU_NO_MENU,				//Doet niks
		MENU_NO_MENU,
		MENU_NO_MENU,
		MENU_NO_MENU,
		MENU_NO_MENU,				//Doet niks
		MENU_NO_MENU				//Doet niks
	},
	{
		/* Menu item text
				 01234567890123456789 */
				"1 Time & Date",
				"2 Timezone",
				"3 Add Alarms",
				"4 All Alarms",
				"5 Set Timer"
	},
	{
		mainMenuEsc,mainMenuOk,mainMenuUp,mainMenuDown, NULL,NULL
	},
	/*On entering menuitem*/
	NULL,
	/*On exiting menuitem*/
	exitItem	
	},
	{
		/* Main menu item */
	MENU_TIME_SETTING_1_ID,
	{	/* New item id (Esc, Ok, Up, Down, Left, Right) Where to jump on keypres*/
	
		MENU_MAIN_0_ID,				//Doet niks
		MENU_NO_MENU,
		MENU_NO_MENU,
		MENU_NO_MENU,
		MENU_NO_MENU,				//Doet niks
		MENU_NO_MENU				//Doet niks
	},
	{
		/* Menu item text
				 01234567890123456789 */
				//"Tijd menu",
				//"set time",
				"Time",
				"Date"
	},
	{
		subMenuEsc,timeChangeButtonOK,timeChangeButtonUp,timeChangeButtonDown, timeChangeButtonLeft,timeChangeButtonRight
	},
	/*On entering menuitem*/
	NULL,
	/*On exiting menuitem*/
	exitItem	
	},	
	{
		/* Main menu item */
	MENU_TIMEZONE_SETTING_2_ID,
	{	/* New item id (Esc, Ok, Up, Down, Left, Right) Where to jump on keypres*/
	
		MENU_MAIN_0_ID,				//Doet niks
		MENU_NO_MENU,
		MENU_NO_MENU,
		MENU_NO_MENU,
		MENU_NO_MENU,				//Doet niks
		MENU_NO_MENU				//Doet niks
	},
	{
		/* Menu item text
				 01234567890123456789 */
				//"Timezone menu",
				//"set timezone",
				"Timezone",
				"GMT : UTC"
	},
	{
		subMenuEsc,timeZoneChangeButtonOK,timeZoneChangeButtonUp,timeZoneChangeButtonDown, NULL,NULL
	},
	/*On entering menuitem*/
	NULL,
	/*On exiting menuitem*/
	exitItem	
	},
	{
		/* Main menu item */
	MENU_ALARM_ADD_3_ID,
	{	/* New item id (Esc, Ok, Up, Down, Left, Right) Where to jump on keypres*/
	
		MENU_MAIN_0_ID,				//Doet niks
		MENU_NO_MENU,
		MENU_NO_MENU,
		MENU_NO_MENU,
		MENU_NO_MENU,				//Doet niks
		MENU_NO_MENU				//Doet niks
	},
	{
		/* Menu item text
				 01234567890123456789 */
				//"Timezone menu",
				//"set timezone",
				"Time:",
				"Snooze:",
				"Enabled:",
				"Game:",
				"Schedular:",
				"Days:",
				"Stream:",
				"Tone:",
				"Save",
				"Cancel",
				"Delete",
	},
	{
		subMenuEsc,addAlarmButtonOk,addAlarmButtonUp,addAlarmButtonDown, addAlarmButtonLeft,addAlarmButtonRight
	},
	/*On entering menuitem*/
	NULL,
	/*On exiting menuitem*/
	exitItem	
	},
	{
		/* Main menu item */
		MENU_All_ALARMS_4_ID,
	
	{	/* New item id (Esc, Ok, Up, Down, Left, Right) Where to jump on keypres*/
	
		MENU_MAIN_0_ID,				//Doet niks
		MENU_NO_MENU,
		MENU_NO_MENU,
		MENU_NO_MENU,
		MENU_NO_MENU,				//Doet niks
		MENU_NO_MENU				//Doet niks
	},
	{
		/* Menu item text
				 01234567890123456789 */
				//"Timezone menu",
				//"set timezone",
				
				"Back"
	},
	{
		subMenuEsc,AllAlarmButtonOk,AllAlarmButtonUp,AllAlarmButtonDown, NULL,NULL
	},
	/*On entering menuitem*/
	NULL,
	/*On exiting menuitem*/
	exitItem	
	},
	{
		/* Main menu item */
		MENU_ALARM_EDIT_5_ID,
	
	{	/* New item id (Esc, Ok, Up, Down, Left, Right) Where to jump on keypres*/
	
		MENU_All_ALARMS_4_ID,				//Doet niks
		MENU_NO_MENU,
		MENU_NO_MENU,
		MENU_NO_MENU,
		MENU_NO_MENU,				//Doet niks
		MENU_NO_MENU				//Doet niks
	},
	{
		/* Menu item text
				 01234567890123456789 */
				//"Timezone menu",
				//"set timezone",
				"Time:",
				"Snooze:",
				"Enabled:",
				"Game:",
				"Schedular:",
				"Days:",
				"Stream:",
				"Tone:",
				"Save",
				"Cancel"
	},
	{
		subMenuEsc,editAlarmButtonOk,addAlarmButtonUp,addAlarmButtonDown, addAlarmButtonLeft,addAlarmButtonRight
	},
	/*On entering menuitem*/
	NULL,
	/*On exiting menuitem*/
	exitItem	
	},
	{
	/* Main menu item */
	MENU_SET_TIMER_6_ID,
	{	/* New item id (Esc, Ok, Up, Down, Left, Right) Where to jump on keypress*/
	
		MENU_MAIN_0_ID,				//Doet niks
		MENU_NO_MENU,
		MENU_NO_MENU,
		MENU_NO_MENU,
		MENU_NO_MENU,				//Doet niks
		MENU_NO_MENU				//Doet niks
	},
	{
		/* Menu item text
				 01234567890123456789 */
				"Timer",
				"",
	},
	{
		mainMenuEsc,addTimerButtonOk ,addTimerButtonUp,addTimerButtonDown, addTimerButtonLeft,addTimerButtonRight
	},
	/*On entering menuitem*/
	NULL,
	/*On exiting menuitem*/
	exitItem	
	}
};


void handleMenu(u_char key)
{
	if(key != KEY_UNDEFINED)
		LcdBackLight(LCD_BACKLIGHT_ON);
	switch (key) {
	/* Esc */
	case KEY_ESC:
		if (NULL != menu[currentItem].fpOnKey[MENU_KEY_ESC]) {
			(*menu[currentItem].fpOnKey[MENU_KEY_ESC])();
		}
		if (MENU_NO_MENU != menu[currentItem].newId[MENU_KEY_ESC]){
			menu[currentItem].fpOnExit();
			currentId = menu[currentItem].newId[MENU_KEY_ESC];
			currentItem = currentId;
			
		}
		break;
	/* Ok */
	case KEY_OK:
		if (NULL != menu[currentItem].fpOnKey[MENU_KEY_OK]) {
			(*menu[currentItem].fpOnKey[MENU_KEY_OK])();
		}
		if(MENU_NO_MENU != menu[currentItem].newId[MENU_KEY_OK]){
			menu[currentItem].fpOnExit();
			currentId = menu[currentItem].newId[MENU_KEY_OK];
			currentItem = currentId;
			
		}
		break;
	/* Up */
	case KEY_UP:
		if (NULL != menu[currentItem].fpOnKey[MENU_KEY_UP]) {
			(*menu[currentItem].fpOnKey[MENU_KEY_UP])();
		}
		if(MENU_NO_MENU != menu[currentItem].newId[MENU_KEY_UP]){
			menu[currentItem].fpOnExit();
			currentId = menu[currentItem].newId[MENU_KEY_UP];
			currentItem = currentId;
			
		}
		break;
	/* Down */
	case KEY_DOWN:
		if (NULL != menu[currentItem].fpOnKey[MENU_KEY_DOWN]) {
			(*menu[currentItem].fpOnKey[MENU_KEY_DOWN])();
		}
		if(MENU_NO_MENU != menu[currentItem].newId[MENU_KEY_DOWN]){
			menu[currentItem].fpOnExit();
			currentId = menu[currentItem].newId[MENU_KEY_DOWN];
			currentItem = currentId;
			
		}
		break;
	/* Left */
	case KEY_LEFT:
		if (NULL != menu[currentItem].fpOnKey[MENU_KEY_LEFT]) {
			(*menu[currentItem].fpOnKey[MENU_KEY_LEFT])();
		}
		if(MENU_NO_MENU != menu[currentItem].newId[MENU_KEY_LEFT]){
			menu[currentItem].fpOnExit();
			currentId = menu[currentItem].newId[MENU_KEY_LEFT];
			currentItem = currentId;
			
		}
		break;
		/* Left */
	case KEY_RIGHT:
		if (NULL != menu[currentItem].fpOnKey[MENU_KEY_RIGHT]) {
			(*menu[currentItem].fpOnKey[MENU_KEY_RIGHT])();
		}
		if(MENU_NO_MENU != menu[currentItem].newId[MENU_KEY_RIGHT]){
			menu[currentItem].fpOnExit();
			currentId = menu[currentItem].newId[MENU_KEY_RIGHT];
			currentItem = currentId;
			
		}
		break;
	default:
		break;
	}
}

void handleMenuDisplay(void){


	switch(currentId){
		case MENU_MAIN_0_ID:
				if(currentMenuIndex == 1) {
					LcdMoveCursor(0x40);
					LcdWriteLine(" ");
					LcdMoveCursor(0x00);
				}else if(currentMenuIndex == 2) {
					LcdMoveCursor(0x00);
					LcdWriteLine(" ");
					LcdMoveCursor(0x40);
				}else if(currentMenuIndex == 3) {
					LcdMoveCursor(0x40);
					LcdWriteLine(" ");
					LcdMoveCursor(0x00);
				}else if(currentMenuIndex == 4) {
					LcdMoveCursor(0x00);
					LcdWriteLine(" ");
					LcdMoveCursor(0x40);
				}else if(currentMenuIndex == 5){
					LcdMoveCursor(0x40);
					LcdWriteLine(" ");
					LcdMoveCursor(0x00);
				}
				
				LcdWriteLine(">");
				
				if(currentMenuIndex == 1 || currentMenuIndex == 2){
					LcdMoveCursor(0x02);
					LcdWriteLine(menu[currentItem].text[0]);
					LcdMoveCursor(0x42);
					LcdWriteLine(menu[currentItem].text[1]);
				}else if(currentMenuIndex == 3 || currentMenuIndex == 4){
					LcdMoveCursor(0x02);
					LcdWriteLine(menu[currentItem].text[2]);
					LcdMoveCursor(0x42);
					LcdWriteLine(menu[currentItem].text[3]);
				}else if(currentMenuIndex == 5){
					LcdMoveCursor(0x02);
					LcdWriteLine(menu[currentItem].text[4]);
				}
		break;
		case MENU_TIME_SETTING_1_ID:
				LcdMoveCursor(0x01);
				LcdWriteLine(menu[currentItem].text[0]);
				LcdMoveCursor(0x0A);
				LcdWriteLine(menu[currentItem].text[1]);
				LcdMoveCursor(0x40);
				if(subMenuIndex == 0)  {
					LcdDrawCustomIcon(3);
				}
				else if(subMenuIndex != 0) {
					LcdWriteLine(" ");
				}
				sprintf(timeToWrite, "%02d:%02d", gmt.tm_hour, gmt.tm_min); // if gmt.tm_hour < 10 it will still display the 0;
				LcdWriteLine(timeToWrite);
				if(subMenuIndex == 1) {
					LcdDrawCustomIcon(3);
				}
				else if(subMenuIndex != 1) {
					LcdWriteLine(" ");
				}

				LcdMoveCursor(0x49);
				if(subMenuIndex == 2) {
					LcdDrawCustomIcon(3);
				}
				else if(subMenuIndex != 2) {
					LcdWriteLine(" ");
				}
				char dateToWrite[10];	
				sprintf(dateToWrite, "%02d/%02d", gmt.tm_mday, gmt.tm_mon); 
				LcdWriteLine(dateToWrite);
				if(subMenuIndex == 3) {
					LcdDrawCustomIcon(3);
				}
				else if(subMenuIndex != 3) {
					LcdWriteLine(" ");
				}




		break;
		case MENU_TIMEZONE_SETTING_2_ID:
				LcdMoveCursor(0x04);
				LcdWriteLine(menu[currentItem].text[0]);
				LcdMoveCursor(0x41);
				char timezoneToWrite[10];
				if(tempTimeZone == 0)
					sprintf(timezoneToWrite, "%s  %d", menu[currentItem].text[1], tempTimeZone);
				else if(tempTimeZone > 0)
					sprintf(timezoneToWrite, "%s +%d", menu[currentItem].text[1], tempTimeZone);
				else
					sprintf(timezoneToWrite, "%s %d", menu[currentItem].text[1], tempTimeZone);
				LcdWriteLine(timezoneToWrite);
				LcdDrawCustomIcon(3);
				LcdWriteLine("  ");
		break;
		case MENU_ALARM_EDIT_5_ID:
		case MENU_ALARM_ADD_3_ID:
		
				if(currentMenuIndex == 1 || currentMenuIndex == 3 || currentMenuIndex == 5 || currentMenuIndex == 7 || currentMenuIndex == 9 || currentMenuIndex == 11) {
					LcdMoveCursor(0x40);
					LcdWriteLine(" ");
					LcdMoveCursor(0x00);
				}else if(currentMenuIndex == 2 || currentMenuIndex == 4 || currentMenuIndex == 6 || currentMenuIndex == 8 || currentMenuIndex == 10) {
					LcdMoveCursor(0x00);
					LcdWriteLine(" ");
					LcdMoveCursor(0x40);
				}
				
				LcdWriteLine(">");
				
				if(currentMenuIndex == 1 || currentMenuIndex == 2){
					LcdMoveCursor(0x02);
					LcdWriteLine(menu[currentItem].text[0]);
					//time
					LcdMoveCursor(0x08);
					if(AddAlarmMenuSelected == 1 && currentMenuIndex == 1){
						if(subMenuIndex == 0) {
							LcdDrawCustomIcon(3);
						}
						else if(subMenuIndex != 0) {
							LcdWriteLine(" ");
						}
					}else{
						LcdWriteLine(" ");
					}
					LcdMoveCursor(0x09);	
					sprintf(timeToWrite, "%02d:%02d", tempAlarmTime->tm_hour, tempAlarmTime->tm_min); // if gmt.tm_hour < 10 it will still display the 0;
					LcdWriteLine(timeToWrite);
					//LcdMoveCursor(0x15);
					if(AddAlarmMenuSelected == 1 && currentMenuIndex == 1){
						if(subMenuIndex == 1) {
							LcdDrawCustomIcon(3);
						}
						else if(subMenuIndex != 1) {
							LcdWriteLine(" ");
						}
					}else
					LcdWriteLine(" ");
					LcdMoveCursor(0x42);
					LcdWriteLine(menu[currentItem].text[1]);	
					//snooze time
					LcdMoveCursor(0x49);
					if(AddAlarmMenuSelected == 1 && currentMenuIndex == 2){
						LcdDrawCustomIcon(3);
					}else{
						LcdWriteLine(" ");
					}
					if(!snoozeTime)
						LcdWriteLine("None");
					else{
						sprintf(timeToWrite, "%02d min",snoozeTime);
						LcdWriteLine(timeToWrite);
					}
				}else if(currentMenuIndex == 3 || currentMenuIndex == 4){
					LcdMoveCursor(0x02);
					LcdWriteLine(menu[currentItem].text[2]);
					LcdMoveCursor(0x0B);
					if(AddAlarmMenuSelected == 1 && currentMenuIndex == 3){
						LcdDrawCustomIcon(3);
					}else{
						LcdWriteLine(" ");
					}
					if(alarmEnabled)
						LcdWriteLine("Yes");
					else
						LcdWriteLine("No");
					LcdMoveCursor(0x42);
					LcdWriteLine(menu[currentItem].text[3]);
					LcdMoveCursor(0x48);
					if(AddAlarmMenuSelected == 1 && currentMenuIndex == 4){
						LcdDrawCustomIcon(3);
					}else{
						LcdWriteLine(" ");
					}
					if(!alarmGame)
						LcdWriteLine("No game");
					else if(alarmGame == 1)
						LcdWriteLine("Math");
					else if(alarmGame == 2)
						LcdWriteLine("Dino");
				}else if(currentMenuIndex == 5 || currentMenuIndex == 6){
					LcdMoveCursor(0x02);
					LcdWriteLine(menu[currentItem].text[4]);
					LcdMoveCursor(0x0C);
					if(AddAlarmMenuSelected == 1 && currentMenuIndex == 5){
						LcdDrawCustomIcon(3);
					}else{
						LcdWriteLine(" ");
					}
					if(alarmSchedular)
						LcdWriteLine("Yes");
					else
						LcdWriteLine("No");
					
					LcdMoveCursor(0x42);
					LcdWriteLine(menu[currentItem].text[5]);
					//days
					LcdMoveCursor(0x48);
					switch (subMenuIndex){
						case 0 :
							LcdWriteLine("SUN");break;
						case 1 :
							LcdWriteLine("MON");break;
						case 2 :
							LcdWriteLine("TUE");break;
						case 3 :
							LcdWriteLine("WED");break;
						case 4 :
							LcdWriteLine("THU");break;
						case 5 :
							LcdWriteLine("FRI");break;
						case 6 :
							LcdWriteLine("SAT");break;
					}
					LcdMoveCursor(0x4C);
					if(AddAlarmMenuSelected == 1 && currentMenuIndex == 6){
						LcdDrawCustomIcon(3);
					}else{
						LcdWriteLine(" ");
					}
					if(alarmDays & 1 << subMenuIndex)
						LcdWriteLine("Yes");
					else 
						LcdWriteLine("No");
				}else if(currentMenuIndex == 7 || currentMenuIndex == 8){
					LcdMoveCursor(0x02);
					LcdWriteLine(menu[currentItem].text[6]);
					if(AddAlarmMenuSelected == 1 && currentMenuIndex == 7){
						LcdDrawCustomIcon(3);
					}else{
						LcdWriteLine(" ");
					}
					switch(alarmIpStream){
						case 0 : LcdWriteLine("Rad 1");break;
						case 1 : LcdWriteLine("Rad 2");break;
						case 2 : LcdWriteLine("3FM");break;
						case 3 : LcdWriteLine("Veron");break;
						case 4 : LcdWriteLine("R 538");break;
					}
					LcdMoveCursor(0x42);
					LcdWriteLine(menu[currentItem].text[7]);
					LcdMoveCursor(0x49);
					if(AddAlarmMenuSelected == 1 && currentMenuIndex == 8){
						LcdDrawCustomIcon(3);
					}else{
						LcdWriteLine(" ");
					}
					if(alarmTone)
						LcdWriteLine("Yes");
					else
						LcdWriteLine("No");
					
				}else if(currentMenuIndex == 9 || currentMenuIndex == 10){
					LcdMoveCursor(0x02);
					LcdWriteLine(menu[currentItem].text[8]);
					LcdMoveCursor(0x42);
					LcdWriteLine(menu[currentItem].text[9]);
				}else if(currentMenuIndex == 11){
					LcdMoveCursor(0x02);
					LcdWriteLine("Delete");
				}
				
				
		break;
		case MENU_All_ALARMS_4_ID :
				if(currentMenuIndex < listSize){
					LcdMoveCursor(0x40);
					LcdWriteLine(" ");
					LcdMoveCursor(0x00);
				
					LcdWriteLine(">");
					LcdMoveCursor(0x02);
					
					if(GetAlarmByIndex(currentMenuIndex - 1,&alarmForPrinting)){
						sprintf(timeToWrite, "%d %02d:%02d", currentMenuIndex,alarmForPrinting->time->tm_hour, alarmForPrinting->time->tm_min); 
					}else{
						sprintf(timeToWrite, "faulty shit");
					}
					LcdWriteLine(timeToWrite);
					
					LcdMoveCursor(0x42);
					
					if(GetAlarmByIndex(currentMenuIndex,&alarmForPrinting)){
						sprintf(timeToWrite, "%d %02d:%02d", currentMenuIndex + 1,alarmForPrinting->time->tm_hour, alarmForPrinting->time->tm_min); 
					}
					LcdWriteLine(timeToWrite);
				}else if(currentMenuIndex == listSize){
					LcdMoveCursor(0x40);
					LcdWriteLine(" ");
					LcdMoveCursor(0x00);
				
					LcdWriteLine(">");
					LcdMoveCursor(0x02);
					if(GetAlarmByIndex(currentMenuIndex - 1 ,&alarmForPrinting)){
						sprintf(timeToWrite, "%d %02d:%02d", currentMenuIndex, alarmForPrinting->time->tm_hour, alarmForPrinting->time->tm_min); 
					}
					LcdWriteLine(timeToWrite);
					LcdMoveCursor(0x42);
					LcdWriteLine(menu[currentItem].text[0]);
				}else if(currentMenuIndex == listSize + 1){
					LcdMoveCursor(0x02);
					//LcdWriteLine(menu[currentItem].text[currentMenuIndex - 2]);
					if(listSize == 0){
						sprintf(timeToWrite,"No alarms");
					}else if(GetAlarmByIndex(currentMenuIndex -2 ,&alarmForPrinting)){
						sprintf(timeToWrite, "%d %02d:%02d",currentMenuIndex - 1, alarmForPrinting->time->tm_hour, alarmForPrinting->time->tm_min); 
					}
					LcdWriteLine(timeToWrite);
					LcdMoveCursor(0x42);
					LcdWriteLine(menu[currentItem].text[0]);
					
					LcdMoveCursor(0x00);
					LcdWriteLine(" ");
					LcdMoveCursor(0x40);
				
					LcdWriteLine(">");
				}
			break;
		case MENU_SET_TIMER_6_ID :
			LcdMoveCursor(0x02);
			LcdWriteLine(menu[currentItem].text[0]);
			LcdMoveCursor(0x09);
			//if the timer isnt running
			if(timerStartSec == -1){
				if(subMenuIndex == 0)  {
					LcdDrawCustomIcon(3);
				}
				else if(subMenuIndex != 0) {
					LcdWriteLine(" ");
				}
				sprintf(timeToWrite, "%02d:%02d", timerMinute, timerSecond);
				LcdWriteLine(timeToWrite);
				if(subMenuIndex == 1) {
					LcdDrawCustomIcon(3);
				}
				else if(subMenuIndex != 1) {
					LcdWriteLine(" ");
				}
			}else {
				
				displayRunningTimer();
			}
			if(subMenuIndex == 2){
				LcdMoveCursor(0x40);
				LcdWriteLine("End stream  ");
				LcdDrawCustomIcon(3);
				if(timerStream)
					LcdWriteLine("Yes");
				else
					LcdWriteLine("No");
			}
		break;
		default:
		
		break;
	}
}

//function that saves the variables from the tempalarm struct pointer to individual variables
void loadAlarm(void){
	tempAlarmTime = tempAlarm->time;
	snoozeTime = tempAlarm->snoozeTime;
	alarmEnabled = tempAlarm->isEnabled;
	alarmGame = tempAlarm->hasGame;
	alarmSchedular = tempAlarm->isShedular;
    alarmTone = tempAlarm->alarmTone;
    alarmDays = tempAlarm->days;
	alarmIpStream = tempAlarm->ipStreamURL;
}

//function that saves the individual variables back to the tempalarm struct pointer
void saveAlarm(void){
	tempAlarm->time->tm_sec = 0;
	tempAlarm->time = tempAlarmTime;
	tempAlarm->snoozeTime = snoozeTime;
	tempAlarm->isEnabled = alarmEnabled;
	tempAlarm->hasGame = alarmGame;
	tempAlarm->isShedular = alarmSchedular;
	tempAlarm->alarmTone = alarmTone;
	tempAlarm->days = alarmDays;
	tempAlarm->ipStreamURL = alarmIpStream;
	
	saveAllAlarms();
}

void addTimerButtonUp(void){
	LcdClear();
	if(timerStartSec == -1){
	switch(subMenuIndex) {
		case 0 :
			if(timerMinute < 60)
				timerMinute += 1;
			else
				timerMinute = 0;
			break;
		case 1 :
			if(timerSecond < 55)
				timerSecond += 5;
			else
				timerSecond = 0;
			break;
	}
	// if the timer is running add 30 seconds to the running timer
	}else if((minutesLeft < 59 || minutesLeft < 30) && subMenuIndex < 2) {
		timerSecond +=30;
		if(timerSecond > 60){
			timerMinute++;
			timerSecond = timerSecond - 60;
		}
	}
	if(subMenuIndex == 2)
		timerStream = 1;
}

void addTimerButtonDown(void){
	LcdClear();
	// if timer is not running set the time
	if(timerStartSec == -1){
	switch(subMenuIndex) {
		case 0 :
			if(timerMinute > 0)
				timerMinute -= 1;
			else
				timerMinute = 60;
			break;
		case 1 :
			if(timerSecond > 0)
				timerSecond -= 5;
			else
				timerSecond = 55;
			break;
	}
	// if the timer is running take 30 seconds from the running timer unless the timer has less then 30 seconds left
	}else if(((minutesLeft * 60) + secondsLeft > 30)  && subMenuIndex < 2){
		timerSecond -=30;
		if(timerSecond < 0){
			timerMinute--;
			timerSecond = 60 + timerSecond;
		}
	}
	if(subMenuIndex == 2)
		timerStream = 0;
}

void addTimerButtonLeft(void){
	LcdClear();
	if(subMenuIndex > 0)
		subMenuIndex -= 1;
}

void addTimerButtonRight(void){
	LcdClear();
	if(subMenuIndex < 2)
		subMenuIndex += 1;
}

void addTimerButtonOk(void){
	LcdClear();
	// turn the timer on or off
	if(timerStartSec == -1){
		timerStartSec = NutGetSeconds();
	}else{
		if(timerMinute < 59 || timerSecond < 59){
			timerMinute = 60;
			timerSecond = 00;
		}
		timerStartSec = -1;
	}
}

//function that displays the running timer on the screen
void displayRunningTimer(void){
	currentSeconds = NutGetSeconds();
	secondsLeft = ((timerMinute * 60) + timerSecond) - (currentSeconds - timerStartSec); // seconds left of the timer
	minutesLeft = secondsLeft / 60; //take the minutes
	secondsLeft = secondsLeft % 60;	//take the seconds
	
	//display the time left
	LcdMoveCursor(0x0A);
	sprintf(timeToWrite, "%02d:%02d", minutesLeft, secondsLeft);
	LcdWriteLine(timeToWrite);

	checkTimer(currentSeconds);
}

//function that checks wheter or not the timer has finished
void checkTimer(int currentSeconds){
	if(timerStartSec != -1){
		secondsLeft = ((timerMinute * 60) + timerSecond) - (currentSeconds - timerStartSec); // seconds left of the timer
		if(secondsLeft <= 0){
			//force display text
			char text[] = "Timer is done";
			LcdMoveCursor(0x40);
			LcdWriteLine(text);
			//sound alarm
			int i;
			for(i = 0;i<5;i++){
				AlarmBeep();
			}
			//turn alarm off
			timerStartSec = -1;
			//stop stream if a stream is running
			if(timerStream)
				stopStream();
			
			subMenuIndex == 0;
		}		
	}
}

void AllAlarmButtonUp(void){
	LcdClear();
	if(currentMenuIndex != 1)
		currentMenuIndex -= 1;
}

void AllAlarmButtonDown(void){
	LcdClear();
	if(currentMenuIndex < listSize + 1)
		currentMenuIndex += 1;
	
}

void AllAlarmButtonOk(void){
	if(currentMenuIndex == listSize + 1){ // back button
		subMenuEsc();
		currentItem = MENU_MAIN_0_ID;
		currentId = menu[currentItem].id;
	}else{  
		// go to the edit screen with the selected alarm
		GetAlarmByIndex(currentMenuIndex - 1,&tempAlarm);
		currentAlarmIndex = currentMenuIndex - 1;
		loadAlarm();
		subMenuEsc();
		currentItem = MENU_ALARM_EDIT_5_ID;
		currentId = menu[currentItem].id;
		AddAlarmMenuSelected = 0;
	}
}

void addAlarmButtonUp(void){
	
	LcdClear();
	if(AddAlarmMenuSelected == 0){
		if(currentMenuIndex != 1)
			currentMenuIndex -= 1;	
	}else{
		switch(currentMenuIndex){
			case 1 :{
			if(subMenuIndex == 0){
				if(tempAlarmTime->tm_hour < 23)
					tempAlarmTime->tm_hour += 1;
				else
					tempAlarmTime->tm_hour = 0;
				break;
			}else if(subMenuIndex == 1){
				if(tempAlarmTime->tm_min < 59)
					tempAlarmTime->tm_min += 1;
				else
					tempAlarmTime->tm_min = 0;
				break;	
			}
			}
			case 2 : {
				if(snoozeTime < 60){
					snoozeTime++;
				}
				break;
			}	
			case 3 : {
				if(!alarmEnabled){
					alarmEnabled = 1;
				}
				break;
			}	
			case 4 : {
				if(alarmGame < 2){
					alarmGame++;
				}
				break;
			}
			case 5 : {
				if(!alarmSchedular){
					alarmSchedular = 1;
				}
				break;
			}
			case 6 : {
				if(!(alarmDays & 1 << subMenuIndex))
					alarmDays |= 1 << subMenuIndex;
				break;
			}
			case 7 : {
				if(alarmIpStream < 4){
					alarmIpStream++;
				}
				break;
			}
			case 8 : {
				if(!alarmTone){
					alarmTone = 1;
				}
				break;
			}
			
		}
	}
}
void addAlarmButtonDown(void){
	
	LcdClear();
	if(AddAlarmMenuSelected == 0){
		if(currentMenuIndex <= ADD_ALARM_ITEMS - 1 || (currentItem == MENU_ALARM_EDIT_5_ID && currentMenuIndex <= ADD_ALARM_ITEMS))
			currentMenuIndex += 1;
	}else{
		switch(currentMenuIndex){
			case 1 :
			if(subMenuIndex == 0){
				if(tempAlarmTime->tm_hour > 0)
					tempAlarmTime->tm_hour -= 1;
				else
					tempAlarmTime->tm_hour = 23;
				break;
			}else if(subMenuIndex == 1){
				if(tempAlarmTime->tm_min > 0)
					tempAlarmTime->tm_min -= 1;
				else
					tempAlarmTime->tm_min = 59;
				break;	
			
			}
			case 2 : {
				if(snoozeTime > 0){
					snoozeTime--;
				}
				break;
			}	
			case 3 : {
				if(alarmEnabled){
					alarmEnabled = 0;
				}
				break;
			}	
			case 4 : {
				if(alarmGame > 0){
					alarmGame = 0;
				}
				break;
			}	
			case 5 : {
				if(alarmSchedular){
					alarmSchedular = 0;
				}
				break;
			}
			case 6 :{
				if(alarmDays & 1 << subMenuIndex)
					alarmDays &= ~(1 << subMenuIndex);
				break;
			}
			case 7 : {
				if(alarmIpStream > 1){
					alarmIpStream--;
				}
				break;
			}
			case 8 : {
				if(alarmTone){
					alarmTone = 0;
				}
				break;
			}
			
		}
	}
}
void addAlarmButtonLeft(void){
	LcdClear();
	if(AddAlarmMenuSelected == 1){
		if(subMenuIndex > 0)
			subMenuIndex -= 1;
	}
}
void addAlarmButtonRight(void){
	LcdClear();
	if(AddAlarmMenuSelected == 1){
		if(currentMenuIndex == 6){
			if(subMenuIndex < 6)
				subMenuIndex += 1;
		}else{
		if(subMenuIndex < 1)
			subMenuIndex += 1;
		}
	}
}
void addAlarmButtonOk(void){
	if(currentMenuIndex == 9){ // save button
		//save alarm and go back
		tempAlarmTime->tm_sec = 0;
		AddAlarm(alarmEnabled,snoozeTime,alarmGame,alarmSchedular,tempAlarmTime,alarmDays,alarmIpStream,alarmTone);
		//reallocate memory for the time struct
		tempAlarmTime = malloc(sizeof(tm));
		tempAlarmTime->tm_sec = 0;
		subMenuEsc();
		currentItem = MENU_MAIN_0_ID;
		currentId = menu[currentItem].id;
		PrintAlarm();
		LcdClear();
		return;
	}else if(currentMenuIndex == 10){ //cancel button
		// cancel alarm and go back
		// free struct memory
		subMenuEsc();
		currentItem = MENU_MAIN_0_ID;
		currentId = menu[currentItem].id;
		return;
	}
	if(AddAlarmMenuSelected == 0)
		AddAlarmMenuSelected = 1;
	else
		AddAlarmMenuSelected = 0;
	
}

void editAlarmButtonOk(void){
	if(currentMenuIndex == 9){
		//save alarm and go back
		saveAlarm();
		saveAllAlarms();
		subMenuEsc();
		currentItem = MENU_MAIN_0_ID;
		currentId = menu[currentItem].id;
	}else if(currentMenuIndex == 10){
		// cancel alarm and go back
		// free struct memory
		subMenuEsc();
		currentItem = MENU_MAIN_0_ID;
		currentId = menu[currentItem].id;
	}else if(currentMenuIndex == 11){
		//delete alarm and go back
		DeleteAlarm(currentAlarmIndex);
		subMenuEsc();
		currentItem = MENU_MAIN_0_ID;
		currentId = menu[currentItem].id;
	}
	if(AddAlarmMenuSelected == 0)
		AddAlarmMenuSelected = 1;
	else
		AddAlarmMenuSelected = 0;
}

void timeChangeButtonLeft(void) {
	if(subMenuIndex > 0)
		subMenuIndex -= 1;
}
void timeChangeButtonRight(void) {
	if(subMenuIndex < 3)
		subMenuIndex += 1;
}
void timeChangeButtonUp(void) {
	switch(subMenuIndex) {
		case 0 :
			if(gmt.tm_hour < 23)
				gmt.tm_hour += 1;
			else
				gmt.tm_hour = 0;
			break;
		case 1 :
			if(gmt.tm_min < 59)
				gmt.tm_min += 1;
			else
				gmt.tm_min = 0;
			break;
		case 2 :
			if(gmt.tm_mday < 31)
				gmt.tm_mday += 1;
			else
				gmt.tm_mday = 1;
			break;
		case 3 :
			if(gmt.tm_mon < 12)
				gmt.tm_mon += 1;
			else
				gmt.tm_mon = 1;
			break;
	}
}
void timeChangeButtonDown(void) {
	switch(subMenuIndex) {
		case 0 :
			if(gmt.tm_hour > 0)
				gmt.tm_hour -= 1;
			else
				gmt.tm_hour = 23;
			break;
		case 1 :
			if(gmt.tm_min > 0)
				gmt.tm_min -= 1;
			else
				gmt.tm_min = 59;
			break;
		case 2 :
			if(gmt.tm_mday > 1)
				gmt.tm_mday -= 1;
			else
				gmt.tm_mday = 31;
			break;
		case 3 :
			if(gmt.tm_mon > 1)
				gmt.tm_mon -= 1;
			else
				gmt.tm_mon = 12;
			break;
	}
}

void timeChangeButtonOK(void) {
	X12RtcSetClock(&gmt);
	subMenuEsc();
}

void timeZoneChangeButtonUp(void) {
	if(tempTimeZone < 12)
		tempTimeZone += 1;
	else 
		tempTimeZone = -11;
}

void timeZoneChangeButtonDown(void) {
	if(tempTimeZone > -11)
		tempTimeZone -= 1;
	else 
		tempTimeZone = 12;
}

void timeZoneChangeButtonOK(void) {
	setTimeZone(tempTimeZone);
	subMenuEsc();
}

void subMenuEsc(void){
	LcdClear();
    currentMenuIndex = 1;
	subMenuIndex = 0;
}

void mainMenuEsc(void){

}

void mainMenuOk(void)
{
	X12RtcGetClock(&gmt);
	X12RtcGetClock(&tempAlarmTime);
	tempTimeZone = getTimeZone();
	

	switch(currentMenuIndex){
	
		case 1:
		currentItem = MENU_TIME_SETTING_1_ID;
		currentId = menu[currentItem].id;
		break;
		
		case 2:
		currentItem = MENU_TIMEZONE_SETTING_2_ID;
		currentId = menu[currentItem].id;
		break;
		
		case 3: 
		currentItem = MENU_ALARM_ADD_3_ID;
		currentId = menu[currentItem].id;
		// time struct malloc
		tempAlarmTime = (tm*)malloc(sizeof(tm));
		break;
		case 4 : 
		currentItem = MENU_All_ALARMS_4_ID;
		currentId = menu[currentItem].id;
		listSize = listLength();
		break;
		case 5 :
		currentItem = MENU_SET_TIMER_6_ID;
		currentId = menu[currentItem].id;
		break;
		default:
		
		break;
		
	
	}
	currentMenuIndex = 1;
	LcdClear();	
}

/* When the up key is pressed in the main menu*/
void mainMenuUp(void)
{
	LcdClear();
	if(currentMenuIndex != 1)
		currentMenuIndex -= 1;
}

/* When the down key is pressed in the main menu*/
void mainMenuDown(void)
{
	LcdClear();
	if(currentMenuIndex != MAIN_MENU_ITEMS)
		currentMenuIndex += 1;
}


void exitItem(void) {
	//printf("Exit\n");
	LcdClear();
	// clear het scherm
}

void enterItem(void) {
	
}

int getCurrentId(void){
	return currentItem;
}


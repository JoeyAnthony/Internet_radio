/*! \mainpage SIR firmware documentation
 *
 *  \section intro Introduction
 *  A collection of HTML-files has been generated using the documentation in the sourcefiles to
 *  allow the developer to browse through the technical documentation of this project.
 *  \par
 *  \note these HTML files are automatically generated (using DoxyGen) and all modifications in the
 *  documentation should be done via the sourcefiles.
 */

/*! \file
 *  COPYRIGHT (C) STREAMIT BV 2010
 *  \date 19 december 2003
 */


/*--------------------------------------------------------------------------*/
/*  Include files                                                           */
/*--------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>

#include <sys/thread.h>
#include <sys/timer.h>
#include <sys/version.h>
#include <dev/irqreg.h>

#include "system.h"
#include "portio.h"
#include "display.h"
#include "remcon.h"
#include "keyboard.h"
#include "led.h"
#include "log.h"
#include "uart0driver.h"
#include "mmc.h"
#include "watchdog.h"
#include "flash.h"
#include "spidrv.h"
#include "menu.h"
#include "player.h"
#include "vs10xx.h"
#include "spidrv.h"
#include "llist.h"

#include <time.h>
#include "rtc.h"
#include "shoutcast.h"
#include "clock.h"
#include "alarmcontroller.h"

#include "Input.h"

#include "server.h"

#include "x1205.h"
/*-------------------------------------------------------------------------*/
/* global variable definitions                                             */
/*-------------------------------------------------------------------------*/
#define LOG_MODULE  LOG_MAIN_MODULE

#define MAIN_MODE 0
#define ALARM_MODE 1

#define TRUE 1
#define FALSE 0

#define MAIN_MODE 0 
#define ALARM_MODE 1 

#define GAME_MODE 3
#define ALARM_SHEDULAR 2
#define ALARM_BEEPING 1
#define ALARM_SILENT 0

#define SNOOZE_MINUTES 1
#define SNOOZE_SECONDS 0
//can be used as minutes
#define MAX_SECONS_ALARM 25


#define NO_GAME 0
#define MATH_GAME 1
#define DINO_GAME 2

#define GAME_NOT_RUNNING 0
#define GAME_RUNNING 1

#define NO_STREAM -1


#define SUN 1 << 0
#define MON 1 << 1
#define TUE 1 << 2
#define WEN 1 << 3
#define THU 1 << 4
#define FRI 1 << 5
#define SAT 1 << 6


/*-------------------------------------------------------------------------*/
/* local variable definitions                                              */
/*-------------------------------------------------------------------------*/
tm gmt;
static uint8_t isSyncing = FALSE;
static uint8_t hasConnection = FALSE;
static uint8_t isAnAlarmOn = TRUE;

uint8_t buttonMode = MAIN_MODE; // 0 = deafault; 1 = when alarm triggers; 
uint8_t menuState = TRUE; // 0 = not in menu; 1 = in menu

static char exampleText[] = "Welcome to the monkey house";
static int index = 0;
int alarmStatus = ALARM_SILENT;
int gameMode = NO_GAME;

int mainGameState = GAME_NOT_RUNNING;

tm *time1;
tm *time2;	
tm *time3;	
tm *time4;
tm *time5;	
tm *time6;



Alarm *CurrentAlarm = NULL;
/*-------------------------------------------------------------------------*/
/* local routines (prototyping)                                            */
/*-------------------------------------------------------------------------*/
static void SysMainBeatInterrupt(void*);
static void SysControlMainBeat(u_char);

/*-------------------------------------------------------------------------*/
/* Stack check variables placed in .noinit section                         */
/*-------------------------------------------------------------------------*/



/*-------------------------------------------------------------------------*/
/*                         start of code                                   */
/*-------------------------------------------------------------------------*/


/* ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ */
/*!
 * \brief ISR MainBeat Timer Interrupt (Timer 2 for Mega128, Timer 0 for Mega256).
 *
 * This routine is automatically called during system
 * initialization.
 *
 * resolution of this Timer ISR is 4,448 msecs
 *
 * \param *p not used (might be used to pass parms from the ISR)
 */
/* ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ */
static void SysMainBeatInterrupt(void *p)
{

    /*
     *  scan for valid keys AND check if a MMCard is inserted or removed
     */
    KbScan();
    CardCheckCard();
}


/* ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ */
/*!
 * \brief Initialise Digital IO
 *  init inputs to '0', outputs to '1' (DDRxn='0' or '1')
 *
 *  Pull-ups are enabled when the pin is set to input (DDRxn='0') and then a '1'
 *  is written to the pin (PORTxn='1')
 */
/* ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ */
void SysInitIO(void)
{
    /*
     *  Port B:     VS1011, MMC CS/WP, SPI
     *  output:     all, except b3 (SPI Master In)
     *  input:      SPI Master In
     *  pull-up:    none
     */
    outp(0xF7, DDRB);

    /*
     *  Port C:     Address bus
     */

    /*
     *  Port D:     LCD_data, Keypad Col 2 & Col 3, SDA & SCL (TWI)
     *  output:     Keyboard colums 2 & 3
     *  input:      LCD_data, SDA, SCL (TWI)
     *  pull-up:    LCD_data, SDA & SCL
     */
    outp(0x0C, DDRD);
    outp((inp(PORTD) & 0x0C) | 0xF3, PORTD);

    /*
     *  Port E:     CS Flash, VS1011 (DREQ), RTL8019, LCD BL/Enable, IR, USB Rx/Tx
     *  output:     CS Flash, LCD BL/Enable, USB Tx
     *  input:      VS1011 (DREQ), RTL8019, IR
     *  pull-up:    USB Rx
     */
    outp(0x8E, DDRE);
    outp((inp(PORTE) & 0x8E) | 0x01, PORTE);

    /*
     *  Port F:     Keyboard_Rows, JTAG-connector, LED, LCD RS/RW, MCC-detect
     *  output:     LCD RS/RW, LED
     *  input:      Keyboard_Rows, MCC-detect
     *  pull-up:    Keyboard_Rows, MCC-detect
     *  note:       Key row 0 & 1 are shared with JTAG TCK/TMS. Cannot be used concurrent
     */
#ifndef USE_JTAG
    sbi(JTAG_REG, JTD); // disable JTAG interface to be able to use all key-rows
    sbi(JTAG_REG, JTD); // do it 2 times - according to requirements ATMEGA128 datasheet: see page 256
#endif //USE_JTAG

    outp(0x0E, DDRF);
    outp((inp(PORTF) & 0x0E) | 0xF1, PORTF);

    /*
     *  Port G:     Keyboard_cols, Bus_control
     *  output:     Keyboard_cols
     *  input:      Bus Control (internal control)
     *  pull-up:    none
     */
    outp(0x18, DDRG);
}

/* ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ */
/*!
 * \brief Starts or stops the 4.44 msec mainbeat of the system
 * \param OnOff indicates if the mainbeat needs to start or to stop
 */
/* ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ */
static void SysControlMainBeat(u_char OnOff)
{
    int nError = 0;

    if (OnOff==ON)
    {
        nError = NutRegisterIrqHandler(&OVERFLOW_SIGNAL, SysMainBeatInterrupt, NULL);
        if (nError == 0)
        {
            init_8_bit_timer();
        }
    }
    else
    {
        // disable overflow interrupt
        disable_8_bit_timer_ovfl_int();
    }
}

void printTime(void)
{
	//LogMsg_P(LOG_INFO, PSTR("RTC time [%02d:%02d:%02d]"), gmt.tm_hour, gmt.tm_min, gmt.tm_sec );
	LcdMoveCursor(0x05);
	char timeToWrite[10];
	sprintf(timeToWrite, "%02d:%02d %02d/%02d", gmt.tm_hour, gmt.tm_min, gmt.tm_mday, gmt.tm_mon);
	LcdWriteLine(timeToWrite);
}

void printSymbols(void) {
    LcdMoveCursor(0x00);
    if(hasConnection) 
        LcdDrawCustomIcon(0);
    else
        LcdWriteLine(" ");

    if(isSyncing)
        LcdDrawCustomIcon(1);
    else
        LcdWriteLine(" ");

    if(isAnAlarmOn)
        LcdDrawCustomIcon(2);
    else
        LcdWriteLine(" ");
}



int connectToStreamWrapped(int radioStream) {
	//radioStream = the number of the stream in the array named IPStreams in shoutcast
    if(hasConnection) {
        isSyncing = 1;
        printSymbols(); //if not here screen would not update with new inforamtion
        if(connectToStream(radioStream))
        {	
            return 1;
        }    
        else {
            return 0;
        }
    }
    else {
        return 0;
    }
}

void toFactorySettings(){
	deleteAllAlarms();
	saveAllAlarms();
	printf("everything is gone, %d", listLength());
	//reset 
}

int getTimeInternet() {
    //NTP get time
    int i = getTimeZone();
	setTimeZone(0);
    int j = setTimeFromNTP();
    if(0 == j){
        isSyncing = TRUE;
    } 
	setTimeZone(i);
    printf("\nResult get time: %i\n", j);
    return j;
}

int masterInit()
{
	SysInitIO();
	
	SPIinit();
    
	LedInit();
	

    LcdLowLevelInit();

    LcdBackLight(LCD_BACKLIGHT_ON);
    LcdMoveCursor(0x00);
    LcdWriteLine("initializing....");

    Uart0DriverInit();
    Uart0DriverStart();
	LogInit();
	LogMsg_P(LOG_INFO, PSTR("Alarm from RTC example"));

	//CardInit();
		
    if (At45dbInit()==AT45DB041B)
    {
        // ......
    }
	X12Init();
	initTimeZone();
	
	RcInit();
    
	KbInit();

	// enable 4.4 msecs hartbeat interrupt
    SysControlMainBeat(ON);             

    
    //Increase our priority so we can feed the watchdog.
    NutThreadSetPriority(1);

	//Enable global interrupts
	sei();
	
	//watchdog
	WatchDogDisable();
	
	//-----------------------------------------------------------
	
	//alarm controller
	AlarmControllerInit();
	
	//Clock inits
	X12Init();
	
	// /* Init network adapter */
	if(1 != initInet() )
	{
		LogMsg_P(LOG_INFO, PSTR("initinet() = nok, no network!"));
 	}
	else
	{
		LogMsg_P(LOG_INFO, PSTR("internet init ok!"));
        hasConnection = TRUE;
        
		
		getTimeInternet();
	}
	
	readAllAlarms();

    PLSetHasConnection(hasConnection);

	printClock();
	
	initServer();
	
	//radioStreams
	initRadioStreams();	
	
	NutDelay(100);
	
	printf("Initialised! \n");
	return 1;
}

void TurnOffAlarm(void)
{
	alarmStatus = ALARM_SILENT;
	buttonMode = MAIN_MODE;
	gameMode = NO_GAME;
	mainGameState = GAME_NOT_RUNNING;
	CurrentAlarm->isEnabled = 0;
	
}

void GenerateTestAlarms(void)
{
	printf("\nMaking alarm lists");
	
	 time1  = malloc(sizeof(tm));
	 time2  = malloc(sizeof(tm));	
	 time3  = malloc(sizeof(tm));	
	 time4  = malloc(sizeof(tm));
	 time5  = malloc(sizeof(tm));	
	 time6  = malloc(sizeof(tm));

	int days1 = 0b1111111;			// All days
	int days2 = MON | TUE | SUN | SAT;
	int days3 = TUE | WEN ; 
	int days4 = 0; 						// No Days
	
	
	// sets device time
	gmt.tm_hour = 9;
	gmt.tm_min = 59;
	gmt.tm_sec = 57;
	gmt.tm_wday = 1;	// monday
	
	X12RtcSetClock(&gmt);
	
	time1->tm_hour = 9;
	time1->tm_min = 59;
	time1->tm_sec = 50;	
	
	time2->tm_hour = 10;
	time2->tm_min = 00;
	time2->tm_sec = 02;
	
	time3->tm_hour = 10;
	time3->tm_min = 1;
	time3->tm_sec = 10;
	
	time4->tm_hour = 10;
	time4->tm_min = 2;
	time4->tm_sec = 10;
	
	time5->tm_hour = 10;
	time5->tm_min = 3;
	time5->tm_sec = 10;
	
	time6->tm_hour = 10;
	time6->tm_min = 4;
	time6->tm_sec = 05;
	

    // AddAlarm( isEnabled, snoozeTime,  hasGame, isShedular, time, days, streamURL, alarmTone)

	//AddAlarm(TRUE,1,NO_GAME,TRUE,time2,days1,NO_STREAM,TRUE);
    
	
	AddAlarm(TRUE,20,NO_GAME,FALSE,time1,days1,NO_STREAM,TRUE);		// Alarm before device time doesnt go off
	AddAlarm(TRUE,FALSE,NO_GAME,FALSE,time2,days4,NO_STREAM,TRUE);		// Non snoozeable alarm that goes off
	AddAlarm(TRUE,2,FALSE,NO_GAME,time3,days1,NO_STREAM,TRUE);			// Snoozeable alarm 
	AddAlarm(TRUE,FALSE,NO_GAME,FALSE,time4,days3,NO_STREAM,TRUE);		// Periodical alarm that only goes off on tue and wednesday
	AddAlarm(TRUE,FALSE,MATH_GAME,FALSE,time5,days1,NO_STREAM,TRUE);	// Joshua's math game alarm
	AddAlarm(TRUE,15,NO_GAME,TRUE,time6,days1,NO_STREAM,TRUE);				// Shedular with stream
	
    //AddAlarm(TRUE,FALSE,DINO_GAME,FALSE,time2,days1,NO_STREAM,TRUE);    // Nick's dino game alarm

	
	PrintAlarm();
	
}

/* ����������������������������������������������������������������������� */
/*!
 * \brief Main entry of the SIR firmware
 *
 * All the initialisations before entering the for(;;) loop are done BEFORE
 * the first key is ever pressed. So when entering the Setup (POWER + VOLMIN) some
 * initialisatons need to be done again when leaving the Setup because new values
 * might be current now
 *
 * \return \b never returns
 */
/* ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ */
int main(void)
{
	masterInit();	

	u_char inputKeyboardClock; // In Keyboard.h you can find alle the values
    uint32_t startValue = -1;
    uint32_t startValueALT = -1;
    uint8_t isALTPressed = FALSE; // 0 = FALSE; 1 = TRUE;


    //GenerateTestAlarms();				//USED FOR EASY TESTING

	lcdChangeLastText(exampleText, strlen(exampleText));
	lcdAddText("Good evening, enjoy your stay!",strlen("Good evening, enjoy your stay!"));
	
	
	int tickCounter = 0;
	tm tempTime;

	LcdClearLine(0);

    //printf("%i\n", JANSSON_MAJOR_VERSION);
    for (;;)
    {         
            NutSleep(100);
            hasConnection = PLGetHasConnection();

			
			// Checks the alarms and times only every 50 ticks.
			if(tickCounter % 50 == 0){
				tickCounter = 0;
                X12RtcGetClock(&gmt);   
				
				int temp;
				X12RtcGetAlarm(0,&tempTime, &temp);				
				if(CheckForAlarms(&gmt) == 1)	// when allarm goes off
				{
					LcdBackLight(LCD_BACKLIGHT_ON);
					GetBeepingAlarm(&CurrentAlarm);  				//Sets the CurrentAlarm that goes off in main
										
					
					// Joshua's individual assignment start (alarmstates)
					// Checks if alarm is shedular and if the device day is matching with the alarm day
					if(CurrentAlarm->isShedular == TRUE && CurrentAlarm->days & 1 << gmt.tm_wday)
					{
						printf("Schedular Alarm is ON\n");
						alarmStatus = ALARM_SHEDULAR;
					}
					// Checks if alarm is an regular alarm 
					else if(CurrentAlarm->isShedular == FALSE && CurrentAlarm->hasGame == FALSE)							
					{
                        printf("Regular alarm beeping\n");
						alarmStatus = ALARM_BEEPING;
					}
					// Checks if alarm has a game and which game
					else if(CurrentAlarm->hasGame)	
					{
						alarmStatus = GAME_MODE;
						mainGameState = GAME_RUNNING;
						switch(CurrentAlarm->hasGame)
						{
    						case 1:
                                gameMode = MATH_GAME;
                                break;						
    						case 2:
                                gameMode = DINO_GAME;
                                break;
    						default:
    							gameMode = NO_GAME;
                                break;	
						}
                        
					}
					
					
					LcdClear();
					menuState = FALSE;
					buttonMode = ALARM_MODE;


				}
				


			}
					
			
			
			if(alarmStatus == ALARM_BEEPING)
			{
				Alarming();
			}
			else if(alarmStatus == ALARM_SHEDULAR)
			{
				// Plays stream while end time is not reached.
				if(tickCounter % 50 == 0)
						X12RtcGetClock(&gmt);
					
				PlayShedular(&gmt);		
			}
			else if(alarmStatus == GAME_MODE)			
			{
				if(mainGameState == GAME_RUNNING)
				{					
					switch(gameMode)
					{
						
						case MATH_GAME:
							alarmStatus = ALARM_SILENT;
							mainGameState = GAME_NOT_RUNNING;
							RunMathGame();
						break;
						
						case DINO_GAME:
                            //TurnOffAlarmSound(); // makes an alarm sound using the main loop sleep instead of AlarmBeep();[vs10xx] aditional delay;
							RunDinoGame();
                            //TurnOnAlarmSound(); 
						break;
						
						default:
								printf("Error in game switch: Wrong game!");
								mainGameState = GAME_NOT_RUNNING;
                                TurnOffAlarmSound();
								TurnOffAlarm();
						break;
					}
					
					mainGameState = isGameRunning();
				
				}
			}
            
			// Joshua's individual assignment end
			
            isALTPressed = FALSE;
			inputKeyboardClock = kbGetKeySingle();
			
            if(getKeyFound() == KEY_ALT) {
                if(startValueALT == -1)
                {
                    startValueALT = NutGetSeconds(); 
                }
                isALTPressed = TRUE; 
            }


			if(menuState){ // startup screen
				if(inputKeyboardClock == KEY_ESC && getCurrentId() == 0){
					startValue = NutGetSeconds(); // >> after 10 sec backligt off
					menuState = 0;
					LcdClear();
				}else{
					handleMenu(inputKeyboardClock);
					handleMenuDisplay();
				}
			}else{
				checkTimer(NutGetSeconds());

                 if(NutGetSeconds() % 120 == 0) {
                    if(hasConnection == TRUE && mainGameState == GAME_NOT_RUNNING) {
                        LogMsg_P(LOG_INFO, PSTR("Veryfing internet"));
                        char text[] = "Verying internet";
                        lcdForceDisplayText(text, strlen(text));
                        lcdUpdateInfo();
                        if(-1 == getTimeInternet()) { // internet has fallen away
                            initInet();
                            hasConnection = FALSE;
                        }
                    }
                    else if (hasConnection == FALSE && mainGameState == GAME_NOT_RUNNING){
                        //char text[] = "Reconecting...";
                        //lcdForceDisplayText(&text, strlen(&text));
                        //lcdUpdateInfo();
                        LogMsg_P(LOG_INFO, PSTR("Trying to reconected to internet"));
                        if(1 == initInet() ) {
                            LogMsg_P(LOG_INFO, PSTR("Internet has reconected!"));
                            hasConnection = TRUE;
                            PLSetHasConnection(hasConnection);
                        }
                        else {
                            LogMsg_P(LOG_INFO, PSTR("Internet has failed to reconected!"));
                        }
                    }
                }

                if(mainGameState == GAME_NOT_RUNNING) {
    				printTime();

                    printSymbols();

                    lcdUpdateInfo();
                }

                if(inputKeyboardClock != KEY_UNDEFINED && mainGameState == GAME_NOT_RUNNING) // KEY_UNDEFINED = no key pressed
                {     // If two or more buttons are pressed at the same time the value will be 136 = default
                    LcdBackLight(LCD_BACKLIGHT_ON); // Turn backlight on on buttonpress
                    startValue = NutGetSeconds();
					
                    switch(buttonMode) 
                    {
                        case MAIN_MODE :
                            switch(inputKeyboardClock) 
                            { 
                                case KEY_01 : 
                                    {
                                        char text[] = "     Radio 1    ";
                                        lcdForceDisplayText(text , strlen(text));
                                        lcdUpdateInfo();
                                        connectToStreamWrapped(1);
                                    }
    								break;
                                case KEY_02 : 
                                    {
                                        char text[] = "     Radio 2    ";
                                        lcdForceDisplayText(text , strlen(text));
                                        lcdUpdateInfo();
                                        connectToStreamWrapped(2);
                                    }
                                    break;
                                case KEY_03 :
                                    { 
                                        char text[] = "       3FM      ";
                                        lcdForceDisplayText(text , strlen(text));
                                        lcdUpdateInfo();
                                        connectToStreamWrapped(3);
                                    }
                                    break;
                                case KEY_04 :
                                    {
                                        char text[] = "    Veronica    ";
                                        lcdForceDisplayText(text , strlen(text));
                                        lcdUpdateInfo();
                                        connectToStreamWrapped(4);
                                    }
                                    break;
                                case KEY_05 :
                                    {
                                        char text[] = "    Radio 538   ";
                                        lcdForceDisplayText(text , strlen(text));
                                        lcdUpdateInfo();
                                        connectToStreamWrapped(5);
                                    }
                                    break;
                                case KEY_ALT :
                                    stopStream();
                                    isSyncing = FALSE;
                                    break;
    							case KEY_ESC : 
    								LcdBackLight(LCD_BACKLIGHT_ON);
									LcdClear();
									menuState = TRUE;
    								break;
                                case KEY_UP :
                                    { // so it deletes variables created inside, also makes it able to define a variable right after the case x:

                                        VsVolumeUp();
										//LcdClearLine(1);
										char tempText2[] = "    Volume up";
                                        lcdForceDisplayText(tempText2, strlen(tempText2));
                                    }
                                    break;
                                case KEY_DOWN :
                                    { // so it deletes variables created inside, also makes it able to define a variable right after the case x:
                                        VsVolumeDown();
										//LcdClearLine(1);
										char tempText[] = "  Volume down";
                                        lcdForceDisplayText(tempText, strlen(tempText));
                                    }
                                    break;
    							
                            }
                            break;
                        case ALARM_MODE :
                            switch(inputKeyboardClock) 
                            {
                                case KEY_02 :	// Reserved for Joshua's game
                                case KEY_03 :
                                case KEY_05 : 	// Reserved for Joshua's game
                                case KEY_ALT :
                                case KEY_ESC :	// Disables alarm if hasGame is FALSE.
								if(gameMode == NO_GAME)    //TODO TEST
								{
									printf("Alarm Disabled\n");
									CurrentAlarm->isEnabled = FALSE;
								}
								break;
                                case KEY_OK :	// Snoozes the alarm if possible
								if(CurrentAlarm->snoozeTime > 0 && gameMode == NO_GAME) 
								{
									printf("Alaram Snoozed\n");
									alarmStatus = ALARM_SILENT;
									buttonMode = MAIN_MODE;
									SnoozeAlarm(CurrentAlarm->snoozeTime,SNOOZE_SECONDS);									
								}
								break;
								case KEY_UP :   // Reserved for Nicks game
                                case KEY_DOWN : // Reserved for Nicks game
                                case KEY_LEFT :
                                case KEY_RIGHT :
                                
                                case KEY_01 : // Reserved for Joshua's game			
                                case KEY_04 : // Reserved for Joshua's game		
								break;
                            }
                            break;
                    }
                    

                }
            }
            if(NutGetSeconds() - startValue >= 10 && startValue != -1) // check to see if the backlight needs to be turned off
            {
                LcdBackLight(LCD_BACKLIGHT_OFF);
                startValue = -1;
            }
            if(!isALTPressed) // check if ALT is still pressed
            {
                startValueALT = -1;
            }
            if(NutGetSeconds() - startValueALT >= 10 && isALTPressed && startValueALT != -1) // if startValueALT == -1 -> button has been released before the 10 sec mark
            {
                startValueALT = -1;
                printf("ALT pressed for 10 seconds\n");

                
                toFactorySettings(); // TODO reset code in function
            }

		tickCounter++;
    }
    return(0);      // never reached, but 'main()' returns a non-void, so.....
}
/* ---------- end of module ------------------------------------------------ */


/* ========================================================================
 * [TITLE]      Controller for the alarm
 * [FILE]       alarmcontroller.h
 * [VSN]        1.0
 * ======================================================================== */

#ifndef _AlarmController_H
#define _AlarmController_H
#include "time.h"

/*-------------------------------------------------------------------------*/
/* global defines                                                          */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* typedefs & structs                                                      */
/*-------------------------------------------------------------------------*/


typedef struct AlarmStruct
{
	int isEnabled;
							// Wordt ook gebruikt als aantal minuten voor de Shedular functie
	int snoozeTime;			// moet een getal zijn in tussen 0 en 59 waarbij 0 betekend dat alarm niet snoozable is.
	int hasGame;			// 0 for no game, 1 for math game of Joshua, 2 for Nicks dino game.
	int isShedular;
	int days;				// 0bxxxxxxx    7 bits set to 1 or 0 to enable or disable a day counting from sunday
	
	tm *time;
	
	int ipStreamURL;
	int alarmTone;			// uses alarm tone yes or no
} Alarm;

// Used for constructing the enabled days
// to create a days int construct it like this
// int days = MON | TUE | WED
// This will create the right binary number
#define SUN 1 << 0
#define MON 1 << 1
#define TUE 1 << 2
#define WEN 1 << 3
#define THU 1 << 4
#define FRI 1 << 5
#define SAT 1 << 6

#define SHEDULAR_RUNNING 1
#define SHEDULAR_NOT_RUNNING 0
#define STREAM_RUNNING 1
#define STREAM_NOT_RUNNING 0
#define NO_STREAM -1

#define SNOOZE_MINUTES 1
#define SNOOZE_SECONDS 0

#define MATH_GAME 1
#define DINO_GAME 2

#define GAME_NOT_RUNNING 0
#define GAME_RUNNING 1

#define TRUE 1
#define FALSE 0
#define MAX_BEEP_COUNT 100		// Determines how long the alarm goes off

//Dino game
#define OBSTACLE_NOTHING 0
#define OBSTACLE_TOP 1
#define OBSTACLE_BOTTOM 2

/*--------------------------------------------------------------------------*/
/*  Global variables                                                        */
/*--------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* export global routines (interface)                                      */
/*-------------------------------------------------------------------------*/
void AddAlarm(int , int , int, int , tm* , int , int , int);
void PrintAlarm(void);
void AlarmControllerInit(void);
void Alarming(void);
void PlayShedular(tm* );
void SnoozeAlarm(int,int);
void GetBeepingAlarm(Alarm**);
void RunMathGame(void);
void RunDinoGame(void);
void GenerateExcersise(void);


void shuffle(int *array, size_t n);
int GetAlarmByIndex(int, Alarm**);		
int CheckForAlarms(tm*);
int listLength(void);
int EditAlarm(int ,int, int , int, int , tm* , int , int , char* );
int DeleteAlarm(int);
int getNextActiveAlarmToday(Alarm*, tm*);
int deleteAllAlarms(void);
int isGameRunning(void);

int saveAllAlarms(void);
int readAllAlarms(void);


#endif  /* _AlarmController_H */






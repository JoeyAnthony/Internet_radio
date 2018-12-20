#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/timer.h>
#include <sys/event.h>
#include <sys/thread.h>
#include <sys/heap.h>

#include "system.h"
#include "portio.h"
#include "alarmcontroller.h"
#include "log.h"
#include "rtc.h"
#include "display.h"

#include "llist.h"
#include "vs10xx.h"
#include "memdefs.h"
#include "flash.h"
#include "keyboard.h"

#include "log.h"
#define LOG_MODULE  LOG_MAIN_MODULE

/*-------------------------------------------------------------------------*/
/* local defines                                                           */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* local variable definitions                                              */
/*-------------------------------------------------------------------------*/
AlarmNode *AlarmList = NULL;
Alarm *currentBeepingAlarm = NULL;
Alarm *emptyAlarm = NULL;
int beepCount = 0;
int dayNr = 0;

// Variables for the math game
int number1 = 0;
int number2 = 0;
int realAnswer = 0;
int fakeAnswer1 = 0;
int fakeAnswer2 = 0;
int fakeAnswer3 = 0;
int correctAnswers = 0;

int gameState = GAME_NOT_RUNNING;
int streamState = STREAM_NOT_RUNNING;

// Dino game vars
int startTimeDinoGame = 0;
int lastTimeInSeconds = 0;
uint8_t isDinoJumping = FALSE;
int obstacleArray[32]; // chose for a 32 size array just so wouldn't have problems with shifitng values or creating new obstacles where you just ahve been
uint8_t indexObstacleArray = 0;
int dinoGameRunning = FALSE;
uint8_t dinoBeepPlaying = FALSE;


/*-------------------------------------------------------------------------*/
/* local routines (prototyping)                                            */
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/*                         start of code                                   */
/*-------------------------------------------------------------------------*/

//This method should always be called first
void AlarmControllerInit()
{
    AlarmList = malloc(sizeof(AlarmNode));
	AlarmList->next = NULL;
	AlarmList->data = NULL;
	emptyAlarm->isEnabled = FALSE;
}

// TODO DEFAULT FIELDS IF ANYTHING IS NULL
void AddAlarm(int isEnabled,int snoozeTime, int hasGame, int isShedular, tm *time, int days, int streamURL, int alarmTone)
{
	Alarm *tempAlarm = malloc(sizeof(Alarm));
	tempAlarm->isEnabled = isEnabled;
	tempAlarm->snoozeTime = snoozeTime;
	tempAlarm->hasGame = hasGame;
	tempAlarm->isShedular = isShedular;
	tempAlarm->time = time;
	tempAlarm->days = days;	
	tempAlarm->ipStreamURL = streamURL;
	tempAlarm->alarmTone = alarmTone;
	if(hasGame != 0)						// if game then shedular and snoozetime wont be enabled
	{
		tempAlarm->isShedular = FALSE;
		tempAlarm->snoozeTime = FALSE;
	}
	// TODO TEST
	
	//printf("\nShedular = %d    Game = %d  ",tempAlarm->isShedular, tempAlarm->snoozeTime);		// TODO VERIFY
	
	ListInsert(AlarmList, tempAlarm);
	
	static char alarmText[13];
	//sprintf(alarmText, "Alarm : %02d:%02d  ", tempAlarm->time->tm_hour, tempAlarm->time->tm_min); 
	lcdAddText(alarmText, 13);
	saveAllAlarms();
}

void PrintAlarm(void)
{
	printf("\nSize of list : %d\n",listLength());
	ListPrint(AlarmList);
}

int listLength()
{
  AlarmNode* currrent = AlarmList;
  int size = 0;

  while (currrent->data != NULL)
  {
    ++size;
    currrent = currrent->next;
  }

  return size;
}

/*!
 * \brief Checks if the current time matches with one of the alarms
 * \brief checks if the day matches if 
 * \brief checks for if there is a game
 * \return 0 on no alarm or 1 if alarm is found
 */
int CheckForAlarms(tm *currentTime)
{
	AlarmNode *currentAlarm = AlarmList;
	int isPeriodical = FALSE;
	//currentBeepingAlarm = emptyAlarm;				CHECK: This breaks the shedular, not sure if needed tough
	
	// Loops trough all alarms in alarmlist checking the HH:MM:SS if they are matching
	while(currentAlarm != NULL)
	{
		isPeriodical = FALSE;
		
		if(currentAlarm->data->days > 0)	// if one of the days is enabled
			isPeriodical = TRUE;

	
		if(currentAlarm->data != NULL)		// if there is an alarm
		{
					
			// TESTLINES TO SEE ALL ALARMS
			//printf("\n Alarm hour: %d   Current hour: %d", currentAlarm->data->time->tm_hour,currentTime->tm_hour);
			//printf("\n Alarm min: %d  Current min : %d", currentAlarm->data->time->tm_min,currentTime->tm_min);
			//printf("\n Alarm sec: %d Current sec: %d    isEnabled: %d", currentAlarm->data->time->tm_sec, currentTime->tm_sec, currentAlarm->data->isEnabled );
					
			
			if(currentAlarm->data->isEnabled == TRUE)										// If selected alarm is enabled
			{	
				if(currentTime->tm_hour == currentAlarm->data->time->tm_hour)				// If currenttime surpassed the alarm    Might need to be changed back to == in the future
				{	
					if(currentTime->tm_min == currentAlarm->data->time->tm_min)
					{	
						if(currentTime->tm_sec - currentAlarm->data->time->tm_sec >= 0 && currentTime->tm_sec - currentAlarm->data->time->tm_sec <= 20)	// 20 sec window for trigering alarm		// THE SECONDS IS ONLY FOR TESTIN CAN BE REMOVED IN THE FUTURE
						{
							// int curTimeInMin = (currentTime->tm_hour * 60) + currentTime->tm_min;
							// int curAlarmInMin = (currentAlarm->data->time->tm_hour * 60) + currentAlarm->data->time->tm_min;
							// int difference = curTimeInMin - curAlarmInMin;
							
							// printf("Time: %d    Alarm:  %d    diff : %d \n", curTimeInMin, curAlarmInMin, difference);
							

							//if(difference <= 3) {											// When setting the alarm before the device time.
								if(isPeriodical == TRUE)													
								{																	
									if(currentAlarm->data->days & 1 << currentTime->tm_wday)	// checks if the right byte is enabled or not in days
									{
										//printf("\nDays are matchin and beeping");
										currentBeepingAlarm = currentAlarm->data;
										return 1;
									}
								}
								else{

																	
									//printf("\nNon Periodical alarm went off");
									//printf("\n Current :Alarm Time : %d:%d \n", currentBeepingAlarm->time->tm_sec,currentBeepingAlarm->time->tm_min);
									currentBeepingAlarm = currentAlarm->data;
									
									return 1;
								}
								PrintAlarm();	
							//}
						}
						
					}	
				
				}
			}
		}
		
		currentAlarm = currentAlarm->next;
	}
		
	return 0;
}


// Deletes the alarm at the given index if in the list return 1 on succes, 0 on no succes
// ALARMINDEX SHOULD BE COUNTED FROM 0 and on.
int DeleteAlarm(int alarmIndex)
{
	AlarmNode *currentAlarm = AlarmList;
	int currentIndex = 0;
	
	if(alarmIndex == 0)	// If head node
	{
		//if list is empty
		if(currentAlarm == NULL) 
		{
			return 0;
		}
		
		if(currentAlarm->next == NULL)
		{
			printf("last item in list\n");
		
			// free memory
			free(currentAlarm->data->time);
			free(currentAlarm->data);
			free(currentAlarm);
			currentAlarm = NULL;
			AlarmControllerInit();
			saveAllAlarms();
			return 1;
		}
		
		// Data of next alarm gets coppied to the current
		AlarmList = currentAlarm->next;
		
		// Removes the link between alarms
		currentAlarm->next = currentAlarm->next->next;
		
		// free memory
		free(currentAlarm->data->time);
		free(currentAlarm->data);
		free(currentAlarm);
	
		printf("Head is deleted :D\n");
		saveAllAlarms();
		return 1;
	}
	else
	{
		AlarmNode *oneBeforeCurrentAlarm = currentAlarm;
		while(currentAlarm->next != NULL && currentIndex != alarmIndex) // while not at the right element
		{
			oneBeforeCurrentAlarm = currentAlarm;
			currentAlarm = currentAlarm->next;
			currentIndex++;					
		}
		
		if(currentIndex == alarmIndex)
		{
			if(currentAlarm->next != NULL)
			{
				oneBeforeCurrentAlarm->next = currentAlarm->next;
			}
			else
			{
				oneBeforeCurrentAlarm->next = NULL;
			}
			
			// free memory
			free(currentAlarm->data->time);
			free(currentAlarm->data);
			free(currentAlarm);	
			return 1;
		}
		else
		{
			printf("Element trying to delete doesn't exist...\n");
			return 0;
		}
		if(currentAlarm->next == NULL)
		{
			printf("Element trying to delete doesn't exist...\n");
			return 0;
		}
				
		printf("Soow deleted :D \n");
		return 1;	
	}
}

/**
* 	Get next active alarm
*	returns 1 if succefuly found a next a alarm. puts the next alarm into the Alarm parameter
*	returns 0 if it was unable to find an alarm(Alarm return value does also not get set)
*/
int getNextActiveAlarmToday(Alarm *nextActiveAlarm, tm *currentTime) {

	Alarm *tempAlarm = AlarmList->data;
	AlarmNode * current = AlarmList;
	LogMsg_P(LOG_INFO, PSTR("Given time: %i:%i"), currentTime->tm_hour, currentTime->tm_min);
	while(current != NULL) {
		if(current->data->isEnabled == TRUE) { 
			if(current->data->days == 0 || current->data->days & 1 << currentTime->tm_wday) { // check if alarm is non periodcal or if not if it is set for today
				LogMsg_P(LOG_INFO, PSTR("Time: %i:%i"), current->data->time->tm_hour, current->data->time->tm_min);
				if(current->data->time->tm_hour >= currentTime->tm_hour) {
					if(current->data->time->tm_hour == currentTime->tm_hour) {
						if(current->data->time->tm_min >= currentTime->tm_min) {
							if(current->data->time->tm_hour <= tempAlarm->time->tm_hour) {
								if(current->data->time->tm_hour == tempAlarm->time->tm_hour) {
									if(current->data->time->tm_min < tempAlarm->time->tm_min ) {
										tempAlarm = current->data;
									}
								}
								else{
									tempAlarm = current->data;
								}
							}
						}
					}
					else{
						if(current->data->time->tm_hour <= tempAlarm->time->tm_hour || tempAlarm->time->tm_hour < currentTime->tm_hour) {
							if(current->data->time->tm_hour == tempAlarm->time->tm_hour) {
								if(current->data->time->tm_min < tempAlarm->time->tm_min ) {
									tempAlarm = current->data;
								}
							}
							else{
								tempAlarm = current->data;
							}
						}
					}
				}
				else{
					tempAlarm = current->next;
				}
			}
		}
		current = current->next;
	}

	// if(current->data->isEnabled == FALSE) {
		// return 0;
	// }
	// if(current->data->days != 0 && !(current->data->days & 1 << currentTime->tm_wday)) {
		// return 0;
	// }
	// if(current->data->time->tm_hour <= tempAlarm->time->tm_hour) {
		// if(current->data->time->tm_min <= currentTime->tm_min) {
			// return 0;
		// }
	// }
	*nextActiveAlarm = *tempAlarm;
	LogMsg_P(LOG_INFO, PSTR("Next alarm time: %i:%i"), nextActiveAlarm->time->tm_hour, nextActiveAlarm->time->tm_min);
	return 1;
}

int deleteAllAlarms(){
	int listSize = listLength();
	int i;
	for(i = 0;i<listSize;i++)
		DeleteAlarm(0);
}

// Changes data for a selected alarm, if parameter is NULL it skipps this field when editing the Alarm.
// Counting from 0 and on. Returns 1 when succes and 0 if no succes
int EditAlarm(int alarmIndex,int isEnabled,int snoozeTime, int hasGame, int isShedular, tm *time, int days, int streamURL, char* alarmTone)
{
	AlarmNode *currentAlarm = AlarmList;
	int currentIndex = 0;
	 
	while(currentIndex != alarmIndex && currentAlarm->next != NULL) // while not at the right element and next is not null
	{
		currentAlarm = currentAlarm->next;
		currentIndex++;					
	}
	
	if(hasGame != 0)													// disables shedular and snoozeTime if there is a game
	{
		snoozeTime = FALSE;
		isShedular = FALSE;											// TODO VERIFY 
	}
	
	if(currentAlarm->next == NULL)
		return -1;
		
	if(isEnabled != NULL)
		currentAlarm->data->isEnabled = isEnabled;

	if(snoozeTime != NULL)
		currentAlarm->data->snoozeTime = snoozeTime;
	
	if(hasGame != NULL)
		currentAlarm->data->hasGame = hasGame;
	 
	if(isShedular != NULL)
		currentAlarm->data->isShedular = isShedular;
	
	if(time != NULL)
		currentAlarm->data->time = time;
	
	if(days != NULL)
		currentAlarm->data->days = days;
	
	if(streamURL != NULL)
		currentAlarm->data->ipStreamURL = streamURL;
	
	if(alarmTone != NULL)
		currentAlarm->data->alarmTone = alarmTone;
	 
	 
	 
	 return 1;

	 
	 
}

/**
* 	Create new obstacles for the dino game
* 	input: uint8_t createNew
*	opstions: TRUE, FALSE
*	TRUE -> create a totaly new obstacle course.
*	FALSE -> create 16 new obstacles while keeping the last 16 obstacles.
*	
*	This function uses the method: rand();  
*	"The rand() function does not return random numbers. What it does return is a pseudo-random number that is generated by a convoluted algorithm. Now the problem with any deterministic algorithm is that given the same input values it will always output the same output values. This is valid for the numbers that are generated by rand() as well." - from Documentation
*	Documentation: http://www.ethernut.de/nutwiki/index.php/Random_Numbers
*/

void fillObstacleArray(uint8_t createNew) {
	uint8_t i = 0;

	uint8_t maxI = 32;

	uint8_t prevObstacle;

	int randomObstacle;

	if(createNew == TRUE) { // set values for full obstacles array init
		for(i; i < 6; i++) {// create 6 empty obstacles when starting up game
			obstacleArray[i] = OBSTACLE_NOTHING;
		}
		prevObstacle = OBSTACLE_NOTHING;

	}
	else { // set values for if new bstacles need te be regenarated
		prevObstacle = obstacleArray[15];

		for(i; i < 16; i++) { // set the last 16 object to the front of the array
			obstacleArray[i] = obstacleArray[i + 16];
		}
		i = 16;
	}

	for(i; i < maxI; i++) {
		randomObstacle = (rand() % 3);
		// if(dinoGameRunning == FALSE && i < 6) { 
		// 	obstacleArray[i] = OBSTACLE_NOTHING;
		// }
		// else {
			if(prevObstacle == OBSTACLE_TOP && randomObstacle == OBSTACLE_BOTTOM) { // makes sure there is a path
				//randomObstacle = (rand() % 2);
				randomObstacle = OBSTACLE_NOTHING;
			}
			else if(prevObstacle == OBSTACLE_BOTTOM && randomObstacle == OBSTACLE_TOP) { // makes sure ther is a path
				// randomObstacle = (rand() % 2);
				// if(randomObstacle != OBSTACLE_NOTHING) {
				// 	randomObstacle = OBSTACLE_BOTTOM;
				// }
				randomObstacle = OBSTACLE_NOTHING;
			}
			obstacleArray[i] = randomObstacle;
			prevObstacle = randomObstacle;
		// }
	}
}

/**
*	draw one of the the obsicle lines
*	input: int line
*	option: OBSTACLE_TOP, OBSTACLE_BOTTOM
*	OBSTACLE_TOP -> draws the first line on the screen.
*	OBSTACLE_BOTTOM -> draws the second line in the screen.
*/
void drawObstacleLine(int line) {
	if(line == OBSTACLE_TOP) {
		LcdMoveCursor(0x00);
	}
	else if(line == OBSTACLE_BOTTOM) {
		LcdMoveCursor(0x40);
	}

	int i = indexObstacleArray;
	for(i; i < (16 + indexObstacleArray); i++) {
		if(obstacleArray[i] == line) {
			// LcdChar('O');
			LcdDrawCustomIcon(6);
		}
		else {
			LcdChar(' ');
		}
	}	
}

/**
*	reset the dino game
*/
void resetDinoGame(void) {
	isDinoJumping = FALSE;
	dinoGameRunning = FALSE;
	startTimeDinoGame = 0;
	indexObstacleArray = 0;
	fillObstacleArray(TRUE);
}

/**
* When dino game is running it should go into this loop to preform dino game mechincs
*/
void RunDinoGame(void)
{
	u_char input = kbGetKeySingle();
	LcdClear();
	LcdBackLight(LCD_BACKLIGHT_ON);
	
	if(gameState == GAME_NOT_RUNNING)
	{
		gameState = GAME_RUNNING;
		resetDinoGame();
	}

	// let the game idle first until user is ready to use it. If player is ready he should press any button to start
	if(input != KEY_UNDEFINED && dinoGameRunning == FALSE) {
		dinoGameRunning = TRUE;
		startTimeDinoGame = NutGetSeconds();
		lastTimeInSeconds = NutGetSeconds();
	}

	if(input == KEY_UP) {
		isDinoJumping = TRUE;

	}
	else if( input == KEY_DOWN) {
		isDinoJumping = FALSE;
	}

	

	// draw objects line by line
	drawObstacleLine(OBSTACLE_TOP);
	drawObstacleLine(OBSTACLE_BOTTOM);

	//draw dino on top or bottom line
	if(isDinoJumping != FALSE) {
		LcdMoveCursor(0x00);
	}
	else {
		LcdMoveCursor(0x40);
	}
	LcdDrawCustomIcon(4); //tale dino
	LcdDrawCustomIcon(5); //body dino


	if(dinoGameRunning == TRUE) { // check if game is running
		if(lastTimeInSeconds != NutGetSeconds()) {	//walk a step if true
			lastTimeInSeconds = NutGetSeconds();
			indexObstacleArray++;

			if(indexObstacleArray == 16) {
				fillObstacleArray(FALSE);
				indexObstacleArray = 0;
			}

			if(dinoBeepPlaying == TRUE) {
				TurnOffAlarmSound();
				dinoBeepPlaying = FALSE;
			} 
			else {
				TurnOnAlarmSoundWithoutInit();
				dinoBeepPlaying = TRUE;
			}
		}
	}

	// check if dino has run into obstacle
	if((isDinoJumping == TRUE && obstacleArray[indexObstacleArray + 1] == OBSTACLE_TOP) || // plus 1 becuase only check dino body not tale
		(isDinoJumping == FALSE && obstacleArray[indexObstacleArray + 1] == OBSTACLE_BOTTOM)) {
		// reset game:
		resetDinoGame();
	}

	if(NutGetSeconds() - startTimeDinoGame >= 60  && startTimeDinoGame != 0) { // After 60 seconds of surving the alarm ends
		TurnOffAlarm();
		TurnOffAlarmSound();
		gameState = GAME_NOT_RUNNING;
		resetDinoGame();
		LcdClear();
	}

	//AlarmBeep();
}

// Joshua's individual assignment 
// Generates 2 numbers, 1 real answer and 3 similar fake answers
void GenerateExcersise(void)
{
	srand ( time(NULL) );								// Rand nrs are sometimes the same because of the time seed
	number1 = rand() % 20 + 1;							
	number2 = rand() % 20 + 1;							// Two random numbers for the exercise
	
	realAnswer = number1 + number2;
	fakeAnswer1 = realAnswer + (rand() % 4 + 1) ;
	fakeAnswer2 = realAnswer - (rand() % 4 + 1) ;
	fakeAnswer3 = realAnswer + (rand() % 10 + 1);		// Generate 3 fake answers which look similar to the realanswers
	
}


// Joshua's individual assignment
// Shuffles all elements in an array
void Shuffle(int *array, size_t n)
{
    if (n > 1) 
    {
        size_t i;
        for (i = 0; i < n - 1; i++) 
        {
          size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
          int t = array[j];
          array[j] = array[i];
          array[i] = t;
        }
    }
}

// Joshua's individual assignment
// Function initialises the math game while setting off the alarm.
void RunMathGame(void)
{
	GenerateExcersise();
	
	gameState = GAME_RUNNING;		
	
	// Moves the selected block on the lcd to the top left.
	LcdMoveCursor(0x00);
		
	// Put all answers in an array
	int answers[4] = {realAnswer, fakeAnswer1, fakeAnswer2, fakeAnswer3};
	
	// Shuffle the array
	Shuffle(&answers, 4);
	
	// Find index of right answer
	int realIndex = 0;
	int i = 0;
	for(; i<4; i++)
	{
		if(answers[i]==realAnswer)
		{
			realIndex = i;
			break;
		}
	}
	
	// Creates string of the excersise 
	char str[20];
	sprintf(str, "%d + %d = ? 1:%d", number1, number2, answers[0]);
	
	// Creates string of the answers
	char str2[20];
	sprintf(str2, "2:%d 3:%d 4:%d",answers[1], answers[2], answers[3]);
	
	// Writes excersise
	LcdWriteLine(str);
	
	// Moves to the second linde
	LcdMoveCursor(0x40);
	
	// Writes answers
	LcdWriteLine(str2);
	
	
	u_char input = KEY_UNDEFINED;	
	
	// Checking answer input while beeping the alarm until a right answer is given
	for(;;)
	{
		AlarmBeep();
		input = kbGetKeySingle();
		if(input != KEY_UNDEFINED)
		{
			if(input == KEY_01 && realIndex==0)
			{
				// You win
				TurnOffAlarm();
				gameState = GAME_NOT_RUNNING;
				LcdClear();
				break;
			}
			else if(input == KEY_02 && realIndex==1)
			{
				// You win
				TurnOffAlarm();
				gameState = GAME_NOT_RUNNING;
				LcdClear();
				break;
			}
			else if(input == KEY_03 && realIndex==2)
			{
				// You win
				TurnOffAlarm();
				gameState = GAME_NOT_RUNNING;
				LcdClear();
				break;
			}
			else if(input == KEY_04 && realIndex==3)
			{
				// You win
				TurnOffAlarm();
				gameState = GAME_NOT_RUNNING;
				LcdClear();
				break;
			}
			else
			{
				// You lost				
				input = KEY_UNDEFINED;
			}
		}
			
	}
}


// Returns 1 if a game is running 
// Returns 0 if no game is running
int isGameRunning(void)					
{
	if(gameState != GAME_NOT_RUNNING)
		return 1;
	
	return 0;
}

// Gets an alarm by Index  
int GetAlarmByIndex(int alarmIndex, Alarm **alarm)
{
	AlarmNode *currentAlarm = AlarmList;
	int currentIndex = 0;	 
	
	while(currentAlarm != NULL) 						// while not at the right element and next is not null
	{
		if(currentIndex == alarmIndex){
			*alarm = currentAlarm->data;
			return 1;
		}	
		currentAlarm = currentAlarm->next;
		currentIndex++;			
	}	
	printf("\n Failed getting by index");
	return 0;	 
 }


/*
	Function which gets called when an shedular typed alarm goes off.
	The function takes the devices time as param
	It compares the device time in sec with the shedular endtime in sec.
*/
void PlayShedular(tm *currentTime)
{
		
		int currentTimeInMin = (currentTime->tm_hour * 60) +   currentTime->tm_min; 						
		/*int shedularEndTimeInMin = (currentBeepingAlarm->time->tm_hour * 60) + (currentBeepingAlarm->time->tm_min ) 
									+ currentBeepingAlarm->time->tm_sec + currentBeepingAlarm->snoozeTime;			*/									// Seconds Shedular

									
		int shedularEndTimeInMin = (currentBeepingAlarm->time->tm_hour * 60) + currentBeepingAlarm->time->tm_min + currentBeepingAlarm->snoozeTime;		// Minutes Shedular (FOR FINAL VERSION)	
		int difference = shedularEndTimeInMin - currentTimeInMin;
		
		//printf("\nCurrentTimein Min: %d   ShedularEndTime: %d   Difference: %d  end hour: %d  end min : %d",currentTimeInMin,shedularEndTimeInMin,difference,currentBeepingAlarm->time->tm_hour,currentBeepingAlarm->time->tm_min);
		// Checks if the device time has not surpassed the shedular time.
		if(difference > 0 && difference < 1439) //1439 because sometimes the difference is really big 																						
		{
			if(streamState == STREAM_NOT_RUNNING && currentBeepingAlarm->ipStreamURL != NO_STREAM)
			{
				if(currentBeepingAlarm->ipStreamURL > 0)																								
					connectToStreamWrapped(currentBeepingAlarm->ipStreamURL);
				else
					connectToStreamWrapped(1);		//TODO JOEY FIX STREAM
				
				printf("\nStarting up stream!   stream number = %d", currentBeepingAlarm->ipStreamURL);
				streamState = STREAM_RUNNING;			
			}
			else
			{			
				//AlarmBeep();	
				//printf("\nShedular is going off but no stream connected");																										//FOR TESTING IF STREAM NO WORK
				return;
			}
		}
		else
		{	
			printf("\nTurning off shedular");
			LcdBackLight(LCD_BACKLIGHT_OFF);
			currentBeepingAlarm->isEnabled = FALSE;
			beepCount = 0;
			streamState = STREAM_NOT_RUNNING;
			TurnOffAlarm();																																// IF STREAM PLAYING THEN STOP PLAYING STREAM HERE
		}
}



// This function get called when an alarm fires.
void Alarming(void)
{		
		// Checks if the alarm has surpassed the max alarming time and turns the alarm off.
		if(beepCount >= MAX_BEEP_COUNT || currentBeepingAlarm->isEnabled == FALSE)
		{
			 LcdBackLight(LCD_BACKLIGHT_OFF);
			currentBeepingAlarm->isEnabled = FALSE;
			beepCount = 0;
			streamState = STREAM_NOT_RUNNING;				// IF STREAM PLAYING THEN TURN IT OFF HERE
			TurnOffAlarm();
		}
		if(currentBeepingAlarm->alarmTone == TRUE)
		{
				AlarmBeep();
		}
		else
		{
			
			if(streamState == STREAM_NOT_RUNNING && currentBeepingAlarm->ipStreamURL != NO_STREAM)
			{
				if(currentBeepingAlarm->ipStreamURL > 0)																								
					connectToStreamWrapped(currentBeepingAlarm->ipStreamURL);
				else
					connectToStreamWrapped(1);		//TODO JOEY FIX STREAM
				
				printf("\nStarting up stream!   stream number = %d", currentBeepingAlarm->ipStreamURL);
				printf("\nNormal Alarm putting on stream");
				streamState = STREAM_RUNNING;			
			}
			else
			{			
																														//FOR TESTING IF STREAM NO WORK
				return;
			}
		}
		
		
		beepCount+=1;
}




void GetBeepingAlarm(Alarm **alarm)
{
	printf("\n Current :Alarm Time : %d:%d \n", currentBeepingAlarm->time->tm_min,currentBeepingAlarm->time->tm_sec);
	(*alarm) = currentBeepingAlarm;
}


// Snoozes the alarm for X seconds (should be minutes in practice)
void SnoozeAlarm(int snoozeTime, int minutes)
{
	
	if(snoozeTime > 0)
	{
		//printf("\n Current :Alarm Time : %d:%d \n", currentBeepingAlarm->time->tm_sec,currentBeepingAlarm->time->tm_min);
		beepCount = 0;
		
		if(minutes == SNOOZE_SECONDS)		// When increasing snoozeTime in seconds
		{
			if(currentBeepingAlarm->time->tm_sec <= 59 - snoozeTime)
			currentBeepingAlarm->time->tm_sec = currentBeepingAlarm->time->tm_sec + snoozeTime;
			else
			{
			currentBeepingAlarm->time->tm_min = currentBeepingAlarm->time->tm_min + 1;
			currentBeepingAlarm->time->tm_sec = currentBeepingAlarm->time->tm_sec - 60;
			}
		}
		else
		{
			if(currentBeepingAlarm->time->tm_min <= 59 - snoozeTime)
			currentBeepingAlarm->time->tm_min = currentBeepingAlarm->time->tm_min + snoozeTime;
			else
			{
			currentBeepingAlarm->time->tm_hour = currentBeepingAlarm->time->tm_hour + 1;
			currentBeepingAlarm->time->tm_min = currentBeepingAlarm->time->tm_min + snoozeTime - 60;
			}
		}
	}

}

/**
*	Save all alarms to flash memory
*/
int saveAllAlarms() {
	AlarmNode *currrent = AlarmList;
	int pageNumber = ALARMS_LIST_START_MEM; // #define @memdefs
	int result;
	
	while(currrent != NULL) {
        result = At45dbPageWrite(pageNumber, currrent->data, sizeof(Alarm));
        if(result == -1) {
        	printf("An error acceurd while saving an alram!!! page: %d\n", pageNumber);
        	return -1;
    	}

		//currrent = currrent->next;
		pageNumber++;
		
		result = At45dbPageWrite(pageNumber, currrent->data->time, sizeof(tm));
        if(result == -1) {
        	printf("An error acceurd while saving an alram!!! page: %d\n", pageNumber);
        	return -1;
    	}

		currrent = currrent->next;
		pageNumber++;
		
	}

	At45dbPageWrite(ALARMS_LIST_SIZE_MEM, &pageNumber, sizeof(int)); // save amount of alarms to flash memory
	return 0;
}

/**
*	read all alarms from flash memory
*/
int readAllAlarms() {
	int listSize;
	At45dbPageRead(ALARMS_LIST_SIZE_MEM, &listSize, sizeof(int)); // retrieve amount of alarms fro flash memory

	int pageNumber = ALARMS_LIST_START_MEM; // #define @memdefs

	int result;
	
	while(pageNumber < listSize) {
		Alarm *newAlarm = malloc(sizeof(Alarm));
		Alarm *alarmTime = malloc(sizeof(tm));
        result = At45dbPageRead(pageNumber, newAlarm, sizeof(Alarm));

        if(result == -1) {
        	printf("An error acceurd while reading an alram!!! page: %d\n", pageNumber);
        	return -1;
        }
		
		pageNumber++;
		
		result = At45dbPageRead(pageNumber, alarmTime, sizeof(tm));

        if(result == -1) {
        	printf("An error acceurd while reading an alram!!! page: %d\n", pageNumber);
        	return -1;
        }

		newAlarm->time = alarmTime;

		ListInsert(AlarmList, newAlarm);

		pageNumber++;
	}

	//At45dbPageRead(pageNumber, AlarmList->data, sizeof(Alarm));
	//AlarmList = start;

	return 0;
}
#ifndef _CLOCK_H_
#define	_CLOCK_H_

/*includes*/
#include <time.h>
#include "rtc.h"
//#include "system.h"
#include <string.h>
#include "log.h"
#include "memdefs.h"
#include "flash.h"
#include <sys/timer.h>
#include <pro/sntp.h>
#include <arpa/inet.h>
#include <time.h>
/*definitions*/
#define LOG_MODULE  LOG_MAIN_MODULE //only for debug

/*
	first initialize the time.
	then include clock.h and use initTimeZone();.
	now you can set the timezone
 */

/*prototypes*/

void initTimeZone(void);
int setClock(int sec, int min, int hour, int mday, int mon, int year, int wday, int yday, int dst);
int setTimeZone(int tn);
void calculateTime(int zn);
void printClock(void);
int getTimeZone(void);
int setTimeFromNTP();


#endif

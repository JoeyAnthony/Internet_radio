#include "clock.h"
#include "log.h"

int timeZone;

//set time in memory
void initTimeZone()
{
	At45dbPageRead(TIMEZONE_MEM, &timeZone, sizeof(int));
}

int setClock(int sec, int min, int hour, int mday, int mon, int year, int wday, int yday, int dst)
{
	tm set;
	set.tm_sec = sec;
	set.tm_min = min;
	set.tm_hour = hour;
	set.tm_mday = mday;
	set.tm_mon = mon;
	set.tm_year = year;
	set.tm_wday = wday;
	set.tm_yday = yday;
	set.tm_isdst = dst;
	
	return(X12RtcSetClock(&set));
}

//set a timezone and save in memory
int setTimeZone(int tn)
{
	if(tn < -11 || tn > 14)
	{
		return -1;
	}
	else
	{
		calculateTime(tn);
		timeZone = tn;
		At45dbPageWrite(TIMEZONE_MEM, &timeZone, sizeof(int));
		return 0;
	}
}

//get time from internet and add the timezone
void calculateTime(int tn)
{
	tm t;
	//when internet
	//--
	
	//when no connection
	int difference;
	difference = tn - timeZone;
	//LogMsg_P(LOG_INFO, PSTR("differencefromclockclass:: %i \n"), difference);
	
	if (X12RtcGetClock(&t) == 0)
    {
		//LogMsg_P(LOG_INFO, PSTR("getclockfromclockclass: [%02d:%02d:%02d]"), t.tm_hour, t.tm_min, t.tm_sec );
    }
	else
	{
		LogMsg_P(LOG_INFO, PSTR("Failed getting the time in clock.c"));
		return;
	}
	t.tm_hour += difference;
	//LogMsg_P(LOG_INFO, PSTR("Hoursaftertimezonechange: %i \n"), t.tm_hour);
	
	if(t.tm_hour > 23)
	{
		t.tm_hour -= 24;
	}
	if(t.tm_hour < 0)
	{
		t.tm_hour += 24;
	}
	
	
	
	X12RtcSetClock(&t);
}

//print clock in console
void printClock()
{
	tm gmt;
	if (X12RtcGetClock(&gmt) == 0)
		{
			LogMsg_P(LOG_INFO, PSTR("RTC time [%02d:%02d:%02d]"), gmt.tm_hour, gmt.tm_min, gmt.tm_sec );
			LogMsg_P(LOG_INFO, PSTR("RTC time [%02d/%02d/%02d]"), gmt.tm_mday, gmt.tm_mon, gmt.tm_year+1900 );
		}
		else
		{
			LogMsg_P(LOG_INFO, PSTR("Failed getting the time in clock.c"));
			return;
		}
}

int getTimeZone()
{
	return timeZone;
}

int setTimeFromNTP(){
	
	tm utctime;
	u_long addr = inet_addr("95.46.198.21");
	time_t tmpTime = 0;
	if(0 == NutSNTPGetTime(&addr, &tmpTime)){
		gmtime_r(&tmpTime, &utctime);
		utctime.tm_mon++;
		X12RtcSetClock(&utctime);
		return 0;
	}
	else{
		return -1;
	}
}

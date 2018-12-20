
///
#define LOG_MODULE  LOG_PLAYER_MODULE
#define true 1
#define false 0

#include <sys/heap.h>
#include <sys/bankmem.h>
#include <sys/thread.h>
#include <sys/timer.h>

#include "player.h"
#include "vs10xx.h"
#include "log.h"



#define OK			1
#define NOK			0

int threadStop = 1;
int hasConnection = false;

THREAD(StreamPlayer, arg);
THREAD(Mp3Player, arg);

int initPlayer(void)
{
	return OK;
}



void stopPlayer()
{
		threadStop = 1;
		VsPlayerStop();
}

void PLSetHasConnection(int value) {
	hasConnection = value;
}

int PLGetHasConnection() 
{
	return hasConnection;
}

int play(FILE *stream)
{
	threadStop = 0;
	NutThreadCreate("Bg", StreamPlayer, stream, 512);
	printf("Play thread created. Device is playing stream now !\n");	
		
	return OK;
}

int PlayFile(FILE *stream) 
{
	threadStop = 0;
	NutThreadCreate("filePlayer", Mp3Player, stream, 512);
	printf("File Play thread created. Device is playing stream now !\n");	
		
	return OK;
} 

THREAD(Mp3Player, arg)
{
	FILE *stream = (FILE*) arg;
	size_t rbytes = 0;
	char *mp3buf;
	int result = NOK;
	int nrBytesRead = 0;
	unsigned char iflag;
	
	//
	// Init MP3 buffer. NutSegBuf is een globale, systeem buffer
	//
	
	if( 0 != NutSegBufInit(8192) )
	{
		// Reset global buffer
		iflag = VsPlayerInterrupts(0);
		NutSegBufReset();
		VsPlayerInterrupts(iflag);
		
		result = OK;
	}
	
	// Init the Vs1003b hardware
	if( OK == result )
	{
		if( -1 == VsPlayerInit() )
		{
			LogMsg_P(LOG_INFO, PSTR("Retry init Vs1003b \n"));
			if( -1 == VsPlayerReset(0) )
			{
				result = NOK;
				LogMsg_P(LOG_INFO, PSTR("Failed init Vs1003b \n"));
			}
			else
			{
				LogMsg_P(LOG_INFO, PSTR("Vs1003b retry initialised! \n"));
			}
		}
		else
		{
			LogMsg_P(LOG_INFO, PSTR("Vs1003b initialised! \n"));
		}
	}
	

	if(threadStop)
	{
		NutThreadExit();
	}

	while(result)
	{
		/*
		 * Query number of byte available in MP3 buffer.
		 */
        iflag = VsPlayerInterrupts(0);
        mp3buf = NutSegBufWriteRequest(&rbytes);
        VsPlayerInterrupts(iflag);
		
		//LogMsg_P(LOG_INFO, PSTR("buffer"));
		
		if(threadStop)
		{
			LogMsg_P(LOG_INFO, PSTR("Exit thread"));
			NutThreadExit();
		}
		
		// Bij de eerste keer: als player niet draait maak player wakker (kickit)
		if( VS_STATUS_RUNNING != VsGetStatus() )
		{
			LogMsg_P(LOG_INFO, PSTR("Player not running\n"));
			VsPlayerKick();
			if( rbytes < 1024 )
			{
				LogMsg_P(LOG_INFO, PSTR("Kicking the player"));
				VsPlayerKick();
			}
		}
		
		while( rbytes )
		{
			
				//LogMsg_P(LOG_INFO, PSTR("streaming"));
				// Copy rbytes (van 1 byte) van stream naar mp3buf.
				
				
				nrBytesRead = fread(mp3buf, 1, rbytes, stream);
				LogMsg_P(LOG_INFO, PSTR("Player byte copied\n"));
				
				if( nrBytesRead > 0 )
				{
					iflag = VsPlayerInterrupts(0);
					mp3buf = NutSegBufWriteCommit(nrBytesRead);
					VsPlayerInterrupts(iflag);
					if( nrBytesRead < rbytes && nrBytesRead < 512 )
					{
						NutSleep(250);
					}
				}
				else
				{
					LogMsg_P(LOG_INFO, PSTR("break1"));
					hasConnection = false;
					break;
				}
				rbytes -= nrBytesRead;
				
				if( nrBytesRead <= 0 )
				{
					LogMsg_P(LOG_INFO, PSTR("break2"));
					break;
				}			
			
		}
	}
}

THREAD(StreamPlayer, arg)
{
	FILE *stream = (FILE *) arg;
	size_t rbytes = 0;
	char *mp3buf;
	int result = NOK;
	int nrBytesRead = 0;
	unsigned char iflag;
	
	//
	// Init MP3 buffer. NutSegBuf is een globale, systeem buffer
	//
	
	if( 0 != NutSegBufInit(8192) )
	{
		// Reset global buffer
		iflag = VsPlayerInterrupts(0);
		NutSegBufReset();
		VsPlayerInterrupts(iflag);
		
		result = OK;
	}
	
	// Init the Vs1003b hardware
	if( OK == result )
	{
		if( -1 == VsPlayerInit() )
		{
			LogMsg_P(LOG_INFO, PSTR("Retry init Vs1003b \n"));
			if( -1 == VsPlayerReset(0) )
			{
				result = NOK;
				LogMsg_P(LOG_INFO, PSTR("Failed init Vs1003b \n"));
			}
			else
			{
				LogMsg_P(LOG_INFO, PSTR("Vs1003b retry initialised! \n"));
			}
		}
		else
		{
			LogMsg_P(LOG_INFO, PSTR("Vs1003b initialised! \n"));
		}
	}
	

	if(threadStop)
	{
		NutThreadExit();
	}

	while(result)
	{
		/*
		 * Query number of byte available in MP3 buffer.
		 */
        iflag = VsPlayerInterrupts(0);
        mp3buf = NutSegBufWriteRequest(&rbytes);
        VsPlayerInterrupts(iflag);
		
		//LogMsg_P(LOG_INFO, PSTR("buffer"));
		
		if(threadStop)
		{
			LogMsg_P(LOG_INFO, PSTR("Exit thread"));
			NutThreadExit();
		}
		
		// Bij de eerste keer: als player niet draait maak player wakker (kickit)
		if( VS_STATUS_RUNNING != VsGetStatus() )
		{
			LogMsg_P(LOG_INFO, PSTR("Player not running\n"));
			if( rbytes < 1024 )
			{
				printf("VsPlayerKick()\n");
				VsPlayerKick();
			}
		}
		
		while( rbytes )
		{
			//LogMsg_P(LOG_INFO, PSTR("streaming"));
			// Copy rbytes (van 1 byte) van stream naar mp3buf.
			nrBytesRead = fread(mp3buf, 1, rbytes, stream);
			
			if( nrBytesRead > 0 )
			{
				iflag = VsPlayerInterrupts(0);
				mp3buf = NutSegBufWriteCommit(nrBytesRead);
				VsPlayerInterrupts(iflag);
				if( nrBytesRead < rbytes && nrBytesRead < 512 )
				{
					NutSleep(250);
				}
			}
			else
			{
				LogMsg_P(LOG_INFO, PSTR("break1"));
				hasConnection = false;
				break;
			}
			rbytes -= nrBytesRead;
			
			if( nrBytesRead <= 0 )
			{
				LogMsg_P(LOG_INFO, PSTR("break2"));
				break;
			}				
		}
	}
}





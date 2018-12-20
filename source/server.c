#include "server.h"
#include "log.h"
#define LOG_MODULE  LOG_MAIN_MODULE
//this lib was not included in nutos so it was done manually
#include "strtok.h"

const char delim[] = "#";

//Linked list of all microROM files. 
//It is externally declared in uroms.h and is needed here
//without this list the prog_char's won't work and the code will not compile
ROMENTRY *romEntryList;

THREAD(Service, arg);
 
void initServer()
{
   /* Register the file system device. */
   NutRegisterDevice(&devUrom, 0, 0);
/*
 * CGI Sample: Show request parameters.
 *
 * See server.h for REQUEST2 structure.
 * this structure is from an enhanced version of httpd.handle
 *
 * This routine must have been registered by NutRegisterCgi() and is
 * automatically called by NutHttpProcessRequest() when the client
 * request the URL 'cgi-bin/test.cgi'.
 */


    /*
     * Register our CGI sample. This will be called
     * by http://host/cgi-bin/form.cgi?anyparams
     */
	 
	 //so a cgi can be called from a webcommand. With registerCgi you can link a cgi to a function.
	 //the parameters for the function contain information about the webcommand and the stream.
	 
	 //cgi's are gegistered here and the server will then start on a different thread
	if( NutRegisterCgi("form.cgi", SendForm) == -1)
	{
		 printf("failed\n");
	}
	 
	if( NutRegisterCgi("radio.cgi", setRadioStream) == -1)
	{
		 printf("failed\n");
	}
	
	if( NutRegisterCgi("radionr.cgi", setRadioStreamNr) == -1)
	{
		 printf("failed\n");
	}
   
	if( NutRegisterCgi("alarm.cgi", setAlarm) == -1)
	{
		 printf("failed\n");
	}
	if( NutRegisterCgi("clock.cgi", setClockBrowser) == -1)
	{
		 printf("failed\n");
	}
	 
 	NutThreadCreate("listen", Service, NULL, HTTPD_SERVICE_STACK);
	printf("Server thread created \n");
}

//send HTML front page to client 
int SendForm(FILE * stream, REQUEST2 * req) 
{
    /*
	 * The special type 'prog_char' forces the
     * string literals to be placed in flash ROM. This saves us a lot of
     * precious RAM.
     */
	
	//vars containing the html code to open the frontpage
    static prog_char foot[] = "</BODY></HTML>";
    static prog_char body[] = "<HTML><HEAD><TITLE>Internetradio</TITLE></HEAD><BODY><iframe src=\"http://administratiemjanse.nl/ti_ipac_radio/index.html\?ip=%s&time=%02d:%02d&latest_alarm=%02d:%02d&latest_schedular=-&stream=%s\" width=\"100%%\" height=\"97%%\" frameborder=\"0\" scrolling=\"no\"></iframe>";


    /* Send HTML header. */
	tm tim;
	Alarm a;
	X12RtcGetClock(&tim);
	if(1 == getNextActiveAlarmToday(&a, &tim)){
	LogMsg_P(LOG_INFO, PSTR("Latest alarm: %i:%i"), a.time->tm_hour, a.time->tm_min);}
	

	/*
     * Include parameters in de URL, this parameters will be read out by the website to show the ip,, latest alarm etc.
    */	
	fprintf_P(stream, body, inet_ntoa(confnet.cdn_ip_addr), tim.tm_hour, tim.tm_min, a.time->tm_hour, a.time->tm_min, getStreamName() );

    /* Send HTML footer and flush output buffer. */
    fputs_P(foot, stream);
    fflush(stream);

    return 0;
} 

//get an ip radiostation from the front page and play it
int setRadioStream(FILE * stream, REQUEST2 * req)
{
	char *buf;
	buf = processForm(stream, req);
	
	//split buffer
	char * bad = strtok(buf, delim);
	char *ip = strtok(NULL, delim);
	int *port = (strtok(NULL, delim));
	char *url = strtok(NULL, delim);
	
	//printf("ip in server: %s %s %i %s \n", bad, ip, port, url);
	connectToStreamFromInet(ip, atoi(port), url);
	free(buf);
}

//put a pre programmed radio station on
int setRadioStreamNr(FILE * stream, REQUEST2 * req)
{
	char *buf;
	buf = processForm(stream, req);
	
	//split buffer
	char * bad = strtok(buf, delim);
	int *nr = atoi(strtok(NULL, delim));
	connectToStream(nr);
	free(buf);
}

//get time and timezon from frontpage and set the clock in the radio
void setClockBrowser(FILE * stream, REQUEST2 * req)
{
	char *buf;
	buf = processForm(stream, req);
	
	//split buffer
	char * bad = strtok(buf, delim);
	int *hour = atoi(strtok(NULL, delim));
	int *min = atoi(strtok(NULL, delim));
	int *tz = atoi(strtok(NULL, delim));
	
	tm t;
	if (X12RtcGetClock(&t) == 0)
    {
		//failed
    }
	
	setTimeZone(tz);
	
	t.tm_hour = hour;
	t.tm_min = min;
	X12RtcSetClock(&t);
}

//set an alarm by webinterface, schedular is also set with this function
void setAlarm(FILE * stream, REQUEST2 * req)
{
	char *buf;
	buf = processForm(stream, req);
	
	//split buffer
	int msg = strtok(buf, delim);
	int isEnabled = atoi(strtok(NULL, delim));
	int snoozeTime = atoi(strtok(NULL, delim));
	int hasGame = atoi(strtok(NULL, delim));
	int isSchedular = atoi(strtok(NULL, delim));
	int hours = atoi(strtok(NULL, delim));
	int min = atoi(strtok(NULL, delim));
	int days = atoi(strtok(NULL, delim));
	int radionr = atoi(strtok(NULL, delim));
	int alarmTone = atoi(strtok(NULL, delim));
		
	//printf("By internet alarm set: %s %i %i %i %i %i %i %i %i %i \n", msg, isEnabled, snoozeTime, hasGame, isSchedular, hours, min, days, radionr, alarmTone);
	
	//make a time object from the hour and minute int
	tm *time;
	time  = malloc(sizeof(tm));
	time->tm_hour = hours;
	time->tm_min = min;
	time->tm_sec = 0;
	
	AddAlarm(isEnabled,snoozeTime,hasGame,isSchedular,time,days,radionr,alarmTone);
	free(buf);
}

//get a post command from a client , decode it and put the characters in a buffer
char* processForm(FILE * stream, REQUEST2 * req) 
{
	printf("Processing form \n");
	printf("METHOD: %i \n",req->req_method);
	if (req->req_method == METHOD_POST) {
		printf("IS POST \n");
       int rc;
       int sz = req->req_length;
       char *buf = malloc(sz + 1);
       /* Read the complete contents. */
       while (sz > 0) {
           /* Get the next parameter. */
           rc = GetEncodedString(stream, buf, sz, '=');
           if (rc <= 0) {
               /* Failed to read encoded string. */
			   printf("Failed to read encoded string 1\n");
			   break;
           }
		   printf("Encoded 1 \n");
          sz -= rc;
		}
		free(buf);
		return buf;
		
	}
	
}


//listen thread for the server, can only handle one connection
THREAD(Service, arg)
{
    TCPSOCKET *sock;
    FILE *stream;


    /*
     * Now loop endless for connections.
     */
    for (;;) {
        /* Create a socket. */
		printf("starting server \n");
        sock = NutTcpCreateSocket();
        if (sock == NULL) {
            printf("Failed to create socket");
            NutSleep(1000);
            continue;
        }

        /* Listen on port 80. NutTcpAccept() will block
           until we get a connection from a client. */
        printf("Listening...\ \n");
		NutSleep(3000);
        NutTcpAccept(sock, 80);
        printf("Connected... \n");

        /* Associate a binary stdio stream with the socket. */
        stream = _fdopen((int) ((uintptr_t) sock), "r+b");
        if (stream == NULL) {
            printf("Failed to open stream for client");
        } else {
            /* This API call saves us a lot of work. It will parse
               the client's HTTP request, send the requested file. */
            NutHttpProcessRequest(stream);

            /* Destroy the associated stream. */
            fclose(stream);
        }

        /* Close the socket. */
        NutTcpCloseSocket(sock);
        //puts("Disconnected");
    }
    return 0;
}

//Below here are functions used to read and decode a post command from a stream

/*brief Read encoded string from stream.
*
* This function reads a specified number of characters from a stream,
* but not beyond a given end-of-string character.
*
* \param stream The stream to read from.
* \param str    Pointer to a buffer that receives the decoded string.
*               The string will be terminated with ASCII 0.
* \param siz    Maximum number of characters to read.
* \param eos    End of string character.
*
* \return Number of characters read or -1 on errors.
*/

static int GetEncodedString(FILE * stream, char *str, int siz, int eos) {
   int ch;
   int rc;
   for (rc = 0; rc < siz; rc++) {
       ch = fgetc(stream);
       if (ch == EOF) {
           return -1;
       }
       if (ch == eos) {
           rc++;
           break;
       }
       if (ch == '+') {
           ch = ' ';
       } else if (ch == '%') {
           if (rc > siz - 3) {
               return -1;
           }
           ch = GetEncodedHexChar(stream);
           if (ch == EOF) {
               return -1;
           }
           rc += 2;
       }
       if (str) {
           *str++ = (char) ch;
       }
   }
   if (str) {
       *str = '\0';
   }
   return rc;
}

/*!
* \brief Read hex-encoded character from stream.
*
* \param stream The stream to read from.
*
* \return Decoded character or EOF on errors.
*/
static int GetEncodedHexChar(FILE * stream) {
   int i;
   int c = 0;
   int ch;
   for (i = 0; i < 2; i++) {
       c <<= 4;
       ch = fgetc(stream);
       if (ch >= '0' && ch <= '9') {
           c |= ch - '0';
       } else if (ch >= 'A' && ch <= 'F') {
           c |= ch - ('A' - 10);
       } else if (ch >= 'a' && ch <= 'f') {
           c |= ch - ('a' - 10);
       } else {
           /* Bad hex code. */
           return EOF;
       }
   }
   return c;
}

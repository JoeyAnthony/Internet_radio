
#define LOG_MODULE  LOG_MAIN_MODULE

#include <sys/thread.h>
#include <sys/timer.h>

#include <dev/nicrtl.h>
#include <arpa/inet.h>
#include <sys/confnet.h>
#include <pro/dhcp.h>


#include "log.h"
#include "shoutcast.h"

#include <sys/socket.h>
#include "player.h"


#define ETH0_BASE	0xC300
#define ETH0_IRQ	5

// number * 1000 = amount of seconds
#define TIMEOUT_TIME  1000

#define OK			1
#define NOK			0
#define TRUE		1
#define false		0
#define TCP_MAXSEG 1000

// #define ADRESS "178.254.19.208" 
// #define RADIO_URL "/"
// #define PORT 6100



static char eth0IfName[9] = "eth0";
FILE *stream;
TCPSOCKET *sock;
int isDeviceRegistered = false;
int isLoadingStream = 0; 
int currentStream = -1;
THREAD(connectToStreamThread, arg);
radio IPStreams[5];

void initRadioStreams()
{
	//0 is reserved for internet streams
	
	int i = 1;
	//radio1
	strcpy(IPStreams[i].name, "Radio 1");
	strcpy(IPStreams[i].ip, "145.58.52.144");
	IPStreams[i].port = 80;
	strcpy(IPStreams[i].radioURL, "/radio1-bb-mp3");
	i++;
	
	//radio2
	strcpy(IPStreams[i].name, "Radio 2");
	strcpy(IPStreams[i].ip, "145.58.52.147");
	IPStreams[i].port = 80;
	strcpy(IPStreams[i].radioURL, "/radio2-bb-mp3");
	i++;
	
	//3fm
	strcpy(IPStreams[i].name, "3FM");
	strcpy(IPStreams[i].ip, "145.58.52.150");
	IPStreams[i].port = 80;
	strcpy(IPStreams[i].radioURL, "/3fm-bb-mp3");
	i++;
	
	//radio veronica
	strcpy(IPStreams[i].name, "Radio Veronica");
	strcpy(IPStreams[i].ip, "213.75.57.116");
	IPStreams[i].port = 80;
	strcpy(IPStreams[i].radioURL, "/VERONICA_SC");
	i++;
	
	//radio538
	strcpy(IPStreams[i].name, "Radio 538");
	strcpy(IPStreams[i].ip, "37.48.116.150");
	IPStreams[i].port = 80;
	strcpy(IPStreams[i].radioURL, "/RADIO538_MP3");
	i++;
	
	
}

int initInet(void)
{
	uint8_t mac_addr[6] = { 0x00, 0x06, 0x98, 0x30, 0x02, 0x76 };
	
	int result = OK;
	if(!isDeviceRegistered)
	{
		// Registreer NIC device (located in nictrl.h)
		if(0 != NutRegisterDevice(&DEV_ETHER, ETH0_BASE, ETH0_IRQ))
		{
			LogMsg_P(LOG_INFO, PSTR("Error: >> NutRegisterDevice()"));
			result = NOK;
		}
		else
		{
			LogMsg_P(LOG_INFO, PSTR("Device Registered initInet"));
			isDeviceRegistered = TRUE;
		}
	}
	
	if( OK == result )
	{
		//if(0 != NutDhcpIfConfig(eth0IfName, mac_addr, 0))
		if(0 != NutDhcpIfConfig(eth0IfName, mac_addr, 30000))
		{
			LogMsg_P(LOG_INFO, PSTR("Error: >> NutDhcpIfConfig()"));
			result = NOK;
		}
		else
		{
			LogMsg_P(LOG_INFO, PSTR("DHCP configured"));
		}
	}
	
	if( OK == result )
	{
		LogMsg_P(LOG_INFO, PSTR("Networking setup OK, new settings are:\n") );	

		LogMsg_P(LOG_INFO, PSTR("if_name: %s"), confnet.cd_name);	
		LogMsg_P(LOG_INFO, PSTR("ip-addr: %s"), inet_ntoa(confnet.cdn_ip_addr) );
		LogMsg_P(LOG_INFO, PSTR("ip-mask: %s"), inet_ntoa(confnet.cdn_ip_mask) );
		LogMsg_P(LOG_INFO, PSTR("gw     : %s"), inet_ntoa(confnet.cdn_gateway) );
	}
	
	NutSleep(1000);
	
	return result;
}


int connectToStream(int radionr)
{
	if(isLoadingStream == 0)
	{
		isLoadingStream = 1;
		NutThreadCreate("radioConnection", connectToStreamThread, radionr, 256);
		printf("stream connectionthread started \n");
		currentStream = radionr;
	}
		
		
	return OK;
}

int connectToStreamFromInet(char *ip, int *port, char *url)
{
	if(isLoadingStream == 0)
	{
		isLoadingStream = 1;
		printf("ip in shoutast: %s %i %s \n", ip, port, url);
		
		strcpy(IPStreams[0].name, "Browser Stream");
		strcpy(IPStreams[0].ip, ip);
		IPStreams[0].port = port;
		strcpy(IPStreams[0].radioURL, url);
			
		NutThreadCreate("radioConnection", connectToStreamThread, 0, 256);
		printf("stream connectionthread started \n");
	}
		
		
	return 1;
}

THREAD(connectToStreamThread, arg)
{
	stopStream();
	
	int radionr = (int) arg;
	char *data;
	int result = OK;
	u_short mss = 1460;
	
	 NutTcpSetSockOpt(sock, TCP_MAXSEG, &mss,
	 sizeof(mss));

	sock = NutTcpCreateSocket();
	
	printf("%s %s : %i \n", &IPStreams[radionr].ip, &IPStreams[radionr].radioURL, &IPStreams[radionr].port);
	
	if( 0 != NutTcpConnect(	sock,
						inet_addr(&IPStreams[radionr].ip), 
						IPStreams[radionr].port) )
	{
		LogMsg_P(LOG_ERR, PSTR("Error: >> NutTcpConnect()\n"));
		return 0;
	}
	stream = _fdopen( (int) sock, "r+b" );
	
    fprintf(stream, "GET %s HTTP/1.0\r\n", &IPStreams[radionr].radioURL);
    fprintf(stream, "Host: %s\r\n", &IPStreams[radionr].ip);
    fprintf(stream, "User-Agent: Ethernut\r\n");
    fprintf(stream, "Accept: */*\r\n");
	fprintf(stream, "Icy-MetaData: 1\r\n");
    fprintf(stream, "Connection: close\r\n\r\n");
    fflush(stream);

	
	// Server stuurt nu HTTP header terug, catch in buffer
	data = (char *) malloc(512 * sizeof(char));
	
	while( fgets(data, 512, stream) )
	{
		if( 0 == *data )
			break;
    
		printf("%s", data);
	}
	
	free(data);
	NutSleep(1000);
	
	playStream();
	
	isLoadingStream = 0;
	NutThreadExit();
}

int playStream(void)
{
	play(stream);
	
	return OK;
}

int stopStream(void)
{
		stopPlayer();
		//NutTcpDiscardBuffers(sock);
		NutTcpCloseSocket(sock);
		fclose(stream);
	return OK;
}

char* getStreamName() {
	if(currentStream == -1 || currentStream > 5)
		return "-";
	else{
		return IPStreams[currentStream].name;
	}
}


/**
*	get the status of the internet
	Returns:
	DHCP status code, which may be any of the following:
	1 = DHCPST_INIT		// no internet
	2 =	DHCPST_SELECTING
	3 =	DHCPST_REQUESTING
	4 =	DHCPST_REBOOTING
	5 =	DHCPST_BOUND	// has internet connection
	6 =	DHCPST_RENEWING 
	7 =	DHCPST_REBINDING
	8 =	DHCPST_INFORMING
	9 =	DHCPST_RELEASING
	0 =	DHCPST_IDLE
*/
int getStatusInternet() {
	int result = NutDhcpStatus(eth0IfName);
	//printf("Internet status: %i\n", result);
	return result;
}
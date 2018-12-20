#ifndef SHOUTCAST_INC
#define SHOUTCAST_INC

int initInet(void);
int connectToStream(int radionr);
int connectToStreamFromInet(char *ip, int *port, char *url);
int playStream(void);
int stopStream(void);
void initRadioStreams();
char* getStreamName();

typedef struct radioStream {
	char name[25];
	char ip[25];
	int port;
	char radioURL[25];
} radio;

#endif

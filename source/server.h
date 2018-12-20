
#ifndef _SERVER_H_
#define	_SERVER_H_

// #ifndef httpd_service_stack
// #if defined(__avr__)
// #define httpd_service_stack ((580 * nut_thread_stack_mult) + nut_thread_stack_add)
// #elif defined(__arm__)
// #define httpd_service_stack ((1024 * nut_thread_stack_mult) + nut_thread_stack_add)
// #else
// #define httpd_service_stack ((2048 * nut_thread_stack_mult) + nut_thread_stack_add)
// #endif
// #endif

#include <cfg/os.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>

#include <dev/board.h>
#include <dev/urom.h>
#include <dev/nplmmc.h>
#include <dev/sbimmc.h>
#include <fs/phatfs.h>

#include <sys/version.h>
#include <sys/thread.h>
#include <sys/timer.h>
#include <sys/heap.h>
#include <sys/confnet.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <net/route.h>

#include <pro/httpd.h>
#include <pro/dhcp.h>
#include <pro/ssi.h>
#include <pro/asp.h>
#include <pro/discover.h>

#include <fs/fs.h>
#include <fs/uromfs.h>

#include "alarmcontroller.h"
#include "shoutcast.h"
#include "clock.h"

/* Server thread stack size. */
#define HTTPD_SERVICE_STACK 1024

typedef struct _REQUEST2 REQUEST2;
/*!
 * \struct _REQUEST httpd.h pro/httpd.h
 * \brief HTTP request information structure.
 */
struct _REQUEST2 {
    int req_method;             /*!< \brief Request method. */
    int req_version;            /*!< \brief 11 = HTTP/1.1, 10 = HTTP/1.0 */
    long req_length;            /*!< \brief Content length */
    char *req_url;              /*!< \brief URI portion of the GET or POST request line */
    char *req_query;            /*!< \brief Argument string. */
    char *req_type;             /*!< \brief Content type. */
    char *req_cookie;           /*!< \brief Cookie. */
    char *req_auth;             /*!< \brief Authorization info. */
    char *req_agent;            /*!< \brief User agent. */
    char **req_qptrs;           /*!< \brief Table of request parameters */
    int req_numqptrs;           /*!< \brief Number of request parameters */
    time_t req_ims;             /*!< \brief If-modified-since condition. */
    char *req_referer;          /*!< \brief Misspelled HTTP referrer. */
    char *req_host;             /*!< \brief Server host. */
    int req_connection;         /*!< \brief Connection type, HTTP_CONN_. */
};


int SendForm(FILE * stream, REQUEST2 * req);
char* processForm(FILE * stream, REQUEST2 * req);
int setRadioStream(FILE * stream, REQUEST2 * req);
int setRadioStreamNr(FILE * stream, REQUEST2 * req);
void setClockBrowser(FILE * stream, REQUEST2 * req);
void setAlarm(FILE * stream, REQUEST2 * req);
void getAlarms(FILE * stream, REQUEST2 * req);
static int GetEncodedString(FILE * stream, char *str, int siz, int eos);
static int GetEncodedHexChar(FILE * stream);
void initServer();

#endif

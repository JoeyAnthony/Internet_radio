/* ========================================================================
 * [TITLE]      Controller for the alarm
 * [FILE]       alarmcontroller.h
 * [VSN]        1.0
 * ======================================================================== */

#ifndef _LLIST_H
#define _LLIST_H

#include "alarmcontroller.h"

/*-------------------------------------------------------------------------*/
/* global defines                                                          */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* typedefs & structs                                                      */
/*-------------------------------------------------------------------------*/
typedef struct nodeStruct {
   Alarm *data;
   struct nodeStruct *next;
} AlarmNode;

typedef struct nodeStructChar {
   char *data;
   int size;
   struct nodeStructChar *next;
} TextNode;

/*--------------------------------------------------------------------------*/
/*  Global variables                                                        */
/*--------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* export global routines (interface)                                      */
/*-------------------------------------------------------------------------*/
void ListInsert(AlarmNode*, Alarm*);
void ListPrint(AlarmNode*);
void ListInsertText(TextNode**, char*, int);
void ListChangeLastText(TextNode*,char*,int);
void ListPrintAlarmNodes(AlarmNode*);

#endif
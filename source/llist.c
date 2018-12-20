#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/timer.h>
#include <sys/event.h>
#include <sys/thread.h>
#include <sys/heap.h>

#include "system.h"
#include "portio.h"
#include "llist.h"
#include "log.h"
#include "rtc.h"
//#include "alarmcontroller.h"



void ListInsert(AlarmNode* head, Alarm *newData) 
{	
	// Eerste element 
	if(head->data == NULL)                            				//Ga alleen verder als de lijst niet leeg is
    {
		head->next = NULL;
		head->data = newData;
		return;
    }

    AlarmNode *current = head;
    while(current->next != NULL)                //Zolang we nog niet aan het einde van de lijst zijn
    {
        current = current->next;
    }

    current->next = (AlarmNode*) malloc(sizeof(AlarmNode));     //Alloceer geheugen voor een nieuwe node
	current->next->data = newData;         //Zet de data van de nieuwe node
    current->next->next = NULL;                 //Nieuwe node wijst naar geen een node, oftewel einde van de lijst
}

void ListPrint(AlarmNode* head)
{
	AlarmNode *current = head;
	int cnt = 0;
	
	while(current != NULL)
	{
		if(current->data != NULL)
		{
			printf("\n %d - Alarm time: %02d:%02d:%02d \t| isEnabled: %i \t| snoozeTime: %i \t| hasGame: %i \t| isShedular: %i\n", cnt, current->data->time->tm_hour, current->data->time->tm_min,current->data->time->tm_sec, current->data->isEnabled, current->data->snoozeTime, current->data->hasGame, current->data->isShedular);
		}
		
		current = current->next;
		cnt++;
	}
}

void ListInsertText(TextNode ** head, char *newData, int sizeArray) {

	TextNode * new_node;
    new_node = malloc(sizeof(TextNode));

    new_node->data = newData;
	new_node->size = sizeArray;
    new_node->next = *head;
    *head = new_node;
	
}

void ListChangeLastText(TextNode *head, char *newData, int sizeArray){
	
	TextNode *current = head;
    while(current->next != NULL)                //Zolang we nog niet aan het einde van de lijst zijn
    {
        current = current->next;
    }

	current->data = newData;
	current->size = sizeArray;
}

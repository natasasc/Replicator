#ifndef _LIST_H
#define _LIST_H

#include "Message.h"


typedef struct listitem {
	messageStruct data;
	struct listitem* next;
} Listitem;

typedef struct list {
	Listitem* head;
} List;

void initlist(List*); /* initialize an empty list */
void insertfront(List*, messageStruct val); /* insert val at front */
void insertback(List*, messageStruct val); /* insert val at back */
int length(List); /* returns list length */
void destroy(List*); /* deletes list */
void setitem(List*, int n, messageStruct val);/* modifies item at n to val*/
bool exists(List*, int id);
//messageStruct getitem(List, int n); /* returns value at n*/
void displaylist(List*); /* displays the entire list*/

#endif /* _LIST_H */
#include "pch.h"

#include <stdio.h>
#include <stdlib.h>
#include "list.h"

void initlist(List* ilist) {
	ilist = (List*)malloc(sizeof(List));
	ilist->head = 0;
}

void insertfront(List* ilist, messageStruct val) {
	Listitem* newitem;
	newitem = (Listitem*)malloc(sizeof(Listitem));
	newitem->next = ilist->head;
	newitem->data = val;
	ilist->head = newitem;
}

void insertback(List* ilist, messageStruct val) {
	Listitem* ptr;
	Listitem* newitem;
	newitem = (Listitem*)malloc(sizeof(Listitem));
	newitem->data = val;
	newitem->next = 0;
	if (!ilist->head) {		// ako je lista prazna
		ilist->head = newitem;
		return;
	}
	ptr = ilist->head;
	while (ptr->next)
	{
		ptr = ptr->next;
	}
	ptr->next = newitem;	// moramo postaviti pokazivac next od poslednjeg elementa na novi
}

int length(List ilist) { /* returns list length */
	Listitem* ptr;
	int count = 1;
	if (!ilist.head) return 0;
	ptr = ilist.head;
	while (ptr->next) {
		ptr = ptr->next;
		count++;
	}
	return count;
}

void destroy(List* ilist) { /* deletes list */
	Listitem* ptr1,
		* ptr2;
	if (!ilist->head)
	{
		free(ilist);
		return;
	}
	ptr1 = ilist->head; /* destroy one by one */
	while (ptr1) {
		ptr2 = ptr1;
		ptr1 = ptr1->next;
		free(ptr2);
	}
	ilist->head = 0;
	free(ilist);
}

void setitem(List* ilist, int n, messageStruct val) {
	/* modifies a value*/
	/* assume length is at least n long */
	Listitem* ptr;
	int count = 0;
	if (!ilist->head) return;
	ptr = ilist->head;
	for (count = 0; count < n; count++)		// provera da li je n > br elemenata iz liste
	{
		if (ptr) 
			ptr = ptr->next;
		else 
			return;
	}
	if (ptr)
		ptr->data = val;
}

bool exists(List* ilist, int id)
{
	Listitem* ptr;
	if (!ilist->head) 
		return false;
	ptr = ilist->head;
	if (id == ptr->data.ID)
		return true;
	while (ptr->next) {
		ptr = ptr->next;
		if (id == ptr->data.ID)
			return true;
	}
	return false;
}

//messageStruct getitem(List ilist, int n) {
//	/* returns a list value,
//	* assume length is at least n long */
//	Listitem* ptr;
//	int count = 0;
//	if (!ilist.head) 
//		return NULL;
//	ptr = ilist.head;
//	if (n == 0) 
//		return ptr->data;
//	while (ptr->next) {
//		ptr = ptr->next;
//		count++;
//		if (n == count)
//			return (ptr->data);
//	}
//	return NULL;
//}

void displaylist(List* list) {
	List* temp;
	printf("IDs of registered clients:\n");
	if (list == NULL)
	{
		printf(" \tList is empty.");
	}
	else
	{
		temp = list;
		while (temp->head != NULL)
		{
			printf("\t%d ", temp->head->data.ID);       // prints the data of current node
			temp->head = temp->head->next;         // advances the position of current node
		}
	}
}

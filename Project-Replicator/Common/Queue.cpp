#include "pch.h"

#include <stdlib.h>
#include <stdio.h>
#include "Queue.h"


void ringBufGet(RingBuffer* apBuffer, messageStruct* msg) {
	//int index;
	//index = apBuffer->head;
	//apBuffer->head = (apBuffer->head + 1) % RING_SIZE;
	//messageStruct ret = apBuffer->data[index];
	//return ret;

	int next;

	if (apBuffer->head == apBuffer->tail)  // if the head == tail, we don't have any data
		return;

	next = apBuffer->tail + sizeof(messageStruct);  // next is where tail will point to after this read.
	//if (next >= apBuffer->maxlen)
	//	next = 0;

	*msg = apBuffer->data[apBuffer->tail];  // Read data and then move
	apBuffer->tail = next;              // tail to next offset.
	return;  // return success to indicate successful push.
}

void ringBufPut(RingBuffer* apBuffer, messageStruct msg) {
	//apBuffer->data[apBuffer->tail] = msg;
	//apBuffer->tail = (apBuffer->tail + 1) % RING_SIZE;

	int next;

	next = apBuffer->head + sizeof(messageStruct);  // next is where head will point to after this write.
	//if (next >= c->maxlen)
	//	next = 0;

	if (next == apBuffer->tail)  // if the head + 1 == tail, circular buffer is full
		return;

	apBuffer->data[apBuffer->head] = msg;  // Load data and then move
	apBuffer->head = next;             // head to next data offset.
	return;  // return success to indicate successful push.
}

void initRingBuffer(struct RingBuffer* rb)
{
	rb->head = 0;
	rb->tail = 0;
	rb->data = (messageStruct*)malloc(RING_SIZE * sizeof(messageStruct));
}

void destroyRingBuffer(struct RingBuffer* rb)
{
	free(rb->data);
	free(rb);
}
#include "Message.h"
#define RING_SIZE 500

// Kruzni bafer - FIFO
struct RingBuffer {
	unsigned int tail;
	unsigned int head;
	messageStruct* data;
};
// Operacije za rad sa kruznim baferom
void ringBufGet(RingBuffer* apBuffer, messageStruct* msg);
void ringBufPut(RingBuffer* apBuffer, messageStruct msg);
void initRingBuffer(RingBuffer* rb);
void destroyRingBuffer(RingBuffer* rb);
#pragma once
#include <winsock2.h>
#include "Threads.h"

struct Params {
	bool* doWork;
	SOCKET listenSocket;
	SOCKET clientSocket;
	SOCKET connectSocket;
	SOCKET replicatorSocket;
	RingBuffer* sendBuffer;
	RingBuffer* recvBuffer;
	List* clientData;
	CRITICAL_SECTION csSendBuffer;
	CRITICAL_SECTION csRecvBuffer;
	CRITICAL_SECTION csClientData;
	HANDLE hSemaphoreSend;
	HANDLE hSemaphoreRecv;
};
#pragma once
#include <winsock2.h>
#include "Threads.h"

struct Params {
	bool* doWork;
	SOCKET listenSocket;		// listen socket replikatora 1
	SOCKET clientSocket;		// soket za komunikaciju sa klijentom
	SOCKET connectSocket;		// socket za prijem sa replikatora 2	
	SOCKET replicatorSocket;	// socket za slanje na replikator 2
	RingBuffer* sendBuffer;
	RingBuffer* recvBuffer;
	List* clientData;			// lista za podatke koji stizu sa drugog klijenta (Client2 - SendData i Client1 - ReceiveData)
	CRITICAL_SECTION csSendBuffer;
	CRITICAL_SECTION csRecvBuffer;
	CRITICAL_SECTION csClientData;
	HANDLE hSemaphoreSend;
	HANDLE hSemaphoreRecv;
};
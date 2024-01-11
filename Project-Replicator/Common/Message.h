#pragma once
#define MESSAGE_H_

#include <ws2tcpip.h>
#include <stdio.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define DATA_SIZE 50

struct messageStruct {
	int ID;
	char data[DATA_SIZE];
	int dataSize;
};
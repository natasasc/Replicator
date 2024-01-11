#pragma once

#include <ws2tcpip.h>
#include <stdio.h>
#include "conio.h"

#include "../Common/Queue.h"
#include "../Common/List.h"
#include "Structures.h"

#define BUFFER_SIZE 256

extern List* RegisteredClients;
extern CRITICAL_SECTION csRegClients;

DWORD WINAPI ClientSend(LPVOID lpParam);
DWORD WINAPI Send(LPVOID lpParam);
DWORD WINAPI Receive(LPVOID lpParam);
DWORD WINAPI ClientReceive(LPVOID lpParam);
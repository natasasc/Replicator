// Client1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include "conio.h"
#include "../Common/Message.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#pragma pack(1)

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 27017		// u klijentu 1 ide 27016
#define BUFFER_SIZE 256

DWORD WINAPI handleIncomingData(LPVOID lpParam);
CRITICAL_SECTION csScreen;

void Menu();


int main()
{
	InitializeCriticalSection(&csScreen);

	// Socket za komunikaciju sa serverom
	SOCKET connectSocket = INVALID_SOCKET;

	// Promenljiva u kojoj ce biti smestena povratna vrednost funkcija
	int iResult;

	// WSADATA struktura podataka koja prima detalje Windows Sockets implementation
	WSADATA wsaData;

	// Inicijalizacija windows sockets biblioteke za ovaj proces
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	// kreiranje socket-a
	connectSocket = socket(AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP);

	if (connectSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Kreiranje i inicijalizacija address strukture
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;								// IPv4 protokol
	serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);	// ip adresa servera
	serverAddress.sin_port = htons(SERVER_PORT);					// port servera

	// Povezivanje na server opisan u serverAddress i connectSocket
	iResult = connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
	if (iResult == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	unsigned long mode = 1; //non-blocking mode
	iResult = ioctlsocket(connectSocket, FIONBIO, &mode);
	if (iResult != NO_ERROR)
		printf("ioctlsocket failed with error: %ld\n", iResult);

	DWORD funId;
	HANDLE handle;

	handle = CreateThread(NULL, 0, &handleIncomingData, &connectSocket, 0, &funId);

	//promenljiva tipa messageStruct cija ce se polja popunuti i cela struktira poslati u okviru jedne poruke
	messageStruct message;

	char id[3];
	EnterCriticalSection(&csScreen);
	printf("Unesite ID: ");
	LeaveCriticalSection(&csScreen);
	gets_s(id, 3);
	message.ID = htonl(atoi(id));

	int length;


	while (true)
	{
		Sleep(1000);

		Menu();

		int function = 0;
		switch (_getch())
		{
		case '1':
			function = 1;
			break;
		case '2':
			function = 2;
			break;
		case '3':
			function = 3;
			break;
		case '4':
			function = 4;
			break;
		default:
			EnterCriticalSection(&csScreen);
			printf("Please enter number 1, 2, 3 or 4.\n\n");
			LeaveCriticalSection(&csScreen);
		}

		if (function == 1)
		{
			char data[DATA_SIZE] = "register";
			memcpy(message.data, data, strlen(data) + 1);

			length = strlen(data);
			message.dataSize = htonl(length);  //obavezna funkcija htond() jer cemo slati podatak tipa int 

			// Slanje pripremljene poruke zapisane unutar strukture messageStruct
			// prosledjujemo adresu promenljive message u memoriji, jer se na toj adresi nalaze podaci koje saljemo
			// kao i velicinu te strukture (jer je to duzina poruke u bajtima)
			iResult = send(connectSocket, (char*)&message, (int)sizeof(messageStruct), 0);

			// Provera rezultata send funkcije
			if (iResult == SOCKET_ERROR)
			{
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(connectSocket);
				WSACleanup();
				return 1;
			}

			EnterCriticalSection(&csScreen);
			printf("Message successfully sent. Total bytes: %ld\n", iResult);
			LeaveCriticalSection(&csScreen);
		}
		else if (function == 2)
		{
			// Unos potrebnih podataka koji ce se poslati serveru
			EnterCriticalSection(&csScreen);
			printf("Enter message data: ");
			LeaveCriticalSection(&csScreen);
			gets_s(message.data, DATA_SIZE);

			length = strlen(message.data);
			message.dataSize = htonl(length);  //obavezna funkcija htond() jer cemo slati podatak tipa int 

			// Slanje pripremljene poruke zapisane unutar strukture messageStruct
			// prosledjujemo adresu promenljive message u memoriji, jer se na toj adresi nalaze podaci koje saljemo
			// kao i velicinu te strukture (jer je to duzina poruke u bajtima)
			iResult = send(connectSocket, (char*)&message, (int)sizeof(messageStruct), 0);

			// Provera rezultata send funkcije
			if (iResult == SOCKET_ERROR)
			{
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(connectSocket);
				WSACleanup();
				return 1;
			}

			EnterCriticalSection(&csScreen);
			printf("Message successfully sent. Total bytes: %ld\n", iResult);
			LeaveCriticalSection(&csScreen);
		}
		else if (function == 3)
		{
			char data[DATA_SIZE] = "receive";
			memcpy(message.data, data, strlen(data) + 1);

			length = strlen(data);
			message.dataSize = htonl(length);  //obavezna funkcija htond() jer cemo slati podatak tipa int 

			// Slanje pripremljene poruke zapisane unutar strukture messageStruct
			// prosledjujemo adresu promenljive message u memoriji, jer se na toj adresi nalaze podaci koje saljemo
			// kao i velicinu te strukture (jer je to duzina poruke u bajtima)
			iResult = send(connectSocket, (char*)&message, (int)sizeof(messageStruct), 0);

			// Provera rezultata send funkcije
			if (iResult == SOCKET_ERROR)
			{
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(connectSocket);
				WSACleanup();
				return 1;
			}

			EnterCriticalSection(&csScreen);
			printf("Message successfully sent. Total bytes: %ld\n", iResult);
			LeaveCriticalSection(&csScreen);
		}
		if (function == 4)
		{
			break;
		}
		else
			continue;

	}

	DeleteCriticalSection(&csScreen);
	if (handle != NULL)
		CloseHandle(handle);

	// Gasenje konekcije jer je proces gotov
	iResult = shutdown(connectSocket, SD_BOTH);

	//Provera da li je konekcija uspesno ugasena
	if (iResult == SOCKET_ERROR)
	{
		printf("Shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	Sleep(1000);

	// Zatvaranje connected socket-a
	closesocket(connectSocket);

	//Deinicijalizacija WSA biblioteke
	WSACleanup();

	return 0;
}



DWORD WINAPI handleIncomingData(LPVOID lpParam)
{
	SOCKET* connectSocket = (SOCKET*)lpParam;

	int iResult;
	char messageBuffer[BUFFER_SIZE];

	unsigned long  mode = 0;
	if (ioctlsocket(*connectSocket, FIONBIO, &mode) != 0)
		printf("ioctlsocket failed with error.");

	while (true)
	{
		
			iResult = recv(*connectSocket, messageBuffer, BUFFER_SIZE - 1, 0);
			if (iResult > 0 && iResult < BUFFER_SIZE)
			{
				messageBuffer[iResult] = '\0';
				if (messageBuffer[0] == '0')
				{
					EnterCriticalSection(&csScreen);
					printf("This process is registered already.\n");
					LeaveCriticalSection(&csScreen);
				}
				else if (messageBuffer[0] == '1')
				{
					EnterCriticalSection(&csScreen);
					printf("Registered successfully.\n");
					LeaveCriticalSection(&csScreen);
				}
				else if (messageBuffer[0] == '2')
				{
					EnterCriticalSection(&csScreen);
					printf("Data wasn't sent successfully.\n");
					LeaveCriticalSection(&csScreen);
				}
				else if (messageBuffer[0] == '4')
				{
					EnterCriticalSection(&csScreen);
					printf("Your copy has stopped working. Please stop sending messages and close connection.\n");
					LeaveCriticalSection(&csScreen);
					break;
				}
				else
				{
					messageStruct* message = (messageStruct*)messageBuffer;

					message->ID = ntohl(message->ID);
					message->dataSize = ntohl(message->dataSize);

					EnterCriticalSection(&csScreen);
					printf("Message received:\n");
					printf("\tID: %d\n", message->ID);
					printf("\tData: %s\n", message->data);
					printf("\tDataSize: %d\n", message->dataSize);
					printf("_______________________________  \n");
					LeaveCriticalSection(&csScreen);
				}
			}
	}

	return 0;
}

void Menu()
{
	EnterCriticalSection(&csScreen);
	printf("Choose an option:\n");
	printf("\t1. Register\n");
	printf("\t2. Send data\n");
	printf("\t3. Receive data\n");
	printf("\t4. Exit\n");
	LeaveCriticalSection(&csScreen);
}
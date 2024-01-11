#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"
#include "Threads.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#pragma pack(1)

#define DEFAULT_PORT_R1 27016
#define DEFAULT_PORT_R2 "27017"
#define BUFFER_SIZE 256
#define MAX_CLIENTS 3
#define THREAD_NUM 20

List* RegisteredClients;
CRITICAL_SECTION csRegClients;

bool InitializeWindowsSockets()
{
	WSADATA wsaData;
	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}

int main()
{

#pragma region listenRegion

// Socket used for listening for new clients 
	SOCKET listenSocket = INVALID_SOCKET;
	// Socket used for communication with client
	SOCKET clientSocket = INVALID_SOCKET;

	// variable used to store function return value
	int iResult;

	if (InitializeWindowsSockets() == false)
	{
		// we won't log anything since it will be logged
		// by InitializeWindowsSockets() function
		return 1;
	}

	// Prepare address information structures
	addrinfo* resultingAddress = NULL;
	addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;       // IPv4 address
	hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
	hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
	hints.ai_flags = AI_PASSIVE;     // 

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT_R2, &hints, &resultingAddress);
	if (iResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	listenSocket = socket(AF_INET,      // IPv4 address famly
		SOCK_STREAM,  // stream socket
		IPPROTO_TCP); // TCP

	if (listenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket - bind port number and local address 
	// to socket
	iResult = bind(listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	// Since we don't need resultingAddress any more, free it
	freeaddrinfo(resultingAddress);

	// Set listenSocket in listening mode
	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

#pragma endregion 

	printf("Server initialized, waiting for clients.\n");

	// set of socket descriptors
	fd_set readfds;

	// timeout za select funkciju
	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;

	DWORD sendID[THREAD_NUM];
	HANDLE hSend[THREAD_NUM];

	for (int i = 0; i < THREAD_NUM; i++)
	{
		hSend[i] = NULL;
	}
	int thread = -1;

	RegisteredClients = (List*)malloc(sizeof(List));
	RegisteredClients->head = 0;
	InitializeCriticalSection(&csRegClients);

#pragma region Accept

	while (true)
	{
		// initialize socket set
		FD_ZERO(&readfds);
		FD_SET(listenSocket, &readfds);

		// wait for events on set

		int selectResult = select(0, &readfds, NULL, NULL, &timeVal);

		if (selectResult == SOCKET_ERROR)
		{
			printf("Select failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}
		else if (selectResult == 0) // timeout expired
		{
			if (_kbhit()) //check if some key is pressed
			{
				_getch();
				printf("...\n");
			}
			continue;
		}
		else if (FD_ISSET(listenSocket, &readfds))
		{
			Params* params = (Params*)malloc(sizeof(Params));
			params->doWork = (bool*)malloc(sizeof(bool));
			*params->doWork = true;
			params->listenSocket = listenSocket;

			// Struktura za informacije o povezanom klijentu
			sockaddr_in clientAddr;
			int clientAddrSize = sizeof(struct sockaddr_in);

			// Novi zahtev za povezivanje je primljen. Dodajemo novi socket u niz na prvu slobodnu poziciju.
			clientSocket = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);

			if (clientSocket == INVALID_SOCKET)
			{
				if (WSAGetLastError() == WSAECONNRESET)
				{
					printf("accept failed, because timeout for client request has expired.\n");
				}
				else
				{
					printf("accept failed with error: %d\n", WSAGetLastError());
				}
			}
			else
			{
				printf("New client connected.\n");

				params->clientSocket = clientSocket;

				unsigned long mode = 1;
				if (ioctlsocket(clientSocket, FIONBIO, &mode) != 0)
				{
					printf("ioctlsocket failed with error.");
					continue;
				}


				printf("Waiting connection with Replicator2...\n");


				// Socket za prijem sa replikatora
				SOCKET connectSocket = INVALID_SOCKET;

				if (InitializeWindowsSockets() == false)
				{
					// we won't log anything since it will be logged
					// by InitializeWindowsSockets() function
					return 1;
				}

				// create a socket
				connectSocket = socket(AF_INET,
					SOCK_STREAM,
					IPPROTO_TCP);

				if (connectSocket == INVALID_SOCKET)
				{
					printf("socket failed with error: %ld\n", WSAGetLastError());
					WSACleanup();
					return 1;
				}

				mode = 0;
				if (ioctlsocket(params->connectSocket, FIONBIO, &mode) != 0)
					printf("ioctlsocket failed with error.");

				// create and initialize address structure
				sockaddr_in serverAddress;
				serverAddress.sin_family = AF_INET;
				serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
				serverAddress.sin_port = htons(DEFAULT_PORT_R1);
				// connect to server specified in serverAddress and socket connectSocket
				if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
				{
					printf("Unable to connect to server.\n");
					closesocket(connectSocket);
				}
				params->connectSocket = connectSocket;


				// socket za slanje na replikator
				SOCKET replicatorSocket = INVALID_SOCKET;
				replicatorSocket = accept(listenSocket, NULL, NULL);

				if (replicatorSocket == INVALID_SOCKET)
				{
					printf("accept failed with error: %d\n", WSAGetLastError());
					closesocket(listenSocket);
					WSACleanup();
					return 1;
				}
				else
				{
					params->replicatorSocket = replicatorSocket;
					printf("Connection with Replicator2 established.\n");
				}

				{
					++thread;
					if (thread >= THREAD_NUM)
						break;

					hSend[thread] = CreateThread(NULL, 0, &ClientSend, params, 0, &sendID[thread]);
				}

				printf("New client request accepted. Client address: %s : %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
			}
		}
	}

#pragma endregion

#pragma region CleanUp

	for (int i = 0; i < THREAD_NUM; i++)
	{
		if (hSend[i] != NULL)
			CloseHandle(hSend[i]);
	}

	destroy(RegisteredClients);
	DeleteCriticalSection(&csRegClients);

	// Zatvori listen i accepted socket-e
	closesocket(listenSocket);

	// Deinicijalizacija WSA biblioteke
	WSACleanup();

	return 0;

#pragma endregion


}
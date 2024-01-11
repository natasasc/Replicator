#include "Threads.h"

DWORD WINAPI ClientSend(LPVOID lpParam)
{
	// timeout za select funkciju
	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;

	int iResult;
	char dataBuffer[BUFFER_SIZE];

	Params* params = (Params*)lpParam;		// imamo popunjena prva 4 polja, ostalo je prazno
	params->hSemaphoreSend = CreateSemaphore(0, 0, RING_SIZE, NULL);
	params->hSemaphoreRecv = CreateSemaphore(0, 0, RING_SIZE, NULL);

	messageStruct* message;

	fd_set readfds;

	params->sendBuffer = (RingBuffer*)malloc(sizeof(RingBuffer));
	initRingBuffer(params->sendBuffer);
	params->recvBuffer = (RingBuffer*)malloc(sizeof(RingBuffer));
	initRingBuffer(params->recvBuffer);
	params->clientData = (List*)malloc(sizeof(List));
	params->clientData->head = 0;

	InitializeCriticalSection(&params->csSendBuffer);
	InitializeCriticalSection(&params->csRecvBuffer);
	InitializeCriticalSection(&params->csClientData);

	DWORD sendID, recvID;
	HANDLE hSend = CreateThread(NULL, 0, &Send, params, 0, &sendID);
	HANDLE hRecv = CreateThread(NULL, 0, &Receive, params, 0, &recvID);

	while (params->doWork != NULL)
	{
		if (*params->doWork == false)
			break;

		// initialize socket set
		FD_ZERO(&readfds);
		FD_SET(params->clientSocket, &readfds);

		int selectResult = select(0, &readfds, NULL, NULL, &timeVal);

		if (selectResult == SOCKET_ERROR)
		{
			printf("Select in thread failed with error: %d\n", WSAGetLastError());
			*params->doWork = false;
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
		// Provera da li je nova poruka primljena od klijenta
		else if (FD_ISSET(params->clientSocket, &readfds))
		{
			iResult = recv(params->clientSocket, dataBuffer, BUFFER_SIZE - 1, 0);
			// dobijamo u iResult broj bajta koji je klijent poslao

			if (iResult > 0 && iResult < BUFFER_SIZE)
			{
				dataBuffer[iResult] = '\0';
				printf("Message received from client.\n");

				message = (messageStruct*)dataBuffer;

				message->ID = ntohl(message->ID);
				message->dataSize = ntohl(message->dataSize);

				/*printf("ID: %d\n", message->ID);
				printf("Data: %s\n", message->data);
				printf("DataSize: %d\n", message->dataSize);
				printf("_______________________________  \n");*/

				if (strcmp(message->data, "register") == 0)
				{
					// Cuvamo klijenta u listi ukoliko nije vec registrovan
					if (!exists(RegisteredClients, message->ID))
					{
						EnterCriticalSection(&csRegClients);
						insertback(RegisteredClients, *message);
						LeaveCriticalSection(&csRegClients);

						displaylist(RegisteredClients);

						char msg[BUFFER_SIZE - 1] = "1";		// flag za uspesnu registraciju

						int res;
						res = send(params->clientSocket, (char*)(msg), strlen(msg), 0);
					}
					else
					{
						printf("Client with id %d is already registered.\n", message->ID);

						char msg[BUFFER_SIZE - 1] = "0";		// flag za vec odradjenu registraciju

						int res;
						res = send(params->clientSocket, (char*)(msg), strlen(msg), 0);

						*params->doWork = false;
						Sleep(2000);
					}
				}
				else
				{
					// Klijent trazi SendData ili ReceiveData
					// Samo postavljamo podatke na red
					EnterCriticalSection(&params->csSendBuffer);
					ringBufPut(params->sendBuffer, *message);
					LeaveCriticalSection(&params->csSendBuffer);
					ReleaseSemaphore(params->hSemaphoreSend, 1, NULL);	// obavestavamo nit Send da je nesto postavljeno na red
				}

			}
			else if (iResult == 0)
			{
				// Konekcija je zatvorena
				printf("Connection with client closed.\n");

				char msg[BUFFER_SIZE - 1] = "4";		// flag za zatvorenu konekciju
				int res = send(params->replicatorSocket, (char*)msg, strlen(msg), 0);

				*params->doWork = false;
				Sleep(2000);
			}
			else if (iResult < 0)
			{
				// desila se greska tokom recv
				printf("recv in thread failed with error: %d\n", WSAGetLastError());

				*params->doWork = false;
				Sleep(2000);
			}
		}
	}

	// Oslobadjanje memorije
	free(params->doWork);
	destroyRingBuffer(params->sendBuffer);
	destroyRingBuffer(params->recvBuffer);
	free(params->clientData);
	DeleteCriticalSection(&params->csSendBuffer);
	DeleteCriticalSection(&params->csRecvBuffer);
	DeleteCriticalSection(&params->csClientData);

	if (hSend != NULL)
		CloseHandle(hSend);
	if (hRecv != NULL)
		CloseHandle(hRecv);
	if (params->hSemaphoreSend != NULL)
		CloseHandle(params->hSemaphoreSend);
	if (params->hSemaphoreRecv != NULL)
		CloseHandle(params->hSemaphoreRecv);

	closesocket(params->clientSocket);
	free(params);
	return 0;
}



DWORD WINAPI Send(LPVOID lpParam)
{
	Params* params = (Params*)lpParam;

	while (params->doWork != NULL)
	{
		if (*params->doWork == false)
			break;

		WaitForSingleObject(params->hSemaphoreSend, INFINITE);	// Nit ceka da se poveca vrednost semafora, do tad "spava"

		messageStruct* msg = (messageStruct*)malloc(sizeof(messageStruct));
		EnterCriticalSection(&params->csSendBuffer);
		ringBufGet(params->sendBuffer, msg);
		LeaveCriticalSection(&params->csSendBuffer);

		msg->dataSize = htonl(msg->dataSize);
		msg->ID = htonl(msg->ID);

		// poslati drugom replikatoru
		int res;
		res = send(params->replicatorSocket, (char*)msg, (int)sizeof(messageStruct), 0);

		if (res == SOCKET_ERROR)
		{
			char msg[BUFFER_SIZE - 1] = "2";		// flag za neuspesno slanje
			res = send(params->clientSocket, (char*)msg, strlen(msg), 0);

			*params->doWork = false;
		}

		printf("Message sent to Replicator.\n");

		free(msg);
	}
	return 0;
}

// Nit koja prima poruke sa replikatora 2
DWORD WINAPI Receive(LPVOID lpParam)
{
	int iResult;
	char dataBuffer[BUFFER_SIZE];
	Params* params = (Params*)lpParam;

	fd_set readfds;
	FD_ZERO(&readfds);

	messageStruct* message;

	DWORD funId;
	HANDLE hConn = CreateThread(NULL, 0, &ClientReceive, params, 0, &funId);

	timeval timeVal;
	timeVal.tv_sec = 2;
	timeVal.tv_usec = 0;

	while (params->doWork != NULL)
	{
		if (*params->doWork == false)
			break;

		FD_CLR(params->connectSocket, &readfds);
		FD_SET(params->connectSocket, &readfds);

		int result = select(0, &readfds, NULL, NULL, &timeVal);

		if (result == 0)
		{
			// vreme za cekanje je isteklo
		}
		else if (result == SOCKET_ERROR)
		{
			//desila se greska prilikom poziva funkcije
			*params->doWork = false;
		}
		else if (FD_ISSET(params->connectSocket, &readfds))
		{

			// iResult je jednak broju bajta koji je stigao sa replikatora 2
			iResult = recv(params->connectSocket, dataBuffer, BUFFER_SIZE - 1, 0);
			if (iResult > 0 && iResult < BUFFER_SIZE)
			{
				dataBuffer[iResult] = '\0';
				printf("Message received from Replicator.\n");

				message = (messageStruct*)dataBuffer;

				message->ID = ntohl(message->ID);
				message->dataSize = ntohl(message->dataSize);

				/*printf("ID: %d\n", message->ID);
				printf("Data: %s\n", message->data);
				printf("DataSize: %d\n", message->dataSize);
				printf("_______________________________  \n");*/

				if (strcmp(message->data, "receive") == 0)
				{
					// Trazi sve podatke sa prvog klijenta i postavi ih na red za slanje
					if (params->clientData->head == 0)
						continue;


					Listitem* temp = (Listitem*)malloc(sizeof(Listitem));
					temp = params->clientData->head;

					while (temp != NULL)
					{
						EnterCriticalSection(&params->csSendBuffer);
						ringBufPut(params->sendBuffer, temp->data);	// u data se nalazi 1 messageStruct
						LeaveCriticalSection(&params->csSendBuffer);
						ReleaseSemaphore(params->hSemaphoreSend, 1, NULL);
						temp = temp->next;
					}

					free(temp);
				}
				else	// Klijent 2 je pozvao SendData
				{
					// Samo postavljamo podatke na red
					EnterCriticalSection(&params->csRecvBuffer);
					ringBufPut(params->recvBuffer, *message);
					LeaveCriticalSection(&params->csRecvBuffer);
					ReleaseSemaphore(params->hSemaphoreRecv, 1, NULL);
				}
			}
			else if (iResult == 0)
			{
				// connection was closed gracefully
				printf("Connection with Replicator closed.\n");

				*params->doWork = false;
			}
			else
			{
				// there was an error during recv
				printf("recv failed with error: %d\n", WSAGetLastError());

				*params->doWork = false;
			}
		}
	}

	if (hConn != NULL)
		CloseHandle(hConn);
	return 0;
}


DWORD WINAPI ClientReceive(LPVOID lpParam)
{
	Params* params = (Params*)lpParam;
	while (params->doWork != NULL)
	{
		if (*params->doWork == false)
			break;

		WaitForSingleObject(params->hSemaphoreRecv, INFINITE);

		messageStruct* msg = (messageStruct*)malloc(sizeof(messageStruct));
		EnterCriticalSection(&params->csRecvBuffer);
		ringBufGet(params->recvBuffer, msg);
		LeaveCriticalSection(&params->csRecvBuffer);

		// Sacuvati na klijentu
		EnterCriticalSection(&params->csClientData);
		insertfront(params->clientData, *msg);
		LeaveCriticalSection(&params->csClientData);

		msg->dataSize = htonl(msg->dataSize);
		msg->ID = htonl(msg->ID);

		// Poslati klijentu da se ispise na ekran
		int res;
		res = send(params->clientSocket, (char*)msg, (int)sizeof(messageStruct), 0);

		if (res == SOCKET_ERROR)
		{
			printf("send failed with error: %d\n", WSAGetLastError());
			*params->doWork = false;
		}

		printf("Message sent to Client.");
		Sleep(300);
	}
	return 0;
}
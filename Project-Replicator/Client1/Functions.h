#pragma once

#include "Message.h"

void RegisterService(int ServiceID);
void SendData(int ServiceID, void* data, int dataSize);
void ReceiveData(void* data, int dataSize);

void Menu();
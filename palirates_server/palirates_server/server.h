#pragma once
#pragma once
#include <string>
#include <winsock2.h>

void HandleGamePacket(SOCKET clientSocket, const std::string& packet, int clientId);
void HandleClient(SOCKET clientSocket, int clientId);
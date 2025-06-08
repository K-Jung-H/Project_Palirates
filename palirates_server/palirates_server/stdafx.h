#pragma once

#define WIN32_LEAN_AND_MEAN   
#define NOMINMAX              

#include <windows.h> 
#include <wrl.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")


#include <Mmsystem.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>

using namespace std;
#pragma once

#define _TEST_

#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#ifndef _WINSOCK2API_
#include <WinSock2.h>
#endif

#include <iostream>

void Init(int major_version, int minor_version);

SOCKET Cnt();

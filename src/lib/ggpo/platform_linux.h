/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#ifndef _GGPO_LINUX_H_
#define _GGPO_LINUX_H_

#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include "pevents.h"
using namespace neosmart;

#define sprintf_s snprintf
#define vsprintf_s vsnprintf
#define INFINITE 0xFFFFFFFF
#define WAIT_OBJECT_0 0
#define TRUE true
#define FALSE false

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

//typedef void HANDLE;
typedef void* HANDLE;
typedef bool BOOL;
typedef int SOCKET;
#define __cdecl 

class Platform {
public:  // types
   typedef pid_t ProcessID;

public:  // functions
   static ProcessID GetProcessID() { return getpid(); }
   static void AssertFailed(char *msg) { }
   static uint32_t GetCurrentTimeMS();
   static int GetConfigInt(const char* name);
   static bool GetConfigBool(const char* name);
};


int strncat_s(char *dest, size_t destSize, const char *src, size_t count);
int strcpy_s(char *dest, size_t destSize, const char *src);
int fopen_s(FILE** file, const char* filename, const char* mode);

#endif

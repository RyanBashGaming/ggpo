/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#include "platform_linux.h"

struct timespec start = { 0 };

uint32_t Platform::GetCurrentTimeMS() {
    if (start.tv_sec == 0 && start.tv_nsec == 0) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        return 0;
    }
    struct timespec current;
    clock_gettime(CLOCK_MONOTONIC, &current);

    return ((current.tv_sec - start.tv_sec) * 1000) +
           ((current.tv_nsec  - start.tv_nsec ) / 1000000);
}

// This is the shit I'm porting from Windows.

int
Platform::GetConfigInt(const char* name)
{
   /*char buf[1024];
   if (GetEnvironmentVariable(name, buf, ARRAY_SIZE(buf)) == 0) {
      return 0;
   }
   return atoi(buf);*/
   return 0;
}

bool Platform::GetConfigBool(const char* name)
{
   /*char buf[1024];
   if (GetEnvironmentVariable(name, buf, ARRAY_SIZE(buf)) == 0) {
      return false;
   }
   return atoi(buf) != 0 || _stricmp(buf, "true") == 0;*/
   return true;
}

// This is the shit I added for ease of porting.

#include <stdio.h>
#include <string.h>

int strncat_s(
    char *dest,
    size_t destSize,
    const char *src,
    size_t count
) {
    size_t destLength = strlen(dest);
    size_t srcLength = strlen(src);
    size_t totalLength = destLength + srcLength;

    if (count < srcLength) {
        // Buffer overflow: not enough space in dest to append src
        return -1;
    }

    if (totalLength >= destSize) {
        // Buffer overflow: not enough space in dest to hold both dest and src
        return -1;
    }

    // Use strncat to concatenate the strings
    strncat(dest, src, count);

    return 0;
}

int strcpy_s(
    char *dest,
    size_t destSize,
    const char *src
) {
    size_t srcLength = strlen(src);

    if (srcLength >= destSize) {
        // Buffer overflow: not enough space in dest to hold src
        return -1;
    }

    strcpy(dest, src);

    return 0;
}

int fopen_s(
    FILE** file,
    const char* filename,
    const char* mode
) {
    if (file == NULL || filename == NULL || mode == NULL) {
        // Invalid arguments
        return -2;
    }

    *file = fopen(filename, mode);
    if (*file == NULL) {
        // File opening failed
        return -1;
    }

    return 0;
}

/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include "Debug.h"
#include <mutex>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <vector>

#include <iostream>
#include <fstream>
using namespace std;
// #define BME_ENABLE_LOGGING

namespace Broadcom
{

static const char* debugPrintExecName = NULL;
static char* debugModules = NULL;

static std::vector<std::string> debugModulesList;

static std::mutex debugPrintMutex;
static char debugPrintTimeStamp[20];
static char debugPrintBuffer[4096];

static void bmeDebugGetTimeStamp(char *timeStamp, int size);

void debugInitialize(const char * execName)
{
    debugPrintExecName = execName;
    debugModules = getenv(TRLS_TRACE_MODULES);

    if (debugModules != NULL) {
        for (;;) {
            char buf[64];
            const char *end = strchr(debugModules, ',');
            size_t name_len;

            if (!end) {
                if (*debugModules) {
                    debugModulesList.push_back(debugModules);
                }

                break;
            }

            name_len = end - debugModules;

            if (name_len > 0 && name_len < sizeof(buf)) {
                strncpy(buf, debugModules, name_len);
                buf[name_len] = '\0';
                debugModulesList.push_back(buf);
            }

            debugModules = (char*)(end + 1);
        }
    }
}

void bmeDebugPrint(Broadcom::DebugLevel level, const char* module, const char* fileName, int32_t lineNumber, const char *format, ...)
{
    bool printMessage = false;
#ifdef BME_ENABLE_LOGGING
    fstream bmeLog;
    bmeLog.open("/tmp/bme.log", ios::in | ios::out | ios::app);
#endif

    // Always print errors and warnings
    if ((level & DebugError) || (level & DebugWarning )) {
        printMessage = true;
    } else {
        for (std::vector<std::string>::const_iterator iter = debugModulesList.begin(); iter != debugModulesList.end(); ++iter) {
            if (iter->compare(module) == 0) {
                printMessage = true;
                break;
            }
        }
    }

    if (printMessage) {
        va_list args;
        va_start(args, format);
        std::lock_guard<std::mutex> scopedLock(debugPrintMutex);
        bmeDebugGetTimeStamp(debugPrintTimeStamp, sizeof(debugPrintTimeStamp));
        size_t position = snprintf(debugPrintBuffer, sizeof(debugPrintBuffer), "%s ", debugPrintTimeStamp);

        if (position > sizeof(debugPrintBuffer)) {
            position = sizeof(debugPrintBuffer);
        }

        if ( debugPrintExecName != NULL ) {
            position += snprintf(&debugPrintBuffer[position], sizeof(debugPrintBuffer) - position, "%s ", debugPrintExecName);

            if (position > sizeof(debugPrintBuffer)) {
                position = sizeof(debugPrintBuffer);
            }
        }

        position += snprintf(&debugPrintBuffer[position], sizeof(debugPrintBuffer) - position, "pid=%d ", getpid());

        if (position > sizeof(debugPrintBuffer)) {
            position = sizeof(debugPrintBuffer);
        }

        position += snprintf(&debugPrintBuffer[position], sizeof(debugPrintBuffer) - position, "tid=%p ", reinterpret_cast<void *>(pthread_self()));

        if (position > sizeof(debugPrintBuffer)) {
            position = sizeof(debugPrintBuffer);
        }

        if ( fileName != NULL ) {
            position += snprintf(&debugPrintBuffer[position], sizeof(debugPrintBuffer) - position, "%s(%d): ", fileName, lineNumber);

            if (position > sizeof(debugPrintBuffer)) {
                position = sizeof(debugPrintBuffer);
            }
        }

        position += vsnprintf(&debugPrintBuffer[position], sizeof(debugPrintBuffer) - position, format, args);

        if (position > sizeof(debugPrintBuffer)) {
            position = sizeof(debugPrintBuffer);
        }

        if (debugPrintBuffer[position - 1] != '\n') {
            snprintf(&debugPrintBuffer[position], sizeof(debugPrintBuffer) - position, "\n");
        }

        debugPrintBuffer[sizeof(debugPrintBuffer) - 1] = '\0';

        if (level == DebugAll) {
            fprintf(stderr, "--- %s", debugPrintBuffer);
#ifdef BME_ENABLE_LOGGING

            if (bmeLog.is_open()) {
                bmeLog << "--- " << debugPrintBuffer;
                bmeLog.close();
            }

#endif
        } else if (level & DebugError) {
            fprintf(stderr, "### %s", debugPrintBuffer);
#ifdef BME_ENABLE_LOGGING

            if (bmeLog.is_open()) {
                bmeLog << "### " << debugPrintBuffer;
                bmeLog.close();
            }

#endif
        } else if (level & DebugWarning) {
            fprintf(stderr, "*** %s", debugPrintBuffer);
#ifdef BME_ENABLE_LOGGING

            if (bmeLog.is_open()) {
                bmeLog << "*** " << debugPrintBuffer;
                bmeLog.close();
            }

#endif
        } else {
            fprintf(stderr, "!!! %s", debugPrintBuffer);
#ifdef BME_ENABLE_LOGGING

            if (bmeLog.is_open()) {
                bmeLog << "!!! " << debugPrintBuffer;
                bmeLog.close();
            }

#endif
        }

        fflush(stderr);
        va_end(args);
    }
}

void bmeDebugGetTimeStamp(char *timeStamp, int size)
{
    if (size > 0) {
        struct timespec currentTime;
        timeStamp[0] = '\0';

        if (clock_gettime(CLOCK_MONOTONIC, &currentTime) == 0) {
            uint32_t hours = (currentTime.tv_sec) / 3600;
            uint32_t minutes = ((currentTime.tv_sec) / 60) % 60;
            uint32_t seconds = currentTime.tv_sec % 60;
            uint32_t milliseconds = currentTime.tv_nsec / 1000000;
            snprintf(timeStamp, size, "%02u:%02u:%02u.%03u", hours, minutes, seconds, milliseconds);
        }

        timeStamp[size - 1] = '\0';
    }
}

}  // namespace Broadcom

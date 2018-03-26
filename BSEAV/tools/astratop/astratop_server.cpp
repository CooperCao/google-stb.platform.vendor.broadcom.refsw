/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.

 ******************************************************************************/
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string>
#include <cstring>
#include <set>
#include <vector>
#include <map>
#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <mutex>
#include <condition_variable>
#include <csignal>
#include <atomic>
#include <errno.h>
#include <sys/un.h>
#include <chrono>
#include <thread>

#include "astratop_utils.h"
#include "parser.h"

using namespace std;

/*32 bit bitMask 'CPUCoresBitMask' is used to save the bitMask of enabled cores. Hence MAX_NUM_CPU_CORES = 32*/
#define MAX_NUM_CPU_CORES               32
#define NUM_CPU_CORES                   4
#define BUFFSIZE                        1024
#define TRACELOG_ENTRY_SIZE             50
#define TRACELOGGER_FILE_PATH           "/dev/tracelog"
/*Define DEBUG_BCKGRND_DATA as 1 if you want to log the tasks present when readRequestWriteResponse is called*/
#define DEBUG_BCKGRND_DATA              0
/*Define DEBUG_PRINT as printf to log debug prints*/
#define DEBUG_PRINT                     noPrint
#define MAX_DATA_FETCHES                400000
#define MAX_NO_ENTRIES_COUNT            100

typedef int parseError;
#define TASK_PARTIALLY_PARSED            0
#define TASK_FULLY_PARSED                1
#define PARSE_FAILED                    -1

enum appState
{
    eRUNNING = 32,    /*A non-standard signal value*/
};

/*Accumalated task Information over a period of 1 sec*/
struct accmltdTaskInformation
{
    unsigned int moduleID            :4;
    unsigned int CPUID               :2;
    unsigned int taskID              :10;
    unsigned int taskStatus          :1;
    union {
        /*Valid for CFS Tasks*/
        uint16_t priority;
        /*Valid for EDF Tasks*/
        uint32_t taskLoadAccumalated;
    };
    float CPUperAccumulated;
};

struct taskData
{
    accmltdTaskInformation taskInfo;
    uint8_t taskType;
    uint64_t timeStamp;
    int numberOfEntries;
    bool refreshed;
};

struct traceLogEntry
{
    uint64_t timeStamp;
    uint32_t dataWord;
    uint32_t index;
};

struct traceLogEntryData
{
    taskInformation taskInfo;
    uint8_t taskType;
    uint64_t timeStamp;
    bool partiallyParsed;
};

class AstraPerf
{
private:
    bool isInitialized;
    char *buf;
    int traceLogFD;
    vector<char> logData;
    char *logDataEnd;
    bool getLine(uint32_t lineNumber, string& line);
    void updateTaskMap(taskData *pGISBData);
    parseError parse32BitTaskInfo(uint32_t dataWord, traceLogEntryData *parsed_data);
public:
    static set<uint8_t> cpuCores;
    mutex tasksMapMutex; /*protect access to cpuTasksMap and tasksMap*/
    map<uint8_t, set<uint16_t>> cpuTasksMap;
    map<uint16_t, struct taskData> tasksMap;
#if DEBUG_BCKGRND_DATA
    map<string, uint8_t> fieldWidthMap;
#endif
    int init();
    void unInit();
    int getCPUCoresInfo();
    int captureEntries();
    int parseEntries();
    void resetDataBase();
    AstraPerf()
    {
        isInitialized = false;
        buf = new char[BUFFSIZE];
    };
    ~AstraPerf()
    {
        delete buf;
    }
};

set<uint8_t> AstraPerf::cpuCores;
volatile sig_atomic_t gSignalStatus;
atomic<int> dataFetchCount;
mutex gAppMutex;
condition_variable cv;
bool terminated;

/**
 *  Function: This function will close the specified socket and exit the app.
 **/
const char *noPrint(
    const char *format,
    ...
    )
{
    return( format );
}

/**
 *  Function: This function will close the specified socket and exit the app.
 **/
static void closeAndExit(
    int         socketFd,
    const char *reason
    )
{
    DEBUG_PRINT("FAILURE: socket %d; reason (%s)\n", socketFd, reason);
    if (socketFd>0) {close(socketFd);}
    raise(SIGTERM);
}

/**
 *  Function: This function will populate response struct with tasks from the tasksMap based on the request.
 **/
static void fillTasksDetails(
    AstraPerf * pInstance,
    astraTopRequest *pRequest,
    cpuCoresInformation *pCoresInfo,
    tasksInformation *pTasksInfo
    )
{
    uint16_t tasksCount = 0;
    uint16_t perCPUTasks = 0, perCPURTTasks = 0, perCPUNRTTasks = 0;
    set<uint8_t>::iterator coreIt;
    task tempTask;
    bool addTask = false;
    dataFetchCount = 0;
    lock_guard<mutex> guard(pInstance->tasksMapMutex);
    if (pRequest->secondaryCmd & ASTRATOP_CMD_GET_TASKID_INFO) {
        taskData& temp = pInstance->tasksMap[pRequest->taskId];
        if (temp.refreshed) {
            memset(&tempTask, 0 , sizeof(tempTask));
            tempTask.taskType = temp.taskType;
            tempTask.taskDetails.CPUID = temp.taskInfo.CPUID;
            tempTask.taskDetails.taskID = temp.taskInfo.taskID;
            tempTask.taskDetails.taskStatus = temp.taskInfo.taskStatus;
            if (temp.taskType == NRT_TASKS) {
                tempTask.taskDetails.priority = temp.taskInfo.priority;
            }
            else if (temp.taskType == RT_TASKS) {
                tempTask.taskDetails.taskLoad = temp.taskInfo.taskLoadAccumalated/temp.numberOfEntries;
            }
            tempTask.taskDetails.CPUper = temp.taskInfo.CPUperAccumulated/temp.numberOfEntries;
            tempTask.timeStamp = temp.timeStamp;
            addTask = true;
        }
    }
    for (coreIt = AstraPerf::cpuCores.begin(); coreIt != AstraPerf::cpuCores.end(); ++coreIt) {
        set<uint16_t>& tempSet = pInstance->cpuTasksMap[*coreIt];
        set<uint16_t>::iterator tempIterator;
        for (tempIterator = tempSet.begin(); tempIterator != tempSet.end(); ++tempIterator) {
            taskData& temp = pInstance->tasksMap[*tempIterator];
            if (temp.refreshed ) {
                if (pRequest->secondaryCmd & ASTRATOP_CMD_GET_PER_CORE_TASKS_INFO) {
                    if (pRequest->coresRequest.coresBitMask & (1 << *coreIt)) {
                        if (pRequest->coresRequest.taskType & temp.taskType) {
                            pTasksInfo->tasks[tasksCount].taskType = temp.taskType;
                            pTasksInfo->tasks[tasksCount].taskDetails.CPUID = temp.taskInfo.CPUID;
                            pTasksInfo->tasks[tasksCount].taskDetails.taskID = temp.taskInfo.taskID;
                            pTasksInfo->tasks[tasksCount].taskDetails.taskStatus = temp.taskInfo.taskStatus;
                            if (temp.taskType == NRT_TASKS) {
                                perCPUNRTTasks++;
                                pTasksInfo->tasks[tasksCount].taskDetails.priority = temp.taskInfo.priority;
                            }
                            else if (temp.taskType == RT_TASKS) {
                                perCPURTTasks++;
                                pTasksInfo->tasks[tasksCount].taskDetails.taskLoad = temp.taskInfo.taskLoadAccumalated/temp.numberOfEntries;
                            }
                            pTasksInfo->tasks[tasksCount].taskDetails.CPUper = temp.taskInfo.CPUperAccumulated/temp.numberOfEntries;
                            pTasksInfo->tasks[tasksCount].timeStamp = temp.timeStamp;
                            tasksCount++;
                            perCPUTasks++;
                        }
                    }
                }
                temp.numberOfEntries = 0;
                temp.refreshed = false;
                temp.taskInfo.CPUperAccumulated = 0;
                temp.taskInfo.taskLoadAccumalated = 0;
            }
            else {
                /*Remove task entry from map if no entries received for this task since previous call to fillTasksDetails*/
                pInstance->tasksMap.erase(*tempIterator);
                tempSet.erase(tempIterator);
            }
        }
        if (pRequest->secondaryCmd & ASTRATOP_CMD_GET_PER_CORE_TASKS_INFO) {
            if (pRequest->coresRequest.coresBitMask & (1 << *coreIt)) {
                pCoresInfo->cores[pCoresInfo->noOfCores].cpuId = *coreIt;
                pCoresInfo->cores[pCoresInfo->noOfCores].noOfTasks = perCPUTasks;
                pCoresInfo->cores[pCoresInfo->noOfCores].noOfNRTTasks = perCPUNRTTasks;
                pCoresInfo->cores[pCoresInfo->noOfCores].noOfRTTasks = perCPURTTasks;
                pCoresInfo->noOfCores++;
                perCPUTasks = 0;
                perCPUNRTTasks = 0;
                perCPURTTasks = 0;
            }
        }
    }
    pInstance->cpuTasksMap.clear();

    if (addTask == true) {
        pTasksInfo->tasks[tasksCount].taskType = tempTask.taskType;
        pTasksInfo->tasks[tasksCount].taskDetails.CPUID = tempTask.taskDetails.CPUID;
        pTasksInfo->tasks[tasksCount].taskDetails.taskID = tempTask.taskDetails.taskID;
        pTasksInfo->tasks[tasksCount].taskDetails.taskStatus = tempTask.taskDetails.taskStatus;
        if (tempTask.taskType == NRT_TASKS) {
            pTasksInfo->tasks[tasksCount].taskDetails.priority = tempTask.taskDetails.priority;
        }
        else if (tempTask.taskType == RT_TASKS) {
            pTasksInfo->tasks[tasksCount].taskDetails.taskLoad = tempTask.taskDetails.taskLoad;
        }
       pTasksInfo->tasks[tasksCount].taskDetails.CPUper = tempTask.taskDetails.CPUper;
       pTasksInfo->tasks[tasksCount].timeStamp = tempTask.timeStamp;
       tasksCount++;
    }
    pTasksInfo->noOfTasks = tasksCount;
#if DEBUG_BCKGRND_DATA
    cout << "No. Of Tasks: " << pTasksInfo->noOfTasks << endl;
    for (int i = 0; i < pTasksInfo->noOfTasks; i++) {
        cout << setw(18) << pTasksInfo->tasks[i].timeStamp;
        cout << setw(pInstance->fieldWidthMap["CPU ID"]) << pTasksInfo->tasks[i].taskDetails.CPUID;
        cout << " " << setw(pInstance->fieldWidthMap["Task ID"]) << pTasksInfo->tasks[i].taskDetails.taskID;
        if (pTasksInfo->tasks[i].taskType == NRT_TASKS) {
            cout << " " << setw(pInstance->fieldWidthMap["Priority"]) << pTasksInfo->tasks[i].taskDetails.priority;
            cout << " " << setw(pInstance->fieldWidthMap["Load"]) << "NA";
        }
        else if (pTasksInfo->tasks[i].taskType == RT_TASKS) {
            cout  << " " << setw(pInstance->fieldWidthMap["Priority"]) << "RT";
            cout  << " " << setw(pInstance->fieldWidthMap["Load"]) << pTasksInfo->tasks[i].taskDetails.taskLoad;
        }
        cout << " " << setw(pInstance->fieldWidthMap["CPU %"]) << pTasksInfo->tasks[i].taskDetails.CPUper << " " <<
                setw(pInstance->fieldWidthMap["Status"]) << pTasksInfo->tasks[i].taskDetails.taskStatus;
        cout << endl;
    }
#endif
}

/**
 *  Function: This function will read the user's request coming in from the socket client(webserver in astratop tool),
 *  fill response struct accordingly and send response back to socket client
 **/
static int readRequestWriteResponse(
    AstraPerf *pInstance,
    int psd,
    astraTopRequest *pRequest,
    astraTopResponse *pResponse
    )
{
    int rc;
    uint32_t CPUCoresBitMask;
    set<uint8_t>::iterator coreIt;
    DEBUG_PRINT( "Server is reading request (%u bytes) from client ... \n", sizeof( *pRequest ));

    if (( rc = recv( psd, pRequest, sizeof( *pRequest ), 0 )) < 0)
    {
        closeAndExit( psd, "receiving stream  message" );
    }

    if (rc > 0)
    {
        switch (pRequest->cmd)
        {
            case ASTRATOP_CMD_GET_CPUCORES:
                DEBUG_PRINT("ASTRATOP_CMD_GET_CPUCORES received\n");
                CPUCoresBitMask = 0;
                for (coreIt = AstraPerf::cpuCores.begin(); coreIt != AstraPerf::cpuCores.end(); ++coreIt) {
                    CPUCoresBitMask |= (1 << (*coreIt));
                }
                pResponse->CPUCoresBitMask = CPUCoresBitMask;
                DEBUG_PRINT("Response sent is: %d\n", pResponse->CPUCoresBitMask);
                if (send( psd, pResponse, sizeof( *pResponse ), 0 ) < 0) {
                    closeAndExit( psd, "sending response" );
                }
                break;
            case ASTRATOP_CMD_GET_TASKINFO:
                DEBUG_PRINT("ASTRATOP_CMD_GET_TASKINFO received. Request =");
                DEBUG_PRINT("SecCmd: %d, CoreBitmask: %d, taskType: %d, taskId: %d\n", pRequest->secondaryCmd, pRequest->coresRequest.coresBitMask,
                    pRequest->coresRequest.taskType, pRequest->taskId);
                fillTasksDetails(pInstance, pRequest, &pResponse->coresInfo, &pResponse->tasksInfo);
                if (send( psd, pResponse, sizeof( *pResponse ), 0 ) < 0) {
                    closeAndExit( psd, "sending response" );
                }
                break;
            default:
                break;
        }
    }
    close( psd );
    return 0;
}

/**
 *  Function: This function will create a socket to listen to Unix socket client connections from SOCK_PATH
 *  and handle request from them.
 **/
static int startServer(
    AstraPerf * pInstance
    )
{
    int sd = 0, psd = 0;
    unsigned long int requestCount = 0;
    struct sockaddr_un server;
    astraTopRequest request;
    astraTopResponse response;

    /* Construct name of socket */
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, SOCK_PATH);
    unlink(server.sun_path);

    /* Create socket on which to send and receive */
    sd = socket( AF_UNIX, SOCK_STREAM, 0 );
    if (sd < 0) {
        perror( "opening stream socket" );
        return -1;
    }

    if (bind( sd, (struct sockaddr *)&server, sizeof( server )) < 0) {
        closeAndExit( sd, "binding name to stream socket" );
    }

    /* accept connections from clients and handle request from them */
    listen( sd, 4 );

    while (gSignalStatus == eRUNNING) {
        DEBUG_PRINT( "%s - Waiting to accept socket connections; requestCount %lu\n", __FUNCTION__, requestCount );
        psd = accept( sd, NULL, NULL );
        requestCount++;
        memset( &request, 0, sizeof( request ));
        memset( &response, 0, sizeof( response ));
        readRequestWriteResponse( pInstance, psd, &request, &response );
    }
    closeAndExit( sd, "closing server socket as server thread is terminated" );
    return( 0 );
}

/**
 *  Function: This function will initialise an instance of AstraPerf.
 **/
int AstraPerf::init(
    )
{
    if (isInitialized) {
        DEBUG_PRINT(" Analyser already initialized\n");
        return -1;
    }
    traceLogFD = open(TRACELOGGER_FILE_PATH, 0);
    if (traceLogFD == -1) {
        DEBUG_PRINT(" Error in opening /dev/tracelog\n");
        goto ERROR;
    }
    close(traceLogFD);
#if DEBUG_BCKGRND_DATA
    fieldWidthMap["Task ID"] = 7;
    fieldWidthMap["Priority"] = 8;
    fieldWidthMap["Load"] = 6;
    fieldWidthMap["CPU %"] = 10;
    fieldWidthMap["Status"] = 6;
    fieldWidthMap["CPU ID"] = 6;
#endif
    isInitialized = true;
    return 0;
    ERROR:
    isInitialized = false;
    return -1;
}

/**
 *  Function: This function will uninitialise an instance of AstraPerf.
 **/
void AstraPerf::unInit(
    void
    )
{
    logData.clear();
    tasksMap.clear();
    cpuTasksMap.clear();
    isInitialized = false;
}

/**
 *  Function: This function will populate cpuCores set with the ID of the enabled cores in the processor.
 **/
int AstraPerf::getCPUCoresInfo(
    void
    )
{
#if 0
    vector<char> cpuInfoLog;
    size_t rn;
    int cpuInfoFD;
    size_t pos = 0;
    unsigned long coreID;
    char *endp;
    const char *core;

    cpuInfoFD = open("/proc/cpuinfo", 0);
    if (cpuInfoFD == -1) {
        DEBUG_PRINT("Error in opening /proc/cpuinfo\n");
        return -1;
    }

    while ((rn = read(cpuInfoFD, buf, BUFFSIZE)) > 0) {
        cpuInfoLog.insert(cpuInfoLog.end(), buf, buf + rn);
    }

    vector<char>::iterator temp, prev;
    for (temp = cpuInfoLog.begin(); temp != cpuInfoLog.end(); temp++) {
        prev = temp;
        temp = find( temp, cpuInfoLog.end(), '\n' );
        string input(prev,temp);
        if (!strncmp(input.c_str(), "processor", 9)) {
            pos = input.find_last_not_of("0123456789");
            core = input.substr(pos + 1).c_str();
            coreID = strtoul(core, &endp, 0);
            if (*endp == '\0' && coreID < MAX_NUM_CPU_CORES) {
                cpuCores.insert((uint8_t)coreID);
            }
        }
    }
    cpuInfoLog.clear();
    return 0;
#endif
    int i;
    for (i = 0; i < NUM_CPU_CORES; i++) {
        cpuCores.insert((uint8_t)i);
    }
    return 0;
}

/**
 *  Function: This function will read from /dev/tracelog and copy into a buffer.
 **/
int AstraPerf::captureEntries(
    void
    )
{
    size_t rn;
    traceLogFD = open(TRACELOGGER_FILE_PATH, 0);
    if (traceLogFD == -1) {
        raise(SIGTERM);
        return -1;
    }
    logData.clear();
    while ((rn = read(traceLogFD, buf, BUFFSIZE)) > 0) {
        logData.insert(logData.end(), buf, buf + rn);
    }

    logDataEnd = &logData.back();
    close(traceLogFD);
    return logData.size();
}

/**
 *  Function: This function will read a line of characters from capture buffer and copy to string line.
 **/
bool AstraPerf::getLine(
    uint32_t lineNumber,
    string& line
    )
{
    bool result = true;
    if ((logData.data() + (TRACELOG_ENTRY_SIZE * (lineNumber + 1))) <= logDataEnd)
        line = string(logData.data() + (lineNumber*TRACELOG_ENTRY_SIZE), TRACELOG_ENTRY_SIZE);
    else
        result = false;

    return result;
}

/**
 *  Function: This function on successfully parsing entries in the capture buffer will return number of tasks for which entries
 *  were received, on failure will return -1.
 **/
int AstraPerf::parseEntries(
    void
    )
{
    traceLogEntry *pentry = 0;
    traceLogEntryData *pGISBData = 0;
    taskData *pTaskData;
    int count = 0, result = 0;
    parseError rc;
    string input = new char[400];
    pentry = new traceLogEntry;
    pGISBData = new traceLogEntryData;
    pTaskData = new taskData;
    unsigned long long int temp;
    char *endp;
    size_t pos = 0;
    uint32_t lineNumber = 0;
    const char* piece;

    if (!isInitialized) {
        result = -1;
        raise(SIGTERM);
        goto DONE;
    }
    memset(pGISBData, 0, sizeof(*pGISBData));
    if (logData.data() == NULL) {
        goto DONE;
    }

    while (getLine(lineNumber, input)) {
        lineNumber++;
        if (input[0] == '\0') {
            continue;
        }

        memset(pentry, 0, sizeof(*pentry));
        count = 0;
        pos = 0;
        while (count < 3) {
            pos = input.find("0x", pos);
            pos +=2;
            switch (count) {
                case 0:
                    piece = input.substr(pos, 12).c_str();
                    temp = strtoull(piece, &endp, 16);
                    if (*endp == '\0') {
                        pentry->timeStamp = temp;
                    }
                    break;
                case 1:
                    piece = input.substr(pos, 8).c_str();
                    temp = strtoull(piece, &endp, 16);
                    if (*endp == '\0') {
                        pentry->dataWord = (uint32_t)temp;
                    }
                    break;
                case 2:
                    piece = input.substr(pos, 8).c_str();
                    temp = strtoull(piece, &endp, 16);
                    if (*endp == '\0') {
                        pentry->index = (uint32_t)temp;
                    }
                    break;
                default:
                    break;
            }
            count++;
        }
        input.erase();
        rc = parse32BitTaskInfo(pentry->dataWord, pGISBData);
        if (rc == TASK_PARTIALLY_PARSED) {
            pGISBData->timeStamp = pentry->timeStamp;
        }
        else if (rc == TASK_FULLY_PARSED) {
            float current;
            pTaskData->taskInfo.CPUID = pGISBData->taskInfo.CPUID;
            pTaskData->taskInfo.moduleID = pGISBData->taskInfo.moduleID;
            pTaskData->taskInfo.taskID = pGISBData->taskInfo.taskID;
            string cpuPer = to_string(pGISBData->taskInfo.CPUperIntegral) + "." + to_string(pGISBData->taskInfo.CPUperFraction);
            current = strtof(cpuPer.c_str(),NULL);
            pTaskData->taskInfo.CPUperAccumulated = current;
            pTaskData->taskInfo.taskStatus = pGISBData->taskInfo.taskStatus;
            if (pGISBData->taskType == NRT_TASKS) {
                pTaskData->taskType = NRT_TASKS;
                pTaskData->taskInfo.priority = pGISBData->taskInfo.priority;
            }
            else if (pGISBData->taskType == RT_TASKS) {
                pTaskData->taskType = RT_TASKS;
                pTaskData->taskInfo.taskLoadAccumalated = pGISBData->taskInfo.taskLoad;
            }
            pTaskData->timeStamp = pGISBData->timeStamp;
            pTaskData->refreshed = true;
            updateTaskMap(pTaskData);
            memset(pGISBData, 0, sizeof(*pGISBData));
        }
        else if (rc == PARSE_FAILED) {
            continue;
        }
    }
    result = cpuTasksMap.size();
DONE:
    delete pentry;
    delete pGISBData;
    delete pTaskData;
    return result;
}

/*
*Return value: 0 - Success, new task Data partially received
*                     1 - Success,task data completely received
*                   -1 -Failure
*/
parseError AstraPerf::parse32BitTaskInfo(
    uint32_t dataWord,
    traceLogEntryData *parsed_data
    )
{
    if (EXTRACT_BITS(dataWord,
                    TASK_INFO_32b_SEQUENCE_MASK,
                    TASK_INFO_32b_SEQUENCE_SHIFT)) {
        struct fourBitField {
            unsigned x: 4;
        }nibble;
        nibble.x = EXTRACT_BITS(dataWord,
            TASK_INFO_32b_MODULE_ID_MASK,
            TASK_INFO_32b_MODULE_ID_SHIFT);
        if (SCHEDULER_MODULE_ID == nibble.x) {
        /*Data with sequence 1 received*/
            if (!parsed_data->partiallyParsed) {
                /*Wait for TraceLog entry with sequence 0*/
                return PARSE_FAILED;
            }
            else {
                struct sevenBitField {
                    unsigned x: 7;
                }tempSevenBit;
                tempSevenBit.x = EXTRACT_BITS(dataWord,
                        TASK_INFO_32b_CPU_PERCENTAGE_INTEGRAL_MASK,
                        TASK_INFO_32b_CPU_PERCENTAGE_INTEGRAL_SHIFT);
                nibble.x = EXTRACT_BITS(dataWord,
                        TASK_INFO_32b_CPU_PERCENTAGE_FRACTION_MASK,
                        TASK_INFO_32b_CPU_PERCENTAGE_FRACTION_SHIFT);
                /*Reject entries with CPU % value greater than 100*/
                if (((tempSevenBit.x == 100) && (nibble.x > 0)) || (tempSevenBit.x > 100)) {
                    return PARSE_FAILED;
                }
                parsed_data->taskInfo.CPUperIntegral = tempSevenBit.x;
                parsed_data->taskInfo.CPUperFraction = nibble.x;
                parsed_data->taskInfo.taskStatus = EXTRACT_BITS(dataWord,
                        TASK_INFO_32b_TASK_STATUS_MASK,
                        TASK_INFO_32b_TASK_STATUS_SHIFT);
                if (parsed_data->taskType == RT_TASKS) {
                    parsed_data->taskInfo.taskLoad = EXTRACT_BITS(dataWord,
                            TASK_INFO_32b_TASK_LOAD_MASK,
                            TASK_INFO_32b_TASK_LOAD_SHIFT);
                }
                return TASK_FULLY_PARSED;
            }
        } else {
            return PARSE_FAILED;
        }
    }
    else {
        /*Data with sequence 0 received*/
        struct fourBitField {
            unsigned x: 4;
        }nibble;
        nibble.x = EXTRACT_BITS(dataWord,
            TASK_INFO_32b_MODULE_ID_MASK,
            TASK_INFO_32b_MODULE_ID_SHIFT);
        if (SCHEDULER_MODULE_ID == nibble.x) {
            struct tenBitField{
                unsigned x: 10;
            }temp;
            temp.x = EXTRACT_BITS(dataWord,
                        TASK_INFO_32b_TASK_PRIORITY_MASK,
                        TASK_INFO_32b_TASK_PRIORITY_SHIFT);
            if (!temp.x) {
                parsed_data->taskType = RT_TASKS;
            }
            else {
                parsed_data->taskType = NRT_TASKS;
                parsed_data->taskInfo.priority = temp.x;
            }
            parsed_data->taskInfo.moduleID = nibble.x;
            parsed_data->taskInfo.CPUID = EXTRACT_BITS(dataWord,
                    TASK_INFO_32b_CPU_ID_MASK,
                    TASK_INFO_32b_CPU_ID_SHIFT);
            parsed_data->taskInfo.taskID = EXTRACT_BITS(dataWord,
                    TASK_INFO_32b_TASK_ID_MASK,
                    TASK_INFO_32b_TASK_ID_SHIFT);
            parsed_data->partiallyParsed = true;
            return TASK_PARTIALLY_PARSED;
        }else {
            return PARSE_FAILED;
        }
    }
}

/**
 *  Function: This function updates cpuTasksMap and tasksMap structure with the information obtained from parsing two
 *  32 bit entries for a task.
 **/
void AstraPerf :: updateTaskMap(
    taskData *pTaskData
    )
{
    lock_guard<mutex> guard(tasksMapMutex);
    /*update cpuTasksMap*/
    set<uint16_t>& tempSet = cpuTasksMap[(uint8_t)pTaskData->taskInfo.CPUID];
    tempSet.insert(pTaskData->taskInfo.taskID);
    /*Update tasksMap*/
    taskData& temp = tasksMap[(uint16_t)pTaskData->taskInfo.taskID];
    temp.numberOfEntries++;
    if (temp.taskInfo.CPUID != pTaskData->taskInfo.CPUID) {
        set<uint16_t>& tempSet = cpuTasksMap[(uint8_t)temp.taskInfo.CPUID];
        tempSet.erase(pTaskData->taskInfo.taskID);
        temp.taskInfo.CPUID = pTaskData->taskInfo.CPUID;
    }
    temp.taskInfo.moduleID = pTaskData->taskInfo.moduleID;
    temp.taskInfo.taskID = pTaskData->taskInfo.taskID;
    temp.taskInfo.taskStatus = pTaskData->taskInfo.taskStatus;
    temp.taskInfo.CPUperAccumulated += pTaskData->taskInfo.CPUperAccumulated;
    temp.taskType = pTaskData->taskType;
    if (temp.taskType == NRT_TASKS) {
        temp.taskInfo.priority = pTaskData->taskInfo.priority;
    }
    else if (temp.taskType == RT_TASKS) {
        temp.taskInfo.taskLoadAccumalated += pTaskData->taskInfo.taskLoadAccumalated;
    }
    temp.timeStamp = pTaskData->timeStamp;
    temp.refreshed = true;
}

void AstraPerf :: resetDataBase(
    void
    )
{
    set<uint8_t>::iterator coreIt;
    lock_guard<mutex> guard(tasksMapMutex);
    for (coreIt = cpuCores.begin(); coreIt != cpuCores.end(); ++coreIt) {
        set<uint16_t>& tempSet = cpuTasksMap[*coreIt];
        set<uint16_t>::iterator tempIterator;
        for (tempIterator = tempSet.begin(); tempIterator != tempSet.end(); ++tempIterator) {
            taskData& temp = tasksMap[*tempIterator];
            if (temp.refreshed ) {
                temp.numberOfEntries = 0;
                temp.refreshed = false;
                temp.taskInfo.CPUperAccumulated = 0;
                temp.taskInfo.taskLoadAccumalated = 0;
            }
            else {
                /*Remove task entry from map if no entries received for this task*/
                tasksMap.erase(*tempIterator);
                tempSet.erase(tempIterator);
            }
        }
    }
    cpuTasksMap.clear();
}


/**
 *  Function: This function is run on a seperate thread to collect entries from /dev/tracelog and parse them every 1 second.
 **/
static void *dataFetchThread(
    void *data
    )
{
    int rc;
    /*Number of consecutive iterations without entries*/
    uint8_t noEntriesIteration = 0;
    AstraPerf *pInstance = (AstraPerf *)data;

    while (gSignalStatus == eRUNNING) {
        dataFetchCount++;
        if (dataFetchCount >= MAX_DATA_FETCHES) {
            pInstance->resetDataBase();
            dataFetchCount = 0;
            DEBUG_PRINT("DataBase was reset\n");
        }
        if((rc = pInstance->captureEntries()) < 0)
            continue;
        if ((rc = pInstance->parseEntries()) < 0)
            continue;
        else if (rc == 0) {
            noEntriesIteration++;
            if (noEntriesIteration >= MAX_NO_ENTRIES_COUNT) {
                raise(SIGTERM);
                DEBUG_PRINT("Astra kernel is not writing entries in /dev/tracelog. Exiting program..\n");
                break;
            }
        }
        else
            noEntriesIteration = 0;
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    lock_guard<mutex> lk(gAppMutex);
    terminated = true;
    cv.notify_one();
}

/**
 *  Function: This function is the signal handler to handle SIGTERM signal.
 **/
static void signalHandler (
    int sig,
    siginfo_t *siginfo,
    void *context
    )
{
    DEBUG_PRINT("Sending PID: %ld, UID: %ld\n",
            (long)siginfo->si_pid, (long)siginfo->si_uid);
    gSignalStatus = siginfo->si_signo;
}

/**
 *  Function: Entry point for astratop_server process.
 **/
int main(
    void
    )
{
    AstraPerf *pInstance = new AstraPerf();
    gSignalStatus = eRUNNING;
    dataFetchCount = 0;
    terminated = false;
    pthread_t dataGatheringThreadId = 0;
    void     *(*threadFunc)( void * );
    int rc;
    struct sigaction act;

    memset (&act, '\0', sizeof(act));

    /* Use the sa_sigaction field because the handler has two additional parameters */
    act.sa_sigaction = &signalHandler;

    /* The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler. */
    act.sa_flags = SA_SIGINFO;

    if (sigaction(SIGTERM, &act, NULL) < 0) {
        DEBUG_PRINT("SIGTERM handler not set!!!\n");
        return 1;
    }
    if (rc = (pInstance->getCPUCoresInfo()) < 0) {
        return 0;
    }
    if (pInstance->init()) {
        DEBUG_PRINT("Astra Top init failed\n");
        return 0;
    }
    threadFunc = dataFetchThread;
    if (pthread_create( &dataGatheringThreadId, NULL, threadFunc, pInstance)) {
        DEBUG_PRINT( "%s: could not create thread for data gathering; %s\n", __FUNCTION__, strerror( errno ));
    }
    else {
        startServer(pInstance);
        unique_lock<mutex> lk(gAppMutex);
        cv.wait(lk, []{return terminated;});
        pInstance->unInit();
        delete pInstance;
    }
}

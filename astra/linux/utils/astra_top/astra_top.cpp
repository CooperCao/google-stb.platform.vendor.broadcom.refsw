/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

/***************************************************************************
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * file : TraceLogAnalyzer.cpp
 *
 ***************************************************************************/
#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdint.h>
#include <cstring>
#include <unistd.h>
#include <map>
#include <vector>
#include <csignal>
#include <atomic>
#include <string>
#include <chrono>
#include <thread>
#include <fcntl.h>
#include <set>
#include <algorithm>
#include "CmdLine.h"
#include "Parser.h"

using namespace std;
#define BUFFSIZE 1024
#define TRACELOGGER_FILE_PATH "/dev/tracelog"
#define FILE_ENTRY_COUNT 10
#define TRACELOG_ENTRY_SIZE 50
/*32 bit bitMask 'CPUCoresBitMask' is used to save the number of active cores along with their IDs. Hence MAX_NUM_CPU_CORES = 32*/
#define MAX_NUM_CPU_CORES 32
#define MAX_NO_ENTRIES_COUNT 10

typedef int parseError;

#define TASK_PARTIALLY_PARSED            0
#define TASK_FULLY_PARSED                1
#define PARSE_FAILED                    -1

struct TraceLogEntry {
    uint64_t timestamp;
    uint32_t dataWord;
    uint32_t index;
};

struct TraceLogEntryData {
    TaskInformation taskInfo;
    eTaskType taskType;
    uint64_t timeStamp;
    bool partiallyParsed;
};

struct DisplayTaskInformation {
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

enum eInMode {
    ePIPE,
    eFILE
};

struct TaskData {
    DisplayTaskInformation taskInfo;
    eTaskType taskType;
    uint64_t timeStamp;
    int numberOfEntries;
    bool refreshed;
};

enum appState {
    /*A non-standard signal value*/
    eRUNNING = 32,
};


class Analyzer: public ICmdLineInterface {
private:
    bool isInitialized;
    fstream *pLogFile;
    int traceLogFD;
    map<uint16_t, struct TaskData> tasksMap;
    map<uint8_t, set<uint16_t>> cpuTasksMap;
    map<string, uint8_t> fieldWidthMap;
    vector<char> logData;
    char *logDataEnd;
    char *buf;
    uint32_t prevLinesCount;
    uint32_t curLinesCount;
    uint8_t maxWidth;
    void updateTaskMap(TaskData *pGISBData);
    parseError parse32BitTaskInfo(uint32_t dataWord, TraceLogEntryData *parsed_data);
    Analyzer(const Analyzer&);
    Analyzer& operator = (const Analyzer&);
    void newLine();
    bool getLine(uint32_t lineNumber, string& line);
public:
    ios_base::fmtflags oldFlags;
    struct CommonOptions options;
    static set<uint8_t> cpuCores;
    Analyzer()
    {
        isInitialized = false;
        buf = new char[BUFFSIZE];
    };
    ~Analyzer()
    {
        delete buf;
        if (pLogFile)
            delete pLogFile;
    }
    eInMode inMode;
    int init();
    void uninit();
    int captureEntries();
    int parseEntries();
    int getCPUCoresInfo();
    void clearPrevDump();
    void dumpTaskDataToConsole(int cpuID);
    int displayTaskTableFields(int cpuID);
};

volatile std::sig_atomic_t gSignalStatus;
void signal_handler(int signal)
{
    gSignalStatus = signal;
}

set<uint8_t> Analyzer::cpuCores;

int main(int argc, const char *argv[])
{
    Analyzer *pInstance = new Analyzer();
    int rc;
    set<uint8_t>::iterator coreIt;
    uint8_t dataFetchIteration = 0;
    uint8_t sleepDuration = 1;
    bool firstIteration = true;
    /*Number of consecutive iterations without entries*/
    uint8_t noEntriesIteration = 0;
    if (rc = (pInstance->getCPUCoresInfo()) < 0) {
        return 0;
    }

    {
        uint32_t CPUCoresBitMask;
        CPUCoresBitMask = 0;
        for (coreIt = Analyzer::cpuCores.begin(); coreIt != Analyzer::cpuCores.end(); ++coreIt) {
            CPUCoresBitMask |= (1 << (*coreIt));
        }
        pInstance->setMaxCoresBitMask(CPUCoresBitMask);
    }

    if (pInstance->cmdLineParse(argc, argv)) {
        return 0;
    }

    if (pInstance->init()) {
        cout << " TraceLog analyser init failed" << endl;
        return 0;
    }

    pInstance->oldFlags = cout.flags();
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    gSignalStatus = eRUNNING;

    pInstance->options.duration = pInstance->getOptions().duration;
    pInstance->options.taskID = pInstance->getOptions().taskID;
    pInstance->options.cores = pInstance->getOptions().cores;
    pInstance->options.taskType = pInstance->getOptions().taskType;
    pInstance->options.dispType = pInstance->getOptions().dispType;

    /*Hide cursor*/
    cout << "\e[?25l";
    /*Clear screen*/
    cout << "\x1B[2J";
    /*Move cursor to home position*/
    cout << "\x1B[H";

    if (pInstance->inMode == eFILE ) {
        sleepDuration = pInstance->options.duration;
    }

    while (gSignalStatus == eRUNNING) {
        if ((pInstance->inMode == ePIPE) && ((rc = pInstance->captureEntries()) < 0)) {
            continue;
        }

        if ((rc = pInstance->parseEntries()) < 0)
            continue;
        else if (rc == 0) {
            noEntriesIteration++;
            if (noEntriesIteration >= MAX_NO_ENTRIES_COUNT) {
                cout << "Astra kernel is not writing entries in /dev/tracelog. Exiting program.." << endl;
                break;
            }
        }
        else
            noEntriesIteration = 0;

        if (pInstance->inMode == ePIPE ) {
            dataFetchIteration++;
            if (false == firstIteration) {
                if (dataFetchIteration < pInstance->options.duration) {
                    goto SLEEP;
                }
                else
                    dataFetchIteration = 0;
            }
            else {
                firstIteration = false;
                dataFetchIteration = 0;
            }
        }

        if (pInstance->options.dispType == eREFRESH)
            cout << "\x1B[H";

        if (pInstance->options.taskID >= 0) {
            if (!(rc = pInstance->displayTaskTableFields(-1))) {
                pInstance->dumpTaskDataToConsole(-1);
            }
        }
        else {
            for (coreIt = Analyzer::cpuCores.begin(); coreIt != Analyzer::cpuCores.end(); ++coreIt) {
                if (pInstance->options.cores & (1 << *coreIt)) {
                    if ((rc = pInstance->displayTaskTableFields(*coreIt)) < 0)
                        continue;
                    pInstance->dumpTaskDataToConsole(*coreIt);
                }
            }
        }

        if (pInstance->options.dispType == eREFRESH)
            pInstance->clearPrevDump();

SLEEP:
        sleep(sleepDuration);
    }

    /*To show cursor*/
    cout << "\e[?25h";
    pInstance->uninit();
    cout.flags(pInstance->oldFlags);
    delete pInstance;
    return 0;
}

inline void Analyzer::newLine()
{
    cout << endl;
    curLinesCount++;
}

/*Clear extra lines if any from previous dump*/
void Analyzer::clearPrevDump()
{
    int count = prevLinesCount - curLinesCount;
    for (int i = 0; i < count; i++) {
        /*Clears line till newline*/
        cout << "\x1B[K";
        cout << endl;
    }
    prevLinesCount = curLinesCount;
    curLinesCount = 0;
}

int Analyzer::displayTaskTableFields(int cpuID)
{
    uint8_t i ,j;
    vector <string> fieldSequence;

    if (cpuID >= 0) {
        fieldSequence.push_back("Task ID");
        if (options.taskType & eNRT) {
            fieldSequence.push_back("Priority");
        }
        if (options.taskType & eRT) {
            fieldSequence.push_back("Load");
        }
    }
    else {
        fieldSequence.push_back("CPU ID");
        if (eNRT == tasksMap[options.taskID].taskType) {
            fieldSequence.push_back("Priority");
        }
        else if (eRT == tasksMap[options.taskID].taskType) {
            fieldSequence.push_back("Load");
        }
    }
    fieldSequence.push_back("CPU %");
    fieldSequence.push_back("Status");

    int total_width = 0;
    for (auto it = fieldSequence.cbegin(); it != fieldSequence.cend(); ++it) {
        total_width += fieldWidthMap[*it];
        /*To account for space in between*/
        total_width++;
    }

    cout << setw(total_width) << " ";
    newLine();

    if (cpuID >= 0) {
        cout << setw(4) << "CPU " << setw(2) << cpuID << setw(25) << ": Total number of tasks:";
        map<uint8_t, set<uint16_t>>::iterator tempIterator;
        tempIterator = cpuTasksMap.find((uint8_t)cpuID);
        if (tempIterator == cpuTasksMap.end()) {
            cout << setw(total_width - 31)<< "0";
            newLine();
            goto ERROR;
        }
        cout << setw(total_width - 31) << tempIterator->second.size();
        newLine();
    }
    else {
        cout << setw(9) << "Task ID:" << setw(total_width - 9) << options.taskID;
        newLine();
        map<uint16_t, struct TaskData>::iterator tempIterator;
        tempIterator = tasksMap.find((uint16_t)options.taskID);
        if (tempIterator == tasksMap.end() || !(tempIterator->second.refreshed)) {
            if (tempIterator != tasksMap.end()) {
                set<uint16_t>& tempSet = cpuTasksMap[tempIterator->second.taskInfo.CPUID];
                tempSet.erase((uint16_t)options.taskID);
                tasksMap.erase((uint16_t)options.taskID);
            }
            cout << setw(total_width)<< "Task is not scheduled.";
            newLine();
            goto ERROR;
        }
    }

    for (auto it = fieldSequence.cbegin(); it != fieldSequence.cend(); ++it) {
        cout << " ";
        for (j = 0; j < fieldWidthMap[*it]; j++) {
            cout << "-";
        }
    }
    newLine();

    for (auto it = fieldSequence.cbegin(); it != fieldSequence.cend(); ++it) {
        cout << " " << setw(fieldWidthMap[*it]) << *it;
    }
    newLine();

    for (auto it = fieldSequence.cbegin(); it != fieldSequence.cend(); ++it) {
        cout << " ";
        for (j = 0; j < fieldWidthMap[*it]; j++) {
            cout << "-";
        }
    }
    newLine();
    return 0;
ERROR:
    return -1;
}

void Analyzer::dumpTaskDataToConsole(int cpuID)
{
    if (!isInitialized) {
        cout << "Analyser not initialized" << endl;
        raise(SIGTERM);
        return;
    }

    if (cpuID >= 0) {
        set<uint16_t>& tempSet = cpuTasksMap[(uint8_t)cpuID];
        set<uint16_t>::iterator tempIterator;
        for (tempIterator = tempSet.begin(); tempIterator != tempSet.end(); ++tempIterator) {
            TaskData& temp = tasksMap[*tempIterator];
            if (temp.refreshed) {
                if (options.taskType & temp.taskType) {
                    cout << " " << setw(fieldWidthMap["Task ID"]) << temp.taskInfo.taskID;
                    if (temp.taskType == eNRT) {
                        cout << " " << setw(fieldWidthMap["Priority"]) << temp.taskInfo.priority;
                        if (options.taskType & eRT)
                            cout << " " << setw(fieldWidthMap["Load"]) << "NA";
                    }
                    else if (temp.taskType == eRT) {
                        if (options.taskType & eNRT)
                            cout  << " " << setw(fieldWidthMap["Priority"]) << "RT";
                        cout  << " " << setw(fieldWidthMap["Load"]) << (temp.taskInfo.taskLoadAccumalated/temp.numberOfEntries);
                    }
                    cout << " " << setw(fieldWidthMap["CPU %"]) << (temp.taskInfo.CPUperAccumulated/temp.numberOfEntries)<< " " <<
                            setw(fieldWidthMap["Status"]) << temp.taskInfo.taskStatus;
                    newLine();
                }
                temp.numberOfEntries = 0;
                temp.refreshed = false;
                temp.taskInfo.CPUperAccumulated = 0;
                temp.taskInfo.taskLoadAccumalated = 0;
            }
            else {
                /*Remove task entry from map if no entries seen in the log*/
                tasksMap.erase(*tempIterator);
                tempSet.erase(tempIterator);
            }
        }
    }
    else {
        TaskData& temp = tasksMap[options.taskID];
        cout << " " << setw(fieldWidthMap["CPU ID"]) << temp.taskInfo.CPUID;
        if (temp.taskType == eNRT)
            cout << " " << setw(fieldWidthMap["Priority"]) << temp.taskInfo.priority;
        else if (temp.taskType == eRT)
            cout << " " << setw(fieldWidthMap["Load"]) << (temp.taskInfo.taskLoadAccumalated/temp.numberOfEntries);
        cout << " " << setw(fieldWidthMap["CPU %"]) << (temp.taskInfo.CPUperAccumulated/temp.numberOfEntries)<< " " <<
                        setw(fieldWidthMap["Status"]) << temp.taskInfo.taskStatus;
        newLine();
        temp.refreshed = false;
        temp.numberOfEntries = 0;
        temp.taskInfo.CPUperAccumulated = 0;
        temp.taskInfo.taskLoadAccumalated = 0;
    }
}

int Analyzer::init()
{
    cout.flags ( std::ios::left | std::ios::dec | std::ios::showbase );
    if (isInitialized) {
        cout << " Analyser already initialized" << endl;
        return -1;
    }

    if (getOptions().inFileName[0]) {
        pLogFile = new fstream;
        pLogFile->open(getOptions().inFileName);
        if (pLogFile->rdstate() & pLogFile->fail()) {
            cout << " Error in opening log file " << getOptions().inFileName << endl;
            goto ERROR;
        }
        inMode = eFILE;
    }
    else {
        traceLogFD = open(TRACELOGGER_FILE_PATH, 0);
        if (traceLogFD == -1) {
            cout << " Error in opening /dev/tracelog" << endl;
            goto ERROR;
        }
        close(traceLogFD);
        inMode = ePIPE;
    }

    fieldWidthMap["Task ID"] = 7;
    fieldWidthMap["Priority"] = 8;
    fieldWidthMap["Load"] = 6;
    fieldWidthMap["CPU %"] = 10;
    fieldWidthMap["Status"] = 6;
    fieldWidthMap["CPU ID"] = 6;
    maxWidth = fieldWidthMap["Task ID"] + fieldWidthMap["Priority"] + fieldWidthMap["Load"] + fieldWidthMap["CPU %"] + fieldWidthMap["Status"];
    prevLinesCount = curLinesCount = 0;
    isInitialized = true;
    return 0;
ERROR:
    if (pLogFile) {
        if (pLogFile->is_open()) {
            pLogFile->close();
        }
    }
    isInitialized = false;
    return -1;
}

void Analyzer::uninit(void)
{
    if (!isInitialized) {
        cout << "Analyser not initialized" << endl;
        return ;
    }
    if (pLogFile) {
        if (pLogFile->is_open()) {
            pLogFile->close();
        }
        delete pLogFile;
        pLogFile = NULL;
    }

    tasksMap.clear();
    cpuTasksMap.clear();
    fieldWidthMap.clear();
    logData.clear();
    isInitialized = false;
}

void Analyzer:: updateTaskMap(TaskData *pTaskData)
{
    /*update cpuTasksMap*/
    set<uint16_t>& tempSet = cpuTasksMap[(uint8_t)pTaskData->taskInfo.CPUID];
    tempSet.insert(pTaskData->taskInfo.taskID);
    /*Update tasksMap*/
    TaskData& temp = tasksMap[(uint16_t)pTaskData->taskInfo.taskID];
    temp.numberOfEntries++;
    if (temp.taskInfo.CPUID != pTaskData->taskInfo.CPUID) {
        set<uint16_t>& tempSet = cpuTasksMap[(uint8_t)temp.taskInfo.CPUID];
        tempSet.erase(pTaskData->taskInfo.taskID);
    }
    temp.taskInfo.CPUID = pTaskData->taskInfo.CPUID;
    temp.taskInfo.moduleID = pTaskData->taskInfo.moduleID;
    temp.taskInfo.taskID = pTaskData->taskInfo.taskID;
    temp.taskInfo.taskStatus = pTaskData->taskInfo.taskStatus;
    temp.taskInfo.CPUperAccumulated += pTaskData->taskInfo.CPUperAccumulated;
    temp.taskType = pTaskData->taskType;
    if (temp.taskType == eNRT) {
        temp.taskInfo.priority = pTaskData->taskInfo.priority;
    }
    else if (temp.taskType == eRT) {
        temp.taskInfo.taskLoadAccumalated += pTaskData->taskInfo.taskLoadAccumalated;
    }
    temp.timeStamp = pTaskData->timeStamp;
    temp.refreshed = true;
}

int Analyzer::captureEntries()
{
    size_t rn;
    traceLogFD = open(TRACELOGGER_FILE_PATH, 0);
    if (traceLogFD == -1) {
        cout << setw(maxWidth) << "Error in opening " <<  TRACELOGGER_FILE_PATH << endl;
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

int Analyzer::getCPUCoresInfo()
{
    vector<char> cpuInfoLog;
    size_t rn;
    int cpuInfoFD;
    size_t pos = 0;
    unsigned long coreID;
    char *endp;
    const char *core;

    cpuInfoFD = open("/proc/cpuinfo", 0);
    if (cpuInfoFD == -1) {
        cout << "Error in opening " <<  "/proc/cpuinfo" << endl;
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
}

bool Analyzer::getLine(uint32_t lineNumber, string& line)
{
    bool result = true;
    if (inMode == ePIPE) {
        if ((logData.data() + (TRACELOG_ENTRY_SIZE * (lineNumber + 1))) <= logDataEnd) {
            line = string(logData.data() + (lineNumber*TRACELOG_ENTRY_SIZE), TRACELOG_ENTRY_SIZE);
        }
        else
            result = false;
    }
    else {
        if (!getline(*pLogFile, line))
            result = false;
        else {
            if (pLogFile->rdstate() & pLogFile->fail()) {
                cout << setw(maxWidth) << "Error reading file";
                newLine();
                line[0] = '\0';
            }
        }
    }
    return result;
}

/*On success, return no of tasks for which entries were received.
On failure, return -1*/
int Analyzer::parseEntries()
{
    TraceLogEntry *pentry = 0;
    TraceLogEntryData *pGISBData = 0;
    TaskData *pTaskData;
    int count = 0,entry_count = 0, result = 0;
    parseError rc;
    string input = new char[400];
    pentry = new TraceLogEntry;
    pGISBData = new TraceLogEntryData;
    pTaskData = new TaskData;
    unsigned long long int temp;
    char *endp;
    size_t pos = 0;
    uint32_t lineNumber = 0;
    const char* piece;

    if (!isInitialized) {
        cout << setw(maxWidth) << "Analyser not initialized" << endl;
        result = -1;
        raise(SIGTERM);
        goto DONE;
    }
    memset(pGISBData, 0, sizeof(*pGISBData));
    cpuTasksMap.clear();
    if (inMode ==ePIPE && logData.data() == NULL) {
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
                        pentry->timestamp = temp;
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
            pGISBData->timeStamp = pentry->timestamp;
        }
        else if (rc == TASK_FULLY_PARSED) {
            /*Filter out cores / task as per user options*/
            if ((options.cores & (1 << pGISBData->taskInfo.CPUID)) || (options.taskID == pGISBData->taskInfo.taskID)) {
                float current;
                pTaskData->taskInfo.CPUID = pGISBData->taskInfo.CPUID;
                pTaskData->taskInfo.moduleID = pGISBData->taskInfo.moduleID;
                pTaskData->taskInfo.taskID = pGISBData->taskInfo.taskID;
                string cpuPer = to_string(pGISBData->taskInfo.CPUperIntegral) + "." + to_string(pGISBData->taskInfo.CPUperFraction);
                current = strtof(cpuPer.c_str(),NULL);
                pTaskData->taskInfo.CPUperAccumulated = current;
                pTaskData->taskInfo.taskStatus = pGISBData->taskInfo.taskStatus;
                if (pGISBData->taskType == eNRT) {
                    pTaskData->taskType = eNRT;
                    pTaskData->taskInfo.priority = pGISBData->taskInfo.priority;
                }
                else if (pGISBData->taskType == eRT) {
                    pTaskData->taskType = eRT;
                    pTaskData->taskInfo.taskLoadAccumalated = pGISBData->taskInfo.taskLoad;
                }
                pTaskData->timeStamp = pGISBData->timeStamp;
                pTaskData->refreshed = true;
                updateTaskMap(pTaskData);
                if ((inMode == eFILE) && (++entry_count == FILE_ENTRY_COUNT)) {
                    result = cpuTasksMap.size();
                    goto DONE;
                }
            }
            memset(pGISBData, 0, sizeof(*pGISBData));
        }
        else if (rc == PARSE_FAILED) {
            continue;
        }
    }

    if (inMode ==eFILE)
        raise(SIGTERM);
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
parseError Analyzer::parse32BitTaskInfo(uint32_t dataWord, TraceLogEntryData *parsed_data)
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
                if (parsed_data->taskType == eRT) {
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
                parsed_data->taskType = eRT;
            }
            else {
                parsed_data->taskType = eNRT;
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

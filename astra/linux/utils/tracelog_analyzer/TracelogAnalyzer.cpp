/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#include <cstdlib>
#include <cstring>
#include <list>

#include "CmdLine.h"
#include "Parser.h"

using namespace std;

struct TraceLogEntry {
    union {
        GISBBus gisb;
        SCBBus scb;
    };
    BusType busType;
    int entryNumber;
};

class Analyzer: public ICmdLineInterface {

private:
    bool isInitialized;
    ifstream *pInFile;
    ofstream *pOutFile;

    void parseEntries(void);
    list<struct TraceLogEntry> logList;

    unsigned int word[4];
    Raw128b raw128;
    int parse128Bit(void);

    Analyzer(const Analyzer&);
    Analyzer& operator = (const Analyzer&);

public:
    Analyzer()
    {
        pInFile = new ifstream;
        pOutFile = new ofstream;
        isInitialized = false;
    };

    ~Analyzer()
    {
        if(pInFile)
            delete pInFile;

        if(pOutFile)
            delete pOutFile;
    }

    int init(const char *inFileName, const char *outFileName);
    void uninit();
    int startAnalyzer(void);
    void dumpEntries(void);
};

int main(int argc, const char *argv[])
{
    Analyzer *pInstance = new Analyzer();

    if (pInstance->cmdLineParse(argc, argv)) {
        return 0;
    }

    if (pInstance->cmdLineProbe()) {
        return 0;
    }


    if(pInstance->init((pInstance->getOpions()).inFileName, (pInstance->getOpions()).outFileName)) {
        cout << " TraceLog analyser init failed" << endl;
        return 0;
    }

    if(pInstance->startAnalyzer()) {
        cout << " TraceLog Analyser Error" << endl;
    }

    pInstance->dumpEntries();

    pInstance->uninit();

    delete pInstance;
    return 0;
}

void Analyzer::dumpEntries(void)
{
    list<struct TraceLogEntry>::iterator tempIterator;

    if (!isInitialized) {
        cout << "Analyser not initialized" << endl;
        return ;
    }

    pOutFile->flags(ios::left | ios::hex | ios::showbase);

    pOutFile->width(14);
    *pOutFile << "Serial No   |" << setw(14) << "Bus Type    |" <<
        setw(14) << "Client Id   |" << setw(14) << "Read/Write  |" <<
        setw(14) << "Address     |" << setw(14) << "Data        |" <<
        setw(18) << "Timestamp   |" << endl << endl;

    for(tempIterator = logList.begin(); tempIterator != logList.end(); ++tempIterator)
    {
        pOutFile->width(14);
        if (eGISB == tempIterator->busType) {
            *pOutFile << tempIterator->entryNumber <<
                setw(14) << tempIterator->busType <<
                setw(14) << tempIterator->gisb.clientId <<
                setw(14) << (tempIterator->gisb.typeWrite ? "Write" : "Read" ) <<
                setw(14) << tempIterator->gisb.addressLower <<
                setw(14) << tempIterator->gisb.dataWord <<
                setw(18) << (uint64_t)(((uint64_t)tempIterator->gisb.timeStampUpper << 32) +
                (tempIterator->gisb.timeStampLower)) <<
                endl;
        }
        else {
            *pOutFile << tempIterator->entryNumber <<
                setw(14) << tempIterator->busType <<
                setw(14) << tempIterator->scb.clientId <<
                setw(14) << (tempIterator->gisb.typeWrite ? "Write" : "Read" ) <<
                setw(14) << tempIterator->scb.addressLower <<
                setw(14) << tempIterator->scb.accessId <<
                setw(18) << (uint64_t)(((uint64_t)tempIterator->gisb.timeStampUpper << 32) +
                (tempIterator->scb.timeStampLower)) <<
                endl;
        }

        if (pOutFile->rdstate() & ifstream::badbit) {
            cout << " Error writing output file" << endl;
            continue;
        }
    }
}

int Analyzer::startAnalyzer(void)
{
    if (!isInitialized) {
        cout << "Analyser not initialized" << endl;
        return -1;
    }

    parseEntries();
    return 0;
}

int Analyzer::init(const char *inFileName, const char *outFileName)
{
    if (isInitialized) {
        cout << " Analyser already initialized" << endl;
        return -1;
    }

    if (!pInFile->is_open()) {
        if (inFileName)
            pInFile->open(inFileName);
        else {
            cout << "Invalid input file name" << endl;
            goto ERROR;
        }
    }

    if (pInFile->rdstate() & std::ifstream::failbit) {
        cout << " Error in opening input file " << inFileName << endl;
        goto ERROR;
    }

    if (!pOutFile->is_open()) {

        if (outFileName)
            pOutFile->open(outFileName);
        else {
            cout << "Invalid output file name" << endl;
            goto ERROR;
        }

    }

    if (pOutFile->rdstate() & std::ofstream::failbit) {
        cout << " Error in opening output file " << outFileName << endl;
        goto ERROR;
    }

    isInitialized = true;
    return 0;
ERROR:
    if (pInFile->is_open()) {
        pInFile->close();
    }

    if (pOutFile->is_open()) {
        pOutFile->close();
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

    if (pInFile->is_open()) {
        pInFile->close();
    }

    if (pOutFile->is_open()) {
        pOutFile->close();
    }

    logList.clear();

    isInitialized = false;

    return ;
}

void Analyzer::parseEntries()
{
    TraceLogEntry *entry = 0;
    int rc = 0;
    char *pBuffer = new char[256];
    char *pTempPos = 0;
    entry = new TraceLogEntry;

    if (!isInitialized) {
        cout << "Analyser not initialized" << endl;
        goto DONE;
    }

    while(!pInFile->eof())
    {
        pInFile->getline(pBuffer, 256);

        if ((pInFile->rdstate() & std::ifstream::badbit)) {
            cout << " Error reading input file" << endl;
            continue;
        }

        if (pInFile->rdstate() & std::ifstream::eofbit ) {
            cout << "Read reached end of file" << endl ;
            break;
        }
        word[3] = strtoul(pBuffer, &pTempPos, 16);
        word[2] = strtoul(pTempPos+1, &pTempPos, 16);
        word[1] = strtoul(pTempPos+1, &pTempPos, 16);
        word[0] = strtoul(pTempPos+1, &pTempPos, 16);

        rc = parse128Bit();

        if (0 != rc) {
            cout << "\n BAD trace log entry 128b format data" << endl;
            continue;
        }

        memset(entry, 0, sizeof(*entry));

        switch(raw128.typeIO)
        {
            case 1:
                entry->busType = eGISB;
                entry->gisb.clientId = raw128.clientId;
                entry->gisb.typeWrite = raw128.typeWrite;
                entry->gisb.typeAck = raw128.typeAck;
                entry->gisb.typeIO = raw128.typeIO;
                entry->gisb.typeUBUS = raw128.typeUBUS;
                entry->gisb.lostEvents = raw128.lostEvents;
                entry->gisb.postTrigger =   raw128.postTrigger;
                entry->gisb.valid = raw128.valid;
                entry->gisb.addressLower = raw128.addressLower;
                entry->gisb.dataWord = raw128.dataWord;
                entry->gisb.timeStampUpper = raw128.timeStampUpper;
                entry->gisb.timeStampLower = raw128.timeStampLower;
                break;

            case 0:
                if (raw128.typeUBUS)
                    entry->busType = eUBUS;
                else
                    entry->busType = eSCB;

                entry->scb.clientId = raw128.clientId;
                entry->scb.typeWrite = raw128.typeWrite;
                entry->scb.typeAck = raw128.typeAck;
                entry->scb.typeIO = raw128.typeIO;
                entry->scb.typeUBUS = raw128.typeUBUS;
                entry->scb.lostEvents = raw128.lostEvents;
                entry->scb.postTrigger = raw128.postTrigger;
                entry->scb.valid = raw128.valid;
                entry->scb.addressLower = raw128.addressLower;
                entry->scb.addressUpper = raw128.addressUpper;
                entry->scb.command = raw128.command;
                entry->scb.burstSize = raw128.burstSize;
                entry->scb.accessId = raw128.accessId;
                entry->scb.timeStampUpper = raw128.timeStampUpper;
                entry->scb.timeStampLower = raw128.timeStampLower;
                break;
            default:
                cout << " BAD trace log bus type TypeIO : bit field [11] " <<
                    raw128.typeIO << endl;

                continue;
                break;
        }

        if (logList.empty()) {
            entry->entryNumber = 1;
        }
        else {
            entry->entryNumber = logList.back().entryNumber + 1;
        }
        logList.push_back(*entry);
    }

DONE:
    delete entry;
    delete [] pBuffer;
    return ;
}

int Analyzer::parse128Bit(void)
{
    raw128.clientId = EXTRACT_BITS(
                word[0],
                TRACELOG_128b_CLIENTID_MASK,
                TRACELOG_128b_CLIENTID_SHIFT);
    raw128.typeWrite = EXTRACT_BITS(
                word[0],
                TRACELOG_128b_TYPEWR_MASK,
                TRACELOG_128b_TYPEWR_SHIFT);
    raw128.typeAck = EXTRACT_BITS(
                word[0],
                TRACELOG_128b_TYPEAK_MASK,
                TRACELOG_128b_TYPEAK_SHIFT);
    raw128.typeIO = EXTRACT_BITS(
                word[0],
                TRACELOG_128b_TYPEIO_MASK,
                TRACELOG_128b_TYPEIO_SHIFT);
    raw128.typeUBUS = EXTRACT_BITS(
                word[0],
                TRACELOG_128b_TYPEUBUS_MASK,
                TRACELOG_128b_TYPEUBUS_SHIFT);
    raw128.lostEvents = EXTRACT_BITS(
                word[0],
                TRACELOG_128b_LOSTEVENTS_MASK,
                TRACELOG_128b_LOSTEVENTS_SHIFT);
    raw128.postTrigger = EXTRACT_BITS(
                word[0],
                TRACELOG_128b_POSTTRIGGER_MASK,
                TRACELOG_128b_POSTTRIGGER_SHIFT);
    raw128.valid = EXTRACT_BITS(
                word[0],
                TRACELOG_128b_VALID_MASK,
                TRACELOG_128b_VALID_SHIFT);
    if (0 == raw128.valid) {
        cout << "Invlaid tracelog entry" << endl;
        return -1;
    }

    raw128.timeStampUpper = EXTRACT_BITS(
                word[0],
                TRACELOG_128b_TS_UPPER_MASK,
                TRACELOG_128b_TS_UPPER_SHIFT);
    raw128.timeStampLower = EXTRACT_BITS(
                word[1],
                TRACELOG_128b_TS_LOWER_MASK,
                TRACELOG_128b_TS_LOWER_SHIFT);
    raw128.addressLower = EXTRACT_BITS(
                word[2],
                TRACELOG_128b_ADDR_MASK,
                TRACELOG_128b_ADDR_SHIFT);

    if (0 == raw128.addressLower) {
        cout << "Bad address tracelog entry" << endl;
        return -1;
    }

    switch(raw128.typeIO)
    {
        case 1:
            raw128.dataWord = EXTRACT_BITS(
                        word[3],
                        TRACELOG_128b_DATAWORD_MASK,
                        TRACELOG_128b_DATAWORD_SHIFT);
            break;
        case 0:
            raw128.command = EXTRACT_BITS(
                        word[3],
                        TRACELOG_128b_COMMAND_MASK,
                        TRACELOG_128b_COMMAND_SHIFT);
            raw128.burstSize = EXTRACT_BITS(
                        word[3],
                        TRACELOG_128b_BURST_SIZE_MASK,
                        TRACELOG_128b_BURST_SIZE_SHIFT);
            raw128.accessId = EXTRACT_BITS(
                        word[3],
                        TRACELOG_128b_ACC_ID_MASK,
                        TRACELOG_128b_ACC_ID_SHIFT);
            raw128.addressUpper = EXTRACT_BITS(
                        word[3],
                        TRACELOG_128b_ADDR_UPPER_MASK,
                        TRACELOG_128b_ADDR_UPPER_SHIFT);
            break;
        default:
            cout << " BAD trace log bus type TypeIO  : bit field [11] " <<
                raw128.typeIO <<
                endl;
            return -1;
            break;
    }

    return 0;
}

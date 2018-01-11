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
 * file : CmdLine.cpp
 *
 ***************************************************************************/
#include <iomanip>
#include <stdint.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <getopt.h>
#include "CmdLine.h"

using namespace std;

class CmdLine {
private:
    /* private data */
    static bool instanceFlag;
    static CmdLine *s_instance;
    static struct option longOptions[];
    static int longIndex;
    static uint32_t maxCoresBitMask;
    /* private methods */
    static void printUsage(const char *app);
    /* private constructor */
    CmdLine()
    {
        /*private constructor*/
    }

public:
    /* public data */
    static struct CommonOptions opts;
    /* public methods */
    static void setMaxCoresBitMask(uint32_t bitMask);
    static int cmdLineParse(int argc, const char *argv[]);
    static CmdLine* getInstance();
    /* public destructor */
    ~CmdLine()
    {
        if (instanceFlag) {
            delete s_instance;
        }
        instanceFlag = false;
    }
};

struct option CmdLine::longOptions[] = {
    {"help", no_argument, 0, 'h' },
    {"core", required_argument, 0, 'c'},
    {"task", required_argument, 0, 't'},
    {"filtertask", required_argument, 0, 'f'},
    {"duration", required_argument , 0, 'd'},
    {"infile", required_argument , 0, 'i'},
    {"scroll", no_argument, 0, 's'},
    {0, 0, 0, 0},
};

int CmdLine::longIndex = 0;
struct CommonOptions CmdLine::opts;
bool CmdLine::instanceFlag = false;
CmdLine *CmdLine::s_instance = NULL;
uint32_t CmdLine::maxCoresBitMask = 0;

CmdLine* CmdLine::getInstance()
{
    if (!instanceFlag) {
        s_instance = new CmdLine();
        instanceFlag = true;
        return s_instance;
    }
    else {
        return s_instance;
    }
}

void CmdLine::printUsage(const char *app)
{
    cout << setw(11) << "Usage:" << app << " [-option] [argument]" << endl;
    cout << setw(11) << "Option:" << "-h  |  --help                  show help information" << endl;
    cout << setw(11) << " " << "-c  |  --core <coresFlag>,     monitor selected CPU core/s, " << endl;
    cout << setw(42) << " "  << "coresFlag = decimal number by setting bit/s at position = core ID/s" << endl;
    cout << setw(11) << " " << "-t  |  --task <taskID>,        monitor only one task with id = taskID" << endl;
    cout << setw(11) << " " << "-f  |  --filtertask <RT/NRT>,  monitor real time tasks if RT, monitor non real time tasks if NRT" << endl;
    cout << setw(11) << " " << "-d  |  --duration <duration>,  log entries after every duration(in secs)" << endl;
    cout << setw(11) << " " << "-i  |  --infile <fileName>,    input file with TraceLog entries" << endl;
    cout << setw(11) << " " << "-s  |  --scroll,               output in scrolling style" << endl;
    return;
}

void CmdLine::setMaxCoresBitMask(uint32_t bitMask)
{
    maxCoresBitMask = bitMask;
}

int CmdLine::cmdLineParse(int argc, const char *argv[])
{
    int opt = 0;
    long long int temp;
    char *endp;
    /*Set default duration to 1 sec*/
    opts.duration = 1;
    opts.cores = 0;
    opts.taskID = -1;
    opts.taskType = eBOTH;
    opts.inFileName[0] = '\0';
    opts.dispType = eREFRESH;
    while ((opt = getopt_long(argc, (char * const *)argv, "+hsc:t:f:i:d:", longOptions, &longIndex)) != -1) {
        switch (opt) {
            case 'c':
                opts.cores = maxCoresBitMask;
                temp = strtoll(optarg, &endp, 0);
                if (*endp == '\0') {
                    if (temp > 0 && temp <= opts.cores)
                        opts.cores = (uint32_t)temp;
                    else {
                        cout << "Maximum bitmask value allowed for coresFlag is " << maxCoresBitMask
                         << ". Please enter a valid positive number for coresFlag." << endl;
                        printUsage(argv[0]);
                        return -1;
                    }
                }
                else {
                    cout << "Please enter a valid number for cores." << endl;
                    printUsage(argv[0]);
                    return -1;
                }
                break;
            case 't':
                temp = strtoll(optarg, &endp, 0);
                if (*endp == '\0' && temp >= 0) {
                    opts.taskID = (int32_t)temp;
                }
                else {
                    cout << "Please enter a valid number for taskID." << endl;
                    printUsage(argv[0]);
                    return -1;
                }
                break;
            case 'f':
                if (!(strncmp(optarg, "RT", strlen(optarg))))
                    opts.taskType = eRT;
                else if (!(strncmp(optarg, "NRT", strlen(optarg))))
                    opts.taskType = eNRT;
                else {
                    cout << "Please enter RT or NRT to select taskType." << endl;
                    printUsage(argv[0]);
                    return -1;
                }
                break;
            case 'd':
                temp = strtoll(optarg, &endp, 0);
                if (*endp == '\0' && temp > 0)
                    opts.duration = (uint32_t)temp;
                else {
                    cout << "Please enter a valid number for duration." <<endl;
                    printUsage(argv[0]);
                    return -1;
                }
                break;
            case 'i':
                strncpy(opts.inFileName, optarg, sizeof(opts.inFileName) - 1);
                break;
            case 's':
                opts.dispType = eSCROLL;
                break;
            case 'h':
            default:
                printUsage(argv[0]);
                return -1;
                break;
        }
    }

    if ((opts.cores != 0) && (opts.taskID != -1)) {
        cout << "Please specify either --core or --taskID, not both." << endl;
        printUsage(argv[0]);
        return -1;
    }

    if ((opts.taskType != eBOTH) && (opts.taskID != -1)) {
        cout << "Please specify either --filtertask or --task, not both." << endl;
        printUsage(argv[0]);
        return -1;
    }

    if (!opts.cores && (opts.taskID == -1)) {
        /*By default dump tasks for all cores*/
        opts.cores = maxCoresBitMask;
    }

    if (optind < argc) {
        cout << "non-option ARGV-elements: ";
        while (optind < argc) {
            cout << argv[optind++] << endl;
        }
        return  -1;
    }
    return 0;
}

int ICmdLineInterface::cmdLineParse(int argc, const char *argv[])
{
    CmdLine *pInst = CmdLine::getInstance();
    return pInst->cmdLineParse(argc, argv);
}

CommonOptions& ICmdLineInterface::getOptions()
{
    CmdLine *pInst = CmdLine::getInstance();
    return pInst->opts;
}

void ICmdLineInterface::setMaxCoresBitMask(uint32_t bitMask)
{
    CmdLine *pInst = CmdLine::getInstance();
    return pInst->setMaxCoresBitMask(bitMask);
}

/*Export functions to common code*/

#ifdef __cplusplus
extern "C" {
#endif

int cmdLineParse(int argc, const char *argv[])
{
    return CmdLine::cmdLineParse(argc, argv);
}

void setMaxCoresBitMask(uint32_t bitMask)
{
    return CmdLine::setMaxCoresBitMask(bitMask);
}


#ifdef __cplusplus
}
#endif

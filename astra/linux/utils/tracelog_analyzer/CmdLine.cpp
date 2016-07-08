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
 * file : CmdLine.cpp
 *
 ***************************************************************************/
#include <iostream>
#include <cstring>
#include <fstream>
#include <getopt.h>

#include "CmdLine.h"

using namespace std;

struct NameValue {
    std::string name;
    int value;
};

class CmdLine {
private:
    /* private data */
    static bool instanceFlag;
    static CmdLine *s_instance;
    static struct NameValue nameToValue[];
    static struct option longOptions[];
    static int longIndex;

    /* private methods */
    static int getValue(const char *name);
    static void printUsage(const char *app);

    /* private constructor */
    CmdLine()
    {
        //private constructor
    }

public:
    /* public data */
    static struct CommonOptions opts;

    /* public methods */
    static int cmdLineProbe(void);
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
    {"infile", required_argument, 0, 'i'},
    {"outfile", required_argument, 0, 'o'},
    {0, 0, 0, 0},
};

int CmdLine::longIndex = 0;

// Static non-const data from CmdLine class
struct NameValue CmdLine::nameToValue[] = {
    {"2", 2},
    {"1", 1},
    {"0", 0},
    {"128", 128},
    {"16", 16},
};

struct CommonOptions CmdLine::opts;
bool CmdLine::instanceFlag = false;
CmdLine *CmdLine::s_instance = NULL;

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

int CmdLine::getValue(const char *str)
{
    int i = 0;
    string name = str;
    if (name.empty()) {
        cout << " Invalid name \n";
    }

    for(i = 0; i < (sizeof(nameToValue) / sizeof(nameToValue[0])); i++)
    {
        if (name == nameToValue[i].name) {
            return nameToValue[i].value;
        }
    }

    return -1;
}

void CmdLine::printUsage(const char *app)
{
    cout << "Usage: " << app << " [-option] [argument]" << endl;
    cout << "option:  " << "  -h  |  --help   show help information" << endl;
    cout << "         " << "  -i  |  --infile filename, to specify input file name" << endl;
    cout << "         " << "  -o  |  --outfile filename, to specify ouput file name" << endl;
    return;
}


int CmdLine::cmdLineParse(int argc, const char *argv[])
{
    int opt = 0;

    memset(&opts, 0, sizeof(opts));

    if (1 == argc) {
        printUsage(argv[0]);
        return -1;
    }

    while((opt = getopt_long(argc, (char * const *)argv, "+hi:o:", longOptions, &longIndex)) != -1)
    {
        switch(opt)
        {
            case 'i':
                strncpy(opts.inFileName, optarg, sizeof(opts.inFileName) - 1);
                break;
            case 'o':
                strncpy(opts.outFileName, optarg, sizeof(opts.outFileName) - 1);
                break;
            case 'h':
            default:
                printUsage(argv[0]);
            return -1;
                break;
        }
    }

    if (optind < argc) {
        cout << "non-option ARGV-elements: ";
        while (optind < argc)
        {
            cout << argv[optind++] << endl;
        }
        return  -1;
    }
    return 0;
}

int CmdLine::cmdLineProbe(void)
{
    int rc = 0;
    ifstream inFile(opts.inFileName);

    if (inFile.rdstate() & std::ifstream::failbit) {
        cout << " Error in opening input file " << opts.inFileName << endl ;
        rc = -1;
        goto done;
    }

    if (!inFile.is_open()) {
        cout << "can't open data file for probing " << opts.inFileName << endl;
        rc = -1;
        goto done;
    }

    inFile.close();
    return 0;
done:
    return rc;
}

int ICmdLineInterface::cmdLineProbe(void)
{
    CmdLine *pInst = CmdLine::getInstance();
    return pInst->cmdLineProbe();
}

int ICmdLineInterface::cmdLineParse(int argc, const char *argv[])
{
    CmdLine *pInst = CmdLine::getInstance();
    return pInst->cmdLineParse(argc, argv);
}

CommonOptions& ICmdLineInterface::getOpions()
{
    CmdLine *pInst = CmdLine::getInstance();
    return pInst->opts;
}

// Export functions to common code

#ifdef __cplusplus
extern "C" {
#endif

int cmdLineParse(int argc, const char *argv[])
{
    return CmdLine::cmdLineParse(argc, argv);
}

int cmdLineProbe(void)
{
    return CmdLine::cmdLineProbe();
}

#ifdef __cplusplus
}
#endif

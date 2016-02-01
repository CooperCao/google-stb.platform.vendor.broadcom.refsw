//****************************************************************************
//
// Copyright (c) 2007-2009 Broadcom Corporation
//
// This program is the proprietary software of Broadcom Corporation and/or
// its licensors, and may only be used, duplicated, modified or distributed
// pursuant to the terms and conditions of a separate, written license
// agreement executed between you and Broadcom (an "Authorized License").
// Except as set forth in an Authorized License, Broadcom grants no license
// (express or implied), right to use, or waiver of any kind with respect to
// the Software, and Broadcom expressly reserves all rights in and to the
// Software and all intellectual property rights therein.  IF YOU HAVE NO
// AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
// AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
// SOFTWARE.
//
// Except as expressly set forth in the Authorized License,
//
// 1.     This program, including its structure, sequence and organization,
// constitutes the valuable trade secrets of Broadcom, and you shall use all
// reasonable efforts to protect the confidentiality thereof, and to use this
// information only in connection with your use of Broadcom integrated circuit
// products.
//
// 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
// "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
// OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
// RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
// IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
// A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
// ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
// THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
// 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
// OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
// INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
// RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
// EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
// WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
// FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//****************************************************************************
//  $Id$
//  
//  Filename:       EstbTLVcodes.cpp
//  Author:         Maurice Turcotte
//  Creation Date:  8 NOV 2006
//
//****************************************************************************
//

#include "EstbTLVcodes.h"
#include "EstbEvents.h"

char* spec = (char *) "";
char* no_spec = (char *) " (NOT specified)";
char* reserved = (char *) "RESERVED";
char* undefined = (char *) "UNDEFINED";
char* kestbconfigfile = (char *) "kEstbConfigFile";

const char* UsageContextName( int context )
{
    // default to returning "RESERVED"
    char* retval = reserved;

    if (context == kEstbConfigFile)
    {
        // legal index value.
        retval = kestbconfigfile;
    }
    return retval;
}


const char* ConfigErrorCodeName( int conf_code )
{
    char* names[] = 
    {
        (char *) "kConfOK",                                  // 0
        (char *) "kConfHashFail",                            // 1
        (char *) "kConfTooBig",                              // 2
        (char *) "kConfNoEOF",                               // 3
        (char *) "kConfDupeOID",                             // 4
        (char *) "kConfBadValue",                            // 5
        (char *) "kConfBadOID",                              // 6
        (char *) "kConfBadType",                             // 7
        (char *) "kConfMissingParam"                         // 8
    };
    

    // default to returning "kConfOK"
    char* retval = (char *) "kConfOK";

    if( (conf_code >= kConfOK) && (conf_code <= kConfMissingParam) )
    {
        // legal index value.
        retval = names[ conf_code ];
    }
       
    return retval;

}

#ifdef NOTDEF
int EstbConfCodeToSnmpEventLogCode( int estb_conf_code )
{
    static int confCodeToEventLogIdCodeTable[][2] = 
    {
        {kEE_9700,  kConfHashFail},
        {kEE_9800,  kConfTooBig},
        {kEE_9900,  kConfNoEOF},
        {kEE_0000,  kConfDupeOID},
        {kEE_0100,  kConfBadValue},
        {kEE_0200,  kConfBadOID},
        {kEE_0300,  kConfBadType},
        { -1,     0  } // end of list marker
    };
    int snmp_log_code = 0; 
    for( int i = 0; confCodeToEventLogIdCodeTable[i][0] != -1; i++ )
    {
        if(confCodeToEventLogIdCodeTable[i][1] == estb_conf_code) 
        {
            snmp_log_code = confCodeToEventLogIdCodeTable[i][0];
            break;
        }
    }
    return snmp_log_code;
}
#endif


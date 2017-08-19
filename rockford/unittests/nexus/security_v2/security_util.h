/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef _SECURITY_TEST_FRAMEWORK_UTILITY__
#define _SECURITY_TEST_FRAMEWORK_UTILITY__

#include "nexus_dma.h"
#include "security_main.h"

#define VIDEO_PID (2316 )
#define XPT_TS_PACKET_SIZE (188)
#define XPT_TS_PACKET_HEAD_SIZE  (4)
#define IN_XPT_TYPE  NEXUS_TransportType_eTs
#define OUT_XPT_TYPE  NEXUS_TransportType_eTs

#define DEBUG_PRINT_ARRAY(description_txt,in_size,in_ptr) {             \
    int x_offset;                                                       \
    verboseLog("[%s][%d]", description_txt, in_size );                  \
    for( x_offset = 0; x_offset < (int)(in_size); x_offset++ )          \
    {                                                                   \
        if( x_offset%16 == 0 ) { verboseLog("\n"); }                    \
        verboseLog("%02X ", in_ptr[x_offset] );                         \
    }                                                                   \
    verboseLog("\n");                                                   \
}

#define PRINT_TS_PACKET(xptTSPackets) {                                 \
    int             ix;                                                 \
    verboseLog("\n---------------------------\n");                      \
    for ( ix = 0; ix < XPT_TS_PACKET_SIZE; ix++ ) {                     \
        if ( ix % 4 == 0 )                                              \
            verboseLog( " " );                                          \
        verboseLog ( "%02X", xptTSPackets[ix] );                        \
        if ( ix % 16 == 15 )                                            \
            verboseLog ( "\n" );                                        \
    }                                                                   \
    verboseLog( "\n" );                                                 \
}


typedef struct
{
    bool verbose;
    bool showHelp;
    bool showMenu;
    int groupNum;
    int testNum;
}securityUtil_options;

/* module data. */
typedef struct groupTestItem
{
    BLST_S_ENTRY(groupTestItem) next;
    securityTestConfig args;
    securityTestFn registeredTest;   /* the test function */
    bool ran;                        /* true if test has been ran. */
    bool passed;                     /* true is test has passed.   */
}groupTestItem;

typedef struct testGroup
{
    BLST_S_HEAD(groupTestItem_LIST, groupTestItem) testList;
    char groupName[64];
    bool registered;
}testGroup;

struct
{
     testGroup groups[securityTestGroup_max];
}gData;

securityUtil_options securityUtil_GetOptions( void );
securityUtil_options securityUtil_ParseOptions( int argc, char **argv );

/* print log to console (if app was called with -v option set. ) */
void verboseLog( char* pFormat, ...  );

int securityUtil_PlatformInit( bool useDisplay );
int securityUtil_PlatformUnInit( void );

/* Handle security DMA transfers */
int securityUtil_DmaTransfer( NEXUS_KeySlotHandle keyslotHandle,
                              uint8_t * pSrc,
                              uint8_t * pDest,
                              NEXUS_DmaDataFormat dataFormat,
                              size_t dataSize,
                              bool securityBpt );

/*
   Composit a single TS packet or multiple packets for tests.
   XPT_TS_PACKET_NUM defines the number of the packets.
*/
void  CompositTSPackets( uint8_t * xptTSPackets, unsigned int scValue );


#endif

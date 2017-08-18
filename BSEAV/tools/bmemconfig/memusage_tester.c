/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "memusage.h"
#include "bmemperf_utils.h"
#include "bmemperf_info.h"

/* these global variables are solely needed to compile with bmemperf_utils.c */
char         *g_client_name[BMEMPERF_MAX_NUM_CLIENT];
int           g_MegaBytes           = 0; /* set to 1 when user wants data displayed in megabytes instead of megabits (default) */
int           g_MegaBytesDivisor[2] = {1, 8};
char         *g_MegaBytesStr[2] = {"Mbps", "MBps"};
char         *g_client_name[BMEMPERF_MAX_NUM_CLIENT];
bmemperf_info g_bmemperf_info;
bsysperf_netStatistics g_netStats[NET_STATS_MAX];
int                    g_netStatsIdx = -1;                 /* index to entries added to g_netStats array */

static void guiInput(
    Memconfig_AppUsageSettings *pInput
    )
{
    /* get user inputs or box mode and set the appropriate configuration.   */
    pInput->bSecure= true;

    pInput->record.number = 4;

    pInput->playback.number = 4;

    pInput->message.number = 2;
    pInput->message.size   = 4*1024; /* message BufSize */

    pInput->live.number = 8;

    pInput->remux.number = 2;

    pInput->videoDecoder[0].enabled    = true;
    pInput->videoDecoder[0].bIp        = false;
    pInput->videoDecoder[0].b3840x2160 = false;
    pInput->videoDecoder[0].bSecure    = false;
    pInput->videoDecoder[0].bSoft      = false;
    pInput->videoDecoder[0].bMvc       = false;
    pInput->videoDecoder[0].numMosaic  = 0;
    pInput->videoDecoder[0].numFcc     = 0;

    pInput->audioDecoder[0].enabled    = true;
    pInput->audioDecoder[0].bPassthru  = true;
    pInput->audioDecoder[0].bSecondary = false;
    pInput->audioDecoder[0].bIp        = false;
    pInput->audioDecoder[0].bSecure    = false;
    pInput->audioDecoder[0].numFcc     = 0;

#if NEXUS_NUM_VIDEO_ENCODERS
    pInput->encoders[0].enabled   = true;
    pInput->encoders[0].streamMux = true;
#endif /* NEXUS_NUM_VIDEO_ENCODERS */
} /* guiInput */

#if 0
static void printTransportSettings(
    Memconfig_AppUsageSettings *pInput
    )
{
    printf( "record.number %d\n", pInput->record.number );

    printf( "playback.number %d\n", pInput->playback.number );

    printf( "message.number %d\n", pInput->message.number );
    printf( "message.size %d \n", pInput->message.size = 4*1024 );

    printf( "live.number %d\n", pInput->live.number );

    printf( "remux.number %d\n", pInput->remux.number );

    printf( "videoDecoder[0].enabled %d\n", pInput->videoDecoder[0].enabled );
    printf( "videoDecoder[0].bIp %d\n",   pInput->videoDecoder[0].bIp );
    printf( "videoDecoder[0].b3840x2160 %d\n",     pInput->videoDecoder[0].b3840x2160 );
    printf( "videoDecoder[0].bSecure %d\n",      pInput->videoDecoder[0].bSecure );
    printf( "videoDecoder[0].bSoft %d\n",       pInput->videoDecoder[0].bSoft );
    printf( "videoDecoder[0].bMvc %d\n",       pInput->videoDecoder[0].bMvc );
    printf( "videoDecoder[0].numMosaic %d\n",       pInput->videoDecoder[0].numMosaic );
    printf( "videoDecoder[0].numFcc %d\n",      pInput->videoDecoder[0].numFcc );

    printf( "audioDecoder[0].enabled %d\n",  pInput->audioDecoder[0].enabled );
    printf( "audioDecoder[0].bPassthru %d\n",     pInput->audioDecoder[0].bPassthru );
    printf( "audioDecoder[0].bSecondary %d\n",    pInput->audioDecoder[0].bSecondary );
    printf( "audioDecoder[0].bIp %d\n",    pInput->audioDecoder[0].bIp );
    printf( "audioDecoder[0].bSecure %d\n",     pInput->audioDecoder[0].bSecure );
    printf( "audioDecoder[0].numFcc %d\n",    pInput->audioDecoder[0].numFcc );

    printf( "encoders[0].enabled %d\n",  pInput->encoders[0].enabled );
    printf( "encoders[0].streamMux %d\n", pInput->encoders[0].streamMux );
} /* printTransportSettings */
#endif
int main(void)
{
     while (1) {
        unsigned config;
        char buf[64];
        Memconfig_AppUsageSettings pInput;
        Memconfig_AppMemUsage pOutput;


        BKNI_Memset( &pInput, 0, sizeof( pInput ));
        BKNI_Memset( &pOutput, 0, sizeof( pOutput ));


        printf(
            "Select config:\n"
            "1) defaultSetings \n"
            "2) guiInput\n"
            );
        fgets(buf, sizeof(buf), stdin);
        config = atoi(buf);
        if (config == 0 || config > 2) {
            printf("Invalid selection\n");
            continue;
        }

        Memconfig_AppUsageGetDefaultSettings(&pInput);


        switch (config) {
        case 1: break;
        case 2: guiInput(&pInput); break;
        }

    /* calculate */
    Memconfig_AppUsageCalculate(&pInput,&pOutput);


    printf(" Main heap totals: %d\n",pOutput.mainHeapTotal);
    printf(" Secure heap totals: %d\n",pOutput.secureHeapTotal);

    }

    return 0;
}

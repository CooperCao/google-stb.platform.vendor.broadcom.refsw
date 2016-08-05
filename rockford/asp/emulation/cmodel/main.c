/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include "asp_proxy_server_api.h"
#include "nexus_aspsim_api.h"
#include "asp_nw_sw_api.h"
#include "asp_xpt_api.h"

#define ASP_SIMULATOR   1

#ifdef  ASP_SIMULATOR
#include "asp_simulator.h"
#endif

static int gGotSigInt = 0;

void
signalHandler(int signal)
{
    fprintf(stdout,"\nGot SIGINT (%d): Cleaning up!!!\n", signal);
    gGotSigInt = 1;
}

int main()
{
    ASP_ProxyServerHandle hProxyServer;
    ASP_ProxyServerCreateSettings settings;
    bool quit = false;
    int rc;

    ASP_ProxyServer_GetDefaultCreateSettings(&settings);
    hProxyServer = ASP_ProxyServer_Create(&settings);
    assert(hProxyServer);

    /* Initialize modules.*/
    NEXUS_AspSim_Init();
    ASP_NwSw_Init();
    ASP_Xpt_Init();

    signal(SIGINT, signalHandler);
    /* ignore SIGPIPE otherwise abnormal termination on client can cause server to crash */
    signal(SIGPIPE, SIG_IGN);

    /* TODO: Call Create/Init of ASP_CModel_Create(). */
    while (!quit)
    {
        rc = ASP_ProxyServer_ProcessIo(hProxyServer);
        if (rc != SUCCESS) break;


        rc = ASP_Nexus_ProcessIo(hProxyServer);
        if (rc != SUCCESS)
        {
            fprintf(stdout,"\n%s: ASP_Nexus_ProcessIo Failed ========================\n", __FUNCTION__);
            break;
        }


#ifdef  ASP_SIMULATOR
        rc = Asp_Simulator_ProcessIo(hProxyServer);
        if (rc != SUCCESS)
        {
            fprintf(stdout,"\n%s: Asp_Simulator_ProcessIo Failed ========================\n", __FUNCTION__);
            break;
        }

#endif
        /* TODO: rc = ASP_CModel_ProcessNetworkIo(hCModel); to process raw frames! */
        sleep(1);
        if (gGotSigInt) {
            fprintf(stdout,"%s:Exiting from main loop\n",__FUNCTION__);
            break;
        }
    }

    /* TODO: ASP_CModel_Destory(hCModel); */
    ASP_ProxyServer_Destory(hProxyServer);


    /* UnInitiazed all modules.*/
    NEXUS_AspSim_UnInit();
    ASP_NwSw_UnInit();
    ASP_Xpt_UnInit();

    return 0;
}

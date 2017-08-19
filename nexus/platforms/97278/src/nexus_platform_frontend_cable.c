/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 **************************************************************************/

#include "nexus_platform_module.h"
#include "priv/nexus_core.h"
#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#endif
#include "nexus_platform_features.h"
#include "nexus_platform_priv.h"
#include "nexus_base.h"
#include "nexus_input_band.h"
#include "bchp_gio.h"

#if defined(NEXUS_FRONTEND_DOCSIS)
#include "nexus_docsis.h"
#endif

#define BCM3390_CONTROLLED_MOST_DS_CHANNEL_NUMBER   34

#define BCM3384_CONTROLLED_MOST_DS_CHANNEL_NUMBER   16
#define BCM3128_CONTROLLED_MOST_DS_CHANNEL_NUMBER   8
#define BCM3384_CONTROLLED_LEAST_DATA_CHANNEL_NUMBER 4

BDBG_MODULE(nexus_platform_frontend_cable);

#if NEXUS_HAS_FRONTEND
static NEXUS_GpioHandle gpioHandle = NULL;
#if defined(NEXUS_FRONTEND_DOCSIS)
static NEXUS_FrontendDeviceHandle hDocsisDevice;
#endif

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    int i = 0;

#if defined(NEXUS_FRONTEND_DOCSIS)
    unsigned docsisChannel;
    NEXUS_DocsisOpenDeviceSettings docsisDeviceSettings;
    NEXUS_DocsisOpenChannelSettings docsisChannelSettings;
    NEXUS_DocsisDeviceCapabilities docsisDeviceCaps;
#endif


    NEXUS_FrontendUserParameters userParams;

    BDBG_MSG(("NEXUS_Platform_InitFrontend: NEXUS_MAX_FRONTENDS %u", NEXUS_MAX_FRONTENDS));

#if defined(NEXUS_FRONTEND_DOCSIS)
    NEXUS_Docsis_GetDefaultOpenDeviceSettings(&docsisDeviceSettings);

    docsisDeviceSettings.rpcTimeOut = 3000; /* units ms */
    hDocsisDevice = NEXUS_Docsis_OpenDevice(0,&docsisDeviceSettings);

    NEXUS_Docsis_GetDeviceCapabilities(hDocsisDevice,&docsisDeviceCaps);

    BDBG_MSG(("CABLE frontend DOCSIS Capabilities : Total Channels %u QAM Channels %u Docsis Channels %u OOB %s",
              docsisDeviceCaps.totalChannels,docsisDeviceCaps.numQamChannels,
              docsisDeviceCaps.numDataChannels,docsisDeviceCaps.numOutOfBandChannels?"true":"false" ));

    for (i=0, docsisChannel=0;
         (docsisChannel<(docsisDeviceCaps.numDataChannels + docsisDeviceCaps.numQamChannels)) && (i<NEXUS_MAX_FRONTENDS-1);
         docsisChannel++)
    {
        NEXUS_Docsis_GetDefaultOpenChannelSettings(&docsisChannelSettings);
        if(docsisChannel >= docsisDeviceCaps.numDataChannels)
        {
            docsisChannelSettings.autoAcquire = true;
            docsisChannelSettings.channelNum = docsisChannel;
            docsisChannelSettings.channelType = NEXUS_DocsisChannelType_eQam;
            docsisChannelSettings.fastAcquire = true;
            BDBG_MSG(("Docsis frontend index %u Docsis QAM channel %u", i, docsisChannel));
        }
        else
        {
            BDBG_MSG(("Docsis data channel %u",docsisChannel));
            continue;
        }

        pConfig->frontend[i] = NEXUS_Docsis_OpenChannel(hDocsisDevice,&docsisChannelSettings);
        if (!pConfig->frontend[i])
        {
            BDBG_ERR(("Unable to open docsis channel frontendIndex %u channel %u",i,docsisChannel));
            continue;
        }
        NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
        userParams.isMtsif = true; /*docsisDeviceCaps.isMtsif; always true for 3384*/
    #if NEXUS_USE_3390_VMS
        userParams.chipId = 0x3390;
    #else
        userParams.chipId = 0x3384;
    #endif

    #if NEXUS_USE_3390_VMS
        if((docsisDeviceCaps.numDataChannels + docsisDeviceCaps.numQamChannels) > BCM3390_CONTROLLED_MOST_DS_CHANNEL_NUMBER )
		{
            BDBG_MSG(("Total DS channel is %u, we do have 312x besides eCM",(docsisDeviceCaps.numDataChannels + docsisDeviceCaps.numQamChannels)));
            if((int)(docsisDeviceCaps.numQamChannels - BCM3128_CONTROLLED_MOST_DS_CHANNEL_NUMBER) > i) /*to determine if we are using 3384 inputbands*/
            {
                userParams.param1 = docsisChannel; /*input band index from 3384 start from the smallest available number*/
                BDBG_MSG(("eCM frontend input number %d",userParams.param1));
            }
            else {
                userParams.pParam2 = 1; /* this is used to indicate that this frontend's IB comes from 3128 */
                BDBG_ASSERT((i -(int)(docsisDeviceCaps.numQamChannels - BCM3128_CONTROLLED_MOST_DS_CHANNEL_NUMBER)) >= 0);
                userParams.param1 = i -(int)(docsisDeviceCaps.numQamChannels - BCM3128_CONTROLLED_MOST_DS_CHANNEL_NUMBER); /*input band index from 312x start from the smallest available number*/
                BDBG_MSG(("eCM controlled 312x frontend input number %d",userParams.param1));
            }
        }
        else{
            BDBG_MSG(("Total DS channel is %u, we do NOT have 312x besides eCM",(docsisDeviceCaps.numDataChannels + docsisDeviceCaps.numQamChannels)));
            BDBG_ASSERT((int)(BCM3390_CONTROLLED_MOST_DS_CHANNEL_NUMBER - i -1) >= 0);
            userParams.param1 = docsisChannel - 16 ; /*input band index from 3390 start from 16, first 16 channels are for DOCSIS data*/
            BDBG_MSG(("eCM frontend input number %d",userParams.param1));
        }
        NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
        i++;
    }
    #else
        if((docsisDeviceCaps.numDataChannels + docsisDeviceCaps.numQamChannels) > BCM3384_CONTROLLED_MOST_DS_CHANNEL_NUMBER ){
            BDBG_MSG(("Total DS channel is %u, we do have 312x besides eCM",(docsisDeviceCaps.numDataChannels + docsisDeviceCaps.numQamChannels)));
            /*if we do need to use eCM demod, we always use it first*/
            if((int)(docsisDeviceCaps.numQamChannels - BCM3128_CONTROLLED_MOST_DS_CHANNEL_NUMBER) > i) /*to determine if we are using 3384 inputbands*/
            {
                userParams.param1 = docsisChannel; /*input band index from 3384 start from the smallest available number*/
                BDBG_MSG(("eCM frontend input number %d",userParams.param1));
                /*docsisDeviceCaps.isMtsif ? docsisChannel : NEXUS_InputBand_e0+docsisChannel;*/
            }
            else {
                userParams.pParam2 = 1; /* this is used to indicate that this frontend's IB comes from 3128 */
                BDBG_ASSERT((i -(int)(docsisDeviceCaps.numQamChannels - BCM3128_CONTROLLED_MOST_DS_CHANNEL_NUMBER)) >= 0);
                userParams.param1 = i -(int)(docsisDeviceCaps.numQamChannels - BCM3128_CONTROLLED_MOST_DS_CHANNEL_NUMBER); /*input band index from 312x start from the smallest available number*/
                BDBG_MSG(("eCM controlled 312x frontend input number %d",userParams.param1));
            }
        }
        else{
            BDBG_MSG(("Total DS channel is %u, we do NOT have 312x besides eCM",(docsisDeviceCaps.numDataChannels + docsisDeviceCaps.numQamChannels)));
            BDBG_ASSERT((int)(BCM3384_CONTROLLED_MOST_DS_CHANNEL_NUMBER - i -1) >= 0);
            userParams.param1 = docsisChannel; /*input band index from 3384 start from the smallest available number*/
            BDBG_MSG(("eCM frontend input number %d",userParams.param1));
        }

        NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
        i++;
    }
    #endif
#endif

    if(i){
        BDBG_MSG(("total %d inband channel initialized", i));
    }
    else
        BDBG_WRN(("no inband channel initialized"));

    /*
     * If OOB channel is present in the Docsis device, check for the channel number
     */
#if defined(NEXUS_FRONTEND_DOCSIS)
    if(docsisDeviceCaps.numOutOfBandChannels)
    {
        NEXUS_Docsis_GetDefaultOpenChannelSettings(&docsisChannelSettings);
        docsisChannelSettings.channelType=NEXUS_DocsisChannelType_eOutOfBand;
        docsisChannelSettings.channelNum = docsisDeviceCaps.numDataChannels + docsisDeviceCaps.numQamChannels;
        pConfig->frontend[i] = NEXUS_Docsis_OpenChannel(hDocsisDevice,&docsisChannelSettings);
        if(!pConfig->frontend[i])
        {
            BDBG_ERR(("DOCSIS OOB channel open failed"));
        }
        NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
        userParams.param1 = docsisChannelSettings.channelNum; /*docsisDeviceCaps.isMtsif ? docsisChannelSettings.channelNum : NEXUS_InputBand_e0+i;*/
        userParams.isMtsif = docsisDeviceCaps.isMtsif;
    #if NEXUS_USE_3390_VMS
        userParams.chipId = 0x3390;
    #else
        userParams.chipId = 0x3384;
    #endif
        NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);

        i++;
    }
    if(docsisDeviceCaps.numUpStreamChannels)
    {
        NEXUS_Docsis_GetDefaultOpenChannelSettings(&docsisChannelSettings);
        docsisChannelSettings.channelType = NEXUS_DocsisChannelType_eUpstream;
        docsisChannelSettings.channelNum = docsisDeviceCaps.numDataChannels + docsisDeviceCaps.numQamChannels +
                                           docsisDeviceCaps.numOutOfBandChannels;
        pConfig->frontend[i] = NEXUS_Docsis_OpenChannel(hDocsisDevice,&docsisChannelSettings);
        if(!pConfig->frontend[i])
        {
            BDBG_ERR(("DOCSIS OOB channel open failed"));
        }
        NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
        userParams.param1 = docsisChannelSettings.channelNum; /*docsisDeviceCaps.isMtsif ? docsisChannelSettings.channelNum : NEXUS_InputBand_e0+i;*/
        userParams.isMtsif = docsisDeviceCaps.isMtsif;
    #if NEXUS_USE_3390_VMS
        userParams.chipId = 0x3390;
    #else
        userParams.chipId = 0x3384;
    #endif
        NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);

    }
#endif

    return BERR_SUCCESS;
}

void NEXUS_Platform_UninitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    unsigned i=0;

    unsigned j=0;
    NEXUS_FrontendDeviceHandle tempHandle, deviceHandles[NEXUS_MAX_FRONTENDS];
    bool handleFound = false;
    BKNI_Memset(deviceHandles, 0, sizeof(deviceHandles));
    for (i=0; i<NEXUS_MAX_FRONTENDS; i++)
    {
        handleFound = false;
        if (pConfig->frontend[i])
        {
            tempHandle = NEXUS_Frontend_GetDevice(pConfig->frontend[i]);
            if(tempHandle != NULL){
                for( j = 0; j<i; j++){
                    if(tempHandle == deviceHandles[j])
                    handleFound = true;
                }
                if(!handleFound)
                    deviceHandles[j] = tempHandle;
            }
            BDBG_MSG(("NEXUS_Platform_UninitFrontend frontend %u",i));
            NEXUS_Frontend_Close(pConfig->frontend[i]);
            pConfig->frontend[i] = NULL;
        }
    }

    for (i=0; i<NEXUS_MAX_FRONTENDS; i++)
    {
        if (deviceHandles[i])
        {
            NEXUS_FrontendDevice_Close(deviceHandles[i]);
            deviceHandles[i] = NULL;
        }
    }

    if(gpioHandle)
    {
        NEXUS_Gpio_Close(gpioHandle);
        gpioHandle = NULL;
    }

    return;
}
#else
NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    return BERR_SUCCESS;
}

void NEXUS_Platform_UninitFrontend(void)
{
    return;
}
#endif

BTRC_MODULE(ChnChange_TuneStreamer, ENABLE);

NEXUS_Error NEXUS_Platform_GetStreamerInputBand(unsigned index, NEXUS_InputBand *pInputBand)
{
    BDBG_ASSERT(pInputBand);
    if (index > 0) {
        BDBG_ERR(("Only 1 streamer input available"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BTRC_TRACE(ChnChange_TuneStreamer, START);
    *pInputBand = NEXUS_InputBand_e0;
    BTRC_TRACE(ChnChange_TuneStreamer, STOP);
    return NEXUS_SUCCESS;
}

NEXUS_FrontendHandle NEXUS_Platform_OpenFrontend(
unsigned id /* platform assigned ID for this frontend. See NEXUS_FrontendUserParameters.id.
See nexus_platform_frontend.c for ID assignment and/or see
nexus_platform_features.h for possible platform-specific macros.
*/
)
{
    NEXUS_Error errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
    BSTD_UNUSED(errCode);
    BSTD_UNUSED(id);
    return NULL;
}

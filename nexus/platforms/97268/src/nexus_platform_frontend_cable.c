/******************************************************************************
 *  Copyright (C) 2018 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#if NEXUS_HAS_FRONTEND
#include "nexus_frontend_3128.h"
#endif

#if NEXUS_USE_7252S_VMS_SFF || NEXUS_USE_7268_VMS
#include "bchp_sun_top_ctrl.h"

static unsigned ltsidCount[NEXUS_MAX_FRONTENDS];
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
    NEXUS_GpioSettings tunerGpioSettings;
    int i = 0;

#if defined(NEXUS_FRONTEND_DOCSIS)
    unsigned docsisChannel;
    NEXUS_DocsisOpenDeviceSettings docsisDeviceSettings;
    NEXUS_DocsisOpenChannelSettings docsisChannelSettings;
    NEXUS_DocsisDeviceCapabilities docsisDeviceCaps;
#endif
#if NEXUS_USE_7252S_VMS_SFF
    BREG_Handle hReg;
#endif

#if NEXUS_USE_7252S_VMS_SFF || NEXUS_USE_7268_VMS
    NEXUS_FrontendLTSIDParameters ltsidParams;
#endif

    NEXUS_Error rc;

    unsigned u3128ch;
    NEXUS_3128ProbeResults results;
    NEXUS_Frontend3128Settings st3128Settings;
    NEXUS_FrontendDevice3128OpenSettings st3128DeviceOpenSettings;
#ifdef BCM3128_OOB_CABLECARD_SUPPORT
    NEXUS_FrontendDevice3128Settings st3128DeviceSettings;
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

#if defined(NEXUS_FRONTEND_DOCSIS)
	#if NEXUS_USE_3390_VMS
	    if((docsisDeviceCaps.numDataChannels + docsisDeviceCaps.numQamChannels) > BCM3390_CONTROLLED_MOST_DS_CHANNEL_NUMBER){
	    }
	#endif
    if((docsisDeviceCaps.numDataChannels + docsisDeviceCaps.numQamChannels) > BCM3384_CONTROLLED_MOST_DS_CHANNEL_NUMBER){
    /*3384 control 3128, host doesn't care 3128 init and how to use it*/
    }
    else
#endif
    {
        /* GPIO is used instead of EXT_IRQ. */
        NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &tunerGpioSettings);

        tunerGpioSettings.mode = NEXUS_GpioMode_eInput;
        tunerGpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;

#if defined NEXUS_USE_7449_SV || defined NEXUS_USE_7439_SV
        gpioHandle = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 60, &tunerGpioSettings);
#elif NEXUS_USE_7252S_VMS_SFF
        gpioHandle = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 103, &tunerGpioSettings);
#elif NEXUS_NUM_FRONTEND_CARD_SLOTS
        gpioHandle = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 82, &tunerGpioSettings);
#else
        gpioHandle = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 28, &tunerGpioSettings);
#endif
        if (NULL == gpioHandle)
        {
          BDBG_ERR(("Unable to open GPIO for tuner %d", i));
          return BERR_NOT_INITIALIZED;
        }

        NEXUS_Frontend_GetDefault3128Settings(&st3128Settings);

        NEXUS_FrontendDevice_GetDefault3128OpenSettings(&st3128DeviceOpenSettings);
        st3128DeviceOpenSettings.i2cDevice = pConfig->i2c[NEXUS_I2C_CHANNEL_DSTRM_TUNER];
#if NEXUS_USE_7252S_VMS_SFF
        st3128DeviceOpenSettings.i2cAddr = 0x6d;
#else
        st3128DeviceOpenSettings.i2cAddr = 0x6c;
#endif
        st3128DeviceOpenSettings.isrNumber = 0;
        st3128DeviceOpenSettings.gpioInterrupt = gpioHandle;
        st3128DeviceOpenSettings.inBandOpenDrain= true;
        st3128DeviceOpenSettings.loadAP = true;
        st3128DeviceOpenSettings.isMtsif = true;

#if NEXUS_USE_7252S_VMS_SFF
    hReg = g_pCoreHandles->reg;

    /* 3128 MTSIF INC/RST on pin 30/31  */
    st3128DeviceOpenSettings.pinmux.data[0] = BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11;
    st3128DeviceOpenSettings.pinmux.data[1] = (BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11) | 0x10000);

    st3128DeviceOpenSettings.pinmux.data[2] = BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11;
    st3128DeviceOpenSettings.pinmux.data[3] = (BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11) | 0x100000);
#endif

    rc = NEXUS_Frontend_Probe3128(&st3128DeviceOpenSettings, &results);
    if(rc) return BERR_TRACE(BERR_NOT_INITIALIZED);

        st3128Settings.device = NEXUS_FrontendDevice_Open3128(0, &st3128DeviceOpenSettings);
        if (st3128Settings.device == NULL) {
            BDBG_WRN(("Unable to open 3128 device, could be no 3128 exists or init failed"));
        } else {
            /*
            * Open the BCM3128 InBand channels
            */
            st3128Settings.loadAP = true;
            st3128Settings.type = NEXUS_3128ChannelType_eInBand;
            st3128Settings.isMtsif = true;
            for(u3128ch=0; ((i<NEXUS_MAX_FRONTENDS-1) && (u3128ch<(results.chip.id & 0xF))); u3128ch++)
            {
                BDBG_MSG((" frontend index %u BCM3128 QAM channel %u", i, u3128ch));
                st3128Settings.channelNumber = u3128ch;
                pConfig->frontend[i] = NEXUS_Frontend_Open3128(&st3128Settings);
                if (NULL == pConfig->frontend[i])
                {
                    BDBG_ERR(("Unable to open onboard 3128 tuner/demodulator channel %d", i));
                    continue;
                }
                NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
                userParams.param1 = st3128DeviceOpenSettings.isMtsif ? st3128Settings.channelNumber : NEXUS_InputBand_e0+u3128ch;
                userParams.isMtsif = st3128DeviceOpenSettings.isMtsif;
                userParams.chipId = 0x3128;
                NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);

#if NEXUS_USE_7252S_VMS_SFF
        NEXUS_Frontend_GetLTSID(pConfig->frontend[i], &ltsidParams);
        ltsidParams.ltsidNum = userParams.param1;
        ltsidParams.chipId = 0x3128;
        ltsidParams.mtsifNum = 0;
        ltsidParams.mtsifEnabled = true;
        NEXUS_Frontend_SetLTSID(pConfig->frontend[i], &ltsidParams);
        ltsidCount[ltsidParams.ltsidNum] += 1;
#endif
                i++;
            }
        }
    }

    if(i){
        BDBG_MSG(("total %d inband channel initialized", i));
    }
    else
        BDBG_WRN(("no inband channel initialized"));

#ifdef BCM3128_OOB_CABLECARD_SUPPORT
    /*Open the BCM3128 OOB channel */
    st3128Settings.type = NEXUS_3128ChannelType_eOutOfBand;
    BDBG_WRN(("Waiting for onboard 3128 Oob channel %d to initialize", u3128ch));
    st3128Settings.channelNumber = u3128ch;
    pConfig->frontend[i] = NEXUS_Frontend_Open3128(&st3128Settings);
    if (NULL == pConfig->frontend[i])
    {
        BDBG_ERR(("Unable to open onboard 3128 Oob channel %d", i));
    }

    NEXUS_Frontend_GetDefault3128ConfigSettings(&st3128DeviceSettings);
    st3128DeviceSettings.outOfBand.outputMode = NEXUS_FrontendOutOfBandOutputMode_eDifferentialDecoder;
    if(NEXUS_Frontend_3128_SetConfigSettings(pConfig->frontend[i], &st3128DeviceSettings))
        return BERR_TRACE(BERR_NOT_INITIALIZED);
    NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
    userParams.param1 = st3128DeviceOpenSettings.isMtsif ? st3128Settings.channelNumber : NEXUS_InputBand_e0+i;
    userParams.isMtsif = st3128DeviceOpenSettings.isMtsif;
    userParams.chipId = 0x3128;
    NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
#else
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

#if NEXUS_USE_7252S_VMS_SFF || NEXUS_USE_7268_VMS
        NEXUS_Frontend_GetLTSID(pConfig->frontend[i], &ltsidParams);
        ltsidParams.ltsidNum = userParams.param1;
        ltsidParams.chipId = 0x3384;
        NEXUS_Frontend_SetLTSID(pConfig->frontend[i], &ltsidParams);
        ltsidCount[ltsidParams.ltsidNum] += 1;
#endif
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

#if NEXUS_USE_7252S_VMS_SFF || NEXUS_USE_7268_VMS
        NEXUS_Frontend_GetLTSID(pConfig->frontend[i], &ltsidParams);
        ltsidParams.ltsidNum = userParams.param1;
        ltsidParams.chipId = 0x3384;
        NEXUS_Frontend_SetLTSID(pConfig->frontend[i], &ltsidParams);
        ltsidCount[ltsidParams.ltsidNum] += 1;
#endif
    }
#endif

#endif

    #if 0
    /*
     * On platforms with DOCSIS and BCM3128 enabled, LNA device is shared between BCM3128 and DOCSIS,
     * but LNA device is controlled by DOCSIS. This linking would be used for extracting the
     * AGC val from DOCSIS device by the BCM3128 private APIs to program the AGC value into
     * BCM3128 device.
     */
    NEXUS_FrontendDevice_Link(hDocsisDevice,st3128Settings.device, NULL);
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

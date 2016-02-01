 /***************************************************************************
*     (c)2004-2014 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
*
***************************************************************************/
#include "nexus_types.h"
#include "nexus_platform.h"
#include "priv/nexus_core.h"
#include "nexus_platform_features.h"
#include "nexus_platform_priv.h"
#include "nexus_base.h"
#include "nexus_input_band.h"
#include "bchp_gio.h"
#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#endif
BDBG_MODULE(nexus_platform_frontend);

#if (NEXUS_PLATFORM == 97429) || (NEXUS_PLATFORM == 974295)

#if NEXUS_FRONTEND_DOCSIS
#include "nexus_docsis.h"
static NEXUS_FrontendDeviceHandle hDocsisDevice;

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    unsigned i = 0;
    unsigned docsisChannel;
    NEXUS_DocsisOpenDeviceSettings docsisDeviceSettings;
    NEXUS_DocsisOpenChannelSettings docsisChannelSettings;
    NEXUS_DocsisDeviceCapabilities docsisDeviceCaps;
    NEXUS_FrontendUserParameters userParams;

    NEXUS_Docsis_GetDefaultOpenDeviceSettings(&docsisDeviceSettings);
    docsisDeviceSettings.rpcTimeOut = 50; /* units ms */
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
        userParams.param1 = docsisDeviceCaps.isMtsif ? docsisChannel : NEXUS_InputBand_e0+docsisChannel;
        userParams.isMtsif = docsisDeviceCaps.isMtsif;
        userParams.chipId = 0x3384; 
        NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
        i++;
    }

    /*
     * If OOB channel is present in the Docsis device, check for the channel number
     */
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
        userParams.param1 = docsisDeviceCaps.isMtsif ? docsisChannelSettings.channelNum : NEXUS_InputBand_e0+i;
        userParams.isMtsif = docsisDeviceCaps.isMtsif;
        userParams.chipId = 0x3384;
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
        userParams.param1 = docsisDeviceCaps.isMtsif ? docsisChannelSettings.channelNum : NEXUS_InputBand_e0+i;
        userParams.isMtsif = docsisDeviceCaps.isMtsif;
        userParams.chipId = 0x3384;
        NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
    }

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

    return;
}
#elif NEXUS_FRONTEND_3383

#include "nexus_frontend_3255.h"
static NEXUS_3255DeviceHandle st3255DeviceHandle;

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    unsigned i=0;

    /*defined(NEXUS_PLATFORM_DOCSIS_IB_SUPPORT)*/
    unsigned st3255Channel;
    NEXUS_3255DeviceSettings st3255DeviceSettings;
    NEXUS_3255ChannelSettings st3255ChannelSettings;
    NEXUS_3255DeviceCapabilities st3255DeviceCapabilities;
    NEXUS_3255ChannelCapabilities st3255ChannelCapabilities;

    NEXUS_FrontendUserParameters userParams;

    NEXUS_Frontend_GetDefault3255DeviceSettings(&st3255DeviceSettings);
    st3255DeviceSettings.rpcTimeout = 50;
    st3255DeviceSettings.mtsif = true;
    st3255DeviceHandle = NEXUS_Frontend_Open3255Device(0,&st3255DeviceSettings);
    NEXUS_Frontend_Get3255DeviceCapabilities(st3255DeviceHandle,&st3255DeviceCapabilities);
    BDBG_ERR(("DOCSIS Capabilities : Total Channels %u QAM Channels %u Docsis Channels %u oob %s",
              st3255DeviceCapabilities.totalChannels,st3255DeviceCapabilities.numOfQamChannels,
              st3255DeviceCapabilities.numOfDocsisChannels,st3255DeviceCapabilities.isOobChannelSupported?"true":"false" ));
    /*
     * Open the DOCSIS Inband channels
     */
    for (i=0,st3255Channel=0;
         (st3255Channel < st3255DeviceCapabilities.totalChannels) && (i < NEXUS_MAX_FRONTENDS-1);
          st3255Channel++)
    {
        NEXUS_Frontend_Get3255ChannelCapabilities(st3255DeviceHandle,st3255Channel,&st3255ChannelCapabilities);
        if(st3255ChannelCapabilities.channelType == NEXUS_3255ChannelType_eInBand)
        {
            BDBG_MSG((" frontend index %u Docsis QAM channel %u",i,st3255Channel));
            NEXUS_Frontend_GetDefault3255ChannelSettings(&st3255ChannelSettings);
            st3255ChannelSettings.channelNumber = st3255Channel;
            pConfig->frontend[i] = NEXUS_Frontend_Open3255Channel(st3255DeviceHandle,&st3255ChannelSettings);
            if ( NULL == (pConfig->frontend[i]) )
            {
                BDBG_ERR(("Unable to open onboard 3255 tuner/demodulator %d",i));
                continue;
            }
            NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
            userParams.isMtsif = st3255DeviceSettings.mtsif;
            userParams.param1 = st3255DeviceSettings.mtsif ? st3255ChannelSettings.channelNumber : NEXUS_InputBand_e0 + st3255Channel;
            userParams.chipId = 0x3255; /* 3255 API for BCM3383*/
            userParams.pParam2 = NULL;
            NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
            #if 0
            NEXUS_Frontend_GetTransportSettings(pConfig->frontend[i],&transportSettings);
            transportSettings.bandmap.virtualParserBandNum = NEXUS_InputBand_e0 + i;
            transportSettings.bandmap.virtualParserIsPlayback = false;
            NEXUS_Frontend_SetTransportSettings(pConfig->frontend[i],&transportSettings);
            #endif
            i++;
        }
        else
        {
            BDBG_MSG(("Docsis Non-QAM channel %u",st3255Channel));
        }
    }


    /*
        * If OOB channel is present in the Docsis device, check for the channel number
        */
    if(st3255DeviceCapabilities.isOobChannelSupported)
    {
        for(st3255Channel=0;st3255Channel<st3255DeviceCapabilities.totalChannels;st3255Channel++)
        {
            NEXUS_Frontend_Get3255ChannelCapabilities(st3255DeviceHandle,st3255Channel,&st3255ChannelCapabilities);
            if(st3255ChannelCapabilities.channelType == NEXUS_3255ChannelType_eOutOfBand)
            {
                BDBG_MSG(("Found Docsis OOB channel index %u",st3255Channel));
                break;
            }
        }
    }
    if (NEXUS_StrCmp(NEXUS_GetEnv("disable_oob_frontend"), "y") != 0)
    {
        BDBG_MSG(("Opening onboard 3255 OOB %u",i));
        NEXUS_Frontend_GetDefault3255ChannelSettings(&st3255ChannelSettings);
        st3255ChannelSettings.channelNumber = st3255Channel;
        pConfig->frontend[i] = NEXUS_Frontend_Open3255Channel(st3255DeviceHandle,&st3255ChannelSettings);
        if ( NULL == (pConfig->frontend[i]) )
        {
            BDBG_ERR(("Unable to open 3255 tuner/demodulator OOB %d",i));
        }
        NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
        userParams.param1 = st3255DeviceSettings.mtsif ? st3255ChannelSettings.channelNumber : NEXUS_InputBand_e0+i;
        userParams.isMtsif = st3255DeviceSettings.mtsif;
        userParams.chipId = 0x3255; /* 3255 API for BCM3383*/
        userParams.pParam2 = NULL;
        NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
    }
    else
    {
        BDBG_MSG(("env - disable_oob_frontend set"));
    }


    return BERR_SUCCESS;
}

void NEXUS_Platform_UninitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    unsigned i;

    for (i=0; i<NEXUS_MAX_FRONTENDS; i++)
    {
        if (pConfig->frontend[i])
        {
            BDBG_MSG(("NEXUS_Platform_UninitFrontend frontend %u",i));
            NEXUS_Frontend_Close(pConfig->frontend[i]);
            pConfig->frontend[i] = NULL;
        }
    }
    if (st3255DeviceHandle)
    {
        NEXUS_Frontend_Close3255Device(st3255DeviceHandle);
        st3255DeviceHandle = NULL;
    }
}

#elif NEXUS_FRONTEND_4538
#include "nexus_spi.h"
#include "nexus_frontend_4538.h"

static   NEXUS_GpioHandle gpioHandle = NULL;

/* Uncomment the following define in order to disable the i2c address search */
/*#define NEXUS_PLATFORM_BYPASS_I2C_ADDRESS_SEARCH 1*/

#define I2C_4538_INDEX_1 0

#define NUM_4538_CHANNELS_PER 8
#define I2C_4538_ADDRESS_1 0x68
#define EXT_4538_IRQ_1 10

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    unsigned i=0;
    NEXUS_FrontendDevice4538OpenSettings st4538Settings;
    NEXUS_GpioSettings gpioSettings;
    NEXUS_FrontendUserParameters userParams;
    uint16_t i2cAddr = I2C_4538_ADDRESS_1;
    NEXUS_FrontendDeviceHandle device;
    NEXUS_4538ProbeResults probeResults;

    if (!pConfig->i2c[I2C_4538_INDEX_1]) {
        BDBG_ERR(("Frontend cannot be initialized without first initializing I2C."));
        return BERR_NOT_INITIALIZED;
    }

    NEXUS_FrontendDevice_GetDefault4538OpenSettings(&st4538Settings);

    /* GPIO 46 is connected to the 4538 reset, so set it high to reset the 4538 */
    st4538Settings.reset.enable = true;
    st4538Settings.reset.pin = 46;
    st4538Settings.reset.type = NEXUS_GpioType_eStandard;
    st4538Settings.reset.value = NEXUS_GpioValue_eHigh;

    st4538Settings.i2cDevice = pConfig->i2c[I2C_4538_INDEX_1];
    st4538Settings.isrNumber = EXT_4538_IRQ_1;
    st4538Settings.gpioInterrupt = NULL;
    st4538Settings.i2cAddr = I2C_4538_ADDRESS_1;

    BDBG_MSG(("Checking i2c: 0x%02x",I2C_4538_ADDRESS_1));
    if (!NEXUS_Frontend_Probe4538(&st4538Settings, &probeResults)) {
        if (probeResults.chip.familyId == 0x4538) {
            i2cAddr = I2C_4538_ADDRESS_1;
        }
    } else {
#if NEXUS_PLATFORM_BYPASS_I2C_ADDRESS_SEARCH
        BDBG_ERR(("Unable to locate 4538 at 0x%02x",I2C_4538_ADDRESS_1));
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
#else
        int ix;
        for (ix=0x68; ix<=0x6f; ix++) {
            BDBG_MSG(("Checking i2c: 0x%02x",ix));
            st4538Settings.i2cAddr = ix;
            if (ix != I2C_4538_ADDRESS_1) {
                if (!NEXUS_Frontend_Probe4538(&st4538Settings, &probeResults)) {
                    if (probeResults.chip.familyId == 0x4538) {
                        i2cAddr = ix;
                        BDBG_MSG(("Found 4538 at 0x%02x",ix));
                        break;
                    }
                }
            }
        }
        if (i2cAddr == I2C_4538_ADDRESS_1) {
            BDBG_ERR(("Unable to locate 4538"));
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
#endif
    }

    st4538Settings.i2cAddr = i2cAddr;

    device = NEXUS_FrontendDevice_Open4538(0, &st4538Settings);
    if (device) {
        for (i=0; i <  NUM_4538_CHANNELS_PER; i++)
        {
            NEXUS_4538Settings channelSettings;
            channelSettings.device = device;
            channelSettings.channelNumber = i;
            pConfig->frontend[i] = NEXUS_Frontend_Open4538(&channelSettings);
            if ( NULL == (pConfig->frontend[i]) )
            {
                BDBG_ERR(("Unable to open onboard 4538 tuner/demodulator %d",i));
            }
            NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
            userParams.isMtsif = true;
            userParams.param1 = userParams.isMtsif ? channelSettings.channelNumber : NEXUS_InputBand_e0 + i;
            userParams.pParam2 = NULL;
            NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
        }
    } else {
        BDBG_ERR(("Unable to open 4538 device"));
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    return NEXUS_SUCCESS;
}

void NEXUS_Platform_UninitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    unsigned i=0, j=0;
    NEXUS_FrontendDeviceHandle tempHandle, deviceHandles[NEXUS_MAX_FRONTENDS];
    bool handleFound = false;

    BKNI_Memset(deviceHandles, 0, sizeof(deviceHandles));

    for (i=0; i<NEXUS_MAX_FRONTENDS; i++)
    {
        handleFound = false;
        if (pConfig->frontend[i]) {
            tempHandle = NEXUS_Frontend_GetDevice(pConfig->frontend[i]);
            if(tempHandle != NULL){
                for( j = 0; j<i; j++){
                    if(tempHandle == deviceHandles[j])
                        handleFound = true;
                }
                if(!handleFound)
                    deviceHandles[j] = tempHandle;
            }
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
#else /* NEXUS_FRONTEND_3255 */
NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    return BERR_SUCCESS;
}

void NEXUS_Platform_UninitFrontend(void)
{
    return;
}
#endif /* NEXUS_FRONTEND_3255 */

#elif (NEXUS_PLATFORM == 97241) || (NEXUS_PLATFORM == 972415)

/* This is only being used this way for now, as there are no other reference boards supporting 7241 + 3128 combination other than the daughter cards. */
#if NEXUS_FRONTEND_3128
#include "nexus_frontend_3128.h"
#ifdef NEXUS_FRONTEND_REVERSE_RMAGNUM_SUPPORT
#include "nexus_docsis.h"
#endif

#define PLATFORM_MAX_TUNERS 1
NEXUS_TunerHandle tunerHandle[PLATFORM_MAX_TUNERS];
#ifdef NEXUS_FRONTEND_REVERSE_RMAGNUM_SUPPORT
NEXUS_FrontendDeviceHandle hDocsisFrontendDevice = NULL;
#endif

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    NEXUS_FrontendUserParameters userParams;
    unsigned i = 0, channelCount=0;
    NEXUS_3128ProbeResults results;
    NEXUS_Frontend3128Settings st3128Settings;
    NEXUS_FrontendDevice3128OpenSettings st3128DeviceOpenSettings;
    NEXUS_TunerOpen3128Settings tunerOpenSettings;
#ifdef NEXUS_FRONTEND_REVERSE_RMAGNUM_SUPPORT
    NEXUS_DocsisOpenDeviceSettings openDeviceSettings;
#endif

    NEXUS_Frontend_GetDefault3128Settings(&st3128Settings);

    NEXUS_FrontendDevice_GetDefault3128OpenSettings(&st3128DeviceOpenSettings);
#if NEXUS_PLATFORM_7241_DCSFBTSFF
    st3128DeviceOpenSettings.isrNumber = 10;
#else
    st3128DeviceOpenSettings.isrNumber = 11;
#endif
    st3128DeviceOpenSettings.i2cDevice = pConfig->i2c[3];    /* Onboard tuner/demod use BSC_M3.*/
    st3128DeviceOpenSettings.i2cAddr = 0x6c ;
    st3128DeviceOpenSettings.outOfBand.ifFrequency = 0;
    st3128DeviceOpenSettings.inBandOpenDrain=true;
    st3128DeviceOpenSettings.loadAP = true;
    st3128DeviceOpenSettings.configureWatchdog = false;
#if (BCHP_VER >= BCHP_VER_B0)
    st3128DeviceOpenSettings.isMtsif = true;
#else
    st3128DeviceOpenSettings.isMtsif = false;
#endif

    NEXUS_Frontend_Probe3128(&st3128DeviceOpenSettings, &results);
    BDBG_WRN(("chip.familyId = 0x%x", results.chip.familyId));
    BDBG_WRN(("chip.id = 0x%x", results.chip.id));
    BDBG_WRN(("version.major = 0x%x", results.chip.version.major ));
    BDBG_WRN(("version.minor = 0x%x", results.chip.version.minor ));

    st3128Settings.device = NEXUS_FrontendDevice_Open3128(0, &st3128DeviceOpenSettings);
    if (NULL == st3128Settings.device)
    {
        BDBG_ERR(("Unable to open onboard 3128 tuner/demodulator device"));
        rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    }

    st3128Settings.type = NEXUS_3128ChannelType_eInBand;
    channelCount = results.chip.id & 0xF;
    if(results.chip.id == 0x3145)channelCount=3;
    else if(results.chip.id == 0x3147)channelCount=4;
    else if(results.chip.id == 0x3127)channelCount=4;
    else if(results.chip.id == 0x3184)channelCount=8;
    else if(results.chip.id == 0x3124)channelCount=4;
    else if(results.chip.id == 0x3123)channelCount=3;
    else if(results.chip.id == 0x3122)channelCount=3;
    else if(results.chip.id == 0x3144)channelCount=4;

    for (i=0; i<channelCount; i++)
    {
        BDBG_MSG(("Waiting for onboard 3128 tuner/demodulator channel %d to initialize", i));
        st3128Settings.channelNumber = i;
        pConfig->frontend[i] = NEXUS_Frontend_Open3128(&st3128Settings);
        if (NULL == pConfig->frontend[i])
        {
            BDBG_ERR(("Unable to open onboard 3128 tuner/demodulator channel %d", i));
            continue;
        }
        NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
        userParams.param1 = st3128DeviceOpenSettings.isMtsif ? st3128Settings.channelNumber : NEXUS_InputBand_e0+i;
        userParams.isMtsif = st3128DeviceOpenSettings.isMtsif;
        userParams.chipId = 0x3128;
        NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
    }

    NEXUS_Tuner_GetDefaultOpen3128Settings(&tunerOpenSettings);
    tunerOpenSettings.device = st3128Settings.device;
    tunerHandle[0] = NEXUS_Tuner_Open3128(0, &tunerOpenSettings);
    if (NULL == tunerHandle[0])
    {
       BDBG_ERR(("Unable to open onboard 7584 Ifdac tuner."));
       return BERR_NOT_INITIALIZED;
    }

#ifdef NEXUS_FRONTEND_REVERSE_RMAGNUM_SUPPORT
    NEXUS_Docsis_GetDefaultOpenDeviceSettings(&openDeviceSettings);
    openDeviceSettings.dataTuner[0] = tunerHandle[0];
    openDeviceSettings.rpcTimeOut = 1000;
    hDocsisFrontendDevice = NEXUS_Docsis_OpenDevice(0,&openDeviceSettings);
#endif

done:
    return rc;

}

void NEXUS_Platform_UninitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    unsigned i=0, j=0;
    NEXUS_FrontendDeviceHandle tempHandle, deviceHandles[NEXUS_MAX_FRONTENDS];
    bool handleFound = false;

    BKNI_Memset(deviceHandles, 0, sizeof(deviceHandles));
    for(i=0; i<PLATFORM_MAX_TUNERS; i++){
        if(tunerHandle[i]){
            NEXUS_Tuner_Close(tunerHandle[i]);
            tunerHandle[i] = NULL;
        }
    }

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

#ifdef NEXUS_FRONTEND_REVERSE_RMAGNUM_SUPPORT
    if(hDocsisFrontendDevice)
    {
        NEXUS_FrontendDevice_Close(hDocsisFrontendDevice);
        hDocsisFrontendDevice = NULL;
    }
#endif

    return;
}

#elif NEXUS_FRONTEND_3461
#include "nexus_frontend_3461.h"
#ifdef USE_SPI_FRONTEND
static NEXUS_SpiHandle g_3461spi[NEXUS_NUM_SPI_CHANNELS];
#endif
NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    NEXUS_3461Settings st3461Settings;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_FrontendDeviceHandle parentDevice=NULL;
    NEXUS_FrontendDevice3461OpenSettings deviceOpenSettings;
    NEXUS_FrontendDevice3461Settings deviceSettings;
    NEXUS_FrontendDeviceLinkSettings linkSettings;
#ifdef USE_SPI_FRONTEND
    NEXUS_SpiSettings spiSettings;
#endif
    BDBG_WRN(("Waiting for the first 3461 Downstream frontend(7241_T2SFF) to initialize"));

    NEXUS_Frontend_GetDefault3461Settings(&st3461Settings);
    NEXUS_FrontendDevice_GetDefault3461OpenSettings(&deviceOpenSettings);

#ifdef USE_SPI_FRONTEND
    NEXUS_Spi_GetDefaultSettings(&spiSettings);
    spiSettings.clockActiveLow = true;
    g_3461spi[0] = NEXUS_Spi_Open(0, &spiSettings);
    if (!g_3461spi[0]) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    deviceOpenSettings.i2cDevice = NULL;
    deviceOpenSettings.spiDevice = g_3461spi[0];
    deviceOpenSettings.spiAddr = 0x40;
#else
    if (!pConfig->i2c[3]) {
            BDBG_ERR(("Frontend cannot be initialized without first initializing I2C."));
            rc = BERR_NOT_INITIALIZED; goto done;
    }
    deviceOpenSettings.i2cDevice = pConfig->i2c[3];    /* Onboard tuner/demod use BSC_M3.*/
    deviceOpenSettings.i2cAddr = 0x6c;
#endif

    deviceOpenSettings.isrNumber = 11;
    deviceOpenSettings.loadAP = true;
    deviceOpenSettings.crystalSettings.enableDaisyChain = true;

    parentDevice = NEXUS_FrontendDevice_Open3461(0, &deviceOpenSettings);
    if(parentDevice == NULL){
        BDBG_ERR(("Unable to open first onboard 3461 tuner/demodulator device"));
        rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    }

    NEXUS_FrontendDevice_GetDefault3461Settings(&deviceSettings);
    deviceSettings.rfDaisyChain = NEXUS_3461RfDaisyChain_eInternalLna;
    deviceSettings.rfInput = NEXUS_3461TunerRfInput_eInternalLna;
    deviceSettings.enableRfLoopThrough = true;
    deviceSettings.terrestrial = true;
    NEXUS_FrontendDevice_Set3461Settings(parentDevice, &deviceSettings);

    NEXUS_Frontend_GetDefault3461Settings(&st3461Settings);
    st3461Settings.device = parentDevice;
    st3461Settings.type = NEXUS_3461ChannelType_eDvbt; /*REDUNDANT for now as there is only one instance of any demod running. */
    st3461Settings.channelNumber = 0;                   /*REDUNDANT for now. */

    pConfig->frontend[0] = NEXUS_Frontend_Open3461(&st3461Settings);
    if (NULL == pConfig->frontend[0])
    {
        BDBG_ERR(("Unable to open onboard 3461 tuner/demodulator channel."));
        rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    }

    NEXUS_Frontend_GetUserParameters(pConfig->frontend[0], &userParams);
    userParams.param1 = NEXUS_InputBand_e0;
    userParams.pParam2 = NULL;
    NEXUS_Frontend_SetUserParameters(pConfig->frontend[0], &userParams);

    BDBG_WRN(("Waiting for the second 3461 Downstream frontend(7241_T2SFF) to initialize"));

#ifdef USE_SPI_FRONTEND
    g_3461spi[1] = NEXUS_Spi_Open(1, &spiSettings);
    if (!g_3461spi[1]) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    deviceOpenSettings.i2cDevice = NULL;
    deviceOpenSettings.spiDevice = g_3461spi[1];
    deviceOpenSettings.spiAddr = 0x40;

#else
    if (!pConfig->i2c[3]) {
            BDBG_ERR(("Frontend cannot be initialized without first initializing I2C."));
            rc = BERR_NOT_INITIALIZED; goto done;
    }
    deviceOpenSettings.i2cDevice = pConfig->i2c[3];   /* Onboard tuner/demod use BSC_M3.*/
    deviceOpenSettings.i2cAddr = 0x6d;
#endif
    deviceOpenSettings.isrNumber = 11;
    deviceOpenSettings.loadAP = true;
    deviceOpenSettings.crystalSettings.enableDaisyChain = false;

    st3461Settings.device = NEXUS_FrontendDevice_Open3461(0, &deviceOpenSettings);
    if(st3461Settings.device == NULL){
        BDBG_ERR(("Unable to open second nboard 3461 tuner/demodulator device"));
        rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    }

    NEXUS_FrontendDevice_GetDefault3461Settings(&deviceSettings);
    deviceSettings.rfDaisyChain = NEXUS_3461RfDaisyChain_eOff;
    deviceSettings.rfInput = NEXUS_3461TunerRfInput_eExternalLna;
    deviceSettings.enableRfLoopThrough = false;
    deviceSettings.terrestrial = true;
    NEXUS_FrontendDevice_Set3461Settings(st3461Settings.device, &deviceSettings);

    pConfig->frontend[1] = NEXUS_Frontend_Open3461(&st3461Settings);
    if (NULL == pConfig->frontend[1])
    {
        BDBG_ERR(("Unable to open onboard 3461 tuner/demodulator channel."));
        rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    }

    NEXUS_Frontend_GetUserParameters(pConfig->frontend[1], &userParams);
    userParams.param1 = NEXUS_InputBand_e1;
    userParams.pParam2 = NULL;
    NEXUS_Frontend_SetUserParameters(pConfig->frontend[1], &userParams);

    NEXUS_FrontendDevice_GetDefaultLinkSettings(&linkSettings);

    rc = NEXUS_FrontendDevice_Link(parentDevice, st3461Settings.device, &linkSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    return NEXUS_SUCCESS;
done:
    if(pConfig->frontend[0]){
        NEXUS_Frontend_Close(pConfig->frontend[0]);
        pConfig->frontend[0] = NULL;
    }
    if(pConfig->frontend[1]){
        NEXUS_Frontend_Close(pConfig->frontend[1]);
        pConfig->frontend[1] = NULL;
    }
    if(parentDevice){
        NEXUS_FrontendDevice_Close(parentDevice);
        parentDevice = NULL;
    }
    if(st3461Settings.device){
        NEXUS_FrontendDevice_Close(st3461Settings.device);
        st3461Settings.device = NULL;
    }
    return rc;
}

void NEXUS_Platform_UninitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    unsigned i=0, j=0;
    NEXUS_FrontendDeviceHandle tempHandle, deviceHandles[NEXUS_MAX_FRONTENDS];
    bool handleFound = false;

    BKNI_Memset(deviceHandles, 0, sizeof(deviceHandles));

    for (i=0; i<NEXUS_MAX_FRONTENDS; i++)
    {
        handleFound = false;
        if (pConfig->frontend[i])
        {
            tempHandle = NEXUS_Frontend_GetDevice(pConfig->frontend[i]);
            BDBG_MSG(("tempHandle = 0x%x", tempHandle));

            if(tempHandle != NULL){
                for( j = 0; j<i; j++){
                    if(tempHandle == deviceHandles[j])
                    handleFound = true;
                }
                if(!handleFound)
                    deviceHandles[j] = tempHandle;
            }
            NEXUS_Frontend_Close(pConfig->frontend[i]);
            pConfig->frontend[i] = NULL;
        }
    }

    for (i=0; i<NEXUS_MAX_FRONTENDS; i++)
    {
        if (deviceHandles[i])
        {
            BDBG_MSG(("deviceHandles[%d] = 0x%x", i, deviceHandles[i]));
            NEXUS_FrontendDevice_Close(deviceHandles[i]);
            deviceHandles[i] = NULL;
        }
    }

#ifdef USE_SPI_FRONTEND
    for (i=0; i<NEXUS_NUM_SPI_CHANNELS; i++)
    {
        if (g_3461spi[i])
        {
            BDBG_MSG(("deviceHandles[%d] = 0x%x", i, deviceHandles[i]));
            NEXUS_Spi_Close(g_3461spi[i]);
            g_3461spi[i] = NULL;
        }
    }
#endif
    return;
}
#elif NEXUS_FRONTEND_4506
#include "nexus_frontend_4506.h"

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    NEXUS_4506Settings st4506Settings;
    NEXUS_FrontendUserParameters userParams;

    BDBG_WRN(("Initializing 4506[0]..."));

    NEXUS_Frontend_GetDefault4506Settings(&st4506Settings);
    if (!pConfig->i2c[3]) {
            BDBG_ERR(("Frontend cannot be initialized without first initializing I2C."));
            return BERR_NOT_INITIALIZED;
    }
    st4506Settings.i2cDevice = pConfig->i2c[3];    /* Onboard tuner/demod use BSC_M3.*/
    st4506Settings.i2cAddr = 0x69;
    st4506Settings.isrNumber = 11;
    st4506Settings.channelNumber = 0;
    st4506Settings.is3445ExternalLna = false;
    st4506Settings.bitWideSync = false;

    pConfig->frontend[0] = NEXUS_Frontend_Open4506(&st4506Settings);
    if (NULL == pConfig->frontend[0])
    {
        BDBG_ERR(("Unable to open 4506 tuner/demodulator channel."));
        rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    }

    NEXUS_Frontend_GetUserParameters(pConfig->frontend[0], &userParams);
    userParams.param1 = NEXUS_InputBand_e0; /* PKT1 */
    userParams.pParam2 = NULL;
    NEXUS_Frontend_SetUserParameters(pConfig->frontend[0], &userParams);

    BDBG_WRN(("Initializing 4506[1]..."));

    st4506Settings.channelNumber = 1;

    pConfig->frontend[1] = NEXUS_Frontend_Open4506(&st4506Settings);
    if (NULL == pConfig->frontend[1])
    {
        BDBG_ERR(("Unable to open 4506 tuner/demodulator channel."));
        rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    }

    NEXUS_Frontend_GetUserParameters(pConfig->frontend[1], &userParams);
    userParams.param1 = NEXUS_InputBand_e1; /* PKT2 */
    userParams.pParam2 = NULL;
    NEXUS_Frontend_SetUserParameters(pConfig->frontend[1], &userParams);

done:
    return rc;
}

void NEXUS_Platform_UninitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    unsigned i=0, j=0;
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

#else /* NEXUS_PLATFORM */
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

NEXUS_Error
NEXUS_Platform_GetStreamerInputBand(unsigned index, NEXUS_InputBand *pInputBand)
{
    BDBG_ASSERT(pInputBand);
    if (index > 0) {
        BDBG_ERR(("Only 1 streamer input available"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BTRC_TRACE(ChnChange_TuneStreamer, START);
#if (NEXUS_PLATFORM == 97428) || (NEXUS_PLATFORM == 974285)
    *pInputBand = NEXUS_InputBand_e0;
#elif (NEXUS_PLATFORM == 97429) || (NEXUS_PLATFORM == 974295)
    *pInputBand = NEXUS_InputBand_e0;  /* 9729 CV support two streamer inputs, select 0 */
#elif (NEXUS_PLATFORM == 97241) || (NEXUS_PLATFORM == 972415)
    *pInputBand = NEXUS_InputBand_e4;
#endif
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





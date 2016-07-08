/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2004-2016 Broadcom. All rights reserved.
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
*
***************************************************************************/
#include "nexus_types.h"
#include "priv/nexus_core.h"
#include "nexus_platform_features.h"
#include "nexus_platform_priv.h"
#include "nexus_platform.h"
#include "nexus_base.h"
#include "nexus_input_band.h"
#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#include "nexus_frontend_7552.h"
#if NEXUS_USE_3128_FRONTEND_DAUGHTER_CARD
#include "nexus_frontend_3128.h"
#endif
#if NEXUS_FRONTEND_3461
#include "nexus_frontend_3461.h"
#endif
#endif

BDBG_MODULE(nexus_platform_frontend);

#if NEXUS_HAS_FRONTEND
#if NEXUS_FRONTEND_3461
NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    NEXUS_3461Settings st3461Settings;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_FrontendDeviceHandle parentDevice;
    NEXUS_FrontendDevice3461OpenSettings deviceOpenSettings;
    NEXUS_FrontendDevice3461Settings deviceSettings;
    NEXUS_FrontendType type;

    BDBG_WRN(("Waiting for 3461 Downstream frontend to initialize"));

    if (!pConfig->i2c[3]) {
            BDBG_ERR(("Frontend cannot be initialized without first initializing I2C."));
            return BERR_NOT_INITIALIZED;
    }
    deviceOpenSettings.i2cDevice = pConfig->i2c[3];    /* Onboard tuner/demod use BSC_M1.*/
    deviceOpenSettings.i2cAddr = 0x6c;
    deviceOpenSettings.isrNumber = 9;
    deviceOpenSettings.loadAP = true;
    deviceOpenSettings.externalFixedGain.total = 700;       /* These are platform specific values given by the board designer. */
    deviceOpenSettings.externalFixedGain.bypassable = 1400; /* These are platform specific values given by the board designer. */

    parentDevice = NEXUS_FrontendDevice_Open3461(0, &deviceOpenSettings);
    
    if(parentDevice == NULL){rc = BERR_TRACE(rc); goto done;}
    
    NEXUS_FrontendDevice_GetDefault3461Settings(&deviceSettings);
    deviceSettings.rfDaisyChain = NEXUS_3461RfDaisyChain_eInternalLna;
    deviceSettings.rfInput = NEXUS_3461TunerRfInput_eInternalLna;
    deviceSettings.enableRfLoopThrough = true;
    deviceSettings.terrestrial = true;
    NEXUS_FrontendDevice_Set3461Settings(parentDevice, &deviceSettings);


    NEXUS_Frontend_GetDefault3461Settings(&st3461Settings);
    st3461Settings.device = parentDevice;
    st3461Settings.type = NEXUS_3461ChannelType_eDvbt; /*REDUNDANT for now as there is only one instance of any demod running. */
    st3461Settings.channelNumber = 0;                    /*REDUNDANT for now. */

    pConfig->frontend[0] = NEXUS_Frontend_Open3461(&st3461Settings);
    if (NULL == pConfig->frontend[0])
    {
        BDBG_ERR(("Unable to open onboard 3461 dvbt2 tuner/demodulator "));
    }

    NEXUS_Frontend_GetType(pConfig->frontend[0], &type);
    BDBG_MSG(("familyId = 0x%x", type.chip.familyId));
    BDBG_MSG(("chipId = 0x%x", type.chip.id));
    BDBG_MSG(("version.major = 0x%x", type.chip.version.major ));
    BDBG_MSG(("version.major = 0x%x", type.chip.version.minor ));
    BDBG_MSG(("version.buildType = 0x%x", type.chip.version.buildType ));
    BDBG_MSG(("version.buildId = 0x%x", type.chip.version.buildId ));
    BDBG_MSG(("bondoutOptions[0] = 0x%x", type.chip.bondoutOptions[0] ));
    BDBG_MSG(("bondoutOptions[1] = 0x%x", type.chip.bondoutOptions[1] ));
    
    BDBG_MSG(("firmwareVersion.major = 0x%x", type.firmwareVersion.major ));
    BDBG_MSG(("firmwareVersion.major = 0x%x", type.firmwareVersion.minor ));
    BDBG_MSG(("firmwareVersion.buildType = 0x%x", type.firmwareVersion.buildType ));
    BDBG_MSG(("firmwareVersion.buildId = 0x%x", type.firmwareVersion.buildId ));

    NEXUS_Frontend_GetUserParameters(pConfig->frontend[0], &userParams);
    userParams.param1 = NEXUS_InputBand_e6;
    userParams.pParam2 = 0;
    NEXUS_Frontend_SetUserParameters(pConfig->frontend[0], &userParams);

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

    for ( i = 0; i < NEXUS_MAX_FRONTENDS; i++ )
    {
        handleFound = false;
        if ( NULL != pConfig->frontend[i] )
        {
            tempHandle = NEXUS_Frontend_GetDevice(pConfig->frontend[i]);
            for( j = 0; j<i; j++){
                if(tempHandle == deviceHandles[j])
                handleFound = true;
            }
            if(!handleFound)
                deviceHandles[j] = tempHandle;
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
}

#else

static NEXUS_GpioHandle gpioHandle = NULL;

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    NEXUS_PlatformStatus platformStatus;    
    int index=0;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_Error rc;
    NEXUS_GpioSettings gpioSettings;
    NEXUS_FrontendDevice7552OpenSettings deviceOpenSettings;
    NEXUS_Frontend7552Settings settings;        
    NEXUS_FrontendDevice7552Settings deviceSettings;
#if NEXUS_USE_3128_FRONTEND_DAUGHTER_CARD
    unsigned i=0;
    NEXUS_3128Settings st3128Settings;
    NEXUS_3128ProbeResults results;
#endif        
    rc = NEXUS_Platform_GetStatus(&platformStatus);
    if(rc != NEXUS_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto error;
    }

    NEXUS_Frontend_GetDefault7552Settings(&settings);

    NEXUS_FrontendDevice_GetDefault7552OpenSettings(&deviceOpenSettings);
    /* default setting support ISDB-T, disable for DVB-T only chip */
    if((platformStatus.chipId == 0x7551) || (platformStatus.chipId == 0x7552))
    {
        deviceOpenSettings.supportIsdbt = false;
    }
    else{
        deviceOpenSettings.supportIsdbt = true;
    }

    deviceOpenSettings.externalFixedGain.total = 700;
    deviceOpenSettings.externalFixedGain.bypassable = 1400;

    settings.device = NEXUS_FrontendDevice_Open7552(0, &deviceOpenSettings);
    if (NULL == settings.device)
    {
       BDBG_ERR(("Unable to open first 7552 tuner/demodulator device"));
       rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto error;
    }

    NEXUS_FrontendDevice_GetDefault7552Settings(&deviceSettings);
    deviceSettings.rfInput = NEXUS_7552TunerRfInput_eInternalLna;
    deviceSettings.enableRfLoopThrough = false;
    rc = NEXUS_FrontendDevice_Set7552Settings(settings.device, &deviceSettings);    
    if(rc) {
        rc = BERR_TRACE(rc); goto error;
    }

    BDBG_MSG(("Chip ID = 0x%x", platformStatus.chipId));
    if((platformStatus.chipId == 0x7581) || (platformStatus.chipId == 0x7582) ||
        (platformStatus.chipId == 0x7591) || (platformStatus.chipId == 0x7592) ||
        (platformStatus.chipId == 0x7574) || (platformStatus.chipId == 0x7023))
    {
        settings.type = NEXUS_7552ChannelType_eQam;
        pConfig->frontend[index] = NEXUS_Frontend_Open7552(&settings);
        if ( NULL == pConfig->frontend[index] )
        {
            (void)BERR_TRACE(BERR_NOT_SUPPORTED);
            goto error;
        }
        BDBG_MSG(("pConfig->frontend[%d] = 0x%x", index, pConfig->frontend[index]));
        NEXUS_Frontend_GetUserParameters(pConfig->frontend[index], &userParams);
        userParams.param1 = NEXUS_InputBand_e1;
        userParams.pParam2 = 0;
        NEXUS_Frontend_SetUserParameters(pConfig->frontend[index], &userParams);
    
        index++;
    }

    if((platformStatus.chipId == 0x7531) || (platformStatus.chipId == 0x7532) ||
        (platformStatus.chipId == 0x7541) || (platformStatus.chipId == 0x7542) ||
        (platformStatus.chipId == 0x7551) || (platformStatus.chipId == 0x7552) ||        
        (platformStatus.chipId == 0x7591) || (platformStatus.chipId == 0x7592))
    {
        settings.type = NEXUS_7552ChannelType_eOfdm;
        pConfig->frontend[index] = NEXUS_Frontend_Open7552(&settings);
        if ( NULL == pConfig->frontend[index] )
        {
            (void)BERR_TRACE(BERR_NOT_SUPPORTED);
            goto error;
        }
        BDBG_MSG(("pConfig->frontend[%d] = 0x%x", index, pConfig->frontend[index]));

        NEXUS_Frontend_GetUserParameters(pConfig->frontend[index], &userParams);
        userParams.param1 = NEXUS_InputBand_e0;
        userParams.pParam2 = 0;
        NEXUS_Frontend_SetUserParameters(pConfig->frontend[index], &userParams);

        index++;
    }

#ifndef NEXUS_FRONTEND_7552_A0
    if((platformStatus.chipId == 0x7574) || (platformStatus.chipId == 0x7592))
    {
        settings.type = NEXUS_7552ChannelType_eOutOfBand;
        pConfig->frontend[index] = NEXUS_Frontend_Open7552(&settings);
        if ( NULL == pConfig->frontend[index] )
        {
           (void)BERR_TRACE(BERR_NOT_SUPPORTED);
           goto error;
        }
        
        NEXUS_Frontend_GetUserParameters(pConfig->frontend[index], &userParams);
        userParams.param1 = NEXUS_InputBand_e9;
        userParams.pParam2 = 0;
        NEXUS_Frontend_SetUserParameters(pConfig->frontend[index], &userParams);

        index++;
    }
#endif
#if NEXUS_USE_3128_FRONTEND_DAUGHTER_CARD
#if (BCHP_VER >= BCHP_VER_B0)
    NEXUS_Frontend_GetDefault3128Settings(&st3128Settings);

    st3128Settings.i2cDevice = pConfig->i2c[1];
    st3128Settings.i2cAddr = 0x6c;
    st3128Settings.isrNumber = 10; /* Make sure that this number is correct. */
    st3128Settings.inBandOpenDrain=true;
    st3128Settings.loadAP = true;
    st3128Settings.type = NEXUS_3128ChannelType_eInBand;
    st3128Settings.ifFrequency = 0;
    st3128Settings.isMtsif = true;
    /* NEXUS_MAX_FRONTENDS=9; BCM3128 has 8 InBand Channels and 1 OOB channel
    * Open the BCM3128 InBand channels
    */
    rc = NEXUS_Frontend_Probe3128(&st3128Settings, &results);
    if(rc) return BERR_TRACE(BERR_NOT_INITIALIZED);

    for (i=0; i < NEXUS_3128_MAX_DOWNSTREAM_CHANNELS;i++)
    {
        BDBG_MSG(("Waiting for onboard 3128 tuner/demodulator channel %d to initialize", i));
        st3128Settings.channelNumber = i;
        pConfig->frontend[index] = NEXUS_Frontend_Open3128(&st3128Settings);
        if (NULL == pConfig->frontend[index])
        {
            BDBG_ERR(("Unable to open onboard 3128 tuner/demodulator channel %d", i));
            continue;
        }

        NEXUS_Frontend_GetUserParameters(pConfig->frontend[index], &userParams);
        userParams.isMtsif = st3128Settings.isMtsif;
        userParams.param1 = st3128Settings.isMtsif ? st3128Settings.channelNumber : NEXUS_InputBand_e0 + i;
        userParams.pParam2 = 0;
        userParams.chipId = 0x3128;
        NEXUS_Frontend_SetUserParameters(pConfig->frontend[index], &userParams);
        index++;
    }
#endif
#endif

    /* set aon_gpio_04 to gpio output for external LNA */
    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eAonStandard, &gpioSettings); 
    gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
    gpioSettings.value = NEXUS_GpioValue_eHigh;
    gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
    gpioHandle = NEXUS_Gpio_Open(NEXUS_GpioType_eAonStandard, 4, &gpioSettings);

    return BERR_SUCCESS;

error:
    while ( index-- > 0 )
    {
        NEXUS_Frontend_Close(pConfig->frontend[index]);
        pConfig->frontend[index] = NULL;
    }
    return BERR_TRACE(BERR_NOT_INITIALIZED);
}

void NEXUS_Platform_UninitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    unsigned i=0, j=0;
    NEXUS_FrontendDeviceHandle tempHandle, deviceHandles[NEXUS_MAX_FRONTENDS];
    bool handleFound = false;

    BKNI_Memset(deviceHandles, 0, sizeof(deviceHandles));

    for ( i = 0; i < NEXUS_MAX_FRONTENDS; i++ )
    {
        handleFound = false;
        if ( NULL != pConfig->frontend[i] )
        {
            tempHandle = NEXUS_Frontend_GetDevice(pConfig->frontend[i]);
            for( j = 0; j<i; j++){
                if(tempHandle == deviceHandles[j])
                handleFound = true;
            }
            if(!handleFound)
                deviceHandles[j] = tempHandle;
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
}

#endif

#else
NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_Error errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
}

void NEXUS_Platform_UninitFrontend(void)
{
    NEXUS_Error errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
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
#if (BCHP_VER >= BCHP_VER_B0)   
    *pInputBand = NEXUS_InputBand_e5;
#else
    *pInputBand = NEXUS_InputBand_e2;
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


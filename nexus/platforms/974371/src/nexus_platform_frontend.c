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
* Module Description:
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*   API name: Platform linuxuser
*    linuxuser OS routines
*
*
* Revision History:
*
* $brcm_Log: $
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
#include "bchp_aon_ctrl.h"

BDBG_MODULE(nexus_platform_frontend);

#if NEXUS_FRONTEND_3128
#include "nexus_frontend_3128.h"
#endif

#if NEXUS_USE_74371_XID
#ifdef NEXUS_FRONTEND_REVERSE_RMAGNUM_SUPPORT
#include "nexus_docsis.h"
#endif

#define PLATFORM_MAX_TUNERS 1
NEXUS_TunerHandle tunerHandle[PLATFORM_MAX_TUNERS];
#ifdef NEXUS_FRONTEND_REVERSE_RMAGNUM_SUPPORT
NEXUS_FrontendDeviceHandle hDocsisFrontendDevice = NULL;
#endif
static NEXUS_GpioHandle gpioHandle = NULL;
static NEXUS_GpioHandle tunerGpio = NULL;
static NEXUS_SpiHandle g_3128spi;

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    NEXUS_FrontendUserParameters userParams;
    unsigned i = 0, channelCount=0;
    NEXUS_3128ProbeResults results;
    NEXUS_Frontend3128Settings st3128Settings;
    NEXUS_FrontendDevice3128OpenSettings st3128DeviceOpenSettings;
    NEXUS_GpioSettings gpioSettings;
#ifdef NEXUS_FRONTEND_REVERSE_RMAGNUM_SUPPORT
    NEXUS_DocsisOpenDeviceSettings openDeviceSettings;
#endif
     NEXUS_SpiSettings spiSettings;

    NEXUS_Frontend_GetDefault3128Settings(&st3128Settings);

    NEXUS_FrontendDevice_GetDefault3128OpenSettings(&st3128DeviceOpenSettings);

    NEXUS_Spi_GetDefaultSettings(&spiSettings);
    spiSettings.clockActiveLow = true;
    g_3128spi = NEXUS_Spi_Open(1, &spiSettings);
    if (!g_3128spi) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    st3128DeviceOpenSettings.i2cDevice = NULL;
    st3128DeviceOpenSettings.spiDevice = g_3128spi;
    st3128DeviceOpenSettings.spiAddr = 0x40;
    st3128DeviceOpenSettings.i2cDevice = NULL;

    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
    gpioSettings.mode = NEXUS_GpioMode_eInput;
    gpioSettings.interruptMode = NEXUS_GpioInterrupt_eFallingEdge;
     /* use level trigger interrupt */
    gpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;
    tunerGpio = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 103, &gpioSettings);
    if (NULL == tunerGpio)
    {
        BDBG_ERR(("Unable to open GPIO for tuner %d", 0));
    }
    st3128DeviceOpenSettings.gpioInterrupt = tunerGpio;
    st3128DeviceOpenSettings.isrNumber = 0;
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
    BDBG_MSG(("chip.familyId = 0x%x", results.chip.familyId));
    BDBG_MSG(("chip.id = 0x%x", results.chip.id));
    BDBG_MSG(("version.major = 0x%x", results.chip.version.major ));
    BDBG_MSG(("version.minor = 0x%x", results.chip.version.minor ));

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
		BDBG_MSG(("i = %d, pConfig->frontend[i] = 0x%x", i, pConfig->frontend[i]));
        NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
        userParams.param1 = st3128DeviceOpenSettings.isMtsif ? st3128Settings.channelNumber : NEXUS_InputBand_e0+i;
        userParams.isMtsif = st3128DeviceOpenSettings.isMtsif;
        userParams.chipId = 0x3128;
        NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
    }

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

    if(gpioHandle)
    {
        NEXUS_Gpio_Close(gpioHandle);
        gpioHandle = NULL;
    }
    if(tunerGpio)
    {
        NEXUS_Gpio_Close(tunerGpio);
        tunerGpio = NULL;
    }
    if (g_3128spi) {
        NEXUS_Spi_Close(g_3128spi);
        g_3128spi = NULL;
    }

    return;
}

#elif (NEXUS_USE_74371_SV && NEXUS_FRONTEND_3128) || NEXUS_USE_7437_USFF

static NEXUS_GpioHandle gpioHandle = NULL;
static NEXUS_GpioHandle tunerGpio = NULL;
#ifndef NEXUS_PLATFORM_974371_3128_USES_I2C
static NEXUS_SpiHandle g_3128spi;
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
    NEXUS_GpioSettings gpioSettings;
#ifndef NEXUS_PLATFORM_974371_3128_USES_I2C
    NEXUS_SpiSettings spiSettings;
#endif

#if NEXUS_USE_7437_USFF
    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eAonStandard, &gpioSettings);
    gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
    gpioSettings.value = NEXUS_GpioValue_eHigh;
    gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
    gpioSettings.interrupt.callback = NULL;
    gpioHandle = NEXUS_Gpio_Open(NEXUS_GpioType_eAonStandard, 1, &gpioSettings);
#endif

    NEXUS_Frontend_GetDefault3128Settings(&st3128Settings);

    NEXUS_FrontendDevice_GetDefault3128OpenSettings(&st3128DeviceOpenSettings);

#if NEXUS_PLATFORM_974371_3128_USES_I2C
    st3128DeviceOpenSettings.i2cDevice = pConfig->i2c[3];    /* Onboard tuner/demod use BSC_M3.*/
    st3128DeviceOpenSettings.i2cAddr = 0x6d;
    st3128DeviceOpenSettings.spiDevice = NULL;
#else
    NEXUS_Spi_GetDefaultSettings(&spiSettings);
    spiSettings.clockActiveLow = true;
    g_3128spi = NEXUS_Spi_Open(0, &spiSettings);
    if (!g_3128spi) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    st3128DeviceOpenSettings.i2cDevice = NULL;
    st3128DeviceOpenSettings.spiDevice = g_3128spi;
    st3128DeviceOpenSettings.spiAddr = 0x40;
    st3128DeviceOpenSettings.i2cDevice = NULL;
#endif

#if NEXUS_USE_7437_USFF
    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
    gpioSettings.mode = NEXUS_GpioMode_eInput;
    gpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;
    tunerGpio = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 0, &gpioSettings);
#elif NEXUS_USE_74371_SV
    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eAonStandard, &gpioSettings);
    gpioSettings.mode = NEXUS_GpioMode_eInput;
    gpioSettings.interruptMode = NEXUS_GpioInterrupt_eFallingEdge;
    tunerGpio = NEXUS_Gpio_Open(NEXUS_GpioType_eAonStandard, 03, &gpioSettings);
#endif
    if (NULL == tunerGpio)
    {
        BDBG_ERR(("Unable to open GPIO for tuner %d", 0));
    }

    st3128DeviceOpenSettings.gpioInterrupt = tunerGpio;
    st3128DeviceOpenSettings.isrNumber = 0;
    st3128DeviceOpenSettings.outOfBand.ifFrequency = 0;
    st3128DeviceOpenSettings.inBandOpenDrain=true;
    st3128DeviceOpenSettings.loadAP = true;
    st3128DeviceOpenSettings.configureWatchdog = false;
    st3128DeviceOpenSettings.isMtsif = true;

    NEXUS_Frontend_Probe3128(&st3128DeviceOpenSettings, &results);
    if(results.chip.familyId == 0x3128){
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
        else BDBG_WRN(("Unrecognized 3128 chip id %x, guessing number of channels is %d",results.chip.id,channelCount));

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
    }
    else {
#if NEXUS_USE_7437_USFF
        BDBG_WRN(("3128 not responding."));
#else
        BDBG_WRN(("3128 daughter card not connected."));
#endif
    }

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

#ifdef NEXUS_FRONTEND_REVERSE_RMAGNUM_SUPPORT
    if(hDocsisFrontendDevice)
    {
        NEXUS_FrontendDevice_Close(hDocsisFrontendDevice);
        hDocsisFrontendDevice = NULL;
    }
#endif

    if(tunerGpio)
    {
        NEXUS_Gpio_Close(tunerGpio);
        tunerGpio = NULL;
    }
#if !NEXUS_PLATFORM_974371_3128_USES_I2C
    if (g_3128spi) {
        NEXUS_Spi_Close(g_3128spi);
        g_3128spi = NULL;
    }
#endif
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

NEXUS_Error
NEXUS_Platform_GetStreamerInputBand(unsigned index, NEXUS_InputBand *pInputBand)
{
    BDBG_ASSERT(pInputBand);
    if (index > 0) {
        BDBG_ERR(("Only 1 streamer input available"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BTRC_TRACE(ChnChange_TuneStreamer, START);
#if NEXUS_USE_7445_SV
    *pInputBand = NEXUS_InputBand_e5;
#else
    *pInputBand = NEXUS_InputBand_e0;
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

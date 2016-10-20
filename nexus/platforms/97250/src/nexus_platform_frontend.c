/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#include "nexus_types.h"
#include "nexus_platform_priv.h"
#include "nexus_platform.h"
#include "priv/nexus_core.h"
#include "nexus_platform_features.h"
#include "nexus_base.h"
#include "nexus_input_band.h"
#include "bchp_gio.h"

BDBG_MODULE(nexus_platform_frontend);

#if NEXUS_HAS_FRONTEND && NEXUS_USE_7250_CD2
#include "nexus_frontend.h"
#include "nexus_gpio.h"

static NEXUS_GpioHandle gpioHandleInt = NULL;

#if NEXUS_HAS_SPI
static NEXUS_SpiHandle g_dc_spi[NEXUS_NUM_SPI_CHANNELS] = {NULL};
#endif

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    NEXUS_FrontendDeviceHandle device;
    NEXUS_FrontendUserParameters userParams;
    unsigned i=0;
    NEXUS_FrontendDeviceOpenSettings deviceSettings;
    NEXUS_GpioSettings gpioSettings;
    bool cd2 = false;
    bool i2c = false;
    unsigned interrupt = 10;
    NEXUS_GpioType interruptType = NEXUS_GpioType_eAonStandard;

    {
        NEXUS_PlatformStatus platformStatus;

        NEXUS_Platform_GetStatus(&platformStatus);
        BDBG_MSG(("board major: %d, minor: %d",platformStatus.boardId.major,platformStatus.boardId.minor));
        if (platformStatus.boardId.major == 10) {
            switch (platformStatus.boardId.minor) {
            case 0: /* v00 */
                interrupt = 10;
                i2c = true;
                cd2 = true;
                interruptType = NEXUS_GpioType_eAonStandard;
                break;
            default:
            case 1: /* v10 */
                interrupt = 12;
                i2c = false;
                cd2 = true;
                interruptType = NEXUS_GpioType_eAonStandard;
                break;
            }
        }
    }

#if NEXUS_HAS_SPI
    {
        NEXUS_SpiSettings spiSettings;
        NEXUS_Spi_GetDefaultSettings(&spiSettings);
        spiSettings.clockActiveLow = true;
        g_dc_spi[0] = NEXUS_Spi_Open(0, &spiSettings);
        if (!g_dc_spi[0]) {
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
    }
#endif

    NEXUS_FrontendDevice_GetDefaultOpenSettings(&deviceSettings);

    if (i2c) {
        deviceSettings.i2cDevice = pConfig->i2c[2];
        deviceSettings.i2cAddress = 0x6b;
    } else {
        deviceSettings.spiDevice = g_dc_spi[0];
    }

    BDBG_MSG(("Setting up interrupt on %sGPIO %d", interruptType == NEXUS_GpioType_eAonStandard ? "AON " : "" , interrupt));
    NEXUS_Gpio_GetDefaultSettings(interruptType, &gpioSettings);
    gpioSettings.mode = NEXUS_GpioMode_eInput;
    gpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;
    gpioHandleInt = NEXUS_Gpio_Open(interruptType, interrupt, &gpioSettings);
    BDBG_ASSERT(NULL != gpioHandleInt);
    deviceSettings.gpioInterrupt = gpioHandleInt;

    {
        NEXUS_FrontendProbeResults probeResults;

        BSTD_UNUSED(userParams);

        NEXUS_FrontendDevice_Probe(&deviceSettings, &probeResults);
        if (probeResults.chip.familyId != 0) {
            NEXUS_FrontendDeviceCapabilities capabilities;

            BDBG_WRN(("Opening %x...",probeResults.chip.familyId));
            device = NEXUS_FrontendDevice_Open(0, &deviceSettings);

            NEXUS_FrontendDevice_GetCapabilities(device, &capabilities);
            BDBG_MSG(("Opening %d %x frontends",capabilities.numTuners,probeResults.chip.familyId));
            for (i=0; i < capabilities.numTuners ; i++)
            {
                NEXUS_FrontendChannelSettings channelSettings;

                NEXUS_Frontend_GetDefaultOpenSettings(&channelSettings);

                channelSettings.device = device;
                channelSettings.channelNumber = i;
                if (probeResults.chip.familyId == 0x3158)
                    channelSettings.type = NEXUS_FrontendChannelType_eCable;
                else
                    channelSettings.type = NEXUS_FrontendChannelType_eSatellite;
                pConfig->frontend[i] = NEXUS_Frontend_Open(&channelSettings);
                if ( NULL == (pConfig->frontend[i]) )
                {
                    BDBG_ERR(("Unable to open %x demod %d (as frontend[%d])",probeResults.chip.familyId,i,i));
                    continue;
                }
                BDBG_MSG(("%xfe: %d(%d):%p",probeResults.chip.familyId,i,i,pConfig->frontend[i]));
            }
        } else {
            BDBG_ERR(("No frontend found."));
        }
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
    if(gpioHandleInt)
    {
        NEXUS_Gpio_Close(gpioHandleInt);
        gpioHandleInt = NULL;
    }
#if NEXUS_HAS_SPI
    for (i=0; i < NEXUS_NUM_SPI_CHANNELS; i++) {
        if (g_dc_spi[i]) {
            NEXUS_Spi_Close(g_dc_spi[i]);
            g_dc_spi[i] = NULL;
        }
    }
#endif
    return;
}

#elif NEXUS_HAS_FRONTEND && NEXUS_USE_72501_SAT
#include "nexus_frontend.h"
#include "nexus_frontend_45216.h"

#include "bchp_sun_top_ctrl.h"

#include "nexus_gpio.h"

#define NEXUS_PLATFORM_45208_GPIO_INTERRUPT 89
#define NEXUS_PLATFORM_45208_I2C 3
#define NEXUS_PLATFORM_45208_FRONTEND_STRING "45216"
#define NEXUS_PLATFORM_45208_FRONTEND_I2C_ADDRESS 0x68
#define NEXUS_PLATFORM_45208_MTSIF_OFFSET 2

static NEXUS_GpioHandle gpioHandleInt = NULL;

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    NEXUS_FrontendDeviceHandle device;
    NEXUS_FrontendUserParameters userParams;
    unsigned i=0;
    NEXUS_FrontendDevice45216OpenSettings deviceSettings;
    NEXUS_GpioSettings gpioSettings;

#if 0
    {
        BREG_Handle hReg = g_pCoreHandles->reg;
        uint32_t reg;
        reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15);
        reg &= ~(
                 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_060)
                 );
        reg |= (
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_060, 0) /* GPIO_060 */
                );
        BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15, reg);
    }
#endif

    NEXUS_FrontendDevice_GetDefault45216OpenSettings(&deviceSettings);

    deviceSettings.reset.enable = true;
    deviceSettings.reset.pin = 88;
    deviceSettings.reset.type = NEXUS_GpioType_eStandard;
    deviceSettings.reset.value = NEXUS_GpioValue_eHigh;

    deviceSettings.i2cDevice = pConfig->i2c[NEXUS_PLATFORM_45208_I2C];
    deviceSettings.i2cAddr = NEXUS_PLATFORM_45208_FRONTEND_I2C_ADDRESS;

    BDBG_MSG(("Setting up interrupt on GPIO %d",NEXUS_PLATFORM_45208_GPIO_INTERRUPT));
    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
    gpioSettings.mode = NEXUS_GpioMode_eInput;
    gpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;
    gpioHandleInt = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, NEXUS_PLATFORM_45208_GPIO_INTERRUPT, &gpioSettings);
    BDBG_ASSERT(NULL != gpioHandleInt);
    deviceSettings.gpioInterrupt = gpioHandleInt;

    BDBG_MSG(("Opening %s device",NEXUS_PLATFORM_45208_FRONTEND_STRING));
    device = NEXUS_FrontendDevice_Open45216(0, &deviceSettings);

    if (device) {
        unsigned j;
        NEXUS_FrontendDeviceCapabilities capabilities;
        NEXUS_FrontendDevice_GetCapabilities(device, &capabilities);
        BDBG_MSG(("Opening %d %s frontends",capabilities.numTuners,NEXUS_PLATFORM_45208_FRONTEND_STRING));
        for (j=0; j < capabilities.numTuners ; j++)
        {
            NEXUS_45216FrontendSettings channelSettings;
            channelSettings.device = device;
            channelSettings.channelNumber = j;
            pConfig->frontend[j+i] = NEXUS_Frontend_Open45216(&channelSettings);
            if ( NULL == (pConfig->frontend[j+i]) )
            {
                BDBG_ERR(("Unable to open %s demod %d (as frontend[%d])",NEXUS_PLATFORM_45208_FRONTEND_STRING,j,j+i));
                continue;
            }
            BDBG_MSG(("%sfe: %d(%d):%p",NEXUS_PLATFORM_45208_FRONTEND_STRING,j,j+i,pConfig->frontend[j+i]));
            NEXUS_Frontend_GetUserParameters(pConfig->frontend[j+i], &userParams);
            userParams.isMtsif = true;
            userParams.param1 = channelSettings.channelNumber + NEXUS_PLATFORM_45208_MTSIF_OFFSET;
            userParams.pParam2 = 0;
            BDBG_MSG(("%sfe: %d(%d):%p: (%s,%i)",NEXUS_PLATFORM_45208_FRONTEND_STRING,j,j+i,pConfig->frontend[j+i],userParams.isMtsif ? "mtsif" : "not mtsif",userParams.param1));
            NEXUS_Frontend_SetUserParameters(pConfig->frontend[j+i], &userParams);
        }
    } else {
        BDBG_ERR(("Unable to open %s device",NEXUS_PLATFORM_45208_FRONTEND_STRING));
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
    if(gpioHandleInt)
    {
        NEXUS_Gpio_Close(gpioHandleInt);
        gpioHandleInt = NULL;
    }
    return;
}
#elif defined NEXUS_USE_7250_CWF
#include "nexus_frontend_3128.h"
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
#ifdef NEXUS_PLATFORM_97250_3128_USES_SPI
    NEXUS_SpiSettings spiSettings;
    static NEXUS_SpiHandle g_3128spi;
#endif

    NEXUS_Frontend_GetDefault3128Settings(&st3128Settings);

    NEXUS_FrontendDevice_GetDefault3128OpenSettings(&st3128DeviceOpenSettings);

#if NEXUS_PLATFORM_97250_3128_USES_SPI
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
#else
    st3128DeviceOpenSettings.i2cDevice = pConfig->i2c[3];    /* Onboard tuner/demod use BSC_M3.*/
    st3128DeviceOpenSettings.i2cAddr = 0x6d;
    st3128DeviceOpenSettings.spiDevice = NULL;
#endif

#if 1
    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eAonStandard, &gpioSettings);
    gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
    gpioSettings.value = NEXUS_GpioValue_eHigh;
    gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
    gpioSettings.interrupt.callback = NULL;
    gpioHandle = NEXUS_Gpio_Open(NEXUS_GpioType_eAonStandard, 10, &gpioSettings);
    BDBG_ASSERT(NULL != gpioHandle);
    BKNI_Sleep(1000);
#endif

#if 0
    st3128DeviceOpenSettings.interruptMode = NEXUS_FrontendInterruptMode_ePolling;
    st3128DeviceOpenSettings.gpioInterrupt = 0;
#else

    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
    gpioSettings.mode = NEXUS_GpioMode_eInput;
    gpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;
    tunerGpio = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 90, &gpioSettings);
    if (NULL == tunerGpio)
    {
        BDBG_ERR(("Unable to open GPIO for tuner %d", 0));
    }
    st3128DeviceOpenSettings.interruptMode = NEXUS_FrontendInterruptMode_eInterrupt;
    st3128DeviceOpenSettings.gpioInterrupt = tunerGpio;
#endif
    st3128DeviceOpenSettings.isrNumber = 0;
    st3128DeviceOpenSettings.outOfBand.ifFrequency = 0;
    st3128DeviceOpenSettings.inBandOpenDrain=true;
    st3128DeviceOpenSettings.loadAP = true;
    st3128DeviceOpenSettings.configureWatchdog = false;
    st3128DeviceOpenSettings.isMtsif = true;

    st3128DeviceOpenSettings.externalFixedGain.total = 800;     /* These are platform specific values given by the board designer. */
    st3128DeviceOpenSettings.externalFixedGain.bypassable = 1400; /* These are platform specific values given by the board designer. */

    NEXUS_Frontend_Probe3128(&st3128DeviceOpenSettings, &results);
    BDBG_WRN(("chip.familyId = 0x%x", results.chip.familyId));
    BDBG_WRN(("chip.id = 0x%x", results.chip.id));
    BDBG_WRN(("version.major = 0x%x", results.chip.version.major ));
    BDBG_WRN(("version.minor = 0x%x", results.chip.version.minor ));

    st3128Settings.device = NEXUS_FrontendDevice_Open3128(0, &st3128DeviceOpenSettings);

    if (NULL == st3128Settings.device) {
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
        for (i=0; i <channelCount; i++)
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

    return;
}
#elif defined NEXUS_USE_7250_CD2
NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    /*TODO*/
    return BERR_SUCCESS;
}

void NEXUS_Platform_UninitFrontend(void)
{
    /*TODO*/
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

    *pInputBand = NEXUS_InputBand_e3;

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

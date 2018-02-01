/***************************************************************************
*  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
* Module Description:
*
* API Description:
*   API name: Platform linuxuser
*    linuxuser OS routines
*
*
***************************************************************************/

#include "nexus_platform_module.h"
#include "nexus_platform_priv.h"
#include "priv/nexus_core.h"
#include "nexus_platform_features.h"
#include "nexus_base.h"
#include "nexus_input_band.h"

#if NEXUS_HAS_SPI
#include "nexus_spi.h"
#endif


#if NEXUS_FRONTEND_3466
#include "nexus_frontend_3466.h"
#endif
BDBG_MODULE(nexus_platform_frontend);

static NEXUS_GpioHandle gpioHandleInt = NULL;

#if NEXUS_HAS_SPI
static NEXUS_SpiHandle g_dc_spi[NEXUS_NUM_SPI_CHANNELS] = {NULL};
#endif

#define SV_BOARD_ID  1
#define SV_INT_GPIO 39
#define DV_BOARD_ID  2
#define DV_INT_GPIO 37
#define HB_BOARD_ID  6
#define HB_INT_GPIO 24
#define HB_INT_GPIO_TYPE NEXUS_GpioType_eAonStandard
#define RMT_BOARD_ID_MINOR 6
#define LD_BOARD_ID 12
#define LD3_BOARD_ID_MINOR 2 /* 12.2 */
#define LD3_INT_GPIO 25
#define LD3_INT_GPIO_TYPE NEXUS_GpioType_eAonStandard
#define LD4_BOARD_ID_MINOR 3 /* 12.3 */
#define LD4_INT_GPIO 25
#define LD4_INT_GPIO_TYPE NEXUS_GpioType_eAonStandard

#if NEXUS_HAS_FRONTEND
#include "priv/nexus_frontend_standby_priv.h"
/* Example function to power down or power up SPI or I2C pads on the backend when transitioning to standby. */
NEXUS_Error NEXUS_Platform_FrontendStandby(NEXUS_FrontendDeviceHandle handle, void *context, const NEXUS_FrontendStandbySettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ERR(("NEXUS_Platform_FrontendStandby"));

    BSTD_UNUSED(handle);
    BSTD_UNUSED(context);
    BSTD_UNUSED(pSettings);

    /* Platform specific SPI and GPIO code goes here */

    return rc;
}

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    NEXUS_FrontendDeviceHandle device;
    NEXUS_FrontendUserParameters userParams;
    unsigned i=0;
    NEXUS_FrontendDeviceOpenSettings deviceSettings;
    NEXUS_GpioSettings gpioSettings;
    bool spi = false;
    int spi_chn = 0;
    bool i2c = false;
    unsigned i2c_chn = 3;
    unsigned i2c_addr = 0x68;
    unsigned intGpio = 18;
    NEXUS_GpioType intGpioType = NEXUS_GpioType_eStandard;

    /* board types */
    bool sv  = false;
    bool hb  = false;
    bool dv  = false;
    bool rmt = false;
    bool ld3 = false;
    bool ld4 = false;
    bool ext_raf = false;

    {
        NEXUS_PlatformStatus platformStatus;

        NEXUS_Platform_GetStatus(&platformStatus);
        BDBG_MSG(("board major: %d, minor: %d", platformStatus.boardId.major, platformStatus.boardId.minor));
        if (platformStatus.boardId.major == SV_BOARD_ID) {
            BDBG_MSG(("SV"));
            sv = true;
            spi = true;
            spi_chn = 1;
            intGpio = SV_INT_GPIO;
        } else if (platformStatus.boardId.major == DV_BOARD_ID) {
            BDBG_MSG(("DV"));
            dv = true;
            spi = true;
            intGpio = DV_INT_GPIO;
        } else if (platformStatus.boardId.major == HB_BOARD_ID) {
            if (platformStatus.boardId.minor == RMT_BOARD_ID_MINOR) {
                BDBG_MSG(("RMT"));
                rmt = true;
                ext_raf = true;
            } else {
                hb = true;
                BDBG_MSG(("HB"));
            }
            spi = true;
            intGpio = HB_INT_GPIO;
            intGpioType = HB_INT_GPIO_TYPE;
        } else if (platformStatus.boardId.major == LD_BOARD_ID) {
            if (platformStatus.boardId.minor == LD3_BOARD_ID_MINOR) {
                BDBG_MSG(("2LD3"));
                ld3 = true;
                ext_raf = true;
            }
            if (platformStatus.boardId.minor == LD4_BOARD_ID_MINOR) {
                BDBG_MSG(("2LD4"));
                ld4 = true;
                intGpio = LD4_INT_GPIO;
                intGpioType = LD4_INT_GPIO_TYPE;
                i2c = true;
                i2c_addr = 0x3e;
                i2c_chn = 2;
            }
        } else {
            BDBG_WRN(("Unrecognized BID(%d, %d), not configuring any frontends", platformStatus.boardId.major, platformStatus.boardId.minor));
            return NEXUS_SUCCESS;
        }
    }

    if (!(sv || dv || hb || rmt || ld3 || ld4 || ext_raf))
        return NEXUS_SUCCESS; /* Exit early, no frontends expected */

#if NEXUS_HAS_SPI
    if (spi) {
        NEXUS_SpiSettings spiSettings;
        NEXUS_Spi_GetDefaultSettings(&spiSettings);
        spiSettings.clockActiveLow = true;
        g_dc_spi[0] = NEXUS_Spi_Open(0, &spiSettings);
        if (!g_dc_spi[0]) {
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
        g_dc_spi[1] = NEXUS_Spi_Open(1, &spiSettings);
        if (!g_dc_spi[1]) {
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
    }
#endif

    NEXUS_FrontendDevice_GetDefaultOpenSettings(&deviceSettings);

    if (i2c) {
        deviceSettings.i2cDevice = pConfig->i2c[i2c_chn];
        deviceSettings.i2cAddress = i2c_addr;
    }
#if NEXUS_HAS_SPI
else if (spi) {
        deviceSettings.spiDevice = g_dc_spi[spi_chn];
    }
#endif

#if 0
    deviceSettings.satellite.diseqc.i2cAddress= 0x0B;
    deviceSettings.satellite.diseqc.i2cDevice = pConfig->i2c[NEXUS_PLATFORM_SAT_I2C];
#endif

#if 0
    /* TODO:
     * Some variation on this block may be required for satellite chips, which read rather than output i2c vs. spi.
     */
    if (dv || sv || hb) {
        NEXUS_GpioHandle gpio;
        NEXUS_GpioStatus gpioStatus;
        BREG_Handle hReg = g_pCoreHandles->reg;
        uint32_t reg;

        NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eAonStandard, &gpioSettings);
        gpioSettings.mode = NEXUS_GpioMode_eInput;
        gpio = NEXUS_Gpio_Open(NEXUS_GpioType_eAonStandard, 0, &gpioSettings);
        if (gpio) {
            NEXUS_Gpio_GetStatus(gpio, &gpioStatus);
            BDBG_MSG(("AON GPIO val: %d", gpioStatus.value));
            NEXUS_Gpio_Close(gpio);

            BDBG_MSG(("setting AON GPIO high"));
            NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eAonStandard, &gpioSettings);
            gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
            gpioSettings.value = NEXUS_GpioValue_eHigh;
            gpio = NEXUS_Gpio_Open(NEXUS_GpioType_eAonStandard, 0, &gpioSettings);
            NEXUS_Gpio_Close(gpio);

            NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eAonStandard, &gpioSettings);
            gpioSettings.mode = NEXUS_GpioMode_eInput;
            gpio = NEXUS_Gpio_Open(NEXUS_GpioType_eAonStandard, 0, &gpioSettings);
            NEXUS_Gpio_GetStatus(gpio, &gpioStatus);
            BDBG_MSG(("AON GPIO val: %d", gpioStatus.value));
            NEXUS_Gpio_Close(gpio);
        }
    }
#endif

    {
        NEXUS_FrontendProbeResults probeResults;

        BSTD_UNUSED(userParams);

        NEXUS_FrontendDevice_Probe(&deviceSettings, &probeResults);
        if (probeResults.chip.familyId != 0) {
            BDBG_WRN(("Opening %x...",probeResults.chip.familyId));

            BDBG_MSG(("Setting up interrupt on GPIO %d",intGpio));
            NEXUS_Gpio_GetDefaultSettings(intGpioType, &gpioSettings);
            gpioSettings.mode = NEXUS_GpioMode_eInput;
            gpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;
            gpioHandleInt = NEXUS_Gpio_Open(intGpioType, intGpio, &gpioSettings);
            BDBG_ASSERT(NULL != gpioHandleInt);

            deviceSettings.gpioInterrupt = gpioHandleInt;
            device = NEXUS_FrontendDevice_Open(0, &deviceSettings);

            if (device) {
                NEXUS_FrontendDeviceCapabilities capabilities;

#if NEXUS_FRONTEND_3466
                if ((probeResults.chip.familyId == 0x3466) || (probeResults.chip.familyId == 0x3465)) {
                    NEXUS_FrontendDeviceSettings genericDeviceSettings;
                    NEXUS_FrontendDevice_GetDefaultSettings(&genericDeviceSettings);
                    genericDeviceSettings.rfDaisyChain = NEXUS_RfDaisyChain_eInternalLna;
                    genericDeviceSettings.rfInput = NEXUS_TunerRfInput_eInternalLna;
                    genericDeviceSettings.enableRfLoopThrough = false;
                    NEXUS_FrontendDevice_SetSettings(device, &genericDeviceSettings);
                }
#endif

                NEXUS_FrontendDevice_GetCapabilities(device, &capabilities);
                for (i=0; i < capabilities.numTuners ; i++)
                {
                    NEXUS_FrontendChannelSettings channelSettings;
                    channelSettings.device = device;
                    channelSettings.channelNumber = i;

                    pConfig->frontend[i] = NEXUS_Frontend_Open(&channelSettings);
                    if ( NULL == (pConfig->frontend[i]) )
                    {
                        BDBG_ERR(("Unable to open %x demod %d (as frontend[%d])",probeResults.chip.familyId,i,i));
                        continue;
                    }
                    BDBG_MSG(("%xfe: %d(%d):%p",probeResults.chip.familyId,i,i,(void *)pConfig->frontend[i]));
                }

                if (probeResults.chip.familyId == 0x3158) {
                    NEXUS_FrontendStandbyCallback callback;
                    callback.platformStandby = NEXUS_Platform_FrontendStandby;
                    /* Set platform specific context data */
                    NEXUS_Frontend_SetStandbyCallback_priv(device, &callback);
                }
            } else {
                BDBG_ERR(("Unable to open detected %x frontend", probeResults.chip.familyId));
            }
        } else {
            BDBG_WRN(("No frontend found."));
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
}

#else /* NEXUS_HAS_FRONTEND */
NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    return NEXUS_SUCCESS;
}

void NEXUS_Platform_UninitFrontend(void)
{
}

#endif /* NEXUS_HAS_FRONTEND */

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

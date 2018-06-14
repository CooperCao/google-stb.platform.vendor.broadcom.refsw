/***************************************************************************
*  Copyright (C) 2018 Broadcom.
*  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to
*  the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied),
*  right to use, or waiver of any kind with respect to the Software, and
*  Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
*  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
*  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization,
*  constitutes the valuable trade secrets of Broadcom, and you shall use all
*  reasonable efforts to protect the confidentiality thereof, and to use this
*  information only in connection with your use of Broadcom integrated circuit
*  products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
*  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
*  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
*  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
*  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
*  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
*  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
*  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
*  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
*  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
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

#include "bchp_sun_top_ctrl.h"
#include "bchp_gio.h"

#if NEXUS_HAS_SPI
#include "nexus_spi.h"
#endif

BDBG_MODULE(nexus_platform_frontend);

#if NEXUS_HAS_FRONTEND
static NEXUS_GpioHandle gpioIrqHandle = NULL;

#if NEXUS_HAS_SPI
static NEXUS_SpiHandle spiHandle[NEXUS_NUM_SPI_CHANNELS] = {NULL};
#endif

typedef struct boardFrontendConfigOptions {
    unsigned boardIdMajor;
    unsigned boardIdMinor;
    bool femtsif;
    bool onboardQam;
    bool spi;
    unsigned irqGpio;
    NEXUS_GpioType irqGpioType;
} boardFrontendConfigOptions;

static boardFrontendConfigOptions boardFrontendConfig[] = {
    /* BID   FEM    QAM    SPI    IRQ settings */
    {  1, 1, false, false, true,  25, NEXUS_GpioType_eAonStandard }, /* 73574A0 on board */
    {  1, 0, true,  false, true,  37, NEXUS_GpioType_eStandard },   /* SV Slot 0 */
    {  2, 0, true,  false, true,  37, NEXUS_GpioType_eStandard },   /* DV */
    {  6, 2, true,  false, true,  24, NEXUS_GpioType_eAonStandard}, /* HB */
    {  6, 3, false, true,  true,   0, NEXUS_GpioType_eStandard},    /* RMT */
    { 12, 2, false, true,  true,   0, NEXUS_GpioType_eStandard},    /* 2LD3 */
    { 12, 3, true,  false, false, 25, NEXUS_GpioType_eAonStandard}, /* 2LD4 */
};

#include "priv/nexus_frontend_standby_priv.h"
/* Function to power down or power up SPI or I2C pads on the backend when transitioning to standby. */
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
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    NEXUS_PlatformStatus *platformStatus;
    NEXUS_FrontendDeviceHandle device;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_FrontendDeviceOpenSettings deviceSettings;
    NEXUS_GpioSettings gpioSettings;
    boardFrontendConfigOptions* boardConfig = NULL;
    unsigned i = 0;
    unsigned i2c_chn = 3;
    unsigned i2c_addr = 0x7c;

    platformStatus = BKNI_Malloc(sizeof(*platformStatus));
    if (!platformStatus) {
        return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
    }
    NEXUS_Platform_GetStatus(platformStatus);
    BDBG_MSG(("board major: %d, minor: %d", platformStatus->boardId.major, platformStatus->boardId.minor));

    for ( i= 0 ; i < (sizeof(boardFrontendConfig) / sizeof(boardFrontendConfigOptions)) ; ++i) {
        if ((boardFrontendConfig[i].boardIdMajor == platformStatus->boardId.major) &&
            (boardFrontendConfig[i].boardIdMinor == platformStatus->boardId.minor)) {
            BDBG_WRN(("Selected Board ID %d.%d - config %d",boardFrontendConfig[i].boardIdMajor,boardFrontendConfig[i].boardIdMinor,i));
            boardConfig = &boardFrontendConfig[i];
            break;
        }
    }

    if (boardConfig == NULL) {
        BDBG_MSG(("BID(%d, %d) does not have a frontend config - not adding any frontends", platformStatus->boardId.major, platformStatus->boardId.minor));
        goto err;
    }

    #if NEXUS_HAS_SPI
    if (boardConfig->spi) {
        NEXUS_SpiSettings spiSettings;
        NEXUS_Spi_GetDefaultSettings(&spiSettings);
        spiSettings.clockActiveLow = true;
        spiHandle[0] = NEXUS_Spi_Open(0, &spiSettings);
        if (!spiHandle[0]) {
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
            goto err;
        }
    }
#endif
    NEXUS_FrontendDevice_GetDefaultOpenSettings(&deviceSettings);

#if NEXUS_HAS_SPI
    if (boardConfig->spi) {
        deviceSettings.spiDevice = spiHandle[0];
    } else
#endif
    {
        deviceSettings.i2cDevice = pConfig->i2c[i2c_chn];
        deviceSettings.i2cAddress = i2c_addr;
    }

    if (boardConfig->femtsif) {

        /*
         * This is required for satellite chips on some boards to configure the comms to be SPI not I2C.
         * HB board has this pin configured differently so not required HB BID is 6.2.
         */

        if (platformStatus->boardId.major != 6)
        {
            NEXUS_GpioHandle gpio;
            NEXUS_GpioStatus gpioStatus;

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

        {
            NEXUS_FrontendProbeResults probeResults;

            BSTD_UNUSED(userParams);

            NEXUS_FrontendDevice_Probe(&deviceSettings, &probeResults);
            if (probeResults.chip.familyId != 0) {
                BDBG_WRN(("Opening %x...",probeResults.chip.familyId));

                BDBG_MSG(("Setting up interrupt on GPIO %d",boardConfig->irqGpio));
                NEXUS_Gpio_GetDefaultSettings(boardConfig->irqGpioType, &gpioSettings);
                gpioSettings.mode = NEXUS_GpioMode_eInput;
                gpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;
                gpioIrqHandle = NEXUS_Gpio_Open(boardConfig->irqGpioType, boardConfig->irqGpio, &gpioSettings);
                BDBG_ASSERT(NULL != gpioIrqHandle);

                deviceSettings.gpioInterrupt = gpioIrqHandle;
                device = NEXUS_FrontendDevice_Open(0, &deviceSettings);

                if (device) {
                    NEXUS_FrontendDeviceCapabilities capabilities;

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
                BDBG_WRN(("No frontend found in FEMTSIF slot."));
            }
        }
    }

    if (boardConfig->onboardQam) {
        NEXUS_FrontendProbeResults probeResults;

        BDBG_MSG(("Search for internal LEAP"));
        NEXUS_FrontendDevice_GetDefaultOpenSettings(&deviceSettings);
        NEXUS_FrontendDevice_Probe(&deviceSettings, &probeResults);
        deviceSettings.tuner.i2c.device = pConfig->i2c[i2c_chn];
        deviceSettings.tuner.i2c.address = i2c_addr;
        {
            NEXUS_GpioHandle gpio;

            BDBG_MSG(("setting GPIO 37 high"));
            NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
            gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
            gpioSettings.value = NEXUS_GpioValue_eHigh;
            gpio = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 37, &gpioSettings);
            if (gpio)
                NEXUS_Gpio_Close(gpio);
        }

        if (probeResults.chip.familyId != 0) {
            device = NEXUS_FrontendDevice_Open(0, &deviceSettings);
            if (device) {
                unsigned j;
                NEXUS_FrontendDeviceCapabilities capabilities;
                NEXUS_FrontendDevice_GetCapabilities(device, &capabilities);
                for (j=0; j < capabilities.numTuners ; j++)
                {
                    NEXUS_FrontendChannelSettings channelSettings;
                    channelSettings.device = device;
                    channelSettings.channelNumber = j;

                    pConfig->frontend[i+j] = NEXUS_Frontend_Open(&channelSettings);
                    if ( NULL == (pConfig->frontend[i+j]) )
                    {
                        BDBG_ERR(("Unable to open %x demod %d (as frontend[%d])",probeResults.chip.familyId,j,i+j));
                        continue;
                    }
                    BDBG_MSG(("%xfe: %d(%d):%p",probeResults.chip.familyId,j,i+j,(void *)pConfig->frontend[i+j]));
                }
            }
        }

    }
    /* This means it is built-in tuner - to be handled by FE expert sample code for now */
    if (!boardConfig->onboardQam && !boardConfig->femtsif ) {
        /* must be a 7357X type with built-in tuner  use product ID or board ID */
        if (platformStatus->boardId.major == 1 && platformStatus->boardId.minor == 1) {
            NEXUS_FrontendProbeResults probeResults;
            BSTD_UNUSED(userParams);
            NEXUS_FrontendDevice_Probe(&deviceSettings, &probeResults);
            if (probeResults.chip.familyId != 0) {
                BDBG_WRN(("Opening %x...",probeResults.chip.familyId));
                BDBG_MSG(("Setting up interrupt on GPIO %d",boardConfig->irqGpio));
                NEXUS_Gpio_GetDefaultSettings(boardConfig->irqGpioType, &gpioSettings);
                gpioSettings.mode = NEXUS_GpioMode_eInput;
                gpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;
                gpioIrqHandle = NEXUS_Gpio_Open(boardConfig->irqGpioType, boardConfig->irqGpio, &gpioSettings);
                BDBG_ASSERT(NULL != gpioIrqHandle);

                deviceSettings.gpioInterrupt = gpioIrqHandle;
                device = NEXUS_FrontendDevice_Open(0, &deviceSettings);

                if (device) {
                    NEXUS_FrontendDeviceCapabilities capabilities;

                    NEXUS_FrontendDevice_GetCapabilities(device, &capabilities);
                    for (i=0; i < capabilities.numTuners ; i++) {
                        NEXUS_FrontendChannelSettings channelSettings;
                        channelSettings.device = device;
                        channelSettings.channelNumber = i;

                        pConfig->frontend[i] = NEXUS_Frontend_Open(&channelSettings);
                        if ( NULL == (pConfig->frontend[i]) ) {
                            BDBG_ERR(("Unable to open %x demod %d (as frontend[%d])",probeResults.chip.familyId,i,i));
                            continue;
                        }
                        BDBG_MSG(("%xfe: %d(%d):%p",probeResults.chip.familyId,i,i,(void *)pConfig->frontend[i]));
                    }
                }
            }
        }
    }
err:
    BKNI_Free(platformStatus);
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
    if(gpioIrqHandle)
    {
        NEXUS_Gpio_Close(gpioIrqHandle);
        gpioIrqHandle = NULL;
    }
#if NEXUS_HAS_SPI
    for (i=0; i < NEXUS_NUM_SPI_CHANNELS; i++) {
        if (spiHandle[i]) {
            NEXUS_Spi_Close(spiHandle[i]);
            spiHandle[i] = NULL;
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

NEXUS_Error
NEXUS_Platform_GetStreamerInputBand(unsigned index, NEXUS_InputBand *pInputBand)
{
    BDBG_ASSERT(pInputBand);
    if (index > 0) {
        BDBG_ERR(("Only 1 streamer input available"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    *pInputBand = NEXUS_InputBand_e0;

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

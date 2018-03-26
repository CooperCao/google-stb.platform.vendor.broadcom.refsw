/***************************************************************************
*  Copyright (C) 2016-2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
***************************************************************************/
#include "nexus_platform_module.h"
#include "nexus_platform_priv.h"
#include "priv/nexus_core.h"
#include "nexus_platform_features.h"
#include "nexus_base.h"
#if NEXUS_HAS_TRANSPORT
#include "nexus_input_band.h"
#endif
#if NEXUS_HAS_SPI
#include "nexus_spi.h"
#endif

#include "bchp_gio.h"
#include "bchp_aon_ctrl.h"
#include "bchp_aon_pin_ctrl.h"
#include "bchp_sun_top_ctrl.h"

#if NEXUS_FRONTEND_3461
#include "nexus_frontend_3461.h"
#endif

#if NEXUS_FRONTEND_3466
#include "nexus_frontend_3466.h"
#endif
BDBG_MODULE(nexus_platform_frontend);

/*
 * Special note for SV boards. If the frontend will be plugged into the second
 * FEMTSIF connector (FEMTSIF1), set:
 *     NEXUS_CFLAGS=-DNEXUS_FRONTEND_USE_SECOND_FEMTSIF
 * This will switch from FEMTSIF0 to FEMTSIF1 for the frontend.
 */

#if NEXUS_HAS_FRONTEND
static NEXUS_GpioHandle gpioHandleInt = NULL;
#if NEXUS_HAS_SPI
static NEXUS_SpiHandle g_dc_spi[NEXUS_NUM_SPI_CHANNELS] = {NULL};
#endif
#endif

#define SV_BOARD_ID 1
#define HB_BOARD_ID 2
#define SV_I2C 3
#define HB_I2C 2
#define SV_INT_GPIO0 68
#define SV_INT_GPIO1 68 /*Can this be right?!*/
#define HB_INT_GPIO 12
#define HB_INT_GPIO_TYPE NEXUS_GpioType_eAonStandard
#define SPI_PIN_MASK 0x33300000

static void ConfigureIntGpioPin(int intGpio);

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
    unsigned i2c = SV_I2C;
    unsigned i2c_addr = 0x68;
    unsigned intGpio = SV_INT_GPIO0;
    NEXUS_GpioType intGpioType = NEXUS_GpioType_eStandard;

    /* board types */
    bool sv = false;
    bool hb = false;
    bool pktOverride = false;

#if 1
    {
        const char *c = NEXUS_GetEnv("pkt_override");
        if (c) {
            pktOverride = true;
        }
    }
#endif

    {
        NEXUS_PlatformStatus platformStatus;

        NEXUS_Platform_GetStatus(&platformStatus);
        BDBG_MSG(("board major: %d, minor: %d", platformStatus.boardId.major, platformStatus.boardId.minor));
        if (platformStatus.boardId.major == SV_BOARD_ID) {
            BDBG_MSG(("SV"));
            i2c = SV_I2C;
#if NEXUS_FRONTEND_USE_SECOND_FEMTSIF
            intGpio = SV_INT_GPIO1;
#else
            intGpio = SV_INT_GPIO0;
#endif
            sv = true;
        } else if (platformStatus.boardId.major == HB_BOARD_ID) {
            BDBG_MSG(("HB"));
            i2c = HB_I2C;
            hb = true;
            intGpio = HB_INT_GPIO;
            intGpioType = HB_INT_GPIO_TYPE;
        } else {
            BDBG_WRN(("Unrecognized BID(%d, %d), not configuring any frontends", platformStatus.boardId.major, platformStatus.boardId.minor));
            return NEXUS_SUCCESS;
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
        g_dc_spi[2] = NEXUS_Spi_Open(2, &spiSettings);
        if (!g_dc_spi[2]) {
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
        g_dc_spi[3] = NEXUS_Spi_Open(3, &spiSettings);
        if (!g_dc_spi[3]) {
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
    }
#endif

    NEXUS_FrontendDevice_GetDefaultOpenSettings(&deviceSettings);

#if NEXUS_HAS_SPI
#if NEXUS_FRONTEND_USE_SECOND_FEMTSIF
    deviceSettings.spiDevice = g_dc_spi[2];
#else
    if (sv) {
        deviceSettings.spiDevice = g_dc_spi[3];
    }
    else {
        deviceSettings.spiDevice = g_dc_spi[0];
    }
#endif
    BSTD_UNUSED(i2c_addr);
    BSTD_UNUSED(i2c);
#else
    deviceSettings.i2cDevice = pConfig->i2c[i2c];
    deviceSettings.i2cAddress = i2c_addr;
#endif

    ConfigureIntGpioPin(intGpio);

    if (sv || hb) {
        NEXUS_GpioHandle gpio;
        NEXUS_GpioStatus gpioStatus;
        BREG_Handle hReg = g_pCoreHandles->reg;
        uint32_t reg;

        if (sv) {
            reg = BREG_Read32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1);
            BDBG_MSG(("AON_PIN_CTRL_PIN_MUX_CTRL_1: %08x",reg));
            if ((reg & 0xFFF00000) != SPI_PIN_MASK) {
                BDBG_MSG(("Reprogramming SPI registers for frontend communication..."));

                reg &= ~(
                         BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_13) |
                         BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_14) |
                         BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_15)
                         );

                reg |= (
                        BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_13, 3) |  /* SPI_M_SCK */
                        BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_14, 3) |  /* SPI_M_MOSI */
                        BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_15, 3)    /* SPI_M_MISO */
                        );
                BREG_Write32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, reg);
                BDBG_MSG(("AON_PIN_CTRL_PIN_MUX_CTRL_1: %08x",reg));
            }
            reg = BREG_Read32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0);
            BDBG_MSG(("AON_PIN_CTRL_PIN_MUX_CTRL_0: %08x",reg));
#if NEXUS_FRONTEND_USE_SECOND_FEMTSIF
            reg &= ~(BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_06));
            reg |= (BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_06, 3));    /* SPI_M_SS2B */
#else
            reg &= ~(BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_07));
            reg |= (BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_07, 3));    /* SPI_M_SS3B */
#endif
            BREG_Write32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0, reg);
            BDBG_MSG(("AON_PIN_CTRL_PIN_MUX_CTRL_0: %08x",reg));
            reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
            BDBG_MSG(("SUN_TOP_CTRL_PIN_MUX_CTRL_10: %08x",reg));
            reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_067));
            reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_067, 0));
            BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10, reg);
            BDBG_MSG(("SUN_TOP_CTRL_PIN_MUX_CTRL_10: %08x",reg));

            BDBG_MSG(("Querying GP067_SPI_BSCn"));
            NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
            gpioSettings.mode = NEXUS_GpioMode_eInput;
            gpio = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 67, &gpioSettings);
            if (gpio) {
                NEXUS_Gpio_GetStatus(gpio, &gpioStatus);
                BDBG_MSG(("GPIO val: %d", gpioStatus.value));
                NEXUS_Gpio_Close(gpio);

                BDBG_MSG(("setting GPIO high"));
                NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
                gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
                gpioSettings.value = NEXUS_GpioValue_eHigh;
                gpio = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 67, &gpioSettings);
                NEXUS_Gpio_Close(gpio);

                NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
                gpioSettings.mode = NEXUS_GpioMode_eInput;
                gpio = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 67, &gpioSettings);
                NEXUS_Gpio_GetStatus(gpio, &gpioStatus);
                BDBG_MSG(("GPIO val: %d", gpioStatus.value));
                NEXUS_Gpio_Close(gpio);
            }
        }
        else if (hb) {
            reg = BREG_Read32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2);
            BDBG_MSG(("AON_PIN_CTRL_PIN_MUX_CTRL_2: %08x",reg));
            reg &= ~(BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_16));
            reg |= (BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_16, 3));    /* SPI_M_SS0B */
            BREG_Write32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2, reg);
            BDBG_MSG(("AON_PIN_CTRL_PIN_MUX_CTRL_2: %08x",reg));
            reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4);
            BDBG_MSG(("SUN_TOP_CTRL_PIN_MUX_CTRL_10: %08x",reg));
            reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_018));
            reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_018, 0));
            BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4, reg);
            BDBG_MSG(("SUN_TOP_CTRL_PIN_MUX_CTRL_4: %08x",reg));

            BDBG_MSG(("Querying GP018_SPI_BSCn"));
            NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
            gpioSettings.mode = NEXUS_GpioMode_eInput;
            gpio = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 18, &gpioSettings);
            if (gpio) {
                NEXUS_Gpio_GetStatus(gpio, &gpioStatus);
                BDBG_MSG(("GPIO val: %d", gpioStatus.value));
                NEXUS_Gpio_Close(gpio);

                BDBG_MSG(("setting GPIO high"));
                NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
                gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
                gpioSettings.value = NEXUS_GpioValue_eHigh;
                gpio = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 18, &gpioSettings);
                NEXUS_Gpio_Close(gpio);

                NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
                gpioSettings.mode = NEXUS_GpioMode_eInput;
                gpio = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 18, &gpioSettings);
                NEXUS_Gpio_GetStatus(gpio, &gpioStatus);
                BDBG_MSG(("GPIO val: %d", gpioStatus.value));
                NEXUS_Gpio_Close(gpio);
            }
        }

    }

    {
        NEXUS_FrontendProbeResults probeResults;

        BSTD_UNUSED(userParams);

        NEXUS_FrontendDevice_Probe(&deviceSettings, &probeResults);
        if (probeResults.chip.familyId != 0) {
            BDBG_WRN(("Opening %x...",probeResults.chip.familyId));
#if 0
            if ((probeResults.chip.familyId == 0x3461) || (probeResults.chip.familyId == 0x3466) || (probeResults.chip.familyId == 0x3465)) {
                deviceSettings.terrestrial.enabled = true;
                deviceSettings.loadAP = true;
            } else if (probeResults.chip.familyId == 0x3158)
                deviceSettings.cable.enabled = true;
            else
                deviceSettings.satellite.enabled = true;
#else
            if ((probeResults.chip.familyId == 0x3461)) {
                deviceSettings.terrestrial.enabled = true;
                deviceSettings.loadAP = true;
            } else if ((probeResults.chip.familyId == 0x3466) || (probeResults.chip.familyId == 0x3465)) {
                deviceSettings.terrestrial.enabled = true;
                deviceSettings.cable.enabled = true;
                deviceSettings.loadAP = true;
            } else if ((probeResults.chip.familyId == 0x3158)) {
                deviceSettings.loadAP = true;
                deviceSettings.cable.enabled = true;
            } else
                deviceSettings.satellite.enabled = true;
#endif

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

#if NEXUS_FRONTEND_3461
                if (probeResults.chip.familyId == 0x3461) {
                    NEXUS_FrontendDevice3461Settings settings;
                    NEXUS_FrontendDevice_GetDefault3461Settings(&settings);
                    settings.rfDaisyChain = NEXUS_3461RfDaisyChain_eInternalLna;
                    settings.rfInput = NEXUS_3461TunerRfInput_eInternalLna;
                    settings.enableRfLoopThrough = false;
                    settings.terrestrial = true;
                    NEXUS_FrontendDevice_Set3461Settings(device, &settings);
                }
#endif

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
                if ((probeResults.chip.familyId == 0x3465) && (capabilities.numTuners > 1))
                    capabilities.numTuners = 1;
                if ((probeResults.chip.familyId == 0x3466) && (capabilities.numTuners > 2))
                    capabilities.numTuners = 2;
                for (i=0; i < capabilities.numTuners ; i++)
                {
                    NEXUS_FrontendChannelSettings channelSettings;
                    channelSettings.device = device;
                    channelSettings.channelNumber = i;
#if 0
                    if ((probeResults.chip.familyId == 0x3461) || (probeResults.chip.familyId == 0x3466) || (probeResults.chip.familyId == 0x3465)) {
                        channelSettings.type = NEXUS_FrontendChannelType_eTerrestrial;
                    } else if (probeResults.chip.familyId == 0x3158)
                        channelSettings.type = NEXUS_FrontendChannelType_eCable;
                    else
                        channelSettings.type = NEXUS_FrontendChannelType_eSatellite;
#else
                    if ((probeResults.chip.familyId == 0x3461)) {
                        channelSettings.type = NEXUS_FrontendChannelType_eTerrestrial;
                    } else if ((probeResults.chip.familyId == 0x3158) || (probeResults.chip.familyId == 0x3466) || (probeResults.chip.familyId == 0x3465))
                        channelSettings.type = NEXUS_FrontendChannelType_eCable;
                    else
                        channelSettings.type = NEXUS_FrontendChannelType_eSatellite;
#endif
                    pConfig->frontend[i] = NEXUS_Frontend_Open(&channelSettings);
                    if ( NULL == (pConfig->frontend[i]) )
                    {
                        BDBG_ERR(("Unable to open %x demod %d (as frontend[%d])",probeResults.chip.familyId,i,i));
                        continue;
                    }
                    BDBG_MSG(("%xfe: %d(%d):%p",probeResults.chip.familyId,i,i,(void *)pConfig->frontend[i]));
                    if (pktOverride) {
                        NEXUS_FrontendUserParameters userParams;
                        NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
                        userParams.isMtsif = false;
                        userParams.param1 = NEXUS_InputBand_e0+i;
                        userParams.pParam2 = 0;
                        NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
                    }
                }

                if (probeResults.chip.familyId == 0x3461 || pktOverride) {
                    BREG_Handle hReg = g_pCoreHandles->reg;
                    uint32_t reg;
                    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);
                    BDBG_MSG(("PIN_CTRL_PIN_MUX_CTRL_12: %08x",reg));
                    reg &= ~(
                             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_087) |
                             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_088)
                             );
                    reg |= (
                            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_087, 2) | /* PKT_CLK1 */
                            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_088, 2)   /* PKT_DATA1 */
                            );
                    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, reg);
                    BDBG_MSG(("PIN_CTRL_PIN_MUX_CTRL_12: %08x",reg));
                    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);
                    BDBG_MSG(("PIN_CTRL_PIN_MUX_CTRL_13: %08x",reg));
                    reg &= ~(
                             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_089) |
                             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_090)
                             );
                    reg |= (
                            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_089, 2) | /* PKT_SYNC1 */
                            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_090, 2)   /* PKT_ERROR1 */
                            );
                    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13, reg);
                    BDBG_MSG(("PIN_CTRL_PIN_MUX_CTRL_13: %08x",reg));
                }
                if (probeResults.chip.familyId == 0x3158) {
                    NEXUS_FrontendStandbyCallback callback;
                    callback.platformStandby = NEXUS_Platform_FrontendStandby;
                    /* Set platform specific context data */
                    NEXUS_Frontend_SetStandbyCallback_priv(device, &callback);
                }
            }
            else {
                BDBG_ERR(("Unable to open detected %x frontend", probeResults.chip.familyId));
            }
        }
        else {
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

static void ConfigureIntGpioPin(int intGpio)
{
    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;

    switch (intGpio)
    {
        case 12:
        {
            reg = BREG_Read32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1);
            BDBG_MSG(("BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0: %08x",reg));
            reg &= ~(
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_12)
                );
            reg |= (
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_12, 0) /* AON_GPIO_12 */
                );
            BREG_Write32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, reg);
            break;
        }

        case 68:
        {
            reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
            BDBG_MSG(("BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10: %08x",reg));
            reg &= ~(
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_068)
                 );
            reg |= (
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_068, 0) /* GPIO_068 */
                );
            BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10, reg);
            break;
        }
    }
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

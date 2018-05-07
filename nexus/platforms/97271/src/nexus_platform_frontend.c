/***************************************************************************
*  Copyright (C) 2018 Broadcom.
*  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Special note for 7271/7268 SV boards. If the frontend will be plugged into the second
 * FEMTSIF connector (FEMTSIF1), set:
 *     NEXUS_CFLAGS=-DNEXUS_FRONTEND_USE_SECOND_FEMTSIF
 * This will switch from FEMTSIF0 to FEMTSIF1 for the frontend.
 */

static NEXUS_GpioHandle gpioHandleInt = NULL;

#if NEXUS_HAS_SPI
static NEXUS_SpiHandle g_dc_spi[NEXUS_NUM_SPI_CHANNELS] = {NULL};
#endif

#define USE_I2C 0

#if (BCHP_CHIP == 7260) || (BCHP_CHIP == 7255)
    #define SV_BOARD_ID 1
    #define DV_BOARD_ID 2
    #define HB_BOARD_ID 6
    #define HB_INT_GPIO 24
    #define HB_INT_GPIO_TYPE NEXUS_GpioType_eAonStandard
    #define SPI_SCK_MOSI_ALT 3
    #define SPI_PIN_MASK 0x04433000
#elif (BCHP_CHIP == 7268)
    #define SV_BOARD_ID 1
    #define DV_BOARD_ID 3
    #define HB_BOARD_ID 4
    #define HB_INT_GPIO 7
    #define HB_INT_GPIO_TYPE NEXUS_GpioType_eAonStandard
    #define SPI_SCK_MOSI_ALT 4
    #define SPI_PIN_MASK 0x04444000
#elif (BCHP_CHIP == 7271)
    #define SV_BOARD_ID 1
    #define DV_BOARD_ID 3
    #define HB_BOARD_ID 4
    #define HB_INT_GPIO 18
    #define HB_INT_GPIO_TYPE NEXUS_GpioType_eStandard
    #define SPI_SCK_MOSI_ALT 4
    #define SPI_PIN_MASK 0x04444000
#endif

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
#if USE_I2C
    unsigned i2c = 3; /* Default for 7260, 7268 & 7271 boards */
    unsigned i2c_addr = 0x68;
#endif
    unsigned intGpio = 18;
    NEXUS_GpioType intGpioType = NEXUS_GpioType_eStandard;

    /* board types */
    bool sv = false;
    bool hb = false; /* C, D, T, HB have the same frontend layout */
    bool dv = false;
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
#if NEXUS_FRONTEND_USE_SECOND_FEMTSIF
            intGpio = 39;
#else
            intGpio = 37;
#endif
            sv = true;
        } else if (platformStatus.boardId.major == DV_BOARD_ID) {
            BDBG_MSG(("DV"));
            intGpio = 37;
            dv = true;
        } else if (platformStatus.boardId.major == HB_BOARD_ID) {
            BDBG_MSG(("C/D/T/HB"));
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
        g_dc_spi[1] = NEXUS_Spi_Open(1, &spiSettings);
        if (!g_dc_spi[1]) {
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
    }
#endif

    NEXUS_FrontendDevice_GetDefaultOpenSettings(&deviceSettings);

#if USE_I2C
    deviceSettings.i2cDevice = pConfig->i2c[i2c];
    deviceSettings.i2cAddress = i2c_addr;
#else
    deviceSettings.spiDevice = g_dc_spi[0];
#endif

#if 0
    deviceSettings.satellite.diseqc.i2cAddress= 0x0B;
    deviceSettings.satellite.diseqc.i2cDevice = pConfig->i2c[NEXUS_PLATFORM_SAT_I2C];
#endif

    ConfigureIntGpioPin(intGpio);

    if (dv || sv || hb) {
        NEXUS_GpioHandle gpio;
        NEXUS_GpioStatus gpioStatus;
        BREG_Handle hReg = g_pCoreHandles->reg;
        uint32_t reg;

        if (sv) {
            reg = BREG_Read32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3);
            BDBG_MSG(("AON_PIN_CTRL_PIN_MUX_CTRL_3: %08x",reg));
            if ((reg & 0x0FFFF000) != SPI_PIN_MASK) {
                BDBG_MSG(("Reprogramming SPI registers for frontend communication..."));

                reg &= ~(
                         BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_21) |
                         BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_22) |
                         BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_23) |
                         BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_24)
                         );

                reg |= (
                        BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_21, SPI_SCK_MOSI_ALT) |  /* SPI_M_SCK */
                        BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_22, SPI_SCK_MOSI_ALT) |  /* SPI_M_MOSI */
                        BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_23, 4) |  /* SPI_M_SS0B */
                        BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_24, 4)    /* SPI_M_MISO */
                        );
                BREG_Write32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3, reg);
                BDBG_MSG(("AON_PIN_CTRL_PIN_MUX_CTRL_3: %08x",reg));
            }
        }

#if (BCHP_CHIP==7271) /* Only 7271 HB needs to adjust for BCM9TS connector */
        if (hb) {
            reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2);
            BDBG_MSG(("SUN_TOP_CTRL_PIN_MUX_CTRL_2: %08x",reg));
            if ((reg & 0xFFFF0000) != 0x66660000 ) {
                BDBG_MSG(("Reprogramming CTRL_2 SPI registers for frontend communication..."));
                reg &= ~(
                         BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_013) |
                         BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_014) |
                         BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_015) |
                         BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_016)
                         );

                reg |= (
                        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_013, 6) |  /* SPI_M_MISO */
                        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_014, 6) |  /* SPI_M_SCK */
                        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_015, 6) |  /* SPI_M_SS0B */
                        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_016, 6)    /* SPI_M_MOSI */
                        );
                BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2, reg);
                BDBG_MSG(("SUN_TOP_CTRL_PIN_MUX_CTRL_2: %08x",reg));
            }
        }
#endif

        reg = BREG_Read32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0);
        BDBG_MSG(("AON_PIN_CTRL_PIN_MUX_CTRL_0: %08x",reg));
        reg &= ~(BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_00));
        reg |= (BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_00, 0));
        BREG_Write32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0, reg);
        BDBG_MSG(("AON_PIN_CTRL_PIN_MUX_CTRL_0: %08x",reg));

        BDBG_MSG(("Querying AON GPIO 0"));
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
                    bool pktUseAlt2 = false;
#if (BCHP_CHIP == 7268) || (BCHP_CHIP == 7271)
                    /* 7268/71 HB boards use different ALT routing for the PKT interface */
                    if (hb) pktUseAlt2 = true;
#endif
                    /* 3 sync
                     * 2 data 0
                     * 7 data 1
                     * 1 clk
                     */
                    /* 3461 is PKT, not MTSIF, and requires board-specific pinmux changes to route transport data */

                    BDBG_ERR(("Legacy transport!"));

                    if (pktUseAlt2) {
                        BDBG_ERR(("use alt2"));
                        reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1);
                        BDBG_WRN(("PIN_CTRL_PIN_MUX_CTRL_1: %08x",reg));
                        reg &= ~(
                                 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_001) |
                                 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_002) |
                                 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_003) |
                                 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_007)
                                 );
                        reg |= (
                                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_001, 2) | /* PKT_CLK0_ALT2 */
                                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_002, 2) | /* PKT_DATA0_ALT2 */
                                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_003, 2) | /* PKT_SYNC0_ALT2 */
                                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_007, 2)   /* PKT_CLK2_ALT2 */
                                );
                        BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1, reg);
                        BDBG_WRN(("PIN_CTRL_PIN_MUX_CTRL_1: %08x",reg));

                        if (pktOverride) {
                            reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1);
                            BDBG_WRN(("PIN_CTRL_PIN_MUX_CTRL_1: %08x",reg));
                            reg &= ~(
                                     BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_004) |
                                     BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_005) |
                                     BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_006)
                                     );
                            reg |= (
                                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_004, 2) | /* PKT_CLK1_ALT2 */
                                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_005, 2) | /* PKT_DATA1_ALT2 */
                                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_006, 2)   /* PKT_SYNC1_ALT2 */
                                    );
                            BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1, reg);
                            BDBG_WRN(("PIN_CTRL_PIN_MUX_CTRL_1: %08x",reg));
                        }
                    } else { /* 7268/71 DV, SV - 7260 */
                        BDBG_ERR(("NOT use alt2"));
                        reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3);
                        BDBG_MSG(("PIN_CTRL_PIN_MUX_CTRL_3: %08x",reg));
                        reg &= ~(
                                 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_021) |
                                 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_022) |
                                 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_023) |
                                 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_024)
                                 );
                        reg |= (
                                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_021, 2) | /* PKT_CLK0_ALT */
                                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_022, 2) | /* PKT_DATA0_ALT */
                                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_023, 2) | /* PKT_SYNC0_ALT */
                                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_024, 2)   /* PKT_ERROR0_ALT */
                                );
                        BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3, reg);
                        BDBG_MSG(("PIN_CTRL_PIN_MUX_CTRL_3: %08x",reg));

                        reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4);
                        BDBG_MSG(("PIN_CTRL_PIN_MUX_CTRL_4: %08x",reg));
                        reg &= ~(
                                 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_026) |
                                 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_027) |
                                 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_028)
                                 );
                        reg |= (
                                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_026, 2) | /* PKT_CLK1_ALT */
                                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_027, 2) | /* PKT_DATA1_ALT */
                                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_028, 2) /* PKT_SYNC1_ALT2 */
                                );
                        BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4, reg);
                        BDBG_WRN(("PIN_CTRL_PIN_MUX_CTRL_4: %08x",reg));
                    }
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
        }
    }

    if (!pConfig->frontend[0]) {
        BDBG_WRN(("No frontend found."));
#if (BCHP_CHIP==7271) /* Only 7271 HB needs to adjust for BCM9TS connector */
        if (hb) {
            uint32_t reg;
            /* If no frontend, set pinmux for PKT3 to enable BCM9TS_DC streamer input. may require board rework. */
            reg = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2);
            reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_013) |
                     BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_014) |
                     BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_015) );
            reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_013, 5) |
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_014, 5) |
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_015, 5) );
            BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2, reg);
        }
#endif
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
        case 7:
        {
            reg = BREG_Read32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0);
            BDBG_MSG(("BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0: %08x",reg));
            reg &= ~(
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_07)
                );
            reg |= (
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_07, 0) /* AON_GPIO_07 */
                );
            BREG_Write32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0, reg);
            break;
        }

        case 18:
        {
            reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3);
            BDBG_MSG(("BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3: %08x",reg));
            reg &= ~(
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_018)
                );
            reg |= (
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_018, 0) /* GPIO_018 */
                );
            BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3, reg);
            break;
        }

        case 24:
        {
            reg = BREG_Read32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3);
            BDBG_MSG(("BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0: %08x",reg));
            reg &= ~(
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_24)
                );
            reg |= (
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_24, 0) /* AON_GPIO_24 */
                );
            BREG_Write32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3, reg);
            break;
        }

#if !NEXUS_FRONTEND_USE_SECOND_FEMTSIF
        case 37:
        {
            reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5);
            BDBG_MSG(("BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5: %08x",reg));
            reg &= ~(
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_037)
                 );
            reg |= (
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_037, 0) /* GPIO_037 */
                );
            BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5, reg);
            break;
        }
#endif

#if NEXUS_FRONTEND_USE_SECOND_FEMTSIF
        case 39:
        {
            reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5);
            BDBG_MSG(("BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5: %08x",reg));
            reg &= ~(
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_039)
                 );
            reg |= (
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_039, 0) /* GPIO_039 */
                );
            BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5, reg);
            break;
        }
#endif
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
#if (BCHP_CHIP == 7260)
    *pInputBand = NEXUS_InputBand_e0;
#else
    *pInputBand = NEXUS_InputBand_e3;
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

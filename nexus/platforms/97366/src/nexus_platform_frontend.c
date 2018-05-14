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
***************************************************************************/
#include "nexus_platform_module.h"
#include "nexus_platform_priv.h"
#include "priv/nexus_core.h"
#include "nexus_platform_features.h"
#include "nexus_base.h"
#include "bchp_gio.h"

BDBG_MODULE(nexus_platform_frontend);

#if NEXUS_HAS_I2C && NEXUS_HAS_FRONTEND
#include "nexus_frontend_7366.h"

#if NEXUS_FRONTEND_4538 || NEXUS_FRONTEND_45216 || NEXUS_FRONTEND_45308 || NEXUS_FRONTEND_3128
/* Assume daughtercard via MTSIF header if built with additional (supported) frontends */
#define NEXUS_PLATFORM_7366_DAUGHTERCARD 1
#define NEXUS_PLATFORM_7366_DAUGHTERCARD_MTSIF0 1
/*#define NEXUS_PLATFORM_7366_DAUGHTERCARD_MTSIF1 1*/
#endif

#include "bchp_sun_top_ctrl.h"
#if NEXUS_PLATFORM_7366_DAUGHTERCARD
#include "bchp_sca.h"
#include "bchp_gio.h"
#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif
#endif


static NEXUS_GpioHandle gpioHandleInt = NULL;

static void NEXUS_Platform_Frontend_P_Mtsif_Pinmux(void)
{
    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;
    /* Pinmux specific to MTSIF0 */
    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);
    reg &= ~(
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_066) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_067) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_068) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_069) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_071)
             );
    reg |= (
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_066, 1) | /* MTSIF0_RX_CLK */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_067, 1) | /* MTSIF0_RX_DATA0 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_068, 1) | /* MTSIF0_RX_SYNC */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_069, 1) | /* MTSIF0_RX_DATA1 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_071, 1)   /* MTSIF0_RX_DATA2 */
            );
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, reg);

    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);
    reg &= ~(
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_072) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_073) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_074) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_075) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_076) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_077) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_078)
             );
    reg |= (
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_072, 1) | /* MTSIF0_RX_DATA3 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_073, 1) | /* MTSIF0_RX_DATA4 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_074, 1) | /* MTSIF0_RX_DATA5 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_075, 1) | /* MTSIF0_RX_DATA6 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_076, 1) | /* MTSIF0_RX_DATA7 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_077, 1) | /* MTSIF_ATS_INC */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_078, 1)   /* MTSIF_ATS_RST */
            );
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13, reg);
}

#if NEXUS_PLATFORM_7366_DAUGHTERCARD
void NEXUS_Platform_Frontend_P_Daughtercard_Pinmux(bool sv, bool sff)
{
    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;

    /* Pinmux specific to the MTSIF external connector */
    /* Default pinmux sets both SPI and I2C pins. */
    /* This disables SPI in favor of I2C, and resets the pinmuxing to generic use of the GPIOs. */
    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0);
    reg &= ~(
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_135) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_136)
             );
    reg |= (
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_135, 0) | /* GPIO_135 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_136, 0)   /* GPIO_136 */
            );
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0, reg);

    if (sff) {
        reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7);
        reg &= ~(
                 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_024)
                 );
        reg |= (
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_024, 0) /* GPIO_024 */
                );
        BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7, reg);
    }

    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);
    reg &= ~(
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_033)
             );
    reg |= (
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_033, 0) /* GPIO_033 */
            );
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8, reg);

    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);
    reg &= ~(
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_070)
             );
    reg |= (
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_070, 0)   /* GPIO_070 */
            );
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, reg);

    NEXUS_Platform_Frontend_P_Mtsif_Pinmux();

    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16);
    reg &= ~(
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_097)
             );
    reg |= (
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_097, 0) /* GPIO_097 */
            );
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16, reg);

    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_19);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_126) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_127)
             );
    reg |= (
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_126, 0) | /* GPIO_126 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_127, 0)   /* GPIO_127 */
            );
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_19, reg);

    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_20);
    reg &= ~(
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_20, gpio_128) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_20, gpio_129) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_20, gpio_130)
             );
    reg |= (
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_20, gpio_128, 0) | /* GPIO_128 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_20, gpio_129, 0) | /* GPIO_129 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_20, gpio_130, 0)   /* GPIO_130 */
            );
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_20, reg);

    if (sv) {
#ifdef BCHP_PWR_RESOURCE_SMARTCARD0
        BCHP_PWR_AcquireResource(g_pCoreHandles->chp, BCHP_PWR_RESOURCE_SMARTCARD0);
#endif

        /* SV board uses GPIO 136 as interrupt. Need to ensure it can be used as such (shared with SC0). */
        reg = BREG_Read32(hReg, BCHP_SCA_AFE_CMD_2);
        reg &= ~(
                BCHP_MASK(SCA_AFE_CMD_2, bp_modeb) |
                BCHP_MASK(SCA_AFE_CMD_2, power_dn)
                 );
        reg |= (
                BCHP_FIELD_DATA(SCA_AFE_CMD_2, bp_modeb, 0) |
                BCHP_FIELD_DATA(SCA_AFE_CMD_2, power_dn, 0)
                );
        BREG_Write32(hReg, BCHP_SCA_AFE_CMD_2, reg);

        reg = BREG_Read32(hReg, BCHP_GIO_IODIR_EXT3);
        reg &= 0x0000027F;
        BREG_Write32(hReg, BCHP_GIO_IODIR_EXT3, reg);
        reg = BREG_Read32(hReg, BCHP_GIO_DATA_EXT3);
        reg &= 0x0000027F;
        BREG_Write32(hReg, BCHP_GIO_DATA_EXT3, reg);

#ifdef BCHP_PWR_RESOURCE_SMARTCARD0
        BCHP_PWR_ReleaseResource(g_pCoreHandles->chp, BCHP_PWR_RESOURCE_SMARTCARD0);
#endif
    }
}
#endif

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    NEXUS_FrontendDeviceHandle device;
    NEXUS_FrontendDeviceOpenSettings deviceSettings;
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned i=0;
    unsigned dsqI2cChannel = 0;
    unsigned extI2cChannel = 0;
    bool externalSatellite = false;
#if NEXUS_PLATFORM_7366_DAUGHTERCARD
    bool daughtercard = false;
    bool sv = false;
    bool sff = false;
    unsigned dcI2cChannel  = 0;
    unsigned dcInterrupt   = 0;
#endif

    {
        NEXUS_PlatformStatus *platformStatus;

        platformStatus = BKNI_Malloc(sizeof(*platformStatus));
        if (!platformStatus) {
            return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        }
        NEXUS_Platform_GetStatus(platformStatus);
        BDBG_MSG(("board major: %d, minor: %d",platformStatus->boardId.major,platformStatus->boardId.minor));
        if (platformStatus->boardId.major == 5) {
            externalSatellite = true;
            extI2cChannel = 3;
        }
#if NEXUS_PLATFORM_7366_DAUGHTERCARD
        if (platformStatus->boardId.major == 1) { /* SV */
            daughtercard = true;
            sv = true;
            dcInterrupt = 136;
            dcI2cChannel = 3;
        }
        if (platformStatus->boardId.major == 2) { /* SFF */
            daughtercard = true;
            sff = true;
            dcInterrupt = 23;
            dcI2cChannel = 3;
        }
        if (platformStatus->boardId.major == 4) { /* SV DDR4 */
            daughtercard = true;
            sv = true;
            dcInterrupt = 64;
            dcI2cChannel = 3;
        }
        BKNI_Free(platformStatus);
#endif
    }

    NEXUS_FrontendDevice_GetDefaultOpenSettings(&deviceSettings);
    deviceSettings.satellite.enabled = true;
#if NEXUS_USE_7399_SFF || NEXUS_USE_7399_SV
    dsqI2cChannel = 1;
#else
#if (BCHP_VER >= BCHP_VER_B0)
    dsqI2cChannel = 5;
#else
    dsqI2cChannel = 2;
#endif
#endif
    deviceSettings.satellite.diseqc.i2cDevice = pConfig->i2c[dsqI2cChannel];
    deviceSettings.satellite.diseqc.i2cAddress = 0x8;

    device = NEXUS_FrontendDevice_Open(0, &deviceSettings);

    if (device) {
        NEXUS_FrontendDeviceCapabilities capabilities;
        NEXUS_FrontendDevice_GetCapabilities(device, &capabilities);
        for (i=0; i < capabilities.numTuners; i++)
        {
            NEXUS_FrontendChannelSettings channelSettings;

            NEXUS_Frontend_GetDefaultOpenSettings(&channelSettings);

            channelSettings.type = NEXUS_FrontendChannelType_eSatellite;
            channelSettings.device = device;
            channelSettings.channelNumber = i;
            pConfig->frontend[i] = NEXUS_Frontend_Open(&channelSettings);
            if ( NULL == (pConfig->frontend[i]) )
            {
                BDBG_ERR(("Unable to open onboard 7366 demod %d",i));
                continue;
            }
            BDBG_MSG(("7366fe: %d:%p",i,(void *)pConfig->frontend[i]));
        }
    } else {
        BDBG_ERR(("Unable to open 7366 device"));
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    if (externalSatellite) { /* 7366EXT8 */
        NEXUS_FrontendProbeResults probeResults;
        NEXUS_FrontendDeviceOpenSettings deviceSettings;
        NEXUS_FrontendDeviceCapabilities capabilities;
        NEXUS_FrontendChannelSettings channelSettings;

        NEXUS_FrontendDevice_GetDefaultOpenSettings(&deviceSettings);

        deviceSettings.i2cAddress = 0x68;
        deviceSettings.i2cDevice = pConfig->i2c[extI2cChannel];
        deviceSettings.reset.enable = true;
        deviceSettings.reset.type = NEXUS_GpioType_eStandard;
        deviceSettings.reset.pin = 99;
        deviceSettings.reset.value = NEXUS_GpioValue_eHigh;

        rc = NEXUS_FrontendDevice_Probe(&deviceSettings, &probeResults);

        if (probeResults.chip.familyId == 0) {
            deviceSettings.i2cAddress = 0x69;
            rc = NEXUS_FrontendDevice_Probe(&deviceSettings, &probeResults);
        }

        if (probeResults.chip.familyId == 0) {
            deviceSettings.i2cAddress = 0x6a;
            rc = NEXUS_FrontendDevice_Probe(&deviceSettings, &probeResults);
        }

        if (probeResults.chip.familyId != 0) {
            NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
            unsigned tuner_offset;
            NEXUS_GpioSettings gpioSettings;

            NEXUS_Platform_Frontend_P_Mtsif_Pinmux(); /* change pinmux for MTSIF0 out */

            tuner_offset = i;

            /* already have default settings, and i2c device and channel from the probe */

            deviceSettings.satellite.enabled = true;
            BDBG_MSG(("Setting up interrupt on GPIO %d",98));
            NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
            gpioSettings.mode = NEXUS_GpioMode_eInput;
            gpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;
            gpioHandleInt = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 98, &gpioSettings);
            BDBG_ASSERT(NULL != gpioHandleInt);
            deviceSettings.gpioInterrupt = gpioHandleInt;

            device = NEXUS_FrontendDevice_Open(0, &deviceSettings);
            if (!device) {
                BDBG_WRN(("No external sat frontend found."));
            }
            else {
                NEXUS_FrontendDevice_GetCapabilities(device, &capabilities);
                for (i=0; i < capabilities.numTuners; i++)
                {
                    NEXUS_Frontend_GetDefaultOpenSettings(&channelSettings);
                    channelSettings.device = device;
                    channelSettings.channelNumber = i;
                    channelSettings.type = NEXUS_FrontendChannelType_eSatellite;
                    pConfig->frontend[i+tuner_offset] = NEXUS_Frontend_Open(&channelSettings);
                    if ( NULL == (pConfig->frontend[i+tuner_offset]) ) {
                        BDBG_ERR(("Unable to open external frontend %x demod %d (fe[%d]",probeResults.chip.familyId,i,i+tuner_offset));
                        continue;
                    }
                }
            }
        }

    }
#if NEXUS_PLATFORM_7366_DAUGHTERCARD
    if (daughtercard) {
        NEXUS_FrontendProbeResults probeResults;
        NEXUS_FrontendDeviceOpenSettings deviceSettings;
        NEXUS_FrontendDeviceCapabilities capabilities;
        NEXUS_FrontendChannelSettings channelSettings;

        NEXUS_Platform_Frontend_P_Daughtercard_Pinmux(sv, sff); /* change SPI/I2C pinmux for MTSIF header */

        NEXUS_FrontendDevice_GetDefaultOpenSettings(&deviceSettings);

        deviceSettings.i2cAddress = 0x68;
        deviceSettings.i2cDevice = pConfig->i2c[dcI2cChannel];

        rc = NEXUS_FrontendDevice_Probe(&deviceSettings, &probeResults);

        if (probeResults.chip.familyId == 0) {
            deviceSettings.i2cAddress = 0x6a;
            rc = NEXUS_FrontendDevice_Probe(&deviceSettings, &probeResults);
        }

        if (probeResults.chip.familyId != 0) {
            NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
            unsigned tuner_offset;
            NEXUS_GpioSettings gpioSettings;

            tuner_offset = i;

            /* already have default settings, and i2c device and channel from the probe */

            deviceSettings.satellite.enabled = true;
            BDBG_MSG(("Setting up interrupt on GPIO %d",dcInterrupt));
            NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
            gpioSettings.mode = NEXUS_GpioMode_eInput;
            gpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;
            gpioHandleInt = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, dcInterrupt, &gpioSettings);
            BDBG_ASSERT(NULL != gpioHandleInt);
            deviceSettings.gpioInterrupt = gpioHandleInt;

            device = NEXUS_FrontendDevice_Open(0, &deviceSettings);
            if (!device) {
                BDBG_WRN(("No daughtercard frontend found."));
            }
            else {
                NEXUS_FrontendDevice_GetCapabilities(device, &capabilities);
                for (i=0; i < capabilities.numTuners; i++)
                {
                    NEXUS_Frontend_GetDefaultOpenSettings(&channelSettings);
                    channelSettings.device = device;
                    channelSettings.channelNumber = i;
                    channelSettings.type = NEXUS_FrontendChannelType_eSatellite;
                    pConfig->frontend[i+tuner_offset] = NEXUS_Frontend_Open(&channelSettings);
                    if ( NULL == (pConfig->frontend[i+tuner_offset]) ) {
                        BDBG_ERR(("Unable to open daughtercard %x demod %d (fe[%d]",probeResults.chip.familyId,i,i+tuner_offset));
                        continue;
                    }
                }
            }
        }
    }

#endif
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
#if NEXUS_PLATFORM_7366_DAUGHTERCARD
    if (gpioHandleInt) {
        NEXUS_Gpio_Close(gpioHandleInt);
        gpioHandleInt = NULL;
    }
#endif

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
    /* 97366 Uses input band 4 or 7 for steramer */
    /* but the HW team recommands using input band 4 for A0 */
    /* 97366B0 only has a single IB and that's 10 (parallel) and IB0-3 for serial */
#if(BCHP_VER >= BCHP_VER_B0)
    *pInputBand = NEXUS_InputBand_e2;
#else
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

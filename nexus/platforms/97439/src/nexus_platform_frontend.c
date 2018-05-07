/******************************************************************************
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
 ******************************************************************************/
#include "nexus_platform_module.h"
#include "nexus_platform_priv.h"
#include "priv/nexus_core.h"
#include "nexus_platform_features.h"
#include "nexus_base.h"
#include "bchp_gio.h"
#include "bchp_aon_ctrl.h"

BDBG_MODULE(nexus_platform_frontend);

#if NEXUS_HAS_FRONTEND && (NEXUS_USE_7439_SV || NEXUS_USE_7449S_CWM)
#include "nexus_frontend.h"
#include "priv/nexus_frontend_standby_priv.h"

#include "nexus_gpio.h"

#include "bchp_sun_top_ctrl.h"
#include "bchp_aon_pin_ctrl.h"

static NEXUS_GpioHandle gpioHandleInt = NULL;

static NEXUS_GpioHandle gpioHandleReset = NULL;

#define NEXUS_PLATFORM_DAUGHTERCARD_USES_SPI 1

#ifdef NEXUS_PLATFORM_DAUGHTERCARD_USES_SPI
static NEXUS_SpiHandle g_dc_spi[NEXUS_NUM_SPI_CHANNELS];
#endif
/* Example function to power down or power up SPI or I2C pads on the backend when transitioning to standby. */
NEXUS_Error NEXUS_Platform_FrontendStandby(NEXUS_FrontendDeviceHandle handle, void *context, const NEXUS_FrontendStandbySettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

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
#if NEXUS_PLATFORM_DAUGHTERCARD_USES_SPI
    NEXUS_SpiSettings spiSettings;
#endif
    bool sv_dr4 = true;
    bool cwm = false;
    unsigned interrupt = 111;
    NEXUS_GpioType interruptType = NEXUS_GpioType_eStandard;

    {
        NEXUS_PlatformStatus platformStatus;

        NEXUS_Platform_GetStatus(&platformStatus);
        BDBG_MSG(("board major: %d, minor: %d",platformStatus.boardId.major,platformStatus.boardId.minor));
        if (platformStatus.boardId.major == 9) {
            interrupt = 3;
            sv_dr4 = false;
            cwm = true;
            interruptType = NEXUS_GpioType_eAonStandard;
        }
    }

    NEXUS_FrontendDevice_GetDefaultOpenSettings(&deviceSettings);

    if (sv_dr4)
    {
        BREG_Handle hReg = g_pCoreHandles->reg;
        uint32_t reg;

        reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_21);
        reg &= ~(
                 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_21, gpio_110)
                 );
        reg |= (
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_21, gpio_110, 0) /* GPIO_110 */
                );
        BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_21, reg);
    }
    if (cwm || sv_dr4)
    {
        BREG_Handle hReg = g_pCoreHandles->reg;
        uint32_t reg;

        reg = BREG_Read32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0);
        reg &= ~(
                 BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_06)
                 );
        reg |= (
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_06, 3)   /* SPI_M_SS2B */
                );
        BREG_Write32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0, reg);

        reg = BREG_Read32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1);
        reg &= ~(
                 BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_13) |
                 BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_14) |
                 BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_15)
                 );
        reg |= (
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_13, 3) | /* SPI_M_SCK */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_14, 3) | /* SPI_M_MOSI */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_15, 3)   /* SPI_M_MISO */
                );
        BREG_Write32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, reg);

        reg = BREG_Read32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2);
        reg &= ~(
                 BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_16)
                 );
        reg |= (
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_16, 3)   /* SPI_M_SS0B */
                );
        BREG_Write32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2, reg);
    }
    if (cwm) {
        BDBG_MSG(("pulling reset AON GPIO 01 high..."));
        NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eAonStandard, &gpioSettings);
        gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
        gpioSettings.value = NEXUS_GpioValue_eLow;
        gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
        gpioHandleReset = NEXUS_Gpio_Open(NEXUS_GpioType_eAonStandard, 1, &gpioSettings);
        BKNI_Delay(200);
        NEXUS_Gpio_Close(gpioHandleReset);
        gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
        gpioSettings.value = NEXUS_GpioValue_eHigh;
        gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
        gpioHandleReset = NEXUS_Gpio_Open(NEXUS_GpioType_eAonStandard, 1, &gpioSettings);
        BKNI_Delay(250);
        NEXUS_Gpio_Close(gpioHandleReset);
        gpioHandleReset = NULL;
    }

#if NEXUS_PLATFORM_DAUGHTERCARD_USES_SPI
    NEXUS_Spi_GetDefaultSettings(&spiSettings);
    spiSettings.clockActiveLow = true;
    if (sv_dr4) {
        BDBG_MSG(("Forcing 110 high..."));
        NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
        gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
        gpioSettings.value = NEXUS_GpioValue_eHigh;
        gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
        gpioHandleInt = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 110, &gpioSettings);
        NEXUS_Gpio_Close(gpioHandleInt);
        BKNI_Delay(100);
        gpioHandleInt = NULL;

        g_dc_spi[0] = NEXUS_Spi_Open(0, &spiSettings);
        if (!g_dc_spi[0]) {
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
        deviceSettings.spiDevice = g_dc_spi[0];
    }

    if (cwm) {
        g_dc_spi[2] = NEXUS_Spi_Open(2, &spiSettings);
        if (!g_dc_spi[2]) {
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
        deviceSettings.spiDevice = g_dc_spi[2];
    }
    deviceSettings.i2cDevice = NULL;
#else
    if (sv_dr4) {
        BDBG_MSG(("Forcing 110 low..."));
        NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
        gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
        gpioSettings.value = NEXUS_GpioValue_eLow;
        gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
        gpioHandleInt = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 110, &gpioSettings);
        NEXUS_Gpio_Close(gpioHandleInt);
        BKNI_Delay(100);
        gpioHandleInt = NULL;
    }

    deviceSettings.i2cDevice = pConfig->i2c[3];
    deviceSettings.i2cAddress = 0x68;
#endif

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
        if (probeResults.chip.familyId != 0 && deviceSettings.i2cDevice) {
            deviceSettings.i2cAddress = 0x6a;
            NEXUS_FrontendDevice_Probe(&deviceSettings, &probeResults);
        }
        if (probeResults.chip.familyId != 0) {
            NEXUS_FrontendDeviceCapabilities capabilities;

            BDBG_WRN(("Opening %x...",probeResults.chip.familyId));
            device = NEXUS_FrontendDevice_Open(0, &deviceSettings);
            if (!device) {
                BDBG_WRN(("No frontend found."));
                return NEXUS_SUCCESS;
            }

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
                BDBG_MSG(("%xfe: %d(%d):%p",probeResults.chip.familyId,i,i,(void *)pConfig->frontend[i]));
            }
            if (probeResults.chip.familyId == 0x3158) {
                NEXUS_FrontendStandbyCallback callback;
                BKNI_Memset(&callback, 0, sizeof(callback));
                callback.platformStandby = NEXUS_Platform_FrontendStandby;
                /* Set platform specific context data */
                callback.context = NULL;
                NEXUS_Frontend_SetStandbyCallback_priv(device, &callback);
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
            NEXUS_Frontend_SetStandbyCallback_priv(deviceHandles[i], NULL);
            NEXUS_FrontendDevice_Close(deviceHandles[i]);
            deviceHandles[i] = NULL;
        }
    }
    if(gpioHandleInt)
    {
        NEXUS_Gpio_Close(gpioHandleInt);
        gpioHandleInt = NULL;
    }
#ifdef NEXUS_PLATFORM_DAUGHTERCARD_USES_SPI
    for (i=0; i<NEXUS_NUM_SPI_CHANNELS; i++)
    {
        if (g_dc_spi[i])
        {
            BDBG_MSG(("spi[%d] = %p", i, (void *)g_dc_spi[i]));
            NEXUS_Spi_Close(g_dc_spi[i]);
            g_dc_spi[i] = NULL;
        }
    }
#endif
    return;
}
#elif NEXUS_HAS_FRONTEND && NEXUS_USE_7252S_SAT
#include "nexus_frontend.h"

#include "bchp_sun_top_ctrl.h"

#include "nexus_gpio.h"

#define NEXUS_PLATFORM_SAT_GPIO_INTERRUPT 60
#define NEXUS_PLATFORM_SAT_I2C 3
#define NEXUS_PLATFORM_SAT_FRONTEND_I2C_ADDRESS 0x68
#define NEXUS_PLATFORM_SAT_MTSIF_OFFSET 2

static NEXUS_GpioHandle gpioHandleInt = NULL;

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    NEXUS_FrontendDeviceHandle device;
    NEXUS_FrontendUserParameters userParams;
    unsigned i=0;
    NEXUS_FrontendDeviceOpenSettings deviceSettings;
    NEXUS_GpioSettings gpioSettings;

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

    NEXUS_FrontendDevice_GetDefaultOpenSettings(&deviceSettings);

    deviceSettings.i2cDevice = pConfig->i2c[NEXUS_PLATFORM_SAT_I2C];
    deviceSettings.i2cAddress = NEXUS_PLATFORM_SAT_FRONTEND_I2C_ADDRESS;
    deviceSettings.satellite.diseqc.i2cAddress= 0x0B;
    deviceSettings.satellite.diseqc.i2cDevice = pConfig->i2c[NEXUS_PLATFORM_SAT_I2C];

    BDBG_MSG(("Setting up interrupt on GPIO %d",NEXUS_PLATFORM_SAT_GPIO_INTERRUPT));
    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
    gpioSettings.mode = NEXUS_GpioMode_eInput;
    gpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;
    gpioHandleInt = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, NEXUS_PLATFORM_SAT_GPIO_INTERRUPT, &gpioSettings);
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
            if (!device) {
                BDBG_WRN(("No frontend found."));
                return NEXUS_SUCCESS;
            }

            NEXUS_FrontendDevice_GetCapabilities(device, &capabilities);
            BDBG_MSG(("Opening %d %x frontends",capabilities.numTuners,probeResults.chip.familyId));
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
    return;
}

#elif NEXUS_HAS_FRONTEND && NEXUS_USE_7439_VMS_SFF
#include "bchp_hif_cpu_intr1.h"
static   NEXUS_GpioHandle gpioHandle = NULL;
/* Uncomment the following define in order to disable the i2c address search */
/*#define NEXUS_PLATFORM_BYPASS_I2C_ADDRESS_SEARCH 1*/

#define I2C_DEVICE_VOLTAGE_REG_CH 1
#define I2C_4538_INDEX_1 4
#define I2C_4538_INDEX_2 4

#define NUM_4538_CHANNELS_PER 8
#define I2C_4538_ADDRESS_1 0x6a
#define EXT_4538_IRQ_1 16

#include "breg_i2c.h"
#include "priv/nexus_i2c_priv.h"

static bool NEXUS_Platform_P_Is4538(NEXUS_I2cHandle i2cDevice, uint16_t i2cAddr)
{
    BREG_I2C_Handle i2cHandle;
    uint8_t buf[5];
    uint16_t chipId=0;
    uint8_t subAddr;

    i2cHandle = NEXUS_I2c_GetRegHandle(i2cDevice, NULL);
    BDBG_MSG(("i2c handle: %p, i2caddr: 0x%x",(void *)i2cHandle,i2cAddr));
    buf[0]= 0x0;
    subAddr = 0x1;
    BREG_I2C_WriteNoAddr(i2cHandle, i2cAddr, (uint8_t *)&subAddr, 1);
    BREG_I2C_ReadNoAddr(i2cHandle, i2cAddr, buf, 1);
    chipId = buf[0];

    subAddr = 0x2;
    BREG_I2C_WriteNoAddr(i2cHandle, i2cAddr, (uint8_t *)&subAddr, 1);
    BREG_I2C_ReadNoAddr(i2cHandle, i2cAddr, buf, 1);
    chipId = (chipId <<8) | buf[0];

    BDBG_MSG(("chip ID = 0x%04x", chipId));

    return chipId == 0x4538;
}


NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    unsigned i=0;
    NEXUS_4538Settings st4538Settings;
    NEXUS_FrontendUserParameters userParams;
    uint16_t i2cAddr = I2C_4538_ADDRESS_1;

    if ((!pConfig->i2c[I2C_4538_INDEX_1]) && (!pConfig->i2c[I2C_4538_INDEX_2]) ){
        BDBG_ERR(("Frontend cannot be initialized without first initializing I2C."));
        return BERR_NOT_INITIALIZED;
    }

    BDBG_MSG(("Checking i2c: 0x%02x",I2C_4538_ADDRESS_1));
    if (!NEXUS_Platform_P_Is4538(pConfig->i2c[I2C_4538_INDEX_1],I2C_4538_ADDRESS_1)) {
#if NEXUS_PLATFORM_BYPASS_I2C_ADDRESS_SEARCH
        BDBG_ERR(("Unable to locate 4538 at 0x%02x",I2C_4538_ADDRESS_1));
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
#else
        int ix;
        for (ix=0x68; ix<=0x6f; ix++) {
            BDBG_MSG(("Checking i2c: 0x%02x",ix));
            if (ix != I2C_4538_ADDRESS_1 && NEXUS_Platform_P_Is4538(pConfig->i2c[I2C_4538_INDEX_1],ix)) {
                i2cAddr = ix;
                BDBG_MSG(("Found 4538 at 0x%02x",ix));
                break;
            }
        }
        if (i2cAddr == I2C_4538_ADDRESS_1) {
            BDBG_ERR(("Unable to locate 4538"));
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
#endif
    }

    NEXUS_Frontend_GetDefault4538Settings(&st4538Settings);

    st4538Settings.i2cDevice = pConfig->i2c[I2C_4538_INDEX_1];
    st4538Settings.i2cAddr = i2cAddr;
    st4538Settings.isrNumber = EXT_4538_IRQ_1;
    st4538Settings.gpioInterrupt = NULL;

    for (i=0; i <  NUM_4538_CHANNELS_PER; i++)
    {
        st4538Settings.channelNumber = i;
        pConfig->frontend[i] = NEXUS_Frontend_Open4538(&st4538Settings);
        if ( NULL == (pConfig->frontend[i]) )
        {
            BDBG_ERR(("Unable to open onboard 4538 tuner/demodulator %d",i));
        }
        NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
        userParams.isMtsif = true;
        userParams.param1 = userParams.isMtsif ? st4538Settings.channelNumber : NEXUS_InputBand_e0 + i;
        userParams.pParam2 = 0;
        NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
    }

    return NEXUS_SUCCESS;
}

void NEXUS_Platform_UninitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    unsigned i=0;

    for (i=0; i<NEXUS_MAX_FRONTENDS; i++)
    {
        if (pConfig->frontend[i]) {
            NEXUS_Frontend_Close(pConfig->frontend[i]);
            pConfig->frontend[i] = NULL;
        }
    }

    if(gpioHandle)
    {
        NEXUS_Gpio_Close(gpioHandle);
        gpioHandle = NULL;
    }
    return;
}

#elif NEXUS_HAS_FRONTEND && NEXUS_USE_7439_SFF && NEXUS_USE_3461_FRONTEND_DAUGHTER_CARD

#include "nexus_frontend_3461.h"

static NEXUS_GpioHandle gpioHandle = NULL;
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
    NEXUS_GpioSettings tunerGpioSettings;
    NEXUS_3461ProbeResults results;

    BDBG_WRN(("Waiting for 3461 Downstream frontend(97252C) to initialize"));
    /* GPIO 109 is used instead of EXT_IRQ. */
    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &tunerGpioSettings);

    tunerGpioSettings.mode = NEXUS_GpioMode_eInput;
    tunerGpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;

    gpioHandle = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard,16, &tunerGpioSettings);
    if (NULL == gpioHandle)
    {
    BDBG_ERR(("Unable to open GPIO for tuner."));
    return BERR_NOT_INITIALIZED;
    }

    if (!pConfig->i2c[3]) {
         BDBG_ERR(("Frontend cannot be initialized without first initializing I2C."));
         return BERR_NOT_INITIALIZED;
    }
    deviceOpenSettings.i2cDevice = pConfig->i2c[3]; /* Onboard tuner/demod use BSC_M3.*/
    deviceOpenSettings.i2cAddr = 0x6c;
    deviceOpenSettings.gpioInterrupt = gpioHandle;
    deviceOpenSettings.isrNumber= 0;
    deviceOpenSettings.loadAP = true;
    deviceOpenSettings.externalFixedGain.total = 0;    /* These are platform specific values given by the board designer. */
    deviceOpenSettings.externalFixedGain.bypassable = 0; /* These are platform specific values given by the board designer. */
    deviceOpenSettings.crystalSettings.enableDaisyChain = false;

    if(NEXUS_Frontend_Probe3461(&deviceOpenSettings, &results) != NEXUS_SUCCESS){
     BDBG_ERR(("3461 tuner not found"));
     rc = BERR_NOT_INITIALIZED; goto done;
    }
    BDBG_WRN(("chip.familyId = 0x%x", results.chip.familyId));
    BDBG_WRN(("chip.id = 0x%x", results.chip.id));
    BDBG_WRN(("version.major = 0x%x", results.chip.version.major ));
    BDBG_WRN(("version.minor = 0x%x", results.chip.version.minor ));

    parentDevice = NEXUS_FrontendDevice_Open3461(0, &deviceOpenSettings);
    if (NULL == parentDevice)
    {
     BDBG_ERR(("Unable to open first 3461 tuner/demodulator device"));
     rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    }
    NEXUS_FrontendDevice_GetDefault3461Settings(&deviceSettings);
    deviceSettings.rfDaisyChain = NEXUS_3461RfDaisyChain_eInternalLna;
    deviceSettings.rfInput = NEXUS_3461TunerRfInput_eInternalLna;
    deviceSettings.enableRfLoopThrough = false;
    deviceSettings.terrestrial = true;
    NEXUS_FrontendDevice_Set3461Settings(parentDevice, &deviceSettings);

    NEXUS_Frontend_GetDefault3461Settings(&st3461Settings);
    st3461Settings.device = parentDevice;
    st3461Settings.type = NEXUS_3461ChannelType_eDvbt; /*REDUNDANT for now as there is only one instance of any demod running. */
    st3461Settings.channelNumber = 0;                     /*REDUNDANT for now. */

    pConfig->frontend[0] = NEXUS_Frontend_Open3461(&st3461Settings);
    if (NULL == pConfig->frontend[0])
    {
     BDBG_ERR(("Unable to open first 3461 dvbt2 tuner/demodulator channel."));
     rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    }

    NEXUS_Frontend_GetType(pConfig->frontend[0], &type);
    BDBG_WRN(("familyId = 0x%x", type.chip.familyId));
    BDBG_WRN(("chipId = 0x%x", type.chip.id));
    BDBG_WRN(("version.major = 0x%x", type.chip.version.major ));
    BDBG_WRN(("version.major = 0x%x", type.chip.version.minor ));
    BDBG_WRN(("version.buildType = 0x%x", type.chip.version.buildType ));
    BDBG_WRN(("version.buildId = 0x%x", type.chip.version.buildId ));
    BDBG_WRN(("bondoutOptions[0] = 0x%x", type.chip.bondoutOptions[0] ));
    BDBG_WRN(("bondoutOptions[1] = 0x%x", type.chip.bondoutOptions[1] ));

    BDBG_WRN(("firmwareVersion.major = 0x%x", type.firmwareVersion.major ));
    BDBG_WRN(("firmwareVersion.major = 0x%x", type.firmwareVersion.minor ));
    BDBG_WRN(("firmwareVersion.buildType = 0x%x", type.firmwareVersion.buildType ));
    BDBG_WRN(("firmwareVersion.buildId = 0x%x", type.firmwareVersion.buildId ));

    NEXUS_Frontend_GetUserParameters(pConfig->frontend[0], &userParams);
    userParams.param1 = NEXUS_InputBand_e0;
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
    if(gpioHandle)
    {
     NEXUS_Gpio_Close(gpioHandle);
     gpioHandle = NULL;
    }

    return;
}

#elif NEXUS_HAS_FRONTEND && NEXUS_USE_7251_T2SFF
#include "nexus_frontend_3461.h"
static NEXUS_GpioHandle tunerGpio[NEXUS_MAX_FRONTENDS];

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
    NEXUS_3461ProbeResults results;
    NEXUS_GpioSettings tunerGpioSettings;
    NEXUS_FrontendDeviceCapabilities capabilities;
#ifdef USE_SPI_FRONTEND
    NEXUS_SpiSettings spiSettings;
#endif
    BDBG_WRN(("Waiting for the first 3461 Downstream frontend(7251_T2SFF) to initialize"));

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
    if (!pConfig->i2c[1]) {
            BDBG_ERR(("Frontend cannot be initialized without first initializing I2C."));
            rc = BERR_NOT_INITIALIZED; goto done;
    }
    deviceOpenSettings.i2cDevice = pConfig->i2c[1];    /* Onboard tuner/demod use BSC_M1.*/
    deviceOpenSettings.i2cAddr = 0x6c;
#endif
    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &tunerGpioSettings);
    tunerGpioSettings.mode = NEXUS_GpioMode_eInput;
    tunerGpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;
    tunerGpio[0] = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard,32, &tunerGpioSettings);
    if (NULL == tunerGpio[0])
    {
        BDBG_ERR(("Unable to open GPIO for tuner %d", 0));
    }
    deviceOpenSettings.gpioInterrupt = tunerGpio[0];
    deviceOpenSettings.loadAP = true;
    deviceOpenSettings.crystalSettings.enableDaisyChain = true;

    if(NEXUS_Frontend_Probe3461(&deviceOpenSettings, &results) != NEXUS_SUCCESS){
        BDBG_ERR(("3461 tuner not found"));
        rc = BERR_NOT_INITIALIZED; goto done;
    }
    BDBG_WRN(("chip.familyId = 0x%x", results.chip.familyId));
    BDBG_WRN(("chip.id = 0x%x", results.chip.id));
    BDBG_WRN(("version.major = 0x%x", results.chip.version.major ));
    BDBG_WRN(("version.minor = 0x%x", results.chip.version.minor ));


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
    userParams.pParam2 = 0;
    NEXUS_Frontend_SetUserParameters(pConfig->frontend[0], &userParams);

    NEXUS_FrontendDevice_GetCapabilities(parentDevice, &capabilities);

    BDBG_WRN(("Waiting for the second 3461 Downstream frontend(7251_T2SFF) to initialize"));

#ifdef USE_SPI_FRONTEND
    g_3461spi[1] = NEXUS_Spi_Open(1, &spiSettings);
    if (!g_3461spi[1]) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    deviceOpenSettings.i2cDevice = NULL;
    deviceOpenSettings.spiDevice = g_3461spi[1];
    deviceOpenSettings.spiAddr = 0x40;

#else
    if (!pConfig->i2c[4]) {
            BDBG_ERR(("Frontend cannot be initialized without first initializing I2C."));
            rc = BERR_NOT_INITIALIZED; goto done;
    }
    deviceOpenSettings.i2cAddr = 0x6d;
#endif
    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &tunerGpioSettings);
    tunerGpioSettings.mode = NEXUS_GpioMode_eInput;
    tunerGpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;
    tunerGpio[1] = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard,33, &tunerGpioSettings);
    if (NULL == tunerGpio[1])
    {
        BDBG_ERR(("Unable to open GPIO for tuner %d", 0));
    }

    deviceOpenSettings.gpioInterrupt = tunerGpio[1];
    deviceOpenSettings.loadAP = true;
    deviceOpenSettings.crystalSettings.enableDaisyChain = false;

    st3461Settings.device = NEXUS_FrontendDevice_Open3461(0, &deviceOpenSettings);
    if(st3461Settings.device == NULL){
        BDBG_ERR(("Unable to open second nboard 3461 tuner/demodulator device"));
        rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    }

    NEXUS_FrontendDevice_GetDefault3461Settings(&deviceSettings);
    deviceSettings.rfDaisyChain = NEXUS_3461RfDaisyChain_eOff;
    deviceSettings.rfInput = NEXUS_3461TunerRfInput_eInternalLna;
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
    userParams.pParam2 = 0;
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
    for (i=0; i<NEXUS_MAX_FRONTENDS; i++)
    {
        if(tunerGpio[i])
        {
            NEXUS_Gpio_Close(tunerGpio[i]);
            tunerGpio[i] = NULL;
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

#elif NEXUS_HAS_FRONTEND && defined NEXUS_USE_74371_XID

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
    NEXUS_SpiSettings spiSettings;
    static NEXUS_SpiHandle g_3128spi;

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

#if !defined(NEXUS_USE_74371_XID)
    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eAonStandard, &gpioSettings);
    gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
    gpioSettings.value = NEXUS_GpioValue_eHigh;
    gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
    gpioSettings.interrupt.callback = NULL;
    gpioHandle = NEXUS_Gpio_Open(NEXUS_GpioType_eAonStandard, 10, &gpioSettings);
    BDBG_ASSERT(NULL != gpioHandle);
#endif

    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
    gpioSettings.mode = NEXUS_GpioMode_eInput;
    gpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;
#if defined(NEXUS_USE_74371_XID)
    /* use level trigger interrupt */
    gpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;
    tunerGpio = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 103, &gpioSettings);
#else
    tunerGpio = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 89, &gpioSettings);
#endif
    if (NULL == tunerGpio)
    {
        BDBG_ERR(("Unable to open GPIO for tuner %d", 0));
    }
    BDBG_WRN(("tunerGpio = 0x%x", tunerGpio));
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

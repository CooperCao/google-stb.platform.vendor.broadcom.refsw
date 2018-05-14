/***************************************************************************
*  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
***************************************************************************/
#include "nexus_platform_module.h"
#include "nexus_platform_priv.h"
#include "nexus_types.h"
#include "priv/nexus_core.h"
#include "bchp_gio.h"
#if NEXUS_HAS_FRONTEND
#if NEXUS_USE_7445_VMS_SFF || NEXUS_USE_7445_SV
#include "nexus_frontend_3128.h"
#elif NEXUS_USE_7445_C || NEXUS_USE_7445_AUTO
#include "nexus_frontend_3461.h"
#endif
#else
#undef NEXUS_USE_7445_SV
#endif
#include "bchp_sun_top_ctrl.h"

#define NEXUS_PLATFORM_7445_EXT24D_DAISY_OVERRIDE 0 /* MTSIF load-balancing options: 0 for normal daisychain config. 1 for hybrid daisychain config. See below */

BDBG_MODULE(nexus_platform_frontend);

#if NEXUS_USE_7445_EXT24
#include "priv/nexus_frontend_mtsif_priv.h"
#include "nexus_gpio.h"

#define NEXUS_PLATFORM_NUM_FRONTEND_INTERRUPTS 2

static NEXUS_GpioHandle gpioHandleInt[NEXUS_PLATFORM_NUM_FRONTEND_INTERRUPTS] = {NULL};

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    NEXUS_FrontendDeviceHandle device;
    NEXUS_FrontendDeviceHandle firstDevice = NULL;
    unsigned i=0;
    unsigned offset = 0;
    NEXUS_FrontendDeviceOpenSettings deviceSettings;
    NEXUS_GpioSettings gpioSettings;
    NEXUS_FrontendProbeResults probeResults;
    unsigned intGpio = 0;

    /* open first frontend */
    NEXUS_FrontendDevice_GetDefaultOpenSettings(&deviceSettings);
    deviceSettings.i2cDevice = pConfig->i2c[4];
    deviceSettings.i2cAddress = 0x68;
    deviceSettings.mtsif[0].clockRate = 135000000;
    deviceSettings.mtsif[1].clockRate = 135000000;

    NEXUS_FrontendDevice_Probe(&deviceSettings, &probeResults);
    if (probeResults.chip.familyId != 0) {
        BDBG_MSG(("Opening %x...", probeResults.chip.familyId));

        intGpio = 46;
        BDBG_MSG(("Setting up interrupt on GPIO %d", intGpio));
        NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
        gpioSettings.mode = NEXUS_GpioMode_eInput;
        gpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;
        gpioHandleInt[0] = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, intGpio, &gpioSettings);
        BDBG_ASSERT(NULL != gpioHandleInt[0]);

        deviceSettings.gpioInterrupt = gpioHandleInt[0];

        device = NEXUS_FrontendDevice_Open(0, &deviceSettings);

        if (device) {
            NEXUS_FrontendDeviceCapabilities capabilities;

            NEXUS_FrontendDevice_GetCapabilities(device, &capabilities);
            BDBG_MSG(("Opening %d %x frontends",capabilities.numTuners,probeResults.chip.familyId));
            for (i=0; i < capabilities.numTuners ; i++)
            {
                NEXUS_FrontendChannelSettings channelSettings;
                channelSettings.device = device;
                channelSettings.channelNumber = i;
                channelSettings.type = NEXUS_FrontendChannelType_eSatellite;
                pConfig->frontend[i] = NEXUS_Frontend_Open(&channelSettings);
                if ( NULL == (pConfig->frontend[i]) )
                {
                    BDBG_ERR(("Unable to open %x demod %d (as frontend[%d])",probeResults.chip.familyId,i,i));
                    continue;
                }
                BDBG_MSG(("%xfe: %d(%d):%p",probeResults.chip.familyId,i,i,(void *)pConfig->frontend[i]));
            }
            offset = i;
            firstDevice = device;
        } else {
            BDBG_ERR(("Unable to open detected %x frontend", probeResults.chip.familyId));
        }
    }

    /* open second frontend */
    device = NULL;
    NEXUS_FrontendDevice_GetDefaultOpenSettings(&deviceSettings);
    deviceSettings.i2cDevice = pConfig->i2c[4];
    deviceSettings.i2cAddress = 0x69;
    deviceSettings.mtsif[0].clockRate = 135000000;
    deviceSettings.mtsif[1].clockRate = 135000000;

    NEXUS_FrontendDevice_Probe(&deviceSettings, &probeResults);
    if (probeResults.chip.familyId != 0) {
        BDBG_MSG(("Opening %x...", probeResults.chip.familyId));

        intGpio = 48;
        BDBG_MSG(("Setting up interrupt on GPIO %d", intGpio));
        NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
        gpioSettings.mode = NEXUS_GpioMode_eInput;
        gpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;
        gpioHandleInt[1] = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, intGpio, &gpioSettings);
        BDBG_ASSERT(NULL != gpioHandleInt[1]);

        deviceSettings.gpioInterrupt = gpioHandleInt[1];

        device = NEXUS_FrontendDevice_Open(0, &deviceSettings);

        if (device) {
            NEXUS_FrontendDeviceCapabilities capabilities;

            NEXUS_FrontendDevice_GetCapabilities(device, &capabilities);
            BDBG_MSG(("Opening %d %x frontends",capabilities.numTuners,probeResults.chip.familyId));
            for (i=0; i < capabilities.numTuners ; i++)
            {
                NEXUS_FrontendChannelSettings channelSettings;
                channelSettings.device = device;
                channelSettings.channelNumber = i;
                channelSettings.type = NEXUS_FrontendChannelType_eSatellite;
                pConfig->frontend[i+offset] = NEXUS_Frontend_Open(&channelSettings);
                if ( NULL == (pConfig->frontend[i+offset]) )
                {
                    BDBG_ERR(("Unable to open %x demod %d (as frontend[%d])",probeResults.chip.familyId,i,i+offset));
                    continue;
                }
                BDBG_MSG(("%xfe: %d(%d):%p",probeResults.chip.familyId,i,i,(void *)pConfig->frontend[i+offset]));

#if NEXUS_PLATFORM_7445_EXT24D_DAISY_OVERRIDE /* option 2: 45216 routes 4 PBs to 45208. Both devices have a single MTSIF connection to backend */
                if (i < 12) {
                    NEXUS_FrontendUserParameters params;
                    if (firstDevice && i==0) {
                        uint32_t val;
                        NEXUS_FrontendDeviceLinkSettings settings;
                        NEXUS_FrontendDevice_GetDefaultLinkSettings(&settings);
                        settings.mtsif = NEXUS_FrontendDeviceMtsifOutput_eDaisy;
                        NEXUS_FrontendDevice_Link(firstDevice, device, &settings);

                        /* pinmux to enable RX on 45208 (firstDevice) */
                        NEXUS_Frontend_ReadRegister(pConfig->frontend[0], 0x6920218, &val);
                        val &= ~(0xfffff000);
                        val |= 0x11111000;
                        NEXUS_Frontend_WriteRegister(pConfig->frontend[0], 0x6920218, val);
                        NEXUS_Frontend_ReadRegister(pConfig->frontend[0], 0x692021c, &val);
                        val &= ~(0xfffff);
                        val |= 0x11111;
                        NEXUS_Frontend_WriteRegister(pConfig->frontend[0], 0x692021c, val);
                    }
                    NEXUS_Frontend_GetUserParameters(pConfig->frontend[i+offset], &params);
                    NEXUS_FRONTEND_USER_PARAM1_SET_MTSIF_TX(params.param1, 0);
                    NEXUS_FRONTEND_USER_PARAM1_SET_DAISYCHAIN_OVERRIDE(params.param1, 1);
                    NEXUS_Frontend_SetUserParameters(pConfig->frontend[i+offset], &params);
                }
                else {
                    NEXUS_FrontendUserParameters params;
                    NEXUS_Frontend_GetUserParameters(pConfig->frontend[i+offset], &params);
                    NEXUS_FRONTEND_USER_PARAM1_SET_MTSIF_TX(params.param1, 1);
                    NEXUS_Frontend_SetUserParameters(pConfig->frontend[i+offset], &params);
                }
#endif
            }
        } else {
            BDBG_ERR(("Unable to open detected %x frontend", probeResults.chip.familyId));
        }
    }

#if (!NEXUS_PLATFORM_7445_EXT24D_DAISY_OVERRIDE) /* option 1: 45208 routes 8 PBs to 45216. 45216 has two MTSIF connections to backend */
    if (firstDevice && device) {
        NEXUS_FrontendDeviceLinkSettings settings;
        NEXUS_FrontendUserParameters params;
        NEXUS_FrontendDevice_GetDefaultLinkSettings(&settings);
        settings.mtsif = NEXUS_FrontendDeviceMtsifOutput_eDaisy;
        NEXUS_FrontendDevice_Link(device, firstDevice, &settings);

        for (i=0; i<12; i++) { /* load-balance MTSIF by sending half the PBs on TX0 and half on TX1 */
            if (pConfig->frontend[i]==NULL) {
                BDBG_ERR(("No frontend handle at index %u", i));
                continue;
            }
            if (i < 8) {
                NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &params);
                NEXUS_FRONTEND_USER_PARAM1_SET_MTSIF_TX(params.param1, 1); /* 45208_TX1 (not TX0) is the one that's wired */
                NEXUS_FRONTEND_USER_PARAM1_SET_DAISYCHAIN_MTSIF_TX(params.param1, 1);
                NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &params);
            }
            else {
                NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &params);
                NEXUS_FRONTEND_USER_PARAM1_SET_MTSIF_TX(params.param1, 1); /* send the first 4 PBs from 45216 to TX1, along with the daisy'd PBs */
                NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &params);
            }
        }
    }
#endif

    return BERR_SUCCESS;
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

    for (i=0; i<NEXUS_PLATFORM_NUM_FRONTEND_INTERRUPTS; i++) {
        if(gpioHandleInt[i])
        {
            NEXUS_Gpio_Close(gpioHandleInt[i]);
            gpioHandleInt[i] = NULL;
        }
    }
}
#elif (NEXUS_USE_7445_VMS_SFF && NEXUS_HAS_GPIO) || (NEXUS_USE_7445_SV && BCHP_VER >= BCHP_VER_D0)
#if NEXUS_HAS_GPIO
static NEXUS_GpioHandle gpioHandle = NULL;
#endif

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    NEXUS_FrontendUserParameters userParams;
    unsigned i = 0;
    NEXUS_3128ProbeResults results;
    NEXUS_Frontend3128Settings st3128Settings;
    NEXUS_FrontendDevice3128OpenSettings st3128DeviceOpenSettings;
    NEXUS_GpioSettings gpioSettings;
    BREG_Handle hReg;
    NEXUS_PlatformStatus *platformStatus;

    NEXUS_Frontend_GetDefault3128Settings(&st3128Settings);
    NEXUS_FrontendDevice_GetDefault3128OpenSettings(&st3128DeviceOpenSettings);

    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
    gpioSettings.mode = NEXUS_GpioMode_eInput;
    gpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;
#if NEXUS_USE_7445_VMS_SFF
    BSTD_UNUSED(platformStatus);
    gpioHandle = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard,23, &gpioSettings);

    if (NULL == gpioHandle)
    {
      BDBG_ERR(("Unable to open GPIO for 3128 frontend interrupt."));
      return BERR_NOT_INITIALIZED;
    }
    st3128DeviceOpenSettings.gpioInterrupt = gpioHandle;
#else
    platformStatus = BKNI_Malloc(sizeof(*platformStatus));
    if (!platformStatus) {
        return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
    }
    rc = NEXUS_Platform_GetStatus(platformStatus);
    BDBG_ASSERT(!rc);
    BDBG_MSG(("Board ID: %d.%d", platformStatus->boardId.major, platformStatus->boardId.minor));

    if((platformStatus->boardId.major >= 1) && (platformStatus->boardId.minor >= 1)){
        gpioHandle = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard,60, &gpioSettings);

        if (NULL == gpioHandle)
        {
            BDBG_ERR(("Unable to open GPIO for 3128 frontend interrupt."));
            BKNI_Free(platformStatus);
            return BERR_NOT_INITIALIZED;
        }

        st3128DeviceOpenSettings.gpioInterrupt = gpioHandle;
    }
    else
        st3128DeviceOpenSettings.interruptMode = NEXUS_FrontendInterruptMode_ePolling;
    BKNI_Free(platformStatus);
#endif
#if NEXUS_USE_7445_SV
    st3128DeviceOpenSettings.i2cDevice = pConfig->i2c[3];    /* Onboard tuner/demod use BSC_M3.*/
#else
    st3128DeviceOpenSettings.i2cDevice = pConfig->i2c[4];    /* Onboard tuner/demod use BSC_M4.*/
#endif
    st3128DeviceOpenSettings.i2cAddr = 0x6c ;
    st3128DeviceOpenSettings.outOfBand.ifFrequency = 0;
    st3128DeviceOpenSettings.inBandOpenDrain=true;
    st3128DeviceOpenSettings.loadAP = true;
    st3128DeviceOpenSettings.configureWatchdog = false;
    st3128DeviceOpenSettings.isMtsif = true;

    hReg = g_pCoreHandles->reg;

    st3128DeviceOpenSettings.pinmux.data[0] = BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11;
    st3128DeviceOpenSettings.pinmux.data[1] = (BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11) | 0x10000);

    st3128DeviceOpenSettings.pinmux.data[2] = BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11;
    st3128DeviceOpenSettings.pinmux.data[3] = (BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11) | 0x20000);

    NEXUS_Frontend_Probe3128(&st3128DeviceOpenSettings, &results);
    BDBG_WRN(("familyId 0x%x, id 0x%x, version %d.%d", results.chip.familyId, results.chip.id, results.chip.version.major, results.chip.version.minor ));

    st3128Settings.device = NEXUS_FrontendDevice_Open3128(0, &st3128DeviceOpenSettings);
    if (NULL == st3128Settings.device)
    {
        BDBG_ERR(("Unable to open onboard 3128 tuner/demodulator device"));
        rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    }

    st3128Settings.type = NEXUS_3128ChannelType_eInBand;

    for (i=0; i<(results.chip.id & 0xF); i++)
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
        BDBG_MSG(("pConfig->frontend[%d] = %p", i, (void *)pConfig->frontend[i]));
    }
    BDBG_WRN(("opened %d channel(s)", i));
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

#if NEXUS_HAS_GPIO
    if(gpioHandle)
    {
        NEXUS_Gpio_Close(gpioHandle);
        gpioHandle = NULL;
    }
#endif
    return;
}

#elif (NEXUS_USE_7445_C || NEXUS_USE_7445_AUTO) && NEXUS_USE_3461_FRONTEND_DAUGHTER_CARD
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

    BDBG_WRN(("Waiting for 3461 Downstream frontend(97445C) to initialize"));
    /* GPIO 109 is used instead of EXT_IRQ. */
    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &tunerGpioSettings);

    tunerGpioSettings.mode = NEXUS_GpioMode_eInput;
    tunerGpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;
#if NEXUS_USE_7445_C && (BCHP_VER < BCHP_VER_D0)
    /* 7445_C V00 board IRQ is on 109. V10 and later (D0 and up) is on 104, same as 7445_AUTO */
    gpioHandle = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard,109, &tunerGpioSettings);
#else /* NEXUS_USE_7445_AUTO || (NEXUS_USE_7445_C && (BCHP_VER >= BCHP_VER_D0)) */
    gpioHandle = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard,104, &tunerGpioSettings);
#endif
    if (NULL == gpioHandle)
    {
      BDBG_ERR(("Unable to open GPIO for tuner."));
      return BERR_NOT_INITIALIZED;
    }

    if (!pConfig->i2c[3]) {
            BDBG_ERR(("Frontend cannot be initialized without first initializing I2C."));
            return BERR_NOT_INITIALIZED;
    }
    deviceOpenSettings.i2cDevice = pConfig->i2c[3];    /* Onboard tuner/demod use BSC_M3.*/
    deviceOpenSettings.i2cAddr = 0x6c;
    deviceOpenSettings.gpioInterrupt = gpioHandle;
    deviceOpenSettings.isrNumber= 0;
    deviceOpenSettings.loadAP = true;
    deviceOpenSettings.externalFixedGain.total = 0;       /* These are platform specific values given by the board designer. */
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
    st3461Settings.channelNumber = 0;                    /*REDUNDANT for now. */

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
#elif NEXUS_USE_7445_DBS
#include "nexus_frontend_4538.h"
#define ISL9492_CH0_I2C_ADDR 0x08
#define ISL9492_CH1_I2C_ADDR 0x09
static NEXUS_GpioHandle gpioHandle4538A = NULL;
static NEXUS_GpioHandle gpioHandle4538B = NULL;
static NEXUS_GpioHandle gpioHandleInt4538A = NULL;
static NEXUS_GpioHandle gpioHandleInt4538B = NULL;
/* Uncomment the following define in order to disable the i2c address search */
/*#define NEXUS_PLATFORM_BYPASS_I2C_ADDRESS_SEARCH 1*/
/* To debug asynchronous initialization, if defined, this skips the second chip */
/*#define SKIP_SECOND_4538_CHIP 1 */

#define I2C_4538_INDEX_1 4
#define I2C_4538_INDEX_2 4

#define NUM_4538_CHANNELS_PER 8
#define I2C_4538_ADDRESS_1 0x68
#define I2C_4538_ADDRESS_2 0x69
#define SPI_4538_ADDRESS_1 0x20
#define SPI_4538_ADDRESS_2 0x20
#define EXT_4538_GPIO_RESET_1 107
#define EXT_4538_GPIO_IRQ_1 23
#if (BCHP_VER >= BCHP_VER_D0)
/* schematic label says 104, trace says 96 */
#define EXT_4538_GPIO_IRQ_2 96
/* schematic label says 110, trace says 95 */
#define EXT_4538_GPIO_RESET_2 95
#define NEXUS_4538_USE_SPI 1
#else
#define EXT_4538_GPIO_RESET_2 71
#define EXT_4538_GPIO_IRQ_2 104
#endif
#define EXT_4538_IRQ_1 10
#define EXT_4538_IRQ_2 10

#if NEXUS_4538_USE_SPI
#include "nexus_spi.h"
static NEXUS_SpiHandle g_4538spi[NEXUS_NUM_SPI_CHANNELS];
#endif

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    unsigned i=0;
    NEXUS_FrontendDevice4538OpenSettings st4538Settings;
    NEXUS_FrontendDevice4538OpenSettings st4538Settings2;
    NEXUS_GpioSettings gpioSettings;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_FrontendDeviceHandle device;
    NEXUS_4538ProbeResults probeResults;
#if NEXUS_4538_USE_SPI
    NEXUS_SpiSettings spiSettings;
    uint16_t spiAddr = SPI_4538_ADDRESS_1;
    uint16_t spiAddr2 = SPI_4538_ADDRESS_2;
#else
    uint16_t i2cAddr = I2C_4538_ADDRESS_1;
    uint16_t i2cAddr2 = I2C_4538_ADDRESS_2;
#endif

    if (!pConfig->i2c[I2C_4538_INDEX_1]) {
        BDBG_ERR(("Frontend cannot be initialized without first initializing I2C."));
        return BERR_NOT_INITIALIZED;
    }

#if NEXUS_4538_USE_SPI
    /* Open SPI devices */
    NEXUS_Spi_GetDefaultSettings(&spiSettings);
    spiSettings.clockActiveLow = true;
    spiSettings.baud = 6750000;
    for (i=0; i < NEXUS_NUM_SPI_CHANNELS; i++) {
        g_4538spi[i] = NEXUS_Spi_Open(i, &spiSettings);
        if (!g_4538spi[i]) {
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
    }
#endif

    /* Bring up first 4538 */

    NEXUS_FrontendDevice_GetDefault4538OpenSettings(&st4538Settings);

    /* GPIO 107 is connected to the 4538A reset, so set it high to reset the first 4538 */
    st4538Settings.reset.enable = true;
    st4538Settings.reset.pin = EXT_4538_GPIO_RESET_1;
    st4538Settings.reset.type = NEXUS_GpioType_eStandard;
    st4538Settings.reset.value = NEXUS_GpioValue_eHigh;

#if NEXUS_4538_USE_SPI
    st4538Settings.i2cDevice = NULL;
    st4538Settings.i2cAddr = I2C_4538_ADDRESS_1; /* used internally as an index */
    st4538Settings.spiDevice = g_4538spi[0];
    st4538Settings.spiAddr = SPI_4538_ADDRESS_1;
    st4538Settings.diseqc.i2cDevice = pConfig->i2c[I2C_4538_INDEX_1];
#else
    st4538Settings.i2cDevice = pConfig->i2c[I2C_4538_INDEX_1];
    st4538Settings.i2cAddr = I2C_4538_ADDRESS_1;
#endif

    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
    gpioSettings.mode = NEXUS_GpioMode_eInput;
    gpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;
    gpioHandleInt4538A = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, EXT_4538_GPIO_IRQ_1, &gpioSettings);
    BDBG_ASSERT(NULL != gpioHandleInt4538A);
    st4538Settings.gpioInterrupt = gpioHandleInt4538A;

#if NEXUS_4538_USE_SPI
    if (!NEXUS_Frontend_Probe4538(&st4538Settings, &probeResults)) {
        if (probeResults.chip.familyId == 0x4538) {
            BDBG_MSG(("Found 4538A at 0x%02x",SPI_4538_ADDRESS_1));
        }
    }
    spiAddr = SPI_4538_ADDRESS_1;
#else
    BDBG_MSG(("Checking i2c: 0x%02x",I2C_4538_ADDRESS_1));
    if (!NEXUS_Frontend_Probe4538(&st4538Settings, &probeResults)) {
        if (probeResults.chip.familyId == 0x4538) {
            i2cAddr = I2C_4538_ADDRESS_1;
            BDBG_MSG(("Found 4538A at 0x%02x",I2C_4538_ADDRESS_1));
        }
    } else {
#if NEXUS_PLATFORM_BYPASS_I2C_ADDRESS_SEARCH
        BDBG_ERR(("Unable to locate 4538A at 0x%02x",I2C_4538_ADDRESS_1));
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
                        BDBG_MSG(("Found 4538A at 0x%02x",ix));
                        break;
                    }
                }
            }
        }
        if (i2cAddr == I2C_4538_ADDRESS_1) {
            BDBG_ERR(("Unable to locate 4538A"));
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
#endif
    }
#endif

#if !SKIP_SECOND_4538_CHIP
    NEXUS_FrontendDevice_GetDefault4538OpenSettings(&st4538Settings2);

    /* Due to asynchronous initialization, all probing has to complete before the device open. */
    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
    gpioSettings.mode = NEXUS_GpioMode_eInput;
    gpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;
    gpioHandleInt4538B = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, EXT_4538_GPIO_IRQ_2, &gpioSettings);
    BDBG_ASSERT(NULL != gpioHandleInt4538B);
    st4538Settings2.gpioInterrupt = gpioHandleInt4538B;

    /* GPIO 110 is connected to the 4538B reset, so set it high to reset the second 4538 */
    /* Note that for A0/B0, we're actually touching 71 here, because of the rework */
    st4538Settings2.reset.enable = true;
    st4538Settings2.reset.pin = EXT_4538_GPIO_RESET_2;
    st4538Settings2.reset.type = NEXUS_GpioType_eStandard;
    st4538Settings2.reset.value = NEXUS_GpioValue_eHigh;

#if NEXUS_4538_USE_SPI
    st4538Settings2.i2cDevice = NULL;
    st4538Settings2.i2cAddr = I2C_4538_ADDRESS_2; /* used internally as an index */
    st4538Settings2.spiDevice = g_4538spi[2];
    st4538Settings2.spiAddr = SPI_4538_ADDRESS_2;
    st4538Settings2.diseqc.i2cDevice = pConfig->i2c[I2C_4538_INDEX_2];
#else
    st4538Settings2.i2cDevice = pConfig->i2c[I2C_4538_INDEX_2];
    st4538Settings2.i2cAddr = I2C_4538_ADDRESS_2;
#endif

#if NEXUS_4538_USE_SPI
    if (!NEXUS_Frontend_Probe4538(&st4538Settings2, &probeResults)) {
        if (probeResults.chip.familyId == 0x4538) {
            BDBG_MSG(("Found 4538B at 0x%02x",SPI_4538_ADDRESS_2));
        }
    }
    spiAddr2 = SPI_4538_ADDRESS_2;
#else
    BDBG_MSG(("Checking i2c: 0x%02x",I2C_4538_ADDRESS_2));
    if (!NEXUS_Frontend_Probe4538(&st4538Settings2, &probeResults)) {
        if (probeResults.chip.familyId == 0x4538) {
            i2cAddr2 = I2C_4538_ADDRESS_2;
            BDBG_MSG(("Found 4538B at 0x%02x",I2C_4538_ADDRESS_2));
        }
    } else {
#if NEXUS_PLATFORM_BYPASS_I2C_ADDRESS_SEARCH
        BDBG_ERR(("Unable to locate 4538B at 0x%02x",I2C_4538_ADDRESS_2));
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
#else
        int ix;
        for (ix=i2cAddr+1; ix<=0x6F; ix++) {
            BDBG_MSG(("Checking i2c: 0x%02x",ix));
            st4538Settings2.i2cAddr = ix;
            if (ix != I2C_4538_ADDRESS_2) {
                if (!NEXUS_Frontend_Probe4538(&st4538Settings2, &probeResults)) {
                    if (probeResults.chip.familyId == 0x4538) {
                        i2cAddr2 = ix;
                        BDBG_MSG(("Found 4538B at 0x%02x on %d",ix,I2C_4538_INDEX_2));
                        break;
                    }
                }
            }
        }
        if (i2cAddr2 == I2C_4538_ADDRESS_2) {
            BDBG_ERR(("Unable to locate 4538B"));
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
#endif
    }
#endif
#endif

#if NEXUS_4538_USE_SPI
    st4538Settings.spiAddr = spiAddr;
#else
    st4538Settings.i2cAddr = i2cAddr;
#endif
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
                BDBG_ERR(("Unable to open onboard 4538A tuner/demodulator %d",i));
            } else {
                BDBG_MSG(("Initialized 4538A[%d]",i));
                NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
                userParams.isMtsif = true;
                userParams.param1 = userParams.isMtsif ? channelSettings.channelNumber : NEXUS_InputBand_e0 + i;
                userParams.pParam2 = 0;
                NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
            }
        }
    }

#if !SKIP_SECOND_4538_CHIP
    /* Bring up second 4538 */
#if NEXUS_4538_USE_SPI
    st4538Settings2.spiAddr = spiAddr2;
#else
    st4538Settings2.i2cDevice = pConfig->i2c[I2C_4538_INDEX_2];
    st4538Settings2.i2cAddr = i2cAddr2;
#endif

    device = NEXUS_FrontendDevice_Open4538(1, &st4538Settings2);
    if (device) {
        for (i=0; i <  NUM_4538_CHANNELS_PER; i++)
        {
            unsigned j = i + NUM_4538_CHANNELS_PER;
            NEXUS_4538Settings channelSettings;
            channelSettings.device = device;
            channelSettings.channelNumber = i;
            pConfig->frontend[j] = NEXUS_Frontend_Open4538(&channelSettings);
            if ( NULL == (pConfig->frontend[j]) )
            {
                BDBG_ERR(("Unable to open onboard 4538B tuner/demodulator %d",i));
            } else {
                BDBG_MSG(("Initialized 4538B[%d](%d)",i,j));
                NEXUS_Frontend_GetUserParameters(pConfig->frontend[j], &userParams);
                userParams.isMtsif = true;
                userParams.param1 = userParams.isMtsif ? channelSettings.channelNumber : NEXUS_InputBand_e0 + j;
                userParams.pParam2 = 0;
                NEXUS_Frontend_SetUserParameters(pConfig->frontend[j], &userParams);
            }
        }
    }
#endif

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

    if(gpioHandleInt4538A)
    {
        NEXUS_Gpio_Close(gpioHandleInt4538A);
        gpioHandleInt4538A = NULL;
    }
    if(gpioHandle4538A)
    {
        NEXUS_Gpio_Close(gpioHandle4538A);
        gpioHandle4538A = NULL;
    }
    if(gpioHandleInt4538B)
    {
        NEXUS_Gpio_Close(gpioHandleInt4538B);
        gpioHandleInt4538B = NULL;
    }
    if(gpioHandle4538B)
    {
        NEXUS_Gpio_Close(gpioHandle4538B);
        gpioHandle4538B = NULL;
    }
#if NEXUS_4538_USE_SPI
    for (i=0; i<NEXUS_NUM_SPI_CHANNELS; i++)
    {
        if (g_4538spi[i])
        {
            NEXUS_Spi_Close(g_4538spi[i]);
            g_4538spi[i] = NULL;
        }
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
#if NEXUS_USE_7445_SV
#if BCHP_VER >= BCHP_VER_D0
    *pInputBand = NEXUS_InputBand_e0;
#else
    *pInputBand = NEXUS_InputBand_e5;
#endif
#elif NEXUS_USE_7445_DBS
#if BCHP_VER >= BCHP_VER_D0
    *pInputBand = NEXUS_InputBand_e0;
#else
    *pInputBand = NEXUS_InputBand_e2;
#endif
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





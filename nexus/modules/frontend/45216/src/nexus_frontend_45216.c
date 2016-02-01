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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*   API name: Frontend 45216
*    APIs to open, close, and setup initial settings for a BCM45216
*    Dual-Channel Satellite Tuner/Demodulator Device.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#include "nexus_frontend_module.h"
#include "nexus_frontend_sat.h"
#include "priv/nexus_transport_priv.h"
#include "priv/nexus_i2c_priv.h"
#include "priv/nexus_gpio_priv.h"
#include "priv/nexus_spi_priv.h"
#include "bsat.h"
#include "bdsq.h"
#include "bwfe.h"
#if NEXUS_HAS_FSK
#include "bfsk.h"
#endif
#include "bsat_45216.h"
#include "bwfe_45216.h"
#include "bdsq_45216.h"
#if NEXUS_HAS_FSK
#include "bfsk_45216.h"
#endif
#include "bhab.h"
#include "bhab_45216.h"
#include "bhab_45216_fw.h"
#include "bhab_satfe_img.h"
#include "priv/nexus_core_img.h"
#include "priv/nexus_core_img_id.h"
#include "bdbg.h"

BDBG_MODULE(nexus_frontend_45216);

BDBG_OBJECT_ID(NEXUS_45216Device);

/* set to 1 to enable L1 interrupt messages */
#define NEXUS_FRONTEND_DEBUG_IRQ 0

/* set to 1 to disable asynchronous initialization */
#define DISABLE_45216_ASYNC_INIT 0

typedef struct NEXUS_45216Device
{
    BDBG_OBJECT(NEXUS_45216Device)
    NEXUS_FrontendDevice45216OpenSettings settings;
    NEXUS_FrontendDevice *pGenericDeviceHandle;
    NEXUS_SatDeviceHandle satDevice;
    bool isExternal;
    uint32_t numChannels;   /* prototype to match BSAT_GetTotalChannels */
    uint8_t numAdc;         /* prototype to match BSAT_GetAdcSelect */
    BKNI_EventHandle isrEvent;
    NEXUS_EventCallbackHandle isrCallback;
    BSAT_ChannelHandle satChannels[NEXUS_45216_MAX_FRONTEND_CHANNELS];
    NEXUS_FrontendHandle handles[NEXUS_45216_MAX_FRONTEND_CHANNELS];
    BWFE_ChannelInfo wfeInfo;
    uint32_t numDsqChannels;   /* prototype to match BDSQ_GetTotalChannels */
    int wfeMap[NEXUS_45216_MAX_FRONTEND_CHANNELS];
    uint8_t A8299_control;
#if NEXUS_HAS_FSK
    uint32_t numFskChannels;
    BFSK_Handle fskHandle;
    BFSK_ChannelHandle fskChannels[NEXUS_45216_MAX_FRONTEND_CHANNELS];
    NEXUS_EventCallbackHandle ftmEventCallback[NEXUS_45216_MAX_FRONTEND_CHANNELS];
#endif
    NEXUS_GpioHandle gpioHandle;
    NEXUS_ThreadHandle deviceOpenThread;
    BIMG_Interface imgInterface;
} NEXUS_45216Device;

#if 0
static BLST_S_HEAD(devList, NEXUS_45216Device) g_deviceList = BLST_S_INITIALIZER(g_deviceList);
#endif

static void NEXUS_Frontend_P_45216_CloseCallback(NEXUS_FrontendHandle handle, void *pParam);
static void NEXUS_Frontend_P_45216_DestroyDevice(void *handle);

static NEXUS_Error NEXUS_Frontend_P_45216_TuneSatellite(void *handle, const NEXUS_FrontendSatelliteSettings *pSettings);
static void NEXUS_Frontend_P_45216_Untune(void *handle);
static NEXUS_Error NEXUS_Frontend_P_45216_ReapplyTransportSettings(void *handle);

static BDSQ_ChannelHandle NEXUS_Frontend_P_45216_GetDiseqcChannelHandle(void *handle, int index);
static NEXUS_Error NEXUS_Frontend_P_45216_SetVoltage(void *pDevice, NEXUS_FrontendDiseqcVoltage voltage);

static void NEXUS_Frontend_P_45216_GetDefaultDiseqcSettings(void *pDevice, BDSQ_ChannelSettings *settings);
#if NEXUS_HAS_FSK
static BFSK_ChannelHandle NEXUS_Frontend_P_45216_GetFskChannelHandle(void *handle, int index);
static void NEXUS_Frontend_P_45216_FtmEventCallback(void *context);
#endif

static NEXUS_Error NEXUS_FrontendDevice_P_Get45216Capabilities(void *handle, NEXUS_FrontendSatelliteCapabilities *pCapabilities);
static NEXUS_Error NEXUS_Frontend_P_Get45216RuntimeSettings(void *handle, NEXUS_FrontendSatelliteRuntimeSettings *pSettings);
static NEXUS_Error NEXUS_Frontend_P_Set45216RuntimeSettings(void *handle, const NEXUS_FrontendSatelliteRuntimeSettings *pSettings);

#if 0
static void NEXUS_Frontend_P_45216_RestoreLnaCallback(NEXUS_FrontendHandle handle, void *pParam);
static BERR_Code NEXUS_Frontend_P_45216_I2cRead(void * i2cHandle, uint16_t chipAddr, uint32_t subAddr, uint8_t *pData, size_t length);
static BERR_Code NEXUS_Frontend_P_45216_I2cWrite(void * i2cHandle, uint16_t chipAddr, uint32_t subAddr, const uint8_t *pData, size_t length);
static BERR_Code NEXUS_Frontend_P_45216_I2cReadNoAddr(void * context, uint16_t chipAddr, uint8_t *pData, size_t length);
static BERR_Code NEXUS_Frontend_P_45216_I2cWriteNoAddr(void * context, uint16_t chipAddr, const uint8_t *pData, size_t length);
#endif
static NEXUS_Error NEXUS_FrontendDevice_P_45216_GetStatus(void * handle, NEXUS_FrontendDeviceStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_45216_GetSatelliteAgcStatus(void *handle, NEXUS_FrontendSatelliteAgcStatus *pStatus);

static void NEXUS_FrontendDevice_P_45216_GetCapabilities(void * handle, NEXUS_FrontendDeviceCapabilities *pCapabilities);

static void NEXUS_Frontend_P_45216_ClearSpectrumCallbacks(void *handle);

static NEXUS_Error NEXUS_Frontend_P_45216_ReadRegister(void *handle, unsigned address, uint32_t *pValue);
static NEXUS_Error NEXUS_Frontend_P_45216_WriteRegister(void *handle, unsigned address, uint32_t value);

static NEXUS_Error NEXUS_FrontendDevice_P_45216_Standby(void * handle, const NEXUS_StandbySettings *pSettings);
static NEXUS_Error NEXUS_Frontend_P_45216_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings);

void NEXUS_FrontendDevice_GetDefault45216OpenSettings(NEXUS_FrontendDevice45216OpenSettings *pSettings)
{
    int i;
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    for (i=0; i < NEXUS_MAX_MTSIF; i++) {
        pSettings->mtsif[i].enabled = true;
    }
}

/***************************************************************************
Summary:
    Enable/Disable interrupts for a 45216 device
 ***************************************************************************/
static void NEXUS_Frontend_P_45216_IsrControl_isr(bool enable, void *pParam)
{
    int isrNumber = (int)pParam;

    if ( enable )
    {
#if NEXUS_FRONTEND_DEBUG_IRQ
        BDBG_MSG(("Enable 45216 Interrupt %u", isrNumber));
#endif
        NEXUS_Core_EnableInterrupt_isr(isrNumber);
    }
    else
    {
#if NEXUS_FRONTEND_DEBUG_IRQ
        BDBG_MSG(("Disable 45216 Interrupt %u", isrNumber));
#endif
        NEXUS_Core_DisableInterrupt_isr(isrNumber);
    }
}

/***************************************************************************
Summary:
    Enable/Disable gpio interrupts for a 45216 device
 ***************************************************************************/
static void NEXUS_Frontend_P_45216_GpioIsrControl_isr(bool enable, void *pParam)
{
    NEXUS_GpioHandle gpioHandle = (NEXUS_GpioHandle)pParam;

#if NEXUS_FRONTEND_DEBUG_IRQ
    BDBG_MSG(("%s 45216 Gpio Interrupt %p", enable ? "Enable" : "Disable", gpioHandle));
#endif
    NEXUS_Gpio_SetInterruptEnabled_isr(gpioHandle, enable);
}

static void NEXUS_Frontend_P_45216_IsrCallback(void *pParam)
{
    NEXUS_45216Device *pDevice = (NEXUS_45216Device *)pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_45216Device);
#if NEXUS_FRONTEND_DEBUG_IRQ
    BDBG_MSG(("45216 ISR Callback (hab: %p)",pDevice->satDevice->habHandle));
#endif
    BHAB_ProcessInterruptEvent(pDevice->satDevice->habHandle);
}

static void NEXUS_Frontend_P_45216_L1_isr(void *param1, int param2)
{
    NEXUS_45216Device *pDevice = (NEXUS_45216Device *)param1;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_45216Device);
    BSTD_UNUSED(param2);
#if NEXUS_FRONTEND_DEBUG_IRQ
    BDBG_MSG(("45216 L1 ISR (hab: %p)",pDevice->satDevice->habHandle));
#endif

    BHAB_HandleInterrupt_isr(pDevice->satDevice->habHandle);
#if NEXUS_FRONTEND_DEBUG_IRQ
    BDBG_MSG(("Done: 45216 L1 ISR (hab: %p)",pDevice->satDevice->habHandle));
#endif
}

typedef struct NEXUS_FrontendDevice_P_45216_InitSettings {
    BHAB_Settings habSettings;
    BSAT_Settings satSettings;
    BWFE_Settings wfeSettings;
    BDSQ_Settings dsqSettings;
#if NEXUS_HAS_FSK
    BFSK_Settings fskSettings;
#endif
} NEXUS_FrontendDevice_P_45216_InitSettings;

static NEXUS_Error NEXUS_FrontendDevice_P_Init45216_PreInitAP(NEXUS_45216Device *pDevice)
{
    NEXUS_FrontendDevice_P_45216_InitSettings *pInitSettings;
    BHAB_Handle habHandle;
    BSAT_Handle satHandle;
    BWFE_Handle wfeHandle;
    BDSQ_Handle dsqHandle;
#if NEXUS_HAS_FSK
    BFSK_Handle fskHandle;
#endif
    unsigned i;
    void *regHandle;
    BERR_Code errCode;
    NEXUS_FrontendDevice45216OpenSettings *pSettings = &pDevice->settings;

    pInitSettings = (NEXUS_FrontendDevice_P_45216_InitSettings *)BKNI_Malloc(sizeof(NEXUS_FrontendDevice_P_45216_InitSettings));
    if (!pInitSettings) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto err;
    }
    BKNI_Memset(pInitSettings, 0, sizeof(NEXUS_FrontendDevice_P_45216_InitSettings));

    BHAB_45216_GetDefaultSettings(&pInitSettings->habSettings);
    {
#if NEXUS_MODE_driver
        if (Nexus_Core_P_Img_Create(NEXUS_CORE_IMG_ID_FRONTEND_45216, &pInitSettings->habSettings.pImgContext, &pDevice->imgInterface ) == NEXUS_SUCCESS) {
            pInitSettings->habSettings.pImgInterface = &pDevice->imgInterface;
        } else {
            pInitSettings->habSettings.pImgContext = NULL;
        }
#else
        pInitSettings->habSettings.pImgInterface = &BHAB_SATFE_IMG_Interface;
        pInitSettings->habSettings.pImgContext = &BHAB_45216_IMG_Context;
#endif
    }

    if (pSettings->reset.enable) {
        NEXUS_GpioSettings gpioSettings;
        if (pDevice->gpioHandle) {
            NEXUS_Gpio_Close(pDevice->gpioHandle);
        }
        BDBG_MSG(("Setting GPIO %d high",pSettings->reset.pin));
        NEXUS_Gpio_GetDefaultSettings(pSettings->reset.type, &gpioSettings);
        gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
        gpioSettings.value = pSettings->reset.value;
        gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
        gpioSettings.interrupt.callback = NULL;
        pDevice->gpioHandle = NEXUS_Gpio_Open(pSettings->reset.type, pSettings->reset.pin, &gpioSettings);
        BDBG_ASSERT(NULL != pDevice->gpioHandle);

        BKNI_Sleep(500);
    }

    if (pSettings->spiDevice) {
        BDBG_MSG(("Configuring for SPI"));
        pInitSettings->habSettings.chipAddr = pSettings->spiAddr;
        pInitSettings->habSettings.isSpi = true;
        regHandle = (void *)NEXUS_Spi_GetRegHandle(pSettings->spiDevice);
    } else if (pSettings->i2cDevice) {
        BDBG_MSG(("Configuring for I2C"));
        pInitSettings->habSettings.chipAddr = pSettings->i2cAddr;
        pInitSettings->habSettings.isSpi = false;
        regHandle = (void *)NEXUS_I2c_GetRegHandle(pSettings->i2cDevice, NEXUS_MODULE_SELF);

        BDBG_MSG(("i2cDevice: %p",pSettings->i2cDevice));
        BDBG_MSG(("BREG_I2C_Handle: %p",regHandle));
        {
            NEXUS_45216ProbeResults results;
            NEXUS_Frontend_Probe45216(pSettings, &results);
            BDBG_MSG(("chipid: %x",results.chip.familyId));
        }
    } else {
        regHandle = NULL;
    }
    pInitSettings->habSettings.isMtsif = true;
    if(pSettings->isrNumber) {
        BDBG_MSG(("Configuring for external interrupt"));
        pInitSettings->habSettings.interruptEnableFunc = NEXUS_Frontend_P_45216_IsrControl_isr;
        pInitSettings->habSettings.interruptEnableFuncParam = (void*)pSettings->isrNumber;
    }
    else if(pSettings->gpioInterrupt){
        BDBG_MSG(("Configuring for GPIO interrupt"));
        pInitSettings->habSettings.interruptEnableFunc = NEXUS_Frontend_P_45216_GpioIsrControl_isr;
        pInitSettings->habSettings.interruptEnableFuncParam = (void*)pSettings->gpioInterrupt;
    }
    BDBG_ASSERT(regHandle);

    BDBG_MSG(("Calling BHAB_Open"));
    errCode = BHAB_Open(&habHandle, regHandle, &pInitSettings->habSettings);
    BDBG_MSG(("Calling BHAB_Open...Done: hab: %p",(void *)habHandle));
    if (errCode) { BERR_TRACE(NEXUS_OS_ERROR); goto err; }

    if(pSettings->isrNumber) {
        BDBG_MSG(("Connecting external interrupt"));
        errCode = NEXUS_Core_ConnectInterrupt(pSettings->isrNumber,
                                             NEXUS_Frontend_P_45216_L1_isr,
                                             (void *)pDevice,
                                             0);
        if ( errCode != BERR_SUCCESS )
        {
            errCode = BERR_TRACE(errCode);
            goto err;
        }
    }
    else if(pSettings->gpioInterrupt){
        BDBG_MSG(("Connecting GPIO interrupt"));
        NEXUS_Gpio_SetInterruptCallback_priv(pSettings->gpioInterrupt,
                                             NEXUS_Frontend_P_45216_L1_isr,
                                             (void *)pDevice,
                                             0);
    }

    errCode = BSAT_45216_GetDefaultSettings(&pInitSettings->satSettings);
    if (errCode) { BERR_TRACE(NEXUS_OS_ERROR); goto err; }

    errCode = BSAT_Open(&satHandle, g_pCoreHandles->chp, habHandle, NULL, &pInitSettings->satSettings); /* CHP and INT are unused by SAT */
    if (errCode) { BERR_TRACE(NEXUS_OS_ERROR); goto err; }

    errCode = BWFE_45216_GetDefaultSettings(&pInitSettings->wfeSettings);
    if (errCode) { BERR_TRACE(NEXUS_OS_ERROR); goto err; }

    errCode = BWFE_Open(&wfeHandle, g_pCoreHandles->chp, habHandle, NULL, &pInitSettings->wfeSettings); /* CHP and INT are unused by WFE */
    if (errCode) { BERR_TRACE(NEXUS_OS_ERROR); goto err; }

    errCode = BDSQ_45216_GetDefaultSettings(&pInitSettings->dsqSettings);
    if (errCode) { BERR_TRACE(NEXUS_OS_ERROR); goto err; }

    errCode = BDSQ_Open(&dsqHandle, g_pCoreHandles->chp, habHandle, NULL, &pInitSettings->dsqSettings); /* CHP and INT are unused by DSQ */
    if (errCode) { BERR_TRACE(NEXUS_OS_ERROR); goto err; }

#if NEXUS_HAS_FSK
    errCode = BFSK_45216_GetDefaultSettings(&pInitSettings->fskSettings);
    if (errCode) { BERR_TRACE(NEXUS_OS_ERROR); goto err; }

    errCode = BFSK_Open(&fskHandle, g_pCoreHandles->chp, habHandle, NULL, &pInitSettings->fskSettings);
    if (errCode) { BERR_TRACE(NEXUS_OS_ERROR); goto err; }
#endif

#if NEXUS_HAS_FSK
    pDevice->fskHandle = fskHandle;
#endif

    pDevice->satDevice->habHandle = habHandle;
    pDevice->satDevice->satHandle = satHandle;
    pDevice->satDevice->wfeHandle = wfeHandle;
    pDevice->satDevice->dsqHandle = dsqHandle;

    /* Determine number of channels -- they will be opened later */
    BSAT_GetTotalChannels(pDevice->satDevice->satHandle, &pDevice->numChannels);
    BDBG_MSG(("frontend has %d channels",pDevice->numChannels));
    if ( pDevice->numChannels > NEXUS_45216_MAX_FRONTEND_CHANNELS )
    {
        BDBG_WRN(("This 45216 device supports more than the expected number of channels. Unexpected channels will not be initialized."));
    }
    pDevice->satDevice->numChannels = pDevice->numChannels;

    /* Open all channels prior to InitAp */
    for ( i = 0; i < pDevice->numChannels; i++ )
    {
        BSAT_ChannelSettings bsatChannelSettings;
        BSAT_GetChannelDefaultSettings(pDevice->satDevice->satHandle, i, &bsatChannelSettings);
        errCode = BSAT_OpenChannel(pDevice->satDevice->satHandle, &pDevice->satChannels[i], i, &bsatChannelSettings);
        if ( errCode ) {
            BDBG_ERR(("Unable to open channel %d", i));
            errCode = BERR_TRACE(errCode);
            goto err;
        }
    }

    /* Determine number of inputs */
    errCode = BWFE_GetTotalChannels(wfeHandle, &pDevice->wfeInfo);
    BDBG_ASSERT(!errCode);
    pDevice->numAdc = pDevice->wfeInfo.numChannels;
    pDevice->satDevice->numWfe = pDevice->numAdc;

    BKNI_Free(pInitSettings);

    return NEXUS_SUCCESS;
err:
    return NEXUS_UNKNOWN;
}

static NEXUS_Error NEXUS_FrontendDevice_P_Init45216_InitAP(NEXUS_45216Device *pDevice)
{
    BERR_Code errCode;
    const uint8_t *fw = NULL;

    BDBG_MSG(("NEXUS_FrontendDevice_P_Init45216_InitAP"));

    BDBG_WRN(("Initializing 45216 Frontend core..."));
    /* Initialize the acquisition processor */
#if !NEXUS_MODE_driver
    fw = bcm45216_ap_image;
#endif
    errCode = BHAB_InitAp(pDevice->satDevice->habHandle, fw);
    if ( errCode ) {
        BDBG_ERR(("Device initialization failed..."));

        errCode = BERR_TRACE(errCode);
        goto err;
    }
    BDBG_WRN(("Initializing 45216 core... Done"));

    {
        BERR_Code errCode;
        BFEC_VersionInfo info;
        errCode = BSAT_GetVersionInfo(pDevice->satDevice->satHandle, &info);
        if (errCode) BERR_TRACE(errCode);
        else {
            pDevice->satDevice->type.chip.version.buildId = info.buildId;
            pDevice->satDevice->type.chip.version.buildType = info.buildType;
            pDevice->satDevice->type.chip.version.major = info.majorVersion;
            pDevice->satDevice->type.chip.version.minor = info.minorVersion;
        }
    }

    {
        BERR_Code errCode;
        BFEC_SystemVersionInfo svInfo;
        /* populate firmware version */
        errCode = BHAB_GetVersionInfo(pDevice->satDevice->habHandle, &svInfo);
        if (errCode) BERR_TRACE(errCode);
        else {
            pDevice->satDevice->type.firmwareVersion.buildId = svInfo.firmware.buildId;
            pDevice->satDevice->type.firmwareVersion.buildType = svInfo.firmware.buildType;
            pDevice->satDevice->type.firmwareVersion.major = svInfo.firmware.majorVersion;
            pDevice->satDevice->type.firmwareVersion.minor = svInfo.firmware.minorVersion;
        }
    }

    BDBG_MSG(("NEXUS_FrontendDevice_P_Init45216_InitAP: done"));
    return NEXUS_SUCCESS;
err:
    return NEXUS_UNKNOWN;
}

static NEXUS_Error NEXUS_FrontendDevice_P_Init45216_PostInitAP(NEXUS_45216Device *pDevice)
{
    unsigned i;
    BERR_Code errCode;
    NEXUS_FrontendDevice45216OpenSettings *pSettings = &pDevice->settings;

    BDBG_MSG(("NEXUS_FrontendDevice_P_Init45216_PostInitAP"));

    BDBG_MSG(("Opening WFE channels"));
    /* Open WFE Channels */
    for (i=0; i < pDevice->wfeInfo.numChannels; i++) {
        BWFE_ChannelSettings wfeChannelSettings;
        BWFE_GetChannelDefaultSettings(pDevice->satDevice->wfeHandle, i, &wfeChannelSettings);
        errCode = BWFE_OpenChannel(pDevice->satDevice->wfeHandle, &pDevice->satDevice->wfeChannels[i], i, &wfeChannelSettings);
        if ( errCode ) {
            BDBG_ERR(("Unable to open wfe channel %d", i));
            errCode = BERR_TRACE(errCode);
            goto err;
        }
        errCode = BWFE_GetWfeReadyEventHandle(pDevice->satDevice->wfeChannels[i], &pDevice->satDevice->wfeReadyEvent[i]);
        if ( errCode ) {
            BDBG_ERR(("Unable to retrieve ready event for wfe channel %d", i));
            errCode = BERR_TRACE(errCode);
            goto err;
        }
    }

    BDBG_MSG(("Opening DSQ channels"));
    /* Open DSQ Channels */
    BDSQ_GetTotalChannels(pDevice->satDevice->dsqHandle, &pDevice->numDsqChannels);
    for (i=0; i < pDevice->numDsqChannels; i++) {
        BDSQ_ChannelSettings dsqChannelSettings;
        BDSQ_GetChannelDefaultSettings(pDevice->satDevice->dsqHandle, i, &dsqChannelSettings);
        errCode = BDSQ_OpenChannel(pDevice->satDevice->dsqHandle, &pDevice->satDevice->dsqChannels[i], i, &dsqChannelSettings);
        if ( errCode ) {
            BDBG_ERR(("Unable to open dsq channel %d", i));
            errCode = BERR_TRACE(errCode);
            goto err;
        }
    }

    BDSQ_Reset(pDevice->satDevice->dsqHandle);
    for (i=0; i < pDevice->numDsqChannels; i++) {
        BDSQ_PowerUpChannel(pDevice->satDevice->dsqChannels[i]);
    }

#ifdef NEXUS_HAS_FSK
    BDBG_MSG(("Opening FSK channels"));
    /* Open FSK Channels */
    BFSK_GetTotalChannels(pDevice->fskHandle, &pDevice->numFskChannels);

    for (i=0; i < pDevice->numFskChannels; i++) {
        BFSK_ChannelSettings fskChannelSettings;

        BFSK_GetChannelDefaultSettings(pDevice->fskHandle, i, &fskChannelSettings);
        errCode = BFSK_OpenChannel(pDevice->fskHandle, &pDevice->fskChannels[i], i, &fskChannelSettings);
        if ( errCode ) {
            BDBG_ERR(("Unable to open fsk channel %d", i));
            errCode = BERR_TRACE(errCode);
            goto err;
        }
    }

    {
        BKNI_EventHandle event;

        /* hook up the FTM data ready callback*/
        for(i=0; i<pDevice->numFskChannels; i++){
            errCode = BFSK_GetRxEventHandle(pDevice->fskChannels[i], &event);
            if (errCode) {
                errCode = BERR_TRACE(errCode);
                goto err;
            }

            pDevice->ftmEventCallback[i] = NEXUS_RegisterEvent(event, NEXUS_Frontend_P_45216_FtmEventCallback, pDevice);
            if (!pDevice->ftmEventCallback[i]) {
                errCode = BERR_TRACE(NEXUS_UNKNOWN);
                goto err;
            }
        }
    }
#endif

    BDBG_MSG(("Configuring MTSIF"));
    /* Configure MTSIF. */
    if (pSettings->mtsif[0].enabled || pSettings->mtsif[1].enabled) {
        BERR_Code e;
        uint32_t val = 1;
        e = BHAB_ReadRegister(pDevice->satDevice->habHandle, 0x06920790, &val);
        if (e) BERR_TRACE(e);
        val &= 0xFFFFFFFE;
        BHAB_WriteRegister(pDevice->satDevice->habHandle, 0x06920790, &val);
    }

#if NEXUS_HAS_MXT
    if (pSettings->mtsif[0].enabled || pSettings->mtsif[1].enabled) {
        /* open MXT */
        BERR_Code rc;
        BMXT_Settings mxtSettings;
        BDBG_MSG(("NEXUS_FrontendDevice_Open45216: configuring MXT"));

        BMXT_45216_GetDefaultSettings(&mxtSettings);
        mxtSettings.hHab = pDevice->satDevice->habHandle;

        for (i=0; i<BMXT_NUM_MTSIF; i++) {
            mxtSettings.MtsifTxCfg[i].Enable = true;
            NEXUS_Module_Lock(g_NEXUS_frontendModuleSettings.transport);
            mxtSettings.MtsifTxCfg[i].Encrypt = NEXUS_TransportModule_P_IsMtsifEncrypted();
            NEXUS_Module_Unlock(g_NEXUS_frontendModuleSettings.transport);
        }

        mxtSettings.MtsifTxCfg[0].TxClockPolarity = 0;
        mxtSettings.MtsifTxCfg[1].TxClockPolarity = 0;

        rc = BMXT_Open(&pDevice->pGenericDeviceHandle->mtsifConfig.mxt, g_pCoreHandles->chp, g_pCoreHandles->reg, &mxtSettings);
        if (rc!=BERR_SUCCESS) goto err;
        rc = NEXUS_Frontend_P_InitMtsifConfig(&pDevice->pGenericDeviceHandle->mtsifConfig, &mxtSettings);
        if (rc!=BERR_SUCCESS) goto err;

        if (!pSettings->mtsif[0].enabled) {
            BMXT_Handle mxt = pDevice->pGenericDeviceHandle->mtsifConfig.mxt;
            unsigned numParsers = BMXT_GetNumResources(mxtSettings.chip, mxtSettings.chipRev, BMXT_ResourceType_eParser);
            for (i=0; i<numParsers; i++) {
                BMXT_ParserConfig pConfig;
                BMXT_GetParserConfig(mxt, i, &pConfig);
                pConfig.mtsifTxSelect = 1;
                BMXT_SetParserConfig(mxt, i, &pConfig);
            }
        }
    }
#endif

    for (i=0; i < 2; i++) {
        uint32_t val;
        uint32_t clockAddr;
        BERR_Code e;

        clockAddr = i == 0 ? 0x06920794 : 0x0692078c;

        if (!pSettings->mtsif[i].enabled) {
            e = BHAB_ReadRegister(pDevice->satDevice->habHandle, clockAddr, &val);
            if (e) BERR_TRACE(e);
            val &= 0xFFFFFFFE;
            val |= 1;
            BHAB_WriteRegister(pDevice->satDevice->habHandle, clockAddr, &val);
        } else {
            e = BHAB_ReadRegister(pDevice->satDevice->habHandle, clockAddr, &val);
            if (!e && (val & 1)) {
                val &= 0xFFFFFFFE;
                BHAB_WriteRegister(pDevice->satDevice->habHandle, clockAddr, &val);
            }
            if (pSettings->mtsif[i].clockRate != 0) {
                unsigned clockRate = pSettings->mtsif[i].clockRate;
                unsigned divider;

                e = BHAB_ReadRegister(pDevice->satDevice->habHandle, clockAddr, &val);
                switch (clockRate) {
                case 81000000: /* 81 MHz. (hardware default) */
                    divider = 32; break;
                case 90000000: /* 90 MHz. */
                    divider = 29; break;
                case 93000000: /* 93 MHz. */
                case 92500000: /* 92.5 MHz. (actual value) */
                    divider = 29; break;
                case 95000000: /* 95.5 MHz. */
                case 95500000:
                case 96000000: /* 96 MHz. (actual value) */
                    divider = 27; break;
                case 98000000: /* 98.2 MHz. */
                case 98100000:
                case 98200000:
                case 100000000: /* 101.25 MHz. */
                case 101000000: /* 101.25 MHz. */
                case 101250000:
                case 101500000:
                case 99700000: /* 99.68 MHz (actual value) */
                    divider = 26; break;
                case 104000000: /* 104.5 MHz. Also accept 104 Mhz. */
                case 104500000:
                case 103680000: /* 103.68MHz (actual value) */
                    divider = 25; break;
                case 108000000: /* 108 MHz. */
                    divider = 24; break;
                case 135000000: /* 135 MHz. */
                case 136000000: /* 135 MHz. */
                case 136400000: /* 136.4 MHz. (actual value) */
                    divider = 19; break;
                default:
                    BDBG_WRN(("NEXUS_FrontendDevice_Open45216: Unrecognized rate (MTSIF%d): %u, defaulting to 81MHz.", i, clockRate));
                    divider = 32;
                    break;
                }
                val &= ~0x000001fe;
                val |= (divider << 1);
                BHAB_WriteRegister(pDevice->satDevice->habHandle, clockAddr, &val);
            }

            if (pSettings->mtsif[i].driveStrength != 0) {
                unsigned driveStrength = pSettings->mtsif[i].driveStrength;
                unsigned str;
                uint32_t driveAddr;

                driveAddr = i == 0 ? 0x06920460 : 0x06920464;

                e = BHAB_ReadRegister(pDevice->satDevice->habHandle, driveAddr, &val);

                switch (driveStrength) {
                case 2:
                    str = 0; break;
                case 4:
                    str = 1; break;
                case 6:
                    str = 2; break;
                case 8: /* default/reset */
                    str = 3; break;
                case 10:
                    str = 4; break;
                case 12:
                    str = 5; break;
                case 14:
                    str = 6; break;
                case 16:
                    str = 7; break;
                default:
                    BDBG_WRN(("NEXUS_FrontendDevice_Open45216: Unrecognized drive strength (MTSIF%d): %u, defaulting to 8 mA.", i, driveStrength));
                    str = 3; break;
                }
                val &= 0xFFFFFFF8;
                val |= str;
                BHAB_WriteRegister(pDevice->satDevice->habHandle, driveAddr, &val);
            }
        }
    }

    if (!pSettings->mtsif[0].enabled && !pSettings->mtsif[1].enabled) {
        uint32_t val;
        BHAB_ReadRegister(pDevice->satDevice->habHandle, 0x06920790, &val);
        val &= 0xFFFFFFFE;
        val |= 1;
        BHAB_WriteRegister(pDevice->satDevice->habHandle, 0x06920790, &val);
    }

    pDevice->pGenericDeviceHandle->openPending = false;

    return NEXUS_SUCCESS;
err:
    return NEXUS_UNKNOWN;
}

/* Helper function for asynchronous initialization thread */
static void NEXUS_Frontend_45216_P_OpenDeviceThread(void *arg) {
    NEXUS_45216Device *pDevice= (NEXUS_45216Device *)arg;
    NEXUS_Error errCode;

    errCode = NEXUS_FrontendDevice_P_Init45216_InitAP(pDevice);
    if (errCode) goto init_error;
    errCode = NEXUS_FrontendDevice_P_Init45216_PostInitAP(pDevice);
    if (errCode) goto init_error;

    return;

init_error:
    BDBG_ERR(("45216(%p) initialization failed...",pDevice));
    BKNI_EnterCriticalSection();
    pDevice->pGenericDeviceHandle->openPending = false;
    pDevice->pGenericDeviceHandle->openFailed = true;
    BKNI_LeaveCriticalSection();
}

static NEXUS_Error NEXUS_Frontend_45216_P_DelayedInitialization(NEXUS_FrontendDeviceHandle handle)
{
    NEXUS_FrontendDevice *pFrontendDevice = (NEXUS_FrontendDevice *)handle;
    NEXUS_45216Device *pDevice = (NEXUS_45216Device *)pFrontendDevice->pDevice;
    NEXUS_Error errCode;

    BDBG_MSG(("NEXUS_Frontend_45216_P_DelayedInitialization"));

    BDBG_MSG(("Connecting interrupt"));
    /* Successfully opened the 45216.  Connect interrupt */
    BHAB_GetInterruptEventHandle(pDevice->satDevice->habHandle, &pDevice->isrEvent);
    pDevice->isrCallback = NEXUS_RegisterEvent(pDevice->isrEvent, NEXUS_Frontend_P_45216_IsrCallback, pDevice);
    if ( NULL == pDevice->isrCallback )
    {
        errCode = BERR_TRACE(BERR_OS_ERROR);
        goto err;
    }

    return NEXUS_SUCCESS;

err:
    return errCode;
}

static NEXUS_Error NEXUS_FrontendDevice_P_Init45216(NEXUS_45216Device *pDevice)
{
    NEXUS_Error errCode;

    errCode = NEXUS_FrontendDevice_P_Init45216_PreInitAP(pDevice);
    if (!errCode)
        errCode = NEXUS_FrontendDevice_P_Init45216_InitAP(pDevice);
    if (!errCode)
        errCode = NEXUS_FrontendDevice_P_Init45216_PostInitAP(pDevice);

    if (!errCode)
        errCode = NEXUS_Frontend_45216_P_DelayedInitialization(pDevice->pGenericDeviceHandle);

    return errCode;
}

NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_Open45216(unsigned index, const NEXUS_FrontendDevice45216OpenSettings *pSettings)
{
    NEXUS_FrontendDevice *pFrontendDevice = NULL;
    NEXUS_45216Device *pDevice=NULL;

    BSTD_UNUSED(index);

    /* Check is maintained in case we need to introduce a list of devices later. */
    if (pDevice == NULL) {
        NEXUS_FrontendSatDeviceSettings satDeviceSettings;

        BERR_Code errCode;

        BDBG_MSG(("Opening new 45216 device"));

        pFrontendDevice = BKNI_Malloc(sizeof(*pFrontendDevice));
        if (NULL == pFrontendDevice) { BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err; }
        BKNI_Memset(pFrontendDevice, 0, sizeof(*pFrontendDevice));

        pDevice = BKNI_Malloc(sizeof(*pDevice));
        if (NULL == pDevice) { BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err; }
        BKNI_Memset(pDevice, 0, sizeof(*pDevice));
        BDBG_OBJECT_SET(pDevice, NEXUS_45216Device);
        pDevice->settings = *pSettings;
        pDevice->pGenericDeviceHandle = pFrontendDevice;
        pDevice->A8299_control = 0x88; /* 13v, 13v default */

        NEXUS_Frontend_P_Sat_GetDefaultDeviceSettings(&satDeviceSettings);

        pDevice->satDevice = NEXUS_Frontend_P_Sat_Create_Device(&satDeviceSettings);
        if (pDevice->satDevice == NULL) { BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err; }

        errCode = NEXUS_FrontendDevice_P_Init45216_PreInitAP(pDevice);
        if (errCode) goto err;

        pFrontendDevice->delayedInit = NEXUS_Frontend_45216_P_DelayedInitialization;
        pFrontendDevice->delayedInitializationRequired = true;

        BKNI_EnterCriticalSection();
        pDevice->pGenericDeviceHandle->openPending = true;
        pDevice->pGenericDeviceHandle->openFailed = false;
        BKNI_LeaveCriticalSection();

#if DISABLE_45216_ASYNC_INIT
        NEXUS_Frontend_45216_P_OpenDeviceThread(pDevice);
#else
        {
            NEXUS_ThreadSettings thread_settings;
            NEXUS_Thread_GetDefaultSettings(&thread_settings);
            thread_settings.priority = 0;
            pDevice->deviceOpenThread = NEXUS_Thread_Create("deviceOpenThread",
                                                            NEXUS_Frontend_45216_P_OpenDeviceThread,
                                                            (void*)pDevice,
                                                            &thread_settings);
        }
#endif

    }

    pFrontendDevice->pDevice = pDevice;
    pFrontendDevice->familyId = 0x45216;
    pFrontendDevice->getCapabilities = NEXUS_FrontendDevice_P_45216_GetCapabilities;
    pFrontendDevice->application = NEXUS_FrontendDeviceApplication_eSatellite;
    pFrontendDevice->getStatus = NEXUS_FrontendDevice_P_45216_GetStatus;
    pFrontendDevice->close = NEXUS_Frontend_P_45216_DestroyDevice;
    pFrontendDevice->getSatelliteCapabilities = NEXUS_FrontendDevice_P_Get45216Capabilities;

    pFrontendDevice->mode = NEXUS_StandbyMode_eOn;
    pFrontendDevice->standby = NEXUS_FrontendDevice_P_45216_Standby;

    pFrontendDevice->nonblocking.getCapabilities = true; /* does not require init complete to fetch number of demods */
    pFrontendDevice->nonblocking.getSatelliteCapabilities = true; /* does not require init complete to fetch number of demods */

    BDBG_MSG(("NEXUS_FrontendDevice_Open45216: returning %p",pFrontendDevice));
    return pFrontendDevice;

err:
    if (pDevice)
        NEXUS_Frontend_P_45216_DestroyDevice(pDevice);
    return NULL;
}

static void NEXUS_Frontend_P_Uninit45216(NEXUS_45216Device *pDevice)
{
    unsigned i;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_45216Device);

    BDBG_MSG(("Closing 45216 device %p handles", pDevice));

    if (pDevice->deviceOpenThread)
        NEXUS_Thread_Destroy(pDevice->deviceOpenThread);
    pDevice->deviceOpenThread = NULL;

#if NEXUS_HAS_MXT
    if (pDevice->pGenericDeviceHandle->mtsifConfig.mxt) {
        BMXT_Close(pDevice->pGenericDeviceHandle->mtsifConfig.mxt);
        pDevice->pGenericDeviceHandle->mtsifConfig.mxt = NULL;
    }
#endif

#if 0
    for ( i = 0; i < pDevice->numAdc; i++ )
    {
        if (pDevice->diseqcCallbackEvents[i]) {
            NEXUS_UnregisterEvent(pDevice->diseqcCallbackEvents[i]);
        }
    }
#endif

#if NEXUS_HAS_FSK
    if (pDevice->fskHandle) {
        for (i=0; i < pDevice->numFskChannels; i++)
        {
            if(pDevice->ftmEventCallback[i]){
                NEXUS_UnregisterEvent(pDevice->ftmEventCallback[i]);
            }
            if (pDevice->fskChannels[i]) {
                BFSK_CloseChannel(pDevice->fskChannels[i]);
            }
        }
        BFSK_Close(pDevice->fskHandle);
    }
#endif


    if (pDevice->satDevice) {
        if (pDevice->satDevice->dsqHandle) {
            for (i=0; i < pDevice->numDsqChannels; i++)
            {
                if (pDevice->satDevice->dsqChannels[i]) {
                    BDSQ_CloseChannel(pDevice->satDevice->dsqChannels[i]);
                    pDevice->satDevice->dsqChannels[i] = NULL;
                }
            }
            BDSQ_Close(pDevice->satDevice->dsqHandle);
            pDevice->satDevice->dsqHandle = NULL;
        }
    }

    for ( i=0; i < pDevice->numChannels && NULL != pDevice->satChannels[i]; i++) {
        BSAT_CloseChannel(pDevice->satChannels[i]);
        pDevice->satChannels[i] = NULL;
    }

    if (pDevice->satDevice) {
        for ( i=0; i < pDevice->wfeInfo.numChannels; i++ )
        {
            if (pDevice->satDevice->wfeChannels[i]) {
                BDBG_MSG(("Closing WFE[%d]",i));
                BWFE_CloseChannel(pDevice->satDevice->wfeChannels[i]);
                pDevice->satDevice->wfeChannels[i] = NULL;
            }
        }
    }
    if (pDevice->satDevice) {
        if (pDevice->satDevice->wfeHandle) {
            BWFE_Close(pDevice->satDevice->wfeHandle);
            pDevice->satDevice->wfeHandle = NULL;
        }

        if (pDevice->satDevice->satHandle) {
            BSAT_Close(pDevice->satDevice->satHandle);
            pDevice->satDevice->satHandle = NULL;
        }
    }

    if (pDevice->isrCallback) {
        BDBG_MSG(("Unregister isrCallback %p",pDevice->isrCallback));
        NEXUS_UnregisterEvent(pDevice->isrCallback);
        pDevice->isrCallback = NULL;
        BDBG_MSG(("...done"));
    }

    if (pDevice->settings.isrNumber) {
        NEXUS_Core_DisconnectInterrupt(pDevice->settings.isrNumber);
    } else if (pDevice->settings.gpioInterrupt) {
        NEXUS_Gpio_SetInterruptCallback_priv(pDevice->settings.gpioInterrupt, NULL, NULL, 0);
    }

    if (pDevice->satDevice) {
        if (pDevice->satDevice->habHandle) {
            BHAB_Close(pDevice->satDevice->habHandle);
            pDevice->satDevice->habHandle = NULL;
        }
    }

    if (pDevice->settings.reset.enable) {
        if (pDevice->gpioHandle) {
            NEXUS_Gpio_Close(pDevice->gpioHandle);
            pDevice->gpioHandle = NULL;
        }
    }
}


void NEXUS_Frontend_GetDefault45216Settings( NEXUS_45216FrontendSettings *pSettings )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_FrontendHandle NEXUS_Frontend_Open45216( const NEXUS_45216FrontendSettings *pSettings )
{
    NEXUS_FrontendHandle frontend = NULL;
    NEXUS_45216Device *pDevice = NULL;
    NEXUS_FrontendDevice *pFrontendDevice = NULL;
    NEXUS_FrontendSatChannelSettings satChannelSettings;
    BREG_I2C_Handle i2cHandle = NULL;

    if(pSettings->device == NULL) {
        NEXUS_FrontendDevice45216OpenSettings openSettings;
        NEXUS_FrontendDevice_GetDefault45216OpenSettings(&openSettings);
        pFrontendDevice = NEXUS_FrontendDevice_Open45216(0, &openSettings);
        pDevice = (NEXUS_45216Device *)pFrontendDevice->pDevice;
    }
    else {
        pFrontendDevice = pSettings->device;
        pDevice = (NEXUS_45216Device *)pSettings->device->pDevice;
    }

    /* Return previously opened frontend handle. */
    if (pSettings->channelNumber >= pDevice->numChannels) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }
    if (pDevice->handles[pSettings->channelNumber])
        return pDevice->handles[pSettings->channelNumber];

    /* Otherwise, open new frontend */
    BDBG_MSG(("Creating channel %u", pSettings->channelNumber));

    if (pDevice->settings.diseqc.i2cDevice) {
        i2cHandle = NEXUS_I2c_GetRegHandle(pDevice->settings.diseqc.i2cDevice, NEXUS_MODULE_SELF);
        BDBG_ASSERT(NULL != i2cHandle);
    }

    /* Open channel */
    NEXUS_Frontend_P_Sat_GetDefaultChannelSettings(&satChannelSettings);
    satChannelSettings.satDevice = pDevice->satDevice;
    satChannelSettings.satChannel = pDevice->satChannels[pSettings->channelNumber];
#define B_SAT_CHIP 45216
    satChannelSettings.satChip = B_SAT_CHIP;
    satChannelSettings.channelIndex = pSettings->channelNumber;
    satChannelSettings.pCloseParam = pDevice;
    satChannelSettings.pDevice = pDevice;
    satChannelSettings.closeFunction = NEXUS_Frontend_P_45216_CloseCallback;
    satChannelSettings.diseqcIndex = 0;
    satChannelSettings.capabilities.diseqc = true;
    if (pDevice->settings.diseqc.i2cDevice) {
        satChannelSettings.getDiseqcChannelHandle = NEXUS_Frontend_P_45216_GetDiseqcChannelHandle;
        satChannelSettings.getDiseqcChannelHandleParam = pDevice;
        satChannelSettings.setVoltage = NEXUS_Frontend_P_45216_SetVoltage;
    }
    satChannelSettings.i2cRegHandle = i2cHandle; /* due to module locking, we need to save our register handle for Diseqc voltage control */
#if 0
    satChannelSettings.getDiseqcEventHandle = NEXUS_Frontend_P_45216_GetDiseqcEventHandle;
    satChannelSettings.getDiseqcAppCallback = NEXUS_Frontend_P_45216_GetDiseqcAppCallback;
    satChannelSettings.setDiseqcAppCallback = NEXUS_Frontend_P_45216_SetDiseqcAppCallback;
#endif
    satChannelSettings.getDefaultDiseqcSettings = NEXUS_Frontend_P_45216_GetDefaultDiseqcSettings;
#if NEXUS_HAS_FSK
    satChannelSettings.getFskChannelHandle = NEXUS_Frontend_P_45216_GetFskChannelHandle;
#endif

    satChannelSettings.wfeHandle = pDevice->satDevice->wfeHandle;

    satChannelSettings.deviceClearSpectrumCallbacks = NEXUS_Frontend_P_45216_ClearSpectrumCallbacks;

    frontend = NEXUS_Frontend_P_Sat_Create_Channel(&satChannelSettings);
    if ( !frontend )
    {
        BERR_TRACE(BERR_NOT_SUPPORTED);
        NEXUS_Frontend_P_45216_CloseCallback(NULL, pDevice); /* Check if channel needs to be closed */
        goto err;
    }
    frontend->tuneSatellite = NEXUS_Frontend_P_45216_TuneSatellite;
    frontend->untune = NEXUS_Frontend_P_45216_Untune;
    frontend->reapplyTransportSettings = NEXUS_Frontend_P_45216_ReapplyTransportSettings;
    frontend->pGenericDeviceHandle = pFrontendDevice;
    frontend->getSatelliteAgcStatus = NEXUS_Frontend_P_45216_GetSatelliteAgcStatus;
    frontend->getSatelliteRuntimeSettings = NEXUS_Frontend_P_Get45216RuntimeSettings;
    frontend->setSatelliteRuntimeSettings = NEXUS_Frontend_P_Set45216RuntimeSettings;

    frontend->standby = NEXUS_Frontend_P_45216_Standby;

    frontend->readRegister = NEXUS_Frontend_P_45216_ReadRegister;
    frontend->writeRegister = NEXUS_Frontend_P_45216_WriteRegister;

    /* preconfigure mtsif settings so platform doesn't need to */
    frontend->userParameters.isMtsif = true;
    frontend->mtsif.inputBand = pSettings->channelNumber + 2; /* IB for demod 0 is 2, demod 1 is 3, etc. Offset channel by 2 to compensate. */

    pDevice->handles[pSettings->channelNumber] = frontend;

    return frontend;

err:
    return NULL;
}

static void NEXUS_Frontend_P_45216_CloseCallback(NEXUS_FrontendHandle handle, void *pParam)
{
    unsigned i;
    NEXUS_45216Device *pDevice = pParam;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_45216Device);

    /* Mark handle as destroyed */
    if ( handle ) {
        for ( i = 0; i < pDevice->numChannels; i++ ) {
            if ( handle == pDevice->handles[i] ) {
                pDevice->handles[i] = NULL;
                break;
            }
        }
        BDBG_ASSERT(i < pDevice->numChannels);
    }

}

static void NEXUS_Frontend_P_45216_DestroyDevice(void *handle)
{
    unsigned i;
    NEXUS_45216Device *pDevice = (NEXUS_45216Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_45216Device);

    for ( i = 0; i < pDevice->numChannels; i++ )
    {
        if ( NULL != pDevice->handles[i] )
        {
            BDBG_ERR(("All channels must be closed before destroying device"));
            BDBG_ASSERT(NULL == pDevice->handles[i]);
        }
    }

    NEXUS_Frontend_P_Uninit45216(pDevice);

    BDBG_MSG(("Destroying 45216 device %p", (void *)pDevice));

    if (pDevice->pGenericDeviceHandle) {
        BKNI_Free(pDevice->pGenericDeviceHandle);
        pDevice->pGenericDeviceHandle = NULL;
    }

    if (pDevice->satDevice) {
        BKNI_Free(pDevice->satDevice);
        pDevice->satDevice = NULL;
    }
#if 0
    BLST_S_REMOVE(&g_deviceList, pDevice, NEXUS_45216Device, node);
#endif
    BDBG_OBJECT_DESTROY(pDevice, NEXUS_45216Device);
    BKNI_Free(pDevice);
}

void NEXUS_FrontendDevice_P_45216_S3Standby(NEXUS_45216Device *pDevice)
{
    NEXUS_Frontend_P_Uninit45216(pDevice);
}

static NEXUS_Error NEXUS_FrontendDevice_P_45216_Standby(void *handle, const NEXUS_StandbySettings *pSettings)
{

    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_45216Device *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_45216Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_45216Device);

    if ((pDevice->pGenericDeviceHandle->mode != NEXUS_StandbyMode_eDeepSleep) && (pSettings->mode == NEXUS_StandbyMode_eDeepSleep)) {

        BDBG_MSG(("NEXUS_FrontendDevice_P_45216_Standby: Entering deep sleep..."));

        NEXUS_FrontendDevice_P_45216_S3Standby(pDevice);

        BKNI_EnterCriticalSection();
        pDevice->pGenericDeviceHandle->openPending = true;
        pDevice->pGenericDeviceHandle->openFailed = false;
        BKNI_LeaveCriticalSection();

    } else if ((pDevice->pGenericDeviceHandle->mode == NEXUS_StandbyMode_eDeepSleep) && (pSettings->mode != NEXUS_StandbyMode_eDeepSleep)) {

        BDBG_MSG(("NEXUS_FrontendDevice_P_45216_Standby: Waking up..."));
        BDBG_MSG(("NEXUS_FrontendDevice_P_45216_Standby: reinitializing..."));
        rc = NEXUS_FrontendDevice_P_Init45216(pDevice);
        if (rc) { rc = BERR_TRACE(rc); goto done;}
    }

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_45216_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings)
{
    NEXUS_SatChannel *pSatChannel = (NEXUS_SatChannel *)handle;
    NEXUS_45216Device *p45216Device;

    BDBG_OBJECT_ASSERT(pSatChannel, NEXUS_SatChannel);
    p45216Device = pSatChannel->settings.pDevice;

    BDBG_MSG(("NEXUS_Frontend_P_45216_Standby: standby %p(%d) %s", handle, pSatChannel->channel, enabled ? "enabled" : "disabled"));

    BDBG_MSG(("Restoring handles on %p",pSatChannel));
    /* update/restore handles */
    pSatChannel->satChannel = p45216Device->satChannels[pSatChannel->channel];

    if (pSettings->mode == NEXUS_StandbyMode_eDeepSleep) {
        BDBG_MSG(("Unregistering events on %p",pSatChannel));
        NEXUS_Frontend_P_Sat_UnregisterEvents(pSatChannel);
    } else if (pSettings->mode != NEXUS_StandbyMode_eDeepSleep && pSatChannel->frontendHandle->mode == NEXUS_StandbyMode_eDeepSleep) {
        BDBG_MSG(("Registering events on %p",pSatChannel));
        NEXUS_Frontend_P_Sat_RegisterEvents(pSatChannel);
    }

    BDBG_MSG(("Done with standby configuration on %p",pSatChannel));
    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_FrontendDevice_P_Get45216Capabilities(void *handle, NEXUS_FrontendSatelliteCapabilities *pCapabilities)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_45216Device *p45216Device = (NEXUS_45216Device *)handle;

    BDBG_ASSERT(handle);
    BDBG_ASSERT(pCapabilities);

    BDBG_OBJECT_ASSERT(p45216Device, NEXUS_45216Device);

    BKNI_Memset(pCapabilities,0,sizeof(*pCapabilities));
    pCapabilities->numAdc = p45216Device->numAdc;
    pCapabilities->numChannels = p45216Device->numChannels;
    pCapabilities->externalBert = true;

    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_Get45216RuntimeSettings(void *handle, NEXUS_FrontendSatelliteRuntimeSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BERR_Code e;
    NEXUS_SatChannel *pSatChannel = (NEXUS_SatChannel *)handle;
    NEXUS_45216Device *p45216Device = pSatChannel->settings.pDevice;
    BSAT_ExternalBertSettings extBertSettings;

    BDBG_ASSERT(handle);
    BDBG_ASSERT(pSettings);
    BDBG_OBJECT_ASSERT(p45216Device, NEXUS_45216Device);

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    pSettings->selectedAdc = pSatChannel->selectedAdc;
    pSatChannel->diseqcIndex = pSatChannel->selectedAdc;

    e = BSAT_GetExternalBertSettings(p45216Device->satDevice->satHandle, &extBertSettings);
    if (!e) {
        if (extBertSettings.channel == pSatChannel->channel) {
            pSettings->externalBert.enabled = extBertSettings.bEnable;
            pSettings->externalBert.invertClock = extBertSettings.bClkInv;
        } else {
            pSettings->externalBert.enabled = false;
            pSettings->externalBert.invertClock = false;
        }
    } else {
        rc = BERR_TRACE(e);
    }

    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_Set45216RuntimeSettings(void *handle, const NEXUS_FrontendSatelliteRuntimeSettings *pSettings)
{
    NEXUS_SatChannel *pSatChannel = (NEXUS_SatChannel *)handle;
    NEXUS_45216Device *p45216Device;
    BERR_Code e;
    BSAT_ExternalBertSettings extBertSettings;

    BDBG_ASSERT(handle);
    BDBG_ASSERT(pSettings);
    p45216Device = pSatChannel->settings.pDevice;
    BDBG_OBJECT_ASSERT(p45216Device, NEXUS_45216Device);

    BDBG_MSG(("adc: %d, mask: 0x%08x",pSettings->selectedAdc,p45216Device->wfeInfo.availChannelsMask));
    /* Ensure the requested adc is within the value range, and advertised by the PI as being available */
    if (pSettings->selectedAdc > p45216Device->numAdc || !((1<<pSettings->selectedAdc) & p45216Device->wfeInfo.availChannelsMask) )
        return NEXUS_INVALID_PARAMETER;

    pSatChannel->selectedAdc = pSettings->selectedAdc;
    pSatChannel->diseqcIndex = pSettings->selectedAdc;
    p45216Device->wfeMap[pSatChannel->channel] = pSettings->selectedAdc;
    pSatChannel->satDevice->wfeMap[pSatChannel->channel] = pSettings->selectedAdc;

    e = BSAT_GetExternalBertSettings(p45216Device->satDevice->satHandle, &extBertSettings);
    if (!e) {
        if (pSettings->externalBert.enabled) {
            extBertSettings.channel = pSatChannel->channel;
            extBertSettings.bEnable = pSettings->externalBert.enabled;
            extBertSettings.bClkInv = pSettings->externalBert.invertClock;
            BSAT_SetExternalBertSettings(p45216Device->satDevice->satHandle, &extBertSettings);
        } else {
            /* Only disable it if it's enabled on the current channel, otherwise we might be
             * interfering with the settings on a different demod. */
            if (extBertSettings.channel == pSatChannel->channel) {
                extBertSettings.channel = pSatChannel->channel;
                extBertSettings.bEnable = pSettings->externalBert.enabled;
                extBertSettings.bClkInv = pSettings->externalBert.invertClock;
                BSAT_SetExternalBertSettings(p45216Device->satDevice->satHandle, &extBertSettings);
            }
        }
    } else {
        BERR_TRACE(e);
    }

    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_45216_GetSatelliteAgcStatus(void *handle, NEXUS_FrontendSatelliteAgcStatus *pStatus)
{
    NEXUS_SatChannel *pSatChannel = (NEXUS_SatChannel *)handle;
    NEXUS_45216Device *p45216Device = pSatChannel->settings.pDevice;
    BERR_Code rc = NEXUS_SUCCESS;
    BSAT_ChannelStatus satStatus;
    BERR_Code errCode;

    BDBG_ASSERT(NULL != pStatus);
    BDBG_ASSERT(NULL != p45216Device);

    if (pSatChannel->satChip != B_SAT_CHIP) {
        return NEXUS_INVALID_PARAMETER;
    }

    /* This short-circuits a second read of the AGC values, in case of a
     * back to back NEXUS_Frontend_GetSatelliteStatus and NEXUS_Frontend_GetSatelliteAgcStatus
     * calling sequence, since both read the values with the same BSAT_GetChannelStatus call.
     */
    if (pSatChannel->cachedAgc.valid) {
        NEXUS_Time current;
        NEXUS_Time_Get(&current);

        pSatChannel->cachedAgc.valid = false;
        if (NEXUS_Time_Diff(&current,&pSatChannel->cachedAgc.lastRead) < 250) {
            int i;
            for (i=0; i<MAX_SATELLITE_AGC_VALUES; i++) {
                pStatus->agc[i].value = pSatChannel->cachedAgc.values.agc[i].value;
                pStatus->agc[i].valid = pSatChannel->cachedAgc.values.agc[i].valid;
            }
            return NEXUS_SUCCESS;
        }
    }

    errCode = BSAT_GetChannelStatus(pSatChannel->satChannel, &satStatus);
    if ( errCode ) {
        BDBG_MSG(("BSAT_GetChannelStatus returned %x",errCode));
        rc = errCode;
    } else {
        int i;
        BKNI_Memset(pStatus,0,sizeof(*pStatus));
        for (i=0; i<3; i++) {
            pStatus->agc[i].value = satStatus.agc.value[i];
            pStatus->agc[i].valid = (satStatus.agc.flags & 1<<i);
        }
    }

    return rc;
}

NEXUS_Error NEXUS_Frontend_P_Sat_TuneSatellite(void *handle, const NEXUS_FrontendSatelliteSettings *pSettings);
void NEXUS_Frontend_P_Sat_Untune(void *handle);

static NEXUS_Error NEXUS_Frontend_P_45216_TuneSatellite(void *handle, const NEXUS_FrontendSatelliteSettings *pSettings)
{
    NEXUS_SatChannel *pSatChannel = handle;
    NEXUS_45216Device *p45216Device = pSatChannel->settings.pDevice;
    NEXUS_Error rc;

    if (p45216Device->pGenericDeviceHandle->mtsifConfig.mxt) {
        rc = NEXUS_Frontend_P_SetMtsifConfig(pSatChannel->frontendHandle);
        if (rc) { return BERR_TRACE(rc); }
    }

#if 0
    p45216Device->useChannels[pSatChannel->channel] = true;
#if NEXUS_POWER_MANAGEMENT
    NEXUS_Frontend_P_SetAdcPower(pSatChannel->frontendHandle);
#endif
#endif

    return NEXUS_Frontend_P_Sat_TuneSatellite(pSatChannel, pSettings);
}

static void NEXUS_Frontend_P_45216_Untune(void *handle)
{
    NEXUS_SatChannel *pSatChannel = handle;
    NEXUS_45216Device *p45216Device = pSatChannel->settings.pDevice;

    if (p45216Device->pGenericDeviceHandle->mtsifConfig.mxt) {
        NEXUS_Frontend_P_UnsetMtsifConfig(pSatChannel->frontendHandle);
    }

    NEXUS_Frontend_P_Sat_Untune(pSatChannel);

#if 0
    p45216Device->useChannels[pSatChannel->channel] = false;
#if NEXUS_POWER_MANAGEMENT
    NEXUS_Frontend_P_SetAdcPower(pSatChannel->frontendHandle);
#endif
#endif

    return;
}

NEXUS_Error NEXUS_Frontend_P_45216_ReapplyTransportSettings(void *handle)
{
    NEXUS_SatChannel *pSatChannel = handle;
    NEXUS_45216Device *p45216Device = pSatChannel->settings.pDevice;
    BERR_Code rc;

    if (p45216Device->pGenericDeviceHandle->mtsifConfig.mxt) {
        rc = NEXUS_Frontend_P_SetMtsifConfig(pSatChannel->frontendHandle);
        if (rc) { return BERR_TRACE(rc); }
    }

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Frontend_P_45216_ReadRegister(void *handle, unsigned address, uint32_t *pValue)
{
    NEXUS_SatChannel *pSatChannel = handle;
    NEXUS_45216Device *p45216Device;
    BERR_Code rc;
    BDBG_ASSERT(pSatChannel);
    p45216Device = pSatChannel->settings.pDevice;
    BDBG_OBJECT_ASSERT(p45216Device, NEXUS_45216Device);

    rc = BHAB_ReadRegister(p45216Device->satDevice->habHandle, address, pValue);
    if (rc) { return BERR_TRACE(rc); }
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Frontend_P_45216_WriteRegister(void *handle, unsigned address, uint32_t value)
{
    NEXUS_SatChannel *pSatChannel = handle;
    NEXUS_45216Device *p45216Device;
    BERR_Code rc;
    BDBG_ASSERT(pSatChannel);
    p45216Device = pSatChannel->settings.pDevice;
    BDBG_OBJECT_ASSERT(p45216Device, NEXUS_45216Device);

    rc = BHAB_WriteRegister(p45216Device->satDevice->habHandle, address, &value);
    if (rc) { return BERR_TRACE(rc); }
    return NEXUS_SUCCESS;
}

static BDSQ_ChannelHandle NEXUS_Frontend_P_45216_GetDiseqcChannelHandle(void *handle, int index)
{
    NEXUS_SatChannel *pSatChannel = handle;
    NEXUS_45216Device *p45216Device;
    p45216Device = pSatChannel->settings.pDevice;
    return p45216Device->satDevice->dsqChannels[index];
}

static NEXUS_Error NEXUS_Frontend_P_45216_SetVoltage(void *pDevice, NEXUS_FrontendDiseqcVoltage voltage)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_SatChannel *pSatChannel = pDevice;
    NEXUS_45216Device *p45216Device = pSatChannel->settings.pDevice;

    /* Just in case DSQ book-keeping requires it: */
    BDSQ_SetVoltage(p45216Device->satDevice->dsqChannels[pSatChannel->diseqcIndex], voltage == NEXUS_FrontendDiseqcVoltage_e18v);

    { /* Write voltage to A8299 */
        int channel = pSatChannel->diseqcIndex;
        uint8_t buf[2];
        uint8_t i2c_addr, shift, ctl;
        uint8_t A8299_control = p45216Device->A8299_control;

        i2c_addr = p45216Device->settings.diseqc.i2cAddr;

        if ((channel & 1) == 0)
            shift = 0;
        else
            shift = 4;

        buf[0] = 0;

        /* Clear A8299 i2c in case of fault */
        rc = BREG_I2C_WriteNoAddr(pSatChannel->settings.i2cRegHandle, i2c_addr, buf, 1);
        BREG_I2C_ReadNoAddr(pSatChannel->settings.i2cRegHandle, i2c_addr, buf, 1);

        ctl = (voltage == NEXUS_FrontendDiseqcVoltage_e18v) ? 0xC : 0x8;
        A8299_control &= ~((0x0F) << shift);
        A8299_control |= (ctl << shift);

        buf[0] = 0;
        buf[1] = A8299_control;

        p45216Device->A8299_control = A8299_control;

        BDBG_MSG(("A8299: channel=%d, i2c_addr=0x%X, ctl=0x%02X 0x%02X", channel, i2c_addr, buf[0], buf[1]));
        rc = BREG_I2C_WriteNoAddr(pSatChannel->settings.i2cRegHandle, i2c_addr, buf, 2);
        if (rc) return BERR_TRACE(rc);

    }

    return rc;
}

static void NEXUS_Frontend_P_45216_GetDefaultDiseqcSettings(void *pDevice, BDSQ_ChannelSettings *settings)
{
    NEXUS_SatChannel *pSatChannel = pDevice;
    NEXUS_45216Device *p45216Device = pSatChannel->settings.pDevice;
    BDSQ_GetChannelDefaultSettings(p45216Device->satDevice->dsqHandle, 0, settings);
}

static void NEXUS_FrontendDevice_P_45216_GetCapabilities(void *handle, NEXUS_FrontendDeviceCapabilities *pCapabilities)
{
    NEXUS_45216Device *pDevice = (NEXUS_45216Device *)handle;

    BDBG_ASSERT(handle);
    BDBG_ASSERT(pCapabilities);
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_45216Device);

    pCapabilities->numTuners = pDevice->numChannels;
}

static void NEXUS_Frontend_P_45216_ClearSpectrumCallbacks(void *handle)
{
    NEXUS_SatChannel *pSatChannel = (NEXUS_SatChannel *)handle;
    NEXUS_45216Device *p45216Device;
    unsigned i;

    BDBG_ASSERT(pSatChannel);
    p45216Device = pSatChannel->settings.pDevice;
    BDBG_OBJECT_ASSERT(p45216Device, NEXUS_45216Device);

    for (i=0; i < p45216Device->numChannels; i++) {
        NEXUS_SatChannel *pChannel = p45216Device->handles[i]->pDeviceHandle;
        if (pChannel) {
            if (pChannel->spectrumEventCallback) {
                NEXUS_UnregisterEvent(pChannel->spectrumEventCallback);
                pChannel->spectrumEventCallback = NULL;
            }
        }
    }
}

static NEXUS_Error NEXUS_FrontendDevice_P_45216_GetStatus(void *handle, NEXUS_FrontendDeviceStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_45216Device *pDevice = NULL;
    BHAB_AvsData avsData;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_45216Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_45216Device);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    rc = BHAB_GetAvsData(pDevice->satDevice->habHandle, &avsData);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pStatus->avs.enabled = avsData.enabled;
    pStatus->temperature = avsData.temperature;
    if(avsData.enabled) {
        pStatus->avs.voltage = avsData.voltage;
    }
    pStatus->openPending = pDevice->pGenericDeviceHandle->openPending;
    pStatus->openFailed = pDevice->pGenericDeviceHandle->openFailed;
done:
    return rc;
}

#if NEXUS_HAS_FSK
static BFSK_ChannelHandle NEXUS_Frontend_P_45216_GetFskChannelHandle(void *pDevice, int index)
{
    NEXUS_SatChannel *pSatChannel = pDevice;
    NEXUS_45216Device *p45216Device = pSatChannel->settings.pDevice;

    if((unsigned)index < p45216Device->numFskChannels)
        return p45216Device->fskChannels[index];
    else
        return NULL;
}

static void NEXUS_Frontend_P_45216_FtmEventCallback(void *context)
{
    NEXUS_SatChannel *pSatChannel;
    NEXUS_45216Device *p45216Device = context;
    unsigned i;

    for (i=0;i<NEXUS_45216_MAX_FRONTEND_CHANNELS;i++) {
        if (p45216Device->handles[i]) {
            pSatChannel = (NEXUS_SatChannel *)p45216Device->handles[i]->pDeviceHandle;
            if(pSatChannel->ftmCallback)
                NEXUS_TaskCallback_Fire(pSatChannel->ftmCallback);
        }
    }
}

#endif

NEXUS_Error NEXUS_Frontend_P_Get45216ChipInfo(const NEXUS_FrontendDevice45216OpenSettings *pSettings, NEXUS_45216ProbeResults *pResults) {
    NEXUS_Error rc = NEXUS_SUCCESS;

    BSTD_UNUSED(pSettings);
    pResults->chip.id = 0x45216;

    return rc;
}

static uint32_t NEXUS_Platform_P_I2c_Get45216ChipId(NEXUS_I2cHandle i2cDevice, uint16_t i2cAddr)
{
    BREG_I2C_Handle i2cHandle;
    uint8_t buf[5];
    uint32_t chipId=0;
    uint8_t subAddr;

    i2cHandle = NEXUS_I2c_GetRegHandle(i2cDevice, NULL);
    BDBG_MSG(("i2c handle: %p, i2caddr: 0x%x",i2cHandle,i2cAddr));
    buf[0]= 0x0;
    subAddr = 0x1;
    BREG_I2C_WriteNoAddr(i2cHandle, i2cAddr, (uint8_t *)&subAddr, 1);
    BREG_I2C_ReadNoAddr(i2cHandle, i2cAddr, buf, 1);
    chipId = buf[0];

    subAddr = 0x2;
    BREG_I2C_WriteNoAddr(i2cHandle, i2cAddr, (uint8_t *)&subAddr, 1);
    BREG_I2C_ReadNoAddr(i2cHandle, i2cAddr, buf, 1);
    chipId = (chipId << 8) | buf[0];

    subAddr = 0x3;
    BREG_I2C_WriteNoAddr(i2cHandle, i2cAddr, (uint8_t *)&subAddr, 1);
    BREG_I2C_ReadNoAddr(i2cHandle, i2cAddr, buf, 1);
    chipId = (chipId << 8) | buf[0];

    BDBG_MSG(("chip family ID = 0x%04x", chipId));

    return chipId;
}

#define DEBUG_SPI_READS 0
static uint32_t NEXUS_Platform_P_Spi_Get45216ChipId(NEXUS_SpiHandle spiDevice, uint16_t spiAddr)
{
    uint16_t chipId=0;
    uint8_t wData[2], rData[8];
    NEXUS_Error rc;

    BDBG_MSG(("Probing for 45216 at SPI 0x%02x",spiAddr));

    wData[0] = spiAddr << 1;
    wData[1] = 0x00;
#if DEBUG_SPI_READS
    {
        int i;
        for (i=0; i < 2; i++) {
            BDBG_MSG(("wData[%d]: 0x%02x",i,wData[i]));
        }
    }
#endif

    rc = NEXUS_Spi_Read(spiDevice, wData, rData, 8);
    if(rc){rc = BERR_TRACE(rc); goto done;}

#if DEBUG_SPI_READS
    {
        int i;
        for (i=0; i < 8; i++) {
            BDBG_MSG(("rData[%d]: 0x%02x",i,rData[i]));
        }
    }
#endif

    chipId = (rData[3] << 16) | (rData[4] << 8) | (rData[5]);

    BDBG_MSG(("chip family ID = 0x%04x", chipId));

done:
    return chipId;
}
NEXUS_Error NEXUS_Frontend_Probe45216(const NEXUS_FrontendDevice45216OpenSettings *pSettings, NEXUS_45216ProbeResults *pResults)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pResults);

    BKNI_Memset(pResults, 0, sizeof(*pResults));

    if (pSettings->i2cDevice) {
        pResults->chip.familyId = (uint32_t)NEXUS_Platform_P_I2c_Get45216ChipId(pSettings->i2cDevice, pSettings->i2cAddr);
        if ( pResults->chip.familyId != 0x45216 )
        {
            BDBG_MSG(("pResults->chip.familyId = 0x%x", pResults->chip.familyId));
            rc = BERR_INVALID_PARAMETER; goto done;
        }
    } else if (pSettings->spiDevice) {
        pResults->chip.familyId = (uint32_t)NEXUS_Platform_P_Spi_Get45216ChipId(pSettings->spiDevice, pSettings->spiAddr);
        if ( pResults->chip.familyId != 0x45216 )
        {
            BDBG_MSG(("pResults->chip.familyId = 0x%x", pResults->chip.familyId));
            rc = BERR_INVALID_PARAMETER; goto done;
        }
    } else { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto done; }

    rc = NEXUS_Frontend_P_Get45216ChipInfo(pSettings, pResults);

done:
    return rc;
}

NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_P_Open45216(unsigned index, const NEXUS_FrontendDeviceOpenSettings *pSettings)
{
    NEXUS_FrontendDevice45216OpenSettings settings;
    int i;

    NEXUS_FrontendDevice_GetDefault45216OpenSettings(&settings);
    settings.i2cDevice = pSettings->i2cDevice;
    settings.i2cAddr = pSettings->i2cAddress;
    settings.gpioInterrupt = pSettings->gpioInterrupt;
    settings.isrNumber = pSettings->isrNumber;
    settings.spiDevice = pSettings->spiDevice;
    settings.spiAddr = 0x20;
    settings.diseqc.i2cDevice = pSettings->satellite.diseqc.i2cDevice;
    settings.diseqc.i2cAddr = pSettings->satellite.diseqc.i2cAddress;
    for (i=0; i < NEXUS_MAX_MTSIF; i++) {
        settings.mtsif[i] = pSettings->mtsif[i];
    }

    return NEXUS_FrontendDevice_Open45216(index, &settings);
}

NEXUS_FrontendHandle NEXUS_Frontend_P_Open45216(const NEXUS_FrontendChannelSettings *pSettings)
{
    NEXUS_45216FrontendSettings settings;

    NEXUS_Frontend_GetDefault45216Settings(&settings);
    settings.device = pSettings->device;
    settings.channelNumber = pSettings->channelNumber;
    return NEXUS_Frontend_Open45216(&settings);
}

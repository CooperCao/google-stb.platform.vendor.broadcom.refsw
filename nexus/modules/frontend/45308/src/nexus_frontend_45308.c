/******************************************************************************
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
 ******************************************************************************/
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
#include "bsat_45308.h"
#include "bwfe_45308.h"
#include "bdsq_45308.h"
#if NEXUS_HAS_FSK
#include "bfsk_45308.h"
#endif
#include "bhab.h"
#include "bhab_45308.h"
#if NEXUS_FRONTEND_45308_FW
#include "bhab_45308_fw.h"
#endif
#if NEXUS_FRONTEND_45316_FW
#include "bhab_45316_fw.h"
#endif
#include "priv/nexus_core_img.h"
#include "priv/nexus_core_img_id.h"
#include "bhab_satfe_img.h"
#include "bdbg.h"

BDBG_MODULE(nexus_frontend_45308);

BDBG_OBJECT_ID(NEXUS_45308Device);

/* set to 1 to enable L1 interrupt messages */
#define NEXUS_FRONTEND_DEBUG_IRQ 0

/* set to 1 to disable asynchronous initialization */
#define DISABLE_45308_ASYNC_INIT 0

#ifndef NEXUS_45308_MAX_FRONTEND_CHANNELS
#define NEXUS_45308_MAX_FRONTEND_CHANNELS 16
#endif

#if NEXUS_HAS_FRONTEND_PID_FILTERING && (!NEXUS_TRANSPORT_EXTENSION_TBG)
#define NEXUS_HAS_453XX_PID_FILTERING 1
#endif

typedef struct NEXUS_FrontendDevice45308OpenSettings
{
    /* either GPIO or an L1 is used for notification from the frontend to the host. */
    NEXUS_FrontendDeviceReset reset;    /* Information required for controlling the GPIO reset for S3, if necessary. */
    NEXUS_GpioHandle gpioInterrupt;     /* GPIO pin for interrupt. If not NULL, isrNumber is ignored. If NULL, isrNumber is used. */
    unsigned isrNumber;                 /* L1 interrupt number. (typically 0..63). See gpioInterrupt for other interrupt option. */

    /* Either SPI or I2C is used for the host to control the frontend chip. */
    NEXUS_I2cHandle i2cDevice;          /* I2C device to use. spiDevice should be NULL to use I2C. */
    uint16_t i2cAddr;                   /* master device I2C Address */
    uint8_t i2cSlaveAddr;               /* slave device I2C Address */

    NEXUS_SpiHandle spiDevice;          /* SPI device to use. i2cDevice should be NULL to use SPI. */
    uint16_t spiAddr;                   /* master device SPI Address */

    NEXUS_FrontendDeviceMtsifSettings mtsif[NEXUS_MAX_MTSIF]; /* Configure MTSIF rate and drive strength at open time. If values are 0, defaults are used. */

    struct {
        uint16_t i2cAddr;               /* I2C address to communicate with diseqc */
        NEXUS_I2cHandle i2cDevice;      /* I2C device to communicate with diseqc */
    } diseqc;
} NEXUS_FrontendDevice45308OpenSettings;

typedef struct NEXUS_45308FrontendSettings
{
    NEXUS_FrontendDeviceHandle device;  /* Previously opened device to use */
    unsigned channelNumber;             /* Which channel to open from this device */
} NEXUS_45308FrontendSettings;

typedef struct NEXUS_45308ProbeResults
{
    NEXUS_FrontendChipType chip;
} NEXUS_45308ProbeResults;

NEXUS_Error NEXUS_Frontend_Probe45308(const NEXUS_FrontendDevice45308OpenSettings *pSettings, NEXUS_45308ProbeResults *pResults);

typedef struct NEXUS_45308Device
{
    BDBG_OBJECT(NEXUS_45308Device)
    NEXUS_FrontendDevice45308OpenSettings settings;
    NEXUS_FrontendDevice *pGenericDeviceHandle;
    NEXUS_SatDeviceHandle satDevice;
    bool isExternal;
    uint32_t numChannels;   /* prototype to match BSAT_GetTotalChannels */
    uint8_t numAdc;         /* prototype to match BSAT_GetAdcSelect */
    BKNI_EventHandle isrEvent;
    NEXUS_EventCallbackHandle isrCallback;
    BSAT_ChannelHandle satChannels[NEXUS_45308_MAX_FRONTEND_CHANNELS];
    NEXUS_FrontendHandle handles[NEXUS_45308_MAX_FRONTEND_CHANNELS];
    BWFE_ChannelInfo wfeInfo;
    uint32_t numDsqChannels;   /* prototype to match BDSQ_GetTotalChannels */
    int wfeMap[NEXUS_45308_MAX_FRONTEND_CHANNELS];
#if NEXUS_FRONTEND_HAS_A8299_DISEQC
    uint8_t A8299_control;
#endif
#if NEXUS_HAS_FSK
    uint32_t numFskChannels;
    BFSK_Handle fskHandle;
    BFSK_ChannelHandle fskChannels[NEXUS_45308_MAX_FRONTEND_CHANNELS];
    NEXUS_EventCallbackHandle ftmEventCallback[NEXUS_45308_MAX_FRONTEND_CHANNELS];
#endif
    NEXUS_GpioHandle gpioHandle;
    NEXUS_ThreadHandle deviceOpenThread;
    BIMG_Interface imgInterface;
    const uint8_t *fw;
    unsigned chipId;
} NEXUS_45308Device;

#if 0
static BLST_S_HEAD(devList, NEXUS_45308Device) g_deviceList = BLST_S_INITIALIZER(g_deviceList);
#endif

static void NEXUS_Frontend_P_45308_CloseCallback(NEXUS_FrontendHandle handle, void *pParam);
static void NEXUS_Frontend_P_45308_DestroyDevice(void *handle);

static NEXUS_Error NEXUS_Frontend_P_45308_TuneSatellite(void *handle, const NEXUS_FrontendSatelliteSettings *pSettings);
static void NEXUS_Frontend_P_45308_Untune(void *handle);
static NEXUS_Error NEXUS_Frontend_P_45308_ReapplyTransportSettings(void *handle);

static BDSQ_ChannelHandle NEXUS_Frontend_P_45308_GetDiseqcChannelHandle(void *handle, int index);
static NEXUS_Error NEXUS_Frontend_P_45308_SetVoltage(void *pDevice, NEXUS_FrontendDiseqcVoltage voltage);

static void NEXUS_Frontend_P_45308_GetDefaultDiseqcSettings(void *pDevice, BDSQ_ChannelSettings *settings);
#if NEXUS_HAS_FSK
static BFSK_ChannelHandle NEXUS_Frontend_P_45308_GetFskChannelHandle(void *handle, int index);
static void NEXUS_Frontend_P_45308_FtmEventCallback(void *context);
#endif

static NEXUS_Error NEXUS_FrontendDevice_P_Get45308Capabilities(void *handle, NEXUS_FrontendSatelliteCapabilities *pCapabilities);
static NEXUS_Error NEXUS_Frontend_P_Get45308RuntimeSettings(void *handle, NEXUS_FrontendSatelliteRuntimeSettings *pSettings);
static NEXUS_Error NEXUS_Frontend_P_Set45308RuntimeSettings(void *handle, const NEXUS_FrontendSatelliteRuntimeSettings *pSettings);

#if 0
static void NEXUS_Frontend_P_45308_RestoreLnaCallback(NEXUS_FrontendHandle handle, void *pParam);
static BERR_Code NEXUS_Frontend_P_45308_I2cRead(void * i2cHandle, uint16_t chipAddr, uint32_t subAddr, uint8_t *pData, size_t length);
static BERR_Code NEXUS_Frontend_P_45308_I2cWrite(void * i2cHandle, uint16_t chipAddr, uint32_t subAddr, const uint8_t *pData, size_t length);
static BERR_Code NEXUS_Frontend_P_45308_I2cReadNoAddr(void * context, uint16_t chipAddr, uint8_t *pData, size_t length);
static BERR_Code NEXUS_Frontend_P_45308_I2cWriteNoAddr(void * context, uint16_t chipAddr, const uint8_t *pData, size_t length);
#endif
static NEXUS_Error NEXUS_FrontendDevice_P_45308_GetStatus(void * handle, NEXUS_FrontendDeviceStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_45308_GetSatelliteAgcStatus(void *handle, NEXUS_FrontendSatelliteAgcStatus *pStatus);

static void NEXUS_FrontendDevice_P_45308_GetCapabilities(void * handle, NEXUS_FrontendDeviceCapabilities *pCapabilities);

static void NEXUS_Frontend_P_45308_ClearSpectrumCallbacks(void *handle);

static NEXUS_Error NEXUS_Frontend_P_45308_ReadRegister(void *handle, unsigned address, uint32_t *pValue);
static NEXUS_Error NEXUS_Frontend_P_45308_WriteRegister(void *handle, unsigned address, uint32_t value);

static NEXUS_Error NEXUS_FrontendDevice_P_45308_Standby(void * handle, const NEXUS_StandbySettings *pSettings);
static NEXUS_Error NEXUS_Frontend_P_45308_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings);

void NEXUS_FrontendDevice_GetDefault45308OpenSettings(NEXUS_FrontendDevice45308OpenSettings *pSettings)
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
    Enable/Disable interrupts for a 45308 device
 ***************************************************************************/
static void NEXUS_Frontend_P_45308_IsrControl_isr(bool enable, void *pParam)
{
    unsigned *isrNumber = (unsigned *)pParam;

    if ( enable )
    {
#if NEXUS_FRONTEND_DEBUG_IRQ
        BDBG_MSG(("Enable 45308 Interrupt %u", *isrNumber));
#endif
        NEXUS_Core_EnableInterrupt_isr(*isrNumber);
    }
    else
    {
#if NEXUS_FRONTEND_DEBUG_IRQ
        BDBG_MSG(("Disable 45308 Interrupt %u", *isrNumber));
#endif
        NEXUS_Core_DisableInterrupt_isr(*isrNumber);
    }
}

/***************************************************************************
Summary:
    Enable/Disable gpio interrupts for a 45308 device
 ***************************************************************************/
static void NEXUS_Frontend_P_45308_GpioIsrControl_isr(bool enable, void *pParam)
{
    NEXUS_GpioHandle gpioHandle = (NEXUS_GpioHandle)pParam;

#if NEXUS_FRONTEND_DEBUG_IRQ
    BDBG_MSG(("%s 45308 Gpio Interrupt %p", enable ? "Enable" : "Disable", (void *)gpioHandle));
#endif
    NEXUS_Gpio_SetInterruptEnabled_isr(gpioHandle, enable);
}

static void NEXUS_Frontend_P_45308_IsrCallback(void *pParam)
{
    NEXUS_45308Device *pDevice = (NEXUS_45308Device *)pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_45308Device);
#if NEXUS_FRONTEND_DEBUG_IRQ
    BDBG_MSG(("45308 ISR Callback (hab: %p)", (void *)pDevice->satDevice->habHandle));
#endif
    BHAB_ProcessInterruptEvent(pDevice->satDevice->habHandle);
}

static void NEXUS_Frontend_P_45308_L1_isr(void *param1, int param2)
{
    NEXUS_45308Device *pDevice = (NEXUS_45308Device *)param1;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_45308Device);
    BSTD_UNUSED(param2);
#if NEXUS_FRONTEND_DEBUG_IRQ
    BDBG_MSG(("45308 L1 ISR (hab: %p)", (void *)pDevice->satDevice->habHandle));
#endif

    BHAB_HandleInterrupt_isr(pDevice->satDevice->habHandle);
#if NEXUS_FRONTEND_DEBUG_IRQ
    BDBG_MSG(("Done: 45308 L1 ISR (hab: %p)", (void *)pDevice->satDevice->habHandle));
#endif
}

typedef struct NEXUS_FrontendDevice_P_45308_InitSettings {
    BHAB_Settings habSettings;
    BSAT_Settings satSettings;
    BWFE_Settings wfeSettings;
    BDSQ_Settings dsqSettings;
#if NEXUS_HAS_FSK
    BFSK_Settings fskSettings;
#endif
} NEXUS_FrontendDevice_P_45308_InitSettings;

static NEXUS_Error NEXUS_FrontendDevice_P_Init45308_PreInitAP(NEXUS_45308Device *pDevice)
{
    NEXUS_FrontendDevice_P_45308_InitSettings *pInitSettings;
    BHAB_Handle habHandle;
    BSAT_Handle satHandle;
    BWFE_Handle wfeHandle;
    BDSQ_Handle dsqHandle;
#if NEXUS_HAS_FSK
    BFSK_Handle fskHandle;
#endif
    NEXUS_45308ProbeResults results;
    unsigned i;
    void *regHandle;
    BERR_Code errCode;
    NEXUS_FrontendDevice45308OpenSettings *pSettings = &pDevice->settings;

    pInitSettings = (NEXUS_FrontendDevice_P_45308_InitSettings *)BKNI_Malloc(sizeof(NEXUS_FrontendDevice_P_45308_InitSettings));
    if (!pInitSettings) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto err;
    }
    BKNI_Memset(pInitSettings, 0, sizeof(NEXUS_FrontendDevice_P_45308_InitSettings));

    NEXUS_Frontend_Probe45308(pSettings, &results);
    BDBG_MSG(("chipid: %x",results.chip.familyId));
    pDevice->chipId = results.chip.familyId;

    BHAB_45308_GetDefaultSettings(&pInitSettings->habSettings);
    {
#if NEXUS_MODE_driver
        const char *img_id = NULL;

        if (results.chip.familyId == 0x45316) {
#if NEXUS_FRONTEND_45316_FW
            img_id = NEXUS_CORE_IMG_ID_FRONTEND_45316;
#else
            BDBG_ERR(("45316 detected without 45316 firmware, rebuild with NEXUS_FRONTEND_45316=y"));
#endif
        } else {
#if NEXUS_FRONTEND_45308_FW
            img_id = NEXUS_CORE_IMG_ID_FRONTEND_45308;
#else
            BDBG_ERR(("%x detected without 45308 firmware, rebuild with NEXUS_FRONTEND_45308=y", results.chip.familyId));
#endif
        }
        if (Nexus_Core_P_Img_Create(img_id, &pInitSettings->habSettings.pImgContext, &pDevice->imgInterface ) == NEXUS_SUCCESS) {
            pInitSettings->habSettings.pImgInterface = &pDevice->imgInterface;
        } else {
            pInitSettings->habSettings.pImgContext = NULL;
        }
#else
        pInitSettings->habSettings.pImgInterface = &BHAB_SATFE_IMG_Interface;
        if (results.chip.familyId == 0x45316) {
#if NEXUS_FRONTEND_45316_FW
            pInitSettings->habSettings.pImgContext = (void *)&BHAB_45316_IMG_Context;
            pDevice->fw = bcm45316_ap_image;
#else
            BDBG_ERR(("45316 detected without 45316 firmware, rebuild with NEXUS_FRONTEND_45316=y"));
#endif
        } else {
#if NEXUS_FRONTEND_45308_FW
            pInitSettings->habSettings.pImgContext = (void *)&BHAB_45308_IMG_Context;
            pDevice->fw = bcm45308_ap_image;
#else
            BDBG_ERR(("%x detected without 45308 firmware, rebuild with NEXUS_FRONTEND_45308=y", results.chip.familyId));
#endif
        }
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

        BDBG_MSG(("i2cDevice: %p", (void *)pSettings->i2cDevice));
        BDBG_MSG(("BREG_I2C_Handle: %p", (void *)regHandle));
    } else {
        regHandle = NULL;
    }
    pInitSettings->habSettings.isMtsif = true;
    if(pSettings->isrNumber) {
        BDBG_MSG(("Configuring for external interrupt"));
        pInitSettings->habSettings.interruptEnableFunc = NEXUS_Frontend_P_45308_IsrControl_isr;
        pInitSettings->habSettings.interruptEnableFuncParam = (void*)&pDevice->settings.isrNumber;
    }
    else if(pSettings->gpioInterrupt){
        BDBG_MSG(("Configuring for GPIO interrupt"));
        pInitSettings->habSettings.interruptEnableFunc = NEXUS_Frontend_P_45308_GpioIsrControl_isr;
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
                                             NEXUS_Frontend_P_45308_L1_isr,
                                             (void *)pDevice,
                                             0);
        if ( errCode != BERR_SUCCESS )
        {
            errCode = BERR_TRACE(errCode);
            goto err;
        }
    }
    else if(pSettings->gpioInterrupt){
        NEXUS_GpioSettings gpioSettings;
        NEXUS_Gpio_GetSettings(pDevice->settings.gpioInterrupt, &gpioSettings);
        gpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;
        NEXUS_Gpio_SetSettings(pDevice->settings.gpioInterrupt, &gpioSettings);
        BDBG_MSG(("Connecting GPIO interrupt"));
        NEXUS_Gpio_SetInterruptCallback_priv(pSettings->gpioInterrupt,
                                             NEXUS_Frontend_P_45308_L1_isr,
                                             (void *)pDevice,
                                             0);
    }

    errCode = BSAT_45308_GetDefaultSettings(&pInitSettings->satSettings);
    if (errCode) { BERR_TRACE(NEXUS_OS_ERROR); goto err; }

    errCode = BSAT_Open(&satHandle, g_pCoreHandles->chp, habHandle, NULL, &pInitSettings->satSettings); /* CHP and INT are unused by SAT */
    if (errCode) { BERR_TRACE(NEXUS_OS_ERROR); goto err; }

    errCode = BWFE_45308_GetDefaultSettings(&pInitSettings->wfeSettings);
    if (errCode) { BERR_TRACE(NEXUS_OS_ERROR); goto err; }

    errCode = BWFE_Open(&wfeHandle, g_pCoreHandles->chp, habHandle, NULL, &pInitSettings->wfeSettings); /* CHP and INT are unused by WFE */
    if (errCode) { BERR_TRACE(NEXUS_OS_ERROR); goto err; }

    errCode = BDSQ_45308_GetDefaultSettings(&pInitSettings->dsqSettings);
    if (errCode) { BERR_TRACE(NEXUS_OS_ERROR); goto err; }

    errCode = BDSQ_Open(&dsqHandle, g_pCoreHandles->chp, habHandle, NULL, &pInitSettings->dsqSettings); /* CHP and INT are unused by DSQ */
    if (errCode) { BERR_TRACE(NEXUS_OS_ERROR); goto err; }

#if NEXUS_HAS_FSK
    errCode = BFSK_45308_GetDefaultSettings(&pInitSettings->fskSettings);
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
    if ( pDevice->numChannels > NEXUS_45308_MAX_FRONTEND_CHANNELS )
    {
        BDBG_WRN(("This 45308 device supports more than the expected number of channels. Unexpected channels will not be initialized."));
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
    BKNI_Memset(&pDevice->wfeInfo,0,sizeof(pDevice->wfeInfo));
    errCode = BWFE_GetTotalChannels(wfeHandle, &pDevice->wfeInfo);
    BDBG_ASSERT(!errCode);
    pDevice->numAdc = pDevice->wfeInfo.numChannels;
    pDevice->satDevice->numWfe = pDevice->numAdc;

    BKNI_Free(pInitSettings);

    return NEXUS_SUCCESS;
err:
    return NEXUS_UNKNOWN;
}

static NEXUS_Error NEXUS_FrontendDevice_P_Init45308_InitAP(NEXUS_45308Device *pDevice)
{
    BERR_Code errCode;
    const uint8_t *fw = NULL;

    BDBG_MSG(("NEXUS_FrontendDevice_P_Init45308_InitAP"));

    BDBG_WRN(("Initializing %x Frontend core...", pDevice->chipId));
    /* Initialize the acquisition processor */
    fw = pDevice->fw;
    errCode = BHAB_InitAp(pDevice->satDevice->habHandle, fw);
    if ( errCode ) {
        BDBG_ERR(("Device initialization failed..."));

        errCode = BERR_TRACE(errCode);
        goto err;
    }
    BDBG_WRN(("Initializing 45308 core... Done"));

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

    BDBG_MSG(("NEXUS_FrontendDevice_P_Init45308_InitAP: done"));
    return NEXUS_SUCCESS;
err:
    return NEXUS_UNKNOWN;
}

static NEXUS_Error NEXUS_FrontendDevice_P_Init45308_PostInitAP(NEXUS_45308Device *pDevice)
{
    unsigned i;
    BERR_Code errCode;
    NEXUS_FrontendDevice45308OpenSettings *pSettings = &pDevice->settings;

    BDBG_MSG(("NEXUS_FrontendDevice_P_Init45308_PostInitAP"));

    BDBG_MSG(("Opening %d WFE channels", pDevice->wfeInfo.numChannels));
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
#endif

#if 0
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
#endif

#if NEXUS_HAS_MXT
    if (pSettings->mtsif[0].enabled || pSettings->mtsif[1].enabled) {
        /* open MXT */
        BMXT_Settings mxtSettings;
        BERR_Code rc;
        uint32_t val=0;
        bool encryptTx = false;

        BDBG_MSG(("NEXUS_FrontendDevice_Open45308: configuring MXT"));
        rc = BHAB_ReadRegister(pDevice->satDevice->habHandle, 0x7020400, &val);
        if (rc) {
            BERR_TRACE(rc);
        }

        BMXT_GetDefaultSettings(&mxtSettings);
        if (val && (((val & 0xFFFFFF00)>>8)==0x45316)) {
            mxtSettings.chip = BMXT_Chip_e45316;
        } else {
            mxtSettings.chip = BMXT_Chip_e45308;
        }
        mxtSettings.hHab = pDevice->satDevice->habHandle;

        NEXUS_Module_Lock(g_NEXUS_frontendModuleSettings.transport);
        encryptTx = NEXUS_TransportModule_P_IsMtsifEncrypted();
        NEXUS_Module_Unlock(g_NEXUS_frontendModuleSettings.transport);
        for (i=0; i<BMXT_NUM_MTSIF; i++) {
            mxtSettings.MtsifTxCfg[i].Enable = true;
            mxtSettings.MtsifTxCfg[i].Encrypt = encryptTx;
            mxtSettings.MtsifTxCfg[i].TxClockPolarity = 0;
        }

        mxtSettings.MtsifRxCfg[0].Enable = true;
        mxtSettings.MtsifRxCfg[0].RxClockPolarity = 1;
        NEXUS_Module_Lock(g_NEXUS_frontendModuleSettings.transport);
        mxtSettings.MtsifRxCfg[0].Decrypt = NEXUS_TransportModule_P_IsMtsifEncrypted();
        NEXUS_Module_Unlock(g_NEXUS_frontendModuleSettings.transport);
        if (val) {
            switch (val & 0xFF) {
            case 0x00:
                mxtSettings.chipRev = BMXT_ChipRev_eA0;
                break;
            case 0x10:
                mxtSettings.chipRev = BMXT_ChipRev_eB0;
                break;
            case 0x20:
                mxtSettings.chipRev = BMXT_ChipRev_eC0;
                break;
            default:
                BDBG_WRN(("Unrecognized chip revision, defaulting to C0"));
                mxtSettings.chipRev = BMXT_ChipRev_eC0;
                break;
            }
        }
#if NEXUS_HAS_453XX_PID_FILTERING
        mxtSettings.enablePidFiltering = true;
#endif

        BDBG_MSG(("NEXUS_FrontendDevice_Open45308: BMXT_Open"));
        rc = BMXT_Open(&pDevice->pGenericDeviceHandle->mtsifConfig.mxt, g_pCoreHandles->chp, g_pCoreHandles->reg, &mxtSettings);
        if (rc!=BERR_SUCCESS) goto err;
        BDBG_MSG(("NEXUS_FrontendDevice_Open45308: NEXUS_Frontend_P_InitMtsifConfig"));
        rc = NEXUS_Frontend_P_InitMtsifConfig(&pDevice->pGenericDeviceHandle->mtsifConfig, &mxtSettings);
        if (rc!=BERR_SUCCESS) goto err;

        if (!pSettings->mtsif[0].enabled) {
            unsigned numParsers = BMXT_GetNumResources(mxtSettings.chip, mxtSettings.chipRev, BMXT_ResourceType_eParser);
            BDBG_MSG(("NEXUS_FrontendDevice_Open45308: tx0 disabled, setting output to 1"));
            for (i=0; i<numParsers; i++) {
                BMXT_ParserConfig pConfig;
                BMXT_GetParserConfig(pDevice->pGenericDeviceHandle->mtsifConfig.mxt, i, &pConfig);
                pConfig.mtsifTxSelect = 1;
                BMXT_SetParserConfig(pDevice->pGenericDeviceHandle->mtsifConfig.mxt, i, &pConfig);
            }
        }

        BDBG_MSG(("NEXUS_FrontendDevice_Open45308: setting input bands to parallel"));
        {
            unsigned numInputBands;

            numInputBands = BMXT_GetNumResources(mxtSettings.chip, mxtSettings.chipRev, BMXT_ResourceType_eInputBand);
            for (i=0; i<numInputBands; i++) {
                BMXT_InputBandConfig pConfig;
                BMXT_GetInputBandConfig(pDevice->pGenericDeviceHandle->mtsifConfig.mxt, i, &pConfig);
                pConfig.ParallelInputSel = true;
                BMXT_SetInputBandConfig(pDevice->pGenericDeviceHandle->mtsifConfig.mxt, i, &pConfig);
            }
        }
        BDBG_MSG(("NEXUS_FrontendDevice_Open45308: done configuring MXT"));
    }
#endif

    {
        /* Change clocks to move out of wifi band */
        uint32_t val;
        uint32_t addr;
        BERR_Code e;

        addr = 0x0702801c;
        e = BHAB_ReadRegister(pDevice->satDevice->habHandle, addr, &val); if (e) BERR_TRACE(e);
        BDBG_MSG(("%08x: %08x", addr, val));
        BDBG_MSG(("VCO Old divider: %d", val & 0x000003FF));
        val &= ~0x000003FF;
        val |= 44;
        e = BHAB_WriteRegister(pDevice->satDevice->habHandle, addr, &val); if (e) BERR_TRACE(e);

        addr = 0x07028004;
        e = BHAB_ReadRegister(pDevice->satDevice->habHandle, addr, &val); if (e) BERR_TRACE(e);
        BDBG_MSG(("%08x: %08x", addr, val));
        BDBG_MSG(("DEMOD_XPT Old divider: %d", (val & 0x000001FE)>>1));
        val &= ~0x000001FE;
        val |= 8<<1;
        e = BHAB_WriteRegister(pDevice->satDevice->habHandle, addr, &val); if (e) BERR_TRACE(e);
    }

    for (i=0; i < 3; i++) {
        uint32_t val;
        uint32_t clockAddr;
        BERR_Code e;

        switch (i) {
        case 0:
            clockAddr = 0x07028008;
            break;
        case 1:
            clockAddr = 0x07028000;
            break;
        case 2:
            clockAddr = 0x07028014;
            break;
        /* This is here solely in case someone bumps the loop condition. */
        /* coverity[dead_error_begin] */
        /* coverity[dead_error_condition] */
        default:
            BDBG_ASSERT(false);
            BKNI_Fail();
            break;
        }

        if (!pSettings->mtsif[i].enabled) {
            e = BHAB_ReadRegister(pDevice->satDevice->habHandle, clockAddr, &val);
            if (e) BERR_TRACE(e);
            val &= 0xFFFFFFFE;
            val |= 1;
            e = BHAB_WriteRegister(pDevice->satDevice->habHandle, clockAddr, &val);
            if (e) BERR_TRACE(e);
        } else {
            e = BHAB_ReadRegister(pDevice->satDevice->habHandle, clockAddr, &val);
            if (e) BERR_TRACE(e);
            if (!e && (val & 1)) {
                val &= 0xFFFFFFFE;
                e = BHAB_WriteRegister(pDevice->satDevice->habHandle, clockAddr, &val);
                if (e) BERR_TRACE(e);
            }

            {
                unsigned clockRate = pSettings->mtsif[i].clockRate / 1000;
                unsigned divider;
                unsigned clock = 4752000;

                if (clockRate == 0) clockRate = 81000;

                divider = clock / (2*clockRate);

                e = BHAB_ReadRegister(pDevice->satDevice->habHandle, clockAddr, &val); if (e) BERR_TRACE(e);
                BDBG_MSG(("%08x: %08x", clockAddr, val));
                BDBG_MSG(("clockRate(%d) divider: %d", clockRate, divider));

                val &= ~0x000001fe;
                val |= (divider << 1);
                e = BHAB_WriteRegister(pDevice->satDevice->habHandle, clockAddr, &val); if (e) BERR_TRACE(e);
            }

            if (pSettings->mtsif[i].driveStrength != 0) {
                unsigned driveStrength = pSettings->mtsif[i].driveStrength;
                unsigned str;
                uint32_t driveAddr;

                switch (i) {
                case 0:
                    driveAddr = 0x07020460;
                    break;
                case 1:
                    driveAddr = 0x07020464;
                    break;
                case 2:
                    driveAddr = 0x07020468;
                    break;
                /* This is here solely in case someone bumps the loop condition. */
                /* coverity[dead_error_begin] */
                /* coverity[dead_error_condition] */
                default:
                    BDBG_ASSERT(false);
                    BKNI_Fail();
                    break;
                }

                e = BHAB_ReadRegister(pDevice->satDevice->habHandle, driveAddr, &val);
                if (e) BERR_TRACE(e);

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
                    BDBG_WRN(("NEXUS_FrontendDevice_Open45308: Unrecognized drive strength (MTSIF%d): %u, defaulting to 8 mA.", i, driveStrength));
                    str = 3; break;
                }
                val &= 0xFFFFFFF8;
                val |= str;
                e = BHAB_WriteRegister(pDevice->satDevice->habHandle, driveAddr, &val);
                if (e) BERR_TRACE(e);
            }

            {
                /* enable half-stagger */
                uint32_t staggerAddr;

                staggerAddr = 0x7105200 + 0x100*i;
                val = 0x98;
                e = BHAB_WriteRegister(pDevice->satDevice->habHandle, staggerAddr, &val);
                if (e) BERR_TRACE(e);

                staggerAddr += 0x4;
                val = 0x76543210 ;
                e = BHAB_WriteRegister(pDevice->satDevice->habHandle, staggerAddr, &val);
                if (e) BERR_TRACE(e);

            }
        }
    }

#if 0
    if (!pSettings->mtsif[0].enabled && !pSettings->mtsif[1].enabled) {
        uint32_t val;
        BHAB_ReadRegister(pDevice->satDevice->habHandle, 0x06920790, &val);
        val &= 0xFFFFFFFE;
        val |= 1;
        BHAB_WriteRegister(pDevice->satDevice->habHandle, 0x06920790, &val);
    }
#endif

    BDBG_MSG(("NEXUS_FrontendDevice_Open45308: setting openPending to false"));
    pDevice->pGenericDeviceHandle->openPending = false;

    BDBG_MSG(("NEXUS_FrontendDevice_Open45308: returning success"));
    return NEXUS_SUCCESS;
err:
    return NEXUS_UNKNOWN;
}

/* Helper function for asynchronous initialization thread */
static void NEXUS_Frontend_45308_P_OpenDeviceThread(void *arg) {
    NEXUS_45308Device *pDevice= (NEXUS_45308Device *)arg;
    NEXUS_Error errCode;

    errCode = NEXUS_FrontendDevice_P_Init45308_InitAP(pDevice);
    if (errCode) goto init_error;
    errCode = NEXUS_FrontendDevice_P_Init45308_PostInitAP(pDevice);
    if (errCode) goto init_error;

    return;

init_error:
    BDBG_ERR(("45308(%p) initialization failed...", (void *)pDevice));
    BKNI_EnterCriticalSection();
    pDevice->pGenericDeviceHandle->openPending = false;
    pDevice->pGenericDeviceHandle->openFailed = true;
    BKNI_LeaveCriticalSection();
}

static NEXUS_Error NEXUS_Frontend_45308_P_DelayedInitialization(NEXUS_FrontendDeviceHandle handle)
{
    NEXUS_FrontendDevice *pFrontendDevice = (NEXUS_FrontendDevice *)handle;
    NEXUS_45308Device *pDevice = (NEXUS_45308Device *)pFrontendDevice->pDevice;
    NEXUS_Error errCode;

    BDBG_MSG(("NEXUS_Frontend_45308_P_DelayedInitialization"));

    BDBG_MSG(("Connecting interrupt"));
    /* Successfully opened the 45308.  Connect interrupt */
    BHAB_GetInterruptEventHandle(pDevice->satDevice->habHandle, &pDevice->isrEvent);
    pDevice->isrCallback = NEXUS_RegisterEvent(pDevice->isrEvent, NEXUS_Frontend_P_45308_IsrCallback, pDevice);
    if ( NULL == pDevice->isrCallback )
    {
        errCode = BERR_TRACE(BERR_OS_ERROR);
        goto err;
    }

#if NEXUS_HAS_453XX_PID_FILTERING
    NEXUS_Frontend_P_EnablePidFiltering();
#endif

#ifdef NEXUS_HAS_FSK
    {
        BKNI_EventHandle event;
        unsigned i;

        BDBG_MSG(("Connecting FSK callbacks"));
        /* hook up the FTM data ready callback*/
        for(i=0; i<pDevice->numFskChannels; i++){
            errCode = BFSK_GetRxEventHandle(pDevice->fskChannels[i], &event);
            if (errCode) {
                errCode = BERR_TRACE(errCode);
                goto err;
            }

            pDevice->ftmEventCallback[i] = NEXUS_RegisterEvent(event, NEXUS_Frontend_P_45308_FtmEventCallback, pDevice);
            if (!pDevice->ftmEventCallback[i]) {
                errCode = BERR_TRACE(NEXUS_UNKNOWN);
                goto err;
            }
        }
    }
#endif

    return NEXUS_SUCCESS;

err:
    return errCode;
}

static NEXUS_Error NEXUS_FrontendDevice_P_Init45308(NEXUS_45308Device *pDevice)
{
    NEXUS_Error errCode;

    errCode = NEXUS_FrontendDevice_P_Init45308_PreInitAP(pDevice);
    if (!errCode)
        errCode = NEXUS_FrontendDevice_P_Init45308_InitAP(pDevice);
    if (!errCode)
        errCode = NEXUS_FrontendDevice_P_Init45308_PostInitAP(pDevice);

    if (!errCode)
        errCode = NEXUS_Frontend_45308_P_DelayedInitialization(pDevice->pGenericDeviceHandle);

    return errCode;
}

NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_Open45308(unsigned index, const NEXUS_FrontendDevice45308OpenSettings *pSettings)
{
    NEXUS_FrontendDevice *pFrontendDevice = NULL;
    NEXUS_45308Device *pDevice=NULL;

    BSTD_UNUSED(index);

    /* Check is maintained in case we need to introduce a list of devices later. */
    if (pDevice == NULL) {
        NEXUS_FrontendSatDeviceSettings satDeviceSettings;

        BERR_Code errCode;

        BDBG_MSG(("Opening new 45308 device"));

        pFrontendDevice = NEXUS_FrontendDevice_P_Create();
        if (NULL == pFrontendDevice) { BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err; }

        pDevice = BKNI_Malloc(sizeof(*pDevice));
        if (NULL == pDevice) { BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err; }
        BKNI_Memset(pDevice, 0, sizeof(*pDevice));
        BDBG_OBJECT_SET(pDevice, NEXUS_45308Device);
        pDevice->settings = *pSettings;
        pDevice->pGenericDeviceHandle = pFrontendDevice;
#if NEXUS_FRONTEND_HAS_A8299_DISEQC
        pDevice->A8299_control = 0x88; /* 13v, 13v default */
#endif

        NEXUS_Frontend_P_Sat_GetDefaultDeviceSettings(&satDeviceSettings);

        pDevice->satDevice = NEXUS_Frontend_P_Sat_Create_Device(&satDeviceSettings);
        if (pDevice->satDevice == NULL) { BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err; }

        errCode = NEXUS_FrontendDevice_P_Init45308_PreInitAP(pDevice);
        if (errCode) goto err;

        pFrontendDevice->delayedInit = NEXUS_Frontend_45308_P_DelayedInitialization;
        pFrontendDevice->delayedInitializationRequired = true;

        BKNI_EnterCriticalSection();
        pDevice->pGenericDeviceHandle->openPending = true;
        pDevice->pGenericDeviceHandle->openFailed = false;
        BKNI_LeaveCriticalSection();

#if DISABLE_45308_ASYNC_INIT
        NEXUS_Frontend_45308_P_OpenDeviceThread(pDevice);
#else
        {
            NEXUS_ThreadSettings thread_settings;
            NEXUS_Thread_GetDefaultSettings(&thread_settings);
            thread_settings.priority = 0;
            pDevice->deviceOpenThread = NEXUS_Thread_Create("deviceOpenThread",
                                                            NEXUS_Frontend_45308_P_OpenDeviceThread,
                                                            (void*)pDevice,
                                                            &thread_settings);
        }
#endif

    }

    pFrontendDevice->pDevice = pDevice;
    pFrontendDevice->familyId = 0x45308;
    pFrontendDevice->getCapabilities = NEXUS_FrontendDevice_P_45308_GetCapabilities;
    pFrontendDevice->application = NEXUS_FrontendDeviceApplication_eSatellite;
    pFrontendDevice->getStatus = NEXUS_FrontendDevice_P_45308_GetStatus;
    pFrontendDevice->close = NEXUS_Frontend_P_45308_DestroyDevice;
    pFrontendDevice->getSatelliteCapabilities = NEXUS_FrontendDevice_P_Get45308Capabilities;

    pFrontendDevice->mode = NEXUS_StandbyMode_eOn;
    pFrontendDevice->standby = NEXUS_FrontendDevice_P_45308_Standby;

    pFrontendDevice->nonblocking.getCapabilities = true; /* does not require init complete to fetch number of demods */
    pFrontendDevice->nonblocking.getSatelliteCapabilities = true; /* does not require init complete to fetch number of demods */

    BDBG_MSG(("NEXUS_FrontendDevice_Open45308: returning %p", (void *)pFrontendDevice));
    return pFrontendDevice;

err:
    if (pDevice)
        NEXUS_Frontend_P_45308_DestroyDevice(pDevice);
    return NULL;
}

static void NEXUS_Frontend_P_Uninit45308(NEXUS_45308Device *pDevice)
{
    unsigned i;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_45308Device);

    BDBG_MSG(("Closing 45308 device %p handle", (void *)pDevice));

    if (pDevice->deviceOpenThread)
        NEXUS_Thread_Destroy(pDevice->deviceOpenThread);
    pDevice->deviceOpenThread = NULL;

#if NEXUS_HAS_MXT
    if (pDevice->pGenericDeviceHandle) {
        if (pDevice->pGenericDeviceHandle->mtsifConfig.mxt) {
            BMXT_Close(pDevice->pGenericDeviceHandle->mtsifConfig.mxt);
            pDevice->pGenericDeviceHandle->mtsifConfig.mxt = NULL;
        }
        BKNI_Memset((void *)&pDevice->pGenericDeviceHandle->mtsifConfig, 0, sizeof(pDevice->pGenericDeviceHandle->mtsifConfig));
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
        BDBG_MSG(("Unregister isrCallback %p", (void *)pDevice->isrCallback));
        NEXUS_UnregisterEvent(pDevice->isrCallback);
        pDevice->isrCallback = NULL;
        BDBG_MSG(("...done"));
    }

    if (pDevice->settings.isrNumber) {
        NEXUS_Core_DisconnectInterrupt(pDevice->settings.isrNumber);
    } else if (pDevice->settings.gpioInterrupt) {
        NEXUS_GpioSettings gpioSettings;
        NEXUS_Gpio_SetInterruptCallback_priv(pDevice->settings.gpioInterrupt, NULL, NULL, 0);
        NEXUS_Gpio_GetSettings(pDevice->settings.gpioInterrupt, &gpioSettings);
        gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
        NEXUS_Gpio_SetSettings(pDevice->settings.gpioInterrupt, &gpioSettings);
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


void NEXUS_Frontend_GetDefault45308Settings( NEXUS_45308FrontendSettings *pSettings )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_FrontendHandle NEXUS_Frontend_Open45308( const NEXUS_45308FrontendSettings *pSettings )
{
    NEXUS_FrontendHandle frontend = NULL;
    NEXUS_45308Device *pDevice = NULL;
    NEXUS_FrontendDevice *pFrontendDevice = NULL;
    NEXUS_FrontendSatChannelSettings satChannelSettings;
    BREG_I2C_Handle i2cHandle = NULL;

    if(pSettings->device == NULL) {
        NEXUS_FrontendDevice45308OpenSettings openSettings;
        NEXUS_FrontendDevice_GetDefault45308OpenSettings(&openSettings);
        pFrontendDevice = NEXUS_FrontendDevice_Open45308(0, &openSettings);
        pDevice = (NEXUS_45308Device *)pFrontendDevice->pDevice;
    }
    else {
        pFrontendDevice = pSettings->device;
        pDevice = (NEXUS_45308Device *)pSettings->device->pDevice;
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
#define B_SAT_CHIP 45308
    satChannelSettings.satChip = B_SAT_CHIP;
    satChannelSettings.channelIndex = pSettings->channelNumber;
    satChannelSettings.pCloseParam = pDevice;
    satChannelSettings.pDevice = pDevice;
    satChannelSettings.closeFunction = NEXUS_Frontend_P_45308_CloseCallback;
    satChannelSettings.diseqcIndex = 0;
    satChannelSettings.capabilities.diseqc = true;
    if (pDevice->settings.diseqc.i2cDevice) {
        satChannelSettings.getDiseqcChannelHandle = NEXUS_Frontend_P_45308_GetDiseqcChannelHandle;
        satChannelSettings.getDiseqcChannelHandleParam = pDevice;
        satChannelSettings.setVoltage = NEXUS_Frontend_P_45308_SetVoltage;
    }
    satChannelSettings.i2cRegHandle = i2cHandle; /* due to module locking, we need to save our register handle for Diseqc voltage control */
#if 0
    satChannelSettings.getDiseqcEventHandle = NEXUS_Frontend_P_45308_GetDiseqcEventHandle;
    satChannelSettings.getDiseqcAppCallback = NEXUS_Frontend_P_45308_GetDiseqcAppCallback;
    satChannelSettings.setDiseqcAppCallback = NEXUS_Frontend_P_45308_SetDiseqcAppCallback;
#endif
    satChannelSettings.getDefaultDiseqcSettings = NEXUS_Frontend_P_45308_GetDefaultDiseqcSettings;
#if NEXUS_HAS_FSK
    satChannelSettings.getFskChannelHandle = NEXUS_Frontend_P_45308_GetFskChannelHandle;
#endif

    satChannelSettings.wfeHandle = pDevice->satDevice->wfeHandle;

    satChannelSettings.deviceClearSpectrumCallbacks = NEXUS_Frontend_P_45308_ClearSpectrumCallbacks;

    frontend = NEXUS_Frontend_P_Sat_Create_Channel(&satChannelSettings);
    if ( !frontend )
    {
        BERR_TRACE(BERR_NOT_SUPPORTED);
        NEXUS_Frontend_P_45308_CloseCallback(NULL, pDevice); /* Check if channel needs to be closed */
        goto err;
    }
    frontend->tuneSatellite = NEXUS_Frontend_P_45308_TuneSatellite;
    frontend->untune = NEXUS_Frontend_P_45308_Untune;
    frontend->reapplyTransportSettings = NEXUS_Frontend_P_45308_ReapplyTransportSettings;
    frontend->pGenericDeviceHandle = pFrontendDevice;
    frontend->getSatelliteAgcStatus = NEXUS_Frontend_P_45308_GetSatelliteAgcStatus;
    frontend->getSatelliteRuntimeSettings = NEXUS_Frontend_P_Get45308RuntimeSettings;
    frontend->setSatelliteRuntimeSettings = NEXUS_Frontend_P_Set45308RuntimeSettings;

    frontend->standby = NEXUS_Frontend_P_45308_Standby;

    frontend->readRegister = NEXUS_Frontend_P_45308_ReadRegister;
    frontend->writeRegister = NEXUS_Frontend_P_45308_WriteRegister;

    /* preconfigure mtsif settings so platform doesn't need to */
    frontend->userParameters.isMtsif = true;
    frontend->mtsif.inputBand = pSettings->channelNumber + 2; /* IB for demod 0 is 2, demod 1 is 3, etc. Offset channel by 2 to compensate. */

    pDevice->handles[pSettings->channelNumber] = frontend;

    return frontend;

err:
    return NULL;
}

static void NEXUS_Frontend_P_45308_CloseCallback(NEXUS_FrontendHandle handle, void *pParam)
{
    unsigned i;
    NEXUS_45308Device *pDevice = pParam;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_45308Device);

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

static void NEXUS_Frontend_P_45308_DestroyDevice(void *handle)
{
    unsigned i;
    NEXUS_45308Device *pDevice = (NEXUS_45308Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_45308Device);

    for ( i = 0; i < pDevice->numChannels; i++ )
    {
        if ( NULL != pDevice->handles[i] )
        {
            BDBG_ERR(("All channels must be closed before destroying device"));
            BDBG_ASSERT(NULL == pDevice->handles[i]);
        }
    }

    NEXUS_Frontend_P_Uninit45308(pDevice);

    BDBG_MSG(("Destroying 45308 device %p", (void *)pDevice));

    if (pDevice->pGenericDeviceHandle) {
        BKNI_Free(pDevice->pGenericDeviceHandle);
        pDevice->pGenericDeviceHandle = NULL;
    }

    if (pDevice->satDevice) {
        BKNI_Free(pDevice->satDevice);
        pDevice->satDevice = NULL;
    }
#if 0
    BLST_S_REMOVE(&g_deviceList, pDevice, NEXUS_45308Device, node);
#endif
    BDBG_OBJECT_DESTROY(pDevice, NEXUS_45308Device);
    BKNI_Free(pDevice);
}

void NEXUS_FrontendDevice_P_45308_S3Standby(NEXUS_45308Device *pDevice)
{
    NEXUS_Frontend_P_Uninit45308(pDevice);
}

static NEXUS_Error NEXUS_FrontendDevice_P_45308_Standby(void *handle, const NEXUS_StandbySettings *pSettings)
{

    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_45308Device *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_45308Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_45308Device);

    if ((pDevice->pGenericDeviceHandle->mode != NEXUS_StandbyMode_eDeepSleep) && (pSettings->mode == NEXUS_StandbyMode_eDeepSleep)) {

        BDBG_MSG(("NEXUS_FrontendDevice_P_45308_Standby: Entering deep sleep..."));

        NEXUS_FrontendDevice_P_45308_S3Standby(pDevice);

        BKNI_EnterCriticalSection();
        pDevice->pGenericDeviceHandle->openPending = true;
        pDevice->pGenericDeviceHandle->openFailed = false;
        BKNI_LeaveCriticalSection();

    } else if ((pDevice->pGenericDeviceHandle->mode == NEXUS_StandbyMode_eDeepSleep) && (pSettings->mode != NEXUS_StandbyMode_eDeepSleep)) {

        BDBG_MSG(("NEXUS_FrontendDevice_P_45308_Standby: Waking up..."));
        BDBG_MSG(("NEXUS_FrontendDevice_P_45308_Standby: reinitializing..."));
        rc = NEXUS_FrontendDevice_P_Init45308(pDevice);
        if (rc) { rc = BERR_TRACE(rc); goto done;}
    }

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_45308_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings)
{
    NEXUS_SatChannel *pSatChannel = (NEXUS_SatChannel *)handle;
    NEXUS_45308Device *p45308Device;

    BDBG_OBJECT_ASSERT(pSatChannel, NEXUS_SatChannel);
    p45308Device = pSatChannel->settings.pDevice;

    BDBG_MSG(("NEXUS_Frontend_P_45308_Standby: standby %p(%d) %s", handle, pSatChannel->channel, enabled ? "enabled" : "disabled"));

    BDBG_MSG(("Restoring handles on %p", (void *)pSatChannel));
    /* update/restore handles */
    pSatChannel->satChannel = p45308Device->satChannels[pSatChannel->channel];

    if (pSettings->mode == NEXUS_StandbyMode_eDeepSleep) {
        BDBG_MSG(("Unregistering events on %p", (void *)pSatChannel));
        NEXUS_Frontend_P_Sat_UnregisterEvents(pSatChannel);
    } else if (pSettings->mode != NEXUS_StandbyMode_eDeepSleep && pSatChannel->frontendHandle->mode == NEXUS_StandbyMode_eDeepSleep) {
        BDBG_MSG(("Registering events on %p", (void *)pSatChannel));
        NEXUS_Frontend_P_Sat_RegisterEvents(pSatChannel);
    }

    BDBG_MSG(("Done with standby configuration on %p", (void *)pSatChannel));
    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_FrontendDevice_P_Get45308Capabilities(void *handle, NEXUS_FrontendSatelliteCapabilities *pCapabilities)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_45308Device *p45308Device = (NEXUS_45308Device *)handle;

    BDBG_ASSERT(handle);
    BDBG_ASSERT(pCapabilities);

    BDBG_OBJECT_ASSERT(p45308Device, NEXUS_45308Device);

    BKNI_Memset(pCapabilities,0,sizeof(*pCapabilities));
    pCapabilities->numAdc = p45308Device->numAdc;
    pCapabilities->numChannels = p45308Device->numChannels;
    pCapabilities->externalBert = true;

    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_Get45308RuntimeSettings(void *handle, NEXUS_FrontendSatelliteRuntimeSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BERR_Code e;
    NEXUS_SatChannel *pSatChannel = (NEXUS_SatChannel *)handle;
    NEXUS_45308Device *p45308Device;
    BSAT_ExternalBertSettings extBertSettings;

    BDBG_ASSERT(handle);
    BDBG_ASSERT(pSettings);
    p45308Device = pSatChannel->settings.pDevice;
    BDBG_OBJECT_ASSERT(p45308Device, NEXUS_45308Device);

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    pSettings->selectedAdc = pSatChannel->selectedAdc;
    pSatChannel->diseqcIndex = pSatChannel->selectedAdc;

    e = BSAT_GetExternalBertSettings(p45308Device->satDevice->satHandle, &extBertSettings);
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

static NEXUS_Error NEXUS_Frontend_P_Set45308RuntimeSettings(void *handle, const NEXUS_FrontendSatelliteRuntimeSettings *pSettings)
{
    NEXUS_SatChannel *pSatChannel = (NEXUS_SatChannel *)handle;
    NEXUS_45308Device *p45308Device;
    BERR_Code e;
    BSAT_ExternalBertSettings extBertSettings;

    BDBG_ASSERT(handle);
    BDBG_ASSERT(pSettings);
    p45308Device = pSatChannel->settings.pDevice;
    BDBG_OBJECT_ASSERT(p45308Device, NEXUS_45308Device);

    BDBG_MSG(("adc: %d, mask: 0x%08x",pSettings->selectedAdc,p45308Device->wfeInfo.availChannelsMask));
    /* Ensure the requested adc is within the value range, and advertised by the PI as being available */
    if (pSettings->selectedAdc > p45308Device->numAdc || !((1<<pSettings->selectedAdc) & p45308Device->wfeInfo.availChannelsMask) )
        return NEXUS_INVALID_PARAMETER;

    pSatChannel->selectedAdc = pSettings->selectedAdc;
    pSatChannel->diseqcIndex = pSettings->selectedAdc;
    p45308Device->wfeMap[pSatChannel->channel] = pSettings->selectedAdc;
    pSatChannel->satDevice->wfeMap[pSatChannel->channel] = pSettings->selectedAdc;

    e = BSAT_GetExternalBertSettings(p45308Device->satDevice->satHandle, &extBertSettings);
    if (!e) {
        if (pSettings->externalBert.enabled) {
            extBertSettings.channel = pSatChannel->channel;
            extBertSettings.bEnable = pSettings->externalBert.enabled;
            extBertSettings.bClkInv = pSettings->externalBert.invertClock;
            BSAT_SetExternalBertSettings(p45308Device->satDevice->satHandle, &extBertSettings);
        } else {
            /* Only disable it if it's enabled on the current channel, otherwise we might be
             * interfering with the settings on a different demod. */
            if (extBertSettings.channel == pSatChannel->channel) {
                extBertSettings.channel = pSatChannel->channel;
                extBertSettings.bEnable = pSettings->externalBert.enabled;
                extBertSettings.bClkInv = pSettings->externalBert.invertClock;
                BSAT_SetExternalBertSettings(p45308Device->satDevice->satHandle, &extBertSettings);
            }
        }
    } else {
        BERR_TRACE(e);
    }

    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_45308_GetSatelliteAgcStatus(void *handle, NEXUS_FrontendSatelliteAgcStatus *pStatus)
{
    NEXUS_SatChannel *pSatChannel = (NEXUS_SatChannel *)handle;
    NEXUS_45308Device *p45308Device;
    BERR_Code rc = NEXUS_SUCCESS;
    BSAT_ChannelStatus satStatus;
    BERR_Code errCode;

    BDBG_ASSERT(NULL != pStatus);
    p45308Device = pSatChannel->settings.pDevice;
    BDBG_OBJECT_ASSERT(p45308Device, NEXUS_45308Device);

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

static NEXUS_Error NEXUS_Frontend_P_45308_TuneSatellite(void *handle, const NEXUS_FrontendSatelliteSettings *pSettings)
{
    NEXUS_SatChannel *pSatChannel = handle;
    NEXUS_45308Device *p45308Device = pSatChannel->settings.pDevice;
    NEXUS_Error rc;

    if (pSettings->mode == NEXUS_FrontendSatelliteMode_eTurboQpsk || pSettings->mode == NEXUS_FrontendSatelliteMode_eTurbo8psk || pSettings->mode == NEXUS_FrontendSatelliteMode_eTurbo) {
        uint32_t disabledFeatures = 0;
        rc = BSAT_GetConfig(pSatChannel->satDevice->satHandle, BSAT_45308_CONFIG_PARAM_OTP_DISABLE_FEATURE, &disabledFeatures);
        if (rc) BERR_TRACE(rc);
        if (disabledFeatures & 0x1) {
            BDBG_ERR(("Turbo modes are not supported on this frontend."));
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
        BHAB_45308_InitXp(p45308Device->satDevice->habHandle, 0); /* 0 = enable turbo acquisitions */
    }

    if (p45308Device->pGenericDeviceHandle->mtsifConfig.mxt) {
        rc = NEXUS_Frontend_P_SetMtsifConfig(pSatChannel->frontendHandle);
        if (rc) { return BERR_TRACE(rc); }
    }

    return NEXUS_Frontend_P_Sat_TuneSatellite(pSatChannel, pSettings);
}

static void NEXUS_Frontend_P_45308_Untune(void *handle)
{
    NEXUS_SatChannel *pSatChannel = handle;
    NEXUS_45308Device *p45308Device = pSatChannel->settings.pDevice;

    if (p45308Device->pGenericDeviceHandle->mtsifConfig.mxt) {
        NEXUS_Frontend_P_UnsetMtsifConfig(pSatChannel->frontendHandle);
    }

    NEXUS_Frontend_P_Sat_Untune(pSatChannel);

    return;
}

NEXUS_Error NEXUS_Frontend_P_45308_ReapplyTransportSettings(void *handle)
{
    NEXUS_SatChannel *pSatChannel = handle;
    NEXUS_45308Device *p45308Device = pSatChannel->settings.pDevice;
    BERR_Code rc;

    if (p45308Device->pGenericDeviceHandle->mtsifConfig.mxt) {
        rc = NEXUS_Frontend_P_SetMtsifConfig(pSatChannel->frontendHandle);
        if (rc) { return BERR_TRACE(rc); }
    }

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Frontend_P_45308_ReadRegister(void *handle, unsigned address, uint32_t *pValue)
{
    NEXUS_SatChannel *pSatChannel = handle;
    NEXUS_45308Device *p45308Device;
    BERR_Code rc;
    BDBG_ASSERT(pSatChannel);
    p45308Device = pSatChannel->settings.pDevice;
    BDBG_OBJECT_ASSERT(p45308Device, NEXUS_45308Device);

    rc = BHAB_ReadRegister(p45308Device->satDevice->habHandle, address, pValue);
    if (rc) { return BERR_TRACE(rc); }
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Frontend_P_45308_WriteRegister(void *handle, unsigned address, uint32_t value)
{
    NEXUS_SatChannel *pSatChannel = handle;
    NEXUS_45308Device *p45308Device;
    BERR_Code rc;
    BDBG_ASSERT(pSatChannel);
    p45308Device = pSatChannel->settings.pDevice;
    BDBG_OBJECT_ASSERT(p45308Device, NEXUS_45308Device);

    rc = BHAB_WriteRegister(p45308Device->satDevice->habHandle, address, &value);
    if (rc) { return BERR_TRACE(rc); }
    return NEXUS_SUCCESS;
}

static BDSQ_ChannelHandle NEXUS_Frontend_P_45308_GetDiseqcChannelHandle(void *handle, int index)
{
    NEXUS_SatChannel *pSatChannel = handle;
    NEXUS_45308Device *p45308Device;
    p45308Device = pSatChannel->settings.pDevice;
    return p45308Device->satDevice->dsqChannels[index];
}

static NEXUS_Error NEXUS_Frontend_P_45308_SetVoltage(void *pDevice, NEXUS_FrontendDiseqcVoltage voltage)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_SatChannel *pSatChannel = pDevice;
    NEXUS_45308Device *p45308Device = pSatChannel->settings.pDevice;

    /* Just in case DSQ book-keeping requires it: */
    BDSQ_SetVoltage(p45308Device->satDevice->dsqChannels[pSatChannel->diseqcIndex], voltage == NEXUS_FrontendDiseqcVoltage_e18v);

#if NEXUS_FRONTEND_HAS_A8299_DISEQC
    { /* Write voltage to A8299 */
        int channel = pSatChannel->diseqcIndex;
        uint8_t buf[2];
        uint8_t i2c_addr, shift, ctl;
        uint8_t A8299_control = p45308Device->A8299_control;

        i2c_addr = p45308Device->settings.diseqc.i2cAddr;

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

        p45308Device->A8299_control = A8299_control;

        BDBG_MSG(("A8299: channel=%d, i2c_addr=0x%X, ctl=0x%02X 0x%02X", channel, i2c_addr, buf[0], buf[1]));
        rc = BREG_I2C_WriteNoAddr(pSatChannel->settings.i2cRegHandle, i2c_addr, buf, 2);
        if (rc) return BERR_TRACE(rc);

    }
#endif

    return rc;
}

static void NEXUS_Frontend_P_45308_GetDefaultDiseqcSettings(void *pDevice, BDSQ_ChannelSettings *settings)
{
    NEXUS_SatChannel *pSatChannel = pDevice;
    NEXUS_45308Device *p45308Device = pSatChannel->settings.pDevice;
    BDSQ_GetChannelDefaultSettings(p45308Device->satDevice->dsqHandle, 0, settings);
}

static void NEXUS_FrontendDevice_P_45308_GetCapabilities(void *handle, NEXUS_FrontendDeviceCapabilities *pCapabilities)
{
    NEXUS_45308Device *pDevice = (NEXUS_45308Device *)handle;

    BDBG_ASSERT(handle);
    BDBG_ASSERT(pCapabilities);
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_45308Device);

    pCapabilities->numTuners = pDevice->numChannels;
}

static void NEXUS_Frontend_P_45308_ClearSpectrumCallbacks(void *handle)
{
    NEXUS_SatChannel *pSatChannel = (NEXUS_SatChannel *)handle;
    NEXUS_45308Device *p45308Device;
    unsigned i;

    BDBG_ASSERT(pSatChannel);
    p45308Device = pSatChannel->settings.pDevice;
    BDBG_OBJECT_ASSERT(p45308Device, NEXUS_45308Device);

    for (i=0; i < p45308Device->numChannels; i++) {
        NEXUS_SatChannel *pChannel = p45308Device->handles[i]->pDeviceHandle;
        if (pChannel) {
            if (pChannel->spectrumEventCallback) {
                NEXUS_UnregisterEvent(pChannel->spectrumEventCallback);
                pChannel->spectrumEventCallback = NULL;
            }
        }
    }
}

static NEXUS_Error NEXUS_FrontendDevice_P_45308_GetStatus(void *handle, NEXUS_FrontendDeviceStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_45308Device *pDevice = NULL;
    BHAB_AvsData avsData;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_45308Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_45308Device);
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
static BFSK_ChannelHandle NEXUS_Frontend_P_45308_GetFskChannelHandle(void *pDevice, int index)
{
    NEXUS_SatChannel *pSatChannel = pDevice;
    NEXUS_45308Device *p45308Device = pSatChannel->settings.pDevice;

    if((unsigned)index < p45308Device->numFskChannels)
        return p45308Device->fskChannels[index];
    else
        return NULL;
}

static void NEXUS_Frontend_P_45308_FtmEventCallback(void *context)
{
    NEXUS_SatChannel *pSatChannel;
    NEXUS_45308Device *p45308Device = context;
    unsigned i;

    for (i=0;i<NEXUS_45308_MAX_FRONTEND_CHANNELS;i++) {
        if (p45308Device->handles[i]) {
            pSatChannel = (NEXUS_SatChannel *)p45308Device->handles[i]->pDeviceHandle;
            if(pSatChannel->ftmCallback)
                NEXUS_TaskCallback_Fire(pSatChannel->ftmCallback);
        }
    }
}

#endif

NEXUS_Error NEXUS_Frontend_P_Get45308ChipInfo(const NEXUS_FrontendDevice45308OpenSettings *pSettings, NEXUS_45308ProbeResults *pResults) {
    NEXUS_Error rc = NEXUS_SUCCESS;

    BSTD_UNUSED(pSettings);
    pResults->chip.id = 0x45308;

    return rc;
}

static uint32_t NEXUS_Platform_P_I2c_Get45308ChipId(NEXUS_I2cHandle i2cDevice, uint16_t i2cAddr)
{
    BREG_I2C_Handle i2cHandle;
    uint8_t buf[5];
    uint32_t chipId=0;
    uint8_t subAddr;

    i2cHandle = NEXUS_I2c_GetRegHandle(i2cDevice, NULL);
    BDBG_MSG(("i2c handle: %p, i2caddr: 0x%x", (void *)i2cHandle,i2cAddr));
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
static uint32_t NEXUS_Platform_P_Spi_Get45308ChipId(NEXUS_SpiHandle spiDevice, uint16_t spiAddr)
{
    uint32_t chipId=0;
    uint8_t wData[2], rData[8];
    NEXUS_Error rc;

    BDBG_MSG(("Probing for 45308 at SPI 0x%02x",spiAddr));

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
NEXUS_Error NEXUS_Frontend_Probe45308(const NEXUS_FrontendDevice45308OpenSettings *pSettings, NEXUS_45308ProbeResults *pResults)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pResults);

    BKNI_Memset(pResults, 0, sizeof(*pResults));

    if (pSettings->i2cDevice) {
        pResults->chip.familyId = (uint32_t)NEXUS_Platform_P_I2c_Get45308ChipId(pSettings->i2cDevice, pSettings->i2cAddr);
        if ( pResults->chip.familyId != 0x45308 )
        {
            BDBG_MSG(("pResults->chip.familyId = 0x%x", pResults->chip.familyId));
            rc = BERR_INVALID_PARAMETER; goto done;
        }
    } else if (pSettings->spiDevice) {
        pResults->chip.familyId = (uint32_t)NEXUS_Platform_P_Spi_Get45308ChipId(pSettings->spiDevice, pSettings->spiAddr);
        if ( pResults->chip.familyId != 0x45308 )
        {
            BDBG_MSG(("pResults->chip.familyId = 0x%x", pResults->chip.familyId));
            rc = BERR_INVALID_PARAMETER; goto done;
        }
    } else { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto done; }

    rc = NEXUS_Frontend_P_Get45308ChipInfo(pSettings, pResults);

done:
    return rc;
}

NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_P_Open45308(unsigned index, const NEXUS_FrontendDeviceOpenSettings *pSettings)
{
    NEXUS_FrontendDevice45308OpenSettings settings;
    int i;

    NEXUS_FrontendDevice_GetDefault45308OpenSettings(&settings);
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

    return NEXUS_FrontendDevice_Open45308(index, &settings);
}

NEXUS_FrontendHandle NEXUS_Frontend_P_Open45308(const NEXUS_FrontendChannelSettings *pSettings)
{
    NEXUS_45308FrontendSettings settings;

    NEXUS_Frontend_GetDefault45308Settings(&settings);
    settings.device = pSettings->device;
    settings.channelNumber = pSettings->channelNumber;
    return NEXUS_Frontend_Open45308(&settings);
}

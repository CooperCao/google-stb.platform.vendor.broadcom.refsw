/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 *****************************************************************************/
#include "nexus_frontend_module.h"
#include "nexus_platform_features.h"
#include "nexus_spi.h"
#include "priv/nexus_spi_priv.h"
#include "nexus_i2c.h"
#include "priv/nexus_i2c_priv.h"
#include "priv/nexus_gpio_priv.h"
#include "btnr.h"
#include "btnr_7563ib.h"
#include "bhab_7563.h"
#include "bods_7563.h"
#include "bhab_7563_fw.h"
#include "bhab.h"
#include "bods.h"
#include "bchp_sun_top_ctrl.h"
#include "nexus_avs.h"
#include "bhab_ctfe_img.h"
#include "priv/nexus_core_img.h"
#include "priv/nexus_core_img_id.h"

BDBG_MODULE(nexus_frontend_7563);

BDBG_OBJECT_ID(NEXUS_7563);

#define BCHIP_7563 0x7563

#define NEXUS_MAX_7563_FRONTENDS 2

#define NEXUS_7563_DVBT_CHN      (0)
#define NEXUS_7563_DVBT2_CHN     (NEXUS_7563_DVBT_CHN  + 1)
#define NEXUS_7563_MAX_OFDM_CHN  (NEXUS_MAX_7563_FRONTENDS)

/* The 7563 has one downstream. */
#define NEXUS_MAX_7563_TUNERS  1

#define TOTAL_SOFTDECISIONS 30

/***************************************************************************
 * Internal Module Structure
 ***************************************************************************/

typedef struct NEXUS_PreviousStatus
{
    unsigned fecCorrected;
    unsigned fecUncorrected;
    unsigned fecClean;
    unsigned viterbiUncorrectedBits;
    unsigned viterbiTotalBits;
    NEXUS_Time time;
}NEXUS_PreviousStatus;

typedef struct NEXUS_7563
{
    BDBG_OBJECT(NEXUS_7563)
    uint32_t  familyId;
    uint32_t  chipId;
    BHAB_Handle hab;
    BHAB_Capabilities capabilities;
    unsigned    isrNumber;
    NEXUS_GpioHandle gpioInterrupt;
    uint16_t    i2cAddr;
    NEXUS_I2cHandle i2cHandle;
    uint16_t    spiAddr;
    NEXUS_SpiHandle spiHandle;
    unsigned    numfrontends;
    uint16_t revId;
    bool adsOpenDrain;
    BODS_Handle ods;
    BKNI_EventHandle isrEvent;
    uint8_t lastChannel;
    NEXUS_EventCallbackHandle isrEventCallback;
    unsigned agcValue;                        /* Gain Value*/
    NEXUS_CallbackDesc updateGainCallbackDesc;    /* Callback will be called when the gain from the lna needs to be updated. */
    NEXUS_3461TunerRfInput rfInput;
    NEXUS_3461RfDaisyChain rfDaisyChain;
    bool enableRfLoopThrough;
    bool terrestrial;
    bool acquireInProgress;
    signed count;
    NEXUS_FrontendDvbt2Status t2PartialStatus;
    BODS_SelectiveStatus odsStatus;
    bool isStatusReady;
    NEXUS_FrontendHandle       frontendHandle;
    NEXUS_FrontendDevice *pGenericDeviceHandle;
    BTNR_Handle                tnr[NEXUS_MAX_7563_TUNERS];
    BODS_ChannelHandle         ods_chn[NEXUS_7563_MAX_OFDM_CHN];
    NEXUS_FrontendOfdmSettings last_ofdm[NEXUS_MAX_7563_TUNERS];
    NEXUS_IsrCallbackHandle    lockAppCallback[NEXUS_MAX_7563_FRONTENDS];
    NEXUS_IsrCallbackHandle    asyncStatusAppCallback[NEXUS_MAX_7563_FRONTENDS];
    NEXUS_IsrCallbackHandle    updateGainAppCallback[NEXUS_MAX_7563_TUNERS];
    bool                       isAsyncStatusReady[NEXUS_MAX_7563_FRONTENDS];
    bool                       isTunerPoweredOn;
    bool                       isPoweredOn[NEXUS_MAX_7563_FRONTENDS];
    NEXUS_FrontendOfdmAcquisitionMode lastAcquisitionMode[NEXUS_MAX_7563_FRONTENDS];
    NEXUS_StandbyMode previousStandbyMode;
    NEXUS_FrontendDevice3461OpenSettings openSettings;
} NEXUS_7563;

typedef struct NEXUS_7563Channel
{
    unsigned chn_num; /* channel number */
    NEXUS_7563 *pDevice; /* 7563 device*/
} NEXUS_7563Channel;

/***************************************************************************
 * Module callback functions for tuning
 ***************************************************************************/
static void NEXUS_Frontend_P_7563_UninstallCallbacks(void *handle);
static void NEXUS_Frontend_P_7563_UnTune(void *handle);
static void NEXUS_Frontend_P_7563_ResetStatus(void *handle);
static NEXUS_Error NEXUS_Frontend_P_7563_ReadSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length, size_t *pNumRead);
static uint16_t NEXUS_Frontend_P_Get7563Rev(const NEXUS_FrontendDevice3461OpenSettings *pSettings);
static void NEXUS_Frontend_P_7563_Close(NEXUS_FrontendHandle handle);
static void NEXUS_Frontend_P_7563_GetType(void *handle, NEXUS_FrontendType *type);
static NEXUS_Error NEXUS_Frontend_P_7563_GetFastStatus(void *handle, NEXUS_FrontendFastStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_7563_TuneOfdm(void *handle, const NEXUS_FrontendOfdmSettings *pSettings);
static NEXUS_Error NEXUS_Frontend_P_7563_GetOfdmStatus(void *handle, NEXUS_FrontendOfdmStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_7563_GetOfdmAsyncStatus(void *handle, NEXUS_FrontendOfdmStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_7563_RequestOfdmAsyncStatus(void *handle);
static NEXUS_Error NEXUS_Frontend_P_7563_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings);

static NEXUS_Error NEXUS_Frontend_P_7563_RequestDvbt2AsyncStatus(void *handle, NEXUS_FrontendDvbt2StatusType type);
static NEXUS_Error NEXUS_Frontend_P_7563_GetDvbt2AsyncStatusReady(void *handle, NEXUS_FrontendDvbt2StatusReady *pStatusReady);
static NEXUS_Error NEXUS_Frontend_P_7563_GetDvbt2AsyncStatus(void *handle, NEXUS_FrontendDvbt2StatusType type, NEXUS_FrontendDvbt2Status *pStatus);
static void NEXUS_Frontend_P_PrintDvbt2PartialStatus(NEXUS_FrontendDvbt2Status *pStatus);

static NEXUS_Error NEXUS_Frontend_P_7563_RequestDvbtAsyncStatus(void *handle, NEXUS_FrontendDvbtStatusType type);
static NEXUS_Error NEXUS_Frontend_P_7563_GetDvbtAsyncStatusReady(void *handle, NEXUS_FrontendDvbtStatusReady *pAsyncStatusReady);
static NEXUS_Error NEXUS_Frontend_P_7563_GetDvbtAsyncStatus(void *handle, NEXUS_FrontendDvbtStatusType type, NEXUS_FrontendDvbtStatus *pStatus);
static NEXUS_Error NEXUS_FrontendDevice_P_7563_GetStatus(void * handle, NEXUS_FrontendDeviceStatus *pStatus);

static NEXUS_Error NEXUS_Tuner_P_7563_GetNSetGain(void *handle);
static void NEXUS_FrontendDevice_P_7563_GetCapabilities(void *handle, NEXUS_FrontendDeviceCapabilities *pCapabilities);
static void NEXUS_FrontendDevice_P_7563_GetTunerCapabilities(void *handle, unsigned tunerIndex, NEXUS_FrontendCapabilities *pCapabilities);
static NEXUS_Error NEXUS_FrontendDevice_P_7563_Standby(void * handle, const NEXUS_StandbySettings *pSettings);
NEXUS_Error NEXUS_FrontendDevice_P_7563_GetInternalGain(void *handle, const NEXUS_GainParameters *params, NEXUS_InternalGainSettings *pSettings);
NEXUS_Error NEXUS_FrontendDevice_P_7563_GetExternalGain(void *handle, NEXUS_ExternalGainSettings *pSettings);
NEXUS_Error NEXUS_FrontendDevice_P_7563_SetExternalGain(void *handle, const NEXUS_ExternalGainSettings *pSettings);
static void NEXUS_FrontendDevice_P_7563_Close(void *handle);
#if NEXUS_AMPLIFIER_SUPPORT
NEXUS_Error NEXUS_FrontendDevice_P_7563_GetAmplifierStatus(void *handle, NEXUS_AmplifierStatus *pStatus);
NEXUS_Error NEXUS_FrontendDevice_P_7563_SetAmplifierStatus(void *handle, const NEXUS_AmplifierStatus *pStatus);
#endif


/***************************************************************************
Summary:
    Lock callback handler for a 7563 Inband device
 ***************************************************************************/
static void NEXUS_Frontend_P_7563_callback_isr(void *pParam)
{
    NEXUS_7563 *pDevice;
    BDBG_ASSERT(pParam != NULL);
    pDevice = (NEXUS_7563 *)pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);

    if(pDevice->acquireInProgress){
        pDevice->count++;
    }
    if(pDevice->count == 2){
        pDevice->acquireInProgress = false;
        pDevice->count = 0;
    }
    if ( pDevice->lockAppCallback[pDevice->lastChannel] )
    {
        NEXUS_IsrCallback_Fire_isr(pDevice->lockAppCallback[pDevice->lastChannel]);
    }
}

/***************************************************************************
Summary:
    Lock callback handler for a 7563 Inband device
 ***************************************************************************/
static void NEXUS_Frontend_P_7563_AsyncStatusCallback_isr(void *pParam)
{
    NEXUS_7563 *pDevice;
    BDBG_ASSERT(pParam != NULL);
    pDevice = (NEXUS_7563 *)pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);

    if (pDevice->asyncStatusAppCallback[pDevice->lastChannel])
    {
        pDevice->isStatusReady = true;
        NEXUS_IsrCallback_Fire_isr(pDevice->asyncStatusAppCallback[pDevice->lastChannel]);
    }
}

/***************************************************************************
Summary:
    Enable/Disable interrupts for a 7563 device
 ***************************************************************************/
static void NEXUS_Frontend_P_7563_L1_isr(void *pParam1, int pParam2)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHAB_Handle hab = (BHAB_Handle)pParam2;
    BSTD_UNUSED(pParam1);

    rc = BHAB_HandleInterrupt_isr(hab);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return;
}
/***************************************************************************
Summary:
    Enable/Disable interrupts for a 7563 device
 ***************************************************************************/
static void NEXUS_Frontend_P_7563_GpioIsrControl_isr(bool enable, void *pParam)
{
    NEXUS_GpioHandle gpioHandle = (NEXUS_GpioHandle)pParam;

    if(enable){
        NEXUS_Gpio_SetInterruptEnabled_isr(gpioHandle, true);
    }
    else {
        NEXUS_Gpio_SetInterruptEnabled_isr(gpioHandle, false);
    }
}

/***************************************************************************
Summary:
    Enable/Disable interrupts for a 7563 device
 ***************************************************************************/
static void NEXUS_Frontend_P_7563_IsrControl_isr(bool enable, void *pParam)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    int isrnumber = (int)pParam;

    if ( enable )
    {
        rc = NEXUS_Core_EnableInterrupt_isr(isrnumber);
        if(rc) BERR_TRACE(rc);
    }
    else
    {
        NEXUS_Core_DisableInterrupt_isr(isrnumber);
    }
}

/***************************************************************************
Summary:
    ISR Event Handler for a 7563 device
 ***************************************************************************/
static void NEXUS_Frontend_P_7563_IsrEvent(void *pParam)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHAB_Handle hab = (BHAB_Handle)pParam;

    rc = BHAB_ProcessInterruptEvent(hab);
    if(rc) BERR_TRACE(rc);
}
/***************************************************************************
Summary:
    Initialize Hab for a 7563 device
***************************************************************************/
void NEXUS_Frontend_P_UnInit_7563_Hab(NEXUS_7563 *pDevice)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_GpioSettings gpioSettings;

    if(pDevice->isrNumber) {
        NEXUS_Core_DisconnectInterrupt(pDevice->isrNumber);
    }
    else if(pDevice->gpioInterrupt){
        NEXUS_Gpio_SetInterruptCallback_priv(pDevice->gpioInterrupt, NULL, NULL, 0);
        NEXUS_Gpio_GetSettings(pDevice->gpioInterrupt,  &gpioSettings);
        gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
        gpioSettings.interrupt.callback = NULL;
        NEXUS_Gpio_SetSettings(pDevice->gpioInterrupt, &gpioSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    if(pDevice->isrEvent) pDevice->isrEvent = NULL;
    if(pDevice->isrEventCallback)NEXUS_UnregisterEvent(pDevice->isrEventCallback);
    pDevice->isrEventCallback = NULL;
    if(pDevice->hab) {
        rc = BHAB_Close(pDevice->hab);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    pDevice->hab = NULL;
done:
    return;
}

/***************************************************************************
Summary:
    Initialize Hab for a 7563 device
***************************************************************************/
NEXUS_Error NEXUS_Frontend_P_Init_7563_Hab(NEXUS_7563 *pDevice, const NEXUS_FrontendDevice3461OpenSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHAB_Settings stHabSettings;

    rc = BHAB_7563_GetDefaultSettings(&stHabSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    stHabSettings.chipAddr = pSettings->i2cAddr;
    stHabSettings.slaveChipAddr = pSettings->i2cSlaveAddr;

    if(pSettings->isrNumber) {
        stHabSettings.interruptEnableFunc = NEXUS_Frontend_P_7563_IsrControl_isr;
        stHabSettings.interruptEnableFuncParam = (void*)pSettings->isrNumber;
    }
    else if(pSettings->gpioInterrupt){
        stHabSettings.interruptEnableFunc = NEXUS_Frontend_P_7563_GpioIsrControl_isr;
        stHabSettings.interruptEnableFuncParam = (void*)pSettings->gpioInterrupt;
    }

    rc = BHAB_Open( &pDevice->hab, (void *)g_pCoreHandles->reg, &stHabSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    /* Success opeining Hab.  Connect Interrupt */
    if(pSettings->isrNumber) {
        rc = NEXUS_Core_ConnectInterrupt(pSettings->isrNumber, NEXUS_Frontend_P_7563_L1_isr, (void *)pDevice, (int)pDevice->hab);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    else if(pSettings->gpioInterrupt){
        NEXUS_Gpio_SetInterruptCallback_priv(pSettings->gpioInterrupt, NEXUS_Frontend_P_7563_L1_isr, (void *)pDevice, (int)pDevice->hab);
    }

    if (pSettings->loadAP)
    {
        uint8_t *fw = NULL;
        const uint8_t *fw_image = NULL;
#if NEXUS_MODE_driver
        {
            unsigned fw_size = 0;
            BIMG_Interface imgInterface;
            void *pImgContext;
            void *pImg;
            uint8_t *pImage;
            unsigned header_size = 313;
            unsigned code_size = 0;
            unsigned num_chunks, chunk_size = MAX_CTFE_IMG_CHUNK_SIZE;
            unsigned chunk;

            rc = Nexus_Core_P_Img_Create(NEXUS_CORE_IMG_ID_FRONTEND_7563, &pImgContext, &imgInterface);
            if (rc) { BERR_TRACE(rc); goto done; }
            rc = imgInterface.open((void*)pImgContext, &pImg, 0);
            if (rc) { BERR_TRACE(rc); goto done; }
            rc = imgInterface.next(pImg, 0, (const void **)&pImage, header_size);
            if (rc) { BERR_TRACE(rc); goto done; }
            code_size = (pImage[72] << 24) | (pImage[73] << 16) | (pImage[74] << 8) | pImage[75];
            fw_size = code_size + header_size;
            rc = NEXUS_Memory_Allocate(fw_size, NULL, (void **)&fw);
            if (rc) { BERR_TRACE(rc); goto done; }

            num_chunks = fw_size / chunk_size;
            if (fw_size % chunk_size != 0) num_chunks++;

            BKNI_Memset(fw, 0, fw_size);
            for (chunk=0; chunk < num_chunks; chunk++) {
                unsigned num_to_read = chunk_size;
                if (chunk==num_chunks-1) num_to_read = fw_size % chunk_size;
                rc = imgInterface.next(pImg, chunk, (const void **)&pImage, num_to_read);
                if (rc) { BERR_TRACE(rc); goto done; }
                BKNI_Memcpy(fw + (chunk*chunk_size), pImage, num_to_read);
            }
            imgInterface.close(pImg);
            fw_image = fw;
        }
#else
        BSTD_UNUSED(fw);
        fw_image = bcm7563_leap_image;
#endif
        rc = BHAB_InitAp(pDevice->hab, fw_image);
#if NEXUS_MODE_driver
        NEXUS_Memory_Free(fw);
#endif
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    /* Get events and register callbacks */
    rc = BHAB_GetInterruptEventHandle(pDevice->hab, &pDevice->isrEvent);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pDevice->isrEventCallback = NEXUS_RegisterEvent(pDevice->isrEvent, NEXUS_Frontend_P_7563_IsrEvent, pDevice->hab);
    if ( NULL == pDevice->isrEventCallback ){rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done; }

    return BERR_SUCCESS;

done:
    NEXUS_Frontend_P_UnInit_7563_Hab(pDevice);
    return rc;
}

/***************************************************************************
Summary:
    Get the default settings for a BCM7563 tuner
See Also:
    NEXUS_Frontend_Open7563
 ***************************************************************************/
void NEXUS_Frontend_GetDefault3461Settings(NEXUS_3461Settings *pSettings)
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->type = NEXUS_3461ChannelType_eDvbt2;
    pSettings->channelNumber = 0;
    pSettings->inBandOpenDrain = true;
    pSettings->loadAP = true;
    pSettings->i2cSlaveAddr = 0x60;
}
/***************************************************************************
Summary:
    UnInitialize all THD/T2 channels.
 ***************************************************************************/
void NEXUS_Frontend_P_UnInit7563(NEXUS_7563 *pDevice)
{
    NEXUS_Error rc = BERR_SUCCESS;
    unsigned int i;

    for ( i = 0; i < NEXUS_MAX_7563_TUNERS && NULL != pDevice->tnr[i]; i++) {
        if(pDevice->tnr[i]){
            rc = BTNR_Close(pDevice->tnr[i]);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->tnr[i] = NULL;
        }
    }
    for ( i = 0; i < NEXUS_7563_MAX_OFDM_CHN && NULL != pDevice->ods_chn[i]; i++) {
        if(pDevice->ods_chn[i]){
            rc = BODS_CloseChannel(pDevice->ods_chn[i]);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            (pDevice->ods_chn[i]) = NULL;
        }
    }
    if (pDevice->ods) {
        rc = BODS_Close(pDevice->ods);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->ods = NULL;
    }

done:
    return;
}

static void NEXUS_FrontendDevice_P_7563_GetCapabilities(void *handle, NEXUS_FrontendDeviceCapabilities *pCapabilities)
{
    NEXUS_7563 *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7563 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);

    pCapabilities->numTuners = pDevice->capabilities.totalTunerChannels;
    return;
}

static void NEXUS_FrontendDevice_P_7563_GetTunerCapabilities(void *handle, unsigned tunerIndex, NEXUS_FrontendCapabilities *pCapabilities)
{
    NEXUS_7563 *pDevice;
    BHAB_ChannelCapability *channelCapability;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7563 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);
    BSTD_UNUSED(tunerIndex);

    channelCapability = pDevice->capabilities.channelCapabilities;

    BKNI_Memset(pCapabilities, 0, sizeof(*pCapabilities));

    if(channelCapability[tunerIndex].demodCoreType.ads) pCapabilities->qam = true;
    if(channelCapability[tunerIndex].demodCoreType.aob) pCapabilities->outOfBand = true;
    if(channelCapability[tunerIndex].demodCoreType.dvbt) {pCapabilities->ofdm = true;pCapabilities->ofdmModes[NEXUS_FrontendOfdmMode_eDvbt]= true;}
    if(channelCapability[tunerIndex].demodCoreType.dvbt2) {pCapabilities->ofdm = true;pCapabilities->ofdmModes[NEXUS_FrontendOfdmMode_eDvbt2]= true;}
    if(channelCapability[tunerIndex].demodCoreType.dvbc2) {pCapabilities->ofdm = true;pCapabilities->ofdmModes[NEXUS_FrontendOfdmMode_eDvbc2]= true;}
    if(channelCapability[tunerIndex].demodCoreType.isdbt) {pCapabilities->ofdm = true;pCapabilities->ofdmModes[NEXUS_FrontendOfdmMode_eIsdbt]= true;}
    if(channelCapability[tunerIndex].demodCoreType.ifdac) pCapabilities->ifdac = true;

    return;
}

/***************************************************************************
Summary:
    Initialize all THD/T2 channels.
 ***************************************************************************/
static NEXUS_Error NEXUS_Frontend_P_Init7563(NEXUS_7563 *pDevice)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned int i, j, num_ch;
    BTNR_7563_Settings tnr7563_cfg;
    BODS_Settings odsCfg;
    BODS_ChannelSettings odsChnCfg;
    unsigned acc_chn_num = 0;

    rc =  BHAB_GetTunerChannels(pDevice->hab, &pDevice->capabilities.totalTunerChannels);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if(!pDevice->capabilities.channelCapabilities){
        pDevice->capabilities.channelCapabilities = (BHAB_ChannelCapability *) BKNI_Malloc( pDevice->capabilities.totalTunerChannels*sizeof(BHAB_ChannelCapability));
        if(!pDevice->capabilities.channelCapabilities){
            rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done;
        }
    }

    rc =  BHAB_GetCapabilities(pDevice->hab, &pDevice->capabilities);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = BODS_7563_GetDefaultSettings( &odsCfg, NULL);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    odsCfg.hGeneric = pDevice->hab;
    rc = BODS_Open(&pDevice->ods, NULL, NULL, NULL, &odsCfg);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = BODS_Init(pDevice->ods);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    for (i=0;i<BODS_Standard_eLast;i++) {
        rc = BODS_GetTotalChannels(pDevice->ods, (BODS_Standard)i, &num_ch);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        if(num_ch > NEXUS_MAX_7563_TUNERS) {
            BDBG_ERR(("The maximum number of channels is incorrect. num_ch = %d", num_ch));
            rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
        }

        rc = BODS_GetChannelDefaultSettings( pDevice->ods, (BODS_Standard)i, &odsChnCfg);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        if(i == BODS_Standard_eDvbt2){
            odsChnCfg.hHeap = g_pCoreHandles->heap[0].mem;

        }
        for (j=0;j<num_ch;j++) {
            odsChnCfg.channelNo = j;
            if(acc_chn_num + j >= NEXUS_7563_MAX_OFDM_CHN) {
                BDBG_ERR(("The maximum number of total ODS channels %d is more than expected. ", acc_chn_num + j));
                rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
            }
            rc = BODS_OpenChannel( pDevice->ods, &pDevice->ods_chn[acc_chn_num + j], &odsChnCfg);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            rc = BODS_InstallCallback(pDevice->ods_chn[acc_chn_num + j], BODS_Callback_eLockChange, (BODS_CallbackFunc)NEXUS_Frontend_P_7563_callback_isr, (void*)pDevice);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            rc = BODS_InstallCallback(pDevice->ods_chn[acc_chn_num + j], BODS_Callback_eAsyncStatusReady, (BODS_CallbackFunc)NEXUS_Frontend_P_7563_AsyncStatusCallback_isr, (void*)pDevice);
            if(rc){rc = BERR_TRACE(rc); goto done;}
        }

        acc_chn_num += num_ch;
    }

    for (i=0;i<NEXUS_MAX_7563_TUNERS;i++) {
        rc = BTNR_7563_GetDefaultSettings(&tnr7563_cfg);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        rc =  BTNR_7563_Open(&pDevice->tnr[i],&tnr7563_cfg, pDevice->hab);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    return BERR_SUCCESS;

done:
    NEXUS_Frontend_P_UnInit7563(pDevice);
    return rc;
}

static uint16_t NEXUS_Frontend_P_Get7563Rev(const NEXUS_FrontendDevice3461OpenSettings *pSettings)
{
    uint16_t revId=0xef;
    BSTD_UNUSED(pSettings);

    revId = 0;

    BDBG_MSG(("revId = 0x%x", revId));
    return revId;
}

#if NEXUS_AMPLIFIER_SUPPORT
NEXUS_Error NEXUS_FrontendDevice_P_7563_GetAmplifierStatus(void *handle, NEXUS_AmplifierStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7563 *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7563 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    if(pDevice->pGenericDeviceHandle->parent != NULL){
        rc = NEXUS_Frontend_P_GetAmplifierStatus(pDevice->pGenericDeviceHandle->parent, pStatus);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    else if(pDevice->pGenericDeviceHandle->amplifier != NULL){
        rc = NEXUS_Amplifier_GetStatus(pDevice->pGenericDeviceHandle->amplifier, pStatus);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    else {
        BDBG_ERR(("Amplifier not linked to the parent device."));
    }

done:
    return rc;
}

NEXUS_Error NEXUS_FrontendDevice_P_7563_SetAmplifierStatus(void *handle, const NEXUS_AmplifierStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7563 *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7563 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);

    BSTD_UNUSED(pStatus);
    /* Set LNA/Amplifier parameters. */

    return rc;
}
#endif

NEXUS_Error NEXUS_FrontendDevice_P_7563_GetInternalGain(void *handle, const NEXUS_GainParameters *params, NEXUS_InternalGainSettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7563 *pDevice;
    BHAB_InternalGainInputParams inputParams;
    BHAB_InternalGainSettings internalGain;
    NEXUS_GainParameters gainParams;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7563 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);

    pSettings->frequency = params->frequency;
    inputParams.frequency = params->frequency;

    rc = BHAB_GetInternalGain(pDevice->hab, &inputParams, &internalGain);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    if(params->accumulateTillRootDevice){
        if(pDevice->pGenericDeviceHandle->parent){
            gainParams.rfInput = pDevice->pGenericDeviceHandle->linkSettings.rfInput;
            gainParams.accumulateTillRootDevice = params->accumulateTillRootDevice;
            gainParams.frequency = params->frequency;
            rc = NEXUS_FrontendDevice_P_GetInternalGain(pDevice->pGenericDeviceHandle->parent, &gainParams, pSettings);
            if(rc){rc = BERR_TRACE(rc); goto done;}
        }
    }

    if(params->rfInput == NEXUS_FrontendDeviceRfInput_eDaisy){
        pSettings->totalVariableGain += internalGain.internalGainDaisy;
    }
    else if(params->rfInput == NEXUS_FrontendDeviceRfInput_eLoopThrough){
        pSettings->totalVariableGain += internalGain.internalGainLoopThrough;
    }

    pSettings->daisyGain += internalGain.internalGainDaisy;
    pSettings->loopThroughGain += internalGain.internalGainLoopThrough;
done:
    return rc;
}
NEXUS_Error NEXUS_FrontendDevice_P_7563_GetExternalGain(void *handle, NEXUS_ExternalGainSettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7563 *pDevice;
    BHAB_ExternalGainSettings gain;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7563 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);
    BSTD_UNUSED(pSettings);

    rc = BHAB_GetExternalGain(pDevice->hab, &gain);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pSettings->bypassableGain = gain.externalGainBypassable;
    pSettings->totalGain = gain.externalGainTotal;

done:
    return rc;

}
NEXUS_Error NEXUS_FrontendDevice_P_7563_SetExternalGain(void *handle, const NEXUS_ExternalGainSettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7563 *pDevice;
    BHAB_ExternalGainSettings externalGain;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7563 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);

    externalGain.externalGainTotal = pSettings->totalGain;
    externalGain.externalGainBypassable = pSettings->bypassableGain;

    rc = BHAB_SetExternalGain(pDevice->hab, &externalGain);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}

/***************************************************************************
Summary:
    Get the default settings for a BCM3461 device.
See Also:
    NEXUS_FrontendDevice_Open3461
 ***************************************************************************/
void NEXUS_FrontendDevice_GetDefault3461OpenSettings(NEXUS_FrontendDevice3461OpenSettings *pSettings)
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->loadAP = true;
    pSettings->i2cSlaveAddr = 0x60;
    pSettings->inBandOpenDrain = true;
}

NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_Open3461(unsigned index, const NEXUS_FrontendDevice3461OpenSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7563 *pDevice = NULL;
    NEXUS_FrontendDevice *pFrontendDevice = NULL;
    NEXUS_3461ProbeResults results;
    BSTD_UNUSED(index);

    BDBG_ASSERT(NULL != pSettings);

    rc = NEXUS_Frontend_Probe3461(pSettings, &results);
    if(rc){rc = BERR_TRACE(rc); goto err_create;}

    if ( NULL == pDevice)
    {
        BCHP_Info info;
        BCHP_GetInfo(g_pCoreHandles->chp, &info);
        info.rev = NEXUS_Frontend_P_Get7563Rev(pSettings);

        pFrontendDevice = BKNI_Malloc(sizeof(*pFrontendDevice));
        if (NULL == pFrontendDevice) {rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_create;}

        /* Memsetting the whole structure should cover initializing the child list. */
        BKNI_Memset(pFrontendDevice, 0, sizeof(*pFrontendDevice));

        pDevice = BKNI_Malloc(sizeof(NEXUS_7563));
        if (NULL == pDevice) {rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_create;}

        BKNI_Memset(pDevice, 0, sizeof(NEXUS_7563));
        BDBG_OBJECT_SET(pDevice, NEXUS_7563);

        pDevice->familyId = info.familyId;
        pDevice->chipId = info.productId;
        pDevice->revId = info.rev;
        pDevice->i2cAddr = pSettings->i2cAddr;
        pDevice->spiAddr = pSettings->spiAddr;
        pDevice->isrNumber = pSettings->isrNumber;
        pDevice->gpioInterrupt = pSettings->gpioInterrupt;
        pDevice->i2cHandle = pSettings->i2cDevice;
        pDevice->spiHandle = pSettings->spiDevice;
        pDevice->adsOpenDrain = pSettings->inBandOpenDrain;
        pDevice->pGenericDeviceHandle = pFrontendDevice;
        pDevice->previousStandbyMode = NEXUS_StandbyMode_eOn;
        pDevice->openSettings = *pSettings;

        rc = NEXUS_Frontend_P_Init_7563_Hab(pDevice, pSettings);
        if(rc){rc = BERR_TRACE(rc); goto err_init_hab;}

        /* Initialize THD and T2 channels*/
        rc = NEXUS_Frontend_P_Init7563(pDevice);
        if(rc){rc = BERR_TRACE(rc); goto err_init;}

    }
    else
    {
        BDBG_MSG(("Found device"));

    }

    pFrontendDevice->pDevice = pDevice;
    pFrontendDevice->familyId = results.chip.familyId;
    pFrontendDevice->bypassableFixedGain = pSettings->externalFixedGain.bypassable;
    pFrontendDevice->totalFixedBoardGain = pSettings->externalFixedGain.total;
    pFrontendDevice->mode = NEXUS_StandbyMode_eOn;
    pFrontendDevice->getInternalGain = NEXUS_FrontendDevice_P_7563_GetInternalGain;
    pFrontendDevice->getExternalGain = NEXUS_FrontendDevice_P_7563_GetExternalGain;
    pFrontendDevice->setExternalGain = NEXUS_FrontendDevice_P_7563_SetExternalGain;
    pFrontendDevice->getStatus = NEXUS_FrontendDevice_P_7563_GetStatus;
    pFrontendDevice->standby = NEXUS_FrontendDevice_P_7563_Standby;
    pFrontendDevice->close = NEXUS_FrontendDevice_P_7563_Close;
    pFrontendDevice->getCapabilities = NEXUS_FrontendDevice_P_7563_GetCapabilities;
    pFrontendDevice->getTunerCapabilities = NEXUS_FrontendDevice_P_7563_GetTunerCapabilities;
#if NEXUS_AMPLIFIER_SUPPORT
    pFrontendDevice->getAmplifierStatus = NEXUS_FrontendDevice_P_7563_GetAmplifierStatus;
    pFrontendDevice->setAmplifierStatus = NEXUS_FrontendDevice_P_7563_SetAmplifierStatus;
#endif
    return pFrontendDevice;

err_init:
    NEXUS_Frontend_P_UnInit_7563_Hab(pDevice);
err_init_hab:
    if ( pDevice ) BKNI_Free(pDevice);
    pDevice = NULL;
err_create:
    if(pFrontendDevice) BKNI_Free(pFrontendDevice);
    return NULL;

}

/***************************************************************************
Summary:
    Open a handle to a BCM7563 device.
 ***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Open3461(const NEXUS_3461Settings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_FrontendHandle frontendHandle = NULL;
    NEXUS_7563 *pDevice = NULL;
    unsigned chn_num=0;
    NEXUS_FrontendDevice *pFrontendDevice = NULL;
    NEXUS_FrontendDevice3461OpenSettings openSettings;

    if(pSettings->device == NULL) {
        NEXUS_FrontendDevice_GetDefault3461OpenSettings(&openSettings);
        openSettings.configureWatchdog = pSettings->configureWatchdog;
        openSettings.gpioInterrupt = pSettings->gpioInterrupt;
        openSettings.i2cAddr = pSettings->i2cAddr;
        openSettings.i2cDevice  = pSettings->i2cDevice;
        openSettings.i2cSlaveAddr  = pSettings->i2cSlaveAddr;
        openSettings.inBandOpenDrain  = pSettings->inBandOpenDrain;
        openSettings.isMtsif  = pSettings->isMtsif;
        openSettings.isrNumber  = pSettings->isrNumber;
        openSettings.loadAP  = pSettings->loadAP;
        openSettings.spiAddr  = pSettings->spiAddr;
        openSettings.spiDevice  = pSettings->spiDevice;
        pFrontendDevice = NEXUS_FrontendDevice_Open3461(0, &openSettings);
        pDevice = (NEXUS_7563 *)pFrontendDevice->pDevice;
    }
    else {
        pDevice = (NEXUS_7563 *)pSettings->device->pDevice;
        pFrontendDevice =  pSettings->device;
    }

    /* Create a Nexus frontend handle */
    frontendHandle = NEXUS_Frontend_P_Create(pDevice);
    if ( NULL == frontendHandle ) {rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_alloc;}

    /* Establish device capabilities */
    frontendHandle->capabilities.ofdm = true;
    frontendHandle->capabilities.ofdmModes[NEXUS_FrontendOfdmMode_eDvbt] = true;
    frontendHandle->capabilities.ofdmModes[NEXUS_FrontendOfdmMode_eDvbt2] = true;

    frontendHandle->requestDvbt2AsyncStatus = NEXUS_Frontend_P_7563_RequestDvbt2AsyncStatus;
    frontendHandle->getDvbt2AsyncStatusReady = NEXUS_Frontend_P_7563_GetDvbt2AsyncStatusReady;
    frontendHandle->getDvbt2AsyncStatus = NEXUS_Frontend_P_7563_GetDvbt2AsyncStatus;
    frontendHandle->requestOfdmAsyncStatus = NEXUS_Frontend_P_7563_RequestOfdmAsyncStatus;
    frontendHandle->getOfdmAsyncStatus = NEXUS_Frontend_P_7563_GetOfdmAsyncStatus;
    frontendHandle->getOfdmStatus = NEXUS_Frontend_P_7563_GetOfdmStatus;
    frontendHandle->tuneOfdm = NEXUS_Frontend_P_7563_TuneOfdm;
    frontendHandle->untune = NEXUS_Frontend_P_7563_UnTune;
    frontendHandle->resetStatus = NEXUS_Frontend_P_7563_ResetStatus;
    frontendHandle->readSoftDecisions = NEXUS_Frontend_P_7563_ReadSoftDecisions;
    frontendHandle->close = NEXUS_Frontend_P_7563_Close;
    frontendHandle->getFastStatus = NEXUS_Frontend_P_7563_GetFastStatus;
    frontendHandle->getType = NEXUS_Frontend_P_7563_GetType;
    frontendHandle->standby = NEXUS_Frontend_P_7563_Standby;
    frontendHandle->requestDvbtAsyncStatus = NEXUS_Frontend_P_7563_RequestDvbtAsyncStatus;
    frontendHandle->getDvbtAsyncStatusReady = NEXUS_Frontend_P_7563_GetDvbtAsyncStatusReady;
    frontendHandle->getDvbtAsyncStatus = NEXUS_Frontend_P_7563_GetDvbtAsyncStatus;
    frontendHandle->uninstallCallbacks = NEXUS_Frontend_P_7563_UninstallCallbacks;

    frontendHandle->pGenericDeviceHandle = pFrontendDevice;

    /* Create app callbacks */
    for(chn_num=0; chn_num < NEXUS_MAX_7563_FRONTENDS; chn_num++){

        pDevice->lockAppCallback[chn_num] = NEXUS_IsrCallback_Create(frontendHandle, NULL);
        if ( NULL == pDevice->lockAppCallback[chn_num] ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err_cbk_create;}

        pDevice->asyncStatusAppCallback[chn_num] = NEXUS_IsrCallback_Create(frontendHandle, NULL);
        if ( NULL == pDevice->asyncStatusAppCallback[chn_num] ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err_cbk_create;}
     }

    pDevice->updateGainAppCallback[0] = NEXUS_IsrCallback_Create(frontendHandle, NULL);
    if ( NULL == pDevice->updateGainAppCallback[0] ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err_cbk_create;}

    pDevice->frontendHandle = frontendHandle;
    frontendHandle->chip.familyId = pDevice->familyId;
    frontendHandle->chip.id = pDevice->chipId;
    return frontendHandle;

err_cbk_create:
    for(chn_num=0; chn_num < NEXUS_MAX_7563_FRONTENDS; chn_num++){
        if ( pDevice->lockAppCallback[chn_num] ) NEXUS_IsrCallback_Destroy(pDevice->lockAppCallback[chn_num]);
        if ( pDevice->asyncStatusAppCallback[chn_num] ) NEXUS_IsrCallback_Destroy(pDevice->asyncStatusAppCallback[chn_num]);
    }
    if ( pDevice->updateGainAppCallback[0]) NEXUS_IsrCallback_Destroy(pDevice->updateGainAppCallback[0]);
    if ( frontendHandle ) BKNI_Free(frontendHandle);
err_alloc:
    NEXUS_Frontend_P_UnInit7563(pDevice);
    return NULL;
}

/***************************************************************************
Summary:
    Close a handle to a BCM7563 device.
See Also:
    NEXUS_Frontend_Open7563
 ***************************************************************************/
static void NEXUS_Frontend_P_7563_Close(NEXUS_FrontendHandle handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7563 *pDevice;
    unsigned chn_num=0;
    BTNR_PowerSaverSettings pwrSettings;

    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    pDevice = (NEXUS_7563 *) handle->pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);

    for(chn_num=0; chn_num< NEXUS_MAX_7563_FRONTENDS; chn_num++){
        pDevice->lastChannel = chn_num;
        if(pDevice->isPoweredOn[chn_num]) {
            NEXUS_Frontend_P_7563_UnTune(pDevice);
        }
    }
    if(pDevice->isTunerPoweredOn){
        pwrSettings.enable = true;
        rc = BTNR_SetPowerSaver(pDevice->tnr[0], &pwrSettings);
        if(rc){rc = BERR_TRACE(rc);}
    }

   for(chn_num=0; chn_num < NEXUS_MAX_7563_FRONTENDS; chn_num++){
        if ( pDevice->lockAppCallback[chn_num] ){
            BODS_InstallCallback(pDevice->ods_chn[chn_num], BODS_Callback_eLockChange, NULL, NULL);
            NEXUS_IsrCallback_Destroy(pDevice->lockAppCallback[chn_num]);
        }
        if ( pDevice->asyncStatusAppCallback[chn_num] ){
             BODS_InstallCallback(pDevice->ods_chn[chn_num], BODS_Callback_eAsyncStatusReady, NULL, NULL);
             NEXUS_IsrCallback_Destroy(pDevice->asyncStatusAppCallback[chn_num]);
        }
    }
    if ( pDevice->updateGainAppCallback[0] ) NEXUS_IsrCallback_Destroy(pDevice->updateGainAppCallback[0]);

    NEXUS_Frontend_P_Destroy(handle);

    pDevice->frontendHandle = NULL;
}

static void NEXUS_FrontendDevice_P_7563_Close(void *handle)
{
    NEXUS_7563 *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7563 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);

    NEXUS_Frontend_P_UnInit7563(pDevice);
    NEXUS_Frontend_P_UnInit_7563_Hab(pDevice);

    NEXUS_FrontendDevice_Unlink(pDevice->pGenericDeviceHandle, NULL);
    BKNI_Free(pDevice->pGenericDeviceHandle);
    pDevice->pGenericDeviceHandle = NULL;

    if(pDevice->capabilities.channelCapabilities)
        BKNI_Free(pDevice->capabilities.channelCapabilities);
    pDevice->capabilities.channelCapabilities = NULL;

    BDBG_OBJECT_DESTROY(pDevice, NEXUS_7563);
    BKNI_Free(pDevice);
    pDevice = NULL;

    return;
}


static NEXUS_FrontendLockStatus  NEXUS_Frontend_P_GetLockStatus(unsigned lockStatus)
{
    switch ( lockStatus )
    {
    /*BODS_LockStatus_eUnlocked == BTHD_LockStatus_eUnlocked*/
    case BODS_LockStatus_eUnlocked:
        return NEXUS_FrontendLockStatus_eUnlocked;
    case BODS_LockStatus_eLocked:
        return NEXUS_FrontendLockStatus_eLocked;
    case BODS_LockStatus_eNoSignal:
        return NEXUS_FrontendLockStatus_eNoSignal;
    default:
        BDBG_WRN(("Unrecognized lock status (%d) ", lockStatus));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return NEXUS_FrontendLockStatus_eUnknown;
    }
}
static NEXUS_Error NEXUS_Frontend_P_7563_GetFastStatus(void *handle, NEXUS_FrontendFastStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7563 *pDevice = (NEXUS_7563 *)handle;
    unsigned lock;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);

    switch ( pDevice->lastChannel )
    {
    case NEXUS_7563_DVBT2_CHN:

        rc = BODS_GetLockStatus(pDevice->ods_chn[pDevice->lastChannel],  &lock);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        break;
    case NEXUS_7563_DVBT_CHN:
        rc = BODS_GetLockStatus(pDevice->ods_chn[pDevice->lastChannel],  &lock);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        break;
    default:
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }
    pStatus->lockStatus = NEXUS_Frontend_P_GetLockStatus(lock);
    BKNI_EnterCriticalSection();
    pStatus->acquireInProgress = pDevice->acquireInProgress;
    BKNI_LeaveCriticalSection();
    return BERR_SUCCESS;
done:
    return rc;
}

static void NEXUS_Frontend_P_7563_ResetStatus(void *handle)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7563 *pDevice = (NEXUS_7563 *)handle;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);

    switch ( pDevice->lastChannel )
    {
    case NEXUS_7563_DVBT_CHN:
    case NEXUS_7563_DVBT2_CHN:
        if(pDevice->isPoweredOn[pDevice->lastChannel]) {
            rc = BODS_ResetStatus(pDevice->ods_chn[pDevice->lastChannel]);
            if(rc){rc = BERR_TRACE(rc); goto done;}
        }
        else{
            BDBG_MSG(("The core is Powered Off"));
        }
        break;
    default:
        BDBG_ERR((" Unsupported channel."));
        BERR_TRACE(BERR_NOT_SUPPORTED);
    }
done:
    return;
}

static void NEXUS_Frontend_P_7563_UninstallCallbacks(void *handle)
{
    NEXUS_7563 *pDevice = (NEXUS_7563 *)handle;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);

    if(pDevice->lockAppCallback[pDevice->lastChannel])NEXUS_IsrCallback_Set(pDevice->lockAppCallback[pDevice->lastChannel], NULL);
    if(pDevice->asyncStatusAppCallback[pDevice->lastChannel])NEXUS_IsrCallback_Set(pDevice->asyncStatusAppCallback[pDevice->lastChannel], NULL);
    return;
}

static void NEXUS_Frontend_P_7563_UnTune(void *handle)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7563 *pDevice = (NEXUS_7563 *)handle;
    BODS_PowerSaverSettings pwrSettings;

    BDBG_MSG(("Untune: pDevice = %p", pDevice));
    BDBG_MSG(("Tuner is not powered down for now to decrease channel change time."));

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);

    switch ( pDevice->lastChannel )
    {
    case NEXUS_7563_DVBT_CHN:
    case NEXUS_7563_DVBT2_CHN:
        if(pDevice->isPoweredOn[pDevice->lastChannel]) {
            rc = BODS_EnablePowerSaver(pDevice->ods_chn[pDevice->lastChannel], &pwrSettings);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->isPoweredOn[pDevice->lastChannel] = false;
        }
        break;
    default:
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED);
    }
done:
    return;
}

static NEXUS_Error NEXUS_Frontend_P_7563_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7563 *pDevice = (NEXUS_7563 *)handle;
    BTNR_PowerSaverSettings tnrPwrSettings;
    BODS_PowerSaverSettings odsPwrSettings;

    BSTD_UNUSED(enabled);
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);

    if(pSettings->mode < NEXUS_StandbyMode_ePassive){
        if(!pDevice->isTunerPoweredOn){
            tnrPwrSettings.enable = false;
            rc = BTNR_SetPowerSaver(pDevice->tnr[0], &tnrPwrSettings);
            if(rc){rc = BERR_TRACE(rc);}
            pDevice->isTunerPoweredOn = true;
        }
        switch ( pDevice->lastChannel )
        {
        case NEXUS_7563_DVBT_CHN:
        case NEXUS_7563_DVBT2_CHN:
            if(!pDevice->isPoweredOn[pDevice->lastChannel]){
                rc = BODS_DisablePowerSaver(pDevice->ods_chn[pDevice->lastChannel], &odsPwrSettings);
                if(rc){rc = BERR_TRACE(rc); goto done;}
                pDevice->isPoweredOn[pDevice->lastChannel] = true;
            }
            break;
        default:
            BDBG_ERR((" Unsupported channel."));
            rc = BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else {
        NEXUS_Frontend_P_7563_UninstallCallbacks(handle);
        switch ( pDevice->lastChannel )
        {
        case NEXUS_7563_DVBT_CHN:
        case NEXUS_7563_DVBT2_CHN:
             if(pDevice->isPoweredOn[pDevice->lastChannel]){
                rc = BODS_EnablePowerSaver(pDevice->ods_chn[pDevice->lastChannel], &odsPwrSettings);
                if(rc){rc = BERR_TRACE(rc); goto done;}
                pDevice->isPoweredOn[pDevice->lastChannel] = false;
             }
            break;
        default:
            BDBG_ERR((" Unsupported channel."));
            rc = BERR_TRACE(BERR_NOT_SUPPORTED);
        }

        if(pDevice->isTunerPoweredOn){
            tnrPwrSettings.enable = true;
            rc = BTNR_SetPowerSaver(pDevice->tnr[0], &tnrPwrSettings);
            if(rc){rc = BERR_TRACE(rc);}
            pDevice->isTunerPoweredOn = false;
        }
    }

done:
    return rc;
}

static NEXUS_Error NEXUS_FrontendDevice_P_7563_Standby(void *handle, const NEXUS_StandbySettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7563 *pDevice;
    NEXUS_FrontendDevice3461Settings deviceSettings;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7563 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);

    if((pDevice->previousStandbyMode != NEXUS_StandbyMode_eDeepSleep) && (pSettings->mode == NEXUS_StandbyMode_eDeepSleep)){
         NEXUS_Frontend_P_UnInit7563(pDevice);
         NEXUS_Frontend_P_UnInit_7563_Hab(pDevice);
    }
    else if((pDevice->previousStandbyMode == NEXUS_StandbyMode_eDeepSleep) && (pSettings->mode != NEXUS_StandbyMode_eDeepSleep)){
        rc = NEXUS_Frontend_P_Init_7563_Hab(pDevice, &pDevice->openSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = NEXUS_Frontend_P_Init7563(pDevice);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        NEXUS_FrontendDevice_GetDefault3461Settings(&deviceSettings);

        deviceSettings.updateGainCallback = pDevice->updateGainCallbackDesc;
        deviceSettings.agcValue = pDevice->agcValue;
        deviceSettings.rfDaisyChain = pDevice->rfDaisyChain;
        deviceSettings.enableRfLoopThrough = pDevice->enableRfLoopThrough;
        deviceSettings.rfInput = pDevice->rfInput;
        deviceSettings.terrestrial = pDevice->terrestrial;

        rc = NEXUS_FrontendDevice_Set3461Settings(pDevice->pGenericDeviceHandle, &deviceSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    pDevice->previousStandbyMode = pSettings->mode;

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7563_ReadSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length, size_t *pNumRead)
{
    size_t i;
    NEXUS_Error rc = NEXUS_SUCCESS;
    int16_t return_length;
    NEXUS_7563 *pDevice = (NEXUS_7563 *)handle;
    int16_t d_i[TOTAL_SOFTDECISIONS], d_q[TOTAL_SOFTDECISIONS];

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);

    BKNI_Memset(pDecisions, 0, (sizeof(NEXUS_FrontendSoftDecision) * length));

    switch ( pDevice->lastChannel )
    {
    case NEXUS_7563_DVBT_CHN:
    case NEXUS_7563_DVBT2_CHN:
            rc = BODS_GetSoftDecision(pDevice->ods_chn[pDevice->lastChannel], (int16_t)TOTAL_SOFTDECISIONS, d_i, d_q, &return_length);
            if(rc){rc = BERR_TRACE(rc); goto done;}
        break;
    default:
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }

    for (i=0; (int)i<return_length && i<length; i++)
    {
        pDecisions[i].i = d_i[i];
        pDecisions[i].q = d_q[i];
    }
    *pNumRead = i;

done:
    return rc;
}

/***************************************************************************
Summary:
  Probe to see if a BCM7563 device exists with the specified settings

Description:
  Probe to see if a BCM7563 device exists with the specified settings

See Also:
    NEXUS_Frontend_Open7563
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Probe3461(const NEXUS_FrontendDevice3461OpenSettings *pSettings, NEXUS_3461ProbeResults *pResults)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BCHP_Info info;
    uint16_t chipVer=0xffff;

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pResults);

    BCHP_GetInfo(g_pCoreHandles->chp, &info);
    pResults->chip.id = info.productId;
    pResults->chip.familyId = info.familyId;
    if ((pResults->chip.familyId != 0x7563) && !((pResults->chip.familyId >= 0x75630) && (pResults->chip.familyId <= 0x75639)))
    {
        BDBG_ERR(("pResults->chip.familyId = 0x%x", pResults->chip.familyId));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }
    chipVer = NEXUS_Frontend_P_Get7563Rev(pSettings);

done:
    pResults->chip.version.major = (chipVer >> 8) + 1;
    pResults->chip.version.minor = chipVer & 0xff;

    return rc;
}

static void NEXUS_Frontend_P_7563_GetType(void *handle, NEXUS_FrontendType *type)
{
    NEXUS_7563 *pDevice = (NEXUS_7563 *)handle;
    BFEC_SystemVersionInfo  versionInfo;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);

    BHAB_GetVersionInfo(pDevice->hab, &versionInfo);

    type->chip.familyId = (uint32_t)pDevice->familyId;
    type->chip.id = (uint32_t)pDevice->chipId;
    type->chip.version.major = (pDevice->revId >> 8) + 1;
    type->chip.version.minor = pDevice->revId & 0xff;
    type->chip.bondoutOptions[0] = versionInfo.bondoutOptions[0];
    type->chip.bondoutOptions[1] = versionInfo.bondoutOptions[1];
    type->chip.version.buildType = 0;
    type->chip.version.buildId = 0;

    type->firmwareVersion.major = versionInfo.firmware.majorVersion;
    type->firmwareVersion.minor = versionInfo.firmware.minorVersion;
    type->firmwareVersion.buildType = versionInfo.firmware.buildType;
    type->firmwareVersion.buildId = versionInfo.firmware.buildId;

    if((type->chip.familyId != versionInfo.familyId) || (type->chip.id != versionInfo.chipId)){
        BDBG_ERR(("Type mismatch while retreiving chip id and family id."));
        BERR_TRACE(BERR_UNKNOWN); goto done;
    }
    if(pDevice->revId != versionInfo.chipVersion){
        BDBG_ERR(("Type mismatch while retreiving chip version."));
        BERR_TRACE(BERR_UNKNOWN); goto done;
    }
done:
    return;
}

NEXUS_Error NEXUS_Tuner_P_7563_GetNSetGain(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_GainParameters params;
    NEXUS_InternalGainSettings settings;
    NEXUS_ExternalGainSettings externalGainSettings;
    NEXUS_7563 *pDevice = (NEXUS_7563 *)handle;

    BKNI_Memset(&externalGainSettings, 0, sizeof(externalGainSettings));

    if(pDevice->pGenericDeviceHandle->parent){

        params.rfInput = pDevice->pGenericDeviceHandle->linkSettings.rfInput;
        params.accumulateTillRootDevice = true;
        params.frequency = pDevice->last_ofdm[0].frequency;

        BKNI_Memset(&settings, 0, sizeof(settings));

        rc = NEXUS_FrontendDevice_P_GetInternalGain(pDevice->pGenericDeviceHandle->parent, &params, &settings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        externalGainSettings.totalGain = settings.totalVariableGain;

        if(settings.isExternalFixedGainBypassed){
            externalGainSettings.totalGain -= pDevice->pGenericDeviceHandle->bypassableFixedGain;
        }
    }

    externalGainSettings.bypassableGain = pDevice->pGenericDeviceHandle->bypassableFixedGain;
    externalGainSettings.totalGain += pDevice->pGenericDeviceHandle->totalFixedBoardGain;

    rc = NEXUS_FrontendDevice_P_SetExternalGain(pDevice->pGenericDeviceHandle, &externalGainSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}

static NEXUS_FrontendOfdmModulation NEXUS_Frontend_P_ModulationToDvbt(NEXUS_FrontendOfdmModulation modulation)
{
    switch ( modulation )
    {
    case NEXUS_FrontendOfdmModulation_eQpsk:
        return BODS_DvbtModulation_eQpsk;
    case NEXUS_FrontendOfdmModulation_eQam16:
        return BODS_DvbtModulation_e16Qam;
    case NEXUS_FrontendOfdmModulation_eQam64:
        return BODS_DvbtModulation_e64Qam;
    default:
        BDBG_WRN(("Unrecognized modulation (%d)", modulation));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return BODS_DvbtModulation_eQpsk;
    }
}

static NEXUS_FrontendOfdmModulation NEXUS_Frontend_P_CodeRateToDvbt(NEXUS_FrontendOfdmCodeRate codeRate)
{
    switch ( codeRate )
    {
    case NEXUS_FrontendOfdmCodeRate_e1_2:
        return BODS_DvbtCodeRate_e1_2;
    case NEXUS_FrontendOfdmCodeRate_e2_3:
        return BODS_DvbtCodeRate_e2_3;
    case NEXUS_FrontendOfdmCodeRate_e3_4:
        return BODS_DvbtCodeRate_e3_4;
    case NEXUS_FrontendOfdmCodeRate_e5_6:
        return BODS_DvbtCodeRate_e5_6;
    case NEXUS_FrontendOfdmCodeRate_e7_8:
        return BODS_DvbtCodeRate_e7_8;
    default:
        BDBG_WRN(("Unrecognized code rate (%d)", codeRate));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return BODS_DvbtCodeRate_e1_2;
    }
}

static NEXUS_FrontendOfdmModulation NEXUS_Frontend_P_HierarchyToDvbt(NEXUS_FrontendOfdmHierarchy hierarchy)
{
    switch ( hierarchy )
    {
    case NEXUS_FrontendOfdmHierarchy_e0:
        return BODS_DvbtHierarchy_e0;
    case NEXUS_FrontendOfdmHierarchy_e1:
        return BODS_DvbtHierarchy_e1;
    case NEXUS_FrontendOfdmHierarchy_e2:
        return BODS_DvbtHierarchy_e2;
    case NEXUS_FrontendOfdmHierarchy_e4:
        return BODS_DvbtHierarchy_e4;
    default:
        BDBG_WRN(("Unrecognized hierarchy (%d)", hierarchy));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return BODS_DvbtHierarchy_e0;
    }
}


static NEXUS_Error NEXUS_Frontend_P_7563_TuneOfdm(void *handle, const NEXUS_FrontendOfdmSettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7563 *pDevice = (NEXUS_7563 *)handle;
    BODS_AcquireParams odsParam;
    unsigned chn_num = 0;
    BTNR_Settings tnrSettings;
    unsigned temp_frequency;
    NEXUS_FrontendOfdmMode temp_mode;
    BODS_PowerSaverSettings odsPwrSettings;
    BTNR_PowerSaverSettings tnrPwrSettings;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);

    switch ( pSettings->mode )
    {
    case NEXUS_FrontendOfdmMode_eDvbt:
        chn_num = NEXUS_7563_DVBT_CHN;
        break;
    case NEXUS_FrontendOfdmMode_eDvbt2:
        chn_num = NEXUS_7563_DVBT2_CHN;
        break;
    case NEXUS_FrontendOfdmMode_eIsdbt:
    default:
        /* It is IMPORTANT to check this condition here. Because it will not be checked later. */
        BDBG_ERR(("Wrong Ofdm mode selected."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }

    /* This is done as DVBT and DVBT2 are in a single core and needs to be powered down*/
    if(pDevice->last_ofdm[0].mode != pSettings->mode){
        NEXUS_Frontend_P_7563_UnTune(handle);
    }

    rc = NEXUS_Tuner_P_7563_GetNSetGain(pDevice);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = BODS_GetDefaultAcquireParams(pDevice->ods_chn[chn_num], &odsParam);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    NEXUS_IsrCallback_Set(pDevice->lockAppCallback[chn_num], &(pSettings->lockCallback));
    NEXUS_IsrCallback_Set(pDevice->asyncStatusAppCallback[chn_num], &(pSettings->asyncStatusReadyCallback));

    if(!pDevice->isPoweredOn[chn_num] || !pDevice->isTunerPoweredOn)
        goto full_acquire;

    if((pSettings->acquisitionMode == NEXUS_FrontendOfdmAcquisitionMode_eScan) && (pDevice->lastAcquisitionMode[chn_num] == NEXUS_FrontendOfdmAcquisitionMode_eScan)){
        temp_frequency = pDevice->last_ofdm[0].frequency;
        pDevice->last_ofdm[0].frequency = pSettings->frequency ;
        temp_mode = pDevice->last_ofdm[0].mode;
        pDevice->last_ofdm[0].mode = pSettings->mode;

        if(!BKNI_Memcmp(pSettings, &pDevice->last_ofdm[0], sizeof(NEXUS_FrontendOfdmSettings))) {
            if (pDevice->tnr[0])
            {
                pDevice->acquireInProgress = true;
                pDevice->last_ofdm[0] = *pSettings;
                pDevice->lastChannel = chn_num;
                rc = BTNR_SetTunerRfFreq(pDevice->tnr[0], pSettings->frequency, BTNR_TunerMode_eDigital);
                if(rc){rc = BERR_TRACE(rc); goto retrack;}

                return rc;
            }
        }
    }

full_acquire:
    pDevice->acquireInProgress = true;
    pDevice->lastChannel = chn_num;
    pDevice->lastAcquisitionMode[chn_num] = pSettings->acquisitionMode;

    switch ( pSettings->acquisitionMode )
    {
    case NEXUS_FrontendOfdmAcquisitionMode_eScan:
        odsParam.tuneAcquire = BODS_TuneAcquire_eAcquireAfterTune;
        /* No break as we set the modes to manual for scan.*/
    case NEXUS_FrontendOfdmAcquisitionMode_eAuto:
        /* Due to get default parameters, the odsParam.tuneAcquire and thdParam.bTuneAcquire are set to false. */
        odsParam.acquisitionMode = BODS_AcquisitionMode_eAuto;
        break;
    case NEXUS_FrontendOfdmAcquisitionMode_eManual:
        /* Due to get default parameters, the odsParam.tuneAcquire and thdParam.bTuneAcquire are set to false. */
        odsParam.acquisitionMode = BODS_AcquisitionMode_eManual;
        break;
    default:
        BDBG_ERR((" Unsupported Acquisition mode."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }

    odsParam.spectrumMode = (BODS_SpectrumMode)pSettings->spectrumMode;
    odsParam.invertSpectrum = (BODS_InvertSpectrum)pSettings->spectralInversion;
    if(pSettings->spectrum){
        odsParam.spectrumMode = (BODS_SpectrumMode)BODS_SpectrumMode_eManual;
        switch ( pSettings->spectrumMode )
        {
        case NEXUS_FrontendOfdmSpectrum_eNonInverted:
            odsParam.invertSpectrum = BODS_InvertSpectrum_eNormal;
            break;
        case NEXUS_FrontendOfdmSpectrum_eInverted:
            odsParam.invertSpectrum = BODS_InvertSpectrum_eInverted;
            break;
        default:
            BDBG_ERR((" Unsupported spectrum inversion mode."));
            rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
        }
    }

    switch ( pSettings->mode )
    {
    case NEXUS_FrontendOfdmMode_eDvbt2:
        switch ( pSettings->bandwidth )
        {
        case 1700000:
            odsParam.acquireParams.dvbt2.bandwidth = BODS_Dvbt2Bandwidth_e1p7MHz;
            break;
        case 5000000:
            odsParam.acquireParams.dvbt2.bandwidth = BODS_Dvbt2Bandwidth_e5MHz;
            break;
        case 6000000:
            odsParam.acquireParams.dvbt2.bandwidth = BODS_Dvbt2Bandwidth_e6MHz;
            break;
        case 7000000:
            odsParam.acquireParams.dvbt2.bandwidth = BODS_Dvbt2Bandwidth_e7MHz;
            break;
        case 8000000:
            odsParam.acquireParams.dvbt2.bandwidth = BODS_Dvbt2Bandwidth_e8MHz;
            break;
        default:
            BDBG_ERR((" Unsupported bandwidth."));
            rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
        }
        odsParam.acquireParams.dvbt2.plpMode = pSettings->dvbt2Settings.plpMode;
        if(!pSettings->dvbt2Settings.plpMode)
            odsParam.acquireParams.dvbt2.plpId = pSettings->dvbt2Settings.plpId;
        switch ( pSettings->dvbt2Settings.profile )
        {
        case NEXUS_FrontendDvbt2Profile_eBase:
            odsParam.acquireParams.dvbt2.profile= BODS_Dvbt2Profile_eBase;
            break;
        case NEXUS_FrontendDvbt2Profile_eLite:
            odsParam.acquireParams.dvbt2.profile = BODS_Dvbt2Profile_eLite;
            break;
        default:
            BDBG_ERR((" Unsupported profile."));
            rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
        }
        break;
    case NEXUS_FrontendOfdmMode_eDvbt:
        switch ( pSettings->bandwidth )
        {
        case 1700000:
            odsParam.acquireParams.dvbt.bandwidth = BODS_DvbtBandwidth_e1p7MHz;
            break;
        case 5000000:
            odsParam.acquireParams.dvbt.bandwidth = BODS_DvbtBandwidth_e5MHz;
            break;
        case 6000000:
            odsParam.acquireParams.dvbt.bandwidth = BODS_DvbtBandwidth_e6MHz;
            break;
        case 7000000:
            odsParam.acquireParams.dvbt.bandwidth = BODS_DvbtBandwidth_e7MHz;
            break;
        case 8000000:
            odsParam.acquireParams.dvbt.bandwidth = BODS_DvbtBandwidth_e8MHz;
            break;
        default:
            BDBG_ERR((" Unsupported bandwidth."));
            rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
        }
        odsParam.acquireParams.dvbt.carrierRange = pSettings->pullInRange;
        odsParam.acquireParams.dvbt.cciMode = (BODS_DvbtCciMode)pSettings->cciMode;
        /* Nexus to PI, there is reversal in logic. */
        if(pSettings->manualTpsSettings) {
            odsParam.acquireParams.dvbt.tpsMode = BODS_DvbtTpsMode_eManual;
            odsParam.acquireParams.dvbt.codeRateHighPriority = NEXUS_Frontend_P_CodeRateToDvbt(pSettings->tpsSettings.highPriorityCodeRate);
            odsParam.acquireParams.dvbt.codeRateLowPriority = NEXUS_Frontend_P_CodeRateToDvbt(pSettings->tpsSettings.lowPriorityCodeRate);
            odsParam.acquireParams.dvbt.hierarchy = NEXUS_Frontend_P_HierarchyToDvbt(pSettings->tpsSettings.hierarchy);
            odsParam.acquireParams.dvbt.modulation = NEXUS_Frontend_P_ModulationToDvbt(pSettings->tpsSettings.modulation);
        }
        else
            odsParam.acquireParams.dvbt.tpsMode = BODS_DvbtTpsMode_eAuto;
        if(pSettings->manualModeSettings){
            odsParam.acquireParams.dvbt.transGuardMode = BODS_DvbtOfdmMode_eManual;
            odsParam.acquireParams.dvbt.guardInterval = pSettings->modeSettings.guardInterval;
            if((pSettings->modeSettings.mode>NEXUS_FrontendOfdmTransmissionMode_e1k) && (pSettings->modeSettings.mode<NEXUS_FrontendOfdmTransmissionMode_e16k))
                odsParam.acquireParams.dvbt.transmissionMode = pSettings->modeSettings.mode;
            else {
                BDBG_ERR((" Unsupported DVBT Transmission mode."));
                rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
            }

        }
        else
            odsParam.acquireParams.dvbt.transGuardMode = BODS_DvbtOfdmMode_eAuto;
        if(pSettings->priority == NEXUS_FrontendOfdmPriority_eHigh)
            odsParam.acquireParams.dvbt.decodeMode = BODS_DvbtDecodeMode_eHighPriority;
        else
            odsParam.acquireParams.dvbt.decodeMode = BODS_DvbtDecodeMode_eLowPriority;
        break;
    default:
        BDBG_ERR((" Unsupported mode."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
        break;
    }

    if (pDevice->tnr[0])
    {
        if(!pDevice->isTunerPoweredOn){
            tnrPwrSettings.enable = false;
            rc = BTNR_SetPowerSaver(pDevice->tnr[0], &tnrPwrSettings);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->isTunerPoweredOn = true;
        }
        if(!pDevice->isPoweredOn[chn_num]){
            rc = BODS_DisablePowerSaver(pDevice->ods_chn[chn_num], &odsPwrSettings);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->isPoweredOn[chn_num] = true;

        }

        rc = BODS_SetAcquireParams( pDevice->ods_chn[chn_num], &odsParam );
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = BTNR_GetSettings(pDevice->tnr[0], &tnrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        if(pSettings->mode == NEXUS_FrontendOfdmMode_eDvbt2){
            tnrSettings.std = BTNR_Standard_eDvbt2;
        }
        else if(pSettings->mode == NEXUS_FrontendOfdmMode_eDvbt){
            tnrSettings.std = BTNR_Standard_eDvbt;
        }
        tnrSettings.bandwidth = pSettings->bandwidth;

        rc = BTNR_SetSettings(pDevice->tnr[0], &tnrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = BTNR_SetTunerRfFreq(pDevice->tnr[0], pSettings->frequency, BTNR_TunerMode_eDigital);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    if ( pSettings->acquisitionMode != NEXUS_FrontendOfdmAcquisitionMode_eScan) {
        switch ( pSettings->mode )
        {
        case NEXUS_FrontendOfdmMode_eDvbt2:
        case NEXUS_FrontendOfdmMode_eDvbt:
            rc = BODS_Acquire(pDevice->ods_chn[chn_num], &odsParam);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            break;
        default:
            break;
        }
    }

    pDevice->last_ofdm[0] = *pSettings;
    return BERR_SUCCESS;
retrack:
    pDevice->last_ofdm[0].frequency = temp_frequency;
    pDevice->last_ofdm[0].mode = temp_mode;
done:
    NEXUS_Frontend_P_7563_UnTune(handle);
    return rc;
}

void NEXUS_FrontendDevice_GetDefault3461Settings(NEXUS_FrontendDevice3461Settings *pSettings)
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->agcValue = 0x1f; /* Max gain*/
}

NEXUS_Error NEXUS_FrontendDevice_Get3461Settings(NEXUS_FrontendDeviceHandle handle, NEXUS_FrontendDevice3461Settings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7563 *pDevice = NULL;
    BDBG_ASSERT(handle != NULL);

    pDevice = handle->pDevice;
    if (!pDevice) { rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto done; }
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);

    pSettings->updateGainCallback = pDevice->updateGainCallbackDesc;
    pSettings->agcValue = pDevice->agcValue;
    pSettings->rfDaisyChain = pDevice->rfDaisyChain;
    pSettings->enableRfLoopThrough = pDevice->enableRfLoopThrough;
    pSettings->rfInput = pDevice->rfInput;
    pSettings->terrestrial = pDevice->terrestrial;
done:
    return rc;
}

NEXUS_Error NEXUS_FrontendDevice_Set3461Settings(NEXUS_FrontendDeviceHandle handle, const NEXUS_FrontendDevice3461Settings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7563 *pDevice = NULL;
    BHAB_ConfigSettings habConfigSettings;

    pDevice = handle->pDevice;
    if (!pDevice) { rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto done; }
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);

    rc = BHAB_GetConfigSettings(pDevice->hab, &habConfigSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    habConfigSettings.daisyChain = pSettings->rfDaisyChain;
    habConfigSettings.enableLoopThrough = pSettings->enableRfLoopThrough;

    habConfigSettings.rfInputMode = pSettings->rfInput;
    habConfigSettings.tnrApplication = pSettings->terrestrial;

    rc = BHAB_SetConfigSettings(pDevice->hab, &habConfigSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pDevice->rfDaisyChain = pSettings->rfDaisyChain;
    pDevice->enableRfLoopThrough = pSettings->enableRfLoopThrough;
    pDevice->rfInput = pSettings->rfInput;
    pDevice->terrestrial = pSettings->terrestrial;
done:
    return rc;
}

static BODS_SelectiveAsyncStatusType NEXUS_Frontend_P_7563_t2StatusTypeToOds(NEXUS_FrontendDvbt2StatusType type)
{
    switch (type)
    {
    case NEXUS_FrontendDvbt2StatusType_eFecStatisticsL1Pre:
        return BODS_SelectiveAsyncStatusType_eDvbt2FecStatisticsL1Pre;
    case NEXUS_FrontendDvbt2StatusType_eFecStatisticsL1Post:
        return BODS_SelectiveAsyncStatusType_eDvbt2FecStatisticsL1Post;
    case NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpA:
        return BODS_SelectiveAsyncStatusType_eDvbt2FecStatisticsPlpA;
    case NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpB:
        return BODS_SelectiveAsyncStatusType_eDvbt2FecStatisticsPlpB;
    case NEXUS_FrontendDvbt2StatusType_eL1Pre:
        return BODS_SelectiveAsyncStatusType_eDvbt2L1Pre;
    case NEXUS_FrontendDvbt2StatusType_eL1PostConfigurable:
        return BODS_SelectiveAsyncStatusType_eDvbt2L1PostConfigurable;
    case NEXUS_FrontendDvbt2StatusType_eL1PostDynamic:
        return BODS_SelectiveAsyncStatusType_eDvbt2L1PostDynamic;
    case NEXUS_FrontendDvbt2StatusType_eL1Plp:
        return BODS_SelectiveAsyncStatusType_eDvbt2L1Plp;
    case NEXUS_FrontendDvbt2StatusType_eBasic:
        return BODS_SelectiveAsyncStatusType_eDvbt2Short;
    default:
        BDBG_WRN((" Unsupported status type."));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return BODS_SelectiveAsyncStatusType_eDvbt2Short;
    }
}

static NEXUS_Error NEXUS_Frontend_P_7563_RequestDvbt2AsyncStatus(void *handle, NEXUS_FrontendDvbt2StatusType type)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7563 *pDevice;
    BODS_SelectiveAsyncStatusType statusType;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7563 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);

    if(pDevice->lastChannel != NEXUS_7563_DVBT2_CHN){
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto done;
    }

    statusType = NEXUS_Frontend_P_7563_t2StatusTypeToOds(type);

    rc = BODS_RequestSelectiveAsyncStatus(pDevice->ods_chn[NEXUS_7563_DVBT2_CHN], statusType);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7563_GetDvbt2AsyncStatusReady(void *handle, NEXUS_FrontendDvbt2StatusReady *pStatusReady)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    BODS_SelectiveAsyncStatusReadyType readyType;
    NEXUS_7563 *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7563 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);

    BKNI_Memset(pStatusReady, 0, sizeof(NEXUS_FrontendDvbt2StatusReady));
    BKNI_Memset(&readyType, 0, sizeof(readyType));

    if(pDevice->lastChannel != NEXUS_7563_DVBT2_CHN){
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto done;
    }

    rc = BODS_GetSelectiveAsyncStatusReadyType(pDevice->ods_chn[NEXUS_7563_DVBT2_CHN], &readyType);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pStatusReady->type[NEXUS_FrontendDvbt2StatusType_eFecStatisticsL1Pre] = readyType.dvbt2FecStatisticsL1Pre;
    pStatusReady->type[NEXUS_FrontendDvbt2StatusType_eFecStatisticsL1Post] = readyType.dvbt2FecStatisticsL1Post;
    pStatusReady->type[NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpA] = readyType.dvbt2FecStatisticsPlpA;
    pStatusReady->type[NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpB] = readyType.dvbt2FecStatisticsPlpB;
    pStatusReady->type[NEXUS_FrontendDvbt2StatusType_eL1Pre] = readyType.dvbt2L1Pre;
    pStatusReady->type[NEXUS_FrontendDvbt2StatusType_eL1PostConfigurable] = readyType.dvbt2L1PostConfigurable;
    pStatusReady->type[NEXUS_FrontendDvbt2StatusType_eL1PostDynamic] = readyType.dvbt2L1PostDynamic;
    pStatusReady->type[NEXUS_FrontendDvbt2StatusType_eL1Plp] = readyType.dvbt2L1Plp;
    pStatusReady->type[NEXUS_FrontendDvbt2StatusType_eBasic] = readyType.dvbt2ShortStatus;

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7563_GetDvbt2AsyncStatus(void *handle, NEXUS_FrontendDvbt2StatusType type, NEXUS_FrontendDvbt2Status *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    unsigned i=0;
    BODS_SelectiveAsyncStatusType statusType;
    NEXUS_7563 *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_7563 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);

    BKNI_Memset(pStatus, 0, sizeof(NEXUS_FrontendDvbt2Status));
    BKNI_Memset(&pDevice->odsStatus, 0, sizeof(pDevice->odsStatus));

    if(pDevice->lastChannel != NEXUS_7563_DVBT2_CHN){
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto done;
    }

    statusType = NEXUS_Frontend_P_7563_t2StatusTypeToOds(type);

    rc = BODS_GetSelectiveAsyncStatus(pDevice->ods_chn[NEXUS_7563_DVBT2_CHN], statusType, &pDevice->odsStatus);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if(statusType != pDevice->odsStatus.type){
        BDBG_ERR(("Requested nexus status type %d does not match the returned pi status type %d.",type, pDevice->odsStatus.type));
        rc = BERR_TRACE(NEXUS_UNKNOWN); goto done;
    }

    switch ( pDevice->odsStatus.type )
    {
    case BODS_SelectiveAsyncStatusType_eDvbt2FecStatisticsL1Pre:
    case BODS_SelectiveAsyncStatusType_eDvbt2FecStatisticsL1Post:
    case BODS_SelectiveAsyncStatusType_eDvbt2FecStatisticsPlpA:
    case BODS_SelectiveAsyncStatusType_eDvbt2FecStatisticsPlpB:
        pStatus->status.fecStatistics.lock = pDevice->odsStatus.status.dvbt2FecStatistics.lock;
        pStatus->status.fecStatistics.snrData = pDevice->odsStatus.status.dvbt2FecStatistics.snrData/256;
        pStatus->status.fecStatistics.ldpcAvgIter = pDevice->odsStatus.status.dvbt2FecStatistics.ldpcAvgIter;
        pStatus->status.fecStatistics.ldpcTotIter = pDevice->odsStatus.status.dvbt2FecStatistics.ldpcTotIter;
        pStatus->status.fecStatistics.ldpcTotFrm = pDevice->odsStatus.status.dvbt2FecStatistics.ldpcTotFrm;
        pStatus->status.fecStatistics.ldpcUncFrm = pDevice->odsStatus.status.dvbt2FecStatistics.ldpcUncFrm;
        pStatus->status.fecStatistics.ldpcBER = pDevice->odsStatus.status.dvbt2FecStatistics.ldpcBER;
        pStatus->status.fecStatistics.bchCorBit = pDevice->odsStatus.status.dvbt2FecStatistics.bchCorBit;
        pStatus->status.fecStatistics.bchTotBlk = pDevice->odsStatus.status.dvbt2FecStatistics.bchTotBlk;
        pStatus->status.fecStatistics.bchClnBlk = pDevice->odsStatus.status.dvbt2FecStatistics.bchClnBlk;
        pStatus->status.fecStatistics.bchCorBlk = pDevice->odsStatus.status.dvbt2FecStatistics.bchCorBlk;
        pStatus->status.fecStatistics.bchUncBlk = pDevice->odsStatus.status.dvbt2FecStatistics.bchUncBlk;
        break;
    case BODS_SelectiveAsyncStatusType_eDvbt2L1Pre:
        pStatus->status.l1Pre.streamType = pDevice->odsStatus.status.dvbt2L1Pre.streamType;
        pStatus->status.l1Pre.bwtExt = pDevice->odsStatus.status.dvbt2L1Pre.bwtExt;
        pStatus->status.l1Pre.s1 = pDevice->odsStatus.status.dvbt2L1Pre.s1;
        pStatus->status.l1Pre.s2 = pDevice->odsStatus.status.dvbt2L1Pre.s2;
        pStatus->status.l1Pre.l1RepetitionFlag = pDevice->odsStatus.status.dvbt2L1Pre.l1RepetitionFlag;
        pStatus->status.l1Pre.guardInterval= pDevice->odsStatus.status.dvbt2L1Pre.guardInterval;
        pStatus->status.l1Pre.papr = pDevice->odsStatus.status.dvbt2L1Pre.papr;
        pStatus->status.l1Pre.l1Mod = pDevice->odsStatus.status.dvbt2L1Pre.l1Modulation;
        pStatus->status.l1Pre.l1CodeRate = pDevice->odsStatus.status.dvbt2L1Pre.l1CodeRate;
        pStatus->status.l1Pre.l1FecType = pDevice->odsStatus.status.dvbt2L1Pre.l1FecType;
        pStatus->status.l1Pre.pilotPattern = pDevice->odsStatus.status.dvbt2L1Pre.pilotPattern;
        pStatus->status.l1Pre.regenFlag = pDevice->odsStatus.status.dvbt2L1Pre.regenFlag;
        pStatus->status.l1Pre.l1PostExt = pDevice->odsStatus.status.dvbt2L1Pre.l1PostExt;
        pStatus->status.l1Pre.numRf = pDevice->odsStatus.status.dvbt2L1Pre.numRf;
        pStatus->status.l1Pre.currentRfIndex = pDevice->odsStatus.status.dvbt2L1Pre.currentRfIndex;
        pStatus->status.l1Pre.txIdAvailability = pDevice->odsStatus.status.dvbt2L1Pre.txIdAvailability;
        pStatus->status.l1Pre.numT2Frames = pDevice->odsStatus.status.dvbt2L1Pre.numT2Frames;
        pStatus->status.l1Pre.numDataSymbols = pDevice->odsStatus.status.dvbt2L1Pre.numDataSymbols;
        pStatus->status.l1Pre.cellId = pDevice->odsStatus.status.dvbt2L1Pre.cellId;
        pStatus->status.l1Pre.networkId = pDevice->odsStatus.status.dvbt2L1Pre.networkId;
        pStatus->status.l1Pre.t2SystemId = pDevice->odsStatus.status.dvbt2L1Pre.t2SystemId;
        pStatus->status.l1Pre.l1PostSize = pDevice->odsStatus.status.dvbt2L1Pre.l1PostSize;
        pStatus->status.l1Pre.l1PostInfoSize = pDevice->odsStatus.status.dvbt2L1Pre.l1PostInfoSize;
        break;
    case BODS_SelectiveAsyncStatusType_eDvbt2L1PostConfigurable:
        pStatus->status.l1PostConfigurable.subSlicesPerFrame = pDevice->odsStatus.status.dvbt2L1PostConfigurable.subSlicesPerFrame;
        pStatus->status.l1PostConfigurable.numPlp = pDevice->odsStatus.status.dvbt2L1PostConfigurable.numPlp;
        pStatus->status.l1PostConfigurable.numAux = pDevice->odsStatus.status.dvbt2L1PostConfigurable.numAux;
        pStatus->status.l1PostConfigurable.fefType = pDevice->odsStatus.status.dvbt2L1PostConfigurable.fefType;
        pStatus->status.l1PostConfigurable.rfIdx = pDevice->odsStatus.status.dvbt2L1PostConfigurable.rfIdx;
        pStatus->status.l1PostConfigurable.fefInterval = pDevice->odsStatus.status.dvbt2L1PostConfigurable.fefInterval;
        pStatus->status.l1PostConfigurable.frequency = pDevice->odsStatus.status.dvbt2L1PostConfigurable.frequency;
        pStatus->status.l1PostConfigurable.fefLength = pDevice->odsStatus.status.dvbt2L1PostConfigurable.fefLength;
        pStatus->status.l1PostConfigurable.auxStreamType = pDevice->odsStatus.status.dvbt2L1PostConfigurable.auxStreamType;
        pStatus->status.l1PostConfigurable.auxPrivateConf = pDevice->odsStatus.status.dvbt2L1PostConfigurable.auxPrivateConf;

        pStatus->status.l1PostConfigurable.plpA.plpId = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.plpId;
        pStatus->status.l1PostConfigurable.plpA.plpGroupId = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.plpGroupId;
        pStatus->status.l1PostConfigurable.plpA.plpType = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.plpType;
        pStatus->status.l1PostConfigurable.plpA.plpPayloadType = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.plpPayloadType;
        pStatus->status.l1PostConfigurable.plpA.ffFlag = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.ffFlag;
        pStatus->status.l1PostConfigurable.plpA.firstRfIdx = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.firstRfIdx;
        pStatus->status.l1PostConfigurable.plpA.firstFrameIdx = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.firstFrameIdx;
        pStatus->status.l1PostConfigurable.plpA.plpCodeRate = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.codeRate;
        pStatus->status.l1PostConfigurable.plpA.plpMod = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.modulation;
        pStatus->status.l1PostConfigurable.plpA.plpRotation = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.plpRotation;
        pStatus->status.l1PostConfigurable.plpA.plpFecType = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.plpFecType;
        pStatus->status.l1PostConfigurable.plpA.plpNumBlocksMax = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.plpNumBlocksMax;
        pStatus->status.l1PostConfigurable.plpA.frameInterval = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.frameInterval;
        pStatus->status.l1PostConfigurable.plpA.timeIlLength = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.timeIlLength;
        pStatus->status.l1PostConfigurable.plpA.timeIlType= pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.timeIlType;
        pStatus->status.l1PostConfigurable.plpA.inBandFlag = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpA.inBandFlag;

        pStatus->status.l1PostConfigurable.plpB.plpId = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.plpId;
        pStatus->status.l1PostConfigurable.plpB.plpGroupId = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.plpGroupId;
        pStatus->status.l1PostConfigurable.plpB.plpType = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.plpType;
        pStatus->status.l1PostConfigurable.plpB.plpPayloadType = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.plpPayloadType;
        pStatus->status.l1PostConfigurable.plpB.ffFlag = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.ffFlag;
        pStatus->status.l1PostConfigurable.plpB.firstRfIdx = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.firstRfIdx;
        pStatus->status.l1PostConfigurable.plpB.firstFrameIdx = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.firstFrameIdx;
        pStatus->status.l1PostConfigurable.plpB.plpCodeRate = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.codeRate;
        pStatus->status.l1PostConfigurable.plpB.plpMod = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.modulation;
        pStatus->status.l1PostConfigurable.plpB.plpRotation = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.plpRotation;
        pStatus->status.l1PostConfigurable.plpB.plpFecType = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.plpFecType;
        pStatus->status.l1PostConfigurable.plpB.plpNumBlocksMax = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.plpNumBlocksMax;
        pStatus->status.l1PostConfigurable.plpB.frameInterval = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.frameInterval;
        pStatus->status.l1PostConfigurable.plpB.timeIlLength = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.timeIlLength;
        pStatus->status.l1PostConfigurable.plpB.timeIlType= pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.timeIlType;
        pStatus->status.l1PostConfigurable.plpB.inBandFlag = pDevice->odsStatus.status.dvbt2L1PostConfigurable.plpB.inBandFlag;
        break;
    case BODS_SelectiveAsyncStatusType_eDvbt2L1PostDynamic:
        pStatus->status.l1PostDynamic.frameIdx = pDevice->odsStatus.status.dvbt2L1PostDynamic.frameIdx;
        pStatus->status.l1PostDynamic.l1ChanlgeCounter = pDevice->odsStatus.status.dvbt2L1PostDynamic.l1ChanlgeCounter;
        pStatus->status.l1PostDynamic.startRfIdx = pDevice->odsStatus.status.dvbt2L1PostDynamic.startRfIdx;
        pStatus->status.l1PostDynamic.subSliceInterval = pDevice->odsStatus.status.dvbt2L1PostDynamic.subSliceInterval;
        pStatus->status.l1PostDynamic.type2Start = pDevice->odsStatus.status.dvbt2L1PostDynamic.type2Start;
        pStatus->status.l1PostDynamic.auxPrivateDyn_31_0 = pDevice->odsStatus.status.dvbt2L1PostDynamic.auxPrivateDyn_31_0;
        pStatus->status.l1PostDynamic.auxPrivateDyn_47_32 = pDevice->odsStatus.status.dvbt2L1PostDynamic.auxPrivateDyn_47_32;
        pStatus->status.l1PostDynamic.plpA.plpId = pDevice->odsStatus.status.dvbt2L1PostDynamic.plpA.plpId;
        pStatus->status.l1PostDynamic.plpA.plpNumBlocks = pDevice->odsStatus.status.dvbt2L1PostDynamic.plpA.plpNumBlocks;
        pStatus->status.l1PostDynamic.plpA.plpStart = pDevice->odsStatus.status.dvbt2L1PostDynamic.plpA.plpNumBlocks;
        pStatus->status.l1PostDynamic.plpB.plpId = pDevice->odsStatus.status.dvbt2L1PostDynamic.plpB.plpId;
        pStatus->status.l1PostDynamic.plpB.plpNumBlocks = pDevice->odsStatus.status.dvbt2L1PostDynamic.plpB.plpNumBlocks;
        pStatus->status.l1PostDynamic.plpB.plpStart = pDevice->odsStatus.status.dvbt2L1PostDynamic.plpB.plpNumBlocks;
        break;
    case BODS_SelectiveAsyncStatusType_eDvbt2L1Plp:
        pStatus->status.l1Plp.numPlp = pDevice->odsStatus.status.dvbt2L1Plp.numPlp;
        for(i=0; i<pStatus->status.l1Plp.numPlp; i++) {
            pStatus->status.l1Plp.plp[i].plpId = pDevice->odsStatus.status.dvbt2L1Plp.plp[i].plpId;
            pStatus->status.l1Plp.plp[i].plpGroupId = pDevice->odsStatus.status.dvbt2L1Plp.plp[i].plpGroupId;
            pStatus->status.l1Plp.plp[i].plpPayloadType = pDevice->odsStatus.status.dvbt2L1Plp.plp[i].plpPayloadType;
            pStatus->status.l1Plp.plp[i].plpType = pDevice->odsStatus.status.dvbt2L1Plp.plp[i].plpType;
        }
        break;
    case BODS_SelectiveAsyncStatusType_eDvbt2Short:
        pStatus->status.basic.fecLock = pDevice->odsStatus.status.dvbt2Short.lock;
        pStatus->status.basic.spectrumInverted = pDevice->odsStatus.status.dvbt2Short.spectrumInverted;
        pStatus->status.basic.snr = pDevice->odsStatus.status.dvbt2Short.snrEstimate*100/256;
        pStatus->status.basic.gainOffset = pDevice->odsStatus.status.dvbt2Short.gainOffset*100/256;
        pStatus->status.basic.carrierOffset = pDevice->odsStatus.status.dvbt2Short.carrierFreqOffset;
        pStatus->status.basic.timingOffset = pDevice->odsStatus.status.dvbt2Short.timingOffset;
        pStatus->status.basic.signalStrength = pDevice->odsStatus.status.dvbt2Short.signalStrength/10;
        pStatus->status.basic.signalLevelPercent = pDevice->odsStatus.status.dvbt2Short.signalLevelPercent;
        pStatus->status.basic.signalQualityPercent = pDevice->odsStatus.status.dvbt2Short.signalQualityPercent;
        pStatus->status.basic.reacquireCount = pDevice->odsStatus.status.dvbt2Short.reacqCount;
        pStatus->status.basic.profile = pDevice->odsStatus.status.dvbt2Short.profile;
        break;
    default:
        BDBG_ERR((" Unsupported status type."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    pStatus->type = type;

    NEXUS_Frontend_P_PrintDvbt2PartialStatus(pStatus);
done:
    return rc;
}

static void NEXUS_Frontend_P_PrintDvbt2PartialStatus(NEXUS_FrontendDvbt2Status *pStatus)
{
    unsigned i=0;

    switch ( pStatus->type )
    {
    case NEXUS_FrontendDvbt2StatusType_eFecStatisticsL1Pre:
    case NEXUS_FrontendDvbt2StatusType_eFecStatisticsL1Post:
    case NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpA:
    case NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpB:
        BDBG_MSG(("pStatus->status.fecStatistics.lock = %d",pStatus->status.fecStatistics.lock));
        BDBG_MSG(("pStatus->status.fecStatistics.snrData = %d",pStatus->status.fecStatistics.snrData));
        BDBG_MSG(("pStatus->status.fecStatistics.ldpcAvgIter = %d",pStatus->status.fecStatistics.ldpcAvgIter));
        BDBG_MSG(("pStatus->status.fecStatistics.ldpcTotIter = %d",pStatus->status.fecStatistics.ldpcTotIter));
        BDBG_MSG(("pStatus->status.fecStatistics.ldpcTotFrm = %d",pStatus->status.fecStatistics.ldpcTotFrm));
        BDBG_MSG(("pStatus->status.fecStatistics.ldpcUncFrm = %d",pStatus->status.fecStatistics.ldpcUncFrm));
        BDBG_MSG(("pStatus->status.fecStatistics.ldpcBER = %d",pStatus->status.fecStatistics.ldpcBER));
        BDBG_MSG(("pStatus->status.fecStatistics.bchCorBit = %d",pStatus->status.fecStatistics.bchCorBit));
        BDBG_MSG(("pStatus->status.fecStatistics.bchTotBlk = %d",pStatus->status.fecStatistics.bchTotBlk));
        BDBG_MSG(("pStatus->status.fecStatistics.bchClnBlk = %d",pStatus->status.fecStatistics.bchTotBlk));
        BDBG_MSG(("pStatus->status.fecStatistics.bchCorBlk = %d",pStatus->status.fecStatistics.bchCorBlk));
        BDBG_MSG(("pStatus->status.fecStatistics.bchUncBlk = %d",pStatus->status.fecStatistics.bchUncBlk));
        break;
    case NEXUS_FrontendDvbt2StatusType_eL1Pre:
        BDBG_MSG(("pStatus->status.l1Pre.streamType = %d",pStatus->status.l1Pre.streamType));
        BDBG_MSG(("pStatus->status.l1Pre.bwtExt = %d",pStatus->status.l1Pre.bwtExt));
        BDBG_MSG(("pStatus->status.l1Pre.s1 = %d",pStatus->status.l1Pre.s1));
        BDBG_MSG(("pStatus->status.l1Pre.s2 = %d",pStatus->status.l1Pre.s2));
        BDBG_MSG(("pStatus->status.l1Pre.l1RepetitionFlag = %d",pStatus->status.l1Pre.l1RepetitionFlag));
        BDBG_MSG(("pStatus->status.l1Pre.guardInterval = %d",pStatus->status.l1Pre.guardInterval));
        BDBG_MSG(("pStatus->status.l1Pre.papr = %d",pStatus->status.l1Pre.papr));
        BDBG_MSG(("pStatus->status.l1Pre.l1Mod = %d",pStatus->status.l1Pre.l1Mod));
        BDBG_MSG(("pStatus->status.l1Pre.l1CodeRate = %d",pStatus->status.l1Pre.l1CodeRate));
        BDBG_MSG(("pStatus->status.l1Pre.l1FecType = %d",pStatus->status.l1Pre.l1FecType));
        BDBG_MSG(("pStatus->status.l1Pre.pilotPattern = %d",pStatus->status.l1Pre.pilotPattern));
        BDBG_MSG(("pStatus->status.l1Pre.regenFlag = %d",pStatus->status.l1Pre.regenFlag));
        BDBG_MSG(("pStatus->status.l1Pre.l1PostExt = %d",pStatus->status.l1Pre.l1PostExt));
        BDBG_MSG(("pStatus->status.l1Pre.numRf = %d",pStatus->status.l1Pre.numRf));
        BDBG_MSG(("pStatus->status.l1Pre.currentRfIndex = %d",pStatus->status.l1Pre.currentRfIndex));
        BDBG_MSG(("pStatus->status.l1Pre.txIdAvailability = %d",pStatus->status.l1Pre.txIdAvailability));
        BDBG_MSG(("pStatus->status.l1Pre.numT2Frames = %d",pStatus->status.l1Pre.numT2Frames));
        BDBG_MSG(("pStatus->status.l1Pre.numDataSymbols = %d",pStatus->status.l1Pre.numDataSymbols));
        BDBG_MSG(("pStatus->status.l1Pre.cellId = %d",pStatus->status.l1Pre.cellId));
        BDBG_MSG(("pStatus->status.l1Pre.networkId = %d",pStatus->status.l1Pre.networkId));
        BDBG_MSG(("pStatus->status.l1Pre.t2SystemId = %d",pStatus->status.l1Pre.t2SystemId));
        BDBG_MSG(("pStatus->status.l1Pre.l1PostSize = %d",pStatus->status.l1Pre.l1PostSize));
        BDBG_MSG(("pStatus->status.l1Pre.l1PostInfoSize = %d",pStatus->status.l1Pre.l1PostInfoSize));
        break;
    case NEXUS_FrontendDvbt2StatusType_eL1PostConfigurable:
        BDBG_MSG(("pStatus->status.l1PostConfigurable.subSlicesPerFrame = %d",pStatus->status.l1PostConfigurable.subSlicesPerFrame));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.numPlp = %d",pStatus->status.l1PostConfigurable.numPlp));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.numAux = %d",pStatus->status.l1PostConfigurable.numAux));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.fefType = %d",pStatus->status.l1PostConfigurable.fefType));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.rfIdx = %d",pStatus->status.l1PostConfigurable.rfIdx));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.fefInterval = %d",pStatus->status.l1PostConfigurable.fefInterval));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.frequency = %d",pStatus->status.l1PostConfigurable.frequency));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.fefLength = %d",pStatus->status.l1PostConfigurable.fefLength));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.auxStreamType = %d",pStatus->status.l1PostConfigurable.auxStreamType));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.auxPrivateConf = %d",pStatus->status.l1PostConfigurable.auxPrivateConf ));

        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpA.plpId = %d",pStatus->status.l1PostConfigurable.plpA.plpId));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpA.plpGroupId = %d",pStatus->status.l1PostConfigurable.plpA.plpGroupId));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpA.plpType = %d",pStatus->status.l1PostConfigurable.plpA.plpType));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpA.plpPayloadType = %d",pStatus->status.l1PostConfigurable.plpA.plpPayloadType));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpA.ffFlag = %d",pStatus->status.l1PostConfigurable.plpA.ffFlag));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpA.firstRfIdx = %d",pStatus->status.l1PostConfigurable.plpA.firstRfIdx));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpA.firstFrameIdx = %d",pStatus->status.l1PostConfigurable.plpA.firstFrameIdx));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpA.plpCodeRate = %d",pStatus->status.l1PostConfigurable.plpA.plpCodeRate ));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpA.plpMod = %d",pStatus->status.l1PostConfigurable.plpA.plpMod));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpA.plpRotation = %d",pStatus->status.l1PostConfigurable.plpA.plpRotation));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpA.plpFecType = %d",pStatus->status.l1PostConfigurable.plpA.plpFecType));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpA.plpNumBlocksMax = %d",pStatus->status.l1PostConfigurable.plpA.plpNumBlocksMax));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpA.frameInterval = %d",pStatus->status.l1PostConfigurable.plpA.frameInterval));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpA.timeIlLength = %d",pStatus->status.l1PostConfigurable.plpA.timeIlLength));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpA.timeIlType = %d",pStatus->status.l1PostConfigurable.plpA.timeIlType));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpA.inBandFlag = %d",pStatus->status.l1PostConfigurable.plpA.inBandFlag));


        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpB.plpId = %d",pStatus->status.l1PostConfigurable.plpB.plpId));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpB.plpGroupId = %d",pStatus->status.l1PostConfigurable.plpB.plpGroupId));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpB.plpType = %d",pStatus->status.l1PostConfigurable.plpB.plpType));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpB.plpPayloadType = %d",pStatus->status.l1PostConfigurable.plpB.plpPayloadType));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpB.ffFlag = %d",pStatus->status.l1PostConfigurable.plpB.ffFlag));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpB.firstRfIdx = %d",pStatus->status.l1PostConfigurable.plpB.firstRfIdx));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpB.firstFrameIdx = %d",pStatus->status.l1PostConfigurable.plpB.firstFrameIdx));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpB.plpCodeRate = %d",pStatus->status.l1PostConfigurable.plpB.plpCodeRate ));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpB.plpMod = %d",pStatus->status.l1PostConfigurable.plpB.plpMod));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpB.plpRotation = %d",pStatus->status.l1PostConfigurable.plpB.plpRotation));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpB.plpFecType = %d",pStatus->status.l1PostConfigurable.plpB.plpFecType));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpB.plpNumBlocksMax = %d",pStatus->status.l1PostConfigurable.plpB.plpNumBlocksMax));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpB.frameInterval = %d",pStatus->status.l1PostConfigurable.plpB.frameInterval));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpB.timeIlLength = %d",pStatus->status.l1PostConfigurable.plpB.timeIlLength));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpB.timeIlType = %d",pStatus->status.l1PostConfigurable.plpB.timeIlType));
        BDBG_MSG(("pStatus->status.l1PostConfigurable.plpB.inBandFlag = %d",pStatus->status.l1PostConfigurable.plpB.inBandFlag));
        break;
    case NEXUS_FrontendDvbt2StatusType_eL1PostDynamic:
        BDBG_MSG(("pStatus->status.l1PostDynamic.frameIdx = %d",pStatus->status.l1PostDynamic.frameIdx));
        BDBG_MSG(("pStatus->status.l1PostDynamic.l1ChanlgeCounter = %d",pStatus->status.l1PostDynamic.l1ChanlgeCounter));
        BDBG_MSG(("pStatus->status.l1PostDynamic.startRfIdx = %d",pStatus->status.l1PostDynamic.startRfIdx));
        BDBG_MSG(("pStatus->status.l1PostDynamic.subSliceInterval = %d",pStatus->status.l1PostDynamic.subSliceInterval));
        BDBG_MSG(("pStatus->status.l1PostDynamic.type2Start = %d",pStatus->status.l1PostDynamic.type2Start));
        BDBG_MSG(("pStatus->status.l1PostDynamic.auxPrivateDyn_31_0 = %d",pStatus->status.l1PostDynamic.auxPrivateDyn_31_0));
        BDBG_MSG(("pStatus->status.l1PostDynamic.auxPrivateDyn_47_32 = %d",pStatus->status.l1PostDynamic.auxPrivateDyn_47_32));
        BDBG_MSG(("pStatus->status.l1PostDynamic.plpA.plpId = %d",pStatus->status.l1PostDynamic.plpA.plpId));
        BDBG_MSG(("pStatus->status.l1PostDynamic.plpA.plpNumBlocks = %d",pStatus->status.l1PostDynamic.plpA.plpNumBlocks));
        BDBG_MSG(("pStatus->status.l1PostDynamic.plpA.plpStart = %d",pStatus->status.l1PostDynamic.plpA.plpStart));
        BDBG_MSG(("pStatus->status.l1PostDynamic.plpB.plpId = %d",pStatus->status.l1PostDynamic.plpB.plpId));
        BDBG_MSG(("pStatus->status.l1PostDynamic.plpB.plpNumBlocks = %d",pStatus->status.l1PostDynamic.plpB.plpNumBlocks));
        BDBG_MSG(("pStatus->status.l1PostDynamic.plpB.plpStart = %d",pStatus->status.l1PostDynamic.plpB.plpStart));
        break;
    case NEXUS_FrontendDvbt2StatusType_eL1Plp:
        BDBG_MSG(("pStatus->status.l1Plp.numPlp = %d",pStatus->status.l1Plp.numPlp));

        for(i=0; i<pStatus->status.l1Plp.numPlp; i++) {
            BDBG_MSG(("pStatus->status.l1Plp.plp[%d].plpId = %d", i,pStatus->status.l1Plp.plp[i].plpId));
            BDBG_MSG(("pStatus->status.l1Plp.plp[%d].plpGroupId = %d",i,pStatus->status.l1Plp.plp[i].plpGroupId));
            BDBG_MSG(("pStatus->status.l1Plp.plp[%d].plpPayloadType = %d",i,pStatus->status.l1Plp.plp[i].plpPayloadType));
            BDBG_MSG(("pStatus->status.l1Plp.plp[%d].plpType = %d",i,pStatus->status.l1Plp.plp[i].plpType));
        }
        break;
    case NEXUS_FrontendDvbt2StatusType_eBasic:
        BDBG_MSG(("pStatus->status.basic.fecLock = %d",pStatus->status.basic.fecLock));
        BDBG_MSG(("pStatus->status.basic.spectrumInverted = %d",pStatus->status.basic.spectrumInverted));
        BDBG_MSG(("pStatus->status.basic.snr = %d",pStatus->status.basic.snr));
        BDBG_MSG(("pStatus->status.basic.gainOffset = %d",pStatus->status.basic.gainOffset));
        BDBG_MSG(("pStatus->status.basic.carrierOffset = %d",pStatus->status.basic.carrierOffset));
        BDBG_MSG(("pStatus->status.basic.timingOffset = %d",pStatus->status.basic.timingOffset));
        BDBG_MSG(("pStatus->status.basic.signalStrength = %d",pStatus->status.basic.signalStrength));
        BDBG_MSG(("pStatus->status.basic.signalLevelPercent = %d",pStatus->status.basic.signalLevelPercent));
        BDBG_MSG(("pStatus->status.basic.signalQualityPercent = %d",pStatus->status.basic.signalQualityPercent));
        BDBG_MSG(("pStatus->status.basic.reacquireCount = %d",pStatus->status.basic.reacquireCount));
        BDBG_MSG(("pStatus->status.basic.profile = %d",pStatus->status.basic.profile));
        break;
    default:
        BDBG_MSG((" Unsupported status type."));
    }
    BDBG_MSG(("pStatus->type = %d",pStatus->type));

}

static void NEXUS_Frontend_P_Dvbt2PartialtoLegacyStatus(const NEXUS_FrontendDvbt2Status *pPartialStatus, NEXUS_FrontendOfdmStatus *pStatus)
{
    switch ( pPartialStatus->type )
    {
        case NEXUS_FrontendDvbt2StatusType_eBasic:
            pStatus->fecLock = pPartialStatus->status.basic.fecLock;
            pStatus->receiverLock = pPartialStatus->status.basic.fecLock;
            pStatus->spectrumInverted = pPartialStatus->status.basic.spectrumInverted;
            pStatus->snr = pPartialStatus->status.basic.snr;
            pStatus->dvbt2Status.gainOffset = pPartialStatus->status.basic.gainOffset;
            pStatus->carrierOffset = pPartialStatus->status.basic.carrierOffset;
            pStatus->timingOffset = pPartialStatus->status.basic.timingOffset;
            pStatus->signalStrength = pPartialStatus->status.basic.signalStrength;
            pStatus->signalLevelPercent = pPartialStatus->status.basic.signalLevelPercent;
            pStatus->signalQualityPercent = pPartialStatus->status.basic.signalQualityPercent;
            pStatus->reacquireCount = pPartialStatus->status.basic.reacquireCount;
            break;
        case NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpA:
            pStatus->fecCorrectedBlocks = pPartialStatus->status.fecStatistics.bchCorBlk;
            pStatus->fecUncorrectedBlocks = pPartialStatus->status.fecStatistics.bchUncBlk;
            pStatus->fecCleanBlocks = pPartialStatus->status.fecStatistics.bchClnBlk;
            pStatus->viterbiErrorRate = pPartialStatus->status.fecStatistics.ldpcBER;
            break;
        case NEXUS_FrontendDvbt2StatusType_eL1Pre:
            pStatus->guardInterval = pPartialStatus->status.l1Pre.guardInterval;
            switch (pPartialStatus->status.l1Pre.s2>>1)
            {
            case 0:
                pStatus->transmissionMode =  NEXUS_FrontendOfdmTransmissionMode_e2k;
                break;
            case 1:
            case 6:
                pStatus->transmissionMode =  NEXUS_FrontendOfdmTransmissionMode_e8k;
                break;
            case 2:
                pStatus->transmissionMode =  NEXUS_FrontendOfdmTransmissionMode_e4k;
                break;
            case 3:
                pStatus->transmissionMode =  NEXUS_FrontendOfdmTransmissionMode_e1k;
                break;
            case 4:
                pStatus->transmissionMode =  NEXUS_FrontendOfdmTransmissionMode_e16k;
                break;
            case 5:
            case 7:
                pStatus->transmissionMode =  NEXUS_FrontendOfdmTransmissionMode_e32k;
                break;
            default:
                BDBG_WRN(("Unrecognized transmissionMode (%d) reported by BTHD", (pPartialStatus->status.l1Pre.s2>>1)));
                BERR_TRACE(BERR_NOT_SUPPORTED);
            }
            break;
        case NEXUS_FrontendDvbt2StatusType_eL1PostConfigurable:
            if(pPartialStatus->status.l1PostConfigurable.plpA.plpMod < NEXUS_FrontendDvbt2PlpMod_eMax)
                pStatus->modulation = pPartialStatus->status.l1PostConfigurable.plpA.plpMod;
            else {
                BDBG_WRN(("Unrecognized modulation(%d) reported by BTHD", pPartialStatus->status.l1PostConfigurable.plpA.plpMod));
                BERR_TRACE(BERR_NOT_SUPPORTED);
            }
            break;
        default:
            BDBG_ERR((" Unsupported status type."));
            BERR_TRACE(BERR_NOT_SUPPORTED);
        }
}

static void NEXUS_Frontend_P_PrintOfdmStatus(NEXUS_FrontendOfdmStatus *pStatus)
{
    BDBG_MSG(("pStatus->modulation = %d", pStatus->modulation));
    BDBG_MSG(("pStatus->receiverLock = %d",pStatus->receiverLock));
    BDBG_MSG(("pStatus->fecLock = %d",pStatus->fecLock));
    BDBG_MSG(("pStatus->spectrumInverted = %d",pStatus->spectrumInverted));
    BDBG_MSG(("pStatus->reacquireCount = %d",pStatus->reacquireCount));
    BDBG_MSG(("pStatus->snr = %d",pStatus->snr));
    BDBG_MSG(("pStatus->carrierOffset = %d",pStatus->carrierOffset));
    BDBG_MSG(("pStatus->timingOffset = %d",pStatus->timingOffset));
    BDBG_MSG(("pStatus->signalStrength = %d",pStatus->signalStrength));
    BDBG_MSG(("pStatus->guardInterval = %d", pStatus->dvbt2Status.l1PreStatus.guardInterval));
    BDBG_MSG(("pStatus->dvbt2Status.gainOffset = %d",pStatus->dvbt2Status.gainOffset));
}

static NEXUS_Error NEXUS_Frontend_P_7563_RequestOfdmAsyncStatus(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_InternalGainSettings settings;
    NEXUS_GainParameters params;
    NEXUS_ExternalGainSettings externalGainSettings;
    NEXUS_7563 *pDevice = (NEXUS_7563 *)handle;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);

    if(pDevice->pGenericDeviceHandle->parent){

        params.rfInput = pDevice->pGenericDeviceHandle->linkSettings.rfInput;
        params.accumulateTillRootDevice = true;
        params.frequency = pDevice->last_ofdm[0].frequency;

        rc = NEXUS_FrontendDevice_P_GetInternalGain(pDevice->pGenericDeviceHandle->parent, &params, &settings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        externalGainSettings.totalGain = settings.totalVariableGain;

        rc = NEXUS_FrontendDevice_P_SetExternalGain(pDevice->pGenericDeviceHandle, &externalGainSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

    }

    switch ( pDevice->lastChannel )
    {
    case NEXUS_7563_DVBT2_CHN:
        rc = NEXUS_Frontend_P_7563_RequestDvbt2AsyncStatus(pDevice, NEXUS_FrontendDvbt2StatusType_eBasic);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = NEXUS_Frontend_P_7563_RequestDvbt2AsyncStatus(pDevice, NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpA);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = NEXUS_Frontend_P_7563_RequestDvbt2AsyncStatus(pDevice, NEXUS_FrontendDvbt2StatusType_eL1Pre);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = NEXUS_Frontend_P_7563_RequestDvbt2AsyncStatus(pDevice, NEXUS_FrontendDvbt2StatusType_eL1PostConfigurable);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        break;
    case NEXUS_7563_DVBT_CHN:
        rc = BODS_RequestSelectiveAsyncStatus(pDevice->ods_chn[NEXUS_7563_DVBT_CHN], BODS_SelectiveAsyncStatusType_eDvbt);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        break;
    default:
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }
done:
    return rc;
}

static NEXUS_FrontendOfdmTransmissionMode NEXUS_Frontend_P_OdsToTransmissionMode(BODS_DvbtTransmissionMode mode)
{
    switch ( mode )
    {
    case BODS_DvbtTransmissionMode_e2K:
        return NEXUS_FrontendOfdmTransmissionMode_e2k;
    case BODS_DvbtTransmissionMode_e4K:
        return NEXUS_FrontendOfdmTransmissionMode_e4k;
    case BODS_DvbtTransmissionMode_e8K:
        return NEXUS_FrontendOfdmTransmissionMode_e8k;
    default:
        BDBG_WRN(("Unrecognized transmission mode."));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return BODS_DvbtTransmissionMode_e8K;
    }
}

static NEXUS_FrontendOfdmModulation NEXUS_Frontend_P_THDToModulation(BODS_DvbtModulation modulation)
{
    switch ( modulation )
    {
    case BODS_DvbtModulation_eQpsk:
        return NEXUS_FrontendOfdmModulation_eQpsk;
    case BODS_DvbtModulation_e16Qam:
        return NEXUS_FrontendOfdmModulation_eQam16;
    case BODS_DvbtModulation_e64Qam:
        return NEXUS_FrontendOfdmModulation_eQam64;
    default:
        BDBG_WRN(("Unrecognized modulation mode (%d) reported by BODS", modulation));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return NEXUS_FrontendOfdmModulation_eQam64;
    }
}
static NEXUS_FrontendOfdmCodeRate NEXUS_Frontend_P_THDToCodeRate(BODS_DvbtCodeRate codeRate)
{
    switch ( codeRate )
    {
    case BODS_DvbtCodeRate_e1_2:
        return NEXUS_FrontendOfdmCodeRate_e1_2;
    case BODS_DvbtCodeRate_e2_3:
        return NEXUS_FrontendOfdmCodeRate_e2_3;
    case BODS_DvbtCodeRate_e3_4:
        return NEXUS_FrontendOfdmCodeRate_e3_4;
    case BODS_DvbtCodeRate_e5_6:
        return NEXUS_FrontendOfdmCodeRate_e5_6;
    case BODS_DvbtCodeRate_e7_8:
        return NEXUS_FrontendOfdmCodeRate_e7_8;
    default:
        BDBG_WRN(("Unrecognized codeRate (%d) reported by BODS", codeRate));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return NEXUS_FrontendOfdmCodeRate_e1_2;
    }
}

static NEXUS_FrontendOfdmGuardInterval NEXUS_Frontend_P_OdsToGuardInterval(BODS_DvbtGuardInterval guard)
{
    switch ( guard )
    {
    case BODS_DvbtGuardInterval_e1_4:
        return NEXUS_FrontendOfdmGuardInterval_e1_4;
    case BODS_DvbtGuardInterval_e1_8:
        return NEXUS_FrontendOfdmGuardInterval_e1_8;
    case BODS_DvbtGuardInterval_e1_16:
        return NEXUS_FrontendOfdmGuardInterval_e1_16;
    case BODS_DvbtGuardInterval_e1_32:
        return NEXUS_FrontendOfdmGuardInterval_e1_32;
    default:
        BDBG_WRN(("Unrecognized guard interval (%d) reported by BODS", guard));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return NEXUS_FrontendOfdmGuardInterval_e1_4;
    }
}

static NEXUS_FrontendOfdmHierarchy NEXUS_Frontend_P_THDToHierarchy(BODS_DvbtHierarchy magnum)
{
    switch ( magnum )
    {
    case BODS_DvbtHierarchy_e0:
        return NEXUS_FrontendOfdmHierarchy_e0;
    case BODS_DvbtHierarchy_e1:
        return NEXUS_FrontendOfdmHierarchy_e1;
    case BODS_DvbtHierarchy_e2:
        return NEXUS_FrontendOfdmHierarchy_e2;
    case BODS_DvbtHierarchy_e4:
        return NEXUS_FrontendOfdmHierarchy_e4;
    default:
        BDBG_WRN(("Unrecognized hierarchy (%d) reported by BODS", magnum));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return NEXUS_FrontendOfdmHierarchy_e0;
    }
}

static NEXUS_Error NEXUS_Frontend_P_7563_GetOfdmAsyncStatus(void *handle, NEXUS_FrontendOfdmStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_FrontendDvbt2StatusReady t2StatusReady;
    NEXUS_7563 *pDevice = (NEXUS_7563 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    switch ( pDevice->lastChannel )
    {
    case NEXUS_7563_DVBT2_CHN:
        BKNI_Memset(&pDevice->t2PartialStatus, 0, sizeof(pDevice->t2PartialStatus));

        rc =  NEXUS_Frontend_P_7563_GetDvbt2AsyncStatusReady(pDevice, &t2StatusReady);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        if((t2StatusReady.type[NEXUS_FrontendDvbt2StatusType_eBasic]) && (t2StatusReady.type[NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpA]) &&
           (t2StatusReady.type[NEXUS_FrontendDvbt2StatusType_eL1Pre]) && (t2StatusReady.type[NEXUS_FrontendDvbt2StatusType_eL1PostConfigurable]))
        {
            rc = NEXUS_Frontend_P_7563_GetDvbt2AsyncStatus(pDevice, NEXUS_FrontendDvbt2StatusType_eBasic, &pDevice->t2PartialStatus);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            NEXUS_Frontend_P_Dvbt2PartialtoLegacyStatus(&pDevice->t2PartialStatus, pStatus);

            rc = NEXUS_Frontend_P_7563_GetDvbt2AsyncStatus(pDevice, NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpA, &pDevice->t2PartialStatus);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            NEXUS_Frontend_P_Dvbt2PartialtoLegacyStatus(&pDevice->t2PartialStatus, pStatus);

            rc = NEXUS_Frontend_P_7563_GetDvbt2AsyncStatus(pDevice, NEXUS_FrontendDvbt2StatusType_eL1Pre, &pDevice->t2PartialStatus);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            NEXUS_Frontend_P_Dvbt2PartialtoLegacyStatus(&pDevice->t2PartialStatus, pStatus);

            rc = NEXUS_Frontend_P_7563_GetDvbt2AsyncStatus(pDevice, NEXUS_FrontendDvbt2StatusType_eL1PostConfigurable, &pDevice->t2PartialStatus);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            NEXUS_Frontend_P_Dvbt2PartialtoLegacyStatus(&pDevice->t2PartialStatus, pStatus);

            NEXUS_Frontend_P_PrintOfdmStatus(pStatus);
        }
        else{
            BDBG_ERR(("Status not ready. Eror reading status."));
            rc = BERR_TRACE(rc); goto done;
        }
        break;
    case NEXUS_7563_DVBT_CHN:
        BKNI_Memset(&pDevice->odsStatus, 0, sizeof(pDevice->odsStatus));

        rc = BODS_GetSelectiveAsyncStatus(pDevice->ods_chn[NEXUS_7563_DVBT_CHN], BODS_SelectiveAsyncStatusType_eDvbt, &pDevice->odsStatus);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        pStatus->receiverLock = pDevice->odsStatus.status.dvbt.receiverLock;
        pStatus->fecLock = pDevice->odsStatus.status.dvbt.fecLock;
        pStatus->noSignalDetected = pDevice->odsStatus.status.dvbt.noSignalDetected;
        pStatus->transmissionMode = NEXUS_Frontend_P_OdsToTransmissionMode(pDevice->odsStatus.status.dvbt.transmissionMode);
        pStatus->guardInterval = NEXUS_Frontend_P_OdsToGuardInterval(pDevice->odsStatus.status.dvbt.guardInterval);
        pStatus->signalStrength = pDevice->odsStatus.status.dvbt.signalStrength/10;
        pStatus->signalLevelPercent = pDevice->odsStatus.status.dvbt.signalLevelPercent;
        pStatus->signalQualityPercent = pDevice->odsStatus.status.dvbt.signalQualityPercent;
        pStatus->carrierOffset = pDevice->odsStatus.status.dvbt.carrierOffset;
        pStatus->timingOffset = pDevice->odsStatus.status.dvbt.timingOffset;
        pStatus->snr = pDevice->odsStatus.status.dvbt.snr*100/256;
        pStatus->spectrumInverted = pDevice->odsStatus.status.dvbt.spectrumInverted;
        pStatus->reacquireCount = pDevice->odsStatus.status.dvbt.reacqCount;
        pStatus->modulation = NEXUS_Frontend_P_THDToModulation(pDevice->odsStatus.status.dvbt.modulation);
        pStatus->codeRate = NEXUS_Frontend_P_THDToCodeRate(pDevice->odsStatus.status.dvbt.codeRate);
        pStatus->hierarchy = NEXUS_Frontend_P_THDToHierarchy(pDevice->odsStatus.status.dvbt.hierarchy);
        pStatus->cellId = pDevice->odsStatus.status.dvbt.cellId;
        pStatus->fecCorrectedBlocks = pDevice->odsStatus.status.dvbt.rsCorrectedBlocks;
        pStatus->fecUncorrectedBlocks = pDevice->odsStatus.status.dvbt.rsUncorrectedBlocks;
        pStatus->fecCleanBlocks = pDevice->odsStatus.status.dvbt.rsCleanBlocks;
        pStatus->reacquireCount = pDevice->odsStatus.status.dvbt.reacqCount;
        pStatus->viterbiErrorRate = pDevice->odsStatus.status.dvbt.viterbiBer;
        pStatus->preViterbiErrorRate = pDevice->odsStatus.status.dvbt.preViterbiBer;
        break;
    default:
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }

    pStatus->settings = pDevice->last_ofdm[0];

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7563_GetOfdmStatus(void *handle, NEXUS_FrontendOfdmStatus *pStatus)
{
    NEXUS_FrontendFastStatus fastStatus;
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7563 *pDevice = (NEXUS_7563 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    BKNI_Memset(&fastStatus, 0, sizeof(fastStatus));

    BSTD_UNUSED(handle);
    BSTD_UNUSED(pStatus);

    BDBG_WRN(("This api is not supported anymore. Please use NEXUS_Frontend_RequestOfdmAsyncStatus() and NEXUS_Frontend_GetOfdmAsyncStatus()."));

    rc = NEXUS_Frontend_P_7563_GetFastStatus(handle, &fastStatus);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    switch ( fastStatus.lockStatus)
    {
    case NEXUS_FrontendLockStatus_eUnlocked:
        pStatus->fecLock = false;
        break;
    case NEXUS_FrontendLockStatus_eNoSignal:
    case NEXUS_FrontendLockStatus_eLocked:
        pStatus->fecLock = true;
        break;
    default:
        BDBG_WRN(("Unrecognized lock status (%d) ", fastStatus.lockStatus));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return NEXUS_FrontendLockStatus_eUnknown;
    }
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7563_RequestDvbtAsyncStatus(void *handle, NEXUS_FrontendDvbtStatusType type)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7563 *pDevice = (NEXUS_7563 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);
    BSTD_UNUSED(type);

    pDevice->isStatusReady = false;
    rc = NEXUS_Frontend_P_7563_RequestOfdmAsyncStatus(pDevice);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7563_GetDvbtAsyncStatusReady(void *handle, NEXUS_FrontendDvbtStatusReady *pAsyncStatusReady)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7563 *pDevice = (NEXUS_7563 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);

    if(pDevice->isStatusReady){
        pAsyncStatusReady->type[NEXUS_FrontendDvbtStatusType_eBasic] = true;
    }
    else{
        pAsyncStatusReady->type[NEXUS_FrontendDvbtStatusType_eBasic] = false;
    }

    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7563_GetDvbtAsyncStatus(void *handle, NEXUS_FrontendDvbtStatusType type, NEXUS_FrontendDvbtStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7563 *pDevice = (NEXUS_7563 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    BKNI_Memset(&pDevice->odsStatus, 0, sizeof(pDevice->odsStatus));

    if(type == NEXUS_FrontendDvbtStatusType_eBasic){
        rc = BODS_GetSelectiveAsyncStatus(pDevice->ods_chn[NEXUS_7563_DVBT_CHN], BODS_SelectiveAsyncStatusType_eDvbt, &pDevice->odsStatus);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        pStatus->status.basic.fecLock = pDevice->odsStatus.status.dvbt.fecLock;
        pStatus->status.basic.spectrumInverted = pDevice->odsStatus.status.dvbt.spectrumInverted;
        pStatus->status.basic.snr = pDevice->odsStatus.status.dvbt.snr*100/256;
        pStatus->status.basic.carrierOffset = pDevice->odsStatus.status.dvbt.carrierOffset;
        pStatus->status.basic.timingOffset = pDevice->odsStatus.status.dvbt.timingOffset;
        pStatus->status.basic.gainOffset = pDevice->odsStatus.status.dvbt.gainOffset;
        pStatus->status.basic.signalStrength = pDevice->odsStatus.status.dvbt.signalStrength/10;
        pStatus->status.basic.signalLevelPercent = pDevice->odsStatus.status.dvbt.signalLevelPercent;
        pStatus->status.basic.signalQualityPercent = pDevice->odsStatus.status.dvbt.signalQualityPercent;
        pStatus->status.basic.reacquireCount = pDevice->odsStatus.status.dvbt.reacqCount;
        pStatus->status.basic.viterbiErrorRate.rate = pDevice->odsStatus.status.dvbt.viterbiBer;

        pStatus->status.basic.tps.modulation = NEXUS_Frontend_P_THDToModulation(pDevice->odsStatus.status.dvbt.modulation);
        pStatus->status.basic.tps.transmissionMode = NEXUS_Frontend_P_OdsToTransmissionMode(pDevice->odsStatus.status.dvbt.transmissionMode);
        pStatus->status.basic.tps.guardInterval = NEXUS_Frontend_P_OdsToGuardInterval(pDevice->odsStatus.status.dvbt.guardInterval);
        pStatus->status.basic.tps.codeRate = NEXUS_Frontend_P_THDToCodeRate(pDevice->odsStatus.status.dvbt.codeRate);
        pStatus->status.basic.tps.hierarchy = NEXUS_Frontend_P_THDToHierarchy(pDevice->odsStatus.status.dvbt.hierarchy);
        pStatus->status.basic.tps.cellId = pDevice->odsStatus.status.dvbt.cellId;
        pStatus->status.basic.tps.inDepthSymbolInterleave = pDevice->odsStatus.status.dvbt.inDepthSymbolInterleave;
        pStatus->status.basic.tps.timeSlicing = pDevice->odsStatus.status.dvbt.timeSlicing;
        pStatus->status.basic.tps.mpeFec = pDevice->odsStatus.status.dvbt.mpeFec;

        pStatus->status.basic.fecBlockCounts.corrected = pDevice->odsStatus.status.dvbt.rsCorrectedBlocks;
        pStatus->status.basic.fecBlockCounts.uncorrected = pDevice->odsStatus.status.dvbt.rsUncorrectedBlocks;
        pStatus->status.basic.fecBlockCounts.clean = pDevice->odsStatus.status.dvbt.rsCleanBlocks;
    }

    pStatus->status.basic.settings = pDevice->last_ofdm[0];
    pDevice->isStatusReady = false;
done:
    return rc;

}


static NEXUS_Error NEXUS_FrontendDevice_P_7563_GetStatus(void *handle, NEXUS_FrontendDeviceStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7563 *pDevice;
    NEXUS_AvsStatus avsData;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7563 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7563);
    BDBG_ASSERT(NULL != handle);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    rc = NEXUS_GetAvsStatus(&avsData);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if(avsData.enabled){
        pStatus->avs.enabled = true;
        pStatus->avs.voltage = avsData.voltage;
        pStatus->temperature = avsData.temperature;
    }
    else
        pStatus->avs.enabled = false;

    BDBG_WRN(("pStatus->avs.enabled = %d", pStatus->avs.enabled));
    BDBG_WRN(("pStatus->avs.voltage = %d", pStatus->avs.voltage));
    BDBG_WRN(("pStatus->temperature = %d", pStatus->temperature));
done:
    return rc;
}

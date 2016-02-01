/******************************************************************************
 *    (c)2011-2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
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
#include "btnr_3461ib.h"
#include "bhab_3461.h"
#include "bhab_3461_priv.h"
#include "btc2_3461.h"
#include "bthd_3461.h"
#include "bhab_3461_fw.h"
#include "bchp_hsi.h"
#include "../../b0/bchp_tm.h"
#include "bhab.h"
#include "btc2.h"
#if NEXUS_AMPLIFIER_SUPPORT
#include "nexus_amplifier.h"
#endif
BDBG_MODULE(nexus_frontend_3461);

BDBG_OBJECT_ID(NEXUS_3461);

#define BCHIP_3461 0x3461

#define NEXUS_MAX_3461_FRONTENDS 3

/* Currently there is only one ADS channel on 3461/3114/3112/3109 */
#define NEXUS_3461_TC2_CHN 0
#define NEXUS_3461_THD_CHN      (NEXUS_3461_TC2_CHN +1)

/* The 3461 has one downstream. */
#define NEXUS_MAX_3461_TUNERS  1

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

typedef struct NEXUS_3461
{
    BDBG_OBJECT(NEXUS_3461)
    BLST_S_ENTRY(NEXUS_3461) node;
    uint16_t  chipId;
    BHAB_Handle hab;
    BHAB_Capabilities capabilities;
    unsigned    numfrontends;
    uint16_t revId;
    BTHD_Handle thd;
    BTC2_Handle tc2;
    BTNR_Handle                tnr[NEXUS_MAX_3461_TUNERS];
    NEXUS_FrontendHandle       frontendHandle;
    BTC2_ChannelHandle         thd_chn[NEXUS_MAX_3461_TUNERS];
    BTC2_ChannelHandle         tc2_chn[NEXUS_MAX_3461_TUNERS];
    NEXUS_FrontendOfdmSettings last_ofdm[NEXUS_MAX_3461_TUNERS];
    BTC2_InbandParam           last_tc2Param;
    NEXUS_FrontendFastStatus   last_status;
    NEXUS_IsrCallbackHandle    lockAppCallback[NEXUS_MAX_3461_FRONTENDS];
    NEXUS_IsrCallbackHandle    asyncStatusAppCallback[NEXUS_MAX_3461_FRONTENDS];
    bool                       isInternalAsyncStatusCall[NEXUS_MAX_3461_FRONTENDS];
    BKNI_EventHandle isrEvent;
    uint8_t lastChannel;
    NEXUS_EventCallbackHandle isrEventCallback;
    unsigned agcValue;                        /* Gain Value*/
    NEXUS_CallbackDesc updateGainCallbackDesc;    /* Callback will be called when the gain from the lna needs to be updated. */
    NEXUS_3461TunerRfInput rfInput;
    NEXUS_3461RfDaisyChain rfDaisyChain;
    bool enableRfLoopThrough;
    bool terrestrial;
    bool isAsyncStatusReady[NEXUS_MAX_3461_FRONTENDS];
    bool isStatusReady;
    bool isPoweredOn[NEXUS_MAX_3461_FRONTENDS];
    bool isTunerPoweredOn;
    bool acquireInProgress;
    unsigned count;
    NEXUS_FrontendDvbt2Status partialStatus;
    BTHD_THDStatus thdStatus;
    BTC2_SelectiveStatus t2Status;
    NEXUS_FrontendOfdmAcquisitionMode lastAcquisitionMode[NEXUS_MAX_3461_FRONTENDS];
    NEXUS_FrontendDevice *pGenericDeviceHandle;
    bool isInternalSetSettings;
#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT
    bool isMaster;
#endif
    NEXUS_ThreadHandle deviceOpenThread;
    NEXUS_FrontendDevice3461OpenSettings openSettings;
    BREG_I2C_Handle i2cRegHandle;
    BREG_SPI_Handle spiRegHandle;
} NEXUS_3461;

static BLST_S_HEAD(devList, NEXUS_3461) g_3461DevList = BLST_S_INITIALIZER(g_3461DevList);

typedef struct NEXUS_3461Channel
{
    unsigned chn_num; /* channel number */
    NEXUS_3461 *pDevice; /* 3461 device*/
} NEXUS_3461Channel;

/***************************************************************************
 * Module callback functions for tuning
 ***************************************************************************/
static void NEXUS_Frontend_P_3461_UninstallCallbacks(void *handle);
static NEXUS_Error NEXUS_Frontend_P_3461_TuneOfdm(void *handle, const NEXUS_FrontendOfdmSettings *pSettings);
static void NEXUS_Frontend_P_3461_UnTune(void *handle);
static void NEXUS_Frontend_P_3461_ResetStatus(void *handle);
static NEXUS_Error NEXUS_Frontend_P_3461_GetOfdmStatus(void *handle, NEXUS_FrontendOfdmStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_3461_GetOfdmAsyncStatus(void *handle, NEXUS_FrontendOfdmStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_3461_RequestOfdmAsyncStatus(void *handle);
static NEXUS_Error NEXUS_Frontend_P_3461_ReadSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length, size_t *pNumRead);
static uint16_t NEXUS_Frontend_P_Get3461Rev(const NEXUS_FrontendDevice3461OpenSettings *pSettings);
static void NEXUS_Frontend_P_PrintTC2Status(NEXUS_FrontendOfdmStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_3461_GetFastStatus(void *handle, NEXUS_FrontendFastStatus *pStatus);
static void NEXUS_Frontend_P_3461_Close(NEXUS_FrontendHandle handle);
static void NEXUS_Frontend_P_3461_GetType(void *handle, NEXUS_FrontendType *type);
static void NEXUS_Frontend_P_3461_UnInstallCallbacks(void *handle);
static NEXUS_Error NEXUS_Frontend_P_3461_RequestDvbt2AsyncStatus(void *handle, NEXUS_FrontendDvbt2StatusType type);
static NEXUS_Error NEXUS_Frontend_P_3461_GetDvbt2AsyncStatusReady(void *handle, NEXUS_FrontendDvbt2StatusReady *pStatusReady);
static NEXUS_Error NEXUS_Frontend_P_3461_GetDvbt2AsyncStatus(void *handle, NEXUS_FrontendDvbt2StatusType type, NEXUS_FrontendDvbt2Status *pStatus);
static void NEXUS_Frontend_P_PrintDvbt2PartialStatus(NEXUS_FrontendDvbt2Status *pStatus);
static NEXUS_Error NEXUS_Frontend_P_3461_WriteRegister(void *handle, unsigned address, uint32_t value);
static NEXUS_Error NEXUS_Frontend_P_3461_ReadRegister(void *handle, unsigned address, uint32_t *value   );
static NEXUS_Error NEXUS_Frontend_P_3461_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings);
static NEXUS_Error NEXUS_Frontend_P_3461_setCrystalDaisySettings(const NEXUS_FrontendDevice3461OpenSettings *pSettings);
static NEXUS_Error NEXUS_Frontend_P_3461_RequestDvbtAsyncStatus(void *handle, NEXUS_FrontendDvbtStatusType type);
static NEXUS_Error NEXUS_Frontend_P_3461_GetDvbtAsyncStatusReady(void *handle, NEXUS_FrontendDvbtStatusReady *pAsyncStatusReady);
static NEXUS_Error NEXUS_Frontend_P_3461_GetDvbtAsyncStatus(void *handle, NEXUS_FrontendDvbtStatusType type, NEXUS_FrontendDvbtStatus *pStatus);
static NEXUS_Error NEXUS_FrontendDevice_P_3461_GetStatus(void * handle, NEXUS_FrontendDeviceStatus *pStatus);

static NEXUS_Error NEXUS_FrontendDevice_P_3461_Standby(void * handle, const NEXUS_StandbySettings *pSettings);
static NEXUS_Error NEXUS_Tuner_P_3461_GetNSetGain(void *handle);
static void NEXUS_FrontendDevice_P_3461_GetCapabilities(void *handle, NEXUS_FrontendDeviceCapabilities *pCapabilities);
static void NEXUS_FrontendDevice_P_3461_GetTunerCapabilities(void *handle, unsigned tunerIndex, NEXUS_FrontendCapabilities *pCapabilities);
NEXUS_Error NEXUS_FrontendDevice_P_3461_GetInternalGain(void *handle, const NEXUS_GainParameters *params, NEXUS_InternalGainSettings *pSettings);
NEXUS_Error NEXUS_FrontendDevice_P_3461_GetExternalGain(void *handle, NEXUS_ExternalGainSettings *pSettings);
NEXUS_Error NEXUS_FrontendDevice_P_3461_SetExternalGain(void *handle, const NEXUS_ExternalGainSettings *pSettings);
static void NEXUS_FrontendDevice_P_3461_Close(void *handle);
#if NEXUS_AMPLIFIER_SUPPORT
NEXUS_Error NEXUS_FrontendDevice_P_3461_GetAmplifierStatus(void *handle, NEXUS_AmplifierStatus *pStatus);
NEXUS_Error NEXUS_FrontendDevice_P_3461_SetAmplifierStatus(void *handle, const NEXUS_AmplifierStatus *pStatus);
#endif

static uint16_t NEXUS_Frontend_P_Get3461Rev(const NEXUS_FrontendDevice3461OpenSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BREG_I2C_Handle i2cRegHandle;
    uint8_t buf[2];
    uint16_t revId=0xef;
    uint8_t subAddr;
    uint8_t wData[2], rData[4];

    if(pSettings->i2cDevice){
        i2cRegHandle = NEXUS_I2c_GetRegHandle(pSettings->i2cDevice, NULL);
        if(i2cRegHandle == NULL ){rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto done;}
        buf[0]= 0x0;
        subAddr = 0x3;
        rc = BREG_I2C_WriteNoAddr(i2cRegHandle, pSettings->i2cAddr, (uint8_t *)&subAddr, 1);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        rc = BREG_I2C_ReadNoAddr(i2cRegHandle, pSettings->i2cAddr, buf, 1);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        revId = buf[0];

        subAddr = 0x4;
        rc = BREG_I2C_WriteNoAddr(i2cRegHandle, pSettings->i2cAddr, (uint8_t *)&subAddr, 1);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        rc = BREG_I2C_ReadNoAddr(i2cRegHandle, pSettings->i2cAddr, buf, 1);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        revId = (revId <<8) | buf[0];
    }
    else if(pSettings->spiDevice){
        wData[0] = pSettings->spiAddr << 1;   /* Chip address is 0x40 << 1 */
        wData[1] = 0x03;                      /* Register offset. */

        rc = NEXUS_Spi_Read(pSettings->spiDevice, wData, rData, 4);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        revId = (rData[2] <<8) | rData[3];
    }

    return revId;
done:
    return 0;
}

static uint16_t NEXUS_Frontend_P_Is3461(const NEXUS_FrontendDevice3461OpenSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BREG_I2C_Handle i2cRegHandle;
    uint8_t buf[2];
    uint8_t subAddr;
    uint16_t chipId=0;
    uint8_t wData[2], rData[4];


    if(pSettings->i2cDevice){
        i2cRegHandle = NEXUS_I2c_GetRegHandle(pSettings->i2cDevice, NULL);
        if(i2cRegHandle == NULL ){rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto done;}
        buf[0]= 0x0;
        subAddr = 0x1;
        rc = BREG_I2C_WriteNoAddr(i2cRegHandle, pSettings->i2cAddr, (uint8_t *)&subAddr, 1);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        rc = BREG_I2C_ReadNoAddr(i2cRegHandle, pSettings->i2cAddr, buf, 1);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        chipId = buf[0];
        subAddr = 0x2;
        rc = BREG_I2C_WriteNoAddr(i2cRegHandle, pSettings->i2cAddr, (uint8_t *)&subAddr, 1);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        rc = BREG_I2C_ReadNoAddr(i2cRegHandle, pSettings->i2cAddr, buf, 1);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        chipId = (chipId <<8) | buf[0];
    }
    else if (pSettings->spiDevice){
        wData[0] = pSettings->spiAddr << 1;   /* Chip address is 0x40 << 1 */
        wData[1] = 0x01;                      /* Register offset. */

        rc = NEXUS_Spi_Read(pSettings->spiDevice, wData, rData, 4);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        chipId = (rData[2] <<8) | rData[3];
    }

    return chipId;
done:
    return 0;
}

/***************************************************************************
Summary:
    Lock callback handler for a 3461 Inband device
 ***************************************************************************/
static void NEXUS_Frontend_P_3461_callback_isr(void *pParam)
{
    NEXUS_3461 *pDevice;
    BDBG_ASSERT(pParam != NULL);
    pDevice = (NEXUS_3461 *)pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

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
    Lock callback handler for a 3461 Inband device
 ***************************************************************************/
static void NEXUS_Frontend_P_3461_AsyncStatusCallback_isr(void *pParam)
{
    NEXUS_3461 *pDevice;
    BDBG_ASSERT(pParam != NULL);
    pDevice = (NEXUS_3461 *)pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

    if(pDevice->isInternalAsyncStatusCall[pDevice->lastChannel]){
        pDevice->isAsyncStatusReady[pDevice->lastChannel] = true;
    }
    else {
        if (pDevice->asyncStatusAppCallback[pDevice->lastChannel])
        {
            pDevice->isStatusReady = true;
            NEXUS_IsrCallback_Fire_isr(pDevice->asyncStatusAppCallback[pDevice->lastChannel]);
        }
    }
}

/***************************************************************************
Summary:
    Enable/Disable interrupts for a 3461 device
 ***************************************************************************/
static void NEXUS_Frontend_P_3461_L1_isr(void *pParam1, int pParam2)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT
    NEXUS_3461 *pDevice=NULL;
    BSTD_UNUSED(pParam1);
    BSTD_UNUSED(pParam2);
#else
    NEXUS_3461 *pDevice=(NEXUS_3461 *)pParam1;
    BSTD_UNUSED(pParam2);
#endif

#if NEXUS_SHARED_FRONTEND_INTERRUPT
    for ( pDevice = BLST_S_FIRST(&g_3461DevList); NULL != pDevice; pDevice = BLST_S_NEXT(pDevice, node) )
    {
        BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

        if(pDevice->hab)
        {
            rc = BHAB_HandleInterrupt_isr(pDevice->hab);
            if(rc){rc = BERR_TRACE(rc); goto done;}
        }
    }
#else
    if(pDevice->hab)
    {
        rc = BHAB_HandleInterrupt_isr(pDevice->hab);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
#endif
done:
    return;
}
/***************************************************************************
Summary:
    Enable/Disable interrupts for a 3461 device
 ***************************************************************************/
static void NEXUS_Frontend_P_3461_GpioIsrControl_isr(bool enable, void *pParam)
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
    Enable/Disable interrupts for a 3461 device
 ***************************************************************************/
static void NEXUS_Frontend_P_3461_IsrControl_isr(bool enable, void *pParam)
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
    ISR Event Handler for a 3461 device
 ***************************************************************************/
static void NEXUS_Frontend_P_3461_IsrEvent(void *pParam)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3461 *pDevice = (NEXUS_3461 *)pParam;

    if(pDevice->hab != NULL){
        rc = BHAB_ProcessInterruptEvent(pDevice->hab);
        if(rc) BERR_TRACE(rc);
    }
}
/***************************************************************************
Summary:
    Initialize Hab for a 3461 device
***************************************************************************/
void NEXUS_Frontend_P_UnInit_3461_Hab(NEXUS_3461 *pDevice)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    if(pDevice->openSettings.isrNumber) {
#if NEXUS_SHARED_FRONTEND_INTERRUPT
        if(pDevice->isMaster)
        {
            NEXUS_Core_DisconnectInterrupt(pDevice->openSettings.isrNumber);
        }
#else
        NEXUS_Core_DisconnectInterrupt(pDevice->openSettings.isrNumber);
#endif
    }
    else if(pDevice->openSettings.gpioInterrupt){
#if NEXUS_SHARED_FRONTEND_INTERRUPT
        if(pDevice->isMaster)
        {
            NEXUS_Gpio_SetInterruptCallback_priv(pDevice->openSettings.gpioInterrupt, NULL, NULL, 0);

        }
#else
        NEXUS_Gpio_SetInterruptCallback_priv(pDevice->openSettings.gpioInterrupt, NULL, NULL, 0);
#endif
    }

    if(pDevice->isrEventCallback)NEXUS_UnregisterEvent(pDevice->isrEventCallback);
    pDevice->isrEventCallback = NULL;

    if(pDevice->isrEvent) pDevice->isrEvent = NULL;
#if NEXUS_SHARED_FRONTEND_INTERRUPT
    BKNI_EnterCriticalSection();
    BLST_S_REMOVE(&g_3461DevList, pDevice, NEXUS_3461, node);
    BKNI_LeaveCriticalSection();
    if(pDevice->hab) {
        rc = BHAB_Close(pDevice->hab);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
#else
    if(pDevice->hab) {
        rc = BHAB_Close(pDevice->hab);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
#endif
    pDevice->hab = NULL;
    if(pDevice->deviceOpenThread)NEXUS_Thread_Destroy(pDevice->deviceOpenThread);
    pDevice->deviceOpenThread = NULL;

done:
    return;
}

/***************************************************************************
Summary:
    UnInitialize all THD/TC2 channels.
 ***************************************************************************/
void NEXUS_Frontend_P_UnInit3461(NEXUS_3461 *pDevice)
{
    NEXUS_Error rc = BERR_SUCCESS;
    unsigned int i;

    for ( i = 0; i < NEXUS_MAX_3461_TUNERS && NULL != pDevice->tnr[i]; i++) {
        if(pDevice->tnr[i]){
            rc = BTNR_Close(pDevice->tnr[i]);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->tnr[i] = NULL;
        }
    }
    for ( i = 0; i < NEXUS_MAX_3461_TUNERS && NULL != pDevice->tc2_chn[i]; i++) {
        if(pDevice->tc2_chn[i]){
            rc = BTC2_CloseChannel(pDevice->tc2_chn[i]);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            (pDevice->tc2_chn[i]) = NULL;
        }
    }
    if (pDevice->tc2) {
        rc = BTC2_Close(pDevice->tc2);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->tc2 = NULL;
    }
    if (pDevice->thd) {
        rc = BTHD_Close(pDevice->thd);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->thd = NULL;
    }
done:
    return;
}

static void NEXUS_FrontendDevice_P_3461_GetCapabilities(void *handle, NEXUS_FrontendDeviceCapabilities *pCapabilities)
{
    NEXUS_3461 *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_3461 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

    pCapabilities->numTuners = pDevice->capabilities.totalTunerChannels;
    return;
}

static void NEXUS_FrontendDevice_P_3461_GetTunerCapabilities(void *handle, unsigned tunerIndex, NEXUS_FrontendCapabilities *pCapabilities)
{
    NEXUS_3461 *pDevice;
    BHAB_ChannelCapability *channelCapability;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_3461 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);
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
    Initialize all ADS/THD/TC2 channels.
 ***************************************************************************/
static NEXUS_Error NEXUS_Frontend_P_Init3461(NEXUS_3461 *pDevice)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned int i, num_ch;
    BTNR_3461_Settings tnr3461_cfg;
    BTC2_Settings tc2_cfg;
    BTC2_ChannelSettings tc2Chn_cfg;
    BTHD_Settings thd_cfg;
    BTNR_PowerSaverSettings pwrSettings;

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

    rc = BTHD_3461_GetDefaultSettings(&thd_cfg);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    thd_cfg.hGeneric = pDevice->hab;
    thd_cfg.hTmr = g_pCoreHandles->tmr;

    rc = BTHD_Open(&pDevice->thd, g_pCoreHandles->chp, g_pCoreHandles->reg, g_pCoreHandles->bint, &thd_cfg);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = BTHD_Init(pDevice->thd, NULL, 0);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = BTC2_3461_GetDefaultSettings( &tc2_cfg, NULL);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    tc2_cfg.hGeneric = pDevice->hab;

    rc = BTC2_Open(&pDevice->tc2, NULL, NULL, NULL, &tc2_cfg);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = BTC2_Init(pDevice->tc2);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = BTC2_GetTotalChannels(pDevice->tc2, &num_ch);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if(num_ch > NEXUS_MAX_3461_TUNERS) {
        BDBG_ERR(("The maximum number of channels is incorrect."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }

    for (i=0;i<num_ch;i++) {
        rc = BTC2_GetChannelDefaultSettings( pDevice->tc2, i, &tc2Chn_cfg);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        rc = BTC2_OpenChannel( pDevice->tc2, &pDevice->tc2_chn[i], i, &tc2Chn_cfg);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    rc = BTNR_3461_GetDefaultSettings(&tnr3461_cfg);
    if(rc){rc = BERR_TRACE(rc); goto done;}
    rc =  BTNR_3461_Open(&pDevice->tnr[0],&tnr3461_cfg, pDevice->hab);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pwrSettings.enable = false;
    rc = BTNR_SetPowerSaver(pDevice->tnr[0], &pwrSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}
    pDevice->isTunerPoweredOn = true;

    return BERR_SUCCESS;

done:
    NEXUS_Frontend_P_UnInit3461(pDevice);
    return rc;
}

static void NEXUS_Frontend_3461DeviceTestThread(void *arg)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    uint8_t buf=0;
    uint16_t chipId=0, revId=0;
    uint8_t writebuf[3], readbuf[3];
    NEXUS_FrontendDevice3461Settings deviceSettings;
    NEXUS_3461 *pDevice = (NEXUS_3461 *)arg;

    if(!pDevice->pGenericDeviceHandle->abortThread){
        chipId = NEXUS_Frontend_P_Is3461(&pDevice->openSettings);
        if ( chipId != 0x3461 )
        {
            rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
        }
        revId = NEXUS_Frontend_P_Get3461Rev(&pDevice->openSettings);

        pDevice->chipId = chipId;
        pDevice->revId = revId;
    }
    else goto done;

    if(!pDevice->pGenericDeviceHandle->abortThread){
        rc = NEXUS_Frontend_P_3461_setCrystalDaisySettings(&pDevice->openSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    else goto done;

    /* Init the acquisition processor */
    if (pDevice->openSettings.loadAP && !pDevice->pGenericDeviceHandle->abortThread)
    {
        if (pDevice->openSettings.loadAP)
        {
            BDBG_ERR(("BHAB_InitAp(rev a image)"));
            rc = BHAB_InitAp(pDevice->hab, bcm3461_leap_image);
            if ( rc != BERR_SUCCESS ) {
                BDBG_ERR(("Initializing 3461 Frontend core...UNSUCCESSFUL."));
            }
            else{
                BDBG_MSG(("Initializing 3461 Frontend core...SUCCESSFUL!!!"));
            }
        }
    }
    else goto done;

    if(!pDevice->pGenericDeviceHandle->abortThread){
        if(pDevice->openSettings.i2cDevice ){
            rc = BREG_I2C_Read(pDevice->i2cRegHandle, pDevice->openSettings.i2cAddr, HIF_OSC_STRAP_OVRD_XCORE_BIAS, &buf, 1);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            buf &= 0xe0;
            buf |= 0x14;
            rc = BREG_I2C_Write(pDevice->i2cRegHandle, pDevice->openSettings.i2cAddr, HIF_OSC_STRAP_OVRD_XCORE_BIAS, &buf, 1);
            if(rc){rc = BERR_TRACE(rc); goto done;}
        }
        else if(pDevice->openSettings.spiDevice ){
            writebuf[0] = (pDevice->openSettings.i2cAddr << 1);
            writebuf[1] = HIF_OSC_STRAP_OVRD_XCORE_BIAS;
            rc = BREG_SPI_Read(pDevice->spiRegHandle,  writebuf, readbuf, 3);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            writebuf[0] = ( pDevice->openSettings.i2cAddr << 1) | 0x1;
            writebuf[1] = HIF_OSC_STRAP_OVRD_XCORE_BIAS;
            readbuf[2] &= 0xe0;
            readbuf[2] |= 0x14;
            writebuf[2] = readbuf[2];
            rc= BREG_SPI_Write (pDevice->spiRegHandle,  writebuf, 3);
            if(rc){rc = BERR_TRACE(rc); goto done;}
        }
    }
    else goto done;

    /* Initialize HAB, THD and TC2 */
    if(!pDevice->pGenericDeviceHandle->abortThread){
        rc = NEXUS_Frontend_P_Init3461(pDevice);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        /* install callback to notify of lock/unlock change */
        rc = BTC2_InstallCallback(pDevice->tc2_chn[0], BTC2_Callback_eLockChange, (BTC2_CallbackFunc)NEXUS_Frontend_P_3461_callback_isr, (void*)pDevice);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        rc = BTC2_InstallCallback(pDevice->tc2_chn[0], BTC2_Callback_eAsyncStatusReady, (BTC2_CallbackFunc)NEXUS_Frontend_P_3461_AsyncStatusCallback_isr, (void*)pDevice);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = BTHD_InstallCallback(pDevice->thd, BTHD_Callback_eLockChange, (BTHD_CallbackFunc)NEXUS_Frontend_P_3461_callback_isr, (void*)pDevice);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        rc = BTHD_InstallCallback(pDevice->thd, BTHD_Callback_eAsyncStatusReady, (BTHD_CallbackFunc)NEXUS_Frontend_P_3461_AsyncStatusCallback_isr, (void*)pDevice);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        NEXUS_FrontendDevice_GetDefault3461Settings(&deviceSettings);
        deviceSettings.updateGainCallback = pDevice->updateGainCallbackDesc;
        deviceSettings.agcValue = pDevice->agcValue;
        deviceSettings.rfDaisyChain = pDevice->rfDaisyChain;
        deviceSettings.enableRfLoopThrough = pDevice->enableRfLoopThrough;
        deviceSettings.rfInput = pDevice->rfInput;
        deviceSettings.terrestrial = pDevice->terrestrial;

        pDevice->isInternalSetSettings = true;
        rc = NEXUS_FrontendDevice_Set3461Settings(pDevice->pGenericDeviceHandle, &deviceSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    else goto done;

    BKNI_EnterCriticalSection();
    pDevice->pGenericDeviceHandle->openPending = false;
    pDevice->pGenericDeviceHandle->openFailed = false;
    BKNI_LeaveCriticalSection();

    return;
done:
    BKNI_EnterCriticalSection();
    pDevice->pGenericDeviceHandle->openPending = false;
    pDevice->pGenericDeviceHandle->openFailed = true;
    BKNI_LeaveCriticalSection();
}



/***************************************************************************
Summary:
    Initialize Hab for a 3461 device
***************************************************************************/
NEXUS_Error NEXUS_FrontendDevice_P_Init3461(NEXUS_3461 *pDevice, const NEXUS_FrontendDevice3461OpenSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHAB_Settings stHabSettings;
    NEXUS_ThreadSettings thread_settings;
#if NEXUS_SHARED_FRONTEND_INTERRUPT
    NEXUS_3461 *ptempDevice = BLST_S_FIRST(&g_3461DevList);
#endif
    rc = BHAB_3461_GetDefaultSettings(&stHabSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    stHabSettings.chipAddr = pSettings->i2cAddr;
    stHabSettings.slaveChipAddr = pSettings->i2cSlaveAddr;

    if(pSettings->isrNumber) {
        stHabSettings.interruptEnableFunc = NEXUS_Frontend_P_3461_IsrControl_isr;
        stHabSettings.interruptEnableFuncParam = (void*)pSettings->isrNumber;
    }
    else if(pSettings->gpioInterrupt){
        stHabSettings.interruptEnableFunc = NEXUS_Frontend_P_3461_GpioIsrControl_isr;
        stHabSettings.interruptEnableFuncParam = (void*)pSettings->gpioInterrupt;
    }

    if(pSettings->i2cDevice ){
        pDevice->i2cRegHandle = NEXUS_I2c_GetRegHandle(pDevice->openSettings.i2cDevice, NEXUS_MODULE_SELF);
        if(pDevice->i2cRegHandle == NULL ){rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto done;}

        stHabSettings.chipAddr = pSettings->i2cAddr;
        stHabSettings.isSpi = false;
        rc = BHAB_Open( &pDevice->hab, (void *)(pDevice->i2cRegHandle), &stHabSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    else if(pSettings->spiDevice ){
        pDevice->spiRegHandle = NEXUS_Spi_GetRegHandle(pDevice->openSettings.spiDevice);
        if(pDevice->spiRegHandle == NULL ){rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto done;}
        stHabSettings.chipAddr = pSettings->spiAddr;
        stHabSettings.isSpi = true;
        rc = BHAB_Open( &pDevice->hab, (void *)(pDevice->spiRegHandle), &stHabSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    else{
        if(g_pCoreHandles->reg == NULL ){rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto done;}
        /* This is a temporary hack. */
        rc = BHAB_Open( &pDevice->hab, (void *)g_pCoreHandles->reg, &stHabSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

#if NEXUS_SHARED_FRONTEND_INTERRUPT
    /* disconnect the previous for shared interrupt, last one is master */
    if(ptempDevice != NULL){
        if(pSettings->isrNumber) {
            NEXUS_Core_DisconnectInterrupt(pSettings->isrNumber);
        }
        else if(pSettings->gpioInterrupt) {
            NEXUS_Gpio_SetInterruptCallback_priv(pSettings->gpioInterrupt, NULL, NULL, 0);
        }
    }
#endif

    /* Success opeining Hab.  Connect Interrupt */
    if(pSettings->isrNumber) {
        rc = NEXUS_Core_ConnectInterrupt(pSettings->isrNumber, NEXUS_Frontend_P_3461_L1_isr, (void *)pDevice, (int)pDevice->hab);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    else if(pSettings->gpioInterrupt){
        NEXUS_Gpio_SetInterruptCallback_priv(pSettings->gpioInterrupt, NEXUS_Frontend_P_3461_L1_isr, (void *)pDevice, (int)pDevice->hab);
    }

    /* Get events and register callbacks */
    rc = BHAB_GetInterruptEventHandle(pDevice->hab, &pDevice->isrEvent);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pDevice->isrEventCallback = NEXUS_RegisterEvent(pDevice->isrEvent, NEXUS_Frontend_P_3461_IsrEvent, pDevice);
    if ( NULL == pDevice->isrEventCallback ){rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done; }

    NEXUS_Thread_GetDefaultSettings(&thread_settings);
    thread_settings.priority = 50;
    pDevice->pGenericDeviceHandle->abortThread = false;
    pDevice->deviceOpenThread = NEXUS_Thread_Create("deviceOpenThread",
                                                NEXUS_Frontend_3461DeviceTestThread,
                                                (void*)pDevice,
                                                &thread_settings);

    return BERR_SUCCESS;

done:
    NEXUS_Frontend_P_UnInit_3461_Hab(pDevice);
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

/***************************************************************************
Summary:
    Get the default settings for a BCM3461 tuner
See Also:
    NEXUS_Frontend_Open3461
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

static NEXUS_Error NEXUS_Frontend_P_3461_WriteRegister(void *handle, unsigned address, uint32_t value)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3461 *pDevice = (NEXUS_3461 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

    rc = BHAB_WriteRegister(pDevice->hab, address, &value);
    if(rc){rc = BERR_TRACE(rc);}

    return rc;
}
static NEXUS_Error NEXUS_Frontend_P_3461_ReadRegister(void *handle, unsigned address, uint32_t *value   )
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3461 *pDevice = (NEXUS_3461 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

    rc = BHAB_ReadRegister(pDevice->hab, address, value);
    if(rc){rc = BERR_TRACE(rc);}

    return NEXUS_SUCCESS;
}

#if NEXUS_AMPLIFIER_SUPPORT
NEXUS_Error NEXUS_FrontendDevice_P_3461_GetAmplifierStatus(void *handle, NEXUS_AmplifierStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3461 *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_3461 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

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

NEXUS_Error NEXUS_FrontendDevice_P_3461_SetAmplifierStatus(void *handle, const NEXUS_AmplifierStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3461 *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_3461 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

    BSTD_UNUSED(pStatus);
    /* Set LNA/Amplifier parameters. */

    return rc;
}
#endif

NEXUS_Error NEXUS_FrontendDevice_P_3461_GetInternalGain(void *handle, const NEXUS_GainParameters *params, NEXUS_InternalGainSettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3461 *pDevice;
    BHAB_InternalGainInputParams inputParams;
    BHAB_InternalGainSettings internalGain;
    NEXUS_GainParameters gainParams;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_3461 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

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
NEXUS_Error NEXUS_FrontendDevice_P_3461_GetExternalGain(void *handle, NEXUS_ExternalGainSettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3461 *pDevice;
    BHAB_ExternalGainSettings gain;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_3461 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);
    BSTD_UNUSED(pSettings);

    rc = BHAB_GetExternalGain(pDevice->hab, &gain);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pSettings->bypassableGain = gain.externalGainBypassable;
    pSettings->totalGain = gain.externalGainTotal;

done:
    return rc;

}
NEXUS_Error NEXUS_FrontendDevice_P_3461_SetExternalGain(void *handle, const NEXUS_ExternalGainSettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3461 *pDevice;
    BHAB_ExternalGainSettings externalGain;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_3461 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

    externalGain.externalGainTotal = pSettings->totalGain;
    externalGain.externalGainBypassable = pSettings->bypassableGain;

    rc = BHAB_SetExternalGain(pDevice->hab, &externalGain);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}

NEXUS_Error NEXUS_Tuner_P_3461_GetNSetGain(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_GainParameters params;
    NEXUS_InternalGainSettings settings;
    NEXUS_ExternalGainSettings externalGainSettings;
    NEXUS_3461 *pDevice = (NEXUS_3461 *)handle;

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

NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_GetHandle(unsigned index, const NEXUS_FrontendDevice3461OpenSettings *pSettings)
{
    NEXUS_3461 *pDevice = NULL;
    BSTD_UNUSED(index);

    for ( pDevice = BLST_S_FIRST(&g_3461DevList); NULL != pDevice; pDevice = BLST_S_NEXT(pDevice, node) )
    {
        BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);
        if(!BKNI_Memcmp(pSettings, &pDevice->openSettings, sizeof(NEXUS_FrontendDevice3461OpenSettings)))
        {
            return pDevice->pGenericDeviceHandle;
        }
    }

    return NULL;
}

NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_Open3461(unsigned index, const NEXUS_FrontendDevice3461OpenSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3461 *pDevice = NULL;
    NEXUS_FrontendDevice *pFrontendDevice = NULL;
    NEXUS_3461ProbeResults results;

    BSTD_UNUSED(index);

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT((NULL != pSettings->i2cDevice) || (NULL != pSettings->spiDevice));

    for ( pDevice = BLST_S_FIRST(&g_3461DevList); NULL != pDevice; pDevice = BLST_S_NEXT(pDevice, node) )
    {
        BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);
        if(((pSettings->i2cAddr == pDevice->openSettings.i2cAddr) && (pSettings->i2cDevice== pDevice->openSettings.i2cDevice)) &&
            ((pSettings->spiAddr == pDevice->openSettings.spiAddr) && (pSettings->spiDevice== pDevice->openSettings.spiDevice)))
        {
            break;
        }
    }

    if ( NULL == pDevice)
    {
#if NEXUS_SHARED_FRONTEND_INTERRUPT
        for ( pDevice = BLST_S_FIRST(&g_3461DevList); NULL != pDevice; pDevice = BLST_S_NEXT(pDevice, node) )
        {
            BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);
            if ((pSettings->gpioInterrupt == pDevice->openSettings.gpioInterrupt) && (pSettings->isrNumber == pDevice->openSettings.isrNumber))
            {
                pDevice->isMaster = false;
            }
        }
#endif
        pFrontendDevice = BKNI_Malloc(sizeof(*pFrontendDevice));
        if (NULL == pFrontendDevice) {rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_create;}

        /* Memsetting the whole structure should cover initializing the child list. */
        BKNI_Memset(pFrontendDevice, 0, sizeof(*pFrontendDevice));

        pDevice = BKNI_Malloc(sizeof(*pDevice));
        if (NULL == pDevice) {rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_create;}

        BKNI_Memset(pDevice, 0, sizeof(NEXUS_3461));
        BDBG_OBJECT_SET(pDevice, NEXUS_3461);

        pDevice->openSettings = *pSettings;

        pDevice->pGenericDeviceHandle = pFrontendDevice;
#if NEXUS_SHARED_FRONTEND_INTERRUPT
        pDevice->isMaster = true;
#endif

        BKNI_EnterCriticalSection();
        pDevice->pGenericDeviceHandle->openPending = true;
        pDevice->pGenericDeviceHandle->openFailed = false;
        BKNI_LeaveCriticalSection();

        rc = NEXUS_FrontendDevice_P_Init3461(pDevice, pSettings);
        if(rc){rc = BERR_TRACE(rc); goto err_init;}

#if NEXUS_SHARED_FRONTEND_INTERRUPT
        BKNI_EnterCriticalSection();
        BLST_S_INSERT_HEAD(&g_3461DevList, pDevice, node);
        BKNI_LeaveCriticalSection();
#else
        BLST_S_INSERT_HEAD(&g_3461DevList, pDevice, node);
#endif

    }
    else
    {
        BDBG_WRN(("Found device"));
        return pDevice->pGenericDeviceHandle;
    }

    pFrontendDevice->pDevice = pDevice;
    pFrontendDevice->familyId = results.chip.familyId;
    pFrontendDevice->mode = NEXUS_StandbyMode_eOn;
    pFrontendDevice->bypassableFixedGain = pSettings->externalFixedGain.bypassable;
    pFrontendDevice->totalFixedBoardGain = pSettings->externalFixedGain.total;
    pFrontendDevice->application = NEXUS_FrontendDeviceApplication_eUnset;
    pFrontendDevice->getInternalGain = NEXUS_FrontendDevice_P_3461_GetInternalGain;
    pFrontendDevice->getExternalGain = NEXUS_FrontendDevice_P_3461_GetExternalGain;
    pFrontendDevice->setExternalGain = NEXUS_FrontendDevice_P_3461_SetExternalGain;
    pFrontendDevice->getStatus = NEXUS_FrontendDevice_P_3461_GetStatus;
    pFrontendDevice->standby = NEXUS_FrontendDevice_P_3461_Standby;
    pFrontendDevice->close = NEXUS_FrontendDevice_P_3461_Close;
    pFrontendDevice->getCapabilities = NEXUS_FrontendDevice_P_3461_GetCapabilities;
    pFrontendDevice->getTunerCapabilities = NEXUS_FrontendDevice_P_3461_GetTunerCapabilities;

#if NEXUS_AMPLIFIER_SUPPORT
    pFrontendDevice->getAmplifierStatus = NEXUS_Frontend_P_3461_GetAmplifierStatus;
    pFrontendDevice->setAmplifierStatus = NEXUS_Frontend_P_3461_SetAmplifierStatus;
#endif
    return pFrontendDevice;

err_init:
    NEXUS_Frontend_P_UnInit_3461_Hab(pDevice);
    if ( pDevice ) BKNI_Free(pDevice);
    pDevice = NULL;
err_create:
    if(pFrontendDevice) BKNI_Free(pFrontendDevice);
    return NULL;

}

/***************************************************************************
Summary:
    Open a handle to a BCM3461 device.
 ***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Open3461(const NEXUS_3461Settings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_FrontendHandle frontendHandle = NULL;
    NEXUS_3461 *pDevice = NULL;
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
        pDevice = (NEXUS_3461 *)pFrontendDevice->pDevice;
    }
    else {
        pDevice = (NEXUS_3461 *)pSettings->device->pDevice;
        pFrontendDevice =  pSettings->device;
    }

    /* Create a Nexus frontend handle */
    frontendHandle = NEXUS_Frontend_P_Create(pDevice);
    if ( NULL == frontendHandle ) {rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_alloc;}

    /* Establish device capabilities */
    frontendHandle->capabilities.ofdm = true;
    frontendHandle->capabilities.ofdmModes[NEXUS_FrontendOfdmMode_eDvbt] = true;
    frontendHandle->capabilities.ofdmModes[NEXUS_FrontendOfdmMode_eDvbt2] = true;

    frontendHandle->tuneOfdm = NEXUS_Frontend_P_3461_TuneOfdm;
    frontendHandle->untune = NEXUS_Frontend_P_3461_UnTune;
    frontendHandle->getOfdmStatus = NEXUS_Frontend_P_3461_GetOfdmStatus;
    frontendHandle->requestOfdmAsyncStatus = NEXUS_Frontend_P_3461_RequestOfdmAsyncStatus;
    frontendHandle->getOfdmAsyncStatus = NEXUS_Frontend_P_3461_GetOfdmAsyncStatus;
    frontendHandle->resetStatus = NEXUS_Frontend_P_3461_ResetStatus;
    frontendHandle->readSoftDecisions = NEXUS_Frontend_P_3461_ReadSoftDecisions;
    frontendHandle->close = NEXUS_Frontend_P_3461_Close;
    frontendHandle->getFastStatus = NEXUS_Frontend_P_3461_GetFastStatus;
    frontendHandle->getType = NEXUS_Frontend_P_3461_GetType;
    frontendHandle->requestDvbt2AsyncStatus = NEXUS_Frontend_P_3461_RequestDvbt2AsyncStatus;
    frontendHandle->getDvbt2AsyncStatusReady = NEXUS_Frontend_P_3461_GetDvbt2AsyncStatusReady;
    frontendHandle->getDvbt2AsyncStatus = NEXUS_Frontend_P_3461_GetDvbt2AsyncStatus;
    frontendHandle->writeRegister = NEXUS_Frontend_P_3461_WriteRegister;
    frontendHandle->readRegister = NEXUS_Frontend_P_3461_ReadRegister;
    frontendHandle->standby = NEXUS_Frontend_P_3461_Standby;
    frontendHandle->requestDvbtAsyncStatus = NEXUS_Frontend_P_3461_RequestDvbtAsyncStatus;
    frontendHandle->getDvbtAsyncStatusReady = NEXUS_Frontend_P_3461_GetDvbtAsyncStatusReady;
    frontendHandle->getDvbtAsyncStatus = NEXUS_Frontend_P_3461_GetDvbtAsyncStatus;
    frontendHandle->uninstallCallbacks = NEXUS_Frontend_P_3461_UninstallCallbacks;

    frontendHandle->pGenericDeviceHandle = pFrontendDevice;

    /* Create app callbacks */
    for(chn_num=0; chn_num < NEXUS_MAX_3461_FRONTENDS; chn_num++){

        pDevice->lockAppCallback[chn_num] = NEXUS_IsrCallback_Create(frontendHandle, NULL);
        if ( NULL == pDevice->lockAppCallback[chn_num] ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err_cbk_create;}

        pDevice->asyncStatusAppCallback[chn_num] = NEXUS_IsrCallback_Create(frontendHandle, NULL);
        if ( NULL == pDevice->asyncStatusAppCallback[chn_num] ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto err_cbk_create;}
    }

    pDevice->frontendHandle = frontendHandle;
    frontendHandle->chip.familyId = 0x3461;
    frontendHandle->chip.id = 0x3461;
    return frontendHandle;

err_cbk_create:
    for(chn_num=0; chn_num < NEXUS_MAX_3461_FRONTENDS; chn_num++){
        if ( pDevice->lockAppCallback[chn_num] ) NEXUS_IsrCallback_Destroy(pDevice->lockAppCallback[chn_num]);
        if ( pDevice->asyncStatusAppCallback[chn_num] ) NEXUS_IsrCallback_Destroy(pDevice->asyncStatusAppCallback[chn_num]);
    }
    if ( frontendHandle ) BKNI_Free(frontendHandle);
err_alloc:
    NEXUS_Frontend_P_UnInit3461(pDevice);
    return NULL;
}

/***************************************************************************
Summary:
    Close a handle to a BCM3461 device.
See Also:
    NEXUS_Frontend_Open3461
 ***************************************************************************/
static void NEXUS_Frontend_P_3461_Close(NEXUS_FrontendHandle handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3461 *pDevice;
    unsigned chn_num=0;
    BTNR_PowerSaverSettings pwrSettings;

    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    pDevice = (NEXUS_3461 *) handle->pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

    while(pDevice->deviceOpenThread && pDevice->pGenericDeviceHandle->openPending){
        BKNI_Delay(500000); /* Wait for a half second everytime. */
    }

    NEXUS_Frontend_P_3461_UnInstallCallbacks(pDevice);
    for(chn_num=0; chn_num < NEXUS_MAX_3461_FRONTENDS; chn_num++){
        if ( pDevice->lockAppCallback[chn_num] ) NEXUS_IsrCallback_Destroy(pDevice->lockAppCallback[chn_num]);
        if ( pDevice->asyncStatusAppCallback[chn_num] ) NEXUS_IsrCallback_Destroy(pDevice->asyncStatusAppCallback[chn_num]);
    }

    for(chn_num=0; chn_num< NEXUS_MAX_3461_FRONTENDS; chn_num++){
        pDevice->lastChannel = chn_num;
        if(pDevice->isPoweredOn[chn_num]) {
            NEXUS_Frontend_P_3461_UnTune(pDevice);
        }
        pDevice->isPoweredOn[chn_num]=false;
    }
    if(pDevice->isTunerPoweredOn){
        pwrSettings.enable = true;
        rc = BTNR_SetPowerSaver(pDevice->tnr[0], &pwrSettings);
        if(rc){rc = BERR_TRACE(rc);}
    }

    NEXUS_Frontend_P_Destroy(handle);

    pDevice->frontendHandle = NULL;
}

static void NEXUS_FrontendDevice_P_3461_Close(void *handle)
{
    NEXUS_3461 *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_3461 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

    if(pDevice->deviceOpenThread){
        NEXUS_Thread_Destroy(pDevice->deviceOpenThread);
        pDevice->deviceOpenThread = NULL;
    }

    NEXUS_Frontend_P_UnInit3461(pDevice);
    NEXUS_Frontend_P_UnInit_3461_Hab(pDevice);

#ifndef NEXUS_SHARED_FRONTEND_INTERRUPT
    BLST_S_REMOVE(&g_3461DevList, pDevice, NEXUS_3461, node);
#endif

    NEXUS_FrontendDevice_Unlink(pDevice->pGenericDeviceHandle, NULL);
    BKNI_Free(pDevice->pGenericDeviceHandle);
    pDevice->pGenericDeviceHandle = NULL;

    if(pDevice->capabilities.channelCapabilities)
        BKNI_Free(pDevice->capabilities.channelCapabilities);
    pDevice->capabilities.channelCapabilities = NULL;

    BDBG_OBJECT_DESTROY(pDevice, NEXUS_3461);
    BKNI_Free(pDevice);
    pDevice = NULL;

    return;
}

static BTHD_GuardInterval NEXUS_Frontend_P_GuardIntervalToTHD(NEXUS_FrontendOfdmGuardInterval guardInterval)
{
    switch ( guardInterval )
    {
    case NEXUS_FrontendOfdmGuardInterval_e1_4:
        return BTHD_GuardInterval_e1_4;
    case NEXUS_FrontendOfdmGuardInterval_e1_8:
        return BTHD_GuardInterval_e1_8;
    case NEXUS_FrontendOfdmGuardInterval_e1_16:
        return BTHD_GuardInterval_e1_16;
    case NEXUS_FrontendOfdmGuardInterval_e1_32:
        return BTHD_GuardInterval_e1_32;
    default:
        BDBG_WRN((" Unsupported Acquisition mode."));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return BTHD_GuardInterval_e1_32;
    }
}

static BTHD_TransmissionMode NEXUS_Frontend_P_TransmissionModeToTHD(NEXUS_FrontendOfdmTransmissionMode mode)
{
    switch ( mode )
    {
    case NEXUS_FrontendOfdmTransmissionMode_e2k:
        return BTHD_TransmissionMode_e2K;
    case NEXUS_FrontendOfdmTransmissionMode_e4k:
        return BTHD_TransmissionMode_e4K;
    case NEXUS_FrontendOfdmTransmissionMode_e8k:
        return BTHD_TransmissionMode_e8K;
    default:
        BDBG_WRN(("Unrecognized transmission mode."));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return BTHD_TransmissionMode_e8K;
    }
}

static NEXUS_FrontendOfdmModulation NEXUS_Frontend_P_ModulationToTHD(NEXUS_FrontendOfdmModulation modulation)
{
    switch ( modulation )
    {
    case NEXUS_FrontendOfdmModulation_eQpsk:
        return BTHD_Modulation_eQpsk;
    case NEXUS_FrontendOfdmModulation_eQam16:
        return BTHD_Modulation_e16Qam;
    case NEXUS_FrontendOfdmModulation_eQam64:
        return BTHD_Modulation_e64Qam;
    case NEXUS_FrontendOfdmModulation_eDqpsk:
        return BTHD_Modulation_eDqpsk;
    default:
        BDBG_WRN(("Unrecognized modulation (%d)", modulation));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return BTHD_Modulation_e64Qam;
    }
}

static NEXUS_FrontendOfdmModulation NEXUS_Frontend_P_CodeRateToTHD(NEXUS_FrontendOfdmCodeRate codeRate)
{
    switch ( codeRate )
    {
    case NEXUS_FrontendOfdmCodeRate_e1_2:
        return BTHD_CodeRate_e1_2;
    case NEXUS_FrontendOfdmCodeRate_e2_3:
        return BTHD_CodeRate_e2_3;
    case NEXUS_FrontendOfdmCodeRate_e3_4:
        return BTHD_CodeRate_e3_4;
    case NEXUS_FrontendOfdmCodeRate_e5_6:
        return BTHD_CodeRate_e5_6;
    case NEXUS_FrontendOfdmCodeRate_e7_8:
        return BTHD_CodeRate_e7_8;
    default:
        BDBG_WRN(("Unrecognized code rate (%d)", codeRate));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return NEXUS_FrontendOfdmCodeRate_e1_2;
    }
}

static NEXUS_FrontendOfdmModulation NEXUS_Frontend_P_HierarchyToTHD(NEXUS_FrontendOfdmHierarchy hierarchy)
{
    switch ( hierarchy )
    {
    case NEXUS_FrontendOfdmHierarchy_e0:
        return BTHD_Hierarchy_0;
    case NEXUS_FrontendOfdmHierarchy_e1:
        return BTHD_Hierarchy_1;
    case NEXUS_FrontendOfdmHierarchy_e2:
        return BTHD_Hierarchy_2;
    case NEXUS_FrontendOfdmHierarchy_e4:
        return BTHD_Hierarchy_4;
    default:
        BDBG_WRN(("Unrecognized hierarchy (%d)", hierarchy));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return NEXUS_FrontendOfdmHierarchy_e0;
    }
}

static void NEXUS_Frontend_P_3461_UnInstallCallbacks(void *handle)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3461 *pDevice = (NEXUS_3461 *)handle;

    BDBG_MSG(("1 NEXUS_Frontend_P_3461_UnInstallCallbacks: pDevice = 0x%x", pDevice));
    BDBG_MSG(("Tuner is not powered down for now to decrease channel change time."));

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

    if(pDevice->thd){
        rc = BTHD_InstallCallback(pDevice->thd, BTHD_Callback_eLockChange, NULL, NULL);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        rc = BTHD_InstallCallback(pDevice->thd, BTHD_Callback_eAsyncStatusReady, NULL, NULL);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    if(pDevice->tc2_chn[0]){
        rc = BTC2_InstallCallback(pDevice->tc2_chn[0], BTC2_Callback_eLockChange, NULL, NULL);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        rc = BTC2_InstallCallback(pDevice->tc2_chn[0], BTC2_Callback_eAsyncStatusReady, NULL, NULL);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

done:
    return;
}

static NEXUS_Error NEXUS_Frontend_P_3461_TuneOfdm(void *handle, const NEXUS_FrontendOfdmSettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3461 *pDevice = (NEXUS_3461 *)handle;
    BTC2_InbandParam tc2Param;
    BTHD_InbandParams thdParam;
    unsigned chn_num = 0;
    BTNR_Settings tnrSettings;
    unsigned temp_frequency;
    NEXUS_FrontendOfdmMode temp_mode;
    BTNR_PowerSaverSettings pwrSettings;
    NEXUS_FrontendDeviceHandle genericDeviceHandle = NULL;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

    NEXUS_Frontend_P_3461_UnInstallCallbacks(pDevice);
    BDBG_MSG(("mode = %d, bandwidth = %d, frequency = %d", pSettings->mode, pSettings->bandwidth, pSettings->frequency));
    BDBG_MSG(("acquisitionMode = %d, spectrum = %d, spectrumMode = %d, spectralInversion = %d", pSettings->acquisitionMode, pSettings->spectrum, pSettings->spectrumMode, pSettings->spectralInversion));


    switch ( pSettings->mode )
    {
    case NEXUS_FrontendOfdmMode_eDvbt:
        rc = BTHD_InstallCallback(pDevice->thd, BTHD_Callback_eLockChange, (BTHD_CallbackFunc)NEXUS_Frontend_P_3461_callback_isr, (void*)pDevice);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        rc = BTHD_InstallCallback(pDevice->thd, BTHD_Callback_eAsyncStatusReady, (BTHD_CallbackFunc)NEXUS_Frontend_P_3461_AsyncStatusCallback_isr, (void*)pDevice);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = BTHD_GetDefaultInbandParams(pDevice->thd, &thdParam);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        chn_num = NEXUS_3461_THD_CHN;
        break;
    case NEXUS_FrontendOfdmMode_eDvbt2:
        rc = BTC2_InstallCallback(pDevice->tc2_chn[0], BTC2_Callback_eLockChange, (BTC2_CallbackFunc)NEXUS_Frontend_P_3461_callback_isr, (void*)pDevice);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        rc = BTC2_InstallCallback(pDevice->tc2_chn[0], BTC2_Callback_eAsyncStatusReady, (BTC2_CallbackFunc)NEXUS_Frontend_P_3461_AsyncStatusCallback_isr, (void*)pDevice);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = BTC2_GetDefaultAcquireParams(pDevice->tc2_chn[0], &tc2Param);
        chn_num = NEXUS_3461_TC2_CHN;
        break;
    case NEXUS_FrontendOfdmMode_eIsdbt:
    default:
        /* It is IMPORTANT to check this condition here. Because it will not be checked later. */
        BDBG_ERR(("Wrong Ofdm mode selected."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }

    NEXUS_IsrCallback_Set(pDevice->lockAppCallback[chn_num], &(pSettings->lockCallback));
    NEXUS_IsrCallback_Set(pDevice->asyncStatusAppCallback[chn_num], &(pSettings->asyncStatusReadyCallback));

    genericDeviceHandle = pDevice->pGenericDeviceHandle;
    if(genericDeviceHandle == NULL) {
        BDBG_WRN(("Generic device handle for this device is corrupted.")); goto done;
    }


    do{
        if(genericDeviceHandle->application != NEXUS_FrontendDeviceApplication_eTerrestrial){
            BDBG_ERR(("Wrong Tuner application set for device 0x%x. Set NEXUS_TunerApplication_eTerrestrial as its tuner application", (unsigned int)pDevice->pGenericDeviceHandle));
            goto done;
        }
        genericDeviceHandle = genericDeviceHandle->parent;
    }while(genericDeviceHandle != NULL);

    rc = NEXUS_Tuner_P_3461_GetNSetGain(pDevice);
    if(rc){rc = BERR_TRACE(rc); goto done;}

     if(!pDevice->isPoweredOn[chn_num] || !pDevice->isTunerPoweredOn)
        goto full_acquire;

    /* Note: The previous lock status is not completely accurate as the lock status could change asynchronously inbetween two acquires initiated by the application.
         But, for scan purposes, the time frame between acquires is generally short. Hence this is only an approximation of the previous locked state. If needs be, the accurate state can be implemented. */
    if((pSettings->mode == NEXUS_FrontendOfdmMode_eDvbt2)&&(pDevice->last_ofdm[0].mode == NEXUS_FrontendOfdmMode_eDvbt2) &&
        (pSettings->frequency == pDevice->last_ofdm[0].frequency) && (!pSettings->dvbt2Settings.plpMode) && (pDevice->last_status.lockStatus == NEXUS_FrontendLockStatus_eLocked)){

        pDevice->last_status.lockStatus = NEXUS_FrontendLockStatus_eUnknown;
        pDevice->last_tc2Param.changePlpOnly = true;
        pDevice->last_tc2Param.plpId = pSettings->dvbt2Settings.plpId;
        pDevice->last_tc2Param.plpMode = pSettings->dvbt2Settings.plpMode;
        rc = BTC2_SetAcquireParams( pDevice->tc2_chn[0], &pDevice->last_tc2Param );
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = BTC2_Acquire( pDevice->tc2_chn[0], NULL );
        if(rc){rc = BERR_TRACE(rc); goto done;}
        return rc;
    }

    if((pSettings->acquisitionMode == NEXUS_FrontendOfdmAcquisitionMode_eScan) && (pDevice->lastAcquisitionMode[chn_num] == NEXUS_FrontendOfdmAcquisitionMode_eScan)){
        temp_frequency = pDevice->last_ofdm[0].frequency;
        pDevice->last_ofdm[0].frequency = pSettings->frequency ;
        temp_mode = pDevice->last_ofdm[0].mode;
        pDevice->last_ofdm[0].mode = pSettings->mode;

        if(!BKNI_Memcmp(pSettings, &pDevice->last_ofdm[0], sizeof(NEXUS_FrontendOfdmSettings))) {
            if (pDevice->tnr[0])
            {
                pDevice->count = 0;
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
    switch ( pSettings->bandwidth )
    {
    case 1700000:
        tc2Param.bandwidth = BTC2_Bandwidth_e1p7MHz;
        break;
    case 5000000:
        tc2Param.bandwidth = BTC2_Bandwidth_e5MHz;
        thdParam.bandwidth = BTHD_Bandwidth_5Mhz;
        break;
    case 6000000:
        tc2Param.bandwidth = BTC2_Bandwidth_e6MHz;
        thdParam.bandwidth = BTHD_Bandwidth_6Mhz;
        break;
    case 7000000:
        tc2Param.bandwidth = BTC2_Bandwidth_e7MHz;
        thdParam.bandwidth = BTHD_Bandwidth_7Mhz;
        break;
    case 8000000:
        tc2Param.bandwidth = BTC2_Bandwidth_e8MHz;
        thdParam.bandwidth = BTHD_Bandwidth_8Mhz;
        break;
    default:
        BDBG_ERR((" Unsupported bandwidth."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }

    switch ( pSettings->acquisitionMode )
    {
    case NEXUS_FrontendOfdmAcquisitionMode_eScan:
        tc2Param.tuneAcquire = true;
        thdParam.bTuneAcquire = true;
        /* No break as we set the modes to manual for scan.*/
    case NEXUS_FrontendOfdmAcquisitionMode_eAuto:
        /* Due to get default parameters, the tc2Param.tuneAcquire and thdParam.bTuneAcquire are set to false. */
        tc2Param.acqMode = BTC2_AcquireMode_eAuto;
        thdParam.eAcquisitionMode = BTHD_ThdAcquisitionMode_eAuto;
        break;
    case NEXUS_FrontendOfdmAcquisitionMode_eManual:
        /* Due to get default parameters, the tc2Param.tuneAcquire and thdParam.bTuneAcquire are set to false. */
        tc2Param.acqMode = BTC2_AcquireMode_eManual;
        thdParam.eAcquisitionMode = BTHD_ThdAcquisitionMode_eManual;
        break;
    default:
        BDBG_ERR((" Unsupported Acquisition mode."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }

    switch ( pSettings->mode )
    {
    case NEXUS_FrontendOfdmMode_eDvbt:
        if(!pDevice->isPoweredOn[chn_num]){
            rc = BTHD_PowerUp(pDevice->thd);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->isPoweredOn[chn_num] = true;
        }
        thdParam.mode = BTHD_InbandMode_eDvbt;
        if(pSettings->priority == NEXUS_FrontendOfdmPriority_eHigh){
            thdParam.decodeMode = BTHD_Decode_Hp;
        }
        else
            thdParam.decodeMode = BTHD_Decode_Lp;

        if(pSettings->manualModeSettings){
            thdParam.eModeGuardAcquire = BTHD_ModeGuard_eManual;
            thdParam.eTransmissionMode = NEXUS_Frontend_P_TransmissionModeToTHD(pSettings->modeSettings.mode);
            thdParam.eGuardInterval = NEXUS_Frontend_P_GuardIntervalToTHD(pSettings->modeSettings.guardInterval);
        }
        else
            thdParam.eModeGuardAcquire = BTHD_ModeGuard_eAuto;

        thdParam.cciMode = (BTHD_CCIMode)pSettings->cciMode;
        thdParam.ePullinRange = pSettings->pullInRange;
        /* Nexus to PI, there is reversal in logic. */
        if(pSettings->manualTpsSettings) {
            thdParam.bTpsAcquire = false;
            thdParam.eModulation = NEXUS_Frontend_P_ModulationToTHD(pSettings->tpsSettings.modulation);
            thdParam.eCodeRateHP = NEXUS_Frontend_P_CodeRateToTHD(pSettings->tpsSettings.highPriorityCodeRate);
            thdParam.eCodeRateLP = NEXUS_Frontend_P_CodeRateToTHD(pSettings->tpsSettings.lowPriorityCodeRate);
            thdParam.eHierarchy = NEXUS_Frontend_P_HierarchyToTHD(pSettings->tpsSettings.hierarchy);
        }
        else
            thdParam.bTpsAcquire = true;

        thdParam.spectrumMode = (BTHD_SpectrumMode)pSettings->spectrumMode;
        thdParam.invertSpectrum = (BTHD_InvertSpectrum)pSettings->spectralInversion;
        if(pSettings->spectrum){
            thdParam.spectrumMode = (BTHD_SpectrumMode)BTHD_SpectrumMode_eManual;
            switch ( pSettings->spectrumMode )
            {
            case NEXUS_FrontendOfdmSpectrum_eNonInverted:
                thdParam.invertSpectrum = BTHD_InvertSpectrum_eNormal;
                break;
            case NEXUS_FrontendOfdmSpectrum_eInverted:
                thdParam.invertSpectrum = BTHD_InvertSpectrum_eInverted;
                break;
            default:
                BDBG_ERR((" Unsupported spectrum inversion mode."));
                rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
            }
        }

        rc = BTHD_SetAcquireParams(pDevice->thd, &thdParam );
        if(rc){rc = BERR_TRACE(rc); goto done;}
        break;
    default:
    case NEXUS_FrontendOfdmMode_eDvbt2:
        if(!pDevice->isPoweredOn[chn_num]){
            rc = BTC2_DisablePowerSaver(pDevice->tc2_chn[0]);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->isPoweredOn[chn_num] = true;
        }
        tc2Param.modType = BTC2_ModulationType_eDvbt2;
        tc2Param.acqMode = pSettings->acquisitionMode;
        tc2Param.plpMode = pSettings->dvbt2Settings.plpMode;
        if(!pSettings->dvbt2Settings.plpMode)
            tc2Param.plpId = pSettings->dvbt2Settings.plpId;

        tc2Param.spectrumMode = (BTC2_SpectrumMode)pSettings->spectrumMode;
        tc2Param.invertSpectrum = (BTC2_InvertSpectrum)pSettings->spectralInversion;
        if(pSettings->spectrum){
            tc2Param.spectrumMode = (BTC2_SpectrumMode)BTC2_SpectrumMode_eManual;
            switch ( pSettings->spectrumMode )
            {
            case NEXUS_FrontendOfdmSpectrum_eNonInverted:
                tc2Param.invertSpectrum = BTC2_InvertSpectrum_eNormal;
                break;
            case NEXUS_FrontendOfdmSpectrum_eInverted:
                tc2Param.invertSpectrum = BTC2_InvertSpectrum_eInverted;
                break;
            default:
                BDBG_ERR((" Unsupported spectrum inversion mode."));
                rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
            }
        }

        rc = BTC2_SetAcquireParams( pDevice->tc2_chn[0], &tc2Param );
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->last_tc2Param = tc2Param;
        break;
    }

    if (pDevice->tnr[0])
    {
        if(!pDevice->isTunerPoweredOn){
            pwrSettings.enable = false;
            rc = BTNR_SetPowerSaver(pDevice->tnr[0], &pwrSettings);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->isTunerPoweredOn = true;
        }

        rc = BTNR_GetSettings(pDevice->tnr[0], &tnrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        if(pSettings->mode == NEXUS_FrontendOfdmMode_eDvbt){
            tnrSettings.std = BTNR_Standard_eDvbt;
        }
        else if(pSettings->mode == NEXUS_FrontendOfdmMode_eDvbt2){
            tnrSettings.std = BTNR_Standard_eDvbt2;
        }
        tnrSettings.bandwidth = pSettings->bandwidth;
        rc = BTNR_SetSettings(pDevice->tnr[0], &tnrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        pDevice->count = 0;
        pDevice->acquireInProgress = true;
        pDevice->lastChannel = chn_num;
        pDevice->lastAcquisitionMode[chn_num] = pSettings->acquisitionMode;
        rc = BTNR_SetTunerRfFreq(pDevice->tnr[0], pSettings->frequency, BTNR_TunerMode_eDigital);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    if ( pSettings->acquisitionMode != NEXUS_FrontendOfdmAcquisitionMode_eScan) {
        switch ( pSettings->mode )
        {
        case NEXUS_FrontendOfdmMode_eDvbt:
            rc = BTHD_TuneAcquire(pDevice->thd, &thdParam);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            break;
        default:
        case NEXUS_FrontendOfdmMode_eDvbt2:
            rc = BTC2_Acquire( pDevice->tc2_chn[0], &tc2Param );
            if(rc){rc = BERR_TRACE(rc); goto done;}
            break;
        }
    }

    pDevice->last_ofdm[0] = *pSettings;
    return BERR_SUCCESS;
retrack:
    pDevice->last_ofdm[0].frequency = temp_frequency;
    pDevice->last_ofdm[0].mode = temp_mode;
done:
    NEXUS_Frontend_P_3461_UnTune(handle);
    return rc;
}

static NEXUS_FrontendLockStatus  NEXUS_Frontend_P_GetLockStatus(unsigned lockStatus)
{
    switch ( lockStatus )
    {
    /*BADS_LockStatus_eUnlocked == BTHD_LockStatus_eUnlocked == BTC2_LockStatus_eUnlocked*/
    case BTHD_LockStatus_eUnlocked:
        return NEXUS_FrontendLockStatus_eUnlocked;
    case BTHD_LockStatus_eLocked:
        return NEXUS_FrontendLockStatus_eLocked;
    case BTHD_LockStatus_eNoSignal:
        return NEXUS_FrontendLockStatus_eNoSignal;
    default:
        BDBG_WRN(("Unrecognized lock status (%d) ", lockStatus));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return NEXUS_FrontendLockStatus_eUnknown;
    }
}
static NEXUS_Error NEXUS_Frontend_P_3461_GetFastStatus(void *handle, NEXUS_FrontendFastStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3461 *pDevice = (NEXUS_3461 *)handle;
    unsigned lock;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

    if(pDevice->isTunerPoweredOn && pDevice->isPoweredOn[pDevice->lastChannel]){
        switch ( pDevice->lastChannel )
        {
        case NEXUS_3461_THD_CHN:
            rc = BTHD_GetThdLockStatus(pDevice->thd,  &lock);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            break;
        case NEXUS_3461_TC2_CHN:
            rc = BTC2_GetLockStatus(pDevice->tc2_chn[0],  &lock);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            break;
        default:
            BDBG_ERR((" Unsupported channel."));
            rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
        }
        pStatus->lockStatus = NEXUS_Frontend_P_GetLockStatus(lock);
        pStatus->acquireInProgress = pDevice->acquireInProgress;
        pDevice->last_status.lockStatus = pStatus->lockStatus;
    }
    else{
        pStatus->lockStatus = NEXUS_FrontendLockStatus_eUnknown;
    }
    return BERR_SUCCESS;
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_3461_RequestOfdmAsyncStatus(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3461 *pDevice = (NEXUS_3461 *)handle;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

    switch ( pDevice->lastChannel )
    {
    case NEXUS_3461_TC2_CHN:
        rc = BTC2_InstallCallback(pDevice->tc2_chn[0], BTC2_Callback_eAsyncStatusReady, NULL, NULL);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = NEXUS_Frontend_P_3461_RequestDvbt2AsyncStatus(pDevice, NEXUS_FrontendDvbt2StatusType_eBasic);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = NEXUS_Frontend_P_3461_RequestDvbt2AsyncStatus(pDevice, NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpA);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = NEXUS_Frontend_P_3461_RequestDvbt2AsyncStatus(pDevice, NEXUS_FrontendDvbt2StatusType_eL1Pre);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = NEXUS_Frontend_P_3461_RequestDvbt2AsyncStatus(pDevice, NEXUS_FrontendDvbt2StatusType_eL1PostConfigurable);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = BTC2_InstallCallback(pDevice->tc2_chn[0], BTC2_Callback_eAsyncStatusReady, (BTC2_CallbackFunc)NEXUS_Frontend_P_3461_AsyncStatusCallback_isr, (void*)pDevice);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = NEXUS_Frontend_P_3461_RequestDvbt2AsyncStatus(pDevice, NEXUS_FrontendDvbt2StatusType_eL1Plp);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        break;
    case NEXUS_3461_THD_CHN:
        rc = BTHD_RequestThdAsyncStatus(pDevice->thd);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        break;
    default:
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_3461_GetOfdmStatus(void *handle, NEXUS_FrontendOfdmStatus *pStatus)
{
    NEXUS_Error rc = BERR_SUCCESS;
    NEXUS_3461 *pDevice = (NEXUS_3461 *)handle;
    unsigned j=0;
    uint32_t buf=0;
    bool callStatus = false;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    if((pDevice->lastChannel != NEXUS_3461_THD_CHN) && (pDevice->lastChannel != NEXUS_3461_TC2_CHN))
    {
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }

    pDevice->isInternalAsyncStatusCall[pDevice->lastChannel] = true;
    pDevice->isAsyncStatusReady[pDevice->lastChannel] = false;

    if(pDevice->isPoweredOn[pDevice->lastChannel]){
        rc = NEXUS_Frontend_P_3461_RequestOfdmAsyncStatus(pDevice);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        for(j=0; j < 100; j++) {

            BKNI_Sleep(10);

            if(pDevice->revId == 0x0) {
#if NEXUS_FRONTEND_3461_A0
                rc = BHAB_ReadRegister(pDevice->hab, BCHP_LEAP_CTRL_HOST_IRQ , &buf);
#endif
            }
            else if(pDevice->revId >= 0x100){
#if NEXUS_FRONTEND_3461_BX
                rc = BHAB_ReadRegister(pDevice->hab, BCHP_LEAP_HOST_IRQ_STATUS0 , &buf);
#endif
            }
            if(rc){rc = BERR_TRACE(rc); goto done;}

            if(((pDevice->lastChannel == NEXUS_3461_THD_CHN) && (buf & BHAB_THD_STATUS_RDY)) ||
               ((pDevice->lastChannel == NEXUS_3461_TC2_CHN) && (buf & BHAB_T2_STATUS_RDY)))
            {
                callStatus = true;
            }

            if((pDevice->isAsyncStatusReady[pDevice->lastChannel]) || (callStatus == true)) {

                rc = NEXUS_Frontend_P_3461_GetOfdmAsyncStatus(pDevice, pStatus);
                if(rc){rc = BERR_TRACE(rc); goto done;}
                pDevice->isInternalAsyncStatusCall[pDevice->lastChannel] = false;
                pDevice->isAsyncStatusReady[pDevice->lastChannel] = false;
                break;
            }
            else {
                continue;
            }
        }
    }
    else{
       BDBG_MSG(("The ofdm core is Powered Off"));
   }

    pStatus->settings = pDevice->last_ofdm[0];

done:
    pDevice->isInternalAsyncStatusCall[pDevice->lastChannel] = false;
    pDevice->isAsyncStatusReady[pDevice->lastChannel] = false;
    return rc;
}

static NEXUS_FrontendOfdmTransmissionMode NEXUS_Frontend_P_THDToTransmissionMode(BTHD_TransmissionMode mode)
{
    switch ( mode )
    {
    case BTHD_TransmissionMode_e2K:
        return NEXUS_FrontendOfdmTransmissionMode_e2k;
    case BTHD_TransmissionMode_e4K:
        return NEXUS_FrontendOfdmTransmissionMode_e4k;
    case BTHD_TransmissionMode_e8K:
        return NEXUS_FrontendOfdmTransmissionMode_e8k;
    default:
        BDBG_WRN(("Unrecognized transmission mode."));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return NEXUS_FrontendOfdmTransmissionMode_e8k;
    }
}

static NEXUS_FrontendOfdmModulation NEXUS_Frontend_P_THDToModulation(BTHD_Modulation magnum)
{
    switch ( magnum )
    {
    case BTHD_Modulation_eQpsk:
        return NEXUS_FrontendOfdmModulation_eQpsk;
    case BTHD_Modulation_e16Qam:
        return NEXUS_FrontendOfdmModulation_eQam16;
    case BTHD_Modulation_e64Qam:
        return NEXUS_FrontendOfdmModulation_eQam64;
    case BTHD_Modulation_eDqpsk:
        return NEXUS_FrontendOfdmModulation_eDqpsk;
    default:
        BDBG_WRN(("Unrecognized modulation mode (%d) reported by BTHD", magnum));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return NEXUS_FrontendOfdmModulation_eQam64;
    }
}
static NEXUS_FrontendOfdmCodeRate NEXUS_Frontend_P_THDToCodeRate(BTHD_CodeRate magnum)
{
    switch ( magnum )
    {
    case BTHD_CodeRate_e1_2:
        return NEXUS_FrontendOfdmCodeRate_e1_2;
    case BTHD_CodeRate_e2_3:
        return NEXUS_FrontendOfdmCodeRate_e2_3;
    case BTHD_CodeRate_e3_4:
        return NEXUS_FrontendOfdmCodeRate_e3_4;
    case BTHD_CodeRate_e5_6:
        return NEXUS_FrontendOfdmCodeRate_e5_6;
    case BTHD_CodeRate_e7_8:
        return NEXUS_FrontendOfdmCodeRate_e7_8;
    default:
        BDBG_WRN(("Unrecognized codeRate (%d) reported by BTHD", magnum));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return NEXUS_FrontendOfdmCodeRate_e1_2;
    }
}

static NEXUS_FrontendOfdmGuardInterval NEXUS_Frontend_P_THDToGuardInterval(BTHD_GuardInterval magnum)
{
    switch ( magnum )
    {
    case BTHD_GuardInterval_e1_4:
        return NEXUS_FrontendOfdmGuardInterval_e1_4;
    case BTHD_GuardInterval_e1_8:
        return NEXUS_FrontendOfdmGuardInterval_e1_8;
    case BTHD_GuardInterval_e1_16:
        return NEXUS_FrontendOfdmGuardInterval_e1_16;
    case BTHD_GuardInterval_e1_32:
        return NEXUS_FrontendOfdmGuardInterval_e1_32;
    default:
        BDBG_WRN(("Unrecognized guard interval (%d) reported by BTHD", magnum));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return NEXUS_FrontendOfdmGuardInterval_e1_4;
    }
}

static NEXUS_FrontendOfdmHierarchy NEXUS_Frontend_P_THDToHierarchy(BTHD_Hierarchy magnum)
{
    switch ( magnum )
    {
    case BTHD_Hierarchy_0:
        return NEXUS_FrontendOfdmHierarchy_e0;
    case BTHD_Hierarchy_1:
        return NEXUS_FrontendOfdmHierarchy_e1;
    case BTHD_Hierarchy_2:
        return NEXUS_FrontendOfdmHierarchy_e2;
    case BTHD_Hierarchy_4:
        return NEXUS_FrontendOfdmHierarchy_e4;
    default:
        BDBG_WRN(("Unrecognized hierarchy (%d) reported by BTHD", magnum));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return NEXUS_FrontendOfdmHierarchy_e0;
    }
}

static void NEXUS_Frontend_P_PrintTC2Status(NEXUS_FrontendOfdmStatus *pStatus)
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

static void NEXUS_Frontend_P_PartialtoLegacyStatus(const NEXUS_FrontendDvbt2Status *pPartialStatus, NEXUS_FrontendOfdmStatus *pStatus)
{
    int i;
    switch ( pPartialStatus->type )    {
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
        case NEXUS_FrontendDvbt2StatusType_eL1Plp:
        {
           pStatus->dvbt2Status.l1PlpStatus.numPlp = pPartialStatus->status.l1Plp.numPlp;
           for(i=0; i<pStatus->dvbt2Status.l1PlpStatus.numPlp; i++)
           {
              pStatus->dvbt2Status.l1PlpStatus.plp[i].plpId           = pPartialStatus->status.l1Plp.plp[i].plpId;
              pStatus->dvbt2Status.l1PlpStatus.plp[i].plpGroupId      = pPartialStatus->status.l1Plp.plp[i].plpGroupId;
              pStatus->dvbt2Status.l1PlpStatus.plp[i].plpPayloadType  = pPartialStatus->status.l1Plp.plp[i].plpPayloadType;
              pStatus->dvbt2Status.l1PlpStatus.plp[i].plpType         = pPartialStatus->status.l1Plp.plp[i].plpType;
          }
        }
           break;
        default:
            BDBG_ERR((" Unsupported status type."));
            BERR_TRACE(BERR_NOT_SUPPORTED);
        }
}


static NEXUS_Error NEXUS_Frontend_P_3461_GetOfdmAsyncStatus(void *handle, NEXUS_FrontendOfdmStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_FrontendDvbt2StatusReady statusReady;
    NEXUS_3461 *pDevice = (NEXUS_3461 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    switch ( pDevice->lastChannel )
    {
    case NEXUS_3461_TC2_CHN:
        BKNI_Memset(&pDevice->partialStatus, 0, sizeof(pDevice->partialStatus));

        rc =  NEXUS_Frontend_P_3461_GetDvbt2AsyncStatusReady(pDevice, &statusReady);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        if((statusReady.type[NEXUS_FrontendDvbt2StatusType_eBasic]) && (statusReady.type[NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpA]) &&
           (statusReady.type[NEXUS_FrontendDvbt2StatusType_eL1Pre]) && (statusReady.type[NEXUS_FrontendDvbt2StatusType_eL1PostConfigurable])&&
           (statusReady.type[NEXUS_FrontendDvbt2StatusType_eL1Plp]))
        {
            rc = NEXUS_Frontend_P_3461_GetDvbt2AsyncStatus(pDevice, NEXUS_FrontendDvbt2StatusType_eBasic, &pDevice->partialStatus);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            NEXUS_Frontend_P_PartialtoLegacyStatus(&pDevice->partialStatus, pStatus);

            rc = NEXUS_Frontend_P_3461_GetDvbt2AsyncStatus(pDevice, NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpA, &pDevice->partialStatus);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            NEXUS_Frontend_P_PartialtoLegacyStatus(&pDevice->partialStatus, pStatus);

            rc = NEXUS_Frontend_P_3461_GetDvbt2AsyncStatus(pDevice, NEXUS_FrontendDvbt2StatusType_eL1Pre, &pDevice->partialStatus);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            NEXUS_Frontend_P_PartialtoLegacyStatus(&pDevice->partialStatus, pStatus);

            rc = NEXUS_Frontend_P_3461_GetDvbt2AsyncStatus(pDevice, NEXUS_FrontendDvbt2StatusType_eL1PostConfigurable, &pDevice->partialStatus);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            NEXUS_Frontend_P_PartialtoLegacyStatus(&pDevice->partialStatus, pStatus);

            rc = NEXUS_Frontend_P_3461_GetDvbt2AsyncStatus(pDevice, NEXUS_FrontendDvbt2StatusType_eL1Plp, &pDevice->partialStatus);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            NEXUS_Frontend_P_PartialtoLegacyStatus(&pDevice->partialStatus, pStatus);
            NEXUS_Frontend_P_PrintTC2Status(pStatus);
        }
        else{
            BDBG_ERR(("Status not ready. Eror reading status."));
            rc = BERR_TRACE(rc); goto done;
        }
        break;
    case NEXUS_3461_THD_CHN:
        BKNI_Memset(&pDevice->thdStatus, 0, sizeof(pDevice->thdStatus));
        rc = BTHD_GetThdAsyncStatus(pDevice->thd,  &pDevice->thdStatus);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        pStatus->fecLock = pDevice->thdStatus.bFecLock;
        pStatus->receiverLock = pDevice->thdStatus.bReceiverLock;
        pStatus->noSignalDetected = pDevice->thdStatus.bNoSignalDetected;
        pStatus->spectrumInverted = pDevice->thdStatus.bSpectrumInverted;
        pStatus->modulation = NEXUS_Frontend_P_THDToModulation(pDevice->thdStatus.eModulation);
        pStatus->transmissionMode = NEXUS_Frontend_P_THDToTransmissionMode(pDevice->thdStatus.eTransmissionMode);
        pStatus->guardInterval = NEXUS_Frontend_P_THDToGuardInterval(pDevice->thdStatus.eGuardInterval);
        pStatus->codeRate = NEXUS_Frontend_P_THDToCodeRate(pDevice->thdStatus.eCodeRate);
        pStatus->hierarchy = NEXUS_Frontend_P_THDToHierarchy(pDevice->thdStatus.eHierarchy);
        pStatus->cellId = pDevice->thdStatus.nCellId;
        pStatus->carrierOffset = pDevice->thdStatus.lCarrierOffset;
        pStatus->timingOffset = pDevice->thdStatus.lTimingOffset;
        pStatus->snr = pDevice->thdStatus.nSnr*100/256;
        pStatus->fecCorrectedBlocks = pDevice->thdStatus.ulRsCorrectedBlocks;
        pStatus->fecUncorrectedBlocks = pDevice->thdStatus.ulRsUncorrectedBlocks;
        pStatus->fecCleanBlocks = pDevice->thdStatus.ulRsCleanBlocks;
        pStatus->reacquireCount = pDevice->thdStatus.ulReacqCount;
        pStatus->signalStrength = pDevice->thdStatus.nSignalStrength/10;
        pStatus->signalLevelPercent = pDevice->thdStatus.signalLevelPercent;
        pStatus->signalQualityPercent = pDevice->thdStatus.signalQualityPercent;
        pStatus->viterbiErrorRate = pDevice->thdStatus.ulViterbiBer;
        pStatus->preViterbiErrorRate = pDevice->thdStatus.ulPreViterbiBer;
        pStatus->rfAgcLevel = pDevice->thdStatus.ulRFAgc;
        pStatus->ifAgcLevel = pDevice->thdStatus.ulIFAgc;

        BDBG_MSG(("To get inDepthSymbolInterleave, timeSlicing, mpeFec or BERT status, please use NEXUS_Frontend_GetDvbtAsyncStatus() api. This api will be deprecated soon."));

        break;
    default:
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }

    pStatus->settings = pDevice->last_ofdm[0];

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_3461_RequestDvbtAsyncStatus(void *handle, NEXUS_FrontendDvbtStatusType type)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3461 *pDevice = (NEXUS_3461 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);
    BSTD_UNUSED(type);

    pDevice->isStatusReady = false;
    rc = NEXUS_Frontend_P_3461_RequestOfdmAsyncStatus(pDevice);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_3461_GetDvbtAsyncStatusReady(void *handle, NEXUS_FrontendDvbtStatusReady *pAsyncStatusReady)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3461 *pDevice = (NEXUS_3461 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

    if(pDevice->isStatusReady){
        pAsyncStatusReady->type[NEXUS_FrontendDvbtStatusType_eBasic] = true;
    }
    else{
        pAsyncStatusReady->type[NEXUS_FrontendDvbtStatusType_eBasic] = false;
    }

    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_3461_GetDvbtAsyncStatus(void *handle, NEXUS_FrontendDvbtStatusType type, NEXUS_FrontendDvbtStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3461 *pDevice = (NEXUS_3461 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    BKNI_Memset(&pDevice->thdStatus, 0, sizeof(pDevice->thdStatus));

    if(type == NEXUS_FrontendDvbtStatusType_eBasic){
        BKNI_Memset(&pDevice->thdStatus, 0, sizeof(pDevice->thdStatus));
        rc = BTHD_GetThdAsyncStatus(pDevice->thd,  &pDevice->thdStatus);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        pStatus->status.basic.fecLock = pDevice->thdStatus.bFecLock;
        pStatus->status.basic.spectrumInverted = pDevice->thdStatus.bSpectrumInverted;
        pStatus->status.basic.snr = pDevice->thdStatus.nSnr*100/256;
        pStatus->status.basic.carrierOffset = pDevice->thdStatus.lCarrierOffset;
        pStatus->status.basic.timingOffset = pDevice->thdStatus.lTimingOffset;
        pStatus->status.basic.gainOffset = pDevice->thdStatus.lgainOffset;
        pStatus->status.basic.signalStrength = pDevice->thdStatus.nSignalStrength/10;
        pStatus->status.basic.signalLevelPercent = pDevice->thdStatus.signalLevelPercent;
        pStatus->status.basic.signalQualityPercent = pDevice->thdStatus.signalQualityPercent;
        pStatus->status.basic.reacquireCount = pDevice->thdStatus.ulReacqCount;
        pStatus->status.basic.viterbiErrorRate.rate = pDevice->thdStatus.ulViterbiBer;

        pStatus->status.basic.tps.modulation = NEXUS_Frontend_P_THDToModulation(pDevice->thdStatus.eModulation);
        pStatus->status.basic.tps.transmissionMode = NEXUS_Frontend_P_THDToTransmissionMode(pDevice->thdStatus.eTransmissionMode);
        pStatus->status.basic.tps.guardInterval = NEXUS_Frontend_P_THDToGuardInterval(pDevice->thdStatus.eGuardInterval);
        pStatus->status.basic.tps.codeRate = NEXUS_Frontend_P_THDToCodeRate(pDevice->thdStatus.eCodeRate);
        pStatus->status.basic.tps.hierarchy = NEXUS_Frontend_P_THDToHierarchy(pDevice->thdStatus.eHierarchy);
        pStatus->status.basic.tps.cellId = pDevice->thdStatus.nCellId;
        pStatus->status.basic.tps.inDepthSymbolInterleave = pDevice->thdStatus.inDepthSymbolInterleave;
        pStatus->status.basic.tps.timeSlicing = pDevice->thdStatus.timeSlicing;
        pStatus->status.basic.tps.mpeFec = pDevice->thdStatus.mpeFec;

        pStatus->status.basic.fecBlockCounts.corrected = pDevice->thdStatus.ulRsCorrectedBlocks;
        pStatus->status.basic.fecBlockCounts.uncorrected = pDevice->thdStatus.ulRsUncorrectedBlocks;
        pStatus->status.basic.fecBlockCounts.clean = pDevice->thdStatus.ulRsCleanBlocks;
    }

    pStatus->status.basic.settings = pDevice->last_ofdm[0];
    pDevice->isStatusReady = false;
done:
    return rc;

}


static NEXUS_Error NEXUS_Frontend_P_3461_RequestDvbt2AsyncStatus(void *handle, NEXUS_FrontendDvbt2StatusType type)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3461 *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_3461 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

    BDBG_CASSERT(NEXUS_FrontendDvbt2StatusType_eMax == (NEXUS_FrontendDvbt2StatusType)BTC2_SelectiveAsyncStatusType_eLast);
    rc = BTC2_RequestSelectiveAsyncStatus(pDevice->tc2_chn[0], (BTC2_SelectiveAsyncStatusType) type);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}
static NEXUS_Error NEXUS_Frontend_P_3461_GetDvbt2AsyncStatusReady(void *handle, NEXUS_FrontendDvbt2StatusReady *pStatusReady)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    BTC2_SelectiveAsyncStatusReady status;
    NEXUS_3461 *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_3461 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

    BKNI_Memset(pStatusReady, 0, sizeof(NEXUS_FrontendDvbt2StatusReady));

    rc = BTC2_GetSelectiveAsyncStatusReadyType(pDevice->tc2_chn[0], &status);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pStatusReady->type[NEXUS_FrontendDvbt2StatusType_eFecStatisticsL1Pre] = status.fecStatisticsL1Pre;
    pStatusReady->type[NEXUS_FrontendDvbt2StatusType_eFecStatisticsL1Post] = status.fecStatisticsL1Post;
    pStatusReady->type[NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpA] = status.fecStatisticsPlpA;
    pStatusReady->type[NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpB] = status.fecStatisticsPlpB;
    pStatusReady->type[NEXUS_FrontendDvbt2StatusType_eL1Pre] = status.l1Pre;
    pStatusReady->type[NEXUS_FrontendDvbt2StatusType_eL1PostConfigurable] = status.l1PostConfigurable;
    pStatusReady->type[NEXUS_FrontendDvbt2StatusType_eL1PostDynamic] = status.l1PostDynamic;
    pStatusReady->type[NEXUS_FrontendDvbt2StatusType_eL1Plp] = status.l1Plp;
    pStatusReady->type[NEXUS_FrontendDvbt2StatusType_eBasic] = status.shortStatus;

done:
    return rc;

}
static NEXUS_Error NEXUS_Frontend_P_3461_GetDvbt2AsyncStatus(void *handle, NEXUS_FrontendDvbt2StatusType type, NEXUS_FrontendDvbt2Status *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    unsigned i=0;
    NEXUS_3461 *pDevice;
    BDBG_ASSERT(handle != NULL);
    pDevice = (NEXUS_3461 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

    BKNI_Memset(pStatus, 0, sizeof(NEXUS_FrontendDvbt2Status));
    BKNI_Memset(&pDevice->t2Status, 0, sizeof(pDevice->t2Status));

    BDBG_CASSERT(NEXUS_FrontendDvbt2StatusType_eMax == (NEXUS_FrontendDvbt2StatusType)BTC2_SelectiveAsyncStatusType_eLast);
    rc = BTC2_GetSelectiveAsyncStatus(pDevice->tc2_chn[0], type, &pDevice->t2Status);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if(type != (NEXUS_FrontendDvbt2StatusType)pDevice->t2Status.type){
        BDBG_ERR(("Requested status type %d does not match the returned status type %d.",type, (NEXUS_FrontendDvbt2StatusType)pDevice->t2Status.type));
        rc = BERR_TRACE(NEXUS_UNKNOWN); goto done;
    }

    switch ( pDevice->t2Status.type )
    {
    case BTC2_SelectiveAsyncStatusType_eFecStatisticsL1Pre:
    case BTC2_SelectiveAsyncStatusType_eFecStatisticsL1Post:
    case BTC2_SelectiveAsyncStatusType_eFecStatisticsPlpA:
    case BTC2_SelectiveAsyncStatusType_eFecStatisticsPlpB:
        pStatus->status.fecStatistics.lock = pDevice->t2Status.status.fecStatistics.lock;
        pStatus->status.fecStatistics.snrData = pDevice->t2Status.status.fecStatistics.snrData/256;
        pStatus->status.fecStatistics.ldpcAvgIter = pDevice->t2Status.status.fecStatistics.ldpcAvgIter;
        pStatus->status.fecStatistics.ldpcTotIter = pDevice->t2Status.status.fecStatistics.ldpcTotIter;
        pStatus->status.fecStatistics.ldpcTotFrm = pDevice->t2Status.status.fecStatistics.ldpcTotFrm;
        pStatus->status.fecStatistics.ldpcUncFrm = pDevice->t2Status.status.fecStatistics.ldpcUncFrm;
        pStatus->status.fecStatistics.ldpcBER = pDevice->t2Status.status.fecStatistics.ldpcBER;
        pStatus->status.fecStatistics.bchCorBit = pDevice->t2Status.status.fecStatistics.bchCorBit;
        pStatus->status.fecStatistics.bchTotBlk = pDevice->t2Status.status.fecStatistics.bchTotBlk;
        pStatus->status.fecStatistics.bchClnBlk = pDevice->t2Status.status.fecStatistics.bchClnBlk;
        pStatus->status.fecStatistics.bchCorBlk = pDevice->t2Status.status.fecStatistics.bchCorBlk;
        pStatus->status.fecStatistics.bchUncBlk = pDevice->t2Status.status.fecStatistics.bchUncBlk;
        break;
    case BTC2_SelectiveAsyncStatusType_eL1Pre:
        pStatus->status.l1Pre.streamType = pDevice->t2Status.status.l1PreStatus.streamType;
        pStatus->status.l1Pre.bwtExt = pDevice->t2Status.status.l1PreStatus.bwtExt;
        pStatus->status.l1Pre.s1 = pDevice->t2Status.status.l1PreStatus.s1;
        pStatus->status.l1Pre.s2 = pDevice->t2Status.status.l1PreStatus.s2;
        pStatus->status.l1Pre.l1RepetitionFlag = pDevice->t2Status.status.l1PreStatus.l1RepetitionFlag;
        pStatus->status.l1Pre.guardInterval= pDevice->t2Status.status.l1PreStatus.guardInterval;
        pStatus->status.l1Pre.papr = pDevice->t2Status.status.l1PreStatus.papr;
        pStatus->status.l1Pre.l1Mod = pDevice->t2Status.status.l1PreStatus.l1Mod;
        pStatus->status.l1Pre.l1CodeRate = pDevice->t2Status.status.l1PreStatus.l1Cod;
        pStatus->status.l1Pre.l1FecType = pDevice->t2Status.status.l1PreStatus.l1FecType;
        pStatus->status.l1Pre.pilotPattern = pDevice->t2Status.status.l1PreStatus.pilotPattern;
        pStatus->status.l1Pre.regenFlag = pDevice->t2Status.status.l1PreStatus.regenFlag;
        pStatus->status.l1Pre.l1PostExt = pDevice->t2Status.status.l1PreStatus.l1PostExt;
        pStatus->status.l1Pre.numRf = pDevice->t2Status.status.l1PreStatus.numRf;
        pStatus->status.l1Pre.currentRfIndex = pDevice->t2Status.status.l1PreStatus.currentRfIndex;
        pStatus->status.l1Pre.txIdAvailability = pDevice->t2Status.status.l1PreStatus.txIdAvailability;
        pStatus->status.l1Pre.numT2Frames = pDevice->t2Status.status.l1PreStatus.numT2Frames;
        pStatus->status.l1Pre.numDataSymbols = pDevice->t2Status.status.l1PreStatus.numDataSymbols;
        pStatus->status.l1Pre.cellId = pDevice->t2Status.status.l1PreStatus.cellId;
        pStatus->status.l1Pre.networkId = pDevice->t2Status.status.l1PreStatus.networkId;
        pStatus->status.l1Pre.t2SystemId = pDevice->t2Status.status.l1PreStatus.t2SystemId;
        pStatus->status.l1Pre.l1PostSize = pDevice->t2Status.status.l1PreStatus.l1PostSize;
        pStatus->status.l1Pre.l1PostInfoSize = pDevice->t2Status.status.l1PreStatus.l1PostInfoSize;
        break;
    case BTC2_SelectiveAsyncStatusType_eL1PostConfigurable:
        pStatus->status.l1PostConfigurable.subSlicesPerFrame = pDevice->t2Status.status.l1PostConfigurableStatus.subSlicesPerFrame;
        pStatus->status.l1PostConfigurable.numPlp = pDevice->t2Status.status.l1PostConfigurableStatus.numPlp;
        pStatus->status.l1PostConfigurable.numAux = pDevice->t2Status.status.l1PostConfigurableStatus.numAux;
        pStatus->status.l1PostConfigurable.fefType = pDevice->t2Status.status.l1PostConfigurableStatus.fefType;
        pStatus->status.l1PostConfigurable.rfIdx = pDevice->t2Status.status.l1PostConfigurableStatus.rfIdx;
        pStatus->status.l1PostConfigurable.fefInterval = pDevice->t2Status.status.l1PostConfigurableStatus.fefInterval;
        pStatus->status.l1PostConfigurable.frequency = pDevice->t2Status.status.l1PostConfigurableStatus.frequency;
        pStatus->status.l1PostConfigurable.fefLength = pDevice->t2Status.status.l1PostConfigurableStatus.fefLength;
        pStatus->status.l1PostConfigurable.auxStreamType = pDevice->t2Status.status.l1PostConfigurableStatus.auxStreamType;
        pStatus->status.l1PostConfigurable.auxPrivateConf = pDevice->t2Status.status.l1PostConfigurableStatus.auxPrivateConf;

        pStatus->status.l1PostConfigurable.plpA.plpId = pDevice->t2Status.status.l1PostConfigurableStatus.plpA.plpId;
        pStatus->status.l1PostConfigurable.plpA.plpGroupId = pDevice->t2Status.status.l1PostConfigurableStatus.plpA.plpGroupId;
        pStatus->status.l1PostConfigurable.plpA.plpType = pDevice->t2Status.status.l1PostConfigurableStatus.plpA.plpType;
        pStatus->status.l1PostConfigurable.plpA.plpPayloadType = pDevice->t2Status.status.l1PostConfigurableStatus.plpA.plpPayloadType;
        pStatus->status.l1PostConfigurable.plpA.ffFlag = pDevice->t2Status.status.l1PostConfigurableStatus.plpA.ffFlag;
        pStatus->status.l1PostConfigurable.plpA.firstRfIdx = pDevice->t2Status.status.l1PostConfigurableStatus.plpA.firstRfIdx;
        pStatus->status.l1PostConfigurable.plpA.firstFrameIdx = pDevice->t2Status.status.l1PostConfigurableStatus.plpA.firstFrameIdx;
        pStatus->status.l1PostConfigurable.plpA.plpCodeRate = pDevice->t2Status.status.l1PostConfigurableStatus.plpA.plpCodeRate;
        pStatus->status.l1PostConfigurable.plpA.plpMod = pDevice->t2Status.status.l1PostConfigurableStatus.plpA.plpMod;
        pStatus->status.l1PostConfigurable.plpA.plpRotation = pDevice->t2Status.status.l1PostConfigurableStatus.plpA.plpRotation;
        pStatus->status.l1PostConfigurable.plpA.plpFecType = pDevice->t2Status.status.l1PostConfigurableStatus.plpA.plpFecType;
        pStatus->status.l1PostConfigurable.plpA.plpNumBlocksMax = pDevice->t2Status.status.l1PostConfigurableStatus.plpA.plpNumBlocksMax;
        pStatus->status.l1PostConfigurable.plpA.frameInterval = pDevice->t2Status.status.l1PostConfigurableStatus.plpA.frameInterval;
        pStatus->status.l1PostConfigurable.plpA.timeIlLength = pDevice->t2Status.status.l1PostConfigurableStatus.plpA.timeIlLength;
        pStatus->status.l1PostConfigurable.plpA.timeIlType= pDevice->t2Status.status.l1PostConfigurableStatus.plpA.timeIlType;
        pStatus->status.l1PostConfigurable.plpA.inBandFlag = pDevice->t2Status.status.l1PostConfigurableStatus.plpA.inBandFlag;

        pStatus->status.l1PostConfigurable.plpB.plpId = pDevice->t2Status.status.l1PostConfigurableStatus.plpB.plpId;
        pStatus->status.l1PostConfigurable.plpB.plpGroupId = pDevice->t2Status.status.l1PostConfigurableStatus.plpB.plpGroupId;
        pStatus->status.l1PostConfigurable.plpB.plpType = pDevice->t2Status.status.l1PostConfigurableStatus.plpB.plpType;
        pStatus->status.l1PostConfigurable.plpB.plpPayloadType = pDevice->t2Status.status.l1PostConfigurableStatus.plpB.plpPayloadType;
        pStatus->status.l1PostConfigurable.plpB.ffFlag = pDevice->t2Status.status.l1PostConfigurableStatus.plpB.ffFlag;
        pStatus->status.l1PostConfigurable.plpB.firstRfIdx = pDevice->t2Status.status.l1PostConfigurableStatus.plpB.firstRfIdx;
        pStatus->status.l1PostConfigurable.plpB.firstFrameIdx = pDevice->t2Status.status.l1PostConfigurableStatus.plpB.firstFrameIdx;
        pStatus->status.l1PostConfigurable.plpB.plpCodeRate = pDevice->t2Status.status.l1PostConfigurableStatus.plpB.plpCodeRate;
        pStatus->status.l1PostConfigurable.plpB.plpMod = pDevice->t2Status.status.l1PostConfigurableStatus.plpB.plpMod;
        pStatus->status.l1PostConfigurable.plpB.plpRotation = pDevice->t2Status.status.l1PostConfigurableStatus.plpB.plpRotation;
        pStatus->status.l1PostConfigurable.plpB.plpFecType = pDevice->t2Status.status.l1PostConfigurableStatus.plpB.plpFecType;
        pStatus->status.l1PostConfigurable.plpB.plpNumBlocksMax = pDevice->t2Status.status.l1PostConfigurableStatus.plpB.plpNumBlocksMax;
        pStatus->status.l1PostConfigurable.plpB.frameInterval = pDevice->t2Status.status.l1PostConfigurableStatus.plpB.frameInterval;
        pStatus->status.l1PostConfigurable.plpB.timeIlLength = pDevice->t2Status.status.l1PostConfigurableStatus.plpB.timeIlLength;
        pStatus->status.l1PostConfigurable.plpB.timeIlType= pDevice->t2Status.status.l1PostConfigurableStatus.plpB.timeIlType;
        pStatus->status.l1PostConfigurable.plpB.inBandFlag = pDevice->t2Status.status.l1PostConfigurableStatus.plpB.inBandFlag;
        break;
    case BTC2_SelectiveAsyncStatusType_eL1PostDynamic:
        pStatus->status.l1PostDynamic.frameIdx = pDevice->t2Status.status.l1PostDynamicStatus.frameIdx;
        pStatus->status.l1PostDynamic.l1ChanlgeCounter = pDevice->t2Status.status.l1PostDynamicStatus.l1ChanlgeCounter;
        pStatus->status.l1PostDynamic.startRfIdx = pDevice->t2Status.status.l1PostDynamicStatus.startRfIdx;
        pStatus->status.l1PostDynamic.subSliceInterval = pDevice->t2Status.status.l1PostDynamicStatus.subSliceInterval;
        pStatus->status.l1PostDynamic.type2Start = pDevice->t2Status.status.l1PostDynamicStatus.type2Start;
        pStatus->status.l1PostDynamic.auxPrivateDyn_31_0 = pDevice->t2Status.status.l1PostDynamicStatus.auxPrivateDyn_31_0;
        pStatus->status.l1PostDynamic.auxPrivateDyn_47_32 = pDevice->t2Status.status.l1PostDynamicStatus.auxPrivateDyn_47_32;
        pStatus->status.l1PostDynamic.plpA.plpId = pDevice->t2Status.status.l1PostDynamicStatus.plpA.plpId;
        pStatus->status.l1PostDynamic.plpA.plpNumBlocks = pDevice->t2Status.status.l1PostDynamicStatus.plpA.plpNumBlocks;
        pStatus->status.l1PostDynamic.plpA.plpStart = pDevice->t2Status.status.l1PostDynamicStatus.plpA.plpNumBlocks;
        pStatus->status.l1PostDynamic.plpB.plpId = pDevice->t2Status.status.l1PostDynamicStatus.plpB.plpId;
        pStatus->status.l1PostDynamic.plpB.plpNumBlocks = pDevice->t2Status.status.l1PostDynamicStatus.plpB.plpNumBlocks;
        pStatus->status.l1PostDynamic.plpB.plpStart = pDevice->t2Status.status.l1PostDynamicStatus.plpB.plpNumBlocks;
        break;
    case BTC2_SelectiveAsyncStatusType_eL1Plp:
        pStatus->status.l1Plp.numPlp = pDevice->t2Status.status.l1PlpStatus.numPlp;
        for(i=0; i<pStatus->status.l1Plp.numPlp; i++) {
            pStatus->status.l1Plp.plp[i].plpId = pDevice->t2Status.status.l1PlpStatus.plp[i].plpId;
            pStatus->status.l1Plp.plp[i].plpGroupId = pDevice->t2Status.status.l1PlpStatus.plp[i].plpGroupId;
            pStatus->status.l1Plp.plp[i].plpPayloadType = pDevice->t2Status.status.l1PlpStatus.plp[i].plpPayloadType;
            pStatus->status.l1Plp.plp[i].plpType = pDevice->t2Status.status.l1PlpStatus.plp[i].plpType;
        }
        break;
    case BTC2_SelectiveAsyncStatusType_eShort:
        pStatus->status.basic.fecLock = pDevice->t2Status.status.shortStatus.lock;
        pStatus->status.basic.spectrumInverted = pDevice->t2Status.status.shortStatus.isSpectrumInverted;
        pStatus->status.basic.snr = pDevice->t2Status.status.shortStatus.snrEstimate*100/256;
        pStatus->status.basic.gainOffset = pDevice->t2Status.status.shortStatus.gainOffset*100/256;
        pStatus->status.basic.carrierOffset = pDevice->t2Status.status.shortStatus.carrierFreqOffset;
        pStatus->status.basic.timingOffset = pDevice->t2Status.status.shortStatus.timingOffset;
        pStatus->status.basic.signalStrength = pDevice->t2Status.status.shortStatus.signalStrength/10;
        pStatus->status.basic.signalLevelPercent = pDevice->t2Status.status.shortStatus.signalLevelPercent;
        pStatus->status.basic.signalQualityPercent = pDevice->t2Status.status.shortStatus.signalQualityPercent;
        pStatus->status.basic.reacquireCount = pDevice->t2Status.status.shortStatus.reacqCount;
        break;
    default:
        BDBG_ERR((" Unsupported status type."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    pStatus->type = (NEXUS_FrontendDvbt2StatusType)pDevice->t2Status.type;

    NEXUS_Frontend_P_PrintDvbt2PartialStatus(pStatus);
done:
    return rc;
}

static void NEXUS_Frontend_P_PrintDvbt2PartialStatus(NEXUS_FrontendDvbt2Status *pStatus)
{
    unsigned i=0;

    switch ( pStatus->type )
    {
    case BTC2_SelectiveAsyncStatusType_eFecStatisticsL1Pre:
    case BTC2_SelectiveAsyncStatusType_eFecStatisticsL1Post:
    case BTC2_SelectiveAsyncStatusType_eFecStatisticsPlpA:
    case BTC2_SelectiveAsyncStatusType_eFecStatisticsPlpB:
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
    case BTC2_SelectiveAsyncStatusType_eL1Pre:
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
    case BTC2_SelectiveAsyncStatusType_eL1PostConfigurable:
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
    case BTC2_SelectiveAsyncStatusType_eL1PostDynamic:
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
    case BTC2_SelectiveAsyncStatusType_eL1Plp:
        BDBG_MSG(("pStatus->status.l1Plp.numPlp = %d",pStatus->status.l1Plp.numPlp));

        for(i=0; i<pStatus->status.l1Plp.numPlp; i++) {
            BDBG_MSG(("pStatus->status.l1Plp.plp[%d].plpId = %d", i,pStatus->status.l1Plp.plp[i].plpId));
            BDBG_MSG(("pStatus->status.l1Plp.plp[%d].plpGroupId = %d",i,pStatus->status.l1Plp.plp[i].plpGroupId));
            BDBG_MSG(("pStatus->status.l1Plp.plp[%d].plpPayloadType = %d",i,pStatus->status.l1Plp.plp[i].plpPayloadType));
            BDBG_MSG(("pStatus->status.l1Plp.plp[%d].plpType = %d",i,pStatus->status.l1Plp.plp[i].plpType));
        }
        break;
    case BTC2_SelectiveAsyncStatusType_eShort:
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
        break;
    default:
        BDBG_MSG((" Unsupported status type."));
    }
    BDBG_MSG(("pStatus->type = %d",pStatus->type));

}

static void NEXUS_Frontend_P_3461_ResetStatus(void *handle)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3461 *pDevice = (NEXUS_3461 *)handle;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

    if(pDevice->isPoweredOn[pDevice->lastChannel]){
        switch ( pDevice->lastChannel )
        {
        case NEXUS_3461_TC2_CHN:
            rc = BTC2_ResetStatus(pDevice->tc2_chn[0]);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            break;
        case NEXUS_3461_THD_CHN:
            rc = BTHD_ResetInbandStatus(pDevice->thd);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            break;
        default:
            BDBG_ERR((" Unsupported channel."));
            BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
done:
    return;
}

static NEXUS_Error NEXUS_Frontend_P_3461_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3461 *pDevice = (NEXUS_3461 *)handle;
    BTNR_PowerSaverSettings pwrSettings;

    BSTD_UNUSED(enabled);
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

    if(pSettings->mode < NEXUS_StandbyMode_ePassive){
        if(!pDevice->isTunerPoweredOn){
            pwrSettings.enable = false;
            rc = BTNR_SetPowerSaver(pDevice->tnr[0], &pwrSettings);
            if(rc){rc = BERR_TRACE(rc);}
            pDevice->isTunerPoweredOn = true;
        }

        switch ( pDevice->lastChannel )
        {
        case NEXUS_3461_THD_CHN:
            rc = BTHD_InstallCallback(pDevice->thd, BTHD_Callback_eLockChange, (BTHD_CallbackFunc)NEXUS_Frontend_P_3461_callback_isr, (void*)pDevice);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            rc = BTHD_InstallCallback(pDevice->thd, BTHD_Callback_eAsyncStatusReady, (BTHD_CallbackFunc)NEXUS_Frontend_P_3461_AsyncStatusCallback_isr, (void*)pDevice);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            rc = BTHD_PowerUp(pDevice->thd);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->isPoweredOn[pDevice->lastChannel] = true;
            break;
        case NEXUS_3461_TC2_CHN:

            rc = BTC2_InstallCallback(pDevice->tc2_chn[0], BTC2_Callback_eAsyncStatusReady, (BTC2_CallbackFunc)NEXUS_Frontend_P_3461_AsyncStatusCallback_isr, (void*)pDevice);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            rc = BTC2_InstallCallback(pDevice->tc2_chn[0], BTC2_Callback_eLockChange, (BTC2_CallbackFunc)NEXUS_Frontend_P_3461_callback_isr, (void*)pDevice);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            rc = BTC2_DisablePowerSaver(pDevice->tc2_chn[0]);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->isPoweredOn[pDevice->lastChannel] = true;
            break;
        default:
            BDBG_ERR((" Unsupported channel."));
            rc = BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else {
        switch ( pDevice->lastChannel )
        {
        case NEXUS_3461_THD_CHN:
            rc = BTHD_InstallCallback(pDevice->thd, BTHD_Callback_eLockChange, NULL, NULL);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            rc = BTHD_InstallCallback(pDevice->thd, BTHD_Callback_eAsyncStatusReady, NULL, NULL);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            rc = BTHD_PowerDown(pDevice->thd);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->isPoweredOn[pDevice->lastChannel] = false;
            break;
        case NEXUS_3461_TC2_CHN:

            rc = BTC2_InstallCallback(pDevice->tc2_chn[0], BTC2_Callback_eLockChange, NULL, NULL);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            rc = BTC2_InstallCallback(pDevice->tc2_chn[0], BTC2_Callback_eAsyncStatusReady, NULL, NULL);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            rc = BTC2_EnablePowerSaver(pDevice->tc2_chn[0]);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->isPoweredOn[pDevice->lastChannel] = false;
            break;
        default:
            BDBG_ERR((" Unsupported channel."));
            rc = BERR_TRACE(BERR_NOT_SUPPORTED);
        }

        if(pDevice->isTunerPoweredOn){
            pwrSettings.enable = true;
            rc = BTNR_SetPowerSaver(pDevice->tnr[0], &pwrSettings);
            if(rc){rc = BERR_TRACE(rc);}
            pDevice->isTunerPoweredOn = false;
        }
    }
done:
    return rc;
}

static void NEXUS_Frontend_P_3461_UninstallCallbacks(void *handle)
{
    NEXUS_3461 *pDevice = (NEXUS_3461 *)handle;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

    if(pDevice->lockAppCallback[pDevice->lastChannel])NEXUS_IsrCallback_Set(pDevice->lockAppCallback[pDevice->lastChannel], NULL);
    if(pDevice->asyncStatusAppCallback[pDevice->lastChannel])NEXUS_IsrCallback_Set(pDevice->asyncStatusAppCallback[pDevice->lastChannel], NULL);
    return;
}

static void NEXUS_Frontend_P_3461_UnTune(void *handle)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3461 *pDevice = (NEXUS_3461 *)handle;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

    switch ( pDevice->lastChannel )
    {
    case NEXUS_3461_THD_CHN:
        rc = BTHD_PowerDown(pDevice->thd);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->isPoweredOn[pDevice->lastChannel] = false;
        break;
    case NEXUS_3461_TC2_CHN:
        rc = BTC2_EnablePowerSaver(pDevice->tc2_chn[0]);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->isPoweredOn[pDevice->lastChannel] = false;
        break;
    default:
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED);
    }
done:
    return;
}

/***************************************************************************
Summary:
    Initialize Hab for a 3461 device
***************************************************************************/
void NEXUS_FrontendDevice_P_3461_S3Standby(NEXUS_3461 *pDevice)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    if(pDevice->deviceOpenThread)NEXUS_Thread_Destroy(pDevice->deviceOpenThread);
    pDevice->deviceOpenThread = NULL;

    NEXUS_Frontend_P_UnInit3461(pDevice);

    if(pDevice->isrEventCallback)NEXUS_UnregisterEvent(pDevice->isrEventCallback);
    pDevice->isrEventCallback = NULL;
    if(pDevice->isrEvent) pDevice->isrEvent = NULL;

    if(pDevice->hab) {
        rc = BHAB_Close(pDevice->hab);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    pDevice->hab = NULL;
done:
    return;
}

static NEXUS_Error NEXUS_FrontendDevice_P_3461_Standby(void *handle, const NEXUS_StandbySettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3461 *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_3461 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

    if((pDevice->pGenericDeviceHandle->mode != NEXUS_StandbyMode_eDeepSleep) && (pSettings->mode == NEXUS_StandbyMode_eDeepSleep)){

        NEXUS_FrontendDevice_P_3461_S3Standby(pDevice);

        BKNI_EnterCriticalSection();
        pDevice->pGenericDeviceHandle->openPending = true;
        pDevice->pGenericDeviceHandle->openFailed = false;
        BKNI_LeaveCriticalSection();
    }
    else if((pDevice->pGenericDeviceHandle->mode == NEXUS_StandbyMode_eDeepSleep) && (pSettings->mode != NEXUS_StandbyMode_eDeepSleep)){

        rc = NEXUS_FrontendDevice_P_Init3461(pDevice, &pDevice->openSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_3461_ReadSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length, size_t *pNumRead)
{
    size_t i;
    NEXUS_Error rc = NEXUS_SUCCESS;
    int16_t return_length;
    NEXUS_3461 *pDevice = (NEXUS_3461 *)handle;
    int16_t d_i[TOTAL_SOFTDECISIONS], d_q[TOTAL_SOFTDECISIONS];

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

    BKNI_Memset(pDecisions, 0, (sizeof(NEXUS_FrontendSoftDecision) * length));

    /* only make one call to ADS. if app needs more, they can loop. */
    switch ( pDevice->lastChannel )
    {
    case NEXUS_3461_THD_CHN:
            rc = BTHD_GetSoftDecisionBuf(pDevice->thd, d_i, d_q);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            return_length = TOTAL_SOFTDECISIONS;
        break;
    case NEXUS_3461_TC2_CHN:
            rc = BTC2_GetSoftDecision(pDevice->tc2_chn[0], (int16_t)TOTAL_SOFTDECISIONS,  d_i, d_q, &return_length);
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

void NEXUS_FrontendDevice_GetDefault3461Settings(NEXUS_FrontendDevice3461Settings *pSettings)
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->agcValue = 0x1f; /* Max gain*/
    pSettings->terrestrial = true;
}

NEXUS_Error NEXUS_FrontendDevice_Get3461Settings(NEXUS_FrontendDeviceHandle handle, NEXUS_FrontendDevice3461Settings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3461 *pDevice = NULL;
    BDBG_ASSERT(handle != NULL);

    pDevice = handle->pDevice;
    if (!pDevice) { rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto done; }
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

    pSettings->updateGainCallback = pDevice->updateGainCallbackDesc;
    pSettings->agcValue = pDevice->agcValue;
    pSettings->rfInput = pDevice->rfInput;
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
    NEXUS_3461 *pDevice = NULL;
    BHAB_ConfigSettings habConfigSettings;

    pDevice = handle->pDevice;
    if (!pDevice) { rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto done; }
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

    if(pDevice->isInternalSetSettings  || (!(pDevice->pGenericDeviceHandle->openPending) && !(pDevice->pGenericDeviceHandle->openFailed))){
        rc = BHAB_GetConfigSettings(pDevice->hab, &habConfigSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        habConfigSettings.daisyChain = pSettings->rfDaisyChain;
        habConfigSettings.enableLoopThrough = pSettings->enableRfLoopThrough;
        habConfigSettings.rfInputMode = pSettings->rfInput;
        habConfigSettings.tnrApplication = pSettings->terrestrial;

        rc = BHAB_SetConfigSettings(pDevice->hab, &habConfigSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->isInternalSetSettings = false;
        pDevice->rfDaisyChain = pSettings->rfDaisyChain;
        pDevice->enableRfLoopThrough = pSettings->enableRfLoopThrough;
        pDevice->rfInput = pSettings->rfInput;
        pDevice->terrestrial = pSettings->terrestrial;

        if(pDevice->terrestrial == true)
            pDevice->pGenericDeviceHandle->application = NEXUS_FrontendDeviceApplication_eTerrestrial;
        else if(pDevice->terrestrial == false)
            pDevice->pGenericDeviceHandle->application = NEXUS_FrontendDeviceApplication_eCable;
    }
    else if(pDevice->pGenericDeviceHandle->openPending && !(pDevice->pGenericDeviceHandle->openFailed)){
        pDevice->rfDaisyChain = pSettings->rfDaisyChain;
        pDevice->enableRfLoopThrough = pSettings->enableRfLoopThrough;
        pDevice->rfInput = pSettings->rfInput;
        pDevice->terrestrial = pSettings->terrestrial;

        if(pDevice->terrestrial == true)
            pDevice->pGenericDeviceHandle->application = NEXUS_FrontendDeviceApplication_eTerrestrial;
        else if(pDevice->terrestrial == false)
            pDevice->pGenericDeviceHandle->application = NEXUS_FrontendDeviceApplication_eCable;
    }
    else{
        BDBG_ERR(("Device open failed."));
        rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    }
done:
    return rc;
}

/***************************************************************************
Summary:
  Probe to see if a BCM3461 device exists with the specified settings

Description:
  Probe to see if a BCM3461 device exists with the specified settings

See Also:
    NEXUS_Frontend_Open3461
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Probe3461(const NEXUS_FrontendDevice3461OpenSettings *pSettings, NEXUS_3461ProbeResults *pResults)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    uint16_t chipVer=0xffff;

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(!((NULL == pSettings->i2cDevice) && (NULL == pSettings->spiDevice)));
    BDBG_ASSERT(NULL != pResults);

    pResults->chip.familyId = 0x0;

    pResults->chip.familyId = (uint32_t)NEXUS_Frontend_P_Is3461(pSettings);
    if ( pResults->chip.familyId != 0x3461 )
    {
        BDBG_WRN(("pResults->chip.familyId = 0x%x", pResults->chip.familyId));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }
    pResults->chip.id = pResults->chip.familyId;
    chipVer = NEXUS_Frontend_P_Get3461Rev(pSettings);
done:
    pResults->chip.version.major = (chipVer >> 8) + 1;
    pResults->chip.version.minor = chipVer & 0xff;

    return rc;
}

/***************************************************************************
Summary:
    Retrieve the chip family id, chip id, chip version and firmware version.
***************************************************************************/
static void NEXUS_Frontend_P_3461_GetType(void *handle, NEXUS_FrontendType *type)
{
    NEXUS_3461 *pDevice = (NEXUS_3461 *)handle;
    BFEC_SystemVersionInfo  versionInfo;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);

    BHAB_GetVersionInfo(pDevice->hab, &versionInfo);

    type->chip.familyId = (uint32_t)pDevice->chipId;
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

static NEXUS_Error NEXUS_Frontend_P_3461_setCrystalDaisySettings(const NEXUS_FrontendDevice3461OpenSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    uint32_t sb1, addr;
    uint8_t data=0, i=0,  buf[4];
    BREG_I2C_Handle i2cRegHandle;
    BREG_SPI_Handle spiHandle;
    uint8_t readbuf[8], writebuf[16];

    addr = BCHP_TM_PWRDN;
    sb1 = ((addr & 0x000000FF) << 24 |
           (addr & 0x0000FF00) << 8 |
           (addr & 0x00FF0000) >> 8 |
           (addr & 0xFF000000) >> 24 );

    if(pSettings->i2cDevice){
        i2cRegHandle = NEXUS_I2c_GetRegHandle(pSettings->i2cDevice, NULL);

       /* set READ_RBUS for read mode  and no increment mode. */
       data = 0x5;
       rc = BREG_I2C_Write(i2cRegHandle, pSettings->i2cAddr, CSR_CONFIG, &data, 1);
       if(rc){rc = BERR_TRACE(rc); goto done;}
       rc = BREG_I2C_Write(i2cRegHandle, pSettings->i2cAddr, CSR_RBUS_ADDR0, (uint8_t *)&sb1, 4);
       if(rc){rc = BERR_TRACE(rc); goto done;}

        /* poll the busy bit to make sure the transaction is completed */
       for(i=0; i < 5; i++){
            rc = BREG_I2C_Read(i2cRegHandle, pSettings->i2cAddr, CSR_STATUS, &data, 1);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            if ((data & (1 << CSR_STATUS_BUSY_SHIFT)) == 0)
                break;
       }

       if(i==5)
           BDBG_WRN(("Read transaction not finished"));

        /* read the data */
        rc = BREG_I2C_Read(i2cRegHandle, pSettings->i2cAddr, CSR_RBUS_DATA0, buf, 4);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        /* set READ_RBUS to the reset value for write mode */
        data = 0x4;
        rc = BREG_I2C_Write(i2cRegHandle, pSettings->i2cAddr, CSR_CONFIG, &data, 1);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        if(!pSettings->crystalSettings.enableDaisyChain){
            buf[3] |= 0x2;
            rc = BREG_I2C_Write(i2cRegHandle, pSettings->i2cAddr, CSR_RBUS_DATA0, buf, 4);
            if(rc){rc = BERR_TRACE(rc); goto done;}
        }
        else{
            buf[3] &= ~0x2;
            rc = BREG_I2C_Write(i2cRegHandle, pSettings->i2cAddr, CSR_RBUS_DATA0, buf, 4);
            if(rc){rc = BERR_TRACE(rc); goto done;}
        }

        data = CSR_CONFIG_READ_RBUS_WRITE;
        rc = BREG_I2C_Write(i2cRegHandle, pSettings->i2cAddr, CSR_CONFIG, &data, 1);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    else if(pSettings->spiDevice){
        spiHandle = NEXUS_Spi_GetRegHandle(pSettings->spiDevice);

        /* set CSR pointer to point to ADDR0 and set RBUS address to zero */
        writebuf[0] = (pSettings->spiAddr << 1) | 0x1;
        writebuf[1] = CSR_CONFIG;
        writebuf[2] = 5;
        rc = BREG_SPI_Write(spiHandle,  writebuf, 3);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        writebuf[1] = CSR_RBUS_ADDR0;
        writebuf[2] = sb1;
        writebuf[3] = (sb1 >> 8);
        writebuf[4] = (sb1 >> 16);
        writebuf[5] = (sb1 >> 24);
        rc = BREG_SPI_Write(spiHandle,  writebuf, 6);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        /* check for host transfer error */
        writebuf[0] = (pSettings->spiAddr << 1);
        writebuf[1] = CSR_STATUS;
        for(i=0; i < 5; i++){
            rc = BREG_SPI_Read(spiHandle,  writebuf, readbuf, 3);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            if ((readbuf[2] & CSR_STATUS_ERROR_BITS) == 0)
                break;
        }
        if(i==5)
            BDBG_WRN(("Write transaction not finished"));

        writebuf[1] = CSR_RBUS_DATA0;
        rc = BREG_SPI_Read(spiHandle,  writebuf, readbuf, 6);
        if(rc){rc = BERR_TRACE(rc); goto done;}


        /* set CSR pointer to point to ADDR0 and set RBUS address to zero */
        writebuf[0] = (pSettings->spiAddr << 1) | 0x1;
        writebuf[1] = CSR_CONFIG;
        writebuf[2] = 4;
        rc = BREG_SPI_Write(spiHandle,  writebuf, 3);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        writebuf[1] = CSR_RBUS_DATA0;
        writebuf[2] = readbuf[2];
        writebuf[3] = readbuf[3];
        writebuf[4] = readbuf[4];
        writebuf[5] = readbuf[5];

        if(!pSettings->crystalSettings.enableDaisyChain){
            writebuf[5] |= 0x2;
            rc = BREG_SPI_Write(spiHandle,  writebuf, 6);
            if(rc){rc = BERR_TRACE(rc); goto done;}
        }
        else{
            writebuf[5] &= ~0x2;
            rc = BREG_SPI_Write(spiHandle,  writebuf, 6);
            if(rc){rc = BERR_TRACE(rc); goto done;}
        }

        writebuf[1] = CSR_CONFIG;
        writebuf[2] = CSR_CONFIG_READ_RBUS_WRITE;
        rc = BREG_SPI_Write(spiHandle,  writebuf, 3);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
done:
    return rc;
}

static NEXUS_Error NEXUS_FrontendDevice_P_3461_GetStatus(void *handle, NEXUS_FrontendDeviceStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3461 *pDevice;
    BHAB_AvsData avsData;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_3461 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3461);
    BDBG_ASSERT(NULL != handle);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    pStatus->openPending = pDevice->pGenericDeviceHandle->openPending;
    pStatus->openFailed = pDevice->pGenericDeviceHandle->openFailed;

    if(!(pDevice->pGenericDeviceHandle->openPending) && !(pDevice->pGenericDeviceHandle->openFailed)){
        rc = BHAB_GetAvsData(pDevice->hab, &avsData);
        if(rc){rc = BERR_TRACE(rc); goto done;}


        if(avsData.enabled){
            pStatus->avs.enabled = pDevice ? true : false;
            pStatus->avs.voltage = avsData.voltage;
            pStatus->temperature = avsData.temperature;
        }
        else
            pStatus->avs.enabled = false;
    }

    BDBG_MSG(("pStatus->avs.enabled = %d", pStatus->avs.enabled));
    BDBG_MSG(("pStatus->avs.voltage = %d", pStatus->avs.voltage));
    BDBG_MSG(("pStatus->temperature = %d", pStatus->temperature));
done:
    return rc;
}


NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_P_Open3461(unsigned index, const NEXUS_FrontendDeviceOpenSettings *pSettings)
{
    NEXUS_FrontendDevice3461OpenSettings settings;

    BDBG_ASSERT(pSettings);

    if (pSettings->terrestrial.enabled) {
        NEXUS_FrontendDevice_GetDefault3461OpenSettings(&settings);

        settings.loadAP = pSettings->loadAP;
        settings.i2cDevice = pSettings->i2cDevice;
        settings.i2cAddr = pSettings->i2cAddress;
        settings.i2cSlaveAddr = pSettings->i2cSlaveAddress;
        settings.gpioInterrupt = pSettings->gpioInterrupt;
        settings.isrNumber = pSettings->isrNumber;
        settings.spiDevice = pSettings->spiDevice;
        settings.spiAddr = 0x40;
        settings.configureWatchdog = pSettings->configureWatchdog;
        settings.isMtsif = true;
        settings.inBandOpenDrain = true;
        settings.crystalSettings.enableDaisyChain = pSettings->crystalSettings.enableDaisyChain;
        settings.externalFixedGain.bypassable = pSettings->externalFixedGain.bypassable;
        settings.externalFixedGain.total = pSettings->externalFixedGain.total;

        return NEXUS_FrontendDevice_Open3461(index, &settings);
    }
    return NULL;
}

NEXUS_FrontendHandle NEXUS_Frontend_P_Open3461(const NEXUS_FrontendChannelSettings *pSettings)
{
    NEXUS_Frontend3461Settings settings;

    BDBG_ASSERT(pSettings);

    if (pSettings->type == NEXUS_FrontendChannelType_eTerrestrial) {
        NEXUS_Frontend_GetDefault3461Settings(&settings);
        settings.device = pSettings->device;
        settings.channelNumber = pSettings->channelNumber;
        settings.type = NEXUS_3461ChannelType_eDvbt2;
        return NEXUS_Frontend_Open3461(&settings);
    }
    return NULL;
}

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
#include "priv/nexus_transport_priv.h"
#include "nexus_spi.h"
#include "priv/nexus_spi_priv.h"
#include "nexus_i2c.h"
#include "priv/nexus_i2c_priv.h"
#include "priv/nexus_gpio_priv.h"
#include "btnr.h"
#include "btnr_3472ib.h"
#include "bhab_3472.h"
#include "bhab_34xx_priv.h"
#include "bods_3472.h"
#include "bhab_3472_fw.h"
#include "bhab.h"
#include "bods.h"
#include "bchp_hsi.h"
#include "../../a0/bchp_tm.h"

#if NEXUS_HAS_MXT
#include "bmxt.h"
#endif

BDBG_MODULE(nexus_frontend_3472);

BDBG_OBJECT_ID(NEXUS_3472);

#define BCHIP_3472 0x3472

#define NEXUS_MAX_3472_FRONTENDS 2

#define NEXUS_3472_MAX_OFDM_CHN  NEXUS_MAX_3472_FRONTENDS

#define NEXUS_MAX_3472_TUNERS  2

#define TOTAL_SOFTDECISIONS 30

/***************************************************************************
 * Internal Module Structure
 ***************************************************************************/
typedef struct NEXUS_3472
{
    BDBG_OBJECT(NEXUS_3472)
    BLST_S_ENTRY(NEXUS_3472) node;
    uint16_t  chipId;
    uint16_t revId;
    BHAB_Handle hab;
    BHAB_Capabilities capabilities;
    unsigned    numfrontends;
    BODS_Handle ods;
    BKNI_EventHandle isrEvent;
    NEXUS_EventCallbackHandle isrEventCallback;
    NEXUS_CallbackDesc updateGainCallbackDesc;    /* Callback will be called when the gain from the lna needs to be updated. */
    BODS_SelectiveStatus odsStatus;
    NEXUS_FrontendDevice *pGenericDeviceHandle;
    unsigned                   count[NEXUS_MAX_3472_FRONTENDS];
    bool                       acquireInProgress[NEXUS_MAX_3472_FRONTENDS];
    NEXUS_FrontendHandle       frontendHandle[NEXUS_MAX_3472_FRONTENDS];
    unsigned                   numChannels[BODS_Standard_eLast];
    BTNR_Handle                tnr[NEXUS_MAX_3472_TUNERS];
    BODS_ChannelHandle         ods_chn[NEXUS_3472_MAX_OFDM_CHN];
    NEXUS_FrontendOfdmSettings last_ofdm[NEXUS_MAX_3472_TUNERS];
    NEXUS_IsrCallbackHandle    lockAppCallback[NEXUS_MAX_3472_FRONTENDS];
    NEXUS_IsrCallbackHandle    asyncStatusAppCallback[NEXUS_MAX_3472_FRONTENDS];
    bool                       isInternalAsyncStatusCall[NEXUS_MAX_3472_FRONTENDS];
    bool                       isStatusReady[NEXUS_MAX_3472_FRONTENDS];
    bool                       isTunerPoweredOn[NEXUS_MAX_3472_FRONTENDS];
    bool                       isPoweredOn[NEXUS_MAX_3472_FRONTENDS];
    NEXUS_FrontendOfdmAcquisitionMode lastAcquisitionMode[NEXUS_MAX_3472_FRONTENDS];
#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT
    bool isMaster;
#endif
#if NEXUS_HAS_MXT
    NEXUS_TsmfSettings tsmfSettings[NEXUS_MAX_3472_FRONTENDS];
#endif

    NEXUS_ThreadHandle deviceOpenThread;
    NEXUS_FrontendDevice3461OpenSettings openSettings;
    NEXUS_FrontendDevice3461Settings deviceSettings;
    bool isInternalSetSettings;
    BREG_I2C_Handle i2cRegHandle;
    BREG_SPI_Handle spiRegHandle;
    bool settingsInitialized;
} NEXUS_3472;

static BLST_S_HEAD(devList, NEXUS_3472) g_3472DevList = BLST_S_INITIALIZER(g_3472DevList);

typedef struct NEXUS_3472Channel
{
    unsigned chn_num; /* channel number */
    NEXUS_3472 *pDevice; /* 3472 device*/
} NEXUS_3472Channel;

/***************************************************************************
 * Module callback functions for tuning
 ***************************************************************************/
static void NEXUS_Frontend_P_3472_UninstallCallbacks(void *handle);
static NEXUS_Error NEXUS_Frontend_P_Init3472(NEXUS_3472 *pDevice);
static void NEXUS_Frontend_P_3472_UnTune(void *handle);
static void NEXUS_Frontend_P_3472_ResetStatus(void *handle);
static NEXUS_Error NEXUS_Frontend_P_3472_ReadSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length, size_t *pNumRead);
static uint16_t NEXUS_Frontend_P_Get3472Rev(const NEXUS_FrontendDevice3461OpenSettings *pSettings);
static void NEXUS_Frontend_P_3472_Close(NEXUS_FrontendHandle handle);
static void NEXUS_Frontend_P_3472_GetType(void *handle, NEXUS_FrontendType *type);
static NEXUS_Error NEXUS_Frontend_P_3472_GetFastStatus(void *handle, NEXUS_FrontendFastStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_3472_TuneOfdm(void *handle, const NEXUS_FrontendOfdmSettings *pSettings);
static NEXUS_Error NEXUS_Frontend_P_3472_GetOfdmStatus(void *handle, NEXUS_FrontendOfdmStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_3472_GetOfdmAsyncStatus(void *handle, NEXUS_FrontendOfdmStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_3472_RequestOfdmAsyncStatus(void *handle);
static NEXUS_Error NEXUS_Frontend_P_3472_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings);
static NEXUS_Error NEXUS_Frontend_P_3472_ReapplyTransportSettings(void *handle);
static NEXUS_Error NEXUS_Frontend_P_3472_RequestIsdbtAsyncStatus(void *handle, NEXUS_FrontendIsdbtStatusType type);
static NEXUS_Error NEXUS_Frontend_P_3472_GetIsdbtAsyncStatusReady(void *handle, NEXUS_FrontendIsdbtStatusReady *pAsyncStatusReady);
static NEXUS_Error NEXUS_Frontend_P_3472_GetIsdbtAsyncStatus(void *handle, NEXUS_FrontendIsdbtStatusType type, NEXUS_FrontendIsdbtStatus *pStatus);
#if 0
static NEXUS_Error NEXUS_FrontendDevice_P_3472_setCrystalDaisySettings(const NEXUS_FrontendDevice3461OpenSettings *pSettings);
#endif
static NEXUS_Error NEXUS_FrontendDevice_P_3472_Standby(void * handle, const NEXUS_StandbySettings *pSettings);
static NEXUS_Error NEXUS_Tuner_P_3472_GetNSetGain(void *handle);
static void NEXUS_FrontendDevice_P_3472_GetCapabilities(void *handle, NEXUS_FrontendDeviceCapabilities *pCapabilities);
static void NEXUS_FrontendDevice_P_3472_GetTunerCapabilities(void *handle, unsigned tunerIndex, NEXUS_FrontendCapabilities *pCapabilities);
NEXUS_Error NEXUS_FrontendDevice_P_3472_GetInternalGain(void *handle, const NEXUS_GainParameters *params, NEXUS_InternalGainSettings *pSettings);
NEXUS_Error NEXUS_FrontendDevice_P_3472_GetExternalGain(void *handle, NEXUS_ExternalGainSettings *pSettings);
NEXUS_Error NEXUS_FrontendDevice_P_3472_SetExternalGain(void *handle, const NEXUS_ExternalGainSettings *pSettings);
static NEXUS_Error NEXUS_FrontendDevice_P_3472_GetStatus(void * handle, NEXUS_FrontendDeviceStatus *pStatus);
static void NEXUS_FrontendDevice_P_3472_Close(void *handle);
#if NEXUS_AMPLIFIER_SUPPORT
NEXUS_Error NEXUS_FrontendDevice_P_3472_GetAmplifierStatus(void *handle, NEXUS_AmplifierStatus *pStatus);
NEXUS_Error NEXUS_FrontendDevice_P_3472_SetAmplifierStatus(void *handle, const NEXUS_AmplifierStatus *pStatus);
#endif

static uint16_t NEXUS_Frontend_P_Get3472Rev(const NEXUS_FrontendDevice3461OpenSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BREG_I2C_Handle i2cHandle;
    uint8_t buf[2];
    uint16_t revId=0xef;
    uint8_t subAddr;
    uint8_t wData[2], rData[4];

    if(pSettings->i2cDevice){
        i2cHandle = NEXUS_I2c_GetRegHandle(pSettings->i2cDevice, NULL);
        if(i2cHandle == NULL ){rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto done;}
        buf[0]= 0x0;
        subAddr = 0x3;
        rc = BREG_I2C_WriteNoAddr(i2cHandle, pSettings->i2cAddr, (uint8_t *)&subAddr, 1);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        rc = BREG_I2C_ReadNoAddr(i2cHandle, pSettings->i2cAddr, buf, 1);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        revId = buf[0];

        subAddr = 0x4;
        rc = BREG_I2C_WriteNoAddr(i2cHandle, pSettings->i2cAddr, (uint8_t *)&subAddr, 1);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        rc = BREG_I2C_ReadNoAddr(i2cHandle, pSettings->i2cAddr, buf, 1);
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

static uint16_t NEXUS_Frontend_P_Is3472(const NEXUS_FrontendDevice3461OpenSettings *pSettings)
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
        rc = BREG_I2C_Read(i2cRegHandle, pSettings->i2cAddr, subAddr, buf, 1);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        chipId = buf[0];
        subAddr = 0x2;
        rc = BREG_I2C_Read(i2cRegHandle, pSettings->i2cAddr, subAddr, buf, 1);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        chipId = (chipId <<8) | buf[0];
    }
    else if (pSettings->spiDevice){
        wData[0] = pSettings->spiAddr << 1;   /* Chip address is 0x40 << 1 */
        wData[1] = 0x01;                      /* Register offset. */
        wData[2] = 0x0;
        wData[3] = 0x0;

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
    Lock callback handler for a 3472 Inband device
 ***************************************************************************/
static void NEXUS_Frontend_P_3472_callback_isr(void *pParam)
{
    NEXUS_FrontendHandle frontendHandle = NULL;
    NEXUS_3472Channel *pChannel;
    NEXUS_3472 *pDevice;
    BDBG_ASSERT(NULL != pParam);
    frontendHandle = *((NEXUS_FrontendHandle *)pParam);
    BDBG_ASSERT(NULL != frontendHandle);
    pChannel = frontendHandle->pDeviceHandle;
    BDBG_ASSERT(NULL != pChannel);
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);

    if(pDevice->acquireInProgress[pChannel->chn_num]){
        pDevice->count[pChannel->chn_num]++;
    }
    if(pDevice->count[pChannel->chn_num] == 2){
        pDevice->acquireInProgress[pChannel->chn_num] = false;
        pDevice->count[pChannel->chn_num] = 0;
    }

    if ( pDevice->lockAppCallback[pChannel->chn_num] )
    {
        NEXUS_IsrCallback_Fire_isr(pDevice->lockAppCallback[pChannel->chn_num]);
    }
}

/***************************************************************************
Summary:
    Lock callback handler for a 3472 Inband device
 ***************************************************************************/
static void NEXUS_Frontend_P_3472_AsyncStatusCallback_isr(void *pParam)
{
    NEXUS_FrontendHandle frontendHandle = NULL;
    NEXUS_3472Channel *pChannel;
    NEXUS_3472 *pDevice;
    BDBG_ASSERT(NULL != pParam);
    frontendHandle = *((NEXUS_FrontendHandle *)pParam);
    BDBG_ASSERT(NULL != frontendHandle);
    pChannel = frontendHandle->pDeviceHandle;
    BDBG_ASSERT(NULL != pChannel);
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);


    if(!pDevice->isInternalAsyncStatusCall[pChannel->chn_num]){
        pDevice->isStatusReady[pChannel->chn_num] = true;
        if (pDevice->asyncStatusAppCallback[pChannel->chn_num])
        {
            NEXUS_IsrCallback_Fire_isr(pDevice->asyncStatusAppCallback[pChannel->chn_num]);
        }
    }
}

/***************************************************************************
Summary:
    Enable/Disable interrupts for a 3472 device
 ***************************************************************************/
static void NEXUS_Frontend_P_3472_L1_isr(void *pParam1, int pParam2)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHAB_Handle hab = (BHAB_Handle)pParam2;
#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT
    NEXUS_3472 *pDevice, *pIntDevice = (NEXUS_3472 *)pParam1;
#else
    BSTD_UNUSED(pParam1);
#endif

    rc = BHAB_HandleInterrupt_isr(hab);
    if(rc){rc = BERR_TRACE(rc); goto done;}

#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT
    for ( pDevice = BLST_S_FIRST(&g_3472DevList); NULL != pDevice; pDevice = BLST_S_NEXT(pDevice, node) )
    {
        BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);

        if ((pDevice->hab != NULL) && (pDevice->openSettings.isrNumber== pIntDevice->openSettings.isrNumber) &&
            (pDevice->openSettings.gpioInterrupt == pIntDevice->openSettings.gpioInterrupt) && (pDevice->hab != hab))
        {
            rc = BHAB_HandleInterrupt_isr(pDevice->hab);
            if(rc){rc = BERR_TRACE(rc); goto done;}
        }
    }
#endif
done:
    return;
}
/***************************************************************************
Summary:
    Enable/Disable interrupts for a 3472 device
 ***************************************************************************/
static void NEXUS_Frontend_P_3472_GpioIsrControl_isr(bool enable, void *pParam)
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
    Enable/Disable interrupts for a 3472 device
 ***************************************************************************/
static void NEXUS_Frontend_P_3472_IsrControl_isr(bool enable, void *pParam)
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
    ISR Event Handler for a 3472 device
 ***************************************************************************/
static void NEXUS_Frontend_P_3472_IsrEvent(void *pParam)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHAB_Handle hab = (BHAB_Handle)pParam;

  if(hab != NULL){
    rc = BHAB_ProcessInterruptEvent(hab);
    if(rc) BERR_TRACE(rc);
  }
}
/***************************************************************************
Summary:
    Initialize Hab for a 3472 device
***************************************************************************/
void NEXUS_Frontend_P_UnInit_3472_Hab(NEXUS_3472 *pDevice)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_GpioSettings gpioSettings;

    if(pDevice->openSettings.isrNumber) {
#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT
        if(pDevice->isMaster)
        {
            NEXUS_Core_DisconnectInterrupt(pDevice->openSettings.isrNumber);
        }
#else
        NEXUS_Core_DisconnectInterrupt(pDevice->openSettings.isrNumber);
#endif
    }
    else if(pDevice->openSettings.gpioInterrupt){
#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT
        if(pDevice->isMaster)
        {
            NEXUS_Gpio_SetInterruptCallback_priv(pDevice->openSettings.gpioInterrupt, NULL, NULL, 0);
            NEXUS_Gpio_GetSettings(pDevice->openSettings.gpioInterrupt,  &gpioSettings);
            gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
            gpioSettings.interrupt.callback = NULL;
            rc = NEXUS_Gpio_SetSettings(pDevice->openSettings.gpioInterrupt, &gpioSettings);
            if(rc){rc = BERR_TRACE(rc); goto done;}

        }
#else
        NEXUS_Gpio_SetInterruptCallback_priv(pDevice->openSettings.gpioInterrupt, NULL, NULL, 0);
        NEXUS_Gpio_GetSettings(pDevice->openSettings.gpioInterrupt,  &gpioSettings);
        gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
        gpioSettings.interrupt.callback = NULL;
        NEXUS_Gpio_SetSettings(pDevice->openSettings.gpioInterrupt, &gpioSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
#endif
    }

    if(pDevice->isrEvent) pDevice->isrEvent = NULL;
    if(pDevice->isrEventCallback)NEXUS_UnregisterEvent(pDevice->isrEventCallback);
    pDevice->isrEventCallback = NULL;
#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT
    BKNI_EnterCriticalSection();
    BLST_S_REMOVE(&g_3472DevList, pDevice, NEXUS_3472, node);
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

static void NEXUS_Frontend_3472DeviceTestThread(void *arg)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    uint16_t chipId=0, revId=0;
    NEXUS_FrontendDevice3461Settings deviceSettings;
    NEXUS_3472 *pDevice = (NEXUS_3472 *)arg;

    chipId = NEXUS_Frontend_P_Is3472(&pDevice->openSettings);
    if ( chipId != 0x3472 )
    {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }
    revId = NEXUS_Frontend_P_Get3472Rev(&pDevice->openSettings);

    pDevice->chipId = chipId;
    pDevice->revId = revId;

    /* Init the acquisition processor */
    if (pDevice->openSettings.loadAP && !pDevice->pGenericDeviceHandle->abortThread)
    {
        BDBG_MSG(("BHAB_InitAp(rev a image)"));
        rc = BHAB_InitAp(pDevice->hab, bcm3472_leap_image);
        if ( rc != BERR_SUCCESS ) {
            BDBG_ERR(("Initializing 3472 Frontend core...UNSUCCESSFUL."));
        }
        else{
            BDBG_MSG(("Initializing 3472 Frontend core...SUCCESSFUL!!!"));
        }
    }
    else {
        BDBG_ERR(("NEXUS_Frontend_3472DeviceTestThread aborted by application."));
        goto done;
    }

    /* Initialize HAB, THD and TC2 */
    if(!pDevice->pGenericDeviceHandle->abortThread){
        rc = NEXUS_Frontend_P_Init3472(pDevice);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    else {
        BDBG_ERR(("NEXUS_Frontend_3472DeviceTestThread aborted by application."));
        goto done;
    }

    if(!pDevice->pGenericDeviceHandle->abortThread){
        NEXUS_FrontendDevice_GetDefault3461Settings(&deviceSettings);
        if(pDevice->settingsInitialized) deviceSettings = pDevice->deviceSettings;

        pDevice->isInternalSetSettings = true;
        rc = NEXUS_FrontendDevice_Set3461Settings(pDevice->pGenericDeviceHandle, &deviceSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    else {
        BDBG_ERR(("NEXUS_Frontend_3472DeviceTestThread aborted by application."));
        goto done;
    }

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
    Initialize Hab for a 3472 device
***************************************************************************/
NEXUS_Error NEXUS_FrontendDevice_P_Init3472(NEXUS_3472 *pDevice)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHAB_Settings stHabSettings;
    NEXUS_ThreadSettings thread_settings;
    BREG_I2C_Handle i2cHandle;
    BREG_SPI_Handle spiHandle;
#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT
    NEXUS_3472 *ptempDevice = BLST_S_FIRST(&g_3472DevList);
#endif
    rc = BHAB_3472_GetDefaultSettings(&stHabSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    stHabSettings.slaveChipAddr = pDevice->openSettings.i2cSlaveAddr;
    stHabSettings.isMtsif = pDevice->openSettings.isMtsif;
    if(pDevice->openSettings.isrNumber) {
        stHabSettings.interruptEnableFunc = NEXUS_Frontend_P_3472_IsrControl_isr;
        stHabSettings.interruptEnableFuncParam = (void*)pDevice->openSettings.isrNumber;
    }
    else if(pDevice->openSettings.gpioInterrupt){
        stHabSettings.interruptEnableFunc = NEXUS_Frontend_P_3472_GpioIsrControl_isr;
        stHabSettings.interruptEnableFuncParam = (void*)pDevice->openSettings.gpioInterrupt;
    }
    if(pDevice->openSettings.i2cDevice ){
        stHabSettings.chipAddr = pDevice->openSettings.i2cAddr;
        i2cHandle = NEXUS_I2c_GetRegHandle(pDevice->openSettings.i2cDevice, NEXUS_MODULE_SELF);
        if(i2cHandle == NULL ){rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto done;}
        stHabSettings.isSpi = false;
        rc = BHAB_Open( &pDevice->hab, (void *)i2cHandle, &stHabSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    else if(pDevice->openSettings.spiDevice ){
        stHabSettings.chipAddr = pDevice->openSettings.spiAddr;
        spiHandle = NEXUS_Spi_GetRegHandle(pDevice->openSettings.spiDevice);
        if(spiHandle == NULL ){rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto done;}
        stHabSettings.isSpi = true;
        rc = BHAB_Open( &pDevice->hab, (void *)spiHandle, &stHabSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT
    /* disconnect the previous for shared interrupt, last one is master */
    if(ptempDevice != NULL){
        if(pDevice->openSettings.isrNumber) {
            NEXUS_Core_DisconnectInterrupt(pDevice->openSettings.isrNumber);
        }
        else if(pDevice->openSettings.gpioInterrupt) {
            NEXUS_Gpio_SetInterruptCallback_priv(pDevice->openSettings.gpioInterrupt, NULL, NULL, 0);
        }
    }
#endif

    /* Success opeining Hab.  Connect Interrupt */
    if(pDevice->openSettings.isrNumber) {
        rc = NEXUS_Core_ConnectInterrupt(pDevice->openSettings.isrNumber, NEXUS_Frontend_P_3472_L1_isr, (void *)pDevice, (int)pDevice->hab);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    else if(pDevice->openSettings.gpioInterrupt){
        NEXUS_Gpio_SetInterruptCallback_priv(pDevice->openSettings.gpioInterrupt, NEXUS_Frontend_P_3472_L1_isr, (void *)pDevice, (int)pDevice->hab);
    }

    /* Get events and register callbacks */
    rc = BHAB_GetInterruptEventHandle(pDevice->hab, &pDevice->isrEvent);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pDevice->isrEventCallback = NEXUS_RegisterEvent(pDevice->isrEvent, NEXUS_Frontend_P_3472_IsrEvent, pDevice->hab);
    if ( NULL == pDevice->isrEventCallback ){rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done; }

    NEXUS_Thread_GetDefaultSettings(&thread_settings);
    thread_settings.priority = 50;
    pDevice->pGenericDeviceHandle->abortThread = false;
    pDevice->deviceOpenThread = NEXUS_Thread_Create("deviceOpenThread",
                                               NEXUS_Frontend_3472DeviceTestThread,
                                               (void*)pDevice,
                                               &thread_settings);
    return BERR_SUCCESS;
done:
    NEXUS_FrontendDevice_P_3472_Close(pDevice);
    return rc;
}

/***************************************************************************
Summary:
    Get the default settings for a BCM3472 tuner
See Also:
    NEXUS_Frontend_Open3472
 ***************************************************************************/
void NEXUS_Frontend_GetDefault3461Settings(NEXUS_3461Settings *pSettings)
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->type = NEXUS_3461ChannelType_eIsdbt;
    pSettings->channelNumber = 0;
}
/***************************************************************************
Summary:
    UnInitialize all ADS/THD/TC2 channels.
 ***************************************************************************/
void NEXUS_Frontend_P_UnInit3472(NEXUS_3472 *pDevice)
{
    NEXUS_Error rc = BERR_SUCCESS;
    unsigned int i;

#if NEXUS_HAS_MXT
    if (pDevice->pGenericDeviceHandle->mtsifConfig.mxt) {
        BMXT_Close(pDevice->pGenericDeviceHandle->mtsifConfig.mxt);
        pDevice->pGenericDeviceHandle->mtsifConfig.mxt = NULL;
    }
#endif

    for ( i = 0; i < NEXUS_MAX_3472_TUNERS && NULL != pDevice->tnr[i]; i++) {
        if(pDevice->tnr[i]){
            rc = BTNR_Close(pDevice->tnr[i]);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->tnr[i] = NULL;
        }
    }
    for ( i = 0; i < NEXUS_3472_MAX_OFDM_CHN && NULL != pDevice->ods_chn[i]; i++) {
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

static void NEXUS_FrontendDevice_P_3472_GetCapabilities(void *handle, NEXUS_FrontendDeviceCapabilities *pCapabilities)
{
    NEXUS_3472 *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_3472 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);

    pCapabilities->numTuners = pDevice->capabilities.totalTunerChannels;
    return;
}

static void NEXUS_FrontendDevice_P_3472_GetTunerCapabilities(void *handle, unsigned tunerIndex, NEXUS_FrontendCapabilities *pCapabilities)
{
    NEXUS_3472 *pDevice;
    BHAB_ChannelCapability *channelCapability;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_3472 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);
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
static NEXUS_Error NEXUS_Frontend_P_Init3472(NEXUS_3472 *pDevice)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned int i, j;
    BTNR_3472_Settings tnr3472_cfg;
    BODS_Settings odsCfg;
    BODS_ChannelSettings odsChnCfg;
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

    rc = BODS_3472_GetDefaultSettings( &odsCfg, NULL);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    odsCfg.hGeneric = pDevice->hab;
    rc = BODS_Open(&pDevice->ods, NULL, NULL, NULL, &odsCfg);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = BODS_Init(pDevice->ods);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = BODS_GetTotalChannels(pDevice->ods, BODS_Standard_eIsdbt, &pDevice->numChannels[BODS_Standard_eIsdbt]);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if(pDevice->numChannels[BODS_Standard_eIsdbt] > NEXUS_MAX_3472_TUNERS) {
        BDBG_MSG(("The maximum number of channels is incorrect. num_ch = %d, standard = %d", pDevice->numChannels[BODS_Standard_eIsdbt], BODS_Standard_eIsdbt));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }

    rc = BODS_GetChannelDefaultSettings( pDevice->ods, BODS_Standard_eIsdbt, &odsChnCfg);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    /* This needs to be fixed for every standard type. */
    for (j=0;j<pDevice->numChannels[BODS_Standard_eIsdbt];j++) {
        odsChnCfg.channelNo = j;
        odsChnCfg.standard = BODS_Standard_eIsdbt;
        rc = BODS_OpenChannel( pDevice->ods, &pDevice->ods_chn[j], &odsChnCfg);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        /* Install ODS callbacks. */
        rc = BODS_InstallCallback(pDevice->ods_chn[j], BODS_Callback_eLockChange, (BODS_CallbackFunc)NEXUS_Frontend_P_3472_callback_isr, (void*)&pDevice->frontendHandle[j]);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        rc = BODS_InstallCallback(pDevice->ods_chn[j], BODS_Callback_eAsyncStatusReady, (BODS_CallbackFunc)NEXUS_Frontend_P_3472_AsyncStatusCallback_isr, (void*)&pDevice->frontendHandle[j]);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    for (i=0;i<NEXUS_MAX_3472_TUNERS;i++) {
        rc = BTNR_3472_GetDefaultSettings(&tnr3472_cfg);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        tnr3472_cfg.channelNo = i;
        rc =  BTNR_3472_Open(&pDevice->tnr[i],&tnr3472_cfg, pDevice->hab);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        pwrSettings.enable = false;
        rc = BTNR_SetPowerSaver(pDevice->tnr[i], &pwrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

#if NEXUS_HAS_MXT
    {
        /* open MXT */
        BMXT_Settings mxtSettings;
        BMXT_3472_GetDefaultSettings(&mxtSettings);
        for (i=0; i<BMXT_MAX_NUM_MTSIF_TX; i++) {
            mxtSettings.MtsifTxCfg[i].Enable = pDevice->openSettings.isMtsif; /* note, .isMtsif passed to BHAB_Open is independent of this */
            NEXUS_Module_Lock(g_NEXUS_frontendModuleSettings.transport);
            mxtSettings.MtsifTxCfg[i].Encrypt = NEXUS_TransportModule_P_IsMtsifEncrypted();
            NEXUS_Module_Unlock(g_NEXUS_frontendModuleSettings.transport);
        }
        for (i=0; i<BMXT_MAX_NUM_MTSIF_RX; i++) {
            if(pDevice->openSettings.i2cAddr == 0x6e) { /* TODO: hack */
                mxtSettings.MtsifRxCfg[i].Enable = true;
                NEXUS_Module_Lock(g_NEXUS_frontendModuleSettings.transport);
                mxtSettings.MtsifRxCfg[i].Decrypt = NEXUS_TransportModule_P_IsMtsifEncrypted();
                NEXUS_Module_Unlock(g_NEXUS_frontendModuleSettings.transport);
            }
        }
        mxtSettings.hHab = pDevice->hab;
        rc = BMXT_Open(&pDevice->pGenericDeviceHandle->mtsifConfig.mxt, NULL, NULL, &mxtSettings);
        if (rc!=BERR_SUCCESS) goto done;
        rc = NEXUS_Frontend_P_InitMtsifConfig(&pDevice->pGenericDeviceHandle->mtsifConfig, &mxtSettings);
        if (rc!=BERR_SUCCESS) goto done;
    }
#endif

    return BERR_SUCCESS;

done:
    NEXUS_Frontend_P_UnInit3472(pDevice);
    return rc;
}

#if NEXUS_AMPLIFIER_SUPPORT
NEXUS_Error NEXUS_FrontendDevice_P_3472_GetAmplifierStatus(void *handle, NEXUS_AmplifierStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3472 *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_3472 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);

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

NEXUS_Error NEXUS_FrontendDevice_P_3472_SetAmplifierStatus(void *handle, const NEXUS_AmplifierStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3472 *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_3472 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);

    BSTD_UNUSED(pStatus);
    /* Set LNA/Amplifier parameters. */

    return rc;
}
#endif

NEXUS_Error NEXUS_Tuner_P_3472_GetNSetGain(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_GainParameters params;
    NEXUS_InternalGainSettings settings;
    NEXUS_ExternalGainSettings externalGainSettings;
    NEXUS_3472 *pDevice;
    NEXUS_3472Channel *pChannel;
    pChannel = (NEXUS_3472Channel *)handle;
    pDevice = pChannel->pDevice;


    BKNI_Memset(&externalGainSettings, 0, sizeof(externalGainSettings));

    if(pDevice->pGenericDeviceHandle->parent){

        params.rfInput = pDevice->pGenericDeviceHandle->linkSettings.rfInput;
        params.accumulateTillRootDevice = true;
        params.frequency = pDevice->last_ofdm[pChannel->chn_num].frequency;

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


NEXUS_Error NEXUS_FrontendDevice_P_3472_GetInternalGain(void *handle, const NEXUS_GainParameters *params, NEXUS_InternalGainSettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3472 *pDevice;
    BHAB_InternalGainInputParams inputParams;
    BHAB_InternalGainSettings internalGain;
    NEXUS_GainParameters gainParams;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_3472 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);

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
NEXUS_Error NEXUS_FrontendDevice_P_3472_GetExternalGain(void *handle, NEXUS_ExternalGainSettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3472 *pDevice;
    BHAB_ExternalGainSettings gain;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_3472 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);
    BSTD_UNUSED(pSettings);

    rc = BHAB_GetExternalGain(pDevice->hab, &gain);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pSettings->bypassableGain = gain.externalGainBypassable;
    pSettings->totalGain = gain.externalGainTotal;

done:
    return rc;

}
NEXUS_Error NEXUS_FrontendDevice_P_3472_SetExternalGain(void *handle, const NEXUS_ExternalGainSettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3472 *pDevice;
    BHAB_ExternalGainSettings externalGain;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_3472 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);

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
    NEXUS_3472 *pDevice = NULL;
    NEXUS_FrontendDevice *pFrontendDevice = NULL;
    NEXUS_3461ProbeResults results;

    BSTD_UNUSED(index);

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT((NULL != pSettings->i2cDevice) || (NULL != pSettings->spiDevice));

    rc = NEXUS_Frontend_Probe3461(pSettings, &results);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    for ( pDevice = BLST_S_FIRST(&g_3472DevList); NULL != pDevice; pDevice = BLST_S_NEXT(pDevice, node) )
    {
        BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);
        if(((pSettings->i2cAddr == pDevice->openSettings.i2cAddr) && (pSettings->i2cDevice== pDevice->openSettings.i2cDevice)) &&
            ((pSettings->spiAddr == pDevice->openSettings.spiAddr) && (pSettings->spiDevice== pDevice->openSettings.spiDevice)))
        {
            break;
        }
    }

    if ( NULL == pDevice)
    {
#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT
        for ( pDevice = BLST_S_FIRST(&g_3472DevList); NULL != pDevice; pDevice = BLST_S_NEXT(pDevice, node) )
        {
            BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);
            if ((pSettings->gpioInterrupt == pDevice->openSettings.gpioInterrupt) && (pSettings->isrNumber == pDevice->openSettings.isrNumber))
            {
                pDevice->isMaster = false;
            }
        }
#endif
        pFrontendDevice = BKNI_Malloc(sizeof(*pFrontendDevice));
        if (NULL == pFrontendDevice) {rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done;}

        /* Memsetting the whole structure should cover initializing the child list. */
        BKNI_Memset(pFrontendDevice, 0, sizeof(*pFrontendDevice));

        pDevice = BKNI_Malloc(sizeof(NEXUS_3472));
        if (NULL == pDevice) {rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done;}

        BKNI_Memset(pDevice, 0, sizeof(NEXUS_3472));
        BDBG_OBJECT_SET(pDevice, NEXUS_3472);

        pDevice->pGenericDeviceHandle = pFrontendDevice;
        pDevice->openSettings = *pSettings;
#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT
        pDevice->isMaster = true;
#endif

        BKNI_EnterCriticalSection();
        pDevice->pGenericDeviceHandle->openPending = true;
        pDevice->pGenericDeviceHandle->openFailed = false;
        BKNI_LeaveCriticalSection();

        /* Initialize ADS channels, THD and TC2 */
        rc = NEXUS_FrontendDevice_P_Init3472(pDevice);
        if(rc){rc = BERR_TRACE(rc); goto done;}

#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT
        BKNI_EnterCriticalSection();
        BLST_S_INSERT_HEAD(&g_3472DevList, pDevice, node);
        BKNI_LeaveCriticalSection();
#else
        BLST_S_INSERT_HEAD(&g_3472DevList, pDevice, node);
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
    pFrontendDevice->getInternalGain = NEXUS_FrontendDevice_P_3472_GetInternalGain;
    pFrontendDevice->getExternalGain = NEXUS_FrontendDevice_P_3472_GetExternalGain;
    pFrontendDevice->setExternalGain = NEXUS_FrontendDevice_P_3472_SetExternalGain;
    pFrontendDevice->getStatus = NEXUS_FrontendDevice_P_3472_GetStatus;
    pFrontendDevice->standby = NEXUS_FrontendDevice_P_3472_Standby;
    pFrontendDevice->close = NEXUS_FrontendDevice_P_3472_Close;
    pFrontendDevice->getCapabilities = NEXUS_FrontendDevice_P_3472_GetCapabilities;
    pFrontendDevice->getTunerCapabilities = NEXUS_FrontendDevice_P_3472_GetTunerCapabilities;
#if NEXUS_AMPLIFIER_SUPPORT
    pFrontendDevice->getAmplifierStatus = NEXUS_FrontendDevice_P_3472_GetAmplifierStatus;
    pFrontendDevice->setAmplifierStatus = NEXUS_FrontendDevice_P_3472_SetAmplifierStatus;
#endif
    return pFrontendDevice;
done:
    NEXUS_FrontendDevice_P_3472_Close(pDevice);
    return NULL;
}

/***************************************************************************
Summary:
    Open a handle to a BCM3472 device.
 ***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Open3461(const NEXUS_3461Settings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_FrontendHandle frontendHandle = NULL;
    NEXUS_3472 *pDevice = NULL;
    unsigned chn_num=0;
    NEXUS_3472Channel *pChannel;
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
        pDevice = (NEXUS_3472 *)pFrontendDevice->pDevice;
    }
    else {
        pDevice = (NEXUS_3472 *)pSettings->device->pDevice;
        pFrontendDevice =  pSettings->device;
    }

    chn_num = pSettings->channelNumber;

    /* chekc if fronthandle is already opened*/
    if ( pDevice->frontendHandle[chn_num] != NULL )
    {
        return pDevice->frontendHandle[chn_num];
    }

    pChannel = (NEXUS_3472Channel*)BKNI_Malloc(sizeof(NEXUS_3472Channel));
    if ( NULL == pChannel ) {rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done;}

    /* Create a Nexus frontend handle */
    frontendHandle = NEXUS_Frontend_P_Create(pChannel);
    if ( NULL == frontendHandle ) {rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done;}

    /* Establish device capabilities */
    frontendHandle->capabilities.ofdm = true;
    frontendHandle->capabilities.ofdmModes[NEXUS_FrontendOfdmMode_eIsdbt] = true;

    frontendHandle->requestOfdmAsyncStatus = NEXUS_Frontend_P_3472_RequestOfdmAsyncStatus;
    frontendHandle->getOfdmAsyncStatus = NEXUS_Frontend_P_3472_GetOfdmAsyncStatus;
    frontendHandle->getOfdmStatus = NEXUS_Frontend_P_3472_GetOfdmStatus;
    frontendHandle->tuneOfdm = NEXUS_Frontend_P_3472_TuneOfdm;
    frontendHandle->untune = NEXUS_Frontend_P_3472_UnTune;
    frontendHandle->resetStatus = NEXUS_Frontend_P_3472_ResetStatus;
    frontendHandle->readSoftDecisions = NEXUS_Frontend_P_3472_ReadSoftDecisions;
    frontendHandle->close = NEXUS_Frontend_P_3472_Close;
    frontendHandle->getFastStatus = NEXUS_Frontend_P_3472_GetFastStatus;
    frontendHandle->getType = NEXUS_Frontend_P_3472_GetType;
    frontendHandle->standby = NEXUS_Frontend_P_3472_Standby;
    frontendHandle->requestIsdbtAsyncStatus = NEXUS_Frontend_P_3472_RequestIsdbtAsyncStatus;
    frontendHandle->getIsdbtAsyncStatusReady = NEXUS_Frontend_P_3472_GetIsdbtAsyncStatusReady;
    frontendHandle->getIsdbtAsyncStatus = NEXUS_Frontend_P_3472_GetIsdbtAsyncStatus;
    frontendHandle->reapplyTransportSettings = NEXUS_Frontend_P_3472_ReapplyTransportSettings;
    frontendHandle->uninstallCallbacks = NEXUS_Frontend_P_3472_UninstallCallbacks;

    frontendHandle->pGenericDeviceHandle = pFrontendDevice;

    /* Create app callbacks */
    pDevice->lockAppCallback[chn_num] = NEXUS_IsrCallback_Create(frontendHandle, NULL);
    if ( NULL == pDevice->lockAppCallback[chn_num] ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto done;}

    pDevice->asyncStatusAppCallback[chn_num] = NEXUS_IsrCallback_Create(frontendHandle, NULL);
    if ( NULL == pDevice->asyncStatusAppCallback[chn_num] ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto done;}

    pDevice->frontendHandle[chn_num] = frontendHandle;
    frontendHandle->chip.familyId = 0x3472;
    frontendHandle->chip.id = 0x3472;
    /* save channel number in pChannel*/
    pChannel->chn_num = chn_num;
    pChannel->pDevice = pDevice;
    pDevice->numfrontends++;

#if NEXUS_HAS_MXT
    frontend->userParameters.isMtsif = true;
    frontend->mtsif.daisyTxOut = 1;
#endif

    return frontendHandle;

done:
    NEXUS_FrontendDevice_P_3472_Close(pDevice);
    return NULL;
}

/***************************************************************************
Summary:
    Close a handle to a BCM3472 device.
See Also:
    NEXUS_Frontend_Open3472
 ***************************************************************************/
static void NEXUS_Frontend_P_3472_Close(NEXUS_FrontendHandle handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3472 *pDevice;
    NEXUS_3472Channel *pChannel;
    BTNR_PowerSaverSettings tnrPwrSettings;

    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    pChannel =(NEXUS_3472Channel*) handle->pDeviceHandle;
    BDBG_ASSERT(NULL != pChannel);
    pDevice = (NEXUS_3472 *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);

    if (pChannel->chn_num >= NEXUS_MAX_3472_FRONTENDS)
    {
        BDBG_MSG((" Unsupported Frontend Handle"));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    while(pDevice->deviceOpenThread && pDevice->pGenericDeviceHandle->openPending){
        BKNI_Delay(500000); /* Wait for a half second everytime. */
    }

    if(pDevice->isPoweredOn[pChannel->chn_num]) {
        NEXUS_Frontend_P_3472_UnTune(pChannel);
    }
    if(pDevice->isTunerPoweredOn[pChannel->chn_num] && pDevice->tnr[pChannel->chn_num]){
        tnrPwrSettings.enable = true;
        rc = BTNR_SetPowerSaver(pDevice->tnr[pChannel->chn_num], &tnrPwrSettings);
        if(rc){rc = BERR_TRACE(rc);}
    }
    if(pDevice->ods_chn[pChannel->chn_num]) {
        BODS_InstallCallback(pDevice->ods_chn[pChannel->chn_num], BODS_Callback_eLockChange, NULL, NULL);
        BODS_InstallCallback(pDevice->ods_chn[pChannel->chn_num], BODS_Callback_eAsyncStatusReady, NULL, NULL);
    }

    if ( NULL != pDevice->lockAppCallback[pChannel->chn_num])
        NEXUS_IsrCallback_Destroy(pDevice->lockAppCallback[pChannel->chn_num]);
    if ( NULL != pDevice->asyncStatusAppCallback[pChannel->chn_num])
        NEXUS_IsrCallback_Destroy(pDevice->asyncStatusAppCallback[pChannel->chn_num]);

    NEXUS_Frontend_P_Destroy(handle);

    pDevice->frontendHandle[pChannel->chn_num] = NULL;
    BKNI_Free(pChannel);
    pDevice->numfrontends--;

done:
    return;
}

static void NEXUS_FrontendDevice_P_3472_Close(void *handle)
{
    NEXUS_3472 *pDevice;
    unsigned i=0;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_3472 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);

    if(pDevice->numfrontends){
        if(pDevice->frontendHandle[i])NEXUS_Frontend_P_3472_Close(pDevice->frontendHandle[i]);
    }

    if(pDevice->deviceOpenThread){
        NEXUS_Thread_Destroy(pDevice->deviceOpenThread);
        pDevice->deviceOpenThread = NULL;
    }

    NEXUS_Frontend_P_UnInit3472(pDevice);
    NEXUS_Frontend_P_UnInit_3472_Hab(pDevice);
#ifndef NEXUS_SHARED_FRONTEND_INTERRUPT
        BLST_S_REMOVE(&g_3472DevList, pDevice, NEXUS_3472, node);
#endif

    NEXUS_FrontendDevice_Unlink(pDevice->pGenericDeviceHandle, NULL);
    BKNI_Free(pDevice->pGenericDeviceHandle);
    pDevice->pGenericDeviceHandle = NULL;

    if(pDevice->capabilities.channelCapabilities)
        BKNI_Free(pDevice->capabilities.channelCapabilities);
    pDevice->capabilities.channelCapabilities = NULL;

    BDBG_OBJECT_DESTROY(pDevice, NEXUS_3472);
    BKNI_Free(pDevice);
    pDevice = NULL;

    return;
}

static NEXUS_FrontendLockStatus  NEXUS_Frontend_P_GetLockStatus(unsigned lockStatus)
{
    switch ( lockStatus )
    {
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
static NEXUS_Error NEXUS_Frontend_P_3472_GetFastStatus(void *handle, NEXUS_FrontendFastStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3472 *pDevice;
    unsigned lock;
    NEXUS_3472Channel *pChannel;

    pChannel =(NEXUS_3472Channel*) handle;
    BDBG_ASSERT(NULL != pChannel);
    pDevice = (NEXUS_3472 *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);

    if (pChannel->chn_num >= NEXUS_MAX_3472_FRONTENDS)
    {
        BDBG_MSG((" Unsupported Frontend Handle"));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    rc = BODS_GetLockStatus(pDevice->ods_chn[pChannel->chn_num],  &lock);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pStatus->lockStatus = NEXUS_Frontend_P_GetLockStatus(lock);
    pStatus->acquireInProgress = pDevice->acquireInProgress[pChannel->chn_num];
    return BERR_SUCCESS;
done:
    return rc;
}

static void NEXUS_Frontend_P_3472_ResetStatus(void *handle)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3472Channel *pChannel;
    NEXUS_3472 *pDevice;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3472Channel *)handle;
    pDevice = (NEXUS_3472 *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);

   if(pChannel->chn_num < NEXUS_3472_MAX_OFDM_CHN){
        if(pDevice->isPoweredOn[pChannel->chn_num]){
            rc = BODS_ResetStatus(pDevice->ods_chn[pChannel->chn_num]);
            if(rc){rc = BERR_TRACE(rc); goto done;}
        }
    }
    else{
        BDBG_ERR((" Unsupported channel."));
        BERR_TRACE(BERR_NOT_SUPPORTED);
    }
done:
    return;
}


/***************************************************************************
Summary:
    Initialize Hab for a 3472 device
***************************************************************************/
void NEXUS_FrontendDevice_P_3472_S3Standby(NEXUS_3472 *pDevice)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    if(pDevice->deviceOpenThread)NEXUS_Thread_Destroy(pDevice->deviceOpenThread);
    pDevice->deviceOpenThread = NULL;

    NEXUS_Frontend_P_UnInit3472(pDevice);

#ifndef NEXUS_SHARED_FRONTEND_INTERRUPT
    if(pDevice->openSettings.isrNumber) {
        NEXUS_Core_DisconnectInterrupt(pDevice->openSettings.isrNumber);
    }
    else if(pDevice->openSettings.gpioInterrupt){
        NEXUS_Gpio_SetInterruptCallback_priv(pDevice->openSettings.gpioInterrupt, NULL, NULL, 0);
    }
#endif

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

static NEXUS_Error NEXUS_FrontendDevice_P_3472_Standby(void *handle, const NEXUS_StandbySettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3472 *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_3472 *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);

    if((pDevice->pGenericDeviceHandle->mode != NEXUS_StandbyMode_eDeepSleep) && (pSettings->mode == NEXUS_StandbyMode_eDeepSleep)){

        NEXUS_FrontendDevice_P_3472_S3Standby(pDevice);

        BKNI_EnterCriticalSection();
        pDevice->pGenericDeviceHandle->openPending = true;
        pDevice->pGenericDeviceHandle->openFailed = false;
        BKNI_LeaveCriticalSection();
    }
    else if((pDevice->pGenericDeviceHandle->mode == NEXUS_StandbyMode_eDeepSleep) && (pSettings->mode != NEXUS_StandbyMode_eDeepSleep)){

        rc = NEXUS_FrontendDevice_P_Init3472(pDevice);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_3472_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3472Channel *pChannel;
    NEXUS_3472 *pDevice;
    BTNR_PowerSaverSettings tnrPwrSettings;
    BODS_PowerSaverSettings odsPwrSettings;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3472Channel *)handle;
    pDevice = (NEXUS_3472 *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);

    BSTD_UNUSED(enabled);

    if (pChannel->chn_num >= NEXUS_MAX_3472_FRONTENDS)
    {
        BDBG_MSG((" Unsupported Frontend Handle"));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    if(pSettings->mode >= NEXUS_StandbyMode_ePassive){
        NEXUS_Frontend_P_3472_UninstallCallbacks(handle);
        if(pDevice->isPoweredOn[pChannel->chn_num]){
            rc = BODS_EnablePowerSaver(pDevice->ods_chn[pChannel->chn_num], &odsPwrSettings);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            pDevice->isPoweredOn[pChannel->chn_num] = false;
        }
        if(pDevice->isTunerPoweredOn[pChannel->chn_num]){
            tnrPwrSettings.enable = true;
            rc = BTNR_SetPowerSaver(pDevice->tnr[pChannel->chn_num], &tnrPwrSettings);
            if(rc){rc = BERR_TRACE(rc);}

            pDevice->isTunerPoweredOn[pChannel->chn_num] = false;
        }
    }
done:
    return rc;
}

static void NEXUS_Frontend_P_3472_UninstallCallbacks(void *handle)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3472 *pDevice = (NEXUS_3472 *)handle;
    NEXUS_3472Channel *pChannel;

    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3472Channel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);

    if (pChannel->chn_num >= NEXUS_3472_MAX_OFDM_CHN)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    if(pDevice->lockAppCallback[pChannel->chn_num])NEXUS_IsrCallback_Set(pDevice->lockAppCallback[pChannel->chn_num], NULL);
    if(pDevice->asyncStatusAppCallback[pChannel->chn_num])NEXUS_IsrCallback_Set(pDevice->asyncStatusAppCallback[pChannel->chn_num], NULL);

done:
    return;
}

static void NEXUS_Frontend_P_3472_UnTune(void *handle)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3472 *pDevice = (NEXUS_3472 *)handle;
    unsigned chn_num = 0;
    NEXUS_3472Channel *pChannel;
    BODS_PowerSaverSettings odsPwrSettings;

    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3472Channel *)handle;
    pDevice = pChannel->pDevice;
    chn_num = pChannel->chn_num;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);

    if (chn_num >= NEXUS_3472_MAX_OFDM_CHN)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    /* TODO: This has been disabled intentionally. */
#if 0
    NEXUS_Frontend_P_UnsetMtsifConfig(pDevice->frontendHandle[pChannel->chn_num], &pDevice->mtsifConfig);
#endif
    if(pDevice->isPoweredOn[pChannel->chn_num] && pDevice->ods_chn[pChannel->chn_num]){
        rc = BODS_EnablePowerSaver(pDevice->ods_chn[pChannel->chn_num], &odsPwrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->isPoweredOn[pChannel->chn_num] = false;
    }

done:
    return;
}

static NEXUS_Error NEXUS_Frontend_P_3472_ReadSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length, size_t *pNumRead)
{
    size_t i;
    NEXUS_Error rc = NEXUS_SUCCESS;
    int16_t return_length;
    NEXUS_3472 *pDevice;
    int16_t d_i[TOTAL_SOFTDECISIONS], d_q[TOTAL_SOFTDECISIONS];
    unsigned chn_num = 0;
    NEXUS_3472Channel *pChannel;

    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3472Channel *)handle;
    pDevice = pChannel->pDevice;
    chn_num = pChannel->chn_num;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);

    if (chn_num >= NEXUS_3472_MAX_OFDM_CHN)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    BKNI_Memset(pDecisions, 0, (sizeof(NEXUS_FrontendSoftDecision) * length));

    /* only make one call to ADS. if app needs more, they can loop. */
    rc = BODS_GetSoftDecision(pDevice->ods_chn[chn_num], (int16_t)TOTAL_SOFTDECISIONS, d_i, d_q, &return_length);
    if(rc){rc = BERR_TRACE(rc); goto done;}

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
  Probe to see if a BCM3472 device exists with the specified settings

Description:
  Probe to see if a BCM3472 device exists with the specified settings

See Also:
    NEXUS_Frontend_Open3472
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Probe3461(const NEXUS_FrontendDevice3461OpenSettings *pSettings, NEXUS_3461ProbeResults *pResults)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    uint16_t chipVer=0xffff;

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(!((NULL == pSettings->i2cDevice) && (NULL == pSettings->spiDevice)));
    BDBG_ASSERT(NULL != pResults);

    pResults->chip.familyId = 0x0;

    pResults->chip.familyId = (uint32_t)NEXUS_Frontend_P_Is3472(pSettings);
    if ( pResults->chip.familyId != 0x3472 )
    {
        BDBG_WRN(("pResults->chip.familyId = 0x%x", pResults->chip.familyId));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }
    pResults->chip.id = pResults->chip.familyId;
    chipVer = NEXUS_Frontend_P_Get3472Rev(pSettings);
done:
    pResults->chip.version.major = (chipVer >> 8) + 1;
    pResults->chip.version.minor = chipVer & 0xff;

    return rc;
}

/***************************************************************************
Summary:
    Retrieve the chip family id, chip id, chip version and firmware version.
***************************************************************************/
static void NEXUS_Frontend_P_3472_GetType(void *handle, NEXUS_FrontendType *type)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3472 *pDevice;
    uint32_t       familyId, chipId, chn_num;
    uint16_t       chipVer;
    uint8_t        majApVer, minApVer;
    NEXUS_3472Channel *pChannel;

    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3472Channel *)handle;
    pDevice = pChannel->pDevice;
    chn_num = pChannel->chn_num;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);

    if (chn_num >= NEXUS_3472_MAX_OFDM_CHN)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    type->chip.familyId = (uint32_t)pDevice->chipId;
    type->chip.id = (uint32_t)pDevice->chipId;
    type->chip.version.major = (pDevice->revId >> 8) + 1;
    type->chip.version.minor = pDevice->revId & 0xff;

    BHAB_GetApVersion(pDevice->hab, &familyId, &chipId, &chipVer, &majApVer, &minApVer);
    if((type->chip.familyId != familyId) || (type->chip.id != chipId)){
        BDBG_ERR(("Type mismatch while retreiving chip id and family id."));
        BERR_TRACE(BERR_UNKNOWN); goto done;
    }
    type->firmwareVersion.major = majApVer;
    type->firmwareVersion.minor = minApVer;
done:
    return;
}

static NEXUS_Error NEXUS_Frontend_P_3472_TuneOfdm(void *handle, const NEXUS_FrontendOfdmSettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3472 *pDevice = (NEXUS_3472 *)handle;
    BODS_AcquireParams odsParam;
    unsigned chn_num = 0;
    BTNR_Settings tnrSettings;
    BTNR_PowerSaverSettings tnrPwrSettings;
    unsigned temp_frequency;
    NEXUS_FrontendOfdmMode temp_mode;
    BODS_PowerSaverSettings pwrSettings;
    NEXUS_3472Channel *pChannel;

    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3472Channel *)handle;
    pDevice = pChannel->pDevice;
    chn_num = pChannel->chn_num;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);

    if (chn_num >= NEXUS_3472_MAX_OFDM_CHN)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    rc = NEXUS_Tuner_P_3472_GetNSetGain(pChannel);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = NEXUS_Frontend_P_SetMtsifConfig(pDevice->frontendHandle[chn_num]);
    if (rc) { rc = BERR_TRACE(rc); goto done; }

    rc = BODS_GetDefaultAcquireParams(pDevice->ods_chn[chn_num], &odsParam);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    NEXUS_IsrCallback_Set(pDevice->lockAppCallback[chn_num], &(pSettings->lockCallback));
    NEXUS_IsrCallback_Set(pDevice->asyncStatusAppCallback[chn_num], &(pSettings->asyncStatusReadyCallback));

    if(!pDevice->isPoweredOn[chn_num] || !pDevice->isTunerPoweredOn[chn_num])
        goto full_acquire;

    if((pSettings->acquisitionMode == NEXUS_FrontendOfdmAcquisitionMode_eScan) && (pDevice->lastAcquisitionMode[chn_num] == NEXUS_FrontendOfdmAcquisitionMode_eScan)){
        temp_frequency = pDevice->last_ofdm[chn_num].frequency;
        pDevice->last_ofdm[chn_num].frequency = pSettings->frequency ;
        temp_mode = pDevice->last_ofdm[chn_num].mode;
        pDevice->last_ofdm[chn_num].mode = pSettings->mode;

        if(!BKNI_Memcmp(pSettings, &pDevice->last_ofdm[chn_num], sizeof(NEXUS_FrontendOfdmSettings))) {
            if (pDevice->tnr[chn_num])
            {
                pDevice->count[chn_num] = 0;
                pDevice->acquireInProgress[chn_num] = true;
                pDevice->last_ofdm[chn_num] = *pSettings;
                rc = BTNR_SetTunerRfFreq(pDevice->tnr[chn_num], pSettings->frequency, BTNR_TunerMode_eDigital);
                if(rc){rc = BERR_TRACE(rc); goto retrack;}
                return rc;
            }
        }
    }

full_acquire:
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
        BDBG_MSG((" Unsupported Acquisition mode."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }

    if(!pDevice->isTunerPoweredOn[chn_num]){
        tnrPwrSettings.enable = false;
        rc = BTNR_SetPowerSaver(pDevice->tnr[pChannel->chn_num], &tnrPwrSettings);
        if(rc){rc = BERR_TRACE(rc);}
        pDevice->isTunerPoweredOn[chn_num] = true;
    }
    if(!pDevice->isPoweredOn[chn_num]){
        rc = BODS_DisablePowerSaver(pDevice->ods_chn[chn_num], &pwrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->isPoweredOn[chn_num] = true;
    }

    switch ( pSettings->mode )
    {
    case NEXUS_FrontendOfdmMode_eIsdbt:
       switch ( pSettings->bandwidth )
        {
        case 1700000:
            odsParam.acquireParams.isdbt.bandwidth = BODS_IsdbtBandwidth_e1p7MHz;
            break;
        case 5000000:
            odsParam.acquireParams.isdbt.bandwidth = BODS_IsdbtBandwidth_e5MHz;
            break;
        case 6000000:
            odsParam.acquireParams.isdbt.bandwidth = BODS_IsdbtBandwidth_e6MHz;
            break;
        case 7000000:
            odsParam.acquireParams.isdbt.bandwidth = BODS_IsdbtBandwidth_e7MHz;
            break;
        case 8000000:
            odsParam.acquireParams.isdbt.bandwidth = BODS_IsdbtBandwidth_e8MHz;
            break;
        default:
            BDBG_MSG((" Unsupported bandwidth."));
            rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
        }
        switch (pSettings->cciMode)
        {
        case NEXUS_FrontendOfdmCciMode_eNone:
            odsParam.acquireParams.isdbt.cciMode = BODS_IsdbtCciMode_eNone;
            break;
        case NEXUS_FrontendOfdmCciMode_eAuto:
            odsParam.acquireParams.isdbt.cciMode = BODS_IsdbtCciMode_eAuto;
            break;
        default:
            BDBG_MSG((" Unsupported bandwidth."));
            rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
        }

        odsParam.spurDetectAndCancel = pSettings->spurDetectAndCancel;
        odsParam.acquireParams.isdbt.carrierRange = (BODS_IsdbtCarrierRange)pSettings->pullInRange;
        if(pSettings->manualModeSettings){
            switch (pSettings->modeSettings.modeGuard)
            {
            case NEXUS_FrontendOfdmModeGuard_eAutoIsdbtJapan:
                odsParam.acquireParams.isdbt.transGuardMode = BODS_IsdbtOfdmMode_eAutoJapan;
                break;
            case NEXUS_FrontendOfdmModeGuard_eAutoIsdbtBrazil:
                odsParam.acquireParams.isdbt.transGuardMode = BODS_IsdbtOfdmMode_eAutoBrazil;
                break;
            case NEXUS_FrontendOfdmModeGuard_eManual:
                odsParam.acquireParams.isdbt.transGuardMode = BODS_IsdbtOfdmMode_eManual;
                odsParam.acquireParams.isdbt.guardInterval = pSettings->modeSettings.guardInterval;
                if((pSettings->modeSettings.mode>NEXUS_FrontendOfdmTransmissionMode_e1k) && (pSettings->modeSettings.mode<NEXUS_FrontendOfdmTransmissionMode_e16k))
                    odsParam.acquireParams.isdbt.transmissionMode = pSettings->modeSettings.mode;
                else {
                    BDBG_MSG((" Unsupported ISDBT Transmission mode."));
                    rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
                }
                break;
            default:
                BDBG_MSG((" Unsupported transGuardMode."));
                rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
            }
        }
        else {
            odsParam.acquireParams.isdbt.transGuardMode = BODS_IsdbtOfdmMode_eAutoBrazil;
        }

        odsParam.acquireParams.isdbt.tmccAcquire = true; /* CHECK THIS. */

        if(pSettings->bert.enabled){
            odsParam.bertEnable = true;
            odsParam.bertHeaderLength = pSettings->bert.header;
            odsParam.bertPolynomial = pSettings->bert.polynomial;
        }
        else
            odsParam.bertEnable = false;
        break;
    default:
        BDBG_ERR((" Unsupported mode."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
        break;
    }

    rc = BODS_SetAcquireParams( pDevice->ods_chn[chn_num], &odsParam );
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if (pDevice->tnr[chn_num])
    {
        rc = BTNR_GetSettings(pDevice->tnr[chn_num], &tnrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        if(pSettings->mode == NEXUS_FrontendOfdmMode_eIsdbt){
            tnrSettings.std = BTNR_Standard_eIsdbt;
        }
        else {
            BDBG_WRN((" Unsupported mode."));
        }
        tnrSettings.bandwidth = pSettings->bandwidth;

        rc = BTNR_SetSettings(pDevice->tnr[chn_num], &tnrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        pDevice->count[chn_num] = 0;
        pDevice->acquireInProgress[chn_num] = true;
        pDevice->lastAcquisitionMode[chn_num] = pSettings->acquisitionMode;

        rc = BTNR_SetTunerRfFreq(pDevice->tnr[chn_num], pSettings->frequency, BTNR_TunerMode_eDigital);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    if ( pSettings->acquisitionMode != NEXUS_FrontendOfdmAcquisitionMode_eScan) {
        switch ( pSettings->mode )
        {
        case NEXUS_FrontendOfdmMode_eIsdbt:
            rc = BODS_Acquire(pDevice->ods_chn[chn_num], &odsParam);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            break;
        default:
            break;
        }
    }

    pDevice->last_ofdm[chn_num] = *pSettings;
    return BERR_SUCCESS;
retrack:
    pDevice->last_ofdm[chn_num].frequency = temp_frequency;
    pDevice->last_ofdm[chn_num].mode = temp_mode;
done:
    NEXUS_Frontend_P_3472_UnTune(handle);
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
    NEXUS_3472 *pDevice = NULL;
    BDBG_ASSERT(handle != NULL);

    pDevice = handle->pDevice;
    if (!pDevice) { rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto done; }
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);

    pSettings->updateGainCallback = pDevice->updateGainCallbackDesc;
    pSettings->agcValue = pDevice->deviceSettings.agcValue;
    pSettings->rfInput = pDevice->deviceSettings.rfInput;
    pSettings->rfDaisyChain = pDevice->deviceSettings.rfDaisyChain;
    pSettings->enableRfLoopThrough = pDevice->deviceSettings.enableRfLoopThrough;
    pSettings->terrestrial = pDevice->deviceSettings.terrestrial;
done:
    return rc;
}

NEXUS_Error NEXUS_FrontendDevice_Set3461Settings(NEXUS_FrontendDeviceHandle handle, const NEXUS_FrontendDevice3461Settings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3472 *pDevice = NULL;
    BHAB_ConfigSettings habConfigSettings;

    pDevice = handle->pDevice;
    if (!pDevice) { rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto done; }
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);

    if(pDevice->isInternalSetSettings  || (!(pDevice->pGenericDeviceHandle->openPending) && !(pDevice->pGenericDeviceHandle->openFailed))){
        pDevice->isInternalSetSettings = false;

        rc = BHAB_GetConfigSettings(pDevice->hab, &habConfigSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        habConfigSettings.daisyChain = pSettings->rfDaisyChain;
        habConfigSettings.enableLoopThrough = pSettings->enableRfLoopThrough;

        habConfigSettings.rfInputMode = pSettings->rfInput;
        habConfigSettings.tnrApplication = pSettings->terrestrial;

        rc = BHAB_SetConfigSettings(pDevice->hab, &habConfigSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        pDevice->deviceSettings.rfDaisyChain = pSettings->rfDaisyChain;
        pDevice->deviceSettings.enableRfLoopThrough = pSettings->enableRfLoopThrough;
        pDevice->deviceSettings.rfInput = pSettings->rfInput;
        pDevice->deviceSettings.terrestrial = pSettings->terrestrial;
    }
    else if(pDevice->pGenericDeviceHandle->openPending && !(pDevice->pGenericDeviceHandle->openFailed)){

        pDevice->settingsInitialized = true;
        pDevice->deviceSettings.enableRfLoopThrough = pSettings->enableRfLoopThrough;
        pDevice->deviceSettings.agcValue = pSettings->agcValue;
        pDevice->deviceSettings.rfInput = pSettings->rfInput;
        pDevice->deviceSettings.rfDaisyChain = pSettings->rfDaisyChain;
        pDevice->deviceSettings.enableRfLoopThrough = pSettings->enableRfLoopThrough;
        pDevice->deviceSettings.terrestrial = pSettings->terrestrial;

        if(pDevice->deviceSettings.terrestrial == true)
            pDevice->pGenericDeviceHandle->application = NEXUS_FrontendDeviceApplication_eTerrestrial;
        else if(pDevice->deviceSettings.terrestrial == false)
            pDevice->pGenericDeviceHandle->application = NEXUS_FrontendDeviceApplication_eCable;
    }
    else{
        BDBG_ERR(("Device open failed."));
        rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    }
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_3472_RequestOfdmAsyncStatus(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3472 *pDevice;
    NEXUS_3472Channel *pChannel;
    unsigned chn_num;
    NEXUS_GainParameters params;
    NEXUS_InternalGainSettings settings;
    NEXUS_ExternalGainSettings externalGainSettings;

    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3472Channel *)handle;
    pDevice = pChannel->pDevice;
    chn_num = pChannel->chn_num;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);

    if (chn_num >= NEXUS_3472_MAX_OFDM_CHN)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    if(pDevice->pGenericDeviceHandle->parent){

        params.rfInput = pDevice->pGenericDeviceHandle->linkSettings.rfInput;
        params.accumulateTillRootDevice = true;
        params.frequency = pDevice->last_ofdm[chn_num].frequency;

        rc = NEXUS_FrontendDevice_P_GetInternalGain(pDevice->pGenericDeviceHandle->parent, &params, &settings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        externalGainSettings.totalGain = settings.totalVariableGain;

        rc = NEXUS_FrontendDevice_P_SetExternalGain(pDevice->pGenericDeviceHandle, &externalGainSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    rc = BODS_RequestSelectiveAsyncStatus(pDevice->ods_chn[chn_num], BODS_SelectiveAsyncStatusType_eIsdbt);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}

static NEXUS_FrontendOfdmTransmissionMode NEXUS_Frontend_P_OdsToTransmissionMode(BODS_IsdbtTransmissionMode mode)
{
    switch ( mode )
    {
    case BODS_IsdbtTransmissionMode_e2K:
        return NEXUS_FrontendOfdmTransmissionMode_e2k;
    case BODS_IsdbtTransmissionMode_e4K:
        return NEXUS_FrontendOfdmTransmissionMode_e4k;
    case BODS_IsdbtTransmissionMode_e8K:
        return NEXUS_FrontendOfdmTransmissionMode_e8k;
    default:
        BDBG_WRN(("Unrecognized transmission mode."));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return BODS_IsdbtTransmissionMode_e8K;
    }
}

static NEXUS_FrontendOfdmGuardInterval NEXUS_Frontend_P_OdsToGuardInterval(BODS_IsdbtGuardInterval guard)
{
    switch ( guard )
    {
    case BODS_IsdbtGuardInterval_e1_4:
        return NEXUS_FrontendOfdmGuardInterval_e1_4;
    case BODS_IsdbtGuardInterval_e1_8:
        return NEXUS_FrontendOfdmGuardInterval_e1_8;
    case BODS_IsdbtGuardInterval_e1_16:
        return NEXUS_FrontendOfdmGuardInterval_e1_16;
    case BODS_IsdbtGuardInterval_e1_32:
        return NEXUS_FrontendOfdmGuardInterval_e1_32;
    default:
        BDBG_WRN(("Unrecognized guard interval (%d) reported by BODS", guard));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return NEXUS_FrontendOfdmGuardInterval_e1_4;
    }
}

static NEXUS_FrontendOfdmTimeInterleaving NEXUS_Frontend_P_ODSToTimeInterleaving(BODS_IsdbtTimeInterleaving magnum)
{
    switch (magnum) {
    default:
        BDBG_WRN(("Unknown Magnum time interleaving %d, defaulting to 0x", magnum));
        /* fall-through */
    case BODS_IsdbtTimeInterleaving_e0x:
        return NEXUS_FrontendOfdmTimeInterleaving_e0x;
    case BODS_IsdbtTimeInterleaving_e1x:
        return NEXUS_FrontendOfdmTimeInterleaving_e1x;
    case BODS_IsdbtTimeInterleaving_e2x:
        return NEXUS_FrontendOfdmTimeInterleaving_e2x;
    case BODS_IsdbtTimeInterleaving_e3x:
        return NEXUS_FrontendOfdmTimeInterleaving_e3x;
    }
}

static NEXUS_FrontendOfdmModulation NEXUS_Frontend_P_ODSToModulation( BODS_IsdbtModulation magnum )
{
    switch (magnum) {
    case BODS_IsdbtModulation_e64Qam:
        return NEXUS_FrontendOfdmModulation_eQam64;
    case BODS_IsdbtModulation_e16Qam:
        return NEXUS_FrontendOfdmModulation_eQam16;
    case BODS_IsdbtModulation_eDqpsk:
        return NEXUS_FrontendOfdmModulation_eDqpsk;
    default:
        BDBG_WRN(("Unrecognized ofdm modulation %d, defaulting to QPSK", magnum));
        /* fall-through */
    case BODS_IsdbtModulation_eQpsk:
        return NEXUS_FrontendOfdmModulation_eQpsk;
    }
}

static NEXUS_FrontendOfdmCodeRate NEXUS_Frontend_P_ODSToCodeRate( BODS_IsdbtCodeRate magnum )
{
    switch (magnum) {
    case BODS_IsdbtCodeRate_e1_2:
        return NEXUS_FrontendOfdmCodeRate_e1_2;
    case BODS_IsdbtCodeRate_e2_3:
        return NEXUS_FrontendOfdmCodeRate_e2_3;
    case BODS_IsdbtCodeRate_e3_4:
        return NEXUS_FrontendOfdmCodeRate_e3_4;
    case BODS_IsdbtCodeRate_e5_6:
        return NEXUS_FrontendOfdmCodeRate_e5_6;
    case BODS_IsdbtCodeRate_e7_8:
        return NEXUS_FrontendOfdmCodeRate_e7_8;
    default:
        return NEXUS_FrontendOfdmCodeRate_eMax;
    }
}

static NEXUS_Error NEXUS_Frontend_P_3472_GetOfdmAsyncStatus(void *handle, NEXUS_FrontendOfdmStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_3472 *pDevice;
    NEXUS_3472Channel *pChannel;
    unsigned chn_num;

    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3472Channel *)handle;
    pDevice = pChannel->pDevice;
    chn_num = pChannel->chn_num;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);

    if (chn_num >= NEXUS_3472_MAX_OFDM_CHN)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    BKNI_Memset(&pDevice->odsStatus, 0, sizeof(pDevice->odsStatus));

    rc = BODS_GetSelectiveAsyncStatus(pDevice->ods_chn[chn_num], BODS_SelectiveAsyncStatusType_eIsdbt, &pDevice->odsStatus);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pStatus->receiverLock = pDevice->odsStatus.status.isdbt.receiverLock;
    pStatus->fecLock = pDevice->odsStatus.status.isdbt.fecLock;
    pStatus->noSignalDetected = pDevice->odsStatus.status.isdbt.noSignalDetected;
    pStatus->transmissionMode = NEXUS_Frontend_P_OdsToTransmissionMode(pDevice->odsStatus.status.isdbt.transmissionMode);
    pStatus->guardInterval = NEXUS_Frontend_P_OdsToGuardInterval(pDevice->odsStatus.status.isdbt.guardInterval);
    /*pStatus->gainOffset = pDevice->odsStatus.status.isdbt.gainOffset;*/
    pStatus->carrierOffset = pDevice->odsStatus.status.isdbt.carrierOffset;
    pStatus->timingOffset = pDevice->odsStatus.status.isdbt.timingOffset;
    pStatus->signalStrength = pDevice->odsStatus.status.isdbt.signalStrength/10;
    pStatus->snr = pDevice->odsStatus.status.isdbt.snr*100/256;
    pStatus->spectrumInverted = pDevice->odsStatus.status.isdbt.spectrumInverted;
    pStatus->reacquireCount = pDevice->odsStatus.status.isdbt.reacqCount;
    pStatus->ews = pDevice->odsStatus.status.isdbt.ews;
    pStatus->partialReception = pDevice->odsStatus.status.isdbt.partialReception;
    pStatus->fecCorrectedBlocks = pDevice->odsStatus.status.isdbt.layerAStatus.rsCorrectedBlocks + pDevice->odsStatus.status.isdbt.layerBStatus.rsCorrectedBlocks;
    pStatus->fecUncorrectedBlocks = pDevice->odsStatus.status.isdbt.layerAStatus.rsUncorrectedBlocks + pDevice->odsStatus.status.isdbt.layerBStatus.rsUncorrectedBlocks;
    pStatus->fecCleanBlocks = pDevice->odsStatus.status.isdbt.layerAStatus.rsCleanBlocks + pDevice->odsStatus.status.isdbt.layerBStatus.rsCleanBlocks;

    pStatus->modulationA =NEXUS_Frontend_P_ODSToModulation(pDevice->odsStatus.status.isdbt.layerAStatus.modulation);
    pStatus->codeRateA = NEXUS_Frontend_P_ODSToCodeRate(pDevice->odsStatus.status.isdbt.layerAStatus.codeRate);
    pStatus->timeInterleavingA = NEXUS_Frontend_P_ODSToTimeInterleaving(pDevice->odsStatus.status.isdbt.layerAStatus.timeInterleaving);
    pStatus->numSegmentsA = pDevice->odsStatus.status.isdbt.layerAStatus.numSegments;
    pStatus->fecCorrectedBlocksA = pDevice->odsStatus.status.isdbt.layerAStatus.rsCorrectedBlocks;
    pStatus->fecUncorrectedBlocksA = pDevice->odsStatus.status.isdbt.layerAStatus.rsUncorrectedBlocks;
    pStatus->fecCleanBlocksA = pDevice->odsStatus.status.isdbt.layerAStatus.rsCleanBlocks;
    pStatus->signalLevelPercentA = pDevice->odsStatus.status.isdbt.layerAStatus.signalLevelPercent;
    pStatus->signalQualityPercentA = pDevice->odsStatus.status.isdbt.layerAStatus.signalQualityPercent;

    pStatus->modulationB =NEXUS_Frontend_P_ODSToModulation(pDevice->odsStatus.status.isdbt.layerBStatus.modulation);
    pStatus->codeRateB = NEXUS_Frontend_P_ODSToCodeRate(pDevice->odsStatus.status.isdbt.layerBStatus.codeRate);
    pStatus->timeInterleavingB = NEXUS_Frontend_P_ODSToTimeInterleaving(pDevice->odsStatus.status.isdbt.layerBStatus.timeInterleaving);
    pStatus->numSegmentsB = pDevice->odsStatus.status.isdbt.layerBStatus.numSegments;
    pStatus->fecCorrectedBlocksB = pDevice->odsStatus.status.isdbt.layerBStatus.rsCorrectedBlocks;
    pStatus->fecUncorrectedBlocksB = pDevice->odsStatus.status.isdbt.layerBStatus.rsUncorrectedBlocks;
    pStatus->fecCleanBlocksB = pDevice->odsStatus.status.isdbt.layerBStatus.rsCleanBlocks;
    pStatus->signalLevelPercentB = pDevice->odsStatus.status.isdbt.layerBStatus.signalLevelPercent;
    pStatus->signalQualityPercentB = pDevice->odsStatus.status.isdbt.layerBStatus.signalQualityPercent;

    pStatus->modulationC =NEXUS_Frontend_P_ODSToModulation(pDevice->odsStatus.status.isdbt.layerCStatus.modulation);
    pStatus->codeRateC = NEXUS_Frontend_P_ODSToCodeRate(pDevice->odsStatus.status.isdbt.layerCStatus.codeRate);
    pStatus->timeInterleavingC = NEXUS_Frontend_P_ODSToTimeInterleaving(pDevice->odsStatus.status.isdbt.layerCStatus.timeInterleaving);
    pStatus->numSegmentsC = pDevice->odsStatus.status.isdbt.layerCStatus.numSegments;
    pStatus->fecCorrectedBlocksC = pDevice->odsStatus.status.isdbt.layerCStatus.rsCorrectedBlocks;
    pStatus->fecUncorrectedBlocksC = pDevice->odsStatus.status.isdbt.layerCStatus.rsUncorrectedBlocks;
    pStatus->fecCleanBlocksC = pDevice->odsStatus.status.isdbt.layerCStatus.rsCleanBlocks;
    pStatus->signalLevelPercentC = pDevice->odsStatus.status.isdbt.layerCStatus.signalLevelPercent;
    pStatus->signalQualityPercentC = pDevice->odsStatus.status.isdbt.layerCStatus.signalQualityPercent;


    pStatus->settings = pDevice->last_ofdm[chn_num];

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_3472_GetOfdmStatus(void *handle, NEXUS_FrontendOfdmStatus *pStatus)
{
    NEXUS_Error rc = BERR_SUCCESS;
    NEXUS_3472 *pDevice;
    unsigned j=0, chn_num;
    uint32_t buf=0;
    NEXUS_3472Channel *pChannel;

    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3472Channel *)handle;
    pDevice = pChannel->pDevice;
    chn_num = pChannel->chn_num;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);

    if (chn_num >= NEXUS_3472_MAX_OFDM_CHN)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));


    if(pDevice->isPoweredOn[pChannel->chn_num]) {
        pDevice->isInternalAsyncStatusCall[pChannel->chn_num] = true;

        rc = NEXUS_Frontend_P_3472_RequestOfdmAsyncStatus(pChannel);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        for(j=0; j < 10; j++) {

            BKNI_Sleep(100);

            rc = BHAB_ReadRegister(pDevice->hab, BCHP_LEAP_CTRL_SW_SPARE0 , &buf);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            if(((chn_num == 0) && (buf & BHAB_THD_CHN0_STATUS_RDY)) || ((chn_num == 1) && (buf & BHAB_THD_CHN1_STATUS_RDY)))
            {
                rc = NEXUS_Frontend_P_3472_GetOfdmAsyncStatus(pChannel, pStatus);
                if(rc){rc = BERR_TRACE(rc); goto done;}
                pDevice->isInternalAsyncStatusCall[pChannel->chn_num] = false;
                break;
            }
            else {
                continue;
            }
        }

        pStatus->settings = pDevice->last_ofdm[chn_num];
    }
    else{
        BDBG_MSG(("The ofdm core is Powered Off"));
    }

done:
    pDevice->isInternalAsyncStatusCall[pChannel->chn_num] = false;
    return rc;
}

NEXUS_Error NEXUS_Frontend_P_3472_ReapplyTransportSettings(void *handle)
{
    NEXUS_3472Channel *pChannel = (NEXUS_3472Channel *)handle;
    NEXUS_3472 *pDevice;
    unsigned chn_num;
    NEXUS_Error rc;

    BDBG_ASSERT(pChannel);
    pDevice = (NEXUS_3472 *)pChannel->pDevice;
    BDBG_ASSERT(pDevice);
    chn_num = pChannel->chn_num;
    rc = NEXUS_Frontend_P_SetMtsifConfig(pDevice->frontendHandle[chn_num]);
    if (rc) { return BERR_TRACE(rc); }

    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_3472_RequestIsdbtAsyncStatus(void *handle, NEXUS_FrontendIsdbtStatusType type)
{
    NEXUS_Error rc = BERR_SUCCESS;
    NEXUS_3472 *pDevice;
    NEXUS_3472Channel *pChannel;
    BSTD_UNUSED(type);
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3472Channel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);

    pDevice->isStatusReady[pChannel->chn_num] = false;

    rc = NEXUS_Frontend_P_3472_RequestOfdmAsyncStatus(pChannel);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_3472_GetIsdbtAsyncStatusReady(void *handle, NEXUS_FrontendIsdbtStatusReady *pAsyncStatusReady)
{
    NEXUS_Error rc = BERR_SUCCESS;
    NEXUS_3472 *pDevice;
    NEXUS_3472Channel *pChannel;

    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3472Channel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);

    if(pDevice->isStatusReady[pChannel->chn_num]){
        pAsyncStatusReady->type[NEXUS_FrontendIsdbtStatusType_eBasic] = true;
    }
    else{
        pAsyncStatusReady->type[NEXUS_FrontendIsdbtStatusType_eBasic] = false;
    }

    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_3472_GetIsdbtAsyncStatus(void *handle, NEXUS_FrontendIsdbtStatusType type, NEXUS_FrontendIsdbtStatus *pStatus)
{
    NEXUS_Error rc = BERR_SUCCESS;
    NEXUS_3472 *pDevice;
    unsigned chn_num;
    NEXUS_3472Channel *pChannel;
    BODS_IsdbtStatus  *status;

    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3472Channel *)handle;
    pDevice = pChannel->pDevice;
    chn_num = pChannel->chn_num;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);

    if (chn_num >= NEXUS_3472_MAX_OFDM_CHN)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    BKNI_Memset(&pDevice->odsStatus, 0, sizeof(pDevice->odsStatus));

    pStatus->status.basic.settings = pDevice->last_ofdm[chn_num];

    if(type == NEXUS_FrontendIsdbtStatusType_eBasic){
        rc = BODS_GetSelectiveAsyncStatus(pDevice->ods_chn[chn_num], BODS_SelectiveAsyncStatusType_eIsdbt, &pDevice->odsStatus);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        status = &pDevice->odsStatus.status.isdbt;

        pStatus->status.basic.fecLock = status->fecLock;
        pStatus->status.basic.tmcc.transmissionMode = NEXUS_Frontend_P_OdsToTransmissionMode(status->transmissionMode);
        pStatus->status.basic.tmcc.guardInterval = NEXUS_Frontend_P_OdsToGuardInterval(status->guardInterval);
        pStatus->status.basic.tmcc.ews = status->ews;
        pStatus->status.basic.tmcc.partialReception = status->partialReception;
        pStatus->status.basic.carrierOffset = status->carrierOffset;
        pStatus->status.basic.timingOffset = status->timingOffset;
        pStatus->status.basic.signalStrength = status->signalStrength/10;
        pStatus->status.basic.snr = status->snr*100/256;
        pStatus->status.basic.spectrumInverted = status->spectrumInverted;
        pStatus->status.basic.reacquireCount = status->reacqCount;

        pStatus->status.basic.layerA.modulation = NEXUS_Frontend_P_ODSToModulation(status->layerAStatus.modulation);
        pStatus->status.basic.layerA.codeRate = NEXUS_Frontend_P_ODSToCodeRate(status->layerAStatus.codeRate);
        pStatus->status.basic.layerA.timeInterleaving = NEXUS_Frontend_P_ODSToTimeInterleaving(status->layerAStatus.timeInterleaving);
        pStatus->status.basic.layerA.numSegments = status->layerAStatus.numSegments;
        pStatus->status.basic.layerA.fecBlockCounts.corrected = status->layerAStatus.rsCorrectedBlocks;
        pStatus->status.basic.layerA.fecBlockCounts.uncorrected= status->layerAStatus.rsUncorrectedBlocks;
        pStatus->status.basic.layerA.fecBlockCounts.clean = status->layerAStatus.rsCleanBlocks;
        pStatus->status.basic.layerA.signalLevelPercent = status->layerAStatus.signalLevelPercent;
        pStatus->status.basic.layerA.signalQualityPercent = status->layerAStatus.signalQualityPercent;

        pStatus->status.basic.layerB.modulation =NEXUS_Frontend_P_ODSToModulation(status->layerBStatus.modulation);
        pStatus->status.basic.layerB.codeRate = NEXUS_Frontend_P_ODSToCodeRate(status->layerBStatus.codeRate);
        pStatus->status.basic.layerB.timeInterleaving = NEXUS_Frontend_P_ODSToTimeInterleaving(status->layerBStatus.timeInterleaving);
        pStatus->status.basic.layerB.numSegments = status->layerBStatus.numSegments;
        pStatus->status.basic.layerB.fecBlockCounts.corrected = status->layerBStatus.rsCorrectedBlocks;
        pStatus->status.basic.layerB.fecBlockCounts.uncorrected = status->layerBStatus.rsUncorrectedBlocks;
        pStatus->status.basic.layerB.fecBlockCounts.clean = status->layerBStatus.rsCleanBlocks;
        pStatus->status.basic.layerB.signalLevelPercent = status->layerBStatus.signalLevelPercent;
        pStatus->status.basic.layerB.signalQualityPercent = status->layerBStatus.signalQualityPercent;

        pStatus->status.basic.layerC.modulation =NEXUS_Frontend_P_ODSToModulation(status->layerCStatus.modulation);
        pStatus->status.basic.layerC.codeRate = NEXUS_Frontend_P_ODSToCodeRate(status->layerCStatus.codeRate);
        pStatus->status.basic.layerC.timeInterleaving = NEXUS_Frontend_P_ODSToTimeInterleaving(status->layerCStatus.timeInterleaving);
        pStatus->status.basic.layerC.numSegments = status->layerCStatus.numSegments;
        pStatus->status.basic.layerC.fecBlockCounts.corrected = status->layerCStatus.rsCorrectedBlocks;
        pStatus->status.basic.layerC.fecBlockCounts.uncorrected = status->layerCStatus.rsUncorrectedBlocks;
        pStatus->status.basic.layerC.fecBlockCounts.clean = status->layerCStatus.rsCleanBlocks;
        pStatus->status.basic.layerC.signalLevelPercent = status->layerCStatus.signalLevelPercent;
        pStatus->status.basic.layerC.signalQualityPercent = status->layerCStatus.signalQualityPercent;

        if(pStatus->status.basic.settings.bert.enabled == true){
            pStatus->status.basic.layerA.bert.locked = status->layerAStatus.bertSync;
            pStatus->status.basic.layerA.bert.errorRate.count = status->layerAStatus.bertErrorBits;
            pStatus->status.basic.layerA.bert.errorRate.total = status->layerAStatus.bertTotalBits;

            if(pStatus->status.basic.layerA.bert.locked){
                if(status->layerAStatus.bertErrorBits <= status->layerAStatus.bertTotalBits){
                    /* This is equvivalent to left shifting the numerator by 15 and right shifting the denominator by 16. */
                    if(status->layerAStatus.bertTotalBits/65536)
                        pStatus->status.basic.layerA.bert.errorRate.rate = (status->layerAStatus.bertErrorBits * 32768)/(status->layerAStatus.bertTotalBits/65536);
                }
                else{
                    NEXUS_Frontend_P_3472_ResetStatus(handle);
                }
            }

            pStatus->status.basic.layerB.bert.locked = status->layerBStatus.bertSync;
            pStatus->status.basic.layerB.bert.errorRate.count = status->layerBStatus.bertErrorBits;
            pStatus->status.basic.layerB.bert.errorRate.total = status->layerBStatus.bertTotalBits;
            if(pStatus->status.basic.layerB.bert.locked){
                if(status->layerBStatus.bertErrorBits <= status->layerBStatus.bertTotalBits){
                    if(status->layerBStatus.bertTotalBits/65536)
                        pStatus->status.basic.layerB.bert.errorRate.rate = (status->layerBStatus.bertErrorBits * 32768)/(status->layerBStatus.bertTotalBits/65536);
                }
                else{
                    NEXUS_Frontend_P_3472_ResetStatus(handle);
                }
            }

            pStatus->status.basic.layerC.bert.locked = status->layerCStatus.bertSync;
            pStatus->status.basic.layerC.bert.errorRate.count = status->layerCStatus.bertErrorBits;
            pStatus->status.basic.layerC.bert.errorRate.total = status->layerCStatus.bertTotalBits;
            if(pStatus->status.basic.layerC.bert.locked){
                if(status->layerCStatus.bertErrorBits <= status->layerCStatus.bertTotalBits){
                    if(status->layerCStatus.bertTotalBits/65536)
                        pStatus->status.basic.layerC.bert.errorRate.rate = (status->layerCStatus.bertErrorBits * 32768)/(status->layerCStatus.bertTotalBits/65536);
                }
                else{
                    NEXUS_Frontend_P_3472_ResetStatus(handle);
                }
            }
        }
    }

    pDevice->isStatusReady[pChannel->chn_num] = false;
done:
    return rc;

}

static NEXUS_Error NEXUS_FrontendDevice_P_3472_GetStatus(void *handle, NEXUS_FrontendDeviceStatus *pStatus)
{
    NEXUS_Error rc = BERR_SUCCESS;
    NEXUS_3472 *pDevice;
    BHAB_AvsData avsData;
    NEXUS_3472Channel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_3472Channel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_3472);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    rc = BHAB_GetAvsData(pDevice->hab, &avsData);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if(avsData.enabled){
        pStatus->avs.enabled = true;
        pStatus->avs.voltage = avsData.voltage;
        pStatus->temperature = avsData.temperature;
    }
    else
        pStatus->avs.enabled = false;

    BDBG_MSG(("pStatus->avs.enabled = %d", pStatus->avs.enabled));
    BDBG_MSG(("pStatus->avs.voltage = %d", pStatus->avs.voltage));
    BDBG_MSG(("pStatus->temperature = %d", pStatus->temperature));
done:
    return rc;
}


#if 0

static NEXUS_Error NEXUS_FrontendDevice_P_3472_setCrystalDaisySettings(const NEXUS_FrontendDevice3461OpenSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    uint32_t sb1, addr;
    uint8_t data=0, i=0,  buf[4];
    BREG_I2C_Handle i2cHandle;

    i2cHandle = NEXUS_I2c_GetRegHandle(pSettings->i2cDevice, NULL);

    addr = BCHP_TM_PWRDN;
    sb1 = ((addr & 0x000000FF) << 24 |
           (addr & 0x0000FF00) << 8 |
           (addr & 0x00FF0000) >> 8 |
           (addr & 0xFF000000) >> 24 );

   /* set READ_RBUS for read mode  and no increment mode. */
   data = 0x5;
   rc = BREG_I2C_Write(i2cHandle, pSettings->i2cAddr, CSR_CONFIG, &data, 1);
   if(rc){rc = BERR_TRACE(rc); goto done;}
   rc = BREG_I2C_Write(i2cHandle, pSettings->i2cAddr, CSR_RBUS_ADDR0, (uint8_t *)&sb1, 4);
   if(rc){rc = BERR_TRACE(rc); goto done;}

    /* poll the busy bit to make sure the transaction is completed */
   for(i=0; i < 5; i++){
        rc = BREG_I2C_Read(i2cHandle, pSettings->i2cAddr, CSR_STATUS, &data, 1);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        if ((data & (1 << CSR_STATUS_BUSY_SHIFT)) == 0)
            break;
   }

   if(i==5)
       BDBG_WRN(("Read transaction not finished\n"));

    /* read the data */
    rc = BREG_I2C_Read(i2cHandle, pSettings->i2cAddr, CSR_RBUS_DATA0, buf, 4);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    /* set READ_RBUS to the reset value for write mode */
    data = 0x4;
    rc = BREG_I2C_Write(i2cHandle, pSettings->i2cAddr, CSR_CONFIG, &data, 1);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if(!pSettings->crystalSettings.enableDaisyChain){
        buf[3] |= 0x2;
        rc = BREG_I2C_Write(i2cHandle, pSettings->i2cAddr, CSR_RBUS_DATA0, buf, 4);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    else{
        buf[3] &= ~0x2;
        rc = BREG_I2C_Write(i2cHandle, pSettings->i2cAddr, CSR_RBUS_DATA0, buf, 4);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    /* poll the busy bit to make sure the transaction is completed */
    for(i=0; i < 5; i++){
        rc = BREG_I2C_Read(i2cHandle, pSettings->i2cAddr, CSR_STATUS, &data, 1);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        if ((data & (1 << CSR_STATUS_BUSY_SHIFT)) == 0)
            break;
    }

    if(i==5)
       BDBG_WRN(("Read transaction not finished\n"));

    data = CSR_CONFIG_READ_RBUS_WRITE;
    rc = BREG_I2C_Write(i2cHandle, pSettings->i2cAddr, CSR_CONFIG, &data, 1);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;
}


#endif

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
*   API name: Frontend 31xx
*    APIs to open, close, and setup initial settings for a BCM31xx
*    Frontend Device.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#include "nexus_frontend_module.h"
#include "nexus_platform_features.h"
#include "nexus_i2c.h"
#include "priv/nexus_i2c_priv.h"
#include "priv/nexus_gpio_priv.h"
#include "btnr.h"
#ifdef NEXUS_FRONTEND_3117
#include "btnr_3117ib.h"
#include "btnr_3117ob.h"
#include "bhab_3117.h"
#include "bads_3117.h"
#define BCHIP_HAS_AUS 1
#endif
#ifdef NEXUS_FRONTEND_3114
#include "btnr_3114ib.h"
#include "btnr_3114ob.h"
#include "bhab_3114.h"
#include "bads_3114.h"
#endif
#ifdef NEXUS_FRONTEND_3112
#include "btnr_3112ib.h"
#include "bhab_3112.h"
#include "bads_3112.h"
#endif
#ifdef NEXUS_FRONTEND_3109
#include "btnr_3109ib.h"
#include "bhab_3109.h"
#include "bads_3109.h"
#endif
#ifdef NEXUS_FRONTEND_31xx_OOB
#define BCHIP_HAS_AOB 1
#endif

#include "bhab.h"
#include "bchp_31xx.h"
#include "bads.h"
#include "bhab_31xx_fw.h"
#ifdef BCHIP_HAS_AOB
#include "baob.h"
#endif
#ifdef BCHIP_HAS_AUS
#include "baus.h"
#endif

BDBG_MODULE(nexus_frontend_31xx);

BDBG_OBJECT_ID(NEXUS_31xx);

#define BCHIP_3117 0x3117
#define BCHIP_3114 0x3114
#define BCHIP_3112 0x3112
#define BCHIP_3109 0x3109

#define NEXUS_MAX_31xx_FRONTENDS 3

/* Currently there is only one ADS channel on 3117/3114/3112/3109 */
#define NEXUS_31xx_ADS_CHN 0

#define NEXUS_MAX_31xx_ADSCHN 1

/* The OOB channel number on 3117 */
#define NEXUS_31xx_OOB_CHN      (NEXUS_31xx_ADS_CHN + 1)
#define NEXUS_31xx_AUS_CHN      (NEXUS_31xx_OOB_CHN + 1)

/* The Upstream frontend will not have a tuner hence 1 is subtracted. */
#define NEXUS_MAX_31xx_TUNERS  (NEXUS_MAX_31xx_FRONTENDS - 1)

/***************************************************************************
 * Internal Module Structure
 ***************************************************************************/
typedef struct NEXUS_31xx
{
    BDBG_OBJECT(NEXUS_31xx)
    BLST_S_ENTRY(NEXUS_31xx) node;
    uint16_t  chipId;
    BHAB_Handle hab;
    unsigned    numfrontends;
    BTNR_Handle tnr[NEXUS_MAX_31xx_TUNERS];
    NEXUS_FrontendHandle    frontendHandle[NEXUS_MAX_31xx_FRONTENDS];
    BADS_Handle ads;
    BADS_ChannelHandle  ads_chn[NEXUS_MAX_31xx_ADSCHN];
    NEXUS_FrontendQamSettings   qam[NEXUS_MAX_31xx_ADSCHN];
    NEXUS_FrontendQamSettings   last_ads[NEXUS_MAX_31xx_ADSCHN];
#ifdef BCHIP_HAS_AOB
    BAOB_Handle aob;
    NEXUS_FrontendOutOfBandSettings oob;
    NEXUS_FrontendOutOfBandSettings last_aob;
#endif
#ifdef BCHIP_HAS_AUS
    BAUS_Handle aus;
    NEXUS_FrontendUpstreamSettings ups;
    NEXUS_FrontendUpstreamSettings last_ups;
#endif
    BKNI_EventHandle isrEvent;
    NEXUS_EventCallbackHandle isrEventCallback;
    NEXUS_IsrCallbackHandle lockAppCallback[NEXUS_MAX_31xx_FRONTENDS];
    NEXUS_IsrCallbackHandle updateGainAppCallback;
    NEXUS_IsrCallbackHandle asyncStatusAppCallback[NEXUS_MAX_31xx_FRONTENDS];
    NEXUS_FrontendLockStatus prevStatus[NEXUS_MAX_31xx_FRONTENDS];
    uint16_t revId;
#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT
    bool isMaster;
#endif

    NEXUS_FrontendDevice *pGenericDeviceHandle;
    NEXUS_FrontendDevice31xxOpenSettings openSettings;
    NEXUS_FrontendDevice31xxSettings deviceSettings;
    NEXUS_ThreadHandle deviceOpenThread;
    bool isInternalSetSettings;
    bool settingsInitialized;
    bool acquireInProgress;
    unsigned count;
    bool isPoweredOn[NEXUS_MAX_31xx_FRONTENDS];
    bool isTunerPoweredOn[NEXUS_MAX_31xx_TUNERS];
} NEXUS_31xx;

static BLST_S_HEAD(devList, NEXUS_31xx) g_31xxDevList = BLST_S_INITIALIZER(g_31xxDevList);

typedef struct NEXUS_31xxChannel
{
    unsigned chn_num; /* channel number */
    NEXUS_31xx *pDevice; /* 31xx device*/
} NEXUS_31xxChannel;

/***************************************************************************
 * Module callback functions for tuning
 ***************************************************************************/
static void NEXUS_Frontend_P_31xx_UninstallCallbacks(void *handle);
static void NEXUS_Frontend_P_31xx_Close(NEXUS_FrontendHandle handle);
static NEXUS_Error NEXUS_Frontend_P_31xx_TuneQam(void *handle, const NEXUS_FrontendQamSettings *pSettings);
static void NEXUS_Frontend_P_31xx_UnTuneQam(void *handle);
static NEXUS_Error NEXUS_Frontend_P_31xx_GetQamStatus(void *handle, NEXUS_FrontendQamStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_31xx_GetQamAsyncStatus(void *handle, NEXUS_FrontendQamStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_31xx_RequestQamAsyncStatus(void *handle);
static NEXUS_Error NEXUS_Frontend_P_31xx_ReadSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length, size_t *pNumRead);
static void NEXUS_Frontend_P_31xx_ResetAdsStatus(void *handle);
static NEXUS_Error NEXUS_Frontend_P_31xx_GetQamScanStatus(void *handle, NEXUS_FrontendQamScanStatus *pScanStatus);
#ifdef BCHIP_HAS_AOB
static void NEXUS_Frontend_P_31xx_UnTuneOob(void *handle);
static NEXUS_Error NEXUS_Frontend_P_31xx_TuneOob(void *handle, const NEXUS_FrontendOutOfBandSettings *pSettings);
static NEXUS_Error NEXUS_Frontend_P_31xx_GetOobStatus(void *handle, NEXUS_FrontendOutOfBandStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_31xx_GetOobAsyncStatus(void *handle, NEXUS_FrontendOutOfBandStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_31xx_RequestOobAsyncStatus(void *handle);
static void NEXUS_Frontend_P_31xx_ResetOobStatus(void *handle);
#endif
#ifdef BCHIP_HAS_AUS
static void NEXUS_Frontend_P_31xx_UnTuneUpstream(void *handle);
static NEXUS_Error NEXUS_Frontend_P_31xx_TuneUpstream(void *handle, const NEXUS_FrontendUpstreamSettings *pSettings);
static NEXUS_Error NEXUS_Frontend_P_31xx_GetUpstreamStatus(void *handle, NEXUS_FrontendUpstreamStatus *pStatus);
#endif
static void NEXUS_Frontend_P_31xx_GetType(void *handle, NEXUS_FrontendType *type);
static NEXUS_Error NEXUS_Frontend_P_31xx_GetFastStatus(void *handle, NEXUS_FrontendFastStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_31xx_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings);
static void NEXUS_FrontendDevice_P_31xx_Close(void *handle);
static NEXUS_Error NEXUS_FrontendDevice_P_31xx_Standby(void * handle, const NEXUS_StandbySettings *pSettings);

/****************************************************************************************************/
static uint16_t NEXUS_Frontend_P_Get31xxRev(const NEXUS_FrontendDevice31xxOpenSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BREG_I2C_Handle i2cHandle;
    uint8_t buf[2];
    uint16_t revId=0;

    i2cHandle = NEXUS_I2c_GetRegHandle(pSettings->i2cDevice, NULL);

    buf[0]= 0x15;
    rc = BREG_I2C_Write(i2cHandle, pSettings->i2cAddr, DEVICE(SH_SFR_H_LOCAL_ADR), buf, 1);
    if(rc){rc = BERR_TRACE(rc); goto done;}
    rc = BREG_I2C_Read(i2cHandle, pSettings->i2cAddr, DEVICE(SH_SFR_H_LOCAL_DAT), buf, 1);
    if(rc){rc = BERR_TRACE(rc); goto done;}
    revId = buf[0];

    buf[0]= 0x16;
    rc = BREG_I2C_Write(i2cHandle, pSettings->i2cAddr, DEVICE(SH_SFR_H_LOCAL_ADR), buf, 1);
    if(rc){rc = BERR_TRACE(rc); goto done;}
    rc = BREG_I2C_Read(i2cHandle, pSettings->i2cAddr, DEVICE(SH_SFR_H_LOCAL_DAT), buf, 1);
    if(rc){rc = BERR_TRACE(rc); goto done;}
    revId = (revId<<8) | buf[0];

    return revId;
done:
    return 0;
}

static uint16_t NEXUS_Frontend_P_Is31xx(const NEXUS_FrontendDevice31xxOpenSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BREG_I2C_Handle i2cHandle;
    uint8_t buf[2];
    uint16_t chipId=0;

    i2cHandle = NEXUS_I2c_GetRegHandle(pSettings->i2cDevice, NULL);

    if (pSettings->loadAP)
    {
        buf[0]= 0x00;
        buf[1]= 0xff;
        rc = BREG_I2C_Write(i2cHandle, pSettings->i2cAddr, DEVICE(SH_SFR_H_LOCAL_ADR), buf, 2);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        buf[0]= 0x01;
        buf[1]= 0xff;
        rc = BREG_I2C_Write(i2cHandle, pSettings->i2cAddr, DEVICE(SH_SFR_H_LOCAL_ADR), buf, 2);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        buf[0]= 0x00;
        buf[1]= 0x00;
        rc = BREG_I2C_Write(i2cHandle, pSettings->i2cAddr, DEVICE(SH_SFR_H_LOCAL_ADR), buf, 2);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        buf[0]= 0x01;
        buf[1]= 0x00;
        rc = BREG_I2C_Write(i2cHandle, pSettings->i2cAddr, DEVICE(SH_SFR_H_LOCAL_ADR), buf, 2);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    buf[0]= 0x13;
    rc = BREG_I2C_Write(i2cHandle, pSettings->i2cAddr, DEVICE(SH_SFR_H_LOCAL_ADR), buf, 1);
    if(rc){rc = BERR_TRACE(rc); goto done;}
    rc = BREG_I2C_Read(i2cHandle, pSettings->i2cAddr, DEVICE(SH_SFR_H_LOCAL_DAT), buf, 1);
    if(rc){rc = BERR_TRACE(rc); goto done;}
    chipId = buf[0];

    buf[0]= 0x14;
    rc = BREG_I2C_Write(i2cHandle, pSettings->i2cAddr, DEVICE(SH_SFR_H_LOCAL_ADR), buf, 1);
    if(rc){rc = BERR_TRACE(rc); goto done;}
    rc = BREG_I2C_Read(i2cHandle, pSettings->i2cAddr, DEVICE(SH_SFR_H_LOCAL_DAT), buf, 1);
    if(rc){rc = BERR_TRACE(rc); goto done;}
    chipId = (chipId<<8) | buf[0];

    BDBG_MSG(("chipId = 0x%x", chipId));

    return chipId;
done:
    return 0;
}

/***************************************************************************
Summary:
  Probe to see if a BCM31xx device exists with the specified settings
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Probe31xx(const NEXUS_FrontendDevice31xxOpenSettings *pSettings, NEXUS_31xxProbeResults *pResults)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    uint16_t chipVer=0xffff;

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pSettings->i2cDevice);
    BDBG_ASSERT(NULL != pResults);

    pResults->chip.familyId = 0x0;

    pResults->chip.id = (uint32_t)NEXUS_Frontend_P_Is31xx(pSettings);
    if((pResults->chip.id & 0xff00)!= 0x3100)
    {
        BDBG_WRN(("pResults->chip.id = 0x%x", pResults->chip.id));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }
    pResults->chip.familyId = 0x3117;
    chipVer = NEXUS_Frontend_P_Get31xxRev(pSettings);
    BDBG_MSG(("ChipVer = 0x%x", chipVer));
done:
    pResults->chip.version.major = (chipVer >> 8) + 1;
    pResults->chip.version.minor = chipVer & 0xff;

    return rc;
}

/***************************************************************************
Summary:
    Lock callback handler for a 31xx Inband device
 ***************************************************************************/
static void NEXUS_Frontend_P_31xx_callback_isr(void *pParam)
{
    NEXUS_FrontendHandle frontendHandle = NULL;
    NEXUS_31xxChannel *pChannel;
    NEXUS_31xx *pDevice;
    BDBG_ASSERT(NULL != pParam);
    frontendHandle = *((NEXUS_FrontendHandle *)pParam);
    BDBG_ASSERT(NULL != frontendHandle);
    pChannel = frontendHandle->pDeviceHandle;
    BDBG_ASSERT(NULL != pChannel);
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);

    if(pDevice->acquireInProgress){
        pDevice->count++;
    }
    if(pDevice->count == 2){
        pDevice->acquireInProgress = false;
        pDevice->count = 0;
    }

    if ( pDevice->lockAppCallback[pChannel->chn_num] )
    {
        NEXUS_IsrCallback_Fire_isr(pDevice->lockAppCallback[pChannel->chn_num]);
    }
}

/***************************************************************************
Summary:
    Lock callback handler for a 31xx Inband device
 ***************************************************************************/
static void NEXUS_Frontend_P_31xx_AsyncStatusCallback_isr(void *pParam)
{
    NEXUS_FrontendHandle frontendHandle = NULL;
    NEXUS_31xxChannel *pChannel;
    NEXUS_31xx *pDevice;
    BDBG_ASSERT(NULL != pParam);
    frontendHandle = *((NEXUS_FrontendHandle *)pParam);
    BDBG_ASSERT(NULL != frontendHandle);
    pChannel = frontendHandle->pDeviceHandle;
    BDBG_ASSERT(NULL != pChannel);
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);

    if (pDevice->asyncStatusAppCallback[pChannel->chn_num])
    {
        NEXUS_IsrCallback_Fire_isr(pDevice->asyncStatusAppCallback[pChannel->chn_num]);
    }
}

/***************************************************************************
Summary:
    Lock callback handler for a 31xx Inband device
 ***************************************************************************/
static void NEXUS_Frontend_P_31xx_UpdateGainCallback_isr(void *pParam)
{
    NEXUS_31xx *pDevice;
    BDBG_ASSERT(NULL != pParam);
    pDevice = (NEXUS_31xx *)pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);

    if (pDevice->updateGainAppCallback)
    {
        NEXUS_IsrCallback_Fire_isr(pDevice->updateGainAppCallback);
    }
}

/***************************************************************************
Summary:
    Enable/Disable interrupts for a 3117 device
 ***************************************************************************/
static void NEXUS_Frontend_P_31xx_L1_isr(void *pParam1, int pParam2)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHAB_Handle hab;
#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT
    NEXUS_31xx *pDevice, *pIntDevice;
#endif
    BDBG_ASSERT(0 != pParam2);
    hab = (BHAB_Handle)pParam2;
    BDBG_ASSERT(NULL != pParam1);
#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT
    pIntDevice = (NEXUS_31xx *)pParam1;
#else
    BSTD_UNUSED(pParam1);
#endif

    rc = BHAB_HandleInterrupt_isr(hab);
    if(rc){rc = BERR_TRACE(rc);}

#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT
    for ( pDevice = BLST_S_FIRST(&g_31xxDevList); NULL != pDevice; pDevice = BLST_S_NEXT(pDevice, node) )
    {
        BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);

        if ((pDevice->hab != NULL) && (pDevice->openSettings.isrNumber== pIntDevice->openSettings.isrNumber) &&
            (pDevice->openSettings.gpioInterrupt == pIntDevice->openSettings.gpioInterrupt) && (pDevice->hab != hab))
        {
            rc = BHAB_HandleInterrupt_isr(pDevice->hab);
            if(rc){rc = BERR_TRACE(rc);}
        }
    }
#endif

}
/***************************************************************************
Summary:
    Enable/Disable interrupts for a 3117 device
 ***************************************************************************/
static void NEXUS_Frontend_P_31xx_GpioIsrControl_isr(bool enable, void *pParam)
{
    NEXUS_GpioHandle gpioHandle;
    BDBG_ASSERT(NULL != pParam);
    gpioHandle = (NEXUS_GpioHandle)pParam;

    if(enable){
        NEXUS_Gpio_SetInterruptEnabled_isr(gpioHandle, true);
    }
    else {
        NEXUS_Gpio_SetInterruptEnabled_isr(gpioHandle, false);
    }
}

/***************************************************************************
Summary:
    Enable/Disable interrupts for a 3117 device
 ***************************************************************************/
static void NEXUS_Frontend_P_31xx_IsrControl_isr(bool enable, void *pParam)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    int isrnumber;
    BDBG_ASSERT(NULL != pParam);
    isrnumber = (int)pParam;

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
    ISR Event Handler for a 3117 device
 ***************************************************************************/
static void NEXUS_Frontend_P_31xx_IsrEvent(void *pParam)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHAB_Handle hab;
    BDBG_ASSERT(NULL != pParam);
    hab = (BHAB_Handle)pParam;

    if(hab != NULL){
        rc = BHAB_ProcessInterruptEvent(hab);
        if(rc) BERR_TRACE(rc);
    }
}

/***************************************************************************
Summary:
    Initialize all ADS/OOB channels.
 ***************************************************************************/
void NEXUS_Frontend_P_UnInit31xx(NEXUS_31xx *pDevice)
{
    NEXUS_Error rc = BERR_SUCCESS;
    unsigned int i;

    for ( i = 0; i < NEXUS_MAX_31xx_TUNERS && NULL != pDevice->tnr[i]; i++) {
        if(pDevice->tnr[i]){
            rc = BTNR_Close(pDevice->tnr[i]);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->tnr[i] = NULL;
        }
    }
    for ( i = 0; i < NEXUS_MAX_31xx_ADSCHN && NULL != pDevice->ads_chn[i]; i++) {
        if(pDevice->ads_chn[i]){
            if(pDevice->ads_chn[i]){
                BADS_InstallCallback(pDevice->ads_chn[i], BADS_Callback_eLockChange, NULL, NULL);
                BADS_InstallCallback(pDevice->ads_chn[i], BADS_Callback_eUpdateGain, NULL, NULL);
                BADS_InstallCallback(pDevice->ads_chn[i], BADS_Callback_eAsyncStatusReady, NULL, NULL);
            }
            rc = BADS_CloseChannel(pDevice->ads_chn[i]);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            (pDevice->ads_chn[i]) = NULL;
        }
    }
    if (pDevice->ads) {
        rc = BADS_Close(pDevice->ads);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->ads = NULL;
    }
#if BCHIP_HAS_AOB
    if (pDevice->aob){
        BAOB_InstallCallback(pDevice->aob, BAOB_Callback_eLockChange, NULL, NULL);
        BAOB_InstallCallback(pDevice->aob, BAOB_Callback_eAsyncStatusReady, NULL, NULL);

        rc = BAOB_Close(pDevice->aob);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->aob = NULL;
    }
#endif
#ifdef BCHIP_HAS_AUS
    if (pDevice->aus){
        rc = BAUS_Close(pDevice->aus);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->aus = NULL;
    }
#endif
done:
    return;
}


/***************************************************************************
Summary:
    Initialize Hab for a 3117 device
***************************************************************************/
void NEXUS_Frontend_P_UnInit_31xx_Hab(NEXUS_31xx *pDevice)
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
        rc = NEXUS_Gpio_SetSettings(pDevice->openSettings.gpioInterrupt, &gpioSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

#endif
    }

    if(pDevice->isrEvent) pDevice->isrEvent = NULL;
    if(pDevice->isrEventCallback)NEXUS_UnregisterEvent(pDevice->isrEventCallback);
    pDevice->isrEventCallback = NULL;
#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT
    BKNI_EnterCriticalSection();
    BLST_S_REMOVE(&g_31xxDevList, pDevice, NEXUS_31xx, node);
    BKNI_LeaveCriticalSection();
#endif
    if(pDevice->hab) {
        rc = BHAB_Close(pDevice->hab);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    pDevice->hab = NULL;

    if(pDevice->deviceOpenThread)NEXUS_Thread_Destroy(pDevice->deviceOpenThread);
    pDevice->deviceOpenThread = NULL;
done:
    return;
}

static void NEXUS_FrontendDevice_P_31xx_Close(void *handle)
{
    NEXUS_31xx *pDevice;
    unsigned i=0;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_31xx *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);

    if(pDevice->numfrontends){
        for(i = 0; i< NEXUS_MAX_31xx_FRONTENDS; i++){
            if(pDevice->frontendHandle[i])NEXUS_Frontend_P_31xx_Close(pDevice->frontendHandle[i]);
        }
    }

    NEXUS_Frontend_P_UnInit31xx(pDevice);
    NEXUS_Frontend_P_UnInit_31xx_Hab(pDevice);

#ifndef NEXUS_SHARED_FRONTEND_INTERRUPT
    BLST_S_REMOVE(&g_31xxDevList, pDevice, NEXUS_31xx, node);
#endif

    NEXUS_FrontendDevice_Unlink(pDevice->pGenericDeviceHandle, NULL);
    BKNI_Free(pDevice->pGenericDeviceHandle);
    pDevice->pGenericDeviceHandle = NULL;

    BDBG_OBJECT_DESTROY(pDevice, NEXUS_31xx);
    BKNI_Free(pDevice);
    pDevice = NULL;

    return;
}

/***************************************************************************
Summary:
    Initialize all ADS/OOB channels.
 ***************************************************************************/
static NEXUS_Error NEXUS_Frontend_P_Init31xx(NEXUS_31xx *pDevice)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned int i, num_ch;
    BTNR_PowerSaverSettings pwrSettings;
#ifdef NEXUS_FRONTEND_3117
    BTNR_3117Ob_Settings tnrOb3117_cfg;
    BTNR_3117Ib_Settings tnrIb3117_cfg;
#endif
#ifdef NEXUS_FRONTEND_3114
    BTNR_3114Ib_Settings tnrIb3114_cfg;
    BTNR_3114Ob_Settings tnrOb3114_cfg;
#endif
#ifdef NEXUS_FRONTEND_3112
    BTNR_3112Ib_Settings tnrIb3112_cfg;
#endif
#ifdef NEXUS_FRONTEND_3109
    BTNR_3109Ib_Settings tnrIb3109_cfg;
#endif
#ifdef BCHIP_HAS_AUS
    BAUS_Settings aus_cfg;
#endif
#ifdef BCHIP_HAS_AOB
    BAOB_Settings aob_cfg;
#endif
    BADS_Settings ads_cfg;
    BADS_ChannelSettings chn_cfg;

    switch (pDevice->chipId)
    {
#ifdef NEXUS_FRONTEND_3117
    case BCHIP_3117:
        rc = BADS_3117_GetDefaultSettings( &ads_cfg, NULL);
        break;
#endif
#ifdef NEXUS_FRONTEND_3114
    case BCHIP_3114:
        rc = BADS_3114_GetDefaultSettings( &ads_cfg, NULL);
        break;
#endif
#ifdef NEXUS_FRONTEND_3112
    case BCHIP_3112:
        rc = BADS_3112_GetDefaultSettings( &ads_cfg, NULL);
        break;
#endif
#ifdef NEXUS_FRONTEND_3109
    case BCHIP_3109:
        rc = BADS_3109_GetDefaultSettings( &ads_cfg, NULL);
        break;
#endif
    default:
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }
    if ( rc != BERR_SUCCESS ) goto done;

    if(pDevice->revId == 0x0) {
        ads_cfg.transportConfig = BADS_TransportData_eGpioSerial;
    }
    else if (pDevice->revId == 0x100){
        ads_cfg.transportConfig = BADS_TransportData_eSerial;
    }
    ads_cfg.hGeneric = pDevice->hab;
    ads_cfg.isOpenDrain = pDevice->openSettings.inBandOpenDrain;
    rc = BADS_Open(&pDevice->ads, NULL, NULL, NULL, &ads_cfg);
    if ( rc != BERR_SUCCESS ) goto done;

    rc = BADS_Init(pDevice->ads);
    if ( rc != BERR_SUCCESS ) goto done;

    /* get total ADS channel number*/
    rc = BADS_GetTotalChannels(pDevice->ads, &num_ch);
    if (rc!=BERR_SUCCESS) goto done;

    /* Configure ADS channels */
    for (i=0;i<num_ch;i++) {
        rc = BADS_GetChannelDefaultSettings( pDevice->ads, i, &chn_cfg);
        if (rc!=BERR_SUCCESS) goto done;
        rc = BADS_OpenChannel( pDevice->ads, &pDevice->ads_chn[i], i, &chn_cfg);
        if (rc!=BERR_SUCCESS) goto done;

        rc = BADS_InstallCallback(pDevice->ads_chn[i], BADS_Callback_eLockChange, (BADS_CallbackFunc)NEXUS_Frontend_P_31xx_callback_isr, (void*)&pDevice->frontendHandle[i]);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        rc = BADS_InstallCallback(pDevice->ads_chn[i], BADS_Callback_eAsyncStatusReady, (BADS_CallbackFunc)NEXUS_Frontend_P_31xx_AsyncStatusCallback_isr, (void*)&pDevice->frontendHandle[i]);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        rc = BADS_InstallCallback(pDevice->ads_chn[i], BADS_Callback_eUpdateGain, (BADS_CallbackFunc)NEXUS_Frontend_P_31xx_UpdateGainCallback_isr, (void*)pDevice);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        rc = BADS_EnablePowerSaver(pDevice->ads_chn[i]);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->isPoweredOn[i] = false;
    }

    for (i=0;i<num_ch;i++) {
        switch (pDevice->chipId)
        {
#ifdef NEXUS_FRONTEND_3117
        case BCHIP_3117:
            rc = BTNR_3117Ib_GetDefaultSettings(&tnrIb3117_cfg);
            if (rc != BERR_SUCCESS) goto done;
            rc =  BTNR_3117Ib_Open(&pDevice->tnr[i],&tnrIb3117_cfg, pDevice->hab);
            break;
#endif
#ifdef NEXUS_FRONTEND_3114
        case BCHIP_3114:
            rc = BTNR_3114Ib_GetDefaultSettings(&tnrIb3114_cfg);
            if (rc != BERR_SUCCESS) goto done;
            rc =  BTNR_3114Ib_Open(&pDevice->tnr[i],&tnrIb3114_cfg, pDevice->hab);
            break;
#endif
#ifdef NEXUS_FRONTEND_3112
        case BCHIP_3112:
            rc = BTNR_3112Ib_GetDefaultSettings(&tnrIb3112_cfg);
            if (rc != BERR_SUCCESS) goto done;
            rc =  BTNR_3112Ib_Open(&pDevice->tnr[i],&tnrIb3112_cfg, pDevice->hab);
            break;
#endif
#ifdef NEXUS_FRONTEND_3109
        case BCHIP_3109:
            rc = BTNR_3109Ib_GetDefaultSettings(&tnrIb3109_cfg);
            if (rc != BERR_SUCCESS) goto done;
            rc =  BTNR_3109Ib_Open(&pDevice->tnr[i],&tnrIb3109_cfg, pDevice->hab);
            break;
#endif
        default:
            rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
        }
        if(rc){rc = BERR_TRACE(rc); goto done;}

        pwrSettings.enable = true;
        rc = BTNR_SetPowerSaver(pDevice->tnr[i], &pwrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->isTunerPoweredOn[i] = false;
    }

    if((pDevice->chipId == BCHIP_3109) || (pDevice->chipId == BCHIP_3112))
        return BERR_SUCCESS;

    switch (pDevice->chipId)
    {
#ifdef NEXUS_FRONTEND_3117
    case BCHIP_3117:
        BTNR_3117Ob_GetDefaultSettings(&tnrOb3117_cfg);

        tnrOb3117_cfg.ifFreq = pDevice->ifFreq;
        rc = BTNR_3117Ob_Open(&pDevice->tnr[NEXUS_31xx_OOB_CHN], &tnrOb3117_cfg, pDevice->hab);
        if (rc!=BERR_SUCCESS) goto done;
        break;
#endif
#ifdef NEXUS_FRONTEND_3114
    case BCHIP_3114:
        BTNR_3114Ob_GetDefaultSettings(&tnrOb3114_cfg);

        tnrOb3114_cfg.ifFreq = pDevice->ifFreq;
        rc = BTNR_3114Ob_Open(&pDevice->tnr[NEXUS_31xx_OOB_CHN], &tnrOb3114_cfg, pDevice->hab);
        if (rc!=BERR_SUCCESS) goto done;
        break;
#endif
    default:
        BDBG_ERR(("Define the frontend type as either NEXUS_FRONTEND_3117 or NEXUS_FRONTEND_3114."));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if(pDevice->tnr[NEXUS_31xx_OOB_CHN]){
        pwrSettings.enable = true;
        rc = BTNR_SetPowerSaver(pDevice->tnr[NEXUS_31xx_OOB_CHN], &pwrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->isTunerPoweredOn[NEXUS_31xx_OOB_CHN] = false;
    }

#ifdef BCHIP_HAS_AOB
    rc = BAOB_InstallCallback(pDevice->aob, BAOB_Callback_eLockChange, (BAOB_CallbackFunc)NEXUS_Frontend_P_31xx_callback_isr, (void*)&pDevice->frontendHandle[NEXUS_31xx_OOB_CHN]);
    if(rc){rc = BERR_TRACE(rc); goto done;}
    rc = BAOB_InstallCallback(pDevice->aob, BAOB_Callback_eAsyncStatusReady, (BAOB_CallbackFunc)NEXUS_Frontend_P_31xx_AsyncStatusCallback_isr, (void*)&pDevice->frontendHandle[NEXUS_31xx_OOB_CHN]);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = BAOB_GetDefaultSettings( &aob_cfg, NULL);
    if (rc!=BERR_SUCCESS) goto done;
    /* There is only one AOB device in 3117/3114. so the device id is left to default settings. */
    aob_cfg.hGeneric = pDevice->hab;
    aob_cfg.ifFreq = pDevice->ifFreq;
    aob_cfg.serialData = true;
    aob_cfg.isOpenDrain = pDevice->oobOpenDrain;
    rc = BAOB_Open(&pDevice->aob, NULL, NULL, NULL, &aob_cfg);
    if (rc!=BERR_SUCCESS) goto done;

    rc = BAOB_EnablePowerSaver(pDevice->aob);
    if(rc){rc = BERR_TRACE(rc); goto done;}
    pDevice->isPoweredOn[NEXUS_31xx_OOB_CHN] = false;
#endif

#ifdef BCHIP_HAS_AUS
    rc = BAUS_GetDefaultSettings(&aus_cfg, NULL);
    if (rc!=BERR_SUCCESS) goto done;
    aus_cfg.hGeneric = pDevice->hab;
    rc = BAUS_Open(&pDevice->aus, NULL, NULL, &aus_cfg);
    if (rc!=BERR_SUCCESS) goto done;

    rc = BAUS_DisableTransmitter(pDevice->aus);
    if (rc!=BERR_SUCCESS) goto done;
    pDevice->isPoweredOn[NEXUS_31xx_AUS_CHN] = false;
#endif

    return BERR_SUCCESS;
done:
    NEXUS_FrontendDevice_P_31xx_Close(pDevice);
    return rc;
}

void NEXUS_FrontendDevice_GetDefault31xxSettings(NEXUS_FrontendDevice31xxSettings *pSettings)
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->enableDaisyChain = false;
    pSettings->agcValue = 0x1f; /* Max gain*/
}

static void NEXUS_Frontend_31xxDeviceTestThread(void *arg)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_FrontendDevice31xxSettings deviceSettings;
    BHAB_NmiSettings nmiSettings;
    BHAB_WatchDogTimerSettings wdtSettings;
    NEXUS_31xx *pDevice = (NEXUS_31xx *)arg;

   if (pDevice->openSettings.loadAP)
    {
        if(pDevice->revId == 0x0) {
#ifdef NEXUS_FRONTEND_31xx_B0
            BDBG_ERR(("Nexus is compiled to support 31xx B0 revision. But, the revision read is not B0.)"));
            rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
#endif
        }
        else if (pDevice->revId == 0x100){
#ifndef NEXUS_FRONTEND_31xx_B0
            BDBG_ERR(("Nexus is compiled to support either 31xx A0 or A1 revision. But, the revision read is neither.)"));
            rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
#endif
        }

        BDBG_WRN(("BHAB_InitAp(rev a image)"));
        rc = BHAB_InitAp(pDevice->hab, bcm31xx_ap_image);
        if ( rc != BERR_SUCCESS ) goto done;
    }
    else
    {
        rc = BHAB_InitAp(pDevice->hab, NULL);
        if ( rc != BERR_SUCCESS ) goto done;
    }

    if(pDevice->openSettings.configureWatchdog) {
        BKNI_Memset(&wdtSettings, 0, sizeof(wdtSettings));

        wdtSettings.clearWatchDog.enable = 0;
        wdtSettings.clearWatchDog.polarity = 1;
        wdtSettings.clearWatchDog.select = 10;
        wdtSettings.nmiMode = 0; /* nmi triggered midway */
        wdtSettings.oneShot = 0;
        wdtSettings.start = 0;
        wdtSettings.timeout = 0x202fbf00;   /*20 secs*/

        rc = BHAB_SetWatchDogTimer(pDevice->hab, &wdtSettings);
        if ( rc != BERR_SUCCESS ) goto done;

        BKNI_Memset(&nmiSettings, 0, sizeof(nmiSettings));

        nmiSettings.nmi.enable = 0;
        nmiSettings.nmi.polarity = 1; /*redundant*/
        nmiSettings.nmi.select = 0;

        rc = BHAB_SetNmiConfig(pDevice->hab, &nmiSettings);
        if ( rc != BERR_SUCCESS ) goto done;
    }

    /* Initialize HAB, THD and TC2 */
    rc = NEXUS_Frontend_P_Init31xx(pDevice);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    NEXUS_FrontendDevice_GetDefault31xxSettings(&deviceSettings);
    if(pDevice->settingsInitialized) deviceSettings = pDevice->deviceSettings;

    pDevice->isInternalSetSettings = true;
    rc = NEXUS_FrontendDevice_Set31xxSettings(pDevice->pGenericDeviceHandle, &deviceSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

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
    Initialize Hab for a 31xx device
***************************************************************************/
NEXUS_Error NEXUS_FrontendDevice_P_Init31xx(NEXUS_31xx *pDevice)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHAB_Settings stHabSettings;
    NEXUS_ThreadSettings thread_settings;
    BREG_I2C_Handle i2cHandle;
#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT
    NEXUS_31xx *ptempDevice = BLST_S_FIRST(&g_31xxDevList);
#endif

    pDevice->chipId = NEXUS_Frontend_P_Is31xx(&pDevice->openSettings);
    pDevice->revId = NEXUS_Frontend_P_Get31xxRev(&pDevice->openSettings);

    switch (pDevice->chipId)
    {
#ifdef NEXUS_FRONTEND_3117
    case BCHIP_3117:
        rc = BHAB_3117_GetDefaultSettings(&stHabSettings);
        break;
#endif
#ifdef NEXUS_FRONTEND_3114
    case BCHIP_3114:
        rc = BHAB_3114_GetDefaultSettings(&stHabSettings);
        break;
#endif
#ifdef NEXUS_FRONTEND_3112
    case BCHIP_3112:
        rc = BHAB_3112_GetDefaultSettings(&stHabSettings);
        break;
#endif
#ifdef NEXUS_FRONTEND_3109
    case BCHIP_3109:
        rc = BHAB_3109_GetDefaultSettings(&stHabSettings);
        break;
#endif
    default:
        rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }
    if ( rc != BERR_SUCCESS ) goto done;

    if(pDevice->openSettings.isrNumber) {
        stHabSettings.interruptEnableFunc = NEXUS_Frontend_P_31xx_IsrControl_isr;
        stHabSettings.interruptEnableFuncParam = (void*)pDevice->openSettings.isrNumber;
    }
    else if(pDevice->openSettings.gpioInterrupt){
        stHabSettings.interruptEnableFunc = NEXUS_Frontend_P_31xx_GpioIsrControl_isr;
        stHabSettings.interruptEnableFuncParam = (void*)pDevice->openSettings.gpioInterrupt;
    }
    stHabSettings.slaveChipAddr = pDevice->openSettings.i2cSlaveAddr;
    stHabSettings.chipAddr = pDevice->openSettings.i2cAddr;

    i2cHandle = NEXUS_I2c_GetRegHandle(pDevice->openSettings.i2cDevice, NEXUS_MODULE_SELF);
    if(i2cHandle == NULL ){rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto done;}

    stHabSettings.isSpi = false;
    rc = BHAB_Open( &pDevice->hab, (void *)i2cHandle, &stHabSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

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
        rc = NEXUS_Core_ConnectInterrupt(pDevice->openSettings.isrNumber, NEXUS_Frontend_P_31xx_L1_isr, (void *)pDevice, (int)pDevice->hab);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    else if(pDevice->openSettings.gpioInterrupt){
        NEXUS_Gpio_SetInterruptCallback_priv(pDevice->openSettings.gpioInterrupt, NEXUS_Frontend_P_31xx_L1_isr, (void *)pDevice, (int)pDevice->hab);
    }

    /* Get events and register callbacks */
    rc = BHAB_GetInterruptEventHandle(pDevice->hab, &pDevice->isrEvent);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pDevice->isrEventCallback = NEXUS_RegisterEvent(pDevice->isrEvent, NEXUS_Frontend_P_31xx_IsrEvent, pDevice->hab);
    if ( NULL == pDevice->isrEventCallback ){rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done; }

    NEXUS_Thread_GetDefaultSettings(&thread_settings);
    thread_settings.priority = 50;
    pDevice->deviceOpenThread = NEXUS_Thread_Create("deviceOpenThread",
                                               NEXUS_Frontend_31xxDeviceTestThread,
                                               (void*)pDevice,
                                               &thread_settings);
    return BERR_SUCCESS;
done:
    NEXUS_FrontendDevice_P_31xx_Close(pDevice);
    return rc;
}

/***************************************************************************
Summary:
  Probe to see if a BCM31xx device exists with the specified settings
 ***************************************************************************/
void NEXUS_FrontendDevice_GetDefault31xxOpenSettings(NEXUS_FrontendDevice31xxOpenSettings *pSettings)
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->loadAP = true;
    pSettings->i2cSlaveAddr = 0x60;
    pSettings->inBandOpenDrain = true;
}

/***************************************************************************
Summary:
    Get the default settings for a BCM3117 tuner
See Also:
    NEXUS_Frontend_Open3117
 ***************************************************************************/
void NEXUS_Frontend_GetDefault31xxSettings(
    NEXUS_31xxSettings *pSettings   /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->type = NEXUS_31xxChannelType_eInBand;
    pSettings->channelNumber = 0;
    pSettings->inBandOpenDrain = true;
    pSettings->outOfBandOpenDrain = true;
    pSettings->loadAP = true;
    pSettings->i2cSlaveAddr = 0x60;
}

NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_Open31xx(unsigned index, const NEXUS_FrontendDevice31xxOpenSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_31xx *pDevice = NULL;
    NEXUS_FrontendDevice *pFrontendDevice = NULL;
    NEXUS_31xxProbeResults results;

    BSTD_UNUSED(index);

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pSettings->i2cDevice);

    rc = NEXUS_Frontend_Probe31xx(pSettings, &results);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    for ( pDevice = BLST_S_FIRST(&g_31xxDevList); NULL != pDevice; pDevice = BLST_S_NEXT(pDevice, node) )
    {
        BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);
        if((pSettings->i2cAddr == pDevice->openSettings.i2cAddr) && (pSettings->i2cDevice == pDevice->openSettings.i2cDevice))
        {
            break;
        }
    }

    if ( NULL == pDevice)
    {
#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT
        for ( pDevice = BLST_S_FIRST(&g_31xxDevList); NULL != pDevice; pDevice = BLST_S_NEXT(pDevice, node) )
        {
            BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);
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

        pDevice = BKNI_Malloc(sizeof(*pDevice));
        if (NULL == pDevice) {rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done;}
        BKNI_Memset(pDevice, 0, sizeof(NEXUS_31xx));
        BDBG_OBJECT_SET(pDevice, NEXUS_31xx);

        pDevice->openSettings = *pSettings;
        pDevice->pGenericDeviceHandle = pFrontendDevice;
#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT
        pDevice->isMaster = true;
#endif

        BKNI_EnterCriticalSection();
        pDevice->pGenericDeviceHandle->openPending = true;
        pDevice->pGenericDeviceHandle->openFailed = false;
        BKNI_LeaveCriticalSection();

        rc = NEXUS_FrontendDevice_P_Init31xx(pDevice);
        if(rc){rc = BERR_TRACE(rc); goto done;}

#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT
        BKNI_EnterCriticalSection();
        BLST_S_INSERT_HEAD(&g_31xxDevList, pDevice, node);
        BKNI_LeaveCriticalSection();
#else
        BLST_S_INSERT_HEAD(&g_31xxDevList, pDevice, node);
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
    pFrontendDevice->application = NEXUS_FrontendDeviceApplication_eCable;
    pFrontendDevice->close = NEXUS_FrontendDevice_P_31xx_Close;
    pFrontendDevice->standby = NEXUS_FrontendDevice_P_31xx_Standby;
    return pFrontendDevice;

done:
    if(pDevice)
        NEXUS_FrontendDevice_P_31xx_Close(pDevice);
    return NULL;

}

/***************************************************************************
Summary:
    Open a handle to a BCM3117 device.
 ***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Open31xx(
    const NEXUS_Frontend31xxSettings *pSettings
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_FrontendHandle frontendHandle = NULL;
    NEXUS_31xx *pDevice = NULL;
    unsigned int chn_num=0;
    NEXUS_31xxChannel *pChannel;
    NEXUS_FrontendDevice *pFrontendDevice = NULL;
    NEXUS_FrontendDevice31xxOpenSettings openSettings;

    BDBG_ASSERT(NULL != pSettings);
    if(pSettings->device == NULL)
        BDBG_ASSERT(NULL != pSettings->i2cDevice);

    if ( ((pSettings->channelNumber >= NEXUS_MAX_31xx_ADSCHN)&& pSettings->type == NEXUS_31xxChannelType_eInBand)
        || (pSettings->channelNumber >= NEXUS_MAX_31xx_FRONTENDS ))
    {
        BDBG_ERR((" channel number exceeds available one"));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    switch(pSettings->type)
    {
    case NEXUS_31xxChannelType_eInBand:
        chn_num = pSettings->channelNumber;
        break;
    case NEXUS_31xxChannelType_eOutOfBand:
        chn_num = NEXUS_31xx_OOB_CHN;
        break;
    case NEXUS_31xxChannelType_eUpstream:
        chn_num = NEXUS_31xx_AUS_CHN;
        break;
    default:
        BDBG_ERR((" channel type not supported"));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    if(pSettings->device == NULL) {
        NEXUS_FrontendDevice_GetDefault31xxOpenSettings(&openSettings);
        openSettings.gpioInterrupt = pSettings->gpioInterrupt;
        openSettings.isrNumber  = pSettings->isrNumber;
        openSettings.i2cDevice  = pSettings->i2cDevice;
        openSettings.i2cAddr = pSettings->i2cAddr;
        openSettings.i2cSlaveAddr  = pSettings->i2cSlaveAddr;
        openSettings.configureWatchdog = pSettings->configureWatchdog;
        openSettings.loadAP  = pSettings->loadAP;
        openSettings.inBandOpenDrain  = pSettings->inBandOpenDrain;

        openSettings.outOfBand.ifFrequency  = pSettings->outOfBandOpenDrain;
        openSettings.outOfBand.openDrain  = pSettings->ifFrequency;

        pFrontendDevice = NEXUS_FrontendDevice_Open31xx(0, &openSettings);
        pDevice = (NEXUS_31xx *)pFrontendDevice->pDevice;
    }
    else {
        pDevice = (NEXUS_31xx *)pSettings->device->pDevice;
        pFrontendDevice =  pSettings->device;
    }

    if ((pSettings->type == NEXUS_31xxChannelType_eOutOfBand ) && ((pDevice->chipId == BCHIP_3109) || (pDevice->chipId == BCHIP_3112)))
    {
        return NULL;
    }

    /* chekc if fronthandle is already opened*/
    if ( pDevice->frontendHandle[chn_num] != NULL )
    {
        return pDevice->frontendHandle[chn_num];
    }

    pChannel = (NEXUS_31xxChannel*)BKNI_Malloc(sizeof(NEXUS_31xxChannel));
    if ( NULL == pChannel ) {rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done;}

    /* Create a Nexus frontend handle */
    frontendHandle = NEXUS_Frontend_P_Create(pChannel);
    if ( NULL == frontendHandle ) {rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done;}

    /* Establish device capabilities */
    if ( pSettings->type == NEXUS_31xxChannelType_eInBand)
    {
        frontendHandle->capabilities.qam = true;
        frontendHandle->capabilities.outOfBand = false;
        frontendHandle->capabilities.upstream = false;
        BKNI_Memset(frontendHandle->capabilities.qamModes, true, sizeof(frontendHandle->capabilities.qamModes));
        /* bind functions*/
        frontendHandle->tuneQam = NEXUS_Frontend_P_31xx_TuneQam;
        frontendHandle->getQamStatus = NEXUS_Frontend_P_31xx_GetQamStatus;
        frontendHandle->readSoftDecisions = NEXUS_Frontend_P_31xx_ReadSoftDecisions;
        frontendHandle->resetStatus = NEXUS_Frontend_P_31xx_ResetAdsStatus;
        frontendHandle->untune = NEXUS_Frontend_P_31xx_UnTuneQam;
        frontendHandle->requestQamAsyncStatus = NEXUS_Frontend_P_31xx_RequestQamAsyncStatus;
        frontendHandle->getQamAsyncStatus = NEXUS_Frontend_P_31xx_GetQamAsyncStatus;
        frontendHandle->getQamScanStatus = NEXUS_Frontend_P_31xx_GetQamScanStatus;
    }
#ifdef BCHIP_HAS_AOB
    else if ( pSettings->type == NEXUS_31xxChannelType_eOutOfBand)
    {
        frontendHandle->capabilities.qam = false;
        frontendHandle->capabilities.outOfBand = true;
        frontendHandle->capabilities.upstream = false;
        BKNI_Memset(frontendHandle->capabilities.outOfBandModes, true, sizeof(frontendHandle->capabilities.outOfBandModes));
        /* bind functions*/
        frontendHandle->tuneOutOfBand = NEXUS_Frontend_P_31xx_TuneOob;
        frontendHandle->getOutOfBandStatus = NEXUS_Frontend_P_31xx_GetOobStatus;
        frontendHandle->readSoftDecisions = NEXUS_Frontend_P_31xx_ReadSoftDecisions;
        frontendHandle->resetStatus = NEXUS_Frontend_P_31xx_ResetOobStatus;
        frontendHandle->untune = NEXUS_Frontend_P_31xx_UnTuneOob;
        frontendHandle->requestOutOfBandAsyncStatus = NEXUS_Frontend_P_31xx_RequestOobAsyncStatus;
        frontendHandle->getOutOfBandAsyncStatus = NEXUS_Frontend_P_31xx_GetOobAsyncStatus;
    }
#endif
#ifdef BCHIP_HAS_AUS
    else if ( pSettings->type == NEXUS_31xxChannelType_eUpstream)
    {
        frontendHandle->capabilities.qam = false;
        frontendHandle->capabilities.outOfBand = false;
        frontendHandle->capabilities.upstream = true;
        BKNI_Memset(frontendHandle->capabilities.upstreamModes, true, sizeof(frontendHandle->capabilities.upstreamModes));
        /* bind functions*/
        frontendHandle->tuneUpstream = NEXUS_Frontend_P_31xx_TuneUpstream;
        frontendHandle->getUpstreamStatus = NEXUS_Frontend_P_31xx_GetUpstreamStatus;
        frontendHandle->untune = NEXUS_Frontend_P_31xx_UnTuneUpstream;
    }
#endif

    frontendHandle->close = NEXUS_Frontend_P_31xx_Close;
    frontendHandle->getType = NEXUS_Frontend_P_31xx_GetType;
    frontendHandle->getFastStatus = NEXUS_Frontend_P_31xx_GetFastStatus;
    frontendHandle->standby = NEXUS_Frontend_P_31xx_Standby;
    frontendHandle->uninstallCallbacks = NEXUS_Frontend_P_31xx_UninstallCallbacks;

    /* Create app callback */
    pDevice->lockAppCallback[chn_num] = NEXUS_IsrCallback_Create(frontendHandle, NULL);
    if ( NULL == pDevice->lockAppCallback[chn_num] ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto done;}
    /* install callback to  notify of lock/unlock change */
    if ( pSettings->type == NEXUS_31xxChannelType_eInBand)
    {
        pDevice->updateGainAppCallback = NEXUS_IsrCallback_Create(frontendHandle, NULL);
        if ( NULL == pDevice->updateGainAppCallback ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto done;}

        pDevice->asyncStatusAppCallback[chn_num] = NEXUS_IsrCallback_Create(frontendHandle, NULL);
        if ( NULL == pDevice->asyncStatusAppCallback[chn_num] ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto done;}
    }
#ifdef BCHIP_HAS_AOB
    else if(pSettings->type == NEXUS_31xxChannelType_eOutOfBand)
    {
        pDevice->asyncStatusAppCallback[chn_num] = NEXUS_IsrCallback_Create(frontendHandle, NULL);
        if ( NULL == pDevice->asyncStatusAppCallback[chn_num] ) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto done;}
    }
#endif
    frontendHandle->pGenericDeviceHandle = pFrontendDevice;

    /* See if upstream needs a callback. */
    pDevice->frontendHandle[chn_num] = frontendHandle;
    /* save channel number in channelHandle*/
    pChannel->chn_num = chn_num;
    pChannel->pDevice = pDevice;
    frontendHandle->chip.id = pDevice->chipId;
    pDevice->numfrontends++;
    return frontendHandle;

done:
    NEXUS_FrontendDevice_P_31xx_Close(pDevice);
    return NULL;
}

/***************************************************************************
Summary:
    Close a handle to a BCM3117 device.
See Also:
    NEXUS_Frontend_Open3117
 ***************************************************************************/
static void NEXUS_Frontend_P_31xx_Close(
    NEXUS_FrontendHandle handle
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_31xx *pDevice;
    NEXUS_31xxChannel *pChannel;

    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    pChannel =(NEXUS_31xxChannel*) handle->pDeviceHandle;
    BDBG_ASSERT(NULL != pChannel);
    pDevice = (NEXUS_31xx *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);

    if (pChannel->chn_num >= NEXUS_MAX_31xx_FRONTENDS)
    {
        BDBG_ERR((" Unsupported Frontend Handle"));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    if(pChannel->chn_num < NEXUS_MAX_31xx_ADSCHN) {
        if( NULL != pDevice->updateGainAppCallback)
            NEXUS_IsrCallback_Destroy(pDevice->updateGainAppCallback);

        if(pDevice->ads_chn[pChannel->chn_num]){
            BADS_InstallCallback(pDevice->ads_chn[pChannel->chn_num], BADS_Callback_eLockChange, NULL, NULL);
            BADS_InstallCallback(pDevice->ads_chn[pChannel->chn_num], BADS_Callback_eUpdateGain, NULL, NULL);
            BADS_InstallCallback(pDevice->ads_chn[pChannel->chn_num], BADS_Callback_eAsyncStatusReady, NULL, NULL);
        }
    }

#ifdef BCHIP_HAS_AOB
    if((pChannel->chn_num == NEXUS_31xx_OOB_CHN) && (pDevice->aob)){
        BAOB_InstallCallback(pDevice->aob, BAOB_Callback_eLockChange, NULL, NULL);
        BAOB_InstallCallback(pDevice->aob, BAOB_Callback_eAsyncStatusReady, NULL, NULL);
    }
#endif

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

static NEXUS_Error NEXUS_Frontend_P_31xx_TuneQam(void *handle, const NEXUS_FrontendQamSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BADS_InbandParam ibParam;
    BTNR_PowerSaverSettings pwrSettings;
    NEXUS_31xx *pDevice;
    NEXUS_31xxChannel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_31xxChannel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_ASSERT(NULL != pDevice);
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);

    if (pChannel->chn_num >= NEXUS_MAX_31xx_ADSCHN)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }
    if ( pSettings->annex == NEXUS_FrontendQamAnnex_eA )
    {
        switch ( pSettings->mode )
        {
        case NEXUS_FrontendQamMode_e16:
            ibParam.modType = BADS_ModulationType_eAnnexAQam16;
            break;
        case NEXUS_FrontendQamMode_e32:
            ibParam.modType = BADS_ModulationType_eAnnexAQam32;
            break;
        case NEXUS_FrontendQamMode_e64:
            ibParam.modType = BADS_ModulationType_eAnnexAQam64;
            break;
        case NEXUS_FrontendQamMode_e128:
            ibParam.modType = BADS_ModulationType_eAnnexAQam128;
            break;
        case NEXUS_FrontendQamMode_e256:
            ibParam.modType = BADS_ModulationType_eAnnexAQam256;
            break;
        default:
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        ibParam.enableNullPackets = pSettings->enableNullPackets;
    }
    else if ( pSettings->annex == NEXUS_FrontendQamAnnex_eB )
    {
        switch ( pSettings->mode )
        {
        case NEXUS_FrontendQamMode_e64:
            ibParam.modType = BADS_ModulationType_eAnnexBQam64;
            break;
        case NEXUS_FrontendQamMode_e256:
            ibParam.modType = BADS_ModulationType_eAnnexBQam256;
            break;
        default:
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        ibParam.enableNullPackets = false;
    }
    else
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    ibParam.symbolRate = pSettings->symbolRate;
    ibParam.autoAcquire = pSettings->autoAcquire;
    ibParam.enableDpm = pSettings->enablePowerMeasurement;
    ibParam.spectrum = pSettings->spectrumMode;
    ibParam.invertSpectrum = pSettings->spectralInversion;
    ibParam.frequencyOffset = pSettings->frequencyOffset;

    NEXUS_IsrCallback_Set(pDevice->lockAppCallback[pChannel->chn_num], &(pSettings->lockCallback));
    NEXUS_IsrCallback_Set(pDevice->asyncStatusAppCallback[pChannel->chn_num], &(pSettings->asyncStatusReadyCallback));

    if (pDevice->tnr[pChannel->chn_num])
    {
        if(!pDevice->isTunerPoweredOn[pChannel->chn_num]){
            pwrSettings.enable = false;
            rc = BTNR_SetPowerSaver(pDevice->tnr[pChannel->chn_num], &pwrSettings);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->isTunerPoweredOn[pChannel->chn_num] = true;
        }

        BKNI_Sleep(10);

        if(!pDevice->isPoweredOn[pChannel->chn_num]){
            rc = BADS_DisablePowerSaver(pDevice->ads_chn[pChannel->chn_num]);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->isPoweredOn[pChannel->chn_num] = true;
        }

        pDevice->count = 0;
        pDevice->acquireInProgress = true;

        rc = BTNR_SetTunerRfFreq(pDevice->tnr[pChannel->chn_num], pSettings->frequency, BTNR_TunerMode_eDigital);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    BKNI_Sleep(90);

    rc = BADS_Acquire(pDevice->ads_chn[pChannel->chn_num], &ibParam );
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pDevice->last_ads[pChannel->chn_num] = *pSettings;

    return BERR_SUCCESS;
done:
    NEXUS_Frontend_P_31xx_UnTuneQam(handle);
    return rc;
}

static void NEXUS_Frontend_P_31xx_UninstallCallbacks(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_31xx *pDevice;
    NEXUS_31xxChannel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_31xxChannel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);

    if (pChannel->chn_num >= NEXUS_MAX_31xx_FRONTENDS)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    if(pDevice->lockAppCallback[pChannel->chn_num])NEXUS_IsrCallback_Set(pDevice->lockAppCallback[pChannel->chn_num], NULL);
    if(pDevice->asyncStatusAppCallback[pChannel->chn_num])NEXUS_IsrCallback_Set(pDevice->asyncStatusAppCallback[pChannel->chn_num], NULL);

done:
    return;
}

static void NEXUS_Frontend_P_31xx_UnTuneQam(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_31xx *pDevice;
    NEXUS_31xxChannel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_31xxChannel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_ASSERT(NULL != pDevice);
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);

    if (pChannel->chn_num >= NEXUS_MAX_31xx_ADSCHN)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    if(pDevice->isPoweredOn[pChannel->chn_num]){
        rc = BADS_EnablePowerSaver(pDevice->ads_chn[pChannel->chn_num]);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->isPoweredOn[pChannel->chn_num] = false;
    }
done:
    return;
}

static NEXUS_Error NEXUS_Frontend_P_31xx_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BTNR_PowerSaverSettings pwrSettings;
    NEXUS_31xx *pDevice;
    NEXUS_31xxChannel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_31xxChannel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_ASSERT(NULL != pDevice);
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);
    BSTD_UNUSED(enabled);

    if (pChannel->chn_num >= NEXUS_MAX_31xx_FRONTENDS)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    if(pSettings->mode >= NEXUS_StandbyMode_ePassive){
        NEXUS_Frontend_P_31xx_UninstallCallbacks(handle);
        if(pChannel->chn_num < NEXUS_MAX_31xx_ADSCHN){
            if(pDevice->isPoweredOn[pChannel->chn_num]){
                rc = BADS_EnablePowerSaver(pDevice->ads_chn[pChannel->chn_num]);
                if(rc){rc = BERR_TRACE(rc); goto done;}
                pDevice->isPoweredOn[pChannel->chn_num] = false;
            }
        }
        else if(pChannel->chn_num == NEXUS_31xx_OOB_CHN){
#ifdef BCHIP_HAS_AOB
            if(pDevice->isPoweredOn[pChannel->chn_num]){
                rc = BAOB_EnablePowerSaver(pDevice->aob);
                if(rc){rc = BERR_TRACE(rc); goto done;}
                pDevice->isPoweredOn[pChannel->chn_num] = false;
            }
#endif
        }
        else if(pChannel->chn_num == NEXUS_31xx_AUS_CHN){
#ifdef BCHIP_HAS_AUS
            if(pDevice->isPoweredOn[pChannel->chn_num]){
                rc = BAUS_DisableTransmitter(pDevice->aus);
                if(rc){BERR_TRACE(rc);}
                pDevice->isPoweredOn[pChannel->chn_num] = false;
            }
#endif
        }
        if(pChannel->chn_num < NEXUS_MAX_31xx_TUNERS){
            if(pDevice->isTunerPoweredOn[pChannel->chn_num]){
                pwrSettings.enable = true;
                rc = BTNR_SetPowerSaver(pDevice->tnr[pChannel->chn_num], &pwrSettings);
                if(rc){rc = BERR_TRACE(rc); goto done;}
                pDevice->isTunerPoweredOn[pChannel->chn_num] = false;
            }
        }
    }
done:
    return rc;
}

/***************************************************************************
Summary:
    Initialize Hab for a 31xx device
***************************************************************************/
void NEXUS_FrontendDevice_P_31xx_S3Standby(NEXUS_31xx *pDevice)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
#ifndef NEXUS_SHARED_FRONTEND_INTERRUPT
    NEXUS_GpioSettings gpioSettings;
#endif

    if(pDevice->deviceOpenThread)NEXUS_Thread_Destroy(pDevice->deviceOpenThread);
    pDevice->deviceOpenThread = NULL;

    NEXUS_Frontend_P_UnInit31xx(pDevice);

#ifndef NEXUS_SHARED_FRONTEND_INTERRUPT
    if(pDevice->openSettings.isrNumber) {
        NEXUS_Core_DisconnectInterrupt(pDevice->openSettings.isrNumber);
    }
    else if(pDevice->openSettings.gpioInterrupt){

        NEXUS_Gpio_SetInterruptCallback_priv(pDevice->openSettings.gpioInterrupt, NULL, NULL, 0);
        NEXUS_Gpio_GetSettings(pDevice->openSettings.gpioInterrupt,  &gpioSettings);
        gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
        gpioSettings.interrupt.callback = NULL;
        NEXUS_Gpio_SetSettings(pDevice->openSettings.gpioInterrupt, &gpioSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
#endif

    if(pDevice->isrEventCallback)NEXUS_UnregisterEvent(pDevice->isrEventCallback);
    pDevice->isrEventCallback = NULL;
    if(pDevice->isrEvent) pDevice->isrEvent = NULL;

    if(pDevice->hab) {
        rc = BHAB_Reset(pDevice->hab);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        rc = BHAB_Close(pDevice->hab);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }
    pDevice->hab = NULL;
done:
    return;
}

static NEXUS_Error NEXUS_FrontendDevice_P_31xx_Standby(void *handle, const NEXUS_StandbySettings *pSettings)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_31xx *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_31xx *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);

    if((pDevice->pGenericDeviceHandle->mode != NEXUS_StandbyMode_eDeepSleep) && (pSettings->mode == NEXUS_StandbyMode_eDeepSleep)){
        rc = BHAB_Reset(pDevice->hab);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        BKNI_EnterCriticalSection();
        pDevice->pGenericDeviceHandle->openPending = true;
        pDevice->pGenericDeviceHandle->openFailed = false;
        BKNI_LeaveCriticalSection();
    }
    else if((pDevice->pGenericDeviceHandle->mode == NEXUS_StandbyMode_eDeepSleep) && (pSettings->mode != NEXUS_StandbyMode_eDeepSleep)){
        rc = BHAB_InitAp(pDevice->hab, bcm31xx_ap_image);
        if ( rc != BERR_SUCCESS ) goto done;

        BKNI_EnterCriticalSection();
        pDevice->pGenericDeviceHandle->openPending = false;
        pDevice->pGenericDeviceHandle->openFailed = false;
        BKNI_LeaveCriticalSection();
    }
done:
    if(rc){
        BKNI_EnterCriticalSection();
        pDevice->pGenericDeviceHandle->openPending = false;
        pDevice->pGenericDeviceHandle->openFailed = true;
        BKNI_LeaveCriticalSection();
    }
    return rc;
}

static NEXUS_FrontendLockStatus  NEXUS_Frontend_P_GetLockStatus(unsigned lockStatus)
{
    switch ( lockStatus )
    {
    /*BADS_LockStatus_eUnlocked == BAOB_LockStatus_eUnlocked.  */
    case BADS_LockStatus_eUnlocked:
        return NEXUS_FrontendLockStatus_eUnlocked;
    case BADS_LockStatus_eLocked:
        return NEXUS_FrontendLockStatus_eLocked;
    case BADS_LockStatus_eNoSignal:
        return NEXUS_FrontendLockStatus_eNoSignal;
    default:
        BDBG_WRN(("Unrecognized lock status (%d) ", lockStatus));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return NEXUS_FrontendLockStatus_eUnknown;
    }
}
static NEXUS_Error NEXUS_Frontend_P_31xx_GetQamScanStatus(void *handle, NEXUS_FrontendQamScanStatus *pScanStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_31xx *pDevice;
    NEXUS_31xxChannel *pChannel;
    NEXUS_FrontendFastStatus status;

    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_31xxChannel *)handle;
    pDevice = (NEXUS_31xx *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);

    BDBG_WRN(("NEXUS_Frontend_GetQamScanStatus() is supported by copying the previously tuned parameters."));
    if (pChannel->chn_num >= NEXUS_MAX_31xx_ADSCHN)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    BKNI_Memset(pScanStatus, 0x0, sizeof(*pScanStatus));
    pScanStatus->symbolRate = pDevice->last_ads[pChannel->chn_num].symbolRate;
    pScanStatus->frequencyOffset = pDevice->last_ads[pChannel->chn_num].frequencyOffset;
    pScanStatus->mode = pDevice->last_ads[pChannel->chn_num].mode;
    pScanStatus->annex = pDevice->last_ads[pChannel->chn_num].annex;
    pScanStatus->spectrumInverted = pDevice->last_ads[pChannel->chn_num].spectralInversion;
    NEXUS_Frontend_P_31xx_GetFastStatus(handle, &status);
    if(status.lockStatus == NEXUS_FrontendLockStatus_eUnlocked)
        pScanStatus->acquisitionStatus = NEXUS_FrontendQamAcquisitionStatus_eNoLock;
    else if(status.lockStatus == NEXUS_FrontendLockStatus_eNoSignal)
        pScanStatus->acquisitionStatus = NEXUS_FrontendQamAcquisitionStatus_eNoSignal;
    else if(status.lockStatus == NEXUS_FrontendLockStatus_eLocked)
        pScanStatus->acquisitionStatus = NEXUS_FrontendQamAcquisitionStatus_eLockedSlow;

done:
    return rc;

}

static NEXUS_Error NEXUS_Frontend_P_31xx_GetFastStatus(void *handle, NEXUS_FrontendFastStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_31xx *pDevice;
    NEXUS_31xxChannel *pChannel;
    unsigned lock;

    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_31xxChannel *)handle;
    pDevice = (NEXUS_31xx *)pChannel->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);

    if(pChannel->chn_num < NEXUS_MAX_31xx_ADSCHN){
        if(!pDevice->isPoweredOn[pChannel->chn_num]){
            rc = BADS_DisablePowerSaver(pDevice->ads_chn[pChannel->chn_num]);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            pDevice->isPoweredOn[pChannel->chn_num] = true;
        }
        rc = BADS_GetLockStatus(pDevice->ads_chn[pChannel->chn_num],  (BADS_LockStatus *)&lock);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pStatus->lockStatus = NEXUS_Frontend_P_GetLockStatus(lock);
    }
#ifdef BCHIP_HAS_AOB
    else if(pChannel->chn_num == NEXUS_31xx_OOB_CHN){
            rc = BAOB_GetLockStatus(pDevice->aob,  (bool *)&lock);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pStatus->lockStatus = NEXUS_Frontend_P_GetLockStatus(lock);
    }
#endif
    pStatus->acquireInProgress = pDevice->acquireInProgress;

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_31xx_GetQamStatus(void *handle, NEXUS_FrontendQamStatus *pStatus)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    struct BADS_Status st;
    NEXUS_31xx *pDevice;
    NEXUS_31xxChannel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_31xxChannel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_ASSERT(NULL != pDevice);
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);

    if (pChannel->chn_num >= NEXUS_MAX_31xx_ADSCHN)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    rc = BADS_GetStatus(pDevice->ads_chn[pChannel->chn_num],  &st);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pStatus->fecLock = st.isFecLock;
    pStatus->receiverLock = st.isQamLock;
    pStatus->rfAgcLevel = st.agcExtLevel;
    pStatus->ifAgcLevel = st.agcIntLevel;
    pStatus->snrEstimate = st.snrEstimate*100/256;
    pStatus->fecCorrected = st.accCorrectedCount;
    pStatus->fecUncorrected = st.accUncorrectedCount;
    pStatus->fecClean = st.cleanCount;
    pStatus->carrierPhaseOffset = st.carrierPhaseOffset;
    pStatus->carrierFreqOffset = st.carrierFreqOffset;
    pStatus->symbolRate = st.rxSymbolRate;
    pStatus->berEstimate = st.berRawCount;
    pStatus->dsChannelPower = st.dsChannelPower;
    pStatus->mainTap = st.mainTap;
    pStatus->equalizerGain = st.equalizerGain;
    pStatus->postRsBer = 0;/* Not supported */
    pStatus->postRsBerElapsedTime = 0;/* Not supported */
    pStatus->settings = pDevice->last_ads[pChannel->chn_num];
    pStatus->spectrumInverted = st.isSpectrumInverted;

    pStatus->viterbiUncorrectedBits = st.correctedBits + (uint32_t)((uint64_t)pStatus->fecUncorrected * 11224)/1000;

    if(pStatus->settings.annex == NEXUS_FrontendQamAnnex_eA){
        pStatus->viterbiTotalBits = (uint32_t)(((uint64_t)pStatus->fecCorrected + (uint64_t)pStatus->fecUncorrected + (uint64_t)pStatus->fecClean) * 204 * 8);
    }
    else if(pStatus->settings.annex == NEXUS_FrontendQamAnnex_eB){
        pStatus->viterbiTotalBits = (uint32_t)(((uint64_t)pStatus->fecCorrected + (uint64_t)pStatus->fecUncorrected + (uint64_t)pStatus->fecClean) * 127 * 7);
    }
    if (pStatus->viterbiTotalBits) {
        pStatus->viterbiErrorRate = (uint32_t)((uint64_t)pStatus->viterbiUncorrectedBits * 2097152 * 1024 / (unsigned)pStatus->viterbiTotalBits);
    }

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_31xx_GetQamAsyncStatus(void *handle, NEXUS_FrontendQamStatus *pStatus)
{

    NEXUS_Error rc = NEXUS_SUCCESS;
    struct BADS_Status st;
    NEXUS_31xx *pDevice;
    NEXUS_31xxChannel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_31xxChannel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_ASSERT(NULL != pDevice);
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);

    if (pChannel->chn_num >= NEXUS_MAX_31xx_ADSCHN)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    rc = BADS_GetAsyncStatus(pDevice->ads_chn[pChannel->chn_num],  &st);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pStatus->fecLock = st.isFecLock;
    pStatus->receiverLock = st.isQamLock;
    pStatus->rfAgcLevel = st.agcExtLevel;
    pStatus->ifAgcLevel = st.agcIntLevel;
    pStatus->snrEstimate = st.snrEstimate*100/256;
    pStatus->fecCorrected = st.accCorrectedCount;
    pStatus->fecUncorrected = st.accUncorrectedCount;
    pStatus->fecClean = st.cleanCount;
    pStatus->carrierPhaseOffset = st.carrierPhaseOffset;
    pStatus->carrierFreqOffset = st.carrierFreqOffset;
    pStatus->symbolRate = st.rxSymbolRate;
    pStatus->berEstimate = st.berRawCount;
    pStatus->dsChannelPower = st.dsChannelPower;
    pStatus->mainTap = st.mainTap;
    pStatus->equalizerGain = st.equalizerGain;
    pStatus->postRsBer = 0;/* Not supported */
    pStatus->postRsBerElapsedTime = 0;/* Not supported */
    pStatus->settings = pDevice->last_ads[pChannel->chn_num];
    pStatus->spectrumInverted = st.isSpectrumInverted;
    pStatus->goodRsBlockCount = st.goodRsBlockCount;
    pStatus->berRawCount = st.berRawCount;

    pStatus->viterbiUncorrectedBits = st.correctedBits + (uint32_t)((uint64_t)pStatus->fecUncorrected * 11224)/1000;

    if(pStatus->settings.annex == NEXUS_FrontendQamAnnex_eA){
        pStatus->viterbiTotalBits = (uint32_t)(((uint64_t)pStatus->fecCorrected + (uint64_t)pStatus->fecUncorrected + (uint64_t)pStatus->fecClean) * 204 * 8);
    }
    else if(pStatus->settings.annex == NEXUS_FrontendQamAnnex_eB){
        pStatus->viterbiTotalBits = (uint32_t)(((uint64_t)pStatus->fecCorrected + (uint64_t)pStatus->fecUncorrected + (uint64_t)pStatus->fecClean) * 127 * 7);
    }
    if (pStatus->viterbiTotalBits) {
        pStatus->viterbiErrorRate = (uint32_t)((uint64_t)pStatus->viterbiUncorrectedBits * 2097152 * 1024 / (unsigned)pStatus->viterbiTotalBits);
    }

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_31xx_RequestQamAsyncStatus(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_31xx *pDevice;
    NEXUS_31xxChannel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_31xxChannel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_ASSERT(NULL != pDevice);
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);

    if(pChannel->chn_num >= NEXUS_MAX_31xx_ADSCHN)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    rc = BADS_RequestAsyncStatus(pDevice->ads_chn[pChannel->chn_num]);
    if(rc){rc = BERR_TRACE(rc); goto done;}

done:
    return rc;

}

static NEXUS_Error NEXUS_Frontend_P_31xx_ReadSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length, size_t *pNumRead)
{
    #define TOTAL_ADS_SOFTDECISIONS 30

    size_t i;
    int16_t return_length;
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_31xx *pDevice;
    NEXUS_31xxChannel *pChannel;
    int16_t d_i[TOTAL_ADS_SOFTDECISIONS], d_q[TOTAL_ADS_SOFTDECISIONS];

    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_31xxChannel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_ASSERT(NULL != pDevice);
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);

    if (pChannel->chn_num >= NEXUS_MAX_31xx_ADSCHN)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    /* only make one call to ADS. if app needs more, they can loop. */
    rc = BADS_GetSoftDecision(pDevice->ads_chn[pChannel->chn_num], (int16_t)TOTAL_ADS_SOFTDECISIONS, d_i, d_q, &return_length);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    for (i=0; (int)i<return_length && i<length; i++)
    {
        pDecisions[i].i = d_i[i] * 16;
        pDecisions[i].q = d_q[i] * 16;
    }
    *pNumRead = i;

    return BERR_SUCCESS;
done:
    return rc;

}

NEXUS_Error NEXUS_FrontendDevice_Get31xxSettings(NEXUS_FrontendDeviceHandle handle, NEXUS_FrontendDevice31xxSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_31xx *pDevice = NULL;
    BDBG_ASSERT(handle != NULL);

    pDevice = handle->pDevice;
    if (!pDevice) { rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto done; }
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);

    pSettings->updateGainCallback = pDevice->deviceSettings.updateGainCallback;
    pSettings->agcValue = pDevice->deviceSettings.agcValue;
    pSettings->enableDaisyChain = pDevice->deviceSettings.enableDaisyChain;
    pSettings->qamAsyncStatusReadyCallback = pDevice->deviceSettings.qamAsyncStatusReadyCallback;
    pSettings->oobAsyncStatusReadyCallback = pDevice->deviceSettings.oobAsyncStatusReadyCallback;

done:
    return rc;
}

NEXUS_Error NEXUS_FrontendDevice_Set31xxSettings(NEXUS_FrontendDeviceHandle handle, const NEXUS_FrontendDevice31xxSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_31xx *pDevice = NULL;
    unsigned  gainValue;
    BHAB_ConfigSettings habConfigSettings;

    pDevice = handle->pDevice;
    if (!pDevice) { rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto done; }
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);

    if(pDevice->isInternalSetSettings  || (!(pDevice->pGenericDeviceHandle->openPending) && !(pDevice->pGenericDeviceHandle->openFailed))){
        pDevice->isInternalSetSettings = false;

        if(pDevice->updateGainAppCallback) {
            NEXUS_IsrCallback_Set(pDevice->updateGainAppCallback, &(pSettings->updateGainCallback));
            pDevice->deviceSettings.updateGainCallback = pSettings->updateGainCallback;
        }

        if(pDevice->asyncStatusAppCallback[NEXUS_31xx_ADS_CHN]) {
            NEXUS_IsrCallback_Set(pDevice->asyncStatusAppCallback[NEXUS_31xx_ADS_CHN], &(pSettings->qamAsyncStatusReadyCallback));
            pDevice->deviceSettings.qamAsyncStatusReadyCallback = pSettings->qamAsyncStatusReadyCallback;
        }

#ifdef BCHIP_HAS_AOB
        if(pDevice->asyncStatusAppCallback[NEXUS_31xx_OOB_CHN]) {
            NEXUS_IsrCallback_Set(pDevice->asyncStatusAppCallback[NEXUS_31xx_OOB_CHN], &(pSettings->oobAsyncStatusReadyCallback));
            pDevice->deviceSettings.oobAsyncStatusReadyCallback = pSettings->oobAsyncStatusReadyCallback;
        }
#endif

        if (pDevice->tnr[NEXUS_MAX_31xx_ADSCHN])
        {
            rc = BTNR_P_GetTunerAgcRegVal(pDevice->tnr[NEXUS_MAX_31xx_ADSCHN], BCM31xx_DS_TUNER_REF_DB0_WDATA_01, &gainValue);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            gainValue &= 0x00003fff;
            gainValue |= (pSettings->agcValue << 14);
            rc = BTNR_SetTunerAgcRegVal(pDevice->tnr[NEXUS_MAX_31xx_ADSCHN], BCM31xx_DS_TUNER_REF_DB0_WDATA_01, &gainValue);
            if(rc){rc = BERR_TRACE(rc); goto done;}
        }
        pDevice->deviceSettings.agcValue = pSettings->agcValue;

        rc = BHAB_GetConfigSettings(pDevice->hab, &habConfigSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        habConfigSettings.daisyChain = pSettings->enableDaisyChain;

        rc = BHAB_SetConfigSettings(pDevice->hab, &habConfigSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        pDevice->deviceSettings.enableDaisyChain = pSettings->enableDaisyChain;
    }
    else if(pDevice->pGenericDeviceHandle->openPending && !(pDevice->pGenericDeviceHandle->openFailed)){
        pDevice->settingsInitialized = true;
        pDevice->deviceSettings.agcValue = pSettings->agcValue;
        pDevice->deviceSettings.updateGainCallback = pSettings->updateGainCallback;
        pDevice->deviceSettings.enableDaisyChain = pSettings->enableDaisyChain;
        pDevice->deviceSettings.qamAsyncStatusReadyCallback = pSettings->qamAsyncStatusReadyCallback;
        pDevice->deviceSettings.oobAsyncStatusReadyCallback = pSettings->oobAsyncStatusReadyCallback;
    }
    else{
        BDBG_ERR(("Device open failed."));
        rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    }

done:
    return rc;
}

static void NEXUS_Frontend_P_31xx_ResetAdsStatus(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_31xx *pDevice;
    NEXUS_31xxChannel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_31xxChannel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_ASSERT(NULL != pDevice);
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);

    if (pChannel->chn_num >= NEXUS_MAX_31xx_ADSCHN)
    {
        BDBG_ERR((" Unsupported channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    rc = BADS_ResetStatus(pDevice->ads_chn[pChannel->chn_num]);
    if (rc){BERR_TRACE(rc);}

done:
    return;
}


/***************************************************************************
Summary:
    Retrieve the chip family id, chip id, chip version and firmware version.
***************************************************************************/
static void NEXUS_Frontend_P_31xx_GetType(void *handle, NEXUS_FrontendType *type)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    uint32_t       familyId, chipId;
    uint16_t       chipVer;
    uint8_t        majApVer, minApVer;
    NEXUS_31xx *pDevice;
    NEXUS_31xxChannel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_31xxChannel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_ASSERT(NULL != pDevice);
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);
    BDBG_ASSERT(pChannel->chn_num < NEXUS_MAX_31xx_FRONTENDS);

    type->chip.familyId = (uint32_t)pDevice->chipId;
    type->chip.id = (uint32_t)pDevice->chipId;
    type->chip.version.major = (pDevice->revId >> 8) + 1;
    type->chip.version.minor = pDevice->revId & 0xff;

    rc = BHAB_GetApVersion(pDevice->hab, &familyId, &chipId, &chipVer, &majApVer, &minApVer);

    if(rc || (type->chip.id != chipId)){
        BDBG_ERR(("Type mismatch while retreiving chip id and family id."));
        BERR_TRACE(BERR_UNKNOWN); goto done;
    }
    type->firmwareVersion.major = majApVer;
    type->firmwareVersion.minor = minApVer;
done:
    return;
}


#ifdef BCHIP_HAS_AOB
static NEXUS_Error NEXUS_Frontend_P_31xx_TuneOob(void *handle, const NEXUS_FrontendOutOfBandSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BTNR_PowerSaverSettings pwrSettings;
    BAOB_AcquireParam obParams;
    NEXUS_31xx *pDevice;
    NEXUS_31xxChannel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_31xxChannel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_ASSERT(NULL != pDevice);
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);

    if (pChannel->chn_num != NEXUS_31xx_OOB_CHN)
    {
        BDBG_ERR((" Unsupported out of band channel."));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    NEXUS_IsrCallback_Set(pDevice->lockAppCallback[NEXUS_31xx_OOB_CHN], &(pSettings->lockCallback));

    if (pDevice->tnr[NEXUS_31xx_OOB_CHN])
    {
        rc = BTNR_SetTunerRfFreq(pDevice->tnr[NEXUS_31xx_OOB_CHN], pSettings->frequency, BTNR_TunerMode_eDigital);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    if(!pDevice->isTunerPoweredOn[NEXUS_31xx_OOB_CHN]){
        pwrSettings.enable = false;
        rc = BTNR_SetPowerSaver(pDevice->tnr[NEXUS_31xx_OOB_CHN], &pwrSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->isTunerPoweredOn[NEXUS_31xx_OOB_CHN] = true;
    }

    if(!pDevice->isPoweredOn[NEXUS_31xx_OOB_CHN]){
        rc = BAOB_DisablePowerSaver(pDevice->aob);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->isPoweredOn[NEXUS_31xx_OOB_CHN] = true;
    }

    obParams.autoAcquire = pSettings->autoAcquire;
    obParams.modType = (BAOB_ModulationType)pSettings->mode ;
    obParams.symbolRate = pSettings->symbolRate;

    rc = BAOB_Acquire(pDevice->aob, &obParams);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pDevice->last_aob = *pSettings;
done:
    return rc;
}

static void NEXUS_Frontend_P_31xx_UnTuneOob(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_31xx *pDevice;
    NEXUS_31xxChannel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_31xxChannel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_ASSERT(NULL != pDevice);
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);
    BDBG_ASSERT(pChannel->chn_num == NEXUS_31xx_OOB_CHN);

    if(pDevice->isPoweredOn[NEXUS_31xx_OOB_CHN]){
        rc = BAOB_EnablePowerSaver(pDevice->aob);
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->isPoweredOn[NEXUS_31xx_OOB_CHN] = false;
    }
done:
    return;
}

static NEXUS_Error NEXUS_Frontend_P_31xx_GetOobStatus(void *handle, NEXUS_FrontendOutOfBandStatus *pStatus)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    struct BAOB_Status st;
    NEXUS_31xx *pDevice;
    NEXUS_31xxChannel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_31xxChannel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_ASSERT(NULL != pDevice);
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);
    BDBG_ASSERT(pChannel->chn_num == NEXUS_31xx_OOB_CHN);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    rc = BAOB_GetStatus(pDevice->aob,  &st);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pStatus->isFecLocked = st.isFecLock;
    pStatus->isQamLocked = st.isQamLock;
    pStatus->symbolRate = st.symbolRate;
    pStatus->snrEstimate = st.snrEstimate*100/256;
    pStatus->agcIntLevel = st.agcIntLevel;
    pStatus->agcExtLevel = st.agcExtLevel;
    pStatus->carrierFreqOffset = st.carrierFreqOffset;
    pStatus->carrierPhaseOffset = st.carrierPhaseOffset;
    pStatus->correctedCount = st.correctedCount;
    pStatus->uncorrectedCount = st.uncorrectedCount;
    pStatus->mode = (NEXUS_FrontendOutOfBandMode)st.modType;
    pStatus->symbolRate = st.symbolRate;
    pStatus->ifFreq = st.ifFreq;
    pStatus->loFreq = st.loFreq;
    pStatus->sysXtalFreq = st.sysXtalFreq;
    pStatus->berErrorCount = st.berErrorCount;

    BDBG_MSG((" OOB STATUS : fec_lock = %d,  qam_lock = %d, snr_estimate = %d, fec_corr_cnt = %d\n",
            st.isFecLock, st.isQamLock, st.agcIntLevel, st.agcExtLevel, st.snrEstimate,
            st.correctedCount, st.uncorrectedCount, st.berErrorCount));
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_31xx_GetOobAsyncStatus(void *handle, NEXUS_FrontendOutOfBandStatus *pStatus)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    struct BAOB_Status st;
    NEXUS_31xx *pDevice;
    NEXUS_31xxChannel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_31xxChannel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_ASSERT(NULL != pDevice);
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);
    BDBG_ASSERT(pChannel->chn_num == NEXUS_31xx_OOB_CHN);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    rc = BAOB_GetAsyncStatus(pDevice->aob,  &st);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pStatus->isFecLocked = st.isFecLock;
    pStatus->isQamLocked = st.isQamLock;
    pStatus->symbolRate = st.symbolRate;
    pStatus->snrEstimate = st.snrEstimate*100/256;
    pStatus->agcIntLevel = st.agcIntLevel;
    pStatus->agcExtLevel = st.agcExtLevel;
    pStatus->carrierFreqOffset = st.carrierFreqOffset;
    pStatus->carrierPhaseOffset = st.carrierPhaseOffset;
    pStatus->correctedCount = st.correctedCount;
    pStatus->uncorrectedCount = st.uncorrectedCount;
    pStatus->mode = (NEXUS_FrontendOutOfBandMode)st.modType;
    pStatus->symbolRate = st.symbolRate;
    pStatus->ifFreq = st.ifFreq;
    pStatus->loFreq = st.loFreq;
    pStatus->sysXtalFreq = st.sysXtalFreq;
    pStatus->berErrorCount = st.berErrorCount;

    BDBG_MSG((" OOB ASYNC STATUS : fec_lock = %d,  qam_lock = %d, snr_estimate = %d, fec_corr_cnt = %d\n",
            st.isFecLock, st.isQamLock, st.agcIntLevel, st.agcExtLevel, st.snrEstimate,
            st.correctedCount, st.uncorrectedCount, st.berErrorCount));
done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_31xx_RequestOobAsyncStatus(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_31xx *pDevice;
    NEXUS_31xxChannel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_31xxChannel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_ASSERT(NULL != pDevice);
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);
    BDBG_ASSERT(pChannel->chn_num == NEXUS_31xx_OOB_CHN);

    rc = BAOB_RequestAsyncStatus(pDevice->aob);
    if(rc){rc = BERR_TRACE(rc);}

    return rc;
}

static void NEXUS_Frontend_P_31xx_ResetOobStatus(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_31xx *pDevice;
    NEXUS_31xxChannel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_31xxChannel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_ASSERT(NULL != pDevice);
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);
    BDBG_ASSERT(pChannel->chn_num == NEXUS_31xx_OOB_CHN);

    rc = BAOB_ResetStatus(pDevice->aob);
    if(rc) BERR_TRACE(rc);

    return;
}
#endif

#ifdef BCHIP_HAS_AUS
static NEXUS_Error NEXUS_Frontend_P_31xx_TuneUpstream(void *handle, const NEXUS_FrontendUpstreamSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BAUS_OperationMode operationMode;
    BAUS_TestModeSettings testModeSettings;
    NEXUS_31xx *pDevice;
    NEXUS_31xxChannel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_31xxChannel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_ASSERT(NULL != pDevice);
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);
    BDBG_ASSERT(pChannel->chn_num == NEXUS_31xx_AUS_CHN);

    switch ( pSettings->mode )
    {
    case NEXUS_FrontendUpstreamMode_eAnnexA:
        operationMode = BAUS_OperationMode_eAnnexA;
        break;
    case NEXUS_FrontendUpstreamMode_eDvs178:
        operationMode = BAUS_OperationMode_eDvs178;
        break;
    case NEXUS_FrontendUpstreamMode_eDocsis:
        operationMode = BAUS_OperationMode_eDocsis;
        break;
    case NEXUS_FrontendUpstreamMode_ePodAnnexA:
        operationMode = BAUS_OperationMode_ePodAnnexA;
        break;
    case NEXUS_FrontendUpstreamMode_ePodDvs178:
        operationMode = BAUS_OperationMode_ePodDvs178;
        break;
    case NEXUS_FrontendUpstreamMode_ePn23:
        operationMode = BAUS_OperationMode_eTestPn23;
        break;
    case NEXUS_FrontendUpstreamMode_eSingleCw:
        operationMode = BAUS_OperationMode_eTestSingleCw;
        break;
    case NEXUS_FrontendUpstreamMode_eDoubleCw:
        operationMode = BAUS_OperationMode_eTestDoubleCw;
        break;
    default:
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BKNI_Memset(&testModeSettings, 0, sizeof(BAUS_TestModeSettings));

    testModeSettings.nyquistAlphaValue = pSettings->testModeSettings.nyquistAlphaValue;
    testModeSettings.spectrumInverted = pSettings->testModeSettings.spectrumInverted;
    testModeSettings.qamMode = pSettings->testModeSettings.qamMode;
    testModeSettings.bypassFilters = pSettings->testModeSettings.bypassFilters;
    testModeSettings.rfFreq2 = pSettings->testModeSettings.rfFreq2;
    testModeSettings.operationMode = operationMode;

    if(!pDevice->isPoweredOn[NEXUS_31xx_AUS_CHN]){
        rc = BAUS_EnableTransmitter( pDevice->aus );
        if(rc){rc = BERR_TRACE(rc); goto done;}
        pDevice->isPoweredOn[NEXUS_31xx_AUS_CHN] = true;
    }

    rc = BAUS_SetOperationMode(pDevice->aus, operationMode);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    if(pSettings->mode > NEXUS_FrontendUpstreamMode_ePodStarvue ) {
        rc = BAUS_SetTestModeParams(pDevice->aus, &testModeSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    rc = BAUS_SetRfFreq(pDevice->aus, pSettings->frequency);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = BAUS_SetPowerLevel(pDevice->aus, pSettings->powerLevel);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = BAUS_SetSymbolRate(pDevice->aus, pSettings->symbolRate);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pDevice->last_ups = *pSettings;
done:
    return rc;
}

static void NEXUS_Frontend_P_31xx_UnTuneUpstream(void *handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_31xx *pDevice;
    NEXUS_31xxChannel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_31xxChannel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_ASSERT(NULL != pDevice);
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);
    BDBG_ASSERT(pChannel->chn_num == NEXUS_31xx_AUS_CHN);

    if(pDevice->isPoweredOn[NEXUS_31xx_AUS_CHN]){
        rc = BAUS_DisableTransmitter(pDevice->aus);
        if(rc){BERR_TRACE(rc);}
        pDevice->isPoweredOn[NEXUS_31xx_AUS_CHN] = false;
    }
}

static NEXUS_Error NEXUS_Frontend_P_31xx_GetUpstreamStatus(void *handle, NEXUS_FrontendUpstreamStatus *pStatus)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    struct BAUS_Status st;
    NEXUS_31xx *pDevice;
    NEXUS_31xxChannel *pChannel;
    BDBG_ASSERT(NULL != handle);
    pChannel = (NEXUS_31xxChannel *)handle;
    pDevice = pChannel->pDevice;
    BDBG_ASSERT(NULL != pDevice);
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_31xx);
    BDBG_ASSERT(pChannel->chn_num == NEXUS_31xx_AUS_CHN);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    rc = BAUS_GetStatus(pDevice->aus,  &st);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pStatus->frequency = st.rfFreq;
    pStatus->powerLevel = st.powerLevel;
    pStatus->symbolRate = st.symbolRate;
    pStatus->sysXtalFreq = st.sysXtalFreq;

    switch ( st.operationMode )
    {
        case BAUS_OperationMode_eAnnexA:
            pStatus->mode = NEXUS_FrontendUpstreamMode_eAnnexA;
            break;
        case BAUS_OperationMode_eDvs178:
            pStatus->mode = NEXUS_FrontendUpstreamMode_eDvs178;
            break;
        case BAUS_OperationMode_eDocsis:
            pStatus->mode = NEXUS_FrontendUpstreamMode_eDocsis;
            break;
        case BAUS_OperationMode_ePodAnnexA:
            pStatus->mode = NEXUS_FrontendUpstreamMode_ePodAnnexA;
            break;
        case BAUS_OperationMode_ePodDvs178:
            pStatus->mode = NEXUS_FrontendUpstreamMode_ePodDvs178;
            break;
        case BAUS_OperationMode_eTestSingleCw:
            pStatus->mode = NEXUS_FrontendUpstreamMode_eSingleCw;
            break;
        case BAUS_OperationMode_eTestDoubleCw:
            pStatus->mode = NEXUS_FrontendUpstreamMode_eDoubleCw;
            break;
        case BAUS_OperationMode_eTestPn23:
            pStatus->mode = NEXUS_FrontendUpstreamMode_ePn23;
            break;
        default:
            return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
done:
    return rc;
}
#endif


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
#include "nexus_smartcard_module.h"
#include "bscd.h"
#include "bscd_datatypes.h"
#include "priv/nexus_core.h"
#include "nexus_platform_features.h"

BDBG_MODULE(nexus_smartcard);

NEXUS_ModuleHandle g_NEXUS_smartcardModule;

struct {
    NEXUS_SmartcardModuleSettings settings;
    BSCD_Handle scd;
} g_NEXUS_smartcard;

/****************************************
* Module functions
***************/
NEXUS_Error NEXUS_Smartcard_Translate_Error_Code_priv(BERR_Code rc){
	if(rc == BSCD_STATUS_SEND_FAILED)
		return NEXUS_SMARTCARD_SEND_FAILED;
	else if(rc == BSCD_STATUS_PARITY_EDC_ERR)
		return NEXUS_SMARTCARD_PARITY_EDC;
	else if(rc == BSCD_STATUS_READ_FAILED)
		return NEXUS_SMARTCARD_READ_FAILED;
	else if(rc == BSCD_STATUS_TIME_OUT)
		return NEXUS_TIMEOUT;
	else if(rc == BSCD_STATUS_DEACTIVATE)
		return NEXUS_SMARTCARD_DEACTIVATED;
	else if(rc == BSCD_STATUS_NO_SC_RESPONSE)
		return NEXUS_SMARTCARD_NO_SC_RESPONSE;
	else if(rc)
		return NEXUS_UNKNOWN;
	else
		return NEXUS_SUCCESS;
}

void NEXUS_SmartcardModule_GetDefaultSettings(NEXUS_SmartcardModuleSettings *pSettings)
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_GetDefaultCommonModuleSettings(&pSettings->common);
    pSettings->common.standbyLevel = NEXUS_ModuleStandbyLevel_eActive;
}

void NEXUS_SmartcardModule_GetDefaultInternalSettings(NEXUS_SmartcardModuleInternalSettings *pSettings)
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_ModuleHandle NEXUS_SmartcardModule_Init( const NEXUS_SmartcardModuleInternalSettings *pModuleSettings, const NEXUS_SmartcardModuleSettings *pSettings)
{
    BSCD_Settings scdSettings;
    NEXUS_Error errCode;
    NEXUS_ModuleSettings moduleSettings;

    BDBG_ASSERT(pModuleSettings);
    BDBG_ASSERT(pSettings);

    BDBG_ASSERT(!g_NEXUS_smartcardModule);
    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_AdjustModulePriority(NEXUS_ModulePriority_eLow, &pSettings->common);
    g_NEXUS_smartcardModule = NEXUS_Module_Create("smartcard", &moduleSettings);
    if(!g_NEXUS_smartcardModule) {
        return NULL;
    }
    g_NEXUS_smartcard.settings = *pSettings;
    BSCD_GetDefaultSettings(&scdSettings, g_pCoreHandles->chp);
    scdSettings.moduleClkFreq.FreqSrc = (BSCD_ClockFreqSrc) g_NEXUS_smartcard.settings.clockSource;
    scdSettings.moduleClkFreq.ulClkFreq = g_NEXUS_smartcard.settings.clockFrequency;
    scdSettings.moduleClkFreq.bIsUsingOsc = g_NEXUS_smartcard.settings.externalOscillatior;
    scdSettings.moduleClkFreq.bIsRoutedInternal = g_NEXUS_smartcard.settings.routedInternal;
    errCode = BSCD_Open(&g_NEXUS_smartcard.scd, g_pCoreHandles->reg, g_pCoreHandles->chp, g_pCoreHandles->bint, &scdSettings);
    if (errCode)
    {
        NEXUS_Module_Destroy(g_NEXUS_smartcardModule);
        g_NEXUS_smartcardModule = NULL;

        BERR_TRACE(NEXUS_UNKNOWN);
        return NULL;
    }

    return g_NEXUS_smartcardModule;
}

void NEXUS_SmartcardModule_Uninit()
{
    BSCD_Close(g_NEXUS_smartcard.scd);
    NEXUS_Module_Destroy(g_NEXUS_smartcardModule);
    g_NEXUS_smartcardModule = NULL;
}

/****************************************
* API functions
***************/

typedef struct NEXUS_Smartcard {
    NEXUS_OBJECT(NEXUS_Smartcard);
    bool opened;
    BSCD_ChannelHandle channelHandle; /* scd channel */
    BSCD_ChannelSettings channelSettings;
    NEXUS_SmartcardSettings settings;
    NEXUS_SmartcardState state;
    NEXUS_IsrCallbackHandle cardAppCallback;
} NEXUS_Smartcard;

static NEXUS_Smartcard g_NEXUS_smartcards[BSCD_MAX_SUPPOTED_CHANNELS];

void NEXUS_Smartcard_GetDefaultSettings(NEXUS_SmartcardSettings *pSettings)
{
    if ( pSettings == NULL )
    {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return;
    }
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    pSettings->standard = NEXUS_SmartcardStandard_eNds;
    pSettings->protocol = NEXUS_SmartcardProtocol_eT0;
    pSettings->fFactor = 1;
    pSettings->dFactor = 1;
    pSettings->extClockDivisor = 1;
    pSettings->txRetries = 4;
    pSettings->rxRetries = 4;
    pSettings->workWaitTime.value = 9600;
    pSettings->blockWaitTime.value = 971;
    pSettings->extraGuardTime.value= 2;
    pSettings->blockGuardTime.value = 22;
    pSettings->timeOut.value= BSCD_DEFAULT_TIME_OUT;
    pSettings->timeOut.unit= NEXUS_TimerUnit_eMilliSec;
    pSettings->scPresDbInfo.dbWidth = 7;
    pSettings->scPresDbInfo.scPresMode = NEXUS_ScPresMode_eMask;
    pSettings->isPresHigh = true;
    pSettings->setPinmux = false;
    pSettings->setVcc = true;
    pSettings->vcc = NEXUS_SmartcardVcc_e5V;
    pSettings->sourceClockFreq = 27000000;
    pSettings->atrReceiveTime.value = 40000;
    pSettings->atrReceiveTime.unit = NEXUS_TimerUnit_eClk;
    pSettings->resetCycles = 42000;
    NEXUS_CallbackDesc_Init(&pSettings->cardCallback);
}

void NEXUS_Smartcard_GetSettings(NEXUS_SmartcardHandle smartcard, NEXUS_SmartcardSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(smartcard, NEXUS_Smartcard);
    *pSettings = smartcard->settings;
}

NEXUS_Error NEXUS_Smartcard_SetSettings(NEXUS_SmartcardHandle smartcard, const NEXUS_SmartcardSettings *pSettings)
{
    NEXUS_Error errCode;
    BSCD_ChannelSettings *pChSettings;

    BDBG_OBJECT_ASSERT(smartcard, NEXUS_Smartcard);

    if ( pSettings == NULL )
    {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    pChSettings = &smartcard->channelSettings;

    errCode = BSCD_Channel_GetParameters(smartcard->channelHandle, pChSettings);
    if ( errCode ) {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }

    if (pSettings->standard == NEXUS_SmartcardStandard_eNds || pSettings->standard == NEXUS_SmartcardStandard_eIso ||
        pSettings->standard == NEXUS_SmartcardStandard_eIrdeto  || pSettings->standard == NEXUS_SmartcardStandard_eNordig) {
        BDBG_CASSERT(NEXUS_SmartcardStandard_eNordig == (NEXUS_SmartcardStandard)BSCD_Standard_eNordig);
        pChSettings->scStandard = pSettings->standard;
    } else {
        BDBG_ERR(("The requested standard is not supported at this time"));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    /* This is hard-coding due to PI requirements.
            This way the PI picks up the right values based on the F and D factors. */

    pChSettings->ucEtuClkDiv = 0;
    pChSettings->ucScClkDiv = 0;
    pChSettings->unPrescale = 0;
    pChSettings->ucBaudDiv = 0;


    BDBG_CASSERT(NEXUS_SmartcardProtocol_eT14 == (NEXUS_SmartcardProtocol)BSCD_AsyncProtocolType_e14_IRDETO);
    pChSettings->eProtocolType = pSettings->protocol;

    pChSettings->ucFFactor = pSettings->fFactor;
    pChSettings->ucDFactor = pSettings->dFactor;
    pChSettings->ucExternalClockDivisor = pSettings->extClockDivisor;
    pChSettings->ucTxRetries = pSettings->txRetries;
    pChSettings->ucRxRetries = pSettings->rxRetries;
    pChSettings->ucBaudDiv = pSettings->baudDiv;
    pChSettings->unCurrentIFSD = pSettings->currentIFSD;

    BDBG_CASSERT(NEXUS_TimerUnit_eMilliSec == (NEXUS_TimerUnit)BSCD_TimerUnit_eMilliSec);
    pChSettings->workWaitTime.ulValue = pSettings->workWaitTime.value;
    pChSettings->workWaitTime.unit = pSettings->workWaitTime.unit;
    pChSettings->blockWaitTime.ulValue = pSettings->blockWaitTime.value;
    pChSettings->blockWaitTime.unit = pSettings->blockWaitTime.unit;
    pChSettings->extraGuardTime.ulValue = pSettings->extraGuardTime.value;
    pChSettings->extraGuardTime.unit = pSettings->extraGuardTime.unit;
    pChSettings->blockGuardTime.ulValue = pSettings->blockGuardTime.value;
    pChSettings->blockGuardTime.unit = pSettings->blockGuardTime.unit;
    pChSettings->ulCharacterWaitTimeInteger = pSettings->characterWaitTime;

    BDBG_CASSERT(NEXUS_EdcEncode_eCrc == (NEXUS_EdcEncode)BSCD_EDCEncode_eCRC);
    pChSettings->edcSetting.edcEncode = pSettings->edcSetting.edcEncode;
    pChSettings->edcSetting.bIsEnabled = pSettings->edcSetting.isEnabled;
    pChSettings->timeOut.ulValue = pSettings->timeOut.value;
    pChSettings->timeOut.unit = pSettings->timeOut.unit;
    pChSettings->bAutoDeactiveReq = pSettings->autoDeactiveReq;
    pChSettings->bNullFilter = pSettings->nullFilter;

    BDBG_CASSERT(NEXUS_ScPresMode_eMask == (NEXUS_ScPresMode)BSCD_ScPresMode_eMask);
    pChSettings->scPresDbInfo.scPresMode = pSettings->scPresDbInfo.scPresMode;
    pChSettings->scPresDbInfo.bIsEnabled = pSettings->scPresDbInfo.isEnabled;
    pChSettings->scPresDbInfo.ucDbWidth = pSettings->scPresDbInfo.dbWidth;

    BDBG_CASSERT(NEXUS_ResetCardAction_eReceiveAndDecode == (NEXUS_ResetCardAction)BSCD_ResetCardAction_eReceiveAndDecode);
    pChSettings->resetCardAction = pSettings->resetCardAction;
    pChSettings->blockWaitTimeExt.ulValue = pSettings->blockWaitTimeExt.value;
    pChSettings->blockWaitTimeExt.unit = pSettings->blockWaitTimeExt.unit;
    pChSettings->bIsPresHigh = pSettings->isPresHigh;
    pChSettings->setPinmux = pSettings->setPinmux;
    pChSettings->setVcc = pSettings->setVcc;
    pChSettings->vcc = pSettings->vcc;
    pChSettings->srcClkFreqInHz = pSettings->sourceClockFreq;
    pChSettings->ATRRecvTimeInteger.ulValue = pSettings->atrReceiveTime.value;
    pChSettings->ATRRecvTimeInteger.unit = pSettings->atrReceiveTime.unit;
    if(!pSettings->connectDirectly){
        if(pSettings->connection == NEXUS_SmartcardConnection_eDirect){
            smartcard->channelSettings.bConnectDirectly = true;
        }
        else if((pSettings->connection == NEXUS_SmartcardConnection_eTda8034) || (pSettings->connection == NEXUS_SmartcardConnection_eTda8024)){
            smartcard->channelSettings.bConnectDirectly = false;
            if(pSettings->connection == NEXUS_SmartcardConnection_eTda8024){
                smartcard->channelSettings.eResetCycles = BSCD_MAX_RESET_IN_CLK_CYCLES;
            }
            else{
                smartcard->channelSettings.eResetCycles = BSCD_TDA803X_MAX_RESET_IN_CLK_CYCLES;
            }
        }
        else{
            BDBG_WRN(("NEXUS_SmartcardConnection_eInternal is not supported"));
            return NEXUS_NOT_SUPPORTED;
        }
    }
    else {
        BDBG_WRN(("Use of connectDirectly should be deprecated. Use NEXUS_SmartcardConnection enum in NEXUS_SmartcardSettings instead."));
        smartcard->channelSettings.bConnectDirectly = true;
    }
    smartcard->channelSettings.bConnectDirectly = pChSettings->bConnectDirectly;
    smartcard->channelSettings.bDirectVccInverted = pSettings->directPowerSupply.vccInverted;
    smartcard->channelSettings.bDirectRstInverted = pSettings->directPowerSupply.resetInverted;
    switch (pSettings->resetCycles)
    {
        case 42000:
            pChSettings->eResetCycles = BSCD_MAX_RESET_IN_CLK_CYCLES;
            break;
        case 60000:
            pChSettings->eResetCycles = BSCD_TDA803X_MAX_RESET_IN_CLK_CYCLES;
            break;
        default:
            pChSettings->eResetCycles = BSCD_MAX_RESET_IN_CLK_CYCLES;
    }

    NEXUS_IsrCallback_Set(smartcard->cardAppCallback, &pSettings->cardCallback);

    errCode = BSCD_Channel_SetParameters(smartcard->channelHandle, pChSettings);
    if ( errCode ) {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }

    BSCD_Channel_EnableInterrupts(smartcard->channelHandle);

    smartcard->settings = *pSettings;

    return NEXUS_SUCCESS;
}

static void NEXUS_Smartcard_P_CardInsertedRemoved_isr(BSCD_ChannelHandle channelHandle, void * pData )
{
    int index;

    /* There is no way of setting pData to the smartcard handle, so we must look it up. */
    BSTD_UNUSED(pData);
    for (index=0; index < BSCD_MAX_SUPPOTED_CHANNELS; index++) {
        if (g_NEXUS_smartcards[index].channelHandle == channelHandle) {
            if ( g_NEXUS_smartcards[index].settings.cardCallback.callback) {
                NEXUS_Module_IsrCallback_Fire_isr(g_NEXUS_smartcardModule, g_NEXUS_smartcards[index].cardAppCallback);
            }
        }
    }
}

static NEXUS_Error NEXUS_Smartcard_P_Open(NEXUS_SmartcardHandle smartcard, unsigned index, const NEXUS_SmartcardSettings *pSettings)
{
    NEXUS_Error errCode;

    errCode = BSCD_GetChannelDefaultSettings(g_NEXUS_smartcard.scd, index, &smartcard->channelSettings);
    if ( errCode )
    {
    errCode = BERR_TRACE(errCode);
    goto error;
    }

    smartcard->channelSettings.ucEtuClkDiv = 0;
    smartcard->channelSettings.ucScClkDiv = 0;
    smartcard->channelSettings.unPrescale = 0;
    smartcard->channelSettings.ucBaudDiv = 0;
    smartcard->channelSettings.setVcc = pSettings->setVcc;
    smartcard->channelSettings.vcc = pSettings->vcc;
    smartcard->channelSettings.setPinmux = pSettings->setPinmux;
    smartcard->channelSettings.eProtocolType = pSettings->protocol;
    smartcard->channelSettings.scStandard = pSettings->standard;
    smartcard->channelSettings.ucFFactor = pSettings->fFactor;
    smartcard->channelSettings.ucDFactor = pSettings->dFactor;
    smartcard->channelSettings.ucExternalClockDivisor = pSettings->extClockDivisor;
    smartcard->channelSettings.ucTxRetries = pSettings->txRetries;
    smartcard->channelSettings.ucRxRetries = pSettings->rxRetries;
    smartcard->channelSettings.ucBaudDiv = pSettings->baudDiv;
    BDBG_CASSERT(NEXUS_ResetCardAction_eReceiveAndDecode == (NEXUS_ResetCardAction)BSCD_ResetCardAction_eReceiveAndDecode);
    smartcard->channelSettings.resetCardAction = pSettings->resetCardAction;
    smartcard->channelSettings.unCurrentIFSD = pSettings->currentIFSD;

    smartcard->channelSettings.ulCharacterWaitTimeInteger = pSettings->characterWaitTime;
    smartcard->channelSettings.bAutoDeactiveReq = pSettings->autoDeactiveReq;
    smartcard->channelSettings.bNullFilter = pSettings->nullFilter;
    smartcard->channelSettings.bIsPresHigh = pSettings->isPresHigh;

    NEXUS_IsrCallback_Set(smartcard->cardAppCallback, &pSettings->cardCallback);

    smartcard->channelSettings.workWaitTime.ulValue = pSettings->workWaitTime.value;
    smartcard->channelSettings.workWaitTime.unit = pSettings->workWaitTime.unit;
    smartcard->channelSettings.blockWaitTime.ulValue = pSettings->blockWaitTime.value;
    smartcard->channelSettings.blockWaitTime.unit = pSettings->blockWaitTime.unit;
    smartcard->channelSettings.extraGuardTime.ulValue = pSettings->extraGuardTime.value;
    smartcard->channelSettings.extraGuardTime.unit = pSettings->extraGuardTime.unit;
    smartcard->channelSettings.blockGuardTime.ulValue = pSettings->blockGuardTime.value;
    smartcard->channelSettings.blockGuardTime.unit = pSettings->blockGuardTime.unit;
    smartcard->channelSettings.edcSetting.edcEncode = pSettings->edcSetting.edcEncode;
    smartcard->channelSettings.edcSetting.bIsEnabled = pSettings->edcSetting.isEnabled;
    smartcard->channelSettings.timeOut.ulValue = pSettings->timeOut.value;
    smartcard->channelSettings.timeOut.unit = pSettings->timeOut.unit;
    smartcard->channelSettings.scPresDbInfo.scPresMode = pSettings->scPresDbInfo.scPresMode;
    smartcard->channelSettings.scPresDbInfo.bIsEnabled = pSettings->scPresDbInfo.isEnabled;
    smartcard->channelSettings.scPresDbInfo.ucDbWidth = pSettings->scPresDbInfo.dbWidth;
    smartcard->channelSettings.blockWaitTimeExt.ulValue = pSettings->blockWaitTimeExt.value;
    smartcard->channelSettings.blockWaitTimeExt.unit = pSettings->blockWaitTimeExt.unit;
    smartcard->channelSettings.srcClkFreqInHz = pSettings->sourceClockFreq;
    smartcard->channelSettings.ATRRecvTimeInteger.ulValue = pSettings->atrReceiveTime.value;
    smartcard->channelSettings.ATRRecvTimeInteger.unit = pSettings->atrReceiveTime.unit;
    /*
            To keep backward compatibility between connectDirectly and the NEXUS_SmartcardConnection enum connection, this is the logic being used:

            connectDirectly     connection             - Action
            ----------------------------------------------------------------
            true                    0/non-zero      - use old api, print WRN

            false (default)         0 (default)     - default, use new api, no WRN

            false                   non-zero        - use new api

    */
    if(!pSettings->connectDirectly){
        if(pSettings->connection == NEXUS_SmartcardConnection_eDirect){
            smartcard->channelSettings.bConnectDirectly = true;
        }
        else if((pSettings->connection == NEXUS_SmartcardConnection_eTda8034) || (pSettings->connection == NEXUS_SmartcardConnection_eTda8024)){
            smartcard->channelSettings.bConnectDirectly = false;
            if(pSettings->connection == NEXUS_SmartcardConnection_eTda8024){
                smartcard->channelSettings.eResetCycles = BSCD_MAX_RESET_IN_CLK_CYCLES;
            }
            else{
                smartcard->channelSettings.eResetCycles = BSCD_TDA803X_MAX_RESET_IN_CLK_CYCLES;
            }
        }
        else{
            BDBG_WRN(("NEXUS_SmartcardConnection_eInternal is not supported"));
            return NEXUS_NOT_SUPPORTED;
        }
    }
    else {
        BDBG_WRN(("Use of connectDirectly should be deprecated. Use NEXUS_SmartcardConnection enum in NEXUS_SmartcardSettings instead."));
        smartcard->channelSettings.bConnectDirectly = true;
    }
    smartcard->channelSettings.bConnectDirectly = pSettings->connectDirectly;
    smartcard->channelSettings.bDirectVccInverted = pSettings->directPowerSupply.vccInverted;
    smartcard->channelSettings.bDirectRstInverted = pSettings->directPowerSupply.resetInverted;

    switch (pSettings->resetCycles)
    {
        case 42000:
            smartcard->channelSettings.eResetCycles = BSCD_MAX_RESET_IN_CLK_CYCLES;
            break;
        case 60000:
            smartcard->channelSettings.eResetCycles = BSCD_TDA803X_MAX_RESET_IN_CLK_CYCLES;
            break;
        default:
            smartcard->channelSettings.eResetCycles = BSCD_MAX_RESET_IN_CLK_CYCLES;
    }


    smartcard->settings = *pSettings;

    errCode = BSCD_Channel_Open(g_NEXUS_smartcard.scd, &smartcard->channelHandle, index, &smartcard->channelSettings);
    if ( errCode ) {
    BERR_TRACE(NEXUS_UNKNOWN);
    goto error;
    }

    BKNI_EnterCriticalSection();
    errCode = BSCD_Channel_EnableIntrCallback_isr(smartcard->channelHandle,BSCD_IntType_eCardInsertInt,NEXUS_Smartcard_P_CardInsertedRemoved_isr);
    BKNI_LeaveCriticalSection();
    if ( errCode ) {
    BERR_TRACE(NEXUS_UNKNOWN);
    goto error;
    }
    BKNI_EnterCriticalSection();
    errCode = BSCD_Channel_EnableIntrCallback_isr(smartcard->channelHandle,BSCD_IntType_eCardRemoveInt,NEXUS_Smartcard_P_CardInsertedRemoved_isr);
    BKNI_LeaveCriticalSection();
    if ( errCode ) {
    BERR_TRACE(NEXUS_UNKNOWN);
    goto error;
    }

error:
    return errCode;

}

NEXUS_SmartcardHandle NEXUS_Smartcard_Open(unsigned index, const NEXUS_SmartcardSettings *pSettings)
{
    NEXUS_Error errCode;
    unsigned char totalChannels;
    NEXUS_SmartcardHandle smartcard;

    if ( pSettings == NULL )
    {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }
    if (index >= BSCD_MAX_SUPPOTED_CHANNELS) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }

    smartcard = &g_NEXUS_smartcards[index];

    if ( smartcard->opened == true )
    {
        BDBG_ERR(("Smartcard %d is already open", index));
        errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        return NULL;
    }

    BSCD_GetTotalChannels(g_NEXUS_smartcard.scd, &totalChannels);
    if ( index >= totalChannels)
    {
        BDBG_ERR(("unable to open Smartcard[%d]", index));
        errCode = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }

    if (pSettings->standard == NEXUS_SmartcardStandard_eNds || pSettings->standard == NEXUS_SmartcardStandard_eIso ||
        pSettings->standard == NEXUS_SmartcardStandard_eIrdeto || pSettings->standard == NEXUS_SmartcardStandard_eNordig) {

        NEXUS_OBJECT_INIT(NEXUS_Smartcard, smartcard);
        smartcard->opened = true;
        smartcard->cardAppCallback = NEXUS_IsrCallback_Create(smartcard, NULL);

        smartcard->state = NEXUS_SmartcardState_eUnknown;

    errCode = NEXUS_Smartcard_P_Open(smartcard, index, pSettings);
    if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto error;
        }
    } else {
        BDBG_ERR(("The requested standard is not supported at this time"));
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    return smartcard;

error:
    NEXUS_Smartcard_Close(smartcard);
    return NULL;
}

static void NEXUS_Smartcard_P_Close(NEXUS_SmartcardHandle smartcard)
{
    if (smartcard->channelHandle)
    {
        BKNI_EnterCriticalSection();
        BSCD_Channel_DisableIntrCallback_isr(smartcard->channelHandle,BSCD_IntType_eCardInsertInt);
        BKNI_LeaveCriticalSection();

        BKNI_EnterCriticalSection();
        BSCD_Channel_DisableIntrCallback_isr(smartcard->channelHandle,BSCD_IntType_eCardRemoveInt);
        BKNI_LeaveCriticalSection();
        if(BSCD_Channel_Close(smartcard->channelHandle))
        {
            BERR_TRACE(NEXUS_UNKNOWN);
        }
    }
}

static void NEXUS_Smartcard_P_Finalizer(NEXUS_SmartcardHandle smartcard)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Smartcard, smartcard);

    NEXUS_Smartcard_P_Close(smartcard);

    smartcard->state = NEXUS_SmartcardState_eUnknown;
    NEXUS_IsrCallback_Destroy(smartcard->cardAppCallback);

    NEXUS_OBJECT_UNSET(NEXUS_Smartcard, smartcard);
    BKNI_Memset(smartcard, 0, sizeof(*smartcard));
    return;
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_Smartcard, NEXUS_Smartcard_Close);

NEXUS_Error NEXUS_Smartcard_Reset(NEXUS_SmartcardHandle smartcard, bool warmReset)
{
    BDBG_OBJECT_ASSERT(smartcard, NEXUS_Smartcard);

    smartcard->state = warmReset ? NEXUS_SmartcardState_eWarmResetting : NEXUS_SmartcardState_eColdResetting;
    if (BSCD_Channel_ResetIFD(smartcard->channelHandle, warmReset ? BSCD_ResetType_eWarm : BSCD_ResetType_eCold)) {
        smartcard->state = NEXUS_SmartcardState_eResetDone;
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
    smartcard->state = NEXUS_SmartcardState_eResetDone;

    return NEXUS_SUCCESS;
}


NEXUS_Error NEXUS_Smartcard_Read(NEXUS_SmartcardHandle smartcard, void *pData,  size_t numBytes, size_t *pBytesRead)
{
    BERR_Code rc=NEXUS_SUCCESS;
    BDBG_OBJECT_ASSERT(smartcard, NEXUS_Smartcard);

    smartcard->state = NEXUS_SmartcardState_eReceiving;
    rc = BSCD_Channel_Receive(smartcard->channelHandle, pData, (unsigned long *) pBytesRead, numBytes);
    if (rc) {
        smartcard->state = NEXUS_SmartcardState_eReady;
        rc = NEXUS_Smartcard_Translate_Error_Code_priv(rc);
        return BERR_TRACE(rc);
    }

    smartcard->state = NEXUS_SmartcardState_eReady;
    if (*pBytesRead <= 0) {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Smartcard_Write(NEXUS_SmartcardHandle smartcard, const void *pData, size_t numBytes, size_t *pBytesWritten)
{
    BERR_Code rc=NEXUS_SUCCESS;
    BDBG_OBJECT_ASSERT(smartcard, NEXUS_Smartcard);

    smartcard->state = NEXUS_SmartcardState_eTransmitting;
    rc = BSCD_Channel_Transmit(smartcard->channelHandle, (uint8_t *) pData, numBytes);

    if (rc) {
        *pBytesWritten = 0;
        smartcard->state = NEXUS_SmartcardState_eTransmitted;
        rc = NEXUS_Smartcard_Translate_Error_Code_priv(rc);
        return BERR_TRACE(rc);
    }
    else {
        *pBytesWritten = numBytes;
        smartcard->state = NEXUS_SmartcardState_eTransmitted;
        return NEXUS_SUCCESS;
    }
}

NEXUS_Error NEXUS_Smartcard_GetStatus(NEXUS_SmartcardHandle smartcard, NEXUS_SmartcardStatus *pStatus)
{
    BSCD_Status  internal_status;
    BSCD_ChannelSettings internal_settings;
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(smartcard, NEXUS_Smartcard);
    BDBG_ASSERT(NULL != pStatus);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    rc = BSCD_Channel_GetStatus(smartcard->channelHandle, &internal_status);
    if (rc) return BERR_TRACE(rc);

    pStatus->cardPresent = internal_status.bCardPresent;
    pStatus->state = smartcard->state;

    /* some SCD settings are actually read-only status */
    rc = BSCD_Channel_GetParameters(smartcard->channelHandle, &internal_settings);
    if (rc) return BERR_TRACE(rc);

    pStatus->iccClockFrequency = internal_settings.currentICCClkFreq;

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Smartcard_DetectCard(NEXUS_SmartcardHandle smartcard)
{
    BDBG_OBJECT_ASSERT(smartcard, NEXUS_Smartcard);

    if (BSCD_Channel_DetectCard(smartcard->channelHandle, BSCD_CardPresent_eInserted)) {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Smartcard_ResetCard(NEXUS_SmartcardHandle smartcard, void *pData, size_t numBytes, size_t *pBytesRead)
{
    uint8_t size = 0;
    unsigned long readCount=0;
    unsigned char TD1 = 0;
    unsigned char check_TD1 = 0, current_td = 0;
    unsigned char history_bytes = 0;
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(smartcard, NEXUS_Smartcard);
    smartcard->state = NEXUS_SmartcardState_eUnknown;

    /* The requirement is that the minimum interval between two card reset is 10ms. Hence we force it. */
    BKNI_Sleep(10);

    rc = BSCD_Channel_ResetCard(smartcard->channelHandle, smartcard->channelSettings.resetCardAction);
    if(rc) {
        rc = NEXUS_Smartcard_Translate_Error_Code_priv(rc);
        return BERR_TRACE(rc);
    }

    if(numBytes == 0){
        return NEXUS_SUCCESS;
    }

    if (pData) {
        rc = BSCD_Channel_Receive(smartcard->channelHandle, pData, &readCount, numBytes);
        rc = NEXUS_Smartcard_Translate_Error_Code_priv(rc);
        if(rc == NEXUS_TIMEOUT) {
            BDBG_WRN(("Timeout occured while reading %d bytes of ATR data. Number of bytes read is %d. Continuing Smartcard reset.", (unsigned)numBytes, (unsigned)readCount));
        }
        else if (rc != NEXUS_SUCCESS) {
            return BERR_TRACE(rc);
        }

        if(readCount < 2){
            BDBG_ERR(("Minimum ATR size is two bytes. Data read (%u bytes) is less than minimum ATR size of two bytes.", (unsigned)readCount));
            return BERR_TRACE(NEXUS_UNKNOWN);
        }

        current_td = ((unsigned char*)pData)[1]; /* T0 */
        history_bytes = current_td & 0xf;
        size = 2;

        do{
            if (current_td  & 0x10 ) {
                size++;
            }

            if (current_td  & 0x20 ) {
                size++;
            }

            if (current_td  & 0x40 ) {
                size++;
            }

            if (current_td  & 0x80 ) {
                {
                    if(readCount > size) {
                        current_td = ((unsigned char*)pData)[size];
                        size++;
                    } else {
                        BDBG_WRN(("The requested ATR of size = %u bytes could not be received, received ATR size = %u bytes and decoded ATR size = %d bytes.", (unsigned) numBytes, (unsigned) *pBytesRead, size));
                        return BERR_TRACE(NEXUS_UNKNOWN);
                    }
                }

                if(check_TD1 == 0)
                {
                    TD1 = current_td;
                }
            }
            else
            {
                current_td =0;
            }
            check_TD1 = 1;
        } while(current_td != 0);

        size += history_bytes;

        /* TCK */
        if( (TD1 & 0xF) != 0)
        {
            size++;
        }
    } else {
        BDBG_ERR(("Null Pointer passed for pData."));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

        if(readCount >= size)
        {
            BDBG_WRN(("The requested ATR size = %u bytes, received ATR size = %u bytes and decoded ATR size = %d bytes.", (unsigned) numBytes, (unsigned) readCount, size));
        } else
        {
        BDBG_WRN(("The requested ATR of size = %u bytes could not be received, received ATR size = %u bytes and decoded ATR size = %d bytes.", (unsigned) numBytes, (unsigned) readCount, size));
            return BERR_TRACE(NEXUS_UNKNOWN);
        }

    *pBytesRead = readCount;
    smartcard->state = NEXUS_SmartcardState_eResetDone;

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Smartcard_PowerIcc (NEXUS_SmartcardHandle smartcard, NEXUS_SmartcardPowerIcc iccAction)
{
    NEXUS_Error errCode = NEXUS_SUCCESS;
    BDBG_OBJECT_ASSERT(smartcard, NEXUS_Smartcard);

    if (iccAction) {
        errCode = BSCD_Channel_PowerICC(smartcard->channelHandle, BSCD_PowerICC_ePowerUp);

    }
    else {
        errCode = BSCD_Channel_PowerICC(smartcard->channelHandle, BSCD_PowerICC_ePowerDown);
    }

    smartcard->state = NEXUS_SmartcardState_eUnknown;
    return errCode;
}

NEXUS_Error NEXUS_Smartcard_Deactivate (NEXUS_SmartcardHandle smartcard)
{
    BDBG_OBJECT_ASSERT(smartcard, NEXUS_Smartcard);

    if (BSCD_Channel_Deactivate(smartcard->channelHandle)){
        return BERR_TRACE(NEXUS_UNKNOWN);
    }

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_SmartcardModule_Standby_priv(bool enabled, const NEXUS_StandbySettings *pSettings)
{
#if NEXUS_POWER_MANAGEMENT
    NEXUS_Error errCode = NEXUS_SUCCESS;
    unsigned i;
    BSTD_UNUSED(pSettings);

    for (i=0; i<sizeof(g_NEXUS_smartcards)/sizeof(g_NEXUS_smartcards[0]); i++) {
        NEXUS_SmartcardHandle smartcard = &g_NEXUS_smartcards[i];
        if (!smartcard->opened) { continue; }

        if (enabled) {
            NEXUS_Smartcard_P_Close(smartcard);
        }
        else {
            errCode = NEXUS_Smartcard_P_Open(smartcard, i, &smartcard->settings);
            if (errCode) { BERR_TRACE(NEXUS_UNKNOWN); return errCode; }
            /* after resume, the app must call NEXUS_Smartcard_SetSettings(), just like
               it must do after NEXUS_Smartcard_Open(). the smartcard API is weird like that */
        }
    }

    return errCode;
#else
    BSTD_UNUSED(enabled);
    BSTD_UNUSED(pSettings);
    return NEXUS_SUCCESS;
#endif
}

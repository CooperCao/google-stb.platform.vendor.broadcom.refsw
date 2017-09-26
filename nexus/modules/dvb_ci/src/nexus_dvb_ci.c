/***************************************************************************
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
*
* API Description:
*   API name: DvbCi
*    Specific APIs related DVB Common Interface
*
***************************************************************************/

#include "nexus_dvb_ci_module.h"

BDBG_MODULE(nexus_dvb_ci);

static NEXUS_Error NEXUS_DvbCi_P_InitializeSlot(NEXUS_DvbCiHandle handle);
static void NEXUS_DvbCi_P_EnableInterface(NEXUS_DvbCiHandle handle, bool enabled);
static void NEXUS_DvbCi_P_CdInterrupt_nosync(void *pParam, int unused);
static void NEXUS_DvbCi_P_ReadyInterrupt_nosync(void *pParam, int unused);
static void NEXUS_DvbCi_P_ReadyTimeout(void *pParam);
static void NEXUS_DvbCi_P_ReadyCallback(void *pParam);
static void NEXUS_DvbCi_P_CheckCardStatus(void *pParam);
static void NEXUS_DvbCi_P_WriteCommandReg(NEXUS_DvbCiHandle handle, uint8_t value);

/***************************************************************************
Summary:
    Get default settings to open a DVB-CI device

Description:
    This function allows for future expansion of the structure

***************************************************************************/
void NEXUS_DvbCi_GetDefaultOpenSettings(
    NEXUS_DvbCiOpenSettings *pSettings      /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->littleEndian = true;
    pSettings->readyInterrupt = pSettings->cd1Interrupt = pSettings->cd2Interrupt = 0xffffffff;
}

/***************************************************************************
Summary:
    Open a DVB-CI device
***************************************************************************/
NEXUS_DvbCiHandle NEXUS_DvbCi_Open(
    const NEXUS_DvbCiOpenSettings *pSettings
    )
{
    NEXUS_Error errCode;
    NEXUS_DvbCiHandle handle;

#ifndef NEXUS_DVB_CI_EXTENSION
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(0 != pSettings->deviceOffset);
    BDBG_ASSERT(pSettings->ebiChipSelect <= 5);
    BDBG_ASSERT(NULL != pSettings->readyPin || pSettings->readyInterrupt < 32);
    BDBG_ASSERT(NULL != pSettings->cd1Pin || pSettings->cd1Interrupt < 32);
    /* cd2 is optional */
    BDBG_ASSERT(NULL != pSettings->resetPin);
#endif

    handle = BKNI_Malloc(sizeof(NEXUS_DvbCi));
    if ( NULL == handle )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    NEXUS_OBJECT_INIT(NEXUS_DvbCi, handle);
    handle->errorCallback = NEXUS_TaskCallback_Create(handle, NULL);
    handle->cardDetectCallback = NEXUS_TaskCallback_Create(handle, NULL);
    handle->ireqCallback = NEXUS_TaskCallback_Create(handle, NULL);
    handle->fpgaTristate = handle->fpgaPcmciaMode = true;
    NEXUS_CallbackDesc_Init(&handle->settings.errorCallback);
    NEXUS_CallbackDesc_Init(&handle->settings.cardDetectCallback);
    NEXUS_CallbackDesc_Init(&handle->settings.interruptCallback);

#ifndef NEXUS_DVB_CI_EXTENSION
    handle->pBaseAddress = NEXUS_OffsetToUncachedAddr(pSettings->deviceOffset);
    if ( NULL == handle->pBaseAddress )
    {
        BDBG_ERR(("Invalid device offset specified"));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto err_address;
    }
#endif

    if(pSettings->deviceOffsetIO == 0)
    {
        handle->pIOBaseAddress  = handle->pBaseAddress;
    }
    else
    {
        handle->pIOBaseAddress = NEXUS_OffsetToUncachedAddr(pSettings->deviceOffsetIO);
        if ( NULL == handle->pIOBaseAddress )
        {
            BDBG_ERR(("Invalid device IO offset specified"));
            errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            goto err_address;
        }
    }

    errCode = BKNI_CreateEvent(&handle->cdEvent);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_cd;
    }
    handle->cdEventCallback = NEXUS_RegisterEvent(handle->cdEvent, NEXUS_DvbCi_P_CheckCardStatus, handle);
    if ( NULL == handle->cdEventCallback )
    {
        errCode = BERR_TRACE(BERR_OS_ERROR);
        goto err_register;
    }
    errCode = BKNI_CreateEvent(&handle->readyEvent);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_ready;
    }
    handle->readyEventCallback = NEXUS_RegisterEvent(handle->readyEvent, NEXUS_DvbCi_P_ReadyCallback, handle);
    if ( NULL == handle->readyEventCallback )
    {
        errCode = BERR_TRACE(BERR_OS_ERROR);
        goto err_register_ready;
    }

    if ( NULL != pSettings )
    {
        handle->openSettings = *pSettings;
    }

    errCode = NEXUS_DvbCi_P_InitializeSlot(handle);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_slot;
    }

    errCode = NEXUS_DvbCi_P_InitChip(handle);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_init;
    }

    /* Success.  Check now for an inserted card, before the user can register callbacks. */
    NEXUS_DvbCi_P_CheckCardStatus(handle);

    return handle;

err_init:
err_slot:
    NEXUS_UnregisterEvent(handle->readyEventCallback);
err_register_ready:
    BKNI_DestroyEvent(handle->readyEvent);
err_ready:
    NEXUS_UnregisterEvent(handle->cdEventCallback);
err_register:
    BKNI_DestroyEvent(handle->cdEvent);
err_cd:
err_address:
    NEXUS_TaskCallback_Destroy(handle->errorCallback);
    NEXUS_TaskCallback_Destroy(handle->cardDetectCallback);
    NEXUS_TaskCallback_Destroy(handle->ireqCallback);
    BKNI_Memset(handle, 0, sizeof(NEXUS_DvbCi));
    BKNI_Free(handle);
    return NULL;
}

/***************************************************************************
Summary:
    Close a DVB-CI device
***************************************************************************/
static void NEXUS_DvbCi_P_Finalizer(
    NEXUS_DvbCiHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_DvbCi);

    /* Cancel polling timer if enabled */
    if ( handle->pollTimer )
    {
        NEXUS_CancelTimer(handle->pollTimer);
        handle->pollTimer = NULL;
    }
    /* Disable possible ready timer and ready interrupt */
    if ( handle->readyTimer )
    {
        NEXUS_CancelTimer(handle->readyTimer);
        handle->readyTimer = NULL;
    }
    NEXUS_DvbCi_P_EnableReadyInterrupt(handle, false, NULL);
    NEXUS_DvbCi_P_EnableCardDetectInterrupt(handle, false, false, NULL);
    /* Return to PCMCIA state */
    NEXUS_DvbCi_P_SetupInterface(handle, true);
    /* Disable Slot Interface */
    NEXUS_DvbCi_P_EnableInterface(handle, false);
    /* Free up chip specifics */
    NEXUS_DvbCi_P_UninitChip(handle);
    /* Free handle */
    NEXUS_UnregisterEvent(handle->readyEventCallback);
    BKNI_DestroyEvent(handle->readyEvent);
    NEXUS_UnregisterEvent(handle->cdEventCallback);
    BKNI_DestroyEvent(handle->cdEvent);
    NEXUS_TaskCallback_Destroy(handle->errorCallback);
    NEXUS_TaskCallback_Destroy(handle->cardDetectCallback);
    NEXUS_TaskCallback_Destroy(handle->ireqCallback);
    NEXUS_OBJECT_DESTROY(NEXUS_DvbCi, handle);
    BKNI_Free(handle);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_DvbCi, NEXUS_DvbCi_Close);

/***************************************************************************
Summary:
    Get the settings for a DVB-CI device
***************************************************************************/
void NEXUS_DvbCi_GetSettings(
    NEXUS_DvbCiHandle handle,
    NEXUS_DvbCiSettings *pSettings      /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_DvbCi);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}

/***************************************************************************
Summary:
    Set the settings for a DVB-CI device
***************************************************************************/
NEXUS_Error NEXUS_DvbCi_SetSettings(
    NEXUS_DvbCiHandle handle,
    const NEXUS_DvbCiSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_DvbCi);
    BDBG_ASSERT(NULL != pSettings);
    handle->settings = *pSettings;
    if ( NULL != handle->pollTimer )
    {
        NEXUS_CancelTimer(handle->pollTimer);
        handle->pollTimer = NULL;
    }
    NEXUS_TaskCallback_Set(handle->errorCallback, &pSettings->errorCallback);
    NEXUS_TaskCallback_Set(handle->cardDetectCallback, &pSettings->cardDetectCallback);
    NEXUS_TaskCallback_Set(handle->ireqCallback, &pSettings->interruptCallback);
    return NEXUS_SUCCESS;
}

/***************************************************************************
Summary:
    Get the status of a DVB-CI device
***************************************************************************/
void NEXUS_DvbCi_GetStatus(
    NEXUS_DvbCiHandle handle,
    NEXUS_DvbCiStatus *pStatus  /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_DvbCi);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof(NEXUS_DvbCiStatus));
    pStatus->cardPresent = NEXUS_DvbCi_P_CardPresent(handle);
    pStatus->validCis = pStatus->cardPresent && handle->validCis;
    pStatus->pcmciaMode = handle->pcmciaMode;
    pStatus->lastError = handle->lastError;
    pStatus->daInterruptEnabled = handle->daIE;
    pStatus->frInterruptEnabled = handle->frIE;
    if ( handle->currentState == NEXUS_DvbCiState_eNegotiated )
    {
        uint8_t status;

        status = NEXUS_DvbCi_P_ReadByte(handle, NEXUS_DVB_CI_REG_STATUS);
        pStatus->daAsserted = (status & NEXUS_DVB_CI_STATUS_DA)?true:false;
        pStatus->frAsserted = (status & NEXUS_DVB_CI_STATUS_FR)?true:false;
        pStatus->iirAsserted = (status & NEXUS_DVB_CI_STATUS_IIR)?true:false;
        pStatus->negotiatedBufferSize = handle->negotiatedBufferSize;
    }
}

/***************************************************************************
Summary:
    Reset a DVB-CI card
***************************************************************************/
NEXUS_Error NEXUS_DvbCi_Reset(
    NEXUS_DvbCiHandle handle
    )
{
    NEXUS_CallbackDesc callbackDesc;

    BDBG_OBJECT_ASSERT(handle, NEXUS_DvbCi);

    if ( !NEXUS_DvbCi_P_CardPresent(handle) )
    {
        BDBG_ERR(("Card not present"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Always disable IREQ interrupts */
    NEXUS_DvbCi_EnableInterrupts(handle, false, false);
    /* Reset back to PCMCIA mode to read CIS */
    NEXUS_DvbCi_P_SetupInterface(handle, true);
    /* Enable the interface, this will apply the reset signal */
    NEXUS_DvbCi_P_EnableInterface(handle, true);
    /* This should be held high for at least 1ms, 10 should suffice.  */
    BKNI_Sleep(10);
    /* de-assert reset */
    NEXUS_DvbCi_P_SetResetState(handle, NEXUS_DvbCiPinState_eLow);
    /* Log state change */
    handle->lastError = NEXUS_DvbCiError_eNone;
    handle->currentState = NEXUS_DvbCiState_ePcmciaReset;
    NEXUS_CallbackDesc_Init(&callbackDesc);
    callbackDesc.callback = NEXUS_DvbCi_P_ReadyInterrupt_nosync;
    callbackDesc.context = handle;
    callbackDesc.param = 0;
    NEXUS_DvbCi_P_EnableReadyInterrupt(handle, true, &callbackDesc);
    /* Setup 5 second ready timeout timer */
    handle->readyTimer = NEXUS_ScheduleTimer(5000, NEXUS_DvbCi_P_ReadyTimeout, handle);
    BDBG_ASSERT(NULL != handle->readyTimer);

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Execute the personality change into DVB mode

Description:
    This routine should be called after the card has been detected and
    the cardPresent and validCis flags of the NEXUS_DvbCiStatus
    structure are both set to true.  If this routine returns successfully,
    NEXUS_DvbCi_NegotiateBuffers should be called.

See Also:
    NEXUS_DvbCi_NegotiateBuffers
***************************************************************************/
NEXUS_Error NEXUS_DvbCi_ChangePersonality(
    NEXUS_DvbCiHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_DvbCi);

    if ( !NEXUS_DvbCi_P_CardPresent(handle) )
    {
        BDBG_ERR(("Card not present"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( handle->currentState != NEXUS_DvbCiState_eReadCis )
    {
        BDBG_ERR(("Can not execute personality change"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Write COR */
    handle->currentState = NEXUS_DvbCiState_eWroteCor;
    *((uint8_t *)(handle->pBaseAddress+handle->corAddr)) = handle->corValue;

    NEXUS_DvbCi_P_SetupInterface(handle, false);

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Negotiate the data channel buffer size

Description:
    This routine should be called after NEXUS_DvbCi_ChangePersonality.
    It will negotiate the data channel buffer size with the module.  The
    negotiated size can be read from NEXUS_DvbCi_GetStatus.  The negotiated
    size wll be the minimum of the buffer size provided to this call and the
    buffer provided by the module.

See Also:
    NEXUS_DvbCi_ChangePersonality
    NEXUS_DvbCi_GetStatus
***************************************************************************/
NEXUS_Error NEXUS_DvbCi_NegotiateBuffers(
    NEXUS_DvbCiHandle handle,
    uint16_t maxBufferSize,         /* Pass 0 for an unlimited buffer size */
    uint16_t *pNegotiatedSize
    )
{
    uint16_t moduleSize;
    uint8_t value;
    int i = 0;
    uint16_t transferSize;

    BDBG_OBJECT_ASSERT(handle, NEXUS_DvbCi);
    BDBG_ASSERT(NULL != pNegotiatedSize);

    if ( !NEXUS_DvbCi_P_CardPresent(handle) )
    {
        BDBG_ERR(("Card not present"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( 0 == maxBufferSize )
    {
        maxBufferSize = 65535;  /* Max allowed */
    }
    else if ( maxBufferSize < 256 )
    {
        BDBG_ERR(("Host must support at least 256 bytes of buffer space"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( handle->currentState < NEXUS_DvbCiState_eWroteCor )
    {
        BDBG_ERR(("Can not negotiate buffers"));
    }

    if ( NULL != handle->pollTimer )
    {
        NEXUS_CancelTimer(handle->pollTimer);
        handle->pollTimer = NULL;
    }

    /* Always disable IREQ interrupts */
    NEXUS_DvbCi_EnableInterrupts(handle, false, false);

    /* Assert RS bit */
    BDBG_MSG(("Negotiating... Write RS"));
    NEXUS_DvbCi_P_WriteCommandReg(handle, NEXUS_DVB_CI_COMMAND_RS);
    /* Wait for FR bit -- up to 5 seconds */
    BDBG_WRN(("Negotiating... Wait FR"));
    value = NEXUS_DvbCi_P_ReadByte(handle, NEXUS_DVB_CI_REG_STATUS);
    if ( 0 == (value & NEXUS_DVB_CI_STATUS_FR) )
    {
        for ( i = 0; i < 10000; i+=100 )
        {
            BKNI_Sleep(100);
            value = NEXUS_DvbCi_P_ReadByte(handle, NEXUS_DVB_CI_REG_STATUS);
            if ( value & NEXUS_DVB_CI_STATUS_FR )
            {
                break;
            }
        }
        if ( i >= 10000 )
        {
            BDBG_ERR(("Timeout waiting for FR after RS"));
            handle->lastError = NEXUS_DvbCiError_eRsTimeout;
            NEXUS_TaskCallback_Fire(handle->errorCallback);
            NEXUS_DvbCi_P_WriteCommandReg(handle, 0);
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    BDBG_WRN(("Negotiating... %d Write SR", i));
    NEXUS_DvbCi_P_WriteCommandReg(handle, NEXUS_DVB_CI_COMMAND_SR);
    BDBG_MSG(("Negotiating... Wait DA"));
    value = NEXUS_DvbCi_P_ReadByte(handle, NEXUS_DVB_CI_REG_STATUS);
    if ( 0 == (value & NEXUS_DVB_CI_STATUS_DA) )
    {
        for ( i = 0; i < 5000; i+=100 )
        {
            BKNI_Sleep(100);
            value = NEXUS_DvbCi_P_ReadByte(handle, NEXUS_DVB_CI_REG_STATUS);
            if ( value & NEXUS_DVB_CI_STATUS_DA )
            {
                break;
            }
        }
        if ( i >= 5000 )
        {
            BDBG_ERR(("Timeout waiting for DA after SR"));
            handle->lastError = NEXUS_DvbCiError_eBufferNegotiationFailed;
            NEXUS_TaskCallback_Fire(handle->errorCallback);
            NEXUS_DvbCi_P_WriteCommandReg(handle, 0);
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    /* read size -- MSB is first */
    BDBG_MSG(("Negotiating... Read Module Size"));
    value = NEXUS_DvbCi_P_ReadByte(handle, NEXUS_DVB_CI_REG_SIZE_LS);
    transferSize = value;
    value = NEXUS_DvbCi_P_ReadByte(handle, NEXUS_DVB_CI_REG_SIZE_MS);
    transferSize |= ((uint16_t)value)<<8;
    if ( transferSize > 2 || transferSize < 1 )
    {
        BDBG_ERR(("Length error"));
            handle->lastError = NEXUS_DvbCiError_eBufferNegotiationFailed;
            NEXUS_TaskCallback_Fire(handle->errorCallback);
            return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    moduleSize = NEXUS_DvbCi_P_ReadByte(handle, NEXUS_DVB_CI_REG_DATA);
    if ( transferSize == 2 )
    {
        /* Modules with a very small buffer may report only one byte of size */
        moduleSize <<= 8;
        moduleSize |= NEXUS_DvbCi_P_ReadByte(handle, NEXUS_DVB_CI_REG_DATA);
    }

    BDBG_MSG(("Negotiating... Clear SR"));
    NEXUS_DvbCi_P_WriteCommandReg(handle, 0);
    BDBG_MSG(("Negotiating... Module reports %d bytes available", moduleSize));
    if ( moduleSize < 4 )   /* Technically this should be 16 according to the spec, but 4 is possible */
    {
        BDBG_ERR(("Invalid module buffer size (%d)", moduleSize));
        handle->lastError = NEXUS_DvbCiError_eBufferNegotiationFailed;
        NEXUS_TaskCallback_Fire(handle->errorCallback);
        NEXUS_DvbCi_P_WriteCommandReg(handle, 0);
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    if ( moduleSize < maxBufferSize )
    {
        BDBG_MSG(("Module Size (%d) less than host size (%d).  Using module size", moduleSize, maxBufferSize));
    }
    else
    {
        BDBG_MSG(("Host Size (%d) less than module size (%d).  Using host size", moduleSize, maxBufferSize));
        moduleSize = maxBufferSize;
    }
    BDBG_MSG(("Negotiating... Write SW"));
    NEXUS_DvbCi_P_WriteCommandReg(handle, NEXUS_DVB_CI_COMMAND_SW);
    BDBG_MSG(("Negotiating... Wait FR"));
    value = NEXUS_DvbCi_P_ReadByte(handle, NEXUS_DVB_CI_REG_STATUS);
    if ( 0 == (value & NEXUS_DVB_CI_STATUS_FR) )
    {
        for ( i = 0; i < 5000; i+=100 )
        {
            BKNI_Sleep(100);
            value = NEXUS_DvbCi_P_ReadByte(handle, NEXUS_DVB_CI_REG_STATUS);
            if ( value & NEXUS_DVB_CI_STATUS_FR )
            {
                break;
            }
        }
        NEXUS_DvbCi_P_WriteCommandReg(handle, 0);
        if ( i >= 5000 )
        {
            BDBG_ERR(("Timeout waiting for FR after RS"));
            handle->lastError = NEXUS_DvbCiError_eBufferNegotiationFailed;
            NEXUS_TaskCallback_Fire(handle->errorCallback);
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    /* Write negotiated size, MSB first */
    NEXUS_DvbCi_P_WriteCommandReg(handle, NEXUS_DVB_CI_COMMAND_HC);
    value = NEXUS_DvbCi_P_ReadByte(handle, NEXUS_DVB_CI_REG_STATUS);
    if ( 0 == (value & NEXUS_DVB_CI_STATUS_FR) )
    {
        for ( i = 0; i < 5000; i+=50 )
        {
            NEXUS_DvbCi_P_WriteCommandReg(handle, 0);
            BKNI_Sleep(50);
            NEXUS_DvbCi_P_WriteCommandReg(handle, NEXUS_DVB_CI_COMMAND_HC);
            value = NEXUS_DvbCi_P_ReadByte(handle, NEXUS_DVB_CI_REG_STATUS);
            if ( value & NEXUS_DVB_CI_STATUS_FR )
            {
                break;
            }
        }
        if ( i >= 5000 )
        {
            BDBG_ERR(("Timeout waiting for FR after RS:%x",value));
            handle->lastError = NEXUS_DvbCiError_eRsTimeout;
            NEXUS_TaskCallback_Fire(handle->errorCallback);
            NEXUS_DvbCi_P_WriteCommandReg(handle, 0);
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }

    BDBG_MSG(("Negotiating... Write Size"));
    NEXUS_DvbCi_P_WriteByte(handle, NEXUS_DVB_CI_REG_SIZE_LS,2 );
    NEXUS_DvbCi_P_WriteByte(handle, NEXUS_DVB_CI_REG_SIZE_MS, 0);
    NEXUS_DvbCi_P_WriteByte(handle, NEXUS_DVB_CI_REG_DATA, moduleSize >> 8);
    NEXUS_DvbCi_P_WriteByte(handle, NEXUS_DVB_CI_REG_DATA, moduleSize & 0xff);
    BDBG_MSG(("Negotiating... Clear SW"));
    NEXUS_DvbCi_P_WriteCommandReg(handle, 0);
    value = NEXUS_DvbCi_P_ReadByte(handle, NEXUS_DVB_CI_REG_STATUS);
    /* Done */
    handle->currentState = NEXUS_DvbCiState_eNegotiated;
    handle->negotiatedBufferSize = moduleSize;
    *pNegotiatedSize = moduleSize;
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Write a buffer of data to a DVB-CI module

Description:
    This routine will write a buffer of data to a DVB-CI module.  If the module
    has asserted the DA bit indicating data is available, this will return an
    error and the application should read data instead.  If the FR bit is not set,
    this routine will also return an error and the host should retry after the
    NEXUS_DvbCiStatus.frAsserted is set to true.  The buffer provided should be
    <= the negotiated buffer size.

See Also:
    NEXUS_DvbCi_GetStatus
    NEXUS_DvbCi_Read
***************************************************************************/
NEXUS_Error NEXUS_DvbCi_Write(
    NEXUS_DvbCiHandle handle,
    const void *pBuffer,        /* Pointer to data */
    uint16_t bufferSize,        /* size of buffer in bytes - this should be >= the negotiated size */
    uint16_t *pSizeWritten      /* [out] number of bytes written */
    )
{
    uint8_t value;
    int i;

    BDBG_OBJECT_ASSERT(handle, NEXUS_DvbCi);
    BDBG_ASSERT(NULL != pBuffer);
    BDBG_ASSERT(bufferSize > 0);
    BDBG_ASSERT(NULL != pSizeWritten);

    if ( !NEXUS_DvbCi_P_CardPresent(handle) )
    {
        BDBG_ERR(("Card not present"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( bufferSize > handle->negotiatedBufferSize )
    {
        BDBG_ERR(("Buffer size larger than negotiated size"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( handle->currentState != NEXUS_DvbCiState_eNegotiated )
    {
        BDBG_ERR(("Can not write to interface"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    value = NEXUS_DvbCi_P_ReadByte(handle, NEXUS_DVB_CI_REG_STATUS);
    if ( value & NEXUS_DVB_CI_STATUS_DA )
    {
        /* Module wants to send.  Retry Later */
        BDBG_ERR(("Module wants to send data.  Can not write to interface"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BDBG_MSG(("Write - Assert HC"));
    NEXUS_DvbCi_P_WriteCommandReg(handle, NEXUS_DVB_CI_COMMAND_HC);
    value = NEXUS_DvbCi_P_ReadByte(handle, NEXUS_DVB_CI_REG_STATUS);
    if ( 0 == (value & NEXUS_DVB_CI_STATUS_FR) )
    {
        for ( i = 0; i < 5000; i+=100 )
        {
	   NEXUS_DvbCi_P_WriteCommandReg(handle, 0);
            BKNI_Sleep(100);
            NEXUS_DvbCi_P_WriteCommandReg(handle, NEXUS_DVB_CI_COMMAND_HC);
            value = NEXUS_DvbCi_P_ReadByte(handle, NEXUS_DVB_CI_REG_STATUS);
            if ( value & NEXUS_DVB_CI_STATUS_FR )
            {
                break;
            }
        }
        if ( i >= 5000 )
        {
            BDBG_ERR(("Timeout waiting for FR after RS:%x",value));
            NEXUS_DvbCi_P_WriteCommandReg(handle, 0);
            handle->lastError = NEXUS_DvbCiError_eRsTimeout;
            NEXUS_TaskCallback_Fire(handle->errorCallback);
            return BERR_TRACE(BERR_TIMEOUT);
        }
    }

    BDBG_MSG(("Write - Set Size"));
    NEXUS_DvbCi_P_WriteByte(handle, NEXUS_DVB_CI_REG_SIZE_LS, bufferSize & 0xff);
    NEXUS_DvbCi_P_WriteByte(handle, NEXUS_DVB_CI_REG_SIZE_MS, bufferSize >> 8);
    BDBG_MSG(("Write - Send Data"));
    for ( i = 0; i < (int)(bufferSize-1); i++ )
    {
        NEXUS_DvbCi_P_WriteByte(handle, NEXUS_DVB_CI_REG_DATA, ((uint8_t *)pBuffer)[i]);
        value = NEXUS_DvbCi_P_ReadByte(handle, NEXUS_DVB_CI_REG_STATUS);
        if ( 0 == (value & NEXUS_DVB_CI_STATUS_WE) )
        {
            /* WE should remain at 1 until the last byte is written.  Anything else is fatal. */
            BDBG_ERR(("Write Error -- interface must be reset"));
            handle->lastError = NEXUS_DvbCiError_eWriteError;
            handle->currentState = NEXUS_DvbCiState_eTransferError;
            NEXUS_TaskCallback_Fire(handle->errorCallback);
            NEXUS_DvbCi_P_WriteCommandReg(handle, 0);
            *pSizeWritten = i;
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    /* Write last byte */
    NEXUS_DvbCi_P_WriteByte(handle, NEXUS_DVB_CI_REG_DATA, ((uint8_t *)pBuffer)[bufferSize-1]);
    value = NEXUS_DvbCi_P_ReadByte(handle, NEXUS_DVB_CI_REG_STATUS);
    if ( value & NEXUS_DVB_CI_STATUS_WE )
    {
        /* WE should clear after the last byte.  Anything else is fatal. */
        BDBG_ERR(("Write Error -- interface must be reset"));
        handle->lastError = NEXUS_DvbCiError_eWriteError;
        handle->currentState = NEXUS_DvbCiState_eTransferError;
        NEXUS_TaskCallback_Fire(handle->errorCallback);
        NEXUS_DvbCi_P_WriteCommandReg(handle, 0);
        *pSizeWritten = bufferSize;
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BDBG_MSG(("Write - Clear HC"));
    NEXUS_DvbCi_P_WriteCommandReg(handle, 0);
    *pSizeWritten = bufferSize;

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Read a buffer of data from a DVB-CI module

Description:
    This routine will read a buffer of data from a DVB-CI module.  If the module
    has not asserted the DA bit, this will return an error and the application should
    retry after the NEXUS_DvbCiStatus.daAsserted is set to true.  The buffer provided
    should be >= the negotiated buffer size.

See Also:
    NEXUS_DvbCi_GetStatus
    NEXUS_DvbCi_Read
***************************************************************************/
NEXUS_Error NEXUS_DvbCi_Read(
    NEXUS_DvbCiHandle handle,
    void *pBuffer,              /* [out] Pointer to data */
    uint16_t bufferSize,        /* size of buffer in bytes - this should be <= the negotiated size */
    uint16_t *pSizeRead         /* [out] number of bytes read */
    )
{
    uint16_t transferSize;
    uint8_t value;
    int i;

    BDBG_OBJECT_ASSERT(handle, NEXUS_DvbCi);
    BDBG_ASSERT(NULL != pBuffer);
    BDBG_ASSERT(bufferSize > 0);
    BDBG_ASSERT(NULL != pSizeRead);

    if ( !NEXUS_DvbCi_P_CardPresent(handle) )
    {
        BDBG_ERR(("Card not present"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( bufferSize < handle->negotiatedBufferSize )
    {
        BDBG_ERR(("Buffer size smaller than negotiated size"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( handle->currentState != NEXUS_DvbCiState_eNegotiated )
    {
        BDBG_ERR(("Can not read from interface"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    value = NEXUS_DvbCi_P_ReadByte(handle, NEXUS_DVB_CI_REG_STATUS);
    if ( 0 == (value & NEXUS_DVB_CI_STATUS_DA) )
    {
        /* No data available - retry later */
        BDBG_MSG(("Read - no DA"));
        return BERR_NOT_SUPPORTED;
    }

    BDBG_MSG(("Read - Get Size"));
    value = NEXUS_DvbCi_P_ReadByte(handle, NEXUS_DVB_CI_REG_SIZE_LS);
    transferSize = value;
    value = NEXUS_DvbCi_P_ReadByte(handle, NEXUS_DVB_CI_REG_SIZE_MS);
    transferSize |= ((uint16_t)value)<<8;
    if ( transferSize == 0 || transferSize > bufferSize || transferSize > handle->negotiatedBufferSize )
    {
        BDBG_ERR(("Module reports an invalid transfer size of %d", transferSize));
        handle->lastError = NEXUS_DvbCiError_eReadError;
        handle->currentState = NEXUS_DvbCiState_eTransferError;
        NEXUS_TaskCallback_Fire(handle->errorCallback);
        *pSizeRead = 0;
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    BDBG_MSG(("Read - Get Data"));
    for ( i = 0; i < (int)(transferSize-1); i++ )
    {
        ((uint8_t *)pBuffer)[i] = NEXUS_DvbCi_P_ReadByte(handle, NEXUS_DVB_CI_REG_DATA);
        value = NEXUS_DvbCi_P_ReadByte(handle, NEXUS_DVB_CI_REG_STATUS);
        if ( 0 == (value & NEXUS_DVB_CI_STATUS_RE) )
        {
            /* RE should remain at 1 until the last byte is written.  Anything else is fatal. */
            BDBG_ERR(("Read Error -- interface must be reset"));
            handle->lastError = NEXUS_DvbCiError_eReadError;
            handle->currentState = NEXUS_DvbCiState_eTransferError;
            NEXUS_TaskCallback_Fire(handle->errorCallback);
            *pSizeRead = i;
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    /* Read last byte */
    ((uint8_t *)pBuffer)[transferSize-1] = NEXUS_DvbCi_P_ReadByte(handle, NEXUS_DVB_CI_REG_DATA);
    value = NEXUS_DvbCi_P_ReadByte(handle, NEXUS_DVB_CI_REG_STATUS);
    if ( value & NEXUS_DVB_CI_STATUS_RE )
    {
        /* RE should clear after the last byte.  Anything else is fatal. */
        BDBG_ERR(("Read Error -- interface must be reset"));
        handle->lastError = NEXUS_DvbCiError_eReadError;
        handle->currentState = NEXUS_DvbCiState_eTransferError;
        NEXUS_TaskCallback_Fire(handle->errorCallback);
        *pSizeRead = transferSize;
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BDBG_MSG(("Read - Done"));

    *pSizeRead = transferSize;

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Get CIS information from the current card

Description:
    This routine will return data extracted from the CIS of the card
    currently inserted in the slot.

See Also:
    NEXUS_DvbCi_GetStatus
***************************************************************************/
NEXUS_Error NEXUS_DvbCi_GetCisData(
    NEXUS_DvbCiHandle handle,
    NEXUS_DvbCiCisData *pCisData    /* [out] */
    )
{
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_DvbCi);
    BDBG_ASSERT(NULL != pCisData);

    if ( NEXUS_DvbCi_P_CardPresent(handle) )
    {
        BKNI_Memcpy(pCisData->manufacturerName, handle->manufacturerName, NEXUS_DVB_CI_MANUFACTURER_NAME_MAX);
        BKNI_Memcpy(pCisData->productName, handle->productName, NEXUS_DVB_CI_PRODUCT_NAME_MAX);
        BKNI_Memcpy(pCisData->additionalInfo, handle->additionalInfo, NEXUS_DVB_CI_ADDITIONAL_INFO_MAX);
        pCisData->corAddr = handle->corAddr;
        pCisData->corValue = handle->corValue;
        pCisData->vpp5v = handle->vpp5v;
        pCisData->interruptsSupported = handle->interruptsSupported;
        errCode = BERR_SUCCESS;
    }
    else
    {
        BKNI_Memset(pCisData, 0, sizeof(NEXUS_DvbCiCisData));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    return errCode;
}

static NEXUS_Error NEXUS_DvbCi_P_InitializeSlot(NEXUS_DvbCiHandle handle)
{
    NEXUS_Error errCode;
    NEXUS_CallbackDesc callback;

    /* Setup EBI CS for PCMCIA mode */
    errCode = NEXUS_DvbCi_P_SetupInterface(handle, true);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    NEXUS_DvbCi_P_EnableInterface(handle, false);

    /* Now we're in a card removed state */
    handle->currentState = NEXUS_DvbCiState_eNoCard;
    handle->validCis = false;

    /* Setup card detect interrupts */
    NEXUS_CallbackDesc_Init(&callback);
    callback.callback=NEXUS_DvbCi_P_CdInterrupt_nosync;
    callback.context = handle;
    NEXUS_DvbCi_P_EnableCardDetectInterrupt(handle, true, true, &callback);

    return BERR_SUCCESS;
}

static void NEXUS_DvbCi_P_EnableInterface(NEXUS_DvbCiHandle handle, bool enabled)
{
    /* Setup GPIO's to initial settings as if module were removed */
    NEXUS_DvbCi_P_EnableReadyInterrupt(handle, false, NULL);

    if ( enabled )
    {
        NEXUS_Error errCode;
        NEXUS_DvbCiPinState vs1State, vs2State;

        errCode = NEXUS_DvbCi_P_GetVoltageSense(handle, &vs1State, &vs2State);
        if ( errCode )
        {
            /* Default to 5V for 5V only keyed slots */
            BDBG_WRN(("*** VS pins not provided.  Defaulting to 5V for CIS read."));
            NEXUS_DvbCi_P_SetVcc(handle, NEXUS_DvbCiVcc_e5v);
            NEXUS_DvbCi_P_SetVpp(handle, NEXUS_DvbCiVpp_e5v);
        }
        else
        {
            /* Checking only VS1 seems appropriate for non-cardbus cards. */
            if ( vs1State == NEXUS_DvbCiPinState_eHigh &&
                 (vs2State == NEXUS_DvbCiPinState_eHigh || vs2State == NEXUS_DvbCiPinState_eUnknown) )
            {
                BDBG_WRN(("*** VS1 pin reads high.  Using 5V for CIS read."));
                NEXUS_DvbCi_P_SetVcc(handle, NEXUS_DvbCiVcc_e5v);
                NEXUS_DvbCi_P_SetVpp(handle, NEXUS_DvbCiVpp_e5v);
            }
            else
            {
                BDBG_WRN(("*** VS1 pin does not read high.  Using 3.3V for CIS read."));
                NEXUS_DvbCi_P_SetVcc(handle, NEXUS_DvbCiVcc_e3v);
                NEXUS_DvbCi_P_SetVpp(handle, NEXUS_DvbCiVpp_e3v);
            }
        }
        /* Assert reset prior to enabling voltage */
        NEXUS_DvbCi_P_SetResetState(handle, NEXUS_DvbCiPinState_eHigh);
    }
    else
    {
        /* Tristate reset pin */
        NEXUS_DvbCi_P_SetResetState(handle, NEXUS_DvbCiPinState_eTristate);
    }

    if ( !enabled )
    {
        NEXUS_DvbCi_P_SetVcc(handle, NEXUS_DvbCiVcc_eTristate);
        NEXUS_DvbCi_P_SetVpp(handle, NEXUS_DvbCiVpp_eTristate);
    }
}

static void NEXUS_DvbCi_P_ReadyInterrupt_nosync(void *pParam, int isIreq)
{
    NEXUS_DvbCiHandle handle = pParam;

    NEXUS_LockModule();
    if ( isIreq )
    {
        BDBG_MSG(("IREQ# asserted"));
        NEXUS_DvbCi_EnableInterrupts(handle, false, false);
        NEXUS_TaskCallback_Fire(handle->ireqCallback);
    }
    else
    {
        BDBG_MSG(("PCMCIA READY signal asserted"));
        BDBG_ASSERT(handle->currentState == NEXUS_DvbCiState_ePcmciaReset);
        NEXUS_DvbCi_P_EnableReadyInterrupt(handle, false, NULL);
        BKNI_SetEvent(handle->readyEvent);
    }
    NEXUS_UnlockModule();
}

static void NEXUS_DvbCi_P_ReadyTimeout(void *pParam)
{
    NEXUS_DvbCiHandle handle = pParam;

    BDBG_MSG(("PCMCIA READY timeout"));

    handle->readyTimer = NULL;
    if ( handle->currentState != NEXUS_DvbCiState_ePcmciaReset )
    {
        BDBG_WRN(("Stale PCMCIA reset timeout"));
        return;
    }

    NEXUS_DvbCi_P_EnableReadyInterrupt(handle, false, NULL);
    handle->currentState = NEXUS_DvbCiState_ePcmciaResetTimeout;
    handle->lastError = NEXUS_DvbCiError_ePcmciaResetTimeout;
    NEXUS_TaskCallback_Fire(handle->errorCallback);
}

static void NEXUS_DvbCi_P_ReadyCallback(void *pParam)
{
    NEXUS_Error errCode;
    NEXUS_DvbCiHandle handle = pParam;

    BDBG_MSG(("Checking Ready Status"));

    /* Cancel ready timer if still running */
    if ( handle->readyTimer )
    {
        NEXUS_CancelTimer(handle->readyTimer);
        handle->readyTimer = NULL;
    }

    /* Bail out if the card has been removed */
    if ( !NEXUS_DvbCi_P_CardPresent(handle) )
    {
        BDBG_ERR(("Card removed"));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        return;
    }

    /* Advance state */
    handle->currentState = NEXUS_DvbCiState_eReadCis;

    /* If we have received the ready signal, we must read the CIS to determine if the card is valid */
    BDBG_MSG(("Reading CIS"));
    errCode = NEXUS_DvbCi_P_ReadCis(handle);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        return;
    }

    /* Set VPP appropriately based on CIS data */
    NEXUS_DvbCi_P_SetVpp(handle, handle->vpp5v?NEXUS_DvbCiVpp_e5v:NEXUS_DvbCiVpp_e3v);

    if ( handle->validCis )
    {
        /* If we have a valid CIS, consider the card inserted and notify the application */
        NEXUS_TaskCallback_Fire(handle->cardDetectCallback);
    }
    else
    {
        /* An invalid CIS was detected */
        handle->currentState = NEXUS_DvbCiState_eInvalidCis;
        handle->lastError = NEXUS_DvbCiError_eInvalidCis;
        NEXUS_TaskCallback_Fire(handle->errorCallback);
    }
}

static void NEXUS_DvbCi_P_CdInterrupt_nosync(void *pParam, int unused)
{
    NEXUS_DvbCiHandle handle = pParam;

    NEXUS_LockModule();
    BSTD_UNUSED(unused);
    BDBG_MSG(("CD interrupt"));
    BKNI_SetEvent(handle->cdEvent);
    /* Flip polarity of interrupt while servicing event */
    NEXUS_UnlockModule();
}

static void NEXUS_DvbCi_P_CheckCardStatus(void *pParam)
{
    NEXUS_DvbCiHandle handle = pParam;
    NEXUS_CallbackDesc callback;
    bool inserted;

    inserted = NEXUS_DvbCi_P_CardPresent(handle);
    BDBG_MSG(("CheckCardStatus = inserted=%d", inserted));
    /* Set polarity of interrupt pins correctly */
    NEXUS_CallbackDesc_Init(&callback);
    callback.callback = NEXUS_DvbCi_P_CdInterrupt_nosync;
    callback.context = handle;
    NEXUS_DvbCi_P_EnableCardDetectInterrupt(handle, true, !inserted, &callback);

    /* Handle stray interrupts -- CD pins fire independently */
    if ( inserted && handle->currentState > NEXUS_DvbCiState_eNoCard )
    {
        BDBG_MSG(("Stray CD interrupt => inserted already"));
        return;
    }
    else if ( !inserted && handle->currentState <= NEXUS_DvbCiState_eNoCard )
    {
        BDBG_MSG(("Stray CD interrupt => removed already"));
        return;
    }

    BDBG_MSG(("Card status change -- card %s", inserted?"inserted":"removed"));

    if ( inserted )
    {
        /* Insertion -- Just notify the app */
    }
    else
    {
        /* Removal */
        /* Disable IREQ interrupts */
        NEXUS_DvbCi_P_EnableIreqInterrupt(handle, false, NULL);
        /* Cancel polling timer if enabled */
        if ( handle->pollTimer )
        {
            NEXUS_CancelTimer(handle->pollTimer);
            handle->pollTimer = NULL;
        }
        /* Disable possible ready timer and ready interrupt */
        if ( handle->readyTimer )
        {
            NEXUS_CancelTimer(handle->readyTimer);
            handle->readyTimer = NULL;
        }
        NEXUS_DvbCi_P_EnableReadyInterrupt(handle, false, NULL);
        /* Return to PCMCIA state */
        NEXUS_DvbCi_P_SetupInterface(handle, true);
        /* Disable Slot Interface */
        NEXUS_DvbCi_P_EnableInterface(handle, false);

        handle->currentState = NEXUS_DvbCiState_eNoCard;
        handle->validCis = false;
    }

    /* Fire card status change callback */
    NEXUS_TaskCallback_Fire(handle->cardDetectCallback);
}

/***************************************************************************
Summary:
    Enable/Disable DA and FR interrupts for the module

Description:
    If either interrupt fires, the callback specified in
    NEXUS_DvbCiSettings.interruptCallback will be called.  Removing or
    resetting a module (PCMCIA or via the ChangePersonality routine)
    will cause the interrupts to be disabled. They must be re-enabled by the
    application if those events occur.  Also, the interrupts are edge-triggered
    and must be re-enabled any time an interrupt has fired.
***************************************************************************/
NEXUS_Error NEXUS_DvbCi_EnableInterrupts(
    NEXUS_DvbCiHandle handle,
    bool daEnabled,
    bool frEnabled)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_DvbCi);

    handle->daIE = daEnabled;
    handle->frIE = frEnabled;

    if ( handle->daIE || handle->frIE )
    {
        NEXUS_CallbackDesc callbackDesc;

        if ( !handle->interruptsSupported )
        {
            BDBG_ERR(("This module does not support interrupts."));
            handle->daIE = handle->frIE = false;
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }

        NEXUS_CallbackDesc_Init(&callbackDesc);
        callbackDesc.callback = NEXUS_DvbCi_P_ReadyInterrupt_nosync;
        callbackDesc.context = handle;
        callbackDesc.param = true;
        NEXUS_DvbCi_P_EnableIreqInterrupt(handle, true, &callbackDesc);
        NEXUS_DvbCi_P_WriteCommandReg(handle, 0);   /* Refresh the command register with the IE updates */
    }
    else
    {
        NEXUS_DvbCi_P_EnableIreqInterrupt(handle, false, NULL);
    }
    return NEXUS_SUCCESS;
}

static void NEXUS_DvbCi_P_WriteCommandReg(NEXUS_DvbCiHandle handle, uint8_t value)
{
    if ( handle->daIE )
    {
        value |= NEXUS_DVB_CI_COMMAND_DAIE;
    }
    if ( handle->frIE )
    {
        value |= NEXUS_DVB_CI_COMMAND_FRIE;
    }

    NEXUS_DvbCi_P_WriteByte(handle, NEXUS_DVB_CI_REG_COMMAND, value);
}


/***************************************************************************
 *     (c)2007-2011 Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ******************************************************************************/


/*
 * includes
 */

#include "bchp_scpu_host_intr2.h"
#include "bchp_cpu_ipi_intr2.h"
#include "bchp_scpu_top_ctrl.h"

#include "bhsi.h"
#include "bhsi_priv.h"


/*
 * defines
 */
#define BCHP_INT_ID_SCPU_HOST_OLOAD_INTR BCHP_INT_ID_CREATE( BCHP_SCPU_HOST_INTR2_CPU_STATUS, BCHP_SCPU_HOST_INTR2_CPU_STATUS_SCPU_HOST_OLOAD_SHIFT )
#define BCHP_INT_ID_SCPU_HOST_DRDY_INTR  BCHP_INT_ID_CREATE( BCHP_SCPU_HOST_INTR2_CPU_STATUS, BCHP_SCPU_HOST_INTR2_CPU_STATUS_SCPU_HOST_DRDY_SHIFT )

/* name for BDBG_XXX() macros */
BDBG_MODULE(BHSI);


/*
 * DEBUG
 */

/*#define BDBG_MSG(x) {printf x; printf("\n");}*/
/*#define BDBG_ERR(x) {printf("BHSI ERR:\t");BDBG_MSG(x);}*/
/*#define BDBG_ENTER(FUNC) printf("\n\t>> %s ENTER\n", #FUNC)*/
/*#define BDBG_LEAVE(FUNC) printf("\t<< %s LEAVE\n\n", #FUNC)*/
/*int printf(const char *format, ...);*/

#ifdef BHSI_DEBUG_REG
#define _REG_LOG(REGHANDLE, REGID, COMMENT) _reg_log(REGHANDLE, REGID, #REGID, COMMENT)
static void _reg_log(BREG_Handle hReg, int id, const char *name, const char * comm)
{
    uint32_t regVal = BREG_Read32(hReg, id);
    BDBG_MSG(( "GETREG(%s): %s = %x", comm, name, regVal));
}
#else
#define _REG_LOG(REGHANDLE, REGID, COMMENT)
#endif


/*
 * Private functions and variables
 */

static void BHSI_P_CleanupContext(BHSI_Handle hHsi);
static void BHSI_P_OloadIntHandler_isr(void *inp_param1, int in_param2);
static void BHSI_P_DrdyIntHandler_isr(void *inp_param1, int in_param2);

/*
 * implementation
 *
 * Note about flush:
 * User registered flush callback (see BHSI_Settings)
 * fired any time right before a communication buffer
 * ownership is transfered to SAGE; i.e.:
 *  - after writing the Request buffer
 * Ensuring:
 *  - local 'cached' value is synced with hardware
 *  - there will be no wild flush at an inapropriate moment
 *    ex. when Host have the ownership
 *
 * Note about invalidate:
 * User registered invalidate callback (see BSHI_Settings)
 * fired any time right before a communication buffer
 * ownership is received from SAGE; i.e.:
 *  - before reading the Response buffer
 *  - before reading the ACK buffer
 * Ensuring:
 *  - cached memory is now invalid and will be read from hardware
 */

/* Receive Interrupt in Forward Communication Channel */
static void BHSI_P_OloadIntHandler_isr(
    void *inp_param1,        /* Device handle */
    int in_param2            /* reserved */
)
{
    BHSI_Handle hHsi = (BHSI_Handle) inp_param1 ;

    BDBG_ENTER(BHSI_P_OloadIntHandler_isr);
    BSTD_UNUSED( in_param2 );

    if (NULL == hHsi) {
        BDBG_ERR(("%s: NULL HSI Handle",
                  __FUNCTION__));
        goto end;
    }
    if (!hHsi->bIsOpen) {
        BDBG_WRN(("%s: HSI=%p closed",
                  __FUNCTION__, hHsi));
        goto end;
    }

    BKNI_SetEvent_isr(hHsi->oLoadWaitEvent);

end:
    BDBG_LEAVE(BHSI_P_OloadIntHandler_isr);
}

/* Receive Interrupt in Reverse Communication Channel */
static void BHSI_P_DrdyIntHandler_isr(
    void *inp_param1, /* Device handle */
    int in_param2     /* reserved */
)
{
    BHSI_Handle hHsi = (BHSI_Handle) inp_param1 ;

    BDBG_ENTER(BHSI_P_DrdyIntHandler_isr);
    BSTD_UNUSED( in_param2 );

    if (NULL == hHsi) {
        BDBG_ERR(("%s: NULL HSI Handle",
                  __FUNCTION__));
        goto end;
    }
    if (!hHsi->bIsOpen) {
        BDBG_WRN(("%s: HSI=%p closed",
                  __FUNCTION__, hHsi));
        goto end;
    }

    /* enable BHSI_Receive() */
    hHsi->bReceivePending = 1;

    /* Call Callback function provided by High Layer software */
    if (hHsi->settings.responseCallback_isr != NULL) {
        hHsi->settings.responseCallback_isr(hHsi->settings.responseCallbackContext);
    }

end:
    BDBG_LEAVE(BHSI_P_DrdyIntHandler_isr);
}

static void BHSI_P_DisableCallbacks_isr(BHSI_Handle hHsi)
{
    if (!hHsi->bCallbacksEnabled) {
        return;
    }

    if (hHsi->drdyIntCallback) {
        BINT_DisableCallback_isr(hHsi->drdyIntCallback);
    }
    if (hHsi->oLoadIntCallback) {
        BINT_DisableCallback_isr(hHsi->oLoadIntCallback);
    }

    hHsi->bCallbacksEnabled = false;
}

/* return default open settings */
BERR_Code BHSI_GetDefaultSettings(
    BHSI_Settings *pSettings
)
{
/* timeout is arbitrary.
 * TODO benchmark and define a proper value */
#define BHSI_DEFAULT_TIMEOUT_MS 300
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_ENTER(BHSI_GetDefaultSettings);

    BDBG_ASSERT( pSettings );

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->timeOutMsec = BHSI_DEFAULT_TIMEOUT_MS;

    BDBG_LEAVE(BHSI_GetDefaultSettings);
    return( errCode );
}

/* Destructor of HSI instance. */
static void BHSI_P_CleanupContext(BHSI_Handle hHsi)
{
    BERR_Code errCode;

    BDBG_MSG(("%s(%p)", __FUNCTION__, hHsi));

    if (NULL == hHsi) {
        return;
    }

    /* first close the instance.
     * Public API is not available anymore.
     * Interrupts handler will not fire registered callbacks. */
    hHsi->bIsOpen = false;

    BKNI_EnterCriticalSection();
    BHSI_P_DisableCallbacks_isr(hHsi);
    BKNI_LeaveCriticalSection();

    if (hHsi->oLoadIntCallback != NULL) {
        errCode = BINT_DestroyCallback(hHsi->oLoadIntCallback);
        if (errCode != BERR_SUCCESS) {
            BDBG_ERR(("%s: BINT_DestroyCallback(Oload) returns error %u",
                      __FUNCTION__, errCode));
            (void)BERR_TRACE(BERR_INVALID_PARAMETER) ;
        }
        hHsi->oLoadIntCallback = NULL;
    }
    if (hHsi->oLoadWaitEvent != NULL) {
        BKNI_DestroyEvent(hHsi->oLoadWaitEvent);
        hHsi->oLoadWaitEvent = NULL;
    }
    if (hHsi->drdyIntCallback != NULL) {
        errCode = BINT_DestroyCallback(hHsi->drdyIntCallback);
        if (errCode != BERR_SUCCESS) {
            BDBG_ERR(("%s: BINT_DestroyCallback(Drdy) returns error %u",
                      __FUNCTION__, errCode));
            (void)BERR_TRACE(BERR_INVALID_PARAMETER) ;
        }
        hHsi->drdyIntCallback = NULL;
    }

    /* finally free instance */
    BKNI_Free(hHsi);
}

/* initial condition requires that all callbacks are valid */
static BERR_Code BHSI_P_EnableCallbacks(BHSI_Handle hHsi)
{
    BERR_Code errCode = BERR_SUCCESS;

    if (hHsi->bCallbacksEnabled) {
        goto BHSI_P_DONE_LABEL;
    }

    /* Enable DRDY ISR */
    errCode = BINT_EnableCallback(hHsi->drdyIntCallback);
    if (errCode != BERR_SUCCESS) {
        BDBG_ERR(( "BINT_EnableCallback(Drdy) failed!" ));
        goto BHSI_P_DONE_LABEL;
    }
    /* Enable OLOAD ISR */
    errCode = BINT_EnableCallback(hHsi->oLoadIntCallback);
    if (errCode != BERR_SUCCESS) {
        BDBG_ERR(( "BINT_EnableCallback(Oload) failed!" ));
        goto BHSI_P_DONE_LABEL;
    }

    hHsi->bCallbacksEnabled = true;

BHSI_P_DONE_LABEL:
    return errCode;
}

BERR_Code BHSI_Reset_isr(BHSI_Handle hHsi)
{   
    BERR_Code rc = BERR_SUCCESS;
    uint32_t regVal;

    BDBG_ENTER(BHSI_Reset_isr);

    if (NULL == hHsi) {
        BDBG_ERR(("%s: NULL HSI Handle",
                  __FUNCTION__));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto BHSI_P_DONE_LABEL;
    }

    /* Reset SAGE-side availability */
    regVal = BREG_Read32(hHsi->regHandle, BCHP_SCPU_TOP_CTRL_SCPU_HOST_IRDY);
    BREG_Write32(hHsi->regHandle,
                 BCHP_SCPU_TOP_CTRL_SCPU_HOST_IRDY,
                 regVal & (~BCHP_SCPU_TOP_CTRL_SCPU_HOST_IRDY_SCPU_HOST_IRDY_MASK));

    hHsi->bReceivePending = false;
    ++hHsi->sendSeq;
    BKNI_ResetEvent(hHsi->oLoadWaitEvent);
    BKNI_SetEvent_isr(hHsi->oLoadWaitEvent);

BHSI_P_DONE_LABEL:
    BDBG_LEAVE(BHSI_Reset_isr);
    return rc;
}

/* configure and open HSI instance
 * Note: only support a single instance */
BERR_Code BHSI_Open(
    BHSI_Handle *hHsi,
    BREG_Handle hRegister,
    BCHP_Handle hChip,
    BINT_Handle hInt,
    const BHSI_Settings *pSettings
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BHSI_Handle hsiHandle;

    BDBG_ENTER(BHSI_Open);

    /* Sanity check on the handles we've been given. */
    BDBG_ASSERT( hHsi );
    BDBG_ASSERT( hChip );
    BDBG_ASSERT( hRegister );
    BDBG_ASSERT( hInt );
    BDBG_ASSERT( pSettings );

    BDBG_ASSERT( pSettings->timeOutMsec );
    BDBG_ASSERT( pSettings->ackBuf );
    BDBG_ASSERT( pSettings->ackBufLen );
    BDBG_ASSERT( pSettings->requestBuf );
    BDBG_ASSERT( pSettings->requestBufLen );
    BDBG_ASSERT( pSettings->responseBuf );
    BDBG_ASSERT( pSettings->responseBufLen );
    BDBG_ASSERT( pSettings->responseCallback_isr );

    /* Alloc memory from the system heap */
    hsiHandle = (BHSI_Handle) BKNI_Malloc(sizeof(BHSI_P_Handle));
    if( hsiHandle == NULL )
    {
        BDBG_ERR(( "BKNI_Malloc(%d) failed!", sizeof(BHSI_P_Handle)));
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto BHSI_P_DONE_LABEL;
    }

    /* Configure instance */
    BKNI_Memset(hsiHandle, 0, sizeof( BHSI_P_Handle ));
    hsiHandle->chipHandle = hChip;
    hsiHandle->regHandle = hRegister;
    hsiHandle->interruptHandle = hInt;
    hsiHandle->settings = *pSettings;

    BDBG_MSG(("%s: timeout %d ms", __FUNCTION__, hsiHandle->settings.timeOutMsec));

    /* save */
    *hHsi = hsiHandle;

    /* Create interrupt service routine (ISR) for DRDY */
    errCode = BINT_CreateCallback(&(hsiHandle->drdyIntCallback),
                                  hsiHandle->interruptHandle ,
                                  BCHP_INT_ID_SCPU_HOST_DRDY_INTR,
                                  BHSI_P_DrdyIntHandler_isr,
                                  (void *) hsiHandle,
                                  0x00 );
    if (errCode != BERR_SUCCESS) {
        BDBG_ERR(( "BINT_CreateCallback(Drdy) failed!" ));
        goto BHSI_P_DONE_LABEL;
    }

    /* Create event for OLOAD. Used to wait for OLOAD interrupt. */
    errCode = BKNI_CreateEvent(&(hsiHandle->oLoadWaitEvent));
    if (errCode != BERR_SUCCESS) {
        BDBG_ERR(( "BKNI_CreateEvent failed!" ));
        goto BHSI_P_DONE_LABEL;
    }
    /* Create interrupt service routine (ISR) for OLOAD */
    errCode = BINT_CreateCallback(&(hsiHandle->oLoadIntCallback),
                                  hsiHandle->interruptHandle ,
                                  BCHP_INT_ID_SCPU_HOST_OLOAD_INTR,
                                  BHSI_P_OloadIntHandler_isr,
                                  (void *) hsiHandle,
                                  0x00);
    if (errCode != BERR_SUCCESS) {
        BDBG_ERR(( "BINT_CreateCallback(Oload) failed!" ));
        goto BHSI_P_DONE_LABEL;
    }

    /* All resources are now available, open API to public and enable callbacks */
    hsiHandle->bIsOpen = true;
    errCode = BHSI_P_EnableCallbacks(hsiHandle);
    if (errCode != BERR_SUCCESS) {
        goto BHSI_P_DONE_LABEL;
    }

    BDBG_MSG(("Callbacks are enabled."));

BHSI_P_DONE_LABEL:

    if (errCode != BERR_SUCCESS) {
        /* error: call destructor */
        *hHsi = NULL;
        BHSI_P_CleanupContext(hsiHandle);
    }

    BDBG_LEAVE(BHSI_Open);
    return( errCode );
}

/* close an opened instance
 * public API to call destructor */
BERR_Code BHSI_Close(
    BHSI_Handle hHsi
    )
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER(BHSI_Close);

    if (NULL == hHsi) {
        BDBG_ERR(("%s: NULL HSI Handle",
                  __FUNCTION__));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto BHSI_P_DONE_LABEL;
    }
    if(!hHsi->bIsOpen) {
        BDBG_WRN(("%s: HSI=%p is closed",
                  __FUNCTION__, (void*)hHsi));
        /* keep destroying instance */
    }
    BHSI_P_CleanupContext(hHsi);

BHSI_P_DONE_LABEL:
    BDBG_LEAVE(BHSI_Close);
    return( rc );
}

/* Send a request to SAGE */
BERR_Code BHSI_Send(
    BHSI_Handle hHsi,
    const uint8_t*sendBuf,
    uint32_t sendBufLen,
    uint8_t *ackBuf,
    uint32_t maxAckBufLen,
    uint32_t *pAckBufLen
    )
{
    BERR_Code errCode = BERR_SUCCESS;
    uint32_t regVal;
    uint32_t i;
    uint32_t totalLen = sendBufLen + BHSI_HEAD_SIZE;
    uint32_t timeoutMsec;
    BHSI_Msg *_msg;
    uint32_t rcvLen;
    uint32_t savedSendSeq;

    BDBG_ENTER(BHSI_Send);

    BDBG_ASSERT( sendBuf );
    BDBG_ASSERT( sendBufLen );
    BDBG_ASSERT( ackBuf );
    BDBG_ASSERT( maxAckBufLen );
    BDBG_ASSERT( pAckBufLen );

    if (NULL == hHsi) {
        BDBG_ERR(("%s: NULL HSI Handle",
                  __FUNCTION__));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto BHSI_P_DONE_LABEL;
    }

    BKNI_EnterCriticalSection();
    savedSendSeq = ++hHsi->sendSeq;
    BKNI_LeaveCriticalSection();

    if(!hHsi->bIsOpen) {
        BDBG_WRN(("%s: HSI=%p is closed",
                  __FUNCTION__, (void*)hHsi));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto BHSI_P_DONE_LABEL;
    }

    if (totalLen > hHsi->settings.requestBufLen) {
        BDBG_ERR(("%s: The length of Input Data + header (%u) > (%u) request Buffer Size",
                  __FUNCTION__, totalLen, hHsi->settings.requestBufLen));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER) ;
        goto BHSI_P_DONE_LABEL;
    }

    timeoutMsec = hHsi->settings.timeOutMsec;

    /* Check IReady
     * timeoutMsec <= 120 000, multiplied by 500 is till less than FFFFFFFF, no overflow yet */
    for (i = 0; i < timeoutMsec * 500; i++) {
        regVal = BREG_Read32(hHsi->regHandle, BCHP_SCPU_TOP_CTRL_SCPU_HOST_IRDY);
        if (regVal & BCHP_SCPU_TOP_CTRL_SCPU_HOST_IRDY_SCPU_HOST_IRDY_MASK) {
            break;
        }

        /* 2us per loop still blocking so far.
         * May give CPU chance to others if it's non-blocking on other platforms */
        BKNI_Delay(2);

        if (hHsi->sendSeq != savedSendSeq) {
            /* probably Reset happens. */
            BDBG_WRN(("%s: HSI=%p send sequence is not valid (reset?)",
                      __FUNCTION__, (void*)hHsi));
            errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            goto BHSI_P_DONE_LABEL;
        }
    }

    if (i == timeoutMsec * 500) {
        BDBG_ERR(("%s: Timout for waiting for SCPU_HOST_IRDY = 1",
                  __FUNCTION__));
        errCode = BERR_TRACE(BERR_TIMEOUT);
        goto BHSI_P_DONE_LABEL;
    }
    timeoutMsec -= (i / 500);

    /*
     * Using a BHSI_Msg internal structure to read/write communication
     * -> easy development
     * -> settings _msg to NULL avoid accessing the bufer afterwards
     *    very important to ensure Flush mecanism is working properly
     */
    /* write to request buffer: [size][ ... payload of <size> bytes ... ] */
    _msg = (BHSI_Msg *)hHsi->settings.requestBuf;
    _msg->size = totalLen;
    BKNI_Memcpy(&_msg->payload, sendBuf, sendBufLen);
    _msg = NULL; /* for flush: invalidate _msg to force segmentation fault if used. */

    /* Call Callback function provided by High Layer software */
    if (hHsi->settings.flushCallback_isrsafe != NULL) {
        hHsi->settings.flushCallback_isrsafe((const void *)hHsi->settings.requestBuf,
                                             (size_t)hHsi->settings.requestBufLen);
    }
    /* NOTE: do not access .requestBuf after the flush!
     * see 'Note about flush' */

    /* Sometime event is not cleared properly and BHSI could return too early making SAGE confused. */
    BKNI_ResetEvent(hHsi->oLoadWaitEvent);

    BHSI_DUMP(sendBuf, sendBufLen,
              "MIPS -----------------------------------> Sage(Request)");

    /* Set ILOAD to signal Sage there's a request ready. */
    _REG_LOG(hHsi->regHandle, BCHP_CPU_IPI_INTR2_CPU_STATUS, "preiload");
    BREG_Write32(hHsi->regHandle,
                 BCHP_CPU_IPI_INTR2_CPU_SET ,
                 BCHP_CPU_IPI_INTR2_CPU_SET_HOST_SCPU_ILOAD_MASK);
    _REG_LOG(hHsi->regHandle, BCHP_CPU_IPI_INTR2_CPU_STATUS, "postiload");

    BDBG_MSG(("****************************Set ILOAD!"));

    /* wait for oload interrupt */
    _REG_LOG(hHsi->regHandle, BCHP_SCPU_HOST_INTR2_CPU_STATUS, "prewait");
    errCode = BKNI_WaitForEvent(hHsi->oLoadWaitEvent, timeoutMsec);
    _REG_LOG(hHsi->regHandle, BCHP_SCPU_HOST_INTR2_CPU_STATUS, "postwait");
    if ( errCode != BERR_SUCCESS ) {
        /* TODO handle error. when will the buffer become available again? */
        BDBG_ERR(("%s:  Error Code= %x returned from BKNI_WaitForEvent",
                  __FUNCTION__, errCode));
        goto BHSI_P_DONE_LABEL;
    }
    if (hHsi->sendSeq != savedSendSeq) {
        /* send sequence is not valid */
        BKNI_ResetEvent(hHsi->oLoadWaitEvent);
        BDBG_WRN(("%s: HSI=%p send sequence differ (reset)",
                  __FUNCTION__, (void*)hHsi));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto BHSI_P_DONE_LABEL;
    }

    /* Call Callback function provided by High Layer software */
    if (hHsi->settings.invalidateCallback_isrsafe != NULL) {
        hHsi->settings.invalidateCallback_isrsafe((const void *)hHsi->settings.ackBuf,
                                                  (size_t)hHsi->settings.ackBufLen);
    }
    /* NOTE: do not access .ackBuf after this call!
     * see 'Note about invalidate' */

    /* Read output buffer [size][... payload ...] */
    _msg = (BHSI_Msg *)hHsi->settings.ackBuf;
    if (_msg->size < BHSI_HEAD_SIZE ||
        _msg->size > hHsi->settings.ackBufLen) {
        BDBG_ERR(("%s: Received Message is %u bytes length.\n"
                  "\tACK buffer: %u <= authorized size <= %u",
                   __FUNCTION__, _msg->size,
                   BHSI_HEAD_SIZE, hHsi->settings.ackBufLen));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto BHSI_P_DONE_LABEL;
    }

    rcvLen = _msg->size - BHSI_HEAD_SIZE;

    if (rcvLen > maxAckBufLen) {
        BDBG_ERR(("%s: The length of ackowledge message ( %d ) < allocated buffer legnth",
                  __FUNCTION__, maxAckBufLen));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto BHSI_P_DONE_LABEL;
    }

    *pAckBufLen = rcvLen;
    BKNI_Memcpy(ackBuf, &_msg->payload, *pAckBufLen);
    _msg = NULL; /* for flush: invalidate _msg to force segmentation fault if used. */

    BHSI_DUMP(ackBuf, *pAckBufLen,
              "MIPS <----------------------------------- Sage(ACK)");

BHSI_P_DONE_LABEL:

    BDBG_LEAVE(BHSI_Send);
    return( errCode );
}

/* Receive response */
BERR_Code BHSI_Receive_isrsafe(
    BHSI_Handle hHsi,
    uint8_t *rcvBuf,
    uint32_t maxRcvBufLen,
    uint32_t *pRcvBufLen
    )
{
    BERR_Code errCode = BERR_SUCCESS;
    BHSI_Msg *_msg;
    uint32_t rcvLen;

    BDBG_ENTER(BHSI_Receive_isrsafe);
    BDBG_ASSERT( rcvBuf );
    BDBG_ASSERT( maxRcvBufLen );
    BDBG_ASSERT( pRcvBufLen );

    if (NULL == hHsi) {
        BDBG_ERR(("%s: NULL HSI Handle",
                  __FUNCTION__));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto BHSI_P_DONE_LABEL;
    }
    if(!hHsi->bIsOpen) {
        BDBG_WRN(("%s: HSI=%p is closed",
                  __FUNCTION__, (void*)hHsi));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto BHSI_P_DONE_LABEL;
    }

    if (!hHsi->bReceivePending) {
        BDBG_ERR(("%s: BHSI_Receive can only be used once after responseCallback is fired.",
                  __FUNCTION__));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto BHSI_P_DONE_LABEL;
    }
    /* disable BHSI_Receive() */
    hHsi->bReceivePending = 0;

    /* Call Callback function provided by High Layer software */
    if (hHsi->settings.invalidateCallback_isrsafe != NULL) {
        hHsi->settings.invalidateCallback_isrsafe((const void *)hHsi->settings.responseBuf,
                                                  (size_t)hHsi->settings.responseBufLen);
    }
    /* NOTE: do not access .responseBuf after this call!
     * see 'Note about invalidate' */

    /* Read output buffer : [size][... payload ...] */
    _msg = (BHSI_Msg *)hHsi->settings.responseBuf;
    if (_msg->size < BHSI_HEAD_SIZE ||
        _msg->size > hHsi->settings.responseBufLen) {
        BDBG_ERR(("%s: Received Message is %u bytes length.\n"
                  "Response buffer: %u <= authorized size <= %u",
                  __FUNCTION__, _msg->size, BHSI_HEAD_SIZE, hHsi->settings.responseBufLen));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto BHSI_P_DONE_LABEL;
    }
    rcvLen = _msg->size - BHSI_HEAD_SIZE;

    if (rcvLen > maxRcvBufLen) {
        BDBG_ERR(("%s: The length of Receiving Message ( %d ) > Allocated Buffer Size (%d)",
                  __FUNCTION__, rcvLen, maxRcvBufLen));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto BHSI_P_DONE_LABEL;
    }

    *pRcvBufLen = rcvLen;
    BKNI_Memcpy(rcvBuf, &_msg->payload, *pRcvBufLen);
    _msg = NULL; /* for flush: invalidate _msg to force segmentation fault if used. */

    BHSI_DUMP(rcvBuf, *pRcvBufLen,
              "MIPS <----------------------------------- Sage(Response)");

BHSI_P_DONE_LABEL:
    /* Set DDone bit */
    /* TODO update CPU STATUS instead of replacing it */
    /* ? BREG_Read32(hHsi->regHandle, BCHP_CPU_IPI_INTR2_CPU_STATUS); */
    if (hHsi != NULL){
        BREG_Write32(hHsi->regHandle,
                 BCHP_CPU_IPI_INTR2_CPU_SET,
                 BCHP_CPU_IPI_INTR2_CPU_SET_HOST_SCPU_DDONE_MASK);
    }
    BDBG_LEAVE(BHSI_Receive_isrsafe);
    return( errCode );
}

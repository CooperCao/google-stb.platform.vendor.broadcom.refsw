/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/

#include "nexus_transport_module.h"

BDBG_MODULE(nexus_message);

#if NEXUS_NUM_MESSAGE_FILTERS

#include "bxpt_interrupt.h"

#define BDBG_MSG_TRACE(X) /* BDBG_MSG(X) */

static NEXUS_Error NEXUS_Message_P_ReadComplete(NEXUS_MessageHandle msg, size_t amountConsumed);

void NEXUS_Message_GetDefaultSettings(NEXUS_MessageSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->bufferSize = 4 * 1024;
    pSettings->maxContiguousMessageSize = 4 * 1024;
    NEXUS_CallbackDesc_Init(&pSettings->dataReady);
    NEXUS_CallbackDesc_Init(&pSettings->overflow);
    NEXUS_CallbackDesc_Init(&pSettings->psiLengthError);
    NEXUS_CallbackDesc_Init(&pSettings->crcError);
    NEXUS_CallbackDesc_Init(&pSettings->pesLengthError);
    NEXUS_CallbackDesc_Init(&pSettings->pesStartCodeError);
}

static void NEXUS_message_p_dataready_callback_isr(void *data, int unused)
{
    NEXUS_MessageHandle msg = (NEXUS_MessageHandle)data;

    BSTD_UNUSED(unused);
    BDBG_OBJECT_ASSERT(msg, NEXUS_Message);

    /* disable until the next GetBuffer call */
    BXPT_Interrupt_DisableMessageInt_isr(pTransport->xpt, msg->MesgBufferNum);

    BDBG_MSG(("dataready %p: %d", (void *)msg, msg->MesgBufferNum));
    NEXUS_IsrCallback_Fire_isr(msg->dataReady);
}

static void NEXUS_message_p_overflow_callback_isr(void *data, int unused)
{
    NEXUS_MessageHandle msg = (NEXUS_MessageHandle)data;

    BSTD_UNUSED(unused);
    BDBG_OBJECT_ASSERT(msg, NEXUS_Message);

    /* disable until the next GetBuffer call */
    BXPT_Interrupt_DisableMessageOverflowInt_isr(pTransport->xpt, msg->MesgBufferNum);

    NEXUS_IsrCallback_Fire_isr(msg->overflow);
}

static void NEXUS_Message_P_EnableInterrupt(NEXUS_MessageHandle msg, bool enabled)
{
    BERR_Code rc;
    BDBG_OBJECT_ASSERT(msg, NEXUS_Message);
    if (enabled) {
        rc = BXPT_Interrupt_EnableMessageInt(pTransport->xpt, msg->MesgBufferNum, NEXUS_message_p_dataready_callback_isr, msg, 0);
        if (rc) rc = BERR_TRACE(rc); /* fall through */
        rc = BXPT_Interrupt_EnableMessageOverflowInt(pTransport->xpt, msg->MesgBufferNum, NEXUS_message_p_overflow_callback_isr, msg, 0);
        if (rc) rc = BERR_TRACE(rc); /* fall through */
    }
    else {
        rc = BXPT_Interrupt_DisableMessageInt(pTransport->xpt, msg->MesgBufferNum);
        if (rc) rc = BERR_TRACE(rc); /* fall through */
        rc = BXPT_Interrupt_DisableMessageOverflowInt(pTransport->xpt, msg->MesgBufferNum);
        if (rc) rc = BERR_TRACE(rc); /* fall through */
    }
}

#if NEXUS_MESSAGE_USE_CHECK_TIMER
static void NEXUS_Message_P_CheckTimer(void *context)
{
    NEXUS_MessageHandle msg = context;
    msg->checkTimer = NULL;

    if (msg->noReadCompleteCount) {
        if (++msg->noReadCompleteCount >= 10) {
            BDBG_WRN(("You must call NEXUS_Message_ReadComplete to re-enable the dataReady callback."));
            msg->noReadCompleteCount = 1; /* allow it to repeat */
        }
    }

    msg->checkTimer = NEXUS_ScheduleTimer(1000, NEXUS_Message_P_CheckTimer, msg);
}
#endif

NEXUS_MessageHandle NEXUS_Message_Open(const NEXUS_MessageSettings *pSettings)
{
    BERR_Code rc;
    NEXUS_MessageSettings settings;
    NEXUS_MessageHandle msg;
    NEXUS_Addr dummy;
    unsigned i;
    NEXUS_HeapHandle heap;
    unsigned hwBufferSize;

    if (!pSettings) {
        NEXUS_Message_GetDefaultSettings(&settings);
        pSettings = &settings;
    }

    msg = BKNI_Malloc(sizeof(*msg));
    if(!msg) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);return NULL; }

    NEXUS_OBJECT_INIT(NEXUS_Message, msg);
    msg->settings = *pSettings;

    heap = NEXUS_P_DefaultHeap(NULL, NEXUS_DefaultHeapType_eFull);

#define MAX_MSG_SIZE (512*1024)
    if (pSettings->bufferSize > MAX_MSG_SIZE) {
        void *temp;
        /* we only support SW copy with GetBufferWithWrap, which does not support maxContiguousMessageSize anyway. */
        if (pSettings->maxContiguousMessageSize) {
            BERR_TRACE(NEXUS_INVALID_PARAMETER);
            goto error;
        }
        /* allocate copy buffer and start copy timer */
        msg->copy.size = pSettings->bufferSize;
        rc = NEXUS_Memory_Allocate(msg->copy.size, NULL, &temp);
        if (rc) {
            BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
            goto error;
        }
        msg->copy.buffer = temp;
        hwBufferSize = MAX_MSG_SIZE;
    }
    else {
        hwBufferSize = pSettings->bufferSize;
    }

    if (hwBufferSize) {
        msg->mmaAllocatedBlock = NEXUS_MemoryBlock_Allocate(heap, hwBufferSize, 1024, NULL);
        if (!msg->mmaAllocatedBlock) {
            rc = BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
            goto error;
        }
        rc = NEXUS_MemoryBlock_Lock(msg->mmaAllocatedBlock, &msg->allocatedBuffer);
        if (rc) {
            rc = BERR_TRACE(rc);
            goto error;
        }
        rc = NEXUS_MemoryBlock_LockOffset(msg->mmaAllocatedBlock, &dummy); /* caller is responsible for locking offset */
        if (rc) {
            rc = BERR_TRACE(rc);
            goto error;
        }
        msg->mmaBlock = msg->mmaAllocatedBlock;
        msg->buffer = msg->allocatedBuffer;
    }

    if (pSettings->maxContiguousMessageSize) {
        msg->wrappedMessage.mmaBlock = NEXUS_MemoryBlock_Allocate(heap, pSettings->maxContiguousMessageSize, 0, NULL);
        if (!msg->wrappedMessage.mmaBlock) {
            rc = BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
            goto error;
        }
        rc = NEXUS_MemoryBlock_Lock(msg->wrappedMessage.mmaBlock, &msg->wrappedMessage.buffer);
        if (rc) {
            rc = BERR_TRACE(rc);
            goto error;
        }
        rc = NEXUS_MemoryBlock_LockOffset(msg->wrappedMessage.mmaBlock, &dummy); /* caller is responsible for locking offset */
        if (rc) {
            rc = BERR_TRACE(rc);
            goto error;
        }
    }

    /* must keep track of Message handles in order to route interrupts back out to Message handles */
    for (i=0;i<NEXUS_TRANSPORT_MAX_MESSAGE_HANDLES;i++) {
        if (!pTransport->message.handle[i]) {
            break;
        }
    }
    if (i == NEXUS_TRANSPORT_MAX_MESSAGE_HANDLES) {
        BDBG_ERR(("You must increase NEXUS_TRANSPORT_MAX_MESSAGE_HANDLES"));
        goto error;
    }
    /* Use the index in the pTransport->message.handle[] array as the physical MesgBufferNum. */
    msg->MesgBufferNum = i;

    msg->dataReady = NEXUS_IsrCallback_Create(msg, NULL);
    if(!msg->dataReady) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto error;}

    msg->overflow = NEXUS_IsrCallback_Create(msg, NULL);
    if(!msg->overflow) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto error;}

    msg->psiLengthError = NEXUS_IsrCallback_Create(msg, NULL);
    if(!msg->psiLengthError) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto error;}

    msg->crcError = NEXUS_IsrCallback_Create(msg, NULL);
    if(!msg->crcError) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto error;}

    msg->pesLengthError = NEXUS_IsrCallback_Create(msg, NULL);
    if(!msg->pesLengthError) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto error;}

    msg->pesStartCodeError = NEXUS_IsrCallback_Create(msg, NULL);
    if(!msg->pesStartCodeError) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto error;}

    BKNI_EnterCriticalSection();
    /* barrier to prevent this code from interrupting isr that is already running. don't assign until entire SW state is set. */
    pTransport->message.handle[i] = msg;
    BKNI_LeaveCriticalSection();

    /* set the interrupts */
    (void)NEXUS_Message_SetSettings(msg, pSettings);

#if NEXUS_MESSAGE_USE_CHECK_TIMER
    msg->checkTimer = NEXUS_ScheduleTimer(1000, NEXUS_Message_P_CheckTimer, msg);
#endif

    return msg;

error:
    NEXUS_Message_Close(msg);
    return NULL;
}

static void NEXUS_Message_P_Finalizer(NEXUS_MessageHandle msg)
{
    unsigned i;
    NEXUS_OBJECT_ASSERT(NEXUS_Message, msg);

#if NEXUS_MESSAGE_USE_CHECK_TIMER
    if (msg->checkTimer) {
        NEXUS_CancelTimer(msg->checkTimer);
        msg->checkTimer = NULL;
    }
#endif

    BDBG_MSG(("Close %p, %d", (void *)msg, msg->MesgBufferNum));
    if (msg->started) {
        NEXUS_Message_Stop(msg);
    }
    /* isr callbacks may still occur because isr is for all message filters, not this one only */

    for (i=0;i<NEXUS_TRANSPORT_MAX_MESSAGE_HANDLES;i++) {
        if (pTransport->message.handle[i] == msg) {
            break;
        }
    }
    /* it's possible msg is not found if Open failed before completing */
    if (i < NEXUS_TRANSPORT_MAX_MESSAGE_HANDLES) {
        BKNI_EnterCriticalSection();
        /* barrier to protect against isr which is already running from accessing this handle */
        pTransport->message.handle[i] = NULL;
        BKNI_LeaveCriticalSection();
    }
    /* call this after message.handle[] is cleared.
    TODO: unless we clear HW state, a stale interrupt may be waiting for new message filter that is immediately opened with same MesgBufferNum. */
    NEXUS_Transport_P_SetInterrupts();

    if (msg->dataReady) {
        NEXUS_IsrCallback_Destroy(msg->dataReady);
    }
    if (msg->overflow) {
        NEXUS_IsrCallback_Destroy(msg->overflow);
    }
    if (msg->psiLengthError) {
        NEXUS_IsrCallback_Destroy(msg->psiLengthError);
    }
    if (msg->crcError) {
        NEXUS_IsrCallback_Destroy(msg->crcError);
    }
    if (msg->pesLengthError) {
        NEXUS_IsrCallback_Destroy(msg->pesLengthError);
    }
    if (msg->pesStartCodeError) {
        NEXUS_IsrCallback_Destroy(msg->pesStartCodeError);
    }
    if (msg->wrappedMessage.mmaBlock) {
        NEXUS_MemoryBlock_Unlock(msg->wrappedMessage.mmaBlock);
        NEXUS_MemoryBlock_UnlockOffset(msg->wrappedMessage.mmaBlock);
        NEXUS_MemoryBlock_Free(msg->wrappedMessage.mmaBlock);
    }
    if (msg->mmaAllocatedBlock) {
        NEXUS_MemoryBlock_Unlock(msg->mmaAllocatedBlock);
        NEXUS_MemoryBlock_UnlockOffset(msg->mmaAllocatedBlock);
        NEXUS_MemoryBlock_Free(msg->mmaAllocatedBlock);
    }
    if (msg->copy.buffer) {
        NEXUS_Memory_Free(msg->copy.buffer);
        msg->copy.buffer = NULL;
    }

    NEXUS_OBJECT_DESTROY(NEXUS_Message, msg);
    BKNI_Free(msg);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_Message, NEXUS_Message_Close);

void NEXUS_Message_GetSettings( NEXUS_MessageHandle msg, NEXUS_MessageSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(msg, NEXUS_Message);
    *pSettings = msg->settings;
}

NEXUS_Error NEXUS_Message_SetSettings( NEXUS_MessageHandle msg, const NEXUS_MessageSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(msg, NEXUS_Message);

    NEXUS_IsrCallback_Set(msg->dataReady, &pSettings->dataReady);
    NEXUS_IsrCallback_Set(msg->overflow, &pSettings->overflow);
    NEXUS_IsrCallback_Set(msg->psiLengthError, &pSettings->psiLengthError);
    NEXUS_IsrCallback_Set(msg->crcError, &pSettings->crcError);
    NEXUS_IsrCallback_Set(msg->pesLengthError, &pSettings->pesLengthError);
    NEXUS_IsrCallback_Set(msg->pesStartCodeError, &pSettings->pesStartCodeError);

    msg->settings = *pSettings;

    NEXUS_Transport_P_SetInterrupts();

    return 0;
}

void NEXUS_Message_GetDefaultStartSettings(NEXUS_MessageHandle msg, NEXUS_MessageStartSettings *pStartSettings)
{
    /* allow NULL msg handle to retrieve defaults w/o a handle */
    BSTD_UNUSED(msg);

    BKNI_Memset(pStartSettings, 0, sizeof(*pStartSettings));
    pStartSettings->format = NEXUS_MessageFormat_ePsi;
    pStartSettings->bufferMode = NEXUS_MessageBufferMode_eContinuous; /* deprecated */
    pStartSettings->bank = -1;
    NEXUS_Message_GetDefaultFilter(&pStartSettings->filter);
    pStartSettings->dssMessageMptFlags = NEXUS_DssMessageMptFlags_eSaveFirst;
}

void NEXUS_Message_GetDefaultFilter(NEXUS_MessageFilter *pFilter)
{
    BKNI_Memset(pFilter, 0, sizeof(*pFilter));
    BKNI_Memset(pFilter->mask, 0xFF, sizeof(pFilter->mask));
    BKNI_Memset(pFilter->exclusion, 0xFF, sizeof(pFilter->exclusion));
}

#define NEXUS_MESSAGE_P_COPY_BUFFER_TIMEOUT 250
static void NEXUS_Message_P_CopyBuffer(void *context)
{
    size_t len0, len1;
    const void *buf0, *buf1;
    NEXUS_MessageHandle msg = context;
    /* a non-destructive read will do the copy */
    NEXUS_Message_GetBufferWithWrap(msg, &buf0, &len0, &buf1, &len1);
    msg->copy.timer = NEXUS_ScheduleTimer(NEXUS_MESSAGE_P_COPY_BUFFER_TIMEOUT, NEXUS_Message_P_CopyBuffer, msg);
}

NEXUS_Error NEXUS_Message_Start(NEXUS_MessageHandle msg, const NEXUS_MessageStartSettings *pStartSettings)
{
    unsigned short pid;
    BERR_Code rc;
    BXPT_MessageBufferSize bufferSizeEnum;
    NEXUS_P_HwPidChannel *hwPidChannel;

    BDBG_OBJECT_ASSERT(msg, NEXUS_Message);
    if (msg->started) {
        BDBG_ERR(("already started"));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    if (!pStartSettings || !pStartSettings->pidChannel) {
        BDBG_ERR(("pidChannel required"));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if ((!pStartSettings->buffer || !pStartSettings->bufferSize) && !msg->allocatedBuffer) {
        BDBG_ERR(("NEXUS_Message_Start requires buffer and bufferSize"));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if (pStartSettings->buffer) {
        /* no other message filter should be using this buffer. if you were explicitly setting a shared buffer using NEXUS_MessageStartSettings.buffer, this
        will cause problems with pid2buf. Each filter needs its own buffer. */
        unsigned i;
        for (i=0;i<NEXUS_TRANSPORT_MAX_MESSAGE_HANDLES;i++) {
            if (pTransport->message.handle[i] && pTransport->message.handle[i]->buffer == pStartSettings->buffer) {
                return BERR_TRACE(NEXUS_INVALID_PARAMETER);
            }
        }
        if (msg->settings.maxContiguousMessageSize) {
            /* if this feature is enabled, the buffer must be cpu accessible inside nexus */
            if (!NEXUS_P_CpuAccessibleAddress(pStartSettings->buffer)) {
                return BERR_TRACE(NEXUS_INVALID_PARAMETER);
            }
        }
    }

    msg->startSettings = *pStartSettings;

    if (pStartSettings->buffer) {
        NEXUS_MemoryBlockHandle block;
        unsigned blockOffset;
        NEXUS_Module_Lock(g_NEXUS_Transport_P_State.moduleSettings.core);
        rc = NEXUS_MemoryBlock_BlockAndOffsetFromRange_priv((void*)pStartSettings->buffer, pStartSettings->bufferSize, &block, &blockOffset);
        NEXUS_Module_Unlock(g_NEXUS_Transport_P_State.moduleSettings.core);
        if (rc) {
            return BERR_TRACE(rc);
        }

        msg->mmaBlock = block;
        msg->buffer = pStartSettings->buffer;
        msg->bufferSize = pStartSettings->bufferSize;
    }
    else {
        msg->mmaBlock = msg->mmaAllocatedBlock;
        msg->buffer = msg->allocatedBuffer;
        msg->bufferSize = msg->settings.bufferSize;
    }

    hwPidChannel = pStartSettings->pidChannel->hwPidChannel;
    msg->PidChannelNum = hwPidChannel->status.pidChannelIndex;

    /* NOTE: the remapped PID in status should in sync with the remap setting */
    pid = hwPidChannel->status.remappedPid;

    if (msg->PidChannelNum >= BXPT_NUM_MESSAGE_CAPABLE_PID_CHANNELS) {
        BDBG_ERR(("Only pid channels 0..%d can be used for message filtering.", BXPT_NUM_MESSAGE_CAPABLE_PID_CHANNELS-1));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if (msg->copy.buffer) {
        bufferSizeEnum = BXPT_MessageBufferSize_e512kB;
    }
    else {
        unsigned bSize, temp;

        /* calculate actual bufferSize as a log of 2 */
        bufferSizeEnum = BXPT_MessageBufferSize_e1kB;
        bSize = 1024;
        temp = msg->bufferSize / 1024;
        while (temp > 1 && bSize < 512*1024)
        {
            temp /= 2; /* throw away remainder */
            bufferSizeEnum++;
            bSize *= 2;
        }
        BDBG_ASSERT(bufferSizeEnum <= BXPT_MessageBufferSize_e512kB);
        if (bSize < msg->bufferSize) {
            BDBG_WRN(("only %d out of %d bytes of message buffer will be used.", bSize, msg->bufferSize));
        }
    }

    rc = BXPT_Mesg_SetPidChannelBuffer(pTransport->xpt, msg->PidChannelNum, msg->MesgBufferNum, msg->buffer, bufferSizeEnum, NEXUS_MemoryBlock_GetBlock_priv(msg->mmaBlock));
    if (rc) {return BERR_TRACE(rc);}

    if (pStartSettings->format == NEXUS_MessageFormat_ePsi) {
        bool override;
        BXPT_PidPsiConfig psiConfig;

        rc = BXPT_Mesg_GetPidChannelPsiSettings(pTransport->xpt, msg->PidChannelNum, msg->MesgBufferNum, &override, &psiConfig);
        if (rc) return BERR_TRACE(rc);
        psiConfig.PsfCrcDis = pStartSettings->psfCrcDisabled;
#if 0
        psiConfig.SaveLongPsiMsg = true; /* set this to capture long PSI messages. */
#endif
        override = true;
        rc = BXPT_Mesg_SetPidChannelPsiSettings(pTransport->xpt, msg->PidChannelNum, msg->MesgBufferNum, override, &psiConfig);
        if (rc) return BERR_TRACE(rc);

        if (pStartSettings->bank != -1) {
            /* explicit filter selection */
            rc = BXPT_AllocPSIFilter( pTransport->xpt, pStartSettings->bank, &msg->FilterNum );
            if (rc) return BERR_TRACE(rc);

            msg->BankNum = (unsigned)pStartSettings->bank;
        }
        else {
            unsigned i;
            unsigned least_used_i = 0;
            unsigned least_used_amt = 0xFFFF;

            /* start with the least used bank */
            for (i=0; i<BXPT_NUM_FILTER_BANKS; i++){
                if (pTransport->message.bank_refcnt[i] < least_used_amt) {
                    least_used_i = i;
                    least_used_amt = pTransport->message.bank_refcnt[i];
                }
            }

            /* For easy application code, we search for an available bank. */
            for (i=least_used_i; i<BXPT_NUM_FILTER_BANKS; i++){
                rc = BXPT_AllocPSIFilter( pTransport->xpt, i, &msg->FilterNum );
                if (!rc) break;
            }
            if (i == BXPT_NUM_FILTER_BANKS) {
                for (i=0; i<least_used_i; i++){
                    rc = BXPT_AllocPSIFilter( pTransport->xpt, i, &msg->FilterNum );
                    if (!rc) break;
                }
                if (i == least_used_i) {
                    i = BXPT_NUM_FILTER_BANKS;
                }
            }
            if (i == BXPT_NUM_FILTER_BANKS) {
                return BERR_TRACE(NEXUS_INVALID_PARAMETER);
            }
            msg->BankNum = i;
        }
        pTransport->message.bank_refcnt[msg->BankNum]++;

        BDBG_CASSERT(sizeof(pStartSettings->filter) == sizeof(BXPT_Filter));
        rc = BXPT_SetFilter( pTransport->xpt, msg->BankNum, msg->FilterNum, (const BXPT_Filter *)&pStartSettings->filter);
        if (rc) { rc = BERR_TRACE(rc); goto setfilter_error;}

        BXPT_GetDefaultPsiSettings(&msg->psiMessageSettings);
        msg->psiMessageSettings.Bank = msg->BankNum;
        msg->psiMessageSettings.FilterEnableMask = 1UL<<msg->FilterNum;
        msg->psiMessageSettings.CrcDisable = pStartSettings->psiCrcDisabled;
        msg->psiMessageSettings.Pid = pid;
        msg->psiMessageSettings.Band = nexus_p_xpt_parser_band(hwPidChannel);
        msg->psiMessageSettings.StartingOffset = pStartSettings->filterOffset;
        msg->psiMessageSettings.SkipByte2 = pStartSettings->includeThirdFilterByte ? false : true;  /* BXPT preserves the hw's inverted logic. */
        msg->psiMessageSettings.UseRPipe = pStartSettings->useRPipe;

        if (msg->startSettings.filterGroup) {
            rc = BXPT_Mesg_AddFilterToGroup(pTransport->xpt, msg->PidChannelNum, msg->MesgBufferNum, msg->FilterNum, &msg->psiMessageSettings);
            if (rc) { rc = BERR_TRACE(rc); goto addfiltertogroup_error;}
        }

#if B_REFSW_DSS_SUPPORT
        if (NEXUS_IS_DSS_MODE(hwPidChannel->status.transportType)) {
            BXPT_DirecTvMessageOptions options;
            BDBG_MSG(("StartDirecTvMessageCapture %p: pidch=%d msgbuf=%d bank=%d", (void *)msg, msg->PidChannelNum, msg->MesgBufferNum, msg->BankNum));
            BKNI_Memset(&options, 0, sizeof(options));
            BDBG_CASSERT(NEXUS_DssMessageMptFlags_eMax-1 == BXPT_DirecTvMessageFlags_eSaveAll);
            options.Flags = pStartSettings->dssMessageMptFlags;
            BDBG_CASSERT(NEXUS_DssMessageType_eMax-1 == BXPT_DirecTvMessageType_eRegular_CapFilter4);
            rc = BXPT_Mesg_StartDirecTvMessageCaptureWithOptions(pTransport->xpt, msg->PidChannelNum, msg->MesgBufferNum,
                (BXPT_DirecTvMessageType)pStartSettings->dssMessageType,
                &msg->psiMessageSettings,
                &options);
            if (rc) { rc = BERR_TRACE(rc); goto start_error;}
        }
        else
#endif
        {
            BDBG_MSG(("StartPsiMessageCapture %p: pidch=%d msgbuf=%d bank=%d", (void *)msg, msg->PidChannelNum, msg->MesgBufferNum, msg->BankNum));
            rc = BXPT_Mesg_StartPsiMessageCapture(pTransport->xpt, msg->PidChannelNum, msg->MesgBufferNum, &msg->psiMessageSettings);
            if (rc) { rc = BERR_TRACE(rc); goto start_error;}
        }
    }
    else {
        BXPT_PidChannelRecordSettings ChanSettings;

        /* SWSTB-5704: Clear any old SAM filter settings before starting a new PID channel record */
        NEXUS_MessageSamSettings samSettings;
        NEXUS_Message_GetDefaultSamSettings(&samSettings);
        NEXUS_Message_SetSamSettings(msg, &samSettings);

        BXPT_GetDefaultPidChannelRecordSettings(&ChanSettings);
        ChanSettings.Pid = pid;
        ChanSettings.Band = nexus_p_xpt_parser_band(hwPidChannel);
        ChanSettings.ByteAlign = false;
        switch (pStartSettings->format) {
        case NEXUS_MessageFormat_eTs:
            ChanSettings.RecordType = BXPT_SingleChannelRecordType_ePacketSaveAll;
            break;
        case NEXUS_MessageFormat_ePes:
            ChanSettings.RecordType = BXPT_SingleChannelRecordType_ePes;
            break;
        default:
            ChanSettings.RecordType = BXPT_SingleChannelRecordType_ePesSaveAll;
            break;
        }
        ChanSettings.UseRPipe = pStartSettings->useRPipe;

        rc = BXPT_Mesg_StartPidChannelRecord(pTransport->xpt, msg->PidChannelNum, msg->MesgBufferNum, &ChanSettings);
        if (rc) return BERR_TRACE(rc);
    }

    NEXUS_Message_P_EnableInterrupt(msg, true);
    NEXUS_PidChannel_ConsumerStarted(pStartSettings->pidChannel);
    hwPidChannel->destinations |= NEXUS_PIDCHANNEL_P_DESTINATION_MESSAGE;

    /* state is controlled by length integers */
    msg->wrappedMessage.length = 0;
    msg->wrappedMessage.amountConsumed = 0;
    msg->getBufferState.length = 0;
    msg->getBufferState.amountConsumed = 0;
    msg->lastGetBufferLength = 0;

    if (msg->copy.buffer) {
        msg->copy.rptr = msg->copy.wptr = 0;
        msg->copy.timer = NEXUS_ScheduleTimer(NEXUS_MESSAGE_P_COPY_BUFFER_TIMEOUT, NEXUS_Message_P_CopyBuffer, msg);
        if (!msg->copy.timer) {
            BERR_TRACE(NEXUS_UNKNOWN);
            /* continue, app can poll */
        }
    }

    msg->started = true;
    NEXUS_OBJECT_ACQUIRE(msg, NEXUS_PidChannel, pStartSettings->pidChannel);
    return 0;

start_error:
    BXPT_RemoveFilterFromGroup(pTransport->xpt, msg->FilterNum, &msg->psiMessageSettings);
addfiltertogroup_error:
setfilter_error:
    BXPT_FreePSIFilter( pTransport->xpt, msg->BankNum, msg->FilterNum );
    pTransport->message.bank_refcnt[msg->BankNum]--;
    return rc;
}

void NEXUS_Message_Stop(NEXUS_MessageHandle msg)
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(msg, NEXUS_Message);

    if (!msg->started) {
        return;
    }

    if (msg->copy.timer) {
        NEXUS_CancelTimer(msg->copy.timer);
        msg->copy.timer = NULL;
    }

#if NEXUS_MESSAGE_USE_CHECK_TIMER
    msg->noReadCompleteCount = 0;
#endif
    NEXUS_Message_P_EnableInterrupt(msg, false);

    if (msg->startSettings.format == NEXUS_MessageFormat_ePsi) {
        unsigned i;

#if B_REFSW_DSS_SUPPORT
        if (NEXUS_IS_DSS_MODE(msg->startSettings.pidChannel->hwPidChannel->status.transportType)) {
            BDBG_MSG(("StopDirecTvMessageCapture %p: %d %d", (void *)msg, msg->PidChannelNum, msg->MesgBufferNum));
            rc = BXPT_Mesg_StopDirecTvMessageCapture(pTransport->xpt, msg->PidChannelNum, msg->MesgBufferNum );
            if (rc) rc = BERR_TRACE(rc); /* fall through */
        }
        else
#endif
        {
            BDBG_MSG(("StopPsiMessageCapture %p: %d %d", (void *)msg, msg->PidChannelNum, msg->MesgBufferNum));
            rc = BXPT_Mesg_StopPsiMessageCapture(pTransport->xpt, msg->PidChannelNum, msg->MesgBufferNum );
            if (rc) rc = BERR_TRACE(rc); /* fall through */
        }
        BXPT_FreePSIFilter(pTransport->xpt, msg->BankNum, msg->FilterNum);

        BDBG_ASSERT(pTransport->message.bank_refcnt[msg->BankNum]);
        pTransport->message.bank_refcnt[msg->BankNum]--;

        for (i=0;i<NEXUS_MESSAGE_P_MAX_ADDITIONAL_FILTERS;i++) {
            if (msg->additionalFilters[i].used) {
                NEXUS_Message_RemoveFilter(msg, msg->additionalFilters[i].filterNum);
            }
        }
    }
    else {
        rc = BXPT_Mesg_StopPidChannelRecord(pTransport->xpt, msg->PidChannelNum, msg->MesgBufferNum );
        if (rc) rc = BERR_TRACE(rc); /* fall through */
    }

    rc = BXPT_Mesg_ClearPidChannelBuffer(pTransport->xpt, msg->PidChannelNum, msg->MesgBufferNum );
    if (rc) rc = BERR_TRACE(rc); /* fall through */

    msg->startSettings.pidChannel->hwPidChannel->destinations &= ~(NEXUS_PIDCHANNEL_P_DESTINATION_MESSAGE);

    NEXUS_OBJECT_RELEASE(msg, NEXUS_PidChannel, msg->startSettings.pidChannel);
    msg->startSettings.pidChannel = NULL; /* let go of this handle. it could be closed after Stop returns. */
    msg->started = false;
    msg->buffer = NULL;
    msg->bufferSize = 0;
}

NEXUS_Error NEXUS_Message_AddFilter( NEXUS_MessageHandle msg, const NEXUS_MessageFilter *pFilter, unsigned *pFilterNum )
{
    BERR_Code rc;
    unsigned i;

    BDBG_OBJECT_ASSERT(msg, NEXUS_Message);

    if (!msg->started) {
        /* must be already started to add an additional filter */
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if (!msg->startSettings.filterGroup) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if (msg->startSettings.format != NEXUS_MessageFormat_ePsi) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    for (i=0;i<NEXUS_MESSAGE_P_MAX_ADDITIONAL_FILTERS;i++) {
        if (!msg->additionalFilters[i].used)
            break;
    }
    if (i == NEXUS_MESSAGE_P_MAX_ADDITIONAL_FILTERS) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    rc = BXPT_AllocPSIFilter( pTransport->xpt, msg->BankNum, &msg->additionalFilters[i].filterNum );
    if (rc) return BERR_TRACE(rc);

    pTransport->message.bank_refcnt[msg->BankNum]++;

    BDBG_CASSERT(sizeof(*pFilter) == sizeof(BXPT_Filter));
    rc = BXPT_SetFilter( pTransport->xpt, msg->BankNum, msg->additionalFilters[i].filterNum, (const BXPT_Filter *)pFilter);
    if (rc) {rc = BERR_TRACE(rc); goto setfilter_error;}

    rc = BXPT_Mesg_AddFilterToGroup(pTransport->xpt, msg->PidChannelNum, msg->MesgBufferNum, msg->additionalFilters[i].filterNum, &msg->psiMessageSettings);
    if (rc) {rc = BERR_TRACE(rc); goto addfilter_error;}

    msg->additionalFilters[i].used = true;
    *pFilterNum = msg->additionalFilters[i].filterNum;
    BDBG_MSG_TRACE(("NEXUS_Message_AddFilter(%p,%d) -> %d", msg, *pFilterNum, i));

    return 0;

setfilter_error:
addfilter_error:
    BXPT_FreePSIFilter( pTransport->xpt, msg->BankNum, msg->additionalFilters[i].filterNum );
    pTransport->message.bank_refcnt[msg->BankNum]--;
    return rc;
}

NEXUS_Error NEXUS_Message_RemoveFilter( NEXUS_MessageHandle msg, unsigned filterNum )
{
    unsigned i;

    BDBG_OBJECT_ASSERT(msg, NEXUS_Message);
    for (i=0;i<NEXUS_MESSAGE_P_MAX_ADDITIONAL_FILTERS;i++) {
        if (msg->additionalFilters[i].used && msg->additionalFilters[i].filterNum == filterNum) {
            BDBG_MSG_TRACE(("NEXUS_Message_RemoveFilter(%p,%d) -> %d", msg, filterNum, i));
            BXPT_RemoveFilterFromGroup(pTransport->xpt, msg->additionalFilters[i].filterNum, &msg->psiMessageSettings);
            BXPT_FreePSIFilter( pTransport->xpt, msg->BankNum, msg->additionalFilters[i].filterNum );

            msg->additionalFilters[i].used = false;
            BDBG_ASSERT(pTransport->message.bank_refcnt[msg->BankNum]);
            pTransport->message.bank_refcnt[msg->BankNum]--;
            return 0;
        }
    }
    return BERR_TRACE(NEXUS_INVALID_PARAMETER);
}

NEXUS_Error NEXUS_Message_UpdateFilter( NEXUS_MessageHandle msg, unsigned filterNum, const NEXUS_MessageFilter *pFilter )
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(msg, NEXUS_Message);

    if (!msg->started) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    /* find or verify the filterNum for this msg handle */
    if (filterNum == NEXUS_MESSAGE_MAIN_FILTER_NUM) {
        filterNum = msg->FilterNum;
    }
    else {
        unsigned i;
        for (i=0;i<NEXUS_MESSAGE_P_MAX_ADDITIONAL_FILTERS;i++) {
            if (msg->additionalFilters[i].used && msg->additionalFilters[i].filterNum == filterNum) {
                break;
            }
        }
        if (i == NEXUS_MESSAGE_P_MAX_ADDITIONAL_FILTERS) {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
    }

    rc = BXPT_SetFilter( pTransport->xpt, msg->BankNum, filterNum, (const BXPT_Filter *)pFilter);
    if (rc) return BERR_TRACE(rc);

    return 0;
}

static unsigned nexus_message_p_pad(NEXUS_MessageHandle msg, const void *buffer, unsigned length)
{
    unsigned depth, pad;
    if (!buffer || !length) return 0;
    depth = ((uint8_t*)buffer - (uint8_t*)msg->buffer) + length;
    pad = depth % 4;
    return pad ? 4 - pad : 0;
}

NEXUS_Error NEXUS_Message_GetBuffer(NEXUS_MessageHandle msg, const void **buffer, size_t *length)
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(msg, NEXUS_Message);

    if (!msg->started) {
        return NEXUS_UNKNOWN; /* fail silently */
    }
    if (msg->copy.buffer) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    if (msg->wrappedMessage.length) {
        *buffer = (uint8_t*)msg->wrappedMessage.buffer + msg->wrappedMessage.amountConsumed;
        BDBG_ASSERT(msg->wrappedMessage.length > msg->wrappedMessage.amountConsumed);
        *length = msg->wrappedMessage.length - msg->wrappedMessage.amountConsumed;
        goto done;
    }

    /* if the user's last ReadComplete did not finish off the GetBuffer, we return the amount left using cached pointers & sizes.  */
    if (msg->getBufferState.length) {
        *buffer = (uint8_t*)msg->getBufferState.buffer + msg->getBufferState.amountConsumed;
        BDBG_ASSERT(msg->getBufferState.length > msg->getBufferState.amountConsumed);
        *length = msg->getBufferState.length - msg->getBufferState.amountConsumed;
        goto done;
    }

    /* read from the HW */
    rc = BXPT_CheckBuffer( pTransport->xpt, msg->MesgBufferNum, (uint8_t **)buffer, length, &msg->status.moreDataAvailable);
    if (rc) {return BERR_TRACE(rc);}

    /* HW is configured to pad up to 4 bytes. report that back to the user and require them to consume it. */
    *length += nexus_message_p_pad(msg, *buffer, *length);

    if (!(*buffer)) {
        goto done;
    }

    if (*length) {
        NEXUS_Memory_FlushCache(*buffer, *length);
    }

    /* remember last read from HW */
    msg->getBufferState.buffer = *buffer;
    msg->getBufferState.length = *length;

#define TS_PSI_GET_SECTION_LENGTH( BUF ) (((((uint8_t*)(BUF))[1] << 8) | ((uint8_t*)(BUF))[2]) & 0x0FFF)

#if 0
    if (*length) {
        unsigned message_length;
        BDBG_ASSERT(*length >= 3);
        message_length = TS_PSI_GET_SECTION_LENGTH(*buffer) + 3;
        message_length += nexus_message_p_pad(msg, *buffer, message_length);
        BDBG_WRN(("%p: message %d, in getbuffer %d", msg, message_length, *length));
    }
#endif

    if (msg->wrappedMessage.buffer && msg->status.moreDataAvailable && msg->startSettings.format == NEXUS_MessageFormat_ePsi
#if B_REFSW_DSS_SUPPORT
        && !NEXUS_IS_DSS_MODE(msg->startSettings.pidChannel->hwPidChannel->status.transportType)
#endif
        )
    {
        uint8_t *buffer2;
        size_t length2;
        unsigned message_length;
        bool skip = false;
        bool MoreDataAvailable;

        /* the 4 byte pad ensures we can check the length before the wrap */
        BDBG_ASSERT(*length >= 3);

        /* we could have multiple messages at the end of the buffer. we only need to memcpy the one message that wrapped.
        so, parse message lengths until we get to the wrapped message. */
        message_length = TS_PSI_GET_SECTION_LENGTH(*buffer) + 3;
        message_length += nexus_message_p_pad(msg, *buffer, message_length);

        if (message_length <= *length) {
            /* if we have an unwrapped message preceding the wrapped message, change the length value to return only it for this call. this simplies our logic greatly.
            this assumes the XPT PI will return whatever portion was not consumed, including setting the MoreDataAvailable boolean again. */
            msg->getBufferState.length = *length = message_length;
            goto done;
        }

        /* now we have a single wrapped message */

        if (message_length > msg->settings.maxContiguousMessageSize) {
            BDBG_ERR(("Wrapped message of size %d bytes is larger than NEXUS_MessageSettings.maxContiguousMessageSize %d.", message_length, msg->settings.maxContiguousMessageSize));
            skip = true;
            /* keep going. still need to consume the message from HW. */
        }
        else {
            BKNI_Memcpy(msg->wrappedMessage.buffer, *buffer, *length);
        }

        /* consume the end of the buffer */
        rc = BXPT_UpdateReadOffset( pTransport->xpt, msg->MesgBufferNum, *length);
        if (rc) return BERR_TRACE(rc);

        /* get the beginning of the buffer, which could be more than just the remainder of the wrapped message.
        don't change msg->status.moreDataAvailable. */
        rc = BXPT_CheckBuffer( pTransport->xpt, msg->MesgBufferNum, &buffer2, &length2, &MoreDataAvailable);
        if (rc) {return BERR_TRACE(rc);}
        length2 += nexus_message_p_pad(msg, buffer2, length2);

        if (!skip) {
            if (length2 + *length < message_length) {
                BDBG_ERR(("Invalid wrapped message. Message is %d bytes, but two portions are %u and %u bytes.",
                    message_length, (unsigned)(*length), (unsigned)length2));
                skip = true;
                /* keep going. still need to consume the message from HW. */
            }
            else {
                /* only process the remainder of the one message */
                unsigned remainder = message_length - *length;

                if (*length) {
                    NEXUS_Memory_FlushCache(buffer2, remainder);
                }

                BKNI_Memcpy((uint8_t*)msg->wrappedMessage.buffer + *length, buffer2, remainder);

                msg->wrappedMessage.length = message_length;
                msg->wrappedMessage.amountConsumed = 0;
                BDBG_MSG(("%p copied wrapped message: %d = %u + %d", (void *)msg, msg->wrappedMessage.length, (unsigned)(*length), remainder));

                /* the user gets a pointer & size to the data in the wrappedMessage buffer */
                *buffer = msg->wrappedMessage.buffer;
                *length = msg->wrappedMessage.length;

                /* we can consume the remainder from HW right now. we are operating out of the wrappedMessage.buffer */
                rc = BXPT_UpdateReadOffset( pTransport->xpt, msg->MesgBufferNum, remainder);
                if (rc) return BERR_TRACE(rc);

                msg->getBufferState.length = 0;
                goto done;
            }
        }

        if (skip) {
            /* just consume it all and return no data. hopefully we recover. */
            rc = BXPT_UpdateReadOffset( pTransport->xpt, msg->MesgBufferNum, length2);
            if (rc) return BERR_TRACE(rc);

            *buffer = NULL;
            *length = 0;
            msg->getBufferState.length = 0;
            goto done;
        }
    }

done:
    msg->lastGetBufferLength = *length;

    if (!msg->lastGetBufferLength) {
        /* reenable the callback if zero, otherwise a call to ReadComplete is required */
        NEXUS_Message_P_EnableInterrupt(msg, true);
    }
#if NEXUS_MESSAGE_USE_CHECK_TIMER
    else {
        /* leave disabled.  */
        msg->noReadCompleteCount = 1; /* kick start checkTimer */
    }
#endif

    if (*length) {
        BDBG_MSG(("GetBuffer %p: %p %u", (void *)msg, *buffer, (unsigned)(*length)));
    }

    return 0;
}

NEXUS_Error NEXUS_Message_GetBufferWithWrap( NEXUS_MessageHandle msg, const void **pBuffer, size_t *pLength, const void **pBuffer2, size_t *pLength2 )
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(msg, NEXUS_Message);

    if (msg->started == false) {
        return BERR_UNKNOWN;
    }
    if (msg->settings.maxContiguousMessageSize) {
        BDBG_ERR(("NEXUS_Message_GetBufferWithWrap requires maxContiguousMessageSize == 0"));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    /* read from the HW */
    rc = BXPT_CheckBufferWithWrap( pTransport->xpt, msg->MesgBufferNum, (uint8_t **)pBuffer, pLength, (uint8_t **)pBuffer2, pLength2);
    if (rc) {return BERR_TRACE(rc);}

    /* HW is configured to pad up to 4 bytes. report that back to the user and require them to consume it. */
    if (*pLength2) {
        *pLength2 += nexus_message_p_pad(msg, *pBuffer2, *pLength2);
    }
    else {
        *pLength += nexus_message_p_pad(msg, *pBuffer, *pLength);
    }

    if (*pBuffer) {
        if (*pLength) {
            NEXUS_Memory_FlushCache(*pBuffer, *pLength);
        }
    }
    if (*pBuffer2) {
        if (*pLength2) {
            NEXUS_Memory_FlushCache(*pBuffer2, *pLength2);
        }
    }

    /* don't cache buffer & length with msg->getBufferState. NEXUS_Message_GetBufferWithWrap is only used if user is trying to peek ahead without completing what
    is already received, so we need the updated information. */
    msg->getBufferState.buffer = NULL;
    msg->getBufferState.length = 0;

    msg->lastGetBufferLength = *pLength + *pLength2;

    if (!msg->lastGetBufferLength) {
        /* reenable the callback if zero, otherwise a call to ReadComplete is required */
        NEXUS_Message_P_EnableInterrupt(msg, true);
    }
#if NEXUS_MESSAGE_USE_CHECK_TIMER
    else {
        /* leave disabled.  */
        msg->noReadCompleteCount = 1; /* kick start checkTimer */
    }
#endif

    if (msg->copy.buffer) {
        /* copy to SW buffer, ReadComplete the HW buffer, then return pointers to SW buffer */
        if (*pLength) {
            while (1) {
                const void *src;
                unsigned avail = msg->copy.wptr > msg->copy.rptr ? (msg->copy.size - msg->copy.wptr) : (msg->copy.rptr - msg->copy.wptr - 1);
                if (!avail) {
                    /* no space */
                    break;
                }
                else if (*pLength) {
                    src = *pBuffer;
                    if (avail > *pLength) avail = *pLength;
                    *pLength -= avail;
                    *pBuffer = (uint8_t*)*pBuffer + avail;
                }
                else if (*pLength2) {
                    src = *pBuffer2;
                    if (avail > *pLength2) avail = *pLength2;
                    *pLength2 -= avail;
                    *pBuffer2 = (uint8_t*)*pBuffer2 + avail;
                }
                else {
                    /* no data */
                    break;
                }
                /* Check for null value of src, per Coverity warning. See SWSTB-2221 */
                if(src)
                {
                    BDBG_MSG(("%p copy write: %u at %u", (void*)msg, avail, msg->copy.wptr));
                    BKNI_Memcpy(&msg->copy.buffer[msg->copy.wptr], src, avail);
                    rc = NEXUS_Message_P_ReadComplete(msg, avail);
                    if (rc) return BERR_TRACE(rc);
                    msg->copy.wptr += avail;
                    BDBG_ASSERT(msg->copy.wptr <= msg->copy.size);
                    if (msg->copy.wptr == msg->copy.size) {
                        msg->copy.wptr = 0;
                    }
                }
            }
        }
        *pBuffer = &msg->copy.buffer[msg->copy.rptr];
        if (msg->copy.wptr >= msg->copy.rptr) {
            *pLength = msg->copy.wptr - msg->copy.rptr;
            *pLength2 = 0;
            *pBuffer2 = NULL;
        }
        else {
            *pLength = msg->copy.size - msg->copy.rptr;
            *pLength2 = msg->copy.wptr;
            *pBuffer2 = msg->copy.buffer;
        }
        BDBG_MSG(("%p copy read: %u at %u, %u at 0", (void*)msg, (unsigned)*pLength, msg->copy.rptr, (unsigned)*pLength2));
        msg->lastGetBufferLength = *pLength + *pLength2;
    }

    if (msg->lastGetBufferLength) {
        BDBG_MSG(("GetBufferWithWrap %p: %p %u, %p %u", (void *)msg, *pBuffer, (unsigned)(*pLength), *pBuffer2, (unsigned)(*pLength2)));
    }

    return 0;
}

NEXUS_Error NEXUS_Message_ReadComplete(NEXUS_MessageHandle msg, size_t amountConsumed)
{
    if (amountConsumed > msg->lastGetBufferLength) {
        /* you can never consume more than you were last given. a ReadComplete call must always be preceded by a GetBuffer call. */
        BDBG_ERR(("NEXUS_Message_ReadComplete called with %u, but last NEXUS_Message_GetBuffer only returned %d", (unsigned)amountConsumed, msg->lastGetBufferLength));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if (msg->copy.buffer) {
        msg->copy.rptr += amountConsumed;
        if (msg->copy.rptr >= msg->copy.size) {
            msg->copy.rptr -= msg->copy.size;
        }
        BDBG_ASSERT(msg->copy.rptr <= msg->copy.size);
        return NEXUS_SUCCESS;
    }
    else {
        return NEXUS_Message_P_ReadComplete(msg, amountConsumed);
    }
}

static NEXUS_Error NEXUS_Message_P_ReadComplete(NEXUS_MessageHandle msg, size_t amountConsumed)
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(msg, NEXUS_Message);

    if (msg->started == false) {
        return BERR_UNKNOWN;
    }

    if (msg->wrappedMessage.length) {
        msg->wrappedMessage.amountConsumed += amountConsumed;
        BDBG_MSG(("ReadComplete %p: %u, %d out of %d consumed (wrapped)", (void *)msg, (unsigned)amountConsumed, msg->wrappedMessage.amountConsumed, msg->wrappedMessage.length));
        if (msg->wrappedMessage.amountConsumed < msg->wrappedMessage.length) {
            goto done;
        }
        BDBG_ASSERT(msg->wrappedMessage.amountConsumed == msg->wrappedMessage.length);
        msg->wrappedMessage.length = 0;
        msg->wrappedMessage.amountConsumed = 0;
        goto done;
    }

    if (msg->getBufferState.length) {
        msg->getBufferState.amountConsumed += amountConsumed;
        BDBG_MSG(("ReadComplete %p: %u, %d out of %d consumed", (void *)msg, (unsigned)amountConsumed, msg->getBufferState.amountConsumed, msg->getBufferState.length));

        if (msg->getBufferState.amountConsumed < msg->getBufferState.length) {
            /* we can't update HW until the full GetBuffer length is consumed. this is required so that we don't have to parse message
            lengths until we hit a wrap around. at that point, we must parse lengths so that we can copy only the one wrapped message into wrappedMessage.buffer. */
            goto done;
        }

        /* user has consumed it all. now tell HW. */
        BDBG_ASSERT(msg->getBufferState.amountConsumed == msg->getBufferState.length);

        rc = BXPT_UpdateReadOffset( pTransport->xpt, msg->MesgBufferNum, msg->getBufferState.length);
        if (rc) return BERR_TRACE(rc);

        msg->getBufferState.length = 0;
        msg->getBufferState.amountConsumed = 0;
    }
    else {
        rc = BXPT_UpdateReadOffset( pTransport->xpt, msg->MesgBufferNum, amountConsumed);
        if (rc) return BERR_TRACE(rc);
    }

done:
    /* reenable the callback */
    NEXUS_Message_P_EnableInterrupt(msg, true);
#if NEXUS_CPU_ARM
    {
        /* for 28nm, new data must arrive for a HW interrupt. to keep backward compat, nexus must fire if any data remains. */
        bool fire = (amountConsumed < msg->lastGetBufferLength);
        if (!fire) {
            uint8_t *buf, *buf2;
            size_t len, len2;
            /* more data may we sitting on wraparound */
            rc = BXPT_CheckBufferWithWrap(pTransport->xpt, msg->MesgBufferNum, &buf, &len, &buf2, &len2);
            fire = (!rc && (len || len2));
        }
        if (fire) {
            BKNI_EnterCriticalSection();
            NEXUS_IsrCallback_Fire_isr(msg->dataReady);
            BKNI_LeaveCriticalSection();
        }
    }
#endif
#if NEXUS_MESSAGE_USE_CHECK_TIMER
    msg->noReadCompleteCount = 0;
#endif
    msg->lastGetBufferLength = 0; /* require another GetBuffer before a ReadComplete, even if we have a remainder */
    return 0;
}

NEXUS_Error NEXUS_Message_GetStatus( NEXUS_MessageHandle msg, NEXUS_MessageStatus *pStatus )
{
    BDBG_OBJECT_ASSERT(msg, NEXUS_Message);
    *pStatus = msg->status;
    return 0;
}

NEXUS_Error NEXUS_Message_SetDssCapPattern( unsigned capFilterIndex, uint32_t pattern)
{
    #if B_REFSW_DSS_SUPPORT
    return BXPT_DirecTv_SetCapPattern(pTransport->xpt, capFilterIndex, pattern);
    #else
    /* get of compiler warnings for the ! B_REFSW_DSS_SUPPORT case. */
    capFilterIndex = capFilterIndex; pattern = pattern;
    return BERR_SUCCESS;
    #endif
}

#else /* NEXUS_NUM_MESSAGE_FILTERS */

/* stubs */

struct NEXUS_Message {
    BDBG_OBJECT(NEXUS_Message)
};

void NEXUS_Message_GetDefaultSettings(NEXUS_MessageSettings *pSettings)
{
    BSTD_UNUSED(pSettings);
    BDBG_WRN(("Message Filter not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return;
}

NEXUS_MessageHandle NEXUS_Message_Open(const NEXUS_MessageSettings *pSettings)
{
    BSTD_UNUSED(pSettings);
    BDBG_WRN(("Message Filter not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
}

void NEXUS_Message_Close(NEXUS_MessageHandle msg)
{
    BSTD_UNUSED(msg);
    BDBG_WRN(("Message Filter not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return;
}

void NEXUS_Message_GetSettings( NEXUS_MessageHandle msg, NEXUS_MessageSettings *pSettings )
{
    BSTD_UNUSED(msg);
    BSTD_UNUSED(pSettings);
    BDBG_WRN(("Message Filter not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return;
}

NEXUS_Error NEXUS_Message_SetSettings( NEXUS_MessageHandle msg, const NEXUS_MessageSettings *pSettings )
{
    BSTD_UNUSED(msg);
    BSTD_UNUSED(pSettings);
    BDBG_WRN(("Message Filter not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return 0;
}

void NEXUS_Message_GetDefaultStartSettings(NEXUS_MessageHandle msg, NEXUS_MessageStartSettings *pStartSettings)
{
    BSTD_UNUSED(msg);
    BSTD_UNUSED(pStartSettings);
    BDBG_WRN(("Message Filter not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return;
}

NEXUS_Error NEXUS_Message_Start(NEXUS_MessageHandle msg, const NEXUS_MessageStartSettings *pStartSettings)
{
    BSTD_UNUSED(msg);
    BSTD_UNUSED(pStartSettings);
    BDBG_WRN(("Message Filter not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return 0;
}

void NEXUS_Message_Stop(NEXUS_MessageHandle msg)
{
    BSTD_UNUSED(msg);
    BDBG_WRN(("Message Filter not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return;
}

NEXUS_Error NEXUS_Message_GetBuffer(NEXUS_MessageHandle msg, const void **buffer, size_t *length)
{
    BSTD_UNUSED(msg);
    BSTD_UNUSED(buffer);
    BSTD_UNUSED(length);
    BDBG_WRN(("Message Filter not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return 0;
}

NEXUS_Error NEXUS_Message_ReadComplete(NEXUS_MessageHandle msg, size_t amountConsumed)
{
    BSTD_UNUSED(msg);
    BSTD_UNUSED(amountConsumed);
    BDBG_WRN(("Message Filter not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return 0;
}

NEXUS_Error NEXUS_Message_GetStatus( NEXUS_MessageHandle msg, NEXUS_MessageStatus *pStatus )
{
    BSTD_UNUSED(msg);
    BSTD_UNUSED(pStatus);
    BDBG_WRN(("Message Filter not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return 0;
}

NEXUS_Error NEXUS_Message_SetDssCapPattern( unsigned capFilterIndex, uint32_t pattern)
{
    BSTD_UNUSED(capFilterIndex);
    BSTD_UNUSED(pattern);
    BDBG_WRN(("Message Filter not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return 0;
}

void NEXUS_Message_GetDefaultFilter(NEXUS_MessageFilter *pFilter)
{
    BKNI_Memset(pFilter, 0, sizeof(*pFilter));
    BKNI_Memset(pFilter->mask, 0xFF, sizeof(pFilter->mask));
    BKNI_Memset(pFilter->exclusion, 0xFF, sizeof(pFilter->exclusion));
}

NEXUS_Error NEXUS_Message_AddFilter( NEXUS_MessageHandle msg, const NEXUS_MessageFilter *pFilter, unsigned *pFilterNum )
{
    BDBG_OBJECT_ASSERT(msg, NEXUS_Message);
    BSTD_UNUSED(pFilter);
    BSTD_UNUSED(pFilterNum);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_Message_RemoveFilter( NEXUS_MessageHandle msg, unsigned filterNum )
{
    BDBG_OBJECT_ASSERT(msg, NEXUS_Message);
    BSTD_UNUSED(filterNum);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_Message_UpdateFilter( NEXUS_MessageHandle msg, unsigned filterNum, const NEXUS_MessageFilter *pFilter )
{
    BDBG_OBJECT_ASSERT(msg, NEXUS_Message);
    BSTD_UNUSED(filterNum);
    BSTD_UNUSED(pFilter);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

#endif /* NEXUS_NUM_MESSAGE_FILTERS */

/* private functions */

void NEXUS_Message_P_FireInterrupt_isr(NEXUS_MessageHandle msg, unsigned pidChannelIndex, NEXUS_XptDataInterrupt xptDataInterrupt)
{
    BDBG_OBJECT_ASSERT(msg, NEXUS_Message);

#if NEXUS_NUM_MESSAGE_FILTERS
    /* In this mode, the pidChannelIndex given is actually the message buffer number */
    if (msg->MesgBufferNum == (int)pidChannelIndex) {
        switch (xptDataInterrupt) {
        case NEXUS_XptDataInterrupt_ePsiLengthError:
            NEXUS_IsrCallback_Fire_isr(msg->psiLengthError);
            break;
        case NEXUS_XptDataInterrupt_eCrcError:
            NEXUS_IsrCallback_Fire_isr(msg->crcError);
            break;
        case NEXUS_XptDataInterrupt_ePesLengthError:
            NEXUS_IsrCallback_Fire_isr(msg->pesLengthError);
            break;
        case NEXUS_XptDataInterrupt_ePesStartCodeError:
            NEXUS_IsrCallback_Fire_isr(msg->pesStartCodeError);
            break;
        default:
            break;
        }
    }
#else
    BSTD_UNUSED(pidChannelIndex);
    BSTD_UNUSED(xptDataInterrupt);
#endif
}

bool NEXUS_Message_P_HasCallback(NEXUS_MessageHandle msg, NEXUS_XptDataInterrupt xptDataInterrupt)
{
    BDBG_OBJECT_ASSERT(msg, NEXUS_Message);
#if NEXUS_NUM_MESSAGE_FILTERS
    switch (xptDataInterrupt) {
    case NEXUS_XptDataInterrupt_ePsiLengthError: return (msg->settings.psiLengthError.callback != NULL);
    case NEXUS_XptDataInterrupt_eCrcError: return (msg->settings.crcError.callback != NULL);
    case NEXUS_XptDataInterrupt_ePesLengthError: return (msg->settings.pesLengthError.callback != NULL);
    case NEXUS_XptDataInterrupt_ePesStartCodeError: return (msg->settings.pesStartCodeError.callback != NULL);
    default: return false;
    }
#else
    BSTD_UNUSED(xptDataInterrupt);
    return false;
#endif
}

/***************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 * This is the implementation of a software message filter. It
 * is based on the 7003 SW message filtering at BCM3543\tests\filter\
 * NEXUS_Recpump is used to record the data, and then a filter is applied to the
 * captured data.
 *  The SW filter consists of 2 parts
 *     - the HW dependent part: which calls RAVE/Nexus_Recpump to capture the data
 *       and then passes the data on to the actual parser/filters in the file
 *       nexus_message_swfilter_parser.c . This interfaces with NEXUS message module on
 *       one end and the HW independent filter on the other end.
 *     - the HW independent part (nexus_message_swfilter_parser.c): this parsers
 *       and filters the data that is passed to it.
 *
 * Nexus part is based on /nexus/modules/transport/7400/src/nexus_message_pid2buf.c
 *
 *
 **************************************************************************/
#include "nexus_transport_module.h"

BDBG_MODULE(nexus_message);

#define BDBG_MSG_TRACE(X) /* BDBG_MSG(X) */

/* macros added for adaptation to older nexus code */
#define B_CALLBACK_HANDLER_SUPPORT 1
#define B_NEW_PARSER_BAND 1

/* if you use one recpump per PES filter, you can just do HW filtering. faster and no possible SW parsing bugs */
#define B_USE_HW_PES_FILTERING 0

#if BCHP_CHIP == 7255 || NEXUS_USE_OTT_TRANSPORT
#define NO_PID2BUF 1 /* TODO evaluate */
#endif

/* this is the impl of NEXUS_MessageHandle */
struct NEXUS_Message {
    NEXUS_OBJECT(NEXUS_Message);
    NEXUS_MessageSettings settings;
    NEXUS_MessageStartSettings startSettings;

    uint8_t *buffer; /* actual buffer in use. cached addr. */
    unsigned bufferSize;
    unsigned wptr, rptr, wrapptr; /* ringbuffer logic. wrapptr allows complete use of buffer, just like HW. and allows for
        contiguous PSI msgs w/o memcpy. key logic is:
            empty buffer: wptr==rptr && !wrapptr
            full buffer:  wptr==rptr && wrapptr
        */

    void *allocatedBuffer; /* buffer allocated by Open if bufferSize is set. can be overridden by Start buffer. */

    void *tempBuffer; /* single message buffer */

    unsigned lastGetBufferLength; /* last length returned by NEXUS_Message_GetBuffer. this could be from the main
                                     buffer or wrappedMessage. */

    bool started;
    NEXUS_TaskCallbackHandle dataReady, overflow;
    NEXUS_IsrCallbackHandle psiLengthError, crcError, pesLengthError, pesStartCodeError;

    struct NEXUS_SwFilterCapture *stream;
    struct NEXUS_SwFilterPid *pid;
    struct NEXUS_SwFilter_FilterState *filter;
    unsigned tempBufSize; /* size of tempBuffer */
    bool isDss;           /* true if Dss message filter */
};

#define SM_POLL_INTERVAL 50

/* one per pidchannel & recpump */
BDBG_OBJECT_ID(NEXUS_SwFilterPid);
struct NEXUS_SwFilterPid {
    BDBG_OBJECT(NEXUS_SwFilterPid)
    struct NEXUS_SwFilterCapture *stream;
    BLST_S_ENTRY(NEXUS_SwFilterPid) link; /* NEXUS_SwFilterCapture.pids */
    NEXUS_PidChannelHandle pidChannel;
    NEXUS_PlaypumpHandle playpump;
    unsigned refcnt;
};

/* one per recpump */
BDBG_OBJECT_ID(NEXUS_SwFilterCapture);
struct NEXUS_SwFilterCapture {
    BDBG_OBJECT(NEXUS_SwFilterCapture)
    BLST_S_ENTRY(NEXUS_SwFilterCapture) link; /* g_swfilter_state_list */
    NEXUS_SwMsgFilterHandle handle;
    NEXUS_RecpumpHandle recpump;
    unsigned recpumpIndex;
    unsigned refcnt;
    NEXUS_MessageFormat format;
    NEXUS_TimerHandle parseDataTimer; /* for PSI and PES data */
    BLST_S_HEAD(NEXUS_SwFilterPidList, NEXUS_SwFilterPid) pids;
#if B_CALLBACK_HANDLER_SUPPORT
    NEXUS_CallbackHandler dataReadyCallbackHandler;
#endif
    NEXUS_MessageHandle msg; /* set for eTs where recpump cannot be shared */
    bool isDss;  /* true if Dss msg filtering */
};

/* static global variable */
static BLST_S_HEAD(NEXUS_SwFilterCaptureList, NEXUS_SwFilterCapture) g_swfilter_state_list;

static struct NEXUS_SwFilterCapture *NEXUS_SwFilter_P_Open(NEXUS_MessageHandle msg);
static void NEXUS_SwFilter_P_Close(struct NEXUS_SwFilterCapture *stream);
static struct NEXUS_SwFilterPid *NEXUS_SwFilter_P_OpenPid(NEXUS_MessageHandle msg);
static void NEXUS_SwFilter_P_ClosePid(struct NEXUS_SwFilterPid *pid);

static void * NEXUS_SwFilter_P_FilterCallback(void * context, size_t msg_size);
static void NEXUS_SwFilter_P_DataReady(void *context);
static void NEXUS_SwFilter_P_PollTask(void *context);

/********************************************************************/

void NEXUS_Message_GetDefaultSettings(NEXUS_MessageSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->bufferSize = 4 * 1024;
    pSettings->maxContiguousMessageSize = 0;
    pSettings->recpumpIndex = NEXUS_ANY_ID;
    NEXUS_CallbackDesc_Init(&pSettings->dataReady);
    NEXUS_CallbackDesc_Init(&pSettings->overflow);
    NEXUS_CallbackDesc_Init(&pSettings->psiLengthError);
    NEXUS_CallbackDesc_Init(&pSettings->crcError);
    NEXUS_CallbackDesc_Init(&pSettings->pesLengthError);
    NEXUS_CallbackDesc_Init(&pSettings->pesStartCodeError);
}

NEXUS_MessageHandle NEXUS_Message_Open(const NEXUS_MessageSettings *pSettings)
{
    NEXUS_Error rc;
    NEXUS_MessageSettings settings;
    NEXUS_MessageHandle msg;
    unsigned i;

    if (!pSettings) {
        NEXUS_Message_GetDefaultSettings(&settings);
        pSettings = &settings;
    }

    msg = BKNI_Malloc(sizeof(*msg));
    if (!msg) {
        BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_Message, msg);
    msg->settings = *pSettings;

    if (pSettings->bufferSize) {
        NEXUS_MemoryAllocationSettings allocSettings;

        /* alloc from heap just in case app is expecting it. */
        NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
        allocSettings.alignment = 1024;
        allocSettings.heap = NEXUS_P_DefaultHeap(NULL, NEXUS_DefaultHeapType_eFull);
        rc = NEXUS_Memory_Allocate(pSettings->bufferSize, &allocSettings, &msg->allocatedBuffer); /* default is heap[0] */
        if (rc) {
            rc = BERR_TRACE(rc);
            goto error;
        }
    }

#ifdef NO_PID2BUF
    BDBG_MSG(("Set maxContiguousMessageSize" ));
#else
    if (pSettings->maxContiguousMessageSize) {
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto error;
    }
#endif

    /* must keep track of Message handles in order to route interrupts back out to Message handles */
    for (i=0;i<NEXUS_TRANSPORT_MAX_MESSAGE_HANDLES;i++) {
        if (!pTransport->message.handle[i]) {
            pTransport->message.handle[i] = msg;
            break;
        }
    }
    if (i == NEXUS_TRANSPORT_MAX_MESSAGE_HANDLES) {
        BDBG_ERR(("You must increase NEXUS_TRANSPORT_MAX_MESSAGE_HANDLES"));
        goto error;
    }

    msg->dataReady = NEXUS_TaskCallback_Create(msg, NULL);
    if(!msg->dataReady) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto error;}

    msg->overflow = NEXUS_TaskCallback_Create(msg, NULL);
    if(!msg->overflow) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto error;}

    msg->psiLengthError = NEXUS_IsrCallback_Create(msg, NULL);
    if(!msg->psiLengthError) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto error;}

    msg->crcError = NEXUS_IsrCallback_Create(msg, NULL);
    if(!msg->crcError) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto error;}

    msg->pesLengthError = NEXUS_IsrCallback_Create(msg, NULL);
    if(!msg->pesLengthError) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto error;}

    msg->pesStartCodeError = NEXUS_IsrCallback_Create(msg, NULL);
    if(!msg->pesStartCodeError) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto error;}

    msg->isDss = false;
    /* set the interrupts */
    (void)NEXUS_Message_SetSettings(msg, pSettings);

    BDBG_MSG(("Open %p", (void *)msg));
    return msg;

error:
    NEXUS_Message_Close(msg);
    return NULL;
}

static void NEXUS_Message_P_Finalizer(NEXUS_MessageHandle msg)
{
    unsigned i;
    NEXUS_OBJECT_ASSERT(NEXUS_Message, msg);

    BDBG_MSG(("Close %p", (void *)msg));
    if (msg->started) {
        NEXUS_Message_Stop(msg);
    }
    /* Guaranteed no callbacks now */

    for (i=0;i<NEXUS_TRANSPORT_MAX_MESSAGE_HANDLES;i++) {
        if (pTransport->message.handle[i] == msg) {
            pTransport->message.handle[i] = NULL;
            break;
        }
    }
    BDBG_ASSERT(i < NEXUS_TRANSPORT_MAX_MESSAGE_HANDLES);
    /* call this after message.handle[] is cleared */
    NEXUS_Transport_P_SetInterrupts();

    if (msg->dataReady) {
        NEXUS_TaskCallback_Destroy(msg->dataReady);
    }
    if (msg->overflow) {
        NEXUS_TaskCallback_Destroy(msg->overflow);
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
    if (msg->allocatedBuffer) {
        NEXUS_Memory_Free(msg->allocatedBuffer);
    }

    if (msg->tempBuffer) {
        BKNI_Free(msg->tempBuffer);
        msg->tempBuffer  = NULL;
        msg->tempBufSize = 0;
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

    NEXUS_TaskCallback_Set(msg->dataReady, &pSettings->dataReady);
    NEXUS_TaskCallback_Set(msg->overflow, &pSettings->overflow);
    NEXUS_IsrCallback_Set(msg->psiLengthError, &pSettings->psiLengthError);
    NEXUS_IsrCallback_Set(msg->crcError, &pSettings->crcError);
    NEXUS_IsrCallback_Set(msg->pesLengthError, &pSettings->pesLengthError);
    NEXUS_IsrCallback_Set(msg->pesStartCodeError, &pSettings->pesStartCodeError);
    NEXUS_Transport_P_SetInterrupts();

    msg->settings = *pSettings;

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
    pStartSettings->filterGroup = false;
}

NEXUS_Error NEXUS_Message_Start(NEXUS_MessageHandle msg, const NEXUS_MessageStartSettings *pStartSettings)
{
    NEXUS_Error rc;
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
    if ((!pStartSettings->buffer || !NEXUS_P_CpuAccessibleAddress(pStartSettings->buffer) || !pStartSettings->bufferSize) && !msg->allocatedBuffer) {
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
    }
    if (pStartSettings->filterGroup) {
        /* If you want filter group support, use the old magnum/syslib/msglib implementation. The sw demux of a filter group buffer is only in msglib software.
        We could consider supporting PID2BUF with filter groups and no sw demux, but this would not be backward compatible with the NEXUS Message API.
        Because the current implementation is compatible, this is only a BDBG_WRN. */
        BDBG_WRN(("Message filter groups are not supported with nexus_message_swfilter.c."));
    }
    switch (pStartSettings->format) {
    case NEXUS_MessageFormat_ePsi:
    case NEXUS_MessageFormat_eTs:
    case NEXUS_MessageFormat_ePes:
        break;
    default:
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    msg->startSettings = *pStartSettings;

    if (pStartSettings->buffer) {
        msg->buffer = pStartSettings->buffer;
        msg->bufferSize = pStartSettings->bufferSize;
    }
    else {
        msg->buffer = msg->allocatedBuffer;
        msg->bufferSize = msg->settings.bufferSize;
    }
    hwPidChannel = pStartSettings->pidChannel->hwPidChannel;
    if (NEXUS_IS_DSS_MODE(hwPidChannel->status.transportType)) {
        msg->isDss = true;
    }

#ifdef NO_PID2BUF
    BDBG_MSG(("No pid2buf support" ));
#else
    if ( msg->settings.recpumpIndex == NEXUS_ANY_ID ) { /* only do search if ANY_ID */
          unsigned findParser;
          unsigned i,listParser;
          NEXUS_MessageHandle listMsg;

          if ( hwPidChannel->parserBand && hwPidChannel->parserBand->enumBand ) {

              findParser = hwPidChannel->parserBand->enumBand;
              for (i=0;i<NEXUS_TRANSPORT_MAX_MESSAGE_HANDLES;i++) {
                  if (pTransport->message.handle[i] ) {
                      listMsg = pTransport->message.handle[i];
                      if ( listMsg->startSettings.pidChannel == NULL ) {
                          continue;
                      }
                      listParser = listMsg->startSettings.pidChannel->hwPidChannel->parserBand->enumBand;
                      if( listParser == findParser && listMsg->settings.recpumpIndex != NEXUS_ANY_ID ) {
                            msg->settings.recpumpIndex = listMsg->settings.recpumpIndex;
                            BDBG_MSG(("Re Use parser with recpumpIdx=%d" , msg->settings.recpumpIndex  ));
                            break;
                        }
                  }
              }
        }
        else {
            BDBG_MSG(("parserband not initialized " ));
        }
      }
#endif
    /* don't enforce HW restrictions in SW filtering. there may be a use-case for more flexibility. */

    msg->stream = NEXUS_SwFilter_P_Open(msg);
    if (!msg->stream) {
        rc = BERR_TRACE(NEXUS_UNKNOWN);
        goto err_open;
    }

    msg->pid = NEXUS_SwFilter_P_OpenPid(msg);
    if (!msg->pid) {
        rc = BERR_TRACE(NEXUS_UNKNOWN);
        goto err_openpid;
    }

    BDBG_MSG(("Start(%p, recpump %d): pid %#x", (void *)msg, msg->stream->recpumpIndex, hwPidChannel->status.pid));

#if B_USE_HW_PES_FILTERING
    if (pStartSettings->format == NEXUS_MessageFormat_ePsi) {
#else
    if ((pStartSettings->format == NEXUS_MessageFormat_ePsi) || (pStartSettings->format == NEXUS_MessageFormat_ePes)) {
#endif
        /* filtered formats */
        NEXUS_SwFilter_MsgParams_t m_params;

        BKNI_Memset(&m_params, 0, sizeof(m_params));
        m_params.pid = hwPidChannel->status.pid;
        BKNI_Memcpy(m_params.filt.coefficient, msg->startSettings.filter.coefficient, NEXUS_MESSAGE_FILTER_SIZE);
        BKNI_Memcpy(m_params.filt.mask, msg->startSettings.filter.mask, NEXUS_MESSAGE_FILTER_SIZE);
        BKNI_Memcpy(m_params.filt.exclusion, msg->startSettings.filter.exclusion, NEXUS_MESSAGE_FILTER_SIZE);

        if ( msg->tempBuffer && msg->stream->format != NEXUS_MessageFormat_ePsi && msg->tempBufSize != (64*1024) ) {
            /* de-allocate so we can get a right sized buffer */
            BKNI_Free(msg->tempBuffer);
            msg->tempBuffer = NULL;
        }

        if (msg->stream->format == NEXUS_MessageFormat_ePsi) {
            m_params.buffer_size = 4*1024; /* max PSI length */
        }
        else {
            m_params.buffer_size = 64*1024; /* max PES length */
        }

        if ( msg->tempBuffer == NULL) {
            /* m_params.buffer is a temporary, single message buffer */
            msg->tempBuffer = BKNI_Malloc(m_params.buffer_size);
            if (!msg->tempBuffer) {
                rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
                goto err_malloc;
            }
            msg->tempBufSize = m_params.buffer_size;
        }
        m_params.buffer = msg->tempBuffer;
        m_params.disable_crc_check = msg->startSettings.psiCrcDisabled;
        m_params.callback = NEXUS_SwFilter_P_FilterCallback;
        m_params.context = msg;
        if (msg->stream->format == NEXUS_MessageFormat_ePsi) {
            if ( msg->isDss ) {
                msg->filter = NEXUS_SwFilter_Msg_P_SetDssFilter(msg->stream->handle, &m_params,msg->startSettings.dssMessageType, msg->startSettings.dssMessageMptFlags);
                if (!msg->filter) {rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto err_setfilter;}
            }
            else
            {
                msg->filter = NEXUS_SwFilter_Msg_P_SetFilter(msg->stream->handle, &m_params);
                if (!msg->filter) {rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto err_setfilter;}
            }
        }
        else {
            msg->filter = NEXUS_SwFilter_Msg_P_SetPesFilter(msg->stream->handle, &m_params);
            if (!msg->filter) {rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto err_setfilter;}
        }

        msg->rptr = msg->wptr = msg->wrapptr = 0;
    }

    NEXUS_PidChannel_ConsumerStarted(pStartSettings->pidChannel);

    /* state is controlled by length integers */
    msg->lastGetBufferLength = 0;

    msg->started = true;
    NEXUS_OBJECT_ACQUIRE(msg, NEXUS_PidChannel, pStartSettings->pidChannel);
    return 0;

err_setfilter:
    BKNI_Free(msg->tempBuffer);
err_malloc:
    NEXUS_SwFilter_P_ClosePid(msg->pid);
err_openpid:
    NEXUS_SwFilter_P_Close(msg->stream);
err_open:
    return rc;
}

void NEXUS_Message_Stop(NEXUS_MessageHandle msg)
{

    BDBG_OBJECT_ASSERT(msg, NEXUS_Message);
    if (!msg->started) {
        return;
    }
    BDBG_MSG(("Stop %p", (void *)msg));

#if 0 /* B_REFSW_DSS_SUPPORT */
    {
        NEXUS_P_HwPidChannel *hwPidChannel;
        hwPidChannel = msg->startSettings.pidChannel->hwPidChannel;
        if (NEXUS_IS_DSS_MODE(hwPidChannel->status.transportType)) {
            isDss = true;
        }
    }
#endif

    NEXUS_OBJECT_RELEASE(msg, NEXUS_PidChannel, msg->startSettings.pidChannel);
    if (msg->filter){
        if (msg->startSettings.format == NEXUS_MessageFormat_ePsi) {
            NEXUS_SwFilter_Msg_P_RemoveFilter(msg->stream->handle, msg->filter);
        }
        else {
            NEXUS_SwFilter_Msg_P_RemovePesFilter(msg->stream->handle, msg->filter);
        }
        msg->filter = NULL;
    }
    NEXUS_SwFilter_P_ClosePid(msg->pid);
    NEXUS_SwFilter_P_Close(msg->stream);
    msg->settings.recpumpIndex = NEXUS_ANY_ID;
    msg->started = false;
    msg->buffer = NULL;
    msg->bufferSize = 0;
}

NEXUS_Error NEXUS_Message_GetBuffer(NEXUS_MessageHandle msg, const void **buffer, size_t *length)
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(msg, NEXUS_Message);

    if (!msg->started) {
        return NEXUS_UNKNOWN; /* fail silently */
    }

    if (msg->filter) {
        if (msg->wrapptr) {
            BDBG_ASSERT(msg->wptr <= msg->rptr);
            *length = msg->wrapptr - msg->rptr;
        }
        else {
            BDBG_ASSERT(msg->wptr >= msg->rptr);
            *length = msg->wptr - msg->rptr;
        }
        BDBG_ASSERT(*length < msg->bufferSize);
        if (*length) {
            *buffer = &msg->buffer[msg->rptr];
        }
        else {
            *buffer = NULL;
        }
    }
    else {
        rc = NEXUS_Recpump_GetDataBuffer(msg->stream->recpump, buffer, length);
        if (rc) return BERR_TRACE(rc);
    }

    msg->lastGetBufferLength = *length;
    BDBG_MSG(("NEXUS_Message_GetBuffer %p: %p %d", (void *)msg, *buffer, (unsigned)*length));

    return 0;
}

NEXUS_Error NEXUS_Message_GetBufferWithWrap( NEXUS_MessageHandle msg, const void **pBuffer, size_t *pLength, const void **pBuffer2, size_t *pLength2 )
{
    /* TODO: this could be supported now */
    *pBuffer2 = NULL;
    *pLength2 = 0;
    return NEXUS_Message_GetBuffer(msg, pBuffer, pLength);
}

NEXUS_Error NEXUS_Message_ReadComplete(NEXUS_MessageHandle msg, size_t amount_consumed)
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(msg, NEXUS_Message);

    if (msg->started == false) {
        return NEXUS_UNKNOWN;
    }
    if (amount_consumed > msg->lastGetBufferLength) {
        BDBG_ERR(("lastGetBufferLength=%d. Therefore can't consume %d bytes",
            msg->lastGetBufferLength, (unsigned)amount_consumed));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    /* In more recent silicon, this 4 byte pad is programmable. But for now the XPT PI defaults it on. */
    if (msg->startSettings.format == NEXUS_MessageFormat_ePsi && amount_consumed % 4) {
        amount_consumed += 4 - (amount_consumed % 4);
        if (amount_consumed > msg->lastGetBufferLength) {
            BDBG_ERR(("4 byte padding corrupted"));
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
    }

    if (msg->filter) {
        msg->rptr += amount_consumed;
        BDBG_ASSERT(msg->wrapptr ? msg->rptr <= msg->wrapptr : msg->rptr <= msg->wptr);
        if (msg->rptr == msg->wrapptr) {
            msg->rptr = 0;
            msg->wrapptr = 0;
        }
        else if (msg->rptr == msg->wptr && !msg->wrapptr) {
            /* if we catch up and empty the buffer, reset to 0 to maximize write space */
            msg->wptr = msg->rptr = 0;
        }
    }
    else {
        rc = NEXUS_Recpump_DataReadComplete(msg->stream->recpump, amount_consumed);
        if (rc) return BERR_TRACE(rc);
    }

    BDBG_MSG(("ReadComplete %p: %d out of %d", (void *)msg, (unsigned)amount_consumed, msg->lastGetBufferLength));
    msg->lastGetBufferLength = 0;
    return 0;
}

NEXUS_Error NEXUS_Message_GetStatus( NEXUS_MessageHandle msg, NEXUS_MessageStatus *pStatus )
{
    BDBG_OBJECT_ASSERT(msg, NEXUS_Message);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    /* no groups; therefore no status. */
    return 0;
}

#if !B_CALLBACK_HANDLER_SUPPORT
static void NEXUS_SwFilter_P_DataReadyWrapper(void *context, int unused)
{
    BSTD_UNUSED(unused);
    NEXUS_LockModule();
    NEXUS_SwFilter_P_DataReady(context);
    NEXUS_UnlockModule();
}
#endif

/* Open a message stream for a particular recpump and format of data */
static struct NEXUS_SwFilterCapture *NEXUS_SwFilter_P_Open(NEXUS_MessageHandle msg)
{
    NEXUS_RecpumpSettings recpumpSettings;
    NEXUS_RecpumpOpenSettings settings;
    struct NEXUS_SwFilterCapture *st;
    NEXUS_Error rc = NEXUS_SUCCESS;

    for (st = BLST_S_FIRST(&g_swfilter_state_list); st; st = BLST_S_NEXT(st, link)) {
        BDBG_OBJECT_ASSERT(st, NEXUS_SwFilterCapture);
        if (st->recpumpIndex == msg->settings.recpumpIndex) {
            if (st->format != msg->startSettings.format) {
                BDBG_ERR(("must use different recpump for different formats"));
                return NULL;
            }
#if B_USE_HW_PES_FILTERING
            if (msg->startSettings.format != NEXUS_MessageFormat_ePsi) {
#else
            if (msg->startSettings.format == NEXUS_MessageFormat_eTs) {
#endif
                BDBG_ERR(("this message filter requires an exclusive recpump"));
                return NULL;
            }
            st->refcnt++;
            return st;
        }
    }
    st = BKNI_Malloc(sizeof(*st));
    if (!st) {
        BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    BKNI_Memset(st, 0, sizeof(*st));
    BDBG_OBJECT_SET(st, NEXUS_SwFilterCapture);

    st->handle = NEXUS_SwFilter_Msg_P_Init();
    if (!st->handle) {
        BERR_TRACE(NEXUS_UNKNOWN);
        goto err_swfilter_init;
    }

    /* we'll use only data buffer, no index buffer from the recpump*/
    NEXUS_Recpump_GetDefaultOpenSettings(&settings);

    /* to avoid waiting for data that's available - keep a low threshold and atom size */

    if (msg->startSettings.format == NEXUS_MessageFormat_ePes)
    {   /* CC data comes as PES packets. And we want to get each pkt as soon as it comes.
           So keep atomsize=0 & threshold=1 pkt */
        settings.data.atomSize = 0;
        settings.data.dataReadyThreshold = 256*2;
#if B_USE_HW_PES_FILTERING
        st->msg = msg;
#endif
    }
    else if (msg->startSettings.format == NEXUS_MessageFormat_ePsi)
    {
        settings.data.atomSize = 0;
        settings.data.dataReadyThreshold = 0;
    }
    else {
        settings.data.atomSize = 256;
        settings.data.dataReadyThreshold = 2048;
        st->msg = msg;
    }
    st->isDss = msg->isDss;

    st->recpumpIndex = msg->settings.recpumpIndex;
    st->recpump = NEXUS_Recpump_Open(st->recpumpIndex, &settings);
    st->format = msg->startSettings.format;
    if (!st->recpump) {
        rc = BERR_TRACE(NEXUS_UNKNOWN);
        goto err_recpump_open;
    }
    if ( msg->settings.recpumpIndex == NEXUS_ANY_ID ) {
          NEXUS_RecpumpStatus recStatus;
          rc = NEXUS_Recpump_GetStatus( st->recpump, &recStatus);
          st->recpumpIndex = recStatus.rave.index;
          msg->settings.recpumpIndex = st->recpumpIndex;
      }


#if B_CALLBACK_HANDLER_SUPPORT
    NEXUS_CallbackHandler_Init(st->dataReadyCallbackHandler, NEXUS_SwFilter_P_DataReady, st);
#endif

    NEXUS_Recpump_GetSettings(st->recpump, &recpumpSettings);
#if B_USE_HW_PES_FILTERING
    recpumpSettings.outputTransportType = (st->format == NEXUS_MessageFormat_ePes) ? NEXUS_TransportType_eMpeg2Pes : NEXUS_TransportType_eTs;
#else
    recpumpSettings.outputTransportType = NEXUS_TransportType_eTs;
#endif
    recpumpSettings.data.overflow.callback = NULL;
    recpumpSettings.index.overflow.callback = NULL;
#if B_CALLBACK_HANDLER_SUPPORT
    NEXUS_CallbackHandler_PrepareCallback(st->dataReadyCallbackHandler, recpumpSettings.data.dataReady);
#else
    recpumpSettings.data.dataReady.callback = NEXUS_SwFilter_P_DataReadyWrapper;
    recpumpSettings.data.dataReady.context = st;
#endif
    rc = NEXUS_Recpump_SetSettings(st->recpump, &recpumpSettings);
    if (rc) {
        rc = BERR_TRACE(rc);
        goto err_recpump_setsettings;
    }

    if (msg->filter) {
        st->parseDataTimer = NEXUS_ScheduleTimer(SM_POLL_INTERVAL, NEXUS_SwFilter_P_PollTask, st);
    }

    st->refcnt = 1;
    BLST_S_INSERT_HEAD(&g_swfilter_state_list, st, link);

    return st;

err_recpump_setsettings:
    NEXUS_Recpump_Close(st->recpump);
err_recpump_open:
    NEXUS_SwFilter_Msg_P_Uninit(st->handle);
err_swfilter_init:
    BDBG_OBJECT_DESTROY(st, NEXUS_SwFilterCapture);
    BKNI_Free(st);
    return NULL;
}

static void NEXUS_SwFilter_P_Close(struct NEXUS_SwFilterCapture *st)
{
    BDBG_OBJECT_ASSERT(st, NEXUS_SwFilterCapture);
    if (--st->refcnt == 0) {
#if B_CALLBACK_HANDLER_SUPPORT
        NEXUS_CallbackHandler_Shutdown(st->dataReadyCallbackHandler);
#endif
        NEXUS_Recpump_Close(st->recpump);
        BLST_S_REMOVE(&g_swfilter_state_list, st, NEXUS_SwFilterCapture, link);

        if (st->parseDataTimer) {
            NEXUS_CancelTimer(st->parseDataTimer);
        }

        NEXUS_SwFilter_Msg_P_Uninit(st->handle);
        BDBG_OBJECT_DESTROY(st, NEXUS_SwFilterCapture);
        BKNI_Free(st);
    }
}

static struct NEXUS_SwFilterPid *NEXUS_SwFilter_P_OpenPid(NEXUS_MessageHandle msg)
{
    struct NEXUS_SwFilterPid *pid;
    int rc;
    NEXUS_P_HwPidChannel *hwPidChannel = msg->startSettings.pidChannel->hwPidChannel;

    for (pid = BLST_S_FIRST(&msg->stream->pids); pid; pid = BLST_S_NEXT(pid, link)) {
        BDBG_OBJECT_ASSERT(pid, NEXUS_SwFilterPid);
        if (pid->pidChannel->hwPidChannel->status.pidChannelIndex == hwPidChannel->status.pidChannelIndex) {
            pid->refcnt++;
            return pid;
        }
    }

    pid = BKNI_Malloc(sizeof(*pid));
    if (!pid) {
        BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    BKNI_Memset(pid, 0, sizeof(*pid));
    BDBG_OBJECT_SET(pid, NEXUS_SwFilterPid);
    pid->playpump = hwPidChannel->playpump;
    pid->pidChannel = NEXUS_PidChannel_P_Create(hwPidChannel);
    if (!pid->pidChannel) {
        goto err_openpidchannel;
    }

    rc = NEXUS_Recpump_AddPidChannel(msg->stream->recpump, pid->pidChannel, NULL);
    if (rc) {
        BERR_TRACE(rc);
        goto err_addpidchannel;
    }

    if (!BLST_S_FIRST(&msg->stream->pids)) {
        rc = NEXUS_Recpump_Start(msg->stream->recpump);
        if (rc) {
            BERR_TRACE(rc);
            goto err_start;
        }
    }
    pid->stream = msg->stream;
    pid->refcnt = 1;
    BLST_S_INSERT_HEAD(&msg->stream->pids, pid, link);
    return pid;

err_start:
    NEXUS_Recpump_RemovePidChannel(msg->stream->recpump, pid->pidChannel);
err_addpidchannel:
    if (!pid->playpump) {
        NEXUS_PidChannel_Close(pid->pidChannel);
    }
    else {
        NEXUS_Playpump_ClosePidChannel(pid->playpump, pid->pidChannel);
    }
err_openpidchannel:
    BDBG_OBJECT_DESTROY(pid, NEXUS_SwFilterPid);
    BKNI_Free(pid);
    return NULL;
}

static void NEXUS_SwFilter_P_ClosePid(struct NEXUS_SwFilterPid *pid)
{
    BDBG_OBJECT_ASSERT(pid, NEXUS_SwFilterPid);
    if (--pid->refcnt == 0) {
        BLST_S_REMOVE(&pid->stream->pids, pid, NEXUS_SwFilterPid, link);
        NEXUS_Recpump_RemovePidChannel(pid->stream->recpump, pid->pidChannel);
        if (!BLST_S_FIRST(&pid->stream->pids)) {
            NEXUS_Recpump_Stop(pid->stream->recpump);
        }
        if (!pid->playpump) {
            NEXUS_PidChannel_Close(pid->pidChannel);
        }
        else {
            NEXUS_Playpump_ClosePidChannel(pid->playpump, pid->pidChannel);
        }
        BDBG_OBJECT_DESTROY(pid, NEXUS_SwFilterPid);
        BKNI_Free(pid);
    }
    return;
}

static void *NEXUS_SwFilter_P_FilterCallback(void *context, size_t msg_size)
{
    NEXUS_MessageHandle msg = context;
    uint8_t last_4bytes[4]={0x55,0x55,0x55,0x55}; /* XPT HW pads with 0x55 */
    unsigned mod4=0;

    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(msg, NEXUS_Message);
    BDBG_ASSERT(msg->started);

    BDBG_MSG_TRACE(("FilterCallback(%p,%#x) size=%d", (void *)msg, msg->startSettings.pidChannel->hwPidChannel->status.pid, msg_size));

    /* For PSI/PES data, is size isn't a multiple of 4, pad it */
    /* first copy the %4 data. then copy the last 4 padded bytes */
    if (msg->filter) {
        mod4 = msg_size % 4;
        if (mod4) {
            unsigned i;
            BDBG_MSG_TRACE(("  padding %d bytes to align", (4-mod4)));
            for (i=0; i<mod4; i ++)
            {
                last_4bytes[i] = * (int8_t *)((uintptr_t)msg->tempBuffer + msg_size - mod4 + i);
            }
            msg_size -= (mod4); /* take just the multiple of 4 */
        }
        if (mod4) { /* copy last 4 padded bytes */
            BKNI_Memcpy((uint8_t *)(uintptr_t)msg->tempBuffer + msg_size, last_4bytes, 4);
            msg_size += 4;
        }
    }

#define CONTIGUOUS_SPACE(msg) ((msg)->wrapptr ? (msg)->rptr - (msg)->wptr : (msg)->bufferSize - (msg)->wptr)

    BDBG_MSG_TRACE(("add: %d to %d %d %d", msg_size, msg->rptr, msg->wptr, msg->wrapptr));

    /* memcpy to ringbuffer in contiguous block */
    if (msg_size > CONTIGUOUS_SPACE(msg) && !msg->wrapptr) {
        /* wrap and try again */
        msg->wrapptr = msg->wptr;
        msg->wptr = 0;
    }

    if (msg_size > CONTIGUOUS_SPACE(msg)) {
        goto overflow;
    }
    else {
        BKNI_Memcpy(&msg->buffer[msg->wptr], msg->tempBuffer, msg_size);
        msg->wptr += msg_size;
        BDBG_ASSERT(msg->wptr <= msg->bufferSize);
        BDBG_ASSERT(msg->wrapptr ? msg->wptr <= msg->rptr : msg->wptr > msg->rptr);
        if (msg->wptr == msg->bufferSize) {
            msg->wrapptr = msg->bufferSize;
            msg->wptr = 0;
        }
        NEXUS_TaskCallback_Fire(msg->dataReady);
    }

    return msg->tempBuffer; /* reuse buffer */

overflow:
/*    BDBG_MSG(("msg overflow" )); */
    NEXUS_TaskCallback_Fire(msg->overflow);
    return msg->tempBuffer; /* reuse buffer */
}

static void NEXUS_SwFilter_P_PollTask(void *context)
{
    struct NEXUS_SwFilterCapture *st = context;
    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(st, NEXUS_SwFilterCapture);
    BDBG_MSG(("PSI/PES timer %p", (void *)st));
    st->parseDataTimer = NULL;
    NEXUS_SwFilter_P_DataReady(st);
    st->parseDataTimer = NEXUS_ScheduleTimer(SM_POLL_INTERVAL, NEXUS_SwFilter_P_PollTask, st);
}

static void NEXUS_SwFilter_P_DataReady(void *context)
{
    NEXUS_Error rc;
    struct NEXUS_SwFilterCapture *st = context;

    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(st, NEXUS_SwFilterCapture);

    /* if the stream is bound to only one NEXUS_MessageHandle, it must be eTs */
    if (!st->msg) {
        uint8_t *buffer;
        size_t size;
        rc = NEXUS_Recpump_GetDataBuffer(st->recpump, (void *)&buffer, &size);
        if (!rc && size) {
            size_t consumed;
            if (st->format == NEXUS_MessageFormat_ePsi) {
                if ( st->isDss ) {
                    consumed = NEXUS_SwFilter_Msg_P_FeedDss(st->handle, buffer, size);
                }
                else
                {
                    consumed = NEXUS_SwFilter_Msg_P_Feed(st->handle, buffer, size);
                }
            }
            else {
                consumed = NEXUS_SwFilter_Msg_P_FeedPes(st->handle, buffer, size);
            }
            BDBG_MSG(("DataReady(recpump %d) consumed %d of %d", st->recpumpIndex, (unsigned)consumed, (unsigned)size));
            (void)NEXUS_Recpump_DataReadComplete(st->recpump, consumed);
        }
    }
    else {
        NEXUS_TaskCallback_Fire(st->msg->dataReady);
    }
    return;
}

NEXUS_Error NEXUS_Message_SetDssCapPattern( unsigned capFilterIndex, uint32_t pattern)
{
    BSTD_UNUSED(capFilterIndex);
    BSTD_UNUSED(pattern);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
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

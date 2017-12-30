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
 * Module Description:
 *
 **************************************************************************/

#ifndef NEXUS_SAGE_H__
#define NEXUS_SAGE_H__

#include "nexus_types.h"
#include "nexus_memory.h"

#include "nexus_sage_types.h"


#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************
private API
***************************************************************************/


typedef struct NEXUS_Sage *NEXUS_SageHandle;
typedef struct NEXUS_SageChannel *NEXUS_SageChannelHandle;

typedef struct NEXUS_SageOpenSettings
{
    NEXUS_CallbackDesc watchdogCallback;
} NEXUS_SageOpenSettings;
void NEXUS_Sage_GetDefaultOpenSettings(
    NEXUS_SageOpenSettings *pSettings /* [out] */
    );
NEXUS_SageHandle NEXUS_Sage_Open( /* attr{destructor=NEXUS_Sage_Close} */
    unsigned index,
    const NEXUS_SageOpenSettings *pSettings /* attr{null_allowed=y} */
    );
void NEXUS_Sage_Close(
    NEXUS_SageHandle sage
    );

typedef struct NEXUS_SageChannelSettings
{
    NEXUS_CallbackDesc successCallback;
    NEXUS_CallbackDesc errorCallback;
    NEXUS_CallbackDesc callbackRequestRecvCallback;
    NEXUS_CallbackDesc taTerminateCallback;
    NEXUS_CallbackDesc indicationCallback;
    NEXUS_HeapHandle heap;
} NEXUS_SageChannelSettings;
void NEXUS_SageChannel_GetDefaultSettings(
    NEXUS_SageChannelSettings *pSettings /* [out] */
    );
NEXUS_SageChannelHandle NEXUS_Sage_CreateChannel( /* attr{destructor=NEXUS_Sage_DestroyChannel} */
    NEXUS_SageHandle sage,
    const NEXUS_SageChannelSettings *pSettings
    );
void NEXUS_Sage_DestroyChannel(
    NEXUS_SageChannelHandle channel
    );
typedef struct NEXUS_SageChannelStatus
{
    uint32_t id;
    NEXUS_Error lastError;
    uint32_t lastErrorSage;
    bool busy;
    bool pendingCallbackRequest;
} NEXUS_SageChannelStatus;
NEXUS_Error NEXUS_SageChannel_GetStatus(
    NEXUS_SageChannelHandle channel,
    NEXUS_SageChannelStatus *pStatus /* [out] */
    );
typedef struct NEXUS_SageCommand {
    uint32_t systemCommandId;
    uint32_t platformId;
    uint32_t moduleId;
    uint32_t moduleCommandId;
    NEXUS_Addr payloadOffset;
} NEXUS_SageCommand;
NEXUS_Error NEXUS_SageChannel_SendCommand(
    NEXUS_SageChannelHandle channel,
    const NEXUS_SageCommand *pCommand
    );

typedef struct NEXUS_SageStatus {
    struct {
        bool secured;
    } urr;
    struct {
        uint32_t THLShortSig;
        uint8_t version[4];
    } framework;
} NEXUS_SageStatus;
/***************************************************************************
Summary:
Queries status information from Sage module.

Description:
This function is called by applications to retrieve specific status from
Sage module, currently used to determine Uncompressed Restricted Region
configuration setup when using Sage mode 5 (URR-Toggle).
This function checks that Sage has booted and consequently may take up to
5 seconds to complete.
***************************************************************************/
NEXUS_Error NEXUS_Sage_GetStatus(
    NEXUS_SageStatus *pStatus
    );
/***************************************************************************
Summary:

Description:
This function is called by applications to retrieve RSA2048 encrypted key
used to encrypt SAGE logs.
***************************************************************************/
NEXUS_Error NEXUS_Sage_GetEncKey(
    uint8_t *pKeyBuff,  /* [out] attr{nelem=keySize;nelem_out=pOutKeySize} pointer to CRC entries. */
    uint32_t keySize,
    uint32_t *pOutKeySize /* [out] size of buffer in bytes */
    );
/***************************************************************************
Summary:

Description:
This function is called by applications to retrieve AES128 encrypted SAGE
logs.
***************************************************************************/
NEXUS_Error NEXUS_Sage_GetLogBuffer(
    uint8_t *pBuff,  /* [out] attr{nelem=bufSize;nelem_out=pOutBufSize} pointer to CRC entries. */
    uint32_t bufSize,
    uint32_t *pOutBufSize, /* [out] size of buffer in bytes */
    uint32_t *pWrapBufSize, /* [out] size of buffer in bytes */
    uint32_t *pActualBufSize, /* [out] size of buffer in bytes */
    uint32_t *pActualWrapBufSize /* [out] size of buffer in bytes */
    );

typedef struct NEXUS_SageResponse {
    uint32_t sequenceId;
    uint32_t returnCode;
} NEXUS_SageResponse;
NEXUS_Error NEXUS_SageChannel_SendResponse(
    NEXUS_SageChannelHandle channel,
    const NEXUS_SageResponse *pResponse
    );

/***************************************************************************
Summary: Get the next indication for the given channel

Description:SAGE Indications are a couple of words (ID, value), that can
be sent from SAGE to the Host to indicate asynchronous actions/events etc
Number of indications could arrive simulaneously.

Returns NEXUS_SUCCESS, if successfully popped an indication
Returns NEXUS_NOT_AVAILABLE is there is no more indication to read
Any other returned value is an error
***************************************************************************/
NEXUS_Error NEXUS_SageChannel_GetNextIndication(
    NEXUS_SageChannelHandle channel,
    uint32_t *pId,
    uint32_t *pValue);

NEXUS_Error NEXUS_Sage_EnableHvd(void);
void NEXUS_Sage_DisableHvd(void);

/***************************************************************************
Summary:  the size of secure log buffer used in sage.

  the secure log buffer size is configed in secure log module init,
  when calling NEXUS_Sage_SecureLog_GetBuffer()
  must pass a buffer from GLR, size SAGE_SECURE_LOG_BUFFER_SIZE.
  GLRBufferSize must be SAGE_SECURE_LOG_BUFFER_SIZE
***************************************************************************/
#define SAGE_SECURE_LOG_BUFFER_SIZE  (128*1024)

/***************************************************************************
Summary:  the secure log buffer's ID

  there could be many secure log buffers in sage,
  brcm common buffer, or TA's private buffers.
  when calling NEXUS_Sage_SecureLog_GetBuffer()
  must from which buffer you want to get log.
  use NEXUS_Sage_SecureLog_BufferId_eFirstAvailable to get log from first buffer
***************************************************************************/
typedef enum {
    NEXUS_Sage_SecureLog_BufferId_eBRCMBuff,
    NEXUS_Sage_SecureLog_BufferId_eTARangeBuff1,
    NEXUS_Sage_SecureLog_BufferId_eTARangeBuff2,
    NEXUS_Sage_SecureLog_BufferId_eTARangeBuff3,
    NEXUS_Sage_SecureLog_BufferId_eTARangeBuff4,
    NEXUS_Sage_SecureLog_BufferId_eTARangeBuff5,
    NEXUS_Sage_SecureLog_BufferId_eTARangeBuff6,
    NEXUS_Sage_SecureLog_BufferId_eTARangeBuff7,
    NEXUS_Sage_SecureLog_BufferId_eTARangeBuff8,
    NEXUS_Sage_SecureLog_BufferId_eTARangeBuff9,
    NEXUS_Sage_SecureLog_BufferId_eTARangeBuff10,
    NEXUS_Sage_SecureLog_BufferId_eFirstAvailable,
    NEXUS_Sage_SecureLog_BufferId_eMax
}NEXUS_Sage_SecureLog_BufferId;

/***************************************************************************
Summary:  headr info of secure log buffer

  after NEXUS_Sage_SecureLog_GetBuffer() call,
  SecureLogBuffCtx point to struct NEXUS_Sage_Secure_Log_TlBufferContext.
  It contains info about log in logBuffer.
***************************************************************************/
typedef struct {
    uint8_t header_ver;
    uint8_t secure_log_type;
    uint8_t sdl_id;
    uint8_t root_key_sel;
    uint8_t opt_id_select;
    uint8_t global_owner_id;
    uint8_t rsvd0;
    uint8_t rsvd1;
    uint32_t secure_logbuf_len;
    uint32_t secure_logtotal_cnt;
}NEXUS_Sage_Secure_Log_TlSubH;

typedef struct {
    NEXUS_Sage_Secure_Log_TlSubH secHead;
    uint8_t EncLSEI[16];
    uint8_t KeyProcIn1[16];
    uint8_t KeyProcIn2[16];
    uint8_t KeyProcIn3[16];
}NEXUS_Sage_Secure_Log_TlBufferContext;

/***************************************************************************
Summary: Attach TA for secure log

Description: Attach TA for secure log. attached TA's log will go to secure log,
        secure log can read by NEXUS_Sage_SecureLog_GetBuffer()
        use 0 as TA_Id for SSF(sage framework)
Returns NEXUS_SUCCESS, if TA is attached.
***************************************************************************/
NEXUS_Error NEXUS_Sage_SecureLog_Attach(
    uint32_t TA_Id
    );

/***************************************************************************
Summary: Detach TA for secure log

Description: Detach TA for secure log. Detached TA's log will NOT go to secure log,
        secure log can read by NEXUS_Sage_SecureLog_GetBuffer()
        use 0 as TA_Id for SSF(sage framework)
Returns void
***************************************************************************/
void NEXUS_Sage_SecureLog_Detach(
    uint32_t TA_Id
    );

/***************************************************************************
Summary: Get encrypted log from secure log buffer.

Description:  Get encrypted log from secure log buffer.
        two buffers must be allcated from GLR and passed in.
  pSecureLogBuffCtx: size must be sizeof(Secure_Log_TlBufferContext),
        it has info of the securelog/GLR buffer on successful return.
  pLogBufferAddr: size must be SAGE_SECURE_LOG_BUFFER_SIZE,
        it contain encrypted log on successful return.
  bufferId: specify which buffer to get log from.
        check enum NEXUS_Sage_SecureLog_BufferId

   earlier log in SAGE will be cleared after the call,
   next NEXUS_Sage_SecureLog_GetBuffer() will return new log.
Returns NEXUS_SUCCESS, if encrypted log is read sucessfully.
***************************************************************************/
NEXUS_Error NEXUS_Sage_SecureLog_GetBuffer(
                        void       *pSecureLogBuffCtx,/* attr{memory=cached;null_allowed=y} */
                        uint32_t   logBuffCtxSize,
                        void       *pLogBufferAddr, /* attr{memory=cached;null_allowed=y} */
                        uint32_t   logBufferSize,
    NEXUS_Sage_SecureLog_BufferId  bufferId
    );

/***************************************************************************
Summary: Is it's fine to call NEXUS_Sage_SecureLog_GetBuffer() to get log.

Description:  check if secure log is enabled and fine to call
  NEXUS_Sage_SecureLog_GetBuffer() to get log.
  or NEXUS_Sage_SecureLog_GetBuffer() will fail to get logs.
  Calling this function will indicate to sage module that capture thread has started
  and will start capturing secure logs.
Returns NEXUS_SUCCESS if secure log is enabled and
  NEXUS_Sage_SecureLog_GetBuffer() will return log, else NEXUS_NOT_INITIALIZED.
***************************************************************************/
NEXUS_Error NEXUS_Sage_SecureLog_StartCaptureOK(void);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_SAGE_H__ */

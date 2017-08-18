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
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "bip_priv.h"
#include "bip_rtsp_response.h"

BDBG_MODULE( bip_rtsp_response );
BDBG_OBJECT_ID( BIP_RtspResponse );

typedef struct BIP_RtspResponse
{
    BDBG_OBJECT( BIP_RtspResponse )
    //BIP_RtspResponseSettings settings;
    BIP_RtspResponseStatus status;
    char *pBuffer; /* pointer to the actual message buffer string */
    BIP_RtspResponseCreateSettings createSettings;
} BIP_RtspResponse;

void
BIP_RtspResponse_GetDefaultCreateSettings(
    BIP_RtspResponseCreateSettings *pSettings
    )
{
    BKNI_Memset( pSettings, 0, sizeof(BIP_RtspResponseCreateSettings));
}

BIP_RtspResponseHandle
BIP_RtspResponse_Create(
    BIP_RtspResponseCreateSettings *pSettings
    )
{
    BIP_RtspResponseHandle         rtspResponse = NULL;
    BIP_RtspResponseCreateSettings defaultSettings;

    if (NULL == pSettings)
    {
        BIP_RtspResponse_GetDefaultCreateSettings( &defaultSettings );
        pSettings = &defaultSettings;
    }

    rtspResponse = BKNI_Malloc( sizeof(*rtspResponse));
    BIP_CHECK_PTR_GOTO( rtspResponse, "Memory Allocation Failed", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );

    BKNI_Memset( rtspResponse, 0, sizeof(*rtspResponse));
    BDBG_OBJECT_SET( rtspResponse, BIP_RtspResponse );
    //BKNI_Memset( &rtspResponse->settings, 0, sizeof(BIP_RtspResponseSettings) );
    rtspResponse->status = BIP_RtspResponseStatus_eInvalid;

    return(rtspResponse);

error:

    if (!rtspResponse)
        return (NULL);

    BDBG_OBJECT_DESTROY( rtspResponse, BIP_RtspResponse );
    BKNI_Free( rtspResponse );

    return(NULL);

} /* BIP_RtspResponse_Create */

void
BIP_RtspResponse_Destroy(
    BIP_RtspResponseHandle rtspResponse
    )
{
    BDBG_OBJECT_ASSERT( rtspResponse, BIP_RtspResponse );

    /* freeup any resources allocated part of the listener object */
    BDBG_OBJECT_DESTROY( rtspResponse, BIP_RtspResponse );

    /* Then finally free BIP_RtspResponse struct itself */
    BKNI_Free( rtspResponse );
    return;
}

void
BIP_RtspResponse_SetBuffer(
    BIP_RtspResponseHandle rtspResponse,
    char                  *pBuffer
    )
{
    BDBG_OBJECT_ASSERT( rtspResponse, BIP_RtspResponse );
    rtspResponse->pBuffer = pBuffer;
}

char *
BIP_RtspResponse_GetBuffer(
    BIP_RtspResponseHandle rtspResponse
    )
{
    BDBG_OBJECT_ASSERT( rtspResponse, BIP_RtspResponse );
    return(rtspResponse->pBuffer);
}

void
BIP_RtspResponse_SetStatus(
    BIP_RtspResponseHandle rtspResponse,
    BIP_RtspResponseStatus responseStatus
    )
{
    BDBG_OBJECT_ASSERT( rtspResponse, BIP_RtspResponse );
    rtspResponse->status = responseStatus;
    BDBG_MSG(("%s: rtpsResponse %p; status %x(%zu); size %zu", BSTD_FUNCTION, (void *)rtspResponse, rtspResponse->status, sizeof(rtspResponse->status),
              sizeof(*rtspResponse) ));
}

void
BIP_RtspResponse_GetStatus(
    BIP_RtspResponseHandle  rtspResponse,
    BIP_RtspResponseStatus *responseStatus
    )
{
    BDBG_OBJECT_ASSERT( rtspResponse, BIP_RtspResponse );
    *responseStatus = rtspResponse->status;
}

bool
BIP_RtspResponse_StatusValid(
    BIP_RtspResponseHandle rtspResponse
    )
{
    BDBG_OBJECT_ASSERT( rtspResponse, BIP_RtspResponse );
    if (rtspResponse->status == BIP_RtspResponseStatus_eInvalid)
    {
        return(false);
    }
    else
    {
        return(true);
    }
}

#if 0
void
BIP_RtspResponse_GetSettings(
    BIP_RtspResponseHandle    rtspResponse,
    BIP_RtspResponseSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT( rtspResponse, BIP_RtspResponse );
    BDBG_ASSERT( pSettings );
    *pSettings = rtspResponse->settings;
}

BIP_Status
BIP_RtspResponse_SetSettings(
    BIP_RtspResponseHandle    rtspResponse,
    BIP_RtspResponseSettings *pSettings
    )
{
    BIP_Status errCode;
    BIP_RtspLiveMediaListenerSettings lmListenerSettings;

    BDBG_OBJECT_ASSERT( rtspResponse, BIP_RtspResponse );
    BDBG_ASSERT( pSettings );
    /* TODO validate pSettings fields */
    rtspResponse->settings = *pSettings;

    errCode = BIP_SUCCESS;
    return(errCode);

error:
    return(errCode);
}

#endif /* if 0 */

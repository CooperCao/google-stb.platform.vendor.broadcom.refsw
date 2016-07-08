/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#include "bip_rtsp_server.h"
#include "bip_rtsp_socket.h"

BDBG_MODULE( bip_rtsp_request );
BDBG_OBJECT_ID( BIP_RtspRequest );

typedef struct BIP_RtspRequest
{
    BDBG_OBJECT( BIP_RtspRequest )
    BIP_RtspRequestCreateSettings createSettings;
    //BIP_RtspRequestSettings settings;
    BKNI_MutexHandle lock;
    char            *pBuffer; /* pointer to the actual message buffer string */
    void *pRtspSocket;
} BIP_RtspRequest;

void
BIP_RtspRequest_GetDefaultCreateSettings(
    BIP_RtspRequestCreateSettings *pSettings
    )
{
    BKNI_Memset( pSettings, 0, sizeof(BIP_RtspRequestCreateSettings));
}

BIP_RtspRequestHandle
BIP_RtspRequest_Create(
    BIP_RtspRequestCreateSettings *pSettings
    )
{
    BIP_RtspRequestHandle         hRtspRequest = NULL;
    BIP_RtspRequestCreateSettings defaultSettings;

    if (NULL == pSettings)
    {
        BIP_RtspRequest_GetDefaultCreateSettings( &defaultSettings );
        pSettings = &defaultSettings;
    }

    hRtspRequest = BKNI_Malloc( sizeof(*hRtspRequest));
    BIP_CHECK_PTR_GOTO( hRtspRequest, "Memory Allocation Failed", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );

    BKNI_Memset( hRtspRequest, 0, sizeof(*hRtspRequest));
    BDBG_OBJECT_SET( hRtspRequest, BIP_RtspRequest );
    //BKNI_Memset( &hRtspRequest->settings, 0, sizeof(BIP_RtspRequestSettings) );

#if 0
    /* TODO: check if we need to have a lock for this class */
    rc = BKNI_CreateMutex( &hRtspRequest->lock );
    BIP_CHECK_ERR_NZ_GOTO( rc, "BKNI_CreateMutex() Failed", error );
#endif
    return(hRtspRequest);

error:

    return(NULL);
} /* BIP_RtspRequest_Create */

void
BIP_RtspRequest_Destroy(
    BIP_RtspRequestHandle hRtspRequest
    )
{
    BDBG_OBJECT_ASSERT( hRtspRequest, BIP_RtspRequest );

    /* freeup any resources allocated part of the listener object */
    if (hRtspRequest->pBuffer)
    {
        BKNI_Free(hRtspRequest->pBuffer);
    }

    if (hRtspRequest->lock)
    {
        BKNI_DestroyMutex( hRtspRequest->lock );
    }

    BDBG_OBJECT_DESTROY( hRtspRequest, BIP_RtspRequest );

    /* Then finally free BIP_RtspRequest struct itself */
    BKNI_Free( hRtspRequest );
    return;
}

void
BIP_RtspRequest_SetBuffer(
    BIP_RtspRequestHandle hRtspRequest,
    char                 *pBuffer,
    unsigned              bufferLength
    )
{
    BDBG_OBJECT_ASSERT( hRtspRequest, BIP_RtspRequest );
    BSTD_UNUSED( bufferLength );
    if ( hRtspRequest->pBuffer ) BKNI_Free( hRtspRequest->pBuffer );
    hRtspRequest->pBuffer = pBuffer;
}

char *
BIP_RtspRequest_GetBuffer(
    BIP_RtspRequestHandle hRtspRequest
    )
{
    BDBG_OBJECT_ASSERT( hRtspRequest, BIP_RtspRequest );
    return(hRtspRequest->pBuffer);
}

void BIP_RtspRequest_SetSocket(
    BIP_RtspRequestHandle hRtspRequest,
    void *pRtspSocket
    )
{
    BDBG_OBJECT_ASSERT( hRtspRequest, BIP_RtspRequest );
    hRtspRequest->pRtspSocket = pRtspSocket;
}

void *BIP_RtspRequest_GetSocket(
    BIP_RtspRequestHandle hRtspRequest
    )
{
    BDBG_OBJECT_ASSERT( hRtspRequest, BIP_RtspRequest );
    return (hRtspRequest->pRtspSocket);
}

BIP_Status
BIP_RtspRequest_GetMethod(
    BIP_RtspRequestHandle  hRtspRequest,
    BIP_RtspRequestMethod *rtspRequestMethod
    )
{
    unsigned i;
    unsigned requestMessageSize = strlen( hRtspRequest->pBuffer );
    char     methodName[32];

    BKNI_Memset ( methodName, 0, sizeof(methodName) );

    BDBG_MSG(("%s: method (%s)", __FUNCTION__, &hRtspRequest->pBuffer[0] ));

    for (i = 0 ; i < sizeof(methodName)-1 && i < requestMessageSize ; i++)
    {
        char c = hRtspRequest->pBuffer[i];
        if ((c == ' ') || (c == '\t'))
        {
            /* found the method name */
            break;
        }
        methodName[i] = c;
    }
    methodName[i] = '\0';

    if (strncasecmp( methodName, "OPTIONS", sizeof("OPTIONS")) == 0)
    {
        *rtspRequestMethod = BIP_RtspRequestMethod_eOptions;
    }
    else if (strncasecmp( methodName, "SETUP", sizeof("SETUP")) == 0)
    {
        *rtspRequestMethod = BIP_RtspRequestMethod_eSetup;
    }
    else if (strncasecmp( methodName, "PLAY", sizeof("PLAY")) == 0)
    {
        if(strstr( hRtspRequest->pBuffer, "?" ))
        {
            *rtspRequestMethod = BIP_RtspRequestMethod_ePlayWithUrl;
        }
        else
        {
            *rtspRequestMethod = BIP_RtspRequestMethod_ePlay;
        }

    }
    else if (strncasecmp( methodName, "TEARDOWN", sizeof("TEARDOWN")) == 0)
    {
        *rtspRequestMethod = BIP_RtspRequestMethod_eTeardown;
    }
    else if (strncasecmp( methodName, "JOIN", sizeof("JOIN")) == 0)
    {
        *rtspRequestMethod = BIP_RtspRequestMethod_eJoin;
    }
    else if (strncasecmp( methodName, "GET_PARAMETER", sizeof("GET_PARAMETER")) == 0)
    {
        *rtspRequestMethod = BIP_RtspRequestMethod_eGetParameter;
    }
    else
    {
        *rtspRequestMethod = BIP_RtspRequestMethod_eMax;
    }
    BDBG_MSG(("%s: hRtspRequest[%p]: method %s, enum %d", __FUNCTION__, (void *)hRtspRequest, methodName, *rtspRequestMethod));

    return(BIP_SUCCESS);
} /* BIP_RtspRequest_GetMethod */

#if 0
void
BIP_RtspRequest_GetSettings(
    BIP_RtspRequestHandle    hRtspRequest,
    BIP_RtspRequestSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT( hRtspRequest, BIP_RtspRequest );
    BDBG_ASSERT( pSettings );
    *pSettings = hRtspRequest->settings;
}

BIP_Status
BIP_RtspRequest_SetSettings(
    BIP_RtspRequestHandle    hRtspRequest,
    BIP_RtspRequestSettings *pSettings
    )
{
    BIP_Status errCode;
    BIP_RtspLiveMediaListenerSettings lmListenerSettings;

    BDBG_OBJECT_ASSERT( hRtspRequest, BIP_RtspRequest );
    BDBG_ASSERT( pSettings );
    /* TODO validate pSettings fields */
    hRtspRequest->settings = *pSettings;

    errCode = BIP_SUCCESS;
    return(errCode);

error:
    return(errCode);
}
#endif /* if 0 */

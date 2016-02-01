/******************************************************************************
 * (c) 2007-2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#ifndef BIP_RTSP_REQUEST_H
#define BIP_RTSP_REQUEST_H

/* BIP Server APIs
 */


#ifdef __cplusplus
extern "C" {
#endif

typedef struct BIP_RtspRequest *BIP_RtspRequestHandle;

/**
 * RTSP Message Abstraction
 * Note:
 * TODO: need to further define/clarify these APIs
 **/
typedef struct BIP_RtspRequestCreateSettings
{
    unsigned unused;
} BIP_RtspRequestCreateSettings;

/**
 * Summary:
 * Get Default RtspSocket settings
 **/
void BIP_RtspRequest_GetDefaultCreateSettings( BIP_RtspRequestCreateSettings *pSettings );

/**
 * Summary:
 * API to create RTSP Request
 *
 * Description:
 *
 **/
BIP_RtspRequestHandle BIP_RtspRequest_Create( BIP_RtspRequestCreateSettings *pSettings );

/**
 * Summary:
 * API to set the message buffer in the RtspRequest Class
 **/
void BIP_RtspRequest_SetBuffer( BIP_RtspRequestHandle rtspRequest, char *pBuffer, unsigned bufferLength );
char *BIP_RtspRequest_GetBuffer( BIP_RtspRequestHandle rtspRequest );

void BIP_RtspRequest_SetSocket( BIP_RtspRequestHandle rtspRequest, void *pRtspSocket);
/*void BIP_RtspRequest_GetSocket( BIP_RtspRequestHandle rtspRequest, void *pRtspSocket);*/
void *BIP_RtspRequest_GetSocket( BIP_RtspRequestHandle rtspRequest);

/**
 * Summary:
 * APIs to retrieve and process RTSP Message fields and headers
 **/
typedef enum
{
    BIP_RtspRequestMethod_eOptions,
    BIP_RtspRequestMethod_eDescribe,
    BIP_RtspRequestMethod_eSetup,
    BIP_RtspRequestMethod_ePlay,
    BIP_RtspRequestMethod_ePlayWithUrl,
    BIP_RtspRequestMethod_eTeardown,
    BIP_RtspRequestMethod_eJoin,
    BIP_RtspRequestMethod_eSeek,
    BIP_RtspRequestMethod_eGetParameter, /* used by ip_client for keepalive */
    BIP_RtspRequestMethod_eMax
} BIP_RtspRequestMethod;

BIP_Status BIP_RtspRequest_GetMethod( BIP_RtspRequestHandle rtspRequest, BIP_RtspRequestMethod *requestMethod );
BIP_Status BIP_RtspRequest_GetUrl( BIP_RtspRequestHandle rtspRequest, const char **url );
/**
 * Summary:
 * Free up the RtspRequest
 **/
void BIP_RtspRequest_Destroy( BIP_RtspRequestHandle rtspRequest );

#ifdef __cplusplus
}
#endif

#endif /* BIP_RTSP_REQUEST_H */

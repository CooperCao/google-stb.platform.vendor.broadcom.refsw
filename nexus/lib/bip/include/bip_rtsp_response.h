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

#ifndef BIP_RTSP_RESPONSE_H
#define BIP_RTSP_RESPONSE_H

/* BIP Server APIs
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BIP_RtspResponse *BIP_RtspResponseHandle;

/**
 * RTSP Message Abstraction
 **/
typedef struct BIP_RtspResponseCreateSettings
{
    int unused;
} BIP_RtspResponseCreateSettings;

/**
 * Summary:
 * Get Default RtspSocket settings
 **/
void BIP_RtspResponse_GetDefaultCreateSettings( BIP_RtspResponseCreateSettings *pSettings );

/**
 * Summary:
 * API to create RTSP Message from a RTSP Socket (for incoming RTSP Requests) or from scratch (for outgoing RTSP Responses)
 *
 * Description:
 *
 * CreateFromSocket API will create RtspResponse from the pending RTSP Request message and parse various headers
 *
 * Create API will create RtspResponse from scratch and allow apps to add/append headers to the outgoing Response (or requests)
 **/
BIP_RtspResponseHandle BIP_RtspResponse_Create( BIP_RtspResponseCreateSettings *pSettings );

typedef enum
{
    BIP_RtspResponseStatus_eSuccess,
    BIP_RtspResponseStatus_eClientError, /* request contains bad syntax or cannot be fulfilled */
    BIP_RtspResponseStatus_eServerError, /* server failed to fulfill a valid request */
    BIP_RtspResponseStatus_eInvalid
} BIP_RtspResponseStatus;

/**
 * Summary:
 * Set Response Status
 **/
void BIP_RtspResponse_SetStatus( BIP_RtspResponseHandle rtspResponse, BIP_RtspResponseStatus responseStatus );

/**
 * Summary:
 * Get Response Status
 **/
void BIP_RtspResponse_GetStatus( BIP_RtspResponseHandle rtspResponse, BIP_RtspResponseStatus *responseStatus );

/**
 * Summary:
 * Check if status has been set to one of the valid codes
 **/
bool BIP_RtspResponse_StatusValid( BIP_RtspResponseHandle rtspResponse );

/**
 * Summary:
 * Set Response Buffer
 *
 * Description:
 * pBuffer is freed internally?
 **/
void BIP_RtspResponse_SetBuffer( BIP_RtspResponseHandle rtspResponse, char *pBuffer );

/**
 * Summary:
 * Free up the RtspResponse
 **/
void BIP_RtspResponse_Destroy( BIP_RtspResponseHandle rtspResponse );

#ifdef __cplusplus
}
#endif

#endif /* BIP_RTSP_RESPONSE_H */

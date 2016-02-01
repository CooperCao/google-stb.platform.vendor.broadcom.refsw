/***************************************************************************
*     (c)2008-2015 Broadcom Corporation
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
* Description: Broadcom IP (BIP) Player library API definition file
*
***************************************************************************/
#ifndef BIP_DLNA_H
#define BIP_DLNA_H

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup bip_dlna

BIP_Dlna Interface Definition.

**/

/* DLNA ORG Flags. */
#define DLNA_ORG_FLAGS_SENDER_PACED                     (1<<31)         /* Sender (Content Source) is pacing the stream. */
                                                                        /* If set, we can use it to setup live clock recovery mode. */

#define DLNA_ORG_FLAGS_LIMITED_OPERATIONS_TIME_SEEK     (1<<30)         /* May be set for Time based seek on the timeshifted buffer. */
#define DLNA_ORG_FLAGS_LIMITED_OPERATIONS_BYTE_SEEK     (1<<29)         /* May be set for Byte based seek on the timeshifted buffer. */
#define DLNA_ORG_FLAGS_PLAY_CONTAINER_PARAM             (1<<28)

#define DLNA_ORG_FLAGS_So_INCREASING                    (1<<27)         /* If set, timeshifting is enabled and starting position is changing. */
#define DLNA_ORG_FLAGS_Sn_INCREASING                    (1<<26)         /* If set, timeshifting is enabled and ending position is changing. */

#define DLNA_ORG_FLAGS_RTSP_PAUSE                       (1<<25)         /* RTP Server supports Pause operation. */
#define DLNA_ORG_FLAGS_STREAMING_MODE                   (1<<24)         /* Streaming mode. No real value for the Player! */
#define DLNA_ORG_FLAGS_INTERACTIVE_MODE                 (1<<23)         /* Interactive mode. No real value for the Player! */
#define DLNA_ORG_FLAGS_BACKGROUND_MODE                  (1<<22)         /* Background mode. No real value for the Player! */

#define DLNA_ORG_FLAGS_CONNECTION_STALLING              (1<<21)         /* Server supports Pause & Pause-Release using TCP Connection Stalling. */
                                                                        /* If this flag is set, then SENDER_PACED flag should be false. */

#define DLNA_ORG_FLAGS_DLNA_VERSIONING                  (1<<20)         /* N/A */
/* BIT 19 - 17 are not defined in the DLNA Spec. */

#define DLNA_ORG_FLAGS_LINK_PROTECTED_CONTENT           (1<<16)         /* Content is protected with DTCP/IP. */
                                                                        /* The Content-Type header contains the DTCP/IP AKE Server IP & Port information. */
#define DLNA_ORG_FLAGS_CLEAR_TEXT_FULL_DATA_SEEK        (1<<15)         /* N/A */
#define DLNA_ORG_FLAGS_CLEAR_TEXT_LIMITED_DATA_SEEK     (1<<14)         /* N/A */
/* Other BITS are not defined. */


/* DLNA OP Flags. */
#define DLNA_ORG_OP_A_VAL     1     /* Server supports Time based seeks. */
#define DLNA_ORG_OP_B_VAL     1     /* Server supports Byte based seeks. */

#define DLNA_CONTENT_FEATURES_HEADER_NAME           "ContentFeatures.DLNA.ORG:"
#define DLNA_AVAILABLE_SEEK_RANGE_HEADER_NAME       "AvailableSeekRange.DLNA.ORG:"
#define DLNA_ORG_OP_ELEMENT                         "DLNA.ORG_OP="
#define DLNA_ORG_FLAGS_ELEMENT                      "DLNA.ORG_FLAGS="
#define DLNA_ORG_PLAYSPEED_ELEMENT                  "DLNA.ORG_PS="
#define DLNA_DTCP_IP_CONTENT_TYPE_HEADER            "Content-Type: application/x-dtcp1"
#define DLNA_DTCP_IP_HOST_NAME                      ";DTCP1HOST="
#define DLNA_DTCP_IP_PORT_NAME                      ";DTCP1PORT="
#ifdef __cplusplus
}
#endif

#endif /* #ifndef BIP_DLNA_H */

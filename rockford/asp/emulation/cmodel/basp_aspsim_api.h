/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef __BASP_ASPSIM_API_H__
#define __BASP_ASPSIM_API_H__

#define BASP_MAX_NUMBER_OF_SUPPORTED_CHANNEL    32

#define BASP_P_FIFO_SIZE                  2048

typedef enum BASP_P_MessageType
{
    BASP_P_MessageType_PI2FW_eChannelStartStreamOut,
    BASP_P_MessageType_PI2FW_eChannelStartStreamIn,
    BASP_P_MessageType_PI2FW_eChannelStop,
    BASP_P_MessageType_FW2PI_eChannelStartStreamOutResponse,
    BASP_P_MessageType_FW2PI_eChannelStartStreamInResponse ,
    BASP_P_MessageType_FW2PI_eChannelStopResponse,
}BASP_P_MessageType;

/* The following structure is temporary, we will later replace it with fifo. This works only for a single message processing, no parallel context can be handled here.*/
typedef struct BASP_P_MessageFifo
{
    uint32_t size;
    uint32_t pi2FwMessage[ BASP_P_FIFO_SIZE ];
    bool pi2FwMessageAvailable;

    uint32_t fw2PiMessage[ BASP_P_FIFO_SIZE ];
    bool fw2PiMessageAvailable;
}BASP_P_MessageFifo;


/***************************************************************************
Summary:
            Describes the common message header.
****************************************************************************/
typedef struct BASP_P_MessageHeader
{
    BASP_P_MessageType  messageType;    /*!< Describes the message type. */
    uint32_t            messageCounter; /*!< This is unique for every message. */
    uint32_t            channelIndex;   /*!< This is the channel number, for init message this field has no meaning/useless */
} BASP_P_MessageHeader;

/* Simulated start message. Later this will be replaced with the actual start message.*/
typedef struct BASP_P_ChannelStartMessage
{
    SocketState sSocketState;
    char        *pXptMcpbBuffer; /*!< Later this will be replaced with raveBufferInfo as defined in actual StreamOutStart message.*/
} BASP_P_ChannelStartMessage;

/* Simulated stop message. Later this will be replaced with the actual stop message.*/
typedef struct BASP_P_ChannelStopMessage
{
    int unsused;
} BASP_P_ChannelStopMessage;

/* Simulated channelStartResponse. */
typedef struct BASP_P_ChannelStartResponse
{
    int        success;
} BASP_P_ChannelStartResponse;

/* Simulated channelStopResponse. */
typedef struct BASP_P_ChannelStopResponse
{
    int         success;
    SocketState sSocketState;
} BASP_P_ChannelStopResponse;

#endif /* __BASP_ASPSIM_API_H__ */

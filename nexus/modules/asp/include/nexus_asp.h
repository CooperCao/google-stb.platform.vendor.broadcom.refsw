/***************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef NEXUS_ASP_CHANNEL_H__
#define NEXUS_ASP_CHANNEL_H__

#include "nexus_asp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
private API
***************************************************************************/

/**
NEXUS AspChannel is a high level abstraction of the streaming portion of the ASP HW Block.

Advanced Stream Processor (ASP) is a H/W block that can send and/or receive AV data over TCP/HTTP/UDP/RTP protocols.
In addition, after the network protocol processing, ASP can stream-in incoming AV data to Playback HW or
stream-out AV data from Rave Context.

**/

/**
Summary:
API to Open an instance of AspChannel.
**/
void NEXUS_AspChannel_GetDefaultCreateSettings(
    NEXUS_AspChannelCreateSettings       *pSettings  /* [out] */
    );

NEXUS_AspChannelHandle NEXUS_AspChannel_Create(      /* attr{destructor=NEXUS_AspChannel_Destroy} */
    const NEXUS_AspChannelCreateSettings *pSettings  /* attr{null_allowed=y} */
    );


/**
Summary:
Stops and Destroys the AspChannel handle. The handle can no longer be used.
**/
void NEXUS_AspChannel_Destroy(
    NEXUS_AspChannelHandle hAspChannel
    );


/**
Summary:
Get Default StartSettings.
**/
void NEXUS_AspChannel_GetDefaultStartSettings(
    NEXUS_AspStreamingProtocol          streamingProtocol,
    NEXUS_AspChannelStartSettings       *pSettings   /* [out] */
    );


/**
Summary:
Start a AspChannel.
Note: it will only starts streaming if NEXUS_AspChannelStartSettings.autoStartStreaming flag is set to true!
**/
NEXUS_Error NEXUS_AspChannel_Start(
    NEXUS_AspChannelHandle              hAspChannel,
    const NEXUS_AspChannelStartSettings *pSettings
    );

/**
Summary:
Start data flow to Playpump or from Recpump for an AspChannel.

If NEXUS_AspChannelStartSettings.mode == NEXUS_AspChannelStreamingMode_eIn, data flows from
    Network -> NEXUS_AspCh (XPT ASP Ch) -> [NEXUS_Dma (XPT M2M DMA)] -> Playpump (XPT Playback Ch).

If NEXUS_AspChannelStartSettings.mode == NEXUS_AspChannelStreamingMode_eOut, data flows from
    NEXUS_Recpump (RAVE Ctx) ->  NEXUS_AspCh (XPT ASP Ch) -> Network.

**/
NEXUS_Error NEXUS_AspChannel_StartStreaming(
    NEXUS_AspChannelHandle              hAspChannel
    );


/**
Summary:
Stop Streaming flow for an AspChannel.

ASP FW will stop feeding AV Stream into XPT Playback or outof XPT Rave pipe.
**/
void NEXUS_AspChannel_StopStreaming(
    NEXUS_AspChannelHandle              hAspChannel
    );


/**
Summary:
Stop a AspChannel.

Description:
This API sends the Abort message to FW to immediately Stop NEXUS_AspChannel.
FW will NOT synchronize the protocol state (such as for TCP) in this case.
For that, caller should use NEXUS_AspChannel_Finish()
**/
void NEXUS_AspChannel_Stop(
    NEXUS_AspChannelHandle              hAspChannel
    );


/**
Summary:
Finish a AspChannel.

Description:
This API initiates the normal finishing of ASP Channel.
FW will synchronize the protocol state (such as for TCP) in this case.
This may involve waiting for TCP ACKs for any pending data and thus may take time.
However, the API returns after sending the message to the FW.
Its completion will be notified via the stateChangedCallback.

For immediate aborting, caller should use NEXUS_AspChannel_Stop().

**/
void NEXUS_AspChannel_Finish(
    NEXUS_AspChannelHandle              hAspChannel
    );


/**
Summary:
API to Get Current Settings.
**/
void NEXUS_AspChannel_GetSettings(
    NEXUS_AspChannelHandle              hAspChannel,
    NEXUS_AspChannelSettings            *pSettings   /* [out] */
    );


/**
Summary:
API to Update Current Settings.
**/
NEXUS_Error NEXUS_AspChannel_SetSettings(
    NEXUS_AspChannelHandle              hAspChannel,
    const NEXUS_AspChannelSettings      *pSettings
    );


/**
Summary:
API to Get Current Status.
**/
NEXUS_Error NEXUS_AspChannel_GetStatus(
    NEXUS_AspChannelHandle              hAspChannel,
    NEXUS_AspChannelStatus              *pStatus     /* [out] */
    );

/**
Summary:
API to Get Current DTCP-IP Settings
**/
void NEXUS_AspChannel_GetDtcpIpSettings(
    NEXUS_AspChannelHandle              hAspChannel,
    NEXUS_AspChannelDtcpIpSettings      *pSettings /* [out] */
    );

/**
Summary:
API to Set Current DTCP-IP Settings.  This must be called prior to NEXUS_AspChannel_Start().
**/
NEXUS_Error NEXUS_AspChannel_SetDtcpIpSettings(
    NEXUS_AspChannelHandle              hAspChannel,
    const NEXUS_AspChannelDtcpIpSettings *pSettings
    );

/**
Summary:
API to provide data to the firmware so that it can transmit it out on the network.

Note: one usage of this API is to allow caller to send HTTP Request or HTTP Response to the remote.
**/
NEXUS_Error NEXUS_AspChannel_GetWriteBufferWithWrap(
    NEXUS_AspChannelHandle              hAspChannel,
    void                                **pBuffer,   /* [out] attr{memory=cached} pointer to data buffer which can be written to network. */
    unsigned                            *pAmount,    /* [out] size of the available space in the pBuffer before the wrap. */
    void                                **pBuffer2,  /* [out] attr{null_allowed=y;memory=cached} optional pointer to data after wrap. */
    unsigned                            *pAmount2    /* [out] size of the available space in the pBuffer2 that can be written to network. */
    );


NEXUS_Error NEXUS_AspChannel_WriteComplete(
    NEXUS_AspChannelHandle              hAspChannel,
    unsigned                            amount       /* number of bytes written to the buffer. */
    );


/**
Summary:
API to receive network data (from firmware) for host access.

Note: one usage of this API is to allow caller to receive HTTP Response from the remote.
**/
NEXUS_Error NEXUS_AspChannel_GetReadBufferWithWrap(
    NEXUS_AspChannelHandle              hAspChannel,
    const void                          **pBuffer,   /* [out] attr{memory=cached} pointer to buffer containing data read from network. */
    unsigned                            *pAmount,    /* [out] number of bytes available in the data buffer pBuffer. */
    const void                          **pBuffer2,  /* [out] attr{null_allowed=y;memory=cached} pointer to buffer after wrap containing data read from network. */
    unsigned                            *pAmount2    /* [out] number of bytes available in the data buffer pBuffer2. */
    );

NEXUS_Error NEXUS_AspChannel_ReadComplete(
    NEXUS_AspChannelHandle              hAspChannel,
    unsigned                            bytesRead    /* number of bytes read/consumed by the caller. */
    );


/**
Summary:
API to directly write Reassembled IP Packets to ASP FW for processing.

Host will receive the reassembled IP Packets via the OS Network Stack and then feed to the ASP FW via this API.
**/
NEXUS_Error NEXUS_AspChannel_GetReassembledWriteBufferWithWrap(
    NEXUS_AspChannelHandle              hAspChannel,
    void                                **pBuffer,   /* [out] attr{memory=cached} pointer to data buffer which can be written to network. */
    unsigned                            *pAmount,    /* [out] size of the available space in the pBuffer before the wrap. */
    void                                **pBuffer2,  /* [out] attr{null_allowed=y;memory=cached} optional pointer to data after wrap which can be consumed */
    unsigned                            *pAmount2    /* [out] size of the available space in the pBuffer2 that can be written to network. */
    );

NEXUS_Error NEXUS_AspChannel_ReassembledWriteComplete(
    NEXUS_AspChannelHandle              hAspChannel,
    unsigned                            amount       /* number of bytes written to the buffer. */
    );


#ifdef __cplusplus
}
#endif

#endif

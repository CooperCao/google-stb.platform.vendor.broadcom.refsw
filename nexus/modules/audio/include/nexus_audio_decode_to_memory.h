/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
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
* API Description:
*   Interfaces for decoding audio into memory
*
***************************************************************************/
#include "nexus_audio_decoder.h"

#ifndef NEXUS_AUDIO_DECODE_TO_MEMORY_H__
#define NEXUS_AUDIO_DECODE_TO_MEMORY_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Get DecodeToMemory settings
***************************************************************************/
void NEXUS_AudioDecoder_GetDecodeToMemorySettings(
    NEXUS_AudioDecoderHandle decoder,
    NEXUS_AudioDecoderDecodeToMemorySettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Set DecodeToMemory settings

Description:
Set the decode to Memory settings for a decoder.  This can only be changed
while the decoder is stopped and any created buffers have been destroyed.
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_SetDecodeToMemorySettings(
    NEXUS_AudioDecoderHandle decoder,
    const NEXUS_AudioDecoderDecodeToMemorySettings *pSettings
    );

/***************************************************************************
Summary:
Get DecodeToMemory status
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_GetDecodeToMemoryStatus(
    NEXUS_AudioDecoderHandle decoder,
    NEXUS_AudioDecoderDecodeToMemoryStatus *pStatus /* [out] */
    );

/***************************************************************************
Summary:
Queue a Memory buffer for the decoder to write into

Description:
This provides a buffer for the audio decoder to write decoded data into.
The decoder must be started prior to calling this routine.
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_QueueBuffer(
    NEXUS_AudioDecoderHandle decoder,
    NEXUS_MemoryBlockHandle buffer,
    unsigned bufferOffset,
    size_t bufferLength
    );

/***************************************************************************
Summary:
Get decoded frames

Description:
This will return the completed frames from the audio decoder.  This is non-
destructive and the app must call NEXUS_AudioDecoder_ConsumeDecodedFrames
to remove them from the queue.
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_GetDecodedFrames(
    NEXUS_AudioDecoderHandle decoder,
    NEXUS_MemoryBlockHandle *pMemoryBlocks,       /* attr{nelem=numEntries;nelem_out=pNumEntriesReturned} [out] */
    NEXUS_AudioDecoderFrameStatus *pFrameStatus,  /* attr{nelem=numEntries;nelem_out=pNumEntriesReturned} [out] */
    unsigned numEntries,
    unsigned *pNumEntriesReturned /* [out] */
    );

/***************************************************************************
Summary:
Consume buffers returned from NEXUS_AudioDecoder_GetBuffers

Description:
This removes one or more completed buffers from the decoder's queue and
transfers ownership to the application.
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_ConsumeDecodedFrames(
    NEXUS_AudioDecoderHandle decoder,
    size_t numBuffers
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_AUDIO_DECODE_TO_MEMORY_H__ */

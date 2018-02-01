/******************************************************************************
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
 ******************************************************************************/

#ifndef SARM_COMMON_H__
#define SARM_COMMON_H__

#define _MAX_SARM_AUDIO_STREAMS     3
#define SAGE_AUDIO_RAVE_ALIGN 4096

typedef enum
{
    SARM_SageAudioState_eNone = 0, /* No Sync WORD Found */
    SARM_SageAudioState_eInit,     /* Stream is started or stream lost sync */
    SARM_SageAudioState_eSync,     /* Sync WORD found with byte by byte search */
    SARM_SageAudioState_eSteady,   /* Sync WORD found with frame size */
    SARM_SageAudioState_eStop,     /* Stream is stopped */
    SARM_SageAudioState_eMax       /* Validity check */
} SARM_SageAudioState_e;

typedef enum
{
    SARM_AudioCodec_eUnknown = 0,  /* unknown/not supported audio format */
    SARM_AudioCodec_eMpeg,         /* MPEG1/2, layer 1/2. This does not support layer 3 (mp3). */
    SARM_AudioCodec_eMp3,          /* MPEG1/2, layer 3. */
    SARM_AudioCodec_eAc3,          /* Dolby Digital AC3 audio */
    SARM_AudioCodec_eAc3Plus,      /* Dolby Digital Plus (AC3+ or DDP) audio */
    SARM_AudioCodec_eAacAdts,      /* Advanced audio coding with ADTS (Audio Data Transport Format) sync */
    SARM_AudioCodec_eAacLoas,      /* Advanced audio coding with LOAS (Low Overhead Audio Stream) sync and LATM mux */
    SARM_AudioCodec_eAacPlusAdts,  /* Advanced audio coding with ADTS (Audio Data Transport Format) sync */
    SARM_AudioCodec_eAacPlusLoas,  /* Advanced audio coding with LOAS (Low Overhead Audio Stream) sync and LATM mux */
    SARM_AudioCodec_eVorbis,       /* Vorbis audio codec.  Typically used with OGG or WebM container formats. */
    SARM_AudioCodec_eOpus,         /* Opus Audio Codec */
    SARM_AudioCodec_ePcm,          /* PCM audio - Generally used only with inputs such as SPDIF or HDMI. */
    SARM_AudioCodec_ePcmWav,       /* PCM audio with Wave header - Used with streams containing PCM audio */
    SARM_AudioCodec_eMax
}SARM_AudioCodec_e;

typedef struct
{
    SARM_SageAudioState_e state; /* SARM TA State for specified stream */
    uint32_t itbBytes;         /* Total Bytes processed for ITB of this stream */
    uint32_t itbSrcWrap;       /* Wrap flip stats */
    uint32_t cdbBytes;         /* Total Bytes processed for CDB of this stream */
    uint32_t numFrames;        /* Number of frames successfully copied to destination CDB */
    uint32_t zeroBytes;        /* Total bytes discarded (copied as zeros) */
    uint32_t lastSyncBytes;    /* Bytes since last sync WORD was found */
    uint32_t lostSyncCount;    /* Number of times Sync frame was lost */
    uint32_t noSpaceCount;     /* No space in destination bump */
    uint32_t congestion;       /* Destination is not draining, congestion */
    uint32_t cdbSrcWrap;       /* Wrap flip stats */
    uint32_t zeroWrap;         /* count for when wrap pointer is zero */
    uint32_t zeroReg;          /* Read register value to be zero */
    uint32_t noFullFrame;      /* No processing due to lack of complete frame */
    uint32_t frameTooLarge;    /* Unsupported frame size detected */
    uint32_t reserved1;        /* Used for temporary development data */
    uint32_t reserved2;        /* Used for temporary development data */
}SARM_SageAudioStatus;

#endif /* #ifndef SARM_COMMON_H__ */

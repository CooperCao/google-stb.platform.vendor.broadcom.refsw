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
#ifndef __NEXUS_GCB_SW_PRIV_H__
#define __NEXUS_GCB_SW_PRIV_H__

#include "nexus_parser_band_channelbonding.h"
#include "priv/nexus_transport_priv.h"
#include "bfifo.h"

#define ITB_WORD0_GET_TYPE(val) (val >> 24)
#define ITB84_WORD4_GET_NUM_BONDED_CH(val) (val >> 24)
#define ITB84_WORD4_GET_CH_NUM(val)        ((val >> 16) & 0xFF)
#define ITB84_WORD4_GET_BUNDLE_SIZE(val)   (val & 0xFFFF)

#define GET_40BIT_OFFSET(itb2, itb3) ( (((uint64_t)(itb2) >> 24) << 32) | ((uint64_t)(itb3)) )

#define ITB_WORDS 6
#define ITB_PAIR_SIZE ITB_WORDS*4*2 /* number of bytes for pair of ITB entries (0x84 and 0x85) */

#define MAX_PARSERS 6
#define MAX_PID_CHANNELS 8

#define SUPPORT_40BIT_OFFSETS 1

#if (!SUPPORT_40BIT_OFFSETS)
typedef uint32_t uintoff_t;
#else
typedef uint64_t uintoff_t;
#endif

typedef enum sstate {
    sstate_findInit = 0,
    sstate_crossInit,
    sstate_crossPriBand,
    sstate_lockMode,
    sstate_reacq,
    sstate_restart
} sstate;

typedef struct {
    uint32_t seqNum, size;
    uintoff_t offset;
} GcbChunk;

struct NEXUS_GcbSw
{
    unsigned index;
    unsigned bondingPid;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_TimerHandle timer, checkTimer;
    NEXUS_ParserBandStartBondingGroupSettings startSettings;

    struct {
        unsigned parserBandIndex;
        NEXUS_RecpumpHandle recpump;
        NEXUS_PidChannelHandle bipPidChannel;
        NEXUS_PidChannelHandle pidChannelList[MAX_PID_CHANNELS];

        uint8_t *bufferBase;
        size_t bufferSize;
        uintoff_t offsetAccum;

        int seqNum; /* the first seqNum available */

        #define CHUNK_FIFO_SIZE 128
        GcbChunk chunks[CHUNK_FIFO_SIZE];
        BFIFO_HEAD(ChunkFifo, GcbChunk) chunkFifo;
        unsigned submitSize; /* number of bytes last submitted */
#if 0 /* for debug */
        FILE* file;
#endif
    } parsers[MAX_PARSERS]; /* this represents a single unit of virtualized HW */
    unsigned numParsers;

    struct {
        int seqThreshold;
        unsigned seqThresholdParser, primaryParser; /* these are the index of parsers[], not the NEXUS_ParserBand_e number */
        sstate state;
        bool reacquire;
        #if 0 /* unused */
        int seqThresholdWrap;
        bool seqNumWrap;
        #endif

        unsigned order[MAX_PARSERS]; /* execution order when submitting descriptors */

        #define MAX_DESC_LEN (CHUNK_FIFO_SIZE*MAX_PARSERS)
        NEXUS_PlaypumpScatterGatherDescriptor desc[MAX_DESC_LEN+MAX_PARSERS]; /* +MAX_PARSERS since at wraparound, each band needs one extra descriptor */
        unsigned numDesc;
        size_t totalBytes;
    } state;

    struct {
        NEXUS_Timebase timebaseFreerun;
    } pacing;
};

typedef struct NEXUS_GcbSw* NEXUS_GcbSwHandle;

#endif

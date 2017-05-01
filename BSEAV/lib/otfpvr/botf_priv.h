/***************************************************************************
 *  Copyright (C) 2007-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *      OTF private header file
 *
 **************************************************************************/

#ifndef BOTF_PRIV_H_
#define BOTF_PRIV_H_

#include "botf_mem.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/**
 * @file
 */

/*****************************************************************************
* BOTF_BufferPtrs
*****************************************************************************/
typedef struct BOTF_ParserPtrs
{
    botf_mem_t     mem; /* interface to convert between virtual and physicall addresses */
    const uint8_t *CdbStartPtr;           /* CDB Start pointer */
    const uint8_t *CdbReadPtr;            /* Current CDB read pointer */
    const uint8_t *CdbValidPtr;           /* Current CDB write pointer */
    const uint8_t *CdbEndPtr;             /* Current CDB End pointer */
    const uint8_t *CdbWrapAroundPtr;      /* Last CDB wraparound pointer */
    uint8_t *ItbStartPtr;                 /* ITB Start pointer */
    const uint8_t *ItbReadPtr;            /* Current ITB Read pointer */
    uint8_t *ItbValidPtr;                 /* Current ITB Write pointer */
    const uint8_t *ItbEndPtr;             /* Current ITB End pointer */
    const uint8_t *ItbWrapAroundPtr;      /* Last ITB wraparound pointer */
} BOTF_ParserPtrs;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#include "botf_scv_parser.h"
#include "botf_gop_manager.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/*****************************************************************************
* BOTF_FeederState
*****************************************************************************/
typedef enum  BOTF_FeederState {
        BOTF_FeederState_eRunning,
        BOTF_FeederState_ePaused, /* no parsing */
        BOTF_FeederState_eStopped,
        BOTF_FeederState_eDraining,
        BOTF_FeederState_eVbvInvalid
} BOTF_FeederState;

/*****************************************************************************
* BOTF_PlayPumpState
*****************************************************************************/
typedef struct BOTF_PlayPumpState
{
        bool                  OtfParser;   /* true if we parse MPEG stream and use secondary feed for decoder */
        BOTF_FeederMode       FeederMode;
        bpvr_gop_player_mode  PlayerMode;
} BOTF_PlayPumpState;

/*****************************************************************************
* BOTF_ParserState
*****************************************************************************/
typedef struct BOTF_ParserState
{
        BOTF_FeederState        State;
        BOTF_PlayPumpState		FeederState; /* current state of the feeder */
        const uint8_t           *ParserScvReadPtr;        
} BOTF_ParserState;

BDBG_OBJECT_ID_DECLARE(BOTF);


typedef struct BOTF_Data
{
    BREG_Handle              hBReg;
    BMMA_Heap_Handle         mma;
    BOTF_FeederState         State;

    botf_scv_parser          ScvParser;
    bpvr_gop_manager         GopManager;
    bpvr_gop_player          GopPlayer;    
    BOTF_ParserState         ParserState;
    unsigned int             DataEndTime;
    unsigned int             DecoderStuckTime;
    bool		             bOtfStop;
    bool                     bOtfTaskDone;
    bool					 bStarted;
    bool                     bUsePtsAsTag;
    BOTF_Config		         Settings;

    uint32_t                 InputParserCDBSize;  /* CDB size for input parser */
    uint32_t                 InputParserITBSize;  /* ITB size for input parser */
    uint32_t                 OPParserITBSize;     /* ITB size for output parser */
    uint32_t                 OPParserITBAlign;    /* ITB alignment in bits for output parser */
    BAVC_XptContextMap       IpParserRegMap;      /* Input Parser register map */
    BAVC_XptContextMap       OpParserRegMap;      /* Output Parser register map */
    BOTF_ParserPtrs          IPParserPtrs;        /* Local copy of latest IP parser buffer pointers*/
    BOTF_ParserPtrs          OPParserPtrs;        /* Local copy of latest OP parser buffer pointers*/    
    struct {
        uint32_t CdbStart; 
        uint32_t ItbStart;
    } OpParserRegSave;     /* Local copy of OP Parser pointers when OTF was opened */
    BOTF_VideoDecoderStatus  prevVideoDecoderStatus;
    BKNI_EventHandle         DataEndEvent;
    botf_mem    mem;
    BMMA_Block_Handle itbMem;
    void *itb_buffer;

    BDBG_OBJECT(BOTF)
} BOTF_Data ;


void BOTF_P_FrameCountCallBack(void *hOtf, uint32_t chunk_no, uint32_t frame_cnt);
void BOTF_P_DetectedEof(void *hOtf);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BOTF_PRIV_H_ */


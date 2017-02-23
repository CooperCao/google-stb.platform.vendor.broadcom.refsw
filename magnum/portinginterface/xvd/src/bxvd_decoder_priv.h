/***************************************************************************
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
 *
 * [File Description:]
 *
 ***************************************************************************/

#ifndef BXVD_DECODER_PRIV_H_
#define BXVD_DECODER_PRIV_H_

#include "bxvd.h"
#include "bxvd_decoder_timer.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Define the following to allow frames to be captured */
/* #define BXVD_DM_ENABLE_YUV_GRAB_MODE 1  */

/*
 * type definitions
 */

#define BXVD_DECODER_DQT_DEADLOCK_THRESHOLD     4

/* The maximum number of fields that the decoder will request AVD to drop.
 */
#define BXVD_DECODER_MAX_FIELDS_TO_DROP   16


/* Support for processing sets of picture.  Stereo/3D is an
 * example of when multiple pictures would be processed as a set.
 */
typedef enum BXVD_Decoder_P_PictureSetType
{
   BXVD_Decoder_P_PictureSet_eSingle = 0,
   BXVD_Decoder_P_PictureSet_eBase,
   BXVD_Decoder_P_PictureSet_eDependent,

   BXVD_Decoder_P_PictureSet_eMax

} BXVD_Decoder_P_PictureSetType;

/* SW7445-3153: with the refactoring of the queue logic, the Unified Picture
 * queue is now smaller than the delivery queue. The size is of the Unified
 * Picture queue is specified by BXVD_P_DECODER_PICTURE_QUEUE_DEPTH.
 * The minimum size is 4; 1 picture on the display, 1 picture at the head
 * of the queue, times 2 for multi picture protocols; MVC,
 * HEVC interlaced, H264 full frame 3D.
 */
#define BXVD_P_DECODER_PICTURE_QUEUE_DEPTH 8

typedef struct BXVD_Decoder_P_PictureContext
{
   /* Support for processing sets of pictures. Currently sets are only of
    * size "2", hence "uiSetCount" is not used.  It will be needed to
    * support larger multiview sets.
    */
   BXVD_Decoder_P_PictureSetType    eSetType;
   uint32_t                         uiSetCount;

   /* SW7425-2686: intra-GOP picture index, the picture number from
    * the beginning of the GOP. '1' based. */
   uint32_t uiIntraGOPIndex;

   /* Drop if not a complete set of pictures (not TSM related) */
   bool                       bDropPicture;

   /* To aid in debug of DQT and multi picture protocols. */
   uint32_t index;

   /* SW7422-72: for 3D support dereferenced some of the PPB data
    * prior to calling BXVD_Decoder_S_UnifiedQ_ValidatePicture
    * TODO: redo logic only parse this information once.
    */
   BXDM_Picture_3D st3D;

   /* TODO: does this need to be a display element or
    * could it just be a BXVD_P_PPB *?
    */
   BXVD_P_DisplayElement stPPB;

   /* If a dependent picture, will point to the associated base picture. */
   struct BXVD_Decoder_P_PictureContext * pBasePicture;

   /* Points to the next dependent picture in the list.  NULL if the last one. */
   struct BXVD_Decoder_P_PictureContext * pDependentPicture;

} BXVD_Decoder_P_PictureContext;

typedef struct BXVD_Decoder_P_PictureContextQueue
{
   uint32_t uiWriteOffset;

   /* FWAVD-289: Use the queue index to detect when a picture is being evaluated for the first time. */
   uint32_t uiPreviousWriteOffset;

   /* SW7425-2686: intra-GOP picture index, running count of the pictures from
    * the beginning of the GOP. '1' based. */
   uint32_t uiIntraGOPIndex;

   /* SW7445-3153:  only used in BXVD_Decoder_S_UnifiedQ_Update_isr, track these values here to aid in debug. */
   uint32_t uiDeliveryQWriteOffset0Based;
   uint32_t uiDeliveryQReadOffset0Based;

   /* SWSTB-788: calculate the number of pictures on the delivery queue.*/
   uint32_t uiPictureCountDeliveryQueue;

   BXVD_Decoder_P_PictureContext    astPictureContext[ BXVD_P_MAX_ELEMENTS_IN_DISPLAY_QUEUE ];

} BXVD_Decoder_P_PictureContextQueue;

/*
 * Unified picture queue types.
 */

typedef struct BXVD_Decoder_P_UnifiedPictureContext
{
   bool  bInUse;

   uint32_t pPPBPhysical;
   BXVD_P_PPB * pPPB;

   BXVD_Decoder_P_PictureSetType    eSetType;
   uint32_t                         uiSetCount;
   uint32_t                         uiDropCount;   /* the AVD drop count at the time this picture was received. */

   /* for debug */
   uint32_t uiIndex;

   BXDM_Picture stUnifiedPicture;

   /* For implementing the Unified Queue as a linked list. */
   struct BXVD_Decoder_P_UnifiedPictureContext * pstPrevious;
   struct BXVD_Decoder_P_UnifiedPictureContext * pstNext;

   /* For managing sets of pictures; e.g. MVC. */
   struct BXVD_Decoder_P_UnifiedPictureContext * pstDependent;

} BXVD_Decoder_P_UnifiedPictureContext;


typedef struct BXVD_Decoder_P_UnifiedPictureQueue
{
   uint32_t uiReadOffset; /* Always the picture under evaluation */
   uint32_t uiWriteOffset; /* if uiWriteOffset == uiReadOffset, queue is empty */

   uint32_t uiSearchIndex;

   int32_t  iNumPictures;

   int32_t  iNumUsedElements;

   BXVD_Decoder_P_UnifiedPictureContext * pstHead;
   BXVD_Decoder_P_UnifiedPictureContext * pstTail;

   BXVD_Decoder_P_UnifiedPictureContext astUnifiedContext[ BXVD_P_DECODER_PICTURE_QUEUE_DEPTH ];

} BXVD_Decoder_P_UnifiedPictureQueue;


/*
 * Parameters to support "Display Queue Trick Mode"
 */
typedef struct BXVD_Decoder_P_DQTContext
{
   bool  bDqtEnabled;   /* enables the reverse playback of GOPs */

   bool  bConvertGopToUniPics;   /* indicates in the middle of playing a GOP backwards */

   uint32_t  uiReverseReadOffset;    /* read pointer when walking backwards through the display queue */

   uint32_t  uiEndOfGopOffset;   /* end of the current GOP, used to updated the display queue read pointer */

   /*
    * Variables for dectecting a deadlock condition and dealing with the subsequent clean up.
    */
   /*uint32_t  uiPreviousWrOffset;*/                 /* delivery queue write pointer from the previous vsync */
   uint32_t  uiTimesOffsetRepeated;        /* how many vsync's the write pointer has remained the same */
   uint32_t    uiDeadlockThreshold;        /* number of vsync's to wait for a new picture in the delivery queue */

   bool    bTruncatingGop;           /* truncating the current GOP  */
   uint32_t    uiCleanupOffset;    /* delivery queue offset for releasing the pictures at the end of a GOP */

   bool        bValidPicTag;               /* indicates if "uiCurrentPicTag" is valid */
   uint32_t    uiCurrentPicTag;        /* picture tag of the current GOP */


   /* SW7445-3153: only used in BXVD_Decoder_S_UnifiedQ_Update_isr, track these values here to aid in debug. */
   bool  bSearchForStartOfNextGop;
   bool  bDeadlock;
   bool  bFoundEndOfGop;
   bool  bDqtMvcError;

   /*
    * SW7425-2686: variables for multi-pass DQT.
    */
   bool     bMultiPass;          /* The same GOP will be sent multiple times.  */
   bool     b1stPass1stGop;      /* First pass of first GOP, trigger on the PTS value if it is coded. */
   uint32_t uiTargetIndex;       /* Trigger intra-GOP picture index. */
   uint32_t uiTargetPTS;
   BXVD_PTSType eTargetPTSType;

   uint32_t uiAvailableBuffers;  /* Number of picture buffers that can be outstanding, from the PPB. */
   uint32_t uiBuffersInUse;      /* for "sliding window" logic, number of curently outstanding picture buffers. */
   uint32_t uiSizeOfChunk;       /* The number of pictures being displayed on this pass. */
   uint32_t uiOpenGopPictures;   /* The number of open GOP pictures in the current GOP, reported by the firmware. */

   /* The following state information is to help wring out the sliding window algorithm, i.e.
    * to help figure out when picture buffers should be returned to AVD.
    * These variables are not used in any XVD logic. */

   uint32_t uiGopLength;         /* length of the current GOP, set when the last picture flag or target PTS is reached.  */

   uint32_t uiPassCount;
   uint32_t uiNumberOfAdditionalPasses;
   uint32_t uiRemainder;
   uint32_t uiPicsPerPass;

} BXVD_Decoder_P_DQTContext;

typedef struct BXVD_Decoder_P_DNR
{
 unsigned int saved_mb_cnt;
 unsigned int adj_Qp_ref;
 unsigned int adj_Qp_pre;
} BXVD_Decoder_P_DNR;

typedef struct BXVD_Decoder_P_LogData
{

   /* Used for debug logging. */
   uint32_t    uiVsyncsPerSecond;
   uint32_t    uiCallbackCount;
   uint32_t    uiPicturesFromAvd;      /* count of pictures received from AVD. */
   uint32_t    uiPicturesToAvd;        /* count of pictures returned to AVD. */
   uint32_t    uiPicturesToPP;         /* count of pictures sent to PP */
   uint32_t    uiPicturesFromPP;       /* count of pictures returned from PP */
   uint32_t    uiOutstandingPics;      /* count of pictures being held by PP from previous decode */

   /* For timing callbacks and functions calls. */
   BXVD_DecoderTimer_P_Data   stTimerData;

   /* SW7125-1285: read these values back to ensure that they have been written to memory.
    * - physical address of the last PPB released
    * - release queue shadow index
    */
   uint32_t     pLastPPBReleased;
   uint32_t     uiShadowIndexAfterLastRelease;

} BXVD_Decoder_P_LogData;

typedef struct BXVD_P_Decoder_Context
{
   bool  bDecoderHasBeenInitialized;   /* set in BXVD_Decoder_StartDecode_isr */

   BXVD_Decoder_P_PictureContextQueue  stPictureContextQueue;

   BXVD_Decoder_P_UnifiedPictureQueue  stUnifiedPictureQueue;

   BXVD_Decoder_P_DQTContext  stDqtCntxt;

   BXVD_Decoder_P_DNR         stDNRInfo;

   BXVD_Decoder_Counters      stCounters;

   uint32_t                   uiDropCountSnapshot;

   bool                       bHostSparseMode; /* Host Sparse Mode */
   bool                       bReversePlayback; /* DQT */
   BXVD_MPEGPulldownOverride  ePulldownOverride;
   bool                       bCRCMode;

   /* SW7425-3358: essentially pPPB->timing_marker snapped
    * to the start of a GOP.
    */
   uint32_t                   uiCurrentChunkId;

   /* For debug. */
   BXVD_Decoder_P_LogData   stLogData;

   /* To help debug queue issues. */
   uint32_t    uiPreviousDeliveryQWriteOffset;

   /* FWAVD-289: Use the queue index to detect when a picture is being evaluated for the first time. */
   /*uint32_t uiPreviousWriteOffset;*/

#if BXVD_DM_ENABLE_YUV_GRAB_MODE
   /* variables assocated with the "grab" debug mode */
   bool  bGrabRvc;
   bool  bGrabPpb;
   bool  bGrabYuv;
#endif

} BXVD_P_Decoder_Context;

typedef struct BXVD_Decoder_P_LocalState
{
   uint32_t uiDeliveryQueueWriteOffset;   /* snapshot the offset to avoid race conditions. */

} BXVD_Decoder_P_LocalState;

BERR_Code
BXVD_Decoder_P_ComputeAspectRatio_isr(
         BXVD_P_PPB * pPPB,
         BXDM_Picture * pstXdmPicture
         );

/* SWSTB-788: move the calculation of the Delivery Queue depth from BXVD_GetChannelStatus_isr to
 * the XVD Decoder logic. BXVD_Decoder_P_GetStatus_isr() provides a method of retrieving the depth. */

typedef struct BXVD_Decoder_P_Status
{
   uint32_t uiPictureCountUnifiedQueue;   /* The number of pictures on the Unified Picture Queue. */
   uint32_t uiPictureCountDeliveryQueue;  /* The number of pictures on the Delivery Queue. For multi-picture protocols,
                                           * this will be the count of pairs of pictures, not the number of PPB's.*/
} BXVD_Decoder_P_Status;

BERR_Code
BXVD_Decoder_P_GetStatus_isr(
         BXVD_ChannelHandle hXvdCh,
         BXVD_Decoder_P_Status * pstStatus
         );

#ifdef __cplusplus
}
#endif

#endif /* BXVD_DECODER_PRIV_H_ */

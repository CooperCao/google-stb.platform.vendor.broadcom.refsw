/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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
 ******************************************************************************/
#ifndef BVDC_BUFFER_PRIV_H__
#define BVDC_BUFFER_PRIV_H__

#include "bavc.h"
#include "bvdc.h"
#include "bpxl.h"
#include "blst_circleq.h"
#include "bvdc_common_priv.h"
#include "bvdc_bufferheap_priv.h"
#if (BVDC_BUF_LOG == 1)
#include "bvdc_dbg.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * When both source and display are interlaced and the picture is not captured
 * as frame, reader has to repeat if it is about to catch up writer. There
 * are 2 possible algorithms to handle this:
 *
 * Algorithm 1: Reader does next-repeat-next-repeat. This will result in 2 field
 * inversion and 2 spreaded repeats over 4 fields.
 *
 * Algorithm 2: Reader does next-repeat-repeat-next. This will result in one picture
 * node being consectively displayed 3 times (2 repeats) but only 1 field inversion.
 *
 * Algorithm 1 favors video with a lot of motion because the 2 repeats are speaded to 4
 * fields. Algorithm 2 favors video with less motion because of only 1 field inversion.
 *
 * Below is a diagram shows the two scenario:
 *
 * VEC polarity     T	B	T	B	T	B	T	B	T	B
 * Captured buffer	T0	B0	T1	B1	T2	B2	T3	B3	T4	B4
 *
 * After reader displayed B0 and is about to move to T1, it detects that it's catching up writer.
 * Algorithm 1 will generate the following reader sequence:
 *
 * Reader (1)       T0	B0	B0	T1	T1	B1	T2	B2	T3	B3
 *
 * There are 2 field inversion at the second B0 and the first T1. Both B0 and T1 are repeated.
 *
 * Algorithm 2 will generate the following sequence:
 *
 * Reader (2)		T0	B0	B0	B0	T1	B1	T2	B2	T3	B3
 *
 * Only one field inversion at the second B0.  However B0 is repeated twice.
 */

#define BVDC_P_REPEAT_ALGORITHM_ONE  1 /* Default algorithm. Define this macro to 0 to enable
                                        * algorithm 2
                                        */

/***************************************************************************
 * Private macros
 ***************************************************************************/
/* check parameters */
#define BVDC_P_Buffer_GetDeviceOffset(pPictureNode)   \
	pPictureNode->pHeapNode->ulDeviceOffset

#define BVDC_P_Buffer_GetDeviceOffset_R(pPictureNode)   \
	pPictureNode->pHeapNode_R->ulDeviceOffset

#define BVDC_P_Buffer_GetNextNode(pPictureNode)   \
	BLST_CQ_NEXT(pPictureNode, link)

#define BVDC_P_Buffer_GetPrevNode(pPictureNode)   \
	BLST_CQ_PREV(pPictureNode, link)

#define BVDC_P_Buffer_GetNextActiveNode(pNextNode, pNode) \
{                                                         \
	pNextNode = BLST_CQ_NEXT(pNode, link);                \
	if(pNode->hBuffer->ulActiveBufCnt)                    \
	{                                                     \
		while( !pNextNode->stFlags.bActiveNode )                  \
			pNextNode = BLST_CQ_NEXT(pNextNode, link);    \
	}                                                     \
}

#define BVDC_P_Buffer_GetPrevActiveNode(pPrevNode, pNode) \
{                                                         \
	pPrevNode = BLST_CQ_PREV(pNode, link);                \
	if(pNode->hBuffer->ulActiveBufCnt)                    \
	{                                                     \
		while( !pPrevNode->stFlags.bActiveNode )                  \
			pPrevNode = BLST_CQ_PREV(pPrevNode, link);    \
	}                                                     \
}


#define BVDC_P_Buffer_GetNextActiveAndNotUsedByUserNode(pNextNode, pNode) \
{                                                         \
	pNextNode = BLST_CQ_NEXT(pNode, link);                \
	if(pNode->hBuffer->ulActiveBufCnt)                    \
	{                                                     \
		while((!pNextNode->stFlags.bActiveNode) || (pNextNode->stFlags.bUsedByUser))                 \
			pNextNode = BLST_CQ_NEXT(pNextNode, link);    \
	}                                                     \
}

#define BVDC_P_Buffer_GetPrevActiveAndNotUsedByUserNode(pPrevNode, pNode) \
{                                                         \
	pPrevNode = BLST_CQ_PREV(pNode, link);                \
	if(pNode->hBuffer->ulActiveBufCnt)                    \
	{                                                     \
		while((!pPrevNode->stFlags.bActiveNode) || (pPrevNode->stFlags.bUsedByUser))                  \
			pPrevNode = BLST_CQ_PREV(pPrevNode, link);    \
	}                                                     \
}

#define BVDC_P_Buffer_GetNextUsedByUserNode(pNextNode, pNode) \
{                                                         \
	pNextNode = BLST_CQ_NEXT(pNode, link);                \
	while(!pNextNode->stFlags.bUsedByUser)                 \
		pNextNode = BLST_CQ_NEXT(pNextNode, link);    \
}

/***************************************************************************
 * Internal defines
 ***************************************************************************/
#define BVDC_P_BUFFER_PRINT_DBG_MSG           (0)
#define BVDC_P_BUFFER_PRINT_DBG_MSG_CNT       (20)
#define BVDC_P_BUF_DBG_MSG_SIZE               ((200) * sizeof(uint32_t))

#define BVDC_P_BUFFER_NUM_FIELD_CAPTURE_B4_DISPLAY      (5)

/* Progressive detection threshold */
#define BVDC_P_BUFFER_PROG_DETECTION_THRESHOLD          (5)

/* 60Hz go into progressive mode with rate gap! */
#define BVDC_P_BUFFER_PROGRESSIVE_MODE_60_HZ_TRIGGER      (60)
#define BVDC_P_BUFFER_PROGRESSIVE_MODE_59_94_HZ_TRIGGER   (59)

/* Maximum active picture nodes of each buffer linked list */
#define BVDC_P_BUFFER_MAX_ACTIVE_NODES_PER_BUFT			  (24)

/***************************************************************************
 * Enums
 ***************************************************************************/
typedef enum BVDC_P_Last_Buffer_Action
{
	BVDC_P_Last_Buffer_Action_Reader_Moved,
	BVDC_P_Last_Buffer_Action_Writer_Moved
} BVDC_P_Last_Buffer_Action;

/****************************************************************************
 * dirty bits for sub-modules to build RUL.
 */
typedef union
{
	struct{
		uint32_t      bInputFormat      : 1; /* from user for VDEC
											  * from user or XVD for HD_DVI */
		uint32_t      bSrcPicChange     : 1; /* either src detected change or ApplyChange called */

	} stBits;

	uint32_t aulInts [BVDC_P_DIRTY_INT_ARRAY_SIZE];
} BVDC_P_PicDirtyBits;


/***************************************************************************
 * BVDC_P_PicComRulInfo
 *
 * Contains the pic info that are shared by all sub-modules when they build
 * RUL
 *
 * TODO: all sub-modules' BuildRul_isr should use the shared struct
 * BVDC_P_PicComRulInfo and a module specific BVDC_P_Pic*RulInfo as input
 * parameters.
 ***************************************************************************/
typedef struct BVDC_P_PicComRulInfo
{
	/* used by a module to remember the 1st window that actively uses it and
	 * therefore it should build the rul for the module */
	BVDC_P_WindowId                eWin;

	/* used by sub-modules to easily decide what init it needs to do when
	 * they build its rul at this picture cycle */
	BVDC_P_PicDirtyBits            PicDirty;

	/* Original source polarity. Updated every field. Used by multi-buffering. */
	BAVC_Polarity                  eSrcOrigPolarity;

	bool                           bDrTf; /* Secam Dr/Db is on Topfield */

	bool                           bNoCoreReset; /* no need to reset the core on drain vnet*/

} BVDC_P_PicComRulInfo;


/***************************************************************************
Summary:
	A structure representing the flags used by a picture node. .

****************************************************************************/
typedef struct
{
	uint32_t    bActiveNode               : 1;        /* Whether the node is active or not */
	uint32_t    bFormatChange             : 1;        /* Source information */
	uint32_t    bMute                     : 1;        /* Used by video feeder, not mpeg feeder */
	uint32_t    bMuteFixedColor           : 1;        /* Used by video feeder, not mpeg feeder */
	uint32_t    bMuteMad                  : 1;        /* Used by video feeder, not mpeg feeder */
	uint32_t    bPictureRepeatFlag        : 1;        /* previous field polarity, for trick mode support */
	uint32_t    bRepeatField              : 1;        /* previous field or frame for anr */
	uint32_t    bUsedByUser               : 1;        /* for user capture */
	uint32_t    bHandleFldInv             : 1;        /* force smooth scl for expected field inversion */
	uint32_t    bCadMatching              : 1;        /* Match captured polarity against VEC polarity */
	uint32_t    bRev32Locked              : 1;        /* reverse 3:2-pull-down locked, for debug only */

} BVDC_P_PicNodeFlags;



/***************************************************************************
 * BVDC_P_PictureNode
 *
 * Contains data in capture buffering. Data are used to build RUL.
 ***************************************************************************/
typedef struct BVDC_P_PictureNode
{
	/* Node info: linked-list bookeeping */
	BLST_CQ_ENTRY(BVDC_P_PictureNode)  link;                 /* doubly-linked list support */
	uint32_t                           ulBufferId;           /* Buffer ID */

	BVDC_P_Buffer_Handle               hBuffer;
	BVDC_P_HeapNodePtr                 pHeapNode;            /* Hold capture memory */

	/* Hold capture memory for Right buffer */
	BVDC_P_HeapNodePtr                 pHeapNode_R;
	/* Capture BVB input orientation */
	BFMT_Orientation                   eOrigSrcOrientation;
	/* Capture BVB input orientation, could be overwrite by Scaler 3D->2D conversion*/
	BFMT_Orientation                   eSrcOrientation;
	BFMT_Orientation                   eCapOrientation;
	/* Video feeder BVB output orientation */
	BFMT_Orientation                   eDispOrientation;
	BVDC_3dSourceBufferSelect          e3dSrcBufSel;
	/* 8 or 10-bit depth */
	BAVC_VideoBitDepth                 eBitDepth;
	bool                               bEnable10Bit;
	bool                               bEnableDcxm;

	/* Field inversion */
	BAVC_Polarity                      eDisplayPolarity;

	BAVC_FrameRateCode                 eFrameRateCode;       /* Frame rate code.
	                                                          * Updated every format change.
	                                                          * Used by scaler?
	                                                          */
	BAVC_MatrixCoefficients            eMatrixCoefficients;  /* Matrix coefficients.
	                                                          * Updated every format change.
	                                                          * Used by scaler?
	                                                          */
	BAVC_TransferCharacteristics       eTransferCharacteristics; /* transfer characteristics */

	BAVC_Polarity                      eSrcPolarity;         /* Top, bottom field or progressive.
	                                                          * Updated every field.
	                                                          * Used by MAD, video feeder and scaler.
	                                                          */
	BAVC_Polarity                      eDstPolarity;         /* Top, bottom field or progressive.*/
	uint32_t                           ulIdrPicID;
	uint32_t                           ulPicOrderCnt;


	/*Mailbox metadata for ViCE2 particularly*/
	uint32_t                           ulOrigPTS;
	int32_t                            iHorzPanScan;
	int32_t                            iVertPanScan;
	uint32_t                           ulDispHorzSize;
	uint32_t                           ulDispVertSize;
	uint32_t                           ulDecodePictureId;
	uint32_t                           ulStgPxlAspRatio_x_y; /* PxlAspRatio_x<<16 | PxlAspRatio_y */

	BAVC_USERDATA_PictureCoding        ePictureType;
	bool                               bIgnorePicture;
	bool                               bStallStc;
	bool                               bLast;
	bool                               bChannelChange;
	bool                               bValidAfd;
	bool                               bMute;
	uint32_t                           ulAfd;
	BAVC_BarDataType                   eBarDataType;
	uint32_t                           ulTopLeftBarValue;
	uint32_t                           ulBotRightBarValue;
	/*  Fast non real time (FNRT) meta data support */
	bool                              bPreChargePicture;
	bool                              bEndofChunk;
	uint32_t                          ulChunkId;

	/* Working rectangles for mpeg feeder, scaler, capture and the following video
	 * feeder:
	 *    Source content rectangle, and user's scaler-out and destination rectangles
	 * are adjusted by BVDC_P_Window_AdjustRectangles_isr().  For a majority of time if
	 * there are no user's changes nor source's stream changes, the rectangles will
	 * stay the same.
	 *    Both scan (i.e. mpeg feeder), capture, video feeder, and compositor
	 * could perform clip. However, only scaler can clip to sub-pixel level.
	 * Mad will not perform any cut.
	 *    Basing on the order defined by vnet mode, we do clip as early as
	 * possible.
	 *    In the case that the previous modules could not perform the needed clip
	 * completely, the following modules clip the rest. For example, scan might
	 * not be able to do clip completely becase another display share the mpeg
	 * source, then the following module should complete the clipping. Another
	 * example is that capture and feeder can only clip to pixel boundary, sub-
	 * pixel clipping should be done by the following scaler.
	 *    Pan-Scan is handled uniformly with other clipping. Sub-pixel pan-scan
	 * is considered as sub-pixel clip.
	 *    stSclCut.lLeft is for horizontal sub-pixel clipping. It is units of 1/16th
	 * of a pixel in S11.6 format. Bit 6 is the pixel grid, bits 5:0 are 1/16
	 * pixel grid. Even number of pixels are handled in feeder. It is updated every
	 * field and is used by scaler.
	 *    stSclCut.lTop is for vertical sub-pixel clipping.  It is units of 1/16th of
	 * a pixel in S11.14 format. Bits 3:0 are 1/16 pixel grid. Down to pixels are handled
	 * in feeder. Updated every field.  Used by scaler. */
	BVDC_P_Rect                        stSrcOut;             /* What XSRC output */
	BVDC_P_Rect                        stSclOut;             /* What scaler output. */
	BVDC_P_Rect                        stDnrOut;             /* What DNR outputs (mfd/vdec/hddvi output) */
	BVDC_P_Rect                        stWinIn;              /* vsur size and L/T cut*/
	BVDC_P_Rect                        stWinOut;             /* vdisp size and pos */
	BVDC_P_Rect                        stSclCut;             /* rect for scaler to pickup src cnt */
	BVDC_P_Rect                        stCapOut;             /* rect for cap out, with cut */
	BVDC_P_Rect                        stVfdOut;             /* rect for vfd out, with cut */
	BVDC_P_Rect                        stMadOut;             /* What MAD output. */

	/* Point to one of the above rectangles.
	 *    Clip in mpeg feeder is specified by pSrcOut, clip in scaler is specified
	 * by stSclCut, clip in capture is specified by pCapOut, clip in feeder is
	 * specified by pVfdOut, and clip in compositor is specified by pWinIn. pSrcOut,
	 * pSclIn, pCapIn, pVfdIn only specify the input width and height. */
	BVDC_P_Rect                       *pSrcOut;              /* -> stSrcOut */

	BVDC_P_Rect                       *pDnrIn;               /* from source mfd/hddvi etc */
	BVDC_P_Rect                       *pDnrOut;              /* -> pDnrIn */

	BVDC_P_Rect                       *pXsrcIn;              /* ->pDnr */
	BVDC_P_Rect                       *pXsrcOut;             /* ->pXsrcIn (H scaled) */

	BVDC_P_Rect                       *pHsclIn;              /* -> pSrcOut */
	BVDC_P_Rect                       *pHsclOut;             /* -> Hrz scaled */

	BVDC_P_Rect                       *pAnrIn;               /* -> pSrcOut, pVfdOut, pHsclOut */
	BVDC_P_Rect                       *pAnrOut;              /* -> pAnrIn */

	BVDC_P_Rect                       *pMadIn;               /* -> pSrcOut, pHsclOut, pVfdOut */
	BVDC_P_Rect                       *pMadOut;              /* -> Hrz scaled w/ framebased */

	BVDC_P_Rect                       *pSclIn;               /* -> pSrcOut, pVfdOut, pMadOut */
	BVDC_P_Rect                       *pSclOut;              /* -> stScalerOutRect */

	BVDC_P_Rect                       *pCapIn;               /* -> pSrcOut, pMadOut, pSclOut */
	BVDC_P_Rect                       *pCapOut;              /* -> pCapIn */

	BVDC_P_Rect                       *pVfdIn;               /* -> pCapOut */
	BVDC_P_Rect                       *pVfdOut;              /* -> pSclOut, stVfdOutput */

	BVDC_P_Rect                       *pWinIn;               /* -> pSrcOut, pSclOut, pVfdOut */
	BVDC_P_Rect                       *pWinOut;              /* -> stDstOutput */

	/* Dest information */
	BVDC_P_VnetMode                    stVnetMode;           /* Dictates what resources to use. */

	/* mcvp/madr/mad buffer config */
	uint32_t                           usMadPixelBufferCnt;
	uint32_t                           usMadQmBufCnt;
	uint32_t                           ulMadPxlBufSize;
	BVDC_P_BufferHeapId                eMadPixelHeapId;
	BVDC_P_BufferHeapId	               eMadQmHeapId;

	BFMT_VideoInfo                     *pStgFmtInfo;
	BFMT_VideoInfo                     stCustomFormatInfo;
	/* Adjusted Quantization Paramter for DNR */
	uint32_t                           ulAdjQp;

	/* for scaling in SCL */
	uint32_t                           ulNrmHrzSrcStep; /* normalized hrz scl factor in fixed point format */
	uint32_t                           ulNrmVrtSrcStep; /* normalized vrt scl factor in fixed point format */
	uint32_t                           ulNonlinearSrcWidth;/* in pxl unit */
	uint32_t                           ulNonlinearSclOutWidth;/* in pxl unit */
	uint32_t                           ulCentralRegionSclOutWidth;/* in pxl unit */

	/* TODO: Use hSclCut */
	/* for h-scaling before deinterlacing */
	uint32_t                           ulHsclNrmHrzSrcStep; /* normalized hrz scl factor in fixed point format */
	int32_t                            lHsclCutLeft; /* S11.6, same fmt as SclCut->lLeft */
	uint32_t                           ulHsclCutWidth; /* similar to SclCut->ulWidth */
	int32_t                            lHsclCutLeft_R; /* S11.6, same fmt as SclCut->lLeft_R */

	/* for h-scaling at Xsrc */
	uint32_t                           ulXsrcNrmHrzSrcStep; /* normalized hrz scl factor in fixed point format */

	const BPXL_Plane                  *pSurface;
	const BPXL_Plane                  *pSurface_R;
	BPXL_Format                        ePixelFormat;

	/* to be used by MAD chroma config (phase 3 & beyond) */
	BVDC_P_ChromaType                  eChromaType;

	/* Lip sync */
	uint32_t                           ulCaptureTimestamp;
	uint32_t                           ulPlaybackTimestamp;
	bool                               bValidTimeStampDelay;

	/* MosaicMode: ClearRect bit mask set (each bit corresponds to a mosaic rectangle, where
	   value 1 for enabled rect; value 0 for missing/invisible channel) */
	uint32_t                           ulMosaicRectSet;
	uint32_t                           ulChannelId;
	uint32_t                           ulPictureIdx; /* channel Id after recorder*/

	bool                               bMosaicMode;
	uint32_t                           ulMosaicCount;
	bool                               abMosaicVisible[BAVC_MOSAIC_MAX];
	BVDC_P_Rect                        astMosaicRect[BAVC_MOSAIC_MAX];

	bool                               bContinuous; /* mcvp buffer continuous indicator*/
	BVDC_P_Compression_Settings        stCapCompression;

	/* pic info shared by all sub-modules when they build rul
	 *
	 * TODO: all sub-modules' BuildRul_isr should use the shared struct
	 * BVDC_P_PicComRulInfo and a module specific BVDC_P_Pic*RulInfo as input
	 * parameters, in order to have clear boundary / interface between sub-
	 * modules.
	 */
	BVDC_P_PicComRulInfo               PicComRulInfo;

	/* bit flags used by picture node */
	BVDC_P_PicNodeFlags                stFlags;

	BVDC_LumaStatus                    stCurHistData;
	uint32_t                           ulCurHistSize;

	uint8_t                            ucAlpha;

	/* deinterlacer output phase in the case of 3:2 pulldown */
	uint32_t                           ulMadOutPhase;

	/* total amount of pixels */
	uint32_t                           ulPixelCount;

} BVDC_P_PictureNode;


/***************************************************************************
 * BRDC_P_Slot_Head
 *      Head of the double Link List for slot
 ***************************************************************************/
typedef struct BVDC_P_Buffer_Head  BVDC_P_Buffer_Head;
BLST_CQ_HEAD(BVDC_P_Buffer_Head, BVDC_P_PictureNode);

/***************************************************************************
 * Writer vs Reader rate enum
 *
 * This enum defines the three cases when comparing writer rate with
 * reader rate.
 *
 * Note: Round off is applied when calculating rate. So 59.97Hz and 60Hz
 * are considered same rate.
 *
 ***************************************************************************/
typedef enum _BVDC_P_WrRateCode
{
	BVDC_P_WrRate_NotFaster = 0,  /* Same rate or slower */
	BVDC_P_WrRate_Faster, /* Faster but not 2 times or more.
	                       * i.e. writer = 50Hz, reader = 60Hz
	                       */
	BVDC_P_WrRate_2TimesFaster /* 2 times or even more faster.
	                            * i.e. writer = 24Hz, reader = 60Hz
	                            * This usually is the pulldown case
	                            */

} BVDC_P_WrRateCode;

/***************************************************************************
 * Buffer Context
 *
 * Contains data per window. Each buffer context contains a list of buffer
 * nodes, a write pointer and a read pointer.
 ***************************************************************************/
typedef struct BVDC_P_BufferContext
{
	BDBG_OBJECT(BVDC_BUF)

	BVDC_Window_Handle             hWindow;       /* Created from this window */

	bool                           bSyncLock;

	uint32_t                       ulActiveBufCnt; /* Number of active buffers */
	uint32_t                       ulBufCnt;      /* Number of buffers */
	uint32_t                       ulSkipCnt;

	/* Skip and Repeat statistic */
	uint32_t                       ulSkipStat;
	uint32_t                       ulRepeatStat;

	/* This is the delay between writer and reader */
	uint32_t                       ulVsyncDelay;

	BVDC_P_Buffer_Head            *pBufList;      /* Double link list */

	BVDC_P_PictureNode            *pCurWriterBuf; /* Write buffer pointer.
	                                               * Points to capture buffer when
	                                               * capture is enabled.
	                                               */
	BVDC_P_PictureNode            *pCurReaderBuf; /* Read buffer pointer.
	                                               * Points to playback buffer when
	                                               * capture is enabled.
	                                               */
	BVDC_P_PictureNode            *pPrevReaderBuf;

	uint32_t                       ulNumCapField; /* Number of field captured */


	/* for progressive mode rates that are not 5060 Hz (VESA formats) */
	BVDC_P_WrRateCode              eWriterVsReaderRateCode;
	BVDC_P_WrRateCode              eReaderVsWriterRateCode;

	/* game mode buffer delay sampling interval */
	uint32_t                       ulGameDelaySamplePeriod;
	uint32_t                       ulGameDelaySampleCnt;

	BAVC_Polarity                  ePrevFieldId;

	/* indicates which buffer action executed last */
	BVDC_P_Last_Buffer_Action      eLastBuffAction;

	/* to track reader and writer isr firing order */
	uint32_t                      ulPrevWriterTimestamp;
	uint32_t                      ulCurrWriterTimestamp;
	uint32_t                      ulPrevReaderTimestamp;
	uint32_t                      ulCurrReaderTimestamp;
	uint32_t                      ulMaxTimestamp;
	bool                          bReaderWrapAround;
	bool                          bWriterWrapAround;
	bool						  bReaderNodeMovedByWriter;
	bool				          bWriterNodeMovedByReader;

	/* This array keeps the order of the active picture nodes being added to the
	 * buffer linked list. When releasing a picture node, it will be
	 * removed using the reverse order of when it was added. Basically
	 * a picture node is last in and first out. This is to help
	 * buffer heap fragmentation.
	 */
	BVDC_P_PictureNode			 *aBufAdded[BVDC_P_BUFFER_MAX_ACTIVE_NODES_PER_BUFT];
	int							  iLastAddedBufIndex;

#if BVDC_P_REPEAT_ALGORITHM_ONE
	/* a flag to indicate that reader repeats so for not catching up writer
	 */
    bool                          bRepeatForGap;
#endif

	/* This is effective while in MTG mode only. Indicates that the relationship
	 * between the display rate and the rate at which the desired pictures coming out
	 * of the deinterlacer is about 1:1. Specifically a display rate of 24Hz (or 25Hz)
	 * and a deinterlacer detecting a 3:2 cadence, the corresponding rate at which
	 * the desired pictures comes out of the deinterlacer is 24Hz; hence, about a 1:1
	 * rate relationship. This is needed by the multibuffering algorithm to ensure a
	 * gap of 1 exists between writer and reader to prevent tearing. Refer to the
	 * multi-buffer algorithm's MTG timeline analysis.
	 */
	bool                          bMtgMadDisplay1To1RateRelationship;

	/* This is effective while in MTG mode only and the associated deinterlacer is
	 * disabled. This indicates that the repeated pictures sent by the DM are to be
	 * dropped or not. It distinguishes a MTG source that has repeats from a MTG source
	 * without repeats. A MTG source without repeats is handled as a regular source
	 * in that all pictures will be processed by the multi-buffer algorithm. Refer to
	 * to the multi-buffer algorithm's MTG timeline analysis.
	 */
	bool                          bMtgRepeatMode;

	/* This keeps track of the number of repeated pictures and is used to determine
	 * bMtgRepeatMode along with ulMtgUniquePicCount.
	 */
	uint32_t                      ulMtgSrcRepeatCount;

	/* This keeps track of the number of non-repeated pictures and is used to determine
	 * bMtgRepeatMode along with ulMtgSrcRepeatCount.
	 */
	uint32_t                      ulMtgUniquePicCount;

	/* Keeps track of how many times the reader node in the multibuffer algorithm
	 * was repeated. This only pertains when when we have a 60i-24 MTG src that is
	 * displayed at 50Hz. This is needed to avoid a 2:2:3:1:2 cadence and instead
	 * produce a 2:2:3:2:2 cadence. See corresponding multi-buffer algorithm's
	 * MTG timeline analysis.
	*/
	uint32_t                      ulMtgDisplayRepeatCount;

} BVDC_P_BufferContext;


/***************************************************************************
 * Buffer private functions
 ***************************************************************************/
BERR_Code BVDC_P_Buffer_Create
	( const BVDC_Window_Handle         hWindow,
	  BVDC_P_Buffer_Handle            *phBuffer );

BERR_Code BVDC_P_Buffer_Destroy
	( BVDC_P_Buffer_Handle             hBuffer );

void BVDC_P_Buffer_Init
	( BVDC_P_Buffer_Handle             hBuffer );

BERR_Code BVDC_P_Buffer_AddPictureNodes_isr
	( BVDC_P_Buffer_Handle             hBuffer,
	  BVDC_P_HeapNodePtr               apHeapNode[],
	  BVDC_P_HeapNodePtr               apHeapNode_R[],
	  uint32_t                         ulSurfaceCount,
	  uint32_t                         ulBufDelay,
	  bool                             bSyncLock,
	  bool                             bInvalidate );

BERR_Code BVDC_P_Buffer_ReleasePictureNodes_isr
	( BVDC_P_Buffer_Handle             hBuffer,
	  BVDC_P_HeapNodePtr               apHeapNode[],
	  BVDC_P_HeapNodePtr               apHeapNode_R[],
	  uint32_t                         ulSurfaceCount,
	  uint32_t                         ulBufDelay );

BERR_Code BVDC_P_Buffer_Invalidate_isr
	( BVDC_P_Buffer_Handle             hBuffer );

BVDC_P_PictureNode* BVDC_P_Buffer_GetPrevWriterNode_isr
	( const BVDC_P_Buffer_Handle       hBuffer );

BVDC_P_PictureNode* BVDC_P_Buffer_GetNextWriterNode_isr
	( BVDC_Window_Handle     hWindow,
	  const BAVC_Polarity    eSrcPolarity,
	  bool                   bMtg,
	  bool                   bMtgRepeat );

BVDC_P_PictureNode* BVDC_P_Buffer_GetNextReaderNode_isr
	( BVDC_Window_Handle     hWindow,
	  const BAVC_Polarity    eVecPolarity,
	  bool                   bMtg,
	  bool                   bMtgRepeat );

void BVDC_P_Buffer_SetCurWriterNode_isr
	( const BVDC_P_Buffer_Handle       hBuffer,
	  BVDC_P_PictureNode              *pPicture);

void BVDC_P_Buffer_SetCurReaderNode_isr
	( const BVDC_P_Buffer_Handle       hBuffer,
	  BVDC_P_PictureNode              *pPicture);

void  BVDC_P_Buffer_ReturnBuffer_isr
	( BVDC_P_BufferContext   *pBuffer,
	BVDC_P_PictureNode *pPicture);

BERR_Code  BVDC_P_Buffer_ExtractBuffer_isr
	( BVDC_P_BufferContext   *pBuffer,
	BVDC_P_PictureNode   **ppPicture);

#define BVDC_P_Buffer_GetCurrWriterNode_isr(hBuffer) \
    ((hBuffer)->pCurWriterBuf)

#define BVDC_P_Buffer_GetCurrReaderNode_isr(hBuffer) \
    ((hBuffer)->pCurReaderBuf)

void BVDC_P_Buffer_CalculateRateGap_isr
	( const uint32_t        ulSrcVertFreq,
	  const uint32_t        ulDispVertFreq,
	  BVDC_P_WrRateCode    *peWriterVsReaderRateCode,
	  BVDC_P_WrRateCode    *peReaderVsWriterRateCode);

uint32_t BVDC_P_Buffer_CalculateBufDelay_isr
	( BVDC_P_PictureNode   *pPicture,
	  bool                 *pbValidDelay );

BVDC_P_PictureNode * BVDC_P_Buffer_GetCurWriterNode_isr
	( const BVDC_P_Buffer_Handle       hBuffer);

BVDC_P_PictureNode * BVDC_P_Buffer_GetCurReaderNode_isr
	( const BVDC_P_Buffer_Handle       hBuffer);

#if (BVDC_P_SUPPORT_3D_VIDEO)
BERR_Code BVDC_P_Buffer_SetRightBufferPictureNodes_isr
	( BVDC_P_Buffer_Handle             hBuffer,
	  BVDC_P_HeapNodePtr               apHeapNode_R[],
	  uint32_t                         ulCount,
	  bool                             bAdd);
#endif

#if (BVDC_BUF_LOG == 1)
void BVDC_P_Buffer_SetLogStateAndDumpTrigger
	( BVDC_BufLogState                 eLogState,
	  const BVDC_CallbackFunc_isr	   pfCallback,
	  void							   *pvParm1,
	  int							   iParm2 );

void BVDC_P_Buffer_DumpLog(void);

void BVDC_P_Buffer_SetManualTrigger(void);

void BVDC_P_Buffer_EnableBufLog
	( BVDC_P_WindowId                   eId,
	  bool                              bEnable);

#endif


#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_BUFFER_PRIV_H__*/

/* End of file. */

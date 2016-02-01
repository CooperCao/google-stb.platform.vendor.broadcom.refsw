/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BXDM_DECODER_H_
#define BXDM_DECODER_H_

#include "bxdm_picture.h"
#include "bxdm_pp_types.h"

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

/* 
 * SW7425-2536: structure returned by displayInterruptStatus_isr.
 */
typedef struct BXDM_PictureProvider_Status
{
   /* The count of vsync interrupts since BXDM_PictureProvider_StartDecode_isr() 
    * was called. Intended for debug, perhaps will be useful in other ways.
    */
   uint32_t uiVsyncCount;

   /* The picture delivery queue was empty on this vsync. */
   bool bPictureQueueIsEmpty;

   /* Status for the picture being delivered to VDC on this vsync. */
   BXDM_PictureProvider_PictureInfo stCurrentPicture;

} BXDM_PictureProvider_Status;

/* BXDM_Decoder_GetPictureCount_isr -
 *
 * BXDM_Decoder_GetPictureCount_isr returns the total number of unified pictures available on the
 * decoder's picture delivery queue.  During a decode, it must be a non-decreasing number as long as
 * BXDM_Decoder_GetNextPicture_isr is not called.
 *
 * See: BXDM_Decoder_PeekAtPicture_isr
 */
typedef BERR_Code
(*BXDM_Decoder_GetPictureCount_isr)(void *pvHandle, uint32_t *puiPictureCount);

/* BXDM_Decoder_PeekAtPicture_isr -
 *
 * BXDM_Decoder_PeekAtPicture_isr returns a pointer to the indexed unified picture.
 * The picture remains in the decoder's picture delivery queue.  The valid index range
 * is [0, (BXDM_Decoder_GetPictureCount_isr() - 1)].
 *
 * Calling BXDM_Decoder_PeekAtPicture_isr with the same index repeatedly should return
 * the same unified picture, until BXDM_Decoder_GetNextPicture_isr is called.
 *
 * See: BXDM_Decoder_GetPictureCount_isr
 * See: BXDM_Decoder_GetNextPicture_isr
 */
typedef BERR_Code
(*BXDM_Decoder_PeekAtPicture_isr)(void *pvHandle, uint32_t uiIndex, const BXDM_Picture **pUnifiedPicture);

/* BXDM_Decoder_GetNextPicture_isr -
 *
 * BXDM_Decoder_GetNextPicture_isr is called by XDM when it wants the next picture off of
 * the decoder's picture delivery queue.  The decoder cannot modify data associated with the
 * unified picture that is returned by this function until the same picture is returned
 * via BXDM_Decoder_ReleasePicture_isr.
 *
 * See: BXDM_Decoder_ReleasePicture_isr
 */
typedef BERR_Code
(*BXDM_Decoder_GetNextPicture_isr)(void *pvHandle, const BXDM_Picture **pUnifiedPicture);


typedef struct BXDM_Decoder_ReleasePictureInfo
{
      bool bValid;
} BXDM_Decoder_ReleasePictureInfo;

/* BXDM_Decoder_ReleasePicture_isr -
 *
 * BXDM_Decoder_ReleasePicture_isr is called when XDM is done with the unified picture.
 * The unified picture may still be in use (e.g. currently being scanned to the display)
 * so it is important that the decoder wait until the NEXT display interrupt event
 * before fully re-using the memory associated with the released picture.
 *
 * BXDM_Decoder_ReleasePicture_isr could be called multiple times before the next display
 * interrupt event, so the decoder may want to implement a pending released queue that is
 * purged on the next display interrupt event.
 *
 * BXDM_Decoder_ReleasePicture_isr should be passed a unified pictures that was obtained
 * via a BXDM_Decoder_GetNextPicture_isr from the SAME decoder channel.  However, it is
 * not necessary for the picture to be released during the same decode.  I.e. it is possible
 * for the XDM to retain the same picture(s) across multiple start/stop decode sequences.
 * E.g. hold last picture during a channel change.
 *
 * The pReleasePictureInfo could be NULL.
 *
 * See: BXDM_Decoder_DisplayInterruptEvent_isr
 * See: BXDM_Decoder_GetNextPicture_isr
 */
typedef BERR_Code
(*BXDM_Decoder_ReleasePicture_isr)(void *pvHandle, const BXDM_Picture *pUnifiedPicture, const BXDM_Decoder_ReleasePictureInfo *pReleasePictureInfo);

/* BXDM_Decoder_GetPictureDropPendingCount_isr -
 *
 * BXDM_Decoder_GetPictureDropPendingCount_isr returns the total number of fields that are
 * pending to be dropped by the decoder.  The picture drop pending count MUST be relative
 * to the first (0th) picture on the picture delivery queue. E.g.
 *
 * | Pic | Request | Pending | Dropped |
 * | 0   | 0       | 0       | 0       |
 * | 1   | 6       | 6       | 0       |
 * | 2   | 6       | 4       | 2       |
 * | 3   | 6       | 2       | 4       |
 * | 4   | 6       | 0       | 6       |
 *
 * To further clarify, if we have the following call sequence:
 *
 *    1) BXDM_Decoder_GetPictureDropPendingCount_isr --> 6
 *    2) BXDM_Decoder_GetNextPicture_isr
 *    3) BXDM_Decoder_GetPictureDropPendingCount_isr --> 4
 *    4) BXDM_Decoder_GetNextPicture_isr
 *    5) BXDM_Decoder_GetPictureDropPendingCount_isr --> 0
 *
 * XDM knows that 2 fields were dropped before the picture returned in #2 was
 * decoded.  XDM also knows that 4 fields were dropped before the picture returned
 * in #4 was decoded.
 *
 * If the decoder is unable to drop the requested
 *
 * See: BXDM_Decoder_RequestPictureDrop_isr
 */
typedef BERR_Code
(*BXDM_Decoder_GetPictureDropPendingCount_isr)(void *pvHandle, uint32_t *puiPictureDropPendingCount);

/* BXDM_Decoder_RequestPictureDrop_isr -
 *
 * BXDM_Decoder_RequestPictureDrop_isr is called by XDM when it wants the decoder to drop a certain
 * number of fields.  The count is ALWAYS in the number of fields, even if the content is progressive.
 *
 *    | pulldown   | fields |
 *    | TB,BT      | 2      |
 *    | F,X2,X3,X4 | 2      |
 *    | TBT,BTB    | 3      |
 *
 * Updates the argument with the actual number of fields requested to be dropped from the decoder.
 * May differ from the original number requested due to capping of the request, truncating the request
 * to an even number of fields, or zero if a request is currently pending.
 *
 * See: BXDM_Decoder_GetPictureDropPendingCount_isr
 */
typedef BERR_Code
(*BXDM_Decoder_RequestPictureDrop_isr)(void *pvHandle, uint32_t *puiPictureDropRequestCount);

/* BXDM_Decoder_DisplayInterruptEvent_isr -
 *
 * BXDM_Decoder_DisplayInterruptEvent_isr is called once per display interrupt BEFORE any XDM processing
 * is done for the display interrupt.  This callback is the ideal place for the decoder to reclaim any
 * released pictures, update it's queue state, etc.
 */
typedef BERR_Code
(*BXDM_Decoder_DisplayInterruptEvent_isr)(void *pvHandle);

/* BXDM_Decoder_DisplayInterruptStatus_isr
 * 
 * SW7425-2536: added to support "drop at decode" for non-AVD decoders, perhaps it will prove useful in 
 * other situations.  This routine is called once per display interrupt, at the end of XDM's execution.
 */
typedef  void 
(*BXDM_Decoder_DisplayInterruptStatus_isr)( void *pvHandle, const BXDM_PictureProvider_Status *pstPictureInfo );

typedef struct BXDM_Decoder_Interface
{
   /* Returns the number of unique base pictures available in the decoder's picture delivery queue */
   BXDM_Decoder_GetPictureCount_isr getPictureCount_isr;

   /* Returns a pointer to the specified unified picture on the decoder's picture delivery queue, 
    * but DOES NOT remove it from the queue */
   BXDM_Decoder_PeekAtPicture_isr peekAtPicture_isr;

   /* Returns a pointer to the next unified picture on the decoder's picture delivery queue 
    * and DOES remove it from the queue */
   BXDM_Decoder_GetNextPicture_isr getNextPicture_isr;

   /* Adds unified picture to the decoder's release queue */
   BXDM_Decoder_ReleasePicture_isr releasePicture_isr;

   /* (optional) Returns the decoder's current pending drop request count */
   BXDM_Decoder_GetPictureDropPendingCount_isr getPictureDropPendingCount_isr;

   /* (optional) Specifies how many fields the decoder should try to drop */
   BXDM_Decoder_RequestPictureDrop_isr requestPictureDrop_isr;

   /* (optional) Will be called once at the beginning of each display interrupt 
    * before the Picture Provider processes pictures */
   BXDM_Decoder_DisplayInterruptEvent_isr displayInterruptEvent_isr;

   /* (optional) Will be called once at the end of the display interrupt,
    * after the Picture Provider (XDM) has finished execution. */
   BXDM_Decoder_DisplayInterruptStatus_isr displayInterruptStatus_isr;    

} BXDM_Decoder_Interface;


/*
 * SW7425-4630
 */
typedef struct BXDM_PictureProvide_P_FilterContext * BXDM_Filter_Handle;

typedef struct BXDM_PictureProvider_FilterInterface
{
   BXDM_Filter_Handle hFilter;

   BXDM_Decoder_Interface stInputInterface;

   /* The video decoder or up stream filter. */
   void *pUpStreamPrivateContext;

} BXDM_PictureProvider_FilterInterface;

#ifdef __cplusplus
}
#endif

#endif /* BXDM_DECODER_H_ */

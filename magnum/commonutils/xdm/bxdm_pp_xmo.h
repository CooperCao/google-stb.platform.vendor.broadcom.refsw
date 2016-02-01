/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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

#ifndef bxdm_pp_XMO_H__
#define bxdm_pp_XMO_H__

#include "bxdm_pp.h"


#ifdef __cplusplus
extern "C" {
#endif

#if 0
}
#endif


/********************************************************************************
 *
 * XMO Public data types and constants
 *
 * TODO: the following should be in a private include file.
 *
 ********************************************************************************/

/* The decoder interfaces  */
#define BXDM_PP_XMO_P_Inputs_Max 2

#define BXDM_PP_XMO_Base_Index       0
#define BXDM_PP_XMO_Enhanced_Index   1

typedef struct BXDM_PP_XMO_DecoderInterface
{
   BXDM_Decoder_Interface stInterface;
   void * pPrivateContext;

} BXDM_PP_XMO_DecoderInterface;


/* The merged unified picture queue. */

#define BXDM_PP_XMO_P_QUEUE_DEPTH   8

typedef struct BXDM_PP_XDM_P_QueueElement
{
   bool  bInUse;

   /* for debug */
   uint32_t uiIndex;

   /* The the pointer to the Unified Pictures in the BXDM_Decoder_Interface
    * interface is "const".  The base picture needs to be modified, so
    * it needs to be copied to a local structure.  When releasing the base
    * picture, we need to return a pointer; store the pointer as well as
    * the copy.
    * The enhanced picture does not need to be modified, it does not
    * need to be copied.
    */
   BXDM_Picture  stBasePicture;
   const BXDM_Picture * pstBasePicture;
   const BXDM_Picture * pstEnhancedPicture;

   /* For implementing the Picture Queue as a linked list. */
   struct BXDM_PP_XDM_P_QueueElement * pstPreviousElement;
   struct BXDM_PP_XDM_P_QueueElement * pstNextElement;

} BXDM_PP_XDM_P_QueueElement;


typedef struct BXDM_PP_XMO_P_PictureQueue
{
   uint32_t uiReadOffset; /* Always the picture under evaluation */
   uint32_t uiWriteOffset; /* if uiWriteOffset == uiReadOffset, queue is empty */

   uint32_t uiSearchIndex;

   int32_t  iNumPictures;

   int32_t  iNumUsedElements;

   BXDM_PP_XDM_P_QueueElement * pstTailElement;
   BXDM_PP_XDM_P_QueueElement * pstHeadElement;

   BXDM_PP_XDM_P_QueueElement astPicture[ BXDM_PP_XMO_P_QUEUE_DEPTH ];

} BXDM_PP_XMO_P_PictureQueue;

/*
 * State/status related types and definitions.
 */

typedef enum BXDM_PP_XMO_P_State
{
      BXDM_PP_XMO_P_State_eStopped,
      BXDM_PP_XMO_P_State_eStarted,

      BXDM_PP_XMO_P_State_eMax
} BXDM_PP_XMO_P_State;


typedef struct BXDM_PP_XMO_P_Status
{
   uint32_t uiNumPicturesReceived;
   uint32_t uiNumPicturesReturned;

   BXDM_PP_XMO_P_State eState;  /* running or stopped */

} BXDM_PP_XMO_P_Status;


/* The main XMO structure */

typedef struct BXDM_PictureProvider_XMO_P_Context
{

   BXDM_PP_XMO_DecoderInterface  astDecoderInterface[BXDM_PP_XMO_P_Inputs_Max];

   BXDM_PP_XMO_P_PictureQueue    stPictureQueue;

   BXDM_PP_XMO_P_Status          astStatus[BXDM_PP_XMO_P_Inputs_Max];

   /* In order to ensure that pictures are returned to AVD, the XMO cannot "hold last picture"
    * at StopDecode time; "hold list picture" is forced to false when the XMO is running.
    * The channel change settings specified by the application are saved in StartDecode
    * and then restored in StopDecode.  Note: this data is not used by the XMO.
    */
   BXDM_PictureProvider_ChannelChangeSettings stSavedChannelChangeSettings;

#if xxx
      BXDM_PictureProvider_P_State stDMState; /* Persistent DM State */
      BXDM_PictureProvider_P_Config stDMConfig; /* DM Configuration */
      BXDM_PictureProvider_P_Status stDMStatus; /* DM Status */

      BXDM_PictureProvider_P_Callback stCallbacks[BXDM_PictureProvider_Callback_eMax];

      BAVC_MFD_Picture *astMFDPicture;
      uint32_t uiMFDPictureCount;
#endif
   BTMR_TimerHandle hTimer;      /* TODO, we may want this for performance testing */

} BXDM_PictureProvider_XMO_P_Context;




/********************************************************************************
 *
 * XMO Public data types and constants
 *
 ********************************************************************************/

typedef struct BXDM_PictureProvider_XMO_P_Context * BXDM_PictureProvider_XMO_Handle;


/********************************************************************************
 *
 * XMO Public API
 *
 ********************************************************************************/

/*
 * Allocates the required memory for the XMO and initializes it to '0'.
 */
BERR_Code BXDM_PictureProvider_XMO_Create(
   BXDM_PictureProvider_XMO_Handle *phXmo
   );

/*
 * Frees the memory associated with the XMO.  Does NOT set hXmo to NULL,
 * that is the responsibility of the application.
 */
BERR_Code BXDM_PictureProvider_XMO_Destroy(
   BXDM_PictureProvider_XMO_Handle hXmo
   );

/*
 * Effectively "OpenChannel", the picture queue is initialized in this routine.
 * Fills in "pstDecoderInterface" with the routines for XMO.
 * Fills in "pPrivateContext" with the XMO context.
 */
BERR_Code BXDM_PictureProvider_XMO_GetDMInterface(
   BXDM_PictureProvider_XMO_Handle hXmo,
   BXDM_Decoder_Interface *pstDecoderInterface,
   void **pPrivateContext
   );

/*
 * Registers a decoder with the XMO filter. Typically this will be the XVD Decoder,
 * but it could by any decoder that supports the BXDM_Decoder_Interface interface.
 * "uiIndex" is the ID of the decoder interface, there is a maximum of two.
 * Interface '0' is treated as the base/master decoder.
 * Interface '1' is treated as the slave/enhanced decoder.
 */
BERR_Code BXDM_PictureProvider_XMO_SetDecoderInterface(
   BXDM_PictureProvider_XMO_Handle hXmo,
   uint32_t uiIndex,
   BXDM_Decoder_Interface *pstDecoderInterface,
   void *pPrivateContext
   );

/*
 * Sets the state of the specified channel to "running".
 *
 * "uiIndex" is the ID of the decoder interface, there is a maximum of two.
 * Interface '0' is treated as the base/master decoder.
 * Interface '1' is treated as the slave/enhanced decoder.
 *
 * The decoders can be started in any order.
 */
BERR_Code BXDM_PictureProvider_XMO_StartDecode_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   uint32_t uiIndex
   );

/*
 * Sets the state of the specified channel to "stopped".
 *
 * "uiIndex" is the ID of the decoder interface, there is a maximum of two.
 * Interface '0' is treated as the base/master decoder.
 * Interface '1' is treated as the slave/enhanced decoder.
 *
 * Needs to be called to release pictures on the internal queue.
 *
 * Should be called between:
 *    BXDM_PictureProvider_StopDecode_isr(pXvdCh->hPictureProvider);
 *    and BXVD_Decoder_StopDecode_isr( pXvdCh );
 *
 */
BERR_Code BXDM_PictureProvider_XMO_StopDecode_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   uint32_t uiIndex
   );

/*
 * Reset the internal picture queue?
 * Call between:
 *    BXDM_PictureProvider_WatchdogReset_isr(hXvdCh->hPictureProvider);
 *    and BXVD_Decoder_WatchdogReset_isr( hXvdCh );
 */
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXDM_PictureProvider_XMO_WatchdogReset_isr(
   BXDM_PictureProvider_XMO_Handle hXmo
   );
#endif

/*
 * Used to save the channel change settings at StartDecode time.
 */
BERR_Code BXDM_PictureProvider_XMO_SaveChannelChangeSettings_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   BXDM_PictureProvider_ChannelChangeSettings * pstChannelChangeSettings
   );

/*
 * Used to restore the channel change settings at StopDecode time.
 */
BERR_Code BXDM_PictureProvider_XMO_RetrieveChannelChangeSettings_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   BXDM_PictureProvider_ChannelChangeSettings * pstChannelChangeSettings
   );



/********************************************************************************
 *
 * XMO BXDM_Decoder_Interface API
 *
 ********************************************************************************/

/*
 * Called once by XDM at the beginning of the vsync prior to any other
 * processing.  This is how the "crank gets turned".  This routine will call
 * "displayInterruptEvent_isr" for the attached decoders to update the Unified
 * Picture Queues.  It will then call the registered "getPictureCount_isr",
 * "peekAtPicture_isr" and "getNextPicture_isr" interfaces to build the list of
 * left/right picture pairs.
 */
BERR_Code BXDM_PictureProvider_XMO_DisplayInterruptEvent_isr(
   BXDM_PictureProvider_XMO_Handle hXmo
   );

/*
 * Returns the number valid elements on the internal picture queue.
 * Currently this is the number of left/right picture pairs, e.g. "1" for one
 * pair of left/right pictures.
 */
BERR_Code BXDM_PictureProvider_XMO_GetPictureCount_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   uint32_t *puiPictureCount
   );

/*
 * Returns a pointer to the pair of pictures at the head of the queue.
 * "uiIndex" is ignored.
 */
BERR_Code BXDM_PictureProvider_XMO_PeekAtPicture_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   uint32_t uiIndex,
   const BXDM_Picture **pUnifiedPicture
   );

/*
 * Called by XDM when it wants the next picture on the queue.
 */
BERR_Code BXDM_PictureProvider_XMO_GetNextPicture_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   const BXDM_Picture **pUnifiedPicture
   );

/*
 * Called by XDM when it is done with a picture.
 * Calls the "releasePicture_isr" routines for the attached decoders.
 */
BERR_Code BXDM_PictureProvider_XMO_ReleasePicture_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   BXDM_Picture *pUnifiedPicture,
   const BXDM_Decoder_ReleasePictureInfo *pReleasePictureInfo
   );

/*
 * How to implement this with two independent streams which are
 * potentially different protocols?
 */
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXDM_PictureProvider_XMO_GetPictureDropPendingCount_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   uint32_t *puiPictureDropPendingCount
   );
#endif

/*
 * How to implement this with two independent streams which are
 * potentially different protocols?
 */
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXDM_PictureProvider_XMO_RequestPictureDrop_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   uint32_t *puiPictureDropRequestCount
   );
#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef bxdm_pp_XMO_H__ */

/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
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

#ifndef BMUXLIB_H_
#define BMUXLIB_H_

#include "bavc.h"

#ifdef __cplusplus
extern "C" {
#if 0
}
#endif
#endif

/* For muxer proposal: These would be the common interface definition between a muxer
 * and the audio/video encoders
 */

typedef enum BMUXlib_FinishMode
{
      BMUXlib_FinishMode_ePrepareForStop,    /* Finishes muxing until all active CDBs are empty and EOS is seen. */

      BMUXlib_FinishMode_eMax
} BMUXlib_FinishMode;


/* State Transitions
+-----------------+-------------------------------------------------------------------------------+
|                 | Current State                                                                 |
|                 +------------+-----------------+------------------+------------------+----------+
| Event           | Stopped[1] | Started         | Finishing_Input  | Finishing_Output | Finished |
+-----------------+------------+-----------------+------------------+------------------+----------+
| Start           | Started    | Invalid         | Invalid          | Invalid          | Invalid  |
| Finish          | Invalid    | Finishing_Input | NOP              | NOP              | NOP      |
| Finished_Input  | Invalid    | Invalid         | Finishing_Output | Invalid          | Invalid  |
| Finished_Output | Invalid    | Invalid         | Invalid          | Finished         | Invalid  |
| Stop            | [3]        | Stopped[2]      | Stopped[2]       | Stopped[2]       | Stopped  |
+-----------------+------------+-----------------+------------------+------------------+----------+

Notes:
 [1] Stopped is the initial state
 [2] Muxer output may not be usable if Stop is called before the Finished state is reached
 [3] A stop in the stopped state simply resets everything
 Invalid Event = Error + No change in state
 NOP = do nothing (no state change)

*/

typedef enum BMUXlib_State
{
   BMUXlib_State_eStopped,          /* Muxing done/idle */
   BMUXlib_State_eStarted,          /* Actively muxing */
   BMUXlib_State_eFinishingInput,   /* Stop requested, still retrieving data from encoder(s) */
   BMUXlib_State_eFinishingOutput,  /* Stop requested, no longer retrieving data from encoder(s), finalizing output */
   BMUXlib_State_eFinished,

   BMUXlib_State_eMax
} BMUXlib_State;

typedef struct BMUXlib_DoMux_Status
{
   unsigned uiNextExecutionTime;    /* (in milliseconds) the maximum time before the muxer must be executed again to prevent underflow at the mux. */
   BMUXlib_State eState;
   bool bBlockedOutput;             /* indicates the mux is blocked waiting for output/storage to complete */
   unsigned uiFinishStepsTotal;     /* total number of steps required to complete the file */
   unsigned uiFinishStepsComplete;  /* number of steps completed so far (allows generation of %complete status) */
   unsigned uiCompletedDuration;    /* (in milliseconds) media duration completed so far */
} BMUXlib_DoMux_Status;

typedef struct BMUXlib_CompressedBufferStatus
{
      /* Note: The following are CACHED addresses.
       *
       * The encoder must guarantee cache coherence for the referenced data.
       * The consumer must convert to uncached or offset as needed if the data will
       * be consumed by HW.
       */
      BMMA_Block_Handle hFrameBufferBlock; /* The BMMA block for the CDB. */
      BMMA_Block_Handle hMetadataBufferBlock; /* The BMMA block for the metadata (must be cpu accessible) */
} BMUXlib_CompressedBufferStatus;

typedef BERR_Code
(*BMUXlib_GetBufferStatus)(
   void *pvContext,
   BMUXlib_CompressedBufferStatus *pBufferStatus
);

typedef BERR_Code
(*BMUXlib_ConsumeBufferDescriptors)(
   void *pvContext,
   size_t uiNumDescriptors
   );

/***************************/
/* Video Encoder Interface */
/***************************/
typedef BERR_Code
(*BMUXlib_GetVideoBufferDescriptors)(
   void *pvContext,
   const BAVC_VideoBufferDescriptor **astDescriptors0, /* Pointer to an array of descriptors. E.g. *astDescriptorsX[0] is the first descriptor. *astDescriptorsX may be set to NULL iff uiNumDescriptorsX=0. */
   size_t *puiNumDescriptors0,
   const BAVC_VideoBufferDescriptor **astDescriptors1, /* Needed to handle FIFO wrap. Can be NULL if uiNumDescriptors1=0. */
   size_t *puiNumDescriptors1
   );

typedef struct BMUXlib_VideoEncoderInterface
{
      void *pContext;
      BMUXlib_GetVideoBufferDescriptors fGetBufferDescriptors;
      BMUXlib_ConsumeBufferDescriptors fConsumeBufferDescriptors;
      BMUXlib_GetBufferStatus fGetBufferStatus;

      BAVC_VideoBufferInfo stBufferInfo;
} BMUXlib_VideoEncoderInterface;

/***************************/
/* Audio Encoder Interface */
/***************************/
typedef BERR_Code
(*BMUXlib_GetAudioBufferDescriptors)(
   void *pvContext,
   const BAVC_AudioBufferDescriptor **astDescriptors0, /* Pointer to an array of descriptors. E.g. *astDescriptors0[0] is the first descriptor.  Can be NULL if uiNumDescriptors0=0. */
   size_t *puiNumDescriptors0,
   const BAVC_AudioBufferDescriptor **astDescriptors1, /* Needed to handle FIFO wrap. Can be NULL if uiNumDescriptors1=0. */
   size_t *puiNumDescriptors1
   );

typedef struct BMUXlib_AudioEncoderInterface
{
      void *pContext;
      BMUXlib_GetAudioBufferDescriptors fGetBufferDescriptors;
      BMUXlib_ConsumeBufferDescriptors fConsumeBufferDescriptors;
      BMUXlib_GetBufferStatus fGetBufferStatus;

      BAVC_AudioBufferInfo stBufferInfo;
} BMUXlib_AudioEncoderInterface;

#ifdef __cplusplus
}
#endif

#endif /* BMUXLIB_H_ */

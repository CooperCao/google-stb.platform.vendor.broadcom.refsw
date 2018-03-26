/***************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

typedef struct BMUXlib_CompressedBufferRegisters
{
   struct
   {
      uint32_t uiRead;  /* Address of the buffer READ register */
      uint32_t uiBase;  /* Address of the buffer BASE register */
      uint32_t uiValid; /* Address of the buffer VALID register */
      uint32_t uiEnd;   /* Address of the buffer END register */
   } stData,stIndex;
   bool bReady; /* Set to true when the buffer has been initialized and ready to be consumed.
                 * Once set to true, it is always true for the duration of the session. */
} BMUXlib_CompressedBufferRegisters;

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
      BMMA_Block_Handle hIndexBufferBlock; /* The BMMA block for the ITB. */
      BMUXlib_CompressedBufferRegisters stContext; /* The ITB/CDB context registers */
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

typedef BERR_Code
(*BMUXlib_ReadVideoIndex)(
   void *pvContext,
   BAVC_VideoBufferDescriptor *astDescriptors, /* Pointer to an array of descriptors. E.g. astDescriptors[0] is the first descriptor */
   unsigned uiNumDescriptorsMax, /* size of astDescriptors array */
   unsigned *puiNumDescriptorsRead
   );

typedef struct BMUXlib_VideoEncoderInterface
{
      void *pContext;
      BMUXlib_GetVideoBufferDescriptors fGetBufferDescriptors;
      BMUXlib_ConsumeBufferDescriptors fConsumeBufferDescriptors;
      BMUXlib_GetBufferStatus fGetBufferStatus;
      BMUXlib_ReadVideoIndex fReadIndex;

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

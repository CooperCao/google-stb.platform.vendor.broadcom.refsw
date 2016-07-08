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

#ifndef BMUXLIB_INPUT_H_
#define BMUXLIB_INPUT_H_

#include "bmuxlib.h"

#ifdef __cplusplus
extern "C" {
#if 0
}
#endif
#endif

/* NOTE: MUX treats a physical offset of zero as "invalid" (since its extremely unlikely to
   occur in reality) as a way of tracking a physical offset that has not been set
   (and is still at "defaults") */
#define BMUXLIB_INPUT_INVALID_OFFSET   0

/*
** General descriptor accessor macros
*/
#define BMUXLIB_INPUT_DESCRIPTOR_TYPE(x)           ((x)->eType)
#define BMUXLIB_INPUT_DESCRIPTOR_FRAMESIZE(x)      ((x)->uiFrameSize)

#define BMUXLIB_INPUT_DESCRIPTOR_FLAGS(x)          ((x)->descriptor.pstCommon->uiFlags)
#define BMUXLIB_INPUT_DESCRIPTOR_OFFSET(x)         ((x)->descriptor.pstCommon->uiOffset)
#define BMUXLIB_INPUT_DESCRIPTOR_LENGTH(x)         ((x)->descriptor.pstCommon->uiLength)
#define BMUXLIB_INPUT_DESCRIPTOR_ORIGINAL_PTS(x)   ((x)->descriptor.pstCommon->uiOriginalPTS)
#define BMUXLIB_INPUT_DESCRIPTOR_PTS(x)            ((x)->descriptor.pstCommon->uiPTS)
#define BMUXLIB_INPUT_DESCRIPTOR_ESCR(x)           ((x)->descriptor.pstCommon->uiESCR)
#define BMUXLIB_INPUT_DESCRIPTOR_TICKS_PER_BIT(x)  ((x)->descriptor.pstCommon->uiTicksPerBit)
#define BMUXLIB_INPUT_DESCRIPTOR_SHR(x)            ((x)->descriptor.pstCommon->iSHR)
#define BMUXLIB_INPUT_DESCRIPTOR_STC_SNAPSHOT(x)   ((x)->descriptor.pstCommon->uiSTCSnapshot)

#define BMUXLIB_INPUT_DESCRIPTOR_BLOCK(x)          ((x)->hBlock)
#define BMUXLIB_INPUT_DESCRIPTOR_VIRTUAL_ADDRESS(_pBaseAddress, x) ((void*)(((uint8_t *) (_pBaseAddress)) + BMUXLIB_INPUT_DESCRIPTOR_OFFSET(x)))
/* NOTE:This macro ensures that if the base offset is "invalid" to begin with, then the
   resulting physical offset is also returned as "invalid" (for subsequent error checking) */
#define BMUXLIB_INPUT_DESCRIPTOR_PHYSICAL_OFFSET(_uiBaseOffset, x) (( BMUXLIB_INPUT_INVALID_OFFSET == (_uiBaseOffset) ) ? BMUXLIB_INPUT_INVALID_OFFSET : ((BSTD_DeviceOffset)((_uiBaseOffset) + BMUXLIB_INPUT_DESCRIPTOR_OFFSET(x))))

#define BMUXLIB_INPUT_DESCRIPTOR_IS_ESCR_VALID(x)  (0 != (((x)->descriptor.pstCommon->uiFlags) & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_ESCR_VALID))
#define BMUXLIB_INPUT_DESCRIPTOR_IS_PTS_VALID(x)   (0 != (((x)->descriptor.pstCommon->uiFlags) & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_PTS_VALID))
#define BMUXLIB_INPUT_DESCRIPTOR_IS_TICKS_PER_BIT_VALID(x)  \
         (0 != (((x)->descriptor.pstCommon->uiFlags) & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_TICKSPERBIT_VALID))
#define BMUXLIB_INPUT_DESCRIPTOR_IS_SHR_VALID(x)   (0 != (((x)->descriptor.pstCommon->uiFlags) & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_SHR_VALID))
#define BMUXLIB_INPUT_DESCRIPTOR_IS_OPTS_VALID(x)  (0 != (((x)->descriptor.pstCommon->uiFlags) & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_ORIGINALPTS_VALID))
#define BMUXLIB_INPUT_DESCRIPTOR_IS_STC_SNAPSHOT_VALID(x)   \
         (0 != (((x)->descriptor.pstCommon->uiFlags) & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_STCSNAPSHOT_VALID))

#define BMUXLIB_INPUT_DESCRIPTOR_IS_FRAMESTART(x)  (0 != (((x)->descriptor.pstCommon->uiFlags) & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_START))
#define BMUXLIB_INPUT_DESCRIPTOR_IS_EOS(x)         (0 != (((x)->descriptor.pstCommon->uiFlags) & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOS))
#define BMUXLIB_INPUT_DESCRIPTOR_IS_EMPTYFRAME(x)  (0 != (((x)->descriptor.pstCommon->uiFlags) & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EMPTY_FRAME))
#define BMUXLIB_INPUT_DESCRIPTOR_IS_FRAMEEND(x)    (0 != (((x)->descriptor.pstCommon->uiFlags) & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_END))
#define BMUXLIB_INPUT_DESCRIPTOR_IS_EOC(x)         (0 != (((x)->descriptor.pstCommon->uiFlags) & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOC))

#define BMUXLIB_INPUT_DESCRIPTOR_IS_METADATA(x)    (0 != (((x)->descriptor.pstCommon->uiFlags) & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_METADATA))

#define BMUXLIB_INPUT_DESCRIPTOR_IS_DTS_VALID(x) \
 ( (BMUXlib_Input_Type_eVideo == (x)->eType) ? \
         (0 != (((x)->descriptor.pstVideo->uiVideoFlags) & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DTS_VALID)) \
         : BMUXLIB_INPUT_DESCRIPTOR_IS_PTS_VALID(x) )

#define BMUXLIB_INPUT_DESCRIPTOR_IS_KEYFRAME(x)    \
 ( (BMUXlib_Input_Type_eVideo == (x)->eType) ? \
          (0 != (((x)->descriptor.pstVideo->uiVideoFlags) & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_RAP)) \
          : true )   /* audio frames are always RAP points */

#define BMUXLIB_INPUT_DESCRIPTOR_DTS(x) \
  ( (BMUXlib_Input_Type_eVideo == (x)->eType) ? \
         ((x)->descriptor.pstVideo->uiDTS) \
         : (x)->descriptor.pstCommon->uiPTS )

#define BMUXLIB_INPUT_DESCRIPTOR_VIDEO_IS_DATA_UNIT_START(x)   \
        (0 != (((x)->descriptor.pstVideo->uiVideoFlags) & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DATA_UNIT_START))

#define BMUXLIB_INPUT_DESCRIPTOR_VIDEO_FLAGS(x)          ((x)->descriptor.pstVideo->uiVideoFlags)
#define BMUXLIB_INPUT_DESCRIPTOR_VIDEO_DTS(x)            ((x)->descriptor.pstVideo->uiDTS)
#define BMUXLIB_INPUT_DESCRIPTOR_VIDEO_DATA_UNIT_TYPE(x) ((x)->descriptor.pstVideo->uiDataUnitType)

#define BMUXLIB_INPUT_DESCRIPTOR_AUDIO_RAWOFFSET(x)      ((x)->descriptor.pstAudio->uiRawDataOffset)
#define BMUXLIB_INPUT_DESCRIPTOR_AUDIO_RAWLENGTH(x)      ((x)->descriptor.pstAudio->uiRawDataLength)
#define BMUXLIB_INPUT_DESCRIPTOR_AUDIO_DATA_UNIT_TYPE(x) ((x)->descriptor.pstAudio->uiDataUnitType)

/*
** Input Types
*/
typedef enum BMUXlib_Input_Type
{
   BMUXlib_Input_Type_eVideo,
   BMUXlib_Input_Type_eAudio,

   BMUXlib_Input_Type_eMax
} BMUXlib_Input_Type;

typedef struct BMUXlib_Input_Descriptor
{
      BMUXlib_Input_Type eType;

      union
      {
            const BAVC_CompressedBufferDescriptor *pstCommon;
            const BAVC_VideoBufferDescriptor *pstVideo;
            const BAVC_AudioBufferDescriptor *pstAudio;
      } descriptor;

      BMMA_Block_Handle hBlock;
      size_t uiFrameSize;                 /* Only valid if BMUXlib_Input_CreateSettings.eBurstMode == BMUXlib_Input_BurstMode_eFrame */
} BMUXlib_Input_Descriptor;

typedef enum BMUXlib_Input_BurstMode
{
   BMUXlib_Input_BurstMode_eDescriptor,   /* Descriptors will be returned as soon as they are available.  Useful for low delay modes */
   BMUXlib_Input_BurstMode_eFrame,        /* Descriptors will be returned only when the entire frame is available.  Useful for optimizing output and/or modes where frame size is needed */

   BMUXlib_Input_BurstMode_eMax
} BMUXlib_Input_BurstMode;

typedef struct BMUXlib_Input_Status
{
   uint64_t uiInitialDTS;
   bool bInitialDTSValid;
   uint64_t uiCurrentDTS;
} BMUXlib_Input_Status;


typedef struct BMUXlib_Input_CreateSettings
{
   uint32_t uiSignature;                  /* [DO NOT MODIFY] Populated by BMUXlib_Input_GetDefaultCreateSettings() */

   BMUXlib_Input_Type eType;
   union
   {
      BMUXlib_VideoEncoderInterface stVideo;
      BMUXlib_AudioEncoderInterface stAudio;
   } interface;


   BMUXlib_Input_BurstMode eBurstMode;
   bool bFilterUntilMetadataSeen;         /* If true, all input descriptors will be filtered until the first
                                           * metadata descriptor is seen.  This is to handle scenarios where
                                           * the the input has stale data leftover from a previous encoder that
                                           * hasn't been properly consumed/flushed. */

   void *pMetadata;                       /* reference to usage-dependent metadata associated with this input */

   /* For debug: */
   unsigned uiMuxId;                      /* indicates the ID of the mux this input is associated with.
                                             E.g. for dual transcode, each mux "output" should have a different instance */
   unsigned uiTypeInstance;               /* indicates which input instance for this type that is accociated with the mux instance.
                                             E.g. for single transcode with multiple audio, each audio input should have a different type instance */
} BMUXlib_Input_CreateSettings;

typedef struct BMUXlib_Input_P_Context * BMUXlib_Input_Handle;

typedef struct
{
   uint32_t uiSignature;                              /* [DO NOT MODIFY] Populated by BMUXlib_Input_GetDefaultSettings() */

   bool bEnable;                                      /* if set, input is disabled and replaced with EMPTY_FRAME descriptors */
} BMUXlib_Input_Settings;

/*
** Input Group Types
*/
/* the following enum defines which of the two descriptors being compared
   is desired for input selection.
   The criteria for which input to select is determined by the implementation
   of the selector function.
   e.g. if the input with the lowest DTS is desired for selection, then the
   selector function would return "SelectA" if A.DTS < B.DTS
*/
typedef enum
{
   BMUXlib_InputGroup_SelectionResult_eSelectA, /* argument A is the desired entry */
   BMUXlib_InputGroup_SelectionResult_eSelectB  /* argument B is the desired entry */
} BMUXlib_InputGroup_SelectionResult;


/* function to choose between two input descriptors based on some
   implementation-dependent criteria */
typedef BMUXlib_InputGroup_SelectionResult (*BMUXlib_InputGroup_DescriptorSelector)(
   BMUXlib_Input_Descriptor *pstDescriptorA,
   BMUXlib_Input_Descriptor *pstDescriptorB
   );

typedef struct BMUXlib_InputGroup_CreateSettings
{
   uint32_t uiSignature;                  /* [DO NOT MODIFY] Populated by BMUXlib_InputGroup_GetDefaultCreateSettings() */

   uint32_t uiInputCount;                 /* number of inputs to add to the group */
   BMUXlib_Input_Handle *pInputTable;     /* table of handles for the inputs in the group */
} BMUXlib_InputGroup_CreateSettings;

typedef struct
{
   uint32_t uiSignature;                              /* [DO NOT MODIFY] Populated by BMUXlib_InputGroup_GetDefaultSettings() */

   bool bWaitForAllInputs;                            /* if set, input selection waits until all inputs have descriptors */
   BMUXlib_InputGroup_DescriptorSelector fSelector;   /* selector function used by GetNextInput() */
} BMUXlib_InputGroup_Settings;

typedef struct
{
   uint32_t uiNumActiveInputs;
   unsigned uiDuration; /* in milliseconds */
} BMUXlib_InputGroup_Status;

typedef struct BMUXlib_InputGroup_P_Context * BMUXlib_InputGroup_Handle;

/****************************
*   P R O T O T Y P E S     *
****************************/
/***************
   Input API
****************/
void
BMUXlib_Input_GetDefaultCreateSettings(
         BMUXlib_Input_CreateSettings *pstSettings
         );

BERR_Code
BMUXlib_Input_Create(
         BMUXlib_Input_Handle *phInput,
         const BMUXlib_Input_CreateSettings *pstSettings
         );

BERR_Code
BMUXlib_Input_GetCreateSettings(
         BMUXlib_Input_Handle hInput,
         BMUXlib_Input_CreateSettings *pstSettings
         );

BERR_Code
BMUXlib_Input_Destroy(
         BMUXlib_Input_Handle hInput
         );

void
BMUXlib_Input_GetDefaultSettings(
         BMUXlib_Input_Settings *pstSettings
         );

BERR_Code
BMUXlib_Input_SetSettings(
         BMUXlib_Input_Handle hInput,
         const BMUXlib_Input_Settings *pstSettings
         );

BERR_Code
BMUXlib_Input_GetSettings(
         BMUXlib_Input_Handle hInput,
         BMUXlib_Input_Settings *pstSettings
         );

BERR_Code
BMUXlib_Input_ProcessNewDescriptors(
         BMUXlib_Input_Handle hInput
         );

bool
BMUXlib_Input_IsDescriptorAvailable(
         BMUXlib_Input_Handle hInput
         );

bool
BMUXlib_Input_PeekAtNextDescriptor(
         BMUXlib_Input_Handle hInput,
         BMUXlib_Input_Descriptor *pstDescriptor
         );

bool
BMUXlib_Input_GetNextDescriptor(
         BMUXlib_Input_Handle hInput,
         BMUXlib_Input_Descriptor *pstDescriptor
         );

BERR_Code
BMUXlib_Input_ConsumeDescriptors(
         BMUXlib_Input_Handle hInput,
         size_t uiCount
         );

BERR_Code
BMUXlib_Input_GetStatus(
         BMUXlib_Input_Handle hInput,
         BMUXlib_Input_Status *pstStatus
         );


/**********************
   Input Group API
**********************/
void
BMUXlib_InputGroup_GetDefaultCreateSettings(
         BMUXlib_InputGroup_CreateSettings *pstSettings);

BERR_Code
BMUXlib_InputGroup_Create(
         BMUXlib_InputGroup_Handle *phInputGroup,
         const BMUXlib_InputGroup_CreateSettings *pstSettings
         );

void
BMUXlib_InputGroup_Destroy(
         BMUXlib_InputGroup_Handle hInputGroup
         );

void
BMUXlib_InputGroup_GetDefaultSettings(
         BMUXlib_InputGroup_Settings *pstSettings
         );

BERR_Code
BMUXlib_InputGroup_SetSettings(
         BMUXlib_InputGroup_Handle hInputGroup,
         const BMUXlib_InputGroup_Settings *pstSettings
         );

BERR_Code
BMUXlib_InputGroup_GetSettings(
         BMUXlib_InputGroup_Handle hInputGroup,
         BMUXlib_InputGroup_Settings *pstSettings
         );

BERR_Code
BMUXlib_InputGroup_ActivateInput(
         BMUXlib_InputGroup_Handle hInputGroup,
         BMUXlib_Input_Handle hInput,
         bool bActive
         );

BERR_Code
BMUXlib_InputGroup_GetStatus(
         BMUXlib_InputGroup_Handle hInputGroup,
         BMUXlib_InputGroup_Status *pstStatus
         );

/* select the next input to process based upon criteria defined by the
   supplied selector function.  The inputs are ranked based on the result
   of the selection between the available descriptor for the inputs, in order
   of decreasing "selection", and the handle of the input at the head of
   the list is returned (the item that was selected the most)
   Sets *phInput to NULL if no input ready to select (i.e. no descriptors available)
*/
void
BMUXlib_InputGroup_GetNextInput(
         BMUXlib_InputGroup_Handle hInputGroup,
         BMUXlib_Input_Handle *phInput
         );

/* pre-defined selection routines */
BMUXlib_InputGroup_SelectionResult
BMUXlib_InputGroup_DescriptorSelectLowestDTS (
         BMUXlib_Input_Descriptor *pstDescriptorA,
         BMUXlib_Input_Descriptor *pstDescriptorB
         );

BMUXlib_InputGroup_SelectionResult
BMUXlib_InputGroup_DescriptorSelectLowestESCR (
         BMUXlib_Input_Descriptor *pstDescriptorA,
         BMUXlib_Input_Descriptor *pstDescriptorB
         );

#ifdef __cplusplus
}
#endif

#endif /* BMUXLIB_INPUT_H_ */

/*****************************************************************************
* EOF
******************************************************************************/

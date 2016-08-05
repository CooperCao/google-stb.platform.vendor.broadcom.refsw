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


#ifndef BMUXLIB_FILE_IVF_PRIV_H__
#define BMUXLIB_FILE_IVF_PRIV_H__

/* Includes */
#include "bmuxlib_file_ivf.h"
#include "bmuxlib_input.h"

#ifdef BMUXLIB_IVF_P_TEST_MODE
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/****************************
*  D E F I N I T I O N S    *
****************************/

/* accessor macros to allow tests to manipulate mux state */
#define BMUXLIB_FILE_IVF_P_GET_MUX_STATE(handle)         ((handle)->stStatus.eState)
#define BMUXLIB_FILE_IVF_P_SET_MUX_STATE(handle, state)  ((handle)->stStatus.eState = (state))

#define BMUXlib_File_IVF_P_Set64_LE( pBuffer, offset, data ) \
{\
   ((uint8_t*)(pBuffer))[offset+0] = (uint8_t) (((uint64_t)(data) >> 0 ) & 0xFF); \
   ((uint8_t*)(pBuffer))[offset+1] = (uint8_t) (((uint64_t)(data) >> 8 ) & 0xFF); \
   ((uint8_t*)(pBuffer))[offset+2] = (uint8_t) (((uint64_t)(data) >> 16) & 0xFF); \
   ((uint8_t*)(pBuffer))[offset+3] = (uint8_t) (((uint64_t)(data) >> 24) & 0xFF); \
   ((uint8_t*)(pBuffer))[offset+4] = (uint8_t) (((uint64_t)(data) >> 32) & 0xFF); \
   ((uint8_t*)(pBuffer))[offset+5] = (uint8_t) (((uint64_t)(data) >> 40) & 0xFF); \
   ((uint8_t*)(pBuffer))[offset+6] = (uint8_t) (((uint64_t)(data) >> 48) & 0xFF); \
   ((uint8_t*)(pBuffer))[offset+7] = (uint8_t) (((uint64_t)(data) >> 56) & 0xFF); \
}

#define BMUXlib_File_IVF_P_Set32_LE( pBuffer, offset, data ) \
{\
   ((uint8_t*)(pBuffer))[offset+0] = (uint8_t) (((uint32_t)(data) >> 0 ) & 0xFF); \
   ((uint8_t*)(pBuffer))[offset+1] = (uint8_t) (((uint32_t)(data) >> 8 ) & 0xFF); \
   ((uint8_t*)(pBuffer))[offset+2] = (uint8_t) (((uint32_t)(data) >> 16) & 0xFF); \
   ((uint8_t*)(pBuffer))[offset+3] = (uint8_t) (((uint32_t)(data) >> 24) & 0xFF); \
}

#define BMUXlib_File_IVF_P_Set16_LE( pBuffer, offset, data ) \
{\
   ((uint8_t*)(pBuffer))[offset+0] = (uint8_t) (((uint16_t)(data) >> 0 ) & 0xFF); \
   ((uint8_t*)(pBuffer))[offset+1] = (uint8_t) (((uint16_t)(data) >> 8 ) & 0xFF); \
}

/********************
  Generic constants
********************/

/* 90kHz units for video duration, PTS, DTS, etc */
#define BMUXLIB_FILE_IVF_P_TIMESCALE_90KHZ      90000

/**************
   Signatures
***************/

#define BMUXLIB_FILE_IVF_P_SIGNATURE_CREATESETTINGS      0x49564601
#define BMUXLIB_FILE_IVF_P_SIGNATURE_STARTSETTINGS       0x49564602
#define BMUXLIB_FILE_IVF_P_SIGNATURE_FINISHSETTINGS      0x49564603

/****************************
*        T Y P E S          *
****************************/

typedef enum BMUXlib_File_IVF_P_InputState
{
   BMUXlib_File_IVF_P_InputState_eStartup,
   BMUXlib_File_IVF_P_InputState_eGenerateFileHeader,
   BMUXlib_File_IVF_P_InputState_eGetNextDescriptor,
   BMUXlib_File_IVF_P_InputState_eProcessMetadata,
   BMUXlib_File_IVF_P_InputState_eGenerateFrameHeader,
   BMUXlib_File_IVF_P_InputState_eProcessFrameData,
   BMUXlib_File_IVF_P_InputState_eProcessEOS,
   BMUXlib_File_IVF_P_InputState_eDone,

   BMUXlib_File_IVF_P_InputState_eMax
} BMUXlib_File_IVF_P_InputState;

/****************************
*    Context definition     *
****************************/

#define BMUXlib_File_IVF_P_MAX_FRAMES (64*8)
#define BMUXlib_File_IVF_P_FILE_HEADER_SIZE 32
#define BMUXlib_File_IVF_P_FRAME_HEADER_SIZE 12

#define BMUXlib_File_IVF_P_FileHeader_Signature_OFFSET 0
#define BMUXlib_File_IVF_P_FileHeader_Version_OFFSET 4
#define BMUXlib_File_IVF_P_FileHeader_HeaderLength_OFFSET 6
#define BMUXlib_File_IVF_P_FileHeader_FourCC_OFFSET 8
#define BMUXlib_File_IVF_P_FileHeader_Width_OFFSET 12
#define BMUXlib_File_IVF_P_FileHeader_Height_OFFSET 14
#define BMUXlib_File_IVF_P_FileHeader_FrameRate_OFFSET 16
#define BMUXlib_File_IVF_P_FileHeader_TimeScale_OFFSET 20
#define BMUXlib_File_IVF_P_FileHeader_FrameCount_OFFSET 24

typedef union BMUXlib_File_IVF_P_FileHeader
{
   uint8_t auiBytes[BMUXlib_File_IVF_P_FILE_HEADER_SIZE];
#if 0
   struct
   {
      uint32_t uiSignature; /* 0:3='DKIF' */
      uint32_t uiVersionAndHeaderLength; /* 4:5=version, 6:7=header length (bytes) */
      uint32_t uiFourCC; /* 8:11='VP80' */
      uint32_t uiWidthAndHeight; /* 12:13=width (pixels), 14:15=height (pixels)*/
      uint32_t uiFrameRate; /* 16-19: frame rate (???) */
      uint32_t uiTimeScale; /* 20-23: timescale (???) */
      uint32_t uiFrameCount; /* 24:27: frame count (frames) */
      uint32_t uiUnused; /* 28:31: unused */
   } stFields;
#endif
} BMUXlib_File_IVF_P_FileHeader;

#define BMUXlib_File_IVF_P_FrameHeader_FrameSize_OFFSET 0
#define BMUXlib_File_IVF_P_FrameHeader_PTS_OFFSET 4

typedef union BMUXlib_File_IVF_P_FrameHeader
{
   uint8_t auiBytes[BMUXlib_File_IVF_P_FRAME_HEADER_SIZE];
#if 0
   struct
   {
      uint32_t uiFrameSize; /* 0:3=frame size (bytes) not including 12-byte header */
      uint64_t uiPTS; /* 4:11=presentation time stamp */
   } stFields;
#endif
} BMUXlib_File_IVF_P_FrameHeader;

typedef struct BMUXlib_File_IVF_P_Context
{
   BDBG_OBJECT(BMUXlib_File_IVF_P_Context)

   BMUXlib_File_IVF_StartSettings stStartSettings;
   BMUXlib_Input_Handle hInput;
   unsigned uiFrameCount;
   unsigned uiPendingCount;
   unsigned uiCompletedCount;

   BMUXlib_Output_Handle hOutput;
   BMUXlib_DoMux_Status stStatus;

   BMUXlib_File_IVF_P_InputState eInputState;

   BMUXlib_File_IVF_P_FileHeader stFileHeader;

   struct
   {
      BMUXlib_File_IVF_P_FrameHeader astFrameHeader[BMUXlib_File_IVF_P_MAX_FRAMES];
      unsigned uiReadOffset;
      unsigned uiWriteOffset;
   } stFrameHeader;

} BMUXlib_File_IVF_P_Context;

/****************************
*    P R O T O T Y P E S    *
****************************/

BERR_Code BMUXlib_File_IVF_P_Start(BMUXlib_File_IVF_Handle hIVFMux);
void BMUXlib_File_IVF_P_Stop(BMUXlib_File_IVF_Handle hIVFMux);

BERR_Code BMUXlib_File_IVF_P_ProcessInputDescriptors( BMUXlib_File_IVF_Handle hIVFMux );

BERR_Code BMUXlib_File_IVF_P_ProcessOutputDescriptorsCompleted(BMUXlib_File_IVF_Handle hIVFMux);
BERR_Code BMUXlib_File_IVF_P_ProcessOutputDescriptorsWaiting(BMUXlib_File_IVF_Handle hIVFMux);

bool BMUXlib_File_IVF_P_EOSSeen( BMUXlib_File_IVF_Handle hIVFMux );
bool BMUXlib_File_IVF_P_IsInputProcessingDone( BMUXlib_File_IVF_Handle hIVFMux );
bool BMUXlib_File_IVF_P_IsOutputProcessingDone( BMUXlib_File_IVF_Handle hIVFMux );

void      BMUXlib_File_IVF_P_WriteU64LE(uint64_t *puiDest, uint64_t uiData);

#ifdef __cplusplus
}
#endif


#endif /* BMUXLIB_FILE_IVF_PRIV_H__ */

/*****************************************************************************
* EOF
******************************************************************************/

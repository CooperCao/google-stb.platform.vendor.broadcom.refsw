/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

 ******************************************************************************/


#ifndef BMUXLIB_FILE_PES_PRIV_H__
#define BMUXLIB_FILE_PES_PRIV_H__

/* Includes */
#include "bmuxlib_file_pes.h"
#include "bmuxlib_input.h"

#ifdef BMUXLIB_PES_P_TEST_MODE
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/****************************
*  D E F I N I T I O N S    *
****************************/

/* accessor macros to allow tests to manipulate mux state */
#define BMUXLIB_FILE_PES_P_GET_MUX_STATE(handle)         ((handle)->stStatus.eState)
#define BMUXLIB_FILE_PES_P_SET_MUX_STATE(handle, state)  ((handle)->stStatus.eState = (state))

#define BMUXlib_File_PES_P_Set32_BE( pBuffer, offset, data ) \
{\
   ((uint8_t*)(pBuffer))[offset+0] = (uint8_t) (((uint32_t)(data) >> 24 ) & 0xFF); \
   ((uint8_t*)(pBuffer))[offset+1] = (uint8_t) (((uint32_t)(data) >> 16 ) & 0xFF); \
   ((uint8_t*)(pBuffer))[offset+2] = (uint8_t) (((uint32_t)(data) >> 8  ) & 0xFF); \
   ((uint8_t*)(pBuffer))[offset+3] = (uint8_t) (((uint32_t)(data) >> 0  ) & 0xFF); \
}

#define BMUXlib_File_PES_P_Set16_BE( pBuffer, offset, data ) \
{\
   ((uint8_t*)(pBuffer))[offset+0] = (uint8_t) (((uint16_t)(data) >> 8 ) & 0xFF); \
   ((uint8_t*)(pBuffer))[offset+1] = (uint8_t) (((uint16_t)(data) >> 0 ) & 0xFF); \
}

/********************
  Generic constants
********************/

/**************
   Signatures
***************/

#define BMUXLIB_FILE_PES_P_SIGNATURE_CREATESETTINGS      0x50455301
#define BMUXLIB_FILE_PES_P_SIGNATURE_STARTSETTINGS       0x50455302
#define BMUXLIB_FILE_PES_P_SIGNATURE_FINISHSETTINGS      0x50455303

/****************************
*        T Y P E S          *
****************************/

typedef enum BMUXlib_File_PES_P_InputState
{
   BMUXlib_File_PES_P_InputState_eStartup,
   BMUXlib_File_PES_P_InputState_eGetNextDescriptor,
   BMUXlib_File_PES_P_InputState_eProcessMetadata,
   BMUXlib_File_PES_P_InputState_eGenerateFrameHeader,
   BMUXlib_File_PES_P_InputState_eGenerateVP8Header,
   BMUXlib_File_PES_P_InputState_eGeneratePESHeader,
   BMUXlib_File_PES_P_InputState_eProcessFrameData,
   BMUXlib_File_PES_P_InputState_eProcessEOS,
   BMUXlib_File_PES_P_InputState_eDone,

   BMUXlib_File_PES_P_InputState_eMax
} BMUXlib_File_PES_P_InputState;

/****************************
*    Context definition     *
****************************/

#define BMUXlib_File_PES_P_MAX_PES_LENGTH (0xFFFF)
#define BMUXlib_File_PES_P_MAX_FRAMES (64*8)
#define BMUXlib_File_PES_P_FRAME_HEADER_SIZE 19

#define BMUXlib_File_PES_P_FrameHeader_FrameSize_OFFSET 4

#define BMUXlib_File_PES_P_PESHeader_SetPTS(_pesHeader, _pts) \
         _pesHeader[13] = ( ( ( _pts << 1  ) & 0xFE ) | (0x01) ); \
         _pesHeader[12] = (   ( _pts >> 7  ) & 0xFF ); \
         _pesHeader[11] = ( ( ( _pts >> 14 ) & 0xFE ) | (0x01) ); \
         _pesHeader[10] = (   ( _pts >> 22 ) & 0xFF ); \
         _pesHeader[9] =  ( ( ( _pts >> 29 ) & 0x0E ) | (0x21) ); \
         _pesHeader[7] |= 0x80;

#define BMUXlib_File_PES_P_PESHeader_SetDTS(_pesHeader, _pts) \
         _pesHeader[18] = ( ( ( _pts << 1  ) & 0xFE ) | (0x01) ); \
         _pesHeader[17] = (   ( _pts >> 7  ) & 0xFF ); \
         _pesHeader[16] = ( ( ( _pts >> 14 ) & 0xFE ) | (0x01) ); \
         _pesHeader[15] = (   ( _pts >> 22 ) & 0xFF ); \
         _pesHeader[14] = ( ( ( _pts >> 29 ) & 0x0E ) | (0x11) ); \
         _pesHeader[9] |= (0x30); \
         _pesHeader[7] |= 0x40;

typedef union BMUXlib_File_PES_P_FrameHeader
{
   uint8_t auiBytes[BMUXlib_File_PES_P_FRAME_HEADER_SIZE];
#if 0
      struct {
            uint32_t uiStreamID:8;
            uint32_t uiStartCode:24; /* 000001h */

            uint32_t bPESExtensionFlag:1;
            uint32_t bPESCRCFlag:1;
            uint32_t bAdditionalCopyInfoFlag:1;
            uint32_t bDSMTrickModeFlag:1;
            uint32_t bESRateFlag:1;
            uint32_t bESCRFlag:1;
            uint32_t bDtsFlag:1;
            uint32_t bPtsFlag:1;
            uint32_t bOriginalFlag:1;
            uint32_t bCopyrightFlag:1;
            uint32_t bDataAlignmentIndicator:1;
            uint32_t bPESPriority:1;
            uint32_t uiPESScramblingControl:2;
            uint32_t uiExtensionCode:2; /* Always 10b */
            uint32_t uiPESPacketLength:16;

            uint8_t uiPESHeaderDataLength;

            uint32_t
      } fields;
#endif
} BMUXlib_File_PES_P_FrameHeader;

#define BMUXlib_File_PES_P_VP8_HEADER_SIZE 10

#define BMUXlib_File_PES_P_VP8Header_FrameSize_OFFSET 4

typedef union BMUXlib_File_PES_P_VP8Header
{
   uint8_t auiBytes[BMUXlib_File_PES_P_VP8_HEADER_SIZE];
} BMUXlib_File_PES_P_VP8Header;

typedef struct BMUXlib_File_PES_P_Context
{
   BDBG_OBJECT(BMUXlib_File_PES_P_Context)

   BMUXlib_File_PES_StartSettings stStartSettings;
   BMUXlib_Input_Handle hInput;
   unsigned uiFrameCount;
   unsigned uiPendingCount;
   unsigned uiCompletedCount;

   BMUXlib_Output_Handle hOutput;
   BMUXlib_DoMux_Status stStatus;

   BMUXlib_File_PES_P_InputState eInputState;

   struct
   {
      BMUXlib_File_PES_P_FrameHeader astFrameHeader[BMUXlib_File_PES_P_MAX_FRAMES];
      unsigned uiReadOffset;
      unsigned uiWriteOffset;
   } stFrameHeader;
   unsigned uiFrameSize;
   unsigned uiBytesLeftInPES;
   unsigned uiBytesLeftInFrame;
   unsigned uiBytesLeftInDescriptor;

   struct
   {
      BMUXlib_File_PES_P_VP8Header astFrameHeader[BMUXlib_File_PES_P_MAX_FRAMES];
      unsigned uiReadOffset;
      unsigned uiWriteOffset;
   } stVP8Header;

   BAVC_VideoCompressionStd eProtocol;
} BMUXlib_File_PES_P_Context;

/****************************
*    P R O T O T Y P E S    *
****************************/

BERR_Code BMUXlib_File_PES_P_Start(BMUXlib_File_PES_Handle hPESMux);
void BMUXlib_File_PES_P_Stop(BMUXlib_File_PES_Handle hPESMux);

BERR_Code BMUXlib_File_PES_P_ProcessInputDescriptors( BMUXlib_File_PES_Handle hPESMux );

BERR_Code BMUXlib_File_PES_P_ProcessOutputDescriptorsCompleted(BMUXlib_File_PES_Handle hPESMux);
BERR_Code BMUXlib_File_PES_P_ProcessOutputDescriptorsWaiting(BMUXlib_File_PES_Handle hPESMux);

bool BMUXlib_File_PES_P_EOSSeen( BMUXlib_File_PES_Handle hPESMux );
bool BMUXlib_File_PES_P_IsInputProcessingDone( BMUXlib_File_PES_Handle hPESMux );
bool BMUXlib_File_PES_P_IsOutputProcessingDone( BMUXlib_File_PES_Handle hPESMux );

#ifdef __cplusplus
}
#endif


#endif /* BMUXLIB_FILE_PES_PRIV_H__ */

/*****************************************************************************
* EOF
******************************************************************************/

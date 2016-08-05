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

#ifndef BMUXLIB_OUTPUT_H_
#define BMUXLIB_OUTPUT_H_

#include "bmma.h"

#include "bmuxlib_file.h"

#ifdef __cplusplus
extern "C" {
#if 0
}
#endif
#endif

typedef enum
{
   BMUXlib_Output_OffsetReference_eStart = 0,   /* absolute offset (default) */
   BMUXlib_Output_OffsetReference_eEnd,         /* location relative to the end, i.e. append data to the end */
   BMUXlib_Output_OffsetReference_eCurrent      /* location to access is relative to current location */
} BMUXlib_Output_OffsetReference;

/* The following is the descriptors provided to the output module by the mux */
typedef struct
{
   bool bWriteOperation;            /* false = read operation */
   void *pBufferAddress;            /* pointer to the data to write, or buffer to read into. If null, hBlock must be non-null */
   BMMA_Block_Handle hBlock;        /* handle to block containing data to be read/written */
   size_t uiBlockOffset;            /* offset from start of block of the data to be read/written */
   /* NOTE: a zero length *is* supported and must NOT alter the storage (a NOP) */
   size_t uiLength;                 /* length in Bytes of the data to read/write  */
   uint64_t uiOffset;               /* the offset within the output stream to read from/write to. */
   BMUXlib_Output_OffsetReference eOffsetFrom;  /* position offset is relative to */
} BMUXlib_Output_StorageDescriptor;

typedef struct BMUXlib_Output_Context *BMUXlib_Output_Handle;

typedef union BMUXlib_Output_Descriptor
{
      BMUXlib_Output_StorageDescriptor stStorage;
} BMUXlib_Output_Descriptor;

typedef void (*BMUXlib_Output_CallbackFunction)( void *pPrivateData, const BMUXlib_Output_Descriptor *pOutputDescriptor );

typedef struct BMUXlib_Output_CompletedCallbackInfo
{
      void *pCallbackData;
      BMUXlib_Output_CallbackFunction pCallback;
} BMUXlib_Output_CompletedCallbackInfo;

typedef struct BMUXlib_Output_CreateSettings
{
      uint32_t uiSignature;         /* [DO NOT MODIFY] Populated by BMUXlib_Output_GetDefaultCreateSettings() */

      size_t uiCount;
      BMUXlib_StorageObjectInterface stStorage;
      uint32_t uiOutputID;          /* used to uniquely identify this output when multiple outputs are used */
} BMUXlib_Output_CreateSettings;

/****************************
*   P R O T O T Y P E S     *
****************************/
void
BMUXlib_Output_GetDefaultCreateSettings(
         BMUXlib_Output_CreateSettings *pstSettings
         );

BERR_Code
BMUXlib_Output_Create(
         BMUXlib_Output_Handle *phOutput,
         const BMUXlib_Output_CreateSettings *pstSettings
         );

BERR_Code
BMUXlib_Output_Destroy(
         BMUXlib_Output_Handle hOutput
         );

bool
BMUXlib_Output_IsSpaceAvailable(
         BMUXlib_Output_Handle hOutput
         );

bool
BMUXlib_Output_IsDescriptorPendingCompletion(
         BMUXlib_Output_Handle hOutput
         );

bool
BMUXlib_Output_IsDescriptorPendingQueue(
         BMUXlib_Output_Handle hOutput
         );

BERR_Code
BMUXlib_Output_AddNewDescriptor(
         BMUXlib_Output_Handle hOutput,
         BMUXlib_Output_Descriptor *pstDescriptor,
         BMUXlib_Output_CompletedCallbackInfo *pstCompletedCallbackInfo
         );

BERR_Code
BMUXlib_Output_ProcessNewDescriptors(
         BMUXlib_Output_Handle hOutput
         );

BERR_Code
BMUXlib_Output_ProcessCompletedDescriptors(
         BMUXlib_Output_Handle hOutput
         );

uint64_t
BMUXlib_Output_GetCurrentOffset(
         BMUXlib_Output_Handle hOutput
         );

uint64_t
BMUXlib_Output_GetEndOffset(
         BMUXlib_Output_Handle hOutput
         );

BERR_Code
BMUXlib_Output_SetCurrentOffset(
         BMUXlib_Output_Handle hOutput,
         uint64_t uiOffset,
         BMUXlib_Output_OffsetReference eOffsetFrom
         );

#ifdef __cplusplus
}
#endif

#endif /* BMUXLIB_OUTPUT_H_ */

/*****************************************************************************
* EOF
******************************************************************************/

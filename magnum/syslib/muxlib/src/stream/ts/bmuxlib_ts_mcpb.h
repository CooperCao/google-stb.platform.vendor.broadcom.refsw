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

#ifndef BMUXLIB_TS_MCPB_H_
#define BMUXLIB_TS_MCPB_H_

#include "bmuxlib_ts.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************/
/* Device Level Functions */
/**************************/

typedef struct BMUXlib_TS_MCPB_P_Context *BMUXlib_TS_MCPB_Handle;

typedef struct BMUXlib_TS_MCPB_CreateSettings
{
      uint32_t uiSignature; /* [DO NOT MODIFY] Populated by BMUXlib_TS_MCPB_GetDefaultCreateSettings() */

      BMMA_Heap_Handle hMma; /* Required. Must be accessible by the host. */

      struct
      {
         BMMA_Block_Handle hBlock;
         size_t uiSize;
      } stMuxSharedMemory;

      unsigned uiMaxNumInputs; /* uiMaxNumInputs indicates the maximum number of input transport channels
                                * that are expected to be opened via BMUXlib_TS_MCPB_Channel_Open
                                */

      BMUXlib_TS_TransportChannelInterface stOutputChannelInterface; /* The MCPB transport output interface */
} BMUXlib_TS_MCPB_CreateSettings;

void
BMUXlib_TS_MCPB_GetDefaultCreateSettings(
         BMUXlib_TS_MCPB_CreateSettings *pstSettings
         );

/* BMUXlib_TS_MCPB_Create - Allocates all system/device memory required for mux operation */
BERR_Code
BMUXlib_TS_MCPB_Create(
         BMUXlib_TS_MCPB_Handle *phMuxMCPB,  /* [out] TSMuxer handle returned */
         const BMUXlib_TS_MCPB_CreateSettings *pstSettings
         );

/* BMUXlib_TS_MCPB_Destroy - Frees all system/device memory allocated */
void
BMUXlib_TS_MCPB_Destroy(
         BMUXlib_TS_MCPB_Handle hMuxMCPB
         );

BERR_Code
BMUXlib_TS_MCPB_DoMux(
         BMUXlib_TS_MCPB_Handle hMuxMCPB,
         unsigned uiEndESCR
         );

/***************************/
/* Channel Level Functions */
/***************************/
typedef struct BMUXlib_TS_MCPB_P_Channel_Context *BMUXlib_TS_MCPB_Channel_Handle;

typedef struct BMUXlib_TS_MCPB_Channel_OpenSettings
{
   uint32_t uiSignature; /* [DO NOT MODIFY] Populated by BMUXlib_TS_MCPB_Channel_GetDefaultOpenSettings() */

   unsigned uiInstance; /* The Channel Number to Open */

   uint16_t uiPID;
   bool bIsTS;
   unsigned uiPIDChannelIndex;
} BMUXlib_TS_MCPB_Channel_OpenSettings;

void
BMUXlib_TS_MCPB_Channel_GetDefaultOpenSettings(
   BMUXlib_TS_MCPB_Channel_OpenSettings *pstSettings
         );

BERR_Code
BMUXlib_TS_MCPB_Channel_Open(
   BMUXlib_TS_MCPB_Handle hMuxMCPB,
   BMUXlib_TS_MCPB_Channel_Handle *phMuxMCPBCh,
   const BMUXlib_TS_MCPB_Channel_OpenSettings *pstSettings
   );

BERR_Code
BMUXlib_TS_MCPB_Channel_AddTransportDescriptors(
   BMUXlib_TS_MCPB_Channel_Handle hMuxMCPBCh,
   const BMUXlib_TS_TransportDescriptor *astTransportDescriptors, /* Array of pointers to transport descriptors */
   size_t uiCount, /* Count of descriptors in array */
   size_t *puiQueuedCount /* Count of descriptors queued (*puiQueuedCount <= uiCount) */
   );

BERR_Code
BMUXlib_TS_MCPB_Channel_GetCompletedTransportDescriptors(
   BMUXlib_TS_MCPB_Channel_Handle hMuxMCPBCh,
   size_t *puiCompletedCount /* Count of descriptors completed */
   );

void
BMUXlib_TS_MCPB_Channel_Close(
   BMUXlib_TS_MCPB_Channel_Handle hMuxMCPBCh
   );

#ifdef __cplusplus
}
#endif

#endif /* BMUXLIB_TS_MCPB_H_ */

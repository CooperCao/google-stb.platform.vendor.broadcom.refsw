/******************************************************************************
* Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
******************************************************************************/

/* base modules */
#include "bstd.h"           /* standard types */
#include "berr.h"           /* error code */
#include "bdbg.h"           /* debug interface */
#include "bkni.h"           /* kernel interface */

#include "bvce.h"
#include "bvce_err.h"
#include "bvce_priv.h"
#include "bvce_platform.h"

#include "bvce_debug_priv.h"

#include "bvce_output.h"
#include "bvce_power.h"
#include "bvce_buffer.h"

BDBG_MODULE(BVCE_OUTPUT);
BDBG_FILE_MODULE(BVCE_OUTPUT_ITB);
BDBG_FILE_MODULE(BVCE_OUTPUT_ITB_INDEX);
BDBG_FILE_MODULE(BVCE_OUTPUT_DESC);
BDBG_FILE_MODULE(BVCE_OUTPUT_CACHE);
BDBG_FILE_MODULE(BVCE_OUTPUT_PARSER);

BDBG_OBJECT_ID_DECLARE(BVCE_P_Output_Context);    /* BVCE_Output_Handle */
BDBG_OBJECT_ID_DECLARE(BVCE_P_Channel_Context);   /* BVCE_Channel_Handle */
BDBG_OBJECT_ID_DECLARE(BVCE_P_Context);           /* BVCE_Handle */

/**********/
/* Output */
/**********/

void
BVCE_Output_GetDefaultAllocBuffersSettings(
         const BBOX_Handle hBox,
         BVCE_Output_AllocBuffersSettings *pstOutputAllocBuffersSettings
   )
{
   BDBG_ENTER( BVCE_Output_GetDefaultAllocBuffersSettings );

   BDBG_ASSERT( pstOutputAllocBuffersSettings );

   BSTD_UNUSED( hBox );
   /* SW7445-1122: TODO: Update CDB/ITB sizing based on box modes.  Need function from FW team */

   BKNI_Memset(
            pstOutputAllocBuffersSettings,
            0,
            sizeof( BVCE_Output_AllocBuffersSettings)
            );

   pstOutputAllocBuffersSettings->uiSignature = BVCE_P_SIGNATURE_ALLOCBUFFERSSETTINGS;
   pstOutputAllocBuffersSettings->stConfig.Cdb.Length = BVCE_P_DEFAULT_CDB_SIZE;
   pstOutputAllocBuffersSettings->stConfig.Cdb.Alignment = BVCE_P_DEFAULT_CDB_ALIGNMENT;
   pstOutputAllocBuffersSettings->stConfig.Itb.Length = BVCE_P_DEFAULT_ITB_SIZE;
   pstOutputAllocBuffersSettings->stConfig.Itb.Alignment = BVCE_P_DEFAULT_ITB_ALIGNMENT;

   BDBG_LEAVE( BVCE_Output_GetDefaultAllocBuffersSettings );

   return;
}

/* BVCE_Output_AllocBuffer - Allocates ITB/CDB data (in *addition* to any memory allocated in BVCE_Open) */
BERR_Code
BVCE_Output_AllocBuffers(
         BVCE_Handle hVce,
         BVCE_OutputBuffers_Handle *phVceOutputBuffers,
         const BVCE_Output_AllocBuffersSettings *pstOutputAllocBuffersSettings /* [in] VCE Output Alloc Buffer settings */
         )
{
   BERR_Code rc;
   BVCE_OutputBuffers_Handle hVceOutputBuffers = NULL;

   BDBG_ENTER( BVCE_Output_AllocBuffers );

   BDBG_OBJECT_ASSERT(hVce, BVCE_P_Context);
   BDBG_ASSERT( phVceOutputBuffers );
   BDBG_ASSERT( pstOutputAllocBuffersSettings );
   BDBG_ASSERT( BVCE_P_SIGNATURE_ALLOCBUFFERSSETTINGS == pstOutputAllocBuffersSettings->uiSignature );

   /* Allocate Output Context */
   hVceOutputBuffers = ( BVCE_OutputBuffers_Handle ) BKNI_Malloc( sizeof( BVCE_P_OutputBuffers ) );
   if ( NULL == hVceOutputBuffers )
   {
      BDBG_ERR( ("Error allocating output buffer context"));
      return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
   }
   BKNI_Memset(
            hVceOutputBuffers,
            0,
            sizeof( BVCE_P_OutputBuffers )
            );

   /* Copy Settings */
   hVceOutputBuffers->stSettings = *pstOutputAllocBuffersSettings;

   if ( 0 != ( hVceOutputBuffers->stSettings.stConfig.Cdb.Length % 32 ) )
   {
      BDBG_ERR( ("CDB Length must be a multiple of 32"));
      BVCE_Output_FreeBuffers( hVceOutputBuffers );
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   if ( 0 != ( hVceOutputBuffers->stSettings.stConfig.Itb.Length % BVCE_PLATFORM_P_ITB_ALIGNMENT ) )
   {
      BDBG_ERR( ("ITB Length must be a multiple of %d", BVCE_PLATFORM_P_ITB_ALIGNMENT));
      BVCE_Output_FreeBuffers( hVceOutputBuffers );
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   /* Setup Allocators */
   rc = BVCE_P_Allocator_Create(
      ( NULL != hVceOutputBuffers->stSettings.hCDBMem ) ? hVceOutputBuffers->stSettings.hCDBMem : ( NULL != hVce->stOpenSettings.hSecureMem ) ? hVce->stOpenSettings.hSecureMem : hVce->handles.hMem,
      NULL,
      &hVceOutputBuffers->stCDB.hAllocator
      );

   if ( BERR_SUCCESS != rc )
   {
      BDBG_ERR( ("Error creating CDB allocator"));
      BVCE_Output_FreeBuffers( hVceOutputBuffers );
      return BERR_TRACE( rc );
      }

   rc = BVCE_P_Allocator_Create(
      ( NULL != hVceOutputBuffers->stSettings.hITBMem ) ? hVceOutputBuffers->stSettings.hITBMem : ( NULL != hVce->stOpenSettings.hSecureMem ) ? hVce->stOpenSettings.hSecureMem : hVce->handles.hMem,
      NULL,
      &hVceOutputBuffers->stITB.hAllocator
      );

   if ( BERR_SUCCESS != rc )
   {
      BDBG_ERR( ("Error creating ITB allocator"));
      BVCE_Output_FreeBuffers( hVceOutputBuffers );
      return BERR_TRACE( rc );
   }

   /* Allocate memory */
   if ( 0 != hVceOutputBuffers->stSettings.stConfig.Cdb.Length )
   {
      BVCE_P_Buffer_AllocSettings stAllocSettings;
      BVCE_P_Buffer_GetDefaultAllocSettings( &stAllocSettings );
      stAllocSettings.uiSize = hVceOutputBuffers->stSettings.stConfig.Cdb.Length;
      stAllocSettings.uiAlignment = hVceOutputBuffers->stSettings.stConfig.Cdb.Alignment;
      stAllocSettings.bLockOffset = true;

      rc = BVCE_P_Buffer_Alloc(
         hVceOutputBuffers->stCDB.hAllocator,
         &stAllocSettings,
         &hVceOutputBuffers->stCDB.hBuffer
         );

      if ( BERR_SUCCESS != rc )
      {
         BDBG_ERR( ("Error allocating CDB memory buffer"));
         BVCE_Output_FreeBuffers( hVceOutputBuffers );
         return BERR_TRACE( rc );
      }

#ifndef BVCE_PLATFORM_P_SUPPORTS_ITB_NAL
      /* Zero Out Buffer */
      {
         /* Get cached address */
         void *pBufferCached;

         pBufferCached = BVCE_P_Buffer_LockAddress( hVceOutputBuffers->stCDB.hBuffer );

         if ( NULL == pBufferCached )
         {
            BDBG_ERR( ("Error converting getting CDB buffer cached address"));
            BVCE_Output_FreeBuffers( hVceOutputBuffers );
            return BERR_TRACE( BERR_UNKNOWN );
         }

         BKNI_Memset(
            pBufferCached,
            0,
            hVceOutputBuffers->stSettings.stConfig.Cdb.Length
         );

         BVCE_P_Buffer_FlushCache_isr(
            hVceOutputBuffers->stCDB.hBuffer,
            pBufferCached,
            hVceOutputBuffers->stSettings.stConfig.Cdb.Length
         );

         BVCE_P_Buffer_UnlockAddress( hVceOutputBuffers->stCDB.hBuffer );
      }
#endif
   }

   if ( 0 != hVceOutputBuffers->stSettings.stConfig.Itb.Length )
   {
      BVCE_P_Buffer_AllocSettings stAllocSettings;
      BVCE_P_Buffer_GetDefaultAllocSettings( &stAllocSettings );
      stAllocSettings.uiSize = hVceOutputBuffers->stSettings.stConfig.Itb.Length;
      stAllocSettings.uiAlignment = hVceOutputBuffers->stSettings.stConfig.Itb.Alignment;
      stAllocSettings.bLockOffset = true;

      rc = BVCE_P_Buffer_Alloc(
         hVceOutputBuffers->stITB.hAllocator,
         &stAllocSettings,
         &hVceOutputBuffers->stITB.hBuffer
         );

      if ( BERR_SUCCESS != rc )
      {
         BDBG_ERR( ("Error allocating ITB memory buffer"));
         BVCE_Output_FreeBuffers( hVceOutputBuffers );
         return BERR_TRACE( rc );
      }

      /* Zero Out Buffer */
      {
         /* Get cached address */
         void *pBufferCached;

         pBufferCached = BVCE_P_Buffer_LockAddress( hVceOutputBuffers->stITB.hBuffer );

         if ( NULL == pBufferCached )
         {
            BDBG_ERR( ("Error converting getting ITB buffer cached address"));
            BVCE_Output_FreeBuffers( hVceOutputBuffers );
            return BERR_TRACE( BERR_UNKNOWN );
         }

         BKNI_Memset(
            pBufferCached,
            0,
            hVceOutputBuffers->stSettings.stConfig.Itb.Length
         );

         BVCE_P_Buffer_FlushCache_isr(
            hVceOutputBuffers->stITB.hBuffer,
            pBufferCached,
            hVceOutputBuffers->stSettings.stConfig.Itb.Length
         );

         BVCE_P_Buffer_UnlockAddress( hVceOutputBuffers->stITB.hBuffer );
      }
   }

   *phVceOutputBuffers = hVceOutputBuffers;

   BDBG_LEAVE( BVCE_Output_AllocBuffers );
   return BERR_TRACE( BERR_SUCCESS );
}

/* Should only be called when the buffer is not attached to a Output context */
BERR_Code
BVCE_Output_FreeBuffers(
         BVCE_OutputBuffers_Handle hVceOutputBuffers
         )
{
   BDBG_ENTER( BVCE_Output_FreeBuffers );

   if ( NULL != hVceOutputBuffers )
   {
      BVCE_P_Buffer_Free( hVceOutputBuffers->stITB.hBuffer );
      BVCE_P_Buffer_Free( hVceOutputBuffers->stCDB.hBuffer );
      BVCE_P_Allocator_Destroy( hVceOutputBuffers->stITB.hAllocator );
      BVCE_P_Allocator_Destroy( hVceOutputBuffers->stCDB.hAllocator );

      BKNI_Free( hVceOutputBuffers );
   }

   BDBG_LEAVE( BVCE_Output_FreeBuffers );

   return BERR_TRACE( BERR_SUCCESS );
}

void
BVCE_Output_GetDefaultOpenSettings(
         const BBOX_Handle hBox,
         BVCE_Output_OpenSettings *pstOutputOpenSettings
   )
{
   BDBG_ENTER( BVCE_Output_GetDefaultOpenSettings );

   BDBG_ASSERT( pstOutputOpenSettings );
   BSTD_UNUSED( hBox );

   BKNI_Memset(
            pstOutputOpenSettings,
            0,
            sizeof( BVCE_Output_OpenSettings )
            );

   pstOutputOpenSettings->bEnableDataUnitDetection = true;

   pstOutputOpenSettings->uiSignature = BVCE_P_SIGNATURE_OUTPUTOPENSETTINGS;

   pstOutputOpenSettings->uiDescriptorQueueDepth = BVCE_P_MAX_VIDEODESCRIPTORS;

   BDBG_LEAVE( BVCE_Output_GetDefaultOpenSettings );

   return;
}

BERR_Code
BVCE_Output_Open(
   BVCE_Handle hVce,
   BVCE_Output_Handle *phVceOutput,
   const BVCE_Output_OpenSettings *pstOutputOpenSettings /* [in] VCE Output settings */
   )
{
   BERR_Code rc;
   BVCE_Output_Handle hVceOutput;

   BDBG_ENTER( BVCE_Output_Open );

   BDBG_OBJECT_ASSERT(hVce, BVCE_P_Context);
   BDBG_ASSERT( phVceOutput );
   BDBG_ASSERT( pstOutputOpenSettings );
   BDBG_ASSERT( BVCE_P_SIGNATURE_OUTPUTOPENSETTINGS == pstOutputOpenSettings->uiSignature );
   BVCE_P_FUNCTION_TRACE_ENTER(0, hVce, pstOutputOpenSettings->uiInstance);

   if ( pstOutputOpenSettings->uiInstance >= BVCE_PLATFORM_P_NUM_OUTPUT_CHANNELS )
   {
      BDBG_ERR( ("Invalid output instance (%d) specified", pstOutputOpenSettings->uiInstance));
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   hVceOutput = &hVce->outputs[pstOutputOpenSettings->uiInstance].context;

   if ( BVCE_P_Status_eIdle != hVceOutput->eStatus )
   {
      BDBG_ERR( ("Output instance (%d) already opened", pstOutputOpenSettings->uiInstance));
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   BKNI_Memset(
      hVceOutput,
      0,
      sizeof( BVCE_P_Output_Context )
      );

   BDBG_OBJECT_SET(hVceOutput, BVCE_P_Output_Context);
   hVceOutput->hVce = hVce;
   hVceOutput->eStatus = BVCE_P_Status_eOpened;
   hVceOutput->stOpenSettings = *pstOutputOpenSettings;

   /*
    * We're reserving a uiDescriptorQueueDepth of 0 for possible future
    * use to indicate that we want an automatically calculated depth. For
    * now if the value is specified as 0, we'll set it to the default value
    * in BVCE_P_MAX_VIDEODESCRIPTORS.
    */
   if ( 0 == pstOutputOpenSettings->uiDescriptorQueueDepth )
   {
      hVceOutput->stOpenSettings.uiDescriptorQueueDepth = BVCE_P_MAX_VIDEODESCRIPTORS;
   }

   /* Platform Init: Set Cabac Registers */
   switch ( hVceOutput->stOpenSettings.uiInstance )
   {
      case 0:
      case 1:
#if BVCE_PLATFORM_P_NUM_OUTPUT_CHANNELS >= 3
      case 2:
#endif
         hVceOutput->stRegisters.CDB_Read = hVce->stPlatformConfig.stOutput[hVceOutput->stOpenSettings.uiInstance].stCDB.uiReadPointer;
         hVceOutput->stRegisters.CDB_Base = hVce->stPlatformConfig.stOutput[hVceOutput->stOpenSettings.uiInstance].stCDB.uiBasePointer;
         hVceOutput->stRegisters.CDB_Valid = hVce->stPlatformConfig.stOutput[hVceOutput->stOpenSettings.uiInstance].stCDB.uiValidPointer;
         hVceOutput->stRegisters.CDB_End = hVce->stPlatformConfig.stOutput[hVceOutput->stOpenSettings.uiInstance].stCDB.uiEndPointer;

         hVceOutput->stRegisters.ITB_Read = hVce->stPlatformConfig.stOutput[hVceOutput->stOpenSettings.uiInstance].stITB.uiReadPointer;
         hVceOutput->stRegisters.ITB_Base = hVce->stPlatformConfig.stOutput[hVceOutput->stOpenSettings.uiInstance].stITB.uiBasePointer;
         hVceOutput->stRegisters.ITB_Valid = hVce->stPlatformConfig.stOutput[hVceOutput->stOpenSettings.uiInstance].stITB.uiValidPointer;
         hVceOutput->stRegisters.ITB_End = hVce->stPlatformConfig.stOutput[hVceOutput->stOpenSettings.uiInstance].stITB.uiEndPointer;
         break;

      default:
         BDBG_ERR( ("Unsupported output instance (%d)", hVceOutput->stOpenSettings.uiInstance));
         BVCE_Output_Close( hVceOutput );
         return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   if ( NULL != hVceOutput->stOpenSettings.hDescriptorMem )
   {
      rc = BVCE_P_Allocator_Create(
         hVceOutput->stOpenSettings.hDescriptorMem,
         NULL,
         &hVceOutput->stDescriptors.hAllocator
         );

      if ( BERR_SUCCESS != rc )
      {
         BDBG_ERR( ("Error creating Descriptor allocator"));
         BVCE_Output_Close( hVceOutput );
         return BERR_TRACE( rc );
      }
   }
   else
   {
      hVceOutput->stDescriptors.hAllocator = hVceOutput->hVce->ahAllocator[BVCE_P_HeapId_eSystem];
   }

   /* Allocate video buffer descriptors */
   {
      BVCE_P_Buffer_AllocSettings stAllocSettings;
      BVCE_P_Buffer_GetDefaultAllocSettings( &stAllocSettings );
      stAllocSettings.uiSize = ( hVceOutput->stOpenSettings.uiDescriptorQueueDepth * sizeof( BAVC_VideoBufferDescriptor ) );
      stAllocSettings.bLockAddress = true;

      rc = BVCE_P_Buffer_Alloc(
         hVceOutput->stDescriptors.hAllocator,
         &stAllocSettings,
         &hVceOutput->stDescriptors.hDescriptorBuffer
         );

      if ( BERR_SUCCESS != rc )
      {
         BDBG_ERR(("Error allocating descriptor memory buffer"));
         BVCE_Output_Close( hVceOutput );
         return BERR_TRACE( rc );
      }

      /* Get Cached Address */
      hVceOutput->stDescriptors.astDescriptors = (BAVC_VideoBufferDescriptor *) BVCE_P_Buffer_GetAddress_isrsafe( hVceOutput->stDescriptors.hDescriptorBuffer );

      if ( NULL == hVceOutput->stDescriptors.astDescriptors )
      {
         BDBG_ERR(("Error locking descriptor virtual address"));
         BVCE_Output_Close( hVceOutput );
         return BERR_TRACE( BERR_UNKNOWN );
      }

      BKNI_Memset( hVceOutput->stDescriptors.astDescriptors,
                   0,
                   BVCE_P_Buffer_GetSize( hVceOutput->stDescriptors.hDescriptorBuffer )
                   );

      /* Allocate descriptor release metadata */
      hVceOutput->stDescriptors.astDescriptorReleaseMetadata = (BVCE_P_VideoBufferDescriptorReleaseMetadata *) BKNI_Malloc(
         ( hVceOutput->stOpenSettings.uiDescriptorQueueDepth * sizeof( BVCE_P_VideoBufferDescriptorReleaseMetadata ) ) );

      if ( NULL == hVceOutput->stDescriptors.astDescriptorReleaseMetadata )
      {
         BDBG_ERR(("Error allocating descriptor release metadata"));
         return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
      }

      BKNI_Memset( hVceOutput->stDescriptors.astDescriptorReleaseMetadata, 0, ( hVceOutput->stOpenSettings.uiDescriptorQueueDepth * sizeof( BVCE_P_VideoBufferDescriptorReleaseMetadata ) ) );

      /* Allocate metadata buffer descriptors */
      stAllocSettings.uiSize = ( hVceOutput->stOpenSettings.uiDescriptorQueueDepth * sizeof( BAVC_VideoMetadataDescriptor ) );

      rc = BVCE_P_Buffer_Alloc(
         hVceOutput->stDescriptors.hAllocator,
         &stAllocSettings,
         &hVceOutput->stDescriptors.hMetadataBuffer
         );

      if ( BERR_SUCCESS != rc )
      {
         BDBG_ERR(("Error allocating metadata memory buffer"));
         BVCE_Output_Close( hVceOutput );
         return BERR_TRACE( rc );
      }

      /* Get Cached Address */
      hVceOutput->stDescriptors.astMetadataDescriptors = (BAVC_VideoMetadataDescriptor *) BVCE_P_Buffer_GetAddress_isrsafe( hVceOutput->stDescriptors.hMetadataBuffer );

      if ( NULL == hVceOutput->stDescriptors.astMetadataDescriptors )
      {
         BDBG_ERR(("Error locking metadata virtual address"));
         BVCE_Output_Close( hVceOutput );
         return BERR_TRACE( BERR_UNKNOWN );
      }

      BKNI_Memset( hVceOutput->stDescriptors.astMetadataDescriptors,
                   0,
                   BVCE_P_Buffer_GetSize( hVceOutput->stDescriptors.hMetadataBuffer )
                   );

      /* Allocate ITB Index */
      stAllocSettings.uiSize = ( hVceOutput->stOpenSettings.uiDescriptorQueueDepth * sizeof( BVCE_P_Output_ITB_IndexEntry ) );

      rc = BVCE_P_Buffer_Alloc(
         hVceOutput->stDescriptors.hAllocator,
         &stAllocSettings,
         &hVceOutput->stDescriptors.hITBIndexBuffer
         );

      if ( BERR_SUCCESS != rc )
      {
         BDBG_ERR(("Error allocating ITB index buffer"));
         BVCE_Output_Close( hVceOutput );
         return BERR_TRACE( rc );
      }

      /* Get Cached Address */
      hVceOutput->stDescriptors.astIndex = ( BVCE_P_Output_ITB_IndexEntry* ) BVCE_P_Buffer_GetAddress_isrsafe( hVceOutput->stDescriptors.hITBIndexBuffer );

      if ( NULL == hVceOutput->stDescriptors.astIndex )
      {
         BDBG_ERR(("Error locking ITB Index virtual address"));
         BVCE_Output_Close( hVceOutput );
         return BERR_TRACE( BERR_UNKNOWN );
      }

      BKNI_Memset( hVceOutput->stDescriptors.astIndex,
                   0,
                   BVCE_P_Buffer_GetSize( hVceOutput->stDescriptors.hITBIndexBuffer )
                   );
   }

   *phVceOutput = hVceOutput;

   BVCE_P_FUNCTION_TRACE_LEAVE(0, hVce, pstOutputOpenSettings->uiInstance);
   BDBG_LEAVE( BVCE_Output_Open );
   return BERR_TRACE( BERR_SUCCESS );
}

static
void
BVCE_Output_S_DetachFromChannel(
   BVCE_Output_Handle hVceOutput
   )
{
   BDBG_OBJECT_ASSERT(hVceOutput, BVCE_P_Output_Context);

   if ( ( NULL != hVceOutput->state.hVceCh )
        && ( BVCE_P_Status_eOpened != BVCE_Channel_P_GetState(hVceOutput->state.hVceCh) )
        && ( BVCE_P_Status_eIdle != BVCE_Channel_P_GetState(hVceOutput->state.hVceCh) ) )
   {
      BVCE_Channel_P_HandleEOSEvent(
         hVceOutput->state.hVceCh
         );
   }
   /* Detach Output Handle from Channel Handle */
   hVceOutput->state.hVceCh = NULL;
}

BERR_Code
BVCE_Output_Close(
         BVCE_Output_Handle hVceOutput
         )
{
   BDBG_ENTER( BVCE_Output_Close );

   /* NOTE: The output could have been closed elsewhere, so a NULL handle is possible */
   if ( NULL != hVceOutput )
   {
      BDBG_OBJECT_ASSERT(hVceOutput, BVCE_P_Output_Context);
      BVCE_P_FUNCTION_TRACE_ENTER(0, hVceOutput->hVce, hVceOutput->stOpenSettings.uiInstance);
      BVCE_Output_S_DetachFromChannel( hVceOutput );

      BVCE_P_Buffer_Free( hVceOutput->stDescriptors.hITBIndexBuffer );
      BVCE_P_Buffer_Free( hVceOutput->stDescriptors.hMetadataBuffer );
      if ( NULL != hVceOutput->stDescriptors.astDescriptorReleaseMetadata )
      {
         BKNI_Free( hVceOutput->stDescriptors.astDescriptorReleaseMetadata );
         hVceOutput->stDescriptors.astDescriptorReleaseMetadata = NULL;
      }
      BVCE_P_Buffer_Free( hVceOutput->stDescriptors.hDescriptorBuffer );
      if ( NULL != hVceOutput->stOpenSettings.hDescriptorMem )
      {
         BVCE_P_Allocator_Destroy( hVceOutput->stDescriptors.hAllocator );
      }

   #if BVCE_P_DUMP_OUTPUT_CDB
      BVCE_Debug_P_CloseLog( hVceOutput->hCDBDumpFile );
      hVceOutput->hCDBDumpFile = NULL;
   #endif
   #if BVCE_P_DUMP_OUTPUT_ITB
      BVCE_Debug_P_CloseLog( hVceOutput->hITBDumpFile );
      hVceOutput->hITBDumpFile = NULL;
   #endif
   #if BVCE_P_DUMP_OUTPUT_ITB_DESC
      BVCE_Debug_P_CloseLog( hVceOutput->hITBDescDumpFile );
      hVceOutput->hITBDescDumpFile = NULL;
   #endif
   #if BVCE_P_DUMP_MAU_PERFORMANCE_DATA
      BVCE_Debug_P_CloseLog( hVceOutput->hMAUPerformanceDumpFile );
      hVceOutput->hMAUPerformanceDumpFile = NULL;
   #endif
   #if BVCE_P_TEST_MODE
      BVCE_Debug_P_CloseLog( hVceOutput->hDescriptorLog );
      hVceOutput->hDescriptorLog = NULL;
   #endif

      /* TODO: Do we need to do anything else? */
      hVceOutput->eStatus = BVCE_P_Status_eIdle;
      BVCE_P_FUNCTION_TRACE_LEAVE(0, hVceOutput->hVce, hVceOutput->stOpenSettings.uiInstance);

      BDBG_OBJECT_UNSET(hVceOutput, BVCE_P_Output_Context);
   }

   BDBG_LEAVE( BVCE_Output_Close );

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BVCE_Output_SetBuffers(
         BVCE_Output_Handle hVceOutput,
         const BVCE_OutputBuffers_Handle hVceOutputBuffers
         )
{
   BDBG_ENTER( BVCE_Output_SetBuffers );

   BDBG_OBJECT_ASSERT(hVceOutput, BVCE_P_Output_Context);
   BVCE_P_FUNCTION_TRACE_ENTER(0, hVceOutput->hVce, hVceOutput->stOpenSettings.uiInstance);

   /* TODO: Do we need to handle NULL output buffer specially? E.g. snapshot read/write pointers? */
   hVceOutput->hOutputBuffers = hVceOutputBuffers;
   BVCE_Output_Reset(
           hVceOutput
           );

   BVCE_P_FUNCTION_TRACE_LEAVE(0, hVceOutput->hVce, hVceOutput->stOpenSettings.uiInstance);
   BDBG_LEAVE( BVCE_Output_SetBuffers );
   return BERR_TRACE( BERR_SUCCESS );
}

static
unsigned
BVCE_Output_S_ITB_Shadow_GetNumFrames(
   BVCE_Output_Handle hVceOutput
   )
{
   unsigned uiNumFrames = 0;

   if ( hVceOutput->state.stITBBuffer.uiIndexWriteOffset >= hVceOutput->state.stITBBuffer.uiIndexShadowReadOffset )
   {
      uiNumFrames = hVceOutput->state.stITBBuffer.uiIndexWriteOffset - hVceOutput->state.stITBBuffer.uiIndexShadowReadOffset;
   }
   else
   {
      uiNumFrames = hVceOutput->stOpenSettings.uiDescriptorQueueDepth - hVceOutput->state.stITBBuffer.uiIndexShadowReadOffset;
      uiNumFrames += hVceOutput->state.stITBBuffer.uiIndexWriteOffset;
   }

   return uiNumFrames;
}

static
BVCE_P_Output_ITB_IndexEntry*
BVCE_Output_S_ITB_Shadow_GetFrameIndexEntry(
   BVCE_Output_Handle hVceOutput,
   unsigned uiIndex
   )
{
   unsigned uiOffset = (hVceOutput->state.stITBBuffer.uiIndexShadowReadOffset + uiIndex) % hVceOutput->stOpenSettings.uiDescriptorQueueDepth;

   return &hVceOutput->stDescriptors.astIndex[uiOffset];
}

static
void
BVCE_Output_S_ITB_Shadow_CalculateDepth(
   BVCE_Output_Handle hVceOutput
   )
{
   if ( hVceOutput->state.stITBBuffer.stOffset.uiValid >= hVceOutput->state.stITBBuffer.stOffset.uiShadowRead )
   {
      hVceOutput->state.stITBBuffer.uiShadowDepth = hVceOutput->state.stITBBuffer.stOffset.uiValid - hVceOutput->state.stITBBuffer.stOffset.uiShadowRead;
   }
   else
   {
      hVceOutput->state.stITBBuffer.uiShadowDepth = hVceOutput->state.stITBBuffer.stOffset.uiEnd - hVceOutput->state.stITBBuffer.stOffset.uiShadowRead;
      hVceOutput->state.stITBBuffer.uiShadowDepth += hVceOutput->state.stITBBuffer.stOffset.uiValid - hVceOutput->state.stITBBuffer.stOffset.uiBase;
   }
}

static
void
BVCE_Output_S_ITB_Shadow_ConsumeEntry(
   BVCE_Output_Handle hVceOutput
   )
{
   BDBG_ASSERT( hVceOutput->state.stITBBuffer.uiIndexShadowReadOffset != hVceOutput->state.stITBBuffer.uiIndexWriteOffset );

   hVceOutput->state.stITBBuffer.stOffset.uiShadowRead += hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexShadowReadOffset].uiSizeInITB;
   if ( hVceOutput->state.stITBBuffer.stOffset.uiShadowRead >= hVceOutput->state.stITBBuffer.stOffset.uiEnd )
   {
      hVceOutput->state.stITBBuffer.stOffset.uiShadowRead -= ( hVceOutput->state.stITBBuffer.stOffset.uiEnd - hVceOutput->state.stITBBuffer.stOffset.uiBase );
   }

   hVceOutput->state.stITBBuffer.uiIndexShadowReadOffset++;
   hVceOutput->state.stITBBuffer.uiIndexShadowReadOffset %= hVceOutput->stOpenSettings.uiDescriptorQueueDepth;
}

static
void
BVCE_Output_S_ITB_ConsumeEntry(
   BVCE_Output_Handle hVceOutput
   )
{
   if ( true == hVceOutput->state.bCabacInitializedShadow )
   {
      BDBG_ASSERT( hVceOutput->state.stITBBuffer.uiIndexReadOffset != hVceOutput->state.stITBBuffer.uiIndexWriteOffset );

      hVceOutput->state.stITBBuffer.stOffset.uiRead += hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexReadOffset].uiSizeInITB;
      /* 7425A0: Hack to prevent HW/SW confusion of full vs empty */
      if ( false == hVceOutput->state.stITBBuffer.bReadHackDone )
      {
         hVceOutput->state.stITBBuffer.stOffset.uiRead--;
         hVceOutput->state.stITBBuffer.bReadHackDone = true;
      }
      if ( hVceOutput->state.stITBBuffer.stOffset.uiRead >= hVceOutput->state.stITBBuffer.stOffset.uiEnd )
      {
         hVceOutput->state.stITBBuffer.stOffset.uiRead -= ( hVceOutput->state.stITBBuffer.stOffset.uiEnd - hVceOutput->state.stITBBuffer.stOffset.uiBase );
      }

      hVceOutput->state.stITBBuffer.uiIndexReadOffset++;
      hVceOutput->state.stITBBuffer.uiIndexReadOffset %= hVceOutput->stOpenSettings.uiDescriptorQueueDepth;
   }
}

static
bool
BVCE_Output_S_IsFrameAvailable(
   BVCE_Output_Handle hVceOutput
   )
{
   BVCE_Output_S_ITB_Shadow_CalculateDepth( hVceOutput );

   if ( 0 == BVCE_Output_S_ITB_Shadow_GetNumFrames( hVceOutput ) )
   {
      /* We ran out of ITB entries */
      return false;
   }

   return true;
}

static
bool
BVCE_Output_S_IsFrameDataAvailable(
   BVCE_Output_Handle hVceOutput
   )
{
   return ( hVceOutput->state.stCDBBuffer.uiShadowValidOffset != hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead );
}

static
bool
BVCE_Output_S_IsDescriptorAvailable(
   BVCE_Output_Handle hVceOutput,
   BVCE_P_Output_ITB_IndexEntry *pITBIndexEntry
   )
{
   /* Check for Available Descriptors */
   uint32_t uiTempWriteOffset = hVceOutput->state.uiDescriptorWriteOffset + 1;
   uiTempWriteOffset %= hVceOutput->stOpenSettings.uiDescriptorQueueDepth;

   if (uiTempWriteOffset == hVceOutput->state.uiDescriptorReadOffset )
   {
      /* We ran out of descriptors */
      return false;
   }

   /* Check to make sure we have another descriptor available for the metadata */
   if ( ( true == hVceOutput->state.bNewITBEntry )
        && ( 0 != ( pITBIndexEntry->stFrameDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_START ) ) )
   {
      if ( ( false == hVceOutput->state.bMetadataSent )
           || ( ( pITBIndexEntry->uiMetadata & BVCE_P_NEW_METADATA_MASK ) != ( hVceOutput->state.uiPreviousMetadata & BVCE_P_NEW_METADATA_MASK ) ) ) /* SW7425-5477: Send metadata again if any metadata desc param has changed */
      {
         uiTempWriteOffset++;
         uiTempWriteOffset %= hVceOutput->stOpenSettings.uiDescriptorQueueDepth;

         if (uiTempWriteOffset == hVceOutput->state.uiDescriptorReadOffset )
         {
            /* We ran out of descriptors */
            return false;
         }

         /* SW7425-3360: Check to make sure we have enough metadata descriptors */
         uiTempWriteOffset = hVceOutput->state.uiMetadataDescriptorWriteOffset + 1;
         uiTempWriteOffset %= BVCE_P_MAX_METADATADESCRIPTORS;

         if (uiTempWriteOffset == hVceOutput->state.uiMetadataDescriptorReadOffset )
         {
            /* We ran out of metadata descriptors */
            return false;
         }
      }
   }

   return true;
}

static
unsigned
BVCE_Output_S_CalculateEndOfPictureOffset(
   BVCE_Output_Handle hVceOutput,
   bool *pbEndOfPicture
   )
{
   if ( NULL != pbEndOfPicture )
   {
      *pbEndOfPicture = false;
   }

   /* Figure out how much CDB data we have for the current picture */
   if ( BVCE_Output_S_ITB_Shadow_GetNumFrames( hVceOutput ) > 1 )
   {
      /* We have a next picture, so we know exactly how long the current picture is */
      uint32_t uiDepthToNext;
      uint32_t uiDepthToValid;
      BVCE_P_Output_ITB_IndexEntry *pITBIndexEntryNext = BVCE_Output_S_ITB_Shadow_GetFrameIndexEntry( hVceOutput, 1 );

      /* It is possible that the CDB Valid doesn't, yet, contain any of the next picture and
       * may still be in the middle of the current picture, so we need use the depth that is the
       * lesser of depth(cdb_read,cdb_next) depth(cdb_read,cdb_valid)
       */
      if ( pITBIndexEntryNext->uiCDBAddress >= hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead )
      {
         uiDepthToNext = pITBIndexEntryNext->uiCDBAddress - hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead;
      }
      else
      {
         uiDepthToNext = hVceOutput->state.stCDBBuffer.stOffset.uiEnd - hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead;
         uiDepthToNext += pITBIndexEntryNext->uiCDBAddress - hVceOutput->state.stCDBBuffer.stOffset.uiBase;
      }

      if ( hVceOutput->state.stCDBBuffer.uiShadowValidOffset >= hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead )
      {
         uiDepthToValid = hVceOutput->state.stCDBBuffer.uiShadowValidOffset - hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead;
      }
      else
      {
         uiDepthToValid = hVceOutput->state.stCDBBuffer.stOffset.uiEnd - hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead;
         uiDepthToValid += hVceOutput->state.stCDBBuffer.uiShadowValidOffset - hVceOutput->state.stCDBBuffer.stOffset.uiBase;
      }

      if ( uiDepthToValid < uiDepthToNext )
      {
         return hVceOutput->state.stCDBBuffer.uiShadowValidOffset;
      }
      else
      {
         if ( NULL != pbEndOfPicture )
         {
            *pbEndOfPicture = true;
         }
         return pITBIndexEntryNext->uiCDBAddress;
      }
   }
   else
   {
      /* We don't have a next picture, so since the ITB entry for the next picture
       * is always written before the CDB data for the next picture, we know the
       * current CDB data is all for the current picture */
      return hVceOutput->state.stCDBBuffer.uiShadowValidOffset;
   }
}

static
void
BVCE_Output_S_DataUnitDetectReset(
         BVCE_Output_Handle hVceOutput
         )
{
   hVceOutput->state.stDataUnitDetect.eState = BVCE_Output_P_DataUnitDetectState_eLookForNALU;
   hVceOutput->state.stCDBBuffer.uiShadowValidOffset = hVceOutput->state.stCDBBuffer.stOffset.uiValid;
}

#ifndef BVCE_PLATFORM_P_SUPPORTS_ITB_NAL
#define BVCE_DATAUNITDETECT_TYPE_MAX_BYTES 1

typedef struct BVCE_DataUnitDetect_Settings
{
   uint8_t *pBuffer; /* Pointer to the CDB buffer */
   size_t uiSize; /* Size of the CDB buffer */
   unsigned uiStartOffset; /* Start Offset - Offset to the first byte where the Data Unit Detect code should start parsing */
   unsigned uiStopOffset; /* End Offset - Offset to the last byte where the Data Unit Detect code should stop parsing */
} BVCE_DataUnitDetect_Settings;

typedef struct BVCE_DataUnitDetect_Result
{
   bool bNaluFound; /* True if the uiReadOffset points to the start of a NALU */
   uint8_t uiDataUnitType[BVCE_DATAUNITDETECT_TYPE_MAX_BYTES]; /* Contains the values following start code */
   unsigned uiBytesProcessed; /* How many bytes (from uiStartOffset) have been parsed */
} BVCE_DataUnitDetect_Results;

/* If pBuffer[uiStartOffset] IS the start of a complete NALU:
 *    - stResult.bNaluFound = true;
 *    - stResult.uiDataUnitType[] = NALU_Type
 *    - stResult.uiBytesProcessed = size of start code + nalu type byte(s)
 * If pBuffer[uiStartOffset] IS NOT the start of a complete NALU:
 *    - stResult.bNaluFound = false
 *    - stResult.uiBytesProcessed = number of bytes that don't contain a NALU
 *
 * Some scenarios:
 *     - stSettings.pBuffer = don't care
 *     - stSettings.uiSize = 20
 *
 *  [ 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 ]
 *  [ XX XX XX XX XX 00 00 00 01 NN YY YY YY YY                   ]
 *     - stSettings.uiStartOffset = 0
 *     - stSettings.uiStopOffset = 14
 *
 *     - stResult.bNaluFound = false
 *     - stResult.uiBytesProcessed = 5
 *
 *  [ 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 ]
 *  [                00 00 00 01 NN YY YY YY YY                   ]
 *     - stSettings.uiStartOffset = 5
 *     - stSettings.uiStopOffset = 14
 *
 *     - stResult.bNaluFound = true
 *     - stResult.uiDataUnitType[0] = NN;
 *     - stResult.uiBytesProcessed = 5;
 *
 *  [ 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 ]
 *  [ 00                            YY YY YY YY ZZ ZZ ZZ ZZ 00 00 ]
 *     - stSettings.uiStartOffset = 10
 *     - stSettings.uiStopOffset = 1
 *
 *     - stResult.bNaluFound = false
 *     - stResult.uiBytesProcessed = 8
 *
 *  [ 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 ]
 *  [ 00                                                    00 00 ]
 *     - stSettings.uiStartOffset = 18
 *     - stSettings.uiStopOffset = 1
 *
 *     - stResult.bNaluFound = false;
 *     - stResult.uiBytesProcessed = 0
 *
 *  [ 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 ]
 *  [ 00 01 MM XX XX XX 00 00 00 01                         00 00 ]
 *     - stSettings.uiStartOffset = 18
 *     - stSettings.uiStopOffset = 10
 *
 *     - stResult.bNaluFound = true
 *     - stResult.uiDataUnitType[] = MM
 *     - stResult.uiBytesProcessed = 5
 */

typedef enum BVCE_Output_S_DataUnitDetect_State
{
   BVCE_Output_S_DataUnitDetect_State_eLookFor1st0,
   BVCE_Output_S_DataUnitDetect_State_eLookFor2nd0,
   BVCE_Output_S_DataUnitDetect_State_eLookFor3rd0,
   BVCE_Output_S_DataUnitDetect_State_eLookFor1,
   BVCE_Output_S_DataUnitDetect_State_eReadNaluType
} BVCE_Output_S_DataUnitDetect_State;


static
void
BVCE_Output_S_DataUnitDetect_Parser(
   const BVCE_DataUnitDetect_Settings *pstSettings,
   BVCE_DataUnitDetect_Results *pstResults
   )
{
   BDBG_ASSERT( pstSettings );
   BDBG_ASSERT( pstResults );
   BKNI_Memset( pstResults, 0, sizeof( BVCE_DataUnitDetect_Results) );
   {
      unsigned uiCurrentOffset = pstSettings->uiStartOffset;
      BVCE_Output_S_DataUnitDetect_State eState = BVCE_Output_S_DataUnitDetect_State_eLookFor1st0;

      while ( uiCurrentOffset != pstSettings->uiStopOffset )
      {
         switch ( eState )
         {
            case BVCE_Output_S_DataUnitDetect_State_eLookFor1st0:
               if ( 0 == pstSettings->pBuffer[uiCurrentOffset] )
               {
                  eState = BVCE_Output_S_DataUnitDetect_State_eLookFor2nd0;
               }
               else
               {
                  pstResults->uiBytesProcessed++;
               }
               break;

            case BVCE_Output_S_DataUnitDetect_State_eLookFor2nd0:
               if ( 0 == pstSettings->pBuffer[uiCurrentOffset] )
               {
                  eState = BVCE_Output_S_DataUnitDetect_State_eLookFor3rd0;
               }
               else
               {
                  eState = BVCE_Output_S_DataUnitDetect_State_eLookFor1st0;
                  pstResults->uiBytesProcessed+=2;
               }

               break;

            case BVCE_Output_S_DataUnitDetect_State_eLookFor3rd0:
               if ( 0 == pstSettings->pBuffer[uiCurrentOffset] )
               {
                  eState = BVCE_Output_S_DataUnitDetect_State_eLookFor1;
               }
               else
               {
                  eState = BVCE_Output_S_DataUnitDetect_State_eLookFor1st0;
                  pstResults->uiBytesProcessed+=3;
               }
               break;

            case BVCE_Output_S_DataUnitDetect_State_eLookFor1:
               if ( 1 == pstSettings->pBuffer[uiCurrentOffset] )
               {
                  if ( 0 == pstResults->uiBytesProcessed )
                  {
                     /* NALU Found */
                     eState = BVCE_Output_S_DataUnitDetect_State_eReadNaluType;
                  }
                  else
                  {
                     /* Return previous bytes processed first */
                     return;
                  }
               }
               else if ( 0 == pstSettings->pBuffer[uiCurrentOffset] )
               {
                  eState = BVCE_Output_S_DataUnitDetect_State_eLookFor1;
                  pstResults->uiBytesProcessed+=1;
               }
               else
               {
                  eState = BVCE_Output_S_DataUnitDetect_State_eLookFor1st0;
                  pstResults->uiBytesProcessed+=4;
               }
               break;

            case BVCE_Output_S_DataUnitDetect_State_eReadNaluType:
               pstResults->bNaluFound = true;
               pstResults->uiBytesProcessed = 5;
               pstResults->uiDataUnitType[0] = pstSettings->pBuffer[uiCurrentOffset];
               return;
         }

         uiCurrentOffset++;
         uiCurrentOffset %= pstSettings->uiSize;
      }
   }
}

static
void
BVCE_Output_S_DataUnitDetect_Helper(
   BVCE_Output_Handle hVceOutput,
   BVCE_DataUnitDetect_Settings *pstSettings,
   BVCE_DataUnitDetect_Results *pstResults
   )
{
   pstSettings->uiStartOffset = hVceOutput->state.stCDBBuffer.uiShadowValidOffset - hVceOutput->state.stCDBBuffer.stOffset.uiBase;
   pstSettings->uiStopOffset = hVceOutput->state.stCDBBuffer.stOffset.uiValid - hVceOutput->state.stCDBBuffer.stOffset.uiBase;

   BVCE_Output_S_DataUnitDetect_Parser(
      pstSettings,
      pstResults
      );

   hVceOutput->state.stCDBBuffer.uiShadowValidOffset += pstResults->uiBytesProcessed;
   if ( hVceOutput->state.stCDBBuffer.uiShadowValidOffset >= hVceOutput->state.stCDBBuffer.stOffset.uiEnd )
   {
      hVceOutput->state.stCDBBuffer.uiShadowValidOffset -= ( hVceOutput->state.stCDBBuffer.stOffset.uiEnd - hVceOutput->state.stCDBBuffer.stOffset.uiBase );
   }
}

static
bool
BVCE_Output_S_DataUnitDetect_SkipToNextFrameStart(
   BVCE_Output_Handle hVceOutput
   )
{
   bool bEndOfFrame;

   hVceOutput->state.stCDBBuffer.uiShadowValidOffset = hVceOutput->state.stCDBBuffer.stOffset.uiValid;
   hVceOutput->state.stCDBBuffer.uiShadowValidOffset = BVCE_Output_S_CalculateEndOfPictureOffset( hVceOutput, &bEndOfFrame );

   return bEndOfFrame;
}
#endif

static
void
BVCE_Output_S_DataUnitDetect(
   BVCE_Output_Handle hVceOutput
)
{
   if ( ( true == hVceOutput->stOpenSettings.bEnableDataUnitDetection )
      && ( true == hVceOutput->state.stChannelCache.bValid )
   )
   {
      /* We only want to process new start codes if the all the previous data referenced by the
       * shadow valid offset has been queued. I.e. all the data associated with the previous data unit
       * has been muxed */
      if ( hVceOutput->state.stCDBBuffer.uiShadowValidOffset == hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead )
      {
#ifdef BVCE_PLATFORM_P_SUPPORTS_ITB_NAL
#ifdef BVCE_PLATFORM_P_ITB_REQUIRES_COHERENCY_WORKAROUND
         /* If there is a next ITB entry, set shadowValid to the lesser of the start of the next NALU of valid offset */
         if ( BVCE_Output_S_ITB_Shadow_GetNumFrames( hVceOutput ) > 1 )
         {
            BVCE_P_Output_ITB_IndexEntry *pITBIndexEntryLast = BVCE_Output_S_ITB_Shadow_GetFrameIndexEntry( hVceOutput, BVCE_Output_S_ITB_Shadow_GetNumFrames( hVceOutput ) - 1 );
            unsigned uiDepthToValid, uiDepthToLastEntry;

            if ( hVceOutput->state.stCDBBuffer.stOffset.uiValid >= hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead )
            {
               uiDepthToValid = hVceOutput->state.stCDBBuffer.stOffset.uiValid - hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead;
            }
            else
            {
               uiDepthToValid = hVceOutput->state.stCDBBuffer.stOffset.uiEnd - hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead;
               uiDepthToValid += hVceOutput->state.stCDBBuffer.stOffset.uiValid - hVceOutput->state.stCDBBuffer.stOffset.uiBase;
            }

            if ( pITBIndexEntryLast->uiCDBAddress >= hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead )
            {
               uiDepthToLastEntry = pITBIndexEntryLast->uiCDBAddress - hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead;
            }
            else
            {
               uiDepthToLastEntry = hVceOutput->state.stCDBBuffer.stOffset.uiEnd - hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead;
               uiDepthToLastEntry += pITBIndexEntryLast->uiCDBAddress - hVceOutput->state.stCDBBuffer.stOffset.uiBase;
            }

            if ( uiDepthToLastEntry < uiDepthToValid )
            {
               hVceOutput->state.stCDBBuffer.uiShadowValidOffset = pITBIndexEntryLast->uiCDBAddress;
            }
            else
            {
               hVceOutput->state.stCDBBuffer.uiShadowValidOffset = hVceOutput->state.stCDBBuffer.stOffset.uiValid;
            }
         }
         else
         {
            /* If the current ITB entry is an EOS, then set shadowValid = valid */
            BVCE_P_Output_ITB_IndexEntry *pITBIndexEntry = pITBIndexEntry = BVCE_Output_S_ITB_Shadow_GetFrameIndexEntry( hVceOutput, 0 );
            if ( 0 != ( pITBIndexEntry->stFrameDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOS ) )
            {
               hVceOutput->state.stCDBBuffer.uiShadowValidOffset = hVceOutput->state.stCDBBuffer.stOffset.uiValid;
            }
         }
#else
         BVCE_Output_S_DataUnitDetectReset( hVceOutput );
#endif
#else
         if ( false == hVceOutput->state.stDataUnitDetect.bFound )
         {
            BVCE_DataUnitDetect_Settings stSettings;
            BVCE_DataUnitDetect_Results stResult;

            BKNI_Memset( &stSettings, 0, sizeof( BVCE_DataUnitDetect_Settings ) );
            BKNI_Memset( &stResult, 0, sizeof( BVCE_DataUnitDetect_Results ) );

            stSettings.pBuffer = BVCE_P_Buffer_LockAddress( hVceOutput->hOutputBuffers->stCDB.hBuffer );
            stSettings.uiSize = BVCE_P_Buffer_GetSize( hVceOutput->hOutputBuffers->stCDB.hBuffer );
            if ( NULL == stSettings.pBuffer )
            {
               BDBG_ERR(("Error locking CDB memory block"));
               return;
            }

            switch ( hVceOutput->state.stChannelCache.stStartEncodeSettings.stProtocolInfo.eProtocol )
            {
               case BAVC_VideoCompressionStd_eH264:
               {
                  switch ( hVceOutput->state.stDataUnitDetect.eState )
                  {
                     case BVCE_Output_P_DataUnitDetectState_eLookForNALU:
                        /* Search for a NALU */
                        BVCE_Output_S_DataUnitDetect_Helper( hVceOutput, &stSettings, &stResult );

                        if ( true == stResult.bNaluFound )
                        {
                           hVceOutput->state.stDataUnitDetect.bFound = true;
                           hVceOutput->state.stDataUnitDetect.uiType = stResult.uiDataUnitType[0];

                           switch ( hVceOutput->state.stDataUnitDetect.uiType & 0x1F )
                           {
                              case 1: /* Coded slice of a non-IDR picture (AVC/H.264)*/
                              case 5: /* Coded slice of an IDR picture (AVC/H.264) */
                                 if ( false == BVCE_Output_S_DataUnitDetect_SkipToNextFrameStart( hVceOutput ) )
                                 {
                                    hVceOutput->state.stDataUnitDetect.eState = BVCE_Output_P_DataUnitDetectState_eSkipToNextFrameStart;
                                 }
                                 break;
                              default:
                                 /* Read as much of the NALU payload as possible */
                                 BVCE_Output_S_DataUnitDetect_Helper( hVceOutput, &stSettings, &stResult );
                                 BDBG_ASSERT( false == stResult.bNaluFound );
                                 break;
                           }
                        }

                        break;

                     case BVCE_Output_P_DataUnitDetectState_eSkipToNextFrameStart:
                     default:
                        {
                           if ( true == BVCE_Output_S_DataUnitDetect_SkipToNextFrameStart( hVceOutput ) )
                           {
                              hVceOutput->state.stDataUnitDetect.eState = BVCE_Output_P_DataUnitDetectState_eLookForNALU;
                           }
                        }
                        break;
                  }
               }
               break;

               default:
                  /* Data Unit Detection Not Supported - Falling Through */
                  BVCE_Output_S_DataUnitDetectReset( hVceOutput );
                  break;
            }

            BVCE_P_Buffer_UnlockAddress( hVceOutput->hOutputBuffers->stCDB.hBuffer );
         }
#endif
      }
   }
   else
   {
      BVCE_Output_S_DataUnitDetectReset( hVceOutput );
   }
}

static
void
BVCE_Output_S_BufferCacheReset(
         BVCE_Output_Handle hVceOutput
         )
{
   BDBG_OBJECT_ASSERT(hVceOutput, BVCE_P_Output_Context);

   BKNI_Memset(
            &hVceOutput->state.stBufferCache,
            0,
            sizeof( hVceOutput->state.stBufferCache )
            );

   if ( NULL != hVceOutput->hOutputBuffers )
   {
      hVceOutput->state.stBufferCache.uiITBCacheValidOffset = BVCE_P_Buffer_GetDeviceOffset_isrsafe( hVceOutput->hOutputBuffers->stITB.hBuffer );
      hVceOutput->state.stBufferCache.uiCDBCacheValidOffset = BVCE_P_Buffer_GetDeviceOffset_isrsafe( hVceOutput->hOutputBuffers->stCDB.hBuffer );
   }
}

static
void
BVCE_Output_S_ChannelCacheUpdate(
         BVCE_Output_Handle hVceOutput
         )
{
   if ( ( NULL != hVceOutput->state.hVceCh )
        && (( BVCE_P_Status_eStarted == BVCE_Channel_P_GetState( hVceOutput->state.hVceCh ) )
             || ( BVCE_P_Status_eStopping == BVCE_Channel_P_GetState( hVceOutput->state.hVceCh ) ))
      )
   {
      if ( false == hVceOutput->state.stChannelCache.bValid )
      {
         /* Start Encode Settings don't change, so only copy them once */
         BVCE_Channel_P_GetStartEncodeSettings(
            hVceOutput->state.hVceCh,
            &hVceOutput->state.stChannelCache.stStartEncodeSettings
            );
      }
      BVCE_Channel_P_GetEncodeSettings(
         hVceOutput->state.hVceCh,
         &hVceOutput->state.stChannelCache.stEncodeSettings
         );

      hVceOutput->state.stChannelCache.bValid = true;
   }
}

/* BVCE_Output_S_ITB_BufferCacheUpdate -
 *
 * will flush the cache for the region of ITB that was written
 * since the last time this function was called
 */
static
void
BVCE_Output_S_ITB_BufferCacheUpdate(
         BVCE_Output_Handle hVceOutput
         )
{
   void *pAddress;
   unsigned uiLength;

   if ( 0 == hVceOutput->state.stBufferCache.uiITBCacheValidOffset )
   {
      hVceOutput->state.stBufferCache.uiITBCacheValidOffset = hVceOutput->state.stITBBuffer.stOffset.uiBase;
   }

   while ( hVceOutput->state.stBufferCache.uiITBCacheValidOffset != hVceOutput->state.stITBBuffer.stOffset.uiValid )
   {
      void *pBaseAddress;

      if ( hVceOutput->state.stITBBuffer.stOffset.uiValid >= hVceOutput->state.stBufferCache.uiITBCacheValidOffset )
      {
         uiLength = hVceOutput->state.stITBBuffer.stOffset.uiValid - hVceOutput->state.stBufferCache.uiITBCacheValidOffset;
      }
      else
      {
         uiLength = hVceOutput->state.stITBBuffer.stOffset.uiEnd - hVceOutput->state.stBufferCache.uiITBCacheValidOffset;
      }

      pBaseAddress = BVCE_P_Buffer_LockAddress( hVceOutput->hOutputBuffers->stITB.hBuffer );
      if ( NULL == pBaseAddress )
      {
         BDBG_ERR(("Error locking ITB memory"));
         break;
      }

      pAddress = (void*) ( (uint8_t*) pBaseAddress + ( hVceOutput->state.stBufferCache.uiITBCacheValidOffset - hVceOutput->state.stITBBuffer.stOffset.uiBase) );

      BVCE_P_Buffer_FlushCache_isr(
               hVceOutput->hOutputBuffers->stITB.hBuffer,
               pAddress,
               uiLength
               );

      BVCE_P_Buffer_UnlockAddress( hVceOutput->hOutputBuffers->stITB.hBuffer );

      BDBG_MODULE_MSG(BVCE_OUTPUT_CACHE, ( "ITB Flushed: %p %d bytes",
                  pAddress,
                  uiLength
                  ));

#if BVCE_P_DUMP_OUTPUT_ITB
                  if ( ( NULL == hVceOutput->hITBDumpFile ) && ( NULL != hVceOutput->state.hVceCh ) )
                  {
                     static unsigned uiInstance[BVCE_PLATFORM_P_NUM_ENCODE_INSTANCES][BVCE_PLATFORM_P_NUM_ENCODE_CHANNELS];
                     char fname[256];
                     sprintf(fname, "BVCE_OUTPUT_%02d_%02d_%03d.itb", hVceOutput->state.hVceCh->hVce->stOpenSettings.uiInstance, hVceOutput->state.hVceCh->stOpenSettings.uiInstance, uiInstance[hVceOutput->state.hVceCh->hVce->stOpenSettings.uiInstance][hVceOutput->state.hVceCh->stOpenSettings.uiInstance]);
                     uiInstance[hVceOutput->state.hVceCh->hVce->stOpenSettings.uiInstance][hVceOutput->state.hVceCh->stOpenSettings.uiInstance]++;
                     if ( false == BVCE_Debug_P_OpenLog( fname, &hVceOutput->hITBDumpFile ) )
                     {
                        BDBG_ERR(("Error Creating BVCE Output ITB Dump File (%s)", fname));
                     }
                  }

                  if ( NULL != hVceOutput->hITBDumpFile )
                  {
                     BVCE_Debug_P_WriteLogBuffer_isr(
                        hVceOutput->hITBDumpFile,
                        pAddress,
                        uiLength
                     );

                  }
#endif

      hVceOutput->state.stBufferCache.uiITBCacheValidOffset += uiLength;
      if ( hVceOutput->state.stBufferCache.uiITBCacheValidOffset >= hVceOutput->state.stITBBuffer.stOffset.uiEnd )
      {
         hVceOutput->state.stBufferCache.uiITBCacheValidOffset -= ( hVceOutput->state.stITBBuffer.stOffset.uiEnd - hVceOutput->state.stITBBuffer.stOffset.uiBase );
      }
   }
}

/* BVCE_Output_S_CDB_BufferCacheUpdate -
 *
 * will flush the cache for the region of CDB that was written
 * since the last time this function was called
 */
static
void
BVCE_Output_S_CDB_BufferCacheUpdate(
         BVCE_Output_Handle hVceOutput
         )
{
   void *pAddress;
   unsigned uiLength;

   if ( 0 == hVceOutput->state.stBufferCache.uiCDBCacheValidOffset )
   {
      hVceOutput->state.stBufferCache.uiCDBCacheValidOffset = hVceOutput->state.stCDBBuffer.stOffset.uiBase;
   }

   while ( hVceOutput->state.stBufferCache.uiCDBCacheValidOffset != hVceOutput->state.stCDBBuffer.stOffset.uiValid )
   {
      if ( hVceOutput->state.stCDBBuffer.stOffset.uiValid >= hVceOutput->state.stBufferCache.uiCDBCacheValidOffset )
      {
         uiLength = hVceOutput->state.stCDBBuffer.stOffset.uiValid - hVceOutput->state.stBufferCache.uiCDBCacheValidOffset;
      }
      else
      {
         uiLength =  hVceOutput->state.stCDBBuffer.stOffset.uiEnd - hVceOutput->state.stBufferCache.uiCDBCacheValidOffset;
      }

#if !BVCE_PLATFORM_P_SUPPORTS_ITB_NAL || BVCE_P_DUMP_OUTPUT_CDB || BVCE_PLATFORM_P_CDB_REQUIRES_BYTESWAP
#if !BVCE_P_DUMP_OUTPUT_CDB && !BVCE_PLATFORM_P_CDB_REQUIRES_BYTESWAP
      /* SW7445-163: Only flush the CDB cache if NAL Data Unit detection is enabled or we are dumping the CDB to file */
      if ( true == hVceOutput->stOpenSettings.bEnableDataUnitDetection )
#endif
      {
         void *pBaseAddress;

         pBaseAddress = BVCE_P_Buffer_LockAddress( hVceOutput->hOutputBuffers->stCDB.hBuffer );
         if ( NULL == pBaseAddress )
         {
            BDBG_ERR(("Error locking CDB memory"));
            break;
         }

         pAddress = (void*) ( (uint8_t*) pBaseAddress + ( hVceOutput->state.stBufferCache.uiCDBCacheValidOffset - hVceOutput->state.stCDBBuffer.stOffset.uiBase) );

         BVCE_P_Buffer_FlushCache_isr(
                  hVceOutput->hOutputBuffers->stCDB.hBuffer,
                  pAddress,
                  uiLength
                  );

#if BVCE_PLATFORM_P_CDB_REQUIRES_BYTESWAP
         {
            unsigned i;
            for ( i = 0; i < uiLength; i+=4 )
            {
               uint32_t* puiWord = (uint32_t*) ((uint8_t*) pAddress + i);
               *puiWord = ((*puiWord << 24) & 0xFF000000) | ((*puiWord >> 24) & 0x000000FF) | ((*puiWord >> 8) & 0x0000FF00) | ((*puiWord << 8) & 0x00FF0000);
            }

            BVCE_P_Buffer_FlushCache_isr(
                     hVceOutput->hOutputBuffers->stCDB.hBuffer,
                     pAddress,
                     uiLength
                     );
         }
#endif

         BVCE_P_Buffer_UnlockAddress( hVceOutput->hOutputBuffers->stCDB.hBuffer );

         BDBG_MODULE_MSG(BVCE_OUTPUT_CACHE, ( "CDB Flushed: %p %d bytes",
                     pAddress,
                     uiLength
                     ));
      }

#if BVCE_P_DUMP_OUTPUT_CDB
            if ( ( NULL == hVceOutput->hCDBDumpFile ) && ( NULL != hVceOutput->state.hVceCh ) )
            {
               static unsigned uiInstance[BVCE_PLATFORM_P_NUM_ENCODE_INSTANCES][BVCE_PLATFORM_P_NUM_ENCODE_CHANNELS];
               char fname[256];
               sprintf(fname, "BVCE_OUTPUT_%02d_%02d_%03d.cdb", hVceOutput->state.hVceCh->hVce->stOpenSettings.uiInstance, hVceOutput->state.hVceCh->stOpenSettings.uiInstance, uiInstance[hVceOutput->state.hVceCh->hVce->stOpenSettings.uiInstance][hVceOutput->state.hVceCh->stOpenSettings.uiInstance]);
               uiInstance[hVceOutput->state.hVceCh->hVce->stOpenSettings.uiInstance][hVceOutput->state.hVceCh->stOpenSettings.uiInstance]++;

               if ( false == BVCE_Debug_P_OpenLog( fname, &hVceOutput->hCDBDumpFile ) )
               {
                  BDBG_ERR(("Error Creating BVCE Output CDB Dump File (%s)", fname));
               }
            }

            if ( NULL != hVceOutput->hCDBDumpFile )
            {
               BVCE_Debug_P_WriteLogBuffer_isr(
                  hVceOutput->hCDBDumpFile,
                  pAddress,
                  uiLength
                  );
            }
#endif
#endif
      hVceOutput->state.stBufferCache.uiCDBCacheValidOffset += uiLength;
      if ( hVceOutput->state.stBufferCache.uiCDBCacheValidOffset >= hVceOutput->state.stCDBBuffer.stOffset.uiEnd )
      {
         hVceOutput->state.stBufferCache.uiCDBCacheValidOffset -= ( hVceOutput->state.stCDBBuffer.stOffset.uiEnd - hVceOutput->state.stCDBBuffer.stOffset.uiBase );
      }
   }
}

static
BERR_Code
BVCE_Output_S_Reset(
         BVCE_Output_Handle hVceOutput
         )
{
   /* TODO: need to figure out what to do here */
   BSTD_UNUSED( hVceOutput );
   BDBG_OBJECT_ASSERT(hVceOutput, BVCE_P_Output_Context);

   BKNI_Memset( &hVceOutput->state, 0, sizeof(hVceOutput->state) );

   /* SW7425-4410: Reset ITB index */
   BKNI_Memset( hVceOutput->stDescriptors.astIndex, 0, ( hVceOutput->stOpenSettings.uiDescriptorQueueDepth * sizeof( BVCE_P_Output_ITB_IndexEntry ) ) );

   if ( NULL != hVceOutput->hOutputBuffers )
   {
      hVceOutput->state.stITBBuffer.stOffset.uiShadowRead = BVCE_P_Buffer_GetDeviceOffset_isrsafe( hVceOutput->hOutputBuffers->stITB.hBuffer );
      hVceOutput->state.stITBBuffer.stOffset.uiValid = BVCE_P_Buffer_GetDeviceOffset_isrsafe( hVceOutput->hOutputBuffers->stITB.hBuffer );
      hVceOutput->state.stITBBuffer.stOffset.uiRead = BVCE_P_Buffer_GetDeviceOffset_isrsafe( hVceOutput->hOutputBuffers->stITB.hBuffer );
      hVceOutput->state.stReadIndex.uiShadowRead = hVceOutput->state.stITBBuffer.stOffset.uiShadowRead;

      hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead = BVCE_P_Buffer_GetDeviceOffset_isrsafe( hVceOutput->hOutputBuffers->stCDB.hBuffer );
      hVceOutput->state.stCDBBuffer.stOffset.uiValid = BVCE_P_Buffer_GetDeviceOffset_isrsafe( hVceOutput->hOutputBuffers->stCDB.hBuffer );
      hVceOutput->state.stCDBBuffer.stOffset.uiRead = BVCE_P_Buffer_GetDeviceOffset_isrsafe( hVceOutput->hOutputBuffers->stCDB.hBuffer );
   }

   BVCE_Output_S_DataUnitDetectReset( hVceOutput);
   BVCE_Output_S_BufferCacheReset( hVceOutput );

   hVceOutput->state.bNewITBEntry = true;

#if BVCE_P_POLL_FW_DATAREADY
   BREG_Write32(
      hVceOutput->hVce->handles.hReg,
      hVceOutput->hVce->stPlatformConfig.stInterrupt.uiInterruptClearRegister,
      hVceOutput->hVce->stPlatformConfig.stInterrupt.stMask.uiDataReady[hVceOutput->stOpenSettings.uiInstance]
   );
#endif

#if BVCE_P_DUMP_OUTPUT_ITB_DESC
   BVCE_Debug_P_CloseLog( hVceOutput->hITBDescDumpFile );
   hVceOutput->hITBDescDumpFile = NULL;
#endif

#if BVCE_P_DUMP_MAU_PERFORMANCE_DATA
   BVCE_Debug_P_CloseLog( hVceOutput->hMAUPerformanceDumpFile );
   hVceOutput->hMAUPerformanceDumpFile = NULL;
#endif

#if BVCE_P_DUMP_OUTPUT_ITB
   BVCE_Debug_P_CloseLog( hVceOutput->hITBDumpFile );
   hVceOutput->hITBDumpFile = NULL;
#endif

#if BVCE_P_DUMP_OUTPUT_CDB
   BVCE_Debug_P_CloseLog( hVceOutput->hCDBDumpFile );
   hVceOutput->hCDBDumpFile = NULL;
#endif

#if BVCE_P_TEST_MODE
   BVCE_Debug_P_CloseLog( hVceOutput->hDescriptorLog );
   hVceOutput->hDescriptorLog = NULL;
#endif

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BVCE_Output_Reset(
         BVCE_Output_Handle hVceOutput
         )
{
   BERR_Code rc;
   BDBG_ENTER( BVCE_Output_Reset );
   BDBG_OBJECT_ASSERT(hVceOutput, BVCE_P_Output_Context);
   BVCE_P_FUNCTION_TRACE_ENTER(0, hVceOutput->hVce, hVceOutput->stOpenSettings.uiInstance);

   BVCE_Power_P_AcquireResource(
         hVceOutput->hVce,
         BVCE_Power_Type_eClock
         );

   rc = BVCE_Output_S_Reset( hVceOutput );

   BVCE_Power_P_ReleaseResource(
         hVceOutput->hVce,
         BVCE_Power_Type_eClock
         );

   BVCE_P_FUNCTION_TRACE_LEAVE(0, hVceOutput->hVce, hVceOutput->stOpenSettings.uiInstance);
   BDBG_LEAVE( BVCE_Output_Reset );

   return BERR_TRACE( rc );
}

/* BVCE_Output_GetRegisters -
 * Returns the registers associated with the specified output hardware
 */
BERR_Code
BVCE_Output_GetRegisters(
         BVCE_Output_Handle hVceOutput,
         BAVC_VideoContextMap *pstVceOutputRegisters
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER( BVCE_Output_GetRegisters);

   BDBG_OBJECT_ASSERT(hVceOutput, BVCE_P_Output_Context);
   BDBG_ASSERT( pstVceOutputRegisters );
   BVCE_P_FUNCTION_TRACE_ENTER(0, hVceOutput->hVce, hVceOutput->stOpenSettings.uiInstance);

   if ( BVCE_P_Output_BufferAccessMode_eUnknown == hVceOutput->state.eBufferAccessMode ) hVceOutput->state.eBufferAccessMode = BVCE_P_Output_BufferAccessMode_eDirect;
   *pstVceOutputRegisters = hVceOutput->stRegisters;

   BVCE_P_FUNCTION_TRACE_LEAVE(0, hVceOutput->hVce, hVceOutput->stOpenSettings.uiInstance);
   BDBG_LEAVE( BVCE_Output_GetRegisters);
   return BERR_TRACE( rc );
}

static
void
BVCE_Output_S_FlushDirect(
   BVCE_Output_Handle hVceOutput
   )
{
   uint64_t uiCDBValid;
   uint64_t uiITBValid;
   uint64_t uiCDBRead;
   uint64_t uiITBRead;
   uint64_t uiCDBBase;
   uint64_t uiITBBase;

   BDBG_OBJECT_ASSERT(hVceOutput, BVCE_P_Output_Context);

   /* Simply set read = valid */
   uiCDBValid = BREG_ReadAddr(
      hVceOutput->hVce->handles.hReg,
      hVceOutput->stRegisters.CDB_Valid
      );

   uiITBValid = BREG_ReadAddr(
      hVceOutput->hVce->handles.hReg,
      hVceOutput->stRegisters.ITB_Valid
      );

   uiCDBRead = BREG_ReadAddr(
      hVceOutput->hVce->handles.hReg,
      hVceOutput->stRegisters.CDB_Read
      );


   uiITBRead = BREG_ReadAddr(
      hVceOutput->hVce->handles.hReg,
      hVceOutput->stRegisters.ITB_Read
      );

   uiCDBBase = BREG_ReadAddr(
      hVceOutput->hVce->handles.hReg,
      hVceOutput->stRegisters.CDB_Base
      );

   uiITBBase = BREG_ReadAddr(
      hVceOutput->hVce->handles.hReg,
      hVceOutput->stRegisters.ITB_Base
      );

   BDBG_WRN(("Direct Flush (ITB Read = "BDBG_UINT64_FMT" --> "BDBG_UINT64_FMT" (Base: "BDBG_UINT64_FMT"), CDB Read = "BDBG_UINT64_FMT" --> "BDBG_UINT64_FMT" (Base: "BDBG_UINT64_FMT")",
      BDBG_UINT64_ARG(uiITBRead),
      BDBG_UINT64_ARG(uiITBValid),
      BDBG_UINT64_ARG(uiITBBase),
      BDBG_UINT64_ARG(uiCDBRead),
      BDBG_UINT64_ARG(uiCDBValid),
      BDBG_UINT64_ARG(uiCDBBase)
   ));

   BREG_WriteAddr(
      hVceOutput->hVce->handles.hReg,
      hVceOutput->stRegisters.CDB_Read,
      uiCDBValid
   );

   BREG_WriteAddr(
      hVceOutput->hVce->handles.hReg,
      hVceOutput->stRegisters.ITB_Read,
      uiITBValid
   );
}

static
void
BVCE_Output_S_ITB_UpdatePointers(
   BVCE_Output_Handle hVceOutput
   )
{
   /* ITB Pointers */
   hVceOutput->state.stITBBuffer.stOffset.uiBase = BREG_ReadAddr(
            hVceOutput->hVce->handles.hReg,
            hVceOutput->stRegisters.ITB_Base
            );

   hVceOutput->state.stITBBuffer.stOffset.uiEnd = BREG_ReadAddr(
            hVceOutput->hVce->handles.hReg,
            hVceOutput->stRegisters.ITB_End
            );

   /* HW treats END as last valid byte before wrap.
    * It is easier for SW to treat END as the byte
    * AFTER the last valid byte before WRAP, so we
    * add one here
    */
   hVceOutput->state.stITBBuffer.stOffset.uiEnd += 1;

   hVceOutput->state.stITBBuffer.stOffset.uiValid = BREG_ReadAddr(
            hVceOutput->hVce->handles.hReg,
            hVceOutput->stRegisters.ITB_Valid
            );

   hVceOutput->state.stITBBuffer.stOffset.uiValid += 1;

   if ( hVceOutput->state.stITBBuffer.stOffset.uiValid >= hVceOutput->state.stITBBuffer.stOffset.uiEnd )
   {
      hVceOutput->state.stITBBuffer.stOffset.uiValid = hVceOutput->state.stITBBuffer.stOffset.uiBase + ( hVceOutput->state.stITBBuffer.stOffset.uiValid - hVceOutput->state.stITBBuffer.stOffset.uiEnd );
   }

   hVceOutput->state.stITBBuffer.stOffset.uiWrite = BREG_ReadAddr(
           hVceOutput->hVce->handles.hReg,
           hVceOutput->hVce->stPlatformConfig.stOutput[hVceOutput->stOpenSettings.uiInstance].stITB.uiWritePointer
           );

   hVceOutput->state.stITBBuffer.stOffset.uiRead = BREG_ReadAddr(
           hVceOutput->hVce->handles.hReg,
           hVceOutput->stRegisters.ITB_Read
           );

   BDBG_MSG( ("[%d][%d] ITB Base/End/sRead (Read)/Valid/Write = "BDBG_UINT64_FMT"/"BDBG_UINT64_FMT"/"BDBG_UINT64_FMT" ("BDBG_UINT64_FMT")/"BDBG_UINT64_FMT"/"BDBG_UINT64_FMT,
            hVceOutput->hVce->stOpenSettings.uiInstance, ( NULL != hVceOutput->state.hVceCh ) ? hVceOutput->state.hVceCh->stOpenSettings.uiInstance : 9,
            BDBG_UINT64_ARG(hVceOutput->state.stITBBuffer.stOffset.uiBase),
            BDBG_UINT64_ARG(hVceOutput->state.stITBBuffer.stOffset.uiEnd),
            BDBG_UINT64_ARG(hVceOutput->state.stITBBuffer.stOffset.uiShadowRead),
            BDBG_UINT64_ARG(hVceOutput->state.stITBBuffer.stOffset.uiRead),
            BDBG_UINT64_ARG(hVceOutput->state.stITBBuffer.stOffset.uiValid),
            BDBG_UINT64_ARG(BREG_ReadAddr(
                     hVceOutput->hVce->handles.hReg,
                     hVceOutput->hVce->stPlatformConfig.stOutput[hVceOutput->stOpenSettings.uiInstance].stITB.uiWritePointer
                     ))
            ));

   if ( NULL != hVceOutput->hVce->stDebugFifo.hDebugFifo )
   {
      BVCE_P_DebugFifo_Entry *pstEntry;
      BDBG_Fifo_Token stToken;

      pstEntry = (BVCE_P_DebugFifo_Entry *) BDBG_Fifo_GetBuffer( hVceOutput->hVce->stDebugFifo.hDebugFifo, &stToken );
      if ( NULL != pstEntry )
      {
         pstEntry->stMetadata.eType = BVCE_DebugFifo_EntryType_eITBOffsets;
         pstEntry->stMetadata.uiInstance = hVceOutput->hVce->stOpenSettings.uiInstance;
         pstEntry->stMetadata.uiChannel = hVceOutput->stOpenSettings.uiInstance;
         pstEntry->stMetadata.uiTimestamp = 0;
         ( NULL != hVceOutput->hVce->hTimer ) ? BTMR_ReadTimer( hVceOutput->hVce->hTimer, &pstEntry->stMetadata.uiTimestamp ) : 0;
         BKNI_Memcpy(&pstEntry->data.stOffset, &hVceOutput->state.stITBBuffer.stOffset, sizeof( BVCE_P_Buffer_Offsets ));
         BDBG_Fifo_CommitBuffer( &stToken );
      }
   }

   if ( true == hVceOutput->state.stITBBuffer.bPreviousValidOffsetValid )
   {
      BDBG_MODULE_MSG( BVCE_OUTPUT_PARSER, ("%u new ITB entries (sRead (Read)/Valid = "BDBG_UINT64_FMT" ("BDBG_UINT64_FMT")/"BDBG_UINT64_FMT")",
         (unsigned) ((hVceOutput->state.stITBBuffer.stOffset.uiValid >= hVceOutput->state.stITBBuffer.uiPreviousValidOffset) ?
            (hVceOutput->state.stITBBuffer.stOffset.uiValid - hVceOutput->state.stITBBuffer.uiPreviousValidOffset)/16 :
            ((hVceOutput->state.stITBBuffer.stOffset.uiEnd - hVceOutput->state.stITBBuffer.uiPreviousValidOffset) + (hVceOutput->state.stITBBuffer.stOffset.uiValid - hVceOutput->state.stITBBuffer.stOffset.uiBase))/16),
         BDBG_UINT64_ARG(hVceOutput->state.stITBBuffer.stOffset.uiShadowRead),
         BDBG_UINT64_ARG(hVceOutput->state.stITBBuffer.stOffset.uiRead),
         BDBG_UINT64_ARG(hVceOutput->state.stITBBuffer.stOffset.uiValid)
         ));
   }
   hVceOutput->state.stITBBuffer.uiPreviousValidOffset = hVceOutput->state.stITBBuffer.stOffset.uiValid;
   hVceOutput->state.stITBBuffer.bPreviousValidOffsetValid = true;
}

static
void
BVCE_Output_S_CDB_UpdatePointers(
   BVCE_Output_Handle hVceOutput
   )
{
   /* CDB Pointers */
   hVceOutput->state.stCDBBuffer.stOffset.uiBase = BREG_ReadAddr(
            hVceOutput->hVce->handles.hReg,
            hVceOutput->stRegisters.CDB_Base
            );

   hVceOutput->state.stCDBBuffer.stOffset.uiEnd = BREG_ReadAddr(
            hVceOutput->hVce->handles.hReg,
            hVceOutput->stRegisters.CDB_End
            );
   /* HW treats END as last valid byte before wrap.
    * It is easier for SW to treat END as the byte
    * AFTER the last valid byte before WRAP, so we
    * add one here
    */
   hVceOutput->state.stCDBBuffer.stOffset.uiEnd += 1;

   hVceOutput->state.stCDBBuffer.stOffset.uiValid = BREG_ReadAddr(
            hVceOutput->hVce->handles.hReg,
            hVceOutput->stRegisters.CDB_Valid
            );

#if BVCE_P_DUMP_OUTPUT_ITB_DESC
   hVceOutput->state.stCDBBuffer.hw.uiValidOffset = hVceOutput->state.stCDBBuffer.stOffset.uiValid;
#endif

   hVceOutput->state.stCDBBuffer.stOffset.uiValid += 1;

   if ( hVceOutput->state.stCDBBuffer.stOffset.uiValid >= hVceOutput->state.stCDBBuffer.stOffset.uiEnd )
   {
      hVceOutput->state.stCDBBuffer.stOffset.uiValid = hVceOutput->state.stCDBBuffer.stOffset.uiBase + ( hVceOutput->state.stCDBBuffer.stOffset.uiValid - hVceOutput->state.stCDBBuffer.stOffset.uiEnd );
   }

#if BVCE_PLATFORM_P_CDB_REQUIRES_BYTESWAP
   {
      if ( false == hVceOutput->state.bEOSITBEntryIndexed )
      {
         /* Adjust to ensure valid is on a 4 byte boundary to support eventual byte swap */
         hVceOutput->state.stCDBBuffer.stOffset.uiValid -= ( hVceOutput->state.stCDBBuffer.stOffset.uiValid % 32 );
         if ( hVceOutput->state.stCDBBuffer.stOffset.uiValid < hVceOutput->state.stCDBBuffer.stOffset.uiBase )
         {
            hVceOutput->state.stCDBBuffer.stOffset.uiValid = hVceOutput->state.stCDBBuffer.stOffset.uiEnd - ( hVceOutput->state.stCDBBuffer.stOffset.uiBase - hVceOutput->state.stCDBBuffer.stOffset.uiValid );
         }
         BDBG_ASSERT( 0 == ( hVceOutput->state.stCDBBuffer.stOffset.uiValid % 32 ) );
      }
   }
#endif

   hVceOutput->state.stCDBBuffer.stOffset.uiWrite = BREG_ReadAddr(
           hVceOutput->hVce->handles.hReg,
           hVceOutput->hVce->stPlatformConfig.stOutput[hVceOutput->stOpenSettings.uiInstance].stCDB.uiWritePointer
           );

#if BVCE_P_DUMP_OUTPUT_ITB_DESC
   hVceOutput->state.stCDBBuffer.hw.uiWriteOffset = hVceOutput->state.stCDBBuffer.stOffset.uiWrite;
#endif

   hVceOutput->state.stCDBBuffer.stOffset.uiRead = BREG_ReadAddr(
           hVceOutput->hVce->handles.hReg,
           hVceOutput->stRegisters.CDB_Read
           );

#if BVCE_P_DUMP_OUTPUT_ITB_DESC
   hVceOutput->state.stCDBBuffer.hw.uiReadOffset = hVceOutput->state.stCDBBuffer.stOffset.uiRead;

   hVceOutput->state.stCDBBuffer.hw.uiDepth = BREG_ReadAddr(
       hVceOutput->hVce->handles.hReg,
       hVceOutput->hVce->stPlatformConfig.stDebug.uiCDBDepth[hVceOutput->stOpenSettings.uiInstance] );
#endif

   BDBG_MSG( ("[%d][%d] CDB Base/End/sRead (Read)/sValid (Valid)/Write = "BDBG_UINT64_FMT"/"BDBG_UINT64_FMT"/"BDBG_UINT64_FMT" ("BDBG_UINT64_FMT")/"BDBG_UINT64_FMT" ("BDBG_UINT64_FMT")/"BDBG_UINT64_FMT,
            hVceOutput->hVce->stOpenSettings.uiInstance, ( NULL != hVceOutput->state.hVceCh ) ? hVceOutput->state.hVceCh->stOpenSettings.uiInstance : 9,
            BDBG_UINT64_ARG(hVceOutput->state.stCDBBuffer.stOffset.uiBase),
            BDBG_UINT64_ARG(hVceOutput->state.stCDBBuffer.stOffset.uiEnd),
            BDBG_UINT64_ARG(hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead),
            BDBG_UINT64_ARG(hVceOutput->state.stCDBBuffer.stOffset.uiRead),
            BDBG_UINT64_ARG(hVceOutput->state.stCDBBuffer.uiShadowValidOffset),
            BDBG_UINT64_ARG(hVceOutput->state.stCDBBuffer.stOffset.uiValid),
            BDBG_UINT64_ARG(BREG_ReadAddr(
                     hVceOutput->hVce->handles.hReg,
                     hVceOutput->hVce->stPlatformConfig.stOutput[hVceOutput->stOpenSettings.uiInstance].stCDB.uiWritePointer
                     ))
            ));

   if ( NULL != hVceOutput->hVce->stDebugFifo.hDebugFifo )
   {
      BVCE_P_DebugFifo_Entry *pstEntry;
      BDBG_Fifo_Token stToken;

      pstEntry = (BVCE_P_DebugFifo_Entry *) BDBG_Fifo_GetBuffer( hVceOutput->hVce->stDebugFifo.hDebugFifo, &stToken );
      if ( NULL != pstEntry )
      {
         pstEntry->stMetadata.eType = BVCE_DebugFifo_EntryType_eCDBOffsets;
         pstEntry->stMetadata.uiInstance = hVceOutput->hVce->stOpenSettings.uiInstance;
         pstEntry->stMetadata.uiChannel = hVceOutput->stOpenSettings.uiInstance;
         pstEntry->stMetadata.uiTimestamp = 0;
         ( NULL != hVceOutput->hVce->hTimer ) ? BTMR_ReadTimer( hVceOutput->hVce->hTimer, &pstEntry->stMetadata.uiTimestamp ) : 0;
         BKNI_Memcpy(&pstEntry->data.stOffset, &hVceOutput->state.stCDBBuffer.stOffset, sizeof( BVCE_P_Buffer_Offsets ));
         BDBG_Fifo_CommitBuffer( &stToken );
      }
   }

   if ( true == hVceOutput->state.stCDBBuffer.bPreviousValidOffsetValid )
   {
      BDBG_MODULE_MSG( BVCE_OUTPUT_PARSER, ("%u new CDB bytes (sRead (Read)/sValid (Valid)= "BDBG_UINT64_FMT" ("BDBG_UINT64_FMT")/"BDBG_UINT64_FMT" ("BDBG_UINT64_FMT"))",
         (unsigned) ((hVceOutput->state.stCDBBuffer.stOffset.uiValid >= hVceOutput->state.stCDBBuffer.uiPreviousValidOffset) ?
            (hVceOutput->state.stCDBBuffer.stOffset.uiValid - hVceOutput->state.stCDBBuffer.uiPreviousValidOffset) :
            ((hVceOutput->state.stCDBBuffer.stOffset.uiEnd - hVceOutput->state.stCDBBuffer.uiPreviousValidOffset) + (hVceOutput->state.stCDBBuffer.stOffset.uiValid - hVceOutput->state.stCDBBuffer.stOffset.uiBase))),
         BDBG_UINT64_ARG(hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead),
         BDBG_UINT64_ARG(hVceOutput->state.stCDBBuffer.stOffset.uiRead),
         BDBG_UINT64_ARG(hVceOutput->state.stCDBBuffer.uiShadowValidOffset),
         BDBG_UINT64_ARG(hVceOutput->state.stCDBBuffer.stOffset.uiValid)
         ));
   }
   hVceOutput->state.stCDBBuffer.uiPreviousValidOffset = hVceOutput->state.stCDBBuffer.stOffset.uiValid;
   hVceOutput->state.stCDBBuffer.bPreviousValidOffsetValid = true;
}

#ifndef BVCE_PLATFORM_P_SUPPORTS_ITB_EOS
static
void
BVCE_Output_S_ITB_CheckForEOS(
   BAVC_VideoBufferDescriptor *pVideoDescriptor
   )
{
   if ( ( 0 == pVideoDescriptor->stCommon.iSHR )
        && ( 0 == pVideoDescriptor->stCommon.uiTicksPerBit )
        && ( 0 == pVideoDescriptor->stCommon.uiPTS )
        && ( 0 == pVideoDescriptor->uiDTS )
        && ( 0 == pVideoDescriptor->stCommon.uiOriginalPTS )
      )
   {
      pVideoDescriptor->stCommon.uiFlags = BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOS;
      pVideoDescriptor->uiVideoFlags = 0;
   }
}
#endif

static
void
BVCE_Output_S_VerifyESCR(
   BVCE_Output_Handle hVceOutput,
   BAVC_VideoBufferDescriptor *pVideoDescriptor,
   unsigned uiEtsDtsOffset
   )
{
   unsigned uiMinESCR = (pVideoDescriptor->uiDTS*300) - uiEtsDtsOffset;
   signed iDiff = pVideoDescriptor->stCommon.uiESCR - uiMinESCR;

#if !BDBG_DEBUG_BUILD
   BSTD_UNUSED( hVceOutput );
#endif

   if ( iDiff < 0 )
   {
#if ( ( BVCE_P_CORE_MAJOR != 0 ) && ( BVCE_P_CORE_MAJOR != 1 ) )
      BDBG_MSG(("[%d][%d] Detected ESCR < minESCR (%08x < %08x) (DTS(90Khz): "BDBG_UINT64_FMT". EtsDtsDelta: %08x)",
         hVceOutput->hVce->stOpenSettings.uiInstance, ( NULL != hVceOutput->state.hVceCh ) ? hVceOutput->state.hVceCh->stOpenSettings.uiInstance : 9,
         pVideoDescriptor->stCommon.uiESCR,
         uiMinESCR,
         BDBG_UINT64_ARG(pVideoDescriptor->uiDTS),
         uiEtsDtsOffset
         ));
#else
      BDBG_MSG(("[%d][%d] Detected ESCR < minESCR (%08x < %08x) (DTS(90Khz): "BDBG_UINT64_FMT". EtsDtsDelta: %08x)",
         hVceOutput->hVce->stOpenSettings.uiInstance, ( NULL != hVceOutput->state.hVceCh ) ? hVceOutput->state.hVceCh->stOpenSettings.uiInstance : 9,
         pVideoDescriptor->stCommon.uiESCR,
         uiMinESCR,
         BDBG_UINT64_ARG(pVideoDescriptor->uiDTS),
         uiEtsDtsOffset
         ));
#endif

#if ( ( BVCE_P_CORE_MAJOR == 0 ) || ( BVCE_P_CORE_MAJOR == 1 ) )
      /* SW7425-4707: Reset the ESCR to minESCR to account for bitrate underflow.
       * In legacy 7425/7435 encoders, the FW's ESCR reset logic kicks in only after
       * it detects that the ESCR < minESCR, so there are frames that violate the
       * minESCR limit.  Newer HW (7445+) performs the ESCR reset in CABAC HW */
      pVideoDescriptor->stCommon.uiESCR = uiMinESCR;
#endif
   }
}

static
void
BVCE_Output_S_ITB_DetectNewFrame(
   BVCE_Output_Handle hVceOutput
   )
{
   if ( ( hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].uiSizeInITB > 0 )
        && ( ( 0 != ( hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].stFrameDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_START ) )
             || ( 0 != ( hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].stFrameDescriptor.uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DATA_UNIT_START ) )
      ))
   {
      BAVC_VideoBufferDescriptor *pVideoDescriptor = &hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].stFrameDescriptor;
      unsigned uiSizeInITB = hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].uiSizeInITB;

#ifndef BVCE_PLATFORM_P_SUPPORTS_ITB_EOS
      BVCE_Output_S_ITB_CheckForEOS(pVideoDescriptor);
#endif
      {
         bool bIsEOS = ( 0 != ( pVideoDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOS ) );
         bool bIsRAP = ( 0 != ( pVideoDescriptor->uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_RAP ) );

         /* Verify ESCR >= DTS - EtsDtsOffset */
         if ( ( true == hVceOutput->state.bChannelStatusValid )
              && ( 0 != ( pVideoDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_ESCR_VALID ) )
              && ( 0 != ( pVideoDescriptor->uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DTS_VALID ) )
              && ( false == hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].bHeaderOnly ) )
         {
            BVCE_Output_S_VerifyESCR( hVceOutput, pVideoDescriptor, hVceOutput->state.stChannelStatus.uiEtsDtsOffset );
         }

         /* SW7425-5765: In cases when an immediate/normal stop is requested, but the CDB is full,
          * the EOS may still be pending when the start command is issued.  Depending on timing,
          * the EOS may incorrectly come as the first descriptor in the new transcode session.
          * To prevent this, we don't allow an EOS or non-RAP frame to be the first frame we send
          * after a start encode.
          */
         if ( ( true == hVceOutput->state.bFirstFrameSeen )
              || ( ( false == bIsEOS )
                   && ( true == bIsRAP ) ) )
         {
            hVceOutput->state.stITBBuffer.uiIndexWriteOffset++;
            hVceOutput->state.stITBBuffer.uiIndexWriteOffset %= hVceOutput->stOpenSettings.uiDescriptorQueueDepth;
            hVceOutput->state.bFirstFrameSeen = true;
            hVceOutput->state.bEOSITBEntryIndexed = bIsEOS;
         }

         BKNI_Memset( &hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset], 0, sizeof( BVCE_P_Output_ITB_IndexEntry ) );

         if ( false == hVceOutput->state.bFirstFrameSeen)
         {
            if ( true == bIsEOS )
            {
               BDBG_WRN(("Ignored stale EOS"));
            }
            else if ( false == bIsRAP )
            {
               BDBG_WRN(("Ignored stale non-RAP start frame"));
            }
            hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].uiSizeInITB = uiSizeInITB;
         }
      }
   }
}

typedef enum BVCE_OUTPUT_P_ITB_IndexingState
{
   BVCE_OUTPUT_P_ITB_IndexingState_eBaseEntry = 0x20, /* Indicates start of new frame (0x20) */
   BVCE_OUTPUT_P_ITB_IndexingState_eBaseOffsetEntry = 0x28, /* Indicates start of new frame (0x28) with offset from beginning of CDB */
   BVCE_OUTPUT_P_ITB_IndexingState_eTimeStampEntry = 0x21, /* PTS/DTS Entry (0x21) */
   BVCE_OUTPUT_P_ITB_IndexingState_eBitrateEntry = 0x60, /* Bitrate Entry (0x60) */
   BVCE_OUTPUT_P_ITB_IndexingState_eESCREntry = 0x61, /* ESCR Entry (0x61) */
   BVCE_OUTPUT_P_ITB_IndexingState_eCRCEntry = 0x6C, /* CRC Entry (0x6C) */
   BVCE_OUTPUT_P_ITB_IndexingState_eSTCEntry = 0x68, /* STC Entry (0x68) */
#if ((BCHP_CHIP == 7278) && (BCHP_VER == BCHP_VER_A0)) /* 7278 A0 Compatibility */
   BVCE_OUTPUT_P_ITB_IndexingState_eNALAddressDataEntry = 0x6D, /* NAL Address and Data Entry (0x6D) */
   BVCE_OUTPUT_P_ITB_IndexingState_eNALDataEntry = 0x6E, /* NAL Data Entry (0x6E) */
   BVCE_OUTPUT_P_ITB_IndexingState_eEOSEntry = 0x6F, /* EOS Entry (0x6F) */
#else
   BVCE_OUTPUT_P_ITB_IndexingState_eNALAddressDataEntry = 0x62, /* NAL Address and Data Entry (0x62) */
   BVCE_OUTPUT_P_ITB_IndexingState_eNALDataEntry = 0x63, /* NAL Data Entry (0x63) */
   BVCE_OUTPUT_P_ITB_IndexingState_eEOSEntry = 0x6D, /* EOS Entry (0x6D) */
#endif
   BVCE_OUTPUT_P_ITB_IndexingState_eNULLEntry = 0x00 /* NULL Entry (0x00) */
#if BVCE_P_DUMP_MAU_PERFORMANCE_DATA
   ,BVCE_OUTPUT_P_ITB_IndexingState_eMAUPerformance0Entry = 0xF0, /* MAU Performance Data 0 */
   BVCE_OUTPUT_P_ITB_IndexingState_eMAUPerformance1Entry = 0xF1, /* MAU Performance Data 1 */
   BVCE_OUTPUT_P_ITB_IndexingState_eMAUPerformance2Entry = 0xF2, /* MAU Performance Data 2 */
   BVCE_OUTPUT_P_ITB_IndexingState_eMAUPerformance3Entry = 0xF3 /* MAU Performance Data 3 */
#endif
} BVCE_OUTPUT_P_ITB_IndexingState;

static
void
BVCE_Output_S_ITB_ExtractNALData(
   BAVC_VideoBufferDescriptor *pVideoDescriptor,
   const void* pITBEntry,
   BAVC_VideoCompressionStd eProtocol
   )
{
   BDBG_MODULE_MSG( BVCE_OUTPUT_ITB_INDEX,("     numDataBytes: %u, start: %u, end: %u",
                                           BVCE_P_ITBEntry_GetNalData_NumDataBytes(pITBEntry),
                                           BVCE_P_ITBEntry_GetNalData_NalDataStart(pITBEntry),
                                           BVCE_P_ITBEntry_GetNalData_NalDataEnd(pITBEntry)
                       ));

   {
      unsigned i;
      unsigned uiEntryType = BVCE_P_ITBEntry_GetEntryType(pITBEntry);
      unsigned uiMaxData = ( BVCE_OUTPUT_P_ITB_IndexingState_eNALDataEntry == uiEntryType ) ? 12 : 8;
      unsigned uiNumBytes = ( 0 != BVCE_P_ITBEntry_GetNalData_NalDataEnd(pITBEntry) ) ? BVCE_P_ITBEntry_GetNalData_NumDataBytes(pITBEntry) : uiMaxData;
      BDBG_ASSERT(uiNumBytes <= uiMaxData);
      for ( i = 0; i < uiNumBytes; i++ )
      {
#if ((BCHP_CHIP == 7278) && (BCHP_VER == BCHP_VER_A0)) /* 7278 A0 Compatibility */
         uint32_t uiValue = ((uint32_t*) pITBEntry)[BVCE_P_ITBENTRY_NALDATA_DATA_MAXOFFSET-(i/4)];
#else
         unsigned uiDataStartOffset = ( BVCE_OUTPUT_P_ITB_IndexingState_eNALDataEntry == uiEntryType ) ? BVCE_P_ITBENTRY_NALDATA_DATA_OFFSET : BVCE_P_ITBENTRY_NALADDR_DATA_OFFSET;
         uint32_t uiValue = ((uint32_t*) pITBEntry)[uiDataStartOffset + (i/4)];
#endif
         uint8_t uiByte = (uiValue >> (((i%4))*8)) & 0xFF;
         if ( ( i == 0 ) && ( 0 != BVCE_P_ITBEntry_GetNalData_NalDataStart(pITBEntry) ) )
         {
            uint8_t uiDataUnitType = 0;

            switch ( eProtocol )
            {
            case BAVC_VideoCompressionStd_eH264:
               uiDataUnitType = uiByte & 0x1F;
               break;

            case BAVC_VideoCompressionStd_eH265:
               uiDataUnitType = (uiByte >> 1) & 0x3F;
               break;

            default:
               uiDataUnitType = uiByte;
               break;
            }

            BDBG_MODULE_MSG( BVCE_OUTPUT_ITB_INDEX,("      Data[%u]: %02x (NALU Type: %u)", i, uiByte, uiDataUnitType ) );
            pVideoDescriptor->uiVideoFlags |= BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DATA_UNIT_START;
            pVideoDescriptor->uiDataUnitType = uiByte;
         }
         else
         {
            BDBG_MODULE_MSG( BVCE_OUTPUT_ITB_INDEX,("      Data[%u]: %02x", i, uiByte) );
         }
      }
   }
}

static
void
BVCE_Output_S_ITB_ParseEntry(
      void* pITBEntry,
      BAVC_VideoBufferDescriptor *pVideoDescriptor
      )
{
   BVCE_OUTPUT_P_ITB_IndexingState eIndexState = BVCE_P_ITBEntry_GetEntryType(pITBEntry);

   switch ( eIndexState )
   {
      case BVCE_OUTPUT_P_ITB_IndexingState_eBaseEntry:
      case BVCE_OUTPUT_P_ITB_IndexingState_eBaseOffsetEntry:
         pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_START;
         break;

      case BVCE_OUTPUT_P_ITB_IndexingState_eTimeStampEntry:
         if ( 0 != BVCE_P_ITBEntry_GetIFrame(pITBEntry) )
         {
            pVideoDescriptor->uiVideoFlags |= BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_RAP;
         }
         pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_PTS_VALID;
         pVideoDescriptor->stCommon.uiPTS = BVCE_P_ITBEntry_GetPTS(pITBEntry);

         pVideoDescriptor->uiVideoFlags |= BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DTS_VALID;
         pVideoDescriptor->uiDTS = BVCE_P_ITBEntry_GetDTS(pITBEntry);
         break;

      case BVCE_OUTPUT_P_ITB_IndexingState_eBitrateEntry:
         pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_TICKSPERBIT_VALID;
         pVideoDescriptor->stCommon.uiTicksPerBit = BVCE_P_ITBEntry_GetTicksPerBit(pITBEntry);

         pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_SHR_VALID;
         pVideoDescriptor->stCommon.iSHR = BVCE_P_ITBEntry_GetSHR(pITBEntry);
         break;

      case BVCE_OUTPUT_P_ITB_IndexingState_eESCREntry:
         pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_ORIGINALPTS_VALID;
         pVideoDescriptor->stCommon.uiOriginalPTS = BVCE_P_ITBEntry_GetOriginalPTS(pITBEntry);

         pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_ESCR_VALID;
         pVideoDescriptor->stCommon.uiESCR = BVCE_P_ITBEntry_GetESCR(pITBEntry);

         /* SW7445-2896: Set segment start flag */
         if ( 0 != ( ( BVCE_P_ITBEntry_GetMetadata(pITBEntry) >> 30 ) & 0x1 ) )
         {
            pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_SEGMENT_START;
         }

         /* SW7425-5925: Set channel change flag */
         if ( 0 != ( ( BVCE_P_ITBEntry_GetMetadata(pITBEntry) >> 31 ) & 0x1 ) )
         {
            pVideoDescriptor->stCommon.uiFlags &= ~BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_ORIGINALPTS_VALID;
         }
         break;

      case BVCE_OUTPUT_P_ITB_IndexingState_eCRCEntry:
         break;

      case BVCE_OUTPUT_P_ITB_IndexingState_eSTCEntry:
         if ( 0 != ( pVideoDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_START ) )
         {
            pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_STCSNAPSHOT_VALID;
            pVideoDescriptor->stCommon.uiSTCSnapshot = BVCE_P_ITBEntry_GetStcSnapshot(pITBEntry);
         }
         break;

      case BVCE_OUTPUT_P_ITB_IndexingState_eNALAddressDataEntry:
         break;
      case BVCE_OUTPUT_P_ITB_IndexingState_eNALDataEntry:
         break;

      case BVCE_OUTPUT_P_ITB_IndexingState_eEOSEntry:
         pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_START;
         pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_ESCR_VALID;
         pVideoDescriptor->stCommon.uiESCR = BVCE_P_ITBEntry_GetEOSESCR(pITBEntry);

         pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOS;
         break;

      case BVCE_OUTPUT_P_ITB_IndexingState_eNULLEntry:
         break;

#if BVCE_P_DUMP_MAU_PERFORMANCE_DATA
      case BVCE_OUTPUT_P_ITB_IndexingState_eMAUPerformance0Entry:
      case BVCE_OUTPUT_P_ITB_IndexingState_eMAUPerformance1Entry:
      case BVCE_OUTPUT_P_ITB_IndexingState_eMAUPerformance2Entry:
      case BVCE_OUTPUT_P_ITB_IndexingState_eMAUPerformance3Entry:
         break;
#endif

      default:
         break;
   }
}

static
void
BVCE_Output_S_ITB_UpdateIndex(
   BVCE_Output_Handle hVceOutput
   )
{
   uint64_t uiReadOffset = hVceOutput->state.stITBBuffer.stOffset.uiRead;
   void *pITBBufferCached = NULL;

   if ( true == hVceOutput->state.stITBBuffer.bReadHackDone )
   {
      uiReadOffset++;
      if ( uiReadOffset >= hVceOutput->state.stITBBuffer.stOffset.uiEnd )
      {
         uiReadOffset -= ( hVceOutput->state.stITBBuffer.stOffset.uiEnd - hVceOutput->state.stITBBuffer.stOffset.uiBase );
      }
   }

   /* Skip ahead to the new ITB entries */
   {
      unsigned uiIndexReadOffset = hVceOutput->state.stITBBuffer.uiIndexReadOffset;

      while ( uiIndexReadOffset != hVceOutput->state.stITBBuffer.uiIndexWriteOffset )
      {
         uiReadOffset += hVceOutput->stDescriptors.astIndex[uiIndexReadOffset].uiSizeInITB;

         if ( uiReadOffset >= hVceOutput->state.stITBBuffer.stOffset.uiEnd )
         {
            uiReadOffset -= ( hVceOutput->state.stITBBuffer.stOffset.uiEnd - hVceOutput->state.stITBBuffer.stOffset.uiBase );
         }

         uiIndexReadOffset++;
         uiIndexReadOffset %= hVceOutput->stOpenSettings.uiDescriptorQueueDepth;
      }
   }

   /* Scan ITB for entries */
   {
      if ( 0 != ( ( hVceOutput->state.stITBBuffer.stOffset.uiValid - uiReadOffset ) % 16 ) )
      {
         BDBG_ERR( ("ITB Base/End/sRead (Read)/Valid/Write = "BDBG_UINT64_FMT"/"BDBG_UINT64_FMT"/"BDBG_UINT64_FMT" ("BDBG_UINT64_FMT")/"BDBG_UINT64_FMT"/"BDBG_UINT64_FMT" ("BDBG_UINT64_FMT" %d)",
                  BDBG_UINT64_ARG(hVceOutput->state.stITBBuffer.stOffset.uiBase),
                  BDBG_UINT64_ARG(hVceOutput->state.stITBBuffer.stOffset.uiEnd),
                  BDBG_UINT64_ARG(hVceOutput->state.stITBBuffer.stOffset.uiShadowRead),
                  BDBG_UINT64_ARG(hVceOutput->state.stITBBuffer.stOffset.uiRead),
                  BDBG_UINT64_ARG(hVceOutput->state.stITBBuffer.stOffset.uiValid),
                  BDBG_UINT64_ARG(BREG_ReadAddr(
                           hVceOutput->hVce->handles.hReg,
                           hVceOutput->hVce->stPlatformConfig.stOutput[hVceOutput->stOpenSettings.uiInstance].stITB.uiWritePointer
                           )),
                  BDBG_UINT64_ARG(uiReadOffset),
                  hVceOutput->state.stITBBuffer.bReadHackDone
                  ));
      }

      BDBG_ASSERT( 0 == ( ( hVceOutput->state.stITBBuffer.stOffset.uiValid - uiReadOffset ) % 16 ) );

      if ( uiReadOffset != hVceOutput->state.stITBBuffer.stOffset.uiValid )
      {
         if ( ( false == hVceOutput->state.bChannelStatusValid ) && ( NULL != hVceOutput->state.hVceCh ) )
         {
            BERR_Code rc = BERR_UNKNOWN;

            rc = BVCE_Channel_GetStatus(
                              hVceOutput->state.hVceCh,
                              &hVceOutput->state.stChannelStatus
                           );
            BERR_TRACE( rc );

            if ( BERR_SUCCESS == rc ) hVceOutput->state.bChannelStatusValid = true;
         }
      }

      pITBBufferCached = BVCE_P_Buffer_LockAddress( hVceOutput->hOutputBuffers->stITB.hBuffer );
      if ( NULL == pITBBufferCached )
      {
         BDBG_ERR(("Error locking ITB memory block"));
         return;
      }

      while ( uiReadOffset != hVceOutput->state.stITBBuffer.stOffset.uiValid ) /* Until end of ITB is reached */
      {
         void* pITBEntry = (void*) (((uint8_t *)pITBBufferCached) + (uiReadOffset - hVceOutput->state.stITBBuffer.stOffset.uiBase));
         BVCE_OUTPUT_P_ITB_IndexingState eIndexState = BVCE_P_ITBEntry_GetEntryType(pITBEntry);
         BAVC_VideoBufferDescriptor *pVideoDescriptor = &hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].stFrameDescriptor;

         /* Check of index is full */
         {
            unsigned uiWriteOffsetNext = (hVceOutput->state.stITBBuffer.uiIndexWriteOffset + 1) % hVceOutput->stOpenSettings.uiDescriptorQueueDepth;
            if ( uiWriteOffsetNext == hVceOutput->state.stITBBuffer.uiIndexReadOffset )
            {
               break;
            }
         }

         if ( NULL != hVceOutput->hVce->stDebugFifo.hDebugFifo )
         {
            BVCE_P_DebugFifo_Entry *pstEntry;
            BDBG_Fifo_Token stToken;

            pstEntry = (BVCE_P_DebugFifo_Entry *) BDBG_Fifo_GetBuffer( hVceOutput->hVce->stDebugFifo.hDebugFifo, &stToken );
            if ( NULL != pstEntry )
            {
               pstEntry->stMetadata.eType = BVCE_DebugFifo_EntryType_eITB;
               pstEntry->stMetadata.uiInstance = hVceOutput->hVce->stOpenSettings.uiInstance;
               pstEntry->stMetadata.uiChannel = hVceOutput->stOpenSettings.uiInstance;
               pstEntry->stMetadata.uiTimestamp = 0;
               ( NULL != hVceOutput->hVce->hTimer ) ? BTMR_ReadTimer( hVceOutput->hVce->hTimer, &pstEntry->stMetadata.uiTimestamp ) : 0;
               BKNI_Memcpy(pstEntry->data.stITB.auiEntry, pITBEntry, 16);
               pstEntry->data.stITB.uiReadOffset = uiReadOffset;
               BDBG_Fifo_CommitBuffer( &stToken );
            }
         }

         switch ( eIndexState )
         {
            case BVCE_OUTPUT_P_ITB_IndexingState_eBaseEntry:
            case BVCE_OUTPUT_P_ITB_IndexingState_eBaseOffsetEntry:
               BVCE_Output_S_ITB_DetectNewFrame( hVceOutput );
               pVideoDescriptor = &hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].stFrameDescriptor;

               BDBG_MODULE_MSG( BVCE_OUTPUT_ITB_INDEX,("%3d: BASE", hVceOutput->state.stITBBuffer.uiIndexWriteOffset) );
               hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].uiCDBAddress = BVCE_P_ITBEntry_GetCDBAddress(pITBEntry);
#ifdef BVCE_PLATFORM_P_SUPPORTS_64_BIT
               /* On 64-bit platforms, the CDB Address in the ITB is relative to the start if the CDB (i.e. no longer an absolute offset)
                  So, to be compatible with the 32-bit parsing code, we add the base CDB offset back in */
               hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].uiCDBAddress += hVceOutput->state.stCDBBuffer.stOffset.uiBase;
#endif
               BDBG_ASSERT( 0 != hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].uiCDBAddress );
               hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].bError = BVCE_P_ITBEntry_GetError(pITBEntry);

               BVCE_Output_S_ITB_ParseEntry( pITBEntry, pVideoDescriptor );

               /* SW7425-4821: Fixed ITB/CDB parser issue when first picture's CDB address does not start at the beginning of the CDB.
                * This can happen during a stop/start sequence.  Now, the shadow read/valid offsets are initialized to the CDB address
                * contained in the first ITB entry.
                */
               if ( false == hVceOutput->state.bReadOffsetInitialized )
               {
                  if ( hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead != hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].uiCDBAddress )
                  {
                     hVceOutput->state.stCDBBuffer.stOffset.uiRead = hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].uiCDBAddress;
                     hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead = hVceOutput->state.stCDBBuffer.stOffset.uiRead;
                     hVceOutput->state.stCDBBuffer.uiShadowValidOffset = hVceOutput->state.stCDBBuffer.stOffset.uiRead;

                     BREG_WriteAddr(
                              hVceOutput->hVce->handles.hReg,
                              hVceOutput->stRegisters.CDB_Read,
                              hVceOutput->state.stCDBBuffer.stOffset.uiRead
                              );
                  }

                  hVceOutput->state.bReadOffsetInitialized = true;
               }
               break;

            case BVCE_OUTPUT_P_ITB_IndexingState_eTimeStampEntry:
               BDBG_MODULE_MSG( BVCE_OUTPUT_ITB_INDEX,("%3d: TIMESTAMP", hVceOutput->state.stITBBuffer.uiIndexWriteOffset) );
               BVCE_Output_S_ITB_ParseEntry( pITBEntry, pVideoDescriptor );
               if ( 0 != BVCE_P_ITBEntry_GetIFrame(pITBEntry) )
               {
                  if ( ( NULL != hVceOutput->state.hVceCh )
                       && ( false == hVceOutput->state.hVceCh->stStartEncodeSettings.stRateControl.stSegmentMode.bEnable ) )

                  {
                     /* SW7445-2896: Set segment start flag */
                     BDBG_MSG(("[%d][%d] Segment Start ITB Seen", hVceOutput->hVce->stOpenSettings.uiInstance, hVceOutput->state.hVceCh->stOpenSettings.uiInstance));
                     pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_SEGMENT_START;
                  }
               }

               if ( 0 != BVCE_P_ITBEntry_GetHDROnly(pITBEntry) )
               {
                  BDBG_MSG(("[%d][%d] Header Only ITB Seen", hVceOutput->hVce->stOpenSettings.uiInstance, ( NULL != hVceOutput->state.hVceCh ) ? hVceOutput->state.hVceCh->stOpenSettings.uiInstance : 9));
                  hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].bHeaderOnly = true;
               }
               break;

            case BVCE_OUTPUT_P_ITB_IndexingState_eBitrateEntry:
               BDBG_MODULE_MSG( BVCE_OUTPUT_ITB_INDEX,("%3d: BITRATE", hVceOutput->state.stITBBuffer.uiIndexWriteOffset) );
               BVCE_Output_S_ITB_ParseEntry( pITBEntry, pVideoDescriptor );
               break;

            case BVCE_OUTPUT_P_ITB_IndexingState_eESCREntry:
               BDBG_MODULE_MSG( BVCE_OUTPUT_ITB_INDEX,("%3d: ESCR", hVceOutput->state.stITBBuffer.uiIndexWriteOffset) );
               BVCE_Output_S_ITB_ParseEntry( pITBEntry, pVideoDescriptor );

               /* SW7435-1133: Record STC when we first see the ITB entry */
               hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].uiSTC = BREG_Read32(
                        hVceOutput->hVce->handles.hReg,
                        hVceOutput->hVce->stPlatformConfig.stDebug.uiSTC[hVceOutput->stOpenSettings.uiInstance]
                        );

               /******************/
               /* Parse Metadata */
               /******************/
               hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].uiMetadata = BVCE_P_ITBEntry_GetMetadata(pITBEntry);

               /* Set Frame Rate */
               hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].uiFrameRate = ( ( BVCE_P_ITBEntry_GetMetadata(pITBEntry) >> 14 ) & 0x0F );

               /* Set Frame Dimensions */
               hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].uiWidth = ( ( BVCE_P_ITBEntry_GetMetadata(pITBEntry) >> 7 ) & 0x7F ) << 4;
               hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].uiHeight = ( ( BVCE_P_ITBEntry_GetMetadata(pITBEntry) >> 0 ) & 0x7F ) << 4;

               /* Set Chunk ID */
               hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].uiChunkId = ( ( BVCE_P_ITBEntry_GetMetadata(pITBEntry) >> 18 ) & 0x3FF );

               if ( ( true == hVceOutput->state.bPreviousChunkIdValid )
                    && ( hVceOutput->state.uiPreviousChunkId != hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].uiChunkId ) )
               {
                  /* If we're at the end of the chunk, set the previous ESCR valid = false since the ESCRs across chunk boundaries may not be always increasing */
                  hVceOutput->state.bPreviousESCRValid = false;
               }
               hVceOutput->state.uiPreviousChunkId = hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].uiChunkId;
               hVceOutput->state.bPreviousChunkIdValid = true;

               if ( true == hVceOutput->state.bPreviousESCRValid )
               {
                  if ( ( hVceOutput->state.uiPreviousESCR > pVideoDescriptor->stCommon.uiESCR )
                       && ( ( hVceOutput->state.uiPreviousESCR - pVideoDescriptor->stCommon.uiESCR ) < 0x80000000 ) )
                  {
                     BDBG_WRN(("Out of order ESCR detected (%08x --> %08x)", hVceOutput->state.uiPreviousESCR, pVideoDescriptor->stCommon.uiESCR));
                  }
               }

               hVceOutput->state.uiPreviousESCR = pVideoDescriptor->stCommon.uiESCR;
               hVceOutput->state.bPreviousESCRValid = true;

               /* SW7425-4608: Set Ignore frame flag */
               if ( 0 != ( ( BVCE_P_ITBEntry_GetMetadata(pITBEntry) >> 29 ) & 0x1 ) )
               {
                  BDBG_MSG(("[%d][%d] Ignore ITB Seen", hVceOutput->hVce->stOpenSettings.uiInstance, ( NULL != hVceOutput->state.hVceCh ) ? hVceOutput->state.hVceCh->stOpenSettings.uiInstance : 9));
                  hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].bIgnoreFrame = true;
               }

               /* SW7445-2896: Set segment start flag */
               if ( 0 != ( ( BVCE_P_ITBEntry_GetMetadata(pITBEntry) >> 30 ) & 0x1 ) )
               {
                  BDBG_MSG(("[%d][%d] Segment Start ITB Seen", hVceOutput->hVce->stOpenSettings.uiInstance, ( NULL != hVceOutput->state.hVceCh ) ? hVceOutput->state.hVceCh->stOpenSettings.uiInstance : 9));
               }

               /* SW7425-5925: Set channel change flag */
               if ( 0 != ( ( BVCE_P_ITBEntry_GetMetadata(pITBEntry) >> 31 ) & 0x1 ) )
               {
                  BDBG_MSG(("[%d][%d] Channel Change ITB Seen", hVceOutput->hVce->stOpenSettings.uiInstance, ( NULL != hVceOutput->state.hVceCh ) ? hVceOutput->state.hVceCh->stOpenSettings.uiInstance : 9));
                  hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].bChannelChange = true;
               }
               break;

            case BVCE_OUTPUT_P_ITB_IndexingState_eCRCEntry:
               BDBG_MODULE_MSG( BVCE_OUTPUT_ITB_INDEX,("%3d: CRC", hVceOutput->state.stITBBuffer.uiIndexWriteOffset) );
               BVCE_Output_S_ITB_ParseEntry( pITBEntry, pVideoDescriptor );
               break;

            case BVCE_OUTPUT_P_ITB_IndexingState_eSTCEntry:
               BVCE_Output_S_ITB_ParseEntry( pITBEntry, pVideoDescriptor );
               if ( 0 != ( pVideoDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_START ) )
               {
                  BDBG_MODULE_MSG( BVCE_OUTPUT_ITB_INDEX,("%3d: STC", hVceOutput->state.stITBBuffer.uiIndexWriteOffset) );
               }
               else
               {
                  BDBG_MODULE_MSG( BVCE_OUTPUT_ITB_INDEX,("%3d: STC (Stale)", hVceOutput->state.stITBBuffer.uiIndexWriteOffset) );
               }
               break;

            case BVCE_OUTPUT_P_ITB_IndexingState_eNALAddressDataEntry:
            {
               BVCE_Output_S_ITB_ParseEntry( pITBEntry, pVideoDescriptor );
               /* If the NALU byte stream contains:           0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08
                * Then the ITB NALU data is in reverse order: 0x08 0x07 0x06 0x05 0x04 0x03 0x02 0x01
                */
               if ( true == hVceOutput->stOpenSettings.bEnableDataUnitDetection )
               {
                  /* Don't detect a new frame if we currently have a FRAME_START and the existing CDB address equals the NALU CDB Address */
                  uint64_t uiCDBAddress = BVCE_P_ITBEntry_GetNalAddr_CDBOffset(pITBEntry);
#ifdef BVCE_PLATFORM_P_SUPPORTS_64_BIT
                  /* On 64-bit platforms, the CDB Address in the ITB is relative to the start if the CDB (i.e. no longer an absolute offset)
                     So, to be compatible with the 32-bit parsing code, we add the base CDB offset back in */
                  uiCDBAddress += hVceOutput->state.stCDBBuffer.stOffset.uiBase;
#endif
                  if (! ( ( 0 != ( hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].stFrameDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_START ) )
                          && ( hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].uiCDBAddress == uiCDBAddress ) ) )
                  {
                     BVCE_Output_S_ITB_DetectNewFrame( hVceOutput );
                  }
                  BDBG_MODULE_MSG( BVCE_OUTPUT_ITB_INDEX,("%3d: NAL ADDR/DATA", hVceOutput->state.stITBBuffer.uiIndexWriteOffset) );
                  pVideoDescriptor = &hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].stFrameDescriptor;

                  hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].uiCDBAddress = uiCDBAddress;

                  BDBG_MODULE_MSG( BVCE_OUTPUT_ITB_INDEX,("     CDB Offset: %08x, startCodeSize: %u",
                                                          BVCE_P_ITBEntry_GetNalAddr_CDBOffset(pITBEntry),
                                                          BVCE_P_ITBEntry_GetNalAddr_NumStartBytes(pITBEntry)
                                      ));

                  BVCE_Output_S_ITB_ExtractNALData( pVideoDescriptor, pITBEntry, hVceOutput->state.stChannelCache.stStartEncodeSettings.stProtocolInfo.eProtocol );
               }
               else
               {
                  BDBG_MODULE_MSG( BVCE_OUTPUT_ITB_INDEX,("%3d: NAL ADDR/DATA", hVceOutput->state.stITBBuffer.uiIndexWriteOffset) );
               }
               break;
            }
            case BVCE_OUTPUT_P_ITB_IndexingState_eNALDataEntry:
               BDBG_MODULE_MSG( BVCE_OUTPUT_ITB_INDEX,("%3d: NAL DATA", hVceOutput->state.stITBBuffer.uiIndexWriteOffset) );
               BVCE_Output_S_ITB_ParseEntry( pITBEntry, pVideoDescriptor );
               if ( true == hVceOutput->stOpenSettings.bEnableDataUnitDetection )
               {
                  BVCE_Output_S_ITB_ExtractNALData( pVideoDescriptor, pITBEntry, hVceOutput->state.stChannelCache.stStartEncodeSettings.stProtocolInfo.eProtocol );
               }
               break;

            case BVCE_OUTPUT_P_ITB_IndexingState_eEOSEntry:
               BVCE_Output_S_ITB_DetectNewFrame( hVceOutput );
               pVideoDescriptor = &hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].stFrameDescriptor;

               BDBG_MODULE_MSG( BVCE_OUTPUT_ITB_INDEX,("%3d: EOS", hVceOutput->state.stITBBuffer.uiIndexWriteOffset) );
               BVCE_Output_S_ITB_ParseEntry( pITBEntry, pVideoDescriptor );
               hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].uiCDBAddress = BVCE_P_ITBEntry_GetEOSCDBAddress(pITBEntry);
#ifdef BVCE_PLATFORM_P_SUPPORTS_64_BIT
               /* On 64-bit platforms, the CDB Address in the ITB is relative to the start if the CDB (i.e. no longer an absolute offset)
                  So, to be compatible with the 32-bit parsing code, we add the base CDB offset back in */
               hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].uiCDBAddress += hVceOutput->state.stCDBBuffer.stOffset.uiBase;
#endif
               BDBG_ASSERT( 0 != hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].uiCDBAddress );

               hVceOutput->state.bEOSITBEntryIndexed = true;
               break;

            case BVCE_OUTPUT_P_ITB_IndexingState_eNULLEntry:
               BDBG_MODULE_MSG( BVCE_OUTPUT_ITB_INDEX,("%3d: NULL", hVceOutput->state.stITBBuffer.uiIndexWriteOffset) );
               BVCE_Output_S_ITB_ParseEntry( pITBEntry, pVideoDescriptor );
               break;

#if BVCE_P_DUMP_MAU_PERFORMANCE_DATA
            case BVCE_OUTPUT_P_ITB_IndexingState_eMAUPerformance0Entry:
            case BVCE_OUTPUT_P_ITB_IndexingState_eMAUPerformance1Entry:
            case BVCE_OUTPUT_P_ITB_IndexingState_eMAUPerformance2Entry:
            case BVCE_OUTPUT_P_ITB_IndexingState_eMAUPerformance3Entry:
               BVCE_Output_S_ITB_ParseEntry( pITBEntry, pVideoDescriptor );
               {
                  unsigned uiIndexMultiplier = eIndexState - BVCE_OUTPUT_P_ITB_IndexingState_eMAUPerformance0Entry;
                  unsigned uiIndexOffset = uiIndexMultiplier * 6;
                  unsigned i;
                  BDBG_MODULE_MSG( BVCE_OUTPUT_ITB_INDEX,("%3d: MAU Performance %d", hVceOutput->state.stITBBuffer.uiIndexWriteOffset, uiIndexMultiplier));

                  for ( i = 0; i < 6; i++ )
                  {
                     uint16_t uiValue = 0xFFFF;
                     hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].stMAUPerformanceData[i + uiIndexOffset].bValid = true;
                     hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].stMAUPerformanceData[i + uiIndexOffset].bOverflow = ( 0x1 == ( ( BVCE_P_ITBEntry_GetMAUPerformanceOverflowStatus(pITBEntry) >> i ) & 0x1 ) );
                     switch (i)
                     {
                        case 0:
                           uiValue = BVCE_P_ITBEntry_GetMAUPerformanceData(pITBEntry, 0);
                           break;

                        case 1:
                           uiValue = BVCE_P_ITBEntry_GetMAUPerformanceData(pITBEntry, 1);
                           break;

                        case 2:
                           uiValue = BVCE_P_ITBEntry_GetMAUPerformanceData(pITBEntry, 2);
                           break;

                        case 3:
                           uiValue = BVCE_P_ITBEntry_GetMAUPerformanceData(pITBEntry, 3);
                           break;

                        case 4:
                           uiValue = BVCE_P_ITBEntry_GetMAUPerformanceData(pITBEntry, 4);
                           break;

                        case 5:
                           uiValue = BVCE_P_ITBEntry_GetMAUPerformanceData(pITBEntry, 5);
                           break;

                        default:
                           break;
                     }
                     hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].stMAUPerformanceData[i + uiIndexOffset].uiValue = uiValue;
                  }
               }
               break;
#endif


            default:
               BDBG_MODULE_MSG( BVCE_OUTPUT_ITB_INDEX,("%3d: UNKNOWN", hVceOutput->state.stITBBuffer.uiIndexWriteOffset) );
               break;
         }

         hVceOutput->stDescriptors.astIndex[hVceOutput->state.stITBBuffer.uiIndexWriteOffset].uiSizeInITB += 16;

         uiReadOffset += 16;
         if ( uiReadOffset >= hVceOutput->state.stITBBuffer.stOffset.uiEnd )
         {
            uiReadOffset -= ( hVceOutput->state.stITBBuffer.stOffset.uiEnd - hVceOutput->state.stITBBuffer.stOffset.uiBase );
         }
      }

      BVCE_P_Buffer_UnlockAddress( hVceOutput->hOutputBuffers->stITB.hBuffer );

      BVCE_Output_S_ITB_DetectNewFrame( hVceOutput );
   }
}

static
void
BVCE_Output_S_DumpDescriptorLegacy(
   BVCE_Output_Handle hVceOutput,
   BVCE_P_Output_ITB_IndexEntry *pITBIndexEntry
   )
{
#if !BDBG_DEBUG_BUILD
   BSTD_UNUSED( pITBIndexEntry );
#endif

   if ( ( true == hVceOutput->state.bNewITBEntry )
         && ( 0 != ( pITBIndexEntry->stFrameDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_START ) ) )
   {
      BDBG_MODULE_MSG( BVCE_OUTPUT_ITB, ("@"BDBG_UINT64_FMT" (%d), I=%d, d="BDBG_UINT64_FMT" (%d), p="BDBG_UINT64_FMT", op=%08x, escr=%08x, tpb=%04x, shr=%6d, metadata=%08x",
               BDBG_UINT64_ARG(pITBIndexEntry->uiCDBAddress),
               pITBIndexEntry->bError,
               (0 != ( pITBIndexEntry->stFrameDescriptor.uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_RAP )),
               BDBG_UINT64_ARG(pITBIndexEntry->stFrameDescriptor.uiDTS),
               (0 != ( pITBIndexEntry->stFrameDescriptor.uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DTS_VALID )),
               BDBG_UINT64_ARG(pITBIndexEntry->stFrameDescriptor.stCommon.uiPTS),
               pITBIndexEntry->stFrameDescriptor.stCommon.uiOriginalPTS,
               pITBIndexEntry->stFrameDescriptor.stCommon.uiESCR,
               pITBIndexEntry->stFrameDescriptor.stCommon.uiTicksPerBit,
               pITBIndexEntry->stFrameDescriptor.stCommon.iSHR,
               pITBIndexEntry->uiMetadata
               ));

      {

#if BVCE_P_DUMP_OUTPUT_ITB_DESC
         if ( ( NULL == hVceOutput->hITBDescDumpFile ) && ( NULL != hVceOutput->state.hVceCh ) )
         {
            static unsigned uiInstance[BVCE_PLATFORM_P_NUM_ENCODE_INSTANCES][BVCE_PLATFORM_P_NUM_ENCODE_CHANNELS];
            char fname[256];
            sprintf(fname, "BVCE_OUTPUT_DESC_%02d_%02d_%03d.csv", hVceOutput->state.hVceCh->hVce->stOpenSettings.uiInstance, hVceOutput->state.hVceCh->stOpenSettings.uiInstance, uiInstance[hVceOutput->state.hVceCh->hVce->stOpenSettings.uiInstance][hVceOutput->state.hVceCh->stOpenSettings.uiInstance]);
            uiInstance[hVceOutput->state.hVceCh->hVce->stOpenSettings.uiInstance][hVceOutput->state.hVceCh->stOpenSettings.uiInstance]++;

            if ( false == BVCE_Debug_P_OpenLog( fname, &hVceOutput->hITBDescDumpFile ) )
            {
               BDBG_ERR(("Error Creating BVCE Output ITB Desc Dump File (%s)", fname));
            }
            else
            {
               BVCE_Debug_P_WriteLog_isr(hVceOutput->hITBDescDumpFile, "address,dts(90Khz),dts valid,pts(90Khz),opts(45Khz),escr(27Mhz),deltaEtsDts,deltaEtsDts(valid),tpb,shr,error,I frame,metadata,STC,ITB depth (shadow),ITB depth (actual),CDB depth (shadow),CDB depth (actual), CDB depth (hw), CDB write (hw), CDB valid (hw), CDB read (hw)\n");
            }
         }

         if ( NULL != hVceOutput->hITBDescDumpFile )
         {
            uint32_t uiCDBDepth;
            uint32_t uiITBDepthActual;
            uint32_t uiCDBDepthActual;

            if ( hVceOutput->state.stCDBBuffer.stOffset.uiValid >= hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead )
            {
               uiCDBDepth = hVceOutput->state.stCDBBuffer.stOffset.uiValid - hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead;
            }
            else
            {
               uiCDBDepth = hVceOutput->state.stCDBBuffer.stOffset.uiEnd - hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead;
               uiCDBDepth += hVceOutput->state.stCDBBuffer.stOffset.uiValid - hVceOutput->state.stCDBBuffer.stOffset.uiBase;
            }

            if ( hVceOutput->state.stCDBBuffer.stOffset.uiValid >= hVceOutput->state.stCDBBuffer.stOffset.uiRead )
            {
               uiCDBDepthActual = hVceOutput->state.stCDBBuffer.stOffset.uiValid - hVceOutput->state.stCDBBuffer.stOffset.uiRead;
            }
            else
            {
               uiCDBDepthActual = hVceOutput->state.stCDBBuffer.stOffset.uiEnd - hVceOutput->state.stCDBBuffer.stOffset.uiRead;
               uiCDBDepthActual += hVceOutput->state.stCDBBuffer.stOffset.uiValid - hVceOutput->state.stCDBBuffer.stOffset.uiBase;
            }

            if ( hVceOutput->state.stITBBuffer.stOffset.uiValid >= hVceOutput->state.stITBBuffer.stOffset.uiRead )
            {
               uiITBDepthActual = hVceOutput->state.stITBBuffer.stOffset.uiValid - hVceOutput->state.stITBBuffer.stOffset.uiRead;
            }
            else
            {
               uiITBDepthActual = hVceOutput->state.stITBBuffer.stOffset.uiEnd - hVceOutput->state.stITBBuffer.stOffset.uiRead;
               uiITBDepthActual += hVceOutput->state.stITBBuffer.stOffset.uiValid - hVceOutput->state.stITBBuffer.stOffset.uiBase;
            }

            BVCE_Debug_P_WriteLog_isr( hVceOutput->hITBDescDumpFile, "%u,%llu,%d,%llu,%u,%u,%u,%d,%d,%d,%08x,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\n",
                     pITBIndexEntry->uiCDBAddress,
                     pITBIndexEntry->stFrameDescriptor.uiDTS,
                     (0 != ( pITBIndexEntry->stFrameDescriptor.uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DTS_VALID )),
                     pITBIndexEntry->stFrameDescriptor.stCommon.uiPTS,
                     pITBIndexEntry->stFrameDescriptor.stCommon.uiOriginalPTS,
                     pITBIndexEntry->stFrameDescriptor.stCommon.uiESCR,
                     hVceOutput->state.stChannelStatus.uiEtsDtsOffset,
                     hVceOutput->state.bChannelStatusValid,
                     pITBIndexEntry->stFrameDescriptor.stCommon.uiTicksPerBit,
                     pITBIndexEntry->stFrameDescriptor.stCommon.iSHR,
                     pITBIndexEntry->bError,
                     (0 != ( pITBIndexEntry->stFrameDescriptor.uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_RAP )),
                     pITBIndexEntry->uiMetadata,
                     pITBIndexEntry->uiSTC,
                     hVceOutput->state.stITBBuffer.uiShadowDepth,
                     uiITBDepthActual,
                     uiCDBDepth,
                     uiCDBDepthActual,
                     hVceOutput->state.stCDBBuffer.hw.uiDepth,
                     hVceOutput->state.stCDBBuffer.hw.uiWriteOffset,
                     hVceOutput->state.stCDBBuffer.hw.uiValidOffset,
                     hVceOutput->state.stCDBBuffer.hw.uiReadOffset
                     );
         }
#endif

#if BVCE_P_DUMP_MAU_PERFORMANCE_DATA
         if ( ( NULL == hVceOutput->hMAUPerformanceDumpFile ) && ( NULL != hVceOutput->state.hVceCh ) )
         {
            static unsigned uiInstance[BVCE_PLATFORM_P_NUM_ENCODE_INSTANCES][BVCE_PLATFORM_P_NUM_ENCODE_CHANNELS];
            char fname[256];
            sprintf(fname, "BVCE_OUTPUT_MAU_PERFORMANCE_%02d_%02d_%03d.csv", hVceOutput->state.hVceCh->hVce->stOpenSettings.uiInstance, hVceOutput->state.hVceCh->stOpenSettings.uiInstance, uiInstance[hVceOutput->state.hVceCh->hVce->stOpenSettings.uiInstance][hVceOutput->state.hVceCh->stOpenSettings.uiInstance]);
            uiInstance[hVceOutput->state.hVceCh->hVce->stOpenSettings.uiInstance][hVceOutput->state.hVceCh->stOpenSettings.uiInstance]++;

            if ( false == BVCE_Debug_P_OpenLog( fname, &hVceOutput->hMAUPerformanceDumpFile ) )
            {
               BDBG_ERR(("Error Creating BVCE Output MAU Performance Dump File (%s)", fname));
            }
            else
            {
               unsigned i;

               for ( i = 0; i < 24; i++ )
               {
                  BVCE_Debug_P_WriteLog_isr(hVceOutput->hMAUPerformanceDumpFile, "value_%02d,", i);
               }
               for ( i = 0; i < 24; i++ )
               {
                  BVCE_Debug_P_WriteLog_isr(hVceOutput->hMAUPerformanceDumpFile, "overflow_%02d", i);
                  if ( 23 != i ) BVCE_Debug_P_WriteLog_isr( hVceOutput->hMAUPerformanceDumpFile, "," );
               }
               BVCE_Debug_P_WriteLog_isr(hVceOutput->hMAUPerformanceDumpFile, "\n");
            }
         }

         if ( NULL != hVceOutput->hMAUPerformanceDumpFile )
         {
            {
               unsigned i;

               for ( i = 0; i < 24; i++ )
               {
                  if ( true == pITBIndexEntry->stMAUPerformanceData[i].bValid )
                  {
                     BVCE_Debug_P_WriteLog_isr( hVceOutput->hMAUPerformanceDumpFile, "%u,", pITBIndexEntry->stMAUPerformanceData[i].uiValue);
                  }
               }

               for ( i = 0; i < 24; i++ )
               {
                  if ( true == pITBIndexEntry->stMAUPerformanceData[i].bValid )
                  {
                     BVCE_Debug_P_WriteLog_isr( hVceOutput->hMAUPerformanceDumpFile, "%u", pITBIndexEntry->stMAUPerformanceData[i].bOverflow);
                  }
                  if ( 23 != i ) BVCE_Debug_P_WriteLog_isr( hVceOutput->hMAUPerformanceDumpFile, "," );
               }

               BVCE_Debug_P_WriteLog_isr(hVceOutput->hMAUPerformanceDumpFile, "\n");
            }
         }
#endif
      }
   }
}

static
bool
BVCE_Output_S_IsStartOfNewFrame(
   BVCE_Output_Handle hVceOutput
   )
{
   if ( BVCE_Output_S_ITB_Shadow_GetNumFrames( hVceOutput ) > 1 )
   {
      BVCE_P_Output_ITB_IndexEntry *pITBIndexEntryNext = BVCE_Output_S_ITB_Shadow_GetFrameIndexEntry( hVceOutput, 1 );

      BDBG_ASSERT( BVCE_P_Buffer_GetDeviceOffset_isrsafe( hVceOutput->hOutputBuffers->stCDB.hBuffer ) <= pITBIndexEntryNext->uiCDBAddress );
      BDBG_ASSERT( ( BVCE_P_Buffer_GetDeviceOffset_isrsafe( hVceOutput->hOutputBuffers->stCDB.hBuffer ) + hVceOutput->hOutputBuffers->stSettings.stConfig.Cdb.Length ) > pITBIndexEntryNext->uiCDBAddress );

      /* Goto next frame's ITB Entry */
      if ( pITBIndexEntryNext->uiCDBAddress == hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead )
      {
         /* We have a next entry, and we've finished with the
          * current entry, so move to the next entry
          */
         BVCE_Output_S_ITB_Shadow_ConsumeEntry( hVceOutput );
         hVceOutput->state.bNewITBEntry = true;
         return true;
      }
   }

   return false;
}

#if BVCE_P_TEST_MODE
static
void
BVCE_Output_S_DumpDescriptor(
   BVCE_Output_Handle hVceOutput,
   const BAVC_VideoBufferDescriptor *pstDescriptor
   )
{
   if ( ( NULL == hVceOutput->hDescriptorLog ) && ( NULL != hVceOutput->state.hVceCh ) )
   {
      char fname[256];
      static unsigned uiDescriptorLogInstance[BVCE_PLATFORM_P_NUM_ENCODE_INSTANCES][BVCE_PLATFORM_P_NUM_ENCODE_CHANNELS];
      sprintf(fname, "BVCE_BUFFER_DESC_%02d_%02d_%03d.csv", hVceOutput->state.hVceCh->hVce->stOpenSettings.uiInstance, hVceOutput->state.hVceCh->stOpenSettings.uiInstance, uiDescriptorLogInstance[hVceOutput->state.hVceCh->hVce->stOpenSettings.uiInstance][hVceOutput->state.hVceCh->stOpenSettings.uiInstance]);
      uiDescriptorLogInstance[hVceOutput->state.hVceCh->hVce->stOpenSettings.uiInstance][hVceOutput->state.hVceCh->stOpenSettings.uiInstance]++;

      if ( false == BVCE_Debug_P_OpenLog( fname, &hVceOutput->hDescriptorLog ) )
      {
         BDBG_ERR(("Error Creating BVCE Output Buffer Desc Dump File (%s)", fname));
      }
      else
      {
         BVCE_Debug_P_WriteLog_isr( hVceOutput->hDescriptorLog, "count,flags,metadata,extended,frame start,eoc,eos,empty frame,first frame,segment start,offset,length,opts valid,opts,pts valid,pts,escr valid,escr,tpb valid,tpb, shr valid, shr, stc valid, stc" );
         BVCE_Debug_P_WriteLog_isr( hVceOutput->hDescriptorLog, ",vflags,dts valid,dts,dut start,dut,rai" );
         BVCE_Debug_P_WriteLog_isr( hVceOutput->hDescriptorLog, "\n" );
      }
   }

   if ( NULL != hVceOutput->hDescriptorLog )
   {
      /* Common */
      BVCE_Debug_P_WriteLog_isr( hVceOutput->hDescriptorLog, "%u,0x%08x,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%llu,%u,%u,%u,%u,%u,%d,%u,%llu",
         hVceOutput->uiDescriptorCount++,
         pstDescriptor->stCommon.uiFlags,
         (0 != (pstDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_METADATA)),
         (0 != (pstDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EXTENDED)),
         (0 != (pstDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_START)),
         (0 != (pstDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOC)),
         (0 != (pstDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOS)),
         (0 != (pstDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EMPTY_FRAME)),
         (0 != (pstDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FIRST_FRAME)),
         (0 != (pstDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_SEGMENT_START)),
         pstDescriptor->stCommon.uiOffset,
         pstDescriptor->stCommon.uiLength,
         (0 != (pstDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_ORIGINALPTS_VALID)),
         pstDescriptor->stCommon.uiOriginalPTS,
         (0 != (pstDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_PTS_VALID)),
         pstDescriptor->stCommon.uiPTS,
         (0 != (pstDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_ESCR_VALID)),
         pstDescriptor->stCommon.uiESCR,
         (0 != (pstDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_TICKSPERBIT_VALID)),
         pstDescriptor->stCommon.uiTicksPerBit,
         (0 != (pstDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_SHR_VALID)),
         pstDescriptor->stCommon.iSHR,
         (0 != (pstDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_STCSNAPSHOT_VALID)),
         pstDescriptor->stCommon.uiSTCSnapshot
         );

      BVCE_Debug_P_WriteLog_isr( hVceOutput->hDescriptorLog, ",0x%08x,%u,%llu,%u,%u,%u",
         pstDescriptor->uiVideoFlags,
         (0 != (pstDescriptor->uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DTS_VALID)),
         pstDescriptor->uiDTS,
         (0 != (pstDescriptor->uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DATA_UNIT_START)),
         pstDescriptor->uiDataUnitType,
         (0 != (pstDescriptor->uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_RAP))
         );

      BVCE_Debug_P_WriteLog_isr( hVceOutput->hDescriptorLog, "\n" );
   }
}
#endif

static
void
BVCE_Output_S_CheckCabacReady(
   BVCE_Output_Handle hVceOutput
   )
{
   /* Check to see if the Cabac Interrupt Fired */
   if ( ( false == hVceOutput->state.bCabacInitializedShadow )
        && ( NULL != hVceOutput->state.hVceCh )
        && ( ( BVCE_P_Status_eStarted == BVCE_Channel_P_GetState( hVceOutput->state.hVceCh ) )
             || ( BVCE_P_Status_eStopping == BVCE_Channel_P_GetState( hVceOutput->state.hVceCh ) ) )
        )
   {
#if BVCE_P_POLL_FW_DATAREADY
      uint32_t uiViceInterruptStatus = BREG_Read32(
               hVceOutput->hVce->handles.hReg,
               hVceOutput->hVce->stPlatformConfig.stInterrupt.uiInterruptStatusRegister
               );

      BDBG_MSG( ("L2 Status[%d]: %08x", hVceOutput->stOpenSettings.uiInstance, uiViceInterruptStatus));

      hVceOutput->state.bCabacInitializedActual = (uiViceInterruptStatus & hVceOutput->hVce->stPlatformConfig.stInterrupt.stMask.uiDataReady[hVceOutput->stOpenSettings.uiInstance]) ? true : false;
#endif

      {
         /* SW7425-4614: If the encoder is started when there is still a pending flush (e.g. when doing a stop(immediate)-->start
          * we don't want ignore the "CABAC Ready" interrupt caused by the flushed descriptors.  So, we qualify the "CABAC Ready"
          * by verifying that the (uiReadOffset - uiValidOffset) is a multiple of 16.  This is ONLY true at start of the encode.
          * Once descriptors are returned to the VCE, the uiReadOffset is always 1 less than what was actually read.  \
          * (See bITBReadHackDone)
          */
         if ( true == hVceOutput->state.bCabacInitializedActual )
         {
            uint64_t uiITBReadOffset;

            uiITBReadOffset = BREG_ReadAddr(
                       hVceOutput->hVce->handles.hReg,
                       hVceOutput->stRegisters.ITB_Read
                       );

            if ( 0 != ( ( hVceOutput->state.stITBBuffer.stOffset.uiValid - uiITBReadOffset ) % 16 ) )
            {
               /* The new frames have not arrived because of stale descriptors being flushed, so
                * reset the cabac interrupt and wait
                */
               hVceOutput->state.bCabacInitializedActual = false;

#if BVCE_P_POLL_FW_DATAREADY
               /* Clear the CABAC data ready interrupt */
               BREG_Write32(
                  hVceOutput->hVce->handles.hReg,
                  hVceOutput->hVce->stPlatformConfig.stInterrupt.uiInterruptClearRegister,
                  hVceOutput->hVce->stPlatformConfig.stInterrupt.stMask.uiDataReady[hVceOutput->stOpenSettings.uiInstance]
               );
#endif

               BDBG_WRN(("Ignoring CABAC ready until stale data from previous encode has been flushed"));
            }
            else
            {
               hVceOutput->state.bCabacInitializedShadow = true;
            }
         }
         else
         /* SW7425-4614: If the cabac is not ready, it is possible the ITB/CDB is full from a previous unclean
          * stop, so the cabac is waiting for space to free up.  If this is the case, we need to flush the
          * ITB/CDB buffers.
          */
         {
            unsigned uiITBDepth;
            unsigned uiITBSize;
            unsigned uiCDBDepth;
            unsigned uiCDBSize;

            /* Compute ITB Depth */
            {
               uint64_t uiBaseOffset;
               uint64_t uiEndOffset;

               uiBaseOffset = BREG_ReadAddr(
                          hVceOutput->hVce->handles.hReg,
                          hVceOutput->stRegisters.ITB_Base
                          );

               uiEndOffset = BREG_ReadAddr(
                          hVceOutput->hVce->handles.hReg,
                          hVceOutput->stRegisters.ITB_End
                          );

               uiITBSize = uiEndOffset - uiBaseOffset;

               {
                  uint64_t uiReadOffset;
                  uint64_t uiValidOffset;

                  uiReadOffset = BREG_ReadAddr(
                              hVceOutput->hVce->handles.hReg,
                              hVceOutput->stRegisters.ITB_Read
                              );

                  uiValidOffset = BREG_ReadAddr(
                             hVceOutput->hVce->handles.hReg,
                             hVceOutput->stRegisters.ITB_Valid
                             );

                  if ( uiValidOffset >= uiReadOffset )
                  {
                     uiITBDepth = uiValidOffset - uiReadOffset;
                  }
                  else
                  {
                     uiITBDepth = (uiEndOffset - uiReadOffset) + (uiValidOffset - uiBaseOffset);
                  }
               }
            }

            /* Compute CDB Depth */
            {
               uint64_t uiBaseOffset;
               uint64_t uiEndOffset;

               uiBaseOffset = BREG_ReadAddr(
                          hVceOutput->hVce->handles.hReg,
                          hVceOutput->stRegisters.CDB_Base
                          );

               uiEndOffset = BREG_ReadAddr(
                          hVceOutput->hVce->handles.hReg,
                          hVceOutput->stRegisters.CDB_End
                          );

               uiCDBSize = uiEndOffset - uiBaseOffset;

               {
                  uint64_t uiReadOffset;
                  uint64_t uiValidOffset;

                  uiReadOffset = BREG_ReadAddr(
                              hVceOutput->hVce->handles.hReg,
                              hVceOutput->stRegisters.CDB_Read
                              );

                  uiValidOffset = BREG_ReadAddr(
                             hVceOutput->hVce->handles.hReg,
                             hVceOutput->stRegisters.CDB_Valid
                             );

                  if ( uiValidOffset >= uiReadOffset )
                  {
                     uiCDBDepth = uiValidOffset - uiReadOffset;
                  }
                  else
                  {
                     uiCDBDepth = (uiEndOffset - uiReadOffset) + (uiValidOffset - uiBaseOffset);
                  }
               }
            }

            if ( ( ( uiCDBDepth > uiCDBSize/2 ) && ( uiCDBDepth != uiCDBSize ) )
                 || ( ( uiITBDepth > uiITBSize/2 ) && ( uiITBDepth != uiITBSize ) ) )
            {
               BDBG_WRN(("CDB: %d/%d and/or ITB: %d/%d is full. Flushing...",
                  uiCDBDepth,
                  uiCDBSize,
                  uiITBDepth,
                  uiITBSize
                  ));

               BVCE_Output_S_FlushDirect( hVceOutput );
            }
         }
      }
   }
}

static
void
BVCE_Output_S_HandlePicturelessEncode(
   BVCE_Output_Handle hVceOutput
   )
{
   /* SW7425-4731: We are in the stopping state, but haven't sent any descriptors to the mux, yet.
    * So, we simply send a dummy metadata descriptor followed by an EOS to keep the mux happy
    */
   if ( ( false == hVceOutput->state.bCabacInitializedShadow )
        && ( false == hVceOutput->state.bMetadataSent )
        && ( NULL != hVceOutput->state.hVceCh )
        &&  ( BVCE_P_Status_eStopping == BVCE_Channel_P_GetState( hVceOutput->state.hVceCh ) ) )
   {
      BAVC_VideoBufferDescriptor *pVideoDescriptor;
      BAVC_VideoMetadataDescriptor *pMetadataDescriptor;

      BDBG_WRN(("Special EOS Handling Enabled"));

      /* Populate and Send Metadata Descriptor */
      pMetadataDescriptor = &hVceOutput->stDescriptors.astMetadataDescriptors[hVceOutput->state.uiMetadataDescriptorWriteOffset];
      hVceOutput->state.uiMetadataDescriptorWriteOffset++;
      hVceOutput->state.uiMetadataDescriptorWriteOffset %= BVCE_P_MAX_METADATADESCRIPTORS;

      BKNI_Memset(
               pMetadataDescriptor,
               0,
               sizeof( BAVC_VideoMetadataDescriptor )
               );

      /* Get Video Descriptor for the metadata */
      pVideoDescriptor = &hVceOutput->stDescriptors.astDescriptors[hVceOutput->state.uiDescriptorWriteOffset];
      hVceOutput->state.uiDescriptorWriteOffset++;
      hVceOutput->state.uiDescriptorWriteOffset %= hVceOutput->stOpenSettings.uiDescriptorQueueDepth;

      BKNI_Memset(
               pVideoDescriptor,
               0,
               sizeof( BAVC_VideoBufferDescriptor )
               );

      pVideoDescriptor->stCommon.uiOffset = BVCE_P_Buffer_GetDeviceBaseOffset( hVceOutput->stDescriptors.hMetadataBuffer );
      pVideoDescriptor->stCommon.uiOffset += (uint8_t*) pMetadataDescriptor - (uint8_t*) &hVceOutput->stDescriptors.astMetadataDescriptors[0];

      pVideoDescriptor->stCommon.uiLength = sizeof( BAVC_VideoMetadataDescriptor );
      pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_METADATA;
      pVideoDescriptor->uiDataUnitType = BAVC_VideoMetadataType_eCommon;

#if BVCE_P_TEST_MODE
               BVCE_Output_S_DumpDescriptor(
                  hVceOutput,
                  pVideoDescriptor
                  );
#endif

      /* Send EOS Descriptor */
      pVideoDescriptor = &hVceOutput->stDescriptors.astDescriptors[hVceOutput->state.uiDescriptorWriteOffset];
      hVceOutput->stDescriptors.astDescriptorReleaseMetadata[hVceOutput->state.uiDescriptorWriteOffset].uiShadowLength = 0;
      hVceOutput->state.uiDescriptorWriteOffset++;
      hVceOutput->state.uiDescriptorWriteOffset %= hVceOutput->stOpenSettings.uiDescriptorQueueDepth;

      BKNI_Memset(
               pVideoDescriptor,
               0,
               sizeof( BAVC_VideoBufferDescriptor )
               );

      pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOS;

      if ( true == hVceOutput->state.hVceCh->stStartEncodeSettings.bNonRealTimeEncodeMode )
      {
         pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_ESCR_VALID;
      }

#if BVCE_P_TEST_MODE
               BVCE_Output_S_DumpDescriptor(
                  hVceOutput,
                  pVideoDescriptor
                  );
#endif

      /* Treat this as an EOS */
      BDBG_MSG( ("[%d][%d] Force EOS", hVceOutput->hVce->stOpenSettings.uiInstance, hVceOutput->stOpenSettings.uiInstance));
      BVCE_Output_S_DetachFromChannel( hVceOutput );
   }
}

static
void
BVCE_Output_S_SendMetadataDescriptor(
   BVCE_Output_Handle hVceOutput,
   BVCE_P_Output_ITB_IndexEntry *pITBIndexEntry
   )
{
   if ( ( true == hVceOutput->state.bNewITBEntry )
        && ( 0 != ( pITBIndexEntry->stFrameDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_START ) )
        && ( false == pITBIndexEntry->bIgnoreFrame )
        && ( NULL != hVceOutput->state.hVceCh )
        )
   {
      if ( ( ( false == hVceOutput->state.bMetadataSent ) /* Always send metadata at least once */
             || ( ( pITBIndexEntry->uiMetadata & BVCE_P_NEW_METADATA_MASK ) != ( hVceOutput->state.uiPreviousMetadata & BVCE_P_NEW_METADATA_MASK ) ) ) /* SW7425-5477: Send metadata again if any metadata desc param has changed */
           && ( 0 == ( pITBIndexEntry->stFrameDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOS ) ) /* Not an EOS entry */
           && ( true == hVceOutput->state.stChannelCache.bValid )
         )
      {
         BAVC_VideoBufferDescriptor *pVideoDescriptor = NULL;
         BAVC_VideoMetadataDescriptor *pMetadataDescriptor = &hVceOutput->stDescriptors.astMetadataDescriptors[hVceOutput->state.uiMetadataDescriptorWriteOffset];
         hVceOutput->state.uiMetadataDescriptorWriteOffset++;
         hVceOutput->state.uiMetadataDescriptorWriteOffset %= BVCE_P_MAX_METADATADESCRIPTORS;

         BKNI_Memset(
                  pMetadataDescriptor,
                  0,
                  sizeof( BAVC_VideoMetadataDescriptor )
                  );

         /* Get Video Descriptor for the metadata */
         pVideoDescriptor = &hVceOutput->stDescriptors.astDescriptors[hVceOutput->state.uiDescriptorWriteOffset];
         hVceOutput->state.uiDescriptorWriteOffset++;
         hVceOutput->state.uiDescriptorWriteOffset %= hVceOutput->stOpenSettings.uiDescriptorQueueDepth;

         BKNI_Memset(
                  pVideoDescriptor,
                  0,
                  sizeof( BAVC_VideoBufferDescriptor )
                  );

         pVideoDescriptor->stCommon.uiOffset = BVCE_P_Buffer_GetDeviceBaseOffset( hVceOutput->stDescriptors.hMetadataBuffer );
         pVideoDescriptor->stCommon.uiOffset += (uint8_t*) pMetadataDescriptor - (uint8_t*) &hVceOutput->stDescriptors.astMetadataDescriptors[0];

         pVideoDescriptor->stCommon.uiLength = sizeof( BAVC_VideoMetadataDescriptor );
         pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_METADATA;
         pVideoDescriptor->uiDataUnitType = BAVC_VideoMetadataType_eCommon;

         /* Populate metadata */
         pMetadataDescriptor->uiMetadataFlags |= BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_BITRATE_VALID;
         pMetadataDescriptor->stBitrate.uiMax = hVceOutput->state.stChannelCache.stEncodeSettings.stBitRate.uiMax;

         /* Populate Frame Rate */
         {
            /* Validate the size of the FW2PI array is equal to highest possible Frame rate code from FW */
            BDBG_ASSERT((unsigned)BAVC_FrameRateCodeNumEntries == (unsigned)(BAVC_FWMaxFrameRateCode+1)); /* +1 to account for "unknown" */

            if ( pITBIndexEntry->uiFrameRate < BAVC_FrameRateCodeNumEntries)
            {
               pMetadataDescriptor->uiMetadataFlags |= BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_FRAMERATE_VALID;
               pMetadataDescriptor->stFrameRate.eFrameRateCode = BVCE_P_FW2PI_FrameRateLUT[pITBIndexEntry->uiFrameRate];
            }

            /* Populate Coded Dimension */
            pMetadataDescriptor->uiMetadataFlags |= BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_DIMENSION_CODED_VALID;
            pMetadataDescriptor->stDimension.coded.uiHeight = pITBIndexEntry->uiHeight;
            pMetadataDescriptor->stDimension.coded.uiWidth = pITBIndexEntry->uiWidth;
         }

         if ( true == hVceOutput->state.stChannelCache.bValid )
         {
            /* Populate Buffer Info */
            pMetadataDescriptor->uiMetadataFlags |= BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_BUFFERINFO_VALID;
            pMetadataDescriptor->stBufferInfo = hVceOutput->state.stChannelCache.stStartEncodeSettings.stProtocolInfo;
         }

         /* Populate STC Snapshot */
         /* SW7425-4355: Populate uiEtsDtsOffset */
         {
            BERR_Code getStatusRC = BERR_UNKNOWN;
            BVCE_Channel_Status stChannelStatus;

            getStatusRC = BVCE_Channel_GetStatus(
               hVceOutput->state.hVceCh,
               &stChannelStatus
               );
            BERR_TRACE( getStatusRC );

            if ( BERR_SUCCESS == getStatusRC )
            {
               pMetadataDescriptor->uiMetadataFlags |= BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_TIMING_STC_VALID;
               pMetadataDescriptor->stTiming.uiSTCSnapshot = stChannelStatus.uiSTCSnapshot;

               pMetadataDescriptor->uiMetadataFlags |= BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_TIMING_ETS_DTS_OFFSET_VALID;
               pMetadataDescriptor->stTiming.uiEtsDtsOffset = stChannelStatus.uiEtsDtsOffset;
            }
         }

         /* SW7425-3360: Populate Chunk ID */
         if ( pMetadataDescriptor->stTiming.uiChunkId != pITBIndexEntry->uiChunkId )
         {
            BDBG_ASSERT( 0 == ( pITBIndexEntry->stFrameDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOC ) );
         }

         pMetadataDescriptor->uiMetadataFlags |= BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_TIMING_CHUNK_ID_VALID;
         pMetadataDescriptor->stTiming.uiChunkId = pITBIndexEntry->uiChunkId;
         hVceOutput->state.uiPreviousMetadata = pITBIndexEntry->uiMetadata;
         BDBG_MSG(("[%d][%d] Chunk ID: %d",
            hVceOutput->hVce->stOpenSettings.uiInstance,
            hVceOutput->state.hVceCh->stOpenSettings.uiInstance,
            pITBIndexEntry->uiChunkId
         ));
         hVceOutput->state.bMetadataSent = true;

      #if BVCE_P_TEST_MODE
         BVCE_Output_S_DumpDescriptor(
            hVceOutput,
            pVideoDescriptor
            );
      #endif

         if ( NULL != hVceOutput->hVce->stDebugFifo.hDebugFifo )
         {
            BVCE_P_DebugFifo_Entry *pstEntry;
            BDBG_Fifo_Token stToken;

            pstEntry = (BVCE_P_DebugFifo_Entry *) BDBG_Fifo_GetBuffer( hVceOutput->hVce->stDebugFifo.hDebugFifo, &stToken );
            if ( NULL != pstEntry )
            {
               pstEntry->stMetadata.eType = BVCE_DebugFifo_EntryType_eBufferDescriptor;
               pstEntry->stMetadata.uiInstance = hVceOutput->hVce->stOpenSettings.uiInstance;
               pstEntry->stMetadata.uiChannel = hVceOutput->stOpenSettings.uiInstance;
               pstEntry->stMetadata.uiTimestamp = 0;
               ( NULL != hVceOutput->hVce->hTimer ) ? BTMR_ReadTimer( hVceOutput->hVce->hTimer, &pstEntry->stMetadata.uiTimestamp ) : 0;
               pstEntry->data.stBufferDescriptor = *pVideoDescriptor;
               BDBG_Fifo_CommitBuffer( &stToken );
            }

            pstEntry = (BVCE_P_DebugFifo_Entry *) BDBG_Fifo_GetBuffer( hVceOutput->hVce->stDebugFifo.hDebugFifo, &stToken );
            if ( NULL != pstEntry )
            {
               pstEntry->stMetadata.eType = BVCE_DebugFifo_EntryType_eMetadataDescriptor;
               pstEntry->stMetadata.uiInstance = hVceOutput->hVce->stOpenSettings.uiInstance;
               pstEntry->stMetadata.uiChannel = hVceOutput->stOpenSettings.uiInstance;
               pstEntry->stMetadata.uiTimestamp = 0;
               ( NULL != hVceOutput->hVce->hTimer ) ? BTMR_ReadTimer( hVceOutput->hVce->hTimer, &pstEntry->stMetadata.uiTimestamp ) : 0;
               pstEntry->data.stMetadataDescriptor = *pMetadataDescriptor;
               BDBG_Fifo_CommitBuffer( &stToken );
            }
         }
      }
   }
}

static
void
BVCE_Output_S_SendVideoDescriptor(
   BVCE_Output_Handle hVceOutput,
   BVCE_P_Output_ITB_IndexEntry *pITBIndexEntry,
   uint64_t uiCDBEndOfPictureOffset
   )
{
   BAVC_VideoBufferDescriptor *pVideoDescriptor = NULL;
   BVCE_P_VideoBufferDescriptorReleaseMetadata *pVideoDescriptorReleaseMetadata;

   /* Get Video Descriptor for this ITB entry */
   pVideoDescriptor = &hVceOutput->stDescriptors.astDescriptors[hVceOutput->state.uiDescriptorWriteOffset];
   pVideoDescriptorReleaseMetadata = &hVceOutput->stDescriptors.astDescriptorReleaseMetadata[hVceOutput->state.uiDescriptorWriteOffset];
   hVceOutput->state.uiDescriptorWriteOffset++;
   hVceOutput->state.uiDescriptorWriteOffset %= hVceOutput->stOpenSettings.uiDescriptorQueueDepth;

   BKNI_Memset( pVideoDescriptorReleaseMetadata,
                0,
                sizeof( BVCE_P_VideoBufferDescriptorReleaseMetadata )
                );

   if ( ( true == hVceOutput->state.bNewITBEntry )
        && ( hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead == pITBIndexEntry->uiCDBAddress )
      )
   {
      *pVideoDescriptor = pITBIndexEntry->stFrameDescriptor;
      pVideoDescriptorReleaseMetadata->bHasITBEntry = true;
      hVceOutput->state.bNewITBEntry = false;
   }
   else
   {
      BKNI_Memset( pVideoDescriptor, 0, sizeof( BAVC_VideoBufferDescriptor ) );
   }

   if ( 0 != ( pVideoDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOS ) )
   {
      hVceOutput->state.bEOSDescriptorSent = true;
   }
   else
   {
      /* Populate LLDInfo for PES */
      if ( uiCDBEndOfPictureOffset >= hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead )
      {
         pVideoDescriptor->stCommon.uiLength = uiCDBEndOfPictureOffset - hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead;
      }
      else
      {
         /* CDB Wrap occurs, so we need to split this picture into two descriptors.  We handle the first one here. */
         pVideoDescriptor->stCommon.uiLength = hVceOutput->state.stCDBBuffer.stOffset.uiEnd - hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead;
      }

      if ( true == hVceOutput->state.stDataUnitDetect.bFound )
      {
         pVideoDescriptor->uiVideoFlags |= BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DATA_UNIT_START;
         pVideoDescriptor->uiDataUnitType = hVceOutput->state.stDataUnitDetect.uiType;
         hVceOutput->state.stDataUnitDetect.bFound = false;
      }

      /* Normalize the offset to 0 */
      pVideoDescriptor->stCommon.uiOffset = BVCE_P_Buffer_GetDeviceBaseOffset( hVceOutput->hOutputBuffers->stCDB.hBuffer );
      pVideoDescriptor->stCommon.uiOffset += hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead - hVceOutput->state.stCDBBuffer.stOffset.uiBase;

      hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead += pVideoDescriptor->stCommon.uiLength;
      if ( hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead >= hVceOutput->state.stCDBBuffer.stOffset.uiEnd )
      {
         hVceOutput->state.stCDBBuffer.stOffset.uiShadowRead -= ( hVceOutput->state.stCDBBuffer.stOffset.uiEnd - hVceOutput->state.stCDBBuffer.stOffset.uiBase );
      }

      pVideoDescriptorReleaseMetadata->uiShadowLength = pVideoDescriptor->stCommon.uiLength;

      /* SW7425-5889: If this is a end-of-sequence (h.264 nal_unit_type=10), then treat the nalu as though it came in a header only ITB */
      if ( 0 != ( pVideoDescriptor->uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DATA_UNIT_START ) )
      {
         if ( ( true == pITBIndexEntry->bIgnoreFrame )
              && ( 10 == ( pVideoDescriptor->uiDataUnitType & 0x1F ) ) )
         {
            pITBIndexEntry->bForceHeaderOnly = true;
         }
         else
         {
            pITBIndexEntry->bForceHeaderOnly = false;
         }
      }

      if ( ( true == pITBIndexEntry->bHeaderOnly )
           || ( true == pITBIndexEntry->bForceHeaderOnly ) )
      {
         /* SW7425-5889: If this is a header-only ITB, then zero out the flags */
         unsigned uiLength = pVideoDescriptor->stCommon.uiLength;
         unsigned uiOffset = pVideoDescriptor->stCommon.uiOffset;
         unsigned uiVideoFlags = ( pVideoDescriptor->uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DATA_UNIT_START );
         unsigned uiDataUnitType = pVideoDescriptor->uiDataUnitType;

         BKNI_Memset( pVideoDescriptor, 0, sizeof( BAVC_VideoBufferDescriptor ) );
         pVideoDescriptor->stCommon.uiLength = uiLength;
         pVideoDescriptor->stCommon.uiOffset = uiOffset;
         pVideoDescriptor->uiVideoFlags = uiVideoFlags;
         pVideoDescriptor->uiDataUnitType = uiDataUnitType;
      }
      /* SW7425-4608: If this is an ignore frame, then zero out the length/offset */
      else if ( true == pITBIndexEntry->bIgnoreFrame )
      {
         BKNI_Memset( pVideoDescriptor, 0, sizeof( BAVC_VideoBufferDescriptor ) );
      }
   }

#if BVCE_P_TEST_MODE
   BVCE_Output_S_DumpDescriptor(
      hVceOutput,
      pVideoDescriptor
      );
#endif

   BDBG_MODULE_MSG( BVCE_OUTPUT_DESC, ("@%08x (%08lx), f=%08x, p="BDBG_UINT64_FMT", op=%08x, escr=%08x, tpb=%04x, shr=%6d, vf=%08x, d="BDBG_UINT64_FMT" du=%08x",
            pVideoDescriptor->stCommon.uiOffset,
            (unsigned long) pVideoDescriptor->stCommon.uiLength,
            pVideoDescriptor->stCommon.uiFlags,
            BDBG_UINT64_ARG(pVideoDescriptor->stCommon.uiPTS),
            pVideoDescriptor->stCommon.uiOriginalPTS,
            pVideoDescriptor->stCommon.uiESCR,
            pVideoDescriptor->stCommon.uiTicksPerBit,
            pVideoDescriptor->stCommon.iSHR,
            pVideoDescriptor->uiVideoFlags,
            BDBG_UINT64_ARG(pVideoDescriptor->uiDTS),
            pVideoDescriptor->uiDataUnitType
            ));

   if ( NULL != hVceOutput->hVce->stDebugFifo.hDebugFifo )
   {
      BVCE_P_DebugFifo_Entry *pstEntry;
      BDBG_Fifo_Token stToken;

      pstEntry = (BVCE_P_DebugFifo_Entry *) BDBG_Fifo_GetBuffer( hVceOutput->hVce->stDebugFifo.hDebugFifo, &stToken );
      if ( NULL != pstEntry )
      {
         pstEntry->stMetadata.eType = BVCE_DebugFifo_EntryType_eBufferDescriptor;
         pstEntry->stMetadata.uiInstance = hVceOutput->hVce->stOpenSettings.uiInstance;
         pstEntry->stMetadata.uiChannel = hVceOutput->stOpenSettings.uiInstance;
         pstEntry->stMetadata.uiTimestamp = 0;
         ( NULL != hVceOutput->hVce->hTimer ) ? BTMR_ReadTimer( hVceOutput->hVce->hTimer, &pstEntry->stMetadata.uiTimestamp ) : 0;
         pstEntry->data.stBufferDescriptor = *pVideoDescriptor;
         BDBG_Fifo_CommitBuffer( &stToken );
      }
   }
}


/* BVCE_Output_GetBufferDescriptors -
 * Returns video buffer descriptors for CDB content in the
 * BAVC_VideoBufferDescriptor array(s)
 */
static
BERR_Code
BVCE_Output_S_GetBufferDescriptors(
   BVCE_Output_Handle hVceOutput,
   const BAVC_VideoBufferDescriptor **astDescriptors0,
   size_t *puiNumDescriptors0,
   const BAVC_VideoBufferDescriptor **astDescriptors1,
   size_t *puiNumDescriptors1
   )
{
   if ( ( NULL == astDescriptors0 )
        || ( NULL == puiNumDescriptors0 )
        || ( NULL == astDescriptors1 )
        || ( NULL == puiNumDescriptors1 ) )
   {
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   *astDescriptors0 = NULL;
   *puiNumDescriptors0 = 0;
   *astDescriptors1 = NULL;
   *puiNumDescriptors1 = 0;

   if ( true == hVceOutput->hVce->bWatchdogOccurred )
   {
      BDBG_WRN(("Watchdog occurred, no longer reading ITB/CDB for new data"));
   }
   else
   {
      BVCE_Output_S_CheckCabacReady( hVceOutput );

      BVCE_Output_S_HandlePicturelessEncode( hVceOutput );

      /* Make sure cabac itb interrupt fired before even looking at the ITB for the first time */
      if ( true == hVceOutput->state.bCabacInitializedShadow )
      {
         /* Snapshot the CDB/ITB read/valid pointers
          * NOTE: We MUST snapshot the CDB pointers first,
          * and then the ITB pointers so that we can properly
          * detect the end of the current frame.  If we read
          * the ITB first, and then the CDB, it is possible a
          * new ITB entry has been written in between the reads,
          * and the CDB write pointer now includes the beginning
          * of the next frame */
         BVCE_Output_S_CDB_UpdatePointers( hVceOutput );

         /* ITB Pointers MUST be updated AFTER CDB pointers to prevent race condition
          * If next picture is written to ITB/CDB after ITB has been updated but before
          * CDB has been updated, then when CDB pointers are updated, it will contain
          * data for the new picture, but ITB will not have a reference to the new picture.
          * So, the new picture's CDB data will incorrectly be treated as part of the
          * previous picture */
         BVCE_Output_S_ITB_UpdatePointers( hVceOutput );

         /* Update Output's Channel Cache */
         BVCE_Output_S_ChannelCacheUpdate( hVceOutput );

         /* Update ITB/CDB Cache */
         BVCE_Output_S_ITB_BufferCacheUpdate( hVceOutput );
         BVCE_Output_S_CDB_BufferCacheUpdate( hVceOutput );

         /* Update ITB Index */
         BVCE_Output_S_ITB_UpdateIndex( hVceOutput );

         while ( false == hVceOutput->state.bEOSDescriptorSent )
         {
            BVCE_P_Output_ITB_IndexEntry *pITBIndexEntry = NULL;
            uint64_t uiCDBEndOfPictureOffset;

            /* Check for Available ITB Entries */
            if ( false == BVCE_Output_S_IsFrameAvailable( hVceOutput ) ) break;

            pITBIndexEntry = BVCE_Output_S_ITB_Shadow_GetFrameIndexEntry( hVceOutput, 0 );

            /* SW7425-74: Data Unit Detection */
            BVCE_Output_S_DataUnitDetect( hVceOutput );

            BVCE_Output_S_DumpDescriptorLegacy( hVceOutput, pITBIndexEntry );

            if ( true == BVCE_Output_S_IsStartOfNewFrame( hVceOutput ) ) continue;

            if ( 0 == ( pITBIndexEntry->stFrameDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOS ) )
            {
               if ( false == BVCE_Output_S_IsFrameDataAvailable( hVceOutput ) ) break;
            }

            if ( false == BVCE_Output_S_IsDescriptorAvailable( hVceOutput, pITBIndexEntry ) ) break;

            uiCDBEndOfPictureOffset = BVCE_Output_S_CalculateEndOfPictureOffset( hVceOutput, NULL );

            BVCE_Output_S_SendMetadataDescriptor( hVceOutput, pITBIndexEntry );

            BVCE_Output_S_SendVideoDescriptor( hVceOutput, pITBIndexEntry, uiCDBEndOfPictureOffset);
         }
      }
   }

   {
      /* Assign array(s) and count(s) */
      if ( hVceOutput->state.uiDescriptorWriteOffset >= hVceOutput->state.uiDescriptorReadOffset )
      {
         *astDescriptors0 = &hVceOutput->stDescriptors.astDescriptors[hVceOutput->state.uiDescriptorReadOffset];
         *puiNumDescriptors0 = hVceOutput->state.uiDescriptorWriteOffset - hVceOutput->state.uiDescriptorReadOffset;

         *astDescriptors1 = NULL;
         *puiNumDescriptors1 = 0;
      }
      else
      {
         *astDescriptors0 = &hVceOutput->stDescriptors.astDescriptors[hVceOutput->state.uiDescriptorReadOffset];
         *puiNumDescriptors0 = hVceOutput->stOpenSettings.uiDescriptorQueueDepth - hVceOutput->state.uiDescriptorReadOffset;

         *astDescriptors1 = &hVceOutput->stDescriptors.astDescriptors[0];
         *puiNumDescriptors1 = hVceOutput->state.uiDescriptorWriteOffset;
      }
   }

   BDBG_MODULE_MSG( BVCE_OUTPUT_PARSER, ("New Descriptors: %lu", (unsigned long) ((*puiNumDescriptors0 + *puiNumDescriptors1) - (hVceOutput->state.uiPendingDescriptors - hVceOutput->state.uiConsumedDescriptors))));

   hVceOutput->state.uiPendingDescriptors = *puiNumDescriptors0 + *puiNumDescriptors1;
   hVceOutput->state.uiConsumedDescriptors = 0;

   if ( ( true == hVceOutput->state.bEOSITBEntryIndexed )
        && ( NULL != hVceOutput->state.hVceCh ) )
   {
      BVCE_Channel_GetStatus( hVceOutput->state.hVceCh, NULL );
      if ( BVCE_P_Status_eOpened == BVCE_Channel_P_GetState(hVceOutput->state.hVceCh ) )
      {
         hVceOutput->state.hVceCh = NULL;
      }
   }

   return BERR_TRACE( BERR_SUCCESS );
}


BERR_Code
BVCE_Output_GetBufferDescriptors(
   BVCE_Output_Handle hVceOutput,
   const BAVC_VideoBufferDescriptor **astDescriptors0,
   size_t *puiNumDescriptors0,
   const BAVC_VideoBufferDescriptor **astDescriptors1,
   size_t *puiNumDescriptors1
   )
{
   BERR_Code rc;
   BDBG_OBJECT_ASSERT(hVceOutput, BVCE_P_Output_Context);
   BDBG_ENTER( BVCE_Output_GetBufferDescriptors );
   BVCE_P_FUNCTION_TRACE_ENTER(1, hVceOutput->hVce, hVceOutput->stOpenSettings.uiInstance);

   BVCE_Power_P_AcquireResource(
         hVceOutput->hVce,
         BVCE_Power_Type_eClock
         );

   rc = BVCE_Output_S_GetBufferDescriptors(
      hVceOutput,
      astDescriptors0,
      puiNumDescriptors0,
      astDescriptors1,
      puiNumDescriptors1
      );

   BVCE_Power_P_ReleaseResource(
         hVceOutput->hVce,
         BVCE_Power_Type_eClock
         );

   BVCE_P_FUNCTION_TRACE_LEAVE(1, hVceOutput->hVce, hVceOutput->stOpenSettings.uiInstance);
   BDBG_LEAVE( BVCE_Output_GetBufferDescriptors );
   return BERR_TRACE( rc );
}

/* BVCE_Output_ConsumeBufferDescriptors -
 * Reclaims the specified number of video buffer descriptors
 * The CDB read pointer is updated accordingly
 */
static
BERR_Code
BVCE_Output_S_ConsumeBufferDescriptors(
   BVCE_Output_Handle hVceOutput,
   size_t uiNumBufferDescriptors
   )
{
   BERR_Code rc = BERR_SUCCESS;
   uint64_t uiCDBReadOffset;
   uint64_t uiCDBEndOffset;
   uint64_t uiCDBBaseOffset;

   if ( hVceOutput->state.uiConsumedDescriptors != hVceOutput->state.uiPendingDescriptors )
   {
      uiCDBReadOffset = BREG_ReadAddr(
               hVceOutput->hVce->handles.hReg,
               hVceOutput->stRegisters.CDB_Read
               );

      uiCDBEndOffset = BREG_ReadAddr(
               hVceOutput->hVce->handles.hReg,
               hVceOutput->stRegisters.CDB_End
               );

      uiCDBEndOffset++;

      uiCDBBaseOffset = BREG_ReadAddr(
               hVceOutput->hVce->handles.hReg,
               hVceOutput->stRegisters.CDB_Base
               );

      hVceOutput->state.uiConsumedDescriptors += uiNumBufferDescriptors;

      while ( uiNumBufferDescriptors )
      {
         if ( 0 != ( hVceOutput->stDescriptors.astDescriptors[hVceOutput->state.uiDescriptorReadOffset].stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_METADATA ) )
         {
            /* SW7425-3360: We have could have multiple metadata descriptors, so increment the metadata descriptor read offset */
            hVceOutput->state.uiMetadataDescriptorReadOffset++;
            hVceOutput->state.uiMetadataDescriptorReadOffset %= BVCE_P_MAX_METADATADESCRIPTORS;
         }
         else
         {
            /* Move CDB Read Offset */
            uiCDBReadOffset += hVceOutput->stDescriptors.astDescriptorReleaseMetadata[hVceOutput->state.uiDescriptorReadOffset].uiShadowLength;

            /* 7425A0: Hack to prevent HW/SW confusion of full vs empty */
            if ( false == hVceOutput->state.bCDBReadHackDone )
            {
               uiCDBReadOffset--;
               hVceOutput->state.bCDBReadHackDone = true;
            }
            if ( uiCDBReadOffset >= uiCDBEndOffset )
            {
               uiCDBReadOffset -= ( uiCDBEndOffset - uiCDBBaseOffset );
            }

            if ( true == hVceOutput->stDescriptors.astDescriptorReleaseMetadata[hVceOutput->state.uiDescriptorReadOffset].bHasITBEntry )
            {
               /* Move ITB Read Offset */
               BVCE_Output_S_ITB_ConsumeEntry( hVceOutput );
            }
         }

         /* Zero out descriptor release metadata */
         BKNI_Memset( &hVceOutput->stDescriptors.astDescriptorReleaseMetadata[hVceOutput->state.uiDescriptorReadOffset],
                      0,
                      sizeof( BVCE_P_VideoBufferDescriptorReleaseMetadata )
                      );

         /* Move Descriptor Read Offset */
         hVceOutput->state.uiDescriptorReadOffset++;
         hVceOutput->state.uiDescriptorReadOffset %= hVceOutput->stOpenSettings.uiDescriptorQueueDepth;

         uiNumBufferDescriptors--;
      }

      /* Update Actual ITB/CDB Read Pointers */
      BREG_WriteAddr(
               hVceOutput->hVce->handles.hReg,
               hVceOutput->stRegisters.CDB_Read,
               uiCDBReadOffset
               );

      BREG_WriteAddr(
               hVceOutput->hVce->handles.hReg,
               hVceOutput->stRegisters.ITB_Read,
               hVceOutput->state.stITBBuffer.stOffset.uiRead
               );
   }
   else
   {
      if ( 0 != uiNumBufferDescriptors )
      {
         BDBG_WRN(("BVCE_Output_ConsumeBufferDescriptors called but there aren't any pending descriptors!"));
         rc = BERR_TRACE(BVCE_ERR_NO_DESC);
      }
   }

   return BERR_TRACE( rc );
}

BERR_Code
BVCE_Output_ConsumeBufferDescriptors(
   BVCE_Output_Handle hVceOutput,
   size_t uiNumBufferDescriptors
   )
{
   BERR_Code rc;
   BDBG_OBJECT_ASSERT(hVceOutput, BVCE_P_Output_Context);
   BDBG_ENTER( BVCE_Output_ConsumeBufferDescriptors );
   BVCE_P_FUNCTION_TRACE_ENTER(1, hVceOutput->hVce, hVceOutput->stOpenSettings.uiInstance);

   hVceOutput->state.eBufferAccessMode = BVCE_P_Output_BufferAccessMode_eDescriptor;

   BVCE_Power_P_AcquireResource(
         hVceOutput->hVce,
         BVCE_Power_Type_eClock
         );

   rc = BVCE_Output_S_ConsumeBufferDescriptors(
      hVceOutput,
      uiNumBufferDescriptors
      );

   BVCE_Power_P_ReleaseResource(
         hVceOutput->hVce,
         BVCE_Power_Type_eClock
         );

   BVCE_P_FUNCTION_TRACE_LEAVE(1, hVceOutput->hVce, hVceOutput->stOpenSettings.uiInstance);
   BDBG_LEAVE( BVCE_Output_ConsumeBufferDescriptors );
   return BERR_TRACE( rc );
}

static
bool
BVCE_Output_S_ReadIndex_HandleFrameDone(
      BVCE_Output_Handle hVceOutput,
      BVCE_OUTPUT_P_ITB_IndexingState eNextITBEntryType,
      BAVC_VideoBufferDescriptor *pstDescriptor
      )
{
   if ( true == hVceOutput->state.stReadIndex.bPartiallyFilled )
   {
      /* Decide if we are done filling in the previous descriptor */
      switch ( eNextITBEntryType )
      {
         case BVCE_OUTPUT_P_ITB_IndexingState_eBaseEntry:
         case BVCE_OUTPUT_P_ITB_IndexingState_eBaseOffsetEntry:
         case BVCE_OUTPUT_P_ITB_IndexingState_eEOSEntry:
         {
#ifndef BVCE_PLATFORM_P_SUPPORTS_ITB_EOS
            BVCE_Output_S_ITB_CheckForEOS(&hVceOutput->state.stReadIndex.stVideoDescriptor);
#endif
            *pstDescriptor = hVceOutput->state.stReadIndex.stVideoDescriptor;

            /* Check for ReadIndex Overflow */
            if ( 0 != ( hVceOutput->state.stReadIndex.stVideoDescriptor.uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DTS_VALID ) )
            {
               if ( true == hVceOutput->state.stReadIndex.bPreviousDTSValid )
               {
                  uint32_t uiCurrentDTS = hVceOutput->state.stReadIndex.stVideoDescriptor.uiDTS >> 1;
                  if ( ( uiCurrentDTS - hVceOutput->state.stReadIndex.uiPreviousDTS ) > ( 45000 * 2 ) )
                  {
                     BDBG_ERR(("ReadIndex Overflow Detected!"));
                  }
               }
               hVceOutput->state.stReadIndex.uiPreviousDTS = hVceOutput->state.stReadIndex.stVideoDescriptor.uiDTS >> 1;
               hVceOutput->state.stReadIndex.bPreviousDTSValid = true;
            }

            hVceOutput->state.stReadIndex.bPartiallyFilled = false;
            BKNI_Memset( &hVceOutput->state.stReadIndex.stVideoDescriptor, 0, sizeof( hVceOutput->state.stReadIndex.stVideoDescriptor ) );
            return true;
         }
         default:
            break;
      }
   }

   return false;
}

static
BERR_Code
BVCE_Output_S_ReadIndex(
   BVCE_Output_Handle hVceOutput,
   BAVC_VideoBufferDescriptor *astDescriptors,
   unsigned uiNumDescriptorsMax,
   unsigned *puiNumDescriptorsRead
   )
{

   *puiNumDescriptorsRead = 0;

   if ( true == hVceOutput->hVce->bWatchdogOccurred )
   {
      BDBG_WRN(("Watchdog occurred, no longer reading ITB for new data"));
   }
   else
   {
      BVCE_Output_S_CheckCabacReady( hVceOutput );

      if ( true == hVceOutput->state.bCabacInitializedShadow )
      {
         BVCE_Output_S_CDB_UpdatePointers( hVceOutput );
         BVCE_Output_S_ITB_UpdatePointers( hVceOutput );
         BVCE_Output_S_ITB_BufferCacheUpdate( hVceOutput );

         {
            void *pITBBufferCached = BVCE_P_Buffer_LockAddress( hVceOutput->hOutputBuffers->stITB.hBuffer );
            if ( NULL == pITBBufferCached )
            {
               BDBG_ERR(("Error locking ITB memory block"));
               return BERR_TRACE( BERR_UNKNOWN );
            }

            while ( ( *puiNumDescriptorsRead != uiNumDescriptorsMax ) /* We still have descriptors available */
                    && ( hVceOutput->state.stReadIndex.uiShadowRead != hVceOutput->state.stITBBuffer.stOffset.uiValid ) ) /* We still have ITB entries to read */
            {
               void* pITBEntry = (void*) (((uint8_t *)pITBBufferCached) + (hVceOutput->state.stReadIndex.uiShadowRead - hVceOutput->state.stITBBuffer.stOffset.uiBase));
               BVCE_OUTPUT_P_ITB_IndexingState eIndexState = BVCE_P_ITBEntry_GetEntryType(pITBEntry);
               uint32_t uiOffset = 0;

               /* Calculate uiOffset for this frame */
               switch ( eIndexState )
               {
                  case BVCE_OUTPUT_P_ITB_IndexingState_eBaseEntry:
                     uiOffset = BVCE_P_ITBEntry_GetCDBAddress(pITBEntry) - hVceOutput->state.stCDBBuffer.stOffset.uiBase;
                     break;

                  case BVCE_OUTPUT_P_ITB_IndexingState_eBaseOffsetEntry:
                  case BVCE_OUTPUT_P_ITB_IndexingState_eEOSEntry:
                     uiOffset = BVCE_P_ITBEntry_GetCDBAddress(pITBEntry);
                     break;

                  default:
                     break;
               }

               if ( true == BVCE_Output_S_ReadIndex_HandleFrameDone( hVceOutput, eIndexState, &astDescriptors[*puiNumDescriptorsRead] ) )
               {
                  /* Calculate Length */
                  if ( astDescriptors[*puiNumDescriptorsRead].stCommon.uiOffset < uiOffset )
                  {
                     astDescriptors[*puiNumDescriptorsRead].stCommon.uiLength = uiOffset - astDescriptors[*puiNumDescriptorsRead].stCommon.uiOffset;
                  }
                  else
                  {
                     astDescriptors[*puiNumDescriptorsRead].stCommon.uiLength = ( hVceOutput->state.stCDBBuffer.stOffset.uiEnd - hVceOutput->state.stCDBBuffer.stOffset.uiBase ) - astDescriptors[*puiNumDescriptorsRead].stCommon.uiOffset + uiOffset;
                  }
                  (*puiNumDescriptorsRead)++;
                  continue;
               }

               BVCE_Output_S_ITB_ParseEntry( pITBEntry, &hVceOutput->state.stReadIndex.stVideoDescriptor );

               switch ( eIndexState )
               {
                  case BVCE_OUTPUT_P_ITB_IndexingState_eBaseEntry:
                  case BVCE_OUTPUT_P_ITB_IndexingState_eBaseOffsetEntry:
                  case BVCE_OUTPUT_P_ITB_IndexingState_eEOSEntry:
                     hVceOutput->state.stReadIndex.bPartiallyFilled = true;
                     hVceOutput->state.stReadIndex.stVideoDescriptor.stCommon.uiOffset = uiOffset;
                     break;
                  case BVCE_OUTPUT_P_ITB_IndexingState_eESCREntry:
                     /* SW7425-4608: Set Ignore frame flag */
                     if ( 0 != ( ( BVCE_P_ITBEntry_GetMetadata(pITBEntry) >> 29 ) & 0x1 ) )
                     {
                        hVceOutput->state.stReadIndex.bPartiallyFilled = false;
                        BKNI_Memset( &hVceOutput->state.stReadIndex.stVideoDescriptor, 0, sizeof( hVceOutput->state.stReadIndex.stVideoDescriptor ) );
                     }
                  default:
                     break;
               }

               hVceOutput->state.stReadIndex.uiShadowRead += 16;
               if ( hVceOutput->state.stReadIndex.uiShadowRead >= hVceOutput->state.stITBBuffer.stOffset.uiEnd )
               {
                  hVceOutput->state.stReadIndex.uiShadowRead -= ( hVceOutput->state.stITBBuffer.stOffset.uiEnd - hVceOutput->state.stITBBuffer.stOffset.uiBase );
               }
            }

            if ( true == BVCE_Output_S_ReadIndex_HandleFrameDone( hVceOutput, BVCE_OUTPUT_P_ITB_IndexingState_eEOSEntry, &astDescriptors[*puiNumDescriptorsRead] ) )
            {
               (*puiNumDescriptorsRead)++;
            }

            BVCE_P_Buffer_UnlockAddress( hVceOutput->hOutputBuffers->stITB.hBuffer );
         }
      }
   }

   return BERR_SUCCESS;
}

BERR_Code
BVCE_Output_ReadIndex(
   BVCE_Output_Handle hVceOutput,
   BAVC_VideoBufferDescriptor *astDescriptors,
   unsigned uiNumDescriptorsMax,
   unsigned *puiNumDescriptorsRead
   )
{
   BERR_Code rc;
   BDBG_OBJECT_ASSERT(hVceOutput, BVCE_P_Output_Context);
   BDBG_ASSERT( astDescriptors );
   BDBG_ASSERT( puiNumDescriptorsRead );
   BDBG_ENTER( BVCE_Output_ReadIndex );
   BVCE_P_FUNCTION_TRACE_ENTER(1, hVceOutput->hVce, hVceOutput->stOpenSettings.uiInstance);

   BVCE_Power_P_AcquireResource(
         hVceOutput->hVce,
         BVCE_Power_Type_eClock
         );

   rc = BVCE_Output_S_ReadIndex(
      hVceOutput,
      astDescriptors,
      uiNumDescriptorsMax,
      puiNumDescriptorsRead
      );

   BVCE_Power_P_ReleaseResource(
         hVceOutput->hVce,
         BVCE_Power_Type_eClock
         );

   BVCE_P_FUNCTION_TRACE_LEAVE(1, hVceOutput->hVce, hVceOutput->stOpenSettings.uiInstance);
   BDBG_LEAVE( BVCE_Output_ReadIndex );
   return BERR_TRACE( rc );
}

static
BERR_Code
BVCE_Output_S_GetBufferStatus_impl(
   BVCE_Output_Handle hVceOutput,
   BAVC_VideoBufferStatus *pBufferStatus
   )
{
   BDBG_ASSERT( pBufferStatus );

   BKNI_Memset(
            pBufferStatus,
            0,
            sizeof( BAVC_VideoBufferStatus )
            );

   if ( NULL != hVceOutput )
   {
      /* SW7425-5384: Send BMMA_Block_Handles to the mux */
      pBufferStatus->stCommon.hFrameBufferBlock = BVCE_P_Buffer_GetBlockHandle( hVceOutput->hOutputBuffers->stCDB.hBuffer );
      pBufferStatus->stCommon.hMetadataBufferBlock = BVCE_P_Buffer_GetBlockHandle( hVceOutput->stDescriptors.hMetadataBuffer );
      pBufferStatus->stCommon.hIndexBufferBlock = BVCE_P_Buffer_GetBlockHandle( hVceOutput->hOutputBuffers->stITB.hBuffer );
      BDBG_ASSERT( pBufferStatus->stCommon.hFrameBufferBlock );
      BDBG_ASSERT( pBufferStatus->stCommon.hMetadataBufferBlock );
      BDBG_ASSERT( pBufferStatus->stCommon.hIndexBufferBlock );

      {
         uint64_t uiBaseOffset, uiEndOffset, uiValidOffset, uiReadOffset;

         /* Read CDB Pointers */
         uiBaseOffset = BREG_ReadAddr(
                  hVceOutput->hVce->handles.hReg,
                  hVceOutput->stRegisters.CDB_Base
                  );

         uiEndOffset = BREG_ReadAddr(
                  hVceOutput->hVce->handles.hReg,
                  hVceOutput->stRegisters.CDB_End
                  );

         /* HW treats END as last valid byte before wrap.
          * It is easier for SW to treat END as the byte
          * AFTER the last valid byte before WRAP, so we
          * add one here
          */
         uiEndOffset += 1;

         uiValidOffset = BREG_ReadAddr(
                  hVceOutput->hVce->handles.hReg,
                  hVceOutput->stRegisters.CDB_Valid
                  );

         uiReadOffset = BREG_ReadAddr(
                 hVceOutput->hVce->handles.hReg,
                 hVceOutput->stRegisters.CDB_Read
                 );

         /* Calculate CDB Depth/Size */
         pBufferStatus->stCommon.stCDB.uiSize = uiEndOffset - uiBaseOffset;
         if ( uiValidOffset >= uiReadOffset )
         {
            pBufferStatus->stCommon.stCDB.uiDepth = uiValidOffset - uiReadOffset;
         }
         else
         {
            pBufferStatus->stCommon.stCDB.uiDepth = (uiEndOffset - uiReadOffset) + (uiValidOffset - uiBaseOffset);
         }

         /* Read ITB Pointers */
         uiBaseOffset = BREG_ReadAddr(
                  hVceOutput->hVce->handles.hReg,
                  hVceOutput->stRegisters.ITB_Base
                  );

         uiEndOffset = BREG_ReadAddr(
                  hVceOutput->hVce->handles.hReg,
                  hVceOutput->stRegisters.ITB_End
                  );

         /* HW treats END as last valid byte before wrap.
          * It is easier for SW to treat END as the byte
          * AFTER the last valid byte before WRAP, so we
          * add one here
          */
         uiEndOffset += 1;

         uiValidOffset = BREG_ReadAddr(
                  hVceOutput->hVce->handles.hReg,
                  hVceOutput->stRegisters.ITB_Valid
                  );

         uiReadOffset = BREG_ReadAddr(
                 hVceOutput->hVce->handles.hReg,
                 hVceOutput->stRegisters.ITB_Read
                 );

         /* Calculate ITB Depth/Size */
         pBufferStatus->stCommon.stITB.uiSize = uiEndOffset - uiBaseOffset;
         if ( uiValidOffset >= uiReadOffset )
         {
            pBufferStatus->stCommon.stITB.uiDepth = uiValidOffset - uiReadOffset;
         }
         else
         {
            pBufferStatus->stCommon.stITB.uiDepth = (uiEndOffset - uiReadOffset) + (uiValidOffset - uiBaseOffset);
         }
      }
   }

   return BERR_TRACE( BERR_SUCCESS );
}

/* BVCE_Output_GetBufferStatus -
 * Returns the output buffer status (e.g. the base virtual address)
 */
BERR_Code
BVCE_Output_GetBufferStatus(
   BVCE_Output_Handle hVceOutput,
   BAVC_VideoBufferStatus *pBufferStatus
   )
{
   BERR_Code rc;
   BDBG_OBJECT_ASSERT(hVceOutput, BVCE_P_Output_Context);
   BDBG_ENTER( BVCE_Output_GetBufferStatus );
   BVCE_P_FUNCTION_TRACE_ENTER(1, hVceOutput->hVce, hVceOutput->stOpenSettings.uiInstance);

   BVCE_Power_P_AcquireResource(
         hVceOutput->hVce,
         BVCE_Power_Type_eClock
         );

   rc = BVCE_Output_S_GetBufferStatus_impl(
      hVceOutput,
      pBufferStatus
      );

   BVCE_Power_P_ReleaseResource(
         hVceOutput->hVce,
         BVCE_Power_Type_eClock
         );

   BVCE_P_FUNCTION_TRACE_LEAVE(1, hVceOutput->hVce, hVceOutput->stOpenSettings.uiInstance);
   BDBG_LEAVE( BVCE_Output_GetBufferStatus );
   return BERR_TRACE( rc );
}

static
void
BVCE_Output_S_Flush(
   BVCE_Output_Handle hVceOutput
   )
{
   BDBG_OBJECT_ASSERT(hVceOutput, BVCE_P_Output_Context);

   if ( BVCE_P_Output_BufferAccessMode_eDirect != hVceOutput->state.eBufferAccessMode )
   {
      const BAVC_VideoBufferDescriptor *astDescriptors[2];
      size_t uiNumDescriptors[2];

      do
      {
         BVCE_Output_S_GetBufferDescriptors(
            hVceOutput,
            &astDescriptors[0],
            &uiNumDescriptors[0],
            &astDescriptors[1],
            &uiNumDescriptors[1]
         );

         BVCE_Output_S_ConsumeBufferDescriptors(
            hVceOutput,
            uiNumDescriptors[0] + uiNumDescriptors[1]
         );
      } while ( uiNumDescriptors[0] + uiNumDescriptors[1] );
   }
   else
   {
      BVCE_Output_S_FlushDirect( hVceOutput );
   }
}

void
BVCE_Output_Flush(
   BVCE_Output_Handle hVceOutput
   )
{
   BDBG_ENTER( BVCE_Output_Flush );

   BDBG_OBJECT_ASSERT(hVceOutput, BVCE_P_Output_Context);
   BVCE_P_FUNCTION_TRACE_ENTER(0, hVceOutput->hVce, hVceOutput->stOpenSettings.uiInstance);

   BVCE_Power_P_AcquireResource(
         hVceOutput->hVce,
         BVCE_Power_Type_eClock
         );

   BVCE_Output_S_Flush( hVceOutput );

   BVCE_Power_P_ReleaseResource(
         hVceOutput->hVce,
         BVCE_Power_Type_eClock
         );

   BVCE_P_FUNCTION_TRACE_LEAVE(0, hVceOutput->hVce, hVceOutput->stOpenSettings.uiInstance);
   BDBG_LEAVE( BVCE_Output_Flush );
}

BERR_Code
BVCE_Channel_Output_Reset(
         BVCE_Channel_Handle hVceCh
         )
{
   BERR_Code rc;
   BVCE_Output_Handle hVceOutput = NULL;

   BDBG_ENTER( BVCE_Channel_Output_Reset );

   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);
   BVCE_P_FUNCTION_TRACE_ENTER(0, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);

   hVceOutput = hVceCh->stStartEncodeSettings.hOutputHandle;
   if ( NULL == hVceOutput )
   {
      hVceOutput = hVceCh->stOutput.hVceOutput;
   }

   BDBG_ASSERT( NULL != hVceOutput );
   BDBG_OBJECT_ASSERT(hVceOutput, BVCE_P_Output_Context);

   rc = BVCE_Output_Reset( hVceOutput );

   BVCE_P_FUNCTION_TRACE_LEAVE(0, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);
   BDBG_LEAVE( BVCE_Channel_Output_Reset );

   return BERR_TRACE( rc );
}

BERR_Code
BVCE_Channel_Output_GetRegisters(
         BVCE_Channel_Handle hVceCh,
         BAVC_VideoContextMap *pstVceChOutputRegisters
         )
{
   BERR_Code rc;
   BVCE_Output_Handle hVceOutput = NULL;

   BDBG_ENTER( BVCE_Channel_Output_GetRegisters );

   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);
   BVCE_P_FUNCTION_TRACE_ENTER(1, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);

   hVceOutput = hVceCh->stStartEncodeSettings.hOutputHandle;
   if ( NULL == hVceOutput )
   {
      hVceOutput = hVceCh->stOutput.hVceOutput;
   }

   BDBG_ASSERT( NULL != hVceOutput );
   BDBG_OBJECT_ASSERT(hVceOutput, BVCE_P_Output_Context);

   rc = BVCE_Output_GetRegisters( hVceOutput, pstVceChOutputRegisters );

   BVCE_P_FUNCTION_TRACE_LEAVE(1, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);
   BDBG_LEAVE( BVCE_Channel_Output_GetRegisters );

   return BERR_TRACE( rc );
}

BERR_Code
BVCE_Channel_Output_GetBufferDescriptors(
   BVCE_Channel_Handle hVceCh,
   const BAVC_VideoBufferDescriptor **astDescriptors0, /* Pointer to an array of descriptors. E.g. *astDescriptorsX[0] is the first descriptor. *astDescriptorsX may be set to NULL iff uiNumDescriptorsX=0. */
   size_t *puiNumDescriptors0,
   const BAVC_VideoBufferDescriptor **astDescriptors1, /* Needed to handle FIFO wrap */
   size_t *puiNumDescriptors1
   )
{
   BERR_Code rc;
   BVCE_Output_Handle hVceOutput = NULL;

   BDBG_ENTER( BVCE_Channel_Output_GetBufferDescriptors );

   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);
   BVCE_P_FUNCTION_TRACE_ENTER(1, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);

   hVceOutput = hVceCh->stStartEncodeSettings.hOutputHandle;
   if ( NULL == hVceOutput )
   {
      hVceOutput = hVceCh->stOutput.hVceOutput;
   }

   BDBG_ASSERT( NULL != hVceOutput );
   BDBG_OBJECT_ASSERT(hVceOutput, BVCE_P_Output_Context);

   rc = BVCE_Output_GetBufferDescriptors( hVceOutput, astDescriptors0, puiNumDescriptors0, astDescriptors1, puiNumDescriptors1 );

   BVCE_P_FUNCTION_TRACE_LEAVE(1, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);
   BDBG_LEAVE( BVCE_Channel_Output_GetBufferDescriptors );

   return BERR_TRACE( rc );
}

BERR_Code
BVCE_Channel_Output_ConsumeBufferDescriptors(
   BVCE_Channel_Handle hVceCh,
   size_t uiNumBufferDescriptors
   )
{
   BERR_Code rc;
   BVCE_Output_Handle hVceOutput = NULL;

   BDBG_ENTER( BVCE_Channel_Output_ConsumeBufferDescriptors );

   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);
   BVCE_P_FUNCTION_TRACE_ENTER(1, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);

   hVceOutput = hVceCh->stStartEncodeSettings.hOutputHandle;
   if ( NULL == hVceOutput )
   {
      hVceOutput = hVceCh->stOutput.hVceOutput;
   }

   BDBG_ASSERT( NULL != hVceOutput );
   BDBG_OBJECT_ASSERT(hVceOutput, BVCE_P_Output_Context);

   rc = BVCE_Output_ConsumeBufferDescriptors( hVceOutput, uiNumBufferDescriptors );

   BVCE_P_FUNCTION_TRACE_LEAVE(1, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);
   BDBG_LEAVE( BVCE_Channel_Output_ConsumeBufferDescriptors );

   return BERR_TRACE( rc );
}

BERR_Code
BVCE_Channel_Output_ReadIndex(
   BVCE_Channel_Handle hVceCh,
   BAVC_VideoBufferDescriptor *astDescriptors,
   unsigned uiNumDescriptorsMax,
   unsigned *puiNumDescriptorsRead
   )
{
   BERR_Code rc;
   BVCE_Output_Handle hVceOutput = NULL;

   BDBG_ENTER( BVCE_Channel_Output_ReadIndex );

   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);
   BVCE_P_FUNCTION_TRACE_ENTER(1, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);

   hVceOutput = hVceCh->stStartEncodeSettings.hOutputHandle;
   if ( NULL == hVceOutput )
   {
      hVceOutput = hVceCh->stOutput.hVceOutput;
   }

   BDBG_ASSERT( NULL != hVceOutput );
   BDBG_OBJECT_ASSERT(hVceOutput, BVCE_P_Output_Context);

   rc = BVCE_Output_ReadIndex( hVceOutput, astDescriptors, uiNumDescriptorsMax, puiNumDescriptorsRead );

   BVCE_P_FUNCTION_TRACE_LEAVE(1, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);
   BDBG_LEAVE( BVCE_Channel_Output_ReadIndex );

   return BERR_TRACE( rc );
}

BERR_Code
BVCE_Channel_Output_GetBufferStatus(
   BVCE_Channel_Handle hVceCh,
   BAVC_VideoBufferStatus *pBufferStatus
   )
{
   BERR_Code rc;
   BVCE_Output_Handle hVceOutput = NULL;

   BDBG_ENTER( BVCE_Channel_Output_GetBufferStatus );

   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);
   BVCE_P_FUNCTION_TRACE_ENTER(1, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);

   hVceOutput = hVceCh->stStartEncodeSettings.hOutputHandle;
   if ( NULL == hVceOutput )
   {
      hVceOutput = hVceCh->stOutput.hVceOutput;
   }

   BDBG_ASSERT( NULL != hVceOutput );
   BDBG_OBJECT_ASSERT(hVceOutput, BVCE_P_Output_Context);

   rc = BVCE_Output_GetBufferStatus( hVceOutput, pBufferStatus );

   BVCE_P_FUNCTION_TRACE_LEAVE(1, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);
   BDBG_LEAVE( BVCE_Channel_Output_GetBufferStatus );

   return BERR_TRACE( rc );
}

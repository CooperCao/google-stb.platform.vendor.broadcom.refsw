/******************************************************************************
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
 ******************************************************************************/

/* base modules */
#include "bstd.h"           /* standard types */
#include "bkni.h"           /* kernel interface */

#include "btmr.h"
#include "bafl.h"

#include "bavc.h"
#include "bavc_vce_mbox.h"

#include "bvce.h"
#include "bvce_priv.h"
#include "bvce_image.h"

#include "bvce_platform.h"
#include "bvce_fw_api_common.h"
#include "bvce_debug_priv.h"
#include "bvce_telem.h"

#include "bvce_output.h"
#include "bvce_power.h"
#include "bvce_buffer.h"
#include "bvce_debug_common.h"

BDBG_MODULE(BVCE);
BDBG_FILE_MODULE(BVCE_CMD);
BDBG_FILE_MODULE(BVCE_RSP);
BDBG_FILE_MODULE(BVCE_USERDATA);
BDBG_FILE_MODULE(BVCE_MEMORY);
BDBG_FILE_MODULE(BVCE_PLATFORM);

BDBG_OBJECT_ID(BVCE_P_Context);           /* BVCE_Handle */
BDBG_OBJECT_ID(BVCE_P_Channel_Context);   /* BVCE_Channel_Handle */
BDBG_OBJECT_ID(BVCE_P_Output_Context);    /* BVCE_Output_Handle */

/**********************/
/* Encoder Open/Close */
/**********************/
void BVCE_GetDefaultPlatformSettings(
   BVCE_PlatformSettings *pstPlatformSettings
   )
{
   if ( NULL != pstPlatformSettings )
   {
      BKNI_Memset( pstPlatformSettings, 0, sizeof( BVCE_PlatformSettings ) );
   }
}

static const BVCE_OpenSettings s_stDefaultOpenSettings =
{
 BVCE_P_SIGNATURE_OPENSETTINGS, /* Signature */

 0, /* Instance */

 /* Memory and Heaps */
 {
  NULL,
  NULL
 },
 { 0, 0 },
 NULL, /* Picture Heap */
 NULL, /* Secure Heap */
 /* Memory Config */
 {
  0,
  0,
  0,
  0,
  0,
  0,
 },

 /* Firmware and Boot */
 NULL, /* Image Interface */
 NULL, /* Image Context */
 NULL, /* Boot Callback */
 NULL, /* Boot Callback Data */
 false, /* Boot Enable */

 /* Debug */
 NULL, /* Timer Handle */
 /* Debug Log Buffer Size */
 {
  BVCE_P_DEFAULT_DEBUG_LOG_SIZE,
  BVCE_P_DEFAULT_DEBUG_LOG_SIZE,
 },
 /* Debug Log Buffering Mode */
 {
  BVCE_Debug_BufferingMode_eDiscardNewData, /* TODO: Which default do we want to PicArc? */
  BVCE_Debug_BufferingMode_eDiscardNewData  /* TODO: Which default do we want to MBArc? */
 },
 false, /* Verification Mode */
 false, /* A2N Picture Drop */
 NULL, /* BOX Handle */
};

void
BVCE_GetDefaultOpenSettings(
         const BVCE_PlatformSettings *pstPlatformSettings,
         BVCE_OpenSettings  *pstOpenSettings /* [out] Default VCE settings */
         )
{
   BDBG_ENTER( BVCE_GetDefaultOpenSettings );

   BDBG_ASSERT( pstOpenSettings );

   if ( NULL != pstOpenSettings )
   {
      *pstOpenSettings = s_stDefaultOpenSettings;

#if !(BVCE_USE_CUSTOM_IMAGE)
      pstOpenSettings->pImgInterface = &BVCE_IMAGE_Interface;
#if !(BVCE_USE_CUSTOM_CONTEXT) && !(BVCE_USE_FILE_IMAGE)
      pstOpenSettings->pImgContext = BVCE_IMAGE_Context;
#endif
#endif

      if ( NULL != pstPlatformSettings )
      {
         /* SW7435-1069: Calculate memory size using BVCE_Channel_GetMemoryConfig() */
         {
            BVCE_MemoryConfig stMemoryConfig;
            unsigned uiTotalChannels;

            BVCE_Channel_GetMemoryConfig( pstPlatformSettings->hBox, NULL, NULL, &stMemoryConfig );
            BVCE_Platform_P_GetTotalChannels( pstPlatformSettings->hBox, pstPlatformSettings->uiInstance, &uiTotalChannels );

#define BVCE_MEMCONFIG_FIELD(_field) pstOpenSettings->stMemoryConfig._field = ( stMemoryConfig._field * uiTotalChannels );
#include "bvce_memconfig.inc"

         /* Add device memory config */
         BVCE_GetMemoryConfig( NULL, NULL, &stMemoryConfig );
#define BVCE_MEMCONFIG_FIELD(_field) pstOpenSettings->stMemoryConfig._field += stMemoryConfig._field;
#include "bvce_memconfig.inc"

         {
            BBOX_Config *pstBoxConfig = (BBOX_Config *) BKNI_Malloc( sizeof( BBOX_Config ) );
            if ( NULL != pstBoxConfig )
            {
               BBOX_GetConfig( pstPlatformSettings->hBox, pstBoxConfig );

               BDBG_MODULE_MSG( BVCE_MEMORY, ("BVCE_GetDefaultOpenSettings(boxMode=%d, instance=%d):", pstBoxConfig->stBox.ulBoxId, 0 ));
               BKNI_Free( pstBoxConfig );
            }
            else
            {
               BDBG_MODULE_MSG( BVCE_MEMORY, ("BVCE_GetDefaultOpenSettings(boxMode=?, instance=%d):", 0 ));
            }
         }
#define BVCE_MEMCONFIG_FIELD(_field) BDBG_MODULE_MSG(BVCE_MEMORY, (#_field"=%lu bytes", (unsigned long) pstOpenSettings->stMemoryConfig._field));
#include "bvce_memconfig.inc"
      }

         pstOpenSettings->hBox = pstPlatformSettings->hBox;
         pstOpenSettings->uiInstance = pstPlatformSettings->uiInstance;
      }
   }

   BDBG_LEAVE( BVCE_GetDefaultOpenSettings );
   return;
}

static
BERR_Code
BVCE_S_CreateTimer(
         BVCE_Handle hVce
         )
{
   BERR_Code rc = BERR_SUCCESS;

   if ( NULL != hVce->stOpenSettings.hTimer )
   {
      BTMR_Settings tmrSettings;

      rc = BTMR_GetDefaultTimerSettings(&tmrSettings);

      if (BERR_SUCCESS == rc)
      {
         tmrSettings.type = BTMR_Type_eSharedFreeRun;
         tmrSettings.exclusive = false;

         rc = BTMR_CreateTimer(hVce->stOpenSettings.hTimer,
                               &hVce->hTimer,
                               &tmrSettings);

      }
   }

   return BERR_TRACE( rc );
}

static
void
BVCE_S_DestroyTimer(
         BVCE_Handle hVce
         )
{
   if ( NULL != hVce->hTimer )
   {
      BTMR_DestroyTimer(
               hVce->hTimer
               );
   }
}

static
BERR_Code
BVCE_S_Reset(
         BVCE_Handle hVce
         )
{
   BERR_Code rc;

   rc = BVCE_Platform_P_PreReset( hVce->stOpenSettings.uiInstance, hVce->handles.hReg );
   BERR_TRACE( rc );

   if ( false == hVce->bBooted )
   {
      /* Set SW_INIT_MODE=0 */
      BVCE_Platform_P_WriteRegisterList(
         hVce->handles.hReg,
         hVce->stPlatformConfig.stViceWatchdogHandlerDisable.astRegisterSettings,
         hVce->stPlatformConfig.stViceWatchdogHandlerDisable.uiRegisterCount
      );
   }

   rc = BVCE_Platform_P_WriteRegisterList(
            hVce->handles.hReg,
            hVce->stPlatformConfig.stViceReset.astRegisterSettings,
            hVce->stPlatformConfig.stViceReset.uiRegisterCount
            );

   return BERR_TRACE( rc );
}

static
void
BVCE_S_DestroyAllocators(
   BVCE_Handle hVce
   )
{
   {
      uint32_t i;

      for (i = 0; i < BVCE_PLATFORM_P_NUM_ARC_CORES; i++ )
      {
         BVCE_P_Allocator_Destroy( hVce->fw.memory[i].hAllocator );
         hVce->fw.memory[i].hAllocator = NULL;
      }
   }

   {
      BVCE_P_HeapId eHeapId;

      for ( eHeapId = 0; eHeapId < BVCE_P_HeapId_eMax; eHeapId++ )
      {
         BVCE_P_Allocator_Destroy( hVce->ahAllocator[eHeapId] );
         hVce->ahAllocator[eHeapId] = NULL;
      }
   }
}

static const unsigned BVCE_P_FirmwareSizeLUT[BVCE_PLATFORM_P_NUM_ARC_CORES] =
{
   MAX_PICARC_FW_SIZE_IN_BYTES,
   MAX_PICARC_FW_SIZE_IN_BYTES
};

static
BERR_Code
BVCE_S_SetupAllocators(
   BVCE_Handle hVce
   )
{
   BERR_Code rc;
   BMMA_Heap_Handle hMem = NULL;
   BVCE_P_Buffer_AllocSettings stAllocSettings;

   BVCE_P_Buffer_GetDefaultAllocSettings( &stAllocSettings );
   stAllocSettings.uiAlignment = BVCE_P_DEFAULT_ALIGNMENT;

   /* Firmware Allocator(s) */
   {
      uint32_t i;

      for (i = 0; i < BVCE_PLATFORM_P_NUM_ARC_CORES; i++ )
      {
         stAllocSettings.uiSize = BVCE_P_FirmwareSizeLUT[i];

         if ( NULL != hVce->stOpenSettings.hFirmwareMem[i] )
         {
            BDBG_MODULE_MSG( BVCE_MEMORY, ("FW Heap[%d]: BVCE_OpenSettings.hFirmwareMem[%d]", i, i) );
            hMem = hVce->stOpenSettings.hFirmwareMem[i];
         }
         else
         {
            BDBG_MODULE_MSG( BVCE_MEMORY, ("FW Heap[%d]: BVCE_Open.hMem", i) );
            hMem = hVce->handles.hMem;
         }

         rc = BVCE_P_Allocator_Create(
            hMem,
            &stAllocSettings,
            &hVce->fw.memory[i].hAllocator
            );

         if ( BERR_SUCCESS != rc )
         {
            BVCE_S_DestroyAllocators( hVce );
            return BERR_TRACE( rc );
         }

         BDBG_MODULE_MSG( BVCE_MEMORY, ("Created Firmware Memory Allocator[%d]: %08lx bytes @ "BDBG_UINT64_FMT,
            i,
            (unsigned long) BVCE_P_Allocator_GetSize( hVce->fw.memory[i].hAllocator ),
            BDBG_UINT64_ARG(BVCE_P_Allocator_GetDeviceOffset( hVce->fw.memory[i].hAllocator ))
            ));
      }
   }

   /* System Allocator */
   stAllocSettings.uiSize = hVce->stOpenSettings.stMemoryConfig.uiGeneralMemSize;

   rc = BVCE_P_Allocator_Create(
      hVce->handles.hMem,
      &stAllocSettings,
      &hVce->ahAllocator[BVCE_P_HeapId_eSystem]
      );

   if ( BERR_SUCCESS != rc )
   {
      BVCE_S_DestroyAllocators( hVce );
      return BERR_TRACE( rc );
   }

   BDBG_MODULE_MSG( BVCE_MEMORY, ("Created Memory Allocator[%d]: %08lx bytes @ "BDBG_UINT64_FMT,
      BVCE_P_HeapId_eSystem,
      (unsigned long) BVCE_P_Allocator_GetSize( hVce->ahAllocator[BVCE_P_HeapId_eSystem] ),
      BDBG_UINT64_ARG(BVCE_P_Allocator_GetDeviceOffset( hVce->ahAllocator[BVCE_P_HeapId_eSystem] ))
      ));

   /* Picture Allocator */
   stAllocSettings.uiSize = hVce->stOpenSettings.stMemoryConfig.uiPictureMemSize;

   if ( NULL != hVce->stOpenSettings.hPictureMem )
   {
      BDBG_MODULE_MSG( BVCE_MEMORY, ("Picture Heap: BVCE_OpenSettings.hPictureMem") );
      hMem = hVce->stOpenSettings.hPictureMem;
   }
   else
   {
      BDBG_MODULE_MSG( BVCE_MEMORY, ("Picture Heap: BVCE_Open.hMem") );
      hMem = hVce->handles.hMem;
   }

   rc = BVCE_P_Allocator_Create(
      hMem,
      &stAllocSettings,
      &hVce->ahAllocator[BVCE_P_HeapId_ePicture]
      );

   if ( BERR_SUCCESS != rc )
   {
      BVCE_S_DestroyAllocators( hVce );
      return BERR_TRACE( rc );
   }

   BDBG_MODULE_MSG( BVCE_MEMORY, ("Created Memory Allocator[%d]: %08lx bytes @ "BDBG_UINT64_FMT,
      BVCE_P_HeapId_ePicture,
      (unsigned long) BVCE_P_Allocator_GetSize( hVce->ahAllocator[BVCE_P_HeapId_ePicture] ),
      BDBG_UINT64_ARG(BVCE_P_Allocator_GetDeviceOffset( hVce->ahAllocator[BVCE_P_HeapId_ePicture] ))
      ));

   /* Secure Allocator */
   stAllocSettings.uiSize = hVce->stOpenSettings.stMemoryConfig.uiSecureMemSize;

   if ( NULL != hVce->stOpenSettings.hSecureMem )
   {
      BDBG_MODULE_MSG( BVCE_MEMORY, ("Secure Heap: BVCE_OpenSettings.hSecureMem") );
      hMem = hVce->stOpenSettings.hSecureMem;
   }
   else if ( NULL != hVce->stOpenSettings.hPictureMem )
   {
      BDBG_MODULE_MSG( BVCE_MEMORY, ("Secure Heap: BVCE_OpenSettings.hPictureMem") );
      hMem = hVce->stOpenSettings.hPictureMem;
   }
   else
   {
      BDBG_MODULE_MSG( BVCE_MEMORY, ("Secure Heap: BVCE_Open.hMem") );
      hMem = hVce->handles.hMem;
   }

   rc = BVCE_P_Allocator_Create(
      hMem,
      &stAllocSettings,
      &hVce->ahAllocator[BVCE_P_HeapId_eSecure]
      );

   if ( BERR_SUCCESS != rc )
   {
      BVCE_S_DestroyAllocators( hVce );
      return BERR_TRACE( rc );
   }

   BDBG_MODULE_MSG( BVCE_MEMORY, ("Created Memory Allocator[%d]: %08lx bytes @ "BDBG_UINT64_FMT,
      BVCE_P_HeapId_eSecure,
      (unsigned long) BVCE_P_Allocator_GetSize( hVce->ahAllocator[BVCE_P_HeapId_eSecure] ),
      BDBG_UINT64_ARG(BVCE_P_Allocator_GetDeviceOffset( hVce->ahAllocator[BVCE_P_HeapId_eSecure] ))
      ));

   return BERR_TRACE( BERR_SUCCESS );
}

static
BERR_Code
BVCE_S_AllocateDeviceMemory(
         BVCE_Handle hVce
         )
{
   BERR_Code rc = BERR_SUCCESS;

   /* Allocate memory */
#if ( BVCE_P_CORE_MAJOR <= 2 )
   {
      BVCE_P_Buffer_AllocSettings stAllocSettings;

      BVCE_P_Buffer_GetDefaultAllocSettings( &stAllocSettings );
      stAllocSettings.uiSize = MIN_CMD_BUFFER_SIZE_IN_BYTES;
      stAllocSettings.uiAlignment = BVCE_P_DEFAULT_ALIGNMENT;
      stAllocSettings.bLockOffset = true;

      rc = BVCE_P_Buffer_Alloc(
         hVce->ahAllocator[BVCE_P_HeapId_eSecure],
         &stAllocSettings,
         &hVce->hCabacCmdBuffer
         );

      if ( BERR_SUCCESS != rc )
      {
         BDBG_ERR(("Error allocating CABAC Command Buffer"));
         return BERR_TRACE( BERR_OUT_OF_DEVICE_MEMORY );
      }
   }

   BDBG_MODULE_MSG( BVCE_MEMORY, ("Allocate CABAC CMD Memory Buffer: %08lx bytes @ "BDBG_UINT64_FMT,
      (unsigned long) BVCE_P_Buffer_GetSize( hVce->hCabacCmdBuffer ),
      BDBG_UINT64_ARG(BVCE_P_Buffer_GetDeviceOffset_isrsafe( hVce->hCabacCmdBuffer ))
      ));
#else
   BSTD_UNUSED( hVce );
#endif

   return BERR_TRACE( rc );
}

static
void
BVCE_S_FreeDeviceMemory(
         BVCE_Handle hVce
         )
{
   /* Free Memory */
#if ( BVCE_P_CORE_MAJOR <= 2 )
   if ( NULL != hVce->hCabacCmdBuffer )
   {
      BDBG_MODULE_MSG( BVCE_MEMORY, ("Free CABAC CMD Memory Buffer: %08lx bytes @ "BDBG_UINT64_FMT,
         (unsigned long) BVCE_P_Buffer_GetSize( hVce->hCabacCmdBuffer ),
         BDBG_UINT64_ARG(BVCE_P_Buffer_GetDeviceOffset_isrsafe( hVce->hCabacCmdBuffer ))
         ));

      BVCE_P_Buffer_Free ( hVce->hCabacCmdBuffer );
      hVce->hCabacCmdBuffer = NULL;
   }
#else
   BSTD_UNUSED( hVce );
#endif
}

static
void
BVCE_S_FreeFirmwareMemory(
         BVCE_Handle hVce
         )
{
   uint32_t i;

   for ( i = 0; i < BVCE_PLATFORM_P_NUM_ARC_CORES; i++ )
   {
      BDBG_MODULE_MSG( BVCE_MEMORY, ("Free Firmware Code Buffer[%d]: @ "BDBG_UINT64_FMT,
         i,
         BDBG_UINT64_ARG(BVCE_P_Buffer_GetDeviceOffset_isrsafe( hVce->fw.memory[i].hBuffer ))
         ));

      BVCE_P_Buffer_Free( hVce->fw.memory[i].hBuffer );
      hVce->fw.memory[i].hBuffer = NULL;
   }
}

static
BERR_Code
BVCE_S_AllocateFirmwareMemory(
         BVCE_Handle hVce
         )
{
   BERR_Code rc;
   uint32_t i;
   BVCE_P_Buffer_AllocSettings stAllocSettings;

   BVCE_P_Buffer_GetDefaultAllocSettings( &stAllocSettings );
   stAllocSettings.uiAlignment = BVCE_P_DEFAULT_ALIGNMENT;
   stAllocSettings.bLockOffset = true;

   for ( i = 0; i < BVCE_PLATFORM_P_NUM_ARC_CORES; i++ )
   {
      stAllocSettings.uiSize = BVCE_P_FirmwareSizeLUT[i];

      rc = BVCE_P_Buffer_Alloc(
         hVce->fw.memory[i].hAllocator,
         &stAllocSettings,
         &hVce->fw.memory[i].hBuffer
         );

      if ( BERR_SUCCESS != rc )
      {
         BDBG_ERR(("Error allocating FW code buffer [%d]", i));
         BVCE_S_FreeFirmwareMemory( hVce );
         return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
      }

      BDBG_MODULE_MSG( BVCE_MEMORY, ("Allocate Firmware Code Buffer[%d]: %08lx bytes @ "BDBG_UINT64_FMT,
         i,
         (unsigned long) stAllocSettings.uiSize,
         BDBG_UINT64_ARG(BVCE_P_Buffer_GetDeviceOffset_isrsafe( hVce->fw.memory[i].hBuffer ))
         ));
   }

   return BERR_TRACE( BERR_SUCCESS );
}

static
BERR_Code
BVCE_S_LoadFirmware(
         BVCE_Handle hVce
         )
{
   BERR_Code rc;
   uint32_t i;
   uint64_t uiRegValue;

   for ( i = 0; i < BVCE_PLATFORM_P_NUM_ARC_CORES; i++ )
   {
      void *pCodeBufferCached = BVCE_P_Buffer_LockAddress( hVce->fw.memory[i].hBuffer );

      if ( NULL == pCodeBufferCached )
      {
         BDBG_ERR(("Error converting FW code memory block to cached address [%d]", i));
         return BERR_TRACE( BERR_UNKNOWN );
      }

      if ( NULL != hVce->stOpenSettings.pARCBootCallback )
      {
         /* If boot call back is set, zero out FW image.  */
         BKNI_Memset(pCodeBufferCached, 0, BVCE_P_Buffer_GetSize( hVce->fw.memory[i].hBuffer ));
      }

      rc = BAFL_Load(
               BVCE_P_CoreDeviceID[i],
               hVce->stOpenSettings.pImgInterface,
               (void**) hVce->stOpenSettings.pImgContext,
               i,
               pCodeBufferCached, /* Virtual Address */
               BVCE_P_Buffer_GetSize( hVce->fw.memory[i].hBuffer ),
               false,
               &hVce->fw.astFirmwareLoadInfo[i]
               );

      if ( BERR_SUCCESS != rc )
      {
         BVCE_P_Buffer_UnlockAddress( hVce->fw.memory[i].hBuffer );
         return BERR_TRACE( rc );
      }

      BVCE_P_Buffer_FlushCache_isrsafe(
         hVce->fw.memory[i].hBuffer,
         pCodeBufferCached,
         BVCE_P_Buffer_GetSize( hVce->fw.memory[i].hBuffer )
      );

      BVCE_P_Buffer_UnlockAddress( hVce->fw.memory[i].hBuffer );

      uiRegValue = BVCE_P_Buffer_GetDeviceOffset_isrsafe( hVce->fw.memory[i].hBuffer ) + ( (uint8_t*) hVce->fw.astFirmwareLoadInfo[i].stCode.pStartAddress - (uint8_t*) pCodeBufferCached );

      BREG_WriteAddr(
               hVce->handles.hReg,
               hVce->stPlatformConfig.stCore[i].uiInstructionStartPhysicalAddress,
               uiRegValue
               );

#if BDBG_DEBUG_BUILD
         {
           uint64_t uiRegValueActual = BREG_ReadAddr(
                                                   hVce->handles.hReg,
                                                   hVce->stPlatformConfig.stCore[i].uiInstructionStartPhysicalAddress
                                                   );

           BDBG_MSG(("@0x%08x <-- "BDBG_UINT64_FMT" ("BDBG_UINT64_FMT") - ARC[%d] %s",
                    hVce->stPlatformConfig.stCore[i].uiInstructionStartPhysicalAddress,
                    BDBG_UINT64_ARG(uiRegValue),
                    BDBG_UINT64_ARG(uiRegValueActual),
                    i,
                    "Instruction Start Address"
                    ));
         }
#endif

      /* NOTE: This code assumes the data is loaded AFTER the code */
      uiRegValue = (uint8_t*) hVce->fw.astFirmwareLoadInfo[i].stData.pStartAddress + hVce->fw.astFirmwareLoadInfo[i].stData.uiSize - (uint8_t*) hVce->fw.astFirmwareLoadInfo[i].stCode.pStartAddress;

      BREG_Write32(
               hVce->handles.hReg,
               hVce->stPlatformConfig.stCore[i].uiDataSpaceStartRelativeOffset,
               uiRegValue
               );

      if ( 0 != hVce->stPlatformConfig.stCore[i].uiDataSpaceStartSystemOffset )
      {
         BREG_WriteAddr(
                  hVce->handles.hReg,
                  hVce->stPlatformConfig.stCore[i].uiDataSpaceStartSystemOffset,
                  0
                  );
      }

#if BDBG_DEBUG_BUILD
      {
           uint64_t uiRegValueActual = BREG_Read32(
                                                   hVce->handles.hReg,
                                                   hVce->stPlatformConfig.stCore[i].uiDataSpaceStartRelativeOffset
                                                   );

           BDBG_MSG(("@0x%08x <-- "BDBG_UINT64_FMT" ("BDBG_UINT64_FMT") - ARC[%d] %s",
                    hVce->stPlatformConfig.stCore[i].uiDataSpaceStartRelativeOffset,
                    BDBG_UINT64_ARG(uiRegValue),
                    BDBG_UINT64_ARG(uiRegValueActual),
                    i,
                    "Data Start Address"
                    ));
         }
#endif
   }

   return BERR_TRACE( BERR_SUCCESS );
}

static
void
BVCE_S_Mailbox_isr(
         void *pContext,
         int iParam
         )
{
   BVCE_Handle hVce = (BVCE_Handle) pContext;

   BSTD_UNUSED(iParam);
   BDBG_OBJECT_ASSERT(hVce, BVCE_P_Context);

   BKNI_SetEvent_isr( hVce->events.hMailbox );
}

static
void
BVCE_S_Watchdog_isr(
         void *pContext,
         int iParam
         )
{
   BVCE_Handle hVce = ( BVCE_Handle ) pContext;
   unsigned uiWatchdogErrorCode = 0;
   unsigned uiProgramCounter = 0;

   BSTD_UNUSED(iParam);
   BDBG_OBJECT_ASSERT(hVce, BVCE_P_Context);

   hVce->bWatchdogOccurred = true;

   if ( 0 != hVce->stPlatformConfig.stDebug.uiArcHostIF[iParam] )
   {
      BREG_Write32(
         hVce->handles.hReg,
         hVce->stPlatformConfig.stDebug.uiArcHostIF[iParam],
         hVce->stPlatformConfig.stDebug.uiArcHostIFMask[iParam]
      );
   }

   if ( 0 != hVce->fw.dccm.uiWatchdogErrorCodeBaseAddress[iParam] )
   {
      uiWatchdogErrorCode = BREG_Read32(
                  hVce->handles.hReg,
                  hVce->fw.dccm.uiWatchdogErrorCodeBaseAddress[iParam]
                  );
   }

   if ( 0 != hVce->stPlatformConfig.stDebug.uiArcPC[iParam] )
   {
      uiProgramCounter = BREG_Read32(
                  hVce->handles.hReg,
                  hVce->stPlatformConfig.stDebug.uiArcPC[iParam]
                  );
   }

   BDBG_ERR(("ARC[%d] Watchdog w/ error code: %08x! (PC = %08x)", iParam, uiWatchdogErrorCode, uiProgramCounter));

   if ( ( true == hVce->callbacks.stCallbackSettings.stWatchdog.bEnable )
        && ( NULL != hVce->callbacks.stCallbackSettings.stWatchdog.fCallback )
      )
   {
      hVce->callbacks.stCallbackSettings.stWatchdog.fCallback(
               hVce->callbacks.stCallbackSettings.stWatchdog.pPrivateContext,
               hVce->callbacks.stCallbackSettings.stWatchdog.iPrivateParam,
               NULL
               );
   }
}

static
void
BVCE_S_Event_isr(
         void *pContext,
         int iParam
         )
{
   BVCE_Handle hVce = ( BVCE_Handle ) pContext;
   uint32_t uiChannelErrorStatus;
   uint32_t i;

   BSTD_UNUSED(iParam);
   BDBG_OBJECT_ASSERT(hVce, BVCE_P_Context);

   /* We handle the device error interrupt by reading the
    * channel error flags and then executing the channel's error
    * callback iff that channel is marked as having an error
    */

   /* Read device channel error status flag */
   uiChannelErrorStatus = BREG_Read32(
               hVce->handles.hReg,
               hVce->fw.dccm.uiChannelErrorStatusBaseAddress
               );

   for ( i = 0; i < BVCE_PLATFORM_P_NUM_ENCODE_CHANNELS; i++ )
   {
      if ( 0 != ( ( uiChannelErrorStatus >> i ) & 0x01 ) )
      {
         /* Trigger Channel's Error Callback */
         if ( ( true == hVce->channels[i].context.stCallbackSettings.stEvent.bEnable )
              && ( NULL != hVce->channels[i].context.stCallbackSettings.stEvent.fCallback )
            )
         {
            BDBG_MSG(("Executing channel[%d] error callback", i));
            hVce->channels[i].context.stCallbackSettings.stEvent.fCallback(
                     hVce->channels[i].context.stCallbackSettings.stEvent.pPrivateContext,
                     hVce->channels[i].context.stCallbackSettings.stEvent.iPrivateParam,
                     NULL
                     );
         }
      }
   }

   /* Check for device errors */
   if ( 0 != ( ( uiChannelErrorStatus >> DEVICE_ERROR_CHANNEL_ID ) & 0x01 ) )
   {
      BDBG_ERR(("Critical Error: VCE FW Stack Overflow Occurred!"));
   }
}

static
void
BVCE_S_DataReady_isr(
         void *pContext,
         int iParam
         )
{
   BVCE_Handle hVce = ( BVCE_Handle ) pContext;
   BDBG_OBJECT_ASSERT(hVce, BVCE_P_Context);

   hVce->outputs[iParam].context.state.bCabacInitializedActual = true;

   /* Trigger Channel's Data Ready Callback */
   if ( ( true == hVce->channels[iParam].context.stCallbackSettings.stDataReady.bEnable )
        && ( NULL != hVce->channels[iParam].context.stCallbackSettings.stDataReady.fCallback )
      )
   {
      BDBG_MSG(("Executing channel[%d] data ready callback", iParam));
      hVce->channels[iParam].context.stCallbackSettings.stDataReady.fCallback(
               hVce->channels[iParam].context.stCallbackSettings.stDataReady.pPrivateContext,
               hVce->channels[iParam].context.stCallbackSettings.stDataReady.iPrivateParam,
               NULL
               );
   }
}

static
BERR_Code
BVCE_S_SetupCallback(
         BVCE_Handle hVce,
         BINT_CallbackHandle *phCallback,
         BINT_Id intId,
         BINT_CallbackFunc func, /* [in] Callback function that should be called when the specified interrupt triggers */
         void * pParm1, /* [in] Parameter that is returned to callback function when interrupt triggers */
         int parm2 /* [in] Parameter that is returned to callback function when interrupt triggers */
         )
{
   BERR_Code rc;

   BDBG_OBJECT_ASSERT(hVce, BVCE_P_Context);
   BDBG_ASSERT( phCallback );

   if ( NULL != *phCallback )
   {
      rc = BINT_DisableCallback(
               *phCallback
               );
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }
   } else {
      rc = BINT_CreateCallback(
               phCallback,
               hVce->handles.hInt,
               intId,
               func,
               pParm1,
               parm2
               );

      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }
   }

   rc = BINT_EnableCallback(
            *phCallback
            );
   if (rc != BERR_SUCCESS )
   {
      return BERR_TRACE(rc);
   }

   return BERR_TRACE( BERR_SUCCESS );
}

static
BERR_Code
BVCE_S_TeardownCallback(
         BINT_CallbackHandle hCallback
         )
{
   BERR_Code rc = BERR_SUCCESS;

   if ( NULL != hCallback )
   {
      rc = BINT_DisableCallback( hCallback );
      BERR_TRACE( rc );

      rc = BINT_DestroyCallback( hCallback );
      BERR_TRACE( rc );
   }

   return BERR_TRACE( rc );
}

static
BERR_Code
BVCE_S_SetupInterrupts(
         BVCE_Handle hVce
         )
{
   BERR_Code rc = BERR_SUCCESS;

   /* Install BINT ISR Handlers */
   /* Mailbox Interrupt  */
   rc = BVCE_S_SetupCallback(
            hVce,
            &hVce->callbacks.ahCallbacks[BVCE_P_CALLBACK_MAILBOX],
            hVce->stPlatformConfig.stInterrupt.idMailbox,
            BVCE_S_Mailbox_isr,
            hVce,
            0
            );
   if (rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

   /* Arc Watchdog Interrupts */
   {
      uint32_t i = 0;
      for ( i = 0; i < BVCE_PLATFORM_P_NUM_ARC_CORES; i++ )
      {
         rc = BVCE_S_SetupCallback(
                  hVce,
                  &hVce->callbacks.ahCallbacks[BVCE_P_CALLBACK_WATCHDOG + i],
                  hVce->stPlatformConfig.stInterrupt.idWatchdog[i],
                  BVCE_S_Watchdog_isr,
                  hVce,
                  i
                  );
         if (rc != BERR_SUCCESS)
         {
            return BERR_TRACE(rc);
         }
      }
   }

   /* Error interrupt */
   rc = BVCE_S_SetupCallback(
            hVce,
            &hVce->callbacks.ahCallbacks[BVCE_P_CALLBACK_EVENT],
            hVce->stPlatformConfig.stInterrupt.idEvent,
            BVCE_S_Event_isr,
            hVce,
            0
            );
   if (rc != BERR_SUCCESS )
   {
      return BERR_TRACE(rc);
   }

#if !BVCE_P_POLL_FW_DATAREADY
   /* Data Ready */
   {
      uint32_t i = 0;

      for ( i = 0; i < BVCE_PLATFORM_P_NUM_OUTPUT_CHANNELS; i++ )
      {
         if ( 0 != hVce->stPlatformConfig.stInterrupt.idDataReady[i] )
         {
            rc = BVCE_S_SetupCallback(
               hVce,
               &hVce->callbacks.ahCallbacks[BVCE_P_CALLBACK_DATAREADY + i],
               hVce->stPlatformConfig.stInterrupt.idDataReady[i],
               BVCE_S_DataReady_isr,
               hVce,
               i
            );
         }
      }
      if (rc != BERR_SUCCESS )
      {
         return BERR_TRACE(rc);
      }
   }
#endif

   return BERR_TRACE( BERR_SUCCESS );
}

static
BERR_Code
BVCE_S_TeardownInterrupts(
         BVCE_Handle hVce
         )
{
   BERR_Code rc;

   /* Uninstall BINT ISR Handlers */
   /* Mailbox Interrupt  */
   rc = BVCE_S_TeardownCallback(
            hVce->callbacks.ahCallbacks[BVCE_P_CALLBACK_MAILBOX]
            );
   hVce->callbacks.ahCallbacks[BVCE_P_CALLBACK_MAILBOX] = NULL;
   if ( BERR_SUCCESS != rc )
   {
      BDBG_WRN(("Error destroying mailbox callback"));
   }

   /* Arc Watchdog Interrupts */
   {
      uint32_t i = 0;

      for ( i = 0; i < BVCE_PLATFORM_P_NUM_ARC_CORES; i++ )
      {
         rc = BVCE_S_TeardownCallback(
                  hVce->callbacks.ahCallbacks[BVCE_P_CALLBACK_WATCHDOG + i]
                  );
         hVce->callbacks.ahCallbacks[BVCE_P_CALLBACK_WATCHDOG + i] = NULL;
         if ( BERR_SUCCESS != rc )
         {
            BDBG_WRN(("Error destroying watchdog[%d] callback", i));
         }
      }
   }

   /* Error interrupt */
   rc = BVCE_S_TeardownCallback(
            hVce->callbacks.ahCallbacks[BVCE_P_CALLBACK_EVENT]
            );
   hVce->callbacks.ahCallbacks[BVCE_P_CALLBACK_EVENT] = NULL;
   if ( BERR_SUCCESS != rc )
   {
      BDBG_WRN(("Error destroying error callback"));
   }

   /* Data Ready */
   {
      uint32_t i = 0;

      for ( i = 0; i < BVCE_PLATFORM_P_NUM_OUTPUT_CHANNELS; i++ )
      {
         rc = BVCE_S_TeardownCallback(
            hVce->callbacks.ahCallbacks[BVCE_P_CALLBACK_DATAREADY + i]
                  );
         hVce->callbacks.ahCallbacks[BVCE_P_CALLBACK_DATAREADY + i] = NULL;
      }

      if (rc != BERR_SUCCESS )
      {
         BDBG_WRN(("Error destroying dataready[%d] callback", i));
      }
   }

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BVCE_P_EnableInterrupts(
         BVCE_Handle hVce,
         bool bEnable
         )
{
   BERR_Code rc = BERR_SUCCESS;
   unsigned i;

   for ( i = 0; i < BVCE_P_CALLBACK_MAX; i++ )
   {
      if ( NULL != hVce->callbacks.ahCallbacks[i] )
      {
         if ( true == bEnable )
         {
            rc = BINT_EnableCallback( hVce->callbacks.ahCallbacks[i] );
            BERR_TRACE( rc );
         }
         else
         {
            rc = BINT_DisableCallback( hVce->callbacks.ahCallbacks[i] );
            BERR_TRACE( rc );
         }

         if ( rc != BERR_SUCCESS )
         {
            return BERR_TRACE( rc );
         }
      }
   }

   return BERR_TRACE( BERR_SUCCESS );
}

static
BERR_Code
BVCE_S_CreateEvents(
         BVCE_Handle hVce
         )
{
   BERR_Code rc;

   rc = BKNI_CreateEvent(
            &hVce->events.hMailbox
            );

   return BERR_TRACE( rc );
}

static
void
BVCE_S_DestroyEvents(
         BVCE_Handle hVce
         )
{
   if ( NULL != hVce->events.hMailbox )
   {
      BKNI_DestroyEvent(
               hVce->events.hMailbox
               );
   }
}

static
BERR_Code
BVCE_S_Boot(
         BVCE_Handle hVce
         )
{
   BERR_Code rc;

   hVce->fw.dccm.uiRegisterBaseAddress[0] = hVce->stPlatformConfig.stCore[0].uiDCCMBase;
   hVce->fw.dccm.uiRegisterBaseAddress[1] = hVce->stPlatformConfig.stCore[1].uiDCCMBase;

#if BVCE_P_POLL_FW_MBX
   /* zero out the response mailbox*/

   BREG_Write32(
            hVce->handles.hReg,
            hVce->stPlatformConfig.stMailbox.uiVice2HostMailboxAddress,
            0
            );

   BDBG_MSG(("ARC[0] PC = %08x, STATUS32 = %08x",
         BREG_Read32(
             hVce->handles.hReg,
             hVce->stPlatformConfig.stDebug.uiArcPC[0]
             ),
         BREG_Read32(
             hVce->handles.hReg,
             hVce->stPlatformConfig.stDebug.uiPicArcStatus32
             )
         ));
#endif

   /* SW7445-581: Set the FW buffer MEMC register base address */
   if ( 0 != hVce->stPlatformConfig.stDebug.uiScratchRegister )
   {
      /* We need at least one MEMC */
      BDBG_ASSERT( 0 != hVce->stPlatformConfig.stDebug.auiMemcRegBaseLUT[0] );

      {
         unsigned uiValue;

         uiValue = hVce->stPlatformConfig.stDebug.auiMemcRegBaseLUT[hVce->stOpenSettings.firmwareMemc[0]];

         BREG_Write32(
            hVce->handles.hReg,
            hVce->stPlatformConfig.stDebug.uiScratchRegister,
            uiValue
         );

         BDBG_MODULE_MSG( BVCE_PLATFORM, ("@0x%08x <-- 0x%08x (0x%08x) - Tracelog Base Sentinel Address",
            hVce->stPlatformConfig.stDebug.uiScratchRegister,
            uiValue,
            BREG_Read32(
               hVce->handles.hReg,
               hVce->stPlatformConfig.stDebug.uiScratchRegister
               )
         ));
      }
   }

#if BVCE_PLATFORM_P_DISABLE_FME_RR
#include "bchp_memc_arb_0.h"
#include "bchp_memc_arb_1.h"
   {
      uint32_t auiFMEMemcArbRegister[2] = {
            BCHP_MEMC_ARB_0_CLIENT_INFO_47,
            BCHP_MEMC_ARB_1_CLIENT_INFO_47
      };
      uint32_t uiValue;
      unsigned i;

      for ( i = 0; i < 2; i++ )
      {
         uiValue = BREG_Read32( hVce->handles.hReg, auiFMEMemcArbRegister[i] );
         uiValue &= ~BCHP_MEMC_ARB_0_CLIENT_INFO_47_RR_EN_MASK;
         uiValue |= ( BCHP_MEMC_ARB_0_CLIENT_INFO_47_RR_EN_DISABLED << BCHP_MEMC_ARB_0_CLIENT_INFO_47_RR_EN_SHIFT );
         BREG_Write32( hVce->handles.hReg, auiFMEMemcArbRegister[i], uiValue );
      }
   }
#endif

   if ( NULL != hVce->stOpenSettings.pARCBootCallback )
   {
      uint32_t i;
      BAFL_BootInfo stBootInfo;
      BAFL_FirmwareInfo stFirmwareInfo[BVCE_PLATFORM_P_NUM_ARC_CORES];


      BKNI_Memset(
               &stBootInfo,
               0,
               sizeof( BAFL_BootInfo )
               );

      BKNI_Memset(
               &stFirmwareInfo,
               0,
               sizeof( BAFL_FirmwareInfo ) * BVCE_PLATFORM_P_NUM_ARC_CORES
               );


      for ( i = 0; i < BVCE_PLATFORM_P_NUM_ARC_CORES; i++ )
      {
         stFirmwareInfo[i].stCode = hVce->fw.astFirmwareLoadInfo[i].stCode;
         stFirmwareInfo[i].uiArcInstance = i;

         if ( i != BVCE_PLATFORM_P_NUM_ARC_CORES - 1)
         {
            stFirmwareInfo[i].pNext = &stFirmwareInfo[i + 1];
         }
      }

      stBootInfo.eMode = BAFL_BootMode_eNormal;
      stBootInfo.pstArc = &stFirmwareInfo[0];

      hVce->stOpenSettings.pARCBootCallback(
               hVce->stOpenSettings.pARCBootCallbackData,
               &stBootInfo
               );
   }

   /* Boot Core */
   if ( ( NULL == hVce->stOpenSettings.pARCBootCallback )
        || ( true == hVce->stOpenSettings.bARCBootEnable ) )
   {
      BVCE_Platform_P_WriteRegisterList(
               hVce->handles.hReg,
               hVce->stPlatformConfig.stViceBoot.astRegisterSettings,
               hVce->stPlatformConfig.stViceBoot.uiRegisterCount
               );
   }

   /* Verify Boot */
#if BVCE_P_POLL_FW_MBX
   {
      uint32_t i;

      rc = BERR_UNKNOWN;

      /* poll for response */
      for ( i = 0; i < BVCE_P_POLL_FW_COUNT; i++)
      {
#if 1 /* Do not sleep in emulation */
     BKNI_Sleep(200);
#else
     {
       uint32_t uiTemp = 0xFFFFFFFF;
       volatile uint32_t uiFoo = uiTemp;
       while (uiFoo)
         {
           uiFoo = uiTemp;
           uiTemp--;
         };
     }
#endif
     {
       uint32_t uiHost2Vice = BREG_Read32(
                          hVce->handles.hReg,
                          hVce->stPlatformConfig.stMailbox.uiHost2ViceMailboxAddress
                          );

       BDBG_MSG(("ARC[0] PC = %08x, STATUS32 = %08x, L2 = %08x, Host2Vice %08x, Vice2Host %08x",
                 BREG_Read32(
                             hVce->handles.hReg,
                             hVce->stPlatformConfig.stDebug.uiArcPC[0]
                             ),
                 BREG_Read32(
                             hVce->handles.hReg,
                             hVce->stPlatformConfig.stDebug.uiPicArcStatus32
                             ),
                 BREG_Read32(
                          hVce->handles.hReg,
                          hVce->stPlatformConfig.stInterrupt.uiInterruptStatusRegister
                          ),
          uiHost2Vice,
          BREG_Read32(
             hVce->handles.hReg,
             hVce->stPlatformConfig.stMailbox.uiVice2HostMailboxAddress
          )
             ));

       /* Wait for zero value in Host2Vice Mailbox Register */

       if ( 0 == uiHost2Vice )
       {
         rc = BERR_SUCCESS;
         break;
       }
     }
      }
   }
#else
   rc = BKNI_WaitForEvent(
            hVce->events.hMailbox,
            BVCE_P_FIRMWARE_BOOT_TIMEOUT
            );
#endif
   {
      /* Retrieve the boot sequence # */
     uint32_t uiHost2Vice = BREG_Read32(
                   hVce->handles.hReg,
                   hVce->stPlatformConfig.stMailbox.uiHost2ViceMailboxAddress
                   );

      if ( BERR_SUCCESS != rc )
      {
         BDBG_ERR(("FW Boot Timeout (%08x)", uiHost2Vice));
         return BERR_TRACE(rc);
      }
      else if ( 0 != uiHost2Vice )
      {
         BDBG_ERR(("FW Boot Failed (%08x)", uiHost2Vice));
         return BERR_TRACE( BERR_UNKNOWN );
      }
   }

   /* Retrieve the DCCM offset of the command buffer */
   hVce->fw.dccm.uiCommandBufferBaseOffset = BREG_Read32(
            hVce->handles.hReg,
            hVce->stPlatformConfig.stMailbox.uiVice2HostMailboxAddress
            );

   BDBG_MSG(("ARC[0] Vice2Host = %08x",
         hVce->fw.dccm.uiCommandBufferBaseOffset
         ));

   hVce->fw.dccm.uiCommandBufferBaseAddress = hVce->fw.dccm.uiRegisterBaseAddress[0] + hVce->fw.dccm.uiCommandBufferBaseOffset;

   return BERR_TRACE( BERR_SUCCESS );
}

static
BERR_Code
BVCE_S_DisableWatchdog(
         BVCE_Handle hVce
         )
{
   BERR_Code rc;

   /* Setup Watchdog */
   rc = BVCE_Platform_P_WriteRegisterList(
            hVce->handles.hReg,
            hVce->stPlatformConfig.stViceWatchdogDisable.astRegisterSettings,
            hVce->stPlatformConfig.stViceWatchdogDisable.uiRegisterCount
            );

   return BERR_TRACE( rc );
}

void
BVCE_P_ValidateStructSizes(void)
{
   BDBG_CWARNING( sizeof( ViceCmdInit_t ) == 14*4 );
   BDBG_CWARNING( sizeof( ViceCmdInitResponse_t ) == 11*4 );
   BDBG_CWARNING( sizeof( ViceCmdOpenChannel_t ) == 9*4 );
   BDBG_CWARNING( sizeof( ViceCmdOpenChannelResponse_t ) == 8*4 );
   BDBG_CWARNING( sizeof( ViceCmdStartChannel_t ) == 2*4 );
   BDBG_CWARNING( sizeof( ViceCmdStartChannelResponse_t ) == 2*4 );
   BDBG_CWARNING( sizeof( ViceCmdStopChannel_t ) == 3*4 );
   BDBG_CWARNING( sizeof( ViceCmdStopChannelResponse_t ) == 2*4 );
   BDBG_CWARNING( sizeof( ViceCmdCloseChannel_t ) == 2*4 );
   BDBG_CWARNING( sizeof( ViceCmdCloseChannelResponse_t ) == 2*4 );
   BDBG_CWARNING( sizeof( ViceCmdConfigChannel_t ) == 33*4 );
   BDBG_CWARNING( sizeof( ViceCmdConfigChannelResponse_t ) == 2*4 );
   BDBG_CWARNING( sizeof( ViceCmdDebugChannel_t ) == (3*4 + COMMAND_BUFFER_SIZE_BYTES));
   BDBG_CWARNING( sizeof( ViceCmdDebugChannelResponse_t ) == 2*4 );
   BDBG_CWARNING( sizeof( ViceCmdGetChannelStatus_t ) == 2*4 );
   BDBG_CWARNING( sizeof( ViceCmdGetChannelStatusResponse_t ) == (2*4 + 14*4 + 1*8) );
   BDBG_CWARNING( sizeof( ViceCmdGetDeviceStatus_t ) == 1*4 );
   BDBG_CWARNING( sizeof( ViceCmdGetDeviceStatusResponse_t ) == (2*4 + 2*4) );
}

void
BVCE_P_ValidateFrameRateEnum(void)
{
   BDBG_CASSERT( (unsigned) ENCODING_FRAME_RATE_CODE_UNKNOWN == (unsigned) BAVC_FrameRateCode_eUnknown );
   BDBG_CASSERT( (unsigned) ENCODING_FRAME_RATE_CODE_2397 == (unsigned) BAVC_FrameRateCode_e23_976 );
   BDBG_CASSERT( (unsigned) ENCODING_FRAME_RATE_CODE_2400 == (unsigned) BAVC_FrameRateCode_e24 );
   BDBG_CASSERT( (unsigned) ENCODING_FRAME_RATE_CODE_2500 == (unsigned) BAVC_FrameRateCode_e25 );
   BDBG_CASSERT( (unsigned) ENCODING_FRAME_RATE_CODE_2997 == (unsigned) BAVC_FrameRateCode_e29_97 );
   BDBG_CASSERT( (unsigned) ENCODING_FRAME_RATE_CODE_3000 == (unsigned) BAVC_FrameRateCode_e30 );
   BDBG_CASSERT( (unsigned) ENCODING_FRAME_RATE_CODE_5000 == (unsigned) BAVC_FrameRateCode_e50 );
   BDBG_CASSERT( (unsigned) ENCODING_FRAME_RATE_CODE_5994 == (unsigned) BAVC_FrameRateCode_e59_94 );
   BDBG_CASSERT( (unsigned) ENCODING_FRAME_RATE_CODE_6000 == (unsigned) BAVC_FrameRateCode_e60 );
   BDBG_CASSERT( (unsigned) ENCODING_FRAME_RATE_CODE_1498 == (unsigned) BAVC_FrameRateCode_e14_985 );
   BDBG_CASSERT( (unsigned) ENCODING_FRAME_RATE_CODE_0749 == (unsigned) BAVC_FrameRateCode_e7_493 );
   BDBG_CASSERT( (unsigned) ENCODING_FRAME_RATE_CODE_1000 == (unsigned) BAVC_FrameRateCode_e10 );
   BDBG_CASSERT( (unsigned) ENCODING_FRAME_RATE_CODE_1500 == (unsigned) BAVC_FrameRateCode_e15 );
   BDBG_CASSERT( (unsigned) ENCODING_FRAME_RATE_CODE_2000 == (unsigned) BAVC_FrameRateCode_e20 );
}

#define BVCE_P_WriteRegisters( handle, addr, buffer, size ) BVCE_S_WriteRegistersNew_isrsafe( handle, addr, buffer, size );
#define BVCE_P_WriteRegisters_isr( handle, addr, buffer, size ) BVCE_S_WriteRegistersNew_isrsafe( handle, addr, buffer, size );

static
void
BVCE_S_WriteRegistersNew_isrsafe(
         BVCE_Handle hVce,
     unsigned uiRegStartAddress,
     const uint32_t *pBuffer,
     unsigned uiSize
     )
{
   unsigned i;
#if BDBG_DEBUG_BUILD
   bool bIsCommand = false;
#endif
   uint32_t uiCommandIndex = 0;

#if BDBG_DEBUG_BUILD
   bIsCommand = ( uiRegStartAddress == hVce->fw.dccm.uiCommandBufferBaseAddress );

   if ( true == bIsCommand )
   {
      bIsCommand = false;
      for ( uiCommandIndex = 0; uiCommandIndex < BVCE_P_CommandLUT_size / sizeof( BVCE_P_CommandDebug ); uiCommandIndex++ )
      {
         if ( BVCE_P_CommandLUT[uiCommandIndex].uiCommand == pBuffer[0] )
         {
            bIsCommand = true;
            break;
         }
      }
   }
#endif

   for ( i = 0; i < ( uiSize / sizeof( uint32_t ) ); i++ )
   {
#if BDBG_DEBUG_BUILD
      if ( true == bIsCommand )
      {
         BDBG_MODULE_MSG( BVCE_CMD, ("@%08lx <-- 0x%08x - %s",
                  (unsigned long) (uiRegStartAddress + (i*sizeof( uint32_t ))),
                  pBuffer[i],
                  BVCE_P_CommandLUT[uiCommandIndex].szCommandParameterName[i]
                  ));
      }
      else
      {
         BDBG_MSG(("@%08lx <-- 0x%08x",
                  (unsigned long) (uiRegStartAddress + (i*sizeof( uint32_t ))),
                  pBuffer[i]));
      }
#endif
     BREG_Write32(
          hVce->handles.hReg,
          uiRegStartAddress + (i*sizeof( uint32_t )),
          pBuffer[i]
          );
   }

   return;
}

#define BVCE_P_ReadRegisters( handle, addr, buffer, size ) BVCE_S_ReadRegistersNew_isrsafe( handle, addr, buffer, size )
#define BVCE_P_ReadRegisters_isr( handle, addr, buffer, size ) BVCE_S_ReadRegistersNew_isrsafe( handle, addr, buffer, size )

static
void
BVCE_S_ReadRegistersNew_isrsafe(
         BVCE_Handle hVce,
     unsigned uiRegStartAddress,
     uint32_t *pBuffer,
     unsigned uiSize /* In bytes (32-bit multiple) */
     )
{
   unsigned i;
#if BDBG_DEBUG_BUILD
   bool bIsCommand = false;
#endif
   uint32_t uiCommandIndex = 0;

#if BDBG_DEBUG_BUILD
   bIsCommand = ( uiRegStartAddress == hVce->fw.dccm.uiCommandBufferBaseAddress );
#endif

   for ( i = 0; i < ( uiSize / sizeof( uint32_t ) ); i++ )
   {
     pBuffer[i] = BREG_Read32(
                  hVce->handles.hReg,
                  uiRegStartAddress + (i*sizeof( uint32_t ))
                  );

#if BDBG_DEBUG_BUILD
     /* Look up command */
     if ( ( true == bIsCommand ) &&
          ( 0 == i ) )
     {
        bIsCommand = false;
        for ( uiCommandIndex = 0; uiCommandIndex < BVCE_P_CommandLUT_size / sizeof( BVCE_P_CommandDebug ); uiCommandIndex++ )
        {
           if ( BVCE_P_CommandLUT[uiCommandIndex].uiCommand == pBuffer[0] )
           {
              bIsCommand = true;
              break;
           }
        }
     }

     if ( true == bIsCommand )
     {
        BDBG_MODULE_MSG( BVCE_RSP, ("@%08lx --> 0x%08x - %s",
                 (unsigned long) (uiRegStartAddress + (i*sizeof( uint32_t ))),
                 pBuffer[i],
                 BVCE_P_CommandLUT[uiCommandIndex].szResponseParameterName[i]
                 ));
     }
     else
     {
        BDBG_MSG(("@%08lx --> 0x%08x",
                 (unsigned long) (uiRegStartAddress + (i*sizeof( uint32_t ))),
                  pBuffer[i]));
     }
#endif
   }

   return;
}

static
BERR_Code
BVCE_S_SendCommand_impl(
         BVCE_Handle hVce,
         const BVCE_P_Command *pstCommand,
         BVCE_P_Response *pstResponse
         )
{
   BERR_Code rc = BERR_SUCCESS;

   if ( false == hVce->bWatchdogOccurred )
   {
      BKNI_ResetEvent(
               hVce->events.hMailbox
               );

      /* Write the command to the command buffer in DCCM */
      BDBG_MSG(("Sending Command"));

      if ( NULL != hVce->stDebugFifo.hDebugFifo )
      {
         BVCE_P_DebugFifo_Entry *pstEntry;
         BDBG_Fifo_Token stToken;

         pstEntry = (BVCE_P_DebugFifo_Entry *) BDBG_Fifo_GetBuffer( hVce->stDebugFifo.hDebugFifo, &stToken );
         if ( NULL != pstEntry )
         {
            pstEntry->stMetadata.eType = BVCE_DebugFifo_EntryType_eCommand;
            pstEntry->stMetadata.uiInstance = hVce->stOpenSettings.uiInstance;
            pstEntry->stMetadata.uiChannel = (VICE_CMD_INITIALIZE  == pstCommand->type.stGeneric.uiCommand ) ? 0 : pstCommand->type.stGeneric.uiChannel;
            pstEntry->stMetadata.uiTimestamp = 0;
            ( NULL != hVce->hTimer ) ? BTMR_ReadTimer( hVce->hTimer, &pstEntry->stMetadata.uiTimestamp ) : 0;
            pstEntry->data.stCommand = *pstCommand;
            BDBG_Fifo_CommitBuffer( &stToken );
         }
      }
      BVCE_P_WriteRegisters(
             hVce,
             hVce->fw.dccm.uiCommandBufferBaseAddress,
             pstCommand->data,
             sizeof( BVCE_P_Command )
             );

   #if BVCE_P_POLL_FW_MBX
      /* zero out the response mailbox*/
      BREG_Write32(
               hVce->handles.hReg,
               hVce->stPlatformConfig.stMailbox.uiVice2HostMailboxAddress,
               0
               );
   #endif

      /* Write the command buffer base offset to the HOST2VICE mailbox */
      BREG_Write32(
               hVce->handles.hReg,
               hVce->stPlatformConfig.stMailbox.uiHost2ViceMailboxAddress,
               hVce->fw.dccm.uiCommandBufferBaseOffset
               );

      /* Trigger a FW interrupt */
      BDBG_ASSERT( 0 != hVce->stPlatformConfig.stMailbox.uiHost2ViceInterruptAddress );

      BREG_Write32(
         hVce->handles.hReg,
         hVce->stPlatformConfig.stMailbox.uiHost2ViceInterruptAddress,
         hVce->stPlatformConfig.stMailbox.uiHost2ViceInterruptMask
      );

      /* Wait for event */
   #if BVCE_P_POLL_FW_MBX
      {
         uint32_t i;

         rc = BERR_UNKNOWN;

         /* poll for response */
         for ( i = 0; i < BVCE_P_POLL_FW_COUNT; i++)
         {
            uint32_t uiVice2Host;

            BKNI_Sleep(200);

            /* Wait for non-zero value in Vice2Host Mailbox Register */
            uiVice2Host = BREG_Read32(
                              hVce->handles.hReg,
                              hVce->stPlatformConfig.stMailbox.uiVice2HostMailboxAddress
                              );

            BDBG_MSG(("ARC[0] PC = %08x, STATUS32 = %08x, L2 = %08x, Vice2Host %08x",
                       BREG_Read32(
                                   hVce->handles.hReg,
                                   hVce->stPlatformConfig.stDebug.uiArcPC[0]
                                   ),
                       BREG_Read32(
                                   hVce->handles.hReg,
                                   hVce->stPlatformConfig.stDebug.uiPicArcStatus32
                                   ),
                       BREG_Read32(
                                hVce->handles.hReg,
                                hVce->stPlatformConfig.stInterrupt.uiInterruptStatusRegister
                                ),
                       uiVice2Host
                       ));

            if ( 0 != uiVice2Host
               )
            {
               rc = BERR_SUCCESS;
               break;
            }
         }
      }
   #else
      rc = BKNI_WaitForEvent(
               hVce->events.hMailbox,
               BVCE_P_FIRMWARE_COMMAND_TIMEOUT
               );
   #endif

      if ( BERR_SUCCESS != rc )
      {
         BDBG_ERR(("Firmware Command Timeout"));
         return BERR_TRACE( rc );
      }

      /* Copy Response */
      {
         uint32_t uiResponseOffset;

         uiResponseOffset = BREG_Read32(
                  hVce->handles.hReg,
                  hVce->stPlatformConfig.stMailbox.uiVice2HostMailboxAddress
                  );

         BDBG_MSG(("Received Response at DCCM Offset: %08x", uiResponseOffset));

         if ( 0 != uiResponseOffset )
         {
            /* Copy the response buffer from DCCM */
            BVCE_P_ReadRegisters(
                       hVce,
                       hVce->fw.dccm.uiRegisterBaseAddress[0] + uiResponseOffset,
                       pstResponse->data,
                       sizeof( BVCE_P_Response )
                       );

            if ( NULL != hVce->stDebugFifo.hDebugFifo )
            {
               BVCE_P_DebugFifo_Entry *pstEntry;
               BDBG_Fifo_Token stToken;

               pstEntry = (BVCE_P_DebugFifo_Entry *) BDBG_Fifo_GetBuffer( hVce->stDebugFifo.hDebugFifo, &stToken );
               if ( NULL != pstEntry )
               {
                  pstEntry->stMetadata.eType = BVCE_DebugFifo_EntryType_eResponse;
                  pstEntry->stMetadata.uiInstance = hVce->stOpenSettings.uiInstance;
                  pstEntry->stMetadata.uiChannel = (VICE_CMD_INITIALIZE  == pstCommand->type.stGeneric.uiCommand ) ? 0 : pstCommand->type.stGeneric.uiChannel;
                  pstEntry->stMetadata.uiTimestamp = 0;
                  ( NULL != hVce->hTimer ) ? BTMR_ReadTimer( hVce->hTimer, &pstEntry->stMetadata.uiTimestamp ) : 0;
                  pstEntry->data.stResponse = *pstResponse;
                  BDBG_Fifo_CommitBuffer( &stToken );
               }
            }

            if ( 0 != pstResponse->type.stGeneric.uiStatus )
            {
#if BDBG_DEBUG_BUILD
               char *szCommandFriendlyName = "Command (Unknown)";
               unsigned uiCommandIndex = 0;

               for ( uiCommandIndex = 0; uiCommandIndex < BVCE_P_CommandLUT_size / sizeof( BVCE_P_CommandDebug ); uiCommandIndex++ )
               {
                  if ( BVCE_P_CommandLUT[uiCommandIndex].uiCommand == pstResponse->type.stGeneric.uiCommand )
                  {
                     szCommandFriendlyName = BVCE_P_CommandLUT[uiCommandIndex].szCommandParameterName[0];
                     break;
                  }
               }

               BDBG_ERR(("%s [%08x] Failed with Status %s [%d]",
                        szCommandFriendlyName,
                        pstResponse->type.stGeneric.uiCommand,
                        BVCE_P_StatusLUT[pstResponse->type.stGeneric.uiStatus],
                        pstResponse->type.stGeneric.uiStatus
                        ));
#endif
               return BERR_TRACE( BERR_UNKNOWN );
            }
         }
         else
         {
            BDBG_WRN(("Response Buffer not received"));
            return BERR_TRACE( BERR_UNKNOWN );
         }
      }
   }
   else
   {
      BDBG_WRN(("Watchdog Occurred, ignoring VCE command"));
      rc = BERR_TRACE( BERR_UNKNOWN );
   }

    return BERR_TRACE( rc );
}

static
BERR_Code
BVCE_S_SendCommand(
         BVCE_Handle hVce,
         const BVCE_P_Command *pstCommand,
         BVCE_P_Response *pstResponse
         )
{
   BERR_Code rc;

   rc = BVCE_S_SendCommand_impl( hVce,
                            pstCommand,
                            pstResponse
                            );

   return BERR_TRACE( rc );
}

typedef struct BVCE_P_FirmwareMemorySettings
{
   uint32_t    StripeWidth;                                /* DRAM Stripe width for the given platform                          */
   uint32_t    X;                                          /* X in Xn+Y formula which is used for NBMY calculation              */
   uint32_t    Y;                                          /* Y in Xn+Y formula which is used for NBMY calculation              */
   uint32_t    WordSize;                                   /* 0: 16-bit.   1: 32-bit    2: 64-bit                               */
   uint32_t    BankType;                                   /* 0: 4 Banks.  1: 8 Banks.  2: 16 Banks.                            */
   uint32_t    PageSize;                                   /* 0: 1 Kbytes. 1: 2 Kbytes. 2: 4 Kbytes. 3: 8 Kbytes. 4: 16 Kbytes. */
   uint32_t    Grouping;                                   /* 0: Disable.  1: Enable.                                           */
} BVCE_P_FirmwareMemorySettings;

static
BERR_Code
BVCE_S_PopulateFirmwareMemorySettings(
   const BCHP_MemoryInfo *pstMemoryInfo,
   BVCE_P_FirmwareMemorySettings *pstFirmwareMemorySettings,
   unsigned uiNonSecureMemcIndex
   )
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_ASSERT( pstMemoryInfo );
   BDBG_ASSERT( pstFirmwareMemorySettings );

   BKNI_Memset( pstFirmwareMemorySettings, 0, sizeof( BVCE_P_FirmwareMemorySettings ) );

   /* Set WordSize */
   switch ( pstMemoryInfo->memc[uiNonSecureMemcIndex].mapVer )
   {
      case BCHP_ScbMapVer_eMap2:
      case BCHP_ScbMapVer_eMap5:
      {
         switch ( pstMemoryInfo->memc[uiNonSecureMemcIndex].width )
            {
               case 16:
                  pstFirmwareMemorySettings->WordSize = WORD_SIZE_GWORD;
                  break;

               case 32:
                  pstFirmwareMemorySettings->WordSize = WORD_SIZE_JWORD;
                  break;

               default:
                  BDBG_ERR(("Unsupported interface width (%d)", pstMemoryInfo->memc[uiNonSecureMemcIndex].width ));
                  return BERR_TRACE( BERR_NOT_SUPPORTED );
            }
         }
         break;

      case BCHP_ScbMapVer_eMap8:
         pstFirmwareMemorySettings->WordSize = WORD_SIZE_MWORD;
         break;

      default:
         BDBG_ERR(("Unsupported map version (%d)", pstMemoryInfo->memc[uiNonSecureMemcIndex].mapVer ));
         return BERR_TRACE( BERR_NOT_SUPPORTED );
   }

   /* Set BankType */
#if !BVCE_PLATFORM_P_SUPPORTS_GROUPAGE
   /* SW7439-252: If the VCE core doesn't support groupage, the bank type
    * needs to be divided by 2
    */
   if ( true == pstMemoryInfo->memc[uiNonSecureMemcIndex].groupageEnabled )
   {
      pstFirmwareMemorySettings->BankType = BANK_TYPE_4_BANKS;
   }
   else
#else
   pstFirmwareMemorySettings->Grouping = pstMemoryInfo->memc[uiNonSecureMemcIndex].groupageEnabled ? 1 : 0;
#endif
   {
      pstFirmwareMemorySettings->BankType = BANK_TYPE_8_BANKS;
   }

   /* Set Page Size */
   switch ( pstMemoryInfo->memc[uiNonSecureMemcIndex].ulPageSize )
   {
      case 1024:
         pstFirmwareMemorySettings->PageSize = PAGE_SIZE_1_KBYTES;
         break;

      case 2048:
         pstFirmwareMemorySettings->PageSize = PAGE_SIZE_2_KBYTES;
         break;

      case 4096:
         pstFirmwareMemorySettings->PageSize = PAGE_SIZE_4_KBYTES;
         break;

      case 8192:
         pstFirmwareMemorySettings->PageSize = PAGE_SIZE_8_KBYTES;
         break;

      case 16384:
         pstFirmwareMemorySettings->PageSize = PAGE_SIZE_16_KBYTES;
         break;

      default:
         BDBG_ERR(("Unsupported page size (%d)", pstMemoryInfo->memc[uiNonSecureMemcIndex].ulPageSize ));
         return BERR_TRACE( BERR_NOT_SUPPORTED );
   }

   /* Set Stripe Width */
   pstFirmwareMemorySettings->StripeWidth = pstMemoryInfo->memc[uiNonSecureMemcIndex].ulStripeWidth;

   /* Set X/Y */
   pstFirmwareMemorySettings->X = pstMemoryInfo->memc[uiNonSecureMemcIndex].ulMbMultiplier;
   pstFirmwareMemorySettings->Y = pstMemoryInfo->memc[uiNonSecureMemcIndex].ulMbRemainder;

   return BERR_TRACE( rc );
}

static
BERR_Code
BVCE_S_SendCommand_Init(
         BVCE_Handle hVce
         )
{
   BERR_Code rc;

   BDBG_CASSERT( HOST2VICE_MBOX_OFFSET == BAVC_VICE_MBOX_OFFSET_HOST2VICE );
   BDBG_CASSERT( VICE2HOST_MBOX_OFFSET == BAVC_VICE_MBOX_OFFSET_VICE2HOST );
   BDBG_CASSERT( BVN2VICE_MBOX_OFFSET == BAVC_VICE_MBOX_OFFSET_BVN2VICE );
   BDBG_CASSERT( BVN2VICE_MBOX_PAYLOAD_OFFSET == BAVC_VICE_MBOX_OFFSET_BVN2VICE_DATA_0_START );

   BKNI_Memset(
            &hVce->fw.stCommand,
            0,
            sizeof( BVCE_P_Command )
            );

   BKNI_Memset(
            &hVce->fw.stResponse,
            0,
            sizeof( BVCE_P_Response )
            );

   hVce->fw.stCommand.type.stInit.Command = VICE_CMD_INITIALIZE;
   hVce->fw.stCommand.type.stInit.API_Version = VICE_API_VERSION;

   /* Get stripe multiple/width from BCHP */
   {
      BCHP_MemoryInfo stMemoryInfo;
      BVCE_P_FirmwareMemorySettings stFirmwareMemorySettings;
      unsigned uiPictureMemcIndex = 0;

      if ( NULL != hVce->stOpenSettings.hBox )
      {
         BBOX_Config *pstBoxConfig = (BBOX_Config *) BKNI_Malloc( sizeof( BBOX_Config ) );
         if ( NULL != pstBoxConfig )
         {
            BBOX_GetConfig( hVce->stOpenSettings.hBox, pstBoxConfig );
            uiPictureMemcIndex = pstBoxConfig->stVce.stInstance[hVce->stOpenSettings.uiInstance].uiMemcIndex;
            BKNI_Free( pstBoxConfig );
         }
      }
      BCHP_GetMemoryInfo( hVce->handles.hChp, &stMemoryInfo );
      BVCE_S_PopulateFirmwareMemorySettings( &stMemoryInfo, &stFirmwareMemorySettings, uiPictureMemcIndex );

      hVce->fw.stCommand.type.stInit.StripeWidth = stFirmwareMemorySettings.StripeWidth;
      hVce->fw.stCommand.type.stInit.X = stFirmwareMemorySettings.X;
      hVce->fw.stCommand.type.stInit.Y = stFirmwareMemorySettings.Y;
      hVce->fw.stCommand.type.stInit.WordSize = stFirmwareMemorySettings.WordSize;
      hVce->fw.stCommand.type.stInit.BankType = stFirmwareMemorySettings.BankType;
      hVce->fw.stCommand.type.stInit.PageSize = stFirmwareMemorySettings.PageSize;
      hVce->fw.stCommand.type.stInit.Grouping = stFirmwareMemorySettings.Grouping;
   }

#if (BSTD_CPU_ENDIAN==BSTD_ENDIAN_LITTLE)
      /* Set the CABAC CDB to little endian mode so we can dump the ES using the MIPs */
   hVce->fw.stCommand.type.stInit.DeviceEndianess = 1;
#else
   hVce->fw.stCommand.type.stInit.DeviceEndianess = 0;
#endif

#if ( BVCE_P_CORE_MAJOR <= 2 )
   hVce->fw.stCommand.type.stInit.DeviceSG_CABACCmdBuffPtr = (uint32_t) BVCE_P_Buffer_GetDeviceOffset_isrsafe( hVce->hCabacCmdBuffer ); /* Only used on 32-bit VICEv2, so safe to cast to 32-bit */
   hVce->fw.stCommand.type.stInit.DeviceSG_CABACCmdBuffSize = BVCE_P_Buffer_GetSize( hVce->hCabacCmdBuffer );
#endif

   hVce->fw.stCommand.type.stInit.VerificationModeFlags = 0;
   hVce->fw.stCommand.type.stInit.VerificationModeFlags |= hVce->stOpenSettings.bVerificationMode ? INIT_CMD_VERIFICATION_MODE_MASK : 0;
   hVce->fw.stCommand.type.stInit.VerificationModeFlags |= hVce->stOpenSettings.bA2NPictureDrop  ? INIT_CMD_A2N_MASK : 0;

#if (BVCE_P_VDC_MANAGE_VIP)
   hVce->fw.stCommand.type.stInit.NewBvnMailbox = 1;
#endif

   rc = BVCE_S_SendCommand(
            hVce,
            &hVce->fw.stCommand,
            &hVce->fw.stResponse
            );

   if ( BERR_SUCCESS != rc )
   {
      return BERR_TRACE( rc );
   }

   hVce->fw.debug[0].uiBufferInfoBaseAddress = hVce->fw.dccm.uiRegisterBaseAddress[0] + (uint32_t) hVce->fw.stResponse.type.stInit.pPicArcDebugBuff;
   hVce->fw.debug[1].uiBufferInfoBaseAddress = hVce->fw.dccm.uiRegisterBaseAddress[1] + (uint32_t) hVce->fw.stResponse.type.stInit.pMbArcDebugBuff;
   hVce->fw.dccm.uiChannelErrorStatusBaseAddress = hVce->fw.dccm.uiRegisterBaseAddress[0] + (uint32_t) hVce->fw.stResponse.type.stInit.pStatusBase;
   hVce->fw.dccm.uiWatchdogErrorCodeBaseAddress[0] = hVce->fw.dccm.uiRegisterBaseAddress[0] + (uint32_t) hVce->fw.stResponse.type.stInit.pPicArcWdogErrCodeBase;
   hVce->fw.dccm.uiWatchdogErrorCodeBaseAddress[1] = hVce->fw.dccm.uiRegisterBaseAddress[1] + (uint32_t) hVce->fw.stResponse.type.stInit.pMbArcWdogErrCodeBase;

   hVce->stVersionInfo.uiFirmwareVersion = hVce->fw.stResponse.type.stInit.Version;
   hVce->stVersionInfo.uiFirmwareApiVersion = hVce->fw.stResponse.type.stInit.FwApiVersion;
   hVce->stVersionInfo.uiBvn2ViceApiVersion = hVce->fw.stResponse.type.stInit.BvnApiVersion;

   BDBG_WRN(("FW v%d.%d.%d.%d (0x%08x) [API v%d.%d.%d.%d (0x%08x)][%d]",
            ( hVce->fw.stResponse.type.stInit.Version >> 24 ) & 0xFF,
            ( hVce->fw.stResponse.type.stInit.Version >> 16 ) & 0xFF,
            ( hVce->fw.stResponse.type.stInit.Version >>  8 ) & 0xFF,
            ( hVce->fw.stResponse.type.stInit.Version >>  0 ) & 0xFF,
            hVce->fw.stResponse.type.stInit.Version,
            ( hVce->fw.stResponse.type.stInit.FwApiVersion >> 24 ) & 0xFF,
            ( hVce->fw.stResponse.type.stInit.FwApiVersion >> 16 ) & 0xFF,
            ( hVce->fw.stResponse.type.stInit.FwApiVersion >>  8 ) & 0xFF,
            ( hVce->fw.stResponse.type.stInit.FwApiVersion >>  0 ) & 0xFF,
            hVce->fw.stResponse.type.stInit.FwApiVersion,
            hVce->stOpenSettings.uiInstance
            ));

   if ( 0 != ( hVce->fw.stResponse.type.stInit.Version & 0x80000000 ) )
   {
      /* DEBUG Build */
      if ( 0 != hVce->fw.stResponse.type.stInit.pszVersionStr )
      {
         void* pCachedAddress;
         void* szVersionCached = NULL;

         pCachedAddress = BVCE_P_Buffer_LockAddress( hVce->fw.memory[0].hBuffer );
         if ( NULL != pCachedAddress )
         {
            szVersionCached = (void*) ((uint8_t*) pCachedAddress + (hVce->fw.stResponse.type.stInit.pszVersionStr - (uint32_t) BVCE_P_Buffer_GetDeviceOffset_isrsafe( hVce->fw.memory[0].hBuffer )));

            BVCE_P_Buffer_FlushCache_isrsafe(
               hVce->fw.memory[0].hBuffer,
               szVersionCached,
               512
               );

            if ( NULL != szVersionCached )
            {
               BDBG_WRN(("%s", (char *)szVersionCached));
            }

            BVCE_P_Buffer_UnlockAddress( hVce->fw.memory[0].hBuffer );
         }
      }
   }

   if ( hVce->fw.stResponse.type.stInit.FwApiVersion != VICE_API_VERSION )
   {
      BDBG_ERR(("FW API Version Mismatch: Expected 0x%08x but got 0x%08x", VICE_API_VERSION, hVce->fw.stResponse.type.stInit.FwApiVersion ));
   }

   if ( BAVC_VICE_BVN2VICE_MAJORREVISION_ID != BAVC_GET_FIELD_DATA(hVce->fw.stResponse.type.stInit.BvnApiVersion, VICE_BVN2VICE, MAJORREVISION ) )
   {
      BDBG_ERR(("BVN API MAJOR Version Mismatch: Expected %d.%d but got %d.%d",
         BAVC_VICE_BVN2VICE_MAJORREVISION_ID,
         BAVC_VICE_BVN2VICE_MINORREVISION_ID,
         BAVC_GET_FIELD_DATA(hVce->fw.stResponse.type.stInit.BvnApiVersion, VICE_BVN2VICE, MAJORREVISION ),
         BAVC_GET_FIELD_DATA(hVce->fw.stResponse.type.stInit.BvnApiVersion, VICE_BVN2VICE, MINORREVISION )
         ));

      return BERR_TRACE( BERR_UNKNOWN );
   }
   else if ( BAVC_VICE_BVN2VICE_MINORREVISION_ID != BAVC_GET_FIELD_DATA(hVce->fw.stResponse.type.stInit.BvnApiVersion, VICE_BVN2VICE, MINORREVISION ) )
   {
      BDBG_WRN(("BVN API MINOR Version Mismatch: Expected %d.%d but got %d.%d",
         BAVC_VICE_BVN2VICE_MAJORREVISION_ID,
         BAVC_VICE_BVN2VICE_MINORREVISION_ID,
         BAVC_GET_FIELD_DATA(hVce->fw.stResponse.type.stInit.BvnApiVersion, VICE_BVN2VICE, MAJORREVISION ),
         BAVC_GET_FIELD_DATA(hVce->fw.stResponse.type.stInit.BvnApiVersion, VICE_BVN2VICE, MINORREVISION )
         ));
   }

   return BERR_TRACE( rc );
}

static
BERR_Code
BVCE_S_SendCommand_OpenChannel(
         BVCE_Handle hVce,
         BVCE_Channel_Handle hVceCh
         )
{
   BERR_Code rc;

   BKNI_Memset(
            &hVce->fw.stCommand,
            0,
            sizeof( BVCE_P_Command )
            );

   BKNI_Memset(
            &hVce->fw.stResponse,
            0,
            sizeof( BVCE_P_Response )
            );

   hVce->fw.stCommand.type.stOpenChannel.Command = VICE_CMD_OPEN_CHANNEL;
   hVce->fw.stCommand.type.stOpenChannel.uiChannel_id = hVceCh->stOpenSettings.uiInstance;
   BVCE_P_SET_32BIT_HI_LO_FROM_64( hVce->fw.stCommand.type.stOpenChannel.pNonSecureBufferBase, BVCE_P_Buffer_GetDeviceOffset_isrsafe(hVceCh->memory[BVCE_P_HeapId_ePicture].hBuffer) );
   hVce->fw.stCommand.type.stOpenChannel.uiNonSecureBufferSize = BVCE_P_Buffer_GetSize(hVceCh->memory[BVCE_P_HeapId_ePicture].hBuffer);
   BVCE_P_SET_32BIT_HI_LO_FROM_64( hVce->fw.stCommand.type.stOpenChannel.pSecureBufferBase, BVCE_P_Buffer_GetDeviceOffset_isrsafe(hVceCh->memory[BVCE_P_HeapId_eSecure].hBuffer) );
   hVce->fw.stCommand.type.stOpenChannel.uiSecureBufferSize = BVCE_P_Buffer_GetSize(hVceCh->memory[BVCE_P_HeapId_eSecure].hBuffer);
   if (  BVCE_MultiChannelMode_eCustom == hVceCh->stOpenSettings.eMultiChannelMode )
   {
      if ( hVceCh->stOpenSettings.uiMaxNumChannels > hVceCh->hVce->stPlatformConfig.stBox.uiTotalChannels )
      {
         BDBG_WRN(("The specificed max number of channels (%u) is higher than what is supported by this device(%u), setting max equal to what is supported",
            hVceCh->stOpenSettings.uiMaxNumChannels,
            hVceCh->hVce->stPlatformConfig.stBox.uiTotalChannels
            ));

         hVce->fw.stCommand.type.stOpenChannel.uiMaxNumChannels = hVceCh->hVce->stPlatformConfig.stBox.uiTotalChannels;
      }
      else
      {
         hVce->fw.stCommand.type.stOpenChannel.uiMaxNumChannels = hVceCh->stOpenSettings.uiMaxNumChannels;
      }
   }
   else
   {
      switch ( hVceCh->stOpenSettings.eMultiChannelMode )
      {
         case BVCE_MultiChannelMode_eSingle:
            hVce->fw.stCommand.type.stOpenChannel.uiMaxNumChannels = 1;
            break;

         default:
            hVce->fw.stCommand.type.stOpenChannel.uiMaxNumChannels = hVceCh->hVce->stPlatformConfig.stBox.uiTotalChannels;
            break;
      }
   }

   rc = BVCE_S_SendCommand(
            hVce,
            &hVce->fw.stCommand,
            &hVce->fw.stResponse
            );

   if ( BERR_SUCCESS != rc )
   {
      return BERR_TRACE( rc );
   }

   /* Handle User Data Queue Info Base */
   hVceCh->userdata.dccm.uiUserDataQueueInfoAddress = hVce->fw.dccm.uiRegisterBaseAddress[0] + (uint32_t) hVce->fw.stResponse.type.stOpenChannel.pUserDataQInfoBase;

#if (BVCE_P_VDC_MANAGE_VIP)
   /* Handle Picture Release Queue Info Base */
   hVceCh->picture.dccm.auiPictureReleaseQueueInfoAddress[BVCE_P_PictureBufferType_eLuma] = hVce->fw.dccm.uiRegisterBaseAddress[0] + (uint32_t) hVce->fw.stResponse.type.stOpenChannel.pLumaBufferReleaseQInfoBase;
   hVceCh->picture.dccm.auiPictureReleaseQueueInfoAddress[BVCE_P_PictureBufferType_eChroma] = hVce->fw.dccm.uiRegisterBaseAddress[0] + (uint32_t) hVce->fw.stResponse.type.stOpenChannel.pChromaBufferReleaseQInfoBase;
   hVceCh->picture.dccm.auiPictureReleaseQueueInfoAddress[BVCE_P_PictureBufferType_e1VLuma] = hVce->fw.dccm.uiRegisterBaseAddress[0] + (uint32_t) hVce->fw.stResponse.type.stOpenChannel.p1VLumaBufferReleaseQInfoBase;
   hVceCh->picture.dccm.auiPictureReleaseQueueInfoAddress[BVCE_P_PictureBufferType_e2VLuma] = hVce->fw.dccm.uiRegisterBaseAddress[0] + (uint32_t) hVce->fw.stResponse.type.stOpenChannel.p2VLumaBufferReleaseQInfoBase;
   hVceCh->picture.dccm.auiPictureReleaseQueueInfoAddress[BVCE_P_PictureBufferType_eShiftedChroma] = hVce->fw.dccm.uiRegisterBaseAddress[0] + (uint32_t) hVce->fw.stResponse.type.stOpenChannel.pShiftedChromaBufferReleaseQInfoBase;
#endif

   return BERR_TRACE( rc );
}

#define BVCE_P_VIDEOCOMPRESSIONPROFILE_UNSUPPORTED 0xFFFFFFFF

static
uint32_t
BVCE_S_ProfileH264LUT(
      BAVC_VideoCompressionProfile eProfile
      )
{
   switch ( eProfile )
   {
      case BAVC_VideoCompressionProfile_eMain:
         return ENCODING_AVC_PROFILE_MAIN;
      case BAVC_VideoCompressionProfile_eHigh:
         return ENCODING_AVC_PROFILE_HIGH;
      case BAVC_VideoCompressionProfile_eBaseline:
         return ENCODING_AVC_PROFILE_BASELINE;
      default:
         return BVCE_P_VIDEOCOMPRESSIONPROFILE_UNSUPPORTED;
   }
}

static
uint32_t
BVCE_S_ProfileMPEG2LUT(
      BAVC_VideoCompressionProfile eProfile
      )
{
   switch ( eProfile )
   {
      case BAVC_VideoCompressionProfile_eMain:
         return ENCODING_MPEG2_PROFILE_MAIN;
      default:
         return BVCE_P_VIDEOCOMPRESSIONPROFILE_UNSUPPORTED;
   }
}

static
uint32_t
BVCE_S_ProfileMPEG4LUT(
      BAVC_VideoCompressionProfile eProfile
      )
{
   switch ( eProfile )
   {
      case BAVC_VideoCompressionProfile_eSimple:
         return ENCODING_MPEG4_PROFILE_SIMPLE;
      case BAVC_VideoCompressionProfile_eAdvancedSimple:
         return ENCODING_MPEG4_PROFILE_ADVANCED_SIMPLE;
      default:
         return BVCE_P_VIDEOCOMPRESSIONPROFILE_UNSUPPORTED;
   }
}

static
uint32_t
BVCE_S_ProfileHEVCLUT(
      BAVC_VideoCompressionProfile eProfile
      )
{
   switch ( eProfile )
   {
      case BAVC_VideoCompressionProfile_eMain:
         return ENCODING_HEVC_PROFILE_TIER_MAIN;
      default:
         return BVCE_P_VIDEOCOMPRESSIONPROFILE_UNSUPPORTED;
   }
}

#define BVCE_P_VIDEOCOMPRESSIONLEVEL_UNSUPPORTED 0xFFFFFFFF

static
uint32_t
BVCE_S_LevelH264LUT(
      BAVC_VideoCompressionLevel eLevel
      )
{
   switch ( eLevel )
   {
      case BAVC_VideoCompressionLevel_e10:
         return ENCODING_AVC_LEVEL_10;
      case BAVC_VideoCompressionLevel_e1B:
         return ENCODING_AVC_LEVEL_10b;
      case BAVC_VideoCompressionLevel_e11:
         return ENCODING_AVC_LEVEL_11;
      case BAVC_VideoCompressionLevel_e12:
         return ENCODING_AVC_LEVEL_12;
      case BAVC_VideoCompressionLevel_e13:
         return ENCODING_AVC_LEVEL_13;
      case BAVC_VideoCompressionLevel_e20:
         return ENCODING_AVC_LEVEL_20;
      case BAVC_VideoCompressionLevel_e21:
         return ENCODING_AVC_LEVEL_21;
      case BAVC_VideoCompressionLevel_e22:
         return ENCODING_AVC_LEVEL_22;
      case BAVC_VideoCompressionLevel_e30:
         return ENCODING_AVC_LEVEL_30;
      case BAVC_VideoCompressionLevel_e31:
         return ENCODING_AVC_LEVEL_31;
      case BAVC_VideoCompressionLevel_e32:
         return ENCODING_AVC_LEVEL_32;
      case BAVC_VideoCompressionLevel_e40:
         return ENCODING_AVC_LEVEL_40;
      case BAVC_VideoCompressionLevel_e41:
         return ENCODING_AVC_LEVEL_41;
      case BAVC_VideoCompressionLevel_e42:
         return ENCODING_AVC_LEVEL_42;
      default:
         return BVCE_P_VIDEOCOMPRESSIONLEVEL_UNSUPPORTED;
   }
}

static
uint32_t
BVCE_S_LevelMPEG2LUT(
      BAVC_VideoCompressionLevel eLevel
      )
{
   switch ( eLevel )
   {
      case BAVC_VideoCompressionLevel_eLow:
         return ENCODING_MPEG2_LEVEL_LOW;
      case BAVC_VideoCompressionLevel_eMain:
         return ENCODING_MPEG2_LEVEL_MAIN;
      case BAVC_VideoCompressionLevel_eHigh:
         return ENCODING_MPEG2_LEVEL_HIGH;
      default:
         return BVCE_P_VIDEOCOMPRESSIONLEVEL_UNSUPPORTED;
   }
}

static
uint32_t
BVCE_S_LevelMPEG4LUT(
      BAVC_VideoCompressionLevel eLevel
      )
{
   switch ( eLevel )
   {
      case BAVC_VideoCompressionLevel_e10:
         return ENCODING_MPEG4_LEVEL_1;
      case BAVC_VideoCompressionLevel_e20:
         return ENCODING_MPEG4_LEVEL_2;
      case BAVC_VideoCompressionLevel_e30:
         return ENCODING_MPEG4_LEVEL_3;
      case BAVC_VideoCompressionLevel_e40:
         return ENCODING_MPEG4_LEVEL_4;
      case BAVC_VideoCompressionLevel_e50:
         return ENCODING_MPEG4_LEVEL_5;
      default:
         return BVCE_P_VIDEOCOMPRESSIONLEVEL_UNSUPPORTED;
   }
}

static
uint32_t
BVCE_S_LevelHEVCLUT(
      BAVC_VideoCompressionLevel eLevel
      )
{
   switch ( eLevel )
   {
      case BAVC_VideoCompressionLevel_e40:
         return ENCODING_HEVC_LEVEL_40;
      case BAVC_VideoCompressionLevel_e41:
         return ENCODING_HEVC_LEVEL_41;
      case BAVC_VideoCompressionLevel_e50:
         return ENCODING_HEVC_LEVEL_50;
      default:
         return BVCE_P_VIDEOCOMPRESSIONLEVEL_UNSUPPORTED;
   }
}

static
uint32_t
BVCE_S_LevelVP9LUT(
      BAVC_VideoCompressionLevel eLevel
      )
{
#ifdef ENCODING_VP9_LEVEL_21
   switch ( eLevel )
   {
      case BAVC_VideoCompressionLevel_e21:
         return ENCODING_VP9_LEVEL_21;
      case BAVC_VideoCompressionLevel_e30:
         return ENCODING_VP9_LEVEL_30;
      case BAVC_VideoCompressionLevel_e31:
         return ENCODING_VP9_LEVEL_30;
      case BAVC_VideoCompressionLevel_e40:
         return ENCODING_VP9_LEVEL_40;
      case BAVC_VideoCompressionLevel_e41:
         return ENCODING_VP9_LEVEL_41;
      default:
         return BVCE_P_VIDEOCOMPRESSIONLEVEL_UNSUPPORTED;
   }
#else
   BSTD_UNUSED(eLevel);
   return 0;
#endif
}

static const uint32_t BVCE_P_InputTypeLUT[BAVC_ScanType_eProgressive + 1] =
{
 ENCODER_INPUT_TYPE_INTERLACED, /* BAVC_ScanType_eInterlaced */
 ENCODER_INPUT_TYPE_PROGRESSIVE, /* BAVC_ScanType_eProgressive */
};

static const uint32_t BVCE_P_InputTypeReverseLUT[BAVC_ScanType_eProgressive + 1] =
{
   BAVC_ScanType_eProgressive, /* ENCODER_INPUT_TYPE_PROGRESSIVE */
   BAVC_ScanType_eInterlaced, /* ENCODER_INPUT_TYPE_INTERLACED */
};

static const uint32_t BVCE_P_EventMaskLUT[32] =
{
 ( 1 << VICE_EVENT_BVN_METADATA_CHANGE_BIT ), /* BVCE_CHANNEL_STATUS_FLAGS_EVENT_INPUT_CHANGE */
 ( 1 << VICE_EVENT_EOS_SENT_BIT ), /* BVCE_CHANNEL_STATUS_FLAGS_EVENT_EOS */
};

static const uint32_t BVCE_P_ErrorMaskLUT[32] =
{
 ( 1 << VICE_ERROR_INVALID_INPUT_DIMENSION_BIT ), /* BVCE_CHANNEL_STATUS_FLAGS_ERROR_INVALID_INPUT_DIMENSION */
 ( 1 << VICE_ERROR_USER_DATA_LATE_BIT ), /* BVCE_CHANNEL_STATUS_FLAGS_ERROR_USER_DATA_LATE */
 ( 1 << VICE_ERROR_USER_DATA_DUPLICATE_BIT ), /* BVCE_CHANNEL_STATUS_FLAGS_ERROR_USER_DATA_DUPLICATE */
 ( 1 << VICE_ERROR_FW_ADJUSTS_WRONG_FRAME_RATE ), /* BVCE_CHANNEL_STATUS_FLAGS_ERROR_ADJUSTS_WRONG_FRAME_RATE */
 ( 1 << VICE_ERROR_UNSUPPORTED_BVN_FRAME_RATE ), /* BVCE_CHANNEL_STATUS_FLAGS_ERROR_UNSUPPORTED_BVN_FRAME_RATE */
 ( 1 << VICE_ERROR_UNSUPPORTED_RESOLUTION ), /* BVCE_CHANNEL_STATUS_FLAGS_ERROR_UNSUPPORTED_RESOLUTION */
 ( 1 << VICE_ERROR_BVN_FRAMERATE_IS_SMALLER_THAN_THE_MINIMUM_ALLOWED ), /* BVCE_CHANNEL_STATUS_FLAGS_ERROR_MISMATCH_BVN_MIN_FRAME_RATE */
 ( 1 << VICE_ERROR_MISMATCH_BVN_PIC_RESOLUTION ), /* BVCE_CHANNEL_STATUS_FLAGS_ERROR_MISMATCH_BVN_PIC_RESOLUTION */
 ( 1 << VICE_ERROR_FW_INCREASED_BITRATE_ABOVE_MAX ), /* BVCE_CHANNEL_STATUS_FLAGS_ERROR_MAX_BITRATE_EXCEEDED */
 ( 1 << VICE_ERROR_BIN_BUFFER_IS_FULL ), /* BVCE_CHANNEL_STATUS_FLAGS_ERROR_BIN_BUFFER_FULL */
 ( 1 << VICE_ERROR_CDB_IS_FULL ), /* BVCE_CHANNEL_STATUS_FLAGS_ERROR_CDB_FULL */
 ( 1 << VICE_ERROR_PICARC_TO_CABAC_DINO_BUFFER_IS_FULL ), /* BVCE_CHANNEL_STATUS_FLAGS_ERROR_PICARC_TO_CABAC_DINO_BUFFER_FULL */
 ( 1 << VICE_ERROR_EBM_IS_FULL ), /* BVCE_CHANNEL_STATUS_FLAGS_ERROR_EBM_FULL */
 ( 1 << VICE_ERROR_NUM_SLICES_ADJUSTED_TO_MAX_ALLOWED ), /* BVCE_CHANNEL_STATUS_FLAGS_ERROR_MAX_NUM_SLICES_EXCEEDED */
 ( 1 << VICE_ERROR_NUM_ENTRIES_INTRACODED_ADJUSTED_TO_MAX_ALLOWED ), /* BVCE_CHANNEL_STATUS_FLAGS_ERROR_MAX_NUM_ENTRIES_INTRACODED_EXCEEDED */
 ( 1 << VICE_ERROR_IBBP_NOT_SUPPORTED_FOR_THIS_RESOLUTION ), /* BVCE_CHANNEL_STATUS_FLAGS_ERROR_IBBP_NOT_SUPPORTED_FOR_RESOLUTION */
 ( 1 << VICE_ERROR_MBARC_BOOT_FAILURE ), /* BVCE_CHANNEL_STATUS_FLAGS_ERROR_MBARC_BOOT_FAILURE */
 ( 1 << VICE_ERROR_MEASURED_ENCODER_DELAY_LONGER_THAN_ESTIMATED ), /* BVCE_CHANNEL_STATUS_FLAGS_ERROR_MEASURED_ENCODER_DELAY_TOO_LONG */
 ( 1 << VICE_ERROR_CRITICAL ), /* BVCE_CHANNEL_STATUS_FLAGS_ERROR_CRITICAL */
 ( 1 << VICE_ERROR_RES_AND_FRAMERATE_NOT_SUPPORTED_IN_3_CH_MODE ), /* BVCE_CHANNEL_STATUS_FLAGS_ERROR_UNSUPPORTED_DISPLAY_FMT_IN_3_CH_MODE */
 ( 1 << VICE_ERROR_RES_AND_FRAMERATE_NOT_SUPPORTED_IN_2_CH_MODE ), /* BVCE_CHANNEL_STATUS_FLAGS_ERROR_UNSUPPORTED_DISPLAY_FMT_IN_2_CH_MODE */
 ( 1 << VICE_ERROR_RESOLUTION_IS_TOO_HIGH_FOR_THIS_LEVEL ), /* BVCE_CHANNEL_STATUS_FLAGS_ERROR_MAX_RESOLUTION_FOR_LEVEL_EXCEEDED */
 ( 1 << VICE_ERROR_FW_INCREASED_BITRATE_TO_MINIMUM_SUPPORTED ), /* BVCE_CHANNEL_STATUS_FLAGS_ERROR_BITRATE_TOO_LOW */
 ( 1 << VICE_ERROR_UNSUPPORTED_FRAME_RATE_FOR_THIS_RESOLUTION_AND_GOP_STRUCTURE ), /* BVCE_CHANNEL_STATUS_FLAGS_ERROR_FRAMERATE_NOT_SUPPORTED_FOR_RESOLUTION_AND_GOP_STRUCTURE */
};

static const uint32_t BVCE_P_EventMaskReverseLUT[32] =
{
 0, /* Reserved */
 0, /* Reserved */
 0, /* Reserved */
 0, /* Reserved */
 0, /* Reserved */
 0, /* Reserved */
 0, /* Reserved */
 0, /* Reserved */
 0, /* Reserved */
 0, /* Reserved */
 0, /* Reserved */
 0, /* Reserved */
 0, /* Reserved */
 0, /* Reserved */
 0, /* Reserved */
 0, /* Reserved */
 0, /* Reserved */
 0, /* Reserved */
 0, /* Reserved */
 0, /* Reserved */
 0, /* Reserved */
 0, /* Reserved */
 0, /* Reserved */
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 BVCE_CHANNEL_STATUS_FLAGS_EVENT_EOS, /* VICE_EVENT_EOS_SENT_BIT */
 BVCE_CHANNEL_STATUS_FLAGS_EVENT_INPUT_CHANGE, /* VICE_EVENT_BVN_METADATA_CHANGE_BIT */
};

static const uint32_t BVCE_P_ErrorMaskReverseLUT[32] =
{
 BVCE_CHANNEL_STATUS_FLAGS_ERROR_INVALID_INPUT_DIMENSION, /* VICE_ERROR_INVALID_INPUT_DIMENSION_BIT */
 BVCE_CHANNEL_STATUS_FLAGS_ERROR_USER_DATA_LATE, /* VICE_ERROR_USER_DATA_LATE_BIT */
 BVCE_CHANNEL_STATUS_FLAGS_ERROR_USER_DATA_DUPLICATE, /* VICE_ERROR_USER_DATA_DUPLICATE_BIT */
 BVCE_CHANNEL_STATUS_FLAGS_ERROR_ADJUSTS_WRONG_FRAME_RATE, /* VICE_ERROR_FW_ADJUSTS_WRONG_FRAME_RATE */
 BVCE_CHANNEL_STATUS_FLAGS_ERROR_UNSUPPORTED_BVN_FRAME_RATE, /* VICE_ERROR_UNSUPPORTED_BVN_FRAME_RATE */
 BVCE_CHANNEL_STATUS_FLAGS_ERROR_UNSUPPORTED_RESOLUTION, /* VICE_ERROR_UNSUPPORTED_RESOLUTION */
 BVCE_CHANNEL_STATUS_FLAGS_ERROR_MISMATCH_BVN_MIN_FRAME_RATE, /* VICE_ERROR_BVN_FRAMERATE_IS_SMALLER_THAN_THE_MINIMUM_ALLOWED */
 BVCE_CHANNEL_STATUS_FLAGS_ERROR_MISMATCH_BVN_PIC_RESOLUTION, /* VICE_ERROR_MISMATCH_BVN_PIC_RESOLUTION */
 BVCE_CHANNEL_STATUS_FLAGS_ERROR_MAX_BITRATE_EXCEEDED, /* VICE_ERROR_FW_INCREASED_BITRATE_ABOVE_MAX */
 BVCE_CHANNEL_STATUS_FLAGS_ERROR_BIN_BUFFER_FULL, /* VICE_ERROR_BIN_BUFFER_IS_FULL */
 BVCE_CHANNEL_STATUS_FLAGS_ERROR_CDB_FULL, /* VICE_ERROR_CDB_IS_FULL */
 BVCE_CHANNEL_STATUS_FLAGS_ERROR_PICARC_TO_CABAC_DINO_BUFFER_FULL, /* VICE_ERROR_PICARC_TO_CABAC_DINO_BUFFER_IS_FULL */
 BVCE_CHANNEL_STATUS_FLAGS_ERROR_EBM_FULL, /* VICE_ERROR_EBM_IS_FULL */
 BVCE_CHANNEL_STATUS_FLAGS_ERROR_MAX_NUM_SLICES_EXCEEDED, /* VICE_ERROR_NUM_SLICES_ADJUSTED_TO_MAX_ALLOWED */
 BVCE_CHANNEL_STATUS_FLAGS_ERROR_MAX_NUM_ENTRIES_INTRACODED_EXCEEDED, /* VICE_ERROR_NUM_ENTRIES_INTRACODED_ADJUSTED_TO_MAX_ALLOWED */
 BVCE_CHANNEL_STATUS_FLAGS_ERROR_IBBP_NOT_SUPPORTED_FOR_RESOLUTION, /* VICE_ERROR_IBBP_NOT_SUPPORTED_FOR_THIS_RESOLUTION */
 BVCE_CHANNEL_STATUS_FLAGS_ERROR_MBARC_BOOT_FAILURE, /* VICE_ERROR_MBARC_BOOT_FAILURE */
 BVCE_CHANNEL_STATUS_FLAGS_ERROR_MEASURED_ENCODER_DELAY_TOO_LONG, /* VICE_ERROR_MEASURED_ENCODER_DELAY_LONGER_THAN_ESTIMATED */
 BVCE_CHANNEL_STATUS_FLAGS_ERROR_CRITICAL, /* VICE_ERROR_CRITICAL */
 BVCE_CHANNEL_STATUS_FLAGS_ERROR_UNSUPPORTED_DISPLAY_FMT_IN_3_CH_MODE, /* VICE_ERROR_RES_AND_FRAMERATE_NOT_SUPPORTED_IN_3_CH_MODE */
 BVCE_CHANNEL_STATUS_FLAGS_ERROR_UNSUPPORTED_DISPLAY_FMT_IN_2_CH_MODE, /* VICE_ERROR_RES_AND_FRAMERATE_NOT_SUPPORTED_IN_2_CH_MODE */
 BVCE_CHANNEL_STATUS_FLAGS_ERROR_MAX_RESOLUTION_FOR_LEVEL_EXCEEDED, /* VICE_ERROR_RESOLUTION_IS_TOO_HIGH_FOR_THIS_LEVEL */
 BVCE_CHANNEL_STATUS_FLAGS_ERROR_BITRATE_TOO_LOW, /* VICE_ERROR_FW_INCREASED_BITRATE_TO_MINIMUM_SUPPORTED */
 BVCE_CHANNEL_STATUS_FLAGS_ERROR_FRAMERATE_NOT_SUPPORTED_FOR_RESOLUTION_AND_GOP_STRUCTURE, /* VICE_ERROR_UNSUPPORTED_FRAME_RATE_FOR_THIS_RESOLUTION_AND_GOP_STRUCTURE */
 0,
 0,
 0,
 0,
 0,
 0,
 0, /* RESERVED */
 0, /* RESERVED */
};

static
BERR_Code
BVCE_S_GOPStructureLUT(
   const BVCE_Channel_StartEncodeSettings *pstStartEncodeSettings,
   const BVCE_GOPStructure *pstGOPStructure,
   unsigned *puiGOPStructure,
   unsigned *puiGOPLength
   )
{
   unsigned uiNumberOfPFrames = pstGOPStructure->uiNumberOfPFrames;
   unsigned uiNumberOfBFrames = pstGOPStructure->uiNumberOfBFrames;
   *puiGOPStructure = 0;
   *puiGOPLength = 0;

   /* SW7425-5073: If GOP Duration is specified, force IP mode by default.  Will switch to IBBP mode if B-pictures are specified */
   if ( 0 != pstGOPStructure->uiDuration )
   {
      uiNumberOfPFrames = 1;
   }

   if ( 0 == uiNumberOfPFrames )
   {
      *puiGOPStructure = ENCODING_GOP_STRUCT_I;
      *puiGOPLength = 1;
   }
   else
   {
      if ( 0xFFFFFFFF == uiNumberOfPFrames )
      {
         *puiGOPStructure = ENCODING_GOP_STRUCT_INFINITE_IP;
      }
      else
      {
         if ( 0 == uiNumberOfBFrames )
         {
            *puiGOPStructure = ENCODING_GOP_STRUCT_IP;
            *puiGOPLength = 1 + uiNumberOfPFrames;
         }
         else
         {
            if ( BAVC_ScanType_eProgressive == pstStartEncodeSettings->eInputType )
            {
               if ( uiNumberOfBFrames > BVCE_PLATFORM_P_MAX_B_PICTURES_PROGRESSIVE )
               {
                  uiNumberOfBFrames = BVCE_PLATFORM_P_MAX_B_PICTURES_PROGRESSIVE;
               }
            }
            else
            {
               if ( uiNumberOfBFrames > BVCE_PLATFORM_P_MAX_B_PICTURES_INTERLACED )
               {
                  uiNumberOfBFrames = BVCE_PLATFORM_P_MAX_B_PICTURES_INTERLACED;
               }
            }

            /* SWSTB-8115: Cap max GOP Structure for MPEG2 */
            if ( BAVC_VideoCompressionStd_eMPEG2 == pstStartEncodeSettings->stProtocolInfo.eProtocol )
            {
               if ( uiNumberOfBFrames > 2 )
               {
                  uiNumberOfBFrames = 2;
               }
            }

            switch ( uiNumberOfBFrames )
            {

               case 1:
                  *puiGOPStructure = ENCODING_GOP_STRUCT_IBP;
                  break;

               case 2:
#if (BVCE_P_CORE_MAJOR < 3)
                  *puiGOPStructure = ENCODING_GOP_STRUCT_IBBP;
#else
                  *puiGOPStructure = (BAVC_VideoCompressionStd_eMPEG2 == pstStartEncodeSettings->stProtocolInfo.eProtocol) ? ENCODING_GOP_STRUCT_IBBP : ENCODING_GOP_STRUCT_IBBP_B_REF;
#endif
                  break;

#if (BVCE_P_CORE_MAJOR >= 3)
               case 3:
                  *puiGOPStructure = ENCODING_GOP_STRUCT_IBBBP;
                  break;

               case 5:
                  *puiGOPStructure = ENCODING_GOP_STRUCT_IBBBBBP;
                  break;

               case 7:
                  *puiGOPStructure = ENCODING_GOP_STRUCT_IBBBBBBBP;
                  break;
#endif

               /* coverity[dead_error_begin] */
               default:
                  BDBG_ERR(("Invalid Parameter"));
                  return BERR_TRACE( BERR_INVALID_PARAMETER );
            }

            if ( true == pstGOPStructure->bAllowOpenGOP )
            {
               *puiGOPLength = (1 + uiNumberOfPFrames)*(1 + uiNumberOfBFrames);
            }
            else
            {
               *puiGOPLength = 1 + uiNumberOfPFrames*(1 + uiNumberOfBFrames);
            }
         }
      }

      /* SWSTB-2122: Set minimum GOP length */
      if ( 0 != pstGOPStructure->uiMinGOPLengthAfterSceneChange )
      {
          unsigned uiMinGopLengthAfterSceneChangeInFrames = ( ( ( ( *puiGOPLength & GOP_LENGTH_MASK ) * pstGOPStructure->uiMinGOPLengthAfterSceneChange ) + 99 ) / 100 );

          /* Check to see if the minimum GOP length is larger than supported */
          if ( 0 != ( ( uiMinGopLengthAfterSceneChangeInFrames << GOP_LENGTH_MINIMAL_SCENE_CHANGE_SHIFT ) & ~GOP_LENGTH_MINIMAL_SCENE_CHANGE_MASK ) )
          {
              *puiGOPLength |= GOP_LENGTH_MINIMAL_SCENE_CHANGE_MASK;
          }
          else
          {
              *puiGOPLength |= ( ( uiMinGopLengthAfterSceneChangeInFrames << GOP_LENGTH_MINIMAL_SCENE_CHANGE_SHIFT ) & GOP_LENGTH_MINIMAL_SCENE_CHANGE_MASK );
          }
      }

      *puiGOPStructure &= GOP_STRUCTURE_MASK;

      if ( true == pstGOPStructure->bAllowOpenGOP )
      {
         *puiGOPStructure |= ALLOW_OPEN_GOP_STRUCTURE_MASK;
      }

      /* SW7425-5073: GOP length can now optionally be determined by the FW by specifying the GOP duration */
      if ( 0 != pstGOPStructure->uiDuration )
      {
         if ( pstGOPStructure->uiDuration > ( 0xFFFF * 16 ) )
         {
            *puiGOPLength = ( 0xFFFF * 16 );
            BDBG_WRN(("Specified GOP Duration %d ms is too large, capping it to %d ms", pstGOPStructure->uiDuration, *puiGOPLength));
         }
         else
         {
            *puiGOPLength = pstGOPStructure->uiDuration;
         }

         if ( *puiGOPLength > GOP_LENGTH_MASK )
         {
            *puiGOPLength = GOP_LENGTH_MASK;
         }
         *puiGOPLength |= GOP_LENGTH_OR_DURATION_FLAG_MASK;

         /* SW7425-5972: GOP duration can be automatically ramped up by the FW to minimize delays at the client during certain streaming applications */
         if ( 0 != pstGOPStructure->uiDurationRampUpFactor )
         {
            *puiGOPLength |= ( ( ( pstGOPStructure->uiDurationRampUpFactor ) << GOP_LENGTH_RAMPING_N_SHIFT ) & GOP_LENGTH_RAMPING_N_MASK );
            /* Set M=N for the time being */
            *puiGOPLength |= ( ( ( pstGOPStructure->uiDurationRampUpFactor ) << GOP_LENGTH_RAMPING_M_SHIFT ) & GOP_LENGTH_RAMPING_M_MASK );
         }
      }
   }

   return BERR_TRACE( BERR_SUCCESS );
}

/* ensure that the GOP structure selected by encoder settings is supported by the encoder ... */
static
BERR_Code
BVCE_S_VerifyGopStructure(
         BVCE_Handle hVce
         )
{
   switch (hVce->fw.stCommand.type.stConfigChannel.GopStructure & GOP_STRUCTURE_MASK)
   {
      case ENCODING_GOP_STRUCT_I:
      case ENCODING_GOP_STRUCT_IP:
      case ENCODING_GOP_STRUCT_INFINITE_IP:
         /* supported in all encoders */
         break;

#if (BVCE_P_CORE_MAJOR < 3)
      case ENCODING_GOP_STRUCT_IBP:
         /* not supported yet ... */
         BDBG_ERR(("GOP Structure of IBP not supported"));
         return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif

      case ENCODING_GOP_STRUCT_IBBP:
#if (BVCE_P_CORE_MAJOR >= 3)
      case ENCODING_GOP_STRUCT_IBP:
      case ENCODING_GOP_STRUCT_IBBP_B_REF:
      case ENCODING_GOP_STRUCT_IBBBP:
      case ENCODING_GOP_STRUCT_IBBBBBP:
      case ENCODING_GOP_STRUCT_IBBBBBBBP:
#endif
         if ((ENCODING_STD_H264 != hVce->fw.stCommand.type.stConfigChannel.Protocol)
            && (ENCODING_STD_MPEG2 != hVce->fw.stCommand.type.stConfigChannel.Protocol)
            && (ENCODING_STD_HEVC != hVce->fw.stCommand.type.stConfigChannel.Protocol)
            && (ENCODING_STD_VP9 != hVce->fw.stCommand.type.stConfigChannel.Protocol)
            )
         {
            BDBG_ERR(("B-Pictures not supported"));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
         }

         /* SW7445-1527: Do not allow B-frames with AVC Baseline profile */
         if ( ( ENCODING_STD_H264 == hVce->fw.stCommand.type.stConfigChannel.Protocol )
              && ( ENCODING_AVC_PROFILE_BASELINE == hVce->fw.stCommand.type.stConfigChannel.Profile ) )
         {
            BDBG_ERR(("GOP Structure of IBBP not supported for H.264/AVC Baseline Profile"));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
         }

         /* else is supported ... */
         {
            unsigned uiMultiplier = 3;

            switch (hVce->fw.stCommand.type.stConfigChannel.GopStructure & GOP_STRUCTURE_MASK)
            {
               case ENCODING_GOP_STRUCT_IBP:
                  uiMultiplier = 2;
                  break;
               case ENCODING_GOP_STRUCT_IBBP:
                  uiMultiplier = 3;
                  break;
         #if (BVCE_P_CORE_MAJOR >= 3)
               case ENCODING_GOP_STRUCT_IBBP_B_REF:
                  uiMultiplier = 3;
                  break;

               case ENCODING_GOP_STRUCT_IBBBP:
                  uiMultiplier = 4;
                  break;

               case ENCODING_GOP_STRUCT_IBBBBBP:
                  uiMultiplier = 6;
                  break;

               case ENCODING_GOP_STRUCT_IBBBBBBBP:
                  uiMultiplier = 8;
                  break;
         #endif
            }

            /* check GOP length - must be 1 + 3*N (open gop) or (1+P)(1+2)*/
            if ( 0 == (hVce->fw.stCommand.type.stConfigChannel.GopLength & GOP_LENGTH_OR_DURATION_FLAG_MASK) )
            {
               if ( 0 == (hVce->fw.stCommand.type.stConfigChannel.GopStructure & ALLOW_OPEN_GOP_STRUCTURE_MASK) )
               {
                  if (((hVce->fw.stCommand.type.stConfigChannel.GopLength & GOP_LENGTH_MASK) - 1) % uiMultiplier)
                  {
                     BDBG_ERR(("GOP length (%d) invalid with B-Pictures Closed GOP Structure - must be 1 + 3*N", hVce->fw.stCommand.type.stConfigChannel.GopLength & GOP_LENGTH_MASK));
                     return BERR_TRACE(BERR_NOT_SUPPORTED);
                  }
               }
               else
               {
                  if (((hVce->fw.stCommand.type.stConfigChannel.GopLength & GOP_LENGTH_MASK) - uiMultiplier) % uiMultiplier)
                  {
                     BDBG_ERR(("GOP length (%d) invalid with B-Pictures Open GOP Structure - must be 3 + 3*N", hVce->fw.stCommand.type.stConfigChannel.GopLength & GOP_LENGTH_MASK));
                     return BERR_TRACE(BERR_NOT_SUPPORTED);
                  }
               }
            }
         }

#if (BVCE_P_CORE_MAJOR >= 3)
         /* Check to ensure IB5P and IB7P are only used with progressive */
         switch (hVce->fw.stCommand.type.stConfigChannel.GopStructure & GOP_STRUCTURE_MASK)
         {
            case ENCODING_GOP_STRUCT_IBBBBBP:
            case ENCODING_GOP_STRUCT_IBBBBBBBP:
               if ( ENCODER_INPUT_TYPE_INTERLACED == hVce->fw.stCommand.type.stConfigChannel.InputType )
               {
                  BDBG_ERR(("Interlaced transcode not allowed with IB5P/IB7P GOP Structure"));
                  return BERR_TRACE(BERR_NOT_SUPPORTED);
               }
         }

         /* Check to ensure B_REF is not used with MPEG2 */
         if ( ENCODING_STD_MPEG2 == hVce->fw.stCommand.type.stConfigChannel.Protocol )
         {
            switch (hVce->fw.stCommand.type.stConfigChannel.GopStructure & GOP_STRUCTURE_MASK)
            {
               case ENCODING_GOP_STRUCT_I:
               case ENCODING_GOP_STRUCT_IP:
               case ENCODING_GOP_STRUCT_INFINITE_IP:
               case ENCODING_GOP_STRUCT_IBP:
               case ENCODING_GOP_STRUCT_IBBP:
                  break;

               default:
                  BDBG_ERR(("MPEG2 transcode not allowed with requested GOP Structure"));
                  return BERR_TRACE(BERR_NOT_SUPPORTED);
            }
         }
#endif

         break;

      default:
         BDBG_ERR(("Unsupported GOP Structure %d", hVce->fw.stCommand.type.stConfigChannel.GopStructure & GOP_STRUCTURE_MASK));
         return BERR_TRACE(BERR_NOT_SUPPORTED);
   }

   return BERR_SUCCESS;
}

static
unsigned
BVCE_S_EncodeModeLUT(
   const BVCE_Channel_StartEncodeSettings *pstStartEncodeSettings
   )
{
   if ( true == pstStartEncodeSettings->bNonRealTimeEncodeMode )
   {
      return ENCODER_MODE_AFAP;
   }
   else if ( true == pstStartEncodeSettings->bPipelineLowDelayMode )
   {
      return ENCODER_MODE_LOW_DELAY;
   }
   else
   {
      return ENCODER_MODE_HIGH_DELAY;
   }
}

#define BVCE_P_DIMENSION_ALIGNMENT 16
#define BVCE_P_ALIGN_DIMENSION(_x) ( ( ( ( _x ) + ( ( BVCE_P_DIMENSION_ALIGNMENT ) - 1 ) ) / ( BVCE_P_DIMENSION_ALIGNMENT ) ) * ( BVCE_P_DIMENSION_ALIGNMENT ) )
static
void
BVCE_S_GetMaxDimension(
   const BVCE_Channel_StartEncodeSettings *pstStartEncodeSettings,
   unsigned *puiWidth,
   unsigned *puiHeight
   )
{
   /* By default use the progressive max dimensions */
   *puiWidth = BVCE_P_ALIGN_DIMENSION(pstStartEncodeSettings->stBounds.stDimensions.stMax.uiWidth);
   *puiHeight = BVCE_P_ALIGN_DIMENSION(pstStartEncodeSettings->stBounds.stDimensions.stMax.uiHeight);

   /* Use the interlaced max dimensions if they are set and is smaller than the progressive max dimensions */
   if ( BAVC_ScanType_eInterlaced == pstStartEncodeSettings->eInputType )
   {
      unsigned uiPixelCount = (*puiWidth) * (*puiHeight);
      unsigned uiInterlacedPixelCount = BVCE_P_ALIGN_DIMENSION(pstStartEncodeSettings->stBounds.stDimensions.stMaxInterlaced.uiWidth) * BVCE_P_ALIGN_DIMENSION(pstStartEncodeSettings->stBounds.stDimensions.stMaxInterlaced.uiHeight);

      if ( ( 0 != uiInterlacedPixelCount ) && ( uiInterlacedPixelCount < uiPixelCount ) )
      {
         *puiWidth = BVCE_P_ALIGN_DIMENSION(pstStartEncodeSettings->stBounds.stDimensions.stMaxInterlaced.uiWidth);
         *puiHeight = BVCE_P_ALIGN_DIMENSION(pstStartEncodeSettings->stBounds.stDimensions.stMaxInterlaced.uiHeight);
      }

      /* 1280x720i interlaced is not valid, so we limit it to 720x576i */
      if ( ( *puiWidth == BVCE_P_ALIGN_DIMENSION(1280) )
           && ( *puiHeight == BVCE_P_ALIGN_DIMENSION(720) ) )
      {
         *puiWidth = BVCE_P_ALIGN_DIMENSION(720);
         *puiHeight = BVCE_P_ALIGN_DIMENSION(576);
      }
   }
}

const BAVC_FrameRateCode BVCE_P_FW2PI_FrameRateLUT[] =
{
   BAVC_FrameRateCode_eUnknown, /* ENCODING_FRAME_RATE_CODE_UNKNOWN */
   BAVC_FrameRateCode_e23_976, /* ENCODING_FRAME_RATE_CODE_2397 */
   BAVC_FrameRateCode_e24, /* ENCODING_FRAME_RATE_CODE_2400 */
   BAVC_FrameRateCode_e25, /* ENCODING_FRAME_RATE_CODE_2500 */
   BAVC_FrameRateCode_e29_97, /* ENCODING_FRAME_RATE_CODE_2997 */
   BAVC_FrameRateCode_e30, /* ENCODING_FRAME_RATE_CODE_3000 */
   BAVC_FrameRateCode_e50, /* ENCODING_FRAME_RATE_CODE_5000 */
   BAVC_FrameRateCode_e59_94, /* ENCODING_FRAME_RATE_CODE_5994 */
   BAVC_FrameRateCode_e60, /* ENCODING_FRAME_RATE_CODE_6000 */
   BAVC_FrameRateCode_e14_985, /* ENCODING_FRAME_RATE_CODE_1498 */
   BAVC_FrameRateCode_e7_493, /* ENCODING_FRAME_RATE_CODE_0749 */
   BAVC_FrameRateCode_e10, /* ENCODING_FRAME_RATE_CODE_1000 */
   BAVC_FrameRateCode_e15, /* ENCODING_FRAME_RATE_CODE_1500 */
   BAVC_FrameRateCode_e20, /* ENCODING_FRAME_RATE_CODE_2000 */
   BAVC_FrameRateCode_e19_98, /* ENCODING_FRAME_RATE_CODE_1998 */
   BAVC_FrameRateCode_e12_5, /* ENCODING_FRAME_RATE_CODE_1250 */
   BAVC_FrameRateCode_e7_5, /* ENCODING_FRAME_RATE_CODE_0750 */
   BAVC_FrameRateCode_e12, /* ENCODING_FRAME_RATE_CODE_1200 */
   BAVC_FrameRateCode_e11_988, /* ENCODING_FRAME_RATE_CODE_1198 */
   BAVC_FrameRateCode_e9_99 /* ENCODING_FRAME_RATE_CODE_0999 */
};

const unsigned BAVC_FrameRateCodeNumEntries = sizeof(BVCE_P_FW2PI_FrameRateLUT)/sizeof(BAVC_FrameRateCode);
/* NOTE: This must be the last value in the LUT above
   (this is verified/asserted at run time)
   This is necessary since we don't know the largest value from the FW
   (there is not MAX entry for the ENCODING_FRAME_RATE) so this
   prevents an array overrun of the above LUT */
const unsigned BAVC_FWMaxFrameRateCode = ENCODING_FRAME_RATE_CODE_0999;

static const uint32_t BVCE_P_PI2FW_FrameRateLUT[BAVC_FrameRateCode_eMax] =
{
   ENCODING_FRAME_RATE_CODE_UNKNOWN, /* BAVC_FrameRateCode_eUnknown */
   ENCODING_FRAME_RATE_CODE_2397, /* BAVC_FrameRateCode_e23_976 */
   ENCODING_FRAME_RATE_CODE_2400, /* BAVC_FrameRateCode_e24 */
   ENCODING_FRAME_RATE_CODE_2500, /* BAVC_FrameRateCode_e25 */
   ENCODING_FRAME_RATE_CODE_2997, /* BAVC_FrameRateCode_e29_97 */
   ENCODING_FRAME_RATE_CODE_3000, /* BAVC_FrameRateCode_e30 */
   ENCODING_FRAME_RATE_CODE_5000, /* BAVC_FrameRateCode_e50 */
   ENCODING_FRAME_RATE_CODE_5994, /* BAVC_FrameRateCode_e59_94 */
   ENCODING_FRAME_RATE_CODE_6000, /* BAVC_FrameRateCode_e60 */
   ENCODING_FRAME_RATE_CODE_1498, /* BAVC_FrameRateCode_e14_985 */
   ENCODING_FRAME_RATE_CODE_0749, /* BAVC_FrameRateCode_e7_493 */
   ENCODING_FRAME_RATE_CODE_1000, /* BAVC_FrameRateCode_e10 */
   ENCODING_FRAME_RATE_CODE_1500, /* BAVC_FrameRateCode_e15 */
   ENCODING_FRAME_RATE_CODE_2000, /* BAVC_FrameRateCode_e20 */
   ENCODING_FRAME_RATE_CODE_1250, /* BAVC_FrameRateCode_e12_5 */
   ENCODING_FRAME_RATE_CODE_UNKNOWN, /* BAVC_FrameRateCode_e100 */
   ENCODING_FRAME_RATE_CODE_UNKNOWN, /* BAVC_FrameRateCode_e119_88 */
   ENCODING_FRAME_RATE_CODE_UNKNOWN, /* BAVC_FrameRateCode_e120 */
   ENCODING_FRAME_RATE_CODE_1998, /* BAVC_FrameRateCode_e19_98 */
   ENCODING_FRAME_RATE_CODE_0750, /* BAVC_FrameRateCode_e7_5 */
   ENCODING_FRAME_RATE_CODE_1200, /* BAVC_FrameRateCode_e12 */
   ENCODING_FRAME_RATE_CODE_1198, /* BAVC_FrameRateCode_e11_988 */
   ENCODING_FRAME_RATE_CODE_0999 /* BAVC_FrameRateCode_e9_99 */
};

static
BERR_Code
BVCE_S_SendCommand_ConfigChannel(
         BVCE_Handle hVce,
         BVCE_Channel_Handle hVceCh,
         BVCE_P_SendCommand_ConfigChannel_Settings *pstConfigChannelSettings
         )
{
   BERR_Code rc;
   const BVCE_Channel_EncodeSettings *pstEncodeSettings;

   if ( ( NULL != pstConfigChannelSettings ) && ( true == pstConfigChannelSettings->bOnInputChange ) )
   {
      pstEncodeSettings = &hVceCh->stPendingEncodeSettings;
   }
   else
   {
      pstEncodeSettings = &hVceCh->stEncodeSettings;
   }

#if BVCE_P_TEST_MODE
   if ( NULL == hVceCh->hConfigLog )
   {
      char fname[256];
      static unsigned uiConfigLogInstance[BVCE_PLATFORM_P_NUM_ENCODE_INSTANCES][BVCE_PLATFORM_P_NUM_ENCODE_CHANNELS];

      sprintf(fname, "BVCE_CONFIG_%02d_%02d_%03d.csv", hVceCh->hVce->stOpenSettings.uiInstance, hVceCh->stOpenSettings.uiInstance, uiConfigLogInstance[hVceCh->hVce->stOpenSettings.uiInstance][hVceCh->stOpenSettings.uiInstance]);
      uiConfigLogInstance[hVceCh->hVce->stOpenSettings.uiInstance][hVceCh->stOpenSettings.uiInstance]++;

      if ( false == BVCE_Debug_P_OpenLog( fname, &hVceCh->hConfigLog ) )
      {
         BDBG_ERR(("Error Creating BVCE Channel Config Dump File (%s)", fname));
      }
      else
      {
         BVCE_Debug_P_WriteLog_isr( hVceCh->hConfigLog, "count,nrt,pipeline low delay,adaptive low delay,protocol,profile,level,input type,stc index,output index,rate buffer delay,num parallel encodes,force entropy coding,single ref P,required patches only,segment rc,segment duration,segment upper tolerance,segment upper slope factor,segment lower tolerance,segment lower slope factor" );
         BDBG_CWARNING( sizeof( BVCE_Channel_StartEncodeSettings ) <= 144 );

         BVCE_Debug_P_WriteLog_isr( hVceCh->hConfigLog, ",on input change,new rap,fast channel change");
         BDBG_CWARNING( sizeof( BVCE_P_SendCommand_ConfigChannel_Settings ) == 3 );

         BVCE_Debug_P_WriteLog_isr( hVceCh->hConfigLog, ",frame rate,frame rate mode,sparse frame rate mode,bitrate,bitrate target,A2P delay,gop restart,min gop length after restart,duration,duration ramp up,p frames,b frames,open gop,itfp,num slices");
         BDBG_CWARNING( sizeof( BVCE_Channel_EncodeSettings ) == 60 );

         BVCE_Debug_P_WriteLog_isr( hVceCh->hConfigLog, "\n" );
      }
   }

   if ( NULL != hVceCh->hConfigLog )
   {
      /* Start Encode Settings */
      BVCE_Debug_P_WriteLog_isr( hVceCh->hConfigLog, "%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u",
         ( NULL == hVceCh->stStartEncodeSettings.hOutputHandle ) ? (unsigned)-1 : hVceCh->stStartEncodeSettings.hOutputHandle->uiDescriptorCount,
         hVceCh->stStartEncodeSettings.bNonRealTimeEncodeMode,
         hVceCh->stStartEncodeSettings.bPipelineLowDelayMode,
         hVceCh->stStartEncodeSettings.bAdaptiveLowDelayMode,
         hVceCh->stStartEncodeSettings.stProtocolInfo.eProtocol,
         hVceCh->stStartEncodeSettings.stProtocolInfo.eProfile,
         hVceCh->stStartEncodeSettings.stProtocolInfo.eLevel,
         hVceCh->stStartEncodeSettings.eInputType,
         hVceCh->stStartEncodeSettings.uiStcIndex,
         ( NULL == hVceCh->stStartEncodeSettings.hOutputHandle ) ? (unsigned)-1 : hVceCh->stStartEncodeSettings.hOutputHandle->stOpenSettings.uiInstance,
         hVceCh->stStartEncodeSettings.uiRateBufferDelay,
         hVceCh->stStartEncodeSettings.uiNumParallelNRTEncodes,
         hVceCh->stStartEncodeSettings.eForceEntropyCoding,
         hVceCh->stStartEncodeSettings.stMemoryBandwidthSaving.bSingleRefP,
         hVceCh->stStartEncodeSettings.stMemoryBandwidthSaving.bRequiredPatchesOnly,
         hVceCh->stStartEncodeSettings.stRateControl.stSegmentMode.bEnable,
         hVceCh->stStartEncodeSettings.stRateControl.stSegmentMode.uiDuration,
         hVceCh->stStartEncodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stUpper.uiTolerance,
         hVceCh->stStartEncodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stUpper.uiSlopeFactor,
         hVceCh->stStartEncodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stLower.uiTolerance,
         hVceCh->stStartEncodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stLower.uiSlopeFactor
      );

      if ( NULL != pstConfigChannelSettings )
      {
         BVCE_Debug_P_WriteLog_isr( hVceCh->hConfigLog, ",%u,%u,%u",
            pstConfigChannelSettings->bOnInputChange,
            pstConfigChannelSettings->bBeginNewRAP,
            pstConfigChannelSettings->bFastChannelChange
            );
      }
      else
      {
         BVCE_Debug_P_WriteLog_isr( hVceCh->hConfigLog, ",%u,%u,%u",
            0,
            0,
            0
            );
      }

      /* Encode Settings */
      BVCE_Debug_P_WriteLog_isr( hVceCh->hConfigLog, ",%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u",
         pstEncodeSettings->stFrameRate.eFrameRate,
         pstEncodeSettings->stFrameRate.bVariableFrameRateMode,
         pstEncodeSettings->stFrameRate.bSparseFrameRateMode,
         pstEncodeSettings->stBitRate.uiMax,
         pstEncodeSettings->stBitRate.uiTarget,
         pstEncodeSettings->uiA2PDelay,
         pstEncodeSettings->stGOPStructure.bAllowNewGOPOnSceneChange,
         pstEncodeSettings->stGOPStructure.uiMinGOPLengthAfterSceneChange,
         pstEncodeSettings->stGOPStructure.uiDuration,
         pstEncodeSettings->stGOPStructure.uiDurationRampUpFactor,
         pstEncodeSettings->stGOPStructure.uiNumberOfPFrames,
         pstEncodeSettings->stGOPStructure.uiNumberOfBFrames,
         pstEncodeSettings->stGOPStructure.bAllowOpenGOP,
         pstEncodeSettings->bITFPEnable,
         pstEncodeSettings->uiNumSlicesPerPic
      );

      BVCE_Debug_P_WriteLog_isr( hVceCh->hConfigLog, "\n" );
   }
#endif

   if ( NULL != hVceCh->hVce->stDebugFifo.hDebugFifo )
   {
      BVCE_P_DebugFifo_Entry *pstEntry;
      BDBG_Fifo_Token stToken;

      pstEntry = (BVCE_P_DebugFifo_Entry *) BDBG_Fifo_GetBuffer( hVceCh->hVce->stDebugFifo.hDebugFifo, &stToken );
      if ( NULL != pstEntry )
      {
         pstEntry->stMetadata.eType = BVCE_DebugFifo_EntryType_eConfig;
         pstEntry->stMetadata.uiInstance = hVceCh->hVce->stOpenSettings.uiInstance;
         pstEntry->stMetadata.uiChannel = hVceCh->stOpenSettings.uiInstance;
         pstEntry->stMetadata.uiTimestamp = 0;
         ( NULL != hVce->hTimer ) ? BTMR_ReadTimer( hVce->hTimer, &pstEntry->stMetadata.uiTimestamp ) : 0;
         pstEntry->data.stConfig.stStartEncodeSettings = hVceCh->stStartEncodeSettings;
         pstEntry->data.stConfig.stEncodeSettings = *pstEncodeSettings;
         if ( NULL != pstConfigChannelSettings )
         {
            pstEntry->data.stConfig.stSettingsModifiers = *pstConfigChannelSettings;
         }
         else
         {
            BKNI_Memset( &pstEntry->data.stConfig.stSettingsModifiers, 0, sizeof( BVCE_P_SendCommand_ConfigChannel_Settings ) );
         }
         BDBG_Fifo_CommitBuffer( &stToken );
      }
   }

   BKNI_Memset(
            &hVce->fw.stCommand,
            0,
            sizeof( BVCE_P_Command )
            );

   BKNI_Memset(
            &hVce->fw.stResponse,
            0,
            sizeof( BVCE_P_Response )
            );

   hVce->fw.stCommand.type.stConfigChannel.Command = VICE_CMD_CONFIG_CHANNEL;
   hVce->fw.stCommand.type.stConfigChannel.uiChannel_id = hVceCh->stOpenSettings.uiInstance;
   if ( hVceCh->stStartEncodeSettings.stProtocolInfo.eProtocol > BAVC_VideoCompressionStd_eMax )
   {
      BDBG_ERR(("Unrecognized video compression protocol (%d)", hVceCh->stStartEncodeSettings.stProtocolInfo.eProtocol));
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }
   hVce->fw.stCommand.type.stConfigChannel.Protocol = BVCE_P_ProtocolLUT(hVceCh->stStartEncodeSettings.stProtocolInfo.eProtocol);
   switch ( hVce->fw.stCommand.type.stConfigChannel.Protocol )
   {
      case ENCODING_STD_H264:
         hVce->fw.stCommand.type.stConfigChannel.Profile = BVCE_S_ProfileH264LUT(hVceCh->stStartEncodeSettings.stProtocolInfo.eProfile);
         hVce->fw.stCommand.type.stConfigChannel.Level = BVCE_S_LevelH264LUT(hVceCh->stStartEncodeSettings.stProtocolInfo.eLevel);
         break;

      case ENCODING_STD_MPEG2:
         hVce->fw.stCommand.type.stConfigChannel.Profile = BVCE_S_ProfileMPEG2LUT(hVceCh->stStartEncodeSettings.stProtocolInfo.eProfile);
         hVce->fw.stCommand.type.stConfigChannel.Level = BVCE_S_LevelMPEG2LUT(hVceCh->stStartEncodeSettings.stProtocolInfo.eLevel);
         break;

      case ENCODING_STD_MPEG4:
         hVce->fw.stCommand.type.stConfigChannel.Profile = BVCE_S_ProfileMPEG4LUT(hVceCh->stStartEncodeSettings.stProtocolInfo.eProfile);
         hVce->fw.stCommand.type.stConfigChannel.Level = BVCE_S_LevelMPEG4LUT(hVceCh->stStartEncodeSettings.stProtocolInfo.eLevel);
         break;

      case ENCODING_STD_VP8:
         hVce->fw.stCommand.type.stConfigChannel.Profile = ENCODING_VP8_PROFILE_STANDARD_LF;
         hVce->fw.stCommand.type.stConfigChannel.Level = 0;
         break;

      case ENCODING_STD_HEVC:
         hVce->fw.stCommand.type.stConfigChannel.Profile = BVCE_S_ProfileHEVCLUT(hVceCh->stStartEncodeSettings.stProtocolInfo.eProfile);
         hVce->fw.stCommand.type.stConfigChannel.Level = BVCE_S_LevelHEVCLUT(hVceCh->stStartEncodeSettings.stProtocolInfo.eLevel);
         break;

      case ENCODING_STD_VP9:
         hVce->fw.stCommand.type.stConfigChannel.Profile = ENCODING_VP9_PROFILE;
         hVce->fw.stCommand.type.stConfigChannel.Level = BVCE_S_LevelVP9LUT(hVceCh->stStartEncodeSettings.stProtocolInfo.eLevel);
         break;

      case BVCE_P_VIDEOCOMPRESSIONSTD_UNSUPPORTED:
      default:
         BDBG_ERR(("Unsupported video compression protocol (%d)", hVceCh->stStartEncodeSettings.stProtocolInfo.eProtocol));
         return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   if ( BVCE_P_VIDEOCOMPRESSIONPROFILE_UNSUPPORTED == hVce->fw.stCommand.type.stConfigChannel.Profile )
   {
      BDBG_ERR(("Unsupported video compression profile (%d)", hVceCh->stStartEncodeSettings.stProtocolInfo.eProfile));
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   if ( BVCE_P_VIDEOCOMPRESSIONLEVEL_UNSUPPORTED == hVce->fw.stCommand.type.stConfigChannel.Level )
   {
      BDBG_ERR(("Unsupported video compression level (%d)", hVceCh->stStartEncodeSettings.stProtocolInfo.eLevel));
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   hVce->fw.stCommand.type.stConfigChannel.FrameRateCode = BVCE_P_PI2FW_FrameRateLUT[pstEncodeSettings->stFrameRate.eFrameRate];
   if ( 0 == hVce->fw.stCommand.type.stConfigChannel.FrameRateCode )
   {
      BDBG_ERR(("Unsupported frame rate (%d)", pstEncodeSettings->stFrameRate.eFrameRate));
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   hVce->fw.stCommand.type.stConfigChannel.Flags |= (( true == pstEncodeSettings->stFrameRate.bVariableFrameRateMode ) << CONFIG_FLAG_RATE_MODE_POS); /* Variable Rate */

   if ( true == pstEncodeSettings->stFrameRate.bSparseFrameRateMode )
   {
      if ( BAVC_ScanType_eInterlaced == hVceCh->stStartEncodeSettings.eInputType )
      {
         BDBG_ERR(("Interlaced transcode not allowed with Sparse Frame Rate Mode"));
         return BERR_TRACE( BERR_INVALID_PARAMETER );
      }
      hVce->fw.stCommand.type.stConfigChannel.Flags |= 1 << CONFIG_FLAG_ENABLE_SPARSE_FRAME_RATE_MODE;
   }

   hVce->fw.stCommand.type.stConfigChannel.MaxBitrate = pstEncodeSettings->stBitRate.uiMax;

   /* SW7425-4489: Added VBR support */
   hVce->fw.stCommand.type.stConfigChannel.TargetBitrate = pstEncodeSettings->stBitRate.uiTarget;

   if ( hVce->fw.stCommand.type.stConfigChannel.TargetBitrate > hVce->fw.stCommand.type.stConfigChannel.MaxBitrate )
   {
      BDBG_WRN(("The specificed Target bitrate (%u) is higher than Max bitrate (%u), setting Target equal to Max",
         hVce->fw.stCommand.type.stConfigChannel.TargetBitrate,
         hVce->fw.stCommand.type.stConfigChannel.MaxBitrate
         ));

      hVce->fw.stCommand.type.stConfigChannel.TargetBitrate = pstEncodeSettings->stBitRate.uiMax;
   }

   /* SW7445-2303: Force CAVLC mode if requested */
   if ( BVCE_EntropyCoding_eCAVLC == hVceCh->stStartEncodeSettings.eForceEntropyCoding )
   {
      hVce->fw.stCommand.type.stConfigChannel.Flags |= (1 << CONFIG_FLAG_FORCE_CAVLC_POS);
   }

   if ( ( NULL != pstConfigChannelSettings ) && ( true == pstConfigChannelSettings->bOnInputChange ) )
   {
      hVce->fw.stCommand.type.stConfigChannel.Flags |= (1 << CONFIG_FLAG_APPLY_BITRATE_BVN_CHANGE_POS);
   }

   /* Calculate GOP Structure */
   {
      BERR_Code rc;
      unsigned uiGOPStructure = 0;
      unsigned uiGOPLength = 0;

      rc = BVCE_S_GOPStructureLUT(
         &hVceCh->stStartEncodeSettings,
         &pstEncodeSettings->stGOPStructure,
         &uiGOPStructure,
         &uiGOPLength
         );

      if ( BERR_SUCCESS != rc )
      {
         return BERR_TRACE( rc );
      }

      hVce->fw.stCommand.type.stConfigChannel.GopStructure = uiGOPStructure;
      hVce->fw.stCommand.type.stConfigChannel.GopLength = uiGOPLength;

      if ( true == hVceCh->stStartEncodeSettings.bPipelineLowDelayMode )
      {
         switch ( uiGOPStructure )
         {
            case ENCODING_GOP_STRUCT_INFINITE_IP:
               break;

            default:
                  BDBG_ERR(("Infinite IP is the required GOP structure when pipeline low delay mode=%d",
                           hVceCh->stStartEncodeSettings.bPipelineLowDelayMode
                  ));
                  return BERR_TRACE( BERR_INVALID_PARAMETER );
               break;
         }
      }

      /* ensure the selected GOP structure is supported by the encoder ... */
      rc = BVCE_S_VerifyGopStructure(hVce);
      if (BERR_SUCCESS != rc)
         return rc;
   }

   /* Calculate Worst Case GOP Structure */
   {
      BERR_Code rc;
      unsigned uiGOPStructure = 0;
      unsigned uiGOPLength = 0;

      rc = BVCE_S_GOPStructureLUT(
         &hVceCh->stStartEncodeSettings,
         &hVceCh->stStartEncodeSettings.stBounds.stGOPStructure,
         &uiGOPStructure,
         &uiGOPLength
         );

      if ( BERR_SUCCESS != rc )
      {
         return BERR_TRACE( rc );
      }

      hVce->fw.stCommand.type.stConfigChannel.MaxAllowedGopStructure = uiGOPStructure;
   }

   /* Ensure mode is supported by multi channel config */
   switch ( hVceCh->stOpenSettings.eMultiChannelMode )
   {
      case BVCE_MultiChannelMode_eSingle:
         /* No limitations */
         break;
      case BVCE_MultiChannelMode_eMulti:
         if ( true == hVceCh->stStartEncodeSettings.bPipelineLowDelayMode )
         {
            BDBG_ERR(("BVCE_Channel_StartEncodeSettings.bPipelineLowDelayMode must be FALSE when BVCE_EncodeSettings.eMultiChannelMode=BVCE_MultiChannelMode_eMulti"));
            return BERR_TRACE( BERR_INVALID_PARAMETER );
         }
         break;
      case BVCE_MultiChannelMode_eMultiNRTOnly:
         if ( false == hVceCh->stStartEncodeSettings.bNonRealTimeEncodeMode )
         {
            BDBG_ERR(("BVCE_Channel_StartEncodeSettings.bNonRealTimeEncodeMode must be TRUE when BVCE_EncodeSettings.eMultiChannelMode=BVCE_MultiChannelMode_eMultiNRTOnly"));
            return BERR_TRACE( BERR_INVALID_PARAMETER );
         }
         break;
      case BVCE_MultiChannelMode_eCustom:
         if ( ( hVceCh->stOpenSettings.uiMaxNumChannels != 1 )
              && ( true == hVceCh->stStartEncodeSettings.bPipelineLowDelayMode ) )
         {
            BDBG_ERR(("BVCE_Channel_StartEncodeSettings.bPipelineLowDelayMode must be FALSE when hVceCh->stOpenSettings.uiMaxNumChannels != 1"));
            return BERR_TRACE( BERR_INVALID_PARAMETER );
         }
         break;
      default:
         BDBG_ERR(("Unrecognized eMultiChannelMode=%d", hVceCh->stOpenSettings.eMultiChannelMode));
         return BERR_TRACE( BERR_INVALID_PARAMETER );
         break;
   };

   hVce->fw.stCommand.type.stConfigChannel.Mode = BVCE_S_EncodeModeLUT( &hVceCh->stStartEncodeSettings );

   /* SW7445-2252: Prevent interlaced encoding if AVC baseline profile or level less than 2.1 */
   if ( ( BAVC_VideoCompressionStd_eH264 == hVceCh->stStartEncodeSettings.stProtocolInfo.eProtocol )
        && ( BAVC_ScanType_eInterlaced == hVceCh->stStartEncodeSettings.eInputType ) )
   {
      if ( BAVC_VideoCompressionProfile_eBaseline == hVceCh->stStartEncodeSettings.stProtocolInfo.eProfile )
      {
         BDBG_ERR(("Interlaced transcode not allowed with AVC Baseline profile"));
         return BERR_TRACE( BERR_INVALID_PARAMETER );
      }

      if ( hVceCh->stStartEncodeSettings.stProtocolInfo.eLevel < BAVC_VideoCompressionLevel_e21 )
      {
         BDBG_ERR(("Interlaced transcode not allowed with AVC Level less than 2.1"));
         return BERR_TRACE( BERR_INVALID_PARAMETER );
      }
   }

   /* SW7445-2298: Prevent interlaced encoding if VP8 is used */
   if ( ( BAVC_VideoCompressionStd_eVP8 == hVceCh->stStartEncodeSettings.stProtocolInfo.eProtocol )
        && ( BAVC_ScanType_eInterlaced == hVceCh->stStartEncodeSettings.eInputType ) )
   {
      BDBG_ERR(("Interlaced transcode not allowed with VP8"));
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   /* Prevent interlaced encoding if VP9 is used */
   if ( ( BAVC_VideoCompressionStd_eVP9 == hVceCh->stStartEncodeSettings.stProtocolInfo.eProtocol )
        && ( BAVC_ScanType_eInterlaced == hVceCh->stStartEncodeSettings.eInputType ) )
   {
      BDBG_ERR(("Interlaced transcode not allowed with VP9"));
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   if ( hVceCh->stStartEncodeSettings.eInputType > BAVC_ScanType_eProgressive )
   {
      BDBG_ERR(("Unrecognized input type (%d)", hVceCh->stStartEncodeSettings.eInputType));
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }
   hVce->fw.stCommand.type.stConfigChannel.InputType = BVCE_P_InputTypeLUT[ hVceCh->stStartEncodeSettings.eInputType ];

   /* Set Event Mask */
   {
      uint32_t uiMask;
      uint8_t uiIndex;

      BDBG_CASSERT( VICE_ERROR_INVALID_INPUT_DIMENSION_BIT     == 0 );
      BDBG_CASSERT( VICE_ERROR_USER_DATA_LATE_BIT              == 1 );
      BDBG_CASSERT( VICE_ERROR_USER_DATA_DUPLICATE_BIT         == 2 );
      BDBG_CASSERT( VICE_ERROR_FW_ADJUSTS_WRONG_FRAME_RATE     == 3 );
      BDBG_CASSERT( VICE_EVENT_BVN_METADATA_CHANGE_BIT         == 31 );

      hVce->fw.stCommand.type.stConfigChannel.EventMask = 0;

      uiMask = hVceCh->stCallbackSettings.stEvent.uiEventMask;

      uiIndex = 0;

      while ( uiMask )
      {
         if ( uiMask & 0x1 )
         {
            hVce->fw.stCommand.type.stConfigChannel.EventMask |= BVCE_P_EventMaskLUT[uiIndex];
         }

         uiMask >>= 1;
         uiIndex++;
      }

      uiMask = hVceCh->stCallbackSettings.stEvent.uiErrorMask;
      uiIndex = 0;

      while ( uiMask )
      {
         if ( uiMask & 0x1 )
         {
            hVce->fw.stCommand.type.stConfigChannel.EventMask |= BVCE_P_ErrorMaskLUT[uiIndex];
         }

         uiMask >>= 1;
         uiIndex++;
      }
   }

   /* TODO: Update cabac reset when implementing suspend/resume */
   /* Do not reset the cabac context pointers if the channel is already started because we are doing a
    * reconfig (e.g. changing bitrate)
    */
   if ( BVCE_P_Status_eStarted != hVceCh->eStatus )
   {
      hVce->fw.stCommand.type.stConfigChannel.Flags |= (1 << CONFIG_FLAG_RESET_CABAC_CONTEXT_POS);
      BVCE_Output_Reset(
               hVceCh->stStartEncodeSettings.hOutputHandle
               );
   }

   hVce->fw.stCommand.type.stConfigChannel.ContextID = hVceCh->stStartEncodeSettings.hOutputHandle->stOpenSettings.uiInstance;
   BVCE_P_SET_32BIT_HI_LO_FROM_64( hVce->fw.stCommand.type.stConfigChannel.ITBBufPtr, BVCE_P_Buffer_GetDeviceOffset_isrsafe( hVceCh->stStartEncodeSettings.hOutputHandle->hOutputBuffers->stITB.hBuffer ) );
   hVce->fw.stCommand.type.stConfigChannel.ITBBufSize = BVCE_P_Buffer_GetSize( hVceCh->stStartEncodeSettings.hOutputHandle->hOutputBuffers->stITB.hBuffer );
   BVCE_P_SET_32BIT_HI_LO_FROM_64( hVce->fw.stCommand.type.stConfigChannel.CDBBufPtr, BVCE_P_Buffer_GetDeviceOffset_isrsafe( hVceCh->stStartEncodeSettings.hOutputHandle->hOutputBuffers->stCDB.hBuffer ) );
   hVce->fw.stCommand.type.stConfigChannel.CDBBufSize = BVCE_P_Buffer_GetSize( hVceCh->stStartEncodeSettings.hOutputHandle->hOutputBuffers->stCDB.hBuffer );

   /* Set ITFP Mode */
   if ( false == pstEncodeSettings->bITFPEnable )
   {
      hVce->fw.stCommand.type.stConfigChannel.Flags |= (1 << CONFIG_FLAG_ITFP_DISABLED_POS);
   }

   /* Set STC Select */
   hVce->fw.stCommand.type.stConfigChannel.StcID = hVceCh->stStartEncodeSettings.uiStcIndex;

   /* Set end-to-end delay */
   hVce->fw.stCommand.type.stConfigChannel.A2PDelay = pstEncodeSettings->uiA2PDelay;

   hVce->fw.stCommand.type.stConfigChannel.FrameRateCodeLimit = ( ( (uint32_t) BVCE_P_PI2FW_FrameRateLUT[hVceCh->stStartEncodeSettings.stBounds.stFrameRate.eMin] ) << CONFIG_FRAME_RATE_CODE_LIMIT_MIN_SHIFT ) & CONFIG_FRAME_RATE_CODE_LIMIT_MIN_MASK;
   hVce->fw.stCommand.type.stConfigChannel.FrameRateCodeLimit |= ( ( (uint32_t) BVCE_P_PI2FW_FrameRateLUT[hVceCh->stStartEncodeSettings.stBounds.stFrameRate.eMax] ) << CONFIG_FRAME_RATE_CODE_LIMIT_MAX_SHIFT ) & CONFIG_FRAME_RATE_CODE_LIMIT_MAX_MASK;
   hVce->fw.stCommand.type.stConfigChannel.RateBufferDelayInMs = hVceCh->stStartEncodeSettings.uiRateBufferDelay;
   hVce->fw.stCommand.type.stConfigChannel.MinAllowedBvnFrameRateCode = hVceCh->stStartEncodeSettings.stBounds.stInputFrameRate.eMin;

   if ( BAVC_ScanType_eProgressive == hVceCh->stStartEncodeSettings.eInputType )
   {
      BVCE_Platform_P_OverrideChannelDimensionBounds(
         hVce->stOpenSettings.hBox,
         hVceCh->stStartEncodeSettings.eInputType,
         &hVceCh->stStartEncodeSettings.stBounds.stDimensions.stMax.uiWidth,
         &hVceCh->stStartEncodeSettings.stBounds.stDimensions.stMax.uiHeight
         );
   }
   else
   {
      BVCE_Platform_P_OverrideChannelDimensionBounds(
         hVce->stOpenSettings.hBox,
         hVceCh->stStartEncodeSettings.eInputType,
         &hVceCh->stStartEncodeSettings.stBounds.stDimensions.stMaxInterlaced.uiWidth,
         &hVceCh->stStartEncodeSettings.stBounds.stDimensions.stMaxInterlaced.uiHeight
         );
   }

   {
      unsigned uiWidth = 0, uiHeight = 0;

      BVCE_S_GetMaxDimension( &hVceCh->stStartEncodeSettings, &uiWidth, &uiHeight );

      hVce->fw.stCommand.type.stConfigChannel.MaxPictureSizeInPels = ( ( (uint32_t)uiWidth) << CONFIG_MAX_PICTURE_SIZE_IN_PELS_WIDTH_SHIFT ) & CONFIG_MAX_PICTURE_SIZE_IN_PELS_WIDTH_MASK;
      hVce->fw.stCommand.type.stConfigChannel.MaxPictureSizeInPels |= ( ( (uint32_t)uiHeight) << CONFIG_MAX_PICTURE_SIZE_IN_PELS_HEIGHT_SHIFT ) & CONFIG_MAX_PICTURE_SIZE_IN_PELS_HEIGHT_MASK;;
   }

   /* Set Begin New Rap */
   if ( ( NULL != pstConfigChannelSettings ) && ( true == pstConfigChannelSettings->bBeginNewRAP ) )
   {
      hVce->fw.stCommand.type.stConfigChannel.Flags |= (1 << CONFIG_FLAG_NEW_RAP_POS);
   }

   /* Set Fast Channel Change */
   if ( ( NULL != pstConfigChannelSettings ) && ( true == pstConfigChannelSettings->bFastChannelChange ) )
   {
      hVce->fw.stCommand.type.stConfigChannel.Flags |= (1 << CONFIG_FLAG_FAST_CHANNEL_CHANGE_POS);
   }

   /* Set Number of Slices */
   {
#ifdef BVCE_PLATFORM_P_SUPPORT_MULTIPLE_SLICES
      if ( pstEncodeSettings->uiNumSlicesPerPic > 16 )
      {
         BDBG_ERR(("BVCE_Channel_EncodeSettings.uiNumSlicesPerPic must be <= 16."));
         return BERR_TRACE( BERR_INVALID_PARAMETER );
      }
      else
      {
         hVce->fw.stCommand.type.stConfigChannel.NumSlicesPerPicture = pstEncodeSettings->uiNumSlicesPerPic;
      }
#else
      if ( 0 != pstEncodeSettings->uiNumSlicesPerPic )
      {
         BDBG_ERR(("Multiple Slices are not supported on this platform.  BVCE_Channel_EncodeSettings.uiNumSlicesPerPic must be 0."));
         return BERR_TRACE( BERR_INVALID_PARAMETER );
      }
#endif
   }

   /* Set number of parallel transcodes (FNRT) */
   if ( ( 2 < hVceCh->stStartEncodeSettings.uiNumParallelNRTEncodes ) && ( false == hVceCh->stStartEncodeSettings.bNonRealTimeEncodeMode ) )
   {
      BDBG_WRN(("Number of Parallel Encodes is greater than 2, but channel is not configured in NRT mode"));
   }

   hVce->fw.stCommand.type.stConfigChannel.NumParallelEncodes = hVceCh->stStartEncodeSettings.uiNumParallelNRTEncodes;

   /* SW7425-5066: Set GOP restart on scene change */
   if ( true == pstEncodeSettings->stGOPStructure.bAllowNewGOPOnSceneChange )
   {
      if ( BAVC_ScanType_eInterlaced == hVceCh->stStartEncodeSettings.eInputType )
      {
         BDBG_WRN(("New GOP on Scene Change is only supported for progressive content."));
      }
      hVce->fw.stCommand.type.stConfigChannel.Flags |= (1 << CONFIG_FLAG_NEW_GOP_ON_SCENE_CHANGE_POS);
   }

   /* SW7425-6057: Set bandwidth reduction flags */
   if ( true == hVceCh->stStartEncodeSettings.stMemoryBandwidthSaving.bSingleRefP )
   {
      hVce->fw.stCommand.type.stConfigChannel.Flags |= (1 << CONFIG_FLAG_FORCE_1_REF_P_PICTURE);
   }

   if ( true == hVceCh->stStartEncodeSettings.stMemoryBandwidthSaving.bRequiredPatchesOnly )
   {
      hVce->fw.stCommand.type.stConfigChannel.Flags |= (1 << CONFIG_FLAG_DISABLE_OPTIONAL_PATCHES);
   }

   /* SW7445-2896: Set Segment Mode RC flags */
   if ( true == hVceCh->stStartEncodeSettings.stRateControl.stSegmentMode.bEnable )
   {
      /* Validate parameters */
      if ( 0 == hVceCh->stStartEncodeSettings.stRateControl.stSegmentMode.uiDuration )
      {
         BDBG_ERR(("Segment Mode RC Duration of 0 is not allowed"));
         return BERR_TRACE( BERR_INVALID_PARAMETER );
      }

      if ( 100 < hVceCh->stStartEncodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stUpper.uiTolerance )
      {
         BDBG_ERR(("Segment Mode RC final upper target bitrate tolerance limit percentage must be <= 100%%"));
         return BERR_TRACE( BERR_INVALID_PARAMETER );
      }

      if ( 100 < hVceCh->stStartEncodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stUpper.uiSlopeFactor )
      {
         BDBG_ERR(("Segment Mode RC initial upper target bitrate tolerance offset percentage must be <= 100%%"));
         return BERR_TRACE( BERR_INVALID_PARAMETER );
      }

      if ( 100 < hVceCh->stStartEncodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stLower.uiTolerance)
      {
         BDBG_ERR(("Segment Mode RC final lower target bitrate tolerance limit percentage must be <= 100%%"));
         return BERR_TRACE( BERR_INVALID_PARAMETER );
      }

      if ( 100 > hVceCh->stStartEncodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stLower.uiSlopeFactor)
      {
         BDBG_ERR(("Segment Mode RC initial lower target bitrate tolerance offset percentage must be >= 100%%"));
         return BERR_TRACE( BERR_INVALID_PARAMETER );
      }

      hVce->fw.stCommand.type.stConfigChannel.Flags |= (1 << CONFIG_FLAG_ENABLE_RC_SEGMENT_MODE);

      /* Set upper limits */
      hVce->fw.stCommand.type.stConfigChannel.RCSegmentDurationInMsec = hVceCh->stStartEncodeSettings.stRateControl.stSegmentMode.uiDuration;
      hVce->fw.stCommand.type.stConfigChannel.RCSegmentModeParams |= ( ((uint32_t)hVceCh->stStartEncodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stUpper.uiTolerance) << CONFIG_UPPER_RC_SEGMENT_TOLERANCE_SHIFT ) & CONFIG_UPPER_RC_SEGMENT_TOLERANCE_SHIFT;
      hVce->fw.stCommand.type.stConfigChannel.RCSegmentModeParams |= ( ((uint32_t)hVceCh->stStartEncodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stUpper.uiSlopeFactor) << CONFIG_UPPER_LIMIT_SLOPE_FACTOR_SHIFT ) & CONFIG_UPPER_LIMIT_SLOPE_FACTOR_MASK;

      /* Set lower limits (defaults to no lower limit) */
      hVce->fw.stCommand.type.stConfigChannel.RCSegmentModeParams |= ( ((uint32_t)hVceCh->stStartEncodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stLower.uiTolerance) << CONFIG_LOWER_RC_SEGMENT_TOLERANCE_SHIFT ) & CONFIG_LOWER_RC_SEGMENT_TOLERANCE_MASK;
      hVce->fw.stCommand.type.stConfigChannel.RCSegmentModeParams |= ( ((uint32_t)hVceCh->stStartEncodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stLower.uiSlopeFactor) << CONFIG_LOWER_LIMIT_SLOPE_FACTOR_SHIFT ) & CONFIG_LOWER_LIMIT_SLOPE_FACTOR_MASK;
   }

   /* SW7445-3434: Disable HRD picture drops */
   if ( true == hVceCh->stStartEncodeSettings.stRateControl.stHrdMode.bDisableFrameDrop )
   {
      hVce->fw.stCommand.type.stConfigChannel.Flags |= (1 << CONFIG_FLAG_DISABLE_HRD_DROP_PICTURE);
   }

   rc = BVCE_S_SendCommand(
            hVce,
            &hVce->fw.stCommand,
            &hVce->fw.stResponse
            );

   if ( BERR_SUCCESS != rc )
   {
      return BERR_TRACE( rc );
   }

   return BERR_TRACE( rc );
}

static
BERR_Code
BVCE_S_SendCommand_StartChannel(
         BVCE_Handle hVce,
         BVCE_Channel_Handle hVceCh
         )
{
   BERR_Code rc;

   BKNI_Memset(
            &hVce->fw.stCommand,
            0,
            sizeof( BVCE_P_Command )
            );

   BKNI_Memset(
            &hVce->fw.stResponse,
            0,
            sizeof( BVCE_P_Response )
            );

   hVce->fw.stCommand.type.stStartChannel.Command = VICE_CMD_START_CHANNEL;
   hVce->fw.stCommand.type.stStartChannel.uiChannel_id = hVceCh->stOpenSettings.uiInstance;

   rc = BVCE_S_SendCommand(
            hVce,
            &hVce->fw.stCommand,
            &hVce->fw.stResponse
            );

   if ( BERR_SUCCESS != rc )
   {
      return BERR_TRACE( rc );
   }

   return BERR_TRACE( rc );
}

static
BERR_Code
BVCE_S_SendCommand_StopChannel(
         BVCE_Handle hVce,
         BVCE_Channel_Handle hVceCh
         )
{
   BERR_Code rc;

   BKNI_Memset(
            &hVce->fw.stCommand,
            0,
            sizeof( BVCE_P_Command )
            );

   BKNI_Memset(
            &hVce->fw.stResponse,
            0,
            sizeof( BVCE_P_Response )
            );

   hVce->fw.stCommand.type.stStopChannel.Command = VICE_CMD_STOP_CHANNEL;
   hVce->fw.stCommand.type.stStopChannel.uiChannel_id = hVceCh->stOpenSettings.uiInstance;

   switch ( hVceCh->stStopEncodeSettings.eStopMode )
   {
      case BVCE_Channel_StopMode_eImmediate:
      case BVCE_Channel_StopMode_eAbort:
         hVce->fw.stCommand.type.stStopChannel.Flags |= (1 << STOP_FLAG_FAST_CHANNEL_STOP_POS);
         break;

      case BVCE_Channel_StopMode_eNormal:
      default:
         /* Do Nothing */
         break;
   }

   rc = BVCE_S_SendCommand(
            hVce,
            &hVce->fw.stCommand,
            &hVce->fw.stResponse
            );

   if ( BERR_SUCCESS != rc )
   {
      return BERR_TRACE( rc );
   }

   return BERR_TRACE( rc );
}

static
BERR_Code
BVCE_S_SendCommand_GetChannelStatus(
         BVCE_Handle hVce,
         BVCE_Channel_Handle hVceCh
         )
{
   BERR_Code rc;

   BKNI_Memset(
            &hVce->fw.stCommand,
            0,
            sizeof( BVCE_P_Command )
            );

   BKNI_Memset(
            &hVce->fw.stResponse,
            0,
            sizeof( BVCE_P_Response )
            );

   hVce->fw.stCommand.type.stGetChannelStatus.Command = VICE_CMD_GET_CHANNEL_STATUS;
   hVce->fw.stCommand.type.stGetChannelStatus.uiChannel_id = hVceCh->stOpenSettings.uiInstance;

   rc = BVCE_S_SendCommand(
            hVce,
            &hVce->fw.stCommand,
            &hVce->fw.stResponse
            );

   if ( BERR_SUCCESS != rc )
   {
      return BERR_TRACE( rc );
   }

   return BERR_TRACE( rc );
}

static
BERR_Code
BVCE_S_SendCommand_CloseChannel(
         BVCE_Handle hVce,
         BVCE_Channel_Handle hVceCh
         )
{
   BERR_Code rc;

   BKNI_Memset(
            &hVce->fw.stCommand,
            0,
            sizeof( BVCE_P_Command )
            );

   BKNI_Memset(
            &hVce->fw.stResponse,
            0,
            sizeof( BVCE_P_Response )
            );

   hVce->fw.stCommand.type.stCloseChannel.Command = VICE_CMD_CLOSE_CHANNEL;
   hVce->fw.stCommand.type.stCloseChannel.uiChannel_id = hVceCh->stOpenSettings.uiInstance;

   rc = BVCE_S_SendCommand(
            hVce,
            &hVce->fw.stCommand,
            &hVce->fw.stResponse
            );

   if ( BERR_SUCCESS != rc )
   {
      return BERR_TRACE( rc );
   }

   return BERR_TRACE( rc );
}

static const uint32_t BVCE_P_ArcInstanceLUT[BVCE_ArcInstance_eMax] =
{
   PROC_ID_PICARC, /* BVCE_ArcInstance_ePicArc */
   PROC_ID_MBARC,  /* BVCE_ArcInstance_eMBArc */
};

static
BERR_Code
BVCE_S_SendCommand_DebugCommand(
         BVCE_Handle hVce,
         BVCE_ArcInstance eARCInstance,
         char *szCommand,
         unsigned uiLength
         )
{
   BERR_Code rc;

   BKNI_Memset(
            &hVce->fw.stCommand,
            0,
            sizeof( BVCE_P_Command )
            );

   BKNI_Memset(
            &hVce->fw.stResponse,
            0,
            sizeof( BVCE_P_Response )
            );

   hVce->fw.stCommand.type.stDebugChannel.Command = VICE_CMD_DEBUG_CHANNEL;
   hVce->fw.stCommand.type.stDebugChannel.uiChannel_id = 0;
   hVce->fw.stCommand.type.stDebugChannel.ProcID = BVCE_P_ArcInstanceLUT[eARCInstance];

   BKNI_Memset( hVce->fw.stCommand.type.stDebugChannel.aCommands, 0, COMMAND_BUFFER_SIZE_BYTES );
   BDBG_ASSERT( uiLength <= ( COMMAND_BUFFER_SIZE_BYTES - 1 ) );
   BKNI_Memcpy( hVce->fw.stCommand.type.stDebugChannel.aCommands, szCommand, uiLength );

   rc = BVCE_S_SendCommand(
            hVce,
            &hVce->fw.stCommand,
            &hVce->fw.stResponse
            );

   if ( BERR_SUCCESS != rc )
   {
      return BERR_TRACE( rc );
   }

   return BERR_TRACE( rc );
}

static const uint32_t BVCE_P_DebugBufferModeLUT[BVCE_Debug_BufferingMode_eMax] =
{
 VICE_DEBUG_BUFFER_MODE_OVERWRITE, /* BVCE_Debug_BufferingMode_eOverwriteOldData */
 VICE_DEBUG_BUFFER_MODE_STANDARD,   /* BVCE_Debug_BufferingMode_eDiscardNewData */
};

static
void
BVCE_S_FreeDebugLog(
         BVCE_Handle hVce
         )
{
   uint32_t i;

   if ( NULL != hVce->stDebugFifo.hDebugFifo )
   {
      BDBG_Fifo_Destroy( hVce->stDebugFifo.hDebugFifo );
      hVce->stDebugFifo.hDebugFifo = NULL;
   }
   if ( NULL != hVce->stDebugFifo.hBuffer )
   {
      if ( NULL != hVce->stDebugFifo.pBuffer )
      {
         BVCE_P_Buffer_UnlockAddress( hVce->stDebugFifo.hBuffer );
         hVce->stDebugFifo.pBuffer = NULL;
      }

      BVCE_P_Buffer_Free( hVce->stDebugFifo.hBuffer );
      hVce->stDebugFifo.hBuffer = NULL;
   }

   for ( i = 0; i < BVCE_PLATFORM_P_NUM_ARC_CORES; i++ )
   {
      if ( NULL != hVce->fw.debug[i].hVceTelem )
      {
         BVCE_Telem_Destroy( hVce->fw.debug[i].hVceTelem );
         hVce->fw.debug[i].hVceTelem = NULL;
      }

      BVCE_P_Buffer_Free( hVce->fw.debug[i].hBuffer );
      hVce->fw.debug[i].hBuffer = NULL;
   }
}

static
BERR_Code
BVCE_S_AllocateDebugLog(
   BVCE_Handle hVce
   )
{
   BERR_Code rc = BERR_SUCCESS;
   uint32_t i;
   BVCE_P_Buffer_AllocSettings stAllocSettings;

   BVCE_P_Buffer_GetDefaultAllocSettings( &stAllocSettings );
   stAllocSettings.uiAlignment = BVCE_P_DEFAULT_ALIGNMENT;
   stAllocSettings.bLockOffset = true;

   for ( i = 0; i < BVCE_PLATFORM_P_NUM_ARC_CORES; i++ )
   {
      if ( 0 != hVce->stOpenSettings.uiDebugLogBufferSize[i] )
      {
         stAllocSettings.uiSize = hVce->stOpenSettings.uiDebugLogBufferSize[i];

          rc = BVCE_P_Buffer_Alloc(
                  hVce->ahAllocator[BVCE_P_HeapId_eSystem],
                  &stAllocSettings,
                  &hVce->fw.debug[i].hBuffer
                  );

          if ( BERR_SUCCESS != rc )
          {
             BDBG_ERR(("Error allocating debug buffer[%d]",i));
             BVCE_S_FreeDebugLog( hVce );
             return BERR_TRACE( rc );
          }
      }

      {
         BVCE_Telem_Settings stTelemSettings;

         BVCE_Telem_GetDefaultCreateSettings( &stTelemSettings );

         rc = BVCE_Telem_Create( &hVce->fw.debug[i].hVceTelem, &stTelemSettings );
         if ( BERR_SUCCESS != rc )
         {
            BVCE_S_FreeDebugLog( hVce );
            return BERR_TRACE( rc );
         }
      }
   }

   /* Setup Debug FIFO */
   {
      BDBG_Fifo_CreateSettings stCreateSettings;
      BDBG_Fifo_GetDefaultCreateSettings( &stCreateSettings );

      hVce->stDebugFifo.uiElementSize = sizeof( BVCE_P_DebugFifo_Entry );
      hVce->stDebugFifo.uiBufferSize = BVCE_P_MAX_DEBUG_FIFO_COUNT * sizeof( BVCE_P_DebugFifo_Entry );

      stAllocSettings.uiSize = hVce->stDebugFifo.uiBufferSize;

      rc = BVCE_P_Buffer_Alloc(
         hVce->ahAllocator[BVCE_P_HeapId_eSystem],
         &stAllocSettings,
         &hVce->stDebugFifo.hBuffer
         );

      if ( BERR_SUCCESS != rc )
      {
         return BERR_TRACE( rc );
      }

      hVce->stDebugFifo.pBuffer = BVCE_P_Buffer_LockAddress( hVce->stDebugFifo.hBuffer );

      stCreateSettings.elementSize = hVce->stDebugFifo.uiElementSize;
      stCreateSettings.bufferSize = hVce->stDebugFifo.uiBufferSize;
      stCreateSettings.buffer = hVce->stDebugFifo.pBuffer;

      rc = BDBG_Fifo_Create(&hVce->stDebugFifo.hDebugFifo, &stCreateSettings);
      if ( BERR_SUCCESS != rc )
      {
         BDBG_WRN(("BDBG_Fifo is not supported"));
         hVce->stDebugFifo.hDebugFifo = NULL;
      }
   }

   return BERR_TRACE( BERR_SUCCESS );
}

static
BERR_Code
BVCE_S_SetupDebugLog(
         BVCE_Handle hVce
         )
{
   uint32_t i;

   for ( i = 0; i < BVCE_PLATFORM_P_NUM_ARC_CORES; i++ )
   {
      if ( ( 0 != hVce->stOpenSettings.uiDebugLogBufferSize[i] )
           && ( 0 != hVce->fw.debug[i].uiBufferInfoBaseAddress )
         )
      {
         ViceDebugBufferInfo_t stDebugBufferInfo;

         BKNI_Memset(
               &stDebugBufferInfo,
               0,
               sizeof( ViceDebugBufferInfo_t )
               );

         BVCE_P_ReadRegisters(
                  hVce,
                  hVce->fw.debug[i].uiBufferInfoBaseAddress,
                  (uint32_t*) (&stDebugBufferInfo),
                  sizeof(  ViceDebugBufferInfo_t )
                  );

         stDebugBufferInfo.uiMode = BVCE_P_DebugBufferModeLUT[hVce->stOpenSettings.eDebugLogBufferMode[i]];

         stDebugBufferInfo.uiSize = hVce->stOpenSettings.uiDebugLogBufferSize[i];

         BVCE_P_SET_32BIT_HI_LO_FROM_64( stDebugBufferInfo.uiPhysicalOffset, BVCE_P_Buffer_GetDeviceOffset_isrsafe( hVce->fw.debug[i].hBuffer ) );

         stDebugBufferInfo.uiReadOffset = 0;
         stDebugBufferInfo.uiWriteOffset = 0;

         BDBG_MSG(("Debug Log[%d] @"BDBG_UINT64_FMT" (%lu bytes)",
                  i,
                  BDBG_UINT64_ARG(BVCE_P_Buffer_GetDeviceOffset_isrsafe( hVce->fw.debug[i].hBuffer )),
                  (unsigned long) BVCE_P_Buffer_GetSize( hVce->fw.debug[i].hBuffer )
                 ));

         BVCE_P_WriteRegisters(
                  hVce,
                  hVce->fw.debug[i].uiBufferInfoBaseAddress,
                  (uint32_t*) (&stDebugBufferInfo),
                  sizeof(  ViceDebugBufferInfo_t )
                  );
      }
   }

   return BERR_TRACE( BERR_SUCCESS );
}

#if (BVCE_P_VDC_MANAGE_VIP)
static void
BVCE_S_ValidateAVCBufferCount(void)
{
   BAVC_VCE_BufferConfig stBufferConfig;

   BAVC_VCE_GetDefaultBufferConfig_isrsafe(false, &stBufferConfig );

   stBufferConfig.eScanType = BAVC_ScanType_eProgressive;
   if ( BAVC_VCE_GetRequiredBufferCount_isrsafe( &stBufferConfig, BAVC_VCE_BufferType_eOriginal ) < PREPROCESSOR_MAX_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_PROGRESSIVE )
   {
      BDBG_WRN(("BAVC_VCE_GetRequiredBufferCount mismatch (Progressive Original)"));
   }

   if ( BAVC_VCE_GetRequiredBufferCount_isrsafe( &stBufferConfig, BAVC_VCE_BufferType_eDecimated ) < PREPROCESSOR_MAX_NUMBER_OF_DECIMATED_PICTURE_BUFF_PROGRESSIVE )
   {
      BDBG_WRN(("BAVC_VCE_GetRequiredBufferCount mismatch (Progressive Decimated)"));
   }

   BAVC_VCE_GetDefaultBufferConfig_isrsafe(true, &stBufferConfig );
   stBufferConfig.eScanType = BAVC_ScanType_eInterlaced;
   if ( BAVC_VCE_GetRequiredBufferCount_isrsafe( &stBufferConfig, BAVC_VCE_BufferType_eOriginal ) < PREPROCESSOR_MAX_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_INTERLACE )
   {
      BDBG_WRN(("BAVC_VCE_GetRequiredBufferCount mismatch (Interlaced Original)"));
   }

   if ( BAVC_VCE_GetRequiredBufferCount_isrsafe( &stBufferConfig, BAVC_VCE_BufferType_eDecimated ) < PREPROCESSOR_MAX_NUMBER_OF_DECIMATED_PICTURE_BUFF_INTERLACE )
   {
      BDBG_WRN(("BAVC_VCE_GetRequiredBufferCount mismatch (Interlaced Decimated)"));
   }

   if ( BAVC_VCE_GetRequiredBufferCount_isrsafe( &stBufferConfig, BAVC_VCE_BufferType_eShiftedChroma ) < PREPROCESSOR_MAX_NUMBER_OF_ORIGINAL_PICTURE_BUFF_SHIFTED_CHROMA_INTERLACE )
   {
      BDBG_WRN(("BAVC_VCE_GetRequiredBufferCount mismatch (Interlaced Shifted Chroma)"));
   }
}
#endif

BERR_Code
BVCE_Open(
         BVCE_Handle *phVce, /* [out] VCE Device handle returned */
         BCHP_Handle hChp,   /* [in] Chip handle */
         BREG_Handle hReg,   /* [in] Register handle */
         BMMA_Heap_Handle hMem,   /* [in] System Memory handle */
         BINT_Handle hInt,   /* [in] Interrupt handle */
         const BVCE_OpenSettings *pstOpenSettings /* [in] VCE Device Open settings */
         )
{
   BERR_Code rc;
   BVCE_Handle hVce = NULL;

   BDBG_ENTER( BVCE_Open );

   /* Validate Parameters */
   BDBG_ASSERT( phVce );
   BDBG_ASSERT( hChp );
   BDBG_ASSERT( hReg );
   BDBG_ASSERT( hMem );
   BDBG_ASSERT( hInt );
   BDBG_ASSERT( pstOpenSettings );
   BDBG_ASSERT( BVCE_P_SIGNATURE_OPENSETTINGS == pstOpenSettings->uiSignature );

   /* Allocate Context */
   *phVce = NULL;

#if (BVCE_P_VDC_MANAGE_VIP)
   BVCE_S_ValidateAVCBufferCount();
#endif

   hVce = ( BVCE_Handle ) BKNI_Malloc( sizeof( BVCE_P_Context ) );
   if ( NULL == hVce )
   {
      return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
   }
   BKNI_Memset(
            hVce,
            0,
            sizeof( BVCE_P_Context )
            );

   BDBG_OBJECT_SET(hVce, BVCE_P_Context);

   hVce->stOpenSettings = *pstOpenSettings;

   /* TODO: Print Settings */
   BDBG_MODULE_MSG(BVCE_MEMORY, ("BVCE_Open():"));
#define BVCE_MEMCONFIG_FIELD(_field) BDBG_MODULE_MSG(BVCE_MEMORY, ("BVCE_OpenSettings.stMemoryConfig."#_field"=%lu bytes", (unsigned long) hVce->stOpenSettings.stMemoryConfig._field));
#include "bvce_memconfig.inc"

#if BVCE_P_ENABLE_UART
   BVCE_Platform_P_EnableUART(
      hReg
      );
#endif

   /* Setup Device Handles */
   hVce->handles.hChp = hChp;
   hVce->handles.hReg = hReg;
   hVce->handles.hInt = hInt;
   hVce->handles.hMem = hMem;

   rc = BVCE_Platform_P_GetConfig(
            pstOpenSettings->hBox,
            pstOpenSettings->uiInstance,
            &hVce->stPlatformConfig
            );
   if ( BERR_SUCCESS != rc )
   {
      BVCE_Close( hVce );
      return BERR_TRACE( rc );
   }

   /* Validate Instance Number */
   if ( pstOpenSettings->uiInstance >= hVce->stPlatformConfig.stBox.uiTotalInstances )
   {
      BDBG_ERR(("Invalid instance number (%d).  Max Instances Supported is %d", pstOpenSettings->uiInstance, hVce->stPlatformConfig.stBox.uiTotalInstances));
      BVCE_Close( hVce );
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   /* Setup Allocators */
   rc = BVCE_S_SetupAllocators( hVce );
   if ( BERR_SUCCESS != rc )
   {
      BVCE_Close( hVce );
      return BERR_TRACE( rc );
   }

   /* Create timer */
   rc = BVCE_S_CreateTimer(
            hVce
            );
   if (BERR_SUCCESS != rc)
   {
      BDBG_WRN(("Error creating timer"));
   }

   /* Check for Image Interface/Context */
#if !(BVCE_USE_CUSTOM_IMAGE)
   if (hVce->stOpenSettings.pImgInterface != &BVCE_IMAGE_Interface)
   {
      BDBG_WRN(("*******************"));
      BDBG_WRN(("You've linked in the default VCE BIMG interface and context."));
      BDBG_WRN(("However, you are providing your own version(s) to BVCE_Open()."));
      BDBG_WRN(("You should compile with BVCE_USE_CUSTOM_IMAGE=1 to prevent linkage"));
      BDBG_WRN(("of the default BIMG interface and context to reduce the binary size"));
      BDBG_WRN(("*******************"));
   }

#if !(BVCE_USE_CUSTOM_CONTEXT) && !(BVCE_USE_FILE_IMAGE)
   if (hVce->stOpenSettings.pImgContext != BVCE_IMAGE_Context)
   {
      BDBG_WRN(("*******************"));
      BDBG_WRN(("You've linked in the default VCE BIMG context."));
      BDBG_WRN(("However, you are providing your own version to BVCE_Open()."));
      BDBG_WRN(("You should compile with BVCE_USE_CUSTOM_CONTEXT=1 to prevent linkage"));
      BDBG_WRN(("of the default BIMG context to reduce the binary size"));
      BDBG_WRN(("*******************"));
   }
#endif
#endif

#if !(BVCE_USE_FILE_IMAGE)
   if ((hVce->stOpenSettings.pImgInterface == NULL) ||
       (hVce->stOpenSettings.pImgContext == NULL)) {
      BDBG_ERR(("*******************"));
      BDBG_ERR(("You've compiled with either BVCE_USE_CUSTOM_IMAGE=1 or BVCE_USE_CUSTOM_CONTEXT=1."));
      BDBG_ERR(("However, you have NOT provided your own version(s) of"));
      BDBG_ERR(("the BIMG interface and context to BVCE_Open()."));
      BDBG_ERR(("If you want to use the default BIMG, use BVCE_USE_CUSTOM_IMAGE=0 or BVCE_USE_CUSTOM_CONTEXT=0"));
      BDBG_ERR(("Otherwise, you MUST provide your own implementation of BIMG."));
      BDBG_ERR(("*******************"));
   }
#endif

   rc = BVCE_S_AllocateDeviceMemory(
            hVce
            );
   if ( BERR_SUCCESS != rc )
   {
      BVCE_Close( hVce );
      return BERR_TRACE( rc );
   }

   rc = BVCE_S_AllocateFirmwareMemory(
            hVce
            );
   if ( BERR_SUCCESS != rc )
   {
      BVCE_Close( hVce );
      return BERR_TRACE( rc );
   }

   rc = BVCE_S_AllocateDebugLog(
            hVce
            );
   if ( BERR_SUCCESS != rc )
   {
      BVCE_Close( hVce );
      return BERR_TRACE( rc );
   }
   BVCE_P_FUNCTION_TRACE_ENTER(0, hVce, 0);

   rc = BVCE_S_CreateEvents(
            hVce
            );
   if ( BERR_SUCCESS != rc )
   {
      BVCE_Close( hVce );
      return BERR_TRACE( rc );
   }

   rc = BVCE_Power_Resume( hVce );
   if ( BERR_SUCCESS != rc )
   {
      BVCE_Close( hVce );
      return BERR_TRACE( rc );
   }

   /* Assign handle */
   *phVce = hVce;

   hVce->bBooted = true;

   BVCE_P_FUNCTION_TRACE_LEAVE(0, hVce, 0);

   BDBG_LEAVE( BVCE_Open );
   return BERR_TRACE( rc );
}

BERR_Code
BVCE_Close(
         BVCE_Handle hVce
         )
{
   BERR_Code rc;

   BDBG_ENTER( BVCE_Close );

   BDBG_OBJECT_ASSERT(hVce, BVCE_P_Context);

   BVCE_P_FUNCTION_TRACE_ENTER(0, hVce, 0);

   BVCE_Power_P_AcquireResource(
         hVce,
         BVCE_Power_Type_ePower
         );

   rc = BVCE_Power_Standby(
      hVce
      );
   BERR_TRACE( rc );

   BVCE_S_DestroyEvents( hVce );

   BVCE_P_FUNCTION_TRACE_LEAVE(0, hVce, 0);
   BVCE_S_FreeDebugLog( hVce );

   BVCE_S_FreeFirmwareMemory( hVce );

   BVCE_S_FreeDeviceMemory( hVce );

   BVCE_S_DestroyTimer( hVce );

   BVCE_S_DestroyAllocators( hVce );

#if BVCE_P_DUMP_ARC_DEBUG_LOG
   {
      BVCE_ArcInstance eArcInstance;

      for ( eArcInstance = 0; eArcInstance < BVCE_ArcInstance_eMax; eArcInstance++ )
      {
         BVCE_Debug_P_CloseLog( hVce->hDebugLogDumpFile[eArcInstance] );
         hVce->hDebugLogDumpFile[eArcInstance] = NULL;
      }
   }
#endif

   BDBG_OBJECT_DESTROY(hVce, BVCE_P_Context);

   BKNI_Free(
            hVce
            );

   BDBG_LEAVE( BVCE_Close );
   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BVCE_GetTotalChannels(
         BVCE_Handle hVce,
         unsigned *puiTotalChannels
         )
{
   BDBG_ENTER( BVCE_GetTotalChannels );

   BDBG_OBJECT_ASSERT(hVce, BVCE_P_Context);
   BDBG_ASSERT( puiTotalChannels );
   BVCE_P_FUNCTION_TRACE_ENTER(0, hVce, 0);

   if ( NULL == puiTotalChannels )
   {
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   BVCE_Platform_P_GetTotalChannels( hVce->stOpenSettings.hBox, hVce->stOpenSettings.uiInstance, puiTotalChannels );

   BVCE_P_FUNCTION_TRACE_LEAVE(0, hVce, 0);
   BDBG_LEAVE( BVCE_GetTotalChannels );

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BVCE_GetVersionInfo(
   BVCE_Handle hVce,
   BVCE_VersionInfo *pstVersionInfo
   )
{
   BDBG_ENTER( BVCE_GetVersionInfo );

   BDBG_OBJECT_ASSERT(hVce, BVCE_P_Context);
   BDBG_ASSERT( pstVersionInfo );
   BVCE_P_FUNCTION_TRACE_ENTER(0, hVce, 0);

   if ( NULL == pstVersionInfo )
   {
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   *pstVersionInfo = hVce->stVersionInfo;

   BVCE_P_FUNCTION_TRACE_LEAVE(0, hVce, 0);
   BDBG_LEAVE( BVCE_GetVersionInfo );

   return BERR_TRACE( BERR_SUCCESS );
}

/*********/
/* Debug */
/*********/
static
BERR_Code
BVCE_Debug_S_ReadBuffer_impl(
         BVCE_Handle hVce,
         BVCE_ArcInstance eARCInstance,
         char *szBuffer,   /* [in] pointer to buffer where log is copied to */
         unsigned uiBufferSize,  /* [in] maximum number of bytes to copy to buffer */
         unsigned *puiBytesRead  /* [out] number of bytes copied from debug log */
         )
{
   ViceDebugBufferInfo_t stDebugBufferInfo;
   unsigned uiInputLengthRead = 0;

   BDBG_OBJECT_ASSERT(hVce, BVCE_P_Context);
   BDBG_ASSERT( szBuffer );
   BDBG_ASSERT( puiBytesRead );

   *puiBytesRead = 0;

   if ( ( 0 == hVce->fw.debug[eARCInstance].uiBufferInfoBaseAddress )
        || ( NULL == hVce->fw.debug[eARCInstance].hBuffer )
      )
   {
      BDBG_ERR(("Debug Log Doesn't Exist for ARC Instance %d", eARCInstance));
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   BVCE_P_ReadRegisters(
            hVce,
            hVce->fw.debug[eARCInstance].uiBufferInfoBaseAddress,
            (uint32_t*) (&stDebugBufferInfo),
            sizeof( ViceDebugBufferInfo_t )
            );

   if ( hVce->stOpenSettings.eDebugLogBufferMode[eARCInstance] == BVCE_Debug_BufferingMode_eOverwriteOldData )
   {
      /* TODO: We need to tell the FW to stop writing to the buffer while we are reading it out */
      BDBG_ERR(("BVCE_Debug_BufferingMode_eOverwriteOldData not supported"));
      return BERR_TRACE( BERR_NOT_SUPPORTED );
   }

   /* Parse the data */
   {
      void *pBufferCached;
      const uint8_t *pInputBuffer0;
      unsigned uiInputLength0;
      const uint8_t *pInputBuffer1 = NULL;
      unsigned uiInputLength1 = 0;

      /* Get cached address */
      pBufferCached = BVCE_P_Buffer_LockAddress( hVce->fw.debug[eARCInstance].hBuffer );

      if ( NULL == pBufferCached )
      {
         return BERR_TRACE( BERR_UNKNOWN );
      }

      pInputBuffer0 = ((uint8_t*)pBufferCached) + stDebugBufferInfo.uiReadOffset;

      if ( stDebugBufferInfo.uiReadOffset <= stDebugBufferInfo.uiWriteOffset )
      {
         uiInputLength0 = stDebugBufferInfo.uiWriteOffset - stDebugBufferInfo.uiReadOffset;
      }
      else
      {
         uiInputLength0 = stDebugBufferInfo.uiSize - stDebugBufferInfo.uiReadOffset;

         pInputBuffer1 = (uint8_t*)pBufferCached;
         uiInputLength1 = stDebugBufferInfo.uiWriteOffset;
      }

      BVCE_P_Buffer_FlushCache_isrsafe(
         hVce->fw.debug[eARCInstance].hBuffer,
         (void*) pInputBuffer0,
         uiInputLength0
         );

      if ( NULL != pInputBuffer1 )
      {
         BVCE_P_Buffer_FlushCache_isrsafe(
            hVce->fw.debug[eARCInstance].hBuffer,
            (void*) pInputBuffer1,
            uiInputLength1
            );
      }

      BVCE_Telem_Parse(
         hVce->fw.debug[eARCInstance].hVceTelem,
         pInputBuffer0,
         uiInputLength0,
         pInputBuffer1,
         uiInputLength1,
         &uiInputLengthRead,
         szBuffer,
         uiBufferSize,
         puiBytesRead
         );

#if BVCE_P_DUMP_ARC_DEBUG_LOG
   if ( NULL == hVce->hDebugLogDumpFile[eARCInstance] )
   {
      char fname[256];
      sprintf(fname, "BVCE_ARC_DEBUG_LOG_%02d_%02d.log", hVce->stOpenSettings.uiInstance, eARCInstance);

      if ( false == BVCE_Debug_P_OpenLog( fname, &hVce->hDebugLogDumpFile[eARCInstance] ) )
      {
         BDBG_ERR(("Error Creating BVCE ARC Debug Log Dump File (%s)", fname));
      }
   }

   if ( NULL != hVce->hDebugLogDumpFile[eARCInstance] )
   {
       unsigned uiBytesLeftToWrite = uiInputLengthRead;
       unsigned uiBytesToWrite = 0;

       if ( uiBytesLeftToWrite > uiInputLength0 )
       {
          uiBytesToWrite = uiInputLength0;
       }
       else
       {
          uiBytesToWrite = uiBytesLeftToWrite;
       }

       BVCE_Debug_P_WriteLogBuffer_isr( hVce->hDebugLogDumpFile[eARCInstance], pInputBuffer0, uiBytesToWrite );

      uiBytesLeftToWrite -= uiBytesToWrite;

      if ( 0 != uiBytesLeftToWrite )
      {
         BVCE_Debug_P_WriteLogBuffer_isr( hVce->hDebugLogDumpFile[eARCInstance], pInputBuffer1, uiBytesLeftToWrite );
      }
   }
#endif
      BVCE_P_Buffer_UnlockAddress( hVce->fw.debug[eARCInstance].hBuffer );
   }

   if ( 0 != uiInputLengthRead )
   {
      stDebugBufferInfo.uiReadOffset += uiInputLengthRead;
      if ( stDebugBufferInfo.uiReadOffset >= stDebugBufferInfo.uiSize )
      {
         stDebugBufferInfo.uiReadOffset -= stDebugBufferInfo.uiSize;
      }

      /* coverity[address_of] */
      /* coverity[callee_ptr_arith] */
      BVCE_P_WriteRegisters(
               hVce,
               hVce->fw.debug[eARCInstance].uiBufferInfoBaseAddress + ( ((uint8_t*)(&stDebugBufferInfo.uiReadOffset)) - ((uint8_t*)&stDebugBufferInfo)),
               (uint32_t*) (&stDebugBufferInfo.uiReadOffset),
               sizeof( stDebugBufferInfo.uiReadOffset )
               );
   }

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BVCE_Debug_ReadBuffer(
         BVCE_Handle hVce,
         BVCE_ArcInstance eARCInstance,
         char *szBuffer,   /* [in] pointer to buffer where log is copied to */
         unsigned uiBufferSize,  /* [in] maximum number of bytes to copy to buffer */
         unsigned *puiBytesRead  /* [out] number of bytes copied from debug log */
         )
{
   BERR_Code rc;
   BDBG_OBJECT_ASSERT(hVce, BVCE_P_Context);
   BDBG_ENTER( BVCE_Debug_ReadBuffer );
   BVCE_P_FUNCTION_TRACE_ENTER(1, hVce, 0);

   BVCE_Power_P_AcquireResource(
         hVce,
         BVCE_Power_Type_eClock
         );

   rc = BVCE_Debug_S_ReadBuffer_impl(
      hVce,
      eARCInstance,
      szBuffer,
      uiBufferSize,
      puiBytesRead
      );

   BVCE_Power_P_ReleaseResource(
         hVce,
         BVCE_Power_Type_eClock
         );

   BVCE_P_FUNCTION_TRACE_LEAVE(1, hVce, 0);
   BDBG_LEAVE( BVCE_Debug_ReadBuffer );
   return BERR_TRACE( rc );
}

static
BERR_Code
BVCE_Debug_S_SendCommand_impl(
         BVCE_Handle hVce,
         BVCE_ArcInstance eARCInstance,
         char        *szCommand  /* [in] pointer to a double null terminated command string of debug uart commands */
         )
{
   BERR_Code rc = BERR_SUCCESS;
   unsigned uiCommandStartIndex;
   unsigned uiCommandLength = 0;
   unsigned uiCommandCurrentIndex = 0;

   BDBG_OBJECT_ASSERT(hVce, BVCE_P_Context);
   BDBG_ASSERT( szCommand );

   /* Parse szCommand and send commands one at a time to FW */
   do
   {
      uiCommandStartIndex = uiCommandCurrentIndex;
      uiCommandLength = 0;

      while ( '\0' != szCommand[uiCommandCurrentIndex] )
      {
         uiCommandCurrentIndex++;
         uiCommandLength++;
      }
      /* Include NULL terminator */
      uiCommandCurrentIndex++;
      uiCommandLength++;

      if ( uiCommandLength > 1 )
      {
         if ( uiCommandLength < ( COMMAND_BUFFER_SIZE_BYTES - 1) )
         {
            BDBG_MSG(("Sending Debug Command: %s", &szCommand[uiCommandStartIndex] ));
            rc = BVCE_S_SendCommand_DebugCommand(
                     hVce,
                     eARCInstance,
                     &szCommand[uiCommandStartIndex],
                     uiCommandLength
                     );

            if ( rc != BERR_SUCCESS )
            {
               BDBG_ERR(("Error Sending Debug Command: %s (All subsequent commands have been aborted)", &szCommand[uiCommandStartIndex] ));
            }
         }
         else
         {
            BDBG_ERR(("Command \"%s\" is longer than support command length", &szCommand[uiCommandStartIndex]));
            rc = BERR_TRACE( BERR_INVALID_PARAMETER );
         }
      }
   } while ( ( BERR_SUCCESS == rc ) && ( '\0' != szCommand[uiCommandCurrentIndex] ) );

   return BERR_TRACE( rc );
}

BERR_Code
BVCE_Debug_SendCommand(
         BVCE_Handle hVce,
         BVCE_ArcInstance eARCInstance,
         char        *szCommand  /* [in] pointer to a double null terminated command string of debug uart commands */
         )
{
   BERR_Code rc;
   BDBG_OBJECT_ASSERT(hVce, BVCE_P_Context);
   BDBG_ENTER( BVCE_Debug_SendCommand );
   BVCE_P_FUNCTION_TRACE_ENTER(1, hVce, 0);

   BVCE_Power_P_AcquireResource(
         hVce,
         BVCE_Power_Type_eClock
         );

   rc = BVCE_Debug_S_SendCommand_impl(
      hVce,
      eARCInstance,
      szCommand
      );

   BVCE_Power_P_ReleaseResource(
         hVce,
         BVCE_Power_Type_eClock
         );

   BVCE_P_FUNCTION_TRACE_LEAVE(1, hVce, 0);
   BDBG_LEAVE( BVCE_Debug_SendCommand );
   return BERR_TRACE( rc );
}

static
void
BVCE_Debug_S_DumpRegisters_impl(
        BVCE_Handle hVce
        )
{
#if BDBG_DEBUG_BUILD
    BVCE_Platform_P_DumpRegisterList(
       hVce->handles.hReg,
       &hVce->stPlatformConfig.stWatchdogRegisterDumpList
       );

    BVCE_Platform_P_DumpRegisterList(
       hVce->handles.hReg,
       &hVce->stPlatformConfig.stHostCPUDebugRegisterDumpList
       );
#else
    BSTD_UNUSED( hVce );
#endif
}

void
BVCE_Debug_DumpRegisters(
      BVCE_Handle hVce
      )
{
   BDBG_ENTER(BVCE_Debug_DumpRegisters);
   BVCE_P_FUNCTION_TRACE_ENTER(1, hVce, 0);

   BVCE_Power_P_AcquireResource(
         hVce,
         BVCE_Power_Type_eClock
         );

   BVCE_Debug_S_DumpRegisters_impl( hVce );

   BVCE_Power_P_ReleaseResource(
         hVce,
         BVCE_Power_Type_eClock
         );

   BVCE_P_FUNCTION_TRACE_LEAVE(1, hVce, 0);
   BDBG_LEAVE(BVCE_Debug_DumpRegisters);
}

static const BVCE_Channel_Debug_DumpStateSettings s_stDefaultDumpStateSettings =
{
 BVCE_P_SIGNATURE_DUMPSTATESETTINGS, /* Signature */
 true,
 true,
 true,
 true,
 true
};

void
BVCE_Channel_Debug_GetDefaultDumpStateSettings(
   BVCE_Channel_Debug_DumpStateSettings *pstDumpStateSettings
   )
{
   BDBG_ENTER(BVCE_Channel_Debug_GetDefaultDumpStateSettings);

   if ( NULL != pstDumpStateSettings )
   {
      *pstDumpStateSettings = s_stDefaultDumpStateSettings;
   }

   BDBG_LEAVE(BVCE_Channel_Debug_GetDefaultDumpStateSettings);
}

typedef enum BVCE_P_BufferType
{
   BVCE_P_BufferType_eReg,
   BVCE_P_BufferType_eCmd,
   BVCE_P_BufferType_eBin,
   BVCE_P_BufferType_eItb,
   BVCE_P_BufferType_eCdb,

   /* Add new enums ABOVE this line */
   BVCE_P_BufferType_eMax
} BVCE_P_BufferType;

static
void
BVCE_Channel_Debug_S_DumpState_impl(
   BVCE_Channel_Handle hVceCh,
   const BVCE_Channel_Debug_DumpStateSettings *pstDumpStateSettings
   )
{
   if ( NULL == pstDumpStateSettings )
   {
      pstDumpStateSettings = &s_stDefaultDumpStateSettings;
   }

   BDBG_ASSERT( BVCE_P_SIGNATURE_DUMPSTATESETTINGS == pstDumpStateSettings->uiSignature );

#if BVCE_P_TEST_MODE
   {
      BVCE_Debug_Log_Handle hLog;
      char fname[256];
      static unsigned uiStatusLogInstance[BVCE_PLATFORM_P_NUM_ENCODE_INSTANCES][BVCE_PLATFORM_P_NUM_ENCODE_CHANNELS][BVCE_P_BufferType_eMax];
      uint64_t uiBufferOffset;
      unsigned uiBufferBase;
      unsigned uiSize;
      void* pBuffer;

      /* Registers */
      if ( true == pstDumpStateSettings->bDumpRegisters )
      {
         unsigned i;
         sprintf(fname, "BVCE_REGISTERS_%02d_%02d_%03d.txt", hVceCh->hVce->stOpenSettings.uiInstance, hVceCh->stOpenSettings.uiInstance, uiStatusLogInstance[hVceCh->hVce->stOpenSettings.uiInstance][hVceCh->stOpenSettings.uiInstance][BVCE_P_BufferType_eCmd] );
         uiStatusLogInstance[hVceCh->hVce->stOpenSettings.uiInstance][hVceCh->stOpenSettings.uiInstance][BVCE_P_BufferType_eReg]++;

         BVCE_Debug_P_OpenLog( fname, &hLog );
         for ( i = 0; i < hVceCh->hVce->stPlatformConfig.stWatchdogRegisterDumpList.uiCount; i++ )
         {
            unsigned uiAddress = hVceCh->hVce->stPlatformConfig.stWatchdogRegisterDumpList.astRegisters[i].uiAddress + hVceCh->hVce->stPlatformConfig.stWatchdogRegisterDumpList.iInstanceOffset;
            uint64_t uiRegValue;
#if BCHP_REGISTER_HAS_64_BIT
            if ( true == hVceCh->hVce->stPlatformConfig.stWatchdogRegisterDumpList.astRegisters[i].bIs64Bit )
            {
               uiRegValue = BREG_Read64( hVceCh->hVce->handles.hReg, uiAddress );
            }
            else
            {
#endif
            uiRegValue = BREG_Read32( hVceCh->hVce->handles.hReg, uiAddress );
#if BCHP_REGISTER_HAS_64_BIT
     }
#endif
            BVCE_Debug_P_WriteLog_isr( hLog, "@%08x = "BDBG_UINT64_FMT" (%s)\n", uiAddress, BDBG_UINT64_ARG(uiRegValue), hVceCh->hVce->stPlatformConfig.stWatchdogRegisterDumpList.astRegisters[i].szName);
         }
         BVCE_Debug_P_CloseLog( hLog );
      }

#if ( BVCE_P_CORE_MAJOR <= 2 )
      /* CMD Buffer */
      if ( ( true == pstDumpStateSettings->bDumpCmdBuffer ) && ( 0 != hVceCh->hVce->stPlatformConfig.stDebug.stCmd.uiBasePointer ) )
      {
         sprintf(fname, "BVCE_BUFFER_CMD_%02d_%02d_%03d.bin", hVceCh->hVce->stOpenSettings.uiInstance, hVceCh->stOpenSettings.uiInstance, uiStatusLogInstance[hVceCh->hVce->stOpenSettings.uiInstance][hVceCh->stOpenSettings.uiInstance][BVCE_P_BufferType_eCmd] );
         uiStatusLogInstance[hVceCh->hVce->stOpenSettings.uiInstance][hVceCh->stOpenSettings.uiInstance][BVCE_P_BufferType_eCmd]++;

         uiBufferBase = BREG_Read32( hVceCh->hVce->handles.hReg, hVceCh->hVce->stPlatformConfig.stDebug.stCmd.uiBasePointer );
         uiSize = 1 + BREG_Read32( hVceCh->hVce->handles.hReg, hVceCh->hVce->stPlatformConfig.stDebug.stCmd.uiEndPointer ) - uiBufferBase;
         uiBufferOffset = BVCE_P_Buffer_GetDeviceOffset_isrsafe( hVceCh->hVce->hCabacCmdBuffer ) - uiBufferBase;
         pBuffer = BVCE_P_Buffer_LockAddress( hVceCh->hVce->hCabacCmdBuffer );
         if ( NULL != pBuffer )
         {
            BVCE_P_Buffer_FlushCache_isrsafe( hVceCh->hVce->hCabacCmdBuffer, pBuffer, uiSize );
            BVCE_Debug_P_OpenLog( fname, &hLog );
            BVCE_Debug_P_WriteLogBuffer_isr( hLog, (void*)((uint8_t*)pBuffer + uiBufferOffset), uiSize );
            BVCE_Debug_P_CloseLog( hLog );
            BVCE_P_Buffer_UnlockAddress( hVceCh->hVce->hCabacCmdBuffer );
         }
      }
#endif

      /* BIN Buffer */
      if ( ( true == pstDumpStateSettings->bDumpBinBuffer ) && ( 0 != hVceCh->hVce->stPlatformConfig.stDebug.stBin[hVceCh->stOpenSettings.uiInstance].uiBasePointer ) )
      {
         sprintf(fname, "BVCE_BUFFER_BIN_%02d_%02d_%03d.bin", hVceCh->hVce->stOpenSettings.uiInstance, hVceCh->stOpenSettings.uiInstance, uiStatusLogInstance[hVceCh->hVce->stOpenSettings.uiInstance][hVceCh->stOpenSettings.uiInstance][BVCE_P_BufferType_eBin] );
         uiStatusLogInstance[hVceCh->hVce->stOpenSettings.uiInstance][hVceCh->stOpenSettings.uiInstance][BVCE_P_BufferType_eBin]++;

         uiBufferBase = BREG_ReadAddr( hVceCh->hVce->handles.hReg, hVceCh->hVce->stPlatformConfig.stDebug.stBin[hVceCh->stOpenSettings.uiInstance].uiBasePointer );
         uiSize = 1 + BREG_ReadAddr( hVceCh->hVce->handles.hReg, hVceCh->hVce->stPlatformConfig.stDebug.stBin[hVceCh->stOpenSettings.uiInstance].uiEndPointer ) - uiBufferBase;

         uiBufferOffset = BVCE_P_Buffer_GetDeviceOffset_isrsafe( hVceCh->memory[BVCE_P_HeapId_eSecure].hBuffer ) - uiBufferBase;
         pBuffer = BVCE_P_Buffer_LockAddress( hVceCh->memory[BVCE_P_HeapId_eSecure].hBuffer );
         if ( NULL != pBuffer )
         {
            BVCE_P_Buffer_FlushCache_isrsafe( hVceCh->memory[BVCE_P_HeapId_eSecure].hBuffer, pBuffer, uiSize );
            BVCE_Debug_P_OpenLog( fname, &hLog );
            BVCE_Debug_P_WriteLogBuffer_isr( hLog, (void*)((uint8_t*)pBuffer + uiBufferOffset), uiSize );
            BVCE_Debug_P_CloseLog( hLog );
            BVCE_P_Buffer_UnlockAddress( hVceCh->memory[BVCE_P_HeapId_eSecure].hBuffer );
         }
      }

      if ( NULL != hVceCh->stStartEncodeSettings.hOutputHandle )
      {
         BVCE_Output_Handle hVceOutput = hVceCh->stStartEncodeSettings.hOutputHandle;
         BDBG_OBJECT_ASSERT(hVceOutput, BVCE_P_Output_Context);

         /* ITB Buffer */
         if ( true == pstDumpStateSettings->bDumpItbBuffer )
         {
             sprintf(fname, "BVCE_BUFFER_ITB_%02d_%02d_%03d.bin", hVceCh->hVce->stOpenSettings.uiInstance, hVceCh->stOpenSettings.uiInstance, uiStatusLogInstance[hVceCh->hVce->stOpenSettings.uiInstance][hVceCh->stOpenSettings.uiInstance][BVCE_P_BufferType_eItb] );

             uiBufferBase = BREG_ReadAddr( hVceCh->hVce->handles.hReg, hVceCh->hVce->stPlatformConfig.stOutput[hVceCh->stOpenSettings.uiInstance].stITB.uiBasePointer );
             uiSize = 1 + BREG_ReadAddr( hVceCh->hVce->handles.hReg, hVceCh->hVce->stPlatformConfig.stOutput[hVceCh->stOpenSettings.uiInstance].stITB.uiEndPointer ) - uiBufferBase;

             uiBufferOffset = BVCE_P_Buffer_GetDeviceOffset_isrsafe( hVceOutput->hOutputBuffers->stITB.hBuffer ) - uiBufferBase;
             pBuffer = BVCE_P_Buffer_LockAddress( hVceOutput->hOutputBuffers->stITB.hBuffer );
             if ( NULL != pBuffer )
             {
                BVCE_P_Buffer_FlushCache_isrsafe( hVceOutput->hOutputBuffers->stITB.hBuffer, pBuffer, uiSize );
                BVCE_Debug_P_OpenLog( fname, &hLog );
                BVCE_Debug_P_WriteLogBuffer_isr( hLog, (void*)((uint8_t*)pBuffer + uiBufferOffset), uiSize );
                BVCE_Debug_P_CloseLog( hLog );
                BVCE_P_Buffer_UnlockAddress( hVceOutput->hOutputBuffers->stITB.hBuffer );
             }
          }

         /* CDB Buffer */
         if ( true == pstDumpStateSettings->bDumpCdbBuffer )
         {
             sprintf(fname, "BVCE_BUFFER_CDB_%02d_%02d_%03d.bin", hVceCh->hVce->stOpenSettings.uiInstance, hVceCh->stOpenSettings.uiInstance, uiStatusLogInstance[hVceCh->hVce->stOpenSettings.uiInstance][hVceCh->stOpenSettings.uiInstance][BVCE_P_BufferType_eCdb] );

             uiBufferBase = BREG_ReadAddr( hVceCh->hVce->handles.hReg, hVceCh->hVce->stPlatformConfig.stOutput[hVceCh->stOpenSettings.uiInstance].stCDB.uiBasePointer );
             uiSize = 1 + BREG_ReadAddr( hVceCh->hVce->handles.hReg, hVceCh->hVce->stPlatformConfig.stOutput[hVceCh->stOpenSettings.uiInstance].stCDB.uiEndPointer ) - uiBufferBase;

             uiBufferOffset = BVCE_P_Buffer_GetDeviceOffset_isrsafe( hVceOutput->hOutputBuffers->stCDB.hBuffer ) - uiBufferBase;
             pBuffer = BVCE_P_Buffer_LockAddress( hVceOutput->hOutputBuffers->stCDB.hBuffer );
             if ( NULL != pBuffer )
             {
                BVCE_P_Buffer_FlushCache_isrsafe( hVceOutput->hOutputBuffers->stCDB.hBuffer, pBuffer, uiSize );
                BVCE_Debug_P_OpenLog( fname, &hLog );
                BVCE_Debug_P_WriteLogBuffer_isr( hLog, (void*)((uint8_t*)pBuffer + uiBufferOffset), uiSize );
                BVCE_Debug_P_CloseLog( hLog );
                BVCE_P_Buffer_UnlockAddress( hVceOutput->hOutputBuffers->stCDB.hBuffer );
             }
          }
      }
      uiStatusLogInstance[hVceCh->hVce->stOpenSettings.uiInstance][hVceCh->stOpenSettings.uiInstance][BVCE_P_BufferType_eItb]++;
      uiStatusLogInstance[hVceCh->hVce->stOpenSettings.uiInstance][hVceCh->stOpenSettings.uiInstance][BVCE_P_BufferType_eCdb]++;
   }
#else
   BSTD_UNUSED( hVceCh );
#endif
}

void
BVCE_Channel_Debug_DumpState(
   BVCE_Channel_Handle hVceCh,
   const BVCE_Channel_Debug_DumpStateSettings *pstDumpStateSettings
   )
{
   BDBG_ENTER( BVCE_Channel_Debug_DumpState );
   BVCE_P_FUNCTION_TRACE_ENTER(1, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);

   BVCE_Power_P_AcquireResource(
          hVceCh->hVce,
          BVCE_Power_Type_eClock
          );

   BVCE_Channel_Debug_S_DumpState_impl(
      hVceCh,
      pstDumpStateSettings
      );

   BVCE_Power_P_ReleaseResource(
          hVceCh->hVce,
          BVCE_Power_Type_eClock
          );

   BVCE_P_FUNCTION_TRACE_LEAVE(1, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);
   BDBG_LEAVE( BVCE_Channel_Debug_DumpState );
}

BERR_Code
BVCE_Debug_GetLogMessageFifo(
   BVCE_Handle hVce,
   BVCE_Debug_FifoInfo *pstDebugFifoInfo
   )
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_OBJECT_ASSERT(hVce, BVCE_P_Context);
   BDBG_ASSERT( pstDebugFifoInfo );
   BVCE_P_FUNCTION_TRACE_ENTER(1, hVce, 0);

   BKNI_Memset( pstDebugFifoInfo, 0, sizeof( BVCE_Debug_FifoInfo) );

   if ( NULL == hVce->stDebugFifo.hBuffer )
   {
      rc = BERR_TRACE( BERR_UNKNOWN );
   }
   else
   {
      pstDebugFifoInfo->uiElementSize = hVce->stDebugFifo.uiElementSize;
      pstDebugFifoInfo->hBlock = BVCE_P_Buffer_GetBlockHandle( hVce->stDebugFifo.hBuffer );
      pstDebugFifoInfo->uiOffset = BVCE_P_Buffer_GetDeviceBaseOffset( hVce->stDebugFifo.hBuffer );
   }

   BVCE_P_FUNCTION_TRACE_LEAVE(1, hVce, 0);
   return BERR_TRACE( rc );
}

/************/
/* Watchdog */
/************/

BERR_Code
BVCE_ProcessWatchdog(
         BVCE_Handle hVce
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER( BVCE_ProcessWatchdog );
   BDBG_OBJECT_ASSERT(hVce, BVCE_P_Context);
   BVCE_P_FUNCTION_TRACE_ENTER(0, hVce, 0);

   if ( false == hVce->bWatchdogOccurred )
   {
      BDBG_WRN(("BVCE_ProcessWatchdog called but watchdog did not occur"));
   }
   else
   {
      /* Set SW_INIT_MODE=1 */
      BVCE_Platform_P_WriteRegisterList(
         hVce->handles.hReg,
         hVce->stPlatformConfig.stViceWatchdogHandlerEnable.astRegisterSettings,
         hVce->stPlatformConfig.stViceWatchdogHandlerEnable.uiRegisterCount
      );

      /* Standby to shutdown/reset core */
      BVCE_Power_Standby( hVce );

      /* Reset BVCE_OUTPUT module for each channel since the watchdog probably occurred in the middle of an encode */
      {
         unsigned i;

         for ( i = 0; i < BVCE_PLATFORM_P_NUM_OUTPUT_CHANNELS; i++ )
         {
            if ( BVCE_P_Status_eOpened == hVce->outputs[i].context.eStatus )
            {
               BVCE_Output_Reset( &hVce->outputs[i].context );
            }
         }
      }

      hVce->bWatchdogOccurred = false;

      /* Resume to reboot core and reopen channels */
      BVCE_Power_Resume( hVce );

      /* Set SW_INIT_MODE=0 */
      BVCE_Platform_P_WriteRegisterList(
         hVce->handles.hReg,
         hVce->stPlatformConfig.stViceWatchdogHandlerDisable.astRegisterSettings,
         hVce->stPlatformConfig.stViceWatchdogHandlerDisable.uiRegisterCount
      );
   }

   BVCE_P_FUNCTION_TRACE_LEAVE(0, hVce, 0);
   BDBG_LEAVE( BVCE_ProcessWatchdog );
   return BERR_TRACE( rc );
}

/*******************/
/* Device Callback */
/*******************/
void
BVCE_GetDefaultCallbackSettings(
         BVCE_CallbackSettings* pstCallbackSettings
         )
{
   BDBG_ENTER( BVCE_GetDefaultCallbackSettings );

   BDBG_ASSERT( pstCallbackSettings );

   BKNI_Memset(
            pstCallbackSettings,
            0,
            sizeof( BVCE_CallbackSettings )
            );

   pstCallbackSettings->uiSignature = BVCE_P_SIGNATURE_DEVICECALLBACKSETTINGS;

   BDBG_LEAVE( BVCE_GetDefaultCallbackSettings );
   return;
}

BERR_Code
BVCE_SetCallbackSettings(
         BVCE_Handle hVce,
         const BVCE_CallbackSettings* pstCallbackSettings
         )
{
   BDBG_ENTER( BVCE_SetCallbackSettings );

   BDBG_OBJECT_ASSERT(hVce, BVCE_P_Context);
   BDBG_ASSERT( pstCallbackSettings );
   BDBG_ASSERT( BVCE_P_SIGNATURE_DEVICECALLBACKSETTINGS == pstCallbackSettings->uiSignature );

   BKNI_EnterCriticalSection();

   hVce->callbacks.stCallbackSettings = *pstCallbackSettings;

   BKNI_LeaveCriticalSection();

   BDBG_LEAVE( BVCE_SetCallbackSettings );
   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BVCE_GetCallbackSettings(
         BVCE_Handle hVce,
         BVCE_CallbackSettings* pstCallbackSettings
         )
{
   BDBG_ENTER( BVCE_GetCallbackSettings );

   BDBG_OBJECT_ASSERT(hVce, BVCE_P_Context);
   BDBG_ASSERT( pstCallbackSettings );
   BVCE_P_FUNCTION_TRACE_ENTER(0, hVce, 0);

   *pstCallbackSettings = hVce->callbacks.stCallbackSettings;

   BVCE_P_FUNCTION_TRACE_LEAVE(0, hVce, 0);
   BDBG_LEAVE( BVCE_GetCallbackSettings );
   return BERR_TRACE( BERR_SUCCESS );
}


/**********************/
/* Channel Open/Close */
/**********************/
static const BVCE_Channel_OpenSettings s_stDefaultChannelOpenSettings =
{
 BVCE_P_SIGNATURE_CHANNELOPENSETTINGS, /* Signature */
 0, /* Channel Instance */
 /* Memory Config */
 {
   0,
   0,
   0,
   0,
   0,
   0,
 },
 BVCE_MultiChannelMode_eMulti,
 0,
 {
    false, /* bAllocateOutputMemory */
    NULL, /* CDB Heap */
    NULL, /* ITB Heap */
    true, /* bEnableDataUnitDetection */
 },
 NULL, /* hPictureMem */
 NULL, /* hSecureMem */
 NULL, /* hGeneralMem */
};

void
BVCE_Channel_GetDefaultOpenSettings(
         const BBOX_Handle hBox,
         BVCE_Channel_OpenSettings *pstChSettings
         )
{
   BDBG_ENTER( BVCE_Channel_GetDefaultOpenSettings );

   BDBG_ASSERT( pstChSettings );

   if ( NULL != pstChSettings )
   {
      *pstChSettings = s_stDefaultChannelOpenSettings;

      /* SW7435-1069: Calculate memory size using BVCE_Channel_GetMemoryConfig() */
      {
         BVCE_MemoryConfig stMemoryConfig;

         BVCE_Channel_GetMemoryConfig( hBox, NULL, NULL, &stMemoryConfig );

         pstChSettings->stMemoryConfig = stMemoryConfig;

         {
            BBOX_Config *pstBoxConfig = (BBOX_Config *) BKNI_Malloc( sizeof( BBOX_Config ) );
            if ( NULL != pstBoxConfig )
            {
               BBOX_GetConfig( hBox, pstBoxConfig );

               BDBG_MODULE_MSG(BVCE_MEMORY, ("BVCE_Channel_GetDefaultOpenSettings(boxMode=%d):", pstBoxConfig->stBox.ulBoxId ));
               BKNI_Free( pstBoxConfig );
            }
            else
            {
               BDBG_MODULE_MSG(BVCE_MEMORY, ("BVCE_Channel_GetDefaultOpenSettings(boxMode=?):"));
            }
         }
#define BVCE_MEMCONFIG_FIELD(_field) BDBG_MODULE_MSG(BVCE_MEMORY, (#_field"=%lu bytes", (unsigned long) stMemoryConfig._field));
#include "bvce_memconfig.inc"
      }
   }

   BDBG_LEAVE( BVCE_Channel_GetDefaultOpenSettings );

   return;
}

static
void
BVCE_Channel_Output_S_Free(
   BVCE_Channel_Handle hVceCh
   )
{
   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);

   BVCE_Output_Close( hVceCh->stOutput.hVceOutput );
   BVCE_Output_FreeBuffers( hVceCh->stOutput.hVceOutputBuffers );
}

static
BERR_Code
BVCE_Channel_Output_S_Allocate(
   BVCE_Channel_Handle hVceCh
   )
{
   BERR_Code rc;
   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);

   {
      BVCE_Output_AllocBuffersSettings stOutputAllocBuffersSettings;


      BVCE_Output_GetDefaultAllocBuffersSettings(
               hVceCh->hVce->stOpenSettings.hBox,
               &stOutputAllocBuffersSettings
         );
      stOutputAllocBuffersSettings.stConfig.Itb.Length = BVCE_P_ALIGN( hVceCh->stOpenSettings.stMemoryConfig.uiIndexMemSize, BVCE_PLATFORM_P_ITB_ALIGNMENT);
      stOutputAllocBuffersSettings.stConfig.Cdb.Length = BVCE_P_ALIGN( hVceCh->stOpenSettings.stMemoryConfig.uiDataMemSize, 32);
      stOutputAllocBuffersSettings.hITBMem = hVceCh->stOpenSettings.stOutput.hIndexMem;
      stOutputAllocBuffersSettings.hCDBMem = hVceCh->stOpenSettings.stOutput.hDataMem;

      rc = BVCE_Output_AllocBuffers(
               hVceCh->hVce,
               &hVceCh->stOutput.hVceOutputBuffers,
               &stOutputAllocBuffersSettings
               );

      if ( BERR_SUCCESS != rc )
      {
         BVCE_Channel_Output_S_Free( hVceCh );
         return BERR_TRACE( rc );
      }
   }

   {
      BVCE_Output_OpenSettings stOutputOpenSettings;

      BVCE_Output_GetDefaultOpenSettings(
         hVceCh->hVce->stOpenSettings.hBox,
         &stOutputOpenSettings
         );
      stOutputOpenSettings.uiInstance = hVceCh->stOpenSettings.uiInstance;
      stOutputOpenSettings.bEnableDataUnitDetection = hVceCh->stOpenSettings.stOutput.bEnableDataUnitDetection;
      stOutputOpenSettings.hDescriptorMem = hVceCh->stOpenSettings.stOutput.hIndexMem;

      rc = BVCE_Output_Open(
            hVceCh->hVce,
            &hVceCh->stOutput.hVceOutput,
            &stOutputOpenSettings
            );
      if ( BERR_SUCCESS != rc )
      {
         BVCE_Channel_Output_S_Free( hVceCh );
         return BERR_TRACE( rc );
      }
   }

   rc = BVCE_Output_SetBuffers(
      hVceCh->stOutput.hVceOutput,
      hVceCh->stOutput.hVceOutputBuffers
      );

   if ( BERR_SUCCESS != rc )
   {
      BVCE_Channel_Output_S_Free( hVceCh );
      return BERR_TRACE( rc );
   }

   return BERR_TRACE( rc );
}

#define BVCE_UserData_P_PacketDescriptor_MAX_PER_FIELD BVCE_FW_P_UserData_PacketType_eMax
#define BVCE_USERDATA_P_QUEUE_SIZE (BVCE_FW_P_USERDATA_QUEUE_LENGTH * BVCE_FW_P_UserData_PacketDescriptor_MAX_LENGTH * BVCE_UserData_P_PacketDescriptor_MAX_PER_FIELD);

static
BERR_Code
BVCE_Channel_S_Open_impl(
         BVCE_Handle hVce,
         BVCE_Channel_Handle *phVceCh,
         const BVCE_Channel_OpenSettings *pstChOpenSettings /* [in] VCE Channel settings */
         )
{
   BERR_Code rc;
   BVCE_Channel_Handle hVceCh;

   BDBG_ENTER( BVCE_Channel_Open );

   BDBG_OBJECT_ASSERT(hVce, BVCE_P_Context);
   BDBG_ASSERT( phVceCh );
   BDBG_ASSERT( pstChOpenSettings );
   BDBG_ASSERT( BVCE_P_SIGNATURE_CHANNELOPENSETTINGS == pstChOpenSettings->uiSignature );

   if ( pstChOpenSettings->uiInstance >= hVce->stPlatformConfig.stBox.uiTotalChannels )
   {
      BDBG_ERR(("Invalid channel instance (%d) specified", pstChOpenSettings->uiInstance));
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   /* Validate the MultiChannelMode settings */
   {
      unsigned uiInstance;

      switch ( pstChOpenSettings->eMultiChannelMode )
      {
         case BVCE_MultiChannelMode_eSingle:
            /* Ensure no other channels are open */
            for (uiInstance = 0; uiInstance < BVCE_PLATFORM_P_NUM_ENCODE_CHANNELS; uiInstance++ )
            {
               if ( BVCE_P_Status_eIdle != hVce->channels[uiInstance].context.eStatus )
               {
                  BDBG_ERR(("Cannot open channel in MultiChannelMode_eSingle when other channels are open.  Channel[%d] is open.",
                     uiInstance
                     ));
                  return BERR_TRACE( BERR_INVALID_PARAMETER );
               }
            }
            break;

         default:
            /* Ensure other open channels have same MultiChannelMode */
            for (uiInstance = 0; uiInstance < BVCE_PLATFORM_P_NUM_ENCODE_CHANNELS; uiInstance++ )
            {
               if ( BVCE_P_Status_eIdle != hVce->channels[uiInstance].context.eStatus )
               {
                  if ( pstChOpenSettings->eMultiChannelMode != hVce->channels[uiInstance].context.stOpenSettings.eMultiChannelMode )
                  {
                     BDBG_ERR(("All channels must be opened with the same MultiChannelMode. Channel[%d] is open with MultiChannelMode=%d.",
                        uiInstance,
                        hVce->channels[uiInstance].context.stOpenSettings.eMultiChannelMode
                        ));
                     return BERR_TRACE( BERR_INVALID_PARAMETER );
                  }
               }
            }
            break;
      }
   }

   hVceCh = &hVce->channels[pstChOpenSettings->uiInstance].context;

   switch ( hVceCh->eStatus )
   {
      case BVCE_P_Status_eIdle:
         /* We're good to go */
         break;

      case BVCE_P_Status_eOpened:
         BDBG_ERR(("Channel is already open"));
         return BERR_TRACE( BERR_UNKNOWN );

      case BVCE_P_Status_eStarted:
         BDBG_ERR(("Channel is already started"));
         return BERR_TRACE( BERR_UNKNOWN );

      case BVCE_P_Status_eStopping:
         /* Continue with warning */
         BDBG_WRN(("Channel is already open and has not completed stopping"));
         return BERR_TRACE( BERR_UNKNOWN );

      default:
         BDBG_ERR(("Unknown state"));
         return BERR_TRACE( BERR_UNKNOWN );
   }

   BKNI_Memset(
            hVceCh,
            0,
            sizeof( BVCE_P_Channel_Context )
            );

   BDBG_OBJECT_SET(hVceCh, BVCE_P_Channel_Context);

   hVceCh->hVce = hVce;
   hVceCh->eStatus = BVCE_P_Status_eOpened;
   hVceCh->stOpenSettings = *pstChOpenSettings;
   BVCE_Channel_GetDefaultEncodeSettings(
            hVce->stOpenSettings.hBox,
            &hVceCh->stEncodeSettings
            );

   /* Select pre-allocated or dynamic heap */
   {
      BVCE_P_HeapId eHeapId;

      BVCE_P_Buffer_AllocSettings stAllocSettings;

      BVCE_P_Buffer_GetDefaultAllocSettings( &stAllocSettings );
      stAllocSettings.uiAlignment = BVCE_P_DEFAULT_ALIGNMENT;

      for ( eHeapId = 0; eHeapId < BVCE_P_HeapId_eMax; eHeapId++ )
      {
         BMMA_Heap_Handle hMma = NULL;

         switch (eHeapId)
         {
         case BVCE_P_HeapId_eSystem:
            stAllocSettings.uiSize = hVceCh->stOpenSettings.stMemoryConfig.uiGeneralMemSize;
            if (NULL != hVceCh->stOpenSettings.hGeneralMem ) BDBG_MODULE_MSG( BVCE_MEMORY, ("System Heap Channel[%d][%d]: BVCE_Channel_OpenSettings.hGeneralMem", hVceCh->hVce->stOpenSettings.uiInstance, hVceCh->stOpenSettings.uiInstance) );
            hMma = hVceCh->stOpenSettings.hGeneralMem;
            break;

         case BVCE_P_HeapId_ePicture:
            stAllocSettings.uiSize = hVceCh->stOpenSettings.stMemoryConfig.uiPictureMemSize;
            if (NULL != hVceCh->stOpenSettings.hPictureMem ) BDBG_MODULE_MSG( BVCE_MEMORY, ("Picture Heap Channel[%d][%d]: BVCE_Channel_OpenSettings.hPictureMem", hVceCh->hVce->stOpenSettings.uiInstance, hVceCh->stOpenSettings.uiInstance) );
            hMma = hVceCh->stOpenSettings.hPictureMem;
            break;

         case BVCE_P_HeapId_eSecure:
            stAllocSettings.uiSize = hVceCh->stOpenSettings.stMemoryConfig.uiSecureMemSize;
            if (NULL != hVceCh->stOpenSettings.hSecureMem ) BDBG_MODULE_MSG( BVCE_MEMORY, ("Secure Heap Channel[%d][%d]: BVCE_Channel_OpenSettings.hSecureMem", hVceCh->hVce->stOpenSettings.uiInstance, hVceCh->stOpenSettings.uiInstance) );
            hMma = hVceCh->stOpenSettings.hSecureMem;
            break;

         /* coverity[dead_error_begin] */
         default:
            continue;
         }

         if ( hMma != NULL )
         {
            rc = BVCE_P_Allocator_Create(
               hMma,
               &stAllocSettings,
               &hVceCh->memory[eHeapId].hAllocator
               );


            if ( BERR_SUCCESS != rc )
            {
               BDBG_ERR(("Error creating local allocator [%d]", eHeapId));
               BVCE_Channel_Close( hVceCh );
               return BERR_TRACE( rc );
            }
            BDBG_MODULE_MSG( BVCE_MEMORY, ("Created Channel Memory Allocator[%d]: %08lx bytes @ "BDBG_UINT64_FMT,
               eHeapId,
               (unsigned long) BVCE_P_Allocator_GetSize( hVceCh->memory[eHeapId].hAllocator ),
               BDBG_UINT64_ARG(BVCE_P_Allocator_GetDeviceOffset( hVceCh->memory[eHeapId].hAllocator ))
               ));
         }
         else
         {
            hVceCh->memory[eHeapId].hAllocator = hVceCh->hVce->ahAllocator[eHeapId];
         }
      }
   }

   /* Allocate Memory */
   {
      BVCE_P_Buffer_AllocSettings astAllocSettings[BVCE_P_HeapId_eMax];
      BVCE_P_HeapId eHeapId;

      for ( eHeapId = 0; eHeapId < BVCE_P_HeapId_eMax; eHeapId++ )
      {
         BVCE_P_Buffer_GetDefaultAllocSettings( &astAllocSettings[eHeapId] );
         astAllocSettings[eHeapId].uiAlignment = BVCE_P_DEFAULT_ALIGNMENT;
         astAllocSettings[eHeapId].bLockOffset = true;
      }

      astAllocSettings[BVCE_P_HeapId_ePicture].uiSize = hVceCh->stOpenSettings.stMemoryConfig.uiPictureMemSize;
      astAllocSettings[BVCE_P_HeapId_eSecure].uiSize = hVceCh->stOpenSettings.stMemoryConfig.uiSecureMemSize;

      BDBG_MODULE_MSG(BVCE_MEMORY, ("BVCE_Channel_Open():"));
#define BVCE_MEMCONFIG_FIELD(_field) BDBG_MODULE_MSG(BVCE_MEMORY, ("BVCE_Channel_OpenSettings.stMemoryConfig."#_field"=%lu bytes", (unsigned long) hVceCh->stOpenSettings.stMemoryConfig._field));
#include "bvce_memconfig.inc"

      for ( eHeapId = 0; eHeapId < BVCE_P_HeapId_eMax; eHeapId++ )
      {
         /* Allocate Memory */
         if ( 0 != astAllocSettings[eHeapId].uiSize )
         {
            rc = BVCE_P_Buffer_Alloc(
               hVceCh->memory[eHeapId].hAllocator,
               &astAllocSettings[eHeapId],
               &hVceCh->memory[eHeapId].hBuffer
               );

            if ( BERR_SUCCESS != rc )
            {
               BDBG_ERR(("Error allocating channel memory buffer"));
               BVCE_Channel_Close( hVceCh );
               return BERR_TRACE( rc );
            }
         }
      }
   }

   /* Allocate user data queue */
   if ( 0 != BVCE_FW_P_USERDATA_QUEUE_LENGTH )
   {
      BVCE_P_Buffer_AllocSettings stAllocSettings;
      BVCE_P_Buffer_GetDefaultAllocSettings( &stAllocSettings );
      stAllocSettings.uiSize = BVCE_USERDATA_P_QUEUE_SIZE;
      stAllocSettings.uiAlignment = BVCE_P_DEFAULT_ALIGNMENT;
      stAllocSettings.bLockOffset = true;
      stAllocSettings.bLockAddress = true;

      rc = BVCE_P_Buffer_Alloc(
               hVceCh->hVce->ahAllocator[BVCE_P_HeapId_eSystem],
               &stAllocSettings,
               &hVceCh->userdata.hBuffer
               );

      if ( BERR_SUCCESS != rc )
      {
         BDBG_ERR(("Error allocating user data queue"));
         BVCE_Channel_Close( hVceCh );
         return BERR_TRACE( BERR_OUT_OF_DEVICE_MEMORY );
      }
   }

   /* Send Open Channel Command */
   rc = BVCE_S_SendCommand_OpenChannel(
            hVce,
            hVceCh
            );
   if ( BERR_SUCCESS != rc )
   {
      BVCE_Channel_Close( hVceCh );
      return BERR_TRACE( rc );
   }

   if ( 0 != hVce->stPlatformConfig.stMailbox.uiBvn2ViceMailboxAddress )
   {
      uint32_t uiBVNMetaDataOffset = BREG_Read32(
               hVce->handles.hReg,
               hVce->stPlatformConfig.stMailbox.uiBvn2ViceMailboxAddress
               );

      BDBG_ASSERT( BVN2VICE_MBOX_PAYLOAD_OFFSET == uiBVNMetaDataOffset );
   }

#if BVCE_P_DUMP_USERDATA_LOG
         /* Debug */
         if ( NULL == hVceCh->userdata.hUserDataLog )
         {
            char fname[256];
            sprintf(fname, "BVCE_USERDATA_LOG_%02d_%02d.csv", hVceCh->hVce->stOpenSettings.uiInstance, hVceCh->stOpenSettings.uiInstance);

            if ( false == BVCE_Debug_P_OpenLog( fname, &hVceCh->userdata.hUserDataLog ) )
            {
               BDBG_ERR(("Error Creating BVCE User Data Log Dump File (%s)", fname));
            }
            else
            {
               BVCE_Debug_P_WriteLog_isr(hVceCh->userdata.hUserDataLog, "stc,queue_read_offset,queue_write_offset,queue_metadata_stg_id,queue_metadata_polarity,queue_metadata_length,offset,index,length,type,analog,num_lines,cc_metadata,cc_data_0,cc_data_1\n");
            }
         }
#endif

   if ( true == hVceCh->stOpenSettings.stOutput.bAllocateOutput )
   {
      rc = BVCE_Channel_Output_S_Allocate( hVceCh );
      if ( BERR_SUCCESS != rc )
      {
         BVCE_Channel_Close( hVceCh );
         return BERR_TRACE( rc );
      }
   }

   *phVceCh = hVceCh;

    return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BVCE_Channel_Open(
         BVCE_Handle hVce,
         BVCE_Channel_Handle *phVceCh,
         const BVCE_Channel_OpenSettings *pstChOpenSettings /* [in] VCE Channel settings */
         )
{
   BERR_Code rc;
   BDBG_OBJECT_ASSERT(hVce, BVCE_P_Context);
   BDBG_ENTER( BVCE_Channel_Open );
   BVCE_P_FUNCTION_TRACE_ENTER(0, hVce, pstChOpenSettings->uiInstance);

   BVCE_Power_P_AcquireResource(
         hVce,
         BVCE_Power_Type_eClock
         );

   rc = BVCE_Channel_S_Open_impl(
            hVce,
            phVceCh,
            pstChOpenSettings
            );

   BVCE_Power_P_ReleaseResource(
         hVce,
         BVCE_Power_Type_eClock
         );

   BVCE_P_FUNCTION_TRACE_LEAVE(0, hVce, pstChOpenSettings->uiInstance);
   BDBG_LEAVE( BVCE_Channel_Open );
   return BERR_TRACE( rc );
}

static
BERR_Code
BVCE_Channel_S_Close_impl(
         BVCE_Channel_Handle hVceCh
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BVCE_P_HeapId eHeapId;

   BDBG_ENTER( BVCE_Channel_Close );

   switch ( hVceCh->eStatus )
   {
      case BVCE_P_Status_eIdle:
         BDBG_ERR(("Channel is not open"));
         return BERR_TRACE( BERR_UNKNOWN );

      case BVCE_P_Status_eOpened:
         /* We're good to go */
         break;

      case BVCE_P_Status_eStarted:
         /* Let's stop the channel and then close */
         BDBG_WRN(("Channel is started, stopping/flushing first..."));
         {
            BVCE_Channel_StopEncodeSettings stChStopEncodeSettings;

            BVCE_Channel_GetDefaultStopEncodeSettings(&stChStopEncodeSettings);

            stChStopEncodeSettings.eStopMode = BVCE_Channel_StopMode_eAbort;

            rc = BVCE_Channel_StopEncode(
               hVceCh,
               &stChStopEncodeSettings
               );
         }
         break;

      case BVCE_P_Status_eStopping:
         /* Continue with warning */
         BDBG_WRN(("Channel is in process of stopping, flushing..."));
         BVCE_Channel_FlushEncode( hVceCh );
         break;

      default:
         BDBG_ERR(("Unknown state"));
         return BERR_TRACE( BERR_UNKNOWN );
   }

#if BVCE_P_TEST_MODE
   BVCE_Debug_P_CloseLog( hVceCh->hStatusLog );
   hVceCh->hStatusLog = NULL;
#endif

   if ( NULL != hVceCh->stStartEncodeSettings.hOutputHandle )
   {
      hVceCh->stStartEncodeSettings.hOutputHandle->state.hVceCh = NULL;
      hVceCh->stStartEncodeSettings.hOutputHandle = NULL;
   }

   if ( true == hVceCh->stOpenSettings.stOutput.bAllocateOutput )
   {
      BVCE_Channel_Output_S_Free( hVceCh );
   }

   if ( false == hVceCh->hVce->bWatchdogOccurred )
   {
      /* Send Close Channel Command */
      rc = BVCE_S_SendCommand_CloseChannel(
               hVceCh->hVce,
               hVceCh
               );
      BERR_TRACE( rc );
   }

   hVceCh->eStatus = BVCE_P_Status_eIdle;

   BVCE_P_Buffer_Free( hVceCh->userdata.hBuffer );
   hVceCh->userdata.hBuffer = NULL;

   for ( eHeapId = 0; eHeapId < BVCE_P_HeapId_eMax; eHeapId++ )
   {
      BMMA_Heap_Handle hMma = NULL;
      BVCE_P_Buffer_Free( hVceCh->memory[eHeapId].hBuffer );

      switch (eHeapId)
      {
      case BVCE_P_HeapId_eSystem:
         hMma = hVceCh->stOpenSettings.hGeneralMem;
         break;

      case BVCE_P_HeapId_ePicture:
         hMma = hVceCh->stOpenSettings.hPictureMem;
         break;

      case BVCE_P_HeapId_eSecure:
         hMma = hVceCh->stOpenSettings.hSecureMem;
         break;

      /* coverity[dead_error_begin] */
      default:
         continue;
      }

      if ( hMma != NULL )
      {
         BDBG_MODULE_MSG( BVCE_MEMORY, ("Freed Channel Memory Allocator[%d]: %08lx bytes @ "BDBG_UINT64_FMT,
               eHeapId,
               (unsigned long) BVCE_P_Allocator_GetSize( hVceCh->memory[eHeapId].hAllocator ),
               BDBG_UINT64_ARG(BVCE_P_Allocator_GetDeviceOffset( hVceCh->memory[eHeapId].hAllocator ))
            ));

         BVCE_P_Allocator_Destroy(
            hVceCh->memory[eHeapId].hAllocator
            );
      }

      hVceCh->memory[eHeapId].hBuffer = NULL;
   }

   return BERR_TRACE( rc );
}

BERR_Code
BVCE_Channel_Close(
         BVCE_Channel_Handle hVceCh
         )
{
   BERR_Code rc;
   BDBG_ENTER( BVCE_Channel_Close );
   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);
   BVCE_P_FUNCTION_TRACE_ENTER(0, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);

   BVCE_Power_P_AcquireResource(
         hVceCh->hVce,
         BVCE_Power_Type_eClock
         );

   rc = BVCE_Channel_S_Close_impl( hVceCh );

   BVCE_Power_P_ReleaseResource(
         hVceCh->hVce,
         BVCE_Power_Type_eClock
         );

   BVCE_P_FUNCTION_TRACE_LEAVE(0, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);
   BDBG_OBJECT_UNSET(hVceCh, BVCE_P_Channel_Context);

   BDBG_LEAVE( BVCE_Channel_Close );
   return BERR_TRACE( rc );
}


/*********************/
/* Channel Callbacks */
/*********************/
void
BVCE_Channel_GetDefaultCallbackSettings(
         BVCE_Channel_CallbackSettings* pstCallbackSettings
         )
{
   BDBG_ENTER( BVCE_Channel_GetDefaultCallbackSettings );

   BDBG_ASSERT( pstCallbackSettings );

   BKNI_Memset(
            pstCallbackSettings,
            0,
            sizeof( BVCE_Channel_CallbackSettings )
            );

   pstCallbackSettings->uiSignature = BVCE_P_SIGNATURE_CHANNELCALLBACKSETTINGS;

   BDBG_LEAVE( BVCE_Channel_GetDefaultCallbackSettings );
   return;
}

static
BERR_Code
BVCE_Channel_S_SetCallbackSettings_impl(
         BVCE_Channel_Handle hVceCh,
         const BVCE_Channel_CallbackSettings* pstCallbackSettings
         )
{
   BERR_Code rc;
   bool bConfigNow = true;

   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);
   BDBG_ASSERT( pstCallbackSettings );
   BDBG_ASSERT( BVCE_P_SIGNATURE_CHANNELCALLBACKSETTINGS == pstCallbackSettings->uiSignature );

   switch ( hVceCh->eStatus )
   {
      case BVCE_P_Status_eIdle:
         BDBG_ERR(("Channel is not open"));
         return BERR_TRACE( BERR_UNKNOWN );

      case BVCE_P_Status_eOpened:
         bConfigNow = false; /* Channel will be configured at StartEncode */
         break;

      case BVCE_P_Status_eStarted:
         bConfigNow = true; /* Channel is started, so needs to configure only dynamic parameters */
         break;

      case BVCE_P_Status_eStopping:
         /* Continue with warning */
         BDBG_WRN(("Channel not completed stopping, will be configured when re-started"));
         bConfigNow = false; /* Channel will be configured at StartEncode */
         break;

      default:
         BDBG_ERR(("Unknown state"));
         return BERR_TRACE( BERR_UNKNOWN );
   }

   if ( ( hVceCh->stCallbackSettings.stEvent.uiEventMask == pstCallbackSettings->stEvent.uiEventMask )
        && ( hVceCh->stCallbackSettings.stEvent.uiErrorMask == pstCallbackSettings->stEvent.uiErrorMask )
      )
   {
      /* Error/Event masks haven't changed, so no need to send a config command to the FW */
      bConfigNow = false;
   }

   BKNI_EnterCriticalSection();

   hVceCh->stCallbackSettings = *pstCallbackSettings;

   BKNI_LeaveCriticalSection();

   if ( true == bConfigNow )
   {
      /* Send Config Channel Command */
      rc = BVCE_S_SendCommand_ConfigChannel(
               hVceCh->hVce,
               hVceCh,
               NULL
               );
      if ( BERR_SUCCESS != rc )
      {
         BDBG_ERR(("Error configuring channel"));
         return BERR_TRACE( rc );
      }
   }

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BVCE_Channel_SetCallbackSettings(
         BVCE_Channel_Handle hVceCh,
         const BVCE_Channel_CallbackSettings* pstCallbackSettings
         )
{
   BERR_Code rc;
   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);
   BDBG_ENTER( BVCE_Channel_SetCallbackSettings );
   BVCE_P_FUNCTION_TRACE_ENTER(0, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);

   BVCE_Power_P_AcquireResource(
         hVceCh->hVce,
         BVCE_Power_Type_eClock
         );

   rc = BVCE_Channel_S_SetCallbackSettings_impl(
            hVceCh,
            pstCallbackSettings
            );

   BVCE_Power_P_ReleaseResource(
         hVceCh->hVce,
         BVCE_Power_Type_eClock
         );

   BVCE_P_FUNCTION_TRACE_LEAVE(0, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);
   BDBG_LEAVE( BVCE_Channel_SetCallbackSettings );
   return BERR_TRACE( rc );
}
BERR_Code
BVCE_Channel_GetCallbackSettings(
         BVCE_Channel_Handle hVceCh,
         BVCE_Channel_CallbackSettings* pstCallbackSettings
         )
{
   BDBG_ENTER( BVCE_Channel_GetCallbackSettings );

   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);
   BDBG_ASSERT( pstCallbackSettings );
   BVCE_P_FUNCTION_TRACE_ENTER(0, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);

   *pstCallbackSettings = hVceCh->stCallbackSettings;

   BVCE_P_FUNCTION_TRACE_LEAVE(0, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);
   BDBG_LEAVE( BVCE_Channel_GetCallbackSettings );
   return BERR_TRACE( BERR_SUCCESS );
}

/*******************************/
/* Channel Start/Settings/Stop */
/*******************************/
static const BVCE_Channel_StartEncodeSettings s_stDefaultStartEncodeSettings =
{
 BVCE_P_SIGNATURE_STARTENCODESETTINGS, /* Signature */
 false, /* Real Time Encode Mode */
 false, /* Pipeline Normal Delay Mode */
 false, /* Fast Channel Change Mode is OFF */
 {
  BAVC_VideoCompressionStd_eH264, /* Protocol */
  BAVC_VideoCompressionProfile_eHigh, /* Profile */
  BAVC_VideoCompressionLevel_e40, /* Level */
 },
 BAVC_ScanType_eProgressive, /* Scan Type */
 NULL, /* Output Handle */
 1, /* Encoder defaults to STC[1].  Decoder typically uses STC[0]. */
 {
    {
       BAVC_FrameRateCode_e7_493,
       BAVC_FrameRateCode_e60,
    },
    {
       BAVC_FrameRateCode_e23_976,
    },
    {
       {
          DEFAULT_A2P_MAX_PIC_WIDTH_PELS,
          DEFAULT_A2P_MAX_PIC_HEIGT_PELS,
       },
       {
          0,
          0,
       },
    },
    {
     false, /* bAllowNewGOPOnSceneChange */
     20,    /* uiMinGOPLengthAfterSceneChange */
     0,     /* uiDuration */
     0,     /* uiDurationRampUpFactor */
     14,    /* uiNumberOfPFrames */
     BVCE_PLATFORM_P_MAX_B_PICTURES_PROGRESSIVE,     /* uiNumberOfBFrames */
     false, /* bAllowOpenGop = false */
    },
    {
       { 0, 0 }, /* Largest Target/Max Bitrate */
    },
 },
 DEFAULT_A2P_RATE_BUFFER,
 0, /* uiNumParallelNRTEncodes */
 BVCE_EntropyCoding_eDefault, /* eForceEntropyCoding */
 /* stMemoryBandwidthSavingMode */
 {
    false,
    false,
 },
 /* stRateControl */
 {
    /* stHrdMode */
    {
       false, /* bDisableFrameDrop */
    },
    /* stSegmentMode */
    {
       false, /* bEnable */
       3000, /* uiDuration */
       /* stTargetBitRateTolerance */
       {
          /* stUpper */
          {
             20, /* uiTolerance */
             80, /* uiSlopeFactor */
          },
          /* stLower */
          {
             20, /* uiTolerance */
             150, /* uiSlopeFactor */
          },

       },
    },
 },
};

void
BVCE_Channel_GetDefaultStartEncodeSettings(
         const BBOX_Handle hBox,
         BVCE_Channel_StartEncodeSettings *pstChStartEncodeSettings
         )
{
   BDBG_ENTER( BVCE_Channel_GetDefaultStartEncodeSettings );

   BDBG_ASSERT( pstChStartEncodeSettings );

   *pstChStartEncodeSettings = s_stDefaultStartEncodeSettings;
   BVCE_Platform_P_OverrideChannelDefaultStartEncodeSettings( hBox, pstChStartEncodeSettings );

   BDBG_LEAVE( BVCE_Channel_GetDefaultStartEncodeSettings );
   return;
}

static
void
BVCE_Channel_S_ValidateSettings(
   BVCE_Channel_StartEncodeSettings *pstChStartEncodeSettings,
   BVCE_Channel_EncodeSettings *pstChEncodeSettings
   )
{
   BDBG_ASSERT( pstChStartEncodeSettings );
   BDBG_ASSERT( pstChEncodeSettings );

   if ( 0 == pstChEncodeSettings->uiA2PDelay )
   {
      BVCE_A2PDelay stA2PDelay;
      BVCE_GetA2PDelayInfo( pstChEncodeSettings, pstChStartEncodeSettings, &stA2PDelay);

      pstChEncodeSettings->uiA2PDelay = stA2PDelay.uiMin;
   }
}

/* BVCE_Channel_StartEncode - Configures the encoder and starts the encode process.
 *
 * Note: BVCE_Channel_SetEncodeSettings() should be called before this to set up the initial encode parameters
 */
static
BERR_Code
BVCE_Channel_S_StartEncode_impl(
         BVCE_Channel_Handle hVceCh,
         const BVCE_Channel_StartEncodeSettings *pstChStartEncodeSettings
         )
{
   BERR_Code rc;
   BVCE_P_SendCommand_ConfigChannel_Settings stConfigChannelSettings;

   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);
   BDBG_ASSERT( pstChStartEncodeSettings );
   BDBG_ASSERT( BVCE_P_SIGNATURE_STARTENCODESETTINGS == pstChStartEncodeSettings->uiSignature );

   switch ( hVceCh->eStatus )
   {
      case BVCE_P_Status_eIdle:
         BDBG_ERR(("Channel is not open"));
         return BERR_TRACE( BERR_UNKNOWN );

      case BVCE_P_Status_eOpened:
         /* We're good to go */
         break;

      case BVCE_P_Status_eStarted:
         BDBG_ERR(("Channel is already started"));
         return BERR_TRACE( BERR_UNKNOWN );

      case BVCE_P_Status_eStopping:
      {
         BVCE_Channel_Status stChannelStatus;
         rc = BVCE_Channel_GetStatus( hVceCh, &stChannelStatus );

         if ( rc != BERR_SUCCESS )
         {
            BDBG_LEAVE( BVCE_Channel_StopEncode );
            return BERR_TRACE( rc );
         }

         /* Continue with warning */
         if ( BVCE_P_Status_eStopping == hVceCh->eStatus )
         {
            BDBG_WRN(("Channel has not completed stopping, mux should flush any stale descriptors"));
         }
      }
         break;

      default:
         BDBG_ERR(("Unknown state"));
         return BERR_TRACE( BERR_UNKNOWN );
   }

   hVceCh->stStartEncodeSettings = *pstChStartEncodeSettings;

   if ( NULL != hVceCh->stStartEncodeSettings.hOutputHandle )
   {
      BDBG_ASSERT( NULL != pstChStartEncodeSettings->hOutputHandle->hOutputBuffers );
   }
   else if ( ( true == hVceCh->stOpenSettings.stOutput.bAllocateOutput )
             && ( NULL != hVceCh->stOutput.hVceOutput ) )
   {
      hVceCh->stStartEncodeSettings.hOutputHandle = hVceCh->stOutput.hVceOutput;
   }
   else
   {
      BDBG_ASSERT( 0 );
   }

   BVCE_Channel_S_ValidateSettings( &hVceCh->stStartEncodeSettings, &hVceCh->stEncodeSettings );

#if BVCE_P_TEST_MODE
   BVCE_Debug_P_CloseLog( hVceCh->hStatusLog );
   hVceCh->hStatusLog = NULL;
#endif

   BKNI_Memset( &stConfigChannelSettings, 0, sizeof( BVCE_P_SendCommand_ConfigChannel_Settings ) );

   stConfigChannelSettings.bFastChannelChange = hVceCh->stStartEncodeSettings.bAdaptiveLowDelayMode;

   /* Send Config Channel Command */
   rc = BVCE_S_SendCommand_ConfigChannel(
            hVceCh->hVce,
            hVceCh,
            &stConfigChannelSettings
            );
   if ( BERR_SUCCESS != rc )
   {
      BDBG_ERR(("Error configuring channel"));
      return BERR_TRACE( rc );
   }

   /* Reset Status */
   rc = BVCE_Channel_ClearStatus(
            hVceCh,
            NULL
            );
   if ( BERR_SUCCESS != rc )
   {
      BDBG_ERR(("Error clearing channel status"));
      return BERR_TRACE( rc );
   }

   BKNI_Memset( &hVceCh->picture.stState, 0, sizeof(hVceCh->picture.stState) );

   /* Send Start Channel Command */
   rc = BVCE_S_SendCommand_StartChannel(
            hVceCh->hVce,
            hVceCh
            );
   if ( BERR_SUCCESS != rc )
   {
      BDBG_ERR(("Error starting channel"));
      return BERR_TRACE( rc );
   }

   hVceCh->stStartEncodeSettings.hOutputHandle->state.hVceCh = hVceCh;
   /* SW7425-4186: Clear EOS event on start encode */
   hVceCh->stStatus.uiEventFlags &= ~BVCE_CHANNEL_STATUS_FLAGS_EVENT_EOS;

   /* Set Channel Status */
   hVceCh->eStatus = BVCE_P_Status_eStarted;

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BVCE_Channel_StartEncode(
         BVCE_Channel_Handle hVceCh,
         const BVCE_Channel_StartEncodeSettings *pstChStartEncodeSettings
         )
{
   BERR_Code rc;
   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);
   BDBG_ENTER( BVCE_Channel_StartEncode );
   BVCE_P_FUNCTION_TRACE_ENTER(0, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);

   BVCE_Power_P_AcquireResource(
         hVceCh->hVce,
         BVCE_Power_Type_eClock
         );

   rc = BVCE_Channel_S_StartEncode_impl(
         hVceCh,
         pstChStartEncodeSettings
         );

   if ( rc != BERR_SUCCESS )
   {
      BVCE_Power_P_ReleaseResource(
            hVceCh->hVce,
            BVCE_Power_Type_eClock
            );
   }

   BVCE_P_FUNCTION_TRACE_LEAVE(0, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);
   BDBG_LEAVE( BVCE_Channel_StartEncode );
   return BERR_TRACE( rc );
}

static const BVCE_Channel_StopEncodeSettings s_stDefaultStopEncodeSettings =
{
 BVCE_P_SIGNATURE_STOPENCODESETTINGS, /* Signature */
#if (BVCE_P_VDC_MANAGE_VIP)
 BVCE_Channel_StopMode_eImmediate     /* Stop Mode */
#else
 BVCE_Channel_StopMode_eNormal        /* Stop Mode */
#endif
};

void
BVCE_Channel_GetDefaultStopEncodeSettings(
         BVCE_Channel_StopEncodeSettings *pstChStopEncodeSettings
         )
{
   BDBG_ENTER( BVCE_Channel_GetDefaultStopEncodeSettings );

   BDBG_ASSERT( pstChStopEncodeSettings );

   *pstChStopEncodeSettings = s_stDefaultStopEncodeSettings;

   BDBG_LEAVE( BVCE_Channel_GetDefaultStopEncodeSettings );
   return;
}

/* BVCE_Channel_StopEncode - Stops the encode process.
 */
#define BVCE_P_STOP_RECOVERY_DELAY 10000
#define BVCE_P_STOP_RECOVERY_TIME 500000

#if (BVCE_P_VDC_MANAGE_VIP)
static
BERR_Code
BVCE_Channel_S_WaitForEOS(
   BVCE_Channel_Handle hVceCh
   )
{
   BERR_Code rc = BERR_SUCCESS;

   unsigned uiNumIterations = BVCE_P_STOP_RECOVERY_TIME / BVCE_P_STOP_RECOVERY_DELAY;

   rc = BVCE_Channel_GetStatus( hVceCh, &hVceCh->stStatus );
   while ( ( uiNumIterations != 0 )
           && ( BVCE_P_Status_eOpened != hVceCh->eStatus )
           && ( BERR_SUCCESS == rc )
         )
   {
      BKNI_Delay(BVCE_P_STOP_RECOVERY_DELAY);
      rc = BVCE_Channel_GetStatus( hVceCh, &hVceCh->stStatus );
      uiNumIterations--;
   }

   if ( 0 == ( hVceCh->stStatus.uiEventFlags & BVCE_CHANNEL_STATUS_FLAGS_EVENT_EOS ) )
   {
      rc = BERR_TRACE( BERR_UNKNOWN );
      BDBG_ERR(("VCE FW not done with stop, yet"));
   }

   BVCE_Channel_P_HandleEOSEvent(
      hVceCh
      );

   return BERR_TRACE( rc );
}
#endif

static
BERR_Code
BVCE_Channel_S_StopEncode_impl(
         BVCE_Channel_Handle hVceCh,
         const BVCE_Channel_StopEncodeSettings *pstChStopEncodeSettings
         )
{
   BERR_Code rc;

   if ( NULL == pstChStopEncodeSettings )
   {
      hVceCh->stStopEncodeSettings = s_stDefaultStopEncodeSettings;
   }
   else
   {
      BDBG_ASSERT( BVCE_P_SIGNATURE_STOPENCODESETTINGS == pstChStopEncodeSettings->uiSignature );
      hVceCh->stStopEncodeSettings = *pstChStopEncodeSettings;
   }

   switch ( hVceCh->eStatus )
   {
      case BVCE_P_Status_eIdle:
         BDBG_ERR(("Channel is not open"));
         BDBG_LEAVE( BVCE_Channel_StopEncode );
         return BERR_TRACE( BERR_UNKNOWN );

      case BVCE_P_Status_eOpened:
         BDBG_LEAVE( BVCE_Channel_StopEncode );
         return BERR_TRACE( BERR_SUCCESS );

      case BVCE_P_Status_eStarted:
         {
            BVCE_Channel_Status stSavedChannelStatus;
            BVCE_Channel_Status stChannelStatus;
            stSavedChannelStatus = hVceCh->stStatus;
            BKNI_Memset( &hVceCh->stStatus, 0, sizeof( hVceCh->stStatus ) );
            rc = BVCE_Channel_GetStatus( hVceCh, &stChannelStatus );
            hVceCh->stStatus = stSavedChannelStatus;
            if ( rc != BERR_SUCCESS )
            {
               BDBG_LEAVE( BVCE_Channel_StopEncode );
               return BERR_TRACE( rc );
            }

            if ( BVCE_P_Status_eOpened == hVceCh->eStatus )
            {
               BDBG_LEAVE( BVCE_Channel_StopEncode );
               return BERR_TRACE( BERR_SUCCESS );
            }

            if ( BVCE_Channel_StopMode_eNormal == hVceCh->stStopEncodeSettings.eStopMode )
            {
#if !(BVCE_P_VDC_MANAGE_VIP)
               if ( 0 != ( stChannelStatus.uiErrorFlags & BVCE_CHANNEL_STATUS_FLAGS_ERROR_CDB_FULL ) )
               {
                  BDBG_WRN(("CDB Full during stop...forcing immediate stop!"));
#endif
                  hVceCh->stStopEncodeSettings.eStopMode = BVCE_Channel_StopMode_eImmediate;
#if !(BVCE_P_VDC_MANAGE_VIP)
               }
#endif
            }
         }
         break;

      case BVCE_P_Status_eStopping:
         /* Continue with warning */
         BDBG_ERR(("Channel is already stopping"));
         BDBG_LEAVE( BVCE_Channel_StopEncode );
         return BERR_TRACE( BERR_UNKNOWN );

      default:
         BDBG_ERR(("Unknown state"));
         BDBG_LEAVE( BVCE_Channel_StopEncode );
         return BERR_TRACE( BERR_UNKNOWN );
   }

#if BVCE_P_TEST_MODE
   BVCE_Debug_P_CloseLog( hVceCh->hConfigLog );
   hVceCh->hConfigLog = NULL;
#endif

   /* Send Stop Channel Command */
   rc = BVCE_S_SendCommand_StopChannel(
            hVceCh->hVce,
            hVceCh
            );
   if ( BERR_SUCCESS != rc )
   {
      BDBG_ERR(("Error stopping channel"));
      return BERR_TRACE( rc );
   }

   /* Set Channel Status */
   hVceCh->eStatus = BVCE_P_Status_eStopping; /* State changes to eOpened when EOS ITB entry is seen */

#if BVCE_P_DUMP_USERDATA_LOG
   BVCE_Debug_P_CloseLog( hVceCh->userdata.hUserDataLog );
   hVceCh->userdata.hUserDataLog = NULL;
#endif

   if ( BVCE_Channel_StopMode_eAbort == hVceCh->stStopEncodeSettings.eStopMode )
   {
      BVCE_Channel_FlushEncode( hVceCh );
   }
#if (BVCE_P_VDC_MANAGE_VIP)
   else
   {
      BVCE_Channel_S_WaitForEOS( hVceCh );
   }
#endif

   return BERR_TRACE( rc );
}

BERR_Code
BVCE_Channel_StopEncode(
         BVCE_Channel_Handle hVceCh,
         const BVCE_Channel_StopEncodeSettings *pstChStopEncodeSettings
         )
{
   BERR_Code rc;
   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);
   BDBG_ENTER( BVCE_Channel_StopEncode );
   BVCE_P_FUNCTION_TRACE_ENTER(0, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);

   BVCE_Power_P_AcquireResource(
         hVceCh->hVce,
         BVCE_Power_Type_eClock
         );

   rc = BVCE_Channel_S_StopEncode_impl(
      hVceCh,
      pstChStopEncodeSettings
      );

   BVCE_Power_P_ReleaseResource(
         hVceCh->hVce,
         BVCE_Power_Type_eClock
         );

   BVCE_P_FUNCTION_TRACE_LEAVE(0, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);
   BDBG_LEAVE( BVCE_Channel_StopEncode );
   return BERR_TRACE( rc );
}

static
void
BVCE_Channel_S_FlushEncode(
         BVCE_Channel_Handle hVceCh
         )
{
   if ( ( NULL != hVceCh->stStartEncodeSettings.hOutputHandle )
        && ( BVCE_P_Status_eOpened != hVceCh->eStatus ) )
   {
      BVCE_Output_Flush( hVceCh->stStartEncodeSettings.hOutputHandle );
   }
}

static
BERR_Code
BVCE_Channel_S_FlushEncode_impl(
   BVCE_Channel_Handle hVceCh
   )
{
   BERR_Code rc = BERR_SUCCESS;

   if ( BVCE_P_Status_eStopping == hVceCh->eStatus )
   {
      unsigned uiNumIterations = BVCE_P_STOP_RECOVERY_TIME / BVCE_P_STOP_RECOVERY_DELAY;

      rc = BVCE_Channel_GetStatus( hVceCh, &hVceCh->stStatus );
      while ( ( uiNumIterations != 0 )
              && ( BVCE_P_Status_eOpened != hVceCh->eStatus )
              && ( BERR_SUCCESS == rc )
            )
      {
         BVCE_Channel_S_FlushEncode( hVceCh );
         BKNI_Delay(BVCE_P_STOP_RECOVERY_DELAY);
         rc = BVCE_Channel_GetStatus( hVceCh, &hVceCh->stStatus );
         uiNumIterations--;
      }

      if ( 0 == ( hVceCh->stStatus.uiEventFlags & BVCE_CHANNEL_STATUS_FLAGS_EVENT_EOS ) )
      {
         rc = BERR_TRACE( BERR_UNKNOWN );
         BDBG_ERR(("VCE FW not done with stop, yet"));
      }

      if ( ( NULL != hVceCh->stStartEncodeSettings.hOutputHandle )
           && ( BVCE_P_Status_eOpened != hVceCh->eStatus )
           && ( BVCE_P_Output_BufferAccessMode_eDirect != hVceCh->stStartEncodeSettings.hOutputHandle->state.eBufferAccessMode )
          )
      {
         rc = BERR_TRACE( BERR_UNKNOWN );
         BDBG_ERR(("EOS not seen in ITB, yet"));
      }

      BVCE_Channel_P_HandleEOSEvent(
         hVceCh
         );
   }
   else
   {
      BVCE_Channel_S_FlushEncode( hVceCh );
   }

   return BERR_TRACE( rc );
}

BERR_Code
BVCE_Channel_FlushEncode(
   BVCE_Channel_Handle hVceCh
   )
{
   BERR_Code rc;
   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);
   BDBG_ENTER( BVCE_Channel_FlushEncode );
   BVCE_P_FUNCTION_TRACE_ENTER(0, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);

   BVCE_Power_P_AcquireResource(
         hVceCh->hVce,
         BVCE_Power_Type_eClock
         );

   rc = BVCE_Channel_S_FlushEncode_impl( hVceCh );

   BVCE_Power_P_ReleaseResource(
         hVceCh->hVce,
         BVCE_Power_Type_eClock
         );

   BVCE_P_FUNCTION_TRACE_LEAVE(0, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);
   BDBG_LEAVE( BVCE_Channel_FlushEncode );
   return BERR_TRACE( rc );
}
/********************/
/* Channel Settings */
/********************/

static const BVCE_Channel_EncodeSettings s_stDefaultChEncodeSettings =
{
 BVCE_P_SIGNATURE_CHANNELENCODESETTINGS, /* Signature */
 /* Frame Rate */
 {
  BAVC_FrameRateCode_e30,  /* 30 fps */
  false, /* variable frame rate mode=off*/
  false, /* sparse frame rate mode=off*/
 },
 {
  6000000, /* Max Bit Rate */
  0, /* Target Bit Rate */
 },
 0, /* end-to-end delay */
 {
  false, /* bAllowNewGOPOnSceneChange */
  20,    /* uiMinGOPLengthAfterSceneChange */
  0,     /* uiDuration */
  0,     /* uiDurationRampUpFactor */
  14,    /* uiNumberOfPFrames */
  0,     /* uiNumberOfBFrames */
  false, /* bAllowOpenGop = false */
 },
 true, /* bITFPEnable */
 0, /* uiNumSlicesPerPic */
};

void
BVCE_Channel_GetDefaultEncodeSettings(
         const BBOX_Handle hBox,
         BVCE_Channel_EncodeSettings *pstChEncodeSettings
         )
{
   BDBG_ENTER( BVCE_Channel_GetDefaultEncodeSettings );

   BDBG_ASSERT( pstChEncodeSettings );

   if ( NULL == pstChEncodeSettings )
   {
      return;
   }

   *pstChEncodeSettings = s_stDefaultChEncodeSettings;

   BVCE_Platform_P_OverrideChannelDefaultEncodeSettings( hBox, pstChEncodeSettings );

   BDBG_LEAVE( BVCE_Channel_GetDefaultEncodeSettings );
   return;
}

static
BERR_Code
BVCE_Channel_S_SetEncodeSettings_impl(
         BVCE_Channel_Handle hVceCh,
         const BVCE_Channel_EncodeSettings *pstChEncodeSettings
         )
{
   BERR_Code rc;
   BVCE_P_SendCommand_ConfigChannel_Settings stConfigChannelSettings;
   bool bConfigNow = true;
   BVCE_Channel_EncodeSettings stNewChEncodeSettings;

   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);
   BDBG_ASSERT( pstChEncodeSettings );

   BKNI_Memset( &stConfigChannelSettings, 0, sizeof( BVCE_P_SendCommand_ConfigChannel_Settings ) );
   stNewChEncodeSettings = *pstChEncodeSettings;

   switch ( hVceCh->eStatus )
   {
      case BVCE_P_Status_eIdle:
         BDBG_ERR(("Channel is not open"));
         return BERR_TRACE( BERR_UNKNOWN );

      case BVCE_P_Status_eOpened:
         bConfigNow = false; /* Channel will be configured at StartEncode */
         break;

      case BVCE_P_Status_eStarted:
         bConfigNow = true; /* Channel is started, so needs to configure only dynamic parameters */
         /* SW7425-2078: Set Begin New RAP if GOP structure changes */
         if ( ( stNewChEncodeSettings.stGOPStructure.bAllowOpenGOP != hVceCh->stEncodeSettings.stGOPStructure.bAllowOpenGOP )
              || ( stNewChEncodeSettings.stGOPStructure.uiNumberOfBFrames != hVceCh->stEncodeSettings.stGOPStructure.uiNumberOfBFrames )
              || ( stNewChEncodeSettings.stGOPStructure.uiNumberOfPFrames != hVceCh->stEncodeSettings.stGOPStructure.uiNumberOfPFrames )
            )
         {
            stConfigChannelSettings.bBeginNewRAP = true;
         }

         /* SW7425-5268: Verify that bITFPEnable and uiA2PDelay do not change since they are no longer dynamic  */
         if ( 0 == stNewChEncodeSettings.uiA2PDelay )
         {
            /* SW7435-1231: If the uiA2PDelay setting is 0, then the value was calculated
             * by VCE PI during the StartEncode, so we need to re-use the same value.
             */
            stNewChEncodeSettings.uiA2PDelay = hVceCh->stEncodeSettings.uiA2PDelay;
         }
         else if ( stNewChEncodeSettings.uiA2PDelay != hVceCh->stEncodeSettings.uiA2PDelay )
         {
            BDBG_WRN(("BVCE_Channel_EncodeSettings.uiA2PDelay cannot be changed during an encode"));
            return BERR_TRACE( BERR_NOT_SUPPORTED );
         }

         if ( stNewChEncodeSettings.bITFPEnable != hVceCh->stEncodeSettings.bITFPEnable )
         {
            BDBG_WRN(("BVCE_Channel_EncodeSettings.bITFPEnable cannot be changed during an encode"));
            return BERR_TRACE( BERR_NOT_SUPPORTED );
         }

         break;

      case BVCE_P_Status_eStopping:
         /* Continue with warning */
         if ( BVCE_Channel_StopMode_eNormal == hVceCh->stStopEncodeSettings.eStopMode )
         {
            BDBG_WRN(("Channel not completed stopping, will be configured when re-started"));
         }
         bConfigNow = false; /* Channel will be configured at StartEncode */
         break;

      default:
         BDBG_ERR(("Unknown state"));
         return BERR_TRACE( BERR_UNKNOWN );
   }

   hVceCh->stEncodeSettings = stNewChEncodeSettings;

   if ( true == bConfigNow )
   {
      if ( true == hVceCh->bPendingEncodeSettings )
      {
         BDBG_WRN(("BVCE_Channel_SetEncodeSettings() - There are pending settings that have not, yet, been applied!"));
      }

      /* Send Config Channel Command */
      rc = BVCE_S_SendCommand_ConfigChannel(
               hVceCh->hVce,
               hVceCh,
               &stConfigChannelSettings
               );

      if ( BERR_SUCCESS != rc )
      {
         BDBG_ERR(("Error configuring channel"));
         return BERR_TRACE( rc );
      }

      if ( true == hVceCh->bPendingEncodeSettings )
      {
         hVceCh->bPendingEncodeSettings = false;
      }
   }

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BVCE_Channel_SetEncodeSettings(
         BVCE_Channel_Handle hVceCh,
         const BVCE_Channel_EncodeSettings *pstChEncodeSettings
         )
{
   BERR_Code rc;
   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);
   BDBG_ENTER( BVCE_Channel_SetEncodeSettings );
   BVCE_P_FUNCTION_TRACE_ENTER(0, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);

   BVCE_Power_P_AcquireResource(
         hVceCh->hVce,
         BVCE_Power_Type_eClock
         );

   rc = BVCE_Channel_S_SetEncodeSettings_impl( hVceCh, pstChEncodeSettings );

   BVCE_Power_P_ReleaseResource(
         hVceCh->hVce,
         BVCE_Power_Type_eClock
         );

   BVCE_P_FUNCTION_TRACE_LEAVE(0, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);
   BDBG_LEAVE( BVCE_Channel_SetEncodeSettings );
   return BERR_TRACE( rc );
}

static
BERR_Code
BVCE_Channel_S_GetEncodeSettings_impl(
         BVCE_Channel_Handle hVceCh,
         BVCE_Channel_EncodeSettings *pstChEncodeSettings
         )
{
   BERR_Code rc;

   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);
   BDBG_ASSERT( pstChEncodeSettings );

   /* We get status to make sure we get any pending settings that were applied first */
   rc = BVCE_Channel_GetStatus( hVceCh, NULL );

   *pstChEncodeSettings = hVceCh->stEncodeSettings;

   return BERR_TRACE( rc );
}

BERR_Code
BVCE_Channel_GetEncodeSettings(
         BVCE_Channel_Handle hVceCh,
         BVCE_Channel_EncodeSettings *pstChEncodeSettings
         )
{
   BERR_Code rc;
   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);
   BDBG_ENTER( BVCE_Channel_GetEncodeSettings );
   BVCE_P_FUNCTION_TRACE_ENTER(0, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);

   BVCE_Power_P_AcquireResource(
         hVceCh->hVce,
         BVCE_Power_Type_eClock
         );

   rc = BVCE_Channel_S_GetEncodeSettings_impl( hVceCh, pstChEncodeSettings );

   BVCE_Power_P_ReleaseResource(
         hVceCh->hVce,
         BVCE_Power_Type_eClock
         );

   BVCE_P_FUNCTION_TRACE_LEAVE(0, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);
   BDBG_LEAVE( BVCE_Channel_GetEncodeSettings );
   return BERR_TRACE( rc );
}

static
BERR_Code
BVCE_Channel_S_GetDefaultEncodeSettings_OnInputChange(
         BVCE_Channel_Handle hVceCh,
         BVCE_Channel_EncodeSettings_OnInputChange *pstChEncodeSettings
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);
   BDBG_ASSERT( pstChEncodeSettings );

   /* We get status to make sure we get any pending settings that were applied first */
   rc = BVCE_Channel_GetStatus( hVceCh, NULL );

   BKNI_Memset( pstChEncodeSettings,
                0,
                sizeof( BVCE_Channel_EncodeSettings_OnInputChange )
              );

   pstChEncodeSettings->uiSignature = BVCE_P_SIGNATURE_CHANNELENCODESETTINGSONINPUTCHANGE;

   pstChEncodeSettings->stBitRate = hVceCh->stEncodeSettings.stBitRate;

   return BERR_TRACE( rc );
}

static
BERR_Code
BVCE_Channel_S_BeginNewRAP_impl(
         BVCE_Channel_Handle hVceCh
         )
{
   BERR_Code rc;
   BVCE_P_SendCommand_ConfigChannel_Settings stConfigChannelSettings;

   if ( BVCE_P_Status_eStarted != hVceCh->eStatus )
   {
      BDBG_ERR(("Channel must be started to insert a RAP point"));
      return BERR_TRACE( BERR_UNKNOWN );
   }

   BKNI_Memset( &stConfigChannelSettings, 0, sizeof( BVCE_P_SendCommand_ConfigChannel_Settings ) );

   stConfigChannelSettings.bBeginNewRAP = true;

   /* Send Config Channel Command */
   rc = BVCE_S_SendCommand_ConfigChannel(
            hVceCh->hVce,
            hVceCh,
            &stConfigChannelSettings
            );

   if ( BERR_SUCCESS != rc )
   {
      BDBG_ERR(("BVCE_Channel_BeginNewRAP: Error configuring channel"));
   }

   return BERR_TRACE( rc );
}

BERR_Code
BVCE_Channel_BeginNewRAP(
         BVCE_Channel_Handle hVceCh
         )
{
   BERR_Code rc;
   BDBG_ENTER( BVCE_Channel_BeginNewRAP );
   BVCE_P_FUNCTION_TRACE_ENTER(0, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);

   BVCE_Power_P_AcquireResource(
         hVceCh->hVce,
         BVCE_Power_Type_eClock
         );

   rc = BVCE_Channel_S_BeginNewRAP_impl( hVceCh );

   BVCE_Power_P_ReleaseResource(
         hVceCh->hVce,
         BVCE_Power_Type_eClock
         );

   BVCE_P_FUNCTION_TRACE_LEAVE(0, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);
   BDBG_LEAVE( BVCE_Channel_BeginNewRAP );
   return BERR_TRACE( rc );
}

static
BERR_Code
BVCE_Channel_S_SetEncodeSettings_OnInputChange_impl(
         BVCE_Channel_Handle hVceCh,
         const BVCE_Channel_EncodeSettings_OnInputChange *pstChEncodeSettings
         )
{
   BERR_Code rc;
   bool bConfigNow = true;
   BVCE_P_SendCommand_ConfigChannel_Settings stConfigChannelSettings;

   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);
   BDBG_ASSERT( pstChEncodeSettings );
   BDBG_ASSERT( BVCE_P_SIGNATURE_CHANNELENCODESETTINGSONINPUTCHANGE == pstChEncodeSettings->uiSignature );

   BKNI_Memset( &stConfigChannelSettings, 0, sizeof( BVCE_P_SendCommand_ConfigChannel_Settings ) );

   switch ( hVceCh->eStatus )
   {
      case BVCE_P_Status_eIdle:
         BDBG_ERR(("Channel is not open"));
         return BERR_TRACE( BERR_UNKNOWN );

      case BVCE_P_Status_eOpened:
         BDBG_ERR(("Channel not started, will be configured immediately"));
         bConfigNow = false; /* Channel will be configured at StartEncode */
         break;

      case BVCE_P_Status_eStarted:
         bConfigNow = true; /* Channel is started, so needs to configure only dynamic parameters */
         break;

      case BVCE_P_Status_eStopping:
         /* Continue with warning */
         BDBG_WRN(("Channel not completed stopping, will be configured when re-started"));
         bConfigNow = false; /* Channel will be configured at StartEncode */
         break;

      default:
         BDBG_ERR(("Unknown state"));
         return BERR_TRACE( BERR_UNKNOWN );
   }

   hVceCh->stPendingEncodeSettings = hVceCh->stEncodeSettings;
   if ( hVceCh->stPendingEncodeSettings.stBitRate.uiMax == pstChEncodeSettings->stBitRate.uiMax )
   {
      /* settings haven't changed, so no need to send config command */
      bConfigNow = false;
   }
   else
   {
      hVceCh->stPendingEncodeSettings.stBitRate = pstChEncodeSettings->stBitRate;
   }

   if ( true == bConfigNow )
   {

      if ( true == hVceCh->bPendingEncodeSettings )
      {
         BDBG_WRN(("BVCE_Channel_SetEncodeSettings_OnInputChange() - There are pending settings that have not, yet, been applied"));
      }

      stConfigChannelSettings.bOnInputChange = true;

      /* Send Config Channel Command */
      rc = BVCE_S_SendCommand_ConfigChannel(
               hVceCh->hVce,
               hVceCh,
               &stConfigChannelSettings
               );
      if ( BERR_SUCCESS != rc )
      {
         BDBG_ERR(("Error configuring channel"));
         return BERR_TRACE( rc );
      }

      hVceCh->bPendingEncodeSettings = true;
   }

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BVCE_Channel_SetEncodeSettings_OnInputChange(
         BVCE_Channel_Handle hVceCh,
         const BVCE_Channel_EncodeSettings_OnInputChange *pstChEncodeSettings
         )
{
   BERR_Code rc;
   BDBG_ENTER( BVCE_Channel_SetEncodeSettings_OnInputChange );
   BVCE_P_FUNCTION_TRACE_ENTER(0, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);

   BVCE_Power_P_AcquireResource(
         hVceCh->hVce,
         BVCE_Power_Type_eClock
         );

   rc = BVCE_Channel_S_SetEncodeSettings_OnInputChange_impl( hVceCh, pstChEncodeSettings );

   BVCE_Power_P_ReleaseResource(
         hVceCh->hVce,
         BVCE_Power_Type_eClock
         );

   BVCE_P_FUNCTION_TRACE_LEAVE(0, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);
   BDBG_LEAVE( BVCE_Channel_SetEncodeSettings_OnInputChange );
   return BERR_TRACE( rc );
}
BERR_Code
BVCE_Channel_GetEncodeSettings_OnInputChange(
         BVCE_Channel_Handle hVceCh,
         BVCE_Channel_EncodeSettings_OnInputChange *pstChEncodeSettings
         )
{
   BERR_Code rc;
   BDBG_ENTER( BVCE_Channel_GetEncodeSettings_OnInputChange );
   BVCE_P_FUNCTION_TRACE_ENTER(0, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);

   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);
   BDBG_ASSERT( pstChEncodeSettings );

   rc = BVCE_Channel_S_GetDefaultEncodeSettings_OnInputChange( hVceCh, pstChEncodeSettings );

   if ( true == hVceCh->bPendingEncodeSettings )
   {
      pstChEncodeSettings->stBitRate = hVceCh->stPendingEncodeSettings.stBitRate;
   }

   BVCE_P_FUNCTION_TRACE_LEAVE(0, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);
   BDBG_LEAVE( BVCE_Channel_GetEncodeSettings_OnInputChange );

   return BERR_TRACE( rc );
}

/******************/
/* Channel Status */
/******************/
static
BERR_Code
BVCE_Channel_S_GetStatus_impl(
         BVCE_Channel_Handle hVceCh,
         BVCE_Channel_Status *pChannelStatus
         )
{
   BERR_Code rc;

   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);

   /* Get Updated Channel Status from FW */
   rc = BVCE_S_SendCommand_GetChannelStatus(
            hVceCh->hVce,
            hVceCh
            );
   BERR_TRACE( rc );

   if ( BERR_SUCCESS != rc )
   {
      BDBG_ERR(("Error getting channel status"));
   }
   else
   {
      /* Update local copy of channel status */

      /* Set Event/Error Flags */
      {
         uint32_t uiFlags;
         uint8_t uiIndex;

         uiFlags = hVceCh->hVce->fw.stResponse.type.stGetChannelStatus.StatusInfoStruct.uiEventFlags;
         uiIndex = 0;

         while ( uiFlags )
         {
            if ( 0 != ( uiFlags & 0x1 ) )
            {
               hVceCh->stStatus.uiEventFlags |= BVCE_P_EventMaskReverseLUT[uiIndex];

               /* Process Pending Changes */
               if ( ( true == hVceCh->bPendingEncodeSettings )
                    && ( BVCE_CHANNEL_STATUS_FLAGS_EVENT_INPUT_CHANGE == BVCE_P_EventMaskReverseLUT[uiIndex] )
                  )
               {
                  hVceCh->stEncodeSettings = hVceCh->stPendingEncodeSettings;
                  hVceCh->bPendingEncodeSettings = false;
               }

            }

            uiFlags >>= 1;
            uiIndex++;
         }

         uiFlags = hVceCh->hVce->fw.stResponse.type.stGetChannelStatus.StatusInfoStruct.uiEventFlags;
         uiIndex = 0;

         while ( uiFlags )
         {
            if ( 0 != ( uiFlags & 0x1 ) )
            {
               hVceCh->stStatus.uiErrorFlags |= BVCE_P_ErrorMaskReverseLUT[uiIndex];
            }

            uiFlags >>= 1;
            uiIndex++;
         }

         /* Mask CDB Full "error" when in NRT mode */
         if ( true == hVceCh->stStartEncodeSettings.bNonRealTimeEncodeMode )
         {
            hVceCh->stStatus.uiErrorFlags &= ~BVCE_CHANNEL_STATUS_FLAGS_ERROR_CDB_FULL;
         }
      }

      hVceCh->stStatus.uiTotalErrors = hVceCh->hVce->fw.stResponse.type.stGetChannelStatus.StatusInfoStruct.uiTotalErrors;
      hVceCh->stStatus.uiTotalPicturesReceived = hVceCh->hVce->fw.stResponse.type.stGetChannelStatus.StatusInfoStruct.NumOfPicsMbArcFinished
              + hVceCh->hVce->fw.stResponse.type.stGetChannelStatus.StatusInfoStruct.NumOfPicsVipDroppedDueToFRC
              + hVceCh->hVce->fw.stResponse.type.stGetChannelStatus.StatusInfoStruct.NumOfPicsVipDroppedDueToPerformance
              + hVceCh->hVce->fw.stResponse.type.stGetChannelStatus.StatusInfoStruct.NumOfPicsVipDroppedDueToHRDUnderFlow
              + hVceCh->picture.stState.stats.uiNumDroppedDueToError
              + hVceCh->picture.stState.stats.uiNumDroppedDueToFRC;
      hVceCh->stStatus.uiTotalPicturesDroppedDueToFrameRateConversion = hVceCh->hVce->fw.stResponse.type.stGetChannelStatus.StatusInfoStruct.NumOfPicsVipDroppedDueToFRC + hVceCh->picture.stState.stats.uiNumDroppedDueToFRC;
      hVceCh->stStatus.uiTotalPicturesDroppedDueToErrors = hVceCh->hVce->fw.stResponse.type.stGetChannelStatus.StatusInfoStruct.NumOfPicsVipDroppedDueToPerformance + hVceCh->picture.stState.stats.uiNumDroppedDueToError;
      hVceCh->stStatus.uiTotalPicturesEncoded = hVceCh->hVce->fw.stResponse.type.stGetChannelStatus.StatusInfoStruct.NumOfPicsMbArcFinished;
      hVceCh->stStatus.uiLastPictureIdEncoded = hVceCh->hVce->fw.stResponse.type.stGetChannelStatus.StatusInfoStruct.PicId;
      hVceCh->stStatus.uiSTCSnapshot = hVceCh->hVce->fw.stResponse.type.stGetChannelStatus.StatusInfoStruct.uiSTCSnapshot;
      hVceCh->stStatus.uiTotalPicturesDroppedDueToHRDUnderflow = hVceCh->hVce->fw.stResponse.type.stGetChannelStatus.StatusInfoStruct.NumOfPicsVipDroppedDueToHRDUnderFlow;
      hVceCh->stStatus.uiEtsDtsOffset = hVceCh->hVce->fw.stResponse.type.stGetChannelStatus.StatusInfoStruct.uiEtsDtsOffset;
      if ( 0 != hVceCh->hVce->fw.stResponse.type.stGetChannelStatus.StatusInfoStruct.Throughput )
      {
         hVceCh->stStatus.uiAverageFramesPerSecond = BVCE_P_CORE_FREQUENCY / hVceCh->hVce->fw.stResponse.type.stGetChannelStatus.StatusInfoStruct.Throughput;
      }
      else
      {
         hVceCh->stStatus.uiAverageFramesPerSecond = 0;
      }
   }

   /* Check if stop has already occurred */
   if ( 0 != ( hVceCh->stStatus.uiEventFlags & BVCE_CHANNEL_STATUS_FLAGS_EVENT_EOS ) )
   {
      BVCE_Channel_P_HandleEOSEvent( hVceCh );
   }

   if ( NULL != pChannelStatus )
   {
      *pChannelStatus = hVceCh->stStatus;

#if BVCE_P_TEST_MODE
   if ( NULL == hVceCh->hStatusLog )
   {
      char fname[256];
      static unsigned uiStatusLogInstance[BVCE_PLATFORM_P_NUM_ENCODE_INSTANCES][BVCE_PLATFORM_P_NUM_ENCODE_CHANNELS];
      sprintf(fname, "BVCE_STATUS_%02d_%02d_%03d.csv", hVceCh->hVce->stOpenSettings.uiInstance, hVceCh->stOpenSettings.uiInstance, uiStatusLogInstance[hVceCh->hVce->stOpenSettings.uiInstance][hVceCh->stOpenSettings.uiInstance]);
      uiStatusLogInstance[hVceCh->hVce->stOpenSettings.uiInstance][hVceCh->stOpenSettings.uiInstance]++;

      if ( false == BVCE_Debug_P_OpenLog( fname, &hVceCh->hStatusLog ) )
      {
         BDBG_ERR(("Error Creating BVCE Channel Status Dump File (%s)", fname));
      }
      else
      {
         BVCE_Debug_P_WriteLog_isr( hVceCh->hStatusLog, "count,error flags,event flags,total errors,received,dropped (frc),dropped (err),encoded,stc snapshot,dropped (hrd underflow),ets/dts offset (27Mhz),fps");
         BVCE_Debug_P_WriteLog_isr( hVceCh->hStatusLog, "\n" );
      }
   }

   if ( NULL != hVceCh->hStatusLog )
   {
      BDBG_CWARNING( sizeof( hVceCh->stStatus ) == (12*4 + 1*8));

      /* Start Encode Settings */
      BVCE_Debug_P_WriteLog_isr( hVceCh->hStatusLog, "%u,%u,%u,%u,%u,%u,%u,%u,%llu,%u,%u,%u",
         ( NULL == hVceCh->stStartEncodeSettings.hOutputHandle ) ? (unsigned)-1 : hVceCh->stStartEncodeSettings.hOutputHandle->uiDescriptorCount,
         hVceCh->stStatus.uiErrorFlags,
         hVceCh->stStatus.uiEventFlags,
         hVceCh->stStatus.uiTotalErrors,
         hVceCh->stStatus.uiTotalPicturesReceived,
         hVceCh->stStatus.uiTotalPicturesDroppedDueToFrameRateConversion,
         hVceCh->stStatus.uiTotalPicturesDroppedDueToErrors,
         hVceCh->stStatus.uiTotalPicturesEncoded,
         hVceCh->stStatus.uiSTCSnapshot,
         hVceCh->stStatus.uiTotalPicturesDroppedDueToHRDUnderflow,
         hVceCh->stStatus.uiEtsDtsOffset,
         hVceCh->stStatus.uiAverageFramesPerSecond
      );

      BVCE_Debug_P_WriteLog_isr( hVceCh->hStatusLog, "\n" );
   }
#endif
      if ( NULL != hVceCh->hVce->stDebugFifo.hDebugFifo )
      {
         BVCE_P_DebugFifo_Entry *pstEntry;
         BDBG_Fifo_Token stToken;

         pstEntry = (BVCE_P_DebugFifo_Entry *) BDBG_Fifo_GetBuffer( hVceCh->hVce->stDebugFifo.hDebugFifo, &stToken );
         if ( NULL != pstEntry )
         {
            pstEntry->stMetadata.eType = BVCE_DebugFifo_EntryType_eStatus;
            pstEntry->stMetadata.uiInstance = hVceCh->hVce->stOpenSettings.uiInstance;
            pstEntry->stMetadata.uiChannel = hVceCh->stOpenSettings.uiInstance;
            pstEntry->stMetadata.uiTimestamp = 0;
            ( NULL != hVceCh->hVce->hTimer ) ? BTMR_ReadTimer( hVceCh->hVce->hTimer, &pstEntry->stMetadata.uiTimestamp ) : 0;
            pstEntry->data.stStatus = hVceCh->stStatus;
            BDBG_Fifo_CommitBuffer( &stToken );
         }
      }
   }

   return BERR_TRACE( rc );
}

BERR_Code
BVCE_Channel_GetStatus(
         BVCE_Channel_Handle hVceCh,
         BVCE_Channel_Status *pChannelStatus
         )
{
   BERR_Code rc;
   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);
   BDBG_ENTER( BVCE_Channel_GetStatus );
   BVCE_P_FUNCTION_TRACE_ENTER(1, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);

   BVCE_Power_P_AcquireResource(
         hVceCh->hVce,
         BVCE_Power_Type_eClock
         );

   rc = BVCE_Channel_S_GetStatus_impl( hVceCh, pChannelStatus );

   BVCE_Power_P_ReleaseResource(
         hVceCh->hVce,
         BVCE_Power_Type_eClock
         );

   BVCE_P_FUNCTION_TRACE_LEAVE(1, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);
   BDBG_LEAVE( BVCE_Channel_GetStatus );
   return BERR_TRACE( rc );
}

BERR_Code
BVCE_Channel_ClearStatus(
         BVCE_Channel_Handle hVceCh,
         const BVCE_Channel_Status *pChannelStatus
         )
{
   BDBG_ENTER( BVCE_Channel_ClearStatus );

   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);
   BVCE_P_FUNCTION_TRACE_ENTER(1, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);

   if ( NULL != pChannelStatus )
   {
      hVceCh->stStatus.uiErrorFlags &= ~pChannelStatus->uiErrorFlags;
      hVceCh->stStatus.uiEventFlags &= ~pChannelStatus->uiEventFlags;

      BDBG_CWARNING( sizeof( BVCE_Channel_Status ) == 12*4 + 1*8 );
#define BVCE_STATUS_FIELD(_field) \
      if ( 0 != pChannelStatus->_field )\
      {\
         hVceCh->stStatus._field = 0;\
      }
#include "bvce_status.inc"
   }
   else
   {
      BKNI_Memset(
               &hVceCh->stStatus,
               0,
               sizeof( BVCE_Channel_Status )
               );
   }

   BVCE_P_FUNCTION_TRACE_LEAVE(1, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);
   BDBG_LEAVE( BVCE_Channel_ClearStatus );
   return BERR_TRACE( BERR_SUCCESS );
}

#define BVCE_UserData_P_Set32_LE( pBuffer, offset, data ) \
{\
   ((uint8_t*)(pBuffer))[offset+0] = (uint8_t) ((data) >> 0 ) & 0xFF; \
   ((uint8_t*)(pBuffer))[offset+1] = (uint8_t) ((data) >> 8 ) & 0xFF; \
   ((uint8_t*)(pBuffer))[offset+2] = (uint8_t) ((data) >> 16) & 0xFF; \
   ((uint8_t*)(pBuffer))[offset+3] = (uint8_t) ((data) >> 24) & 0xFF; \
}

#define BVCE_UserData_P_Set16_LE( pBuffer, offset, data ) \
{\
   ((uint8_t*)(pBuffer))[offset+0] = (uint8_t) ((data) >> 0 ) & 0xFF; \
   ((uint8_t*)(pBuffer))[offset+1] = (uint8_t) ((data) >> 8 ) & 0xFF; \
}

#define BVCE_UserData_P_Get16_LE( pBuffer, offset ) ((((uint16_t) ((uint8_t*)(pBuffer))[offset+1]) << 8) | ((uint8_t*)(pBuffer))[offset+0])

#define BVCE_UserData_P_Set8( pBuffer, offset, data ) \
{\
   ((uint8_t*)(pBuffer))[offset+0] = (uint8_t) (data); \
}

#define BVCE_FW_P_UserData_PacketDescriptor_Length_OFFSET 0
#define BVCE_FW_P_UserData_PacketDescriptor_Type_OFFSET 2
#define BVCE_FW_P_UserData_PacketDescriptor_CCPayload_OFFSET 4

#define BVCE_FW_P_UserData_CCPayload_NumValid608Lines_OFFSET 0
#define BVCE_FW_P_UserData_CCPayload_608Data_OFFSET (BVCE_FW_P_UserData_CCPayload_NumValid608Lines_OFFSET + 1)
#define BVCE_FW_P_UserData_CCPayload_Get608Line_OFFSET(ui608Line) ( BVCE_FW_P_UserData_CCPayload_608Data_OFFSET + ((1 + BVCE_FW_P_UserData_Payload_CC_608_BYTES_PER_LINE_MAX) * ui608Line ))

#define BVCE_FW_P_UserData_CCPayload_NumValid708Lines_OFFSET BVCE_FW_P_UserData_Payload_CC_608_LENGTH
#define BVCE_FW_P_UserData_CCPayload_708Data_OFFSET (BVCE_FW_P_UserData_CCPayload_NumValid708Lines_OFFSET+1)
#define BVCE_FW_P_UserData_CCPayload_Get708Line_OFFSET(ui708Line) ( BVCE_FW_P_UserData_CCPayload_708Data_OFFSET + ((1 + BVCE_FW_P_UserData_Payload_CC_708_BYTES_PER_LINE_MAX) * ui708Line ))


#define BVCE_P_UserData_PacketType_UNSUPPORTED 0xFFFF

static const uint16_t BVCE_P_UserData_PacketTypeLUT[BUDP_DCCparse_Format_LAST] =
{
   BVCE_P_UserData_PacketType_UNSUPPORTED, /* BUDP_DCCparse_Format_Unknown */
   BVCE_FW_P_UserData_PacketType_eSCTE_20, /* BUDP_DCCparse_Format_DVS157 */
   BVCE_FW_P_UserData_PacketType_eATSC_A53, /* BUDP_DCCparse_Format_ATSC53 */
   BVCE_FW_P_UserData_PacketType_eSCTE_21, /* BUDP_DCCparse_Format_DVS053 */
   BVCE_P_UserData_PacketType_UNSUPPORTED, /* BUDP_DCCparse_Format_SEI */
   BVCE_P_UserData_PacketType_UNSUPPORTED, /* BUDP_DCCparse_Format_SEI2 */
};

static void
BVCE_UserData_P_ParsePacketDescriptor_isrsafe(
   const BUDP_Encoder_PacketDescriptor *pPacketDescriptor,
   void *pFWPacketDescriptor
   )
{
   unsigned i;
   volatile void * pCCPayload = (void*) ((uint8_t*) pFWPacketDescriptor + BVCE_FW_P_UserData_PacketDescriptor_CCPayload_OFFSET);

   if ( NULL == pFWPacketDescriptor ) return;
   BKNI_Memset( pFWPacketDescriptor, 0, BVCE_FW_P_UserData_PacketDescriptor_MAX_LENGTH );

   BDBG_ASSERT( BVCE_P_UserData_PacketType_UNSUPPORTED != BVCE_P_UserData_PacketTypeLUT[pPacketDescriptor->ePacketFormat] );

   BVCE_UserData_P_Set16_LE(pFWPacketDescriptor, BVCE_FW_P_UserData_PacketDescriptor_Length_OFFSET, BVCE_FW_P_UserData_PacketDescriptor_MAX_LENGTH);
   BVCE_UserData_P_Set16_LE(pFWPacketDescriptor, BVCE_FW_P_UserData_PacketDescriptor_Type_OFFSET, BVCE_P_UserData_PacketTypeLUT[pPacketDescriptor->ePacketFormat]);

   {
      unsigned uiNumValid608Lines = 0;
      unsigned uiNumValid708Lines = 0;
      bool bMax608LinesExceededWarningPrinted = false;
      bool bMax708LinesExceededWarningPrinted = false;

      for (i = 0; i < pPacketDescriptor->data.stDvs157.stCC.uiNumLines; i++ )
      {
         if ( true == pPacketDescriptor->data.stDvs157.stCC.astLine[i].bIsAnalog )
         {
            /* Process 608 Data */
            if ( uiNumValid608Lines > BVCE_FW_P_UserData_Payload_CC_608_LINES_PER_FIELD_MAX )
            {
               if ( false == bMax608LinesExceededWarningPrinted )
               {
                  BDBG_WRN(("Error: Number of Valid 608 Lines (%d) is greater than max (%d). Dropping extra lines.", uiNumValid608Lines, BVCE_FW_P_UserData_Payload_CC_608_LINES_PER_FIELD_MAX));
               }
               bMax608LinesExceededWarningPrinted = true;
            }
            else
            {
               volatile uint8_t* p608Payload = (uint8_t*)((uint8_t*) pCCPayload + BVCE_FW_P_UserData_CCPayload_Get608Line_OFFSET(uiNumValid608Lines));

               p608Payload[0] = 0;
               p608Payload[0] &= ~BVCE_FW_P_UserData_Payload_CC_608Metadata_LineOffset_MASK;
               p608Payload[0] |= ( pPacketDescriptor->data.stDvs157.stCC.astLine[i].line_offset << BVCE_FW_P_UserData_Payload_CC_608Metadata_LineOffset_SHIFT ) & BVCE_FW_P_UserData_Payload_CC_608Metadata_LineOffset_MASK;

               p608Payload[0] &= ~BVCE_FW_P_UserData_Payload_CC_608Metadata_Priority_MASK;
               p608Payload[0] |= ( pPacketDescriptor->data.stDvs157.stCC.astLine[i].cc_priority << BVCE_FW_P_UserData_Payload_CC_608Metadata_Priority_SHIFT ) & BVCE_FW_P_UserData_Payload_CC_608Metadata_Priority_MASK;

               if ( true == pPacketDescriptor->data.stDvs157.stCC.astLine[i].cc_valid )
               {
                  p608Payload[0] &= ~BVCE_FW_P_UserData_Payload_CC_608Metadata_Valid_MASK;
                  p608Payload[0] |= ( 1 << BVCE_FW_P_UserData_Payload_CC_608Metadata_Valid_SHIFT ) & BVCE_FW_P_UserData_Payload_CC_608Metadata_Valid_MASK;
               }

               p608Payload[1] = pPacketDescriptor->data.stDvs157.stCC.astLine[i].cc_data_1;
               p608Payload[2] = pPacketDescriptor->data.stDvs157.stCC.astLine[i].cc_data_2;

               uiNumValid608Lines++;
            }
         }
         else
         {
            /* Process 708 Data */
            if ( uiNumValid708Lines > BVCE_FW_P_UserData_Payload_CC_708_LINES_PER_FIELD_MAX )
            {
               if ( false == bMax708LinesExceededWarningPrinted )
               {
                  BDBG_WRN(("Error: Number of Valid 708 Lines (%d) is greater than max (%d). Dropping extra lines.", uiNumValid708Lines, BVCE_FW_P_UserData_Payload_CC_708_LINES_PER_FIELD_MAX));
               }
               bMax708LinesExceededWarningPrinted = true;
            }
            else
            {
               volatile uint8_t* p708Payload = (uint8_t*)((uint8_t*) pCCPayload + BVCE_FW_P_UserData_CCPayload_Get708Line_OFFSET(uiNumValid708Lines));

               p708Payload[0] = 0;

               if ( true == pPacketDescriptor->data.stDvs157.stCC.astLine[i].cc_valid )
               {
                  p708Payload[0] &= ~BVCE_FW_P_UserData_Payload_CC_708Metadata_Valid_MASK;
                  p708Payload[0] |= ( 1 << BVCE_FW_P_UserData_Payload_CC_708Metadata_Valid_SHIFT ) & BVCE_FW_P_UserData_Payload_CC_708Metadata_Valid_MASK;
               }

               if ( pPacketDescriptor->data.stDvs157.stCC.astLine[i].seq.cc_type == 0x3 )
               {
                  p708Payload[0] &= ~BVCE_FW_P_UserData_Payload_CC_708Metadata_PacketStart_MASK;
                  p708Payload[0] |= ( 1 << BVCE_FW_P_UserData_Payload_CC_708Metadata_PacketStart_SHIFT ) & BVCE_FW_P_UserData_Payload_CC_708Metadata_PacketStart_MASK;
               }

               p708Payload[1] = pPacketDescriptor->data.stDvs157.stCC.astLine[i].cc_data_1;
               p708Payload[2] = pPacketDescriptor->data.stDvs157.stCC.astLine[i].cc_data_2;

               uiNumValid708Lines++;
            }
         }
      }

      BVCE_UserData_P_Set8(pCCPayload, BVCE_FW_P_UserData_CCPayload_NumValid608Lines_OFFSET, uiNumValid608Lines);
      BVCE_UserData_P_Set8(pCCPayload, BVCE_FW_P_UserData_CCPayload_NumValid708Lines_OFFSET, uiNumValid708Lines);
   }
}

/*************/
/* User Data */
/*************/
/* BVCE_Channel_UserData_AddBuffers_isr - adds user data field info structs(s) to user data queue for stream insertion */
BERR_Code
BVCE_Channel_UserData_AddBuffers_isr(
         BVCE_Channel_Handle hVceCh,
         const BUDP_Encoder_FieldInfo *pstUserDataFieldInfo, /* Pointer to first field info descriptor */
         unsigned uiCount, /* Count of user data field buffer info structs */
         unsigned *puiQueuedCount /* Count of user data field info structs queued by encoder (*puiQueuedCount <= uiCount) */
         )
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_ENTER( BVCE_Channel_UserData_AddBuffers_isr );

   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);
   BDBG_ASSERT( pstUserDataFieldInfo );
   BDBG_ASSERT( puiQueuedCount );
   BVCE_P_FUNCTION_TRACE_ENTER_isr(1, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);

   BKNI_Memset( (void*) &hVceCh->userdata.stUserDataQueue, 0, sizeof( BVCE_FW_P_UserData_Queue ) );

   *puiQueuedCount = 0;

   if ( ( BAVC_VideoCompressionStd_eVP8 == hVceCh->stStartEncodeSettings.stProtocolInfo.eProtocol )
         || ( BAVC_VideoCompressionStd_eVP9 == hVceCh->stStartEncodeSettings.stProtocolInfo.eProtocol ) )
   {
      BDBG_MODULE_WRN( BVCE_USERDATA, ("User data not allowed with VP8/VP9") );
      rc = BERR_TRACE( BERR_INVALID_PARAMETER );
   }
   else if ( BVCE_P_Status_eStarted == hVceCh->eStatus )
   {
      void *pDescriptorBufferCached = NULL;

      /* Get cached offset */
      pDescriptorBufferCached = BVCE_P_Buffer_GetAddress_isrsafe( hVceCh->userdata.hBuffer );
      if ( NULL == pDescriptorBufferCached )
      {
         BDBG_MODULE_ERR( BVCE_USERDATA, ("Userdata cached pointer is invalid"));
         rc = BERR_TRACE( BERR_UNKNOWN );
      }

      /* Get FW Queue Info */
      BVCE_P_ReadRegisters_isr(
               hVceCh->hVce,
               hVceCh->userdata.dccm.uiUserDataQueueInfoAddress,
               (uint32_t*) (&hVceCh->userdata.stUserDataQueue),
               sizeof( BVCE_FW_P_UserData_Queue )
               );

      if ( ( hVceCh->userdata.stUserDataQueue.uiReadOffset >= BVCE_FW_P_USERDATA_QUEUE_LENGTH )
           || ( hVceCh->userdata.stUserDataQueue.uiWriteOffset >= BVCE_FW_P_USERDATA_QUEUE_LENGTH ) )
      {
         BDBG_MODULE_ERR( BVCE_USERDATA, ("Userdata queue pointers are invalid R/W = %08x/%08x", hVceCh->userdata.stUserDataQueue.uiReadOffset, hVceCh->userdata.stUserDataQueue.uiWriteOffset));
         rc = BERR_TRACE( BERR_UNKNOWN );
      }
      else
      {
   #if BVCE_P_DUMP_USERDATA_LOG
         if ( hVceCh->userdata.stUserDataQueue.uiReadOffset == ( ( hVceCh->userdata.stUserDataQueue.uiWriteOffset + 1 ) % BVCE_FW_P_USERDATA_QUEUE_LENGTH ) )
         {
            if ( NULL != hVceCh->userdata.hUserDataLog )
            {
               BVCE_Debug_P_WriteLog_isr(hVceCh->userdata.hUserDataLog, "%d,%d,%d\n",
                  hVceCh->userdata.stUserDataQueue.uiReadOffset,
                  hVceCh->userdata.stUserDataQueue.uiWriteOffset,
                  pstUserDataFieldInfo->uiStgPictureId
               );
            }
         }
   #endif

         while ( ( hVceCh->userdata.stUserDataQueue.uiReadOffset != ( ( hVceCh->userdata.stUserDataQueue.uiWriteOffset + 1 ) % BVCE_FW_P_USERDATA_QUEUE_LENGTH ) )
                 && ( *puiQueuedCount < uiCount ) )
         {
            BVCE_FW_P_UserData_PacketType ePacketType;
            unsigned uiSourceDescNum = 0;
            unsigned uiTargetDescNum = 0;
            uint32_t uiDescriptorOffset = ( hVceCh->userdata.stUserDataQueue.uiWriteOffset * BVCE_FW_P_UserData_PacketDescriptor_MAX_LENGTH * BVCE_UserData_P_PacketDescriptor_MAX_PER_FIELD);
            void *pBaseFWPacketDescriptor =  (void*) ( (uint8_t*) pDescriptorBufferCached + uiDescriptorOffset );

            BDBG_ASSERT( BVCE_UserData_P_PacketDescriptor_MAX_PER_FIELD >= pstUserDataFieldInfo->uiNumDescriptors );

            /* FW wants packets to be inserted in ascending packet type order, so for now, just do a brute force traverse of all the descriptors
             * process them in ascending packet type order
             */
            for ( ePacketType = 0; ePacketType < BVCE_FW_P_UserData_PacketType_eMax; ePacketType++ )
            {
               for ( uiSourceDescNum = 0; uiSourceDescNum < pstUserDataFieldInfo->uiNumDescriptors; uiSourceDescNum++ )
               {

                  /* Only add packet types that FW understands */
                  if ( ePacketType == BVCE_P_UserData_PacketTypeLUT[pstUserDataFieldInfo->stPacketDescriptor[uiSourceDescNum].ePacketFormat] )
                  {
                     void *pFWPacketDescriptor = (void*) ( (uint8_t*) pBaseFWPacketDescriptor + uiTargetDescNum*BVCE_FW_P_UserData_PacketDescriptor_MAX_LENGTH );
                     const BUDP_Encoder_PacketDescriptor *pPacketDescriptor = &pstUserDataFieldInfo->stPacketDescriptor[uiSourceDescNum];

                     /* Populate BVCE_FW_P_UserData_PacketDescriptor */
                     BVCE_UserData_P_ParsePacketDescriptor_isrsafe( pPacketDescriptor, pFWPacketDescriptor );

                     uiTargetDescNum++;
                  }
               }
            }

            BVCE_P_Buffer_FlushCache_isrsafe(
               hVceCh->userdata.hBuffer,
               pBaseFWPacketDescriptor,
               uiTargetDescNum*BVCE_FW_P_UserData_PacketDescriptor_MAX_LENGTH
               );

            /* Populate BVCE_FW_P_UserData_QueueEntry */
            hVceCh->userdata.stUserDataQueue.astQueue[hVceCh->userdata.stUserDataQueue.uiWriteOffset].uiMetadata = 0;

            hVceCh->userdata.stUserDataQueue.astQueue[hVceCh->userdata.stUserDataQueue.uiWriteOffset].uiMetadata &= ~BVCE_FW_P_UserData_QueueEntry_Metadata_STGPictureID_MASK;
            hVceCh->userdata.stUserDataQueue.astQueue[hVceCh->userdata.stUserDataQueue.uiWriteOffset].uiMetadata |= ( pstUserDataFieldInfo->uiStgPictureId << BVCE_FW_P_UserData_QueueEntry_Metadata_STGPictureID_SHIFT ) & BVCE_FW_P_UserData_QueueEntry_Metadata_STGPictureID_MASK;

            switch ( pstUserDataFieldInfo->ePolarity )
            {
               case BAVC_Polarity_eTopField:
                  /* Do Nothing */
                  break;

               case BAVC_Polarity_eBotField:
                  hVceCh->userdata.stUserDataQueue.astQueue[hVceCh->userdata.stUserDataQueue.uiWriteOffset].uiMetadata |= ( 1 << BVCE_FW_P_UserData_QueueEntry_Metadata_Polarity_SHIFT ) & BVCE_FW_P_UserData_QueueEntry_Metadata_Polarity_MASK;
                  break;

               default:
                  BDBG_ERR(("Unsupported polarity: %d", pstUserDataFieldInfo->ePolarity));
                  return BERR_TRACE( BERR_INVALID_PARAMETER );
                  break;
            }

            hVceCh->userdata.stUserDataQueue.astQueue[hVceCh->userdata.stUserDataQueue.uiWriteOffset].uiMetadata |= ( ( BVCE_FW_P_UserData_PacketDescriptor_MAX_LENGTH * uiTargetDescNum ) << BVCE_FW_P_UserData_QueueEntry_Metadata_Length_SHIFT ) & BVCE_FW_P_UserData_QueueEntry_Metadata_Length_MASK;
            hVceCh->userdata.stUserDataQueue.astQueue[hVceCh->userdata.stUserDataQueue.uiWriteOffset].uiOffset = uiDescriptorOffset;

   #if BVCE_P_DUMP_USERDATA_LOG
            /* Debug */
            if ( NULL != hVceCh->userdata.hUserDataLog )
            {
               unsigned i;

               for ( i = 0; i < uiTargetDescNum; i++ )
               {
                  void *pFWPacketDescriptor = (void*) ( (uint8_t*) pBaseFWPacketDescriptor + i*BVCE_FW_P_UserData_PacketDescriptor_MAX_LENGTH );

                  {
                     unsigned uiNum608Lines = ((uint8_t*) pFWPacketDescriptor)[BVCE_FW_P_UserData_PacketDescriptor_CCPayload_OFFSET + BVCE_FW_P_UserData_CCPayload_NumValid608Lines_OFFSET];
                     unsigned line;

                     for ( line = 0; line < uiNum608Lines; line++ )
                     {

                        BVCE_Debug_P_WriteLog_isr(hVceCh->userdata.hUserDataLog, "%u,%u,%u,%d,%d,%d,%u,%d,%d,%d,%d,%d,%d,%d,%d\n",
                           BREG_Read32(
                              hVceCh->hVce->handles.hReg,
                              hVceCh->hVce->stPlatformConfig.stDebug.uiSTC[hVceCh->stOpenSettings.uiInstance]),
                           hVceCh->userdata.stUserDataQueue.uiReadOffset,
                           hVceCh->userdata.stUserDataQueue.uiWriteOffset,
                           (hVceCh->userdata.stUserDataQueue.astQueue[hVceCh->userdata.stUserDataQueue.uiWriteOffset].uiMetadata & BVCE_FW_P_UserData_QueueEntry_Metadata_STGPictureID_MASK) >> BVCE_FW_P_UserData_QueueEntry_Metadata_STGPictureID_SHIFT,
                           (hVceCh->userdata.stUserDataQueue.astQueue[hVceCh->userdata.stUserDataQueue.uiWriteOffset].uiMetadata & BVCE_FW_P_UserData_QueueEntry_Metadata_Polarity_MASK) >> BVCE_FW_P_UserData_QueueEntry_Metadata_Polarity_SHIFT,
                           (hVceCh->userdata.stUserDataQueue.astQueue[hVceCh->userdata.stUserDataQueue.uiWriteOffset].uiMetadata & BVCE_FW_P_UserData_QueueEntry_Metadata_Length_MASK) >> BVCE_FW_P_UserData_QueueEntry_Metadata_Length_SHIFT,
                           hVceCh->userdata.stUserDataQueue.astQueue[hVceCh->userdata.stUserDataQueue.uiWriteOffset].uiOffset,
                           i,
                           BVCE_UserData_P_Get16_LE( pFWPacketDescriptor, BVCE_FW_P_UserData_PacketDescriptor_Length_OFFSET ),
                           BVCE_UserData_P_Get16_LE( pFWPacketDescriptor, BVCE_FW_P_UserData_PacketDescriptor_Type_OFFSET ),
                           true,
                           uiNum608Lines,
                           ((uint8_t*) pFWPacketDescriptor)[0 + BVCE_FW_P_UserData_PacketDescriptor_CCPayload_OFFSET + BVCE_FW_P_UserData_CCPayload_Get608Line_OFFSET(line)],
                           ((uint8_t*) pFWPacketDescriptor)[1 + BVCE_FW_P_UserData_PacketDescriptor_CCPayload_OFFSET + BVCE_FW_P_UserData_CCPayload_Get608Line_OFFSET(line)],
                           ((uint8_t*) pFWPacketDescriptor)[2 + BVCE_FW_P_UserData_PacketDescriptor_CCPayload_OFFSET + BVCE_FW_P_UserData_CCPayload_Get608Line_OFFSET(line)]
                        );
                     }
                  }

                  {
                     unsigned uiNum708Lines = ((uint8_t*) pFWPacketDescriptor)[BVCE_FW_P_UserData_PacketDescriptor_CCPayload_OFFSET + BVCE_FW_P_UserData_CCPayload_NumValid708Lines_OFFSET];
                     unsigned line;

                     for ( line = 0; line < uiNum708Lines; line++ )
                     {

                        BVCE_Debug_P_WriteLog_isr(hVceCh->userdata.hUserDataLog, "%u,%u,%u,%d,%d,%d,%u,%d,%d,%d,%d,%d,%d,%d,%d\n",
                           BREG_Read32(
                              hVceCh->hVce->handles.hReg,
                              hVceCh->hVce->stPlatformConfig.stDebug.uiSTC[hVceCh->stOpenSettings.uiInstance]),
                           hVceCh->userdata.stUserDataQueue.uiReadOffset,
                           hVceCh->userdata.stUserDataQueue.uiWriteOffset,
                           (hVceCh->userdata.stUserDataQueue.astQueue[hVceCh->userdata.stUserDataQueue.uiWriteOffset].uiMetadata & BVCE_FW_P_UserData_QueueEntry_Metadata_STGPictureID_MASK) >> BVCE_FW_P_UserData_QueueEntry_Metadata_STGPictureID_SHIFT,
                           (hVceCh->userdata.stUserDataQueue.astQueue[hVceCh->userdata.stUserDataQueue.uiWriteOffset].uiMetadata & BVCE_FW_P_UserData_QueueEntry_Metadata_Polarity_MASK) >> BVCE_FW_P_UserData_QueueEntry_Metadata_Polarity_SHIFT,
                           (hVceCh->userdata.stUserDataQueue.astQueue[hVceCh->userdata.stUserDataQueue.uiWriteOffset].uiMetadata & BVCE_FW_P_UserData_QueueEntry_Metadata_Length_MASK) >> BVCE_FW_P_UserData_QueueEntry_Metadata_Length_SHIFT,
                           hVceCh->userdata.stUserDataQueue.astQueue[hVceCh->userdata.stUserDataQueue.uiWriteOffset].uiOffset,
                           i,
                           BVCE_UserData_P_Get16_LE( pFWPacketDescriptor, BVCE_FW_P_UserData_PacketDescriptor_Length_OFFSET ),
                           BVCE_UserData_P_Get16_LE( pFWPacketDescriptor, BVCE_FW_P_UserData_PacketDescriptor_Type_OFFSET ),
                           false,
                           uiNum708Lines,
                           ((uint8_t*) pFWPacketDescriptor)[0 + BVCE_FW_P_UserData_PacketDescriptor_CCPayload_OFFSET + BVCE_FW_P_UserData_CCPayload_Get708Line_OFFSET(line)],
                           ((uint8_t*) pFWPacketDescriptor)[1 + BVCE_FW_P_UserData_PacketDescriptor_CCPayload_OFFSET + BVCE_FW_P_UserData_CCPayload_Get708Line_OFFSET(line)],
                           ((uint8_t*) pFWPacketDescriptor)[2 + BVCE_FW_P_UserData_PacketDescriptor_CCPayload_OFFSET + BVCE_FW_P_UserData_CCPayload_Get708Line_OFFSET(line)]
                        );
                     }
                  }
               }

               if ( 0 == uiTargetDescNum )
               {
                  BVCE_Debug_P_WriteLog_isr(hVceCh->userdata.hUserDataLog, "%u,%u,%u,%d,%d,%d,%u,-,-,-,-,-,-,-,-\n",
                     BREG_Read32(
                        hVceCh->hVce->handles.hReg,
                        hVceCh->hVce->stPlatformConfig.stDebug.uiSTC[hVceCh->stOpenSettings.uiInstance]),
                     hVceCh->userdata.stUserDataQueue.uiReadOffset,
                     hVceCh->userdata.stUserDataQueue.uiWriteOffset,
                     (hVceCh->userdata.stUserDataQueue.astQueue[hVceCh->userdata.stUserDataQueue.uiWriteOffset].uiMetadata & BVCE_FW_P_UserData_QueueEntry_Metadata_STGPictureID_MASK) >> BVCE_FW_P_UserData_QueueEntry_Metadata_STGPictureID_SHIFT,
                     (hVceCh->userdata.stUserDataQueue.astQueue[hVceCh->userdata.stUserDataQueue.uiWriteOffset].uiMetadata & BVCE_FW_P_UserData_QueueEntry_Metadata_Polarity_MASK) >> BVCE_FW_P_UserData_QueueEntry_Metadata_Polarity_SHIFT,
                     (hVceCh->userdata.stUserDataQueue.astQueue[hVceCh->userdata.stUserDataQueue.uiWriteOffset].uiMetadata & BVCE_FW_P_UserData_QueueEntry_Metadata_Length_MASK) >> BVCE_FW_P_UserData_QueueEntry_Metadata_Length_SHIFT,
                     hVceCh->userdata.stUserDataQueue.astQueue[hVceCh->userdata.stUserDataQueue.uiWriteOffset].uiOffset
                     );
               }
            }
#endif

            /* Update *puiQueuedCount */
            (*puiQueuedCount)++;
            hVceCh->userdata.stUserDataQueue.uiWriteOffset = ( hVceCh->userdata.stUserDataQueue.uiWriteOffset + 1 ) % BVCE_FW_P_USERDATA_QUEUE_LENGTH;
            hVceCh->userdata.uiQueuedBuffers++;

            pstUserDataFieldInfo = BUDP_ENCODER_FIELDINFO_GET_NEXT (pstUserDataFieldInfo);
         }

         if ( 0 != *puiQueuedCount )
         {
            /* Update FW Queue Info */
            /* Update Queue First */
            unsigned uiWriteOffset = ( (uint8_t*) (hVceCh->userdata.stUserDataQueue.astQueue) - (uint8_t*) (&hVceCh->userdata.stUserDataQueue.uiReadOffset) );

            /* Set base offset */
            BVCE_P_SET_32BIT_HI_LO_FROM_64( hVceCh->userdata.stUserDataQueue.uiBaseOffset, BVCE_P_Buffer_GetDeviceOffset_isrsafe( hVceCh->userdata.hBuffer ) );

            /* coverity[address_of] */
            /* coverity[callee_ptr_arith] */
            BVCE_P_WriteRegisters_isr(
                     hVceCh->hVce,
                     hVceCh->userdata.dccm.uiUserDataQueueInfoAddress + uiWriteOffset,
                     (uint32_t*) (hVceCh->userdata.stUserDataQueue.astQueue),
                     sizeof( BVCE_FW_P_UserData_Queue ) - uiWriteOffset
                     );

            /* Update Write Offset AFTER queue is updated */
            uiWriteOffset = ( (uint8_t*) (&hVceCh->userdata.stUserDataQueue.uiWriteOffset) - (uint8_t*) (&hVceCh->userdata.stUserDataQueue.uiReadOffset) );
            /* coverity[address_of] */
            /* coverity[callee_ptr_arith] */
            BVCE_P_WriteRegisters_isr(
                     hVceCh->hVce,
                     hVceCh->userdata.dccm.uiUserDataQueueInfoAddress + uiWriteOffset,
                     (uint32_t*) (&hVceCh->userdata.stUserDataQueue.uiWriteOffset),
                     sizeof( hVceCh->userdata.stUserDataQueue.uiWriteOffset )
                     );

            BDBG_MODULE_MSG( BVCE_USERDATA, ("Userdata queue pointers             R/W = %08x/%08x", hVceCh->userdata.stUserDataQueue.uiReadOffset, hVceCh->userdata.stUserDataQueue.uiWriteOffset));
         }
      }
   }
   else
   {
      BDBG_MODULE_WRN( BVCE_USERDATA, ("Cannot queue user data when channel is not started") );
      rc = BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   BVCE_P_FUNCTION_TRACE_LEAVE_isr(1, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);
   BDBG_LEAVE( BVCE_Channel_UserData_AddBuffers_isr );
   return BERR_TRACE( rc );
}

#if (BVCE_P_VDC_MANAGE_VIP)
static bool BVCE_S_AcquirePictureBlockInfoFromEncodePictureBuffer_isrsafe(
   BVCE_P_PictureBlockInfo *pstBlockInfo,
   const BAVC_EncodePictureBuffer *pstEncodePictureBuffer,
   BVCE_P_PictureBufferType ePictureBufferType
   )
{
   BKNI_Memset( pstBlockInfo, 0, sizeof( BVCE_P_PictureBlockInfo ) );

   switch ( ePictureBufferType )
   {
      case BVCE_P_PictureBufferType_eLuma:
         pstBlockInfo->hBlock = pstEncodePictureBuffer->hLumaBlock;
         pstBlockInfo->uiOffset = pstEncodePictureBuffer->ulLumaOffset;
         pstBlockInfo->uiNMBY = pstEncodePictureBuffer->ulLumaNMBY;
         pstBlockInfo->bDcx = pstEncodePictureBuffer->bDcx;
         break;

      case BVCE_P_PictureBufferType_eChroma:
         pstBlockInfo->hBlock = pstEncodePictureBuffer->hChromaBlock;
         pstBlockInfo->uiOffset = pstEncodePictureBuffer->ulChromaOffset;
         pstBlockInfo->uiNMBY = pstEncodePictureBuffer->ulChromaNMBY;
         pstBlockInfo->bDcx = pstEncodePictureBuffer->bChromaDcx;
         break;

      case BVCE_P_PictureBufferType_e1VLuma:
         pstBlockInfo->hBlock = pstEncodePictureBuffer->h1VLumaBlock;
         pstBlockInfo->uiOffset = pstEncodePictureBuffer->ul1VLumaOffset;
         pstBlockInfo->uiNMBY = pstEncodePictureBuffer->ul1VLumaNMBY;
         pstBlockInfo->uiHorizontalDecimationShift = pstEncodePictureBuffer->ulHorizontalDecimationShift;
         pstBlockInfo->bDcx = pstEncodePictureBuffer->b1VLumaDcx;
         break;

      case BVCE_P_PictureBufferType_e2VLuma:
         pstBlockInfo->hBlock = pstEncodePictureBuffer->h2VLumaBlock;
         pstBlockInfo->uiOffset = pstEncodePictureBuffer->ul2VLumaOffset;
         pstBlockInfo->uiNMBY = pstEncodePictureBuffer->ul2VLumaNMBY;
         pstBlockInfo->uiHorizontalDecimationShift = pstEncodePictureBuffer->ulHorizontalDecimationShift;
         pstBlockInfo->bDcx = pstEncodePictureBuffer->b2VLumaDcx;
         break;

      case BVCE_P_PictureBufferType_eShiftedChroma:
         pstBlockInfo->hBlock = pstEncodePictureBuffer->hShiftedChromaBlock;
         pstBlockInfo->uiOffset = pstEncodePictureBuffer->ulShiftedChromaOffset;
         pstBlockInfo->uiNMBY = pstEncodePictureBuffer->ulShiftedChromaNMBY;
         break;

      default:
         BDBG_ASSERT(0);
   }

   if ( NULL != pstBlockInfo->hBlock )
   {
      /* Acquire the block and physical offset */
      pstBlockInfo->uiBlockOffset = BMMA_GetOffset_isr( pstBlockInfo->hBlock );
      pstBlockInfo->uiPhysicalOffset = pstBlockInfo->uiBlockOffset + pstBlockInfo->uiOffset;
      return true;
   }
   else
   {
      return false;
   }
}

static void BVCE_S_ReleasePictureBlockInfoToEncodePictureBuffer_isrsafe(
   BVCE_P_PictureBlockInfo *pstBlockInfo,
   BAVC_EncodePictureBuffer *pstEncodePictureBuffer,
   BVCE_P_PictureBufferType ePictureBufferType
   )
{
   switch ( ePictureBufferType )
   {
      case BVCE_P_PictureBufferType_eLuma:
         pstEncodePictureBuffer->hLumaBlock = pstBlockInfo->hBlock;
         pstEncodePictureBuffer->ulLumaOffset = pstBlockInfo->uiOffset;
         break;

      case BVCE_P_PictureBufferType_eChroma:
         pstEncodePictureBuffer->hChromaBlock = pstBlockInfo->hBlock;
         pstEncodePictureBuffer->ulChromaOffset = pstBlockInfo->uiOffset;
         break;

      case BVCE_P_PictureBufferType_e1VLuma:
         pstEncodePictureBuffer->h1VLumaBlock = pstBlockInfo->hBlock;
         pstEncodePictureBuffer->ul1VLumaOffset = pstBlockInfo->uiOffset;
         break;

      case BVCE_P_PictureBufferType_e2VLuma:
         pstEncodePictureBuffer->h2VLumaBlock = pstBlockInfo->hBlock;
         pstEncodePictureBuffer->ul2VLumaOffset = pstBlockInfo->uiOffset;
         break;

      case BVCE_P_PictureBufferType_eShiftedChroma:
         pstEncodePictureBuffer->hShiftedChromaBlock = pstBlockInfo->hBlock;
         pstEncodePictureBuffer->ulShiftedChromaOffset = pstBlockInfo->uiOffset;
         break;

      default:
         BDBG_ASSERT(0);
   }

   if ( ( NULL != pstBlockInfo->hBlock )
        && ( 0 != pstBlockInfo->uiBlockOffset ) )
   {
      /* Release the block and physical offset */
      BKNI_Memset( pstBlockInfo, 0, sizeof( BVCE_P_PictureBlockInfo ) );
   }
}

static void BVCE_S_SetPictureBufferInfoFromPictureBlockInfo_isrsafe(
   BVCE_P_FW_PictureBufferMailbox *pstPictureBufferMailbox,
   const BVCE_P_PictureBlockInfo *pstBlockInfo,
   BVCE_P_PictureBufferType ePictureBufferType
   )
{
   BVCE_P_FW_PictureBufferInfo* pstPictureBufferInfo = NULL;

   switch ( ePictureBufferType )
   {
      case BVCE_P_PictureBufferType_eLuma:
         pstPictureBufferInfo = &pstPictureBufferMailbox->LumaBufPtr;
         break;

      case BVCE_P_PictureBufferType_eChroma:
         pstPictureBufferInfo = &pstPictureBufferMailbox->ChromaBufPtr;
         break;

      case BVCE_P_PictureBufferType_e1VLuma:
         pstPictureBufferInfo = &pstPictureBufferMailbox->Luma1VBufPtr;
         break;

      case BVCE_P_PictureBufferType_e2VLuma:
         pstPictureBufferInfo = &pstPictureBufferMailbox->Luma2VBufPtr;
         break;

      case BVCE_P_PictureBufferType_eShiftedChroma:
         pstPictureBufferInfo = &pstPictureBufferMailbox->ShiftedChromaBufPtr;
         break;

      default:
         BDBG_ASSERT(0);
   }

   BVCE_P_SET_32BIT_HI_LO_FROM_64( pstPictureBufferInfo->stOffset.uiOffset, pstBlockInfo->uiPhysicalOffset );

   pstPictureBufferInfo->uiMetadata &= ~BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_MASK;
   pstPictureBufferInfo->uiMetadata |= ( pstBlockInfo->uiNMBY << BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_SHIFT ) & BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_MASK;

   pstPictureBufferInfo->uiMetadata &= ~BVCE_P_FW_PictureBufferInfo_Metadata_DECIMATION_MASK;
   pstPictureBufferInfo->uiMetadata |= ( pstBlockInfo->uiHorizontalDecimationShift << BVCE_P_FW_PictureBufferInfo_Metadata_DECIMATION_SHIFT ) & BVCE_P_FW_PictureBufferInfo_Metadata_DECIMATION_MASK;

   pstPictureBufferInfo->uiMetadata &= ~BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_MASK;
   pstPictureBufferInfo->uiMetadata |= ( ( true == pstBlockInfo->bDcx ) << BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_SHIFT ) & BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_MASK;
}

/***********/
/* Picture */
/***********/
static const uint32_t BVCE_P_PI2FW_BarDataTypeLUT[BAVC_BarDataType_eLeftRight + 1] =
{
   SOURCE_BAR_DATA_TYPE_INVALID,    /* BAVC_BarDataType_eInvalid */
   SOURCE_BAR_DATA_TYPE_TOP_BOTTOM, /* BAVC_BarDataType_eTopBottom */
   SOURCE_BAR_DATA_TYPE_LEFT_RIGHT  /* BAVC_BarDataType_eLeftRight */
};

static const uint32_t BVCE_P_PI2FW_PolarityLUT[BAVC_Polarity_eFrame + 1] =
{
   SOURCE_POLARITY_TOP,  /* BAVC_Polarity_eTopField */
   SOURCE_POLARITY_BOT,  /* BAVC_Polarity_eBotField */
   SOURCE_POLARITY_FRAME /* BAVC_Polarity_eFrame */
};

static const uint32_t BVCE_P_PI2FW_CadenceTypeLUT[BAVC_CadenceType_eMax] =
{
   SOURCE_CADENCE_TYPE_UNLOCKED, /* BAVC_CadenceType_eUnlocked */
   SOURCE_CADENCE_TYPE_3_2,      /* BAVC_CadenceType_e3_2 */
   SOURCE_CADENCE_TYPE_2_2       /* BAVC_CadenceType_e2_2 */
};

static const uint32_t BVCE_P_PI2FW_CadencePhaseLUT[BAVC_CadencePhase_eMax] =
{
   SOURCE_CADENCE_PHASE_0, /* BAVC_CadencePhase_e0 */
   SOURCE_CADENCE_PHASE_1, /* BAVC_CadencePhase_e1 */
   SOURCE_CADENCE_PHASE_2, /* BAVC_CadencePhase_e2 */
   SOURCE_CADENCE_PHASE_3, /* BAVC_CadencePhase_e3 */
   SOURCE_CADENCE_PHASE_4  /* BAVC_CadencePhase_e4 */
};

static const uint32_t BVCE_P_PI2FW_OrientationLUT[BFMT_Orientation_eLeftRight_Enhanced + 1] =
{
   SOURCE_ORIENTATION_TYPE_2D,                    /* BFMT_Orientation_e2D */
   SOURCE_ORIENTATION_TYPE_3D_LEFT_RIGHT,         /* BFMT_Orientation_e3D_LeftRight */
   SOURCE_ORIENTATION_TYPE_3D_OVER_UNDER,         /* BFMT_Orientation_e3D_OverUnder */
   SOURCE_ORIENTATION_TYPE_3D_LEFT,               /* BFMT_Orientation_e3D_Left */
   SOURCE_ORIENTATION_TYPE_3D_RIGHT,              /* BFMT_Orientation_e3D_Right */
   SOURCE_ORIENTATION_TYPE_3D_LEFT_RIGHT_ENHANCED /* BFMT_Orientation_eLeftRight_Enhanced */
};

#define BVCE_P_PI2FW_DELTAPTS_BASE ((uint64_t) 90000*4)
#define BVCE_P_PI2FW_DELTAPTS_BASE_RATE (1000)
#define BVCE_P_PI2FW_DELTAPTS_ANALOG_RATE (1001)
#define BVCE_P_PI2FW_DELTAPTS_DIGITAL_RATE (1000)
#define BVCE_P_PI2FW_DELTAPTS(_frameRateNumerator, _frameRateDenominator, _type) (((BVCE_P_PI2FW_DELTAPTS_BASE) * (BVCE_P_PI2FW_DELTAPTS_##_type##_RATE) * (_frameRateDenominator)) / ((_frameRateNumerator) * (BVCE_P_PI2FW_DELTAPTS_BASE_RATE)))

static const uint16_t BVCE_P_PI2FW_FrameRate2DeltaPtsLUT[BAVC_FrameRateCode_eMax] =
{
   0,                                   /* BAVC_FrameRateCode_eUnknown */
   BVCE_P_PI2FW_DELTAPTS(24,1,ANALOG),  /* BAVC_FrameRateCode_e23_976 */
   BVCE_P_PI2FW_DELTAPTS(24,1,DIGITAL), /* BAVC_FrameRateCode_e24 */
   BVCE_P_PI2FW_DELTAPTS(25,1,DIGITAL), /* BAVC_FrameRateCode_e25 */
   BVCE_P_PI2FW_DELTAPTS(60,2,ANALOG),  /* BAVC_FrameRateCode_e29_97 */
   BVCE_P_PI2FW_DELTAPTS(60,2,DIGITAL), /* BAVC_FrameRateCode_e30 */
   BVCE_P_PI2FW_DELTAPTS(50,1,DIGITAL), /* BAVC_FrameRateCode_e50 */
   BVCE_P_PI2FW_DELTAPTS(60,1,ANALOG),  /* BAVC_FrameRateCode_e59_94 */
   BVCE_P_PI2FW_DELTAPTS(60,1,DIGITAL), /* BAVC_FrameRateCode_e60 */
   BVCE_P_PI2FW_DELTAPTS(60,4,ANALOG),  /* BAVC_FrameRateCode_e14_985 */
   BVCE_P_PI2FW_DELTAPTS(60,8,ANALOG),  /* BAVC_FrameRateCode_e7_493 */
   BVCE_P_PI2FW_DELTAPTS(60,6,DIGITAL), /* BAVC_FrameRateCode_e10 */
   BVCE_P_PI2FW_DELTAPTS(60,4,DIGITAL), /* BAVC_FrameRateCode_e15 */
   BVCE_P_PI2FW_DELTAPTS(60,3,DIGITAL), /* BAVC_FrameRateCode_e20 */
   BVCE_P_PI2FW_DELTAPTS(25,2,DIGITAL), /* BAVC_FrameRateCode_e12_5 */
   BVCE_P_PI2FW_DELTAPTS(100,1,DIGITAL),/* BAVC_FrameRateCode_e100 */
   BVCE_P_PI2FW_DELTAPTS(120,1,ANALOG), /* BAVC_FrameRateCode_e119_88 */
   BVCE_P_PI2FW_DELTAPTS(120,1,DIGITAL),/* BAVC_FrameRateCode_e120 */
   BVCE_P_PI2FW_DELTAPTS(60,3,ANALOG),  /* BAVC_FrameRateCode_e19_98 */
   BVCE_P_PI2FW_DELTAPTS(60,8,DIGITAL), /* BAVC_FrameRateCode_e7_5 */
   BVCE_P_PI2FW_DELTAPTS(60,5,DIGITAL), /* BAVC_FrameRateCode_e12 */
   BVCE_P_PI2FW_DELTAPTS(60,5,ANALOG),  /* BAVC_FrameRateCode_e11_988 */
   BVCE_P_PI2FW_DELTAPTS(60,6,ANALOG),  /* BAVC_FrameRateCode_e9_99 */
};

/* TODO:
 *  - Remove references to FrameRate
 *  - Remove ITFP references?
 *  - Update A2PDelay function to not include VIP Delay
 *    - Should STC snapshot account for BVN's VIP A2PDelay?
 */

typedef struct BVCE_P_FrameRateConversionSettings
{
   bool bIs60to24;
   BAVC_FrameRateCode eEffectiveFrameRate; /* Indicates the effective frame rate after the conversion */
} BVCE_P_FrameRateConversionSettings;

static
BERR_Code BVCE_P_FrameRateConversionLUT_isrsafe(
      BAVC_FrameRateCode eSourceFrameRate,
      BAVC_FrameRateCode eEncodeFrameRate,
      BVCE_P_FrameRateConversionSettings *pstFrameRateConversionSettings
      )
{
   BERR_Code rc = BERR_NOT_SUPPORTED;
   BKNI_Memset( pstFrameRateConversionSettings, 0, sizeof( BVCE_P_FrameRateConversionSettings ) );
   pstFrameRateConversionSettings->eEffectiveFrameRate = BAVC_FrameRateCode_eUnknown;

   switch ( eSourceFrameRate )
   {
      case BAVC_FrameRateCode_e7_493:
      case BAVC_FrameRateCode_e7_5:
         /* 7.5 Hz Display */
         switch ( eEncodeFrameRate )
         {
            /* 1:1 */
            case BAVC_FrameRateCode_e7_493:
            case BAVC_FrameRateCode_e7_5:
               pstFrameRateConversionSettings->eEffectiveFrameRate = eSourceFrameRate;
               rc = BERR_SUCCESS;
               break;

            default:
               break;
         }
         break;

      case BAVC_FrameRateCode_e9_99:
      case BAVC_FrameRateCode_e10:
         /* 10 Hz Display */
         switch ( eEncodeFrameRate )
         {
            /* 1:1 */
            case BAVC_FrameRateCode_e9_99:
            case BAVC_FrameRateCode_e10:
               pstFrameRateConversionSettings->eEffectiveFrameRate = eSourceFrameRate;
               rc = BERR_SUCCESS;
               break;

            default:
               break;
         }
         break;

      case BAVC_FrameRateCode_e11_988:
      case BAVC_FrameRateCode_e12:
         /* 12 Hz Display */
         switch ( eEncodeFrameRate )
         {
            /* 1:1 */
            case BAVC_FrameRateCode_e11_988:
            case BAVC_FrameRateCode_e12:
               pstFrameRateConversionSettings->eEffectiveFrameRate = eSourceFrameRate;
               rc = BERR_SUCCESS;
               break;

            default:
               break;
         }
         break;

      case BAVC_FrameRateCode_e12_5:
         /* 12.5 Hz Display */
         switch ( eEncodeFrameRate )
         {
            /* 1:1 */
            case BAVC_FrameRateCode_e12_5:
               pstFrameRateConversionSettings->eEffectiveFrameRate = eSourceFrameRate;
               rc = BERR_SUCCESS;
               break;

            default:
               break;
         }
         break;

      case BAVC_FrameRateCode_e14_985:
      case BAVC_FrameRateCode_e15:
         /* 15 Hz Display */
         switch ( eEncodeFrameRate )
         {
            /* 1:1 */
            case BAVC_FrameRateCode_e14_985:
            case BAVC_FrameRateCode_e15:
               pstFrameRateConversionSettings->eEffectiveFrameRate = eSourceFrameRate;
               rc = BERR_SUCCESS;
               break;

            case BAVC_FrameRateCode_e7_493:
            case BAVC_FrameRateCode_e7_5:
               pstFrameRateConversionSettings->eEffectiveFrameRate = ( BAVC_FrameRateCode_e15 == eSourceFrameRate ) ? BAVC_FrameRateCode_e7_5 : BAVC_FrameRateCode_e7_493;
               rc = BERR_SUCCESS;
               break;

            default:
               break;
         }
         break;

      case BAVC_FrameRateCode_e19_98:
      case BAVC_FrameRateCode_e20:
         /* 20 Hz Display */
         switch ( eEncodeFrameRate )
         {
            /* 1:1 */
            case BAVC_FrameRateCode_e19_98:
            case BAVC_FrameRateCode_e20:
               pstFrameRateConversionSettings->eEffectiveFrameRate = eSourceFrameRate;
               rc = BERR_SUCCESS;
               break;

            case BAVC_FrameRateCode_e9_99:
            case BAVC_FrameRateCode_e10:
               pstFrameRateConversionSettings->eEffectiveFrameRate = ( BAVC_FrameRateCode_e20 == eSourceFrameRate ) ? BAVC_FrameRateCode_e10 : BAVC_FrameRateCode_e9_99;
               rc = BERR_SUCCESS;
               break;

            default:
               break;
         }
         break;

      case BAVC_FrameRateCode_e23_976:
      case BAVC_FrameRateCode_e24:
         /* 24 Hz Display */
         switch ( eEncodeFrameRate )
         {
            /* 1:1 */
            case BAVC_FrameRateCode_e23_976:
            case BAVC_FrameRateCode_e24:
               pstFrameRateConversionSettings->eEffectiveFrameRate = eSourceFrameRate;
               rc = BERR_SUCCESS;
               break;

            case BAVC_FrameRateCode_e11_988:
            case BAVC_FrameRateCode_e12:
               pstFrameRateConversionSettings->eEffectiveFrameRate = ( BAVC_FrameRateCode_e24 == eSourceFrameRate ) ? BAVC_FrameRateCode_e12 : BAVC_FrameRateCode_e11_988;
               rc = BERR_SUCCESS;
               break;

            default:
               break;
         }
         break;

      case BAVC_FrameRateCode_e25:
         /* 25 Hz Display */
         switch ( eEncodeFrameRate )
         {
            /* 1:1 */
            case BAVC_FrameRateCode_e25:
               pstFrameRateConversionSettings->eEffectiveFrameRate = eSourceFrameRate;
               rc = BERR_SUCCESS;
               break;

            /* 2:1 */
            case BAVC_FrameRateCode_e12_5:
               pstFrameRateConversionSettings->eEffectiveFrameRate = BAVC_FrameRateCode_e12_5;
               rc = BERR_SUCCESS;
               break;

            default:
               break;
         }
         break;

      case BAVC_FrameRateCode_e29_97:
      case BAVC_FrameRateCode_e30:
         /* 30 Hz Display */
         switch ( eEncodeFrameRate )
         {
            /* 1:1 */
            case BAVC_FrameRateCode_e29_97:
            case BAVC_FrameRateCode_e30:
               pstFrameRateConversionSettings->eEffectiveFrameRate = eSourceFrameRate;
               rc = BERR_SUCCESS;
               break;

            case BAVC_FrameRateCode_e14_985:
            case BAVC_FrameRateCode_e15:
               pstFrameRateConversionSettings->eEffectiveFrameRate = ( BAVC_FrameRateCode_e30 == eSourceFrameRate ) ? BAVC_FrameRateCode_e15 : BAVC_FrameRateCode_e14_985;
               rc = BERR_SUCCESS;
               break;

            case BAVC_FrameRateCode_e9_99:
            case BAVC_FrameRateCode_e10:
               pstFrameRateConversionSettings->eEffectiveFrameRate = ( BAVC_FrameRateCode_e30 == eSourceFrameRate ) ? BAVC_FrameRateCode_e10 : BAVC_FrameRateCode_e9_99;
               rc = BERR_SUCCESS;
               break;

            case BAVC_FrameRateCode_e7_493:
            case BAVC_FrameRateCode_e7_5:
               pstFrameRateConversionSettings->eEffectiveFrameRate = ( BAVC_FrameRateCode_e30 == eSourceFrameRate ) ? BAVC_FrameRateCode_e7_5 : BAVC_FrameRateCode_e7_493;
               rc = BERR_SUCCESS;
               break;

            default:
               break;
         }
         break;

      case BAVC_FrameRateCode_e50:
         /* 50 Hz Display */
         switch ( eEncodeFrameRate )
         {
            /* 1:1 */
            case BAVC_FrameRateCode_e50:
               pstFrameRateConversionSettings->eEffectiveFrameRate = eSourceFrameRate;
               rc = BERR_SUCCESS;
               break;

            case BAVC_FrameRateCode_e25:
               pstFrameRateConversionSettings->eEffectiveFrameRate = BAVC_FrameRateCode_e25;
               rc = BERR_SUCCESS;
               break;

            case BAVC_FrameRateCode_e12_5:
               pstFrameRateConversionSettings->eEffectiveFrameRate = BAVC_FrameRateCode_e12_5;
               rc = BERR_SUCCESS;
               break;

            default:
               break;
         }
         break;

      case BAVC_FrameRateCode_e59_94:
      case BAVC_FrameRateCode_e60:
         /* 60 Hz Display */
         switch ( eEncodeFrameRate )
         {
            /* 1:1 */
            case BAVC_FrameRateCode_e59_94:
            case BAVC_FrameRateCode_e60:
               pstFrameRateConversionSettings->eEffectiveFrameRate = eSourceFrameRate;
               rc = BERR_SUCCESS;
               break;

            case BAVC_FrameRateCode_e23_976:
            case BAVC_FrameRateCode_e24:
               pstFrameRateConversionSettings->bIs60to24 = true;
               pstFrameRateConversionSettings->eEffectiveFrameRate = ( BAVC_FrameRateCode_e60 == eSourceFrameRate ) ? BAVC_FrameRateCode_e24 : BAVC_FrameRateCode_e23_976;
               rc = BERR_SUCCESS;
               break;

            case BAVC_FrameRateCode_e29_97:
            case BAVC_FrameRateCode_e30:
               pstFrameRateConversionSettings->eEffectiveFrameRate = ( BAVC_FrameRateCode_e60 == eSourceFrameRate ) ? BAVC_FrameRateCode_e30 : BAVC_FrameRateCode_e29_97;
               rc = BERR_SUCCESS;
               break;

            case BAVC_FrameRateCode_e19_98:
            case BAVC_FrameRateCode_e20:
               pstFrameRateConversionSettings->eEffectiveFrameRate = ( BAVC_FrameRateCode_e60 == eSourceFrameRate ) ? BAVC_FrameRateCode_e20 : BAVC_FrameRateCode_e19_98;
               rc = BERR_SUCCESS;
               break;

            case BAVC_FrameRateCode_e14_985:
            case BAVC_FrameRateCode_e15:
               pstFrameRateConversionSettings->eEffectiveFrameRate = ( BAVC_FrameRateCode_e60 == eSourceFrameRate ) ? BAVC_FrameRateCode_e15 : BAVC_FrameRateCode_e14_985;
               rc = BERR_SUCCESS;
               break;

            case BAVC_FrameRateCode_e11_988:
            case BAVC_FrameRateCode_e12:
               pstFrameRateConversionSettings->eEffectiveFrameRate = ( BAVC_FrameRateCode_e60 == eSourceFrameRate ) ? BAVC_FrameRateCode_e12 : BAVC_FrameRateCode_e11_988;
               rc = BERR_SUCCESS;
               break;

            case BAVC_FrameRateCode_e9_99:
            case BAVC_FrameRateCode_e10:
               pstFrameRateConversionSettings->eEffectiveFrameRate = ( BAVC_FrameRateCode_e60 == eSourceFrameRate ) ? BAVC_FrameRateCode_e10 : BAVC_FrameRateCode_e9_99;
               rc = BERR_SUCCESS;
               break;

            case BAVC_FrameRateCode_e7_493:
            case BAVC_FrameRateCode_e7_5:
               pstFrameRateConversionSettings->eEffectiveFrameRate = ( BAVC_FrameRateCode_e60 == eSourceFrameRate ) ? BAVC_FrameRateCode_e7_5 : BAVC_FrameRateCode_e7_493;
               rc = BERR_SUCCESS;
               break;

            default:
               break;
         }
         break;

      default:
         break;
   }

   if ( rc != BERR_SUCCESS )
   {
      BDBG_ERR(("Unsupported frame rate conversion from %u to %u", eSourceFrameRate, eEncodeFrameRate ));
   }

   return BERR_TRACE( rc );
}

static
BERR_Code
BVCE_S_Channel_Picture_Drop_Enqueue_isr(
         BVCE_Channel_Handle hVceCh,
         const BAVC_EncodePictureBuffer *pstPicture /* Pointer to picture info descriptor */
         )
{
   BERR_Code rc = BERR_NOT_AVAILABLE;
   unsigned uiTempWriteOffset = ( hVceCh->picture.stState.dropQueue.uiWriteOffset + 1 ) % BVCE_P_DROP_QUEUE_SIZE;
   if ( uiTempWriteOffset != hVceCh->picture.stState.dropQueue.uiReadOffset )
   {
      hVceCh->picture.stState.dropQueue.astPicture[hVceCh->picture.stState.dropQueue.uiWriteOffset] = *pstPicture;
      hVceCh->picture.stState.dropQueue.uiWriteOffset = uiTempWriteOffset;
      rc = BERR_SUCCESS;
   }

  if ( BERR_NOT_AVAILABLE != rc )
  {
     return BERR_TRACE(rc);
  }
  else
  {
     return rc;
  }
}

static
BERR_Code
BVCE_S_Channel_Picture_Drop_Dequeue_isr(
         BVCE_Channel_Handle hVceCh,
         BAVC_EncodePictureBuffer *pstPicture /* Pointer to picture info descriptor */
         )
{
   BERR_Code rc = BERR_NOT_AVAILABLE;
   if ( hVceCh->picture.stState.dropQueue.uiReadOffset != hVceCh->picture.stState.dropQueue.uiWriteOffset )
   {
      *pstPicture = hVceCh->picture.stState.dropQueue.astPicture[hVceCh->picture.stState.dropQueue.uiReadOffset];
      hVceCh->picture.stState.dropQueue.uiReadOffset = ( hVceCh->picture.stState.dropQueue.uiReadOffset + 1 ) % BVCE_P_DROP_QUEUE_SIZE;
      rc = BERR_SUCCESS;
   }

  if ( BERR_NOT_AVAILABLE != rc )
  {
     return BERR_TRACE(rc);
  }
  else
  {
     return rc;
  }
}

typedef enum BVCE_P_PictureResult
{
   BVCE_P_PictureResult_eWait,
   BVCE_P_PictureResult_eEncode,
   BVCE_P_PictureResult_eDropDueToFRC,
   BVCE_P_PictureResult_eDropDueToError,
   BVCE_P_PictureResult_eDropDueToPolarity,
   BVCE_P_PictureResult_eError,

   BVCE_P_PictureResult_eMax
} BVCE_P_PictureResult;

static const char* const BVCE_S_PolarityFriendlyNameLUT[] =
{
      "Top        ",
      "Bottom     ",
      "Frame      "
};

static
void
BVCE_Channel_S_Picture_Print_Debug_isr(
      BVCE_Channel_Handle hVceCh,
      const BAVC_EncodePictureBuffer *pstPicture,
      uint32_t uiSTCSnapshotLo,
      const char *szError,
      bool bError
      )
{
   const BAVC_EncodePictureBuffer *pstPicturePreviousEncoded = &hVceCh->picture.stState.stPreviousEncoded.stPicture;
   const BAVC_EncodePictureBuffer *pstPicturePreviousReceived = &hVceCh->picture.stState.stPreviousReceived.stPicture;
   int iBytes;

   iBytes = BKNI_Snprintf( hVceCh->picture.stState.szMessage, BVCE_CHANNEL_MAX_MESSAGE_SIZE,
                  "[%d] %s: %s\n"
                  "         Current     Lst Recv'd  Lst Enq'd  Delta\n"
                  " PicID : 0x%08x  0x%08x  0x%08x (%4d %4d pic(s))\n"
                  " PicSTC: 0x%08x  0x%08x  0x%08x (%4d %4d ms)\n"
                  " EnqSTC: 0x%08x  0x%08x  0x%08x (%4d %4d ms)\n"
                  " PicPol: %s %s %s\n"
                  " Recv'd: %d pics\n"
                  " Enq'd : %d pics\n"
                  " FRC   : %d pics\n"
                  " Drop  : %d pics\n",
                  hVceCh->stOpenSettings.uiInstance,
                  ( true == bError ) ? "ERROR" : "WARNING",
                  szError,
                  pstPicture->ulPictureId,
                  pstPicturePreviousReceived->ulPictureId,
                  pstPicturePreviousEncoded->ulPictureId,
                  pstPicture->ulPictureId - pstPicturePreviousReceived->ulPictureId,
                  pstPicture->ulPictureId - pstPicturePreviousEncoded->ulPictureId,

                  pstPicture->ulSTCSnapshotLo,
                  pstPicturePreviousReceived->ulSTCSnapshotLo,
                  pstPicturePreviousEncoded->ulSTCSnapshotLo,
                  (pstPicture->ulSTCSnapshotLo - pstPicturePreviousReceived->ulSTCSnapshotLo)/27000,
                  (pstPicture->ulSTCSnapshotLo - pstPicturePreviousEncoded->ulSTCSnapshotLo)/27000,

                  uiSTCSnapshotLo,
                  hVceCh->picture.stState.stPreviousReceived.uiSTCSnapshotLo,
                  hVceCh->picture.stState.stPreviousEncoded.uiSTCSnapshotLo,
                  (uiSTCSnapshotLo - hVceCh->picture.stState.stPreviousReceived.uiSTCSnapshotLo)/27000,
                  (uiSTCSnapshotLo - hVceCh->picture.stState.stPreviousEncoded.uiSTCSnapshotLo)/27000,

                  BVCE_S_PolarityFriendlyNameLUT[pstPicture->ePolarity],
                  BVCE_S_PolarityFriendlyNameLUT[pstPicturePreviousReceived->ePolarity],
                  BVCE_S_PolarityFriendlyNameLUT[pstPicturePreviousEncoded->ePolarity],

                  hVceCh->picture.stState.stats.uiNumReceived,
                  hVceCh->picture.stState.stats.uiNumSentToVice,
                  hVceCh->picture.stState.stats.uiNumDroppedDueToFRC,
                  hVceCh->picture.stState.stats.uiNumDroppedDueToError
               );
   if ( iBytes == BVCE_CHANNEL_MAX_MESSAGE_SIZE ) BDBG_WRN(("Possible Message Truncation"));

   if ( true == bError )
   {
      BDBG_ERR(("%s", hVceCh->picture.stState.szMessage));
   }
   else
   {
      BDBG_WRN(("%s", hVceCh->picture.stState.szMessage));
   }
}
#endif

BERR_Code
BVCE_Channel_Picture_Enqueue_isr(
         BVCE_Channel_Handle hVceCh,
         const BAVC_EncodePictureBuffer *pstPicture /* Pointer to picture info descriptor */
         )
{
#if (BVCE_P_VDC_MANAGE_VIP)
   if ( ( ( BVCE_P_Status_eStarted == hVceCh->eStatus )
          || ( BVCE_P_Status_eStopping == hVceCh->eStatus ) )
        && ( false == hVceCh->hVce->bWatchdogOccurred ) )
   {
      BERR_Code rc = BERR_SUCCESS;
      BVCE_P_PictureResult ePictureResult = BVCE_P_PictureResult_eEncode;
      BVCE_P_FrameRateConversionSettings stFrameRateConversionSettings;
      uint64_t uiNewPTSin90Khz = 0;
      uint32_t uiCurrentSTCin45Khz = 0;
      uint32_t uiSTCSnapshotLo = 0;

      BDBG_ENTER( BVCE_Channel_Picture_Enqueue_isr );
      BVCE_P_FUNCTION_TRACE_ENTER_isr( 1, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance );

      BDBG_ASSERT( hVceCh );
      BDBG_ASSERT( pstPicture );

      uiSTCSnapshotLo = BREG_Read32(
                   hVceCh->hVce->handles.hReg,
                   hVceCh->hVce->stPlatformConfig.stDebug.uiSTC[hVceCh->stOpenSettings.uiInstance]
                   );

      /* Ignore pictures that have the wrong polarity */
      if ( ( ( BAVC_ScanType_eProgressive == hVceCh->stStartEncodeSettings.eInputType )
             && ( BAVC_Polarity_eFrame != pstPicture->ePolarity ) )
           || ( ( BAVC_ScanType_eInterlaced == hVceCh->stStartEncodeSettings.eInputType )
                && ( BAVC_Polarity_eFrame == pstPicture->ePolarity ) )
           || ( ( BAVC_ScanType_eInterlaced == hVceCh->stStartEncodeSettings.eInputType ) /* If interlaced, we always want to start with a top field */
                && ( BAVC_Polarity_eBotField == pstPicture->ePolarity )
                && ( 0 == hVceCh->picture.stState.stats.uiNumReceived ) )
           )
      {
         ePictureResult = BVCE_P_PictureResult_eDropDueToPolarity;
      }

      if ( BVCE_P_PictureResult_eEncode == ePictureResult )
      {
         BVCE_P_PictureBufferType ePictureBufferType;
         BKNI_Memset( &stFrameRateConversionSettings, 0, sizeof( stFrameRateConversionSettings ) );
         stFrameRateConversionSettings.eEffectiveFrameRate = pstPicture->eFrameRate;

         /* Seed the New PTS */
         if ( false == hVceCh->picture.stState.bNextNewPTSIn360Khz )
         {
            if ( true == hVceCh->stStartEncodeSettings.bNonRealTimeEncodeMode )
            {
               /* In NRT: PTS[0] = 0 */
               hVceCh->picture.stState.uiNextNewPTSIn360Khz = 0;
            }
            else
            {
               /* In RT: PTS[0] = STCSnapshot */
               uint64_t uiSTCSnapshot = pstPicture->ulSTCSnapshotHi;
               uiSTCSnapshot <<= 32;
               uiSTCSnapshot |= pstPicture->ulSTCSnapshotLo;
               hVceCh->picture.stState.uiNextNewPTSIn360Khz = uiSTCSnapshot / 75;
            }
            hVceCh->picture.stState.bNextNewPTSIn360Khz = true;
         }
         else if ( false == hVceCh->stStartEncodeSettings.bNonRealTimeEncodeMode )
         {
#define BVCE_P_MAX_STC_DIVERGENCE_IN_45Khz (45000*2)
            uint32_t uiNextNewPTSin45Khz = hVceCh->picture.stState.uiNextNewPTSIn360Khz / 8;
            uint32_t uiSTCSnapshotin45Khz = (uint32_t) ( ( ( (uint64_t) pstPicture->ulSTCSnapshotHi << 32 ) | pstPicture->ulSTCSnapshotLo ) / 600 );
            int32_t iDelta = uiSTCSnapshotin45Khz - uiNextNewPTSin45Khz;

            /* Check to make sure The algorithmic PTS is not diverging from the actual STC snapshot */
            if ( ( iDelta > BVCE_P_MAX_STC_DIVERGENCE_IN_45Khz )
                 || ( iDelta < -BVCE_P_MAX_STC_DIVERGENCE_IN_45Khz ) )
            {
               BDBG_ERR(("STC is diverging from algorithmic PTS (%d in 45Khz)", iDelta));
            }
         }

         /* Keep track of pictures dropped by VDC */
         if ( ( true == hVceCh->picture.stState.stPreviousReceived.bValid )
              && ( pstPicture->ulPictureId != hVceCh->picture.stState.stPreviousReceived.stPicture.ulPictureId )
              && ( pstPicture->ulPictureId != ( hVceCh->picture.stState.stPreviousReceived.stPicture.ulPictureId + 1 ) ) )
         {
            unsigned uiNumDroppedPics = ( ( pstPicture->ulPictureId - hVceCh->picture.stState.stPreviousReceived.stPicture.ulPictureId ) - 1);

            BVCE_Channel_S_Picture_Print_Debug_isr(
                  hVceCh,
                  pstPicture,
                  uiSTCSnapshotLo,
                  "Display dropped pictures",
                  (true == hVceCh->stStartEncodeSettings.bNonRealTimeEncodeMode)
                  );

            hVceCh->picture.stState.stats.uiNumDroppedDueToError += uiNumDroppedPics;
            hVceCh->picture.stState.uiNextNewPTSIn360Khz += BVCE_P_PI2FW_FrameRate2DeltaPtsLUT[pstPicture->eFrameRate] * uiNumDroppedPics;
         }

         /* Set the new PTS for this picture */
         {
            uint64_t uiNewPTSLocalin90Khz = hVceCh->picture.stState.uiNextNewPTSIn360Khz/4;
            /* Generate PTS */
            uiNewPTSin90Khz = ( uiNewPTSLocalin90Khz >> 32 ) & 0x1;
            uiNewPTSin90Khz <<= 32;
            uiNewPTSin90Khz |= uiNewPTSLocalin90Khz & 0xFFFFFFFF;
         }
         uiCurrentSTCin45Khz = uiNewPTSin90Khz/2;

         if ( true == hVceCh->stEncodeSettings.stFrameRate.bSparseFrameRateMode ) /* Sparse Mode */
         {
            if ( ( true == hVceCh->picture.stState.bLastOriginalPTSValid )
                 && ( pstPicture->ulOriginalPTS == hVceCh->picture.stState.ulLastOriginalPTS ) )
            {
               /* Drop the frame */
               ePictureResult = BVCE_P_PictureResult_eDropDueToFRC;
            }
         }
         else if ( BAVC_Polarity_eFrame == pstPicture->ePolarity ) /* Frame Rate Conversion */
         {
            rc = BERR_TRACE( BVCE_P_FrameRateConversionLUT_isrsafe( pstPicture->eFrameRate, hVceCh->stEncodeSettings.stFrameRate.eFrameRate, &stFrameRateConversionSettings ) );
            if ( BERR_SUCCESS == rc )
            {
               uint32_t uiTargetPTSin45Khz = hVceCh->picture.stState.uiNextTargetPTSin360Khz/8;
               int32_t iDeltaTargetPtsStc = (int32_t) (uiTargetPTSin45Khz - uiCurrentSTCin45Khz);

               /* For 60 --> 24 conversion, make sure STC = PTS when Locked to 3:2 and in Phase 0 */
               if ( ( true == stFrameRateConversionSettings.bIs60to24 )
                     && ( BAVC_CadenceType_e3_2 == pstPicture->stCadence.type )
                     && ( BAVC_CadencePhase_e0 == pstPicture->stCadence.phase )
                     && ( ( iDeltaTargetPtsStc > 2 ) || ( iDeltaTargetPtsStc < -2 ) ) ) /* +/- 2 is to account for STC jitter */
               {
                  BDBG_MSG(("3:2 Phase 0 TargetPTS Reset"));
                  hVceCh->picture.stState.bNextTargetPTSin360KhzValid = false;
               }

               if ( false == hVceCh->picture.stState.bNextTargetPTSin360KhzValid )
               {
                  /* Seed the next target PTS to equal the current STC */
                  BDBG_MSG(("Reset TargetPTS %#x --> %#x", uiTargetPTSin45Khz, uiCurrentSTCin45Khz));
                  uiTargetPTSin45Khz = uiCurrentSTCin45Khz;
                  hVceCh->picture.stState.uiNextTargetPTSin360Khz = uiCurrentSTCin45Khz*8;
                  hVceCh->picture.stState.bNextTargetPTSin360KhzValid = true;
               }

               /* For 60 --> 24 conversion, override the uiNewPTS */
               if ( true == stFrameRateConversionSettings.bIs60to24 )
               {
                  uint64_t uiNewPTSLocalin90Khz = hVceCh->picture.stState.uiNextTargetPTSin360Khz/4;
                  /* Generate PTS */
                  uiNewPTSin90Khz = ( uiNewPTSLocalin90Khz >> 32 ) & 0x1;
                  uiNewPTSin90Khz <<= 32;
                  uiNewPTSin90Khz |= uiNewPTSLocalin90Khz & 0xFFFFFFFF;
               }

               if ( ((int32_t) (uiTargetPTSin45Khz - uiCurrentSTCin45Khz)) > 2) /* 2 is to account for STC jitter */
               {
                  /* Drop the frame */
                  ePictureResult = BVCE_P_PictureResult_eDropDueToFRC;
                  BDBG_MSG(("D: stc:%#x < targetPts:%#x", uiCurrentSTCin45Khz, uiTargetPTSin45Khz));
               }
               else
               {
                  BDBG_MSG(("E: stc:%#x >= targetPts:%#x", uiCurrentSTCin45Khz, uiTargetPTSin45Khz));
                  hVceCh->picture.stState.uiNextTargetPTSin360Khz += BVCE_P_PI2FW_FrameRate2DeltaPtsLUT[stFrameRateConversionSettings.eEffectiveFrameRate];
               }
            }
            else
            {
               BDBG_ERR(("[%d] ERROR: Invalid frame rate conversion (%u --> %u)", hVceCh->stOpenSettings.uiInstance, pstPicture->eFrameRate, hVceCh->stEncodeSettings.stFrameRate.eFrameRate));
               ePictureResult = BVCE_P_PictureResult_eDropDueToError;
            }
         }
         else
         {
            /* Check if we've received the same polarity from VDC twice */
            if ( ( true == hVceCh->picture.stState.stPreviousReceived.bValid )
                 && ( hVceCh->picture.stState.stPreviousReceived.stPicture.ePolarity == pstPicture->ePolarity )
                 && ( ( hVceCh->picture.stState.stPreviousReceived.stPicture.ulPictureId + 1 ) == pstPicture->ulPictureId ) )
            {
               /* If we've encoded the same polarity previously, then drop the frame and print an error */
               if ( ( true == hVceCh->picture.stState.stPreviousEncoded.bValid )
                    && ( pstPicture->ePolarity == hVceCh->picture.stState.stPreviousEncoded.stPicture.ePolarity ) )
               {
                  BVCE_Channel_S_Picture_Print_Debug_isr(
                        hVceCh,
                        pstPicture,
                        uiSTCSnapshotLo,
                        "Duplicate Polarity received",
                        false);
                  rc = BERR_TRACE(BERR_UNKNOWN);
                  ePictureResult = BVCE_P_PictureResult_eDropDueToPolarity;
               }
               else
               {
                  if ( false == hVceCh->stStartEncodeSettings.bNonRealTimeEncodeMode )
                  {
                     /* If we have not encoded the same polarity previously, then encode the frame and print a warning */
                     BVCE_Channel_S_Picture_Print_Debug_isr(
                           hVceCh,
                           pstPicture,
                           uiSTCSnapshotLo,
                           "Duplicate Polarity received",
                           false);
                  }
               }
            }

            if ( BVCE_P_PictureResult_eEncode == ePictureResult )
            {
               /* Check if we've encoded the same polarity previously */
               if ( ( true == hVceCh->picture.stState.stPreviousEncoded.bValid )
                    && ( pstPicture->ePolarity == hVceCh->picture.stState.stPreviousEncoded.stPicture.ePolarity ) )
               {
                  BVCE_Channel_S_Picture_Print_Debug_isr(
                        hVceCh,
                        pstPicture,
                        uiSTCSnapshotLo,
                        "Duplicate Polarity encoded",
                        false
                        );
                  rc = BERR_TRACE(BERR_UNKNOWN);
                  ePictureResult = BVCE_P_PictureResult_eDropDueToPolarity;
               }
            }
         }

         hVceCh->picture.stState.bLastOriginalPTSValid = true;
         hVceCh->picture.stState.ulLastOriginalPTS = pstPicture->ulOriginalPTS;

         if ( BVCE_P_PictureResult_eEncode == ePictureResult )
         {
            /* Check if LUT is full */
            for ( ePictureBufferType = 0; ePictureBufferType < BVCE_P_PictureBufferType_eMax; ePictureBufferType++ )
            {
               if ( BVCE_P_PICTURE_OFFSET_LUT_SIZE == hVceCh->picture.stState.astLUT[ePictureBufferType].uiNumEntries )
               {
                  if ( false == hVceCh->stStartEncodeSettings.bNonRealTimeEncodeMode )
                  {
                     BDBG_ERR(("[%d] ERROR: PI LUT full in RT mode for pic id %u (%u) (deltaSTC = %u ms elapsed = %u ms)", hVceCh->stOpenSettings.uiInstance, pstPicture->ulPictureId, hVceCh->picture.stState.stats.uiNumReceived,
                           (pstPicture->ulSTCSnapshotLo - hVceCh->picture.stState.stPreviousEncoded.stPicture.ulSTCSnapshotLo)/27000,
                           (uiSTCSnapshotLo - hVceCh->picture.stState.stPreviousEncoded.uiSTCSnapshotLo)/27000
                           ));
                     rc = BERR_TRACE(BERR_UNKNOWN);
                     ePictureResult = BVCE_P_PictureResult_eDropDueToError;
                  }
                  else
                  {
                     /* If in NRT mode, we just wait */
                     ePictureResult = BVCE_P_PictureResult_eWait;
                  }
                  break;
               }
            }
         }

         if ( BVCE_P_PictureResult_eEncode == ePictureResult )
         {
   #ifdef BVCE_P_PIC_LOOPBACK_TEST_MODE
            /* Force mailbox bit to not be busy so that we always "queue" the picture when in loopback test mode */
            hVceCh->picture.stState.stPictureBufferMailbox.uiMetadata = 0;
   #else
            /* Read the FW Mailbox Info */
            {
               unsigned uiMetadataOffset = ( (uint8_t*) (&hVceCh->picture.stState.stPictureBufferMailbox.uiMetadata) - (uint8_t*) (&hVceCh->picture.stState.stPictureBufferMailbox) );

               /* Read the metadata field containing the busy flag */
               /* coverity[address_of] */
               /* coverity[callee_ptr_arith] */
               BVCE_P_ReadRegisters_isr(
                        hVceCh->hVce,
                        hVceCh->hVce->stPlatformConfig.stPictureMailbox[hVceCh->stOpenSettings.uiInstance].uiBufferAddress + uiMetadataOffset,
                        (uint32_t*) (&hVceCh->picture.stState.stPictureBufferMailbox.uiMetadata),
                        sizeof( hVceCh->picture.stState.stPictureBufferMailbox.uiMetadata )
                        );
            }
   #endif

            /* Check if the busy flag is set in the BVN2VICE MBOX */
            if ( 0 == ( hVceCh->picture.stState.stPictureBufferMailbox.uiMetadata & BVCE_P_FW_PictureBufferMailbox_Metadata_BUSY_MASK ) )
            {
               BKNI_Memset( &hVceCh->picture.stState.stPictureBufferMailbox, 0, sizeof( BVCE_P_FW_PictureBufferMailbox ) );

               BDBG_ASSERT( true == pstPicture->bStriped );
               /* Unused Fields
                *  - ulStripeWidth
                *  - ulLumaNMBY
                *  - ulChromaNMBY
                *  - ulWidthInMbs
                *  - ulHeightInMbs
                */

               /* Fill on MBOX Info */
               /************/
               /* Metadata */
               /************/
               /* Busy Flag */
               hVceCh->picture.stState.stPictureBufferMailbox.uiMetadata |= BVCE_P_FW_PictureBufferMailbox_Metadata_BUSY_MASK;

               /* Channel Change Flag */
               hVceCh->picture.stState.stPictureBufferMailbox.uiMetadata |= pstPicture->bChannelChange ? BVCE_P_FW_PictureBufferMailbox_Metadata_CHANNEL_CHANGE_MASK : 0;

               /* Last Flag */
               hVceCh->picture.stState.stPictureBufferMailbox.uiMetadata |= pstPicture->bLast ? BVCE_P_FW_PictureBufferMailbox_Metadata_CHANNEL_CHANGE_MASK : 0;

               /****************/
               /* Picture Info */
               /****************/
               {
                  /* We compute the cropping info based on the resolution rounded up to 8x8 blocks */
                  unsigned uiHorizontalPadding = 0;
                  unsigned uiVerticalPadding = 0;

                  if ( 0 != ( pstPicture->ulWidth % 8 ) )
                  {
                     uiHorizontalPadding = 8 - ( pstPicture->ulWidth % 8 );
                  }

                  if ( 0 != ( pstPicture->ulHeight % 8 ) )
                  {
                     uiVerticalPadding = 8 - ( pstPicture->ulHeight % 8 );
                  }

                  /* Resolution */
                  hVceCh->picture.stState.stPictureBufferMailbox.uiResolution |= ( ( ( ( pstPicture->ulWidth + uiHorizontalPadding ) / 8 ) << BVCE_P_FW_PictureBufferMailbox_Resolution_HORIZONTAL_SIZE_IN_8x8_BLOCKS_SHIFT ) & BVCE_P_FW_PictureBufferMailbox_Resolution_HORIZONTAL_SIZE_IN_8x8_BLOCKS_MASK );
                  hVceCh->picture.stState.stPictureBufferMailbox.uiResolution |= ( ( ( ( pstPicture->ulHeight + uiVerticalPadding ) / 8 ) << BVCE_P_FW_PictureBufferMailbox_Resolution_VERTICAL_SIZE_IN_8x8_BLOCKS_SHIFT ) & BVCE_P_FW_PictureBufferMailbox_Resolution_VERTICAL_SIZE_IN_8x8_BLOCKS_MASK );

                  /* Cropping */
                  hVceCh->picture.stState.stPictureBufferMailbox.uiCropping |= ( ( uiHorizontalPadding << BVCE_P_FW_PictureBufferMailbox_Cropping_HORIZONTAL_SIZE_SHIFT ) & BVCE_P_FW_PictureBufferMailbox_Cropping_HORIZONTAL_SIZE_MASK );
                  hVceCh->picture.stState.stPictureBufferMailbox.uiCropping |= ( ( uiVerticalPadding << BVCE_P_FW_PictureBufferMailbox_Cropping_VERTICAL_SIZE_SHIFT ) & BVCE_P_FW_PictureBufferMailbox_Cropping_VERTICAL_SIZE_MASK );
               }

               /* Sample Aspect Ratio */
               hVceCh->picture.stState.stPictureBufferMailbox.uiSampleAspectRatio |= ( ( pstPicture->ulAspectRatioX << BVCE_P_FW_PictureBufferMailbox_SampleAspectRatio_HORIZONTAL_SIZE_SHIFT ) & BVCE_P_FW_PictureBufferMailbox_SampleAspectRatio_HORIZONTAL_SIZE_MASK );
               hVceCh->picture.stState.stPictureBufferMailbox.uiSampleAspectRatio |= ( ( pstPicture->ulAspectRatioY << BVCE_P_FW_PictureBufferMailbox_SampleAspectRatio_VERTICAL_SIZE_SHIFT ) & BVCE_P_FW_PictureBufferMailbox_SampleAspectRatio_VERTICAL_SIZE_MASK );

               /* Bar Data */
               hVceCh->picture.stState.stPictureBufferMailbox.uiBarData |= ( ( BVCE_P_PI2FW_BarDataTypeLUT[pstPicture->stBarData.eType] << BVCE_P_FW_PictureBufferMailbox_BarData_TYPE_SHIFT ) & BVCE_P_FW_PictureBufferMailbox_BarData_TYPE_MASK );
               hVceCh->picture.stState.stPictureBufferMailbox.uiBarData |= ( ( pstPicture->stBarData.uiBotRight << BVCE_P_FW_PictureBufferMailbox_BarData_BOTTOM_RIGHT_VALUE_SHIFT ) & BVCE_P_FW_PictureBufferMailbox_BarData_BOTTOM_RIGHT_VALUE_MASK );
               hVceCh->picture.stState.stPictureBufferMailbox.uiBarData |= ( ( pstPicture->stBarData.uiTopLeft << BVCE_P_FW_PictureBufferMailbox_BarData_TOP_LEFT_VALUE_SHIFT ) & BVCE_P_FW_PictureBufferMailbox_BarData_TOP_LEFT_VALUE_MASK );

               /* Polarity */
               hVceCh->picture.stState.stPictureBufferMailbox.uiFormatInfo |= ( ( BVCE_P_PI2FW_PolarityLUT[pstPicture->ePolarity] << BVCE_P_FW_PictureBufferMailbox_FormatInfo_POLARITY_SHIFT ) & BVCE_P_FW_PictureBufferMailbox_FormatInfo_POLARITY_MASK );

               /* Cadence */
               {
                  BAVC_CadenceType eType = pstPicture->stCadence.type;
                  BAVC_CadencePhase ePhase = pstPicture->stCadence.phase;

                  hVceCh->picture.stState.stPictureBufferMailbox.uiFormatInfo |= ( ( BVCE_P_PI2FW_CadenceTypeLUT[eType] << BVCE_P_FW_PictureBufferMailbox_FormatInfo_CADENCE_TYPE_SHIFT ) & BVCE_P_FW_PictureBufferMailbox_FormatInfo_CADENCE_TYPE_MASK );
                  hVceCh->picture.stState.stPictureBufferMailbox.uiFormatInfo |= ( ( BVCE_P_PI2FW_CadencePhaseLUT[ePhase] << BVCE_P_FW_PictureBufferMailbox_FormatInfo_CADENCE_PHASE_SHIFT ) & BVCE_P_FW_PictureBufferMailbox_FormatInfo_CADENCE_PHASE_MASK );
               }


               /* Active Format Description (AFD) */
               hVceCh->picture.stState.stPictureBufferMailbox.uiFormatInfo |= pstPicture->stAFD.bValid ? BVCE_P_FW_PictureBufferMailbox_FormatInfo_AFD_VALID_MASK : 0;
               hVceCh->picture.stState.stPictureBufferMailbox.uiFormatInfo |= ( ( pstPicture->stAFD.uiMode << BVCE_P_FW_PictureBufferMailbox_FormatInfo_AFD_MODE_SHIFT ) & BVCE_P_FW_PictureBufferMailbox_FormatInfo_AFD_MODE_MASK );

               /* Orientation */
               hVceCh->picture.stState.stPictureBufferMailbox.uiFormatInfo |= ( ( BVCE_P_PI2FW_OrientationLUT[pstPicture->eOrientation] << BVCE_P_FW_PictureBufferMailbox_FormatInfo_ORIENTATION_SHIFT ) & BVCE_P_FW_PictureBufferMailbox_FormatInfo_ORIENTATION_MASK );

               /**********/
               /* Timing */
               /**********/
               /* Frame Rate */
               hVceCh->picture.stState.stPictureBufferMailbox.uiTimingInfo |= ( ( BVCE_P_PI2FW_FrameRate2DeltaPtsLUT[stFrameRateConversionSettings.eEffectiveFrameRate] << BVCE_P_FW_PictureBufferMailbox_TimingInfo_DELTA_PTS_IN_360KHZ_SHIFT ) & BVCE_P_FW_PictureBufferMailbox_TimingInfo_DELTA_PTS_IN_360KHZ_MASK );

               /* New PTS */
               hVceCh->picture.stState.stPictureBufferMailbox.uiTimingInfo |= ( ( uiNewPTSin90Khz << BVCE_P_FW_PictureBufferMailbox_TimingInfo_NEW_PTS_LSB_SHIFT ) & BVCE_P_FW_PictureBufferMailbox_TimingInfo_NEW_PTS_LSB_MASK );
               hVceCh->picture.stState.stPictureBufferMailbox.uiNewPts = (uint32_t) (uiNewPTSin90Khz >> 1);

               /* Original PTS */
               hVceCh->picture.stState.stPictureBufferMailbox.uiOriginalPts = pstPicture->ulOriginalPTS;

               /* Picture ID */
               hVceCh->picture.stState.stPictureBufferMailbox.uiPictureId = pstPicture->ulPictureId;

               /* STC Snapshot */
               hVceCh->picture.stState.stPictureBufferMailbox.uiSTCSnapshotLo = pstPicture->ulSTCSnapshotLo;
               hVceCh->picture.stState.stPictureBufferMailbox.uiSTCSnapshotHi = pstPicture->ulSTCSnapshotHi;

               /* Set Buffer Info */
               for ( ePictureBufferType = 0; ePictureBufferType < BVCE_P_PictureBufferType_eMax; ePictureBufferType++ )
               {
                  unsigned i;
                  BVCE_P_PictureBlockInfo* pstPictureBlockInfo = NULL;

                  /* Find free spot in LUT */
                  for ( i = 0; i < BVCE_P_PICTURE_OFFSET_LUT_SIZE; i++ )
                  {
                     if ( false == hVceCh->picture.stState.astLUT[ePictureBufferType].astEntry[i].bInUse )
                     {
                        pstPictureBlockInfo = &hVceCh->picture.stState.astLUT[ePictureBufferType].astEntry[i].blockInfo;
                        break;
                     }
                  }
                  BDBG_ASSERT( i < BVCE_P_PICTURE_OFFSET_LUT_SIZE );

                  if ( true == BVCE_S_AcquirePictureBlockInfoFromEncodePictureBuffer_isrsafe( pstPictureBlockInfo, pstPicture, ePictureBufferType ) )
                  {
                     /* Sanity check to make sure buffer isn't already in LUT */
                     {
                        unsigned j;
                        for ( j = 0; j < BVCE_P_PICTURE_OFFSET_LUT_SIZE; j++ )
                        {
                           if ( true == hVceCh->picture.stState.astLUT[ePictureBufferType].astEntry[j].bInUse )
                           {
                              if ( hVceCh->picture.stState.astLUT[ePictureBufferType].astEntry[j].blockInfo.uiPhysicalOffset == pstPictureBlockInfo->uiPhysicalOffset )
                              {
                                 BDBG_ERR(("Buffer Queued Twice!"));
                              }
                           }
                        }
                     }

                     hVceCh->picture.stState.astLUT[ePictureBufferType].astEntry[i].bInUse = true;
                     hVceCh->picture.stState.astLUT[ePictureBufferType].astEntry[i].ePictureBufferType = ePictureBufferType;
                     hVceCh->picture.stState.astLUT[ePictureBufferType].uiNumEntries++;
                     BVCE_S_SetPictureBufferInfoFromPictureBlockInfo_isrsafe( &hVceCh->picture.stState.stPictureBufferMailbox, pstPictureBlockInfo, ePictureBufferType );
                  }
               }

#ifndef BVCE_P_PIC_LOOPBACK_TEST_MODE
               /* "DMA" MBOX to VICE DCCM */
               BVCE_P_WriteRegisters_isr(
                        hVceCh->hVce,
                        hVceCh->hVce->stPlatformConfig.stPictureMailbox[hVceCh->stOpenSettings.uiInstance].uiBufferAddress,
                        (uint32_t*) (&hVceCh->picture.stState.stPictureBufferMailbox),
                        sizeof( BVCE_P_FW_PictureBufferMailbox )
                        );

               /* Trigger BVN2VICE MBOX Interrupt */
               BDBG_ASSERT( 0 != hVceCh->hVce->stPlatformConfig.stPictureMailbox[hVceCh->stOpenSettings.uiInstance].uiInterruptAddress );

               BREG_Write32(
                  hVceCh->hVce->handles.hReg,
                  hVceCh->hVce->stPlatformConfig.stPictureMailbox[hVceCh->stOpenSettings.uiInstance].uiInterruptAddress,
                  hVceCh->hVce->stPlatformConfig.stPictureMailbox[hVceCh->stOpenSettings.uiInstance].uiInterruptMask
               );
#endif
               rc = BERR_SUCCESS;
            }
            else if ( false == hVceCh->stStartEncodeSettings.bNonRealTimeEncodeMode )
            {
               /* If in RT mode, we should always have space in the FW queue, so we need to drop this picture and report an error */
               BDBG_ERR(("[%d] ERROR: FW Mailbox full in RT mode for pic id %u (%u) (deltaSTC = %u ms elapsed = %u ms)", hVceCh->stOpenSettings.uiInstance, pstPicture->ulPictureId, hVceCh->picture.stState.stats.uiNumReceived,
                     (pstPicture->ulSTCSnapshotLo - hVceCh->picture.stState.stPreviousEncoded.stPicture.ulSTCSnapshotLo)/27000,
                     (uiSTCSnapshotLo - hVceCh->picture.stState.stPreviousEncoded.uiSTCSnapshotLo)/27000
               ));
               rc = BERR_TRACE(BERR_UNKNOWN);
               ePictureResult = BVCE_P_PictureResult_eDropDueToError;
            }
            else
            {
               /* If in NRT mode, we just wait */
               ePictureResult = BVCE_P_PictureResult_eWait;
            }
         }
      }

      switch ( ePictureResult )
      {
         case BVCE_P_PictureResult_eDropDueToError:
         case BVCE_P_PictureResult_eDropDueToFRC:
         case BVCE_P_PictureResult_eDropDueToPolarity:
            rc = BVCE_S_Channel_Picture_Drop_Enqueue_isr( hVceCh, pstPicture );
            if ( BERR_SUCCESS != rc )
            {
               if ( true == hVceCh->stStartEncodeSettings.bNonRealTimeEncodeMode )
               {
                  /* If in NRT mode, we just wait */
                  ePictureResult = BVCE_P_PictureResult_eWait;
               }
               else
               {
                  ePictureResult = BVCE_P_PictureResult_eError;
                  rc = BERR_TRACE(BERR_UNKNOWN);
               }
            }
            break;
         default:
            break;
      }

      BVCE_P_FUNCTION_TRACE_LEAVE_isr( 1, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance );
      BDBG_LEAVE( BVCE_Channel_Picture_Enqueue_isr );

      /* Update Stats */
      switch ( ePictureResult )
      {
         case BVCE_P_PictureResult_eDropDueToError:
            hVceCh->picture.stState.stats.uiNumReceived++;
            hVceCh->picture.stState.stats.uiNumDroppedDueToError++;
            BVCE_P_DEBUG_ENTRY_isr( hVceCh->hVce, ePictureBufferDropEnqueue, hVceCh->stOpenSettings.uiInstance, *pstPicture );
            break;

         case BVCE_P_PictureResult_eDropDueToFRC:
            hVceCh->picture.stState.stats.uiNumReceived++;
            hVceCh->picture.stState.stats.uiNumDroppedDueToFRC++;
            BVCE_P_DEBUG_ENTRY_isr( hVceCh->hVce, ePictureBufferDropEnqueue, hVceCh->stOpenSettings.uiInstance, *pstPicture );
            break;

         case BVCE_P_PictureResult_eEncode:
            hVceCh->picture.stState.stats.uiNumReceived++;
            hVceCh->picture.stState.stats.uiNumSentToVice++;
            hVceCh->picture.stState.stPreviousEncoded.stPicture = *pstPicture;
            hVceCh->picture.stState.stPreviousEncoded.uiSTCSnapshotLo = uiSTCSnapshotLo;
            hVceCh->picture.stState.stPreviousEncoded.bValid = true;
            BVCE_P_DEBUG_ENTRY_isr( hVceCh->hVce, ePictureBufferEnqueue, hVceCh->stOpenSettings.uiInstance, hVceCh->picture.stState.stPictureBufferMailbox );
            break;

         case BVCE_P_PictureResult_eDropDueToPolarity:
            if ( 0 != hVceCh->picture.stState.stats.uiNumReceived )
            {
               hVceCh->picture.stState.stats.uiNumReceived++;
               hVceCh->picture.stState.stats.uiNumDroppedDueToError++;
               BDBG_WRN(("Drop due to Polarity Mismatch"));
            }
            BVCE_P_DEBUG_ENTRY_isr( hVceCh->hVce, ePictureBufferDropEnqueue, hVceCh->stOpenSettings.uiInstance, *pstPicture );
            break;

         case BVCE_P_PictureResult_eWait:
            return BERR_NOT_AVAILABLE;
            break;

         default:
            break;
      }

      if ( BVCE_P_PictureResult_eWait != ePictureResult )
      {
         hVceCh->picture.stState.uiNextNewPTSIn360Khz += BVCE_P_PI2FW_FrameRate2DeltaPtsLUT[pstPicture->eFrameRate];
         hVceCh->picture.stState.stPreviousReceived.stPicture = *pstPicture;
         hVceCh->picture.stState.stPreviousReceived.uiSTCSnapshotLo = uiSTCSnapshotLo;
         hVceCh->picture.stState.stPreviousReceived.bValid = true;
      }

      return BERR_TRACE( rc );
   }
   else
   {
      return BERR_NOT_AVAILABLE;
   }
#else
   BSTD_UNUSED(hVceCh);
   BSTD_UNUSED(pstPicture);
   return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
}

BERR_Code
BVCE_Channel_Picture_Dequeue_isr(
      BVCE_Channel_Handle hVceCh,
      BAVC_EncodePictureBuffer *pstPicture
      )
{
#if (BVCE_P_VDC_MANAGE_VIP)
   BERR_Code rc = BERR_NOT_AVAILABLE;
   BVCE_P_PictureBufferType ePictureBufferType;
   bool bDropFrame = false;

   BDBG_ENTER( BVCE_Channel_Picture_Dequeue_isr );
   BVCE_P_FUNCTION_TRACE_ENTER_isr( 1, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance );

   BDBG_ASSERT( hVceCh );
   BDBG_ASSERT( pstPicture );

   BKNI_Memset( pstPicture, 0, sizeof( BAVC_EncodePictureBuffer) );
   BKNI_Memset( &hVceCh->picture.stState.stPictureBufferMailbox, 0, sizeof( BVCE_P_FW_PictureBufferMailbox ) );

   {
      rc = BVCE_S_Channel_Picture_Drop_Dequeue_isr( hVceCh, pstPicture );

      if ( rc == BERR_SUCCESS )
      {
         bDropFrame = true;
      }
   }

   if ( BERR_NOT_AVAILABLE == rc )
   {
      /* Loop through each release queue */
      for ( ePictureBufferType = 0; ePictureBufferType < BVCE_P_PictureBufferType_eMax; ePictureBufferType++ )
      {
#ifdef BVCE_P_PIC_LOOPBACK_TEST_MODE
         /* Write fake queue entry for this buffer type */
         /* Zero out temp local buffer queue */
         BKNI_Memset( &hVceCh->picture.stState.stPictureBufferQueue, 0, sizeof( BVCE_P_FW_PictureBuffer_Queue ) );

         /* See if a buffer of this type is available in the LUT */
         {
            unsigned i;

            if ( 0 != hVceCh->picture.stState.astLUT[ePictureBufferType].uiNumEntries )
            {
               /* Find physical buffer address in LUT */
               for ( i = 0; i < BVCE_P_PICTURE_OFFSET_LUT_SIZE; i++ )
               {
                  if ( true == hVceCh->picture.stState.astLUT[ePictureBufferType].astEntry[i].bInUse )
                  {
                     if ( hVceCh->picture.stState.astLUT[ePictureBufferType].astEntry[i].ePictureBufferType == ePictureBufferType )
                     {
                        hVceCh->picture.stState.stPictureBufferQueue.astQueue[hVceCh->picture.stState.stPictureBufferQueue.uiReadOffset].uiOffset = hVceCh->picture.stState.astLUT[ePictureBufferType].astEntry[i].blockInfo.uiPhysicalOffset;
                        hVceCh->picture.stState.stPictureBufferQueue.uiWriteOffset++;
                        break;
                     }
                  }
               }
            }
         }
#else
         if ( ( false == hVceCh->hVce->bWatchdogOccurred )
              &&  ( ( BVCE_P_Status_eStarted == hVceCh->eStatus )
                    || ( BVCE_P_Status_eStopping == hVceCh->eStatus ) ) )

         {
            /* Read the Picture Buffer Queue Info */
            BVCE_P_ReadRegisters_isr(
                     hVceCh->hVce,
                     hVceCh->picture.dccm.auiPictureReleaseQueueInfoAddress[ePictureBufferType],
                     (uint32_t*) (&hVceCh->picture.stState.stPictureBufferQueue),
                     sizeof( BVCE_P_FW_PictureBuffer_Queue )
                     );
         }
         else
         {
            if ( 0 == hVceCh->picture.stState.astLUT[ePictureBufferType].uiNumEntries )
            {
               hVceCh->picture.stState.stPictureBufferQueue.uiWriteOffset = hVceCh->picture.stState.stPictureBufferQueue.uiReadOffset;
            }
            else
            {
               unsigned i;

               hVceCh->picture.stState.stPictureBufferQueue.uiWriteOffset = ( hVceCh->picture.stState.stPictureBufferQueue.uiReadOffset + 1 ) % BVCE_P_FW_PICTURE_QUEUE_LENGTH;

               for ( i = 0; i < BVCE_P_PICTURE_OFFSET_LUT_SIZE; i++ )
               {
                  if ( ( true == hVceCh->picture.stState.astLUT[ePictureBufferType].astEntry[i].bInUse )
                       && ( hVceCh->picture.stState.astLUT[ePictureBufferType].astEntry[i].ePictureBufferType == ePictureBufferType ) )
                  {
                     hVceCh->picture.stState.stPictureBufferQueue.astQueue[hVceCh->picture.stState.stPictureBufferQueue.uiReadOffset].uiOffsetHi = ( hVceCh->picture.stState.astLUT[ePictureBufferType].astEntry[i].blockInfo.uiPhysicalOffset >> 32 ) & 0xFFFFFFFF;
                     hVceCh->picture.stState.stPictureBufferQueue.astQueue[hVceCh->picture.stState.stPictureBufferQueue.uiReadOffset].uiOffsetLo = hVceCh->picture.stState.astLUT[ePictureBufferType].astEntry[i].blockInfo.uiPhysicalOffset & 0xFFFFFFFF;
                  }
               }
            }
         }
#endif

         /* Check read/write ptrs to see if data exists */
         if ( hVceCh->picture.stState.stPictureBufferQueue.uiReadOffset != hVceCh->picture.stState.stPictureBufferQueue.uiWriteOffset )
         {
            unsigned i;
            BVCE_P_PictureBlockInfo* pstPictureBlockInfo = NULL;

            if ( 0 == hVceCh->picture.stState.astLUT[ePictureBufferType].uiNumEntries )
            {
               /* Handle scenario where the release queue is stale from a previous session */
               hVceCh->picture.stState.stPictureBufferQueue.uiReadOffset = hVceCh->picture.stState.stPictureBufferQueue.uiWriteOffset;
            }
            else
            {
               /* Find physical buffer address in LUT */
               for ( i = 0; i < BVCE_P_PICTURE_OFFSET_LUT_SIZE; i++ )
               {
                  if ( ( true == hVceCh->picture.stState.astLUT[ePictureBufferType].astEntry[i].bInUse )
                        && ( hVceCh->picture.stState.astLUT[ePictureBufferType].astEntry[i].ePictureBufferType == ePictureBufferType ) )
                  {
                     if ( ( hVceCh->picture.stState.stPictureBufferQueue.astQueue[hVceCh->picture.stState.stPictureBufferQueue.uiReadOffset].uiOffsetHi == ( ( hVceCh->picture.stState.astLUT[ePictureBufferType].astEntry[i].blockInfo.uiPhysicalOffset >> 32 ) & 0xFFFFFFFF ) )
                          && ( hVceCh->picture.stState.stPictureBufferQueue.astQueue[hVceCh->picture.stState.stPictureBufferQueue.uiReadOffset].uiOffsetLo == ( hVceCh->picture.stState.astLUT[ePictureBufferType].astEntry[i].blockInfo.uiPhysicalOffset & 0xFFFFFFFF ) ) )
                     {
                        /* We found the matching block */
                        pstPictureBlockInfo = &hVceCh->picture.stState.astLUT[ePictureBufferType].astEntry[i].blockInfo;
                        break;
                     }
                  }
               }
               BDBG_ASSERT( i < BVCE_P_PICTURE_OFFSET_LUT_SIZE );

               hVceCh->picture.stState.astLUT[ePictureBufferType].astEntry[i].bInUse = false;
               hVceCh->picture.stState.astLUT[ePictureBufferType].uiNumEntries--;
               BVCE_S_ReleasePictureBlockInfoToEncodePictureBuffer_isrsafe( pstPictureBlockInfo, pstPicture, ePictureBufferType );

               {
                  BVCE_P_FW_PictureBufferOffset *pstPictureBufferOffset = NULL;

                  switch ( ePictureBufferType )
                  {
                     case BVCE_P_PictureBufferType_eLuma:
                        pstPictureBufferOffset = &hVceCh->picture.stState.stPictureBufferMailbox.LumaBufPtr.stOffset;
                        break;

                     case BVCE_P_PictureBufferType_eChroma:
                        pstPictureBufferOffset = &hVceCh->picture.stState.stPictureBufferMailbox.ChromaBufPtr.stOffset;
                        break;

                     case BVCE_P_PictureBufferType_e1VLuma:
                        pstPictureBufferOffset = &hVceCh->picture.stState.stPictureBufferMailbox.Luma1VBufPtr.stOffset;
                        break;

                     case BVCE_P_PictureBufferType_e2VLuma:
                        pstPictureBufferOffset = &hVceCh->picture.stState.stPictureBufferMailbox.Luma2VBufPtr.stOffset;
                        break;

                     case BVCE_P_PictureBufferType_eShiftedChroma:
                        pstPictureBufferOffset = &hVceCh->picture.stState.stPictureBufferMailbox.ShiftedChromaBufPtr.stOffset;
                        break;

                     default:
                        BDBG_ASSERT(0);
                  }

                  *pstPictureBufferOffset = hVceCh->picture.stState.stPictureBufferQueue.astQueue[hVceCh->picture.stState.stPictureBufferQueue.uiReadOffset];
               }

               /* Update read pointer */
               hVceCh->picture.stState.stPictureBufferQueue.uiReadOffset++;
               hVceCh->picture.stState.stPictureBufferQueue.uiReadOffset %= BVCE_P_FW_PICTURE_QUEUE_LENGTH;

               rc = BERR_SUCCESS;
            }
#ifndef BVCE_P_PIC_LOOPBACK_TEST_MODE
            {
               unsigned uiReadOffset = ( (uint8_t*) (&hVceCh->picture.stState.stPictureBufferQueue.uiReadOffset) - (uint8_t*) (&hVceCh->picture.stState.stPictureBufferQueue) );

               /* coverity[address_of] */
               /* coverity[callee_ptr_arith] */
               BVCE_P_WriteRegisters_isr(
                        hVceCh->hVce,
                        hVceCh->picture.dccm.auiPictureReleaseQueueInfoAddress[ePictureBufferType] + uiReadOffset,
                        (uint32_t*) (&hVceCh->picture.stState.stPictureBufferQueue.uiReadOffset),
                        sizeof( hVceCh->picture.stState.stPictureBufferQueue.uiReadOffset )
                        );
            }
#endif
         }
      }
   }

   BVCE_P_FUNCTION_TRACE_LEAVE_isr( 1, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance );
   BDBG_LEAVE( BVCE_Channel_Picture_Dequeue_isr );

   if ( BERR_NOT_AVAILABLE != rc )
   {
      if ( true == bDropFrame )
      {
         BVCE_P_DEBUG_ENTRY_isr( hVceCh->hVce, ePictureBufferDropDequeue, hVceCh->stOpenSettings.uiInstance, *pstPicture );
      }
      else
      {
         BVCE_P_DEBUG_ENTRY_isr( hVceCh->hVce, ePictureBufferDequeue, hVceCh->stOpenSettings.uiInstance, hVceCh->picture.stState.stPictureBufferMailbox );
      }
      return BERR_TRACE( rc );
   }
   else
   {
      return rc;
   }
#else
   BSTD_UNUSED(hVceCh);
   BSTD_UNUSED(pstPicture);
   return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
}

BERR_Code
BVCE_Channel_UserData_GetStatus_isr(
      BVCE_Channel_Handle hVceCh,
      BAVC_VideoUserDataStatus *pstUserDataStatus
      )
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_ENTER( BVCE_Channel_UserData_GetStatus_isr );

   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);
   BDBG_ASSERT( pstUserDataStatus );
   BVCE_P_FUNCTION_TRACE_ENTER_isr(1, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);

   BKNI_Memset( pstUserDataStatus, 0, sizeof( BAVC_VideoUserDataStatus ) );

   if ( BVCE_P_Status_eStarted == hVceCh->eStatus )
   {
      BKNI_Memset( &hVceCh->userdata.stUserDataQueue, 0, sizeof( BVCE_FW_P_UserData_Queue ) );

      /* Get FW Queue Info */
      BVCE_P_ReadRegisters_isr(
               hVceCh->hVce,
               hVceCh->userdata.dccm.uiUserDataQueueInfoAddress,
               (uint32_t*) (&hVceCh->userdata.stUserDataQueue),
               sizeof( BVCE_FW_P_UserData_Queue )
               );

      if ( hVceCh->userdata.stUserDataQueue.uiReadOffset < hVceCh->userdata.stUserDataQueue.uiWriteOffset )
      {
         pstUserDataStatus->uiPendingBuffers = hVceCh->userdata.stUserDataQueue.uiWriteOffset - hVceCh->userdata.stUserDataQueue.uiReadOffset;
      }
      else
      {
         pstUserDataStatus->uiPendingBuffers = BVCE_FW_P_USERDATA_QUEUE_LENGTH - hVceCh->userdata.stUserDataQueue.uiReadOffset;
         pstUserDataStatus->uiPendingBuffers += hVceCh->userdata.stUserDataQueue.uiWriteOffset;
      }

      pstUserDataStatus->uiCompletedBuffers = hVceCh->userdata.uiQueuedBuffers - pstUserDataStatus->uiPendingBuffers;
      hVceCh->userdata.uiQueuedBuffers = pstUserDataStatus->uiPendingBuffers;
   }
   else
   {
      BDBG_MODULE_WRN( BVCE_USERDATA, ("Cannot get user data status when channel is not started") );
      rc = BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   BVCE_P_FUNCTION_TRACE_LEAVE_isr(1, hVceCh->hVce, hVceCh->stOpenSettings.uiInstance);
   BDBG_LEAVE( BVCE_Channel_UserData_GetStatus_isr );
   return BERR_TRACE( rc );
}

/********************/
/* Helper Functions */
/********************/
BERR_Code
BVCE_GetA2PDelayInfo(
         const BVCE_Channel_EncodeSettings *pstChEncodeSettings,
         const BVCE_Channel_StartEncodeSettings *pstChStartEncodeSettings,
         BVCE_A2PDelay *pstA2PDelay
         )
{
   BDBG_ENTER( BVCE_GetA2PDelayInfo );

   BDBG_ASSERT( pstChEncodeSettings );
   BDBG_ASSERT( pstChStartEncodeSettings );
   BDBG_ASSERT( pstA2PDelay );

   if ( NULL != pstA2PDelay )
   {
      BKNI_Memset( pstA2PDelay, 0, sizeof( BVCE_A2PDelay ) );

      if ( ( NULL != pstChEncodeSettings )
           && ( NULL != pstChStartEncodeSettings )
         )
      {
         uint32_t uiProtocol;
         uint32_t uiProfile;
         uint32_t uiLevel;
         uint32_t uiA2PDelayMin;
         uint32_t uiA2PDelayMax;
         uint32_t uiGOPStructure;

         BDBG_ASSERT( BVCE_P_SIGNATURE_STARTENCODESETTINGS == pstChStartEncodeSettings->uiSignature );
         BDBG_ASSERT( BVCE_P_SIGNATURE_CHANNELENCODESETTINGS == pstChEncodeSettings->uiSignature );

         uiProtocol = BVCE_P_ProtocolLUT(pstChStartEncodeSettings->stProtocolInfo.eProtocol);

         switch ( uiProtocol )
         {
            case ENCODING_STD_H264:
               uiProfile = BVCE_S_ProfileH264LUT(pstChStartEncodeSettings->stProtocolInfo.eProfile);
               uiLevel = BVCE_S_LevelH264LUT(pstChStartEncodeSettings->stProtocolInfo.eLevel);
               break;

            case ENCODING_STD_MPEG2:
               uiProfile = BVCE_S_ProfileMPEG2LUT(pstChStartEncodeSettings->stProtocolInfo.eProfile);
               uiLevel = BVCE_S_LevelMPEG2LUT(pstChStartEncodeSettings->stProtocolInfo.eLevel);
               break;

            case ENCODING_STD_MPEG4:
               uiProfile = BVCE_S_ProfileMPEG4LUT(pstChStartEncodeSettings->stProtocolInfo.eProfile);
               uiLevel = BVCE_S_LevelMPEG4LUT(pstChStartEncodeSettings->stProtocolInfo.eLevel);
               break;

            case ENCODING_STD_VP8:
               uiProfile = ENCODING_VP8_PROFILE_STANDARD_LF;
               uiLevel = 0;
               break;

            case ENCODING_STD_HEVC:
               uiProfile = BVCE_S_ProfileHEVCLUT(pstChStartEncodeSettings->stProtocolInfo.eProfile);
               uiLevel = BVCE_S_LevelHEVCLUT(pstChStartEncodeSettings->stProtocolInfo.eLevel);
               break;

            case ENCODING_STD_VP9:
               uiProfile = ENCODING_VP9_PROFILE;
               uiLevel = BVCE_S_LevelVP9LUT(pstChStartEncodeSettings->stProtocolInfo.eLevel);
               break;

            case BVCE_P_VIDEOCOMPRESSIONSTD_UNSUPPORTED:
            default:
               BDBG_ERR(("Unsupported video compression protocol (%d)", pstChStartEncodeSettings->stProtocolInfo.eProtocol));
               return BERR_TRACE( BERR_INVALID_PARAMETER );
         }

         if ( BVCE_P_VIDEOCOMPRESSIONPROFILE_UNSUPPORTED == uiProfile )
         {
            BDBG_ERR(("Unsupported video compression profile (%d)", pstChStartEncodeSettings->stProtocolInfo.eProfile));
            return BERR_TRACE( BERR_INVALID_PARAMETER );
         }

         if ( BVCE_P_VIDEOCOMPRESSIONLEVEL_UNSUPPORTED == uiLevel )
         {
            BDBG_ERR(("Unsupported video compression level (%d)", pstChStartEncodeSettings->stProtocolInfo.eLevel));
            return BERR_TRACE( BERR_INVALID_PARAMETER );
         }

         {
            unsigned uiGOPLength = 0;
            BERR_Code rc;

            rc = BVCE_S_GOPStructureLUT(
               pstChStartEncodeSettings,
               &pstChStartEncodeSettings->stBounds.stGOPStructure,
               &uiGOPStructure,
               &uiGOPLength
               );
            if ( rc != BERR_SUCCESS ) return BERR_TRACE( rc );
            uiGOPStructure &= GOP_STRUCTURE_MASK;
         }

         if (pstChEncodeSettings->stFrameRate.eFrameRate == BAVC_FrameRateCode_eUnknown)
         {
            return BERR_TRACE( BERR_INVALID_PARAMETER );
         }
         else
         {
            unsigned uiWidth = 0, uiHeight = 0;

            BVCE_S_GetMaxDimension( pstChStartEncodeSettings, &uiWidth, &uiHeight );

            uiA2PDelayMin = BVCE_FW_P_CalcVideoA2Pdelay(
               &uiA2PDelayMax,
                     uiProtocol,
                     uiProfile,
                     uiLevel,
                     BVCE_P_PI2FW_FrameRateLUT[pstChEncodeSettings->stFrameRate.eFrameRate],
                     (0 != pstChStartEncodeSettings->stBounds.stBitRate.stLargest.uiMax) ? pstChStartEncodeSettings->stBounds.stBitRate.stLargest.uiMax : pstChEncodeSettings->stBitRate.uiMax,
               BVCE_S_EncodeModeLUT( pstChStartEncodeSettings ),
               pstChStartEncodeSettings->uiRateBufferDelay,
               BVCE_P_PI2FW_FrameRateLUT[pstChStartEncodeSettings->stBounds.stFrameRate.eMin],
               BVCE_P_PI2FW_FrameRateLUT[pstChStartEncodeSettings->stBounds.stInputFrameRate.eMin],
               0,
               pstChEncodeSettings->bITFPEnable,
               BVCE_P_InputTypeLUT[pstChStartEncodeSettings->eInputType],
               uiGOPStructure,
               uiWidth,
               uiHeight,
               pstChStartEncodeSettings->uiNumParallelNRTEncodes
            );
         }

         pstA2PDelay->uiMin = uiA2PDelayMin;
         pstA2PDelay->uiMax = uiA2PDelayMax;
      }
   }

   BDBG_LEAVE( BVCE_GetA2PDelayInfo );
   return BERR_TRACE( BERR_SUCCESS );
}

void
BVCE_GetDefaultMemoryBoundsSettings(
         const BVCE_PlatformSettings *pstPlatformSettings,
         BVCE_MemoryBoundsSettings *pstMemoryBoundsSettings
         )
{
   BDBG_ENTER( BVCE_GetDefaultMemoryBoundsSettings );

   BSTD_UNUSED( pstPlatformSettings );
   BDBG_ASSERT( pstMemoryBoundsSettings );

   BKNI_Memset( pstMemoryBoundsSettings, 0, sizeof( BVCE_MemoryBoundsSettings ) );

   BDBG_LEAVE( BVCE_GetDefaultMemoryBoundsSettings );
}

/* BVCE_GetMemoryConfig - returns the memory requirements for a single device */
void
BVCE_GetMemoryConfig(
         const BVCE_PlatformSettings *pstPlatformSettings,
         const BVCE_MemoryBoundsSettings *pstMemoryBoundsSettings,
         BVCE_MemoryConfig *pstMemoryConfig
         )
{
   unsigned uiNumInstances;

   BDBG_ENTER( BVCE_GetMemoryConfig );

   BSTD_UNUSED( pstPlatformSettings );
   BSTD_UNUSED( pstMemoryBoundsSettings );
   BDBG_ASSERT( pstMemoryConfig );

   BKNI_Memset( pstMemoryConfig, 0, sizeof( BVCE_MemoryConfig ) );

   if ( NULL != pstPlatformSettings )
   {
      BVCE_Platform_P_GetTotalInstances( pstPlatformSettings->hBox, &uiNumInstances );
   }
   else
   {
      BVCE_Platform_P_GetTotalInstances( NULL, &uiNumInstances );
   }

   if ( ( NULL == pstPlatformSettings )
        || ( ( NULL != pstPlatformSettings )
           && ( BVCE_Platform_P_IsInstanceSupported( pstPlatformSettings->hBox, pstPlatformSettings->uiInstance ) ) ) )
   {
#if ( BVCE_P_CORE_MAJOR <= 2 )
      pstMemoryConfig->uiSecureMemSize = MIN_CMD_BUFFER_SIZE_IN_BYTES;
#endif
      pstMemoryConfig->uiGeneralMemSize = BVCE_P_DEFAULT_DEBUG_LOG_SIZE * BVCE_PLATFORM_P_NUM_ARC_CORES;
      pstMemoryConfig->uiGeneralMemSize += BVCE_P_MAX_DEBUG_FIFO_COUNT * sizeof( BVCE_P_DebugFifo_Entry );
      {
         unsigned i;

         for (i = 0; i < BVCE_PLATFORM_P_NUM_ARC_CORES; i++ )
         {
            pstMemoryConfig->uiFirmwareMemSize += BVCE_P_FirmwareSizeLUT[i];
         }
      }

#define BVCE_MEMCONFIG_FIELD(_field) pstMemoryConfig->_field = BVCE_P_ALIGN(pstMemoryConfig->_field, BVCE_P_DEFAULT_ALIGNMENT);
#include "bvce_memconfig.inc"
   }

   if ( NULL != pstPlatformSettings )
   {
      BBOX_Config *pstBoxConfig = (BBOX_Config *) BKNI_Malloc( sizeof( BBOX_Config ) );
      if ( NULL != pstBoxConfig )
      {
         BBOX_GetConfig( pstPlatformSettings->hBox, pstBoxConfig );

         BDBG_MODULE_MSG( BVCE_MEMORY, ("BVCE_GetMemoryConfig(boxMode=%d, instance=%d):", pstBoxConfig->stBox.ulBoxId, pstPlatformSettings->uiInstance ));
         BKNI_Free( pstBoxConfig );
      }
      else
      {
         BDBG_MODULE_MSG( BVCE_MEMORY, ("BVCE_GetMemoryConfig(boxMode=?, instance=%d):", pstPlatformSettings->uiInstance ));
      }
   }
   else
   {
      BDBG_MODULE_MSG( BVCE_MEMORY, ("BVCE_GetMemoryConfig():" ));
   }
#define BVCE_MEMCONFIG_FIELD(_field) BDBG_MODULE_MSG(BVCE_MEMORY, (#_field"=%lu bytes", (unsigned long) pstMemoryConfig->_field));
#include "bvce_memconfig.inc"

   BDBG_LEAVE( BVCE_GetMemoryConfig );
}

void
BVCE_Channel_GetDefaultMemoryBoundsSettings(
         const BBOX_Handle hBox,
         const BVCE_Channel_MemorySettings *pstChMemorySettings,
         BVCE_Channel_MemoryBoundsSettings *pstChMemoryBoundsSettings
         )
{
   BVCE_FW_P_CoreSettings_t stCoreSettings;
   BVCE_FW_P_NonSecureMemSettings_t stNonSecureMemSettings;

   BDBG_ENTER( BVCE_Channel_GetDefaultMemoryBoundsSettings );

   BDBG_ASSERT( pstChMemoryBoundsSettings );

   /* Populate core settings */
   BDBG_CWARNING( sizeof( BVCE_FW_P_CoreSettings_t ) == 1*4 );
   BKNI_Memset( &stCoreSettings, 0, sizeof( BVCE_FW_P_CoreSettings_t ) );
   stCoreSettings.eVersion = CORE_VERSION;

   /* Get default non-secure memory settings */
   BDBG_CWARNING( sizeof( BVCE_FW_P_NonSecureMemSettings_t ) == 10*4 );
   BKNI_Memset( &stNonSecureMemSettings, 0, sizeof( BVCE_FW_P_NonSecureMemSettings_t ) );
   BVCE_FW_P_GetDefaultNonSecureMemSettings( &stCoreSettings, &stNonSecureMemSettings );

   /* Map private FW defaults to public VCE PI default */
   BDBG_CWARNING( sizeof( BVCE_Channel_MemoryBoundsSettings ) == 7*4 );
   BKNI_Memset( pstChMemoryBoundsSettings, 0, sizeof( BVCE_Channel_MemoryBoundsSettings ) );

   pstChMemoryBoundsSettings->eInputType = BVCE_P_InputTypeReverseLUT[stNonSecureMemSettings.InputType];
   pstChMemoryBoundsSettings->stDimensions.stMax.uiHeight = stNonSecureMemSettings.MaxPictureHeightInPels;
   pstChMemoryBoundsSettings->stDimensions.stMax.uiWidth= stNonSecureMemSettings.MaxPictureWidthInPels;

   switch ( stNonSecureMemSettings.MaxGopStructure )
   {
      case ENCODING_GOP_STRUCT_I:
         pstChMemoryBoundsSettings->stGOPStructure.uiNumberOfPFrames = 0;
         pstChMemoryBoundsSettings->stGOPStructure.uiNumberOfBFrames = 0;
         break;

      case ENCODING_GOP_STRUCT_IP:
         pstChMemoryBoundsSettings->stGOPStructure.uiNumberOfPFrames = 1;
         pstChMemoryBoundsSettings->stGOPStructure.uiNumberOfBFrames = 0;
         break;

      case ENCODING_GOP_STRUCT_INFINITE_IP:
         pstChMemoryBoundsSettings->stGOPStructure.uiNumberOfPFrames = 0xFFFFFFFF;
         pstChMemoryBoundsSettings->stGOPStructure.uiNumberOfBFrames = 0;
         break;

      case ENCODING_GOP_STRUCT_IBP:
         pstChMemoryBoundsSettings->stGOPStructure.uiNumberOfPFrames = 1;
         pstChMemoryBoundsSettings->stGOPStructure.uiNumberOfBFrames = 1;
         break;

      case ENCODING_GOP_STRUCT_IBBP:
      case ENCODING_GOP_STRUCT_IBBP_B_REF:
         pstChMemoryBoundsSettings->stGOPStructure.uiNumberOfPFrames = 1;
         pstChMemoryBoundsSettings->stGOPStructure.uiNumberOfBFrames = 2;
         break;

      case ENCODING_GOP_STRUCT_IBBBP:
         pstChMemoryBoundsSettings->stGOPStructure.uiNumberOfPFrames = 1;
         pstChMemoryBoundsSettings->stGOPStructure.uiNumberOfBFrames = 3;
         break;

      case ENCODING_GOP_STRUCT_IBBBBBP:
         pstChMemoryBoundsSettings->stGOPStructure.uiNumberOfPFrames = 1;
         pstChMemoryBoundsSettings->stGOPStructure.uiNumberOfBFrames = 5;
         break;

      case ENCODING_GOP_STRUCT_IBBBBBBBP:
         pstChMemoryBoundsSettings->stGOPStructure.uiNumberOfPFrames = 1;
         pstChMemoryBoundsSettings->stGOPStructure.uiNumberOfBFrames = 7;
         break;

      default:
         break;
   }

   BVCE_Platform_P_OverrideChannelDefaultMemoryBoundsSettings( hBox, pstChMemorySettings, pstChMemoryBoundsSettings );

   BDBG_LEAVE( BVCE_Channel_GetDefaultMemoryBoundsSettings );
}

void
BVCE_Channel_GetMemoryConfig(
         const BBOX_Handle hBox,
         const BVCE_Channel_MemorySettings *pstChMemorySettings,
         const BVCE_Channel_MemoryBoundsSettings *pstChMemoryBoundsSettings,
         BVCE_MemoryConfig *pstMemoryConfig
         )
{
   BVCE_Channel_MemoryBoundsSettings stChMemoryBoundsSettings;
   unsigned uiNumInstances = 0;

   BDBG_ENTER( BVCE_Channel_GetMemoryConfig );

   BDBG_ASSERT( pstMemoryConfig );

   BDBG_CWARNING( sizeof( BVCE_Channel_MemoryBoundsSettings ) == 7*4 );

   if ( NULL == pstChMemoryBoundsSettings )
   {
      BVCE_Channel_GetDefaultMemoryBoundsSettings( hBox, pstChMemorySettings, &stChMemoryBoundsSettings );
      pstChMemoryBoundsSettings = &stChMemoryBoundsSettings;
   }

   BKNI_Memset( pstMemoryConfig, 0, sizeof( BVCE_MemoryConfig ) );

   BVCE_Platform_P_GetTotalInstances( hBox, &uiNumInstances );

   if ( 0 != uiNumInstances )
   {
      BBOX_Config *pstBoxConfig = (BBOX_Config *) BKNI_Malloc( sizeof( BBOX_Config ) );

      if ( NULL != pstBoxConfig )
      {
         BBOX_GetConfig( hBox, pstBoxConfig );
      }

      /* SW7435-1069: Calculate non-secure memory requirmeents by calling BVCE_FW_P_CalcNonSecureMem() */
      {
         BVCE_FW_P_CoreSettings_t stCoreSettings;
         BVCE_FW_P_NonSecureMemSettings_t stNonSecureMemSettings;

         /* Populate core settings */
         BDBG_CWARNING( sizeof( BVCE_FW_P_CoreSettings_t ) == 1*4 );
         BKNI_Memset( &stCoreSettings, 0, sizeof( BVCE_FW_P_CoreSettings_t ) );
         stCoreSettings.eVersion = CORE_VERSION;

         /* Populate non-secure memory settings */

         BDBG_CWARNING( sizeof( BVCE_FW_P_NonSecureMemSettings_t ) == 10*4 );
         BKNI_Memset( &stNonSecureMemSettings, 0, sizeof( BVCE_FW_P_NonSecureMemSettings_t ) );

         BVCE_FW_P_GetDefaultNonSecureMemSettings( &stCoreSettings, &stNonSecureMemSettings );

         stNonSecureMemSettings.InputType = BVCE_P_InputTypeLUT[pstChMemoryBoundsSettings->eInputType];
         stNonSecureMemSettings.MaxPictureHeightInPels = BVCE_P_ALIGN_DIMENSION(pstChMemoryBoundsSettings->stDimensions.stMax.uiHeight);
         stNonSecureMemSettings.MaxPictureWidthInPels = BVCE_P_ALIGN_DIMENSION(pstChMemoryBoundsSettings->stDimensions.stMax.uiWidth);
#if (BVCE_P_VDC_MANAGE_VIP)
         stNonSecureMemSettings.NewBvnMailboxEnable = 1;
#endif

         switch ( pstChMemoryBoundsSettings->stGOPStructure.uiNumberOfBFrames )
         {
            case 0:
               switch ( pstChMemoryBoundsSettings->stGOPStructure.uiNumberOfPFrames )
               {
                  case 0:
                     stNonSecureMemSettings.MaxGopStructure = ENCODING_GOP_STRUCT_I;
                     break;

                  case 0xFFFFFFFF:
                     stNonSecureMemSettings.MaxGopStructure = ENCODING_GOP_STRUCT_INFINITE_IP;
                     break;

                  default:
                     stNonSecureMemSettings.MaxGopStructure = ENCODING_GOP_STRUCT_IP;
                     break;
               }
               break;

            case 1:
               stNonSecureMemSettings.MaxGopStructure = ENCODING_GOP_STRUCT_IBP;
               break;

            case 3:
               stNonSecureMemSettings.MaxGopStructure = ENCODING_GOP_STRUCT_IBBBP;
               break;

            case 5:
               stNonSecureMemSettings.MaxGopStructure = ENCODING_GOP_STRUCT_IBBBBBP;
               break;

            case 7:
               stNonSecureMemSettings.MaxGopStructure = ENCODING_GOP_STRUCT_IBBBBBBBP;
               break;

            case 2:
            default:
#if (BVCE_P_CORE_MAJOR < 3)
               stNonSecureMemSettings.MaxGopStructure = ENCODING_GOP_STRUCT_IBBP;
#else
               stNonSecureMemSettings.MaxGopStructure = ENCODING_GOP_STRUCT_IBBP_B_REF;
#endif
               break;
         }

         if ( NULL != pstChMemorySettings )
         {
            BERR_Code rc;
            BVCE_P_FirmwareMemorySettings stFirmwareMemorySettings;
            unsigned uiPictureMemcIndex = 0;

            if ( NULL != pstBoxConfig )
            {
               uiPictureMemcIndex = pstBoxConfig->stVce.stInstance[pstChMemorySettings->uiInstance].uiMemcIndex;
            }

            rc = BVCE_S_PopulateFirmwareMemorySettings( pstChMemorySettings->pstMemoryInfo, &stFirmwareMemorySettings, uiPictureMemcIndex );

            if ( BERR_SUCCESS == rc )
            {
               stNonSecureMemSettings.DramStripeWidth = stFirmwareMemorySettings.StripeWidth;
               stNonSecureMemSettings.X = stFirmwareMemorySettings.X;
               stNonSecureMemSettings.Y = stFirmwareMemorySettings.Y;
               stNonSecureMemSettings.PageSize = stFirmwareMemorySettings.PageSize;
            }
         }

         BDBG_MODULE_MSG(BVCE_MEMORY, ("StripeWidth=%d, X=%d, Y=%d, PageSize=%d", stNonSecureMemSettings.DramStripeWidth, stNonSecureMemSettings.X, stNonSecureMemSettings.Y, stNonSecureMemSettings.PageSize));
         pstMemoryConfig->uiPictureMemSize = BVCE_FW_P_CalcNonSecureMem( &stCoreSettings, &stNonSecureMemSettings );
      }

      pstMemoryConfig->uiSecureMemSize = MIN_SECURE_BUFFER_SIZE_IN_BYTES; /* A constant depending on the the core revision */
      pstMemoryConfig->uiGeneralMemSize = BVCE_USERDATA_P_QUEUE_SIZE; /* User Data Queue */
      pstMemoryConfig->uiGeneralMemSize += BVCE_P_ALIGN( BVCE_P_MAX_VIDEODESCRIPTORS * sizeof( BAVC_VideoBufferDescriptor ), BVCE_P_DEFAULT_ALIGNMENT ); /* Video Output Descriptors */
      pstMemoryConfig->uiGeneralMemSize += BVCE_P_ALIGN( BVCE_P_MAX_METADATADESCRIPTORS * sizeof( BAVC_VideoMetadataDescriptor ), BVCE_P_DEFAULT_ALIGNMENT ); /* Video Output Metadata Descriptors */
      pstMemoryConfig->uiGeneralMemSize += BVCE_P_ALIGN( BVCE_P_MAX_VIDEODESCRIPTORS * sizeof( BVCE_P_Output_ITB_IndexEntry ), BVCE_P_DEFAULT_ALIGNMENT ); /* Video ITB Index Entries */

      /* Perform size alignment to prevent issues during sub-allocation */
   #define BVCE_MEMCONFIG_FIELD(_field) pstMemoryConfig->_field = BVCE_P_ALIGN(pstMemoryConfig->_field, BVCE_P_DEFAULT_ALIGNMENT);
   #include "bvce_memconfig.inc"

      /* TODO: Adjust ITB/CDB size based on the specified bitrate */
      pstMemoryConfig->uiIndexMemSize = BVCE_P_DEFAULT_ITB_SIZE;
      pstMemoryConfig->uiDataMemSize = BVCE_P_DEFAULT_CDB_SIZE;

      if ( NULL != pstBoxConfig )
      {
         BDBG_MODULE_MSG( BVCE_MEMORY, ("BVCE_Channel_GetMemoryConfig(boxMode=%d,progressive=%d,width=%d,height=%d,bitrateMax=%d,bitrateTarget=%d,max_b=%d):",
            pstBoxConfig->stBox.ulBoxId,
            pstChMemoryBoundsSettings->eInputType,
            pstChMemoryBoundsSettings->stDimensions.stMax.uiWidth,
            pstChMemoryBoundsSettings->stDimensions.stMax.uiHeight,
            pstChMemoryBoundsSettings->stBitRate.stLargest.uiMax,
            pstChMemoryBoundsSettings->stBitRate.stLargest.uiTarget,
            pstChMemoryBoundsSettings->stGOPStructure.uiNumberOfBFrames
            ));
         BKNI_Free( pstBoxConfig );
      }
      else
      {
         BDBG_MODULE_MSG( BVCE_MEMORY, ("BVCE_Channel_GetMemoryConfig(boxMode=?,progressive=%d,width=%d,height=%d,bitrateMax=%d,bitrateTarget=%d,max_b=%d):",
            pstChMemoryBoundsSettings->eInputType,
            pstChMemoryBoundsSettings->stDimensions.stMax.uiWidth,
            pstChMemoryBoundsSettings->stDimensions.stMax.uiHeight,
            pstChMemoryBoundsSettings->stBitRate.stLargest.uiMax,
            pstChMemoryBoundsSettings->stBitRate.stLargest.uiTarget,
            pstChMemoryBoundsSettings->stGOPStructure.uiNumberOfBFrames
            ));
      }
   }
#define BVCE_MEMCONFIG_FIELD(_field) BDBG_MODULE_MSG(BVCE_MEMORY, (#_field"=%lu bytes", (unsigned long) pstMemoryConfig->_field));
#include "bvce_memconfig.inc"

   BDBG_LEAVE( BVCE_Channel_GetMemoryConfig );
}

/* Power Management */
BERR_Code
BVCE_Power_Standby(
         BVCE_Handle hVce
         )
{
   BERR_Code rc = BERR_SUCCESS;
   unsigned uiChannelNum = 0;

   BDBG_OBJECT_ASSERT(hVce, BVCE_P_Context);
   BDBG_ENTER( BVCE_Power_Standby );
   BVCE_P_FUNCTION_TRACE_ENTER(0, hVce, 0);

   /* Verify that we are not already in a standby state */
   if ( 0 == BVCE_Power_P_QueryResource( hVce, BVCE_Power_Type_ePower ) )
   {
      BDBG_ERR(("Already in standby!"));
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   BVCE_Power_P_AcquireResource(
         hVce,
         BVCE_Power_Type_eClock
         );

#if BDBG_DEBUG_BUILD
   if ( true == hVce->bWatchdogOccurred )
   {
      /* Dump PC */
      {
         unsigned i;

         for ( i = 0; i < BVCE_PLATFORM_P_NUM_ARC_CORES; i++ )
         {
            if ( 0 != hVce->stPlatformConfig.stDebug.uiArcHostIF[i] )
            {
               BREG_Write32(
                  hVce->handles.hReg,
                  hVce->stPlatformConfig.stDebug.uiArcHostIF[i],
                  hVce->stPlatformConfig.stDebug.uiArcHostIFMask[i]
               );
            }
         }

         for ( i = 0; i < 10; i++ )
         {
            unsigned j;

            for ( j = 0; j < BVCE_PLATFORM_P_NUM_ARC_CORES; j++ )
            {
               if ( 0 != hVce->stPlatformConfig.stDebug.uiArcPC[j] )
               {
                  BDBG_ERR(("@%08x = %08x (ARC[%d] PC)",
                     hVce->stPlatformConfig.stDebug.uiArcPC[j],
                     BREG_Read32(
                        hVce->handles.hReg,
                        hVce->stPlatformConfig.stDebug.uiArcPC[j]
                        ),
                     j
                     ));
               }
            }

            BKNI_Sleep(10);
         }
      }

      /* Dump VCE HW Registers to Console */
      BVCE_Debug_DumpRegisters(hVce);
#if BVCE_P_ASSERT_ON_WATCHDOG
      BDBG_ASSERT(0);
#endif
   }
#endif

   /* Verify that all channels are stopped.  Keep track of channels that are open */
   for ( uiChannelNum = 0; uiChannelNum < BVCE_PLATFORM_P_NUM_ENCODE_CHANNELS; uiChannelNum++ )
   {
      switch( hVce->channels[uiChannelNum].context.eStatus )
      {
         case BVCE_P_Status_eStarted:
         case BVCE_P_Status_eStopping:
            if ( false == hVce->bWatchdogOccurred )
            {
               BDBG_ERR(("BVCE_Power_Standby called while channel[%d] is still actively encoding", uiChannelNum));
               return BERR_TRACE( BERR_INVALID_PARAMETER );
            }
            else
            {
               /* Watchdog, force the channel state to opened, and fall-through */
               hVce->channels[uiChannelNum].context.eStatus = BVCE_P_Status_eOpened;
#if BVCE_P_TEST_MODE
               BVCE_Debug_P_CloseLog( hVce->channels[uiChannelNum].context.hConfigLog );
               hVce->channels[uiChannelNum].context.hConfigLog = NULL;
#endif
            }

         case BVCE_P_Status_eOpened:
            hVce->channels[uiChannelNum].bResume = true;

            BVCE_Channel_Close( &hVce->channels[uiChannelNum].context );
            break;

         case BVCE_P_Status_eIdle:
            hVce->channels[uiChannelNum].bResume = false;
            break;

         default:
            BDBG_ERR(("Unknown channel state (%d)", hVce->channels[uiChannelNum].context.eStatus));
            BDBG_ASSERT(0);
      }
   }

   /* Shutdown the encoder gracefully */
   rc = BVCE_S_DisableWatchdog(
            hVce
            );
   BERR_TRACE( rc );

   rc = BVCE_S_Reset(
            hVce
            );
   BERR_TRACE( rc );

   rc = BVCE_S_TeardownInterrupts(
               hVce
               );
   BERR_TRACE( rc );

   /* Power down the encoder */
   BVCE_Power_P_ReleaseAllResources(
         hVce
         );

   BVCE_P_FUNCTION_TRACE_LEAVE(0, hVce, 0);
   BDBG_LEAVE( BVCE_Power_Standby );

   return BERR_TRACE( rc );
}

BERR_Code
BVCE_Power_Resume(
         BVCE_Handle hVce
         )
{
   BERR_Code rc = BERR_SUCCESS;
   unsigned uiChannelNum = 0;

   BDBG_OBJECT_ASSERT(hVce, BVCE_P_Context);
   BDBG_ENTER( BVCE_Power_Resume );
   BVCE_P_FUNCTION_TRACE_ENTER(0, hVce, 0);

   /* Verify that we in a standby state */
   if ( 0 != BVCE_Power_P_QueryResource( hVce, BVCE_Power_Type_ePower ) )
   {
      BDBG_ERR(("Not in standby!"));
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   /* Power up the encoder */
   BVCE_Power_P_AcquireResource(
         hVce,
         BVCE_Power_Type_ePower
         );

   BVCE_Power_P_AcquireResource(
         hVce,
         BVCE_Power_Type_eClock
         );

   /* Startup the encoder */
   rc = BVCE_S_Reset(
            hVce
            );
   if ( BERR_SUCCESS != rc )
   {
      return BERR_TRACE( rc );
   }

   rc = BVCE_S_LoadFirmware(
            hVce
            );
   if ( BERR_SUCCESS != rc )
   {
      return BERR_TRACE( rc );
   }

   rc = BVCE_S_SetupInterrupts(
            hVce
            );
   if ( BERR_SUCCESS != rc )
   {
      return BERR_TRACE( rc );
   }

   rc = BVCE_S_Boot(
            hVce
            );
   if ( BERR_SUCCESS != rc )
   {
      return BERR_TRACE( rc );
   }

   /* Send Init Command */
   rc = BVCE_S_SendCommand_Init(
            hVce
            );
   if ( BERR_SUCCESS != rc )
   {
      return BERR_TRACE( rc );
   }

   rc = BVCE_S_SetupDebugLog(
            hVce
            );
   if ( BERR_SUCCESS != rc )
   {
      return BERR_TRACE( rc );
   }

   BVCE_Power_P_ReleaseResource(
      hVce,
      BVCE_Power_Type_eClock
      );

   /* Re-Open any channels that were open when standby was called */
   for ( uiChannelNum = 0; uiChannelNum < BVCE_PLATFORM_P_NUM_ENCODE_CHANNELS; uiChannelNum++ )
   {
      if ( true == hVce->channels[uiChannelNum].bResume )
      {
         BVCE_Channel_Handle hVceCh;
         BVCE_Channel_OpenSettings stChannelOpenSettings = hVce->channels[uiChannelNum].context.stOpenSettings;
         BVCE_Channel_CallbackSettings stChannelCallbackSettings = hVce->channels[uiChannelNum].context.stCallbackSettings;
         BVCE_Channel_EncodeSettings stChannelEncodeSettings = hVce->channels[uiChannelNum].context.stEncodeSettings;

         BVCE_Channel_Open(
            hVce,
            &hVceCh,
            &stChannelOpenSettings
            );

         /* SW7445-3214: Restore callback settings */
         if ( BVCE_P_SIGNATURE_CHANNELCALLBACKSETTINGS == stChannelCallbackSettings.uiSignature )
         {
            BVCE_Channel_SetCallbackSettings(
               hVceCh,
               &stChannelCallbackSettings
               );
         }

         /* SW7445-3214: Restore encode settings */
         if ( BVCE_P_SIGNATURE_ENCODESETTINGS == stChannelEncodeSettings.uiSignature )
         {
            BVCE_Channel_SetEncodeSettings(
               hVceCh,
               &stChannelEncodeSettings
               );
         }

         hVce->channels[uiChannelNum].bResume = false;
      }
   }

   BVCE_P_FUNCTION_TRACE_LEAVE(0, hVce, 0);
   BDBG_LEAVE( BVCE_Power_Resume );

   return BERR_TRACE( rc );
}

BVCE_P_Status
BVCE_Channel_P_GetState(
   BVCE_Channel_Handle hVceCh
   )
{
   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);
   return hVceCh->eStatus;
}

void
BVCE_Channel_P_GetStartEncodeSettings(
   BVCE_Channel_Handle hVceCh,
   BVCE_Channel_StartEncodeSettings *pstStartEncodeSettings
   )
{
   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);
   BDBG_ASSERT( pstStartEncodeSettings );

   *pstStartEncodeSettings = hVceCh->stStartEncodeSettings;
}

void
BVCE_Channel_P_GetEncodeSettings(
   BVCE_Channel_Handle hVceCh,
   BVCE_Channel_EncodeSettings *pstEncodeSettings
   )
{
   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);
   BDBG_ASSERT( pstEncodeSettings );

   *pstEncodeSettings = hVceCh->stEncodeSettings;
}

void
BVCE_Channel_P_HandleEOSEvent(
   BVCE_Channel_Handle hVceCh
   )
{
   BDBG_OBJECT_ASSERT(hVceCh, BVCE_P_Channel_Context);
   /* SW7425-4186: Set EOS event when EOS ITB entry is seen */
   hVceCh->stStatus.uiEventFlags |= BVCE_CHANNEL_STATUS_FLAGS_EVENT_EOS;

   if ( ( BVCE_P_Status_eStarted == hVceCh->eStatus )
        || ( BVCE_P_Status_eStopping == hVceCh->eStatus ) )
   {
      BVCE_Power_P_ReleaseResource(
            hVceCh->hVce,
            BVCE_Power_Type_eClock
            );

      hVceCh->eStatus = BVCE_P_Status_eOpened;
   }
}

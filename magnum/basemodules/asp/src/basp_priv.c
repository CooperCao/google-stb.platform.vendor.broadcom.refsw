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
#include <stdio.h>
#include <inttypes.h>
#include "bstd.h"           /* standard types */
#include "berr.h"           /* error code */
#include "bdbg.h"           /* debug interface */
#include "bkni.h"           /* kernel interface */

#include "bafl.h"

#include "basp.h"

#include "basp_priv.h"
#include "basp_image.h"

#include "basp_hw_init_info.h"
#include "bchp_asp_arcss_ctrl.h"
#include "bchp_asp_arcss_dccm.h"

#include "bchp_asp_mcpb.h"
#include "bchp_asp_mcpb_ch0.h"



#define BASP_DEBUG 1

#ifdef BASP_DEBUG
#include "bchp_asp_rgr.h" /* this is to check whether rdb is loaded properly on emu platform by checking the RGR_REVISION*/
#endif

BDBG_MODULE(BASP_PRIV);

BERR_Code
BASP_P_AllocateDeviceMemory(
         BASP_P_Allocator_Handle hAllocator,
         BASP_P_Buffer_Handle    *phBuffer,
         uint32_t                ui32BufferSizeInBytes,
         uint32_t                ui32Allignment
         )
{
    BERR_Code rc = BERR_SUCCESS;
    BASP_P_Buffer_AllocSettings sAllocSettings;

    BASP_P_Buffer_GetDefaultAllocSettings(&sAllocSettings);
    sAllocSettings.uiSize = ui32BufferSizeInBytes;
    sAllocSettings.uiAlignment = ui32Allignment;
    sAllocSettings.bLockOffset = true;

    rc = BASP_P_Buffer_Alloc(hAllocator, &sAllocSettings, phBuffer);

    return BERR_TRACE(rc);
}

void
BASP_P_FreeDeviceMemory(BASP_P_Buffer_Handle hBuffer)
{
    BASP_P_Buffer_Free(hBuffer);
}

void
BASP_P_DestroyAllocators(
   BASP_Handle hAsp
   )
{
   if (hAsp->fwMemory.hAllocator)
   {
      BASP_P_Allocator_Destroy(hAsp->fwMemory.hAllocator);
      hAsp->fwMemory.hAllocator = NULL;
   }

   if (hAsp->aspSystemMemory.hAllocator)
   {
      BASP_P_Allocator_Destroy(hAsp->aspSystemMemory.hAllocator);
      hAsp->aspSystemMemory.hAllocator = NULL;
   }

   if (hAsp->aspCommMemory.hAllocator)
   {
      BASP_P_Allocator_Destroy(hAsp->aspCommMemory.hAllocator);
      hAsp->aspCommMemory.hAllocator = NULL;
   }
}

BERR_Code
BASP_P_SetupAllocators(
   BASP_Handle hAsp
   )
{
   BERR_Code rc;
   BMMA_Heap_Handle hMem = NULL;
   BASP_P_Buffer_AllocSettings sAllocSettings;

   hMem = hAsp->handles.hMem;

   /* Create allocator for firmware buffer.*/
   BASP_P_Buffer_GetDefaultAllocSettings(&sAllocSettings);
   sAllocSettings.uiAlignment = BASP_P_ALLOCATOR_DEFAULT_ALIGNMENT;
   sAllocSettings.uiSize = BASP_P_MAX_FIRMWARE_SIZE + 8000;

   rc = BASP_P_Allocator_Create( hMem, &sAllocSettings, &hAsp->fwMemory.hAllocator);
   if ( BERR_SUCCESS != rc)
   {
      BASP_P_DestroyAllocators( hAsp );
      return BERR_TRACE( rc );
   }

   BDBG_MODULE_MSG( BASP_MEMORY, ("Created Firmware Memory Allocator: %08x bytes @ " BASP_X64_FMT,
            (unsigned) BASP_P_Allocator_GetSize( hAsp->fwMemory.hAllocator ),
            BASP_X64_ARG((uint64_t) BASP_P_Allocator_GetDeviceOffset( hAsp->fwMemory.hAllocator ))
            ));

   /* Create allocator for asp firmware data transfer and message communication buffer.*/
   sAllocSettings.uiAlignment = BASP_P_ALLOCATOR_DEFAULT_ALIGNMENT; /* TODO: Later this can be reduced to smaller size.*/
   sAllocSettings.uiSize = BASP_P_MAX_INTERFACE_COMM_BUFFER_SIZE;

    rc = BASP_P_Allocator_Create(hMem, &sAllocSettings, &hAsp->aspCommMemory.hAllocator);
    if ( BERR_SUCCESS != rc)
    {
       BASP_P_DestroyAllocators( hAsp );
       return BERR_TRACE( rc );
    }

   BDBG_MODULE_MSG( BASP_MEMORY, ("Host Asp Communication Memory Allocator: %08x bytes @ " BASP_X64_FMT,
            (unsigned) BASP_P_Allocator_GetSize( hAsp->aspCommMemory.hAllocator ),
            BASP_X64_ARG((uint64_t) BASP_P_Allocator_GetDeviceOffset( hAsp->aspCommMemory.hAllocator ))
            ));

   /* Create allocator for asp only usage buffers.*/
   sAllocSettings.uiAlignment = BASP_P_ALLOCATOR_DEFAULT_ALIGNMENT; /* TODO: Later this can be reduced to smaller size.*/
   sAllocSettings.uiSize = BASP_P_MAX_ASP_SYSTEM_MEMORY_SIZE;

    rc = BASP_P_Allocator_Create(hMem, &sAllocSettings, &hAsp->aspSystemMemory.hAllocator);
   if ( BERR_SUCCESS != rc)
   {
      BASP_P_DestroyAllocators( hAsp );
      return BERR_TRACE( rc );
   }

   BDBG_MODULE_MSG( BASP_MEMORY, ("Host Asp System Memory Allocator: %08x bytes @ " BASP_X64_FMT,
            (unsigned) BASP_P_Allocator_GetSize( hAsp->aspSystemMemory.hAllocator ),
            BASP_X64_ARG((uint64_t) BASP_P_Allocator_GetDeviceOffset( hAsp->aspSystemMemory.hAllocator ))
            ));

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BASP_P_Reset(
      BASP_Handle hAsp
      )
{
   BREG_Handle hReg = hAsp->handles.hReg;

   /* rc = BASP_Platform_P_Preset(hAsp); Any registers that need to be
      cleared or mask before starting asp can be done inside this function.*/

#ifdef BASP_DEBUG
   {
      uint32_t ui32RegvalueRead = BREG_Read32( hReg, BCHP_ASP_RGR_REVISION );

      BDBG_MSG(("%s: Asp ------------------>  BCHP_ASP_RGR_REVISION = 0x%x ", BSTD_FUNCTION, ui32RegvalueRead));
   }
#endif

   /* write 1 to re-initialize the processor and then set it to 0 to bring it to idle state.*/
    BREG_Write32(hReg,       BCHP_ASP_ARCSS_CTRL_INIT_PROC_CTRL2,
                 BCHP_FIELD_DATA(ASP_ARCSS_CTRL_INIT_PROC_CTRL2, SW_INIT, 1));

   BREG_Write32(hReg,       BCHP_ASP_ARCSS_CTRL_INIT_PROC_CTRL2,
                 BCHP_FIELD_DATA(ASP_ARCSS_CTRL_INIT_PROC_CTRL2, SW_INIT, 0) );

   return BERR_TRACE( BERR_SUCCESS );
}

/* Temp , this will be later replaced with actual asp Device id that is used by fw team.*/
const uint32_t BASP_P_CoreDeviceID[1] =
{
   0,
};


BERR_Code
BASP_P_LoadFirmware(
   BASP_Handle hAsp
   )
{
    BERR_Code rc = BERR_SUCCESS;
    BREG_Handle hReg = hAsp->handles.hReg;
    uint32_t ui32RegValue = 0;
    void *pFirmwareBufferCached = NULL;     /* Virtual address of buffer holding the firmware while it's being downloaded. */

    BDBG_ENTER(BASP_P_LoadFirmware);

    /* First allocate memory to hold the firmware while we are DMA'ing it into the ARC's memory. */
    rc = BASP_P_AllocateDeviceMemory(hAsp->fwMemory.hAllocator,              /* Use this allocator */
                                     &hAsp->fwMemory.hBuffer,                /* Put allocated buffer handle here */
                                     BASP_P_MAX_FIRMWARE_SIZE,               /* Allocate this many bytes */
                                     BASP_P_DEFAULT_FW_MEMORY_ALIGNMENT );   /* Aligned like this */
    if ( BERR_SUCCESS != rc ) { BERR_TRACE( rc ); goto cleanup; }

    BDBG_MSG(("%s: Firmware memory allocated", BSTD_FUNCTION));

    /* Now, we need to lock the virtual address in memory so that it doesn't get relocated
     * while we are accessing it.  */
    pFirmwareBufferCached = BASP_P_Buffer_LockAddress(hAsp->fwMemory.hBuffer);
    if (NULL == pFirmwareBufferCached) { BERR_TRACE( rc ); goto cleanup; }

    /* Zero out FW image buffer.  This is mandatory because BAFL_Load() fails to process
     * any ELF sections of type SHT_NOBITS, which are sections that must be zero filled
     * (such as ".bss", ".stack", and ".heap").  So if we start with a zeroed buffer,
     * BAFL_Load() will populate it with the required non-zero data, leaving zeroes
     * in the sections that it skipped.  */
    BKNI_Memset(pFirmwareBufferCached, 0, BASP_P_Buffer_GetSize(hAsp->fwMemory.hBuffer));

    /* Load the firmware into the memory buffer. */
    rc = BAFL_Load(BASP_P_CoreDeviceID[0],
                   hAsp->sOpenSettings.pImgInterface,
                   (void**) hAsp->sOpenSettings.pImgContext,
                   0,
                   pFirmwareBufferCached, /* Virtual Address */
                   BASP_P_Buffer_GetSize(hAsp->fwMemory.hBuffer),
                   false,
                   &hAsp->sFirmwareLoadInfo);
    if ( BERR_SUCCESS != rc) { BERR_TRACE( rc ); goto cleanup; }

    /* Flush the buffer's cache to make sure the DMA hardware sees what we put there. */
    BASP_P_Buffer_FlushCache_isr(hAsp->fwMemory.hBuffer,
                                 pFirmwareBufferCached,
                                 BASP_P_Buffer_GetSize(hAsp->fwMemory.hBuffer));

    BDBG_MSG(("%s: Code size is %u and Code startAddress=%p",BSTD_FUNCTION,
                    hAsp->sFirmwareLoadInfo.stCode.uiSize,
                    hAsp->sFirmwareLoadInfo.stCode.pStartAddress ));
    BDBG_MSG(("%s: pFirmwareBufferCached=%p ", BSTD_FUNCTION, pFirmwareBufferCached));

    /* Download the firmware's code/instruction section.
     * Set the DMA source: the physical address of the code in the buffer. */
    {
        uint32_t ui32OffsetToCodeInBuffer = (uint8_t *) hAsp->sFirmwareLoadInfo.stCode.pStartAddress - (uint8_t *) pFirmwareBufferCached;
        uint64_t ui64CodeAddress = (uint64_t)BASP_P_Buffer_GetDeviceOffset_isrsafe(hAsp->fwMemory.hBuffer) + ui32OffsetToCodeInBuffer;

        if (ui64CodeAddress % 4 != 0)
        {
            BDBG_ERR(("%s:%u ASP Firmware Code DRAM address 0x" BASP_X64_FMT " is not 4-byte aligned!", BSTD_FUNCTION, __LINE__, BASP_X64_ARG(ui64CodeAddress)));
            BDBG_ASSERT(false);
            rc = BERR_UNKNOWN;
            goto cleanup;
        }

        if (hAsp->sFirmwareLoadInfo.stCode.uiSize % 4 != 0)
        {
            BDBG_ERR(("%s:%u ASP Firmware Code size %#x is not multiple of 4!", BSTD_FUNCTION, __LINE__, hAsp->sFirmwareLoadInfo.stCode.uiSize));
            BDBG_ASSERT(false);
            rc = BERR_UNKNOWN;
            goto cleanup;
        }

        BDBG_MSG(("%s : %d ui64CodeAddress (physical)=" BASP_X64_FMT, BSTD_FUNCTION, __LINE__, BASP_X64_ARG(ui64CodeAddress) ));

        BREG_Write64(hReg, BCHP_ASP_ARCSS_CTRL_XL_IDMA_DRAM_ADDRi_ARRAY_BASE +
                          (BCHP_ASP_ARCSS_CTRL_XL_IDMA_DRAM_ADDRi_ARRAY_ELEMENT_SIZE/8) * BASP_IDMA_CHANNEL_FIRMWARE_DOWNLOAD,
                     BCHP_FIELD_DATA(ASP_ARCSS_CTRL_XL_IDMA_DRAM_ADDRi, ADDRESS, ui64CodeAddress));
    }

    /* Set the DMA destination to be address zero of the ASP's instruction space. */
    BREG_Write32(hReg, BCHP_ASP_ARCSS_CTRL_IDMA_LOCAL_ADDRi_ARRAY_BASE +
                      (BCHP_ASP_ARCSS_CTRL_IDMA_LOCAL_ADDRi_ARRAY_ELEMENT_SIZE/8 * BASP_IDMA_CHANNEL_FIRMWARE_DOWNLOAD),
                 BCHP_FIELD_DATA(ASP_ARCSS_CTRL_IDMA_LOCAL_ADDRi, ADDRESS, 0));

#if BASP_DEBUG_LEVEL2
    {
        uint64_t ui64RegValueRead = BREG_Read64(hReg, BCHP_ASP_ARCSS_CTRL_XL_IDMA_DRAM_ADDRi_ARRAY_BASE );

        BDBG_MSG(("%s: Address value written %"PRIu64"and ui64RegValueRead= %"PRIu64" ", BSTD_FUNCTION, ui64CodeAddress, ui64RegValueRead));
    }
#endif

    /* Now set the code download size in transfer_info. */
    BREG_Write32(hReg, BCHP_ASP_ARCSS_CTRL_IDMA_TRANSFER_INFOi_ARRAY_BASE +
                      (BCHP_ASP_ARCSS_CTRL_IDMA_TRANSFER_INFOi_ARRAY_ELEMENT_SIZE/8 * BASP_IDMA_CHANNEL_FIRMWARE_DOWNLOAD),
#if (BCHP_CHIP==7278 && BCHP_VER == BCHP_VER_A0)    /* 7278 A0 hardware requires byte swap */
                 BCHP_FIELD_DATA(ASP_ARCSS_CTRL_IDMA_TRANSFER_INFOi, SWAP, 1) |
#endif
                 BCHP_FIELD_DATA(ASP_ARCSS_CTRL_IDMA_TRANSFER_INFOi, SIZE, (hAsp->sFirmwareLoadInfo.stCode.uiSize)));

    /* Now start instruction DMA transfer.  We'll wait for it to complete later. */
    BREG_Write32(hReg, BCHP_ASP_ARCSS_CTRL_IDMA_REQ,
                 BCHP_FIELD_DATA(ASP_ARCSS_CTRL_IDMA_REQ, REQ, 1<<BASP_IDMA_CHANNEL_FIRMWARE_DOWNLOAD));

    /* Download the firmware's data section.
     * Instead of using registers, the DDMA expects the DMA transfer information to be
     * placed into the DCCM memory, then we tell the DDMA where that information is.
     * We'll put the transfer info into the first three bytes at DCCM address zero.
     * I hope this is okay, since the DMA transfer destination will be that same address,
     * so the transfer will be overwriting the transfer info. */
    BREG_Write32(hReg, BCHP_ASP_ARCSS_CTRL_DDMA1_CONFIG_LOCAL_ADDR,
                 BCHP_FIELD_DATA(ASP_ARCSS_CTRL_DDMA1_CONFIG_LOCAL_ADDR, ADDRESS, 0));  /* Transfer info is at DCCM address zero. */

    BDBG_MSG(("%s: Data size is %u and Data startAddress=%p",BSTD_FUNCTION,
                    hAsp->sFirmwareLoadInfo.stData.uiSize,
                    hAsp->sFirmwareLoadInfo.stData.pStartAddress ));

    BDBG_ASSERT(BASP_DDMA_CHANNEL_FIRMWARE_DOWNLOAD==0);

    /* Transfer info word0:  Transfer direction, tranfer size, and swap flag.  */
    BREG_Write32(hReg, BCHP_ASP_ARCSS_DCCM_DATAi_ARRAY_BASE,
                 BCHP_FIELD_DATA(ASP_REGSET_ARCSS_CTRL_DDMA_CONFIG_INFO_WORD0, TRANSFER_DIRECTION, 0) |
#if (BCHP_CHIP==7278 && BCHP_VER == BCHP_VER_A0)    /* 7278 A0 hardware requires byte swap */
                 BCHP_FIELD_DATA(ASP_REGSET_ARCSS_CTRL_DDMA_CONFIG_INFO_WORD0, SWAP, 1) |
#endif
                 BCHP_FIELD_DATA(ASP_REGSET_ARCSS_CTRL_DDMA_CONFIG_INFO_WORD0, SIZE, hAsp->sFirmwareLoadInfo.stData.uiSize));

    /* Transfer info word1:  Physical DRAM address (low 32 bits).  */
    /* Transfer info word2:  Physical DRAM address (high 8 bits), and destination DCCM address.  */
    {
        uint32_t ui32OffsetToDataInBuffer = (uint8_t *) hAsp->sFirmwareLoadInfo.stData.pStartAddress - (uint8_t *) pFirmwareBufferCached;
        uint64_t ui64DataAddress = BASP_P_Buffer_GetDeviceOffset_isrsafe(hAsp->fwMemory.hBuffer) + ui32OffsetToDataInBuffer;

        BDBG_MSG(("%s : %d ui64DataAddress (physical)=" BASP_X64_FMT, BSTD_FUNCTION, __LINE__, BASP_X64_ARG(ui64DataAddress) ));

        BREG_Write32(hReg, (BCHP_ASP_ARCSS_DCCM_DATAi_ARRAY_BASE + 1 * (BCHP_ASP_ARCSS_DCCM_DATAi_ARRAY_ELEMENT_SIZE / 8)),
                     BCHP_FIELD_DATA(ASP_REGSET_ARCSS_CTRL_DDMA_CONFIG_INFO_WORD1, DRAM_ADDRESS, ui64DataAddress & 0xFFFFFFFF));

        BREG_Write32(hReg, (BCHP_ASP_ARCSS_DCCM_DATAi_ARRAY_BASE + 2 * (BCHP_ASP_ARCSS_DCCM_DATAi_ARRAY_ELEMENT_SIZE / 8)),
                     BCHP_FIELD_DATA(ASP_REGSET_ARCSS_CTRL_DDMA_CONFIG_INFO_WORD2, DRAM_ADDRESS_HI, ui64DataAddress >> 32) |
                     BCHP_FIELD_DATA(ASP_REGSET_ARCSS_CTRL_DDMA_CONFIG_INFO_WORD2, LOCAL_ADDRESS, 0));
    }

    /* Now start data DMA transfer */
    BREG_Write32(hReg, BCHP_ASP_ARCSS_CTRL_DDMA1_REQ_31_0,
                 BCHP_FIELD_DATA(ASP_ARCSS_CTRL_DDMA1_REQ_31_0, REQ, 1<<BASP_DDMA_CHANNEL_FIRMWARE_DOWNLOAD));

    /* Now wait for the code DMA to complete.
     * If our channel is in the PENDING state, wait until it isn't. */
    /* TODO: Will also check where to check this In case of secure fw download.*/
    BDBG_MSG(("%s: Waiting for IDMA not pending", BSTD_FUNCTION));
    while (true)
    {
        ui32RegValue = BREG_Read32(hReg, BCHP_ASP_ARCSS_CTRL_IDMA_REQ_PENDING);
        if ((BCHP_GET_FIELD_DATA(ui32RegValue, ASP_ARCSS_CTRL_IDMA_REQ_PENDING, REQ_PENDING) & (1<<BASP_IDMA_CHANNEL_FIRMWARE_DOWNLOAD)) == 0) break;
        BKNI_Sleep(0);
    }

    /* If our channel is ACTIVE, wait until it isn't. */
    BDBG_MSG(("%s: Waiting for IDMA not active", BSTD_FUNCTION));
    while (true)
    {
        ui32RegValue = BREG_Read32(hReg, BCHP_ASP_ARCSS_CTRL_IDMA_ACTIVE_CHANNEL);
        if (BCHP_GET_FIELD_DATA(ui32RegValue, ASP_ARCSS_CTRL_IDMA_ACTIVE_CHANNEL, CHANNEL) != BASP_IDMA_CHANNEL_FIRMWARE_DOWNLOAD) break;
        BKNI_Sleep(0);
    }

    /* Code is done downloading, now wait for the data DMA to complete.
     * If our channel is in the PENDING state, wait until it isn't. */
    BDBG_MSG(("%s: Waiting for DDMA not pending", BSTD_FUNCTION));
    while (true)
    {
        ui32RegValue = BREG_Read32(hReg, BCHP_ASP_ARCSS_CTRL_DDMA1_REQ_PENDING_31_0);
        if ((BCHP_GET_FIELD_DATA(ui32RegValue, ASP_ARCSS_CTRL_DDMA1_REQ_PENDING_31_0, REQ_PENDING) & (1<<BASP_DDMA_CHANNEL_FIRMWARE_DOWNLOAD)) == 0) break;
        BKNI_Sleep(0);
    }

    /* If our channel is ACTIVE, wait until it isn't. */
    BDBG_MSG(("%s: Waiting for DDMA not active", BSTD_FUNCTION));
    while (true)
    {
        ui32RegValue = BREG_Read32(hReg, BCHP_ASP_ARCSS_CTRL_DDMA1_ACTIVE_CHANNEL);
        if (BCHP_GET_FIELD_DATA(ui32RegValue, ASP_ARCSS_CTRL_DDMA1_ACTIVE_CHANNEL, CHANNEL) != BASP_DDMA_CHANNEL_FIRMWARE_DOWNLOAD) break;
        BKNI_Sleep(0);
    }

    BDBG_MSG(("%s: Firmware download is complete", BSTD_FUNCTION));

    /* Now start the processor.*/
    BDBG_MSG(("%s: Starting the ASP Processor", BSTD_FUNCTION));

    BREG_Write32(hReg, BCHP_ASP_ARCSS_CTRL_INIT_PROC_START,
                 BCHP_FIELD_DATA(ASP_ARCSS_CTRL_INIT_PROC_START, ENABLE, 1));

    BDBG_MSG(("%s: Processor started", BSTD_FUNCTION));

#ifdef BASP_DEBUG_LEVEL2
    /* Read back 64 bits of fw code back to check if the download is proper.*/
    {
        uint32_t ui32ReadBack = 0;
        uint32_t i=0;

        for (i=0; i<50; i++)
        {

            ui32ReadBack = BREG_Read32(
                    hAsp->handles.hReg,
                    (BCHP_ASP_ARCSS_ICCM_DATAi_ARRAY_BASE + i*BCHP_ASP_ARCSS_ICCM_DATAi_ARRAY_ELEMENT_SIZE/8) );
            BDBG_LOG(("%s: The %uth 32bits of downloaded Code is 0x%x ", BSTD_FUNCTION, i, ui32ReadBack));
        }
    }

    BDBG_LOG(("%s:************************** Now start data verification **************************** ",BSTD_FUNCTION));
    /* Read back 64 bits of data memory back to check if the data download is proper.*/
    {
        uint32_t ui32ReadBack = 0;
        uint32_t i=0;

        for (i=0; i<50; i++)
        {

            ui32ReadBack = BREG_Read32(
                    hAsp->handles.hReg,
                    (BCHP_ASP_ARCSS_DCCM_DATAi_ARRAY_BASE + i*BCHP_ASP_ARCSS_DCCM_DATAi_ARRAY_ELEMENT_SIZE/8) );
            BDBG_LOG(("%s: The %uth 32bits of downloaded Data is 0x%x ", BSTD_FUNCTION, i, ui32ReadBack));

        }
    }
#endif /* BASP_DEBUG_LEVEL2 */


    /* All done.  Unlock and free the firmware memory. */
cleanup:
    if (NULL != pFirmwareBufferCached)
    {
        BASP_P_Buffer_UnlockAddress(hAsp->fwMemory.hBuffer);
        pFirmwareBufferCached = NULL;
    }

    /* If firmware memory was allocated, free it. */
    if (hAsp->fwMemory.hBuffer)
    {
        BASP_P_FreeDeviceMemory(hAsp->fwMemory.hBuffer);
        hAsp->fwMemory.hBuffer = NULL;
        BDBG_MSG(("%s: Firmware memory freed", BSTD_FUNCTION));
    }

    BDBG_LEAVE(BASP_P_LoadFirmware);
    return BERR_TRACE(rc);
}


BERR_Code
BASP_P_FreeHwInitInfo(
   BASP_Handle hAsp
   )
{
    BDBG_ENTER(BASP_P_FreeHwInitInfo);

    if (hAsp->aspCommMemory.hHwInitInfoBuffer)
    {
        BASP_P_FreeDeviceMemory(hAsp->aspCommMemory.hHwInitInfoBuffer);
        hAsp->aspCommMemory.hHwInitInfoBuffer = NULL;
        BDBG_MSG(("%s: ASP communication memory freed", BSTD_FUNCTION));
    }

    BDBG_LEAVE(BASP_P_FreeHwInitInfo);
    return BERR_TRACE(BERR_SUCCESS);
}


BERR_Code
BASP_P_LoadHwInitInfo(
   BASP_Handle hAsp
   )
{
    BERR_Code rc;
    uint32_t *pHwInitInfoLocked = NULL;     /* Virtual address of buffer holding the hardware init info. */

    BDBG_ENTER(BASP_P_LoadHwInitInfo);

    /* Now allocate fw memory */
    rc = BASP_P_AllocateDeviceMemory(hAsp->aspCommMemory.hAllocator,          /* Use this allocator */
                                     &hAsp->aspCommMemory.hHwInitInfoBuffer,  /* Put allocated buffer handle here */
                                     BASP_HW_INIT_INFO_size,                  /* Allocate this many bytes */
                                     BASP_P_DEFAULT_FW_MEMORY_ALIGNMENT );    /* Aligned like this */
    if ( BERR_SUCCESS != rc ) { BERR_TRACE( rc ); goto cleanup; }

    BDBG_MSG(("%s: Hardware Init Info memory allocated", BSTD_FUNCTION));

    /* Now, we need to lock the virtual address in memory so that it doesn't get relocated
     * while we are accessing it.  */
    pHwInitInfoLocked = BASP_P_Buffer_LockAddress(hAsp->aspCommMemory.hHwInitInfoBuffer);
    if (NULL == pHwInitInfoLocked) { rc = BERR_TRACE( BERR_UNKNOWN ); goto cleanup; }

    {
        unsigned i;
        for (i = 0; i < sizeof BASP_HW_INIT_INFO / sizeof BASP_HW_INIT_INFO[0]; i++)
        {
            pHwInitInfoLocked[i * 2] = BASP_HW_INIT_INFO[i].address;
            pHwInitInfoLocked[i * 2 + 1] = BASP_HW_INIT_INFO[i].data;
        }
    }

    /* Flush the cache so the ASP can see what we put there. */
    BASP_P_Buffer_FlushCache_isr(hAsp->aspCommMemory.hHwInitInfoBuffer, pHwInitInfoLocked, BASP_HW_INIT_INFO_size);

    /* All done.  Unlock the virtual address.  Free the memory on error. */
cleanup:
    if (NULL != pHwInitInfoLocked)
    {
        BASP_P_Buffer_UnlockAddress(hAsp->aspCommMemory.hHwInitInfoBuffer);
        pHwInitInfoLocked = NULL;
    }

    if (BERR_SUCCESS != rc)
    {
        BASP_P_FreeHwInitInfo(hAsp);
    }

    BDBG_LEAVE(BASP_P_LoadHwInitInfo);
    return BERR_TRACE(BERR_SUCCESS);
}


BERR_Code
BASP_P_CheckHwInitInfo(
        BASP_Handle hAsp
)
{
    uint32_t *pHwInitInfoLocked = NULL; /* Virtual address of buffer holding the hardware init info. */

    BDBG_ENTER(BASP_P_CheckHwInitInfo);

    /* Now, we need to lock the virtual address in memory so that it doesn't get relocated
     * while we are accessing it.  */
    if (NULL == hAsp->aspCommMemory.hHwInitInfoBuffer)
    {
        BDBG_ERR(("%s: Can't find handle for HwInitInfoBuffer!", BSTD_FUNCTION));
        return (BERR_TRACE(BERR_UNKNOWN));
    }

    pHwInitInfoLocked = BASP_P_Buffer_LockAddress(hAsp->aspCommMemory.hHwInitInfoBuffer);
    if (NULL == pHwInitInfoLocked)
    {
        BERR_TRACE( BERR_UNKNOWN ); goto cleanup;
    }

    {
        unsigned i;
        unsigned errCount = 0;
        uint32_t regAddr;
        uint32_t regExpected;
        uint32_t regActual;

        for (i=0; i < sizeof BASP_HW_INIT_INFO / sizeof BASP_HW_INIT_INFO[0]; i++)
        {
            regAddr = BASP_HW_INIT_INFO[i].address & ~0xf0000000;
            regAddr -= BCHP_PHYSICAL_OFFSET;
            regExpected = BASP_HW_INIT_INFO[i].data;

            regActual = BREG_Read32(hAsp->handles.hReg, regAddr);
            if (regActual != regExpected)
            {
                BDBG_WRN(("%s: HwInitInfo mismatch! Reg: 0x%08x  Expected 0x%08x,  Found 0x%08x!     %s",
                         BSTD_FUNCTION, regAddr, regExpected, regActual, regExpected==regActual?"":"Wrong!"));
                errCount++;
            }
        }
        if (errCount)
        {
            BDBG_WRN(("%s: HwInitInfo: Found %d mismatches", BSTD_FUNCTION, errCount));
        }
        else
        {
            BDBG_MSG(("%s: HwInitInfo: Verified correctly", BSTD_FUNCTION));
        }
    }

    /* All done.  Unlock memory. */
    cleanup:
    if (NULL != pHwInitInfoLocked)
    {
        BASP_P_Buffer_UnlockAddress(hAsp->aspCommMemory.hHwInitInfoBuffer);
        pHwInitInfoLocked = NULL;
    }

    BDBG_LEAVE(BASP_P_CheckHwInitInfo);
    return BERR_TRACE(BERR_SUCCESS);
}

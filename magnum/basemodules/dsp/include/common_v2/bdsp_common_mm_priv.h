/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#include "bdsp_common_priv_include.h"

#ifndef BDSP_COMMON_MM_PRIV_H_
#define BDSP_COMMON_MM_PRIV_H_

#define BDSP_MIN_DEBUG_BUFFER_SIZE 0

#define BDSP_MAX_FW_TASK_PER_DSP            12
#define BDSP_MAX_SOFT_FMM_PER_DSP           1
#define BDSP_MAX_MSGS_PER_QUEUE             10
#define BDSP_MAX_ASYNC_MSGS_PER_QUEUE       40

#define BDSP_ALIGN_SIZE(x,y)      (((x+(y-1))/y)*y)

#define BDSP_INVALID_DRAM_ADDRESS       0xFFFFFFFF
#define BDSP_MAX_BRANCH                      2
#define BDSP_MAX_STAGE_PER_BRANCH           10
#define BDSP_MAX_FW_TASK_PER_AUDIO_CTXT     12
#define BDSP_MAX_POOL_OF_DESCRIPTORS       100
#define BDSP_MAX_DESCRIPTORS_PER_POOL      (BDSP_AF_P_MAX_CHANNELS+BDSP_AF_P_MAX_TOC+BDSP_AF_P_MAX_METADATA+BDSP_AF_P_MAX_OBJECTDATA)
#define BDSP_MAX_HOST_DSP_L2C_SIZE         512

#define BDSP_MAX_INTERTASKBUFFER_INPUT_TO_MIXER         4
#define BDSP_MAX_INTERTASKBUFFER_INPUT_TO_ECHOCANCELLER 1

#define BDSP_ReadReg32(hReg, addr)             BREG_Read32(hReg, addr)
#define BDSP_WriteReg32(hReg, addr, data)      BREG_Write32(hReg, addr, data)
#define BDSP_ReadReg32_isr(hReg, addr)         BREG_Read32_isr(hReg, addr)
#define BDSP_WriteReg32_isr(hReg, addr, data)  BREG_Write32_isr(hReg, addr, data)

#define BDSP_ReadReg64(hReg, addr)             BREG_Read64(hReg, addr)
#define BDSP_WriteReg64(hReg, addr, data)      BREG_Write64(hReg, addr, data)
#define BDSP_ReadReg64_isr(hReg, addr)         BREG_Read64_isr(hReg, addr)
#define BDSP_WriteReg64_isr(hReg, addr, data)  BREG_Write64_isr(hReg, addr, data)

#if defined BCHP_AUD_FMM_BF_CTRL_REG_START
#define BDSP_SIZE_OF_FMMREG                    (BCHP_AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_ENDADDR - BCHP_AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_BASEADDR)
#else
#define BDSP_SIZE_OF_FMMREG 4
#endif
#define BDSP_FMM_WRAP_BIT                      ((BDSP_SIZE_OF_FMMREG == 8)?39:31)
#define BDSP_FMM_WRAP_MASK                     ((dramaddr_t)0x1<<BDSP_FMM_WRAP_BIT)
#define BDSP_FMM_ADDR_MASK                     (BDSP_FMM_WRAP_MASK-1)
#define BDSP_ReadFMMReg(hReg, addr)            ((BDSP_SIZE_OF_FMMREG == 8)? BDSP_ReadReg64(hReg, addr): BDSP_ReadReg32(hReg, addr))
#define BDSP_WriteFMMReg(hReg, addr, data)     ((BDSP_SIZE_OF_FMMREG == 8)? BDSP_WriteReg64(hReg, addr, data): BDSP_WriteReg32(hReg, addr, data))

#define BDSP_MMA_P_FlushCache(Memory, size) BMMA_FlushCache(Memory.hBlock, Memory.pAddr, size)
#define BDSP_MMA_P_FlushCache_isr(Memory, size) BMMA_FlushCache_isr(Memory.hBlock, Memory.pAddr, size)

#define BDSP_MMA_P_OffsetToVirtual(StartVirtual,StartOffset,Offset) (void *)((uint8_t *)StartVirtual+(Offset-StartOffset))
#define BDSP_MMA_P_VirtualToOffset(StartVirtual,StartOffset,Virtual) (dramaddr_t)(StartOffset+((uint8_t *)Virtual-(uint8_t *)StartVirtual))

typedef struct BDSP_P_MemoryPool{
    BDSP_MMA_Memory Memory;
    uint32_t        ui32Size;
    uint32_t        ui32UsedSize;
}BDSP_P_MemoryPool;

typedef struct BDSP_P_FwBuffer{
    BDSP_MMA_Memory Buffer;
    uint32_t        ui32Size;
}BDSP_P_FwBuffer;

typedef struct BDSP_P_HostBuffer
{
    void       *pAddr;
    uint32_t    ui32Size;
}BDSP_P_HostBuffer;

typedef struct BDSP_P_BufferPointer{
    dramaddr_t BaseOffset;
    dramaddr_t ReadOffset;
    dramaddr_t WriteOffset;
    dramaddr_t EndOffset;
}BDSP_P_BufferPointer;

typedef struct BDSP_P_BufferDescriptor{
    void     *pBasePtr;      /*  Circular buffer's base address */
    void     *pEndPtr;       /*  Circular buffer's End address */
    void     *pReadPtr;      /*  Circular buffer's read address */
    void     *pWritePtr;     /*  Circular buffer's write address */
    void     *pWrapPtr;      /*  Circular buffer's wrap address */
}BDSP_P_BufferDescriptor;

void BDSP_P_CalculateDescriptorMemory(
    unsigned *pMemReqd
);
void BDSP_P_CalculateInterTaskMemory(
    unsigned  *pMemReqd,
    unsigned   numchannels
);
BERR_Code BDSP_MMA_P_AllocateAlignedMemory(BMMA_Heap_Handle memHandle,
    uint32_t size,
    BDSP_MMA_Memory *pMemory,
    BDSP_MMA_Alignment alignment
);
void BDSP_MMA_P_FreeMemory(BDSP_MMA_Memory *pMemory);
BERR_Code BDSP_MMA_P_CopyDataToDram (
    BDSP_MMA_Memory *dest,
    void *src,
    uint32_t size
);
BERR_Code BDSP_MMA_P_CopyDataToDram_isr (
    BDSP_MMA_Memory *dest,
    void *src,
    uint32_t size
);
BERR_Code BDSP_MMA_P_CopyDataFromDram (
    void *dest,
    BDSP_MMA_Memory *src,
    uint32_t size
);
BERR_Code BDSP_MMA_P_CopyDataFromDram_isr(
    void *dest,
    BDSP_MMA_Memory *src,
    uint32_t size
);
void* BDSP_MMA_P_GetVirtualAddressfromOffset(
    BDSP_P_FwBuffer *pBuffer,
    dramaddr_t       offset
);
dramaddr_t BDSP_MMA_P_GetOffsetfromVirtualAddress(
    BDSP_P_FwBuffer *pBuffer,
    void            *pAddr
);
BERR_Code BDSP_P_RequestMemory(
    BDSP_P_MemoryPool *pChunkBuffer,
    uint32_t ui32Size,
    BDSP_MMA_Memory *pMemory
);
BERR_Code BDSP_P_ReleaseMemory(
    BDSP_P_MemoryPool *pChunkBuffer,
    uint32_t ui32Size,
    BDSP_MMA_Memory *pMemory
);
#endif /*BDSP_COMMON_MM_PRIV_H_*/

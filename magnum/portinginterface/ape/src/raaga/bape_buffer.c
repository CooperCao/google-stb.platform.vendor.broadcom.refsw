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
 * Module Description: APE Buffer Interface
 *
***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bape_buffer.h"

BDBG_MODULE(bape_buffer);
BDBG_FILE_MODULE(bape_buffer_detail);
BDBG_FILE_MODULE(bape_buffer_lock);

BDBG_OBJECT_ID(BAPE_Buffer);
BDBG_OBJECT_ID(BAPE_BufferGroup);

#define INVALID_SIZE    0xffffffff

static BERR_Code BAPE_Buffer_P_ControlLock_isrsafe(BAPE_BufferHandle handle);
static BERR_Code BAPE_Buffer_P_ControlUnlock_isrsafe(BAPE_BufferHandle handle);
static BERR_Code BAPE_BufferGroup_P_ControlLock_isrsafe(BAPE_BufferGroupHandle handle);
static BERR_Code BAPE_BufferGroup_P_ControlUnlock_isrsafe(BAPE_BufferGroupHandle handle);
static BERR_Code BAPE_BufferGroup_P_FillOutputs_isr(BAPE_BufferGroupHandle pSource);
static void BAPE_BufferGroup_P_DataReady_isr(BAPE_BufferGroupHandle handle, unsigned size);
static void BAPE_BufferGroup_P_FreeSpaceAvailable_isr(BAPE_BufferGroupHandle handle, unsigned size);

typedef struct BAPE_Buffer
{
    BDBG_OBJECT(BAPE_Buffer)

    BAPE_Handle deviceHandle;

    BMMA_Block_Handle interfaceBlock;
    BMMA_DeviceOffset interfaceOffset;
    BAPE_BufferInterface * pInterface;
    bool userAllocatedBuffer;
    BMMA_Block_Handle block;
    BMMA_DeviceOffset offset;
    void * pBuffer;
    size_t size;
    BAPE_BufferType type;
    BAPE_BufferSettings settings;
} BAPE_Buffer;

/***************************************************************************
Summary:
Buffer Get Default Settings
***************************************************************************/
void BAPE_Buffer_GetDefaultSettings(
    BAPE_BufferSettings *pSettings /*[out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

/***************************************************************************
Summary:
Buffer Open
***************************************************************************/
BERR_Code BAPE_Buffer_Open(
    BAPE_Handle deviceHandle,
    const BAPE_BufferSettings * pSettings, 
    BAPE_BufferHandle * pHandle /* [out] */
    )
{
    BERR_Code errCode = BERR_SUCCESS;
    BMMA_Heap_Handle heap = NULL;
    BAPE_BufferHandle handle;
    BMMA_Block_Handle block = NULL;
    BMMA_DeviceOffset offset = 0;
    void * pBuffer = NULL;
    unsigned size = 0;
    BMMA_Block_Handle interfaceBlock = NULL;
    BAPE_BufferInterface * pInterface = NULL;
    BMMA_DeviceOffset interfaceOffset = 0;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    /* check for valid settings */
    if ( NULL == pSettings )
    {
        BDBG_ERR(("Buffer settings null"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* check for valid pHandle */
    if ( NULL == pHandle )
    {
        BDBG_ERR(("pHandle cannot be null"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_MSG(("%s - pBuffer %p, size %lu, heap %p, bufferInterface %p", BSTD_FUNCTION, (void *)pSettings->pBuffer, (unsigned long)pSettings->bufferSize, (void*)pSettings->heap, (void*)pSettings->pInterface));
    if ( pSettings->pInterface || pSettings->interfaceBlock || pSettings->pBuffer )
    {
        if ( !pSettings->interfaceBlock || pSettings->pBuffer == NULL || pSettings->pInterface == NULL )
        {
            BDBG_ERR(("MMA block handle, buffer ptr, and interface ptr must all be specified when a buffer interface is used"));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        interfaceBlock = pSettings->interfaceBlock;
        interfaceOffset = pSettings->interfaceOffset;
        pInterface = pSettings->pInterface;
        BDBG_ASSERT(pInterface->end > pInterface->base);
        size = pInterface->end - pInterface->base;
        block = pInterface->block;
        pBuffer = pSettings->pBuffer;
        offset = pInterface->base;
    }
    else /* allocate resources ourselves */
    {
        unsigned allocSize;
        if ( pSettings->heap )
        {
            heap = pSettings->heap;
        }
        else
        {
            heap = deviceHandle->memHandle;
        }
        if ( NULL == heap )
        {
            BDBG_ERR(("no heap specified"));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        allocSize = pSettings->bufferSize;
        BAPE_SIZE_ALIGN(allocSize);
        BDBG_MSG(("%s - calling BMMA_Alloc(%p, %lu)", BSTD_FUNCTION, (void *)heap, (unsigned long)allocSize));
        block = BMMA_Alloc(heap, allocSize, BAPE_ADDRESS_ALIGN, NULL);
        if ( NULL == block )
        {
            BDBG_ERR(("failed to allocate %lu byte buffer", (unsigned long)allocSize));
            return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        }
        size = allocSize;
        pBuffer = BMMA_Lock(block);
        if ( NULL == pBuffer )
        {
            BDBG_ERR(("could not lock block"));
            errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto err_cleanup;
        }
        offset = BMMA_LockOffset(block);
        if ( 0 == offset )
        {
            BDBG_ERR(("could not lock offset"));
            errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto err_cleanup;
        }

        allocSize = sizeof(BAPE_BufferInterface);
        BAPE_SIZE_ALIGN(allocSize);
        BDBG_MSG(("%s - calling BMMA_Alloc(%p, %lu)", BSTD_FUNCTION, (void *)heap, (unsigned long)allocSize));
        interfaceBlock = BMMA_Alloc(heap, allocSize, BAPE_ADDRESS_ALIGN, NULL);
        if ( NULL == interfaceBlock )
        {
            BDBG_ERR(("failed to allocate %lu byte buffer", (unsigned long)allocSize));
            errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto err_cleanup;
        }
        pInterface = BMMA_Lock(interfaceBlock);
        if ( NULL == pInterface )
        {
            BDBG_ERR(("could not lock block"));
            errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto err_cleanup;
        }
        interfaceOffset = BMMA_LockOffset(interfaceBlock);
        if ( 0 == interfaceOffset )
        {
            BDBG_ERR(("could not lock offset"));
            errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto err_cleanup;
        }

        BKNI_Memset(pBuffer, 0, pSettings->bufferSize);
        BAPE_FLUSHCACHE_ISRSAFE(block, pBuffer, pSettings->bufferSize);
        BKNI_Memset(pInterface, 0, sizeof(BAPE_BufferInterface));
        pInterface->base = offset;
        pInterface->read = pInterface->base;
        pInterface->valid = pInterface->base;
        pInterface->end = pInterface->base + size;
        BAPE_FLUSHCACHE_ISRSAFE(interfaceBlock, pInterface, sizeof(BAPE_BufferInterface));
    }

    /* Allocate the device structure, then fill in all the fields. */
    handle = BKNI_Malloc(sizeof(BAPE_Buffer));
    if ( NULL == handle )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_cleanup;
    }

    BKNI_Memset(handle, 0, sizeof(BAPE_Buffer));
    BDBG_OBJECT_SET(handle, BAPE_Buffer);
    handle->userAllocatedBuffer = (pSettings->pBuffer != NULL);
    handle->block = block;
    handle->pBuffer = pBuffer;
    handle->offset = offset;

    handle->size = size;
    handle->interfaceBlock = interfaceBlock;
    handle->interfaceOffset = interfaceOffset;
    handle->pInterface = pInterface;

    handle->type = pSettings->type;
    handle->settings = *pSettings;

    BDBG_MSG(("%s - SUCCESS, returning BAPE_BufferHandle %p", BSTD_FUNCTION, (void *)handle));

    *pHandle = handle;
    return BERR_SUCCESS;

err_cleanup:
    if ( offset != 0 )
    {
        BMMA_UnlockOffset(block, offset);
        offset = 0;
    }
    if ( pBuffer != NULL )
    {
        BMMA_Unlock(block, pBuffer);
        pBuffer = NULL;
    }
    if ( block != NULL )
    {
        BMMA_Free(block);
        block = NULL;
    }
    if ( interfaceOffset != 0 )
    {
        BMMA_UnlockOffset(interfaceBlock, interfaceOffset);
        interfaceOffset = 0;
    }
    if ( pInterface != NULL )
    {
        BMMA_Unlock(interfaceBlock, pInterface);
        pInterface = NULL;
    }
    if ( interfaceBlock != NULL )
    {
        BMMA_Free(interfaceBlock);
        interfaceBlock = NULL;
    }
    return errCode;
}

/***************************************************************************
Summary:
Buffer Close
***************************************************************************/
void BAPE_Buffer_Close(
    BAPE_BufferHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Buffer);

    BDBG_MSG(("%s - (%p)", BSTD_FUNCTION, (void*)handle));
    if ( !handle->userAllocatedBuffer )
    {
        if ( handle->interfaceOffset != 0 )
        {
            BMMA_UnlockOffset(handle->interfaceBlock, handle->interfaceOffset);
            handle->interfaceOffset = 0;
        }
        if ( handle->pInterface != NULL )
        {
            BMMA_Unlock(handle->interfaceBlock, handle->pInterface);
            handle->pInterface = NULL;
        }
        if ( handle->interfaceBlock != NULL )
        {
            BMMA_Free(handle->interfaceBlock);
            handle->interfaceBlock = NULL;
        }

        BDBG_MSG(("Buffer block %p offset " BDBG_UINT64_FMT " pBuffer %p",
                  (void*)handle->block, BDBG_UINT64_ARG(handle->offset), (void*)handle->pBuffer));
        if ( handle->offset != 0 )
        {
            BMMA_UnlockOffset(handle->block, handle->offset);
            handle->offset = 0;
        }
        if ( handle->pBuffer != NULL )
        {
            BMMA_Unlock(handle->block, handle->pBuffer);
            handle->pBuffer = NULL;
        }
        if ( handle->block != NULL )
        {
            BMMA_Free(handle->block);
            handle->block = NULL;
        }
    }

    BDBG_OBJECT_DESTROY(handle, BAPE_Buffer);
    BKNI_Free(handle);
    BDBG_MSG(("  - SUCCESS, returning"));
}

/***************************************************************************
Summary:
Buffer Read
***************************************************************************/
unsigned BAPE_Buffer_Read_isr(
    BAPE_BufferHandle handle, 
    BAPE_SimpleBufferDescriptor * pDescriptor /* [out] */
    )
{
    BMMA_DeviceOffset base, end, read, valid, watermark = 0;
    unsigned size = 0;

    BDBG_ASSERT(pDescriptor != NULL);
    BDBG_OBJECT_ASSERT(handle, BAPE_Buffer);

    BKNI_Memset(pDescriptor, 0, sizeof(BAPE_SimpleBufferDescriptor));

    if ( handle->type == BAPE_BufferType_eWriteOnly )
    {
        BDBG_WRN(("Attempt to read from a write only buffer."));
        BERR_TRACE(BERR_UNKNOWN);
        return 0;
    }

    /* Flush Cache */
    BAPE_FLUSHCACHE_ISRSAFE(handle->interfaceBlock, handle->pInterface, sizeof(BAPE_BufferInterface));

    if ( !BAPE_BUFFER_INTERFACE_ENABLED(handle->pInterface->control) )
    {
        BDBG_WRN(("Buffer interface not enabled"));
        return 0;
    }

    /* Snap offsets */
    base = handle->pInterface->base;
    end = handle->pInterface->end;
    read = handle->pInterface->read;
    valid = handle->pInterface->valid;
    watermark = handle->pInterface->watermark;

    BAPE_BUFFER_ASSERT_INTERFACE_VALID(base, end, read, valid);
    BDBG_MODULE_MSG(bape_buffer_detail, ("++READ(%p) - base %x, read %x, write %x, end %x",(void *)handle,(unsigned)base,(unsigned)read,(unsigned)valid,(unsigned)end));

    /* check for read ptr wrap */
    if ( read == end )
    {
        if ( valid != end )
        {
            read = base;
        }
    }

    BDBG_MODULE_MSG(bape_buffer_detail, ("  READ(%p) - base %x, read %x, write %x, end %x",(void *)handle,(unsigned)base,(unsigned)read,(unsigned)valid,(unsigned)end));

    if ( read != handle->pInterface->read )
    {
        handle->pInterface->read = read;
        /* Flush Cache */
        BAPE_FLUSHCACHE_ISRSAFE(handle->interfaceBlock, handle->pInterface, sizeof(BAPE_BufferInterface));
    }

    if ( (watermark < handle->pInterface->base || watermark > handle->pInterface->end) && watermark < (handle->pInterface->end - handle->pInterface->base) )
    {
        watermark += base;
    }

    BDBG_ASSERT( watermark >= base );
    BDBG_ASSERT( watermark < end );

    if ( watermark != base && valid < watermark )
    {
        /* waiting to hit watermark */
        BDBG_MODULE_MSG(bape_buffer_detail, ("  returning. watermark %u not hit yet. valid - base = %u", (unsigned int)watermark, (unsigned int)(valid-base)));
        return 0;
    }

    /* we are hitting the watermark, clear it */
    if ( watermark != base )
    {
        handle->pInterface->watermark = 0;
        /* Flush Cache */
        BAPE_FLUSHCACHE_ISRSAFE(handle->interfaceBlock, handle->pInterface, sizeof(BAPE_BufferInterface));
    }

    /* Set up descriptor */
    if ( read < valid )
    {
        pDescriptor->block = handle->block;
        pDescriptor->offset = read;
        pDescriptor->pBuffer = (uint8_t*)handle->pBuffer + (pDescriptor->offset-base);
        pDescriptor->bufferSize = valid - read;
        size = pDescriptor->bufferSize;
    }
    else if ( read == valid && read == end ) /* empty, waiting for write ptr to wrap */
    {
        /* return 0 below */
    }
    else if ( read > valid )
    {
        /* read -> end */
        BDBG_ASSERT( read < end );
        pDescriptor->block = handle->block;
        pDescriptor->offset = read;
        pDescriptor->pBuffer = (uint8_t*)handle->pBuffer + (pDescriptor->offset-base);
        pDescriptor->bufferSize = (unsigned) (end - read);
        size = pDescriptor->bufferSize;
        if ( valid > base )
        {
            pDescriptor->pWrapBuffer = handle->pBuffer;
            pDescriptor->wrapOffset = base;
            pDescriptor->wrapBufferSize = (unsigned) (valid - base);
            size += pDescriptor->wrapBufferSize;
        }
    }
    BDBG_MODULE_MSG(bape_buffer_detail, ("  returning %d bytes", size));
    BDBG_MODULE_MSG(bape_buffer_detail, ("--READ(%p) - base %x, read %x, write %x, end %x",(void *)handle,(unsigned)base,(unsigned)read,(unsigned)valid,(unsigned)end));

    return size;
}

/***************************************************************************
Summary:
Get Buffer for writing
***************************************************************************/
BERR_Code BAPE_Buffer_GetWriteBuffer_isr(
    BAPE_BufferHandle handle,
    BAPE_SimpleBufferDescriptor * pDescriptor /* [out] */
    )
{
    BMMA_DeviceOffset base, end, read, valid;

    BDBG_OBJECT_ASSERT(handle, BAPE_Buffer);

    if ( pDescriptor == NULL )
    {
        BDBG_ERR(("NULL descriptor not allowed"));
        return BERR_TRACE(BERR_UNKNOWN);
    }

    BKNI_Memset(pDescriptor, 0, sizeof(BAPE_SimpleBufferDescriptor));

    if ( handle->type == BAPE_BufferType_eReadOnly )
    {
        BDBG_WRN(("Attempt to get write buffers from read only buffer."));
        return BERR_TRACE(BERR_UNKNOWN);
    }

    /* Flush Cache */
    BAPE_FLUSHCACHE_ISRSAFE(handle->interfaceBlock, handle->pInterface, sizeof(BAPE_BufferInterface));

    if ( !BAPE_BUFFER_INTERFACE_ENABLED(handle->pInterface->control) )
    {
        BDBG_WRN(("Buffer interface not enabled"));
        return 0;
    }

    /* Snap offsets */
    base = handle->pInterface->base;
    end = handle->pInterface->end;
    read = handle->pInterface->read;
    valid = handle->pInterface->valid;

    BDBG_MODULE_MSG(bape_buffer_detail, ("++GETWRITEBUFFER(%p) - base %x, read %x, write %x, end %x",(void *)handle,(unsigned)base,(unsigned)read,(unsigned)valid,(unsigned)end));
    BAPE_BUFFER_ASSERT_INTERFACE_VALID(base, end, read, valid);

    /* check for write ptr wrap */
    if ( valid >= end )
    {
        if ( read > base )
        {
            valid = base;
        }
    }

    BDBG_MODULE_MSG(bape_buffer_detail, ("  GETWRITEBUFFER(%p) - base %x, read %x, write %x, end %x",(void *)handle,(unsigned)base,(unsigned)read,(unsigned)valid,(unsigned)end));
    if ( handle->pInterface->valid != valid )
    {
        handle->pInterface->valid = valid;
        /* Flush Cache */
        BAPE_FLUSHCACHE_ISRSAFE(handle->interfaceBlock, handle->pInterface, sizeof(BAPE_BufferInterface));
    }

    /* Set up descriptor */
    if ( valid == read ) /* empty */
    {
        BDBG_ASSERT( valid < end );

        pDescriptor->block = handle->block;
        pDescriptor->offset = valid;
        pDescriptor->pBuffer = (uint8_t*)handle->pBuffer + (pDescriptor->offset-base);
        pDescriptor->bufferSize = end - valid;
        if ( read != base )
        {
            pDescriptor->pWrapBuffer = handle->pBuffer;
            pDescriptor->wrapOffset = base;
            pDescriptor->wrapBufferSize = read - base - 1; /* leave 1 byte in between valid -> read */
        }
    }
    else if ( (valid + 1) < read )
    {
        pDescriptor->block = handle->block;
        pDescriptor->offset = valid;
        pDescriptor->pBuffer = (uint8_t*)handle->pBuffer + (pDescriptor->offset-base);
        pDescriptor->bufferSize = read - valid - 1; /* leave 1 byte in between valid -> read */
    }
    else if ( read < valid )
    {
        pDescriptor->block = handle->block;
        pDescriptor->offset = valid;
        pDescriptor->pBuffer = (uint8_t*)handle->pBuffer + (pDescriptor->offset-base);
        pDescriptor->bufferSize = (unsigned) (end - valid);
        if ( read != base )
        {
            pDescriptor->pWrapBuffer = handle->pBuffer;
            pDescriptor->wrapOffset = base;
            pDescriptor->wrapBufferSize = (unsigned) (read - base - 1);  /* leave 1 byte in between valid -> read */
        }
    }
    BDBG_MODULE_MSG(bape_buffer_detail, ("  returning %d bytes of buffer for writing", pDescriptor->bufferSize + pDescriptor->wrapBufferSize));
    BDBG_MODULE_MSG(bape_buffer_detail, ("--GETWRITEBUFFER(%p) - base %x, read %x, write %x, end %x",(void *)handle,(unsigned)base,(unsigned)read,(unsigned)valid,(unsigned)end));

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Buffer Write - if pData is NULL, just move ptrs (WriteComplete)
***************************************************************************/
unsigned BAPE_Buffer_Write_isr(
    BAPE_BufferHandle handle, 
    void * pData,
    unsigned size
    )
{
    BMMA_DeviceOffset base, end, read, valid;
    bool copy = (pData != NULL);
    unsigned written = 0;
    unsigned currentWriteSize;

    if ( size == 0 )
    {
        return 0;
    }

    BDBG_OBJECT_ASSERT(handle, BAPE_Buffer);

    if ( handle->type == BAPE_BufferType_eReadOnly )
    {
        BDBG_WRN(("Attempt to write to read only buffer."));
        BERR_TRACE(BERR_UNKNOWN);
        return 0;
    }

    /* Flush Cache */
    BAPE_FLUSHCACHE_ISRSAFE(handle->interfaceBlock, handle->pInterface, sizeof(BAPE_BufferInterface));

    /* Snap offsets */
    base = handle->pInterface->base;
    end = handle->pInterface->end;
    read = handle->pInterface->read;
    valid = handle->pInterface->valid;

    BDBG_MODULE_MSG(bape_buffer_detail, ("++WRITE(%p) - base %x, read %x, write %x, end %x",(void *)handle,(unsigned)base,(unsigned)read,(unsigned)valid,(unsigned)end));
    BAPE_BUFFER_ASSERT_INTERFACE_VALID(base, end, read, valid);

    /* check for write ptr wrap */
    if ( valid == end )
    {
        if ( read > base )
        {
            valid = base;
        }
    }

    BDBG_MODULE_MSG(bape_buffer_detail, ("  WRITE(%p) - base %x, read %x, write %x, end %x",(void *)handle,(unsigned)base,(unsigned)read,(unsigned)valid,(unsigned)end));

    /* we can assert here that read and valid are not BOTH at the end */

    /* write in space between write ptr and end ptr */
    if ( read <= valid )
    {
        BDBG_ASSERT(valid<end);
        currentWriteSize = BAPE_MIN(size, end - valid);

        if ( copy )
        {
            BKNI_Memcpy((uint8_t *)handle->pBuffer + (valid-base), pData, currentWriteSize);
        }
        written += currentWriteSize;
        valid += currentWriteSize;
    }

    BAPE_BUFFER_ASSERT_INTERFACE_VALID(base, end, read, valid);

    /* check again for write ptr wrap since we may have just moved write ptr */
    if ( valid == end )
    {
        if ( read != base )
        {
            valid = base;
        }
    }

    BDBG_MODULE_MSG(bape_buffer_detail, ("  WRITE(%p) - base %x, read %x, write %x, end %x",(void *)handle,(unsigned)base,(unsigned)read,(unsigned)valid,(unsigned)end));

    /* write ptr may have wrapped - write data between write and read.
       check for at least 1 free byte */
    if ( written < size && (valid + 1) < read )
    {
        currentWriteSize = BAPE_MIN(size - written, read - valid - 1);

        if ( copy )
        {
            BKNI_Memcpy((uint8_t *)handle->pBuffer + (valid-base), (unsigned char *)pData + written, currentWriteSize);
        }
        written += currentWriteSize;
        valid += currentWriteSize;
        BDBG_ASSERT(valid<read);
    }

    /* move write ptr */
    handle->pInterface->valid = valid;
    BAPE_BUFFER_ASSERT_INTERFACE_VALID(base, end, read, valid);

    /* Flush Cache */
    BAPE_FLUSHCACHE_ISRSAFE(handle->interfaceBlock, handle->pInterface, sizeof(BAPE_BufferInterface));

    if ( written < size )
    {
        BDBG_WRN(("WRITE(%p) - WARNING - buffer full. only wrote %d of %d bytes", (void *)handle, written, size));
        BERR_TRACE(BERR_UNKNOWN);
    }

    BDBG_MODULE_MSG(bape_buffer_detail, ("  wrote %d of %d bytes", written, size));
    BDBG_MODULE_MSG(bape_buffer_detail, ("--WRITE(%p) - base %x, read %x, write %x, end %x",(void *)handle,(unsigned)base,(unsigned)read,(unsigned)valid,(unsigned)end));

    return written;
}

/***************************************************************************
Summary:
Buffer Write Complete
***************************************************************************/
unsigned BAPE_Buffer_WriteComplete_isr(
    BAPE_BufferHandle handle,
    unsigned size
    )
{
    return BAPE_Buffer_Write_isr(handle, NULL, size);
}

/***************************************************************************
Summary:
Buffer Read Complete
***************************************************************************/
unsigned BAPE_Buffer_ReadComplete_isr(
    BAPE_BufferHandle handle, 
    unsigned size
    )
{
    BMMA_DeviceOffset base, end, read, valid;
    unsigned advanced = 0;
    unsigned curAdvanceSize;
    unsigned depth;

    BDBG_OBJECT_ASSERT(handle, BAPE_Buffer);

    if ( size == 0 )
    {
        return 0;
    }

    if ( handle->type == BAPE_BufferType_eWriteOnly )
    {
        BDBG_WRN(("Attempt to read complete a write only buffer"));
        BERR_TRACE(BERR_UNKNOWN);
        return 0;
    }

    depth = BAPE_Buffer_GetBufferDepth_isr(handle);

    if ( depth < size )
    {
        BDBG_ERR(("READCOMPLETE(%p) - ERROR - Request exceeds buffer depth (requested %lu, depth %lu)", (void *)handle, (unsigned long)size, (unsigned long)depth));
    }

    /* Flush Cache */
    BAPE_FLUSHCACHE_ISRSAFE(handle->interfaceBlock, handle->pInterface, sizeof(BAPE_BufferInterface));

    if ( !BAPE_BUFFER_INTERFACE_ENABLED(handle->pInterface->control) )
    {
        BDBG_WRN(("Buffer interface not enabled"));
        return 0;
    }

    /* Snap offsets */
    base = handle->pInterface->base;
    end = handle->pInterface->end;
    read = handle->pInterface->read;
    valid = handle->pInterface->valid;

    BDBG_MODULE_MSG(bape_buffer_detail, ("++READCOMPLETE(%p) - base %x, end %x, read %x, write %x, bufSize %x, AdvSize %x", (void *)handle, (unsigned)base, (unsigned)end, (unsigned)read, (unsigned)valid, (unsigned)handle->size, (unsigned)size));
    BAPE_BUFFER_ASSERT_INTERFACE_VALID(base, end, read, valid);

    /* check for read ptr wrap */
    if ( read == end )
    {
        if ( valid != end )
        {
            read = base;
        }
    }

    /* advance read ptr between read and end of buffer */
    if ( read > valid )
    {
        curAdvanceSize = BAPE_MIN(end - read, (unsigned long)size);
        read += curAdvanceSize;
        advanced += curAdvanceSize;

    }

    /* check for read ptr wrap again, since we may have just move read ptr */
    if ( read == end )
    {
        if ( valid != end )
        {
            read = base;
        }
    }

    /* advance read ptr chasing write ptr.
       read ptr can meet write ptr, indicating buffer empty */
    if ( advanced < size && read < valid )
    {
        curAdvanceSize = BAPE_MIN(valid - read, size - advanced);
        read += curAdvanceSize;
        advanced += curAdvanceSize;
    }

    handle->pInterface->read = read;
    BAPE_BUFFER_ASSERT_INTERFACE_VALID(base, end, read, valid);
    BDBG_MODULE_MSG(bape_buffer_detail, ("  Consumed %u of %u bytes", advanced, size));
    BDBG_MODULE_MSG(bape_buffer_detail, ("--READCOMPLETE(%p) - base %x, end %x, read %x, write %x, bufSize %x, AdvSize %x", (void *)handle, (unsigned)base, (unsigned)end, (unsigned)read, (unsigned)valid, (unsigned)handle->size, (unsigned)size));

    /* Flush Cache */
    BAPE_FLUSHCACHE_ISRSAFE(handle->interfaceBlock, handle->pInterface, sizeof(BAPE_BufferInterface));

    return (unsigned)advanced;
}

/***************************************************************************
Summary:
Buffer Flush
***************************************************************************/
void BAPE_Buffer_Flush_isr(
    BAPE_BufferHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Buffer);

    BDBG_MSG(("%s - (%p)", BSTD_FUNCTION, (void*)handle));

    /* Flush Cache */
    BAPE_FLUSHCACHE_ISRSAFE(handle->interfaceBlock, handle->pInterface, sizeof(BAPE_BufferInterface));
    handle->pInterface->read = handle->pInterface->base;
    handle->pInterface->valid = handle->pInterface->base;
    BAPE_FLUSHCACHE_ISRSAFE(handle->interfaceBlock, handle->pInterface, sizeof(BAPE_BufferInterface));
}

/***************************************************************************
Summary:
Buffer Get Depth
***************************************************************************/
unsigned BAPE_Buffer_GetBufferDepth_isr(
    BAPE_BufferHandle handle
    )
{
    BMMA_DeviceOffset base, end, read, valid;

    BDBG_OBJECT_ASSERT(handle, BAPE_Buffer);

    /* Flush Cache */
    BAPE_FLUSHCACHE_ISRSAFE(handle->interfaceBlock, handle->pInterface, sizeof(BAPE_BufferInterface));

    if ( !BAPE_BUFFER_INTERFACE_ENABLED(handle->pInterface->control) )
    {
        BDBG_WRN(("Buffer interface not enabled"));
        return 0;
    }

    /* Snap offsets */
    base = handle->pInterface->base;
    end = handle->pInterface->end;
    read = handle->pInterface->read;
    valid = handle->pInterface->valid;
    BAPE_BUFFER_ASSERT_INTERFACE_VALID(base, end, read, valid);

    if ( read < valid )
    {
        return ( valid - read );
    }
    else if ( read > valid )
    {
        return (end - read) + (valid - base);
    }

    return 0;
}

/***************************************************************************
Summary:
Buffer Get Free Space
***************************************************************************/
unsigned BAPE_Buffer_GetBufferFree_isr(
    BAPE_BufferHandle handle
    )
{
    unsigned free = 0;
    BMMA_DeviceOffset base, end, read, valid;

    BDBG_OBJECT_ASSERT(handle, BAPE_Buffer);

    /* Flush Cache */
    BAPE_FLUSHCACHE_ISRSAFE(handle->interfaceBlock, handle->pInterface, sizeof(BAPE_BufferInterface));

    if ( !BAPE_BUFFER_INTERFACE_ENABLED(handle->pInterface->control) )
    {
        BDBG_WRN(("Buffer interface not enabled"));
        return 0;
    }

    /* Snap offsets */
    base = handle->pInterface->base;
    end = handle->pInterface->end;
    read = handle->pInterface->read;
    valid = handle->pInterface->valid;
    BAPE_BUFFER_ASSERT_INTERFACE_VALID(base, end, read, valid);

    if ( valid == read && valid == base ) /* special case, whole buffer may be reported */
    {
        free = end - base;
    }
    else if ( valid == read )
    {
        free = end - base - 1;
    }

    if ( read < valid )
    {
        free = (end - valid);
        if ( read > base )
        {
            free += read - base - 1;
        }
    }
    else if ( valid < read )
    {
        free = read - valid - 1;
    }

    return free;
}

/***************************************************************************
Summary:
Buffer Enable Consumption
***************************************************************************/
void BAPE_Buffer_Enable_isr(
    BAPE_BufferHandle handle,
    bool enable
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Buffer);

    BDBG_MSG(("%s - (%p)", BSTD_FUNCTION, (void*)handle));

    /* Flush Cache */
    BAPE_FLUSHCACHE_ISRSAFE(handle->interfaceBlock, handle->pInterface, sizeof(BAPE_BufferInterface));

    if ( enable )
    {
        handle->pInterface->control |= BAPE_BUFFER_INTERFACE_CTRL_ENABLE_MASK;
    }
    else
    {
        handle->pInterface->control &= ~BAPE_BUFFER_INTERFACE_CTRL_ENABLE_MASK;
    }

    /* Flush Cache */
    BAPE_FLUSHCACHE_ISRSAFE(handle->interfaceBlock, handle->pInterface, sizeof(BAPE_BufferInterface));
}

/***************************************************************************
Summary:
Buffer report enable status
***************************************************************************/
bool BAPE_Buffer_IsEnabled_isrsafe(
    BAPE_BufferHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Buffer);

    /* Flush Cache */
    BAPE_FLUSHCACHE_ISRSAFE(handle->interfaceBlock, handle->pInterface, sizeof(BAPE_BufferInterface));
    return BAPE_BUFFER_INTERFACE_ENABLED(handle->pInterface->control);
}

/***************************************************************************
Summary:
Buffer Get Format
***************************************************************************/
BERR_Code BAPE_Buffer_GetFormat_isrsafe(
    BAPE_BufferHandle handle,
    BAPE_BufferFormat * pFormat
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Buffer);

    if ( pFormat == NULL )
    {
        BDBG_ERR(("pFormat cannot be NULL"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Flush Cache */
    BAPE_FLUSHCACHE_ISRSAFE(handle->interfaceBlock, handle->pInterface, sizeof(BAPE_BufferInterface));

    pFormat->interleaved = BAPE_BUFFER_INTERFACE_FMT_INTERLEAVED(handle->pInterface->format);
    pFormat->compressed = BAPE_BUFFER_INTERFACE_FMT_COMPRESSED(handle->pInterface->format);
    pFormat->bitsPerSample = BAPE_BUFFER_INTERFACE_FMT_BITSPERSAMPLE(handle->pInterface->format);
    pFormat->numChannels = BAPE_BUFFER_INTERFACE_FMT_NUMCHANNELS(handle->pInterface->format);
    pFormat->samplesPerDword = BAPE_BUFFER_INTERFACE_FMT_SAMPLESPERDWORD(handle->pInterface->format);

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Buffer Set Format
***************************************************************************/
BERR_Code BAPE_Buffer_SetFormat_isr(
    BAPE_BufferHandle handle,
    BAPE_BufferFormat * pFormat
    )
{
    uint32_t format;
    BDBG_OBJECT_ASSERT(handle, BAPE_Buffer);

    if ( pFormat == NULL )
    {
        BDBG_ERR(("pFormat cannot be NULL"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( handle->type == BAPE_BufferType_eReadOnly )
    {
        BDBG_ERR(("Read Only buffer interface cannot modify format"));
        return BERR_TRACE(BERR_NOT_AVAILABLE);
    }

    /* Flush Cache */
    BAPE_FLUSHCACHE_ISRSAFE(handle->interfaceBlock, handle->pInterface, sizeof(BAPE_BufferInterface));

    /* TBD7211 - if samplerate is added, this will need to be relaxed. */
    if ( BAPE_BUFFER_INTERFACE_ENABLED(handle->pInterface->control) )
    {
        BDBG_ERR(("Cannot update the format while running"));
        return BERR_TRACE(BERR_NOT_AVAILABLE);
    }

    format = handle->pInterface->format;
    format &= ~BAPE_BUFFER_INTERFACE_FMT_INTERLEAVED_MASK;
    format |= pFormat->interleaved?BAPE_BUFFER_INTERFACE_FMT_INTERLEAVED_MASK : 0;

    format &= ~BAPE_BUFFER_INTERFACE_FMT_COMPRESSED_MASK;
    format |= pFormat->compressed?BAPE_BUFFER_INTERFACE_FMT_COMPRESSED_MASK : 0;

    format &= ~BAPE_BUFFER_INTERFACE_FMT_BITSPERSAMPLE_MASK;
    format |= (pFormat->bitsPerSample<<BAPE_BUFFER_INTERFACE_FMT_BITSPERSAMPLE_SHIFT) & BAPE_BUFFER_INTERFACE_FMT_BITSPERSAMPLE_MASK;

    format &= ~BAPE_BUFFER_INTERFACE_FMT_SAMPLESPERDWORD_MASK;
    format |= (pFormat->samplesPerDword<<BAPE_BUFFER_INTERFACE_FMT_SAMPLESPERDWORD_SHIFT) & BAPE_BUFFER_INTERFACE_FMT_SAMPLESPERDWORD_MASK;

    format &= ~BAPE_BUFFER_INTERFACE_FMT_NUMCHANNELS_MASK;
    format |= (pFormat->numChannels<<BAPE_BUFFER_INTERFACE_FMT_NUMCHANNELS_SHIFT) & BAPE_BUFFER_INTERFACE_FMT_NUMCHANNELS_MASK;

    handle->pInterface->format = format;

    /* Flush Cache */
    BAPE_FLUSHCACHE_ISRSAFE(handle->interfaceBlock, handle->pInterface, sizeof(BAPE_BufferInterface));

    return BERR_SUCCESS;
}

#define BAPE_BUFFER_LOCK(interface) \
    (__sync_bool_compare_and_swap(&interface->lock, 0, 1))
#define BAPE_BUFFER_UNLOCK(interface) \
    (__sync_bool_compare_and_swap(&interface->lock, 1, 0))
    /* interface->lock = 0; */

static BERR_Code BAPE_Buffer_P_ControlLock_isrsafe(BAPE_BufferHandle handle)
{
    unsigned count=500000;

    /* Flush Cache */
    BAPE_FLUSHCACHE_ISRSAFE(handle->interfaceBlock, handle->pInterface, sizeof(BAPE_BufferInterface));

    /* spin lock */
    while (!BAPE_BUFFER_LOCK(handle->pInterface) && count-- > 0 ) {
        BKNI_Delay(1);
    }

    if ( count == 0 )
    {
        BDBG_ERR(("ERROR: Took too long to lock"));
        return BERR_TRACE(BERR_UNKNOWN);
    }

    BDBG_ASSERT( handle->pInterface->lock == 1 );

    BDBG_MODULE_MSG(bape_buffer_lock, ("--LOCK   BUF %p, intf %p, lock(%p) %d", (void*)handle, (void*)handle->pInterface, (void*)&handle->pInterface->lock, (int)handle->pInterface->lock));

    /* Flush Cache */
    BAPE_FLUSHCACHE_ISRSAFE(handle->interfaceBlock, handle->pInterface, sizeof(BAPE_BufferInterface));

    return BERR_SUCCESS;
}

static BERR_Code BAPE_Buffer_P_ControlUnlock_isrsafe(BAPE_BufferHandle handle)
{
    BDBG_MODULE_MSG(bape_buffer_lock, ("++UNLOCK BUF %p, intf %p, lock(%p) %d", (void*)handle, (void*)handle->pInterface, (void*)&handle->pInterface->lock, (int)handle->pInterface->lock));

    /* Flush Cache */
    BAPE_FLUSHCACHE_ISRSAFE(handle->interfaceBlock, handle->pInterface, sizeof(BAPE_BufferInterface));

    if ( handle->pInterface->lock == 0 )
    {
        BDBG_ERR(("ERROR UNLOCK BUF %p, intf %p, lock %d prior to Unlock", (void*)handle, (void*)handle->pInterface, (int)handle->pInterface->lock));
        return BERR_TRACE(BERR_UNKNOWN);
    }

    if (!BAPE_BUFFER_UNLOCK(handle->pInterface)) {
        BDBG_ERR(("ERROR UNLOCK BUF %p, intf %p, Unlock failed", (void*)handle, (void*)handle->pInterface));
        return BERR_TRACE(BERR_UNKNOWN);
    }

    BDBG_MODULE_MSG(bape_buffer_lock, ("--UNLOCK BUF %p, intf %p, lock(%p) %d", (void*)handle, (void*)handle->pInterface, (void*)&handle->pInterface->lock, (int)handle->pInterface->lock));

    /* Flush Cache */
    BAPE_FLUSHCACHE_ISRSAFE(handle->interfaceBlock, handle->pInterface, sizeof(BAPE_BufferInterface));

    return BERR_SUCCESS;
}

/* ------------------- Buffer Group --------------------*/

typedef struct BAPE_BufferGroup
{
    BDBG_OBJECT(BAPE_BufferGroup)

    BAPE_Handle deviceHandle;

    BAPE_BufferType type;
    unsigned numChannelPairs;
    bool interleaved;
    bool buffered;
    BMMA_DeviceOffset localReadOffset; /* used for non-buffered linked outputs */
    unsigned bufferSize;
    BAPE_BufferHandle pBufferHandle[BAPE_Channel_eMax];
    BAPE_BufferGroupInterruptHandlers interrupts;
    bool enabled;

    BAPE_BufferGroupHandle pSource; /* set if this buffer group is linked to a source buffer group */

    BLST_S_ENTRY(BAPE_BufferGroup) node;
    BLST_S_HEAD(BufferGroupOutputList, BAPE_BufferGroup) outputList;
} BAPE_BufferGroup;

/***************************************************************************
Summary:
Buffer Get Default Settings
***************************************************************************/
void BAPE_BufferGroup_GetDefaultOpenSettings(
    BAPE_BufferGroupOpenSettings *pSettings /*[out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->numChannels = 1; /* stereo, since we are defaulting to interleaved */
    pSettings->interleaved = true;
}

/***************************************************************************
Summary:
Buffer Group Open
***************************************************************************/
BERR_Code BAPE_BufferGroup_Open(
    BAPE_Handle deviceHandle,
    const BAPE_BufferGroupOpenSettings * pSettings,
    BAPE_BufferGroupHandle * pHandle /* [out] */
    )
{
    unsigned numChannelPairs = 0;
    unsigned bufferSize = 0;
    BAPE_BufferGroupHandle handle = NULL;
    unsigned i;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    /* check for valid settings */
    if ( NULL == pSettings )
    {
        BDBG_ERR(("Buffer settings null"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* check for valid pHandle */
    if ( NULL == pHandle )
    {
        BDBG_ERR(("pHandle cannot be null"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_MSG(("BufferGroupOpen - numChannels %d, interleaved %d", pSettings->numChannels, pSettings->interleaved));
    switch ( pSettings->numChannels )
    {
    case 1:
        /* whether it is stereo interleaved or mono non interleaved, set to 1 chpair */
        numChannelPairs = pSettings->numChannels;
        break;
    case 3:
    case 4:
        if ( !pSettings->interleaved )
        {
            BDBG_ERR(("Invalid number of channels(%d) for a non-interleaved buffer group", (int)pSettings->numChannels));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
        numChannelPairs = pSettings->numChannels;
        break;
    case 2:
    case 6:
    case 8:
        if ( pSettings->interleaved )
        {
            BDBG_ERR(("Invalid number of channels(%d) for an interleaved buffer group", (int)pSettings->numChannels));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
        numChannelPairs = pSettings->numChannels / 2;
        break;
    default:
        BDBG_ERR(("Invalid number of channels(%d) for a buffer group", (int)pSettings->numChannels));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
        break; /* unreachable */
    }

    handle = BKNI_Malloc(sizeof(BAPE_BufferGroup));
    if ( !handle )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_cleanup;
    }

    BKNI_Memset(handle, 0, sizeof(BAPE_BufferGroup));
    BDBG_OBJECT_SET(handle, BAPE_BufferGroup);

    /* If we need buffers, create them here. */
    if ( !pSettings->bufferless )
    {
        for ( i = 0; i < numChannelPairs * 2; i++ )
        {
            if ( i%2 == 0 || !pSettings->interleaved )
            {
                BAPE_BufferSettings bufferSettings;
                BAPE_Buffer_GetDefaultSettings(&bufferSettings);
                /* read only turns into read write, in case a source buffer gets linked.
                   Buffer Group is responsible for managing write permissions.
                   Write only stays write only. */
                if ( pSettings->type == BAPE_BufferType_eWriteOnly )
                {
                    bufferSettings.type = BAPE_BufferType_eWriteOnly;
                }
                bufferSettings.heap = pSettings->heap;
                bufferSettings.bufferSize = pSettings->bufferSize;

                bufferSettings.interfaceBlock = pSettings->buffers[i].interfaceBlock;
                bufferSettings.interfaceOffset = pSettings->buffers[i].interfaceOffset;
                bufferSettings.pInterface = pSettings->buffers[i].pInterface;
                bufferSettings.pBuffer = pSettings->buffers[i].pBuffer;
                errCode = BAPE_Buffer_Open(deviceHandle, &bufferSettings, &handle->pBufferHandle[i]);
                if ( errCode != BERR_SUCCESS )
                {
                    BDBG_ERR(("failed to open buffer index %d", (int)i));
                    BERR_TRACE(errCode);
                    goto err_cleanup;
                }
                if ( bufferSize == 0 )
                {
                    bufferSize = handle->pBufferHandle[i]->size;
                }
                BDBG_MSG(("bufferSize %d, bufferDepth[%d] %d", (int)bufferSize, (int)i, (int)handle->pBufferHandle[i]->size));
                if ( handle->pBufferHandle[i]->size != bufferSize )
                {
                    BDBG_ERR(("buffer interface size does not match last buffer size %d", (int)bufferSize));
                    BERR_TRACE(BERR_UNKNOWN);
                    goto err_cleanup;
                }
            }
        }
    }

    handle->deviceHandle = deviceHandle;
    handle->type = pSettings->type;
    handle->numChannelPairs = numChannelPairs;
    handle->interleaved = pSettings->interleaved;
    handle->buffered = !pSettings->bufferless;
    handle->bufferSize = bufferSize;
    BLST_S_INIT(&handle->outputList);

    *pHandle = handle;
    return BERR_SUCCESS;

err_cleanup:
    BAPE_BufferGroup_Close(handle);
    *pHandle = NULL;
    return errCode;
}

/***************************************************************************
Summary:
Buffer Group Close
***************************************************************************/
void BAPE_BufferGroup_Close(
    BAPE_BufferGroupHandle handle
    )
{
    if ( handle != NULL )
    {
        unsigned i;
        for ( i = 0; i < handle->numChannelPairs*2; i++ )
        {
            if ( handle->pBufferHandle[i] )
            {
                BAPE_Buffer_Close(handle->pBufferHandle[i]);
                handle->pBufferHandle[i] = NULL;
            }
        }
        BKNI_Free(handle);
    }
}

/***************************************************************************
Summary:
Buffer Group - link an output buffer group
***************************************************************************/
BERR_Code BAPE_BufferGroup_LinkOutput(
    BAPE_BufferGroupHandle pSource,
    BAPE_BufferGroupHandle pDest
    )
{
    BAPE_BufferGroupStatus sourceStatus, destStatus;
    BAPE_BufferGroupHandle pBufferGroup = NULL;

    BDBG_OBJECT_ASSERT(pSource, BAPE_BufferGroup);
    BDBG_OBJECT_ASSERT(pDest, BAPE_BufferGroup);

    BAPE_BufferGroup_GetStatus_isrsafe(pSource, &sourceStatus);
    BAPE_BufferGroup_GetStatus_isrsafe(pDest, &destStatus);

    if ( destStatus.enabled || sourceStatus.enabled )
    {
        BDBG_ERR(("Cannot link outputs while one is running Source(%d) Dest(%d)", (int)sourceStatus.enabled, (int)destStatus.enabled));
        return BERR_TRACE(BERR_NOT_AVAILABLE);
    }

    if ( !pSource->buffered )
    {
        BDBG_ERR(("Cannot link outputs to a linked dummy output buffer group"));
        return BERR_TRACE(BERR_NOT_AVAILABLE);
    }

    for ( pBufferGroup = BLST_S_FIRST(&pSource->outputList);
        pBufferGroup != NULL;
        pBufferGroup = BLST_S_NEXT(pBufferGroup, node) ) {
        if ( pBufferGroup == pDest )
        {
            return BERR_SUCCESS;
        }
    }

    /* TBD7211 - Should we flush here? Maybe it's ok to allow consumers to keep old data.
       must flush here to assure that the destination is initialized properly */

    /* Set the source so we know we are linked */
    pDest->pSource = pSource;

    /* reset the read pointer offset */
    pDest->localReadOffset = 0;

    /* Add to the output list of the source */
    BLST_S_INSERT_HEAD(&pSource->outputList, pDest, node);
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Buffer Group - unlink an output buffer group
***************************************************************************/
void BAPE_BufferGroup_UnLinkOutput(
    BAPE_BufferGroupHandle pSource,
    BAPE_BufferGroupHandle pDest
    )
{
    BAPE_BufferGroupHandle pBufferGroup = NULL;

    BDBG_OBJECT_ASSERT(pSource, BAPE_BufferGroup);
    BDBG_OBJECT_ASSERT(pDest, BAPE_BufferGroup);

    for ( pBufferGroup = BLST_S_FIRST(&pSource->outputList);
        pBufferGroup != NULL;
        pBufferGroup = BLST_S_NEXT(pBufferGroup, node) ) {
        if ( pBufferGroup == pDest )
        {
            BLST_S_REMOVE(&pSource->outputList, pDest, BAPE_BufferGroup, node);
            pDest->pSource = NULL;

            /* reset the read pointer offset */
            pDest->localReadOffset = 0;
        }
    }
}

/***************************************************************************
Summary:
Buffer Group - remove all linked output buffer groups
***************************************************************************/
void BAPE_BufferGroup_UnLinkAllOutputs(
    BAPE_BufferGroupHandle pSource
    )
{
    BAPE_BufferGroupHandle pBufferGroup = NULL;

    BDBG_OBJECT_ASSERT(pSource, BAPE_BufferGroup);

    for ( pBufferGroup = BLST_S_FIRST(&pSource->outputList);
        pBufferGroup != NULL;
        pBufferGroup = BLST_S_NEXT(pBufferGroup, node) ) {
        BLST_S_REMOVE(&pSource->outputList, pBufferGroup, BAPE_BufferGroup, node);
        pBufferGroup->pSource = NULL;
    }

    BDBG_ASSERT(BLST_S_EMPTY(&pSource->outputList));
}

static BERR_Code BAPE_BufferGroup_P_ControlLock_isrsafe(BAPE_BufferGroupHandle handle)
{
    BERR_Code errCode;
    unsigned i;

    BDBG_MODULE_MSG(bape_buffer_lock, ("LOCK   BUF GRP %p", (void*)handle));

    for ( i = 0; i < handle->numChannelPairs*2; i++ )
    {
        if ( handle->pBufferHandle[i] )
        {
            errCode = BAPE_Buffer_P_ControlLock_isrsafe(handle->pBufferHandle[i]);
            if ( errCode != BERR_SUCCESS )
            {
                BDBG_ERR(("Error, unable to get control lock for buffer interface[%d]", (int)i));
                return BERR_TRACE(errCode);
            }
        }
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_BufferGroup_P_ControlUnlock_isrsafe(BAPE_BufferGroupHandle handle)
{
    BERR_Code errCode;
    unsigned i;

    BDBG_MODULE_MSG(bape_buffer_lock, ("UNLOCK BUF GRP %p", (void*)handle));
    for ( i = 0; i < handle->numChannelPairs*2; i++ )
    {
        if ( handle->pBufferHandle[i] )
        {
            errCode = BAPE_Buffer_P_ControlUnlock_isrsafe(handle->pBufferHandle[i]);
            if ( errCode != BERR_SUCCESS )
            {
                BDBG_ERR(("Error, unable to release control lock for buffer interface[%d]", (int)i));
                return BERR_TRACE(errCode);
            }
        }
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_BufferGroup_P_FillOutputs_isr(BAPE_BufferGroupHandle pSource)
{
    unsigned freeSpace = INVALID_SIZE;
    unsigned bytesToComplete = INVALID_SIZE;
    BAPE_BufferGroupHandle pBufferGroup = NULL;

    BDBG_MODULE_MSG(bape_buffer_detail, ("Fill output buffers"));
    for ( pBufferGroup = BLST_S_FIRST(&pSource->outputList);
        pBufferGroup != NULL;
        pBufferGroup = BLST_S_NEXT(pBufferGroup, node) ) {
        if ( pBufferGroup->buffered )
        {
            freeSpace = BAPE_MIN(freeSpace, BAPE_BufferGroup_GetBufferFree_isr(pBufferGroup));
        }
    }

    BDBG_MODULE_MSG(bape_buffer_detail, ("  freeSpace %d", (int)freeSpace));

    /* freeSpace will be set to INVALID_SIZE if we have no outputs or
       we have only linked dummy output buffer groups */
    if ( freeSpace > 0 && freeSpace != INVALID_SIZE )
    {
        BERR_Code errCode;
        BAPE_BufferDescriptor srcDescriptor;
        unsigned bytesToWrite;

        errCode = BAPE_BufferGroup_Read_isr(pSource, &srcDescriptor);
        if ( errCode != BERR_SUCCESS )
        {
            BDBG_ERR(("Error getting source buffers"));
            return BERR_TRACE(errCode);
        }

        BDBG_MODULE_MSG(bape_buffer_detail, ("  source bytes avail %d", (int)srcDescriptor.bufferSize+srcDescriptor.wrapBufferSize));

        bytesToWrite = BAPE_MIN(srcDescriptor.bufferSize+srcDescriptor.wrapBufferSize, freeSpace);

        BDBG_MODULE_MSG(bape_buffer_detail, ("  bytesToWrite %d", (int)bytesToWrite));

        if ( bytesToWrite > 0 )
        {
            for ( pBufferGroup = BLST_S_FIRST(&pSource->outputList);
                pBufferGroup != NULL;
                pBufferGroup = BLST_S_NEXT(pBufferGroup, node) ) {
                BAPE_BufferDescriptor destDescriptor;
                unsigned destBytesToWrite = bytesToWrite;
                unsigned written = 0;
                unsigned offset = pBufferGroup->localReadOffset;

                /* check local read offset */
                if ( destBytesToWrite > pBufferGroup->localReadOffset && pBufferGroup->buffered )
                {
                    destBytesToWrite -= pBufferGroup->localReadOffset;
                }
                else
                {
                    /* buffer already up to date */
                    destBytesToWrite = 0;
                }

                while ( destBytesToWrite != 0 )
                {
                    unsigned i, writeSize;
                    errCode = BAPE_BufferGroup_GetWriteBuffers_isr(pBufferGroup, &destDescriptor);
                    if ( errCode != BERR_SUCCESS )
                    {
                        BDBG_ERR(("Error getting destination buffers"));
                        return BERR_TRACE(errCode);
                    }
                    if ( (destDescriptor.bufferSize+destDescriptor.wrapBufferSize) < destBytesToWrite )
                    {
                        BDBG_ERR(("Free space does not match what was reported above."));
                        BDBG_ERR(("  Buffer reports %d, expected %d", destDescriptor.bufferSize+destDescriptor.wrapBufferSize, destBytesToWrite));
                    }
                    BDBG_ASSERT((destDescriptor.bufferSize+destDescriptor.wrapBufferSize) >= destBytesToWrite);
                    BDBG_ASSERT(destDescriptor.numBuffers == srcDescriptor.numBuffers);

                    writeSize = BAPE_MIN(destBytesToWrite, destDescriptor.bufferSize);
                    if ( (written+offset) < srcDescriptor.bufferSize )
                    {
                        writeSize = BAPE_MIN(writeSize, srcDescriptor.bufferSize - (written+offset));
                    }
                    else
                    {
                        BDBG_ASSERT(writeSize <= ((srcDescriptor.wrapBufferSize+srcDescriptor.bufferSize) - (written+offset)));
                    }

                    for ( i = 0; i < destDescriptor.numBuffers; i++ )
                    {
                        BDBG_ASSERT(destDescriptor.buffers[i].pBuffer != NULL);
                        if ( (written+offset) < srcDescriptor.bufferSize )
                        {
                            BDBG_ASSERT(srcDescriptor.buffers[i].pBuffer != NULL);
                            BDBG_ASSERT((written+offset+writeSize) <= srcDescriptor.bufferSize);
                            BKNI_Memcpy(destDescriptor.buffers[i].pBuffer, (uint8_t*)srcDescriptor.buffers[i].pBuffer+written+offset, writeSize);
                        }
                        else
                        {
                            BDBG_ASSERT(srcDescriptor.buffers[i].pWrapBuffer != NULL);
                            BDBG_ASSERT((written+offset-srcDescriptor.bufferSize) <= srcDescriptor.wrapBufferSize);
                            BKNI_Memcpy(destDescriptor.buffers[i].pBuffer, (uint8_t*)srcDescriptor.buffers[i].pWrapBuffer+(written+offset-srcDescriptor.bufferSize), writeSize);
                        }
                    }

                    written += writeSize;

                    errCode = BAPE_BufferGroup_WriteComplete_isr(pBufferGroup, writeSize);
                    if ( errCode != BERR_SUCCESS )
                    {
                        BDBG_ERR(("Error completing write to destination buffers"));
                        return BERR_TRACE(errCode);
                    }
                    destBytesToWrite -= writeSize;
                }

                pBufferGroup->localReadOffset += written;

                if ( written > 0 )
                {
                    BAPE_BufferGroup_P_DataReady_isr(pBufferGroup, written);
                }
            }
        }
    }

    /* look to see how much we can read complete */
    for ( pBufferGroup = BLST_S_FIRST(&pSource->outputList);
        pBufferGroup != NULL;
        pBufferGroup = BLST_S_NEXT(pBufferGroup, node) ) {
        bytesToComplete = BAPE_MIN(bytesToComplete, pBufferGroup->localReadOffset);
    }

    if ( bytesToComplete != INVALID_SIZE && bytesToComplete > 0 )
    {
        BERR_Code errCode;
        errCode = BAPE_BufferGroup_ReadComplete_isr(pSource, bytesToComplete);
        if ( errCode != BERR_SUCCESS )
        {
            BDBG_ERR(("Error completing read of source buffers"));
            return BERR_TRACE(errCode);
        }

        for ( pBufferGroup = BLST_S_FIRST(&pSource->outputList);
            pBufferGroup != NULL;
            pBufferGroup = BLST_S_NEXT(pBufferGroup, node) ) {
            BDBG_ASSERT(pBufferGroup->localReadOffset >= bytesToComplete);
            pBufferGroup->localReadOffset -= bytesToComplete;
        }
    }

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Buffer Group Read - non destructive
***************************************************************************/
BERR_Code BAPE_BufferGroup_Read(
    BAPE_BufferGroupHandle handle,
    BAPE_BufferDescriptor * pGroupDescriptor /* [out] */
    )
{
    BERR_Code errCode;

    BKNI_EnterCriticalSection();
    errCode = BERR_TRACE(BAPE_BufferGroup_Read_isr(handle, pGroupDescriptor));
    BKNI_LeaveCriticalSection();

    return errCode;
}

/***************************************************************************
Summary:
Buffer Group Read - non destructive
***************************************************************************/
BERR_Code BAPE_BufferGroup_Read_isr(
    BAPE_BufferGroupHandle handle,
    BAPE_BufferDescriptor * pGroupDescriptor /* [out] */
    )
{
    BAPE_BufferGroupStatus bgStatus;
    BERR_Code errCode;
    unsigned i;
    unsigned bufferSize = (unsigned)INVALID_SIZE;
    unsigned wrapBufferSize = (unsigned)INVALID_SIZE;

    BDBG_OBJECT_ASSERT(handle, BAPE_BufferGroup);

    if ( pGroupDescriptor == NULL )
    {
        BDBG_ERR(("pGroupDescriptor cannot be NULL"));
        BERR_TRACE(BERR_INVALID_PARAMETER);
        return 0;
    }

    BKNI_Memset_isr(pGroupDescriptor, 0, sizeof(BAPE_BufferDescriptor));

    if ( handle->type == BAPE_BufferType_eWriteOnly )
    {
        BDBG_WRN(("Attempt to read from a WriteOnly Buffer"));
        return BERR_TRACE(BERR_UNKNOWN);
    }

    /* We are a linked buffer. Ask for data */
    if ( handle->pSource )
    {
        errCode = BAPE_BufferGroup_P_FillOutputs_isr(handle->pSource);
        if ( errCode )
        {
            BDBG_ERR(("Error filling output buffers"));
            return BERR_TRACE(errCode);
        }
    }

    if ( handle->buffered )
    {
        /* Lock all the buffers in the group */
        errCode = BAPE_BufferGroup_P_ControlLock_isrsafe(handle);
        if ( errCode != BERR_SUCCESS )
        {
            BDBG_ERR(("Error, unable to get control lock for buffer group"));
            return BERR_TRACE(BERR_UNKNOWN);
        }

        for ( i = 0; i < handle->numChannelPairs * 2; i++ )
        {
            BAPE_SimpleBufferDescriptor descriptor;
            if ( handle->pBufferHandle[i] )
            {
                if ( BAPE_Buffer_Read_isr(handle->pBufferHandle[i], &descriptor) == 0 )
                {
                    /* no data returned from at least one of the buffers*/
                    bufferSize = wrapBufferSize = 0;
                    break;
                }
                pGroupDescriptor->buffers[i].block = descriptor.block;
                pGroupDescriptor->buffers[i].pBuffer = descriptor.pBuffer;
                pGroupDescriptor->buffers[i].offset = descriptor.offset;
                pGroupDescriptor->buffers[i].pWrapBuffer = descriptor.pWrapBuffer;
                pGroupDescriptor->buffers[i].wrapOffset = descriptor.wrapOffset;
                bufferSize = BAPE_MIN(bufferSize, descriptor.bufferSize);
                wrapBufferSize = BAPE_MIN(wrapBufferSize, descriptor.wrapBufferSize);
            }
        }

        /* Unlock all the buffers in the group */
        errCode = BAPE_BufferGroup_P_ControlUnlock_isrsafe(handle);
        if ( errCode != BERR_SUCCESS )
        {
            BDBG_ERR(("Error, unable to get control lock for buffer group"));
            return BERR_TRACE(BERR_UNKNOWN);
        }
    }
    else if ( handle->pSource ) /* use the source group's buffers to read directly */
    {
        BAPE_BufferDescriptor srcDescriptor;
        unsigned offset = handle->localReadOffset;

        errCode = BAPE_BufferGroup_Read_isr(handle->pSource, &srcDescriptor);
        if ( errCode != BERR_SUCCESS )
        {
            BDBG_ERR(("Error, unable to from source buffer group"));
            return BERR_TRACE(BERR_UNKNOWN);
        }

        if ( (srcDescriptor.bufferSize+srcDescriptor.wrapBufferSize) > 0 &&
             offset < (srcDescriptor.bufferSize+srcDescriptor.wrapBufferSize) )
        {
            if ( offset < srcDescriptor.bufferSize )
            {
                bufferSize = srcDescriptor.bufferSize - offset;
                wrapBufferSize = srcDescriptor.wrapBufferSize;
            }
            else
            {
                bufferSize = srcDescriptor.wrapBufferSize - (offset-srcDescriptor.bufferSize);
                /* no wrap buffer */
            }

            for ( i = 0; i < handle->numChannelPairs * 2; i++ )
            {
                if ( srcDescriptor.buffers[i].pBuffer )
                {
                    pGroupDescriptor->buffers[i].block = srcDescriptor.buffers[i].block;
                    /* we can provide some data */
                    if ( offset < srcDescriptor.bufferSize )
                    {
                        pGroupDescriptor->buffers[i].pBuffer = (uint8_t*)srcDescriptor.buffers[i].pBuffer+offset;
                        pGroupDescriptor->buffers[i].offset = srcDescriptor.buffers[i].offset+offset;
                        pGroupDescriptor->buffers[i].pWrapBuffer = srcDescriptor.buffers[i].pWrapBuffer;
                        pGroupDescriptor->buffers[i].wrapOffset = srcDescriptor.buffers[i].wrapOffset;
                    }
                    else
                    {
                        pGroupDescriptor->buffers[i].pBuffer = (uint8_t*)srcDescriptor.buffers[i].pWrapBuffer+(offset-srcDescriptor.bufferSize);
                        pGroupDescriptor->buffers[i].offset = srcDescriptor.buffers[i].wrapOffset+(offset-srcDescriptor.bufferSize);
                        /* no wrap buffer */
                    }
                }
            }
        }
    }


    /* finalize the descriptor */
    if ( bufferSize != INVALID_SIZE )
    {
        pGroupDescriptor->bufferSize = bufferSize;
    }
    if ( wrapBufferSize != INVALID_SIZE )
    {
        pGroupDescriptor->wrapBufferSize = wrapBufferSize;
    }
    pGroupDescriptor->interleaved = handle->interleaved;
    pGroupDescriptor->numBuffers = handle->numChannelPairs;

    BAPE_BufferGroup_GetStatus_isrsafe(handle, &bgStatus);
    pGroupDescriptor->bitsPerSample = bgStatus.bitsPerSample;
    pGroupDescriptor->samplesPerDword = bgStatus.samplesPerDword;
    pGroupDescriptor->compressed = bgStatus.compressed;

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Buffer Group Read Complete
***************************************************************************/
BERR_Code BAPE_BufferGroup_ReadComplete(
    BAPE_BufferGroupHandle handle,
    size_t size
    )
{
    BERR_Code errCode;

    BKNI_EnterCriticalSection();
    errCode = BERR_TRACE(BAPE_BufferGroup_ReadComplete_isr(handle, size));
    BKNI_LeaveCriticalSection();

    return errCode;
}

/***************************************************************************
Summary:
Buffer Group Read Complete
***************************************************************************/
BERR_Code BAPE_BufferGroup_ReadComplete_isr(
    BAPE_BufferGroupHandle handle,
    size_t size
    )
{
    BERR_Code errCode = BERR_SUCCESS;
    unsigned i;
    unsigned bufferDepth = INVALID_SIZE;

    BDBG_OBJECT_ASSERT(handle, BAPE_BufferGroup);

    if ( size <= 0 )
    {
        return BERR_SUCCESS;
    }

    if ( handle->type == BAPE_BufferType_eWriteOnly )
    {
        BDBG_WRN(("Attempt to read complete a WriteOnly Buffer"));
        return BERR_TRACE(BERR_UNKNOWN);
    }

    if ( handle->buffered )
    {
        /* Lock all the buffers in the group */
        errCode = BAPE_BufferGroup_P_ControlLock_isrsafe(handle);
        if ( errCode != BERR_SUCCESS )
        {
            BDBG_ERR(("Error, unable to get control lock for buffer group"));
            return BERR_TRACE(BERR_UNKNOWN);
        }

        for ( i = 0; i < handle->numChannelPairs*2; i++ )
        {
            BAPE_SimpleBufferDescriptor descriptor;
            if ( handle->pBufferHandle[i] )
            {
                if ( BAPE_Buffer_Read_isr(handle->pBufferHandle[i], &descriptor) == 0 )
                {
                    /* no data returned from at least one of the buffers*/
                    bufferDepth = 0;
                    break;
                }
                bufferDepth = BAPE_MIN(bufferDepth, descriptor.bufferSize + descriptor.wrapBufferSize);
            }
        }

        if ( size > bufferDepth )
        {
            BDBG_ERR(("Attempt to read complete %d bytes, but only %d bytes in the buffer", (int)size, (int)(bufferDepth)));
            errCode = BERR_TRACE(BERR_UNKNOWN);
        }
        else
        {
            for ( i = 0; i < handle->numChannelPairs*2; i++ )
            {
                if ( handle->pBufferHandle[i] )
                {
                    BDBG_ASSERT( BAPE_Buffer_ReadComplete_isr(handle->pBufferHandle[i], size) == size );
                }
            }
        }

        /* Unlock all the buffers in the group */
        if ( BAPE_BufferGroup_P_ControlUnlock_isrsafe(handle) != BERR_SUCCESS )
        {
            BDBG_ERR(("Error, unable to get control lock for buffer group"));
            return BERR_TRACE(BERR_UNKNOWN);
        }
    }
    else
    {
        BDBG_ASSERT(handle->pSource);

        handle->localReadOffset += size;

        errCode = BERR_TRACE(BAPE_BufferGroup_P_FillOutputs_isr(handle->pSource));
    }

    if ( size > 0 && errCode == BERR_SUCCESS )
    {
        BAPE_BufferGroup_P_FreeSpaceAvailable_isr(handle, size);
    }

    return errCode;
}

/***************************************************************************
Summary:
Get Buffer Group descriptors for writing - non destructive
***************************************************************************/
BERR_Code BAPE_BufferGroup_GetWriteBuffers(
    BAPE_BufferGroupHandle handle,
    BAPE_BufferDescriptor * pGroupDescriptor /* [out] */
    )
{
    BERR_Code errCode;

    BKNI_EnterCriticalSection();
    errCode = BERR_TRACE(BAPE_BufferGroup_GetWriteBuffers_isr(handle, pGroupDescriptor));
    BKNI_LeaveCriticalSection();

    return errCode;
}

/***************************************************************************
Summary:
Get Buffer Group descriptors for writing - non destructive
***************************************************************************/
BERR_Code BAPE_BufferGroup_GetWriteBuffers_isr(
    BAPE_BufferGroupHandle handle,
    BAPE_BufferDescriptor * pGroupDescriptor /* [out] */
    )
{
    BERR_Code errCode;
    unsigned i;
    unsigned bufferSize = (unsigned)INVALID_SIZE;
    unsigned wrapBufferSize = (unsigned)INVALID_SIZE;

    BDBG_OBJECT_ASSERT(handle, BAPE_BufferGroup);

    if ( pGroupDescriptor == NULL )
    {
        BDBG_ERR(("pGroupDescriptor cannot be NULL"));
        BERR_TRACE(BERR_INVALID_PARAMETER);
        return 0;
    }

    BKNI_Memset(pGroupDescriptor, 0, sizeof(BAPE_BufferDescriptor));

    if ( handle->type == BAPE_BufferType_eReadOnly )
    {
        BDBG_WRN(("Attempt to get write buffers from a ReadOnly Buffer"));
        return BERR_TRACE(BERR_UNKNOWN);
    }

    if ( handle->buffered )
    {
        /* Lock all the buffers in the group */
        errCode = BAPE_BufferGroup_P_ControlLock_isrsafe(handle);
        if ( errCode != BERR_SUCCESS )
        {
            BDBG_ERR(("Error, unable to get control lock for buffer group"));
            return BERR_TRACE(BERR_UNKNOWN);
        }

        for ( i = 0; i < handle->numChannelPairs*2; i++ )
        {
            BAPE_SimpleBufferDescriptor descriptor;
            if ( handle->pBufferHandle[i] )
            {
                if ( BAPE_Buffer_GetWriteBuffer_isr(handle->pBufferHandle[i], &descriptor) != BERR_SUCCESS )
                {
                    /* no data returned from at least one of the buffers*/
                    bufferSize = wrapBufferSize = 0;
                    break;
                }
                pGroupDescriptor->buffers[i].block = descriptor.block;
                pGroupDescriptor->buffers[i].pBuffer = descriptor.pBuffer;
                pGroupDescriptor->buffers[i].offset = descriptor.offset;
                pGroupDescriptor->buffers[i].pWrapBuffer = descriptor.pWrapBuffer;
                pGroupDescriptor->buffers[i].wrapOffset = descriptor.wrapOffset;
                bufferSize = BAPE_MIN(bufferSize, descriptor.bufferSize);
                wrapBufferSize = BAPE_MIN(wrapBufferSize, descriptor.wrapBufferSize);
            }
        }

        /* Unlock all the buffers in the group */
        errCode = BAPE_BufferGroup_P_ControlUnlock_isrsafe(handle);
        if ( errCode != BERR_SUCCESS )
        {
            BDBG_ERR(("Error, unable to get control lock for buffer group"));
            return BERR_TRACE(BERR_UNKNOWN);
        }
    }
    else
    {
        BDBG_WRN(("Attempt to get write buffers from a linked dummy output buffer group."));
    }

    /* finalize the descriptor */
    if ( bufferSize != INVALID_SIZE )
    {
        pGroupDescriptor->bufferSize = bufferSize;
    }
    if ( wrapBufferSize != INVALID_SIZE )
    {
        pGroupDescriptor->wrapBufferSize = wrapBufferSize;
    }
    pGroupDescriptor->interleaved = handle->interleaved;
    pGroupDescriptor->numBuffers = handle->numChannelPairs;

    return BERR_SUCCESS;
}

static void BAPE_BufferGroup_P_DataReady_isr(
    BAPE_BufferGroupHandle handle,
    unsigned size
    )
{
    BAPE_BufferGroupHandle pBufferGroup;

    BDBG_OBJECT_ASSERT(handle, BAPE_BufferGroup);

    /* pass through to linked outputs */
    for ( pBufferGroup = BLST_S_FIRST(&handle->outputList);
        pBufferGroup != NULL;
        pBufferGroup = BLST_S_NEXT(pBufferGroup, node) ) {

        BAPE_BufferGroup_P_DataReady_isr(pBufferGroup, size);
    }
    /* notify our direct consumer, if any exists */
    if ( handle->interrupts.dataReady.pCallback_isr )
    {
        handle->interrupts.dataReady.pCallback_isr(handle->interrupts.dataReady.pParam1, handle->interrupts.dataReady.param2);
    }
}

static void BAPE_BufferGroup_P_FreeSpaceAvailable_isr(
    BAPE_BufferGroupHandle handle,
    unsigned size
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_BufferGroup);

    BSTD_UNUSED(size);

    #if 0 /* this is done by ReadComplete / FillOutputs */
    /* pass upstream to parent buffer group */
    if ( handle->pSource )
    {
        BAPE_BufferGroup_P_FreeSpaceAvailable_isr(handle->pSource, size);
    }
    #endif

    /* notify our direct consumer, if any exists */
    if ( handle->interrupts.freeAvailable.pCallback_isr )
    {
        handle->interrupts.freeAvailable.pCallback_isr(handle->interrupts.freeAvailable.pParam1, handle->interrupts.freeAvailable.param2);
    }
}

/***************************************************************************
Summary:
Buffer Group Write
***************************************************************************/
BERR_Code BAPE_BufferGroup_WriteComplete(
    BAPE_BufferGroupHandle handle,
    size_t size
    )
{
    BERR_Code errCode;

    BKNI_EnterCriticalSection();
    errCode = BERR_TRACE(BAPE_BufferGroup_WriteComplete_isr(handle, size));
    BKNI_LeaveCriticalSection();

    return errCode;
}

/***************************************************************************
Summary:
Buffer Group Write
***************************************************************************/
BERR_Code BAPE_BufferGroup_WriteComplete_isr(
    BAPE_BufferGroupHandle handle,
    size_t size
    )
{
    BERR_Code errCode;
    unsigned i;
    unsigned bufferSize = (unsigned)INVALID_SIZE;
    unsigned wrapBufferSize = (unsigned)INVALID_SIZE;

    BDBG_OBJECT_ASSERT(handle, BAPE_BufferGroup);

    if ( size <= 0 )
    {
        return 0;
    }

    if ( handle->type == BAPE_BufferType_eReadOnly )
    {
        BDBG_WRN(("Attempt to write complete a ReadOnly Buffer"));
        return BERR_TRACE(BERR_UNKNOWN);
    }

    if ( handle->buffered)
    {
        /* Lock all the buffers in the group */
        errCode = BAPE_BufferGroup_P_ControlLock_isrsafe(handle);
        if ( errCode != BERR_SUCCESS )
        {
            BDBG_ERR(("Error, unable to get control lock for buffer group"));
            return BERR_TRACE(BERR_UNKNOWN);
        }

        for ( i = 0; i < handle->numChannelPairs*2; i++ )
        {
            BAPE_SimpleBufferDescriptor descriptor;
            if ( handle->pBufferHandle[i] )
            {
                if ( BAPE_Buffer_GetWriteBuffer_isr(handle->pBufferHandle[i], &descriptor) != BERR_SUCCESS )
                {
                    /* no data returned from at least one of the buffers*/
                    bufferSize = wrapBufferSize = 0;
                    break;
                }
                bufferSize = BAPE_MIN(bufferSize, descriptor.bufferSize);
                wrapBufferSize = BAPE_MIN(wrapBufferSize, descriptor.wrapBufferSize);
            }
        }

        bufferSize = (bufferSize==INVALID_SIZE)? 0 : bufferSize;
        wrapBufferSize = (wrapBufferSize==INVALID_SIZE)? 0 : wrapBufferSize;

        if ( wrapBufferSize > 0 && bufferSize == 0 )
        {
            BDBG_ERR(("Error, data in wrapBuffer (%u) but not in base buffer.", wrapBufferSize));
            BDBG_ASSERT(wrapBufferSize > 0 && bufferSize == 0);
        }

        if ( size > (bufferSize + wrapBufferSize) )
        {
            BDBG_ERR(("Attempt to write complete %d bytes, but only %d bytes in the buffer", (int)size, (int)(bufferSize+wrapBufferSize)));
            return BERR_TRACE(BERR_UNKNOWN);
        }

        for ( i = 0; i < handle->numChannelPairs*2; i++ )
        {
            if ( handle->pBufferHandle[i] )
            {
                BDBG_ASSERT( BAPE_Buffer_WriteComplete_isr(handle->pBufferHandle[i], size) == size );
            }
        }

        /* Unlock all the buffers in the group */
        errCode = BAPE_BufferGroup_P_ControlUnlock_isrsafe(handle);
        if ( errCode != BERR_SUCCESS )
        {
            BDBG_ERR(("Error, unable to get control lock for buffer group"));
            return BERR_TRACE(BERR_UNKNOWN);
        }

        /* if we have outputs, push data to them */
        if ( !BLST_S_EMPTY(&handle->outputList) )
        {
            /* BDBG_MSG(("Fill output buffer consumers")); */
            BERR_TRACE(BAPE_BufferGroup_P_FillOutputs_isr(handle));
        }

        if ( size > 0 )
        {
            BAPE_BufferGroup_P_DataReady_isr(handle, size);
        }
    }
    else
    {
        BDBG_ERR(("ERROR, cannot write complete a linked dummy output buffer group."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Buffer Group Flush
***************************************************************************/
void BAPE_BufferGroup_Flush(
    BAPE_BufferGroupHandle handle
    )
{
    BKNI_EnterCriticalSection();
    BAPE_BufferGroup_Flush_isr(handle);
    BKNI_LeaveCriticalSection();
}

/***************************************************************************
Summary:
Buffer Group Flush
***************************************************************************/
void BAPE_BufferGroup_Flush_isr(
    BAPE_BufferGroupHandle handle
    )
{
    BAPE_BufferGroupHandle pBufferGroup;
    BERR_Code errCode;
    unsigned i;

    BDBG_OBJECT_ASSERT(handle, BAPE_BufferGroup);

    if ( handle->type == BAPE_BufferType_eReadOnly )
    {
        BDBG_WRN(("Attempt to flush a ReadOnly Buffer (only producer should flush)"));
        BERR_TRACE(BERR_UNKNOWN);
        return;
    }

    if ( handle->buffered )
    {
        /* Lock all the buffers in the group */
        errCode = BAPE_BufferGroup_P_ControlLock_isrsafe(handle);
        if ( errCode != BERR_SUCCESS )
        {
            BDBG_ERR(("Error, unable to get control lock for buffer group"));
            BERR_TRACE(BERR_UNKNOWN);
            return;
        }

        for ( i = 0; i < handle->numChannelPairs*2; i++ )
        {
            if ( handle->pBufferHandle[i] )
            {
                BAPE_Buffer_Flush_isr(handle->pBufferHandle[i]);
            }
        }

        /* Unlock all the buffers in the group */
        errCode = BAPE_BufferGroup_P_ControlUnlock_isrsafe(handle);
        if ( errCode != BERR_SUCCESS )
        {
            BDBG_ERR(("Error, unable to get control lock for buffer group"));
            BERR_TRACE(BERR_UNKNOWN);
            return;
        }

        BDBG_MSG(("Flush linked output buffer groups"));
        for ( pBufferGroup = BLST_S_FIRST(&handle->outputList);
            pBufferGroup != NULL;
            pBufferGroup = BLST_S_NEXT(pBufferGroup, node) ) {
            BAPE_BufferGroup_Flush_isr(pBufferGroup);
        }
    }
    else
    {
        handle->localReadOffset = 0;
    }
}

/***************************************************************************
Summary:
Buffer Group Get Depth
***************************************************************************/
unsigned BAPE_BufferGroup_GetBufferDepth(
    BAPE_BufferGroupHandle handle
    )
{
    unsigned depth;

    BKNI_EnterCriticalSection();
    depth = BAPE_BufferGroup_GetBufferDepth_isr(handle);
    BKNI_LeaveCriticalSection();

    return depth;
}

/***************************************************************************
Summary:
Buffer Group Get Depth
***************************************************************************/
unsigned BAPE_BufferGroup_GetBufferDepth_isr(
    BAPE_BufferGroupHandle handle
    )
{
    unsigned i;
    unsigned bufferDepth = INVALID_SIZE;

    BDBG_OBJECT_ASSERT(handle, BAPE_BufferGroup);

    if ( handle->buffered )
    {
        for ( i = 0; i < handle->numChannelPairs * 2; i++ )
        {
            if ( handle->pBufferHandle[i] )
            {
                bufferDepth = BAPE_MIN(bufferDepth, BAPE_Buffer_GetBufferDepth_isr(handle->pBufferHandle[i]));
            }
        }
    }
    else if ( handle->pSource )
    {
        unsigned srcBufferDepth = BAPE_BufferGroup_GetBufferDepth_isr(handle->pSource);

        if ( handle->localReadOffset <= srcBufferDepth )
        {
            bufferDepth = srcBufferDepth - handle->localReadOffset;
        }
        else
        {
            BDBG_ERR(("Invalid state. localReadOffset > srcBufferDepth"));
        }
    }

    return (bufferDepth==INVALID_SIZE) ? 0 : bufferDepth;
}

/***************************************************************************
Summary:
Buffer Group Get Free Space
***************************************************************************/
unsigned BAPE_BufferGroup_GetBufferFree(
    BAPE_BufferGroupHandle handle
    )
{
    unsigned free;

    BKNI_EnterCriticalSection();
    free = BAPE_BufferGroup_GetBufferFree_isr(handle);
    BKNI_LeaveCriticalSection();

    return free;
}

/***************************************************************************
Summary:
Buffer Group Get Free Space
***************************************************************************/
unsigned BAPE_BufferGroup_GetBufferFree_isr(
    BAPE_BufferGroupHandle handle
    )
{
    unsigned i;
    unsigned freeSpace = INVALID_SIZE;

    BDBG_OBJECT_ASSERT(handle, BAPE_BufferGroup);

    if ( handle->buffered )
    {
        for ( i = 0; i < handle->numChannelPairs * 2; i++ )
        {
            if ( handle->pBufferHandle[i] )
            {
                freeSpace = BAPE_MIN(freeSpace, BAPE_Buffer_GetBufferFree_isr(handle->pBufferHandle[i]));
            }
        }
    }
    else if ( handle->pSource )
    {
        freeSpace = BAPE_BufferGroup_GetBufferFree_isr(handle->pSource);
    }

    return (freeSpace==INVALID_SIZE) ? 0 : freeSpace;
}

/***************************************************************************
Summary:
Buffer Group Enable Consumption
***************************************************************************/
BERR_Code BAPE_BufferGroup_Enable(
    BAPE_BufferGroupHandle handle,
    bool enable
    )
{
    BERR_Code errCode;

    BKNI_EnterCriticalSection();
    errCode = BERR_TRACE(BAPE_BufferGroup_Enable_isr(handle, enable));
    BKNI_LeaveCriticalSection();

    return errCode;
}

/***************************************************************************
Summary:
Buffer Group Enable Consumption
***************************************************************************/
BERR_Code BAPE_BufferGroup_Enable_isr(
    BAPE_BufferGroupHandle handle,
    bool enable
    )
{
    BERR_Code errCode;
    unsigned i;

    BDBG_OBJECT_ASSERT(handle, BAPE_BufferGroup);

    if ( handle->buffered )
    {
        /* Lock all the buffers in the group */
        errCode = BAPE_BufferGroup_P_ControlLock_isrsafe(handle);
        if ( errCode != BERR_SUCCESS )
        {
            BDBG_ERR(("Error, unable to get control lock for buffer group"));
            return BERR_TRACE(BERR_UNKNOWN);
        }

        if ( BAPE_Buffer_IsEnabled_isrsafe(handle->pBufferHandle[0]) != enable )
        {
            for ( i = 0; i < handle->numChannelPairs*2; i++ )
            {
                if ( handle->pBufferHandle[i] )
                {
                    BAPE_Buffer_Enable_isr(handle->pBufferHandle[i], enable);
                }
            }
        }

        /* Unlock all the buffers in the group */
        errCode = BAPE_BufferGroup_P_ControlUnlock_isrsafe(handle);
        if ( errCode != BERR_SUCCESS )
        {
            BDBG_ERR(("Error, unable to get control lock for buffer group"));
            return BERR_TRACE(BERR_UNKNOWN);
        }
    }

    handle->enabled = enable;

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Buffer Group Get Status
***************************************************************************/
void BAPE_BufferGroup_GetStatus_isrsafe(
    BAPE_BufferGroupHandle handle,
    BAPE_BufferGroupStatus * pStatus /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_BufferGroup);

    BDBG_ASSERT(pStatus != NULL);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    pStatus->type = handle->type;
    pStatus->enabled = handle->enabled;

    if ( handle->buffered )
    {
        BERR_Code errCode;
        BAPE_BufferFormat bufferFormat;

        pStatus->bufferSize = handle->bufferSize;
        errCode = BERR_TRACE(BAPE_Buffer_GetFormat_isrsafe(handle->pBufferHandle[0], &bufferFormat));
        if ( errCode )
        {
            BDBG_WRN(("Unable to get buffer format"));
            return;
        }
        pStatus->interleaved = bufferFormat.interleaved;
        pStatus->compressed = bufferFormat.compressed;
        pStatus->bitsPerSample = bufferFormat.bitsPerSample;
        pStatus->samplesPerDword = bufferFormat.samplesPerDword;
        pStatus->numChannels = handle->numChannelPairs * (bufferFormat.interleaved ? 2 : 1);
    }
    else if ( handle->pSource )
    {
        BAPE_BufferGroupStatus sourceStatus;
        BAPE_BufferGroup_GetStatus_isrsafe(handle->pSource, &sourceStatus);
        pStatus->bufferSize = sourceStatus.bufferSize;
        pStatus->interleaved = sourceStatus.interleaved;
        pStatus->compressed = sourceStatus.compressed;
        pStatus->bitsPerSample = sourceStatus.bitsPerSample;
        pStatus->samplesPerDword = sourceStatus.samplesPerDword;
        pStatus->numChannels = sourceStatus.numChannels;
    }
}

/***************************************************************************
Summary:
Buffer Group Set Format
***************************************************************************/
BERR_Code BAPE_BufferGroup_SetFormat(
    BAPE_BufferGroupHandle handle,
    BAPE_BufferGroupFormat * pFormat
    )
{
    BERR_Code errCode;

    BKNI_EnterCriticalSection();
    errCode = BERR_TRACE(BAPE_BufferGroup_SetFormat_isr(handle, pFormat));
    BKNI_LeaveCriticalSection();

    return errCode;
}

/***************************************************************************
Summary:
Buffer Group Set Format
***************************************************************************/
BERR_Code BAPE_BufferGroup_SetFormat_isr(
    BAPE_BufferGroupHandle handle,
    BAPE_BufferGroupFormat * pFormat
    )
{
    if ( pFormat == NULL )
    {
        BDBG_ERR(("pFormat must not be NULL"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( handle->buffered )
    {
        BERR_Code errCode;

        /* Lock all the buffers in the group */
        errCode = BAPE_BufferGroup_P_ControlLock_isrsafe(handle);
        if ( errCode != BERR_SUCCESS )
        {
            BDBG_ERR(("Error, unable to get control lock for buffer group"));
            return BERR_TRACE(BERR_UNKNOWN);
        }

        if ( !BAPE_Buffer_IsEnabled_isrsafe(handle->pBufferHandle[0]) )
        {
            unsigned i;
            BAPE_BufferFormat bufferFormat;

            bufferFormat.interleaved = handle->interleaved;
            bufferFormat.compressed = pFormat->compressed;
            bufferFormat.bitsPerSample = pFormat->bitsPerSample;
            bufferFormat.samplesPerDword = pFormat->samplesPerDword;
            /* TBD7211 - refine number of channels if we add support for interleaving > 2 channels into one buffer */
            bufferFormat.numChannels = (handle->interleaved ? 2 : 1);

            for ( i = 0; i < handle->numChannelPairs*2; i++ )
            {
                if ( handle->pBufferHandle[i] )
                {
                    errCode = BERR_TRACE(BAPE_Buffer_SetFormat_isr(handle->pBufferHandle[i], &bufferFormat));
                    if ( errCode )
                    {
                        BDBG_ERR(("Unable set format for buffer %u", i));
                        break;
                    }
                }
            }
        }
        else
        {
            BDBG_WRN(("Can't set format of a buffer group while it is enabled."));
        }

        /* Unlock all the buffers in the group */
        errCode = BAPE_BufferGroup_P_ControlUnlock_isrsafe(handle);
        if ( errCode != BERR_SUCCESS )
        {
            BDBG_ERR(("Error, unable to get control lock for buffer group"));
            return BERR_TRACE(BERR_UNKNOWN);
        }
    }

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Buffer Group Get Interrupt Handlers
***************************************************************************/
void BAPE_BufferGroup_GetInterruptHandlers(
    BAPE_BufferGroupHandle handle,
    BAPE_BufferGroupInterruptHandlers *pInterrupts    /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_BufferGroup);
    BDBG_ASSERT(NULL != pInterrupts);
    *pInterrupts = handle->interrupts;
}

/***************************************************************************
Summary:
Buffer Group Set Interrupt Handlers
***************************************************************************/
BERR_Code BAPE_BufferGroup_SetInterruptHandlers(
    BAPE_BufferGroupHandle handle,
    const BAPE_BufferGroupInterruptHandlers *pInterrupts
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_BufferGroup);
    BDBG_ASSERT(NULL != pInterrupts);

    /* store new settings */
    handle->interrupts = *pInterrupts;

    return BERR_SUCCESS;
}

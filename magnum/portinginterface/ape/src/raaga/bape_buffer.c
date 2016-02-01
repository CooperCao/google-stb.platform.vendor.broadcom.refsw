/***************************************************************************
 *     Copyright (c) 2006-2013, Broadcom Corporation
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
 * Module Description: APE Buffer Interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bape_buffer.h"

BDBG_MODULE(bape_buffer);

BDBG_OBJECT_ID(BAPE_Buffer);

typedef struct BAPE_Buffer
{
    BDBG_OBJECT(BAPE_Buffer)

    bool userAllocatedBuffer;
    void * buffer;
    size_t size;
    volatile unsigned long read;
    volatile unsigned long write;
    unsigned long end;

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
    const BAPE_BufferSettings * pSettings, 
    BAPE_BufferHandle * pHandle /* [out] */
    )
{
    BAPE_BufferHandle handle;
    void * uncached = NULL;
    void * buffer = NULL;
    BERR_Code err;

    /* check for valid settings */
    if ( NULL == pSettings )
    {
        BDBG_ERR(("Buffer settings null"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_MSG(("%s - userBuffer %p, size %lu", __FUNCTION__, (void *)pSettings->userBuffer, (unsigned long)pSettings->bufferSize));
    if ( NULL == pSettings->userBuffer )
    {
        if ( NULL == pSettings->heap )
        {
            BDBG_ERR(("no heap specificed"));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        BDBG_MSG(("%s - calling BMEM_Heap_Alloc(%p, %lu)", __FUNCTION__, (void *)pSettings->heap, (unsigned long)pSettings->bufferSize));
        uncached = BMEM_Heap_Alloc(pSettings->heap, pSettings->bufferSize);
        if ( NULL == uncached )
        {
            BDBG_ERR(("failed to allocate %lu byte buffer", (unsigned long)pSettings->bufferSize));
            return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        }

        err = BMEM_Heap_ConvertAddressToCached(pSettings->heap, uncached, &buffer);
        if ( err )
        {
            BDBG_ERR(("BMEM_Heap_ConvertAddressToCached returned %lu", (unsigned long)err));
            BMEM_Heap_Free(pSettings->heap, uncached);
            return BERR_TRACE(err);
        }
        BMEM_Heap_FlushCache(pSettings->heap, buffer, pSettings->bufferSize);
    }
    else
    {
        buffer = pSettings->userBuffer;
    }

    /* Allocate the device structure, then fill in all the fields. */
    handle = BKNI_Malloc(sizeof(BAPE_Buffer));
    if ( NULL == handle )
    {
        if ( !pSettings->userBuffer )
        {
            BMEM_Heap_Free(pSettings->heap, uncached);
        }
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    BDBG_MSG(("%s:%d - memset buffer %p size %lu", __FUNCTION__, __LINE__, (void *)buffer, (unsigned long)pSettings->bufferSize));
    BKNI_Memset(buffer, 0, pSettings->bufferSize);
    BKNI_Memset(handle, 0, sizeof(BAPE_Buffer));
    BDBG_OBJECT_SET(handle, BAPE_Buffer);

    handle->buffer = buffer;
    handle->read = handle->write = (unsigned long) handle->buffer;
    handle->size = pSettings->bufferSize;
    handle->end = (unsigned long) handle->buffer + pSettings->bufferSize;
    handle->userAllocatedBuffer = (pSettings->userBuffer != NULL);
    handle->settings = *pSettings;

    BDBG_MSG(("%s - SUCCESS, returning BAPE_BufferHandle %p", __FUNCTION__, (void *)handle));

    *pHandle = handle;
    return BERR_SUCCESS;
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

    BDBG_MSG(("%s - ", __FUNCTION__));
    if ( !handle->userAllocatedBuffer )
    {
        BDBG_ERR(("%s - calling BMEM_Heap_Free", __FUNCTION__));
        BMEM_Heap_FreeCached(handle->settings.heap, handle->buffer);
    }

    handle->buffer = NULL;

    BDBG_OBJECT_DESTROY(handle, BAPE_Buffer);
    BKNI_Free(handle);
    BDBG_ERR(("%s - SUCCESS, returning", __FUNCTION__));
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
    unsigned size = 0;

    BDBG_MSG(("%s - ", __FUNCTION__));
    if ( handle == NULL || pDescriptor == NULL )
    {
        return 0;
    }

    BDBG_OBJECT_ASSERT(handle, BAPE_Buffer);

    BKNI_Memset(pDescriptor, 0, sizeof(BAPE_SimpleBufferDescriptor));

    BDBG_MSG(("READ(%p) - read %lx, write %lx, bufSize %lx", (void *)handle, (unsigned long)handle->read, (unsigned long)handle->write, (unsigned long)handle->size));
    if ( handle->read < handle->write )
    {
        pDescriptor->pBuffer = (void*) handle->read;
        pDescriptor->bufferSize = handle->write - handle->read;
        size = pDescriptor->bufferSize;
    }
    else if ( handle->read > handle->write )
    {
        pDescriptor->pBuffer = (void*) handle->read;
        pDescriptor->bufferSize = (unsigned) (handle->end - handle->read);
        pDescriptor->pWrapBuffer = (void*) handle->buffer;
        pDescriptor->wrapBufferSize = (unsigned) (handle->write - (unsigned long) handle->buffer);
        size = pDescriptor->bufferSize + pDescriptor->wrapBufferSize;
    }

    BDBG_MSG(("\treturning %d bytes", size));
    return size;
}

/***************************************************************************
Summary:
Buffer Write
***************************************************************************/
unsigned BAPE_Buffer_Write_isr(
    BAPE_BufferHandle handle, 
    void * pData,
    unsigned size
    )
{
    unsigned written = 0;
    unsigned currentWriteSize;

    if ( handle == NULL || pData == NULL || size == 0 )
    {
        return 0;
    }

    BDBG_OBJECT_ASSERT(handle, BAPE_Buffer);

    BDBG_MSG(("WRITE(%p) - read %lx, write %lx, bufSize %lx, writeSize %lx", (void *)handle, (unsigned long)handle->read, (unsigned long)handle->write, (unsigned long)handle->size, (unsigned long)size));
    /* write in space between write ptr and end ptr */
    if ( handle->read <= handle->write )
    {
        currentWriteSize = BAPE_MIN(size, handle->end - handle->write);

        BKNI_Memcpy((void*)handle->write, pData, currentWriteSize);
        written += currentWriteSize;

        handle->write += written;
    }

    /* check for read ptr wrap */
    if ( handle->read >= handle->end )
    {
        if ( handle->write < handle->end )
        {
            handle->read = (unsigned long) handle->buffer;
        }
    }

    /* check for write ptr wrap */
    if ( handle->write >= handle->end )
    {
        if ( handle->read != (unsigned long)handle->buffer )
        {
            handle->write = (unsigned long)handle->buffer;
        }
    }

    /* write ptr wrapped - write data between write and read.
       check for at least 1 free byte */
    if ( (size - written) > 0 && handle->read > (handle->write + 1 ) )
    {
        currentWriteSize = BAPE_MIN(size - written, handle->read - handle->write - 1);

        BKNI_Memcpy((void*)handle->write, (unsigned char*)pData + written, currentWriteSize);
        written += currentWriteSize;
    }

    if ( written < size )
    {
        BDBG_WRN(("WRITE(%p) - WARNING - buffer full", (void *)handle));
    }
    return written;
}

/***************************************************************************
Summary:
Buffer Advance
***************************************************************************/
unsigned BAPE_Buffer_Advance_isr(
    BAPE_BufferHandle handle, 
    unsigned size
    )
{
    unsigned long advanced = 0;
    unsigned long curAdvanceSize;

    if ( handle == NULL || size == 0 )
    {
        return 0;
    }

    BDBG_OBJECT_ASSERT(handle, BAPE_Buffer);
    
    BDBG_MSG(("ADVANCE(%p) - read %lx, write %lx, bufSize %lx, AdvSize %lx", (void *)handle, (unsigned long)handle->read, (unsigned long)handle->write, (unsigned long)handle->size, (unsigned long)size));

    /* advance read ptr between read and end of buffer */
    if ( handle->read > handle->write )
    {
        curAdvanceSize = BAPE_MIN(handle->end - handle->read, (unsigned long)size);
        handle->read += curAdvanceSize;
        advanced += curAdvanceSize;

    }

    /* check for write ptr wrap */
    if ( handle->write >= handle->end )
    {
        if ( handle->read != (unsigned long)handle->buffer )
        {
            handle->write = (unsigned long)handle->buffer;
        }
    }

    /* check for read ptr wrap */
    if ( handle->read >= handle->end )
    {
        if ( handle->write < handle->end )
        {
            handle->read = (unsigned long) handle->buffer;
        }
    }

    /* advance read ptr chasing write ptr. 
       read ptr can meet write ptr, indicating buffer empty*/  
    if ( (size - advanced) > 0 && handle->read < ( handle->write + 1 ) )
    {
        curAdvanceSize = BAPE_MIN(handle->write - handle->read, (unsigned long)size - (unsigned long)advanced);
        handle->read += curAdvanceSize;
        advanced += curAdvanceSize;
    }

    if ( advanced < (unsigned long)size )
    {
        BDBG_WRN(("ADVANCE(%p) - WARNING - could only advance %lu of %lu bytes", (void *)handle, (unsigned long)advanced, (unsigned long)size));
    }

    BDBG_MSG(("\tAdvanced %u bytes", size));
    return (unsigned)advanced;
}

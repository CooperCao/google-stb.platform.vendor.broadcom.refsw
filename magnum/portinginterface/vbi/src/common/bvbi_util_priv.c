/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 * Module Description:
 *
 ***************************************************************************/

#include "bvbi_cap.h"
#include "bvbi_util_priv.h"
#include "bvbi_priv.h"
#include "bkni.h"

BDBG_MODULE(BVBI);

/*
 * BVBI/BMMA functions
 */

/***************************************************************************
 *
 */
BERR_Code BVBI_P_MmaAlloc
    (BMMA_Heap_Handle hMmaHeap, size_t size, unsigned alignment,
     BVBI_P_MmaData* pMmaData)
{
    BERR_Code eErr = BERR_SUCCESS;

    pMmaData->handle = BMMA_Alloc (hMmaHeap, size, alignment, NULL);
    if (pMmaData->handle == NULL)
    {
        eErr = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto init_done;
    }
    pMmaData->pSwAccess = BMMA_Lock (pMmaData->handle);
    if (pMmaData->pSwAccess == NULL)
    {
        eErr = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto init_done;
    }
    pMmaData->pHwAccess = BMMA_LockOffset (pMmaData->handle);
    if (pMmaData->pHwAccess == (BMMA_DeviceOffset)0)
    {
        eErr = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto init_done;
    }

init_done:
    if (eErr != BERR_SUCCESS)
    {
        if (pMmaData->handle != NULL)
        {
            if (pMmaData->pSwAccess != NULL)
            {
                BMMA_Unlock (pMmaData->handle, pMmaData->pSwAccess);
                pMmaData->pSwAccess = NULL;
            }
            BMMA_Free (pMmaData->handle);
            pMmaData->handle = NULL;
        }
    }
    return eErr;
}

/***************************************************************************
 *
 */
void BVBI_P_MmaFree (BVBI_P_MmaData* pMmaData)
{
    if (pMmaData->handle != NULL)
    {
        BMMA_UnlockOffset (pMmaData->handle, pMmaData->pHwAccess);
        pMmaData->pHwAccess = (BMMA_DeviceOffset)0;
        BMMA_Unlock (pMmaData->handle, pMmaData->pSwAccess);
        pMmaData->pSwAccess = NULL;
        BMMA_Free (pMmaData->handle);
        pMmaData->handle = NULL;
    }
}

/***************************************************************************
 *
 */
void BVBI_P_MmaFlush_isrsafe (BVBI_P_MmaData* pMmaData, size_t size)
{
    BMMA_FlushCache_isrsafe (pMmaData->handle, pMmaData->pSwAccess, size);
}


/*
 * LineBuilder functions
 */

/* The LineBuilder object implements a double buffer. This struct defines
 properties common to both buffers. */
typedef struct P_Bank
{
    BVBI_P_MmaData rawData; /* The raw data being stored/buffered.          */
    size_t accumLength;     /* How many bytes have been accumulated.        */
    int sequenceNumber;     /* A trigger that causes accumulation to reset.
                               If zero, indicates there is no valid data.   */
    int lineNumber;         /* Another trigger that causes accumulation
                               to reset.                                    */

} P_Bank;

BDBG_OBJECT_ID (BVBI_LBLDR);
BDBG_OBJECT_ID_DECLARE (BVBI_LBLDR);
struct BVBI_P_LineBuilder_Handle
{
    BDBG_OBJECT (BVBI_LBLDR)

    /* Needed to allocate memory */
    BMMA_Heap_Handle hMmaHeap;

    /* Usable line length in bytes, set at creation time. */
    size_t lineCount;

    /* Physical line length in bytes, set at creation time. */
    size_t lineSize;

    /* Which bank is being accumulated */
    int accumBank;

    /* Raw storage, double buffered. */
    P_Bank bank[2];

};

/* Debug code: knocked out */
#if (1 || BVBI_NUM_SCTEE > 0) || (BVBI_NUM_SCTEE_656 > 0) /** { **/

/***************************************************************************
 *
 */
BERR_Code BVBI_LineBuilder_Open (
    BVBI_LineBuilder_Handle* pHandle,
    BMMA_Heap_Handle hMmaHeap, size_t lineCount, size_t lineSize)
{

    int iBank;
    size_t iEntry;
    struct BVBI_P_LineBuilder_Handle* context;
    BERR_Code eErr = BERR_SUCCESS;

    BDBG_ENTER(BVBI_LineBuilder_Open);

    /* A few sanity checks */
    if(!pHandle)
    {
        BDBG_ERR(("Invalid parameter"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if ((lineSize == 0) ||
        (lineCount == 0) ||
        (lineSize < lineCount))
    {
        BDBG_ERR(("Invalid parameter"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Alloc the main context. */
    context = (struct BVBI_P_LineBuilder_Handle*)
        (BKNI_Malloc(sizeof(struct BVBI_P_LineBuilder_Handle)));

    if(!context)
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BDBG_OBJECT_INIT (context, BVBI_LBLDR);

    /* Store attributes provided by user */
    context->hMmaHeap  = hMmaHeap;
    context->lineCount = lineCount;
    context->lineSize  = lineSize;

    /* Two allocations to go. Avoid memory leaks after failure. */
    eErr = BVBI_P_MmaAlloc (
        context->hMmaHeap, lineSize, 16, &(context->bank[0].rawData));
    if (eErr != BERR_SUCCESS)
    {
        BDBG_OBJECT_DESTROY (context, BVBI_LBLDR);
        BKNI_Free (context);
        return BERR_TRACE(eErr);
    }
    eErr = BVBI_P_MmaAlloc (
        context->hMmaHeap, lineSize, 4, &(context->bank[1].rawData));
    if (context->bank[1].rawData.handle == NULL)
    {
        BVBI_P_MmaFree (&(context->bank[0].rawData));
        BDBG_OBJECT_DESTROY (context, BVBI_LBLDR);
        BKNI_Free (context);
        return BERR_TRACE(eErr);
    }

    /* Initialize state */
    context->accumBank = 0;
    for (iBank = 0 ; iBank < 2 ; ++iBank)
    {
        uint8_t* rawData;
        P_Bank* bank = &(context->bank[iBank]);
        bank->accumLength    = 0;
        bank->sequenceNumber = 0;
        bank->lineNumber     = 0;
        rawData = (uint8_t*)bank->rawData.pSwAccess;
        for (iEntry = lineCount ; iEntry < lineSize ; ++iEntry)
            rawData[iEntry] = 0;
        BVBI_P_MmaFlush_isrsafe (&(bank->rawData), lineSize);
    }

    /* All done. now return the new context to user. */
    *pHandle = context;

    BDBG_LEAVE(BVBI_LineBuilder_Open);
    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
void BVBI_LineBuilder_Close (BVBI_LineBuilder_Handle handle)
{
    struct BVBI_P_LineBuilder_Handle* context;
    int iBank;

    BDBG_ENTER (BVBI_LineBuilder_Close);

    /* check parameters */
    context = handle;
    BDBG_OBJECT_ASSERT (context, BVBI_LBLDR);

    /* Wipe out bank storage */
    for (iBank = 0 ; iBank < 2 ; ++iBank)
        BVBI_P_MmaFree (&(context->bank[iBank].rawData));

    /* Release context in system memory */
    BDBG_OBJECT_DESTROY (context, BVBI_LBLDR);
    BKNI_Free((void*)context);

    BDBG_LEAVE (BVBI_LineBuilder_Close);
}


/***************************************************************************
 *
 */
BERR_Code BVBI_LineBuilder_Put_isr (
    BVBI_LineBuilder_Handle handle, uint8_t* sectionData, size_t sectionSize,
    size_t sectionOffset, int sequenceNumber, int lineNumber)
{
    struct BVBI_P_LineBuilder_Handle* context;
    P_Bank* bank;
    uint8_t* rawData;

    BDBG_ENTER (BVBI_LineBuilder_Put_isr);

    /* check parameters */
    context = handle;
    BDBG_OBJECT_ASSERT (context, BVBI_LBLDR);

    /* Will work on the accumulation bank, mostly. */
    bank = &context->bank[context->accumBank];

    /* Debug code: if non-accumulation bank is complete, freeze everything!
    if (context->bank[1-context->accumBank].sequenceNumber)
        return BERR_SUCCESS;
    */

    /* Special case: forced reset. */
    if (sequenceNumber == 0)
    {
        bank->sequenceNumber = 0;
        bank->lineNumber     = 0;
        bank->accumLength    = 0;
        bank = &context->bank[1 - context->accumBank];
        bank->sequenceNumber = 0;
        bank->lineNumber     = 0;
        bank->accumLength    = 0;
        BDBG_LEAVE (BVBI_LineBuilder_Put_isr);
        return BERR_SUCCESS;
    }

    /* Access cached memory only */
    rawData = (uint8_t*)(bank->rawData.pSwAccess);

    /* Can the new data be added? */
    if ((sequenceNumber == bank->sequenceNumber) &&
        (lineNumber     == bank->lineNumber    ) &&
        (sectionOffset  == bank->accumLength   )   )
    {
        /* Slide the new data in at the end */
        if (bank->accumLength + sectionSize > context->lineCount)
        {
            BDBG_ERR(("Invalid parameter"));
            BDBG_LEAVE (BVBI_LineBuilder_Put_isr);
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
        BKNI_Memcpy (
            rawData + sectionOffset, sectionData, sectionSize);
        bank->accumLength += sectionSize;
    }
    else
    {
        /* Replace data completely, it does not match the new data */
        if (sectionOffset == 0)
        {
            if (sectionSize > context->lineCount)
            {
                BDBG_ERR(("Invalid parameter"));
                BDBG_LEAVE (BVBI_LineBuilder_Put_isr);
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
            BKNI_Memcpy (rawData, sectionData, sectionSize);
            bank->accumLength    = sectionSize;
            bank->sequenceNumber = sequenceNumber;
            bank->lineNumber     = lineNumber;
        }
        else
        {
            bank->accumLength    = 0;
            bank->sequenceNumber = 0;
            bank->lineNumber     = 0;
        }
    }

    /* If the accumulation bank is full, start using it for output. */
    if (bank->accumLength == context->lineCount)
    {
        context->accumBank = 1 - context->accumBank;
        bank = &context->bank[context->accumBank];
        bank->sequenceNumber = 0;
        bank->lineNumber     = 0;
        bank->accumLength    = 0;
    }

    BDBG_LEAVE (BVBI_LineBuilder_Put_isr);
    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BVBI_LineBuilder_Get_isr (
    BVBI_LineBuilder_Handle handle,
    BMMA_DeviceOffset* pLineData, int* pSequenceNumber, int* pLineNumber)
{
    struct BVBI_P_LineBuilder_Handle* context;
    P_Bank* bank;

    BDBG_ENTER (BVBI_LineBuilder_Get_isr);

    /* check parameters */
    context = handle;
    BDBG_OBJECT_ASSERT (context, BVBI_LBLDR);

    /* Will work on the non-accumulation bank */
    bank = &context->bank[1 - context->accumBank];

    /* Flush cached DRAM */
    BVBI_P_MmaFlush_isrsafe (&(bank->rawData), bank->accumLength);

    /* Point to data, or lack thereof. */
    *pLineData = bank->rawData.pHwAccess;
    *pSequenceNumber = bank->sequenceNumber;
    *pLineNumber = bank->lineNumber;

    BDBG_LEAVE (BVBI_LineBuilder_Get_isr);
    return BERR_SUCCESS;
}

#endif /** } (BVBI_NUM_SCTEE > 0) || (BVBI_NUM_SCTEE_656 > 0) **/

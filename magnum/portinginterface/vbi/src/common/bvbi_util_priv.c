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
 * Handle processing
 */

/* The LineBuilder object implements a double buffer. This struct defines
 properties common to both buffers. */
typedef struct P_Bank
{
    uint8_t* rawData;       /* The raw data being stored/buffered.          */
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
    BMEM_Handle hMem;

    /* Usable line length in bytes, set at creation time. */
    size_t lineCount;

    /* Physical line length in bytes, set at creation time. */
    size_t lineSize;

    /* Which bank is being accumulated */
    int accumBank;

    /* Raw storage, double buffered. */
    P_Bank bank[2];

};

#if (BVBI_NUM_SCTEE > 0) || (BVBI_NUM_SCTEE_656 > 0) /** { **/

/***************************************************************************
 *
 */
BERR_Code BVBI_LineBuilder_Open (
    BVBI_LineBuilder_Handle* pHandle,
    BMEM_Handle hMem, size_t lineCount, size_t lineSize)
{

    int iBank;
    size_t iEntry;
    struct BVBI_P_LineBuilder_Handle* context;

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
    context->hMem      =      hMem;
    context->lineCount = lineCount;
    context->lineSize  =  lineSize;

    /* Two allocations to go. Avoid memory leaks after failure. */
    context->bank[0].rawData =
        (uint8_t*)(BMEM_AllocAligned (
            context->hMem,
            lineSize,
            4,
            0));
    if (!(context->bank[0].rawData))
    {
        BDBG_OBJECT_DESTROY (context, BVBI_LBLDR);
        BKNI_Free (context);
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    context->bank[1].rawData =
        (uint8_t*)(BMEM_AllocAligned (
            context->hMem,
            lineSize,
            4,
            0));
    if (!(context->bank[1].rawData))
    {
        BMEM_Free (context->hMem, context->bank[0].rawData);
        BDBG_OBJECT_DESTROY (context, BVBI_LBLDR);
        BKNI_Free (context);
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Initialize state */
    context->accumBank = 0;
    for (iBank = 0 ; iBank < 2 ; ++iBank)
    {
        void* cached_ptr;
        uint8_t* rawData;
        BERR_Code eErr;
        P_Bank* bank = &(context->bank[iBank]);
        bank->accumLength    = 0;
        bank->sequenceNumber = 0;
        bank->lineNumber     = 0;
        if ((eErr = BERR_TRACE (BMEM_ConvertAddressToCached (
            context->hMem, bank->rawData, &cached_ptr))) !=
                BERR_SUCCESS)
        {
            BMEM_Free (context->hMem, context->bank[1].rawData);
            BMEM_Free (context->hMem, context->bank[0].rawData);
            BDBG_OBJECT_DESTROY (context, BVBI_LBLDR);
            BKNI_Free (context);
            return eErr;
        }
        rawData = (uint8_t*)cached_ptr;
        for (iEntry = lineCount ; iEntry < lineSize ; ++iEntry)
            rawData[iEntry] = 0;
        BMEM_FlushCache (context->hMem, cached_ptr, lineSize);
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
        BMEM_Free (context->hMem, context->bank[iBank].rawData);

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
    void* cached_ptr;
    uint8_t* rawData;
    BERR_Code eErr;

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
    if ((eErr = BERR_TRACE (BMEM_ConvertAddressToCached_isr (
        context->hMem, bank->rawData, &cached_ptr))) !=
            BERR_SUCCESS)
    {
        BDBG_ERR(("Cache memory failure"));
        BDBG_LEAVE (BVBI_LineBuilder_Put_isr);
        return eErr;
    }
    rawData = (uint8_t*)cached_ptr;

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
    BVBI_LineBuilder_Handle handle, uint8_t** pLineData, int* pSequenceNumber,
    int* pLineNumber)
{
    struct BVBI_P_LineBuilder_Handle* context;
    P_Bank* bank;
    BERR_Code eErr;
    void* cached_ptr;

    BDBG_ENTER (BVBI_LineBuilder_Get_isr);

    /* check parameters */
    context = handle;
    BDBG_OBJECT_ASSERT (context, BVBI_LBLDR);

    /* Will work on the non-accumulation bank */
    bank = &context->bank[1 - context->accumBank];

    /* Flush cached DRAM */
    if ((eErr = BERR_TRACE (BMEM_ConvertAddressToCached_isr (
        context->hMem, bank->rawData, &cached_ptr))) !=
            BERR_SUCCESS)
    {
        BDBG_ERR(("Cache memory failure"));
        BDBG_LEAVE (BVBI_LineBuilder_Get_isr);
        return eErr;
    }
    BMEM_FlushCache_isr (context->hMem, cached_ptr, bank->accumLength);

    /* Point to data, or lack thereof. */
    *pLineData = bank->rawData;
    *pSequenceNumber = bank->sequenceNumber;
    *pLineNumber = bank->lineNumber;

    BDBG_LEAVE (BVBI_LineBuilder_Get_isr);
    return BERR_SUCCESS;
}

#endif /** } (BVBI_NUM_SCTEE > 0) || (BVBI_NUM_SCTEE_656 > 0) **/

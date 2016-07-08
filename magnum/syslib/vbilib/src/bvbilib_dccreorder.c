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

#include "bvbilib_dccreorder.h"
#include "bkni.h"

BDBG_MODULE(BVBIlib);
BDBG_OBJECT_ID (BVBIlib_Reo);

/***************************************************************************
* Private data structures
***************************************************************************/

/* This is an entry in one of the queues of closed caption data */
typedef struct
{
    uint8_t datumL;
    uint8_t datumH;
}
P_CCdata;

/* This is a queue of closed caption data */
typedef struct
{
    unsigned int size;
    unsigned int readN;
    unsigned int writeN;
    P_CCdata* array;
}
P_Queue;

/* This is the complete state of the module */
BDBG_OBJECT_ID_DECLARE (BVBIlib_Reo);
typedef struct BVBIlib_P_DCCReorder_Handle
{
    BDBG_OBJECT (BVBIlib_Reo)

    P_Queue topQ;
    P_Queue botQ;
    unsigned int threshold;
    BAVC_Polarity last_rtn_polarity;
}
BVBIlib_P_DCCReorder_Handle;

/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/

#ifndef MAX
    #define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

#if !B_REFSW_MINIMAL
static BERR_Code P_Put (P_Queue* queue, uint8_t datumL, uint8_t datumH);
static BERR_Code P_Get (P_Queue* queue, uint8_t* pDatumL, uint8_t* pDatumH);
static unsigned int P_Count (P_Queue* queue);
#endif

/***************************************************************************
* Implementation of "BVBIlib_" API functions
***************************************************************************/

#if !B_REFSW_MINIMAL /** { **/

/***************************************************************************
 *
 */
BERR_Code BVBIlib_DCCReorder_Open (
    BVBIlib_DCCReorder_Handle* pHandle,
    unsigned int histSize,
    unsigned int threshold
)
{
    BVBIlib_P_DCCReorder_Handle *prHandle;
    unsigned int vtest;
    unsigned int count = 0;

    BDBG_ENTER(BVBIlib_DCCReorder_Open);

    if (!pHandle)
    {
        BDBG_ERR(("Invalid parameter"));
        BDBG_LEAVE(BVBIlib_DCCReorder_Open);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Verify that the size parameter is nonzero, and a power of two. */
    if (histSize == 0)
    {
        BDBG_ERR(("Invalid parameter"));
        BDBG_LEAVE(BVBIlib_DCCReorder_Open);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    vtest = histSize;
    while (vtest > 1)
    {
        ++count;
        vtest >>= 1;
    }
    vtest = 1 << count;
    if (vtest != histSize)
    {
        BDBG_ERR(("Invalid parameter"));
        BDBG_LEAVE(BVBIlib_DCCReorder_Open);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Don't allow a large threshold */
    if (threshold > histSize)
    {
        BDBG_ERR(("Invalid parameter"));
        BDBG_LEAVE(BVBIlib_DCCReorder_Open);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Alloc the main context. */
    prHandle =
        (BVBIlib_P_DCCReorder_Handle*)(BKNI_Malloc(
            sizeof(BVBIlib_P_DCCReorder_Handle)));

    if(!prHandle)
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BDBG_OBJECT_INIT (prHandle, BVBIlib_Reo);

    /* Allocate queues */
    prHandle->topQ.array = (P_CCdata*)BKNI_Malloc (
        histSize * sizeof(P_CCdata));
    if (!prHandle->topQ.array)
    {
        BDBG_OBJECT_DESTROY (prHandle, BVBIlib_Reo);
        BKNI_Free ((void*)prHandle);
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    prHandle->botQ.array = (P_CCdata*)BKNI_Malloc (
        histSize * sizeof(P_CCdata));
    if (!prHandle->botQ.array)
    {
        BKNI_Free ((void*)(prHandle->topQ.array));
        BDBG_OBJECT_DESTROY (prHandle, BVBIlib_Reo);
        BKNI_Free ((void*)prHandle);
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Store user's settings */
    prHandle->topQ.size = histSize;
    prHandle->botQ.size = histSize;
    prHandle->threshold = threshold;

    /* Initialize empty queues of closed caption data */
    prHandle->topQ.readN  = 0;
    prHandle->topQ.writeN = 0;
    prHandle->botQ.readN  = 0;
    prHandle->botQ.writeN = 0;

    /* The module has returned no CC data yet */
    prHandle->last_rtn_polarity = BAVC_Polarity_eFrame;    /* undefined */

    /* All done. now return the new fresh context to user. */
    *pHandle = (BVBIlib_DCCReorder_Handle)prHandle;

    BDBG_LEAVE(BVBIlib_DCCReorder_Open);
    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
void BVBIlib_DCCReorder_Close (BVBIlib_DCCReorder_Handle handle)
{
    BVBIlib_P_DCCReorder_Handle *prHandle;

    BDBG_ENTER(BVBIlib_DCCReorder_Close);

    /* check parameters */
    prHandle = handle;
    BDBG_OBJECT_ASSERT (prHandle, BVBIlib_Reo);

    /* Release context in system memory */
    BKNI_Free ((void*)(prHandle->topQ.array));
    BKNI_Free ((void*)(prHandle->botQ.array));
    BDBG_OBJECT_DESTROY (prHandle, BVBIlib_Reo);
    BKNI_Free((void*)prHandle);

    BDBG_LEAVE(BVBIlib_DCCReorder_Close);
}


/***************************************************************************
 *
 */
BERR_Code BVBIlib_DCCReorder_Put (
    BVBIlib_DCCReorder_Handle handle,
    uint8_t datumL,
    uint8_t datumH,
    BAVC_Polarity polarity
)
{
    BVBIlib_P_DCCReorder_Handle *prHandle;
    P_Queue* queue;
    BERR_Code eStatus;

    BDBG_ENTER (BVBIlib_DCCReorder_Put);

    /* check parameters */
    prHandle = handle;
    BDBG_OBJECT_ASSERT (prHandle, BVBIlib_Reo);

    /* Choose a queue to operate on */
    switch (polarity)
    {
    case BAVC_Polarity_eTopField:
        queue = &(prHandle->topQ);
        break;
    case BAVC_Polarity_eBotField:
        queue = &(prHandle->botQ);
        break;
    default:
        queue = 0x0;
        return BERR_TRACE (BERR_INVALID_PARAMETER);
        break;
    }

    /* Store the data if there is room */
    eStatus = P_Put (queue, datumL, datumH);

    BDBG_LEAVE (BVBIlib_DCCReorder_Put);
    return eStatus;
}


/***************************************************************************
 *
 */
BERR_Code BVBIlib_DCCReorder_Get (
    BVBIlib_DCCReorder_Handle handle,
    uint8_t* pDatumL,
    uint8_t* pDatumH,
    BAVC_Polarity* pPolarity
)
{
    BVBIlib_P_DCCReorder_Handle *prHandle;
    P_Queue* queue;
    uint8_t datumL = 0x0;
    uint8_t datumH = 0x0;
    BAVC_Polarity polarity = BAVC_Polarity_eFrame;
    BERR_Code eStatus = BERR_OUT_OF_SYSTEM_MEMORY;

    BDBG_ENTER (BVBIlib_DCCReorder_Get);

    /* check parameters */
    prHandle = handle;
    BDBG_OBJECT_ASSERT (prHandle, BVBIlib_Reo);

    /*
     * This is the core algorithm of the entire BVBIlib_dccreorder module. See
     * description of this function in bvbilib_dccreorder.h for details.
     */

    /* Special case: no data returned yet. Any polarity data will do. */
    if (prHandle->last_rtn_polarity == BAVC_Polarity_eFrame)
    {
        if ((eStatus = P_Get (&prHandle->topQ, &datumL, &datumH)) ==
            BERR_SUCCESS)
        {
            prHandle->last_rtn_polarity = BAVC_Polarity_eTopField;
            polarity                    = BAVC_Polarity_eTopField;
        }
        else
        {
            if ((eStatus = P_Get (&prHandle->botQ, &datumL, &datumH)) ==
                BERR_SUCCESS)
            {
                prHandle->last_rtn_polarity = BAVC_Polarity_eBotField;
                polarity                    = BAVC_Polarity_eBotField;
            }
        }
    }

    /* Next, try to maintain alternating polarity. */
    if (eStatus != BERR_SUCCESS)
    {
        if (prHandle->last_rtn_polarity == BAVC_Polarity_eTopField)
        {
            queue = &prHandle->botQ;
            polarity = BAVC_Polarity_eBotField;
        }
        else
        {
            queue = &prHandle->topQ;
            polarity = BAVC_Polarity_eTopField;
        }
        eStatus = P_Get (queue, &datumL, &datumH);
        if (eStatus == BERR_SUCCESS)
        {
            prHandle->last_rtn_polarity = polarity;
        }
    }

    /* Next, check for a too-full top queue. */
    if (eStatus != BERR_SUCCESS)
    {
        if (P_Count (&prHandle->topQ) + prHandle->threshold >
            prHandle->topQ.size)
        {
            eStatus = P_Get (&prHandle->topQ, &datumL, &datumH);
            if (eStatus == BERR_SUCCESS)
            {
                prHandle->last_rtn_polarity = BAVC_Polarity_eTopField;
                polarity                    = BAVC_Polarity_eTopField;
            }
        }
    }

    /* Finally, check for a too-full bottom queue. */
    if (eStatus != BERR_SUCCESS)
    {
        if (P_Count (&prHandle->botQ) + prHandle->threshold >
            prHandle->botQ.size)
        {
            eStatus = P_Get (&prHandle->botQ, &datumL, &datumH);
            if (eStatus == BERR_SUCCESS)
            {
                prHandle->last_rtn_polarity = BAVC_Polarity_eBotField;
                polarity                    = BAVC_Polarity_eBotField;
            }
        }
    }

    /* Return what we got */
    if (eStatus == BERR_SUCCESS)
    {
        *pDatumL   = datumL;
        *pDatumH   = datumH;
        *pPolarity = polarity;
    }

    BDBG_LEAVE (BVBIlib_DCCReorder_Get);
    return eStatus;
}


/***************************************************************************
 *
 */
BERR_Code BVBIlib_DCCReorder_Count (
    BVBIlib_DCCReorder_Handle handle,
    unsigned int* count
)
{
    BVBIlib_P_DCCReorder_Handle *prHandle;

    BDBG_ENTER (BVBIlib_DCCReorder_Count);

    /* check parameters */
    prHandle = handle;
    BDBG_OBJECT_ASSERT (prHandle, BVBIlib_Reo);

    *count = MAX (P_Count (&prHandle->topQ), P_Count (&prHandle->botQ));

    BDBG_LEAVE (BVBIlib_DCCReorder_Count);
    return BERR_SUCCESS;
}

#endif /** } !B_REFSW_MINIMAL **/

/***************************************************************************
* Implementation of private (static) functions
***************************************************************************/

/*
 * Rules for the queues:
 * --------------------
 *
 *  The queue size must be a power of two.
 *
 *  A queue is empty when readN == writeN
 *
 *  A queue is full when readN != writeN AND (readN == writeN (modulo size))
 *
 *  Increment one counter on every read and write. No other counter
 *  modification is allowed.
 */

#if !B_REFSW_MINIMAL /** { **/

/***************************************************************************
 *
 */
static BERR_Code P_Put (P_Queue* queue, uint8_t datumL, uint8_t datumH)
{
    unsigned int index;
    P_CCdata* ccdata;

    /* Check for queue full condition */
    if (queue->readN != queue->writeN)
    {
        if (((queue->writeN - queue->readN) % queue->size) == 0)
        {
            return BERR_OUT_OF_SYSTEM_MEMORY;
        }
    }

    /* Queue not full, work it. */
    index = (queue->writeN) % (queue->size);
    ccdata = &queue->array[index];
    ccdata->datumL = datumL;
    ccdata->datumH = datumH;

    /* Advance the queue */
    ++(queue->writeN);

    /* Success */
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
static BERR_Code P_Get (P_Queue* queue, uint8_t* pDatumL, uint8_t* pDatumH)
{
    unsigned int index;
    P_CCdata* ccdata;

    /* Check for queue empty condition */
    if (queue->readN == queue->writeN)
    {
        return BERR_OUT_OF_SYSTEM_MEMORY;
    }

    /* Queue not empty, work it. */
    index = (queue->readN) % (queue->size);
    ccdata = &queue->array[index];
    *pDatumL = ccdata->datumL;
    *pDatumH = ccdata->datumH;

    /* Advance the queue */
    ++(queue->readN);

    /* Success */
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
static unsigned int P_Count (P_Queue* queue)
{
    unsigned int count = 0;

    if (queue->writeN < queue->readN)
    {
        count = queue->size;
    }

    count += queue->writeN;
    count -= queue->readN;

    return count;
}

#endif /** } !B_REFSW_MINIMAL **/

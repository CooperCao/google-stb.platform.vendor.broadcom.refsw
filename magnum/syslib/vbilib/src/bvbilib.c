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
#include "bstd.h"
#include "berr.h"
#include "berr_ids.h"
#include "bdbg.h"                /* Dbglib */
#include "bkni.h"                /* malloc */
#include "bvbi.h"                /* VBI p.i. API */
#include "bvbi_cap.h"            /* VBI hardware capabilities */
#include "bvbi_prot.h"           /* VBI p.i. protected data structures */
#include "bvbilib.h"             /* This module. */
#include "bvbilib_priv.h"        /* VBI lib internal data structures */

BDBG_MODULE(BVBIlib);
BDBG_OBJECT_ID (BVBIlib);
BDBG_OBJECT_ID (BVBIlib_Enc);
BDBG_OBJECT_ID (BVBIlib_Dec);

/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/

static BVBIlib_P_FieldHanger*
BVBIlib_P_RemoveNull_isr (field_head* field_contexts);


/***************************************************************************
* Implementation of "BVBIlib_" API functions
***************************************************************************/

/***************************************************************************
 *
 */
BERR_Code BVBIlib_Open(
    BVBIlib_Handle  *pVbilHandle,
    BVBI_Handle        vbiHandle
)
{
    BVBIlib_P_Handle *pVbilib;

    BDBG_ENTER(BVBIlib_Open);

    if((!pVbilHandle) ||
       (!vbiHandle)   )
    {
        BDBG_ERR(("Invalid parameter"));
        BDBG_LEAVE(BVBIlib_Open);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Alloc the main VBI context. */
    pVbilib = (BVBIlib_P_Handle*)(BKNI_Malloc(sizeof(BVBIlib_P_Handle)));
    if(!pVbilib)
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BDBG_OBJECT_INIT (pVbilib, BVBIlib);

    /* Store the VBI handle for later use */
    pVbilib->hBvbi = vbiHandle;

    /* Initialize empty lists of decode and encode contexts */
#if (BVBI_NUM_IN656 > 0)
    BLST_Q_INIT(&pVbilib->decode_contexts);
#endif
    BLST_Q_INIT(&pVbilib->encode_contexts);

    /* All done. now return the new fresh context to user. */
    *pVbilHandle = (BVBIlib_Handle)pVbilib;

    BDBG_LEAVE(BVBIlib_Open);
    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
void BVBIlib_Close( BVBIlib_Handle vbilHandle )
{
    BVBIlib_P_Handle *pVbil;

    BDBG_ENTER(BVBIlib_Close);

    /* check parameters */
    pVbil = vbilHandle;
    BDBG_OBJECT_ASSERT (pVbil, BVBIlib);

#if (BVBI_NUM_IN656 > 0)
    /* Refuse service if user left any decoder objects open */
    BDBG_ASSERT (BLST_Q_EMPTY (&pVbil->decode_contexts));
#endif

    /* Refuse service if user left any encoder objects open */
    BDBG_ASSERT (BLST_Q_EMPTY (&pVbil->encode_contexts));

    /* Release context in system memory */
    BDBG_OBJECT_DESTROY (pVbil, BVBIlib);
    BKNI_Free((void*)pVbil);

    BDBG_LEAVE(BVBIlib_Close);
}


#if (BVBI_NUM_IN656 > 0)
/***************************************************************************
 *
 */
BERR_Code BVBIlib_Decode_Create(
    BVBIlib_Handle         vbilHandle,
    BVBIlib_List_Handle   vbillHandle,
    BVBI_Decode_Handle         decHdl,
    BVBIlib_Decode_Handle *  pDeclHdl )
{
    BVBIlib_P_Handle *pVbil;
    BVBIlib_P_Decode_Handle *pVbildec;

    BDBG_ENTER(BVBIlib_Decode_Create);

    pVbil = vbilHandle;
    BDBG_OBJECT_ASSERT (pVbil, BVBIlib);
    if((!vbillHandle ) ||
       (!decHdl      ) ||
       (!pDeclHdl    )  )
    {
        BDBG_ERR(("Invalid parameter"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Alloc the main context. */
    pVbildec =
        (BVBIlib_P_Decode_Handle*)(BKNI_Malloc(
            sizeof(BVBIlib_P_Decode_Handle)));
    if(!pVbildec)
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BDBG_OBJECT_INIT (pVbildec, BVBIlib_Dec);

    /* TODO: verify that the supplied VBI_Handle "owns" the supplied
       VBI_Decode_Handle */

    /* Store all handles passed in */
    pVbildec->pVbilib = pVbil;
    pVbildec->hVbiDec = decHdl;
    pVbildec->hVbill  = vbillHandle;

    /* Join up with the controlling BVBIlib_Handle */
    BLST_Q_INSERT_HEAD (&pVbil->decode_contexts, pVbildec, link);

    /* All done. now return the new fresh context to user. */
    *pDeclHdl = (BVBIlib_Decode_Handle)pVbildec;

    BDBG_LEAVE(BVBIlib_Decode_Create);
    return BERR_SUCCESS;
}
#endif


/***************************************************************************
 *
 */
BERR_Code BVBIlib_Encode_Create(
    BVBIlib_Handle         vbilHandle,
    BVBIlib_List_Handle   vbillHandle,
    BVBI_Encode_Handle         encHdl,
    int                     nMaxQueue,
    BVBIlib_Encode_Handle *  pEnclHdl )
{
    BVBIlib_P_Handle *pVbil;
    BVBIlib_P_Encode_Handle *pVbilenc;
    BVBIlib_P_FieldHanger *hanger;
    int iHandle;

    BDBG_ENTER(BVBIlib_Encode_Create);

    pVbil = vbilHandle;
    BDBG_OBJECT_ASSERT (pVbil, BVBIlib);
    if((!vbillHandle ) ||
       (!encHdl      ) ||
       (!pEnclHdl    )  )
    {
        BDBG_ERR(("Invalid parameter"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Alloc the main context. */
    pVbilenc =
        (BVBIlib_P_Encode_Handle*)(BKNI_Malloc(
            sizeof(BVBIlib_P_Encode_Handle)));
    if(!pVbilenc)
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BDBG_OBJECT_INIT (pVbilenc, BVBIlib_Enc);

    /* TODO: verify that the supplied VBI_Handle "owns" the supplied
       VBI_Encode_Handle */

    /* Store handles and options passed in */
    pVbilenc->pVbilib   = pVbil;
    pVbilenc->hVbiEnc   = encHdl;
    pVbilenc->hVbill    = vbillHandle;
    pVbilenc->nMaxQueue = nMaxQueue;

    /* Join up with the controlling BVBIlib_Handle */
    BLST_Q_INSERT_HEAD (&pVbil->encode_contexts, pVbilenc, link);

    /* Initialize empty lists of field handle hangers */
    BLST_Q_INIT(&pVbilenc->encode_queue);
    pVbilenc->encode_queue_length = 0;
    BLST_Q_INIT(&pVbilenc->empty_hangers);

    /* Create a sufficient number of empty handle hangers */
    for (iHandle = 0 ; iHandle < nMaxQueue ; ++iHandle)
    {
        hanger =
            (BVBIlib_P_FieldHanger*)(BKNI_Malloc(
                sizeof(BVBIlib_P_FieldHanger)));
        if(!hanger)
        {
            BVBIlib_Encode_Destroy ((BVBIlib_Encode_Handle)pVbilenc);
            BDBG_LEAVE(BVBIlib_Encode_Create);
            return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        }
        hanger->hField = NULL;
        BLST_Q_INSERT_HEAD (&pVbilenc->empty_hangers, hanger, link);
    }

    /* All done. now return the new fresh context to user. */
    *pEnclHdl = (BVBIlib_Encode_Handle)pVbilenc;

    BDBG_LEAVE(BVBIlib_Encode_Create);
    return BERR_SUCCESS;
}


#if (BVBI_NUM_IN656 > 0)
/***************************************************************************
 *
 */
BERR_Code BVBIlib_Decode_Destroy( BVBIlib_Decode_Handle declHdl )
{
    BVBIlib_P_Decode_Handle *pVbildec;

    BDBG_ENTER(BVBIlib_Decode_Destroy);

    /* check parameters */
    pVbildec = declHdl;
    BDBG_OBJECT_ASSERT (pVbildec, BVBIlib_Dec);

    /* Unlink from parent handle */
    BLST_Q_REMOVE (&pVbildec->pVbilib->decode_contexts, pVbildec, link);

    /* Release context in system memory */
    BDBG_OBJECT_DESTROY (pVbildec, BVBIlib_Dec);
    BKNI_Free((void*)pVbildec);

    BDBG_LEAVE(BVBIlib_Decode_Destroy);
    return BERR_SUCCESS;
}
#endif


/***************************************************************************
 *
 */
BERR_Code BVBIlib_Encode_Destroy( BVBIlib_Encode_Handle enclHdl )
{
    BVBIlib_P_Encode_Handle *pVbilenc;
    BVBIlib_P_FieldHanger *hanger;
    BERR_Code eErr = BERR_SUCCESS;

    BDBG_ENTER(BVBIlib_Encode_Destroy);

    /* check parameters */
    pVbilenc = enclHdl;
    BDBG_OBJECT_ASSERT (pVbilenc, BVBIlib_Enc);

    /* Unlink from parent handle */
    BLST_Q_REMOVE (&pVbilenc->pVbilib->encode_contexts, pVbilenc, link);

    /* Clear out list of unused field handle hangers */
    /* coverity[alias] */
    /* coverity[USE_AFTER_FREE] */
    while ((hanger = BLST_Q_FIRST (&pVbilenc->empty_hangers)) != NULL)
    {
        BLST_Q_REMOVE (&pVbilenc->empty_hangers, hanger, link);
        /* coverity[freed_arg] */
        BKNI_Free ((void*)hanger);
    }

    /* Complain if user still has some field handles in queue */
    if (!BLST_Q_EMPTY (&pVbilenc->encode_queue))
    {
        eErr = BVBIlib_USER_LEAK;
    }

    /* Release context in system memory */
    BDBG_OBJECT_DESTROY (pVbilenc, BVBIlib_Enc);
    BKNI_Free((void*)pVbilenc);

    BDBG_LEAVE(BVBIlib_Encode_Destroy);
    return eErr;
}


#if (BVBI_NUM_IN656 > 0)
/***************************************************************************
 *
 */
BERR_Code BVBIlib_Decode_Data_isr(
    BVBIlib_Decode_Handle     declHdl,
    BVBI_Field_Handle *  pFieldHandle,
    BAVC_Polarity            polarity
)
{
    BVBIlib_P_Decode_Handle *pVbildec;
    BVBI_Field_Handle hField = 0;
    BERR_Code eErr = BERR_SUCCESS;

    BDBG_ENTER(BVBIlib_Decode_Data_isr);

    /* check parameters */
    pVbildec = declHdl;
    BDBG_OBJECT_ASSERT (pVbildec, BVBIlib_Dec);

    /* Obtain a free field handle */
    eErr = BVBIlib_List_Obtain_isr (pVbildec->hVbill, &hField);
    if (eErr != BERR_SUCCESS)
    {
        hField = 0;
        BDBG_LEAVE(BVBIlib_Decode_Data_isr);
        return eErr;
    }

    /* Call into BVBI module to decode */
    eErr = BVBI_Decode_Data_isr (pVbildec->hVbiDec, hField, polarity);

    /* Prevent a memory leak */
    if (eErr != BERR_SUCCESS)
    {
        BVBIlib_List_Return_isr (pVbildec->hVbill, hField);
        hField = 0;
    }

    /* Return data to caller */
    *pFieldHandle = hField;

    BDBG_LEAVE(BVBIlib_Decode_Data_isr);
    return eErr;
}
#endif


/***************************************************************************
 *
 */
BERR_Code BVBIlib_Encode_Enqueue_isr(
    BVBIlib_Encode_Handle      enclHdl,
    BVBI_Field_Handle      fieldHandle
)
{
    BVBIlib_P_Encode_Handle *pVbilenc;
    BVBIlib_P_FieldHanger *hanger;

    BDBG_ENTER(BVBIlib_Encode_Enqueue_isr);

    /* check parameters */
    pVbilenc = enclHdl;
    BDBG_OBJECT_ASSERT (pVbilenc, BVBIlib_Enc);

    /* If there are no empty hangers available */
    if ((hanger = BLST_Q_FIRST (&pVbilenc->empty_hangers)) == NULL)
    {
        /* The input queue must be full. Look for a NULL datum to remove. */
        if ((hanger = BVBIlib_P_RemoveNull_isr (&pVbilenc->encode_queue)) ==
            NULL)
        {
            /* Deny service. */
            return BVBIlib_QUEUE_FULL;
            /* Programming note: no BERR_TRACE(), because this statement
             * is likely to occur 50 to 60 times per second if it occurs
             * at all. */
        }
        --pVbilenc->encode_queue_length;
        BVBI_Field_Decrement_UsageCount_isr (hanger->hField);
        if (BVBI_Field_Get_UsageCount_isr (hanger->hField) == 0)
        {
            BVBIlib_List_Return_isr (pVbilenc->hVbill, hanger->hField);
        }
        hanger->hField = NULL;
    }
    else
    {
        /* Use the first empty hanger (obtained above) */
        BLST_Q_REMOVE (&pVbilenc->empty_hangers, hanger, link);
    }

    /* Enqueue caller's data for subsequent encoding */
    hanger->hField = fieldHandle;
    BLST_Q_INSERT_TAIL (&pVbilenc->encode_queue, hanger, link);
    ++pVbilenc->encode_queue_length;

    /* Increment use count of field handle */
    BVBI_Field_Increment_UsageCount_isr (fieldHandle);

    BDBG_LEAVE(BVBIlib_Encode_Enqueue_isr);
    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BVBIlib_Encode_Data_isr(
    BVBIlib_Encode_Handle      enclHdl,
    BAVC_Polarity             polarity
)
{
    BVBIlib_P_Encode_Handle *pVbilenc;
    BVBIlib_P_FieldHanger *hanger;
    BERR_Code eErr = BERR_SUCCESS;

    BDBG_ENTER(BVBIlib_Encode_Data_isr);

    /* check parameters */
    pVbilenc = enclHdl;
    BDBG_OBJECT_ASSERT (pVbilenc, BVBIlib_Enc);

    /* If there is a queued field handle available */
    if ((hanger = BLST_Q_FIRST (&pVbilenc->encode_queue)) != NULL)
    {
        /* Check polarity */
        uint32_t myMask;
        uint32_t fieldMask;
        myMask = (uint32_t)1 << (uint32_t)polarity;
        eErr = BVBI_Field_GetPolarity_isr (hanger->hField, &fieldMask);
        BDBG_ASSERT (eErr == BERR_SUCCESS);
        if (fieldMask == 0)
        {
            /* A field handle with no polarity.  Get rid of it, or
             * it will stay in the queue forever. The following code
             * block is a repeat of what appears below, but without
             * comments.
             */
            BLST_Q_REMOVE (&pVbilenc->encode_queue, hanger, link);
            --pVbilenc->encode_queue_length;
            BVBI_Field_Decrement_UsageCount_isr (hanger->hField);
            if (BVBI_Field_Get_UsageCount_isr (hanger->hField) == 0)
            {
                BVBIlib_List_Return_isr (pVbilenc->hVbill, hanger->hField);
            }
            hanger->hField = NULL;
            BLST_Q_INSERT_TAIL (&pVbilenc->empty_hangers, hanger, link);

            /* Cause the encoder hardware to output no VBI next field */
            eErr = BVBI_Encode_Data_isr (pVbilenc->hVbiEnc, NULL, polarity);
        }
        else if ((polarity == BAVC_Polarity_eFrame) ||
                 ((myMask & fieldMask) == myMask   )   )
        {
            /*
             * Normal case
             */
            /* Remove from input queue */
            BLST_Q_REMOVE (&pVbilenc->encode_queue, hanger, link);
            --pVbilenc->encode_queue_length;

            /* Give to hardware for encoding at next field */
            eErr =
                BVBI_Encode_Data_isr (
                    pVbilenc->hVbiEnc, hanger->hField, polarity);

            /* Decrement use count of field handle */
            BVBI_Field_Decrement_UsageCount_isr (hanger->hField);

            /* Recycle the field handle if appropriate */
            if (BVBI_Field_Get_UsageCount_isr (hanger->hField) == 0)
            {
                BVBIlib_List_Return_isr (pVbilenc->hVbill, hanger->hField);
            }

            /* Recycle the hanger in any case */
            hanger->hField = NULL;
            BLST_Q_INSERT_TAIL (&pVbilenc->empty_hangers, hanger, link);
        }
        else
        {
            /*
             * Field mismatch. The field handle stays in the queue until
             * the next interrupt.
             */
            /* Cause the encoder hardware to output no VBI next field */
            eErr = BVBI_Encode_Data_isr (pVbilenc->hVbiEnc, NULL, polarity);
        }
    }
    else
    {
        /* Cause the encoder hardware to output no VBI next field */
        eErr = BVBI_Encode_Data_isr (pVbilenc->hVbiEnc, NULL, polarity);
    }

    BDBG_LEAVE(BVBIlib_Encode_Encode_Data_isr);
    return eErr;
}


/***************************************************************************
 *
 */
BERR_Code BVBIlib_Encode_GetOldestDatum_isr(
    BVBIlib_Encode_Handle enclHdl,
    BVBI_Field_Handle* pFieldHandle)
{
    BVBIlib_P_Encode_Handle *pVbilenc;
    BVBIlib_P_FieldHanger *hanger;
    BVBI_Field_Handle hField = NULL;

    BDBG_ENTER(BVBIlib_Encode_GetOldestDatum_isr);

    /* check parameters */
    pVbilenc = enclHdl;
    BDBG_OBJECT_ASSERT (pVbilenc, BVBIlib_Enc);

    /* If there is a queued field handle available */
    if ((hanger = BLST_Q_FIRST (&pVbilenc->encode_queue)) != NULL)
    {
        hField = hanger->hField;
    }

    /* Return what we got, if anything. */
    *pFieldHandle = hField;
    return BERR_SUCCESS;
}


#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVBIlib_Encode_NoData_isr(
    BVBIlib_Encode_Handle      enclHdl,
    BAVC_Polarity             polarity
)
{
    BVBIlib_P_Encode_Handle *pVbilenc;
    BERR_Code eErr = BERR_SUCCESS;

    BDBG_ENTER(BVBIlib_Encode_NoData_isr);

    /* check parameters */
    pVbilenc = enclHdl;
    BDBG_OBJECT_ASSERT (pVbilenc, BVBIlib_Enc);

    /* Cause the encoder hardware to output no VBI next field */
    eErr = BVBI_Encode_Data_isr (pVbilenc->hVbiEnc, NULL, polarity);

    BDBG_LEAVE(BVBIlib_Encode_Encode_Data_isr);
    return eErr;
}
#endif


/***************************************************************************
 *
 */
BERR_Code BVBIlib_Encode_Flush_isr( BVBIlib_Encode_Handle enclHdl )
{
    BVBIlib_P_Encode_Handle *pVbilenc;
    BVBIlib_P_FieldHanger *hanger;
    BERR_Code eErr = BERR_SUCCESS;

    BDBG_ENTER(BVBIlib_Encode_Flush_isr);

    /* check parameters */
    pVbilenc = enclHdl;
    BDBG_OBJECT_ASSERT (pVbilenc, BVBIlib_Enc);

    /* While there are queued field handles available */
    while ((hanger = BLST_Q_FIRST (&pVbilenc->encode_queue)) != NULL)
    {
        /* Remove from input queue */
        BLST_Q_REMOVE (&pVbilenc->encode_queue, hanger, link);
        --pVbilenc->encode_queue_length;

        /* Decrement use count of field handle */
        BVBI_Field_Decrement_UsageCount_isr (hanger->hField);

        /* Recycle the field handle if appropriate */
        if (BVBI_Field_Get_UsageCount_isr (hanger->hField) == 0)
        {
            BVBIlib_List_Return_isr (pVbilenc->hVbill, hanger->hField);
        }

        /* Recycle the hanger too */
        hanger->hField = NULL;
        BLST_Q_INSERT_TAIL (&pVbilenc->empty_hangers, hanger, link);
    }

    BDBG_LEAVE(BVBIlib_Encode_Encode_Flush_isr);
    return eErr;
}


#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
int BVBIlib_GetQueueLength (BVBIlib_Encode_Handle enclHdl)
{
    BVBIlib_P_Encode_Handle *pVbilenc;

    BDBG_ENTER(BVBIlib_Encode_Flush_isr);

    /* check parameters */
    pVbilenc = enclHdl;
    BDBG_OBJECT_ASSERT (pVbilenc, BVBIlib_Enc);

    /* Return value as requested */
    return pVbilenc->encode_queue_length;
}
#endif


/***************************************************************************
 *
 */
static BVBIlib_P_FieldHanger*
BVBIlib_P_RemoveNull_isr (field_head* field_contexts)
{
    BVBIlib_P_FieldHanger *hanger;

    for (hanger = BLST_Q_FIRST(field_contexts) ;
         hanger != NULL ;
         hanger = BLST_Q_NEXT(hanger, link))
    {
        BVBI_Field_Handle handle = hanger->hField;
        if (BVBI_Field_IsNull_isr (handle))
        {
            BLST_Q_REMOVE (field_contexts, hanger, link);
            break;
        }

    }
    return hanger;
}

/* End of File */

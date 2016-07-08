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
#include "bvbi_prot.h"           /* VBI p.i. protected data structures */
#include "bvbilib.h"             /* This module. */
#include "bvbilib_priv.h"        /* VBI lib internal data structures */

BDBG_MODULE(BVBIlib);
BDBG_OBJECT_ID (BVBIlib_List);

/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/

/***************************************************************************
* Local (private) data
***************************************************************************/
static const BVBIlib_List_Settings sDefaultSettings = {
    false, false, false, false, false, false, 0, 0, 0, 0
};

/***************************************************************************
* Implementation of "BVBI_" API functions
***************************************************************************/

/***************************************************************************
 *
 */
BERR_Code BVBIlib_List_GetDefaultSettings (BVBIlib_List_Settings*  pSettings)
{
    if (!pSettings)
    {
        BDBG_ERR(("Invalid parameter"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    *pSettings = sDefaultSettings;

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVBIlib_List_Create(
    BVBIlib_List_Handle *pVbillHandle,
    BVBI_Handle             vbiHandle,
    int                      nHandles,
    const BVBIlib_List_Settings*  pSettings
)
{
    BVBIlib_P_List_Handle *pVbilibl;
    int iHandle;
    BVBIlib_P_FieldHanger *hanger;
    BVBI_Field_Handle hField;
    BERR_Code eErr;

    BDBG_ENTER(BVBIlib_List_Create);

    if((!pVbillHandle) ||
       (nHandles <= 0)   )
    {
        BDBG_ERR(("Invalid parameter"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Alloc the main VBI context. */
    pVbilibl =
        (BVBIlib_P_List_Handle*)(BKNI_Malloc(sizeof(BVBIlib_P_List_Handle)));

    if(!pVbilibl)
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BDBG_OBJECT_INIT (pVbilibl, BVBIlib_List);

    /* Store memory handle to allocate teletext data as necessary */
    pVbilibl->hVbi = vbiHandle;

    /* Store various settings */
    if (!pSettings)
        pSettings = &sDefaultSettings;
    pVbilibl->settings = *pSettings;
    /* Programming note: these settings are not really needed. But in a future
       ----------------  version of the module, they might be? */

    /* Initialize empty lists of field handle hangers */
    BLST_Q_INIT(&pVbilibl->field_contexts);
    BLST_Q_INIT(&pVbilibl->empty_hangers);

    /* Loop over handles to be created */
    for (iHandle = 0 ; iHandle < nHandles ; ++iHandle)
    {
        /* Create a field handle */
        if ((eErr = BVBI_Field_Create (&hField, vbiHandle)) != BERR_SUCCESS)
        {
            BVBIlib_List_Destroy ((BVBIlib_List_Handle)pVbilibl);
            BDBG_LEAVE(BVBIlib_List_Create);
            return eErr;
        }

        /* Create a field handle hanger to hold the above handle */
        hanger =
            (BVBIlib_P_FieldHanger*)(BKNI_Malloc(
                sizeof(BVBIlib_P_FieldHanger)));
        if(!hanger)
        {
            BVBI_Field_Destroy (hField);
            BVBIlib_List_Destroy ((BVBIlib_List_Handle)pVbilibl);
            BDBG_LEAVE(BVBIlib_List_Create);
            return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        }
        hanger->hField = hField;

        /* Add field/hanger assembly to the free list */
        BLST_Q_INSERT_HEAD (&pVbilibl->field_contexts, hanger, link);

        /* Allocate for teletext if necessary */
        if (pSettings->bAllowTeletext)
        {
            if ((eErr =
                BVBI_Field_TT_Allocate (
                    hField, BVBI_TT_MAX_LINES, BVBI_TT_MAX_LINELENGTH)) !=
                BERR_SUCCESS)
            {
                BVBIlib_List_Destroy ((BVBIlib_List_Handle)pVbilibl);
                BDBG_LEAVE(BVBIlib_List_Create);
                return eErr;
            }
        }

        /* Allocate for VPS if necessary */
        if (pSettings->bAllowVPS)
        {
            if ((eErr = BVBI_Field_VPS_Allocate (hField, true)) != BERR_SUCCESS)
            {
                BVBIlib_List_Destroy ((BVBIlib_List_Handle)pVbilibl);
                BDBG_LEAVE(BVBIlib_List_Create);
                return eErr;
            }
        }

        /* Allocate for Gemstar if necessary */
        if (pSettings->bAllowGemstar)
        {
            if ((eErr = BVBI_Field_GS_Allocate (hField, true)) != BERR_SUCCESS)
            {
                BVBIlib_List_Destroy ((BVBIlib_List_Handle)pVbilibl);
                BDBG_LEAVE(BVBIlib_List_Create);
                return eErr;
            }
        }

        /* Allocate for AMOL if necessary */
        if (pSettings->bAllowAmol)
        {
            if ((eErr =
                BVBI_Field_AMOL_Allocate (hField, BVBI_AMOL_Type_II_Highrate))
                != BERR_SUCCESS)
            {
                BVBIlib_List_Destroy ((BVBIlib_List_Handle)pVbilibl);
                BDBG_LEAVE(BVBIlib_List_Create);
                return eErr;
            }
        }

        /* Allocate for multi-line closed caption, if necessary */
        if (pSettings->bAllowMcc)
        {
            if ((eErr = BVBI_Field_MCC_Allocate (hField, true)) != BERR_SUCCESS)
            {
                BVBIlib_List_Destroy ((BVBIlib_List_Handle)pVbilibl);
                BDBG_LEAVE(BVBIlib_List_Create);
                return eErr;
            }
        }

        /* Allocate for CGMS-B, if necessary */
        if (pSettings->bAllowCgmsB)
        {
            if ((eErr = BVBI_Field_CGMSB_Allocate (hField, true)) !=
                BERR_SUCCESS)
            {
                BVBIlib_List_Destroy ((BVBIlib_List_Handle)pVbilibl);
                BDBG_LEAVE(BVBIlib_List_Create);
                return eErr;
            }
        }

        /* Allocate for SCTE if necessary.  Not space efficient. */
        if ((pSettings->scteCcSize  != 0) || (pSettings->scteNrtvSize != 0) ||
            (pSettings->sctePamSize != 0) || (pSettings->scteMonoSize != 0)   )
        {
            if ((eErr =
                BVBI_Field_SCTE_Allocate (hField,
                    pSettings->scteCcSize,  pSettings->scteNrtvSize,
                    pSettings->sctePamSize, pSettings->scteMonoSize))
                != BERR_SUCCESS)
            {
                BVBIlib_List_Destroy ((BVBIlib_List_Handle)pVbilibl);
                BDBG_LEAVE(BVBIlib_List_Create);
                return eErr;
            }
        }
    }

    /* Initialize status counters */
    pVbilibl->nAllocated = nHandles;
    pVbilibl->nInUse     =        0;

    /* All done. now return the new fresh context to user. */
    *pVbillHandle = (BVBIlib_List_Handle)pVbilibl;

    BDBG_LEAVE(BVBIlib_List_Create);
    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BVBIlib_List_Destroy( BVBIlib_List_Handle listHandle )
{
    BVBIlib_P_List_Handle *pVbill;
    BVBIlib_P_FieldHanger* hanger;
    BVBI_Field_Handle      hField;
    BERR_Code eErr      = BERR_SUCCESS;
    BERR_Code eErrFirst = BERR_SUCCESS;

    BDBG_ENTER(BVBIlib_List_Destroy);

    /* check parameters */
    pVbill = listHandle;
    BDBG_OBJECT_ASSERT (pVbill, BVBIlib_List);

    /* Determine if user has failed to turn in all handles */
    if (pVbill->nInUse != 0)
    {
        BDBG_ERR(("Field handle leak"));
        return BERR_TRACE(BVBIlib_USER_LEAK);
    }

    /* Clear out list of free handles */
    /* coverity[alias] */
    /* coverity[USE_AFTER_FREE] */
    while ((hanger = BLST_Q_FIRST (&pVbill->field_contexts)) != NULL)
    {
        BLST_Q_REMOVE (&pVbill->field_contexts, hanger, link);
        hField = hanger->hField;
        if ((eErr = BVBI_Field_Destroy (hField)) != BERR_SUCCESS)
        {
            if (eErrFirst == BERR_SUCCESS)
            {
                eErrFirst = eErr;
            }
        }
        BKNI_Free ((void*)hanger);
    }

    /* Clear out list of unused field handle hangers */
    /* coverity[alias] */
    /* coverity[USE_AFTER_FREE] */
    while ((hanger = BLST_Q_FIRST (&pVbill->empty_hangers)) != NULL)
    {
        BLST_Q_REMOVE (&pVbill->empty_hangers, hanger, link);
        /* coverity[freed_arg] */
        BKNI_Free ((void*)hanger);
    }

    /* Release context in system memory */
    BDBG_OBJECT_DESTROY (pVbill, BVBIlib_List);
    BKNI_Free((void*)pVbill);

    BDBG_LEAVE(BVBIlib_List_Destroy);
    return eErrFirst;
}


/***************************************************************************
 *
 */
BERR_Code BVBIlib_List_Obtain_isr(
    BVBIlib_List_Handle   vbillHandle,
    BVBI_Field_Handle *  pFieldHandle
)
{
    BVBIlib_P_List_Handle *pVbill;
    BVBIlib_P_FieldHanger* hanger;
    BERR_Code eErr      = BERR_SUCCESS;

    BDBG_ENTER(BVBIlib_List_Obtain_isr);

    /* check parameters */
    pVbill = vbillHandle;
    BDBG_OBJECT_ASSERT (pVbill, BVBIlib_List);

    /* If there is a handle available */
    if ((hanger = BLST_Q_FIRST (&pVbill->field_contexts)) != NULL)
    {
        /* Take it out of the free list */
        BLST_Q_REMOVE (&pVbill->field_contexts, hanger, link);

        /* Adjust attributes */
        BVBI_Field_Zero_UsageCount_isr (hanger->hField);
        BVBI_Field_ClearState_isr      (hanger->hField);

        /* Give it to the caller */
        *pFieldHandle = hanger->hField;

        /* Hold on to the empty hanger for use later */
        hanger->hField = NULL;
        BLST_Q_INSERT_TAIL (&pVbill->empty_hangers, hanger, link);

        /* Adjust statistic */
        ++(pVbill->nInUse);
    }
    else
    {
        /* Complain */
        eErr = BERR_TRACE(BVBIlib_NO_HANDLES);
    }

    BDBG_LEAVE(BVBIlib_List_Obtain_isr);
    return eErr;
}


/***************************************************************************
 *
 */
BERR_Code BVBIlib_List_Return_isr(
    BVBIlib_List_Handle    vbillHandle,
    BVBI_Field_Handle     fieldHandle
)
{
    BVBIlib_P_List_Handle *pVbill;
    BVBIlib_P_FieldHanger *hanger;

    BDBG_ENTER(BVBIlib_List_Return_isr);

    /* check parameters */
    pVbill = vbillHandle;
    BDBG_OBJECT_ASSERT (pVbill, BVBIlib_List);

    /* Usage check */
    if (BVBI_Field_Get_UsageCount_isr (fieldHandle) != 0)
    {
        BDBG_ERR(("BVBIlib_List_Return_isr leak"));
        return BVBIlib_USER_LEAK;
    }

    /* If there are no empty hangers left, then the list is corrupted */
    hanger = BLST_Q_FIRST (&pVbill->empty_hangers);
    BDBG_ASSERT (hanger != NULL);
    BLST_Q_REMOVE (&pVbill->empty_hangers, hanger, link);

    /* Hook up the user's field hanger and file it */
    hanger->hField = fieldHandle;
    BLST_Q_INSERT_HEAD (&pVbill->field_contexts, hanger, link);

    /* Adjust statistic */
    --(pVbill->nInUse);

    BDBG_LEAVE(BVBIlib_List_Return_isr);
    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
void BVBIlib_List_IncrementUsage_isr(
    BVBI_Field_Handle     fieldHandle
)
{
    BDBG_ENTER(BVBIlib_List_IncrementUsage_isr);

    /* Just increment usage count */
    BVBI_Field_Increment_UsageCount_isr (fieldHandle);

    BDBG_LEAVE(BVBIlib_List_IncrementUsage_isr);
}


#if !B_REFSW_MINIMAL /** { **/

/***************************************************************************
 *
 */
BERR_Code BVBIlib_List_DecrementUsage_isr(
    BVBIlib_List_Handle    vbillHandle,
    BVBI_Field_Handle     fieldHandle
)
{
    BERR_Code eStatus = BERR_SUCCESS;

    BDBG_ENTER(BVBIlib_List_DecrementUsage_isr);

    /* Decrement usage count */
    BVBI_Field_Decrement_UsageCount_isr (fieldHandle);

    /* Recycle if usage count is zero */
    if (BVBI_Field_Get_UsageCount_isr (fieldHandle) == 0)
    {
        eStatus = BVBIlib_List_Return_isr (vbillHandle, fieldHandle);
    }

    BDBG_LEAVE(BVBIlib_List_DecrementUsage_isr);
    return eStatus;
}


/***************************************************************************
 *
 */
BERR_Code BVBIlib_List_DecrementUsage(
    BVBIlib_List_Handle    vbillHandle,
    BVBI_Field_Handle     fieldHandle
)
{
    BERR_Code eStatus;

    BDBG_ENTER(BVBIlib_List_DecrementUsage);

    BKNI_EnterCriticalSection();
    eStatus = BVBIlib_List_DecrementUsage_isr (vbillHandle, fieldHandle);
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BVBIlib_List_DecrementUsage);
    return eStatus;
}

#endif /** } !B_REFSW_MINIMAL **/

/* End of File */

/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "bstd.h"           /* standard types */
#include "bdbg.h"           /* Dbglib */
#include "bkni.h"           /* For critical sections */
#include "bvbi.h"           /* VBI processing, this module. */
#include "bvbi_priv.h"      /* VBI internal data structures */

BDBG_MODULE(BVBI);
BDBG_OBJECT_ID(BVBI_FIELD);

/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/

#if !B_REFSW_MINIMAL
static BERR_Code
BVBI_P_Field_ClearFlagData_isr (
    BVBI_Field_Handle fieldHandle,
    uint32_t flagBit);
#endif


/***************************************************************************
 *
 */
BERR_Code BVBI_Field_Create (
    BVBI_Field_Handle* pFieldHandle, BVBI_Handle vbiHandle )
{
    BVBI_P_Handle *pVbi;
    BVBI_P_Field_Handle *pVbi_Fld = NULL;
    BVBI_P_TTData* pttData;
    BERR_Code eStatus = BERR_SUCCESS;

    BDBG_ENTER(BVBI_Field_Create);

    /* check parameters */
    pVbi = vbiHandle;
    BDBG_OBJECT_ASSERT (pVbi, BVBI);
    if (!pFieldHandle)
    {
        BDBG_ERR(("Invalid parameter"));
        eStatus = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto BVBI_Field_Create_Done;
    }

    /* Alloc the VBI field handle. */
    pVbi_Fld =
        (BVBI_P_Field_Handle*)(BKNI_Malloc(sizeof(BVBI_P_Field_Handle)));
    if(!pVbi_Fld)
    {
        eStatus = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto BVBI_Field_Create_Done;
    }
    BDBG_OBJECT_INIT (pVbi_Fld, BVBI_FIELD);

    /* Set defaults. */
    pVbi_Fld->usCCData = BVBI_P_SetCCParityBits_isrsafe(0x0);
    pVbi_Fld->ulErrInfo = 0x0;

    /* Remember VBI main handle for subsequent allocations */
    pVbi_Fld->pVbi = pVbi;

    /* Indicate no room for teletext, VPS, Gemstar, AMOL, or MCC  data */
    pttData = (BVBI_P_TTData*)(BKNI_Malloc(sizeof(BVBI_P_TTData)));
    if(!pttData)
    {
        eStatus = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto BVBI_Field_Create_Done;
    }
    BKNI_Memset((void*)pttData, 0x0, sizeof(BVBI_P_TTData));
    BVBI_P_TTData_Alloc (pVbi_Fld->pVbi->hMmaHeap, 0, 0, pttData);
    BVBI_P_LCOP_CREATE (pVbi_Fld, TTDataO, pttData, clink);
    pVbi_Fld->pVPSData = 0x0;
    pVbi_Fld->pGSData = 0x0;
    pVbi_Fld->amolType = BVBI_AMOL_Type_None;
    pVbi_Fld->pAmolData = 0x0;
    pVbi_Fld->amolSize = 0;
    pVbi_Fld->pMCCData = 0x0;
    pVbi_Fld->pPScteData = 0x0;
    pVbi_Fld->pCgmsbDatum = 0x0;

    /* Indicate no valid VBI data yet */
    pVbi_Fld->ulWhichPresent = 0x0;

    /* We don't know what the polarity is */
    pVbi_Fld->polarityMask = 0x0;

    /* Initialize protected attributes */
    pVbi_Fld->inUseCount = 0;

    /* All done. now return the new fresh context to user. */
    *pFieldHandle = (BVBI_Field_Handle)pVbi_Fld;

  BVBI_Field_Create_Done:

    BDBG_LEAVE(BVBI_Field_Create);

    if ((BERR_SUCCESS != eStatus) && (NULL != pVbi_Fld))
    {
        BDBG_OBJECT_DESTROY (pVbi_Fld, BVBI_FIELD);
        BKNI_Free((void*)pVbi_Fld);
    }

    /* coverity[leaked_storage: FALSE] */
    return eStatus;
}

/***************************************************************************
 *
 */
BERR_Code BVBI_Field_TT_Allocate(
                BVBI_Field_Handle fieldHandle,
                uint8_t ucMaxLines,
                uint8_t ucLineSize )
{
    BVBI_P_Field_Handle* pVbi_Fld;
    BVBI_P_TTData* pttData;
    BERR_Code eErr;

    BDBG_ENTER(BVBI_Field_TT_Allocate);

    /* check parameters */
    pVbi_Fld = fieldHandle;
    BDBG_OBJECT_ASSERT (pVbi_Fld, BVBI_FIELD);
    if ((ucMaxLines != 0) && (ucLineSize == 0))
    {
        BDBG_ERR(("Invalid parameter"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* (Re)allocate TT data as necessary */
    BVBI_P_LCOP_WRITE (pVbi_Fld, TTDataO, &pVbi_Fld->pVbi->ttFreelist, clink);
    pttData = BVBI_P_LCOP_GET (pVbi_Fld, TTDataO);
    eErr = BERR_TRACE (BVBI_P_TTData_Alloc (
        pVbi_Fld->pVbi->hMmaHeap, ucMaxLines, ucLineSize, pttData));
    if (eErr != BERR_SUCCESS)
    {
        BDBG_LEAVE(BVBI_Field_TT_Allocate);
        return eErr;
    }

    BDBG_LEAVE(BVBI_Field_TT_Allocate);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVBI_Field_VPS_Allocate(
                BVBI_Field_Handle fieldHandle,
                bool bEnable )
{
    BVBI_P_Field_Handle* pVbi_Fld;

    BDBG_ENTER(BVBI_Field_VPS_Allocate);

    /* check parameters */
    pVbi_Fld = fieldHandle;
    BDBG_OBJECT_ASSERT (pVbi_Fld, BVBI_FIELD);

    /* Lose any VPS data that may exist already */
    if (pVbi_Fld->pVPSData)
    {
        BKNI_Free((void*)pVbi_Fld->pVPSData);
        pVbi_Fld->pVPSData = 0x0;
    }

    /* If user wants to hold any VPS data */
    if (bEnable)
    {
        pVbi_Fld->pVPSData =
            (BVBI_VPSData*)(BKNI_Malloc(sizeof(BVBI_VPSData)));
        if(!pVbi_Fld->pVPSData)
        {
            return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        }
    }

    BDBG_LEAVE(BVBI_Field_VPS_Allocate);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVBI_Field_GS_Allocate(
                BVBI_Field_Handle fieldHandle,
                bool bEnable )
{
    BVBI_P_Field_Handle* pVbi_Fld;

    BDBG_ENTER(BVBI_Field_GS_Allocate);

    /* check parameters */
    pVbi_Fld = fieldHandle;
    BDBG_OBJECT_ASSERT (pVbi_Fld, BVBI_FIELD);

    /* Lose any GS data that may exist already */
    if (pVbi_Fld->pGSData)
    {
        BKNI_Free((void*)pVbi_Fld->pGSData);
        pVbi_Fld->pGSData = 0x0;
    }

    /* If user wants to hold any GS data */
    if (bEnable)
    {
        pVbi_Fld->pGSData =
            (BVBI_GSData*)(BKNI_Malloc(sizeof(BVBI_GSData)));
        if(!pVbi_Fld->pGSData)
        {
            return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        }
    }

    BDBG_LEAVE(BVBI_Field_GS_Allocate);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVBI_Field_AMOL_Allocate(
                BVBI_Field_Handle fieldHandle,
                BVBI_AMOL_Type amolType)
{
    BVBI_P_Field_Handle* pVbi_Fld;
    int dataSize;

    BDBG_ENTER(BVBI_Field_AMOL_Allocate);

    /* check parameters */
    pVbi_Fld = fieldHandle;
    BDBG_OBJECT_ASSERT (pVbi_Fld, BVBI_FIELD);

    /* Lose any AMOL data that may exist already */
    if (pVbi_Fld->pAmolData)
    {
        BKNI_Free((void*)pVbi_Fld->pAmolData);
        pVbi_Fld->pAmolData = 0x0;
        pVbi_Fld->amolSize = 0;
    }

    switch (amolType)
    {
    case BVBI_AMOL_Type_None:
        dataSize = 0;
        break;
    case BVBI_AMOL_Type_I:
        dataSize = 6;
        break;
    case BVBI_AMOL_Type_II_Lowrate:
        dataSize = 12;
        break;
    case BVBI_AMOL_Type_II_Highrate:
        dataSize = 24;
        break;
    default:
        BDBG_ERR(("Invalid parameter"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
        break;
    }

    if (dataSize != 0)
    {
        pVbi_Fld->pAmolData =
            (uint8_t*)(BKNI_Malloc(dataSize));
        if(!pVbi_Fld->pAmolData)
        {
            return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        }
        pVbi_Fld->amolSize = dataSize;
    }

    BDBG_LEAVE(BVBI_Field_AMOL_Allocate);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVBI_Field_MCC_Allocate(
                BVBI_Field_Handle fieldHandle,
                bool alloc)
{
    BVBI_P_Field_Handle* pVbi_Fld;

    BDBG_ENTER(BVBI_Field_MCC_Allocate);

    /* check parameters */
    pVbi_Fld = fieldHandle;
    BDBG_OBJECT_ASSERT (pVbi_Fld, BVBI_FIELD);

    /* Lose any MCC data that may exist already */
    if (pVbi_Fld->pMCCData)
    {
        BKNI_Free((void*)pVbi_Fld->pMCCData);
        pVbi_Fld->pMCCData = 0x0;
    }

    if (alloc)
    {
        pVbi_Fld->pMCCData = (BVBI_MCCData*)(BKNI_Malloc(sizeof(BVBI_MCCData)));
        if(!pVbi_Fld->pMCCData)
        {
            return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        }
    }
    else
    {
        if (pVbi_Fld->pMCCData)
        {
            BKNI_Free ((void*)(pVbi_Fld->pMCCData));
            pVbi_Fld->pMCCData = 0x0;
        }
    }

    BDBG_LEAVE(BVBI_Field_MCC_Allocate);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVBI_Field_SCTE_Allocate(
    BVBI_Field_Handle fieldHandle,
    size_t cc_size, bool scteEnableNrtv, size_t pam_size, bool scteEnableMono)
{
    BVBI_P_Field_Handle* pVbi_Fld;
    BERR_Code eStatus = BERR_SUCCESS;

    BDBG_ENTER(BVBI_Field_SCTE_Allocate);

    /* check parameters */
    pVbi_Fld = fieldHandle;
    BDBG_OBJECT_ASSERT (pVbi_Fld, BVBI_FIELD);

    /* If any data is requested */
    if (cc_size || scteEnableNrtv || pam_size || scteEnableMono)
    {
        /* Allocate the internal struct if necessary */
        if (!(pVbi_Fld->pPScteData))
        {
            pVbi_Fld->pPScteData = BKNI_Malloc (sizeof (BVBI_P_SCTE_Data));
            if (!(pVbi_Fld->pPScteData))
            {
                return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            }
            pVbi_Fld->pPScteData->field_number     = 0;
            pVbi_Fld->pPScteData->line_size        = 0;
            pVbi_Fld->pPScteData->nrtv_data[0]     = 0;
            pVbi_Fld->pPScteData->nrtv_data[1]     = 0;
            pVbi_Fld->pPScteData->pam_data_size    = 0;
            pVbi_Fld->pPScteData->mono_data[0]     = 0;
            pVbi_Fld->pPScteData->mono_data[1]     = 0;
        }
        /* Internal standard for SCTE data allocation */
        eStatus =
            BVBI_P_SCTEData_Alloc (
                pVbi_Fld->pVbi->hMmaHeap, cc_size, scteEnableNrtv, pam_size,
                scteEnableMono, pVbi_Fld->pPScteData);
        }
    else /* no data is requested */
    {
        /* If internal data has been allocated */
        if (pVbi_Fld->pPScteData)
        {
            /* Internal standard for SCTE data de-allocation */
            eStatus =
                BVBI_P_SCTEData_Alloc (
                    pVbi_Fld->pVbi->hMmaHeap, 0, false, 0, false,
                    pVbi_Fld->pPScteData);
            if (eStatus != BERR_SUCCESS)
            {
                return BERR_TRACE(eStatus);
            }
            /* Clear out the internal data structure itself */
            BKNI_Free ((void*)((pVbi_Fld->pPScteData)));
            pVbi_Fld->pPScteData = 0x0;
        }
    }

    BDBG_LEAVE(BVBI_Field_SCTE_Allocate);
    return eStatus;
}

/***************************************************************************
 *
 */
BERR_Code BVBI_Field_CGMSB_Allocate(
                BVBI_Field_Handle fieldHandle,
                bool bEnable)
{
    BVBI_P_Field_Handle* pVbi_Fld;

    BDBG_ENTER(BVBI_Field_CGMSB_Allocate);

    /* check parameters */
    pVbi_Fld = fieldHandle;
    BDBG_OBJECT_ASSERT (pVbi_Fld, BVBI_FIELD);

    /* Lose any CGMSB data that may exist already */
    if (pVbi_Fld->pCgmsbDatum)
    {
        BKNI_Free((void*)pVbi_Fld->pCgmsbDatum);
        pVbi_Fld->pCgmsbDatum = 0x0;
    }

    if (bEnable)
    {
        pVbi_Fld->pCgmsbDatum =
            (BVBI_CGMSB_Datum*)(BKNI_Malloc(sizeof(BVBI_CGMSB_Datum)));
        if(!pVbi_Fld->pCgmsbDatum)
        {
            return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        }
    }
    else
    {
        if (pVbi_Fld->pCgmsbDatum)
        {
            BKNI_Free ((void*)(pVbi_Fld->pCgmsbDatum));
            pVbi_Fld->pCgmsbDatum = 0x0;
        }
    }

    BDBG_LEAVE(BVBI_Field_CGMSB_Allocate);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVBI_Field_Destroy(BVBI_Field_Handle fieldHandle)
{
    BVBI_P_Field_Handle* pVbi_Fld;
    BVBI_P_TTData* pttData;

    BDBG_ENTER(BVBI_Field_Destroy);

    /* check parameters */
    pVbi_Fld = fieldHandle;
    BDBG_OBJECT_ASSERT (pVbi_Fld, BVBI_FIELD);

    /* Deallocate bulky data if necessary */
    BVBI_P_LCOP_DESTROY (pVbi_Fld, TTDataO, &pVbi_Fld->pVbi->ttFreelist, clink);
    pttData = BVBI_P_LCOP_GET (pVbi_Fld, TTDataO);
    BVBI_P_TTData_Alloc (pVbi_Fld->pVbi->hMmaHeap, 0, 0, pttData);
    BKNI_Free ((void*)pttData);
    BVBI_Field_MCC_Allocate (fieldHandle, false);
    BVBI_Field_AMOL_Allocate(fieldHandle, BVBI_AMOL_Type_None);
    BVBI_Field_GS_Allocate (fieldHandle, false);
    BVBI_Field_VPS_Allocate (fieldHandle, false);
    BVBI_Field_CGMSB_Allocate (fieldHandle, false);
    BVBI_Field_SCTE_Allocate (fieldHandle, 0, false, 0, false);

    /* Release context in system memory */
    BDBG_OBJECT_DESTROY (pVbi_Fld, BVBI_FIELD);
    BKNI_Free((void*)pVbi_Fld);

    BDBG_LEAVE(BVBI_Field_Destroy);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVBI_Field_SetPolarity_isr(
    BVBI_Field_Handle fieldHandle,
    uint32_t         polarityMask
)
{
    BVBI_P_Field_Handle *pVbi_Fld;

    BDBG_ENTER(BVBI_Field_SetPolarity_isr);

    /* check parameters */
    pVbi_Fld = fieldHandle;
    BDBG_OBJECT_ASSERT (pVbi_Fld, BVBI_FIELD);

    if ((polarityMask != 0x0                         ) &&
        (polarityMask != (1<<BAVC_Polarity_eTopField)) &&
        (polarityMask != (1<<BAVC_Polarity_eBotField)) &&
        (polarityMask !=
            ((1<<BAVC_Polarity_eTopField) | (1<<BAVC_Polarity_eBotField)))
       )
    {
        BDBG_ERR(("Invalid parameter"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Set attribute as requested */
    pVbi_Fld->polarityMask = polarityMask;

    BDBG_LEAVE(BVBI_Field_SetPolarity_isr);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVBI_Field_GetPolarity_isr(
    BVBI_Field_Handle fieldHandle,
    uint32_t *      pPolarityMask
)
{
    BVBI_P_Field_Handle *pVbi_Fld;

    BDBG_ENTER(BVBI_Field_GetPolarity_isr);

    /* check parameters */
    pVbi_Fld = fieldHandle;
    BDBG_OBJECT_ASSERT (pVbi_Fld, BVBI_FIELD);
    if(!pPolarityMask)
    {
        BDBG_ERR(("Invalid parameter"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Get attribute as requested */
    *pPolarityMask = pVbi_Fld->polarityMask;

    BDBG_LEAVE(BVBI_Field_GetPolarity_isr);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVBI_Field_GetErrorInfo_isr(
    BVBI_Field_Handle fieldHandle, uint32_t *pErrInfo )
{
    BVBI_P_Field_Handle *pVbi_Fld;

    BDBG_ENTER(BVBI_Field_GetErrorInfo_isr);

    /* check parameters */
    pVbi_Fld = fieldHandle;
    BDBG_OBJECT_ASSERT (pVbi_Fld, BVBI_FIELD);
    if(!pErrInfo)
    {
        BDBG_ERR(("Invalid parameter"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Return info as requested */
    *pErrInfo = pVbi_Fld->ulErrInfo;

    BDBG_LEAVE(BVBI_Field_GetErrorInfo_isr);
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVBI_Field_ClearCCData_isr (BVBI_Field_Handle fieldHandle)
{
    return BERR_TRACE (BVBI_P_Field_ClearFlagData_isr (
        fieldHandle, BVBI_P_SELECT_CC));
}
BERR_Code BVBI_Field_ClearTTData_isr (BVBI_Field_Handle fieldHandle)
{
    return BERR_TRACE (BVBI_P_Field_ClearFlagData_isr (
        fieldHandle, BVBI_P_SELECT_TT));
}
BERR_Code BVBI_Field_ClearCGMSAData_isr (BVBI_Field_Handle fieldHandle)
{
    return BERR_TRACE (BVBI_P_Field_ClearFlagData_isr (
        fieldHandle, BVBI_P_SELECT_CGMSA));
}
BERR_Code BVBI_Field_ClearCGMSBData_isr (BVBI_Field_Handle fieldHandle)
{
    return BERR_TRACE (BVBI_P_Field_ClearFlagData_isr (
        fieldHandle, BVBI_P_SELECT_CGMSB));
}
BERR_Code BVBI_Field_ClearWSSData_isr (BVBI_Field_Handle fieldHandle)
{
    return BERR_TRACE (BVBI_P_Field_ClearFlagData_isr (
        fieldHandle, BVBI_P_SELECT_WSS));
}
BERR_Code BVBI_Field_ClearVPSData_isr (BVBI_Field_Handle fieldHandle)
{
    return BERR_TRACE (BVBI_P_Field_ClearFlagData_isr (
        fieldHandle, BVBI_P_SELECT_VPS));
}
BERR_Code BVBI_Field_ClearGSData_isr (BVBI_Field_Handle fieldHandle)
{
    return BERR_TRACE (BVBI_P_Field_ClearFlagData_isr (
        fieldHandle, BVBI_P_SELECT_GS));
}
BERR_Code BVBI_Field_ClearAMOLData_isr (BVBI_Field_Handle fieldHandle)
{
    return BERR_TRACE (BVBI_P_Field_ClearFlagData_isr (
        fieldHandle, BVBI_P_SELECT_AMOL));
}
BERR_Code BVBI_Field_ClearMCCData_isr (BVBI_Field_Handle fieldHandle)
{
    return BERR_TRACE (BVBI_P_Field_ClearFlagData_isr (
        fieldHandle, BVBI_P_SELECT_MCC));
}
BERR_Code BVBI_Field_ClearSCTEData_isr (BVBI_Field_Handle fieldHandle)
{
    return BERR_TRACE (BVBI_P_Field_ClearFlagData_isr (
        fieldHandle, BVBI_P_SELECT_SCTE));
}
#endif

bool BVBI_Field_IsNull (BVBI_Field_Handle fieldHandle)
{
    bool retval = false;

    BDBG_ENTER(BVBI_Field_IsNull);
    BKNI_EnterCriticalSection();
    retval = BVBI_Field_IsNull_isr (fieldHandle);
    BKNI_LeaveCriticalSection();
    BDBG_ENTER(BVBI_Field_IsNull);
    return retval;
}
bool BVBI_Field_IsNull_isr (BVBI_Field_Handle fieldHandle)
{
    BVBI_P_Field_Handle *pVbi_Fld;
    bool retval = false;

    BDBG_ENTER(BVBI_Field_IsNull_isr);

    /* Check parameter */
    pVbi_Fld = fieldHandle;
    BDBG_OBJECT_ASSERT (pVbi_Fld, BVBI_FIELD);

    /* Only closed caption data can be present */
    if ((pVbi_Fld->ulWhichPresent & ~BVBI_P_SELECT_CC) != 0x0)
    {
        goto done;
    }
    if ((pVbi_Fld->ulWhichPresent & BVBI_P_SELECT_CC) == 0x0)
    {
        retval = true;
        goto done;
    }

    /* Only null closed caption data can be present */
    if (pVbi_Fld->usCCData == 0x8080)
    {
        retval = true;
    }

done:
    BDBG_LEAVE(BVBI_Field_IsNull_isr);
    return retval;
}

/***************************************************************************
* Static (private) functions
***************************************************************************/

/***************************************************************************
 *
 */
#if !B_REFSW_MINIMAL
static BERR_Code
BVBI_P_Field_ClearFlagData_isr (
    BVBI_Field_Handle fieldHandle,
    uint32_t flagBit)
{
    BVBI_P_Field_Handle *pVbi_Fld;

    BDBG_ENTER(BVBI_P_Field_ClearFlagData_isr);

    /* check parameter */
    pVbi_Fld = fieldHandle;
    BDBG_OBJECT_ASSERT (pVbi_Fld, BVBI_FIELD);

    /* Clear the data as requested */
    pVbi_Fld->ulWhichPresent &= ~flagBit;

    BDBG_LEAVE(BVBI_P_Field_ClearFlagData_isr);
    return BERR_SUCCESS;
}
#endif


#if 0
/***************************************************************************
 *
 */
BERR_Code BVBI_P_Field_Dump( BVBI_Field_Handle fieldHandle,
                             BAVC_Polarity polarity,
                             char* printme, int printmetoo )
{
    BVBI_P_Field_Handle* pVbi_Fld;
    char* polarityString;
    uint8_t* pucData;
    uint32_t mask;
    char* p1;
    char* p2;

    BDBG_ENTER(BVBI_P_Field_Dump);

    /* check parameters */
    pVbi_Fld = fieldHandle;
    BDBG_OBJECT_ASSERT (pVbi_Fld, BVBI_FIELD);

    switch (polarity)
    {
    case BAVC_Polarity_eTopField:
        polarityString = "Top";
        break;
    case BAVC_Polarity_eBotField:
        polarityString = "Bottom";
        break;
    case BAVC_Polarity_eFrame:
        polarityString = "Frame";
        break;
    default:
        return BERR_TRACE(BERR_INVALID_PARAMETER);
        break;
    }

    pucData = pVbi_Fld->TTDataO.pucData;
    mask = *(uint32_t*)pucData;
    p1 = pucData + 4;
    p2 = pucData + (4 + 34);
    if (mask)
    {
        printf ("%s %d\n", printme, printmetoo);
        printf ("Polarity %s, fpm %x\n",
            polarityString, pVbi_Fld->polarityMask);
        printf ("Data %08x  \"%s\"  \"%s\"\n", mask, p1, p2);
    }

    BDBG_LEAVE(BVBI_P_Field_Dump);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
void BVBI_Field_Check(BVBI_Field_Handle fieldHandle)
{
    BVBI_P_Field_Handle* pVbi_Fld;

    BDBG_ENTER(BVBI_P_Field_Check);

    /* check parameters */
    pVbi_Fld = fieldHandle;
    BDBG_OBJECT_ASSERT (pVbi_Fld, BVBI_FIELD);
    BDBG_ASSERT (pVbi_Fld);
    BDBG_ASSERT (pVbi_Fld->TTDataO.ucDataSize);
    BDBG_ASSERT (pVbi_Fld->TTDataO.pucData);

    BDBG_LEAVE(BVBI_P_Field_Check);
}
#endif /* 0 */

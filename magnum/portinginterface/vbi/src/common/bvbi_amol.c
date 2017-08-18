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

#include "bstd.h"           /* standard types */
#include "bdbg.h"           /* Dbglib */
#include "bkni.h"           /* For critical sections */
#include "bvbi.h"           /* VBI processing, this module. */
#include "bvbi_cap.h"
#include "bvbi_priv.h"      /* VBI internal data structures */

BDBG_MODULE(BVBI);

/***************************************************************************
* Private data
***************************************************************************/

/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/

static int P_size_by_type_isr (BVBI_AMOL_Type type);


/***************************************************************************
* Implementation of "BVBI_" API functions
***************************************************************************/


#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVBI_Field_GetAMOLData_isr (
    BVBI_Field_Handle fieldHandle,
    BVBI_AMOL_Type      *pAmolType,
    uint8_t            *pAMOLData,
    unsigned int*         pLength
)
{
    int count;
    int size;
    uint8_t* fdata;
    BVBI_P_Field_Handle* pVbi_Fld;
    BERR_Code eErr = BERR_SUCCESS;

    BDBG_ENTER(BVBI_Field_GetAMOLData_isr);

    /* check parameters */
    pVbi_Fld = fieldHandle;
    BDBG_OBJECT_ASSERT (pVbi_Fld, BVBI_FIELD);
    if((!pAMOLData) || (!pAmolType) || (!pLength))
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Verify that data is present on this field handle */
    if (!(pVbi_Fld->ulWhichPresent & BVBI_P_SELECT_AMOL))
        return (BVBI_ERR_FIELD_NODATA);
    else if (pVbi_Fld->ulErrInfo & BVBI_LINE_ERROR_AMOL_NOENCODE)
        eErr = (BVBI_ERR_FIELD_BADDATA);

    /* Check that field handle was properly sized */
    fdata = pVbi_Fld->pAmolData;
    if (!fdata)
    {
        return BERR_TRACE (BVBI_ERR_FLDH_CONFLICT);
    }

    /* Return data as requested */
    BDBG_ASSERT (pVbi_Fld->amolType != BVBI_AMOL_Type_None);
    size = P_size_by_type_isr (pVbi_Fld->amolType);
    BDBG_ASSERT (size >= 0);
    *pAmolType = pVbi_Fld->amolType;
    for (count = 0 ; count < size ; ++count)
        *pAMOLData++ = *fdata++;
    *pLength = P_size_by_type_isr (*pAmolType);

    BDBG_LEAVE(BVBI_Field_GetAMOLData_isr);
    return eErr;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVBI_Field_SetAMOLData_isr(
    BVBI_Field_Handle fieldHandle,
    BVBI_AMOL_Type        amolType,
    uint8_t             *pAMOLData,
    unsigned int            length
)
{
    int count;
    int size;
    BVBI_P_Field_Handle* pVbi_Fld;

    BDBG_ENTER(BVBI_Field_SetAMOLData_isr);

    /* check parameters */
    pVbi_Fld = fieldHandle;
    BDBG_OBJECT_ASSERT (pVbi_Fld, BVBI_FIELD);
    if(!pAMOLData)
    {
        BDBG_ERR(("Invalid parameter"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    /* TODO: Check amolType more carefully? */
    if (amolType == BVBI_AMOL_Type_None)
    {
        BDBG_ERR(("Invalid parameter"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    size = P_size_by_type_isr (amolType);
    if (size < 0)
    {
        BDBG_ERR(("Invalid parameter"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if ((unsigned int)size > length)
    {
        BDBG_ERR(("Invalid parameter"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Check that field handle was properly sized */
    if (!pVbi_Fld->pAmolData)
    {
        return BERR_TRACE (BVBI_ERR_FLDH_CONFLICT);
    }
    if (pVbi_Fld->amolSize < size)
    {
        return BERR_TRACE (BVBI_ERR_FLDH_CONFLICT);
    }

    /* Store data as requested */
    pVbi_Fld->amolType = amolType;
    for (count = 0 ; count < size ; ++count)
        pVbi_Fld->pAmolData[count] = *pAMOLData++;

    /* Indicate valid data is present */
    pVbi_Fld->ulWhichPresent |= BVBI_P_SELECT_AMOL;

    BDBG_LEAVE(BVBI_Field_SetAMOLData_isr);
    return BERR_SUCCESS;
}


/***************************************************************************
* Implementation of supporting AMOL functions that are not in API
***************************************************************************/

BERR_Code BVBI_P_AMOL_Init( BVBI_P_Handle *pVbi )
{
#if (BVBI_NUM_AMOLE > 0) || (BVBI_NUM_AMOLE_656 > 0)
    uint8_t hwIndex;
#endif

    BDBG_ENTER(BVBI_P_AMOL_Init);

#if (BVBI_NUM_AMOLE == 0) && (BVBI_NUM_AMOLE_656 == 0)
    BSTD_UNUSED (pVbi);
#endif

#if (BVBI_NUM_AMOLE > 0)
    for (hwIndex = 0 ; hwIndex < BVBI_NUM_AMOLE ; ++hwIndex)
        BVBI_P_AMOL_Enc_Init (pVbi->hReg, hwIndex);
#endif
#if (BVBI_NUM_AMOLE_656 > 0)
    for (hwIndex = 0 ; hwIndex < BVBI_NUM_AMOLE_656 ; ++hwIndex)
        BVBI_P_AMOL_Enc_656_Init (pVbi->hReg, hwIndex);
#endif

    BDBG_LEAVE(BVBI_P_AMOL_Init);
    return BERR_SUCCESS;
}


/***************************************************************************
* Static (private) functions
***************************************************************************/

/***************************************************************************
 *
 */
static int P_size_by_type_isr (BVBI_AMOL_Type type)
{
    int size;

    switch (type)
    {
    case BVBI_AMOL_Type_None:
        size = 0;
        break;
    case BVBI_AMOL_Type_I:
        size = 6;
        break;
    case BVBI_AMOL_Type_II_Lowrate:
        size = 12;
        break;
    case BVBI_AMOL_Type_II_Highrate:
        size = 24;
        break;
    default:
        size = -1;
        break;
    }

    return size;
}

/* End of file */

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

#include "bstd.h"             /* standard types */
#include "bdbg.h"             /* Dbglib */
#include "bkni.h"             /* For critical sections */
#include "bvbi.h"             /* VBI processing, this module. */
#include "bvbi_cap.h"
#include "bvbi_priv.h"        /* VBI internal data structures */

BDBG_MODULE(BVBI);


/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/


/***************************************************************************
* Implementation of "BVBI_" API functions
***************************************************************************/

/***************************************************************************
 *
 */
BERR_Code
BVBI_Field_GetCGMSAData_isr( BVBI_Field_Handle vbiData, uint32_t *pulCGMSData )
{
    BVBI_P_Field_Handle* pVbi_Fld;
    BERR_Code eErr = BERR_SUCCESS;

    BDBG_ENTER(BVBI_Field_GetCGMSAData_isr);

    /* check parameters */
    pVbi_Fld = vbiData;
    BDBG_OBJECT_ASSERT (pVbi_Fld, BVBI_FIELD);

    /* Verify that data is present on this field handle */
    if (!(pVbi_Fld->ulWhichPresent & BVBI_P_SELECT_CGMSA))
        return (BVBI_ERR_FIELD_NODATA);
    else if (pVbi_Fld->ulErrInfo & BVBI_LINE_ERROR_CGMS_NOENCODE)
        eErr = (BVBI_ERR_FIELD_BADDATA);

    /* Return data as requested */
    *pulCGMSData = pVbi_Fld->ulCGMSData;
    BDBG_LEAVE(BVBI_Field_GetCGMSAData_isr);
    return eErr;
}

/***************************************************************************
 *
 */
BERR_Code BVBI_Field_SetCGMSAData_isr(
    BVBI_Field_Handle vbiData, uint32_t ulCGMSData )
{
    BVBI_P_Field_Handle* pVbi_Fld;

    BDBG_ENTER(BVBI_Field_SetCGMSAData_isr);

    /* check parameters */
    pVbi_Fld = vbiData;
    BDBG_OBJECT_ASSERT (pVbi_Fld, BVBI_FIELD);

    /* Store data as requested */
    pVbi_Fld->ulCGMSData = ulCGMSData;

    /* Indicate valid data is present */
    pVbi_Fld->ulWhichPresent |= BVBI_P_SELECT_CGMSA;

    BDBG_LEAVE(BVBI_Field_SetCGMSAData_isr);
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code
BVBI_Field_GetCGMSBData_isr(
    BVBI_Field_Handle vbiData,  BVBI_CGMSB_Datum* pDatum)
{
    int subIndex;
    BVBI_P_Field_Handle* pVbi_Fld;
    BERR_Code eErr = BERR_SUCCESS;

    BDBG_ENTER(BVBI_Field_GetCGMSBData_isr);

    /* check parameters */
    pVbi_Fld = vbiData;
    BDBG_OBJECT_ASSERT (pVbi_Fld, BVBI_FIELD);

    /* Verify that data is present on this field handle */
    if (!(pVbi_Fld->ulWhichPresent & BVBI_P_SELECT_CGMSB))
        return (BVBI_ERR_FIELD_NODATA);
    else if (pVbi_Fld->ulErrInfo & BVBI_LINE_ERROR_CGMS_NOENCODE)
        eErr = (BVBI_ERR_FIELD_BADDATA);

    /* Check that field handle was properly sized */
    if (!pVbi_Fld->pCgmsbDatum)
    {
        return BERR_TRACE (BVBI_ERR_FLDH_CONFLICT);
    }

    /* Return data as requested */
    for (subIndex = 0 ; subIndex < 5 ; ++subIndex)
        (*pDatum)[subIndex] = (*pVbi_Fld->pCgmsbDatum)[subIndex];
    BDBG_LEAVE(BVBI_Field_GetCGMSBData_isr);
    return eErr;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVBI_Field_SetCGMSBData_isr(
    BVBI_Field_Handle vbiData, BVBI_CGMSB_Datum* pDatum)
{
    BVBI_P_Field_Handle* pVbi_Fld;
    int subIndex;

    BDBG_ENTER(BVBI_Field_SetCGMSBData_isr);

    /* check parameters */
    pVbi_Fld = vbiData;
    BDBG_OBJECT_ASSERT (pVbi_Fld, BVBI_FIELD);

    /* Check that field handle was properly sized */
    if (!pVbi_Fld->pCgmsbDatum)
    {
        return BERR_TRACE (BVBI_ERR_FLDH_CONFLICT);
    }

    /* Store data as requested */
    for (subIndex = 0 ; subIndex < 5 ; ++subIndex)
        (*pVbi_Fld->pCgmsbDatum)[subIndex] = (*pDatum)[subIndex];

    /* Indicate valid data is present */
    pVbi_Fld->ulWhichPresent |= BVBI_P_SELECT_CGMSB;

    BDBG_LEAVE(BVBI_Field_SetCGMSBData_isr);
    return BERR_SUCCESS;
}


/***************************************************************************
* Implementation of supporting CGMS functions that are not in API
***************************************************************************/

#if BVBI_NUM_CGMSAE >= 1
BERR_Code BVBI_P_CGMS_Init( BVBI_P_Handle *pVbi )
{
    uint8_t hwIndex;

    BDBG_ENTER(BVBI_P_CGMS_Init);

    /* Initialize CGMS encoders */
    for (hwIndex = 0 ; hwIndex < BVBI_NUM_CGMSAE ; ++hwIndex)
        BVBI_P_CGMS_Enc_Init (pVbi->hReg, hwIndex);

    BDBG_LEAVE(BVBI_P_CGMS_Init);
    return BERR_SUCCESS;
}
#endif

uint32_t BVPI_P_CGMS_format_data_isr (uint32_t userdata)
{
    /* Do CRC calculation if hardware is broken */
    /* Otherwise, just transform from decoder format to encoder format. */
    userdata <<= 2;
    userdata &= 0x0000fffc;

    return userdata;
}


/***************************************************************************
* Static (private) functions
***************************************************************************/


/* End of file */

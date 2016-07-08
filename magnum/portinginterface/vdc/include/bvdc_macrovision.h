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
 *   Header file for Macrovision support
 *
 ***************************************************************************/
#ifndef BVDC_MACROVISION_H__
#define BVDC_MACROVISION_H__

#ifdef __cplusplus
extern "C" {
#endif

#define BVDC_CPC_COUNT     2
#define BVDC_CPS_COUNT     33
typedef uint8_t      BVDC_CpcTable[BVDC_CPC_COUNT];
typedef uint8_t      BVDC_CpsTable[BVDC_CPS_COUNT];


/***************************************************************************
Summary:
    This function sets the Macrovision type

Description:
    Sets the macrovision type associated with a Display handle.
    Returns an error if the macrovision type is invalid, or the display
    output does not support macrovision. 656, and DVI do not support
    Macrovision.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hDisplay - Display handle created earlier with BVDC_Display_Create.
    eMacrovisionType - macrovision type

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_GetMacrovisionType
    Note that HDCP, content protection for DVI is supported in the DVI PI.
**************************************************************************/
BERR_Code BVDC_Display_SetMacrovisionType
    ( BVDC_Display_Handle              hDisplay,
      BVDC_MacrovisionType             eMacrovisionType );

/***************************************************************************
Summary:
    This function queries the Macrovision type applied

Description:
    Returns the macrovision type associated with a Display
    handle.

Input:
    hDisplay - Display handle created earlier with BVDC_Display_Create.

Output:
    peMacrovisionType - pointer to macrovision type

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_SetMacrovisionType
**************************************************************************/
BERR_Code BVDC_Display_GetMacrovisionType
    ( const BVDC_Display_Handle        hDisplay,
      BVDC_MacrovisionType            *peMacrovisionType );

/***************************************************************************
Summary:
    Provide custom Macrovision CPC/CPS values to use, instead of the
    pre-defined Macrovision types.

Description:
    This function programs the Macrovision settings with the CPC/CPS
    provided by the user. Applications are required to call
    BVDC_Display_SetMacrovisionType with BVDC_MacrovisionType_eCustomized.

Input:
    hDisplay   - Display handle
    pCpcTable  - pointer to CPC table (CPC0,CPC1)
    pCpsTable  - pointer to CPS table (CPS0..CPS32)

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_SetMacrovisionType
**************************************************************************/
BERR_Code BVDC_Display_SetMacrovisionTable
    ( BVDC_Display_Handle            hDisplay,
      const BVDC_CpcTable            pCpcTable,
      const BVDC_CpsTable            pCpsTable );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_MACROVISION_H__ */
/* End of file. */

/***************************************************************************
 *     Copyright (c) 2003-2008, Broadcom Corporation
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
 * Module Description:
 *   Header file for Macrovision support
 *
 * Revision History:
 *
 * $brcm_Log: $
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
	Sets the macrovision type associated with a Display	handle. 
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

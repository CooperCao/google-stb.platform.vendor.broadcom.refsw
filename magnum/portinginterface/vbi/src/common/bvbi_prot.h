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
 *   This piece of the BVBI porting interface implements a "usage count"
 *   property of the BVBI field handle.  The functions in this file will 
 *   enforce the requiremente that the usage count must be non-negative at all
 *   times.  Note that the usage count is used extensively by the BVBIlib
 *   syslib.  Therefore, users who access the usage count property themselves
 *   should NOT use BVBIlib.
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BVBI_PROT_H__
#define BVBI_PROT_H__

#include "bvbi.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
	Note:  These functions manipulate the "usage count" attribute of a
	----   BVBI_Field_Handle.  This attribute is used heavily in the syslib
		   module BVBIlib.  Therefore, these functions should NOT be used by
		   most users.  
*/


/*****************************************************************************
  Summary:
    Sets the "usage count" property of a field handle to zero.

  Description:
    This function sets the usage count property of a field handle to zero.
	Note that the usage count property is used extensively by the BVBIlib
	syslib.  Therefore, if a user modifies this property directly, BVBIlib
	should NOT be used.
 *****************************************************************************/
void BVBI_Field_Zero_UsageCount_isr      (
	BVBI_Field_Handle fieldHandle 	/* [in] A valid BVBI_Field_Handle object */
);

/*****************************************************************************
  Summary:
    Increments the "usage count" property of a field handle.

  Description:
    This function increments the usage count property of a field handle.
	Note that the usage count property is used extensively by the BVBIlib
	syslib.  Therefore, if a user modifies this property directly, BVBIlib
	should NOT be used.
 *****************************************************************************/
void BVBI_Field_Increment_UsageCount_isr (
	BVBI_Field_Handle fieldHandle 	/* [in] A valid BVBI_Field_Handle object */
);

/*****************************************************************************
  Summary:
    Decrements the "usage count" property of a field handle.

  Description:
    This function decrements the usage count property of a field handle.
	Note that the usage count property is used extensively by the BVBIlib
	syslib.  Therefore, if a user modifies this property directly, BVBIlib
	should NOT be used.
 *****************************************************************************/
void BVBI_Field_Decrement_UsageCount_isr (
	BVBI_Field_Handle fieldHandle 	/* [in] A valid BVBI_Field_Handle object */
);

/*****************************************************************************
  Summary:
    Obtains the "usage count" property of a field handle.

  Description:
    This function returns the usage count property of a field handle.
	Note that the usage count property is used extensively by the BVBIlib
	syslib.  Therefore, if a user modifies this property directly, BVBIlib
	should NOT be used.
  
  Returns:
    The usage count of the field handle.
 *****************************************************************************/
int  BVBI_Field_Get_UsageCount_isr       (
	BVBI_Field_Handle fieldHandle 	/* [in] A valid BVBI_Field_Handle object */
);


/*****************************************************************************
  Summary:
    Prepares a field handle for re-use by clearing out attributes.

  Description:
	This function resets these attributes of a field handle:
	 - What VBI data is contained and valid.
	 - What error conditions have been encountered.
	 - What field polarities (top, bottom, both) are valid for the field
	   handle.
	This function does not change the "usage count" property of the field 
	handle.

  See Also:
    BVBI_Field_Zero_UsageCount_isr
 *****************************************************************************/
void BVBI_Field_ClearState_isr (
	BVBI_Field_Handle fieldHandle 	/* [in] A valid BVBI_Field_Handle object */
);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVBI_PROT_H__ */

/* End of file. */

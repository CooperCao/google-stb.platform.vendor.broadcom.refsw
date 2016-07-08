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
 *   This piece of the BVBI porting interface implements a "usage count"
 *   property of the BVBI field handle.  The functions in this file will
 *   enforce the requiremente that the usage count must be non-negative at all
 *   times.  Note that the usage count is used extensively by the BVBIlib
 *   syslib.  Therefore, users who access the usage count property themselves
 *   should NOT use BVBIlib.
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
    BVBI_Field_Handle fieldHandle   /* [in] A valid BVBI_Field_Handle object */
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
    BVBI_Field_Handle fieldHandle   /* [in] A valid BVBI_Field_Handle object */
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
    BVBI_Field_Handle fieldHandle   /* [in] A valid BVBI_Field_Handle object */
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
    BVBI_Field_Handle fieldHandle   /* [in] A valid BVBI_Field_Handle object */
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
    BVBI_Field_Handle fieldHandle   /* [in] A valid BVBI_Field_Handle object */
);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVBI_PROT_H__ */

/* End of file. */

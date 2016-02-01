/***************************************************************************
 *     Copyright (c) 2006-2011, Broadcom Corporation
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
 *   See Module Overview below.
 *
 * Revision History:
 *
  *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BCHP_7125_H__
#define BCHP_7125_H__

#include "bstd.h"
#include "bchp.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Chip Id */
#define BCHP_BCM7125                 0x7125
#define BCHP_BCM7119                 0x7119
#define BCHP_BCM7019                 0x7019
#define BCHP_BCM7025                 0x7025
#define BCHP_BCM7116                 0x7116
#define BCHP_BCM7124                 0x7124

/* Major revision */
#define BCHP_MAJOR_A                  (0)
#define BCHP_MAJOR_B                  (1)
#define BCHP_MAJOR_C                  (2)
#define BCHP_MAJOR_D                  (3)
#define BCHP_MAJOR_E                  (4)

/* Minor revision */
#define BCHP_MINOR_0                  (0)
#define BCHP_MINOR_1                  (1)
#define BCHP_MINOR_2                  (2)
#define BCHP_MINOR_3                  (3)
#define BCHP_MINOR_4                  (4)
#define BCHP_MINOR_5                  (5)

/***************************************************************************
Summary:
    Open a chip handle for Bcm7125 chip

Description:
    This function open a chip handle for Bcm7125 chip.

Input:
    hRegister - A valid Bcm7125 register handle previous open using BREG_Open.

Output:
    phChip - Upon successful completion of open this pointer is non NULL
    and contains valid information about this chip.  This handle is used
    on subsequences BCHP_??? API.  *phChip is NULL if failure.

See Also:
    BCHP_Close

**************************************************************************/
BERR_Code BCHP_Open7125
    ( BCHP_Handle                     *phChip,
      BREG_Handle                      hRegister );


#ifdef __cplusplus
}
#endif

#endif /* BCHP_7125_H__ */

/* End of File */

/***************************************************************************
 *     Copyright (c) 2006-2012, Broadcom Corporation
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
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BCHP_7552_H__
#define BCHP_7552_H__

#include "bstd.h"
#include "bchp.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Chip Id */
#define BCHP_BCM7531                  0x7531
#define BCHP_BCM7532                  0x7532
#define BCHP_BCM7541                  0x7541
#define BCHP_BCM7542                  0x7542
#define BCHP_BCM7551                  0x7551
#define BCHP_BCM7552                  0x7552
#define BCHP_BCM7561                  0x7561
#define BCHP_BCM7562                  0x7562
#define BCHP_BCM7574                  0x7574
#define BCHP_BCM7581                  0x7581
#define BCHP_BCM7582                  0x7582
#define BCHP_BCM7591                  0x7591
#define BCHP_BCM7592                  0x7592
#define BCHP_BCM7023                  0x7023

/* Major revision */
#define BCHP_MAJOR_A                  (0x0)
#define BCHP_MAJOR_B                  (0x1)
#define BCHP_MAJOR_C                  (0x2)
#define BCHP_MAJOR_D                  (0x3)

/* Minor revision */
#define BCHP_MINOR_0                  (0)
#define BCHP_MINOR_1                  (1)
#define BCHP_MINOR_2                  (2)
#define BCHP_MINOR_3                  (3)
#define BCHP_MINOR_4                  (4)
#define BCHP_MINOR_5                  (5)

/***************************************************************************
Summary:
    Open a chip handle for Bcm7552 chip

Description:
    This function open a chip handle for Bcm7552 chip.

Input:
    hRegister - A valid Bcm7552 register handle previous open using BREG_Open.

Output:
    phChip - Upon successful completion of open this pointer is non NULL
    and contains valid information about this chip.  This handle is used
    on subsequences BCHP_??? API.  *phChip is NULL if failure.

See Also:
    BCHP_Close

**************************************************************************/
BERR_Code BCHP_Open7552
    ( BCHP_Handle                     *phChip,
      BREG_Handle                      hRegister );

/***************************************************************************
 *  Be called in bint_7552.c since bchp handle can not pass to bint module
 */
uint16_t BCHP_GetChipID (void);

#ifdef __cplusplus
}
#endif

#endif /* BCHP_7552_H__ */

/* End of File */

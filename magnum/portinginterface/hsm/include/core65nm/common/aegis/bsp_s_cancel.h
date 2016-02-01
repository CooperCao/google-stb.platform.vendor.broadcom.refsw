/***************************************************************************
 *     Copyright (c) 2005-2009, Broadcom Corporation
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
 * Security Processor Code for Register Access
 ***************************************************************************/

 #ifndef BSP_S_CANCEL_H__
#define BSP_S_CANCEL_H__

/*

BCMD_Cancel_ExecuteCancelCommand - request to cancel an RSA, DSA, or DH
	operation.

Returned:
eStatus - Status of cancel command.

    
BCMD_Cancel_OutputCommandField_e: 
|-------------------|-------------------|-------------------|-------------------| 
|  31..24           | 23..16            | 15..8             | 7..0             	| 
|-------------------|-------------------|-------------------|-------------------| 
|  unused			|  unused			|  unused			| eStatus -	8		|
|-------------------|-------------------|-------------------|-------------------| 

 */

typedef enum BCMD_Cancel_OutputCommandField_e
{
    BCMD_Cancel_OutputCommandField_eStatus = (5<<2) + 3 /* 8 bit index */
}BCMD_Cancel_OutputCommandField_e;

#endif /* BSP_S_CANCEL_H__ */


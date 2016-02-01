/***************************************************************************
 *     Copyright (c) 2002-2011, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: berr.h $
 * $brcm_Revision: Hydra_Software_Devel/1 $
 * $brcm_Date: 2/11/11 3:35p $
 *
 * Module Description:
 *
 * THIS IS A STUB of berr.h. This is only used for the xvd_save_FW_Image program.
 *
 * Revision History:
 *
 * $brcm_Log: /rockford/applications/xvd_save_FW_image/berr.h $
 *
 * Hydra_Software_Devel/1   2/11/11 3:35p davidp
 * SW7422-22: Initial checkin for xvd_save_image FW signing tool.
 *
 ****************************************************************************/

typedef int BERR_Code;

/* standard error codes */

#define BERR_SUCCESS              0  /* success (always zero) */
#define BERR_NOT_INITIALIZED      1  /* parameter not initialized */
#define BERR_INVALID_PARAMETER    2  /* parameter is invalid */
#define BERR_OUT_OF_SYSTEM_MEMORY 3  /* out of KNI module memory */
#define BERR_OUT_OF_DEVICE_MEMORY 4  /* out of MEM module memory */
#define BERR_TIMEOUT              5  /* reached timeout limit */
#define BERR_OS_ERROR             6  /* generic OS error */
#define BERR_LEAKED_RESOURCE      7  /* resource being freed has attached
                                        resources that haven't been freed */
#define BERR_NOT_SUPPORTED        8  /* requested feature is not supported */
#define BERR_UNKNOWN              9  /* unknown */

#define BERR_TRACE(x) (x)

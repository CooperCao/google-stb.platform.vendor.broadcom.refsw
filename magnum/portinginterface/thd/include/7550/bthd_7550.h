/***************************************************************************
 *     Copyright (c) 2005-2012, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef _BTHD_7550_H__
#define _BTHD_7550_H__           

#include "bthd.h"

#ifdef __cplusplus
extern "C" {
#endif
  
/***************************************************************************
 * Summary:
 *   This function returns the default settings for 7550 THD module.
 *
 * Description:
 *   This function is responsible for returns the default setting for 
 *   7550 THD module. The returning default setting should be used when
 *   opening the device.
 *
 * Returns:
 *   TODO:
 *  
 * See Also:
 *   None.
 *   
 ***************************************************************************/
BERR_Code BTHD_7550_GetDefaultSettings(BTHD_Settings *);
  
/***************************************************************************
 * Summary:
 *   This function returns the default inbandParams for 7550 THD module.
 *
 * Description:
 *   This function is responsible for returns the default setting for 
 *   7550 THD module. The returning default setting should be used when
 *   opening the device.
 *
 * Returns:
 *   TODO:
 *   
 * See Also:
 *   None.
 *     
 ***************************************************************************/
  BERR_Code BTHD_7550_GetDefaultInbandParams(BTHD_InbandParams *);
  

#ifdef __cplusplus
}
#endif

#endif /* BTHD_7550_H__ */


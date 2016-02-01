/***************************************************************************
 *    Copyright (c) 2004-2010, Broadcom Corporation
 *    All Rights Reserved
 *    Confidential Property of Broadcom Corporation
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
 *	 This module contains the definitions and prototypes for the XVD FW image
 *   interface.
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************************/
#ifndef BXVD_IMAGE_H__
#define BXVD_IMAGE_H__

#include "bimg.h"
#include "bxvd_image_header.h"

#ifdef __cplusplus
extern "C" {
#endif

/* FW image IDs used in the BXVD_IMAGE_Open function. The AVD0 IDs are used
 * for all architectures. Designs with more than 1 decoder can add image IDs
 *  here.
 */
typedef enum BXVD_IMAGE_FirmwareID
{       
        BXVD_IMAGE_FirmwareID_eOuterELF_AVD0 = 0,  /* AVD0 Outer ELF firmware image*/        
        BXVD_IMAGE_FirmwareID_eInnerELF_AVD0,      /* AVD0 Inner ELF firmware image */
        BXVD_IMAGE_FirmwareID_eOuterELF_AVD1,      /* AVD1 Outer ELF firmware image*/        
        BXVD_IMAGE_FirmwareID_eInnerELF_AVD1,      /* AVD1 Inner ELF firmware image */        
	
	BXVD_IMAGE_FirmwareID_eAuthenticated_AVD0, /* AVD0 Authenticated firmware image */
	BXVD_IMAGE_FirmwareID_eAuthenticated_AVD1, /* AVD1 Authenticated firmware image */

        /* Add additional image IDs ABOVE this line */
        BXVD_IMAGE_FirmwareID_Max
} BXVD_IMAGE_FirmwareID;

/* AVD/AVD core Rev K FW image Ids */
typedef enum BXVD_IMAGE_RevK_FirmwareID
{       
        BXVD_IMAGE_RevK_FirmwareID_eOuterELF_AVD = 0,  /* AVD Outer ELF firmware image*/        
        BXVD_IMAGE_RevK_FirmwareID_eInnerELF_AVD,      /* AVD Inner ELF firmware image */
        BXVD_IMAGE_RevK_FirmwareID_eBaseELF_SVD,      /* SVD BASE ELF firmware image */
        BXVD_IMAGE_RevK_FirmwareID_Max
} BXVD_IMAGE_RevK_FirmwareID;

extern const void* const BXVD_IMAGE_Context;
extern const BIMG_Interface BXVD_IMAGE_Interface;

#ifdef __cplusplus
}
#endif

#endif /* BXVD_IMAGE_H__ */
/* End of file. */
 



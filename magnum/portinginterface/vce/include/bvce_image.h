/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
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

#ifndef BVCE_IMAGE_H_
#define BVCE_IMAGE_H_

#ifdef __cplusplus
extern "C" {
#if 0
}
#endif
#endif

/* FW image IDs used in the BVCE_IMAGE_Open function. The AVD0 IDs are used
 * for all architectures. Designs with more than 1 decoder can add image IDs
 *  here.
 */
typedef enum BVCE_IMAGE_FirmwareID
{
        BVCE_IMAGE_FirmwareID_ePicArc = 0,     /* PicArc ELF firmware image */
        BVCE_IMAGE_FirmwareID_eMBArc,          /* MBArc ELF firmware image */

        /* Add additional image IDs ABOVE this line */
        BVCE_IMAGE_FirmwareID_Max
} BVCE_IMAGE_FirmwareID;

extern const void* const BVCE_IMAGE_Context;
extern const BIMG_Interface BVCE_IMAGE_Interface;

#ifdef __cplusplus
}
#endif

#endif /* BVCE_IMAGE_H_ */

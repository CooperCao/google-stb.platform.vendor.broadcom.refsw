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

#ifndef BVCE_IMAGE_PRIV_H_
#define BVCE_IMAGE_PRIV_H_

#include "bafl.h"

#ifdef __cplusplus
extern "C" {
#if 0
}
#endif
#endif

typedef struct BVCE_IMAGE_ContextEntry
{
        const unsigned int *puiImageSize;
        const void *pImageData;
} BVCE_IMAGE_ContextEntry;

typedef struct BVCE_IMAGE_Container
{
        size_t uiImageSize;
        const void *pImageData;
        BAFL_ImageHeader stImageHeader;
        BVCE_IMAGE_FirmwareID uiImageId;
} BVCE_IMAGE_Container;

#ifdef __cplusplus
}
#endif

#endif /* BVCE_IMAGE_PRIV_H_ */

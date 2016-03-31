/***************************************************************************
 *     Copyright (c) 2009, Broadcom Corporation
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
 *    Reference JPEG Decode into YUYV or RGB Color spaces with slideshow
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *************************************************************************/
#ifndef B_SOFT_JPEG_H__
#define B_SOFT_JPEG_H__

#include "nexus_surface.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
This is a simple wrapper around libjpeg and an integration with the NEXUS_Surface interface.
**/

NEXUS_SurfaceHandle b_decompress_jpeg(
    const char *filename,
    unsigned int maxWidth,
    unsigned int maxHeight
    );

#ifdef __cplusplus
}
#endif

#endif

/***************************************************************************
 *     Copyright (c) 2006-2009, Broadcom Corporation
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

#ifndef BXVD_IMAGE_PRIV_H__
#define BXVD_IMAGE_PRIV_H__

#if BXVD_P_AVD_ARC600
#include "bafl.h"

#define BXVD_P_OL 0x5856444F  /* XVDO */
#define BXVD_P_IL 0x58564449  /* XVDI */
#define BXVD_P_BL 0x58564442  /* XVDB */
#endif

#if BXVD_P_USE_BINARY_IMAGE

#include "stdio.h"

#define BXVD_P_BLK_SIZE (1024*16)

typedef struct BXVD_IMAGE_ContextEntry
{
   char BinaryFile[48];
} BXVD_IMAGE_ContextEntry;

#else

typedef struct BXVD_IMAGE_ContextEntry
{
   const unsigned int *puiImageSize;
   const void *pImageData;
   const unsigned int *puiImageOffset;
#if BXVD_P_AVD_ARC600
   const unsigned int  uiDevice;
#endif
} BXVD_IMAGE_ContextEntry;
#endif

typedef struct BXVD_IMAGE_Container
{
#if BXVD_P_AVD_ARC600
   BAFL_ImageHeader imageHeader;
#else
   BXVD_AvdImgHdr imageHeader;
#endif
#if BXVD_P_USE_BINARY_IMAGE
   FILE *fpBinImage;
   void *pBlockData;
#endif
   const void *pImageData;
} BXVD_IMAGE_Container;

#endif /* BXVD_IMAGE_PRIV_H__ */
/* End of file. */


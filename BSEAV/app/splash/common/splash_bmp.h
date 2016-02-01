/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef __SPLASH_BMP_H
#define __SPLASH_BMP_H

#include "splash_magnum.h"

#define INVALID_FILE	1

typedef struct {
	unsigned short int type;                 /* Magic identifier            */
	unsigned int size;                       /* File size in bytes          */
	unsigned short int reserved1, reserved2;
	unsigned int offset;                     /* Offset to image data, bytes */
} BMP_HEADER;

typedef struct {
	unsigned int size;               /* Header size in bytes      */
	unsigned int width,height;       /* Width and height of image */
	unsigned short int planes;       /* Number of colour planes   */
	unsigned short int bits;         /* Bits per pixel            */
	unsigned int compression;        /* Compression type          */
	unsigned int imagesize;          /* Image size in bytes       */
	int xresolution,yresolution;     /* Pixels per meter          */
	unsigned int ncolours;           /* Number of colours         */
	unsigned int importantcolours;   /* Important colours         */
} BMP_INFOHEADER;

typedef struct BMP_HEADER_INFO {
	BMP_HEADER header;
	BMP_INFOHEADER info;
} BMP_HEADER_INFO;


int splash_bmp_getinfo(uint8_t *imgAddress, BMP_HEADER_INFO *bmpinfo);

int splash_render_bmp_into_surface(int x, int y, 
	uint8_t *bmpAddress, void* surfaceAddress);

int splash_fillbuffer( void* surfaceAddress, int r, int g, int b);
int splash_set_surf_params(BPXL_Format pxlFmt,uint32_t splashPitch,
	uint32_t splashWidth,uint32_t splashHeight);

#endif  /* #ifndef __SPLASH_BMP_H */

/* End of File */


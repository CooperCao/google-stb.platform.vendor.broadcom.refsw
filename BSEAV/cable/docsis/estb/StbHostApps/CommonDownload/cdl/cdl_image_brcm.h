/***************************************************************************
 *     Copyright (c) 2007-2014, Broadcom Corporation
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
 * Module Description:  OpenCable Common Download image interface. Functions
 *                      in this module provides info about broadcom monolithic 
 *                      image
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef CDL_IMAGE_BRCM_H
#define CDL_IMAGE_BRCM_H

#define CDL_MAX_FILES 8   /* maximum number of files in the bundled image */
/*
 * cdl_image_header is the header of the monolithic image downloaded directly
 * from headend, it can help locate individual files in the monolithic image.
 */

struct cdl_image_header {
#define CDL_IMAGE_HEADER_MAGIC 0x42434d44   /* 'BCMD' */
	uint32_t magic; /* magic number */
	int ver; /* image header version */
	int num; /* number of individual files in this image */
	int tot_len; /* total length of the bundled image */
	union {
		struct ver1 {

			int type; /* image type */
			char filename[64]; /* flash partition name, shared by cfe and kernel */
			char devname[64]; /* flash device node */
			uint32_t start; /* start address in the monolithic image, must aligned to 32bit, required by CFE */
			uint32_t size;/* size of the file */
		} v1[CDL_MAX_FILES];
	} v;
};

/*
 * print the monolithic image header
 */
void cdl_image_brcm_print_header(char * mono_file, int sec_header_len);

/* 
 * generate monolithic image. 
 * return >= 0 if succeed, < 0 if fail.
 */
int cdl_image_brcm_generate(char * mono_file, char * param_file );

int cdl_image_brcm_verify(char * mono_file, char * param_file );

int cdl_image_brcm_check_integrity(char * mono_file);

#endif  /* CDL_IMAGE_BRCM_H */

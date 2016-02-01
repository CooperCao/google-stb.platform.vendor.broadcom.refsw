/***************************************************************************
 *     (c)2007-2008 Broadcom Corporation
 *  
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
 *   
 *  Except as expressly set forth in the Authorized License,
 *   
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *   
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *  
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
 *  ANY LIMITED REMEDY.
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

#define CDL_MAX_FILES 20   /* maximum number of files in the bundled image */
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
			char filename[MAX_DEV_STR_LEN]; /* flash partition name, shared by cfe and kernel */
			char devname[MAX_DEV_STR_LEN]; /* flash device node */
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

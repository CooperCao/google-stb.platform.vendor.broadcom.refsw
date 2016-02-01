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
 * Module Description:  OpenCable Common Download storage interface. Functions
 *                      in this module provides image storage
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef CDL_STORAGE_H
#define CDL_STORAGE_H

#define CDL_FILE_TYPE_INVALID  0  /* invalid type */
#define CDL_FILE_TYPE_ECMBOOT  1  /* eCM bootloader */
#define CDL_FILE_TYPE_DOCSIS0  2  /* DOCSIS0 image */
#define CDL_FILE_TYPE_CFE      3  /* eSTB bootloader CFE */
#define CDL_FILE_TYPE_KERNEL   4  /* eSTB kernel */
#define CDL_FILE_TYPE_ROOTFS   5  /* eSTB root file system */


void * cdl_storage_flash_open(const char *pathname, int flags) ;
int cdl_storage_flash_close(void * h) ;
int cdl_storage_flash_read(void * h, void * buf, size_t count);
int cdl_storage_flash_write(void * h, const void * buf, size_t count);
int cdl_storage_flash_lseek(void * h, int offset, int whence);
int cdl_storage_flash_expand(void * h, char * fn, int sec_header_len);
#endif  /* CDL_STORAGE_H */

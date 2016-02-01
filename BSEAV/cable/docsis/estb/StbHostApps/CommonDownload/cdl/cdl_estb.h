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
 * Module Description:  OpenCable Common Download estb functions.
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef CDL_ESTB_H
#define CDL_ESTB_H

/* 
 * there's no spec about how big the secure header can be,
 * this is the maximum secure header size we can think of.
 */
#define MAX_SECURE_HEADER_SIZE (6 * 1024) 


/* definations based on dsg_api.h */
enum
{
	cdl_estb_kManufCvc		= 0x01,
	cdl_estb_kCosignerCvc	= 0x02
};

int cdl_estb_init();
int cdl_estb_uninit();
int cdl_estb_open() ;
int cdl_estb_close() ;
int cdl_estb_check_cvc(uint8_t *cvc, uint32_t length, uint8_t type);
int cdl_estb_authenticate_code_image( uint8_t * fileName, uint8_t fileNameLen, 
				      uint32_t tftpServerIpAddress);
int cdl_estb_download_tftp_by_cvt( char * fileName, uint32_t fileNameLen, 
				   uint32_t tftpServerIpAddress);
int cdl_estb_validate_secure_header_by_file( char * fileName);
int cdl_estb_validate_secure_header_by_buf( uint8_t * buf, int len);
int cdl_estb_authenticate_cvt(void *cdl_cvt_recv, unsigned int len);
void cdl_estb_set_monolithic_image_temp_file_name (char * fn) ;
char * cdl_estb_get_monolithic_image_temp_file_name (void) ;
void cdl_estb_set_cb_check_integrity(int (*check_integrity) (char * mono_file));

#endif  /* CDL_ESTB_H */

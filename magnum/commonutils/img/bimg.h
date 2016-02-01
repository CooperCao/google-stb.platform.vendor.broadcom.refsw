/***************************************************************************
 *     Copyright (c) 2005, Broadcom Corporation
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
 * Module Description: throttled playback
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************/
#ifndef BIMG_H__
#define BIMG_H__

/*================== Module Overview =====================================
This modules defines interface to access firware image files.

User of the interface (low level software, aka Porting Interface module) is responsible: 
	o to pair 'open' with 'close' call 
	o do not nest calls to 'open' 
	o test return codes from 'open' and 'next' calls, each error considered to be fatal 
	o do not use interface in the _isr context 

Provider of the image loader interface is resposible: 
	o open,next and close calls shall take bounded time to execute 
	o 'next' method should either read requested number of bytes from the media or return an error 
  
========================================================================*/

/***************************************************************************
Summary:
	This method is used to open image interface and allow further access to the image

Input:
	context - conext of the image interface
    image_id - system specific ID for the specific image

Output:
	image - context for the opened image

Returns:
    BERR_SUCCESS - if function succeded
	error code - if function has failed
	

***************************************************************************/
typedef BERR_Code (*BIMG_Method_Open)(
	void *context,  /* context of the image interface */
	void **image,  /* [out] pointer to the image context */
	unsigned image_id /* ID of the image */
);

/***************************************************************************
Summary:
	This method is used to access to the image data

Input:
	image - conext of previously opened image
	chunk - number of microcode block
    length - size of the microcode block

Output:
	data - pointer to the images data

Returns:
    BERR_SUCCESS - if function succeded and data points to the memory with exact 'length' bytes of data
	error code - if function has failed

***************************************************************************/
typedef BERR_Code (*BIMG_Method_Next)(
	void *image,  /* image context information, one returned by the 'open' call */
	unsigned chunk,  /* number of chunk, starts from 0, shall increment with each call */
	const void **data,  /*  [out] returns pointer to next piece of data, contents of this pointer is valid until succeding call to the 'next' function */
	uint16_t length /* number of bytes to read from the image,  length shall be less than 64K */
); 

/***************************************************************************
Summary:
	This method is used to close image context

Input:
	image - conext of previously opened image

Returns:
    <none>

***************************************************************************/
typedef void (*BIMG_Method_Close)(
	void *image /* image context */
);

/***************************************************************************
Summary:
	This interface is used to access firmware images

***************************************************************************/
typedef struct BIMG_Interface {
	BIMG_Method_Open open; /* open method */
	BIMG_Method_Next next; /* 'next' method */
	BIMG_Method_Close close; /* close method */
} BIMG_Interface;

#endif /* BIMG_H__ */



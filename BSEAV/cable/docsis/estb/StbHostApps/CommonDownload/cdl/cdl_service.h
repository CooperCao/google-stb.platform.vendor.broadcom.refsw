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
 * Module Description:  cdl service interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef CDL_SERVICE_H
#define CDL_SERVICE_H
#ifdef __cplusplus
extern "C" {
#endif



/***************************************************************************
Summary:
Image interface for Common Download (CDL) App Lib. The image interface is responsble 
for generating, and verifying the monolithic image. In reference design, cdl_image_brcm.c
implements the interface. The customer is expected to replace 
the interface with their own implementations. 
***************************************************************************/

struct cdl_image_interface {
	
/***************************************************************************
Summary:
Print the monolithic image header

Description:
Print the monolithic image header. It's used for displaying the internal
modules of a monolithic image. Mainly for debugging purpose.
Input:
	mono_file - the monolithic image file name
	sec_header_len - the length of the secure header (digital signature)
Returns:
        none
***************************************************************************/
	void (*print_header) (char * mono_file, int sec_header_len ); 

/***************************************************************************
Summary:
Generate the monolithic image

Description:
This function generates the monolithic image. The function takes the individual
firmware modules and combine them into one unsigned monolithic image. A separate
program is needed to sign the image.
Input:
	mono_file - the monolithic image file name
	param_file - the parameter file name, the individual firmware modules 
                     (cfe, kernel, application, etc) are specified in this file
Returns:
	>=0 - If image is generated correctly
	<0 - If fail.
***************************************************************************/
	int (*generate) (char * mono_file, char * param_file );

/***************************************************************************
Summary:
Verify the monolithic image

Description:
This function verifies the monolithic image(debug only). The individual firmware 
modules are extracted from the monolithic image and compared with the files 
specified in param_file. 
Input:
	mono_file - the monolithic image file name
	param_file - the parameter file name, the individual firmware modules 
                     (cfe, kernel, application, etc) are specified in this file
Returns:
	>=0 - If monolithic image contains the same modules listed in param_file
	<0 - If monolithic image diffres from the modules listed in param_file.
***************************************************************************/
	int (*verify) (char * mono_file, char * param_file);

/***************************************************************************
Summary:
Check the integrity of the monolithic image

Description:
This function checks the integrity of the monolithic image. One example of
integrity check is CRC. The function is called after the image has been downloaded. 
Input:
	mono_file - the monolithic image file name
	param_file - the parameter file name, the individual firmware modules 
                     (cfe, kernel, application, etc) are specified in this file
Returns:
	>=0 - If monolithic image is valid
	<0 - If monolithic image is not valid.
***************************************************************************/
	int (*check_integrity) (char * mono_file);
};


/***************************************************************************
Summary:
Storage interface for Common Download (CDL) App Lib. The storage interface is responsble 
for storing the image. The image can be stored on non-volatile device. In the reference
design, the image is stored in flash(cdl_storage_flash.c). The customer is expected to 
replace the interface with their own implementations. No matter which device is 
used to store, there should be enough space for 2 monolithic images. One for newly
downloaded image, one for recovery. Once the new image is expanded and booted successfully,
it becomes the recovery image.

***************************************************************************/
struct cdl_storage_interface {

/***************************************************************************
Summary:
Open the storage device

Description:
This function opens the storage device.
Input:
	pathname - the path name of the storage device
	flags - optional open flags, similar to open(2) in linux

Returns:
	Non-zero pointer - If succeed, opaque pointer handle.
	NULL - If fail, 
***************************************************************************/
	void * (*open) (const char *pathname, int flags) ;

/***************************************************************************
Summary:
Close the storage device

Description:
This function closes the storage device.
Input:
	h - handle of the interface
Returns:
	>=0 - If succeed
	<0 - If fail.
***************************************************************************/
	int (*close) (void * h) ;


/***************************************************************************
Summary:
Read from the storage device

Description:
This function reads data from  the storage device.
Input:
	h - handle of the interface
        buf - buffer to hold the data 
        count - number of bytes to read
Returns:
	>0 - Return number of bytes read successfully
	<0 - If fail.
        =0 - If end of file is reached
***************************************************************************/
	int (*read) (void * h, void * buf, size_t count);

/***************************************************************************
Summary:
Write to the storage device

Description:
This function writes data to the storage device.
Input:
	h - handle of the interface
        buf - buffer to hold the data 
        count - number of bytes to write
Returns:
	>=0 - Return number of bytes written successfully
	<0 - If fail.
***************************************************************************/
	int (*write) (void * h, const void * buf, size_t count);


/***************************************************************************
Summary:
seek in the storage device

Description:
This function seek in the storage device.
Input:
	h - handle of the interface
        offset - offset related to whence 
        whence - starting point, SEEK_SET, SEEK_END, or SEEK_CUR
Returns:
	>=0 - Return number of bytes written successfully
	<0 - If fail.
***************************************************************************/
	int (*lseek) (void * h, int offset, int whence);


/***************************************************************************
Summary:
Expand the individual firmware modules from the storage device

Description:
This function expand the individual firmware modules (cfe, kernel, application)
from the storage device.
Input:
	h - handle of the interface
        fn - the target device to expand the image. 
        sec_header_len - the length of the secure header (digital signature)  
        whence - starting point, SEEK_SET, SEEK_END, or SEEK_CUR
Returns:
	>=0 - Return number of bytes written successfully
	<0 - If fail.
***************************************************************************/
	int (*expand) (void * h, char * fn, int sec_header_len);
};

/***************************************************************************
Summary:
Bootinfo interface for Common Download (CDL) App Lib. The bootinf interface is responsble 
for storing the boot and download information. An example of the bootinfo is the current
download status(just downloaded, first reboot, expand succeed, etc). Since some steps 
of the download may be done in bootloader, (for example, expand firmware). The bootinfo
may be shared by both kernel and bootloader. This is the case in  reference design
(cdl_bootinfo_nvram.c). The customer is expected to replace the interface with their own 
implementations. 
***************************************************************************/
struct cdl_bootinfo_interface {

/***************************************************************************
Summary:
Open the bootinfo device

Description:
This function opens the bootinfo device.
Input:
	pathname - the path name of the bootinfo device
Returns:
	>=0  - If succeed.
	<0 - If fail 
***************************************************************************/
	int (*open) (char *pathname) ;

/***************************************************************************
Summary:
Close the bootinfo device

Description:
This function closes the bootinfo device.
Input:
	None
Returns:
	>=0  - If succeed.
	<0 - If fail 
***************************************************************************/
	int (*close) (void) ;

/***************************************************************************
Summary:
Update the bootinfo when the aplication just started

Description:
This function updates the bootinfo when the proram just started. Whe the app
just started, it needs to check if last image download has succeed, it also
needs to update the download status, and optionally notify the headend. All 
these operations should be done in this function.
Input:
	None
Returns:
	>=0  - If succeed.
	<0 - If fail 
***************************************************************************/
	int (*just_started) (void);

/***************************************************************************
Summary:
Update the bootinfo when a new image has been downloaded.

Description:
This function updates the bootinfo when a new image has been downloaded and 
is ready to reboot. In this function, the download status will be updated.

Input:
	None
Returns:
	>=0  - If succeed.
	<0 - If fail 
***************************************************************************/
	int (*ready_to_reboot) (void);


/***************************************************************************
Summary:
Get the device name for the new image.

Description:
This function returns the the device name for new image, the same device 
may have different name in kernel and the bootloader. The function return
both names (For example, /dev/mtdocapx and flash0.imagex represents the 
same device, but they are named differently in linux and cfe). 

Input:
	None
Output:
        fn_kernel - the linux device name for the new image (for example, /dev/mtdocapxxx)
        fn_bootloader - the bootloader device name for the new image ( for example, flash0.imagex in cfe)
Returns:
	>=0  - If succeed.
	<0 - If fail 
***************************************************************************/
	int (*get_new_storage)(char * fn_kernel, char * fn_bootloader);


/***************************************************************************
Summary:
Get the bootinfo device name.

Description:
This function returns the the device name for bootinfo. The application can 
call this function to get the bootinfo device, and use the returned 
device name to open the bootinfo device.

Input:
	None
Returns:
	Char pointer  - If succeed, the string pointed to the bootinfo device.
	NULL - If fail 
***************************************************************************/
	char * (*get_dev_name)(void);

};

struct debug {
	char fn_cvc[MAX_DEV_STR_LEN];
	char fn_sec_header[MAX_DEV_STR_LEN];
	unsigned char authenticate_only;
};
typedef struct cdl_service_params {
	char storage_file_downloaded[MAX_DEV_STR_LEN]; /* used by kernel to temporarily save
							* the downloaded monolithic image. 
							* If the file is generated by data carousel, 
							* it includes digital signature.
							* if the file is generated by tftp, 
							* the digital signature has alreay been verified
							* and stripped off by eCM.
							* if this string is empty, the file will
							* be downloaded to its final 
							* device (storage_kernel)  directly. 
							*/
	char storage_kernel[MAX_DEV_STR_LEN]; /* final storage device of the mono image, 
					       * name in linux kernel
					       */
	char storage_bootloader[MAX_DEV_STR_LEN]; /*final storage device of the mono image, 
						   * name in bootloader
						   */

	char bootinfo_dev_kernel[MAX_DEV_STR_LEN]; /* kernel device/file used to save
						    * the boot and download info
						    */

	char bootinfo_dev_bootloader[MAX_DEV_STR_LEN];/* bootloader device/file used to save
						       * the boot and download info
						       */
	struct cdl_image_interface image_if;
	struct cdl_storage_interface storage_if;
	struct cdl_bootinfo_interface bootinfo_if;
	
	/* the following field is for debugging purpose, */
	struct debug debug;
} cdl_service_params;

#define CDL_SERVICE_ATTACH_TUNER  0 

/* return >= 0 if succeed; < 0 if fail */
void cdl_service_print_image_header(int sec_header_len);
/* return >= 0 if succeed; < 0 if fail */
int cdl_service_generate_image(char * param_file );
/* return >= 0 if succeed; < 0 if fail */
int cdl_service_verify_image(char * param_file );
/* return >= 0 if succeed; < 0 if fail */
int cdl_service_upgrade_image(int sec_header_len);
/* return >= 0 if succeed; < 0 if fail */
int cdl_service_authenticate_image(int force);
/* return >= 0 if succeed; < 0 if fail */
int cdl_service_start(void);
/* return >= 0 if succeed; < 0 if fail */
int cdl_service_stop(void);
void cdl_service_get_params(cdl_service_params * params);
void cdl_service_set_params(cdl_service_params * params);
void cdl_service_init();
void cdl_service_uninit();
int cdl_service_control(int command, void *data, int id);

#ifdef __cplusplus
}
#endif

#endif  /* CDL_SERVICE_H */

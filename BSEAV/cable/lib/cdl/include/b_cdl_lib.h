/***************************************************************************
 *     (c)2007-2009 Broadcom Corporation
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
 * Module Description:  cdl service interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef B_CDL_LIB_H
#define B_CDL_LIB_H
#ifdef __cplusplus
extern "C" {
#endif

/*=Module Overview: ********************************************************
The purpose of this module is to implement OpenCable Common Download 2.0,
(OC-SP-CDL2.0-I06-080118) in Broadcom Application Library.

CDL defines a common protocol to download and upgrade OpenCable Host Device(OCHD)
firmware. The OCHD firmware is delivered as a monolithic image, the image is
signed in X.509 PKCS#7 format. The download is triggerred by eCM (eCM config file,
or SNMP set command)  or eSTB (Code Verifcation Table). The image is delivered
by TFTP or DSM_CC data carouse.

CDL only specifies the download and authentication of the monolithic image. The
internal of the image is implementation dependent. In the CDL library,
we are trying to handle the CDL protocal internally, and provide some external
interfaces (callbacks) for customer to implement their own design. One example
is the cdl_storage_interface, where the user specifies where and how to save
the monolithic image, and how to expand the monolithic image onto the
final storage device.

An psuedo code for CDL library is like following,

Int main() {

  // init internal modules

  B_Cdl_Settings  params;

  B_CdlGetDefaultSettings(&params);

  // update the vendor specific settings

  B_Cdl_Init(&params);

  // get the default parameters
  B_Cdl_GetSettingss(&cdl_params);

  // customize design by setting the cdl_xxx_if callback functions,
  //  the callbacks are used by cdl library internally.
  cdl_params.storage_if.open = cdl_storage_custom1_open;
  cdl_params.storage_if.close = cdl_storage_custom1_close;
  B_Cdl_SetSettings(&cdl_params);

  // start waiting for the CDL trigger
  B_Cdl_Start();

  //STB will reboot when download is finished
}

***************************************************************************/
#ifdef MPOD_SUPPORT
#include "nexus_base_types.h"
#endif

#define BUF_SIZE 0x10000   /* read/write buffer size. for file read/write and download */

#define MAX_DEV_STR_LEN 64 /* path/file length */

#ifndef HOST_ONLY
/*
 * download method
 */
#define CDL_DLD_SIMU              0  /* simulate the monolithic image by creating one */
#define CDL_DLD_INBAND_SRC_ID     1    /* download via inband data carousel, triggered by eSTB CVT */
#define CDL_DLD_INBAND_PID        2    /* download via inband data carousel, triggered by eSTB CVT */
#define CDL_DLD_INBAND_PROG       3    /* download via inband data carousel, triggered by eSTB CVT */
#define CDL_DLD_DSG_BASIC         4    /* download via DSG tunnel basic mode, triggered by eSTB CVT */
#define CDL_DLD_DSG_ADVANCED      5    /* download via DSG tunnel advanced mode, triggered by eSTB CVT */
#define CDL_DLD_TFTP_BY_CFG_FILE  6   /* download via TFTP, triggered by eCM config file */
#define CDL_DLD_TFTP_BY_SNMP      7   /* download via TFTP, triggered by eCM SNMP Set */
#define CDL_DLD_TFTP_BY_CVT       8   /* download via TFTP, triggered by eSTB CVT */
#define CDL_DLD_TFTP              9   /* download via TFTP, don't care which trigger */
#define CDL_DLD_MAX               10  /* maximum methods */

/*
 * CVT related definations.
 * used by cdl to exchange messages with app
 */
struct cdl_adpu_cvt_inband_src_id {
	unsigned short	source_id;
}; /* type 0 */
struct cdl_adpu_cvt_inband_pid {
	unsigned short	freq;
	unsigned char	mod_type;
	unsigned short	pid;
}; /* type 1 */
struct cdl_adpu_cvt_inband_prog {
	unsigned short	freq;
#define CVT_MOD_QAM64  0x01
#define CVT_MOD_QAM256 0x02
	unsigned char	mod_type;
	unsigned short	program_number;
}; /* type 2 */
struct cdl_adpu_cvt_dsg_basic {
	char	dsg_tunnel_addr[6];
	char	src_ip_addr[16];
	char	dst_ip_addr[16];
	unsigned short	src_port;
	unsigned short	dst_port;
}; /* type 3 */
struct cdl_adpu_cvt_dsg_adv {
	unsigned short	application_id;
}; /* type 4 */

struct cdl_adpu_cvt_tftp {
	char	 server_addr[16];
}; /* tftp */

#define	CDL_ADPU_CVT_CMD_DOWNLOAD_NOW   0
#define	CDL_ADPU_CVT_CMD_DOWNLOAD_DEFER 1

struct cdl_adpu_cvt {
	int version;
	int method;
	int cmd;
	union {
		struct cdl_adpu_cvt_inband_src_id type0;
		struct cdl_adpu_cvt_inband_pid type1;
		struct cdl_adpu_cvt_inband_prog type2;
		struct cdl_adpu_cvt_dsg_basic type3;
		struct cdl_adpu_cvt_dsg_adv type4;
		struct cdl_adpu_cvt_tftp tftp;
	} t;

    unsigned short      obj_type; /* only for cvt2 v2*/
	unsigned char		obj_data_length; /* only for cvt2 v2*/
	unsigned char		obj_data[128]; /* only for cvt2 v2*/

	unsigned char		code_filename_length;
	unsigned char		code_filename[128];

};

typedef struct cdl_adpu_cvt * cdl_adpu_cvt_t;

#define B_CDL_MSG_TO_APP_ANY 0  /* any msg to app */
#define B_CDL_MSG_TO_APP_NEW_TRIGGER 0x1  /* got new trigger */
#define B_CDL_MSG_TO_APP_DOWNLOAD_NOTIFICATION 0x3  /* various notification from CDL to the app */
#define B_CDL_MSG_FROM_APP_DOWNLOAD_GRANT 0x2  /* eSTB inband only, allowing download from  app,
                                             * the pidchannel handle is included in this message
                                             */

struct b_cdl_msg {
    unsigned int type;
    union {
        struct new_trigger {
            struct cdl_adpu_cvt cvt; /* app need the inband tuning info in cvt */
#if defined (MPOD_SUPPORT) 
            NEXUS_Callback tunnel_callback; /* tunnel callback for dsg carousel download */
#else
            void * tunnel_callback;
#endif
        } new_trigger;
        struct notification {
            int host_command;   /* various status notification sent to app*/
        } notification;
    } to_app;
    union {
        struct download_grant {
            int granted; /* 1 if granted, 0 if denied */
            void * pid_channel; /* the pid channel handler for inband download */
        } download_grant;
    } from_app;
};
/***************************************************************************
[B_CDL_MSG] When CDL library and application needs to exchange information in run time,
a set of messaging APIs are defined. Currently the only sceneria for these APIs are
CVT triggered Inband download, as following,

1. CDL library got a CVT with inband download, and send a message to application.
2. Application received the message from CDL, by calling b_cdl_msg_recv_from_cdl()
3. Application wait for certain time(if deferfed download).
4. Application send a grant message to CDL library, along with the PID channel handler
   by calling b_cdl_msg_send_to_cdl().
5. CDL library regularly check the message from application. If a grant message is
   received, CDL library starts downloading using the PID channel handler.

The API for sending messags (bcdl_msg_send_to_cdl()) is non-blocking, the return value 
shows if a message is sent (> 0), busy (= 0), or error (<0).

The API for receiving messages (b_cdl_msg_recv_from_cdl()) is blocking. However, it is 
possible to make it non-blocking by using cdl_msg_callback, as following,
1. create your own callback function,
    NEXUS_Callback your_cdl_msg_callback (void * context) {
        BKNI_EventHandle event = (BKNI_EventHandle)context;
        BKNI_SetEvent(event);
    }

    your_event_processing_func() {
         BKNI_WaitForEvent(your_cdl_msg_event, BKNI_INFINITE);
         cdl_msg_recv_from_cdl();
    }

2. install callback in cdl library

    B_Cdl_Settings setttings;
    B_Cdl_GetSettings(&settings);
    setting.cdl_msg_callback = your_cdl_msg_callback;
    settings.cdl_msg_callback_context = your_cdl_msg_event;
    B_Cdl_SetSettings(&settings);

3. When CDL library needs to send a message to app, your_cdl_msg_callback() will 
   be called to notify app.


In CDL library, the message is implemented as shared memory. A semaphore is protecting
the shared memory. So on one message can be sent or received at a time.

***************************************************************************/

/***************************************************************************
Summary:
Send message to CDL.

Description:
This function is called by application, when application needs to send a message
to CDL. For example, when a granting a inband download.

Input:
	Message pointer.
Returns:
	> 0 - if succeed
    = 0 - if busy ( a message is pending)
    < 0 - if fail with error
***************************************************************************/
int b_cdl_msg_send_to_cdl(struct b_cdl_msg * msg);

/***************************************************************************
Summary:
Receive message from CDL.

Description:
This function is called by application, when application needs to receive a message
from CDL. For example, waiting for a trigger.

Input:
	Message pointer.
Returns:
	> 0 - if succeed
    = 0 - if empty ( no message)
    < 0 - if fail with error
***************************************************************************/

int b_cdl_msg_recv_from_cdl(struct b_cdl_msg * msg);

int cdl_msg_init(void);
int cdl_msg_uninit(void);
#endif /* HOST_ONLY */
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
typedef struct B_Cdl_Settings {
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
	
#if defined (MPOD_SUPPORT) 
    NEXUS_Callback cdl_msg_callback; /* called when CDL library needs to send a message
                                      * to app. App can use this callback to receive
                                      * cdl message in non-blocking way. see [B_CDL_MSG] 
                                      * for more description.
                                      */
    void * cdl_msg_callback_context;
#endif

	/* the following field is for debugging purpose, */
	struct debug debug;
} B_Cdl_Settings;
typedef enum {
    e_ImageStatusImageAuthorized = 1,
    e_ImageStatusImageCorrupted,
    e_ImageStatusCertFailure,
    e_ImageStatusMaxDownloadRetry,
    e_ImageStatusMaxRebootRetry
} e_ImageStatus;

typedef enum {
    e_CodeDownloadStatusStarted = 1,
    e_CodeDownloadStatusComplete,
    e_CodeDownloadStatusFailed
} e_CodeDownloadStatus;
typedef enum {
    /* Based on OC-STB-HOST-MIB-I09-091211.txt */
    e_DownloadFailedStatusCdlError1 = 1, /* No Failure */
    e_DownloadFailedStatusCdlError2, /* Improper code file controls - CVC subject 
                                      * organizationName for manufacturer does not match the
                                      * Host device manufacturer name
                                      */
    e_DownloadFailedStatusCdlError3, /* Improper code file controls - CVC subject 
                                      * organizationName for code cosigning agent does not 
                                      * match the Host device current code cosigning agent.
                                      */
    e_DownloadFailedStatusCdlError4, /* Improper code file controls - The 
                                      * manufacturer's PKCS #7 signingTime value is equal-to
                                      * or less-than the codeAccessStart value currently held
                                      * in the Host device.
                                      */
    e_DownloadFailedStatusCdlError5, /* Improper code file controls - The 
                                      *  manufacturer's PKCS #7 validity start time value is 
                                      * less-than the cvcAccessStart value currently held in 
                                      * the Host device.
                                      */
    e_DownloadFailedStatusCdlError6, /* Improper code file controls - The manufacturer's
                                      * CVC validity start time is less-than the cvcAccessStart
                                      * value currently held in the Host device.
                                      */
    e_DownloadFailedStatusCdlError7, /* Improper code file controls - The manufacturer's
                                      * PKCS #7 signingTime value is less-than the CVC validity
                                      * start time.
                                      */
    e_DownloadFailedStatusCdlError8, /* Improper code file controls - Missing or 
                                      * improper extendedKeyUsage extension in the 
                                      * manufacturer CVC.
                                      */
    e_DownloadFailedStatusCdlError9, /* Improper code file controls - The cosigner's 
                                      * PKCS #7 signingTime value is equal-to or less-than the
                                      * codeAccessStart value currently held in the 
                                      * Host device.
                                      */
    e_DownloadFailedStatusCdlError10, /* Improper code file controls - The cosigner's 
                                       * PKCS #7 validity start time value is less-than the 
                                       * cvcAccessStart value currently held in the Host device.
                                       */
    e_DownloadFailedStatusCdlError11, /* Improper code file controls - The cosigner's 
                                       * CVC validity start time is less-than the cvcAccessStart
                                       * value currently held in the Host device.
                                       */
    e_DownloadFailedStatusCdlError12, /* Improper code file controls - The cosigner's 
                                       * PKCS #7 signingTime value is less-than the CVC validity
                                       * start time.
                                       */
    e_DownloadFailedStatusCdlError13, /* Improper code file controls - Missing or 
                                        * improper extended key-usage extension in the cosigner's
                                        * CVC.
                                        */
    e_DownloadFailedStatusCdlError14, /* Code file manufacturer CVC validation failure. */
    e_DownloadFailedStatusCdlError15, /* Code file manufacturer CVS validation failure. */
    e_DownloadFailedStatusCdlError16, /* Code file cosigner CVC validation failure. */
    e_DownloadFailedStatusCdlError17, /* Code file cosigner CVS validation failure. */
    e_DownloadFailedStatusCdlError18, /* Improper eCM configuration file CVC format 
                                       * (e.g., missing or improper key usage attribute).
                                       */
    e_DownloadFailedStatusCdlError19, /* eCM configuration file CVC validation failure. */
    e_DownloadFailedStatusCdlError20, /* Improper SNMP CVC format. */
    e_DownloadFailedStatusCdlError21, /* CVC subject organizationName for manufacturer
                                       * does not match the Host devices manufacturer name.
                                       */
    e_DownloadFailedStatusCdlError22, /* CVC subject organizationName for code cosigning
                                       * agent does not match the Host devices current code 
                                       * cosigning agent.
                                       */
    e_DownloadFailedStatusCdlError23, /* The CVC validity start time is less-than or 
                                       * equal-to the corresponding subject's cvcAccessStart 
                                       * value currently held in the Host device.
                                       */
    e_DownloadFailedStatusCdlError24, /* Missing or improper key usage attribute. */
    e_DownloadFailedStatusCdlError25 /* SNMP CVC validation failure." */
} e_DownloadFailedStatus;

typedef struct B_Cdl_Status {
    /* status for snmp */
	e_ImageStatus imageStatus; /* This object details the image status recently downloaded */
	e_CodeDownloadStatus codeDownloadStatus; /* This object details the download status of the target image. */
	char name[256]; /* The file name of the software image to be loaded into this device. */
	e_DownloadFailedStatus downloadFailedStatus; /* This object details the firmware download failed status codes */
    unsigned int inbandCarouselTimeoutCount; /* These values increment each time an implementation
                                              * specific Inband Carousel timeout has occurred since the last
                                              * boot or reset.
                                              */
    unsigned int inbandCarouselTimeoutUs; /* download timeout period in microseconds */

    /* private status */
    int method; /* current download method */
    union {
        struct dsmcc {
            unsigned int error; /* indicates last error status */
            
            /* status of currently download module */
            int module_id;
            unsigned int offset;
            unsigned int module_size; 
            unsigned char module_info_length; /* 8 bits */
            unsigned char module_info_byte[256]; /* depends on module info length */
        } dsmcc;
    } priv;
} B_Cdl_Status;

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

/***************************************************************************
Summary:
Start the CDL.

Description:
This function starts CDL. CDL will wait for external trigger, automatically
start download and upgrades the monolithic image.
Input:
	None
Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/
int B_Cdl_Start(void);

/***************************************************************************
Summary:
Stop the CDL.

Description:
This function stops CDL.
Input:
	None
Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/
int B_Cdl_Stop(void);

/***************************************************************************
Summary:
Get the default parameters for CDL.

Description:
This function returns the default parameters used in CDL.
Customer is expected to call this function before calling B_Cdl_Init

Input:
	B_Cdl_Settings * params - Pointer to CDL parameters
Returns:
        None
***************************************************************************/

void B_Cdl_GetDefaultSettings(B_Cdl_Settings * params);

/***************************************************************************
Summary:
Get the parameters for CDL.

Description:
This function returns the current parameters used in CDL. This function and
the corresponding set function are the main entry point for customization.
Customer is expected to replace the default interfaces (mono image format,
storage device, and boot info, etc) with their own implementation. Before
calling the start function.

Input:
	B_Cdl_Settings * params - The current CDL parameters
Returns:
        None
***************************************************************************/
void B_Cdl_GetSettings(B_Cdl_Settings * params);

/***************************************************************************
Summary:
Set the parameters for CDL.

Description:
This function sets the parameters used in CDL. See the get function for more
details. This function must be called before the CDL is started.

Input:
	B_Cdl_Settings * params - The new CDL parameters
Returns:
        None
***************************************************************************/
void B_Cdl_SetSettings(B_Cdl_Settings * params);

/***************************************************************************
Summary:
Initialize CDL.

Description:
This function initialize the CDL internal parameters.
Input:
	None
Returns:
        None
***************************************************************************/
void B_Cdl_Init(B_Cdl_Settings * params);

/***************************************************************************
Summary:
Uninitialize CDL.

Description:
This function uninitialize the CDL.
Input:
	None
Returns:
        None
***************************************************************************/
void B_Cdl_Uninit(void);

/***************************************************************************
Summary:
Get the CDL status.

Description:
This function returns CDL status
Input:
	B_Cdl_Status * pStatus
Returns:
	>= 0 - if succeed
    < 0 - if fail
***************************************************************************/
#ifndef HOST_ONLY
int B_Cdl_Get_Status(B_Cdl_Status * pStatus);
#endif

#ifdef __cplusplus
}
#endif

#endif  /* B_CDL_LIB_H */

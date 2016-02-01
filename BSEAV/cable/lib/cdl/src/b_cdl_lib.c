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
 * Module Description:  OpenCable Common Download test routines
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> /* for inet_ntoa */
#include "cdl_common.h"
#include "b_cdl_lib.h"
#include "cdl_msg.h"
#include <sys/time.h>

#ifdef DSG_SUPPORT
#include "cdl_ctl.h"
#include "cdl_estb.h"
#include "rProgramStoreCustomerApi.h"
#endif

#ifdef MPOD_SUPPORT
#include "cdl_adpu.h"
#include "nexus_pid_channel.h"
#include "b_dsmcc_lib.h"
#endif /* MPOD_SUPPORT */

#ifdef TEST_INBAND_DOWNLOAD
#include "b_dsmcc_lib.h"
#endif /* TEST_INBAND_DOWNLOAD*/
#ifdef ESTB_CFG_SUPPORT
#include "b_estb_cfg_lib.h"
#endif

/*
 * The following interfaces are the only files that need customization.
 * Depending on the layout of the customer's monolithic image(cdl_image_xxx),
 * the storage device (cdl_storage_xxx) for the monolithic image (flash, HD, etc),
 * and how the download status is saved (cdl_bootinfo_xxx) in nvram.
 *
 */
#include "cdl_image_brcm.h"
#include "cdl_storage_file.h"
#include "cdl_storage_dir.h"
#ifdef HOST_ONLY
#else
#include "cdl_storage_flash.h"
#include "cdl_bootinfo_nvram.h"
#endif

BDBG_MODULE(b_cdl_lib);


#if defined(CDL_SUPPORT_ENABLE_SIGNED_IMAGE)
/*
 * for eSTB triggered CDL,
 * if DEBUG_SEC_HEADER is defined, the eSTB uses s_params.debug.fn_sec_header
 * to do the security header check (debug only).
 *
 * Otherwise, the actual downloaded file (s_params.storage_file_downloaded) will be
 * used to do the check.
 */
/* #define DEBUG_SEC_HEADER */
#endif
/*
 * debug CVT signaling only, no data download
 */
/* #define DEBUG_CVT */


#if defined(MPOD_SUPPORT)
#ifdef SETTOPAPI_SUPPORT
/* drop the support for legacy settop API, we only support native NEXUS API now */
#error "SETTOPAPI not supported"
#endif

/*
 * For eSTB triggered inband download, CDL library will send the channel info
 * (frequency, modulation, program number, pid number etc) to application,
 * and wait for granting from application. The grant message from application
 * includes the pid channel handler. The pidChannel will be used by cdl library
 * to do message filtering.
 */
static NEXUS_PidChannelHandle pidChannel = NULL;
#endif


static B_Cdl_Settings s_params;

#ifdef ESTB_CFG_SUPPORT
static char tmp_new_fw_name[MAX_DEV_STR_LEN] = "";
static int tmp_new_fw_name_len = 0;
#endif
cdl_thread_info wait_for_ca_cvt_thread_info;
cdl_thread_info post_download_processing_thread_info;


/***************************************************************************
Summary:
Print the monolithic image header.

Description:
This function prints the the original monolithic image header, with the secure
header skipped.

Input:
	int sec_header_len - the length of the secure header
Returns:
	None
***************************************************************************/
void cdl_service_print_image_header(int sec_header_len) {
	if (s_params.image_if.print_header) {
		s_params.image_if.print_header(s_params.storage_file_downloaded,
						 sec_header_len);
	}
}

/***************************************************************************
Summary:
Generate the original monolithic image .

Description:
This function generates the monolithic image from the individual files specifiedin param_file. This function is not required for CDL, but a convinent tool
for CDL utility.

Input:
	char * param_file - the file name that specify the individual images.
Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/
int cdl_service_generate_image(char * param_file ) {
	if (s_params.image_if.generate) {
		return s_params.image_if.generate(s_params.storage_file_downloaded, param_file);
	}
	return -1;
}

/***************************************************************************
Summary:
verify the monolithic image .

Description:
This function verifies the monolithic image from the individual files specifiedin param_file. It is paired with the generate_image function, to verify the contents
 generated are correct.This function is not required for CDL, but a convinent tool
for CDL utility.

Input:
	char * param_file - the file name that specify the individual images.
Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/
int cdl_service_verify_image(char * param_file ) {
	if (s_params.image_if.verify) {
		return s_params.image_if.verify(s_params.storage_file_downloaded, param_file);
	}
	return -1;
}

/* return >=0 if succeed, -1 if fail */
int cdl_service_expand_image(int sec_header_len) {
	int ret = -1;
    void * h_storage = NULL;
#ifdef HOST_ONLY
    if (strlen(s_params.storage_file_downloaded) == 0) {
        BDBG_ERR(("Skip expand"));
        return 0;
    }
    if ((h_storage = s_params.storage_if.open(s_params.storage_file_downloaded,
                                              O_RDONLY)) == NULL) {
        return -1;
    }
        
    if ( (ret = s_params.storage_if.expand(h_storage, s_params.storage_kernel, sec_header_len)) < 0) {
        printf("expand error %d\n", ret);
        goto out;
    }
#else
    char * str;
    if ((str = getenv("EXPAND_CMD"))== NULL) {
		BDBG_WRN(("%s: EXPAND_CMD environment variable not defined. ", __FUNCTION__));;
        if (strlen(s_params.storage_file_downloaded) == 0) {
            BDBG_ERR(("Skip expand"));
            return 0;
        }
        if ((h_storage = s_params.storage_if.open(s_params.storage_file_downloaded,
                                                  O_RDONLY)) == NULL) {
            return -1;
        }

        if ( (ret = s_params.storage_if.expand(h_storage, s_params.storage_kernel, sec_header_len)) < 0) {
            printf("expand error %d\n", ret);
            goto out;
        }
    } else {
        /* call the external command to expand the mono image */
        char tmp[256];
#if defined(CDL_SUPPORT_ENABLE_SIGNED_IMAGE)
        snprintf(tmp, 255, "%s %s", str, s_params.storage_kernel);
#else
        snprintf(tmp, 255, "%s %s", str, s_params.storage_file_downloaded);
#endif
		BDBG_WRN(("%s: use EXPAND_CMD environment variable", __FUNCTION__));;
        BDBG_ERR(("%s", tmp));
        return system(tmp);
    }
#endif /* HOST_ONLY */
 out:
	s_params.storage_if.close(h_storage);
	return ret;
}


/***************************************************************************
Summary:
upgrade the monolithic image .

Description:
This function upgrades the monolithic image. Normally, the application does
not need to call this function directly. As CDL library will call it internally
after the download. We keep this function external so the cdltest utility can
use it in debugging.

Input:
	int sec_header_len - The length of the secure header.
Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/
int cdl_service_upgrade_image(int sec_header_len) {
	/* exapnd image */

	if (cdl_service_expand_image(sec_header_len) < 0) return -1;
#ifdef HOST_ONLY
	BDBG_ERR(("\n*********** monolithic image %s expand to %s successfully ***********\n",
		  s_params.storage_file_downloaded, s_params.storage_kernel));
#else
	/* do nothing as more objects may need to be downloaded  */
#endif
	return 0;
}

#ifdef DSG_SUPPORT
/* return >= 0 if succeed, <0 if fail */
int cdl_service_authenticate_image(int force) {
	int ret = -1;
	uint32_t server_addr;
#if defined(CDL_SUPPORT_ENABLE_SIGNED_IMAGE)
	if (force) {
		extern int load_cvc(char *fn);
		uint8_t * buf = NULL;
		void * h_storage = NULL;
		int n;

		load_cvc(s_params.debug.fn_cvc);

		/* check secure header */
		if ( (buf = (uint8_t *)malloc(BUF_SIZE)) == NULL) {
			BDBG_ERR(("malloc failed\n"));
			return ret;
		}
#ifdef DEBUG_SEC_HEADER
		if ((h_storage = s_params.storage_if.open(s_params.debug.fn_sec_header,
							  O_RDONLY)) == NULL) {
			free(buf);
			return ret;
		}
#else
		if ((h_storage = s_params.storage_if.open(s_params.storage_file_downloaded,
							  O_RDONLY)) == NULL) {
			free(buf);
			return ret;
		}
#endif
		if ( (n = s_params.storage_if.read(h_storage, buf, MAX_SECURE_HEADER_SIZE)) <= 0 ) {
			free(buf);
			s_params.storage_if.close(h_storage);
			return ret;
		}
		if (cdl_estb_validate_secure_header_by_buf(buf, n) == 0) {
#if defined(MPOD_SUPPORT)
			cdl_adpu_send_host_download_control(B_MPOD_DL_CERTIFICATE_FAILURE);
            cdl_ctl_set_state(CDL_CTL_ST_CERT_FAILED);
#endif
			return ret;
		}
		/* cdl_ctl_set_secure_header_len(sec_header_len); */
		free(buf);
		s_params.storage_if.close(h_storage);
	}


	if (cdl_ctl_need_authenticate() || force) {
		/* authenticate image */
		/*
		if ((sec_header_len = cdl_ctl_get_secure_header_len()) <= 0) {
			BDBG_ERR(("Invalid secure header %d\n", sec_header_len));
#if defined(MPOD_SUPPORT)
			cdl_adpu_send_host_download_control(B_MPOD_DL_CERTIFICATE_FAILURE);
#endif
			return 0;
		} else {
			BDBG_MSG(("secure header %d \n", sec_header_len));
		}
		*/
		server_addr = htonl(inet_addr("192.168.17.10")); /* local tftp server */
		ret = cdl_estb_authenticate_code_image((uint8_t *)s_params.storage_file_downloaded,
						       strlen(s_params.storage_file_downloaded),
							  server_addr );
		BDBG_MSG(("authenticate code image  returns %d\n", ret));
		if (ret < 0 ) {
            cdl_ctl_set_state(CDL_CTL_ST_CERT_FAILED);
#if defined(MPOD_SUPPORT)
			cdl_adpu_send_host_download_control(B_MPOD_DL_CERTIFICATE_FAILURE);
#endif
            return ret;
		}
	} else {
		BDBG_MSG(("TFTP donwload already authenticated by eCM, do nothing here\n"));
	}
#else
    BDBG_ERR(("%s Unsigned image, skip security check", __FUNCTION__));
    ret = 0;
#endif /* CDL_SUPPORT_ENABLE_SIGNED_IMAGE */
	/*
	 * customer specific image validation
	 */
    cdl_ctl_set_state(CDL_CTL_ST_IMAGE_CHECK);
	if (s_params.image_if.check_integrity(s_params.storage_file_downloaded) < 0) {
#if defined(MPOD_SUPPORT)
		cdl_adpu_send_host_download_control(B_MPOD_DL_DOWNLOAD_IMAGE_DAMAGED);
#endif
        cdl_ctl_set_state(CDL_CTL_ST_IMAGE_DAMAGED);
		BDBG_ERR(("Image damaged\n"));
		ret = -1;
	}
	return ret;
}

int trigger_by_ecm(void) {
	int ret = -1;
	if ( (ret = cdl_estb_open()) >= 0) {
		cdl_ctl_set_trigger(CDL_CTL_TRIGGER_ECM_CFG_FILE | CDL_CTL_TRIGGER_SNMP);
		if (cdl_ctl_is_busy() == 0) {
			cdl_ctl_set_state(CDL_CTL_ST_WAIT_TRIGGER);
		}
	}
	return ret;
}
#endif

#if defined(MPOD_SUPPORT)
/* return >= 0 if succeed, < 0 if fail */
static int download_carousel(int method, unsigned char * name, int name_len) {
#ifdef DEBUG_CVT
    int ret = 0;
    BDBG_ERR(("%s debug CVT only, skip downloading", __FUNCTION__));
    return ret;
#else
	int ret = 0, n = 0;
	int module_id = 1;
	uint8_t * buf = NULL;
	int fd = -1;
	int len = 0;
	int sec_header_valid = 0;
	int sec_header_len = 0;
    B_Dsmcc_Handle dsmccHandle = NULL;
    B_Dsmcc_Settings dsmccSettings;
    B_Dsmcc_Status dsmcc_status;
    struct timeval start_time, cur_time;
#define MAX_DOWNLOAD_TIMEOUT_S 3600
    unsigned int s_download_time_out_s = MAX_DOWNLOAD_TIMEOUT_S;
    unsigned int s_download_time_passed_s = 0;

	cdl_ctl_set_method(method);
	cdl_ctl_set_state(CDL_CTL_ST_DOWNLOAD);
#ifdef ESTB_CFG_SUPPORT
    /* This should be done before reboot */
    memset(tmp_new_fw_name, 0, MAX_DEV_STR_LEN);
    memcpy(tmp_new_fw_name, name, name_len);
    tmp_new_fw_name_len = name_len;
#endif
	cdl_estb_open();
	cdl_adpu_send_host_download_control(B_MPOD_DL_DOWNLOAD_STARTED);
    if (method == CDL_DLD_DSG_ADVANCED) {
        B_Dsmcc_GetDefaultSettings(&dsmccSettings);
        dsmccSettings.inputType = B_DSMCC_INPUT_TYPE_DSG;
    } else {
        if (pidChannel == NULL) {
            BDBG_ERR(("tuner not available"));
            return -1;
        }
        
        /* setup dsmcc settings */
        B_Dsmcc_GetDefaultSettings(&dsmccSettings);
        dsmccSettings.pidChannel = pidChannel;
    }
    /* open dsmcc */
    dsmccHandle = B_Dsmcc_Open(&dsmccSettings);
    if (dsmccHandle == NULL) 
    {
        BDBG_ERR(("Dsmcc open failed"));
        B_Dsmcc_GetStatus(&dsmcc_status);
        if (B_DSMCC_ERR_MODULE_NOT_FOUND == dsmcc_status.error) 
            cdl_adpu_send_host_download_control(B_MPOD_DL_DOWNLOAD_NOTIFY_HEADEND);
        return -1;
    }

    ret = B_Dsmcc_GetStatus(&dsmcc_status);
    if (ret < 0) {
        BDBG_ERR(("Cannot get DSMCC Status"));
        return ret;
    }
    if (dsmcc_status.inbandCarouselTimeoutUs) {
        s_download_time_out_s = (dsmcc_status.inbandCarouselTimeoutUs / 1000000);
    } else {
        s_download_time_passed_s = MAX_DOWNLOAD_TIMEOUT_S;
    }
    gettimeofday(&start_time, NULL);
    s_download_time_passed_s = 0;

	/* write image */
	if ( (buf = (uint8_t *)malloc(BUF_SIZE)) == NULL) {
		BDBG_ERR(("malloc failed\n"));
		ret = -1;
		goto out;
	}

	module_id = B_Dsmcc_GetModuleIdFromName(dsmccHandle, name, name_len);
    if (module_id < 0) {
        ret = -1;
        B_Dsmcc_GetStatus(&dsmcc_status);
        if (B_DSMCC_ERR_MODULE_NOT_FOUND == dsmcc_status.error) 
            cdl_adpu_send_host_download_control(B_MPOD_DL_DOWNLOAD_NOTIFY_HEADEND);
        goto out;
    }

	if ((fd = open(s_params.storage_file_downloaded, O_WRONLY | O_CREAT | O_TRUNC)) < 0) {
		BDBG_ERR(("cannot open %s\n", s_params.storage_file_downloaded));
		ret = -1;
		goto out;
	}
	len = 0;

	while ((n = B_Dsmcc_Read(dsmccHandle, module_id, buf, BUF_SIZE)) > 0) {
        gettimeofday(&cur_time, NULL);
        s_download_time_passed_s = cur_time.tv_sec - start_time.tv_sec;
        if (s_download_time_passed_s >= s_download_time_out_s) {
            BDBG_ERR(("Time out %d seconds", s_download_time_out_s));
            break;
        }
        if (sec_header_valid == 0 && (len >= MAX_SECURE_HEADER_SIZE || n >=MAX_SECURE_HEADER_SIZE)) {
			/* check the secure header first */
#if defined(CDL_SUPPORT_ENABLE_SIGNED_IMAGE)
            /* bypass sec check */
#ifdef DEBUG_SEC_HEADER
			/* debug only, simulate downloaded image header with a local file fn_sec_header */
			if ((sec_header_len = cdl_estb_validate_secure_header_by_file(s_params.debug.fn_sec_header)) == 0) {
#else
                  if ((sec_header_len = cdl_estb_validate_secure_header_by_buf(buf, n)) == 0) {
#endif
                BDBG_ERR(("%s: invalid secure header", __FUNCTION__));
				cdl_adpu_send_host_download_control(B_MPOD_DL_CERTIFICATE_FAILURE);
                cdl_ctl_set_state(CDL_CTL_ST_CERT_FAILED);
				ret = -1;
				goto out;
			}
#else
            BDBG_WRN(("Unsigned image, bypass security header check"));
            sec_header_len = 0;
#endif
			if (getenv(CDL_ENV_DLD_SLEEP)) {
				long sec;
				sec = strtol(getenv(CDL_ENV_DLD_SLEEP), NULL, 0);
				BDBG_ERR(("DEBUG ONLY!! Sleep %ld seconds to simulate downloading", sec));
			}
			sec_header_valid = 1;
			n= n - sec_header_len;
			/* stripping off the secure header after it's been validated */
			if ( (ret = write(fd, buf + sec_header_len, n)) != n) {
				BDBG_ERR(("Write error, write %d, expect %d bytes\n", ret, n));
				ret = -1;
				goto out;
			}
        } else {
			if ( (ret = write(fd, buf, n)) < n) {
				BDBG_ERR(("Write error, write %d, expect %d bytes\n", ret, n));
				ret = -1;
				goto out;
			}
        }
		len += n;
        
    }
    if (s_download_time_passed_s >= s_download_time_out_s) {
        ret = -1;
        BDBG_MSG(("donwload inband timeout\n"));
        cdl_adpu_send_host_download_control(B_MPOD_DL_DOWNLOAD_IMAGE_DAMAGED);
        goto out;
    }
    if ( n == 0) {
        /*
         * If we get here, it means no error for the whole process, the image has
         * been downloaded successfully. The new STARTUP has been updated in above expand function.
         */
        ret = 0;
        BDBG_MSG(("donwload inband succeed\n"));
        cdl_adpu_send_host_download_control(B_MPOD_DL_DOWNLOAD_COMPLETED);
        cdl_ctl_set_state(CDL_CTL_ST_DLD_COMPLETE);
    } else if ( n < 0) {
        ret = -1;
        BDBG_MSG(("donwload inband failed\n"));
        cdl_adpu_send_host_download_control(B_MPOD_DL_DOWNLOAD_IMAGE_DAMAGED);
    }


 out:
	if (fd >= 0)close(fd);
    if (dsmccHandle) B_Dsmcc_Close(dsmccHandle);
	if (buf) free(buf);
	return ret;
#endif /* DEBUG_CVT */
}

/* return >= 0 if succeed, < 0 if fail */
static int download_tftp_by_cvt(cdl_adpu_cvt_t h_cvt) {
#ifdef DEBUG_CVT
    int ret = 0;
    BDBG_ERR(("%s debug CVT only, skip downloading", __FUNCTION__));

    return ret;
#else
	int ret  = -1;
	struct in_addr iaddr;
	char ch[32];
	uint32_t server_addr;
	uint32_t fn_len;

	BDBG_ERR(("TODO: tftp download by cvt"));
	cdl_ctl_set_method(h_cvt->method);
	cdl_ctl_set_state(CDL_CTL_ST_DOWNLOAD);
	cdl_estb_open();
	cdl_adpu_send_host_download_control(B_MPOD_DL_DOWNLOAD_STARTED);
	ret = 1;

	/* server address is 16 byte ipv6 address, for ipv4, only the last 4 bytes matters */
	memcpy (&iaddr.s_addr, &h_cvt->t.tftp.server_addr[12], 4);
	strncpy(ch, inet_ntoa(iaddr), 32);

	memcpy (&server_addr, &h_cvt->t.tftp.server_addr[12], 4);
	server_addr = ntohl(server_addr);
	BDBG_ERR(("tftp addr %s 0x%x", ch, server_addr));
	fn_len = (uint32_t)strlen(h_cvt->code_filename);

	ret = cdl_estb_download_tftp_by_cvt( h_cvt->code_filename,
					     fn_len,
					     server_addr);
	return ret;
#endif /* DEBUG_CVT */
}

int wait_for_ca_cvt() {
	int ret = -1;
	cdl_adpu_cvt_t h_cvt;
    struct b_cdl_msg msg;
    int downloaded_obj = 0;

    h_cvt = NULL;
	while (1) {
        /* 
         * wait for cvt if no CVT has been received, 
         * or a current download has not been granted by application yet (deferred download)
         */
        downloaded_obj = 0;
        h_cvt = NULL;
		ret = cdl_adpu_cvt_wait();
		if (wait_for_ca_cvt_thread_info.exit == 1) break;
        if (cdl_ctl_is_busy()) {
            /* a download is already in progress */
            BDBG_MSG(("Download in progress, ignore new CVT\n"));
            goto wait_for_cvt;
        }
        if (ret == 0) {
            /*
             * Received new CVT
             * parse cvt
             */
            do {
                h_cvt = cdl_adpu_cvt_get_next_object();
                if ( h_cvt == NULL) {
                    BDBG_MSG(("No more objects in CVT\n"));
                    break;
                }
                if (cdl_ctl_need_upgrade(h_cvt)) {
                    BDBG_MSG(("fw upgrade is needed"));
                    break;                    
                } else {
                    BDBG_MSG(("no fw upgrade is needed"));
                }
            } while (h_cvt);
            if (h_cvt == NULL) goto wait_for_cvt;
            /* 
             * inband download needs grant from app,
             * tftp download will call EstbSwUpgrade_IsUpgradePermittedNow() later
             */
            
            if (h_cvt->method == CDL_DLD_INBAND_SRC_ID ||
                h_cvt->method == CDL_DLD_INBAND_PID ||
                h_cvt->method == CDL_DLD_INBAND_PROG ||
                h_cvt->method == CDL_DLD_DSG_ADVANCED) {
                BDBG_MSG(("got new cvt"));
                memset(&msg, 0, sizeof(msg));
                msg.type = B_CDL_MSG_TO_APP_NEW_TRIGGER;
                memcpy(&msg.to_app.new_trigger.cvt, h_cvt, sizeof(msg.to_app.new_trigger.cvt));
                if (h_cvt->method == CDL_DLD_DSG_ADVANCED) {
                    B_Dsmcc_Settings dsmccSettings;

                    /* setup dsmcc settings */
                    B_Dsmcc_GetDefaultSettings(&dsmccSettings);
                    msg.to_app.new_trigger.tunnel_callback = dsmccSettings.tunnel;
                }

                /* send msg to app */
                ret = b_cdl_msg_send_to_app(&msg);
                if ( ret < 0) {
                    BDBG_ERR(("Failed to send msg to app, abort"));
                    goto wait_for_cvt;
                }
            }
        }
        if (h_cvt == NULL) goto wait_for_cvt;
        while (1) {        
            /* when we get here, a cdl_msg has been sent to app (inband download), 
             * or a tftp CDL needs to start.
             */

            switch (h_cvt->method) {
            case CDL_DLD_INBAND_SRC_ID:
            case CDL_DLD_INBAND_PID:
            case CDL_DLD_INBAND_PROG:
            case CDL_DLD_DSG_ADVANCED:
                /*
                 * check to see if app grant a pending deferred download request
                 */
                memset(&msg, 0, sizeof(msg));
                msg.type = B_CDL_MSG_FROM_APP_DOWNLOAD_GRANT;
                if (b_cdl_msg_recv_from_app(&msg) > 0) {
                    /* got the grant, start downloading */
                    BDBG_MSG(("got grant msg from app\n"));
                } else {
                    goto sleep1;
                }
                
                if (msg.from_app.download_grant.granted) {
                    pidChannel = (NEXUS_PidChannelHandle)msg.from_app.download_grant.pid_channel;
                    if ( (NULL == pidChannel) && (CDL_DLD_DSG_ADVANCED != h_cvt->method ) ) {
                        BDBG_ERR(("Invalid pidChannel %p", pidChannel));
                        goto wait_for_cvt;
                    }
                } else {
                    BDBG_ERR(("download denied"));
                    h_cvt = NULL;
                    goto wait_for_cvt;
                }

                if (download_carousel(h_cvt->method, h_cvt->code_filename, (int) h_cvt->code_filename_length) >= 0) {
                    {
#if 0
                        char str[64];
                        
                        /*
                         * Since we don't have data carousel headend, we simply copy the new fw
                         * to simulate the dsm_cc data carouse;
                         */
                        BDBG_ERR(("TODO: simulating data carousel downloading"));
                        
                        sprintf(str, "cp -f %s  %s", s_params.debug.fn_sec_header, s_params.storage_file_downloaded );
                        printf("Simulate dsmm_cc download by %s\n", str);
                        system(str);
                        BDBG_ERR(("sleep 3 sec\n"));
                        sleep(3);
                        /* send to eCM to authenticate digital signature */
                        
                        /*
                         * send to eCM to authenticate digital content,
                         */
#endif
                        
                    }
                    
                    /* wake up post download processing */
                    ret = post_download_processing();
                    if (ret >= 0) {
                        downloaded_obj++;
                    }
                    BDBG_ERR((" [%d] %s has been downloaded", downloaded_obj, h_cvt->code_filename)); 
                } else {
                    BDBG_ERR(("inband pid/program download fail\n"));
                    goto wait_for_cvt;
                }
                break;
            case CDL_DLD_TFTP_BY_CVT:
                BDBG_ERR(("TFTP by cvt\n"));
                if (download_tftp_by_cvt(h_cvt) >= 0) {
                    /* wake up post download processing */
                    ret = post_download_processing();
                    if (ret >= 0) {
                        downloaded_obj++;
                    }
                    BDBG_ERR((" [%d] %s has been downloaded", downloaded_obj, h_cvt->code_filename)); 
                } else {
                    BDBG_ERR(("tftp by cvt download fail\n"));
                    goto wait_for_cvt;
                }
                
                break;
            default:
                BDBG_ERR(("Unsupported download method %d\n", h_cvt->method));
                break;
            }
            /* find next object that needs to be downloaded */
            do {
                if ( (h_cvt = cdl_adpu_cvt_get_next_object()) == NULL) {
                    BDBG_MSG(("No more objects in CVT\n"));
                    break;
                }
                if (cdl_ctl_need_upgrade(h_cvt)) {
                    BDBG_MSG(("fw upgrade is needed"));
                    break;                    
                } else {
                    BDBG_MSG(("no fw upgrade is needed"));
                }
            } while (h_cvt);

            if (h_cvt == NULL) {
                if (downloaded_obj) {
                    /* the whole CVT has been scanned, some objects has been upgraded */
                    break;
                } else {
                    /* no object is upgraded*/
                    goto wait_for_cvt; 
                }
            }
            /* 
             * inband download needs grant from app,
             * tftp download will call EstbSwUpgrade_IsUpgradePermittedNow() later
             */
            
            if (h_cvt->method == CDL_DLD_INBAND_SRC_ID ||
                h_cvt->method == CDL_DLD_INBAND_PID ||
                h_cvt->method == CDL_DLD_INBAND_PROG ||
                h_cvt->method == CDL_DLD_DSG_ADVANCED) {
                BDBG_MSG(("got new cvt object"));
                memset(&msg, 0, sizeof(msg));
                msg.type = B_CDL_MSG_TO_APP_NEW_TRIGGER;
                memcpy(&msg.to_app.new_trigger.cvt, h_cvt, sizeof(msg.to_app.new_trigger.cvt));

                if (h_cvt->method == CDL_DLD_DSG_ADVANCED) {
                    B_Dsmcc_Settings dsmccSettings;

                    /* setup dsmcc settings */
                    B_Dsmcc_GetDefaultSettings(&dsmccSettings);
                    msg.to_app.new_trigger.tunnel_callback = dsmccSettings.tunnel;
                }

                /* send msg to app */
                    
                ret = b_cdl_msg_send_to_app(&msg);
                if ( ret < 0) {
                    BDBG_ERR(("Failed to send msg to app, abort"));
                    goto wait_for_cvt;
                }
            }
            /* wait for the app to grant */
        sleep1:
            BKNI_Sleep(1000); /* sleep for 1 sec */
        }
        if (downloaded_obj > 0) {
            /* 
             * what to do with the components that are not upgraded?
             * copy new component from existing bank to new bank ???
             * or write a script to handle it?
             */
            /* update boot parameter  */
            if (s_params.bootinfo_if.ready_to_reboot() < 0 ) return -1;
            BDBG_ERR(("\n*********** Rebooting system to complete firmware upgrade ***********\n"));
            system("reboot");
        }
	wait_for_cvt:
		/*
		cdl_dsmcc_close(h_dsmcc);
		if (buf) free(buf);
		*/
		cdl_ctl_set_state(CDL_CTL_ST_WAIT_TRIGGER);
	}
	return ret;
}
int trigger_by_estb(void) {
	int ch;


	/* cablecard is expected to be ready by now */
	/* there's no difference between trigger by cablecard or dsg delivered cvt */
	cdl_ctl_set_trigger(CDL_CTL_TRIGGER_CA_CVT | CDL_CTL_TRIGGER_DSG_CVT);
	cdl_create_thread(&wait_for_ca_cvt_thread_info, (void*)wait_for_ca_cvt, (void *)NULL);

	return 0;
}
#endif /* MPOD_SUPPORT */


#if defined(TEST_INBAND_DOWNLOAD)
/* return >= 0 if succeed, < 0 if fail */ 
 int test_inband_download(NEXUS_PidChannelHandle  pid_channel, unsigned char * name, int name_len) { 

	int ret = 0, n = 0;
	int module_id = 1;
	uint8_t * buf = NULL;
	int fd = -1;
	int len = 0;
	int sec_header_valid = 0;
	int sec_header_len = 0;
    B_Dsmcc_Handle dsmccHandle = NULL;
    B_Dsmcc_Settings dsmccSettings;

	if (pid_channel == NULL) {
		BDBG_ERR(("tuner not available"));
		return -1;
	}

    /* setup dsmcc settings */
    B_Dsmcc_GetDefaultSettings(&dsmccSettings);
    dsmccSettings.pidChannel = pid_channel;

    /* open dsmcc */
    dsmccHandle = B_Dsmcc_Open(&dsmccSettings);
    if (dsmccHandle == NULL) 
    {
        BDBG_ERR(("Dsmcc open failed"));
#if defined(MPOD_SUPPORT)
        B_Dsmcc_GetStatus(&dsmcc_status);
        if (B_DSMCC_ERR_MODULE_NOT_FOUND == dsmcc_status.error) 
            cdl_adpu_send_host_download_control(B_MPOD_DL_DOWNLOAD_NOTIFY_HEADEND);
#endif
        return -1;
    }


	/* write image */
	if ( (buf = (uint8_t *)malloc(BUF_SIZE)) == NULL) {
		BDBG_ERR(("malloc failed\n"));
		ret = -1;
		goto out;
	}

	module_id = B_Dsmcc_GetModuleIdFromName(dsmccHandle, name, name_len);
    if (module_id < 0) {
        ret = -1;
#if defined(MPOD_SUPPORT)
        B_Dsmcc_GetStatus(&dsmcc_status);
        if (B_DSMCC_ERR_MODULE_NOT_FOUND == dsmcc_status.error) 
            cdl_adpu_send_host_download_control(B_MPOD_DL_DOWNLOAD_NOTIFY_HEADEND);
#endif
        goto out;
    }
	if ((fd = open(s_params.storage_file_downloaded, O_WRONLY | O_CREAT | O_TRUNC)) < 0) {
		BDBG_ERR(("cannot open %s\n", s_params.storage_file_downloaded));
		ret = -1;
		goto out;
	}
	len = 0;

	while ((n = B_Dsmcc_Read(dsmccHandle, module_id, buf, BUF_SIZE)) > 0) {
        if ( (ret = write(fd, buf, n)) < n) {
            BDBG_ERR(("Write error, write %d, expect %d bytes\n", ret, n));
            ret = -1;
            goto out;
        }
        BDBG_ERR(("Write %d bytes at %d", n, len));
		len += n;
    }

    if (ret == 0) {
        ret = 0;
        BDBG_MSG(("donwload inband succeed\n"));
    } else if (ret < 0) {
        ret = -1;
        BDBG_ERR(("donwload inband failed\n"));
    }
 out:
	if (fd >= 0)close(fd);
    if (dsmccHandle) B_Dsmcc_Close(dsmccHandle);
	if (buf) free(buf);
	return ret;
}
#endif /* TEST_INBAND_DOWNLOAD */


/*
 * The processing include image authentication (CVT triggered only), copy monolithic image
 * to flash (all), update nvram (all) and reboot (all).
 */
#ifdef DSG_SUPPORT
void post_download_processing(void) {
	uint8_t method;

    /*
     * Waiting for the monolithic image download to finish, and do post_download processing.
     */
    if (post_download_processing_thread_info.exit == 1) return;
    if (s_params.debug.authenticate_only)
        return;
    method = cdl_ctl_get_method();
    if (method == CDL_DLD_SIMU) {
        /*use cdltest to generate image, this option is obsolete */
        BDBG_ERR(("method %d is not supported", method));
        return;
    } else if ( method == CDL_DLD_INBAND_SRC_ID
			    || method == CDL_DLD_INBAND_PID
			    || method == CDL_DLD_INBAND_PROG
			    || method == CDL_DLD_DSG_BASIC
			    || method == CDL_DLD_DSG_ADVANCED) {
        /* authenticate image, needed for carousel download only */
        BDBG_ERR(("Authenticating %s", s_params.storage_file_downloaded));
        if (cdl_service_authenticate_image(0) < 0) {
            BDBG_ERR(("Authenticate image failed\n"));
            return;
        }
    } else {
        BDBG_ERR(("%s downloaded by tftp, method %d, skip authenticate",
				  s_params.storage_file_downloaded, method));
    }
    
    if (cdl_ctl_get_state() < CDL_CTL_ST_DOWNLOAD ) {
        BDBG_ERR(("download failed, waiting for new download state %d", cdl_ctl_get_state()));
        return;
    }
    /*
     * For both tftp and carousel download,
     * the secure header has already been stripped off during downloading
     */
    
    if (cdl_service_upgrade_image(0) < 0) {
        return;
    }
}

/*
 * post download processing,
 * - (CVT data carousel only) image authentication
 * - (all) update nvram and reboot
 */
int start_post_download_daemon(void) {
        return cdl_create_thread(&post_download_processing_thread_info, (void*)post_download_processing, (void *)NULL);
}


/* return 0 if succeed, -1 if fail */
int cdl_image_download(void) {
	char * str;
	int ret = 0;

	/*
	 * there's really not much difference between triggered by ecm config file or snmp.
	 * The real difference is triggered by ecm (config file, snmp) or by estb (cvt).
	 */
	if ((str = getenv(CDL_ENV_TRIGGER)) == NULL) {
		str = CDL_ENV_TRIGGER_DFT;
		BDBG_ERR(("env %s not set, use default %s", CDL_ENV_TRIGGER, str));
	}
	if (strstr(str, "ecm")) {
		trigger_by_ecm();
	}

#if defined(MPOD_SUPPORT)
	if (strstr(str, "estb")) {
		trigger_by_estb();
	}
#endif

	start_post_download_daemon();

	return ret;
}
#endif


void closeall(void) {
	uint8_t trigger;

	B_Cdl_Stop();
#ifdef DSG_SUPPORT

	/* cdl_kill_thread(&post_download_processing_thread_info, 0);  */
	cdl_kill_thread(&wait_for_ca_cvt_thread_info, 0);

	trigger = cdl_ctl_get_trigger();
	if ( trigger & CDL_CTL_TRIGGER_ECM_CFG_FILE) {
		cdl_estb_close();
	}

	if ( trigger & CDL_CTL_TRIGGER_CA_CVT) {
	}
	cdl_ctl_set_trigger(CDL_CTL_TRIGGER_NONE);
#endif
}
 static void cdl_image_init(B_Cdl_Settings * params) {
	 memset(&params->image_if, 0, sizeof (params->image_if));
	 params->image_if.print_header = cdl_image_brcm_print_header;
	 params->image_if.generate = cdl_image_brcm_generate;
	 params->image_if.verify = cdl_image_brcm_verify;
	 params->image_if.check_integrity = cdl_image_brcm_check_integrity;
 }

#ifdef HOST_ONLY
 static int cdl_bootinfo_dummy_open (char *pathname) { return 1;}
 static int cdl_bootinfo_dummy_close (void) { return 1;}
 static int cdl_bootinfo_dummy_just_started (void) { return 1;}
 static int cdl_bootinfo_dummy_ready_to_reboot (void) {return 1;}
 static int cdl_bootinfo_dummy_get_new_storage (char * fn_kernel, char * fn_bootloader) {return 1;}
 static char * cdl_bootinfo_dummy_get_dev_name(void) { return NULL;}
#endif

 static void cdl_bootinfo_init(B_Cdl_Settings * params) {

	 memset(&params->bootinfo_if, 0, sizeof (params->bootinfo_if));
#ifdef HOST_ONLY
	 params->bootinfo_if.open = cdl_bootinfo_dummy_open;
	 params->bootinfo_if.close = cdl_bootinfo_dummy_close;
	 params->bootinfo_if.just_started = cdl_bootinfo_dummy_just_started;
	 params->bootinfo_if.ready_to_reboot = cdl_bootinfo_dummy_ready_to_reboot;
	 params->bootinfo_if.get_new_storage = cdl_bootinfo_dummy_get_new_storage;
	 params->bootinfo_if.get_dev_name = cdl_bootinfo_dummy_get_dev_name;

#else
	 params->bootinfo_if.open = cdl_bootinfo_nvram_open;
	 params->bootinfo_if.close = cdl_bootinfo_nvram_close;
	 params->bootinfo_if.just_started = cdl_bootinfo_nvram_just_started;
	 params->bootinfo_if.ready_to_reboot = cdl_bootinfo_nvram_ready_to_reboot;
	 params->bootinfo_if.get_new_storage = cdl_bootinfo_nvram_get_new_storage;
	 params->bootinfo_if.get_dev_name = cdl_bootinfo_nvram_get_dev_name;
#endif
 }
static void cdl_bootinfo_uninit(void) {
	if (s_params.bootinfo_if.close)
		s_params.bootinfo_if.close();
	 return;
 }

static void cdl_storage_init(B_Cdl_Settings * params) {
	 memset(&params->storage_if, 0, sizeof (params->storage_if));
#ifdef HOST_ONLY
	 params->storage_if.open = cdl_storage_file_open;
	 params->storage_if.close = cdl_storage_file_close;
	 params->storage_if.read = cdl_storage_file_read;
	 params->storage_if.write = cdl_storage_file_write;
	 params->storage_if.lseek = cdl_storage_file_lseek;
	 params->storage_if.expand = cdl_storage_file_expand;
#else
     /*
      * use ramdisk to save and expand mono image.
      */
	 params->storage_if.open = cdl_storage_flash_open;
	 params->storage_if.close = cdl_storage_flash_close;
	 params->storage_if.read = cdl_storage_flash_read;
	 params->storage_if.write = cdl_storage_flash_write;
	 params->storage_if.lseek = cdl_storage_flash_lseek;
	 params->storage_if.expand = cdl_storage_flash_expand;
#endif
 }

 static void cdl_storage_uninit(void) {
 }

void B_Cdl_Init(B_Cdl_Settings * params) {
    if (params == NULL) {
        B_Cdl_GetDefaultSettings(&s_params);
    } else {
        memcpy(&s_params, params, sizeof(s_params));
    }

	memset(&wait_for_ca_cvt_thread_info, 0 , sizeof(cdl_thread_info));
	memset(&post_download_processing_thread_info, 0 , sizeof(cdl_thread_info));

#ifdef DSG_SUPPORT
	cdl_ctl_init();
    cdl_msg_init();
	cdl_estb_init();
#endif
#if defined(MPOD_SUPPORT)
	cdl_adpu_init();
	pidChannel = NULL;
#endif

	/*
	 * extract the parameters.
	 */
	snprintf(&s_params.bootinfo_dev_kernel[0], MAX_DEV_STR_LEN, "%s",
		 s_params.bootinfo_if.get_dev_name());

	s_params.bootinfo_if.open(s_params.bootinfo_dev_kernel);

	/* start checking CDL status */
	s_params.bootinfo_if.just_started();

	s_params.bootinfo_if.get_new_storage(s_params.storage_kernel,
					     s_params.storage_bootloader);
    /* for non-tftp download, temporary downloaded file with secure header */
	snprintf(&s_params.storage_file_downloaded[0], MAX_DEV_STR_LEN, "%s",
		 "./mono_downloaded.bin");
#ifdef DEBUG_SEC_HEADER
	snprintf(&s_params.debug.fn_sec_header[0], MAX_DEV_STR_LEN, "%s", "./sec_header.bin");
    BDBG_WRN(("%s: %s is used to simulate security header check", __FUNCTION__, s_params.debug.fn_sec_header));
#endif
	BDBG_ERR(("storage device %s, %s, temp %s", s_params.storage_kernel,
              s_params.storage_bootloader, s_params.storage_file_downloaded));
#ifdef DSG_SUPPORT
	cdl_estb_set_monolithic_image_temp_file_name(s_params.storage_kernel);
#endif


#if defined (MPOD_SUPPORT) 
    /* callback needs to be set after cdl_msg_init() is called */
    if (s_params.cdl_msg_callback)
        cdl_msg_set_callback(s_params.cdl_msg_callback, s_params.cdl_msg_callback_context);
#endif
}
 void B_Cdl_GetDefaultSettings(B_Cdl_Settings * params)
 {
     memset(params, 0, sizeof(B_Cdl_Settings));
     cdl_image_init(params);
     cdl_bootinfo_init(params);
     cdl_storage_init(params);
 }

 void B_Cdl_GetSettings(B_Cdl_Settings * params) {
	 memcpy(params, &s_params, sizeof(s_params));
 }
 void B_Cdl_SetSettings(B_Cdl_Settings * params) {
	 memcpy(&s_params, params, sizeof(s_params));
#if defined (MPOD_SUPPORT) 
     cdl_msg_set_callback(s_params.cdl_msg_callback, s_params.cdl_msg_callback_context);
#endif
 }


int B_Cdl_Start(void) {


#ifdef DSG_SUPPORT
	cdl_estb_set_cb_check_integrity(s_params.image_if.check_integrity);

	/*
	 * enable download methods, multiple methods can be enabled.
	 * But once one download starts, other methods are blocked.
	 */
	cdl_image_download();
#endif
	return 1;
}

int B_Cdl_Stop(void) {
	return 1;
}
static void cdl_image_uninit(void) {
	 return;
 }

void B_Cdl_Uninit(void) {

	closeall();

	cdl_storage_uninit();
	cdl_bootinfo_uninit();
	cdl_image_uninit();

#if defined(DSG_SUPPORT)
#if defined(MPOD_SUPPORT)
	cdl_adpu_uninit();
#endif
	cdl_estb_uninit();
    cdl_msg_uninit();
	cdl_ctl_uninit();
#endif
}

#ifndef HOST_ONLY
int B_Cdl_Get_Status(B_Cdl_Status * pStatus) {
    int ret = -1;
#ifdef MPOD_SUPPORT
    B_Dsmcc_Status dsmcc_status;
#endif
    ret = cdl_ctl_get_status(pStatus);
    if (ret < 0) {
        BDBG_ERR(("Cannot get CDL Status"));
        return ret;
    }
#ifdef MPOD_SUPPORT
    ret = B_Dsmcc_GetStatus(&dsmcc_status);
    if (ret < 0) {
        BDBG_ERR(("Cannot get DSMCC Status"));
        return ret;
    }
    pStatus->inbandCarouselTimeoutCount = dsmcc_status.inbandCarouselTimeoutCount;
    pStatus->inbandCarouselTimeoutUs = dsmcc_status.inbandCarouselTimeoutUs;
#endif
    return 1;
}
#endif

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
 * Module Description:  OpenCable Common Download control interface. Control
 *                      and status info
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "cdl_common.h"
#include "b_cdl_lib.h"
#include "cdl_ctl.h"
#ifdef ESTB_CFG_SUPPORT
#include "b_estb_cfg_lib.h"
#endif

#include "cdl_image_brcm.h"

BDBG_MODULE(cdl_ctl);


/* 
 * In CDL I09, the CVT version 2 can contain multiple monolithic images, each image is 
 * called  an object, with its own (object type, object data) to identify it.
 * 
 * Also the new monolithic for that object is represented by its own obj_filename.
 * How to parse the object is vendor-specific, here's how we define our objects.
 *  object 0 : For ver 2, a monolithic image for object 0 contains all the fw components, 
 *             For ver 1, this is the only object supported.
 *  object 1-7: A monolithic image for these objects contains only one fw component. 
 *             See below for details.
 *              
 */
#define OBJ_ID_MAX 8
static char fw_name[OBJ_ID_MAX][64] = {
    "/dyn/estb/fw_name", /* for ver 1 and ver 2,  mono image is for all objects */
    "/dyn/estb/fw_name_1", /* for ver 2 only,object is ecmboot */
    "/dyn/estb/fw_name_2", /* for ver 2 only,object is docsis */
    "/dyn/estb/fw_name_3", /* for ver 2 only,object is kernel */
    "/dyn/estb/fw_name_4", /* for ver 2 only,object is rootfs */
    "/dyn/estb/fw_name_5", /* for ver 2 only,object is app */
    "/dyn/estb/fw_name_6", /* for ver 2 only, object is CFE */
    "/dyn/estb/fw_name_7"  /* for ver 2 only,object is splash */
};
static char new_fw_name[64] = "/dyn/estb/new_fw_name";

struct cdl_ctl {
	uint32_t size; /* size of the data structure */
	uint8_t ver; /* version of the ctl block */
	uint8_t trigger; /* active trigger */
	uint8_t res[2]; /* padding */

	uint8_t cmd; /* download now or deferred download */
	uint8_t method; /* download method defined as CDL_DLD_xxx */
	int state;  /* state machine defined as CDL_CTL_ST_XXX, >0 ok, <0 error */

	pthread_mutex_t m_ecm; 
	pthread_cond_t c_ecm;

	uint32_t vendor_id; /* 24 bit OUI, needed by host_info_response */
	uint32_t hw_version_id; /* needed by host_info_response */
	uint32_t group_id; /* needed by cvt */

	char fw_name[OBJ_ID_MAX][MAX_DEV_STR_LEN];

	B_Cdl_Status status;

	int secure_header_len; 
    int download_count; 
	char new_fw_name[MAX_DEV_STR_LEN];
	int allow_unsigned_image; /* for debug purpose, allow unsigned image */
};

static struct cdl_ctl s_ctl;
static int cdl_ctl_load_status_from_nonvol(void);
static int cdl_ctl_set_status(int state) ;
static int cdl_ctl_get_download_count(int * download_count);
static int cdl_ctl_set_download_count(int download_count);
static int cdl_ctl_get_status_code_download_status(int *code_download_status) ;
static int cdl_ctl_set_status_code_download_status(int code_download_status);
static int print_params(void) {
    int len;
	BDBG_MSG(("/**** current params *****\n"));
	BDBG_MSG(("vendor_id = 0x%x\n", s_ctl.vendor_id));
	BDBG_MSG(("hw_version_id = 0x%x\n", s_ctl.hw_version_id));
#ifdef ESTB_CFG_SUPPORT
	if ( (B_Estb_cfg_Get_uint32("/dyn/estb/group_id", &s_ctl.group_id))  < 0) {
		return -1;
	}
#endif
	BDBG_MSG(("group_id = %d\n", s_ctl.group_id));
#ifdef ESTB_CFG_SUPPORT
	memset(s_ctl.fw_name, 0, MAX_DEV_STR_LEN);
	if ( (B_Estb_cfg_Get_txt("/dyn/estb/fw_name", s_ctl.fw_name[0], &len))  < 0) {
		return -1;
	}
#endif
	BDBG_MSG(("firmware_file_name = %s\n", s_ctl.fw_name));
	return 0;
}
static void cdl_ctl_init_status(void) {
    s_ctl.status.imageStatus = e_ImageStatusImageAuthorized;
    s_ctl.status.codeDownloadStatus = e_CodeDownloadStatusComplete;
    s_ctl.status.downloadFailedStatus = e_DownloadFailedStatusCdlError1;
    s_ctl.status.inbandCarouselTimeoutCount = 0;
    s_ctl.status.inbandCarouselTimeoutUs = 0;
    memset(s_ctl.status.name, 0, sizeof(s_ctl.status.name));

    cdl_ctl_load_status_from_nonvol();
}
int cdl_ctl_init(void) {
	int ret = -1;
	int len;
	char *str;
	BDBG_MSG(("Enter %s\n", __FUNCTION__));
	memset(&s_ctl, 0, sizeof(s_ctl));

    BDBG_WRN(("Allow unsigned image"));
    s_ctl.allow_unsigned_image  = 1;

    cdl_ctl_init_status();

#ifdef ESTB_CFG_SUPPORT
	if ( (B_Estb_cfg_Get_uint32("/perm/estb/vendor_id", &s_ctl.vendor_id))  < 0) {
		return -1;
	}
	if ( (B_Estb_cfg_Get_uint32("/perm/estb/hw_version_id", &s_ctl.hw_version_id))  < 0) {
		return -1;
	}
	if ( (B_Estb_cfg_Get_uint32("/dyn/estb/group_id", &s_ctl.group_id))  < 0) {
		return -1;
	}
	memset(s_ctl.fw_name, 0, MAX_DEV_STR_LEN);
	if ( (B_Estb_cfg_Get_txt("/dyn/estb/fw_name", s_ctl.fw_name[0], &len))  < 0) {
		return -1;
	}
	BDBG_WRN(("vendor id 0x%x, hw_Version_id 0x%x, group id 0x%x, fw_name %s\n", 
              s_ctl.vendor_id, s_ctl.hw_version_id, s_ctl.group_id, s_ctl.fw_name));

    if (cdl_ctl_get_download_count(&s_ctl.download_count) < 0) {
		return -1;
	}
    BDBG_WRN(("Download count %d", s_ctl.download_count));
#else /* ESTB_CFG_SUPPORT */
	/*
	 * These parameters are generally imported from somewhere else
	 */
	if ((str = getenv(CDL_ENV_VENDOR_ID))== NULL) {
		str = CDL_ENV_VENDOR_ID_DFT;
	}
	s_ctl.vendor_id = strtoul(str, NULL, 0);

	if ((str = getenv(CDL_ENV_HW_VERSION_ID))== NULL) {
#ifdef BCHP_CHIP
		s_ctl.hw_version_id = BCHP_CHIP;
#else
		s_ctl.hw_version_id = 0xdeadbeef;
#endif
	} else {
		s_ctl.hw_version_id = strtoul(str, NULL, 0);
	}
	if ((str = getenv(CDL_ENV_FW_NAME))== NULL) {
		str = CDL_ENV_FW_NAME_DFT;
	}
	snprintf(s_ctl.fw_name, 64, "%s", str);
#endif /* ESTB_CFG_SUPPORT */
	print_params();
	pthread_mutex_init(&s_ctl.m_ecm, NULL);
	pthread_cond_init(&s_ctl.c_ecm, NULL);
	return 0;
}

int cdl_ctl_uninit(void){
	BDBG_MSG(("Enter %s\n", __FUNCTION__));
	/* memset(&s_ctl, 0, sizeof(s_ctl)); */
	return 0;
}

int cdl_ctl_get_state(void) {
	return s_ctl.state;
}
uint8_t cdl_ctl_get_method(void) {
	return s_ctl.method;
}
uint8_t cdl_ctl_get_cmd(void) {
	return s_ctl.cmd;
}
uint8_t cdl_ctl_get_trigger(void) {
	return s_ctl.trigger;
}

uint32_t cdl_ctl_get_vendor_id(void) {
	BDBG_MSG(("Vendor ID 0x%x", s_ctl.vendor_id & 0x00ffffff));
	return (s_ctl.vendor_id & 0x00ffffff);
}

uint32_t cdl_ctl_get_hw_version_id(void) {
	BDBG_MSG(("HW version ID 0x%x", s_ctl.hw_version_id));
	return s_ctl.hw_version_id;
}

uint32_t cdl_ctl_get_group_id(void) {
#ifdef ESTB_CFG_SUPPORT
	if ( (B_Estb_cfg_Get_uint32("/dyn/estb/group_id", &s_ctl.group_id))  < 0) {
		return 0xffffffff;
	}
#endif
	BDBG_MSG(("Group ID 0x%x", s_ctl.group_id));
	return s_ctl.group_id;
}

/* return >=0 if succeed, <0 if fail */
int cdl_ctl_set_state(int state) {
    int ret;
	if (state >= CDL_CTL_ST_MAX || state <= -CDL_CTL_ST_MAX) {
		BDBG_ERR(("Invalid state %d\n", state));
		return -1;
	}
	s_ctl.state = state;
    ret = cdl_ctl_set_status(state);
	return ret;
}


void cdl_ctl_set_failed_state(void) {
	if (s_ctl.state > 0) 
		s_ctl.state = -s_ctl.state;
	else {
		BDBG_ERR(("already in failed state %d\n", s_ctl.state));
	}
}

/* return 1 if succeed, 0 if fail */
int cdl_ctl_set_method(uint8_t method) {
	if (method >= CDL_DLD_MAX) {
		BDBG_ERR(("Invalid method %d\n", method));
		return 0;
	}
	if (cdl_ctl_is_busy()) {
		BDBG_ERR(("Busy downloading, cannot change method\n"));
		return 0;		
	}
	/* fixme! how about a downloading is in deferred waiting? */
	/* fixme! clear control block? */
	s_ctl.method = method;
	
	return 1;
}

/* return 1 if succeed, 0 if fail */
int cdl_ctl_set_cmd(uint8_t cmd) {
	if (cdl_ctl_is_busy()) {
		BDBG_ERR(("Busy downloading, cannot change command\n"));
		return 0;		
	}
	if (cmd > CDL_CTL_CMD_DLD_DEFER) {
		BDBG_ERR(("Invalid command %d\n", cmd));
		return 0;
	}
	/* fixme! what if it's deferred download? */
	s_ctl.cmd = cmd;
	return 1;
}

/* return 1 if succeed, 0 if fail */
int cdl_ctl_set_trigger(uint8_t trigger) {
	if (trigger == CDL_CTL_TRIGGER_NONE) {
		s_ctl.trigger = 0;
	} else {
		s_ctl.trigger |= trigger;
	}
	return 1;
}

int cdl_ctl_is_busy(void) {
	/* fixme! how about deferred download */
	BDBG_MSG(("ctl state %d\n", s_ctl.state));
	return ( s_ctl.state >= CDL_CTL_ST_DOWNLOAD && s_ctl.state < CDL_CTL_ST_SUCCEED );
}


int cdl_ctl_wait_for_ecm_notification(void) {
	int ret = 0;

	pthread_mutex_lock(&s_ctl.m_ecm); 
	ret = pthread_cond_wait(&s_ctl.c_ecm, &s_ctl.m_ecm);
	pthread_mutex_unlock(&s_ctl.m_ecm);
	return ret;
}


int cdl_ctl_wakeup_by_ecm_notification(void) {
	int ret = 0;
	pthread_mutex_lock(&s_ctl.m_ecm);
	ret = pthread_cond_broadcast(&s_ctl.c_ecm);
	pthread_mutex_unlock(&s_ctl.m_ecm);	
	return ret;
}

int cdl_ctl_need_authenticate(void) {
	int method = s_ctl.method;
    if (cdl_ctl_allow_unsigned_image()) {
        BDBG_WRN(("Allow unsigned image, no authentication needed"));
        return 0;
    }
	/* DSM_CC download needs authenticate */
	return ( method == CDL_DLD_INBAND_SRC_ID
	    || method == CDL_DLD_INBAND_PID 
	    || method == CDL_DLD_INBAND_PROG 
	    || method == CDL_DLD_DSG_BASIC 
	    || method == CDL_DLD_DSG_ADVANCED ) ? 1 : 0;
}
static int cdl_ctl_get_obj_id(cdl_adpu_cvt_t h_cvt ) {
    int obj_id = -1;
    if (h_cvt->version == 1) {
        /* always return obj 0 for version 0 */
        obj_id = 0;
    } else if (h_cvt->version == 2) {
        /*
         * TODO: How to parse obj_data shou be defined by vendor
         * this is just an example, we simply use a string to represent the obj id in obj_data
         * "0x2" represents obj 2
         */
        if ( h_cvt->obj_data_length >= 127) {
            BDBG_ERR(("obj_data_len %d too big, max 127", h_cvt->obj_data_length ));
            return -1;
        }
        if (!(h_cvt->obj_type == 0x00 && h_cvt->obj_type == 0x01)) {
            BDBG_ERR(("Unsupported object type %x", h_cvt->obj_type));
            return -1;
        }
        if (h_cvt->obj_data_length ) {
            h_cvt->obj_data[h_cvt->obj_data_length + 1] = 0;
            obj_id = strtol(h_cvt->obj_data, NULL, 0);
        }
        if (obj_id < 0 || obj_id >= OBJ_ID_MAX) {
            BDBG_ERR(("Invalid obj id %d", obj_id));
            return -1;
        }
    }
    return obj_id;
}
/* 
 * return <=0 if no need to upgrade
 * return 1 if need to upgrade
 */
int cdl_ctl_need_upgrade(cdl_adpu_cvt_t h_cvt) {
    int obj_id = 0;
    int len = 0;
    int need_upgrade = 0;

    obj_id = cdl_ctl_get_obj_id(h_cvt);
    if (obj_id < 0 || obj_id >= OBJ_ID_MAX) {
        BDBG_ERR(("Cannot find the object"));
        return 0;
    }

#ifdef ESTB_CFG_SUPPORT

	memset(s_ctl.fw_name[obj_id], 0, MAX_DEV_STR_LEN);
	if ( (B_Estb_cfg_Get_txt(fw_name[obj_id], s_ctl.fw_name[obj_id], &len))  < 0) {
        BDBG_ERR(("Can not read %s", fw_name[obj_id]));
		return -1;
	}
#endif

	BDBG_MSG(("new image name %s [len %d], current image name %s [len %d], obj_id %d", 
              h_cvt->code_filename, h_cvt->code_filename_length, s_ctl.fw_name[obj_id], len, obj_id));
    need_upgrade = ( (strncmp(h_cvt->code_filename, s_ctl.fw_name[obj_id], h_cvt->code_filename_length) == 0) &&
                     (len == h_cvt->code_filename_length) ) ? 0 : 1;
    if (need_upgrade == 0) return 0;

    /* now check to see if max download has reached */

#ifdef ESTB_CFG_SUPPORT

	memset(s_ctl.new_fw_name, 0, MAX_DEV_STR_LEN);
	if ( (B_Estb_cfg_Get_txt(new_fw_name, s_ctl.new_fw_name, &len))  < 0) {
        BDBG_ERR(("Can not read %s", new_fw_name));
		return -1;
	}
#endif

	BDBG_MSG(("new image name %s [len %d], last failed image name %s [len %d], obj_id %d", 
              h_cvt->code_filename, h_cvt->code_filename_length, s_ctl.new_fw_name, len, obj_id));
    if ( (strncmp(h_cvt->code_filename, s_ctl.new_fw_name, h_cvt->code_filename_length) == 0) &&
         (len == h_cvt->code_filename_length) )  {
        /* same image, has it reached max download retry? */
        if ((s_ctl.download_count) < CDL_CTL_MAX_DOWNLOAD_RETRY) {
            /* download count is not increased until the download has started */
            BDBG_ERR(("Retry %d for %s [len %d]", s_ctl.download_count, s_ctl.new_fw_name, len));
            need_upgrade = 1;
        } else {
            BDBG_ERR(("Image %s has reached max download retry 3, can not upgrade", s_ctl.new_fw_name));
            cdl_ctl_set_state(CDL_CTL_ST_DLD_MAX_RETRY);
            return 0;
        }
    } else {
        /* first try of a new image, reset download_count */ 
        s_ctl.download_count = 0;
        need_upgrade = 1;
        BDBG_ERR(("New image %s [len %d] for download, current image %s [len %d]", h_cvt->code_filename, h_cvt->code_filename_length, s_ctl.fw_name[obj_id], len));
#ifdef ESTB_CFG_SUPPORT
        if (cdl_ctl_set_download_count(s_ctl.download_count) < 0) {
            BDBG_ERR(("Can not set download_count %d", s_ctl.download_count));
            return 0;
        }
        if ( (B_Estb_cfg_Set_txt("/dyn/estb/new_fw_name", h_cvt->code_filename, h_cvt->code_filename_length))  < 0) {
            return 0;
        }
        memcpy(s_ctl.new_fw_name, h_cvt->code_filename, sizeof(s_ctl.new_fw_name));
#endif
    }
    BDBG_ERR(("Need upgrade"));
    return 1;
}

int cdl_ctl_get_secure_header_len(void) {
	return s_ctl.secure_header_len;
}

void cdl_ctl_set_secure_header_len(int len) {
	s_ctl.secure_header_len = len;
}

/* return >= 0 if succeed, < 0 if fail */
int cdl_ctl_get_status_name(char * name) {
#ifdef ESTB_CFG_SUPPORT
    int len;
	if ( (B_Estb_cfg_Get_txt("/dyn/estb/new_fw_name", name, &len))  < 0) {
		return -1;
	}
#endif
	BDBG_MSG(("new fw name 0x%s", name));
	return 1;
}
/* return >= 0 if succeed, < 0 if fail */
int cdl_ctl_get_status_image_status(int *image_status) {
#ifdef ESTB_CFG_SUPPORT
	if ( (B_Estb_cfg_Get_uint32("/dyn/estb/image_status", (unsigned int *)image_status))  < 0) {
		return -1;
	}
#endif
	BDBG_MSG(("image_status 0x%x", *image_status));
	return 1;
}

/* return >= 0 if succeed, < 0 if fail */
static int cdl_ctl_get_status_code_download_status(int *code_download_status) {
#ifdef ESTB_CFG_SUPPORT
	if ( (B_Estb_cfg_Get_uint32("/dyn/estb/code_download_status", (unsigned int *)code_download_status))  < 0) {
		return -1;
	}
#endif
	BDBG_MSG(("code_download_status 0x%x", *code_download_status));
	return 1;
}

/* return >= 0 if succeed, < 0 if fail */
int cdl_ctl_get_status_download_failed_status(int *download_failed_status) {
#ifdef ESTB_CFG_SUPPORT
	if ( (B_Estb_cfg_Get_uint32("/dyn/estb/download_failed_status", (unsigned int *)download_failed_status))  < 0) {
		return -1;
	}
#endif
	BDBG_MSG(("image_status 0x%x", *download_failed_status));
	return 1;
}

/* return >= 0 if succeed, 
 *        < 0 if fail. 
 */
static int cdl_ctl_load_status_from_nonvol(void) {
    int ret = -1;
    
    /* The following needs to be retrieved from dyn cfg directly */
    ret = cdl_ctl_get_status_image_status(&s_ctl.status.imageStatus);
    if (ret < 0) {
        BDBG_ERR(("Cannot get image status"));
        return ret;
    }
    ret = cdl_ctl_get_status_code_download_status(&s_ctl.status.codeDownloadStatus);
    if (ret < 0) {
        BDBG_ERR(("Cannot get code download status"));
        return ret;
    }
    ret = cdl_ctl_get_status_download_failed_status(&s_ctl.status.downloadFailedStatus);
    if (ret < 0) {
        BDBG_ERR(("Cannot get download failed status"));
        return ret;
    }
    ret = cdl_ctl_get_status_name(s_ctl.status.name);
    if (ret < 0) {
        BDBG_ERR(("Cannot get name"));
        return ret;
    }
    return 1;
}


/* return >= 0 if succeed, 
 *        < 0 if fail. 
 */
int cdl_ctl_get_status(B_Cdl_Status * pStatus) {
    int ret = -1;
    
    if (cdl_ctl_load_status_from_nonvol() < 0) {
        BDBG_ERR(("Failed to load status from nonvol"));
        return -1;
    }
    memcpy(pStatus, &s_ctl.status, sizeof(s_ctl.status));
    return 1;
}

/* return >= 0 if succeed, < 0 if fail */
int cdl_ctl_set_status_image_status(int image_status) {
#ifdef ESTB_CFG_SUPPORT
    int ret = 0;
    int old_image_status = 0;
    ret = cdl_ctl_get_status_image_status(&old_image_status);
    if (ret < 0) {
        return ret;
    } 
    if (old_image_status == image_status) goto out;

	if ( (ret = B_Estb_cfg_Set_uint32("/dyn/estb/image_status", (unsigned int) image_status))  < 0) {
		return ret;
	}
 out:
#endif
	BDBG_MSG(("Set image_status 0x%x", image_status));
	return 1;
}

/* return >= 0 if succeed, < 0 if fail */
static int cdl_ctl_set_status_code_download_status(int code_download_status) {
#ifdef ESTB_CFG_SUPPORT
    int ret = 0;
    int old_code_download_status = 0;
    ret = cdl_ctl_get_status_code_download_status(&old_code_download_status);
    if (ret < 0) {
        return ret;
    } 
    if (old_code_download_status == code_download_status) goto out;

	if ( (ret = B_Estb_cfg_Set_uint32("/dyn/estb/code_download_status", (unsigned int) code_download_status))  < 0) {
		return ret;
	}
 out:
#endif
	BDBG_MSG(("Set code_download_status 0x%x", code_download_status));
	return 1;
}

/* return >= 0 if succeed, < 0 if fail */
static int cdl_ctl_get_download_count(int * download_count) {
#ifdef ESTB_CFG_SUPPORT
	if ( (B_Estb_cfg_Get_uint32("/dyn/estb/download_count", (unsigned int*) download_count))  < 0) {
		return -1;
	}
#endif
	BDBG_MSG(("Set download_count 0x%x", *download_count));
	return 1;
}

/* return >= 0 if succeed, < 0 if fail */
static int cdl_ctl_set_download_count(int download_count) {
#ifdef ESTB_CFG_SUPPORT
    int ret = -1;
    int old_download_count = 0;
    ret = cdl_ctl_get_download_count(&old_download_count);
    if (ret < 0) {
        return ret;
    } 
    if (old_download_count == download_count) goto out;
	if ( (B_Estb_cfg_Set_uint32("/dyn/estb/download_count", (unsigned int) download_count))  < 0) {
		return ret;
	}
 out:
#endif
	BDBG_MSG(("Set download_count 0x%x", download_count));
	return 1;
}

/* return >=0 if succeed, <0 if fail */
static int cdl_ctl_set_status(int state) {
    int ret;

    if (state == s_ctl.status.codeDownloadStatus) return 1;
    if (state == CDL_CTL_ST_DOWNLOAD) {
        /* increase download count */
        if ((s_ctl.download_count) < CDL_CTL_MAX_DOWNLOAD_RETRY) {
            s_ctl.download_count++;
#ifdef ESTB_CFG_SUPPORT
            if (cdl_ctl_set_download_count(s_ctl.download_count) < 0) {
                BDBG_ERR(("Can not set download_count %d", s_ctl.download_count));
                return -1;
            }
#endif
            BDBG_ERR(("Retry %d for %s", s_ctl.download_count, s_ctl.new_fw_name));

        } else {
            BDBG_ERR(("Image %s has reached max download retry 3, can not upgrade", s_ctl.new_fw_name));
            return -1;
        }
        s_ctl.status.codeDownloadStatus = e_CodeDownloadStatusStarted;
    } else if (state == CDL_CTL_ST_CERT_FAILED) {
        s_ctl.status.imageStatus = e_ImageStatusCertFailure;
        s_ctl.status.codeDownloadStatus = e_CodeDownloadStatusFailed;
    } else if (state == CDL_CTL_ST_IMAGE_DAMAGED) {
        s_ctl.status.imageStatus = e_ImageStatusImageCorrupted;
        s_ctl.status.codeDownloadStatus = e_CodeDownloadStatusFailed;
    } else if (state == CDL_CTL_ST_DLD_COMPLETE) {
        s_ctl.status.imageStatus = e_ImageStatusImageAuthorized;
        s_ctl.status.codeDownloadStatus = e_CodeDownloadStatusComplete;
    } else if (state == CDL_CTL_ST_DLD_MAX_RETRY) {
        s_ctl.status.imageStatus = e_ImageStatusMaxDownloadRetry;
        s_ctl.status.codeDownloadStatus = e_CodeDownloadStatusFailed;
    } else if (state == CDL_CTL_ST_RESTORE) {
        /*
         * Download was successful, the image failed after max reboots. 
         */
        s_ctl.status.imageStatus = e_ImageStatusMaxRebootRetry;
        s_ctl.status.codeDownloadStatus = e_CodeDownloadStatusFailed;
    } else { 
        return 0;
    }
    ret = cdl_ctl_set_status_image_status(s_ctl.status.imageStatus);
    if (ret < 0) return ret;
    ret = cdl_ctl_set_status_code_download_status(s_ctl.status.codeDownloadStatus);
    if (ret < 0) return ret;
	return 1;
}
/* return > 0 if allow unsigned image, 
 *        <= 0 if not allow. 
 */
int cdl_ctl_allow_unsigned_image() {
    return s_ctl.allow_unsigned_image;
}

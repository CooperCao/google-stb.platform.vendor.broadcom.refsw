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
 * Module Description:  OpenCable Common Download control interface. Control
 *                      and status info
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "cdl_common.h"
#include "cdl_ctl.h"
BDBG_MODULE(cdl_ctl);

/*
 * struct cdl_ctl is a packed data structure that will be saved in flash (flash0.bootinfo)
 * and shared between CFE and kernel, similar to nonvol. 
 */
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
	char fw_name[64];

	struct cdl_ctl_status status;

	int secure_header_len; 
};

static struct cdl_ctl s_ctl;

static int print_params() {
	BDBG_MSG(("/**** current params *****\n"));
	BDBG_MSG(("vendor_id = %d\n", s_ctl.vendor_id));
	BDBG_MSG(("hw_version_id = %d\n", s_ctl.hw_version_id));
	BDBG_MSG(("firmware_file_name = %s\n", s_ctl.fw_name));
	return 0;
}

int cdl_ctl_init() {
	int ret = -1;
	char *str;
	BDBG_MSG(("Enter %s\n", __FUNCTION__));
	memset(&s_ctl, 0, sizeof(s_ctl));
	memset(&s_ctl, 0, sizeof(s_ctl));

	/*
	 * These parameters are generally imported from somewhere else
	 */
	if ((str = getenv(CDL_ENV_VENDOR_ID))== NULL) {
		str = CDL_ENV_VENDOR_ID_DFT;
	}
	s_ctl.vendor_id = strtoul(str, NULL, 0);

	if ((str = getenv(CDL_ENV_HW_VERSION_ID))== NULL) {
#ifdef NO_SETTOPAPI
		s_ctl.hw_version_id = 0xdeadbeef;
#else
		s_ctl.hw_version_id = BCHP_CHIP;
#endif
	} else {
		s_ctl.hw_version_id = strtoul(str, NULL, 0);
	}
	if ((str = getenv(CDL_ENV_FW_NAME))== NULL) {
		str = CDL_ENV_FW_NAME_DFT;
	}
	snprintf(s_ctl.fw_name, 64, "%s", str);

	print_params();
	pthread_mutex_init(&s_ctl.m_ecm, NULL);
	pthread_cond_init(&s_ctl.c_ecm, NULL);
	return 0;
 err:
	return -1;
}

int cdl_ctl_uninit(){
	BDBG_MSG(("Enter %s\n", __FUNCTION__));
	memset(&s_ctl, 0, sizeof(s_ctl));
	memset(&s_ctl, 0, sizeof(s_ctl));
	return 0;
 err: 
	return -1;
}

int cdl_ctl_get_state() {
	return s_ctl.state;
}
uint8_t cdl_ctl_get_method() {
	return s_ctl.method;
}
uint8_t cdl_ctl_get_cmd() {
	return s_ctl.cmd;
}
uint8_t cdl_ctl_get_trigger() {
	return s_ctl.trigger;
}

uint32_t cdl_ctl_get_vendor_id() {
	BDBG_MSG(("Vendor ID 0x%x", s_ctl.vendor_id & 0x00ffffff));
	return (s_ctl.vendor_id & 0x00ffffff);
}

uint32_t cdl_ctl_get_hw_version_id() {
	BDBG_MSG(("HW version ID 0x%x", s_ctl.hw_version_id));
	return s_ctl.hw_version_id;
}

/* return 1 if succeed, 0 if fail */
int cdl_ctl_set_state(int state) {
	if (state >= CDL_CTL_ST_MAX || state <= -CDL_CTL_ST_MAX) {
		BDBG_ERR(("Invalid state %d\n", state));
		return 0;
	}
	s_ctl.state = state;
	return 1;
}

void cdl_ctl_set_failed_state() {
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

int cdl_ctl_is_busy() {
	/* fixme! how about deferred download */
	BDBG_MSG(("ctl state %d\n", s_ctl.state));
	return ( s_ctl.state >= CDL_CTL_ST_DOWNLOAD && s_ctl.state < CDL_CTL_ST_SUCCEED );
}

#if 0
/* 
 * Any function that needs to use eCM must call this function to get exclusive access to eCM.
 * return >0 if succeed, <= 0 if fail. 
 */
int cdl_ctl_get_ecm_mutex() {
	int ret;

	if ( (ret = pthread_mutex_trylock(&s_ctl.m_ecm)) ) {
		if (ret == EBUSY) {
			BDBG_ERR(("ecm busy"));
		} else {
			BDBG_ERR(("%s error %d", __FUNCTION__, ret));
		}
		return -1;
	}	
	return 1;
}
#endif

int cdl_ctl_wait_for_ecm_notification() {
	int ret = 0;

	pthread_mutex_lock(&s_ctl.m_ecm); 
	ret = pthread_cond_wait(&s_ctl.c_ecm, &s_ctl.m_ecm);
	pthread_mutex_unlock(&s_ctl.m_ecm);
	return ret;
}


int cdl_ctl_wakeup_by_ecm_notification() {
	int ret = 0;
	pthread_mutex_lock(&s_ctl.m_ecm);
	ret = pthread_cond_broadcast(&s_ctl.c_ecm);
	pthread_mutex_unlock(&s_ctl.m_ecm);	
	return ret;
}

int cdl_ctl_need_authenticate() {
	int method = s_ctl.method;
	/* DSM_CC download needs authenticate */
	return ( method == CDL_DLD_INBAND_SRC_ID
	    || method == CDL_DLD_INBAND_PID 
	    || method == CDL_DLD_INBAND_PROG 
	    || method == CDL_DLD_DSG_BASIC 
	    || method == CDL_DLD_DSG_ADVANCED ) ? 1 : 0;
}

int cdl_ctl_need_upgrade(char * filename, unsigned int length) {
#if 1
	BDBG_ERR(("Debug only, ignure filename check, force upgrade"));
	return 1;
#else
	return (!strncmp(filename, s_ctl.fw_name, length)) ? 1 : 0;
#endif
}

int cdl_ctl_get_secure_header_len() {
	return s_ctl.secure_header_len;
}

void cdl_ctl_set_secure_header_len(int len) {
	s_ctl.secure_header_len = len;
}

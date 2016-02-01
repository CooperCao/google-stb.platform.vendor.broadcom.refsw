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
#ifndef CDL_CTL_H
#define CDL_CTL_H


struct cdl_ctl_status {
	uint8_t status;
#define CDL_CTL_MAX_REBOOT_RETRY 3
	int reboot_retry;
#define CDL_CTL_MAX_DLD_RETRY    3
	int dld_retry;
#define CDL_CTL_MAX_TRTP_RETRY   16  /* eCM already handles it? */
};
/*
 * download command
 */
#define CDL_CTL_CMD_DLD_NOW      0  
#define CDL_CTL_CMD_DLD_DEFER    1

/* 
 * Internal download state 
 */
/* these values should match with cfe, ui_expand.c */

#define CDL_CTL_ST_INIT             0      /* just initialized */
#define CDL_CTL_ST_WAIT_TRIGGER     1      /* trigger method specified ,waiting for trigger */
#define CDL_CTL_ST_DEFERRED_WAIT    2      /* differed wait */
#define CDL_CTL_ST_VERIFY_CVC       3      /* waiting for cvc verification */
#define CDL_CTL_ST_PARSE_CVT        4      /* parse CVT or eCM CFG file */
#define CDL_CTL_ST_DOWNLOAD         5      /* downloading in progress */
#define CDL_CTL_ST_DOWNLOAD_RETRY   6      /* download retry in progress */
#define CDL_CTL_ST_AUTH_IMAGE       7      /* authenticating image */
#define CDL_CTL_ST_REBOOT_0         8      /* download ok, reboot to expand */
#define CDL_CTL_ST_REBOOT_1         9      /* 1st retry */
#define CDL_CTL_ST_REBOOT_2         10      /* 2nd retry */
#define CDL_CTL_ST_RESTORE          11     /* expand fail, restore current image */
#define CDL_CTL_ST_EXPAND_DONE      12     /* expand suceed */
#define CDL_CTL_ST_SUCCEED          13     /* firmware upgrade succeeded */
#define CDL_CTL_ST_MAX              14     /* maximum state */
         
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
 * trigger flag
 */
#define CDL_CTL_TRIGGER_NONE          0x00
#define CDL_CTL_TRIGGER_ECM_CFG_FILE  0x01
#define CDL_CTL_TRIGGER_SNMP          0x02
#define CDL_CTL_TRIGGER_CA_CVT        0x04
#define CDL_CTL_TRIGGER_DSG_CVT       0x08


typedef struct cdl_ctl * cdl_ctl_t;

int cdl_ctl_init();
int cdl_ctl_uninit();
int cdl_ctl_get_state();
uint8_t cdl_ctl_get_method();
uint8_t cdl_ctl_get_cmd();
uint8_t cdl_ctl_get_trigger();
uint32_t cdl_ctl_get_vendor_id();
uint32_t cdl_ctl_get_hw_version_id();

int cdl_ctl_set_state(int state);
void cdl_ctl_set_failed_state();
int cdl_ctl_set_method(uint8_t method);
int cdl_ctl_set_cmd(uint8_t cmd);
int cdl_ctl_set_trigger(uint8_t trigger);

int cdl_ctl_is_busy();
int cdl_ctl_wait_for_ecm_notification();
int cdl_ctl_wakeup_by_ecm_notification(); 
int cdl_ctl_need_authenticate(); 
int cdl_ctl_need_upgrade(char * filename, unsigned int length);
int cdl_ctl_get_secure_header_len();
void cdl_ctl_set_secure_header_len(int len);
#endif  /* CDL_CTL_H */

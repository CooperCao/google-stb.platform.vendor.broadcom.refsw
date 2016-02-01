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
 * Module Description:  OpenCable Common Download estb functions.
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "cdl_common.h"
#include "b_cdl_lib.h"
#include "cdl_ctl.h"
#include "cdl_estb.h"
BDBG_MODULE(cdl_estb);
#ifndef DSG_SUPPORT
typedef unsigned char   UCHAR;
typedef unsigned short  USHORT;
typedef unsigned int    UINT;
typedef unsigned long   ULONG;
#endif

#include "EstbCommonDownloadInterface.h"
#include "rProgramStoreCustomerApi.h"

/* eSTB APIs have changed. Passing the ipV4 as address type in AuthenticateMonolithicImageSignedContent and AuthenticateAndUpgradeMonolithicImage APIs*/
static int ipAddrType = 4;

/* functions from dsgcc */
extern int InitializeRemoteProgramStoreHost(void);
static int (*cdl_estb_check_integrity) (char * mono_file) = NULL;
static char * cdl_estb_mono_image_fn = NULL;

static int ref_cnt = 0;
static uint8_t cdl_ecm_notification_status = 0;

static void default_cdl_ecm_notification_callback(CommonDownloadNotificationMsg msg);
static int default_EstbSwUpgrade_InProgress_cb_func(void);
static int default_EstbSwUpgrade_IsUpgradePermittedNow_cb_func (void);
static unsigned int default_EstbSwUpgrade_IsMonolithicImageValid_cb_func (void);

static void cdl_ecm_notification_callback(CommonDownloadNotificationMsg msg);
static int EstbSwUpgrade_InProgress_cb_func(void);
static int EstbSwUpgrade_IsUpgradePermittedNow_cb_func (void);
static unsigned int EstbSwUpgrade_IsMonolithicImageValid_cb_func(void);

static void set_default_cb(void) {
	CommonDownloadSetEcmNotificationCallback(default_cdl_ecm_notification_callback);
	EstbSwUpgrade_InProgressSetCallback(default_EstbSwUpgrade_InProgress_cb_func);
	EstbSwUpgrade_IsUpgradePermittedNowSetCallback(default_EstbSwUpgrade_IsUpgradePermittedNow_cb_func);
	/* needs to be implemented in dsgcc */
	EstbSwUpgrade_IsMonolithicImageValidSetCallback(default_EstbSwUpgrade_IsMonolithicImageValid_cb_func);
	
}
int cdl_estb_init() {
	int ret = -1;
	BDBG_MSG(("Enter %s\n", __FUNCTION__));
	ref_cnt = 0;
	/* SetupRemoteInterface(); part of dsgcc now */
	set_default_cb();
	ret = 0;
	return ret;
}

int cdl_estb_uninit(){
	BDBG_MSG(("Enter %s\n", __FUNCTION__));
	set_default_cb();
	return 0;
}

int cdl_estb_open(void) {

	BDBG_MSG(("Enter %s\n", __FUNCTION__));	
	if (ref_cnt > 0) {
		BDBG_ERR(("cdl_estb already open\n"));
		return 0;
	}
	/*
	 * start estb-ecm download thread, 
	 * fixme! customize rProgramStoreHostFunc to report download status.
	 */
	/* cdl_create_thread(&info, (void*)InitializeRemoteProgramStoreHost, NULL); */
	CommonDownloadSetEcmNotificationCallback(cdl_ecm_notification_callback);
	EstbSwUpgrade_InProgressSetCallback(EstbSwUpgrade_InProgress_cb_func);
	EstbSwUpgrade_IsUpgradePermittedNowSetCallback(EstbSwUpgrade_IsUpgradePermittedNow_cb_func);
	/* needs to be implemented in dsgcc */
	EstbSwUpgrade_IsMonolithicImageValidSetCallback(EstbSwUpgrade_IsMonolithicImageValid_cb_func);

	ref_cnt++;
	return 0;
}
int cdl_estb_close() {
	BDBG_MSG(("Enter %s\n", __FUNCTION__));
	if (ref_cnt <= 0) {
		BDBG_ERR(("Cannot open cdl_estb, not open yet\n"));
		return 0;
	}
	ref_cnt--;
	set_default_cb();

	/* fixme! kill the rProgramStoreHostFunc thread */
	return 0;
}

/* return length of CVC if succeed, 0 if fail */
int cdl_estb_check_cvc(uint8_t *cvc, uint32_t length, uint8_t type) {
	int ret;
	CommonDownloadResponseMsg resp_msg;
	BDBG_MSG(("Enter %s\n", __FUNCTION__));
	BDBG_MSG(("cvc %x len %d type %d\n", (uint32_t) cvc, length, type));

	/* fixme: only merged api, functionality not verified */
	ret = CommonDownloadProcessCvc(cvc, length, type, &resp_msg);	
	BDBG_MSG(("resp_msg.returnCode %d, returnValue %d", resp_msg.returnCode, resp_msg.returnValue));
	ret = (ret) ? resp_msg.returnValue : 0;
	return ret;
}

/* 
 * For CVT triggerred data carousel download, the image downloaded by data carousel
 * needs to be authenticated by eCM
 * return > 0 if succeed; <= 0 if fail 
 */
int cdl_estb_authenticate_code_image( uint8_t * fileName, uint8_t fileNameLen, 
					 uint32_t tftpServerIpAddress) {
	int ret = -1;
	CommonDownloadResponseMsg resp_msg;

	BDBG_MSG(("Enter %s\n", __FUNCTION__));

	BDBG_ERR(("file name %s, name len %d, server 0x%x\n", fileName, fileNameLen, tftpServerIpAddress));
	AuthenticateMonolithicImageSignedContent( fileName, fileNameLen, ipAddrType, 
						  tftpServerIpAddress, &resp_msg);
	cdl_ctl_wait_for_ecm_notification();
	if (kMonolithicImageAuthenticationSuccessful == cdl_ecm_notification_status) {
		BDBG_ERR(("authenticate image succeed\n"));
		ret = 1;
	} else {
		BDBG_ERR(("authenticate image failed\n"));
		ret = -1;
	}
	BDBG_MSG(("ret %d\n", ret));
	return ret;
}

/* 
 * For CVT triggerred tftp download
 * return > 0 if succeed; <= 0 if fail 
 */
int cdl_estb_download_tftp_by_cvt( char * fileName, uint32_t fileNameLen, 
					 uint32_t tftpServerIpAddress) {
	int ret;
	CommonDownloadResponseMsg resp_msg;

	BDBG_MSG(("Enter %s\n", __FUNCTION__));
	BDBG_ERR(("file name %s, name len %d, server 0x%x\n", fileName, fileNameLen, tftpServerIpAddress));
	AuthenticateAndUpgradeMonolithicImage( fileName, fileNameLen,ipAddrType, 
					       tftpServerIpAddress, &resp_msg);
	cdl_ctl_wait_for_ecm_notification();
	if (kMonolithicImageDownloadSuccessful == cdl_ecm_notification_status) {
		BDBG_MSG(("authenticate image succeed\n"));
		ret = 1;
	} else {
		BDBG_ERR(("authenticate image failed\n"));
		ret = -1;
	}
    /* 
     * for tftp download, secure header has been stripped;
     */
	return ret;
}

/* return the length of the secure header, == 0 if fail? */
int cdl_estb_validate_secure_header_by_buf( uint8_t * buf, int len) {
	CommonDownloadResponseMsg resp_msg;
	int n = 0;

	if (len <= 0) return 0;
	len = (len < MAX_SECURE_HEADER_SIZE) ? len : MAX_SECURE_HEADER_SIZE;
	AuthenticateMonolithicImageDigitalSignature(buf, len, &resp_msg);
	n = resp_msg.returnValue; 
	if (n == 0) {
		BDBG_ERR(("Secure header %d NOT valid", n));
	} else {
		BDBG_MSG(("Secure header %d valid", n));
	}
	cdl_ctl_set_secure_header_len(n);
	return n;
}

/* return the length of the secure header, == 0 if fail? */
int cdl_estb_validate_secure_header_by_file( char * fileName) {
	int fd;
	uint8_t * buf, *ptr;
	int left = 0, n = 0;

	if ((fd = open(fileName, O_RDONLY)) < 0) {
		BDBG_ERR(("Cannot open %s\n"));
		return 0;
	}
	buf = malloc(MAX_SECURE_HEADER_SIZE);
	if (buf == NULL) {
		BDBG_ERR(("malloc failed\n"));
		return 0;
	}
	BDBG_MSG(("%s opened\n", fileName));
	left = MAX_SECURE_HEADER_SIZE;
	ptr = buf;
	while (left > 0) {
		if ((n = read(fd, ptr, left)) < 0 ) {
			BDBG_ERR(("read error %d, left %d \n", n, left));
			n = 0;
			goto out;
		}
		if (n == 0) {
			BDBG_ERR(("EOF\n"));
			break;
		}
		ptr +=n;
		left -=n;
	}
	
	n = cdl_estb_validate_secure_header_by_buf(buf, (int)(ptr - buf));
 out:
	free(buf);
	close(fd);
	return n;
}

static void cdl_ecm_notification_callback(CommonDownloadNotificationMsg msg){
	cdl_ecm_notification_status = msg.eventCode;
	if (!((kMonolithicImageDownloadSuccessful == cdl_ecm_notification_status) 
	      || (kMonolithicImageAuthenticationSuccessful == cdl_ecm_notification_status))
	    ) {
		BDBG_ERR(("failed %u\n", cdl_ecm_notification_status));
		cdl_ctl_set_failed_state();
	}

	cdl_ctl_wakeup_by_ecm_notification();

}
static void default_cdl_ecm_notification_callback(CommonDownloadNotificationMsg msg){
	BDBG_ERR(("unexpexted notification from eCM, event code %d", msg.eventCode));
}

int load_cvc(char * fn) {
	/* 
	 * fixme! CVC should really come from ecm cfg file, snmp or cvt already.
	 * This is a hack!
	 */
	char buf[1024];
	int fd;
	int len;
	if ((fd = open(fn, O_RDONLY)) < 0) {
		BDBG_ERR(("Cannot open CVC file. Image authenticate will fail\n"));
		return -1;
	} else {
		len = read(fd, buf, 1024);
		if (cdl_estb_check_cvc(buf, len, kManufCvc) == 0) {
			BDBG_ERR(("CVC failed\n"));
			return -1;
		}
		close(fd);
	}
	BDBG_MSG(("CVC ok\n"));
	return 1;
}

static int default_EstbSwUpgrade_InProgress_cb_func(void)
{
	BDBG_ERR(("unexpected callback %s", __FUNCTION__));

	return 0;
}
static int EstbSwUpgrade_InProgress_cb_func(void)
{
	/*
	if (load_cvc() < 0) {
		cdl_ctl_set_failed_state();
		return -1;
	}
	*/
#if 0
	if (trigger == kTftpDownloadTriggerEcmCfgFile) 
		cdl_ctl_set_method(CDL_DLD_TFTP_BY_CFG_FILE);
	else if (trigger == kTftpDownloadTriggerSnmp) 
		cdl_ctl_set_method(CDL_DLD_TFTP_BY_SNMP);
	else if (trigger == kTftpDownloadTriggerEcmCfgFile) 
		cdl_ctl_set_method(CDL_DLD_TFTP_BY_CVT);
	else {
		printf("Invalid trigger %d\n", trigger);
		cdl_ctl_set_failed_state();
		return -1;
	}
#else
	/* 
	 * for now, we don't see any need to know what kind of tftp trigger it is,
	 * so we just set it as CDL_DLD_TFTP;
	 * If we need to know the tftp grigger source, it should come from eCM
	 */
	cdl_ctl_set_method(CDL_DLD_TFTP);
#endif
	cdl_ctl_set_state(CDL_CTL_ST_DOWNLOAD);
	return 1;
}

/* return 1 if permitted, 0 if not */
static int default_EstbSwUpgrade_IsUpgradePermittedNow_cb_func (void) {
	BDBG_ERR(("Sw upgrade not permitted"));
	return 0;
}
static int EstbSwUpgrade_IsUpgradePermittedNow_cb_func (void) {
    /*
     * This function is meaningful only for eCM triggered download, as eCM triggered
     * CDL will call this function for permission, before CDL.
     * For eSTB triggered CDL, this function may be called in the middle of CDL, 
     * as a result of AuthenticateMonolithicImageSignedContent(). It should always
     * be permitted in this case.
     */
	if (cdl_ctl_is_busy()) {
		if (cdl_ctl_get_method() == CDL_DLD_TFTP_BY_CFG_FILE ||
            cdl_ctl_get_method() == CDL_DLD_TFTP_BY_SNMP) {
			return 0;
		} 
	}
	return 1;
}
/* return 1 if permitted, 0 if not */
static unsigned int default_EstbSwUpgrade_IsMonolithicImageValid_cb_func (void) {
	BDBG_ERR(("Monolithic image not valid"));
	return 0;
}
static unsigned int EstbSwUpgrade_IsMonolithicImageValid_cb_func (void) {
	/*FIXME */
	if (cdl_estb_check_integrity) {
		return (cdl_estb_check_integrity)(cdl_estb_mono_image_fn);
	}
	BDBG_ERR(("Integrity check not implemented, force succeed"));
	return 1;
}

/* return the length of the secure header, return  -1 if fail, return 0 if no secure header */
int cdl_estb_authenticate_cvt(void *cdl_cvt_recv, unsigned int len) {
	CommonDownloadResponseMsg resp_msg;
	AuthenticateCvtFile( cdl_cvt_recv, len, &resp_msg );
	BDBG_MSG(("resp_msg.returnCode %d, returnValue %d", resp_msg.returnCode, resp_msg.returnValue));
	return resp_msg.returnValue;
}
void cdl_estb_set_monolithic_image_temp_file_name (char * fn) {
	cdl_estb_mono_image_fn = fn;
	SetMonolithicImageTempFileName(fn);
}

char * cdl_estb_get_monolithic_image_temp_file_name (void) {
#define MAX_RETRY 10
	int i =0;
	char * str = NULL; 

	while ( i < MAX_RETRY) {
		if ((str = GetMonolithicImageTempFileName())) {
			BDBG_ERR(("!!!!! MonolithicImageTempFileName %s\n", str));
			break;
		}
		BDBG_ERR(("MonolithicImageTempFileName not found, wait for %d seonds", MAX_RETRY - i));
		sleep(1);
		i++;
	}
	if (i >= MAX_RETRY) {
		BDBG_ERR(("Cannot find MonolithicImageTempFileName"));
	}
	return str;
}

void cdl_estb_set_cb_check_integrity(int (*check_integrity) (char * mono_file)) {
	cdl_estb_check_integrity = check_integrity;
}

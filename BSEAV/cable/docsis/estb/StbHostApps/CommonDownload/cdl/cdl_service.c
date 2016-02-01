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
#ifndef NO_SETTOPAPI 
#include "cdl_dsmcc.h"
#include "bsettop_types.h"
#endif
#ifdef DSG_SUPPORT
#include "cdl_ctl.h"
#include "cdl_estb.h"
#include "rProgramStoreCustomerApi.h"
#endif
#if defined(MPOD_SUPPORT) 
#include "cdl_adpu.h"
#endif
#include "cdl_service.h"
/* 
 * The following interfaces are the only files that need customization.
 * Depending on the layout of the customer's monolithic image(cdl_image_xxx), 
 * the storage device (cdl_storage_xxx) for the monolithic image (flash, HD, etc),
 * and how the download status is saved (cdl_bootinfo_xxx) in nvram.
 * 
 */
#include "cdl_image_brcm.h" 
#ifndef HOST_ONLY
#include "cdl_storage_flash.h" 
#include "cdl_bootinfo_nvram.h" 
#endif

BDBG_MODULE(cdl_service);

#ifndef NO_SETTOPAPI
static btuner_t tuner[MAX_TUNER];
#endif


static cdl_service_params s_params;

cdl_thread_info wait_for_ca_cvt_thread_info;
cdl_thread_info post_download_processing_thread_info;


/* return < 0 if fail */
void cdl_service_print_image_header(int sec_header_len) {
	if (s_params.image_if.print_header) {
		s_params.image_if.print_header(s_params.storage_file_downloaded, 
						 sec_header_len);
	}
}

/* return < 0 if fail */
int cdl_service_generate_image(char * param_file ) {
	if (s_params.image_if.generate) {
		return s_params.image_if.generate(s_params.storage_file_downloaded, param_file);
	}
	return -1;
}

/* return < 0 if fail */
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
	
 out:
	s_params.storage_if.close(h_storage);

	return ret;
}
int cdl_service_upgrade_image(int sec_header_len) {
	/* exapnd image */
	if (cdl_service_expand_image(sec_header_len) < 0) return -1;

	/* update boot parameter  */
	if (s_params.bootinfo_if.ready_to_reboot() < 0 ) return -1;
	/* cdl_service_uninit(); */
	BDBG_ERR(("\n*********** Rebooting system to complete ffirmware upgrade ***********\n"));
	system("reboot"); 

	return 0;
}

#ifdef DSG_SUPPORT
/* return >= 0 if succeed, <0 if fail */
int cdl_service_authenticate_image(int force) {
	int ret = -1;
	uint32_t server_addr;

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
		if ((h_storage = s_params.storage_if.open(s_params.debug.fn_sec_header, 
							  O_RDONLY)) == NULL) {
			free(buf);
			return ret;
		}
		if ( (n = s_params.storage_if.read(h_storage, buf, MAX_SECURE_HEADER_SIZE)) <= 0 ) {
			free(buf);
			s_params.storage_if.close(h_storage);
			return ret;
		}
		if (cdl_estb_validate_secure_header_by_buf(buf, n) == 0) {
#if defined(MPOD_SUPPORT) 
			cdl_adpu_send_host_download_control(HC_CERTIFICATE_FAILURE);
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
			cdl_adpu_send_host_download_control(HC_CERTIFICATE_FAILURE);
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
			cdl_ctl_set_failed_state();
#if defined(MPOD_SUPPORT) 
			cdl_adpu_send_host_download_control(HC_CERTIFICATE_FAILURE);
#endif
		}
	} else {
		BDBG_MSG(("TFTP donwload already authenticated by eCM, do nothing here\n"));
	}

	/*
	 * customer specific image validation
	 */
	if (s_params.image_if.check_integrity(s_params.storage_file_downloaded) < 0) {
#if defined(MPOD_SUPPORT) 
		cdl_adpu_send_host_download_control(HC_DOWNLOAD_IMAGE_DAMAGED);
#endif
		BDBG_ERR(("Image damaged\n"));
		ret = -1;
	}
	return ret;
}

int trigger_by_ecm() {
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
static int download_inband(cdl_adpu_cvt_t h_cvt) {
	btuner_qam_params qam;
	bband_t band;
	bmessage_stream_params params;
	int i = 0, ret = 0, n = 0;
	char str[64];
	int module_index = 1;
	cdl_dsmcc_t h_dsmcc = NULL;
	void * h_storage = NULL;
	uint8_t * buf = NULL;
	int fd = -1;
	int len = 0;
	int sec_header_valid = 0;
	int sec_header_len = 0;

	cdl_ctl_set_method(h_cvt->method);
	cdl_ctl_set_state(CDL_CTL_ST_DOWNLOAD);
	cdl_estb_open();
	cdl_adpu_send_host_download_control(HC_DOWNLOAD_STARTED);

	if (tuner[0])
		btuner_qam_params_init(&qam, tuner[0]);
	else {
		BDBG_ERR(("tuner 0 not available"));
		return -1;
	}
	if (h_cvt->t.type1.mod_type == CVT_MOD_QAM256) {
		qam.symbol_rate = 5360400;/* 5056900; */
		qam.mode = 256; /* 64; */
	} else if (h_cvt->t.type1.mod_type == CVT_MOD_QAM64) {
		qam.symbol_rate = 5056900; 
		qam.mode = 64; 
	} else {
		BDBG_ERR(("Invalid mod type %d\n", h_cvt->t.type1.mod_type));
		ret = -1;
		goto out;
	}

	if (h_cvt->method == CDL_DLD_INBAND_PID) {
		if (h_cvt->t.type1.mod_type == CVT_MOD_QAM256) {
			qam.symbol_rate = 5360400;/* 5056900; */
			qam.mode = 256; /* 64; */
		} else if (h_cvt->t.type1.mod_type == CVT_MOD_QAM64) {
			qam.symbol_rate = 5056900; 
			qam.mode = 64; 
		} else {
			BDBG_ERR(("Invalid mod type %d\n", h_cvt->t.type1.mod_type));
			ret = -1;
			goto out;
		}

		BDBG_MSG(("Decoding from a QAM demod...\n"));
		band = btuner_tune_qam(tuner[0], h_cvt->t.type1.freq * 1000000, &qam);
		if (!band) { 
			cdl_adpu_send_host_download_control(HC_DOWNLOAD_NOTIFY_HEADEND);
			BDBG_ERR(("failed to tne to %dMHz", h_cvt->t.type1.freq));
			ret = -1;
			goto out;
		}
		if ((h_dsmcc = cdl_dsmcc_open_pid(0, band, h_cvt->t.type1.pid)) == NULL) { 
			/* pid 0x1102 */
			cdl_adpu_send_host_download_control(HC_DOWNLOAD_NOTIFY_HEADEND);
			BDBG_ERR(("cannot open dsmcc\n"));
			ret = -1;
			goto out;		
		}	
	} else if (h_cvt->method == CDL_DLD_INBAND_PROG) {
		if (h_cvt->t.type2.mod_type == CVT_MOD_QAM256) {
			qam.symbol_rate = 5360400;/* 5056900; */
			qam.mode = 256; /* 64; */
		} else if (h_cvt->t.type2.mod_type == CVT_MOD_QAM64) {
			qam.symbol_rate = 5056900; 
			qam.mode = 64; 
		} else {
			BDBG_ERR(("Invalid mod type %d\n", h_cvt->t.type2.mod_type));
			cdl_adpu_send_host_download_control(HC_DOWNLOAD_NOTIFY_HEADEND);
			ret = -1;
			goto out;
		}

		printf("Decoding from a QAM demod...\n");
		band = btuner_tune_qam(tuner[0], h_cvt->t.type2.freq * 1000000, &qam);
		if (!band) { 
			ret = -1;
			goto out;
		}
		if ((h_dsmcc = cdl_dsmcc_open_prog(0, band, h_cvt->t.type2.program_number)) == NULL) { 
			/* prog 2101 */
			BDBG_ERR(("cannot open dsmcc\n"));
			cdl_adpu_send_host_download_control(HC_DOWNLOAD_NOTIFY_HEADEND);
			ret = -1;
			goto out;		
		}	
	} else {
		BDBG_ERR(("unupported metohd %d", h_cvt->method));
		goto out;
	}
			
	/* write image */
	if ( (buf = (uint8_t *)malloc(BUF_SIZE)) == NULL) {
		BDBG_ERR(("malloc failed\n"));
		ret = -1;
		goto out;
	}

#ifdef OCAP_495MHZ
	/* test for a specific stream */
	memset(str, 0xff, 255);
	len = 0x15;
	if ((module_index = cdl_dsmcc_get_module_index(h_dsmcc, str, len)) < 0 ) {
		ret = -1;
		goto out;
	}
#endif
	module_index = 0;
	if ((fd = open(s_params.storage_file_downloaded, O_WRONLY | O_CREAT | O_TRUNC)) < 0) {
		BDBG_ERR(("cannot open %s\n", s_params.storage_file_downloaded));
		ret = -1;
		goto out;
	}
	len = 0;
	while ((n = cdl_dsmcc_read(h_dsmcc, module_index, buf, BUF_SIZE)) >0) {
		if (sec_header_valid == 0 && len >= MAX_SECURE_HEADER_SIZE) {
			/* check the secure header first */

#if 0
			if ((sec_header_len = cdl_estb_validate_secure_header_by_buf(buf, n)) == 0) {
#else
			/* debug only */
			if ((sec_header_len = cdl_estb_validate_secure_header_by_file(s_params.debug.fn_sec_header)) == 0) {
#endif
#if defined(MPOD_SUPPORT) 
				cdl_adpu_send_host_download_control(HC_CERTIFICATE_FAILURE);
#endif
				ret = -1;
				goto out;
			}
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
	ret = 0;
	BDBG_MSG(("donwload inband succeed\n"));
#if defined(MPOD_SUPPORT) 
	cdl_adpu_send_host_download_control(HC_DOWNLOAD_COMPLETED);
#endif

 out:
	if (fd >= 0)close(fd);
	if (h_dsmcc) cdl_dsmcc_close(h_dsmcc);
	if (buf) free(buf);
	return ret;
}

/* return >= 0 if succeed, < 0 if fail */
static int download_tftp_by_cvt(cdl_adpu_cvt_t h_cvt) {
	int ret  = -1;
	struct in_addr iaddr; 
	char ch[32];
	uint32_t server_addr;
	uint32_t fn_len;

	BDBG_ERR(("TODO: tftp download by cvt"));
	cdl_ctl_set_method(h_cvt->method);
	cdl_ctl_set_state(CDL_CTL_ST_DOWNLOAD);
	cdl_estb_open();
	cdl_adpu_send_host_download_control(HC_DOWNLOAD_STARTED);
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
}

int wait_for_ca_cvt() {
	int ret = -1;
	cdl_adpu_cvt_t h_cvt;

	while (1) {
		cdl_adpu_cvt_wait();
		if (wait_for_ca_cvt_thread_info.exit == 1) break;

		BDBG_ERR(("got new cvt"));
		/* start downloading*/
		if ( (h_cvt = cdl_adpu_cvt_get()) == NULL) {
			BDBG_MSG(("Cannot get cvt\n"));
			continue;
		}
		if (cdl_ctl_is_busy()) {
			/* a download is already in progress */
			BDBG_MSG(("Download in progress\n"));
			continue;
		} 

		if (!cdl_ctl_need_upgrade((char *)h_cvt->code_filename, h_cvt->code_filename_length)) {
			BDBG_MSG(("no fw upgrade is needed"));
			continue;
		}
		switch (h_cvt->method) {
		case CDL_DLD_INBAND_SRC_ID:
		case CDL_DLD_INBAND_PID:
		case CDL_DLD_INBAND_PROG:
			if (download_inband(h_cvt) >= 0) {
				{
					char str[64];

					/*
					 * Since we don't have data carousel headend, we simply copy the new fw 
					 * to simulate the dsm_cc data carouse;
					 */
					BDBG_ERR(("TODO: simulating data carousel downloading"));

					sprintf(str, "cp -f %s  %s", h_cvt->code_filename, s_params.storage_file_downloaded );
					printf("Simulate dsmm_cc download by %s\n", str);
					system(str);

					/* send to eCM to authenticate digital signature */
					
					/* 
					 * send to eCM to authenticate digital content,
					 * the final file will be stored in storage_file_downloaded, same as docsis 
					 * tftp download.
					 */


					
				}

				/* wake up post download processing */

				cdl_ctl_wakeup_by_ecm_notification();
			} else {
				BDBG_ERR(("inband pid download fail\n"));
				goto wait;
			}
			break;
		case CDL_DLD_TFTP_BY_CVT:
			BDBG_ERR(("TFTP by cvt\n"));
			if (download_tftp_by_cvt(h_cvt) >= 0) {
				/* wake up post download processing */
				cdl_ctl_wakeup_by_ecm_notification();

			} else {
				BDBG_ERR(("tftp by cvt download fail\n"));
				goto wait;
			}
			
			break;
		default:
			BDBG_ERR(("Unsupported download method %d\n", h_cvt->method));
			break;
		}
		
		continue;
	wait:
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

/*
 * The processing include image authentication (CVT triggered only), copy monolithic image
 * to flash (all), update nvram (all) and reboot (all). 
 */
#ifdef DSG_SUPPORT
void post_download_processing(void) {
	uint8_t method;

	while (1) {
		/*
		 * Waiting for the monolithic image download to finish, and do post_download processing.
		 */
		cdl_ctl_wait_for_ecm_notification();
		if (post_download_processing_thread_info.exit == 1) break;
		if (s_params.debug.authenticate_only) 
			continue;
		method = cdl_ctl_get_method();
		if (method == CDL_DLD_SIMU) {
			/*use cdltest to generate image, this option is obsolete */
			BDBG_ERR(("method %d is not supported", method));
			continue;
		} else if ( method == CDL_DLD_INBAND_SRC_ID
			    || method == CDL_DLD_INBAND_PID
			    || method == CDL_DLD_INBAND_PROG
			    || method == CDL_DLD_DSG_BASIC
			    || method == CDL_DLD_DSG_ADVANCED) {
			/* authenticate image, needed for carousel download only */
			BDBG_ERR(("Authenticating %s", s_params.storage_file_downloaded));
			if (cdl_service_authenticate_image(0) < 0) {
				BDBG_ERR(("Authenticate image failed\n"));
				continue;
			}
		} else {
			BDBG_ERR(("%s downloaded by tftp, method %d, skip authenticate", 
				  s_params.storage_file_downloaded, method));
		}

		if (cdl_ctl_get_state() < 0) {
			BDBG_ERR(("download failed, waiting for new download"));
			continue;
		}
		/* 
		 * For both tftp and carousel download,
		 * the secure header has already been stripped off during downloading
		 */

		if (cdl_service_upgrade_image(0) < 0) {
			continue;
		}

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
int cdl_image_download() {
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


void closeall() {
	uint8_t trigger;

	cdl_service_stop();
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

 static void cdl_image_init() {
	 memset(&s_params.image_if, 0, sizeof (s_params.image_if));
	 s_params.image_if.print_header = cdl_image_brcm_print_header;
	 s_params.image_if.generate = cdl_image_brcm_generate;
	 s_params.image_if.verify = cdl_image_brcm_verify;
	 s_params.image_if.check_integrity = cdl_image_brcm_check_integrity;
 }

#ifdef HOST_ONLY
 static int cdl_bootinfo_dummy_open (char *pathname) { return 1;}
 static int cdl_bootinfo_dummy_close (void) { return 1;}
 static int cdl_bootinfo_dummy_just_started (void) { return 1;}
 static int cdl_bootinfo_dummy_ready_to_reboot (void) {return 1;}
 static int cdl_bootinfo_dummy_get_new_storage (char * fn_kernel, char * fn_bootloader) {return 1;}
 static char * cdl_bootinfo_dummy_get_dev_name(void) { return NULL;}
#endif
 static void cdl_bootinfo_init() {
	 memset(&s_params.bootinfo_if, 0, sizeof (s_params.bootinfo_if));

#ifdef HOST_ONLY
	 s_params.bootinfo_if.open = cdl_bootinfo_dummy_open;
	 s_params.bootinfo_if.close = cdl_bootinfo_dummy_close;
	 s_params.bootinfo_if.just_started = cdl_bootinfo_dummy_just_started;
	 s_params.bootinfo_if.ready_to_reboot = cdl_bootinfo_dummy_ready_to_reboot;
	 s_params.bootinfo_if.get_new_storage = cdl_bootinfo_dummy_get_new_storage;
	 s_params.bootinfo_if.get_dev_name = cdl_bootinfo_dummy_get_dev_name;

#else	 
	 s_params.bootinfo_if.open = cdl_bootinfo_nvram_open;
	 s_params.bootinfo_if.close = cdl_bootinfo_nvram_close;
	 s_params.bootinfo_if.just_started = cdl_bootinfo_nvram_just_started;
	 s_params.bootinfo_if.ready_to_reboot = cdl_bootinfo_nvram_ready_to_reboot;
	 s_params.bootinfo_if.get_new_storage = cdl_bootinfo_nvram_get_new_storage;
	 s_params.bootinfo_if.get_dev_name = cdl_bootinfo_nvram_get_dev_name;
#endif
 }
static void cdl_bootinfo_uninit() {
	if (s_params.bootinfo_if.close)
		s_params.bootinfo_if.close();
	 return;
 }

static void cdl_storage_init() {
	 memset(&s_params.storage_if, 0, sizeof (s_params.storage_if));
#ifndef HOST_ONLY
	 s_params.storage_if.open = cdl_storage_flash_open;
	 s_params.storage_if.close = cdl_storage_flash_close;
	 s_params.storage_if.read = cdl_storage_flash_read;
	 s_params.storage_if.write = cdl_storage_flash_write;
	 s_params.storage_if.lseek = cdl_storage_flash_lseek;
	 s_params.storage_if.expand = cdl_storage_flash_expand;
#endif
 }
 static void cdl_storage_uninit() {
 }
 
void cdl_service_init() {

	memset(&s_params, 0, sizeof(s_params));
	printf("AAAAAAAAAA\n");
#ifdef DSG_SUPPORT
	cdl_ctl_init();
	printf("BBBBBBBBBBBB\n");
	cdl_estb_init();
	printf("CCCCCCCCCCCCC\n");
#endif
#if defined(MPOD_SUPPORT) 
	cdl_adpu_init();
#endif
#ifndef NO_SETTOPAPI 
	memset(tuner,0, sizeof(tuner));
	cdl_dsmcc_init();
#endif
	/* 
	 * the following modules should be the only modules that can be customized 
	 * cdl_image_xxx: monolithic image generation, validation, etc.
	 * cdl_bootinfo_xxx: mainly to save CDL download and boot information, like download status.
	 * cdl_storage_xxx: monolithic image storage functions. 
	 */
	printf("DDDDDDDDD\n");

	cdl_image_init();
	printf("EEEEEEEEEE\n");

	cdl_bootinfo_init();
	printf("FFFFFFFFFFFFFFF\n");
	cdl_storage_init();
	printf("GGGGGGGGGGG\n");
	/*
	 * extract the parameters.
	 */
	snprintf(&s_params.bootinfo_dev_kernel[0], MAX_DEV_STR_LEN, "%s", 
		 s_params.bootinfo_if.get_dev_name());
	printf("HHHHHHHH\n");
	s_params.bootinfo_if.open(s_params.bootinfo_dev_kernel);
	printf("IIIIIIIIIIIIIIII\n");
	/* start checking CDL status */
	s_params.bootinfo_if.just_started(); 
	printf("JJJJJJJJJJ\n");
	s_params.bootinfo_if.get_new_storage(s_params.storage_kernel, 
					     s_params.storage_bootloader);

	printf("KKKKKKKKKK\n");
	BDBG_ERR(("storage device %s, %s", s_params.storage_kernel, 
		  s_params.storage_bootloader));

	printf("LLLLLLLLLL\n");
#ifdef DSG_SUPPORT
	cdl_estb_set_monolithic_image_temp_file_name(s_params.storage_kernel);
#endif
}

 void cdl_service_get_params(cdl_service_params * params) {
	 memcpy(params, &s_params, sizeof(s_params));
 }
 void cdl_service_set_params(cdl_service_params * params) {
	 memcpy(&s_params, params, sizeof(s_params));
 }


int cdl_service_start(void) {


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

int cdl_service_stop(void) {
	return 1;
}
static void cdl_image_uninit() {
	 return;
 }

void cdl_service_uninit() {

	closeall();

	cdl_storage_uninit();
	cdl_bootinfo_uninit();
	cdl_image_uninit();

#ifndef NO_SETTOPAPI  
	cdl_dsmcc_uninit();
#endif
#if defined(DSG_SUPPORT)
#if defined(MPOD_SUPPORT) 
	cdl_adpu_uninit();
#endif
	cdl_estb_uninit();
	cdl_ctl_uninit();
#endif
}

int cdl_service_control(int command, void *data, int id)
{
	
	switch (command) {
	case CDL_SERVICE_ATTACH_TUNER:
#ifndef NO_SETTOPAPI 
		tuner[id] = (btuner_t)data;
#endif
		return 0;
	default:
		BDBG_ERR(("Unknown cdl service %d", command));
		break;
	}
	return 0;
}

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
 * Module Description:  OpenCable Common Download cablecard interface. Functions
 *                      in this module provides ADPU interface
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
#include "b_cdl_lib.h"
#include "cdl_adpu.h"
#include "cdl_estb.h" /* cvc */
#include "cdl_ctl.h"
#include "cdl_msg.h"
#include <netinet/in.h>
#include <arpa/inet.h> /* for inet_ntoa */
#include <sys/time.h>

BDBG_MODULE(cdl_adpu);


/* DL_CVT2 is not really raw CVT, it has been parsed to fit in the data structure*/
static DL_CVT2 cur_raw_cvt; /* copy of current valid cvt, used to compaire with new cvt */

/* cdl_adpu_cvt is the per-object data structure that is passed to app. */
static struct cdl_adpu_cvt cur_cvt[CDL_ADPU_NUM_OBJ];
static int cur_cvt_num_obj = 0; /* number of objects in the current cvt */
static int cur_cvt_cur_obj = 0; /* current object id the current cvt */

static B_EventHandle cvt_event;
static DL_CVT2		new_cvt; /* temporarily holding the new cvt*/

static B_MPOD_DL_CVT_HOST_RESPONSE cdl_adpu_cvt_process_internal(DL_CVT2 * cvt) ;

static int same_cvt_in_use = 1; /* We allow resend same cvt to make sure the 
			    * CVT reaches app.
			    * It's not allowed if a CVT is being processed,
			    * or a CDL download is in progress.
			    */
static void cdl_adpu_clear_new_cvt(void) {
	memset(&new_cvt, 0, sizeof(cur_cvt));
}
static void cdl_adpu_clear_cur_cvt(void) {
	memset(&cur_cvt, 0, sizeof(cur_cvt));
    cur_cvt_num_obj = 0;
    cur_cvt_cur_obj = 0;
}
static void cdl_adpu_clear_all_cvt(void) {
    cdl_adpu_clear_new_cvt();
    cdl_adpu_clear_cur_cvt();
	memset(&cur_raw_cvt, 0, sizeof(cur_raw_cvt));
}

static void print_raw_cvt( DL_CVT2 * cvt) {
    int i, j;
	BDBG_MSG(("********  CVT received  ****************\n"));
#if 0
	{
		int k;
		unsigned char * ch = (unsigned char *) &cvt->download_type;
		for ( k = 0; k < 32; k++) {
			BDBG_MSG(("%02x ",*ch++));
			if ((k &0xf) == 0xf) BDBG_MSG(("\n"));
		}
	}
#endif

    BDBG_MSG(("CVT Type %02x", cvt->type));
    BDBG_MSG(("Protocol version %02x", cvt->protocol_version));
    BDBG_MSG(("Configuration counter change %02x", cvt->ccc));
    BDBG_MSG(("num descriptors %d", cvt->num_descriptors));
    for (i = 0; i< cvt->num_descriptors; i++) {
        DL_DESCRIPTOR * dl_desc;
        dl_desc = &cvt->descriptor[i];
        BDBG_MSG(("\t[%d] tag 0x%02x", i, dl_desc->tag));
        BDBG_MSG(("\t[%d] len %d", i, dl_desc->len));
        for (j = 0; j < dl_desc->len; j++) {
            BDBG_MSG(("\t\t 0x%02x", dl_desc->data[j]));
        }
    }
    BDBG_MSG(("num obj %d", cvt->num_obj));
    for (i = 0; i< cvt->num_obj; i++) {
        DL_CVT_OBJ * dl_obj;    
        dl_obj = &cvt->obj[i];
        BDBG_MSG(("\t[%d]Download Cmd: %s", i, (dl_obj->download_command == DOWNLOAD_NOW) ? "Now" : "Deffered" ));
        switch (dl_obj->download_type) {
        case INBAND_FAT:
            BDBG_MSG(("\t[%d]Download Type: Inband\n", i));
            BDBG_MSG(("\t[%d]Location Type %d\n", i, dl_obj->download_data.type0.location_type));
            switch (dl_obj->download_data.type0.location_type) {
            case 0x00:
                BDBG_MSG(("\t[%d]source_id %d\n", i, dl_obj->download_data.type0.location_data.source_id));
                break;
            case 0x01:
                BDBG_MSG(("\t[%d]freq %d\n", i, dl_obj->download_data.type0.location_data.type1.freq));
                BDBG_MSG(("\t[%d]mod_type %d\n", i, dl_obj->download_data.type0.location_data.type1.mod_type));
                BDBG_MSG(("\t[%d]pid 0x%x\n", i, dl_obj->download_data.type0.location_data.type1.pid));
                break;
            case 0x02:
                BDBG_MSG(("\t[%d]freq %d\n", i, dl_obj->download_data.type0.location_data.type2.freq));
                BDBG_MSG(("\t[%d]mod_type %d\n", i, dl_obj->download_data.type0.location_data.type2.mod_type));
                BDBG_MSG(("\t[%d]prog %d\n", i, dl_obj->download_data.type0.location_data.type2.program_number));
                break;
            default:
                BDBG_ERR(("\t[%d]Invalid location type %d\n", i, dl_obj->download_data.type0.location_type));
                break;
            }
            break;
        case DSG_DSM_CC:
            BDBG_MSG(("\t[%d]Download Type: DSG DSM_CC\n", i));
            BDBG_MSG(("\t[%d]Location Type %d\n", i, dl_obj->download_data.type1.location_type));
            switch (dl_obj->download_data.type1.location_type) {
            case 0x03:
                BDBG_MSG(("\t[%d]dsg_tunnel_addr %s\n", i, dl_obj->download_data.type1.location_data.type3.dsg_tunnel_addr));
                BDBG_MSG(("\t[%d]src_ip_addr %s\n", i, dl_obj->download_data.type1.location_data.type3.src_ip_addr));
                BDBG_MSG(("\t[%d]dst_ip_addr %s\n", i, dl_obj->download_data.type1.location_data.type3.dst_ip_addr));
                BDBG_MSG(("\t[%d]src_port %d\n", i, dl_obj->download_data.type1.location_data.type3.src_port));
                BDBG_MSG(("\t[%d]dst_port %d\n", i, dl_obj->download_data.type1.location_data.type3.dst_port));
                break;
            case 0x04:
                BDBG_MSG(("\t[%d]application_id %d\n", i, dl_obj->download_data.type1.location_data.application_id));
                break;
            default:
                BDBG_ERR(("\t[%d]Invalid location type %d\n", i, dl_obj->download_data.type0.location_type));
                break;
            }
            break;
        case DOCSIS_TFTP :
            BDBG_MSG(("\t[%d]Download Type: DOCSIS_TFTP\n", i));
            BDBG_MSG(("\t[%d]Tftp_server_addr %s\n", i, dl_obj->download_data.tftp_server_addr));
            break;
        default:
            BDBG_ERR(("\t[%d]invalid download type %d\n", i, dl_obj->download_type));
            break;
        }

        BDBG_MSG(("\t[%d]Obj type %d\n", i, dl_obj->obj_type));
        BDBG_MSG(("\t[%d]Obj data_length %d\n", i, dl_obj->obj_data_length));
        BDBG_MSG(("\t[%d]Obj data\n", i));
        for (j = 0; j < dl_obj->obj_data_length; j++)
            BDBG_MSG(("%2x ", dl_obj->obj_data[j]));

        BDBG_MSG(("\t[%d]Code_filename_length %d\n", i, dl_obj->code_filename_length));
        BDBG_MSG(("\t[%d]Code_filename %s\n", i, dl_obj->code_filename));
        BDBG_MSG(("********** ****************************\n"));
    }
}
/* return >= 0 if succeed, < 0 if fail */
static int parse_cvt( DL_CVT2 * cvt) {
	cdl_adpu_cvt_t h;
    DL_CVT_OBJ * dl_obj;    
    int i;
    int ret = 0;

    cdl_adpu_clear_cur_cvt();
    for (i = 0; i < cvt->num_obj; i++) {
        h = (cdl_adpu_cvt_t)&cur_cvt[i];
        dl_obj = &cvt->obj[i];
        switch (dl_obj->download_type) {
        case INBAND_FAT:
            switch (dl_obj->download_data.type0.location_type) {
            case 0x00:
                h->method = CDL_DLD_INBAND_SRC_ID;
                h->t.type0.source_id = dl_obj->download_data.type0.location_data.source_id;
                break;
            case 0x01:
                h->method = CDL_DLD_INBAND_PID;
                h->t.type1.freq = dl_obj->download_data.type0.location_data.type1.freq;
                h->t.type1.mod_type = dl_obj->download_data.type0.location_data.type1.mod_type;
                h->t.type1.pid = dl_obj->download_data.type0.location_data.type1.pid;
                break;
            case 0x02:
                h->method = CDL_DLD_INBAND_PROG;
                h->t.type2.freq = dl_obj->download_data.type0.location_data.type2.freq;
                h->t.type2.mod_type = dl_obj->download_data.type0.location_data.type2.mod_type;
                h->t.type2.program_number = dl_obj->download_data.type0.location_data.type2.program_number;
                break;
            default:
                BDBG_ERR(("Invalid location type %d\n", dl_obj->download_data.type0.location_type));
                goto error;
            }
            break;
        case DSG_DSM_CC:
            switch (dl_obj->download_data.type1.location_type) {
            case 0x03:
                h->method = CDL_DLD_DSG_BASIC;
                memcpy(h->t.type3.dsg_tunnel_addr, dl_obj->download_data.type1.location_data.type3.dsg_tunnel_addr, 6);
                memcpy(h->t.type3.src_ip_addr, dl_obj->download_data.type1.location_data.type3.src_ip_addr, 16);
                memcpy(h->t.type3.dst_ip_addr, dl_obj->download_data.type1.location_data.type3.dst_ip_addr, 16);
                h->t.type3.src_port = dl_obj->download_data.type1.location_data.type3.src_port;
                h->t.type3.dst_port = dl_obj->download_data.type1.location_data.type3.dst_port;
                break;
            case 0x04:
                h->method = CDL_DLD_DSG_ADVANCED;
                h->t.type4.application_id = dl_obj->download_data.type1.location_data.application_id;
                break;
            default:
                BDBG_ERR(("Invalid location type %d\n", dl_obj->download_data.type0.location_type));
                goto error;
                break;
            }
            break;
        case DOCSIS_TFTP :
            h->method = CDL_DLD_TFTP_BY_CVT;
            memcpy(h->t.tftp.server_addr, dl_obj->download_data.tftp_server_addr, 16);
            break;
        default:
            BDBG_ERR(("invalid download type %d\n", dl_obj->download_type));
            goto error;
        }
        h->code_filename_length = dl_obj->code_filename_length;
        memcpy(h->code_filename, dl_obj->code_filename, h->code_filename_length);
        h->obj_type = dl_obj->obj_type;
        h->obj_data_length = dl_obj->obj_data_length;
        if (h->obj_data_length) memcpy(h->obj_data, dl_obj->obj_data, h->obj_data_length);


        h->cmd = dl_obj->download_command;

        h->version = cvt->protocol_version;
        cur_cvt_num_obj++;
        continue;
    error:
        memset(h, 0, sizeof(struct cdl_adpu_cvt));
        ret = -1;
        break;
    }
    return ret;
}


static void print_cvt(void) {
	cdl_adpu_cvt_t h;
    int i, j;

    for (i = 0; i < cur_cvt_num_obj; i++) {
        h = (cdl_adpu_cvt_t)&cur_cvt[i];
        BDBG_MSG(("--------  CVT parsed  ----------------\n"));
        BDBG_MSG(("\t[%d] version %d", i, h->version));
        BDBG_MSG(("\t[%d]Download Cmd: %s", i, (h->cmd == DOWNLOAD_NOW) ? "Now" : "Deffered" ));

        switch (h->method) {
        case CDL_DLD_INBAND_SRC_ID:
            BDBG_MSG(("\t[%d]Type: CDL_DLD_INBAND_SRC_ID\n", i));
            BDBG_MSG(("\t[%d]source_id %d\n", i, h->t.type0.source_id));
            break;
        case CDL_DLD_INBAND_PID:
            BDBG_MSG(("\t[%d]Type: CDL_DLD_INBAND_PID\n", i));
            BDBG_MSG(("\t[%d]freq %d KHz\n", i, (h->t.type1.freq * 250000)));
            BDBG_MSG(("\t[%d]%s \n", i, (h->t.type1.mod_type == CVT_MOD_QAM64) ? "QAM64" :
                      ((h->t.type1.mod_type == CVT_MOD_QAM256) ? "QAM256" : "other")));
            BDBG_MSG(("\t[%d]pid 0x%x\n", i, h->t.type1.pid));
		break;
        case CDL_DLD_INBAND_PROG:
            BDBG_MSG(("\t[%d]Type: CDL_DLD_INBAND_PROG\n", i));
            BDBG_MSG(("\t[%d]freq %d KHz\n", i, (h->t.type2.freq * 250000)));
            BDBG_MSG(("\t[%d]%s \n", i, (h->t.type2.mod_type == 0x1) ? "QAM64" :
                      ((h->t.type2.mod_type == 0x2) ? "QAM256" : "other")));
            BDBG_MSG(("\t[%d]prog 0x%x\n", i, h->t.type2.program_number));
            break;
        case CDL_DLD_DSG_BASIC:
            BDBG_MSG(("\t[%d]Type: CDL_DLD_DSG_BASIC\n", i));
            BDBG_MSG(("\t[%d]dsg_tunnel_addr %s\n", i, h->t.type3.dsg_tunnel_addr));
            BDBG_MSG(("\t[%d]src_ip_addr %s\n", i, h->t.type3.src_ip_addr));
            BDBG_MSG(("\t[%d]dst_ip_addr %s\n", i, h->t.type3.dst_ip_addr));
            BDBG_MSG(("\t[%d]src_port %d\n", i, h->t.type3.src_port));
            BDBG_MSG(("\t[%d]dst_port %d\n", i, h->t.type3.dst_port));
            break;
        case CDL_DLD_DSG_ADVANCED:
            BDBG_MSG(("\t[%d]Type: CDL_DLD_DSG_ADVANCED\n", i));
            BDBG_MSG(("\t[%d]application_id %d\n", i, h->t.type4.application_id));
            break;
        case CDL_DLD_TFTP_BY_CVT:
            BDBG_MSG(("\t[%d]Type: CDL_DLD_TFTP_CVT\n", i));
            {
                struct in_addr iaddr;
                char * ch;
                /* server address is 16 byte ipv6 address, for ipv4, only the last 4 bytes matters */
                memcpy (&iaddr.s_addr, &h->t.tftp.server_addr[12], 4);
                ch = inet_ntoa(iaddr);
                BDBG_MSG(("\t[%d]Tftp_server_addr %s\n", i, (ch) ? ch : "NULL"));
                
            }
            break;
        default:
            BDBG_ERR(("\t[%d]invalid download method %d\n", i, h->method));
            break;
        }
        BDBG_MSG(("\t[%d]Obj_type %d\n", i, h->obj_type));
        BDBG_MSG(("\t[%d]Obj data_length %d\n", i, h->obj_data_length));
        BDBG_MSG(("\t[%d]Obj data\n", i));
        for (j = 0; j < h->obj_data_length; j++)
            BDBG_MSG(("%2x ", h->obj_data[j]));

        BDBG_MSG(("\t[%d]Code_filename_length %d\n", i, h->code_filename_length));
        BDBG_MSG(("\t[%d]Code_filename %s\n", i, h->code_filename));
        BDBG_MSG(("---------------------------------------\n"));
    }
}
/*
 * should be set explicitly by upper layer code
 */
/*
static int set_ctl() {
	int ret;
	cdl_adpu_cvt_t h;

	h = (cdl_adpu_cvt_t)&cur_cvt;
	if (cdl_ctl_set_method(h->method)  == 0) return 0;
	if (cdl_ctl_set_cmd(h->cmd) == 0) return 0;
 err:
	return 1;
}
*/
static int cdl_adpu_cvt_signal(void) {

	BDBG_MSG(("Enter %s\n", __FUNCTION__));
	B_Event_Set(cvt_event);
	return 0;
}

int cdl_adpu_init(void) {

	BDBG_MSG(("Enter %s\n", __FUNCTION__));
	cvt_event = B_Event_Create(NULL);
    if (cvt_event == NULL) {
        BDBG_ERR(("%s: Failed to create event", __FUNCTION__));
        return -1;
    }
	cdl_adpu_clear_all_cvt();
	return 0;
}

int cdl_adpu_uninit(){
	BDBG_MSG(("Enter %s\n", __FUNCTION__));
	B_Event_Destroy(cvt_event);
	return 0;
}

/*
 * return > 0 if new cvt received; 
 *        = 0 if same cvt received;
 *         <0 if fail
 */
static int cdl_adpu_cvt_set(void * data) {
	int ret = 0;
	DL_CVT2 * cvt;
    static struct timeval last_time = {0,0};
    struct timeval cur_time;

	cvt = &cur_raw_cvt;
    ret = memcmp(cvt, data, sizeof(DL_CVT2));
    if (ret == 0) {
      /* same CVT */
      gettimeofday(&cur_time, NULL);
      if (same_cvt_in_use) {
	return 0; /* return as existing CVT is being processed */
      } else { 
         /* need throttle , 1 pkt / 10 second */
         if ( (cur_time.tv_sec <= (last_time.tv_sec + 10) ) &&
             (cur_time.tv_sec >= (last_time.tv_sec - 10) ))
	  return 0;
	 /* the same CVT will be resent */
	memcpy(&last_time, &cur_time, sizeof(struct timeval));
      } 
    }


    /* parse cvt */

    print_raw_cvt((DL_CVT2 *) data);
    ret = parse_cvt(data);
    if (ret >= 0) {
        print_cvt();
        memcpy(cvt, data, sizeof(DL_CVT2));
        ret = 1;
    } else {
        BDBG_ERR(("parse cvt error\n"));
        ret = -1;
    }
	return ret;
}

#if 0
cdl_adpu_cvt_t cdl_adpu_cvt_get() {
	cdl_adpu_cvt_t h;
	BDBG_MSG(("Enter %s\n", __FUNCTION__));

	h = (cdl_adpu_cvt_t)&cur_cvt;
	return ((h->version)) ? h : NULL;
}
#endif
cdl_adpu_cvt_t cdl_adpu_cvt_get_next_object() {
	cdl_adpu_cvt_t h;
	BDBG_MSG(("Enter %s\n", __FUNCTION__));
    BDBG_MSG(("cur_Cvt_num_obj %d, cur_cvt_cur_obj %d", cur_cvt_num_obj, cur_cvt_cur_obj));
    if (cur_cvt_num_obj == 0 || 
        cur_cvt_cur_obj >= cur_cvt_num_obj) {
        same_cvt_in_use = 0;
        return NULL;
    }
	h = (cdl_adpu_cvt_t)&cur_cvt[cur_cvt_cur_obj];
    if (h->version) {
        cur_cvt_cur_obj++;
	same_cvt_in_use = 1;
        return h;
    } else {
      same_cvt_in_use = 0;
        return NULL;
    }
}
static B_MPOD_DL_CVT_HOST_RESPONSE cdl_adpu_cvt_process_internal(DL_CVT2 * cvt) {

	int i;
	unsigned char cvc_type;
	unsigned char * data;
	unsigned long len_left = 0;
	unsigned long n;
	B_MPOD_DL_CVT_HOST_RESPONSE cvt_response = B_MPOD_DL_OTHER_PARAMETER_ERROR;
    int new_cvt_received = 0;

	BDBG_MSG(("Enter %s\n", __FUNCTION__));

	cdl_ctl_set_state(CDL_CTL_ST_VERIFY_CVC);
#if defined(CDL_SUPPORT_ENABLE_SIGNED_IMAGE)
	data = cvt->cvc;
	len_left = cvt->cvc_len;
	for (i = 0; i < cvt->num_cvc; i++) {
		if (*data == CVT_MANUF_CVC) {
			cvc_type = cdl_estb_kManufCvc;
		} else if (*data == CVT_COSIGNER_CVC) {
			cvc_type = cdl_estb_kCosignerCvc;
		} else {
			BDBG_ERR(("invalid cvc type %02x", *data));
			goto error;
		}
		data++;
		len_left--;
		BDBG_MSG(("num_cvc %d, type %d, cvt->cvc[%d][0] %02x \n", cvt->num_cvc, i, cvc_type,
			  *data));
		if ((n = cdl_estb_check_cvc(data, len_left, cvc_type)) == 0) {
			BDBG_ERR(("CVC failed %d\n", i));
			cdl_ctl_set_state(-CDL_CTL_ST_VERIFY_CVC);
			goto error;
		}
		if (n > len_left) {
			BDBG_ERR(("CVC length %d exceed buffer %d", n, len_left));
			break;
		}
		data += n;
		len_left -= n;
		BDBG_MSG(("[cvc %d] len %d, len_left %d", i, n, len_left));

	}
	if (len_left) {
		BDBG_WRN(("CVT has %d garbage bytes\n", len_left));
	}
#else
    BDBG_MSG(("No CVC in cvt"));
#endif
	BDBG_MSG(("CVC ok\n"));
	/* update cvt */
    if (!cdl_ctl_is_busy()) {
        new_cvt_received = cdl_adpu_cvt_set((void *)cvt);
        if (new_cvt_received < 0) {
            cdl_ctl_set_state(-CDL_CTL_ST_PARSE_CVT);
            goto error;
        }
        /* send CVT only if cdl is not busy */
        cdl_ctl_set_state(CDL_CTL_ST_PARSE_CVT);
        cvt_response = B_MPOD_DL_ACT_NO_ERROR;
        cdl_adpu_cvt_signal();
    }

	return cvt_response;
 error:
	/* fixme! invalid id, or other error */
	BDBG_ERR(("TODO: invalid id or other error"));
	cvt_response = B_MPOD_DL_OTHER_PARAMETER_ERROR;
	return cvt_response;
}


int cdl_adpu_cvt_wait(void) {
#define TIMEWAIT_MS   500
	int ret;

	ret = B_Event_Wait(cvt_event, TIMEWAIT_MS);
	return ret;
}

int cdl_adpu_get_host_info(void * unused1, void *cdl_host_info) {
	B_MPOD_DL_HOST_INFO *p_host_info = (B_MPOD_DL_HOST_INFO *) cdl_host_info;

	unused1 = unused1; /* get ride of compiler warning */

	p_host_info->vendorId = cdl_ctl_get_vendor_id();
	p_host_info->hardwareVersionId = cdl_ctl_get_hw_version_id();
	p_host_info->numDescriptors = 0;

	BDBG_MSG(("vendor id 0x%x, hw_Version_id 0x%x\n",
		  p_host_info->vendorId, p_host_info->hardwareVersionId));

	return 0;
}

int cdl_adpu_cvt_type1_process(void * unused1,  unsigned char *data, unsigned long len) {
	DL_CVT2 *pcvt;
	unsigned char *pdata;
	unsigned short tmp;
	int i, j, k;
    int vendor_id_ok = 0;
    int hw_version_id_ok = 0;
	B_MPOD_DL_CVT_HOST_RESPONSE cvt_response = B_MPOD_DL_OTHER_PARAMETER_ERROR;
    int group_id_ok = 1; /* absense of group id tag is same as match */
    unsigned char *tmp_ptr;
    DL_CVT_OBJ * dl_obj;    



	unused1 = unused1; /* get ride of compiler warning */
	BDBG_MSG(("Download:DL_CODE_VER_TABLE1_TAG from POD, len %d\n", len));
    if (cdl_ctl_is_busy()) {
        BDBG_MSG(("%s: CDL busy", __FUNCTION__));
        return B_MPOD_DL_ACT_NO_ERROR;
    }
#if 0
	for (i = 0; i < len ; i++) {
		BDBG_MSG(("%02x ", data[i]));
		if ( i % 16 == 15) BDBG_MSG(("\n"));

	}

	BDBG_MSG(("\n"));
#endif
	pdata = data;
#define CAPTURE_CVT
#ifdef CAPTURE_CVT
	do {
		int fd;
		int n;
		char capture[64] ="./cvt_capture.bin";
		if ((fd = open(capture, O_CREAT | O_WRONLY)) < 0) {
			BDBG_ERR(("Cannot open %s\n", capture));
			break;
		}
		if ((n = write(fd, (void *)pdata, len)) < 0) {
			BDBG_ERR(("write error, expect %d, write %d", len, n));
			break;
		}
		BDBG_MSG(("Captured %ld bytes of CVT to %s", n, capture));
		close(fd);
	} while (0);
#endif

    cdl_adpu_clear_new_cvt();
	pcvt = &new_cvt;
    pcvt->type = 1;
	pcvt->num_descriptors = *data++;

	for (i = 0; i < pcvt->num_descriptors; i++) {
		pcvt->descriptor[i].tag = *data++;
		pcvt->descriptor[i].len = *data++;
        BDBG_MSG(("%d : tag %02x len %02x", i, pcvt->descriptor[i].tag, pcvt->descriptor[i].len));
		for (j = 0; j <(int) pcvt->descriptor[i].len; j++) {
			pcvt->descriptor[i].data[j] = *data++;
            BDBG_MSG(("%02x ", pcvt->descriptor[i].data[j]));
        }
        if (pcvt->descriptor[i].tag == (unsigned char) 0x00) {
            /* vendor_id */
            if (pcvt->descriptor[i].len == 3) {
                unsigned int vendor_id;

                tmp_ptr = (unsigned char *)&vendor_id;
                vendor_id = cdl_ctl_get_vendor_id();

                BDBG_MSG(("vendor id (box) %02x:%02x:%02x, (CVT) %02x:%02x:%02x",
                          tmp_ptr[0], tmp_ptr[1], tmp_ptr[2],
                          pcvt->descriptor[i].data[0],
                          pcvt->descriptor[i].data[1],
                          pcvt->descriptor[i].data[2]));
                if (memcmp(&pcvt->descriptor[i].data[0], (unsigned char *)&vendor_id, 3) == 0) {
                    BDBG_MSG(("vendor id matches"));
                    vendor_id_ok = 1;
                } else {
                    BDBG_MSG(("vendor id does not match"));
                }
            } else {
                BDBG_MSG(("vendor_id length %d does not equal to 3", pcvt->descriptor[i].len));
            }
        } else if (pcvt->descriptor[i].tag == (unsigned char) 0x01) {
            if (pcvt->descriptor[i].len == 4) {
                unsigned int hw_version_id;
                tmp_ptr = (unsigned char *)&hw_version_id;
                hw_version_id = cdl_ctl_get_hw_version_id();
                BDBG_MSG(("hw_version id (box) %02x:%02x:%02x:%02x, (CVT) %02x:%02x:%02x:%02x",
                          tmp_ptr[0], tmp_ptr[1], tmp_ptr[2], tmp_ptr[3], 
                          pcvt->descriptor[i].data[0],
                          pcvt->descriptor[i].data[1],
                          pcvt->descriptor[i].data[2], 
                          pcvt->descriptor[i].data[3]));
                if (memcmp(&pcvt->descriptor[i].data[0], (unsigned char *)&hw_version_id, 4) == 0) {
                    BDBG_MSG(("hw_version_id  matches"));
                    hw_version_id_ok = 1;
                } else {
                    BDBG_MSG(("hw_Version_id does not match"));
                }
            } else {
                BDBG_MSG(("hw_version_id length %d does not equal to 4", pcvt->descriptor[i].len));
            }
        } else if (pcvt->descriptor[i].tag == (unsigned char) 0x02) {
            if (pcvt->descriptor[i].len > 16) {
                BDBG_WRN(("Host propreitary data length is greater than 16 (0x%02x)", pcvt->descriptor[i].len));
            } 
        } else  {
            BDBG_WRN(("Unknown tag %d (len %d)", pcvt->descriptor[i].tag, pcvt->descriptor[i].len));
        }
	}
    if (!(vendor_id_ok && hw_version_id_ok)) {
        BDBG_MSG(("CVT does not match this box vendor_id %s, hw_version_id %s",
                  (vendor_id_ok) ? "match" : "not match",
                  (hw_version_id_ok) ? "match" : "not match" ));

        return B_MPOD_DL_ACT_NO_ERROR;
    }

    pcvt->protocol_version = 1;
    pcvt->num_obj = 1;
    dl_obj = &pcvt->obj[0];
    dl_obj->download_type = *data >> 4;
    dl_obj->download_command = *data++ & 0x0f;
    dl_obj->download_data.type0.location_type = 1;
    if (dl_obj->download_type == INBAND_FAT) {
        tmp = (((unsigned short) (*data++)) << 8) & 0xff00;
        dl_obj->download_data.type0.location_data.type1.freq = tmp | *data++;
        dl_obj->download_data.type0.location_data.type1.mod_type = *data++;
        tmp = (((unsigned short) (*data++)) << 8) & 0x1f00;
        dl_obj->download_data.type0.location_data.type1.pid = tmp | *data++;        
    } else {
        BDBG_ERR(("Wrong download type rcv'ed %d !!!\n", dl_obj->download_type));
        return B_MPOD_DL_OTHER_PARAMETER_ERROR;
    }

    dl_obj->code_filename_length = *data++;
    for (i = 0; i < dl_obj->code_filename_length; ++i)
        dl_obj->code_filename[i] = *data++;
    dl_obj->code_filename[dl_obj->code_filename_length] = 0;
    

    BDBG_MSG(("data %ld pdata %ld\n", (unsigned long) data, (unsigned long) pdata));

#if defined(CDL_SUPPORT_ENABLE_SIGNED_IMAGE)
	pcvt->num_cvc = 1;

	if ((unsigned long) (data - pdata) >= len) {
		BDBG_ERR(("data exceed len %ld\n", len));
		return B_MPOD_DL_OTHER_PARAMETER_ERROR;
	}
	pcvt->cvc_len = len - (unsigned long) (data - pdata);
	pcvt->cvc = data;
	BDBG_MSG(("code_file %s num_cvc %d, , cvc %x, cvc[0] %x, cvc len %ld\n", pcvt->obj[0].code_filename,
		  pcvt->num_cvc, (unsigned long) pcvt->cvc, pcvt->cvc[0], pcvt->cvc_len));
#else
    /* for cvt without cvc, (data - pdata ) == len is ok */
	pcvt->num_cvc = 0;
	if ((unsigned long) (data - pdata) > len) {
		BDBG_ERR(("data exceed len %ld\n", len));
		return B_MPOD_DL_OTHER_PARAMETER_ERROR;
	}
    BDBG_MSG(("No cvc processed !"));
	BDBG_MSG(("code_file %s \n", pcvt->obj[0].code_filename));
#endif
	cvt_response = cdl_adpu_cvt_process_internal(pcvt);
    cdl_adpu_clear_new_cvt();
	return cvt_response;
}

int cdl_adpu_cvt_type2_process(void * unused1,  unsigned char *data, unsigned long len) {
	DL_CVT2 *pcvt;
	unsigned char *pdata;
	unsigned short tmp;
	int i, j, k;
    int vendor_id_ok = 0;
    int hw_version_id_ok = 0;
	B_MPOD_DL_CVT_HOST_RESPONSE cvt_response = B_MPOD_DL_OTHER_PARAMETER_ERROR;
    int group_id_ok = 1; /* absense of group id tag is same as match */
    unsigned char *tmp_ptr;
    DL_CVT_OBJ * dl_obj;    

	unused1 = unused1; /* get ride of compiler warning */
	BDBG_MSG(("Download:DL_CODE_VER_TABLE2_TAG from POD, len %d\n", len));
    if (cdl_ctl_is_busy()) {
        BDBG_MSG(("%s: CDL busy", __FUNCTION__));
        return B_MPOD_DL_ACT_NO_ERROR;
    }
#if 0
	for (i = 0; i < len ; i++) {
		BDBG_MSG(("%02x ", data[i]));
		if ( i % 16 == 15) BDBG_MSG(("\n"));

	}

	BDBG_MSG(("\n"));
#endif
	pdata = data;
#define CAPTURE_CVT
#ifdef CAPTURE_CVT
	do {
		int fd;
		int n;
		char capture[64] ="./cvt_capture.bin";
		if ((fd = open(capture, O_CREAT | O_WRONLY)) < 0) {
			BDBG_ERR(("Cannot open %s\n", capture));
			break;
		}
		if ((n = write(fd, (void *)pdata, len)) < 0) {
			BDBG_ERR(("write error, expect %d, write %d", len, n));
			break;
		}
		BDBG_MSG(("Captured %ld bytes of CVT to %s", n, capture));
		close(fd);
	} while (0);
#endif

    cdl_adpu_clear_new_cvt();
	pcvt = &new_cvt;
    pcvt->type = 2;
	pcvt->protocol_version = *data++;
	pcvt->ccc = *data++;
	pcvt->num_descriptors = *data++;

	for (i = 0; i < pcvt->num_descriptors; i++) {
		pcvt->descriptor[i].tag = *data++;
		pcvt->descriptor[i].len = *data++;
        BDBG_MSG(("%d : tag %02x len %02x", i, pcvt->descriptor[i].tag, pcvt->descriptor[i].len));
		for (j = 0; j <(int) pcvt->descriptor[i].len; j++) {
			pcvt->descriptor[i].data[j] = *data++;
            BDBG_MSG(("%02x ", pcvt->descriptor[i].data[j]));
        }
        if (pcvt->descriptor[i].tag == (unsigned char) 0x00) {
            /* vendor_id */
            if (pcvt->descriptor[i].len == 3) {
                unsigned int vendor_id;

                tmp_ptr = (unsigned char *)&vendor_id;
                vendor_id = cdl_ctl_get_vendor_id();

                BDBG_MSG(("vendor id (box) %02x:%02x:%02x, (CVT) %02x:%02x:%02x",
                          tmp_ptr[0], tmp_ptr[1], tmp_ptr[2],
                          pcvt->descriptor[i].data[0],
                          pcvt->descriptor[i].data[1],
                          pcvt->descriptor[i].data[2]));
                if (memcmp(&pcvt->descriptor[i].data[0], (unsigned char *)&vendor_id, 3) == 0) {
                    BDBG_MSG(("vendor id matches"));
                    vendor_id_ok = 1;
                } else {
                    BDBG_MSG(("vendor id does not match"));
                }
            } else {
                BDBG_MSG(("vendor_id length %d does not equal to 3", pcvt->descriptor[i].len));
            }
        } else if (pcvt->descriptor[i].tag == (unsigned char) 0x01) {
            if (pcvt->descriptor[i].len == 4) {
                unsigned int hw_version_id;
                tmp_ptr = (unsigned char *)&hw_version_id;
                hw_version_id = cdl_ctl_get_hw_version_id();
                BDBG_MSG(("hw_version id (box) %02x:%02x:%02x:%02x, (CVT) %02x:%02x:%02x:%02x",
                          tmp_ptr[0], tmp_ptr[1], tmp_ptr[2], tmp_ptr[3], 
                          pcvt->descriptor[i].data[0],
                          pcvt->descriptor[i].data[1],
                          pcvt->descriptor[i].data[2], 
                          pcvt->descriptor[i].data[3]));
                if (memcmp(&pcvt->descriptor[i].data[0], (unsigned char *)&hw_version_id, 4) == 0) {
                    BDBG_MSG(("hw_version_id  matches"));
                    hw_version_id_ok = 1;
                } else {
                    BDBG_MSG(("hw_Version_id does not match"));
                }
            } else {
                BDBG_MSG(("hw_version_id length %d does not equal to 4", pcvt->descriptor[i].len));
            }
        } else if (pcvt->descriptor[i].tag == (unsigned char) 0x02) {
            if (pcvt->descriptor[i].len == 6) {
                BDBG_WRN(("Depricated tag host_MAC_addr (0x%02x)", pcvt->descriptor[i].tag));
            } else {
                BDBG_MSG(("Depricated tag host_MAC_addr length not equal to 6", pcvt->descriptor[i].len));
            }
        } else if (pcvt->descriptor[i].tag == (unsigned char) 0x03) {
            if (pcvt->descriptor[i].len == 5) {
                BDBG_WRN(("Depricated tag host_ID (0x%02x)", pcvt->descriptor[i].tag));
            } else {
                BDBG_MSG(("Depricated tag host_ID length not equal to 5", pcvt->descriptor[i].len));
            }
        } else if (pcvt->descriptor[i].tag == (unsigned char) 0x04) {
            if (pcvt->descriptor[i].len == 2) {
                unsigned int group_id;
                tmp_ptr = (unsigned char *)&group_id;
                group_id = cdl_ctl_get_group_id();
                BDBG_MSG(("group ID (box) %02x:%02x, (CVT) %02x:%02x",
                          tmp_ptr[0], tmp_ptr[1], 
                          pcvt->descriptor[i].data[0],
                          pcvt->descriptor[i].data[1]));
                if (memcmp(&pcvt->descriptor[i].data[0], (unsigned char *)&group_id, 2) == 0) {
                    BDBG_MSG(("group_id  matches"));
                    group_id_ok = 1;
                } else {
                    BDBG_MSG(("group_id does not match"));
                    group_id_ok = 0;
                }
            } else {
                BDBG_MSG(("group_id length %d does not equal to 2", pcvt->descriptor[i].len));
            }
        } else  {
            BDBG_WRN(("Unknown tag %d (len %d)", pcvt->descriptor[i].tag, pcvt->descriptor[i].len));
        }
	}
    if (!(vendor_id_ok && hw_version_id_ok)) {
        BDBG_MSG(("CVT does not match this box vendor_id %s, hw_version_id %s",
                  (vendor_id_ok) ? "match" : "not match",
                  (hw_version_id_ok) ? "match" : "not match" ));

        return B_MPOD_DL_ACT_NO_ERROR;
    }

    if (pcvt->protocol_version == 2 && group_id_ok == 0) {
        BDBG_MSG(("CVT (version 2) group_id does not match"));    
        return B_MPOD_DL_ACT_NO_ERROR;        
    }

    if (pcvt->protocol_version == 2) {
        pcvt->num_obj = *data++;
        if (pcvt->num_obj > CDL_ADPU_NUM_OBJ) {
            BDBG_MSG(("Too many objects in CVT %d, max %d", pcvt->num_obj, CDL_ADPU_NUM_OBJ));
        return B_MPOD_DL_ACT_NO_ERROR;        
        }
    } else if (pcvt->protocol_version == 1) {
        pcvt->num_obj = 1;    
    } else {
        BDBG_WRN((" CVT protocol version %d not supported", pcvt->protocol_version));
        return B_MPOD_DL_ACT_NO_ERROR;        
    }

    for (k = 0; k < pcvt->num_obj; k++) {
        dl_obj = &pcvt->obj[k];
        dl_obj->download_type = *data >> 4;
        dl_obj->download_command = *data++ & 0x0f;
        switch (dl_obj->download_type) {
        case INBAND_FAT:
            dl_obj->download_data.type0.location_type = *data++;
            if (dl_obj->download_data.type0.location_type == 0) {
                tmp = (((unsigned short) (*data++)) << 8) & 0xff00;
                BDBG_MSG(("tmp %d \n",tmp));
                dl_obj->download_data.type0.location_data.source_id = tmp | *data++;
                BDBG_MSG(("src id %d \n",dl_obj->download_data.type0.location_data.source_id));
            }
            else if (dl_obj->download_data.type0.location_type == 1) {
                tmp = (((unsigned short) (*data++)) << 8) & 0xff00;
                dl_obj->download_data.type0.location_data.type1.freq = tmp | *data++;
                dl_obj->download_data.type0.location_data.type1.mod_type = *data++;
                tmp = (((unsigned short) (*data++)) << 8) & 0x1f00;
                dl_obj->download_data.type0.location_data.type1.pid = tmp | *data++;
            } else	{
                tmp = (unsigned short) (*data++ << 8) & 0xff00;
                dl_obj->download_data.type0.location_data.type2.freq = tmp | *data++;
                dl_obj->download_data.type0.location_data.type2.mod_type = *data++;
                tmp = (unsigned short) (*data++ << 8) & 0xff00;
                dl_obj->download_data.type0.location_data.type2.program_number = tmp | *data++;
            }
            break;

        case DSG_DSM_CC:
            dl_obj->download_data.type1.location_type = *data++;
            if (dl_obj->download_data.type1.location_type == 3)	{
                for (i = 0; i < 6; ++i) /*dsg tunnel addr */
                    dl_obj->download_data.type1.location_data.type3.dsg_tunnel_addr[i] = *data++;
                for (i = 0; i < 16; ++i) /*source ip addr */
                    dl_obj->download_data.type1.location_data.type3.src_ip_addr[i] = *data++;
                for (i = 0; i < 16; ++i) /*dst ip addr */
                    dl_obj->download_data.type1.location_data.type3.dst_ip_addr[i] = *data++;
                tmp = (unsigned short) (*data++ << 8) & 0xff00;
                dl_obj->download_data.type1.location_data.type3.src_port= tmp | *data++;
                tmp = (unsigned short) (*data++ << 8) & 0xff00;
                dl_obj->download_data.type1.location_data.type3.dst_port= tmp | *data++;
            } else	{
                tmp = (unsigned short) (*data++ << 8) & 0xff00;
                dl_obj->download_data.type1.location_data.application_id = tmp | *data++;
            }
            break;
            
        case DOCSIS_TFTP:
            for (i = 0; i < 16; ++i) /*source ip addr */
                dl_obj->download_data.tftp_server_addr[i] = *data++;
            break;
        default:
            BDBG_ERR(("Wrong download type rcv'ed %d !!!\n", dl_obj->download_type));
            return B_MPOD_DL_OTHER_PARAMETER_ERROR;
        }
        if (pcvt->protocol_version == 2) {
            tmp = (unsigned short) (*data++ << 8) & 0xff00;
            dl_obj->obj_type = tmp | *data++;
            dl_obj->obj_data_length = *data++;
            for (i = 0; i < dl_obj->obj_data_length; ++i)
                dl_obj->obj_data[i] = *data++;
            dl_obj->code_filename_length = *data++;
        } else if (pcvt->protocol_version == 1) {
            dl_obj->obj_type = 0;
            dl_obj->obj_data_length = 0;
            memset(&dl_obj->obj_data[0], 0, sizeof(dl_obj->obj_data));
        }
        dl_obj->code_filename_length = *data++;
        for (i = 0; i < dl_obj->code_filename_length; ++i)
            dl_obj->code_filename[i] = *data++;
        dl_obj->code_filename[dl_obj->code_filename_length] = 0;
    }

    BDBG_MSG(("data %ld pdata %ld\n", (unsigned long) data, (unsigned long) pdata));

#if defined(CDL_SUPPORT_ENABLE_SIGNED_IMAGE)
	pcvt->num_cvc = *data++;

	if ((unsigned long) (data - pdata) >= len) {
		BDBG_ERR(("data exceed len %ld\n", len));
		return B_MPOD_DL_OTHER_PARAMETER_ERROR;
	}
	pcvt->cvc_len = len - (unsigned long) (data - pdata);
	pcvt->cvc = data;
	BDBG_MSG(("code_file %s num_cvc %d, , cvc %x, cvc[0] %x, cvc len %ld\n", pcvt->obj[0].code_filename,
		  pcvt->num_cvc, (unsigned long) pcvt->cvc, pcvt->cvc[0], pcvt->cvc_len));
#else
    /* for cvt without cvc, (data - pdata ) == len is ok */
	pcvt->num_cvc = 0;
	if ((unsigned long) (data - pdata) > len) {
		BDBG_ERR(("data exceed len %ld\n", len));
		return B_MPOD_DL_OTHER_PARAMETER_ERROR;
	}
    BDBG_MSG(("No cvc processed !"));
	BDBG_MSG(("code_file %s \n", pcvt->obj[0].code_filename));
#endif
	cvt_response = cdl_adpu_cvt_process_internal(pcvt);
    cdl_adpu_clear_new_cvt();
	return cvt_response;
}
void cdl_adpu_send_host_download_control(B_MPOD_DL_HOST_COMMAND command) {
#if 0
	B_Mpod_DownloadSendHostDlCtrl(command);
#else
	struct b_cdl_msg msg;
	memset(&msg, 0, sizeof(msg));
	msg.type = B_CDL_MSG_TO_APP_DOWNLOAD_NOTIFICATION;
	msg.to_app.notification.host_command = command;
	b_cdl_msg_send_to_app(&msg);
#endif
    BDBG_MSG(("%s: command %d", __FUNCTION__, command));
}

/***************************************************************************
*     (c)2003-2008 Broadcom Corporation
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
* Description:
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifdef HOST_ONLY
#include "b_dsmcc_lib_host.h"
#else
#include "b_os_lib.h"
#include "bdbg.h"
#include "bkni.h"
#endif
#include "b_dsmcc_lib.h"
#include "b_dsmcc_lib_priv.h"
#ifndef HOST_ONLY
#include "b_dsmcc_lib_priv_inband.h"
#endif
#include "b_dsmcc_lib_priv_file.h"
#include "b_dsmcc_lib_priv_dsg.h"
#include <netinet/in.h>
#include <string.h>

BDBG_MODULE(b_dsmcc_lib);


static B_Dsmcc_P_Handle s_Dsmcc_Handle = NULL;
static B_Dsmcc_Status s_status;

/* debug packet loss by simulating. set following env variable to simulate pkt loss.
 * 
 * export pkt_loss_times "2"   
 * export pkt_loss_interval "100" 
 */
/* #define DEBUG_PKT_LOSS   */
#ifdef DEBUG_PKT_LOSS
#define DEFAULT_PKT_LOSS_TIMES 4
#define DEFAULT_PKT_LOSS_INTERVAL 100

static int pkt_loss_times = DEFAULT_PKT_LOSS_TIMES;
static int pkt_loss_interval  = DEFAULT_PKT_LOSS_INTERVAL;
static int pkt_loss_count = 0;
static void pkt_loss_print() {
    BDBG_WRN(("pkt_loss_times = %d", pkt_loss_times));
    BDBG_WRN(("pkt_loss_interval = %d", pkt_loss_interval));
    BDBG_WRN(("pkt_loss_count = %d", pkt_loss_count));
}
static void pkt_loss_init(void) {
    char * str;
    pkt_loss_times = DEFAULT_PKT_LOSS_TIMES;
    pkt_loss_interval = DEFAULT_PKT_LOSS_INTERVAL;
    pkt_loss_count = 0;
    if ((str = getenv("pkt_loss_times"))) {
        pkt_loss_times = atoi(str);
    }
    if ((str = getenv("pkt_loss_interval"))) {
        pkt_loss_interval = atoi(str);
    }
    pkt_loss_print();
}
static int do_pkt_loss(void) {
    int ret = 0;
    if (!pkt_loss_times) return 0;
    if (!pkt_loss_interval) return 0;
    if (!pkt_loss_count) pkt_loss_count = pkt_loss_interval;
    
    pkt_loss_count--;
    if (pkt_loss_count) {
        ret = 0;
    } else {
        ret = 1;
        pkt_loss_times--;
    }
    return ret;
}
#endif

static void dump_data(void * buffer, int len, int ascii) {
#if 1
    BSTD_UNUSED(buffer);
    BSTD_UNUSED(len);
    BSTD_UNUSED(ascii);
#else
	int n;
	for (n=0;n<len;n++) {
		if (ascii) 
			BDBG_MSG(("%c", ((unsigned char *)buffer)[n]));
		else
			BDBG_MSG(("%02x", ((unsigned char *)buffer)[n]));
		if ( (n % 4) == 3) BDBG_MSG((" "));
		if ( (n % 16) == 15) BDBG_MSG(("\n"));
	}
	BDBG_MSG(("\n"));
#endif
}

static void parse_dsmcc_msg_dii(B_Dsmcc_P_Handle h, uint8_t * data) {
	uint16_t i, j;
	struct dsmcc_module_info * mi, *nmi;
	uint8_t * c;
	struct dsmcc_dii_msg * dii;


	if ((h->p_dii = BKNI_Malloc(sizeof(struct dsmcc_dii_msg))) == NULL) {
		BDBG_ERR(("out of memory\n"));
		return;
	}

	dii = (struct dsmcc_dii_msg * ) (data +  sizeof(struct dsmcc_section_header));
	mi = &dii->module_info;

	BKNI_Memcpy(h->p_dii, dii, sizeof(struct dsmcc_dii_msg));
	dii = h->p_dii;
	dii->dsmcc_message_header.message_id = ntohs(dii->dsmcc_message_header.message_id);
	dii->dsmcc_message_header.transaction_id = ntohl(dii->dsmcc_message_header.transaction_id);
	dii->dsmcc_message_header.message_length = ntohs(dii->dsmcc_message_header.message_length);
	dii->download_id = ntohl(dii->download_id);
	dii->block_size = ntohs(dii->block_size);
	dii->t_download_window = ntohl(dii->t_download_window);
	dii->t_download_scenario = ntohl(dii->t_download_scenario);
    s_status.inbandCarouselTimeoutUs = dii->t_download_scenario;
	dii->compatibility_descriptor_length = ntohs(dii->compatibility_descriptor_length);

	dii->number_of_modules = ntohs(dii->number_of_modules);

	BDBG_MSG(("Download Info Indicator Msg\n"));
	BDBG_MSG(("\tMessage header\n"));
	BDBG_MSG(("\t\tprotocol_discriminator: 0x%02x\n", dii->dsmcc_message_header.protocol_discriminator));
	BDBG_MSG(("\t\tdsmcc_type: 0x%02x\n", dii->dsmcc_message_header.dsmcc_type));
	BDBG_MSG(("\t\tmessage_id: 0x%04x\n", dii->dsmcc_message_header.message_id));
	BDBG_MSG(("\t\ttransaction_id: 0x%08x\n", dii->dsmcc_message_header.transaction_id));
	BDBG_MSG(("\t\treserved: 0x%02x\n", dii->dsmcc_message_header.reserved));
	BDBG_MSG(("\t\tadaptation_length: 0x%02x\n", dii->dsmcc_message_header.adaptation_length));
	BDBG_MSG(("\t\tmessage_length: 0x%04x\n", dii->dsmcc_message_header.message_length));

	/* BDBG_MSG(("\t\tdsmcc_adaptation_header: skipped\n")); */

	BDBG_MSG(("\tdownload_id: 0x%08x\n", dii->download_id));
	BDBG_MSG(("\tblock_size: 0x%04x\n", dii->block_size));
	BDBG_MSG(("\twindow_size: 0x%02x\n", dii->window_size));
	BDBG_MSG(("\tack_period: 0x%02x\n", dii->ack_period));
	BDBG_MSG(("\tt_download_window: 0x%08x\n", dii->t_download_window));
	BDBG_MSG(("\tt_download_scenario: 0x%08x\n", dii->t_download_scenario));

	BDBG_MSG(("\tcompatibility_descriptor.length: 0x%04x\n", dii->compatibility_descriptor_length));

	BDBG_MSG(("\tnumber_of_modules: 0x%04x\n", dii->number_of_modules));


	if ((h->p_module_info = BKNI_Malloc(dii->number_of_modules * sizeof(struct dsmcc_module_info *))) == NULL) {
		BDBG_ERR(("out of memory\n"));
		return;
	}
	if ((h->p_module_status = BKNI_Malloc(dii->number_of_modules * sizeof(struct dsmcc_module_status *))) == NULL) {
		BDBG_ERR(("out of memory\n"));
		return;
	}

	for (i = 0; i < dii->number_of_modules; i++) { 
		if ((h->p_module_info[i] = BKNI_Malloc(sizeof(struct dsmcc_module_info))) == NULL) {
			BDBG_ERR(("out of memory\n"));
			return;
		}
		BKNI_Memcpy(h->p_module_info[i], mi, sizeof(struct dsmcc_module_info));
		if ((h->p_module_status[i] = BKNI_Malloc(sizeof(struct dsmcc_module_status))) == NULL) {
			BDBG_ERR(("out of memory\n"));
			return;
		}
		BKNI_Memset(h->p_module_status[i], 0, sizeof(struct dsmcc_module_status));
		if ((h->p_module_info[i]->module_info_byte = BKNI_Malloc(mi->module_info_length)) == NULL) {
			BDBG_ERR(("out of memory\n"));
			return;
		}
		
		c  =  (uint8_t *) &mi->module_info_byte;
		BKNI_Memcpy(h->p_module_info[i]->module_info_byte, c, mi->module_info_length);
		nmi = h->p_module_info[i];
		nmi->module_id = ntohs(nmi->module_id);
		nmi->module_size = ntohl(nmi->module_size);

		BDBG_MSG(("\tModule info [%d]\n", i));
		BDBG_MSG(("\t\tmodule_id: 0x%04x\n", nmi->module_id));
		BDBG_MSG(("\t\tmodule_size: 0x%08x\n", nmi->module_size));
		BDBG_MSG(("\t\tmodule_version: 0x%02x\n", nmi->module_version));
		BDBG_MSG(("\t\tmodule_info_length: 0x%02x\n", nmi->module_info_length));
		BDBG_MSG(("\t\tmodule_info_byte: %d\n", nmi->module_info_length));
		for (j = 0; j < nmi->module_info_length; j++) {
			BDBG_MSG(("%02x %c", c[j], c[j])); 
			/* BDBG_MSG(("%c", c[i])); */
		}
		BDBG_MSG(("\n"));
		c+= mi->module_info_length;
		mi = (struct dsmcc_module_info * ) c;

        h->p_module_status[i]->state = STATE_INIT;
        h->p_module_status[i]->module_id = nmi->module_id;

	}
	BDBG_MSG(("\t\tprivate data: skipped\n"));
}

static void parse_dsmcc_msg_ddb(B_Dsmcc_P_Handle h, uint8_t *data) {
	struct dsmcc_ddb *  ddb;

	if ((h->p_ddb = BKNI_Malloc(sizeof(struct dsmcc_ddb))) == NULL) {
		BDBG_ERR(("out of memory\n"));
		return;
	}

	ddb = (struct dsmcc_ddb * ) (data +  sizeof(struct dsmcc_section_header));
	BKNI_Memcpy(h->p_ddb, ddb, sizeof(struct dsmcc_ddb));
	ddb = h->p_ddb;
	ddb->download_data_header.message_id = ntohs(ddb->download_data_header.message_id);
	ddb->download_data_header.download_id = ntohl(ddb->download_data_header.download_id);
	ddb->download_data_header.message_length = ntohs(ddb->download_data_header.message_length);
	ddb->module_id = ntohs(ddb->module_id);
	ddb->block_number = ntohs(ddb->block_number);

	BDBG_MSG(("Download Data Block\n"));
	BDBG_MSG(("\tMessage header\n"));
	BDBG_MSG(("\t\tprotocol_discriminator: 0x%02x\n", ddb->download_data_header.protocol_discriminator));
	BDBG_MSG(("\t\tdsmcc_type: 0x%02x\n", ddb->download_data_header.dsmcc_type));
	BDBG_MSG(("\t\tmessage_id: 0x%04x\n", ddb->download_data_header.message_id));
	BDBG_MSG(("\t\tdownload_id: 0x%08x\n", ddb->download_data_header.download_id));
	BDBG_MSG(("\t\treserved: 0x%02x\n", ddb->download_data_header.reserved));
	BDBG_MSG(("\t\tadaptation_length: 0x%02x\n", ddb->download_data_header.adaptation_length));
	BDBG_MSG(("\t\tmessage_length: 0x%04x\n", ddb->download_data_header.message_length));

	/* BDBG_MSG(("\t\tdsmcc_adaptation_header: skipped\n")); */

	BDBG_MSG(("\tmodule_id: 0x%04x\n", ddb->module_id));
	BDBG_MSG(("\tmodule_version: 0x%02x\n", ddb->module_version));
	BDBG_MSG(("\treserved: 0x%02x\n", ddb->reserved));
	BDBG_MSG(("\tblock_number: 0x%04x\n", ddb->block_number));
	dump_data(&ddb->block_data_byte, 0x10, 0);
}

void parse_dsmcc_sect(B_Dsmcc_P_Handle h, uint8_t *data) {
	struct dsmcc_section_header * a;
	h->p_sect = (struct dsmcc_section_header *) data;
	a = h->p_sect;
	BDBG_MSG(("\nDSMCC section\n"));
	BDBG_MSG(("\tTABLE_ID: 0x%02x\n", DSMCC_SECTION_TABLE_ID(a)));
	BDBG_MSG(("\tSYNTAX_INDICATOR: 0x%02x\n", DSMCC_SECTION_SYNTAX_INDICATOR(a)));
	BDBG_MSG(("\tSECTION_LEN: 0x%04x\n", DSMCC_SECTION_SECTION_LEN(a)));
	BDBG_MSG(("\tTABLE_ID_EXT: 0x%04x\n",DSMCC_SECTION_TABLE_ID_EXT(a)));
	BDBG_MSG(("\tVERSION: 0x%02x\n",DSMCC_SECTION_VER(a)));
	BDBG_MSG(("\tCUR_NEXT_INDICATOR: 0x%02x\n",DSMCC_SECTION_CUR_NEXT_INDICATOR(a)));
	BDBG_MSG(("\tSECT_NUM: 0x%02x\n",DSMCC_SECTION_SECT_NUM(a)));
	BDBG_MSG(("\tLAST_SECT_NUM: 0x%02x\n",DSMCC_SECTION_LAST_SECT_NUM(a)));
	if (DSMCC_SECTION_TABLE_ID(a) == DSMCC_TABLE_ID_DII_OR_DSI) {
		parse_dsmcc_msg_dii(h, data);
	} else if (DSMCC_SECTION_TABLE_ID(a) == DSMCC_TABLE_ID_DDB) {
		parse_dsmcc_msg_ddb(h, data);
	} else {
		BDBG_ERR(("unrecogazined table id\n"));
	}
	

}


struct dsmcc_module_status * get_module_status_from_module_id(B_Dsmcc_P_Handle hDsmcc, int module_id) {
	struct dsmcc_dii_msg * dii = hDsmcc->p_dii;
    int i;
	for (i = 0; i < dii->number_of_modules; i++) { 
        BDBG_MSG(("module [%d] id %d, module_id %d, num_of_modules %d", i,hDsmcc->p_module_status[i]->module_id,  
                  module_id,  dii->number_of_modules));

		if (hDsmcc->p_module_status[i]->module_id == module_id) {
            return (struct dsmcc_module_status * )hDsmcc->p_module_status[i];
        }
    }
    BDBG_ERR(("Module id %d status not found", module_id));
    return NULL;
}

struct dsmcc_module_info * get_module_info_from_module_id(B_Dsmcc_P_Handle hDsmcc, int module_id) {
	struct dsmcc_dii_msg * dii = hDsmcc->p_dii;
    int i;
	for (i = 0; i < dii->number_of_modules; i++) { 
		if (hDsmcc->p_module_info[i]->module_id == module_id) {
            return (struct dsmcc_module_info * )hDsmcc->p_module_info[i];
        }
    }
    BDBG_ERR(("Module id %d info not found", module_id));
    hDsmcc->status->error = B_DSMCC_ERR_MODULE_NOT_FOUND;

    return NULL;
}

static int dsmcc_is_done(B_Dsmcc_P_Handle hDsmcc, int module_id) {
	uint32_t module_size;
	struct dsmcc_module_status * ms;
	struct dsmcc_module_info * mi;
    ms = get_module_status_from_module_id(hDsmcc, module_id);
    if (!ms) return 0;
    mi = get_module_info_from_module_id(hDsmcc, module_id);
    if (!mi) return 0;

	module_size = mi->module_size;
	return (((ms->state >= STATE_DONE) && (ms->module_id == module_id) 
	    && (ms->offset >= module_size)) ? 1: 0);
}

static int dsmcc_is_started(B_Dsmcc_P_Handle hDsmcc, int module_id) {
	struct dsmcc_module_status * ms;
    ms = get_module_status_from_module_id(hDsmcc, module_id);
    if (!ms) return 0;

	return (((ms->state >= STATE_FILTER_STARTED) && (ms->module_id == module_id)) ? 1: 0);
}
static int dsmcc_stop(B_Dsmcc_P_Handle hDsmcc, int module_id) {
	struct dsmcc_module_status * ms;

	BDBG_MSG(("Enter %s\n", __FUNCTION__));
    ms = get_module_status_from_module_id(hDsmcc, module_id);
    if (!ms) goto error;

	if (ms->state >= STATE_FILTER_STARTED) {
		hDsmcc->stop(hDsmcc);
		ms->state = STATE_INIT;
        ms->next_block = 0;
        ms->module_id = -1;
        ms->offset = 0;
	}
    hDsmcc->current_module = -1;
	return 0;
error:
        BDBG_ERR(("%s error", __FUNCTION__));
        return -1;
}

static int dsmcc_start(B_Dsmcc_P_Handle h, int module_id) {
	struct dsmcc_module_status * ms;
	struct dsmcc_module_info * mi;
	BDBG_MSG(("Enter %s\n", __FUNCTION__));
	
    ms = get_module_status_from_module_id(h, module_id);
    if (!ms) return 0;
    mi = get_module_info_from_module_id(h, module_id);
    if (!mi) return 0;

    if (h->start(h, module_id)) goto error;
	ms->state = STATE_FILTER_STARTED;
	ms->module_id = module_id;
    h->current_module = module_id;

	return 0;
 error: 
	BDBG_ERR(("%s error\n", __FUNCTION__));
	ms->state = STATE_INIT;
	return -1;
}


/***************************************************************************
Summary:
This function returns the default and recommended values for the App Lib
public settings. A pointer to a valid B_Dsmcc_Settings structure must be
provide or an error will be returned.
***************************************************************************/
B_Error B_Dsmcc_GetDefaultSettings(B_Dsmcc_Settings *pDsmccSettings)
{
	BDBG_ENTER(B_Dsmcc_GetDefaultSettings);

	if (NULL == pDsmccSettings) return BERR_INVALID_PARAMETER;
	BKNI_Memset( pDsmccSettings, 0, sizeof(B_Dsmcc_Settings) );
    pDsmccSettings->pidChannel = NULL;
    pDsmccSettings->inputType = B_DSMCC_INPUT_TYPE_INBAND;

    pDsmccSettings->tunnel = b_dsmcc_lib_priv_dsg_get_tunnel_handler();
	BDBG_LEAVE(B_Dsmcc_GetDefaultSettings);
	return BERR_SUCCESS;
}
/***************************************************************************
Summary:
This function initializes the App Lib based on the settings selected. A shallow
copy of the B_Dsmcc_Settings structure is made in this call. The private App Lib
structure is malloc'ed.
***************************************************************************/
B_Dsmcc_Handle * B_Dsmcc_Open(B_Dsmcc_Settings *pDsmccSettings)
{
    B_Dsmcc_P_Handle hDsmcc = NULL;

    if (NULL == pDsmccSettings) return NULL;

    BDBG_ENTER(B_Dsmcc_Open);

    if (s_Dsmcc_Handle) {
        BDBG_ERR(("s_Dsmcc_Handle already opened. Close it first"));
        return NULL;
    }
    hDsmcc = (B_Dsmcc_P_Handle) BKNI_Malloc(sizeof(struct B_Dsmcc_P_Struct));
    if(NULL == hDsmcc) {
        BDBG_ERR(("Malloc failed"));
        return NULL;
    }
    BKNI_Memset(hDsmcc, 0, sizeof(struct B_Dsmcc_P_Struct));
    hDsmcc->current_module = -1;
    BKNI_Memset(&s_status, 0, sizeof(struct B_Dsmcc_Status));
    hDsmcc->status = &s_status;

#ifndef HOST_ONLY
    if (pDsmccSettings->inputType == B_DSMCC_INPUT_TYPE_INBAND) {
        hDsmcc->open = b_dsmcc_lib_priv_inband_open;
        hDsmcc->close = b_dsmcc_lib_priv_inband_close;
        hDsmcc->stop = b_dsmcc_lib_priv_inband_stop;
        hDsmcc->start = b_dsmcc_lib_priv_inband_start;
        hDsmcc->get_buffer = b_dsmcc_lib_priv_inband_get_buffer;
        hDsmcc->read_complete = b_dsmcc_lib_priv_inband_read_complete;
    } else 
#endif /* HOST_ONLY */
    if (pDsmccSettings->inputType == B_DSMCC_INPUT_TYPE_FILE) {
        hDsmcc->open = b_dsmcc_lib_priv_file_open;
        hDsmcc->close = b_dsmcc_lib_priv_file_close;
        hDsmcc->stop = b_dsmcc_lib_priv_file_stop;
        hDsmcc->start = b_dsmcc_lib_priv_file_start;
        hDsmcc->get_buffer = b_dsmcc_lib_priv_file_get_buffer;
        hDsmcc->read_complete = b_dsmcc_lib_priv_file_read_complete;
    } else if (pDsmccSettings->inputType == B_DSMCC_INPUT_TYPE_DSG) {
        hDsmcc->open = b_dsmcc_lib_priv_dsg_open;
        hDsmcc->close = b_dsmcc_lib_priv_dsg_close;
        hDsmcc->stop = b_dsmcc_lib_priv_dsg_stop;
        hDsmcc->start = b_dsmcc_lib_priv_dsg_start;
        hDsmcc->get_buffer = b_dsmcc_lib_priv_dsg_get_buffer;
        hDsmcc->read_complete = b_dsmcc_lib_priv_dsg_read_complete;
    } else {
        BDBG_ERR(("Unsupported type %d", pDsmccSettings->inputType));
        BKNI_Free(hDsmcc);
        return NULL; 
    }

    if (hDsmcc->open(hDsmcc, pDsmccSettings) < 0) {    
        BDBG_ERR(("Cannot open input %d", pDsmccSettings->inputType));
        BKNI_Free(hDsmcc);
        return NULL;
    }
    s_Dsmcc_Handle = hDsmcc;
#ifdef DEBUG_PKT_LOSS
    pkt_loss_init();
#endif

    BDBG_LEAVE(B_Dsmcc_Open);
    return (B_Dsmcc_Handle) hDsmcc;
}

/***************************************************************************
Summary:
This function de-initializes the App Lib. The private App Lib structure is freed.
***************************************************************************/
B_Error B_Dsmcc_Close(B_Dsmcc_Handle h)
{
    int i;
    struct dsmcc_module_info * mi;
    struct dsmcc_dii_msg *  dii;
    B_Dsmcc_P_Handle hDsmcc = (B_Dsmcc_P_Handle) h;

    BDBG_ENTER(B_Dsmcc_Close);
    if (NULL == hDsmcc) return BERR_INVALID_PARAMETER;

    dii = hDsmcc->p_dii;

    for (i = 0; i < dii->number_of_modules; i++) {
            if (dsmcc_is_started(hDsmcc, i)) {
                    dsmcc_stop(hDsmcc, i);
            }
            mi = hDsmcc->p_module_info[i];
            BKNI_Free(mi->module_info_byte);
            BKNI_Free(mi);		
            BKNI_Free(hDsmcc->p_module_status[i]);
    }
    hDsmcc->close(hDsmcc);
    if (hDsmcc->p_module_info) {
        BKNI_Free(hDsmcc->p_module_info);
        hDsmcc->p_module_info = NULL;
    }
    if (hDsmcc->p_module_status) {
        BKNI_Free(hDsmcc->p_module_status);
        hDsmcc->p_module_status = NULL;
    }
    if (hDsmcc->p_dii) {
        BKNI_Free(hDsmcc->p_dii);
        hDsmcc->p_dii = NULL;
    }
    if (hDsmcc->p_ddb) { 
        BKNI_Free(hDsmcc->p_ddb);
        hDsmcc->p_ddb = NULL;
    }
    BKNI_Free(hDsmcc);
    s_Dsmcc_Handle = NULL;
    BDBG_LEAVE(B_Dsmcc_Close);
    return BERR_SUCCESS;
}
/***************************************************************************
Summary:
This function read data from dsmcc data carousel. 
It waits for the first block of data before starting.
***************************************************************************/
size_t B_Dsmcc_Read(B_Dsmcc_Handle h, int module_id, unsigned char * buf, size_t len)
{
    B_Dsmcc_P_Handle hDsmcc = (B_Dsmcc_P_Handle) h;
    unsigned char *buffer;
    size_t size = 0;
    int ret = 0;
    size_t left = 0;
    size_t offset = 0;
    uint16_t block_size = 0;
    uint32_t module_size = 0;
    uint32_t skip_head = 0;
    uint32_t skip_tail = 0;
    uint16_t block_number;
	struct dsmcc_module_status * ms;
	struct dsmcc_module_info * mi;
    struct dsmcc_dii_msg *  dii;

    BDBG_ENTER(B_Dsmcc_Read);
    BDBG_ASSERT(NULL != hDsmcc);

    dii = hDsmcc->p_dii;

    BDBG_MSG(("Enter %s\n", __FUNCTION__));
    ms = get_module_status_from_module_id(hDsmcc, module_id);
    if (!ms) return 0;
    mi = get_module_info_from_module_id(hDsmcc, module_id);
    if (!mi) return 0;

    /* update module info */
    hDsmcc->status->module_id = module_id;
    hDsmcc->status->offset = ms->offset;
    hDsmcc->status->module_size = mi->module_size;
    hDsmcc->status->module_info_length = mi->module_info_length;
    memcpy(hDsmcc->status->module_info_byte, mi->module_info_byte, mi->module_info_length);
    hDsmcc->status->block_size = dii->block_size;

    if (len == 0 ) return 0;
    block_size = dii->block_size;
    module_size = mi->module_size;
    if (!block_size) {
            BDBG_ERR(("block size == 0\n"));
            return 0;
    }
    if (dsmcc_is_done(hDsmcc, module_id)) {
            BDBG_WRN(("[%d] EOF, offset %d, blk %d, size %d\n", module_id, ms->offset, 
                     dii->block_size, mi->module_size));
            /* stop to avoid msg overflow */
            dsmcc_stop(hDsmcc, module_id); 
            return 0;
    }
    if ((hDsmcc->current_module != module_id) && hDsmcc->current_module >= 0) {
        /* if there's another module running, stop it */
        if (!dsmcc_is_started(hDsmcc, hDsmcc->current_module)) {
                dsmcc_stop(hDsmcc, hDsmcc->current_module);
        }
        
    } else {
        if (!dsmcc_is_started(hDsmcc, module_id)) {
            dsmcc_start(hDsmcc, module_id);
        }
    }

    /*
     * To simplify the algorithm, each time we call B_Dsmcc_Read(), the message buffer
     * starts with a complete PSI DDB message. 
     * the return data size is always integer of block_size.
     * only for the last block, the actual remaining data is returned.
     */
    if (ms->offset + len >= mi->module_size) {
        /* include the last block */
        left = mi->module_size - ms->offset;
        BDBG_WRN(("%d bytes left", left));
    } else {
        /* not include the last block */
        left = (len / block_size) * block_size; 
        BDBG_WRN(("Reading %d bytes", left));
    }
    while ( (left != 0) && (ms->state != STATE_DONE)) {
            skip_head = 0;
            skip_tail = 0;
            if ( hDsmcc->get_buffer(hDsmcc, (unsigned char **)&buffer, &size)) goto error;
            

            /* dump_data(buffer, 80, 0);  */
            /* parse_dsmcc_sect(h, buffer);  */

            /* 
             * check the start of a dsmcc section 
             * Since NEXUS_Message_GetBuffer() always return a whole PSI message, 
             * we can expect the DDB header for each message received.
             */
            skip_head = DSMCC_DDB_BLOCK_DATA_BYTE_OFFSET;
            if (size < skip_head) goto error;

            block_number = ntohs(DSMCC_DDB_BLOCK_NUMBER(buffer));
            BDBG_MSG(("\n[%d] Recv %d bytes, blk# %d, off %d, exp blk %d\n", module_id, size, 
                     block_number,  ms->offset, ms->next_block)); 

            
            /* wait for the first block */
            if (ms->state < STATE_READ_STARTED && block_number != 0) {
                hDsmcc->read_complete(hDsmcc, size);
                continue;
            }
            ms->state = STATE_READ_STARTED;

            /* wait for the next block */
            if (ms->state == STATE_READ_STARTED && block_number != ms->next_block) {
                hDsmcc->read_complete(hDsmcc, size);
                continue;
            }
            /* 
             * for a 4000 byte section, we received 4032 bytes, with 26 bytes header, + 4000 bytes data
             * + 4 bytes CRC + 2 bytes padding 
             * #define TAIL_BYTES 6 
             # for a 4096 byte section, data is 4066 byte + 26 byte header  + 4 byte CRC = 4096  */
            if ( block_number == ms->next_block) {
                /* data size is equal to block size except the last block */
                len = ((mi->module_size - ms->offset) >= block_size) ? block_size : left;
                skip_tail = size - len - skip_head;
            }
            BKNI_Memcpy(buf + offset, (uint8_t *)buffer + skip_head, len);
            left -= len;
            offset += len;
            ms->offset += len;

            BDBG_MSG(("[%d] Done blk %d, size %d, offset %d, skip_head %d, skip_tail %d left %d\n", 
                      module_id, dii->block_size, mi->module_size, ms->offset, skip_head, skip_tail, left));
            if (ms->offset % block_size == 0) {
                    ms->next_block++;
            }
            if (ms->offset == mi->module_size) { 
                    ms->state = STATE_DONE;
            }
            /* BDBG_MSG(("%d - %d\n", module_id, mi->module_id)); */
            hDsmcc->read_complete(hDsmcc, size);
    }
    /* update download status */
    hDsmcc->status->offset = ms->offset;

    ret = offset;
    return ret;
error:
    BDBG_ERR(("read error\n"));
    return ret;
    
    BDBG_LEAVE(B_Dsmcc_Read);
    return 0;
}

/***************************************************************************
Summary:
This function read data from dsmcc data carousel. 
It does not wait for the first block. Instead the current block number is 
returned along with the data.
***************************************************************************/
size_t B_Dsmcc_Read_NoWait(B_Dsmcc_Handle h, int module_id, unsigned char * buf, size_t len, uint16_t * block_nr)
{
#define INVALID_BLOCK_NUMBER 0xFFFF
    B_Dsmcc_P_Handle hDsmcc = (B_Dsmcc_P_Handle) h;
    unsigned char *buffer;
    size_t size = 0;
    int ret = 0;
    size_t left = 0;
    size_t tmp_len = 0;
    size_t offset = 0;
    uint16_t block_size = 0;
    uint32_t module_size = 0;
    uint32_t skip_head = 0;
    uint32_t skip_tail = 0;
    uint16_t block_number = INVALID_BLOCK_NUMBER;
	struct dsmcc_module_status * ms;
	struct dsmcc_module_info * mi;
    struct dsmcc_dii_msg *  dii;

    BDBG_ENTER(B_Dsmcc_Read);
    BDBG_ASSERT(NULL != hDsmcc);

    if (!block_nr) return 0;

    dii = hDsmcc->p_dii;

    BDBG_MSG(("Enter %s\n", __FUNCTION__));
    ms = get_module_status_from_module_id(hDsmcc, module_id);
    if (!ms) return 0;
    mi = get_module_info_from_module_id(hDsmcc, module_id);
    if (!mi) return 0;

    /* update module info */
    hDsmcc->status->module_id = module_id;
    hDsmcc->status->offset = ms->offset;
    hDsmcc->status->module_size = mi->module_size;
    hDsmcc->status->module_info_length = mi->module_info_length;
    memcpy(hDsmcc->status->module_info_byte, mi->module_info_byte, mi->module_info_length);
    hDsmcc->status->block_size = dii->block_size;
    
    if (len == 0 ) return 0;
    block_size = dii->block_size;
    module_size = mi->module_size;
    if (!block_size) {
            BDBG_ERR(("block size == 0\n"));
            return 0;
    }
    if (dsmcc_is_done(hDsmcc, module_id)) {
            BDBG_WRN(("[%d] EOF, offset %d, blk %d, size %d\n", module_id, ms->offset, 
                     dii->block_size, mi->module_size));
            /* stop to avoid msg overflow */
            dsmcc_stop(hDsmcc, module_id); 
            return 0;
    }
    if ((hDsmcc->current_module != module_id) && hDsmcc->current_module >= 0) {
        /* if there's another module running, stop it */
        if (!dsmcc_is_started(hDsmcc, hDsmcc->current_module)) {
                dsmcc_stop(hDsmcc, hDsmcc->current_module);
        }
        
    } else {
        if (!dsmcc_is_started(hDsmcc, module_id)) {
            dsmcc_start(hDsmcc, module_id);
        }
    }

    /*
     * To simplify the algorithm, each time we call B_Dsmcc_Read(), the message buffer
     * starts with a complete PSI DDB message. 
     * the return data size is always integer of block_size.
     * only for the last block, the actual remaining data is returned.
     */
    left = (len >= block_size) ? ((len) / block_size) * block_size : len; 
    *block_nr = INVALID_BLOCK_NUMBER;
    ms->state = STATE_FILTER_STARTED;

    while ( (left != 0) && (ms->state != STATE_DONE)) { 
            skip_head = 0;
            skip_tail = 0;
            if ( hDsmcc->get_buffer(hDsmcc, (unsigned char **)&buffer, &size)) goto error;

            /* dump_data(buffer, 80, 0);  */
            /* parse_dsmcc_sect(h, buffer);  */

            /* 
             * check the start of a dsmcc section 
             * Since NEXUS_Message_GetBuffer() always return a whole PSI message, 
             * we can expect the DDB header for each message received.
             */
            skip_head = DSMCC_DDB_BLOCK_DATA_BYTE_OFFSET;
            if (size < skip_head) goto error;

            block_number = ntohs(DSMCC_DDB_BLOCK_NUMBER(buffer));
#ifdef DEBUG_PKT_LOSS
            if (do_pkt_loss()) {
                /* drop a packet */
                BDBG_WRN(("\n[%d] Drop %d bytes blk# %d offset 0x%x\n", module_id, size, block_number, 
                          block_number * block_size)); 

                hDsmcc->read_complete(hDsmcc, size);
                continue;
            }
#endif            
            
            /* no wait for the first block 
             * just return data from current block. 
             */
            BDBG_MSG(("\n[%d] Recv %d bytes, blk# %d, off %d, exp blk %d *block_nr %d\n", module_id, size, 
                      block_number,  ms->offset, ms->next_block, *block_nr)); 
            if (INVALID_BLOCK_NUMBER == *block_nr) {
                ms->state = STATE_READ_STARTED;
                *block_nr = block_number;
                ms->next_block = block_number;
                ms->offset = block_number * block_size;
                BDBG_MSG(("Starting block %d, offset 0x%x", ms->next_block, ms->offset));
                if (ms->offset + len >= mi->module_size) {
                    /* include the last block */
                    left = mi->module_size - ms->offset;
                    BDBG_WRN(("%d bytes left", left));
                }
            }
            /* wait for the next block */
            if (ms->state == STATE_READ_STARTED && block_number != ms->next_block) {
                /* missing a block, reset block_nr */
                BDBG_WRN(("Missing block %d, discard 0x%x bytes", ms->next_block, offset));
                *block_nr = INVALID_BLOCK_NUMBER;
                left = (len >= block_size) ? ((len) / block_size) * block_size : len; 
                ms->state = STATE_FILTER_STARTED;
                offset = 0;
                hDsmcc->read_complete(hDsmcc, size);
                continue;
            }
            /* 
             * for a 4000 byte section, we received 4032 bytes, with 26 bytes header, + 4000 bytes data
             * + 4 bytes CRC + 2 bytes padding 
             * #define TAIL_BYTES 6 
             # for a 4096 byte section, data is 4066 byte + 26 byte header  + 4 byte CRC = 4096  */
            if ( block_number == ms->next_block) {
                /* data size is equal to block size except the last block */
                tmp_len = ((mi->module_size - ms->offset) >= block_size) ? block_size : left;
                skip_tail = size - tmp_len - skip_head;
            }
            BKNI_Memcpy(buf + offset, (uint8_t *)buffer + skip_head, tmp_len);
            left -= tmp_len;
            offset += tmp_len;
            ms->offset += tmp_len;

            BDBG_MSG(("[%d] Done blk %d, size %d, offset %d, skip_head %d, skip_tail %d left %d\n", 
                      module_id, dii->block_size, mi->module_size, ms->offset, skip_head, skip_tail, left));
            if (ms->offset % block_size == 0) {
                    ms->next_block++;
            }
            /*
            if (ms->offset == mi->module_size) { 
                    ms->state = STATE_DONE;
            }
            */
            /* BDBG_MSG(("%d - %d\n", module_id, mi->module_id)); */
            hDsmcc->read_complete(hDsmcc, size);
    }
    /* update download status */
    hDsmcc->status->offset = ms->offset;

    ret = offset;
    return ret;
error:
    BDBG_ERR(("read error\n"));
    return ret;
    
    BDBG_LEAVE(B_Dsmcc_Read);
    return 0;
}

int B_Dsmcc_GetNumOfModules(B_Dsmcc_Handle h) {
    B_Dsmcc_P_Handle hDsmcc = (B_Dsmcc_P_Handle) h;
    struct dsmcc_dii_msg * dii;

    if (NULL == hDsmcc) {
        BDBG_ERR(("Null pointer"));
        return -1;
    }
    dii = hDsmcc->p_dii;
    if (NULL == dii) {
        BDBG_ERR(("Null pointer"));
        return -1;
    }

    return dii->number_of_modules;
}

int B_Dsmcc_GetModuleIdFromName(B_Dsmcc_Handle h, uint8_t * name, int len) {
    int i, j;
    int module_id = 0; 
    int module_info_length = 0;
    int number_of_modules = 0;
	struct dsmcc_module_info *nmi;
    B_Dsmcc_P_Handle hDsmcc = (B_Dsmcc_P_Handle) h;

    if (NULL == hDsmcc) {
        BDBG_ERR(("Null pointer"));
        return -1;
    }
    number_of_modules = B_Dsmcc_GetNumOfModules(h);
    if ( number_of_modules < 0) {
        return -1;
    }
    BDBG_MSG(("%s: name %s, len %d", __FUNCTION__, name, len));
    for ( i = 0; i < number_of_modules; i++) {
        nmi = hDsmcc->p_module_info[i];
        module_id = nmi->module_id;
        module_info_length = nmi->module_info_length;
        BDBG_MSG(("module[%d] id = %d, module info length %d", i, module_id, module_info_length));
		for (j = 0; j < nmi->module_info_length; j++) {
			BDBG_MSG(("%02x %c", nmi->module_info_byte[j], nmi->module_info_byte[j])); 
			/* BDBG_MSG(("%c", c[i])); */
		}
        if (module_info_length != len) {
            continue;
        }
        if (BKNI_Memcmp(&nmi->module_info_byte[0], (uint8_t *)name, len) == 0) {
            break;
        }
    }
    if (i < number_of_modules) {
        BDBG_MSG(("Found module id %d for %s\n", module_id, name));
        return module_id;
    }

    if ((strncmp((char *)name, "testonly", strlen("testonly") + 1)) == 0 ) {
        module_id = 0;
        BDBG_ERR(("!!!!!!Debug only!!!!!!, %s matches testonly, return module id %d", name, module_id));
        return module_id;
    }
    BDBG_ERR(("Module id for %s not found\n", name));
    hDsmcc->status->error = B_DSMCC_ERR_MODULE_NOT_FOUND;
    return -1;
}

int B_Dsmcc_GetStatus(B_Dsmcc_Status * pStatus) {
    memcpy(pStatus, &s_status, sizeof(B_Dsmcc_Status));
    return 1;
}

void B_Dsmcc_Init(B_Dsmcc_Settings * params) {
#ifndef HOST_ONLY
    b_dsmcc_lib_priv_inband_init();
#endif
}
void B_Dsmcc_Uninit(void) {
#ifndef HOST_ONLY
    b_dsmcc_lib_priv_inband_uninit();
#endif
}

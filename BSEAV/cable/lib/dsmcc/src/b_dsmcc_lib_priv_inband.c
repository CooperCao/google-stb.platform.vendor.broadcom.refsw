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
#include "b_os_lib.h"
#include "bdbg.h"
#include "bkni.h"
#include "b_dsmcc_lib.h"
#include "b_dsmcc_lib_priv.h"
#include "b_dsmcc_lib_priv_inband.h"
#include <netinet/in.h>

BDBG_MODULE(b_dsmcc_lib_priv_inband);
#define GET_PRIV_INBAND_MSG(h) ((struct b_dsmcc_lib_priv_inband * )h->priv)->msg
#define GET_PRIV_INBAND_MESSAGE_READY_EVENT(h) ((struct b_dsmcc_lib_priv_inband * )h->priv)->message_ready_event

static void * s_msg_buffer = NULL;
static unsigned int s_msg_buffer_size = MSG_BUFFER_SIZE;
/*
 * This module handles the DSMCC message filter from live inband input. It fills 
 * the function pointers (open, close, start, stop, etc) in b_dsmcc_lib.c 
 * with Nexus nexus functions. 
 */

static void message_callback(void *context, int param)
{
    BSTD_UNUSED(param);
	BKNI_SetEvent((BKNI_EventHandle)context);
}
static void message_overflow(void *context, int param)
{
	static int times = 0;
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
	times ++;
	BDBG_ERR(("\n\n message overflow \n\n"));
	/* exit(0); */
}

static void set_filter_coef(NEXUS_MessageStartSettings *p, int offset, uint8_t * val, int len) {
	int i;

	BDBG_MSG(("%s: offset %d, len %d", __FUNCTION__, offset, len));	
	for (i = 0; i < len; i++) {
		p->filter.mask[offset + i] = 0;
		p->filter.coefficient[offset + i] = val[i];
	}
}
/* 
static void set_filter_coef_uint8(NEXUS_MessageStartSettings *p, int offset, uint8_t val) {
	set_filter_coef(p, offset, &val, 1);
}
*/
static void set_filter_coef_uint16(NEXUS_MessageStartSettings *p, int offset, uint16_t val) {
	set_filter_coef(p, offset, (uint8_t *)&val, 2);
}


void b_dsmcc_lib_priv_inband_stop(B_Dsmcc_P_Handle hDsmcc) {
	NEXUS_MessageHandle msg = GET_PRIV_INBAND_MSG(hDsmcc);
    NEXUS_Message_Stop(msg); 
    return; 
}
/* Return 0 if succeed, otherwise fail. */
int b_dsmcc_lib_priv_inband_start(B_Dsmcc_P_Handle h, int module_id) {
	NEXUS_MessageStartSettings messageStartSettings;
	NEXUS_MessageHandle msg = GET_PRIV_INBAND_MSG(h);
	struct dsmcc_module_status * ms;
	struct dsmcc_module_info * mi;
	BDBG_MSG(("Enter %s\n", __FUNCTION__));
	
    ms = get_module_status_from_module_id(h, module_id);
    if (!ms) return -1;
    mi = get_module_info_from_module_id(h, module_id);
    if (!mi) return -1;

	if (!msg) goto error;
    NEXUS_Message_GetDefaultStartSettings(msg, &messageStartSettings);
    messageStartSettings.format = NEXUS_MessageFormat_ePsi;
    /* messageStartSettings.filterGroup = false; */
    messageStartSettings.pidChannel = h->settings.pidChannel;
    messageStartSettings.buffer = s_msg_buffer; /* use the app-allocated buffer. */
    BDBG_MSG(("%s: Message buffer size = 0x%lx", __FUNCTION__, s_msg_buffer_size));
    messageStartSettings.bufferSize = s_msg_buffer_size;

	/* messagestartsettings.buffer_size = 64; don't need it?*/
	
	messageStartSettings.filter.mask[0] = 0;
	messageStartSettings.filter.coefficient[0] = DSMCC_TABLE_ID_DDB;
    /* byte 2 is skipped */
    messageStartSettings.filter.mask[7] = 0;
    messageStartSettings.filter.coefficient[7] = DSMCC_PROTOCOL_DISCRIMINITOR;
    messageStartSettings.filter.mask[8] = 0;
    messageStartSettings.filter.coefficient[8] = DSMCC_TYPE;
    messageStartSettings.filter.mask[9] = 0;
    messageStartSettings.filter.coefficient[9] = ((DSMCC_MESSAGE_ID_DDB & 0xff00) >> 8);
    messageStartSettings.filter.mask[10] = 0;
    messageStartSettings.filter.coefficient[10] = (DSMCC_MESSAGE_ID_DDB & 0xff);
    /* for DDB, table_id_ext equals to module id */
    set_filter_coef_uint16(&messageStartSettings, DSMCC_SECTION_TABLE_ID_EXT_OFFSET - 1, htons(mi->module_id)); 
	
	if (NEXUS_Message_Start(msg, &messageStartSettings)) goto error;
	ms->state = STATE_FILTER_STARTED;
	ms->module_id = module_id;
    h->current_module = module_id;

	return 0;
 error: 
	BDBG_ERR(("%s error\n", __FUNCTION__));
	ms->state = STATE_INIT;
	return -1;
}

/* return >=0 if succeed, < 0 if fail */
int b_dsmcc_lib_priv_inband_open(B_Dsmcc_P_Handle hDsmcc, B_Dsmcc_Settings *pDsmccSettings)
{
    NEXUS_MessageHandle msg;
    NEXUS_MessageSettings messageSettings;
    NEXUS_MessageStartSettings messageStartSettings;
    BKNI_EventHandle message_ready_event;
    int dii_found = 0;
    struct b_dsmcc_lib_priv_inband * priv_inband;
    int ret;
    if (NULL == pDsmccSettings) return -1;

    BDBG_ENTER(B_Dsmcc_Open);
    
    /* create message filter */
    BKNI_CreateEvent(&message_ready_event);

    if (b_dsmcc_lib_priv_inband_init() < 0) {
        return -1;
    }

    NEXUS_Message_GetDefaultSettings(&messageSettings);
    messageSettings.dataReady.callback = message_callback;
    messageSettings.dataReady.context = message_ready_event;
    messageSettings.overflow.callback = message_overflow;
    messageSettings.maxContiguousMessageSize = MAX_SECTION_LEN;
    messageSettings.bufferSize = 0; /* don't have Message alloc the buffer. recommended for maximum flexibility. */
    msg = NEXUS_Message_Open(&messageSettings);
    if (NULL == msg) {
        BDBG_ERR(("Cannot open message"));
        goto Error_Message_Open;
    }

    /* start message filter */
    NEXUS_Message_GetDefaultStartSettings(msg, &messageStartSettings);
    messageStartSettings.format = NEXUS_MessageFormat_ePsi;
    /* messageStartSettings.filterGroup = false; */
    messageStartSettings.pidChannel = pDsmccSettings->pidChannel;
    messageStartSettings.buffer = s_msg_buffer; /* use the app-allocated buffer. */
    messageStartSettings.bufferSize = s_msg_buffer_size;

    /* filter the DII message */
    messageStartSettings.filter.mask[0] = 0;
    messageStartSettings.filter.coefficient[0] = DSMCC_TABLE_ID_DII_OR_DSI;
    /* byte 2 is skipped */
    messageStartSettings.filter.mask[7] = 0;
    messageStartSettings.filter.coefficient[7] = DSMCC_PROTOCOL_DISCRIMINITOR;
    messageStartSettings.filter.mask[8] = 0;
    messageStartSettings.filter.coefficient[8] = DSMCC_TYPE;
    messageStartSettings.filter.mask[9] = 0;
    messageStartSettings.filter.coefficient[9] = ((DSMCC_MESSAGE_ID_DII & 0xff00) >> 8);
    messageStartSettings.filter.mask[10] = 0;
    messageStartSettings.filter.coefficient[10] = (DSMCC_MESSAGE_ID_DII & 0xff);

    if (NEXUS_Message_Start(msg, &messageStartSettings)) goto Error_Message_Start;

    BDBG_WRN(("Waiting for DII"));
    /* parse the DII message */
    while ( !dii_found) {
        const void *buffer;
        size_t size = 0;

        if (NEXUS_Message_GetBuffer(msg, &buffer, &size)) {
            BDBG_ERR(("Failed to get message buffer"));
            goto Error_Message_GetBuffer;
        }
        BDBG_MSG(("Recv %d bytes", size));        

        if (!size) ret = BKNI_WaitForEvent(message_ready_event, DEFAULT_TIMEOUT_DII_MS); 
        if (BERR_TIMEOUT == ret) {
            hDsmcc->status->error = B_DSMCC_ERR_TIMEOUT;
            goto Error_Message_GetBuffer;
        }
        if (!size) continue;
        BKNI_Memcpy(hDsmcc->data, buffer, size);
        /* dump_data(hDsmcc->data, 80, 0); */
        parse_dsmcc_sect(hDsmcc, hDsmcc->data);
        NEXUS_Message_ReadComplete(msg, size);
        dii_found = 1;
    }
    BDBG_WRN(("DII found"));

    /* stop message filter */
    NEXUS_Message_Stop(msg);
    
    priv_inband = (struct b_dsmcc_lib_priv_inband *) BKNI_Malloc(sizeof(struct b_dsmcc_lib_priv_inband));
    if (!priv_inband) {
        BDBG_ERR(("Cannot alloc priv_inband"));
        goto Error_Message_GetBuffer;
    }
    priv_inband->msg = msg;
    priv_inband->message_ready_event = message_ready_event;
    hDsmcc->priv = priv_inband;
    hDsmcc->settings = *pDsmccSettings; 
    return 1;

Error_Message_GetBuffer:
    NEXUS_Message_Stop(msg);
Error_Message_Start:
    NEXUS_Message_Close(msg);
Error_Message_Open:
    if (hDsmcc) {
        BKNI_Free(hDsmcc);
        BKNI_DestroyEvent(message_ready_event);
    }
    return -1;

}
/* return  0 if succeed, otherwise if fail */
int b_dsmcc_lib_priv_inband_close(B_Dsmcc_P_Handle hDsmcc)
{
	NEXUS_MessageHandle msg = GET_PRIV_INBAND_MSG(hDsmcc);
	BKNI_EventHandle  message_ready_event = GET_PRIV_INBAND_MESSAGE_READY_EVENT(hDsmcc);

    BDBG_ENTER(B_Dsmcc_Close);

    if (msg) NEXUS_Message_Stop(msg);
    if (msg) NEXUS_Message_Close(msg);
    BKNI_DestroyEvent(message_ready_event);
    BKNI_Free(hDsmcc->priv);
    return 0;
}
/* return 0 if succeed, otherwise fail */
int b_dsmcc_lib_priv_inband_get_buffer(B_Dsmcc_P_Handle hDsmcc, unsigned char ** buffer, size_t * size)
{
	NEXUS_MessageHandle msg = GET_PRIV_INBAND_MSG(hDsmcc);
	BKNI_EventHandle  message_ready_event = GET_PRIV_INBAND_MESSAGE_READY_EVENT(hDsmcc);
    int timeout = (int)hDsmcc->status->inbandCarouselTimeoutUs;
    BERR_Code ret;
    /* */
    if (timeout == 0) {
        timeout = DEFAULT_TIMEOUT_MS;
    } else {
        timeout = timeout / 1000;
    }
    do {
        if (NEXUS_Message_GetBuffer(msg, (void **)buffer, size)) return -1;
        if (!*size) ret = BKNI_WaitForEvent(message_ready_event, timeout);
        if (BERR_TIMEOUT == ret) {
            hDsmcc->status->error = B_DSMCC_ERR_TIMEOUT;
            return -1;
        }
    } while (!*size);
    return 0;
}
/* return 0 if succeed, otherwise fail */
int b_dsmcc_lib_priv_inband_read_complete(B_Dsmcc_P_Handle hDsmcc, size_t size){
	NEXUS_MessageHandle msg = GET_PRIV_INBAND_MSG(hDsmcc);
    NEXUS_Message_ReadComplete(msg, size);
    return 0;
}

int b_dsmcc_lib_priv_inband_init(void) {
    char * tmp_str;
    NEXUS_MemoryAllocationSettings allocSettings;

    if (!s_msg_buffer) {
        tmp_str = getenv("MSG_BUF_SIZE");
        if (tmp_str) {
            s_msg_buffer_size = strtoul(tmp_str, NULL, 0);
            BDBG_MSG(("Message buffer size from MSG_BUF_SIZE = 0x%lx", s_msg_buffer_size));
            if (s_msg_buffer_size < MSG_BUFFER_SIZE || s_msg_buffer_size > (512*1024) ) {
                BDBG_ERR(("MSG_BUF_SIZE out of range [0x%lx - 0x%lx], reset to ",
                          MSG_BUFFER_SIZE, (512*1024), MSG_BUFFER_SIZE));
                s_msg_buffer_size = MSG_BUFFER_SIZE;
            } else {
                /* must aligned to section len */
                s_msg_buffer_size = (s_msg_buffer_size / MAX_SECTION_LEN) * MAX_SECTION_LEN;            
            }
        } else {
            s_msg_buffer_size = MSG_BUFFER_SIZE;
        }
        /* use app-allocated message buffer as recommended in Nexus_Usage.pdf. */
        NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
        allocSettings.alignment = 1024;
        BDBG_MSG(("%s: Message buffer size = 0x%lx", __FUNCTION__, s_msg_buffer_size));
        NEXUS_Memory_Allocate(s_msg_buffer_size, &allocSettings, &s_msg_buffer);
        if (!s_msg_buffer) {
            BDBG_ERR(("NEXUS_Memory_Allocate error"));
            return -1;
        }
    } else {
        BDBG_MSG(("Use pre-allocated message buffer %p, size 0x%lx", s_msg_buffer, s_msg_buffer_size));
    }
    return 1;
}
int b_dsmcc_lib_priv_inband_uninit(void) {
    if (s_msg_buffer) {
            NEXUS_Memory_Free(s_msg_buffer);        
            s_msg_buffer = NULL;
            s_msg_buffer_size = 0;
    }
    return 1;
}

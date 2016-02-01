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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#ifdef HOST_ONLY
#include "b_dsmcc_lib_host.h"
#else
#include "b_os_lib.h"
#include "bdbg.h"
#include "bkni.h"
#endif
#include "b_dsmcc_lib.h"
#include "b_dsmcc_lib_priv.h"
#include "b_dsmcc_lib_priv_dsg.h"
#include <sched.h>
#include <semaphore.h>
#include <sys/time.h>

BDBG_MODULE(b_dsmcc_lib_priv_dsg);

#define DSG_PKT_SIZE (1518)

#define DSG_BUF_SIZE (200*DSG_PKT_SIZE)

#define ETH_TAIL_SIZE 4 /* 4 bytes eth CRC */
#define ETH_HDR_SIZE 14 
#define IP_HDR_SIZE 20
#define UDP_HDR_SIZE 8
#define MPEG_HDR_SIZE 2
#define DSG_HDR_SIZE (ETH_HDR_SIZE + IP_HDR_SIZE +UDP_HDR_SIZE + MPEG_HDR_SIZE) /* eth 14, ip 20, udp 8, mpeg 2 */
#define OFFSET_IP_HEADER_TOTAL_LEN 16
#define OFFSET_DSG_CAROUSEL_HEADER_PID 42
#define GET_DSG_PKT_LEN(buf) (((buf[OFFSET_IP_HEADER_TOTAL_LEN] << 8) | buf[OFFSET_IP_HEADER_TOTAL_LEN + 1]) + 14)

#define SECT_FILTER_SIZE 16
struct sect_filter {
    unsigned char mask[SECT_FILTER_SIZE];
    unsigned char coefficient[SECT_FILTER_SIZE];    
} s_filter;

struct b_dsmcc_lib_priv_dsg
{
    unsigned short pid;
    /* for unfiltered TS packet, sync byte found */

    /* element in the fifo is fixed to be DSG_PKT_SIZE  */
    unsigned char * fifo_base; /* start address */
    unsigned char * fifo_wrptr; /* write pointer, updated by download function */
    unsigned char * fifo_rdptr; /* read pointer, updated when both expand and authentication consumed the data */
    int fifo_size; /* size of the fifo */

    /* for filtered data*/
    unsigned char * buf_filtered;
    int bytes_filtered; /* size of the data in buf_filtered*/ 
    int started ;

    /* */
    sem_t pkt_sem;	/* semaphore set to 1 if there's packet in the buffer */

};



static B_Dsmcc_P_Handle s_Dsmcc_Handle_dsg = NULL;

static int sect_filter(struct b_dsmcc_lib_priv_dsg * priv_dsg, int timeout_s);

int b_dsmcc_lib_priv_dsg_pkt_recv_callback (unsigned char * buf, int len) ;


static int fifo_get_depth( struct b_dsmcc_lib_priv_dsg * h) {
    int depth = (((h)->fifo_wrptr >= (h)->fifo_rdptr)  ?         
            ((h)->fifo_wrptr - (h)->fifo_rdptr) : ((h)->fifo_wrptr + (h)->fifo_size - (h)->fifo_rdptr));
    BDBG_MSG(("FIFO depth %d", depth)); 
    return depth;
}
static int fifo_is_full( struct b_dsmcc_lib_priv_dsg * h) {
    int depth = fifo_get_depth(h);
    if (depth > (h->fifo_size - 1)) {
        BDBG_ERR(("FIFO overflow, depth %d, max %d", depth, h->fifo_size - 1));
    } else  if (depth < 0) {
        BDBG_ERR(("FIFO underflow, depth %d, min 0 ", depth));
    } else if (depth == h->fifo_size - 1) {
        BDBG_MSG(("FIFO full %d", depth));
        return 1;
    }
    return 0;
}


void b_dsmcc_lib_priv_dsg_stop(B_Dsmcc_P_Handle hDsmcc) {
    struct b_dsmcc_lib_priv_dsg * priv_dsg= (struct b_dsmcc_lib_priv_dsg *) hDsmcc->priv;
    priv_dsg->fifo_rdptr = priv_dsg->fifo_base;
    priv_dsg->fifo_wrptr = priv_dsg->fifo_base;
    priv_dsg->bytes_filtered = 0;
    sem_init(&priv_dsg->pkt_sem, 0, 0);
    priv_dsg->started  = 0;
    return; 
}
/* Return 0 if succeed, otherwise fail. */
int b_dsmcc_lib_priv_dsg_start(B_Dsmcc_P_Handle hDsmcc, int module_id) {
    struct b_dsmcc_lib_priv_dsg * priv_dsg= (struct b_dsmcc_lib_priv_dsg *) hDsmcc->priv;

    BSTD_UNUSED(module_id);
    priv_dsg->fifo_rdptr = priv_dsg->fifo_base;
    priv_dsg->fifo_wrptr = priv_dsg->fifo_base;
    priv_dsg->bytes_filtered = 0;
    sem_init(&priv_dsg->pkt_sem, 0, 0);
    priv_dsg->started = 1;
    
	return 0;
}
/* 
 * Before calling this function, make sure buf[0] is the start of a TS packet. 
 * Return the first DII message in the buffer.
 * return section length if succeed, < 0 if fail 
 */
static int find_dii(struct b_dsmcc_lib_priv_dsg * priv_dsg, int timeout_s) {
 
    memset(&s_filter, 0xff, sizeof(struct sect_filter));
    /* filter the DII message */
    s_filter.mask[0] = 0;
    s_filter.coefficient[0] = DSMCC_TABLE_ID_DII_OR_DSI;
    /* byte 2 is skipped */
    s_filter.mask[8] = 0;
    s_filter.coefficient[8] = DSMCC_PROTOCOL_DISCRIMINITOR;
    s_filter.mask[9] = 0;
    s_filter.coefficient[9] = DSMCC_TYPE;
    s_filter.mask[10] = 0;
    s_filter.coefficient[10] = ((DSMCC_MESSAGE_ID_DII & 0xff00) >> 8);
    s_filter.mask[11] = 0;
    s_filter.coefficient[11] = (DSMCC_MESSAGE_ID_DII & 0xff);
    return sect_filter(priv_dsg, timeout_s);
}

/*
 * If the packet has been accepted by dsmcc library, return the packet size ( > 0).
 * If the packet was rejected (buffer full, pid not match), return 0, caller shoud drop the packet
 * If fail, return < 0.
 */
int b_dsmcc_lib_priv_dsg_pkt_recv_callback (unsigned char * buf, int len) {
    B_Dsmcc_P_Handle hDsmcc;
    struct b_dsmcc_lib_priv_dsg * priv_dsg;
    unsigned short plen;
    unsigned short pid;

    if (!s_Dsmcc_Handle_dsg) return 0;
    hDsmcc = s_Dsmcc_Handle_dsg;
    priv_dsg= (struct b_dsmcc_lib_priv_dsg *)hDsmcc->priv;
    if (!priv_dsg->started) return 0;

    plen = GET_DSG_PKT_LEN(buf) ;
    if (plen != (unsigned short) len) {
        BDBG_ERR(("Invalid data, pkt len %d does not match pkt header length %d", len, plen));
        return -1;
    }
#if 0
    /* CDL spec des not require pid check */
    pid = ((buf[OFFSET_DSG_CAROUSEL_HEADER_PID] & 0x1f) << 8) | buf[OFFSET_DSG_CAROUSEL_HEADER_PID + 1];
    if (priv_dsg->pid != pid) {
        BDBG_ERR(("Invalid data, pid %d in packet not match pid %d", pid, priv_dsg->pid));
        return -1;
    }
#endif
    if (fifo_is_full(priv_dsg)) {
        static int count = 0;
        if (count < 10) {
            BDBG_MSG(("Buffer full"));
            count++;
        }
        return 0;
    }
    if (priv_dsg->fifo_wrptr >= (priv_dsg->fifo_base + priv_dsg->fifo_size)) {
        priv_dsg->fifo_wrptr = priv_dsg->fifo_base;
    }
    memcpy(priv_dsg->fifo_wrptr, buf, DSG_PKT_SIZE); 

    BDBG_MSG(("cb: len %d pad %d", len, DSG_PKT_SIZE - len)); 
    
    BDBG_MSG(("cb: %02x-%02x %02x-%02x", buf[42],buf[43],buf[44],buf[45]));
    
    priv_dsg->fifo_wrptr += DSG_PKT_SIZE;
    sem_post(&priv_dsg->pkt_sem);
    return len;
}

/* return >=0 if succeed, < 0 if fail */
int b_dsmcc_lib_priv_dsg_open(B_Dsmcc_P_Handle hDsmcc, B_Dsmcc_Settings *pDsmccSettings)
{
    unsigned char * buf = NULL;
    unsigned char * buf_filtered = NULL;
    struct b_dsmcc_lib_priv_dsg * priv_dsg= NULL;
    int offset = 0;
    int dii_found = 0;

    buf = (unsigned char *) BKNI_Malloc(DSG_BUF_SIZE);
    if (!buf) {
        BDBG_ERR(("Cannot alloc buf"));
        goto err;
    }
    buf_filtered = (unsigned char *) BKNI_Malloc(DSG_PKT_SIZE);
    if (!buf_filtered) {
        BDBG_ERR(("Cannot alloc buf_filtered"));
        goto err;
    }

    priv_dsg = (struct b_dsmcc_lib_priv_dsg *) BKNI_Malloc(sizeof(struct b_dsmcc_lib_priv_dsg));
    if (!priv_dsg) {
        BDBG_ERR(("Cannot alloc priv_inband"));
        goto err;
    }
    priv_dsg->pid = pDsmccSettings->pid;
    BDBG_MSG(("priv_dsg->pid %d", priv_dsg->pid));
    priv_dsg->fifo_base = buf;
    priv_dsg->fifo_size = DSG_BUF_SIZE;
    priv_dsg->fifo_wrptr = priv_dsg->fifo_base;
    priv_dsg->fifo_rdptr = priv_dsg->fifo_base;

    priv_dsg->buf_filtered = buf_filtered;

	if (sem_init(&priv_dsg->pkt_sem, 0, 0) != 0){
		BDBG_ERR(("%s: sem_init failed", __FUNCTION__));
		goto err;
	}

    s_Dsmcc_Handle_dsg = hDsmcc;
    hDsmcc->priv = priv_dsg;
    hDsmcc->settings = *pDsmccSettings; 

    priv_dsg->started = 1;
    /* filter DII packet; */
    BDBG_MSG(("Looking for DII"));
    if ((offset = find_dii(priv_dsg, DEFAULT_TIMEOUT_DII_MS)) >= 0) {
        BDBG_ERR(("Found DII\n"));
        BKNI_Memcpy(hDsmcc->data, priv_dsg->buf_filtered, priv_dsg->bytes_filtered);
        parse_dsmcc_sect(hDsmcc, hDsmcc->data);
        dii_found = 1;
    } else {
        hDsmcc->status->error = B_DSMCC_ERR_TIMEOUT;
        BDBG_ERR(("DII not found\n"));
        goto err;
    }
    priv_dsg->started = 0;


    return 1;
 err:
    priv_dsg->started = 0;
    if (!buf) BKNI_Free(buf);
    if (!priv_dsg) BKNI_Free(priv_dsg);
    s_Dsmcc_Handle_dsg = NULL;
    return -1;
}
/* return  0 if succeed, otherwise if fail */
int b_dsmcc_lib_priv_dsg_close(B_Dsmcc_P_Handle hDsmcc)
{
    struct b_dsmcc_lib_priv_dsg * priv_dsg= (struct b_dsmcc_lib_priv_dsg *) hDsmcc->priv;
    s_Dsmcc_Handle_dsg = NULL;
    if (!priv_dsg) return 0;
    if (priv_dsg->fifo_base) BKNI_Free(priv_dsg->fifo_base);
    if (priv_dsg->buf_filtered) BKNI_Free(priv_dsg->buf_filtered);
    BKNI_Free(priv_dsg);
    hDsmcc->priv = NULL;

    return 0;
}

/* 
 * Before calling this function, make sure buf[0] is the start of a DSG packet. 
 * Return until one complete section has been filtered. 
 * the filter has been setup in s_filter[] before calling this function.
 * return section length if succeed, < 0 if fail 
 */
static int sect_filter(struct b_dsmcc_lib_priv_dsg * priv_dsg, int timeout_s) {
    int j;
    unsigned char *tbuf, *buf;
    int msg_len = 0;
    int section_started;
    int section_len;
    struct timeval start_time, cur_time;
    unsigned int time_passed_s = 0;
    
    section_started = 0;
    section_len = -1;

    gettimeofday(&start_time, NULL);
    time_passed_s = 0;

    do {
        sem_wait(&priv_dsg->pkt_sem);
        if (!fifo_get_depth(priv_dsg)) {
            BDBG_ERR(("Error, fifo should not be empty"));
            continue;
        }
        if (priv_dsg->fifo_rdptr >= (priv_dsg->fifo_base + priv_dsg->fifo_size)) {
            priv_dsg->fifo_rdptr = priv_dsg->fifo_base;
        }
        buf = priv_dsg->fifo_rdptr;
        tbuf = buf + DSG_HDR_SIZE;
        /*

        BDBG_ERR(("%02x-%02x-%02x-%02x", tbuf[16],tbuf[17],tbuf[18],tbuf[19]));
        */
        BDBG_MSG(("sf: %02x-%02x %02x-%02x", buf[42],buf[43],buf[44],buf[45]));
        msg_len = GET_DSG_PKT_LEN(buf) - (IP_HDR_SIZE + UDP_HDR_SIZE + MPEG_HDR_SIZE + 
                                          ETH_TAIL_SIZE + ETH_HDR_SIZE);
        BDBG_MSG(("msg_len %d pkt len %d", msg_len, GET_DSG_PKT_LEN(buf)));
        section_started = 1;
        for (j = 0; j < SECT_FILTER_SIZE ; j++) {
            if (s_filter.mask[j] == 0xff) continue;
            if ( (tbuf[j] &(~s_filter.mask[j])) != (s_filter.coefficient[j] & (~s_filter.mask[j]))) {
                section_started = 0;
                break;
            }
        }
        priv_dsg->fifo_rdptr += DSG_PKT_SIZE;

        if (section_started) {
            BDBG_MSG(("Found section (%d bytes)\n", msg_len));
            
            BDBG_MSG(("%02x-%02x-%02x-%02x", tbuf[0],tbuf[1],tbuf[2],tbuf[3]));
            BDBG_MSG(("%02x-%02x-%02x-%02x", tbuf[4],tbuf[5],tbuf[6],tbuf[7]));
            BDBG_MSG(("%02x-%02x-%02x-%02x", tbuf[8],tbuf[9],tbuf[10],tbuf[11]));
            
            BKNI_Memcpy(priv_dsg->buf_filtered, tbuf, msg_len);
            priv_dsg->bytes_filtered = msg_len;
            section_started = 1;
            section_len = msg_len;
            break;
        } else {
            sched_yield();
        }
        gettimeofday(&cur_time, NULL);
        time_passed_s = cur_time.tv_sec - start_time.tv_sec;
    } while (time_passed_s < timeout_s);
    return section_len;
}

/* 
 * Before calling this function, make sure buf[0] is the start of a TS packet. 
 * Return until one complete DDB message has been filtered.
 * return section length if succeed, < 0 if fail 
 */
static int find_ddb(struct b_dsmcc_lib_priv_dsg * priv_dsg, unsigned short module_id, int timeout_s) {
    

    memset(&s_filter, 0xff, sizeof(struct sect_filter));

	s_filter.mask[0] = 0;
	s_filter.coefficient[0] = DSMCC_TABLE_ID_DDB;
    /* byte 2 is skipped */
    s_filter.mask[8] = 0;
    s_filter.coefficient[8] = DSMCC_PROTOCOL_DISCRIMINITOR;
    s_filter.mask[9] = 0;
    s_filter.coefficient[9] = DSMCC_TYPE;
    s_filter.mask[10] = 0;
    s_filter.coefficient[10] = ((DSMCC_MESSAGE_ID_DDB & 0xff00) >> 8);
    s_filter.mask[11] = 0;
    s_filter.coefficient[11] = (DSMCC_MESSAGE_ID_DDB & 0xff);

    /* for DDB, table_id_ext equals to module id */
    s_filter.mask[3] = 0;
    s_filter.coefficient[3] = (module_id &0xff00) >> 8;
    s_filter.mask[4] = 0;
    s_filter.coefficient[4] = (module_id &0xff);

    return sect_filter(priv_dsg, timeout_s);
}

/*
 * return 1 DDB message
 * return 0 if succeed, otherwise fail 
 */
int b_dsmcc_lib_priv_dsg_get_buffer(B_Dsmcc_P_Handle hDsmcc, unsigned char ** buffer, size_t * size)
{
    struct b_dsmcc_lib_priv_dsg * priv_dsg= (struct b_dsmcc_lib_priv_dsg *) hDsmcc->priv;
    int offset = 0;
    int timeout_s = (int)hDsmcc->status->inbandCarouselTimeoutUs;
    /* */
    if (timeout_s == 0) {
        timeout_s = DEFAULT_TIMEOUT_MS/1000;
    } else {
        timeout_s = timeout_s / 1000000;
    }

    /* filter DDB packet; */
    if ((offset = find_ddb(priv_dsg, hDsmcc->current_module, timeout_s)) >= 0) {
        *buffer = priv_dsg->buf_filtered;
        *size = priv_dsg->bytes_filtered;
        
        return 0;
    } else {
        hDsmcc->status->error = B_DSMCC_ERR_TIMEOUT;
        BDBG_ERR(("DDB not found"));
    }
    return -1;
}
/* return 0 if succeed, otherwise fail */
int b_dsmcc_lib_priv_dsg_read_complete(B_Dsmcc_P_Handle hDsmcc, size_t size){
    struct b_dsmcc_lib_priv_dsg * priv_dsg= (struct b_dsmcc_lib_priv_dsg *) hDsmcc->priv;
    
    /* 
     * assuming the filter is done per section,
     * to simplify, empty the whole sections
     */
    /* BDBG_ERR(("size %d", size)); */
    priv_dsg->bytes_filtered = 0;
    return 0;
}

void * b_dsmcc_lib_priv_dsg_get_tunnel_handler(void) {
    return (void *)b_dsmcc_lib_priv_dsg_pkt_recv_callback;
}

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
#include "b_dsmcc_lib_priv_file.h"

BDBG_MODULE(b_dsmcc_lib_priv_file);

#define TS_SIZE (188)
#define TS_PL_SIZE (TS_SIZE - 4) /* Transport payload size */

#define BUF_SIZE (64*4096)
#define TS_BUF_SIZE (2*BUF_SIZE)
#define TS_BUF_SIZE_ALIGNED (((TS_BUF_SIZE)/TS_SIZE) * TS_SIZE)

#define SECT_FILTER_SIZE 16
struct sect_filter {
    unsigned char mask[SECT_FILTER_SIZE];
    unsigned char coefficient[SECT_FILTER_SIZE];    
} s_filter;

static int get_first_ts_pkt(struct b_dsmcc_lib_priv_file * priv_file) ;
static int pidfilter(unsigned char *buf, unsigned short pid);
static int sect_filter(struct b_dsmcc_lib_priv_file * priv_file);

void b_dsmcc_lib_priv_file_stop(B_Dsmcc_P_Handle hDsmcc) {
    struct b_dsmcc_lib_priv_file * priv_file= (struct b_dsmcc_lib_priv_file *) hDsmcc->priv;
    int fd = priv_file->fd;
    lseek(fd, 0, SEEK_END);
    return; 
}
/* Return 0 if succeed, otherwise fail. */
int b_dsmcc_lib_priv_file_start(B_Dsmcc_P_Handle hDsmcc, int module_id) {
    struct b_dsmcc_lib_priv_file * priv_file= (struct b_dsmcc_lib_priv_file *) hDsmcc->priv;
    int ret;

    BSTD_UNUSED(module_id);
    priv_file->bytes_read = 0;
    priv_file->bytes_filtered = 0;
    priv_file->bytes_complete = 0;
    priv_file->loop = 0;

    /* find 1st TS packet */
    ret = get_first_ts_pkt(priv_file);
    if (ret < 0) {
        BDBG_MSG(("Read TS stream error"));
        return -1;
    }
	return 0;
}

/* 
 * return >= 0 if succeed, < 0 if error, the first ts packet is returned at buf[0]
 * lenght is returned in *len
 */
static int get_first_ts_pkt(struct b_dsmcc_lib_priv_file * priv_file) {
    int fd = priv_file->fd;
    unsigned char * buf = priv_file->buf;
    int len;
    int i, bytes_read;
    i = 0; 
    lseek(fd, 0, SEEK_SET);
    len = TS_BUF_SIZE;
    priv_file->bytes_read = 0;
    priv_file->offset = 0;
    while ((bytes_read = read(fd, buf, len)) >= 0) {
        BDBG_ERR(("lseek %ld, bytes_read %d\n", lseek(priv_file->fd, 0, SEEK_CUR), bytes_read));
        if (bytes_read == 0 || (bytes_read < (TS_SIZE * 2))) {
            BDBG_ERR(("Read reached end of file \n"));            
            return 0;
        }
        /* look for the first sync byte */
        for (i = 0; i < (bytes_read - TS_SIZE); i++) {
            if (buf[i] == 0x47 && (buf[i + TS_SIZE] == 0x47)) { 
                /* found sync byte */
                if (pidfilter(&buf[i], priv_file->pid) == 0) continue; 

                /* found the first pkt, align buffer */
                BDBG_ERR(("Found first pkt\n"));
                lseek(fd, i - bytes_read, SEEK_CUR);
                len = ((bytes_read - i)/TS_SIZE) * TS_SIZE;
                BDBG_ERR(("offset %lx len %d\n", lseek(fd, 0, SEEK_CUR), len));
                read(fd, buf, len);
                BDBG_ERR(("%02x-%02x-%02x-%02x\n", buf[0],buf[1],buf[2],buf[3]));
                priv_file->bytes_read = len;
                priv_file->offset = 0;
                return len;
            }
        }
    }
    if (bytes_read <0) {
        BDBG_ERR(("Read error \n"));
    } 
    return -1;
}
/* 
 * return >= 0 if succeed, < 0 if error, 
 * The TS packet is returned starting at buf[0]
 */
static int get_ts_pkts(struct b_dsmcc_lib_priv_file * priv_file) {
    int fd = priv_file->fd;
    unsigned char * buf = priv_file->buf;
    int len;
    int i, bytes_read;

    i = 0; 
    /* sync byte already found, read integer TS bytes*/
    len = TS_BUF_SIZE_ALIGNED;

    bytes_read = read(fd, buf, len);
    if (bytes_read <0) {
        BDBG_ERR(("Read error \n"));
        return -1;
    } else if (bytes_read == 0 || (bytes_read < (TS_SIZE * 2))) {
        BDBG_ERR(("Read reached end of file \n"));            
        priv_file->bytes_read = bytes_read;
        return 0;
    }
    BDBG_ERR(("lseek %ld, bytes_read %d\n", lseek(priv_file->fd, 0, SEEK_CUR), bytes_read));
    /* check the sync byte */
#if 0
    BDBG_ERR(("%02x-%02x-%02x-%02x", buf[0],buf[1],buf[2],buf[3]));
    BDBG_ERR(("%02x-%02x-%02x-%02x", buf[TS_SIZE],buf[TS_SIZE + 1],buf[TS_SIZE +2],buf[TS_SIZE +3]));
#endif
    for (i = 0; i < (bytes_read - TS_SIZE); i+=TS_SIZE) {
        if (!(buf[i] == 0x47 && (buf[i + TS_SIZE] == 0x47))) { 
            BDBG_ERR(("Missing sync byte i =%d\n", i));
            return -1;
        }
    }
    priv_file->bytes_read = bytes_read;
    priv_file->offset = 0;
    return bytes_read;
}
/* return > 0 if matches, 0 if not match */
static int pidfilter(unsigned char *buf, unsigned short pid) {
    unsigned short ts_pid = ((buf[1] & 0x1f) << 8) | buf[2];
    /*
    BDBG_MSG(("ts_pid 0x%x, ntohs(ts_pid) 0x%x, pid 0x%x",ts_pid, ntohs(ts_pid), pid)); 
     ts_pid = ntohs(ts_pid); 
    */
    return (ts_pid == pid)? 1 : 0;
}
/* 
 * Before calling this function, make sure buf[0] is the start of a TS packet. 
 * Return the first DII message in the buffer.
 * return section length if succeed, < 0 if fail 
 */
static int find_dii(struct b_dsmcc_lib_priv_file * priv_file) {
 
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
    return sect_filter(priv_file);
}
/* return >=0 if succeed, < 0 if fail */
int b_dsmcc_lib_priv_file_open(B_Dsmcc_P_Handle hDsmcc, B_Dsmcc_Settings *pDsmccSettings)
{
    int fd = -1;
    unsigned char * buf = NULL;
    unsigned char * buf_filtered = NULL;
    struct b_dsmcc_lib_priv_file * priv_file= NULL;
    int bytes_read = 0, offset = 0;
    int ret = -1;
    int dii_found = 0;

    if ((fd = open(pDsmccSettings->inputFile, O_RDONLY)) < 0) {
        BDBG_ERR(("cannot open %s\n", pDsmccSettings->inputFile));
        return -1;
    }
    buf = (unsigned char *) BKNI_Malloc(TS_BUF_SIZE);
    if (!buf) {
        BDBG_ERR(("Cannot alloc buf"));
        goto err;
    }
    buf_filtered = (unsigned char *) BKNI_Malloc(TS_BUF_SIZE);
    if (!buf_filtered) {
        BDBG_ERR(("Cannot alloc buf_filtered"));
        goto err;
    }

    priv_file = (struct b_dsmcc_lib_priv_file *) BKNI_Malloc(sizeof(struct b_dsmcc_lib_priv_file));
    if (!priv_file) {
        BDBG_ERR(("Cannot alloc priv_inband"));
        goto err;
    }
    priv_file->fd = fd;
    priv_file->pid = pDsmccSettings->pid;
    priv_file->buf = buf;
    priv_file->buf_filtered = buf_filtered;
    priv_file->bytes_read = 0;

    /* find 1st TS packet */
    offset = -1;
    bytes_read = TS_BUF_SIZE;
    ret = get_first_ts_pkt(priv_file);
    if (ret < 0) {
        BDBG_MSG(("Read TS stream error"));
        return -1;
    }
    bytes_read = priv_file->bytes_read;

    /* filter DII packet; */
    do {
        if ((offset = find_dii(priv_file)) >= 0) {
            static int abs_offset = 0;
            abs_offset = lseek(fd, -bytes_read, SEEK_CUR);
            abs_offset += offset;
            BDBG_ERR(("Found DII at %d\n", abs_offset));
            BKNI_Memcpy(hDsmcc->data, buf_filtered, priv_file->bytes_filtered);
            parse_dsmcc_sect(hDsmcc, hDsmcc->data);
            dii_found = 1;
            break;
        }
        ret = get_ts_pkts(priv_file);
        if (ret < 0) {
            BDBG_MSG(("Read TS stream error"));
            return -1;
        }
    } while (bytes_read > 0);
    if (dii_found == 0) {
        BDBG_ERR(("DII not found\n"));
        goto err;
    }
    hDsmcc->priv = priv_file;
    hDsmcc->settings = *pDsmccSettings; 
    return 1;
 err:
    if (fd == -1) close(fd);
    if (!buf) BKNI_Free(buf);
    if (!priv_file) BKNI_Free(priv_file);
    return -1;
}
/* return  0 if succeed, otherwise if fail */
int b_dsmcc_lib_priv_file_close(B_Dsmcc_P_Handle hDsmcc)
{
    struct b_dsmcc_lib_priv_file * priv_file= (struct b_dsmcc_lib_priv_file *) hDsmcc->priv;
    if (!priv_file) return 0;
    if (priv_file->buf) BKNI_Free(priv_file->buf);
    if (priv_file->buf_filtered) BKNI_Free(priv_file->buf_filtered);
    if (priv_file->fd >= 0) close(priv_file->fd);
    BKNI_Free(priv_file);
    hDsmcc->priv = NULL;
    return 0;
}


/* 
 * Before calling this function, make sure buf[0] is the start of a TS packet. 
 * Return until one complete section has been filtered. 
 * the filter has been setup in s_filter[] before calling this function.
 * return section length if succeed, < 0 if fail 
 */
static int sect_filter(struct b_dsmcc_lib_priv_file * priv_file) {
    unsigned char * buf = priv_file->buf;
    unsigned char * buf_filtered = priv_file->buf_filtered;
    int bytes_read = priv_file->bytes_read;
    int bytes_filtered = 0;
    int offset = priv_file->offset;
    int i,j;
    unsigned char *tbuf;
    unsigned char pointer_field = 0;
    int ret = -1;
    int msg_len = 0;
    int section_started;
    int section_ended;
    int section_len;
    
    section_started = 0;
    section_ended = 0;
    section_len = -1;
    priv_file->loop = 0;

    do {
        for (i = offset; i <= (bytes_read - TS_SIZE); i+=TS_SIZE) {
            /* pid filter */
            if (pidfilter((buf + i), priv_file->pid) == 0) continue; 
            tbuf = buf + i + 4;

            /* find the start of section */
            if (section_started == 0) {
                if (!(buf[i + 1] & 0x40)) continue; /* PUSI not detected */

                if ((buf[i + 3] & 0x30) == 0x10) {
                    pointer_field = tbuf[0];
                    tbuf++;
                    tbuf += pointer_field; /* skip pointer field */ 
                } else {
                    BDBG_ERR(("Start of section can not have adpation field 0x%2x", buf[i+3] & 0x30));
                    return -1;
                }
                section_len = ((tbuf[1] & 0x0f) << 8) | tbuf[2];
                /* BDBG_ERR(("section len %d", section_len)); */
                msg_len = (section_len > (TS_PL_SIZE - 1))? (TS_PL_SIZE - 1) : section_len; /* exclude pointer field byte */

                section_started = 1;
                for (j = 0; j < SECT_FILTER_SIZE ; j++) {
                    if (s_filter.mask[j] == 0xff) continue;
                    if ( (tbuf[j] &(~s_filter.mask[j])) != (s_filter.coefficient[j] & (~s_filter.mask[j]))) {
                        section_started = 0;
                        break;
                    }
                }
                if (section_started) {
                    offset = tbuf - buf;
                    BDBG_ERR(("Found section (%d bytes)\n", msg_len));
                    /*
                    BDBG_ERR(("%02x-%02x-%02x-%02x", tbuf[0],tbuf[1],tbuf[2],tbuf[3]));
                    BDBG_ERR(("%02x-%02x-%02x-%02x", tbuf[4],tbuf[5],tbuf[6],tbuf[7]));
                    BDBG_ERR(("%02x-%02x-%02x-%02x", tbuf[8],tbuf[9],tbuf[10],tbuf[11]));
                    */
                    BKNI_Memcpy(buf_filtered + bytes_filtered, tbuf, msg_len);
                    bytes_filtered += msg_len;
                    section_started = 1;
                }
            } else {
                /* new PUSI indicates a new section, return current buffer */
                if ((buf[i + 1] & 0x40)) { 
                    priv_file->bytes_filtered = bytes_filtered;
                    priv_file->offset = i;
                    return 0;
                }
                /* continue current section */
                msg_len = TS_PL_SIZE; 
                /*
                BDBG_ERR(("%d bytes at %d, lseek %d\n", msg_len, bytes_filtered, lseek(priv_file->fd, 0, SEEK_CUR)));
                BDBG_ERR(("%02x-%02x-%02x-%02x", tbuf[0],tbuf[1],tbuf[2],tbuf[3]));
                */
                BKNI_Memcpy(buf_filtered + bytes_filtered, tbuf, msg_len);
                bytes_filtered += msg_len;
                
            }
        }
        ret = get_ts_pkts(priv_file);
        if (ret < 0) {
            BDBG_MSG(("Read TS stream error"));
            return -1;
        }
        offset = priv_file->offset;
        bytes_read = priv_file->bytes_read;

        if (ret == 0) {
            BDBG_MSG(("End of file reached, loop %d", priv_file->loop));
            priv_file->loop++;
            if (priv_file->loop >= 2) break;
            ret = get_first_ts_pkt(priv_file);
            if (ret < 0) {
                BDBG_MSG(("Read TS stream error"));
                return -1;
            }

        }
        bytes_read = priv_file->bytes_read;
    } while (priv_file->loop < 2);
    return section_len;
}

/* 
 * Before calling this function, make sure buf[0] is the start of a TS packet. 
 * Return until one complete DDB message has been filtered.
 * return section length if succeed, < 0 if fail 
 */
static int find_ddb(struct b_dsmcc_lib_priv_file * priv_file, unsigned short module_id) {
    
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

    return sect_filter(priv_file);
}

/*
 * return 1 DDB message
 * return 0 if succeed, otherwise fail 
 */
int b_dsmcc_lib_priv_file_get_buffer(B_Dsmcc_P_Handle hDsmcc, unsigned char ** buffer, size_t * size)
{
    struct b_dsmcc_lib_priv_file * priv_file= (struct b_dsmcc_lib_priv_file *) hDsmcc->priv;
    int offset = 0;
    /* filter DDB packet; */
    if ((offset = find_ddb(priv_file, hDsmcc->current_module)) >= 0) {
        *buffer = priv_file->buf_filtered;
        *size = priv_file->bytes_filtered;
        
        return 0;
    }
    BDBG_ERR(("DDB not found"));
    return -1;
}
/* return 0 if succeed, otherwise fail */
int b_dsmcc_lib_priv_file_read_complete(B_Dsmcc_P_Handle hDsmcc, size_t size){
    struct b_dsmcc_lib_priv_file * priv_file= (struct b_dsmcc_lib_priv_file *) hDsmcc->priv;
    
    /* 
     * assuming the filter is done per section,
     * to simplify, empty the whole sections
     */
    BDBG_ERR(("size %d", size));
    priv_file->bytes_filtered = 0;
    return 0;
}


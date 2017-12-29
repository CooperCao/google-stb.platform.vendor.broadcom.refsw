/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 **************************************************************************/
#include "bstd.h"
#include "bmmt_utils.h"
#include "bkni.h"
#include "bmedia_util.h"

/* strtol */
#include <stdlib.h>

BDBG_MODULE(bmmt_utils);

#define BTLV_NTP_TIME_SCALE  (((int64_t)1) << 32)
#define BTLV_NTP_SHORT_TIME_SCALE  (((int32_t)1) << 16)


btlv_ntp_time btplv_ntp_add_offset(btlv_ntp_time base, uint32_t timescale, int offset)
{
    int64_t ntp_offset;

    ntp_offset = (offset *  BTLV_NTP_TIME_SCALE) / (long)timescale;
    BDBG_MSG(("btplv_ntp_add_offset: %d/%u -> %ldmsec", offset, (unsigned)timescale, (long)((ntp_offset*1000)/BTLV_NTP_TIME_SCALE)));
    return base + ntp_offset;
}

uint32_t btlv_ntp_45khz(btlv_ntp_time base, btlv_ntp_time time)
{
    uint64_t diff_ntp;
    uint64_t diff_45khz;
    const unsigned factor = 8;

    BDBG_CASSERT(45000 % 8 == 0);
    BDBG_CASSERT(BTLV_NTP_TIME_SCALE % 8 == 0);

    diff_ntp = time - base;
    diff_45khz = (diff_ntp * (45000 / factor)) / (BTLV_NTP_TIME_SCALE/ factor);

    return diff_45khz & 0xFFFFFFFFul;
}

btlv_ntp_time btplv_ntp_time_init(unsigned sec, unsigned msec)
{
    uint64_t ntp_time;
    ntp_time = (BTLV_NTP_TIME_SCALE * sec) + (msec * BTLV_NTP_TIME_SCALE)/1000;
    return ntp_time;
}

static btlv_ntp_short_time btplv_ntp_to_short(btlv_ntp_time full)
{
    uint32_t _short  = full / (BTLV_NTP_TIME_SCALE/BTLV_NTP_SHORT_TIME_SCALE);
    return _short;
}

btlv_ntp_time btplv_ntp_merge(btlv_ntp_time full, btlv_ntp_short_time _short)
{
    uint32_t full_seconds = full / BTLV_NTP_TIME_SCALE;
    uint32_t _short_seconds = _short / BTLV_NTP_SHORT_TIME_SCALE;
    btlv_ntp_short_time _full_shorted = btplv_ntp_to_short(full);
    uint32_t  _full_short_seconds = _full_shorted / BTLV_NTP_SHORT_TIME_SCALE;
    int diff_seconds = _short_seconds - _full_short_seconds;
    uint32_t _short_seconds_full = full_seconds + diff_seconds;
    uint32_t _short_fraction = _short % BTLV_NTP_SHORT_TIME_SCALE;
    btlv_ntp_time merge = _short_seconds_full * BTLV_NTP_TIME_SCALE + (_short_fraction * BTLV_NTP_TIME_SCALE)/ BTLV_NTP_SHORT_TIME_SCALE;
    BDBG_MSG(("btplv_ntp_merge: full:" BDBG_UINT64_FMT " short:%#x diff:%d merged:" BDBG_UINT64_FMT "", BDBG_UINT64_ARG(full), (unsigned)_short, diff_seconds, BDBG_UINT64_ARG(merge)));
    return merge;
}

int bmmt_parse_ipv6_address(const char *str, btlv_ipv6_address *addr)
{
    uint16_t tmp_addr[8];
    unsigned tmp_addr_offset=0;
    unsigned str_offset=0;
    bool has_double_colon = false;
    unsigned double_colon_offset = 0;
    bool done = false;
    bool port = false;
    unsigned i;

    /* RFC-4291 IP Version 6 Addressing Architecture
     * 2.2. Text Representation of Addresses */
    BKNI_Memset(addr, 0, sizeof(*addr));
    while(!done) {
        unsigned tmp_str_offset;
        char tmp_str[5];

        for(tmp_str_offset=0;;) {
            char ch = str[str_offset];
            str_offset++;
            if(ch=='\0' || ch=='.') {
                port = ch=='.';
                done = true;
                break;
            } else if (ch==':') {
                if(str[str_offset]==':') {
                    double_colon_offset = tmp_addr_offset;
                    has_double_colon = true;
                }
                break;
            } else {
                if(tmp_str_offset + 1 >= sizeof(tmp_str)) {
                    return -1;
                }
                tmp_str[tmp_str_offset] = ch;
                tmp_str_offset++;
            }
        }
        if(tmp_addr_offset>=sizeof(tmp_addr)) {
            return -1;
        }
        tmp_str[tmp_str_offset] = '\0';
        tmp_addr[tmp_addr_offset] = strtol(tmp_str,NULL,0x10);
        tmp_addr_offset++;
    }
    if(has_double_colon) {
        unsigned cur_addr = sizeof(addr->addr)/sizeof(addr->addr[0]);

        for(i=0;i<double_colon_offset+1 && i<tmp_addr_offset;i++) {
            B_MEDIA_SAVE_UINT16_BE(&addr->addr[i*2],tmp_addr[i]);
        }

        for(i=tmp_addr_offset;i>double_colon_offset+2;i--) {
            B_MEDIA_SAVE_UINT16_BE(&addr->addr[cur_addr-2],tmp_addr[i-1]);
            cur_addr -= 2;
        }
    } else {
        for(i=0;i<tmp_addr_offset;i++) {
            B_MEDIA_SAVE_UINT16_BE(&addr->addr[i*2],tmp_addr[i]);
        }
    }
    if(port) {
        addr->port = strtol(str+str_offset, NULL, 10);
    }
    return 0;
}

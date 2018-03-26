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
#ifndef __BMMT_H__
#define __BMMT_H__ 1

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <pthread.h>
#include "bmmt_common_types.h"
#include "bstd.h"
#include "bkni.h"
#include "nexus_platform.h"
#include "nexus_playpump.h"
#include "nexus_recpump.h"
#include "nexus_frontend.h"
#include "nexus_parser_band.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define BMMT_FILE_NAME_LENGTH 256
#define BMMT_MAX_STREAMS 256;
#define BMMT_TLV_PKT_READ_SIZE 188 /*16*1024*/
#define BMMT_MAX_TLV_BUFFERS 360 /*8*/
#define BMMT_MAX_TS_BUFFERS 360
#define BMMT_MAX_MSG_BUFFERS 8
#define BMMT_MAX_MMT_SI_BUFFER_SIZE 4096*2
#define BMMT_MAX_TLV_SI_BUFFER_SIZE 1024

/**
Summary:
 bmmt_io_data holds the pointers of buffers containing either
 TLV or MPEG2TS packets. These buffers are filled by a file or
 live source and emptied by a TLV parser
 */
typedef struct bmmt_io_data {
    void *io_buf;
} bmmt_io_data;

/**
Summary:
 bmmt_input_format defines types of input mmt library can
 take from either a file source or live source
**/
typedef enum bmmt_input_format {
    ebmmt_input_format_mpeg2ts, /* packet format -> MPEG2TS->TLV->IP->UDP->MMTP->MMT */
    ebmmt_input_format_tlv,     /* packet format -> TLV->IP->UDP->MMTP->MMT */
    ebmmt_input_format_max
}bmmt_input_format;

/**
Summary:
 bmmt_msg_type defines the type of messages from divided TLV
 packets (mpeg2ts) or TLV packets.
**/
typedef enum bmmt_msg_type {
    ebmmt_msg_type_mmt, /* mmt si */
    ebmmt_msg_type_tlv, /* tlv si */
    ebmmt_msg_type_max
}bmmt_msg_type;

/**
Summary:
 bmmt_open_settings configures the mmt instance settings.
**/
typedef struct bmmt_open_settings {
    char fileName[BMMT_FILE_NAME_LENGTH]; /* valid only if playback = true */
    char fileOut[BMMT_FILE_NAME_LENGTH];  /* valid if pesOut = true */
    NEXUS_ParserBand parserBand; /* valid only if playback = false */
    bool playback; /* Is the input sources a file or tuner/demod */
    bmmt_input_format input_format;
    uint32_t tlv_pid; /* valid only for ebmmt_input_format_tlv */
    bool pesOut; /* pes file output from CPU writes */
    bool loop;
}bmmt_open_settings;

/**
Summary:
 bmmt_buffer defines buffer paramers for es streams, tlv
 messages and MMT messages
**/
typedef struct bmmt_buffer {
    void *data;      /* buffer pointer */
    unsigned offset; /* current data size */
    size_t length; /* buffer size */
}bmmt_buffer;

/**
Summary:
 bmmt_stream_settings defines settings for individual es
 streams embedded in MMT packets
**/
typedef struct bmmt_stream_settings {
    unsigned pid; /* MMT packet ID */
    bmmt_stream_type stream_type; /* audio or video */
}bmmt_stream_settings;


/**
Summary:
 bmmt_msg_settings defines settings for either TLV or MMT
 messages.
**/
typedef struct bmmt_msg_settings {
    unsigned pid; /* valid for ebmmt_msg_type_mmt */
    bmmt_msg_type msg_type;
}bmmt_msg_settings;

/**
Summary:
 bmmt defines a context for an instance of mmt library.
**/
typedef struct bmmt *bmmt_t;

/**
Summary:
 bmmt_stream defines a context for an es stream extracted from
 MMT packets
**/
typedef struct bmmt_stream *bmmt_stream_t;

/**
Summary:
 bmmt_stream defines a context for an es stream extracted from
 MMT packets
**/
typedef struct bmmt_msg *bmmt_msg_t;

/**
Summary:
bmmt_get_default_open_settings gets default mmt open settings
**/

void bmmt_get_default_open_settings(bmmt_open_settings *pOpenSettings);

/**
Summary:
bmmt_open opens an instance of mmt library
**/

bmmt_t bmmt_open(bmmt_open_settings *pOpenSettings);


/**
Summary:
bmmt_close closes an instance of mmt library
**/

int bmmt_close(bmmt_t);


/**
Summary:
bmmt_start starts processing TLV or MPEG2TS packets
**/
int bmmt_start(bmmt_t mmt);

/**
Summary:
bmmt_stop stops processing TLV or MPEG2TS packets
**/
int bmmt_stop(bmmt_t mmt);


/**
Summary:
bmmt_set_ip_filter sets an ip filter for a particular service
extracted from TLV SI AMT. passing btlv_ip_address as null will
clear the IP filtering and not process any MMTP packets.
When ip filter is cleared only TLV messages are processed.
**/
void bmmt_set_ip_filter(bmmt_t mmt, btlv_ip_address *addr);


/**
Summary:
bmmt_get_default_stream_settings gets default es stream settings
**/
void bmmt_stream_get_default_settings(bmmt_stream_settings *settings);

/**
Summary:
bmmt_es_stream_opens an es stream context associated with a
particular MMT packet ID.
**/
bmmt_stream_t bmmt_stream_open(bmmt_t mmt, bmmt_stream_settings *settings);

/**
Summary: bmmt_es_stream_closes an es stream context associated
with a particular MMT packet ID.
**/
int bmmt_stream_close(bmmt_stream_t stream);

/**
Summary:
bmmt_stream_get_pid_channel returns a nexus PID channel
assocated with an es stream context.
*/
NEXUS_PidChannelHandle bmmt_stream_get_pid_channel(bmmt_stream_t);


/**
Summary: bmmt_msg_get_default_settings gets default msg settings
**/
void bmmt_msg_get_default_settings(bmmt_msg_settings *settings);

/**
Summary:
bmmt_msg_open opens either a TLV message or MMT message context.
MMT message context requires an MMT packet ID.
There can be many mmt message contexts, however only one tlv
message context is required.
**/
bmmt_msg_t bmmt_msg_open(bmmt_t mmt, bmmt_msg_settings *settings);

/**
Summary:
bmmt_msg_close closes mmt message context (tlv si or mmt si)
**/
int bmmt_msg_close(bmmt_msg_t msg);

/**
Summary:
bmmt_msg_get_buffer copies a message buffer into buf and returns
number of bytes in the buf.
buf_len is input from app and indicates size (in bytes) of buf.
buf_len has to be BMMT_MAX_MMT_SI_BUFFER_SIZE for mmt message.
buf_len has to be BMMT_MAX_TLV_SI_BUFFER_SIZE for TLV message.
**/
size_t bmmt_msg_get_buffer(bmmt_msg_t msg,void *buf, size_t buf_len);

/**
Summary:
bmmt_get_pl_table parses buf and populates the MMT package list
table. Returns true if pl_table is populated else false.
**/
bool bmmt_get_pl_table(void *buf, size_t len, bmmt_pl_table *pl_table);

/**
Summary:
bmmt_get_mp_table parses buf and populates the MMT media package
table. Returns true if mp_table is populated else false.
**/
bool bmmt_get_mp_table(void *buf, size_t len, bmmt_mp_table *mp_table);

/**
Summary:
bmmt_get_am_table parses buf and populates the TLV address
mapping table. Returns true if am_table is populated else false.
*/
bool bmmt_get_am_table(void *buf, size_t len, btlv_am_table *am_table);

/**
Summary:
bmmt_get_tlv_sync_byte_bitshift returns a number between 1 and 8 if
the passed buffer has detected sync byte for 2 tlv packets.
if not sync byte is found, then returned number would be 255
**/
uint8_t bmmt_get_tlv_sync_byte_bitshift(uint8_t *buf, size_t len);

#ifdef __cplusplus
}
#endif


#endif /* __BMMT_H__ */

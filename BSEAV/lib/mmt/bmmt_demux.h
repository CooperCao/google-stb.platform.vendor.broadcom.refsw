/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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

 **************************************************************************/
#ifndef __BMMT_DEMUX_H__
#define __BMMT_DEMUX_H__ 1

#include "bmmt_parser.h"
#include "bmedia_util.h"
#include "bioatom.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct bmmt_timestamp_queue *bmmt_timestamp_queue;

bmmt_timestamp_queue bmmt_timestamp_queue_create(void);
void bmmt_timestamp_queue_destroy(bmmt_timestamp_queue queue);

int bmmt_timestamp_queue_add_timestamp_descriptors(bmmt_timestamp_queue queue,
                                            const btlv_mpu_timestamp_descriptor_entry * timestamps, unsigned n_timestamps);

int bmmt_timestamp_queue_add_extended_timestamp_descriptors(bmmt_timestamp_queue queue,
                                            const btlv_mpu_extended_timestamp_descriptor_header *extended_timestamp_descriptor_header,
                                            const btlv_mpu_extended_timestamp_descriptor_entry *extended_timestamp_descriptor_entry, unsigned n_extended_timestamps
                                           );

int bmmt_timestamp_queue_get_presentation_time(bmmt_timestamp_queue queue, uint32_t sequence_number, uint32_t sample_number, btlv_ntp_time *presentation_time);


typedef enum bmmt_stream_type {
    bmmt_stream_type_unknown,
    bmmt_stream_type_h265,
    bmmt_stream_type_aac
} bmmt_stream_type;

typedef struct bmmt_demux_time_info {
    bmedia_pes_info pes_info;
    uint32_t mpu_time; /* in 45KHz ticks */
    bool mpu_time_valid;
} bmmt_demux_time_info;

typedef struct bmmt_demux_stream_config {
    uint16_t packet_id;
    uint8_t pes_stream_id;
    bmmt_stream_type stream_type;
#if 0
    btlv_ip_address ip_address;
#endif
    void *stream_context;
    void (*stream_data)(void *stream_context, batom_accum_t data, const bmmt_demux_time_info *time_info);
    int (*copy_payload)(void *stream_context, batom_accum_t accum, batom_cursor *cursor, unsigned bytes);
} bmmt_demux_stream_config;

typedef struct bmmt_demux_config {
    void *context;
    int (*copy_payload)(void *context, batom_accum_t accum, batom_cursor *cursor, unsigned bytes);
} bmmt_demux_config;


typedef struct bmmt_demux *bmmt_demux_t;
typedef struct bmmt_demux_stream *bmmt_demux_stream_t;

void bmmt_demux_config_init(bmmt_demux_config *config);
bmmt_demux_t bmmt_demux_create(batom_factory_t factory, const bmmt_demux_config *config);
void bmmt_demux_destroy(bmmt_demux_t demux);

void bmmt_demux_stream_config_init(bmmt_demux_stream_config *config);
bmmt_demux_stream_t bmmt_demux_stream_create(bmmt_demux_t demux,  bmmt_demux_stream_config *config);
void bmmt_demux_stream_destroy(bmmt_demux_t demux, bmmt_demux_stream_t stream);
int bmmt_demux_process_signaling_message(bmmt_demux_t demux, batom_cursor *payload, const btlv_signalling_header *signalling_header);
int bmmt_demux_stream_process_payload(bmmt_demux_t demux, bmmt_demux_stream_t stream, const btlv_mmt_packet_header *header, batom_cursor *payload);



#ifdef __cplusplus
}
#endif


#endif /* __BMMT_DEMUX_H__ */

/***************************************************************************
 *     Broadcom Proprietary and Confidential. (c)2014 Broadcom.  All rights reserved.
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
 *
 **************************************************************************/
#ifndef __KHRN_EVENT_MONITOR_H__
#define __KHRN_EVENT_MONITOR_H__

#include "khrn_int_common.h"
#include "libs/platform/bcm_perf_structs.h"

typedef struct bcm_sched_event_track_desc    KHRN_DRIVER_EVENT_TRACK_DESC;
typedef struct bcm_sched_event_desc          KHRN_DRIVER_EVENT_DESC;
typedef struct bcm_sched_event_field_desc    KHRN_DRIVER_EVENT_FIELD_DESC;
typedef bcm_sched_event_type                 KHRN_DRIVER_EVENT_TYPE;

typedef enum KHRN_DRIVER_EVENT_ID {
   KHRN_DRIVER_EVENT_CPU_CACHE_FLUSH      = 0,
   KHRN_DRIVER_EVENT_FENCE_WAIT           = 1,
   KHRN_DRIVER_EVENT_EGL_IMAGE_UPDATE     = 2,
   KHRN_DRIVER_EVENT_GENERATE_MIPMAPS     = 3,
   KHRN_DRIVER_EVENT_MONITOR_NUM_EVENTS   = 4
} KHRN_DRIVER_EVENT_ID;

typedef enum KHRN_DRIVER_TRACK_ID {
   KHRN_DRIVER_TRACK_DRIVER               = 0,
   KHRN_DRIVER_TRACK_IMAGE_CONV           = 1,
   KHRN_DRIVER_EVENT_MONITOR_NUM_TRACKS   = 2
} KHRN_DRIVER_TRACK_ID;

uint32_t khrn_driver_get_num_event_tracks();

uint32_t khrn_driver_get_num_events();

bool khrn_driver_describe_event_track(uint32_t track_index,
                                    KHRN_DRIVER_EVENT_TRACK_DESC *track_desc);

bool khrn_driver_describe_event(uint32_t event_index,
                              KHRN_DRIVER_EVENT_DESC *event_desc);

bool khrn_driver_describe_event_data(uint32_t event_index,
                                    uint32_t field_index,
                                    KHRN_DRIVER_EVENT_FIELD_DESC *field_desc);

uint32_t khrn_driver_poll_event_timeline(size_t    max_event_buffer_bytes,
                                       void        *event_buffer,
                                       bool        *lost_data,
                                       uint64_t    *timestamp_us);

void khrn_driver_poll_set_event_collection(bool collect_events);

bool khrn_driver_add_event(uint32_t                track_index,
                           uint32_t                rec_event_id,
                           KHRN_DRIVER_EVENT_ID    event_index,
                           KHRN_DRIVER_EVENT_TYPE  event_type);

uint32_t khrn_driver_track_next_id(KHRN_DRIVER_TRACK_ID track);

#endif

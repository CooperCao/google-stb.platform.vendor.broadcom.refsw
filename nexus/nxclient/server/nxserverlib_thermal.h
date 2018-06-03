/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 *****************************************************************************/

#ifndef NXSERVERLIB_THERMAL_H
#define NXSERVERLIB_THERMAL_H

#include "blst_queue.h"

#define MAX_NUM_TZONE 16
#define MAX_NUM_CDEV 32
#define MAX_NUM_TRIP 16
#define MAX_NUM_FREQ 5

#define THERMAL_SYSFS "/sys/class/thermal"
#define CDEV "cooling_device"
#define TZONE "thermal_zone"

#define CPUFREQ_SYSFS "/sys/devices/system/cpu/cpu0/cpufreq"

#define MAX_NUM_POWER_SAMPLES 20

#define PMU_SYSFS "/sys/class/i2c-adapter/i2c-0/0-0040/iio:device0"
#define POWER_RAW "in_power2_raw"

typedef struct thermal_data {
    unsigned temp[MAX_NUM_TZONE];
    struct {
        unsigned idx, total;
        unsigned samples[MAX_NUM_POWER_SAMPLES];
        unsigned instant;
        unsigned average;
    } power;
} thermal_data;

typedef struct cooling_device_info {
    char type[256];
    int instance;
    int  max_state;
    int  cur_state;
    unsigned  flag;
} cooling_device_info;

typedef struct trip_point_info {
    char type[256];
    unsigned  temp;
    unsigned  hyst;
    int attribute; /* programmability etc. */
} trip_point_info;

/* thermal zone configuration information, binding with cooling devices could
 * change at runtime.
 */
typedef struct thermal_zone_info {
    char type[256];
    int instance;
    int passive; /* active zone has passive node to force passive mode */
    int num_cooling_devices; /* number of cooling device binded */
    int num_trip_points;
    trip_point_info tp[MAX_NUM_TRIP];
    unsigned cdev_binding; /* bitmap for attached cdevs */
    /* cdev bind trip points, allow one cdev bind to multiple trips */
    unsigned trip_binding[MAX_NUM_CDEV];
} thermal_zone_info;

typedef struct thermal_info {
    int num_thermal_zones;
    int num_cooling_devices;
    thermal_zone_info tzi[MAX_NUM_TZONE];
    cooling_device_info cdi[MAX_NUM_CDEV];
} thermal_info;

/* Cooling Agents */
typedef struct cooling_agent {
    char name[32];
    NEXUS_Error (*func) (struct cooling_agent *, unsigned);
    unsigned levels;    /* Max number of levels of throttling */
    unsigned cur_level;
} cooling_agent;

typedef struct priority_list {
    BLST_Q_ENTRY(priority_list) link;
    cooling_agent *agent;
    unsigned level;
    NEXUS_Timestamp last_applied_time;
    NEXUS_Timestamp last_removed_time;
} priority_list;


typedef struct cpufreq_info {
    unsigned avail_freqs[MAX_NUM_FREQ];
    unsigned num_p_states;
} cpufreq_info;

#endif /* NXSERVERLIB_THERMAL_H */

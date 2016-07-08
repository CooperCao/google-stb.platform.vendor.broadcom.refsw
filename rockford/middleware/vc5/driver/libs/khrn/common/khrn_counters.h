/*=============================================================================
Broadcom Proprietary and Confidential. (c)2011 Broadcom.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Driver and performance counters
=============================================================================*/
#include "khrn_int_common.h"
#include "libs/platform/bcm_perf_structs.h"
#include "libs/core/v3d/v3d_gen.h"

#ifndef KHRN_COUNTERS_H
#define KHRN_COUNTERS_H

typedef enum {
   KHRN_PERF_DRAW_CALLS                = 0,
   KHRN_PERF_HARD_CLEAR                = 1,
   KHRN_PERF_SOFT_CLEAR                = 2,
   KHRN_PERF_COUNTERS_NUM              = 3,
} KHRN_PERF_COUNTER_NAME_T;

typedef struct bcm_sched_counter_group_desc        KHRN_PERF_COUNTER_GROUP_DESC;
typedef struct bcm_sched_counter                   KHRN_PERF_COUNTER;
typedef struct bcm_sched_group_counter_selector    KHRN_GROUP_PERF_COUNTER_SELECTOR;

uint32_t khrn_driver_get_num_counter_groups();

bool  khrn_driver_enumerate_group_counters(uint32_t group,
                                          KHRN_PERF_COUNTER_GROUP_DESC *desc);

uint32_t khrn_driver_get_counters(KHRN_PERF_COUNTER *counters,
                                 uint32_t max_counters,
                                 bool resetCounters);

void khrn_driver_incr_counters(uint32_t counter);

bool khrn_driver_select_group_counters(const KHRN_GROUP_PERF_COUNTER_SELECTOR *selector);

#endif

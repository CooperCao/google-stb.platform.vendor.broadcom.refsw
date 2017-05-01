/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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
} khrn_perf_counter_name;

typedef struct bcm_sched_counter_group_desc        khrn_perf_counter_group_desc;
typedef struct bcm_sched_counter                   khrn_perf_counter;
typedef struct bcm_sched_group_counter_selector    khrn_group_perf_counter_selector;

uint32_t khrn_driver_get_num_counter_groups();

bool  khrn_driver_enumerate_group_counters(uint32_t group,
                                          khrn_perf_counter_group_desc *desc);

uint32_t khrn_driver_get_counters(khrn_perf_counter *counters,
                                 uint32_t max_counters,
                                 bool resetCounters);

void khrn_driver_incr_counters(uint32_t counter);

bool khrn_driver_select_group_counters(const khrn_group_perf_counter_selector *selector);

#endif

/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "bcm_perf_structs.h"

#include <stdint.h>
#include <stdbool.h>

typedef enum {
   KHRN_PERF_HARD_CLEARS,
   KHRN_PERF_SOFT_CLEARS,
   KHRN_PERF_TB_GRP_COLOR_LOADS,
   KHRN_PERF_TB_GRP_MS_COLOR_LOADS,
   KHRN_PERF_TB_GRP_DS_LOADS,
   KHRN_PERF_TB_GRP_COLOR_STORES,
   KHRN_PERF_TB_GRP_MS_COLOR_STORES,
   KHRN_PERF_TB_GRP_DS_STORES,
   KHRN_PERF_TB_COLOR_LOADS,
   KHRN_PERF_TB_MS_COLOR_LOADS,
   KHRN_PERF_TB_DS_LOADS,
   KHRN_PERF_TB_COLOR_STORES,
   KHRN_PERF_TB_MS_COLOR_STORES,
   KHRN_PERF_TB_DS_STORES,
   KHRN_PERF_TEX_SUBMISSIONS,
   KHRN_PERF_TEX_FAST_PATHS,
   KHRN_PERF_MIPMAP_GENS,
   KHRN_PERF_MIPMAP_GENS_FAST,
   KHRN_PERF_DRAW_CALLS,
   KHRN_PERF_NUM_SWAPS,

   KHRN_PERF_COUNTERS_NUM /* leave as last */
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

/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION

=============================================================================*/

#include "middleware/khronos/common/khrn_counters.h"
#include "middleware/khronos/common/khrn_process.h"
#include "v3d_platform/bcm_perf_structs.h"
#include "v3d_platform/bcm_perf_api.h"

static uint32_t                        s_v3d_driver_perf_counter_group;
static KHRN_PERF_COUNTER               s_driver_counters[V3D_MAX_COUNTERS_PER_GROUP];
static bool                            s_driver_counter_enabled[V3D_MAX_COUNTERS_PER_GROUP];
static KHRN_PERF_COUNTER_GROUP_DESC    s_driver_counter_group_desc;
static bool                            s_initialise_group_desc = false;

/* ******************************************************************************** */
/*                      Helper functions                                            */

static void SetCounterNamesEx(KHRN_PERF_COUNTER_GROUP_DESC    *grp,
                              KHRN_PERF_COUNTER_NAME_T        index,
                              const char                      *name,
                              const char                      *unit,
                              uint64_t                        minVal,
                              uint64_t                        maxVal,
                              uint64_t                        denom
                              )
{
   strncpy(grp->counters[index].name, name, V3D_MAX_COUNTER_NAME_LEN);
   strncpy(grp->counters[index].unit_name, unit, V3D_MAX_COUNTER_UNIT_NAME_LEN);

   if ((uint32_t)index >= grp->total_counters)
   {
      grp->total_counters = index + 1;
      grp->max_active_counters = index + 1;
   }

   grp->counters[index].min_value = minVal;
   grp->counters[index].max_value = maxVal;
   grp->counters[index].denominator = denom;
}

static void SetCounterNames(KHRN_PERF_COUNTER_GROUP_DESC *grpDesc,
                           KHRN_PERF_COUNTER_NAME_T      index,
                           const char                    *name,
                           const char                    *unit
                           )
{
   SetCounterNamesEx(grpDesc, index, name, unit, 0, ~0, 1);
}

static bool initialise_group_desc()
{
   s_v3d_driver_perf_counter_group = bcm_sched_get_num_counter_groups();
   memset(&s_driver_counters, 0, sizeof(s_driver_counters));
   memset(&s_driver_counter_enabled, 0, sizeof(s_driver_counter_enabled));
   strncpy(s_driver_counter_group_desc.name, "Driver Counters", V3D_MAX_GROUP_NAME_LEN);
   SetCounterNames(&s_driver_counter_group_desc, KHRN_PERF_DRAW_CALLS, "khrn_draw_calls", "draws");
   SetCounterNames(&s_driver_counter_group_desc, KHRN_PERF_HARD_CLEAR, "khrn_hard_clear", "clears");
   SetCounterNames(&s_driver_counter_group_desc, KHRN_PERF_SOFT_CLEAR, "khrn_soft_clear", "clears");

   return true;
}

/* ******************************************************************************** */

uint32_t khrn_driver_get_num_counter_groups()
{
   return V3D_DRIVER_PERF_COUNTER_NUM_GROUPS;
}

uint32_t khrn_driver_get_counters(KHRN_PERF_COUNTER *counters,
                                 uint32_t max_counters,
                                 bool resetCounters
                                 )
{
   uint32_t num_counters = 0;

   if (!s_initialise_group_desc)
      s_initialise_group_desc = initialise_group_desc();

   // if it is a request for the number of counters
   if ((max_counters == 0) || (counters == NULL))
   {
      num_counters = KHRN_PERF_COUNTERS_NUM;
   }
   else
   {
      uint32_t counter_index = 0;
      for (counter_index = 0; counter_index < max_counters; counter_index++)
      {
         if (s_driver_counter_enabled[counter_index])
         {
            counters[counter_index].group_index = s_v3d_driver_perf_counter_group;
            counters[counter_index].counter_index = counter_index;
            counters[counter_index].value = s_driver_counters[counter_index].value;

            if (resetCounters)
               s_driver_counters[counter_index].value = 0;

            num_counters++;
         }
      }
   }

   return num_counters;
}

bool khrn_driver_enumerate_group_counters(uint32_t group,
                                          KHRN_PERF_COUNTER_GROUP_DESC *desc
                                          )
{
   bool ret = false;

   if (!s_initialise_group_desc)
      s_initialise_group_desc = initialise_group_desc();

   if (group == s_v3d_driver_perf_counter_group)
   {
      *desc = s_driver_counter_group_desc;

      ret = true;
   }
   else
      ret = false;

   return ret;
}

bool khrn_driver_select_group_counters(const KHRN_GROUP_PERF_COUNTER_SELECTOR *selector)
{
   bool ret = false;

   if (!s_initialise_group_desc)
      s_initialise_group_desc = initialise_group_desc();

   if (selector->group_index == s_v3d_driver_perf_counter_group)
   {
      uint32_t counter_index;

      assert(selector->num_counters <= V3D_MAX_COUNTERS_PER_GROUP);

      for (counter_index = 0; counter_index < selector->num_counters; counter_index++)
      {
         assert(selector->counters[counter_index] < V3D_MAX_COUNTERS_PER_GROUP);
         s_driver_counter_enabled[selector->counters[counter_index]] = selector->enable;
         s_driver_counters[selector->counters[counter_index]].value  = 0;
      }

      ret = true;
   }
   else
      ret = false;

   return ret;
}

void khrn_driver_incr_counters(uint32_t counter)
{
   if (s_driver_counter_enabled[counter])
      s_driver_counters[counter].value++;
}

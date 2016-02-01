/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
Structures used by the performance counter and event timeline parts of the
scheduler API.
=============================================================================*/
#ifndef BCM_PERF_STRUCTS_H
#define BCM_PERF_STRUCTS_H

/*******************************************************************************
 * PERFORMANCE COUNTERS
 *******************************************************************************/

#define V3D_MAX_GROUP_NAME_LEN         64
#define V3D_MAX_COUNTER_NAME_LEN       64
#define V3D_MAX_COUNTER_UNIT_NAME_LEN  32
#define V3D_MAX_COUNTERS_PER_GROUP     96

#define V3D_DRIVER_PERF_COUNTER_NUM_GROUPS  1

enum bcm_sched_counter_state
{
   BCM_CTR_ACQUIRE   = 0,
   BCM_CTR_RELEASE   = 1,
   BCM_CTR_START     = 2,
   BCM_CTR_STOP      = 3
};

/* Descriptor structure for a counter.
 * Examples of unit_name : cycles, %, bytes, frames, hits, misses, etc.
*/
struct bcm_sched_counter_desc
{
   char                    name[V3D_MAX_COUNTER_NAME_LEN];
   char                    unit_name[V3D_MAX_COUNTER_UNIT_NAME_LEN];
   uint64_t                min_value;
   uint64_t                max_value;
   uint64_t                denominator;
};

/* Descriptor for a counter group */
struct bcm_sched_counter_group_desc
{
   char                           name[V3D_MAX_GROUP_NAME_LEN];
   uint32_t                       total_counters;
   uint32_t                       max_active_counters;
   struct bcm_sched_counter_desc  counters[V3D_MAX_COUNTERS_PER_GROUP];
};

/* Holds a list of counter values to be enabled/disabled for a given group */
struct bcm_sched_group_counter_selector
{
   uint32_t    group_index;
   uint32_t    enable;
   uint32_t    counters[V3D_MAX_COUNTERS_PER_GROUP];
   uint32_t    num_counters;
};

/* A single counter entry */
struct bcm_sched_counter
{
   uint32_t   group_index;
   uint32_t   counter_index;  /* Within group */
   uint64_t   value;
};

/*******************************************************************************
 * EVENT TIMELINE
 *******************************************************************************/
#define V3D_MAX_EVENT_STRING_LEN   64

enum bcm_sched_event_state
{
   BCM_EVENT_ACQUIRE = 0,
   BCM_EVENT_RELEASE = 1,
   BCM_EVENT_START   = 2,
   BCM_EVENT_STOP    = 3
};

typedef enum bcm_sched_event_type
{
   BCM_EVENT_BEGIN      = 0,
   BCM_EVENT_END        = 1,
   BCM_EVENT_ONESHOT    = 2
} bcm_sched_event_type;

enum bcm_sched_field_type
{
   BCM_EVENT_INT32  = 0,
   BCM_EVENT_UINT32 = 1,
   BCM_EVENT_INT64  = 2,
   BCM_EVENT_UINT64 = 3
};

/* Contains an event description. */
struct bcm_sched_event_desc
{
   char     name[V3D_MAX_EVENT_STRING_LEN];
   uint32_t num_data_fields;  /* How many optional extra fields there are */
};

/* Contains an event data field description. */
struct bcm_sched_event_field_desc
{
   char                       name[V3D_MAX_EVENT_STRING_LEN];
   enum bcm_sched_field_type  data_type;
};

/* Contains an event track description.
* The track is used to group related events. You can think of it as the row
* in an event table to which the event belongs. All binner related events
* should have a common track for example. This means they will all get displayed
* in the binner row in an event graphing tool.
*/
struct bcm_sched_event_track_desc
{
   char     name[V3D_MAX_EVENT_STRING_LEN];
};

#endif /* BCM_PERF_STRUCTS_H */

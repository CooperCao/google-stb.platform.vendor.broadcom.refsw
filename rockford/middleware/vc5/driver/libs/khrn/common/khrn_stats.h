/*=============================================================================
Broadcom Proprietary and Confidential. (c)2010 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Stats

FILE DESCRIPTION
Gather performance statistics.
=============================================================================*/

#ifndef KHRN_STATS_H
#define KHRN_STATS_H

#include "vcos.h"
#include <EGL/egl.h>
#include "khrn_int_common.h"

typedef enum {
   KHRN_STATS_GL,
   KHRN_STATS_VG,
   KHRN_STATS_IMAGE,
   KHRN_STATS_WAIT,
   KHRN_STATS_HW_RENDER,
   KHRN_STATS_HW_BIN,
   KHRN_STATS_COMPILE,
   KHRN_STATS_DRAW_CALL,
   KHRN_STATS_RECV_BULK,
   KHRN_STATS_SEND_BULK,
   KHRN_STATS_RECV_BULK_STONE,
   KHRN_STATS_SEND_BULK_STONE,
   KHRN_STATS_SEND_CTRL,
   KHRN_STATS_SEND_CTRL_STONE,
   KHRN_STATS_DISPATCH_TASK,
   KHRN_STATS_BUFFER_DATA,
   KHRN_STATS_VERTEX_CACHE_DATA,
   KHRN_STATS_UNMEM_DATA,
   KHRN_STATS_IMAGE_DATA,
   KHRN_STATS_FETCH_AND_DISPATCH,
   KHRN_STATS_THING_MAX,
} KHRN_STATS_TIME_T;

typedef enum {
   KHRN_STATS_SWAP_BUFFERS,
   KHRN_STATS_ROUND_TRIP,
   KHRN_STATS_ROUND_TRIP_STONE,
   KHRN_STATS_INTERNAL_FLUSH,
   KHRN_STATS_ANDROID_APP_FRAMES,
   KHRN_STATS_DRAW_CALL_COUNT,
   KHRN_STATS_DRAW_CALL_FASTPATH,
   KHRN_STATS_RENDER_UNMERGED,
   KHRN_STATS_RENDER_MERGED,
   KHRN_STATS_SHADER_CACHE_MISS,
   KHRN_STATS_EVENT_MAX,
} KHRN_STATS_EVENT_T;

#ifdef KHRN_STATS_ENABLE

#ifdef __VIDEOCORE__

#else
#include <time.h>
#endif

typedef struct {
   uint32_t id;
   uint32_t value;
   const char name[32];
   const char desc[80];
   bool stone;
} KHRN_STAT_VALUE_T;

typedef struct
{
   KHRN_STAT_VALUE_T event_count[KHRN_STATS_EVENT_MAX];
   KHRN_STAT_VALUE_T thing_time[KHRN_STATS_THING_MAX];
   bool     in_thing           [KHRN_STATS_THING_MAX];
   uint32_t start_time         [KHRN_STATS_THING_MAX];
   uint32_t reset_time;
   uint32_t reset_time_stone;
} KHRN_STATS_RECORD_T;

extern KHRN_STATS_RECORD_T khrn_stats_global_record;

/* Main interface */
extern void khrn_stats_reset(bool clear_in_thing, bool stone);
extern void khrn_stats_get_human_readable(char *buffer, uint32_t len, bool reset, bool stone);
extern void khrn_stats_get_xml(char *buffer, uint32_t len, bool reset, bool stone);
extern bool khrn_stats_busy(void);

/* Hooks */

#ifdef KHRN_STATS_TIMERS_ENABLE
static inline uint32_t khrn_stats_getmicrosecs() { return vcos_getmicrosecs(); }
#else
static inline uint32_t khrn_stats_getmicrosecs() { return 0; }
#endif

static inline void khrn_stats_record_start(uint32_t thing)
{
   assert(thing < KHRN_STATS_THING_MAX);
   //assert(!khrn_stats_global_record.in_thing[thing]);
   khrn_stats_global_record.in_thing[thing] = true;
   khrn_stats_global_record.start_time[thing] = khrn_stats_getmicrosecs();
}

static inline void khrn_stats_record_end(uint32_t thing)
{
   assert(thing < KHRN_STATS_THING_MAX);
   //assert(khrn_stats_global_record.in_thing[thing]);
   khrn_stats_global_record.in_thing[thing] = false;
   khrn_stats_global_record.thing_time[thing].value += khrn_stats_getmicrosecs() -
      khrn_stats_global_record.start_time[thing];
}

static inline void khrn_stats_record_add(uint32_t thing, uint32_t microsecs)
{
   assert(!khrn_stats_global_record.in_thing[thing]);
   khrn_stats_global_record.thing_time[thing].value += microsecs;
}

static inline void khrn_stats_record_event(uint32_t event)
{
   assert(event <= KHRN_STATS_EVENT_MAX);
   khrn_stats_global_record.event_count[event].value++;
}

static inline uint32_t khrn_stats_get_event(uint32_t event)
{
   assert(event <= KHRN_STATS_EVENT_MAX);
   return khrn_stats_global_record.event_count[event].value;
}

static inline bool khrn_stats_in(uint32_t thing)
{
   assert(thing < KHRN_STATS_THING_MAX);
   return khrn_stats_global_record.in_thing[thing];
}

#else
/* Stats recording is disabled */
static inline void khrn_stats_record_start(uint32_t thing) {}
static inline void khrn_stats_record_end(uint32_t thing) {}
static inline void khrn_stats_record_add(uint32_t thing, uint32_t microsecs) {}
static inline void khrn_stats_record_event(uint32_t event) {}
static inline uint32_t khrn_stats_get_event(uint32_t event) { return 0; }
static inline bool khrn_stats_in(uint32_t thing) { return false; }
static inline void khrn_stats_get_xml(char *buffer, uint32_t len, bool reset, bool stone) {}
#endif

#endif

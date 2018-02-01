/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "sched_nexus.h"
#include "bcm_perf_structs.h"
#include <stdint.h>
#include "vcos_chrono.h"

#ifdef __cplusplus
extern "C" {
#endif

// Event tracks
#define PERF_EVENT_TRACK_QUEUE      0
#define PERF_EVENT_TRACK_DISPLAY    1

// Events
#define PERF_EVENT_QUEUE            0
#define PERF_EVENT_DEQUEUE          1
#define PERF_EVENT_ON_DISPLAY       2

void PerfInitialize(EventContext *ctx);
void PerfTerminate(EventContext *ctx);

uint64_t PerfGetTimeNow();
void PerfAddEvent(EventContext *ctx, uint32_t track, uint32_t event, uint32_t id,
                  bcm_sched_event_type type, ...);
void PerfAddEventWithTime(EventContext *ctx, uint32_t track, uint32_t event, uint32_t id,
                          bcm_sched_event_type type, uint64_t timestamp, ...);

void PerfAdjustEventCounts(EventContext *ctx, uint32_t *numTracks, uint32_t *numEvents);
BEGL_SchedStatus PerfGetEventTrackInfo(EventContext *ctx, uint32_t track,
                                       struct bcm_sched_event_track_desc *track_desc);
BEGL_SchedStatus PerfGetEventInfo(EventContext *ctx, uint32_t event, struct bcm_sched_event_desc *event_desc);
BEGL_SchedStatus PerfGetEventDataFieldInfo(EventContext *ctx, uint32_t event, uint32_t field,
                                           struct bcm_sched_event_field_desc *field_desc);
BEGL_SchedStatus PerfSetEventCollection(EventContext *ctx, BEGL_SchedEventState state);
uint32_t PerfGetEventData(EventContext *ctx, uint32_t event_buffer_bytes, void *event_buffer,
                          uint32_t *lost_data, uint64_t *timestamp);

#ifdef __cplusplus
}
#endif

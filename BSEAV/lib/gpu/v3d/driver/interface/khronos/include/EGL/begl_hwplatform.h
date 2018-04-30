/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#ifndef __cplusplus
#include <stdint.h>
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BEGL_HWInfo
{
   char     name[10];
   uint32_t numSlices;
   uint32_t textureUnitsPerSlice;
   uint32_t techRev;
   uint32_t revision;
   char     revStr[3];
   uint64_t time;
} BEGL_HWInfo;

typedef enum
{
   BEGL_HW_OP_END = 0,     /* Args none                        */
   BEGL_HW_OP_BIN,         /* Args bin_start    and bin_end    */
   BEGL_HW_OP_RENDER,      /* Args render_start and render_end */
   BEGL_HW_OP_USER,        /* Args user_start                  */
   BEGL_HW_OP_WAIT,        /* Args flags                       */
   BEGL_HW_OP_SYNC,        /* Args none                        */
   BEGL_HW_OP_NOTIFY,      /* Args flags                       */
   BEGL_HW_OP_FENCE,       /* Args fd for the fence            */
   BEGL_HW_OP_SECURE       /* Args is the job secure           */
} BEGL_HWOperation;

typedef enum
{
   BEGL_HW_SIG_BIN    = 1 << 0,
   BEGL_HW_SIG_RENDER = 1 << 1,
   BEGL_HW_SIG_USER   = 1 << 2,

   BEGL_HW_SIG_ALL_UNITS = BEGL_HW_SIG_BIN | BEGL_HW_SIG_RENDER | BEGL_HW_SIG_USER
} BEGL_HWWaitSignaller;

typedef struct BEGL_HWInstruction
{
   BEGL_HWOperation   operation;
   uint32_t           arg1;
   uint64_t           arg2;
   uint64_t           callbackParam;
} BEGL_HWInstruction;

#define BEGL_HW_JOB_MAX_INSTRUCTIONS   8

typedef struct BEGL_HWJob
{
   BEGL_HWInstruction   program[BEGL_HW_JOB_MAX_INSTRUCTIONS];

   uint32_t          binMemory;         /* Pointer to initial bin memory for tasks in this program  */
   bool              binMemorySecure;   /* Bin memory came from secure pool, so it can be attached  */
   uint32_t          userVPM;           /* Settings for the V3D VPM for this job                    */
   bool              collectTimeline;   /* Requests time-line data for the job                      */
   uint64_t          jobSequence;       /* Unique sequence value                                    */
} BEGL_HWJob;

typedef struct BEGL_HWEventTime
{
   uint32_t secs;
   uint32_t microsecs;
} BEGL_HWEventTime;

typedef struct BEGL_HWTimelineData
{
   BEGL_HWEventTime binStart;
   BEGL_HWEventTime binEnd;
   BEGL_HWEventTime renderStart;
   BEGL_HWEventTime renderEnd;
   BEGL_HWEventTime userStart;
   BEGL_HWEventTime userEnd;
} BEGL_HWTimelineData;

typedef struct BEGL_HWNotification
{
   void                 *param;
   bool                 needsSync;
   bool                 outOfMemory;
   uint64_t             jobSequence;
   BEGL_HWTimelineData  *timelineData;
} BEGL_HWNotification;

typedef struct BEGL_HWBinMemorySettings
{
   bool        secure;
} BEGL_HWBinMemorySettings;

typedef struct BEGL_HWBinMemory
{
   uint32_t    address;
   uint32_t    size;
} BEGL_HWBinMemory;

typedef struct BEGL_HWCallbackRecord
{
   uint32_t    reason;
   uint64_t    payload[15];
} BEGL_HWCallbackRecord;

typedef enum
{
   BEGL_CtrAcquire   = 0,
   BEGL_CtrRelease   = 1,
   BEGL_CtrStart     = 2,
   BEGL_CtrStop      = 3
} BEGL_SchedCounterState;

/* Forwards declare types used in interface */
struct bcm_sched_counter_group_desc;
struct bcm_sched_counter;
struct bcm_sched_group_counter_selector;

typedef struct BEGL_SchedPerfCountInterface
{
   void               (*GetPerfNumCounterGroups)(void *context, void *session, uint32_t *numGroups);
   bool               (*GetPerfCounterGroupInfo)(void *context, void *session, uint32_t group, struct bcm_sched_counter_group_desc *desc);
   bool               (*SetPerfCounting)(void *context, void *session, BEGL_SchedCounterState state);
   bool               (*ChoosePerfCounters)(void *context, void *session, const struct bcm_sched_group_counter_selector *selector);
   uint32_t           (*GetPerfCounterData)(void *context, void *session, struct bcm_sched_counter  *counters, uint32_t max_counters, uint32_t reset_counts);
} BEGL_SchedPerfCountInterface;

typedef enum
{
   BEGL_EventAcquire = 0,
   BEGL_EventRelease = 1,
   BEGL_EventStart   = 2,
   BEGL_EventStop    = 3
} BEGL_SchedEventState;

/* Forwards declare types used in interface */
struct bcm_sched_event_track_desc;
struct bcm_sched_event_desc;
struct bcm_sched_event_field_desc;

typedef struct BEGL_SchedEventTrackInterface
{
   /* Event timeline */
   void               (*GetEventCounts)(void *context, void *session, uint32_t *numTracks, uint32_t *numEvents);
   bool               (*GetEventTrackInfo)(void *context, void *session, uint32_t track, struct bcm_sched_event_track_desc *track_desc);
   bool               (*GetEventInfo)(void *context, void *session, uint32_t event, struct bcm_sched_event_desc *event_desc);
   bool               (*GetEventDataFieldInfo)(void *context, void *session, uint32_t event, uint32_t field, struct bcm_sched_event_field_desc *field_desc);
   bool               (*SetEventCollection)(void *context, void *session, BEGL_SchedEventState state);
   uint32_t           (*GetEventData)(void *context, void *session, uint32_t event_buffer_bytes, void *event_buffer, uint32_t *overflowed, uint64_t *timebase_us);
} BEGL_SchedEventTrackInterface;

 /* The platform MUST provide an implementation of this interface in order that the EGL driver
 * can interact with platform hardware.
 */
typedef struct BEGL_HWInterface
{
   /* Context pointer - opaque to the 3d driver code, but passed out in all function pointer calls.
      Prevents the client code needing to perform context lookups. */
   void *context;

   /***************************/
   /* NEW Job-based interface */
   /***************************/

   /* Fills in information about the version and features of the V3D core */
   bool (*GetInfo)(void *context, BEGL_HWInfo *info);

   /* Send a job to the V3D job queue */
   bool (*SendJob)(void *context, BEGL_HWJob *job);

   /* Retrieves and removes latest notification for this client */
   bool (*GetNotification)(void *context, BEGL_HWNotification *notification);

   /* Acknowledge the last synchronous notification */
   void (*SendSync)(void *context, bool abandon);

   /* Request bin memory */
   bool (*GetBinMemory)(void *context, const BEGL_HWBinMemorySettings *settings, BEGL_HWBinMemory *memory);

   /* Create a fence */
   void (*FenceOpen)(void *context, int *fd, uint64_t *p, char type);

   /* Signal a fence (only used for userspace sync) */
   void (*FenceSignal)(void *context, int fd);

   /* Get a platform fence from fd (only used for userspace sync) */
   void *(*FenceGet)(void *context, int fd);

   /* Fence wait async */
   void (*FenceWaitAsync)(void *context, int fd, uint64_t *v3dfence);

   /* Performance counters */
   BEGL_SchedPerfCountInterface  perf_count_iface;

   /* Event timeline */
   BEGL_SchedEventTrackInterface event_track_iface;

} BEGL_HWInterface;

/* The client application, or default platform library must register valid versions of each
   of these interfaces before any EGL or GL functions are invoked, using the following functions
   provided by the 3D driver.
*/
typedef struct
{
   /* Callback which will be called by the job server when a callback-flagged instruction has completed. */
   /* Call GetNotification() to get the data associated with this callback. */
   /* Synchronous callbacks must be acknowledged using SendSync. */
   void (*JobCallback)(void);

} BEGL_HardwareCallbacks;

#ifdef __cplusplus
}
#endif

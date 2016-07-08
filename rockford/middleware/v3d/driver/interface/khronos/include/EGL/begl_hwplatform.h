/*=============================================================================
Broadcom Proprietary and Confidential. (c)2010 Broadcom.
All rights reserved.

Project  :  EGL driver
Module   :  Abstract H/W Interface

FILE DESCRIPTION
Defines an abstract interface that will be used to interact with platform h/w.
=============================================================================*/

#ifndef _BEGL_HWPLATFORM_H__
#define _BEGL_HWPLATFORM_H__

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
   BEGL_HW_PERF_NONE   = 0,
   BEGL_HW_PERF_START  = 1,
   BEGL_HW_PERF_RESET  = 2,
   BEGL_HW_PERF_STOP   = 4
} BEGL_HWPerfMonitorFlags;

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
   uint32_t           arg2;
   uint32_t           callbackParam;
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
   uint32_t    payload[15];
} BEGL_HWCallbackRecord;

typedef struct BEGL_HWPerfMonitorSettings
{
   uint32_t    hwBank;        /* 0 = no bank, 1 = 1st bank, 2 = 2nd bank         */
   uint32_t    memBank;       /* 0 = no bank, 1 = 1st bank, 2 = 2nd bank         */
   uint32_t    flags;         /* Bitwise or of flags in BEGL_HWPerfMonitorFlags  */
} BEGL_HWPerfMonitorSettings;

typedef struct BEGL_HWPerfMonitorData
{
   uint64_t    hwCounters[16];
   uint64_t    memCounters[2];
} BEGL_HWPerfMonitorData;

 /* The platform MUST provide an implementation of this interface in order that the EGL driver
 * can interact with platform hardware.
 */
typedef struct
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

   /* Setup or change performance monitoring */
   void (*SetPerformanceMonitor)(void *context, const BEGL_HWPerfMonitorSettings *settings);

   /* Get performance data */
   void (*GetPerformanceData)(void *context, BEGL_HWPerfMonitorData *data);

   /* Create a fence */
   void(*FenceOpen)(void *context, int *fd);

   /* Clock the pipeline, trigger all fences waiting at this point */
   void(*FenceSignal)(int fd);

   /* Close a fence */
   void(*FenceClose)(int fd);

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

#endif /* _BEGL_HWPLATFORM_H__ */

/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#ifndef EGLEXT_BRCM_H
#define EGLEXT_BRCM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "eglplatform.h"

#if KHRONOS_SUPPORT_INT64

typedef khronos_uint64_t EGLuint64BRCM;

/*  EGL_BRCM_performance_counters */
#ifndef EGL_BRCM_performance_counters
#define EGL_BRCM_performance_counters 1

#define EGL_ACQUIRE_COUNTERS_BRCM         0x33C0
#define EGL_RELEASE_COUNTERS_BRCM         0x33C1
#define EGL_START_COUNTERS_BRCM           0x33C2
#define EGL_STOP_COUNTERS_BRCM            0x33C3
#define EGL_NUM_COUNTER_GROUPS_BRCM       0x33C4
#define EGL_MAX_COUNTER_STRING_LEN_BRCM   0x33C5

   typedef EGLint     (EGLAPIENTRYP PFNEGLGETPERFCOUNTERCONSTANTBRCMPROC) (EGLenum pname);
   typedef EGLBoolean (EGLAPIENTRYP PFNEGLGETPERFCOUNTERGROUPINFOBRCMPROC) (EGLint group, EGLint nameStrSize, char *groupNameStr, EGLint *numCounters, EGLint *maxActiveCounters);
   typedef EGLBoolean (EGLAPIENTRYP PFNEGLGETPERFCOUNTERINFOBRCMPROC) (EGLint group, EGLint counter, EGLuint64BRCM *minValue, EGLuint64BRCM *maxValue, EGLuint64BRCM *denominator, EGLint nameStrSize, char *nameStr, EGLint unitStrSize, char *unitStr);
   typedef EGLBoolean (EGLAPIENTRYP PFNEGLSETPERFCOUNTINGBRCMPROC) (EGLenum pname);
   typedef EGLBoolean (EGLAPIENTRYP PFNEGLCHOOSEPERFCOUNTERSBRCMPROC) (EGLBoolean enable, EGLint group, EGLint numCounters, EGLint *counterList);
   typedef EGLBoolean (EGLAPIENTRYP PFNEGLGETPERFCOUNTERDATABRCMPROC) (EGLint dataBufferBytes, void *data, EGLint *bytesWritten, EGLBoolean resetCounters);

#ifdef EGL_EGLEXT_PROTOTYPES
   EGLAPI EGLint     EGLAPIENTRY eglGetPerfCounterConstantBRCM(EGLenum pname);
   EGLAPI EGLBoolean EGLAPIENTRY eglGetPerfCounterGroupInfoBRCM(EGLint group, EGLint nameStrSize, char *groupNameStr, EGLint *numCounters, EGLint *maxActiveCounters);
   EGLAPI EGLBoolean EGLAPIENTRY eglGetPerfCounterInfoBRCM(EGLint group, EGLint counter, EGLuint64BRCM *minValue, EGLuint64BRCM *maxValue, EGLuint64BRCM *denominator, EGLint nameStrSize, char *nameStr, EGLint unitStrSize, char *unitStr);
   EGLAPI EGLBoolean EGLAPIENTRY eglSetPerfCountingBRCM(EGLenum pname);
   EGLAPI EGLBoolean EGLAPIENTRY eglChoosePerfCountersBRCM(EGLBoolean enable, EGLint group, EGLint numCounters, EGLint *counterList);
   EGLAPI EGLBoolean EGLAPIENTRY eglGetPerfCounterDataBRCM(EGLint dataBufferBytes, void *data, EGLint *bytesWritten, EGLBoolean resetCounters);
#endif

#endif /* EGL_BRCM_performance_counters */

/*  EGL_BRCM_event_monitor */

#ifndef EGL_BRCM_event_monitor
#define EGL_BRCM_event_monitor 1

#define EGL_ACQUIRE_EVENTS_BRCM           0x33D0
#define EGL_RELEASE_EVENTS_BRCM           0x33D1
#define EGL_START_EVENTS_BRCM             0x33D2
#define EGL_STOP_EVENTS_BRCM              0x33D3
#define EGL_NUM_EVENT_TRACKS_BRCM         0x33D4
#define EGL_NUM_EVENTS_BRCM               0x33D5
#define EGL_MAX_EVENT_STRING_LEN_BRCM     0x33D6

   typedef EGLint     (EGLAPIENTRYP PFNEGLGETEVENTCONSTANTBRCMPROC) (EGLenum pname);
   typedef EGLBoolean (EGLAPIENTRYP PFNEGLGETEVENTTRACKINFOBRCMPROC) (EGLint track, EGLint nameStrSize, char *trackNameStr);
   typedef EGLBoolean (EGLAPIENTRYP PFNEGLGETEVENTINFOBRCMPROC) (EGLint event, EGLint nameStrSize, char *nameStr, EGLint *numDataFields);
   typedef EGLBoolean (EGLAPIENTRYP PFNEGLGETEVENTDATAFIELDINFOBRCMPROC) (EGLint event, EGLint field, EGLint nameStrSize, char *nameStr, EGLBoolean *isSigned, EGLint *numBytes);
   typedef EGLBoolean (EGLAPIENTRYP PFNEGLSETEVENTCOLLECTIONBRCMPROC) (EGLenum pname);
   typedef EGLBoolean (EGLAPIENTRYP PFNEGLGETEVENTDATABRCMPROC) (EGLint dataBufferBytes, void *data, EGLint *bytesWritten, EGLBoolean *overflowed, EGLuint64BRCM *timebase);

#ifdef EGL_EGLEXT_PROTOTYPES
   EGLAPI EGLint     EGLAPIENTRY eglGetEventConstantBRCM(EGLenum pname);
   EGLAPI EGLBoolean EGLAPIENTRY eglGetEventTrackInfoBRCM(EGLint track, EGLint nameStrSize, char *trackNameStr);
   EGLAPI EGLBoolean EGLAPIENTRY eglGetEventInfoBRCM(EGLint event, EGLint nameStrSize, char *nameStr, EGLint *numDataFields);
   EGLAPI EGLBoolean EGLAPIENTRY eglGetEventDataFieldInfoBRCM(EGLint event, EGLint field, EGLint nameStrSize, char *nameStr, EGLBoolean *isSigned, EGLint *numBytes);
   EGLAPI EGLBoolean EGLAPIENTRY eglSetEventCollectionBRCM(EGLenum pname);
   EGLAPI EGLBoolean EGLAPIENTRY eglGetEventDataBRCM(EGLint dataBufferBytes, void *data, EGLint *bytesWritten, EGLBoolean *overflowed, EGLuint64BRCM *timebase);
#endif

#endif /* EGL_BRCM_event_monitor */

#endif /* KHRONOS_SUPPORT_INT64 */

#ifdef __cplusplus
}
#endif

#endif

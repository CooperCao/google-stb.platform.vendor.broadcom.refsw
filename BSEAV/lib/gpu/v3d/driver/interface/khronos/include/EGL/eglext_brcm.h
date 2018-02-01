/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef EGLEXT_BRCM_H
#define EGLEXT_BRCM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "eglplatform.h"
#include "eglext.h"

/* We want this */
#ifndef EGL_EGLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES
#endif

#ifndef EGL_BRCM_driver_monitor
#define EGL_BRCM_driver_monitor 1
#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLBoolean EGLAPIENTRY eglInitDriverMonitorBRCM(EGLDisplay display, EGLint hw_bank, EGLint l3c_bank);
EGLAPI void EGLAPIENTRY eglGetDriverMonitorXMLBRCM(EGLDisplay display, EGLint bufSize, EGLint *length, char *xmlStats);
EGLAPI EGLBoolean EGLAPIENTRY eglTermDriverMonitorBRCM(EGLDisplay display);
#endif /* EGL_EGLEXT_PROTOTYPES */
typedef void (EGLAPIENTRYP PFNEGLINITDRIVERMONITORBRCMPROC)(EGLDisplay display, EGLint hw_bank, EGLint l3c_bank);
typedef void (EGLAPIENTRYP PFNEGLGETDRIVERMONITORXMLBRCMPROC)(EGLDisplay display, EGLint bufSize, EGLint *length, char *xmlStats);
typedef void (EGLAPIENTRYP PFNEGLTERMDRIVERMONITORBRCMPROC)(EGLDisplay display);
#endif

#ifndef EGL_BRCM_flush
#define EGL_BRCM_flush 0
#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI void EGLAPIENTRY eglFlushBRCM(void);
#endif /* EGL_EGLEXT_PROTOTYPES */
typedef void (EGLAPIENTRYP PFNEGLFLUSHBRCMPROC)(void);
#endif

#ifndef EGL_BRCM_image_wrap_bcg
#define EGL_BRCM_image_wrap_bcg 1
#define EGL_IMAGE_WRAP_BRCM_BCG 0x9993141

typedef struct {
   BEGL_BufferFormat format;

   uint16_t width;
   uint16_t height;

   int32_t stride; /* in bytes */

   void *storage;
   uint32_t offset;
} EGL_IMAGE_WRAP_BRCM_BCG_IMAGE_T;
#endif

#ifndef EGL_BRCM_image_update_control
#define EGL_BRCM_image_update_control 0
#define EGL_IMAGE_UPDATE_CONTROL_SET_MODE_BRCM           0x3260
#define EGL_IMAGE_UPDATE_CONTROL_CHANGED_REGION_BRCM     0x3261
#define EGL_IMAGE_UPDATE_CONTROL_SET_LOCK_STATE_BRCM     0x3262
#define EGL_IMAGE_UPDATE_CONTROL_ALWAYS_BRCM             0x3263
#define EGL_IMAGE_UPDATE_CONTROL_EXPLICIT_BRCM           0x3264
#define EGL_IMAGE_UPDATE_CONTROL_LOCK_BRCM               0x3265
#define EGL_IMAGE_UPDATE_CONTROL_UNLOCK_BRCM             0x3266

#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLBoolean EGLAPIENTRY eglImageUpdateParameterivBRCM(EGLDisplay dpy, EGLImageKHR image, EGLenum pname, const EGLint *params);
EGLAPI EGLBoolean EGLAPIENTRY eglImageUpdateParameteriBRCM(EGLDisplay dpy, EGLImageKHR image, EGLenum pname, EGLint param);
#endif /* EGL_EGLEXT_PROTOTYPES */
typedef EGLBoolean (EGLAPIENTRYP PFNEGLIMAGEUPDATEPARAMETERIVBRCMPROC) (EGLDisplay dpy, EGLImageKHR image, EGLenum pname, const EGLint *params);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLIMAGEUPDATEPARAMETERIBRCMPROC) (EGLDisplay dpy, EGLImageKHR image, EGLenum pname, EGLint param);
#endif

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

#ifdef GL_GLEXT_PROTOTYPES
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

#ifdef GL_GLEXT_PROTOTYPES
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

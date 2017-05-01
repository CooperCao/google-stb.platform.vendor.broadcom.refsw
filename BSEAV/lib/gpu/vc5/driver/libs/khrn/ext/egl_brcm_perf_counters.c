/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglext_brcm.h>
#include "../egl/egl_thread.h"
#include "../egl/egl_display.h"
#include "../common/khrn_process.h"
#include "../common/khrn_counters.h"
#include "libs/platform/bcm_perf_api.h"

#include "vcos.h"

#if EGL_BRCM_performance_counters

static EGLint egl_get_num_counter_groups()
{
   EGLint ret = 0;

   ret = bcm_sched_get_num_counter_groups();
   ret += khrn_driver_get_num_counter_groups();

   return ret;
}

EGLAPI EGLint EGLAPIENTRY eglGetPerfCounterConstantBRCM(
   EGLenum pname
   )
{
   EGLint ret = -1;
   EGLint error = EGL_SUCCESS;

   if (!egl_initialized(0, false))
      return ret;

   switch (pname)
   {
   case EGL_MAX_COUNTER_STRING_LEN_BRCM:
      ret = 64;
      break;
   case EGL_NUM_COUNTER_GROUPS_BRCM:
      ret = egl_get_num_counter_groups();
      break;
   default:
      ret = -1;
      error = EGL_BAD_PARAMETER;
      break;
   }

   egl_thread_set_error(error);
   return ret;
}

EGLAPI EGLBoolean EGLAPIENTRY eglGetPerfCounterGroupInfoBRCM(
   EGLint group,
   EGLint nameStrSize,
   char *groupNameStr,
   EGLint *numCounters,
   EGLint *maxActiveCounters
   )
{
   bool                                   ok = false;
   struct bcm_sched_counter_group_desc    desc;

   if (!egl_initialized(0, false))
      return EGL_FALSE;

   ok = bcm_sched_enumerate_group_counters(group, &desc);
   if (!ok)
      // Check driver counters
      ok = khrn_driver_enumerate_group_counters(group, &desc);

   if (ok)
   {
      if (nameStrSize > 0 && groupNameStr != NULL)
         strncpy(groupNameStr, desc.name, nameStrSize);

      if (numCounters != NULL)
         *numCounters = desc.total_counters;

      if (maxActiveCounters != NULL)
         *maxActiveCounters = desc.max_active_counters;
   }
   else
   {
      egl_thread_set_error(EGL_BAD_PARAMETER);
      return EGL_FALSE;
   }

   egl_thread_set_error(EGL_SUCCESS);
   return EGL_TRUE;
}

EGLAPI EGLBoolean EGLAPIENTRY eglGetPerfCounterInfoBRCM(
   EGLint group,
   EGLint counter,
   EGLuint64BRCM *minValue,
   EGLuint64BRCM *maxValue,
   EGLuint64BRCM *denominator,
   EGLint nameStrSize,
   char *nameStr,
   EGLint unitStrSize,
   char *unitStr
   )
{
   bool                                   ok = false;
   struct bcm_sched_counter_group_desc    desc;

   if (!egl_initialized(0, false))
      return EGL_FALSE;

   ok = bcm_sched_enumerate_group_counters(group, &desc);

   if (!ok)
      ok = khrn_driver_enumerate_group_counters(group, &desc);

   if (ok)
   {
      if ((uint32_t)counter >= desc.total_counters)
      {
         egl_thread_set_error(EGL_BAD_PARAMETER);
         return EGL_FALSE;
      }

      if (minValue != NULL)
         *minValue = desc.counters[counter].min_value;

      if (maxValue != NULL)
         *maxValue = desc.counters[counter].max_value;

      if (denominator != NULL)
         *denominator = desc.counters[counter].denominator;

      if (nameStr != NULL && nameStrSize > 0)
         strncpy(nameStr, desc.counters[counter].name, nameStrSize);

      if (unitStr != NULL && unitStrSize > 0)
         strncpy(unitStr, desc.counters[counter].unit_name, unitStrSize);
   }
   else
   {
      if (minValue != NULL)
         *minValue = 0;

      if (maxValue != NULL)
         *maxValue = 0;

      if (denominator != NULL)
         *denominator = 1;

      egl_thread_set_error(EGL_BAD_PARAMETER);
      return EGL_FALSE;
   }

   egl_thread_set_error(EGL_SUCCESS);
   return EGL_TRUE;
}

EGLAPI EGLBoolean EGLAPIENTRY eglSetPerfCountingBRCM(
   EGLenum pname
   )
{
   bool                          ok = false;
   enum bcm_sched_counter_state  s = BCM_CTR_STOP;
   EGL_THREAD_T                  *thread = egl_thread_get();

   if (!egl_initialized(0, false))
      return EGL_FALSE;

   if (thread == NULL)
      return EGL_FALSE;

   switch (pname)
   {
   case EGL_ACQUIRE_COUNTERS_BRCM:  s = BCM_CTR_ACQUIRE; break;
   case EGL_RELEASE_COUNTERS_BRCM:  s = BCM_CTR_RELEASE; break;
   case EGL_START_COUNTERS_BRCM:    s = BCM_CTR_START;   break;
   case EGL_STOP_COUNTERS_BRCM:     s = BCM_CTR_STOP;    break;
   default:                         egl_thread_set_error(EGL_BAD_PARAMETER); return EGL_FALSE;
   }

   ok = bcm_sched_set_counter_collection(s);
   if (!ok)
   {
      egl_thread_set_error(EGL_BAD_ACCESS);
      return EGL_FALSE;
   }

   // All good - adjust the acquired state. We will use this in other APIs to
   // determine the correct error to set.
   if (pname == EGL_ACQUIRE_COUNTERS_BRCM)
       thread->perf_counters_acquired = true;
   else if (pname == EGL_RELEASE_COUNTERS_BRCM)
      thread->perf_counters_acquired = false;

   egl_thread_set_error(EGL_SUCCESS);
   return EGL_TRUE;
}

EGLAPI EGLBoolean EGLAPIENTRY eglChoosePerfCountersBRCM(
   EGLBoolean enable,
   EGLint group,
   EGLint numCounters,
   EGLint *counterList
   )
{
   bool                                    ok = false;
   struct bcm_sched_group_counter_selector sel;
   uint32_t                                i;
   EGL_THREAD_T                            *thread = egl_thread_get();

   if (!egl_initialized(0, false))
      return EGL_FALSE;

   if (thread == NULL || !thread->perf_counters_acquired)
   {
      egl_thread_set_error(EGL_BAD_ACCESS);
      return EGL_FALSE;
   }

   if (group >= egl_get_num_counter_groups() || numCounters >= V3D_MAX_COUNTERS_PER_GROUP)
   {
      egl_thread_set_error(EGL_BAD_PARAMETER);
      return EGL_FALSE;
   }

   sel.group_index = group;
   sel.enable = enable;

   if (numCounters == 0 || counterList == NULL)
   {
      /* If <numCounters> is 0 or counterList is NULL, all the counters in
       * the group are affected. When the number of enabled counters would exceed the
       * maximum allowable for a group, only the first maxActiveCounters will be
       * enabled.
       */
      struct bcm_sched_counter_group_desc desc;
      bool                                ok;

      ok = bcm_sched_enumerate_group_counters(group, &desc);

      if (!ok)
         ok = khrn_driver_enumerate_group_counters(group, &desc);

      if (ok)
      {
         if (enable)
            sel.num_counters = desc.max_active_counters;
         else
            sel.num_counters = desc.total_counters;

         for (i = 0; i < sel.num_counters; i++)
            sel.counters[i] = i;
      }
   }
   else
   {
      sel.num_counters = numCounters;
      for (i = 0; i < (uint32_t)numCounters; i++)
         sel.counters[i] = counterList[i];
   }

   ok = bcm_sched_select_group_counters(&sel);

   if (!ok)
      ok = khrn_driver_select_group_counters(&sel);

   if (!ok)
   {
      egl_thread_set_error(EGL_BAD_PARAMETER);
      return EGL_FALSE;
   }

   egl_thread_set_error(EGL_SUCCESS);
   return EGL_TRUE;
}

EGLAPI EGLBoolean EGLAPIENTRY eglGetPerfCounterDataBRCM(
   EGLint dataBufferBytes,
   void *data,
   EGLint *bytesWritten,
   EGLBoolean resetCounters
   )
{
   uint32_t countersRead = 0;
   uint32_t maxCounters;
   EGL_THREAD_T *thread = egl_thread_get();

   if (!egl_initialized(0, false))
      return EGL_FALSE;

   if (bytesWritten)
      *bytesWritten = 0;

   if (thread == NULL || !thread->perf_counters_acquired)
   {
      egl_thread_set_error(EGL_BAD_ACCESS);
      return EGL_FALSE;
   }

   maxCounters = dataBufferBytes / sizeof(struct bcm_sched_counter);

   countersRead = bcm_sched_get_counters((struct bcm_sched_counter *)data,
                                          maxCounters, resetCounters);

   if ((data != NULL) && (maxCounters - countersRead > 0))
      countersRead += khrn_driver_get_counters(((struct bcm_sched_counter *)data) + countersRead,
                                                maxCounters, resetCounters);
   else
      countersRead += khrn_driver_get_counters(((struct bcm_sched_counter *)data),
                                                0, resetCounters);

   if (bytesWritten)
      *bytesWritten = countersRead * sizeof(struct bcm_sched_counter);

   egl_thread_set_error(EGL_SUCCESS);
   return EGL_TRUE;
}

#endif

/*=============================================================================
 Broadcom Proprietary and Confidential. (c)2010 Broadcom.
 All rights reserved.

 Project  :  khronos
 Module   :  EGL display

 FILE DESCRIPTION
 EGL server-side display management.
 =============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_client_platform.h"
#include "interface/khronos/common/khrn_api_interposer.h"

#include "interface/khronos/include/EGL/egl.h"

#include "middleware/khronos/common/khrn_hw.h"
#include "middleware/khronos/common/khrn_image.h"
#include "middleware/khronos/egl/egl_disp.h"
#include "middleware/khronos/egl/egl_platform.h"

static MEM_HANDLE_T disp_pop(EGL_DISP_THREAD_STATE_T *thread_state);

/* One thread per eglWindowSurface.
 * The display thread pulls finished frames from the display queue and posts them to the
 * platform display system.
 */
static void *display_thread_func(void *arg)
{
   BEGL_DriverInterfaces   *driverInterfaces = BEGL_GetDriverInterfaces();
   EGL_DISP_THREAD_STATE_T *thread_state     = (EGL_DISP_THREAD_STATE_T *)arg;
   uint32_t                lastDisplayTime   = vcos_getmicrosecs();
   uint32_t                elapsed;
   bool                    allDone = false;
   bool                    refreshRateReported = true;
   MEM_HANDLE_T            lastDisplayedBuffer = MEM_INVALID_HANDLE;

   vcos_assert(driverInterfaces != NULL);
   vcos_assert(driverInterfaces->displayInterface != NULL);
   vcos_assert(driverInterfaces->displayInterface->BufferDisplay != NULL);
   vcos_assert(driverInterfaces->displayInterface->WindowUndisplay != NULL);

   vcos_semaphore_post(&thread_state->running);

   while (!allDone)
   {
      /* Get a rendered buffer to display - will block if none are yet available */
      MEM_HANDLE_T   buffer = disp_pop(thread_state);

      if (buffer != MEM_INVALID_HANDLE)
      {
         KHRN_IMAGE_T            *image = (KHRN_IMAGE_T *)mem_lock(buffer, NULL);
         BEGL_BufferDisplayState bufferDisplayState;

         /* Prepare to display the buffer */
         memset(&bufferDisplayState, 0, sizeof(BEGL_BufferDisplayState));

         bufferDisplayState.buffer = (BEGL_BufferHandle)image->opaque_buffer_handle;
         vcos_assert(image->window_state != NULL);
         bufferDisplayState.windowState = *image->window_state;

         /* Should the platform layer wait for the vSync after display */
         bufferDisplayState.waitVSync = (image->swap_interval != 0);

         /* Handle swap intervals > 1 by simply sleeping :-) */
         if (image->swap_interval > 1)
         {
            uint32_t refreshRateMilliHertz = 59940;
            uint32_t usPerRefresh;
            uint32_t delayWantedUs;

            if (refreshRateReported && driverInterfaces->displayInterface->WindowGetInfo)
            {
               BEGL_WindowInfo info;
               driverInterfaces->displayInterface->WindowGetInfo(driverInterfaces->displayInterface->context,
                                                                bufferDisplayState.windowState.window,
                                                                BEGL_WindowInfoRefreshRate, &info);
               if (info.refreshRateMilliHertz != 0)
                  refreshRateMilliHertz = info.refreshRateMilliHertz;
               else
                  refreshRateReported = false;  /* Don't bother asking again */
            }

            usPerRefresh = 1000000000 / refreshRateMilliHertz;
            delayWantedUs = image->swap_interval * usPerRefresh - (usPerRefresh / 2);   /* Sleep until roughly half the last frame */

            elapsed = vcos_getmicrosecs() - lastDisplayTime;
            if (elapsed < delayWantedUs)
               vcos_sleep((delayWantedUs - elapsed) / 1000);
         }

         /* Check that we're not sending the same buffer for display again. This could lock the
            sequencer. */
         if (buffer != lastDisplayedBuffer)
         {
            /* Ask the platform layer to display the buffer. If waitVSync is set, this will block until the
            vSync has fired. */
            driverInterfaces->displayInterface->BufferDisplay(driverInterfaces->displayInterface->context, &bufferDisplayState);

            khrn_remote_buffer_display_stamp();

            lastDisplayTime = vcos_getmicrosecs();
         }
         else
         {
            /* This is an error - something is wrong in the driver if this happens */
            fprintf(stderr, "ERROR: V3D driver is displaying the same buffer twice\n");
         }
      }

      if (vcos_semaphore_wait_timeout(&thread_state->running, 0) == VCOS_SUCCESS)
         allDone = true;
   }

   driverInterfaces->displayInterface->WindowUndisplay(driverInterfaces->displayInterface->context,
                                                       thread_state->platform_state);
   return NULL;
}

/* Fifo "constructor" */
static void init_fifo(EGL_DISP_FIFO_T *fifo)
{
   uint32_t i;

   fifo->push_at  = 0;
   fifo->pop_from = 0;

   for (i = 0; i < EGL_DISP_MAX_BUFFERS; ++i)
      fifo->buffer[i] = MEM_INVALID_HANDLE;

   vcos_mutex_create(&fifo->mutex, "Fifo mutex");
   vcos_semaphore_create(&fifo->semaphore, "Fifo semaphore", 0);
}

/* Fifo "destructor" */
static void term_fifo(EGL_DISP_FIFO_T *fifo)
{
   vcos_mutex_delete(&fifo->mutex);
   vcos_semaphore_delete(&fifo->semaphore);
}

void egl_disp_create_display_thread(EGL_DISP_THREAD_STATE_T *thread_state, BEGL_WindowState *platformState)
{
   vcos_semaphore_create(&thread_state->running, "Termination signal", 0);
   thread_state->platform_state = platformState;
   init_fifo(&thread_state->display_fifo);
   vcos_thread_create(&thread_state->thread, "EGL Display thread", NULL, display_thread_func, (void*)thread_state);

   /* Wait for the thread to really start */
   vcos_semaphore_wait(&thread_state->running);
}

extern void *client_lock_api(void);
extern void client_unlock_api(void);
void egl_disp_destroy_display_thread(EGL_DISP_THREAD_STATE_T *thread_state)
{
   void *ret = NULL;
   EGL_DISP_FIFO_T *fifo = &thread_state->display_fifo;

   vcos_semaphore_post(&thread_state->running);

   /* push a NULL to wake the display thread */
   vcos_mutex_lock(&fifo->mutex);
   fifo->buffer[fifo->push_at] = MEM_INVALID_HANDLE;

   fifo->push_at++;
   if (fifo->push_at >= EGL_DISP_MAX_BUFFERS)
      fifo->push_at = 0;

   vcos_mutex_unlock(&fifo->mutex);
   vcos_semaphore_post(&fifo->semaphore);

#ifdef ANDROID
   /* In Android, this can trigger recursion into the driver, so unlock the client mutex while we wait */
   client_unlock_api();
#endif
   /* Wait for thread to die */
   vcos_thread_join(&thread_state->thread, &ret);
#ifdef ANDROID
   /* In Android, this can trigger recursion into the driver, so unlock the client mutex while we wait */
   client_lock_api();
#endif
   vcos_semaphore_delete(&thread_state->running);

   term_fifo(&thread_state->display_fifo);
}

/* Add a finished frame to the display queue */
void egl_disp_push(MEM_HANDLE_T handle)
{
   KHRN_IMAGE_T      *image = (KHRN_IMAGE_T *)mem_lock(handle, NULL);
   EGL_DISP_FIFO_T   *fifo  = &((EGL_DISP_THREAD_STATE_T *)image->display_thread_state)->display_fifo;

   vcos_mutex_lock(&fifo->mutex);

   mem_unlock(handle);

   fifo->buffer[fifo->push_at] = handle;

   fifo->push_at++;
   if (fifo->push_at >= EGL_DISP_MAX_BUFFERS)
      fifo->push_at = 0;

   vcos_mutex_unlock(&fifo->mutex);

   vcos_semaphore_post(&fifo->semaphore);
}

/* Get a finished frame from the display queue, or block until one is available. */
static MEM_HANDLE_T disp_pop(EGL_DISP_THREAD_STATE_T *thread_state)
{
   MEM_HANDLE_T      ret;
   EGL_DISP_FIFO_T   *fifo = &thread_state->display_fifo;

   if (vcos_semaphore_wait_timeout(&fifo->semaphore, 1000) == VCOS_EAGAIN)
      return MEM_INVALID_HANDLE;

   vcos_mutex_lock(&fifo->mutex);

   ret = fifo->buffer[fifo->pop_from];
   fifo->buffer[fifo->pop_from] = MEM_INVALID_HANDLE;
   fifo->pop_from++;
   if (fifo->pop_from >= EGL_DISP_MAX_BUFFERS)
      fifo->pop_from = 0;

   vcos_mutex_unlock(&fifo->mutex);

   return ret;
}

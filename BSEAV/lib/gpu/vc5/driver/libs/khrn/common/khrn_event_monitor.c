/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Implementation of Broadcom extension EGL_BRCM_event_monitor for driver events
=============================================================================*/

#include "khrn_event_monitor.h"
#include "libs/platform/bcm_perf_structs.h"
#include "libs/platform/bcm_sched_api.h"
#include "libs/platform/bcm_perf_api.h"
#include "khrn_process.h"

// This needs to be 4 bytes aligned
#define KHRN_EVENT_BUFFER_BYTES                 (1 * 1024 * 1024)

#define KHRN_DRIVER_EVENT_MONITOR_MAX_FIELDS    2

typedef struct KHRN_DRIVER_EVENT_FULL_DESC
{
   KHRN_DRIVER_EVENT_DESC           desc;
   KHRN_DRIVER_EVENT_FIELD_DESC     field_descs[KHRN_DRIVER_EVENT_MONITOR_MAX_FIELDS];
} KHRN_DRIVER_EVENT_FULL_DESC;

typedef struct KHRN_DRIVER_EVENTBUFFER
{
   uint8_t     buffer[KHRN_EVENT_BUFFER_BYTES];
   size_t      capacity_bytes;
   size_t      bytes_used;
   void        *write_pointer;
   void        *read_pointer;
   bool         overflow;
} KHRN_DRIVER_EVENTBUFFER;

typedef struct KHRN_DRIVER_EVENT_MONITOR
{
   /* track and event ids are unique and carry on from the list of scheduler events */
   uint32_t                      khrn_driver_event_monitor_first_event_id;
   uint32_t                      khrn_driver_event_monitor_first_track_id;

   /* Scheduler and driver events need the same time base for time ordering */
   int64_t                       time_diff_from_sched_to_driver;

   KHRN_DRIVER_EVENT_TRACK_DESC  driver_event_track_desc[KHRN_DRIVER_EVENT_MONITOR_NUM_TRACKS];
   KHRN_DRIVER_EVENT_FULL_DESC   driver_event_desc[KHRN_DRIVER_EVENT_MONITOR_NUM_EVENTS];
   KHRN_DRIVER_EVENTBUFFER       driver_event_buffer;
   bool                          driver_collect_events;
} KHRN_DRIVER_EVENT_MONITOR;

static KHRN_DRIVER_EVENT_MONITOR                s_event_monitor;
static bool                                     s_initialised = false;
static uint32_t                                 s_track_next_id[KHRN_DRIVER_EVENT_MONITOR_NUM_TRACKS];

/* --------------------------------------------------------------------------------------------- */
/*                                   Helper functions                                            */

static void init_event_buffer()
{
   // If the event buffer has not been created yet
   if (s_event_monitor.driver_event_buffer.capacity_bytes == 0)
   {
      s_event_monitor.driver_event_buffer.capacity_bytes = KHRN_EVENT_BUFFER_BYTES;
      s_event_monitor.driver_event_buffer.bytes_used = 0;
      s_event_monitor.driver_event_buffer.read_pointer = s_event_monitor.driver_event_buffer.buffer;
      s_event_monitor.driver_event_buffer.write_pointer = s_event_monitor.driver_event_buffer.buffer;
      s_event_monitor.driver_event_buffer.overflow = false;
      s_event_monitor.driver_collect_events = false;
   }
}

static void init_event_monitor()
{
   uint64_t timebase;
   bool overflow;
   memset(&s_event_monitor, 0, sizeof(s_event_monitor));

   // Track KHRN_DRIVER_TRACK_DRIVER
   strncpy(s_event_monitor.driver_event_track_desc[KHRN_DRIVER_TRACK_DRIVER].name, "Driver", V3D_MAX_EVENT_STRING_LEN);
   strncpy(s_event_monitor.driver_event_track_desc[KHRN_DRIVER_TRACK_IMAGE_CONV].name, "ImageConv", V3D_MAX_EVENT_STRING_LEN);

   strncpy(s_event_monitor.driver_event_desc[KHRN_DRIVER_EVENT_CPU_CACHE_FLUSH].desc.name, "Cache Flush", V3D_MAX_EVENT_STRING_LEN);
   s_event_monitor.driver_event_desc[KHRN_DRIVER_EVENT_CPU_CACHE_FLUSH].desc.num_data_fields = 0;

   strncpy(s_event_monitor.driver_event_desc[KHRN_DRIVER_EVENT_FENCE_WAIT].desc.name, "Fence Wait", V3D_MAX_EVENT_STRING_LEN);
   s_event_monitor.driver_event_desc[KHRN_DRIVER_EVENT_FENCE_WAIT].desc.num_data_fields = 0;

   strncpy(s_event_monitor.driver_event_desc[KHRN_DRIVER_EVENT_EGL_IMAGE_UPDATE].desc.name, "EGLImage Update", V3D_MAX_EVENT_STRING_LEN);
   s_event_monitor.driver_event_desc[KHRN_DRIVER_EVENT_EGL_IMAGE_UPDATE].desc.num_data_fields = 0;

   strncpy(s_event_monitor.driver_event_desc[KHRN_DRIVER_EVENT_GENERATE_MIPMAPS].desc.name, "Generate Mipmaps", V3D_MAX_EVENT_STRING_LEN);
   s_event_monitor.driver_event_desc[KHRN_DRIVER_EVENT_GENERATE_MIPMAPS].desc.num_data_fields = 0;

   // Retrieve the num of scheduler tracks and events to know what should be the first driver track
   // and event ids
   s_event_monitor.khrn_driver_event_monitor_first_event_id = bcm_sched_get_num_events();
   s_event_monitor.khrn_driver_event_monitor_first_track_id = bcm_sched_get_num_event_tracks();

   // Initialise a buffer to store the events before they get read
   init_event_buffer();

   // Calculate the time difference between the scheduler and the driver time system
   // The driver events should be on the same time base as the scheduler events
   bcm_sched_poll_event_timeline(0, NULL, &overflow, &timebase);
   s_event_monitor.time_diff_from_sched_to_driver = timebase - vcos_getmicrosecs64();

   for (uint32_t i = 0; i < KHRN_DRIVER_EVENT_MONITOR_NUM_TRACKS; i++)
      s_track_next_id[i] = 0;

   s_initialised = true;
}
/* --------------------------------------------------------------------------------------------- */


/* --------------------------------------------------------------------------------------------- */
/*                 EGLBRCM_event_monitor implementation for driver events                        */

uint32_t khrn_driver_get_num_event_tracks()
{
   return KHRN_DRIVER_EVENT_MONITOR_NUM_TRACKS;
}

uint32_t khrn_driver_get_num_events()
{
   return KHRN_DRIVER_EVENT_MONITOR_NUM_EVENTS;
}

bool khrn_driver_describe_event_track(uint32_t track_index,
                                    KHRN_DRIVER_EVENT_TRACK_DESC *track_desc)
{
   bool ret = false;

   if (!s_initialised)
      init_event_monitor();

   track_index -= s_event_monitor.khrn_driver_event_monitor_first_track_id;

   if (track_index < KHRN_DRIVER_EVENT_MONITOR_NUM_TRACKS)
   {
      strncpy(track_desc->name, s_event_monitor.driver_event_track_desc[track_index].name, V3D_MAX_EVENT_STRING_LEN);
      ret = true;
   }
   else
      ret = false;

   return ret;
}

bool khrn_driver_describe_event(uint32_t event_index,
                              KHRN_DRIVER_EVENT_DESC *event_desc)
{
   bool ret = false;

   if (!s_initialised)
      init_event_monitor();

   event_index -= s_event_monitor.khrn_driver_event_monitor_first_event_id;

   if (event_index < KHRN_DRIVER_EVENT_MONITOR_NUM_EVENTS)
   {
      event_desc->num_data_fields = s_event_monitor.driver_event_desc[event_index].desc.num_data_fields;
      strncpy(event_desc->name, s_event_monitor.driver_event_desc[event_index].desc.name, V3D_MAX_EVENT_STRING_LEN);
      ret = true;
   }
   else
      ret = false;

   return ret;
}

bool khrn_driver_describe_event_data(uint32_t event_index,
                                    uint32_t field_index,
                                    KHRN_DRIVER_EVENT_FIELD_DESC *field_desc)
{
   bool ret = false;

   if (!s_initialised)
      init_event_monitor();

   event_index -= s_event_monitor.khrn_driver_event_monitor_first_event_id;

   if (event_index < KHRN_DRIVER_EVENT_MONITOR_NUM_EVENTS)
   {
      if (field_index < s_event_monitor.driver_event_desc[event_index].desc.num_data_fields)
      {
         field_desc->data_type = s_event_monitor.driver_event_desc[event_index].field_descs[field_index].data_type;
         strncpy(field_desc->name, s_event_monitor.driver_event_desc[event_index].field_descs[field_index].name,
               V3D_MAX_EVENT_STRING_LEN);

         ret = true;
      }
      else
         ret = false;
   }
   else
      ret = false;

   return ret;
}

uint32_t khrn_driver_poll_event_timeline(size_t    max_event_buffer_bytes,
                                       void        *event_buffer,
                                       bool        *lost_data,
                                       uint64_t    *timestamp_us)
{
   KHRN_DRIVER_EVENTBUFFER *ev_buffer = &s_event_monitor.driver_event_buffer;
   uint32_t bytes  = 0;

   if (!s_initialised)
      init_event_monitor();

   if (lost_data)
      *lost_data = ev_buffer->overflow;

   if (max_event_buffer_bytes == 0 || event_buffer == NULL)
   {
      bytes = ev_buffer->bytes_used;
   }
   else
   {
      /* Copy the data out */
      uint32_t    bytesToCopy, bytesToEnd;
      uintptr_t   end = (uintptr_t)ev_buffer->buffer + ev_buffer->capacity_bytes;

      bytes = bytesToCopy = ev_buffer->bytes_used;
      if (max_event_buffer_bytes < bytesToCopy)
         bytesToCopy = max_event_buffer_bytes;

      bytesToEnd = end - (uintptr_t)ev_buffer->read_pointer;
      if (bytesToEnd > bytesToCopy)
      {
         /* No wrapping */
         memcpy(event_buffer, ev_buffer->read_pointer, bytesToCopy);
         ev_buffer->read_pointer = (void*)((uintptr_t)ev_buffer->read_pointer + bytesToCopy);
         ev_buffer->bytes_used -= bytesToCopy;
      }
      else
      {
         /* Buffer wraps */
         memcpy(event_buffer, ev_buffer->read_pointer, bytesToEnd);
         ev_buffer->read_pointer = ev_buffer->buffer;
         ev_buffer->bytes_used -= bytesToEnd;
         event_buffer = (void*)((uintptr_t)event_buffer + bytesToEnd);
         bytesToCopy -= bytesToEnd;

         memcpy(event_buffer, ev_buffer->read_pointer, bytesToCopy);
         ev_buffer->read_pointer = (void*)((uintptr_t)ev_buffer->read_pointer + bytesToCopy);
         ev_buffer->bytes_used -= bytesToCopy;
      }
   }

   if (timestamp_us)
      *timestamp_us = vcos_getmicrosecs64();

   return bytes;
}

void khrn_driver_poll_set_event_collection(bool collect_events)
{
   if (!s_initialised)
      init_event_monitor();

   s_event_monitor.driver_collect_events = collect_events;
}

uint32_t khrn_driver_track_next_id(KHRN_DRIVER_TRACK_ID track)
{
   return s_track_next_id[track]++;
}

/* --------------------------------------------------------------------------------------------- */


/* --------------------------------------------------------------------------------------------- */
/*                                    Driver interface to add events                             */

static bool add_32(uint32_t data)
{
   KHRN_DRIVER_EVENTBUFFER *ev_buffer = &s_event_monitor.driver_event_buffer;
   uintptr_t          end = (uintptr_t)ev_buffer->buffer + ev_buffer->capacity_bytes;
   uint32_t           bytesToEnd = end - (uintptr_t)ev_buffer->write_pointer;

   if (ev_buffer->capacity_bytes - ev_buffer->bytes_used < 4)
      return false;

   if (bytesToEnd < 4)
   {
      // Checking that buffer size is 4 bytes aligned
      assert(bytesToEnd == 0);
      ev_buffer->write_pointer = ev_buffer->buffer;
   }

   *((uint32_t*)ev_buffer->write_pointer) = data;
   ev_buffer->write_pointer = (void*)((uintptr_t)ev_buffer->write_pointer + 4);
   ev_buffer->bytes_used += 4;

   return true;
}

static bool add_64(uint64_t data)
{
   KHRN_DRIVER_EVENTBUFFER *ev_buffer = &s_event_monitor.driver_event_buffer;
   uintptr_t          end = (uintptr_t)ev_buffer->buffer + ev_buffer->capacity_bytes;
   uint32_t           bytesToEnd = end - (uintptr_t)ev_buffer->write_pointer;

   if (ev_buffer->capacity_bytes - ev_buffer->bytes_used < 8)
      return false;

   if (bytesToEnd < 8)
   {
      uint32_t *dw = (uint32_t*)&data;
      add_32(dw[0]);
      add_32(dw[1]);
   }
   else
   {
      *((uint64_t*)ev_buffer->write_pointer) = data;
      ev_buffer->write_pointer = (void*)((uintptr_t)ev_buffer->write_pointer + 8);
      ev_buffer->bytes_used += 8;
   }

   return true;
}

bool khrn_driver_add_event(uint32_t                track_index,
                           uint32_t                rec_event_id,
                           KHRN_DRIVER_EVENT_ID    event_index,
                           KHRN_DRIVER_EVENT_TYPE  event_type)
{
   KHRN_DRIVER_EVENTBUFFER *ev_buffer = &s_event_monitor.driver_event_buffer;
   bool                    ok = true;

   if (s_event_monitor.driver_collect_events)
   {
      if (ev_buffer->capacity_bytes - ev_buffer->bytes_used < 24)
      {
         /* No room for this event */
         ev_buffer->overflow = true;
         ok = false;
      }
      else
      {

         ok = ok && add_64(vcos_getmicrosecs64() + s_event_monitor.time_diff_from_sched_to_driver);
         ok = ok && add_32(track_index + s_event_monitor.khrn_driver_event_monitor_first_track_id);
         ok = ok && add_32(rec_event_id);
         ok = ok && add_32(event_index + s_event_monitor.khrn_driver_event_monitor_first_event_id);
         ok = ok && add_32(event_type);

         assert(ok); /* Should be ok as we check for space above */

         if (!ok)
            ev_buffer->overflow = true;
      }
   }
   else
      ok = false;

   return ok;
}
/* --------------------------------------------------------------------------------------------- */

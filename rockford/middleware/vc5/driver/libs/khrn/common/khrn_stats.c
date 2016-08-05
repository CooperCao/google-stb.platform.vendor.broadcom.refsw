/*=============================================================================
Broadcom Proprietary and Confidential. (c)2010 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Stats

FILE DESCRIPTION
Gather performance statistics.
=============================================================================*/

#include "khrn_int_common.h"
#include "khrn_stats.h"
#include "libs/util/snprintf.h"
#include <stdio.h>

#ifndef KHRN_STATS_ENABLE
void khrn_stats_reset(void, bool stone)
{
}
void khrn_stats_get_human_readable(char *buffer, uint32_t len, bool reset)
{
   snprintf(buffer, len, "Stats recording is disabled\n");
}
#else

static void get_human_readable(char *buffer, uint32_t len, bool reset, bool stone, bool timers);

KHRN_STATS_RECORD_T khrn_stats_global_record =
{
   {
      {KHRN_STATS_SWAP_BUFFERS, 0, "Swap_Buffers", "", false },
      {KHRN_STATS_ROUND_TRIP, 0, "Round_Trip", "", false },
      {KHRN_STATS_ROUND_TRIP_STONE, 0, "Round_Trip", "", true },
      {KHRN_STATS_INTERNAL_FLUSH, 0, "Flush", "", false },
      {KHRN_STATS_ANDROID_APP_FRAMES, 0, "Android_App_Frames", "", false },
      {KHRN_STATS_DRAW_CALL_COUNT, 0, "Draw_Calls", "", false },
      {KHRN_STATS_DRAW_CALL_FASTPATH, 0, "Draw_Calls_FastPath", "", false },
      {KHRN_STATS_RENDER_UNMERGED, 0, "Render_Unmerged", "", false },
      {KHRN_STATS_RENDER_MERGED, 0, "Render_Merged", "", false },
      {KHRN_STATS_SHADER_CACHE_MISS, 0, "Shader_Cache_Miss", "" , false},
   },
   {
      {KHRN_STATS_GL, 0, "GL_Call", "", false },
      {KHRN_STATS_VG, 0, "VG_Call", "", false },
      {KHRN_STATS_IMAGE, 0, "Image_Conv", "", true },
      {KHRN_STATS_WAIT, 0, "Wait", "", true },
      {KHRN_STATS_HW_RENDER, 0, "HW_Render_Time", "", true },
      {KHRN_STATS_HW_BIN, 0, "HW_Bin_Time", "", true },
      {KHRN_STATS_COMPILE, 0, "Compile", "", false },
      {KHRN_STATS_DRAW_CALL, 0, "Draw_Call", "", false },
      {KHRN_STATS_RECV_BULK, 0, "Recv_Bulk", "", false },
      {KHRN_STATS_SEND_BULK, 0, "Send_Bulk", "", false },
      {KHRN_STATS_RECV_BULK_STONE, 0, "Recv_Bulk", "", true },
      {KHRN_STATS_SEND_BULK_STONE, 0, "Send_Bulk", "", true },
      {KHRN_STATS_SEND_CTRL, 0, "Send_Ctrl", "", false },
      {KHRN_STATS_SEND_CTRL_STONE, 0, "Send_Ctrl", "", true },
      {KHRN_STATS_DISPATCH_TASK, 0, "Dispatch_Task", "", true },
      {KHRN_STATS_BUFFER_DATA, 0, "Buffer_Data", "", false },
      {KHRN_STATS_VERTEX_CACHE_DATA, 0, "Vertex_Cache_Data", "", false },
      {KHRN_STATS_UNMEM_DATA, 0, "Unmem_Data", "", false },
      {KHRN_STATS_IMAGE_DATA, 0, "Image_Data", "", false },
      {KHRN_STATS_FETCH_AND_DISPATCH, 0, "Fetch_Dispatch", "", true },
   }
};

void khrn_stats_reset(bool clear_in_thing, bool stone)
{
   int i;
   if(stone)
      khrn_stats_global_record.reset_time_stone = khrn_stats_getmicrosecs();
   else
      khrn_stats_global_record.reset_time = khrn_stats_getmicrosecs();
   for (i = 0; i < KHRN_STATS_THING_MAX; i++)
   {
      assert(khrn_stats_global_record.thing_time[i].id == i);
      if(khrn_stats_global_record.thing_time[i].stone == stone)
      {
         if (clear_in_thing)
            khrn_stats_global_record.in_thing[i] = false;
         khrn_stats_global_record.thing_time[i].value = 0;
      }
   }
   for (i = 0; i < KHRN_STATS_EVENT_MAX; i++)
   {
      assert(khrn_stats_global_record.event_count[i].id == i);
      if(khrn_stats_global_record.event_count[i].stone == stone)
         khrn_stats_global_record.event_count[i].value = 0;
   }
}



void khrn_stats_get_human_readable(char *buffer, uint32_t len, bool reset, bool stone)
{
   get_human_readable(buffer, len, reset, stone, false);
}

bool khrn_stats_busy(void)
{
   int i;
   for (i = 0; i < KHRN_STATS_THING_MAX; i++)
      if (khrn_stats_global_record.in_thing[i])
         return true;
   return false;
}

static void get_human_readable(char *buffer, uint32_t len, bool reset, bool stone, bool timers)
{
   int i;
   uint32_t t = khrn_stats_getmicrosecs();
   float elapsed = (float)(t - (stone ? khrn_stats_global_record.reset_time_stone :
      khrn_stats_global_record.reset_time)) / 1000000.0f;
   float scale = 100.0f / elapsed;

   //don't assert that some of the stats are busy as e.g. we know STATS_IMAGE is used on the worker thread
   //for (i = 0; i < KHRN_STATS_THING_MAX; i++)
   //   assert(!khrn_stats_global_record.in_thing[i]);

   snprintf(buffer, len,
      "Time: %5.2fs\n", elapsed);
   if(timers)
   {
      for(i = 0; i < KHRN_STATS_THING_MAX; i++)
      {
         if(khrn_stats_global_record.thing_time[i].stone == stone &&
            strnlen(khrn_stats_global_record.thing_time[i].name,1) != 0)
         {
            int current_len = strnlen(buffer, len);
            float val = khrn_stats_global_record.thing_time[i].value / 1000000.0f;
            snprintf(buffer + current_len, len - current_len,
               "%s: %5.2fs (%2.2f%%)\n",
                  khrn_stats_global_record.thing_time[i].name,
                  val,
                  val*scale);
         }
      }
   }

   for(i = 0; i < KHRN_STATS_EVENT_MAX; i++)
   {
      if(khrn_stats_global_record.event_count[i].stone == true &&
         strnlen(khrn_stats_global_record.event_count[i].name,1) != 0)
      {
         int current_len = strnlen(buffer, len);
         snprintf(buffer + current_len, len - current_len,
            "%s: %d (%2.2f/s)\n",
               khrn_stats_global_record.event_count[i].name,
               khrn_stats_global_record.event_count[i].value,
               khrn_stats_global_record.event_count[i].value/elapsed);
      }
   }

   if (reset)
   {
      khrn_stats_reset(false, stone);//don't clear in_thing as we know there is use on the worker thread
      if(stone)
         khrn_stats_global_record.reset_time_stone = t;
      else
         khrn_stats_global_record.reset_time = t;
   }
}

/**
    Append an integer value to an XML string.
**/
/* Copied from egl_brcm_driver_monitor.c */
static void xml_line32(char *xml, EGLint bufsiz, const char *tag, uint32_t value)
{
   size_t tag_len = strlen(tag)*2 + 5 + 10 + 1; /* Opening and closing tags, value and newline. */
   size_t xml_len = strlen(xml);

   if (bufsiz >= 0 && (xml_len + tag_len + 1) < (unsigned)bufsiz)
   {
      sprintf(xml +xml_len, "<%s>0x%08X</%s>\n", tag, value, tag);
   }
}

/**
    Generate into a buffer some XML to document Khronos statistics information,
    similar to what khrn_stats_get_human_readable() does above.
**/
void khrn_stats_get_xml(char *buffer, uint32_t len, bool reset, bool stone)
{
   int i;
   uint32_t t = khrn_stats_getmicrosecs();
   float elapsed = (float)(t - khrn_stats_global_record.reset_time) / 1000000.0f;

   //don't assert that some of the stats are busy as e.g. we know STATS_IMAGE is used on the worker thread
   //for (i = 0; i < KHRN_STATS_THING_MAX; i++)
   //   assert(!khrn_stats_global_record.in_thing[i]);

   xml_line32(buffer, len, "time_ms", (uint32_t)(elapsed*1000.0f));
   for(i = 0; i < KHRN_STATS_THING_MAX; i++)
   {
      if(khrn_stats_global_record.thing_time[i].stone == stone &&
         strnlen(khrn_stats_global_record.thing_time[i].name,1) != 0)
      {
         float val = khrn_stats_global_record.thing_time[i].value / 1000000.0f;
         xml_line32(buffer, len, khrn_stats_global_record.thing_time[i].name,
               (uint32_t)(val*1000.0f));
      }
   }

   for(i = 0; i < KHRN_STATS_EVENT_MAX; i++)
   {
      if(khrn_stats_global_record.event_count[i].stone == true &&
         strnlen(khrn_stats_global_record.event_count[i].name,1) != 0)
      {
         xml_line32(buffer, len, khrn_stats_global_record.event_count[i].name,
               (uint32_t)khrn_stats_global_record.event_count[i].value);
      }
   }

   if(reset)
   {
      khrn_stats_reset(false, stone);//don't clear in_thing as we know there is use on the worker thread
      if(stone)
         khrn_stats_global_record.reset_time_stone = t;
      else
         khrn_stats_global_record.reset_time = t;
   }
}


#endif

/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_options.h"
#include "middleware/khronos/common/2708/khrn_prod_4.h"
#include "middleware/khronos/common/2708/khrn_pool_4.h"
#include "middleware/khronos/common/khrn_hw.h"
#include "middleware/khronos/common/khrn_mem.h"
#include "interface/khronos/common/khrn_int_util.h"
#include "middleware/khronos/egl/egl_disp.h"
#ifdef PRINT_CLIST_DEBUG
#include "middleware/khronos/common/2708/khrn_cle_debug_4.h"
#endif

#include "interface/khronos/include/EGL/egl.h"
#include "vcfw/drivers/chip/abstract_v3d.h"

#include <stddef.h> /* for offsetof */
#include <stdio.h>
#ifdef __mips__
#include <dlfcn.h>
#endif /* __mips__ */

static VCOS_MUTEX_T  backtrace_mutex;

bool khrn_hw_init(void)
{
   vcos_mutex_create(&backtrace_mutex, "backtrace_mutex");
   return true;
}

void khrn_hw_term(void)
{
   vcos_mutex_delete(&backtrace_mutex);
}

void khrn_hw_wait(void)
{
   khrn_issue_finish_job();
}

#ifdef CARBON
typedef struct CARBON_STATISTICS_S_
{
   unsigned __int64 calls;
   unsigned __int64 runtime;
   unsigned __int64 realtime;
   unsigned __int64 transactions[4][8][16];
} carbon_statistics_t;

void v3d_carbonStatistics(void *p);
#endif

#ifdef __mips__
int backtrace_mips32(void **buffer, int size)
{
   unsigned int *p;
   unsigned int *ra = __builtin_return_address(0);
   unsigned int *sp = __builtin_frame_address(0);
   size_t ra_offset;
   size_t stack_size = 0;
   int depth;
   void *x;

   if (!((buffer != NULL) && (size > 0)))
      return 0;

   for (p = (unsigned int *)backtrace_mips32; !stack_size; p++)
   {
      if((*p & 0xffff0000) == 0x27bd0000) /* addiu sp,sp */
         stack_size = abs((short)(*p & 0xffff));
      else if (*p == 0x03e00008) /* jr, ra (got to the end of the function) */
         break;
   }

   /* oops, didnt find the stack */
   if (stack_size)
   {
      sp = (unsigned int *)((unsigned int)sp + stack_size);

      for (depth = 0; depth < size && ra; depth++)
      {
         buffer[depth] = ra;

         ra_offset = 0;
         stack_size = 0;

         for (p = ra; !ra_offset || !stack_size; p--)
         {
            switch (*p & 0xffff0000)
            {
               case 0x27bd0000:  /* addiu sp,sp */
                   stack_size = abs((short)(*p & 0xffff));
                   break;
               case 0xafbf0000:  /* sw ra,???? */
                   ra_offset = (short)(*p & 0xffff);
                   break;
               case 0x3c1c0000:  /* lui gp, 0 */
                   return depth + 1;
               default:
                   break;
            }
         }

         ra = *(unsigned int **)((unsigned int)sp + ra_offset);
         sp = (unsigned int *)((unsigned int)sp + stack_size);
      }

      return depth;
   }
   else
      return 0;
}
#endif /* __mips__ */

void khrn_print_backtrace(void)
{
#ifdef __mips__
   vcos_mutex_lock(&backtrace_mutex);

   /* do a backtrace, if available */
   void *array[255];
   int size;
   int i;

   size = backtrace_mips32(array, sizeof(array));

   CONSOLE_LOG("Obtained %d stack frames.\n", size);
#ifndef ANDROID
   printf("to see more backtrace info, set PROFILING?=1 in V3DDriver.mk.\n");
#endif /* ANDROID */
   for (i = 0; i < size; i++)
   {
      Dl_info dlinfo;
      dladdr(array[i], &dlinfo);
      CONSOLE_LOG("%s, %s, %p\n", dlinfo.dli_fname, dlinfo.dli_sname, dlinfo.dli_saddr);
   }

   vcos_mutex_unlock(&backtrace_mutex);
#endif /* __mips__ */
}

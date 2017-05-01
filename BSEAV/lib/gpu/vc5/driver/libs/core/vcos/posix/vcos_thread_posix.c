/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "vcos_thread.h"
#include "libs/util/assert_helpers.h"
#include <string.h>
#include <unistd.h>

/* Cygwin doesn't always have prctl.h and it doesn't have PR_SET_NAME */
#if defined(__linux__)
 #if !defined(HAVE_PRCTL)
  #define HAVE_PRCTL
 #endif
#include <sys/prctl.h>
#endif

typedef struct thread_entry_args
{
   VCOS_THREAD_ENTRY_FN_T entry;
   void *arg;
   char name[];
} thread_entry_args;

static void *thread_entry(void* void_args)
{
   thread_entry_args* args = (thread_entry_args*)void_args;

   if (args->name[0])
      vcos_this_thread_set_name(args->name);

   VCOS_THREAD_ENTRY_FN_T entry = args->entry;
   void* arg = args->arg;
   free(args);

   (*entry)(arg);
   return 0;
}

VCOS_STATUS_T vcos_thread_create(
   VCOS_THREAD_T *thread,
   const char *name,
   VCOS_THREAD_ENTRY_FN_T entry,
   void *arg)
{
   // Cope with NULL name.
   if (name == NULL)
      name = "";

   size_t name_len = strlen(name);
   thread_entry_args* args = (thread_entry_args*)malloc(sizeof(thread_entry_args) + name_len + 1);
   if (!args)
      return VCOS_ENOMEM;

   args->entry = entry;
   args->arg = arg;
   memcpy(args->name, name, name_len);
   args->name[name_len] = 0;

   int rc = pthread_create(thread, NULL, thread_entry, args);
   if (rc < 0)
   {
      free(args);
      return VCOS_ENOMEM;
   }

   return VCOS_SUCCESS;
}

void vcos_thread_join(VCOS_THREAD_T *thread)
{
   pthread_join(*thread, NULL);
}

void vcos_sleep(uint32_t ms)
{
   struct timespec ts;
   ts.tv_sec = ms/1000;
   ts.tv_nsec = ms % 1000 * (1000000);
   nanosleep(&ts, NULL);
}

void vcos_sleep_us(uint32_t us)
{
   struct timespec ts;
   ts.tv_sec = us/1000000;
   ts.tv_nsec = (us % 1000000) * 1000;
   nanosleep(&ts, NULL);
}

#if !defined(ANDROID) && !defined(USE_ANDROID)
void vcos_yield(void)
{
   verif(pthread_yield() == 0);
}
#endif

void vcos_this_thread_set_name(const char* name)
{
#if defined( HAVE_PRCTL ) && defined( PR_SET_NAME )
   prctl( PR_SET_NAME, (unsigned long)name, 0, 0, 0 );
#endif
}

uint32_t vcos_this_thread_id(void)
{
   static __thread uint32_t s_thread_id = 0;
   static volatile uint32_t s_prev_thread_id = 0;

   if (s_thread_id == 0)
      s_thread_id = __atomic_add_fetch(&s_prev_thread_id, 1u, __ATOMIC_RELAXED);
   return s_thread_id;
}

unsigned vcos_thread_num_processors(void)
{
   return (unsigned)sysconf(_SC_NPROCESSORS_ONLN);
}

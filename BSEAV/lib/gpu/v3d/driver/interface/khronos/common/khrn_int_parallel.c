/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_int_parallel.h"
#include "interface/vcos/vcos.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifndef WIN32
#include <sys/syscall.h>
#endif

typedef enum
{
   NONE = 0,
   MEMCPY,
   MEMSET,
   MEMCMP,
   ABSTRACT_SERVER_JOB,
   TERMINATE
} JOB_TYPE_T;

typedef enum
{
   UNKNOWN = 0,
   PENDING,
   BUSY,
   COMPLETE
} JOB_STATUS_T;

typedef struct
{
   void           *ret;
   void           *dest;
   const void     *src;
   size_t         bytes;
} MEMCPY_ARGS_T;

typedef struct
{
   void           *ret;
   void           *ptr;
   int            val;
   size_t         bytes;
} MEMSET_ARGS_T;

typedef struct
{
   int            ret;
   const void     *ptr1;
   const void     *ptr2;
   size_t         bytes;
} MEMCMP_ARGS_T;

typedef struct
{
   ABSTRACT_SERVER_FUNC_T  serverFunc; /* The server side function pointer    */
   ABSTRACT_SERVER_ARGS_T  args;
   bool                    ret;
} SERVER_ARGS_T;

typedef union
{
   MEMCPY_ARGS_T  memcpy;
   MEMSET_ARGS_T  memset;
   MEMCMP_ARGS_T  memcmp;
   SERVER_ARGS_T  server;
} ARGS_T;

typedef struct
{
   JOB_TYPE_T     jobType;
   JOB_STATUS_T   jobStatus;
   ARGS_T         jobArgs;
} JOB_T;

typedef enum
{
   IDLE = 0,
   ACTIVE
} WORKER_STATUS_T;

typedef struct
{
   VCOS_THREAD_T     thread;
   WORKER_STATUS_T   status;
   JOB_T             job;
   VCOS_EVENT_T      jobAvailable;
   VCOS_EVENT_T      jobDone;
} WORKER_T;

#define MAX_INTERNAL_THREADS  8
#define MEMCMP_BLOCK_SIZE     4096

static bool          s_threadPoolInitialized = false;
static uint32_t      s_numThreads = 2;                   /* Defaults to two threads */
static WORKER_T      s_workers[MAX_INTERNAL_THREADS];
static VCOS_MUTEX_T  s_mutex;

static volatile bool s_memcmpTerminate = false;

static void *worker_thread_main(void *arg)
{
   WORKER_T *worker = (WORKER_T*)arg;

/*
#ifndef WIN32
   int cpu;
   syscall(__NR_getcpu, &cpu, NULL, NULL);
   printf("Parallel worker thread running on cpu %d\n", cpu);
#endif
*/

   while (1)
   {
      /* Wait for a new job */
      worker->status = IDLE;

      if (vcos_event_wait(&worker->jobAvailable) == VCOS_SUCCESS)
      {
         JOB_T *job = &worker->job;

         worker->status = ACTIVE;

         job->jobStatus = BUSY;
         switch (job->jobType)
         {
         case MEMCPY :
            {
               MEMCPY_ARGS_T *args = &job->jobArgs.memcpy;
               args->ret = memcpy(args->dest, args->src, args->bytes);
               break;
            }
         case MEMSET :
            {
               MEMSET_ARGS_T *args = &job->jobArgs.memset;
               args->ret = memset(args->ptr, args->val, args->bytes);
               break;
            }
         case MEMCMP :
            {
               MEMCMP_ARGS_T *args = &job->jobArgs.memcmp;

               /* Split memcmp into chunks to enable early exit */
               uintptr_t p1 = (uintptr_t)args->ptr1;
               uintptr_t p2 = (uintptr_t)args->ptr2;
               int32_t b = 0;
               while (!s_memcmpTerminate && b <= (int32_t)args->bytes - MEMCMP_BLOCK_SIZE)
               {
                  args->ret = memcmp((void*)p1, (void*)p2, MEMCMP_BLOCK_SIZE);
                  if (args->ret != 0)
                  {
                     s_memcmpTerminate = true;
                     break;
                  }
                  else
                  {
                     p1 += MEMCMP_BLOCK_SIZE;
                     p2 += MEMCMP_BLOCK_SIZE;
                     b  += MEMCMP_BLOCK_SIZE;
                  }
               }

               if (!s_memcmpTerminate && args->bytes - b > 0)
                  args->ret = memcmp((void*)p1, (void*)p2, args->bytes - b);

               break;
            }
         case ABSTRACT_SERVER_JOB :
            {
               SERVER_ARGS_T *args = &job->jobArgs.server;
               args->ret = args->serverFunc(&args->args);
               break;
            }
         case TERMINATE :
            {
               job->jobStatus = COMPLETE;
               vcos_event_signal(&worker->jobDone);
               return NULL;
            }
         default :
            UNREACHABLE();
         }
         job->jobStatus = COMPLETE;
         worker->status = IDLE;

         vcos_event_signal(&worker->jobDone);
      }
   }

   return NULL;
}


void khrn_init_thread_pool(void)
{
   uint32_t t, numProcessors;
   char     *env;

   if (s_threadPoolInitialized)
      return;

   s_threadPoolInitialized = true;

   /* Make our protection mutex */
   vcos_demand(vcos_mutex_create(&s_mutex, "parallel_funcs_mtx") == VCOS_SUCCESS);

   numProcessors = vcos_thread_num_processors();
   s_numThreads = numProcessors;

   env = getenv("V3D_MAX_THREADS");
   if (env)
      s_numThreads = atoi(env);

   if (s_numThreads > MAX_INTERNAL_THREADS)
      s_numThreads = MAX_INTERNAL_THREADS;

   /* Clamp to number of actual processors */
   if (numProcessors < s_numThreads)
      s_numThreads = numProcessors;

   if (s_numThreads == 1)
   {
      s_numThreads = 0;
      return;  /* We don't use a worker thread for just one */
   }

   for (t = 0; t < s_numThreads; t++)
   {
      char                 name[16];
      VCOS_THREAD_ATTR_T   attr;

      vcos_demand(vcos_event_create(&s_workers[t].jobAvailable, "jobAvailable") == VCOS_SUCCESS);
      vcos_demand(vcos_event_create(&s_workers[t].jobDone, "jobDone") == VCOS_SUCCESS);

      vcos_snprintf(name, sizeof(name), "worker%d", t);

      vcos_thread_attr_init(&attr);
      vcos_thread_attr_setpriority(&attr, VCOS_THREAD_PRI_NORMAL);

      switch (t % 4)
      {
      case 0 : vcos_thread_attr_setaffinity(&attr, VCOS_AFFINITY_CPU0); break;
      case 1 : vcos_thread_attr_setaffinity(&attr, VCOS_AFFINITY_CPU1); break;
      case 2 : vcos_thread_attr_setaffinity(&attr, VCOS_AFFINITY_CPU2); break;
      case 3 : vcos_thread_attr_setaffinity(&attr, VCOS_AFFINITY_CPU3); break;
      }

      vcos_demand(vcos_thread_create(&s_workers[t].thread, name, &attr, worker_thread_main, &s_workers[t]) == VCOS_SUCCESS);
   }
}

void khrn_term_thread_pool(void)
{
   uint32_t t;

   if (!s_threadPoolInitialized)
      return;

   vcos_mutex_lock(&s_mutex);

   /* Instruct all the threads to exit */
   for (t = 0; t < s_numThreads; t++)
   {
      JOB_T *job = &s_workers[t].job;

      assert(s_workers[t].status == IDLE);

      job->jobType = TERMINATE;
      job->jobStatus = PENDING;
      vcos_event_signal(&s_workers[t].jobAvailable);
   }

   /* Wait for all the threads to finish */
   for (t = 0; t < s_numThreads; t++)
   {
      while (vcos_event_wait(&s_workers[t].jobDone) != VCOS_SUCCESS)
         continue;

      vcos_thread_join(&s_workers[t].thread, NULL);

      /* Delete the thread related resources */
      vcos_event_delete(&s_workers[t].jobAvailable);
      vcos_event_delete(&s_workers[t].jobDone);
   }

   memset(s_workers, 0, sizeof(s_workers));

   s_threadPoolInitialized = false;

   vcos_mutex_unlock(&s_mutex);
   vcos_mutex_delete(&s_mutex);
}

#ifndef DISABLE_PARALLEL_FUNCS

void *khrn_par_memcpy(void *destination, const void *source, size_t num)
{
   if (num == 0)
      return destination;
   else if (num < 512 * 1024 || s_numThreads <= 1)
      return memcpy(destination, source, num);
   else
   {
      uint32_t t;
      uint32_t bytesPerChunk = num / s_numThreads;

      vcos_mutex_lock(&s_mutex);

#ifdef SHOW_ACTIVITY
      printf("MEMCPY %d\n", num);
#endif

      for (t = 0; t < s_numThreads; t++)
      {
         JOB_T *job = &s_workers[t].job;

         assert(s_workers[t].status == IDLE);

         job->jobType = MEMCPY;
         job->jobStatus = PENDING;
         job->jobArgs.memcpy.src = (void*)((uintptr_t)source + bytesPerChunk * t);
         job->jobArgs.memcpy.dest = (void*)((uintptr_t)destination + bytesPerChunk * t);
         job->jobArgs.memcpy.bytes = (t < s_numThreads - 1) ? bytesPerChunk : num - bytesPerChunk * (s_numThreads - 1);

         vcos_event_signal(&s_workers[t].jobAvailable);
      }

      for (t = 0; t < s_numThreads; t++)
      {
         while (vcos_event_wait(&s_workers[t].jobDone) != VCOS_SUCCESS)
            continue;
      }

      vcos_mutex_unlock(&s_mutex);

      return destination;
   }
}

void *khrn_par_memset(void *ptr, int val, size_t num)
{
   if (num == 0)
      return ptr;
   else if (num < 512 * 1024 || s_numThreads <= 1)
      return memset(ptr, val, num);
   else
   {
      uint32_t t;
      uint32_t bytesPerChunk = num / s_numThreads;

      vcos_mutex_lock(&s_mutex);

#ifdef SHOW_ACTIVITY
      printf("MEMSET %d\n", num);
#endif
      for (t = 0; t < s_numThreads; t++)
      {
         JOB_T *job = &s_workers[t].job;

         assert(s_workers[t].status == IDLE);

         job->jobType = MEMSET;
         job->jobStatus = PENDING;
         job->jobArgs.memset.ptr = (void*)((uintptr_t)ptr + bytesPerChunk * t);
         job->jobArgs.memset.val = val;
         job->jobArgs.memset.bytes = (t < s_numThreads - 1) ? bytesPerChunk : num - bytesPerChunk * (s_numThreads - 1);

         vcos_event_signal(&s_workers[t].jobAvailable);
      }

      for (t = 0; t < s_numThreads; t++)
      {
         while (vcos_event_wait(&s_workers[t].jobDone) != VCOS_SUCCESS)
            continue;
      }

      vcos_mutex_unlock(&s_mutex);

      return ptr;
   }
}

int khrn_par_memcmp(const void *ptr1, const void *ptr2, size_t num)
{
   if (num == 0)
      return 0;
   else if (num < 48 * 1024 || s_numThreads <= 1)
      return memcmp(ptr1, ptr2, num);
   else
   {
      uint32_t t;
      uint32_t bytesPerChunk = num / s_numThreads;
      int      ret = 0;
      int      diff = 0;

      vcos_mutex_lock(&s_mutex);

#ifdef SHOW_ACTIVITY
      printf("MEMCMP %d\n", num);
#endif
      s_memcmpTerminate = false;

      for (t = 0; t < s_numThreads; t++)
      {
         JOB_T *job = &s_workers[t].job;

         assert(s_workers[t].status == IDLE);

         job->jobType = MEMCMP;
         job->jobStatus = PENDING;
         job->jobArgs.memcmp.ptr1 = (void*)((uintptr_t)ptr1 + bytesPerChunk * t);
         job->jobArgs.memcmp.ptr2 = (void*)((uintptr_t)ptr2 + bytesPerChunk * t);
         job->jobArgs.memcmp.bytes = (t < s_numThreads - 1) ? bytesPerChunk : num - bytesPerChunk * (s_numThreads - 1);

         vcos_event_signal(&s_workers[t].jobAvailable);
      }

      /* Check the last byte in the compare while we're waiting on the threads. Cheeky optimization. */
      diff = (*(uint8_t*)ptr1 + num - 1) - (*(uint8_t*)ptr2 + num - 1);
      if (diff != 0)
         s_memcmpTerminate = true;

      for (t = 0; t < s_numThreads; t++)
      {
         while (vcos_event_wait(&s_workers[t].jobDone) != VCOS_SUCCESS)
            continue;

         if (ret == 0)
            ret = s_workers[t].job.jobArgs.memcmp.ret;
      }

      vcos_mutex_unlock(&s_mutex);

      if (diff != 0 && ret == 0)
         return diff;

      return ret;
   }
}

uint32_t khrn_par_num_workers(void)
{
   return s_numThreads;
}

#endif

bool khrn_par_server_job(ABSTRACT_SERVER_FUNC_T func, void *opaqueArgs)
{
   uint32_t t;
   bool     ret = true;

   vcos_mutex_lock(&s_mutex);

#ifdef SHOW_ACTIVITY
   printf("SERVER JOB\n");
#endif
   for (t = 0; t < s_numThreads; t++)
   {
      JOB_T *job = &s_workers[t].job;

      assert(s_workers[t].status == IDLE);

      job->jobType = ABSTRACT_SERVER_JOB;
      job->jobStatus = PENDING;
      job->jobArgs.server.serverFunc = func;
      job->jobArgs.server.ret = true;
      job->jobArgs.server.args.args = opaqueArgs;
      job->jobArgs.server.args.part = t;
      job->jobArgs.server.args.numParts = s_numThreads;

      vcos_event_signal(&s_workers[t].jobAvailable);
   }

   for (t = 0; t < s_numThreads; t++)
   {
      while (vcos_event_wait(&s_workers[t].jobDone) != VCOS_SUCCESS)
         continue;

      ret = ret && s_workers[t].job.jobArgs.server.ret;
   }

   vcos_mutex_unlock(&s_mutex);

   return ret;
}

/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Parallel execution of jobs

FILE DESCRIPTION
Execute jobs in parallel threads
=============================================================================*/

#include "vcos.h"
#include "v3d_parallel.h"
#include "helpers/gfx/gfx_util.h"
#include "helpers/snprintf.h"

typedef enum
{
   V3D_PARALLEL_UNINITIALIZED = 0,
   V3D_PARALLEL_READY,
   V3D_PARALLEL_EXECUTING,
   V3D_PARALLEL_TERMINATED
} v3d_parallel_status;

typedef struct v3d_parallel_worker
{
   v3d_parallel_exec_fn exec;
   void                *data;
   v3d_parallel_status  status; /* TODO: is this used/useful? */

   VCOS_THREAD_T        thread;
   VCOS_EVENT_T         jobAvailable;
   VCOS_EVENT_T         jobDone;

   bool                 createdThread;
   bool                 createdJobAvailable;
   bool                 createdJobDone;
} v3d_parallel_worker;

typedef struct v3d_parallel_context
{
   bool                 initialized;
   bool                 initialisedThreadPool;
   uint32_t             numThreads;
   v3d_parallel_worker  workers[V3D_PARALLEL_MAX_THREADS];
   VCOS_MUTEX_T         mutex;
} v3d_parallel_context;

static bool create_thread_pool(void);
static void destroy_thread_pool(void);

/* The context for the parallel thread pool */
static v3d_parallel_context   s_context;

/* The worker thread */
static void *worker_thread_main(void *arg)
{
   v3d_parallel_worker *worker = (v3d_parallel_worker *)arg;

/*
#ifndef WIN32
   int cpu;
   syscall(__NR_getcpu, &cpu, NULL, NULL);
   printf("Parallel worker thread running on cpu %d\n", cpu);
#endif
*/
   worker->status = V3D_PARALLEL_READY;

   /* TODO -- what to do if event_wait doesn't return SUCCESS? */
   while (vcos_event_wait(&worker->jobAvailable) == VCOS_SUCCESS)
   {
      v3d_parallel_exec_fn  exec = worker->exec;

      if (exec == NULL)
         break;

      worker->status = V3D_PARALLEL_EXECUTING;

      exec(worker->data);

      worker->status = V3D_PARALLEL_READY;

      vcos_event_signal(&worker->jobDone);
   }

   worker->status = V3D_PARALLEL_TERMINATED;

   return NULL;
}

static uint32_t calc_num_threads(uint32_t maxThreads)
{
   uint32_t   numThreads = V3D_PARALLEL_MAX_THREADS;
   uint32_t   numCores   = vcos_thread_num_processors() - 1;

   if (numThreads > maxThreads)
      numThreads = maxThreads;

   if (numThreads > numCores)
      numThreads = numCores;

   return numThreads;
}

static bool create_thread_pool(void)
{
   assert(!s_context.initialisedThreadPool);
   assert(vcos_mutex_is_locked(&s_context.mutex));

   for (uint32_t t = 0; t < s_context.numThreads; t++)
   {
      char                 name[16];
      VCOS_THREAD_ATTR_T   attr;
      v3d_parallel_worker *worker = &s_context.workers[t];

      memset(worker, 0, sizeof(v3d_parallel_worker));

      if (vcos_event_create(&worker->jobAvailable, "jobAvailable") != VCOS_SUCCESS)
         goto error;

      worker->createdJobAvailable = true;

      if (vcos_event_create(&worker->jobDone, "jobDone") != VCOS_SUCCESS)
         goto error;

      worker->createdJobDone = true;

      snprintf(name, sizeof(name), "worker%d", t);

      vcos_thread_attr_init(&attr);
      vcos_thread_attr_setpriority(&attr, VCOS_THREAD_PRI_NORMAL);

      /* In VC4 we set the thread affinity here to try to spread out the threads
         For now, we will let the scheduler handle it.  If it proves inadequate, add
         affinity back.
       */

      if (vcos_thread_create(&worker->thread, name, &attr, worker_thread_main, worker) != VCOS_SUCCESS)
         goto error;

      worker->createdThread = true;
   }
   s_context.initialisedThreadPool = true;
   return true;

error:
   destroy_thread_pool();
   s_context.numThreads = 0;
   return false;
}

static void destroy_thread_pool(void)
{
   assert(vcos_mutex_is_locked(&s_context.mutex));

   /* Instruct all the threads to exit */
   for (uint32_t t = 0; t < s_context.numThreads; t++)
   {
      v3d_parallel_worker *worker = &s_context.workers[t];

      if (worker->createdThread)
      {
         worker->exec = NULL;
         worker->data = NULL;
         vcos_event_signal(&s_context.workers[t].jobAvailable);
      }
   }

   /* Wait for all the threads to finish */
   for (uint32_t t = 0; t < s_context.numThreads; t++)
   {
      v3d_parallel_worker *worker = &s_context.workers[t];

      if (worker->createdThread)
         vcos_thread_join(&worker->thread, NULL);
   }

   for (uint32_t t = 0; t < s_context.numThreads; t++)
   {
      v3d_parallel_worker *worker = &s_context.workers[t];

      /* Delete the thread related resources */
      if (worker->createdJobAvailable)
         vcos_event_delete(&worker->jobAvailable);

      if (worker->createdJobDone)
         vcos_event_delete(&worker->jobDone);

      memset(worker, 0, sizeof(v3d_parallel_worker));
   }

   s_context.initialisedThreadPool = false;
}

bool v3d_parallel_init(uint32_t maxThreads)
{
   /* If its already initialized, job done */
   assert(!s_context.initialized);
   memset(&s_context, 0, sizeof(v3d_parallel_context));

   /* Make our protection mutex */
   if (vcos_mutex_create(&s_context.mutex, "parallel_funcs_mtx") != VCOS_SUCCESS)
      return false;

   s_context.numThreads = calc_num_threads(maxThreads);
   s_context.initialized = true;

   return true;
}

void v3d_parallel_term()
{
   vcos_mutex_lock(&s_context.mutex);
   destroy_thread_pool();
   vcos_mutex_unlock(&s_context.mutex);
   vcos_mutex_delete(&s_context.mutex);

   s_context.initialized = false;
}

void v3d_parallel_exec(v3d_parallel_exec_fn exec, unsigned int num_threads, void *data[])
{
   assert(exec != NULL);
   assert(num_threads > 0);

   // Use thread-pool for multiple tasks.
   if (num_threads > 1)
   {
      vcos_mutex_lock(&s_context.mutex);

      // Lazy creation of thread pool.
      bool using_thread_pool = s_context.initialisedThreadPool || create_thread_pool();
      if (using_thread_pool)
      {
         assert(num_threads <= v3d_parallel_get_num_threads());

         for (unsigned t = 0; t != num_threads-1; ++t)
         {
            v3d_parallel_worker *worker = &s_context.workers[t];
            worker->exec = exec;
            worker->data = data[t];
            vcos_event_signal(&worker->jobAvailable);
         }

         // Handle last item with this thread.
         exec(data[num_threads-1]);

         for (unsigned t = 0; t != (num_threads-1); ++t)
         {
            v3d_parallel_worker *worker = &s_context.workers[t];
            vcos_event_wait(&worker->jobDone);
         }
      }

      vcos_mutex_unlock(&s_context.mutex);

      if (using_thread_pool)
         return;
   }

   // Fallback to using current thread.
   for (unsigned t = 0; t != num_threads; ++t)
   {
      exec(data[t]);
   }
}

unsigned int v3d_parallel_get_num_threads()
{
   if (s_context.initialized)
      return s_context.numThreads + 1; // count calling thread

   return 0;
}

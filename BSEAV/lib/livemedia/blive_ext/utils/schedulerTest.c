/***************************************************************************
  * Copyright (c) 2004-2006, Broadcom Corporation
  * All Rights Reserved
  * Confidential Property of Broadcom Corporation
  *
  * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
  * AGREEMENT BETWEEN THE USER AND BROADCOM. YOU HAVE NO RIGHT TO USE OR
  * EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
  *
  * Module Description: Demo App to show single threaded use of Live Media library
  *
  *
  * $brcm_Workfile: $
  * $brcm_Revision: $
  * $brcm_Date: $
  *
  * Revision History:
  * $brcm_Log: $
  * 
   ************************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

/* SAP Library Header File */
#include "blive_ext.h"

/*
// Application to demo the Single Threaded Use of Live Media Library
*/
  
typedef struct appCtx
{
  UsageEnvironment *env;        // Pointer to Live Media env object: needed for LM calls
  int xyz;                      // application specific fields
  /* ... */
} appCtx_t;

//
// Structure to Serialize func1() parameters so that they can be passed to 
// the callback function (i.e. the delayed task) which runs in the LM Scheduler context
//
typedef struct _func1Params {
  void *ctx;                    // application context that needs to be passed to the callback fun (i.e. delayed task)
  blive_scheduler_task_wait_ctx_t waitCtx; // allows app thread to wait for delayed task completion
  char *ipAddr;                 // Func1() specific parameters
  unsigned port;
  unsigned ttl;
} func1Params_t;


/* Application Context */
appCtx_t appCtx1; 
appCtx_t appCtx2; 
appCtx_t appCtx3; 

/* Callback function */
void func1(void* ctx)
{
  int64_t timeout;
  func1Params_t *func1Params;

  func1Params = ( func1Params_t *)ctx;

  printf("*** In func1() CB (run in LM Scheduler PID = %d), app ctx = 0x%x\n", 
      getpid(), (unsigned)func1Params->ctx);

  timeout = 10 * 1000000; //every 10sec

  //now wake up the waiting application thread
  blive_scheduler_signal(&func1Params->waitCtx);

  //blive_scheduler_queue_delayed_task(timeout, (TaskFunc *)func1, ctx);
}
  
/* Callback function */
void func2(void* ctx)
{
  int64_t timeout;

  printf("*** In func2() CB (run in LM Scheduler PID = %d), app ctx = 0x%x\n", 
      getpid(), (unsigned)ctx);

  // reschedule the function
  timeout = 10 * 1000000; //every 10sec
  blive_scheduler_queue_delayed_task(timeout, (TaskFunc *)func2, ctx);
}

/* Callback function */
void func3(void* ctx)
{
  static int cnt = 0;
  int64_t timeout;

  printf("*** In func3() CB (run in LM Scheduler PID = %d), app ctx = 0x%x, cnt = %d\n", 
      getpid(), (unsigned)ctx, cnt++);

  // reschedule the function
  //timeout = 10 * 1000000; //every 10sec
  //blive_scheduler_queue_delayed_task(timeout, (TaskFunc *)func3, ctx);
}

int main()
{
  int i;
  int64_t timeout;
  UsageEnvironment *env;
  func1Params_t func1Params;
  

  // Start Live Media Library Scheduler Thread
  if ( (env = blive_scheduler_open()) == NULL )
  {
    printf("ERROR: Failed to Open the Live Media Scheduler");
    return (-1);
  }

  // when should LM scheduler run this function 
  timeout = 10 * 1000000; 

  // env pointer is needed to access create the Groupsock
  appCtx1.env = env;

  //
  // unit test1: schedule a task multiple times and check for its completion
  //
restart_test:
  for (i=0; i<30; i++)
  {
    timeout = i * 1000000; 
    blive_scheduler_queue_delayed_task(timeout, (TaskFunc *)func3, &appCtx1);
  }
  timeout = i * 1000000; 
  // now setup the func1() parameters
  func1Params.ipAddr = "10.15.5.211";
  func1Params.port = 9999;
  func1Params.ttl = 255;
  func1Params.ctx = &appCtx1;

  blive_scheduler_queue_delayed_task(timeout, (TaskFunc *)func1, &func1Params);

  printf("done scheduling func3 32 times\n");

  printf("waiting for func1() completion (APP thread PID = %d)\n", getpid());
  blive_scheduler_wait(&func1Params.waitCtx);
  printf("after func1() is run, sleeping 5 sec\n");
  sleep(5);
  goto restart_test;
  

  blive_scheduler_close();
  pause();

  /*
   * func1 needs to use Live Media to create a Groupsock object to be able to send & 
   * received UDP Datagrams. 
   * Since Live Media library is **NOT** Thread Safe, a multi-threaded application 
   * must utilize the scheduler provided by the Broadcom's Live Media Extension Library (blive_ext)
   * to serialize access to the Live Media Library.
   *
   */ 

  // setup a callback to invoke func1 in a Serialized manner:
  // this function only takes a parameter to application context structure.
  blive_scheduler_queue_delayed_task(timeout, (TaskFunc *)func1, &func1Params);

  // if application needs to wait for function completion, call the blive_ext APIs to block
  printf("waiting for func1() completion (APP thread PID = %d)\n", getpid());
  blive_scheduler_wait(&func1Params.waitCtx);
  printf("after func1() is run\n");

  /*
   * Example to demonstrate how an application thread can schedule two delayed tasks
   * (which utilize Live Media Library). These tasks are periodically run in the 
   * context of LM Scheduler.
   */
  appCtx2.env = env;
  timeout = 10 * 1000000; //every 10sec
  // setup a callback to invoke func2 in a Serialized manner:
  // this function only takes a parameter to application context structure.
  blive_scheduler_queue_delayed_task(timeout, (TaskFunc *)func2, &appCtx2);

  appCtx3.env = env;
  timeout = 10 * 1000000; //every 10sec
  // setup a callback to invoke func3 in a Serialized manner:
  // this function only takes a parameter to application context structure.
  blive_scheduler_queue_delayed_task(timeout, (TaskFunc *)func3, &appCtx3);

  printf("Main thread now just waits\n");
  pause();
  return (0);
}

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

#include "Groupsock.hh"
#include "BasicUDPSource.hh"
#include "GroupsockHelper.hh"
#include "BasicUsageEnvironment.hh"

/* Live Media Extension Library Header File */
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

// Live Media socket related state
typedef struct 
{
  int fd;                                         
  char rcvBuffer[2048];
  Groupsock *inputGroupsock;
  FramedSource* source;
  appCtx_t *parent;
} socketState_t;

/* Application Context */
appCtx_t appCtx1; 
appCtx_t appCtx2; 
appCtx_t appCtx3; 

void onSourceClosureCB(void *)
{
  //
  //process the socket close event here
  //
}

void newMsgCB(void *ctx, unsigned rcvLen, unsigned , struct timeval , unsigned )
{
  socketState_t *listener;

  //
  //process the received message here
  //
  printf("received msg len %d, ctx = 0x%x\n", rcvLen, (unsigned)ctx);
  
  listener = (socketState_t *)ctx;

  // register a callback with Live Media Library when this socket has data to read or is being closed
  listener->source->getNextFrame(
      (unsigned char *)listener->rcvBuffer, 
      (unsigned)2048,
      newMsgCB, listener, 
      onSourceClosureCB, listener);
}

socketState_t listener;

/* Callback function */
void func1(void* ctx)
{
  func1Params_t *func1Params;
  appCtx_t *appCtx;
  struct in_addr inp;
  const unsigned char ttl = 255;  //immaterial for receiving sockets

  // store the listener specific context

  func1Params = (func1Params_t *)ctx;
  appCtx = (appCtx_t *)func1Params->ctx;

  printf("*** In func1() CB (run in LM Scheduler PID = %d), app ctx = 0x%x\n", 
      getpid(), (unsigned)func1Params->ctx);

  // convert IP Address from ASCII to binary
  if (inet_aton(func1Params->ipAddr, &inp) == 0)
  {
    printf("inet_aton failed: invalid address");
    return;
  }

  const Port port(func1Params->port);

  // create UDP/Datagrams Group socket
  listener.inputGroupsock = new Groupsock(*appCtx->env, inp, port, ttl);

  listener.fd = listener.inputGroupsock->socketNum();

  // create a source object to receive SAP Announcements
  listener.source = BasicUDPSource::createNew(*appCtx->env, listener.inputGroupsock);


  // register a callback with Live Media Library when this socket has data to read or is being closed
  listener.source->getNextFrame(
      (unsigned char *)listener.rcvBuffer, 
      (unsigned)2048,
      newMsgCB, &listener, 
      onSourceClosureCB, &listener);

  //now wake up the waiting application thread
  blive_scheduler_signal(&func1Params->waitCtx);

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
  int64_t timeout;

  printf("*** In func3() CB (run in LM Scheduler PID = %d), app ctx = 0x%x\n", 
      getpid(), (unsigned)ctx);

  // reschedule the function
  timeout = 10 * 1000000; //every 10sec
  blive_scheduler_queue_delayed_task(timeout, (TaskFunc *)func3, ctx);
}

int main()
{
  int64_t timeout;
  UsageEnvironment *env;
  func1Params_t func1Params;
  

  // Start Live Media Library Scheduler Thread
  if ( (env = blive_scheduler_open()) == NULL )
  {
    printf("ERROR: Failed to Open the Live Media Scheduler");
    return (-1);
  }

  //
  // Examples of Scheduler Usage
  //

  /*
   * func1 needs to use Live Media to create a Groupsock object to be able to send & 
   * received UDP Datagrams. 
   * Since Live Media library is **NOT** Thread Safe, a multi-threaded application 
   * must utilize the scheduler provided by the Broadcom's Live Media Extension Library (blive_ext)
   * to serialize access to the Live Media Library.
   *
   */ 

  // when should LM scheduler run this function 
  timeout = 1 * 1000000; 

  // env pointer is needed to access create the Groupsock
  appCtx1.env = env;

  // now setup the func1() parameters
  //func1Params.ipAddr = "10.15.5.211"
  //func1Params.ipAddr = "224.2.127.255";
  func1Params.ipAddr = "224.0.0.255";
  func1Params.port = 9875;
  func1Params.ttl = 255;
  func1Params.ctx = &appCtx1;

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

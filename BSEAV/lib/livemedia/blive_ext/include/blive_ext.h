/***************************************************************************
 *     Copyright (c) 2004-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * Module Description: Live Media Extension Library Header File
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Revision History:
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BLIVE_EXT_H__
#define BLIVE_EXT_H__

#include "Groupsock.hh"
#include "GroupsockHelper.hh"
#include "BasicUsageEnvironment.hh"

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
// APIs for Extensions to Live Media Scheduler: provides Serialized access
// to Live Media Library.
*/

/* 
// API to Open Live Media Scheduler (runs in its own thread of execution).
*/
UsageEnvironment* blive_scheduler_open();

/* 
// Callback Function Prototype (aka prototype for the delayed task)
// This is invoked by the Live Media Scheduler Thread.
*/
typedef void blive_task_func(void *clientCtx, void *clientParams);

/*
// API to Queue a task (aka function) w/ the Live Media Scheduler. 
*/
void blive_scheduler_queue_delayed_task(int64_t microseconds, TaskFunc* func, void *context);

/* 
// API to Close Live Media Scheduler
*/
void blive_scheduler_close(void);

/*
// this context allows an application thread to wait for completion of a task/function
// that is supposed to run in the LM Scheduler Thread context.
*/
typedef struct blive_scheduler_task_wait_ctx 
{
  pthread_mutex_t mutex;
  pthread_mutexattr_t mutex_attr;
  pthread_cond_t cond;
} blive_scheduler_task_wait_ctx_t; 

/*
// APIs to allow an application thread to wait for completion of a task/function
// that is supposed to run in the LM Scheduler Thread context.
*/
void blive_scheduler_signal(blive_scheduler_task_wait_ctx_t* wait_ctx);


/*
// APIs to allow an application callback function (running in LM Scheduler context) to indicate 
// a waiting application thread about a task/function completion.
*/
void blive_scheduler_wait(blive_scheduler_task_wait_ctx_t* wait_ctx);

/*
// APIs to print scheduler stats
*/
void blive_scheduler_print_stats();

/*
// IP Address Type associated with the new IP Channel
*/
typedef enum 
{
  IPv4 = 0,
  IPv6
} bIpChMapAddrType_t;

/*
// Media Protocol Type associated with the new IP Channel
*/
typedef enum 
{
  UDP = 0,
  RTP_AVP
} bIpChMapProtocol_t;

/* Structure to Hold IP Channel information, part of bIpChannelMapInfo */
typedef struct bIpChannelInfo
{
  char * ipAddr; /* IP Channel Multicast address to tune to */
  bIpChMapAddrType_t ipAddrType; /* IP Address type: v4 or v6 */
  unsigned int port; /* IP Channel Port # to tune to */
  bIpChMapProtocol_t protocol; /* IP Channel Protocol: RTP or direct UDP */
} bIpChannelInfo_t;

/*
// Channel Map Info Structure: contains the IP Channel Related Information
// Library fills this structcure using incoming SAP/SDP annoucements and 
// passes this information to the application using Add Channel Callback.
*/
typedef struct bIpChannelMapInfo
{
  bIpChannelInfo_t videoInfo; /* IP Channel Info for Video */
  bIpChannelInfo_t audioInfo; /* IP Channel Info for Audio */
  int sourceChNum;            /* Ch num assigned by the creator/source */
} bIpChannelMapInfo_t;

/* ChannelMapAddChannel Callback Function type definition */
typedef void (bSapAddChannel_t)(void *appCtx, bIpChannelMapInfo_t *); 
typedef void (bSapDelChannel_t)(void *appCtx, int chNum); 

/* 
// API to initialize & enable the SAP Functionality Provided by the 
// Live Media Extension Library.
*/
int blive_sap_init(
    void *appCtx,                           /* Input: app specific context ptr*/
    int sapTimeout,                         /* Input: SAP Session Cache Entry timout in seconds */
    bSapAddChannel_t bSapAddChannelCB,      /* Input: Ch Add Callback Func ptr*/
    bSapDelChannel_t bSapDelChannelCB       /* Input: Ch Del Callback Func ptr*/
    );

/* 
// API to de-initialize the SAP Functionality Provided by the 
// Live Media Extension Library.
*/
void blive_sap_exit(void);

/* 
// API to register SAP Multicast Group Addresses w/ SAP Library
*/
int blive_sap_add_listener_addr(
    char *sapMulticastGroupAddr,        /* SAP Multicast Group Address */
    bIpChMapAddrType_t addrType         /* Address type, IPv4 or IPv6 */
    );

/* 
// API to print SAP Library Stats (for debug purposes)
*/
void blive_sap_print_stats();

#ifdef __cplusplus
}
#endif

#endif /* BLIVE_EXT_H__ */

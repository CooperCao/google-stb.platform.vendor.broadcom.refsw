/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 **************************************************************************/
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

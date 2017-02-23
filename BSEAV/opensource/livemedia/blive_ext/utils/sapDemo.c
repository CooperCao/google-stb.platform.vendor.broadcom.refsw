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
// Application to demo the SAP library usage
*/

#define CH_MAP_SIZE 1024
int channelMap[CH_MAP_SIZE];

/*
// Function to delete an IP channel from Channel Map
// Called by SAP library
*/
void delChannelEntry(void *appCtx, int channelIndex)
{

  printf("App: Got a Request to delete IP Channel # %x, appCtx 0x%x:\n", channelIndex, (int)appCtx);
  /*
  // synchronize access to channel map table w/ the main application
  // by possibly aquiring mutex lock
  */

  /*
  // Call Application routine to delete an entry from a channel map
  // given its channel index/num
  */

  /*
  // release the lock
  */

  printf("*********************************************\n");
  printf("App: Deleted Channel # = %d\n", channelIndex);
  printf("*********************************************\n");

  return;
}

/*
// Function to receive a new channel entry
// Called by the SAP library
*/
void *addChannelEntry(void *appCtx, bIpChannelMapInfo_t *chMapInfo)
{
  static int channelNum = 0;

  /*
  // ipChannelInfo structure contains the IP Channel Information
  // use that to add a new IP channel and then return its index
  // back to the SAP library. SAP library passes this index in the
  // delete Channel callback to allow application to delete a
  // particular IP Channel.
  */

#if 1
  printf("App: Got a Request to add a New IP Channel (appCtx = 0x%x)\n", (int)appCtx);
  printf("\tVideo IP Addr = %s\n", chMapInfo->videoInfo.ipAddr);
  printf("\tVideo Port Number Addr = %d\n", chMapInfo->videoInfo.port);
  printf("\tVideo Transport Protocol = %s\n", (chMapInfo->videoInfo.protocol == UDP) ? "UDP": "RTP");
  if (chMapInfo->audioInfo.ipAddr != NULL)
  {
    printf("\taudio IP Addr = %s\n", chMapInfo->audioInfo.ipAddr);
    printf("\taudio Port Number Addr = %d\n", chMapInfo->audioInfo.port);
    printf("\taudio Transport Protocol = %s\n", (chMapInfo->audioInfo.protocol == UDP) ? "UDP": "RTP");
  }
#endif

  /*
  // synchronize access to channel map table w/ the main application
  // by possibly aquiring mutex lock
  */

  /*
  // Call Application routine to add an entry to the channel map
  */

  /*
  // release the lock
  */

  printf("*********************************************\n");
  channelMap[channelNum] = channelNum;
  printf("Added Channel # = %d\n", channelMap[channelNum++]);
  printf("*********************************************\n");


  /* return this channel number back to the library*/
  return ((void *)&channelMap[channelNum-1]);
}

int ipChannelLineupAcquisitionStart(void *appCtxPtr)
{
  int sapTimeout = 3600; //timeout IP Channel Session Entry in 1 hour

  /*
  // initialize SAP library & register the callbacks for
  // receiving channel add & delete information.
  */
  if (blive_sap_init(appCtxPtr, sapTimeout, addChannelEntry, delChannelEntry) != 0)
  {
    printf("Failed to Setup the IP Channel Acquisition Module\n");
    return (-1);
  }

  /*
  // add an IP Multipcast Group Address to listen on
  */
  //blive_sap_add_listener_addr("239.1.1.255", IPv4);

  return (0);
}

void signalHandler(int sigNum)
{
  printf("Got Signal to Print SAP Library Stats (num = %d)\n", sigNum);
  blive_sap_print_stats();
}

typedef struct appCtx
{
  int xyz;
  /* ... */
} appCtx_t;

int main()
{
  appCtx_t appCtx;

  /* catch SIGINT to enable SAP library stat printing */
  signal(SIGINT, signalHandler);

  /* initialize the SAP library */
  if (ipChannelLineupAcquisitionStart(&appCtx) != 0)
  {
    printf("Failed to initialize the SAP library\n");
  }

  printf("SAP library is now initialized\n");

  /* wait to receive the IP Channel Addition & Deletion Information */
  pause();
  printf("after pause\n");
  return (0);
}

/***************************************************************************
  * Copyright (c) 2004-2007, Broadcom Corporation
  * All Rights Reserved
  * Confidential Property of Broadcom Corporation
  *
  * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
  * AGREEMENT BETWEEN THE USER AND BROADCOM. YOU HAVE NO RIGHT TO USE OR
  * EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
  *
  * Module Description: SAP Library Demo App
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

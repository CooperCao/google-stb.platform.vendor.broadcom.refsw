
//****************************************************************************
//
// Copyright (c) 2007-2009 Broadcom Corporation
//
// This program is the proprietary software of Broadcom Corporation and/or
// its licensors, and may only be used, duplicated, modified or distributed
// pursuant to the terms and conditions of a separate, written license
// agreement executed between you and Broadcom (an "Authorized License").
// Except as set forth in an Authorized License, Broadcom grants no license
// (express or implied), right to use, or waiver of any kind with respect to
// the Software, and Broadcom expressly reserves all rights in and to the
// Software and all intellectual property rights therein.  IF YOU HAVE NO
// AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
// AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
// SOFTWARE.
//
// Except as expressly set forth in the Authorized License,
//
// 1.     This program, including its structure, sequence and organization,
// constitutes the valuable trade secrets of Broadcom, and you shall use all
// reasonable efforts to protect the confidentiality thereof, and to use this
// information only in connection with your use of Broadcom integrated circuit
// products.
//
// 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
// "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
// OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
// RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
// IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
// A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
// ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
// THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
// 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
// OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
// INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
// RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
// EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
// WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
// FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//****************************************************************************
//
//  Filename:       LnxInterface.h
//  Author:         
//  Creation Date:  October 25, 2002
//
//****************************************************************************
//  Description: 
//                  
//
//****************************************************************************
#ifndef LNX_INTERFACE_H
#define LNX_INTERFACE_H

//********************** Include Files ***************************************
#include <pthread.h>
#include <time.h>
#include <arpa/inet.h> 
#include <unistd.h>
#include <errno.h>
#include <net/if_arp.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <net/if.h>

#include <netinet/in.h>
#include <unistd.h>

#include "bcmtypes.h"
//********************** Global Constants ************************************

#define MAX_MSGQ_LENGTH    64

//********************** Global Types ****************************************

/* FROM "AIX Version 4.3 Base Documentation "
 * "http://nscp.upenn.edu/aix4.3html/aixgen/topnav/topnav.htm"
 * The PTHREAD_COND_INITIALIZER macro initializes the static
 * condition variable cond, setting its attributes to default 
 * values. This macro should only be used for static condition 
 * variables, since no error checking is performed
 */
#define msgQueueInitializer      { PTHREAD_COND_INITIALIZER, PTHREAD_MUTEX_INITIALIZER, 0, 0 }

typedef struct _MSGQUEUE
{
    pthread_cond_t condition;
    pthread_mutex_t lock;
    int busy;
    char message[MAX_MSGQ_LENGTH];
	int messageSize;
}MSGQUEUE, *PTR_MSGQUEUE;

typedef PTR_MSGQUEUE	msgQid;
typedef char			MessageNo;

typedef struct _THREADINFO
{
	char *name;
	pthread_t tid;
	pid_t pid;
	struct _THREADINFO *next;
}THREADINFO, *PTHREADINFO;

typedef void *(*PFN_TREAD_ENTRY)(void* arg);

typedef struct _THRADARG
{
	void * arg0; // the first one is for our ThreadMain
	void * arg1; // the second one is the name of the thread
	void * arg2; // the third one is the real argument for the startup routine
}THREADARG, *PTHREADARG;


//********************** Global Variables ************************************


#if defined __cplusplus
extern "C" {
#endif

//********************** Function Declaration ***********************************
int StartMyThread( char *name, int *taskId, void * (*start_routine)(void *) );
void WaitForMsgQNotification(msgQid msgQ, MessageNo *pmsgNo);
msgQid CreateMyMsgQueue(void);
void MsgQueueSend(msgQid msgQID, MessageNo *pMsgPtr, unsigned long nbytes);
void osSleep( unsigned long seconds );
void Lnx_CtrlC_Catch();
void RegisterThreadInfo(char *name);
void UnRegisterThreadInfo(char *name);

int lnxgetline(char *str, int maxStringSize);
void DsgTestPrompt(char * choice);
uint32 GetInterfaceMacAddress(char* interfaceName, uint8* macAddr);
uint32 GetMacAddress(uint32 ipAddr, uint8* macAddr);
uint32 GetIpStackAddress(uint32* ipAddress, char * interfaceName);
uint32 GetIpStackNetmask(uint32* ipAddress, char * interfaceName);
int InterfaceIsUp(char * interfaceName);

void BcmAddRoute(unsigned long routerAddress, char *pInterfaceName, BOOL isDefault);
BOOL BcmDeleteRoute(unsigned long routerAddress, char *pInterfaceName);
void SetStaticIpAddress(unsigned long ipAddressLong, char *pInterfaceName);
void SetSubnetMask(unsigned long netMaskLong, char *pInterfaceName);
void RenewDhcpPriIpAddress(char *pInterfaceName);

uint32 SetInterfaceFlags(uint32 ipAddress, char *pInterfaceName, uint32 flags);

BOOL SetEcmArpEntry(unsigned long ipAddress, uint8* macAddress, char* devName);
extern int oldstdout;
extern int oldstderr;
int Print2File(int fd, char *FmtStr, ...);

#if NPTL_SUPPORT
void SetSignalMask(void);
#endif


#if defined __cplusplus
}
#endif

#endif //LNX_INTERFACE_H


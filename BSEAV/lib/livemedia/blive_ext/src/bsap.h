/***************************************************************************
  * Copyright (c) 2004-2007, Broadcom Corporation
  * All Rights Reserved
  * Confidential Property of Broadcom Corporation
  *
  * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
  * AGREEMENT BETWEEN THE USER AND BROADCOM. YOU HAVE NO RIGHT TO USE OR
  * EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
  *
  * Module Description: SAP Library Private Header File 
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

#ifndef BSAP_H__
#define BSAP_H__
//
// IP Channel Map related stats
//
typedef struct
{
  unsigned int totalSapMulticastsPkts;
  unsigned int totalIncompleteSapPkts;
  unsigned int totalNotSupportedPkts;
  unsigned int totalEncryptedPkts;
  unsigned int totalCompressedPkts;
  unsigned int totalAuthPkts;
  unsigned int totalIncorrectSdpPkts;
  unsigned int totalSessionDelMsgs;
  unsigned int totalSessionDelMsgsTimedoutSession;
  unsigned int totalBadSessionAnnouncementMsgs;
  unsigned int totalSessionTimeouts;
  unsigned int totalDuplicateSessionAnnounceMsgs;
  unsigned int totalNewSessionAnnouncementMsgs;
  unsigned int totalSessionTimeoutsCalls;
  unsigned int currentlyAnnouncedSessions;
} bSapStats_t;

// Minimum SAP header length
#define BIP_CHANNEL_MAP_MIN_SAP_HDR_LEN 4   //4 bytes of SAP fixed header
#define BIP_CHANNEL_MAP_SUPPORTED_SAP_VERSION 1
#define BLIVE_MAX_SAP_LISTENERS 10
#define B_IP_CHANNEL_MAP_RCV_BUF_SIZE 2048


//
// Session Info Entry Structure: stores/aggregates per session information
//
typedef struct bSessionInfoEntry
{
  unsigned int srcIpAddr[4];    //Source address (i.e. announcer's address)
  unsigned short addressType;   //IP Address type: 0 for IPv4, 1 for IPv6
  unsigned short msgidHash;     //hash of the annoucement message (used to differentiate duplicates)
  time_t timeout;         //timeval in sec when this entry should be expired (i.e. deleted)
  MediaSession *sessionInfo;    //detailed info about a session (connx, media, etc. details)
  struct bSessionInfoEntry *nextSessionInfoEntry; //single linked list of currently cached sessions
  int sourceChNum;              //channel number as allocated by the sap library
} bSessionInfoEntry_t;

//
// Per Listener's specific state. One Listener Per SAP Group Address
//
typedef struct 
{
  int fd;                                         //fd for IPv4 multicast listerning sockets
  char *rcvBuffer;
  Groupsock *inputGroupsock;
  FramedSource* source;
  struct bSapCtx *parent;
} bSapListenerCtx_t;

//
// IP Channel Map Library Context
//
typedef struct bSapCtx
{
  void *appCtx;                                   //app specific context ptr
  bSapAddChannel_t *bSapAddChannelCB;              //Ch Add Callback Func ptr
  bSapDelChannel_t *bSapDelChannelCB;              //Ch Del Callback Func ptr
  bSessionInfoEntry_t *sessionInfoListHead;       //head of session Info list
  bSessionInfoEntry_t *sessionInfoListTail;       //tail of session Info list
  int sapTimeout;                                 //session entry timeout interval
  UsageEnvironment* env;
  unsigned int listenerCnt;                       //how many listeners are currently defined
  int sourceChNum;                                //next source ch number to use
  bSapStats_t bSapStats;                          //SAP related stats
  bSapListenerCtx_t listeners[BLIVE_MAX_SAP_LISTENERS]; //Max SAP Listeners
} bSapCtx_t;

//
// parameters for addSAPListener() callback
//
typedef struct 
{
  bSapCtx_t *ctx;
  blive_scheduler_task_wait_ctx_t waitCtx;
  char *ipAddr;
} addSAPListenerParams_t;

#define SAP_PORT 9875
#define SAP_IPV4_GLOBAL_SCOPE_MULTICAST_ADDR "224.2.127.254"
#define SAP_IPV4_LOCAL_SCOPE_MULTICAST_ADDR "239.255.255.255"
#define SAP_IPV4_LINK_LOCAL_SCOPE_MULTICAST_ADDR "224.0.0.255"

#define SAP_ANNOUNCEMENT_TIMEOUT 3600    //min timeout value is 1 hour
#define SAP_TIMEOUT_PERIOD 30            // run timeout function every 5secs
#define SAP_MESSAGE_TYPE_ANNOUNCE 0
#define SAP_MESSAGE_TYPE_DELETE 1

//
// SAP Fixed Header Format
//
typedef struct _sap 
{
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE
  unsigned char compressed:1;
  unsigned char encrypted:1;
  unsigned char messageType:1;
  unsigned char rsvd:1;
  unsigned char addressType:1;
  unsigned char version:3;
#else   //BSTD_ENDIAN_BIG
  unsigned char version:3;
  unsigned char addressType:1;
  unsigned char rsvd:1;
  unsigned char messageType:1;
  unsigned char encrypted:1;
  unsigned char compressed:1;
#endif
  unsigned char authLen;
  unsigned short msgidHash;
  unsigned int srcIpAddr[4];
} SAPHeader_t;

#endif // BSAP_H__

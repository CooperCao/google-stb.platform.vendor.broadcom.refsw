/***************************************************************************
  * Copyright (c) 2004-2010, Broadcom Corporation
  * All Rights Reserved
  * Confidential Property of Broadcom Corporation
  *
  * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
  * AGREEMENT BETWEEN THE USER AND BROADCOM. YOU HAVE NO RIGHT TO USE OR
  * EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
  *
  * Module Description: SAP Library Implementation
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
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "bstd.h"

#include "Groupsock.hh"
#include "BasicUDPSource.hh"
#include "GroupsockHelper.hh"
#include "BasicUsageEnvironment.hh"
#include "Media.hh"
#include "MediaSession.hh"
#include "blive_ext.h"
#include "bsap.h"

BDBG_MODULE(blive_ext); 

int bSapInitDone = 0;

//
// Function to return number of seconds since Epoch
//
static time_t getTimeInSeconds()
{
  int rc = -1;
  struct timeval now;

  rc = gettimeofday(&now, NULL);
#ifdef B_REFSW_DEBUG
  BDBG_ASSERT((rc == 0));
#else
  BSTD_UNUSED (rc);
#endif
  return (now.tv_sec);
}

//
// Function to check if the input IPv4 address is defined on one of local interfaces
static int 
isIpAddrLocal(bSapCtx_t *bSapCtx, char *addr)
{
  struct ifreq ifr;
  sockaddr_in *ifaddr;
  struct in_addr inp;
  
  // convert IP Address from ASCII to binary
  if (inet_aton(addr, &inp) == 0)
  {
    BDBG_MSG(("inet_aton failed: invalid address %s", addr));
    return (-1);
  }

  if (IN_MULTICAST(ntohl(inp.s_addr)))
  {
    BDBG_MSG(("Session is on a Multicast Address"));
    return (0);
  }


  // TODO: Add code to retrieve the interface index instead of hard coding the i/f name
  strcpy(ifr.ifr_name, "eth0");

  // now retrieve the IP address associated with the media
  if (ioctl(bSapCtx->listeners[0].fd, SIOCGIFADDR, &ifr) != 0)
  {
    BDBG_WRN(("Failed to get Interface Address Information"));
    return (-1);
  }

  ifaddr = (sockaddr_in *)&ifr.ifr_addr;

  // compare the addresses
  struct in_addr sin_addr = ifaddr->sin_addr;
  if (strcmp(addr, inet_ntoa(sin_addr)) != 0)
  {
           BDBG_MSG(("Received non-local Unicast Announcement: our address %s, session address %s", 
        inet_ntoa(sin_addr), addr));
    return (-1);
  }

  // unicast addresses matches our local address, return success!
  return(0);

}

//
// Function to verify that required SDP header fields are present.
//
static int verifySDPHeaderFields(bSapCtx_t *bSapCtx, MediaSession *newSession)
{
  MediaSubsessionIterator iter(*newSession);
  MediaSubsession* subSession;
  char *sessionIpAddr = NULL;

  //
  // check the session level fields
  //

  // TODO: add check for origin field when SDP library supports it

  if (newSession->sessionName() == NULL)
  {
    BDBG_WRN(("Incomplete SDP Header: missing Session Name (s=) field"));
    return (-1);
  }

  // if c field (connection info) is present at the session level, and it contains
  // a unicast address (instead of a multicast address), ensure that the 
  // unicast IP addresses matches one of our local IP address. i.e. accept only
  // those session announcements that will be unicast to our local address.
  sessionIpAddr = newSession->connectionEndpointName();
  if (sessionIpAddr != NULL)
  {
    if (isIpAddrLocal(bSapCtx, sessionIpAddr) != 0)
    {
      BDBG_WRN(("SAP Annoucement w/ Non-Local Unicast IP Address, dropping it"));
      return (-1);
    }
  }


  //
  // now check the media level fields
  //

  //check if there is no sub-session fields, it is a invalid announcement
  if (!newSession->hasSubsessions())
  {
    BDBG_WRN(("Incomplete SDP header: it contains no subsession fields"));
     return (-1);
  }

  // check sub-session level fields
  while ((subSession = iter.next()) != NULL) 
  {
    //
    // verify that essential m fields are present
    //

    // we only support media of type video or audio
    if (strncmp(subSession->mediumName(), "video", strlen("video")) != 0 &&
       (strncmp(subSession->mediumName(), "audio", strlen("audio")) != 0) )
    {
      BDBG_WRN(("Media Type ""%s"" is NOT Supported", subSession->mediumName()));
      return (-1);
    }
    
    if (subSession->protocolName() == NULL)
    {
      BDBG_WRN(("Incomplete SDP header: missing protocolName field"));
      return (-1);
    }
    
    if (subSession->clientPortNum() == 0)
    {
      BDBG_WRN(("Incomplete SDP header: missing clientPortNum field"));
      return (-1);
    }

    // verify that either c field is present part of sub-session. if not, it must be 
    // present at the session level.
    if (subSession->connectionEndpointName() == NULL && newSession->connectionEndpointName() == NULL) 
    {
      BDBG_WRN(("Incomplete SDP header: missing c field (connection Info)"));
      return (-1);
    }

    // if c field (connection info) is present at the sub-session level, and it contains
    // a unicast address (instead of a multicast address), ensure that the 
    // unicast IP addresses matches one of our local IP address. i.e. accept only
    // those session announcements that will be unicast to our local address.
    sessionIpAddr = subSession->connectionEndpointName();
    if (sessionIpAddr != NULL)
    {
      if (isIpAddrLocal(bSapCtx, sessionIpAddr) != 0)
      {
        BDBG_MSG(("SAP Annoucement w/ Non-Local Unicast IP Address, dropping it"));
        return (-1);
      }
    }
  }

  // message contains required SAP & SDP headers, return success
  return (0);
}

static void printChMapInfo(bIpChannelMapInfo_t *chMapInfo, int sourceChNum)
{
  BSTD_UNUSED(chMapInfo);
  BSTD_UNUSED(sourceChNum);
  BDBG_MSG(("Added Channel Map Info"));
  BDBG_MSG(("\tVideo IP Addr = %s", chMapInfo->videoInfo.ipAddr));
  BDBG_MSG(("\tVideo Port Number Addr = %d", chMapInfo->videoInfo.port));
  BDBG_MSG(("\tVideo Transport Protocol = %s", (chMapInfo->videoInfo.protocol == UDP) ? "UDP": "RTP"));
  BDBG_MSG(("\taudio IP Addr = %s", chMapInfo->audioInfo.ipAddr));
  BDBG_MSG(("\taudio Port Number Addr = %d", chMapInfo->audioInfo.port));
  BDBG_MSG(("\taudio Transport Protocol = %s", (chMapInfo->audioInfo.protocol == UDP) ? "UDP": "RTP"));
  BDBG_MSG(("\tIP App Channel # = %d", sourceChNum));

  //TODO: this printf causes core dump
  //BDBG_MSG(("\trtpPayloadFormat #: %s\n", subSession->rtpPayloadFormat()));
}

//
// Function to build the IP Channel information from a sub-session record
//
static void bldChMapInfo(MediaSession *session, MediaSubsession* subSession, bIpChannelInfo_t *ipChInfo)
{
  // set the IP Address & Type: use sub-session level c field if present.
  if (subSession->connectionEndpointName() != NULL)
  {
    ipChInfo->ipAddr = subSession->connectionEndpointName();
  }
  else
  {
    ipChInfo->ipAddr = session->connectionEndpointName();
  }

  // hardcoding address type to IPv4 as we dont yet support IPv6
  ipChInfo->ipAddrType = IPv4;

  // set the Port Number
  ipChInfo->port = subSession->clientPortNum();

  // set the Transport Protocol
  if (strncmp(subSession->protocolName(), "RTP", strlen("RTP/AVP")) == 0)
  {
    ipChInfo->protocol = RTP_AVP;
  }
  else if (strncmp(subSession->protocolName(), "udp", strlen("udp")) == 0)
  {
    ipChInfo->protocol = UDP;
  }
}

//
// Function to notify application that a new IP channel is now available
//
int notifyNewChannelToApp(bSapCtx_t *bSapCtx, MediaSession *newSession, bSessionInfoEntry_t *sessionInfoEntry)
{
  bIpChannelMapInfo_t chMapInfo;
  MediaSubsessionIterator iter(*newSession);
  MediaSubsession* subSession;

  //
  // The SDP header can multiple sub-sessions. We treat each one as a separate channel
  // e.g. one sub-session can be for video session & other can be for audio only.
  //
  memset(&chMapInfo, 0, sizeof(bIpChannelMapInfo_t));
  while ((subSession = iter.next()) != NULL) 
  {
    if (strncmp(subSession->mediumName(), "video", strlen("video")) == 0)
    {
      // build IP address information for the video portion of the channel info
      bldChMapInfo(newSession, subSession, &chMapInfo.videoInfo);
    }
    else if (strncmp(subSession->mediumName(), "audio", strlen("audio")) == 0)
    {
      // build IP address information for the audio portion of the channel info
      bldChMapInfo(newSession, subSession, &chMapInfo.audioInfo);
    }
  }

  // set the source channel number
  chMapInfo.sourceChNum = sessionInfoEntry->sourceChNum;

  // invoke application callback to pass the IP new channel information
  bSapCtx->bSapAddChannelCB(bSapCtx->appCtx, &chMapInfo);

  printChMapInfo(&chMapInfo, sessionInfoEntry->sourceChNum);

  BDBG_MSG(("Notified application about the new channel"));
  return (0);
}

/*
static void printSessionInfoEntry(bSessionInfoEntry_t *sessionInfoEntry)
{
  MediaSession *session;

  BDBG_MSG(("Cached Session Info entry *************************:"));
  BDBG_MSG(("\t%0x Source IP Address", ntohl(sessionInfoEntry->srcIpAddr[0])));
  BDBG_MSG(("\t%0x msgidHash", ntohs(sessionInfoEntry->msgidHash)));
  BDBG_MSG(("\t%d addressType", sessionInfoEntry->addressType));
  BDBG_MSG(("\t%d timeout", sessionInfoEntry->timeout));
  session = sessionInfoEntry->sessionInfo;
  BDBG_MSG(("\t%s Session Names", session->sessionName()));
  BDBG_MSG(("\t%s Session Descriptions", session->sessionDescription()));
  BDBG_MSG(("\t%s Session Types", session->mediaSessionType()));
  BDBG_MSG(("\t%s Session Connection Name/IP", session->connectionEndpointName()));
}
*/

static void printSessionInfoList(bSapCtx_t *bSapCtx)
{
  bSessionInfoEntry_t *sessionInfoEntry;
  MediaSession *session;

  sessionInfoEntry = bSapCtx->sessionInfoListHead;
  while (sessionInfoEntry != NULL)
  {
    BDBG_MSG(("Cached Session Info entry *************************:"));
    BDBG_MSG(("\t%0x Source IP Address", ntohl(sessionInfoEntry->srcIpAddr[0])));
    BDBG_MSG(("\t%0x msgidHash", ntohs(sessionInfoEntry->msgidHash)));
    BDBG_MSG(("\t%d addressType", sessionInfoEntry->addressType));
    BDBG_MSG(("\t%d timeout", sessionInfoEntry->timeout));
    session = sessionInfoEntry->sessionInfo;
    BDBG_MSG(("\t%s Session Names", session->sessionName()));
    BDBG_MSG(("\t%s Session Descriptions", session->sessionDescription()));
    BDBG_MSG(("\t%s Session Types", session->mediaSessionType()));
    BDBG_MSG(("\t%s Session Connection Name/IP", session->connectionEndpointName()));

    sessionInfoEntry = sessionInfoEntry->nextSessionInfoEntry; 
  }
#ifndef B_REFSW_DEBUG
  BSTD_UNUSED (session);
#endif
}

//
// Function to remove an entry (prev->nextSessionEntry) from the session list and return the deleted
// entry to the caller.
//
static bSessionInfoEntry_t *delListEntry(bSapCtx_t *bSapCtx, bSessionInfoEntry_t *prev)
{
  bSessionInfoEntry_t *deleteEntry = NULL;

  if (prev != NULL)     // 1st entry is not being deleted
  {
    // get the entry to be deleted
    deleteEntry = prev->nextSessionInfoEntry;

    // remove entry from the linked list
    prev->nextSessionInfoEntry = deleteEntry->nextSessionInfoEntry;

    // update tail pointer if last entry is being deleted
    if (bSapCtx->sessionInfoListTail == deleteEntry)
    {
      bSapCtx->sessionInfoListTail = prev;
    }
  }
  else
  {
    // 1st entry is being deleted: get the entry to be deleted
    deleteEntry = bSapCtx->sessionInfoListHead;

    // remove entry from the linked list
    bSapCtx->sessionInfoListHead = deleteEntry->nextSessionInfoEntry;

    // if list only has one entry that is being deleted, then reset the tail pointer too
    if (bSapCtx->sessionInfoListTail == deleteEntry)
    {
      bSapCtx->sessionInfoListTail = NULL;
    }
  }

  // return the entry that is removed from the list so that caller can free it
  deleteEntry->nextSessionInfoEntry = NULL;
  return (deleteEntry);
}

//
// Function to add a session info entry in the session cache (a single linked list)
// All new entries are added at the tail of the list.
//
static void addListEntry(bSapCtx_t *bSapCtx, bSessionInfoEntry_t *sessionInfoEntry)
{
  // entry is added at the tail of the list
  sessionInfoEntry->nextSessionInfoEntry = NULL;

  if (bSapCtx->sessionInfoListTail != NULL)
  {
    bSapCtx->sessionInfoListTail->nextSessionInfoEntry = sessionInfoEntry;
    bSapCtx->sessionInfoListTail = sessionInfoEntry;
  }
  else
  {
    // empty list, so add 1st element
    bSapCtx->sessionInfoListTail = sessionInfoEntry;
    bSapCtx->sessionInfoListHead = sessionInfoEntry;
  }
}

//
// Function to reset the timeout of a cached entry. It updates the timeout value and also moves 
// the entry to the end of list. This is because the session entry list is maintained in the 
// ascending announcement timeout order. If no repeat announcement is received in 1 hour, session
// entry will be removed from this cache.
//
static void restartSessionInfoEntryTimeout(bSapCtx_t *bSapCtx, bSessionInfoEntry_t *prev)
{
  bSessionInfoEntry_t *resetEntry = NULL;

  //
  // move entry to the tail of the list & then reset the timer value
  //
  resetEntry = delListEntry(bSapCtx, prev);
  addListEntry(bSapCtx, resetEntry);

  // now reset the entry's timeout value
  resetEntry->timeout = getTimeInSeconds() + bSapCtx->sapTimeout;

  BDBG_MSG(("reset timeout value of the session"));
  printSessionInfoList(bSapCtx);
  return;
}

//
// Function to delete a session cache entry and notify application 
//
static void deleteSessionInfoEntry(bSapCtx_t *bSapCtx, bSessionInfoEntry_t *prev)
{
  bSessionInfoEntry_t *deleteEntry = NULL;

  // remove entry from the session cache
  deleteEntry = delListEntry(bSapCtx, prev);
  BDBG_ASSERT(deleteEntry != NULL);

  //notify application to delete this entry from its Channel Map
  bSapCtx->bSapDelChannelCB(bSapCtx->appCtx, deleteEntry->sourceChNum);

  // cleanup Live Media objects
  Medium::close(deleteEntry->sessionInfo);

  // now free the entry
  free(deleteEntry);

  // print the remaining list
  printSessionInfoList(bSapCtx);
  return;
}

//
// Function to match an incoming SAP Message to an existing SAP session cache entry
//
static int lookupUsingSapFields(bSapCtx_t *bSapCtx, SAPHeader_t *sapHeader, bSessionInfoEntry_t **prev)
{
  bSessionInfoEntry_t *sessionInfoEntry;

  *prev = NULL;
  sessionInfoEntry = bSapCtx->sessionInfoListHead;
  while (sessionInfoEntry != NULL)
  {
    // note: we store srcIpAddr & msgidHash in Network byteorder only to allow
    // us to do a comparision w/ incoming pkt w/o doing ntoh conversion.
    if (sessionInfoEntry->srcIpAddr[0] == sapHeader->srcIpAddr[0] &&
        sessionInfoEntry->msgidHash == sapHeader->msgidHash )
    {
      // match found
      return (1);
    }
    *prev = sessionInfoEntry;
    sessionInfoEntry = sessionInfoEntry->nextSessionInfoEntry; 
  }

  // no match
  return (0);

}

//
// Function to add a new sessionInfoEntry into the session cache (which is maintained as single linked list)
//
static int addSessionInfoEntry(bSapCtx_t *bSapCtx, SAPHeader_t *sapHeader, MediaSession *sessionInfo)
{
  bSessionInfoEntry_t *sessionInfoEntry;

  //
  // allocate & initialize a new session info entry. Then add it to the session info cache.
  //
  if ( (sessionInfoEntry = (bSessionInfoEntry_t *)malloc(sizeof(bSessionInfoEntry_t))) == NULL)
  {
    BDBG_WRN(("malloc failure to allocate a session info entry\n"));
    return (-1);
  }

  //initialize the entry
  memset(sessionInfoEntry, 0, sizeof(bSessionInfoEntry_t));

  // store address and msgHashId in network byte order only, this way
  // we dont need to convert them to host order during subsequent lookups
  memcpy((unsigned char *)sessionInfoEntry->srcIpAddr, (unsigned char *)sapHeader->srcIpAddr, 16);
  sessionInfoEntry->addressType = sapHeader->addressType;
  sessionInfoEntry->msgidHash = sapHeader->msgidHash;

  // set the entry timeout value
  sessionInfoEntry->timeout = getTimeInSeconds() + bSapCtx->sapTimeout;

  // save a reference to the parsed SDP fields 
  // Note: sessionInfo object is built by Live Media Library
  sessionInfoEntry->sessionInfo = sessionInfo;

  // assign the next source channel number
  sessionInfoEntry->sourceChNum = bSapCtx->sourceChNum++;

  // now add this session info entry in the session cache.
  // all new entries are added at the tail of the list
  addListEntry(bSapCtx, sessionInfoEntry);

  // and finally tell application about the new session/channel information
  notifyNewChannelToApp(bSapCtx, sessionInfo, sessionInfoEntry);
  printSessionInfoList(bSapCtx);

  // return success
  return (0);
}

//
// Function to do session cache timeout processing : it checks if any of the 
// session entries have timed out. If so, it deleted those entries.
//
static void timeoutSessionInfoCache(bSapCtx_t *bSapCtx)
{
  time_t now;
  int64_t timeout;
  bSessionInfoEntry_t *sessionInfoEntry;
  bSessionInfoEntry_t *prevSessionInfoEntry = NULL;


  BDBG_ASSERT(bSapCtx != NULL);

  now = getTimeInSeconds();
  bSapCtx->bSapStats.totalSessionTimeoutsCalls++;
  //BDBG_MSG(("Cached Session Info entry timer handler (now = %d)*************************", now));
  sessionInfoEntry = bSapCtx->sessionInfoListHead;

  // since session info entries are maintained in a time order, if current entry
  // has not yet expired, rest of entries haven't either.
  while (sessionInfoEntry != NULL && now > sessionInfoEntry->timeout)
  {
    BDBG_MSG(("Cached Session Info entry timed out (now = %d)*************************", now));
    // delete the timedout entry 
    deleteSessionInfoEntry(bSapCtx, prevSessionInfoEntry);

    // also check the next entry for possible timeout
    prevSessionInfoEntry = sessionInfoEntry;
    sessionInfoEntry = sessionInfoEntry->nextSessionInfoEntry; 

    bSapCtx->bSapStats.totalSessionTimeouts++;
    bSapCtx->bSapStats.currentlyAnnouncedSessions--;
  }

  // print SAP stats
  if(getenv("blive_stats")) 
  {
    if((strcasecmp(getenv("blive_stats"), "y"))==0)
    {
      blive_sap_print_stats();
      blive_scheduler_print_stats();
    }
  }

  // re-register a delayed task for timer processing of SAP Session Cache
  timeout = SAP_TIMEOUT_PERIOD * 1000000;
  blive_scheduler_queue_delayed_task(timeout, (TaskFunc *)timeoutSessionInfoCache, bSapCtx);
}

static void printSAPHeader(SAPHeader_t *sapHeader)
{
  struct in_addr announcerIpAddr;

  BDBG_MSG(("version %x, authLen %d, msgId %x", 
        sapHeader->version, sapHeader->authLen, ntohs(sapHeader->msgidHash)));
  announcerIpAddr.s_addr = ntohl(sapHeader->srcIpAddr[0]);
  BDBG_MSG(("Source IPv4 Address %s", inet_ntoa(announcerIpAddr)));
#ifndef B_REFSW_DEBUG
  BSTD_UNUSED (announcerIpAddr);
#endif
}
  
//
// Function to process a SAP Message (both SAP & SDP Processing)
//
static void processSAPMsg(bSapCtx_t *bSapCtx, char *rcvBuffer, int rcvLen)
{
  char *nextHeader;
  SAPHeader_t *sapHeader;
  MediaSession *newSession = NULL;
  int found = 0; 
  bSessionInfoEntry_t *prevSessionInfoEntry;

  BDBG_MSG(("*************** Received a SAP Messages *************** "));


  // Null terminate the received data [SAP Header + SDP Payload]
  rcvBuffer[rcvLen] = '\0';

  //
  // First we validate and process SAP Header
  // Then process SDP header.
  // After that notify app of new IP Channel Info
  //
  bSapCtx->bSapStats.totalSapMulticastsPkts++;

  // verify that we received minimum SAP Header 
  if (rcvLen < BIP_CHANNEL_MAP_MIN_SAP_HDR_LEN)
  {
    BDBG_WRN(("Incomplete SAP Header received"));
    bSapCtx->bSapStats.totalIncompleteSapPkts++;
    return;
  }

  // verfiy the fixed SAP Header: 
  sapHeader = (SAPHeader_t *)rcvBuffer;
  if (sapHeader->version != BIP_CHANNEL_MAP_SUPPORTED_SAP_VERSION)
  {
    BDBG_WRN(("Received SAP packet version is not supported"));
    bSapCtx->bSapStats.totalNotSupportedPkts++;
    return;
  }

  if (sapHeader->encrypted)
  {
    BDBG_WRN(("Received Encrypted SAP packet, encryption is not currently supported"));
    bSapCtx->bSapStats.totalNotSupportedPkts++;
    bSapCtx->bSapStats.totalEncryptedPkts++;
    return;
  }

  if (sapHeader->compressed)
  {
    BDBG_WRN(("Received Compressed SAP packet, de-compression is not currently supported"));
    bSapCtx->bSapStats.totalNotSupportedPkts++;
    bSapCtx->bSapStats.totalEncryptedPkts++;
    return;
  }

  if (sapHeader->authLen != 0)
  {
    BDBG_WRN(("Received SAP packet with Authentication information, ignoring Auth header"));
    bSapCtx->bSapStats.totalAuthPkts++;
    //we ignore the auth information for now and continue processing rest of message
  }

  //TODO: add strict mode parsing
  // e.g. if in strict mode, drop SAP message if msgidHash == 0

  // Validate & store the Source Address 
  if (sapHeader->addressType == 0)
  {
    // Source has IPv4 address
    if (rcvLen < BIP_CHANNEL_MAP_MIN_SAP_HDR_LEN + 4)
    {
      BDBG_WRN(("Received SAP packet with incomplete IPv4 Address"));
      bSapCtx->bSapStats.totalIncompleteSapPkts++;
      return;
    }
  }
  else if (sapHeader->addressType == 1) 
  {
    // Source has IPv6 address
    if (rcvLen < BIP_CHANNEL_MAP_MIN_SAP_HDR_LEN + 16)
    {
      BDBG_WRN(("Received SAP packet with incomplete IPv6 Address"));
      bSapCtx->bSapStats.totalIncompleteSapPkts++;
      return;
    }
    BDBG_WRN(("Source has IPv6 Address, IPv6 support is not yet enabled\n"));
    return;
  }

  //
  // go to next header (beyond SAP fixed Header, IP Addr, Auth Hdr)
  // it can be either 
  //    a) the optional payload type ("application/sdp), or
  //    b) just a '\0', or 
  //    c) start of SDP Header (v=0)
  //
  nextHeader = rcvBuffer + 
    BIP_CHANNEL_MAP_MIN_SAP_HDR_LEN +
    ( (sapHeader->addressType == 1) ? 16 : 4 ) +
    sapHeader->authLen;

  // make sure we have *not* reached end of the pkt
  if (nextHeader >= (rcvBuffer + rcvLen))
  {
    BDBG_WRN(("Received incomplete SAP packet: 1"));
    bSapCtx->bSapStats.totalIncompleteSapPkts++;
    return;
  }

  // is payload type present?
  if (strcasecmp(nextHeader, "application/sdp") == 0)
  {
    // SAP payload MIME type is SDP, advance nextHeader pointer
    nextHeader += strlen("application/sdp");
    // make sure we have *not* reached end of the pkt
    if (nextHeader >= (rcvBuffer + rcvLen))
    {
      BDBG_WRN(("Received incomplete SAP packet: 2"));
      bSapCtx->bSapStats.totalIncompleteSapPkts++;
      return;
    }
  }

  // is the next byte Null Char, if so, skip it. Some announcers may not
  // send the payload type, thus may only have a NULL char or not event that
  if (nextHeader[0] == '\0')
  {
    nextHeader++;
    // make sure we have *not* reached end of the pkt
    if (nextHeader >= (rcvBuffer + rcvLen))
    {
      BDBG_WRN(("Received incomplete SAP packet: 3"));
      bSapCtx->bSapStats.totalIncompleteSapPkts++;
      return;
    }
  }

  BDBG_MSG(("Valid SAP Header Received:"));
  printSAPHeader(sapHeader);

  // at this point, nextHeader must point to the start of SDP header
  if (nextHeader[0] != 'v' ||
      nextHeader[1] != '=' ||
      nextHeader[2] != '0')
  {
    BDBG_WRN(("Received incorrect SDP packet"));
    BDBG_WRN((" 0: %c, 1: %c, 2:%c", nextHeader[0], nextHeader[1], nextHeader[2]));
    BDBG_WRN(("nextHeader %x, recvBuffer %x, rcvLen %d", nextHeader, rcvBuffer, rcvLen));
    bSapCtx->bSapStats.totalIncorrectSdpPkts++;
    return;
  }

  // now we can process SDP payload. We use Live Media's 
  // SDP implementation to parse a SDP payload.
  BDBG_MSG(("SDP payload: \n%s", nextHeader));

  // Now create a MediaSession object which Live Media builds
  // by parsing the contents of the SDP payload
  newSession = MediaSession::createNew(*bSapCtx->env, nextHeader);

  if (newSession == NULL)
  {
    BDBG_WRN(("Live Media SDP Parser failed to Parse SDP Header: invalid SDP packet"));
    bSapCtx->bSapStats.totalIncorrectSdpPkts++;
    return;
  }

  // At this point, both SAP header and SDP payload is parsed.

  //
  // if this a SAP messages to delete an announcement, delete the cached session
  // from the session entry list. Also, inform application to remove this session 
  // from its Channel Map.
  //
  // Else it can be either a new annoucement or a duplicate announcement. Duplicate
  // annoucements simply refresh the timeout period of the cached entry and are dropped.
  // For new annoucements, a new session info entry is created and cached. Then
  // application is informed about the new channel map information. 
  //

  // search for the matching cached session info entry
  found = lookupUsingSapFields(bSapCtx, sapHeader, &prevSessionInfoEntry);

  // if message to delete an annoucement & we still have a cached entry, perform deletion task.
  if (sapHeader->messageType == SAP_MESSAGE_TYPE_DELETE)
  {
    if (found)
    {
      BDBG_MSG(("Session Deletion Packet received for a cached session"));
      // cached entry exists, delete the entry and notify application as well.
      deleteSessionInfoEntry(bSapCtx, prevSessionInfoEntry);

      // cleanup Live Media object associated w/ the delete message
      Medium::close(newSession);

      bSapCtx->bSapStats.totalSessionDelMsgs++;
      bSapCtx->bSapStats.currentlyAnnouncedSessions--;
      return;
    }
    else
    {
      // no cached entry associated w/ this session deletion message
      // probably entry was internally timed out. Ignore this message
      BDBG_MSG(("Session Deletion Packet received for a non-cached session, ignoring it\n"));

      // cleanup Live Media object associated w/ the delete message
      Medium::close(newSession);

      bSapCtx->bSapStats.totalSessionDelMsgsTimedoutSession++;
      return;
    }
  }

  // Else it is either a new SAP annoucement or a repeat of an existing one
  // (either same announcer periodically send out the same announcements or multiple
  // announcers can also do so. Now determine whether this is a new or duplicate.
  // and tell application about the new IP channel information
  if (found)
  {
    // cached entry exists, so it must be a duplicate annoucement
    BDBG_MSG(("Duplicate Session Announcement Message received, ignoring it"));
    restartSessionInfoEntryTimeout(bSapCtx, prevSessionInfoEntry);

    // cleanup Live Media object associated w/ the delete message
    Medium::close(newSession);

    bSapCtx->bSapStats.totalDuplicateSessionAnnounceMsgs++;
    return;
  }
  else
  {
    // no cached entry associated w/ this session annoucement message
    // so it is a new session annoucement. Allocate new sessionInfoEntry
    // entry and cache it.
    BDBG_MSG(("New Session Annoucement Message received"));

    // verify that necessary SDP header fields are present, else ignore this announcement
    if (verifySDPHeaderFields(bSapCtx, newSession) != 0)
    {
      BDBG_MSG(("Incomplete/Incorrect SDP header, dropping SAP Announcement\n"));

      // cleanup Live Media object associated w/ this message
      Medium::close(newSession);

      bSapCtx->bSapStats.totalBadSessionAnnouncementMsgs++;
      return;
    }

    // add session to cache and notify application about the new session
    if (addSessionInfoEntry(bSapCtx, sapHeader, newSession) != 0)
    {
      BDBG_WRN(("Failed to cache a session Announcement Message, dropping it\n"));
      // cleanup Live Media object associated w/ this message
      Medium::close(newSession);

      return;
    }

    bSapCtx->bSapStats.totalNewSessionAnnouncementMsgs++;
    bSapCtx->bSapStats.currentlyAnnouncedSessions++;
    return;
  }
  return;
}

//
// Callback Function that Live Media Scheduler invokes in its thread of execution.
// This function freed up the resources associated with a socket FD. Scheduler should
// not send any more callbacks on this socket.
// Note: For UDP sockets, this function shouldn't get called.
//
static void onSourceClosureCB(void *ctx)
{
  bSapListenerCtx_t *listener;
  
  if (bSapInitDone == 0)
      return;

  BDBG_ASSERT((ctx != NULL));
  listener = (bSapListenerCtx_t *)ctx;

  // free the associated resource
  BDBG_ASSERT((listener->rcvBuffer != NULL));
  free(listener->rcvBuffer);
  listener->rcvBuffer = NULL;

  BDBG_MSG(("*** onSourceClosureCB(): close socket %d\n", listener->fd));
}

//
// Callback Function that Live Media Scheduler invokes in its thread of execution.
// This function processes a incoming SAP Message.
//
static void newSAPMsgCB(void *ctx, unsigned rcvLen, unsigned , struct timeval , unsigned )
{
  bSapCtx_t *sapCtx;
  bSapListenerCtx_t *listener;
  
  if (bSapInitDone == 0)
      return;

  listener = (bSapListenerCtx_t *)ctx;
  BDBG_ASSERT((listener != NULL));

  sapCtx = listener->parent;
  BDBG_ASSERT((sapCtx != NULL));
  BDBG_ASSERT((listener->rcvBuffer != NULL));

  BDBG_MSG(("newSAPMsgCB(), rcvLen = %d, fd = %d\n", rcvLen, listener->fd));

  // call the function to process a SAP Message
  processSAPMsg(sapCtx, listener->rcvBuffer, (int)rcvLen);

  // and re-register the callback function w/ the scheduler
  listener->source->getNextFrame((unsigned char *)listener->rcvBuffer, 
                                 B_IP_CHANNEL_MAP_RCV_BUF_SIZE,
                                newSAPMsgCB, 
                                listener, 
                                onSourceClosureCB, 
                                listener);
  return;
}

addSAPListenerParams_t params;

//
// Callback Function that Live Media Scheduler invokes in its thread of execution.
// This function uses the Live Media Groupsock object to create & bind to a UDP
// socket. It then creates a Source object for this socket which enables to receive
// SAP Announcements on this socket (select list of managed by the Live Media Library)
//
static void addSAPListenerCB(void *ctx)
{
  struct in_addr inp;
  addSAPListenerParams_t *params = (addSAPListenerParams_t *)ctx;
  bSapCtx_t *bSapCtx;
  const Port port(SAP_PORT);
  const unsigned char ttl = 255;  //immaterial for receiving sockets
  bSapListenerCtx_t *listener;

  BDBG_ASSERT(params != NULL);
  bSapCtx = params->ctx;
  BDBG_ASSERT(bSapCtx != NULL);

  // convert IP Address from ASCII to binary
  if (inet_aton(params->ipAddr, &inp) == 0)
  {
    BDBG_ERR(("inet_aton failed: invalid address"));
    return;
  }

  // store the listener specific context
  listener = &bSapCtx->listeners[bSapCtx->listenerCnt];
  listener->parent = bSapCtx;

  // create UDP/Datagrams Group socket
  listener->inputGroupsock = new Groupsock(*bSapCtx->env, inp, port, ttl);
  BDBG_ASSERT( listener->inputGroupsock != NULL );

  listener->fd = listener->inputGroupsock->socketNum();
  BDBG_ASSERT( listener->fd != 0 );

  BDBG_MSG(("Group sock created for Listener Address %s", params->ipAddr));

  // create a source object to receive SAP Announcements
  listener->source = BasicUDPSource::createNew(*bSapCtx->env, listener->inputGroupsock);
  BDBG_ASSERT( listener->source != NULL );

  // allocate the read buffer for this socket
  if ( (listener->rcvBuffer = (char *)malloc(B_IP_CHANNEL_MAP_RCV_BUF_SIZE+1)) == NULL )
  {
    BDBG_ERR(("malloc"));
    BDBG_ASSERT(0);
    return;
  }
  
  // register a callback with Live Media Library when this socket has data to read or is being closed
  listener->source->getNextFrame(
      (unsigned char *)listener->rcvBuffer, 
      B_IP_CHANNEL_MAP_RCV_BUF_SIZE,
      newSAPMsgCB, listener, 
      onSourceClosureCB, listener);

  // now increment the listener Count
  bSapCtx->listenerCnt++;

  // now wakeup the waiting application thread indicating task completion 
  blive_scheduler_signal(&params->waitCtx);
}

//
// Function to start Listening on a Multicast Address (join the given multicast address)
//
static int joinMcastGroup(
    bSapCtx_t *bSapCtx ,        //input: SAP context Pointer
    char *sapMGpAddress         //input: well known Multicast SAP address to join
    )
{
  struct ip_mreq sapMreq;

  //
  // join the multicast group as specified by sapAddress
  //

  // convert IP Address from ASCII to binary
  if (inet_aton(sapMGpAddress, &sapMreq.imr_multiaddr) == 0)
  {
    BDBG_ERR(("inet_aton failed: invalid address %s", sapMGpAddress));
    return (-1);
  }

  // verify that the input address is indeed a Multicast address
  if (!IN_MULTICAST(ntohl(sapMreq.imr_multiaddr.s_addr)))
  {
    BDBG_ERR(("input SAP address is not a multicast address"));
    return (-1);
  }

  // Note: if we ever support multi-homed env, we will need to add this address on all interfaces
  // for now, let kernel figure out the default interface
  sapMreq.imr_interface.s_addr = INADDR_ANY;

  if ( setsockopt(bSapCtx->listeners[bSapCtx->listenerCnt-1].fd, 
        IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&sapMreq, sizeof(sapMreq)) == -1 )
  {
    BDBG_ERR(("IP_ADD_MEMBERSHIP failed\n"));
    return (-1);
  }

  return (0);
}

//
// Function to start the SAP Listener on a given well known SAP Address
//
static void startSAPListener(bSapCtx_t* bSapCtx, char *ipAddr)
{
  int64_t timeout;

  // schedule a task (callback function) to register the SAP Listener w/ Live Media Library
  params.ipAddr = ipAddr;

  // set timeout to 0 to indicate scheduler to immediately run this callback function 
  timeout = 0; 

  // now call scheduler function to register the callback 
  params.ctx = bSapCtx;
  blive_scheduler_queue_delayed_task(timeout, 
      (TaskFunc *)addSAPListenerCB, 
      &params);

  // now wait until the callback function gets to run
  blive_scheduler_wait(&params.waitCtx);
}

/* ***********************************************************************************************/
/* ************************************ SAP APIs *************************************************/
/* ***********************************************************************************************/

// defining the global context pointer here limits its scope to only functions below this point
bSapCtx_t *bSapCtx = NULL;

//
// SAP API to print SAP library Stats
//
void blive_sap_print_stats()
{
  bSapStats_t *bSapStats = &bSapCtx->bSapStats; 
  
  printf("SAP Stats ------------------------------------------------------->\n");
  printf("%10d totalSapMulticastsPkts\n", bSapStats->totalSapMulticastsPkts);
  printf("%10d totalIncompleteSapPkts\n", bSapStats->totalIncompleteSapPkts);
  printf("%10d totalNotSupportedPkts\n", bSapStats->totalNotSupportedPkts);
  printf("%10d totalEncryptedPkts\n", bSapStats->totalEncryptedPkts);
  printf("%10d totalCompressedPkts\n", bSapStats->totalCompressedPkts);
  printf("%10d totalAuthPkts\n", bSapStats->totalAuthPkts);
  printf("%10d totalIncorrectSdpPkts\n", bSapStats->totalIncorrectSdpPkts);
  printf("%10d totalSessionDelMsgs\n", bSapStats->totalSessionDelMsgs);
  printf("%10d totalSessionDelMsgsTimedoutSession\n", bSapStats->totalSessionDelMsgsTimedoutSession);
  printf("%10d totalBadSessionAnnouncementMsgs\n", bSapStats->totalBadSessionAnnouncementMsgs);
  printf("%10d totalSessionTimeouts\n", bSapStats->totalSessionTimeouts);
  printf("%10d totalDuplicateSessionAnnounceMsgs\n", bSapStats->totalDuplicateSessionAnnounceMsgs);
  printf("%10d totalNewSessionAnnouncementMsgs\n", bSapStats->totalNewSessionAnnouncementMsgs);
  printf("%10d currentlyAnnouncedSessions\n", bSapStats->currentlyAnnouncedSessions);
  printf("%10u totalSessionTimeoutsCalls\n", bSapStats->totalSessionTimeoutsCalls);
}

//
// SAP API to start a SAP Listener on a given SAP Multicast Group Address
//
int blive_sap_add_listener_addr(char *sapMulticastGroupAddr, bIpChMapAddrType_t addrType)
{

  if (!bSapInitDone)
  {
    BDBG_ERR(("SAP Library is not yet initialized\n"));
    return (-1);
  }

  if (addrType != IPv4)
  {
    BDBG_MSG(("IPv6 is not yet supported\n"));
    return (-1);
  }

  if (bSapCtx->listenerCnt >= BLIVE_MAX_SAP_LISTENERS)
  {
    BDBG_ERR(("Can't Start anymore listerner's, current MAX LISTENER Count is set to %d\n", BLIVE_MAX_SAP_LISTENERS));
    return (-1);
  }

  // now start SAP listener on the passed in Multicast Group Address
  if (joinMcastGroup(bSapCtx, sapMulticastGroupAddr) != 0)
  {
    BDBG_ERR(("Failed to start the SAP Listener for Address %s\n", sapMulticastGroupAddr));
    free(bSapCtx);
    return (-1);
  }

  return (0);
}

//
// SAP API to de-initialize the IP Channel Map Library 
//
void blive_sap_exit(void)
{

  // if already initialized, simply return
  if (bSapInitDone == 0)
  {
    BDBG_ERR(("SAP Library is not initialized, returning\n"));
    return;
  }

  BDBG_ASSERT(bSapCtx != NULL);

  // TODO: empty the session info cache
  bSapCtx->sessionInfoListHead = NULL; //initialize list of session info entries
  bSapCtx->sessionInfoListTail = NULL;

  // TODO: wait for any pending timeout completions

  // Stop the Live Media Library Scheduler
  blive_scheduler_close();

  // TODO: free all the Groupsock & UDPSource objects

  bSapInitDone = 0;

  // free the bSapCtx
  memset(bSapCtx, 0, sizeof(bSapCtx_t));
  free(bSapCtx);

  return;
}

//
// SAP API to initialize the IP Channel Map Library 
//
int blive_sap_init(
    void *appCtx,                           //Input: app specific context ptr
    int sapTimeout,                         //Input: SAP Session Cache Entry timout in seconds
    bSapAddChannel_t *bSapAddChannelCB,     //Input: Ch Add Callback Func ptr
    bSapDelChannel_t *bSapDelChannelCB      //Input: Ch Del Callback Func ptr
    )
{
  int64_t timeout;

  // if already initialized, simply return
  if (bSapInitDone)
  {
    BDBG_ERR(("SAP Library is already initialized, returning\n"));
    return (0);
  }

  //
  // allocate & initialize global SAP context
  //
  if ( (bSapCtx = (bSapCtx_t *)malloc(sizeof(bSapCtx_t)) ) == NULL )
  {
    BDBG_ERR(("malloc"));
    BDBG_ASSERT(0);
    return (-1);
  }
  memset(bSapCtx, 0, sizeof(bSapCtx_t));

  // save the application passed values
  bSapCtx->appCtx = appCtx;  //app specific context ptr
  bSapCtx->bSapAddChannelCB = bSapAddChannelCB;  //callback function to add a channel
  bSapCtx->bSapDelChannelCB = bSapDelChannelCB;  //callback function to delete a channel
  bSapCtx->sessionInfoListHead = NULL; //initialize list of session info entries
  bSapCtx->sessionInfoListTail = NULL;

  // Start Live Media Library Scheduler
  if ( (bSapCtx->env = blive_scheduler_open()) == NULL )
  {
    BDBG_ERR(("ERROR: Failed to Open the Live Media Scheduler"));
    free(bSapCtx);
    return (-1);
  }

  // set the session cache entry timeout interval
  if (sapTimeout <= 0 || sapTimeout > SAP_ANNOUNCEMENT_TIMEOUT)
    bSapCtx->sapTimeout = SAP_ANNOUNCEMENT_TIMEOUT;
  else
    bSapCtx->sapTimeout = sapTimeout;

  // schedule a task (callback function) to register the SAP Listener w/ Live Media Library
  // using IPv4 Global Scope Multicast Group Address
  startSAPListener(bSapCtx, (char *)SAP_IPV4_GLOBAL_SCOPE_MULTICAST_ADDR);

  // using IPv4 Local Scope Multicast Group Address
  if (joinMcastGroup(bSapCtx, (char *)SAP_IPV4_LOCAL_SCOPE_MULTICAST_ADDR) != 0)
  {
    BDBG_ERR(("Failed to start the SAP Listener for Address %s\n", SAP_IPV4_LOCAL_SCOPE_MULTICAST_ADDR));
    free(bSapCtx);
    return (-1);
  }


  // using IPv4 Link Local Scope Multicast Group Address (224.0.0.0 - 224.0.0.255) 
  if (joinMcastGroup(bSapCtx, (char *)SAP_IPV4_LINK_LOCAL_SCOPE_MULTICAST_ADDR) != 0)
  {
    BDBG_ERR(("Failed to start the SAP Listener for Address %s\n", SAP_IPV4_LINK_LOCAL_SCOPE_MULTICAST_ADDR));
    free(bSapCtx);
    return (-1);
  }

  // now schedule the periodic timer task
  timeout = SAP_TIMEOUT_PERIOD * 1000000;
  blive_scheduler_queue_delayed_task(timeout, (TaskFunc *)timeoutSessionInfoCache, bSapCtx);

  bSapInitDone = 1;
  return 0;
}

/***************************************************************************
 *     Copyright (c) 2004-2006, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * Module Description: simple program to test the IP Multicast behavior
 *
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Revision History:
 * $brcm_Log: $
 * 
 ***************************************************************************/

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
#include <pthread.h>
#include <string.h>

#if 0
//
// Function to return number of seconds since Epoch
//
static time_t getTimeInSeconds()
{
  int rc = -1;
  struct timeval now;

  rc = gettimeofday(&now, NULL);
  BDBG_ASSERT((rc == 0));
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
  if (ioctl(bSapCtx->fd, SIOCGIFADDR, &ifr) != 0)
  {
    BDBG_WRN(("Failed to get Interface Address Information"));
    return (-1);
  }

  ifaddr = (sockaddr_in *)&ifr.ifr_addr;

  // compare the addresses
  if (strcmp(addr, inet_ntoa(ifaddr->sin_addr)) != 0)
  {
    BDBG_WRN(("Received non-local Unicast Announcement: our address %s, session address %s", 
        inet_ntoa(ifaddr->sin_addr), addr));
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
    // verify that essential m fields r present
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
        BDBG_WRN(("SAP Annoucement w/ Non-Local Unicast IP Address, dropping it"));
        return (-1);
      }
    }
  }

  // message contains required SAP & SDP headers, return success
  return (0);
}

static void printChMapInfo(bIpChannelMapInfo_t *chMapInfo, int chNum)
{
  BDBG_MSG(("Added Channel Map Info"));
  BDBG_MSG(("\tIP Addr = %s", chMapInfo->ipAddr));
  BDBG_MSG(("\tPort Number Addr = %d", chMapInfo->port));
  BDBG_MSG(("\tTransport Protocol = %s", (chMapInfo->protocol == UDP) ? "UDP": "RTP"));
  BDBG_MSG(("\tChannel Number = %d", chNum));

  //TODO: this printf causes core dump
  //BDBG_MSG(("\trtpPayloadFormat #: %s\n", subSession->rtpPayloadFormat()));
}

//
// Function to notify application that a new IP channel is now available
//
int notifyNewChannelToApp(bSapCtx_t *bSapCtx, MediaSession *newSession)
{
  bIpChannelMapInfo_t chMapInfo;
  MediaSubsessionIterator iter(*newSession);
  MediaSubsession* subSession;
  unsigned int chIdx;
  int newChannels = 0;

  //
  // The SDP header can multiple sub-sessions. We treat each one as a separate channel
  // e.g. one sub-session can be for video session & other can be for audio only.
  //
  while ((subSession = iter.next()) != NULL) 
  {
    memset(&chMapInfo, 0, sizeof(bIpChannelMapInfo_t));

    newChannels++;

    // set the IP Address & Type: use sub-session level c field if present.
    if (subSession->connectionEndpointName() != NULL)
    {
      chMapInfo.ipAddr = subSession->connectionEndpointName();
    }
    else
    {
      chMapInfo.ipAddr = newSession->connectionEndpointName();
    }
    chMapInfo.ipAddrType = IPv4;

    // set the Port Number
    chMapInfo.port = subSession->clientPortNum();

    // set the Transport Protocol
    chMapInfo.protocol = UDP;

    // invoke application callback to pass the new channel information
    chIdx = bSapCtx->bSapAddChannelCB(bSapCtx->appCtx, &chMapInfo);

    // now store this chIdx into session structure so that we can later 
    // use it to delete a channel when its annoucement is being deleted.
    // Note we are reusing the serverPortNum field of the subSession
    // class to store the chIndex value. Originally this field is used 
    // by RTSP to store the Server Port Number.
    subSession->serverPortNum = (unsigned short)chIdx;

    printChMapInfo(&chMapInfo, chIdx);
  }

  BDBG_MSG(("Notified application about %d new channels", newChannels));
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
  resetEntry->timeout = getTimeInSeconds() + SAP_ANNOUNCEMENT_TIMEOUT;

  BDBG_MSG(("reset timeout value of the session"));
  printSessionInfoList(bSapCtx);
  return;
}

//
// Function to delete a session cache entry and notify application 
//
static void deleteSessionInfoEntry(bSapCtx_t *bSapCtx, bSessionInfoEntry_t *prev)
{
  int channelIndex;
  bSessionInfoEntry_t *deleteEntry = NULL;
  MediaSession *session;
  MediaSubsessionIterator *iter;
  MediaSubsession* subSession;

  // remove entry from the session cache
  deleteEntry = delListEntry(bSapCtx, prev);
  session = deleteEntry->sessionInfo;

  iter = new MediaSubsessionIterator::MediaSubsessionIterator(*session);

  //notify application to delete this entry from its Channel Map
  while ((subSession = iter->next()) != NULL) 
  {
    channelIndex = subSession->serverPortNum;
    bSapCtx->bSapDelChannelCB(bSapCtx->appCtx, channelIndex);
  }

  // cleanup Live Media objects
  Medium::close(session);

  // now free the entry
  BDBG_ASSERT(deleteEntry != NULL);
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
  sessionInfoEntry->timeout = getTimeInSeconds() + SAP_ANNOUNCEMENT_TIMEOUT;

  // save a reference to the parsed SDP fields 
  // Note: sessionInfo object is built by Live Media Library
  sessionInfoEntry->sessionInfo = sessionInfo;

  // now add this session info entry in the session cache.
  // all new entries are added at the tail of the list
  addListEntry(bSapCtx, sessionInfoEntry);

  // and finally tell application about the new session/channel information
  notifyNewChannelToApp(bSapCtx, sessionInfo);
  printSessionInfoList(bSapCtx);

  // return success
  return (0);
}

//
// Function to do session cache timeout processing : it checks if any of the 
// session entries have timed out. If so, it deleted those entries.
//
static void timeoutSessionInfoCache(bSapCtx_t *bSapCtx, time_t now)
{
  bSessionInfoEntry_t *sessionInfoEntry;
  bSessionInfoEntry_t *prevSessionInfoEntry = NULL;

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
  }
}

static void printSAPHeader(SAPHeader_t *sapHeader)
{
  struct in_addr announcerIpAddr;

  BDBG_MSG(("version %x, authLen %d, msgId %x", 
        sapHeader->version, sapHeader->authLen, ntohs(sapHeader->msgidHash)));
  announcerIpAddr.s_addr = ntohl(sapHeader->srcIpAddr[0]);
  BDBG_MSG(("Source IPv4 Address %s", inet_ntoa(announcerIpAddr)));
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
  bSapCtx->ipChMapStats.totalSapMulticastsPkts++;

  // verify that we received minimum SAP Header 
  if (rcvLen < BIP_CHANNEL_MAP_MIN_SAP_HDR_LEN)
  {
    BDBG_WRN(("Incomplete SAP Header received"));
    bSapCtx->ipChMapStats.totalIncompleteSapPkts++;
    return;
  }

  // verfiy the fixed SAP Header: 
  sapHeader = (SAPHeader_t *)rcvBuffer;
  if (sapHeader->version != BIP_CHANNEL_MAP_SUPPORTED_SAP_VERSION)
  {
    BDBG_WRN(("Received SAP packet version is not supported"));
    bSapCtx->ipChMapStats.totalNotSupportedPkts++;
    return;
  }

  if (sapHeader->encrypted)
  {
    BDBG_WRN(("Received Encrypted SAP packet, encryption is not currently supported"));
    bSapCtx->ipChMapStats.totalNotSupportedPkts++;
    bSapCtx->ipChMapStats.totalEncryptedPkts++;
    return;
  }

  if (sapHeader->compressed)
  {
    BDBG_WRN(("Received Compressed SAP packet, de-compression is not currently supported"));
    bSapCtx->ipChMapStats.totalNotSupportedPkts++;
    bSapCtx->ipChMapStats.totalEncryptedPkts++;
    return;
  }

  if (sapHeader->authLen != 0)
  {
    BDBG_WRN(("Received SAP packet with Authentication information, ignoring Auth header"));
    bSapCtx->ipChMapStats.totalAuthPkts++;
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
      bSapCtx->ipChMapStats.totalIncompleteSapPkts++;
      return;
    }
  }
  else if (sapHeader->addressType == 1) 
  {
    // Source has IPv6 address
    if (rcvLen < BIP_CHANNEL_MAP_MIN_SAP_HDR_LEN + 16)
    {
      BDBG_WRN(("Received SAP packet with incomplete IPv6 Address"));
      bSapCtx->ipChMapStats.totalIncompleteSapPkts++;
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
    bSapCtx->ipChMapStats.totalIncompleteSapPkts++;
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
      bSapCtx->ipChMapStats.totalIncompleteSapPkts++;
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
      bSapCtx->ipChMapStats.totalIncompleteSapPkts++;
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
    bSapCtx->ipChMapStats.totalIncorrectSdpPkts++;
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
    bSapCtx->ipChMapStats.totalIncorrectSdpPkts++;
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

      return;
    }
    else
    {
      // no cached entry associated w/ this session deletion message
      // probably entry was internally timed out. Ignore this message
      BDBG_MSG(("Session Deletion Packet received for a non-cached session, ignoring it\n"));

      // cleanup Live Media object associated w/ the delete message
      Medium::close(newSession);

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
      BDBG_WRN(("Incomplete/Incorrect SDP header, dropping SAP Announcement\n"));

      // cleanup Live Media object associated w/ this message
      Medium::close(newSession);

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
    return;
  }
  return;
}

#define B_IP_CHANNEL_MAP_RCV_BUF_SIZE 2048
static void *processMessages(void *data)
{
  bSapCtx_t *bSapCtx = (bSapCtx_t *)data;         //input: IP Channel Map Library Context
  int rcvLen;
  char rcvBuffer[B_IP_CHANNEL_MAP_RCV_BUF_SIZE + 1];
  fd_set rfds;
  struct timeval tv;
  int rc;
  time_t lastTimeout;
  time_t now;


  BDBG_MSG(("processMessage: Main loop to receive SAP Annoucements"));
  lastTimeout = getTimeInSeconds();

  for(;;)
  {
    // add the list of SAP multicast group related sockets to select monitoring list
    FD_ZERO(&rfds);
    FD_SET(bSapCtx->fd, &rfds);

    // set the select timeout
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    // now wait for SAP annoucements to arrive
    rc = select(bSapCtx->maxFd + 1, &rfds, NULL, NULL, &tv);

    // if we got interrupted due to a signal, continue 
    if (rc == -1 && errno == EINTR)
      continue;
    
    //BDBG_MSG(("******** SAP Main Thread ********: select returns %d\n", rc));
    BDBG_ASSERT((rc != -1));

    // either select timeout (rc == 0) or an events arrived on one or more sockets (rc >=1)
    // handle both cases 

    // do timeout processing first: call the timeout routine to check if any of the 
    // session descriptions have timedout
    now = getTimeInSeconds();
    if (now - lastTimeout >= SAP_CACHE_TIMEOUT_PERIOD)
    {
      timeoutSessionInfoCache(bSapCtx, now);
      lastTimeout = getTimeInSeconds();
    }

    if (rc != 0)
    {
      // check for read ready event
      if (FD_ISSET(bSapCtx->fd, &rfds))
      {
        if ( (rcvLen = recvfrom(bSapCtx->fd, rcvBuffer, B_IP_CHANNEL_MAP_RCV_BUF_SIZE, 0, NULL, 0)) == -1 )
        {
          BDBG_ERR(("rcvLenfrom failed: errno %d", errno));
          continue;
        }

        // now process the SAP Message (both SAP & SDP Processing)
        processSAPMsg(bSapCtx, rcvBuffer, rcvLen);
      } //FD_ISSET()
    } //rc != 0
  } //forever loop
}

// defining the global context pointer here limits its scope to only functions below this point
bSapCtx_t *bSapCtx = NULL;
int bSapInitDone = 0;

//
// SAP API to start a SAP Listener on a given SAP Multicast Group Address
//
int bSapAddListenerAddr(char *sapMulticastGroupAddr, bIpChMapAddrType_t addrType)
{

  if (!bSapInitDone)
  {
    BDBG_MSG(("SAP Library is not yet initialized\n"));
    return (-1);
  }

  if (addrType != IPv4)
  {
    BDBG_MSG(("IPv6 is not yet supported\n"));
    return (-1);
  }

  // now start SAP listener on the passed in Multicast Group Address
  if (startSAPListener(bSapCtx->fd, sapMulticastGroupAddr) != 0)
  {
    BDBG_MSG(("Failed to start the SAP Listener\n"));
    return (-1);
  }

  return (0);
}
#endif

//
// Function to start Listening on a Multicast Address (join the given multicast address)
//
static int startSAPListener(
    int fd ,                    //input: UDP socket descriptor
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
    printf("inet_aton failed: invalid address %s", sapMGpAddress);
    return (-1);
  }

  // verify that the input address is indeed a Multicast address
  if (!IN_MULTICAST(ntohl(sapMreq.imr_multiaddr.s_addr)))
  {
    printf("input SAP address is not a multicast address");
    return (-1);
  }

  // Note: if we ever support multi-homed env, we will need to add this address on all interfaces
  // for now, let kernel figure out the default interface
  sapMreq.imr_interface.s_addr = INADDR_ANY;

  if ( setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&sapMreq, sizeof(sapMreq)) == -1 )
  {
    printf("IP_ADD_MEMBERSHIP failed\n");
    return (-1);
  }

  printf("startSAPListener: started listener on SAP Group Address: %s\n", sapMGpAddress);
  
  return (0);
}

#define B_IP_CHANNEL_MAP_RCV_BUF_SIZE 2048
#define SAP_PORT 9875
#define SAP_IPV4_GLOBAL_SCOPE_MULTICAST_ADDR "224.2.127.254"
#define SAP_IPV4_LOCAL_SCOPE_MULTICAST_ADDR "239.255.255.255"
#define SAP_IPV4_LINK_LOCAL_SCOPE_MULTICAST_ADDR "224.0.0.255"
//
// SAP API to initialize the IP Channel Map Library 
//
int main ()
{
  int fd;
  //int reUseFlag;
  struct sockaddr_in localAddr;
  int rcvLen;
  char rcvBuffer[B_IP_CHANNEL_MAP_RCV_BUF_SIZE + 1];

  // now create the UDP Datagram Socket and start the UDP end point for the well-known SAP Port
  if ((fd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) 
  {
    printf("socket");
    return (-1);
  }

  // initialize local address
  memset(&localAddr, 0, sizeof(localAddr));
  localAddr.sin_family = AF_INET;
  localAddr.sin_port = htons(SAP_PORT);
  localAddr.sin_addr.s_addr = htonl(INADDR_ANY);

  // bind to well-known SAP Port
  if (bind(fd, (struct sockaddr *)&localAddr, sizeof(localAddr)) == -1)
  {
    printf("bind");
    return (-1);
  }

  // now start SAP listener on Well Known IPv4 Global Scope Multicast Group Address
  if (startSAPListener(fd, SAP_IPV4_GLOBAL_SCOPE_MULTICAST_ADDR) != 0)
  {
    printf("Failed to start the SAP Listener for Address %s\n", SAP_IPV4_GLOBAL_SCOPE_MULTICAST_ADDR);
    return (-1);
  }

  // now start SAP listener on Well Known IPv4 Global Scope Multicast Group Address
  if (startSAPListener(fd, SAP_IPV4_LOCAL_SCOPE_MULTICAST_ADDR) != 0)
  {
    printf("Failed to start the SAP Listener for Address %s\n", SAP_IPV4_LOCAL_SCOPE_MULTICAST_ADDR);
    return (-1);
  }

  if (startSAPListener(fd, SAP_IPV4_LINK_LOCAL_SCOPE_MULTICAST_ADDR) != 0)
  {
    printf("Failed to start the SAP Listener for Address %s\n", SAP_IPV4_LINK_LOCAL_SCOPE_MULTICAST_ADDR);
    return (-1);
  }

  if ( (rcvLen = recvfrom(fd, rcvBuffer, B_IP_CHANNEL_MAP_RCV_BUF_SIZE, 0, NULL, 0)) == -1 )
  {
    perror("rcvLenfrom failed: ");
    return (0);
  }
  printf("received data bytes = %d\n", rcvLen);

#if 0
  // set up Live Media related usage environment
  bSapCtx->scheduler = BasicTaskScheduler::createNew();
  bSapCtx->env = BasicUsageEnvironment::createNew(*bSapCtx->scheduler);

  // set variable to indicate library is now initialized
  bSapInitDone = 1;

  // now start a thread to do the run time processing of incoming SAP packets
  if (pthread_create(&bSapCtx->thread, NULL, processMessages, bSapCtx) != 0)
  {
    BDBG_MSG(("ptrhead_create"));
    return (-1);
  }
#endif
  printf("bSapInit: SAP Thread Created*****\n");
 return (0);
}

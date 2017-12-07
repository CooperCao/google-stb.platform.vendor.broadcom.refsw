/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/tcp.h>
#include <linux/igmp.h>
#include <netinet/ip.h>

#include <sys/ioctl.h>
#include <net/if.h>

#include "b_os_lib.h"
#include "nexus_platform.h"

#include "bip_priv.h"
#include "bip_igmp_listener.h"

BDBG_MODULE( bip_igmp_listener );
BDBG_FILE_MODULE( bip_igmp_listener_packet );
#define PRINTMSG_PACKET( bdbg_args ) BDBG_MODULE_MSG( bip_igmp_listener_packet, bdbg_args );
BDBG_FILE_MODULE( bip_igmp_listener_list );
#define PRINTMSG_LIST( bdbg_args ) BDBG_MODULE_MSG( bip_igmp_listener_list, bdbg_args );


BDBG_OBJECT_ID( BIP_IgmpListener );

#define PKTS_PER_CHUNK 8        /* how many packets to process at one time */
#if !defined ( B_HAS_NETACCEL ) || defined ( B_HAS_PLAYPUMP_IP )
#define PKTS_PER_READ 1                 /* For normal sockets, UDP allows only 1pkt/recvfrom call*/
#else
#define PKTS_PER_READ PKTS_PER_CHUNK    /* Accelerated Sockets allows packet aggregation */
#endif                  /* For normal sockets, UDP allows only 1pkt/recvfrom call*/
#define MAX_BUFFER_SIZE (( 1316+28 ) * PKTS_PER_CHUNK )

#define IGMP_ALL_ROUTERS    "224.0.0.2"  /* IGMP V2 */
#define IGMP_ALL_ROUTERS_V3 "224.0.0.22"
#define IGMP_ALL_HOST       "224.0.0.1"

typedef enum IGMP_MembershipReportType
{
    IGMP_MembershipReportType_eUnknown = 0,    /* unknown/not supported IGMP Membership Type format */
    IGMP_MembershipReportType_eJoin,
    IGMP_MembershipReportType_eLeave,
    IGMP_MembershipReportType_eMax
} IGMP_MembershipReportType;

/*!
 *  \brief Authenticated device id struct wrapper, for linked list operation
 */
typedef struct B_RTSP_MulticastSession
{
    BLST_S_ENTRY( B_RTSP_MulticastSession ) node; /*!< list node. */
    unsigned int           sessionID;             /*Session id */
    unsigned int           multicast_address;
    int                    groupSpecificQueryCount;
    bool                   waitingForGroupReport;
    bool                   recievedGroupReport;
    bool                   recievedJoinReport;  /* session is passive monitoring till first Join */
    B_MutexHandle          hMutex;              /*!< Mutex to protect shared data.*/
    B_SchedulerTimerId groupSpecificResponseTimer; /* should be per session */
    bool               groupSpecificResponseTimerIsStarted;
    BIP_IgmpListenerHandle pIGMPContext;
} B_RTSP_MulticastSession_T;

typedef struct B_MembershipReportCallbacks
{
    BLST_S_ENTRY( B_MembershipReportCallbacks ) node;         /*!< list node. */
    unsigned int multicast_address;
    BIP_IgmpListener_MembershipReportType memRepType;

    B_MutexHandle          hMutex;                           /*!< Mutex to protect shared data.*/
    BIP_IgmpListenerHandle pIGMPContext;
} B_MembershipReportCallbacks_T;

typedef struct BIP_IgmpListener
{
    BDBG_OBJECT( BIP_IgmpListener )
    BIP_IgmpListenerCreateSettings createSettings;
    BIP_IgmpListenerSettings settings;

    unsigned char *recvBuffer;      /* buffer to recvfrom on socket */
    struct in_addr host_inaddr;     /* host ip_address in network byte order */
    int            igmpSock;        /* file descriptor of igmp socket */
    fd_set         readfds, master; /* master fd_set for the select() command */

    bool electedQuerier;  /* true if  general querier, false if other server the general querier*/

    /* Timers */
    B_MutexHandle      mutexIGMPdata; /* mutex for all data here */
    B_SchedulerHandle  schedulerIGMPdata;
    B_SchedulerTimerId generalQueryTimer;  /* per listener */
    B_SchedulerTimerId querierTimer;     /* per listener */
    B_SchedulerTimerId generalResponseTimer;  /* per listener */
    B_ThreadHandle     schedulerThread; /*scheduler thread that is used for the timers */
    bool               generalQueryTimerIsStarted;
    bool               querierTimerIsStarted;
    bool               generalResponseTimerIsStarted;

    B_ThreadHandle recvThread;             /*main loop thread */
    bool           stopRecvLoop;           /* used when a Ctrl-C is fired to stop the listening thread */
    bool           sentFirstGenQryQuarter; /*Satip spec says first scheduled general query to be (delay/4) */

    /* Questions BLST_Q_HEAD vs BLST_S_HEAD ?? */
    /* MulticastSessions List: maintains list of multicast addresses that should be monitored by IGMP module */
    BLST_S_HEAD( MulticastSessions, B_RTSP_MulticastSession ) MulticastSession_list; /*!< active  Multicast sessions */
    /* Memebership reports: contains a list of information related to each LEAVE recieved that App should know about*/
    BLST_S_HEAD( MembershipReports, B_MembershipReportCallbacks ) MembershipReports_list;
} BIP_IgmpListener;

/*Prototypes for static  functions */
/*  Internal Utilitiy functions */
static unsigned short int computeCheckSum(
    unsigned short int *addr,
    int                 len
    );
static int compareIpAddress(
    struct in_addr A,
    struct in_addr B
    );
static int findMulitcastAddressSessionList(
    BIP_IgmpListenerHandle      hIgmpListener,
    unsigned int                multicast_address,
    B_RTSP_MulticastSession_T **pSession
    );
static int sendIgmpQueryMessage(
    BIP_IgmpListenerHandle hIgmpListener,
    unsigned int           multicast_address
    );
/*  Multicast Session List functions */
static BIP_Status createMulticastSession(
    BIP_IgmpListenerHandle      hIgmpListener,
    unsigned int                multicast_address,
    int                         sessionID,
    B_RTSP_MulticastSession_T **pSession
    );
static BIP_Status destroyMulticastSession(
    BIP_IgmpListenerHandle     hIgmpListener,
    B_RTSP_MulticastSession_T *pSession
    );
static BIP_Status addMulticastSessionToList(
    BIP_IgmpListenerHandle     hIgmpListener,
    B_RTSP_MulticastSession_T *pSession
    );
static BIP_Status removeMulticastSessionFromList(
    BIP_IgmpListenerHandle      hIgmpListener,
    B_RTSP_MulticastSession_T **pSession,
    unsigned int                multicast_address
    );
static BIP_Status cleanMulticastSessionList(
    BIP_IgmpListenerHandle hIgmpListener
    );
static BIP_Status printMulitcastSessionList(
    BIP_IgmpListenerHandle hIgmpListener
    );
/*  MembershipReportNotification List functions */
static BIP_Status createMembershipReportNotification(
    BIP_IgmpListenerHandle                hIgmpListener,
    unsigned int                          multicast_address,
    BIP_IgmpListener_MembershipReportType memRepType,
    B_MembershipReportCallbacks_T       **mMembershipRepHandle
    );
static BIP_Status destroyMembershipReportNotification(
    BIP_IgmpListenerHandle         hIgmpListener,
    B_MembershipReportCallbacks_T *mMembershipRepHandle
    );
static BIP_Status addMembershipReportNotificationToList(
    BIP_IgmpListenerHandle         hIgmpListener,
    B_MembershipReportCallbacks_T *mMembershipRepHandle
    );
static BIP_Status removeMembershipReportNotificationFromList(
    BIP_IgmpListenerHandle         hIgmpListener,
    B_MembershipReportCallbacks_T *mMembershipRepHandle
    );
static BIP_Status cleanMembershipReportNotificationList(
    BIP_IgmpListenerHandle hIgmpListener
    );
static BIP_Status cleanMembershipReportNotificationListForMulticastAddress(
    BIP_IgmpListenerHandle hIgmpListener,
    unsigned int           multicast_address
    );
static int searchMembershipReportNotificationListByMulticast(
    BIP_IgmpListenerHandle                hIgmpListener,
    unsigned int                          multicast_address,
    BIP_IgmpListener_MembershipReportType memRepType
    );
static BIP_Status printMembershipReportNotificationList(
    BIP_IgmpListenerHandle hIgmpListener
    );
/*Timer related functions */
static void verifySpecificMulticast(
    void *session
    );
static void setGroupSpecificResponseTimer(
    BIP_IgmpListenerHandle hIgmpListener,
    B_RTSP_MulticastSession_T *pSession, bool activate
    );
static void verifyAllMulticasts(
    void *context
    );
static void setGeneralResponseTimer(
    BIP_IgmpListenerHandle hIgmpListener,
    bool                   activate
    );
static void sendGeneralQueryMessage(
    void *context
    );
static void setAsElectedQuerier(
    BIP_IgmpListenerHandle hIgmpListener,
    bool                   actvateGeneralQuery
    );
static void enableGeneralQuery(
    void *context
    );
static void setHeardFromElectedQuerierTimer(
    BIP_IgmpListenerHandle hIgmpListener,
    bool                   activate
    );
/* Listening and Processing  functions */
static BIP_Status processIGMPPacket(
    BIP_IgmpListenerHandle hIgmpListener
    );
static void listenIGMPPacket(
    void *data
    );

/* end of Prototype for static functions */

/* IP header computeCheckSum */
static unsigned short int computeCheckSum(
    unsigned short int *addr,
    int                 len
    )
{
    int                 nleft  = len;
    int                 sum    = 0;
    unsigned short int *w      = addr;
    unsigned short int  answer = 0;

    BDBG_ENTER( computeCheckSum );

    while (nleft > 1) {
        sum   += *w++;
        nleft -= sizeof ( unsigned short int );
    }

    if (nleft == 1)
    {
        *(unsigned char *) ( &answer ) = *(unsigned char *) w;
        sum += answer;
    }

    sum    = ( sum >> 16 ) + ( sum & 0xFFFF );
    sum   += ( sum >> 16 );
    answer = ~sum;

    BDBG_LEAVE( computeCheckSum );
    return( answer );
} /* computeCheckSum */

/*
 Both inputs of   network byte order for for the  in_addr_t . and stored in to in_addr

A  equal to B  return value 0
 A  greater than B  retuvn value postive
 A less than B return value negative
*/
static int compareIpAddress(
    struct in_addr A,
    struct in_addr B
    )
{
    unsigned int addressA, addressB;

    BDBG_ENTER( compareIpAddress );
    addressA = (unsigned int)( ntohl( A.s_addr ));
    addressB = (unsigned int)( ntohl( B.s_addr ));
    PRINTMSG_PACKET(( "\tcompare Ip Address" ));
    PRINTMSG_PACKET(( "\tA = %x\n", addressA ));
    PRINTMSG_PACKET(( "\tB = %x\n", addressB ));
    PRINTMSG_PACKET(( "\tCompare value %d \n", addressA - addressB ));

    BDBG_LEAVE( compareIpAddress );
    return( addressA-  addressB );

#if 0
    /*host_ip = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr )->sin_addr);  */
    PRINTMSG_PACKET(( " host %x \n ", ntohl( inet_addr( inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr )->sin_addr )))));
    temp_addr = inet_network( inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr )->sin_addr ));
    PRINTMSG_PACKET(( " other %x \n", ntohl( inet_addr( "192.168.1.13" ))));
    PRINTMSG_PACKET(( "inet_netof %x \n", (unsigned long) temp_addr ));
    if (temp_addr < ntohl( inet_addr( "192.168.1.13" )))
    {
        PRINTMSG_PACKET(( " host ip less than  192.168.1.13\n" ));
    }

    if (temp_addr > ntohl( inet_addr( "224.168.1.15" )))
    {
        PRINTMSG_PACKET(( " host ip greater than  224.168.1.15\n" ));
    }

    if (temp_addr == ntohl( inet_addr( "192.168.1.12" )))
    {
        PRINTMSG_PACKET(( " host ip equal to 192.168.1.12\n" ));
    }
#endif /* if 0 */
} /* compareIpAddress */

/**
 return 1: if address ia  Mulitcast Address from our  RTSP Server
  return 0:  Another multicast address
  input: IGMPContext and multiaddress( in host order)
  input/output: B_RTSP_MulticastSession_T
*/
static int findMulitcastAddressSessionList(
    BIP_IgmpListenerHandle      hIgmpListener,
    unsigned int                multicast_address,
    B_RTSP_MulticastSession_T **pSessionPtr
    )
{
    B_RTSP_MulticastSession_T *pSession=NULL;

    /**
         * A recognized  Multicast Address must:
         * 1.  Validate that mutlicast in an active RTSP Session
         **/

    BDBG_ENTER( findMulitcastAddressSessionList );
    PRINTMSG_LIST(( "Check Multicast Address from Report  %d.%d.x.x\n", ((( multicast_address ) >> 24 )& 0xFF ), ((( multicast_address ) >> 16 )& 0xFF )));


    /* Check multicast in RTSP  */
    pSession = BLST_S_FIRST( &( hIgmpListener->MulticastSession_list ));
    /*  find sessions matching mutlicast*/
    while (pSession != NULL)
    {
        int i = 0;
        PRINTMSG_LIST(( " Looking Through Sesion List looking for Matching Mulitcast Addres   %d.%d.x.x\n", ((( pSession->multicast_address ) >> 24 )& 0xFF ), ((( pSession->multicast_address ) >> 16 )& 0xFF )));
        PRINTMSG_LIST(( " %d Session's Mutlicast Address %d  compared to packet's multicast Address %d\n", i, pSession->multicast_address, multicast_address ));
        if (pSession->multicast_address == multicast_address)
        {
           PRINTMSG_LIST(( "Valid Mulitcast Address FOUND from Report  %p\n", (void *)pSession ));
           *pSessionPtr = pSession;
           BDBG_LEAVE( findMulitcastAddressSessionList );
           return( 1 );
        }

        pSession = BLST_S_NEXT( pSession, node );
    }
    PRINTMSG_LIST(( "Valid Group Mulitcast Address  NOT FOUND from Report  %p\n",  (void *)pSession ));

    BDBG_LEAVE( findMulitcastAddressSessionList );
    return( 0 );
} /* findMulitcastAddressSessionList */

/* input mutlicast is in host order */
static int sendIgmpQueryMessage(
    BIP_IgmpListenerHandle hIgmpListener,
    unsigned int           multicast_address
    )
{
    int          rc=0;
    int          igmp_group_address=0;
    struct iphdr ip_hdr;

#if IGMP_V2
    struct igmphdr igmp_hdr;  /* v2 */
#else
    struct igmpv3_query igmp_query_hdr;  /* v3*/
#endif
    struct sockaddr_in localIpAddr;
    struct sockaddr_in remoteIpAddr;
    socklen_t          one = 1;
    /*const int         *val = &one; */
    int totalbytes=0;
#if IGMP_V2
    int packet_len = sizeof( struct iphdr ) + sizeof( struct igmphdr );
#else
    int packet_len = sizeof( struct iphdr ) + sizeof( struct igmpv3_query );
#endif
    char *packet=NULL;
    packet = BKNI_Malloc( packet_len );
    BIP_CHECK_GOTO(( packet ), ( "Memory Allocation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );
    BDBG_ENTER( sendIgmpQueryMessage );

    memset( &ip_hdr, 0, sizeof( struct iphdr ));
#if IGMP_V2
    memset( &igmp_hdr, 0, sizeof( struct igmphdr ));
    totalbytes = sizeof( struct iphdr ) + sizeof( struct igmphdr );
#else
    memset( &igmp_query_hdr, 0, sizeof( struct igmpv3_query ));
    totalbytes = sizeof( struct iphdr ) + sizeof( struct igmpv3_query );
#endif /* if IGMP_V2 */

    if (multicast_address ==0)
    {
        /*send general query and 224.0.0.1  */
        multicast_address   = inet_addr( "224.0.0.1" );
        igmp_group_address  =  inet_addr( "0.0.0.0" );
        igmp_query_hdr.code = 100; /*10 second resp time */
    }
    else
    {
        /* send group specific query */
        multicast_address   = htonl( multicast_address );
        igmp_group_address  =  multicast_address;
        igmp_query_hdr.code = 10; /* 1 second */
    }

    localIpAddr.sin_family = AF_INET;
    localIpAddr.sin_addr   = hIgmpListener->host_inaddr;
    localIpAddr.sin_port   = htons( 5000 );

    remoteIpAddr.sin_family      = AF_INET;
    remoteIpAddr.sin_addr.s_addr = multicast_address;
    remoteIpAddr.sin_port        = htons( 5000 );

    ip_hdr.ihl     = 5;
    ip_hdr.version = 4;
    ip_hdr.tos     = 0;

    ip_hdr.tot_len  = htons( totalbytes );
    ip_hdr.id       = htons( 3 ); /*check if id has to be from the original socket*/
    ip_hdr.frag_off = 0x00;
    ip_hdr.ttl      = 1;
    ip_hdr.protocol = IPPROTO_IGMP;
    ip_hdr.check    = 0;
    ip_hdr.saddr    = hIgmpListener->host_inaddr.s_addr;
    ip_hdr.daddr    = multicast_address;

#if IGMP_V2
    igmp_hdr.type  = 0x11;
    igmp_hdr.code  = 100;
    igmp_hdr.csum  = 0;
    igmp_hdr.group = igmp_group_address;
    igmp_hdr.csum  = computeCheckSum((unsigned short int *) &igmp_hdr, sizeof( struct igmphdr ));
#else /* if IGMP_V2 */
    igmp_query_hdr.type     = 0x11;
    igmp_query_hdr.csum     = 0;
    igmp_query_hdr.group    = igmp_group_address;
    igmp_query_hdr.qrv      = 2;
    igmp_query_hdr.suppress = 0;
    igmp_query_hdr.resv     = 0;
    igmp_query_hdr.qqic     = 125;
    igmp_query_hdr.nsrcs    = 0;
    /*  igmp_query_hdr.srcs[0]  = 0; Causes overwriting of stack variable: one */
    igmp_query_hdr.csum = computeCheckSum((unsigned short int *) &igmp_query_hdr, sizeof( struct igmpv3_query ));
#endif /* if IGMP_V2 */

    ip_hdr.check = computeCheckSum((unsigned short int *) &ip_hdr, sizeof( struct iphdr ));

    BKNI_Memset( packet, 0, packet_len );
    memcpy( packet, &ip_hdr, sizeof( struct iphdr ));
#if IGMP_V2
    memcpy(( packet+sizeof( struct iphdr )), &igmp_hdr, sizeof( struct igmphdr ));
#else
    memcpy(( packet+sizeof( struct iphdr )), &igmp_query_hdr, sizeof( struct igmpv3_query ));
#endif
#if 0
    BDBG_MSG(( "\n-- NEW ######### Outgoing Packet start--\n" ));
    rc          = packet_len;
    temp_packet = packet;
    while (rc--)
    {
        BDBG_MSG(( "%.2x ", *temp_packet ));
        temp_packet++;
    }
    BDBG_MSG(( "\n-- Outgoing Packet end-- value of one %d\n", one ));
#endif /* if 0 */

    rc = setsockopt( hIgmpListener->igmpSock, IPPROTO_IP, IP_HDRINCL, (const void *) &one, sizeof( one ));
    BIP_CHECK_GOTO(( rc==0 ), ( "setsocketopt failed to set IP_HDRINCL ..." ), error, rc, rc );

    BDBG_MSG((  BIP_MSG_PRE_FMT "sending to "BIP_INET_ADDR_FMT":%d "
                BIP_MSG_PRE_ARG, BIP_INET_ADDR_ARG(remoteIpAddr.sin_addr), ntohs( remoteIpAddr.sin_port )));
    BDBG_MSG((  BIP_MSG_PRE_FMT "   from "BIP_INET_ADDR_FMT":%d \n"
                BIP_MSG_PRE_ARG, BIP_INET_ADDR_ARG(localIpAddr.sin_addr), ntohs( localIpAddr.sin_port )));

    rc = sendto( hIgmpListener->igmpSock, packet, totalbytes, 0, (struct sockaddr *)&remoteIpAddr, sizeof( remoteIpAddr ));
    BIP_CHECK_GOTO(( rc >= 0 ), ( "sendto failed ..." ), error, rc, rc );
    /* BDBG_MSG(( BIP_MSG_PRE_FMT "Sent %d bytes of initial Data \n" BIP_MSG_PRE_ARG, initialDataLen));*/

    BKNI_Free( packet );

    BDBG_LEAVE( sendIgmpQueryMessage );
    return( rc );

error:

    if (packet)
    {BKNI_Free( packet ); }
    BDBG_LEAVE( sendIgmpQueryMessage );
    return( rc );
} /* sendIgmpQueryMessage */

/*
 Sessions
   API wiil be created to add and remove multicast session

*/
/*! \brief open an Multicast session, initialize session data.
 *  \param[in] IGMPContext  IGMPContext core data poiner.
 *  \param[in] Multicast Address ( host order)
 *  \param[in] SessionID;
 *  \param[in,out] pSession returned  multicast session handle, if success.
 */
static BIP_Status createMulticastSession(
    BIP_IgmpListenerHandle      hIgmpListener,
    unsigned int                multicast_address,
    int                         sessionID,
    B_RTSP_MulticastSession_T **pSession
    )
{
    B_RTSP_MulticastSession_T *Session  = NULL;
    BIP_Status                  retValue = BIP_SUCCESS;

    /* sanity check */
    BDBG_ASSERT( hIgmpListener );
    BDBG_ASSERT( pSession );
    BDBG_ENTER( createMulticastSession );

    Session = BKNI_Malloc( sizeof( B_RTSP_MulticastSession_T ));
    BIP_CHECK_GOTO(( Session ), ( "Memory Allocation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, retValue );
    BKNI_Memset( Session, 0, sizeof( B_RTSP_MulticastSession_T ));

    Session->multicast_address       = multicast_address;
    Session->sessionID               = sessionID;
    /* coverity[missing_lock] */
    Session->groupSpecificQueryCount = 0;
    /* coverity[missing_lock] */
    Session->waitingForGroupReport   = false;
    /* coverity[missing_lock] */
    Session->recievedGroupReport     = false;
    /* coverity[missing_lock] */
    Session->recievedJoinReport      = false;
    Session->pIGMPContext            = hIgmpListener;
    Session->hMutex = B_Mutex_Create( NULL );
    Session->groupSpecificResponseTimer = NULL;
    Session->groupSpecificResponseTimerIsStarted = false;
    BIP_CHECK_GOTO(( Session->hMutex ), ( "Memory Allocation Failed" ), error, BIP_ERR_OS_CHECK_ERRNO, retValue );
    *pSession = Session;

    BDBG_LEAVE( createMulticastSession );
    return( retValue );

error:
    if (Session)
    {BKNI_Free( Session ); }
    BDBG_LEAVE( createMulticastSession );
    return( retValue );
} /* createMulticastSession */

static BIP_Status destroyMulticastSession(
    BIP_IgmpListenerHandle     hIgmpListener,
    B_RTSP_MulticastSession_T *pSession
    )
{
    BSTD_UNUSED( hIgmpListener );
    BDBG_ASSERT( pSession );
    BDBG_ENTER( destroyMulticastSession );

    B_Mutex_Lock( pSession->hMutex );
    /*TODO??: Is there anything else we need to destroy  */

    B_Mutex_Unlock( pSession->hMutex );
    B_Mutex_Destroy( pSession->hMutex );
    BKNI_Free( pSession );
    pSession = NULL;
    BDBG_LEAVE( destroyMulticastSession );
    return( 0 );
}

static BIP_Status addMulticastSessionToList(
    BIP_IgmpListenerHandle     hIgmpListener,
    B_RTSP_MulticastSession_T *pSession
    )
{
    BDBG_ASSERT( pSession );
    BDBG_ENTER( addMulticastSessionToList );
    B_Mutex_Lock( hIgmpListener->mutexIGMPdata );
    /* Are there any more checks before adding to the list, Do I need a counter */
    BLST_S_INSERT_HEAD( &( hIgmpListener->MulticastSession_list ), pSession, node );

    B_Mutex_Unlock( hIgmpListener->mutexIGMPdata );

    BDBG_LEAVE( addMulticastSessionToList );
    return( BIP_SUCCESS );
}

static BIP_Status removeMulticastSessionFromList(
    BIP_IgmpListenerHandle      hIgmpListener,
    B_RTSP_MulticastSession_T **pSession,
    unsigned int                multicast_address
    )
{
    /* Delete based on field not session  */
    B_RTSP_MulticastSession_T *iter=NULL, *elem = NULL;

    /* sanity check */
    BDBG_ASSERT( hIgmpListener );
    BDBG_ENTER( removeMulticastSessionFromList );

    B_Mutex_Lock( hIgmpListener->mutexIGMPdata );

    if (!BLST_S_EMPTY( &( hIgmpListener->MulticastSession_list )))
    {
        iter = BLST_S_FIRST( &( hIgmpListener->MulticastSession_list ));
        while (iter) {
            elem = iter;
            iter = BLST_S_NEXT( iter, node );
            if (elem->multicast_address == multicast_address)
            {
                *pSession = elem;
                PRINTMSG_LIST(( BIP_MSG_PRE_FMT "Cancel timer for Verify Group Specific address" BIP_MSG_PRE_ARG));
                setGroupSpecificResponseTimer(hIgmpListener, elem, false);

                PRINTMSG_LIST(( BIP_MSG_PRE_FMT "Removing RTSPMulitcastSession  : %p with field %d\n" BIP_MSG_PRE_ARG, (void *)elem, multicast_address ));
                BLST_S_REMOVE( &( hIgmpListener->MulticastSession_list ), elem, B_RTSP_MulticastSession, node );
                B_Mutex_Unlock( hIgmpListener->mutexIGMPdata );

                return( BIP_SUCCESS );
            }
        }
        PRINTMSG_LIST(( "Removing RTSPMulitcastSession  : did not find elem in list with this multicast: %d", multicast_address ));
    }
    else
    {
        PRINTMSG_LIST(( "Removing RTSPMulitcastSession  : empty list of Multicast sessions" ));
    }
    B_Mutex_Unlock( hIgmpListener->mutexIGMPdata );
    BDBG_LEAVE( removeMulticastSessionFromList );
    return( BIP_ERR_NOT_AVAILABLE );
} /* removeMulticastSessionFromList */

static BIP_Status cleanMulticastSessionList(
    BIP_IgmpListenerHandle hIgmpListener
    )
{
    B_RTSP_MulticastSession_T *iter=NULL, *elem = NULL;

    BDBG_ENTER( cleanMulticastSessionList );

    /* sanity check */
    BDBG_ASSERT( hIgmpListener );

    if (!BLST_S_EMPTY( &( hIgmpListener->MulticastSession_list )))
    {
        iter = BLST_S_FIRST( &( hIgmpListener->MulticastSession_list ));
        while (iter) {
            elem = iter;
            iter = BLST_S_NEXT( iter, node );
            PRINTMSG_LIST(( BIP_MSG_PRE_FMT "Removing RTSPMulitcastSession from List: %p\n" BIP_MSG_PRE_ARG, (void *)elem ));
            BLST_S_REMOVE( &( hIgmpListener->MulticastSession_list ), elem, B_RTSP_MulticastSession, node );
            destroyMulticastSession( hIgmpListener, elem );
        }
    }
    BDBG_LEAVE( cleanMulticastSessionList );
    return( 0 );
} /* cleanMulticastSessionList */

static BIP_Status printMulitcastSessionList(
    BIP_IgmpListenerHandle hIgmpListener
    )
{
    B_RTSP_MulticastSession_T *pSession=NULL;
    int i = 0, temp=0;

    BDBG_ENTER( printMulitcastSessionList );
    PRINTMSG_LIST(( "--Printing List of Multicast Sessions --" ));
    pSession = BLST_S_FIRST( &( hIgmpListener->MulticastSession_list ));
    if(pSession ==NULL)
    {
        PRINTMSG_LIST(( "  Empty List of Multicast Sessions" ));
        PRINTMSG_LIST(( "------------------------\n" ));
        BDBG_LEAVE( printMulitcastSessionList );
        return( BIP_SUCCESS );
    }

    while (pSession != NULL)
    {
        PRINTMSG_LIST(( "ListIndex %d \n ", i++ ));
        PRINTMSG_LIST(( "SessionID %d \n ", pSession->sessionID ));
        temp = htonl( pSession->multicast_address );
        PRINTMSG_LIST(( "multicast address "BIP_INET_ADDR_FMT" \n ", BIP_INET_ADDR_ARG(temp)));
        PRINTMSG_LIST(( "groupSpecificQuerycount %d \n", pSession->groupSpecificQueryCount ));
        PRINTMSG_LIST(( "waitingforGroupReport %d \n ", pSession->waitingForGroupReport ));
        PRINTMSG_LIST(( "recieved GroupReport %d \n ", pSession->recievedGroupReport ));
        PRINTMSG_LIST(( "recieved JoinReport %d \n ", pSession->recievedJoinReport ));
        PRINTMSG_LIST(( "------------------------\n" ));
        pSession = BLST_S_NEXT( pSession, node );
    }
    BDBG_LEAVE( printMulitcastSessionList );

    return( BIP_SUCCESS );
} /* printMulitcastSessionList */

/* List entries to notify the upper layer of joins and leaves*/
/*! \brief open an Multicast session, initialize session data.
 *  \param[in] IGMPContext  IGMPContext core data poiner.
 *  \param[in] Multicast Address
 *  \param[in] Membership Report type either Join or Leave
 *  \param[in,out] pSession returned  multicast session handle, if success.
 */
/* General Membership report list  replace the join and leave list*/
static BIP_Status createMembershipReportNotification(
    BIP_IgmpListenerHandle                hIgmpListener,
    unsigned int                          multicast_address,
    BIP_IgmpListener_MembershipReportType memRepType,
    B_MembershipReportCallbacks_T       **mMembershipRepHandle
    )
{
    B_MembershipReportCallbacks_T *MembershipReport = NULL;
    BERR_Code                      retValue         = BERR_SUCCESS;

    /* sanity check */
    BDBG_ASSERT( hIgmpListener );
    BDBG_ASSERT( mMembershipRepHandle );

    BDBG_ENTER( createMembershipReportNotification );
    MembershipReport = BKNI_Malloc( sizeof( B_MembershipReportCallbacks_T ));
    BIP_CHECK_GOTO(( MembershipReport ), ( "Memory Allocation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, retValue );
    BKNI_Memset( MembershipReport, 0, sizeof( B_MembershipReportCallbacks_T ));

    MembershipReport->multicast_address = multicast_address;
    MembershipReport->memRepType        = memRepType;
    MembershipReport->pIGMPContext      = hIgmpListener;
    MembershipReport->hMutex            = B_Mutex_Create( NULL );
    BIP_CHECK_GOTO(( MembershipReport->hMutex ), ( "Failed to create mutex for Multicast session data" ), error, BIP_ERR_OS_CHECK_ERRNO, retValue );
    *mMembershipRepHandle = MembershipReport;

    BDBG_LEAVE( createMembershipReportNotification );
    return( retValue );

error:
    BDBG_LEAVE( createMembershipReportNotification );
    if (MembershipReport)
    {
        BKNI_Free( MembershipReport );
    }
    return( retValue );
} /* createMembershipReportNotification */

static BIP_Status destroyMembershipReportNotification(
    BIP_IgmpListenerHandle         hIgmpListener,
    B_MembershipReportCallbacks_T *mMembershipRepHandle
    )
{
    BSTD_UNUSED( hIgmpListener );
    BDBG_ASSERT( mMembershipRepHandle );
    BDBG_ENTER( destroyMembershipReportNotification );

    B_Mutex_Lock( mMembershipRepHandle->hMutex );
    /*TODO??: Is there anything else we need to destroy  */

    B_Mutex_Unlock( mMembershipRepHandle->hMutex );
    B_Mutex_Destroy( mMembershipRepHandle->hMutex );
    BKNI_Free( mMembershipRepHandle );
    mMembershipRepHandle = NULL;
    BDBG_LEAVE( destroyMembershipReportNotification );
    return( BERR_SUCCESS );
}

static BIP_Status addMembershipReportNotificationToList(
    BIP_IgmpListenerHandle         hIgmpListener,
    B_MembershipReportCallbacks_T *mMembershipRepHandle
    )
{
    BDBG_ASSERT( mMembershipRepHandle );
    BDBG_ENTER( addMembershipReportNotificationToList );
    /* Not needed as callback function which calls this sets the mutext. B_Mutex_Lock(hIgmpListener->mutexIGMPdata); */
    /* Are there any more checks before adding to the list, Do I need a counter */
    BLST_S_INSERT_HEAD( &( hIgmpListener->MembershipReports_list ), mMembershipRepHandle, node );

    /* Not needed as callback function which calls this sets the mutext.   B_Mutex_Unlock(hIgmpListener->mutexIGMPdata); */
    BDBG_LEAVE( addMembershipReportNotificationToList );
    return( BERR_SUCCESS );
}

static BIP_Status removeMembershipReportNotificationFromList(
    BIP_IgmpListenerHandle         hIgmpListener,
    B_MembershipReportCallbacks_T *mMembershipRepHandle
    )
{
    BDBG_ASSERT( mMembershipRepHandle );
    BDBG_ENTER( removeMembershipReportNotificationFromList );

    /* Not needed as callback function which calls this sets the mutext. B_Mutex_Lock(hIgmpListener->mutexIGMPdata);*/
    /* Are there any more checks before removing from the list */
    BLST_S_REMOVE( &( hIgmpListener->MembershipReports_list ), mMembershipRepHandle, B_MembershipReportCallbacks, node );

    /*
     * caller need to destroy the session
     */
    /* Not needed as callback function which calls this sets the mutext. B_Mutex_Unlock(hIgmpListener->mutexIGMPdata);*/
    BDBG_LEAVE( removeMembershipReportNotificationFromList );
    return( 0 );
}

static BIP_Status cleanMembershipReportNotificationList(
    BIP_IgmpListenerHandle hIgmpListener
    )
{
    B_MembershipReportCallbacks_T *iter=NULL, *elem = NULL;

    /* sanity check */
    BDBG_ASSERT( hIgmpListener );
    BDBG_ENTER( cleanMembershipReportNotificationList );

    if (!BLST_S_EMPTY( &( hIgmpListener->MembershipReports_list )))
    {
        iter = BLST_S_FIRST( &( hIgmpListener->MembershipReports_list ));
        while (iter) {
            elem = iter;
            iter = BLST_S_NEXT( iter, node );
            PRINTMSG_LIST(( BIP_MSG_PRE_FMT "Removing MembershipReport from List: %p\n" BIP_MSG_PRE_ARG, (void *)elem ));
            removeMembershipReportNotificationFromList( hIgmpListener, elem );
            destroyMembershipReportNotification( hIgmpListener, elem );
        }
    }

    BDBG_LEAVE( cleanMembershipReportNotificationList );
    return( 0 );
} /* cleanMembershipReportNotificationList */

/* Deletes all MembershipNotification for particular Multicast address */
static BIP_Status cleanMembershipReportNotificationListForMulticastAddress(
    BIP_IgmpListenerHandle hIgmpListener,
    unsigned int           multicast_address
    )
{
    B_MembershipReportCallbacks_T *iter=NULL, *elem = NULL;

    /* sanity check */
    BDBG_ASSERT( hIgmpListener );
    BDBG_ENTER( cleanMembershipReportNotificationList );

    if (!BLST_S_EMPTY( &( hIgmpListener->MembershipReports_list )))
    {
        iter = BLST_S_FIRST( &( hIgmpListener->MembershipReports_list ));
        while (iter != NULL) {
            elem = iter;
            iter = BLST_S_NEXT( iter, node );
            PRINTMSG_LIST(( BIP_MSG_PRE_FMT "iterator: %p\n" BIP_MSG_PRE_ARG, (void *)iter ));

            if ( elem->multicast_address == multicast_address )
            {
                PRINTMSG_LIST(( BIP_MSG_PRE_FMT "Removing MembershipReport from List: %p\n" BIP_MSG_PRE_ARG, (void *)elem ));
                removeMembershipReportNotificationFromList( hIgmpListener, elem );
                destroyMembershipReportNotification( hIgmpListener, elem );
            }
        }
    }

    BDBG_LEAVE( cleanMembershipReportNotificationList );
    return( 0 );
} /* cleanMembershipReportNotificationListForMulticastAddress */


/**
 * Returns 1 if found multicast address
 * Returns 0 if multicast address not found
 **/
static int searchMembershipReportNotificationListByMulticast(
    BIP_IgmpListenerHandle                hIgmpListener,
    unsigned int                          multicast_address,
    BIP_IgmpListener_MembershipReportType memRepType
    )
{
    B_MembershipReportCallbacks_T *iter=NULL, *elem = NULL;

    /* sanity check */
    BDBG_ASSERT( hIgmpListener );
    BDBG_ENTER( searchMembershipReportNotificationListByMulticast );

    if (!BLST_S_EMPTY( &( hIgmpListener->MembershipReports_list )))
    {
        iter = BLST_S_FIRST( &( hIgmpListener->MembershipReports_list ));
        while (iter) {
            elem = iter;
            if (( elem->multicast_address == multicast_address ) && ( elem->memRepType == memRepType ))
            {
                return( 1 );
            }

            iter = BLST_S_NEXT( iter, node );
        }
    }

    BDBG_LEAVE( searchMembershipReportNotificationListByMulticast );
    return( 0 );
} /* searchMembershipReportNotificationListByMulticast */

static BIP_Status printMembershipReportNotificationList(
    BIP_IgmpListenerHandle hIgmpListener
    )
{
    B_MembershipReportCallbacks_T *membershipReport=NULL;
    int i = 0, temp=0;

    BDBG_ENTER( printMembershipReportNotificationList );
    membershipReport = BLST_S_FIRST( &( hIgmpListener->MembershipReports_list ));
    if (membershipReport==NULL)
    {
        PRINTMSG_LIST(( "MembershipReports_list is empty  \n " ));
    }

    while (membershipReport != NULL)
    {
        PRINTMSG_LIST(( "MembershipReportListIndex %d \n ", i++ ));
        temp = htonl( membershipReport->multicast_address );
        PRINTMSG_LIST(( " multicast address "BIP_INET_ADDR_FMT" \n ", BIP_INET_ADDR_ARG(temp)));
        PRINTMSG_LIST(( " memRepType %d \n", membershipReport->memRepType ));
        PRINTMSG_LIST(( "------------------------\n" ));
        membershipReport = BLST_S_NEXT( membershipReport, node );
    }

    BDBG_LEAVE( printMembershipReportNotificationList );

    return( 0 );
} /* printMembershipReportNotificationList */

/*End  of  List entries to notify the upper layer of joins and leaves*/

/* the multicast address need to be in HOST order  format
    i.e. ntohl(inet_addr("239.1.x.x"))  will convert string
*/
BIP_Status BIP_IgmpListener_AddGroupAddress(
    BIP_IgmpListenerHandle hIgmpListener,
    unsigned int           multicast_address,
    unsigned int           sessionID
    )
{
    BIP_Status                  rc=0;
    B_RTSP_MulticastSession_T *pSession = NULL;

    BDBG_ENTER( BIP_IgmpListener_AddGroupAddress );

    if (!IN_MULTICAST(multicast_address))
    {
        BDBG_ERR(( BIP_MSG_PRE_FMT "multicast address is not valid." BIP_MSG_PRE_ARG ));
        return BIP_ERR_INVALID_PARAMETER;
    }

    /* check is multicast Address is already in the list.  */
    if (findMulitcastAddressSessionList( hIgmpListener, multicast_address, &pSession ))
    {
        BDBG_MSG((  BIP_MSG_PRE_FMT "This Multicast Already has been added to IgmpListener's list of Multicast Addresses" BIP_MSG_PRE_ARG));
        BDBG_LEAVE( BIP_IgmpListener_AddGroupAddress );
        return( BIP_SUCCESS );
    }
    else
    {
        /* Unfound multicast address */
        BDBG_MSG(( BIP_MSG_PRE_FMT "Add group Address to internal Session List" BIP_MSG_PRE_ARG));
        rc = createMulticastSession( hIgmpListener, multicast_address, sessionID, &pSession );
        if (!rc)
        {
            addMulticastSessionToList( hIgmpListener, pSession );

            /*Debug */
            printMulitcastSessionList( hIgmpListener );
            BDBG_MSG(( BIP_MSG_PRE_FMT "Multicast session created and added to list" BIP_MSG_PRE_ARG));

            BDBG_LEAVE( BIP_IgmpListener_AddGroupAddress );
            return( BIP_SUCCESS );
        }
        else
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "Unable to create Multicast Sesstion for internal list bip error %d" BIP_MSG_PRE_ARG, rc ));
            BDBG_LEAVE( BIP_IgmpListener_AddGroupAddress );
            return( BIP_ERR_INTERNAL );
        }
    }
} /* BIP_IgmpListener_AddGroupAddress */

/* the multicast address need to be in HOST order  format
    i.e. ntohl(inet_addr("239.1.x.x"))  will convert string
*/
BIP_Status BIP_IgmpListener_DelGroupAddress(
    BIP_IgmpListenerHandle hIgmpListener,
    unsigned int           multicast_address
    )
{
    /*Find  handle to the session */
    BIP_Status                  rc       = BIP_SUCCESS;
    B_RTSP_MulticastSession_T *pSession = NULL;

    BDBG_ENTER( BIP_IgmpListener_DelGroupAddress );
    printMembershipReportNotificationList( hIgmpListener);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Delete MembershipReportNotifications for Particular Multicast Address from internal notifcation List" BIP_MSG_PRE_ARG));
    cleanMembershipReportNotificationListForMulticastAddress( hIgmpListener, multicast_address);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Delete group Address from  internal Session List" BIP_MSG_PRE_ARG));
    rc = removeMulticastSessionFromList( hIgmpListener, &pSession, multicast_address );
    if (rc || ( pSession==NULL ))
    {
        return( rc );
    }
    else
    {
        destroyMulticastSession( hIgmpListener, pSession );
    }
    /* debug */
    printMulitcastSessionList( hIgmpListener );

    BDBG_LEAVE( BIP_IgmpListener_DelGroupAddress );

    return( rc );
} /* BIP_IgmpListener_DelGroupAddress */

/*error code is BIP_NOT_AVAIL is there is an empty list*/
BIP_Status BIP_IgmpListener_GetMembershipReportStatus(
    BIP_IgmpListenerHandle                     hIgmpListener,
    BIP_IgmpListener_MembershipReportStatus_T *pStatus
    )
{
    /* Pop off first in list */
    B_MembershipReportCallbacks_T *elem = NULL;

    /* sanity check */
    BDBG_ASSERT( hIgmpListener );

    BDBG_ENTER( BIP_IgmpListener_GetMembershipReportStatus );

    printMembershipReportNotificationList( hIgmpListener );
    if (!BLST_S_EMPTY( &( hIgmpListener->MembershipReports_list )))
    {
        unsigned int temp;

        elem = BLST_S_FIRST( &( hIgmpListener->MembershipReports_list ));
        pStatus->multicast_address = elem->multicast_address;
        pStatus->memRepType        = elem->memRepType;

        temp = htonl( pStatus->multicast_address );

        BDBG_MSG(( BIP_MSG_PRE_FMT "Removing MembershipReport from List: %p, multicast_address "BIP_INET_ADDR_FMT" membershipReport Type %d\n" BIP_MSG_PRE_ARG,
                    (void *)elem, BIP_INET_ADDR_ARG(temp), elem->memRepType ));
        removeMembershipReportNotificationFromList( hIgmpListener, elem );
        destroyMembershipReportNotification( hIgmpListener, elem );

        BDBG_LEAVE( BIP_IgmpListener_GetMembershipReportStatus );
        return( BIP_SUCCESS );
    }
    else
    {
        return( BIP_ERR_NOT_AVAILABLE );
    }
} /* BIP_IgmpListener_GetJoinStatus */

void BIP_IgmpListener_GetDefaultCreateSettings(
    BIP_IgmpListenerCreateSettings *pSettings
    )
{
    BDBG_ENTER( BIP_IgmpListener_GetDefaultCreateSettings );
    pSettings->igmp_v2_support = false;  /* set for  Sat Ip Features */
    pSettings->igmp_v3_support = true;   /* SesSatIP only uses igmp_v3 */

    /*what about  igmpv v4 vs igmp v6 ?? */
    pSettings->lan_iface = "eth0"; /*  eth0 vs eth2*/

    pSettings->enableSesSatIpFeatures = false;

    /* add connection timeout  */
    pSettings->device_id               = 1;      /* used in second octet of  multicast address 239.<device_id>.x.x*/
    pSettings->gen_query_interval      = 125000; /*125000 ms */
    pSettings->querier_interval        = 255000; /*255000 ms */
    pSettings->gen_response_interval   = 10000;  /*10000 ms */
    pSettings->group_specific_response = 1000;   /* 1000 ms */
    BDBG_LEAVE( BIP_IgmpListener_GetDefaultCreateSettings );
} /* BIP_IgmpListener_GetDefaultCreateSettings */

void BIP_IgmpSesSatIpListener_GetDefaultCreateSettings(
    BIP_IgmpListenerCreateSettings *pSettings
    )
{
    BDBG_ENTER( BIP_IgmpSesSatIpListener_GetDefaultCreateSettings );
    pSettings->igmp_v2_support = false;  /* set for  Sat Ip Features */
    pSettings->igmp_v3_support = true;   /* SesSatIP only uses igmp_v3 */

    /*what about  igmpv v4 vs igmp v6 ?? */
    pSettings->lan_iface = "eth0"; /*  eth0 vs eth2*/

    pSettings->enableSesSatIpFeatures = true;

    /* add connection timeout  */
    pSettings->device_id               = 1;      /* used in second octet of  multicast address*/
    pSettings->gen_query_interval      = 125000; /*125000 ms */
    pSettings->querier_interval        = 255000; /*255000 ms */
    pSettings->gen_response_interval   = 10000;  /*10000 ms */
    pSettings->group_specific_response = 1000;   /* 1000 ms */
    BDBG_LEAVE( BIP_IgmpSesSatIpListener_GetDefaultCreateSettings );
} /* BIP_IgmpSesSatIpListener_GetDefaultCreateSettings */

BIP_IgmpListenerHandle BIP_IgmpListener_Create(
    BIP_IgmpListenerCreateSettings *pSettings
    )
{
    struct ifreq           ifr;
    struct ip_mreq         mreq;
    int                    optval = 1;
    BIP_IgmpListenerHandle hIgmpListner;

    hIgmpListner = (BIP_IgmpListenerHandle) BKNI_Malloc( sizeof( *hIgmpListner ));

    BKNI_Memset( hIgmpListner, 0, sizeof( *hIgmpListner ));
    BDBG_OBJECT_SET( hIgmpListner, BIP_IgmpListener );
    BKNI_Memset( &hIgmpListner->settings, 0, sizeof( BIP_IgmpListenerSettings ));
    BKNI_Memset( &ifr, 0, sizeof( ifr ));
    BKNI_Memset( &mreq, 0, sizeof( mreq ));

    BDBG_ENTER( BIP_IgmpListener_Create );

    if (NULL == pSettings)
    {
        BIP_IgmpListener_GetDefaultCreateSettings( &hIgmpListner->createSettings );
    }
    else
    {
        /* If caller passed CreateSettings, use those. */
        BKNI_Memcpy( &hIgmpListner->createSettings, pSettings, sizeof( hIgmpListner->createSettings ));
    }
    pSettings = &hIgmpListner->createSettings;  /* Now we can use our own copy of the CreateSettings. */

    /*Create a raw socket that shall recieve */
    hIgmpListner->igmpSock = socket( AF_INET, SOCK_RAW, IPPROTO_IGMP );
    if (hIgmpListner->igmpSock < 0)
    {
        BDBG_ERR(( BIP_MSG_PRE_FMT "Can't create socket\n" BIP_MSG_PRE_ARG));
        goto error;
    }

    /* From David E suggestion.  allow immediate reuse of sockets in TIME_WAIT state, at risk of receiving stale data */
    if (setsockopt( hIgmpListner->igmpSock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval ))
    {
        BDBG_ERR(( BIP_MSG_PRE_FMT "Can't set option to allow immediate reuse of socket. \n" BIP_MSG_PRE_ARG));
        goto error;
    }

    /* Find own ip_address */
    /*Type of address to retrieve - IPv4 IP address*/
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy( ifr.ifr_name, pSettings->lan_iface, IFNAMSIZ-1 ); /*Copy the interface name in the ifreq structure */
    if (ioctl( hIgmpListner->igmpSock, SIOCGIFADDR, &ifr ) != 0)
    {
        BDBG_ERR(( BIP_MSG_PRE_FMT "Can't get interface address of socket. \n" BIP_MSG_PRE_ARG));
        goto error;
    }

    hIgmpListner->host_inaddr = ((struct sockaddr_in *)&ifr.ifr_addr )->sin_addr;
    BDBG_MSG(( BIP_MSG_PRE_FMT "%s - "BIP_INET_ADDR_FMT"\n" BIP_MSG_PRE_ARG,  pSettings->lan_iface, BIP_INET_ADDR_ARG(hIgmpListner->host_inaddr)));

    /* Allocated buffer to recv from socket */
    hIgmpListner->recvBuffer = (unsigned char *)BKNI_Malloc( MAX_BUFFER_SIZE );
    BKNI_Memset(hIgmpListner->recvBuffer, 0,  MAX_BUFFER_SIZE );

    /* Add membership to ALL_ROUTERS and ALL_ROUTERS_V3 on this interface */
    /* Not needed if we follow strictly IGMPv3*/
    mreq.imr_multiaddr.s_addr = inet_addr( IGMP_ALL_ROUTERS );
    mreq.imr_interface.s_addr = htonl( INADDR_ANY );
    if (setsockopt( hIgmpListner->igmpSock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
            (void *)&mreq, sizeof( mreq )) < 0)
    {
        BDBG_ERR(( BIP_MSG_PRE_FMT "IP_ADD_MEMBERSHIP fails for %s\n" BIP_MSG_PRE_ARG, IGMP_ALL_ROUTERS ));
        goto error;
    }

    mreq.imr_multiaddr.s_addr = inet_addr( IGMP_ALL_ROUTERS_V3 );
    mreq.imr_interface.s_addr = htonl( INADDR_ANY );

    if (setsockopt( hIgmpListner->igmpSock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
            (void *)&mreq, sizeof( mreq )) < 0)
    {
        BDBG_ERR(( BIP_MSG_PRE_FMT "IP_ADD_MEMBERSHIP fails for %s\n" BIP_MSG_PRE_ARG, IGMP_ALL_ROUTERS_V3 ));
        goto error;
    }

    /* Setup Multicast List */
    BLST_S_INIT( &hIgmpListner->MulticastSession_list );
    /*Setup Membership Reports Callback  List*/
    BLST_S_INIT( &hIgmpListner->MembershipReports_list );
    BDBG_MSG(( BIP_MSG_PRE_FMT "successful" BIP_MSG_PRE_ARG));

    BDBG_LEAVE( BIP_IgmpListener_Create );
    return( hIgmpListner );

error:
    BKNI_Free( hIgmpListner );  /* not sure if we should destroy the igmpListener if can't create socket */
    return( NULL );
} /* BIP_IgmpListener_Create */

BIP_Status BIP_IgmpListener_GetSettings(
    BIP_IgmpListenerHandle    hIgmpListener,
    BIP_IgmpListenerSettings *pSettings
    )
{
    BIP_Status errCode = BIP_SUCCESS;

    BDBG_ENTER( BIP_IgmpListener_GetSettings );
    BDBG_OBJECT_ASSERT( hIgmpListener, BIP_IgmpListener );
    BDBG_ASSERT( pSettings );
    *pSettings = hIgmpListener->settings;
    BDBG_LEAVE( BIP_IgmpListener_GetSettings );
    return( errCode );
}

/**
 * Summary:
 * API to Set Listener Settings
 *
 * Description:
 **/
BIP_Status BIP_IgmpListener_SetSettings(
    BIP_IgmpListenerHandle    hIgmpListener,
    BIP_IgmpListenerSettings *pSettings
    )
{
    BIP_Status errCode = BIP_SUCCESS;

    BDBG_ENTER( BIP_IgmpListener_SetSettings );
    BDBG_OBJECT_ASSERT( hIgmpListener, BIP_IgmpListener );
    BDBG_ASSERT( pSettings );

    /* validate parameters */
    hIgmpListener->settings = *pSettings;
    BDBG_LEAVE( BIP_IgmpListener_SetSettings );

    return( errCode );
}

/* Start of  Timers (4)  */

/*
  1 second reponse timer. If you  dont' get your response do it again. for another 1 second and then
*/
static void verifySpecificMulticast(
    void *session
    )
{
    B_RTSP_MulticastSession_T *pSession      = (B_RTSP_MulticastSession_T *) session;
    BIP_IgmpListenerHandle     hIgmpListener = (BIP_IgmpListenerHandle) pSession->pIGMPContext;

    BDBG_ENTER( verifySpecificMulticast );

    /* Check List */
    if (pSession->waitingForGroupReport && pSession->recievedGroupReport)
    {
        /* recieved group Report  after a LEAVE, reset variables*/
        BDBG_MSG(( BIP_MSG_PRE_FMT "Recieved JOIN before Group Specifc Response expires" BIP_MSG_PRE_ARG));
        B_Mutex_Lock( pSession->hMutex );
        pSession->groupSpecificQueryCount = 0;
        pSession->waitingForGroupReport   = false;
        pSession->recievedGroupReport     = false;
        B_Mutex_Unlock( pSession->hMutex );
    }
    else  /* did not receive JOIN yet */
    {
        if (pSession->groupSpecificQueryCount < 2)
        {
            BDBG_MSG(( BIP_MSG_PRE_FMT "group specific timer expires once. Reset group specific timer one more time" BIP_MSG_PRE_ARG));
            B_Mutex_Lock( pSession->hMutex );
            pSession->groupSpecificQueryCount++;
            B_Mutex_Unlock( pSession->hMutex );
            sendIgmpQueryMessage( hIgmpListener, pSession->multicast_address );

            pSession->groupSpecificResponseTimer = B_Scheduler_StartTimer(
                    hIgmpListener->schedulerIGMPdata, hIgmpListener->mutexIGMPdata, hIgmpListener->createSettings.group_specific_response, verifySpecificMulticast, pSession );
            if (pSession->groupSpecificResponseTimer==NULL) {BDBG_ERR(( BIP_MSG_PRE_FMT "group sepcific response timer error" BIP_MSG_PRE_ARG )); }
            pSession->groupSpecificResponseTimerIsStarted = true;
        }
        else  /* did not recieve JOIN after 2 */
        {
            int temp = htonl( pSession->multicast_address );

            /* Don't send  callback unless there has been a Join on the session*/
            if (pSession->recievedJoinReport ==false)
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "Session( "BIP_INET_ADDR_FMT" ) has Not recieved a Join(In passive mode). Skip Sending Callback. \n" BIP_MSG_PRE_ARG,
                           BIP_INET_ADDR_ARG(temp) ));
            }
            /* check if multicast address is already in the Leave Notification list */
            else if (searchMembershipReportNotificationListByMulticast( hIgmpListener, pSession->multicast_address, BIP_IgmpListener_MembershipReportType_eLeave ))
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "Found this mulitcastAddress( "BIP_INET_ADDR_FMT" ) already in LeaveReport in  MembershipReportNotificationList.Not Adding \n" BIP_MSG_PRE_ARG,
                           BIP_INET_ADDR_ARG(temp) ));
            }
            else
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "Insert this mulitcastAddress( "BIP_INET_ADDR_FMT" ) as LeaveReport to MembershipReportNotificationList. \n" BIP_MSG_PRE_ARG,
                           BIP_INET_ADDR_ARG(temp) ));

                /* Add  multicast to Leave membership status  to our list */
                B_MembershipReportCallbacks_T *memReport=NULL;
                createMembershipReportNotification( hIgmpListener, pSession->multicast_address, BIP_IgmpListener_MembershipReportType_eLeave, &memReport );
                addMembershipReportNotificationToList( hIgmpListener, memReport );
                /*Send leave callback back to App */
                if (hIgmpListener->settings.membershipReportCallback.callback)
                {
                    ( *hIgmpListener->settings.membershipReportCallback.callback )( hIgmpListener->settings.membershipReportCallback.context, hIgmpListener->settings.membershipReportCallback.param );
                }
            }
            /*reset variables*/
            BDBG_MSG(( BIP_MSG_PRE_FMT "group specific timer expires twice, No JOIN recieved" BIP_MSG_PRE_ARG));
            B_Mutex_Lock( pSession->hMutex );
            pSession->groupSpecificQueryCount = 0;
            pSession->waitingForGroupReport   = false;
            pSession->recievedGroupReport     = false;
            B_Mutex_Unlock( pSession->hMutex );
        }
    }

    BDBG_LEAVE( verifySpecificMulticast );
} /* verifySpecificMulticast */

static void setGroupSpecificResponseTimer(
    BIP_IgmpListenerHandle     hIgmpListener,
    B_RTSP_MulticastSession_T *pSession,
    bool                       activate
    )
{
    BDBG_ENTER( setGroupSpecificResponseTimer );
    /*Schedule Group specific */
    if (activate ==false) /* turn off */
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT "Stop group specific Response Timer( %d ms)" BIP_MSG_PRE_ARG, hIgmpListener->createSettings.group_specific_response  ));
        pSession->groupSpecificResponseTimerIsStarted = false;
        if (NULL != pSession->groupSpecificResponseTimer)
        {
            B_Scheduler_CancelTimer( hIgmpListener->schedulerIGMPdata, pSession->groupSpecificResponseTimer );
            pSession->groupSpecificResponseTimer = NULL;
        }
    }
    else
    {
        /*Send group Specific Query */
        sendIgmpQueryMessage( hIgmpListener, pSession->multicast_address );
        pSession->groupSpecificQueryCount++;
        BDBG_MSG(( BIP_MSG_PRE_FMT "Start group specific Response Timer( %d ms)" BIP_MSG_PRE_ARG, hIgmpListener->createSettings.group_specific_response  ));

        pSession->groupSpecificResponseTimer = B_Scheduler_StartTimer(
                hIgmpListener->schedulerIGMPdata, hIgmpListener->mutexIGMPdata /*  Do I need this  as session or listener*/, hIgmpListener->createSettings.group_specific_response, verifySpecificMulticast, pSession );
        if (pSession->groupSpecificResponseTimer==NULL) {BDBG_ERR(( BIP_MSG_PRE_FMT "group sepcific response timer error"  BIP_MSG_PRE_ARG)); }
        pSession->groupSpecificResponseTimerIsStarted = true;  /* TODO ?? do I need to wrap this wiht Mutex, will complain that it will be a recursive mutex if I do */
    }

    BDBG_LEAVE( setGroupSpecificResponseTimer );
} /* setGroupSpecificResponseTimer */

/*
  Timer for Response to Genral Queries
  10 second reponse timer. After 10 seconds make sure that you have recieved a response for all of your streams
*/

static void verifyAllMulticasts(
    void *context
    )
{
    BIP_IgmpListenerHandle     hIgmpListener = (BIP_IgmpListenerHandle) context;
    B_RTSP_MulticastSession_T *pSession=NULL;

    BDBG_ENTER( verifyAllMulticasts );
    BDBG_MSG(( BIP_MSG_PRE_FMT "General Response timer(%d ms) expired.Check if active on All multicast sessions in IGMP Listener's List" BIP_MSG_PRE_ARG, hIgmpListener->createSettings.gen_response_interval ));
#if 1
    PRINTMSG_LIST(( BIP_MSG_PRE_FMT "Printing Active MulticastSession of List:" BIP_MSG_PRE_ARG));

    printMulitcastSessionList( hIgmpListener );
#endif
    pSession = BLST_S_FIRST( &( hIgmpListener->MulticastSession_list ));
    if(pSession ==NULL)
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT "No Multicast Sessions in IGMP Listener's List" BIP_MSG_PRE_ARG));
    }
    while (pSession != NULL)
    {
        B_Mutex_Lock( pSession->hMutex );
        /* JOIN Recieve keep session alive */
        if (pSession->waitingForGroupReport && pSession->recievedGroupReport)
        {
            BDBG_MSG(( BIP_MSG_PRE_FMT "Found join response for session %d. keep session alive" BIP_MSG_PRE_ARG, pSession->sessionID ));
        }
        else /* no join recieved.  CMD_STOP */
        {
            int temp = htonl( pSession->multicast_address );

            BDBG_MSG(( BIP_MSG_PRE_FMT "Did not find response for session %d. submit LEAVE" BIP_MSG_PRE_ARG, pSession->sessionID ));
            /* Don't send  callback unless there has been a Join on the session*/
            if (pSession->recievedJoinReport ==false)
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "Session( "BIP_INET_ADDR_FMT" ) has Not recieved a Join(In passive mode). Skip Sending Callback. \n" BIP_MSG_PRE_ARG,
                           BIP_INET_ADDR_ARG(temp) ));
            }
            /* check if multicast address is already in the Leave Notification list */
            else if (searchMembershipReportNotificationListByMulticast( hIgmpListener, pSession->multicast_address, BIP_IgmpListener_MembershipReportType_eLeave ))
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "Found this mulitcastAddress( "BIP_INET_ADDR_FMT" ) already in LeaveReport in  MembershipReportNotificationList.Not Adding \n" BIP_MSG_PRE_ARG,
                           BIP_INET_ADDR_ARG(temp) ));
            }
            else
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "Insert this  mulitcastAddress( "BIP_INET_ADDR_FMT" ) as LeaveReport to MembershipReportNotificationList. \n" BIP_MSG_PRE_ARG,
                           BIP_INET_ADDR_ARG(temp) ));

                /* Add  multicast to Leave membership status  to our list */
                B_MembershipReportCallbacks_T *memReport=NULL;
                createMembershipReportNotification( hIgmpListener, pSession->multicast_address, BIP_IgmpListener_MembershipReportType_eLeave, &memReport );
                addMembershipReportNotificationToList( hIgmpListener, memReport );
                /*Send leave callback back to App */
                if (hIgmpListener->settings.membershipReportCallback.callback)
                {
                    ( *hIgmpListener->settings.membershipReportCallback.callback )( hIgmpListener->settings.membershipReportCallback.context, hIgmpListener->settings.membershipReportCallback.param );
                }
            }
        }

        pSession->waitingForGroupReport = false;
        pSession->recievedGroupReport   = false;
        B_Mutex_Unlock( pSession->hMutex );
        pSession = BLST_S_NEXT( pSession, node );
    }

    BDBG_LEAVE( verifyAllMulticasts );
} /* verifyAllMulticasts */

/* This  is a 10 second response timeer in the SES Satip spec */
static void setGeneralResponseTimer(
    BIP_IgmpListenerHandle hIgmpListener,
    bool                   activate
    )
{
    BDBG_ENTER( setGeneralResponseTimer );
    if (activate ==false)
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT "Stop General Response Timer (%d ms) " BIP_MSG_PRE_ARG, hIgmpListener->createSettings.gen_response_interval ));
        hIgmpListener->querierTimerIsStarted = false;
        if (NULL != hIgmpListener->generalResponseTimer)
        {
            B_Scheduler_CancelTimer( hIgmpListener->schedulerIGMPdata, hIgmpListener->generalQueryTimer );
            hIgmpListener->generalResponseTimer = NULL;
        }
    }
    else
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT "Start General Response Timer(%d ms) " BIP_MSG_PRE_ARG, hIgmpListener->createSettings.gen_response_interval ));
        hIgmpListener->generalResponseTimer = B_Scheduler_StartTimer(
                hIgmpListener->schedulerIGMPdata, hIgmpListener->mutexIGMPdata, hIgmpListener->createSettings.gen_response_interval, verifyAllMulticasts, hIgmpListener );
        if (hIgmpListener->generalResponseTimer==NULL) {BDBG_MSG(( BIP_MSG_PRE_FMT "general response timer error" BIP_MSG_PRE_ARG )); }
        hIgmpListener->querierTimerIsStarted = true;
    }

    BDBG_LEAVE( setGeneralResponseTimer );
} /* setGeneralResponseTimer */

/*
  Timer for sending out period General Queries
  125 Second general query
*/
static void sendGeneralQueryMessage(
    void *context
    )
{
    BIP_IgmpListenerHandle hIgmpListener = context;

    BDBG_ENTER( sendGeneralQueryMessage );
    /* 1.Send Query
        2. Schedule timer*/
    if (hIgmpListener->electedQuerier)
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT "General Query Timer(%d ms) expired. Send General Query" BIP_MSG_PRE_ARG, hIgmpListener->createSettings.gen_query_interval ));
        sendIgmpQueryMessage( hIgmpListener, 0 );
    }
    else
    {
        BDBG_ERR(( BIP_MSG_PRE_FMT "Not Querier but  told to sending Query. Error" BIP_MSG_PRE_ARG ));
        return;
    }

    if (hIgmpListener->generalQueryTimerIsStarted)
    {

        BDBG_MSG(( BIP_MSG_PRE_FMT "Rescheduled General Query Timer" BIP_MSG_PRE_ARG));
        hIgmpListener->generalQueryTimer = B_Scheduler_StartTimer(
                hIgmpListener->schedulerIGMPdata, hIgmpListener->mutexIGMPdata, hIgmpListener->createSettings.gen_query_interval, sendGeneralQueryMessage, hIgmpListener );
        if (hIgmpListener->generalQueryTimer==NULL) {BDBG_ERR(( BIP_MSG_PRE_FMT "schedule timer error %d" BIP_MSG_PRE_ARG, NEXUS_OUT_OF_SYSTEM_MEMORY )); }
    }

    BDBG_LEAVE( sendGeneralQueryMessage );
    return;
} /* sendGeneralQueryMessage */

/* setAsElectedQuerier  was setGeneralQuery*/
static void setAsElectedQuerier(
    BIP_IgmpListenerHandle hIgmpListener,
    bool                   actvateGeneralQuery
    )
{
    BDBG_ENTER( setAsElectedQuerier );

    /* schedule querier interval */
    if (actvateGeneralQuery ==false)  /* turn off */
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT "Stop General Query Timer(%d ms)" BIP_MSG_PRE_ARG, hIgmpListener->createSettings.gen_query_interval  ));
        hIgmpListener->electedQuerier             = false;
        hIgmpListener->generalQueryTimerIsStarted = false;
        if (NULL != hIgmpListener->generalQueryTimer)
        {
            B_Scheduler_CancelTimer( hIgmpListener->schedulerIGMPdata, hIgmpListener->generalQueryTimer );
            hIgmpListener->generalQueryTimer = NULL;
        }
    }
    else /* turn on*/
    {
        /*send the general query message */
        sendIgmpQueryMessage( hIgmpListener, 0 );

        hIgmpListener->electedQuerier = true;
        if (hIgmpListener->sentFirstGenQryQuarter == false)
        {
            /* Schedule timer to be Quarter of delay for first General Query */
            BDBG_MSG(( BIP_MSG_PRE_FMT "Start. Schedule General Query timer to a Quarter of its delay( %d ms)" BIP_MSG_PRE_ARG, hIgmpListener->createSettings.gen_query_interval/4 ));
            hIgmpListener->generalQueryTimer = B_Scheduler_StartTimer(
                    hIgmpListener->schedulerIGMPdata, hIgmpListener->mutexIGMPdata, ( hIgmpListener->createSettings.gen_query_interval/4 ), sendGeneralQueryMessage, hIgmpListener );

            hIgmpListener->sentFirstGenQryQuarter = true; /* From now on schedule full delay between General Queries */
        }
        else
        {
            /* Use standard delay */
            BDBG_MSG(( BIP_MSG_PRE_FMT "Start. Schedule General Query timer to %d ms" BIP_MSG_PRE_ARG, hIgmpListener->createSettings.gen_query_interval ));
            hIgmpListener->generalQueryTimer = B_Scheduler_StartTimer(
                    hIgmpListener->schedulerIGMPdata, hIgmpListener->mutexIGMPdata, hIgmpListener->createSettings.gen_query_interval, sendGeneralQueryMessage, hIgmpListener );
        }

        if (hIgmpListener->generalQueryTimer==NULL) {BDBG_ERR(( BIP_MSG_PRE_FMT " general query timer error" BIP_MSG_PRE_ARG )); }
        hIgmpListener->generalQueryTimerIsStarted = true;
    }
    BDBG_LEAVE( setAsElectedQuerier );
} /* setAsElectedQuerier */

/*
Timer for see if heard a General Query from an Elected Querier(not myself).
 255 second Querier
 */
static void enableGeneralQuery(
    void *context
    )
{
    BIP_IgmpListenerHandle hIgmpListener = context;

    BDBG_ENTER( enableGeneralQuery );
    /* Start General Query */
    BDBG_MSG(( BIP_MSG_PRE_FMT "Have not heard another GeneralQuery:  Myself becomes General Query" BIP_MSG_PRE_ARG ));
    setAsElectedQuerier( hIgmpListener, true );

    BDBG_LEAVE( enableGeneralQuery );
}

/** setHeardFromElectedQuerierTimer,
    true if  heard from  Elected Querier( start timer),
    false if you are the querier. stop timer.  **/
static void setHeardFromElectedQuerierTimer(
    BIP_IgmpListenerHandle hIgmpListener,
    bool                   activate
    )
{
    BDBG_ENTER( setHeardFromElectedQuerierTimer );
    if (activate ==false)
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT "Stop Querier Timer(%d ms)" BIP_MSG_PRE_ARG, hIgmpListener->createSettings.querier_interval ));
        hIgmpListener->querierTimerIsStarted = false;
        if (NULL != hIgmpListener->querierTimer)
        {
            B_Scheduler_CancelTimer( hIgmpListener->schedulerIGMPdata, hIgmpListener->querierTimer );
            hIgmpListener->querierTimer = NULL;
        }
    }
    else
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT "Start Querier Timer(%d ms)" BIP_MSG_PRE_ARG, hIgmpListener->createSettings.querier_interval ));

        hIgmpListener->querierTimer = B_Scheduler_StartTimer(
                hIgmpListener->schedulerIGMPdata, hIgmpListener->mutexIGMPdata, hIgmpListener->createSettings.querier_interval, enableGeneralQuery, hIgmpListener );
        if (hIgmpListener->querierTimer==NULL) {BDBG_ERR(( BIP_MSG_PRE_FMT "querier timer error" BIP_MSG_PRE_ARG )); }
        hIgmpListener->querierTimerIsStarted = true;
    }

    BDBG_LEAVE( setHeardFromElectedQuerierTimer );
} /* setHeardFromElectedQuerierTimer */

/* End of Timers */
static BIP_Status processIGMPPacket(
    BIP_IgmpListenerHandle hIgmpListener
    )
{
    BIP_Status          rc = BIP_SUCCESS;
    unsigned short     iphdrlen=0;
    int                data_size=0;
    socklen_t          saddr_size;
    struct sockaddr_in saddr;
    /*Get the IP Header part of this packet */
    struct iphdr         *iph = (struct iphdr *)hIgmpListener->recvBuffer;
    struct igmpv3_query  *igmpv3_query_hdr=NULL;
    struct igmpv3_report *igmpv3_rep_hdr=NULL;

    BDBG_ENTER( processIGMPPacket );

    BKNI_Memset(&saddr, 0, sizeof(saddr));
    saddr_size            = sizeof( saddr );
    saddr.sin_family      = AF_INET;
    saddr.sin_addr.s_addr = htonl( INADDR_ANY );
    saddr.sin_port        = htons( 1234 );

    data_size = recvfrom( hIgmpListener->igmpSock, hIgmpListener->recvBuffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&saddr, &saddr_size );
    /*data_size = recv(sock_raw , buffer , sizeof(buffer) , 0); */
    BIP_CHECK_GOTO(( data_size>=0 ), ( "Recvfrom error , failed to get packets" ), error, BIP_ERR_INTERNAL, rc );
    /*Now extract data from Recieved Packet */
    PRINTMSG_PACKET(( BIP_MSG_PRE_FMT "Recieved and Begin Processing Packet of data size %d --\n" BIP_MSG_PRE_ARG, data_size ));

    iphdrlen         = iph->ihl*4;
    igmpv3_query_hdr = (struct igmpv3_query *)( hIgmpListener->recvBuffer + iphdrlen );  /* don't think we need this */
    igmpv3_rep_hdr   = (struct igmpv3_report *)( hIgmpListener->recvBuffer + iphdrlen );
#if 0
    {
        int tcp = 0, udp = 0, icmp = 0, others = 0, igmp = 0, total = 0;
        ++total;
        switch (iph->protocol) /*Check the Protocol and do accordingly... */
        {
            case 1:
            {   /*ICMP Protocol */
                ++icmp;
                break;
            }

            case 2:
            {   /*IGMP Protocol */
                ++igmp;
                break;
            }

            case 6:
            {   /*TCP Protocol */
                ++tcp;
                break;
            }

            case 17:
            {   /*UDP Protocol */
                ++udp;

                break;
            }

            default:
            {   /*Some Other Protocol like ARP etc.*/
                ++others;
                break;
            }
        } /* switch */
        PRINTMSG_PACKET(( "\n--Packet start--\n" ));
        while (data_size--)
        {
            PRINTMSG_PACKET(( "%.2x ", *hIgmpListener->recvBuffer ));
            hIgmpListener->recvBuffer++;
        }
        PRINTMSG_PACKET(( "\n--Packet end--\n" ));
        PRINTMSG_PACKET(( "TCP : %d   UDP : %d   ICMP : %d   IGMP : %d   Others : %d   Total : %d \r", tcp, udp, icmp, igmp, others, total ));
    }
#endif /* if 0 */
    PRINTMSG_PACKET(( "IP Header: source ipaddress: "BIP_INET_ADDR_FMT", binary %x\n", BIP_INET_ADDR_ARG(iph->saddr), iph->saddr ));
    PRINTMSG_PACKET(( "IP Header: dest ipaddress: "BIP_INET_ADDR_FMT", binary %x\n", BIP_INET_ADDR_ARG(iph->daddr), iph->daddr ));

    PRINTMSG_PACKET(( "IGMPv3 Header type: 0x%x \n", igmpv3_rep_hdr->type ));

    /* deteremine whether its an igmp Query or Report */
    if (igmpv3_rep_hdr->type== 0x22)
    {   /* Report */
        int i;

        PRINTMSG_PACKET(( "   IGMPv3 Membership Report: has %d group reports\n ", ntohs( igmpv3_rep_hdr->ngrec )));
        /* Only  Look at other source  ip addresses*/
        PRINTMSG_PACKET(( "   Checking if Packet is from Myself" ));
        if (compareIpAddress( *(struct in_addr *)&iph->saddr, hIgmpListener->host_inaddr ) != 0)
        {
            /* go through list of multicast group addresses*/
            for (i = 0; i< (int)ntohs( igmpv3_rep_hdr->ngrec ); i++)  /* a client can belong to multiple multicast addresses*/
            {
                B_RTSP_MulticastSession_T *pSession = NULL; /* aget session form CheckReport Multicast Addresses*/

               #if 0
                /* Check multicast in RTSP  */
                pSession = BLST_S_FIRST( &( hIgmpListener->MulticastSession_list ));
                /*  find sessions matching mutlicast*/
                #endif
                if (findMulitcastAddressSessionList( hIgmpListener, ntohl( igmpv3_rep_hdr->grec[i].grec_mca ), &pSession ))
                {
                    PRINTMSG_PACKET(( "Group Report %d :\n ", i ));
                    /* Group Record Types
                                    1 = MODE_IS_INCLUDE = LEAVE
                                    2 = MODE_IS_EXCLUDE= JOIN
                                    3 = CHANGE_TO_INCLUDE_MODE  = LEAVE
                                    4 = CHANGE_TO_EXCLUDE_MODE = JOIN
                                    */
                    if (( igmpv3_rep_hdr->grec[i].grec_type == 4 ) || ( igmpv3_rep_hdr->grec[i].grec_type == 2 ))
                    {
                        PRINTMSG_PACKET(( "JOIN Group Report "BIP_INET_ADDR_FMT"\n", BIP_INET_ADDR_ARG(igmpv3_rep_hdr->grec[i].grec_mca)));
                        /* Start Monitoring Session when first join comes in  */
                        if (pSession->recievedJoinReport ==false)
                        {
                            B_Mutex_Lock( pSession->hMutex );
                            pSession->recievedJoinReport = true;
                            B_Mutex_Unlock( pSession->hMutex );
                        }

                        /* Mark Sessions who are waiting for Group Report */
                        if (pSession->waitingForGroupReport)
                        {
                            /* Don't clear WaitGroupReport, only allow callback from timers to do so */
                            B_Mutex_Lock( pSession->hMutex );
                            pSession->recievedGroupReport = true;
                            B_Mutex_Unlock( pSession->hMutex );
                        }

                        /* check if multicast address is already in the Join Notification list */
                        if (searchMembershipReportNotificationListByMulticast( hIgmpListener, ntohl( igmpv3_rep_hdr->grec[i].grec_mca ), BIP_IgmpListener_MembershipReportType_eJoin ))
                        {
                            BDBG_MSG(( BIP_MSG_PRE_FMT "Found this mulitcastAddress( "BIP_INET_ADDR_FMT" )from ("BIP_INET_ADDR_FMT") "
                                                       "already for a Join in MembershipReportNotificationList.Not Adding \n" BIP_MSG_PRE_ARG,
                                       BIP_INET_ADDR_ARG(igmpv3_rep_hdr->grec[i].grec_mca), BIP_INET_ADDR_ARG(iph->saddr) ));
                        }
                        else
                        {
                            BDBG_MSG(( BIP_MSG_PRE_FMT "Insert this mulitcastAddress( "BIP_INET_ADDR_FMT" ) to JoinMembershipReportNotificationList.\n"
                                       BIP_MSG_PRE_ARG, BIP_INET_ADDR_ARG(igmpv3_rep_hdr->grec[i].grec_mca)));

                            /* Add  multicast to joinmembership status  to our list */
                            B_MembershipReportCallbacks_T *memReport = NULL;
                            rc = createMembershipReportNotification( hIgmpListener, ntohl( igmpv3_rep_hdr->grec[i].grec_mca ), BIP_IgmpListener_MembershipReportType_eJoin, &memReport );
                            BIP_CHECK_GOTO(( !rc ), ( "Failed to create MembershipReportNotification" ), error, rc, rc );

                            addMembershipReportNotificationToList( hIgmpListener, memReport );

                            /*Send callback back to App */
                            if (hIgmpListener->settings.membershipReportCallback.callback)
                            {
                                ( *hIgmpListener->settings.membershipReportCallback.callback )( hIgmpListener->settings.membershipReportCallback.context, hIgmpListener->settings.membershipReportCallback.param );
                            }
                        }
                    }
                    else if (( igmpv3_rep_hdr->grec[i].grec_type == 3 ) || ( igmpv3_rep_hdr->grec[i].grec_type == 1 ))
                    {
                        PRINTMSG_PACKET(( "LEAVE Group Report on "BIP_INET_ADDR_FMT"\n",  BIP_INET_ADDR_ARG(igmpv3_rep_hdr->grec[i].grec_mca)));

                        /*
                                            1. Mark sessions waiting for a JOIN
                                            2. schedule GroupSpecificResponseTimer*/

                        /*Mark waiting for group report */
                        B_Mutex_Lock( pSession->hMutex );
                        pSession->waitingForGroupReport = true;
                        pSession->recievedGroupReport   = false;
                        B_Mutex_Unlock( pSession->hMutex );

                        setGroupSpecificResponseTimer( hIgmpListener, pSession, true );
                    }
                    else
                    {PRINTMSG_PACKET(( "Unknown group report type %d\n", igmpv3_rep_hdr->grec[i].grec_type )); }
                }
            }
        }
        else
        {PRINTMSG_PACKET(( "\n-Don't Process Packets from MySelf-\n" )); }
    }
    else if (igmpv3_rep_hdr->type== 0x11)
    {   /*  Query */
        struct in_addr t_addr;

        BKNI_Memset(&t_addr, 0, sizeof(t_addr));
        PRINTMSG_PACKET(( "   IGMPv3 Membership Query:\n group address "BIP_INET_ADDR_FMT"\n ", BIP_INET_ADDR_ARG(igmpv3_query_hdr->group)));

        /* As server only look at General Query, CAM be from Myself or from Another Server*/
        t_addr.s_addr = inet_addr( IGMP_ALL_HOST );
        if (compareIpAddress( t_addr, *(struct in_addr *)&iph->daddr ) ==0)
        {
            /*
                        if  Recieve a Query from someone and lower ip address
                        - stop query
                        - start 255 sec Query
                        else
                            - start/continue to query
                            - Reset 255 sec timer

                            start 10 second general response timer
                    */

            B_RTSP_MulticastSession_T *pSession=NULL;
            /* every session is waiting for Group Report after General query */
            pSession = BLST_S_FIRST( &( hIgmpListener->MulticastSession_list ));
            while (pSession != NULL)
            {
                B_Mutex_Lock( pSession->hMutex );
                pSession->waitingForGroupReport = true;
                B_Mutex_Unlock( pSession->hMutex );
                pSession = BLST_S_NEXT( pSession, node );
            }
            /*Debug here */
            printMulitcastSessionList( hIgmpListener );

            if (compareIpAddress( hIgmpListener->host_inaddr, *(struct in_addr *)&iph->saddr ) == 0)
            {
                PRINTMSG_PACKET(( "Recieved  this Server's own GeneralQuery message: CONTINUE/BECOME querier.  \n" ));
                if (hIgmpListener->electedQuerier)
                {
                    PRINTMSG_PACKET(( "Already querier.  \n" ));
                }

                /* After General Query start Response Timer */
                setGeneralResponseTimer( hIgmpListener, true );
            }
            else if (compareIpAddress( hIgmpListener->host_inaddr, *(struct in_addr *)&iph->saddr ) < 0)
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT " Recieved query and  this Server has lowest Ip Address: CONTINUE/BECOME querier.  \n" BIP_MSG_PRE_ARG ));
                /*start Query*/
                if (hIgmpListener->electedQuerier)
                {
                    BDBG_MSG(( BIP_MSG_PRE_FMT "Already querier.  \n" BIP_MSG_PRE_ARG ));
                }
                else
                {
                    BDBG_MSG(( BIP_MSG_PRE_FMT "not querier, but recieved higher ip,ONLY can become querier again on a 255 sec timeout  \n" BIP_MSG_PRE_ARG ));
                    /* If recieved a higher IP, don't start General Query, there coudl be other servers that hold Lowest IP*/
                }

                BDBG_ERR(( BIP_MSG_PRE_FMT "DON't Set General Response Timer, if you are lowest, and recieve general Query from someone HIGER \n" BIP_MSG_PRE_ARG ));
            }
            else
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "Recieved Another Server has lowest Ip Address: STOP  query  \n" BIP_MSG_PRE_ARG ));
                if (hIgmpListener->generalQueryTimerIsStarted)
                {
                    /*STOP Query*/
                    setAsElectedQuerier( hIgmpListener, false );
                    /* Start255 second timer to recieve  next General Query */
                    setHeardFromElectedQuerierTimer( hIgmpListener, true );
                }
                else
                {
                    /*Reset  255 second timer to recieve  next General Query */
                    setHeardFromElectedQuerierTimer( hIgmpListener, false );
                    setHeardFromElectedQuerierTimer( hIgmpListener, true );
                }

                /* After General Query start Response Timer */
                setGeneralResponseTimer( hIgmpListener, true );
            }
        }
    }
    else
    {
        PRINTMSG_PACKET(( "IGMPv3 Header: type of IGMP header: Looking for 0x22 or 0x11  \n" ));
        PRINTMSG_PACKET(( "IGMPv3 Header: These types not supported yet Membership Report (IGMPv1: 0x12, IGMPv2: 0x16), Leave Group (0x17)\n" ));
    }

    PRINTMSG_PACKET(( "--Finished Processing Packet--\n" ));
    PRINTMSG_PACKET(( " " ));
    PRINTMSG_PACKET(( " " )); /* second print */

    BDBG_LEAVE( processIGMPPacket );
    return( BIP_SUCCESS );

error:
    return( rc );
} /* processIGMPPacket */

static void listenIGMPPacket(
    void *data
    )
{
    BIP_IgmpListenerHandle hIgmpListener = (BIP_IgmpListenerHandle)data;
    struct timeval         timer;

    BDBG_ENTER( listenIGMPPacket );
    FD_ZERO( &hIgmpListener->readfds );
    FD_ZERO( &hIgmpListener->master );
    FD_SET( hIgmpListener->igmpSock, &hIgmpListener->master );

    PRINTMSG_PACKET(( BIP_MSG_PRE_FMT "Beginning Listening for Packets \n" BIP_MSG_PRE_ARG));

    BKNI_Memset(&timer, 0, sizeof(timer));
    while (hIgmpListener->stopRecvLoop == false)
    {
        /* Select command */
        timer.tv_sec           = 0;
        timer.tv_usec          = 200000;
        hIgmpListener->readfds = hIgmpListener->master;
        select( hIgmpListener->igmpSock+1, &hIgmpListener->readfds, NULL, NULL, &timer );
        if (FD_ISSET( hIgmpListener->igmpSock, &hIgmpListener->readfds ))
        {
            processIGMPPacket( hIgmpListener );
        }
        else
        {
            /* BDBG_MSG(( BIP_MSG_PRE_FMT "timeout!\n" BIP_MSG_PRE_ARG));*/
        }
    }

    BDBG_LEAVE( listenIGMPPacket );
} /* listenIGMPPacket */

/**
 * Summary:
 * API to start/stop IGMP listener
 *
 * Description:
 *
 *
 **/
BIP_Status BIP_IgmpListener_Start(
    BIP_IgmpListenerHandle hIgmpListener /* address format */
    )
{
    BIP_Status errCode = BIP_SUCCESS;

    BDBG_ENTER( BIP_IgmpListener_Start );
    /* schedule a periodic timer to send General Query Messages */
    {
        B_ThreadSettings settingsThread;
        BKNI_Memset(&settingsThread, 0, sizeof(settingsThread));
        hIgmpListener->mutexIGMPdata     = B_Mutex_Create( NULL );
        hIgmpListener->schedulerIGMPdata = B_Scheduler_Create( NULL );
        /* create thread to run scheduler */
        B_Thread_GetDefaultSettings( &settingsThread );
        hIgmpListener->schedulerThread = B_Thread_Create( "BipIgmpTmr",
                (B_ThreadFunc)B_Scheduler_Run,
                hIgmpListener->schedulerIGMPdata,
                &settingsThread );

        BIP_CHECK_GOTO(( hIgmpListener->schedulerThread ), ( "failed to create scheduler thread" ), error, BIP_ERR_OS_CHECK_ERRNO, errCode );
        hIgmpListener->generalQueryTimer          = NULL;
        hIgmpListener->querierTimer               = NULL;
        hIgmpListener->generalResponseTimer       = NULL;
    }

    /* Create another thread for main listener loop */
    if (NULL == hIgmpListener->recvThread)
    {
        /* create a thread to carryout the LiveMedia eventLoop processing */
        hIgmpListener->recvThread = B_Thread_Create( "BipIgmpPktLstnr", (B_ThreadFunc)listenIGMPPacket, (void *)hIgmpListener, NULL );
        BIP_CHECK_GOTO(( hIgmpListener->recvThread ), ( "Thread Allocation Failed" ), error, BIP_ERR_OS_CHECK_ERRNO, errCode );
    }

    /* settings this flag will cause the recv loop to start/resume */
    hIgmpListener->stopRecvLoop = false;

    /* variable to mark that  first delay between query needs to be quarter delayed */
    hIgmpListener->sentFirstGenQryQuarter = false;

    /* Start General Query Timer*/
    setAsElectedQuerier( hIgmpListener, true );

    BDBG_LEAVE( BIP_IgmpListener_Start );
    return( errCode );

error:

    BDBG_LEAVE( BIP_IgmpListener_Start );
    return( errCode );
} /* BIP_IgmpListener_Start */

BIP_Status BIP_IgmpListener_Stop(
    BIP_IgmpListenerHandle hIgmpListener
    )
{
    BDBG_ENTER( BIP_IgmpListener_Stop );

    /*1. Stop Receving packets ?? remove from select*/
    hIgmpListener->stopRecvLoop = true;
    FD_CLR( hIgmpListener->igmpSock, &hIgmpListener->master );

    /* TODO: wait on an event from the lmSchedulerThread to know that the thread is done */
    /* Then destroy the thread */


    /*2.  Stop timers */
    if (hIgmpListener->generalQueryTimer !=NULL)
    {
        B_Scheduler_CancelTimer( hIgmpListener->schedulerIGMPdata, hIgmpListener->generalQueryTimer );
        hIgmpListener->generalQueryTimer = NULL;
    }
    if (hIgmpListener->generalResponseTimer !=NULL)
    {
        B_Scheduler_CancelTimer( hIgmpListener->schedulerIGMPdata, hIgmpListener->generalResponseTimer );
        hIgmpListener->generalResponseTimer = NULL;
    }
    if (hIgmpListener->querierTimer != NULL)
    {
        B_Scheduler_CancelTimer( hIgmpListener->schedulerIGMPdata, hIgmpListener->querierTimer );
        hIgmpListener->querierTimer = NULL;
    }
    if (hIgmpListener->schedulerIGMPdata != NULL)
    {
        B_Scheduler_Stop( hIgmpListener->schedulerIGMPdata );
        B_Scheduler_Destroy( hIgmpListener->schedulerIGMPdata );
    }

    if (hIgmpListener->schedulerThread)
    {
        B_Thread_Destroy( hIgmpListener->schedulerThread );
        hIgmpListener->schedulerThread = NULL;
    }

    if (hIgmpListener->mutexIGMPdata) {B_Mutex_Destroy( hIgmpListener->mutexIGMPdata ); }

    /* Stop the listenIgmpPacketThread */
    if (hIgmpListener->recvThread != NULL)
    {
        B_Thread_Destroy( hIgmpListener->recvThread );
        hIgmpListener->recvThread = NULL;
    }

    BDBG_LEAVE( BIP_IgmpListener_Stop );
    return( BIP_SUCCESS );
} /* BIP_IgmpListener_Stop */

void BIP_IgmpListener_Destroy(
    BIP_IgmpListenerHandle hIgmpListener
    )
{
    BDBG_ENTER( BIP_IgmpListener_Destroy );
    BDBG_MSG(( BIP_MSG_PRE_FMT "Finished Destroy" BIP_MSG_PRE_ARG ));
    /* tear down the list */
    cleanMembershipReportNotificationList( hIgmpListener );
    cleanMulticastSessionList( hIgmpListener );
    BKNI_Free( hIgmpListener->recvBuffer ); /* buffer used to recieve packets */

    close( hIgmpListener->igmpSock );
    BKNI_Free( hIgmpListener );

    BDBG_LEAVE( BIP_IgmpListener_Destroy );
}

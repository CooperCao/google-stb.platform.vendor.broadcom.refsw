/***************************************************************************
 * (c) 2007-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#ifndef BIP_IGMP_LISTENER_H
#define BIP_IGMP_LISTENER_H

#ifdef __cplusplus
extern "C" {
#endif

/* TODO: BIP_Init() should allow to pass thru B_Os thread settings to allow apps to change the thread name & priority */
typedef enum BIP_IgmpListener_MembershipReportType
{
    BIP_IgmpListener_MembershipReportType_eUnknown = 0,    /* unknown/not supported IGMP Membership Type format */
    BIP_IgmpListener_MembershipReportType_eJoin,
    BIP_IgmpListener_MembershipReportType_eLeave,
    BIP_IgmpListener_MembershipReportType_eMax
} BIP_IgmpListener_MembershipReportType;

typedef struct BIP_IgmpListener *BIP_IgmpListenerHandle;
/*typedef struct BIP_IgmpListener_JoinStatus *BIP_IgmpListener_JoinStatusHandle;
typedef struct BIP_IgmpListener_LeaveStatus *BIP_IgmpListener_LeaveStatusHandle;*/
/**
 * Summary:
 * Status information passed back when a  Membership  message is recieved
 *
 * Description:
 * After a Memberhsip Notification Callback  is generated to the Upper Layer. Upper layer will call BIP_IgmpListener_GetMembershipStatus
 * to recieve information on the Join memembership report.
 *
 * See Also:
 * BIP_IgmpListener_GetMembershipStatus
 **/
typedef struct BIP_IgmpListener_MembershipReportStatus
{
    unsigned int multicast_address; /*In host byte order. Multicast address of the client is joined on.   */
    BIP_IgmpListener_MembershipReportType           memRepType;
} BIP_IgmpListener_MembershipReportStatus_T;

/**
 * Summary:
 * Status information passed back when a Join message is recieved
 *
 * Description:
 * After a Join Notification Callback  is generated to the Upper Layer. Upper layer will call BIP_IgmpListener_GetJoinStatus
 * to recieve information on the Join memembership report.
 *
 * See Also:
 * BIP_IgmpListener_GetJoinStatus
 **/
typedef struct BIP_IgmpListener_JoinStatus
{
    unsigned int multicast_address; /*In host byte order. Multicast address of the client is joined on.   */
} BIP_IgmpListener_JoinStatus_T;

/**
 * Summary:
 * Status information passed back when a Leave membership report is recieved
 *
 * Description:
 * After a Leave Notification Callback  is generated to the Upper Layer. Upper layer will call BIP_IgmpListener_GetJoinStatus
 * to recieve information on the Leave membership report.
 *
 * See Also:
 * BIP_IgmpListener_GetLeaveStatus
 **/
typedef struct BIP_IgmpListener_LeaveStatus
{
    unsigned int multicast_address; /*In host byte order. Multicast address of the client is joined on.   */
} BIP_IgmpListener_LeaveStatus_T;

/**
 * Summary:
 * Create settings for the Igmp Listener
 *
 * Description:
 * Note: if you are going to follow SES SatIp Spec v1.2 call BIP_IgmpSesSatIpListener_GetDefaultCreateSettings to generate
 * specs settings.
 *
 * See Also:
 * BIP_IgmpListener_GetDefaultCreateSettings
 * BIP_IgmpSesSatIpListener_GetDefaultCreateSettings
 **/
typedef struct BIP_IgmpListenerCreateSettings
{
    bool igmp_v2_support;   /* Support igmp v2 packets */
    bool igmp_v3_support;   /* Support igmp v3 packets */

    /*TODO: Add variable to set what about  ipv4 vs ipv6 ??*/
    char *lan_iface; /*  lan interface: ie. eth0 or eth2*/

    bool enableSesSatIpFeatures; /* set if we are to follow the SesSatIp Spec */

    unsigned int device_id;               /* used in second octet of  multicast address*/
    unsigned int gen_query_interval;      /*time between sending general queries when querier*/
    unsigned     querier_interval;        /* when this timer expires, means have not heard from another Elected Querier (w/ lower ip addres) and should become querier*/
    unsigned     gen_response_interval;   /*time given between a general query and when it should recieve a response for multicast sessions */
    unsigned     group_specific_response; /* time given to recieve repsonse from group specific query*/
} BIP_IgmpListenerCreateSettings;

/**
 * Summary:
 * Get Default IgmpListener settings
 * See Also:
 * BIP_IgmpListener_Create
 **/
void
BIP_IgmpListener_GetDefaultCreateSettings(
    BIP_IgmpListenerCreateSettings *pSettings
    );
/**
 * Summary:
 * Get Default IgmpSesSatIpListener settings
 **/
void
BIP_IgmpSesSatIpListener_GetDefaultCreateSettings(
    BIP_IgmpListenerCreateSettings *pSettings
    );

/**
 * Summary:
 * API to setup a igmp listener for a particular protocol.
 *
 * Description:
 * See Also:
 * BIP_IgmpListener_GetDefaultCreateSettings
 **/
BIP_IgmpListenerHandle BIP_IgmpListener_Create(
    BIP_IgmpListenerCreateSettings *pSettings
    );

/**
 * Summary:
 * Run time settings for the IGMP Listener
 *
 * Description:
 *
 **/
typedef struct BIP_IgmpListenerSettings
{
    BIP_CallbackDesc membershipReportCallback;
} BIP_IgmpListenerSettings;

/**
 * Summary:
 * API to Get Listener Settings
 *
 * Description:
 * See Also:
 * BIP_IgmpListener_SetSettings
 **/
BIP_Status
BIP_IgmpListener_GetSettings(
    BIP_IgmpListenerHandle    hIgmpListener,
    BIP_IgmpListenerSettings *pSettings
    );

/**
 * Summary:
 * API to Set Listener Settings
 *
 * Description:
 * See Also:
 * BIP_IgmpListener_GetSettings
 **/
BIP_Status
BIP_IgmpListener_SetSettings(
    BIP_IgmpListenerHandle    hIgmpListener,
    BIP_IgmpListenerSettings *pSettings
    );

/**
 * Summary:
 * API to start IGMP listener
 *
 * Description:
 * Start threads for listener, and  adds timer to scheduler
 *
 **/
BIP_Status
BIP_IgmpListener_Start(
    BIP_IgmpListenerHandle hIgmpListener
    );

/**
 * Summary:
 * API to stop IGMP listener
 *
 * Description:
 * Destroy thread for listenr and removes timers from scheduler
 *
 **/
BIP_Status
BIP_IgmpListener_Stop(
    BIP_IgmpListenerHandle hIgmpListener
    );

/**
 * Summary:
 * API to add IGMP Group addresses that needs to be monitored for IGMP JOIN events
 *
 * Description:
 * The multicast address need to be in HOST order  formati.e. ntohl(inet_addr("239.1.x.x"))  will convert x.x.x.x string to
 * Host byte order
 **/
BIP_Status
BIP_IgmpListener_AddGroupAddress(
    BIP_IgmpListenerHandle hIgmpListener,
    unsigned int           multicast_address, /*  the multicast address need to be in HOST  byte order  format*/
    unsigned int           sessionID          /* give RTSP session ID associated with multicast. TODO pass in 0 no SessionID*/
    );

/**
 * Summary:
 * API to delete IGMP Group addresses that needs to be monitored for IGMP  LEAVE events
 *
 * Description:
 * The multicast address need to be in HOST order  formati.e. ntohl(inet_addr("239.1.x.x"))  will convert x.x.x.x string to
 * Host byte order
 **/
BIP_Status
BIP_IgmpListener_DelGroupAddress(
    BIP_IgmpListenerHandle hIgmpListener,
    unsigned int           multicast_address /*  the multicast address need to be in HOST  byte order  format*/
    );

/**
 * Summary:
 * API to Get the Membership Status  after recieving a Membership callback
 *
 * Description:
 * Can be a leave or join
 *
 **/
BIP_Status
BIP_IgmpListener_GetMembershipReportStatus(
    BIP_IgmpListenerHandle         hIgmpListener,
    BIP_IgmpListener_MembershipReportStatus_T *pStatus
    );

/**
 * Summary:
 * Destroy IgmpListener
 *
 * Description:
 **/
void
BIP_IgmpListener_Destroy(
    BIP_IgmpListenerHandle hIgmpListener
    );

#ifdef __cplusplus
}
#endif

#endif /* BIP_IGMP_LISTENER_H */

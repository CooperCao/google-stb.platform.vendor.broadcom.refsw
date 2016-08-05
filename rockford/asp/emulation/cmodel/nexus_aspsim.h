/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef __NEXUS_ASPSIM_H__
#define __NEXUS_ASPSIM_H__

#include "asp_utils.h"

typedef enum NEXUS_AspSim_IpVersion
{
    NEXUS_AspSim_IpVersion_eIpV4,
    NEXUS_AspSim_IpVersion_eIpV6,
    NEXUS_AspSim_IpVersion_eOther,
    NEXUS_AspSim_IpVersion_eMax
} NEXUS_AspSim_IpVersion;

typedef enum NEXUS_AspSim_ProtocolType
{
    NEXUS_AspSim_ProtocolType_eTcp,
    NEXUS_AspSim_ProtocolType_eUdp,
    NEXUS_AspSim_ProtocolType_eRtp,
    NEXUS_AspSim_ProtocolType_eOther,
    NEXUS_AspSim_ProtocolType_eMax
} NEXUS_AspSim_ProtocolType;

#define NEXUS_ASPSIM_MAC_ADDRESS_SIZE   2
#define NEXUS_ASPSIM_IP_ADDRESS_SIZE    4

typedef struct NEXUS_AspSim_ConnectionControlInfo
{
    unsigned    destMacAddress[NEXUS_ASPSIM_MAC_ADDRESS_SIZE];
    unsigned    srcMacAddress[NEXUS_ASPSIM_MAC_ADDRESS_SIZE];
    unsigned    etherType;      /*!< This field defines the protocol at layer 3 level. For av streaming this is ip 0x800.*/
                                /*!< we can define an enum for this and then pi will internally map this to the value. */
    unsigned    brcmTag;
    NEXUS_AspSim_IpVersion  ipVersion;
    unsigned    dscp;           /*!< Differentiated service code point. This will be used to provide quality of service.*/
    unsigned    ecn;            /*!< Explicit congestion notification.*/

    unsigned ipIdentification;  /*!< Need to check whether this will come from linux, if not then this can be removed and Pi can generate an unique number.*/
    unsigned timeToLive;

    NEXUS_AspSim_ProtocolType   protocolType;

    /****** Following fields are specific to the ipv6 ******/
    unsigned trafficClass;
    unsigned flowLabel;
    unsigned nextsHeader;
    unsigned hopLimit;
    /*******************************************************/

    unsigned srcIpAddress[ NEXUS_ASPSIM_IP_ADDRESS_SIZE ];
    unsigned destIpAddress[ NEXUS_ASPSIM_IP_ADDRESS_SIZE ];

    unsigned srcPort;
    unsigned destPort;

    unsigned sequenceNumber;
    unsigned currentAckNumber;

    unsigned windowSize;
    bool     windowScaleEnable;
    unsigned windowScaleValue; /*!< window scale value should be set to 1 when window scale is disabled.*/
    bool     sackEnable;

    bool     timeStampEnable;
    unsigned timeStampEchoValue; /*TODO: Need to check how to get this(stc or ).*/

    unsigned maxSegmentSize;

    /* Bryan has commented the following parameter , will check with him.*/
    /* vlanTag;
       mtuSize;
       L2 headerInfo
       */
} NEXUS_AspSim_ConnectionControlInfo;

typedef struct NEXUS_AspSim_SwitchConfig
{
    unsigned switchSlotsPerEthernetPacker;
    unsigned switchQueueNumber;
} NEXUS_AspSim_SwitchConfig;

#endif /*   __NEXUS_ASPSIM_H__  */

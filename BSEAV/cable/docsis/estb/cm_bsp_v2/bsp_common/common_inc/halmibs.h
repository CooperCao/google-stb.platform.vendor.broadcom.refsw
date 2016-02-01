//****************************************************************************
//
// Copyright (c) 2008 Broadcom Corporation
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
//    Filename: HalMibs.h
//    Author:   Kevin O'Neal
//    Creation Date: 14-Feb-2000
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    Set/Query info OID's for various DOCSIS MIB's
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef HALMIBS_H
#define HALMIBS_H

/* This is the interface number assigned to the CM HAL. */
#define IFTABLE_BASE          10

/* These are the sub-interfaces assigned to the MAC, U/S, and D/S.  They are
   offsets from IFTABLE_BASE.  These are used only for the MIB_OID queries for
   the CM HAL. */
#define IFTABLE_MAC           0
#define IFTABLE_UPSTREAM      1
#define IFTABLE_DOWNSTREAM    2
#define IFTABLE_MAC_DATAONLY  3

/*----------------------------------------------------------------------*/
/* Definitions for Interface Group MIB variables                        */
/* GET command definitions                                              */
/*----------------------------------------------------------------------*/
#define MIB_OID_GIFDESCR                  0x40016D04 /* Get NI description */
#define MIB_OID_GIFMTUNIT                 0x40016D06 /* Get NI mtu */
#define MIB_OID_GIFSPEED                  0x40016D07 /* Get NI speed */
#define MIB_OID_GIFOPERSTATUS             0x40016D08 /* Per RFC-2233 (up/down/etc) */
#define MIB_OID_GIFINOCTETS               0x40016D0C /* Get no of octets rcvd. */
#define MIB_OID_GIFINUCASTPKTS            0x40016D0D /* Get no. of unicast pkts rcvd  */
#define MIB_OID_GIFINNUCASTPKTS           0x40016D0E /* Get no. of multi. pkts rcvd */
#define MIB_OID_GIFINDISCARDS             0x40016D0F /* Get no of pckts discarded */
#define MIB_OID_GIFINERRORS               0x40016D10 /* Get no of error pckts */
#define MIB_OID_GIFINUNKNOWNPROTOS        0x40016D11 /* No of pckts for unk. protocol */
#define MIB_OID_GIFOUTOCTETS              0x40016D12 /* No of octets rcvd */
#define MIB_OID_GIFOUTUCASTPKTS           0x40016D13 /* No of unicast pkts sent */
#define MIB_OID_GIFOUTNUCASTPKTS          0x40016D14 /* No of multi. pkts sent */
#define MIB_OID_GIFOUTDISCARDS            0x40016D15 /* No of out pkts discarded */
#define MIB_OID_GIFOUTERRORS              0x40016D16 /* No of outbounds pkts errors */
#define MIB_OID_GIFOUTQLEN                0x40016D17 /* Length of output Q length */

// MIB_OID_GIFOPERSTATUS definitions:
#define GIFOPERSTATUS_UP				  	1	 	// ready to pass packets
#define GIFOPERSTATUS_DOWN				  	2  		// Not ready to pass packets
#define GIFOPERSTATUS_TESTING				3	  	// in some test mode
#define GIFOPERSTATUS_UNKNOWN				4		// status can not be determined for some reason
#define GIFOPERSTATUS_DORMANT				5  		// not currently active
#define GIFOPERSTATUS_NOTPRESENT			6		// some component is missing
#define GIFOPERSTATUS_LOWERLAYERDOWN		7 		// down due to state of lower-layer interface(s)

/*----------------------------------------------------------------------*/
/* set command definitions                                              */
/*----------------------------------------------------------------------*/
#define MIB_OID_SIFADMINSTATUS            0x80016D09 /* set Interface admin. status */

/*----------------------------------------------------------------------*/
/* Definitions for IP Group MIB variables                               */
/* GET command definitions                                              */
/*----------------------------------------------------------------------*/
#define MIB_OID_GIPFORWARDING             0x40026D01 /* IP forwarding var.   */
#define MIB_OID_GIPDEFAULTTTL             0x40026D02 /* IP def. Time-to-Live var */
#define MIB_OID_GIPINRECEIVES             0x40026D03 /* IP dgrams rec. from below */
#define MIB_OID_GIPINHDRERRORS            0x40026D04 /* IP dgrams format errs drops */
#define MIB_OID_GIPINADDRERRORS           0x40026D05 /* IP dgrams misdelivery drops */
#define MIB_OID_GIPFORWDATAGRAMS          0x40026D06 /* IP dgrams forwarded */
#define MIB_OID_GIPINUNKNOWNPROTOS        0x40026D07 /* IP unknown proto. dgrams */
#define MIB_OID_GIPINDISCARDS             0x40026D08 /* IP disc. dgrms for resources */
#define MIB_OID_GIPINDELIVERS             0x40026D09 /* IP dgrams delivered above */
#define MIB_OID_GIPOUTREQUESTS            0x40026D0A /* IP dgrams sent out */
#define MIB_OID_GIPOUTDISCARDS            0x40026D0B /* IP out dgrams discarded */
#define MIB_OID_GIPOUTNOROUTES            0x40026D0C /* IP dgrams drops for no routes */
#define MIB_OID_GIPREASMTIMEOUT           0x40026D0D /* IP retrans. queue timeout */
#define MIB_OID_GIPREASMREQDS             0x40026D0E /* IP frags. needing reass. */
#define MIB_OID_GIPREASMOKS               0x40026D0F /* IP frags. reassembled */
#define MIB_OID_GIPREASMFAILS             0x40026D10 /* IP frag reassembly fails */
#define MIB_OID_GIPFRAGOKS                0x40026D11 /* IP dgrams fragmented */
#define MIB_OID_GIPFRAGFAILS              0x40026D12 /* IP dgrams fragmentation fails */
#define MIB_OID_GIPFRAGCREATES            0x40026D13 /* IP frags. created */
#define MIB_OID_GIPROUTINGDISCARDS        0x40026D14 /* IP Routing ent. discarded */

/*----------------------------------------------------------------------*/
/* SET command definitions                                              */
/*----------------------------------------------------------------------*/
#define MIB_OID_SIPFORWARDING             0x80026D01 /* IP forwarding variable */
#define MIB_OID_SIPDEFAULTTTL             0x80026D02 /* IP Defautl time-to-live */
#define MIB_OID_SIPREASMTIMEOUT           0x80026D0D /*  IP retrans. queue timeout */

/*----------------------------------------------------------------------*/
/* Definitions for IP NI address table                                  */
/* GET commands definitions                                             */
/*----------------------------------------------------------------------*/
#define MIB_OID_GIPADDRTABLE              0x40036D01 /* IP Address table */
#define MIB_OID_GIPADDRENTADDR            0x40036D03 /* IP Address of the addr entry */
#define MIB_OID_GIPADDRENTIFINDEX         0x40036D04 /* NI number of the interface */
#define MIB_OID_GIPADDRENTNETMASK         0x40036D05 /* Netmask of the NI */
#define MIB_OID_GIPADDRENTBCASTADDR       0x40036D06 /* IP Broadcast Address of the NI*/
#define MIB_OID_GIPADDRENTREASMMAXSIZE    0x40036D07 /* IP max. reassm. size */

/*----------------------------------------------------------------------*/
/* Definitions for IP route table                                       */
/* GET command definitions                                              */
/*----------------------------------------------------------------------*/
#define MIB_OID_GIPROUTETABLE             0x40046D01 /* IP routing table */
#define MIB_OID_GIPROUTEDEST              0x40046D03 /* Route destination */
#define MIB_OID_GIPROUTEIFINDEX           0x40046D04 /* Interface Index of the route */
#define MIB_OID_GIPROUTENEXTHOP           0x40046D05 /* Next hop of the route */
#define MIB_OID_GIPROUTETYPE              0x40046D06 /* Type of the route */
#define MIB_OID_GIPROUTEPROTO             0x40046D07 /* Protocol used by the route */
#define MIB_OID_GIPROUTEAGE               0x40046D08 /* Age of the Route  */
#define MIB_OID_GIPROUTEMASK              0x40046D09 /* Mask used by the route */

/*----------------------------------------------------------------------*/
/* SET command definitions                                              */
/*----------------------------------------------------------------------*/
#define MIB_OID_SIPROUTEDEST              0x80046D03 /* Route destination */
#define MIB_OID_SIPROUTENEXTHOP           0x80046D05 /* Next hop of the route */
#define MIB_OID_SIPROUTETYPE              0x80046D06 /* Type of the route */

/*----------------------------------------------------------------------*/
/* Definitions for IP NET-TO-MEDIA table                                */
/* GET command definitions                                              */
/*----------------------------------------------------------------------*/
#define MIB_OID_GIPNETTOMEDIATABLE        0x40056D01 /* IP Net-to-Media table */
#define MIB_OID_GIPNETTOMEDIAIFINDEX      0x40056D03 /* Index of the Entry */
#define MIB_OID_GIPNETTOMEDIAPHYSADDRESS  0x40056D04 /* Physical add. of entry */
#define MIB_OID_GIPNETTOMEDIANETADDRESS   0x40056D05 /* IP address of entry   */
#define MIB_OID_GIPNETTOMEDIATYPE         0x40056D06 /* Type of the entry     */

/*----------------------------------------------------------------------*/
/* SET command definitions                                              */
/*----------------------------------------------------------------------*/
#define MIB_OID_SIPNETTOMEDIAPHYSADDRESS  0x80056D04 /* Physical add. of entry */
#define MIB_OID_SIPNETTOMEDIANETADDRESS   0x80056D05 /* IP address of entry   */
#define MIB_OID_SIPNETTOMEDIATYPE         0x80056D06

/*----------------------------------------------------------------------*/
/* Definitions for ICMP Group MIB variables                             */
/* GET command definitions                                              */
/*----------------------------------------------------------------------*/
#define MIB_OID_GICMPINMSGS               0x40066D01 /* ICMP msgs. received */
#define MIB_OID_GICMPINERRORS             0x40066D02 /* ICMP msgs. with format errs */
#define MIB_OID_GICMPINDESTUNREACHS       0x40066D03 /* ICMP dest. Unreach. msgs rcvd*/
#define MIB_OID_GICMPINTIMEEXCDS          0x40066D04 /* ICMP time excds. msgs. rcvd. */
#define MIB_OID_GICMPINPARAMPROBS         0x40066D05 /* ICMP param. prob. msgs rcvd */
#define MIB_OID_GICMPINSRCQUENCHS         0x40066D06 /*ICMP src. quench msgs rcvd */
#define MIB_OID_GICMPINREDIRECTS          0x40066D07 /* ICMP src quenchs rcvd */
#define MIB_OID_GICMPINECHOS              0x40066D08 /* ICMP echo msgs rcvd */
#define MIB_OID_GICMPINECHOREPS           0x40066D09 /* ICMP echo reply msgs rcvd */
#define MIB_OID_GICMPINTIMESTAMPS         0x40066D0A /* ICMP time stamp msgs rcvd */
#define MIB_OID_GICMPINTIMESTAMPREPS      0x40066D0B /* ICMP time stp rep msgs rcvd */
#define MIB_OID_GICMPINADDRMASKS          0x40066D0C /* ICMP address masks msgs rcvd */
#define MIB_OID_GICMPINADDRMASKREPS       0x40066D0D /*ICMP addr masks reps msgs rcvd */
#define MIB_OID_GICMPOUTMSGS              0x40066D0E /* ICMP messages sent */
#define MIB_OID_GICMPOUTERRORS            0x40066D0F /* ICMP out error msgs. */
#define MIB_OID_GICMPOUTDESTUNREACHS      0x40066D10 /* ICMP dest unreach msgs sent */
#define MIB_OID_GICMPOUTTIMEEXCDS         0x40066D11 /* ICMP time exceed msgs sent */
#define MIB_OID_GICMPOUTPARAMPROBS        0x40066D12 /* ICMP param prob. msgs sent */
#define MIB_OID_GICMPOUTSRCQUENCHS        0x40066D13 /* ICMP src quench msgs sent */
#define MIB_OID_GICMPOUTREDIRECTS         0x40066D14 /* ICMP redirect msgs sent */
#define MIB_OID_GICMPOUTECHOS             0x40066D15 /* ICMP echo msgs sent */
#define MIB_OID_GICMPOUTECHOREPS          0x40066D16 /* ICMP echo reply msgs sent */
#define MIB_OID_GICMPOUTTIMESTAMPS        0x40066D17 /* ICMP time stamps msgs sent */
#define MIB_OID_GICMPOUTTIMESTAMPREPS     0x40066D18 /* ICMP time stps reps sent */
#define MIB_OID_GICMPOUTADDRMASKS         0x40066D19 /* ICMP addr mask msgs sent */
#define MIB_OID_GICMPOUTADDRMASKREPS      0x40066D1A /* ICMP addr mask rep msgs sent */

/*----------------------------------------------------------------------*/
/* Definitions for TCP Group MIB variables                              */
/* GET command definitions                                              */
/*----------------------------------------------------------------------*/
#define MIB_OID_GTCPRTOALGORITHM          0x40076D01 /* TCP retransmission algorithm */
#define MIB_OID_GTCPRTOMIN                0x40076D02 /* TCP min. retrans. timeout */
#define MIB_OID_GTCPRTOMAX                0x40076D03 /* TCP max. retrans. timeout */
#define MIB_OID_GTCPMAXCONN               0x40076D04 /* TCP max. simul. connections */
#define MIB_OID_GTCPACTIVEOPENS           0x40076D05 /* TCP active opens */
#define MIB_OID_GTCPPASSIVEOPENS          0x40076D06 /* TCP number of passive opens */
#define MIB_OID_GTCPATTEMPTFAILS          0x40076D07 /* TCP no. of failed conn. attempts*/
#define MIB_OID_GTCPESTABRESETS           0x40076D08 /* TCP no. connections reset */
#define MIB_OID_GTCPCURRESTAB             0x40076D09 /* TCP no. of curr. connections */
#define MIB_OID_GTCPINSEGS                0x40076D0A /* TCP no. of segs. received */
#define MIB_OID_GTCPOUTSEGS               0x40076D0B /* TCP no. of segs. sent */
#define MIB_OID_GTCPRETRANSSEGS           0x40076D0C /* TCP no. of segs retransmitted */
#define MIB_OID_GTCPCONNTABLE             0x40076D0D /* TCP connection table */
#define MIB_OID_GTCPCONNSTATE             0x40076D0E /* TCP connection entry */
#define MIB_OID_GTCPINERRS                0x40076D0F /* TCP no. of segs. disc. due to errs*/
#define MIB_OID_GTCPOUTRSTS               0x40076D10 /* TCP no. of reset segs. sent */

/*----------------------------------------------------------------------*/
/* SET command definitions                                              */
/*----------------------------------------------------------------------*/
#define MIB_OID_STCPCONNSTATE             0x80076D0E  /* TCP connection entry */

/*----------------------------------------------------------------------*/
/* Definitions for UDP MIB variables                                    */
/* GET command definitions                                              */
/*----------------------------------------------------------------------*/
#define MIB_OID_GUDPINDATAGRAMS           0x40086D01 /* UDP dgrams delivered */
#define MIB_OID_GUDPNOPORTS               0x40086D02 /* UDP dgrams rcvd for unkown ports*/
#define MIB_OID_GUDPINERRORS              0x40086D03 /* UDP dgrams dropped to format errs*/
#define MIB_OID_GUDPOUTDATAGRAMS          0x40086D04 /* UDP dgrms sent from appls. */
#define MIB_OID_GUDPTABLE                 0x40086D05 /* UDP table */


/*----------------------------------------------------------------------*/
/* Definitions for ifXTable MIB variables                               */
/* RFC-2233, GET's and SET's use the same OID                           */
/*----------------------------------------------------------------------*/
#define MIB_OID_IFNAME                    0x400A0001 /* ifName - RO string */
#define MIB_OID_IFINMCASTPKTS             0x400A0002 /* ifInMulticastPkts - 32 bit counter */
#define MIB_OID_IFINBCASTPKTS             0x400A0003 /* ifInBroadcastPkts - 32 bit counter  */
#define MIB_OID_IFOUTMCASTPKTS            0x400A0004 /* ifOutMulticastPkts - 32 bit counter  */
#define MIB_OID_IFOUTBCASTPKTS            0x400A0005 /* ifOutBroadcastPkts - 32 bit counter  */
#define MIB_OID_IFHCINOCTETS              0x400A0006 /* ifHCInOctets - 64 bit counter */
#define MIB_OID_IFHCINUCASTPKTS           0x400A0007 /* ifHCInUcastPkts - RO 64 bit counter */
#define MIB_OID_IFCHINMCASTPKTS           0x400A0008 /* ifHCInMulticastPkts - RO 64 bit counter */
#define MIB_OID_IFHCINCBASTPKTS           0x400A0009 /* ifHCInBroadcastPkts - RO 64 bit counter */
#define MIB_OID_IFHCOUTOCTETS             0x400A000A /* ifHCOutOctets - RO 64 bit counter */
#define MIB_OID_IFHCOUTUCASTPKTS          0x400A000B /* ifHCOutUcastPkts - RO 64 bit counter */
#define MIB_OID_IFHCOUTMCASTPKTS          0x400A000C /* ifHCOutMulticastPkts - RO 64 bit counter */
#define MIB_OID_IFHCOUTBCASTPKTS          0x400A000D /* ifHCOutBroadcastPkts - RO 64 bit counter */
#define MIB_OID_IFLINKUPDOWNTRAP          0x400A000E /* ifLinkUpDownTrapEnable - RO truth value */
#define MIB_OID_IFHIGHSPEED               0x400A000F /* ifHighSpeed - RO ifSpeed / 1Mhz if ifSpeed > 32 bits */
#define MIB_OID_IFPROMISCUOUS             0x400A0010 /* ifPromiscuousMode - RW truth value (0/1) */
#define MIB_OID_IFCONNECTOR               0x400A0011 /* ifConnectorPresent - RO truth value */
#define MIB_OID_IFALIAS                   0x400A0012 /* ifAlias - RW string */
#define MIB_OID_IFCOUNTERDISCONT          0x400A0013 /* ifCounterDiscontinuityTime - RO time_t */

/*----------------------------------------------------------------------*/
/* Definitions for etherLikeMib MIB variables dot3StatsTable            */
/* RFC-1643, GET's and SET's use the same OID                           */
/*----------------------------------------------------------------------*/
//#define MIB_OID_DOT3STATSINDEX          0x400A0101 /* dot3StatsIndex */
#define MIB_OID_DOT3ALIGNMENTERRORS       0x400A0102 /* dot3StatsAlignmentErrors - RO 32 bit counter*/
#define MIB_OID_DOT3FCSERRORS             0x400A0103 /* dot3StatsFCSErrors - RO 32 bit counter*/
#define MIB_OID_DOT3SINGLECOLLFRAMES      0x400A0104 /* dot3StatsSingleCollisionFrames - RO 32 bit counter */
#define MIB_OID_DOT3MULTCOLLFRAMES        0x400A0105 /* dot3StatsMultipleCollisionFrames - RO 32 bit counter */
#define MIB_OID_DOT3SQETESTERRORS         0x400A0106 /* dot3StatsSQETestErrors - RO 32 bit counter */
#define MIB_OID_DOT3DEFERREDTXS           0x400A0107 /* dot3StatsDeferredTransmissions - RO 32 bit counter */
#define MIB_OID_DOT3LATECOLLS             0x400A0108 /* dot3StatsLateCollisions  - RO 32 bit counter*/
#define MIB_OID_DOT3EXCESSIVECOLLS        0x400A0109 /* dot3StatsExcessiveCollisions - RO 32 bit counter*/
#define MIB_OID_DOT3INTERNALMACTXERRS     0x400A010A /* dot3StatsInternalMacTransmitErrors - RO 32 bit counter */
#define MIB_OID_DOT3CARRIERSENSEERRS      0x400A010B /* dot3StatsCarrierSenseErrors - RO 32 bit counter */
#define MIB_OID_DOT3FRAMETOOLONGS         0x400A010D /* dot3StatsFrameTooLongs - RO 32 bit counter */
#define MIB_OID_DOT3INTERNALMACRXERRS     0x400A0110 /* dot3StatsInternalMacReceiveErrors - RO 32 bit counter */
//#define MIB_OID_DOT3ETHERCHIPSET        0x400A0111 /* dot3StatsEtherChipSet - RO OID */
#define MIB_OID_DOT3SYMBOLERRS            0x400A0112 /* dot3StatsSymbolErrors - RO 32 bit counter */
#define MIB_OID_DOT3DUPLEXSTATUS          0x400A0113 /* dot3StatsDuplexStatus - integer:1=unknown,2=half,3=full */

#define MIB_OID_CHIPTYPE				  0x400A0114 /* chiptype 0=5325M  1=5325E*/

/*----------------------------------------------------------------------*/
/* Definitions for etherLikeMib MIB variables dot3CollTable             */
/* RFC-1643, GET's and SET's use the same OID                           */
/*----------------------------------------------------------------------*/
#define MIB_OID_DOT3COLLCOUNT             0x400A0201 /* dot3CollCount - RO integer*/
#define MIB_OID_DOT3COLLFREQUENCIES       0x400A0202 /* dot3CollFrequencies - RO 32 bit counter*/

/*----------------------------------------------------------------------*/
/* Definitions for docsIfMib MIB variables docsIfSignalQualityTable     */
/* RFC-2670, GET's and SET's use the same OID                           */
/*----------------------------------------------------------------------*/
#define MIB_OID_SIGQINCLUDESCONTENTION    0x400A0301 /* docsIfSigQIncludesContention  - RO truth value (0/1) */
#define MIB_OID_SIGQUNERROREDS            0x400A0302 /* docsIfSigQUnerroreds - RO 32 bit counter */
#define MIB_OID_SIGQCORRECTEDS            0x400A0303 /* docsIfSigQCorrecteds - RO 32 bit counter */
#define MIB_OID_SIGQUNCORRECTABLES        0x400A0304 /* docsIfSigQUncorrectables - RO 32 bit counter */
#define MIB_OID_SIGQSIGNALNOISE           0x400A0305 /* docsIfSigQSignalNoise - RO integer, units 10th dBmV's */
#define MIB_OID_SIGQMICROREFLECTIONS      0x400A0306 /* docsIfSigQMicroreflections - RO integer, units dBc */
#define MIB_OID_SIGQEQUALIZATIONDATA      0x400A0307 /* docsIfSigQEqualizationData - RO octet string */
// NEW 3/25/03 from DOCS-IF-MIB version 5 (draft-ietf-ipcdn-docs-rfmibv2-05)
#define MIB_OID_CMSTATUSEQUALIZATIONDATA  0x400A0308 /* docsIfCmStatusEqualizationData - RO octet string */

/*----------------------------------------------------------------------*/
/* Definitions for docsIfMib MIB variables docsIfUpstreamChannelTable   */
/* RFC-2670, GET's and SET's use the same OID                           */
/*----------------------------------------------------------------------*/
#define MIB_OID_UPCHANRNGBACKOFFSTART     0x400A0407 /* docsIfUpChannelRangingBackoffStart */
#define MIB_OID_UPCHANRNGBACKOFFEND       0x400A0408 /* docsIfUpChannelRangingBackoffEnd */
#define MIB_OID_UPCHANTXBACKOFFSTART      0x400A0409 /* docsIfUpChannelTxBackoffStart */ 
#define MIB_OID_UPCHANTXBACKOFFEND        0x400A040A /* docsIfUpChannelTxBackoffEnd */ 

/*----------------------------------------------------------------------*/
/* Definitions for docsIfMib MIB variables docsIfDownstreamChannelTable */
/* RFC-2670, GET's and SET's use the same OID                           */
/* Note that some of these MIB's may be maintained by the app and not   */
/* retrieved vial HAL calls, but the OIDs are defined for completeness. */
/*----------------------------------------------------------------------*/
#define MIB_OID_DOWNCHANNELFREQUENCY      0x400A0502
#define MIB_OID_DOWNCHANNELWIDTH          0x400A0503
#define MIB_OID_DOWNCHANNELMODULATION     0x400A0504
#define MIB_OID_DOWNCHANNELINTERLEAVE     0x400A0505
#define MIB_OID_DOWNCHANNELPOWER          0x400A0506
#define MIB_OID_DOWNCHANNELPOWER_DIRECT   0x400A0507
// DS frequency error
#define MIB_OID_DOWNCHANNEL_FREQ_ERR      0x400A0508

typedef struct _DsPowerDirect
{
  int FrequencyHz;
  int PowerTenthdBmV;
  int AgcSum;
} DsPowerDirect;

typedef struct _DsPowerCharacterization
{
  int PowerTenthdBmV;// Optionally passed to HAL from app (used for printing only)
  unsigned long NormalLDAIT; // Returned from HAL to app
  unsigned long NormalLDAIF; // Returned from HAL to app
  unsigned long NormalLDAII; // Returned from HAL to app
  unsigned long FrozenLDAIT; // Returned from HAL to app
  unsigned long FrozenLDAIF; // Returned from HAL to app
  unsigned long FrozenLDAII; // Returned from HAL to app
  unsigned long FGainTap; // Returned from HAL to app
  unsigned long TGainTap; // Returned from HAL to app
  unsigned long IGainTap; // Returned from HAL to app
} DsPowerCharacterization;

// Some constants for DS power level have special meanings when setting
// used for calibration of DS channel power lookup matrix (if required)
#define DS_PWR_CAL_FLUSH_CURRENT_TABLE    8600
#define DS_PWR_CAL_FLUSH_MATRIX           8686
#define DS_PWR_CAL_NV_READ_MATRIX         9900
#define DS_PWR_CAL_NV_WRITE_MATRIX        9999
#define DS_PWR_CAL_NV_DEFAULT_MATRIX      7700
#define DS_PWR_CAL_NV_PRINT_MATRIX        7777
#define DS_PWR_CAL_NV_LOCK_MATRIX         7778

/*----------------------------------------------------------------------*/
/* Definitions for docsIfExt2 MIB variables                             */
/* The DOCS-IFEXT2-MIB as presented in OSSIv2[1].0-N-04.0166-3          */
/* GET's and SET's use the same OID                                     */
/* Note that some of these MIB's may be maintained by the app and not   */
/* retrieved vial HAL calls, but the OIDs are defined for completeness. */
/*----------------------------------------------------------------------*/

// docsIfExt2CmMscStatusIUC2Control
#define MIB_OID_CM_MSC_STATUS_IUC2_CONTROL      0x400A0600

// Max Scheduled Codes MIB (docsIfExt2CmMscStatusIUC2Control) 
// This defaults to the CM not being able to use IUC2 if MSC is enabled on the CM.  
// However, this object is read/write.  If the value is set to freeIUC2(3), the CM 
// is allowed to use IUC2 even if it could potentially cause the CM to exceed the MSC
// value in some cases.
typedef enum
{
    kCmIUC2ControlNoIUC2 = 1,  // CM usage of IUC2 is prohibited
    kCmIUC2ControlLimitedIUC2 = 2,  // CM is allowed to use IUC2 as long as the CM does not exceed MSC
    kCmIUC2ControlFreeIUC2 = 3 // CM is allowed to use IUC2 without regard for MSC.
}CmMscStatusIUC2Control;



/*----------------------------------------------------------------------*/
/* Definitions for USB MIB variables                                    */
/* Currently internet draft.                                            */
/*----------------------------------------------------------------------*/

/* usbPortTable objects */
#define MIB_OID_USBPORTTYPE               0x400B0202 /* usbPortType */ 
#define MIB_OID_USBPORTRATE               0x400B0203 /* usbPortRate */

/* usbDeviceTable objects */
#define MIB_OID_USBDEVICEPOWER            0x400B0302 /* usbDevicePower */
#define MIB_OID_USBDEVICEVENDORID         0x400B0303 /* usbDeviceVendorID */
#define MIB_OID_USBDEVICEPRODUCTID        0x400B0304 /* usbDeviceProductID */
#define MIB_OID_USBDEVICENUMCFGS          0x400B0305 /* usbDeviceNumberConfigurations */
#define MIB_OID_USBDEVICEACTIVECLASS      0x400B0306 /* usbDeviceActiveClass */
#define MIB_OID_USBDEVICESTATUS           0x400B0307 /* usbDeviceStatus */
#define MIB_OID_USBDEVICEENUMCOUNTER      0x400B0308 /* usbDeviceEnumCounter */
#define MIB_OID_USBDEVICEREMOTEWAKEUP     0x400B0309 /* usbDeviceRemoteWakeup */
#define MIB_OID_USBDEVICEREMOTEWAKEUPON   0x400B030A /* usbDeviceRemoteWakeupOn */

/* usbCDCTable objects */
#define MIB_OID_USBCDCIFINDEX             0x400B0402 /* usbCDCIfIndex */
#define MIB_OID_USBCDCSUBCLASS            0x400B0403 /* usbCDCSubclass */
#define MIB_OID_USBCDCVERSION             0x400B0404 /* usbCDCVersion */
#define MIB_OID_USBCDCDATATRANSFERTYPE    0x400B0405 /* usbCDCDataTransferType */
#define MIB_OID_USBCDCDATAENDPOINTS       0x400B0406 /* usbCDCDataEndPoints */
#define MIB_OID_USBCDCSTALLS              0x400B0407 /* usbCDCStalls */

/* usbCDCEtherTable objects */
#define MIB_OID_USBCDCETHERIFINDEX        0x400B0502 /* usbCDCEtherIfIndex */
#define MIB_OID_USBCDCETHERMACADDRESS     0x400B0503 /* usbCDCEtherMacAddress */
#define MIB_OID_USBCDCETHERPACKETFILTER   0x400B0504 /* usbCDCEtherPacketFilter */
#define MIB_OID_USBCDCETHERDATASTATSCAP   0x400B0505 /* usbCDCEtherDataStatisticsCapabilities */
#define MIB_OID_USBCDCETHERDATACHECKERRS  0x400B0506 /* usbCDCEtherDataCheckErrs */

/* usbCDCEtherXmtAddressTable objects */
#define MIB_OID_USBCDCETHERXMTADDR        0x400B0601 /* usbCDCEtherXmtAddress */

/* Misc objects */
#define MIB_OID_USBHOSTVERSION		  	  0x400B0701 /* USB Host Version */    	


/*\
 *  OIDs from the 802.11 MIB
 *
 *  Note that constants are derived from the MIB OIDS in the following manner
 *  0x4211GTLL
 *  where 0x4 follows the MIB OID convention
 *  211 indicates 802.11 MIB
 *  G is group number
 *  T is table number
 *  LL is leaf number
\*/
 
/*      dot11smt group                                */

//      dot11StationConfigTable
#define MIB_OID_DOT11STATIONID	                      0x42111101
#define MIB_OID_DOT11MEDIUMOCCUPANCYLIMIT	          0x42111102
#define MIB_OID_DOT11CFPOLLABLE	                      0x42111103 
#define MIB_OID_DOT11CFPPERIOD                        0x42111104 
#define MIB_OID_DOT11CFPMAXDURATION	                  0x42111105 
#define MIB_OID_DOT11AUTHENTICATIONRESPONSETIMEOUT	  0x42111106 
#define MIB_OID_DOT11PRIVACYOPTIONIMPLEMENTED	      0x42111107 // CW22
#define MIB_OID_DOT11POWERMANAGEMENTMODE	          0x42111108 
#define MIB_OID_DOT11DESIREDSSID	                  0x42111109 
#define MIB_OID_DOT11DESIREDBSSTYPE	                  0x4211110A 
#define MIB_OID_DOT11OPERATIONALRATESET               0x4211110B 
#define MIB_OID_DOT11BEACONPERIOD	                  0x4211110C // CW22
#define MIB_OID_DOT11DTIMPERIOD	                      0x4211110D 
#define MIB_OID_DOT11ASSOCIATIONRESPONSETIMEOUT	      0x4211110E 
#define MIB_OID_DOT11DISASSOCIATEREASON	              0x4211110F 
#define MIB_OID_DOT11DISASSOCIATESTATION	          0x42111110 
#define MIB_OID_DOT11DEAUTHENTICATEREASON             0x42111111 
#define MIB_OID_DOT11DEAUTHENTICATESTATION            0x42111112 
#define MIB_OID_DOT11AUTHENTICATEFAILSTATUS           0x42111113 
#define MIB_OID_DOT11AUTHENTICATEFAILSTATION          0x42111114 
#define MIB_OID_DOT11MULTIDOMAINCAPABILITYIMPLEMENTED 0x42111115 	
#define MIB_OID_DOT11MULTIDOMAINCAPABILITYENABLED     0x42111116 
#define MIB_OID_DOT11COUNTRYSTRING	                  0x42111117 
//      dot11AuthenticationAlgorithmsTable
#define MIB_OID_DOT11AUTHENTICATIONALGORITHM	      0x42111202
#define MIB_OID_DOT11AUTHENTICATIONALGORITHMSENABLE	  0x42111203 
//      dot11WEPDefaultKeysTable
#define MIB_OID_DOT11WEPDEFAULTKEYVALUE	              0x42111302 // CW22
//      dot11WEPKeyMappingsTable
#define MIB_OID_DOT11WEPKEYMAPPINGADDRESS             0x42111402 
#define MIB_OID_DOT11WEPKEYMAPPINGWEPON	              0x42111403 
#define MIB_OID_DOT11WEPKEYMAPPINGVALUE	              0x42111404 
#define MIB_OID_DOT11WEPKEYMAPPINGSTATUS	          0x42111405 
//      dot11PrivacyTable
#define MIB_OID_DOT11PRIVACYINVOKED                   0x42111501 // CW22
#define MIB_OID_DOT11WEPDEFAULTKEYID                  0x42111502 // CW22
#define MIB_OID_DOT11WEPKEYMAPPINGLENGTH              0x42111503 
#define MIB_OID_DOT11EXCLUDEUNENCRYPTED               0x42111504 // CW22
#define MIB_OID_DOT11WEPICVERRORCOUNT                 0x42111505 
#define MIB_OID_DOT11WEPEXCLUDEDCOUNT                 0x42111506 // CW22
//      dot11MultiDomainCapabilityTable
#define MIB_OID_DOT11FIRSTCHANNELNUMBER               0x42111702 
#define MIB_OID_DOT11NUMBEROFCHANNELS                 0x42111703 
#define MIB_OID_DOT11MAXIMUMTRANSMITPOWERLEVEL        0x42111704 

/*      dot11mac group                                */

//      dot11OperationTable
#define MIB_OID_DOT11MACADDRESS                       0x42112101 // CW22
#define MIB_OID_DOT11RTSTHRESHOLD                     0x42112102 // CW22
#define MIB_OID_DOT11SHORTRETRYLIMIT                  0x42112103 
#define MIB_OID_DOT11LONGRETRYLIMIT                   0x42112104 
#define MIB_OID_DOT11FRAGMENTATIONTHRESHOLD           0x42112105 // CW22
#define MIB_OID_DOT11MAXTRANSMITMSDULIFETIME          0x42112106 
#define MIB_OID_DOT11MAXRECEIVELIFETIME               0x42112107 
#define MIB_OID_DOT11MANUFACTURERID                   0x42112108 // CW22
#define MIB_OID_DOT11PRODUCTID                        0x42112109 // CW22
//      dot11CountersTable
#define MIB_OID_DOT11TRANSMITTEDFRAGMENTCOUNT         0x42112201 // CW22
#define MIB_OID_DOT11MULTICASTTRANSMITTEDFRAMECOUNT   0x42112202 // CW22
#define MIB_OID_DOT11FAILEDCOUNT                      0x42112203 // CW22
#define MIB_OID_DOT11RETRYCOUNT                       0x42112204 // CW22
#define MIB_OID_DOT11MULTIPLERETRYCOUNT               0x42112205 // CW22
#define MIB_OID_DOT11FRAMEDUPLICATECOUNT              0x42112206 // CW22
#define MIB_OID_DOT11RTSSUCCESSCOUNT                  0x42112207 // CW22
#define MIB_OID_DOT11RTSFAILURECOUNT                  0x42112208 // CW22
#define MIB_OID_DOT11ACKFAILURECOUNT                  0x42112209 // CW22
#define MIB_OID_DOT11RECEIVEDFRAGMENTCOUNT            0x4211220A // CW22
#define MIB_OID_DOT11MULTICASTRECEIVEDFRAMECOUNT      0x4211220B // CW22
#define MIB_OID_DOT11FCSERRORCOUNT                    0x4211220C // CW22
#define MIB_OID_DOT11TRANSMITTEDFRAMECOUNT            0x4211220D // CW22
#define MIB_OID_DOT11WEPUNDECRYPTABLECOUNT            0x4211220E // CW22
//      dot11GroupAddressesTable
#define MIB_OID_DOT11ADDRESS                          0x42112302 
#define MIB_OID_DOT11GROUPADDRESSESSTATUS             0x42112303 

/*      dot11res group                                */

#define MIB_OID_DOT11RESOURCETYPEIDNAME               0x42113101 
//      dot11ResourceInfoTable
#define MIB_OID_DOT11MANUFACTUREROUI                  0x42113201 
#define MIB_OID_DOT11MANUFACTURERNAME                 0x42113202 
#define MIB_OID_DOT11MANUFACTURERPRODUCTNAME          0x42113203 
#define MIB_OID_DOT11MANUFACTURERPRODUCTVERSION       0x42113204 

/*      dot11phy group                                */

//      dot11PhyOperationTable
#define MIB_OID_DOT11PHYTYPE                          0x42114101 // CW22
#define MIB_OID_DOT11CURRENTREGDOMAIN                 0x42114102 // CW22
#define MIB_OID_DOT11TEMPTYPE                         0x42114103 // CW22
//      dot11PhyAntennaTable
#define MIB_OID_DOT11CURRENTTXANTENNA                 0x42114201 
#define MIB_OID_DOT11DIVERSITYSUPPORT                 0x42114202 
#define MIB_OID_DOT11CURRENTRXANTENNA                 0x42114203 
//      dot11PhyTxPowerTable
#define MIB_OID_DOT11NUMBERSUPPORTEDPOWERLEVELS       0x42114301 
#define MIB_OID_DOT11TXPOWERLEVEL1                    0x42114302 
#define MIB_OID_DOT11TXPOWERLEVEL2                    0x42114304 
#define MIB_OID_DOT11TXPOWERLEVEL3                    0x42114305 
#define MIB_OID_DOT11TXPOWERLEVEL4                    0x42114306 
#define MIB_OID_DOT11TXPOWERLEVEL5                    0x42114307 
#define MIB_OID_DOT11TXPOWERLEVEL6                    0x42114308 
#define MIB_OID_DOT11TXPOWERLEVEL7                    0x42114309 
#define MIB_OID_DOT11TXPOWERLEVEL8                    0x4211430A 
#define MIB_OID_DOT11CURRENTTXPOWERLEVEL              0x4211430B
//      dot11PhyFHSSTable
#define MIB_OID_DOT11HOPTIME                          0x42114401 
#define MIB_OID_DOT11CURRENTCHANNELNUMBER             0x42114402 
#define MIB_OID_DOT11MAXDWELLTIME                     0x42114403 
#define MIB_OID_DOT11CURRENTDWELLTIME                 0x42114404 
#define MIB_OID_DOT11CURRENTSET                       0x42114404 
#define MIB_OID_DOT11CURRENTPATTERN                   0x42114405 
#define MIB_OID_DOT11CURRENTINDEX                     0x42114406 
#define MIB_OID_DOT11EHCCPRIMERADIX                   0x42114407 
#define MIB_OID_DOT11EHCCNUMBEROFCHANNELSFAMILYINDEX  0x42114408 
#define MIB_OID_DOT11EHCCCAPABILITYIMPLEMENTED        0x42114409 
#define MIB_OID_DOT11EHCCCAPABILITYENABLED            0x4211440A 
#define MIB_OID_DOT11HOPALGORITHMADOPTED              0x4211440B 
#define MIB_OID_DOT11RANDOMTABLEFLAG                  0x4211440C 
#define MIB_OID_DOT11NUMBEROFHOPPINGSETS              0x4211440D 
#define MIB_OID_DOT11HOPMODULUS                       0x4211440E 
#define MIB_OID_DOT11HOPOFFSET                        0x4211440F 
//      dot11PhyDSSSEntry
#define MIB_OID_DOT11CURRENTCHANNEL                   0x42114501 
#define MIB_OID_DOT11CCAMODESUPPORTED                 0x42114502 
#define MIB_OID_DOT11CURRENTCCAMODE                   0x42114503 
#define MIB_OID_DOT11EDTHRESHOLD                      0x42114504 
//      dot11PhyIRTable
#define MIB_OID_DOT11CCAWATCHDOGTIMERMAX              0x42114601 
#define MIB_OID_DOT11CCAWATCHDOGCOUNTMAX              0x42114602 
#define MIB_OID_DOT11CCAWATCHDOGTIMERMIN              0x42114603 
#define MIB_OID_DOT11CCAWATCHDOGCOUNTMIN              0x42114604 
//      dot11RegDomainsSupportedTable
#define MIB_OID_DOT11REGDOMAINSSUPPORTVALUE           0x42114702 
//      dot11AntennasListTable
#define MIB_OID_DOT11SUPPORTEDTXANTENNA               0x42114802 
#define MIB_OID_DOT11SUPPORTEDRXANTENNA               0x42114803 
#define MIB_OID_DOT11DIVERSITYSELECTIONRX             0x42114804 
//      dot11SupportedDataRatesTxTable
#define MIB_OID_DOT11SUPPORTEDDATARATESTXVALUE        0x42114902 
//      dot11SupportedDataRatesRxTable
#define MIB_OID_DOT11SUPPORTEDDATARATESRXVALUE        0x42114A02 
//      dot11PhyHRDSSSTable
#define MIB_OID_DOT11SHORTPREAMBLEOPTIONIMPLEMENTED   0x42114B01 
#define MIB_OID_DOT11PBCCOPTIONIMPLEMENTED            0x42114B02 
#define MIB_OID_DOT11CHANNELAGILITYPRESENT            0x42114B03 
#define MIB_OID_DOT11CHANNELAGILITYENABLED            0x42114B04 
#define MIB_OID_DOT11HRCCAMODESUPPORTED               0x42114B05 
//      dot11HoppingPatternTable
#define MIB_OID_DOT11RANDOMTABLEFIELDNUMBER           0x42114C02 

/* Special MIB to reset counters all to 0, not from the IEEE MIB */
#define MIB_OID_DOT11COUNTERSRESET                    0x40331008

/* Some MIBs that certain MSOs want (eg Comcast) */
#define MIB_OID_RXBUFFERNUM                           0x41100001
#define MIB_OID_TXBUFFERNUM                           0x41100002

/* DOCS-QOS3-MIB */

// docsQos3CmDsidTable
#define MIB_OID_DOCSQOS3CMDSIDOUTOFRANGEDISCARDS                      0x41511508
#define MIB_OID_DOCSQOS3CMDSIDNEXTEXPECTEDSEQNUM                      0x41511509
// docsQos3CmDsidStatsTable
#define MIB_OID_DOCSQOS3CMDSIDSTATSSEQNUMMISSING                      0x41511602
#define MIB_OID_DOCSQOS3CMDSIDSTATSSKEWTHRESHEXCEEDS                  0x41511603
#define MIB_OID_DOCSQOS3CMDSIDSTATSOUTOFRANGEPACKETS                  0x41511604
#define MIB_OID_DOCSQOS3CMDSIDSTATSNUMPACKETS                         0x41511605

/* DOCS-IF3-MIB */

// docsIf3SignalQualityExtTable
#define MIB_OID_DOCSIF3SIGNALQUALITYEXTRXMER                          0x41411801
#define MIB_OID_DOCSIF3SIGNALQUALITYEXTRXMERSAMPLES                   0x41411802

/*----------------------------------------------------------------------*/
/* Structure that contains the MIB variables that are                   */
/* common to all HAL interfaces.                                        */
/*----------------------------------------------------------------------*/

#include "Counter64.h" // for Counter64 structure
typedef struct _HalMib
{
   /*----------------------------------------------------------------------*/
   /* Definitions for Interface Group MIB variables                        */
   /* GET command definitions (ifTable and ifXTable objects, specifically) */
   /*----------------------------------------------------------------------*/
    unsigned long   ifMtu;
    unsigned long   ifSpeed;
    unsigned long   ifInOctets;
    unsigned long   ifInUcastPkts;
    unsigned long   ifInNUcastPkts;
    unsigned long   ifInDiscards;
    unsigned long   ifInErrors;
    unsigned long   ifInUnknownProtos;
    unsigned long   ifOutOctets;
    unsigned long   ifOutUcastPkts;
    unsigned long   ifOutNUcastPkts;
    unsigned long   ifOutDiscards;
    unsigned long   ifOutErrors;
    unsigned char    ifOutQLen;

   /*----------------------------------------------------------------------*/
   /* Definitions for ifXTable MIB variables                               */
   /* RFC-2233, GET's and SET's use the same OID                           */
   /*----------------------------------------------------------------------*/
    unsigned long   ifInMulticastPkts;       
    unsigned long   ifInBroadcastPkts;       
    unsigned long   ifOutMulticastPkts;       
    unsigned long   ifOutBroadcastPkts;       

    Counter64 ifHCInOctets;
    Counter64 ifHCInUcastPkts;
    Counter64 ifHCInMulticastPkts;
    Counter64 ifHCInBroadcastPkts;
    Counter64 ifHCOutOctets;
    Counter64 ifHCOutUcastPkts;
    Counter64 ifHCOutMulticastPkts;
    Counter64 ifHCOutBroadcastPkts;

} HalMib;

#endif

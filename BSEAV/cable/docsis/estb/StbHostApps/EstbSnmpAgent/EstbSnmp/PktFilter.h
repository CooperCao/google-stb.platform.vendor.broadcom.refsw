//**************************************************************************
//
//    Copyright 1999  Broadcom Corporation
//    All Rights Reserved
//    No portions of this material may be reproduced in any form without the
//    written permission of:
//             Broadcom Corporation
//             16251 Laguna Canyon Road
//             Irvine, California  92618
//    All information contained in this document is Broadcom Corporation
//    company private, proprietary, and trade secret.
//
//
//
//**************************************************************************
//    Filename: PktFilter.h
//    Author:   Kevin O'Neal
//    Creation Date: 27-april-2000
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    Common definitions for packet filters
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef PKTFILTER_H
#define PKTFILTER_H

typedef unsigned long PKTFILTER_STATUS;

#define PKTFILTER_INBOUND           1
#define PKTFILTER_OUTBOUND          2
#define PKTFILTER_BOTH              3

#define PKTFILTER_NOT_ACTIVE        2
#define PKTFILTER_MATCH             0
#define PKTFILTER_NO_MATCH          1
// NEW PKTFILTER_STATUS value PKTFILTER_IF_MATCH: this return code indicates
// that the packet matched no filters, but that there is at least one active
// filter configured for the interface associated with the packet.  This is
// necessary to support the undocumented requirement that the filter default
// action must be ignored if no filters are configured for the interface
// associated with the packet.
#define PKTFILTER_IF_MATCH          3 

#define PKTFILTER_NO_OPINION        0x00000000
#define PKTFILTER_DISCARD           0x00000001
#define PKTFILTER_PASS              0x00000002

#define PKTFILTER_CONTINUE          0x00000100 // OR'd in w/ pass, discard, or N/O

#define STATIC_ALL_CPES_IFNUM       1
#define STATIC_CM_IFNUM             2
#define STATIC_CM_DS_IFNUM          3
#define STATIC_CM_US_IFNUM          4

// Include the following definition to get support for the special
// 1.1 requirement that may be secretly also required by DOCSIS 1.0,
// never mind what the spec says.  Here it is, from OSSI_I02_000714.pdf page 16:
//
//"DOCSIS CM may have multiple interfaces. If filter(s) are applied to IfIndex 
// 1, then the filter(s) are also applied to each CPE interface; however, 
// filters are never used to limit traffic between CM CPE interfaces."
#define SUPPORT_CMCI_IF_CONVERSION

// If the following macro is defined, only bridged packets will have filters
// applied.  That is, if the packet is targeted to the CM or originating from
// the CM, it will not have filters applied.  This means, for example, that if
// an IP filter was set up to discard ICMP packets, then you could still ping
// the CM, but the CM would not forward ping packets upstream or downstream.
// The spec doesn't explicitly state this requirement anywhere that I can see,
// but it does seem to be the community consensus based on reflector traffic, 
// etc. that we should only apply filters to bridged traffic.
#define FILTER_BRIDGED_PACKETS_ONLY

// By default, we'll support filter logging if we aren't building a slim load.
// This does introduce a slight throughput hit, though, so if you like you can
// remove filter logging support even for the full load by removing this macro.
#if (!BCM_REDUCED_IMAGE_SIZE)
#define FILTER_LOGGING_SUPPORT
#endif

#endif

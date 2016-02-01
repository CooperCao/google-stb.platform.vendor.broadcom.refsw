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
//    Filename: SnmpAgnt.h
//    Author:   Kevin O'Neal
//    Creation Date: 25-may-99
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    Portable starting point for the SNMP agent
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************
#ifndef SNMPAGNT_H
#define SNMPAGNT_H

// 11/14/2000: Split the CM snmp agent and the EMTA snmp agent into
// 2 modules:
#if (CM_HAL_INCLUDED)
#include "CmSnmpAgent.h"
#include "CpeSnmpAgent.h"
#endif

#if (BCM_CABLEHOME_SUPPORT)
#include "CableHomeSnmpAgent.h"
#endif

#if (EMTA_SUPPORT)
#include "EmtaSnmpAgent.h"
#endif

#endif

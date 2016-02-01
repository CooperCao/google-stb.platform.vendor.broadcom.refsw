//**************************************************************************
//
//    Copyright 2001 Broadcom Corporation
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
//    Filename: SnmpLog.h
//    Author:   Kevin O'Neal
//    Creation Date: 31-Aug-2001
//
//**************************************************************************
//    Description:
//
//      Cablemodem V2 code
//      
//  Macros for SNMP message logging, plus ability to compile them in or out
//
//      
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef SNMPLOG_H
#define SNMPLOG_H

#include "MessageLog.h"

// Supported logging with SNMP support compiled in:
#if (SNMP_SUPPORT)

#include "BcmSnmpAgent.h"

#define SNMP_RAW_LOGGING        1
#define SNMP_WARN_LOGGING       1
#define SNMP_ERR_LOGGING        1
#define SNMP_START_LOGGING    0
#define SNMP_INIT_LOGGING     0
#define SNMP_MEM_LOGGING      0
#define SNMP_CFG_LOGGING      0
#define SNMP_SEARCH_LOGGING   0
#define SNMP_REQ_LOGGING        1
#define SNMP_NM_LOGGING         1
#define SNMP_FILT_LOGGING       1
#define SNMP_EV_LOGGING         1
#define SNMP_THREAD_LOGGING     1
#define SNMP_AGENT_LOGGING      1

// Supported logging with SNMP support compiled out:
#else

#define SNMP_RAW_LOGGING        1
#define SNMP_WARN_LOGGING     0  
#define SNMP_ERR_LOGGING      0  
#define SNMP_START_LOGGING    0
#define SNMP_INIT_LOGGING     0
#define SNMP_MEM_LOGGING      0
#define SNMP_CFG_LOGGING      0
#define SNMP_SEARCH_LOGGING   0
#define SNMP_REQ_LOGGING      0
#define SNMP_NM_LOGGING       0
#define SNMP_FILT_LOGGING     0
#define SNMP_EV_LOGGING       0  
#define SNMP_THREAD_LOGGING   0
#define SNMP_AGENT_LOGGING    0

#endif


/* Macros for SNMP message logging defined based on value of SNMP_LOG_SUPPORT */

#if (SNMP_RAW_LOGGING)
#define SnmpLogRaw gLogMessageRaw
#else
#define SnmpLogRaw gNullMessageLog
#endif

#if (SNMP_WARN_LOGGING)
#define SnmpLogWarn gWarningMsgNoFields (BcmSnmpAgent::gMessageLogSettings)
#else
#define SnmpLogWarn gNullMessageLog
#endif

#if (SNMP_ERR_LOGGING)
#define SnmpLogErr gErrorMsgNoFields (BcmSnmpAgent::gMessageLogSettings)
#else
#define SnmpLogErr gNullMessageLog
#endif

#if (SNMP_START_LOGGING)
#define SnmpLogStart gAppMsgNoFields (BcmSnmpAgent::gMessageLogSettings, BcmSnmpAgent::kStartupMessages)
#else
#define SnmpLogStart gNullMessageLog
#endif

#if (SNMP_INIT_LOGGING)
#define SnmpLogInit gAppMsgNoFields (BcmSnmpAgent::gMessageLogSettings, BcmSnmpAgent::kInitializationMessages)
#else
#define SnmpLogInit gNullMessageLog
#endif

#if (SNMP_SEARCH_LOGGING)
#define SnmpLogSearch gAppMsgNoFields (BcmSnmpAgent::gMessageLogSettings, BcmSnmpAgent::kSearchMessages)
#else
#define SnmpLogSearch gNullMessageLog
#endif

#if (SNMP_MEM_LOGGING)
#define SnmpLogMem gAppMsgNoFields (BcmSnmpAgent::gMessageLogSettings, BcmSnmpAgent::kMemoryMessages)
#else
#define SnmpLogMem gNullMessageLog
#endif

#if (SNMP_CFG_LOGGING)
#define SnmpLogCfg gAppMsgNoFields (BcmSnmpAgent::gMessageLogSettings, BcmSnmpAgent::kConfigFileMessages)
#else
#define SnmpLogCfg gNullMessageLog
#endif

#if (SNMP_REQ_LOGGING)
#define SnmpLogReq gAppMsgNoFields (BcmSnmpAgent::gMessageLogSettings, BcmSnmpAgent::kRequestMessages)
#else
#define SnmpLogReq gNullMessageLog
#endif

#if (SNMP_NM_LOGGING)
#define SnmpLogNm gAppMsgNoFields (BcmSnmpAgent::gMessageLogSettings, BcmSnmpAgent::kNmAccessMessages)
#else
#define SnmpLogNm gNullMessageLog
#endif

#if (SNMP_FILT_LOGGING)
#define SnmpLogFilt gAppMsgNoFields (BcmSnmpAgent::gMessageLogSettings, BcmSnmpAgent::kPktFilterMessages)
#else
#define SnmpLogFilt gNullMessageLog
#endif

#if (SNMP_EV_LOGGING)
#define SnmpLogEv gAppMsgNoFields (BcmSnmpAgent::gMessageLogSettings, BcmSnmpAgent::kEventLogMessages)
#else
#define SnmpLogEv gNullMessageLog
#endif

#if (SNMP_THREAD_LOGGING)
#define SnmpLogThd gAppMsgNoFields (BcmSnmpAgent::gMessageLogSettings, BcmSnmpAgent::kThreadMessages)
#else
#define SnmpLogThd gNullMessageLog
#endif

#if (SNMP_AGENT_LOGGING)
#define SnmpLogAgnt gAppMsgNoFields (BcmSnmpAgent::gMessageLogSettings, BcmSnmpAgent::kAgentMessages)
#else
#define SnmpLogAgnt gNullMessageLog
#endif

#endif

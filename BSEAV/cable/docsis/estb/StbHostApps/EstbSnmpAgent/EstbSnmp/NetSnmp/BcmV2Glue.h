//****************************************************************************
//
// Copyright (c) 2007-2010 Broadcom Corporation
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
//**************************************************************************
//    Filename: BcmV2glue.h
//    Author:  
//    Creation Date: Moved from LibSupport/NetSnmp/include location 11-18-03
//
//**************************************************************************
//    Description:
//
//      		
//    Agent API functions for configuring NetSnmp.
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef BCMV2GLUE_H
#define BCMV2GLUE_H

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/types.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "SLog.h"

#ifdef __cplusplus
extern          "C" {
#endif

    int
    bcmV2_set_engineID(unsigned char *buf, int bufLen);

    int
    bcmV2_set_context(unsigned char *buf, int bufLen);

    int
    bcmV2_get_authFailTrapEnabled(void);

    int
    bcmV2_install_user(unsigned char *userName, int secLevel);

    int
    bcmV2_enable_user(unsigned char *engineId, int engineIdLen, unsigned char *userName);

    int
    bcmV2_disable_user(unsigned char *engineId, int engineIdLen, unsigned char *userName);

    int
    bcmV2_destroy_user(unsigned char *engineId, int engineIdLen, unsigned char *userName);

    struct usmUser *
    bcmV2_usm_get_user_from_list(unsigned char *engineID, size_t engineIDLen,
                                 struct usmUser *puserList);
    void
    bcmV2_destroy_all_users(unsigned char *engineId, int engineIdLen);

    int
    bcmV2_get_docsis_users(unsigned char *engineId, int engineIdLen, unsigned char *buf);

    int
    bcmV2_set_userKey(unsigned char *engineId, int engineIdLen, unsigned char *userName,
                      int keyId, unsigned char *key, int keyLen);

    void
    bcmV2_destroy_all_views (void);

    void
    bcmV2_destroy_all_groups (void);

    void
    bcmV2_destroy_all_accessEntries (void);

    int
    bcmV2_set_engineID(unsigned char *buf, int bufLen);

    void
    bcmV2_input_parameters(char param); 

    int
    bcmV2_agent_init();
    
    int 
    bcmV2_snmp_read (unsigned long DstIp, unsigned short DstPort, int Socket);

    int
    bcmV2_using_openssl_check();

    int
    bcmV2_scalar_nosuchinstance(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests);

    int 
    bcmV2_do_scalar  (netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests);

    int 
    bcmV2_do_tabular (netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests);
    
    int 
    bcmV2_do_group  (netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests);

// Can't define this here right now because of include path issues...
//netsnmp_variable_list *bcmV2_get_variable_list (MibObject *pObjects);

    int bcmV2_send_V2_trap (const unsigned char *pFromIp, const unsigned char *pToIp, int ToPort, const char *pCommunity, oid *pTrapOID, int TrapOIDLength, time_t sysUpTime, netsnmp_variable_list *vars);

    int bcmV2_send_V1_trap (const unsigned char *pFromIp, const unsigned char *pToIp, int ToPort, const char *pCommunity, int generic, int specific, oid *enterprise, int enterprise_length, time_t sysUpTime, netsnmp_variable_list *vars);
    
    void bcmV2_agent_show ();

// Given a SNMPv2c Trap we try to figure out what kind of trap
// If we have a clue, we return our internal code for the trap type
int
GetSnmpV2cTrapType (unsigned char *data, int dataLen);
#define BCMV2_TRAP_TYPE_UNKNOWN 0
#define BCMV2_TRAP_TYPE_OCSTBPANICDUMP 1


// Prototype for printf drop-in that does nothing.
    int 
    bcmV2_dontprintf (const char *format, ...);

// Use this if you want debugging turned on no matter what...
// #define DEBUG printf

// Use this if you don't want debugging turned on no matter what....
//#define DEBUG bcmV2_dontprintf

// Use this if you want simple run-time debugging based on debug level.
// Debugging will be turned on if debug level is != 0, off if 0
#define DEBUG SSimpleLog

int
EcmGetObject(oid *ecmObj, int ecmObjLen, u_char *ecmObjType,
            void *val, size_t *valLen);

#ifdef __cplusplus
}
#endif
#endif


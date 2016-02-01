//**************************************************************************
//
// Copyright (c) 2007-2009 Broadcom Corporation
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
//  $Id$
//
//  Filename:       DSGSvcMgrThread.cpp
//  Author:         Paul McGlynn
//  Creation Date:  June 4, 2003
//
//****************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    Portable types for SNMP agent
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef SNMPSTATUS_H
#define SNMPSTATUS_H

typedef unsigned long SNMP_STATUS;

/* Error codes for get/set functions        V1/V2   translation */  
#define SNMP_NO_ERROR                 0  //  Both   SNMP_NO_ERROR
#define SNMP_TOO_BIG                  1  //  Both   SNMP_TOO_BIG
#define SNMP_NO_SUCH_NAME             2  //  V1     SNMP_NO_SUCH_OBJECT
#define SNMP_BAD_VALUE                3  //  Both   SNMP_BAD_VALUE
#define SNMP_READ_ONLY                4  //  Both   SNMP_READ_ONLY
#define SNMP_GEN_ERR                  5  //  Both   SNMP_GEN_ERROR
#define SNMP_NO_ACCESS                6  //   V2    SNMP_NO_SUCH_NAME
#define SNMP_WRONG_TYPE               7  //   V2    SNMP_BAD_VALUE
#define SNMP_WRONG_LENGTH             8  //   V2    SNMP_BAD_VALUE
#define SNMP_WRONG_ENCODING           9  //   V2    SNMP_BAD_VALUE
#define SNMP_WRONG_VALUE              10 //   V2    SNMP_BAD_VALUE
#define SNMP_NO_CREATION              11 //   V2    SNMP_NO_SUCH_NAME
#define SNMP_INCONSISTENT_VALUE       12 //   V2    SNMP_BAD_VALUE
#define SNMP_RESOURCE_UNAVAILABLE     13 //   V2    SNMP_GEN_ERROR
#define SNMP_COMMIT_FAILED            14 //   V2    SNMP_GEN_ERROR
#define SNMP_UNDO_FAILED              15 //   V2    SNMP_GEN_ERROR
#define SNMP_AUTHORIZATION_ERROR      16 //   V2    SNMP_NO_SUCH_NAME
#define SNMP_NOT_WRITABLE             17 //   V2    SNMP_NO_SUCH_NAME
#define SNMP_INCONSISTENT_NAME        18 //   V2    SNMP_NO_SUCH_NAME
#define SNMP_LAST_ERROR               19 //   V2    none

// PR 11894:  noSuchObject and noSuchInstance are actually exceptions, 
// not errors, which are encoded in the type field of the varbinds that
// which have these exceptions.
#define SNMP_NO_SUCH_INSTANCE         129

// This is really not correct per PR 11894 but unfortunately NO_SUCH_NAME and
// NO_SUCH_OBJECT are used interchangeably throughout the code and unwinding it
// would be much too risky.
#define SNMP_NO_SUCH_OBJECT           SNMP_NO_SUCH_NAME // Really should be 128


// A couple of error/status codes that aren't SNMP standard codes, but used
// internally by our implementation:
#define SNMP_SCALAR_OBJECT            100
#define SNMP_NO_OPINION               101
#define SNMP_EVENT_LOG_THROTTLED      102
#define SNMP_DUPLICATE_VALUE          103  // Somethings is duplicated
#define SNMP_BAD_COMMUNITY_NAME       104
#define SNMP_BAD_COMMUNITY_USE        105
#define SNMP_TEST_DONT_SET            0x100 // 0x100: OR'ed in w/ return value sometimes
#define SNMP_ROW_NOT_CREATED          0x200 // Indicate a set failed because the row hasn't been created
#define SNMP_APP_SPECIFIC_BASE        SNMP_SCALAR_OBJECT

// For user-creatable/destroyable tables, here are the status values:
#define SNMP_STATUS_INVISIBLE           0
#define SNMP_STATUS_ACTIVE	            1
#define SNMP_STATUS_NOT_IN_SERVICE	    2
#define SNMP_STATUS_NOT_READY	        3
#define SNMP_STATUS_CREATE_AND_GO	    4
#define SNMP_STATUS_CREATE_AND_WAIT	    5
#define SNMP_STATUS_DESTROY	            6

// Constant values for StorageType objects
#define SNMP_STORAGETYPE_OTHER        1
#define SNMP_STORAGETYPE_VOLATILE     2
#define SNMP_STORAGETYPE_NONVOLATILE  3
#define SNMP_STORAGETYPE_PERMANENT    4
#define SNMP_STORAGETYPE_READONLY     5

// Constant values for SecurityModel objects
#define SNMP_SECMODEL_ANY             0
#define SNMP_SECMODEL_SNMPV1          1
#define SNMP_SECMODEL_SNMPV2C         2
#define SNMP_SECMODEL_USM             3

// Constant values for InetAddressType / InetAddress things
#define SNMP_INETADDRESSTYPE_UNKNOWN  0
#define SNMP_INETADDRESSTYPE_IPV4  1
#define SNMP_INETADDRESSTYPE_IPV6  2
#define SNMP_INETADDRESSTYPE_IPV4Z  3
#define SNMP_INETADDRESSTYPE_IPV6Z  4
#define SNMP_INETADDRESSTYPE_DNS  16
#define SNMP_INETADDRESSLEN_UNKNOWN  0
#define SNMP_INETADDRESSLEN_IPV4  4
#define SNMP_INETADDRESSLEN_IPV6  16

/* SNMP_MODE values */
#define SNMP_MODE_NMACCESS            1
#define SNMP_MODE_COEXISTENCE         2 // support v1, v2c, and v3
#define SNMP_MODE_V3                  3

/* SNMP_VIEW_STATUS values control OID inclusion/exclusion */
#define VIEW_STATUS_EXCLUDE           2
#define VIEW_STATUS_INCLUDE           1

/* SNMP_SEC_LEVEL values */
#define SNMP_SEC_NONE                 1
#define SNMP_SEC_AUTH                 2
#define SNMP_SEC_PRIV                 3

/* SNMP_SEC_Model values  */
#define SNMP_SEC_MDL_V1               1
#define SNMP_SEC_MDL_V2               2
#define SNMP_SEC_MDL_USM              3

/* Message processing models */ 
#define SNMP_MP_MDL_V1	              0
#define SNMP_MP_MDL_V2C	              1
#define SNMP_MP_MDL_V2U	              2
#define SNMP_MP_MDL_V3	              3

/* Notify Table Types  */
#define SNMP_NOTIFYTYPE_TRAP          1
#define SNMP_NOTIFYTYPE_INFORM        2

/* NotifyFilter Table Types */
#define SNMP_NOTIFYFILTER_TYPE_INCLUDED 1
#define SNMP_NOTIFYFILTER_TYPE_EXCLUDED 2


/* SNMP version for this query */
/* Changed 11/20/2003 to use real packet numbers to avoid confusion w/ agent core */
#define SNMP_QUERY                    0
#define SNMP_V1_QUERY                 0
#define SNMP_V2_QUERY                 1 // SNMPv2c.  SNMPv2u was version 2, never really took off.
#define SNMP_V3_QUERY                 3

#define SNMP_TRUTH_TRUE                 1
#define SNMP_TRUTH_FALSE                2

// PDU types for authentication
#define PDU_GET_REQUEST                 0
#define PDU_GET_NEXT_REQUEST            1
#define PDU_GET_RESPONSE                2
#define PDU_SET_REQUEST                 3
#define PDU_TRAP                        4
#define PDU_GET_BULK_REQUEST            5
#define PDU_INFORM_REQUEST              6
#define PDU_TRAP2                       7
#define PDU_REPORT                      8
#define PDU_MAX_V1                      4
#define PDU_MAX                         8
#define PDU_NONE                        0xFF

// Not a real PDU value, used internally for preliminary set phases.
#define PDU_VALIDATE_SET_REQUEST        98 // check writability, type
#define PDU_TEST_REQUEST                99 // check value

// Generic trap types:
#define COLD_START_TRAP                 0
#define WARM_START_TRAP                 1
#define LINK_DOWN_TRAP                  2
#define LINK_UP_TRAP                    3
#define AUTH_FAILURE_TRAP               4
#define EGP_NEIGHBOR_LOSS_TRAP          5
#define ENTERPRISE_SPECIFIC_TRAP        6


#endif

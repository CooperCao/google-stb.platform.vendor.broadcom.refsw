//**************************************************************************
//
//    Copyright (c) 2008 Broadcom Corporation
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
//    Filename: SnmpStatus.h
//    Author:   Kevin O'Neal
//    Creation Date: 30-May-2001
//
//**************************************************************************
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
#define SNMP_NO_SUCH_OBJECT           SNMP_NO_SUCH_NAME
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

// A couple of error/status codes that aren't SNMP standard codes, but used
// internally by our implementation:
#define SNMP_SCALAR_OBJECT            100
#define SNMP_NO_OPINION               101
#define SNMP_EVENT_LOG_THROTTLED      102
#define SNMP_DUPLICATE_VALUE          103  // Somethings is duplicated
#define SNMP_BAD_COMMUNITY_NAME       104
#define SNMP_BAD_COMMUNITY_USE        105
#define SNMP_NO_SUCH_INSTANCE         106 // as opposed to NO_SUCH_OBJECT
#define SNMP_TEST_DONT_SET            0x80 // 128: OR'ed in w/ return value sometimes
#define SNMP_ROW_NOT_CREATED          0x100 // Indicate a set failed because the row hasn't been created
#define SNMP_APP_SPECIFIC_BASE        SNMP_SCALAR_OBJECT

// For user-creatable/destroyable tables, here are the status values:
#define SNMP_STATUS_INVISIBLE           0
#define SNMP_STATUS_ACTIVE	            1
#define SNMP_STATUS_NOT_IN_SERVICE	    2
#define SNMP_STATUS_NOT_READY	        3
#define SNMP_STATUS_CREATE_AND_GO	    4
#define SNMP_STATUS_CREATE_AND_WAIT	    5
#define SNMP_STATUS_DESTROY	            6


/* DOCSIS_MODE values - Docsis 1.0 or 1.1 */
#define DOCSIS_MODE_UNREGISTERED      0xFFFFFFFF // why not?
#define DOCSIS_MODE_PURE_1_0          0 // from NV setting
#define DOCSIS_MODE_1_0               kDocsisVersion1_0 // 1
#define DOCSIS_MODE_1_1               kDocsisVersion1_1 // 2
#define DOCSIS_MODE_2_0               kDocsisVersion2_0 // 3

/* SNMP_MODE values */
#define SNMP_MODE_NMACCESS            1
#define SNMP_MODE_COEXISTENCE         2 // support v1, v2c, and v3
#define SNMP_MODE_V3                  3

/* SNMP_VIEW_STATUS values control OID inclusion/exclusion */
#define VIEW_STATUS_EXCLUDE           0
#define VIEW_STATUS_INCLUDE           1

/* SNMP_SEC_LEVEL values - note these may be bitwise or-ed together */
#define SNMP_SEC_NONE                 0
#define SNMP_SEC_AUTH                 1
#define SNMP_SEC_PRIV                 2

/* SNMP_SEC_Model values  */
#define SNMP_SEC_MDL_V1               1
#define SNMP_SEC_MDL_V2               2
#define SNMP_SEC_MDL_USM              3


/* SNMP version for this query */
#define SNMP_QUERY                    0
#define SNMP_V1_QUERY                 1
#define SNMP_V2_QUERY                 2
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

// Generic trap types:
#define COLD_START_TRAP                 0
#define WARM_START_TRAP                 1
#define LINK_DOWN_TRAP                  2
#define LINK_UP_TRAP                    3
#define AUTH_FAILURE_TRAP               4
#define EGP_NEIGHBOR_LOSS_TRAP          5
#define ENTERPRISE_SPECIFIC_TRAP        6

#endif

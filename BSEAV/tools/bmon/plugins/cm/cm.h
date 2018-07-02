/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#ifndef CM_H
#define CM_H

#include <sys/utsname.h> /* struct utsname */

#define CM_PLUGIN_NAME         "cm"
#define CM_PLUGIN_VERSION      "0.3"
#define CM_PLUGIN_DESCRIPTION  "Collect specific cable modem MIBs or a MIB that is the top of a subtree"
#define BMON_CM_MIB_MAX_NUM    32

typedef enum
{
    CM_ERROR_NONE = 0,
    CM_ERROR_SNMP_OPEN_FAILED,
    CM_ERROR_SNMP_PARSE_OID,
    CM_ERROR_SNMP_BAD_OBJECT,
    CM_ERROR_SNMP_TIMEOUT_WAITING_FOR_PEER,
    CM_ERROR_SNMP_SYNCH_RESPONSE,
    CM_ERROR_TOO_MANY_OIDS_SPECIFIED,
    CM_ERROR_OID_NOT_SPECIFIED,
    CM_ERROR_INVALID_MIB_TYPE,
    CM_ERROR_SNMP_PACKET_ERROR,
    CM_ERROR_APPEND_PLUGIN_HEADER_FAILED,
    CM_ERROR_MAX
} cm_errors;

typedef struct bmon_cm_t
{
    unsigned int   mib_count;
    cm_errors      status_code;
    struct
    {
        char   mib_name[96];
        char   mib_type[32];
        char * mib_value;
    } mib_name_value[BMON_CM_MIB_MAX_NUM];
} bmon_cm_t;

#endif /* ifndef CM_H */
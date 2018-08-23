/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#ifndef _BRCMSTB_MBOX_H_
#define _BRCMSTB_MBOX_H_

/* Brcmstb service system functions */
#define BRCMSTB_SVC_MBOX_MSG            0x00
#define BRCMSTB_SVC_MBOX_MAX            0x01

/* Brcmstb mbox layout (following SCMI) */
typedef struct brcmstb_mbox {
    uint32_t reserved1;
    uint32_t status;
    uint32_t reserved2;
    uint32_t reserved3;
    uint32_t flags;
    uint32_t msg_len;
    uint32_t msg_hdr;
    uint32_t msg_payload[];
} brcmstb_mbox_t;

/* Brcmstb mbox msg header (following SCMI) */
typedef struct brcmstb_msg_hdr {
    uint32_t id     : 8;
    uint32_t type   : 2;
    uint32_t prot   : 8;
    uint32_t token  : 10;
    uint32_t mbz    : 4; /* must be zero */
} brcmstb_msg_hdr_t;

/* Brcmstb mbox msg common payload (following SCMI) */
typedef struct brcmstb_msg_payload {
    /* Return status */
    uint32_t status;
} brcmstb_msg_payload_t;

/* Brcmstb mbox msg protocols (following SCMI) */
#define BRCMSTB_MSG_PROT_BASE           0x10
#define BRCMSTB_MSG_PROT_POWER          0x11
#define BRCMSTB_MSG_PROT_SYSTEM         0x12
#define BRCMSTB_MSG_PROT_PERF           0x13
#define BRCMSTB_MSG_PROT_CLOCK          0x14
#define BRCMSTB_MSG_PROT_SENSOR         0x15
#define BRCMSTB_MSG_PROT_LINUX          0x80
#define BRCMSTB_MSG_PROT_ASTRA          0x81

/* Brcmstb mbox msg status (following SCMI) */
#define BRCMSTB_MSG_SUCCESS               0
#define BRCMSTB_MSG_NOT_SUPPORTED        -1
#define BRCMSTB_MSG_INVALID_PARAMETERS   -2
#define BRCMSTB_MSG_DENIED               -3
#define BRCMSTB_MSG_NOT_FOUND            -4
#define BRCMSTB_MSG_OUT_OF_RANGE         -5
#define BRCMSTB_MSG_BUSY                 -6
#define BRCMSTB_MSG_COMMS_ERROR          -7
#define BRCMSTB_MSG_GENERIC_ERROR        -8
#define BRCMSTB_MSG_HARDWARE_ERROR       -9
#define BRCMSTB_MSG_PROTOCOL_ERROR      -10

#endif /* _BRCMSTB_MBOX_H_ */

/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#ifndef ASTRA_MSG_H
#define ASTRA_MSG_H

#include <cstdint>

/* Astra messages */
#define ASTRA_MSG_VERSION               0x00
#define ASTRA_MSG_ATTRIBUTES            0x01
#define ASTRA_MSG_NSG_ATTRIBUTES        0x02

#define ASTRA_MSG_CPU_OFF               0x10
#define ASTRA_MSG_CPU_SUSPEND           0x11
#define ASTRA_MSG_CPU_RESUME            0x12
#define ASTRA_MSG_SYSTEM_OFF            0x13
#define ASTRA_MSG_SYSTEM_SUSPEND        0x14

/* Brcmstb mbox Astra message payloads */
typedef struct astra_msg_system_suspend {
    /* Return status */
    uint32_t status;

    /* Non-secure scratch mem */
    uint32_t smem_addr_hi;
    uint32_t smem_addr_lo;
    uint32_t smem_size;
} astra_msg_system_suspend_t;

#endif /* _ASTRA_MSG_H_ */

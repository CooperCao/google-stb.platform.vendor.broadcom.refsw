/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 ***************************************************************************/

#ifndef ASTRA_TEST_MSG_H
#define ASTRA_TEST_MSG_H

typedef struct astra_test_msg_hdr {
    uint8_t  ucType;                    /* msg type */
    uint8_t  ucSeq;                     /* msg sequence number */
    uint8_t  ucUnused[2];
} astra_test_msg_hdr;

#define ASTRA_TEST_MSG_PAYLOAD(pHdr)    ((uint8_t *)pHdr + sizeof(astra_test_msg_hdr))

enum
{
    ASTRA_TEST_MSG_START = 0,           /* unused */
    ASTRA_TEST_MSG_HELLO,               /* hello msg, to astra_tapp */
    ASTRA_TEST_MSG_MEM_ALLOC,           /* mem alloc msg, to astra_tapp */
    ASTRA_TEST_MSG_MAP_PADDR,           /* map paddr msg, to astra_tapp */
    ASTRA_TEST_MSG_MAP_PADDRS,          /* map paddr msg, to astra_tapp */
    ASTRA_TEST_MSG_LAST
};

struct astra_test_msg_echo
{
    uint32_t value;
};

struct astra_test_msg_hello_cmd
{
    char greet[64];
};

struct astra_test_msg_hello_rpy
{
    char reply[64];
};

struct astra_test_msg_mem_alloc_cmd
{
    uint32_t offset;
    uint32_t size;
};

struct astra_test_msg_mem_alloc_rpy
{
    uint32_t checksum;
};

struct astra_test_msg_map_paddr_cmd
{
    uint32_t paddr;
    uint32_t size;
    uint32_t flags;
};

struct astra_test_msg_map_paddr_rpy
{
    uint32_t checksum;
};

struct astra_test_msg_map_paddrs_cmd
{
    uint32_t count;
    uint32_t paddrs[32];
    uint32_t sizes[32];
    uint32_t flags[32];
};

struct astra_test_msg_map_paddrs_rpy
{
    uint32_t checksum;
};

#endif /* ASTRA_TEST_MSG_H */

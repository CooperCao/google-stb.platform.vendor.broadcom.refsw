/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#ifndef TZIOC_TEST_MSG_H
#define TZIOC_TEST_MSG_H

/* echo uses invalid client id of 255 */
#define TZIOC_CLIENT_ID_ECHO            255

/* hello string (greet/reply) max */
#define TZIOC_TEST_MSG_HELLO_MAX        64

enum
{
    TZIOC_TEST_MSG_START = 0,           /* unused */
    TZIOC_TEST_MSG_ECHO,                /* echo msg, to echo client */
    TZIOC_TEST_MSG_HELLO,               /* hello msg, to tzioc_tapp */
    TZIOC_TEST_MSG_MEM_ALLOC,           /* mem alloc msg, to tzioc_tapp */
    TZIOC_TEST_MSG_MAP_PADDR,           /* map paddr msg, to tzioc_tapp */
    TZIOC_TEST_MSG_MAP_PADDRS,          /* map paddr msg, to tzioc_tapp */
    TZIOC_TEST_MSG_LAST
};

struct tzioc_test_msg_echo
{
    uint32_t value;
};

struct tzioc_test_msg_hello_cmd
{
    char greet[TZIOC_TEST_MSG_HELLO_MAX];
};

struct tzioc_test_msg_hello_rpy
{
    char reply[TZIOC_TEST_MSG_HELLO_MAX];
};

struct tzioc_test_msg_mem_alloc_cmd
{
    uint32_t offset;
    uint32_t size;
};

struct tzioc_test_msg_mem_alloc_rpy
{
    uint32_t checksum;
};

struct tzioc_test_msg_map_paddr_cmd
{
    uintptr_t paddr;
    uint32_t size;
    uint32_t flags;
};

struct tzioc_test_msg_map_paddr_rpy
{
    uint32_t checksum;
};

struct tzioc_test_msg_map_paddrs_cmd
{
    uint32_t count;
    uintptr_t paddrs[32];
    uint32_t sizes[32];
    uint32_t flags[32];
};

struct tzioc_test_msg_map_paddrs_rpy
{
    uint32_t checksum;
};

#endif /* TZIOC_TEST_MSG_H */

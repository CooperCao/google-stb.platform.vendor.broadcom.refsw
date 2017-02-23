/******************************************************************************
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
 *****************************************************************************/

#ifndef UAPPD_MSG_H
#define UAPPD_MSG_H

/* uappd uses fixed client id of 1 */
#ifndef TZIOC_CLIENT_ID_UAPPD
#define TZIOC_CLIENT_ID_UAPPD           1
#endif

#define UAPPD_NAME_LEN_MAX              32
#define UAPPD_PATH_LEN_MAX              128

enum
{
    UAPPD_MSG_START = 0,                /* unused */

    /* cmd/rpy initiated by client */
    UAPPD_MSG_UAPP_START,               /* user app start */
    UAPPD_MSG_UAPP_STOP,                /* user app stop */
    UAPPD_MSG_UAPP_GETID,               /* user app get id */

    /* ntf/ack initiated by uappd */
    UAPPD_MSG_UAPP_EXIT,

    /* file operations */
    UAPPD_MSG_FILE_OPEN,
    UAPPD_MSG_FILE_CLOSE,
    UAPPD_MSG_FILE_WRITE,
    UAPPD_MSG_FILE_READ,

    /* cmd/rpy initiated by Client */
    UAPPD_MSG_UAPP_COREDUMP,

    UAPPD_MSG_LAST
};

struct uappd_msg_uapp_start_cmd
{
    uintptr_t cookie;
    char name[UAPPD_NAME_LEN_MAX];
    char exec[UAPPD_PATH_LEN_MAX];
    uint32_t shared;
};

struct uappd_msg_uapp_start_rpy
{
    uintptr_t cookie;
    char name[UAPPD_NAME_LEN_MAX];
    uint32_t retVal;
};

struct uappd_msg_uapp_stop_cmd
{
    uintptr_t cookie;
    char name[UAPPD_NAME_LEN_MAX];
    uint32_t reserved;
};

struct uappd_msg_uapp_stop_rpy
{
    uintptr_t cookie;
    char name[UAPPD_NAME_LEN_MAX];
    uint32_t retVal;
};

struct uappd_msg_uapp_getid_cmd
{
    uintptr_t cookie;
    char name[UAPPD_NAME_LEN_MAX];
    uint32_t reserved[2];
};

struct uappd_msg_uapp_getid_rpy
{
    uintptr_t cookie;
    char name[UAPPD_NAME_LEN_MAX];
    uint32_t retVal;
    uint32_t id;
};

struct uappd_msg_uapp_exit_nfy
{
    uintptr_t cookie;
    char name[UAPPD_NAME_LEN_MAX];
};

struct uappd_msg_file_open_cmd
{
    uintptr_t cookie;
    char path[UAPPD_PATH_LEN_MAX];
    uint32_t flags;
};

struct uappd_msg_file_open_rpy
{
    uintptr_t cookie;
    char path[UAPPD_PATH_LEN_MAX];
    uint32_t retVal;
};

struct uappd_msg_file_close_cmd
{
    uintptr_t cookie;
    char path[UAPPD_PATH_LEN_MAX];
    uint32_t reserved;
};

struct uappd_msg_file_close_rpy
{
    uintptr_t cookie;
    char path[UAPPD_PATH_LEN_MAX];
    uint32_t retVal;
};

struct uappd_msg_file_write_cmd
{
    uintptr_t cookie;
    char path[UAPPD_PATH_LEN_MAX];
    uintptr_t paddr;
    uint32_t bytes;
};

struct uappd_msg_file_write_rpy
{
    uintptr_t cookie;
    char path[UAPPD_PATH_LEN_MAX];
    uint32_t retVal;
};

struct uappd_msg_file_read_cmd
{
    uintptr_t cookie;
    char path[UAPPD_PATH_LEN_MAX];
    uintptr_t paddr;
    uint32_t bytes;
};

struct uappd_msg_file_read_rpy
{
    uintptr_t cookie;
    char path[UAPPD_PATH_LEN_MAX];
    uint32_t retVal;
};

struct uappd_msg_uapp_coredump_cmd
{
    uintptr_t cookie;
    char name[UAPPD_NAME_LEN_MAX];
    uintptr_t paddr;
    uint32_t bytes;
};

struct uappd_msg_uapp_coredump_rpy
{
    uintptr_t cookie;
    char name[UAPPD_NAME_LEN_MAX];
    uint32_t retVal;
};

#endif /* UAPPD_MSG_H */

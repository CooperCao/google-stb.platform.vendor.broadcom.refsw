/***************************************************************************
 * Copyright (c)2016 Broadcom. All rights reserved.
 *
 * Unless you and Broadcom execute a separate written software license agreement
 * governing use of this software, this software is licensed to you under the
 * terms of the GNU General Public License version 2 (GPLv2).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation (the "GPL").
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License version 2 (GPLv2) for more details.
 ***************************************************************************/

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

    UAPPD_MSG_LAST
};

struct uappd_msg_uapp_start_cmd
{
    uint32_t cookie;
    char name[UAPPD_NAME_LEN_MAX];
    char exec[UAPPD_PATH_LEN_MAX];
    uint32_t shared;
};

struct uappd_msg_uapp_start_rpy
{
    uint32_t cookie;
    char name[UAPPD_NAME_LEN_MAX];
    int retVal;
};

struct uappd_msg_uapp_stop_cmd
{
    uint32_t cookie;
    char name[UAPPD_NAME_LEN_MAX];
    int reserved;
};

struct uappd_msg_uapp_stop_rpy
{
    uint32_t cookie;
    char name[UAPPD_NAME_LEN_MAX];
    int retVal;
};

struct uappd_msg_uapp_getid_cmd
{
    uint32_t cookie;
    char name[UAPPD_NAME_LEN_MAX];
    int reserved[2];
};

struct uappd_msg_uapp_getid_rpy
{
    uint32_t cookie;
    char name[UAPPD_NAME_LEN_MAX];
    int retVal;
    uint32_t id;
};

struct uappd_msg_uapp_exit_nfy
{
    uint32_t cookie;
    char name[UAPPD_NAME_LEN_MAX];
};

struct uappd_msg_file_open_cmd
{
    uint32_t cookie;
    char path[UAPPD_PATH_LEN_MAX];
    uint32_t flags;
};

struct uappd_msg_file_open_rpy
{
    uint32_t cookie;
    char path[UAPPD_PATH_LEN_MAX];
    int retVal;
};

struct uappd_msg_file_close_cmd
{
    uint32_t cookie;
    char path[UAPPD_PATH_LEN_MAX];
    int reserved;
};

struct uappd_msg_file_close_rpy
{
    uint32_t cookie;
    char path[UAPPD_PATH_LEN_MAX];
    int retVal;
};

struct uappd_msg_file_write_cmd
{
    uint32_t cookie;
    char path[UAPPD_PATH_LEN_MAX];
    uint32_t paddr;
    uint32_t bytes;
};

struct uappd_msg_file_write_rpy
{
    uint32_t cookie;
    char path[UAPPD_PATH_LEN_MAX];
    int retVal;
};

struct uappd_msg_file_read_cmd
{
    uint32_t cookie;
    char path[UAPPD_PATH_LEN_MAX];
    uint32_t paddr;
    uint32_t bytes;
};

struct uappd_msg_file_read_rpy
{
    uint32_t cookie;
    char path[UAPPD_PATH_LEN_MAX];
    int retVal;
};

#endif /* UAPPD_MSG_H */

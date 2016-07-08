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

#ifndef TZIOC_SYS_MSG_H
#define TZIOC_SYS_MSG_H

/* system uses fixed client id of 0 */
#ifndef TZIOC_CLIENT_ID_SYS
#define TZIOC_CLIENT_ID_SYS             0
#endif

enum {
    SYS_MSG_START = 0,                  /* unused */
    SYS_MSG_UP,                         /* system up */
    SYS_MSG_DOWN,                       /* system down */
    SYS_MSG_VUART_ON,                   /* turn on vuart */
    SYS_MSG_VUART_OFF,                  /* turn off vuart */
    SYS_MSG_TRACELOG_ON,                /* turn on tracelog */
    SYS_MSG_TRACELOG_OFF,               /* turn off tracelog */
    SYS_MSG_LAST
};

struct sys_msg_vuart_on_cmd
{
    uint32_t rxFifoPaddr;
    uint32_t rxFifoSize;

    uint32_t txFifoPaddr;
    uint32_t txFifoSize;
};

struct sys_msg_tracelog_on_cmd
{
    uint32_t tracelogBase;
    uint32_t sentinelBase;
    uint32_t sentinelSize;

    uint32_t traceBuffPaddr;
    uint32_t traceBuffSize;
};

#endif /* TZIOC_SYS_MSG_H */

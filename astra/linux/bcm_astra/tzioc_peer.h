/***************************************************************************
 * Copyright (c)2016 Broadcom
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation (the "GPL").
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License version 2 (GPLv2) for more details.
 *
 * You should have received a copy of the GNU General Public License
 * version 2 (GPLv2) along with this source code.
 ***************************************************************************/

#ifndef TZIOC_PEER_H
#define TZIOC_PEER_H

#include <linux/types.h>

#include "tzioc_common.h"

int _tzioc_peer_start(
    struct tzioc_client *pClient,
    const char *pPeerName,
    const char *pPeerExec,
    bool bPeerShared);

int _tzioc_peer_stop(
    struct tzioc_client *pClient,
    const char *pPeerName);

int _tzioc_peer_getid(
    struct tzioc_client *pClient,
    const char *pPeerName);

#endif /* TZIOC_PEER_H */

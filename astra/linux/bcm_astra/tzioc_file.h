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

#ifndef TZIOC_FILE_H
#define TZIOC_FILE_H

#include <linux/types.h>

#include "tzioc_common.h"

int _tzioc_file_open(
    struct tzioc_client *pClient,
    const char *pPath,
    uint32_t ulFlags);

int _tzioc_file_close(
    struct tzioc_client *pClient,
    const char *pPath);

int _tzioc_file_write(
    struct tzioc_client *pClient,
    const char *pPath,
    uint32_t ulPaddr,
    uint32_t ulBytes);

int _tzioc_file_read(
    struct tzioc_client *pClient,
    const char *pPath,
    uint32_t ulPaddr,
    uint32_t ulBytes);

#endif /* TZIOC_FILE_H */

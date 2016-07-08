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

#ifndef TZIOC_IOCTL_H
#define TZIOC_IOCTL_H

#include <linux/fs.h>
#include "tzioc_ioctls.h"

/* ioctl handler type */
typedef int (*tzioc_ioctl_handler)(struct file *file, void *arg);

/* ioctl module */
struct tzioc_ioctl_module {
    /* ioctl handlers */
    tzioc_ioctl_handler handlers[TZIOC_IOCTL_LAST - TZIOC_IOCTL_FIRST];
};

int __init _tzioc_ioctl_module_init(void);
int _tzioc_ioctl_module_deinit(void);

int _tzioc_ioctl_do_ioctl(struct file *file, uint32_t cmd, void *arg);

#endif /* TZIOC_IOCTL_H */

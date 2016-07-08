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

#ifndef TZIOC_COMMON_H
#define TZIOC_COMMON_H

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

#if defined(__linux__)

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/printk.h>

#ifndef LOGI
#define LOGD(format, ...) pr_devel("%s: " format "\n", __FUNCTION__, ## __VA_ARGS__)
#define LOGW(format, ...) pr_warn ("%s: " format "\n", __FUNCTION__, ## __VA_ARGS__)
#define LOGE(format, ...) pr_err  ("%s: " format "\n", __FUNCTION__, ## __VA_ARGS__)
#define LOGI(format, ...) pr_info ("%s: " format "\n", __FUNCTION__, ## __VA_ARGS__)
#endif

#elif defined(__TZOS__)

#include <stdint.h>
#include <stdbool.h>
#include <bits/errno.h>
#include "lib_types.h"
#include "lib_string.h"
#include "lib_printf.h"

#ifndef LOGI
#define LOGD(format, ...) printf("%s: " format "\n", __FUNCTION__, ## __VA_ARGS__)
#define LOGW(format, ...) printf("%s: " format "\n", __FUNCTION__, ## __VA_ARGS__)
#define LOGE(format, ...) printf("%s: " format "\n", __FUNCTION__, ## __VA_ARGS__)
#define LOGI(format, ...) printf("%s: " format "\n", __FUNCTION__, ## __VA_ARGS__)
#endif

#else
#error "Unknown operating system!"
#endif

#include "tzioc_common_msg.h"
#include "tzioc_common_mem.h"
#include "tzioc_common_client.h"

/* shared memory */
struct tzioc_shared_mem {
    uint32_t ulMagic;

    /* msg rings */
    struct tzioc_ring_buf t2nRing;  /* TZOS to NWOS msg ring */
    struct tzioc_ring_buf n2tRing;  /* NWOS to TZOS msg ring */

    /* data to be shared */
};

#ifdef __cplusplus
extern "C" {
#endif

/* To be implemented by the specific OS */
extern uint32_t _tzioc_offset2addr(uint32_t ulOffset);
extern uint32_t _tzioc_addr2offset(uint32_t ulAddr);

#ifdef __cplusplus
}
#endif

#endif /* TZIOC_H */

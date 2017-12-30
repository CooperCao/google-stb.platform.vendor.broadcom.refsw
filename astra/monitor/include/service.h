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

#ifndef _SERVICE_H_
#define _SERVICE_H_

#define SERVICE_DESC_SIZE_LOG2          5
#define SERVICE_DESC_SIZE               (1 << SERVICE_DESC_SIZE_LOG2)
#define SERVICE_DESC_INIT_OFFSET        16
#define SERVICE_DESC_PROC_OFFSET        24

#ifndef __ASSEMBLY__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <cassert.h>

/* Prototype for service init function */
typedef void (*service_init_t)(void);

/* Prototype for service processing function */
typedef void (*service_proc_t)(
    uint32_t fid,
    uint64_t *ctx,
    uint64_t flags);

/* Service descriptor structure */
typedef struct service_desc {
    const char *name;
    uint8_t oen_start;
    uint8_t oen_end;
    service_init_t init;
    service_proc_t proc;
} service_desc_t;

/*
 * Macro to declare a service
 * This macro puts the declared service descriptor into the read-only
 * .services.descs section, where it is put together as an array.
 */
#define DECLARE_SERVICE(_name, _oen_start, _oen_end, _init, _proc) \
static const service_desc_t _service_desc_ ## _name \
__attribute__((__section__(".service_descs"))) \
__attribute__((__used__)) = { \
    .name = #_name, \
    .oen_start = _oen_start, \
    .oen_end = _oen_end, \
    .init = _init, \
    .proc = _proc }

/*
 * Compile time assertions to make sure assembly code and C compiler
 * agree on the size and offsets related to service_desc_t.
 */
CASSERT((SERVICE_DESC_SIZE == sizeof(service_desc_t)), \
        assert_service_desc_size_mismatch);
CASSERT((SERVICE_DESC_INIT_OFFSET == offsetof(service_desc_t, init)), \
        assert_service_desc_init_offset_mismatch);
CASSERT((SERVICE_DESC_PROC_OFFSET == offsetof(service_desc_t, proc)), \
        assert_service_desc_proc_offset_mismatch);

/* Function prototypes */
void service_init(void);


/*
 * Common defines that may be used for implementation of each service,
 * if the service consists of multiple modules, or if the service uses
 * a table to look up do functions.
 */

/* Service module check function prototype */
typedef bool (*service_mod_check_t)(uint32_t fid);

/* Service module init function prototype */
typedef void (*service_mod_init_t)(void);

/* Service module processing function prototype */
typedef void (*service_mod_proc_t)(
    uint32_t fid,
    uint64_t *ctx,
    uint64_t flags);

/* Service module descriptor structure */
typedef struct service_mod_desc {
    uint8_t call_count;
    service_mod_check_t check;
    service_mod_init_t init;
    service_mod_proc_t proc;
} service_mod_desc_t;

/* Service do function prototype */
typedef void (*service_do_func_t)(uint64_t *ctx);

#endif /* __ASSEMBLY__ */

#endif /* _SERVICE_H_ */

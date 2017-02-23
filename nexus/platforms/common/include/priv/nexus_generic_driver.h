/***************************************************************************
* Copyright (C) 2011-2016 2xxx Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
*  USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
*  ANY LIMITED REMEDY.
*
***************************************************************************/
#ifndef NEXUS_GENERIC_DRIVER_H_
#define NEXUS_GENERIC_DRIVER_H_

/* used for custom linuxkernel drivers */

#define NEXUS_DRIVER_MAJOR 33

struct nexus_generic_driver_init_settings
{
    struct {
        unsigned offset; /* physical address */
        unsigned size;
    } region[16];

    unsigned max_dcache_line_size; /* to ensure nexus heap allocations are cache coherent */
};

int  nexus_generic_driver_init(const struct nexus_generic_driver_init_settings *settings);
void nexus_generic_driver_uninit(void);

int  nexus_generic_driver_open(
    unsigned module, /* driver can open multiple modules, each given a unique module id */
    void **context, /* [out] storage for generic driver context */
    unsigned process_id, /* does not have to be actual OS process_id. will correspond to terminate_process(process_id). */
    const char *process_name, /* optional name for process */
    bool trusted /* generally, true for root, false for non-root */
    );
void nexus_generic_driver_close(unsigned module, void *context, bool abnormal_termination);
int  nexus_generic_driver_validate_mmap(unsigned module, void *context, uint64_t offset, unsigned size);
int  nexus_generic_driver_ioctl(unsigned module, void *context, unsigned int cmd, unsigned long arg, bool compat);
void nexus_generic_driver_read_register(uint32_t addr, uint32_t *p_value);
void nexus_generic_driver_write_register(uint32_t addr, uint32_t value);

/* if embedding nexus_driver.c into another module, define NEXUS_EXTERNAL_MODULE_INIT and use these entry/exit points. */
int nexus_init_module(void);
void nexus_cleanup_module(void);

#endif

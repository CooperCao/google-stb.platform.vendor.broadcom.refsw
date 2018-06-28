/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/
#ifndef CPU_H
#define CPU_H

#include "bmon_cpu_defines.h"

#define CPU_PLUGIN_VERSION      "0.2"
#define CPU_PLUGIN_NAME         "cpu"
#define CPU_PLUGIN_DESCRIPTION  "Gathers information about each CPU in the system"

typedef enum
{
    /* conservative ondemand userspace powersave performance */
    DVFS_GOVERNOR_CONSERVATIVE = 1,
    DVFS_GOVERNOR_ONDEMAND,
    DVFS_GOVERNOR_USERSPACE,
    DVFS_GOVERNOR_POWERSAVE,
    DVFS_GOVERNOR_PERFORMANCE,
    DVFS_GOVERNOR_MAX
} DVFS_GOVERNOR_TYPES;

typedef struct bmon_cpu_t
{
    unsigned char        total_cpus;
    unsigned char        num_active_cpus;
    unsigned long long   uptime_msec;                        /* time since boot */
    struct
    {
        unsigned char   active;                              /* 1=on, 0 =off */
        unsigned int    idle;                                /* jiffies - twiddling thumbs */
        unsigned int    user;                                /* jiffies - normal processes executing in user mode */
        unsigned int    system;                              /* jiffies - processes executing in kernel mode */
        unsigned int    nice;                                /* jiffies - niced processes executing in user mode */
        unsigned int    iowait;                              /* jiffies - waiting for I/O to complete */
        unsigned int    irq;                                 /* jiffies - servicing interrupts */
        unsigned int    softirq;                             /* jiffies - servicing softirqs */
        unsigned int    steal;                               /* jiffies - ticks spent executing other virtual hosts */
    } cpu[BMON_CPU_MAX_NUM];
    unsigned int   frequency_mhz[BMON_CPU_MAX_NUM];         /* cpu freq */
} bmon_cpu_t;

int cpu_get_data(const char * filter, char * json_output, unsigned int json_output_size);

#endif /* ifndef CPU_H */
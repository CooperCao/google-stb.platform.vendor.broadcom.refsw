/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#ifndef BCMDRIVER_H
#define BCMDRIVER_H
#include <linux/ioctl.h>

/* update this number for any non-trivial changes in the file */
#define BCMDRIVER_VERSION  11

#define BCM_INTC_MAX_SIZE   5    /* maximum interrupt controller size
                                    in words supported by this interface */

typedef struct s_bcm_linux_dd_interrupt
{
    int     timeout;
    unsigned char waitforallints;
    uint32_t interruptmask[BCM_INTC_MAX_SIZE];
    uint32_t interruptstatus[BCM_INTC_MAX_SIZE];

} tbcm_linux_dd_interrupt;

typedef struct s_bcm_linux_dd_ticks
{
    unsigned int jiffies;
    unsigned int timerFreq;
} tbcm_linux_dd_ticks;

typedef struct s_bcm_linux_dd_atomic_update
{
    uint32_t reg; /* register offset */
    uint32_t mask; /* set 1 for bits that will be modified */
    uint32_t value; /* value only applies for bits where mask is 1 */
} t_bcm_linux_dd_atomic_update;

/* all updates will happen in one atomic block with interrupts disabled */
#define BCM_MULTI_ATOMIC_UPDATE_NUM 4
typedef struct s_bcm_linux_dd_multi_atomic_update
{
    struct {
        uint32_t reg; /* register offset (use 0 for unused) */
        uint32_t mask; /* set 1 for bits that will be modified */
        uint32_t value; /* value only applies for bits where mask is 1 */
    } set[BCM_MULTI_ATOMIC_UPDATE_NUM];
} t_bcm_linux_dd_multi_atomic_update;

typedef struct s_bcm_linux_rw_reserved
{
    uint32_t address;       /* Physical Address */
    uint32_t value;         /* Value - out if read, in if write */
} t_bcm_linux_rw_reserved;

typedef struct s_bcm_linux_rw_core_register
{
    uint32_t offset;        /* Offset to core register from core base */
    uint32_t value;         /* Value - out if read, in if write */
} t_bcm_linux_rw_core_register;

typedef struct s_bcm_linux_mem_addr_range
{
    unsigned long address; /* Virtual memory address */
    unsigned length;       /* Length of range (in bytes) */
} t_bcm_linux_mem_addr_range;

#define BRCM_IOCTL_WAIT_FOR_INTERRUPTS      _IOWR(101, 2, tbcm_linux_dd_interrupt ) /* use the tbcm_linux_dd_interrupt structure with this IOCTL */
#define BRCM_IOCTL_DEBUG_LEVEL              _IO(101,1)
#define BRCM_IOCTL_CHANGE_REQUEST           _IO(101,3)
#define BRCM_IOCTL_INT_RESET                _IO(101,4)
/* 1015 has been deprecated. The number can be reused later. */
#define BRCM_IOCTL_GET_TICKS                _IOR(101,6, tbcm_linux_dd_ticks) /* use the tbcm_linux_dd_ticks structure with this IOCTL */
#define BRCM_IOCTL_GET_DCACHE_LINE_SIZE     _IOR(101,7, unsigned)
#define BRCM_IOCTL_ATOMIC_UPDATE            _IOW(101,8, t_bcm_linux_dd_atomic_update) /* use the t_bcm_linux_dd_atomic_update structure with this IOCTL */
#define BRCM_IOCTL_READ_RESERVED            _IOR(101,9, t_bcm_linux_rw_reserved) /* use the t_bcm_linux_rw_reserved structure with this IOCTL */
#define BRCM_IOCTL_WRITE_RESERVED           _IOW(101,20, t_bcm_linux_rw_reserved) /* use the t_bcm_linux_rw_reserved structure with this IOCTL */
#define BRCM_IOCTL_READ_CORE_REGISTER       _IOR(101,21, t_bcm_linux_rw_core_register)  /* use the t_bcm_linux_rw_core_register structure with this IOCTL */
#define BRCM_IOCTL_WRITE_CORE_REGISTER      _IOW(101,22, t_bcm_linux_rw_core_register)  /* use the t_bcm_linux_rw_core_register structure with this IOCTL */
#define BRCM_IOCTL_READ_CMT_CONTROL         _IOR(101,23, uint32_t) /* pass a uint32_t pointer that will be filled with the value of the register */
#define BRCM_IOCTL_WRITE_CMT_CONTROL        _IOW(101,24, uint32_t) /* pass a uint32_t value to be written to the register */
#define BRCM_IOCTL_GET_DEBUG                _IO(101,25)
#define BRCM_IOCTL_MULTI_ATOMIC_UPDATE      _IOR(101,26, t_bcm_linux_dd_multi_atomic_update) /* use t_bcm_linux_dd_multi_atomic_update with this IOCTL */

#define BCM_MAX_HEAPS 16
typedef struct t_bcm_linux_mmap_access
{
    unsigned pid; /* process id of client */
    struct {
        uint64_t offset; /* set all to zero to revoke access */
        unsigned size;
    } heap[BCM_MAX_HEAPS];
} t_bcm_linux_mmap_access;
#define BRCM_IOCTL_SET_MMAP_ACCESS          _IOW(101, 27, t_bcm_linux_mmap_access) /* use t_bcm_linux_mmap_access with this IOCTL */

typedef struct t_bcm_linux_write_cp0_25
{
    unsigned    select;
    unsigned    value;
} t_bcm_linux_write_cp0_25;
#define BRCM_IOCTL_WRITE_CP0_25             _IOW(101, 28,  t_bcm_linux_write_cp0_25) /* use t_bcm_linux_write_cp0_25 with this IOCTL */

#define BRCM_IOCTL_FLUSH_DCACHE_RANGE       _IOW(101, 29, t_bcm_linux_mem_addr_range) /* use t_bcm_linux_mem_addr_range with this IOCTL */

typedef struct s_bcm_linux_pm_mem_exclude
{
    uint64_t addr;
    uint64_t len;
} t_bcm_linux_pm_mem_exclude;
#define BRCM_IOCTL_PM_MEM_EXCLUDE           _IOW(101, 30, t_bcm_linux_pm_mem_exclude)



/*
 *  The following definitions and structures are restricted to the kernel build
 *  User app's include this file but they don't need the folowing definitions
 */
#ifdef __KERNEL__
/* utility function */
void brcm_get_dev_info(struct inode *inode, unsigned int *devnum, unsigned int *devmode);
#endif

/**
CMA allocation
**/
struct bcmdriver_cma_getmem {
    unsigned cmaIndex;
    unsigned alignment;
    uint64_t size;

    /* OUT */
    uint64_t addr;
};
#define BRCM_IOCTL_CMA_GETMEM       _IOWR(101, 31, struct bcmdriver_cma_getmem)

struct bcmdriver_cma_putmem {
    unsigned cmaIndex;
    uint64_t addr;
    uint64_t size;
};
#define BRCM_IOCTL_CMA_PUTMEM       _IOW(101, 32, struct bcmdriver_cma_putmem)

struct bcmdriver_cma_numregions {
    unsigned cmaIndex;

    /* OUT */
    unsigned num;
};
#define BRCM_IOCTL_CMA_NUMREGIONS   _IOWR(101, 33, struct bcmdriver_cma_numregions)

struct bcmdriver_cma_getregion {
    unsigned cmaIndex;
    unsigned region;

    /* OUT */
    unsigned memc;
    uint64_t addr;
    uint64_t size;
};
#define BRCM_IOCTL_CMA_GETREGIONINFO _IOWR(101, 34, struct bcmdriver_cma_getregion)

struct bcmdriver_cma_getinfo {
    unsigned cmaIndex;

    /* OUT */
    unsigned memc;
    uint64_t addr;
    unsigned size;
};
#define BRCM_IOCTL_CMA_GETINFO       _IOWR(101, 35, struct bcmdriver_cma_getinfo)

struct bcmdriver_version {
    unsigned version;
};
#define BRCM_IOCTL_GET_VERSION  _IOWR(101, 36, struct bcmdriver_version)

struct bcmdriver_dynamic_region {
    uint64_t base;
    uint64_t length;
};
#define BRCM_IOCTL_DYNAMIC_REGION_ADD       _IOW(101, 37, struct bcmdriver_dynamic_region )
#define BRCM_IOCTL_DYNAMIC_REGION_REMOVE    _IOW(101, 38, struct bcmdriver_dynamic_region )

struct bcmdriver_memory_range {
    uint64_t addr;
    uint64_t size;
};

#define BCMDRIVER_MAX_RANGES        8
#define BCMDRIVER_MAX_MEMC          3
#define BCMDRIVER_MAX_RESERVED_RANGES   16

struct bcmdriver_memory_info {
    struct {
        struct bcmdriver_memory_range range[BCMDRIVER_MAX_RANGES];
        int count;
    } memc[BCMDRIVER_MAX_MEMC];
    /* fixed map space */
    struct {
        struct bcmdriver_memory_range range[BCMDRIVER_MAX_RANGES];
        int count;
    } lowmem;
    /* bmem carveout regions */
    struct {
        struct bcmdriver_memory_range range[BCMDRIVER_MAX_RANGES];
        int count;
    } bmem;
    /* CMA carveout regions */
    struct {
        struct bcmdriver_memory_range range[BCMDRIVER_MAX_RANGES];
        int count;
    } cma;
    /* regions that nexus cannot recommend for bmem or CMA */
    struct {
        struct bcmdriver_memory_range range[BCMDRIVER_MAX_RESERVED_RANGES];
        struct {
                char name[8];
        } range_name[BCMDRIVER_MAX_RESERVED_RANGES];
        int count;
    } reserved;
};

#define BRCM_IOCTL_GET_MEMORY_INFO _IOW(101, 39, struct bcmdriver_memory_info )

struct bcmdriver_get_vmalloc {
    struct {
        unsigned required_vmalloc; /* vmalloc space required by Nexus to map physical memory  and registers */
        uint64_t highest_installed_dram; /* (highest address of address-contiguous installed DRAM) + 1 */
        unsigned lowest_bmem_addr; /* lowest address of all bmem regions */
    } in;
    struct {
        unsigned vmalloc; /* suggested vmalloc boot parameter, in MBytes */
    } out;
};

#define BRCM_IOCTL_GET_VMALLOC _IOWR(101, 40, struct bcmdriver_get_vmalloc )

#include "b_virtual_irq_types.h"

typedef enum bcmdriver_irq_command {
    bcmdriver_irq_command_status,
    bcmdriver_irq_command_clear,
    bcmdriver_irq_command_mask,
    bcmdriver_irq_command_control,
    bcmdriver_irq_command_l1_is_virtual,
    bcmdriver_irq_command_max
} bcmdriver_irq_command;

typedef struct bcmdriver_irq_control {
    bcmdriver_irq_command command;
    union {
        struct b_virtual_irq_status status;
        struct {
            b_virtual_irq_line line;
        } clear;
        struct {
            b_virtual_irq_line line;
            bool disable;
        } mask;
        struct {
            b_virtual_irq_line line;
            bool os;
        } control;
        struct {
            unsigned irq;
            bool is_virtual;
        } l1;
    } data;
} bcmdriver_irq_control;
#define BRCM_IOCTL_IRQ_CONTROL _IOW(101, 41, struct bcmdriver_irq_control)

#define BRCM_IOCTL_IRQ_MAKE_GROUP _IOW(101, 42, struct b_virtual_irq_group)

#include "b_shared_gpio_types.h"

typedef enum bcmdriver_shared_gpio_command {
    bcmdriver_shared_gpio_command_init_banks,
    bcmdriver_shared_gpio_command_open_pin,
    bcmdriver_shared_gpio_command_close_pin,
    bcmdriver_shared_gpio_command_get_int_status,
    bcmdriver_shared_gpio_command_clear_int,
    bcmdriver_shared_gpio_command_set_int_mask,
    bcmdriver_shared_gpio_command_set_standby,
    bcmdriver_shared_gpio_command_max
} bcmdriver_shared_gpio_command;

typedef struct bcmdriver_shared_gpio_control {
    bcmdriver_shared_gpio_command command;
    union {
        b_shared_gpio_init_banks_settings init_banks;
        b_shared_gpio_int_status get_int_status;
        struct {
            b_shared_gpio_pin_desc pin;
            b_shared_gpio_irq_type irq_type;
        } open_pin;
        struct {
            b_shared_gpio_pin_desc pin;
        } close_pin;
        struct {
            b_shared_gpio_pin_desc pin;
        } clear_int;
        struct {
            b_shared_gpio_pin_desc pin;
            bool disable;
        } set_int_mask;
        struct {
            b_shared_gpio_pin_desc pin;
            bool enable;
        } set_standby;
    } data;
} bcmdriver_shared_gpio_control;
#define BRCM_IOCTL_SHARED_GPIO_CONTROL _IOW(101, 43, struct bcmdriver_shared_gpio_control)

typedef struct bcmdriver_os_config {
    bool virtual_irq_supported;
    bool shared_gpio_supported;
    bool os_64bit;
} bcmdriver_os_config;
#define BRCM_IOCTL_GET_OS_CONFIG _IOW(101, 44, struct bcmdriver_os_config)

typedef struct bcmdriver_chip_info {
    uint64_t bchp_physical_offset;
} bcmdriver_chip_info;
#define BRCM_IOCTL_SET_CHIP_INFO _IOW(101, 45, struct bcmdriver_chip_info)

typedef struct bcm_driver_pstate_info {
    unsigned index;
    unsigned pstate;
    unsigned num_writes;
    unsigned command[9]; /* 9 32byte commands */
} bcm_driver_pstate_info;

#define BRCM_IOCTL_GET_PSTATE_INFO _IOWR(101, 46, struct bcm_driver_pstate_info)
#define BRCM_IOCTL_SET_PSTATE_INFO _IOW(101, 47, struct bcm_driver_pstate_info)

typedef struct bcm_driver_avs_data {
    unsigned index;
    unsigned value;
} bcm_driver_avs_data;
#define BRCM_IOCTL_GET_AVS_DATA    _IOW(101, 48, struct bcm_driver_avs_data)
#endif /* BCMDRIVER_H */

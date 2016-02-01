/***************************************************************************
 *     Copyright (c) 2002-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 * User mode driver with 128 bit interrupt support.
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#ifndef BCMDRIVER_H
#define BCMDRIVER_H
#include <linux/ioctl.h>

/* update this number for any non-trivial changes in the file */
#define BCMDRIVER_VERSION  4

#define BCM_INTC_MAX_SIZE   5    /* maximum interrupt controller size
                                    in words supported by this interface */

typedef struct s_bcm_linux_dd_interrupt
{
    int     timeout;
    unsigned char waitforallints;
    unsigned long interruptmask[BCM_INTC_MAX_SIZE];
    unsigned long interruptstatus[BCM_INTC_MAX_SIZE];

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
    uintptr_t address;     /* Virtual memory address */
    uint32_t length;       /* Length of range (in bytes) */

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
        unsigned offset; /* set all to zero to revoke access */
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
    size_t len;
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
    uint64_t size;
    unsigned alignment;

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
    size_t length;
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


#endif /* BCMDRIVER_H */


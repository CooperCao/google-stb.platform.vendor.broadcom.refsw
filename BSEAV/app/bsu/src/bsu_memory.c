/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
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
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#include <stdio.h>
typedef unsigned int uintptr_t;
#include "lib_malloc.h"

typedef enum { memnode_free = 0, memnode_alloc } memnode_status_t;

typedef struct memnode_s {
    unsigned int seal;
    struct memnode_s *next;		/* pointer to next node */
    unsigned int length;		/* length of the entire data section */
    memnode_status_t status;		/* alloc/free status */
    unsigned char *data;		/* points to actual user data */
    void *memnodeptr;			/* memnode back pointer (see comments) */
} memnode_t;

struct mempool_s {
    memnode_t *root;			/* pointer to root node */
    unsigned char *base;		/* base of memory region */
    unsigned int length;		/* size of memory region */
};

mempool_t kmempool;     /* default pool */

void bsu_mem_test(unsigned int *start_addr, unsigned int *end_addr)
{
    volatile unsigned int *addr;

    printf("start=%p, end=%p\n", (void *)start_addr, (void *)end_addr);
    addr = start_addr;
    while (addr < end_addr) {
        *addr = (unsigned int)addr;
        if (((unsigned int)addr % 0x1000000) == 0) printf("writing to addr=%p\n", (void *)addr);
        addr++;
    }
    printf("writing done\n");

    addr = start_addr;
    while (addr < end_addr) {
        if (((unsigned int)addr % 0x1000000) == 0) printf("reading from addr=%p\n", (void *)addr);
        if ((unsigned int)*addr != (unsigned int)addr) {
            printf("miscompare at %p:  0x%08x\n", (void *)addr, *addr);
            /*break;*/
        }
        addr++;
    }
    printf("reading done\n");
}

void bsu_heap_init(unsigned char *addr, unsigned int size)
{
    KMEMINIT(addr, size);
}

#ifdef USE_BSU_MALLOC_NAME
void *bsu_malloc(size_t size)
#else
void *malloc(size_t size)
#endif
{
    return KMALLOC(size, 0);
}


#ifdef USE_BSU_MALLOC_NAME
void bsu_free(void *ptr)
#else
void free(void *ptr)
#endif
{
    return KFREE(ptr);
}

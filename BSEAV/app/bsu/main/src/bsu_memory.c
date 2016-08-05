/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description:
 *
 ***************************************************************************/
#include <stdio.h>
typedef unsigned int uintptr_t;
#include "lib_malloc.h"

typedef enum { memnode_free = 0, memnode_alloc } memnode_status_t;

typedef struct memnode_s {
    unsigned int seal;
    struct memnode_s *next;     /* pointer to next node */
    unsigned int length;        /* length of the entire data section */
    memnode_status_t status;        /* alloc/free status */
    unsigned char *data;        /* points to actual user data */
    void *memnodeptr;           /* memnode back pointer (see comments) */
} memnode_t;

struct mempool_s {
    memnode_t *root;            /* pointer to root node */
    unsigned char *base;        /* base of memory region */
    unsigned int length;        /* size of memory region */
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

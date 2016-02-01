/***************************************************************************
 *     Copyright (c) 2009, Broadcom Corporation
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

/* 
 * this header files is included from both host (TP0) and decodeer (TP1) sides
 *
 * It provides basic IPC primitives and defines top level data structure */
#ifndef B_KERNEL_IPC_H
#define B_KERNEL_IPC_H

#define     B_KERNEL_DCACHE_LINE_SIZE                            64

extern const unsigned B_KernelLock_ID;

#define B_KERNEL_BOOT_FLAGS_IPC_TEST    (1<<0)

typedef struct B_Kernel_HeapInfo {
    unsigned sharedMemorySize; /* number of bytes reserved for the shared memory, including this header */
    unsigned messageQueueLength;
    unsigned flags;
    uint8_t pad[B_KERNEL_DCACHE_LINE_SIZE-(sizeof(unsigned)+sizeof(unsigned)+sizeof(unsigned))]; /* allign size of the structure to the L1 cache line */
} B_Kernel_HeapInfo;

/* layout of the heap passed from the host to the decoder */
/*
 * +===================+ <- HEAP_BASE (stored in the TP1_BOOT_HEAP_ADDR and converted to the KSEG0 
 * | B_Kernel_HeapInfo |
 * +-------------------+ <- HEAP_BASE + sizeof(B_Kernel_HeapInfo)
 * |   shared data     |
 * |   status          |
 * |   message queues  |
 * |   mail boxes      |
 * |   etc             |
 * +-------------------+ <- HEAP_BASE + B_Kernel_HeapInfo.sharedMemorySize;
 * |   Private heap    |
 * |  managed by TP1   |
 * |      using        |
 * |  malloc and free  |
 * +-------------------+ <- HEAP_BASE + (HEAP_SIZE - STACK_SIZE)
 * |      stack        |
 * +===================+ <- HEAP_BASE + HEAP_SIZE
 */

/* B_KernelLock is used to synchrnonize between TP0 and TP1 using shared memory
 * All locks are intialized by the host (TP0)
 */
typedef struct B_KernelLock {
    unsigned token;
} B_KernelLock;


void B_KernelLock_Init(B_KernelLock *lock);

/* this function returns  true if it was able to acquire lock, it executes in bounded amount of time */
bool B_KernelLock_TryAcquire(B_KernelLock *lock);

/* this function releases lock, it should only be called on the succesfully acquired lock */
void B_KernelLock_Release(B_KernelLock *lock);


#endif /* B_KERNEL_IPC_H */


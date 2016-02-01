/***************************************************************************
 *	   Copyright (c) 2006-2009, Broadcom Corporation
 *	   All Rights Reserved
 *	   Confidential Property of Broadcom Corporation
 *
 *	THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *	AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *	EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: Background printf module
 *
 ***************************************************************************/

#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"

#include "bkni_print.h"

/**
   Non blocking print works as follows:
   1. Block is atomically allocated from circular buffer.
   2. vsnprinf from standard lirary is used to print message in to this block.
   3. If (2) was not preempted by another print call block is atomically
      reduced to the size actually occupied by message. Freed memory released
      back to the circular buffer.
   4. If (2) was preempted by higher priority print, block size is left at the
      original size. This result in some amount of wasted memory but allows
      concurrent printing from interrupts and tasks.
   Allocation and release are around 80 mips instructions each. Interrupts are
   never locked during allocation and release of memory. If atomic operation
   is interrupted it will return error and procedure will be retried.
   Atomic compare and swap function is used to implement atomic allocation
   and release operations. This function is coded using LL and SC mips 
   instructions.
 */

#define RING_SIZE 0x4000
#define MIN_FREE_SPACE 0x200

static char print_ring_buffer[RING_SIZE];

struct BKNI_Print_Ring_t {
    char * start;
    char * end;
    char * write;
    char * read;
    char * wrap;
    int overflow;
    BKNI_EventHandle evt;
};

#define BP_SIZE_T unsigned
#define MAX_DATA_SIZE (MIN_FREE_SPACE - sizeof(BP_SIZE_T))
struct BKNI_Print_Block_t {
    BP_SIZE_T size;
    char data[MAX_DATA_SIZE];
};

static struct BKNI_Print_Ring_t ring;

void BKNI_Print_Init(void)
{
    BERR_Code berr;
    BSTD_UNUSED(berr);

    ring.start = print_ring_buffer;
    ring.end = print_ring_buffer + RING_SIZE - MIN_FREE_SPACE;
    ring.write = print_ring_buffer;
    ring.read = print_ring_buffer;
    ring.wrap = ring.end + MIN_FREE_SPACE;
    
    berr = BKNI_CreateEvent(&ring.evt);
    BDBG_ASSERT(BERR_SUCCESS == berr);
    BKNI_ResetEvent(ring.evt);
}

/**
   retrieve read pointer pointing at the data in the ring buffer, if there is
   no data NULL is returned.
 */
static void BKNI_Print_RetrieveRead(char **read)
{
    if(ring.read < ring.write){
        *read = ring.read;
    }else if (ring.read > ring.write){
        if(ring.read == ring.wrap){
            ring.wrap = ring.end + MIN_FREE_SPACE;
            ring.read = ring.start;
            if(ring.read == ring.write){
                *read = NULL;
            }else{
                *read = ring.read;
            }
        }else{
            *read = ring.read;
        }
    }else{/*ring.read == ring.write*/
        *read = NULL;
    }
}

/**
   advance read pointer indicating that data is consumed and the ring buffer 
   can be reused for new data.
*/

static void BKNI_Print_AdvanceRead(unsigned size)
{
    char * tmp;
    tmp = ring.read + size;
    BDBG_ASSERT(tmp <= ring.wrap);
    if(ring.read < ring.write){
        if(tmp <= ring.write){
            ring.read = tmp;
        }else{
            /*circular buffer underflow on read.*/
            BDBG_ASSERT(0);
        }
    }else{
        if(tmp <= ring.wrap){
            ring.read = tmp;
        }else{
            /*attempt to read past end of buffer*/
            BDBG_ASSERT(0);
        }
    }
}

/**
 Atomically allocate block from the ring buffer. Function will return pointer
 to the block that is MIN_FREE_SPACE size or NULL if no free space 
 is available.
*/
static void BKNI_Print_GetBlock(struct BKNI_Print_Block_t ** block)
{
    void * old_write;
    void * new_write;

    do {
        old_write = ring.write;
        if(old_write >= ring.read){ /*write ahead of read or buf is empty*/
            if(old_write < ring.end){
                new_write = old_write + MIN_FREE_SPACE;
            }else{
                if((ring.read - ring.start) > MIN_FREE_SPACE){
                    new_write = ring.start + MIN_FREE_SPACE;
                }else{
                    new_write = NULL; /* no free space */
                }
            }
        }else{
            if((old_write + MIN_FREE_SPACE) < ring.read){
                new_write = old_write + MIN_FREE_SPACE;
            }else{
                new_write = NULL;   /* no free space */
            }
        }
        /* if there is room allocate block if not return NULL */
        if(NULL != new_write){
            if(1 == CAS(&ring.write, old_write, new_write)){
                if(new_write > old_write){ 
                    *block = old_write;
                    if(new_write >= ring.end){
                        ring.wrap = new_write;
                    }
                }else{/* wrap */
                    *block = (void*)ring.start;
                }
                (*block)->size = MIN_FREE_SPACE;
                break;
            }else{
                continue;
            }
        }else{
            *block = NULL;
            break;
        }
    }while(1);
}

/**
   Atomically shrink block to size if possible. If not possible block will
   be left at original size. 
*/
static void BKNI_Print_ShrinkBlock(struct BKNI_Print_Block_t * block, BP_SIZE_T size)
{
    BP_SIZE_T new_size;
    void * old_write;
    void * new_write;
    /* Align on the BP_SIZE_T plus leave room for header*/
    new_size = (size + (2 * sizeof(BP_SIZE_T))) & ~(sizeof(BP_SIZE_T) - 1);
    old_write = (char*)block + MIN_FREE_SPACE;;
    new_write = (char*)block + new_size;
    while(ring.write == old_write){
        if(1 == CAS(&ring.write, old_write, new_write)){
            block->size = new_size;
            BDBG_ASSERT(MIN_FREE_SPACE >= block->size);
            if(old_write == ring.wrap){
                if(new_write >= ring.end){
                    ring.wrap = new_write;
                }else{
                    ring.wrap = ring.end + MIN_FREE_SPACE;
                }
            }
        }else{
            continue;
        }
    }
}

int BKNI_Print_Vprintf(const char *fmt, va_list ap)
{
    int cnt = 0;
    struct BKNI_Print_Block_t * block;

    BKNI_Print_GetBlock(&block);

    if(NULL != block){
        cnt = vsnprintf(block->data, MIN_FREE_SPACE - sizeof(BP_SIZE_T), fmt, ap);
        if(cnt > (MIN_FREE_SPACE - sizeof(BP_SIZE_T))){
            cnt = MIN_FREE_SPACE - sizeof(BP_SIZE_T);
        }
        BKNI_Print_ShrinkBlock(block, (cnt + 1));
        BKNI_SetEvent(ring.evt);
    }
    return cnt;
}

int BKNI_Print_Printf(const char *fmt, ...)
{
    va_list ap;
    int cnt;
    
    va_start(ap, fmt);
    cnt = BKNI_Print_Vprintf(fmt, ap);
    va_end(ap);
    return cnt;
}

void BKNI_Print_Worker(BKNI_Print_Out Output)
{
    char *end;
    int cnt;
    struct BKNI_Print_Block_t * block;

    while(1){
        BKNI_EnterCriticalSection();
        BKNI_Print_RetrieveRead((void*)&block);
        BKNI_LeaveCriticalSection();
        if(NULL != block){
            BDBG_ASSERT(MIN_FREE_SPACE >= block->size);
            cnt = 0;
            end = block->data;
            while((0 != *end++)&&(cnt++ < MIN_FREE_SPACE)) ;
            Output(block->data, cnt);
            BKNI_EnterCriticalSection();
            BKNI_Print_AdvanceRead(block->size);
            BKNI_LeaveCriticalSection();
        }else{
            BKNI_WaitForEvent(ring.evt, BKNI_INFINITE);
        }
    }
}

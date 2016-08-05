/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2003-2016 Broadcom. All rights reserved.
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
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bdbg_fifo.h"

#if !defined(BDBG_P_LOG_SUPPORTED)
#if defined(__GNUC__) && (defined(__mips__) || defined(__arm__) || defined(__aarch64__) ||  (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 4)) && (defined(__i386__) || defined(__x86_64__)))
#define BDBG_P_LOG_SUPPORTED    1
#endif
#endif

#if BDBG_P_LOG_SUPPORTED
#ifdef BSTD_INLINE
#define BDBG_P_INLINE BSTD_INLINE
#else
#define BDBG_P_INLINE static __inline__ __attribute__((always_inline))
#endif /* BSTD_INLINE */
#else
#define BDBG_P_INLINE static
#endif

BDBG_P_INLINE void 
BDBG_P_Atomic_Set_isrsafe(BDBG_P_Atomic *a, long val) {
	a->atomic = val;
    return;
}

BDBG_P_INLINE long 
BDBG_P_Atomic_Get_isrsafe(const BDBG_P_Atomic *a) {
	return a->atomic;
}

BDBG_P_INLINE long
BDBG_P_Atomic_AddReturnOld_isrsafe(BDBG_P_Atomic *a, long val)
{
	unsigned long result;
#if BDBG_P_LOG_SUPPORTED && defined(__mips__)
	unsigned long temp;
	__asm__ __volatile__(
		"	.set	mips32					\n"
		"1:	ll	%1, %2		# BDBG_P_Atomic_AddReturnOld_isrsafe \n"
		"	addu	%0, %1, %3				\n"
		"	sc	%0, %2					\n"
		"	beqz	%0, 1b					\n"
		"	move    %0, %1				    \n"
		"	.set	mips0					\n"
		: "=&r" (result), "=&r" (temp), "=m" (a->atomic)
		: "Ir" (val), "m" (a->atomic)
		: "memory");
#elif BDBG_P_LOG_SUPPORTED && defined(__arm__) && defined(__ARM_ARCH_7A__)
	unsigned long temp;
	unsigned long temp2;
    __asm__ __volatile__ ("dmb" : : : "memory");

   __asm__ __volatile__(
        "1:     ldrex   %0, [%5]  @ BDBG_P_Atomic_AddReturn_Old_isrsafe \n"
        "       add     %1, %0, %4                          \n"
        "       strex   %2, %1, [%5]                        \n"
        "       teq     %2, #0                              \n"
        "       bne     1b                                  \n"
        : "=&r" (result), "=&r" (temp), "=&r" (temp2), "+Qo" (a->atomic)
        : "Ir" (val), "r" (&a->atomic)
        : "cc");

    __asm__ __volatile__ ("dmb" : : : "memory");
#elif BDBG_P_LOG_SUPPORTED && defined(__GNUC__)
    result = __sync_fetch_and_add(&a->atomic, val);
#else
	result = a->atomic;
	a->atomic += val;
#endif
	return result;
}


#define BDBG_P_LOG_TAG_EMPTY    0
#define BDBG_P_LOG_TAG_ALLOCATED    1
#define BDBG_P_LOG_TAG_COMPLETED    2


struct BDBG_Fifo {
    size_t element_size;
    unsigned nelements;
    bool buffer_allocated;
    bool enabled;
    BDBG_P_Atomic write_counter;
    BDBG_OBJECT(BDBG_Fifo)
    unsigned buffer[1]; /* variable size array */
};

BDBG_OBJECT_ID(BDBG_Fifo);

struct BDBG_FifoReader  {
    BDBG_OBJECT(BDBG_FifoReader)
    BDBG_Fifo_CHandle writer;
    long read_counter;
};
BDBG_OBJECT_ID(BDBG_FifoReader);



void 
BDBG_Fifo_GetDefaultCreateSettings(BDBG_Fifo_CreateSettings *createSettings)
{
    BDBG_ASSERT(createSettings);
    BKNI_Memset(createSettings, 0, sizeof(*createSettings));
    createSettings->nelements = 32;
    return;
}

#if BDBG_P_LOG_SUPPORTED  
/* acual element size should be power of 2 */
static BERR_Code
BDBG_Fifo_P_NearPow2(unsigned a, unsigned *result)
{
    unsigned n;
    unsigned i;

    for(i=0,n=1;n<a;i++,n*=2) {
        if(i>=30) {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    *result = n;
    return BERR_SUCCESS;
}
#endif

BERR_Code 
BDBG_Fifo_Create(BDBG_Fifo_Handle *pFifo, const BDBG_Fifo_CreateSettings *createSettings)
{
#if BDBG_P_LOG_SUPPORTED
    BDBG_Fifo_Handle fifo;
    BERR_Code rc;
    unsigned i;
    size_t element_size;
    unsigned nelements=0;

    BDBG_ASSERT(pFifo);
    BDBG_ASSERT(createSettings);
    if(createSettings->elementSize == 0) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if(createSettings->bufferSize == 0 && createSettings->nelements == 0) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if(createSettings->bufferSize !=0 && createSettings->buffer == NULL) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    element_size = createSettings->elementSize + 2*sizeof(BDBG_P_Atomic) - 1;
    element_size = element_size - (element_size % sizeof(BDBG_P_Atomic));

    if(createSettings->buffer) {
        unsigned nelementsLimit;
        unsigned bufferLeft = createSettings->bufferSize;
        if(bufferLeft<sizeof(*fifo)) {rc=BERR_TRACE(BERR_INVALID_PARAMETER);goto err_alloc;}
        bufferLeft -= sizeof(fifo);
        nelementsLimit = bufferLeft / element_size;
        rc = BDBG_Fifo_P_NearPow2(nelementsLimit, &nelements);
        if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_nelements; }
        if(nelements>nelementsLimit) {
            nelements /=2;
            BDBG_ASSERT(nelements<=nelementsLimit);
            if(nelements==0) {rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto err_nelements;}
        }
        fifo = createSettings->buffer;
        BDBG_OBJECT_INIT(fifo, BDBG_Fifo);
        fifo->buffer_allocated = false;
     } else {
        rc = BDBG_Fifo_P_NearPow2(createSettings->nelements, &nelements);
        if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_nelements; }
        fifo = BKNI_Malloc(sizeof(*fifo) + nelements*element_size);
        if(fifo==NULL) {rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_alloc;}
        BDBG_OBJECT_INIT(fifo, BDBG_Fifo);
        fifo->buffer_allocated = true;
    }
    BDBG_ASSERT(nelements>0);
    fifo->enabled = true;
    fifo->nelements = nelements;
    fifo->element_size = element_size;

    BDBG_P_Atomic_Set_isrsafe(&fifo->write_counter, 0);
    for(i=0;i<fifo->nelements;i++) {
        BDBG_P_Atomic *atomic;

        atomic = (void *)((uint8_t *)fifo->buffer + i*fifo->element_size);
        BDBG_P_Atomic_Set_isrsafe(atomic, BDBG_P_LOG_TAG_EMPTY);
    }
    *pFifo = fifo;
    return BERR_SUCCESS;

    /* BKNI_Free(fifo); */
err_alloc:
err_nelements:
    return rc;
#else /* BDBG_P_LOG_SUPPORTED */
    /* short circuit execution */
    BSTD_UNUSED(createSettings);
    *pFifo = NULL;
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
}

void 
BDBG_Fifo_Destroy(BDBG_Fifo_Handle fifo)
{
    BDBG_OBJECT_ASSERT(fifo, BDBG_Fifo);
    if(fifo->buffer_allocated) {
        BDBG_OBJECT_DESTROY(fifo, BDBG_Fifo);  
        BKNI_Free(fifo);
    }
    return;
}


static void *
BDBG_Fifo_P_GetElementPointer_isrsafe(BDBG_Fifo_CHandle fifo, unsigned long counter)
{
    unsigned offset;
    void *buf;

    offset = (unsigned) (counter % fifo->nelements);
    offset = offset * fifo->element_size;
    buf = (uint8_t *)fifo->buffer + offset;
    return buf;
}


void *
BDBG_Fifo_GetBuffer_isrsafe(BDBG_Fifo_Handle fifo, BDBG_Fifo_Token *token)
{
    void *buf;
#if 0
    /* we can't use BDBG_XXX since they could print, which then would call back to this function */
    BDBG_OBJECT_ASSERT(fifo, BDBG_Fifo);
    BDBG_ASSERT(token);
#endif
    if(fifo->enabled) {
        buf = BDBG_Fifo_P_GetElementPointer_isrsafe(fifo, BDBG_P_Atomic_AddReturnOld_isrsafe(&fifo->write_counter, 1));
        token->marker = buf;
        BDBG_P_Atomic_Set_isrsafe(token->marker, BDBG_P_LOG_TAG_EMPTY);
        return (uint8_t *)buf + sizeof(token->marker);
    } else {
        return NULL;
    }
}

void 
BDBG_Fifo_CommitBuffer_isrsafe(const BDBG_Fifo_Token *token)
{
    BDBG_P_Atomic_Set_isrsafe(token->marker, BDBG_P_LOG_TAG_COMPLETED);
    return;
}


BERR_Code 
BDBG_FifoReader_Create(BDBG_FifoReader_Handle *pReader, BDBG_Fifo_Handle fifo)
{
    BDBG_FifoReader_Handle reader;

    BDBG_ASSERT(pReader);
    /* BDBG_OBJECT_ASSERT(fifo, BDBG_Fifo); */

    reader = BKNI_Malloc(sizeof(*reader));
    if(!reader) {return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);}

    BDBG_OBJECT_INIT(reader, BDBG_FifoReader);
    reader->writer = fifo;
    reader->read_counter = 0;
    *pReader = reader;

    return BERR_SUCCESS;
}

void
BDBG_FifoReader_Destroy(BDBG_FifoReader_Handle fifo)
{
    BDBG_OBJECT_DESTROY(fifo, BDBG_FifoReader);
    BKNI_Free(fifo);
    return;
}

BERR_Code 
BDBG_FifoReader_Read(BDBG_FifoReader_Handle fifo, void *buffer, size_t buffer_size)
{
    size_t element_size;
    long distance;
    BDBG_P_Atomic *marker;

    BDBG_OBJECT_ASSERT(fifo, BDBG_FifoReader);
    /* BDBG_OBJECT_ASSERT(fifo->writer, BDBG_Fifo); */
    BDBG_ASSERT(buffer);
    element_size = fifo->writer->element_size - sizeof(BDBG_P_Atomic);
    if(buffer_size < element_size) { return BERR_TRACE(BERR_INVALID_PARAMETER); }
    distance = BDBG_P_Atomic_Get_isrsafe(&fifo->writer->write_counter)-fifo->read_counter;
    if(distance == 0) {
        return BERR_FIFO_NO_DATA;
    }
    if(distance < 0 || distance >= (long)fifo->writer->nelements) {
        return BERR_FIFO_OVERFLOW;
    }
    marker = BDBG_Fifo_P_GetElementPointer_isrsafe(fifo->writer, fifo->read_counter);
    if(BDBG_P_Atomic_Get_isrsafe(marker)!=BDBG_P_LOG_TAG_COMPLETED) {
        if(distance==1) {
            return BERR_FIFO_BUSY;
        } else {
            return BERR_FIFO_NO_DATA;
        }
    }
    BKNI_Memcpy(buffer, marker+1, element_size);
    /* verify pointer after copying data */
    distance = BDBG_P_Atomic_Get_isrsafe(&fifo->writer->write_counter)-fifo->read_counter;
    if(distance < 0 || distance >= (long)fifo->writer->nelements) {
        return BERR_FIFO_OVERFLOW;
    }
    fifo->read_counter++;
    return BERR_SUCCESS;
}

BERR_Code 
BDBG_FifoReader_Resync(BDBG_FifoReader_Handle fifo)
{
    long distance;
    long write_counter;

    BDBG_OBJECT_ASSERT(fifo, BDBG_FifoReader);
    /* BDBG_OBJECT_ASSERT(fifo->writer, BDBG_Fifo); */
    write_counter = BDBG_P_Atomic_Get_isrsafe(&fifo->writer->write_counter);
    distance = write_counter-fifo->read_counter;
    if(distance == 0) {
        /* do nothing */
    } else if(distance < 0 || distance >= (long)fifo->writer->nelements) {
        fifo->read_counter = write_counter;
    } else {
        BDBG_P_Atomic *marker;
        marker = BDBG_Fifo_P_GetElementPointer_isrsafe(fifo->writer, fifo->read_counter);
        if(BDBG_P_Atomic_Get_isrsafe(marker)==BDBG_P_LOG_TAG_ALLOCATED) {
            fifo->read_counter++;
        }
    }
    return BERR_SUCCESS;
}






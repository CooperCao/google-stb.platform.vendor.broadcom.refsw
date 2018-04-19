/***************************************************************************
 *  Copyright (C) 2009-2018 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description:
 *
 * Start Code Parser module for the on the fly PVR
 *
 ***************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "botf_priv.h"
#include "botf_itb_feeder.h"
#include "botf_scv_parser.h"

BDBG_MODULE(botf_itb_feeder);


#define BDBG_MSG_TRACE(x)   BDBG_MSG(x) 
#define BDBG_MSG_FEED(x)    BDBG_MSG(x)
#define BDBG_MSG_PARSE(x)   BDBG_MSG(x) 

#define B_OTF_FEEDER_CAPTURE    0

#if B_OTF_FEEDER_CAPTURE    
#include <stdio.h>
#endif

BDBG_OBJECT_ID(botf_itb_feeder);

struct botf_itb_feeder_tag {
    const BOTF_ParserPtrs *IPParserPtrs;  
    /* Fields used for svp itb entry translation */
    const uint8_t *lastbaseptr;
    bool updatebaseptr;
    bool used40bitBase;
    bool used32bitBase;
#if B_OTF_FEEDER_CAPTURE    
    struct {
        unsigned file_no;
        const uint8_t *last_scode;
        FILE *fout;
    } capture;
#endif
    BDBG_OBJECT(botf_itb_feeder)
};

botf_itb_feeder
botf_itb_feeder_create(const BOTF_ParserPtrs *IPParserPtrs)
{
    botf_itb_feeder feeder;

    feeder = BKNI_Malloc(sizeof(*feeder));
    if(!feeder) {(void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_alloc;}
    BDBG_OBJECT_INIT(feeder, botf_itb_feeder);

    feeder->IPParserPtrs = IPParserPtrs;
    feeder->updatebaseptr = false;
    feeder->used40bitBase = false;
    feeder->used32bitBase = false;
    feeder->lastbaseptr = NULL;

#if B_OTF_FEEDER_CAPTURE    
    feeder->capture.last_scode = NULL;
    feeder->capture.fout = NULL;
    feeder->capture.file_no = 0;
#endif

    return feeder;

err_alloc:
    return NULL;
}

void 
botf_itb_feeder_destroy( botf_itb_feeder feeder)
{
    BDBG_OBJECT_ASSERT(feeder, botf_itb_feeder);
    BDBG_OBJECT_DESTROY(feeder, botf_itb_feeder);

    BKNI_Free(feeder);
    return;
}

static void
b_otf_itb_feeder_print_itb(uint32_t *destn)
{
    BSTD_UNUSED(destn);
    BDBG_MSG_FEED(("ITB:%p %08x %08x %08x %08x", (void *)destn, (unsigned)destn[0], (unsigned)destn[1], (unsigned)destn[2], (unsigned)destn[3]));
    return;
}

static uint32_t *
b_otf_itb_feeder_add_baseentry(botf_itb_feeder feeder, uint32_t *destn, const uint8_t *baseptr)
{
    if(!feeder->IPParserPtrs->mem->otf->cdb40bit) {
#ifdef UNIFIED_ITB_SUPPORT
        destn[0] = (B_SCV_TYPE_BASE << 24);
#else
        destn[0] = 0x01000000;
#endif
        destn[1] = botf_mem_paddr(feeder->IPParserPtrs->mem, baseptr);
        if(!feeder->used32bitBase) {
            feeder->used32bitBase = true;
            if(feeder->used40bitBase) {
                (void)BERR_TRACE(BERR_NOT_SUPPORTED);
            }
        }
    } else {
#ifdef UNIFIED_ITB_SUPPORT
        destn[0] = (B_SCV_TYPE_BASE_40BIT << 24);
#else
#error "Not supported"
#endif
        if(baseptr >= feeder->IPParserPtrs->CdbStartPtr || baseptr <= feeder->IPParserPtrs->CdbEndPtr) {
            destn[1] = baseptr -  feeder->IPParserPtrs->CdbStartPtr;
        } else {
            (void)BERR_TRACE(BERR_NOT_SUPPORTED);
            destn[1] = 0;
        }
        if(!feeder->used40bitBase) {
            feeder->used40bitBase = true;
            if(feeder->used32bitBase) {
                (void)BERR_TRACE(BERR_NOT_SUPPORTED);
            }
        }
    }
    destn[2] = 0;
    destn[3] = 0;
    botf_mem_flush(feeder->IPParserPtrs->mem, destn, B_SCV_LEN);
    b_otf_itb_feeder_print_itb(destn);
    return destn+B_SCV_LEN/4;                
}

#if B_OTF_FEEDER_CAPTURE    
static void
b_otf_itb_feeder_check_file(botf_itb_feeder feeder)
{

    if(!feeder->capture.fout) {
        char fname[64];

        BKNI_Snprintf(fname, sizeof(fname), "scv%u.es", feeder->capture.file_no);
        BDBG_WRN(("writing to %s", fname));
        feeder->capture.fout = fopen(fname, "wb");
        feeder->capture.file_no++;
        BDBG_ASSERT(feeder->capture.fout);
    }
    return;
}

static uint8_t
b_otf_itb_feeder_fetch_byte(const uint8_t *addr)
{
    unsigned offset =  0x03&(unsigned)addr;
    const uint8_t *base = addr-offset;
    return base[3-offset];
}

static uint32_t
b_otf_itb_feeder_fetch_scode(const uint8_t *addr)
{
    return 
    (((uint32_t)b_otf_itb_feeder_fetch_byte(addr)) << 24) |
    (((uint32_t)b_otf_itb_feeder_fetch_byte(addr+1)) << 16) |
    (((uint32_t)b_otf_itb_feeder_fetch_byte(addr+2)) << 8) |
    b_otf_itb_feeder_fetch_byte(addr+3);
}


static void
b_otf_itb_feeder_scode_check(botf_itb_feeder feeder, const char *name, const uint8_t *baseptr, unsigned scode, int off )
{
    uint32_t data =  b_otf_itb_feeder_fetch_scode(baseptr+off) ;
    scode |= 0x100;
    if(data != scode) {
        BDBG_WRN(("%s: %#lx not matching scode %#x:%#x(%02x %02x %02x %02x %02x) at %#x(%#x:%d)", name, (unsigned)feeder, scode, data, b_otf_itb_feeder_fetch_byte(baseptr+off), b_otf_itb_feeder_fetch_byte(baseptr+off+1), b_otf_itb_feeder_fetch_byte(baseptr+off+2), b_otf_itb_feeder_fetch_byte(baseptr+off+3), b_otf_itb_feeder_fetch_byte(baseptr+off+4), (unsigned)baseptr+off, (unsigned)baseptr, off ));
    } else {
        BDBG_MSG_FEED(("%s: %#lx startcode %#x at %#x(%#x:%d)", name, (unsigned)feeder, scode, (unsigned)baseptr+off,(unsigned)baseptr,off));
    }
}

static void
b_otf_itb_feeder_capture_data(botf_itb_feeder feeder, const uint8_t *start, const uint8_t *end)
{
    unsigned i=0;
    b_otf_itb_feeder_check_file(feeder);
    BDBG_ASSERT(start >= feeder->IPParserPtrs->CdbStartPtr);
    BDBG_ASSERT(end >= feeder->IPParserPtrs->CdbStartPtr);
    BDBG_ASSERT(start <= feeder->IPParserPtrs->CdbWrapAroundPtr);
    BDBG_ASSERT(end <= feeder->IPParserPtrs->CdbWrapAroundPtr);
    if(start>end) { /* WRAP */
        const uint8_t *wrap = feeder->IPParserPtrs->CdbWrapAroundPtr;
        BDBG_WRN(("handling WRAP %p..%p %p..%p", start, wrap, feeder->IPParserPtrs->CdbStartPtr, end));
        for(;start<=wrap;start++,i++) {
            fputc(b_otf_itb_feeder_fetch_byte(start), feeder->capture.fout);
        }
        start = feeder->IPParserPtrs->CdbStartPtr;
    } 
    for(;start<end;start++,i++) {
        fputc(b_otf_itb_feeder_fetch_byte(start), feeder->capture.fout);
    }
    fflush(feeder->capture.fout);
    BDBG_MSG(("b_otf_itb_feeder_capture_data: %#lx wrote %u bytes", (unsigned)feeder, i));
    return;
}

static void
b_otf_itb_feeder_capture_termination(botf_itb_feeder feeder, const uint8_t *baseptr)
{
    /* BDBG_ERR(("term")); */
    if(feeder->capture.last_scode) {
        b_otf_itb_feeder_capture_data(feeder, feeder->capture.last_scode, baseptr);
    }
    feeder->capture.last_scode = NULL;
    return;
}

static void
b_otf_itb_feeder_capture_scode(botf_itb_feeder feeder, const uint8_t *baseptr, unsigned scode, int off )
{
    const uint8_t *cur_scode;
    /* BDBG_ERR(("capture")); */
    off-=3; /* feeder points to the start code suffix */
    b_otf_itb_feeder_scode_check(feeder, "b_otf_itb_feeder_capture_scode", baseptr, scode, off);
    cur_scode = baseptr + off;
    if(feeder->capture.last_scode) {
        b_otf_itb_feeder_capture_data(feeder, feeder->capture.last_scode, cur_scode);
    }
    feeder->capture.last_scode = cur_scode;
    return;
}
#else /* B_OTF_FEEDER_CAPTURE */
#define b_otf_itb_feeder_capture_scode(feeder,baseptr,scode,off)
#define b_otf_itb_feeder_capture_termination(feeder, baseptr)
#define b_otf_itb_feeder_scode_check(feeder, name, baseptr, scode, off)
#endif

void 
botf_itb_feeder_checkpoint(botf_itb_feeder feeder) 
{
    BSTD_UNUSED(feeder);
#if B_OTF_FEEDER_CAPTURE 
    if(feeder->capture.fout) {
        fclose(feeder->capture.fout);
        feeder->capture.fout = NULL;
    }
#endif
    return;
}



/* This routine copies the source svp itb entry to destn non-svp itb entry 
 * Returns the new destination pointer. 
 * This routine may write into multiple ITB entries, to avoid memory corruption,
 * the caller should make sure that at least 8 ITB entries are available from dst.
 */
void *
botf_itb_feeder_copy(botf_itb_feeder feeder, void *dst, const void *src_)
{
    uint32_t word0;
    const uint8_t *baseptr;
    int offset;	
    uint32_t *destn = dst;
    const uint32_t *src = src_;

    BDBG_ASSERT((unsigned long)dst%4==0);
    BDBG_ASSERT((unsigned long)src%4==0);

    word0 = src[0];
    if ( B_SCV_ISSPLIT(word0) && (B_SCV_SPLITINDX(word0) == 1)) {
        /* Ignore second split entry */
        return destn;
    } 
    botf_mem_flush(feeder->IPParserPtrs->mem, destn, B_SCV_LEN);
    switch(B_SCV_TYPE(word0)) {
    case B_SCV_TYPE_SVPSCODE:
        /* Convert svp start code itb entry to non-svp itb entry */
        offset = B_SCV_SCODE_OFF(src[1]);
        baseptr = feeder->lastbaseptr;
        /* Decoder needs start code entry to point int start code suffix, but HW copied some data, so move offset back */
        if (B_SCV_ISSPLIT(word0)) {
            offset -= 7;
        } else {
            offset -= 8;
        }
        /* BDBG_ERR(("%p %p %p", baseptr, feeder->IPParserPtrs->CdbEndPtr, feeder->IPParserPtrs->CdbWrapAroundPtr)); */
        if (offset >= 0) {
            if (feeder->updatebaseptr) {
                /* Base pointer copied */
                feeder->updatebaseptr = false;
                BDBG_MSG_FEED(("baseentry SCODE PREV %p", baseptr));
                destn = b_otf_itb_feeder_add_baseentry(feeder, destn, baseptr);
            }
        } else {
            const uint8_t *baseptr_wrap;
            const uint8_t *cdb_end = feeder->IPParserPtrs->CdbWrapAroundPtr;
            /* if offset negative, resend baseptr and set offset to 0 */
            baseptr += offset;
            /* Check if we crossed the boundaries */
            if (baseptr >= cdb_end) {                    
                baseptr_wrap = feeder->IPParserPtrs->CdbStartPtr  + (baseptr - cdb_end); 
                BDBG_WRN(("FifoWrap forward: %p %p %p:%p -> %p", feeder->IPParserPtrs->CdbStartPtr, baseptr, cdb_end, feeder->IPParserPtrs->CdbEndPtr, baseptr_wrap));
                baseptr = baseptr_wrap;
            } else if (baseptr < feeder->IPParserPtrs->CdbStartPtr) {                    
                baseptr_wrap = cdb_end - (feeder->IPParserPtrs->CdbStartPtr - baseptr); 
                BDBG_WRN(("FifoWrap backward: %p %p %p:%p -> %p", feeder->IPParserPtrs->CdbStartPtr, baseptr, cdb_end, feeder->IPParserPtrs->CdbEndPtr, baseptr_wrap));
                baseptr = baseptr_wrap;
            }                 
            offset = 0;
            BDBG_MSG_FEED(("baseentry SCODE %p", baseptr));
            destn = b_otf_itb_feeder_add_baseentry(feeder, destn, baseptr);
            feeder->updatebaseptr = true;
        }
        b_otf_itb_feeder_capture_scode(feeder, baseptr, B_SCV_SCODE(src[1]), offset);
#ifdef UNIFIED_ITB_SUPPORT
        destn[0] = (word0 & 0x00f00000);
        destn[1] = (((src[1] & 0xff00) | offset) << 16) | 0x00ff;
#else
        destn[0] = ((word0 & 0x00f00000) | 0x04000000) | (src[1] & 0xff00) | offset;
        destn[1] = 0x00ff0000;
#endif
        destn[2] = 0;
        destn[3] = 0;
        botf_mem_flush(feeder->IPParserPtrs->mem, destn, B_SCV_LEN);
        b_otf_itb_feeder_print_itb(destn);
        return destn+B_SCV_LEN/4;
    case B_SCV_TYPE_BASE:
        /* Keep a copy of last base ptr, copy to destn itb later */
        feeder->lastbaseptr = botf_mem_vaddr(feeder->IPParserPtrs->mem,src[1]);
        feeder->updatebaseptr = true;
        return destn;
    case B_SCV_TYPE_BASE_40BIT:
        feeder->lastbaseptr = feeder->IPParserPtrs->CdbStartPtr+src[1];
        feeder->updatebaseptr = true;
        return destn;
#if 0
    case B_SCV_TYPE_BTP:
        BDBG_WRN(("skiping BTP entry"));
        return destn;
#endif
    case B_SCV_TYPE_TERM:
        /* Termination itb entry, always insert base address entry before and after termination entry */
        BDBG_MSG_FEED(("baseentry TERM %p", feeder->lastbaseptr));
        destn = b_otf_itb_feeder_add_baseentry(feeder, destn, feeder->lastbaseptr);
        b_otf_itb_feeder_capture_termination(feeder, feeder->lastbaseptr);
        feeder->updatebaseptr = true;
        /* keep going */
    default:
        /* not a start code or base address or termination entry, just copy it */
        B_SCV_COPY(destn, src);
        b_otf_itb_feeder_print_itb(destn);
        botf_mem_flush(feeder->IPParserPtrs->mem, destn, B_SCV_LEN);
        return destn+B_SCV_LEN/4;
    }
}

unsigned 
botf_itb_feeder_get_free(const BOTF_ParserPtrs *parserPtrs)
{
    int size;

    /* Calculate the available size in ITB */
    if (parserPtrs->ItbReadPtr > parserPtrs->ItbValidPtr) {
        size = parserPtrs->ItbReadPtr - parserPtrs->ItbValidPtr - 1;
    } else if (parserPtrs->ItbReadPtr < parserPtrs->ItbValidPtr ) {
        size = (parserPtrs->ItbEndPtr - parserPtrs->ItbValidPtr + parserPtrs->ItbReadPtr - parserPtrs->ItbStartPtr);
    } else {
        size = parserPtrs->ItbEndPtr + 1 - parserPtrs->ItbStartPtr;
    }
    return size>=0?size:0;
}

unsigned 
botf_itb_feeder_get_occupied(const BOTF_ParserPtrs *parserPtrs)
{
    int size;

    /* Calculate the available size in ITB */
    if (parserPtrs->ItbReadPtr <= parserPtrs->ItbValidPtr) {
        size = parserPtrs->ItbValidPtr - parserPtrs->ItbReadPtr;
    } else {
        size = (parserPtrs->ItbWrapAroundPtr - parserPtrs->ItbReadPtr) +
               (parserPtrs->ItbValidPtr - parserPtrs->ItbStartPtr);
    }
    return size>=0?size:0;
}


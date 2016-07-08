/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2007-2016 Broadcom. All rights reserved.
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
 * SVP Start Code Parser module for the on the fly PVR 
 *
 ***************************************************************************/
/* Magnum */
#include "bstd.h"
#include "bkni.h"
#include "bchp_xpt_rave.h"
#if BXPT_HAS_XCBUF
#include "bchp_xpt_xcbuff.h"
#endif

/* OTF */
#include "botf_priv.h"

BDBG_MODULE(botf_scv_parser);

#define BDBG_MSG_TRACE(x)   /* BDBG_MSG(x) */
#define BDBG_MSG_PARSE(x)   /* BDBG_MSG(x) */
/* This flag must be undefined for normal operation 
 * This flag is used to introduce errors to the stream by skipping 
 * 5 startcodes each time. This allows us to check the robustness of
 * OTF when the input stream is corrupted.
 */
#undef  BOTF_SCV_PARSER_ADD_ERRORS

#define B_DBG_DATA(x) (*((uint8_t *)(x)))


#define B_OTF_FEEDER_CAPTURE    0

#if B_OTF_FEEDER_CAPTURE    
#include <stdio.h>
#endif

struct botf_scv_parser_impl {
    bpvr_gop_manager manager; /* instance of the GOP manager */
    struct bpvr_start_code scode;
    const uint8_t *cdb_base;    
    const void *baseentryptr;
    const BOTF_ParserPtrs *IPParserPtrs;  
};

botf_scv_parser
botf_scv_parser_create(bpvr_gop_manager manager, const BOTF_ParserPtrs *IPParserPtrs)
{
    botf_scv_parser  parser;

    parser = BKNI_Malloc(sizeof(*parser));
    if (!parser) {
        return NULL;
    }
    parser->manager = manager;
    parser->IPParserPtrs = IPParserPtrs;

    return parser;
}

void 
botf_scv_parser_destroy(botf_scv_parser parser)
{
    BKNI_Free(parser);
    return;
}

void botf_scv_parser_reset(botf_scv_parser parser)
{
    parser->cdb_base = NULL;
    parser->baseentryptr = NULL;    
}

void botf_scv_parser_getlastscode(botf_scv_parser parser, bpvr_start_code *scode)
{
    *scode = parser->scode;
}

void botf_scv_parser_getdatabytes(botf_scv_parser parser, uint8_t offset, uint8_t count, uint8_t *buf)
{   
    uint8_t i;
    uint32_t word2, word3;

    word2 = ((uint32_t *) parser->scode.itbentryptr)[2];
    word3 = ((uint32_t *) parser->scode.itbentryptr)[3];

    BDBG_ASSERT(offset+count <= 7);

    for(i=0; i<count; i++)
    {
        if (i+offset < 4) {
            buf[i] = ((word3 >> (i+offset)*8) & 0xff);
        } else {
            buf[i] = ((word2 >> (i+offset-4)*8) & 0xff);
        }
    }
}

bool
botf_scv_parser_feed(botf_scv_parser parser, const void *scv_ptr_, size_t scv_len, size_t *scv_processed)
{
    size_t scv_count;
    uint32_t word0, word1, word3;
    unsigned type;    
    int offset;	
    const uint32_t *scv_ptr=scv_ptr_;
    bool paused;

    BDBG_MSG_PARSE(("%#x:new SCV entries %#x %u(%u)", (unsigned)parser, (unsigned)scv_ptr, (unsigned)scv_len/B_SCV_LEN, (unsigned)scv_len));
    if( (scv_len%B_SCV_LEN)!=0) {
        BDBG_ERR(("%p:new SCV entries %p %u(%u)", (void *)parser, (void *)scv_ptr, (unsigned)scv_len/B_SCV_LEN, (unsigned)scv_len));
    }
    BDBG_ASSERT((scv_len%B_SCV_LEN)==0);
    BDBG_ASSERT(((unsigned long)scv_ptr%sizeof(uint32_t))==0);
    BDBG_MSG_TRACE(("%#x %#x %#x %#x", parser->IPParserPtrs->ItbStartPtr, scv_ptr,  (const uint8_t *)scv_ptr + scv_len, (parser->IPParserPtrs->ItbEndPtr)+1));
    BDBG_ASSERT(parser->IPParserPtrs->ItbStartPtr <= (const uint8_t *)scv_ptr && (const uint8_t *)scv_ptr + scv_len <= (parser->IPParserPtrs->ItbEndPtr+1));

    /*
     * RAVE always reads and writes in 32 bit data words to ITB. System endianness will
     * take care of proper swapping of bytes. OTF also does the same thing, reads and writes
     * 32 bit words, so that OTF doesn't need to worry about the endianness.
     */
    botf_mem_flush(parser->IPParserPtrs->mem, scv_ptr, scv_len);
    for(paused=false, scv_count=0; scv_count<scv_len && !paused; scv_count+=B_SCV_LEN) {
        word0 = scv_ptr[0];
        type = (word0 >> 24)&0xff;
        BDBG_MSG_TRACE(("SCV(%d:%d) %#x", scv_len/B_SCV_LEN, scv_count/B_SCV_LEN, type));
        switch(type) {
        case B_SCV_TYPE_BASE:
            word1 = scv_ptr[1];
            parser->cdb_base = botf_mem_vaddr(parser->IPParserPtrs->mem, word1);
            parser->baseentryptr = scv_ptr;
            BDBG_MSG_TRACE(("%#x base %#x:%#x", (unsigned)parser, word1, (unsigned)parser->cdb_base));
            break;
        case B_SCV_TYPE_PCR_OFFSET:
        case B_SCV_TYPE_PCR:
            break;
        case B_SCV_TYPE_PTS:
            {                
                word1 = scv_ptr[1];                 
                BDBG_MSG_PARSE(("%#x:PTS %#x(%u)", (unsigned)parser, word1, word1));
                bpvr_gop_manager_set_pts(parser->manager, word1);
            }
            break;
        case B_SCV_TYPE_SVPSCODE:			
            /* currently we are using only first 7 bytes, so ignore 2nd ITB entry when ITB is split */
            if ((B_SCV_ISSPLIT(word0)) && (B_SCV_SPLITINDX(word0) != 0)){
                /* Second entry in Split case */
                BDBG_MSG_PARSE(("%#x:split entry found and ignored", (unsigned)parser));
                /* Ignore */
            } else {
                const uint8_t *scode_cdb, *scode_cdb_wrap;
                const uint8_t *cdb_end = parser->IPParserPtrs->CdbWrapAroundPtr;

                word1 = scv_ptr[1];
                offset = B_SCV_SCODE_OFF(word1);
                /* offset points to the last byte copied. And copied bytes are start counting after start code suffix. 
                 * But we need to keep all data starting from the start code preffix 00 00 01, so we subtract from offset either 3+7 or 3+8.
                 * After substraction offset could become negative. */
                parser->scode.code = B_SCV_SCODE(word1);
                if ( B_SCV_ISSPLIT(word0))
                {
                    /* Split entry */
                    BDBG_ASSERT( B_SCV_SPLITINDX(word0) == 0);
                    offset -= 10;
                }					
                else
                {
                    offset -= 11;
                }
                /* b_pvr_scv_feeder_scode_check(parser, "botf_scv_parser_feed", parser->cdb_base, parser->scode.code, offset); */
                scode_cdb = parser->cdb_base + offset; 
                if (scode_cdb >= cdb_end) {
                    scode_cdb_wrap = parser->IPParserPtrs->CdbStartPtr  + (scode_cdb - cdb_end); 
                    BDBG_WRN((">scode: FORWARD 0x000001%02x %d %p -> %p", parser->scode.code, offset, scode_cdb, scode_cdb_wrap));
                    scode_cdb = scode_cdb_wrap;
                } else if (scode_cdb < parser->IPParserPtrs->CdbStartPtr) {
                    scode_cdb_wrap = cdb_end - (parser->IPParserPtrs->CdbStartPtr - scode_cdb); 
                    BDBG_WRN(("<scode: BACKWARD 0x000001%02x %d %p -> %p", parser->scode.code, offset, scode_cdb, scode_cdb_wrap));
                    scode_cdb = scode_cdb_wrap;
                }                 
                parser->scode.cdb = scode_cdb;
                BDBG_ASSERT(parser->baseentryptr);
                parser->scode.itbentryptr = scv_ptr;
                parser->scode.prevbaseentryptr = parser->baseentryptr;

                BDBG_MSG_TRACE(("scode: 0x000001%02x %#x %#x", parser->scode.code, offset, (unsigned)parser->scode.cdb));
                paused = bpvr_gop_manager_feed(parser->manager, &parser->scode);
            }
            break;
        case B_SCV_TYPE_BTP:
            {
                uint8_t btp_cmd;                
                uint8_t disp_iframe;

                btp_cmd = (word0>>8) & 0xff;
                if (btp_cmd != B_SCV_BTP_PICTAGCMD) {
                    break;
                }
                word3 = ((uint32_t *) scv_ptr)[3];                
                BDBG_MSG_PARSE(("%#x: TAG %#x", (unsigned)parser, word3));
                bpvr_gop_manager_set_tag(parser->manager, word3);
                disp_iframe = (word0 & 0x1);
                bpvr_gop_manager_set_disp_only_lfiframe(parser->manager, disp_iframe);                
                disp_iframe = (word0>>1) & 0x1;
                bpvr_gop_manager_set_disp_lfiframe(parser->manager, disp_iframe);
            }
            
            break;

        default:           
            BDBG_WRN(("%p:unknown SCV entry[%u] %#x(%p..%p..%p %#x:%#x:%#x:%#x)", (void *)parser, (unsigned)(scv_count/B_SCV_LEN), type, parser->IPParserPtrs->ItbStartPtr, (void *)scv_ptr,  parser->IPParserPtrs->ItbEndPtr, (unsigned)scv_ptr[0], (unsigned)scv_ptr[1], (unsigned)scv_ptr[2], (unsigned)scv_ptr[3]));
            break;
        }
        scv_ptr = (void *) (B_SCV_LEN + (uint8_t *)scv_ptr);
#ifdef BOTF_SCV_PARSER_ADD_ERRORS
       if (scv_count+5*B_SCV_LEN<scv_len)
       {
           scv_ptr = (void *) (5*B_SCV_LEN + (uint8_t *)scv_ptr);
           scv_count += 5*B_SCV_LEN;
       }
#endif
    }
    *scv_processed = scv_count;
    return paused;
}



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
 *  OTF platform(7401) specific file
 *
 **************************************************************************/

 /* Magnum */
#include "bstd.h"
#include "bkni.h"
 
/* OTF */
#include "botf.h"
#include "botf_priv.h"

BDBG_MODULE(BOTF_PLATFORM);

static uint8_t *
BOTF_P_ReadPtr(BOTF_Handle hOtf, uint32_t reg)
{
    uint32_t val;    
    void *addr;

    val = BREG_Read32(hOtf->hBReg, reg);
    if(val) {
        addr = botf_mem_vaddr(&hOtf->mem, val);
    } else {
        addr = NULL;
    }
    return addr;
}

static uint8_t *
BOTF_P_ReadPtrAndUpdate(BOTF_Handle hOtf, uint32_t src, uint32_t dst)
{
    uint32_t val;    
    void *addr;

    val = BREG_Read32(hOtf->hBReg, src);
    BREG_Write32(hOtf->hBReg, dst, val);
    if(val) {
        addr = botf_mem_vaddr(&hOtf->mem, val);
    } else {
        addr = NULL;
    }
    return addr;
}

static void
BOTF_P_WritePtr(BOTF_Handle hOtf, uint32_t reg, const void *ptr)
{
    uint32_t offset;

    offset = botf_mem_paddr(&hOtf->mem, ptr);
    BREG_Write32(hOtf->hBReg, reg, offset);
    return;
}

/* This routine updates local copies of pointers with the RAVE hardware 
 * Also, it copies the IP CDB pointers to OP CDB pointers
*/
void BOTF_P_UpdateBufferPtrs (BOTF_Handle hOtf)
{
    uint8_t *ptr;
    /* Update OP CDB wrap and valid pointers with input pointers */
    ptr = BOTF_P_ReadPtrAndUpdate(hOtf, hOtf->IpParserRegMap.CDB_Wrap, hOtf->OpParserRegMap.CDB_Wrap);
    hOtf->OPParserPtrs.CdbWrapAroundPtr = hOtf->IPParserPtrs.CdbWrapAroundPtr = ptr?ptr:hOtf->IPParserPtrs.CdbEndPtr;
    hOtf->OPParserPtrs.CdbValidPtr = hOtf->IPParserPtrs.CdbValidPtr = 
    BOTF_P_ReadPtrAndUpdate(hOtf, hOtf->IpParserRegMap.CDB_Valid, hOtf->OpParserRegMap.CDB_Valid); 

    /* Get IP Parser ITB wrap and valid pointers */
    ptr=BOTF_P_ReadPtr(hOtf, hOtf->IpParserRegMap.ITB_Wrap);
    hOtf->IPParserPtrs.ItbWrapAroundPtr=ptr?ptr:hOtf->IPParserPtrs.ItbEndPtr;
    hOtf->IPParserPtrs.ItbValidPtr=BOTF_P_ReadPtr(hOtf, hOtf->IpParserRegMap.ITB_Valid);
    hOtf->OPParserPtrs.ItbReadPtr=BOTF_P_ReadPtr(hOtf, hOtf->OpParserRegMap.ITB_Read);
    hOtf->OPParserPtrs.CdbReadPtr=BOTF_P_ReadPtr(hOtf, hOtf->OpParserRegMap.CDB_Read);

    return;
}

void BOTF_P_InitBufferPtrs(BOTF_Handle hOtf)
{

    hOtf->IPParserPtrs.CdbStartPtr=BOTF_P_ReadPtr(hOtf, hOtf->IpParserRegMap.CDB_Base);
    hOtf->IPParserPtrs.CdbEndPtr=BOTF_P_ReadPtr(hOtf, hOtf->IpParserRegMap.CDB_Base+4);
    hOtf->IPParserPtrs.CdbReadPtr=BOTF_P_ReadPtr(hOtf, hOtf->IpParserRegMap.CDB_Read);
    hOtf->IPParserPtrs.ItbStartPtr=BOTF_P_ReadPtr(hOtf, hOtf->IpParserRegMap.ITB_Base);
    hOtf->IPParserPtrs.ItbEndPtr=BOTF_P_ReadPtr(hOtf, hOtf->IpParserRegMap.ITB_Base+4);

    /* Initialize wraparound pointer to the end */
    hOtf->IPParserPtrs.ItbWrapAroundPtr = hOtf->IPParserPtrs.ItbEndPtr;
    hOtf->IPParserPtrs.CdbWrapAroundPtr = hOtf->IPParserPtrs.CdbEndPtr;

    hOtf->IPParserPtrs.ItbReadPtr=BOTF_P_ReadPtr(hOtf, hOtf->IpParserRegMap.ITB_Read);
    hOtf->OPParserPtrs.CdbStartPtr=BOTF_P_ReadPtr(hOtf, hOtf->OpParserRegMap.CDB_Base);
    hOtf->OPParserPtrs.CdbEndPtr=BOTF_P_ReadPtr(hOtf, hOtf->OpParserRegMap.CDB_Base+4);
    hOtf->OPParserPtrs.CdbValidPtr=BOTF_P_ReadPtr(hOtf, hOtf->OpParserRegMap.CDB_Valid);
    hOtf->OPParserPtrs.ItbStartPtr=BOTF_P_ReadPtr(hOtf, hOtf->OpParserRegMap.ITB_Base);
    hOtf->OPParserPtrs.ItbEndPtr=BOTF_P_ReadPtr(hOtf, hOtf->OpParserRegMap.ITB_Base+4);
    hOtf->OPParserPtrs.ItbValidPtr=BOTF_P_ReadPtr(hOtf, hOtf->OpParserRegMap.ITB_Valid);
    hOtf->OPParserPtrs.ItbReadPtr=BOTF_P_ReadPtr(hOtf, hOtf->OpParserRegMap.ITB_Read);
    hOtf->OPParserPtrs.CdbReadPtr=BOTF_P_ReadPtr(hOtf, hOtf->OpParserRegMap.CDB_Read);

    hOtf->OPParserPtrs.CdbWrapAroundPtr = hOtf->OPParserPtrs.CdbEndPtr;
    hOtf->OPParserPtrs.ItbWrapAroundPtr = hOtf->OPParserPtrs.ItbEndPtr;

    return;
}

void BOTF_P_SetIPReadPtrs(BOTF_Handle hOtf, const void *CdbReadPtr, const void *ItbReadPtr)
{
    BDBG_MSG(("CDB:%p->%p ITB:%p->%p", hOtf->IPParserPtrs.CdbReadPtr, CdbReadPtr, hOtf->IPParserPtrs.ItbReadPtr, ItbReadPtr));
    BOTF_P_WritePtr(hOtf, hOtf->IpParserRegMap.CDB_Read, CdbReadPtr);
    hOtf->IPParserPtrs.CdbReadPtr = CdbReadPtr;

    BOTF_P_WritePtr(hOtf, hOtf->IpParserRegMap.ITB_Read, ItbReadPtr);
    hOtf->IPParserPtrs.ItbReadPtr = (void *)ItbReadPtr;    
    return;
}

void BOTF_P_SetOPITBValidPtr(BOTF_Handle hOtf)
{
    BOTF_P_WritePtr(hOtf, hOtf->OpParserRegMap.ITB_Valid, hOtf->OPParserPtrs.ItbValidPtr);
    return;
}

/* Set ITB wrap around pointer to the current valid pointer */
void BOTF_P_SetOPITBWrapPtr(BOTF_Handle hOtf)
{
    BOTF_P_WritePtr(hOtf, hOtf->OpParserRegMap.ITB_Wrap, hOtf->OPParserPtrs.ItbWrapAroundPtr);
    return;
}


void BOTF_PlatformOpen(BOTF_Handle hOtf)
{
    uint32_t baseptr, val;
    void *addr;
    void *cached_addr;
    BERR_Code rc;

    BDBG_MSG(("BOTF_PlatformOpen"));

    /* Save current OP parser pointers */
    hOtf->OpParserRegSave.CdbStart = BREG_Read32(hOtf->hBReg, hOtf->OpParserRegMap.CDB_Base);
    hOtf->OpParserRegSave.ItbStart = BREG_Read32(hOtf->hBReg, hOtf->OpParserRegMap.ITB_Base);    

    /* initialize OP CDB ptrs */
    val = BREG_Read32(hOtf->hBReg, hOtf->IpParserRegMap.CDB_Base);
    BREG_Write32(hOtf->hBReg, hOtf->OpParserRegMap.CDB_Base, val);

    val = BREG_Read32(hOtf->hBReg, hOtf->IpParserRegMap.CDB_Base+4);
    BREG_Write32(hOtf->hBReg, hOtf->OpParserRegMap.CDB_Base+4, val);
    val = BREG_Read32(hOtf->hBReg, hOtf->IpParserRegMap.CDB_Read);
    BREG_Write32(hOtf->hBReg, hOtf->OpParserRegMap.CDB_Read, val);
    val = BREG_Read32(hOtf->hBReg, hOtf->IpParserRegMap.CDB_Wrap);
    BREG_Write32(hOtf->hBReg, hOtf->OpParserRegMap.CDB_Wrap, val);
    val = BREG_Read32(hOtf->hBReg, hOtf->IpParserRegMap.CDB_Valid);
    BREG_Write32(hOtf->hBReg, hOtf->OpParserRegMap.CDB_Valid, val);

    /* Allocate and set destn ITB pointers */
    addr =  BMEM_AllocAligned( hOtf->hBMem, hOtf->OPParserITBSize, hOtf->OPParserITBAlign, 0 ); 
    rc = BMEM_Heap_ConvertAddressToCached(hOtf->hBMem, addr, &cached_addr);
    BDBG_ASSERT(rc==BERR_SUCCESS);
    baseptr = botf_mem_paddr(&hOtf->mem, cached_addr);
    hOtf->itb_buffer = addr;
    BREG_Write32(hOtf->hBReg, hOtf->OpParserRegMap.ITB_Base, baseptr);
    BREG_Write32(hOtf->hBReg, hOtf->OpParserRegMap.ITB_Read, baseptr);
    BREG_Write32(hOtf->hBReg, hOtf->OpParserRegMap.ITB_Valid, baseptr);
    BREG_Write32(hOtf->hBReg, hOtf->OpParserRegMap.ITB_Wrap, baseptr+hOtf->OPParserITBSize - 1);
    BREG_Write32(hOtf->hBReg, hOtf->OpParserRegMap.ITB_Base+4,  baseptr+hOtf->OPParserITBSize - 1);

    return;
}

void BOTF_PlatformFlushOpParser(BOTF_Handle hOtf)
{
    uint32_t val;
    /* Write baseptr to both read and valid to indicate empty buffer */
    val = BREG_Read32(hOtf->hBReg, hOtf->OpParserRegMap.ITB_Base);
    BREG_Write32(hOtf->hBReg, hOtf->OpParserRegMap.ITB_Read, val);
    BREG_Write32(hOtf->hBReg, hOtf->OpParserRegMap.ITB_Valid, val);
}

void BOTF_PlatformClose(BOTF_Handle hOtf)
{       
    BMEM_Free(hOtf->hBMem, hOtf->itb_buffer);
    hOtf->itb_buffer = NULL;
    /* Restore the OP parser pointers */
    BREG_Write32(hOtf->hBReg, hOtf->OpParserRegMap.CDB_Base, hOtf->OpParserRegSave.CdbStart);
    BREG_Write32(hOtf->hBReg, hOtf->OpParserRegMap.ITB_Base, hOtf->OpParserRegSave.ItbStart);
}


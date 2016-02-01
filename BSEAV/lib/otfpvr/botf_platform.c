/***************************************************************************
 *	   Copyright (c) 2007-2013, Broadcom Corporation
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
 * Module Description:
 *  OTF platform(7401) specific file
 * 
 * Revision History:
 *
 * $brcm_Log: $
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
    BDBG_MSG(("CDB:%#x->%#x ITB:%#x->%#x", (unsigned)hOtf->IPParserPtrs.CdbReadPtr, (unsigned)CdbReadPtr, (unsigned)hOtf->IPParserPtrs.ItbReadPtr, (unsigned)ItbReadPtr));
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


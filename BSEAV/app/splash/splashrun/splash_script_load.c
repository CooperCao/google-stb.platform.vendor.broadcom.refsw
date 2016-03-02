/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 ******************************************************************************/
#include "splash_script_load.h"

#include "splash_magnum.h"

#include "bchp_rdc.h"
#include "bchp_common.h"
/* #include "lib_printf.h" */

#include "splash_script_load.h"

BDBG_MODULE(splash_script_load);

#define BRDC_REGISTER(reg)      ((reg) + BCHP_PHYSICAL_OFFSET)

/* Remember highest slot number ever used */
static int s_highSlot = -1;
/* A little package for remembering pointers */
#define REMEMBER_CHUNK 256
static unsigned int s_numPointers = 0;
static unsigned int s_maxPointers = 0;
static uint32_t** s_Pointers = NULL;
static void RememberPointer (uint32_t* pointer)
{
	if (s_numPointers == s_maxPointers)
	{
		unsigned int new_length = s_maxPointers + REMEMBER_CHUNK;
		uint32_t** new_array =
			(uint32_t**)BKNI_Malloc ((new_length) * sizeof (uint32_t*));
		BDBG_ASSERT(new_array);
		BKNI_Memcpy (
			(void*)new_array, (void*)s_Pointers,
			sizeof(uint32_t*) * s_maxPointers);
		s_maxPointers = new_length;
		s_Pointers = new_array;
	}
	s_Pointers[s_numPointers++] = pointer;
}
static void FreePointers (BMEM_Handle hHeap)
{
	unsigned int index;
	for (index = 0 ; index < s_numPointers ; ++index)
		BMEM_Free (hHeap, (void*)s_Pointers[index]);
	BKNI_Free ((void*)s_Pointers);
	s_Pointers = NULL;
	s_numPointers = 0;
	s_maxPointers = 0;
}

/**************************
 * This function takes a set of RULs for a given slot (say all the RULS for top
 * field or all for bottom field), copies them into memory, adds a command
 * to the first N-1 RULs so that on completion the next RUL is made active,
 * and puts the first RUL in the slot ready to be executed.
 *
 * Instead of having a function do this, this work can be done ahead of time
 * as long as the user knows exactly where each RUL is located in memory */
static void InitializeSlot
(
    BREG_Handle       hRegister,
    BMEM_Handle       hHeap,
    int               iSlotIndex,
    uint32_t          ulTriggerSelect,
    uint32_t         *pListCount,
    uint32_t         *pulList,
    int               iListCount
)
{
    int              i;
    int              iNumEntries, iPrvNumEntries = 0, iCount;
    uint32_t         ulAddrOffset;
    uint32_t         ulValue;
    uint32_t        *pulCurr;
    uint32_t         ulSlotOffset = 4 * iSlotIndex * sizeof(uint32_t);
    int              iRUL;
	uint32_t        *apulAlloced[30];
	uint32_t        *apulCached[30];
    BERR_Code        rc;

    BDBG_MSG(("InitializeSlot : iSlotIndex:%d ulTriggerSelect:%d pListCount:%p pulList:%p iListCount:%d \n",
			  iSlotIndex,   ulTriggerSelect,(void *)pListCount,(void *)pulList, iListCount));
    /************************
     * We have to place the dumped RULs into allocated memory. Instead a user could
     * have pre-determined memory locations for each of the RULs instead of directly
     * allocating them in an application. In that case, this entire loop is pre-defined.
     */
    iRUL = 0;
    for(i=0; i<iListCount; i++)
    {
        /* how many entries for this RUL? */
        iNumEntries = pListCount[i+1] - pListCount[i];

        /* RUL passed inspection */
        /* allocate aligned memory (add 6 more elements for extra entries -- see below) */
        apulAlloced[iRUL] = (uint32_t *) BMEM_AllocAligned(hHeap,
            sizeof(uint32_t) * (iNumEntries + 6), 5, 0);
		RememberPointer (apulAlloced[iRUL]);
		BMEM_Heap_ConvertAddressToCached(hHeap, (void *)apulAlloced[iRUL], (void **)&apulCached[iRUL]);

        /* copy list into memory */
        BKNI_Memcpy(apulCached[iRUL], pulList + pListCount[i],
            sizeof(uint32_t) * iNumEntries);

		/* there is previous RUL in list? */
        if(iRUL > 0)
        {
            /* modify previous RUL (end of RUL) to install this new RUL when finished. This
             * can also be done ahead of time if the memory address is fixed. */

            /* get pointer to the current end of the list */
            pulCurr = apulCached[iRUL-1] + iPrvNumEntries;

            BDBG_MSG(("********* pulCurr = %p, apulAlloced[%d] = %p i=%d, iPrvNumEntries=%d\n",
                (void*)pulCurr, iRUL-1, (void*)apulAlloced[iRUL-1],i, iPrvNumEntries));

            /* write command to install new RUL address into slot after the
               previous list is complete  */
            rc = BMEM_ConvertAddressToOffset(hHeap,
                (void*)apulAlloced[iRUL], &ulAddrOffset);
            if(rc)
                BDBG_ERR(("Error Converting %d", rc));
            *pulCurr++ = BCHP_FIELD_ENUM(RDC_RUL, opcode, REG_WRITE_IMM) ;
            *pulCurr++ = BRDC_REGISTER(BCHP_RDC_desc_0_addr) + ulSlotOffset;
            *pulCurr++ =
                BCHP_FIELD_DATA(RDC_desc_0_addr, addr, ulAddrOffset);

            /* are we going to add more entries for this list (not the last list)? */
            iCount = (i != iListCount - 1) ? (iNumEntries+6) : iNumEntries ;

            /* write new count into slot */
            *pulCurr++ = BCHP_FIELD_ENUM(RDC_RUL, opcode, REG_WRITE_IMM) ;
            *pulCurr++ = BRDC_REGISTER(BCHP_RDC_desc_0_config) + ulSlotOffset;
            *pulCurr++ =
                BCHP_FIELD_DATA(RDC_desc_0_config, count,          iCount - 1) |
                BCHP_FIELD_DATA(RDC_desc_0_config, trigger_select, ulTriggerSelect) |
                #if defined(BCHP_RDC_desc_0_config_reserved0_SHIFT)
                BCHP_FIELD_DATA(RDC_desc_0_config, reserved0,      0)               |
                #elif defined(BCHP_RDC_desc_0_config_segmented_SHIFT)
                BCHP_FIELD_DATA(RDC_desc_0_config, segmented,      0)               |
                #endif
                BCHP_FIELD_DATA(RDC_desc_0_config, repeat,         1)               |
                BCHP_FIELD_DATA(RDC_desc_0_config, enable,         1)               |
                BCHP_FIELD_DATA(RDC_desc_0_config, done,           1);

            /* flush previous RUL: we just appended cmd to it to link the current RUL start addr */
            BMEM_Heap_FlushCache(hHeap, apulCached[iRUL-1], sizeof(uint32_t)*(iPrvNumEntries+6));

            /* Remember this slot */
            if (iSlotIndex > s_highSlot) s_highSlot = iSlotIndex;
        }

        iRUL++;
        iPrvNumEntries = iNumEntries ;
    }

	/* flush the last RUL */
	BMEM_Heap_FlushCache(hHeap, apulCached[iRUL-1], sizeof(uint32_t)*(iPrvNumEntries+6));

    /*****************************
     * Setup the initial RDC slot with the first list.
     * Again if the memory was fixed ahead of time then
     * the calculations below would be fixed and unchanging.
     */

    /* configure initial slot */
    iNumEntries = pListCount[1] - pListCount[0];
    if(iListCount > 1)
    {
        /* we have 6 more entries on this list */
        iNumEntries += 6;
    }

    ulValue =
        BCHP_FIELD_DATA(RDC_desc_0_config, count,          iNumEntries - 1) |
        BCHP_FIELD_DATA(RDC_desc_0_config, trigger_select, ulTriggerSelect) |
		#if defined(BCHP_RDC_desc_0_config_reserved0_SHIFT)
		BCHP_FIELD_DATA(RDC_desc_0_config, reserved0,      0)               |
		#elif defined(BCHP_RDC_desc_0_config_segmented_SHIFT)
		BCHP_FIELD_DATA(RDC_desc_0_config, segmented,      0)               |
		#endif
        BCHP_FIELD_DATA(RDC_desc_0_config, repeat,         1)               |
        BCHP_FIELD_DATA(RDC_desc_0_config, enable,         1)               |
        BCHP_FIELD_DATA(RDC_desc_0_config, done,           1);
    BREG_Write32(hRegister, BCHP_RDC_desc_0_config + ulSlotOffset,
        ulValue);

    /* put first entry into slot */
    rc = BMEM_ConvertAddressToOffset(hHeap,
        (void*)apulAlloced[0], &ulAddrOffset);
    if(rc)
        BDBG_ERR(("Error Converting %d", rc));

    BREG_Write32(hRegister, BCHP_RDC_desc_0_addr + ulSlotOffset,
        ulAddrOffset);

	/* Remember this slot */
	if (iSlotIndex > s_highSlot) s_highSlot = iSlotIndex;
}

void splash_bvn_init(
BREG_Handle hRegister, SplashBufInfo *pSplashBufInfo, SplashData* pSplashData)
{
    int  ii;
    int  iTriggerIndex;
	int  iSurIdx;

    if( 0 == pSplashData->nAulReg )
    {
        BDBG_ERR(("\n\n\n" "***************************************************************************"
                  "\n" "Error ... Error ... Error ! " "\n" "Register Dump is empty !!! "
                  "\n" "Looks like you have not chosen the instrumented version of REG basemodule"
                  "\n" "Please rebuild/rerun the splash script generator with instrumented BREG "
                  "\n" "module"
                  "\n" "***************************************************************************"
                  "\n\n\n")) ;
        return ;
    }
    /* Dumped registers */
    for(ii=0 ; ii < pSplashData->nAulReg ; ++ii)
    {
        /* Write dumped register value */
        BREG_Write32(
			hRegister,
			*(pSplashData->pAulReg + 2*ii),
			*(pSplashData->pAulReg + 2*ii+1));
    }

	/* write our surface addr into RDC scratch registers,
	 * note that the RUL has been built to pick them up for gfx display */
	for (ii=0; ii<pSplashData->iNumDisplay; ii++)
	{
		iSurIdx = pSplashData->pDispInfo[ii].iSurIdx;
		BREG_Write32(hRegister, pSplashData->pDispInfo[ii].ulRdcScratchReg0,
			pSplashBufInfo->aulSurfaceBufOffset[iSurIdx]);
		if (pSplashData->pDispInfo[ii].ulRdcScratchReg1)
		{
			BREG_Write32(
				hRegister, pSplashData->pDispInfo[ii].ulRdcScratchReg1,
				pSplashBufInfo->aulSurfaceBufOffset[iSurIdx]);
		}
		BDBG_MSG(("** set surface offset 0x%8.8x for display %d\n",pSplashBufInfo->aulSurfaceBufOffset[iSurIdx], ii));
	}

    BDBG_MSG(("***** Number of triggers = %d \n", pSplashData->iNumTrigMap )) ;
    for( iTriggerIndex = 0 ;
	     iTriggerIndex < pSplashData->iNumTrigMap ;
		 ++iTriggerIndex)
    {
        if(pSplashData->pTrigMap[iTriggerIndex].TriggerHwNum != -1)
        {
            BDBG_MSG(("Slot number %d, Trigger number %d aList%d[]\n",
                pSplashData->pTrigMap[iTriggerIndex].SlotNum,
				pSplashData->pTrigMap[iTriggerIndex].TriggerHwNum,
				iTriggerIndex));
            /* Initialize slot for RUL list n */
            InitializeSlot(
                hRegister,
                pSplashBufInfo->hRulMem,
                pSplashData->pTrigMap[iTriggerIndex].SlotNum,
                pSplashData->pTrigMap[iTriggerIndex].TriggerHwNum,
                pSplashData->pTrigMap[iTriggerIndex].aListCountArray,
                pSplashData->pTrigMap[iTriggerIndex].aListArray,
                pSplashData->pTrigMap[iTriggerIndex].ListCountMaxIndex);
        }
    }

    for( iTriggerIndex = 0 ;
	     iTriggerIndex < pSplashData->iNumTrigMap ;
		 ++iTriggerIndex)
    {
        if(pSplashData->pTrigMap[iTriggerIndex].TriggerHwNum != -1)
        {
            /* Force Top slot to start engine for each display */
            if(0 == (pSplashData->pTrigMap[iTriggerIndex].SlotNum%2))
            {
                uint32_t ulSlotOffset =
					4 * pSplashData->pTrigMap[iTriggerIndex].SlotNum *
					sizeof(uint32_t);

                BDBG_MSG(("Writing immediate to %08x",
					BCHP_RDC_desc_0_immediate+ulSlotOffset));
                BREG_Write32(hRegister, BCHP_RDC_desc_0_immediate+ulSlotOffset, 1);
            }
        }
    }
}

void splash_bvn_uninit(BREG_Handle hRegister, SplashBufInfo *pSplashBufInfo)
{
	/* Program the register DMA controller to just stop */

	int iSlotIndex;
	for (iSlotIndex = 0 ; iSlotIndex <= s_highSlot ; ++iSlotIndex)
	{
		uint32_t ulSlotOffset = 4 * iSlotIndex * sizeof(uint32_t);
		uint32_t ulValue =
			BCHP_FIELD_DATA(RDC_desc_0_config, count,          0)              |
			BCHP_FIELD_DATA(RDC_desc_0_config, trigger_select, 0)              |
			#if defined(BCHP_RDC_desc_0_config_reserved0_SHIFT)
			BCHP_FIELD_DATA(RDC_desc_0_config, reserved0,      0)              |
			#elif defined(BCHP_RDC_desc_0_config_segmented_SHIFT)
			BCHP_FIELD_DATA(RDC_desc_0_config, segmented,      0)              |
			#endif
			BCHP_FIELD_DATA(RDC_desc_0_config, repeat,         0)              |
			BCHP_FIELD_DATA(RDC_desc_0_config, enable,         0)              |
			BCHP_FIELD_DATA(RDC_desc_0_config, done,           1);
		BREG_Write32(hRegister, BCHP_RDC_desc_0_config + ulSlotOffset,
			ulValue);
	}

	/* It takes a non-zero amount of time for the old RULs to stop executing */
	BKNI_Sleep (500);

	/* Now the RULs can be freed, because RDC is no longer reading them. */
	FreePointers (pSplashBufInfo->hRulMem);
}

/* End of File */

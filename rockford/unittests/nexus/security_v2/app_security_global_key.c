/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#if NEXUS_HAS_SECURITY && NEXUS_SECURITY_API_VERSION==2

#include "nexus_platform.h"
#include "nexus_memory.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

BDBG_MODULE(BSAGElib);

#include "nexus_security_datatypes.h"
#include "nexus_security.h"
#include "nexus_keyladder.h"
#include "security_main.h"
#include "security_util.h"
#include "nexus_dma.h"

static void CompleteCallback(void *pParam, int iParam)
{
    BSTD_UNUSED(iParam);
    fprintf(stderr, "CompleteCallback:%#lx fired. DMA complete.\n", (unsigned long)pParam);
    BKNI_SetEvent(pParam);
}


int main(int argc, char **argv)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_KeyLadderHandle hKeyLadder = NULL;
    NEXUS_KeyLadderAllocateSettings ladderAllocSettings;
    NEXUS_KeyLadderSettings ladderSettings;
    NEXUS_KeyLadderLevelKey levelKey;
    NEXUS_KeySlotHandle hKeySlot = NULL;
    NEXUS_KeySlotAllocateSettings keyslotAllocSettings;
    NEXUS_KeySlotSettings keyslotSettings;
    NEXUS_KeySlotEntrySettings keyslotEntrySettings;
    NEXUS_KeySlotBlockEntry slotEntry = NEXUS_KeySlotBlockEntry_eCpsClear;
    NEXUS_DmaHandle           hDma;
    NEXUS_DmaJobSettings      jobSettings;
    NEXUS_DmaJobHandle        hDmaJob;
    NEXUS_DmaJobBlockSettings blockSettings;
    NEXUS_DmaJobStatus        jobStatus;
    BKNI_EventHandle          hDmaEvent = NULL;
    uint8_t *pSrc;
    uint8_t *pDest;
    char procIn[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x0A, 0x0B, 0x0C, 0x0D, 0x11, 0x22, 0x33, 0x44};
    int result = 0;
    NEXUS_Error rc;
    uint8_t plaintext[16]  = { 0x59, 0xb5, 0x08, 0x8e, 0x6d, 0xad, 0xc3, 0xad, 0x5f, 0x27, 0xa4, 0x60, 0x87, 0x2d, 0x59, 0x29 };

    BDBG_ENTER(main);

    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);

    /* platform initialisation */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

    /* allocate keyslot */
    NEXUS_KeySlot_GetDefaultAllocateSettings( &keyslotAllocSettings );
    keyslotAllocSettings.owner = NEXUS_SecurityCpuContext_eHost;
    keyslotAllocSettings.slotType = NEXUS_KeySlotType_eIvPerBlock;
    keyslotAllocSettings.useWithDma = true;
    hKeySlot = NEXUS_KeySlot_Allocate( &keyslotAllocSettings );
    if( !hKeySlot ) { result = BERR_TRACE(-1); goto exit; };
    BKNI_Sleep(500);

    /* configure keyslot parameters */
    NEXUS_KeySlot_GetSettings( hKeySlot, &keyslotSettings );
    rc = NEXUS_KeySlot_SetSettings( hKeySlot, &keyslotSettings );
    if( rc != NEXUS_SUCCESS ) { result = BERR_TRACE(-1); goto exit; };
    BKNI_Sleep(500);

    /* configure keyslot entry parameters*/
    NEXUS_KeySlot_GetEntrySettings( hKeySlot, slotEntry, &keyslotEntrySettings );
    keyslotEntrySettings.algorithm = NEXUS_CryptographicAlgorithm_eAes128;
    keyslotEntrySettings.algorithmMode = NEXUS_CryptographicAlgorithmMode_eEcb;
    keyslotEntrySettings.terminationMode = NEXUS_KeySlotTerminationMode_eClear;
    keyslotEntrySettings.rPipeEnable = true;
    keyslotEntrySettings.gPipeEnable = true;
    rc = NEXUS_KeySlot_SetEntrySettings( hKeySlot, slotEntry, &keyslotEntrySettings );
    if( rc != NEXUS_SUCCESS ) { result = BERR_TRACE(-1); goto exit; };
    BKNI_Sleep(500);

    /* allocate the keyladder */
    NEXUS_KeyLadder_GetDefaultAllocateSettings( &ladderAllocSettings );
    ladderAllocSettings.owner = NEXUS_SecurityCpuContext_eHost;
    hKeyLadder = NEXUS_KeyLadder_Allocate( NEXUS_ANY_ID, &ladderAllocSettings );
    if( !hKeyLadder ) { result = BERR_TRACE(-1); goto exit; };
    BKNI_Sleep(500);

    /* configure the root key  */
    NEXUS_KeyLadder_GetSettings( hKeyLadder, &ladderSettings );
    ladderSettings.algorithm = NEXUS_CryptographicAlgorithm_eAes128;
    ladderSettings.keySize = 128;
    ladderSettings.operation = NEXUS_CryptographicOperation_eDecrypt;
    ladderSettings.mode = NEXUS_KeyLadderMode_eCp_128_4;
    ladderSettings.root.type = NEXUS_KeyLadderRootType_eGlobalKey;
    ladderSettings.root.askm.caVendorId = 0x1234;
    ladderSettings.root.askm.caVendorIdScope = NEXUS_KeyladderCaVendorIdScope_eFixed;
    ladderSettings.root.askm.stbOwnerSelect = NEXUS_KeyLadderStbOwnerIdSelect_eOne;
    ladderSettings.root.globalKey.owner = NEXUS_KeyLadderGlobalKeyOwnerIdSelect_eOne;
    ladderSettings.root.globalKey.index = 0x0;
    rc = NEXUS_KeyLadder_SetSettings( hKeyLadder, &ladderSettings );
    if( rc != NEXUS_SUCCESS ) { result = BERR_TRACE(-1); goto exit; };
    BKNI_Sleep(500);

    /* set the keyladder level keys */
    NEXUS_KeyLadder_GetLevelKeyDefault( &levelKey );
    levelKey.level = 3;
    BKNI_Memcpy( levelKey.ladderKey, procIn, sizeof(levelKey.ladderKey) );
    rc = NEXUS_KeyLadder_GenerateLevelKey( hKeyLadder, &levelKey );
    if( rc != NEXUS_SUCCESS ) { result = BERR_TRACE(-1); goto exit; };
    BKNI_Sleep(500);

    NEXUS_KeyLadder_GetLevelKeyDefault( &levelKey );
    levelKey.level = 4;
    levelKey.route.destination = NEXUS_KeyLadderDestination_eKeyslotKey;
    levelKey.route.keySlot.handle = hKeySlot;
    levelKey.route.keySlot.entry = slotEntry;
    BKNI_Memcpy( levelKey.ladderKey, procIn, sizeof(levelKey.ladderKey) );
    rc = NEXUS_KeyLadder_GenerateLevelKey( hKeyLadder, &levelKey );
    if( rc != NEXUS_SUCCESS ) { result = BERR_TRACE(-1); goto exit; };
    BKNI_Sleep(500);


    /* Open DMA handle */
    hDma = NEXUS_Dma_Open(0, NULL);

    /* Create DMA event */
    BKNI_CreateEvent(&hDmaEvent);

    NEXUS_Memory_Allocate( sizeof(plaintext), NULL, (void **)&pSrc );
    NEXUS_Memory_Allocate( sizeof(plaintext), NULL, (void **)&pDest );

    BKNI_Memcpy( pSrc, plaintext, sizeof(plaintext) );
    BKNI_Memset( pDest, 0, sizeof(plaintext) );
    BKNI_Sleep(500);

    NEXUS_DmaJob_GetDefaultSettings(&jobSettings);
    jobSettings.numBlocks                   = 1;
    jobSettings.keySlot                     = hKeySlot;
    jobSettings.dataFormat                  = NEXUS_DmaDataFormat_eBlock;
    jobSettings.completionCallback.callback = CompleteCallback;
    jobSettings.completionCallback.context  = hDmaEvent;

    hDmaJob = NEXUS_DmaJob_Create(hDma, &jobSettings);
    BKNI_Sleep(500);

    NEXUS_DmaJob_GetDefaultBlockSettings(&blockSettings);
    blockSettings.pSrcAddr                  = pSrc;
    blockSettings.pDestAddr                 = pDest;
    blockSettings.blockSize                 = sizeof(plaintext);
    blockSettings.resetCrypto               = true;
    blockSettings.scatterGatherCryptoStart  = true;
    blockSettings.scatterGatherCryptoEnd    = true;
    blockSettings.cached                    = true;
    rc = NEXUS_DmaJob_ProcessBlocks(hDmaJob, &blockSettings, 1);
	    if (rc == NEXUS_DMA_QUEUED )
    {
        BKNI_WaitForEvent(hDmaEvent, BKNI_INFINITE);
        NEXUS_DmaJob_GetStatus(hDmaJob, &jobStatus);
        BDBG_ASSERT(jobStatus.currentState == NEXUS_DmaJobState_eComplete);
    }
    BKNI_Sleep(500);

exit:

    if( hKeySlot )NEXUS_KeySlot_Free( hKeySlot );
    if( hKeyLadder ) NEXUS_KeyLadder_Free( hKeyLadder );
    if( pSrc ) NEXUS_Memory_Free(pSrc);
    if( pDest ) NEXUS_Memory_Free(pDest);
    if( hDmaEvent ) BKNI_DestroyEvent(hDmaEvent);
    if( hDmaJob ) NEXUS_DmaJob_Destroy(hDmaJob);
    if( hDma ) NEXUS_Dma_Close(hDma);

    NEXUS_Platform_Uninit();

    BDBG_LEAVE(main);
    return result;
}

#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported.\n");
    return -1;
}
#endif

/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
/*  This example shows how to use scatter and gather for CBC based algorithm */

#if NEXUS_HAS_SECURITY && (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 4)

#include "nexus_platform.h"
#include "nexus_dma.h"
#include "nexus_memory.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "nexus_security.h"

#define DMA_BLOCK   32
#define DMA_JOBS    1


static void CompleteCallback(void *pParam, int iParam)
{
    BSTD_UNUSED(iParam);
    fprintf(stderr, "CompleteCallback:%#lx fired\n", (unsigned long)pParam);
    BKNI_SetEvent(pParam);
}



int main(void)
{
    NEXUS_PlatformSettings platformSettings;
	NEXUS_Error rc;
    NEXUS_DmaHandle dma;
    NEXUS_DmaJobSettings jobSettings;
    NEXUS_DmaJobHandle dmaJob;
    NEXUS_DmaJobBlockSettings blockSettings[2];
    NEXUS_DmaJobStatus jobStatus;
    BKNI_EventHandle		dmaEvent = NULL;
    NEXUS_SecurityKeySlotSettings keySettings;
    NEXUS_KeySlotHandle                 encKeyHandle;
    NEXUS_SecurityAlgorithmSettings     NexusConfig;
    NEXUS_SecurityClearKey key;

    uint8_t *pSrc, *pDest, *pDest2;
    unsigned int i;
    uint8_t keys[16] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
    uint8_t iv[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 };


    /* Platform init */
    NEXUS_Platform_GetDefaultSettings(&platformSettings); 
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

	


    /* Allocate AV keyslots */
    NEXUS_Security_GetDefaultKeySlotSettings(&keySettings);
    keySettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
    encKeyHandle = NEXUS_Security_AllocateKeySlot(&keySettings);
    if(!encKeyHandle) 
    {
        printf("\nAllocate enc keyslot failed \n");
        return 1;
    }

	

    /* Set up encryption key */
    NEXUS_Security_GetDefaultAlgorithmSettings(&NexusConfig);
    NexusConfig.algorithm           = NEXUS_SecurityAlgorithm_eAes;
    NexusConfig.algorithmVar        = NEXUS_SecurityAlgorithmVariant_eCbc;
    NexusConfig.operation           = NEXUS_SecurityOperation_eEncrypt;
    NexusConfig.keyDestEntryType    = NEXUS_SecurityKeyType_eEven;

    if ( NEXUS_Security_ConfigAlgorithm(encKeyHandle, &NexusConfig)!= 0)
    {
        printf("\nConfigAlg enc keyslot failed \n");
        return 1;
    }

	printf("\n\n\nTEST\n\n\n");

	
    /* Load key */
	NEXUS_Security_GetDefaultClearKey(&key);
    key.keyEntryType = NEXUS_SecurityKeyType_eEven;
	key.keyIVType 	= NEXUS_SecurityKeyIVType_eNoIV;
    key.keySize = sizeof(keys);
    BKNI_Memcpy(key.keyData, keys, key.keySize);
    if (NEXUS_Security_LoadClearKey(encKeyHandle, &key) != 0) {
        printf("\nLoad encryption key failed \n");
        return 1;
    }
    /* Load IV */
	key.keyEntryType = NEXUS_SecurityKeyType_eEven;
    key.keyIVType = NEXUS_SecurityKeyType_eIv;
    key.keySize = sizeof(iv);
    BKNI_Memcpy(key.keyData, iv, key.keySize);
    if (NEXUS_Security_LoadClearKey(encKeyHandle, &key) != 0) {
        printf("\nLoad encryption IV failed \n");
        return 1;
    }


    /* Open DMA handle */
    dma = NEXUS_Dma_Open(0, NULL);
	/* and DMA event */
	BKNI_CreateEvent(&dmaEvent);

    NEXUS_Memory_Allocate(DMA_BLOCK, NULL, (void *)&pSrc);
    NEXUS_Memory_Allocate(DMA_BLOCK, NULL, (void *)&pDest);
    NEXUS_Memory_Allocate(DMA_BLOCK, NULL, (void *)&pDest2); 

    memset(pSrc, 0, DMA_BLOCK*sizeof(unsigned char));
    memset(pDest, 1, DMA_BLOCK*sizeof(unsigned char));
    memset(pDest2, 2, DMA_BLOCK*sizeof(unsigned char));

    NEXUS_DmaJob_GetDefaultSettings(&jobSettings);
    jobSettings.numBlocks                   = 1;
    jobSettings.keySlot                     = encKeyHandle;
    jobSettings.dataFormat                  = NEXUS_DmaDataFormat_eBlock;
	jobSettings.completionCallback.callback = CompleteCallback;
	jobSettings.completionCallback.context	= dmaEvent;

    dmaJob = NEXUS_DmaJob_Create(dma, &jobSettings);

    NEXUS_DmaJob_GetDefaultBlockSettings(&blockSettings[0]);
    blockSettings[0].pSrcAddr                   = pSrc;  
    blockSettings[0].pDestAddr                  = pDest; 
    blockSettings[0].blockSize                  = DMA_BLOCK;
    blockSettings[0].resetCrypto                = true;
    blockSettings[0].scatterGatherCryptoStart   = true;
    blockSettings[0].scatterGatherCryptoEnd     = true;
    blockSettings[0].cached                     = true;

	rc = NEXUS_DmaJob_ProcessBlocks(dmaJob, blockSettings, 1);
	if (rc == NEXUS_DMA_QUEUED )
	{
		BKNI_WaitForEvent(dmaEvent, BKNI_INFINITE);
		NEXUS_DmaJob_GetStatus(dmaJob, &jobStatus);
		BDBG_ASSERT(jobStatus.currentState == NEXUS_DmaJobState_eComplete);
	}

    NEXUS_DmaJob_Destroy(dmaJob);


    /*  Encryption using scatter and gather */
    NEXUS_DmaJob_GetDefaultSettings(&jobSettings);
    jobSettings.numBlocks                   = 2;  /* Create two descriptors */
    jobSettings.keySlot                     = encKeyHandle;
    jobSettings.dataFormat                  = NEXUS_DmaDataFormat_eBlock;
	jobSettings.completionCallback.callback = CompleteCallback;
	jobSettings.completionCallback.context	= dmaEvent;

    dmaJob = NEXUS_DmaJob_Create(dma, &jobSettings);

    /* Set up the first block */
    NEXUS_DmaJob_GetDefaultBlockSettings(&blockSettings[0]);
    blockSettings[0].pSrcAddr                   = pSrc;  
    blockSettings[0].pDestAddr                  = pDest2; 
    blockSettings[0].blockSize                  = 16; /* 16 bytes first */
    blockSettings[0].resetCrypto                = true;
    blockSettings[0].scatterGatherCryptoStart   = true;
    blockSettings[0].scatterGatherCryptoEnd     = false;
    blockSettings[0].cached                     = true;

    /* Set up the second block */
    NEXUS_DmaJob_GetDefaultBlockSettings(&blockSettings[1]);
    blockSettings[1].pSrcAddr                   = pSrc+16;  
    blockSettings[1].pDestAddr                  = pDest2+16; 
    blockSettings[1].blockSize                  = DMA_BLOCK-16; /* Remaining bytes */
    blockSettings[1].resetCrypto                = false;
    blockSettings[1].scatterGatherCryptoStart   = false;
    blockSettings[1].scatterGatherCryptoEnd     = true;
    blockSettings[1].cached                     = true;

    rc = NEXUS_DmaJob_ProcessBlocks(dmaJob, blockSettings, 2);
	if (rc == NEXUS_DMA_QUEUED )
	{
		BKNI_WaitForEvent(dmaEvent, BKNI_INFINITE);
		NEXUS_DmaJob_GetStatus(dmaJob, &jobStatus);
		BDBG_ASSERT(jobStatus.currentState == NEXUS_DmaJobState_eComplete);
	}

    NEXUS_DmaJob_Destroy(dmaJob);

    /* Make sure pSrc matches pDest 2 */
    for (i=0;i<DMA_BLOCK; i++ )
    {
        if ( pDest[i] != pDest2[i] )
        {
            printf ("\nComparison failed at location %d\n", i);
            break;
        }
    }

    if ( i==DMA_BLOCK )
            printf ("\nTest passed\n");
    else
    {
        printf("\nSource buffer1 is: \n");
        for (i=0;i<DMA_BLOCK; i++ )
        {
            printf ("%02x ", pDest[i]);
        }   
        printf("\nDestination buffer 2 is: \n");
        for (i=0;i<DMA_BLOCK; i++ )
        {
            printf ("%02x ", pDest2[i]);
        }   
    }


    NEXUS_Dma_Close(dma);


    NEXUS_Security_FreeKeySlot(encKeyHandle);

    return 0;
}

#else /* NEXUS_HAS_SECURITY */
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform!\n");
    return -1;
}
#endif

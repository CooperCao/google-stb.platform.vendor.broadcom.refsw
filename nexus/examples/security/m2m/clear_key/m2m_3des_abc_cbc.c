/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

/* 
 M2M using ASKM keyladder. For 7420B0 the following values are used:
   OTP key         = FEEFFEFF48D98D088D461ED16803B200
   Key B            = 818897154F3907D6C04F1283B142E4D4
   STB Owner ID = 55aa 
   CA Vendor ID  = 1234 
   Module ID	   = 04 
 ProcIn values result in the following clear key:
   0x111260bb5acbc2831f6f899d8012c492 
 
 For 7422A0 the following values are used:
   OTP keyA            = 121B3B37AA12EBCE828FCC9327EAA67D
   STB Owner ID      = 55aa 
   CA Vendor ID       = 1234 
   Module ID            = 8 


*/
#if NEXUS_HAS_SECURITY && (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 1)


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

void PrintBlock(uint8_t *pData)
{
unsigned int i;
	for(i=0; i < DMA_BLOCK; i++)
	{
		printf( " %x ", pData[i]);
	}
	printf( "\n\n");
}

 
int main(void)
{
    NEXUS_PlatformSettings platformSettings;

	NEXUS_DmaHandle dma;
	NEXUS_DmaJobSettings jobSettings;
	NEXUS_DmaJobHandle dmaJob;
	NEXUS_DmaJobBlockSettings blockSettings;
	NEXUS_DmaJobStatus jobStatus;
	NEXUS_Error rc;
	BKNI_EventHandle		dmaEvent = NULL;
    
	NEXUS_SecurityKeySlotSettings keySettings;
	NEXUS_KeySlotHandle					encKeyHandle, decKeyHandle;
	NEXUS_SecurityAlgorithmSettings 	NexusConfig;
    NEXUS_SecurityClearKey key;

	uint8_t *pSrc, *pDest, *pDest2;
	unsigned int i;
	
	uint8_t keys[24] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77 };
    uint8_t iv[24]   = { 0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00, 0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88 };
    

	

	/* Platform init */
    NEXUS_Platform_GetDefaultSettings(&platformSettings); 
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);


	/* Allocate M2M keyslots */
    NEXUS_Security_GetDefaultKeySlotSettings(&keySettings);
    keySettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
    encKeyHandle = NEXUS_Security_AllocateKeySlot(&keySettings);
    if(!encKeyHandle) 
	{
        printf("\nAllocate keyslot failed \n");
        return 1;
    }

    NEXUS_Security_GetDefaultKeySlotSettings(&keySettings);
    keySettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
    decKeyHandle = NEXUS_Security_AllocateKeySlot(&keySettings);
    if(!decKeyHandle) 
	{
        printf("\nAllocate keyslot failed \n");
        return 1;
    }

	/* Set up key for clear key */
    NEXUS_Security_GetDefaultAlgorithmSettings(&NexusConfig);
	NexusConfig.algorithm 			= NEXUS_SecurityAlgorithm_e3DesAbc;
	NexusConfig.algorithmVar  		= NEXUS_SecurityAlgorithmVariant_eCbc;

	/* ++++++++ */
	NexusConfig.terminationMode     = NEXUS_SecurityTerminationMode_eClear;
	NexusConfig.ivMode              = NEXUS_SecurityIVMode_eRegular;
	NexusConfig.solitarySelect      = NEXUS_SecuritySolitarySelect_eClear;
	NexusConfig.caVendorID          = 0x1234;
	NexusConfig.askmModuleID        = NEXUS_SecurityAskmModuleID_eModuleID_8;
	NexusConfig.otpId               = NEXUS_SecurityOtpId_eOtpVal;
    /* -------- */
	NexusConfig.key2Select			= NEXUS_SecurityKey2Select_eRealKey;

    /* -------- */
	/* ++++++++ */

	NexusConfig.operation 			= NEXUS_SecurityOperation_eEncrypt;
	NexusConfig.keyDestEntryType 	= NEXUS_SecurityKeyType_eClear;

	if ( NEXUS_Security_ConfigAlgorithm(encKeyHandle, &NexusConfig)!= 0)
	{
        printf("\nConfigAlg clear keyslot failed \n");
        return 1;
    }

	NEXUS_Security_GetDefaultClearKey(&key);
    key.keyEntryType = NEXUS_SecurityKeyType_eClear;
    key.keySize = sizeof(keys);
    /*--------*/
    key.keyIVType    = NEXUS_SecurityKeyIVType_eNoIV;
    /*--------*/

    BKNI_Memcpy(key.keyData, keys, sizeof(keys));
    if (NEXUS_Security_LoadClearKey(encKeyHandle, &key) != 0) {
        printf("\nLoad encryption key failed \n");
        return 1;
    }


	key.keyIVType	 = NEXUS_SecurityKeyIVType_eIV;
	key.keyEntryType = NEXUS_SecurityKeyType_eClear;
	key.keySize = sizeof(iv);
	BKNI_Memcpy(key.keyData, iv, key.keySize);
	if (NEXUS_Security_LoadClearKey(encKeyHandle, &key) != 0) 
	{
		printf("\nLoad encryption IV failed \n");
		return 1;
	}


	 /* -------- */
	/* ++++++++ */

	NexusConfig.operation 			= NEXUS_SecurityOperation_eDecrypt;
	NexusConfig.keyDestEntryType 	= NEXUS_SecurityKeyType_eClear;

	if ( NEXUS_Security_ConfigAlgorithm(decKeyHandle, &NexusConfig)!= 0)
	{
        printf("\nConfigAlg clear keyslot failed \n");
        return 1;
    }

	NEXUS_Security_GetDefaultClearKey(&key);
    key.keyEntryType = NEXUS_SecurityKeyType_eClear;
    key.keySize = sizeof(keys);
    /*--------*/
    key.keyIVType    = NEXUS_SecurityKeyIVType_eNoIV;
    /*--------*/

    BKNI_Memcpy(key.keyData, keys, sizeof(keys));
    if (NEXUS_Security_LoadClearKey(decKeyHandle, &key) != 0) {
        printf("\nLoad encryption key failed \n");
        return 1;
    }

	
	key.keyIVType	 = NEXUS_SecurityKeyIVType_eIV;
	key.keyEntryType = NEXUS_SecurityKeyType_eClear;
	key.keySize = sizeof(iv);
	BKNI_Memcpy(key.keyData, iv, key.keySize);
	if (NEXUS_Security_LoadClearKey(decKeyHandle, &key) != 0) 
	{
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
	memset(pDest, 0, DMA_BLOCK*sizeof(unsigned char));
	memset(pDest2, 0, DMA_BLOCK*sizeof(unsigned char));

	memset(pSrc, 0x22, DMA_BLOCK*sizeof(unsigned char));

	PrintBlock(pSrc);
	PrintBlock(pDest);
	PrintBlock(pDest2);
	
	NEXUS_DmaJob_GetDefaultSettings(&jobSettings);
	jobSettings.numBlocks 					= 1;
	jobSettings.keySlot 					= encKeyHandle;
	jobSettings.dataFormat 					= NEXUS_DmaDataFormat_eBlock;
	jobSettings.completionCallback.callback = CompleteCallback;
	jobSettings.completionCallback.context	= dmaEvent;

	dmaJob = NEXUS_DmaJob_Create(dma, &jobSettings);

	NEXUS_DmaJob_GetDefaultBlockSettings(&blockSettings);
	blockSettings.pSrcAddr 					= pSrc;  
	blockSettings.pDestAddr 				= pDest; 
	blockSettings.blockSize 				= DMA_BLOCK;
	blockSettings.resetCrypto 				= true;
	blockSettings.scatterGatherCryptoStart 	= true;
	blockSettings.scatterGatherCryptoEnd 	= true;
	blockSettings.cached 					= true;

	rc = NEXUS_DmaJob_ProcessBlocks(dmaJob, &blockSettings, 1);
	if (rc == NEXUS_DMA_QUEUED )
	{
		BKNI_WaitForEvent(dmaEvent, BKNI_INFINITE);
		NEXUS_DmaJob_GetStatus(dmaJob, &jobStatus);
		BDBG_ASSERT(jobStatus.currentState == NEXUS_DmaJobState_eComplete);
	}

	NEXUS_DmaJob_Destroy(dmaJob);

	PrintBlock(pSrc);
	PrintBlock(pDest);
	PrintBlock(pDest2);
		


    NEXUS_DmaJob_GetDefaultSettings(&jobSettings);
	jobSettings.numBlocks 					= 1;
	jobSettings.keySlot 					= decKeyHandle;
	jobSettings.dataFormat 					= NEXUS_DmaDataFormat_eBlock;
	jobSettings.completionCallback.callback = CompleteCallback;
	jobSettings.completionCallback.context	= dmaEvent;

	dmaJob = NEXUS_DmaJob_Create(dma, &jobSettings);

	NEXUS_DmaJob_GetDefaultBlockSettings(&blockSettings);
	blockSettings.pSrcAddr 					= pDest;  
	blockSettings.pDestAddr 				= pDest2; 
	blockSettings.blockSize 				= DMA_BLOCK;
	blockSettings.resetCrypto 				= true;
	blockSettings.scatterGatherCryptoStart 	= true;
	blockSettings.scatterGatherCryptoEnd 	= true;
	blockSettings.cached 					= true;

	rc = NEXUS_DmaJob_ProcessBlocks(dmaJob, &blockSettings, 1);
	if (rc == NEXUS_DMA_QUEUED )
	{
		BKNI_WaitForEvent(dmaEvent, BKNI_INFINITE);
		NEXUS_DmaJob_GetStatus(dmaJob, &jobStatus);
		BDBG_ASSERT(jobStatus.currentState == NEXUS_DmaJobState_eComplete);
	}
	
	NEXUS_DmaJob_Destroy (dmaJob);

	PrintBlock(pSrc);
	PrintBlock(pDest);
	PrintBlock(pDest2);
		


	/* Make sure pSrc matches pDest 2 */
	for (i=0;i<DMA_BLOCK; i++ )
	{
		if ( pSrc[i] != pDest2[i] )
		{
			printf ("\nComparison failed at location %d\n", i);
			break;
		}
	}

	if ( i==DMA_BLOCK )
			printf ("\nTest passed\n");


	NEXUS_Dma_Close(dma);


	BKNI_DestroyEvent(dmaEvent);
	NEXUS_Security_FreeKeySlot(encKeyHandle);
	NEXUS_Security_FreeKeySlot(decKeyHandle);

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

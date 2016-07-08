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

/* This example shows how to use keyladder with Cuskey+Swizzle for root key generation.
   Due to differences in HW design, the value provided in this sample only works for 
   settop chips like 740x.  For other chips, the key values may need to be changed for 
   the test to pass
*/
#if NEXUS_HAS_SECURITY && (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 1)

#include "nexus_platform.h"
#include "nexus_dma.h"
#include "nexus_memory.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "nexus_security.h"
#include "nexus_keyladder.h"

#define DMA_BLOCK   32

#define DMA_JOBS    1


static void CompleteCallback(void *pParam, int iParam)
{
    BSTD_UNUSED(iParam);
    fprintf(stderr, "CompleteCallback:%#lx fired\n", (unsigned long)pParam);
    BKNI_SetEvent(pParam);
}


NEXUS_Error GenerateRouteKey(NEXUS_VirtualKeyLadderHandle vklHandle, NEXUS_KeySlotHandle keyHandle)
{
	NEXUS_SecurityEncryptedSessionKey decryptSessionkey;
	NEXUS_SecurityEncryptedControlWord decryptCW;
	unsigned char ucProcInForKey3[16] = { 		
		0x0f, 0x09, 0xa2, 0x06,  /*high key */  
		0x19, 0x88, 0xb6, 0x89,	    
		0x28, 0xeb, 0x90, 0x2e,   /*low key  use twice */
		0xb2, 0x36, 0x18, 0x88};

	unsigned char ucProcInKey4[16] = { 0xe4, 0x62, 0x75, 0x1b, 0x5d, 0xd4, 0xbc, 0x02, 0x27, 0x9e, 0xc9, 0x6c, 0xc8, 0x66, 0xec, 0x10 };
	NEXUS_Error rc;
	NEXUS_VirtualKeyLadderInfo  vklInfo;


	/* get VKL info from handle */
	NEXUS_Security_GetVKLInfo(vklHandle, &vklInfo);
    printf("\nVKL Handle is %p  for VKL %d\n", (void *)vklHandle, vklInfo.vkl);

	/* Load session key  - key3 */
	NEXUS_Security_GetDefaultSessionKeySettings(&decryptSessionkey);
	decryptSessionkey.keyladderType 	= NEXUS_SecurityKeyladderType_e3Des;
	decryptSessionkey.swizzleType		= NEXUS_SecuritySwizzleType_eSwizzle0;
	decryptSessionkey.cusKeyL 			= 0x00; 
	decryptSessionkey.cusKeyH 			= 0x00; 
	decryptSessionkey.cusKeyVarL 		= 0x00; 
	decryptSessionkey.cusKeyVarH 		= 0xFF; 
	decryptSessionkey.keyGenCmdID       = NEXUS_SecurityKeyGenCmdID_eKeyGen;
	decryptSessionkey.sessionKeyOp      = NEXUS_SecuritySessionKeyOp_eNoProcess;
	decryptSessionkey.bASKMMode         = false;
	decryptSessionkey.rootKeySrc        = NEXUS_SecurityRootKeySrc_eCuskey;
	decryptSessionkey.bSwapAESKey       = false;
    decryptSessionkey.keyDestIVType     = NEXUS_SecurityKeyIVType_eNoIV;
	decryptSessionkey.bRouteKey 		= false;
	decryptSessionkey.operation 		= NEXUS_SecurityOperation_eDecrypt;
	decryptSessionkey.operationKey2 	= NEXUS_SecurityOperation_eEncrypt;
	decryptSessionkey.keyEntryType 		= NEXUS_SecurityKeyType_eClear;

	/* ++++++++ */
	decryptSessionkey.custSubMode   	  	= NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4;
	decryptSessionkey.virtualKeyLadderID	= vklInfo.vkl;
	decryptSessionkey.keyMode            	= NEXUS_SecurityKeyMode_eRegular;
	/* ++++++++ */

	/*decryptSessionkey.pKeyData 		= ucProcInForKey3;*/
	BKNI_Memcpy(decryptSessionkey.keyData, ucProcInForKey3, sizeof(ucProcInForKey3));

	if ((rc = NEXUS_Security_GenerateSessionKey(keyHandle, &decryptSessionkey)) !=0)
	{
		printf("\nLoading session key failed \n");
		return BERR_TRACE(rc);
	}

	/* Load CW - key4 --- routed out */
	NEXUS_Security_GetDefaultControlWordSettings(&decryptCW);
    decryptCW.keyladderType = NEXUS_SecurityKeyladderType_e3Des;
    decryptCW.keySize 		= sizeof(ucProcInKey4); 
    decryptCW.keyEntryType 	= NEXUS_SecurityKeyType_eClear;

	/* ++++++++ */
	decryptCW.custSubMode        = NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4;
	decryptCW.virtualKeyLadderID = vklInfo.vkl;
	decryptCW.keyMode            = NEXUS_SecurityKeyMode_eRegular;
	/* ++++++++ */


    /*decryptCW.pKeyData = ucProcInKey4;*/
	BKNI_Memcpy(decryptCW.keyData, ucProcInKey4, decryptCW.keySize);
    decryptCW.operation = NEXUS_SecurityOperation_eDecrypt;     
    decryptCW.bRouteKey = true;
	if ((rc = NEXUS_Security_GenerateControlWord(keyHandle, &decryptCW)))
	{
		printf("\nLoading session key failed for M2M ODD key\n");
		return BERR_TRACE(rc);
	}

    return (rc);

}


void startM2MDMATask(NEXUS_KeySlotHandle encKeyHandle, NEXUS_KeySlotHandle decKeyHandle)
{
	NEXUS_DmaHandle dma;
	NEXUS_DmaJobSettings jobSettings;
	NEXUS_DmaJobHandle dmaJob;
	NEXUS_DmaJobBlockSettings blockSettings;
	NEXUS_DmaJobStatus jobStatus;
	NEXUS_Error rc;
	BKNI_EventHandle		dmaEvent = NULL;
	uint8_t *pSrc, *pDest, *pDest2;
	unsigned int i;


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

	/* Try different pattern */
	for (i = 0; i < DMA_BLOCK*sizeof(unsigned char); i += 2)
	{
		pSrc[i] 	= 0x55;
		pSrc[i + 1] = 0xAA;
	}

	NEXUS_DmaJob_GetDefaultSettings(&jobSettings);
	jobSettings.numBlocks					= 1;
	jobSettings.keySlot 					= encKeyHandle;
	jobSettings.dataFormat					= NEXUS_DmaDataFormat_eBlock;
	jobSettings.completionCallback.callback = CompleteCallback;
	jobSettings.completionCallback.context	= dmaEvent;

	dmaJob = NEXUS_DmaJob_Create(dma, &jobSettings);

	NEXUS_DmaJob_GetDefaultBlockSettings(&blockSettings);
	blockSettings.pSrcAddr					= pSrc;  
	blockSettings.pDestAddr 				= pDest; 
	blockSettings.blockSize 				= DMA_BLOCK;
	blockSettings.resetCrypto				= true;
	blockSettings.scatterGatherCryptoStart	= true;
	blockSettings.scatterGatherCryptoEnd	= true;
	blockSettings.cached					= true;

	rc = NEXUS_DmaJob_ProcessBlocks(dmaJob, &blockSettings, 1);

	if (rc == NEXUS_DMA_QUEUED )
	{
		BKNI_WaitForEvent(dmaEvent, BKNI_INFINITE);
		NEXUS_DmaJob_GetStatus(dmaJob, &jobStatus);
		BDBG_ASSERT(jobStatus.currentState == NEXUS_DmaJobState_eComplete);
	}
	NEXUS_DmaJob_Destroy (dmaJob);

	NEXUS_DmaJob_GetDefaultSettings(&jobSettings);
	jobSettings.numBlocks					= 1;
	jobSettings.keySlot 					= decKeyHandle;
	jobSettings.dataFormat					= NEXUS_DmaDataFormat_eBlock;
	jobSettings.completionCallback.callback = CompleteCallback;
	jobSettings.completionCallback.context	= dmaEvent;

	dmaJob = NEXUS_DmaJob_Create(dma, &jobSettings);

	NEXUS_DmaJob_GetDefaultBlockSettings(&blockSettings);
	blockSettings.pSrcAddr					= pDest;  
	blockSettings.pDestAddr 				= pDest2; 
	blockSettings.blockSize 				= DMA_BLOCK;
	blockSettings.resetCrypto				= true;
	blockSettings.scatterGatherCryptoStart	= true;
	blockSettings.scatterGatherCryptoEnd	= true;
	blockSettings.cached					= true;

	rc = NEXUS_DmaJob_ProcessBlocks(dmaJob, &blockSettings, 1);

	if (rc == NEXUS_DMA_QUEUED )
	{
		BKNI_WaitForEvent(dmaEvent, BKNI_INFINITE);
		NEXUS_DmaJob_GetStatus(dmaJob, &jobStatus);
		BDBG_ASSERT(jobStatus.currentState == NEXUS_DmaJobState_eComplete);
	}
	NEXUS_DmaJob_Destroy (dmaJob);

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
	else
		printf ("\nTest failed\n");
	
	printf("\nSource buffer1 is: \n");
	for (i=0;i<DMA_BLOCK; i++ )
	{
		printf ("%02x ", pSrc[i]);
	}	
	printf("\nEncryption buffer is: \n");
	for (i=0;i<DMA_BLOCK; i++ )
	{
		printf ("%02x ", pDest[i]);
	}	
	printf("\nDecryption buffer is: \n");
	for (i=0;i<DMA_BLOCK; i++ )
	{
		printf ("%02x ", pDest2[i]);
	}	
	
	NEXUS_Dma_Close(dma);
	BKNI_DestroyEvent(dmaEvent);

}


int main(void)
{
    NEXUS_PlatformSettings platformSettings;
	NEXUS_Error rc;
	NEXUS_SecurityKeySlotSettings keySettings;
	NEXUS_KeySlotHandle					encKeyHandle, decKeyHandle;
	NEXUS_SecurityAlgorithmSettings 	NexusConfig;
    NEXUS_SecurityClearKey key;
	NEXUS_SecurityVKLSettings          vklReq;
	NEXUS_VirtualKeyLadderHandle       vklHandle[8];
	unsigned int                       index;
	unsigned char                      reply;
	unsigned int                       allocNewVKL = 0;

    uint8_t keys[16] = { 0xbe, 0xf9, 0xb0, 0x67,0x13, 0xb8, 0xbc, 0x87, 0xbc, 0xfb, 0xb2, 0x69,0x13, 0xba, 0xbe, 0x8b };
	


	/* Platform init */
    NEXUS_Platform_GetDefaultSettings(&platformSettings); 
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);


	/* Allocate M2M keyslots */
    NEXUS_Security_GetDefaultKeySlotSettings(&keySettings);
    keySettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
	keySettings.keySlotType   = NEXUS_SecurityKeySlotType_eType3;
    encKeyHandle = NEXUS_Security_AllocateKeySlot(&keySettings);
    if(!encKeyHandle) 
	{
        printf("\nAllocate keyslot failed \n");
        return 1;
    }

    NEXUS_Security_GetDefaultKeySlotSettings(&keySettings);
    keySettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
	keySettings.keySlotType   = NEXUS_SecurityKeySlotType_eType3;
    decKeyHandle = NEXUS_Security_AllocateKeySlot(&keySettings);
    if(!decKeyHandle) 
	{
        printf("\nAllocate keyslot failed \n");
        return 1;
    }

	/* Set up key for clear key */
    NEXUS_Security_GetDefaultAlgorithmSettings(&NexusConfig);
	NexusConfig.algorithm 			= NEXUS_SecurityAlgorithm_e3DesAba;
	NexusConfig.algorithmVar  		= NEXUS_SecurityAlgorithmVariant_eEcb;

	/* ++++++++ */
	NexusConfig.terminationMode     = NEXUS_SecurityTerminationMode_eClear;
	NexusConfig.ivMode              = NEXUS_SecurityIVMode_eRegular;
	NexusConfig.solitarySelect      = NEXUS_SecuritySolitarySelect_eClear;
	NexusConfig.caVendorID          = 0x1234;
	NexusConfig.askmModuleID        = NEXUS_SecurityAskmModuleID_eModuleID_4;
	NexusConfig.otpId               = NEXUS_SecurityOtpId_eOtpVal;
	NexusConfig.key2Select		    = NEXUS_SecurityKey2Select_eReserved1;
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

	/* Set up key for keyladder  */
	NexusConfig.operation 			= NEXUS_SecurityOperation_eDecrypt;
	if ( NEXUS_Security_ConfigAlgorithm(decKeyHandle, &NexusConfig)!= 0)
	{
        printf("\nConfigAlg keyladder keyslot failed \n");
        return 1;
    }

    while (1)
    {
        printf("\nLoop %d....\n", allocNewVKL);
        for (index = 0; index < 8; index++)
        {
            /* request for an VKL to use */
			NEXUS_Security_GetDefaultVKLSettings(&vklReq);
			vklReq.client      = NEXUS_SecurityClientType_eHost;
			vklReq.custSubMode = NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4;
			if (allocNewVKL % 2)
			{
			    if (index % 2)
				    vklReq.newVKLCustSubModeAssoc = false;  /* use existing associated VKL (if any) */
			    else
				    vklReq.newVKLCustSubModeAssoc = true;  /* allocate new VKL */
			}
			else
			{
			    vklReq.newVKLCustSubModeAssoc = true;  /* allocate new VKL */
			}
		    if ((vklHandle[index] = NEXUS_Security_AllocateVKL(&vklReq)) == NULL)
				break;
	        if ((rc = GenerateRouteKey(vklHandle[index], decKeyHandle)) != 0)
	        {
                printf("\nGenerateRouteKey failed \n");
                break;
	        }
	        startM2MDMATask(encKeyHandle, decKeyHandle);
        }
		for (index = 0; index < 8; index++)
		{
		    NEXUS_Security_FreeVKL(vklHandle[index]);
		}
		
		printf("Press 'q' to stop the test\n");
		reply = getchar();
		if (reply == 'q')
			break;
		allocNewVKL++;
		
    }
	
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

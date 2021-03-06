/******************************************************************************
 *    (c)2007-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 *****************************************************************************/
/* This example shows how to use keyladder with OTP KeyA+No Swizzle for root key generation.
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

#define DMA_BLOCK_SIZE    16
#define NUM_DMA_BLOCK     1

static void CompleteCallback(void *pParam, int iParam)
{
    BSTD_UNUSED(iParam);
    fprintf(stderr, "CompleteCallback:%#lx fired\n", (unsigned long)pParam);
    BKNI_SetEvent(pParam);
}

static int Security_AllocateVkl ( NEXUS_SecurityCustomerSubMode custSubMode, NEXUS_SecurityVirtualKeyladderID * vkl )
{
    NEXUS_SecurityVKLSettings vklSettings;
    NEXUS_VirtualKeyLadderHandle vklHandle;
    NEXUS_VirtualKeyLadderInfo vklInfo;

    BDBG_ASSERT ( vkl );

    NEXUS_Security_GetDefaultVKLSettings ( &vklSettings );

    /* For pre Zeus 3.0, please set vklSettings.custSubMode */
	vklSettings.custSubMode = custSubMode;

    vklHandle = NEXUS_Security_AllocateVKL ( &vklSettings );

    if ( !vklHandle )
    {
        printf ( "\nAllocate VKL failed \n" );
        return 1;
    }

    NEXUS_Security_GetVKLInfo ( vklHandle, &vklInfo );
    printf ( "\nVKL Handle %p is allocated for VKL#%d\n", ( void * ) vklHandle, vklInfo.vkl & 0x7F );

    /* For Zeus 4.2 or later
     * if ((vklInfo.vkl & 0x7F ) >= NEXUS_SecurityVirtualKeyLadderID_eMax)
     * {
     * printf ( "\nAllocate VKL failed with invalid VKL Id.\n" );
     * return 1;
     * }
     */

    *vkl = vklInfo.vkl;

    return 0;
}

int M2M_operation(
	NEXUS_KeySlotHandle 	 keyHandle,
	unsigned char	        *outBuf,
	unsigned int	         outBufLen,
	unsigned char	        *ivBuf,
	unsigned int	         ivBufLen,
	unsigned char	        *inBuf,
	unsigned int	         inBufLen
)
{
	NEXUS_DmaHandle						dma;
	NEXUS_DmaJobSettings				jobSettings;
	NEXUS_DmaJobHandle					dmaJob;
	NEXUS_DmaJobBlockSettings			blockSettings;
	NEXUS_DmaJobStatus					jobStatus;
	NEXUS_Error							rc;
	BKNI_EventHandle		            dmaEvent = NULL;
    NEXUS_SecurityClearKey              key;


	/* First we need to load the IV values for CBC */
	key.keySize = ivBufLen;
	key.keyEntryType = NEXUS_SecurityKeyType_eEven;
	key.keyIVType	 = NEXUS_SecurityKeyIVType_eIV;
	BKNI_Memcpy(key.keyData, ivBuf, ivBufLen);
	if (NEXUS_Security_LoadClearKey(keyHandle, &key) != 0) 
	{
		printf("\nIV Value Loading failed \n");
		return 1;
	}
	if (outBufLen < inBufLen)
	{
		printf("\nOutput buffer too small! \n");
		return 1;
	}

	/* Open DMA handle */
	dma = NEXUS_Dma_Open(0, NULL);

	/* and DMA event */
	BKNI_CreateEvent(&dmaEvent);

	NEXUS_DmaJob_GetDefaultSettings(&jobSettings);
	jobSettings.numBlocks					= 1;
	jobSettings.keySlot 					= keyHandle;
	jobSettings.dataFormat					= NEXUS_DmaDataFormat_eBlock;
	jobSettings.completionCallback.callback = CompleteCallback;
	jobSettings.completionCallback.context  = dmaEvent;

	dmaJob = NEXUS_DmaJob_Create(dma, &jobSettings);

	NEXUS_DmaJob_GetDefaultBlockSettings(&blockSettings);
	blockSettings.pSrcAddr                  = inBuf;  
	blockSettings.pDestAddr                 = outBuf;
	blockSettings.blockSize                 = inBufLen;
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
	BKNI_DestroyEvent(dmaEvent);

	NEXUS_Dma_Close(dma);

	return (0);

}



int keySlotSetup(
		NEXUS_KeySlotHandle      keyHandle,
		NEXUS_SecurityOperation  operation
)
{
	NEXUS_SecurityAlgorithmSettings 	NexusConfig;
	NEXUS_SecurityEncryptedSessionKey	encryptedSessionkey;
	NEXUS_SecurityEncryptedControlWord	encrytedCW;
    NEXUS_SecurityVirtualKeyladderID vklId;

	unsigned char ucProcInForKey3[16] = { 		
		0x0f, 0x09, 0xa2, 0x06, 0x19, 0x88, 0xb6, 0x89,	    
		0x28, 0xeb, 0x90, 0x2e, 0xb2, 0x36, 0x18, 0x88};

	unsigned char ucProcInForKey4[16] = { 
		0xe4, 0x62, 0x75, 0x1b, 0x5d, 0xd4, 0xbc, 0x02, 
	    0x27, 0x9e, 0xc9, 0x6c, 0xc8, 0x66, 0xec, 0x10};

	/* Request for an VKL to use */
	if ( Security_AllocateVkl ( NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4, &vklId ) )
	{
		printf ( "\nAllocate VKL failed.\n" );
		return 1;
	}

	/* configure the key slot */
	NEXUS_Security_GetDefaultAlgorithmSettings(&NexusConfig);
	NexusConfig.algorithm			= NEXUS_SecurityAlgorithm_eAes128;
	NexusConfig.algorithmVar		= NEXUS_SecurityAlgorithmVariant_eCbc;
	/*
	No padding necessary: Clear termination (NEXUS_SecurityTerminationMode_eClear), 
      ciphertext stealing (NEXUS_SecurityTerminationMode_eCipherStealing), or 
      residual block termination (NEXUS_SecurityTerminationMode_eCbcResidual).
	*/
	NexusConfig.terminationMode 	= NEXUS_SecurityTerminationMode_eCbcResidual;
	NexusConfig.ivMode				= NEXUS_SecurityIVMode_eRegular;
	NexusConfig.solitarySelect		= NEXUS_SecuritySolitarySelect_eClear;
	NexusConfig.caVendorID			= 0x1234;
	NexusConfig.askmModuleID		= NEXUS_SecurityAskmModuleID_eModuleID_4;
	NexusConfig.otpId				= NEXUS_SecurityOtpId_eOtpVal;
	NexusConfig.key2Select			= NEXUS_SecurityKey2Select_eReserved1;
	NexusConfig.testKey2Select		= 0;
	NexusConfig.operation			= operation;
	NexusConfig.keyDestEntryType	= NEXUS_SecurityKeyType_eEven;

	if ( NEXUS_Security_ConfigAlgorithm(keyHandle, &NexusConfig)!= 0)
	{
		printf("\nConfigAlg clear keyslot failed \n");
		return 1;
	}

	/* set up the key ladder to route out the key for (operation) */
	NEXUS_Security_GetDefaultSessionKeySettings(&encryptedSessionkey);
	encryptedSessionkey.keyladderType	= NEXUS_SecurityKeyladderType_e3Des;
	encryptedSessionkey.swizzleType 	= NEXUS_SecuritySwizzleType_eNone;
	encryptedSessionkey.cusKeyL 		= 0x00; 
	encryptedSessionkey.cusKeyH 		= 0x00; 
	encryptedSessionkey.cusKeyVarL		= 0x00; 
	encryptedSessionkey.cusKeyVarH		= 0xFF; 
	encryptedSessionkey.keyGenCmdID 	= NEXUS_SecurityKeyGenCmdID_eKeyGen;
	encryptedSessionkey.sessionKeyOp	= NEXUS_SecuritySessionKeyOp_eNoProcess;
	encryptedSessionkey.bASKMMode		= false;
	encryptedSessionkey.rootKeySrc		= NEXUS_SecurityRootKeySrc_eOtpKeyA;
	encryptedSessionkey.bSwapAESKey 	= false;
	encryptedSessionkey.keyDestIVType	= NEXUS_SecurityKeyIVType_eNoIV;
	encryptedSessionkey.bRouteKey		= false;
	encryptedSessionkey.operation		= NEXUS_SecurityOperation_eDecrypt;
	encryptedSessionkey.operationKey2	= NEXUS_SecurityOperation_eEncrypt;
	encryptedSessionkey.keyEntryType	= NEXUS_SecurityKeyType_eEven;

	encryptedSessionkey.custSubMode 	   = NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4;
	encryptedSessionkey.virtualKeyLadderID = vklId;
	encryptedSessionkey.keyMode 		   = NEXUS_SecurityKeyMode_eRegular;

	BKNI_Memcpy(encryptedSessionkey.keyData, ucProcInForKey3, sizeof(ucProcInForKey3));

	if (NEXUS_Security_GenerateSessionKey(keyHandle, &encryptedSessionkey) != 0)
	{
		printf("\nLoading session key failed \n");
		return 1;
	}

	/* Load CW - key4 -- routed out */
	NEXUS_Security_GetDefaultControlWordSettings(&encrytedCW);
	encrytedCW.keyladderType = NEXUS_SecurityKeyladderType_e3Des;
	encrytedCW.keySize       = sizeof(ucProcInForKey4); 
	encrytedCW.keyEntryType  = NEXUS_SecurityKeyType_eEven;

	encrytedCW.custSubMode		  = NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4;
	encrytedCW.virtualKeyLadderID = vklId;
	encrytedCW.keyMode			  = NEXUS_SecurityKeyMode_eRegular;

	BKNI_Memcpy(encrytedCW.keyData, ucProcInForKey4, encrytedCW.keySize);
	encrytedCW.operation = NEXUS_SecurityOperation_eDecrypt;	 
	encrytedCW.bRouteKey = true;
	encrytedCW.keyDestIVType = NEXUS_SecurityKeyIVType_eNoIV;
	encrytedCW.keyGenCmdID	 = NEXUS_SecurityKeyGenCmdID_eKeyGen;
	encrytedCW.bSwapAESKey	 = false;
	if (NEXUS_Security_GenerateControlWord(keyHandle, &encrytedCW))
	{
		printf("\nGenerate Control Word failed for Key Slot Handle %x\n", (unsigned int)keyHandle);
		return 1;
	}

	return (0);

}

int main(int argc, char *argv[])
{
    NEXUS_PlatformSettings              platformSettings;
	NEXUS_SecurityKeySlotSettings       keySettings;
	NEXUS_KeySlotHandle					encKeyHandle;
/*	NEXUS_KeySlotHandle					decKeyHandle;  */
	FILE                                *dataFile;

	uint8_t *pEncSrc, *pEncDest /*, *pDecDest */;

	unsigned int   i;
	unsigned int   size;
	unsigned short bufLen;
	/*unsigned char  rbytes[2]; */

    uint8_t ivkeys[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


	/* Platform init */
    NEXUS_Platform_GetDefaultSettings(&platformSettings); 
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);


    if (argc < 2)
    {
	    printf( "Please provide the input/output files\n");
		printf( "%s  <data file for MAC calculation>\n", argv[0]);
		printf( "e.g.  %s data.bin\n", argv[0]);
       	return 0;
    }

	dataFile =  fopen(argv[1],"rb");
	
    if ( ( dataFile == 0 ) )
    {
	    printf( "Error: can not open input file %s\n", argv[1] );
       	return 0;
    }

	fseek (dataFile, 0, SEEK_END); 
	
	size = ftell(dataFile);

	/*size = fread(rbytes, 1, 2, dataFile);
	if (size < 2)
	{
		printf( "%s does not have sufficient number of bytes.\n", argv[1] );
		return 0;
	}
	bufLen = (unsigned short)((rbytes[0] << 8) | rbytes[1]);
	*/

	rewind(dataFile); 
	bufLen = size;

	printf( "\n\n\n\n\n\nFile size %d\n", size);

	
	NEXUS_Memory_Allocate(bufLen, NULL, (void *)&pEncSrc);
	NEXUS_Memory_Allocate(bufLen, NULL, (void *)&pEncDest);
	BKNI_Memset((void *)pEncSrc,  0, bufLen);
	BKNI_Memset((void *)pEncDest, 0, bufLen);

	printf("bufLen is %d\n", bufLen);
	size = fread(pEncSrc, 1, bufLen, dataFile);
	if (size < bufLen)
	{
		printf( "%s does not have %d bytes for MAC calculation.\n", argv[1], bufLen );
		return 0;
	}




	/* Allocate  keyslot for encryption */
    NEXUS_Security_GetDefaultKeySlotSettings(&keySettings);
    keySettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
    encKeyHandle = NEXUS_Security_AllocateKeySlot(&keySettings);
    if(!encKeyHandle) 
	{
        printf("\nAllocate encryption keyslot failed \n");
        return 1;
    }

	/* configure the key slot and set up the key ladder for encryption */
	if (keySlotSetup(encKeyHandle, NEXUS_SecurityOperation_eEncrypt))
	{
		printf("\nKeyslot setup for encryption failed \n");
		return 1;
	}
	

	/* call M2M_operation() to encrypt the string */
	if (M2M_operation(encKeyHandle,
	                   pEncDest,
	                   bufLen,
	                   ivkeys,
	                   sizeof(ivkeys),
	                   pEncSrc,
	                   bufLen))
	{
		printf("\nError performing encryption! \n");
		return 1;
	}

	printf("Source data   : ");
	for (i = 0; i < bufLen; i++)
		printf(" %2x", pEncSrc[i]);
	printf("\n");
	printf("Encrypted data: ");
	for (i = 0; i < bufLen; i++)
		printf(" %2x", pEncDest[i]);
	printf("\n");
	printf("MAC is : ");
	if (bufLen < 16)
		for (i = 0; i < bufLen; i++)
			printf(" %2x", pEncDest[i]);
	else
		for (i = bufLen - 16; i < bufLen; i++)
			printf(" %2x", pEncDest[i]);

	printf("\n");

	NEXUS_Security_FreeKeySlot(encKeyHandle);
	NEXUS_Memory_Free(pEncSrc);
	NEXUS_Memory_Free(pEncDest);

	return (0);

	
}


#else /* NEXUS_HAS_SECURITY */
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform!\n");
    return -1;
}
#endif

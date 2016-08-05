/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * THE SOURCE CODE CONTAINS BROADCOM HIGHLY CONFIDENTIAL
 * INFORMATION AND MUST BE HANDLED AS AGREED UPON IN THE
 * HIGHLY CONFIDENTIAL SOFTWARE LICENSE AGREEMENT (HC-SLA).
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

#if NEXUS_HAS_SECURITY  && (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 4)

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


/* Data to be customized for Global Key Derivation on the tested chip. */

#define KL_ALGO               (NEXUS_SecurityKeyladderType_eAes128)
#define KL_OPERATION          (NEXUS_SecurityOperation_eDecrypt)
#define GLB_KEY_OWNER_ID      (NEXUS_SecurityGlobalKeyOwnerID_eUse1)
#define GLB_KEY_INDEX         (0)
#define CAVENDOR_ID           (0x1234)
#define STB_OWNER_ID          (NEXUS_SecurityOtpId_eOneVal)
#define ASKM_MODULE_ID        (NEXUS_SecurityAskmModuleID_eModuleID_10)
#define CUSTOMER_SUBMODE      (NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_5)

uint8_t finalSWKey[16] =            {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                                     0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};

uint8_t ucProcInKey3[16] =          {0x22, 0x22, 0x22, 0x22, 0x33, 0x33, 0x33, 0x33,
                                     0x44, 0x44, 0x44, 0x44, 0x55, 0x55, 0x55, 0x55};

uint8_t ucProcInKey4[16] =          {0x33, 0x33, 0x33, 0x33, 0x44, 0x44, 0x44, 0x44,
                                     0x55, 0x55, 0x55, 0x55, 0x66, 0x66, 0x66, 0x66 };

uint8_t ucProcInKey5[16] =          {0x44, 0x44, 0x44, 0x44, 0x55, 0x55, 0x55, 0x55,
                                     0x66, 0x66, 0x66, 0x66, 0x77, 0x77, 0x77, 0x77};




static void CompleteCallback(void *pParam, int iParam){
    BSTD_UNUSED(iParam);
    fprintf(stderr, "CompleteCallback:%#lx fired\n", (unsigned long)pParam);
    BKNI_SetEvent(pParam);
}


static void PrintBlock(char * desc, uint8_t *pData)
{
unsigned int i;
    printf("%s",desc);
    for(i=0; i < DMA_BLOCK; i++)
    {
        printf( " %x ", pData[i]);
    }
    printf( "\n\n");
}

static int Security_AllocateVkl ( NEXUS_SecurityVirtualKeyladderID * vkl, NEXUS_VirtualKeyLadderHandle * pVklHandle )
{
    NEXUS_SecurityVKLSettings vklSettings;
    NEXUS_VirtualKeyLadderInfo vklInfo;
    NEXUS_VirtualKeyLadderHandle vklHandle;

    BDBG_ASSERT ( vkl );

    NEXUS_Security_GetDefaultVKLSettings ( &vklSettings );

    /* For pre Zeus 3.0, please set vklSettings.custSubMode */
    vklHandle = NEXUS_Security_AllocateVKL ( &vklSettings );

    if ( !vklHandle )
    {
        printf ( "\nAllocate VKL failed \n" );
        return 1;
    }

    NEXUS_Security_GetVKLInfo ( vklHandle, &vklInfo );
    printf ( "\nVKL Handle %p is allocated for VKL#%d\n", ( void * ) vklHandle, vklInfo.vkl & 0x7F );

    *vkl = vklInfo.vkl;
    *pVklHandle = vklHandle;

    return 0;
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
    BKNI_EventHandle        dmaEvent = NULL;

    NEXUS_SecurityKeySlotSettings keySettings;
    NEXUS_KeySlotHandle                 clrKeyHandle, encKeyHandle;
    NEXUS_SecurityAlgorithmSettings     NexusConfig;
    NEXUS_SecurityClearKey key;
    NEXUS_SecurityEncryptedSessionKey encryptedSessionkey;
    NEXUS_SecurityEncryptedControlWord encrytedCW;
    NEXUS_SecurityEncryptedKey encKey;
    NEXUS_SecurityVirtualKeyladderID vklId;
    NEXUS_VirtualKeyLadderHandle vklHandle;

    uint8_t *pSrc, *pDest, *pDest2;
    unsigned int i;


    /* Platform init */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);


    /* request for an VKL to use */
    if ( Security_AllocateVkl ( &vklId, &vklHandle ) )
    {
        printf ( "\nAllocate VKL failed.\n" );
        return 1;
    }

	/* Brief descriptions of the tests:
	   keyslot clrKeyHandle is used to encrypt data pSrc[] to pDest[] using clear key finalSWKey[].
       keyslot encKeyHandle is used to setup the a key ladder for three stage key unwrapping of the key for encryption: key3->key4 ->key5,  with route key5 using the related procin[]s.   The routed key is used to encrypt pSrc[] to pDest2[]. The app compares if the encryption results of pDest[] matches pDest2[].

	   The app then reconfigures keyslot  encKeyHandle for decryption, using the key ladder key4->key5 with the related procin[]s, and routing key5 to decrypt pDest2[] to pDest[].
	   The test then compares the decryption results pDest[] matches pSrc[].
     */

    /* Allocate AV keyslots */
    NEXUS_Security_GetDefaultKeySlotSettings(&keySettings);
    keySettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
    clrKeyHandle = NEXUS_Security_AllocateKeySlot(&keySettings);
    if(!clrKeyHandle)
    {
        printf("\nAllocate keyslot failed \n");
        return 1;
    }

    NEXUS_Security_GetDefaultKeySlotSettings(&keySettings);
    keySettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
    encKeyHandle = NEXUS_Security_AllocateKeySlot(&keySettings);
    if(!encKeyHandle)
    {
        printf("\nAllocate keyslot failed \n");
        return 1;
    }

    /* Set up key for clear key */
    NEXUS_Security_GetDefaultAlgorithmSettings(&NexusConfig);
    NexusConfig.algorithm           = NEXUS_SecurityAlgorithm_eAes;
    NexusConfig.algorithmVar        = NEXUS_SecurityAlgorithmVariant_eEcb;

    /* ++++++++ */
    NexusConfig.terminationMode     = NEXUS_SecurityTerminationMode_eClear;
    NexusConfig.ivMode              = NEXUS_SecurityIVMode_eRegular;
    NexusConfig.solitarySelect      = NEXUS_SecuritySolitarySelect_eClear;
    NexusConfig.caVendorID          = CAVENDOR_ID;
    NexusConfig.askmModuleID        = ASKM_MODULE_ID;
    NexusConfig.otpId               = STB_OWNER_ID;
    /* -------- */
    NexusConfig.key2Select          = NEXUS_SecurityKey2Select_eFixedKey;
    NexusConfig.dest = NEXUS_SecurityAlgorithmConfigDestination_eCps;
    /* -------- */
    /* ++++++++ */

    NexusConfig.operation           = NEXUS_SecurityOperation_eEncrypt;
    NexusConfig.keyDestEntryType    = NEXUS_SecurityKeyType_eClear;

    if ( NEXUS_Security_ConfigAlgorithm(clrKeyHandle, &NexusConfig)!= 0)
    {
        printf("\nConfigAlg clear keyslot failed \n");
        return 1;
    }

    NEXUS_Security_GetDefaultClearKey(&key);
    key.keyEntryType = NEXUS_SecurityKeyType_eClear;
    key.keySize = sizeof(finalSWKey);
    key.keyIVType    = NEXUS_SecurityKeyIVType_eNoIV;
    BKNI_Memcpy(key.keyData, finalSWKey, sizeof(finalSWKey));
    if (NEXUS_Security_LoadClearKey(clrKeyHandle, &key) != 0) {
        printf("\nLoad encryption key failed \n");
        return 1;
    }

    /* Set up key for keyladder  */
    if ( NEXUS_Security_ConfigAlgorithm(encKeyHandle, &NexusConfig)!= 0)
    {
        printf("\nConfigAlg keyladder keyslot failed \n");
        return 1;
    }

    /* Load session key */
    NEXUS_Security_GetDefaultSessionKeySettings(&encryptedSessionkey);
    encryptedSessionkey.keyladderID     = NEXUS_SecurityKeyladderID_eA;
    encryptedSessionkey.keyladderType   = KL_ALGO;
    encryptedSessionkey.swizzleType     = NEXUS_SecuritySwizzleType_eNone;

    /* -------- */
    encryptedSessionkey.keyGenCmdID     = NEXUS_SecurityKeyGenCmdID_eKeyGen;
    encryptedSessionkey.sessionKeyOp    = NEXUS_SecuritySessionKeyOp_eNoProcess;
    encryptedSessionkey.bASKMMode       = true;

    encryptedSessionkey.rootKeySrc          = NEXUS_SecurityRootKeySrc_eGlobalKey;
    encryptedSessionkey.globalKeyOwnerId    = GLB_KEY_OWNER_ID;
    encryptedSessionkey.askmGlobalKeyIndex  = GLB_KEY_INDEX;

    encryptedSessionkey.bSwapAESKey     = false;
    encryptedSessionkey.keyDestIVType   = NEXUS_SecurityKeyIVType_eNoIV;
    /* -------- */
    encryptedSessionkey.bRouteKey       = false;
    encryptedSessionkey.operation       = KL_OPERATION;
    encryptedSessionkey.operationKey2   = NEXUS_SecurityOperation_eEncrypt;
    encryptedSessionkey.keyEntryType    = NEXUS_SecurityKeyType_eClear;

    /* ++++++++ */
    encryptedSessionkey.custSubMode        = CUSTOMER_SUBMODE;
    encryptedSessionkey.virtualKeyLadderID = vklId;
    encryptedSessionkey.keyMode            = NEXUS_SecurityKeyMode_eRegular;
    /* ++++++++ */

    BKNI_Memcpy(encryptedSessionkey.keyData, ucProcInKey3, sizeof(ucProcInKey3));

    printf("\nRoot Key SRC %d \n", encryptedSessionkey.rootKeySrc);

    if (NEXUS_Security_GenerateSessionKey(encKeyHandle, &encryptedSessionkey) !=0)
    {
        printf("\nLoading session key failed \n");
        return 1;
    }

    /* Generate Key 4 */
    NEXUS_Security_GetDefaultControlWordSettings(&encrytedCW);
    encKey.keyladderID = NEXUS_SecurityKeyladderID_eA;
    encKey.keyladderType = KL_ALGO;
    encKey.keySize = sizeof(ucProcInKey4);
    encKey.keyEntryType = NEXUS_SecurityKeyType_eClear;

    encKey.custSubMode        = NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_5;
    encKey.virtualKeyLadderID = vklId;
    encKey.keyMode            = NEXUS_SecurityKeyMode_eRegular;
    encKey.bASKMMode          = true;
    encKey.operation          = KL_OPERATION;

    encKey.keyDestIVType = NEXUS_SecurityKeyIVType_eNoIV;
    encKey.keyGenCmdID	 = NEXUS_SecurityKeyGenCmdID_eKeyGen;
    encKey.bSwapAESKey	 = false;

    BKNI_Memcpy(encKey.keyData, ucProcInKey4, encKey.keySize);
    encKey.keylayer = NEXUS_SecurityKeyLayer_eKey4;
    encKey.bRouteKey = false;
    if (NEXUS_Security_GenerateNextLayerKey(encKeyHandle, &encKey))
    {
        printf("\nKey 4 Generation Failed\n");
        return 1;
    }

    /* Generate Key 5 */

    BKNI_Memcpy(encKey.keyData, ucProcInKey5, encKey.keySize);
    encKey.keylayer = NEXUS_SecurityKeyLayer_eKey5;
    encKey.bRouteKey = true;

    if (NEXUS_Security_GenerateNextLayerKey(encKeyHandle, &encKey))
    {
        printf("\nKey 5 Generation Failed\n");
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

    printf("\nBuffers Before::\n");

    PrintBlock("Src Buffer           ",pSrc);
    PrintBlock("Enc with Sw Key      ",pDest);
    PrintBlock("Enc with Deriverd Key",pDest2);

    NEXUS_DmaJob_GetDefaultSettings(&jobSettings);
    jobSettings.numBlocks   = 1;
    jobSettings.keySlot     = clrKeyHandle;
    jobSettings.dataFormat  = NEXUS_DmaDataFormat_eBlock;

    jobSettings.completionCallback.callback = CompleteCallback;
    jobSettings.completionCallback.context	= dmaEvent;

    dmaJob = NEXUS_DmaJob_Create(dma, &jobSettings);

    NEXUS_DmaJob_GetDefaultBlockSettings(&blockSettings);
    blockSettings.pSrcAddr                  = pSrc;
    blockSettings.pDestAddr                 = pDest;
    blockSettings.blockSize                 = DMA_BLOCK;
    blockSettings.resetCrypto               = true;
    blockSettings.scatterGatherCryptoStart  = true;
    blockSettings.scatterGatherCryptoEnd    = true;
    blockSettings.cached                    = true;

    rc = NEXUS_DmaJob_ProcessBlocks(dmaJob, &blockSettings, 1);

    if (rc == NEXUS_DMA_QUEUED )
    {
        BKNI_WaitForEvent(dmaEvent, BKNI_INFINITE);
        NEXUS_DmaJob_GetStatus(dmaJob, &jobStatus);
        BDBG_ASSERT(jobStatus.currentState == NEXUS_DmaJobState_eComplete);
    }

    NEXUS_DmaJob_Destroy (dmaJob);


    NEXUS_DmaJob_GetDefaultSettings(&jobSettings);
    jobSettings.numBlocks                   = 1;
    jobSettings.keySlot                     = encKeyHandle;
    jobSettings.dataFormat                  = NEXUS_DmaDataFormat_eBlock;
    jobSettings.completionCallback.callback = CompleteCallback;
    jobSettings.completionCallback.context	= dmaEvent;
    dmaJob = NEXUS_DmaJob_Create(dma, &jobSettings);

    NEXUS_DmaJob_GetDefaultBlockSettings(&blockSettings);
    blockSettings.pSrcAddr                  = pSrc;
    blockSettings.pDestAddr                 = pDest2;
    blockSettings.blockSize                 = DMA_BLOCK;
    blockSettings.resetCrypto               = true;
    blockSettings.scatterGatherCryptoStart  = true;
    blockSettings.scatterGatherCryptoEnd    = true;
    blockSettings.cached                    = true;

    rc = NEXUS_DmaJob_ProcessBlocks(dmaJob, &blockSettings, 1);
    if (rc == NEXUS_DMA_QUEUED )
    {
        BKNI_WaitForEvent(dmaEvent, BKNI_INFINITE);
        NEXUS_DmaJob_GetStatus(dmaJob, &jobStatus);
        BDBG_ASSERT(jobStatus.currentState == NEXUS_DmaJobState_eComplete);
    }
    NEXUS_DmaJob_Destroy (dmaJob);

    printf(" 2) ... %x %x %x\n",pDest[0],pSrc[0], pDest2[0]);
    printf("\nBuffers After::\n");

    PrintBlock("Src Buffer           ",pSrc);
    PrintBlock("Enc with Sw Key      ",pDest);
    PrintBlock("Enc with Deriverd Key",pDest2);


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
    {
        printf ("\nEncryption Test passed\n\n\n");
    }
    else
    {
        printf ("\nEncryption Test failed\n\n\n");
        return 1;
    }


    /* Change the algorithm for Decryption. */
    NexusConfig.operation   = NEXUS_SecurityOperation_eDecrypt;
    NexusConfig.dest = NEXUS_SecurityAlgorithmConfigDestination_eCpd;

    /* Set up key for  */
    if ( NEXUS_Security_ConfigAlgorithm(encKeyHandle, &NexusConfig)!= 0)
    {
        printf("\nConfigAlg keyladder keyslot failed \n");
        return 1;
    }

    /* Generate Key 4 */
    NEXUS_Security_GetDefaultControlWordSettings(&encrytedCW);
    encKey.keyladderID = NEXUS_SecurityKeyladderID_eA;
    encKey.keyladderType = KL_ALGO;
    encKey.keySize = sizeof(ucProcInKey4);
    encKey.keyEntryType = NEXUS_SecurityKeyType_eClear;

    encKey.custSubMode        = NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_5;
    encKey.virtualKeyLadderID = vklId;
    encKey.keyMode            = NEXUS_SecurityKeyMode_eRegular;
    encKey.bASKMMode          = true;
    encKey.operation          = NEXUS_SecurityOperation_eDecrypt;

    encKey.keyDestIVType = NEXUS_SecurityKeyIVType_eNoIV;
    encKey.keyGenCmdID   = NEXUS_SecurityKeyGenCmdID_eKeyGen;
    encKey.bSwapAESKey   = false;

    BKNI_Memcpy(encKey.keyData, ucProcInKey4, encKey.keySize);
    encKey.keylayer = NEXUS_SecurityKeyLayer_eKey4;
    encKey.bRouteKey = false;

    if (NEXUS_Security_GenerateNextLayerKey(encKeyHandle, &encKey))
    {
        printf("\nKey 4 Generation Failed\n");
        return 1;
    }

    /* Generate Key 5 */
    BKNI_Memcpy(encKey.keyData, ucProcInKey5, encKey.keySize);
    encKey.keylayer = NEXUS_SecurityKeyLayer_eKey5;
    encKey.bRouteKey = true;

    if (NEXUS_Security_GenerateNextLayerKey(encKeyHandle, &encKey))
    {
        printf("\nKey 5 Generation Failed\n");
        return 1;
    }

    memset(pDest, 0, DMA_BLOCK*sizeof(unsigned char));
    NEXUS_DmaJob_GetDefaultSettings(&jobSettings);
    jobSettings.numBlocks                   = 1;
    jobSettings.keySlot                     = encKeyHandle;
    jobSettings.dataFormat                  = NEXUS_DmaDataFormat_eBlock;
    jobSettings.completionCallback.callback = CompleteCallback;
    jobSettings.completionCallback.context	= dmaEvent;

    dmaJob = NEXUS_DmaJob_Create(dma, &jobSettings);
    NEXUS_DmaJob_GetDefaultBlockSettings(&blockSettings);
    blockSettings.pSrcAddr                  = pDest2;
    blockSettings.pDestAddr                 = pDest;
    blockSettings.blockSize                 = DMA_BLOCK;
    blockSettings.resetCrypto               = true;
    blockSettings.scatterGatherCryptoStart  = true;
    blockSettings.scatterGatherCryptoEnd    = true;
    blockSettings.cached                    = true;

    rc = NEXUS_DmaJob_ProcessBlocks(dmaJob, &blockSettings, 1);
    if (rc == NEXUS_DMA_QUEUED )
    {
        BKNI_WaitForEvent(dmaEvent, BKNI_INFINITE);
        NEXUS_DmaJob_GetStatus(dmaJob, &jobStatus);
        BDBG_ASSERT(jobStatus.currentState == NEXUS_DmaJobState_eComplete);
    }

    NEXUS_DmaJob_Destroy (dmaJob);

    PrintBlock("Src Buffer           ",pSrc);
    PrintBlock("Dec with Driverd Key ",pDest);
    PrintBlock("Enc with Deriverd Key",pDest2);

    /* Make sure pSrc matches pDest 2 */
    for (i=0;i<DMA_BLOCK; i++ )
    {
        if (pDest[i] != pSrc[i])
        {
            printf ("\nComparison failed at location %d\n", i);
            break;
        }
    }

    if ( i==DMA_BLOCK )
    {
        printf ("\nDecryption Test passed\n");
    }
    else
    {
        printf ("\nDecryption Test Failed\n");
        return 1;
    }

    BKNI_DestroyEvent(dmaEvent);
    NEXUS_Dma_Close(dma);

    NEXUS_Security_FreeKeySlot(clrKeyHandle);
    NEXUS_Security_FreeKeySlot(encKeyHandle);
    NEXUS_Security_FreeVKL ( vklHandle );

    NEXUS_Platform_Uninit (  );

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

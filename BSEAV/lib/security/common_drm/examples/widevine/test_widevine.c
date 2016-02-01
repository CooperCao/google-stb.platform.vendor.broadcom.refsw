/***************************************************************************
 *    (c)2010-2014 Broadcom Corporation
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 * 
 * Module Description:
 * 
 * Revision History:
 * 
 * $brcm_Log: $
 * 
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "nexus_platform.h"
#include "nexus_memory.h"
#include "drm_widevine.h"
#include "drm_metadata.h"
#include "drm_common.h"

#include "bstd.h"
#include "bdbg.h" 
#include "bkni.h"

/*************************** AES-CBC
TEST VECTOR [1] length(16)
KEY: 4b72bcee7aa629a4a96a67fa942514d8
INPUT: 44cf1e4aa75002d9918955db4dabb016
ENCRYPTED: 32b8af8a0ebf24c5a51341f3345faee0
DECRYPTED: 44cf1e4aa75002d9918955db4dabb016
************************************************/
uint8_t short_key[16] = {0x4b, 0x72, 0xbc, 0xee, 0x7a, 0xa6, 0x29, 0xa4, 0xa9, 0x6a, 0x67, 0xfa, 0x94, 0x25, 0x14, 0xd8 };
uint8_t short_data[16] = {0x32, 0xb8, 0xaf, 0x8a, 0x0e, 0xbf, 0x24, 0xc5, 0xa5, 0x13, 0x41, 0xf3, 0x34, 0x5f, 0xae, 0xe0};
uint8_t short_Wrapped_ECM[32] = {0x91, 0x6c, 0x20, 0xab, 0x08, 0xaa, 0x9b, 0xcf,
						0x94, 0x6b, 0x40, 0x14, 0xca, 0xf9, 0x6f, 0x6c,
						0x10, 0xd6, 0x19, 0x92, 0x14, 0x17, 0x79, 0x00,
						0xac, 0x54, 0x37, 0x6b, 0x90, 0xb9, 0xfd, 0x66};


/****************  AES solitary short block
TEST VECTOR [12] length(2)
KEY: e488a6d6d6097525161ad886d001acbc
INPUT: a2c8
ENCRYPTED: a2c8
DECRYPTED: a2c8
***************************************** */
uint8_t solitary_key[16] = {0xe4, 0x88, 0xa6, 0xd6, 0xd6, 0x09, 0x75, 0x25, 0x16, 0x1a, 0xd8, 0x86, 0xd0, 0x01, 0xac, 0xbc };
uint8_t solitary_data[2] = {0xa2, 0xc8};
uint8_t solitray_Wrapped_ECM[32] = {0xfa, 0x15, 0x3a, 0x50, 0x2d, 0xe8, 0x79, 0x5c,
									0x1d, 0x68, 0xb4, 0x50, 0x97, 0xbd, 0x45, 0x5e,
									0xf1, 0x7b, 0x1c, 0xd4, 0x85, 0x97, 0x73, 0x0d,
									0x10, 0x8a, 0x04, 0xd2, 0x86, 0xc7, 0x42, 0xe8};

/* CTS residual block
TEST VECTOR [23] length(33)
KEY: 2ebb84ec272e2a6baeb9ce1d9d860505
INPUT: 621c751a4088c9c3d8a295971d0cfcc8c9b0f5e0dc67b06ba45cacc92b9673004c
ENCRYPTED: 98d3a023589d007e99c577a1764b0f1771a0ae4a548c89b26b1ca04e0775dfc839
DECRYPTED: 621c751a4088c9c3d8a295971d0cfcc8c9b0f5e0dc67b06ba45cacc92b9673004c
**************************************************** * */
uint8_t residual_key[16] = {0x2e, 0xbb, 0x84, 0xec, 0x27, 0x2e, 0x2a, 0x6b, 0xae, 0xb9, 0xce, 0x1d, 0x9d, 0x86, 0x05, 0x05 };
uint8_t residual_data[33] = {0x98, 0xd3, 0xa0, 0x23, 0x58, 0x9d, 0x00, 0x7e,
							0x99, 0xc5, 0x77, 0xa1,  0x76, 0x4b, 0x0f, 0x17,
							0x71, 0xa0, 0xae, 0x4a, 0x54, 0x8c, 0x89, 0xb2,
							0x6b, 0x1c, 0xa0, 0x4e, 0x07, 0x75, 0xdf, 0xc8,
							0x39};
uint8_t residual_Wrapped_ECM[32] = {0x21, 0x0c, 0xf0, 0x6b, 0x0d, 0xb0, 0xcd, 0x79,
									0x56, 0xae, 0x9e, 0x31, 0xd3, 0x3c, 0x98, 0xff,
									0xb7, 0x38, 0x6a, 0xa3, 0xb0, 0xe6, 0xad, 0xc1,
									0xb5, 0x56, 0xfc, 0x11, 0xdc, 0x89, 0x74, 0xc9};

/* Reference CNN variables */
static void test_vector(uint8_t *pData, uint32_t size, uint8_t *pKey);
static void test_vector_crypto(uint8_t *pData, uint32_t size, uint8_t *pKey);
static void generate_wrapped_key(uint8_t *pKey);
static void test_fixed_decryption(uint8_t *pEcm, uint8_t* pData, uint32_t size);

int main(int argc, char* argv[])
{
 int i = 0;
 NEXUS_PlatformSettings platformSettings;
 DRM_Widevine_Init_t inputWvInitStruct;
 BSTD_UNUSED(argc);


   /* Call Platform Init */
   NEXUS_Platform_GetDefaultSettings(&platformSettings); 
   platformSettings.openFrontend = false;
   NEXUS_Platform_Init(&platformSettings);

   /* Set input WV structure params */
   inputWvInitStruct.key_file = argv[1];
   inputWvInitStruct.mode = DRM_WidevineDecryptMode_eVod;

   DRM_Widevine_Initialize(inputWvInitStruct);

   generate_wrapped_key(short_key);

   generate_wrapped_key(solitary_key);
   generate_wrapped_key(residual_key);


   /*test_vector(short_data, sizeof(short_data), short_key);*/
   for(i = 0; i< 1;i++)
   {
   test_vector_crypto(short_data, sizeof(short_data), short_key);

	   /*test_vector(solitary_data, sizeof(solitary_data), solitary_key);*/
	   test_vector_crypto(solitary_data, sizeof(solitary_data), solitary_key);

	   /*test_vector(residual_data, sizeof(residual_data), residual_key);*/
	   test_vector_crypto(residual_data, sizeof(residual_data), residual_key);

	   test_fixed_decryption(short_Wrapped_ECM, short_data, 16 );
	   test_fixed_decryption(solitray_Wrapped_ECM, solitary_data, sizeof(solitary_data));
	   test_fixed_decryption(residual_Wrapped_ECM, residual_data, sizeof(residual_data));
	   DRM_Widevine_Finalize();
   }
    return 0;
}


void test_fixed_decryption(uint8_t *pEcm, uint8_t* pData, uint32_t size)
{
	unsigned int i = 0;
	uint8_t *dmaBuf_Dst = NULL;
	uint8_t iv_0[16] = {0x00};
    uint8_t pFixedKey[16] = {0xc7, 0x97, 0x93, 0x11, 0xfe, 0xee, 0x04, 0x68,
    							0x30, 0x58, 0x29, 0xf3, 0x74, 0xfa, 0x98, 0x4e};


	uint8_t Id[4] = {0x00};

	DRM_Widevine_LoadCwFixed(pFixedKey,	pEcm, Id, NEXUS_SecurityKeyType_eOdd);

    printf("ID Result:\n");
    for(i = 0; i< 4;i++)
    {
    	printf("%02x ", Id[i]);
    }
    printf("\n\n");


    NEXUS_Memory_Allocate(size, NULL,(void **) &dmaBuf_Dst);

    BKNI_Memcpy(dmaBuf_Dst, pData, size);

    DRM_Widevine_Decrypt(dmaBuf_Dst,
    					size,
                        dmaBuf_Dst,
                        iv_0,
                        DrmDestinationType_eExternal,
                        NEXUS_SecurityKeyType_eOdd);

    printf("Decryption Result:\n");
    for(i = 0; i< size;i++)
    {
    	printf("%02x ", dmaBuf_Dst[i]);
    }
    printf("\n\n");
	return;
}


void generate_wrapped_key(uint8_t *pKey)
{
    uint8_t  *dmaBuf_Src = NULL;
    uint8_t  *dmaBuf_Dst = NULL;
    int i = 0;
    uint8_t wrapper_key[16] = {0xc3, 0x97, 0x12, 0x47, 0xd2, 0xea, 0x4b, 0x4b,
    		0x6a, 0x41, 0xf5, 0x5c, 0x85, 0x66, 0x75, 0x1a};
    uint8_t iv_0[16] = {0x00};
   DmaBlockInfo_t dmaBlock;
   DrmCommonOperationStruct_t drmCommonOpStruct;

    /*allocate a continuous physical address buffer for DMA */
    NEXUS_Memory_Allocate(32, NULL,(void **) &dmaBuf_Src);
    NEXUS_Memory_Allocate(32, NULL,(void **) &dmaBuf_Dst);

    /*copy content to DMA buffer*/
    BKNI_Memset(dmaBuf_Src, 0xAA, 32);
    BKNI_Memcpy(&dmaBuf_Src[4], pKey, 16);

    /* set DMA parameters */
    dmaBlock.pSrcData = dmaBuf_Src;
    dmaBlock.pDstData = dmaBuf_Dst;
    dmaBlock.uiDataSize = 32;
    dmaBlock.sg_start = true;
    dmaBlock.sg_end = true;

    printf("Source:\n");
    for(i = 0; i< 32;i++)
    {
    	printf("%02x ", dmaBuf_Src[i]);
    }
    printf("\n\n");

    drmCommonOpStruct.keyConfigSettings.settings.opType    = NEXUS_SecurityOperation_eEncrypt;
    drmCommonOpStruct.keyConfigSettings.settings.algType   = NEXUS_SecurityAlgorithm_eAes;
    drmCommonOpStruct.keyConfigSettings.settings.algVariant = NEXUS_SecurityAlgorithmVariant_eCbc;
    drmCommonOpStruct.keySrc    = CommonCrypto_eClearKey;
    BKNI_Memcpy(drmCommonOpStruct.keyIvSettings.key, wrapper_key, 16);
    drmCommonOpStruct.keyIvSettings.keySize  = 16;
    BKNI_Memcpy(drmCommonOpStruct.keyIvSettings.iv, iv_0, 16);
    drmCommonOpStruct.keyIvSettings.ivSize = 16;

    drmCommonOpStruct.pKeyLadderInfo = NULL;
    drmCommonOpStruct.keyConfigSettings.settings.keySlotType = NEXUS_SecurityKeyType_eOdd;
    drmCommonOpStruct.keyConfigSettings.settings.termMode = NEXUS_SecurityTerminationMode_eClear;
    drmCommonOpStruct.pDmaBlock = &dmaBlock;
    drmCommonOpStruct.num_dma_block = 1;

    DRM_Common_OperationDma(&drmCommonOpStruct);


    printf("Wrapped key Result:\n");
    for(i = 0; i< 32;i++)
    {
    	printf("%02x ", dmaBuf_Dst[i]);
    }
    printf("\n\n");

	return;
}







void test_vector(uint8_t *pData, uint32_t size, uint8_t *pKey)
{
    uint8_t  *dmaBuf_Src = NULL;
    uint8_t  *dmaBuf_Dst = NULL;
    uint32_t i = 0;
    uint8_t iv_0[16] = {0x00};
   DmaBlockInfo_t dmaBlock;
   DrmCommonOperationStruct_t drmCommonOpStruct;

    /*allocate a continuous physical address buffer for DMA */
    NEXUS_Memory_Allocate(size, NULL,(void **) &dmaBuf_Src);
    NEXUS_Memory_Allocate(size, NULL,(void **) &dmaBuf_Dst);

    /*copy content to DMA buffer*/
    BKNI_Memcpy(dmaBuf_Src, pData, size);

    /* set DMA parameters */
    dmaBlock.pSrcData = dmaBuf_Src;
    dmaBlock.pDstData = dmaBuf_Dst;
    dmaBlock.uiDataSize = size;
    dmaBlock.sg_start = true;
    dmaBlock.sg_end = true;

    printf("Source:\n");
    for(i = 0; i< size;i++)
    {
    	printf("%02x ", dmaBuf_Src[i]);
    }
    printf("\n\n");

    BKNI_Memset( (uint8_t*)&drmCommonOpStruct, 0x00, sizeof(DrmCommonOperationStruct_t));
    drmCommonOpStruct.keyConfigSettings.settings.opType    = NEXUS_SecurityOperation_eDecrypt;
    drmCommonOpStruct.keyConfigSettings.settings.algType   = NEXUS_SecurityAlgorithm_eAes;
    drmCommonOpStruct.keyConfigSettings.settings.algVariant = NEXUS_SecurityAlgorithmVariant_eCbc;
    drmCommonOpStruct.keySrc    = CommonCrypto_eClearKey;
    BKNI_Memcpy(drmCommonOpStruct.keyIvSettings.key, pKey, 16);
    drmCommonOpStruct.keyIvSettings.keySize  = 16;
    BKNI_Memcpy(drmCommonOpStruct.keyIvSettings.iv, iv_0, 16);
    drmCommonOpStruct.keyIvSettings.ivSize = 16;

    drmCommonOpStruct.pKeyLadderInfo = NULL;
    drmCommonOpStruct.keyConfigSettings.settings.keySlotType = NEXUS_SecurityKeyType_eOdd;
    drmCommonOpStruct.keyConfigSettings.settings.termMode = NEXUS_SecurityTerminationMode_eClear;
    drmCommonOpStruct.pDmaBlock = &dmaBlock;
    drmCommonOpStruct.num_dma_block = 1;

    DRM_Common_OperationDma(&drmCommonOpStruct);

    printf("Result:\n");
    for(i = 0; i< size;i++)
    {
    	printf("%02x ", dmaBuf_Dst[i]);
    }
    printf("\n\n");

	return;
}





void test_vector_crypto(uint8_t *pData, uint32_t size, uint8_t *pKey)
{
    uint8_t  *dmaBuf_Src = NULL;
    uint8_t  *dmaBuf_Dst = NULL;
    uint32_t i = 0;
    uint8_t iv_0[16] = {0x00};

   DmaBlockInfo_t dmaBlock;
   DrmCommonOperationStruct_t drmCommonOpStruct;

    /*allocate a continuous physical address buffer for DMA */
    NEXUS_Memory_Allocate(size, NULL,(void **) &dmaBuf_Src);
    NEXUS_Memory_Allocate(size, NULL,(void **) &dmaBuf_Dst);

    /*copy content to DMA buffer*/
    BKNI_Memcpy(dmaBuf_Src, pData, size);

    /* set DMA parameters */
    dmaBlock.pSrcData = dmaBuf_Src;
    dmaBlock.pDstData = dmaBuf_Dst;
    dmaBlock.uiDataSize = size;
    dmaBlock.sg_start = true;
    dmaBlock.sg_end = true;

    printf("Source:\n");
    for(i = 0; i< size;i++)
    {
    	printf("%02x ", dmaBuf_Src[i]);
    }
    printf("\n\n");


    /* set DMA parameters */
    dmaBlock.pSrcData = dmaBuf_Src;
    dmaBlock.pDstData = dmaBuf_Dst;
    dmaBlock.uiDataSize = size;
    dmaBlock.sg_start = true;
    dmaBlock.sg_end = true;

    BKNI_Memset( (uint8_t*)&drmCommonOpStruct, 0x00, sizeof(DrmCommonOperationStruct_t));
    drmCommonOpStruct.keyConfigSettings.settings.opType    = NEXUS_SecurityOperation_eDecrypt;
    drmCommonOpStruct.keyConfigSettings.settings.algType   = NEXUS_SecurityAlgorithm_eAes;
    drmCommonOpStruct.keyConfigSettings.settings.algVariant = NEXUS_SecurityAlgorithmVariant_eCbc;
    drmCommonOpStruct.keyConfigSettings.settings.keySlotType = NEXUS_SecurityKeyType_eOdd;
    drmCommonOpStruct.keyConfigSettings.settings.termMode = NEXUS_SecurityTerminationMode_eCtsCpcm;

    drmCommonOpStruct.keySrc    = CommonCrypto_eClearKey;
    BKNI_Memcpy(drmCommonOpStruct.keyIvSettings.key, pKey, 16);
    drmCommonOpStruct.keyIvSettings.keySize  = 16;
    BKNI_Memcpy(drmCommonOpStruct.keyIvSettings.iv, iv_0, 16);
    drmCommonOpStruct.keyIvSettings.ivSize = 16;

    drmCommonOpStruct.pKeyLadderInfo = NULL;
    drmCommonOpStruct.pDmaBlock = &dmaBlock;
    drmCommonOpStruct.num_dma_block = 1;

    DRM_Common_OperationDma(&drmCommonOpStruct);

    printf("Result:\n");
    for(i = 0; i< size;i++)
    {
    	printf("%02x ", dmaBuf_Dst[i]);
    }
    printf("\n\n");

	return;
}



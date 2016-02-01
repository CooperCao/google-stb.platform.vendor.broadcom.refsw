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
#if NEXUS_HAS_SECURITY && (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 1)

#include "nexus_platform.h"
#include "nexus_dma.h"
#include "nexus_memory.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "nexus_security.h"
#include "nexus_bsp_config.h"

#define DATA_BLOCK_SIZE   (400)
#define DMA_JOBS    1


#define DEBUG_PRINT_ARRAY(description_txt,in_size,in_ptr) { int x_offset;       \
            printf("[%s][%d]", description_txt, in_size );                      \
            for( x_offset = 0; x_offset < (int)(in_size); x_offset++ )          \
            {                                                                   \
                if( x_offset%16 == 0 ) printf("\n");                            \
                                                                                \
                printf("%02X ", in_ptr[x_offset] );                             \
            }                                                                   \
            printf("\n");                                                       \
}

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
    NEXUS_DmaJobBlockSettings blockSettings;
    NEXUS_DmaJobStatus jobStatus;
    BKNI_EventHandle dmaEvent = NULL;
    NEXUS_SecurityKeySlotSettings keySettings;
    NEXUS_KeySlotHandle keyslotHandle;
    NEXUS_SecurityAlgorithmSettings NexusConfig;
    NEXUS_SecurityClearKey key;
    uint8_t *pSrc = NULL;
    uint8_t *pDest = NULL;
    NEXUS_SecurityXPTM2MStatusOutput statusQuery;
    unsigned int i;
    uint8_t keys[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
    uint8_t iv[16]  = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
    uint8_t expectedResult[DATA_BLOCK_SIZE] = { 0x4B, 0xE6, 0xC2, 0xFA, 0x48, 0x59, 0x26, 0x12, 0xFA, 0xEF, 0x95, 0xA2, 0x52, 0x96, 0xE7, 0x78,
                                                0xFF, 0x5A, 0xA5, 0x1F, 0x88, 0x7F, 0xA5, 0xDA, 0xC6, 0xB5, 0x9C, 0xD7, 0x63, 0x3C, 0xEC, 0x10,
                                                0xB4, 0x52, 0x31, 0x9D, 0x39, 0x33, 0x4E, 0x6F, 0x1B, 0x0A, 0x96, 0x9F, 0x41, 0x87, 0x0C, 0xA2,
                                                0x5B, 0x81, 0xBB, 0x2F, 0x89, 0x1F, 0x1E, 0x33, 0x6C, 0x8A, 0x57, 0xDC, 0xDA, 0x38, 0xCC, 0x55,
                                                0x67, 0xD3, 0x72, 0x3F, 0x1B, 0xA7, 0x77, 0x72, 0x9D, 0x99, 0xE0, 0xF7, 0x16, 0xA9, 0xF9, 0x3D,
                                                0x55, 0xD1, 0x06, 0xE8, 0xD9, 0xE0, 0x3A, 0x96, 0x85, 0x20, 0xCA, 0x83, 0xDD, 0x4B, 0x4D, 0x70,
                                                0x47, 0xF3, 0x85, 0xCD, 0x8F, 0xAE, 0x20, 0x00, 0x09, 0x5F, 0xEF, 0x0B, 0x03, 0x6A, 0x6C, 0x8E,
                                                0xB5, 0x41, 0xE2, 0x7C, 0x13, 0x0A, 0x1B, 0x63, 0xF3, 0x89, 0x06, 0x7F, 0x01, 0x6F, 0x8A, 0xEE,
                                                0x2E, 0xAF, 0x8C, 0x84, 0xE6, 0x75, 0x04, 0x63, 0x71, 0x91, 0xAF, 0x6A, 0xAE, 0xFD, 0xF0, 0x98,
                                                0xAA, 0xA7, 0xC2, 0xD5, 0x84, 0x6E, 0xE2, 0xF5, 0xFA, 0xC0, 0xDA, 0x92, 0xF2, 0x92, 0xF3, 0x0F,
                                                0xE6, 0xC8, 0x56, 0x4E, 0x11, 0x3A, 0x69, 0x2A, 0x43, 0x82, 0xD5, 0xC1, 0x15, 0x71, 0x36, 0x03,
                                                0xDF, 0x04, 0x1C, 0x87, 0xCD, 0x77, 0xB8, 0xDB, 0xB6, 0xC9, 0x98, 0x11, 0xC4, 0x2E, 0x3F, 0x30,
                                                0x2C, 0x30, 0x94, 0x3A, 0x1B, 0xA8, 0x9D, 0x1C, 0xE8, 0x46, 0xFD, 0x63, 0x84, 0x86, 0xD4, 0x7B,
                                                0x86, 0x7A, 0x00, 0x33, 0xA3, 0x57, 0x0A, 0xDD, 0x45, 0xDC, 0x6B, 0x2A, 0xEF, 0x82, 0x0B, 0xBF,
                                                0xBA, 0x10, 0xFC, 0x5A, 0x01, 0x7B, 0x13, 0xC1, 0xFB, 0xF5, 0xDF, 0xF4, 0xAD, 0xA6, 0x84, 0xA0,
                                                0xEE, 0x4F, 0x2E, 0x00, 0x16, 0xC7, 0x65, 0x1E, 0x53, 0x87, 0xDC, 0xD1, 0x06, 0x42, 0x8C, 0x3A,
                                                0xC5, 0x4E, 0xA2, 0x3C, 0x4B, 0xAF, 0xDF, 0xBF, 0xAF, 0xD0, 0xDC, 0x1C, 0x68, 0x40, 0xFE, 0xF1,
                                                0xAC, 0x96, 0x53, 0xDE, 0x1A, 0x8D, 0xF4, 0x6C, 0x61, 0xFD, 0xBF, 0xC6, 0x8E, 0x98, 0x2A, 0x45,
                                                0x62, 0x56, 0x3A, 0xFA, 0x44, 0x3F, 0xDB, 0xCB, 0x6B, 0xFF, 0x73, 0x94, 0x15, 0xB6, 0x00, 0xEF,
                                                0x3A, 0x7A, 0x1F, 0xBB, 0x3C, 0xB7, 0x80, 0x95, 0x0A, 0x14, 0xCB, 0xE6, 0x7E, 0x49, 0x9F, 0xF3,
                                                0xD8, 0x74, 0x3B, 0x76, 0x3F, 0xE6, 0x6B, 0x04, 0x29, 0x8E, 0x22, 0x9B, 0x80, 0x72, 0xE6, 0xF1,
                                                0x37, 0xC8, 0x81, 0xC6, 0xF5, 0xCA, 0xCA, 0x05, 0xF3, 0x89, 0x9F, 0xD7, 0x43, 0x42, 0x66, 0x60,
                                                0x9B, 0x58, 0xAB, 0x23, 0xEF, 0xF6, 0xE6, 0xC7, 0x1A, 0xEB, 0x54, 0xE2, 0x9F, 0x2F, 0xA2, 0xB7,
                                                0xE1, 0x21, 0x23, 0xAA, 0x48, 0x2E, 0x92, 0x44, 0xDC, 0xEC, 0xF5, 0xB7, 0x31, 0x42, 0xEB, 0xEF,
                                                0x8D, 0x2F, 0xB8, 0x5E, 0xCA, 0xCD, 0x1C, 0x3D, 0xE7, 0xBA, 0xD4, 0x2E, 0xBA, 0x79, 0x09, 0x92, };

    /* Platform init */
    NEXUS_Platform_GetDefaultSettings( &platformSettings );
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init( &platformSettings );

    /* Allocate AV keyslots */
    NEXUS_Security_GetDefaultKeySlotSettings( &keySettings );
    keySettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
    keyslotHandle = NEXUS_Security_AllocateKeySlot( &keySettings );
    BDBG_ASSERT( keyslotHandle );

    /* Set up encryption key */
    NEXUS_Security_GetDefaultAlgorithmSettings(&NexusConfig);
    NexusConfig.algorithm           = NEXUS_SecurityAlgorithm_eAes;
    NexusConfig.algorithmVar        = NEXUS_SecurityAlgorithmVariant_eCounter;
    NexusConfig.aesCounterSize      = NEXUS_SecurityAesCounterSize_e128Bits;
    NexusConfig.aesCounterMode      = NEXUS_SecurityCounterMode_eGenericAllBlocks;
    NexusConfig.terminationMode     = NEXUS_SecurityTerminationMode_eClear;
    NexusConfig.operation           = NEXUS_SecurityOperation_eEncrypt;
    NexusConfig.keyDestEntryType    = NEXUS_SecurityKeyType_eOdd;
    NexusConfig.solitarySelect      = NEXUS_SecuritySolitarySelect_eClear;
    NexusConfig.ivMode              = NEXUS_SecurityIVMode_eRegular;

    rc =  NEXUS_Security_ConfigAlgorithm( keyslotHandle, &NexusConfig );
    BDBG_ASSERT( rc == NEXUS_SUCCESS && "failed to configure algorithm" );

    NEXUS_Security_GetDefaultClearKey( &key );
    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    key.keyIVType    = NEXUS_SecurityKeyIVType_eNoIV;
    key.keySize      = sizeof(keys);
    BKNI_Memcpy( key.keyData, keys, key.keySize );

    printf("\nLoading encryption clear Odd key.\n");
    if (NEXUS_Security_LoadClearKey(keyslotHandle, &key) != 0) {
        printf("\nLoad encryption key failed \n");
        return 1;
    }

    key.keyIVType = NEXUS_SecurityKeyIVType_eIV;
    key.keyEntryType = NEXUS_SecurityKeyType_eIv;
    key.keySize = sizeof(keys);
    BKNI_Memcpy(key.keyData, iv, key.keySize);

    printf("\nLoading encryption IV.\n");
    if (NEXUS_Security_LoadClearKey(keyslotHandle, &key) != 0) {
        printf("\nLoad encryption IV failed \n");
        return 1;
    }

    /* Open DMA handle */
    dma = NEXUS_Dma_Open(0, NULL);
    BDBG_ASSERT( dma );

    /* and DMA event */
    BKNI_CreateEvent( &dmaEvent );

    NEXUS_Memory_Allocate( DATA_BLOCK_SIZE, NULL, (void*)&pSrc );
    NEXUS_Memory_Allocate( DATA_BLOCK_SIZE, NULL, (void*)&pDest );

    /*  Encryption */
    memset( pSrc, 0x22, DATA_BLOCK_SIZE );

    NEXUS_DmaJob_GetDefaultSettings(&jobSettings);
    jobSettings.numBlocks                   = 1;
    jobSettings.keySlot                     = keyslotHandle;
    jobSettings.dataFormat                  = NEXUS_DmaDataFormat_eBlock;
    jobSettings.completionCallback.callback = CompleteCallback;
    jobSettings.completionCallback.context  = dmaEvent;

    dmaJob = NEXUS_DmaJob_Create(dma, &jobSettings);

    NEXUS_DmaJob_GetDefaultBlockSettings(&blockSettings);
    blockSettings.pSrcAddr                  = pSrc;
    blockSettings.pDestAddr                 = pDest;
    blockSettings.blockSize                 = DATA_BLOCK_SIZE;
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

    rc =  NEXUS_Security_XPTM2MStatusQuery( &statusQuery );
    printf( "\nM2M Security Err Status [%02X]\n", statusQuery.unM2MSecurityErrStatus );
    printf( "\nXPT Security Err Status [%02X]\n", statusQuery.unXPTSecurityErrStatus );

    for( i = 0; i < DATA_BLOCK_SIZE; i++ )
    {
        if ( pDest[i] != expectedResult[i] )
        {
            printf ("\nComparison failed at location %d\n", i);
            break;
        }
    }

    DEBUG_PRINT_ARRAY( "KEY", sizeof(keys), keys );
    DEBUG_PRINT_ARRAY( "IV", sizeof(iv), iv );
    DEBUG_PRINT_ARRAY( "Plaintext",  DATA_BLOCK_SIZE, pSrc );
    DEBUG_PRINT_ARRAY( "Ciphertext Software AES-Counter", DATA_BLOCK_SIZE, pDest );

    printf ("\nTest %s\n", i==DATA_BLOCK_SIZE? "PASSSED" : "FAILED" );

    BKNI_DestroyEvent( dmaEvent );
    NEXUS_Memory_Free( pSrc );
    NEXUS_Memory_Free( pDest );
    NEXUS_DmaJob_Destroy(dmaJob);
    NEXUS_Dma_Close( dma );
    NEXUS_Security_FreeKeySlot(keyslotHandle);
    NEXUS_Platform_Uninit();
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

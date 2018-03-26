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

#if NEXUS_HAS_SECURITY && (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 1)

#include "nexus_platform.h"
#include "nexus_dma.h"
#include "nexus_memory.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "nexus_security.h"

#define DMA_BLOCK   400
#define DMA_JOBS    1


#define DEBUG_PRINT_ARRAY(description_txt,in_size,in_ptr) { int x_offset;       \
            printf("[%s]", description_txt );                                   \
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
    NEXUS_KeySlotHandle hKeySlot;
    NEXUS_SecurityAlgorithmSettings NexusConfig;
    NEXUS_SecurityCapabilities securityCaps;
    NEXUS_SecurityClearKey key;
    uint8_t *pSource = NULL;
    uint8_t *pIntermediate = NULL;
    uint8_t *pDestination = NULL;

    unsigned int i;
    uint8_t keys[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F  };
    uint8_t iv[16]   = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
    /* result calculated from openssl
    openssl enc -in 0x02x256.bin -aes-128-cbc -K 000102030405060708090A0B0C0D0E0F -iv 00112233445566778899AABBCCDDEEFF -out output.bin */
    uint8_t expectedResult [DMA_BLOCK]  = { 0xF7, 0x40, 0x30, 0xFD, 0x41, 0x1A, 0x8E, 0x9A, 0x1F, 0xB0, 0xC3, 0x03, 0x1E, 0x65, 0x03, 0x4A,
                                  0xEF, 0xF1, 0x30, 0x6A, 0x19, 0xD7, 0xA2, 0x1D, 0xFA, 0x12, 0xB7, 0x0A, 0x5E, 0x3A, 0x44, 0x83,
                                  0x12, 0x3F, 0x72, 0xC1, 0x67, 0xD1, 0xE6, 0x42, 0x5B, 0x43, 0x1A, 0xB4, 0x48, 0x19, 0xFB, 0xF9,
                                  0x6C, 0xEC, 0x9B, 0x4D, 0x79, 0x1F, 0xF5, 0x1C, 0x16, 0xAB, 0x17, 0xC8, 0xD7, 0xE4, 0x3D, 0x3D,
                                  0x0D, 0x25, 0xD0, 0x2E, 0x4F, 0xFA, 0xD6, 0xCE, 0xE1, 0xCA, 0x25, 0xAF, 0x55, 0xEF, 0x7C, 0x70,
                                  0x11, 0x72, 0xF0, 0x2B, 0xED, 0x67, 0x94, 0x46, 0x88, 0x3B, 0x9E, 0x95, 0x1F, 0x9C, 0xCB, 0xAD,
                                  0x5E, 0x50, 0x86, 0x3F, 0xB3, 0xC4, 0x54, 0x7A, 0x8E, 0x10, 0x5E, 0x3C, 0x0B, 0xAF, 0x2F, 0x42,
                                  0x6A, 0xAC, 0xD3, 0x37, 0x8F, 0xC1, 0x7A, 0x50, 0x0E, 0x61, 0xCB, 0x1F, 0x87, 0x67, 0x44, 0xB0,
                                  0x09, 0xE6, 0xFC, 0x62, 0x82, 0xA8, 0x82, 0x99, 0x7E, 0x77, 0x3E, 0x92, 0xBB, 0x09, 0xB3, 0xA2,
                                  0x88, 0x38, 0x38, 0x20, 0x7F, 0x98, 0x6F, 0x42, 0xBF, 0xA4, 0x40, 0x71, 0xC4, 0x9A, 0x9A, 0x9E,
                                  0xD6, 0x33, 0x9A, 0xB7, 0x2E, 0x37, 0x74, 0xBE, 0xF2, 0x44, 0xE2, 0xEA, 0x8C, 0x86, 0xE6, 0x39,
                                  0xCD, 0x5A, 0x26, 0x84, 0x38, 0x36, 0xFC, 0x1D, 0x42, 0xD3, 0x1C, 0x14, 0x59, 0x3B, 0xC0, 0x67,
                                  0x0A, 0x73, 0x60, 0xB8, 0x9A, 0xD1, 0xF8, 0x99, 0x2C, 0x5D, 0xFA, 0x85, 0xA4, 0x30, 0x7E, 0x46,
                                  0xD9, 0x7B, 0x1C, 0x43, 0x82, 0x62, 0x84, 0x88, 0x6E, 0x49, 0x3F, 0xB1, 0xA3, 0x46, 0x0A, 0x6A,
                                  0xAA, 0xD9, 0x3B, 0x52, 0xFA, 0x42, 0x11, 0x6F, 0x85, 0xBE, 0xE4, 0x34, 0xBC, 0xEC, 0x2D, 0x6D,
                                  0x56, 0x97, 0x1D, 0xA2, 0x3E, 0xC8, 0x7E, 0x10, 0x6F, 0x29, 0xEE, 0x39, 0xD3, 0x73, 0xC8, 0xFF,
                                  0x50, 0x33, 0x2E, 0xFF, 0xAD, 0x49, 0x4E, 0x0A, 0xE6, 0xE0, 0x43, 0x11, 0xE4, 0x13, 0xAD, 0xF4,
                                  0xC9, 0x87, 0x4F, 0x8A, 0xF7, 0xDD, 0x09, 0x94, 0x97, 0xB5, 0x9D, 0x51, 0x68, 0x1F, 0x9F, 0x6E,
                                  0xDA, 0xAE, 0x75, 0xEC, 0x4A, 0xBD, 0x38, 0x07, 0x6C, 0x52, 0x95, 0x52, 0x95, 0xD6, 0xBD, 0x36,
                                  0x69, 0x4D, 0x98, 0x47, 0x1D, 0x1F, 0x81, 0x80, 0xD3, 0xF1, 0xFF, 0xF1, 0x19, 0x37, 0x1A, 0x13,
                                  0xF0, 0xE5, 0xD7, 0x07, 0x65, 0x59, 0xCE, 0x64, 0x05, 0x9D, 0x73, 0xBA, 0xD9, 0xED, 0x86, 0x00,
                                  0xF2, 0xD7, 0xFF, 0x07, 0x6D, 0x06, 0xE2, 0x48, 0x80, 0x73, 0xBF, 0xDE, 0xD4, 0x73, 0x6D, 0x36,
                                  0xE5, 0xE1, 0xB6, 0x4B, 0x4D, 0xC3, 0x1F, 0x01, 0xD2, 0x8D, 0x35, 0xD4, 0x44, 0x34, 0xBD, 0xEB,
                                  0xC0, 0xF5, 0xB0, 0x88, 0x4D, 0x4C, 0xA3, 0xA5, 0xF8, 0x2C, 0xA0, 0xA4, 0xA1, 0xBB, 0xD7, 0xCF,
                                  0x24, 0x2C, 0x49, 0x79, 0x5A, 0x53, 0x5A, 0x4A, 0x7C, 0x98, 0xDB, 0x8B, 0x06, 0xAD, 0x3F, 0xEE };

    /* Platform init */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);


    BKNI_Sleep( 1000 );  /* let debug flush through com port.*/
    NEXUS_GetSecurityCapabilities( &securityCaps );
    printf("NEXUS: ZeusVersion[%d.%d] FwVersion[%d.%d.%d] "
                                                         , NEXUS_ZEUS_VERSION_MAJOR(securityCaps.version.zeus)
                                                         , NEXUS_ZEUS_VERSION_MINOR(securityCaps.version.zeus)
                                                         , NEXUS_BFW_VERSION_MAJOR( securityCaps.version.firmware )
                                                         , NEXUS_BFW_VERSION_MINOR( securityCaps.version.firmware )
                                                         , NEXUS_BFW_VERSION_SUBMINOR( securityCaps.version.firmware ) );

    if( securityCaps.firmwareEpoch.valid )
    {
        printf("Epoch[0x%X]\n", securityCaps.firmwareEpoch.value );
    }
    else
    {
        printf("Epoch[not-available]\n");
    }

    printf("KeySlot types  ");
    for( i = 0; i < NEXUS_SECURITY_MAX_KEYSLOT_TYPES; i++ )
    {
        printf("[%d]", securityCaps.keySlotTableSettings.numKeySlotsForType[i] );
    }
    printf(" multi[%d]\n", securityCaps.keySlotTableSettings.numMulti2KeySlots );


    /* Allocate AV keyslots */
    NEXUS_Security_GetDefaultKeySlotSettings(&keySettings);
    keySettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
    hKeySlot = NEXUS_Security_AllocateKeySlot(&keySettings);
    if( !hKeySlot )
    {
        printf("\nAllocate keyslot failed \n");
        return 1;
    }

    /* Set up encryption key */
    NEXUS_Security_GetDefaultAlgorithmSettings( &NexusConfig );
    NexusConfig.algorithm           = NEXUS_SecurityAlgorithm_eAes;
    NexusConfig.algorithmVar        = NEXUS_SecurityAlgorithmVariant_eCbc;
    NexusConfig.terminationMode     = NEXUS_SecurityTerminationMode_eClear;
    NexusConfig.keyDestEntryType    = NEXUS_SecurityKeyType_eClear;
    NexusConfig.solitarySelect      = NEXUS_SecuritySolitarySelect_eClear;
    NexusConfig.ivMode              = NEXUS_SecurityIVMode_eRegular;
    NexusConfig.operation           = NEXUS_SecurityOperation_eEncrypt;

    if( NEXUS_Security_ConfigAlgorithm( hKeySlot, &NexusConfig ) != 0 )
    {
        printf("\nConfigAlg enc keyslot failed \n");
        return 1;
    }
    BKNI_Sleep( 2000 );  /* let debug flush through com port.*/

    NEXUS_Security_GetDefaultClearKey( &key );
    key.keyEntryType = NEXUS_SecurityKeyType_eClear;
    key.keyIVType = NEXUS_SecurityKeyIVType_eNoIV;
    key.keySize = sizeof(keys);
    BKNI_Memcpy( key.keyData, keys, key.keySize );
    if(  NEXUS_Security_LoadClearKey( hKeySlot, &key ) != 0 )
    {
        printf("\nLoad encryption key failed \n");
        return 1;
    }
    BKNI_Sleep( 2000 );  /* let debug flush through com port.*/

    key.keyEntryType = NEXUS_SecurityKeyType_eClear;
    key.keyIVType    = NEXUS_SecurityKeyIVType_eIV;
    key.keySize = sizeof(iv);
    BKNI_Memcpy( key.keyData, iv, key.keySize );
    if (NEXUS_Security_LoadClearKey( hKeySlot, &key ) != 0)
    {
        printf("\nLoad encryption IV failed \n");
        return 1;
    }

    BKNI_Sleep( 2000 );  /* let debug flush through com port.*/

    /* Open DMA handle */
    dma = NEXUS_Dma_Open( 0, NULL );

    BKNI_CreateEvent( &dmaEvent );  /* create an event. */

    NEXUS_Memory_Allocate( DMA_BLOCK, NULL, (void **)&pSource   );
    NEXUS_Memory_Allocate( DMA_BLOCK, NULL, (void **)&pIntermediate  );
    NEXUS_Memory_Allocate( DMA_BLOCK, NULL, (void **)&pDestination );

    memset( pSource,   0x22, DMA_BLOCK*sizeof(unsigned char) );
    memset( pIntermediate,  0x33, DMA_BLOCK*sizeof(unsigned char) );
    memset( pDestination, 0x44, DMA_BLOCK*sizeof(unsigned char) );

    /*  Encryption */
    BKNI_Sleep( 2000 );  /* let debug flush through com port.*/

    DEBUG_PRINT_ARRAY( "keys",  sizeof(keys),   keys );
    DEBUG_PRINT_ARRAY( "iv",    sizeof(iv), iv );

    NEXUS_DmaJob_GetDefaultSettings(&jobSettings);
    jobSettings.numBlocks                   = 1;
    jobSettings.keySlot                     = hKeySlot;
    jobSettings.dataFormat                  = NEXUS_DmaDataFormat_eBlock;
	jobSettings.completionCallback.callback = CompleteCallback;
	jobSettings.completionCallback.context	= dmaEvent;

    dmaJob = NEXUS_DmaJob_Create( dma, &jobSettings );

    NEXUS_DmaJob_GetDefaultBlockSettings( &blockSettings );
    blockSettings.pSrcAddr                  = pSource;
    blockSettings.pDestAddr                 = pIntermediate;
    blockSettings.blockSize                 = DMA_BLOCK;
    blockSettings.resetCrypto               = true;
    blockSettings.scatterGatherCryptoStart  = true;
    blockSettings.scatterGatherCryptoEnd    = true;
    blockSettings.cached                    = true;

    rc = NEXUS_DmaJob_ProcessBlocks( dmaJob, &blockSettings, 1 );
    if (rc == NEXUS_DMA_QUEUED )
    {
        BKNI_WaitForEvent( dmaEvent, BKNI_INFINITE );
        NEXUS_DmaJob_GetStatus( dmaJob, &jobStatus );
        BDBG_ASSERT( jobStatus.currentState == NEXUS_DmaJobState_eComplete );
    }

    NEXUS_DmaJob_Destroy(dmaJob);
    NEXUS_Security_FreeKeySlot( hKeySlot );	/* free keyslot for now. */

    /*  Decryption */
    /* Allocate AV keyslots */

    /* Make sure pSource matches pDest 2 */
    for (i=0;i<DMA_BLOCK; i++ ) {
        if ( pIntermediate[i] != expectedResult[i] ) {
            printf ("\nFAIL: Comparison failed at location %d\n", i);
            return 1;
        }
    }


    NEXUS_Security_GetDefaultKeySlotSettings( &keySettings );
    keySettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
    hKeySlot = NEXUS_Security_AllocateKeySlot( &keySettings );
    if( !hKeySlot )
    {
        printf("\nAllocate keyslot failed \n");
        return 1;
    }

    /* Set up decryption key */
    NEXUS_Security_GetDefaultAlgorithmSettings( &NexusConfig );

    NexusConfig.algorithm           = NEXUS_SecurityAlgorithm_eAes;
    NexusConfig.algorithmVar        = NEXUS_SecurityAlgorithmVariant_eCbc;

    NexusConfig.terminationMode     = NEXUS_SecurityTerminationMode_eClear;
    NexusConfig.keyDestEntryType    = NEXUS_SecurityKeyType_eClear;
    NexusConfig.solitarySelect      = NEXUS_SecuritySolitarySelect_eClear;
    NexusConfig.ivMode              = NEXUS_SecurityIVMode_eRegular;
    NexusConfig.operation           = NEXUS_SecurityOperation_eDecrypt;

    if ( NEXUS_Security_ConfigAlgorithm( hKeySlot, &NexusConfig )!= 0)
    {
        printf("\nConfigAlg decrypt keyslot failed \n");
        return 1;
    }

    key.keyEntryType = NEXUS_SecurityKeyType_eClear;
    key.keyIVType = NEXUS_SecurityKeyIVType_eNoIV;
    key.keySize = sizeof(keys);
    BKNI_Memcpy( key.keyData, keys, key.keySize );

    if(  NEXUS_Security_LoadClearKey( hKeySlot, &key ) != 0 )
    {
        printf("\nLoad decryption key failed \n");
        return 1;
    }

    key.keyEntryType = NEXUS_SecurityKeyType_eClear;
    key.keyIVType    = NEXUS_SecurityKeyIVType_eIV;
    key.keySize = sizeof(iv);
    BKNI_Memcpy( key.keyData, iv, key.keySize );

    if (NEXUS_Security_LoadClearKey( hKeySlot, &key ) != 0)
    {
        printf("\nLoad decryption IV failed \n");
        return 1;
    }

    NEXUS_DmaJob_GetDefaultSettings( &jobSettings );
    jobSettings.numBlocks                   = 1;
    jobSettings.keySlot                     = hKeySlot;
    jobSettings.dataFormat                  = NEXUS_DmaDataFormat_eBlock;
    jobSettings.completionCallback.callback = CompleteCallback;
    jobSettings.completionCallback.context	= dmaEvent;

    dmaJob = NEXUS_DmaJob_Create(dma, &jobSettings);

    NEXUS_DmaJob_GetDefaultBlockSettings(&blockSettings);
    blockSettings.pSrcAddr                  = pIntermediate;
    /*blockSettings.pSrcAddr                  = pSource;  */
    blockSettings.pDestAddr                 = pDestination;
    blockSettings.blockSize                 = DMA_BLOCK;
    blockSettings.resetCrypto               = true;
    blockSettings.scatterGatherCryptoStart  = true;
    blockSettings.scatterGatherCryptoEnd    = true;
    blockSettings.cached                    = true;

    rc = NEXUS_DmaJob_ProcessBlocks( dmaJob, &blockSettings, 1);
    if (rc == NEXUS_DMA_QUEUED )
    {
        BKNI_WaitForEvent(dmaEvent, BKNI_INFINITE);
        NEXUS_DmaJob_GetStatus(dmaJob, &jobStatus);
        BDBG_ASSERT(jobStatus.currentState == NEXUS_DmaJobState_eComplete);
    }

    NEXUS_DmaJob_Destroy( dmaJob );

    /* Make sure pSource matches pDest 2 */
    for (i=0;i<DMA_BLOCK; i++ ) {
        if ( pSource[i] != pDestination[i] ) {
            printf ("\nComparison failed at location %d\n", i);
            break;
        }
    }

    BKNI_Sleep( 1000 );  /* let debug flush through com port.*/

    DEBUG_PRINT_ARRAY( "Source Data",      DMA_BLOCK, pSource   );
    DEBUG_PRINT_ARRAY( "After Encryption", DMA_BLOCK, pIntermediate  );
    DEBUG_PRINT_ARRAY( "After Decryption", DMA_BLOCK, pDestination );

    if ( i==DMA_BLOCK )
    {
            printf ("\nTest passed\n");
    }

    NEXUS_Dma_Close(dma);
    NEXUS_Security_FreeKeySlot( hKeySlot );

    printf ("\nTHE END\n");
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

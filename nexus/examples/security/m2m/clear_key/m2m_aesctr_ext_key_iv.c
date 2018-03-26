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

/*  This example shows how to use external key and IV to speed up
    crypto operation.  NOTE that external key and IV must be enabled
    by OTP */

#if NEXUS_HAS_SECURITY

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "nexus_platform.h"
#include "nexus_dma.h"
#include "nexus_memory.h"
#include "nexus_security.h"

#define DATA_BLOCK_SIZE  400
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

typedef struct {
   unsigned printHelp;
   unsigned secondKeyslot;
   unsigned secondExterenalKeyslot;

}internalData_t;

internalData_t gData;

typedef struct{

    unsigned slotIndex;  /* Index into external key-slot table.  */
    struct {
        bool valid;      /* Indicates that the associated offset is valid.  */
        unsigned offset;  /* offset into external key slot. */
        unsigned size;
        uint8_t *pData;
    } key, iv;

} external_key_data_t;

#define MIN(a,b) ((a)<(b)?(a):(b))

static int test( void );
static void compileBtp( uint8_t *pBtp, external_key_data_t *pBtpData );

int main( int argc, char *argv[] )
{
    int rc = 0;
    int x;
    unsigned count;
    NEXUS_PlatformSettings platformSettings;


    /* Platform init */
    NEXUS_Platform_GetDefaultSettings( &platformSettings );
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init( &platformSettings );


    for( x = 1; x < argc; x++ )
    {
        #define TAG_PRINT_HELP "-h"
        if ( strncmp( argv[x], TAG_PRINT_HELP, strlen(TAG_PRINT_HELP) ) == 0 )  { gData.printHelp = true; }

        #define TAG_SECOND_EXTERNAL_SLOT "-sek"
        if ( strncmp( argv[x], TAG_SECOND_EXTERNAL_SLOT, strlen(TAG_SECOND_EXTERNAL_SLOT) ) == 0 )  { gData.secondExterenalKeyslot = true; }

        #define TAG_SECOND_SLOT "-sk"
        if ( strncmp( argv[x], TAG_SECOND_SLOT, strlen(TAG_SECOND_SLOT) ) == 0 )  { gData.secondKeyslot = true; }
        /*
        #define ALIGNMENT_TAG "-align="
        if ( strncmp( argv[x], ALIGNMENT_TAG, strlen(ALIGNMENT_TAG)) == 0 )
        {
            alignment = atoi(argv[x]+strlen(ALIGNMENT_TAG) );
            printf("alignment [%d]", alignment );
        }
        */
    }

    if( gData.printHelp == true )
    {
        /*printf("-max_size=    DMA Max Transfer size [integer]\n"); */
        printf("%s            Help \n", TAG_PRINT_HELP);
        printf("%s            Open Second External keySlot \n", TAG_SECOND_EXTERNAL_SLOT );
        printf("%s            Open Second keySlot \n", TAG_SECOND_SLOT );
        goto CLEANUP_AND_EXIT;
    }


    count = 1 /*50*/;
    while( count-- && rc == 0 )
    {
        BKNI_Sleep( 500 );  /* let debug flush through com port.*/

        rc = test( );
    }

CLEANUP_AND_EXIT:

    NEXUS_Platform_Uninit();
    return rc;
}

static int test( void )
{
    NEXUS_DmaHandle dma = NULL;
    NEXUS_DmaJobSettings jobSettings;
    NEXUS_DmaJobHandle dmaJob = NULL;
    NEXUS_DmaJobBlockSettings blockSettings[2];
    NEXUS_DmaJobStatus jobStatus;
    NEXUS_Error rc = NEXUS_SUCCESS;
    external_key_data_t btp;
    NEXUS_KeySlotExternalKeyData extKeyData;
    NEXUS_SecurityKeySlotSettings keySettings;
    NEXUS_KeySlotHandle keySlotExt = NULL;   /* keyslot that will use exteral key*/
    NEXUS_SecurityAlgorithmSettings keySlotonfig;
    NEXUS_SecurityCapabilities securityCaps;
    uint8_t *pExternalSource = NULL;
    uint8_t *pExternalDestination = NULL;
    uint8_t *pExternalKey = NULL;
    uint8_t *pExternalKeyShaddow = NULL;
    unsigned int externalSrcSize;
    unsigned int externalKeySize;
    unsigned int externalDstSize;
    unsigned int i;
    uint8_t key[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
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

    BKNI_Sleep( 500 );    /* let debug flush through com port.*/

    NEXUS_GetSecurityCapabilities( &securityCaps );

    /* Open DMA handle */
    dma = NEXUS_Dma_Open( 0, NULL );
    if( !dma )  { printf("ERROR line[%d] \n", __LINE__ ); goto CLEANUP_AND_EXIT; }

    externalSrcSize = DATA_BLOCK_SIZE;
    externalDstSize = DATA_BLOCK_SIZE;

    /* Allocate 32 extra bytes for external key and IV */
    NEXUS_Memory_Allocate( externalSrcSize, NULL, (void **)&pExternalSource );
    NEXUS_Memory_Allocate( externalDstSize, NULL, (void **)&pExternalDestination );

    memset( pExternalSource,   0x22, externalSrcSize );
    memset( pExternalDestination, 3, externalDstSize ); /*put random pattern in destiantion buffer. */

    NEXUS_Security_GetDefaultKeySlotSettings( &keySettings );
    keySettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;

    if( gData.secondKeyslot )
    {
        printf("+++++++++++++++++ allocating second keyslot \n" );

        keySlotExt = NEXUS_Security_AllocateKeySlot( &keySettings );
        if( !keySlotExt )  { printf("ERROR line[%d] \n", __LINE__ ); goto CLEANUP_AND_EXIT; }
    }

    keySlotExt = NEXUS_Security_AllocateKeySlot( &keySettings );
    if( !keySlotExt )  { printf("ERROR line[%d] \n", __LINE__ ); goto CLEANUP_AND_EXIT; }

    /* This example enables both external ket and IV. External key and IV can be enabled separately */

    NEXUS_Security_GetDefaultAlgorithmSettings( &keySlotonfig );

    if( gData.secondExterenalKeyslot )
    {
        keySlotonfig.keyDestEntryType    = NEXUS_SecurityKeyType_eOdd;
        keySlotonfig.algorithm           = NEXUS_SecurityAlgorithm_eAes128;
        keySlotonfig.algorithmVar        = NEXUS_SecurityAlgorithmVariant_eCounter;
        keySlotonfig.terminationMode     = NEXUS_SecurityTerminationMode_eClear;
        keySlotonfig.solitarySelect      = NEXUS_SecuritySolitarySelect_eClear;
        keySlotonfig.ivMode              = NEXUS_SecurityIVMode_eRegular;
        keySlotonfig.operation           = NEXUS_SecurityOperation_eEncrypt;
        keySlotonfig.aesCounterSize      = NEXUS_SecurityAesCounterSize_e128Bits;
        keySlotonfig.aesCounterMode      = NEXUS_SecurityCounterMode_ePartialBlockInNextPacket;
        keySlotonfig.enableExtKey        = true;
        keySlotonfig.enableExtIv         = true;
        rc = NEXUS_Security_ConfigAlgorithm( keySlotExt, &keySlotonfig );
        if( rc )  { printf("ERROR line[%d] \n", __LINE__ ); goto CLEANUP_AND_EXIT; }
    }

    keySlotonfig.algorithm           = NEXUS_SecurityAlgorithm_eAes128;
    keySlotonfig.algorithmVar        = NEXUS_SecurityAlgorithmVariant_eCounter;
    keySlotonfig.terminationMode     = NEXUS_SecurityTerminationMode_eClear;
    keySlotonfig.solitarySelect      = NEXUS_SecuritySolitarySelect_eClear;
    keySlotonfig.ivMode              = NEXUS_SecurityIVMode_eRegular;
    keySlotonfig.operation           = NEXUS_SecurityOperation_eEncrypt;
    keySlotonfig.aesCounterSize      = NEXUS_SecurityAesCounterSize_e128Bits;
    keySlotonfig.aesCounterMode      = NEXUS_SecurityCounterMode_ePartialBlockInNextPacket;
    keySlotonfig.enableExtKey        = true;
    keySlotonfig.enableExtIv         = true;
    rc = NEXUS_Security_ConfigAlgorithm( keySlotExt, &keySlotonfig );
    if( rc )  { printf("ERROR line[%d] \n", __LINE__ ); goto CLEANUP_AND_EXIT; }

    if( securityCaps.version.zeus >= NEXUS_ZEUS_VERSION_CALC(4,0) )
    {
        /* Zeus 4 ...  external Key and IV are encapsulated within BTP and inserted in front of DMA tansfer */
        externalKeySize = 188;
        NEXUS_Memory_Allocate( externalKeySize, NULL, (void **)&pExternalKey );
        NEXUS_Memory_Allocate( externalKeySize, NULL, (void **)&pExternalKeyShaddow );
        memset( pExternalKey, 0, externalKeySize );
        memset( pExternalKeyShaddow, 0, externalKeySize );

        rc = NEXUS_KeySlot_GetExternalKeyData( keySlotExt,
                                               NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem,
                                               NEXUS_SecurityKeyType_eClear,
                                               &extKeyData );
        if( rc )  { printf("ERROR line[%d] \n", __LINE__ ); goto CLEANUP_AND_EXIT; }

        BKNI_Memset( &btp, 0, sizeof(btp) );

        btp.slotIndex = extKeyData.slotIndex;

        if( extKeyData.key.valid )
        {
            btp.key.valid = true;
            btp.key.offset = extKeyData.key.offset;
            btp.key.size  = sizeof(key);
            btp.key.pData = key;
        }

        if( extKeyData.iv.valid )
        {
            btp.iv.valid  = true;
            btp.iv.offset  = extKeyData.iv.offset;
            btp.iv.size  = sizeof(iv);
            btp.iv.pData = iv;
        }

        BKNI_Sleep( 250 );    /* let debug flush through com port.*/

        compileBtp( pExternalKey, &btp ); /* compile a Broadcom Transport Packet into pExternalKey */
    }
    else
    {
        /* Zeus 3 ... external Key and IV are concatinated and inserted in front of DMA transfer. */
        externalKeySize = sizeof(key) + sizeof(iv);
        NEXUS_Memory_Allocate( externalKeySize, NULL, (void **)&pExternalKey );
        NEXUS_Memory_Allocate( externalKeySize, NULL, (void **)&pExternalKeyShaddow );
        memset( pExternalKey, 0, externalKeySize );
        memset( pExternalKeyShaddow, 0, externalKeySize );
        BKNI_Memcpy( pExternalKey, (key + 8), 8 ); /* Copy key.  H and L need to be swapped */
        BKNI_Memcpy( (pExternalKey + 8), key, 8 );
        BKNI_Memcpy( (pExternalKey + sizeof(key)), (iv+8), 8 );  /* Copy IV.  H and L need to be swapped */
        BKNI_Memcpy( (pExternalKey + sizeof(key) + 8) , iv, 8 );  /* Copy IV */
    }

    NEXUS_DmaJob_GetDefaultSettings( &jobSettings );
    jobSettings.numBlocks                   = 2;
    jobSettings.keySlot                     = keySlotExt;
    jobSettings.dataFormat                  = NEXUS_DmaDataFormat_eBlock;
    jobSettings.completionCallback.callback = NULL;

    dmaJob = NEXUS_DmaJob_Create( dma, &jobSettings );
    if( !dmaJob ) { printf("ERROR line[%d] \n", __LINE__ ); goto CLEANUP_AND_EXIT; }

    NEXUS_DmaJob_GetDefaultBlockSettings( &blockSettings[0] );
    NEXUS_DmaJob_GetDefaultBlockSettings( &blockSettings[1] );

    blockSettings[0].pSrcAddr                   = pExternalKey;
    blockSettings[0].pDestAddr                  = pExternalKeyShaddow;
    blockSettings[0].blockSize                  = externalKeySize;
    blockSettings[0].resetCrypto                = true;
    blockSettings[0].scatterGatherCryptoStart   = true;  /* Scatter and gather must be used for external key and IV to work */

    if( securityCaps.version.zeus >= NEXUS_ZEUS_VERSION_CALC(4,0) )
    {
        blockSettings[0].scatterGatherCryptoEnd     = true;
        blockSettings[0].securityBtp                = true;
        blockSettings[1].resetCrypto                = true;
        blockSettings[1].scatterGatherCryptoStart   = true;
    }

    blockSettings[1].pSrcAddr                  = pExternalSource;
    blockSettings[1].pDestAddr                 = pExternalDestination;
    blockSettings[1].blockSize                 = externalDstSize;
    blockSettings[1].scatterGatherCryptoEnd    = true;

    rc = NEXUS_DmaJob_ProcessBlocks( dmaJob, blockSettings, 2 );

    for(;;)
    {
        BKNI_Delay(1000);
        NEXUS_DmaJob_GetStatus( dmaJob, &jobStatus );

        if( jobStatus.currentState == NEXUS_DmaJobState_eComplete )
        {
            break;
        }
        printf("Waiting [%d]+\n", jobStatus.currentState );
        BKNI_Delay(100);
    }

    DEBUG_PRINT_ARRAY( "External KEY", externalKeySize, pExternalKey );
    DEBUG_PRINT_ARRAY( "Plaintext",    externalSrcSize, pExternalSource );
    DEBUG_PRINT_ARRAY( "Ciphertext External - Counter",   externalDstSize, pExternalDestination );

    /* Make sure External matches Internal    */
    for( i=0; i < DATA_BLOCK_SIZE; i++ )
    {
        if ( expectedResult[i] != pExternalDestination[i] )
        {
            printf ("\nComparison failed at location %d\n", i);
            break;
        }
    }
    printf ("\nTest %s \n\n", (i==DATA_BLOCK_SIZE)?"PASSED":"FAILED");

CLEANUP_AND_EXIT:

    BKNI_Sleep( 500 );    /* let debug flush through com port.*/
    if(dmaJob) NEXUS_DmaJob_Destroy( dmaJob );
    if(dma) NEXUS_Dma_Close(dma);
    if(keySlotExt) NEXUS_Security_FreeKeySlot( keySlotExt );
    if(pExternalSource) NEXUS_Memory_Free( pExternalSource );
    if(pExternalDestination) NEXUS_Memory_Free( pExternalDestination );
    if(pExternalKey) NEXUS_Memory_Free( pExternalKey );
    if(pExternalKeyShaddow) NEXUS_Memory_Free( pExternalKeyShaddow );

    return 0;
}




static void compileBtp( uint8_t *pBtp, external_key_data_t *pBtpData )
{
    unsigned char *p = pBtp;
    unsigned len = 0;
    unsigned char templateBtp[] =
    {               /* ( 0) */  0x47,
                    /* ( 1) */  0x00,
                    /* ( 2) */  0x21,
                    /* ( 3) */  0x20,
                    /* ( 4) */  0xb7,
                    /* ( 5) */  0x82,
                    /* ( 6) */  0x45,
                    /* ( 7) */  0x00,
                    /* ( 8) */  0x42, /*'B'*/
                    /* ( 9) */  0x52, /*'R'*/
                    /* (10) */  0x43, /*'C'*/
                    /* (11) */  0x4d, /*'M'*/
                    /* (12) */  0x00,
                    /* (13) */  0x00,
                    /* (14) */  0x00,
                    /* (15) */  0x1a  /* security BTP */};

    assert ( pBtp );
    assert ( pBtpData );
    assert ( sizeof(templateBtp) <= 188 );

    memset( pBtp, 0, 188 );
    memcpy( pBtp, templateBtp, sizeof(templateBtp) );

    /* Location of external  keyslot in external keyslot table  */
    pBtp[18] = (pBtpData->slotIndex>>8) &0xFF;
    pBtp[19] =  pBtpData->slotIndex     &0xFF;


    printf ("\n Slot offset [%d] \n", pBtpData->slotIndex );

    /*pack key into BTP*/
    printf ("KEY valid[%d] offset[%d] size[%d]\n",pBtpData->key.valid, pBtpData->key.offset, pBtpData->key.size );
    if( pBtpData->key.valid )
    {
        p = &pBtp[20];                  /* start of BTP data section . */
        p += (pBtpData->key.offset * 16); /* locate where to write the key within the BTP data section. */

        len = pBtpData->key.size;

        DEBUG_PRINT_ARRAY("KEY", len, pBtpData->key.pData );

        pBtpData->key.pData += (len-8);   /*  write the data into BTP in reversed 64bit chunks !! */

        while( len )
        {
            memcpy(  p   , pBtpData->key.pData, MIN(len,8) );   /* set Key   */
            memset( (p+8), 0xFF               , MIN(len,8) );   /* set Mask */
            p += 16;                                        /* 8 bytes for data, 8 for mask */
            len -= MIN(len,8);
            pBtpData->key.pData -= MIN(len,8);
        }
    }

    /* pack iv into BTP */
    printf ("IV valid[%d] offset[%d] size[%d]\n",pBtpData->iv.valid, pBtpData->iv.offset, pBtpData->iv.size );
    if( pBtpData->iv.valid )
    {

        p = &pBtp[20];                   /* start of BTP data section . */
        p += (pBtpData->iv.offset * 16);  /* move to offset withtin BTP for IV */

        len = pBtpData->iv.size;

        DEBUG_PRINT_ARRAY("IV", len, pBtpData->iv.pData );

        pBtpData->iv.pData += (len-8);   /*  write the data into BTP in reverse!! */

        while( len )
        {
            memcpy( p, pBtpData->iv.pData, MIN(len,8) );   /* set IV    */
            memset( p+8, 0xFF            , MIN(len,8) );   /* set Mask */
            p += 16;
            len -= MIN(len,8);
            pBtpData->iv.pData -= MIN(len,8);
        }
    }

    p = &pBtp[0];                /* start of BTP data section . */
    p += 92;
    BKNI_Memset( p, 0xFF, 8 );   /* set Mask */
    p += 16;
    BKNI_Memset( p, 0xFF, 8 );   /* set Mask */
    p += 16;
    BKNI_Memset( p, 0xFF, 8 );   /* set Mask */
    p += 16;
    BKNI_Memset( p, 0xFF, 8 );   /* set Mask */

    return;
}

#else /* NEXUS_HAS_SECURITY */
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform!\n");
    return -1;
}
#endif

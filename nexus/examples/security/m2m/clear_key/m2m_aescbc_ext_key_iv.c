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

/*  This example shows how to use external key and IV to speed up
    crypto operation.  NOTE that external key and IV must be enabled
    by OTP */

#if NEXUS_HAS_SECURITY && (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 3)


#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "nexus_platform.h"
#include "nexus_dma.h"
#include "nexus_memory.h"
#include "nexus_security.h"

#define DATA_BLOCK_SIZE   400

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



int main( void )
{
    int rc = 0;
    unsigned count;
    NEXUS_PlatformSettings platformSettings;

    /* Platform init */
    NEXUS_Platform_GetDefaultSettings( &platformSettings );
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init( &platformSettings );

    count = 1 /*50*/;
    while( count-- && rc == 0 )
    {
        BKNI_Sleep( 500 );  /* let debug flush through com port.*/

        rc = test( );
    }

    return rc;
}

static int test( void )
{
    NEXUS_DmaHandle dma = NULL;
    NEXUS_DmaJobSettings jobSettings;
    NEXUS_DmaJobHandle dmaJob;
    NEXUS_DmaJobBlockSettings blockSettings[2];
    NEXUS_DmaJobStatus jobStatus;
    NEXUS_Error rc = NEXUS_SUCCESS;
    external_key_data_t btp;
    NEXUS_KeySlotExternalKeyData extKeyData;
    NEXUS_SecurityKeySlotSettings keySettings;
    NEXUS_KeySlotHandle keySlotInt = NULL;   /* keyslot that will use internal software key*/
    NEXUS_KeySlotHandle keySlotExt = NULL;   /* keyslot that will use exteral key*/
    NEXUS_SecurityAlgorithmSettings NexusConfig;
    NEXUS_SecurityClearKey nexusClearKey;
    NEXUS_SecurityCapabilities securityCaps;
    uint8_t *pInternalSource = NULL;
    uint8_t *pInternalDestination = NULL;
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

    NEXUS_GetSecurityCapabilities( &securityCaps );

    /*************************************************************/
    /*                    Part ONE ...  Using software key         */
    /*************************************************************/
    NEXUS_Security_GetDefaultKeySlotSettings( &keySettings );
    keySettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
    keySlotInt = NEXUS_Security_AllocateKeySlot( &keySettings );
    if( !keySlotInt )
    {
        printf("\nAllocate enc keySlotInt failed \n");
        return 1;
    }

    printf( "\nAllocated key slot %x \n", (unsigned int)keySlotInt );

    /* Set up encryption key */
    NEXUS_Security_GetDefaultAlgorithmSettings( &NexusConfig );

    if( securityCaps.version.zeus >= NEXUS_ZEUS_VERSION_CALC(4,0) )
    {
        NexusConfig.keyDestEntryType = NEXUS_SecurityKeyType_eClear;
    }

    NexusConfig.algorithm           = NEXUS_SecurityAlgorithm_eAes128;
    NexusConfig.algorithmVar        = NEXUS_SecurityAlgorithmVariant_eCbc;
    NexusConfig.terminationMode     = NEXUS_SecurityTerminationMode_eClear;
    NexusConfig.solitarySelect      = NEXUS_SecuritySolitarySelect_eClear;
    NexusConfig.ivMode              = NEXUS_SecurityIVMode_eRegular;
    NexusConfig.operation           = NEXUS_SecurityOperation_eEncrypt;

    if( NEXUS_Security_ConfigAlgorithm( keySlotInt, &NexusConfig ) != 0 )
    {
        printf("\nConfigAlg enc keySlotInt failed \n");
        return 1;
    }

    NEXUS_Security_GetDefaultClearKey( &nexusClearKey );

    if( securityCaps.version.zeus >= NEXUS_ZEUS_VERSION_CALC(4,0) )
    {
        nexusClearKey.keyEntryType = NEXUS_SecurityKeyType_eClear;
    }

    /* load the key  */
    nexusClearKey.keyIVType = NEXUS_SecurityKeyIVType_eNoIV;
    nexusClearKey.keySize = sizeof(key);
    BKNI_Memcpy( nexusClearKey.keyData, key, nexusClearKey.keySize );

    if(  NEXUS_Security_LoadClearKey( keySlotInt, &nexusClearKey ) != 0 )
    {
        printf("\nLoad encryption key failed \n");
        return 1;
    }

    /* load the iv */
    nexusClearKey.keyIVType    = NEXUS_SecurityKeyIVType_eIV;
    nexusClearKey.keySize      = sizeof(iv);
    BKNI_Memcpy( nexusClearKey.keyData, iv, nexusClearKey.keySize );

    if (NEXUS_Security_LoadClearKey( keySlotInt, &nexusClearKey ) != 0)
    {
        printf("\nLoad encryption IV failed \n");
        return 1;
    }

    /* Open DMA handle */
    dma = NEXUS_Dma_Open( 0, NULL );

    NEXUS_Memory_Allocate( DATA_BLOCK_SIZE, NULL, (void *)&pInternalSource );
    NEXUS_Memory_Allocate( DATA_BLOCK_SIZE, NULL, (void *)&pInternalDestination );

    memset( pInternalSource,        0x22,    DATA_BLOCK_SIZE );
    memset( pInternalDestination,   3,    DATA_BLOCK_SIZE );

    NEXUS_DmaJob_GetDefaultSettings(&jobSettings);
    jobSettings.numBlocks                   = 1;
    jobSettings.keySlot                     = keySlotInt;
    jobSettings.dataFormat                  = NEXUS_DmaDataFormat_eBlock;
    jobSettings.completionCallback.callback = NULL;

    dmaJob = NEXUS_DmaJob_Create( dma, &jobSettings );

    NEXUS_DmaJob_GetDefaultBlockSettings(&blockSettings[0]);
    blockSettings[0].pSrcAddr                 = pInternalSource;
    blockSettings[0].pDestAddr                = pInternalDestination;
    blockSettings[0].blockSize                = DATA_BLOCK_SIZE;
    blockSettings[0].resetCrypto              = true;
    blockSettings[0].scatterGatherCryptoStart = true;
    blockSettings[0].scatterGatherCryptoEnd   = true;
    blockSettings[0].cached                   = true;

    rc = NEXUS_DmaJob_ProcessBlocks( dmaJob, &blockSettings[0], 1 );

    for(;;)
    {
        NEXUS_DmaJob_GetStatus(dmaJob, &jobStatus);
        if(jobStatus.currentState == NEXUS_DmaJobState_eComplete )
        {
           break;
        }
        BKNI_Delay(1);
    }

    NEXUS_DmaJob_Destroy( dmaJob );
    DEBUG_PRINT_ARRAY( "IV", sizeof(iv), iv );
    DEBUG_PRINT_ARRAY( "KEY", sizeof(key), key );
    DEBUG_PRINT_ARRAY( "Plaintext", DATA_BLOCK_SIZE, pInternalSource );
    DEBUG_PRINT_ARRAY( "Ciphertext - AES-CBC - software key ", DATA_BLOCK_SIZE, pInternalDestination );

    BKNI_Sleep( 500 );  /* let debug flush through com port.*/

    /*************************************************************/
    /*              Part TWO ... using EXTERNAL Key              */
    /*************************************************************/

    externalSrcSize = DATA_BLOCK_SIZE;
    externalDstSize = DATA_BLOCK_SIZE;

    /* Allocate 32 extra bytes for external key and IV */
    NEXUS_Memory_Allocate( externalSrcSize, NULL, (void *)&pExternalSource );
    NEXUS_Memory_Allocate( externalDstSize, NULL, (void *)&pExternalDestination );

    memset( pExternalSource,   0x22, externalSrcSize );
    memset( pExternalDestination, 3, externalDstSize );

    NEXUS_Security_GetDefaultKeySlotSettings( &keySettings );
    keySettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
    keySlotExt = NEXUS_Security_AllocateKeySlot(&keySettings);
    if(!keySlotExt)
    {
        printf("\nAllocate dec keySlotExt failed \n");
        return 1;
    }

    printf("\nAllocated key slot %x \n",(unsigned int)keySlotExt );

    /* This example enables both external ket and IV. External key and IV can be enabled separately */

    NEXUS_Security_GetDefaultAlgorithmSettings( &NexusConfig );
    if( securityCaps.version.zeus >= NEXUS_ZEUS_VERSION_CALC(4,0) )
    {
        NexusConfig.keyDestEntryType = NEXUS_SecurityKeyType_eClear;
    }

    NexusConfig.algorithm        = NEXUS_SecurityAlgorithm_eAes128;
    NexusConfig.algorithmVar     = NEXUS_SecurityAlgorithmVariant_eCbc;
    NexusConfig.terminationMode  = NEXUS_SecurityTerminationMode_eClear;
    NexusConfig.solitarySelect   = NEXUS_SecuritySolitarySelect_eClear;
    NexusConfig.ivMode           = NEXUS_SecurityIVMode_eRegular;
    NexusConfig.operation        = NEXUS_SecurityOperation_eEncrypt;

    NexusConfig.enableExtKey     = true;
    NexusConfig.enableExtIv      = true;

    if( NEXUS_Security_ConfigAlgorithm( keySlotExt, &NexusConfig ) != NEXUS_SUCCESS )
    {
        printf("\nConfigAlg 2nd keySlotInt failed \n");
        return 1;
    }

    printf("\nZeus Version [%d.%d] \n"  , NEXUS_ZEUS_VERSION_MAJOR(securityCaps.version.zeus)
                                        , NEXUS_ZEUS_VERSION_MINOR(securityCaps.version.zeus) );
    printf("BSP Firmware version [0x%08X] \n", securityCaps.version.firmware );

    if( securityCaps.version.zeus >= NEXUS_ZEUS_VERSION_CALC(4,0) )
    {
        /* Zeus 4 ...  external Key and IV are encapsulated within BTP and inserted in front of DMA tansfer */
        externalKeySize = 188;
        NEXUS_Memory_Allocate( externalKeySize, NULL, (void *)&pExternalKey );
        NEXUS_Memory_Allocate( externalKeySize, NULL, (void *)&pExternalKeyShaddow );
        memset( pExternalKey, 0, externalKeySize );
        memset( pExternalKeyShaddow, 0, externalKeySize );

        rc = NEXUS_KeySlot_GetExternalKeyData( keySlotExt,
                                               NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem,
                                               NEXUS_SecurityKeyType_eClear,
                                               &extKeyData );
        assert(rc == NEXUS_SUCCESS);

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
            btp.iv.offset = extKeyData.iv.offset;
            btp.iv.size   = sizeof(iv);
            btp.iv.pData  = iv;
        }

        BKNI_Sleep( 250 );    /* let debug flush through com port.*/

        compileBtp( pExternalKey, &btp ); /* compile a Broadcom Transport Packet into pExternalKey */

        DEBUG_PRINT_ARRAY( "External KEY BTP", externalKeySize, pExternalKey );
    }
    else
    {
        /* Zeus 3 ... external Key and IV are concatinated and inserted in front of DMA transfer. */
        externalKeySize = sizeof(key) + sizeof(iv);

        NEXUS_Memory_Allocate( externalKeySize, NULL, (void *)&pExternalKey );
        NEXUS_Memory_Allocate( externalKeySize, NULL, (void *)&pExternalKeyShaddow );
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

    NEXUS_DmaJob_GetDefaultBlockSettings( &blockSettings[0] );
    NEXUS_DmaJob_GetDefaultBlockSettings( &blockSettings[1] );

    blockSettings[0].pSrcAddr                  = pExternalKey;
    blockSettings[0].pDestAddr                 = pExternalKeyShaddow;
    blockSettings[0].blockSize                 = externalKeySize;
    blockSettings[0].resetCrypto               = true;
    blockSettings[0].scatterGatherCryptoStart  = true;  /* Scatter and gather must be used for external key and IV to work */

    if( securityCaps.version.zeus >= NEXUS_ZEUS_VERSION_CALC(4,0) )
    {
        blockSettings[0].scatterGatherCryptoEnd     = true;
        blockSettings[0].securityBtp                = true;
        blockSettings[1].resetCrypto                = true;
        blockSettings[1].scatterGatherCryptoStart   = true;
    }

    blockSettings[1].pSrcAddr                   = pExternalSource;
    blockSettings[1].pDestAddr                  = pExternalDestination;
    blockSettings[1].blockSize                  = externalDstSize;
    blockSettings[1].scatterGatherCryptoEnd     = true;

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

    NEXUS_DmaJob_Destroy( dmaJob );

    DEBUG_PRINT_ARRAY( "External KEY", externalKeySize, pExternalKey );
    DEBUG_PRINT_ARRAY( "External KEY shaddow", externalKeySize, pExternalKeyShaddow );
    DEBUG_PRINT_ARRAY( "Plaintext",    externalSrcSize, pExternalSource );
    DEBUG_PRINT_ARRAY( "Ciphertext - AES-CBC - External key",   externalDstSize, pExternalDestination );

    /* Make sure External matches Internal  */
    for( i=0; i < DATA_BLOCK_SIZE; i++ )
    {
        if ( pInternalDestination[i] != pExternalDestination[i] )
        {
            printf ("\nComparison failed at location %d\n", i);
            break;
        }
    }

    printf ("\nTest %s \n\n", (i==DATA_BLOCK_SIZE)?"PASSED":"FAILED");


    if( dma )NEXUS_Dma_Close(dma);

    if( keySlotInt ) NEXUS_Security_FreeKeySlot( keySlotInt );
    if( keySlotExt ) NEXUS_Security_FreeKeySlot( keySlotExt );
    if( pInternalSource )      NEXUS_Memory_Free( pInternalSource );
    if( pInternalDestination ) NEXUS_Memory_Free( pInternalDestination );
    if( pExternalSource )      NEXUS_Memory_Free( pExternalSource );
    if( pExternalDestination ) NEXUS_Memory_Free( pExternalDestination );
    if( pExternalKey )         NEXUS_Memory_Free( pExternalKey );
    if( pExternalKeyShaddow )  NEXUS_Memory_Free( pExternalKeyShaddow );

    return 0;
}



static void compileBtp( uint8_t *pBtp, external_key_data_t *pBtpData )
{
    unsigned char *p = pBtp;
    unsigned x = 0;
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
        x = 0;

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
        x = 0;

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

    DEBUG_PRINT_ARRAY("BTP  0- 19", 20      , pBtp);
    DEBUG_PRINT_ARRAY("BTP 20-188", (188-20), (pBtp+20));

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

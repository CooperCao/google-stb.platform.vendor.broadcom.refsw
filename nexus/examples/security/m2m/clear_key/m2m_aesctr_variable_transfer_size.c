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

#if NEXUS_HAS_SECURITY

#include "nexus_platform.h"
#include "nexus_dma.h"
#include "nexus_memory.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include "nexus_security.h"
#include "nexus_bsp_config.h"

/* This application examplifies how DMA transfers that are not a multiple of algorithm block size (usually 16 bytes) can be handled.
   It is achieved with the assiatance of a thin layer on top of the DMA engine (sampleDmaTransferManager_*) . */

typedef struct sampleDmaTransferManagerInstance*  sampleDmaTransferManagerHandle;

typedef struct{
    NEXUS_KeySlotHandle keySlotHandle;
}sampleDmaTransferManagerAllocSettings;

static sampleDmaTransferManagerHandle sampleDmaTransferManager_Create( sampleDmaTransferManagerAllocSettings *pSettings );
static void sampleDmaTransferManager_Destroy( sampleDmaTransferManagerHandle handle );
static unsigned sampleDmaTransferManager_Transfer( sampleDmaTransferManagerHandle handle, char* pSource, char* pDestination, unsigned size );
static unsigned sampleDmaTransferManager_Flush( sampleDmaTransferManagerHandle handle );



const uint8_t g_Key[] = { 0xe9, 0x7a, 0xd2, 0x85, 0xcb, 0x5d, 0x3b, 0xe4, 0xa7, 0x96, 0x1b, 0x7a, 0xf6, 0x91, 0x70, 0xda };
const uint8_t g_iv[]  = { 0x42, 0xbc, 0x2e, 0x0d, 0xc9, 0x52, 0xaf, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


#define MAX_DATA_SIZE    (1024*1024)
uint8_t* g_clearData     = NULL;  /* initial clear data. */
uint8_t* g_encryptedData = NULL;  /* encrypted data.     */
uint8_t* g_decryptedData = NULL;  /* decrypted data      */


#define DEBUG_PRINT_ARRAY(description_txt,in_size,in_ptr) { int x_offset;       \
            printf("[%s][%d]", description_txt, in_size );                      \
            for( x_offset = 0; x_offset < (int)(in_size); x_offset++ )          \
            {                                                                   \
                if( x_offset%32 == 0 ) printf("\n");                            \
                                                                                \
                printf("0x%02X, ", in_ptr[x_offset] );                             \
            }                                                                   \
            printf("\n");                                                       \
}

static void CompleteCallback(void *pParam, int iParam)
{
    BSTD_UNUSED(iParam);
    /* fprintf(stderr, ".", (unsigned long)pParam); */
    BKNI_SetEvent(pParam);
}


NEXUS_KeySlotHandle CreateAndConfigureKeySlot(void)
{
  /* Allocate AV keyslots */
  NEXUS_SecurityKeySlotSettings keySettings;
  NEXUS_Security_GetDefaultKeySlotSettings( &keySettings );
  keySettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
  NEXUS_KeySlotHandle keyslotHandle = NEXUS_Security_AllocateKeySlot(&keySettings);
  BDBG_ASSERT( keyslotHandle );

  /* Set up encryption key */
  NEXUS_SecurityAlgorithmSettings NexusConfig;
  NEXUS_Security_GetDefaultAlgorithmSettings(&NexusConfig);
  NexusConfig.algorithm           = NEXUS_SecurityAlgorithm_eAes;
  NexusConfig.algorithmVar        = NEXUS_SecurityAlgorithmVariant_eCounter;
  NexusConfig.aesCounterSize      = NEXUS_SecurityAesCounterSize_e128Bits;
  NexusConfig.aesCounterMode      = NEXUS_SecurityCounterMode_eGenericAllBlocks;
  NexusConfig.terminationMode     = NEXUS_SecurityTerminationMode_eClear;
  NexusConfig.operation           = NEXUS_SecurityOperation_eDecrypt;
  NexusConfig.keyDestEntryType    = NEXUS_SecurityKeyType_eClear;
  NexusConfig.solitarySelect      = NEXUS_SecuritySolitarySelect_eClear;
  NexusConfig.ivMode              = NEXUS_SecurityIVMode_eRegular;

  NEXUS_Error rc = NEXUS_Security_ConfigAlgorithm( keyslotHandle, &NexusConfig );
  BDBG_ASSERT( rc == NEXUS_SUCCESS && "failed to configure algorithm" );
  (void)rc;

  return keyslotHandle;
}

void InvalidateKey(NEXUS_KeySlotHandle keyslotHandle)
{
  NEXUS_SecurityInvalidateKeySettings invSettings;
  NEXUS_Security_GetDefaultInvalidateKeySettings ( &invSettings );
  invSettings.invalidateAllEntries = true;
  invSettings.invalidateKeyType = NEXUS_SecurityInvalidateKeyFlag_eDestKeyOnly;
  NEXUS_Security_InvalidateKey ( keyslotHandle, &invSettings );
  return;
}

void LoadKey(NEXUS_KeySlotHandle keyslotHandle)
{
  NEXUS_SecurityClearKey key;
  NEXUS_Security_GetDefaultClearKey( &key );
  key.keyEntryType = NEXUS_SecurityKeyType_eClear;
  key.keyIVType    = NEXUS_SecurityKeyIVType_eNoIV;
  key.keySize      = sizeof(g_Key);
  BKNI_Memcpy( key.keyData, g_Key, key.keySize );

  if (NEXUS_Security_LoadClearKey(keyslotHandle, &key) != 0) {
    printf("\nLoad encryption key failed \n");
  }
  return;
}

void LoadIv(NEXUS_KeySlotHandle keyslotHandle, const uint8_t* _iv)
{
    NEXUS_SecurityClearKey key;
    NEXUS_Security_GetDefaultClearKey( &key );
    key.keyIVType    = NEXUS_SecurityKeyIVType_eIV;
    key.keyEntryType = NEXUS_SecurityKeyType_eClear;
    key.keySize = sizeof(g_iv);
    BKNI_Memcpy(key.keyData, _iv, sizeof(g_iv));

    if (NEXUS_Security_LoadClearKey(keyslotHandle, &key) != 0) {
        fprintf(stderr, "\nLoad encryption IV failed \n");
    }
}


void initData(void)
{
    NEXUS_Error rc;
    NEXUS_DmaJobSettings jobSettings;
    NEXUS_DmaJobHandle dmaJobHandle;
    NEXUS_DmaJobBlockSettings blockSettings;
    NEXUS_DmaHandle dmaHandle;
    BKNI_EventHandle dmaEvent = NULL;

    NEXUS_Memory_Allocate( MAX_DATA_SIZE, NULL, (void*)&g_clearData     );  BDBG_ASSERT( g_clearData  );
    NEXUS_Memory_Allocate( MAX_DATA_SIZE, NULL, (void*)&g_encryptedData );  BDBG_ASSERT( g_encryptedData );
    NEXUS_Memory_Allocate( MAX_DATA_SIZE, NULL, (void*)&g_decryptedData );  BDBG_ASSERT( g_decryptedData  );

    BKNI_Memset( g_clearData,     0xEE, MAX_DATA_SIZE );
    BKNI_Memset( g_encryptedData, 0,    MAX_DATA_SIZE );
    BKNI_Memset( g_decryptedData, 0xFF, MAX_DATA_SIZE );

    NEXUS_KeySlotHandle keyslotHandle = CreateAndConfigureKeySlot();

    InvalidateKey( keyslotHandle );
    LoadKey( keyslotHandle );
    LoadIv( keyslotHandle, g_iv );

     /* Open DMA handle */
    dmaHandle = NEXUS_Dma_Open(0, NULL);
    BDBG_ASSERT( dmaHandle );

    /* and DMA event */
    BKNI_CreateEvent( &dmaEvent );

    NEXUS_DmaJob_GetDefaultSettings(&jobSettings);
    jobSettings.numBlocks                   = 1;
    jobSettings.keySlot                     = keyslotHandle;
    jobSettings.dataFormat                  = NEXUS_DmaDataFormat_eBlock;
    jobSettings.completionCallback.callback = CompleteCallback;
    jobSettings.completionCallback.context  = dmaEvent;
    dmaJobHandle = NEXUS_DmaJob_Create(dmaHandle, &jobSettings);

    NEXUS_DmaJob_GetDefaultBlockSettings(&blockSettings);
    blockSettings.pSrcAddr  = g_clearData;
    blockSettings.pDestAddr = g_encryptedData;
    blockSettings.blockSize = MAX_DATA_SIZE;
    blockSettings.cached = true;
    blockSettings.resetCrypto = true;
    blockSettings.scatterGatherCryptoStart = true;
    blockSettings.scatterGatherCryptoEnd = true;
    rc = NEXUS_DmaJob_ProcessBlocks( dmaJobHandle, &blockSettings, 1 );

    if (rc == NEXUS_DMA_QUEUED )
    {
      BKNI_WaitForEvent(dmaEvent, BKNI_INFINITE);
      NEXUS_DmaJobStatus jobStatus;
      NEXUS_DmaJob_GetStatus(dmaJobHandle, &jobStatus);
      BDBG_ASSERT(jobStatus.currentState == NEXUS_DmaJobState_eComplete);
    }
    NEXUS_DmaJob_Destroy(dmaJobHandle);

    NEXUS_Security_FreeKeySlot(keyslotHandle);

    BKNI_DestroyEvent( dmaEvent );
    NEXUS_Dma_Close( dmaHandle );

    return;
}

void unInitData(void)
{
    NEXUS_Memory_Free( g_clearData );
    NEXUS_Memory_Free( g_encryptedData );
    NEXUS_Memory_Free( g_decryptedData );

    return;
}


int main(void)
{
    NEXUS_PlatformSettings platformSettings;
    unsigned byteOffset = 0;
    unsigned bytesTransferred = 0;
    unsigned errorCount = 0;
    unsigned transferCount = 0;
    unsigned transferSize = 0;
    unsigned x;
    unsigned rnd = 0;

    NEXUS_Platform_GetDefaultSettings( &platformSettings );
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init( &platformSettings );

    initData( );

    {
        sampleDmaTransferManagerAllocSettings dmaTransferManagerSettings;
        sampleDmaTransferManagerHandle dmaTransferManagerHandle;

        srand(time(NULL));

        dmaTransferManagerSettings.keySlotHandle = CreateAndConfigureKeySlot();
        InvalidateKey( dmaTransferManagerSettings.keySlotHandle );
        LoadKey( dmaTransferManagerSettings.keySlotHandle );
        LoadIv( dmaTransferManagerSettings.keySlotHandle, g_iv );

        dmaTransferManagerHandle = sampleDmaTransferManager_Create( &dmaTransferManagerSettings );

        for( byteOffset = 0, transferSize = 1;
             byteOffset + transferSize < MAX_DATA_SIZE;
             byteOffset += transferSize, transferSize = rand()%1050 )
        {
            bytesTransferred += sampleDmaTransferManager_Transfer( dmaTransferManagerHandle,
                                                                  (char*)&g_encryptedData[byteOffset],
                                                                  (char*)&g_decryptedData[byteOffset],
                                                                  transferSize );
            transferCount++;
        }

        bytesTransferred += sampleDmaTransferManager_Flush( dmaTransferManagerHandle );

        sampleDmaTransferManager_Destroy( dmaTransferManagerHandle );

        /* count the errors */
        for( x = 0; x < bytesTransferred; x++ )
        {
            if( g_clearData[x] != g_decryptedData[x] )
            {
                errorCount++;
            }
        }

        printf("Transfers count[%d] requested[%d] actuall[%d] errors[%d]\n", transferCount,
                                                                             byteOffset,
                                                                             bytesTransferred,
                                                                             errorCount );

        NEXUS_Security_FreeKeySlot( dmaTransferManagerSettings.keySlotHandle );
    }

    unInitData();

    NEXUS_Platform_Uninit();
    return 0;
}


#define ALGORITHM_BLOCK_SIZE (16)

typedef struct{
    NEXUS_KeySlotHandle keySlotHandle;

    NEXUS_DmaHandle dmaHandle;
    BKNI_EventHandle dmaEvent;
    NEXUS_DmaJobHandle dmaJobHandle;
    NEXUS_DmaJobBlockSettings blockSettings[ALGORITHM_BLOCK_SIZE]; /* cached data. */
    unsigned numBlocks;         /* number of configure blocks. */
    unsigned numCachedBytes;    /* number of bytes currently cached. */
    char *pTransferCache;       /* cache for data remnant that is not a multiple of ALGORITHM_BLOCK_SIZE.
                                        - Max size will be alogorithm block size.
                                        - needs to be nexus device memory. */
    bool resetCrypto;
}sampleDmaTransferManagerInstance;


static sampleDmaTransferManagerHandle sampleDmaTransferManager_Create( sampleDmaTransferManagerAllocSettings *pSettings )
{
    sampleDmaTransferManagerInstance *pThis;
    NEXUS_DmaJobSettings jobSettings;
    unsigned x = 0;

    /* allocate object instance data. */
    pThis = (sampleDmaTransferManagerInstance*)malloc( sizeof(*pThis));
    BDBG_ASSERT( pThis );
    BKNI_Memset( pThis, 0, sizeof(*pThis) );

    /* allocate transfer cache */
    NEXUS_Memory_Allocate( ALGORITHM_BLOCK_SIZE, NULL, (void*)&(pThis->pTransferCache) );
    BDBG_ASSERT( pThis->pTransferCache );

    /* allocate DMA resources. */
    pThis->dmaHandle = NEXUS_Dma_Open( 0, NULL );
    BDBG_ASSERT( pThis->dmaHandle );

    /* reset the transfer blocks. */
    for( x=0; x<ALGORITHM_BLOCK_SIZE; x++ )
    {
        NEXUS_DmaJob_GetDefaultBlockSettings( &(pThis->blockSettings[x]) );
    }
    pThis->keySlotHandle = pSettings->keySlotHandle;

    BKNI_CreateEvent( &pThis->dmaEvent );
    BDBG_ASSERT( pThis->dmaEvent );

    NEXUS_DmaJob_GetDefaultSettings( &jobSettings );
    jobSettings.numBlocks                   = ALGORITHM_BLOCK_SIZE;
    jobSettings.keySlot                     = pThis->keySlotHandle;
    jobSettings.dataFormat                  = NEXUS_DmaDataFormat_eBlock;
    jobSettings.completionCallback.callback = CompleteCallback;
    jobSettings.completionCallback.context  = pThis->dmaEvent;
    pThis->dmaJobHandle = NEXUS_DmaJob_Create( pThis->dmaHandle, &jobSettings );
    BDBG_ASSERT( pThis->dmaJobHandle );

    pThis->resetCrypto = true;

    return (sampleDmaTransferManagerHandle)pThis;
}

static void sampleDmaTransferManager_Destroy( sampleDmaTransferManagerHandle handle )
{
    sampleDmaTransferManagerInstance *pThis = (sampleDmaTransferManagerInstance*) handle;

    /* free resources. */
    NEXUS_Memory_Free( pThis->pTransferCache );
    BKNI_DestroyEvent( pThis->dmaEvent );
    NEXUS_DmaJob_Destroy( pThis->dmaJobHandle );
    NEXUS_Dma_Close( pThis->dmaHandle );

    /* free instance memory. */
    BKNI_Memset( pThis, 0, sizeof(*pThis) );
    free( pThis );

    return;
}

/* transfer a data block.
    - function is synchronous, it will block until data is transfered.
    - if size is not a muiltipe of algorithm block size, the remnant will be cached locally.
    - the lenght of the actuall transfer will be returned. This may be less that or greater than the
      specified size.
*/
static unsigned sampleDmaTransferManager_Transfer( sampleDmaTransferManagerHandle handle,
                                          char* pSource,
                                          char* pDestination,
                                          unsigned size )
{
    sampleDmaTransferManagerInstance *pThis = (sampleDmaTransferManagerInstance*) handle;
    unsigned transferSize = 0;  /* the number of bytes that will actually be DMAed on the call */
    unsigned remnant = 0;
    unsigned x = 0;
    NEXUS_Error rc;

    if( size == 0 ) return 0;

    remnant = (pThis->numCachedBytes + size) % ALGORITHM_BLOCK_SIZE;
    transferSize = ((pThis->numCachedBytes + size)/ALGORITHM_BLOCK_SIZE)*ALGORITHM_BLOCK_SIZE;

    if( transferSize ) /* have we enough data for a transfer? */
    {
        NEXUS_DmaJobBlockSettings *pBlockSettings = &pThis->blockSettings[pThis->numBlocks];

        pBlockSettings->pSrcAddr  = pSource;
        pBlockSettings->pDestAddr = pDestination;
        pBlockSettings->blockSize = size-remnant;
        pBlockSettings->cached = true;
        pBlockSettings->resetCrypto = pThis->resetCrypto;
        if( pThis->numBlocks == 0 )
        {
            pBlockSettings->scatterGatherCryptoStart = true;
        }
        pBlockSettings->scatterGatherCryptoEnd = true;

        pThis->numBlocks++;

        rc = NEXUS_DmaJob_ProcessBlocks( pThis->dmaJobHandle, &pThis->blockSettings[0], pThis->numBlocks );

        if( rc == NEXUS_DMA_QUEUED )
        {
            NEXUS_DmaJobStatus jobStatus;
            BKNI_WaitForEvent( pThis->dmaEvent, BKNI_INFINITE );
            NEXUS_DmaJob_GetStatus( pThis->dmaJobHandle, &jobStatus );
            BDBG_ASSERT( jobStatus.currentState == NEXUS_DmaJobState_eComplete );
        }

        for( x=0; x < pThis->numBlocks; x++ )
        {
           NEXUS_DmaJob_GetDefaultBlockSettings( &(pThis->blockSettings[x]) );
        }
        pThis->resetCrypto = false;

        pThis->numBlocks = 0;
        pThis->numCachedBytes = 0;

        if( remnant )
        {   /* updated source and destination. */
            pSource += size-remnant;
            pDestination += size-remnant;
        }
    }

    if( remnant )
    {
        NEXUS_DmaJobBlockSettings *pBlockSettings = &pThis->blockSettings[pThis->numBlocks];
        unsigned remnantDelta = remnant - pThis->numCachedBytes;

        BKNI_Memcpy( &pThis->pTransferCache[pThis->numCachedBytes], pSource, remnantDelta );
        pBlockSettings->pSrcAddr = &pThis->pTransferCache[pThis->numCachedBytes];
        pBlockSettings->pDestAddr = pDestination;
        pBlockSettings->blockSize = remnantDelta;
        pBlockSettings->cached = true;
        pBlockSettings->resetCrypto = pThis->resetCrypto;
        if( pThis->numBlocks == 0 )
        {
            pBlockSettings->scatterGatherCryptoStart = true;
        }

        pThis->numCachedBytes += remnantDelta;
        pThis->numBlocks++;
        pThis->resetCrypto = false;
    }

    return transferSize;
}

/* flush all blocks through the DMA, even if they are not a multiple of algorithm block size. */
static unsigned sampleDmaTransferManager_Flush( sampleDmaTransferManagerHandle handle )
{
    sampleDmaTransferManagerInstance *pThis = (sampleDmaTransferManagerInstance*) handle;
    NEXUS_DmaJobBlockSettings *pBlockSettings = &pThis->blockSettings[pThis->numBlocks];
    unsigned transferSize = 0;  /* the number of bytes that will actually be DMAed on the call */
    NEXUS_Error rc;
    unsigned x = 0;

    if( pThis->numBlocks == 0 )
    {
        return 0;
    }

    pBlockSettings = &pThis->blockSettings[pThis->numBlocks-1];
    pBlockSettings->scatterGatherCryptoEnd = true;

    rc = NEXUS_DmaJob_ProcessBlocks( pThis->dmaJobHandle, &pThis->blockSettings[0], pThis->numBlocks );

    if( rc == NEXUS_DMA_QUEUED )
    {
        NEXUS_DmaJobStatus jobStatus;
        BKNI_WaitForEvent( pThis->dmaEvent, BKNI_INFINITE );
        NEXUS_DmaJob_GetStatus( pThis->dmaJobHandle, &jobStatus );
        BDBG_ASSERT( jobStatus.currentState == NEXUS_DmaJobState_eComplete );
    }

    transferSize = pThis->numCachedBytes;

    /* reset blocks to default. */
    for( x=0; x < pThis->numBlocks; x++ )
    {
       NEXUS_DmaJob_GetDefaultBlockSettings( &(pThis->blockSettings[x]) );
    }
    pThis->numBlocks = 0;
    pThis->numCachedBytes = 0;

    return transferSize;
}

#else /* NEXUS_HAS_SECURITY */
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform!\n");
    return -1;
}
#endif

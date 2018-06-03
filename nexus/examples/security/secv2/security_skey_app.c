/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#if NEXUS_HAS_SECURITY && (NEXUS_SECURITY_API_VERSION==2)

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "nexus_platform.h"
#include "nexus_memory.h"
#include "nexus_security.h"
#include "nexus_keyslot.h"
#include "nexus_dma.h"
#include "nexus_security_datatypes.h"

#define DEBUG_PRINT_ARRAY(description_txt,in_size,in_ptr) {             \
    int x_offset;                                                       \
    printf("[%s][%u]", description_txt, (unsigned)in_size );                      \
    for( x_offset = 0; x_offset < (int)(in_size); x_offset++ )          \
    {                                                                   \
        if( x_offset%16 == 0 ) { printf("\n"); }                        \
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


int main(int argc, char **argv)
{
    unsigned i;
    NEXUS_Error rc;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_SecurityCapabilities securityCapabilities;
    NEXUS_KeySlotHandle keyslotHandle = NULL;
    NEXUS_KeySlotAllocateSettings keyslotAllocSettings;
    NEXUS_KeySlotSettings keyslotSettings;
    NEXUS_KeySlotEntrySettings keyslotEntrySettings;
    NEXUS_KeySlotKey slotKey;
    NEXUS_DmaHandle           dma;
    NEXUS_DmaJobSettings      jobSettings;
    NEXUS_DmaJobHandle        dmaJob;
    NEXUS_DmaJobBlockSettings blockSettings;
    NEXUS_DmaJobStatus        jobStatus;
    BKNI_EventHandle          dmaEvent = NULL;
    uint8_t *pSrc = NULL;
    uint8_t *pDest = NULL;
    uint8_t key[16]        = { 0x8d, 0x2e, 0x60, 0x36, 0x5f, 0x17, 0xc7, 0xdf, 0x10, 0x40, 0xd7, 0x50, 0x1b, 0x4a, 0x7b, 0x5a };
    uint8_t plaintext[16]  = { 0x59, 0xb5, 0x08, 0x8e, 0x6d, 0xad, 0xc3, 0xad, 0x5f, 0x27, 0xa4, 0x60, 0x87, 0x2d, 0x59, 0x29 };
    uint8_t ciphertext[16] = { 0xe6, 0x42, 0x55, 0x6d, 0x1c, 0x30, 0x1b, 0xaf, 0x2e, 0xba, 0x96, 0x31, 0x49, 0xcd, 0xec, 0x79  };


    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

    NEXUS_GetSecurityCapabilities( &securityCapabilities );

    printf("\nSecurity Capabilities:\n");
    printf("Zeus Version    : %d.%d.%d:\n", securityCapabilities.version.zeus.major
                                          , securityCapabilities.version.zeus.minor
                                          , securityCapabilities.version.zeus.subminor );
    printf("Firmware Version: %d.%d.%d ", securityCapabilities.version.bfw.major
                                        , securityCapabilities.version.bfw.minor
                                        , securityCapabilities.version.bfw.subminor );

    if( securityCapabilities.firmwareEpoch.valid )
    {
        printf("Epoch[0x%X]\n", securityCapabilities.firmwareEpoch.value );
    }
    else
    {
        printf("Epoch[not-available]\n");
    }

    printf("KeySlot types\n");

    for( i = 0; i < NEXUS_KeySlotType_eMax; i++ )
    {
        printf("            Type%d [%d] \n", i, securityCapabilities.numKeySlotsForType[i] );
    }

    NEXUS_KeySlot_GetDefaultAllocateSettings( &keyslotAllocSettings );
    keyslotAllocSettings.owner = NEXUS_SecurityCpuContext_eHost;
    keyslotAllocSettings.slotType = NEXUS_KeySlotType_eIvPerEntry;
    keyslotAllocSettings.useWithDma = true;
    keyslotHandle = NEXUS_KeySlot_Allocate( &keyslotAllocSettings );
    if( !keyslotHandle ){ printf( "Can't allocate keyslot\n" ); return -1; }

    NEXUS_KeySlot_GetSettings( keyslotHandle, &keyslotSettings );
    #if 0
    for( i = 0; i < NEXUS_SecurityRegion_eMax; i++ ) {
        printf("      source[%d] [%d] \n", keyslotSettings.regions.source[i], i  );
        printf("       Gpipe[%d] [%d] \n", keyslotSettings.regions.destinationGPipe[i], i  );
        printf("       Rpipe[%d] [%d] \n", keyslotSettings.regions.destinationRPipe[i], i  );
    }
    #endif
    rc = NEXUS_KeySlot_SetSettings( keyslotHandle, &keyslotSettings );
    if( rc != NEXUS_SUCCESS ) { BERR_TRACE( rc ); goto exit; }

    NEXUS_KeySlot_GetEntrySettings( keyslotHandle,
                                    NEXUS_KeySlotBlockEntry_eCpsClear,
                                    &keyslotEntrySettings );

    keyslotEntrySettings.algorithm       = NEXUS_CryptographicAlgorithm_eAes128;
    keyslotEntrySettings.algorithmMode   = NEXUS_CryptographicAlgorithmMode_eEcb;
    keyslotEntrySettings.terminationMode = NEXUS_KeySlotTerminationMode_eClear;
    keyslotEntrySettings.rPipeEnable     = true;
    keyslotEntrySettings.gPipeEnable     = true;
    rc = NEXUS_KeySlot_SetEntrySettings( keyslotHandle,
                                         NEXUS_KeySlotBlockEntry_eCpsClear,
                                         &keyslotEntrySettings );
    if( rc != NEXUS_SUCCESS ) { BERR_TRACE( rc ); goto exit; }

    slotKey.size = sizeof(key);
	BKNI_Memcpy( slotKey.key, key, slotKey.size );

    rc = NEXUS_KeySlot_SetEntryKey( keyslotHandle,
                                    NEXUS_KeySlotBlockEntry_eCpsClear,
                                    &slotKey );
    if( rc != NEXUS_SUCCESS ) { BERR_TRACE( rc ); goto exit; }

	/* Open DMA handle */
    dma = NEXUS_Dma_Open(0, NULL);

    /* and DMA event */
    BKNI_CreateEvent(&dmaEvent);

    NEXUS_Memory_Allocate( sizeof(plaintext), NULL, (void **)&pSrc );
    NEXUS_Memory_Allocate( sizeof(plaintext), NULL, (void **)&pDest );

	BKNI_Memcpy( pSrc, plaintext, sizeof(plaintext) );
	BKNI_Memset( pDest, 0, sizeof(plaintext) );

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
    blockSettings.blockSize                 = sizeof(plaintext);
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

    NEXUS_DmaJob_Destroy( dmaJob );
    NEXUS_Dma_Close( dma );
    BKNI_DestroyEvent( dmaEvent );

    if( BKNI_Memcmp( pDest, ciphertext, sizeof(plaintext) ) )
	{
        printf("    FAILED!\n");
		DEBUG_PRINT_ARRAY("key:", sizeof(plaintext), key );
		DEBUG_PRINT_ARRAY("Source:", sizeof(plaintext), pSrc );
		DEBUG_PRINT_ARRAY("Destination", sizeof(plaintext), pDest );
	}
	else
	{
		printf("    PASSED!\n");
	}

exit:

    if( pSrc ) NEXUS_Memory_Free( pSrc );
    if( pDest ) NEXUS_Memory_Free( pDest );
    if (keyslotHandle) NEXUS_KeySlot_Free( keyslotHandle );

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

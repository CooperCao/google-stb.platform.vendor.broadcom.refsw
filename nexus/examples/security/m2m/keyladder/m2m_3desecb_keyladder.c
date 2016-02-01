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

/* This example shows how to use keyladder with Cuskey+Swizzle for root key generation.
   Due to differences in HW design, the value provided in this sample only works for
   settop chips like 740x.  For other chips, the key values may need to be changed for
   the test to pass
*/

#if NEXUS_HAS_SECURITY  && (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 1)

/* #error("should not get here %d ", NEXUS_HAS_SECURITY )*/

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

#define DMA_COMPLETE_WAIT_TIMEOUT ( 10000 )

/* Job completion callback */

static void CompleteCallback ( void *pParam, int iParam )
{
    BSTD_UNUSED ( iParam );
    fprintf ( stderr, "CompleteCallback:%#lx fired\n", ( unsigned long ) pParam );
    BKNI_SetEvent ( pParam );
}

void PrintBlock ( uint8_t * pData )
{
    unsigned int    i;
    for ( i = 0; i < DMA_BLOCK; i++ )
    {
        printf ( " %x ", pData[i] );
    }
    printf ( "\n\n" );
}

static int Security_AllocateVkl ( NEXUS_SecurityVirtualKeyladderID * vkl )
{
    NEXUS_SecurityVKLSettings vklSettings;
    NEXUS_VirtualKeyLadderHandle vklHandle;
    NEXUS_VirtualKeyLadderInfo vklInfo;

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

	/* For Zeus 4.2 or later
	if ((vklInfo.vkl & 0x7F ) >= NEXUS_SecurityVirtualKeyLadderID_eMax)
	{
        printf ( "\nAllocate VKL failed with invalid VKL Id.\n" );
        return 1;
	}
	*/

    *vkl = vklInfo.vkl;

	return 0;
}

int main ( void )
{
    NEXUS_PlatformSettings platformSettings;

    NEXUS_DmaHandle dma;
    NEXUS_DmaJobSettings jobSettings;
    NEXUS_DmaJobHandle dmaJob;
    NEXUS_DmaJobBlockSettings blockSettings;
    /*NEXUS_DmaJobStatus jobStatus; */
    NEXUS_Error     rc;
    BKNI_EventHandle dmaEvent = NULL;
    NEXUS_SecurityKeySlotSettings keySettings;
    NEXUS_KeySlotHandle encKeyHandle, encKeyHandle2;
    NEXUS_SecurityAlgorithmSettings NexusConfig;
    NEXUS_SecurityClearKey key;
    NEXUS_SecurityEncryptedSessionKey encryptedSessionkey;
    NEXUS_SecurityEncryptedControlWord encrytedCW;
	NEXUS_SecurityVirtualKeyladderID vklId;

    uint8_t        *pSrc, *pDest, *pDest2;
    unsigned int    i;

    uint8_t         keys[16] =
        { 0xbe, 0xf9, 0xb0, 0x67, 0x13, 0xb8, 0xbc, 0x87, 0xbc, 0xfb, 0xb2, 0x69, 0x13, 0xba, 0xbe, 0x8b };
    unsigned char   ucProcInForKey3[16] = {
        0x0f, 0x09, 0xa2, 0x06, /*high key */
        0x19, 0x88, 0xb6, 0x89,
        0x28, 0xeb, 0x90, 0x2e, /*low key  use twice */
        0xb2, 0x36, 0x18, 0x88
    };

    unsigned char   ucProcInKey4[16] =
        { 0xe4, 0x62, 0x75, 0x1b, 0x5d, 0xd4, 0xbc, 0x02, 0x27, 0x9e, 0xc9, 0x6c, 0xc8, 0x66, 0xec, 0x10 };

    /* Platform init */
    NEXUS_Platform_GetDefaultSettings ( &platformSettings );
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init ( &platformSettings );

	/* request for an VKL to use */
	if (Security_AllocateVkl ( &vklId ))
	{
		printf ( "\nAllocate VKL failed.\n");
		return 1;
	}

    /* Allocate AV keyslots */
    NEXUS_Security_GetDefaultKeySlotSettings ( &keySettings );
    keySettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
    encKeyHandle = NEXUS_Security_AllocateKeySlot ( &keySettings );
    if ( !encKeyHandle )
    {
        printf ( "\nAllocate keyslot failed \n" );
        return 1;
    }

    printf ( "\nStep 1\n\n\n\n" );
    NEXUS_Security_GetDefaultKeySlotSettings ( &keySettings );
    keySettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
    encKeyHandle2 = NEXUS_Security_AllocateKeySlot ( &keySettings );
    if ( !encKeyHandle2 )
    {
        printf ( "\nAllocate keyslot failed \n" );
        return 1;
    }

    printf ( "\nStep 2\n\n\n\n" );

    /* Set up key for clear key */
    NEXUS_Security_GetDefaultAlgorithmSettings ( &NexusConfig );
    NexusConfig.algorithm = NEXUS_SecurityAlgorithm_e3DesAba;
    NexusConfig.algorithmVar = NEXUS_SecurityAlgorithmVariant_eEcb;
    NexusConfig.operation = NEXUS_SecurityOperation_eEncrypt;
    NexusConfig.keyDestEntryType = NEXUS_SecurityKeyType_eOdd;

    if ( NEXUS_Security_ConfigAlgorithm ( encKeyHandle, &NexusConfig ) != 0 )
    {
        printf ( "\nConfigAlg clear keyslot failed \n" );
        return 1;
    }

    printf ( "\nStep 3\n\n\n\n" );

    NEXUS_Security_GetDefaultClearKey ( &key );
    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    key.keySize = sizeof ( keys );
    BKNI_Memcpy ( key.keyData, keys, sizeof ( keys ) );
    printf ( "\nStep 3.5  %x\n\n\n\n", ( unsigned int ) encKeyHandle );
    if ( NEXUS_Security_LoadClearKey ( encKeyHandle, &key ) != 0 )
    {
        printf ( "\nLoad encryption key failed \n" );
        return 1;
    }

    printf ( "\nStep 4\n\n\n\n" );

    /* Set up key for keyladder  */
    if ( NEXUS_Security_ConfigAlgorithm ( encKeyHandle2, &NexusConfig ) != 0 )
    {
        printf ( "\nConfigAlg keyladder keyslot failed \n" );
        return 1;
    }

    /* Load session key */
    NEXUS_Security_GetDefaultSessionKeySettings ( &encryptedSessionkey );
    encryptedSessionkey.keyladderType = NEXUS_SecurityKeyladderType_e3Des;
    encryptedSessionkey.swizzleType = NEXUS_SecuritySwizzleType_eSwizzle0;
    encryptedSessionkey.cusKeyL = 0x00;
    encryptedSessionkey.cusKeyH = 0x00;
    encryptedSessionkey.cusKeyVarL = 0x00;
    encryptedSessionkey.cusKeyVarH = 0xFF;
    encryptedSessionkey.rootKeySrc = NEXUS_SecurityRootKeySrc_eCuskey;
    encryptedSessionkey.bRouteKey = false;
    encryptedSessionkey.operation = NEXUS_SecurityOperation_eDecrypt;
    encryptedSessionkey.operationKey2 = NEXUS_SecurityOperation_eEncrypt;
    encryptedSessionkey.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    encryptedSessionkey.virtualKeyLadderID = vklId;
    encryptedSessionkey.custSubMode = NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4;
    /*encryptedSessionkey.pKeyData          = ucProcInForKey3; */
    BKNI_Memcpy ( encryptedSessionkey.keyData, ucProcInForKey3, sizeof ( ucProcInForKey3 ) );

    if ( NEXUS_Security_GenerateSessionKey ( encKeyHandle2, &encryptedSessionkey ) != 0 )
    {
        printf ( "\nLoading session key failed \n" );
        return 1;
    }

    printf ( "\nStep 5\n\n\n\n" );

    /* Load CW */
    NEXUS_Security_GetDefaultControlWordSettings ( &encrytedCW );
    encrytedCW.keyladderType = NEXUS_SecurityKeyladderType_e3Des;
    encrytedCW.keySize = sizeof ( ucProcInKey4 );
    encrytedCW.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    /*encrytedCW.pKeyData = ucProcInKey4; */
    BKNI_Memcpy ( encrytedCW.keyData, ucProcInKey4, encrytedCW.keySize );
    encrytedCW.operation = NEXUS_SecurityOperation_eDecrypt;
    encrytedCW.bRouteKey = true;
    encrytedCW.virtualKeyLadderID = vklId;
    encrytedCW.custSubMode = NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4;

    if ( NEXUS_Security_GenerateControlWord ( encKeyHandle2, &encrytedCW ) )
    {
        printf ( "\nLoading session key failed for video ODD key\n" );
        return 1;
    }

    printf ( "\nStep 6\n\n\n\n" );

    /* Open DMA handle */
    dma = NEXUS_Dma_Open ( 0, NULL );

    NEXUS_Memory_Allocate ( DMA_BLOCK, NULL, ( void * ) &pSrc );
    NEXUS_Memory_Allocate ( DMA_BLOCK, NULL, ( void * ) &pDest );
    NEXUS_Memory_Allocate ( DMA_BLOCK, NULL, ( void * ) &pDest2 );

    memset ( pSrc, 0, DMA_BLOCK * sizeof ( unsigned char ) );

    NEXUS_DmaJob_GetDefaultSettings ( &jobSettings );

    /* create event on first use and add callback to the job */
    if ( dmaEvent == NULL )
        BKNI_CreateEvent ( &dmaEvent );
    jobSettings.completionCallback.callback = CompleteCallback;
    jobSettings.completionCallback.context = dmaEvent;

    jobSettings.numBlocks = 1;
    jobSettings.keySlot = encKeyHandle;
    jobSettings.dataFormat = NEXUS_DmaDataFormat_eBlock;

    dmaJob = NEXUS_DmaJob_Create ( dma, &jobSettings );

    NEXUS_DmaJob_GetDefaultBlockSettings ( &blockSettings );
    blockSettings.pSrcAddr = pSrc;
    blockSettings.pDestAddr = pDest;
    blockSettings.blockSize = DMA_BLOCK;
    blockSettings.resetCrypto = true;
    blockSettings.scatterGatherCryptoStart = true;
    blockSettings.scatterGatherCryptoEnd = true;
    blockSettings.cached = true;

    rc = NEXUS_DmaJob_ProcessBlocks ( dmaJob, &blockSettings, 1 );
    if ( rc == NEXUS_DMA_QUEUED )
    {
        if ( BKNI_WaitForEvent ( dmaEvent, DMA_COMPLETE_WAIT_TIMEOUT ) )
        {
            printf ( " DMA Fails\n" );

            NEXUS_DmaJob_Destroy ( dmaJob );
            return 1;
        }
    }
    else if ( rc != NEXUS_SUCCESS )
    {
        printf ( "NEXUS_DmaJob_ProcessBlocks Fails\n" );
        NEXUS_DmaJob_Destroy ( dmaJob );
        return 1;
    }

    NEXUS_DmaJob_Destroy ( dmaJob );

    printf ( " DMA Done\n" );

    PrintBlock ( pSrc );
    PrintBlock ( pDest );
    PrintBlock ( pDest2 );

    NEXUS_DmaJob_GetDefaultSettings ( &jobSettings );
    jobSettings.numBlocks = 1;
    jobSettings.keySlot = encKeyHandle2;
    jobSettings.dataFormat = NEXUS_DmaDataFormat_eBlock;
    jobSettings.completionCallback.callback = CompleteCallback;
    jobSettings.completionCallback.context = dmaEvent;

    dmaJob = NEXUS_DmaJob_Create ( dma, &jobSettings );

    NEXUS_DmaJob_GetDefaultBlockSettings ( &blockSettings );
    blockSettings.pSrcAddr = pSrc;
    blockSettings.pDestAddr = pDest2;
    blockSettings.blockSize = DMA_BLOCK;
    blockSettings.resetCrypto = true;
    blockSettings.scatterGatherCryptoStart = true;
    blockSettings.scatterGatherCryptoEnd = true;
    blockSettings.cached = true;

    rc = NEXUS_DmaJob_ProcessBlocks ( dmaJob, &blockSettings, 1 );
    if ( rc == NEXUS_DMA_QUEUED )
    {
        if ( BKNI_WaitForEvent ( dmaEvent, DMA_COMPLETE_WAIT_TIMEOUT ) )
        {
            printf ( " DMA Fails\n" );

            NEXUS_DmaJob_Destroy ( dmaJob );
            return 1;
        }
    }
    else if ( rc != NEXUS_SUCCESS )
    {
        printf ( "NEXUS_DmaJob_ProcessBlocks Fails\n" );
        NEXUS_DmaJob_Destroy ( dmaJob );
        return 1;
    }

    NEXUS_DmaJob_Destroy ( dmaJob );

    PrintBlock ( pSrc );
    PrintBlock ( pDest );
    PrintBlock ( pDest2 );

    /* Make sure pSrc matches pDest 2 */
    for ( i = 0; i < DMA_BLOCK; i++ )
    {
        if ( pDest[i] != pDest2[i] )
        {
            printf ( "\nComparison failed at location %d\n", i );
            break;
        }
    }

    if ( i == DMA_BLOCK )
        printf ( "\nTest passed\n" );
    else
    {
        printf ( "\nSource buffer1 is: \n" );
        for ( i = 0; i < DMA_BLOCK; i++ )
        {
            printf ( "%02x ", pDest[i] );
        }
        printf ( "\nDestination buffer 2 is: \n" );
        for ( i = 0; i < DMA_BLOCK; i++ )
        {
            printf ( "%02x ", pDest2[i] );
        }
    }

    NEXUS_Dma_Close ( dma );

    NEXUS_Security_FreeKeySlot ( encKeyHandle );
    NEXUS_Security_FreeKeySlot ( encKeyHandle2 );

    return 0;
}

#else /* NEXUS_HAS_SECURITY && .. */
#include <stdio.h>
int main ( void )
{
    printf ( "This application is not supported on this platform!\n" );
    return -1;
}
#endif

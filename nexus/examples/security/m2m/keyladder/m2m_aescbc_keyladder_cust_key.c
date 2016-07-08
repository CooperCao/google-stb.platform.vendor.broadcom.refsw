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

#if NEXUS_HAS_SECURITY && (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 1)

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

/* Please define the following values to test. */
#define CUS_KEY_SELE_H (0x0)
#define CUS_KEY_SELE_L (0x0)
#define CUS_KEY_VAR_L  (0x3)
#define CUS_KEY_VAR_H  (0xFF)

/* Please define the following values to test. */
static unsigned char ucProcInForKey3[16] = { 0 };

/* Please define the following values to test. */
static unsigned char ucProcInKey4[16] = { 0 };

/* Please define the following values to test. */
static uint8_t  iv[16] = { 0 };

static int Security_AllocateVkl ( NEXUS_SecurityCustomerSubMode custSubMode, NEXUS_SecurityVirtualKeyladderID * vkl,
                                  NEXUS_VirtualKeyLadderHandle * vklHandle )
{
    NEXUS_SecurityVKLSettings vklSettings;
    NEXUS_VirtualKeyLadderInfo vklInfo;

    BDBG_ASSERT ( vkl );
    BDBG_ASSERT ( vklHandle );

    NEXUS_Security_GetDefaultVKLSettings ( &vklSettings );

    /* For pre Zeus 3.0, please set vklSettings.custSubMode */
    vklSettings.custSubMode = custSubMode;

    *vklHandle = NEXUS_Security_AllocateVKL ( &vklSettings );

    if ( !( *vklHandle ) )
    {
        printf ( "\nAllocate VKL failed \n" );
        return 1;
    }

    NEXUS_Security_GetVKLInfo ( *vklHandle, &vklInfo );
    printf ( "\nVKL Handle %p is allocated for VKL#%d\n", ( void * ) *vklHandle, vklInfo.vkl & 0x7F );

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

static void CompleteCallback ( void *pParam, int iParam )
{
    BSTD_UNUSED ( iParam );
    fprintf ( stderr, "CompleteCallback:%#lx fired\n", ( unsigned long ) pParam );
    BKNI_SetEvent ( pParam );
}

int main ( void )
{
    NEXUS_PlatformSettings platformSettings;

    NEXUS_DmaHandle dma;
    NEXUS_DmaJobSettings jobSettings;
    NEXUS_DmaJobHandle dmaJob;
    NEXUS_DmaJobBlockSettings blockSettings;
    NEXUS_DmaJobStatus jobStatus;
    NEXUS_Error     rc;
    BKNI_EventHandle dmaEvent = NULL;

    NEXUS_SecurityKeySlotSettings keySettings;
    NEXUS_KeySlotHandle encKeyHandle,
                    decKeyHandle;
    NEXUS_SecurityAlgorithmSettings NexusConfig;
    NEXUS_SecurityClearKey key;
    NEXUS_SecurityEncryptedSessionKey encryptedSessionkey;
    NEXUS_SecurityEncryptedControlWord encrytedCW;
    NEXUS_SecurityVirtualKeyladderID vklId;
    NEXUS_VirtualKeyLadderHandle vklHandle;
    uint8_t        *pSrc,
                   *pDest,
                   *pDest2;
    unsigned int    i;

    /* Platform init */
    NEXUS_Platform_GetDefaultSettings ( &platformSettings );
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init ( &platformSettings );

    /* Request for an VKL to use */
    if ( Security_AllocateVkl ( NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4, &vklId, &vklHandle ) )
    {
        printf ( "\nAllocate VKL failed.\n" );
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

    decKeyHandle = NEXUS_Security_AllocateKeySlot ( &keySettings );
    if ( !decKeyHandle )
    {
        printf ( "\nAllocate keyslot failed \n" );
        return 1;
    }

    /* Set up key for clear key */
    NEXUS_Security_GetDefaultAlgorithmSettings ( &NexusConfig );
    NexusConfig.algorithm = NEXUS_SecurityAlgorithm_eAes;
    NexusConfig.algorithmVar = NEXUS_SecurityAlgorithmVariant_eCbc;
    NexusConfig.operation = NEXUS_SecurityOperation_eEncrypt;
    NexusConfig.keyDestEntryType = NEXUS_SecurityKeyType_eOdd;

    /* ++++++++ */
    NexusConfig.terminationMode = NEXUS_SecurityTerminationMode_eClear;
    NexusConfig.ivMode = NEXUS_SecurityIVMode_eRegular;
    NexusConfig.solitarySelect = NEXUS_SecuritySolitarySelect_eClear;
    NexusConfig.caVendorID = 0x1234;
    NexusConfig.askmModuleID = NEXUS_SecurityAskmModuleID_eModuleID_4;
    NexusConfig.otpId = NEXUS_SecurityOtpId_eOtpVal;
    NexusConfig.testKey2Select = 0;

    if ( NEXUS_Security_ConfigAlgorithm ( encKeyHandle, &NexusConfig ) != 0 )
    {
        printf ( "\nConfigAlg clear keyslot failed \n" );
        return 1;
    }

    /* Set up key for keyladder  */
    NexusConfig.operation = NEXUS_SecurityOperation_eDecrypt;
    if ( NEXUS_Security_ConfigAlgorithm ( decKeyHandle, &NexusConfig ) != 0 )
    {
        printf ( "\nConfigAlg keyladder keyslot failed \n" );
        return 1;
    }

    /* Load session key */
    NEXUS_Security_GetDefaultSessionKeySettings ( &encryptedSessionkey );
    encryptedSessionkey.keyladderType = NEXUS_SecurityKeyladderType_e3Des;
    encryptedSessionkey.swizzleType = NEXUS_SecuritySwizzleType_eSwizzle0;
    encryptedSessionkey.cusKeyL = CUS_KEY_SELE_L;
    encryptedSessionkey.cusKeyH = CUS_KEY_SELE_H;
    encryptedSessionkey.cusKeyVarL = CUS_KEY_VAR_L;
    encryptedSessionkey.cusKeyVarH = CUS_KEY_VAR_H;
    encryptedSessionkey.keyGenCmdID = NEXUS_SecurityKeyGenCmdID_eKeyGen;
    encryptedSessionkey.sessionKeyOp = NEXUS_SecuritySessionKeyOp_eNoProcess;
    encryptedSessionkey.bASKMMode = false;
    encryptedSessionkey.rootKeySrc = NEXUS_SecurityRootKeySrc_eCuskey;
    encryptedSessionkey.bSwapAESKey = false;
    encryptedSessionkey.keyDestIVType = NEXUS_SecurityKeyIVType_eNoIV;
    encryptedSessionkey.operation = NEXUS_SecurityOperation_eDecrypt;
    encryptedSessionkey.operationKey2 = NEXUS_SecurityOperation_eEncrypt;
    encryptedSessionkey.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    encryptedSessionkey.bRouteKey = false;

    /* ++++++++ */
    encryptedSessionkey.custSubMode = NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4;
    encryptedSessionkey.virtualKeyLadderID = vklId;
    encryptedSessionkey.keyMode = NEXUS_SecurityKeyMode_eRegular;
    /* ++++++++ */
    BKNI_Memcpy ( encryptedSessionkey.keyData, ucProcInForKey3, sizeof ( ucProcInForKey3 ) );

    if ( NEXUS_Security_GenerateSessionKey ( encKeyHandle, &encryptedSessionkey ) != 0 )
    {
        printf ( "\nLoading encryption session key failed \n" );
        return 1;
    }
    if ( NEXUS_Security_GenerateSessionKey ( decKeyHandle, &encryptedSessionkey ) != 0 )
    {
        printf ( "\nLoading decryption session key failed \n" );
        return 1;
    }
    /* Load CW */
    NEXUS_Security_GetDefaultControlWordSettings ( &encrytedCW );
    encrytedCW.keyladderType = NEXUS_SecurityKeyladderType_e3Des;
    encrytedCW.keySize = sizeof ( ucProcInKey4 );
    encrytedCW.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    /* ++++++++ */
    encrytedCW.custSubMode = NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4;
    encrytedCW.virtualKeyLadderID = vklId;
    encrytedCW.keyMode = NEXUS_SecurityKeyMode_eRegular;
    /* ++++++++ */
    BKNI_Memcpy ( encrytedCW.keyData, ucProcInKey4, encrytedCW.keySize );
    encrytedCW.operation = NEXUS_SecurityOperation_eDecrypt;
    encrytedCW.bRouteKey = true;
    encrytedCW.keyDestIVType = NEXUS_SecurityKeyIVType_eNoIV;
    encrytedCW.keyGenCmdID = NEXUS_SecurityKeyGenCmdID_eKeyGen;
    encrytedCW.bSwapAESKey = false;
    if ( NEXUS_Security_GenerateControlWord ( encKeyHandle, &encrytedCW ) )
    {
        printf ( "\nLoading CW failed for encryption key\n" );
        return 1;
    }

    if ( NEXUS_Security_GenerateControlWord ( decKeyHandle, &encrytedCW ) )
    {
        printf ( "\nLoading CW failed for decryption key\n" );
        return 1;
    }

    /* Load IV */
    NEXUS_Security_GetDefaultClearKey ( &key );
    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    key.keyIVType = NEXUS_SecurityKeyIVType_eIV;
    key.dest = NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem;
    key.keySize = sizeof ( iv );
    BKNI_Memcpy ( key.keyData, iv, key.keySize );
    if ( NEXUS_Security_LoadClearKey ( encKeyHandle, &key ) != 0 )
    {
        printf ( "\nLoad encryption IV failed \n" );
        return 1;
    }

    if ( NEXUS_Security_LoadClearKey ( decKeyHandle, &key ) != 0 )
    {
        printf ( "\nLoad encryption IV failed \n" );
        return 1;
    }

    /* Open DMA handle */
    dma = NEXUS_Dma_Open ( 0, NULL );

    /* and DMA event */
    BKNI_CreateEvent ( &dmaEvent );

    NEXUS_Memory_Allocate ( DMA_BLOCK, NULL, ( void * ) &pSrc );
    NEXUS_Memory_Allocate ( DMA_BLOCK, NULL, ( void * ) &pDest );
    NEXUS_Memory_Allocate ( DMA_BLOCK, NULL, ( void * ) &pDest2 );

    memset ( pSrc, 0xBB, DMA_BLOCK * sizeof ( unsigned char ) );
    memset ( pDest, 0x55, DMA_BLOCK * sizeof ( unsigned char ) );
    memset ( pDest2, 0xAA, DMA_BLOCK * sizeof ( unsigned char ) );

    NEXUS_DmaJob_GetDefaultSettings ( &jobSettings );
    jobSettings.numBlocks = 1;
    jobSettings.keySlot = encKeyHandle;
    jobSettings.dataFormat = NEXUS_DmaDataFormat_eBlock;
    jobSettings.completionCallback.callback = CompleteCallback;
    jobSettings.completionCallback.context = dmaEvent;

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
        BKNI_WaitForEvent ( dmaEvent, BKNI_INFINITE );
        NEXUS_DmaJob_GetStatus ( dmaJob, &jobStatus );
        BDBG_ASSERT ( jobStatus.currentState == NEXUS_DmaJobState_eComplete );
    }
    NEXUS_DmaJob_Destroy ( dmaJob );

    NEXUS_DmaJob_GetDefaultSettings ( &jobSettings );
    jobSettings.numBlocks = 1;
    jobSettings.keySlot = decKeyHandle;
    jobSettings.dataFormat = NEXUS_DmaDataFormat_eBlock;
    jobSettings.completionCallback.callback = CompleteCallback;
    jobSettings.completionCallback.context = dmaEvent;

    dmaJob = NEXUS_DmaJob_Create ( dma, &jobSettings );

    NEXUS_DmaJob_GetDefaultBlockSettings ( &blockSettings );
    blockSettings.pSrcAddr = pDest;
    blockSettings.pDestAddr = pDest2;
    blockSettings.blockSize = DMA_BLOCK;
    blockSettings.resetCrypto = true;
    blockSettings.scatterGatherCryptoStart = true;
    blockSettings.scatterGatherCryptoEnd = true;
    blockSettings.cached = true;

    rc = NEXUS_DmaJob_ProcessBlocks ( dmaJob, &blockSettings, 1 );

    if ( rc == NEXUS_DMA_QUEUED )
    {
        BKNI_WaitForEvent ( dmaEvent, BKNI_INFINITE );
        NEXUS_DmaJob_GetStatus ( dmaJob, &jobStatus );
        BDBG_ASSERT ( jobStatus.currentState == NEXUS_DmaJobState_eComplete );
    }
    NEXUS_DmaJob_Destroy ( dmaJob );

    /* Make sure pSrc matches pDest 2 */
    for ( i = 0; i < DMA_BLOCK; i++ )
    {
        if ( pSrc[i] != pDest2[i] )
        {
            printf ( "\nComparison failed at location %d\n", i );
            break;
        }
    }

    if ( i == DMA_BLOCK )
        printf ( "\nTest passed\n" );

    printf ( "\nSource buffer is: \n" );
    for ( i = 0; i < DMA_BLOCK; i++ )
    {
        if ( i % 16 == 0 )
            printf ( "\n" );
        printf ( "%02x ", pSrc[i] );
    }
    printf ( "\nDestination buffer is: \n" );
    for ( i = 0; i < DMA_BLOCK; i++ )
    {
        if ( i % 16 == 0 )
            printf ( "\n" );
        printf ( "%02x ", pDest[i] );
    }
    printf ( "\nDestination buffer 2 is: \n" );
    for ( i = 0; i < DMA_BLOCK; i++ )
    {
        if ( i % 16 == 0 )
            printf ( "\n" );
        printf ( "%02x ", pDest2[i] );
    }

    BKNI_DestroyEvent ( dmaEvent );

    NEXUS_Dma_Close ( dma );

    NEXUS_Security_FreeKeySlot ( encKeyHandle );
    NEXUS_Security_FreeKeySlot ( decKeyHandle );

    NEXUS_Security_FreeVKL ( vklHandle );

    NEXUS_Memory_Free ( pSrc );
    NEXUS_Memory_Free ( pDest );
    NEXUS_Memory_Free ( pDest2 );

    NEXUS_Platform_Uninit (  );

    return 0;
}

#else /* NEXUS_HAS_SECURITY */
#include <stdio.h>
int main ( void )
{
    printf ( "This application is not supported on this platform!\n" );
    return -1;
}
#endif

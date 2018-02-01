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

#include "nexus_platform.h"
#if NEXUS_HAS_SECURITY && NEXUS_SECURITY_API_VERSION==2
#include "nexus_memory.h"
#include "nexus_dma.h"
#include "nexus_security.h"
#include "security_utils.h"

#define IN_XPT_TYPE  NEXUS_TransportType_eTs
#define OUT_XPT_TYPE  NEXUS_TransportType_eTs

#define XPT_TS_PACKET_NUM (1)

int securityUtil_PlatformInit( bool useDisplay )
{
    NEXUS_PlatformSettings platformSettings;

    NEXUS_Platform_GetDefaultSettings( &platformSettings );
    platformSettings.openFrontend = false;
    platformSettings.openOutputs = useDisplay;
    platformSettings.openCec = useDisplay;
    NEXUS_Platform_Init( &platformSettings );

    return 0;
}

int securityUtil_PlatformUnInit( void )
{
    NEXUS_Platform_Uninit(  );

    return 0;
}

static void CompleteCallback( void *pParam, int iParam )
{
    BSTD_UNUSED( iParam );
    BKNI_SetEvent( pParam );
    return;
}

int securityUtil_DmaTransfer( NEXUS_KeySlotHandle keyslotHandle,
                              uint8_t * pSrc,
                              uint8_t * pDest,
                              NEXUS_DmaDataFormat dataFormat,
                              size_t dataSize,
                              bool securityBtp )
{
    NEXUS_Error   rc;
    NEXUS_DmaHandle dma;
    NEXUS_DmaJobSettings jobSettings;
    NEXUS_DmaJobHandle dmaJob;
    NEXUS_DmaJobStatus jobStatus;
    NEXUS_DmaJobBlockSettings blockSettings;
    BKNI_EventHandle dmaEvent = NULL;

    if( !pSrc || !pDest ) {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    dma = NEXUS_Dma_Open( 0, NULL );

    BKNI_CreateEvent( &dmaEvent );

    NEXUS_DmaJob_GetDefaultSettings( &jobSettings );

    jobSettings.numBlocks = 1;
    jobSettings.keySlot = keyslotHandle;
    jobSettings.dataFormat = dataFormat; /* NEXUS_DmaDataFormat_eBlock, or eTS; */
    jobSettings.completionCallback.callback = CompleteCallback;
    jobSettings.completionCallback.context = dmaEvent;

    dmaJob = NEXUS_DmaJob_Create( dma, &jobSettings );

    NEXUS_DmaJob_GetDefaultBlockSettings( &blockSettings );
    blockSettings.pSrcAddr = pSrc;
    blockSettings.pDestAddr = pDest;
    blockSettings.blockSize = dataSize;
    blockSettings.resetCrypto = true;
    blockSettings.cached = true;
    blockSettings.scatterGatherCryptoStart = true;
    blockSettings.scatterGatherCryptoEnd = true;

    blockSettings.securityBtp = securityBtp;

    rc = NEXUS_DmaJob_ProcessBlocks( dmaJob, &blockSettings, 1 );
    if( rc == NEXUS_DMA_QUEUED ) {
        /* BKNI_WaitForEvent( dmaEvent, BKNI_INFINITE ); */
        BKNI_WaitForEvent( dmaEvent, 1000);
        NEXUS_DmaJob_GetStatus( dmaJob, &jobStatus );
        BDBG_ASSERT( jobStatus.currentState == NEXUS_DmaJobState_eComplete );
    }

    NEXUS_DmaJob_Destroy( dmaJob );
    NEXUS_Dma_Close( dma );
    BKNI_DestroyEvent( dmaEvent );

    return NEXUS_SUCCESS;
}

/*
   Composit a single TS packet or multiple packets for tests.
   XPT_TS_PACKET_NUM defines the number of the packets.
*/
void CompositTSPackets( uint8_t * xptTSPackets, unsigned int scValue )
{
    int i;

    if( scValue > 3 ) {
        printf( "\nError: invalid transport TS SC value %d", scValue );
        return;
    }

    /* Make up the packet heads */
    for( i = 0; i < XPT_TS_PACKET_NUM; i++ ) {
        xptTSPackets[XPT_TS_PACKET_SIZE * i] = 0x47;
        xptTSPackets[XPT_TS_PACKET_SIZE * i + 1] = ( VIDEO_PID & 0xFF00 ) >> 8;
        xptTSPackets[XPT_TS_PACKET_SIZE * i + 2] = VIDEO_PID & 0xFF;
        xptTSPackets[XPT_TS_PACKET_SIZE * i + 3] = ( scValue << 6 ) | 0x10 | ( ( 0x9 + i ) & 0xF );
    }

    /* Make up a TS packet payload. */
    for( i = XPT_TS_PACKET_HEAD_SIZE; i < XPT_TS_PACKET_SIZE; i++ ) {
        xptTSPackets[i] = i - XPT_TS_PACKET_HEAD_SIZE;
    }

    return;
}

size_t securityGetAlogrithmKeySize(
    NEXUS_CryptographicAlgorithm algorithm )
{
    size_t        key_size = 0;

    switch ( algorithm ) {
    case NEXUS_CryptographicAlgorithm_eDes:
        key_size = 8;
        break;
    case NEXUS_CryptographicAlgorithm_e3DesAba:
        key_size = 8 * 2;       /* Actually 8 * 2 */
        break;
    case NEXUS_CryptographicAlgorithm_e3DesAbc:
        key_size = 8 * 3;
        break;
    case NEXUS_CryptographicAlgorithm_eAes128:
        key_size = 128 / 8;
        break;
    case NEXUS_CryptographicAlgorithm_eAes192:
        key_size = 192 / 8;
        break;
    case NEXUS_CryptographicAlgorithm_eAes256:
    case NEXUS_CryptographicAlgorithm_eMulti2:
        key_size = 256 / 8;
        break;
    default:
        /* Invalid */
        key_size = 0;
        BDBG_ERR( ( "Can't get algorithm %d's key size", algorithm ) );
        break;
    }

    return key_size;
}

#endif

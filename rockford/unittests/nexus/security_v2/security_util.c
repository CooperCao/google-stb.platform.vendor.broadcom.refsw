/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include <stdio.h>

#if NEXUS_HAS_SECURITY && NEXUS_SECURITY_API_VERSION==2

#include <unistd.h>
#include <string.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "nexus_platform.h"
#include "nexus_memory.h"
#include "nexus_dma.h"
#include "nexus_security.h"
#include "security_util.h"

#define IN_XPT_TYPE  NEXUS_TransportType_eTs
#define OUT_XPT_TYPE  NEXUS_TransportType_eTs

#define XPT_TS_PACKET_NUM (1)

BDBG_MODULE( security_util );

securityUtil_options gOptions;

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
    jobSettings.dataFormat = dataFormat; /* normally, NEXUS_DmaDataFormat_eBlock; */
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

/* return true if matched, false otherwise. */
static bool parseBool(char *pName, char *pArg, bool *pBool )
{
    if( strcmp( pName, pArg ) == 0 ) { *pBool = true; }
    return false;
}

static bool parseString(char *pName, char *pArg, char *pStr, int size )
{
    int nameSize = strlen( pName );

    if( strncmp( pName, pArg, nameSize )  == 0 )
    {
        pArg += nameSize;
        memset( pStr, 0, size );
        strncpy( pStr, pArg, size-1 );
        return true;
    }
    return false;
}

static bool parseInt(char *pName, char *pArg, int *pInt )
{
    int nameSize = strlen( pName );

    if( strncmp( pName, pArg, nameSize ) == 0 )
    {
        int base = 10;
        pArg += nameSize;

        if( pArg[0] == '0' && ( pArg[1] == 'x' || pArg[1] == 'X' ) )
        {
            base = 16;
        }

        *pInt = (int)strtol( pArg, NULL, base );
        return true;
    }
    return false;
}

securityUtil_options securityUtil_ParseOptions( int argc, char **argv )
{
    int i;
    char buf[6];

    /* set defaults. */
    gOptions.groupNum = -1;
    gOptions.testNum  = -1;

    for( i = 1; i < argc; i++  )
    {
        parseString( "-f=", argv[i], buf, sizeof(buf)   );  /*test!*/
        parseBool("-v",     argv[i], &gOptions.verbose  );
        parseBool("-h",     argv[i], &gOptions.showHelp );
        parseBool("-help",  argv[i], &gOptions.showHelp );
        parseBool("--help", argv[i], &gOptions.showHelp );
        parseBool("-m",     argv[i], &gOptions.showMenu );
        parseBool("menu",   argv[i], &gOptions.showMenu ); /* deprecated */
        parseInt("-g=",     argv[i], &gOptions.groupNum );
        parseInt("-t=",     argv[i], &gOptions.testNum  );

        /*parse more args!!!!*/
    }

    return gOptions;
}

securityUtil_options securityUtil_GetOptions( void )
{
    return gOptions;
}


void verboseLog( char* pFormat, ...  )
{
    va_list args;

    if( gOptions.verbose )
    {
        va_start( args, pFormat );
        vprintf( pFormat, args );
        va_end( args );
    }
    return;
}
void  securityFramework_registerGroup( securityTestGroup groupId, char *pGroupName )
{
    testGroup *pGroup = NULL;

    if( groupId >= securityTestGroup_max ){ printf( "Invalid Group id [%d]\n", groupId ); return; }
    if( !pGroupName ) { printf("Invalid pGroupName [NULL]");  return; }

    pGroup = &gData.groups[groupId];

    if( pGroup->registered ){ printf("Group[%d] already registered to [%s]\n", groupId, pGroup->groupName); return; }

    BLST_S_INIT( &pGroup->testList );

    pGroup->registered = true;

    strcpy(pGroup->groupName, pGroupName );

    return;
}


/* All test must be registered with this function.  */
void  securityFramework_registerTest( securityTestFn registeredTest )
{
    groupTestItem *pNewTest = NULL;
    groupTestItem *pNextTest = NULL;
    groupTestItem *pPreviousTest = NULL;

    pNewTest = BKNI_Malloc( sizeof(*pNewTest) );
    assert( pNewTest );

    /* call the the test function to retrieve information from it. */
    memset( pNewTest, 0, sizeof(*pNewTest) );
    pNewTest->args.enquire = true;
    registeredTest( &pNewTest->args );
    pNewTest->args.enquire = false;
    pNewTest->registeredTest = registeredTest;

    if( pNewTest->args.number == 0 ){
        printf( "ERROR: 0 not a valid test number [%s]\n", pNewTest->args.name );
        goto error;
    }

    if( pNewTest->args.group >= securityTestGroup_max ) {
        printf( "ERROR: 0 not a valid GROUP [%s]\n", pNewTest->args.name );
        goto error;
    }

    /* insert the test items in-order of test number */
    pNextTest = BLST_S_FIRST( &gData.groups[pNewTest->args.group].testList );
    while( pNextTest && ( pNewTest->args.number >= pNextTest->args.number ) )
    {
        pPreviousTest = pNextTest;
        pNextTest = BLST_S_NEXT( pPreviousTest, next );
    }
    if( !pPreviousTest )
    {
        BLST_S_INSERT_HEAD( &gData.groups[pNewTest->args.group].testList, pNewTest, next );
    }
    else
    {
        BLST_S_INSERT_AFTER( &gData.groups[pNewTest->args.group].testList, pPreviousTest, pNewTest, next );
    }

    return;

error:
    BKNI_Free( pNewTest );
    return;
}
#endif

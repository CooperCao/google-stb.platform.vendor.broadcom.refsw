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

#if NEXUS_HAS_SECURITY

#include "nexus_platform.h"
#include "nexus_memory.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "nexus_security.h"
#include "nexus_hmac_sha_cmd.h"
#include "nexus_base_mmap.h"

#define MAX_HEAP_BUFFER_SIZE ((1024 * 1024 * 50))

#define MIN(a,b) (((a)<(b))?(a):(b))

int hex2bin( char* pkey, char *pKeyHex, unsigned sizeHex )
{
    unsigned x = 0;
    unsigned i = 0;
    char byte, hexByte;

    if( sizeHex % 2 ) {
        printf("error .. invalid sizeHex[%d]", sizeHex );
        return -1;
    }


    printf("\t [%s] \t[", BSTD_FUNCTION  );

    for( x = i = 0; x < sizeHex; x+=2, i++ )
    {
        byte = 0;

        hexByte = toupper( pKeyHex[x] );
        if((hexByte >= 48) && (hexByte <= 57))  /* a digit */
        {
            byte = ( hexByte - 48 ) << 4;
        }
        else if((hexByte >= 65) && (hexByte <= 70)) /* a number */
        {
            byte += ( 10 + ( hexByte - 65 )) << 4;
        }
        else
        {
            printf("error [%s] x[%d] %X", BSTD_FUNCTION, x, hexByte );
            return -1;
        }

        hexByte = toupper( pKeyHex[x+1] );
        if((hexByte >= 48) && (hexByte <= 57))  /* a digit */
        {
            byte += ( hexByte - 48 );
        }
        else if((hexByte >= 65) && (hexByte <= 70)) /* a number */
        {
            byte += 10 + ( hexByte - 65 );
        }
        else
        {
            printf("error [%s] x[%d] %X b", BSTD_FUNCTION, x, hexByte );
            return -1;
        }

        pkey[i] = byte;
        printf("%X", byte );
    }

    printf("]\n");
    return 0;
}



int main( int argc, char **argv )
{

#if NEXUS_HAS_SECURITY
    NEXUS_PlatformSettings platformSettings;
    NEXUS_HMACSHA_OpSettings shaOpSettings;
    NEXUS_HMACSHA_DigestOutput hmacOut;
    uint8_t*  pBuffer = NULL;
    unsigned  bufferSize = 0;
    NEXUS_Error rc;
    unsigned bytesRead = 0;
    signed i;
    char *pFilename = NULL;
    FILE *hFile = NULL;
    bool swapBytes = false;
    bool osMemory = false;
    unsigned fileBytesToRead = 0;
    char *pArg = NULL;
    char *pKey = NULL;
    unsigned keyLen;
    NEXUS_HMACSHA_SHAType shaType = NEXUS_HMACSHA_SHAType_eSha160;
    NEXUS_HMACSHA_KeyInclusion_Op keyIncMode = NEXUS_HMACSHA_KeyInclusion_Op_eNo;
    NEXUS_HMACSHA_Op operation = NEXUS_HMACSHA_Op_eSHA;


    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);

    for( i = 1; i < argc; i++ )
    {
        printf("ARG %s ...\n", argv[i] );
        /* parse for filename.  */
        pArg = "-f=";
        if( strncmp( argv[i], pArg, strlen(pArg) ) == 0 )
        {
            unsigned filenameLen = strlen( argv[i]+strlen(pArg) );

            if( filenameLen )
            {
                pFilename = malloc( filenameLen + 1 );
                if( pFilename  )
                {
                    strcpy( pFilename, argv[i]+strlen(pArg) );
                }
            }
        }

        /* parse for byteswap.  */
        pArg = "-swap";
        if( strncmp( argv[i], pArg, strlen(pArg) ) == 0 ) { swapBytes = true; }

        /* parse for SHA type.  */
        pArg = "-sha1";
        if( strncmp( argv[i], pArg, strlen(pArg) ) == 0 ) { shaType = NEXUS_HMACSHA_SHAType_eSha160; }
        pArg = "-sha160";
        if( strncmp( argv[i], pArg, strlen(pArg) ) == 0 ) { shaType = NEXUS_HMACSHA_SHAType_eSha160; }
        pArg = "-sha224";
        if( strncmp( argv[i], pArg, strlen(pArg) ) == 0 ) { shaType = NEXUS_HMACSHA_SHAType_eSha224; }
        pArg = "-sha256";
        if( strncmp( argv[i], pArg, strlen(pArg) ) == 0 ) { shaType = NEXUS_HMACSHA_SHAType_eSha256; }

        pArg = "-hmac";
        if( strncmp( argv[i], pArg, strlen(pArg) ) == 0 ) { operation = NEXUS_HMACSHA_Op_eHMAC; }

        /* append Key to source data before sha operation */
        pArg = "-shaKey";
        if( strncmp( argv[i], pArg, strlen(pArg) ) == 0 ) { keyIncMode = NEXUS_HMACSHA_KeyInclusion_Op_eAppend; }

        /* use OS/Linux memory */
        pArg = "-osmem";
        if( strncmp( argv[i], pArg, strlen(pArg) ) == 0 ) { osMemory = true; }

        /* Key in HEX format */
        pArg = "-hexKey=";
        if( strncmp( argv[i], pArg, strlen(pArg) ) == 0 )
        {
            if( ( keyLen = strlen( argv[i]+strlen(pArg) ) ) > 0 )
            {
                if( (pKey = malloc( keyLen )) != NULL )
                {
                    hex2bin( pKey, argv[i]+strlen(pArg), keyLen );
                }
            }
        }
    }

    if( pFilename == NULL )
    {
        if(pKey) free(pKey);
        printf( "Options:        REQUIRED \n"
                "\t -f=<filenaem> \n"
                "\t -sha1/sha160/sha224/sha256 \n"
                "\t -shaKey   ...  append key to data to be sha'ed \n"
                "\t -hexKey=<key in hex> \n"
                "\t -swap     ... swap bytes in data buffer \n"
                "\t -hmac     ... sha default.  \n"
                "\t -osmem    ... use OS memory, instead of BMEM.  \n"
                "EXAMPLES: The following pairs are equilivant\n"
                " --- ./nexus security_sha -f=<filename> -swap\n"
                " --- sha1sum <filename>\n"
                "and\n"
                " --- ./nexus security_sha -f=<filename> -hmac -hexKey=010203040506070809101112131415161700 -swap\n"
                " --- openssl dgst -sha1 -mac HMAC -macopt hexkey:010203040506070809101112131415161700 <filename>\n"
                 );
        return -1;
    }

    hFile = fopen( pFilename, "rb" );
    if( !hFile ) {
        if(pKey) free(pKey);
        printf("Unable to open file %s\n", pFilename );
        return -1;
    }
    fseek( hFile, 0L, SEEK_END );
    fileBytesToRead = ftell( hFile );
    fseek( hFile, 0L, SEEK_SET );
    printf( "Opening [%s] size[%d]\n", pFilename, fileBytesToRead );
    free( pFilename );
    pFilename = NULL;


    NEXUS_Platform_GetDefaultSettings( &platformSettings );
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init( &platformSettings );

    bufferSize = MIN( fileBytesToRead, MAX_HEAP_BUFFER_SIZE );

    if( osMemory )
    {
        pBuffer = malloc( bufferSize );
    }
    else
    {
        if(!bufferSize)   rc= NEXUS_Memory_Allocate( 1, NULL, (void *)&pBuffer );
        else  rc = NEXUS_Memory_Allocate( bufferSize, NULL, (void *)&pBuffer );
    }

    if( pBuffer == NULL )
    {
        printf("%s ### FAILED  to allocate %d bytes for submit buffer", BSTD_FUNCTION, bufferSize ) ;
        return -1;
    }

    memset ( &hmacOut, 0x00, sizeof( hmacOut ) );

    NEXUS_HMACSHA_GetDefaultOpSettings( &shaOpSettings );
    shaOpSettings.opMode      = operation;
    shaOpSettings.dataSrc     = NEXUS_HMACSHA_DataSource_eDRAM; /* only allowed option. */
    shaOpSettings.shaType     = shaType;
    shaOpSettings.dataMode    = NEXUS_HMACSHA_DataMode_eAllIn;
    shaOpSettings.dataAddress = pBuffer;
    shaOpSettings.dataSize    = bytesRead;
    shaOpSettings.byteSwap    = swapBytes;
    shaOpSettings.keyIncMode  = keyIncMode;
    if( keyLen && pKey )
    {
        shaOpSettings.keyLength = MIN( (keyLen/2), NEXUS_HMACSHA_KEY_LEN );
        memcpy( shaOpSettings.keyData, pKey, shaOpSettings.keyLength );
    }

    while( fileBytesToRead > bufferSize )
    {
        bytesRead = fread( pBuffer, 1, bufferSize, hFile );
        if( osMemory == false ){ NEXUS_FlushCache( pBuffer, bytesRead );  }
        printf("read [%d] bytes bufferSize[%d] fileBytesToRead[%d]\n",  bytesRead, bufferSize, fileBytesToRead );

        shaOpSettings.dataMode = NEXUS_HMACSHA_DataMode_eMore;
        shaOpSettings.dataSize = bytesRead;
        if( NEXUS_HMACSHA_PerformOp( &shaOpSettings, &hmacOut ) )
        {
            printf("%s NEXUS_HMACSHA_PerformOp Failed", BSTD_FUNCTION);
            if(pKey) free(pKey);
            printf("ERROR \n");
            return -1;
        }

        fileBytesToRead -= bytesRead;
    }

    bytesRead = fread( pBuffer, 1, bufferSize, hFile);
    if( osMemory == false ){ NEXUS_FlushCache( pBuffer, bytesRead ); }

    printf("read [%d] bytes bufferSize[%d] fileBytesToRead[%d] last\n",  bytesRead, bufferSize, fileBytesToRead );

    if( shaOpSettings.dataMode == NEXUS_HMACSHA_DataMode_eMore ) {
        shaOpSettings.dataMode = NEXUS_HMACSHA_DataMode_eLast;
    }
    shaOpSettings.dataSize = bytesRead;

    if( NEXUS_HMACSHA_PerformOp( &shaOpSettings, &hmacOut) )
    {
        printf("ERROR ... %s NEXUS_HMACSHA_PerformOp Failed", BSTD_FUNCTION);
        return -1;
    }

    printf("[0x%08X] HASH ", shaOpSettings.dataSize  );
    for( i = 0; i < (signed)hmacOut.digestSize; i++ )
    {
        printf ("%02x", hmacOut.digestData[i] );
    }
    printf("\n");

    if( osMemory )
    {
        free( pBuffer );
    }
    else
    {
        NEXUS_Memory_Free( pBuffer );
    }

    if( pKey ) free( pKey );

    NEXUS_Platform_Uninit();

#else
    BSTD_UNUSED( argc );
    BSTD_UNUSED( argv );
#endif /* NEXUS_HAS_SECURITY */
    return 0;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform!\n");
    return -1;
}
#endif

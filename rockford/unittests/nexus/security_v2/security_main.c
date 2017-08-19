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

#if NEXUS_HAS_SECURITY && NEXUS_SECURITY_API_VERSION==2

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "nexus_platform.h"
#include "nexus_memory.h"
#include "nexus_dma.h"
#include "nexus_security_datatypes.h"
#include "nexus_security.h"
#include "security_main.h"
#include "security_util.h"

/* print details of what tests have been executed and passed/failed. */
static void printReport(void)
{
    unsigned i;
    groupTestItem *pTestItem;

    BKNI_Sleep(600);

    printf( "\n\n************** TEST REPORT **************\n" );
    for( i = 0; i < securityTestGroup_max; i++ )
    {
        testGroup *pGroupData = &gData.groups[i];

        printf( "GROUP %s \n", pGroupData->groupName );

        pTestItem = BLST_S_FIRST( &pGroupData->testList );
        while( pTestItem )
        {
            printf( "   %d:", pTestItem->args.number );

            if(  pTestItem->ran )
            {
                printf( " %s", pTestItem->passed?"SUCCESS":"FAILED " );
            }
            else
            {
                printf( "       ");
            }
            printf( " %s [%s]\n", pTestItem->args.name , pTestItem->args.description );

            pTestItem = BLST_S_NEXT( pTestItem, next );
        }
    }
    printf( "****************************\n\n\n" );

    return;
}

static void printUsage( char *pAppName )
{

    printf( "\n%s: Application to unit test security.\n", pAppName?pAppName:"" );
    printf( "OPTIONS:\n" );
    printf( "   -h   : print this \n" );
    printf( "   -m   : display menu to select test \n" );
    printf( "   -v   : display test debug \n" );
    printf( "   -g=x : run test from group number \"x\". All, if not test (-t=y) specified \n" );
    printf( "   -t=y : run test y from group number -g=x \n" );

    printf( "\n" );
    return;
}


int main( int argc, char **argv )
{
    NEXUS_PlatformSettings platformSettings;
    securityUtil_options options;
    groupTestItem *pTestItem;
    int result = 0;
    unsigned i;
    char ch;

    /* initialise module data */
    BKNI_Memset( &gData, 0, sizeof(gData) );

    options = securityUtil_ParseOptions( argc, argv );

    if( options.showHelp ) { printUsage( argv[0] ); return 0; }

    /* start NEXUS */
    NEXUS_Platform_GetDefaultSettings( &platformSettings );
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init( &platformSettings );

    initTestGroup_keyslot( );
    initTestGroup_keyladder( );
    initTestGroup_randomNumber( );
    initTestGroup_hash( );
    initTestGroup_hmac( );
    initTestGroup_rsa( );
    initTestGroup_regionVerification( );

    /* "menu" to display menu.*/
    if( options.showMenu )
    {
        int groupNumber = 0;
        int testNumber = 0;

        BKNI_Sleep(600);

        do {
            printf( "\n\nSelect a GROUP:\n" );
            for( i = 0; i < securityTestGroup_max;  i++) {
                printf( "%d: %s\n", i, gData.groups[i].groupName );
            }
            printf( "q to quit\n" );

            while( ( ch=getchar( ) ) == '\n'){ }

            if( ch == 'q' ) { goto clean_up; }
            groupNumber = (int)ch - '0';

            if( groupNumber >= securityTestGroup_max ) {
               /*  printf( "\n\nInvalid group [%d]. Try AGAIN\n",  (int)groupNumber );*/
                 continue;
            }

            /*printf( "\nGROUP [%s] was selected:\n", gData.groups[groupNumber].groupName );*/

            printf( "    Select TEST from %s:\n",  gData.groups[groupNumber].groupName );

            pTestItem = BLST_S_FIRST( &gData.groups[groupNumber].testList );
            while( pTestItem ) {
                printf( "    %d: %s [%s]\n", pTestItem->args.number
                                       , pTestItem->args.name
                                       , pTestItem->args.description );

                pTestItem = BLST_S_NEXT( pTestItem, next );
            }
            printf( "    q to quit\n" );

            while( ( ch=getchar( ) ) == '\n'){ }
            if( ch == 'q' ) { goto clean_up; }

            testNumber = (int)ch - '0';

            printf( "%d was selected\n", testNumber );


            pTestItem = BLST_S_FIRST( &gData.groups[groupNumber].testList );
            while( pTestItem )
            {
                if( testNumber == (int)pTestItem->args.number )
                {
                    result = pTestItem->registeredTest( &pTestItem->args );
                    pTestItem->ran = true;
                    pTestItem->passed = result==0?true:false;

                    verboseLog( "%s [%s]\n",result?"FAILED":"SUCCESS" , pTestItem->args.name );
                }
                pTestItem = BLST_S_NEXT( pTestItem, next );
            }

        }while( ch != 'q' );

        goto clean_up;
    }

    /* no arguments, run all tests */
    if( options.groupNum < 0 &&  options.testNum < 0 )
    {
        verboseLog( "RUNNING ALL TEST\n" );
        for( i = 0; i < securityTestGroup_max; i++ ) {

            pTestItem = BLST_S_FIRST( &gData.groups[i].testList );
            while( pTestItem ) {

                result = pTestItem->registeredTest( &pTestItem->args );
                pTestItem->ran = true;
                pTestItem->passed = result==0?true:false;
                verboseLog( "%s [%s]\n",result?"FAILED ":"SUCCESS" , pTestItem->args.name );

                pTestItem = BLST_S_NEXT( pTestItem, next );
            }
        }
        goto clean_up;
    }

    /* check group validity.. */
    if( options.groupNum  >= securityTestGroup_max ) {
        printf( "INVALID GROUP SPECIFIED [%s]\n", argv[1] );
        goto clean_up;
    }

    if( options.groupNum >= 0 )
    {
        int groupNumber = options.groupNum;

        if( options.testNum < 0 ) /* run all test in the group. */
        {
            printf( "RUNNING ALL [%s] TESTS\n", gData.groups[groupNumber].groupName );
            pTestItem = BLST_S_FIRST( &gData.groups[groupNumber].testList );
            while( pTestItem ) {

                result = pTestItem->registeredTest( &pTestItem->args );
                pTestItem->ran = true;
                pTestItem->passed = result==0?true:false;
                printf( "%s [%s]\n",result?"FAILED":"SUCCESS" , pTestItem->args.name );

                pTestItem = BLST_S_NEXT( pTestItem, next );
            }

            goto clean_up;
        }

        /* find gData.options.testNum */
        pTestItem = BLST_S_FIRST( &gData.groups[groupNumber].testList );
        while( pTestItem ) {

            if( options.testNum == (int)pTestItem->args.number )
            {
                result = pTestItem->registeredTest( &pTestItem->args );
                pTestItem->ran = true;
                pTestItem->passed = result==0?true:false;
                verboseLog( "%s [%s]\n",result?"FAILED":"SUCCESS" , pTestItem->args.name );
            }
            pTestItem = BLST_S_NEXT( pTestItem, next );
        }

        goto clean_up;
    }

clean_up:

    printReport();

    /* cleanup test list */
    for( i = 0; i < securityTestGroup_max; i++ ) {
        groupTestItem *pTestItem = NULL;

        while(( pTestItem = BLST_S_FIRST( &gData.groups[i].testList ) )) {
            /*verboseLog( "deleting [%s]\n", pTestItem->args.name );*/
            BLST_S_REMOVE( &gData.groups[i].testList, pTestItem, groupTestItem, next );
            BKNI_Free( pTestItem );
            pTestItem = NULL;
        }
    }

    /* stop NEXUS. */
    NEXUS_Platform_Uninit( );

    return 0;
}
#if 0
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
#endif /* #if 0 */
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supportedi.\n");
    return -1;
}
#endif

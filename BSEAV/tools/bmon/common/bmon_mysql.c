/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include <stdlib.h>
#include <string.h>
#include "bmon_mysql.h"

static unsigned long long int mysql_init_count  = 0;
static unsigned long long int mysql_close_count = 0;

int bmon_mysql_close(
    MYSQL *hMysql
    )
{
    if (hMysql)
    {
        mysql_close( hMysql );
        mysql_close_count++;
        if (mysql_init_count != mysql_close_count)
        {
            fprintf( stderr, "%s -  hMysql (%p) ... init %llu ... close %llu\n", __FUNCTION__, (void *) hMysql, mysql_init_count, mysql_close_count );
        }
    }

    return( 0 );
}

MYSQL *bmon_mysql_init(
    void
    )
{
    char            g_szDatabase[] = "dmsmaster";
    register MYSQL *hMysql         = 0;
    int             rc             =   0;
    MYSQL          *hrc            = 0;

    // Initialize MySQL...
    hMysql = mysql_init( NULL );
    mysql_init_count++;

    if (mysql_init_count != ( mysql_close_count+1 ))
    {
        fprintf( stderr, "%s -  hMysql (%p) ... init %llu ... close %llu\n", __FUNCTION__, (void *) hMysql, mysql_init_count, mysql_close_count );
    }

    // Failed...
    if (!hMysql)
    {
        // Alert user...
        fprintf( stderr, "Error: Unable to initialize MySQL API...\n" );

        // Cleanup, abort, terminate...
        return( 0 );
    }

    // Connect to server and check for error...
    /*fprintf( stderr, "Trying mysql_real_connect(%p, %s, %s, %s)\n", hMysql, MYSQL_SERVER, MYSQL_USER, MYSQL_PW); fflush(stdout);*/
    hrc = mysql_real_connect( hMysql, MYSQL_SERVER /* was NULL */, MYSQL_USER, MYSQL_PW, g_szDatabase, 0, NULL, 0 );
    /*fprintf( stderr, "after mysql_real_connect, hrc (%p)\n", hrc); fflush(stdout);*/
    if (hrc == 0)
    {
        // Alert user...
        fprintf( stderr, "Error: Unable to connect to server ... (%s)\n", mysql_error( hMysql ));

        // Cleanup, abort, terminate...
        bmon_mysql_close( hMysql );
        return( 0 );
    }

    /*fprintf( stderr, "Connected - host (%s) user (%s) passwd (%s); server_ver (%s)  host_info (%s) \n",
        hMysql->host, hMysql->user, hMysql->passwd, hMysql->server_version, hMysql->host_info );*/

    // Select database in server and check for error...
    rc = mysql_select_db( hMysql, g_szDatabase );
    /*fprintf( stderr, "mysql_select(%s); rc %i\n", g_szDatabase, rc);*/
    if (rc)
    {
        // Alert user...
        fprintf( stderr, "Error: Unable to select database...(%s)\n", g_szDatabase );

        // Cleanup, abort, terminate...
        bmon_mysql_close( hMysql );
        return( 0 );
    }

    /*fprintf( stderr, "%s - mysql_init(%p)\n", __FUNCTION__, hMysql );*/
    return( hMysql );
} /* bmon_mysql_init */

int bmon_mysql_cat_int(
    char                            *outputStr,
    unsigned int                     outputStr_Len,
    unsigned long int                FieldValue,
    const char                      *FieldName,
    BMON_MYSQL_CONJUNCTION_TYPE_T conjunction
    )
{
    char temp[255];

    // do not output a comma before the first entry in the update list
    if (conjunction == BMON_MYSQL_CONJUNCTION_TYPE_COMMA) {strncat( outputStr, ",", outputStr_Len - strlen( outputStr ) - 1 ); }
    else if (conjunction == BMON_MYSQL_CONJUNCTION_TYPE_AND)
    {
        strncat( outputStr, " AND ", outputStr_Len - strlen( outputStr ) - 1 );
    }

    sprintf( temp, "%s='%lu'", FieldName, FieldValue );
    strncat( outputStr, temp, outputStr_Len - strlen( outputStr ) - 1 );

    return( 0 );
}

int bmon_mysql_cat_str(
    char                            *outputStr,
    unsigned int                     outputStr_Len,
    const char                      *FieldValue,
    const char                      *FieldName,
    BMON_MYSQL_CONJUNCTION_TYPE_T conjunction
    )
{
    // do not output a comma before the first entry in the update list
    if (conjunction == BMON_MYSQL_CONJUNCTION_TYPE_COMMA) {strncat( outputStr, ",", outputStr_Len - strlen( outputStr ) - 1 ); }
    else if (conjunction == BMON_MYSQL_CONJUNCTION_TYPE_AND)
    {
        strncat( outputStr, " AND ", outputStr_Len - strlen( outputStr ) - 1 );
    }

    strncat( outputStr, FieldName, outputStr_Len - strlen( outputStr ) - 1 );
    strncat( outputStr, "=", outputStr_Len - strlen( outputStr ) - 1 );
    strncat( outputStr, "'", outputStr_Len - strlen( outputStr ) - 1 );
    strncat( outputStr, FieldValue, outputStr_Len - strlen( outputStr ) - 1 );
    strncat( outputStr, "'", outputStr_Len - strlen( outputStr ) - 1 );

    return( 0 );
}                                                          /* bmon_mysql_cat_str */

int bmon_mysql_cat_hex(
    char                            *outputStr,
    unsigned int                     outputStr_Len,
    const char                      *FieldValue,
    const char                      *FieldName,
    BMON_MYSQL_CONJUNCTION_TYPE_T conjunction
    )
{
    // do not output a comma before the first entry in the update list
    if (conjunction == BMON_MYSQL_CONJUNCTION_TYPE_COMMA) {strncat( outputStr, ",", outputStr_Len - strlen( outputStr ) - 1 ); }
    else if (conjunction == BMON_MYSQL_CONJUNCTION_TYPE_AND)
    {
        strncat( outputStr, " AND ", outputStr_Len - strlen( outputStr ) - 1 );
    }

    strncat( outputStr, FieldName, outputStr_Len - strlen( outputStr ) - 1 );
    strncat( outputStr, "=0x", outputStr_Len - strlen( outputStr ) - 1 );
    strncat( outputStr, FieldValue, outputStr_Len - strlen( outputStr ) - 1 );

    return( 0 );
}                                                          /* bmon_mysql_cat_hex */

unsigned long int bmon_mysql_find_tbl_struct_id(
    MYSQL       *hMysql,
    char        *outputStr,
    unsigned int outputStr_Len,
    const char  *tv_sec,
    const char  *tv_usec,
    const char  *board_id,
    const char  *plugin_name
    )
{
    unsigned long int rc          = -1;
    unsigned int      uRecords    = 0;
    unsigned long int tblStructID = 0;
    MYSQL_RES        *myResult    = NULL;
    MYSQL_ROW         myRow       = NULL;

    do
    {
        if (hMysql == NULL) {break; }

        snprintf( outputStr, outputStr_Len - strlen( outputStr ) - 1, "SELECT tblStructID from tblStructs WHERE " );
        bmon_mysql_cat_str( outputStr, outputStr_Len, tv_sec, "tv_sec", BMON_MYSQL_CONJUNCTION_TYPE_NONE );
        bmon_mysql_cat_str( outputStr, outputStr_Len, tv_usec, "tv_usec", BMON_MYSQL_CONJUNCTION_TYPE_COMMA );
        bmon_mysql_cat_str( outputStr, outputStr_Len, board_id, "board_id", BMON_MYSQL_CONJUNCTION_TYPE_COMMA );
        bmon_mysql_cat_str( outputStr, outputStr_Len, plugin_name, "plugin_name", BMON_MYSQL_CONJUNCTION_TYPE_COMMA );

        if (mysql_query( hMysql, outputStr ) != 0)
        {
            fprintf( stderr, "%s - unable to execute query (%s)\n", __FUNCTION__, outputStr );
            break;
        }

        // Retrieve query result from server
        myResult = mysql_store_result( hMysql );

        if (myResult == NULL)
        {
            fprintf( stderr, "%s - unable to retrieve result\n", __FUNCTION__ );
            break;
        }

        uRecords = mysql_num_rows( myResult );

        if (uRecords > 0)
        {
            myRow = mysql_fetch_row( myResult );

            if (myRow[0] != NULL)
            {
                sscanf( myRow[0], "%lu", &tblStructID );
                rc = tblStructID;
            }
        }

        mysql_free_result( myResult );
    } while (0);
    return( rc );
}                                                          /* bmon_mysql_find_tbl_struct_id */

unsigned long int bmon_mysql_insert_record(
    MYSQL            *hMysql,
    unsigned long int tv_sec,
    unsigned long int tv_usec,
    const char       *board_id,
    const char       *plugin_name,
    const char       *struct_data
    )
{
    unsigned long int insert_index  = 0;
    unsigned int      outputStr_Len = 1024*1024;
    char             *outputStr     = NULL;

    outputStr = malloc( outputStr_Len );
    if (outputStr == NULL)
    {
        fprintf( stderr, "%s - could not malloc(%u) bytes\n", __FUNCTION__, outputStr_Len );
        return( 0 );
    }

    memset( outputStr, 0, outputStr_Len );

    do
    {
        if (hMysql == NULL) {break; }

        snprintf( outputStr, outputStr_Len - 1, "INSERT INTO tblStructs SET " );
        bmon_mysql_cat_int( outputStr, outputStr_Len, tv_sec, "tv_sec", BMON_MYSQL_CONJUNCTION_TYPE_NONE );
        bmon_mysql_cat_int( outputStr, outputStr_Len, tv_usec, "tv_usec", BMON_MYSQL_CONJUNCTION_TYPE_COMMA );
        bmon_mysql_cat_str( outputStr, outputStr_Len, board_id, "board_id", BMON_MYSQL_CONJUNCTION_TYPE_COMMA );
        bmon_mysql_cat_str( outputStr, outputStr_Len, plugin_name, "plugin_name", BMON_MYSQL_CONJUNCTION_TYPE_COMMA );
        bmon_mysql_cat_hex( outputStr, outputStr_Len, struct_data, "struct_data", BMON_MYSQL_CONJUNCTION_TYPE_COMMA );

        /*fprintf( stderr, "%s - sql (%s)\n", __FUNCTION__, outputStr );*/
        if (mysql_query( hMysql, outputStr ) != 0)
        {
            fprintf( stderr, "%s - unable to execute query (%s)\n", __FUNCTION__, outputStr );
            break;
        }

        insert_index = mysql_insert_id( hMysql );
    } while (0);

    free( outputStr );

    /*fprintf( stderr, "%s - returning insert_index (%lu)\n", __FUNCTION__, insert_index );*/

    return( insert_index );
} /* bmon_mysql_insert_record */

int bmon_mysql_select_record(
    MYSQL        *hMysql,
    const char   *plugin_name,
    const char   *board_id,
    void         *struct_data,
    unsigned int  struct_size,
    unsigned int *ptv_sec,
    unsigned int *ptv_usec
    )
{
    unsigned int uRecords = 0;
    MYSQL_RES   *myResult = NULL;
    MYSQL_ROW    myRow    = NULL;
    char         sqlStr[256];
    unsigned int sqlStr_Len = sizeof( sqlStr );

    do
    {
        if (hMysql == NULL) {break; }

        memset( sqlStr, 0, sizeof( sqlStr ));

        // select tblStructID from tblStructs where plugin_name="wifi" order by tblStructID DESC LIMIT 1;
        snprintf( sqlStr, sqlStr_Len - strlen( sqlStr ) - 1, "SELECT hex(struct_data),board_id,tblStructID,tv_sec,tv_usec from tblStructs WHERE " );
        bmon_mysql_cat_str( sqlStr, sqlStr_Len, plugin_name, "plugin_name", BMON_MYSQL_CONJUNCTION_TYPE_NONE );
        if (board_id) {bmon_mysql_cat_str( sqlStr, sqlStr_Len, board_id, "board_id", BMON_MYSQL_CONJUNCTION_TYPE_AND ); }
        strncat( sqlStr, " order by tblStructID DESC limit 1", sqlStr_Len );

        /*fprintf( stderr, "%s - sql (%s) \n", __FUNCTION__, sqlStr ); fflush(stdout); fflush(stderr);*/
        if (mysql_query( hMysql, sqlStr ) != 0)
        {
            fprintf( stderr, "%s - unable to execute query (%s)\n", __FUNCTION__, sqlStr );
            break;
        }

        // Retrieve query result from server
        myResult = mysql_store_result( hMysql );

        if (myResult == NULL)
        {
            fprintf( stderr, "%s - unable to retrieve result\n", __FUNCTION__ );
            break;
        }

        uRecords = mysql_num_rows( myResult );
        /*fprintf( stderr, "%s - uRecords %u \n", __FUNCTION__, uRecords );*/

        if (uRecords > 0)
        {
            myRow = mysql_fetch_row( myResult );

            if (( myRow[0] != NULL ) && strlen( myRow[0] ))
            {
                unsigned int   idx     = 0;
                unsigned int   data    = 0;
                unsigned char *hexdata = (unsigned char *) struct_data; /* work around compiler warning about void pointer */
                /*fprintf( stderr, "%s - for %u bytes and len %u ... struct_data (%s) \n", __FUNCTION__, struct_size, (unsigned int) strlen(myRow[0]), myRow[0] );*/

                /* convert hex string to binary */
                if (( struct_size*2 ) != (unsigned int) strlen( myRow[0] ))
                {
                    fprintf( stderr, "%s - ERROR ... compiled struct %u bytes does not match MySQL size (%d) \n",
                        __FUNCTION__, struct_size, (unsigned int) strlen( myRow[0] ));
                }
                for (idx = 0; idx<strlen( myRow[0] )/2 && ( idx*2 )<struct_size; idx++) {
                    sscanf( &( myRow[0][idx*2] ), "%02x", &data );
                    hexdata[idx] = data;
                }
            }

            if (myRow[1] != NULL)
            {
                /*fprintf( stderr, "%s - board_id (\"%s\") \n", __FUNCTION__, myRow[1] );*/
            }

            if (myRow[2] != NULL)
            {
                /*fprintf( stderr, "%s - tblStructID (%s) \n", __FUNCTION__, myRow[2] );*/
            }

            if (myRow[3] != NULL)
            {
                /*fprintf( stderr, "%s - tblStructID (%s) \n", __FUNCTION__, myRow[3] );*/
                *ptv_sec = atoi( myRow[3] );
            }

            if (myRow[4] != NULL)
            {
                /*fprintf( stderr, "%s - tblStructID (%s) \n", __FUNCTION__, myRow[4] );*/
                *ptv_usec = atoi( myRow[4] );
            }
        }

        mysql_free_result( myResult );
    } while (0);

    return( 0 );
} /* bmon_mysql_select_record */

#if defined ( NATIVE_SUPPORT )
/**
 *  Function: This function will collect all requred wifi data from a MySQL database and
 *            store the data into a known structure.
 **/
int bmon_mysql_get_data(
    const char       *plugin_name,
    const char       *url_in,
    void             *mysql_data,
    unsigned int      mysql_data_size,
    unsigned int     *ptv_sec,
    unsigned int     *ptv_usec
    )
{
    MYSQL *hMysql = NULL;

    /*fprintf( stderr, "%s - type %s ... url (%s) ... size %u \n", __FUNCTION__, plugin_name, url_in, mysql_data_size );*/

    do {
        hMysql = bmon_mysql_init();
        if (hMysql == NULL)
        {
            fprintf( stderr, "bmon_mysql_init() Failed ... hMysql %p\n", hMysql );
            break;
        }

        bmon_mysql_select_record( hMysql, plugin_name, NULL, mysql_data, mysql_data_size, ptv_sec, ptv_usec );

        bmon_mysql_close( hMysql );
    } while (0);

    return( 0 );
} /* bmon_mysql_get_data */

#endif /* #if defined(NATIVE_SUPPORT) */

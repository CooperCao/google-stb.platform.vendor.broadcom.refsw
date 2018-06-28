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
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sqlite3.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>

#include "bmon_sqlite3.h"
#include "bmon_utils.h"
#ifdef RSFTP_SUPPORT
#include "rsftpc.h"
#endif /* RSFTP_SUPPORT */
#ifdef MYSQL_SUPPORT
#include "bmon_mysql.h"
#endif /* MYSQL_SUPPORT */

static TABLE_SCHEMA g_TableSystem;
static int          g_TableSystem_Count = 0;
static sqlite3     *g_db                = NULL;

int bmon_sqlite3_open ( char *db_name )
{
    int         rc = 0;
    struct stat statbuf;

    /* if database has not been opened yet, try now */
    if ( g_db == NULL ) {
        memset( &statbuf, 0, sizeof(statbuf) );

        /* if the database file does not exist, create it */
        if (lstat( db_name, &statbuf ) == -1) {
            char  system_line[256];
            fprintf( stderr, "%s: Could not stat (%s)\n", __FUNCTION__, db_name );

            sprintf( system_line, "./sqlite3 %s < bmond.sql", db_name );
            /*fprintf( stderr, "Issuing system(%s)\n", system_line );*/
            system( system_line );
        }

        /* Open database */
        rc = sqlite3_open( db_name, &g_db );

        if (rc) {
            fprintf( stderr, "Can not open database (%s) ... %s\n", db_name, sqlite3_errmsg( g_db ));
            return( 0 );
        } else {
            /*fprintf( stderr, "Opened database (%s) successfully\n", db_name );*/
        }
    }

    return 0;
}

int bmon_sqlite3_callback_insert(
    void  *NotUsed,
    int    argc,
    char **argv,
    char **azColName
    )
{
    int i;

    /*fprintf( stderr, "%s ... argc %d\n", __FUNCTION__, argc );*/
    for (i = 0; i<argc; i++) {
        fprintf( stdout, "%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL" );
    }
    return( 0 );
}

int bmon_sqlite3_table_schema_store( const char *token, char *output_buffer )
{
    char *pos                = (char*) token;
    bool  comment_in_process = false;

    if ( token == NULL || strlen(token) == 0 ) return 0;

    /* ignore any leading new lines or spaces or inline comments */
    while ( *pos ) {
        if ( comment_in_process ) {
            if ( *pos == '*' && pos[1] == '/' ) {
                comment_in_process = false;
                pos++;
                pos++;
                /*printf( "%s: comment ended (%s)\n", __FUNCTION__, pos );*/
            } else {
                pos++;
            }
        } else if ( *pos == '\n' || *pos == '\r' || *pos == ' ' ) {
            pos++;
            /*printf( "%s: found newline or space (%s)\n", __FUNCTION__, pos );*/
        } else if ( *pos == '/' && pos[1] == '*' ) {
            comment_in_process = true;
            /*printf( "%s: comment started (%s)\n", __FUNCTION__, pos );*/
            pos++;
            pos++;
        } else {
            break; /* this should be the beginning of the schema definition */
        }
    }

    if ( pos && g_TableSystem_Count < SQLITE_NUM_COLUMNS_MAX ) {
        char  one_line[1024];
        strncpy( g_TableSystem.column_definition[g_TableSystem_Count], pos, sizeof( g_TableSystem.column_definition[g_TableSystem_Count] ) );
        /*printf( "%s - g_TableSystem[%d] is (%s)\n", __FUNCTION__, g_TableSystem_Count, g_TableSystem.column_definition[g_TableSystem_Count] );*/
        if ( g_TableSystem_Count == 0 ) {
            strcat( output_buffer, "{ \"TimeNow\":\"" );
            strcat( output_buffer, bmon_date_str() );
            strcat( output_buffer, "\"" );
        }
        sprintf( one_line, " ,\"Column %d\":\"%s\"", g_TableSystem_Count, g_TableSystem.column_definition[g_TableSystem_Count] );
        strcat( output_buffer, one_line );
        g_TableSystem_Count++;
    }
    return 0;
}

int bmon_sqlite3_callback_table_schema(
    void  *poutput_buffer,
    int    argc,
    char **argv,
    char **azColName
    )
{
    char *output_buffer = (char*)poutput_buffer;

    /*printf( "%s ... argc %d\n", __FUNCTION__, argc );*/
    /* this callback should only return 1 big long string */
    if ( argc == 1 && argv && argv[0] ) {
        char *left_paren = NULL;
        char *token      = NULL;
        char *token_ptr  = NULL;
        int   line_len   = 0;

        /*printf( "%s = %s\n", azColName[0], argv[0] );*/

        /* find the beginning of the table columns */
        left_paren = strchr( argv[0], '(' );

        if ( left_paren ) {
            left_paren++;
            line_len = strlen(left_paren);
            /* line should end with " )" ... remove the space and right paren */
            if ( line_len > 2 && left_paren[line_len-2] == ' ' && left_paren[line_len-1] == ')' ) {
                left_paren[line_len-2] = '\0';
            }
            token = strtok_r( left_paren, ",", &token_ptr );
            /*printf( "%s - token (%p)\n", __FUNCTION__, token );*/
            while ( token != NULL) {
                /*printf( "%s - got token (%s)\n", __FUNCTION__, token );*/

                bmon_sqlite3_table_schema_store( token, output_buffer );

                token = strtok_r(NULL, ",", &token_ptr );
            }
        }
    }
    return( 0 );
}

int bmon_sqlite3_callback_select_csv(
    void  *poutput_buffer,
    int    argc,
    char **argv,
    char **azColName
    )
{
    int i = 0;
    char  one_line[1024];

    /*fprintf( stderr, "%s ... argc %d\n", __FUNCTION__, argc );*/
    strcat( poutput_buffer, "{ " );
    for (i = 0; i<argc; i++) {
        /*fprintf( stderr, "%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL" );*/
        if ( i ) strcat( poutput_buffer, "," );
        snprintf( one_line, sizeof(one_line), "\"%s\":\"%s\"", azColName[i], argv[i] ? argv[i] : "NULL" );
        strcat( poutput_buffer, one_line );
    }
    strcat( poutput_buffer, " }" );

    return( 0 );
}

int bmon_sqlite3_create(
    const char *create_str
    )
{
    int   rc      = 0;
    char *zErrMsg = 0;

    if ( create_str == NULL ) {return( 0 ); }

    /* Execute SQL statement */
    /*fprintf( stderr, "calling sqlite3_exec()\n" );*/
    rc = sqlite3_exec( g_db, create_str, bmon_sqlite3_callback_insert, 0, &zErrMsg );

    /*fprintf( stderr, "after sqlite3_exec() ... rc %d\n", rc );*/
    if (rc != SQLITE_OK)
    {
        fprintf( stderr, "SQL error: %s\n", zErrMsg );
        sqlite3_free( zErrMsg );
    }
    else
    {
        /*fprintf( stderr, "Table created successfully\n" );*/
    }

    return( 0 );
} /* bmon_sqlite3_create */

#define sql_str_size ( SQL_BUFFER_SIZE + 64 )
int bmon_sqlite3_insert(
    const char *sql_str
    )
{
    int   rc      = 0;
    char *zErrMsg = 0;

    if (( g_db == NULL ) || ( sql_str== NULL )) {return( 0 ); }

    /* Execute SQL statement */
    rc = sqlite3_exec( g_db, sql_str, bmon_sqlite3_callback_insert, 0, &zErrMsg );

    if (rc != SQLITE_OK)
    {
        fprintf( stderr, "SQL error: %s\n", zErrMsg );
        sqlite3_free( zErrMsg );
    }
    else
    {
        /*fprintf( stderr, "Records created successfully ... len %d\n", (int) strlen(sql_str) );*/
    }

    return( 0 );
} /* bmon_sqlite3_insert */

int bmon_sqlite3_select_table_schema(
    const char *tableName,
    char       *outputBuffer
    )
{
    char  *sql_str = NULL;
    char  *zErrMsg = 0;
    int    rc      = 0;

    if ( sql_str == NULL ) sql_str = malloc( sql_str_size );

    memset( &g_TableSystem, 0, sizeof( g_TableSystem ) );
    g_TableSystem_Count = 0;

    sprintf( sql_str, "SELECT sql FROM sqlite_master WHERE tbl_name = '%s' AND type = 'table';", tableName );

    /* Execute SQL statement */
    rc = sqlite3_exec( g_db, sql_str, bmon_sqlite3_callback_table_schema, outputBuffer, &zErrMsg );

    if (rc != SQLITE_OK)
    {
        fprintf( stderr, "SQL error: %s\n", zErrMsg );
        sqlite3_free( zErrMsg );
    }

    if ( sql_str == NULL ) free( sql_str );

    return 0;

} /* bmon_sqlite3_select_table_schema */

int bmon_sqlite3_callback_select_last_row(
    void  *pCtx,
    int    argc,
    char **argv,
    char **azColName
    )
{
    int i = 0;
    char *outputBuffer = (char*) pCtx;
    char  one_line[1024];

    /*printf( "%s ... argc %d\n", __FUNCTION__, argc );*/
    if ( pCtx == NULL || argc == 0 ) return ( 0 );

    strcat( outputBuffer, "{ \"TimeNow\":\"" );
    strcat( outputBuffer, bmon_date_str() );
    strcat( outputBuffer, "\" " );
    for (i = 0; i<argc; i++) {
        strcat( outputBuffer, "," );
        memset( one_line, 0, sizeof(one_line) );
        /* Note: argv[i] could be really long ... 192*4 bytes */
        sprintf( one_line, "\"%s\":\"%s\" ", azColName[i], argv[i] ? argv[i] : "NULL" );
        strcat( outputBuffer, one_line );
    }
    strcat( outputBuffer, "} " );
    return( 0 );
}   /* bmon_sqlite3_callback_select_last_row */

/*
 * SELECT * FROM tblStreams ORDER BY StreamID DESC LIMIT 1;
 */
int bmon_sqlite3_select_last_row(
    const char *tableName,
    const char *indexKeyName,
    char       *outputBuffer
    )
{
    char  *sql_str = NULL;
    char  *zErrMsg = 0;
    int    rc      = 0;

    if ( sql_str == NULL ) sql_str = malloc( sql_str_size );

    sprintf( sql_str, "SELECT * FROM %s ORDER BY %s DESC LIMIT 1;", tableName,indexKeyName );

    /* Execute SQL statement */
    rc = sqlite3_exec( g_db, sql_str, bmon_sqlite3_callback_select_last_row, outputBuffer, &zErrMsg );

    if (rc != SQLITE_OK)
    {
        fprintf( stderr, "SQL error: %s\n", zErrMsg );
        sqlite3_free( zErrMsg );
    }

    if ( sql_str == NULL ) free( sql_str );

    return 0;

} /* bmon_sqlite3_select_last_row */


int bmon_sqlite3_callback_select_tables(
    void  *pCtx,
    int    argc,
    char **argv,
    char **azColName
    )
{
    int i = 0;
    char *outputBuffer = (char*) pCtx;
    char  one_line[1024];

    /*fprintf( stderr, "%s ... argc %d\n", __FUNCTION__, argc );*/
    if ( pCtx == NULL || argc == 0 ) return ( 0 );

    strcat( outputBuffer, "{ \"TimeNow\":\"" );
    strcat( outputBuffer, bmon_date_str() );
    strcat( outputBuffer, "\" " );
    for (i = 0; i<argc; i++) {
        strcat( outputBuffer, "," );
        memset( one_line, 0, sizeof(one_line) );
        /* Note: argv[i] could be really long ... 192*4 bytes */
        sprintf( one_line, "\"%s\":\"%s\" ", azColName[i], argv[i] ? argv[i] : "NULL" );
        strcat( outputBuffer, one_line );
    }
    strcat( outputBuffer, "} " );
    return( 0 );
}   /* bmon_sqlite3_callback_select_tables */

int bmon_sqlite3_select_tables(
    char       *outputBuffer
    )
{
    char  *sql_str = NULL;
    char  *zErrMsg = 0;
    int    rc      = 0;

    if ( sql_str == NULL ) {
        sql_str = malloc( sql_str_size );
    }

    sprintf( sql_str, "SELECT name FROM sqlite_master WHERE type='table';" );

    /* Execute SQL statement */
    rc = sqlite3_exec( g_db, sql_str, bmon_sqlite3_callback_select_tables, outputBuffer, &zErrMsg );

    if (rc != SQLITE_OK)
    {
        fprintf( stderr, "SQL error: %s\n", zErrMsg );
        sqlite3_free( zErrMsg );
    }

    if ( sql_str == NULL ) free( sql_str );

    return 0;

} /* bmon_sqlite3_select_tables */

int bmon_sqlite3_close(
    void
    )
{
    if ( g_db == NULL) {return( 0 ); }

    sqlite3_close( g_db );
    g_db = NULL;
    return( 0 );
}

int bmon_get_last_row(
    const char *pTmpUrlPath,
    const char *tableName,
    const char *indexKeyName,
    char       *outputBuffer
    )
{
    bmon_sqlite3_select_last_row( tableName, indexKeyName, outputBuffer );

    return ( 0 );
}

int bmon_get_tables(
    const char *pTmpUrlPath,
    char       *outputBuffer
    )
{
    bmon_sqlite3_select_tables( outputBuffer );

    return ( 0 );
}

int bmon_get_table_schema(
    const char *tableName,
    char       *outputBuffer
    )
{
    if ( tableName && strlen(tableName) ) {
        bmon_sqlite3_select_table_schema( tableName, outputBuffer );
    }

    return ( 0 );
}

int bmon_select_csv(
    char       *outputBuffer,
    int         outputBufferSize,
    const char *CsvString,
    const char *tableName
    )
{
    char  *sql_str = NULL;
    char  *zErrMsg = 0;
    int    rc      = 0;

    if ( g_db == NULL || outputBuffer == NULL || outputBufferSize == 0 || CsvString == NULL || tableName == NULL) return 0;

    if ( sql_str == NULL ) sql_str = malloc( sql_str_size );

    sprintf( sql_str, "SELECT %s FROM %s;", CsvString, tableName );
    /*fprintf( stderr, "%s - sql (%s)\n", __FUNCTION__, sql_str );*/

    /* Execute SQL statement */
    rc = sqlite3_exec( g_db, sql_str, bmon_sqlite3_callback_select_csv, outputBuffer, &zErrMsg );

    if (rc != SQLITE_OK)
    {
        fprintf( stderr, "SQL error: %s\n", zErrMsg );
        sqlite3_free( zErrMsg );
    }

    if ( sql_str == NULL ) free( sql_str );

    return 0;
}

/**
 *  Function: This function inserts the specified binary blob into the tblStructs database.
 **/
int bmon_sqlite3_write_to_db(
    const unsigned char *pBlob,                            /* Pointer to blob of data */
    int                  lBlobLen,                         /* Length of data pointed to by pBlob */
    const char          *plugin_name,
    const char          *board_id
    )
{
    const char   *zSql = "INSERT INTO tblStructs(tblStructID, sqltime, tv_sec, tv_usec, board_id, plugin_name, struct_data ) VALUES(?, ?, ?, ?, ?, ?, ?)";
    sqlite3_stmt *pStmt;
    int           rc;
    time_t        mytime;
    struct timeval tv = {0,0};

    do {
        mytime = time(NULL);
        gettimeofday( &tv, NULL );
        /* Compile the INSERT statement into a virtual machine. */
        rc = sqlite3_prepare( g_db, zSql, -1, &pStmt, 0 );
        if (rc!=SQLITE_OK)
        {
            return( rc );
        }

        /* Bind the key and value data for the new table entry to SQL variables
        ** (the ? characters in the sql statement) in the compiled INSERT
        ** statement.
        **
        ** NOTE: variables are numbered from left to right from 1 upwards.
        ** Passing 0 as the second parameter of an sqlite3_bind_XXX() function
        ** is an error.
        */
        //sqlite3_bind_int  ( pStmt, 1, -1 );
        sqlite3_bind_int64( pStmt, 2, mytime );
        sqlite3_bind_int  ( pStmt, 3, tv.tv_sec);
        sqlite3_bind_int  ( pStmt, 4, tv.tv_usec);
        sqlite3_bind_text ( pStmt, 5, board_id, strlen(board_id), 0 );
        sqlite3_bind_text ( pStmt, 6, plugin_name, strlen(plugin_name), 0 );
        sqlite3_bind_blob ( pStmt, 7, pBlob, lBlobLen, SQLITE_STATIC );

        /* Call sqlite3_step() to run the virtual machine. Since the SQL being
        ** executed is not a SELECT statement, we assume no data will be returned.
        */
        rc = sqlite3_step( pStmt );
        assert( rc!=SQLITE_ROW );

        /* Finalize the virtual machine. This releases all memory and other
        ** resources allocated by the sqlite3_prepare() call above.
        */
        rc = sqlite3_finalize( pStmt );

        /* If sqlite3_finalize() returned SQLITE_SCHEMA, then try to execute
        ** the statement again.
        */
    } while (rc==SQLITE_SCHEMA);

    return( rc );
} /* bmon_sqlite3_write_to_db */

static int bmon_sqlite3_callback_select_all(
    void  *poutput_buffer,
    int    argc,
    char **argv,
    char **azColName
    )
{
    int            i = 0;
    int            nBlob = 0;
    unsigned char *zBlob = NULL;
    unsigned long int tblStructID = 0;
    unsigned long int tv_sec = 0;
    unsigned long int tv_usec = 0;
    char              *plugin_name = NULL;
    char              *board_id = NULL;
    unsigned char     *struct_data = NULL;

    /*fprintf( stderr, "%s ... argc %d\n", __FUNCTION__, argc );*/
    for (i = 0; i<argc; i++) {
        if ( strcmp( azColName[i], "struct_data" ) == 0 ) {
            if (SQLITE_OK!= bmon_sqlite3_read_blob( tblStructID, &zBlob, &nBlob ))
            {
                fprintf( stderr, "bmon_sqlite3_read_blob(%lu) Failed\n", tblStructID );
            } else {
                if (!zBlob)
                {
                    fprintf( stderr, "No such database entry for  %lu\n", tblStructID );
                    fprintf( stderr, "tv_sec %lu; tv_usec %lu; plugin_name %s; board_id %p; struct_data %p\n",
                            tv_sec, tv_usec, plugin_name, board_id, struct_data );
                } else {
                    /*fprintf( stderr, "%s = bytes(%d) \n", azColName[i], nBlob );*/
                    struct_data = zBlob;
                }
            }
        } else {
            /*fprintf( stderr, "%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL" );*/
            /* if this entry is the table index, save the index so we can used it to get the binary blob data */
            if ( strcmp( azColName[i], "tblStructID" ) == 0 ) {
                tblStructID = atoi( argv[i] );
            } else if ( strcmp( azColName[i], "tv_sec" ) == 0 ) {
                tv_sec = atoi( argv[i] );
            } else if ( strcmp( azColName[i], "tv_usec" ) == 0 ) {
                tv_usec = atoi( argv[i] );
            } else if ( strcmp( azColName[i], "plugin_name" ) == 0 ) {
                plugin_name = argv[i];
            } else if ( strcmp( azColName[i], "board_id" ) == 0 ) {
                board_id = argv[i];
            }
        }
    }

#ifdef MYSQL_SUPPORT
    do {
        MYSQL          *hMysql = NULL;
        unsigned long int mysqlID = 0;

        if ( nBlob > 0 ) {
            char * hexstr = NULL;
            unsigned hexstr_len = 0;

            hMysql = bmon_mysql_init ();
            if ( hMysql == NULL ) {
                fprintf( stderr, "bmon_mysql_init() Failed\n" );
                break;
            }

            /* convert the binary blob to a hex string for insertion into mysql database */
            hexstr_len = nBlob*2 + 8;
            hexstr = malloc ( hexstr_len );
            if ( hexstr) {
                int idx = 0;

                memset( hexstr, 0, hexstr_len );

                for( idx=0; idx<nBlob; idx++ ) {
                    sprintf( &hexstr[idx*2], "%02x", zBlob[idx] );
                }
                /*fprintf( stderr, "%s - nBlob %d ... strlen %d ... (%s)\n", __FUNCTION__, nBlob, (int) strlen(hexstr), hexstr );*/

                mysqlID = bmon_mysql_insert_record( hMysql, tv_sec, tv_usec, board_id, plugin_name, hexstr );
                free(hexstr);

                if ( ! mysqlID ) {
                    fprintf( stderr, "bmon_mysql_insert_record() Failed\n" );
                }
            }

            bmon_mysql_close( hMysql );
        }
    } while (0);
#endif /* MYSQL_SUPPORT */


    if ( zBlob ) free( zBlob );

    /*fprintf( stderr, "%s - done\n", __FUNCTION__ );*/
    return( 0 );
}

int bmon_parse_all_records(
    const char *tableName
    )
{
    char  *sql_str = NULL;
    char  *zErrMsg = 0;
    int    rc      = 0;

    if ( g_db == NULL || tableName == NULL) return 0;

    sql_str = malloc( sql_str_size );
    if ( sql_str == NULL ) {
        fprintf( stderr, "%s - failed to malloc(%d)\n", __FUNCTION__, sql_str_size );
        return -1;
    }

    sprintf( sql_str, "SELECT * FROM %s;", tableName );
    /*fprintf( stderr, "%s - sql (%s)\n", __FUNCTION__, sql_str );*/

    /* Execute SQL statement */
    rc = sqlite3_exec( g_db, sql_str, bmon_sqlite3_callback_select_all, NULL, &zErrMsg );

    if (rc != SQLITE_OK)
    {
        fprintf( stderr, "SQL error: %s\n", zErrMsg );
        sqlite3_free( zErrMsg );
    }

    if ( sql_str == NULL ) free( sql_str );

    return 0;
}
int bmon_parse_sqlite_db(
    const char *filename
        )
{
    int rc = 0;

    /*fprintf( stderr, "%s - file (%s)\n", __FUNCTION__, filename );*/
    if ( filename == NULL ) return -1;

    rc = bmon_sqlite3_open( (char*) filename );
    if (rc)
    {
        fprintf( stderr, "%s - bmon_sqlite3_open(%s) Failed\n", __FUNCTION__, filename );
        return 0;
    }

    rc = bmon_parse_all_records( "tblStructs" );
    if (rc)
    {
        fprintf( stderr, "%s - bmon_parse_all_records() Failed\n", __FUNCTION__ );
    }

    rc = bmon_sqlite3_close();
    if (rc)
    {
        fprintf( stderr, "%s - bmon_sqlite3_close() Failed\n", __FUNCTION__ );
    }

    return rc;
}

/*
** Read a blob from database db. Return an SQLite error code.
*/
int bmon_sqlite3_read_blob(
    unsigned int    tblStructID,
    unsigned char **pzBlob,                                /* Set *pzBlob to point to the retrieved blob */
    int            *pnBlob                                 /* Set *pnBlob to the size of the retrieved blob */
    )
{
    const char   *zSql = "SELECT struct_data FROM tblStructs WHERE tblStructID = ?";
    sqlite3_stmt *pStmt;
    int           rc;

    //fprintf( stderr, "%s - tblStructID %u\n", __FUNCTION__, tblStructID );

    /* In case there is no table entry for key zKey or an error occurs, set *pzBlob and *pnBlob to 0 now.  */
    *pzBlob = 0;
    *pnBlob = 0;

    do {
        /* Compile the SELECT statement into a virtual machine. */
        rc = sqlite3_prepare( g_db, zSql, -1, &pStmt, 0 );
        if (rc!=SQLITE_OK)
        {
            fprintf( stderr, "%s - sqlite3_prepare Failed\n", __FUNCTION__ );
            break;
        }

        /* Bind the key to the SQL variable. */
        sqlite3_bind_int( pStmt, 1, tblStructID );

        /* Run the virtual machine. We can tell by the SQL statement that
        ** at most 1 row will be returned. So call sqlite3_step() once
        ** only. Normally, we would keep calling sqlite3_step until it
        ** returned something other than SQLITE_ROW.
        */
        rc = sqlite3_step( pStmt );
        if (rc==SQLITE_ROW)
        {
            /* The pointer returned by sqlite3_column_blob() points to memory
            ** that is owned by the statement handle (pStmt). It is only good
            ** until the next call to an sqlite3_XXX() function (e.g. the
            ** sqlite3_finalize() below) that involves the statement handle.
            ** So we need to make a copy of the blob into memory obtained from
            ** malloc() to return to the caller.
            */
            *pnBlob = sqlite3_column_bytes( pStmt, 0 );
            *pzBlob = (unsigned char *)malloc( *pnBlob );
            memcpy( *pzBlob, sqlite3_column_blob( pStmt, 0 ), *pnBlob );
            //fprintf( stderr, "%s - memcpy(%d) \n", __FUNCTION__, *pnBlob );
        } else {
            fprintf( stderr, "%s - sqlite3_step() Failed\n", __FUNCTION__ );
        }

        /* Finalize the statement (this releases resources allocated by
        ** sqlite3_prepare() ).
        */
        rc = sqlite3_finalize( pStmt );

        /* If sqlite3_finalize() returned SQLITE_SCHEMA, then try to execute
        ** the statement all over again.
        */
    } while (rc==SQLITE_SCHEMA);

    return( rc );
} /* bmon_sqlite3_read_blob */

int bmon_upload_file( const char * filename )
{
    int rc = -1;
    unsigned char *contents = NULL;
    unsigned int   filesize = 1;
    unsigned int   retry    = 0;

    do {
        if ( filename == NULL ) {
            fprintf( stderr, "%s - filename cannot be null\n", __FUNCTION__ );
            break;
        }

        filesize = bmon_get_file_size( filename );
        if ( filesize == 0 ) {
            fprintf( stderr, "%s - filesize cannot be zero\n", __FUNCTION__ );
            break;
        }

        contents = (unsigned char*) bmon_get_file_contents( filename );
        if ( contents == NULL ) {
            fprintf( stderr, "%s - could not read contents of file (%s)\n", __FUNCTION__, filename );
            break;
        }

        do {
#ifdef RSFTP_SUPPORT
            rc = rsftpSend ( CLOUD_SERVER, contents, filesize, filename );
            if ( rc != 0 ) {
                fprintf( stderr, "%s - could not rsftpSend file (%s) to (%s) ... retry %u \n", __FUNCTION__, filename, CLOUD_SERVER, retry );
            }
#else
            rc = 0;
#endif /* RSFTP_SUPPORT */
            retry++;
        } while ( rc != 0 && retry < 5 );

    } while ( 0 );

    if ( contents ) free( contents );

    return rc;
}

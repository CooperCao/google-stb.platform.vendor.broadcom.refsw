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
#ifndef _BMON_SQLITE_UTILS_H_
#define _BMON_SQLITE_UTILS_H_
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "bmon.h"

#define  BMON_DATABASE_NAME  "/bin/bmon.db"
#define  SQL_BUFFER_SIZE        4096
#define  SQLITE_NUM_COLUMNS_MAX 70

typedef char COLUMN_DEFINITION[128];
typedef struct
{
    COLUMN_DEFINITION column_definition[ SQLITE_NUM_COLUMNS_MAX ];
} TABLE_SCHEMA;

int bmon_sqlite3_callback_insert( void *NotUsed, int argc, char **argv, char **azColName );
int bmon_sqlite3_open( char *db_name );
int bmon_sqlite3_create( const char *create_str );
int bmon_sqlite3_insert( const char *urlString );
int bmon_sqlite3_select_table_schema( const char *tableName, char *outputBuffer );
int bmon_sqlite3_select_last_row( const char *tableName, const char *indexKeyName, char *outputBuffer );
int bmon_sqlite3_callback_select_last_row( void *NotUsed, int argc, char **argv, char **azColName );
int bmon_sqlite3_close( void );
int bmon_sqlite3_write_to_db( const unsigned char *pBlob, int lBlobLen, const char *plugin_name, const char *myMacAddress );
int bmon_parse_sqlite_db( const char *filename );
int bmon_sqlite3_read_blob( unsigned int tblStructID, unsigned char **pzBlob, int *pnBlob );
int bmon_upload_file( const char * filename );

#endif /* _BMON_SQLITE_UTILS_H_ */

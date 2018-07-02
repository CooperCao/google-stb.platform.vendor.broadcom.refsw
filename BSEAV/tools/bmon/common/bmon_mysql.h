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
#ifndef BMON_MYSQL_H
#define BMON_MYSQL_H

#include <mysql.h>
#include "bmon.h"

#define MYSQL_SERVER "localhost"
#define MYSQL_USER   "dmsroot"
#define MYSQL_PW     "Br0adc0m"

typedef enum
{
    BMON_MYSQL_CONJUNCTION_TYPE_NONE,
    BMON_MYSQL_CONJUNCTION_TYPE_COMMA,
    BMON_MYSQL_CONJUNCTION_TYPE_AND,
} BMON_MYSQL_CONJUNCTION_TYPE_T;

int bmon_mysql_close( MYSQL * hMysql );
MYSQL * bmon_mysql_init ( void );
int bmon_mysql_cat_int( char * outputStr, unsigned int outputStr_Len, unsigned long int FieldValue, const char *FieldName, BMON_MYSQL_CONJUNCTION_TYPE_T conjunction );
int bmon_mysql_cat_str( char * outputStr, unsigned int outputStr_Len, const char *FieldValue, const char *FieldName, BMON_MYSQL_CONJUNCTION_TYPE_T conjunction );
unsigned long int bmon_mysql_find_tbl_struct_id(
    MYSQL       *hMysql,
    char        *outputStr,
    unsigned int outputStr_Len,
    const char  *tv_sec,
    const char  *tv_usec,
    const char  *board_id,
    const char  *plugin_name
    );
unsigned long int bmon_mysql_insert_record(
    MYSQL       *hMysql,
    unsigned long int tv_sec,
    unsigned long int tv_usec,
    const char  *board_id,
    const char  *plugin_name,
    const char  *struct_data
    );
int bmon_mysql_cat_hex(
    char                            *outputStr,
    unsigned int                     outputStr_Len,
    const char                      *FieldValue,
    const char                      *FieldName,
    BMON_MYSQL_CONJUNCTION_TYPE_T conjunction
    );
int bmon_mysql_get_data(
    const char  *plugin_name,
    const char *url_in,
    void *mysql_data,
    unsigned int mysql_data_size,
    unsigned int *ptv_sec,
    unsigned int *ptv_usec
    );

#endif /* BMON_MYSQL_H */

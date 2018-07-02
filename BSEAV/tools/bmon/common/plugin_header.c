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
#include <string.h>
#include <time.h>
#include <sys/time.h>

static int strgmdate( char *tmbuf, int tmbuf_len )
{
    struct timeval tv;
    struct timezone tz;
    time_t nowtime;
    struct tm *gmtm;

    memset( &tv, 0, sizeof(tv) );
    memset( &tz, 0, sizeof(tz) ); /* int tz_minuteswest ... and ... int tz_dsttime */
    gettimeofday(&tv, &tz );
    /*fprintf(stderr, "tv: %ld.%06ld ... minwest %d ... dsttime %d \n", tv.tv_sec, tv.tv_usec, tz.tz_minuteswest, tz.tz_dsttime );*/
    nowtime = tv.tv_sec;
    gmtm = gmtime(&nowtime);
    /* $date  Thu Jan  1 04:19:05 1970 */
    if ( tmbuf_len > 28 ) {
        strftime(tmbuf, tmbuf_len, "%b %e %H:%M:%S.000 %Y", gmtm); /* Jan  1 12:01:02.456 1970 */
        snprintf(&tmbuf[16], tmbuf_len, "%03ld", tv.tv_usec/1000 );
        tmbuf[19] = ' ';
    }
    /*fprintf(stderr, "tmbuf (%s) ... len %d \n", tmbuf, tmbuf_len );*/

    return 0;
}
/**
 *  Function: This function will collect all requred power data and store the data into a
 *            known structure.
 **/
int append_plugin_header(
    char            *payload,
    int              payload_size,
    const char      *plugin_version,
    const char      *plugin_name,
    const char      *plugin_description)
{
    unsigned int    num_bytes = 0;
    char            pctime[128];
    struct timeval  tv;
    char            one_line[128];

    gettimeofday( &tv, NULL );
    strgmdate(pctime, sizeof(pctime) );
    num_bytes += snprintf( one_line, sizeof( one_line ), "{\"version\":\"%s\",\"name\":\"%s\",\"description\":\"%s\"", plugin_version, plugin_name, plugin_description );
    strncat( payload, one_line, payload_size );

    num_bytes += snprintf( one_line, sizeof( one_line ), ",\"datetime\":\"%s\",\"timestamp\":\"%ld.%06ld\"", pctime,
        (unsigned long int) tv.tv_sec, (unsigned long int) tv.tv_usec );
    strncat( payload, one_line, payload_size - strlen( payload ) - 1 );

    strncat( payload, ",\"data\":[", payload_size - strlen( payload ) - 1 );
    num_bytes += 9;

    return num_bytes;
}

int append_plugin_footer(
    char            *payload,
    int              payload_size)
{
    unsigned num_bytes = 0;

    if ( payload && payload_size ) {
        strncat( payload, "]}", payload_size - strlen( payload ) - 1 );
        num_bytes += 2;
    }
    return num_bytes ;
}

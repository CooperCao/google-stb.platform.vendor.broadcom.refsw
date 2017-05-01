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
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <locale.h>
#include <math.h>
#include <sys/stat.h>
#include <errno.h>
#include "bstd.h"
#include "bmemperf_server.h"
#include "bmemperf_cgi.h"
#include "bmemperf_utils.h"
#include "bmemperf_info.h"
#include "nexus_platform.h"

#define SYSFS_SCB_FREQ_FILENAME "/sys/kernel/debug/clk/clk_summary"

#ifdef USE_BOXMODES
extern bmemperf_boxmode_info g_boxmodes[BMEMPERF_MAX_BOXMODES];
#endif /* USE_BOXMODES */
extern int   g_MegaBytes;
extern int   g_MegaBytesDivisor[2];
extern char *g_MegaBytesStr[2];

#define UTILS_LOG_FILE_FULL_PATH_LEN 64
#define MAX_LENGTH_DDR_FREQ 256
#define MAX_LENGTH_SCB_FREQ 256

static unsigned int g_bmemperf_ddr_freq_mhz[NEXUS_MAX_MEMC] = {0,0,0};
static unsigned int g_bmemperf_scb_freq_mhz = 0;


extern char         *g_client_name[BMEMPERF_MAX_NUM_CLIENT];
extern bmemperf_info g_bmemperf_info;

const char *nofprintf(
    FILE       *stdsomething,
    const char *format,
    ...
    )
{
    if (stdsomething == NULL)
    {
    }
    return( format );
}

char *getClientName(
    int client_index
    )
{
    return( &g_client_name[client_index][0] );
} /* getClientName */

pid_t daemonize(
    const char * logFileName
    )
{
    pid_t pid = 0, sid = 0;
    int   fd  = 0;

    /* already a daemon */
    if (getppid() == 1)
    {
        printf( "%s: already a daemon\n", __FUNCTION__ );
        return( 0 );
    }

    /* Fork off the parent process */
    pid = fork();
    if (pid < 0)
    {
        printf( "%s: fork() command failed\n", __FUNCTION__ );
        exit( EXIT_FAILURE );
    }

    if (pid > 0)
    {
        /*printf( "%u: This is the parent; exiting immediately\n", pid );*/
    }
    else
    {
        char utilsLogFilename[UTILS_LOG_FILE_FULL_PATH_LEN];

        /*printf( "%u: This is the child; starting daemonize process\n", pid );*/

        /* At this point we are executing as the child process */
        /* Create a new SID for the child process */
        sid = setsid();
        if (sid < 0)
        {
            exit( EXIT_FAILURE );
        }

        printf( "%s: new sid is %d\n", __FUNCTION__, sid );
        /* Change the current working directory. */
        if (( chdir( GetTempDirectoryStr())) < 0)
        {
            exit( EXIT_FAILURE );
        }

        fd = open( "/dev/null", O_RDWR, 0 );

        if (fd != -1)
        {
            /*printf( "%u: Assign STDIN to /dev/null\n", pid );*/
            dup2( fd, STDIN_FILENO );

            if (fd > 2)
            {
                close( fd );
            }
        }

        if ( logFileName && strlen(logFileName) )
        {
            PrependTempDirectory( utilsLogFilename, sizeof( utilsLogFilename ), logFileName );
            fd = open( utilsLogFilename, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR  );

            if (fd != -1)
            {
                /*printf( "%u: Assign STDOUT/STDERR to fd (%u)\n", pid, fd );*/
                dup2( fd, STDOUT_FILENO );
                dup2( fd, STDERR_FILENO );
            }
        }

        /*reset file creation mask */
        umask( 0177 );
    }

    return( pid );
} /* daemonize */

int sort_on_id(
    const void *a,
    const void *b
    )
{
    int rc = 0;

    bmemperf_client_data *client_a = (bmemperf_client_data *)a;
    bmemperf_client_data *client_b = (bmemperf_client_data *)b;

    rc = client_b->client_id - client_a->client_id;

    return( rc );
} /* sort_on_id */

int sort_on_id_rev(
    const void *a,
    const void *b
    )
{
    return( sort_on_id( b, a ));
} /* sort_on_id_rev */

int sort_on_bw(
    const void *a,
    const void *b
    )
{
    int rc = 0;

    bmemperf_client_data *client_a = (bmemperf_client_data *)a;
    bmemperf_client_data *client_b = (bmemperf_client_data *)b;

    rc = client_b->bw - client_a->bw;

    return( rc );
} /* sort_on_bw */

int sort_on_bw_rev(
    const void *a,
    const void *b
    )
{
    return( sort_on_bw( b, a ));
} /* sort_on_bw_rev */

int changeFileExtension(
    char       *destFilename,
    int         sizeofDestFilename,
    const char *srcFilename,
    const char *fromext,
    const char *toext
    )
{
    char *pos = NULL;

    PRINTF( "~%s: strcpy(%s)(%s)(%d)\n~", __FUNCTION__, destFilename, srcFilename, sizeofDestFilename );
    strncpy( destFilename, srcFilename, sizeofDestFilename );

    /* change the extension to new one ...   .tgz to 2.csv */
    pos = strstr( destFilename, fromext );
    PRINTF( "~%s: dest (%s)(%p); pos (%p); size (%d)\n~", __FUNCTION__, destFilename, destFilename, pos, sizeofDestFilename );
    PRINTF( "~%s: src  (%s)(%p); from (%s); to (%s)\n~", __FUNCTION__, srcFilename, srcFilename, fromext, toext );
    if (pos)
    {
        pos[0] = '\0'; /* null-terminate the beginning part of the name */
        strncat( destFilename, toext, sizeofDestFilename - strlen( destFilename ));
    }

    PRINTF( "destfilename (%s)\n", destFilename );

    return( 0 );
} /* changeFileExtension */

/**
 *  Function: This function will create the HTML needed to display the Top X clients.
 **/
int clientListCheckboxes(
    unsigned int top10Number
    )
{
    unsigned int idx = 0, memc = 0, client_idx = 0, displayCount[NEXUS_NUM_MEMC];

    printf( "~CLIENT_LIST_CHECKBOXES~" );
    PRINTF( "top10Number %u<br>\n", top10Number );

    memset( displayCount, 0, sizeof( displayCount ));

#define COLUMNS_PER_MEMC 2
    printf( "<table style=border-collapse:collapse; border=1 >\n" );
    for (idx = 0; idx<top10Number+2 /*there are two header rows */; idx++)
    {
        printf( "<tr>" );
        for (memc = 0; memc<g_bmemperf_info.num_memc; memc++)
        {
            /* 1st row */
            if (idx==0)
            {
                printf( "<th colspan=%u >MEMC %u</th>", COLUMNS_PER_MEMC, memc );
            }
            /* 2nd row */
            else if (idx==1)
            {
                printf( "<th style=\"width:50px;\"  >ID</th>" );
                printf( "<th style=\"width:150px;\" align=left >Name</th>" );
            }
            else
            {
                client_idx = idx-2;
                if (( strncmp( g_client_name[client_idx], "sage", 4 ) != 0 ) && ( strncmp( g_client_name[client_idx], "bsp", 3 ) != 0 ))
                {
                    printf( "<td id=client%umemc%ubox align=left ><input type=checkbox id=client%umemc%u onclick=\"MyClick(event);\" >%u</td>",
                        client_idx, memc, client_idx, memc, client_idx );
                    printf( "<td id=client%umemc%uname >%s</td>", client_idx, memc, g_client_name[client_idx] );
                }
            }
        }
        PRINTF( "<br>\n" );
        printf( "</tr>\n" );
    }
    printf( "</table>\n" );
    return( 0 );
} /* clientListCheckboxes */

int getCpuOnline(
    bmemperf_cpu_status *pCpuStatus
    )
{
    unsigned int cpu = 0;
    FILE        *cmd = NULL;
    char         line[MAX_LENGTH_CPU_STATUS];

    if (pCpuStatus == NULL)
    {
        return( -1 );
    }

    /* clear out the array */
    memset( pCpuStatus, 0, sizeof( *pCpuStatus ));

    sprintf( line, "grep processor /proc/cpuinfo" );
    cmd = popen( line, "r" );

    PRINTF( "detected cpu: " );
    do {
        memset( line, 0, sizeof( line ));
        fgets( line, MAX_LENGTH_CPU_STATUS, cmd );
        if (strlen( line ))
        {
            char *pos = strchr( line, ':' );
            if (pos)
            {
                pos++;
                cpu = strtoul( pos, NULL, BMEMPERF_CPU_VALUE_LENGTH );
                PRINTF( "%u ", cpu );
                if (cpu < BMEMPERF_MAX_NUM_CPUS)
                {
                    pCpuStatus->isActive[cpu] = true;
                }
            }
        }
    } while (strlen( line ));
    PRINTF( "\n" );

    pclose( cmd );
    return( 0 );
} /* getCpuOnline */

#ifdef USE_BOXMODES
int bmemperf_boxmode_init(
    long int boxmode
    )
{
    unsigned int idx         = 0;
    bool         bMatchFound = false;

    if (boxmode >= 0) /* older chips (like 97425) only have boxmode 0 */
    {
        /* loop through all boxmodes to find the client names, MHz, and SCB freq for the current box mode */
        for (idx = 0; idx<BMEMPERF_MAX_BOXMODES; idx++)
        {
            if (( 0 < g_boxmodes[idx].boxmode ) && ( g_boxmodes[idx].boxmode < BMEMPERF_MAX_BOXMODES ) &&
                ( g_boxmodes[idx].g_bmemperf_info ) && ( g_boxmodes[idx].g_client_name) )
            {
                if (boxmode == (long int) g_boxmodes[idx].boxmode)
                {
                    PRINTF( "~boxmode:%ld; copying from idx %u\n~", boxmode, idx );
                    memcpy( &g_bmemperf_info, g_boxmodes[idx].g_bmemperf_info, sizeof( g_bmemperf_info ));
                    memcpy( &g_client_name, g_boxmodes[idx].g_client_name, sizeof( g_client_name ));
                    boxmode     = g_boxmodes[idx].boxmode;
                    bMatchFound = true;
                    break;
                }
            }
            else
            {
                printf( "~boxmode:%ld; stopped looking at idx %u\n~", boxmode, idx );
                break;
            }
        }
    }

    /* if a match was not found above, then use the first entry in the array */
    if (bMatchFound == false)
    {
        idx = 0;
        printf( "~boxmode:%ld; copying from default idx %u\n~", boxmode, idx );
        memcpy( &g_bmemperf_info, g_boxmodes[idx].g_bmemperf_info, sizeof( g_bmemperf_info ));
        memcpy( &g_client_name, g_boxmodes[idx].g_client_name, sizeof( g_client_name ));
        boxmode = g_boxmodes[idx].boxmode;
    }

    PRINTF( "~boxmode: returning %ld\n~", boxmode );
    return( boxmode );
} /* bmemperf_boxmode_init */

int getBoxModeHtml(
    char *buf,
    int   len,
    int   boxmodePlatform
    )
{
    unsigned int            idx       = 0;
    int                     remaining = len;
    int                     written   = 0;
    char                   *output    = buf;
    bmemperf_boxmode_source boxmodeSource;
    char                   *boxmodeSources[BOXMODE_SOURCES_NUM] = {"NEXUS", "BOLT", "BROWSER"};
    char                    boxmodeSourceStr[16];

    memset( buf, '\0', len );
    memset( &boxmodeSource, 0, sizeof( boxmodeSource ));
    memset( &boxmodeSourceStr, 0, sizeof( boxmodeSourceStr ));

    get_boxmode( &boxmodeSource );

    written    = snprintf( output, remaining, "<select id=\"boxmode\" onchange=\"MyClick(event);\" onmousedown=\"MySelect(event);\" >" );
    remaining -= written;
    output    += written;

    /* loop through all boxmodes to find the client names, DDR freq, and SCB freq for the current box mode */
    for (idx = 0; idx<BMEMPERF_MAX_BOXMODES; idx++)
    {
        if (g_boxmodes[idx].boxmode == boxmodeSource.boxmode)
        {
            snprintf( boxmodeSourceStr, sizeof( boxmodeSourceStr ), "  (%s)", boxmodeSources[boxmodeSource.source] );
        }
        else
        {
            boxmodeSourceStr[0] = '\0';
        }
        if (( g_boxmodes[idx].boxmode > 0 ) && ( g_boxmodes[idx].boxmode < BMEMPERF_MAX_BOXMODES ) &&
            ( g_boxmodes[idx].g_bmemperf_info ) && ( g_boxmodes[idx].g_client_name) )
        {
            written = snprintf( output, remaining, "<option value=%d %s >BoxMode %d%s</option>", g_boxmodes[idx].boxmode,
                    ( boxmodePlatform == g_boxmodes[idx].boxmode ) ? "selected" : "", g_boxmodes[idx].boxmode, boxmodeSourceStr );

            if (written>=remaining)
            {
                remaining = 0;
                break;
            }
            remaining -= written;
            output    += written;
        }
        else
        {
            break;
        }
    }

    written    = snprintf( output, remaining, "</select >" );
    remaining -= written;
    output    += written;

    /*printf("%s",buf);*/
    return( len-remaining );
} /* getBoxModeHtml */

#endif /* USE_BOXMODES */

int overall_stats_html(
    bmemperf_response *pResponse
    )
{
    unsigned int      memc = 0;
    unsigned int      cpu  = 0;
    unsigned int      numProcessorsConfigured = sysconf( _SC_NPROCESSORS_CONF );
    unsigned int      numProcessorsOnline     = sysconf( _SC_NPROCESSORS_ONLN );
    float             averageCpuUtilization   = 0.0;
    bmemperf_irq_data irqData;
    char              tempStr[16];
    char              irqTotalStr[64];
    unsigned int      numberOfMemc = g_bmemperf_info.num_memc;
    char              warningHtml[256];

    PRINTF( "%s: numberOfMemc %u<br>\n", __FUNCTION__, numberOfMemc );
    printf( "~OVERALL~" );
    printf( "<table ><tr><th>OVERALL STATISTICS:</th></tr></table>\n" );
    printf( "<table style=border-collapse:collapse; border=1 >\n" );

    memset( &irqData, 0, sizeof( irqData ));
    memset( &irqTotalStr, 0, sizeof( irqTotalStr ));

    for (memc = 0; memc<numberOfMemc; memc++)
    {
        /* first column */
        if (memc == 0)
        {
            printf( "<tr><th>&nbsp;</th>" );
        }
        printf( "<th>MEMC&nbsp;%u</th>", memc );

        /* last column */
        if (( memc+1 ) == numberOfMemc)
        {
            printf( "</tr>\n" );
        }
    }
    for (memc = 0; memc<numberOfMemc; memc++)
    {
        /* first column */
        if (memc == 0)
        {
            printf( "<tr><td>Data BW&nbsp;(%s)</td>", g_MegaBytesStr[g_MegaBytes] );
        }
        convert_to_string_with_commas( pResponse->response.overallStats.systemStats[memc].dataBW / g_MegaBytesDivisor[g_MegaBytes], tempStr, sizeof( tempStr ) );
        printf( "<td>%s</td>", tempStr );

        /* last column */
        if (( memc+1 ) == numberOfMemc)
        {
            printf( "</tr>\n" );
        }
    }
    for (memc = 0; memc<numberOfMemc; memc++)
    {
        /* first column */
        if (memc == 0)
        {
            printf( "<tr><td>Transaction&nbsp;BW&nbsp;(%s)</td>", g_MegaBytesStr[g_MegaBytes] );
        }
        convert_to_string_with_commas( pResponse->response.overallStats.systemStats[memc].transactionBW / g_MegaBytesDivisor[g_MegaBytes], tempStr, sizeof( tempStr ) );
        printf( "<td>%s</td>", tempStr );

        /* last column */
        if (( memc+1 ) == numberOfMemc)
        {
            printf( "</tr>\n" );
        }
    }
    for (memc = 0; memc<numberOfMemc; memc++)
    {
        /* first column */
        if (memc == 0)
        {
            printf( "<tr><td>Idle&nbsp;BW&nbsp;(%s)</td>", g_MegaBytesStr[g_MegaBytes] );
        }
        convert_to_string_with_commas( pResponse->response.overallStats.systemStats[memc].idleBW / g_MegaBytesDivisor[g_MegaBytes], tempStr, sizeof( tempStr ) );
        printf( "<td>%s</td>", tempStr );

        /* last column */
        if (( memc+1 ) == numberOfMemc)
        {
            printf( "</tr>\n" );
        }
    }
    for (memc = 0; memc<numberOfMemc; memc++)
    {
        /* first column */
        if (memc == 0)
        {
            printf( "<tr><td>Utilization %%</td>" );
        }
        printf( "<td>%5.2f</td>", pResponse->response.overallStats.systemStats[memc].dataUtil );

        /* last column */
        if (( memc+1 ) == numberOfMemc)
        {
            printf( "</tr>\n" );
        }
    }
    for (memc = 0; memc<numberOfMemc; memc++)
    {
        memset ( warningHtml, 0, sizeof(warningHtml) );

        /* first column */
        if (memc == 0)
        {
            unsigned idx;

            /* if the frequency differs from the frequency read from the header file during compile time, show a warning */
            for (idx = 0; idx<numberOfMemc; idx++)
            {
                if ( g_bmemperf_info.ddrFreqInMhz != pResponse->response.overallStats.systemStats[idx].ddrFreqMhz )
                {
                    snprintf( warningHtml, sizeof(warningHtml) - 1, "<span title=\"Compile-time header frequency (in red) does not match BOLT frequency (in black)!\" "
                                                                    "style=\"font-size:8pt;color:red;\" > (%u)</span>", g_bmemperf_info.ddrFreqInMhz );
                    break;
                }
            }
            printf( "<tr><td>DDR&nbsp;Freq&nbsp;(MHz)%s</td>", warningHtml );
        }
        printf( "<td>%u</td>", pResponse->response.overallStats.systemStats[memc].ddrFreqMhz );

        /* last column */
        if (( memc+1 ) == numberOfMemc)
        {
            printf( "</tr>\n" );
        }
    }
    for (memc = 0; memc<numberOfMemc; memc++)
    {
        memset ( warningHtml, 0, sizeof(warningHtml) );

        /* first column */
        if (memc == 0)
        {
            unsigned idx;

            /* if the frequency differs from the frequency read from the header file during compile time, show a warning */
            for (idx = 0; idx<numberOfMemc; idx++)
            {
                if ( g_bmemperf_info.scbFreqInMhz != pResponse->response.overallStats.systemStats[idx].scbFreqMhz )
                {
                    snprintf( warningHtml, sizeof(warningHtml) - 1, "<span title=\"Compile-time header frequency (in red) does not match BOLT frequency (in black)!\" "
                                                                    "style=\"font-size:8pt;color:red;\" > (%u)</span>", g_bmemperf_info.scbFreqInMhz );
                    break;
                }
            }
            printf( "<tr><td>SCB&nbsp;Freq&nbsp;(MHz)%s</td>", warningHtml );
        }
        printf( "<td>%u</td>", pResponse->response.overallStats.systemStats[memc].scbFreqMhz );

        /* last column */
        if (( memc+1 ) == numberOfMemc)
        {
            printf( "</tr>\n" );
        }
    }

    printf( "</table>~" );

    /* convert 999999 to 999,999 ... e.g. add commas */
    convert_to_string_with_commas( pResponse->response.overallStats.irqData.irqTotal,  irqTotalStr, sizeof( irqTotalStr ));

    printf( "~DETAILSTOP~" );
    printf( "<table border=0 style=\"border-collapse:collapse;\" width=\"100%%\" ><tr><th width=160px align=left >CLIENT DETAILS:</th>" );
    printf( "<th align=right ><table border=0 style=\"border-collapse:collapse;\" ><tr><th align=left>CPU&nbsp;UTILIZATION:&nbsp;&nbsp;</th>" );
    for (cpu = 0; cpu<numProcessorsConfigured; cpu++)
    {
        /* 255 indicates this CPU is offline */
        if (pResponse->response.overallStats.cpuData.idlePercentage[cpu] != 255)
        {
            averageCpuUtilization += pResponse->response.overallStats.cpuData.idlePercentage[cpu];
            printf( "<td align=center style=\"width:50px;border-top: solid thin; border-right: solid thin; border-left: solid thin; border-bottom: solid thin\" >%u%%</td>",
                pResponse->response.overallStats.cpuData.idlePercentage[cpu] );
        }
    }
    averageCpuUtilization /= numProcessorsOnline;
    printf( "<td align=center style=\"width:50px;border-top: solid thin; border-right: solid thin; border-left: solid thin; border-bottom: solid thin; "
            "background-color:lightgray;\"  >%5.1f%%</td>", averageCpuUtilization );
    printf( "<td>&nbsp;&nbsp;Irq/sec:&nbsp;&nbsp;</td>"
            "<td align=center style=\"width:50px;border-top: solid thin; border-right: solid thin; border-left: solid thin; border-bottom: solid thin; "
            "background-color:lightgray;\"  >%s</td>", irqTotalStr );
    printf( "</tr></table></th></tr></table>~\n" );

    return( 0 );
} /* overall_stats_html */

unsigned int getNextNonZeroIdx(
    unsigned int       displayCount,
    unsigned int       memc,
    bmemperf_response *pResponse,
    int                sortColumn
    )
{
    unsigned int client;
    unsigned int nonZeroCount = 0;

    BSTD_UNUSED( sortColumn );

    for (client = 0; client<BMEMPERF_MAX_NUM_CLIENT; client++)
    {
        /* if bandwidth is non-zero OR we are sorting on ID column or NAME column */
        /*if (pResponse->response.overallStats.clientOverallStats[memc].clientData[client].bw > 0 || abs(sortColumn) != BMEMPERF_COLUMN_BW )*/
        if (pResponse->response.overallStats.clientOverallStats[memc].clientData[client].client_id != BMEMPERF_CGI_INVALID)
        {
            if (nonZeroCount == displayCount)
            {
                PRINTF( "%s: for memc %u, idx %u\n", __FUNCTION__, memc, client );
                break;
            }
            nonZeroCount++;
        }
    }
    return( client );
} /* getNextNonZeroIdx */

/**
 *  Function: This function will create the HTML needed to display the Top X clients.
 **/
int top10_html(
    bmemperf_response *pResponse,
    unsigned int       top10Number
    )
{
    unsigned int idx = 0, memc = 0, client_idx = 0, displayCount[NEXUS_NUM_MEMC];
    char         client_checked[8];
    unsigned int numberOfMemc = g_bmemperf_info.num_memc;

    PRINTF( "%s: top10Number %u; numberOfMemc %u\n", __FUNCTION__, top10Number, numberOfMemc );
    printf( "~TOP10~" );

    for (memc = 0; memc<numberOfMemc; memc++)
    {
        displayCount[memc] = 0;
    }

#define TOP_COLUMNS_PER_MEMC 5
    printf( "<table style=border-collapse:collapse; border=1 >\n" );
    for (idx = 0; idx<top10Number+2 /*there are two header rows */; idx++)
    {
        printf( "<tr>" );
        for (memc = 0; memc<numberOfMemc; memc++)
        {
            /* 1st row */
            if (idx==0)
            {
                printf( "<th colspan=%u >MEMC %u</th>", TOP_COLUMNS_PER_MEMC, memc );
            }
            /* 2nd row */
            else if (idx==1)
            {
                /* negative values mean sort the opposite direction */
                int temp = (int) pResponse->response.overallStats.sortColumn[memc];
                PRINTF( "%s: sortColumn temp %d\n", __FUNCTION__, temp );
                printf( "<th style=\"width:50px;\"  id=sort%uid   onclick=\"MyClick(event);\" >ID</th>", memc );
                printf( "<th style=\"width:150px;\" id=sort%uname onclick=\"MyClick(event);\" align=left >Name</th>", memc );
                printf( "<th style=\"width:40px;\"><table><tr><th id=sort%ubw   onclick=\"MyClick(event);\" >BW&nbsp;(%s)</th></tr></table></th>", memc,
                    g_MegaBytesStr[g_MegaBytes] );
                printf( "<th>RR</th><th>Block&nbspOut</th>\n" );
            }
            else
            {
                client_idx = idx-2;
                {
                    char        *background     = NULL;
                    unsigned int background_len = 0;
                    char         rr_checked[8];
                    unsigned int nonZeroIdx = 0;
                    bool         isChecked  = 0;
                    int          temp       = (int) pResponse->response.overallStats.sortColumn[memc];

                    nonZeroIdx = getNextNonZeroIdx( displayCount[memc], memc, pResponse, temp );

                    if (memc==0) {PRINTF( "%s: idx %u: client %u: memc %u; displayCount %u; nonZeroIdx %u;<br>\n", __FUNCTION__, idx, client_idx, memc, displayCount[memc], nonZeroIdx ); }
                    /* if BW is zero OR name contains unprintable character, do not print anything */
                    if (nonZeroIdx >= BMEMPERF_MAX_NUM_CLIENT)
                    {
                        printf( "<td colspan=%u>&nbsp;</td>\n", TOP_COLUMNS_PER_MEMC );
                    }
                    else
                    {
                        isChecked = pResponse->response.overallStats.clientOverallStats[memc].clientData[nonZeroIdx].is_detailed;
                        if (memc==0)
                        {
                            PRINTF( "memc %u; cnt:%u-nonZero %u; id %u; chkd-%u;<br>", memc, displayCount[memc], nonZeroIdx,
                                pResponse->response.overallStats.clientOverallStats[memc].clientData[nonZeroIdx].client_id, isChecked );
                        }
                        memset( client_checked, 0, sizeof( client_checked ));
                        memset( rr_checked, 0, sizeof( rr_checked ));

                        if (isChecked)
                        {
                            strncpy( client_checked, "checked", sizeof( client_checked ));
                        }

                        if (pResponse->response.overallStats.clientOverallStats[memc].clientData[nonZeroIdx].rr)
                        {
                            strncpy( rr_checked, "checked", sizeof( rr_checked ));
                        }

                        if (pResponse->response.overallStats.clientOverallStats[memc].clientData[nonZeroIdx].err_cnt)
                        {
                            background_len = strlen( BACKGROUND_RED ) + 1;
                            background     = malloc( background_len );
                            strncpy( background, BACKGROUND_RED, background_len-1 );
                            background[background_len-1] = '\0';
                        }
                        else if (isChecked)
                        {
                            background_len = strlen( BACKGROUND_YELLOW ) + 1;
                            background     = malloc( background_len );
                            strncpy( background, BACKGROUND_YELLOW, background_len-1 );
                            background[background_len-1] = '\0';
                        }
                        else
                        {
                            background_len = strlen( BACKGROUND_WHITE ) + 1;
                            background     = malloc( background_len );
                            strncpy( background, BACKGROUND_WHITE, background_len-1 );
                            background[background_len-1] = '\0';
                        }
                        printf( "<td align=left %s ><input type=checkbox id=client%umemc%u onclick=\"MyClick(event);\" %s >%u</td>", background,
                            pResponse->response.overallStats.clientOverallStats[memc].clientData[nonZeroIdx].client_id, memc, client_checked,
                            pResponse->response.overallStats.clientOverallStats[memc].clientData[nonZeroIdx].client_id );
                        printf( "<td %s >%s",
                            background, g_client_name[pResponse->response.overallStats.clientOverallStats[memc].clientData[nonZeroIdx].client_id] );
                        if (pResponse->response.overallStats.clientOverallStats[memc].clientData[nonZeroIdx].err_cnt)
                        {
                            printf( " (%u)", pResponse->response.overallStats.clientOverallStats[memc].clientData[nonZeroIdx].err_cnt );
                        }
                        printf( "</td><td align=center %s >%u</td>",
                            background, pResponse->response.overallStats.clientOverallStats[memc].clientData[nonZeroIdx].bw / g_MegaBytesDivisor[g_MegaBytes] );
                        printf( "<td %s ><input type=checkbox id=client%urrobin%u %s onclick=\"MyClick(event);\" ></td>",
                            background, pResponse->response.overallStats.clientOverallStats[memc].clientData[nonZeroIdx].client_id, memc, rr_checked );
                        printf( "<td %s ><input type=text %s id=client%ublockout%u onmouseover=\"MyMouseOver(event);\" onmouseout=\"MyMouseOut(event);\" size=5 value=%u "
                                "onchange=\"MyChange(event);\" /></td>\n", background, background,
                            pResponse->response.overallStats.clientOverallStats[memc].clientData[nonZeroIdx].client_id, memc,
                            pResponse->response.overallStats.clientOverallStats[memc].clientData[nonZeroIdx].block_out );

                        Bsysperf_Free( background );
                    }

                    displayCount[memc]++;
                }
            }
        }
        PRINTF( "<br>\n" );
        printf( "</tr>\n" );
    }
    printf( "</table>\n" );
    return( 0 );
} /* top10_html */

int get_boxmode(
    bmemperf_boxmode_source *pboxmode_info
    )
{
    unsigned    boxMode = 0;
    const char *override;
    FILE       *pFile = NULL;

    if (pboxmode_info == NULL)
    {
        return( -1 );
    }

    override = getenv( "B_REFSW_BOXMODE" );
    if (override)
    {
        boxMode = atoi( override );
    }
    if (!boxMode)
    {
        pFile = fopen( BOXMODE_RTS_FILE, "r" );
        pboxmode_info->source = BOXMODE_RTS;
    }
    else
    {
        pboxmode_info->source = BOXMODE_ENV;
    }
    if (pFile)
    {
        char buf[64];
        if (fgets( buf, sizeof( buf ), pFile ))
        {
            /* example: 20140402215718_7445_box1, but skip over everything (which may change) and get to _box# */
            char *p = strstr( buf, "_box" );
            if (p)
            {
                boxMode = atoi( &p[4] );
            }
        }
        fclose( pFile );
    }
    pboxmode_info->boxmode = boxMode;
    return( 0 );
} /* get_boxmode */

/**
 *  Function: This function will scan the specified string and return true if a number digit is found; it will
 *  return false otherwise.
 **/
bool hasNumeric(
    const char *mystring
    )
{
    const char *nextchar = mystring;

    while (*nextchar != '\0')
    {
        if (( '0' <= *nextchar ) && ( *nextchar <= '9' ))
        {
            return( 1 );
        }
        nextchar++;
        /*printf("nextchar (%c)\n", *nextchar );*/
    }

    /*printf("string (%s) has no numbers\n", mystring );*/
    return( 0 );
} /* hasNumeric */

/**
 *  Function: This function will open the "frequency" file and convert the string frequency value to an integer
 *  value and set the global variable from the converted integer value.
 **/
static unsigned int bmemperf_read_ddrFreq( char * path_to_frequency_file )
{
    FILE * fp = NULL;
    char strFrequency[12];
    unsigned int frequency = 0;

    if (path_to_frequency_file == NULL)
    {
        return ( frequency );
    }

    strcat ( path_to_frequency_file, "frequency" );
    /*printf("%s: fullname (%s)\n", __FUNCTION__, path_to_frequency_file );*/
    fp = fopen ( path_to_frequency_file, "r" );
    if (fp)
    {
        fread ( strFrequency, 1, sizeof(strFrequency), fp);
        if (strlen(strFrequency))
        {
            frequency = atoi( strFrequency );
            /*printf("%s: frequency (%u)\n", __FUNCTION__, frequency );*/
        }

        fclose ( fp );
    }
    else
    {
        printf("%s: Unable to find the file (%s).\n", __FUNCTION__, path_to_frequency_file );
        printf("%s: Make sure the kernel version is at least 3.14-1.12 and BOLT version is at least v1.11.\n", __FUNCTION__ );
    }

    return ( frequency );
}

/**
 *  Function: This function will remove the carriage return character at the end of the
 *            provided string ... if one is there.
 **/
static int trim_line ( char * line )
{
    unsigned int len = 0;

    if ( line )
    {
        len = strlen(line);
        if (line[len-1] == '\n')
        {
            line[len-1] = 0;
        }
    }

    return 0;
}

/**
 *  Function: This function will set the global DDR frequency to the value specified.
 *            The user CGI will get the DDR frequency from the bmemperf_server and then
 *            set the internal value to the value returned in the response from
 *            bmemperf_server. Scanning the /sys/devices file system for the DDR frequency
 *            takes about 200 msec. In order to avoid that 200 msec, we get the frequency
 *            from the server, and then set the global value. All other library functions
 *            will subsequently use the global internal value and bypass scanning for the
 *            value in the /sys/devices file system.
 **/
int bmemperf_set_ddrFreq( unsigned long int memc, unsigned long int ddrFreq )
{
    if ( ddrFreq && memc < NEXUS_MAX_MEMC )
    {
        g_bmemperf_ddr_freq_mhz[memc] = ddrFreq;
    }
    return 0;
}
/*
   The cmdline:   find /sys/devices -name "uevent" | grep memory_controllers | grep memc-ddr | xargs grep FULLNAME
   returns something like this:
        /sys/devices/rdb.4/memory_controllers.6/memc.7/f1102000.memc-ddr/uevent:OF_FULLNAME=/rdb/memory_controllers/memc@0/memc-ddr@f1102000
        /sys/devices/rdb.4/memory_controllers.6/memc.8/f1182000.memc-ddr/uevent:OF_FULLNAME=/rdb/memory_controllers/memc@1/memc-ddr@f1182000
        /sys/devices/rdb.4/memory_controllers.6/memc.9/f1202000.memc-ddr/uevent:OF_FULLNAME=/rdb/memory_controllers/memc@2/memc-ddr@f1202000

   The number after the first '@' character gives the MEMC index.
   Replace "uevent" with "frequency" to get the frequency of the DDR for the particular MEMC. The frequency needs to be divided by 1000000 to be
   compatible with the existing code.
*/
int bmemperf_init_ddrFreq( void )
{
    unsigned int memc = 0;
    FILE        *cmd = NULL;
    FILE        *freq = NULL;
    char         line[MAX_LENGTH_DDR_FREQ];

    /* if we have already done the initialization, do not do it again. */
    if ( g_bmemperf_ddr_freq_mhz[0] > 0 )
    {
        return ( 0 );
    }

    sprintf( line, "find /sys/devices -name \"uevent\" | grep memory_controllers | grep memc-ddr | xargs grep FULLNAME" );
    cmd = popen( line, "r" );

    do {
        memset( line, 0, sizeof( line ));
        fgets( line, MAX_LENGTH_DDR_FREQ, cmd );
        trim_line ( line );
        PRINTF("got line (%s)\n", line);
        if (strlen( line ))
        {
            char *pos = strchr( line, '@' );
            if (pos)
            {
                /* printf("found @ at idx %d\n", pos - line );*/
                pos++;
                memc = strtoul( pos, NULL, BMEMPERF_DDR_VALUE_LENGTH );
                /* printf("memc is %d; NEXUS_MAX_MEMC %d\n", memc, NEXUS_MAX_MEMC );*/
                if (memc < NEXUS_MAX_MEMC )
                {
                    /* some platforms do not respond with the full path file name in the string */
                    if (strstr(line, "/uevent"))
                    {
                        /* find the start of the uevent filename so we can use the beginning path to find the frequency filename */
                        pos = strstr( line, "uevent" );
                        if (pos)
                        {
                            /* terminate the beginning of the path name so that we can append the "frequency" filename */
                            *pos = 0;
                        }
                        g_bmemperf_ddr_freq_mhz[memc] = bmemperf_read_ddrFreq( line ) / 1000000;

                        PRINTF("%s:%lu for memc %u: BOLT ddrFreq %u; compile ddrFreq %u\n", __FUNCTION__, __LINE__, memc,
                            g_bmemperf_ddr_freq_mhz[memc], g_bmemperf_info.ddrFreqInMhz );
                    }
                    else /* the line does not have the full path in it ... find the file that we just searched */
                    {
                        sprintf( line, "find /sys/devices -name \"uevent\" | grep memory_controllers | grep memc-ddr" );
                        freq = popen( line, "r" );

                        do {
                            memset( line, 0, sizeof( line ));
                            fgets( line, MAX_LENGTH_DDR_FREQ, freq );
                            if (strlen( line ))
                            {
                                trim_line ( line );

                                PRINTF("got line (%s)\n", line);

                                /* find the start of the uevent filename so we can use the beginning path to find the frequency filename */
                                pos = strstr( line, "uevent" );
                                if (pos)
                                {
                                    /* terminate the beginning of the path name to that we can append the "frequency" filename */
                                    *pos = 0;
                                }
                                g_bmemperf_ddr_freq_mhz[memc] = bmemperf_read_ddrFreq( line ) / 1000000;

                                /* send this value back to the browser */
                                printf( "~DDRFREQ_MEMC~%u,%u~", memc, g_bmemperf_ddr_freq_mhz[memc] );

                                PRINTF("%s:%lu for memc %u: BOLT ddrFreq %u; compile ddrFreq %u\n", __FUNCTION__, __LINE__, memc,
                                        g_bmemperf_ddr_freq_mhz[memc], g_bmemperf_info.ddrFreqInMhz );
                            }
                        } while (strlen( line ));
                    }
                }
            }
        }
    } while (strlen( line ));

    return ( 0 );
}

/**
 *  Function: This function will scan the file /sys/kernel/debug/clk/clk_summary to find the line that has the
 *  sw_scb frequency on it. Once found, the frequency will be scanned to an integer value and set a global
 *  variable with the scanned value.
 **/
int bmemperf_init_scbFreq( void )
{
    static bool  initDone = false;
    FILE        *cmd = NULL;
    char         line[MAX_LENGTH_SCB_FREQ];
    bool         fileFound = false;

    /* if we have already done the initialization, do not do it again. */
    if (initDone == true)
    {
        return ( 0 );
    }

    initDone = true;

    sprintf( line, "cat %s | grep -i sw_scb", SYSFS_SCB_FREQ_FILENAME );
    cmd = popen( line, "r" );

    do {
        memset( line, 0, sizeof( line ));
        fgets( line, MAX_LENGTH_SCB_FREQ, cmd );
        /* the line should look something like this:sw_scb                         0           0            432000000  0 */
        if (strlen( line ))
        {
            line[strlen(line)-1] = 0; /* get rid of the carriage return at the end of the line */
            g_bmemperf_scb_freq_mhz = atoi( &line[57] ) / 1000000;
            /*printf("%s: g_bmemperf_scb_freq_mhz (%u) from line (%s); compile scbFreq (%u) \n", __FUNCTION__, g_bmemperf_scb_freq_mhz, &line[57], g_bmemperf_info.scbFreqInMhz );*/
            /*printf("%s: found SCB frequency of %u MHz in file (%s).\n", __FUNCTION__, g_bmemperf_scb_freq_mhz, SYSFS_SCB_FREQ_FILENAME );*/
            fileFound = true;
            break;
        }
    } while (strlen( line ));

    if ( fileFound == false )
    {
        printf("%s: Unable to find the file (%s).\n", __FUNCTION__, SYSFS_SCB_FREQ_FILENAME );
        printf("%s: Make sure the kernel version is at least 3.14-1.12 and BOLT version is at least v1.11.\n", __FUNCTION__ );
    }

    return ( 0 );
}

/**
 *  Function: This function will return the DDR Frequency that is read from the "frequency" file in the /sys/
 *  file system.
 **/
unsigned int bmemperf_get_ddrFreqInMhz(
    unsigned int compileTimeDefault
    )
{
    unsigned int freq = 0;

    bmemperf_init_ddrFreq();

    /* try to use the frequency read from the /sys/devices file system */
    freq = g_bmemperf_ddr_freq_mhz[0];
    /*printf("~%s: compileTimeDefault %u: BOLT ddrFreq %u \n~", __FUNCTION__, compileTimeDefault, freq );*/

    #if 1
    /* in case the DDR frequency could not be read from the /sys/devices file system, use the one scanned during compile time */
    if ( freq == 0 )
    {
        freq = compileTimeDefault /* g_bmemperf_info.ddrFreqInMhz*/;
        /*printf("%s: compileTimeDefault %u: sysfs value is 0; using compile time ddrFreq %u \n", __FUNCTION__, compileTimeDefault, freq );*/
    }
    #endif

    /*printf("~%s: returning freq %u \n~", __FUNCTION__, freq );*/
    return ( freq );
}

/**
 *  Function: This function will return the SCB Frequency that is read from the clk_summary file in the /sys/
 *  file system.
 **/
unsigned int bmemperf_get_scbFreqInMhz(
    unsigned int compileTimeDefault
    )
{
    unsigned int freq = 0;

    bmemperf_init_scbFreq();

    /* try to use the frequency read from the /sys/kernel/debug/clk/clk_summary file */
    freq = g_bmemperf_scb_freq_mhz;
    /*printf("~%s: compileTimeDefault %u; BOLT scbFreq %u \n~", __FUNCTION__, compileTimeDefault, freq );*/

    /* in case the SCB frequency could not be read from the /sys/kernel/debug/clk/clk_summary file, use the one scanned during compile time */
    if ( freq == 0 )
    {
        freq = compileTimeDefault /*g_bmemperf_info.scbFreqInMhz*/;
        /*printf("%s: compileTimeDefault %u: sysfs value is 0; using compile time scbFreq %u \n", __FUNCTION__, compileTimeDefault, freq );*/
    }

    /*printf("~%s: returning freq %u \n~", __FUNCTION__, freq );*/
    return ( freq );
}

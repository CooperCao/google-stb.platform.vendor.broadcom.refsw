/******************************************************************************
 * (c) 2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/

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
#include <sys/stat.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

#include "bstd.h"
#include "bmemperf_server.h"
#include "bmemperf_cgi.h"
#include "bmemperf_utils.h"
#include "bmemperf_info.h"
#include "nexus_platform.h"
#include "bheaps.h"

#define MAXHOSTNAME       80
#define CONTENT_TYPE_HTML "Content-type: text/html\n\n"

char         *g_client_name[BMEMPERF_MAX_NUM_CLIENT];
int           g_MegaBytes           = 0;                   /* set to 1 when user wants data displayed in megabytes instead of megabits (default) */
int           g_MegaBytesDivisor[2] = {1, 8};
char         *g_MegaBytesStr[2] = {"Mbps", "MBps"};
char         *g_client_name[BMEMPERF_MAX_NUM_CLIENT];
bmemperf_info g_bmemperf_info;
bool          gPerfError = false;

typedef struct
{
    unsigned long int rxErrors;
    unsigned long int txErrors;
    unsigned long int rxBytes;
    unsigned long int txBytes;
    char              name[16];
    char              ipAddress[32];
} bsysperf_netStatistics;

#define NET_STATS_MAX       10
#define BSYSPERF_VALUE_BASE 10
bsysperf_netStatistics g_netStats[NET_STATS_MAX];
int                    g_netStatsIdx = -1;                 /* index to entries added to g_netStats array */

char outString[32];

/**
 *  Function: This function will format an integer to output an indicator for kilo, mega, or giga.
 **/
char *formatul(
    unsigned long int value
    )
{
    float fValue = value;

    if (value < 1024)
    {
        sprintf( outString, "%lu", value );
    }
    else if (value < 1024 * 1024)
    {
        sprintf( outString, "%5.1f K", fValue / 1024 );
    }
    else if (value < 1024 * 1024 * 1024)
    {
        sprintf( outString, "%6.2f M", fValue / 1024 / 1024 );
    }
    else
    {
        sprintf( outString, "%7.3f G", fValue / 1024 / 1024 / 1024 );
    }

    return( outString );
}                                                          /* formatul */

/**
 *  Function: This function will look in the provided string for the specified tag string. If the tag string is
 *  found on the line, the converted integer value will be returned.
 **/
int scan_for_tag(
    const char        *line,
    const char        *tag,
    unsigned long int *value
    )
{
    char             *pos    = NULL;
    unsigned long int lvalue = 0;
    char              valueStr[64];

    pos = strstr( line, tag );
    if (pos)
    {
        pos   += strlen( tag );
        lvalue = strtoul( pos, NULL, BSYSPERF_VALUE_BASE );
        if (strstr( tag, "RX packets:" ) && ( lvalue > 0 ))
        {
            valueStr[0] = 0;
            convert_to_string_with_commas( lvalue, valueStr, sizeof( valueStr ));
            PRINTF( "%s %lu (%s) (%s) (addr -> %p)<br>\n", tag, lvalue, valueStr, pos, (void *)value );
        }

        if (value)
        {
            *value = lvalue;
        }
    }
    return( 0 );
}                                                          /* scan_for_tag */

/**
 *  Function: This function will return the version of the perf utility. This is used to determine if the kernel
 *  has been built with perf support or not.
 **/
int get_perf_version(
    char        *versionString,
    unsigned int versionStringLen
    )
{
    char *pos = NULL;
    FILE *cmd = NULL;
    char  line[MAX_LINE_LENGTH];

    if (versionString == NULL)
    {
        return( -1 );
    }

    sprintf( line, "%s/perf --version", BIN_DIR );
    cmd = popen( line, "r" );

    memset( line, 0, sizeof( line ));
    fgets( line, MAX_LINE_LENGTH, cmd );

    pos = strchr( line, '\n' );
    if (pos != NULL)
    {
        *pos = '\0';
    }
    PRINTF( "~%s: got len %u: line (%s)<br>~", __FUNCTION__, strlen( line ), line );

    pclose( cmd );

    strncpy( versionString, line, versionStringLen );

    return( 0 );
} /* get_perf_version */

/**
 *  Function: This function will output the common UI controls used for the Perf utilities. E.g. perf top, perf
 *  record, and perf stat.
 **/
int output_profiling_controls(
    const char *whichTop /* PerfTop or LinuxTop */
    )
{
    printf( "~%s~", whichTop );
    printf( "<table cols=5 style=\"border-collapse:collapse;\" border=0 cellpadding=3 >" );
    printf( "<tr><th colspan=5 class=whiteborders18 align=left >Profiling</th></tr>" );
    printf( "<tr><th colspan=5 class=whiteborders18 align=left >\n<table border=0 ><tr>" );
    printf( "<td width=100 ><input type=checkbox id=checkboxPerfTop    onclick=\"MyClick(event);\" >Perf&nbsp;Top</td>\n" );
    printf( "<td width=100 ><input type=checkbox id=checkboxLinuxTop   onclick=\"MyClick(event);\" >Linux&nbsp;Top</td>\n" );
    printf( "<td width=150 ><input type=checkbox id=checkboxPerfDeep onclick=\"MyClick(event);\" >Deep "
            "(<input type=input id=PerfDeepDuration style=\"width:2em;\" value=4>sec)</td>\n" );
    printf( "<td width=250 ><table><tr><td><input type=checkbox id=checkboxContextSwitch onclick=\"MyClick(event);\" >Context&nbsp;Switches<span id=spanContextSwitches></span></td></tr></table></td>\n" );
    printf( "</tr></table></th></tr>" );
    printf( "<tr id=row_profiling_html style=\"display:none;\"><th colspan=5 class=whiteborders18 align=left ></th></tr>" );

    return( 0 );
} /* output_profiling_controls */

int output_profiling_error(
    void
    )
{
    bmemperf_version_info versionInfo = {0, 0, 0, "97445", "d0"};

    printf( "<tr><td colspan=5 ><textarea cols=120 rows=24>" );
    printf( "Perf version could not be determined. Make sure you have compiled the kernel with perf enabled!\n\n" );
    printf( "For example:\n    1) cd $MYROOT/kernel-3.14-1.4/rootfs\n" );
    printf( "    2) make vmlinuz-initrd-%s%s-perf\n", &versionInfo.platform[1], versionInfo.platVersion );
    printf( "    3) when building the apps: export LINUX=<directory_path_to_your_Linux_source_tree>\n" );
    printf( "       e.g. export LINUX=$MYROOT/kernel-3.14-1.4/linux\n" );
    printf( "</textarea></td></tr></table>~" );
    return( 0 );
} /* output_profiling_error */

/**
 *  Function: This function will grab the current linux top data
 **/
int GetLinuxTopData(
    void
    )
{
    char *contents = NULL;
    char  tempFilename[TEMP_FILE_FULL_PATH_LEN];

    output_profiling_controls( "LinuxTop" );
    printf( "<tr><td colspan=5 ><textarea cols=120 rows=24 id=textareaTopResults >" );

    PrependTempDirectory( tempFilename, sizeof( tempFilename ), LINUX_TOP_OUTPUT_FILE );
    PRINTF( "~Prepended (%s)~", tempFilename ); fflush( stdout ); fflush( stderr );
    contents = GetFileContents( tempFilename );

    if (contents != NULL)
    {
        printf( "%s", contents );
        free( contents );
    }
    else
    {
        printf( "Could not open file ... %s", LINUX_TOP_OUTPUT_FILE );
    }

    printf( "</textarea></td></tr></table>~" );

    return( 0 );
}                                                          /* GetLinuxTopData */

/**
 *  Function: This function will grab the current perf top data
 **/
#define PERF_TOP_LIMIT 20
int GetPerfTopData(
    void
    )
{
    char        *pos = NULL;
    FILE        *cmd = NULL;
    char         line[MAX_LINE_LENGTH];
    unsigned int lineCount       = 0;
    bool         bBeginningFound = false;

    if (gPerfError == true)
    {
        output_profiling_controls( "PerfTop" );
        output_profiling_error();
        return( -1 );
    }
    /* activate perf with limit to top 20 */
    sprintf( line, "%s/perf top -e cpu-clock -E %u", BIN_DIR, PERF_TOP_LIMIT );
    cmd = popen( line, "r" );

    output_profiling_controls( "PerfTop" );
    printf( "<tr><td colspan=5 ><textarea cols=120 rows=24 id=textareaTopResults >" );

    do {
        memset( line, 0, sizeof( line ));
        fgets( line, MAX_LINE_LENGTH, cmd );
        lineCount++;
        pos = strchr( line, '\r' );
        if (pos != NULL)
        {
            *pos = '\0';
        }
        pos = strchr( line, '\n' );
        if (pos != NULL)
        {
            *pos = '\0';
        }
        PRINTF( "%s: got len %u: cnt %u: line (%s)<br>\n", __FUNCTION__, strlen( line ), lineCount, line );
        if (bBeginningFound == false)
        {
            bBeginningFound = strstr( line, "PerfTop:" );
        }

        if (strlen( line ) && bBeginningFound)
        {
            printf( "%s\n", line );
        }
    }
    while (lineCount < PERF_TOP_LIMIT + 3);
    PRINTF( "\n" );
    printf( "</textarea></td></tr></table>~" );

    pclose( cmd );

    return( 0 );
}                                                          /* GetPerfTopData */

/**
 *  Function: This function will output the checkboxes used in the Memory row.
 **/
int output_memory_controls(
    void
    )
{
    printf( "~MEMORY~" );
    printf( "<table cols=4 style=\"border-collapse:collapse;\" border=0 cellpadding=3 >" );

    printf( "<tr><th colspan=4 class=whiteborders18 align=left >Memory</th></tr>" );
    printf( "<tr>\n" );
    printf( "<td width=150 ><input type=checkbox id=checkboxheapstats onclick=\"MyClick(event);\" >Heap Stats</td>\n" );
    printf( "<td width=150 ><input type=checkbox id=checkboxsatausb   onclick=\"MyClick(event);\" >SATA/USB Stats</td>\n" );
    printf( "<td width=200 ><input type=checkbox id=checkboxPerfCache   onclick=\"MyClick(event);\" >Cache hit/miss "
            "(<input type=input id=PerfCacheDuration style=\"width:2em;\" value=2>sec)</td>\n" );
    printf( "</tr>\n" );

    printf( "<tr id=row_memory_html style=\"visibility:display;\" ><th align=left valign=top id=MEMORY_HTML colspan=4 ></th></tr>\n" );

    printf( "</table>" );                                  /* end MEMORY */

    printf( "~" );

    return( 0 );
}                                                          /* output_memory_controls */

/**
 *  Function: This function will perform a 10-second perf stat analysis
 **/
int get_PerfCache_Results(
    void
    )
{
    char *contents = NULL;
    char  tempFilename[TEMP_FILE_FULL_PATH_LEN];

    printf( "~HEAPTABLE~" );
    printf( "<table id=heapstable border=0 ><tr><td colspan=5 ><textarea cols=120 rows=24 id=textareaPerfCache >" );

    PrependTempDirectory( tempFilename, sizeof( tempFilename ), PERF_STAT_OUTPUT_FILE );
    contents = GetFileContents( tempFilename );

    if (contents != NULL)
    {
        printf( "%s", contents );
        free( contents );
    }
    else
    {
        printf( "Could not open file ... %s", tempFilename );
    }

    printf( "</textarea></td></tr></table>" );
    printf( "~PERFCACHEDONE~" );
    return( 0 );
}                                                          /* get_PerfCache_Results */

/**
 *  Function: This function will enable or disable the specified CPU. Negative values indicate we need to
 *  DISABLE the specified CPU; positive values mean we need to ENABLE the CPU. For example, a -3 means DISABLE
 *  CPU 3; a +3 means ENABLE CPU 3
 **/
void change_cpu_state(
    int new_state
    )
{
    char         line[MAX_LINE_LENGTH];
    unsigned int disableEnable = ( new_state<1 ) ? 0 : 1;

    /* you cannot disable/enable CPU 0 */
    if (new_state == 0)
    {
        return;
    }
    sprintf( line, "echo %u > /sys/devices/system/cpu/cpu%d/online", disableEnable, abs( new_state ));
    printf( "~issuing system(%s)~", line );
    system( line );
}

void get_PerfDeep_Results(
    void
    )
{
    char  line[MAX_LINE_LENGTH];
    char *contents = NULL;
    char  tempFilename[TEMP_FILE_FULL_PATH_LEN];

    output_profiling_controls( "PERFDEEPRESULTS" );
    printf( "<tr><td colspan=5 ><textarea cols=120 rows=24 id=textareaTopResults >" );

    PrependTempDirectory( tempFilename, sizeof( tempFilename ), PERF_REPORT_OUTPUT_FILE );

    sprintf( line, "%s/perf report --sort cpu > %s 2>&1", BIN_DIR, tempFilename );
    system( line );

    contents = GetFileContents( tempFilename );

    if (contents != NULL)
    {
        printf( "%s", contents );
        free( contents );
    }
    else
    {
        printf( "Could not open file ... %s", tempFilename );
    }

    printf( "</textarea></td></tr></table>" );
    printf( "~PERFDEEPRESULTSDONE~" );

    return;
}                                                          /* get_PerfDeep_Results */

/**
 *  Function: This function will gather various network statistics
 **/
int get_netstat_data(
    bsysperf_netStatistics *pNetStats
    )
{
    char *pos = NULL;
    FILE *cmd = NULL;
    char  line[MAX_LINE_LENGTH];

    if (pNetStats == NULL)
    {
        return( -1 );
    }

    /* clear out the array */
    memset( pNetStats, 0, sizeof( *pNetStats ));

    sprintf( line, "%s", IFCONFIG_UTILITY );
    cmd = popen( line, "r" );

    do {
        memset( line, 0, sizeof( line ));
        fgets( line, MAX_LINE_LENGTH, cmd );
        PRINTF( "%s: got len %u: line (%s)<br>\n", __FUNCTION__, strlen( line ), line );
        if (strlen( line ))
        {
            /* if something is in column 1, it must be a name of another interface */
            if (( 'a' <= line[0] ) && ( line[0] <= 'z' ))
            {
                /* if there is room for another interface */
                if (g_netStatsIdx < NET_STATS_MAX)
                {
                    g_netStatsIdx++;

                    /* look for a space; that marks the end of the i/f name */
                    pos = strchr( line, ' ' );
                    if (pos)
                    {
                        *pos = '\0';                       /* null-terminate the i/f name */
                        strncpy( pNetStats[g_netStatsIdx].name, line, sizeof( pNetStats[g_netStatsIdx].name ) - 1 );
                        PRINTF( "g_netStats[%u].name is (%s)<br>\n", g_netStatsIdx, pNetStats[g_netStatsIdx].name );
                    }
                }
                else
                {
                    fprintf( stderr, "%s: not enough room for new interface (%s) in array; max'ed out at %u\n", __FUNCTION__, line, g_netStatsIdx );
                }
            }

            /* if we haven't found the first interface name yet, keep looking for the next line */
            if (g_netStatsIdx >= 0)
            {
                /* if line has IP address on it */
                if (( pos = strstr( line, "inet addr:" )))
                {
                    char *ipAddrStart = NULL;

                    pos += strlen( "inet addr:" );

                    ipAddrStart = pos;

                    /* look for a space; that marks the end of the i/f name */
                    pos = strchr( ipAddrStart, ' ' );
                    if (pos)
                    {
                        *pos = '\0';                       /* null-terminate the IP address */
                        strncpy( pNetStats[g_netStatsIdx].ipAddress, ipAddrStart, sizeof( pNetStats[g_netStatsIdx].ipAddress ) - 1 );
                        PRINTF( "IF (%s) has IP addr (%s)<br>\n", pNetStats[g_netStatsIdx].name, pNetStats[g_netStatsIdx].ipAddress );
                    }
                }
                scan_for_tag( line, "RX packets:", &pNetStats[g_netStatsIdx].rxErrors );
                scan_for_tag( line, "TX packets:", &pNetStats[g_netStatsIdx].txErrors );
                scan_for_tag( line, "RX bytes:", &pNetStats[g_netStatsIdx].rxBytes );
                scan_for_tag( line, "TX bytes:", &pNetStats[g_netStatsIdx].txBytes );
            }
        }
    }
    while (strlen( line ));
    PRINTF( "\n" );

    pclose( cmd );

    return( 0 );
}                                                          /* get_netstat_data */

/**
 *  Function: This function will sort the irq details based on the number of interrupts.
 **/
int sort_on_irq0(
    const void *a,
    const void *b
    )
{
    int rc = 0;

    bmemperf_irq_details *client_a = (bmemperf_irq_details *)a;
    bmemperf_irq_details *client_b = (bmemperf_irq_details *)b;

    rc = client_b->irqCount[0] - client_a->irqCount[0];

    return( rc );
}                                                          /* sort_on_irq0 */

/**
 *  Function: This function is the main entry point for the BSYSPERF client app.
 **/
int main(
    int   argc,
    char *argv[]
    )
{
    int                   idx = 0;
    int                   rc  = 0;
    int                   cpu = 0;
    int                   irq = 0;
    struct sockaddr_in    server;
    struct hostent       *hp = NULL;
    char                  ThisHost[80];
    bmemperf_request      request;
    bmemperf_response     response;
    char                  ServerIpAddr[INET6_ADDRSTRLEN];
    bmemperf_version_info versionInfo;
    char                  valueStr[64];
    int                   numCpusConf =  sysconf( _SC_NPROCESSORS_CONF );
    unsigned long int     irqTotal    = 0;
    unsigned long int     irqTotal2   = 0;
    char                  irqTotalStr[64];

    char *queryString      = NULL;
    char *contentType      = NULL;
    char *contentLength    = NULL;
    char *remoteAddress    = NULL;
    int   epochSeconds     = 0;
    int   tzOffset         = 0;
    int   cpuInfo          = 0;
    int   memory           = 0;
    int   netStats         = 0;
    int   irqInfo          = 0;
    int   heapStats        = 0;
    int   sataUsb          = 0;
    int   profiling        = 0;
    int   PerfTop          = 0;
    int   PerfDeep         = 0;
    int   PerfDeepResults  = 0;
    int   PerfCache        = 0;
    int   PerfCacheResults = 0;
    int   ChangeCpuState   = 0;
    int   LinuxTop         = 0;
    int   ContextSwitches  = 0;
    char  perfVersion[MAX_LINE_LENGTH];

    if (argc > 1) {printf( "%s: no arguments are expected\n", argv[0] ); }

    memset( &versionInfo, 0, sizeof( versionInfo ));
    memset( &response, 0, sizeof( response ));
    memset( &irqTotalStr, 0, sizeof( irqTotalStr ));

    contentType   = getenv( "CONTENT_TYPE" );
    contentLength = getenv( "CONTENT_LENGTH" );
    remoteAddress = getenv( "REMOTE_ADDR" );
    queryString   = getenv( "QUERY_STRING" );

    if (queryString && strlen( queryString ))
    {
        scanForInt( queryString, "datetime=", &epochSeconds );
        scanForInt( queryString, "tzoffset=", &tzOffset );
        scanForInt( queryString, "cpuinfo=", &cpuInfo );
        scanForInt( queryString, "memory=", &memory );
        scanForInt( queryString, "netstats=", &netStats );
        scanForInt( queryString, "irqinfo=", &irqInfo );
        scanForInt( queryString, "heapstats=", &heapStats );
        scanForInt( queryString, "satausb=", &sataUsb );
        scanForInt( queryString, "profiling=", &profiling );
        scanForInt( queryString, "PerfTop=", &PerfTop );
        scanForInt( queryString, "PerfCache=", &PerfCache );
        scanForInt( queryString, "PerfCacheResults=", &PerfCacheResults );
        scanForInt( queryString, "PerfDeep=", &PerfDeep );
        scanForInt( queryString, "PerfDeepResults=", &PerfDeepResults );
        scanForInt( queryString, "ChangeCpuState=", &ChangeCpuState );
        scanForInt( queryString, "LinuxTop=", &LinuxTop );
        scanForInt( queryString, "ContextSwitch=", &ContextSwitches );
    }
    else
    {
        printf( CONTENT_TYPE_HTML );
        printf( "~ERROR: QUERY_STRING is not defined~" );
        return( -1 );
    }

    printf( CONTENT_TYPE_HTML );

    printf( "QUERY_STRING len %u; (%s)", strlen( queryString ), queryString );

    /* if the checkbox for Memory is checked, determine if kernel has been compiled with perf tools */
    /*if (memory || profiling)*/
    {
        get_perf_version( perfVersion, sizeof( perfVersion ));

        printf( "~perfVersion:%s~", perfVersion );
        if (strstr( perfVersion, "perf version " ) == NULL)
        {
            gPerfError = true;
        }
    }

    strncpy( versionInfo.platform, getPlatform(), sizeof( versionInfo.platform ) - 1 );
    strncpy( versionInfo.platVersion, getPlatformVersion(), sizeof( versionInfo.platVersion ) - 1 );
    versionInfo.majorVersion   = MAJOR_VERSION;
    versionInfo.minorVersion   = MINOR_VERSION;
    versionInfo.sizeOfResponse = sizeof( response );
    printf( "~PLATFORM~%s", versionInfo.platform );
    printf( "~PLATVER~%s", versionInfo.platVersion );
    printf( "~VERSION~Ver: %u.%u~", versionInfo.majorVersion, versionInfo.minorVersion );

    if (ChangeCpuState != 0)
    {
        change_cpu_state( ChangeCpuState );
    }
    /* if browser provided a new date/time value; this only happens once during initialization */
    if (epochSeconds)
    {
        int            cpupair      = 0;
        int            leftright    = 0;
        int            leftrightmax = 1;
        struct timeval now          = {1400000000, 0};

        if (numCpusConf % 2 == 0)
        {
            leftrightmax = 2;
        }

        now.tv_sec = epochSeconds - ( tzOffset * 60 );
        settimeofday( &now, NULL );

        usleep( 100 );

        printf( "~CPUPERCENTS~" );
        printf( "<table cols=2 border=0 id=cpugraphs style=\"border-collapse:collapse;\" >\n" );

        for (cpupair = 0; cpupair < numCpusConf; cpupair += 2) {
            int tickidx = 0;
            int penoff  = 0;
            printf( "<tbody>\n" );
            printf( "  <tr id=row%02ua style=\"visibility:hidden;\" >\n", cpupair + 1 );
            for (leftright = 0; leftright < leftrightmax; leftright++) {
                printf( "    <th id=%scol%02ua  align=center valign=bottom >\n", ( leftright == 0 ) ? "left" : "right", cpupair + 1 );
                printf( "      <table cols=3 border=0 style=\"border-collapse:collapse;\" ><tr>\n" );
                printf( "          <th id=cpuoverall align=left  width=230 style=\"color:red;\" >&nbsp;</th>\n" );
                printf( "          <th align=center width=208 ><span id=cputitle%02u >CPU %u</span></th>",
                    cpupair + leftright, cpupair + leftright );
                printf( "          <th width=50 align=center id=ChangeCpuTag%u title=\"Click to disable/enable CPU %u\" >",
                    cpupair + leftright, cpupair + leftright );
                printf( "&nbsp;" );
                /*
                 Javascript will insert something similar to this based on which CPU comes back enabled and which is disabled
                    <svg height=20 width=20 ><circle cx=10 cy=10 r=10 onclick="MyClick(event);" id=ChangeCpuState3 fill=lime /></svg>
                 */
                printf( "      </tr></table>\n" );
                printf( "    </th>\n" );
            }
            printf( "  </tr>\n" );
            printf( "  <tr id=row%02ub style=\"visibility:hidden;\" >\n", cpupair + 1 );
            for (leftright = 0; leftright < leftrightmax; leftright++) {
                printf( "    <td id=%scol%02ub  >\n", ( leftright == 0 ) ? "left" : "right", cpupair + 1 );
                printf( "      <svg id=svg%02u height=\"100\" width=\"500\" style=\"border:solid 1px black;font-size:8pt;\" >"
                        "<polyline id=polyline%02u style=\"fill:none;stroke:blue;stroke-width:2\" />\n", cpupair + 1, cpupair + 1 + leftright );
                /* for the very first one, create a polyline to be used for the average cpu utilization ... polyline00 */
                if (( cpupair == 0 ) && ( leftright == 0 ))
                {
                    printf( "<polyline id=polyline00 style=\"fill:none;stroke:red;stroke-width:2\" />\n" );
                    /* add a polyline for the limegreen 5-second CPU utilization average */
                    printf( "<polyline id=polyline0%u style=\"fill:none;stroke:limegreen;stroke-width:2\" />\n", numCpusConf + 1 );
                }
                /* make a vertical tick mark every 10% */
                for (tickidx = 1; tickidx<10; tickidx++)
                {
                    penoff = ( 1-tickidx%2 ) * 5;          /* dashed line for 20,40,60,80; solid line for 10,30,50,70,90 */
#if 0
                    printf( "<line x1=0 y1=%d x2=500 y2=%d style=\"stroke:lightgray;stroke-width:1;stroke-dasharray=5,5; \" />", tickidx*10, tickidx*10 );
#else
                    /* for dasharray: how many pixels will the pen be on ... how many pixels will the pen be off. 5 on 5 off is a dash; 5 on 0 off is solid */
                    printf( "<path d=\"M0 %d L500 %d\" stroke=lightgray stroke-width=1 stroke-dasharray=\"5,%d\" />", tickidx*10, tickidx*10, penoff );
#endif
                    if (( tickidx%2 ) == 1)                                                           /* output text for 10, 30, 50, 70, 90 */
                    {
                        printf( "<text x=2 y=%d>%d</text>\n", tickidx*10+3, ( 100 - ( tickidx*10 ))); /* offset 3 pixels to drop the number into the middle of the tickmark */
                    }
                }
                printf( "</svg>\n" );
                printf( "      </td>\n" );
            }
            printf( "  </tr>\n" );
            printf( "  <tr id=row%02uc style=\"visibility:hidden;\" >\n", cpupair + 1 );
            for (leftright = 0; leftright < leftrightmax; leftright++) {
                printf( "    <td id=%scol%02uc align=left valign=bottom ><textarea rows=1 id=cpudata%02u style=\"border:solid 1px black;width:100%%;\" >&nbsp;</textarea></td>\n",
                    ( leftright == 0 ) ? "left" : "right", cpupair + 1, cpupair + leftright );
            }
            printf( "  </tr>\n" );
            printf( "</tbody>\n" );
        }
        printf( "</table>\n" );
    }

    printf( "~STBTIME~%s~", DayMonDateYear( 0 ));

    /* if the checkbox for CPU Utilization OR Network Stats OR IRQ Counts is checked (any one of these needs to request data from bmemperf_server) */
    if (cpuInfo || netStats || irqInfo || PerfDeep || PerfCache || sataUsb || LinuxTop || ContextSwitches)
    {
        strncpy( ThisHost, "localhost", sizeof( ThisHost ));
        getservbyname( "echo", "tcp" );

        strncpy( ThisHost, "localhost", sizeof( ThisHost ));

        if (( hp = gethostbyname2( ThisHost, AF_INET )) == NULL)
        {
            fprintf( stderr, "Can't find host %s\n", "localhost" );
            exit( -1 );
        }
        bcopy( hp->h_addr, &( server.sin_addr ), hp->h_length );
        printf( "~TCP/Client running at HOST (%s) at INET ADDRESS : (%s)~", ThisHost, inet_ntoa( server.sin_addr ));

        rc = gethostbyaddr2( ThisHost, BSYSPERF_SERVER_PORT, ServerIpAddr, sizeof( ServerIpAddr ));
        if (rc != 0)
        {
            perror( "ERROR getting address for port" );
            return( -1 );
        }

        inet_pton( AF_INET, ServerIpAddr, &server.sin_addr );

        /* Construct name of socket to send to. */
        server.sin_family = AF_INET;
        server.sin_port   = htons( BSYSPERF_SERVER_PORT );

        if (PerfDeep || PerfCache)
        {
            if (gPerfError == true)
            {
                output_profiling_controls( "PerfTop" );
                output_profiling_error();
            }
            else
            {
                if (PerfDeep)
                {
                    PRINTF( "~Sending BMEMPERF_CMD_START_PERF_DEEP request~" );
                    request.cmdSecondary       = BMEMPERF_CMD_START_PERF_DEEP;
                    request.cmdSecondaryOption = PerfDeep; /* duration of report */
                }
                else if (PerfCache)
                {
                    PRINTF( "~Sending BMEMPERF_CMD_START_PERF_CACHE request~" );
                    request.cmdSecondary       = BMEMPERF_CMD_START_PERF_CACHE;
                    request.cmdSecondaryOption = PerfCache; /* duration of report */
                }
            }
        }

        if (sataUsb)
        {
            if (sataUsb == 1)                              /* user requested we start data collection */
            {
                PRINTF( "~Sending BMEMPERF_CMD_START_SATA_USB request~" );
                request.cmdSecondary = BMEMPERF_CMD_START_SATA_USB;
            }
            else                                           /* user requested we stop data collection */
            {
                PRINTF( "~Sending BMEMPERF_CMD_STOP_SATA_USB request~" );
                request.cmdSecondary = BMEMPERF_CMD_STOP_SATA_USB;
            }
            request.cmdSecondaryOption = request.cmdSecondary;
        }

        if (LinuxTop)                                      /* could be 1 (start) or 2 (stop) */
        {
            if (LinuxTop == 1)                             /* user requested we start data collection */
            {
                PRINTF( "~Sending BMEMPERF_CMD_START_LINUX_TOP request~" );
                request.cmdSecondary = BMEMPERF_CMD_START_LINUX_TOP;
            }
            else                                           /* user requested we stop data collection */
            {
                PRINTF( "~Sending BMEMPERF_CMD_STOP_LINUX_TOP request~" );
                request.cmdSecondary = BMEMPERF_CMD_STOP_LINUX_TOP;
            }
            request.cmdSecondaryOption = request.cmdSecondary;
        }

        rc = send_request_read_response( &server, (unsigned char*) &request, sizeof(request), (unsigned char*) &response, sizeof(response), BSYSPERF_SERVER_PORT, BMEMPERF_CMD_GET_CPU_IRQ_INFO );
        if (rc < 0)
        {
            printf( "error sending BMEMPERF_CMD_GET_CPU_IRQ_INFO request; rc %d \n", rc );
            printf( "~FATAL~ERROR connecting to server. Make sure bsysperf_server is running.~" );
            return( -1 );
        }
        PRINTF( "Received from server: cmd (%d)\n", response.cmd );
        irqTotal2 = response.response.cpuIrqData.irqData.irqTotal;

        if (PerfDeep)
        {
            printf( "~PERFDEEPSTARTED~SUCCESS~" );
        }
        else if (PerfCache)
        {
            printf( "~PERFCACHESTARTED~SUCCESS~" );
        }
    }

    /* if the checkbox for CPU Utilization is checked */
    if (cpuInfo)
    {
        /* output the CPU utilization data */
        printf( "~CPUINFO~%u ", numCpusConf );
        for (cpu = 0; cpu < numCpusConf; cpu++) {
            /* if the CPU is not active */
            if (response.response.cpuIrqData.cpuData.idlePercentage[cpu] == 255)
            {
                printf( "%06u ", response.response.cpuIrqData.cpuData.idlePercentage[cpu] );
            }
            else
            {
                printf( "%06u ", 100 - response.response.cpuIrqData.cpuData.idlePercentage[cpu] );
            }
        }
        printf( "~" );
    }                                                      /* cpuInfo */

    /* if the checkbox for IRQ Stats is checked */
    if (irqInfo)
    {
        /* output the IRQ data */
        printf( "~IRQINFO~%u ", response.response.cpuIrqData.cpuData.numActiveCpus );
        for (cpu = 0; cpu < response.response.cpuIrqData.cpuData.numActiveCpus; cpu++) {
            printf( "%06lu ", response.response.cpuIrqData.irqData.irqCount[cpu] );
            irqTotal += response.response.cpuIrqData.irqData.irqCount[cpu];
        }
        printf( "~" );

        qsort( &response.response.cpuIrqData.irqData.irqDetails[0], BMEMPERF_IRQ_MAX_TYPES,
            sizeof( response.response.cpuIrqData.irqData.irqDetails[0] ), sort_on_irq0 );

        printf( "irqTotal (%lu); irqTotal2 (%lu)\n", irqTotal, irqTotal2 );
        /* convert 999999 to 999,999 ... e.g. add commas */
        convert_to_string_with_commas( irqTotal,  irqTotalStr, sizeof( irqTotalStr ));

        printf( "~IRQDETAILS~" );
        printf( "<table cols=%u style=\"border-collapse:collapse;\" border=1 cellpadding=3 >", response.response.cpuIrqData.cpuData.numActiveCpus + 1 );

        /* the first pass through, output the table description and table header row */
        printf( "<tr><th colspan=%u class=whiteborders18 align=left >%s</th></tr>", response.response.cpuIrqData.cpuData.numActiveCpus + 1,
            "Interrupt Counts Since Bootup" );
        printf( "<tr><th class=allborders50 style=\"background-color:lightgray;font-size:12pt;\" width=50 >%s</th><th class=whiteborders18 colspan=%u "
                "style=\"border-left:solid thin black;\" ></th></tr>", irqTotalStr, response.response.cpuIrqData.cpuData.numActiveCpus );

        printf( "<tr bgcolor=lightgray >" );

        /* output the header line for the cpus */
        for (cpu = 0; cpu < numCpusConf; cpu++) {
            /* if the cpu is not inactive */
            if (response.response.cpuIrqData.cpuData.idlePercentage[cpu] != 255)
            {
                printf( "<td width=130 >CPU %u (delta)</td>", cpu );
            }
        }
        printf( "<td>IRQ Description</td></tr>\n" );
        for (irq = 0; irq < BMEMPERF_IRQ_MAX_TYPES; irq++) {
            /* if the entry has a valid IRQ name */
            if (strlen( response.response.cpuIrqData.irqData.irqDetails[irq].irqName ))
            {
                unsigned long int summation = 0;
                /* determine if something on the row is non-zero */
                for (cpu = 0; cpu < response.response.cpuIrqData.cpuData.numActiveCpus; cpu++) {
                    summation += response.response.cpuIrqData.irqData.irqDetails[irq].irqCount[cpu];
                }

                /* if at least one of the numbers on the row is non-zero, then display the row */
                if (summation)
                {
                    printf( "<tr>" );
                    for (cpu = 0; cpu < response.response.cpuIrqData.cpuData.numActiveCpus; cpu++) {
                        valueStr[0] = 0;
                        convert_to_string_with_commas( response.response.cpuIrqData.irqData.irqDetails[irq].irqCount[cpu], valueStr, sizeof( valueStr ));
                        printf( "<td>%s", formatul( response.response.cpuIrqData.irqData.irqDetails[irq].irqCount[cpu] ));
                        /* if there is a non-zero delta from the previous pass, display the delta in parens; otherwise no need to display a zero in parens */
                        if (( response.response.cpuIrqData.irqData.irqDetails[irq].irqCount[cpu] - response.response.cpuIrqData.irqData.irqDetails[irq].irqCountPrev[cpu] ) > 0)
                        {
                            valueStr[0] = 0;
                            convert_to_string_with_commas(
                                response.response.cpuIrqData.irqData.irqDetails[irq].irqCount[cpu] - response.response.cpuIrqData.irqData.irqDetails[irq].irqCountPrev[cpu],
                                valueStr, sizeof( valueStr ));
                            printf( " (%s)", valueStr );
                        }
                        printf( "</td>" );
                    }
                    printf( "<td>%s</td></tr>", response.response.cpuIrqData.irqData.irqDetails[irq].irqName );
                }
                else
                {
                    /*printf("<tr><td colspan=5> all numbers are zero</td></tr>\n");*/
                }
            }
        }

        printf( "</table>~" );                             /* end IRQDETAILS */
    }

    /* if the checkbox for Network Stats is checked */
    if (netStats)
    {
        printf( "~NETSTATS~" );
        printf( "<table cols=7 style=\"border-collapse:collapse;\" border=0 cellpadding=3 >" );

        get_netstat_data( &g_netStats[0] );

        printf( "<tr><th colspan=7 class=whiteborders18 align=left >Network Interface Statistics</th></tr>" );
        printf( "<tr><td colspan=7><table style=\"border-collapse:collapse;\" border=1 cellpadding=3 >\n" );
        printf( "<tr bgcolor=lightgray ><th>Name</th><th>IP Addr</th><th>Rx Bytes</th><th>Tx Bytes</th><th>Rx Errors</th><th>Tx Errors</th>"
                "<th>Rx Mbps (Avg)</th><th>Tx Mbps (Avg)</th></tr>\n" );
        for (idx = 0; idx <= g_netStatsIdx; idx++) {
            printf( "<tr><td>%s</td> <td>%s</td> <td>%s</td>", g_netStats[idx].name, g_netStats[idx].ipAddress, formatul( g_netStats[idx].rxBytes ));
            printf( "<td>%s</td>", formatul( g_netStats[idx].txBytes ));
            printf( "<td>%s</td>", formatul( g_netStats[idx].rxErrors ));
            printf( "<td>%s</td>", formatul( g_netStats[idx].txErrors ));
            printf( "<td id=netif%urx align=center ><!-- value inserted via javascript --></td>", idx );
            printf( "<td id=netif%utx align=center ><!-- value inserted via javascript --></td></tr>\n", idx );
        }
        printf( "</table></td></tr>\n" );

        printf( "</table>~" );                             /* end NETSTATS */

        /* output some of the above information again in an unformatted way to make it easier to compute bits per second */
        printf( "~NETBYTES~" );
        for (idx = 0; idx <= g_netStatsIdx; idx++) {
            printf( "%lu ", g_netStats[idx].rxBytes );
            printf( "%lu,", g_netStats[idx].txBytes );
        }
        printf( "~" );
    }

    /* if the memory row is displaying, output the controls */
    if (memory)
    {
        output_memory_controls();
    }

    /* if the checkbox for Memory is checked */
    if (heapStats)
    {
        /* if heap statistics has been requested via the checkbox on the html page */
        if (heapStats)
        {
            int          rc      = 0;
            unsigned int heapIdx = 0;
            bheap_info   heap_info[BHEAP_MAX_HEAP_NUM];

            memset( &heap_info, 0, sizeof( heap_info ));

            rc = get_heap_info( &heap_info[0] );

            if (rc == 0)
            {
                printf( "~HEAPTABLE~" );
                printf( "<table id=heapstable border=0 >" );
                printf( "  <tr id=heaprow1222 ><th align=center ><h2>Heap Information</h2></th></tr>" );
                printf( "  <tr id=heaprow2222 ><td><textarea cols=95 rows=11 id=heapinfo ></textarea></td></tr>" );
                printf( "  <tr id=heaprow3222 ><td id=heapgraph >&nbsp;</td></tr>" );
                printf( "</table>" );
                printf( "~HEAPINFO~" );
                for (heapIdx = 0; heapIdx < BHEAP_MAX_HEAP_NUM; heapIdx++) {
                    if (heap_info[heapIdx].offset > 0)
                    {
                        /* the first time through, output header */
                        if (heapIdx == 0)
                        {
                            printf( "IDX OFFSET     MEMC SIZE        MB VADDR       USED PEAK MAPPING\n" );
                        }

                        printf( "%u   0x%09lx  %u  0x%-8lx %3lu 0x%08lx  %3u%% %3u%% %s\n",  heapIdx, heap_info[heapIdx].offset, heap_info[heapIdx].memc, heap_info[heapIdx].size,
                            heap_info[heapIdx].megabytes, heap_info[heapIdx].vaddr, heap_info[heapIdx].usedPercentage, heap_info[heapIdx].peakPercentage, heap_info[heapIdx].mapping );
                    }
                }

                printf( "~HEAPGRAPH~" );
                show_heaps( &heap_info[0] );
            }
            else
            {
                /* we need to send something back to browser to force it to stop asking for a valid heap status */
                printf( "~HEAPINFO~Could not get_heap_info()~" );
            }
        }
    }

    /* if the checkbox for SATA/USB is checked */
    if (sataUsb)
    {
        unsigned int idx = 0;
        char         debugBuffer[1024];
        memset( debugBuffer, 0, sizeof( debugBuffer ));

        printf( "~SATAUSB~" );
        printf( "<table id=satauabtable cols=3 border=0 style=\"border-collapse:collapse;\" cellpadding=5 >" );
        printf( "  <tr id=satausbrow1222 ><th align=left colspan=3 style=\"font-size:18pt;\" ><b>SATA/USB Information</b></th></tr>" );
        printf( "<tr><th class=allborders50 style=\"background-color:lightgray;font-size:12pt;\" >Device</th>"
                "<th class=allborders50 style=\"background-color:lightgray;font-size:12pt;\" >Read&nbsp;Mbps</th>"
                "<th class=allborders50 style=\"background-color:lightgray;font-size:12pt;\" >Write&nbsp;Mbps</th>"
                "</tr>" );
        for (idx = 0; idx<sizeof( response.response.cpuIrqData.sataUsbData )/sizeof( response.response.cpuIrqData.sataUsbData[0] ); idx++)
        {
            if (response.response.cpuIrqData.sataUsbData[idx].deviceName[0] != '\0')
            {
                printf( " <tr id=satausbrow2222 ><td class=allborders50 >%s</td><td class=allborders50 align=center >",
                    response.response.cpuIrqData.sataUsbData[idx].deviceName );
                if (response.response.cpuIrqData.sataUsbData[idx].readMbps > 0.0)
                {
                    printf( "%6.1f", response.response.cpuIrqData.sataUsbData[idx].readMbps );
                }
                printf( "</td><td class=allborders50 align=center >" );
                if (response.response.cpuIrqData.sataUsbData[idx].writeMbps > 0.0)
                {
                    printf( "%6.1f", response.response.cpuIrqData.sataUsbData[idx].writeMbps );
                }
                printf( "</td></tr>" );
                {
                    char line[64];
                    sprintf( line, "~SATADEBUG~%s rd %6.1f wr %6.1f~", response.response.cpuIrqData.sataUsbData[idx].deviceName,
                        response.response.cpuIrqData.sataUsbData[idx].readMbps,
                        response.response.cpuIrqData.sataUsbData[idx].writeMbps  );
                    strncat( debugBuffer, line, sizeof( debugBuffer ) - strlen( debugBuffer ) - 1 );
                }
            }
        }
        printf( "</table>" );
        printf( "%s", debugBuffer );
    }

    /* if perf top has been requested via the checkbox on the html page */
    if (PerfTop || profiling)
    {
        GetPerfTopData();
    }

    /* if linux top has been requested via the checkbox on the html page */
    if (LinuxTop)
    {
        GetLinuxTopData();
    }

    if (PerfCacheResults)
    {
        get_PerfCache_Results();
    }

    if (PerfDeepResults)
    {
        get_PerfDeep_Results();
    }

    /* we need to send this last to allow the previous functions to send back the HTML that might get disabled if gPerfError is true */
    if (gPerfError == true)
    {
        printf( "~PERFERROR~" );
    }
    else
    {
        printf( "~PERFENABLED~" );
    }

    /* if the checkbox for context switch per second is checked */
    if (ContextSwitches)
    {
        printf( "~CONTEXTSWITCH~" );
        #if 0
        printf( "<tr><td>" );
        printf( "<table id=contextswitchtable cols=2 border=0 style=\"border-collapse:collapse;\" cellpadding=5 >" );
        printf( "<th class=allborders50 style=\"background-color:lightgray;font-size:12pt;\" width=200 >Context&nbsp;Switches</th>" );
        printf( "<td class=allborders50 style=\"background-color:white;font-size:12pt;\" width=100 >%lu</td>", response.response.overallStats.contextSwitches );
        printf( "</tr>" );
        printf( "</table>" );
        printf( "</td></tr>" );
        #else
        printf( "%lu", response.response.overallStats.contextSwitches );
        #endif
        printf( "~" );
    }

    printf( "~ALLDONE~" );

    return( 0 );
}                                                          /* main */

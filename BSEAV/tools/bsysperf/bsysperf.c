/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
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
******************************************************************************/
#include "bmemperf_types64.h"
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
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/utsname.h>

#include "bstd.h"
#include "bmemperf_server.h"
#include "bmemperf_cgi.h"
#include "bmemperf_utils.h"
#include "bmemperf_info.h"
#include "nexus_platform.h"
#include "bheaps.h"
#ifdef BWL_SUPPORT
#include "bsysperf_wifi.h"
#include "bwl.h"
#endif

#define MAXHOSTNAME       80
#define CONTENT_TYPE_HTML "Content-type: text/html\n\n"
#define MYUUID_LEN 16

char         *g_client_name[BMEMPERF_MAX_NUM_CLIENT];
int           g_MegaBytes           = 0;                   /* set to 1 when user wants data displayed in megabytes instead of megabits (default) */
int           g_MegaBytesDivisor[2] = {1, 8};
char         *g_MegaBytesStr[2] = {"Mbps", "MBps"};
char         *g_client_name[BMEMPERF_MAX_NUM_CLIENT];
bmemperf_info g_bmemperf_info;
bool          gPerfError = false;

typedef struct
{
    unsigned long long int rxErrors;
    unsigned long long int txErrors;
    unsigned long long int rxBytes;
    unsigned long long int txBytes;
    char                   name[16];
    char                   ipAddress[32];
} bsysperf_netStatistics;

#define NET_STATS_MAX       10
#define BSYSPERF_VALUE_BASE 10
bsysperf_netStatistics g_netStats[NET_STATS_MAX];
int                    g_netStatsIdx = -1;                 /* index to entries added to g_netStats array */

char outString[32];

typedef enum {
    PERF_FLAME_UNINITED=0,
    PERF_FLAME_INIT,
    PERF_FLAME_IDLE,
    PERF_FLAME_START,
    PERF_FLAME_RECORDING,
    PERF_FLAME_STOP,
    PERF_FLAME_CREATE_SCRIPT_OUT,
    PERF_FLAME_GET_SVG,
    PERF_FLAME_DELETE_OUT_FILE,
    PERF_FLAME_MAX
} Bsysperf_PerfFlame_State;

/**
 *  Function: This function will format an integer to output an indicator for kilo, mega, or giga.
 **/
char *formatul(
    unsigned long long int value
    )
{
    float fValue = value;

    if (value < 1024)
    {
        sprintf( outString, "%llu", value );
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
    const char             *line,
    const char             *tag,
    unsigned long long int *value
    )
{
    int                     rc     = -1;
    char                   *pos    = NULL;
    unsigned long long int  lvalue = 0;
    char                    valueStr[64];

    pos = strstr( line, tag );
    if (pos)
    {
        pos   += strlen( tag );
        lvalue = strtoull( pos, NULL, BSYSPERF_VALUE_BASE );
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

        rc = 0;
    }
    /* -1 is used to indicate the tag was not found; rc=0 indicates the tag was found and some values were scanned */
    return( rc );
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
    printf( "<tr><td colspan=5 class=whiteborders18 align=left ><table border=0 ><tr>" );
    printf( "<td width=100 ><table><tr><td><input type=checkbox id=checkboxPerfTop    onclick=\"MyClick(event);\" ></td><td style=\"font-size:12.0pt;font-weight:normal;\" >Perf&nbsp;Top</td></tr></table></td>\n" );
    printf( "<td width=100 ><table><tr><td><input type=checkbox id=checkboxLinuxTop   onclick=\"MyClick(event);\" ></td><td style=\"font-size:12.0pt;font-weight:normal;\" >Linux&nbsp;Top</td></tr></table></td>\n" );
    printf( "<td width=150 ><table><tr><td><input type=checkbox id=checkboxPerfDeep onclick=\"MyClick(event);\" ></td><td style=\"font-size:12.0pt;font-weight:normal;\" >Deep</td>"
            "<td style=\"font-size:12.0pt;font-weight:normal;\" >(<input type=input id=PerfDeepDuration style=\"width:2em;\" value=4>sec)</td></tr></table></td>\n" );
    printf( "<td width=250 ><table><tr><td><input type=checkbox id=checkboxContextSwitch onclick=\"MyClick(event);\" ></td>"
            "<td style=\"font-size:12.0pt;font-weight:normal;\" >Context&nbsp;Switches<span id=spanContextSwitches></span></td></tr></table></td>\n" );
    printf( "</tr></table></td></tr>" );
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
    PRINTF( "~Prepended (%s)~", tempFilename );
    contents = GetFileContents( tempFilename );

    if (contents != NULL)
    {
        printf( "%s", contents );
        Bsysperf_Free( contents );
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
        Bsysperf_Free( contents );
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
        Bsysperf_Free( contents );
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
                    printf( "%s: not enough room for new interface (%s) in array; max'ed out at %u\n", __FUNCTION__, line, g_netStatsIdx );
                }
            }

            /* if we haven't found the first interface name yet, keep looking for the next line */
            if (g_netStatsIdx >= 0)
            {
                int rc = 0;

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
                /*
                eth0      Link encap:Ethernet  HWaddr 00:10:18:D2:C3:C9
                          inet addr:10.14.244.188  Bcast:10.14.245.255  Mask:255.255.254.0
                          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
                          RX packets:11832 errors:0 dropped:0 overruns:0 frame:0
                          TX packets:2150 errors:0 dropped:0 overruns:0 carrier:0
                          collisions:0 txqueuelen:1000
                          RX bytes:2429458 (2.3 MiB)  TX bytes:632148 (617.3 KiB)
                */

                rc = scan_for_tag( line, "RX packets:", &pNetStats[g_netStatsIdx].rxErrors );
                /* if the RX packets tag was found, now scan for errors tag on the same line */
                if (rc == 0)
                {
                    rc = scan_for_tag( line, " errors:", &pNetStats[g_netStatsIdx].rxErrors );
                }

                rc = scan_for_tag( line, "TX packets:", &pNetStats[g_netStatsIdx].txErrors );
                /* if the TX packets tag was found, now scan for errors tag on the same line */
                if (rc == 0)
                {
                    rc = scan_for_tag( line, " errors:", &pNetStats[g_netStatsIdx].txErrors );
                }
                scan_for_tag( line, "RX bytes:", &pNetStats[g_netStatsIdx].rxBytes );
                scan_for_tag( line, "TX bytes:", &pNetStats[g_netStatsIdx].txBytes );
                /*printf("~DEBUG~ip_addr (%s) ... TX bytes (%lld)~", pNetStats[g_netStatsIdx].ipAddress, pNetStats[g_netStatsIdx].txBytes );*/
            }
        }
    } while (strlen( line ));
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

int create_uuid( char * strUuid )
{
    unsigned int  idx;
    unsigned long int myUuid[4];
    unsigned char     *uuid = (unsigned char *)myUuid;
    unsigned long int temp = 0;
    struct timespec time1;
    char   two_digits[3];

    memset ( myUuid, 0, MYUUID_LEN );
    memset ( &time1, 0, sizeof(time1) );

    clock_gettime( CLOCK_REALTIME, &time1 );

    srandom ( (unsigned int) time1.tv_nsec/1000 );

    /*printf("~DEBUG~uuid %p~\n", uuid );*/
    for (idx=0; idx<4; idx++)
    {
        myUuid[idx] = temp = random();
        /*printf("~DEBUG~random(%u) returned %08lx; uuid %p~\n", idx, temp, myUuid[idx] );*/
    }

    /*printf("~DEBUG~%s: myUuid: ", __FUNCTION__ );*/
    for (idx=0; idx<MYUUID_LEN; idx++)
    {
        /*printf( "%02x ", uuid[idx] );*/
        snprintf( two_digits, sizeof(two_digits), "%02x", uuid[idx] );
        strcat( strUuid, two_digits);
    }
    /*printf("~\n");*/
    /*printf("strUuid:(%s)\n", strUuid );*/

    return 0;
}

/**
 *  Function: This function will attempt to find the IP address of the wlan0 interface.
 **/
static char *Bsysperf_FindWlan0Address( void )
{
     int idx = 0;
     static char address[32] = "192.168.0.100";

     for (idx=0; idx<=g_netStatsIdx; idx++)
     {
         if ( strcmp(g_netStats[idx].name, "wlan0") == 0 )
         {
             strncpy( address, g_netStats[idx].ipAddress, sizeof(address) -1 );
             break;
         }
     }
     return ( address );
}

static int outputDashTickLines( const char *svg_text_id )
{
    int            tickidx      = 0;
    int            penoff       = 0;
    char           l_text_id[32];

    /* make a vertical tick mark every 10% */
    for (tickidx = 1; tickidx<10; tickidx++)
    {
        penoff = ( 1-tickidx%2 ) * 5;          /* dashed line for 20,40,60,80; solid line for 10,30,50,70,90 */
        /* for dasharray: how many pixels will the pen be on ... how many pixels will the pen be off. 5 on 5 off is a dash; 5 on 0 off is solid */
        printf( "<path d=\"M0 %d L500 %d\" stroke=lightgray stroke-width=1 stroke-dasharray=\"5,%d\" />", tickidx*10, tickidx*10, penoff );
        if (( tickidx%2 ) == 1)                                                           /* output text for 10, 30, 50, 70, 90 */
        {
            l_text_id[0] = '\0';
            if ( svg_text_id ) sprintf( l_text_id, "id=%s_%u", svg_text_id, tickidx/2 ); /* if user provided a string id, append idx to end of string and use it as text id */
            printf( "<text x=2 y=%d %s >%d</text>\n", tickidx*10+3,  /* offset 3 pixels to drop the number into the middle of the tickmark */
                    l_text_id, ( 100 - ( tickidx*10 )));
        }
    }

    return ( 0 );
}

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
    char                  ThisHost[80];
    bmemperf_request      request;
    bmemperf_response     response;
    bmemperf_version_info versionInfo;
    char                  valueStr[64];
    int                   numCpusConf =  sysconf( _SC_NPROCESSORS_CONF );
    unsigned long int     irqTotal    = 0;
    unsigned long int     irqTotal2   = 0;
    char                  irqTotalStr[64];

    char *queryString      = NULL;
    int   epochSeconds     = 0;
    int   tzOffset         = 0;
    int   cpuInfo          = 0;
    int   memory           = 0;
    int   netStatsInit     = 0; // when true, send to browser the Net Stats html table structure
    int   netStatsUpdate   = 0; // when true, send network interface values from last second; browser will populate the html table structure
    int   iperfInit        = 0;
    int   iperfPidClient   = 0;
    int   iperfPidServer   = 0;
    int   iperfRunningClient= 0;
    int   iperfRunningServer= 0;
    int   wifiInit         = 0; /* set to true when checkbox is checked; get versions and ids that do not change */
    int   wifiStats        = 0;
    int   wifiScanStart    = 0;
    int   wifiAmpduStart   = 0;
    int   wifiAmpduGraph   = 0;
    int   wifiScanResults  = -1;
    int   irqInfo          = 0;
    int   heapStats        = 0;
    int   sataUsb          = 0;
    int   profiling        = 0;
    int   PerfTop          = 0;
    int   PerfDeep         = 0;
    int   PerfDeepResults  = 0;
    int   PerfCache        = 0;
    int   PerfCacheResults = 0;
    int   PerfFlame        = 0;
    char  PerfFlameCmdLine[128];
    int   PerfFlameSvgCount= 1;
    char  strUuid[MYUUID_LEN*2+1];
    int   ChangeCpuState   = 0;
    int   LinuxTop         = 0;
    int   ContextSwitches  = 0;
    char  perfVersion[MAX_LINE_LENGTH];
#ifdef BWL_SUPPORT
    AMPDU_DATA_T lAmpduData;
#endif
    char   iperfCmdLine[sizeof(request.request.strCmdLine)];
    struct utsname uname_info;
    int    DvfsControl     = 0;
    int    GovernorSetting = 0;

    if (argc > 1) {printf( "%s: no arguments are expected\n", argv[0] ); }

    memset( &versionInfo, 0, sizeof( versionInfo ));
    memset( &response, 0, sizeof( response ));
    memset( &irqTotalStr, 0, sizeof( irqTotalStr ));
    memset( &PerfFlameCmdLine, 0, sizeof( PerfFlameCmdLine ));
    memset( strUuid, 0, sizeof(strUuid) );
    memset( iperfCmdLine, 0, sizeof(iperfCmdLine) );
    memset( &uname_info, 0, sizeof(uname_info));

    queryString   = getenv( "QUERY_STRING" );

    if (queryString && strlen( queryString ))
    {
        scanForInt( queryString, "datetime=", &epochSeconds );
        scanForInt( queryString, "tzoffset=", &tzOffset );
        scanForInt( queryString, "cpuinfo=", &cpuInfo );
        scanForInt( queryString, "memory=", &memory );
        scanForInt( queryString, "netStatsInit=", &netStatsInit );
        scanForInt( queryString, "netStatsUpdate=", &netStatsUpdate );
        scanForInt( queryString, "iperfInit=", &iperfInit );
        scanForInt( queryString, "iperfPidClient=", &iperfPidClient);
        scanForInt( queryString, "iperfPidServer=", &iperfPidServer);
        scanForInt( queryString, "iperfRunningClient=", &iperfRunningClient);
        scanForInt( queryString, "iperfRunningServer=", &iperfRunningServer);
        scanForStr( queryString, "iperfCmdLine=", sizeof( iperfCmdLine ), iperfCmdLine );
        scanForInt( queryString, "wifiinit=", &wifiInit);
        scanForInt( queryString, "wifiStats=", &wifiStats );
        scanForInt( queryString, "wifiscanstart=", &wifiScanStart );
        scanForInt( queryString, "wifiscanresults=", &wifiScanResults );
        scanForInt( queryString, "wifiAmpduGraph=", &wifiAmpduGraph );
        scanForInt( queryString, "wifiAmpduStart=", &wifiAmpduStart ); /* set to one when use first checks the AMPDU Graph checkbox */
        scanForInt( queryString, "irqinfo=", &irqInfo );
        scanForInt( queryString, "heapstats=", &heapStats );
        scanForInt( queryString, "satausb=", &sataUsb );
        scanForInt( queryString, "profiling=", &profiling );
        scanForInt( queryString, "PerfTop=", &PerfTop );
        scanForInt( queryString, "PerfCache=", &PerfCache );
        scanForInt( queryString, "PerfCacheResults=", &PerfCacheResults );
        scanForInt( queryString, "PerfDeep=", &PerfDeep );
        scanForInt( queryString, "PerfDeepResults=", &PerfDeepResults );
        scanForInt( queryString, "PerfFlame=", &PerfFlame );
        scanForStr( queryString, "PerfFlameCmdLine=", sizeof( PerfFlameCmdLine ), PerfFlameCmdLine );
        scanForInt( queryString, "PerfFlameSvgCount=", &PerfFlameSvgCount );
        scanForInt( queryString, "ChangeCpuState=", &ChangeCpuState );
        scanForInt( queryString, "LinuxTop=", &LinuxTop );
        scanForInt( queryString, "ContextSwitch=", &ContextSwitches );
        scanForInt( queryString, "DvfsControl=", &DvfsControl );
        scanForInt( queryString, "GovernorSetting=", &GovernorSetting );
    }
    else
    {
        printf( CONTENT_TYPE_HTML );
        printf( "~ERROR: QUERY_STRING is not defined~" );
        return( -1 );
    }

    printf( CONTENT_TYPE_HTML );

    printf( "QUERY_STRING len %u; (%s)", (unsigned int) strlen( queryString ), queryString );

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

        strncpy( versionInfo.platform, getPlatform(), sizeof( versionInfo.platform ) - 1 );
        strncpy( versionInfo.platVersion, getPlatformVersion(), sizeof( versionInfo.platVersion ) - 1 );
        versionInfo.majorVersion   = MAJOR_VERSION;
        versionInfo.minorVersion   = MINOR_VERSION;
        versionInfo.sizeOfResponse = sizeof( response );
        printf( "~PLATFORM~%s", versionInfo.platform );
        printf( "~PLATVER~%s", versionInfo.platVersion );
        printf( "~VERSION~Ver: %u.%u~", versionInfo.majorVersion, versionInfo.minorVersion );

        uname(&uname_info);
        printf("~UNAME~%d-bit %s %s~", (sizeof(char*) == 8)?64:32, uname_info.machine , uname_info.release );

        if (numCpusConf % 2 == 0)
        {
            leftrightmax = 2;
        }

        now.tv_sec = epochSeconds - ( tzOffset * 60 );
        settimeofday( &now, NULL );
        usleep( 200 );
        /*fflush(stdout);fflush(stderr);*/

        printf( "~CPUPERCENTS~" );
        printf( "<table cols=2 border=0 id=cpugraphs style=\"border-collapse:collapse;\" >\n" );

        for (cpupair = 0; cpupair < numCpusConf; cpupair += 2) {
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

                outputDashTickLines( NULL );

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
        printf( "</table>~" );
    }

    if ( wifiAmpduStart == 0 )
    {
        wifiAmpduStart = wifiStats; /* every time we get the wifi stats (every 10 seconds), reset the AMPUD collection and start collecting again */
    }

    /* if the checkbox for CPU Utilization OR Network Stats OR IRQ Counts is checked (any one of these needs to request data from bmemperf_server) */
    if (cpuInfo || netStatsUpdate || wifiScanStart || (wifiScanResults != -1) || wifiAmpduStart || irqInfo || PerfDeep || PerfCache || sataUsb ||
        LinuxTop || ContextSwitches || PerfFlame || strlen(iperfCmdLine) )
    {
        struct in_addr sin_temp_addr;

        strncpy( ThisHost, "localhost", sizeof( ThisHost ));
        getservbyname( "echo", "tcp" );

        strncpy( ThisHost, "localhost", sizeof( ThisHost ));

        sin_temp_addr.s_addr = get_my_ip4_addr();

        if ( sin_temp_addr.s_addr == 0 )
        {
            printf( "~FATAL~get_my_ip4_addr() failed to determine IP address.~" );
            return( -1 );
        }
        bcopy( &sin_temp_addr.s_addr, &( server.sin_addr ), sizeof( sin_temp_addr.s_addr ) );
        /*printf( "~TCP/Client running at HOST (%s) at INET ADDRESS : (%s)~", ThisHost, inet_ntoa( server.sin_addr ));*/

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

        if (PerfFlame)
        {
            if ( gPerfError == true)
            {
                printf( "~FATAL~kernel is not PERF kernel.~" );
            }
            else
            {
                if (PerfFlame == PERF_FLAME_INIT) /* Init */
                {
                    printf( "~PerfFlameInit~" );
                    printf("<h2>Flame&nbsp;Graph</h2>");
                    printf("<table cols=10 border=0 style=\"border-collapse:collapse;\" width=\"100%%\" >\n");
                    printf("<tr><td width=100 >CmdLine:</td><td colspan=9 width=800 ><input type=text id=PerfFlameCmdLine size=120 ></td></tr>\n");
                    printf("<tr><td>&nbsp;</td><td style=\"width:70px;\" ><input type=button value=\"Start\" onclick=\"MyClick(event);\" id=PerfFlameStartStop ></td>\n");
                    printf("<td width=70 align=right id=PerfFlameDurationText onclick=\"MyClick(event);\" >Duration:</td><td width=70 align=left id=PerfFlameDuration >&nbsp;</td>\n");
                    printf("<td width=70 align=right >File&nbsp;Size:</td><td width=70 align=left id=PerfFlameSize >&nbsp;</td>\n");
                    printf("<td width=70 align=right >State:</td><td width=70 align=left id=PerfFlameState >&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td></tr>\n");
                    printf(" <tr><td colspan=10 id=PerfFlameSvg ></td></tr>\n");
                    printf("</table>\n");
                    printf("~");
                    /*printf("<div id=PerfFlameSvg >&nbsp;</div>\n~");*/
                    request.cmdSecondary       = BMEMPERF_CMD_STOP_PERF_FLAME;
                    request.cmdSecondaryOption = 0;
                } else if (PerfFlame == PERF_FLAME_START /* start recording */ ) {
                    printf( "~PERF_FLAME start; cmdLine (%s)~", PerfFlameCmdLine );
                    request.cmdSecondary       = BMEMPERF_CMD_START_PERF_FLAME;
                    request.cmdSecondaryOption = 0;
                    strncpy ( (char*) &request.request.strCmdLine, PerfFlameCmdLine, sizeof(request.request.strCmdLine) - 1 );
                    decodeURL( request.request.strCmdLine );

                    create_uuid( strUuid );
                    printf("~PERFRECORDUUID~%s~", strUuid );
                } else if (PerfFlame == PERF_FLAME_RECORDING /* get status of recording */ ) {
                    PRINTF( "~PERF_FLAME status~" );
                    request.cmdSecondary       = BMEMPERF_CMD_STATUS_PERF_FLAME;
                } else if (PerfFlame == PERF_FLAME_STOP /* stop record and create svg file */ ) {
                    PRINTF( "~PERF_FLAME stop~" );
                    request.cmdSecondary       = BMEMPERF_CMD_STOP_PERF_FLAME;
                } else if (PerfFlame == PERF_FLAME_CREATE_SCRIPT_OUT /* run perf script command */ ) {
                    char         line[MAX_LINE_LENGTH];
                    char         tempPath[MAX_LINE_LENGTH];

                    scanForStr( queryString, "perf_out=", sizeof( strUuid ), strUuid );
                    printf( "~DEBUG~PERF_FLAME create_script_out ... strUuid (%s)~", strUuid );
                    PrependTempDirectory( tempPath, sizeof( tempPath ), "" );
                    snprintf( line, sizeof(line)-1, "(cd %s && perf script > %s.out 2>/dev/null)", tempPath, strUuid );
                    printf( "~DEBUG~issuing system(%s)~", line );
                    system( line );
                    snprintf( line, sizeof(line)-1, "(cp %s%s.out . && rm %s%s.out)", tempPath, strUuid, tempPath, strUuid );
                    printf( "~DEBUG~issuing system(%s)~", line );
                    system( line );
                    printf( "~PERFSCRIPTDONE~" );
                } else if (PerfFlame == PERF_FLAME_DELETE_OUT_FILE ) {
                    char  svgFilename[TEMP_FILE_FULL_PATH_LEN];
                    char *pos = NULL;

                    scanForStr( queryString, "perf_out=", sizeof( strUuid ), strUuid );
                    snprintf( svgFilename, sizeof(svgFilename), "%s.svg", strUuid );

                    /* remove the .out files */
                    pos = strstr(svgFilename, ".svg");
                    if (pos)
                    {
                        strncpy ( pos, ".out", 4 );
                        remove( svgFilename );
                        printf( "~PERFFLAME_DELETEOUTFILE_DONE~%s~", svgFilename );
                    }
                } else {
                    PRINTF( "~PERF_FLAME unknown~" );
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

        if ( wifiScanStart )
        {
            PRINTF( "~Sending BMEMPERF_CMD_WIFI_SCAN_START request %d ~", BMEMPERF_CMD_WIFI_SCAN_START );
            request.cmdSecondary       = BMEMPERF_CMD_WIFI_SCAN_START;
            request.cmdSecondaryOption = 0;
        }
        else if ( wifiScanResults != -1 )
        {
            PRINTF( "~Sending BMEMPERF_CMD_WIFI_SCAN_RESULTS request ... ServerIdx %d~", wifiScanResults );
            request.cmdSecondary       = BMEMPERF_CMD_WIFI_SCAN_GET_RESULTS;
            request.cmdSecondaryOption = wifiScanResults;
        }

        if ( wifiAmpduStart == 1 )
        {
            PRINTF( "~DEBUG~Sending BMEMPERF_CMD_WIFI_AMPDU_START request~" );
            request.cmdSecondary       = BMEMPERF_CMD_WIFI_AMPDU_START;
        }

        if ( strlen(iperfCmdLine) )
        {
            const char * fullpath = executable_fullpath( "iperf" );
            if ( strlen( fullpath ) )
            {
                if (strncmp(iperfCmdLine, "STOP", 4) == 0 )
                {
                    request.cmdSecondary       = BMEMPERF_CMD_IPERF_STOP;
                }
                else
                {
                    request.cmdSecondary       = BMEMPERF_CMD_IPERF_START;
                    strncpy ( (char*) &request.request.strCmdLine, iperfCmdLine, sizeof(request.request.strCmdLine) - 1 );
                    decodeURL( request.request.strCmdLine );
                    printf( "~DEBUG~iperfCmdLine; cmdLine (%s)~", iperfCmdLine );
                }
                request.cmdSecondaryOption = (strstr( iperfCmdLine, "-c")) ? 0 : 1;
                printf( "~DEBUG~iperfCmdLine; cmdLine (%s) ... fullpath (%s) ... secondary (%d) ~", iperfCmdLine, fullpath, (int) request.cmdSecondaryOption );
            }
            else
            {
                printf( "~iperfErrorClient~Executable not found~" );
                printf( "~iperfErrorServer~Executable not found~" );
            }
        }

        PRINTF( "~DEBUG~Sending request ... cmdSecondary 0x%x ... option 0x%lx ~", request.cmdSecondary, request.cmdSecondaryOption );
        rc = send_request_read_response( &server, (unsigned char*) &request, sizeof(request), (unsigned char*) &response, sizeof(response), BSYSPERF_SERVER_PORT, BMEMPERF_CMD_GET_CPU_IRQ_INFO );
        if (rc < 0)
        {
            PRINTF( "error sending BMEMPERF_CMD_GET_CPU_IRQ_INFO request; rc %d \n", rc );
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
        else if (PerfFlame)
        {
            printf( "~PERFFLAMESTATUS~%lu~", (unsigned long int) response.response.overallStats.fileSize );
            printf( "~PERFFLAMEPIDCOUNT~%lu~", response.response.overallStats.pidCount );
            printf( "PERFFLAMESIZE %lu; PIDCOUNT %lu~", (unsigned long int) response.response.overallStats.fileSize, (unsigned long int) response.response.overallStats.pidCount );
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

        output_cpu_frequencies();
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

        PRINTF( "~DEBUG~irqTotal (%lu); irqTotal2 (%lu)~", irqTotal, irqTotal2 );
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

    /* if the checkbox for Network Stats was just checked */
    if ( netStatsInit )
    {
        char dash_line_id[32];

        get_netstat_data( &g_netStats[0] );

        printf( "~netStatsInit~" );
        printf( "<table cols=9 width=\"1024\" style=\"border-collapse:collapse;\" border=0 cellpadding=0 >" );

        printf( "<tr><td class=whiteborders18 align=left style=\"font-weight:bold;width:320px;\" >Network Interface Statistics</td>" );
        printf( "<td style=\"width:20px;\" ><input type=checkbox id=checkboxiperfrow onclick=\"MyClick(event);\" ></td>");
        printf( "<td style=\"width:50px;\" align=left >iperf</td>" );
        printf( "<td>&nbsp;</td>" );
        printf( "<td>&nbsp;</td>" );
        printf( "<td>&nbsp;</td>" );
        printf( "<td>&nbsp;</td>" );
        printf( "<td>&nbsp;</td>" );
        printf( "<td>&nbsp;</td>" );
        printf( "</tr>" );
        printf( "<tr><td colspan=9><table width=\"100%%\" style=\"border-collapse:collapse;\" border=1 cellpadding=3 >\n" );
        printf( "<tr bgcolor=lightgray ><th>Name</th><th>IP Addr</th><th>Rx Bytes</th><th>Tx Bytes</th><th>Rx Errors</th><th>Tx Errors</th>"
                "<th>Rx Mbps (Avg)</th><th>Tx Mbps (Avg)</th><th>Graph</th></tr>\n" );
        for (idx = 0; idx <= g_netStatsIdx; idx++) {
            printf( "<tr><td>%s</td> <td align=center >%s</td> <td align=center id=netif_rxBytes_%d >%s</td>", g_netStats[idx].name, g_netStats[idx].ipAddress, idx, formatul( g_netStats[idx].rxBytes ));
            printf( "<td id=netif_txBytes_%d align=center >%s</td>", idx, formatul( g_netStats[idx].txBytes ));
            printf( "<td id=netif_rxError_%d align=center >%s</td>", idx, formatul( g_netStats[idx].rxErrors ));
            printf( "<td id=netif_txError_%d align=center >%s</td>", idx, formatul( g_netStats[idx].txErrors ));
            printf( "<td id=netif%urx align=center ><!-- value inserted via javascript --></td>", idx );
            printf( "<td id=netif%utx align=center ><!-- value inserted via javascript --></td>", idx );
            printf( "<td align=center ><input type=checkbox id=checkbox_netgfx%u onclick=\"MyClick(event);\" ></td>", idx );
            printf( "</tr>\n" );

            /* add in the histogram for Rx and Tx */
            printf( "<tr id=row_netgfxsvg%u style=\"visibility:hidden;\" ><td colspan=9 align=left "
                    "style=\"border-right: 1pt solid white;border-left: 1pt solid white;\" ><table style=\"border-collapse:collapse;\" border=0 ><tr>", idx );
            printf( "<td width=500 align=left ><svg id=svg_netgfx_rx_%u height=\"100\" width=\"500\" style=\"border:solid 1px black;font-size:8pt;\" >", idx );
            printf( "<polyline id=polyline_netgfx_rx_%u style=\"fill:none;stroke:red;stroke-width:2\" />\n", idx );
            sprintf(dash_line_id, "dash_rx_%u", idx );
            outputDashTickLines( dash_line_id );
            printf( "<text x=30 y=13>RX</text>" );
            printf( "</svg></td>" );
            printf( "<td width=500 align=left ><svg id=svg_netgfx_tx_%u height=\"100\" width=\"500\" style=\"border:solid 1px black;font-size:8pt;\" >", idx );
            printf( "<polyline id=polyline_netgfx_tx_%u style=\"fill:none;stroke:turquoise;stroke-width:2\" />\n", idx );
            sprintf(dash_line_id, "dash_tx_%u", idx );
            outputDashTickLines( dash_line_id );
            printf( "<text x=30 y=13>TX</text>" );
            printf( "</svg></td>" );
            printf( "</tr>" );
            printf( "<tr id=row_netgfxtxt%u style=\"visibility:hidden;\" >", idx );
            printf( "<td width=500 align=left ><textarea id=txt_netgfx_rx_%u rows=1 style=\"border:solid 1px black;width:100%%;\" ></textarea></td>", idx );
            printf( "<td width=500 align=left ><textarea id=txt_netgfx_tx_%u rows=1 style=\"border:solid 1px black;width:100%%;\" ></textarea></td>", idx );
            printf( "</tr>" );
            printf( "</table></td>" );
        }
        printf( "</table></td></tr>\n" );
        printf( "</table>~" ); /* end netStatsInit */
    }

    /* if the checkbox for Network Stats is checked */
    if ( netStatsUpdate )
    {
        get_netstat_data( &g_netStats[0] );

        printf( "~netStatsUpdate~%d~", g_netStatsIdx + 1 );
        for (idx = 0; idx <= g_netStatsIdx; idx++) {
            #if 1
            printf( "%s|", formatul( g_netStats[idx].rxBytes ));
            printf( "%s|", formatul( g_netStats[idx].txBytes ));
            printf( "%s|", formatul( g_netStats[idx].rxErrors ));
            printf( "%s~", formatul( g_netStats[idx].txErrors ));
            #else
            printf( "%llu %llu %llu %llu~", g_netStats[idx].rxBytes , g_netStats[idx].txBytes , g_netStats[idx].rxErrors , g_netStats[idx].txErrors );
            #endif
        }
        /* end netStatsUpdate */

        /* output some of the above information again in an unformatted way to make it easier to compute bits per second */
        printf( "~NETBYTES~" );
        for (idx = 0; idx <= g_netStatsIdx; idx++) {
            printf( "%llu ", g_netStats[idx].rxBytes );
            printf( "%llu,", g_netStats[idx].txBytes );
        }
        printf( "~" );

        if (iperfInit)
        {
            char *wlan0Address = Bsysperf_FindWlan0Address();

            printf( "~iperfInit~" );
            printf( "<span style=\"display:inline-block; font-size:18pt; margin-bottom:10px; margin-top:5px; \" >&nbsp;iperf</span><div style=\"border:1px solid black;\" >" );
            printf( "<table cols=\"3\" style=\"border-collapse:collapse;borders:1px solid black;\" border=\"0\" cellpadding=\"3\" width=900 >" );
            printf( "   <tr>" );
            printf( "       <td width=\"110\" align=\"left\" nowrap ><b>Server:&nbsp;</b>iperf&nbsp;-s&nbsp;</td>" );
            printf( "       <td width=\"40\" align=\"left\" >" );
            printf( "           <input type=\"text\" id=iperf_options_s placeholder=\"Extra Options\" style=\"border:1px solid black;width:200px;\" onfocus=\"FocusEntryBox(event);\" "
                    "onblur=\"BlurEntryBox(event);\" onkeyup=\"KeyupEntryBox(event);\" value=\"-w4096K -N\" title=\"Extra Options\" >" );
            printf( "       </td>" );
            printf( "       <td>" );
            printf( "           <table style=\"border-collapse:collapse;\" border=\"0\" cellpadding=\"0\" style=\"border:1px solid black;\" >" );
            printf( "               <tr>" );
            printf( "                   <td style=\"border:none;\" width=\"30\" nowrap>&nbsp;</td>" );
            printf( "                   <td align=\"left\" class=whiteborders18 style=\"width:40px;\" align=\"left\" ><input type=\"button\" id=iperf_start_stop_s value=\"START\" "
                    "onclick=\"MyClick(event);\" ></td>" );
            printf( "                   <td style=\"border:none;\" width=\"20\" nowrap>&nbsp;</td>" );
            printf( "                   <td align=\"left\" class=whiteborders18 style=\"width:20px;font-size:12pt;\" id=iperf_count_s ></td>" );
            printf( "                   <td style=\"border:none;\" width=\"20\" nowrap>&nbsp;</td>" );
            printf( "                   <td align=\"left\" class=whiteborders18 style=\"width:400px;font-size:12pt;\" id=iperf_status_s ></td>" );
            printf( "               </tr>" );
            printf( "           </table>" );
            printf( "       </td>" );
            printf( "   </tr>" );
            printf( "   <tr><td colspan=\"3\" id=iperf_cmd_s align=\"left\" style=\"color:red;font-style:italic;\" ></td></tr>" );

            printf( "   <tr>" );
            printf( "       <td width=\"110\" align=\"left\" nowrap ><b>Client:&nbsp;</b>iperf&nbsp;-c&nbsp;</td>" );
            printf( "       <td><table border=0 style=\"border-collapse:collapse;\" >" );
            printf( "           <tr>" );
            printf( "               <td><input type=\"text\" id=iperf_addr placeholder=\"Server IP\" style=\"border:1px solid black;\" size=\"10\" value=\"%s\" "
                    "onfocus=\"FocusEntryBox(event);\" onblur=\"BlurEntryBox(event);\" onkeyup=\"KeyupEntryBox(event);\" title=\"Server Address\" > </td>",  wlan0Address );
            printf( "               <td style=\"border:none;\" width=\"25\" nowrap align=right >-t&nbsp;</td>" );
            printf( "               <td style=\"border:none;\" ><input type=\"text\" id=iperf_duration placeholder=\"Duration\" style=\"border:1px solid black;\" size=\"3\" value=\"100\" "
                    "onfocus=\"FocusEntryBox(event);\" onblur=\"BlurEntryBox(event);\" onkeyup=\"KeyupEntryBox(event);\" title=\"Duration\" ></td>" );
            printf( "           </tr>" );
            printf( "       </table></td>" );
            printf( "       <td >" );
            printf( "           <table style=\"border-collapse:collapse;\" border=\"0\" cellpadding=\"0\" >" );
            printf( "               <tr>" );
            printf( "                   <td style=\"border:none;\" width=\"20\" nowrap align>-p&nbsp;</td>" );
            printf( "                   <td><input type=\"text\" id=iperf_port_c placeholder=\"# Streams\" style=\"border:1px solid black;\" size=\"3\" value=\"5001\" "
                    "onfocus=\"FocusEntryBox(event);\" onblur=\"BlurEntryBox(event);\" onkeyup=\"KeyupEntryBox(event);\" title=\"# Streams\" ></td>" );
            printf( "                   <td style=\"border:none;\" width=\"20\" nowrap>&nbsp;</td>" );
            printf( "                   <td><input type=\"text\" id=iperf_options_c placeholder=\"Extra Options\" style=\"border:1px solid black;\" size=\"20\" value=\"-w4096K -N\" "
                    "onfocus=\"FocusEntryBox(event);\" onblur=\"BlurEntryBox(event);\" onkeyup=\"KeyupEntryBox(event);\" title=\"Extra Options\" ></td>" );
            printf( "                   <td style=\"border:none;\" width=\"30\" nowrap>&nbsp;</td>" );
            printf( "                   <td align=\"left\" class=whiteborders18 style=\"width:40px;\" ><input type=\"button\" id=iperf_start_stop_c value=\"START\" onclick=\"MyClick(event);\" ></td>" );
            printf( "                   <td align=\"left\" class=whiteborders18 style=\"width:10px;\" >&nbsp;</td>" );
            printf( "                   <td align=\"left\" class=whiteborders18 style=\"width:20px;font-size:12pt;\" id=iperf_count_c ></td>" );
            printf( "                   <td align=\"left\" class=whiteborders18 style=\"width:10px;\" >&nbsp;</td>" );
            printf( "                   <td align=\"left\" class=whiteborders18 style=\"width:400px;font-size:12pt;\" id=iperf_status_c ></td>" );
            printf( "               </tr>" );
            printf( "           </table>" );
            printf( "       </td>" );
            printf( "   </tr>" );
            printf( "   <tr><td colspan=\"3\" id=iperf_cmd_c align=\"left\" style=\"color:red;font-style:italic;\" > </td></tr>" );
            printf( "</table></div>~" ); /* end iperfInit */
        }

        if ( iperfRunningClient || iperfRunningServer )
        {
            char *iperf_processes = Bsysperf_GetProcessCmdline( "iperf -" );
            if ( iperf_processes )
            {
                char *posEol   = NULL;
                /*printf("~DEBUG~iperf_processes (%s)~", iperf_processes );*/
                if ( iperfRunningClient )
                {
                    char * line=strstr( iperf_processes, "iperf -c" ); /* check to see if iperf client is already running */
                    /* could get something like this: root      6424  iperf -c 192.168.1.209 -t 100 -p 5001 -w4096K -N */
                    /*                                      ^                 */
                    /*                                      -10               */
                    if ( line )
                    {
                        posEol = Bsysperf_ReplaceNewlineWithNull( line );
                        printf( "~iperfRunningClient~%s~", line );
                        Bsysperf_RestoreNewline( posEol );

                        if ( iperfPidClient && line && strlen(line) )
                        {
                            unsigned long int pid = 0;
                            sscanf( &line[-10], "%lu", &pid ); /* backup 10 characters to get access to the pid */
                            printf( "~iperfPidClient~%lu~", pid );
                        }
                    }
                    else
                    {
                        printf( "~iperfPidClient~0~iperfRunningClient~NONE~" );
                    }
                }

                if ( iperfRunningServer )
                {
                    char * line=strstr( iperf_processes, "iperf -s" ); /* check to see if iperf server is already running */
                    /* could get something like this: root      6424  iperf -s -p 5001 -w4096K -N -Q 1469667343 */
                    /*                                      ^                 */
                    /*                                      -10               */
                    if ( line )
                    {
                        posEol = Bsysperf_ReplaceNewlineWithNull( line );
                        printf( "~iperfRunningServer~%s~", line );
                        Bsysperf_RestoreNewline( posEol );

                        if ( iperfPidServer && line && strlen(line) )
                        {
                            unsigned long int pid = 0;
                            sscanf( &line[-10], "%lu", &pid ); /* backup 10 characters to get access to the pid */
                            printf( "~iperfPidServer~%lu~", pid );
                        }
                    }
                    else
                    {
                        printf( "~iperfPidServer~0~iperfRunningServer~NONE~" );
                    }
                }

                Bsysperf_Free ( iperf_processes );
            }
            else
            {
                printf( "~iperfPidClient~0~iperfPidServer~0~" );
                if ( iperfRunningServer )
                {
                    printf( "~iperfRunningClient~NONE~" );
                }
                if ( iperfRunningClient )
                {
                    printf( "~iperfRunningServer~NONE~" );
                }
            }
        }
    }

#ifdef BWL_SUPPORT
    /* if the checkbox for WiFi Statistics is checked */
    if ( wifiInit )
    {
        printf( "~WIFIINIT~" );
        printf( "<table cols=8 style=\"border-collapse:collapse;\" border=0 cellpadding=3 >" );

        printf( "<tr><th style=\"vertical-align:middle;font-size:18.0pt; \" align=left width=250 ><span>WiFi&nbsp;Statistics</span></th>");
        printf( "<th colspan=6 style=\"vertical-align:middle;\" >");
        printf( "<table border=0 style=\"border-collapse:collapse;\" ><tr><td><input type=button id=WifiScan value=Scan onclick=\"MyClick(event);\" ></td>");
        printf( "<td width=30 >&nbsp;</td>");
        printf(" <td><table border=0 ><tr><td><input type=checkbox checked>Stats</td><td id=WIFICOUNTDOWN >ab</td></tr></table></td>");
        printf( "<td width=30 >&nbsp;</td>");
        printf(" <td><table border=0 ><tr><td><input type=checkbox id=checkboxWifiAmpduGraph onclick=\"MyClick(event);\" >AMPDU Graph</td><td id=countWifiAmpduGraph ></td></tr></table></td>");
        printf( "<td width=30 >&nbsp;</td>");
        printf( "</tr></table></td>" );
        printf(" </tr>");

        /* this row is where the wifi statistics will be put */
        printf( "<tr><td colspan=8><div id=WIFISTATS></div></td></tr>");

        /* this row is where the results from the SCAN will be put */
        printf( "<tr><td colspan=8 ><table border=0 style=\"border-collapse:collapse;\" ><tbody id=WIFISCANRESULTS ></tbody></table></td></tr>");

        #if 0
        /* this row is where the AMPDU results will be put */
        printf( "<tr><td colspan=8 ><table border=0 style=\"border-collapse:collapse;\" ><tbody id=WIFIAMPDURESULTS >\n");
        Bsysperf_WifiAmpduInit();
        printf( "</tbody></table></td></tr>");
        #endif

        printf("</table>");

        printf( "~WIFISCANMAXAPS~%d~", wl_bss_info_t_max_num );
    }

    if ( wifiStats )
    {
        int             tRetCode = BWL_ERR_SUCCESS;
        uint32_t        nrate    = 0;
        int             rate     = 0;
        float           frate    = 0.0;
        RevInfo_t       tRevInfo;
        ScanInfo_t      tScanInfo;
        WiFiCounters_t  tCounters;
        char            mac_addr[32];
        BSYSPERF_PHY_RSSI_ANT_T tRssiAnt;

        memset( &tRevInfo, 0, sizeof(tRevInfo) );
        memset( &tScanInfo, 0, sizeof(tScanInfo) );
        memset( &tCounters, 0, sizeof(tCounters) );
        memset( &tRssiAnt, 0, sizeof(tRssiAnt) );

        if ( (tRetCode == BWL_ERR_SUCCESS) && (tRetCode = Bsysperf_WifiGetRevs( WIFI_INTERFACE_NAME, &tRevInfo )) != BWL_ERR_SUCCESS)
        {
            printf("%s:%u: Bsysperf_WifiGetRevs() failed ... rc %d \n", __FILE__, __LINE__, tRetCode );

            /* if wifi is not supported, disable the WiFi checkbox and uncheck it */
            printf("~WIFIDISABLED~");
        }
        else
        {
            printf("~WIFIENABLED~");
        }

        if ( (tRetCode == BWL_ERR_SUCCESS) && (tRetCode = Bsysperf_WifiGetConnectedInfo( WIFI_INTERFACE_NAME, &tScanInfo )) != BWL_ERR_SUCCESS)
        {
            printf("%s:%u: Bsysperf_WifiGetConnectedInfo() failed ... rc %d \n", __FILE__, __LINE__, tRetCode );
        }

        if ( (tRetCode == BWL_ERR_SUCCESS) && (tRetCode = Bsysperf_WifiGetCounters( WIFI_INTERFACE_NAME, &tCounters )) != BWL_ERR_SUCCESS)
        {
            printf("%s:%u: Bsysperf_WifiGetCounters() failed ... rc %d \n", __FILE__, __LINE__, tRetCode );
        }

        if ( (tRetCode == BWL_ERR_SUCCESS) && (tRetCode = Bsysperf_WifiGetNRate( WIFI_INTERFACE_NAME, &nrate )) != BWL_ERR_SUCCESS)
        {
            printf("%s:%u: Bsysperf_WifiGetNRate() failed ... rc %d \n", __FILE__, __LINE__, tRetCode );
        }

        if ( (tRetCode == BWL_ERR_SUCCESS) && (tRetCode = Bsysperf_WifiGetRssiAnt( WIFI_INTERFACE_NAME, &tRssiAnt )) != BWL_ERR_SUCCESS)
        {
            printf("%s:%u: Bsysperf_WifiGetRssiAnt() failed ... rc %d \n", __FILE__, __LINE__, tRetCode );
        }


        printf( "~WIFISTATS~" );
        printf( "<table cols=8 style=\"border-collapse:collapse;\" border=0 cellpadding=3 ><tr>" );
        printf( "<td align=left colspan=2 class=silver_allborders style=\"border-top: solid thin black;border-left: solid thin black;\" >Chip Number:<span class=bluetext>%d (0x%x)</span></td> ",
                tRevInfo.ulChipNum, tRevInfo.ulChipNum );
        printf( "<td align=left colspan=2 class=silver_allborders style=\"border-top: solid thin black;\" >Driver Version:<span class=bluetext>%d.%d.%d.%d </span></td> ",
                (uint8_t)( tRevInfo.ulDriverRev>>24 ), (uint8_t)( tRevInfo.ulDriverRev>>16 ), (uint8_t)( tRevInfo.ulDriverRev>>8 ), (uint8_t)tRevInfo.ulDriverRev );
        printf( "<td align=left colspan=2 class=silver_allborders style=\"border-top: solid thin black;\" >PCI vendor id:<span class=bluetext>0x%x </span></td> ", tRevInfo.ulVendorId );
        printf( "<td align=left colspan=2 class=silver_allborders style=\"border-top: solid thin black;border-right: solid thin black;\" >Device id of chip:<span class=bluetext>0x%x </span></td> ",
                tRevInfo.ulDeviceId );
        printf( "</tr>\n");

        #if 0
        printf( "<tr>");
        printf( "<td align=right >Radio Revision:</td><td align=left >    0x%08x </td><td width=30></td> ", tRevInfo.ulRadioRev );
        printf( "<td align=right >Chip Revision:</td><td align=left >     0x%08x </td><td width=30></td> ", tRevInfo.ulChipRev );
        printf( "<td align=right >Core Revision:</td><td align=left >     0x%08x </td><td width=30></td> ", tRevInfo.ulCoreRev );
        printf( "<td align=right >Board Identifier:</td><td align=left >  0x%08x </td><td width=30></td> ", tRevInfo.ulBoardId );
        printf( "</tr>\n<tr>");
        printf( "<td align=right >Board Vendor:</td><td align=left >      0x%08x </td><td width=30></td> ", tRevInfo.ulBoardVendor );
        printf( "<td align=right >Board Revision:</td><td align=left >    0x%08x </td><td width=30></td> ", tRevInfo.ulBoardRev );
        printf( "<td align=right >Microcode Version:</td><td align=left > 0x%08x </td><td width=30></td> ", tRevInfo.ulUcodeRev );
        printf( "<td align=right >Bus Type:</td><td align=left >          0x%08x </td><td width=30></td> ", tRevInfo.ulBus );
        printf( "</tr>\n<tr>");
        printf( "<td align=right >Phy Type:</td><td align=left >          0x%08x </td><td width=30></td> ", tRevInfo.ulPhyType );
        printf( "<td align=right >Phy Revision:</td><td align=left >      0x%08x </td><td width=30></td> ", tRevInfo.ulPhyRev );
        printf( "<td align=right >Anacore Rev:</td><td align=left >       0x%08x </td><td width=30></td> ", tRevInfo.ulAnaRev );
        printf( "<td align=right >Chip Package Info:</td><td align=left > 0x%08x </td><td width=30></td> ", tRevInfo.ulChipPkg );
        printf( "</tr>");
        #endif

        printf( "<tr>");
        printf( "<td align=left colspan=2 class=silver_allborders style=\"border-left: solid thin black;\" >RSSI:<span class=bluetext>%d (%s)</span></td> ", tScanInfo.lRSSI, Bsysperf_WifiDbStrength ( tScanInfo.lRSSI ) );
        printf( "<td align=left colspan=2 class=silver_allborders style=\"border-left: solid thin black;\" >PHY_RSSI_ANT:<span class=bluetext nowrap >" );
        for (idx=0; idx<BSYSPERF_RSSI_ANT_MAX; idx++)
        {
            printf( "&nbsp;&nbsp;%d", tRssiAnt.rssi_ant[idx] );
        }
        printf(" </span></td>" );
        printf( "<td align=left colspan=2 class=silver_allborders >PHY Noise:<span class=bluetext>%d (%s)</span></td> ", tScanInfo.lPhyNoise, Bsysperf_WifiDbStrength ( tScanInfo.lPhyNoise )  );
        printf( "<td align=left colspan=2 class=silver_allborders style=\"border-right: solid thin black;\" >802.11 Modes:<span class=bluetext>%d </span></td> ", tScanInfo.ul802_11Modes );
        printf( "</tr>");

        printf( "<tr>");
        printf( "<td align=left colspan=2 class=silver_allborders style=\"border-left: solid thin black;\" >Chan:<span class=bluetext>%d </span></td> ", tScanInfo.ulChan );
        printf( "<td align=left colspan=2 class=silver_allborders >PrimChan:<span class=bluetext>%d </span></td> ", tScanInfo.ulPrimChan );
        printf( "<td align=left colspan=2 class=silver_allborders >Locked:<span class=bluetext>%s </span></td> ", TRUE_OR_FALSE(tScanInfo.bLocked) );
        printf( "<td align=left colspan=2 class=silver_allborders style=\"border-right: solid thin black;\" >WPS:<span class=bluetext>%s</span></td> ", TRUE_OR_FALSE(tScanInfo.bWPS) );
        printf( "</tr>");

        printf( "<tr>");
        printf( "<td align=left colspan=2 class=silver_allborders style=\"border-left: solid thin black;\" >Rate:<span class=bluetext>%d </span></td> ", tScanInfo.lRate );
        printf( "<td align=left colspan=2 class=silver_allborders >Bandwidth:<span class=bluetext>%s </span></td> ", Bsysperf_WifiBandwidthStr( tScanInfo.tBandwidth ) );
        printf( "<td align=left colspan=2 class=silver_allborders >ChanSpec:<span class=bluetext>%s </span></td> ", tScanInfo.cChanSpec );
        {
            char rate_buf[128];
            memset(rate_buf, 0, sizeof(rate_buf) );
            wl_rate_print(rate_buf, nrate );
            replace_space_with_nbsp(rate_buf, sizeof(rate_buf) );
            printf( "<td align=left colspan=2 style=\"width:310px;border-right: solid thin black;\" >NRate:<span class=bluetext nowrap >%s</span></td> ", rate_buf );

        }
        printf( "</tr>");

        memset(mac_addr, 0, sizeof(mac_addr));
        snprintf (mac_addr, sizeof(mac_addr), "%02X:%02X:%02X:%02X:%02X:%02X",
               tScanInfo.BSSID.octet[0], tScanInfo.BSSID.octet[1], tScanInfo.BSSID.octet[2],
               tScanInfo.BSSID.octet[3], tScanInfo.BSSID.octet[4], tScanInfo.BSSID.octet[5]);

#ifdef BWL_SUPPORT
        if ( wifiAmpduGraph )
        {
            memset( &lAmpduData, 0, sizeof(lAmpduData) );

            Bsysperf_WifiReadAmpduData( &lAmpduData );
        }
#endif

        /*printf( "<tr><td colspan=8 >&nbsp;</td></tr>");*/ /* blank row */
        printf( "<tr><td align=left nowrap colspan=2 class=silver_allborders style=\"border-left: solid thin black;\" >SSID:<span class=bluetext>%s</span></td> ", tScanInfo.tCredentials.acSSID );
        printf( "<td align=left colspan=2 class=silver_allborders >MAC Addr:<span class=bluetext>%s</span></td> ", mac_addr );
        printf( "<td align=left colspan=2 class=silver_allborders ><table style=\"border-collapse:collapse;\" ><tr><td>Rate (Mbps):</td><td><span class=bluetext><div id=WIFIRATE ></div></span></td></tr></table></td> " );
        printf( "<td align=left colspan=2 class=silver_allborders style=\"border-right: solid thin black;\" >AuthType:<span class=bluetext>%d</span></td>", tScanInfo.ulAuthType );
        printf( "</tr>");
        /*printf( "<tr><td colspan=8 >&nbsp;</td></tr>");*/ /* blank row */

#ifdef BWL_SUPPORT
        printf( "<tr><td align=left nowrap colspan=2 class=silver_allborders style=\"border-left: solid thin black;\" >Tot_mpdus:<span class=bluetext>%u</span></td> ", lAmpduData.tot_mpdus );
        printf( "<td align=left colspan=2 class=silver_allborders >Tot_ampdus:<span class=bluetext>%u</span></td> ", lAmpduData.tot_ampdus );
        printf( "<td align=left colspan=2 class=silver_allborders >MpdusPerAmdpu:<span class=bluetext>%u</span></td> ", lAmpduData.mpduperampdu );
        printf( "<td align=left colspan=2 class=silver_allborders style=\"border-right: solid thin black;\" >&nbsp;</td> " );
        printf( "</tr>");
#endif
        printf( "<tr>");
        printf( "<td align=left colspan=2 class=silver_allborders style=\"border-left: solid thin black;\" >Tx Bytes:<span class=bluetext>%s </span></td> ", formatul( tCounters.txbyte ) );
        printf( "<td align=left colspan=2 class=silver_allborders >Tx Frames:<span class=bluetext>%s </span></td> ", formatul( tCounters.txframe ) );
        printf( "<td align=left colspan=2 class=silver_allborders >Tx Errors:<span class=bluetext>%s </span></td> ", formatul( tCounters.txerror ) );
        printf( "<td align=left colspan=2 class=silver_allborders style=\"border-right: solid thin black;\" >Tx Re-trans:<span class=bluetext>%s </span></td> ", formatul( tCounters.txretrans ) );
        printf( "</tr>");

        printf( "<tr>");
        printf( "<td align=left colspan=2 class=silver_allborders style=\"border-left: solid thin black;border-bottom: solid thin black;\" >Rx Bytes:<span class=bluetext>%s </span></td> ", formatul( tCounters.rxbyte ) );
        printf( "<td align=left colspan=2 class=silver_allborders style=\"border-bottom: solid thin black;\" >Rx Frames:<span class=bluetext>%s </span></td> ", formatul( tCounters.rxframe ) );
        printf( "<td align=left colspan=2 class=silver_allborders style=\"border-bottom: solid thin black;\" >Rx Errors:<span class=bluetext>%s </span></td> ", formatul( tCounters.rxerror ) );
        printf( "<td align=left colspan=2 class=silver_allborders style=\"border-right: solid thin black;border-bottom: solid thin black;\" >&nbsp;</td> " );
        printf( "</tr>");
        printf( "</table>~" );

        Bsysperf_WifiGetRate( WIFI_INTERFACE_NAME, &rate );
        frate = rate / 2.0;
        printf("~WIFIRATE~%5.3f~", frate );

#ifdef BWL_SUPPORT
        if ( wifiAmpduGraph )
        {
            printf("~wifiAmpduGraph~");
            Bsysperf_WifiOutputAntennasHtml( &lAmpduData.antennas );
            printf("~");
        }
#endif
    }

    if ( wifiScanResults != -1 )
    {
        printf("~DEBUG~WIFISCANNUMAPS-%lu~", response.response.overallStats.ulWifiScanApCount );
        if ( response.response.overallStats.ulWifiScanApCount > 0 )
        {
            int i = 0;
            unsigned char *pbssInfo = (unsigned char *)&response.response.overallStats.bssInfo;
            char           bssPrintBuffer[1024];
            int            line_count = 0;
            int            buffer_len = 0;
            char           tempFilename[TEMP_FILE_FULL_PATH_LEN];

            PrependTempDirectory( tempFilename, sizeof( tempFilename ), "stdout_redirected.txt" );

            printf("~WIFISCANNUMAPS~%lu~DEBUG~sizeof(wl_bss_info_t) %d ~", response.response.overallStats.ulWifiScanApCount, wl_bss_info_t_size );

            for (i=0; i<response.response.overallStats.ulWifiScanApCount; i++)
            {
                int serverIdx = i + wifiScanResults;
                unsigned char *pTemp = (unsigned char*) pbssInfo;

                memset(bssPrintBuffer, 0, sizeof(bssPrintBuffer) );

                printf( "~WIFISCANRESULTS~");
                BWL_escanresults_print1( pbssInfo, bssPrintBuffer, sizeof(bssPrintBuffer), tempFilename );

                buffer_len = strlen( bssPrintBuffer );
                /* if the buffer ends with a newline character, remove the last character */
                if ( buffer_len && bssPrintBuffer[buffer_len-1] == '\n' )
                {
                    bssPrintBuffer[buffer_len-1] = 0;
                }

                line_count = BWL_count_lines(bssPrintBuffer, 120 );
                if (line_count == 0)
                {
                    line_count++;
                }
                printf( "<tr id=wifiscantextarea%ddr style=\"visibility:display;\" ><th align=left valign=top class=black_allborders220 >", serverIdx );
                printf( "<textarea id=wifiscantextarea%dd onclick=\"MyClick(event);\" cols=120 rows=%d style=\"border:none;\" >%s</textarea></th></tr>", serverIdx, line_count, bssPrintBuffer);
                pTemp += wl_bss_info_t_size;
                pbssInfo = pTemp;
            }
        }
    }

#endif /* BWL_SUPPORT */

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
        printf( "%lu", response.response.overallStats.contextSwitches );
        printf( "~" );
    }

    if ( DvfsControl == 1)
    {
        int       numCpusConf   = 0;

        numCpusConf = sysconf( _SC_NPROCESSORS_CONF );
        if (numCpusConf > BMEMPERF_MAX_NUM_CPUS)
        {
            numCpusConf = BMEMPERF_MAX_NUM_CPUS;
        }

        printf( "~DvfsControl~" );
        printf( "<table cols=9 style=\"border-collapse:collapse;\" border=0 cellpadding=3 >" );
        printf( "<tr><th colspan=9 class=whiteborders18 align=left >%s</th></tr>", "DVFS Controls" );

        printf( "<tr><th colspan=9 class=whiteborders18 align=left >%s</th></tr>", "Power Saving Techniques" );
        printf( "<tr style=\"outline: thin solid\" ><td colspan=9><table border=0 style=\"border-collapse:collapse;\" ><tr>");
        printf( "<td><input type=radio name=radioGovernor id=radioGovernor1 value=1 onclick=\"MyClick(event);\" >Conservative</td>" );
        printf( "<td width=50>&nbsp;</td>" ); /* spacer */
        printf( "<td><input type=radio name=radioGovernor id=radioGovernor2 value=2 onclick=\"MyClick(event);\" >Performance</td>" );
        printf( "<td width=50>&nbsp;</td>" ); /* spacer */
        printf( "<td><input type=radio name=radioGovernor id=radioGovernor3 value=3 onclick=\"MyClick(event);\" >Power Save</td>" );
        printf( "</tr></table></td></tr>" );

        printf( "</table>~" ); /* end DvfsControl */

        printf( "~GovernorSetting~%d~", get_governor_control( 0 ) );
    }

    if ( GovernorSetting )
    {
        printf( "~GovernorSetting~%d~", GovernorSetting );
        set_governor_control ( 0, GovernorSetting );
    }

    printf( "~STBTIME~%s~", DayMonDateYear( 0 ));

    printf( "~ALLDONE~" );

    return( 0 );
}                                                          /* main */

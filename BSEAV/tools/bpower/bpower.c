/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its licensors,
* and may only be used, duplicated, modified or distributed pursuant to the terms and
* conditions of a separate, written license agreement executed between you and Broadcom
* (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
* no license (express or implied), right to use, or waiver of any kind with respect to the
* Software, and Broadcom expressly reserves all rights in and to the Software and all
* intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
* secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
* and to use this information only in connection with your use of Broadcom integrated circuit products.
*
* 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
* AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
* WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
* THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
* OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
* LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
* OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
* USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
* LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
* EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
* USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
* ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
* LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
* ANY LIMITED REMEDY.
******************************************************************************/
#include "bmemperf_types64.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/utsname.h>
#include "bmemperf_info.h"
#include "bmemperf.h"
#include "bpower_utils.h"
#include "bmemperf_lib.h"
#include "bmemperf_utils.h"
#include "bthermal.h"

char         *g_client_name[BMEMPERF_MAX_NUM_CLIENT];
int           g_MegaBytes           = 0;                   /* set to 1 when user wants data displayed in megabytes instead of megabits (default) */
int           g_MegaBytesDivisor[2] = {1, 8};
char         *g_MegaBytesStr[2] = {"Mbps", "MBps"};
bmemperf_info g_bmemperf_info;

int main(
    void
    )
{
    char            *queryString     = NULL;
    int              clocks          = 0;
    int              GetThermal      = 0;
    int              GetThermalScale = BTHERMAL_CELCIUS;
    char             GetThermalScaleStr[2];
    int              getShrinkList   = 0;
    int              epochSeconds    = 0;
    int              tzOffset        = 0;
    int              DvfsControl     = 0;
    int              GovernorSetting = 0;
    bmemperf_version_info versionInfo;
    struct           utsname uname_info;
    BTHERMAL_DEVICE  bthermal_devices;

    memset( &bthermal_devices, 0, sizeof(bthermal_devices) );
    memset( &versionInfo, 0, sizeof( versionInfo ));

    printf( "Content-type: text/html%c%c", 10, 10 );

    queryString = getenv( "QUERY_STRING" );

    if (queryString && strlen( queryString ))
    {
        printf("~DEBUG~queryString (%s)~", queryString );
        scanForInt( queryString, "datetime=", &epochSeconds );
        scanForInt( queryString, "tzoffset=", &tzOffset );
        scanForInt( queryString, "clocks=", &clocks );
        scanForInt( queryString, "getshrinklist=", &getShrinkList );
        scanForInt( queryString, "GetThermal=", &GetThermal );
        scanForStr( queryString, "GetThermalScale=", sizeof(GetThermalScaleStr), GetThermalScaleStr );
        scanForInt( queryString, "DvfsControl=", &DvfsControl );
        scanForInt( queryString, "GovernorSetting=", &GovernorSetting );
        if (strcmp(GetThermalScaleStr, "F") == 0 )
        {
            GetThermalScale = BTHERMAL_FAHRENHEIT;
        }
        else
        {
            GetThermalScale = BTHERMAL_CELCIUS;
        }
        /*printf("~DEBUG~GetThermalScale (%u)~", GetThermalScale );*/
    }

    /* if browser provided a new date/time value; this only happens once during initialization */
    if (epochSeconds)
    {
        struct timeval now          = {1400000000, 0};

        strncpy( versionInfo.platform, getPlatform(), sizeof( versionInfo.platform ) - 1 );
        strncpy( versionInfo.platVersion, getPlatformVersion(), sizeof( versionInfo.platVersion ) - 1 );
        versionInfo.majorVersion   = MAJOR_VERSION;
        versionInfo.minorVersion   = MINOR_VERSION;
        printf( "~PLATFORM~%s", versionInfo.platform );
        printf( "~PLATVER~%s", versionInfo.platVersion );
        printf( "~variant~%s~", getProductIdStr() );
        printf( "~VERSION~Ver: %u.%u~", versionInfo.majorVersion, versionInfo.minorVersion );

        uname(&uname_info);
        printf("~UNAME~%d-bit %s %s~", (sizeof(char*) == 8)?64:32, uname_info.machine , uname_info.release );

        now.tv_sec = epochSeconds - ( tzOffset * 60 );
        settimeofday( &now, NULL );
        usleep( 200 );
        /*fflush(stdout);fflush(stderr);*/
    }

    /* if clock tree has been requested via the checkbox on the html page */
    if (clocks)
    {
        char *clockTree = NULL;

        clockTree = get_clock_tree( queryString );

        if (clockTree)
        {
            printf( "~CLOCKS~" );
            printf( "<table id=clocktable border=0 >" );
            printf( "  <tr id=clockrow1 ><th align=left ><table cols=6 border=0 ><tr><th valign=center align=left width=520 >" );
            printf( "  <span style=\"font-size:18.0pt;\" >Clock&nbsp;Tree</span></th>" );
            printf( "    <td align=right bgcolor=%s >OFF (saving power)</td><td width=30>&nbsp;</td>", COLOR_POWER_OFF );
            printf( "    <td align=right bgcolor=%s >ON (consuming power)</td>", COLOR_POWER_ON );
            printf( "  </tr></table>\n" );
            printf( "</th></tr>" );
            printf( "  <tr id=clockrow2 ><td align=left >%s</td></tr>", clockTree );
            printf( "</table>" );
            printf( "~" );

            FPRINTF( stderr, "(%p) free'ed ... %s\n", (void *)clockTree, __FUNCTION__ );
            Bsysperf_Free( clockTree );
        }
        else
        {
            /* we need to send something back to browser to force it to stop asking for a valid clock status */
            printf( "~CLOCKS~&nbsp;~" );
        }
    }

    if (getShrinkList)
    {
        char *shrinkList = NULL;

        shrinkList = get_shrink_list();

        if (shrinkList)
        {
            printf( "~SHRINKLIST~%s~", shrinkList );
            Bsysperf_Free( shrinkList );
        }
    }

    if ( GetThermal == 1 )
    {
        int tz = 0;
        int tp = 0;

        bthermal_get_information ( &bthermal_devices );

        printf( "~GetThermalSvg~");
        bthermal_output_svg_beginning( GetThermalScale );

        bthermal_output_tick_marks( GetThermalScale );

        for (tp=0; tp < bthermal_devices.thermal_zone[tz].num_trip_points; tp++)
        {
            bthermal_output_trip_point( tp, bthermal_devices.thermal_zone[tz].trip_point[tp].temperature / 1000, GetThermalScale );
        }

        bthermal_adjust_mercury( 0, GetThermalScale );

        bthermal_output_svg_ending();
        printf( "~");
        for (tp=0; tp < bthermal_devices.thermal_zone[tz].num_trip_points; tp++)
        {
            printf( "~GetThermalTripPoint%d~%u~", tp, X_COORDINATE_BASE + (
                bthermal_convert_to_scale(  bthermal_devices.thermal_zone[tz].trip_point[tp].temperature / 1000, GetThermalScale ) *
                bthermal_settings[GetThermalScale].degrees_per_pixel ) - 9 );
        }
    }

    if ( GetThermal == 2)
    {
        int currentTemperature = bthermal_get_temperature(0);
        float temp = bthermal_convert_to_scale( currentTemperature, GetThermalScale );
        int currentX = X_COORDINATE_BASE;
        printf("~DEBUG~ftemp (%5.1f) currentX %d; GetThermalScale %d; degrees_per_pixel %d ~", (temp + bthermal_settings[GetThermalScale].freezing),
               currentX, GetThermalScale, bthermal_settings[GetThermalScale].degrees_per_pixel );
        if ( temp > (X_COORDINATE_BASE - 9 ) )
        {
            currentX = X_COORDINATE_BASE + ( temp * bthermal_settings[GetThermalScale].degrees_per_pixel ) - 9;
        }
        else
        {
            currentX = currentTemperature;
        }
        printf( "~GetThermalTemperature~%d~", currentX );
    }

    if ( DvfsControl == 1)
    {
        int       cpu           = 0;
        int       numCpusConf   = 0;
        long int  cpu_freq_int  = 0;
        char      cpu_frequencies_supported[128];

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

        printf( "<tr><th colspan=9 class=whiteborders18 align=left >%s</th></tr>", "Frequencies Supported" );
        printf( "<tr bgcolor=lightgray style=\"outline: thin solid\" >");
        printf( "<td align=center style=\"border-right: 1px black solid;\" >CPU</td>");
        printf( "<td align=left style=\"border-right: 1px black solid;\" >Frequencies Supported</td>" );
        printf( "<td align=center > Current</td></tr>" );
        for (cpu = 0; cpu < numCpusConf; cpu++)
        {
            cpu_freq_int = get_cpu_frequency(cpu) * 1000;
            memset( cpu_frequencies_supported, 0, sizeof(cpu_frequencies_supported) );

            get_cpu_frequencies_supported( cpu, cpu_frequencies_supported, sizeof(cpu_frequencies_supported) );

            printf("<tr><td class=black_allborders align=center >%d</td><td class=black_allborders >%s</td>",
                    cpu, cpu_frequencies_supported );
            printf("<td class=black_allborders align=center >%ld</td></tr>", cpu_freq_int );
        }

        printf( "</table>~" ); /* end DvfsControl */

        printf( "~GovernorSetting~%d~", get_governor_control( 0 ) );
    }

    if ( GovernorSetting )
    {
        printf( "~GovernorSetting~%d~", GovernorSetting );
        set_governor_control ( 0, GovernorSetting );
    }

    printf( "~STBTIME~%s~", DayMonDateYear( 0 ));

    return( 0 );
} /* main */

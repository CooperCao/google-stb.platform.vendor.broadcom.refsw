/******************************************************************************
 * (c)2008-2015 Broadcom Corporation
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <fcntl.h>
#include "bmemperf_info.h"
#include "bmemperf.h"
#include "bpower_utils.h"
#include "bmemperf_utils.h"

char         *g_client_name[BMEMPERF_MAX_NUM_CLIENT];
int           g_MegaBytes           = 0;                   /* set to 1 when user wants data displayed in megabytes instead of megabits (default) */
int           g_MegaBytesDivisor[2] = {1, 8};
char         *g_MegaBytesStr[2] = {"Mbps", "MBps"};
bmemperf_info g_bmemperf_info;

int main(
    void
    )
{
    char *queryString   = NULL;
    int   clocks        = 0;
    int   getShrinkList = 0;
    int   epochSeconds  = 0;
    int   tzOffset      = 0;

    printf( "Content-type: text/html%c%c", 10, 10 );

    queryString = getenv( "QUERY_STRING" );

    if (queryString && strlen( queryString ))
    {
        scanForInt( queryString, "datetime=", &epochSeconds );
        scanForInt( queryString, "tzoffset=", &tzOffset );
        scanForInt( queryString, "clocks=", &clocks );
        scanForInt( queryString, "getshrinklist=", &getShrinkList );
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
            printf( "  <span style=\"font-size:18.0pt;\" >Power&nbsp;Management</span></th>" );
            printf( "    <td align=right bgcolor=%s >OFF (saving power)</td><td width=30>&nbsp;</td>", COLOR_POWER_OFF );
            printf( "    <td align=right bgcolor=%s >ON (consuming power)</td>", COLOR_POWER_ON );
            printf( "  </tr></table>\n" );
            printf( "</th></tr>" );
            printf( "  <tr id=clockrow2 ><td align=left >%s</td></tr>", clockTree );
            printf( "</table>" );
            printf( "~" );

            FPRINTF( stderr, "(%p) free'ed ... %s\n", (void *)clockTree, __FUNCTION__ );
            free( clockTree );
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
            free( shrinkList );
        }
    }

    return( 0 );
} /* main */

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
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include "bheaps.h"
#include "bstd.h"
#include "bmemperf_cgi.h"
#include "bmemperf_utils.h"
#include "bmemperf_lib.h"

/*
    echo core > /proc/bcmdriver/debug

    00:00:21.244 nexus_core: Core:
    00:00:21.244 nexus_core: heap offset memc size        MB vaddr     used peak mapping
    00:00:21.244 nexus_core: 0  0x010000000 0 0xd000000  208 0xa7667000  37% 37% DRIVER APP
    00:00:21.244 nexus_core: 1  0x030000000 0 0xa800000  168 0xcd000000  60% 60% SECURE
    00:00:21.244 nexus_core: 2  0x01d000000 0 0x86d0000  134 0x00000000  70% 70% MANAGED NOT_MAPPED
    00:00:21.244 nexus_core: 3  0x040000000 1 0x2000000   32 0xd7800000   0%  0% SECURE
    00:00:21.244 nexus_core: 5  0x042000000 1 0x1354a000 309 0x00000000  99% 99% MANAGED NOT_MAPPED
    00:00:21.244 nexus_core: 6  0x056000000 1 0x800000     8 0xa6e67000   0%  0% DRIVER APP
    00:00:21.244 nexus_core: 7  0x080000000 2 0x20000000 512 0x86e67000   3%  3% APP
    00:00:21.244 nexus_core: 8  0x0a0000000 2 0x13866000 312 0x00000000  99% 99% MANAGED NOT_MAPPED
    00:00:21.244 nexus_core: 9  0x0b4000000 2 0x500000     5 0x86967000   9%  9% DRIVER APP MANAGED
    00:00:21.244 nexus_memory: memory blocks: Allocated:0(0) Relocatable:0(0)
*/

char g_formatulStr[32];

/* this is used to help debug. the Y coordinate is used to draw SVG text strings for debug. */
static unsigned long int debug_y = 440;
/**
 *  Function: This function will output a rectangle using SVG HTML code. The rectangle is one of three MEMC
 *  regions. The addr field is used to determine where in the MEMC region the usage rectangle starts. The
 *  num_bytes field is used to determine how big the rectangle is. The usage_percentage is used to control a
 *  hash overlay to show what percentage the region was used during its peak usage.
 **/
int output_rect(
    unsigned long int memc,
    unsigned long int addr,
    unsigned long int num_bytes,
    unsigned long int usage_percentage,
    const char       *mappingStr
    )
{
    unsigned long int y1            = 0, y2 = 0;
    float             y1_percentage = 0, y2_percentage = 0.0;
    long unsigned int y_bottom      = 0;
    unsigned long int fillhgt       = debug_y;

    printf( "<!-- %s: memc %lu, addr %lx; bytes %lx; usage %ld -->\n", __FUNCTION__, memc, addr, num_bytes, usage_percentage );

    y1_percentage = ( 1.0 * addr - ( memc * ONE_GIGABYTE )) / ONE_GIGABYTE * MEMC_HEIGHT;
    y2_percentage = ( 1.0 * addr - ( memc * ONE_GIGABYTE ) + num_bytes ) / ONE_GIGABYTE * MEMC_HEIGHT;
    y1            = y1_percentage;
    if (y1 >= MEMC_HEIGHT)
    {
        y1 -= MEMC_HEIGHT;
    }
    y2 = y2_percentage;
    if (y2 >= MEMC_HEIGHT)
    {
        y2 -= MEMC_HEIGHT;
    }

    /*printf("%s: y1 (%lu); y2 (%lu); y1p (%02f2) y2p (%02f.2)\n", __FUNCTION__, y1, y2, y1_percentage, y2_percentage );*/
    printf( "<polygon points=\"0,%lu 200,%lu 200,%lu 0,%lu\" style=\"fill:#00FF00;stroke:black;stroke-width:1\"  fill=\"url(#diagonalHatch)\" />\n", y1, y1, y2, y2 );

    fillhgt = y2-y1;
    /*printf( "<text x=0 y=%lu fill=black style=\"font-size:10pt;\" >fill %lu</text>\n", debug_y, fillhgt ); debug_y+=20;*/
    fillhgt *= usage_percentage;
    fillhgt /= 100;
    /*printf( "<text x=0 y=%lu fill=black style=\"font-size:10pt;\" >fill %lu (usage %lu)</text>\n", debug_y, fillhgt, usage_percentage ); debug_y+=20;*/
    if (fillhgt > 0)
    {
        fillhgt -= 1; /* do not fill over the bottom line of the rectangle */
    }
    printf( "<rect x=\"1\" y=\"%lu\" width=\"198\" height=\"%lu\" style=\"fill:url(#diagonal-stripes-4-8);\" />\n", y1+1, fillhgt );

    printf( "<text x=0 y=%lu fill=black style=\"font-size:10pt;\" >0x%lx for 0x%lx</text>\n", y1+10, addr, num_bytes );

    /* if not enough room to display the text within the rectangle, display the bottom line just a bit lower than the top line */
    if (num_bytes < 0x1000000)
    {
        y_bottom = y1+19;
    }
    else
    {
        y_bottom = y2-1;
    }
    printf( "<text x=0 y=%lu fill=black style=\"font-size:10pt;\" >0x%lx</text><text x=80 y=%lu fill=black style=\"font-size:7pt;\" >%s</text>\n",
        y_bottom, addr + num_bytes, y_bottom, mappingStr );

    return( 0 );
} /* output_rect */

/**
 *  Function: This function will walk through the known heaps and determine how many heaps are active in this
 *  particular build. Could be 1, 2, or 3.
 **/
int get_num_memcs(
    bheap_info *pheap_info
    )
{
    unsigned int idx      = 0;
    unsigned int memc_max = 0;

    for (idx = 0; idx<BHEAP_MAX_HEAP_NUM; idx++)
    {
        if (pheap_info[idx].memc > memc_max)
        {
            memc_max = pheap_info[idx].memc;
        }
    }

    return( memc_max+1 );
}

/**
 *  Function: This function will walk through each of the known heaps and create a series of SVG rectangles
 *  depicting how much heap memory is used for each region.
 **/
int show_heaps(
    bheap_info *pheap_info
    )
{
    unsigned int memc      = 0, idx = 0;
    unsigned int num_memcs = get_num_memcs( pheap_info );

    printf( "<table cols=%u><tr>\n", num_memcs );
    for (memc = 0; memc < num_memcs; memc++)
    {
        printf( "<th align=center style=\"width:240px;\">MEMC %u</th>", memc );
    }
    printf( "</tr><tr>\n" );
    printf( "<td><pattern id=\"diagonalHatch\" width=\"10\" height=\"10\" patternTransform=\"rotate(45 0 0)\" patternUnits=\"userSpaceOnUse\">"
            "<line x1=\"0\" y1=\"0\" x2=\"0\" y2=\"10\" style=\"stroke:black; stroke-width:1\" /></pattern></td></tr><tr>\n" );
    for (memc = 0; memc < num_memcs; memc++)
    {
        printf( "<td><svg id=svgmemc%u width=200 height=%u style=\"border:1px solid black;\" >\n", memc, MEMC_HEIGHT );
        if (memc==0)
        {
            printf( "<defs><pattern id=\"diagonal-stripes-4-8\" x=\"0\" y=\"0\" width=\"8\" height=\"8\" patternUnits=\"userSpaceOnUse\" patternTransform=\"rotate(45)\">\n" );
            printf( "<rect x=\"0\" y=\"0\" width=\"4\" height=\"8\" style=\"stroke:none; fill:lightgray;\" /></pattern></defs>\n" );
        }
        for (idx = 0; idx<BHEAP_MAX_HEAP_NUM; idx++)
        {
            if (( pheap_info[idx].memc == memc ) && ( pheap_info[idx].size > 0 ))
            {
                output_rect( memc, pheap_info[idx].offset, pheap_info[idx].size, pheap_info[idx].peakPercentage, pheap_info[idx].mapping );
            }
        }
        printf( "</svg></td>\n" );
    }
    printf( "</tr></table>\n" );

    return( 0 );
} /* show_heaps */

/**
 *  Function: This function will issue the "echo core" command which causes the bcmdriver to output the usages
 *  for each heap region. Once the heap information has been created, this function will parse the strings of
 *  heap information and convert the string values to integer values.
 **/
int get_heap_info(
    bheap_info *pheap_info
    )
{
    char             *contents  = NULL;
    FILE             *fpInput   = NULL;
    char             *pos1      = NULL;
    unsigned int      heapCount = 0;
    struct stat       statbuf;
    char              systemCmd[NEXUS_LOG_FILE_FULL_PATH_LEN + TEMP_LOG_FILE_FULL_PATH_LEN + 3];
    unsigned int      heapIdx;
    unsigned long int offset;
    unsigned int      memc;
    unsigned long int size;
    unsigned long int megabytes;
    unsigned long int vaddr;
    unsigned int      usedPercentage;
    unsigned int      peakPercentage;
    char              mapping[32];
    char              nexusLogFilename[NEXUS_LOG_FILE_FULL_PATH_LEN];
    char              tempLogFilename[TEMP_LOG_FILE_FULL_PATH_LEN];


    if (pheap_info == NULL)
    {
        return( -1 );
    }

    sprintf( systemCmd, "echo core > /proc/bcmdriver/debug" );
    PRINTF( "system(%s)\n", systemCmd );
    system( systemCmd );
    usleep( 500 );

    PrependTempDirectory ( nexusLogFilename, sizeof( nexusLogFilename ), NEXUS_LOG_FILE_NAME );
    PrependTempDirectory ( tempLogFilename, sizeof( tempLogFilename ), TEMP_LOG_FILE_NAME );
    sprintf( systemCmd, "cp %s %s", nexusLogFilename, tempLogFilename );
    PRINTF( "system(%s)\n", systemCmd );
    system( systemCmd );
    usleep( 100 );

    if (lstat( tempLogFilename, &statbuf ) == -1)
    {
        printf( "~ALERT~Could not find file %s.\n\n\nA Nexus app needs to be running for Heap Status to work.\n\n~", nexusLogFilename );
        return( -1 );
    }

    PRINTF( "Size of (%s) is %lu\n", tempLogFilename, (unsigned long int) statbuf.st_size );
    contents = malloc( statbuf.st_size );
    if (contents == NULL) {return( -1 ); }

    fpInput = fopen( tempLogFilename, "r" );
    fread( contents, 1, statbuf.st_size, fpInput );
    fclose( fpInput );

    pos1 = contents;

    while (pos1 < ( contents + statbuf.st_size )) {
        if ((( pos1[0] == 'n' ) && ( pos1[1] == 'e' ) && ( pos1[2] == 'x' ) && ( pos1[3] == 'u' ) && ( pos1[4] == 's' ) && ( pos1[5] == '_' ) &&
             ( pos1[6] == 'c' ) && ( pos1[7] == 'o' ) && ( pos1[8] == 'r' )) ||
            (( pos1[0] == 'n' ) && ( pos1[1] == 'e' ) && ( pos1[2] == 'x' ) && ( pos1[3] == 'u' ) && ( pos1[4] == 's' ) && ( pos1[5] == '_' ) &&
             ( pos1[6] == 'm' ) && ( pos1[7] == 'e' ) && ( pos1[8] == 'm' )))
        {
            char *pos3 = pos1;

            PRINTF( "found (%s", pos1 );
            pos3 += strlen( "nexus_core: " );
            while (pos3 < ( contents + statbuf.st_size )) {
                if (( pos3[0] == '0' ) && ( pos3[1] == 'x' ))
                {
                    /* nexus_core:  9  0x0b4000000 2 0x500000     5 0x8693d000   9%  9% DRIVER APP MANAGED */
                    /* at this point, we have gone past the heap index 0 .. 10; backup two spaces and 1 or 2 digits */
                    pos3 -= 2; /* backup past two spaces */
                    pos3--;    /* backup to the 1st digit */
                    pos3--;    /* backup to the 2nd digit */
                    if (( *pos3 != '0' ) && ( *pos3 != '1' ))
                    {
                        pos3++;
                    }
                    PRINTF( " %s)\n", pos3 );
                    sscanf( pos3, "%u %lx %u %lx %lu %lx %u%% %u%% %s", &heapIdx, &offset, &memc, &size, &megabytes, &vaddr, &usedPercentage, &peakPercentage, &mapping[0] );
                    PRINTF( "scanned:            %u  0x%09lx %u 0x%-8lx %lu 0x%8lx  %u%% %u%% %s\n", heapIdx, offset, memc, size, megabytes, vaddr, usedPercentage, peakPercentage, mapping );

                    /* if the parsed index matches the expected index */
                    if (heapIdx < BHEAP_MAX_HEAP_NUM)
                    {
                        char *pos4 = NULL;
                        pheap_info[heapIdx].offset         = offset;
                        pheap_info[heapIdx].memc           = memc;
                        pheap_info[heapIdx].size           = size;
                        pheap_info[heapIdx].megabytes      = megabytes;
                        pheap_info[heapIdx].vaddr          = vaddr;
                        pheap_info[heapIdx].usedPercentage = usedPercentage;
                        pheap_info[heapIdx].peakPercentage = peakPercentage;

                        /* the mapping string could have spaces in it; find the beginning of the string and copy until the null terminator */
                        pos4 = strstr( pos3, mapping );
                        if (pos4)
                        {
                            PRINTF( "pos4 (%s): mapping (%s); strncpy max %u bytes\n", pos4, mapping, sizeof( pheap_info[heapIdx].mapping ));
                            strncpy( pheap_info[heapIdx].mapping, pos4, sizeof( pheap_info[heapIdx].mapping ));
                        }
                    }
                    else
                    {
                        printf( "Scanned heap index (%u) exceeds max heap index (%u)\n", heapIdx, BHEAP_MAX_HEAP_NUM );
                    }

                    heapCount++;

                    break;
                }
                pos3++;

                /* start next line search where this one left off */
                pos1 = pos3;
            }
        }

        pos1++;
    }

    Bsysperf_Free( contents );

    return( 0 );
} /* get_heap_info */

/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#include "bmon_types64.h"
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
#include <time.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/utsname.h>
#include <stdbool.h>
#include <sys/klog.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <locale.h>

#include "bmon_utils.h"
#include "interrupt.h"
#include "bmon_interrupt.h"

int bmon_interrupt_get_counts(
    bmon_interrupt_t *interrupt_data
    )
{
    unsigned int      interrupt_idx = 0;
    unsigned int      num_lines     = 0;
    struct stat       statbuf;
    char             *contents      = NULL;
    char             *posstart      = NULL;
    char             *posend        = NULL;
    unsigned long int lineNum       = 0;
    unsigned long int cpu           = 0;
    unsigned int      numProcessors = sysconf( _SC_NPROCESSORS_ONLN );
    unsigned int      numCpusConf   = sysconf( _SC_NPROCESSORS_CONF );
    unsigned int      numCpusActive = 0;

    setlocale( LC_NUMERIC, "" );

    memset( &statbuf, 0, sizeof( statbuf ));

    contents = bmon_get_file_contents_proc( BMON_PROC_INTERRUPTS_FILE, BMON_INTERRUPT_MAX_PROC_FILE_SIZE );
    if (contents == NULL)
    {
        printf( "could not open (%s) \n", BMON_PROC_INTERRUPTS_FILE );
        return( -1 );
    }

    posstart = contents;                                   /* start parsing the file at the beginning of the file */

    /* step through the file line by line, searching for interrupt counts for each CPU */
    do {
        posend = strstr( posstart, "\n" );
        if (posend)
        {
            *posend = '\0';
            posend++;
        }
        /*printf("next line (%s); posend %p\n", posstart, posend );*/

        num_lines++;

        if (lineNum == 0)
        {
            char *cp, *restOfLine;
            restOfLine = posstart;

            /*printf("numProcessors %u\n", numProcessors);*/
            while (( cp = strstr( restOfLine, "CPU" )) != NULL && numCpusActive < numProcessors)
            {
                cpu = strtoul( cp + 3, &restOfLine, BMON_INTERRUPT_VALUE_LENGTH );
                interrupt_data->cpu[cpu].active = true;
                numCpusActive++;
            }
            /*fprintf( stderr, "found %u cpus in header; numProcessors %u\n", numCpusActive, numProcessors );*/
        }
        else if (strlen( posstart ))
        {
            char        *cp         = NULL;
            char        *restOfLine = NULL;
            char        *pos        = NULL;

            interrupt_idx     = lineNum - 1;         /* the first line has the header on it */

            if (interrupt_idx < BMON_INTERRUPTS_MAX_TYPES)
            {
                /* Skip over "interrupt_NAME:" */
                cp = strchr( posstart, ':' );
                if (!cp)
                {
                    continue;
                }

                cp++;

                pos  = cp;
                pos += numCpusActive * ( BMON_INTERRUPT_VALUE_LENGTH + 1 ); /* each number is 10 digits long separated by 1 space */
                pos += 2;                                                 /* after all of the numbers are listed, the name is separated by 2 more spaces */

                /* some names have a few spaces at the beginning of them; advance the pointer past all of the beginning spaces */
                while (*pos == ' ')
                {
                    pos++;
                }

                /* the line is long enough to have a name at the end of it */
                if (pos < ( cp + strlen( cp )))
                {
                    strncpy( interrupt_data->interrupt[interrupt_idx].name, pos, sizeof( interrupt_data->interrupt[interrupt_idx].name ));
                    PRINTF( "%s: added new interrupt_ to idx %2u (%s)\n", __FUNCTION__, interrupt_idx, pos );
                }

                PRINTF( "line %3u: (%s) ", num_lines, cp );
                for (cpu = 0; cpu < numCpusConf; cpu++)
                {
                    unsigned long int value = 0;

                    /* only scan for values if the cpu is online ... could have 4 CPUs in system with only 2 active */
                    if ( interrupt_data->cpu[cpu].active ) {
                        /* parse the next value from the current line */
                        value = strtoul( cp, &restOfLine, BMON_INTERRUPT_VALUE_LENGTH );

                        /* add interrupt count to the running value for this CPU */
                        interrupt_data->cpu[cpu].accumulated_count += value;

                        /* save the value for this specific interrupt_ */
                        interrupt_data->interrupt[interrupt_idx].cpu_count[cpu] = value;
                        PRINTF( "%lu ", value );

                        cp = restOfLine;
                        if (cp == NULL)
                        {
                            break;
                        }
                    }
                }
                PRINTF( "\n" );
            }
            else
            {
                fprintf( stderr, "%s: interrupt_idx (%u) exceeded BMON_INTERRUPTS_MAX_TYPES\n", __FUNCTION__, interrupt_idx );
            }
        }

        posstart = posend;

        lineNum++;
    } while (posstart != NULL);

    FREE_SAFE( contents );

    /* add up all of the cpu's interrupts into one big total */
    for (interrupt_idx= 0; interrupt_idx < BMON_INTERRUPTS_MAX_TYPES; interrupt_idx++)
    {
        for (cpu = 0; cpu < numCpusActive; cpu++)
        {
            interrupt_data->interrupt_total += interrupt_data->interrupt[interrupt_idx].cpu_count[cpu];
        }
    }
    PRINTF( "after %u lines, total %lu\n\n\n", num_lines, interrupt_data->interrupt_total );

    return( 0 );
}                                                          /* bmon_interrupt_get_counts */

int bmon_get_proc_stat_info(
    bmon_interrupt_t *interrupt_data
    )
{
    unsigned long int interrupt_total = 0;
    FILE             *fpStats  = NULL;
    char              buf[1024];
    char             *pos = NULL;

    if ( interrupt_data == NULL)
    {
        return( -1 );                                      /* invalid parameter */
    }

    fpStats = fopen( BMON_PROC_STAT_FILE, "r" );
    if (fpStats != NULL)
    {
        while (fgets( buf, sizeof( buf ), fpStats ))
        {
            pos = strchr( buf, '\n' );
            if (pos)
            {
                *pos = 0;                                  /* null-terminate the line */
            }
            /*fprintf( stderr, "%s: line (%s) \n", __FUNCTION__, pos );*/
            if (strncmp( buf, "intr ", 5 ) == 0)
            {
                /* just read the first number. it contains the total number of interrupts since boot */
                sscanf( buf + 5, "%u", &( interrupt_data->interrupt_total ));
                /*fprintf( stderr, "%s: interrupt_total %u\n", __FUNCTION__, interrupt_data->interrupt_total );*/
            }
            else if (strncmp( buf, "ctxt ", 5 ) == 0)
            {
                sscanf( buf + 5, "%u", &( interrupt_data->context_switches ));
                /*fprintf( stderr, "%s: context_switches %u\n", __FUNCTION__, interrupt_data->context_switches );*/
                break;                                     /* after finding context switches, do not need to look any further in the file */
            }
        }

        fclose( fpStats );
    }
    else
    {
        fprintf( stderr, "Could not open %s\n", BMON_PROC_STAT_FILE );
    }
    return( interrupt_total );
}                                                          /* bmon_get_proc_stat_info */

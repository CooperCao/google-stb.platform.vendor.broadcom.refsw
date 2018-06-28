/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#include "bmon_cpu_get_frequency.h"

/**
 *  Function: This function will return the frequency for the specified CPU.
 *            You can use this command to list the current frequency:
 *            $ cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq
 **/
static int bmon_cpu_get_frequency( unsigned int cpuId)
{
    long int    freq = 0;
    FILE       *fp   = NULL;
    struct stat statbuf;
    char        cpuinfo_cur_freq[64];

    memset( &statbuf, 0, sizeof( statbuf ));

    sprintf( cpuinfo_cur_freq, "/sys/devices/system/cpu/cpu%u/cpufreq/cpuinfo_cur_freq", cpuId );

    /*printf( "%s:%u - trying lstat(%s)\n", __FUNCTION__, __LINE__, cpuinfo_cur_freq );*/
    if (( lstat( cpuinfo_cur_freq, &statbuf ) == -1 ) || ( statbuf.st_size == 0 ))
    {
        /*PRINTF( "%s:%u - lstat(%s) failed; %s\n", __FUNCTION__, __LINE__, cpuinfo_cur_freq, strerror( errno ));*/
        return( freq );
    }

    fp = fopen( cpuinfo_cur_freq, "r" );
    if (fp)
    {
        int  num_bytes = 0;
        char freq_buffer[32];
        memset( freq_buffer, 0, sizeof( freq_buffer ));

        num_bytes = fread( freq_buffer, 1, sizeof( freq_buffer ) - 1, fp );
        /*PRINTF( "%s:%u - fread() returned num_bytes %d \n", __FUNCTION__, __LINE__, num_bytes );*/
        if (num_bytes)
        {
            sscanf( freq_buffer, "%ld", &freq );
            /*PRINTF( "%s:%u - sscanf(%s) returned freq %ld \n", __FUNCTION__, __LINE__, freq_buffer, freq );*/
            freq /= 1000;
        }
        fclose( fp );
    }

    return( freq );
}                                                          /* bmon_cpu_get_frequency */

/**
 *  Function: This function will loop through all processors in the system
 *  and look for the CPU frequency associated each one. For older versions
 *  of BOLT (pre v1.20) and kernel versions before 4.1-1.3, this CPU frequency
 *  data will not be found in the /sys file system.
 **/
int bmon_cpu_get_frequencies( unsigned int *pcpu_data )
{
    int numCpusConf = 0;
    int cpu         = 0;

    numCpusConf = sysconf( _SC_NPROCESSORS_CONF );
    if (numCpusConf > BMON_CPU_MAX_NUM )
    {
        numCpusConf = BMON_CPU_MAX_NUM;
    }

    for (cpu = 0; cpu < numCpusConf; cpu++)
    {
        pcpu_data[cpu] = bmon_cpu_get_frequency( cpu ) * 1000;
    }

    return( numCpusConf );
}                                                          /* bmon_cpu_get_frequencies */

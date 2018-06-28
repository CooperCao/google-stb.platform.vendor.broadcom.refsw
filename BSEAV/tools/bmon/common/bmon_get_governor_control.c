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
#include "bmon_get_governor_control.h"
#include "bmon_get_file_contents_proc.h"
#include "bmon_utils.h"

/**
 *  Function: This function will read the current scaling governor from the /sys
 *            file system. The string value will be converted to an internal
 *            enum and returned to the caller.
 *            File: /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
 **/
DVFS_GOVERNOR_TYPES bmon_get_governor_control ( int cpu )
{
    DVFS_GOVERNOR_TYPES value = DVFS_GOVERNOR_PERFORMANCE;
    char  filename[128];
    char *contents = NULL;

    memset(filename, 0, sizeof(filename));

    sprintf( filename, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_governor", cpu );
    contents = bmon_get_file_contents_proc( filename, 128 );

    /* Hosahalli - The following command will list all the available governors. We do not want to list
       the "userspace" in your tool. First reason is we have not defined what that should be. Linux
       kernel CPUfreq framework makes this available as standard offering.
          cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors
          conservative ondemand userspace powersave performance
    */
    if ( contents )
    {
        /*printf("for filename (%s); contents (%s)\n", filename, contents );*/
        if ( strstr(contents, "conservative" ) )
        {
            value = DVFS_GOVERNOR_CONSERVATIVE;
        }
        else if ( strstr(contents, "performance" ) )
        {
            value = DVFS_GOVERNOR_PERFORMANCE;
        }
        else if ( strstr(contents, "powersave" ) )
        {
            value = DVFS_GOVERNOR_POWERSAVE;
        }
        else if ( strstr(contents, "ondemand" ) )
        {
            value = DVFS_GOVERNOR_ONDEMAND;
        }
        FREE_SAFE( contents );
    }

    return ( value );
}

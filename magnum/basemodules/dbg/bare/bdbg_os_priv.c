/***************************************************************************
 * Copyright (C) 2006-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "b_bare_os.h"

b_bare_os_tick g_init_timestamp;
b_bare_os_lock g_mutex;

void
BDBG_P_InitializeTimeStamp(void)
{
    g_init_timestamp = pb_bare_os->current_tick();
}

void
BDBG_P_GetTimeStamp_isrsafe(char *timeStamp, int size_t)
{
    b_bare_os_tick currentTime;
    int hours, minutes, seconds;
    int milliseconds;
    int rc;

    currentTime = pb_bare_os->current_tick();
    if (currentTime > g_init_timestamp) {
        milliseconds = pb_bare_os->tick_diff(currentTime, g_init_timestamp);
    }
    else {
        milliseconds = pb_bare_os->tick_diff(g_init_timestamp, currentTime);
    }

    /* Calculate the time   */
    hours = milliseconds/(1000*60*60);
    minutes = milliseconds/(1000*60)%60;
    seconds = milliseconds/1000%60;

    /* print the formatted time including the milliseconds  */
    rc = BKNI_Snprintf(timeStamp, size_t, "%02u:%02u:%02u.%03u", hours, minutes, seconds, milliseconds);
    return;

}

BERR_Code BDBG_P_OsInit(void)
{
    g_mutex = pb_bare_os->lock_create();
    return 0;
}

void BDBG_P_OsUninit(void)
{
    pb_bare_os->lock_destroy(g_mutex);
}

/* NOTE: this function is called from both magnum task and isr context */
void BDBG_P_Lock_isrsafe(void)
{
    int rc;
    rc = pb_bare_os->lock_acquire(g_mutex);
    BDBG_ASSERT(0 == rc);
    return;
}

/* NOTE: this function is called from both magnum task and isr context */
void BDBG_P_Unlock_isrsafe(void)
{
    pb_bare_os->lock_release(g_mutex);
    return;
}

/* End of file */

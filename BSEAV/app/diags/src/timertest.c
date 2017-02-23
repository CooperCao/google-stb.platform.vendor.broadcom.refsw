/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description:
 *
 ***************************************************************************/
#include <stdio.h>
#include "test.h"
#include "bstd.h"
#include "bkni.h"
#include "btmr.h"
#include "nexus_base_types.h"
#include "prompt.h"
#include "nexus_base.h"
#include "priv/nexus_core.h"

#define MAX_PHYSICAL_TIMERS 14

static int count[MAX_PHYSICAL_TIMERS];

static void myIntHandler(void *p1, int p2) 
{
    if (p2<MAX_PHYSICAL_TIMERS)
        count[p2]++;
    printf("timer %d interrupt:  %d\n", p2, count[p2]);
}

static int get_number(void)
{
    char buf[256];
    fgets(buf, 256, stdin);
    return atoi(buf);
}

void bcmTimerTest (void)
{
    BERR_Code rc;
    BTMR_TimerHandle timer[MAX_PHYSICAL_TIMERS];
    BTMR_TimerSettings timerSettings = { BTMR_Type_ePeriodic, (BTMR_CallbackFunc)myIntHandler, NULL, 0, false };
    static int current_timer=0;
    static int running[MAX_PHYSICAL_TIMERS];
    int i;
    int num_timers_created=0;

    for (i=0; i<MAX_PHYSICAL_TIMERS; i++) {
        running[i]=0;
        count[i]=0;
    }

    for (i=0; i<MAX_PHYSICAL_TIMERS; i++) {
        timerSettings.parm2 = i;
        rc = BTMR_CreateTimer(g_pCoreHandles->tmr, &timer[i], &timerSettings);
        if (rc == BERR_SUCCESS) {
            printf("created timer %d\n", i);
            num_timers_created++;
        }
        else
        {
            printf("error creating timer %d\n", i);
            break;
        }
    }

    while (1)
    {
        printf("\n\n");
        printf("===================\n");
        printf("  Timer Test Menu  \n");
        printf("===================\n");
        printf("    0) Exit\n");
        printf("    1) %s timer\n", running[current_timer]==0 ? "Start" : "Stop");
        printf("    2) Change timer number (current channel=%d)\n", current_timer);

        switch(Prompt()) {
            case 0:
                for (i=0; i<num_timers_created; i++) {
                    if (running[i]) {
                        printf("stopping timer %d...\n", i);
                        BTMR_StopTimer(timer[i]);
                        running[i]=0;
                    }
                    printf("destroying timer %d...\n", i);
                    BTMR_DestroyTimer(timer[i]);
                }
                return;

            case 1:
                if (running[current_timer]==0) {
                    count[current_timer]=0;
                    printf("starting timer %d...\n", current_timer);
                    BTMR_StartTimer(timer[current_timer], 1*1000*1000); /* start with a timeout of 1,000,000 usec or 1,000 millisecond or 1 second*/
                    running[current_timer]=1;
                }
                else {
                    printf("stopping timer %d...\n", current_timer);
                    BTMR_StopTimer(timer[current_timer]);
                    running[current_timer]=0;
                }
                break;

            case 2:
                if (num_timers_created > 1) {
                    printf("Enter timer number (0 to %d):  ", num_timers_created-1);
                    current_timer = get_number();
                }
                else {
                    printf("Only one timer created\n");
                }
                break;

            default:
                break;
        }
    }

}

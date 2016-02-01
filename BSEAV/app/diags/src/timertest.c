/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include <stdio.h>
#include "test.h"
#include "bstd.h"
#include "bkni.h"
#include "btmr.h"
#include "nexus_base_types.h"
#include "prompt.h"
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

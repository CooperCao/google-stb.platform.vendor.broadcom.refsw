/***************************************************************************
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
 *
 * Module Description:
 *   The source code for the stub ikos server.
 *
 ***************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include "utils.h"
#include "bchp_sun_top_ctrl.h"
#define P_UNUSED(x) ((void)x)

/* Environment and blocked signal state for restoration in a signal handler. */
static sigjmp_buf sj_env;

static void sigbus_hdl (int sig, siginfo_t *siginfo, void *ptr);


void jump_setup (void)
{
    struct sigaction act;
    memset (&act, 0, sizeof(act));
    act.sa_sigaction = sigbus_hdl;
    act.sa_flags = SA_SIGINFO;

    if (sigaction(SIGBUS, &act, 0)) {
        perror ("sigaction");
        exit (1);
    }

    if (sigaction(SIGSEGV, &act, 0)) {
        perror ("sigaction");
        exit (1);
    }
}

unsigned long deref32 (
    unsigned long seq, uintptr_t addr, unsigned long offset,
    unsigned long* regval)
{
    if (sigsetjmp(sj_env, 1))
    {
        *regval =  0xDEADC0DE;
    }
    else
    {
        *regval = *((volatile unsigned long *)
            (offset + ((addr & BTST_REG_OFFSET_MASK) |
              (BCHP_SUN_TOP_CTRL_CHIP_FAMILY_ID & (~BTST_REG_OFFSET_MASK)))));
    }

    return seq + 1;
}

unsigned long deref64 (
    unsigned long seq, unsigned long addr, void* offset, uint64_t* regval)
{
    if (sigsetjmp(sj_env, 1))
    {
        *regval =  0xDEADC0DE;
    }
    else
    {
        *regval = *((volatile uint64_t*)
            ((unsigned long)offset + (addr & BTST_REG_OFFSET_MASK)));
    }

    return seq + 1;
}

static void sigbus_hdl (int sig, siginfo_t *siginfo, void *ptr)
{
    /* Jump (goto) to the saved program state where we don't use mmapped()
     * memory. */
P_UNUSED(sig);
P_UNUSED(siginfo);
P_UNUSED(ptr);
    siglongjmp (sj_env, 1);
}

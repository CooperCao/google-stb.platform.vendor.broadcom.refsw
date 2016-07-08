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
 *****************************************************************************/

/*
 * ioctl.cpp
 *
 *  Created on: Jan 29, 2015
 *      Author: gambhire
 */

#include <cstdint>
#include "tzerrno.h"

#include "svcutils.h"
#include "tztask.h"

#include "ioctl.h"
#include "termios.h"
#include "tzioc.h"

void SysCalls::doIoctl(TzTask *currTask) {
    unsigned long fd = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);
    unsigned long arg2 = currTask->userReg(TzTask::UserRegs::r2);
    unsigned long arg3 = currTask->userReg(TzTask::UserRegs::r3);
    int rv = 0;

    UNUSED(arg3);
    // printf("%s: fd 0x%lx 0x%lx 0x%lx 0x%lx\n", __FUNCTION__, fd, arg1, arg2, arg3);

    if (fd == TzIoc::TZIOC_DEV_DEFAULT_FD)
    {
        rv = TzIoc::ioctl(fd, arg1, arg2);
        currTask->writeUserReg(TzTask::UserRegs::r0, rv);
    }
    else if (fd > 4)
    {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EBADF);
    }
    else // (fd <= 3) => termios
    {
        switch (arg1) {
        case TCGETS:
            if (fd > 0) {
                struct termios tios;
                tios.c_iflag = 0;
                tios.c_oflag = ONLCR;
                tios.c_cflag = CLOCAL;
                tios.c_lflag = ICANON;
                tios.__c_ospeed = 115200;
                tios.__c_ispeed = 115200;

                struct termios *userTios = (struct termios *)arg2;
                copyToUser(userTios, &tios);
                currTask->writeUserReg(TzTask::UserRegs::r0, 0);
                return;
            }
            else { // (fd == 0)
                struct termios tios;
                tios.c_iflag = IGNBRK | IGNPAR;
                tios.c_oflag = 0;
                tios.c_cflag = CLOCAL;
                tios.c_lflag = ICANON;
                tios.__c_ospeed = 115200;
                tios.__c_ispeed = 115200;

                struct termios *userTios = (struct termios *)arg2;
                copyToUser(userTios, &tios);
                currTask->writeUserReg(TzTask::UserRegs::r0, 0);
                return;
            }
            break;

        default:
            currTask->writeUserReg(TzTask::UserRegs::r0, -ENOTTY);
            break;
        }
    }
}

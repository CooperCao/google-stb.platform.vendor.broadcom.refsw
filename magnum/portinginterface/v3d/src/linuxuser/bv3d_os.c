/***************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 **************************************************************************/

#include "bstd.h"

#include <time.h>

#include "../bv3d_priv.h"

#if BCHP_CHIP == 11360

#include <fcntl.h>
#include <sys/ioctl.h>

#define BRCM_IOCTL_V3D_SOFT_RESET \
        _IO(101, 99)

#endif

BDBG_MODULE(BV3D);

BERR_Code BV3D_P_GetTime_isrsafe(uint64_t *pMicroseconds)
{
   struct timespec tp;

   if (clock_gettime(CLOCK_MONOTONIC, &tp) == 0)
   {
      uint64_t res = 1000000 * (uint64_t)tp.tv_sec;
      res += tp.tv_nsec / 1000;
      *pMicroseconds = res;

      return BERR_SUCCESS;
   }
   else
   {
      *pMicroseconds = 0;
      return BERR_OS_ERROR;
   }
}

BERR_Code BV3D_P_OsInit(BV3D_Handle hV3d)
{
#if BCHP_CHIP == 11360
   hV3d->hFd = open("/dev/brcm0", O_RDWR | O_SYNC);
   if (hV3d->hFd < 0)
      return BERR_TRACE(BERR_OS_ERROR);
#else
   BSTD_UNUSED(hV3d);
#endif
   return BERR_SUCCESS;
}

void BV3D_P_OsUninit(BV3D_Handle hV3d)
{
   if (hV3d->hFd >= 0)
   {
      close(hV3d->hFd);
      hV3d->hFd = -1;
   }
}

#if BCHP_CHIP == 11360
void BV3D_P_OsSoftReset (BV3D_Handle hV3d)
{
   if (hV3d->hFd < 0)
      return;
   ioctl(hV3d->hFd, BRCM_IOCTL_V3D_SOFT_RESET, 0);
}
#endif

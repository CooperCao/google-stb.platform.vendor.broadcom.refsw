/******************************************************************************
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
 ******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "../src/bvc5_bin_pool_priv.h"

#include <time.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

BDBG_MODULE(BVC5);

BERR_Code BVC5_P_GetTime_isrsafe(uint64_t *pMicroseconds)
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

void BVC5_P_DebugDumpHeapContents(uint64_t ulOffset, unsigned uSize, uint32_t uiCoreIndex)
{
   FILE          *fp;

   BSTD_UNUSED(uiCoreIndex);

   fp = fopen("memdump.bin", "w");
   if (fp != NULL)
   {
      int mem_fd = 0;

      mem_fd = open("/dev/mem", O_RDONLY);
      if (mem_fd)
      {
         uint32_t            offset = ulOffset;
         size_t              chunk = 4 * 1024 * 1024;

         BKNI_Printf("\nDumping heap memory to memdump.bin ...\n");

         while (offset < ulOffset + uSize)
         {
            void *mem_p;
            size_t blockSz = chunk;

            if (ulOffset + uSize - offset < chunk)
               blockSz = ulOffset + uSize - offset;

            mem_p = mmap(NULL, blockSz, PROT_READ, MAP_SHARED, mem_fd, offset);
            if (mem_p != NULL)
               fwrite(mem_p, 1, blockSz, fp);
            else
               BKNI_Printf("Error while mapping heap\n");

            munmap(mem_p, blockSz);

            offset += chunk;
         }

         BKNI_Printf("Dumping memory done\n");

         close(mem_fd);
      }

      fclose(fp);
   }
}

/* Only Android without MMU combination supports programable bin pools */
BVC5_BinPoolBlock_MemInterface *BVC5_P_GetBinPoolMemInterface(void)
{
   return NULL;
}

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

/*
 * C90 is not compatible with the 4.1 Linux uerspace headers, which use
 * "long long". Also they use trailing "," at the end of enumerations lists
 * which triggers a pedantic warning. So use some GCC magic to hide the
 * warnings.
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Wpedantic"

#include "brcmv3d_drm.h"

#pragma GCC diagnostic pop

/*
 * Singleton filedescriptor to access the DRM device.
 */
static int drmFd = -1;

void BVC5_P_DRMOpen(uint32_t uiDRMDevice)
{
   if (drmFd < 0)
   {
      char drmDeviceName[128];
      sprintf(drmDeviceName,"/dev/dri/card%u", uiDRMDevice);
      drmFd = open(drmDeviceName, O_RDWR);
   }
}

bool BVC5_P_HasBrcmv3dko(void)
{
   return (drmFd != -1);
}

void BVC5_P_DRMTerminateClient(uint64_t uiPlatformToken)
{
   if (BVC5_P_HasBrcmv3dko())
   {
      int res;
      struct drm_v3d_file_private_token s;

      s.token = uiPlatformToken;
      res = ioctl(drmFd, DRM_IOCTL_V3D_SET_CLIENT_TERM, &s);
      BDBG_ASSERT(res == 0);
   }
}

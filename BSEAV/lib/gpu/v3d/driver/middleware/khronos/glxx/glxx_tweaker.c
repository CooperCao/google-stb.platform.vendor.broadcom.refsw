/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2014 Broadcom.  All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

#include "middleware/khronos/glxx/glxx_tweaker.h"
#include "interface/khronos/common/khrn_options.h"
#include "interface/vcos/vcos.h"

void glxx_tweaker_init(TWEAK_STATE_T *ts, bool es2)
{
   ts->tweaks_disabled = khrn_options.disable_tweaks;
   ts->es2 = es2;
   ts->depthmask = true;
   ts->depthfunc = GL_LESS;
   ts->blend_enabled = false;
   ts->source_matches = false;
}

bool glxx_tweaker_update(TWEAK_STATE_T *ts)
{
   if (ts->tweaks_disabled)
      return false;

   if (ts->es2 && ts->source_matches && !ts->depthmask && ts->depthfunc == GL_EQUAL && ts->blend_enabled)
      return true;

   return false;
}

void glxx_tweaker_setdepthmask(TWEAK_STATE_T *ts, bool m)
{
   ts->depthmask = m;
}

void glxx_tweaker_setdepthfunc(TWEAK_STATE_T *ts, GLenum func)
{
   ts->depthfunc = func;
}

void glxx_tweaker_setblendenabled(TWEAK_STATE_T *ts, bool enabled)
{
   ts->blend_enabled = enabled;
}

void glxx_tweaker_setshadersource(TWEAK_STATE_T *ts, GLsizei count, const char **string, const GLint *length)
{
   if (ts->tweaks_disabled)
      return;

   ts->source_matches = false;

   if (count == 1 && length == NULL)
   {
      size_t len = 0;
      if (string && string[0])
         len = strlen(string[0]);

      if (len == 191)
      {
         uint32_t i;
         uint32_t sum = 0;

         /* Simple checksum */
         for (i = 0; i < len; i++)
            sum += string[0][i];

         if (sum == 0x38A7)
            ts->source_matches = true;
      }
   }
}
/***************************************************************************
 *     Broadcom Proprietary and Confidential. (c)2014 Broadcom.  All rights reserved.
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
#include <cutils/log.h>
#include "default_android.h"

void __attribute__ ((constructor)) v3d_android_load(void);
void __attribute__ ((destructor)) v3d_android_unload(void);

/* These functions are defined in nexus_egl_client.cpp (part of libnexuseglclient.so) */
void* EGL_nexus_join(char *client_process_name);
void EGL_nexus_unjoin(void *nexus_client);

static RSOANPL_PlatformHandle s_platformHandle;
static void *nexus_client = NULL;

__attribute__((visibility("default")))
void *v3d_get_nexus_client_context(void)
{
   return nexus_client;
}

/* Called:
 *  1) automatically when the library is loaded and before dlopen() returns.
 *  2) explicitely when the the library is dynamically loaded by third party module.
 */
__attribute__((visibility("default")))
void v3d_android_load(void)
{
   FILE *fp;
   char procid[256];
   pid_t current_pid = getpid();
   nexus_client = EGL_nexus_join("libegl_nexus");
   if (nexus_client == NULL)
   {
      ALOGE("EGL_nexus_join [CONSTRUCTOR] Failed\n");
      goto error0;
   }

   RSOANPL_RegisterAndroidDisplayPlatform(&s_platformHandle, NULL);

error0:
   return;
}

/* Called:
 *  1) automatically when the library is unloaded and before dlclose().
 *  2) explicitely when the the library is dynamically unloaded by third party module.
 */
__attribute__((visibility("default")))
void v3d_android_unload(void)
{
   /* as surfaceflinger is active for the entire lifetime, its probably not possible
      for this to be called with platform_type == 1 */

   RSOANPL_UnregisterAndroidDisplayPlatform(s_platformHandle);

   EGL_nexus_unjoin(nexus_client);
   nexus_client = NULL;
}

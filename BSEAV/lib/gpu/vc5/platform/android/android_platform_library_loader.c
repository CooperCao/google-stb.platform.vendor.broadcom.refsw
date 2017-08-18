/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <cutils/log.h>
#include "default_android.h"

void __attribute__ ((constructor)) v3d_android_load(void);
void __attribute__ ((destructor)) v3d_android_unload(void);

/* These functions are defined in libnxwrap. */
void* nxwrap_create_client(void **wrap);
void nxwrap_destroy_client(void *wrap);

static RSOANPL_PlatformHandle s_platformHandle;
static void *nexus_client = NULL;
static void *nxwrap = NULL;

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
   nexus_client = nxwrap_create_client(&nxwrap);
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

   nxwrap_destroy_client(nxwrap);
   nxwrap = NULL;
}

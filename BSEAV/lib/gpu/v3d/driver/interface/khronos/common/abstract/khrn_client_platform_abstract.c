/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <stdlib.h>

#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_int_parallel.h"
#include "interface/khronos/common/khrn_client_platform.h"
#include "interface/khronos/common/khrn_client.h"
#include "interface/khronos/egl/egl_client_config.h"
#include "middleware/khronos/egl/egl_server.h"
#include "interface/vcos/vcos.h"
#include "middleware/khronos/common/khrn_hw.h"
#include "interface/khronos/common/khrn_int_ids.h"

#include "interface/khronos/common/khrn_client_platform.h"
#include "interface/khronos/common/abstract/khrn_client_platform_abstract.h"

// Client is single-threaded on direct platform so no mutex stuff necessary

VCOS_STATUS_T platform_tls_create(PLATFORM_TLS_T *tls, void (*destructor)(void*))
{
   return vcos_tls_create(tls, destructor);
}

void platform_tls_destroy(PLATFORM_TLS_T tls)
{
   vcos_tls_delete(tls);
}

void platform_tls_set(PLATFORM_TLS_T tls, void *v)
{
   if (vcos_tls_set(tls, v) != VCOS_SUCCESS)
   {
      ANDROID_LOGD("platform_tls_set() : FAILED (key %p), (v %p)\n", (void *)tls, v);
   }
}

void *platform_tls_get(PLATFORM_TLS_T tls)
{
   void * tsd = vcos_tls_get(tls);
   if (tsd == NULL)
   {
      ANDROID_LOGD("platform_tls_get() : creating TLS block, (tid = %d) (pid = %d)\n", gettid(), getpid());

      if (client_thread_attach(tls))
         tsd = vcos_tls_get(tls);
      else
         ANDROID_LOGD("client_thread_attach() : FAILED\n");
   }

   if (tsd == NULL)
   {
      ANDROID_LOGD("***************************************************************************\n");
      ANDROID_LOGD("ERROR : TLS = NULL, (tid = %d) (pid = %d)\n", gettid(), getpid());
      ANDROID_LOGD("***************************************************************************\n");
   }

   return tsd;
}

void platform_tls_remove(PLATFORM_TLS_T tls)
{
   vcos_tls_set(tls, NULL);
}

bool platform_match_pixmap_api_support(EGLNativePixmapType pixmap, uint32_t api_support)
{
   UNUSED(pixmap);
   UNUSED(api_support);

   return true;
}

uint32_t platform_memcmp(const void * aLeft, const void * aRight, size_t aLen) { return khrn_par_memcmp(aLeft, aRight, aLen); }
void platform_memcpy(void * aTrg, const void * aSrc, size_t aLength) { khrn_par_memcpy(aTrg, aSrc, aLength); }

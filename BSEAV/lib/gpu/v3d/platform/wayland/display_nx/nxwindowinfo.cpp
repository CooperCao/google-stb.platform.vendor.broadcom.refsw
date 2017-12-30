/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "nxwindowinfo.h"

#include <stdio.h>

#include "nexus_surface_client.h"
#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#endif

namespace wlpl
{

bool NxWindowInfo::Init()
{
   printf("%s: BEGIN\n", __FUNCTION__);
#ifdef NXCLIENT_SUPPORT
   NxClient_AllocSettings allocSettings;
   NxClient_GetDefaultAllocSettings(&allocSettings);
   allocSettings.surfaceClient = 1;
   int rc = NxClient_Alloc(&allocSettings, &m_allocResults);
   if (rc)
   {
      printf("%s: NxClient_Alloc failed!\n", __FUNCTION__);
      rc = BERR_TRACE(rc);
      return false;
   }

   /* Attach the surface client for this swapChain. There is one swapChain per native window, so
    * we will have one client per native window. */
   m_clientID = m_allocResults.surfaceClient[0].id;
   printf("%s: m_allocResults.surfaceClient[0].id: %u\n\n", __FUNCTION__, m_clientID);
#else
   m_clientID = m_info.clientID;
   printf("%s: m_info.clientID: %u\n\n", __FUNCTION__, m_clientID);
#endif

   m_surfaceClient = NEXUS_SurfaceClient_Acquire(m_clientID);
   if (!m_surfaceClient)
   {
      printf("Failed to acquire compositing client surface for client id %d",
            m_clientID);
#ifdef NXCLIENT_SUPPORT
      NxClient_Free(&m_allocResults);
#endif
      return false;
   }

#ifdef NXCLIENT_SUPPORT
   NEXUS_SurfaceComposition comp;
   NxClient_GetSurfaceClientComposition(m_clientID, &comp);
   comp.colorBlend = m_info.colorBlend;
   comp.alphaBlend = m_info.alphaBlend;
   NxClient_SetSurfaceClientComposition(m_clientID, &comp);
#endif

   return true;
}

void NxWindowInfo::Term()
{
   NEXUS_SurfaceClient_Release(m_surfaceClient);

#ifdef NXCLIENT_SUPPORT
   NxClient_Free(&m_allocResults);
#endif
}

}

/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#include <stdio.h>

#include "windowinfo.h"

#include "nexus_surface_client.h"
#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#endif

namespace nxpl
{

bool NativeWindowInfo::Init()
{
#ifdef NXCLIENT_SUPPORT
   NxClient_AllocSettings allocSettings;
   NxClient_GetDefaultAllocSettings(&allocSettings);
   allocSettings.surfaceClient = 1;
   int rc = NxClient_Alloc(&allocSettings, &m_allocResults);
   if (rc)
   {
       rc = BERR_TRACE(rc);
       return false;
   }

   /* Attach the surface client for this swapChain. There is one swapChain per native window, so
      we will have one client per native window. */
   m_clientID = m_allocResults.surfaceClient[0].id;
#else
   m_clientID = m_info.clientID;
#endif

   m_surfaceClient = NEXUS_SurfaceClient_Acquire(m_clientID);
   if (!m_surfaceClient)
   {
      printf("Failed to acquire compositing client surface for client id %d", m_clientID);
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

void NativeWindowInfo::Term()
{
   NEXUS_SurfaceClient_Release(m_surfaceClient);

   if (m_videoClient)
      NEXUS_SurfaceClient_Release(m_videoClient);

#ifdef NXCLIENT_SUPPORT
   NxClient_Free(&m_allocResults);
#endif
}

}

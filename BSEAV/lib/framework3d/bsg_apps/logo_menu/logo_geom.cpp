/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "logo_text.c"
#include "logo_pulse.c"
#include "logo_geom.h"
#include "logo_menu.h"
#include "bsg_geometry.h"
#include "bsg_surface.h"
#include "bsg_material.h"

using namespace bsg;

static uint32_t Offset(uint32_t n)
{
   return sizeof(float) * n;
}

GeometryHandle BroadcomLogo::InstanceLogo(MaterialHandle textMat, MaterialHandle pulseMat, bool reflected)
{
   GeometryHandle geom(New);

   if (m_textSurfHandle.IsNull())
   {
      // Make the text geom
      m_textSurfHandle = SurfaceHandle(New);

      m_textSurfHandle->SetDraw(GL_TRIANGLE_STRIP, sizeof(g_index_logo_text) / sizeof(uint16_t),
                                                   sizeof(g_vert_logo_text), g_vert_logo_text,
                                                   sizeof(g_index_logo_text), g_index_logo_text);

      m_textSurfHandle->SetPointer(EffectSemantics::eVATTR_POSITION,  GLVertexPointer(3, GL_FLOAT, 8 * sizeof(float), Offset(0)));
      m_textSurfHandle->SetPointer(EffectSemantics::eVATTR_NORMAL,    GLVertexPointer(3, GL_FLOAT, 8 * sizeof(float), Offset(3)));
      m_textSurfHandle->SetPointer(EffectSemantics::eVATTR_TEXCOORD1, GLVertexPointer(2, GL_FLOAT, 8 * sizeof(float), Offset(6)));

      m_textSurfHandle->SetBound(Bound(sqrtf((4 * 4) + (2 * 2) + (2 * 2)) , Vec3(0.0f, 0.0f, 0.0f)));
   }

   geom->AppendSurface(m_textSurfHandle,  textMat);

   if (m_pulseSurfHandle.IsNull())
   {
      m_pulseSurfHandle = SurfaceHandle(New);

      m_pulseSurfHandle->SetDraw(GL_TRIANGLE_STRIP, sizeof(g_index_logo_pulse) / sizeof(uint16_t),
                                                    sizeof(g_vert_logo_pulse),  g_vert_logo_pulse,
                                                    sizeof(g_index_logo_pulse), g_index_logo_pulse);

      m_pulseSurfHandle->SetPointer(EffectSemantics::eVATTR_POSITION,  GLVertexPointer(3, GL_FLOAT, 8 * sizeof(float), Offset(0)));
      m_pulseSurfHandle->SetPointer(EffectSemantics::eVATTR_NORMAL,    GLVertexPointer(3, GL_FLOAT, 8 * sizeof(float), Offset(3)));
      m_pulseSurfHandle->SetPointer(EffectSemantics::eVATTR_TEXCOORD1, GLVertexPointer(2, GL_FLOAT, 8 * sizeof(float), Offset(6)));

      m_pulseSurfHandle->SetBound(Bound(sqrtf((4 * 4) + (2 * 2) + (2 * 2)) , Vec3(0.0f, 0.0f, 0.0f)));
   }

   geom->AppendSurface(m_pulseSurfHandle, pulseMat);

   geom->SetReflected(reflected);

   return geom;
}

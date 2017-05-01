/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bsg_semantic_data.h"
#include "bsg_application.h"
#include "bsg_exception.h"

namespace bsg
{

SemanticData::SemanticData() :
   m_mvDirty(true),
   m_vpDirty(true),
   m_mvpDirty(true),
   m_imtDirty(true),
   m_ivtDirty(true),
   m_imvtDirty(true),
   m_opacity(1.0f)
{
}

float SemanticData::GetFloat(EffectSemantics::eSemantic semantic) const
{
   switch (semantic)
   {
   case EffectSemantics::eSCALAR_OPACITY :
      return GetOpacity();
      break;
   default :
      BSG_THROW("Invalid semantic");
      break;
   }

   return 1.0f;
}

const Vec4 &SemanticData::GetVec4(EffectSemantics::eSemantic semantic) const
{
   static Vec4 def;

   switch (semantic)
   {
   case EffectSemantics::eVECTOR4_SCREEN_SIZE :
      return GetScreenSize();
      break;

   default:
      BSG_THROW("Invalid semantic");
      break;
   }

   return def;
}

const Mat4 &SemanticData::GetMat4(EffectSemantics::eSemantic semantic) const
{
   static Mat4 def;

   switch (semantic)
   {
   case EffectSemantics::eMATRIX4_MODEL :
      return m_modelMx;
      break;
   case EffectSemantics::eMATRIX4_VIEW :
      return m_viewMx;
      break;
   case EffectSemantics::eMATRIX4_PROJECTION :
      return m_projMx;
      break;
   case EffectSemantics::eMATRIX4_MODEL_VIEW :
      return GetModelViewMatrix();
      break;
   case EffectSemantics::eMATRIX4_VIEW_PROJECTION :
      return GetViewProjectionMatrix();
      break;
   case EffectSemantics::eMATRIX4_MODEL_VIEW_PROJECTION :
      return GetModelViewProjectionMatrix();
      break;
   default :
      BSG_THROW("Invalid semantic");
      break;
   }

   return def;
}

const Mat3 &SemanticData::GetMat3(EffectSemantics::eSemantic semantic) const
{
   static Mat3 def;

   switch (semantic)
   {
   case EffectSemantics::eMATRIX3_INVT_MODEL :
      return GetInvTModelMatrix();
      break;
   case EffectSemantics::eMATRIX3_INVT_VIEW :
      return GetInvTViewMatrix();
      break;
   case EffectSemantics::eMATRIX3_INVT_MODEL_VIEW :
      return GetInvTModelViewMatrix();
      break;
   default :
      BSG_THROW("Invalid semantic");
      break;
   }

   return def;
}

const Vec4 &SemanticData::GetScreenSize() const
{
   const Application &app = *Application::Instance();

   float width  = (float)app.GetWindowWidth();
   float height = (float)app.GetWindowHeight();

   m_screenSize = Vec4(width, height, 1.0f / width, 1.0f / height);

   return m_screenSize;
}

void SemanticData::SetModelMatrix(const Mat4 &mx)
{
   if (m_modelMx != mx)
   {
      m_modelMx = mx;
      m_mvDirty  = true;
      m_mvpDirty = true;
      m_imtDirty  = true;
      m_ivtDirty  = true;
      m_imvtDirty = true;
   }
}

void SemanticData::SetViewMatrix(const Mat4 &mx)
{
   if (m_viewMx != mx)
   {
      m_viewMx    = mx;
      m_mvDirty   = true;
      m_vpDirty   = true;
      m_mvpDirty  = true;
      m_ivtDirty  = true;
      m_imvtDirty = true;
   }
}

void SemanticData::SetModelViewMatrix(const Mat4 &mx)
{
   if (m_modelViewMx != mx)
   {
      m_modelViewMx = mx;
      m_mvpDirty    = true;
      m_imvtDirty   = true;
   }
}

void SemanticData::SetProjMatrix(const Mat4 &mx)
{
   if (m_projMx != mx)
   {
      m_projMx   = mx;
      m_vpDirty  = true;
      m_mvpDirty = true;
   }
}

}

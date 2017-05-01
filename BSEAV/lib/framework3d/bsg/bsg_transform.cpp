/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bsg_transform.h"

namespace bsg
{

Transform::Transform() :
   m_dirty(false),
   m_referenced(false)
{
   m_scale    = Vec3(1.0f, 1.0f, 1.0f);
   m_position = Vec3(0.0f, 0.0f, 0.0f);
}

const Mat4 &Transform::GetMatrix() const
{
   // If it has been changed (dirty) or has exposed itself (referenced) then we need to recalculate
   if (m_dirty || m_referenced)
   {
      m_matrix = Translate(m_position.X(), m_position.Y(), m_position.Z());

      if (m_rotation != Quaternion())
      {
         Mat4 rot;
         m_rotation.AsMatrix(&rot);
         m_matrix = m_matrix * rot;
      }

      if (m_scale != Vec3(1.0f))
         m_matrix = m_matrix * Scale(m_scale.X(), m_scale.Y(), m_scale.Z());

      m_dirty    = false;
   }

   return m_matrix;
}

void Transform::PreRotate(const bsg::Quaternion &val)
{
   m_dirty = true;
   m_rotation = m_rotation * val;
}

void Transform::PreRotate(float degrees, const Vec3 &axis)
{
   m_dirty = true;
   m_rotation = m_rotation * Quaternion(degrees, axis);
}

void Transform::PreScale(const Vec3 &vec)
{
   m_dirty = true;
   m_scale[0] *= vec[0];
   m_scale[1] *= vec[1];
   m_scale[2] *= vec[2];
}

void Transform::PostTranslate(const Vec3 &vec)
{
   m_dirty = true;
   m_position[0] += vec[0];
   m_position[1] += vec[1];
   m_position[2] += vec[2];
}

}

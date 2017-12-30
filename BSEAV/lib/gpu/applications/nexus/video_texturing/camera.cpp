/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "camera.h"

#include <math.h>
#include <algorithm>

namespace video_texturing
{

static float DegToRad(float deg) { return deg * (M_PI / 180.0f); }
static float RadToDeg(float rad) { return rad * (180.0f / M_PI); }

void Camera::SetPosition(const ESVec3 &pos)
{
   m_position = pos;
   m_dirty = true;
}

void Camera::AdjustPosition(const ESVec3 &posOff)
{
   ESVec3 p { m_position.v[0] + posOff.v[0],
              m_position.v[1] + posOff.v[1],
              m_position.v[2] + posOff.v[2] };
   SetPosition(p);
}

void Camera::SetRotation(float yawDeg, float pitchDeg, float rollDeg)
{
   m_yaw   = DegToRad(yawDeg);
   m_pitch = DegToRad(pitchDeg);
   m_roll  = DegToRad(rollDeg);
   m_dirty = true;
}

void Camera::AdjustRotation(float yawDeg, float pitchDeg, float rollDeg)
{
   m_yaw   += DegToRad(yawDeg);
   m_pitch += DegToRad(pitchDeg);
   m_roll  += DegToRad(rollDeg);
   m_dirty = true;
}

void Camera::AdjustRotation(const ESVec3 &ypr)
{
   AdjustRotation(ypr.v[0], ypr.v[1], ypr.v[2]);
}

float Camera::GetYaw() const
{
   return RadToDeg(m_yaw);
}

float Camera::GetPitch() const
{
   return RadToDeg(m_pitch);
}

float Camera::GetRoll() const
{
   return RadToDeg(m_roll);
}

void Camera::SetFov(float fovDeg)
{
   m_fov   = fovDeg;
   m_dirty = true;
}

void Camera::AdjustFov(float fovDeg)
{
   SetFov(m_fov + fovDeg);
}

const ESMatrix &Camera::GetViewMatrix()
{
   if (!m_dirty)
      return m_viewMatrix;

   ESVec3 right, front, up;
   ESVec3 worldUp{ { 0, 1, 0 } };

   // The right axis comes from yaw & roll angles
   right.v[0] = cosf(m_yaw) * cosf(m_roll);
   right.v[1] = sinf(m_roll);
   right.v[2] = sinf(m_yaw) * cosf(m_roll);
   right = esNormalize(right);

   // calculate front vector perpendicular to right and worldUp
   front = esNormalize(esCross(right, worldUp));

   // rotate front vector according to pitch
   front = esNormalize(esRotateVec(front, right, m_pitch));

   // calculate the camera up vector perpendicular to right and front
   up = esNormalize(esCross(front, right));

   m_viewMatrix.m[0][0] = right.v[0];
   m_viewMatrix.m[1][0] = right.v[1];
   m_viewMatrix.m[2][0] = right.v[2];
   m_viewMatrix.m[3][0] = m_position.v[0];

   m_viewMatrix.m[0][1] = up.v[0];
   m_viewMatrix.m[1][1] = up.v[1];
   m_viewMatrix.m[2][1] = up.v[2];
   m_viewMatrix.m[3][1] = m_position.v[1];

   m_viewMatrix.m[0][2] = front.v[0];
   m_viewMatrix.m[1][2] = front.v[1];
   m_viewMatrix.m[2][2] = front.v[2];
   m_viewMatrix.m[3][2] = m_position.v[2];

   m_viewMatrix.m[0][3] = 0.0f;
   m_viewMatrix.m[1][3] = 0.0f;
   m_viewMatrix.m[2][3] = 0.0f;
   m_viewMatrix.m[3][3] = 1.0f;

   m_dirty = false;
   return m_viewMatrix;
}

} // namespace

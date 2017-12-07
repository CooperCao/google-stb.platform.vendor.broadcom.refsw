/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/
#pragma once

#include "esutil.h"

#include <math.h>

namespace video_texturing
{

// Encapsulates camera properties and calculates view matrix
class Camera
{
public:
   Camera() :
      m_yaw(0.0f), m_pitch(0.0f), m_roll(0.0f), m_fov(70.0f),
      m_viewMatrix(), m_dirty(true)
   {
      m_position.v[0] = 0; m_position.v[1] = 0; m_position.v[2] = 0;
   };

   const ESMatrix &GetViewMatrix();

   void SetPosition(const ESVec3 &pos);
   void AdjustPosition(const ESVec3 &posOff);

   void SetRotation(float yawDeg, float pitchDeg, float rollDeg);    // Set absolutes
   void AdjustRotation(float yawDeg, float pitchDeg, float rollDeg); // Offset values
   void AdjustRotation(const ESVec3 &yawPitchRoll);                  // Offset values
   float GetYaw() const;   // In degrees
   float GetPitch() const; // In degrees
   float GetRoll() const;  // In degrees

   void  SetFov(float fovDeg);
   void  AdjustFov(float fovDeg);
   float GetFov() const { return m_fov; } // In degrees

private:
   ESVec3  m_position;                 // camera position in 3D space

   float   m_yaw;                      // camera horizontal rotation in radians
   float   m_pitch;                    // camera vertical rotation in radians
   float   m_roll;                     // camera roll in radians
   float   m_fov;                      // field of view in degrees

   ESMatrix m_viewMatrix;              // last calculated view matrix
   bool     m_dirty;                   // do we need to recalc matrix?
};

} // namespace

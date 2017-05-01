/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bsg_camera.h"
#include "bsg_matrix.h"
#include "bsg_application.h"

#include <algorithm>
#include <stdio.h>
namespace bsg
{

const float RADTODEG = 180.0f / (float)M_PI;
const float DEGTORAD = (float)M_PI / 180.0f;

Camera::Camera() :
   m_type(Camera::ePERSPECTIVE),
   m_eyeSepAbs(-1.0f),
   m_eyeSepDivider(20.0f),
   m_yFov(60.0f),
   m_aspectRatio(1.0f),
   m_nearClippingPlane(1.0f),
   m_farClippingPlane(100.0f),
   m_focalPlane(33.0f),
   m_callback(0)
{
}

void Camera::MakeProjectionMatrix(Mat4 *proj) const
{
   if (m_type == Camera::eORTHOGRAPHIC)
   {
      float xFov = XFovOrtho();
      *proj = Ortho(-xFov * 0.5f, xFov * 0.5f, -m_yFov * 0.5f, m_yFov * 0.5f,
                    m_nearClippingPlane, m_farClippingPlane);
   }
   else if (m_type == Camera::ePERSPECTIVE)
   {
      *proj = Perspective(m_yFov, m_aspectRatio, m_nearClippingPlane, m_farClippingPlane);
   }
}

float Camera::GetEyeSeparation() const
{
   if (m_eyeSepAbs > 0.0f)
      return m_eyeSepAbs;
   else
      return m_nearClippingPlane / m_eyeSepDivider;
}

void Camera::SetEyeSeparationAbsolute(float value)
{
   m_eyeSepAbs = value;
}

void Camera::SetEyeSeparationNearDistDivideFactor(float divider /*= 20.0f*/)
{
   m_eyeSepDivider = divider;
   m_eyeSepAbs = -1.0f;
}

void Camera::MakeLeftEyeProjectionMatrix(Mat4 *proj) const
{
   if (m_type == Camera::eORTHOGRAPHIC)
   {
      float xFov = XFovOrtho();
      *proj = Ortho(-xFov * 0.5f, xFov * 0.5f, -m_yFov * 0.5f, m_yFov * 0.5f,
                    m_nearClippingPlane, m_farClippingPlane);
   }
   else if (m_type == Camera::ePERSPECTIVE)
   {
      float eyeSep   = GetEyeSeparation();
      float xFov     = XFovPersp();
      float halfXFov = DEGTORAD * xFov * 0.5f;
      float wd2      = m_nearClippingPlane * tan(halfXFov);
      float ndfl     = m_nearClippingPlane / -m_focalPlane;

      float left   = -wd2 - 0.5f * eyeSep * ndfl;
      float right  =  wd2 - 0.5f * eyeSep * ndfl;
      float top    =  wd2 / m_aspectRatio;
      float bottom = -wd2 / m_aspectRatio;

      *proj = Frustum(left, right, bottom, top, m_nearClippingPlane, m_farClippingPlane);
   }
}

void Camera::MakeRightEyeProjectionMatrix(Mat4 *proj) const
{
   if (m_type == Camera::eORTHOGRAPHIC)
   {
      float xFov = XFovOrtho();
      *proj = Ortho(-xFov * 0.5f, xFov * 0.5f, -m_yFov * 0.5f, m_yFov * 0.5f,
                     m_nearClippingPlane, m_farClippingPlane);
   }
   else if (m_type == Camera::ePERSPECTIVE)
   {
      float eyeSep   = GetEyeSeparation();
      float xFov     = XFovPersp();
      float halfXFov = DEGTORAD * xFov * 0.5f;
      float wd2      = m_nearClippingPlane * tan(halfXFov);
      float ndfl     = m_nearClippingPlane / -m_focalPlane;

      float left   = -wd2 + 0.5f * eyeSep * ndfl;
      float right  =  wd2 + 0.5f * eyeSep * ndfl;
      float top    =  wd2 / m_aspectRatio;
      float bottom = -wd2 / m_aspectRatio;

      *proj = Frustum(left, right, bottom, top, m_nearClippingPlane, m_farClippingPlane);
   }
}

//////////////////////////////////////////////////////////////////////////////////

Transform CameraTransformHelper::Lookat(const Vec3 &cameraPos, const Vec3 &target, const Vec3 &upVec)
{
   Vec3 def(0.0, 0.0, -1.0f);
   Vec3 dir = Normalize(target - cameraPos);
   Vec3 rotAxis = Normalize(Cross(def, dir));
   Vec3 up = Normalize(upVec);

   if (rotAxis == Vec3(0.0f, 0.0f, 0.0f))
      rotAxis = Vec3(-1.0f, 0.0f, 0.0f);

   float dotp = def.Dot(dir);
   dotp = std::min(dotp, 1.0f);
   dotp = std::max(dotp, -1.0f);
   float angle = RADTODEG * acosf(dotp);

   // Work out the new default up vector
   Quaternion q(angle, rotAxis);
   Mat4 mx;
   q.AsMatrix(&mx);

   Vec4 defUp4 = mx * Vec4(0, 1, 0, 0);
   Vec3 defUp(defUp4[0], defUp4[1], defUp4[2]);

   // Project up onto defUp
   Vec3 projUp = Normalize(up - (dir * up.Dot(dir)));

   // And then the angle to rotate around the direction vector
   dotp = projUp.Dot(defUp);
   dotp = std::min(dotp, 1.0f);
   dotp = std::max(dotp, -1.0f);
   float yaw = -RADTODEG * acosf(dotp);

   Transform tf;
   tf.SetRotation(q);
   tf.SetPosition(cameraPos);
   tf.PreRotate(yaw, def);

   return tf;
}

void Camera::GetViewVolume(ViewVolume *frustumPtr) const
{
   if (frustumPtr == 0)
      return;

   // TODO: This view volume is based on a whole screen viewport.  By rights it should
   // use the viewport to adjust the view volume.

   ViewVolume &frustum = *frustumPtr;

   if (m_type == eORTHOGRAPHIC)
   {
      float yFov2 = m_yFov * 0.5f;
      float xFov2 = XFovOrtho() * 0.5f;

      frustum[ViewVolume::eLEFT]   = Plane( 1.0f,  0.0f,  0.0f, xFov2, true);
      frustum[ViewVolume::eRIGHT]  = Plane(-1.0f,  0.0f,  0.0f, xFov2, true);
      frustum[ViewVolume::eBOTTOM] = Plane( 0.0f,  1.0f,  0.0f, yFov2, true);
      frustum[ViewVolume::eTOP]    = Plane( 0.0f, -1.0f,  0.0f, yFov2, true);
   }
   else
   {
      float yFov2 = tan(m_yFov * 0.5f * DEGTORAD) * m_nearClippingPlane;
      float xFov2 = yFov2 * m_aspectRatio;

      float farOverNear = m_farClippingPlane / m_nearClippingPlane;

      Vec3  nearBL(-xFov2, -yFov2, -m_nearClippingPlane);
      Vec3  nearTL(-xFov2,  yFov2, -m_nearClippingPlane);
      Vec3  nearTR( xFov2,  yFov2, -m_nearClippingPlane);
      Vec3  nearBR( xFov2, -yFov2, -m_nearClippingPlane);
      Vec3  farBL(nearBL * farOverNear);
      Vec3  farTL(nearTL * farOverNear);
      Vec3  farTR(nearTR * farOverNear);
      Vec3  farBR(nearBR * farOverNear);

      frustum[ViewVolume::eLEFT]   = Plane(nearBL, farBL, nearTL);
      frustum[ViewVolume::eTOP]    = Plane(nearTL, farTL, nearTR);
      frustum[ViewVolume::eRIGHT]  = Plane(nearTR, farTR, nearBR);
      frustum[ViewVolume::eBOTTOM] = Plane(nearBR, farBR, nearBL);
   }

   // Positive distance means outside the frustum plane.
   frustum[ViewVolume::eFARTHEST]  = Plane( 0.0f,  0.0f,  1.0f,  m_farClippingPlane,  true);      // true => normalized already
   frustum[ViewVolume::eNEAREST]   = Plane( 0.0f,  0.0f, -1.0f, -m_nearClippingPlane, true);
}

float Camera::XFovPersp() const
{
   return 2.0f * RADTODEG * atan(tan(0.5f * m_yFov * DEGTORAD) * m_aspectRatio);
}

}

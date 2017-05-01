/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_CAMERA_H__
#define __BSG_CAMERA_H__

#include "bsg_common.h"
#include "bsg_animatable.h"
#include "bsg_library.h"
#include "bsg_transform.h"
#include "bsg_plane.h"

#include <string>
#include <vector>

namespace bsg
{

class Mat4;
class Camera;

// @cond
struct CameraTraits
{
   typedef Camera       Base;
   typedef Camera       Derived;
   typedef CameraTraits BaseTraits;
};
// @endcond

//! @addtogroup handles
//! @{
typedef Handle<CameraTraits>     CameraHandle;
//! @}

/** @addtogroup scenegraph
@{
*/

//! Camera callbacks are invoked when events pertaining to the camera happen
class CameraCallback
{
public:
   virtual ~CameraCallback() {}

   //! Called by bsg when the view is calculated.  Use this callback if you need to know the
   //! camera's state e.g. to set up a light.
   virtual void OnViewTransform(const Mat4 & /*view*/, const Mat4 &/*iview*/)  =0;
};

//! The camera object encapsulates all the physical aspects of the camera, except for position and
//! orientation. The scene graph nodes contain the camera, and the position and orientation are
//! therefore defined by its position in the scene graph, and the transforms in its node and those
//! above it.
class Camera : public RefCount
{
   friend class Handle<CameraTraits>;

public:
   //! The type of camera
   enum eType
   {
      ORTHOGRAPHIC,     //!< \deprecated Use eORTHOGRAPHIC
      PERSPECTIVE,      //!< \deprecated Use ePERSPECTIVE

      eORTHOGRAPHIC = ORTHOGRAPHIC,
      ePERSPECTIVE  = PERSPECTIVE
   };

   virtual ~Camera() { delete m_callback; }

   //! Get the previously set camera name
   const std::string &GetName() const     { return m_name; }

   //! Set a name for the camera
   void SetName(const std::string &val)   { m_name = val;  }

   //! Return the type of the camera
   eType GetType()                  const { return m_type; }

   //! Set the type of the camera
   void SetType(eType val)                { m_type = val;  }

   //! Return the vertical field-of-view (in degrees for perspective camera, distance for orthographic).
   //! The field-of-view is animatable.
   AnimatableFloat &GetYFov()             { return m_yFov; }

   //! Return the vertical field-of-view (in degrees for perspective camera, distance for orthographic).
   //! The field-of-view is animatable.
   const AnimatableFloat &GetYFov() const { return m_yFov; }

   //! Set the vertical field-of-view (in degrees for perspective camera, distance for orthographic).
   void SetYFov(float val)                { m_yFov.Set(val);  }

   //! Get the camera aspect ratio (width / height).
   //! The aspect ratio is animatable.
   AnimatableFloat &GetAspectRatio()             { return m_aspectRatio; }

   //! Get the camera aspect ratio (width / height).
   //! The aspect ratio is animatable.
   const AnimatableFloat &GetAspectRatio() const { return m_aspectRatio; }

   //! Set the camera aspect ratio (width / height).
   void SetAspectRatio(float val)                { m_aspectRatio.Set(val);  }

   //! Set the camera aspect ratio using width & height.
   void SetAspectRatio(uint32_t w, uint32_t h)   { SetAspectRatio((float) w / (float) h); }

   //! Get the near clipping plane distance from the camera.
   //! The near clipping plane is animatable.
   AnimatableFloat &GetNearClippingPlane()             { return m_nearClippingPlane; }

   //! Get the near clipping plane distance from the camera.
   //! The near clipping plane is animatable.
   const AnimatableFloat &GetNearClippingPlane() const { return m_nearClippingPlane; }

   //! Set the near clipping plance distance.
   //! For perspective cameras this should be positive and >0.0f
   //! For best results, place the near clipping plane as close to the objects as possible.
   void SetNearClippingPlane(float val)                { m_nearClippingPlane.Set(val);  }

   //! Get the far clipping plane distance from the camera.
   //! The far clipping plane is animatable.
   AnimatableFloat &GetFarClippingPlane()              { return m_farClippingPlane; }

   //! Get the far clipping plane distance from the camera.
   //! The far clipping plane is animatable.
   const AnimatableFloat &GetFarClippingPlane() const  { return m_farClippingPlane; }

   //! Set the far clipping plane distance from the camera.
   //! For perspective cameras this should be positive and >0.0f
   //! For best results, place the far clipping plane as close to the back of the objects as possible.
   void SetFarClippingPlane(float val)                 { m_farClippingPlane.Set(val);  }

   //! Set both near and far clipping plane distances simultaneously.
   void SetClippingPlanes(float nr, float fr)          { SetNearClippingPlane(nr); SetFarClippingPlane(fr); }

   //! Return the focal plane distance from the camera.
   //! The focal plane is the distance which will appear to be in the plane of the display (neither in front nor behind)
   //! when viewing in stereo.
   //! The focal plane is animatable.
   const AnimatableFloat &GetFocalPlane() const        { return m_focalPlane; }

   //! Return the focal plane distance from the camera.
   //! The focal plane is the distance which will appear to be in the plane of the display (neither in front nor behind)
   //! when viewing in stereo.
   //! The focal plane is animatable.
   AnimatableFloat &GetFocalPlane()                    { return m_focalPlane; }

   //! Set the focal plane distance from the camera.
   //! The focal plane is the distance which will appear to be in the plane of the display (neither in front nor behind)
   //! when viewing in stereo.
   void SetFocalPlane(float val)                       { m_focalPlane.Set(val); }

   //! Sets the absolute eye separation.
   //! Used when calculating left & right eye images for stereo.
   void SetEyeSeparationAbsolute(float value);

   //! Sets the eye separation for stereo display based upon a division of the near plane distance.
   //! This is the default mode. The default divisor is 20.
   void SetEyeSeparationNearDistDivideFactor(float divider = 20.0f);

   //! Returns the absolute eye separation.
   float GetEyeSeparation() const;

   //! Calculates and returns a projection matrix for this camera.
   void MakeProjectionMatrix(Mat4 *proj) const;

   //! Calculates and returns a projection matrix for the left eye of a stereo view for this camera.
   void MakeLeftEyeProjectionMatrix(Mat4 *proj) const;

   //! Calculates and returns a projection matrix for the right eye of a stereo view for this camera.
   void MakeRightEyeProjectionMatrix(Mat4 *proj) const;

   //! Returns a view volume for the frustum used by this camera.
   void GetViewVolume(ViewVolume *frustumPtr) const;

   //! Retrieves any bsg::CameraCallback that was registered with this node.
   CameraCallback *GetCallback() const                { return m_callback;                   }

   //! Sets a bsg::CameraCallback on this node it is now ownded by the node.
   void SetCallback(CameraCallback *callback)         { if (m_callback != NULL) delete m_callback; m_callback = callback; }

protected:
   Camera();

private:
   float XFovOrtho() const { return m_yFov * m_aspectRatio; }
   float XFovPersp() const;

private:
   std::string       m_name;
   eType             m_type;
   float             m_eyeSepAbs;
   float             m_eyeSepDivider;
   AnimatableFloat   m_yFov;                 // The vertical field of view in degrees
   AnimatableFloat   m_aspectRatio;
   AnimatableFloat   m_nearClippingPlane;
   AnimatableFloat   m_farClippingPlane;
   AnimatableFloat   m_focalPlane;           // Distance to focal plane (where parallax=0) for stereo cameras only

   CameraCallback    *m_callback;
};

/** @} */

//! @addtogroup math
//! @{

//! A helper class used to generate a Transform to be used in a scene node (or an animation), based on a
//! camera position, target position and up-vector.
class CameraTransformHelper
{
public:
   //! Return a transform for the given camera position, target position and up-vector
   static Transform Lookat(const Vec3 &cameraPos, const Vec3 &target, const Vec3 &up);
};

//! @}

typedef std::vector<CameraHandle>   Cameras;

}

#endif /* __BSG_CAMERA_H__ */

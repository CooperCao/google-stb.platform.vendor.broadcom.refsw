/******************************************************************************
 *   Broadcom Proprietary and Confidential. (c)2011-2012 Broadcom.  All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
 * AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
 * SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE
 * ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 * ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#include "bsg_application.h"
#include "bsg_exception.h"
#include "bsg_surface.h"
#include "bsg_geometry.h"
#include "bsg_scene_node.h"
#include "bsg_constraint.h"
#include "bsg_semantic_data.h"
#include "bsg_font.h"
#include "bsg_matrix.h"

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

namespace bsg
{

Application    *Application::m_instance = NULL;
CameraHandle    Application::m_nullCamera;

namespace prv
{
   struct SortPriorityCompare
   {
      bool operator()(const DrawPacket &p1, const DrawPacket &p2)
      {
         int32_t sp1 = p1.GetSortPriority();
         int32_t sp2 = p2.GetSortPriority();

         if (sp1 == sp2)
            return p1.GetMaterial() < p2.GetMaterial();

         return sp1 < sp2;
      }
   };

   struct DepthCompare
   {
      static float GetDepth(const DrawPacket &p)
      {
         float          depth;
         EffectHandle   effect = p.GetMaterial()->GetEffect();

         if (effect->OverrideDepth())
            depth = effect->GetDepth();
         else
            depth = p.GetBound().GetCenter().Z();

         return depth;
      }
   };

   struct DepthCompareFTB : public DepthCompare
   {
      bool operator()(const DrawPacket &p1, const DrawPacket &p2)
      {
         float z1 = GetDepth(p1);
         float z2 = GetDepth(p2);

         if (z1 > z2)
            return true;

         if (z1 < z2)
            return false;

         return SortPriorityCompare()(p1, p2);
      }
   };

   struct DepthCompareBTF : public DepthCompare
   {
      bool operator()(const DrawPacket &p1, const DrawPacket &p2)
      {
         float z1 = GetDepth(p1);
         float z2 = GetDepth(p2);

         if (z1 < z2)
            return true;

         if (z1 > z2)
            return false;

         return SortPriorityCompare()(p1, p2);
      }
   };

}

static void RenderPackets(DrawPackets &drawPackets, ShadowState &glState, const ViewVolume &frustum, const RenderOptions &options)
{
   for (DrawPackets::iterator i = drawPackets.begin(); i != drawPackets.end(); ++i)
   {
      DrawPacket& packet = *i;

      if (packet.GetRender())
      {
         bool cull = false;

         if (options.GetEnableViewFrustumCull() && packet.GetSurface()->GetViewFrustumCull())
            cull = -frustum.MinDistance(packet.GetBound().GetCenter()) > packet.GetBound().GetRadius();

         if (!cull)
         {
            const Geometry    *geom       = packet.GetGeometry();
            const Surface     *surf       = packet.GetSurface();
            Material          *material   = packet.GetMaterial();
            const PassState   &surfState  = surf->GetGLState();

            PassState   surfState1(surfState);

            if (geom->GetReflected())
            {
               surfState1.SetFrontFace(surfState.GetFrontFace() == GL_CW ? GL_CCW : GL_CW);
            }

            // Render all passes
            for (uint32_t pass = 0; pass < material->NumPasses(); pass++)
            {
               const PassState &matState  = material->GetGLState(pass);

               if (packet.GetAutoSorted() && packet.GetOpacity() > 0.999f)
               {
                  // This material was flagged as auto-sorted. If its opacity is 1, it's actually opaque,
                  // so we will disable blending automatically too.
                  PassState matState1 = matState;
                  matState1.SetEnableBlend(false);

                  glState.UpdateGLState(matState1, surfState1);
               }
               else
               {
                  glState.UpdateGLState(matState, surfState1);
               }

               // Check if the geometry has a LOD range specified and if so whether
               // it is within that range
               if (geom->GetLodRange().InRange(packet.GetSemanticData(), packet.GetBound()))
                  if (packet.OnDraw())
                     surf->Draw(pass, material, packet.GetSemanticData());
            }
         }
      }
   }
}

static void CalculateRenderData(DrawPackets &packets, const Mat4 &view, const Mat4 &proj)
{
   for (DrawPackets::iterator i = packets.begin(); i != packets.end(); ++i)
      (*i).SetRenderData(view, proj);
}

static void CalculateAllRenderData(GatherVisitor &gather, const Mat4 &view, const Mat4 &proj)
{
   CalculateRenderData(gather.GetFrontToBackPackets(), view, proj);
   CalculateRenderData(gather.GetBackToFrontPackets(), view, proj);
   CalculateRenderData(gather.GetNoSortPackets(),      view, proj);
}

static void SortAllPackets(GatherVisitor &gather)
{
   std::sort(gather.GetFrontToBackPackets().begin(), gather.GetFrontToBackPackets().end(), prv::DepthCompareFTB());
   std::sort(gather.GetNoSortPackets().begin(), gather.GetNoSortPackets().end(), prv::SortPriorityCompare());
   std::sort(gather.GetBackToFrontPackets().begin(), gather.GetBackToFrontPackets().end(), prv::DepthCompareBTF());
}

static void RenderAllPackets(GatherVisitor &gather, const ViewVolume &frustum, const RenderOptions &options)
{
   // Default state
   ShadowState   glState;

   RenderPackets(gather.GetFrontToBackPackets(), glState, frustum, options);
   RenderPackets(gather.GetNoSortPackets(),      glState, frustum, options);
   RenderPackets(gather.GetBackToFrontPackets(), glState, frustum, options);

   // Reset default state
   glState.SetToDefault();
}

static uint32_t FindCameraPacket(const CameraPackets &packets, const CameraHandle &camera)
{
   uint32_t index = 0;
   Camera   *ptr = camera.GetPtr();

   for (CameraPackets::const_iterator i = packets.begin(); i != packets.end(); ++i, ++index)
   {
      if (ptr == (*i).m_camera)
         return index;
   }

   BSG_THROW("Camera not found");

   return 0;
}

static bool IsItBigEndian()
{
   uint32_t endianW = 0x11223344;
   uint8_t *endianB = (uint8_t*)&endianW;

   return endianB[0] == 0x11;
}


Application::Application(Platform &platform) :
   m_platform(platform),
   m_devHud(0),
   m_fpsHud(0),
   m_bigEndian(IsItBigEndian()),
   m_renderTarget(0)
{
   if (m_instance != NULL)
      BSG_THROW("Only one application can be created\n");

   m_instance = this;

   Libraries::Create();

   m_devHud = new DevHud;
   m_fpsHud = new FpsHud;

   m_platform.Install(this);
}

Application::~Application()
{
   // Delete the HUD before we delete the libraries
   delete m_devHud;
   delete m_fpsHud;

#ifndef BSG_STAND_ALONE
   Print::DeleteDefaultEffect();
#endif

   Libraries::Destroy();

   // Stop people trying to access the dead application
   m_instance = 0;
}

void Application::SetupCamera(Mat4 *view, Mat4 *proj, ViewVolume *frustum, GatherVisitor &gather, const CameraHandle &camera, const RenderOptions &options) const
{
   const CameraPackets  &cameraPackets = gather.GetCameraPackets();

   if (cameraPackets.size() == 0)
      BSG_THROW("No cameras found");

   uint32_t camIndex = 0;

   if (!camera.IsNull())
      camIndex = FindCameraPacket(cameraPackets, camera);

   const Camera *cam = cameraPackets[camIndex].m_camera;

   Mat4  iview = cameraPackets[camIndex].m_view;

   *view = Invert(cameraPackets[camIndex].m_view);

   CameraCallback *callback = cam->GetCallback();

   if (callback != 0)
      callback->OnViewTransform(*view, iview);

   if (m_platform.IsStereo())
      AdjustStereoEyePosition(cam, view);

   if (m_platform.IsStereo())
   {
      if (m_platform.CurrentEye() == Platform::eLEFT)
         cam->MakeLeftEyeProjectionMatrix(proj);
      else if (m_platform.CurrentEye() == Platform::eRIGHT)
         cam->MakeRightEyeProjectionMatrix(proj);
   }
   else
   {
      cam->MakeProjectionMatrix(proj);
   }

   if (options.GetEnableViewFrustumCull())
      cam->GetViewVolume(frustum);
}

void Application::SetRenderTarget(FramebufferHandle *handle)
{
   if (m_renderTarget != handle)
   {
      if (handle != 0)
         (*handle)->Bind();
      else
         Framebuffer::BindDefault();
   }

   m_renderTarget = handle;
}

void Application::RenderSceneGraph(const SceneNodeHandle &rootNode, const CameraHandle &camera)
{
   static RenderOptions defaultOptions;

   RenderSceneGraph(rootNode, defaultOptions, camera);
}

void Application::RenderSceneGraph(const SceneNodeHandle &rootNode, const RenderOptions &options, const CameraHandle &camera)
{
   GatherVisitor  gather;
   Mat4           view;
   Mat4           proj;
   ViewVolume     frustum;

   // Gather the draw packets (traverses nodes gathering objects and composing transformations)
   rootNode->Accept(gather);

   if (!gather.IsEmpty())
   {
      // Find and setup the camera, fill in frustum and semantic data
      SetupCamera(&view, &proj, &frustum, gather, camera, options);

      // Calculate the bounding boxes and sort and render
      CalculateAllRenderData(gather, view, proj);
      SortAllPackets(gather);
      RenderAllPackets(gather, frustum, options);

      // Check for GL errors
      GLenum   err = glGetError();
      if (err != GL_NO_ERROR)
         fprintf(stderr, "GL error 0x%x\n", err);
   }
}

#ifndef BSG_STAND_ALONE

void Application::DrawTextStringAbs(const std::string &str, float xpos, float ypos,
   FontHandle font, const Vec4 &color) const
{
   font->DrawTextString(str, xpos, ypos, color);
}

void Application::DrawTextString(const std::string &str, float xpos, float ypos, FontHandle font, const Vec4 &color) const
{
   float x = xpos * GetWindowWidth();
   float y = ypos * GetWindowHeight();

   DrawTextStringAbs(str, x, y, font, color);
}

#endif

void Application::AdjustStereoEyePosition(const Camera *cam, Mat4 *viewMx) const
{
   // Extract x-direction vector from view matrix
   Vec3 xdir((*viewMx)(0, 0), (*viewMx)(1, 0), (*viewMx)(2, 0));

   if (m_platform.CurrentEye() == Platform::eLEFT)
      xdir = Normalize(xdir) * (0.5f * cam->GetEyeSeparation());
   else if (m_platform.CurrentEye() == Platform::eRIGHT)
      xdir = Normalize(xdir) * (-0.5f * cam->GetEyeSeparation());

   *viewMx = Translate(xdir[0], xdir[1], xdir[2]) * (*viewMx);
}

void Application::glViewport(GLint x, GLint y, GLsizei width, GLsizei height) const
{
   if (m_platform.IsStereo())
   {
      if (m_platform.CurrentEye() == Platform::eLEFT)
         ::glViewport(x / 2, y, width / 2, height);
      else if (m_platform.CurrentEye() == Platform::eRIGHT)
         ::glViewport((GetWindowWidth() + x) / 2, y, width / 2, height);
   }
   else
   {
      ::glViewport(x, y, width, height);
   }
}

bool Application::IsLeftFrame() const
{
   if (!m_platform.IsStereo())
      return true;

   return m_platform.CurrentEye() == Platform::eLEFT;
}

bool Application::IsBeginFrame() const
{
   if (m_platform.IsStereo())
      return m_platform.CurrentEye() == Platform::eLEFT;

   return true;
}

bool Application::IsEndFrame() const
{
   if (m_platform.IsStereo())
      return m_platform.CurrentEye() == Platform::eLEFT;

   return true;
}

void Application::glClear(GLbitfield mask) const
{
   if (m_platform.IsStereo())
   {
      if (m_platform.CurrentEye() == Platform::eLEFT)
         ::glClear(mask);

      // Do nothing for right eye - assume cleared during left
      // This won't work if scissored clears are used.
   }
   else
      ::glClear(mask);
}

bool Application::IsBigEndian() const
{
   return m_bigEndian;
}

void Application::HandleInternalResize(uint32_t w, uint32_t h)
{
   // Need to at least resize the HUD fonts
   m_devHud->Resize(w, h);
   m_fpsHud->Resize(w, h);

   ResizeHandler(w, h);
}

}

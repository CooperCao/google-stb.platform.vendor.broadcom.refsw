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

#include "geom_viewer.h"

#include "bsg_application_options.h"
#include "bsg_scene_node.h"
#include "bsg_shape.h"
#include "bsg_material.h"
#include "bsg_effect.h"
#include "bsg_gl_texture.h"
#include "bsg_image_png.h"
#include "bsg_image_pkm.h"
#include "bsg_exception.h"
#include "bsg_math.h"

#include <istream>
#include <fstream>

using namespace bsg;

#define ADD_GEOM(geoms, cull, geom) AddGeom(geoms, cull, geom, #geom)

void AddGeom(std::vector<GeometryItem> &geoms, bool cull, const GeometryHandle &handle, const std::string &info)
{
   geoms.push_back(GeometryItem(handle, info));

   if (!cull)
      handle->GetSurface(0)->SetCull(false);
}

GeomViewer::GeomViewer(Platform &platform) :
   Application(platform),
   m_root(New),
   m_camNode(New),
   m_camera(New),
   m_font(New)
{
   m_font->SetFontInPercent("DroidSans.ttf", 4.0f, GetOptions().GetHeight());

   // Setup the camera and its node
   m_camera->SetNearClippingPlane(10.0f);
   m_camera->SetFarClippingPlane(100.0f);
   m_camera->SetFocalPlane(10.0f);
   m_camera->SetAspectRatio(GetWindowWidth(), GetWindowHeight());

   m_camNode->SetCamera(m_camera);
   m_camNode->GetTransform().SetPosition(Vec3(4.0f, 0.0f, 14.0f));

   EffectHandle  effect(New);
   effect->Load("tex.bfx");

   TextureHandle texture(New);
   texture->TexImage2D(ImageSet("grid", "pkm", Image::eETC1));

   MaterialHandle mat(New);
   mat->SetEffect(effect);
   mat->SetTexture("u_tex", texture);

   ADD_GEOM(m_geoms, true,  SphereFactory(Vec3(), 1.0f, 30).MakeGeometry(mat));
   ADD_GEOM(m_geoms, true,  CuboidFactory(1.4f).MakeGeometry(mat));
   ADD_GEOM(m_geoms, false, DiskFactory(Vec3(), 1.0f, 32, eZ_AXIS).MakeGeometry(mat));
   ADD_GEOM(m_geoms, false, RingFactory(Vec3(), 0.25f, 1.0f, 64, eZ_AXIS).MakeGeometry(mat));
   ADD_GEOM(m_geoms, false, ConeFactory(Vec3(), 0.25f, 1.0f, 2.0f, 64, eY_AXIS).MakeGeometry(mat));
   ADD_GEOM(m_geoms, false, RoundedRectFactory(Vec3(), 1.4f, 1.4f, 0.25f, 64, eZ_AXIS).MakeGeometry(mat));
   ADD_GEOM(m_geoms, true,  ObjFactory("torus.obj").MakeGeometry(mat));
   ADD_GEOM(m_geoms, false, QuadFactory(1.0f, 1.0f, 0.0f, eZ_AXIS).MakeGeometry(mat));
   ADD_GEOM(m_geoms, false, PanelFactory(Vec2(-0.5f), Vec2(0.5f), 0.2f, 0.35f, eZ_AXIS).MakeGeometry(mat));

   Time now = FrameTimestamp();

   for (uint32_t i = 0; i < m_geoms.size(); ++i)
   {
      // Create a node for the geometry
      SceneNodeHandle   node(New);

      // Normalize the positions
      Vec3  center = m_geoms[i].GetGeometry()->GetSurface(0)->GetBound().GetCenter();
      node->GetTransform().SetPosition(-center);

      node->AppendGeometry(m_geoms[i].GetGeometry());

      // Add node to list of carousel objects
      m_menuItems.push_back(node);

      // Attach their animations. Use the LerpMod variant as we are extrapolating.
      AnimBindingLerpQuaternionAngle *anim = new AnimBindingLerpQuaternionAngle(&node->GetTransform().GetRotation());
      anim->Interpolator()->Init(now, now + 10.0f, BaseInterpolator::eREPEAT);
      anim->Evaluator()->Init(Vec3(0.0f, 1.0f, 0.0f), 0.0f, 360.0f);

      m_animList.Append(anim);
   }

   // Create the carousel
   m_carousel = new MenuCarousel(4.0f, 4.0f, m_menuItems);

   // Add the camera
   m_root->AppendChild(m_camNode);
   m_root->AppendChild(m_carousel->RootNode());

   // Init any global GL state
   glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
}

bool GeomViewer::UpdateFrame(int32_t * /*idleMs*/)
{
   m_animList.UpdateTime(FrameTimestamp());
   m_carousel->UpdateAnimationList(FrameTimestamp());

   return true;
}

void GeomViewer::ResizeHandler(uint32_t width, uint32_t height)
{
   m_camera->SetAspectRatio(width, height);
   m_font->SetFontInPercent("DroidSans-Bold.ttf", 4.0f, height);
}

void GeomViewer::RenderFrame()
{
   // Clear all the buffers
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   RenderSceneGraph(m_root);

   DrawTextString(m_geoms[m_carousel->CurrentIndex()].GetInfo(), 0.2f, 0.95f, m_font, Vec4(1.0f));
}

void GeomViewer::KeyEventHandler(KeyEvents &queue)
{
   // Service one pending key event
   while (queue.Pending())
   {
      KeyEvent ev = queue.Pop();

      if (ev.State() == KeyEvent::eKEY_STATE_DOWN)
      {
         switch (ev.Code())
         {
         case KeyEvent::eKEY_EXIT :
         case KeyEvent::eKEY_ESC :
            Stop(255); 
            break;
         case KeyEvent::eKEY_UP : 
            m_carousel->Prev(FrameTimestamp(), Time(0.3f));
            break;
         case KeyEvent::eKEY_DOWN : 
            m_carousel->Next(FrameTimestamp(), Time(0.3f)); 
            break;
         case KeyEvent::eKEY_OK : 
         case KeyEvent::eKEY_ENTER :
            // do something
            break;
         default : 
            break;
         }
      }
   }
}

void GeomViewer::MouseEventHandler(MouseEvents &queue)
{
   // Service pending mouse events
   while (queue.Pending())
   {
      MouseEvent ev = queue.Pop();

      if (ev.Type() == MouseEvent::eMOUSE_EVENT_WHEEL)
      {
         IVec2 vec = ev.RelativeWheelVector();

         if (vec.Y() > 0)
            m_carousel->Prev(FrameTimestamp(), Time(0.3f));
         else if (vec.Y() < 0)
            m_carousel->Next(FrameTimestamp(), Time(0.3f)); 
      }
   }
}

int main(int argc, char **argv)
{
   uint32_t ret = 0;

   try
   {
      ApplicationOptions   options;

      options.SetDisplayDimensions(1280, 720);

      if (!options.ParseCommandLine(argc, argv))
         BSG_THROW("Invalid command-line options");

      Platform    platform(options);
      GeomViewer  app(platform);

      ret = platform.Exec();
   }
   catch (const Exception &e)
   {
      std::cerr << "Exception : " << e.Message() << "\n";
   }

   return ret;
}

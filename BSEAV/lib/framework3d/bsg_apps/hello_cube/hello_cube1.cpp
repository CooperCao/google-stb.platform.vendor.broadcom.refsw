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

#include "hello_cube1.h"

#include "bsg_application_options.h"
#include "bsg_scene_node.h"
#include "bsg_shape.h"
#include "bsg_material.h"
#include "bsg_effect.h"
#include "bsg_gl_texture.h"
#include "bsg_image_pkm.h"
#include "bsg_exception.h"

#include <iostream>
#include <istream>
#include <fstream>

/////////////////////////////////////////////////////////////////////

HelloCubeApp1::HelloCubeApp1(bsg::Platform &platform) :
   bsg::Application(platform),
   m_root  (bsg::New),
   m_camera(bsg::New)
{
   // Setup a basic camera
   m_camera->SetClippingPlanes(0.5f, 100.0f);
   m_camera->SetYFov(65.0f);

   // Add camera into its node and position it
   bsg::SceneNodeHandle   cameraNode(bsg::New);

   cameraNode->SetCamera(m_camera);
   cameraNode->SetTransform(
      bsg::CameraTransformHelper::Lookat(bsg::Vec3(0.0f, 1.5f, 8.0f),    // Where
                                         bsg::Vec3(0.0f, 0.0f, 0.0f),    // Lookat
                                         bsg::Vec3(0.0f, 1.0f, 0.0f)));  // Up-vector

   // Load the effect file for the cube and create a material with it
   bsg::EffectHandle  cubeEffect(bsg::New);
   cubeEffect->Load("cube_flat.bfx");

   bsg::MaterialHandle cubeMat(bsg::New);
   cubeMat->SetEffect(cubeEffect);

   // Create the geometry and add to its node
   bsg::GeometryHandle    cubeGeom(bsg::CuboidFactory(5.0f).MakeGeometry(cubeMat));
   bsg::SceneNodeHandle   cubeNode(bsg::New);
   cubeNode->AppendGeometry(cubeGeom);

   // Construct the scene graph (root with a camera and cube child)
   m_root->AppendChild(cameraNode);
   m_root->AppendChild(cubeNode);

   // Init any global GL state
   glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
}

void HelloCubeApp1::RenderFrame()
{
   // Clear all the buffers (this is the most efficient thing to do)
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   RenderSceneGraph(m_root);
}

void HelloCubeApp1::KeyEventHandler(bsg::KeyEvents &queue)
{
   // Service one pending key event
   while (queue.Pending())
   {
      bsg::KeyEvent ev = queue.Pop();

      if (ev.State() == bsg::KeyEvent::eKEY_STATE_DOWN)
      {
         switch (ev.Code())
         {
         case bsg::KeyEvent::eKEY_EXIT :
         case bsg::KeyEvent::eKEY_ESC :
            Stop(255);
            break;
         default :
            break;
         }
      }
   }
}

void HelloCubeApp1::ResizeHandler(uint32_t width, uint32_t height)
{
   m_camera->SetAspectRatio(width, height);
}

bool HelloCubeApp1::UpdateFrame(int32_t * /*idleMs*/)
{
   return true;
}

int main(int argc, char **argv)
{
   uint32_t ret = 0;

   try
   {
      // Create the default application options object
      bsg::ApplicationOptions   options;

      // Request a specific display size
      options.SetDisplayDimensions(1280, 720);

      // Read any command-line options (possibly overriding the display size)
      if (!options.ParseCommandLine(argc, argv))
         return 1;

      // Initialise the platform
      bsg::Platform       platform(options);

      // Initialise the application
      HelloCubeApp1  app(platform);

      // Run the application
      ret = platform.Exec();
   }
   catch (const bsg::Exception &e)
   {
      // BSG will throw exceptions of type bsg::Exception if anything goes wrong
      std::cerr << "Exception : " << e.Message() << "\n";
   }

   return ret;
}

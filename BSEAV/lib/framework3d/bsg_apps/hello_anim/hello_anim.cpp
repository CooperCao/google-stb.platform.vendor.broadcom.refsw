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

#include "hello_anim.h"

#include "bsg_application_options.h"
#include "bsg_animator.h"
#include "bsg_exception.h"
#include "bsg_shape.h"

#include <iostream>
#include <istream>
#include <fstream>

using namespace bsg;

/////////////////////////////////////////////////////////////////////

SceneNodeHandle &operator<<(SceneNodeHandle &parent, const SceneNodeHandle &child)
{
   parent->AppendChild(child);
   return parent;
}

HelloAnimApp::HelloAnimApp(Platform &platform) :
   Application(platform),
   m_root      (New),
   m_camera    (New)
{
   // Setup a basic camera
   m_camera->SetClippingPlanes(0.5f, 150.0f);
   m_camera->SetYFov(35.0f);

   // Add camera into its node and position it
   SceneNodeHandle   cameraNode(New);

   cameraNode->SetCamera(m_camera);
   cameraNode->SetTransform(
      CameraTransformHelper::Lookat(Vec3(0.0f, 0.0f, 150.0f),  // Where
                                    Vec3(0.0f, 0.0f, 0.0f),    // Lookat
                                    Vec3(0.0f, 1.0f, 0.0f)));  // Up-vector

   // Load the effect file for the cube and create a material with it
   EffectHandle  effect(New);
   effect->Load("tex.bfx");

   MaterialHandle mat(New);
   mat->SetEffect(effect);

   // Load the texture and connect it to the cube
   TextureHandle tex(New);
   tex->TexImage2D(ImageSet("grid", "pkm", Image::eETC1));
   mat->SetTexture("u_tex", tex);

   GeometryHandle ball = SphereFactory(Vec3(), 10.0f, 40).MakeGeometry(mat);

   SceneNodeHandle nodeLimit(New);
   SceneNodeHandle nodeExtrapolate(New);
   SceneNodeHandle nodeRepeat(New);
   SceneNodeHandle nodeMirror(New);
   SceneNodeHandle nodes(New);

   m_root << nodes << cameraNode;
   nodes  << nodeLimit << nodeExtrapolate << nodeRepeat << nodeMirror;

   nodeLimit->AppendGeometry(ball);
   nodeExtrapolate->AppendGeometry(ball);
   nodeRepeat->AppendGeometry(ball);
   nodeMirror->AppendGeometry(ball);

   // Set up animations
   Time now = FrameTimestamp();

   AnimBindingLerpVec3 *anim1 = new AnimBindingLerpVec3(&nodeExtrapolate->GetPosition());
   anim1->Interpolator()->Init(now + 5.0f, now + 10.0f, BaseInterpolator::eEXTRAPOLATE);
   anim1->Evaluator()->Init(Vec3(-20.0f, 30.0f, 0.0f), Vec3(20.0f, 30.0f, 0.0f));
   m_animList.Append(anim1);

   AnimBindingLerpVec3 *anim2 = new AnimBindingLerpVec3(&nodeLimit->GetPosition());
   anim2->Interpolator()->Init(now + 5.0f, now + 10.0f, BaseInterpolator::eLIMIT);
   anim2->Evaluator()->Init(Vec3(-20.0f, 10.0f, 0.0f), Vec3(20.0f, 10.0f, 0.0f));
   m_animList.Append(anim2);

   AnimBindingLerpVec3 *anim3 = new AnimBindingLerpVec3(&nodeRepeat->GetPosition());
   anim3->Interpolator()->Init(now + 5.0f, now + 10.0f, BaseInterpolator::eREPEAT);
   anim3->Evaluator()->Init(Vec3(-20.0f, -10.0f, 0.0f), Vec3(20.0f, -10.0f, 0.0f));
   m_animList.Append(anim3);

   AnimBindingLerpVec3 *anim4 = new AnimBindingLerpVec3(&nodeMirror->GetPosition());
   anim4->Interpolator()->Init(now + 5.0f, now + 10.0f, BaseInterpolator::eMIRROR);
   anim4->Evaluator()->Init(Vec3(-20.0f, -30.0f, 0.0f), Vec3(20.0f, -30.0f, 0.0f));
   m_animList.Append(anim4);

   // Init any global GL state
   glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
}

void HelloAnimApp::RenderFrame()
{
   // Clear all the buffers (this is the most efficient thing to do)
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   // Draw the text
   RenderSceneGraph(m_root);
}

void HelloAnimApp::KeyEventHandler(KeyEvents &queue)
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
         default :
            break;
         }
      }
   }
}

void HelloAnimApp::ResizeHandler(uint32_t width, uint32_t height)
{
   m_camera->SetAspectRatio(width, height);
}

bool HelloAnimApp::UpdateFrame(int32_t * /*idleMs*/)
{
   m_animList.UpdateTime(FrameTimestamp());
   return true;
}

int main(int argc, char **argv)
{
   uint32_t ret = 0;

   try
   {
      // Create the default application options object
      ApplicationOptions   options;

      // Request a specific display size
      options.SetDisplayDimensions(1280, 720);

      // Read any command-line options (possibly overriding the display size)
      if (!options.ParseCommandLine(argc, argv))
         return 1;

      // Initialise the platform
      Platform       platform(options);

      // Initialise the application
      HelloAnimApp  app(platform);

      // Run the application
      ret = platform.Exec();
   }
   catch (const Exception &e)
   {
      // BSG will throw exceptions of type bsg::Exception if anything goes wrong
      std::cerr << "Exception : " << e.Message() << "\n";
   }

   return ret;
}

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

#include "hello_fbo.h"

#include "bsg_application_options.h"
#include "bsg_animator.h"
#include "bsg_shape.h"
#include "bsg_exception.h"

#include <iostream>
#include <istream>
#include <fstream>

using namespace bsg;


/////////////////////////////////////////////////////////////////////

HelloFBO::HelloFBO(Platform &platform) :
   Application(platform),
   m_root       (New),
   m_cameraNode (New),
   m_cubeNode   (New),
   m_camera     (New),
   m_cubeMat    (New),
   m_textureFBO (New),
   m_textureLogo(New),
   m_fbo        (New),
   m_depth      (New),
   m_discard    (true)
{
   // Setup a basic camera
   m_camera->SetClippingPlanes(0.5f, 50.0f);
   m_camera->SetYFov(60.0f);

   // Add camera into its node and position it
   m_cameraNode->SetCamera(m_camera);
   m_cameraNode->SetTransform(
      CameraTransformHelper::Lookat(Vec3(0.0f, 1.5f, 8.0f),    // Where
                                    Vec3(0.0f, 0.0f, 0.0f),    // Lookat
                                    Vec3(0.0f, 1.0f, 0.0f)));  // Up-vector
   m_root->AppendChild(m_cameraNode);

   // Load the effect file for the cube and create a material with it
   EffectHandle  cubeEffect(New);
   cubeEffect->Load("cube_tex.bfx");
   m_cubeMat->SetEffect(cubeEffect);

   // Load texture
   m_textureLogo->TexImage2D(0, Image("bcm_logo", "png", Image::eRGBA8888));

   // Create storage for FBO color buffer (no data required)
   m_textureFBO->TexImage2D(GL_RGBA, FBO_WIDTH, FBO_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, 0);

   // Create storage for fbo depth buffer
   m_depth->Storage(GL_DEPTH_COMPONENT16, FBO_WIDTH, FBO_HEIGHT);

   // Attach depth and color buffers to the FBO
   SetRenderTarget(&m_fbo);
   m_fbo->Attach(Attachment(m_textureFBO, GL_COLOR_ATTACHMENT0),
                 Attachment(m_depth, GL_DEPTH_ATTACHMENT));

   // Make sure it worked
   if (m_fbo->Check() != GL_FRAMEBUFFER_COMPLETE)
      BSG_THROW("Framebuffer is incomplete");

   m_fbo->Discardable(GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT);

   // Create the geometry and add to its node
   GeometryHandle cubeGeom = CuboidFactory(5.0f).MakeGeometry(m_cubeMat);
   m_cubeNode->AppendGeometry(cubeGeom);
   m_root->AppendChild(m_cubeNode);

   // Ask for the time
   Time now = FrameTimestamp();

   // Create an animator to rotate the cube
   AnimBindingLerpQuaternionAngle *anim = new AnimBindingLerpQuaternionAngle(&m_cubeNode->GetTransform().GetRotation());
   anim->Interpolator()->Init(now, now + 10.0f, BaseInterpolator::eREPEAT);
   anim->Evaluator()->Init(Vec3(0.7f, 1.0f, 0.0f), 0.0f, 360.0f);
   m_animList.Append(anim);

}

void HelloFBO::RenderFrame()
{
   //--------------------------------------------------------------------------
   // Render to the FBO
   //--------------------------------------------------------------------------

   if (IsBeginFrame())
   {
      SetRenderTarget(&m_fbo);

      glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
      // Use the global viewport function because we are not rendering stereo into the FBO and
      // the Application class has a glViewport method that compensates for which field is active
      ::glViewport(0, 0, FBO_WIDTH, FBO_HEIGHT);

      m_camera->SetAspectRatio(FBO_WIDTH, FBO_HEIGHT);
      m_cubeMat->SetTexture("u_tex", m_textureLogo);

      RenderSceneGraph(m_root);

      if (m_discard)
         m_fbo->Discard();
   }

   //--------------------------------------------------------------------------
   // Render to the default framebuffer
   //--------------------------------------------------------------------------

   SetRenderTarget();

   glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
   glViewport(0, 0, GetWindowWidth(), GetWindowHeight());

   m_camera->SetAspectRatio(GetWindowWidth(), GetWindowHeight());
   m_cubeMat->SetTexture("u_tex", m_textureFBO);

   RenderSceneGraph(m_root);
}

void HelloFBO::KeyEventHandler(KeyEvents &queue)
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
            std::cout << "Discard enabled\n";
            m_discard = true;
            break;

         case KeyEvent::eKEY_DOWN :
            std::cout << "Discard disabled\n";
            m_discard = false;
            break;

         default : 
            break;
         }
      }
   }
}

void HelloFBO::ResizeHandler(uint32_t /* width */, uint32_t /* height */)
{
   // Nothing to do.  Camera FOV is set in RenderFrame
}

bool HelloFBO::UpdateFrame(int32_t * /*idleMs*/)
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
      HelloFBO  app(platform);

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

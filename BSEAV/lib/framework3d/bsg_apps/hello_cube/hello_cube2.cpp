/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "hello_cube2.h"

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

using namespace bsg;

/////////////////////////////////////////////////////////////////////

HelloCubeApp2::HelloCubeApp2(Platform &platform) :
   Application(platform),
   m_root     (New),
   m_camera   (New)
{
   // Setup a basic camera
   m_camera->SetClippingPlanes(0.5f, 100.0f);
   m_camera->SetYFov(65.0f);

   // Add camera into its node and position it
   SceneNodeHandle   cameraNode(New);
   cameraNode->SetCamera(m_camera);
   cameraNode->SetTransform(
      CameraTransformHelper::Lookat(Vec3(0.0f, 1.5f, 8.0f),    // Where
                                    Vec3(0.0f, 0.0f, 0.0f),    // Lookat
                                    Vec3(0.0f, 1.0f, 0.0f)));  // Up-vector

   // Load the effect file for the cube and create a material with it
   EffectHandle  cubeEffect(New);
   cubeEffect->Load("cube_flat.bfx");

   MaterialHandle cubeMat(New);
   cubeMat->SetEffect(cubeEffect);

   // Create the geometry and add to its node
   GeometryHandle    cubeGeom = CuboidFactory(5.0f).MakeGeometry(cubeMat);
   SceneNodeHandle   cubeNode(New);
   cubeNode->AppendGeometry(cubeGeom);

   // Create an animator to rotate the cube
   Time now = FrameTimestamp();
   AnimBindingLerpQuaternionAngle *anim = new AnimBindingLerpQuaternionAngle(&cubeNode->GetRotation());
   anim->Interpolator()->Init(now, now + 10.0f, BaseInterpolator::eREPEAT);
   anim->Evaluator()->Init(Vec3(0.7f, 1.0f, 0.0f), 0.0f, 360.0f);
   m_animList.Append(anim);

   // Construct the scene graph (root with a camera and cube child)
   m_root->AppendChild(cameraNode);
   m_root->AppendChild(cubeNode);

   // Init any global GL state
   glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
}

void HelloCubeApp2::RenderFrame()
{
   // Clear all the buffers (this is the most efficient thing to do)
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   RenderSceneGraph(m_root);
}

void HelloCubeApp2::KeyEventHandler(KeyEvents &queue)
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

void HelloCubeApp2::ResizeHandler(uint32_t width, uint32_t height)
{
   m_camera->SetAspectRatio(width, height);
}

bool HelloCubeApp2::UpdateFrame(int32_t * /*idleMs*/)
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
      HelloCubeApp2  app(platform);

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

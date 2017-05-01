/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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

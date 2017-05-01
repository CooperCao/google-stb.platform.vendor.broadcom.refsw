/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "hello_task.h"

#include "bsg_application_options.h"
#include "bsg_exception.h"
#include "bsg_task.h"
#include "bsg_shape.h"

#include <iostream>
#include <stdint.h>

using namespace bsg;

/////////////////////////////////////////////////////////////////////

class TickTask : public Task
{
public:
   TickTask(uint32_t howMany, const HelloTaskApp &app) :
      m_app(app),
      m_count(0),
      m_howMany(howMany)
   {}

   // This is the task that is run on the secondary thread.  Do not do BSG or GL operations here
   virtual void OnThread()
   {
      std::cout << "Thread started\n";
      for (m_count = 0; m_count < m_howMany; ++m_count)
      {
         for (int32_t i = 0; i < (rand() & 0x3ff); ++i)
            std::cout << "Time wasting " << i << "\n";

         Callback();
      }
      std::cout << "Thread ending\n";
   }

   // These calls are executed in the main thread (so it is safe to do BSG and graphics operations)
   virtual void OnCallback(bool finished)
   {
      if (!finished)
         m_app.SetBoxSize(m_count);
      else
         m_app.Done();
   }

private:
   const HelloTaskApp   &m_app;
   uint32_t             m_count;
   uint32_t             m_howMany;
};

HelloTaskApp::HelloTaskApp(Platform &platform) :
   Application(platform),
   m_root      (New),
   m_camera    (New),
   m_material  (New),
   m_tasker    (new Tasker)
{
   // Setup a basic camera
   m_camera->SetClippingPlanes(0.5f, 500.0f);
   m_camera->SetYFov(65.0f);

   // Add camera into its node and position it
   SceneNodeHandle   cameraNode(New);

   cameraNode->SetCamera(m_camera);
   cameraNode->SetTransform(
      CameraTransformHelper::Lookat(Vec3(0.0f, 0.0f, 300.0f),  // Where
                                    Vec3(0.0f, 0.0f, 0.0f),    // Lookat
                                    Vec3(0.0f, 1.0f, 0.0f)));  // Up-vector

   EffectHandle   effect(New);
   effect->Load("box.bfx");

   m_material->SetEffect(effect);

   GeometryHandle geom = QuadFactory(400.0f, 25.0f, 0.0, eZ_AXIS).MakeGeometry(m_material);

   SceneNodeHandle   node(New);

   node->AppendGeometry(geom);

   // Link to root
   m_root->AppendChild(cameraNode);
   m_root->AppendChild(node);

   SetBoxSize(0);
   Done();

   // Init any global GL state
   glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void HelloTaskApp::SetBoxSize(uint32_t size) const
{
   m_material->SetUniform("u_size", (float)size / COUNT);
}

void HelloTaskApp::Done() const
{
   std::cout << "Launching Task\n";
   m_tasker->Submit(new TickTask(COUNT, *this));
}

void HelloTaskApp::RenderFrame()
{
   // Clear all the buffers (this is the most efficient thing to do)
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   m_tasker->DoCallbacks();

   // Draw the text
   RenderSceneGraph(m_root);
}

void HelloTaskApp::KeyEventHandler(KeyEvents &queue)
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

void HelloTaskApp::ResizeHandler(uint32_t width, uint32_t height)
{
   m_camera->SetAspectRatio(width, height);
}

bool HelloTaskApp::UpdateFrame(int32_t * /*idleMs*/)
{
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
      HelloTaskApp  app(platform);

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

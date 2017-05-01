/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "hello_text.h"

#include "bsg_application_options.h"
#include "bsg_animator.h"
#include "bsg_exception.h"

#include <iostream>
#include <istream>
#include <fstream>

using namespace bsg;

/////////////////////////////////////////////////////////////////////

HelloTextApp::HelloTextApp(Platform &platform) :
   Application(platform),
   m_root      (New),
   m_cameraNode(New),
   m_camera    (New),
   m_font2D    (New)
{
   // Setup a basic camera
   m_camera->SetClippingPlanes(0.5f, 500.0f);
   m_camera->SetYFov(65.0f);

   // Add camera into its node and position it
   m_cameraNode->SetCamera(m_camera);
   m_cameraNode->SetTransform(
      CameraTransformHelper::Lookat(Vec3(0.0f, 0.0f, 300.0f),  // Where
                                    Vec3(0.0f, 0.0f, 0.0f),    // Lookat
                                    Vec3(0.0f, 1.0f, 0.0f)));  // Up-vector

   // Load a 2D font
   m_font2D->SetFontInPixels("DroidSans-Bold.ttf", 40.0f);

   // Create a font
   PrintFontHandle   font(New);
   font->Load("DroidSans-Bold.ttf", 50.0f);

   // Create geometries
   SceneNodeHandle   node1(New);
   PrintHandle       geom1(New);

   geom1->SetFont(font, PrintOptions(PrintOptions::eCENTER, true));
   geom1->SetText("Hello Ordinary", 50.0f);
   geom1->SetUniform("u_textColor", Vec4(1.0f, 0.5f, 0.0f, 1.0f));
   node1->AppendGeometry(geom1);

   SceneNodeHandle   node2(New);
   PrintHandle       geom2(New);
   EffectHandle      effect2(New);

   effect2->Load("ring.bfx");
   geom2->SetFont(font, effect2, PrintOptions(PrintOptions::eCENTER, true));
   geom2->SetText("Hello Rings", 50.0f);
   node2->AppendGeometry(geom2);

   SceneNodeHandle   node3(New);
   PrintHandle       geom3(New);
   EffectHandle      effect3(New);
   TextureHandle     texture(New);

   texture->TexImage2D(Image("earth_land0", "pkm", Image::eETC1));

   effect3->Load("texture.bfx");
   geom3->SetFont(font, effect3, PrintOptions(PrintOptions::eCENTER, true));
   geom3->SetText("Hello Textured", 50.0f);
   geom3->SetTexture("u_texture", texture);
   node3->AppendGeometry(geom3);

   // Link to root
   m_root->AppendChild(m_cameraNode);
   m_root->AppendChild(node1);
   m_root->AppendChild(node2);
   m_root->AppendChild(node3);

   // Set up animations
   Time now = FrameTimestamp();

   // The first three animation target the 3D text
   AnimBindingLerpQuaternionAngle *anim1 = new AnimBindingLerpQuaternionAngle(&node1->GetRotation());
   anim1->Interpolator()->Init(now, now + 5.0f, BaseInterpolator::eREPEAT);
   anim1->Evaluator()->Init(Vec3(0.0f, 0.5f, 1.0f), 0.0f, 360.0f);
   m_animList.Append(anim1);

   AnimBindingLerpCircle *anim2 = new AnimBindingLerpCircle(&node2->GetPosition());
   anim2->Interpolator()->Init(now, now + 7.0f, BaseInterpolator::eREPEAT);
   anim2->Evaluator()->Init(Circle(150.0f, 0.0f, eZ_AXIS));
   m_animList.Append(anim2);

   AnimBindingLerpQuaternionAngle *anim3 = new AnimBindingLerpQuaternionAngle(&node3->GetRotation());
   anim3->Interpolator()->Init(now, now + 15.0f, BaseInterpolator::eREPEAT);
   anim3->Evaluator()->Init(Vec3(0.0f, 1.0f, 0.0f), 0.0f, 360.0f);
   m_animList.Append(anim3);

   // This animation controls the 2D text
   AnimBindingLerpCircle *anim4 = new AnimBindingLerpCircle(&m_textPos);
   anim4->Interpolator()->Init(now, now + 7.0f, BaseInterpolator::eREPEAT);
   anim4->Evaluator()->Init(Circle(-0.25f, 0.0f, eZ_AXIS, eCW_WINDING));
   m_animList.Append(anim4);

   // Init any global GL state
   glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
}

void HelloTextApp::RenderFrame()
{
   // Clear all the buffers (this is the most efficient thing to do)
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   // Draw the text
   RenderSceneGraph(m_root);

   DrawTextString("Simple 2D text", m_textPos.X() + 0.5f, m_textPos.Y() + 0.5f, m_font2D, Vec4(0.5f, 0.5f, 1.0f, 1.0f));
}

void HelloTextApp::KeyEventHandler(KeyEvents &queue)
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

void HelloTextApp::ResizeHandler(uint32_t width, uint32_t height)
{
   m_camera->SetAspectRatio(width, height);
}

bool HelloTextApp::UpdateFrame(int32_t * /*idleMs*/)
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
      HelloTextApp  app(platform);

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

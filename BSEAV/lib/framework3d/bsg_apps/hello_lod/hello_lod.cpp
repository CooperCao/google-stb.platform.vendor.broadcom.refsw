/******************************************************************************
 *   (c)2013 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
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

#include "hello_lod.h"

#include "bsg_application_options.h"
#include "bsg_animator.h"
#include "bsg_exception.h"
#include "bsg_shape.h"
#include "bsg_lod_group.h"

using namespace bsg;

/////////////////////////////////////////////////////////////////////
class CustomArgumentParser : public ArgumentParser
{
public:
   CustomArgumentParser()
   {}

   virtual bool ParseArgument(const std::string &arg)
   {
      if (ApplicationOptions::ArgMatch(arg.c_str(), "tex="))
      {
         std::vector<char> tmp(arg.size() - 3);

         if (sscanf(arg.c_str(), "tex=%s", &tmp[0]) == 1)
         {
            m_texFile = AddSuffix(&tmp[0], "png");
            return true;
         }
      }

      return false;
   }

   virtual std::string UsageString() const
   {
      return "\nhello_lod options\n"
             "tex=texFile    png file for sphere-map lighting\n";
   }

   const std::string &GetTexFileName() const
   {
      return m_texFile;
   }

private:
   std::string AddSuffix(const char *str, const char *suffix)
   {
      std::string res(str);

      size_t   pos = res.find_first_of('.');

      if (pos == std::string::npos)
         res = res + "." + suffix;

      return res;
   }

private:
   std::string m_texFile;
};

/////////////////////////////////////////////////////////////////////

class ColorCallback : public DrawCallback
{
public:
   ColorCallback(const MaterialHandle &material, const Vec4 &color, const bool *showChanges) :
      m_uniform(material->GetUniform<Vec4>("u_color")),
      m_color(color),
      m_showChanges(showChanges)
   {}

   virtual bool OnDraw()
   {
      if (*m_showChanges)
         m_uniform.Set(m_color);
      else
         m_uniform.Set(Vec4(1.0f, 1.0f, 1.0f, 1.0f));

      return true;   // Draw it
   }

private:
   AnimTarget<Vec4>  &m_uniform;
   const Vec4        m_color;
   const bool        *m_showChanges;
};

/////////////////////////////////////////////////////////////////////

HelloLod::HelloLod(Platform &platform, const CustomArgumentParser &options) :
   Application(platform),
   m_root       (New),
   m_obj        (New),
   m_objLOD     (New),
   m_objNoLOD   (New),
   m_font       (New),
   m_camera     (New),
   m_showChanges(false),
   m_useLOD     (true)
{
   m_font->SetFontInPercent("DroidSans-Bold.ttf", 4.0f, GetOptions().GetHeight());

   EffectHandle   effect(New);
   effect->Load("effect.bfx");

   MaterialHandle material(New);
   material->SetEffect(effect);

   // Load the shader texture and connect it to the object
   std::string lightingTexFile = options.GetTexFileName() == "" ? "Satin.png" : options.GetTexFileName();
   std::string colorTexFile    = "Die3.png";

   TextureHandle lightingTex(New);
   lightingTex->SetAutoMipmap(true);
   lightingTex->TexImage2D(Image(lightingTexFile, Image::eRGB888));

   material->SetTexture("u_lightingTex", lightingTex);

   TextureHandle colorTex(New);
   colorTex->SetAutoMipmap(true);
   colorTex->TexImage2D(Image(colorTexFile, Image::eRGB888));

   material->SetTexture("u_colorTex", colorTex);

   // Load in the models
   GeometryHandle geomLow  = ObjFactory("Die3Low.obj").MakeGeometry(material);
   GeometryHandle geomMed  = ObjFactory("Die3Med.obj").MakeGeometry(material);

   ObjFactory  highFactory("Die3High.obj");

   // Make two copies because one will not use LODing
   GeometryHandle geomHighLOD   = highFactory.MakeGeometry(material);
   GeometryHandle geomHighNoLOD = highFactory.MakeGeometry(material);

   // The callback lets us set the color of the die
   geomLow->SetDrawCallback(0,  new ColorCallback(material, Vec4(0.9f, 0.5f, 0.9f, 1.0f), &m_showChanges));
   geomMed->SetDrawCallback(0,  new ColorCallback(material, Vec4(0.8f, 0.8f, 0.4f, 1.0f), &m_showChanges));

   geomHighLOD->SetDrawCallback(0, new ColorCallback(material, Vec4(0.4f, 0.8f, 0.8f, 1.0f), &m_showChanges));
   geomHighNoLOD->SetDrawCallback(0, new ColorCallback(material, Vec4(0.4f, 0.8f, 0.8f, 1.0f), &m_showChanges));

   // To create a lof group, use the LODGroup class and enumerate the geometry and the approx.
   // screen size at which the switches should occur.
   LODGroup  lodGroup;
   lodGroup.Begin();
   lodGroup.Add(geomHighLOD, 800.0f);  // Over 800 pixels approx, draw hi-res
   lodGroup.Add(geomMed,  200.0f);     // Over 200 pixels approx, draw med-res
   lodGroup.Add(geomLow);              // Under 200 pixels approx, draw low res
   lodGroup.End();

   // The geometry should be added to the node as well
   m_objLOD->AppendGeometry(geomHighLOD);
   m_objLOD->AppendGeometry(geomMed);
   m_objLOD->AppendGeometry(geomLow);

   m_objNoLOD->AppendGeometry(geomHighNoLOD);

   // Link to main tree
   m_root->AppendChild(m_obj);
   m_obj->AppendChild(m_objLOD);

   // Animate it
   Time now = FrameTimestamp();

   AnimBindingLerpQuaternionAngle *animRot = new AnimBindingLerpQuaternionAngle(&m_obj->GetRotation());
   animRot->Interpolator()->Init(now, now + 10.0f, BaseInterpolator::eREPEAT);
   animRot->Evaluator()->Init(Vec3(0.7f, 1.0f, 0.0f), 0.0f, 360.0f);
   m_animList.Append(animRot);

   AnimBindingHermiteVec3 *animPos = new AnimBindingHermiteVec3(&m_obj->GetPosition());
   animPos->Interpolator()->Init(now, now + 10.0f, BaseInterpolator::eMIRROR);
   animPos->Evaluator()->Init(Vec3(), Vec3(0.0f, 0.0f, -0.2f));
   m_animList.Append(animPos);

   // Setup a basic camera
   m_camera->SetClippingPlanes(0.01f, 1.0f);
   m_camera->SetYFov(65.0f);

   SceneNodeHandle   cameraNode(New);

   cameraNode->SetCamera(m_camera);
   cameraNode->SetTransform(CameraTransformHelper::Lookat(Vec3(0.0f, 0.0f, 0.02f),  // Where
                                                          Vec3(),                   // Lookat
                                                          Vec3(0.0f, 1.0f, 0.0f))); // Up-vector

   m_root->AppendChild(cameraNode);

   // Init any global GL state
   glClearColor(0.5f, 0.5f, 0.6f, 1.0f);
}

static std::string OnOrOff(bool flag)
{
   return flag ? "on" : "off";
}

void HelloLod::RenderFrame()
{
   // Clear all the buffers (this is the most efficient thing to do)
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   RenderSceneGraph(m_root);

   std::string msg = "FAV1: Show Lod: " + OnOrOff(m_showChanges) + "\n\n" +
                     "FAV2: Use  Lod: " + OnOrOff(m_useLOD);

   DrawTextString(msg, 0.1f, 0.95f, m_font, Vec4(0.3f, 0.9f, 0.3f, 1.0f));
}

void HelloLod::ToggleLOD()
{
   m_useLOD = !m_useLOD;

   m_obj->ClearChildren();
   m_obj->AppendChild(m_useLOD ? m_objLOD : m_objNoLOD);
}

void HelloLod::KeyEventHandler(KeyEvents &queue)
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

         case KeyEvent::eKEY_FAV1:
         case KeyEvent::eKEY_F1:
            m_showChanges = !m_showChanges;
            break;

         case KeyEvent::eKEY_FAV2:
         case KeyEvent::eKEY_F2:
            ToggleLOD();
            break;

         default : 
            break;
         }
      }
   }
}

void HelloLod::ResizeHandler(uint32_t width, uint32_t height)
{
   m_camera->SetAspectRatio(width, height);
}

bool HelloLod::UpdateFrame(int32_t * /*idleMs*/)
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
      CustomArgumentParser flagParser;

      if (!options.ParseCommandLine(argc, argv, &flagParser))
         return 1;

      // Initialise the platform
      Platform       platform(options);

      // Initialise the application
      HelloLod  app(platform, flagParser);

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

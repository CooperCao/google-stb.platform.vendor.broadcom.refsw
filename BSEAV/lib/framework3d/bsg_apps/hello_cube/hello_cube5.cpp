/******************************************************************************
 *   (c)2011-2012 Broadcom Corporation
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
#include "hello_cube5.h"

#include "bsg_application_options.h"
#include "bsg_scene_node.h"
#include "bsg_shape.h"
#include "bsg_material.h"
#include "bsg_constraint.h"
#include "bsg_effect.h"
#include "bsg_gl_texture.h"
#include "bsg_image_pkm.h"
#include "bsg_exception.h"
#include "bsg_effect_generator.h"

#include <iostream>
#include <istream>
#include <fstream>
#include <string>

using namespace bsg;
using namespace std;

class Helper
{
public:
   Helper()
   {}

   EffectHandle Effect(const std::string &name)
   {
      EffectHandle  effect(New);
      effect->Load(name);

      return effect;
   }

   MaterialHandle Material(const std::string &name, const EffectHandle &effect)
   {
      MaterialHandle material(New);
      material->SetEffect(effect);
      m_materials.Append(name, material);

      return material;
   }

   TextureHandle TextureSet(const std::string &name, const std::string &ext, Image::eFormat format)
   {
      TextureHandle tex(New);
      tex->TexImage2D(ImageSet(name, ext, format));

      return tex;
   }

   GeometryHandle ObjData(const std::string &name, std::set<MaterialHandle> *materials)
   {
      ObjMaterialOptions   options;
      options.SetNumLights(1).SetFragmentLighting(true).SetDebug(true);
      options.GetPassState().SetEnableDepthTest(true);

      GeometryHandle cubeGeom = ObjFactory(name).MakeGeometry(options, materials);

      return cubeGeom;
   }

private:
   MaterialMap m_materials;
};

/////////////////////////////////////////////////////////////////////

HelloCubeApp5::HelloCubeApp5(Platform &platform) :
   Application(platform),
   m_root      (New),
   m_cameraNode(New),
   m_cubeNode1 (New),
   m_cubeNode2 (New),
   m_camera    (New)
{
   Helper   help;

   // Setup a basic camera
   m_camera->SetClippingPlanes(0.5f, 100.0f);
   m_camera->SetYFov(45.0f);

   // Add camera into its node and position it
   m_cameraNode->SetCamera(m_camera);
   m_cameraNode->SetTransform(
      CameraTransformHelper::Lookat(Vec3(0.0f, 1.5f, 16.0f),   // Where
                                    Vec3(0.0f, 0.0f, 0.0f),    // Lookat
                                    Vec3(0.0f, 1.0f, 0.0f)));  // Up-vector

   // Load the effect files for the object and create materials with it
   EffectHandle   effect1(help.Effect("cube_tex.bfx"));
   EffectHandle   effect2(help.Effect("cube_flat.bfx"));
   //EffectHandle   effect3(help.Effect("adobe.bfx"));

   MaterialHandle cubeMat1(help.Material("mat1", effect1));
   MaterialHandle cubeMat2(help.Material("mat2", effect2));
   //MaterialHandle cubeMat3(help.Material("mat3", effect3));

   TextureHandle  cubeTex(help.TextureSet("cube", "pkm", Image::eETC1));

   cubeMat1->SetTexture("u_tex", cubeTex);

   // Create the geometry and add to the nodes
   GeometryHandle cubeGeom(help.ObjData("fred.obj", &m_objMaterials));
   GeometryHandle cylinderGeom(ConeFactory(Vec3(), 1.0f, 1.0f, 3.0f, 32, eY_AXIS).MakeGeometry(cubeMat1));

   for (std::set<MaterialHandle>::const_iterator i = m_objMaterials.begin(); i != m_objMaterials.end(); ++i)
   {
      MaterialHandle m = *i;
      m_lightPos.Append(m->GetUniform3f("u_lightPos"));
   }

   m_cubeNode1->AppendGeometry(cubeGeom);
   m_cubeNode2->AppendGeometry(cylinderGeom);

   // Position and scale node2 to be on top face of cube in node1
   m_cubeNode2->SetPosition(Vec3(0.0f, 5.0f, 0.0f));
   m_cubeNode2->SetScale(Vec3(0.5f, 0.5f, 0.5f));

   // Create animators to rotate the cubes
   Time now = FrameTimestamp();

   AnimBindingLerpQuaternionAngle *anim1 = new AnimBindingLerpQuaternionAngle(&m_cubeNode1->GetRotation());
   anim1->Interpolator()->Init(now, now + 10.0f, BaseInterpolator::eREPEAT);
   anim1->Evaluator()->Init(Vec3(0.7f, 1.0f, 0.0f), 0.0f, 360.0f);
   m_animList.Append(anim1);

   AnimBindingLerpQuaternionAngle *anim2 = new AnimBindingLerpQuaternionAngle(&m_cubeNode2->GetRotation());
   anim2->Interpolator()->Init(now, now + 7.0f, BaseInterpolator::eREPEAT);
   anim2->Evaluator()->Init(Vec3(0.0f, 1.0f, 0.0f), 0.0f, 360.0f);
   m_animList.Append(anim2);

   // Construct the scene graph (root with a camera and cube child)
   m_root->AppendChild(m_cameraNode);
   m_root->AppendChild(m_cubeNode1);

   // Cube2 hangs off cube1 and hence inherits all its transformations
   m_cubeNode1->AppendChild(m_cubeNode2);

   SceneNodeHandle   constrained(New);

   constrained->AppendGeometry(cubeGeom);
   constrained->GetTransform().SetScale(Vec3(0.25f, 0.25f, 0.25f));

   ConstraintLerpPositionHandle  constraint(New);

   // Constrains "constrained" to lie half way between node1 and node2.
   constraint->Install(constraint, constrained, m_cubeNode1, m_cubeNode2, 0.7f);

   AnimBindingHermiteFloat *anim3 = new AnimBindingHermiteFloat(&constraint->GetAlpha());
   anim3->Init(0.25f, now, 0.75f, now + 5.0f);
   m_animList.Append(anim3);

   AnimBindingHermiteVec3 *anim4 = new AnimBindingHermiteVec3(&m_lightPos);
   anim4->Evaluator()->Init(Vec3(0.0f, 0.0f, 0.0f), Vec3(-100.0f, 0.0f, 0.0f));
   anim4->Interpolator()->Init(now, now + 15.0f);
   m_animList.Append(anim4);

   //AnimatableFloat f, g;
   //AnimBindingHermiteFloat *animX = new AnimBindingHermiteFloat(&f);
   //animX->Init(1.0f, now, 2.0f, now + 100.0f);

   //f.Set(1.0f);
   //g.Set(1.0f);

   // Init any global GL state
   glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
}

void HelloCubeApp5::RenderFrame()
{
   // Clear all the buffers (this is the most efficient thing to do)
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   RenderSceneGraph(m_root);
}

void HelloCubeApp5::KeyEventHandler(KeyEvents &queue)
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

void HelloCubeApp5::ResizeHandler(uint32_t width, uint32_t height)
{
   m_camera->SetAspectRatio(width, height);
}

bool HelloCubeApp5::UpdateFrame(int32_t * /*idleMs*/)
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
      HelloCubeApp5  app(platform);

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

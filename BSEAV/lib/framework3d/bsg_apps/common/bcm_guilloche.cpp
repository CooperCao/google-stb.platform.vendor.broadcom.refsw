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

#include "bcm_guilloche.h"

#include "bsg_exception.h"
#include "bsg_shape.h"
#include "bsg_application.h"

using namespace bsg;

class GuillocheFactory : public StdFactory
{
public:
   GuillocheFactory(uint32_t isteps, uint32_t lines);
};

GuillocheFactory::GuillocheFactory(uint32_t isteps, uint32_t lines) :
   StdFactory(GL_TRIANGLE_STRIP, 2 * (isteps * lines + lines - 1), false)
{
   float step      = 0.999f / (float)(isteps - 3);
   float line_step = 1.0f / (float)lines;
   
   No(ShapeFactory::eNORMAL);
   No(ShapeFactory::eTEXCOORD1);
   
   for (uint32_t j = 0; j < lines; ++j)
   {
      float interp = j * line_step;

      if (j != 0)
         AddVertex(StdVertex(Vec3(0.0f, interp, 1.0f), Vec3(0.0f), Vec2(0.0f)));

      for (uint32_t i = 0; i < isteps; ++i)
      {
         // TOP
         AddVertex(StdVertex(Vec3(i * step, interp, 1.0f), Vec3(0.0f), Vec2(0.0f)));
         // BOTTOM
         AddVertex(StdVertex(Vec3(i * step, interp, -1.0f), Vec3(0.0f), Vec2(0.0f)));
      }

      if (j != lines - 1)
         AddVertex(StdVertex(Vec3((isteps - 1) * step, interp, -1.0f), Vec3(0.0f), Vec2(0.0f)));
   }
}

/////////////////////////////////////////////////////////////////////
static void CopyControl(std::vector<Vec2> &to, uint32_t size, const Vec2 *from)
{
   float step = 1.0f / (size - 1);

   to.resize(size);

   // The X-coordinates are stored as differences from the parameter value which makes it easier
   // to animate features moving though the waveform
   for (uint32_t i = 0; i < size; ++i)
   {
      to[i].X() = from[i].X() - i * step;
      to[i].Y() = from[i].Y();
   }
}

// Guilloche
//
// Basic form of Guilloche is specified via two curves.  Each curve is specified via control points which are expected to
// lie in the region 0 <= x <= 1 and -0.5 <= y <= 0.5.
// 
// The pattern is drawn using numSteps line segments (each segment is 2 triangles)
// The number of lines in the pattern is specified using numLines
//
// The Guilloche class exposes a set of animatable parameters (see header) and a root scene node handle which
// can be included in a scene graph.
//
Guilloche::Guilloche(uint32_t numControl, const Vec2 *control1, const Vec2 *control2, uint32_t numSteps, uint32_t numLines) :
   m_root(New),
   m_material(New)
{
   if (numControl > MAX_CONTROLS)
      BSG_THROW("Guilloche: Too many control points (maximum " << MAX_CONTROLS << ")");

   EffectHandle    effect(New);
   effect->Load("bcm_guilloche.bfx");
   m_material->SetEffect(effect);

   std::vector<Vec2> ctrl;

   CopyControl(ctrl, numControl, control1);
   m_material->SetUniformValue("u_control1", ctrl);

   CopyControl(ctrl, numControl, control2);
   m_material->SetUniformValue("u_control2", ctrl);

   Mat4 catmulRom(-1.0f,  3.0f, -3.0f,  1.0f,
                   2.0f, -5.0f,  4.0f, -1.0f,
                  -1.0f,  0.0f,  1.0f,  0.0f,
                   0.0f,  2.0f,  0.0f,  0.0f);

   catmulRom = catmulRom.Transpose() * 0.5f;

   Vec4  white(1.0f);

   m_material->SetUniform("u_width",        0.01f);
   m_material->SetUniform("u_basis",        catmulRom);
   m_material->SetUniform("u_color1Start",  white);
   m_material->SetUniform("u_color1Finish", white);
   m_material->SetUniform("u_color2Start",  white);
   m_material->SetUniform("u_color2Finish", white);
   m_material->SetUniform("u_num_control",  (int)numControl);
   m_material->SetUniform("u_offsetX",      0.0f);
   m_material->SetUniform("u_offsetY",      0.0f);
   m_material->SetUniform("u_scaleStart",   1.0f);
   m_material->SetUniform("u_scaleFinish",  1.0f);

   GeometryHandle  geom = GuillocheFactory(numSteps, numLines).MakeGeometry(m_material);
   m_root->AppendGeometry(geom);
}

// SetColorRamp
//
// Sets the start and end colors of the two lines.
// Line between are interpolated
void Guilloche::SetColorRamp(const Vec4 &c1Start, const Vec4 &c1Finish, const Vec4 &c2Start, const Vec4 &c2Finish)
{
   m_material->SetUniform("u_color1Start",  c1Start);
   m_material->SetUniform("u_color1Finish", c1Finish);

   m_material->SetUniform("u_color2Start",  c2Start);
   m_material->SetUniform("u_color2Finish", c2Finish);
}

// SetScale
//
// Sets the scale factor for the start and end of the curve
// Scale is linearly interpolated from start to end
void Guilloche::SetScale(float horizScale, float vertScaleStart, float vertScaleFinish)
{
   m_material->SetUniform("u_horizScale",      horizScale);
   m_material->SetUniform("u_vertScaleStart",  vertScaleStart);
   m_material->SetUniform("u_vertScaleFinish", vertScaleFinish);
}

// SetWidth
//
// Sets the width of lines in NDC screen (0 .. 1) space
//
void Guilloche::SetWidth(float width, bool useY)
{
   IVec2 dim = Application::Instance()->GetQuadRender().GetDimensions();

   m_material->SetUniform("u_width", width * (useY ? dim.Y() : dim.X()));
}

////////////////////////////////////////////////////////////////////////////////////////////

// GuillochePanel
//
// The Guilloche panel uses the guilloche to provide a stand-alone renderable object.
// It can be positioned using pos and rot.
//
// pos runs 0 <= x <= 1 and 0 <= y <= 1 over the display (y positive up)
// rot is the angle of rotation about the left centre of the waveform 
GuillochePanel::GuillochePanel(const Guilloche &guilloche, const Vec2 &pos, float rot) :
   m_guilloche(guilloche),
   m_camera(New),
   m_root(New)
{
   m_camera->SetType(Camera::eORTHOGRAPHIC);
   m_camera->SetClippingPlanes(0.0f, 10.0f);
   m_camera->SetYFov(1.0f);

   SceneNodeHandle   cameraNode(New);
   cameraNode->SetPosition(Vec3(0.0f, 0.0f, 1.0f));
   cameraNode->SetCamera(m_camera);

   SceneNodeHandle   guillocheMove(New);
   SceneNodeHandle   guillocheRot(New);

   guillocheMove->SetPosition(Vec3(pos.X() - 0.5f, pos.Y() - 0.5f, 0.0f));
   guillocheMove->AppendChild(guillocheRot);   

   guillocheRot->SetRotation(rot, Vec3(0.0f, 0.0f, 1.0f));
   guillocheRot->AppendChild(m_guilloche.GetRoot());   

   m_root->AppendChild(guillocheMove);
   m_root->AppendChild(cameraNode);
}

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

#include "body.h"
#include "bsg_scene_node.h"
#include "bsg_exception.h"
#include "bsg_parse_utils.h"
#include "bsg_shape.h"
#include "bsg_animator.h"
#include "bsg_scene_node_callback.h"
#include "bsg_semantic_data.h"
#include "bsg_lod_group.h"

#include "planets.h"
#include "align.h"

using namespace bsg;
using namespace std;

class AlignQuad : public CallbackOnRenderData
{
public:
   AlignQuad(float radius) : m_radius2(radius * radius) {}

   virtual bool OnRenderData(SemanticData &semData)
   {
      Mat4  mv = semData.GetModelViewMatrix();

      Vec3  from(0.0f, 0.0f, 0.0f);
      Vec3  to(mv(0, 3), mv(1, 3), mv(2, 3));
      Vec3  up(mv.Drop() * Vec3(0.0f, 1.0f, 0.0f));

      Mat4 rot = LookAt(to, from, up);

      float dist   = Length(Vec3(mv(0, 3), mv(1, 3), mv(2, 3)));
      float offset = m_radius2 / dist;

      rot = rot * Translate(0.0f, 0.0f, -offset / 2.0f);

      semData.SetModelViewMatrix(rot);

      return true;
   }

private:
   float m_radius2;
};

class InsideBody : public CallbackOnRenderData
{
public:
   InsideBody(float radius) : m_radius(radius + 1.1f) {}

   virtual bool OnRenderData(SemanticData &semData)
   {
      Vec4 center = semData.GetModelViewMatrix() * Vec4(0.0f, 0.0f, 0.0f, 1.0f);

      return Length(center.Proj()) > m_radius;
   }

private:
   float m_radius;
};

static string GetLine(istream &is, uint32_t lineNumber)
{
   bool     gotLine = false;
   string   line;

   while (!gotLine && is.good())
   {
      getline(is, line);
      lineNumber += 1;
      gotLine = true;

      line = ParseUtils::StripWhite(line);
      line = ParseUtils::RemoveDelims(line, ";=,");

      if (line.length() == 0)
         gotLine = false;

      if (line.length() >= 2 && line[0] == '/' && line[1] == '/')
         gotLine = false;
   }

   return line;
}

void Body::Initialise()
{
   m_surfaceFile      = "undefined";
   m_radius           = 0.0f;
   m_oblateness       = 0.0f;
   m_axialTilt        = 0.0f;
   m_day              = 0.0f;
   m_cloudDay         = 0.0f;
   m_cloudFile        = "";
   m_orbitRadius      = 0.0f;
   m_orbitTilt        = 0.0f;
   m_eccentricity     = 0.0f;
   m_year             = 0.0f;
   m_phase            = 0.0f;
   m_ringInner        = 0.0f;
   m_ringOuter        = 0.0f;
   m_ringFile         = "";
   m_state            = eEXPECT_OPEN;
   m_atmosphereHeight = 0.0f;
   m_light            = false;
}

Body::Body() :
   m_name("undefined")
{
   Initialise();
}

Body::Body(istream &is, const string &name, uint32_t &lineNumber, const BodyTable &symbols) :
   m_name(name)
{
   Initialise();

   if (!is)
      BSG_THROW("Invalid stream");

   try
   {
      bool done = false;

      while (!done && is.good())
      {
         string line = GetLine(is, lineNumber);

         done = ParseLine(line, symbols);
      }
   }
   catch (string s)
   {
      BSG_THROW("Parse error at line " << lineNumber << ": " << s);
   }
}

static void ParseValue(float &value, istringstream &iss)
{
   iss >> value;
}

static void ParseValue(string &value, istringstream &iss)
{
   iss >> value;
}

bool Body::ParseLine(const string &line, const BodyTable &symbols)
{
   if (line.length() == 0) // Blank line
      return false;

   if (line.length() >= 2 && line[0] == '/' && line[1] == '/')    // Comment
      return false;

   istringstream iss(line, istringstream::in);
   string token;

   switch (m_state)
   {
   case eEXPECT_OPEN:
      iss >> token;
      if (token == "{")
         m_state = eIN_BODY;
      else
         throw "Expected open brace";
      break;

   case eIN_BODY:
      iss >> token;
      if (token == "}")
         return true;

      if (token == "radius")
      {
         ParseValue(m_radius, iss);
         ParseValue(m_realRadius, iss);
      }
      else if (token == "oblateness")
         ParseValue(m_oblateness, iss);
      else if (token == "axialTilt")
         ParseValue(m_axialTilt, iss);
      else if (token == "day")
         ParseValue(m_day, iss);
      else if (token == "orbitRadius")
      {
         ParseValue(m_orbitRadius, iss);
         ParseValue(m_realOrbitRadius, iss);
      }
      else if (token == "orbitTilt")
         ParseValue(m_orbitTilt, iss);
      else if (token == "eccentricity")
         ParseValue(m_eccentricity, iss);
      else if (token == "year")
         ParseValue(m_year, iss);
      else if (token == "phase")
         ParseValue(m_phase, iss);
      else if (token == "satellites")
         ParseSatellites(iss, symbols);
      else if (token == "surface")
         ParseValue(m_surfaceFile, iss);
      else if (token == "ring")
         ParseRing(iss);
      else if (token == "clouds")
         ParseClouds(iss);
      else if (token == "atmosphere")
         ParseAtmosphere(iss);
      else if (token == "light")
         m_light = true;
      else
         UnexpectedToken(token);

      break;

   default:
      throw "Internal error";
      break;
   }

   return false;
}

void Body::UnexpectedToken(const string &token) const
{
   throw string("Unexpected token '") + token + "'";
}

void Body::ParseRing(istringstream &iss)
{
   iss >> m_ringInner;
   iss >> m_ringOuter;
   iss >> m_ringFile;
}

void Body::ParseClouds(istringstream &iss)
{
   iss >> m_cloudDay;
   iss >> m_cloudFile;
}

void Body::ParseAtmosphere(istringstream &iss)
{
   float r, g, b;

   iss >> r >> g >> b;
   m_atmosphereColor = Vec3(r, g, b);

   iss >> m_atmosphereHeight;
}

void Body::ParseSatellites(istringstream &iss, const BodyTable &symbols)
{
   string   token;

   while (iss.good())
   {
      iss >> token;

      if (iss.good())
      {
         BodyTable::const_iterator   iter = symbols.find(token);

         if (iter == symbols.end())
            throw string("Body '") + token + "' not found";

         const Body  &body = iter->second;

         m_satellites.push_back(&body);
      }
   }
}

void Body::CreatePlanetGeometry(SceneNodeHandle &node, const PlanetEffects &effects) const
{
   MaterialHandle material(New, m_name);

   if (IsLight())
   {
      material->SetEffect(effects.GetSunEffect());
   }
   else
   {
      Planets::Instance()->AddMaterial(material);

      if (HasClouds())
      {
         material->SetEffect(effects.GetSurfaceCloudsEffect());

         TextureHandle  cloudTex(New, m_name + "_cloud");
         // TODO really only need a one or two channel texture for earth clouds
         cloudTex->TexImage2D(ImageSet(GetCloudFile(), "pkm", Image::eETC1));
         material->SetTexture("u_cloudTex", cloudTex);

         Time now = Planets::Instance()->FrameTimestamp();

         if (m_day - m_cloudDay != 0.0f)
         {
            AnimBindingLerpFloat *rotator = new AnimBindingLerpFloat(&material->GetUniform1f("u_cloudOffset"));
            rotator->Interpolator()->Init(now, now + (m_day / (m_day - m_cloudDay)), BaseInterpolator::eREPEAT);
            rotator->Evaluator()->Init(0.0f, 1.0f);
            Planets::Instance()->AddAnimation(rotator);
         }
      }
      else
      {
         material->SetEffect(effects.GetSurfaceEffect());
      }
   }

   TextureHandle surfaceTex(New, m_name);
   surfaceTex->TexImage2D(ImageSet(GetSurfaceFile(), "pkm", Image::eETC1));
   material->SetTexture("u_surfaceTex", surfaceTex);

   SurfaceHandle  sphere      = effects.GetSphereSurface();
   GeometryHandle geom1(New);
   geom1->AppendSurface(sphere, material);
   node->AppendGeometry(geom1);

   if (Planets::Instance()->GetExtraOptions().GetUseLOD())
   {
      SurfaceHandle  smallSphere = effects.GetSmallSphereSurface();

      GeometryHandle geom2(New);
      geom2->AppendSurface(smallSphere, material);

      LODGroup  lodGroup;
      lodGroup.Begin();
      lodGroup.Add(geom1, 100.0f);
      lodGroup.Add(geom2);
      lodGroup.End();

      node->AppendGeometry(geom2);
   }

   float radius = GetRadius();
   node->SetScale(Vec3(radius, radius * (1.0f - GetOblateness()), radius));
   node->SetCallback(new InsideBody(GetRadius()));
}

SceneNodeHandle Body::CreateAtmosphere(const PlanetEffects &effects) const
{
   SceneNodeHandle   node(New, m_name + "_atmosphere");

   MaterialHandle material(New, m_name + "_atmosphere");
   material->SetEffect(effects.GetAtmosphereEffect());
   material->SetUniformValue("u_color", GetAtmosphereColor());
   // Smaller to prevent gaps
   material->SetUniformValue("u_inner", GetRadius() / GetAtmosphereRadius());

   float          size = GetAtmosphereRadius() * 2.0f;
   GeometryHandle geom = QuadFactory(size, size, 0.0f, eZ_AXIS).MakeGeometry(material);

   //geom->GetSurface(0)->SetViewFrustumCull(false);

   geom->SetCull(false);
   node->SetCallback(new AlignQuad(GetRadius()));
   node->SetCallback(new InsideBody(GetRadius()));
   node->AppendGeometry(geom);

   return node;
}

GeometryHandle Body::CreateRingGeometry(const PlanetEffects &effects) const
{
   MaterialHandle material(New, m_name + "_ring");
   material->SetEffect(effects.GetRingEffect());

   TextureHandle texture(New, m_name + "_ring");
   texture->TexImage2D(ImageSet(GetRingFile(), "png", Image::eRGBA8888));
   material->SetTexture("u_ringTex", texture);
   material->SetUniformValue("u_inner", GetRingInner() / GetRingOuter());

   float ringSize = GetRingOuter() * 2.0f;
   GeometryHandle geom = QuadFactory(ringSize, ringSize, 0.0f, eY_AXIS).MakeGeometry(material);
   geom->SetCull(false);

   return geom;
}

SceneNodeHandle Body::CreateGraph(const PlanetEffects &effects) const
{
   // Tree is either top->system->planet->body when implemeting axial and orbital tilts
   // Or system->body without the tilts included
   bool  tilt = Planets::Instance()->GetExtraOptions().GetTilt();

   SceneNodeHandle   topNode;
   SceneNodeHandle   systemNode(New, m_name + "_system");
   SceneNodeHandle   planetNode;
   SceneNodeHandle   bodyNode(New, m_name);

   if (tilt)
   {
      topNode    = SceneNodeHandle(New, m_name + "_top");
      planetNode = SceneNodeHandle(New, m_name + "_planet");
      topNode->AppendChild(systemNode);
      systemNode->AppendChild(planetNode);
      planetNode->AppendChild(bodyNode);
   }
   else
   {
      systemNode->AppendChild(bodyNode);
   }

   CreatePlanetGeometry(bodyNode, effects);

   // Make the body rotate
   Time now = Planets::Instance()->FrameTimestamp();

   AnimBindingLerpQuaternionAngle *rotator = new AnimBindingLerpQuaternionAngle(&bodyNode->GetRotation());
   rotator->Interpolator()->Init(now, now + std::abs(GetDay()), BaseInterpolator::eREPEAT);
   rotator->Evaluator()->Init(Vec3(0.0f, -1.0f, 0.0f), GetDay() < 0.0f ? 0.0f : 360.0f, GetDay() < 0.0f ? 360.0f : 0.0f);
   Planets::Instance()->AddAnimation(rotator);

   if (HasRings())
   {
      SceneNodeHandle   ringNode(New, m_name + "_ring");

      if (tilt)
         planetNode->AppendChild(ringNode);
      else
         systemNode->AppendChild(ringNode);

      AnimBindingLerpQuaternionAngle *ringRot = new AnimBindingLerpQuaternionAngle(&ringNode->GetRotation());
      ringRot->Interpolator()->Init(now, now + std::abs(GetYear()), BaseInterpolator::eREPEAT);
      ringRot->Evaluator()->Init(Vec3(0.0f, -1.0f, 0.0f), GetYear() < 0.0f ? 0.0f : 360.0f, GetYear() < 0.0f ? 360.0f : 0.0f);
      Planets::Instance()->AddAnimation(ringRot);

      ringNode->AppendGeometry(CreateRingGeometry(effects));
   }

   if (HasAtmosphere())
      bodyNode->AppendChild(CreateAtmosphere(effects));

   for (BodyList::const_iterator iter = m_satellites.begin(); iter != m_satellites.end(); ++iter)
   {
      const Body        *childBody   = *iter;
      SceneNodeHandle   childTopNode = childBody->CreateGraph(effects);
      SceneNodeHandle   childNode;
      
      if (tilt)
         childNode = childTopNode->GetChild(0);
      else
         childNode = childTopNode;

      // Make it orbit it's parent
      AnimBindingLerpCircle   *orbiter = new AnimBindingLerpCircle(&childNode->GetPosition());
      orbiter->Interpolator()->Init(now, now + std::abs(childBody->GetYear()), BaseInterpolator::eREPEAT);
      orbiter->Evaluator()->Init(Circle(childBody->GetOrbitRadius(), childBody->GetPhase(), eY_AXIS, childBody->GetYear() < 0.0f ? eCW_WINDING : eCCW_WINDING));
      Planets::Instance()->AddAnimation(orbiter);

      if (tilt)
      {
         childTopNode->SetRotation(childBody->GetOrbitTilt(), Vec3(0.0f, 0.0f, 1.0f));
         planetNode->AppendChild(childTopNode);
      }
      else
      {
         systemNode->AppendChild(childNode);
      }
   }

   if (tilt)
      planetNode->SetRotation(GetAxialTilt(), Vec3(0.0f, 0.0f, 1.0f));

   return systemNode;
}

BodyBuilder::BodyBuilder(istream &is)
{
   if (!is)
      BSG_THROW("Invalid stream");

   uint32_t lineNumber = 1;

   while (is.good())
   {
      string line = GetLine(is, lineNumber);

      istringstream   ss(line);
      string   tok;
      string   name;

      ss >> tok >> name;

      if (tok == "BODY")
         m_symbols[name] = Body(is, name, lineNumber, m_symbols);
      else if (tok == "ROOT")
         m_root = name;
      else if (tok != "")
         BSG_THROW("Unrecognised word: " << tok);
   }
}

const Body *BodyBuilder::Find(const string &name) const
{
   map<string, Body>::const_iterator   iter = m_symbols.find(name);

   if (iter == m_symbols.end())
      return 0;

   return &(*iter).second;
}

const Body *BodyBuilder::GetRoot() const
{
   return Find(m_root);
}

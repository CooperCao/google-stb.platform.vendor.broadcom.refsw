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

#include "bsg_pass_state.h"
#include "bsg_parse_utils.h"
#include "bsg_exception.h"

using namespace std;

namespace bsg
{

class PassStateValue
{
public:
   void Set(const string &str, char type)
   {
      istringstream iss(str, istringstream::in);

      switch (type)
      {
      case 'f' : iss >> m_float; break;
      case 'i' : iss >> m_int; break;
      case 'u' : iss >> m_uint; break;
      case 'b' : 
      {
         string bs;
         iss >> bs; 
         if (bs == "true" || bs == "TRUE" || bs == "1")
            m_bool = true;
         else
            m_bool = false;
         break;
      }
      case 'e' : 
      {
         string eStr;
         iss >> eStr;
         m_enum = PassStateEnum::MapEnum(eStr);
         break;
      }
      default  : BSG_THROW("Unknown type"); break;
      }
   }
   float  F() const { return m_float; }
   GLenum E() const { return m_enum; }
   GLint  I() const { return m_int; }
   GLuint U() const { return m_uint; }
   bool B() const { return m_bool; }

private:
   float  m_float;
   GLenum m_enum;
   GLint  m_int;
   GLuint m_uint;
   bool  m_bool;
};

#define EMAP(a) m_enumMap.insert(pair<std::string, GLenum>(#a, GL_##a))

bool                          PassStateEnum::m_enumMapInited = false;
std::map<std::string, GLenum> PassStateEnum::m_enumMap;

void PassStateEnum::FillMap()
{
   EMAP(FALSE);
   EMAP(TRUE);
   EMAP(ZERO);
   EMAP(ONE);
   EMAP(SRC_COLOR);
   EMAP(ONE_MINUS_SRC_COLOR);
   EMAP(SRC_ALPHA);
   EMAP(ONE_MINUS_SRC_ALPHA);
   EMAP(DST_ALPHA);
   EMAP(ONE_MINUS_DST_ALPHA);
   EMAP(DST_COLOR);
   EMAP(ONE_MINUS_DST_COLOR);
   EMAP(SRC_ALPHA_SATURATE);
   EMAP(FUNC_ADD);
   EMAP(BLEND_EQUATION);
   EMAP(BLEND_EQUATION_RGB);
   EMAP(BLEND_EQUATION_ALPHA);
   EMAP(FUNC_SUBTRACT);
   EMAP(FUNC_REVERSE_SUBTRACT);
   EMAP(BLEND_DST_RGB);
   EMAP(BLEND_SRC_RGB);
   EMAP(BLEND_DST_ALPHA);
   EMAP(BLEND_SRC_ALPHA);
   EMAP(CONSTANT_COLOR);
   EMAP(ONE_MINUS_CONSTANT_COLOR);
   EMAP(CONSTANT_ALPHA);
   EMAP(ONE_MINUS_CONSTANT_ALPHA);
   EMAP(BLEND_COLOR);
   EMAP(FRONT);
   EMAP(BACK);
   EMAP(FRONT_AND_BACK);
   EMAP(CW);
   EMAP(CCW);
   EMAP(NEVER);
   EMAP(LESS);
   EMAP(EQUAL);
   EMAP(LEQUAL);
   EMAP(GREATER);
   EMAP(NOTEQUAL);
   EMAP(GEQUAL);
   EMAP(ALWAYS);
   EMAP(KEEP);
   EMAP(REPLACE);
   EMAP(INCR);
   EMAP(DECR);
   EMAP(INVERT);
   EMAP(INCR_WRAP);
   EMAP(DECR_WRAP);
   EMAP(NEAREST);
   EMAP(LINEAR);
   EMAP(NEAREST_MIPMAP_NEAREST);
   EMAP(LINEAR_MIPMAP_NEAREST);
   EMAP(NEAREST_MIPMAP_LINEAR);
   EMAP(LINEAR_MIPMAP_LINEAR);
   EMAP(REPEAT);
   EMAP(CLAMP_TO_EDGE);
   EMAP(MIRRORED_REPEAT);

   m_enumMapInited = true;
}

GLenum PassStateEnum::MapEnum(const string &name)
{
   if (!m_enumMapInited)
      FillMap();

   auto iter = m_enumMap.find(name);

   if (iter != m_enumMap.end())
      return iter->second;

   return (GLenum)0;
}

//////////////////////////////////////////////////////////////////////////////

bool                                 PassState::m_parseMapInited = false;
std::map<std::string, PassStateInfo> PassState::m_parseMap;

#define PMAP(func, proto) m_parseMap.insert(pair<std::string, PassStateInfo>(#func, PassStateInfo(PassStateInfo:: func, proto)))

void PassState::FillParseMap()
{
   // Syntax for each parse item -- commented out ones should not be under the control of the effects.
   PMAP(BlendColor, "ffff");
   PMAP(BlendEquation, "e");
   PMAP(BlendEquationSeparate, "ee");
   PMAP(BlendFunc, "ee");
   PMAP(BlendFuncSeparate, "eeee");
   PMAP(ColorMask, "bbbb");
   // PMAP(CullFace, "e");                      // Geometry
   PMAP(DepthFunc, "e");
   PMAP(DepthMask, "b");
   // PMAP(DepthRangef, "ff");                  // Renderer
   // PMAP(FrontFace, "e");                     // Geometry
   // PMAP(LineWidth, "f");                     // Geometry
   // PMAP(PolygonOffset, "ff");                // Discourage this
   PMAP(StencilFunc, "eiu");                    // Render property
   PMAP(StencilFuncSeparate, "eeiu");
   PMAP(StencilMask, "u");
   PMAP(StencilMaskSeparate, "eu");
   PMAP(StencilOp, "eee");
   PMAP(StencilOpSeparate, "eeee");
   // PMAP(EnableCullFace, "b");                // Geometry
   // PMAP(EnablePolygonOffsetFill, "b");       // Discourage
   // PMAP(EnableSampleAlphaToCoverage, "b");   // Render property
   // PMAP(EnableSampleCoverage, "b");          // Render
   // PMAP(EnableScissorTest, "b");             // Render
   PMAP(EnableStencilTest, "b");
   PMAP(EnableDepthTest, "b");
   PMAP(EnableBlend, "b");
   // PMAP(EnableDither, "b");                  // Render

   m_parseMapInited = true;
}

void PassState::Invoke(uint32_t dirty) const
{
   if (dirty == 0)
      return;

   if (dirty & BlendColor)
      glBlendColor(m_blendColor[0], m_blendColor[1], m_blendColor[2], m_blendColor[3]);

   if (dirty & BlendEquationSeparate)
      glBlendEquationSeparate(m_blendEquationSeparate[0], m_blendEquationSeparate[1]);

   if (dirty & BlendFuncSeparate)
      glBlendFuncSeparate(m_blendFuncSeparate[0], m_blendFuncSeparate[1], m_blendFuncSeparate[2], m_blendFuncSeparate[3]);

   if (dirty & ColorMask)
      glColorMask(m_colorMask[0], m_colorMask[1], m_colorMask[2], m_colorMask[3]);

   if (dirty & CullFace)
      glCullFace(m_cullFace);

   if (dirty & DepthFunc)
      glDepthFunc(m_depthFunc);

   if (dirty & DepthMask)
      glDepthMask(m_depthMask);

   if (dirty & DepthRange)
      glDepthRangef(m_depthRange[0], m_depthRange[1]);

   if (dirty & FrontFace)
      glFrontFace(m_frontFace);

   if (dirty & LineWidth)
      glLineWidth(m_lineWidth);

   if (dirty & PolygonOffset)
      glPolygonOffset(m_polygonOffset[0], m_polygonOffset[1]);

   if (dirty & StencilFuncSeparate)
   {
      glStencilFuncSeparate(GL_FRONT, m_stencilFuncFront[0], m_stencilFuncFront[1], m_stencilFuncFront[2]);
      glStencilFuncSeparate(GL_BACK,  m_stencilFuncBack[0] , m_stencilFuncBack[1] , m_stencilFuncBack[2] );
   }

   if (dirty & StencilMaskSeparate)
   {
      glStencilMaskSeparate(GL_FRONT, m_stencilMaskFront);
      glStencilMaskSeparate(GL_BACK , m_stencilMaskBack );
   }

   if (dirty & StencilOpSeparate)
   {
      glStencilOpSeparate(GL_FRONT, m_stencilOpFront[0], m_stencilOpFront[1], m_stencilOpFront[2]);
      glStencilOpSeparate(GL_BACK , m_stencilOpBack[0] , m_stencilOpBack[1] , m_stencilOpBack[2] );
   }

   if (dirty & EnableCullFace)
   {
      if (m_enableCullFace)
         glEnable(GL_CULL_FACE);
      else
         glDisable(GL_CULL_FACE);
   }

   if (dirty & EnablePolygonOffsetFill)
   {
      if (m_enablePolygonOffsetFill)
         glEnable(GL_POLYGON_OFFSET_FILL);
      else
         glDisable(GL_POLYGON_OFFSET_FILL);
   }

   if (dirty & EnableSampleAlphaToCoverage)
   {
      if (m_enableSampleAlphaToCoverage)
         glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
      else
         glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
   }

   if (dirty & EnableSampleCoverage)
   {
      if (m_enableSampleCoverage)
         glEnable(GL_SAMPLE_COVERAGE);
      else
         glDisable(GL_SAMPLE_COVERAGE);
   }

   if (dirty & EnableScissorTest)
   {
      if (m_enableScissorTest)
         glEnable(GL_SCISSOR_TEST);
      else
         glDisable(GL_SCISSOR_TEST);
   }

   if (dirty & EnableStencilTest)
   {
      if (m_enableStencilTest)
         glEnable(GL_STENCIL_TEST);
      else
         glDisable(GL_STENCIL_TEST);
   }

   if (dirty & EnableDepthTest)
   {
      if (m_enableDepthTest)
         glEnable(GL_DEPTH_TEST);
      else
         glDisable(GL_DEPTH_TEST);
   }

   if (dirty & EnableBlend)
   {
      if (m_enableBlend)
         glEnable(GL_BLEND);
      else
         glDisable(GL_BLEND);
   }

   if (dirty & EnableDither)
   {
      if (m_enableDither)
         glEnable(GL_DITHER);
      else
         glDisable(GL_DITHER);
   }
}

void PassState::SetTo(const PassState &mat)
{
   // The "toSet" bits are the pieces of state that are requested in the mat state modifier
   uint32_t toSet  = mat.m_dirty;

   // The revert bits are those pieces of state that have been modified in "this" state, and which
   // need to be reset.  There is no need to reset anything in the "toSet" since this will
   // be modified below
   uint32_t revert = m_dirty & ~toSet;

   // Revert the changed state
   Invoke(revert);

   // Set new state
   mat.Set();

   // Record the new dirty states
   m_dirty = toSet;
}

void PassState::ParseArguments(const string &stateIdent, const string &argStr)
{
   auto iter = m_parseMap.find(stateIdent);
   if (iter == m_parseMap.end())
      BSG_THROW("Unknown state identifier");

   string argTemplate = iter->second.m_argSpec;
   int    start = 0;
   int    comma = -1;

   PassStateValue v[4];  // Max 4 args currently

   for (uint32_t c = 0; c < argTemplate.length(); c++)
   {
      char type = argTemplate[c];
      comma = argStr.find_first_of(",", start);
      if (comma == (int)argStr.npos)
      {
         if (c < argTemplate.length() - 1)
            BSG_THROW("Wrong number of arguments for state");
         else
            comma = argStr.length();
      }
      
      v[c].Set(ParseUtils::StripWhite(argStr.substr(start, comma - start)), type);

      start = comma + 1;
   }

   switch (iter->second.m_func)
   {
   case PassStateInfo::BlendColor                  : SetBlendColor(v[0].F(), v[1].F(), v[2].F(), v[3].F()); break;
   case PassStateInfo::BlendEquation               : SetBlendEquation(v[0].E()); break;
   case PassStateInfo::BlendEquationSeparate       : SetBlendEquationSeparate(v[0].E(), v[1].E()); break;
   case PassStateInfo::BlendFunc                   : SetBlendFunc(v[0].E(), v[1].E()); break;
   case PassStateInfo::BlendFuncSeparate           : SetBlendFuncSeparate(v[0].E(), v[1].E(), v[2].E(), v[3].E()); break;
   case PassStateInfo::ColorMask                   : SetColorMask(v[0].B(), v[1].B(), v[2].B(), v[3].B()); break;
   case PassStateInfo::CullFace                    : SetCullFace(v[0].E()); break;
   case PassStateInfo::DepthFunc                   : SetDepthFunc(v[0].E()); break;
   case PassStateInfo::DepthMask                   : SetDepthMask(v[0].B()); break;
   case PassStateInfo::DepthRangef                 : SetDepthRange(v[0].F(), v[1].F()); break;
   case PassStateInfo::FrontFace                   : SetFrontFace(v[0].E()); break;
   case PassStateInfo::LineWidth                   : SetLineWidth(v[0].F()); break;
   case PassStateInfo::PolygonOffset               : SetPolygonOffset(v[0].F(), v[1].F()); break;
   case PassStateInfo::StencilFunc                 : SetStencilFunc(v[0].E(), v[1].I(), v[2].U()); break;
   case PassStateInfo::StencilFuncSeparate         : SetStencilFuncSeparate(v[0].E(), v[1].E(), v[2].I(), v[3].U()); break;
   case PassStateInfo::StencilMask                 : SetStencilMask(v[0].U()); break;
   case PassStateInfo::StencilMaskSeparate         : SetStencilMaskSeparate(v[0].E(), v[1].U()); break;
   case PassStateInfo::StencilOp                   : SetStencilOp(v[0].E(), v[1].E(), v[2].E()); break;
   case PassStateInfo::StencilOpSeparate           : SetStencilOpSeparate(v[0].E(), v[1].E(), v[2].E(), v[3].E()); break;
   case PassStateInfo::EnableCullFace              : SetEnableCullFace(v[0].B()); break;
   case PassStateInfo::EnablePolygonOffsetFill     : SetEnablePolygonOffsetFill(v[0].B()); break;
   case PassStateInfo::EnableSampleAlphaToCoverage : SetEnableSampleAlphaToCoverage(v[0].B()); break;
   case PassStateInfo::EnableSampleCoverage        : SetEnableSampleCoverage(v[0].B()); break;
   case PassStateInfo::EnableScissorTest           : SetEnableScissorTest(v[0].B()); break;
   case PassStateInfo::EnableStencilTest           : SetEnableStencilTest(v[0].B()); break;
   case PassStateInfo::EnableDepthTest             : SetEnableDepthTest(v[0].B()); break;
   case PassStateInfo::EnableBlend                 : SetEnableBlend(v[0].B()); break;
   case PassStateInfo::EnableDither                : SetEnableDither(v[0].B()); break;
   default                                         : BSG_THROW("Invalid state func"); break;
   }
}

void PassState::ParseLine(const std::string &stateStr)
{
   int equal = stateStr.find_first_of("=");
   string l = ParseUtils::StripWhite(stateStr.substr(0, equal - 1));
   string r = ParseUtils::StripWhite(stateStr.substr(equal + 1, stateStr.npos));

   ParseArguments(l, r);
}

void PassState::Parse(const std::string &stateStr)
{
   if (!m_parseMapInited)
      FillParseMap();

   int start = 0;
   int semi;

   do
   {
      semi = stateStr.find_first_of(";", start);
      if (semi != (int)stateStr.npos)
         ParseLine(stateStr.substr(start, semi - start));

      start = semi + 1;
   }
   while (semi != (int)stateStr.npos);

}

static string ToString(const Vec4 &v)
{
   stringstream   ss;

   ss << v.X() << ", " << v.Y() << ", " << v.Z() << ", " << v.W();

   return ss.str();
}

static string ToString(bool b)
{
   return b ? "true" : "false";
}

static string BVec4ToString(const IVec4 &v)
{
   return ToString(v[0] != 0) + ", " + ToString(v[1] != 0) + ", " + ToString(v[2] != 0) + ", " + ToString(v[3] != 0);
}

static string ToString(const Vec2 &v)
{
   stringstream   ss;

   ss << v.X() << ", " << v.Y();

   return ss.str();
}

static string ToString(float v)
{
   stringstream   ss;

   ss << v;

   return ss.str();
}

static string ToString(uint32_t i)
{
   stringstream   ss;

   hex(ss);

   ss << i;

   return ss.str();
}

#define ENUM(e)   case GL_##e: return #e;

static string EnumToString(GLenum e)
{
   switch (e)
   {
   ENUM(FRONT)
   ENUM(BACK)
   ENUM(FRONT_AND_BACK)
   ENUM(KEEP)
   ENUM(ZERO)
   ENUM(REPLACE)
   ENUM(INCR)
   ENUM(DECR)
   ENUM(INVERT)
   ENUM(INCR_WRAP)
   ENUM(DECR_WRAP)
   ENUM(NEVER)
   ENUM(ALWAYS)
   ENUM(LESS)
   ENUM(LEQUAL)
   ENUM(EQUAL)
   ENUM(GREATER)
   ENUM(GEQUAL)
   ENUM(NOTEQUAL)
   ENUM(ONE)
   ENUM(FUNC_ADD)
   ENUM(FUNC_SUBTRACT)
   ENUM(FUNC_REVERSE_SUBTRACT)
   ENUM(SRC_COLOR)
   ENUM(ONE_MINUS_SRC_COLOR)
   ENUM(DST_COLOR)
   ENUM(ONE_MINUS_DST_COLOR)
   ENUM(SRC_ALPHA)
   ENUM(ONE_MINUS_SRC_ALPHA)
   ENUM(DST_ALPHA)
   ENUM(ONE_MINUS_DST_ALPHA)
   ENUM(CONSTANT_COLOR)
   ENUM(ONE_MINUS_CONSTANT_COLOR)
   ENUM(CONSTANT_ALPHA)
   ENUM(ONE_MINUS_CONSTANT_ALPHA)
   }

   return "";
}

void PassState::ToLines(vector<string> &lines) const
{
   if (m_dirty & BlendColor)
      lines.push_back(string("BlendColor = ") + ToString(m_blendColor) + ";");

   if (m_dirty & BlendEquationSeparate)
      lines.push_back(string("BlendEquationSeparate = ") + EnumToString(m_blendEquationSeparate[0]) + ", " + EnumToString(m_blendEquationSeparate[1]) + ";");

   if (m_dirty & BlendFuncSeparate)
      lines.push_back(string("BlendFuncSeparate = ") + EnumToString(m_blendFuncSeparate[0]) + ", " + EnumToString(m_blendFuncSeparate[1]) + ", " +
                                                       EnumToString(m_blendFuncSeparate[2]) + ", " + EnumToString(m_blendFuncSeparate[3]) + ";");

   if (m_dirty & ColorMask)
      lines.push_back(string("ColorMask = ") + BVec4ToString(m_colorMask) + ";");

   if (m_dirty & CullFace)
      lines.push_back(string("CullFace = ") + EnumToString(m_cullFace) + ";");

   if (m_dirty & DepthFunc)
      lines.push_back(string("DepthFunc = ") + EnumToString(m_depthFunc) + ";");

   if (m_dirty & DepthMask)
      lines.push_back(string("DepthMask = ") + ToString(m_depthMask) + ";");

   if (m_dirty & DepthRange)
      lines.push_back(string("DepthRange = ") + ToString(m_depthRange) + ";");

   if (m_dirty & FrontFace)
      lines.push_back(string("FrontFace = ") + EnumToString(m_frontFace) + ";");

   if (m_dirty & LineWidth)
      lines.push_back(string("LineWidth = ") + ToString(m_lineWidth) + ";");

   if (m_dirty & PolygonOffset)
      lines.push_back(string("PolygonOffset = ") + ToString(m_polygonOffset) + ";");

   if (m_dirty & StencilFuncSeparate)
   {
      if (m_stencilFuncFront[0] == m_stencilFuncBack[0] &&
          m_stencilFuncFront[1] == m_stencilFuncBack[1] &&
          m_stencilFuncFront[2] == m_stencilFuncBack[2])
      {
         lines.push_back(string("StencilFunc = ") + EnumToString(m_stencilFuncFront[0]) + ", " + EnumToString(m_stencilFuncFront[1]) + ", " + EnumToString(m_stencilFuncFront[2]) + ";");
      }
      else
      {
         lines.push_back(string("StencilFuncSeparate = FRONT, ") + EnumToString(m_stencilFuncFront[0]) + ", " + EnumToString(m_stencilFuncFront[1]) + ", " + EnumToString(m_stencilFuncFront[2]) + ";");
         lines.push_back(string("StencilFuncSeparate = BACK,  ") + EnumToString(m_stencilFuncBack[0])  + ", " + EnumToString(m_stencilFuncBack[1])  + ", " + EnumToString(m_stencilFuncBack[2]) + ";");
      }
   }

   if (m_dirty & StencilMaskSeparate)
   {
      if (m_stencilMaskFront == m_stencilMaskBack)
      {
         lines.push_back(string("StencilMask = ") + ToString(m_stencilMaskFront) + ";");
      }
      else
      {
         lines.push_back(string("StencilMaskSeparate = FRONT, ") + ToString(m_stencilMaskFront) + ";");
         lines.push_back(string("StencilMaskSeparate = BACK,  ") + ToString(m_stencilMaskBack) + ";");
      }
   }

   if (m_dirty & StencilOpSeparate)
   {
      if (m_stencilOpFront[0] == m_stencilOpBack[0] &&
          m_stencilOpFront[1] == m_stencilOpBack[1] &&
          m_stencilOpFront[2] == m_stencilOpBack[2])
      {
         lines.push_back(string("StencilOp = ") + EnumToString(m_stencilOpFront[0]) + ", " + EnumToString(m_stencilOpFront[1]) + ", " + EnumToString(m_stencilOpFront[2]) + ";");
      }
      else
      {
         lines.push_back(string("StencilOpSeparate = FRONT, ") + EnumToString(m_stencilOpFront[0]) + ", " + EnumToString(m_stencilOpFront[1]) + ", " + EnumToString(m_stencilOpFront[2]) + ";");
         lines.push_back(string("StencilOpSeparate = BACK,  ") + EnumToString(m_stencilOpBack[0])  + ", " + EnumToString(m_stencilOpBack[1])  + ", " + EnumToString(m_stencilOpBack[2]) + ";");
      }
   }

   if (m_dirty & EnableCullFace)
      lines.push_back(string("EnableCullFace = ") + ToString(m_enableCullFace) + ";");

   if (m_dirty & EnablePolygonOffsetFill)
      lines.push_back(string("EnablePolygonOffsetFill = ") + ToString(m_enablePolygonOffsetFill) + ";");

   if (m_dirty & EnableSampleAlphaToCoverage)
      lines.push_back(string("EnableSampleAlphaToCoverage = ") + ToString(m_enableSampleAlphaToCoverage) + ";");

   if (m_dirty & EnableSampleCoverage)
      lines.push_back(string("EnableSampleCoverage = ") + ToString(m_enableSampleCoverage) + ";");

   if (m_dirty & EnableScissorTest)
      lines.push_back(string("EnableScissorTest = ") + ToString(m_enableScissorTest) + ";");

   if (m_dirty & EnableStencilTest)
      lines.push_back(string("EnableStencilTest = ") + ToString(m_enableStencilTest) + ";");

   if (m_dirty & EnableDepthTest)
      lines.push_back(string("EnableDepthTest = ") + ToString(m_enableDepthTest) + ";");

   if (m_dirty & EnableBlend)
      lines.push_back(string("EnableBlend = ") + ToString(m_enableBlend) + ";");

   if (m_dirty & EnableDither)
      lines.push_back(string("EnableDither = ") + ToString(m_enableDither) + ";");
}

PassState  ShadowState::m_defaultState;

void ShadowState::UpdateGLState(const PassState &mat, const PassState &geom)
{
   // The "toSet" bits are the pieces of state that are requested in the mat and geom state modifiers
   uint32_t toSet  = mat.GetDirty() | geom.GetDirty();

   // The revert bits are those pieces of state that are modified in this state, and which
   // need to be reset.  There is no need to reset anything in the "toSet" since this will
   // be modified below
   uint32_t revert = m_notDefault & ~toSet;

   // Sets the default state back to what "this" requires
   MergeFrom(m_defaultState, revert);
   m_notDefault &= ~revert;

   // Set new state
   MergeFrom(mat, mat.GetDirty());
   MergeFrom(geom, geom.GetDirty());
}

void ShadowState::UpdateGLState(const PassState &state)
{
   // The "toSet" bits are the pieces of state that are requested 
   uint32_t toSet  = state.GetDirty();

   // The revert bits are those pieces of state that are modified in this state, and which
   // need to be reset.  There is no need to reset anything in the "toSet" since this will
   // be modified below
   uint32_t revert = m_notDefault & ~toSet;

   // Sets the default state back to what "this" requires
   MergeFrom(m_defaultState, revert);
   m_notDefault &= ~revert;

   // Set new state
   MergeFrom(state, state.GetDirty());
}

void ShadowState::SetToDefault(bool force)
{
   uint32_t notDefault = force ? 0xffffffff : m_notDefault;

   MergeFrom(m_defaultState, notDefault);
   m_notDefault = 0;
}

void ShadowState::MergeFrom(const PassState &state, uint32_t dirty)
{
   if (dirty == 0)
      return;

   if ((dirty & BlendColor) && (m_blendColor != state.m_blendColor))
   {
      m_blendColor = state.m_blendColor;
      m_notDefault |= BlendColor;
      glBlendColor(m_blendColor[0], m_blendColor[1], m_blendColor[2], m_blendColor[3]);
   }

   if ((dirty & BlendEquationSeparate) && (m_blendEquationSeparate != state.m_blendEquationSeparate))
   {
      m_blendEquationSeparate = state.m_blendEquationSeparate;
      m_notDefault |= BlendEquationSeparate;
      glBlendEquationSeparate(m_blendEquationSeparate[0], m_blendEquationSeparate[1]);
   }

   if ((dirty & BlendFuncSeparate) && (m_blendFuncSeparate != state.m_blendFuncSeparate))
   {
      m_blendFuncSeparate = state.m_blendFuncSeparate;
      m_notDefault |= BlendFuncSeparate;
      glBlendFuncSeparate(m_blendFuncSeparate[0], m_blendFuncSeparate[1], m_blendFuncSeparate[2], m_blendFuncSeparate[3]);
   }

   if ((dirty & ColorMask) && (m_colorMask != state.m_colorMask))
   {
      m_colorMask = state.m_colorMask;
      m_notDefault |= ColorMask;
      glColorMask(m_colorMask[0], m_colorMask[1], m_colorMask[2], m_colorMask[3]);
   }

   if ((dirty & CullFace) && (m_cullFace != state.m_cullFace))
   {
      m_cullFace = state.m_cullFace;
      m_notDefault |= CullFace;
      glCullFace(m_cullFace);
   }

   if ((dirty & DepthFunc) && (m_depthFunc != state.m_depthFunc))
   {
      m_depthFunc = state.m_depthFunc;
      m_notDefault |= DepthFunc;
      glDepthFunc(m_depthFunc);
   }

   if ((dirty & DepthMask) && (m_depthMask != state.m_depthMask))
   {
      m_depthMask = state.m_depthMask;
      m_notDefault |= DepthMask;
      glDepthMask(m_depthMask);
   }

   if ((dirty & DepthRange) && (m_depthRange != state.m_depthRange))
   {
      m_depthRange = state.m_depthRange;
      m_notDefault |= DepthRange;
      glDepthRangef(m_depthRange[0], m_depthRange[1]);
   }

   if ((dirty & FrontFace) && (m_frontFace != state.m_frontFace))
   {
      m_frontFace = state.m_frontFace;
      m_notDefault |= FrontFace;
      glFrontFace(m_frontFace);
   }

   if ((dirty & LineWidth) && (m_lineWidth != state.m_lineWidth))
   {
      m_lineWidth = state.m_lineWidth;
      m_notDefault |= LineWidth;
      glLineWidth(m_lineWidth);
   }

   if ((dirty & PolygonOffset) && (m_polygonOffset != state.m_polygonOffset))
   {
      m_polygonOffset = state.m_polygonOffset;
      m_notDefault |= PolygonOffset;
      glPolygonOffset(m_polygonOffset[0], m_polygonOffset[1]);
   }

   if ((dirty & StencilFuncSeparate) && 
      (m_stencilFuncFront != state.m_stencilFuncFront || m_stencilFuncBack != state.m_stencilFuncBack))
   {
      m_stencilFuncFront = state.m_stencilFuncFront;
      m_stencilFuncBack = state.m_stencilFuncBack;
      m_notDefault |= StencilFuncSeparate;
      glStencilFuncSeparate(GL_FRONT, m_stencilFuncFront[0], m_stencilFuncFront[1], m_stencilFuncFront[2]);
      glStencilFuncSeparate(GL_BACK,  m_stencilFuncBack[0] , m_stencilFuncBack[1] , m_stencilFuncBack[2] );
   }

   if ((dirty & StencilMaskSeparate) && 
      (m_stencilMaskFront != state.m_stencilMaskFront || m_stencilMaskBack != state.m_stencilMaskBack))
   {
      m_stencilMaskFront = state.m_stencilMaskFront;
      m_stencilMaskBack = state.m_stencilMaskBack;
      m_notDefault |= StencilMaskSeparate;
      glStencilMaskSeparate(GL_FRONT, m_stencilMaskFront);
      glStencilMaskSeparate(GL_BACK , m_stencilMaskBack );
   }

   if ((dirty & StencilOpSeparate) && 
      (m_stencilOpFront != state.m_stencilOpFront || m_stencilOpBack != state.m_stencilOpBack))
   {
      m_stencilOpFront = state.m_stencilOpFront;
      m_stencilOpBack = state.m_stencilOpBack;
      m_notDefault |= StencilOpSeparate;
      glStencilOpSeparate(GL_FRONT, m_stencilOpFront[0], m_stencilOpFront[1], m_stencilOpFront[2]);
      glStencilOpSeparate(GL_BACK , m_stencilOpBack[0] , m_stencilOpBack[1] , m_stencilOpBack[2] );
   }

   if ((dirty & EnableCullFace) && (m_enableCullFace != state.m_enableCullFace))
   {
      m_enableCullFace = state.m_enableCullFace;
      m_notDefault |= EnableCullFace;
      if (m_enableCullFace)
         glEnable(GL_CULL_FACE);
      else
         glDisable(GL_CULL_FACE);
   }

   if ((dirty & EnablePolygonOffsetFill) && (m_enablePolygonOffsetFill != state.m_enablePolygonOffsetFill))
   {
      m_enablePolygonOffsetFill = state.m_enablePolygonOffsetFill;
      m_notDefault |= EnablePolygonOffsetFill;
      if (m_enablePolygonOffsetFill)
         glEnable(GL_POLYGON_OFFSET_FILL);
      else
         glDisable(GL_POLYGON_OFFSET_FILL);
   }

   if ((dirty & EnableSampleAlphaToCoverage) && (m_enableSampleAlphaToCoverage != state.m_enableSampleAlphaToCoverage))
   {
      m_enableSampleAlphaToCoverage = state.m_enableSampleAlphaToCoverage;
      m_notDefault |= EnableSampleAlphaToCoverage;
      if (m_enableSampleAlphaToCoverage)
         glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
      else
         glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
   }

   if ((dirty & EnableSampleCoverage) && (m_enableSampleCoverage != state.m_enableSampleCoverage))
   {
      m_enableSampleCoverage = state.m_enableSampleCoverage;
      m_notDefault |= EnableSampleCoverage;
      if (m_enableSampleCoverage)
         glEnable(GL_SAMPLE_COVERAGE);
      else
         glDisable(GL_SAMPLE_COVERAGE);
   }

   if ((dirty & EnableScissorTest) && (m_enableScissorTest != state.m_enableScissorTest))
   {
      m_enableScissorTest = state.m_enableScissorTest;
      m_notDefault |= EnableScissorTest;
      if (m_enableScissorTest)
         glEnable(GL_SCISSOR_TEST);
      else
         glDisable(GL_SCISSOR_TEST);
   }

   if ((dirty & EnableStencilTest) && (m_enableStencilTest != state.m_enableStencilTest))
   {
      m_enableStencilTest = state.m_enableStencilTest;
      m_notDefault |= EnableStencilTest;
      if (m_enableStencilTest)
         glEnable(GL_STENCIL_TEST);
      else
         glDisable(GL_STENCIL_TEST);
   }

   if ((dirty & EnableDepthTest) && (m_enableDepthTest != state.m_enableDepthTest))
   {
      m_enableDepthTest = state.m_enableDepthTest;
      m_notDefault |= EnableDepthTest;
      if (m_enableDepthTest)
         glEnable(GL_DEPTH_TEST);
      else
         glDisable(GL_DEPTH_TEST);
   }

   if ((dirty & EnableBlend) && (m_enableBlend != state.m_enableBlend))
   {
      m_enableBlend = state.m_enableBlend;
      m_notDefault |= EnableBlend;
      if (m_enableBlend)
         glEnable(GL_BLEND);
      else
         glDisable(GL_BLEND);
   }

   if ((dirty & EnableDither) && (m_enableDither != state.m_enableDither))
   {
      m_enableDither = state.m_enableDither;
      m_notDefault |= EnableDither;
      if (m_enableDither)
         glEnable(GL_DITHER);
      else
         glDisable(GL_DITHER);
   }
}



}

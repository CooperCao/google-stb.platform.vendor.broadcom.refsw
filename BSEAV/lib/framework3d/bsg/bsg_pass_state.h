/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_PASS_STATE_H__
#define __BSG_PASS_STATE_H__

#include <string>
#include <map>
#include <vector>
#include "bsg_common.h"
#include "bsg_gl.h"
#include "bsg_vector.h"

namespace bsg
{

class ShadowState;

// @cond
class PassStateInfo
{
public:
   enum StateFunc
   {
      Unknown                    ,
      BlendColor                 ,
      BlendEquation              ,
      BlendEquationSeparate      ,
      BlendFunc                  ,
      BlendFuncSeparate          ,
      ColorMask                  ,
      CullFace                   ,
      DepthFunc                  ,
      DepthMask                  ,
      DepthRangef                ,
      FrontFace                  ,
      LineWidth                  ,
      PolygonOffset              ,
      StencilFunc                ,
      StencilFuncSeparate        ,
      StencilMask                ,
      StencilMaskSeparate        ,
      StencilOp                  ,
      StencilOpSeparate          ,
      EnableCullFace             ,
      EnablePolygonOffsetFill    ,
      EnableSampleAlphaToCoverage,
      EnableSampleCoverage       ,
      EnableScissorTest          ,
      EnableStencilTest          ,
      EnableDepthTest            ,
      EnableBlend                ,
      EnableDither
   };

public:
   PassStateInfo(StateFunc func, const char *spec) :
      m_argSpec(spec),
      m_func(func)
   {}

public:
   std::string m_argSpec;
   StateFunc   m_func;
};

class PassStateEnum
{
public:
   static GLenum MapEnum(const std::string &name);

private:
   static void FillMap();

private:
   static bool                          m_enumMapInited;
   static std::map<std::string, GLenum> m_enumMap;
};

class PassState
{
public:
   // Initialise with default GL state
   PassState() :
      m_dirty                 (0),
      m_blendColor            (Vec4(0, 0, 0, 0)),
      m_blendEquationSeparate (IVec2(GL_FUNC_ADD, GL_FUNC_ADD)),
      m_blendFuncSeparate     (IVec4(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO)),
      m_colorMask             (IVec4(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE)),
      m_depthFunc             (GL_LESS),
      m_depthMask             (GL_TRUE),
      m_depthRange            (Vec2(0.0f, 1.0f)),
      m_frontFace             (GL_CCW),
      m_cullFace              (GL_BACK),
      m_lineWidth             (1.0f),
      m_polygonOffset         (Vec2(0.0f, 0.0f)),
      m_stencilFuncFront      (IVec3(GL_ALWAYS, 0, 0xffffffff)),
      m_stencilFuncBack       (IVec3(GL_ALWAYS, 0, 0xffffffff)),
      m_stencilMaskFront      (0xffffffff),
      m_stencilMaskBack       (0xffffffff),
      m_stencilOpFront        (IVec3(GL_KEEP, GL_KEEP, GL_KEEP)),
      m_stencilOpBack         (IVec3(GL_KEEP, GL_KEEP, GL_KEEP)),

      m_enableCullFace              (false),
      m_enablePolygonOffsetFill     (false),
      m_enableSampleAlphaToCoverage (false),
      m_enableSampleCoverage        (false),
      m_enableScissorTest           (false),
      m_enableStencilTest           (false),
      m_enableDepthTest             (false),
      m_enableBlend                 (false),
      m_enableDither                (false)
   {
   }

   // Parse the state from a string
   void Parse(const std::string &stateStr);

   void SetTo(const PassState &mat);

   // Resets the state according to this's dirty bits and state settings
   void Set() const
   {
      Invoke(m_dirty);
   }

   uint32_t GetDirty() const
   {
      return m_dirty;
   }

   friend class ShadowState;

protected:
   void Dirty(uint32_t dirty)
   {
      m_dirty |= dirty;
   }

   void Clear(uint32_t clear)
   {
      m_dirty &= ~clear;
   }

public:
   void SetBlendColor(float r, float g, float b, float a)
   {
      m_blendColor = Vec4(r, g, b, a);
      Dirty(BlendColor);
   }

   void SetBlendEquationSeparate(GLenum modeRgb, GLenum modeAlpha)
   {
      m_blendEquationSeparate = IVec2(modeRgb, modeAlpha); Dirty(BlendEquationSeparate);
   }

   void SetBlendEquation(GLenum mode)
   {
      SetBlendEquationSeparate(mode, mode);
   }

   void SetBlendFuncSeparate(GLenum srcRgb, GLenum dstRgb, GLenum srcAlpha, GLenum dstAlpha)
   {
      m_blendFuncSeparate = IVec4(srcRgb, dstRgb, srcAlpha, dstAlpha);
      Dirty(BlendFuncSeparate);
   }

   void SetBlendFunc(GLenum src, GLenum dst)
   {
      SetBlendFuncSeparate(src, dst, src, dst);
   }

   void SetColorMask(GLenum r, GLenum g, GLenum b, GLenum a)
   {
      m_colorMask = IVec4(r, g, b, a);
      Dirty(ColorMask);
   }

   void SetCullFace(GLenum mode)
   {
      m_cullFace = mode;
      Dirty(CullFace);
   }

   void SetDepthFunc(GLenum func)
   {
      m_depthFunc = func;
      Dirty(DepthFunc);
   }

   void SetDepthMask(GLenum mask)
   {
      m_depthMask = mask;
      Dirty(DepthMask);
   }

   void SetDepthRange(GLfloat n, GLfloat f)
   {
      m_depthRange = Vec2(n, f);
      Dirty(DepthRange);
   }

   void SetFrontFace(GLenum dir)
   {
      m_frontFace = dir;
      Dirty(FrontFace);
   }

   void SetLineWidth(GLfloat width)
   {
      m_lineWidth = width;
      Dirty(LineWidth);
   }

   void SetPolygonOffset(GLfloat factor, GLfloat units)
   {
      m_polygonOffset = Vec2(factor, units);
      Dirty(PolygonOffset);
   }

   void SetStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask)
   {
      if (face == GL_FRONT || face == GL_FRONT_AND_BACK)
         m_stencilFuncFront = IVec3(func, ref, mask);

      if (face == GL_BACK  || face == GL_FRONT_AND_BACK)
         m_stencilFuncBack  = IVec3(func, ref, mask);

      Dirty(StencilFuncSeparate);
   }

   void SetStencilFunc(GLenum func, GLint ref, GLuint mask)
   {
      SetStencilFuncSeparate(GL_FRONT_AND_BACK, func, ref, mask);
   }

   void SetStencilMaskSeparate(GLenum face, GLuint mask)
   {
      if (face == GL_FRONT || face == GL_FRONT_AND_BACK)
         m_stencilMaskFront = mask;
      if (face == GL_BACK  || face == GL_FRONT_AND_BACK)
         m_stencilMaskBack  = mask;

      Dirty(StencilMaskSeparate);
   }

   void SetStencilMask(GLuint mask)
   {
      SetStencilMaskSeparate(GL_FRONT_AND_BACK, mask);
   }

   void SetStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)
   {
      if (face == GL_FRONT || face == GL_FRONT_AND_BACK)
         m_stencilOpFront = IVec3(sfail, dpfail, dppass);
      if (face == GL_BACK  || face == GL_FRONT_AND_BACK)
         m_stencilOpBack  = IVec3(sfail, dpfail, dppass);

      Dirty(StencilOpSeparate);
   }

   void SetStencilOp(GLenum sfail, GLenum dpfail, GLenum dppass)
   {
      SetStencilOpSeparate(GL_FRONT_AND_BACK, sfail, dpfail, dppass);
   }

   void SetEnableCullFace(bool en)
   {
      m_enableCullFace = en;
      Dirty(EnableCullFace);
   }

   void SetEnablePolygonOffsetFill(bool en)
   {
      m_enablePolygonOffsetFill = en;
      Dirty(EnablePolygonOffsetFill);
   }

   void SetEnableSampleAlphaToCoverage(bool en)
   {
      m_enableSampleAlphaToCoverage = en;
      Dirty(EnableSampleAlphaToCoverage);
   }

   void SetEnableSampleCoverage(bool en)
   {
      m_enableSampleCoverage = en;
      Dirty(EnableSampleCoverage);
   }

   void SetEnableScissorTest(bool en)
   {
      m_enableScissorTest = en;
      Dirty(EnableScissorTest);
   }

   void SetEnableStencilTest(bool en)
   {
      m_enableStencilTest = en;
      Dirty(EnableStencilTest);
   }

   void SetEnableDepthTest(bool en)
   {
      m_enableDepthTest = en;
      Dirty(EnableDepthTest);
   }

   void SetEnableBlend(bool en)
   {
      m_enableBlend = en;
      Dirty(EnableBlend);
   }

   void SetEnableDither(bool en)
   {
      m_enableDither = en;
      Dirty(EnableDither);
   }

   ///////////////////////
   //! Clear dirty fields
   ///////////////////////

   void ClearBlendColor()
   {
      Clear(BlendColor);
   }

   void ClearBlendEquationSeparate()
   {
      Clear(BlendEquationSeparate);
   }

   void ClearBlendFuncSeparate()
   {
      Clear(BlendFuncSeparate);
   }

   void ClearColorMask()
   {
      Clear(ColorMask);
   }

   void ClearCullFace()
   {
      Clear(CullFace);
   }

   void ClearDepthFunc()
   {
      Clear(DepthFunc);
   }

   void ClearDepthMask()
   {
      Clear(DepthMask);
   }

   void ClearDepthRange()
   {
      Clear(DepthRange);
   }

   void ClearFrontFace()
   {
      Clear(FrontFace);
   }

   void ClearLineWidth()
   {
      Clear(LineWidth);
   }

   void ClearPolygonOffset()
   {
      Clear(PolygonOffset);
   }

   void ClearStencilFuncSeparate()
   {
      Clear(StencilFuncSeparate);
   }

   void ClearStencilMaskSeparate()
   {
      Clear(StencilMaskSeparate);
   }

   void ClearStencilOpSeparate()
   {
      Clear(StencilOpSeparate);
   }

   void ClearEnableCullFace()
   {
      Clear(EnableCullFace);
   }

   void ClearEnablePolygonOffsetFill()
   {
      Clear(EnablePolygonOffsetFill);
   }

   void ClearEnableSampleAlphaToCoverage()
   {
      Clear(EnableSampleAlphaToCoverage);
   }

   void ClearEnableSampleCoverage()
   {
      Clear(EnableSampleCoverage);
   }

   void ClearEnableScissorTest()
   {
      Clear(EnableScissorTest);
   }

   void ClearEnableStencilTest()
   {
      Clear(EnableStencilTest);
   }

   void ClearEnableDepthTest()
   {
      Clear(EnableDepthTest);
   }

   void ClearEnableBlend()
   {
      Clear(EnableBlend);
   }

   void ClearEnableDither()
   {
      Clear(EnableDither);
   }

   // Get states -- TODO them all
   GLenum GetFrontFace() const
   {
      return m_frontFace;
   }

   // Print out dirty states
   void ToLines(std::vector<std::string> &lines) const;

protected:
   static void FillParseMap();
   void ParseLine(const std::string &stateStr);
   void ParseArguments(const std::string &stateIdent, const std::string &argStr);
   void Invoke(uint32_t dirty) const;
   void MergeFrom(const PassState &state);

protected:
   static bool                                 m_parseMapInited;
   static std::map<std::string, PassStateInfo> m_parseMap;

protected:
   enum DirtyBits
   {
      BlendColor                    = 1 << 0,
      BlendEquationSeparate         = 1 << 1,
      BlendFuncSeparate             = 1 << 2,
      ColorMask                     = 1 << 3,
      DepthFunc                     = 1 << 4,
      DepthMask                     = 1 << 5,
      DepthRange                    = 1 << 6,
      FrontFace                     = 1 << 7,
      CullFace                      = 1 << 8,
      LineWidth                     = 1 << 9,
      PolygonOffset                 = 1 << 10,
      StencilFuncSeparate           = 1 << 11,
      StencilMaskSeparate           = 1 << 12,
      StencilOpSeparate             = 1 << 13,

      EnableCullFace                = 1 << 16,
      EnablePolygonOffsetFill       = 1 << 17,
      EnableSampleAlphaToCoverage   = 1 << 18,
      EnableSampleCoverage          = 1 << 19,
      EnableScissorTest             = 1 << 20,
      EnableStencilTest             = 1 << 21,
      EnableDepthTest               = 1 << 22,
      EnableBlend                   = 1 << 23,
      EnableDither                  = 1 << 24
   };

protected:
   // State touched mask
   uint32_t  m_dirty;

   // State
   Vec4     m_blendColor;
   IVec2    m_blendEquationSeparate;
   IVec4    m_blendFuncSeparate;
   IVec4    m_colorMask;
   GLenum   m_depthFunc;
   GLenum   m_depthMask;
   Vec2     m_depthRange;
   GLenum   m_frontFace;
   GLenum   m_cullFace;
   GLfloat  m_lineWidth;
   Vec2     m_polygonOffset;
   IVec3    m_stencilFuncFront;
   IVec3    m_stencilFuncBack;
   GLuint   m_stencilMaskFront;
   GLuint   m_stencilMaskBack;
   IVec3    m_stencilOpFront;
   IVec3    m_stencilOpBack;

   bool     m_enableCullFace;
   bool     m_enablePolygonOffsetFill;
   bool     m_enableSampleAlphaToCoverage;
   bool     m_enableSampleCoverage;
   bool     m_enableScissorTest;
   bool     m_enableStencilTest;
   bool     m_enableDepthTest;
   bool     m_enableBlend;
   bool     m_enableDither;
};

class ShadowState : public PassState
{
public:
   ShadowState() : m_notDefault(0) {}

   // Sets the state according to the dirty bits and state settings in mat and geom
   // Records the changes in this and sets them into GL
   void UpdateGLState(const PassState &mat, const PassState &geom);
   void UpdateGLState(const PassState &state);
   void MergeFrom(const PassState &state, uint32_t dirty);
   void SetToDefault(bool force = false);

protected:
   uint32_t          m_notDefault;
   static PassState  m_defaultState;
};

// @endcond

}

#endif /* __BSG_PASS_STATE_H__ */

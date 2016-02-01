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

#ifndef __BSG_EFFECT_H__
#define __BSG_EFFECT_H__

#include "bsg_common.h"
#include "bsg_pass.h"
#include "bsg_library.h"

#include <stdint.h>
#include <vector>
#include <string>
#include <sstream>

namespace bsg
{


class Effect;

class EffectOptions
{
public:
   EffectOptions() :
      m_sortOrder(eFRONT_TO_BACK),
      m_sortDepthOverride(false),
      m_sortDepth(0.0f)
   {}

   void ToLines(std::vector<std::string> &lines) const;

   //! Sort order for geometry
   enum eSortOrder
   {
      eAUTO = 0,      //!< Sort transparent geometry back-to-front, and opaque geoemtry front-to-back.
      eNO_SORT,       //!< Don't sort geometry. Render in sumbission order.
      eFRONT_TO_BACK, //!< Sort front-to-back
      eBACK_TO_FRONT  //!< Sort back-to-front
   };

   eSortOrder  GetSortOrder()         const  { return m_sortOrder;         }
   bool        GetSortDepthOverride() const  { return m_sortDepthOverride; }
   float       GetSortDepth()         const  { return m_sortDepth;         }

   void SetSortOrder(eSortOrder order)       { m_sortOrder = order;              }
   void SetSortDepthOverride(bool override)  { m_sortDepthOverride = override;   }
   void SetSortDepth(float depth)            { m_sortDepth = depth;              }

private:
   eSortOrder   m_sortOrder;
   bool        m_sortDepthOverride;
   float       m_sortDepth;
};

// @cond
struct EffectTraits
{
   typedef Effect       Base;
   typedef Effect       Derived;
   typedef EffectTraits BaseTraits;
};
// @endcond

//! @addtogroup handles
//! @{
typedef Handle<EffectTraits>   EffectHandle;
//! @}

typedef std::vector<EffectHandle> Effects;

/** @addtogroup scenegraph
@{
*/

//! An Effect represents a combination of these things:
//! - Vertex shader
//! - Fragment shader
//! - Render state
//! - Render passes
//! 
//! All of theses things are required to be defined in order to quantify any kind of rendering algorithm
//! for a material. It makes sense to bring them all together as an 'effect', and to allow the effect to 
//! be represented in human-readable text form for easy editing.
//! 
//! A bsg::Material is in instantiation of an Effect, where the variable parameters of the effect are set
//! by the material. So, for example, we could have a 'plastic' effect, and a 'green shiny plastic' material, or
//! a 'brick' effect, and a 'red brick' material. The same Effect can be re-used by multiple bsg::Material objects.
//! 
//! A very basic effect file for a plain color shader might look like this:
//! \code
//! OPTIONS
//! {
//!    SortOrder = BACK_TO_FRONT;
//! }
//! 
//! PASS 0
//! {
//!    SEMANTICS
//!    {
//!       u_mvpMatrix = MATRIX4_MODEL_VIEW_PROJECTION;
//!       a_position  = VATTR_POSITION;
//!       u_opacity   = SCALAR_USER;
//!       u_color     = VECTOR4_USER;
//!    }
//! 
//!    STATE
//!    {
//!       EnableDepthTest = true;
//!    }
//! 
//!    VERTEX_SHADER
//!    {
//!       uniform mat4   u_mvpMatrix;
//!       attribute vec4 a_position;
//! 
//!       void main()
//!       {
//!          vec4 p = u_mvpMatrix * a_position;
//!          gl_Position = p;
//!       }
//!    }
//! 
//!    FRAGMENT_SHADER
//!    {
//!       precision mediump float; 
//!       uniform float u_opacity;
//!       uniform vec4  u_color;
//! 
//!       void main()
//!       {
//!          gl_FragColor = u_color;
//!          gl_FragColor.a = gl_FragColor.a * u_opacity;
//!       }
//!    }
//! }
//! \endcode
//!
//! An effect file contains:
//! - One OPTIONS block
//! - One or more PASS blocks
//!
//! Any line in the effect file that begins "#include" will include and parse another file (whose name is 
//! parsed next on the "#include" line). This is useful if you have standard GLSL function declarations for example,
//! as you can "#include" this into your vertex or fragment shader code.
//! 
//!
//! <b>OPTIONS</b><br>
//! - SortOrder = 
//!  - FRONT_TO_BACK (renders front-to-back - best option for opaque objects)
//!  - BACK_TO_FRONT (renders back-to-front - best option for transparent objects)
//!  - AUTO          (chooses FRONT_TO_BACK or BACK_TO_FRONT based on the scene node's opacity)
//!  - NONE          (renders in the order of submission)
//! - SortDepth =
//!  - AUTO                 (the sort depth is calculated from the actual geometry)
//!  - floating point value (the sort depth is overridden to the given value)
//!
//! <b>PASS</b>
//! - SEMANTICS
//! - STATE
//! - SAMPLER_2D (optional)
//! - SAMPLER_CUBE (optional)
//! - VERTEX_SHADER
//! - FRAGMENT_SHADER
//!
//! <b>SEMANTICS</b><br>
//! Contains mappings from uniform/attribute name to the specific meaning of that name.
//! Each semantic mapping looks like:
//! uniform_or_attrib_name = SEMANTIC_TAG;
//! During rendering, the actual semantic values are only calculated if listed in the semantic block.
//!
//! The possible <b>SEMANTIC</b> tags are:
//! - MATRIX4_MODEL (the model matrix)
//! - MATRIX4_VIEW (the view matrix)
//! - MATRIX4_PROJECTION (the projection matrix)
//! - MATRIX4_MODEL_VIEW (the modelview matrix)
//! - MATRIX4_MODEL_VIEW_PROJECTION (the modelview-projection matrix)
//! - MATRIX3_INVT_MODEL (inverse-transpose of the model matrix)
//! - MATRIX3_INVT_VIEW (inverse-transpose of the view matrix)
//! - MATRIX3_INVT_MODEL_VIEW (inverse-transpose of the modelview matrix)
//! - SCALAR_OPACITY (the opacity from the scene nodes)
//! - SCALAR_USER (an arbitrary user supplied float uniform)
//! - VECTOR2_USER (an arbitrary user supplied 2D vector uniform)
//! - VECTOR3_USER (an arbitrary user supplied 3D vector uniform)
//! - VECTOR4_USER (an arbitrary user supplied 4D vector uniform)
//! - MATRIX2_USER (an arbitrary user supplied 2x2 matrix uniform)
//! - MATRIX3_USER (an arbitrary user supplied 3x3 matrix uniform)
//! - MATRIX4_USER (an arbitrary user supplied 4x4 matrix uniform)
//! - VECTOR4_SCREEN_SIZE (dimensions of screen including reciprocals in z and w)
//! - VECTOR4_QUAD_OFFSET (takes a point in -1 to 1 space and adjusts for quad mode. xy * quadOffset.xy + quadOffset.zw)
//! - VATTR_POSITION (the position vertex attribute)
//! - VATTR_NORMAL (the normal vertex attribute)
//! - VATTR_TANGENT (the tangent vertex attribute)
//! - VATTR_BINORMAL (the binormal vertex attribute)
//! - VATTR_TEXCOORD1 (first texture coordinate vertex attribute)
//! - VATTR_TEXCOORD2 (subsequent texture coordinate vertex attribute)
//! - VATTR_TEXCOORD3 (subsequent texture coordinate vertex attribute)
//! - VATTR_COLOR (the color vertex attribute)
//! - VATTR_USER1 (arbitrary user supplied vertex attribute)
//! - VATTR_USER2 (arbitrary user supplied vertex attribute)
//! - VATTR_USER3 (arbitrary user supplied vertex attribute)
//! - VATTR_USER4 (arbitrary user supplied vertex attribute)
//! - VATTR_USER5 (arbitrary user supplied vertex attribute)
//! - VATTR_USER6 (arbitrary user supplied vertex attribute)
//!
//! <b>STATE</b><br>
//! Render state can be set for each PASS.
//! Each state assignment looks like:
//! STATE_TAG = value;
//! The possible <b>STATE</b> tags are:
//! - BlendColor = float, float, float, float
//! - BlendEquation = [FUNC_ADD|FUNC_SUBTRACT|FUNC_REVERSE_SUBTRACT]
//! - BlendEquationSeparate = [FUNC_ADD|FUNC_SUBTRACT|FUNC_REVERSE_SUBTRACT],[FUNC_ADD|FUNC_SUBTRACT|FUNC_REVERSE_SUBTRACT]
//! - BlendFunc = <BlendCoefficient (src), BlendCoefficient (dst)>
//! - BlendFuncSeparate = <BlendCoefficient (srcRGB), BlendCoefficient (dstRGB), BlendCoefficient (srcA), BlendCoefficient (dstA)>
//!  - BlendCoefficent = <br>
//! [<br>
//! ZERO |<br>
//! ONE |<br>
//! SRC_COLOR |<br>
//! ONE_MINUS_SRC_COLOR |<br>
//! SRC_ALPHA |<br>
//! ONE_MINUS_SRC_ALPHA |<br>
//! DST_COLOR |<br>
//! ONE_MINUS_DST_COLOR |<br>
//! DST_ALPHA |<br>
//! ONE_MINUS_DST_ALPHA |<br>
//! CONSTANT_COLOR |<br>
//! ONE_MINUS_CONSTANT_COLOR |<br>
//! CONSTANT_ALPHA |<br>
//! ONE_MINUS_CONSTANT_ALPHA |<br>
//! SRC_ALPHA_SATURATE |<br>
//! ]
//! - ColorMask = [TRUE|FALSE], [TRUE|FALSE], [TRUE|FALSE], [TRUE|FALSE]
//! - DepthFunc = [LESS|GREATER|LEQUAL|GEQUAL|EQUAL|NOTEQUAL|ALWAYS|NEVER]
//! - DepthMask = [TRUE|FALSE]
//! - StencilFunc = [LESS|GREATER|LEQUAL|GEQUAL|EQUAL|NOTEQUAL|ALWAYS|NEVER], int, uint
//! - StencilFuncSeparate = [FRONT|BACK|FRONT_AND_BACK], [LESS|GREATER|LEQUAL|GEQUAL|EQUAL|NOTEQUAL|ALWAYS|NEVER], int, uint
//! - StencilMask = uint
//! - StencilMaskSeparate = [FRONT|BACK|FRONT_AND_BACK], uint
//! - StencilOp = [KEEP|ZERO|REPLACE|INCR|DECR|INCR_WRAP|DECR_WRAP|INVERT], <br>
//! [KEEP|ZERO|REPLACE|INCR|DECR|INCR_WRAP|DECR_WRAP|INVERT], <br>
//! [KEEP|ZERO|REPLACE|INCR|DECR|INCR_WRAP|DECR_WRAP|INVERT]
//! - StencilOpSeparate = [FRONT|BACK|FRONT_AND_BACK], <br>
//! [KEEP|ZERO|REPLACE|INCR|DECR|INCR_WRAP|DECR_WRAP|INVERT], <br>
//! [KEEP|ZERO|REPLACE|INCR|DECR|INCR_WRAP|DECR_WRAP|INVERT], <br>
//! [KEEP|ZERO|REPLACE|INCR|DECR|INCR_WRAP|DECR_WRAP|INVERT]
//! - EnableStencilTest = [TRUE|FALSE]
//! - EnableDepthTest = [TRUE|FALSE]
//! - EnableBlend = [TRUE|FALSE]
//!
//! <b>SAMPLERS</b><br>
//! The samplers describe the binding of textures to texture units, and the filter and wrap modes to be used
//! with the texture e.g.
//! \code
//! SAMPLER_2D u_tex
//! {
//!   Unit = 0;
//!   Wrap = CLAMP, CLAMP;
//!   Filter = LINEAR_MIPMAP_LINEAR, LINEAR;
//! }
//! \endcode
//! This specifies that texture unit zero will be used for u_tex (the name of the sampler uniform to be used
//! in the fragment of vertex shader) and that it is a 2D texture.  For a cube-map texture use SAMPLER_CUBE instead.
//! In the fragment shader you will need to declare the corresponding uniform e.g.
//! \code 
//! uniform sampler2D u_tex;
//! \endcode
//!
//! The wrap modes are for u and v respectively and can be CLAMP, REPEAT or MIRROR.
//!
//! The filters are for minimisation and magnification respectively.  Minimisaton can be LINEAR_MIPMAP_LINEAR, LINEAR_MIPMAP_NEAREST,
//! NEAREST_MIPMAP_NEAREST, NEAREST_MIPMAP_LINEAR, LINEAR or NEAREST.  Magnification filter must be one of LINEAR or NEAREST.
//!
//! Use a cube-map texture:
//! \code
//! SAMPLER_CUBE u_tex
//!
//! \endcode
//! <b>VERTEX_SHADER</b><br>
//! The vertex shader block contains standard OpenGL ES2 vertex shader code
//!
//! <b>FRAGMENT_SHADER</b><br>
//! The vertex shader block contains standard OpenGL ES2 fragment shader code

class Effect : public RefCount
{
   friend class Handle<EffectTraits>;

public:
   typedef enum EffectOptions::eSortOrder eSortOrder;

public:
   virtual ~Effect();

   //! Load an effect from a std::istream, typically from a disk-based file.
   //! You can specify an optional list of define values that will be included in the shader.
   //! The istream is useful if you have in memory data to load (an uncompressed file for example).
   void Load(std::istream &is, const std::vector<std::string> &defines = m_defaultDefines);

   //! Load an effect from a disk-based file.
   //! You can specify an optional list of define values that will be included in the shader.
   //! This is purely a shortcut for the istream based loader.
   void Load(const std::string &pathName, const std::vector<std::string> &defines = m_defaultDefines);

   //! Load an effect from a memory-based string.
   //! You can specify an optional list of define values that will be included in the shader.
   void Read(const char *effect, const std::vector<std::string> &defines = m_defaultDefines);

   //! Return the number of render passes in the loaded effect file
   uint32_t    NumPasses()          const { return m_passes.size(); }
   //! Return the bsg::Pass object for a particular render pass
   Pass        *GetPass(uint32_t p) const;
   //! Return the sort order for this effect
   eSortOrder   GetSortOrder() const { return m_options.GetSortOrder(); }

   //! Returns true if the depth for the objects rendered by this effect has been overridden in the effect 
   //! file to a particular value, rather than using the actual calculated depth.
   bool OverrideDepth() const       { return m_options.GetSortDepthOverride(); }

   //! Returns the overridden sort depth. Only valid if OverrideDepth() is true.
   float GetDepth()     const       { return m_options.GetSortDepth();         }

   //! Returns true if the effect has been loaded
   bool IsLoaded() const            { return m_loaded;            }

protected:
   Effect();

private:
   void ParseLine(const std::string &in);
   void ParseOptions(const std::string &in);
   void ProcessInclude(const std::string &line);
   void Reset();

private:
   std::vector<Pass*>   m_passes;

   // Parsing state
   enum eParseState
   {
      ePASS_OR_OPTIONS,
      eIN_OPTIONS,
      ePASS_OPEN_BRACE,
      eIN_PASS,
      eCAPTURE_PASS,
      eCAPTURE_OPTIONS
   };

   bool        m_loaded;
   uint32_t    m_lineNumber;
   eParseState m_state;

   uint32_t    m_passNumber;
   uint32_t    m_captureBraces;
   std::string m_captureBlock;
   std::string m_captureText;
   std::string m_vertShader;
   std::string m_fragShader;
   std::string m_samplerName;

   EffectOptions                     m_options;
   std::vector<std::string>          m_defines;

   static std::vector<std::string>   m_includePaths;
   static std::vector<std::string>   m_defaultDefines;
};

/** @} */
}

#endif /* __BSG_EFFECT_H__ */


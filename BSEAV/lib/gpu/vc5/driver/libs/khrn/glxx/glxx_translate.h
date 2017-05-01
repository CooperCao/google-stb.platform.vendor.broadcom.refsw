/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef GLXX_TRANSLATE_H
#define GLXX_TRANSLATE_H

#include "../common/khrn_int_common.h"
#include "gl_public_api.h"
#include "glxx_server.h"

/*** Texture env ***/

static uint32_t tex_modes[] =
{
   /* Replace */
   GL11_TEX_STRING(REP,P,C,x,x,x,x ,REP,S,A,x,x,x,x ),
   GL11_TEX_STRING(REP,S,C,x,x,x,x ,REP,P,A,x,x,x,x ),
   GL11_TEX_STRING(REP,S,C,x,x,x,x ,REP,S,A,x,x,x,x ),
   /* Modulate */
   GL11_TEX_STRING(REP,P,C,x,x,x,x ,MOD,P,A,S,A,x,x ),
   GL11_TEX_STRING(MOD,P,C,S,C,x,x ,REP,P,A,x,x,x,x ),
   GL11_TEX_STRING(MOD,P,C,S,C,x,x ,MOD,P,A,S,A,x,x ),
   /* Decal */
   GL11_TEX_STRING(REP,P,C,x,x,x,x ,REP,P,A,x,x,x,x ),  /* undefined */
   GL11_TEX_STRING(REP,S,C,x,x,x,x ,REP,P,A,x,x,x,x ),
   GL11_TEX_STRING(INT,S,C,P,C,S,A ,REP,P,A,x,x,x,x ),
   /* Blend */
   GL11_TEX_STRING(REP,P,C,x,x,x,x ,MOD,P,A,S,A,x,x ),
   GL11_TEX_STRING(INT,K,C,P,C,S,C ,REP,P,A,x,x,x,x ),
   GL11_TEX_STRING(INT,K,C,P,C,S,C ,MOD,P,A,S,A,x,x ),
   /* Add */
   GL11_TEX_STRING(REP,P,C,x,x,x,x ,MOD,P,A,S,A,x,x ),
   GL11_TEX_STRING(ADD,P,C,S,C,x,x ,REP,P,A,x,x,x,x ),
   GL11_TEX_STRING(ADD,P,C,S,C,x,x ,MOD,P,A,S,A,x,x ),
};

static inline uint32_t lookup_tex_mode(GLenum mode, bool has_rgb, bool has_alpha)
{
   int i;
   assert(mode != GL_COMBINE);
   if (!has_rgb)
      i = 0;
   else if (!has_alpha)
      i = 1;
   else
      i = 2;
   switch (mode)
   {
      case GL_REPLACE: return tex_modes[i];
      case GL_MODULATE: return tex_modes[i+3];
      case GL_DECAL: return tex_modes[i+6];
      case GL_BLEND: return tex_modes[i+9];
      case GL_ADD: return tex_modes[i+12];
      case GL_COMBINE:
      default:
         unreachable();
         return 0;
   }
}

static inline uint32_t translate_combine_rgb(GLenum combine)
{
   switch (combine)
   {
      case GL_REPLACE: return GL11_COMB_REP;
      case GL_MODULATE: return GL11_COMB_MOD;
      case GL_ADD: return GL11_COMB_ADD;
      case GL_ADD_SIGNED: return GL11_COMB_ADS;
      case GL_INTERPOLATE: return GL11_COMB_INT;
      case GL_SUBTRACT: return GL11_COMB_SUB;
      case GL_DOT3_RGB: return GL11_COMB_DOT;
      case GL_DOT3_RGBA: return GL11_COMB_DOTA;
      default: return ~0u;
   }
}

static inline uint32_t translate_combine_alpha(GLenum combine)
{
   switch (combine)
   {
      case GL_REPLACE: return GL11_COMB_REP;
      case GL_MODULATE: return GL11_COMB_MOD;
      case GL_ADD: return GL11_COMB_ADD;
      case GL_ADD_SIGNED: return GL11_COMB_ADS;
      case GL_INTERPOLATE: return GL11_COMB_INT;
      case GL_SUBTRACT: return GL11_COMB_SUB;
      default: return ~0u;
   }
}

static inline GLenum untranslate_combine(uint32_t combine)
{
   switch (combine)
   {
      case GL11_COMB_REP: return GL_REPLACE;
      case GL11_COMB_MOD: return GL_MODULATE;
      case GL11_COMB_ADD: return GL_ADD;
      case GL11_COMB_ADS: return GL_ADD_SIGNED;
      case GL11_COMB_INT: return GL_INTERPOLATE;
      case GL11_COMB_SUB: return GL_SUBTRACT;
      case GL11_COMB_DOT: return GL_DOT3_RGB;
      case GL11_COMB_DOTA: return GL_DOT3_RGBA;
      default: unreachable(); return 0u;
   }
}

static inline uint32_t translate_tex_source(GLenum source)
{
   switch (source)
   {
      case GL_TEXTURE: return GL11_SRC_S;
      case GL_CONSTANT: return GL11_SRC_K;
      case GL_PRIMARY_COLOR: return GL11_SRC_F;
      case GL_PREVIOUS: return GL11_SRC_P;
      default: return ~0u;
   }
}

static inline GLenum untranslate_tex_source(uint32_t source)
{
   switch (source)
   {
      case GL11_SRC_S: return GL_TEXTURE;
      case GL11_SRC_K: return GL_CONSTANT;
      case GL11_SRC_F: return GL_PRIMARY_COLOR;
      case GL11_SRC_P: return GL_PREVIOUS;
      default: unreachable(); return 0;
   }
}

static inline uint32_t translate_tex_operand_rgb(GLenum operand)
{
   switch (operand)
   {
      case GL_SRC_COLOR: return GL11_OP_C;
      case GL_ONE_MINUS_SRC_COLOR: return GL11_OP_CX;
      case GL_SRC_ALPHA: return GL11_OP_A;
      case GL_ONE_MINUS_SRC_ALPHA: return GL11_OP_AX;
      default: return ~0u;
   }
}

static inline uint32_t translate_tex_operand_alpha(GLenum operand)
{
   switch (operand)
   {
      case GL_SRC_ALPHA: return GL11_OP_A;
      case GL_ONE_MINUS_SRC_ALPHA: return GL11_OP_AX;
      default: return ~0u;
   }
}

static inline GLenum untranslate_tex_operand(uint32_t operand)
{
   switch (operand)
   {
      case GL11_OP_C: return GL_SRC_COLOR;
      case GL11_OP_CX: return GL_ONE_MINUS_SRC_COLOR;
      case GL11_OP_A: return GL_SRC_ALPHA;
      case GL11_OP_AX: return GL_ONE_MINUS_SRC_ALPHA;
      default: unreachable(); return 0;
   }
}

/*** Alpha func ***/

static inline uint32_t translate_alpha_func(GLenum func)
{
   switch (func)
   {
      case GL_NEVER: return GL11_AFUNC_NEVER;
      case GL_ALWAYS: return GL11_AFUNC_ALWAYS;
      case GL_LESS: return GL11_AFUNC_LESS;
      case GL_LEQUAL: return GL11_AFUNC_LEQUAL;
      case GL_EQUAL: return GL11_AFUNC_EQUAL;
      case GL_GREATER: return GL11_AFUNC_GREATER;
      case GL_GEQUAL: return GL11_AFUNC_GEQUAL;
      case GL_NOTEQUAL: return GL11_AFUNC_NOTEQUAL;
      default: return ~0u;
   }
}

static inline GLenum untranslate_alpha_func(uint32_t func)
{
   switch (func)
   {
      case GL11_AFUNC_NEVER:     return GL_NEVER;
      case GL11_AFUNC_ALWAYS:    return GL_ALWAYS;
      case GL11_AFUNC_LESS:      return GL_LESS;
      case GL11_AFUNC_LEQUAL:    return GL_LEQUAL;
      case GL11_AFUNC_EQUAL:     return GL_EQUAL;
      case GL11_AFUNC_GREATER:   return GL_GREATER;
      case GL11_AFUNC_GEQUAL:    return GL_GEQUAL;
      case GL11_AFUNC_NOTEQUAL:  return GL_NOTEQUAL;
      default: return ~0u;
   }
}

/*** Fog ***/

static inline uint32_t translate_fog_mode(GLenum mode)
{
   switch (mode)
   {
      case GL_EXP: return GL11_FOG_EXP;
      case GL_EXP2: return GL11_FOG_EXP2;
      case GL_LINEAR: return GL11_FOG_LINEAR;
      default: return ~0u;
   }
}

static inline GLenum untranslate_fog_mode(uint32_t mode)
{
   switch (mode)
   {
      case GL11_FOG_EXP: return GL_EXP;
      case GL11_FOG_EXP2: return GL_EXP2;
      case GL11_FOG_LINEAR: return GL_LINEAR;
      default:
         unreachable(); return 0;
   }
}

/*** Logic op ***/

static inline uint32_t translate_logic_op(GLenum op)
{
   switch (op)
   {
      case GL_CLEAR:          return GL11_LOGIC_CLEAR ;
      case GL_AND:            return GL11_LOGIC_AND;
      case GL_AND_REVERSE:    return GL11_LOGIC_AND_REVERSE;
      case GL_COPY:           return GL11_LOGIC_COPY;
      case GL_AND_INVERTED:   return GL11_LOGIC_AND_INVERTED;
      case GL_NOOP:           return GL11_LOGIC_NOOP;
      case GL_XOR:            return GL11_LOGIC_XOR;
      case GL_OR:             return GL11_LOGIC_OR;
      case GL_NOR:            return GL11_LOGIC_NOR;
      case GL_EQUIV:          return GL11_LOGIC_EQUIV;
      case GL_INVERT:         return GL11_LOGIC_INVERT;
      case GL_OR_REVERSE:     return GL11_LOGIC_OR_REVERSE;
      case GL_COPY_INVERTED:  return GL11_LOGIC_COPY_INVERTED;
      case GL_OR_INVERTED:    return GL11_LOGIC_OR_INVERTED;
      case GL_NAND:           return GL11_LOGIC_NAND;
      case GL_SET:            return GL11_LOGIC_SET;
      default:                return ~0u;
   }
}

static inline GLenum untranslate_logic_op(uint64_t op)
{
   switch (op)
   {
      case GL11_LOGIC_CLEAR:           return GL_CLEAR ;
      case GL11_LOGIC_AND:             return GL_AND;
      case GL11_LOGIC_AND_REVERSE:     return GL_AND_REVERSE;
      case GL11_LOGIC_COPY:            return GL_COPY;
      case GL11_LOGIC_AND_INVERTED:    return GL_AND_INVERTED;
      case GL11_LOGIC_NOOP:            return GL_NOOP;
      case GL11_LOGIC_XOR:             return GL_XOR;
      case GL11_LOGIC_OR:              return GL_OR;
      case GL11_LOGIC_NOR:             return GL_NOR;
      case GL11_LOGIC_EQUIV:           return GL_EQUIV;
      case GL11_LOGIC_INVERT:          return GL_INVERT;
      case GL11_LOGIC_OR_REVERSE:      return GL_OR_REVERSE;
      case GL11_LOGIC_COPY_INVERTED:   return GL_COPY_INVERTED;
      case GL11_LOGIC_OR_INVERTED:     return GL_OR_INVERTED;
      case GL11_LOGIC_NAND:            return GL_NAND;
      case GL11_LOGIC_SET:             return GL_SET;
      default:
         unreachable(); return 0;
   }
}

/*** Blend ***/
static inline glxx_blend_eqn_t translate_blend_equation(GLenum mode)
{
   switch (mode)
   {
   case GL_FUNC_ADD:                return GLXX_BLEND_EQN_ADD;
   case GL_FUNC_SUBTRACT:           return GLXX_BLEND_EQN_SUB;
   case GL_FUNC_REVERSE_SUBTRACT:   return GLXX_BLEND_EQN_RSUB;
   case GL_MIN:                     return GLXX_BLEND_EQN_MIN;
   case GL_MAX:                     return GLXX_BLEND_EQN_MAX;

   case GL_MULTIPLY:                return GLXX_ADV_BLEND_EQN_MULTIPLY;
   case GL_SCREEN:                  return GLXX_ADV_BLEND_EQN_SCREEN;
   case GL_OVERLAY:                 return GLXX_ADV_BLEND_EQN_OVERLAY;
   case GL_DARKEN:                  return GLXX_ADV_BLEND_EQN_DARKEN;
   case GL_LIGHTEN:                 return GLXX_ADV_BLEND_EQN_LIGHTEN;
   case GL_COLORDODGE:              return GLXX_ADV_BLEND_EQN_COLORDODGE;
   case GL_COLORBURN:               return GLXX_ADV_BLEND_EQN_COLORBURN;
   case GL_HARDLIGHT:               return GLXX_ADV_BLEND_EQN_HARDLIGHT;
   case GL_SOFTLIGHT:               return GLXX_ADV_BLEND_EQN_SOFTLIGHT;
   case GL_DIFFERENCE:              return GLXX_ADV_BLEND_EQN_DIFFERENCE;
   case GL_EXCLUSION:               return GLXX_ADV_BLEND_EQN_EXCLUSION;
   case GL_HSL_HUE:                 return GLXX_ADV_BLEND_EQN_HSL_HUE;
   case GL_HSL_SATURATION:          return GLXX_ADV_BLEND_EQN_HSL_SATURATION;
   case GL_HSL_COLOR:               return GLXX_ADV_BLEND_EQN_HSL_COLOR;
   case GL_HSL_LUMINOSITY:          return GLXX_ADV_BLEND_EQN_HSL_LUMINOSITY;

   default:                         return GLXX_BLEND_EQN_INVALID;
   }
}

static inline GLenum untranslate_blend_equation(glxx_blend_eqn_t mode)
{
   switch (mode)
   {
   case GLXX_BLEND_EQN_ADD:                  return GL_FUNC_ADD;
   case GLXX_BLEND_EQN_SUB:                  return GL_FUNC_SUBTRACT;
   case GLXX_BLEND_EQN_RSUB:                 return GL_FUNC_REVERSE_SUBTRACT;
   case GLXX_BLEND_EQN_MIN:                  return GL_MIN;
   case GLXX_BLEND_EQN_MAX:                  return GL_MAX;

   case GLXX_ADV_BLEND_EQN_MULTIPLY:         return GL_MULTIPLY;
   case GLXX_ADV_BLEND_EQN_SCREEN:           return GL_SCREEN;
   case GLXX_ADV_BLEND_EQN_OVERLAY:          return GL_OVERLAY;
   case GLXX_ADV_BLEND_EQN_DARKEN:           return GL_DARKEN;
   case GLXX_ADV_BLEND_EQN_LIGHTEN:          return GL_LIGHTEN;
   case GLXX_ADV_BLEND_EQN_COLORDODGE:       return GL_COLORDODGE;
   case GLXX_ADV_BLEND_EQN_COLORBURN:        return GL_COLORBURN;
   case GLXX_ADV_BLEND_EQN_HARDLIGHT:        return GL_HARDLIGHT;
   case GLXX_ADV_BLEND_EQN_SOFTLIGHT:        return GL_SOFTLIGHT;
   case GLXX_ADV_BLEND_EQN_DIFFERENCE:       return GL_DIFFERENCE;
   case GLXX_ADV_BLEND_EQN_EXCLUSION:        return GL_EXCLUSION;
   case GLXX_ADV_BLEND_EQN_HSL_HUE:          return GL_HSL_HUE;
   case GLXX_ADV_BLEND_EQN_HSL_SATURATION:   return GL_HSL_SATURATION;
   case GLXX_ADV_BLEND_EQN_HSL_COLOR:        return GL_HSL_COLOR;
   case GLXX_ADV_BLEND_EQN_HSL_LUMINOSITY:   return GL_HSL_LUMINOSITY;
   default:
      unreachable();                         return 0;
   }
}

static inline v3d_blend_mul_t translate_blend_func(GLenum func)
{
   switch (func)
   {
   case GL_ZERO:                     return V3D_BLEND_MUL_ZERO;
   case GL_ONE:                      return V3D_BLEND_MUL_ONE;
   case GL_SRC_ALPHA:                return V3D_BLEND_MUL_SRC_ALPHA;
   case GL_ONE_MINUS_SRC_ALPHA:      return V3D_BLEND_MUL_OM_SRC_ALPHA;
   case GL_DST_ALPHA:                return V3D_BLEND_MUL_DST_ALPHA;
   case GL_ONE_MINUS_DST_ALPHA:      return V3D_BLEND_MUL_OM_DST_ALPHA;
   case GL_SRC_ALPHA_SATURATE:       return V3D_BLEND_MUL_SRC_ALPHA_SAT;
   case GL_SRC_COLOR:                return V3D_BLEND_MUL_SRC;
   case GL_ONE_MINUS_SRC_COLOR:      return V3D_BLEND_MUL_OM_SRC;
   case GL_DST_COLOR:                return V3D_BLEND_MUL_DST;
   case GL_ONE_MINUS_DST_COLOR:      return V3D_BLEND_MUL_OM_DST;
   case GL_CONSTANT_COLOR:           return V3D_BLEND_MUL_CONST;
   case GL_ONE_MINUS_CONSTANT_COLOR: return V3D_BLEND_MUL_OM_CONST;
   case GL_CONSTANT_ALPHA:           return V3D_BLEND_MUL_CONST_ALPHA;
   case GL_ONE_MINUS_CONSTANT_ALPHA: return V3D_BLEND_MUL_OM_CONST_ALPHA;
   default: return V3D_BLEND_MUL_INVALID;
   }
}

static inline GLenum untranslate_blend_func(v3d_blend_mul_t func)
{
   switch (func)
   {
   case V3D_BLEND_MUL_ZERO:            return GL_ZERO;
   case V3D_BLEND_MUL_ONE:             return GL_ONE;
   case V3D_BLEND_MUL_SRC_ALPHA:       return GL_SRC_ALPHA;
   case V3D_BLEND_MUL_OM_SRC_ALPHA:    return GL_ONE_MINUS_SRC_ALPHA;
   case V3D_BLEND_MUL_DST_ALPHA:       return GL_DST_ALPHA;
   case V3D_BLEND_MUL_OM_DST_ALPHA:    return GL_ONE_MINUS_DST_ALPHA;
   case V3D_BLEND_MUL_SRC_ALPHA_SAT:   return GL_SRC_ALPHA_SATURATE;
   case V3D_BLEND_MUL_SRC:             return GL_SRC_COLOR;
   case V3D_BLEND_MUL_OM_SRC:          return GL_ONE_MINUS_SRC_COLOR;
   case V3D_BLEND_MUL_DST:             return GL_DST_COLOR;
   case V3D_BLEND_MUL_OM_DST:          return GL_ONE_MINUS_DST_COLOR;
   case V3D_BLEND_MUL_CONST:           return GL_CONSTANT_COLOR;
   case V3D_BLEND_MUL_OM_CONST:        return GL_ONE_MINUS_CONSTANT_COLOR;
   case V3D_BLEND_MUL_CONST_ALPHA:     return GL_CONSTANT_ALPHA;
   case V3D_BLEND_MUL_OM_CONST_ALPHA:  return GL_ONE_MINUS_CONSTANT_ALPHA;
   default:
      unreachable(); return 0;
   }
}

static inline v3d_compare_func_t translate_stencil_func(GLenum func)
{
   switch (func)
   {
   case GL_NEVER:       return V3D_COMPARE_FUNC_NEVER;
   case GL_LESS:        return V3D_COMPARE_FUNC_LESS;
   case GL_EQUAL:       return V3D_COMPARE_FUNC_EQUAL;
   case GL_LEQUAL:      return V3D_COMPARE_FUNC_LEQUAL;
   case GL_GREATER:     return V3D_COMPARE_FUNC_GREATER;
   case GL_NOTEQUAL:    return V3D_COMPARE_FUNC_NOTEQUAL;
   case GL_GEQUAL:      return V3D_COMPARE_FUNC_GEQUAL;
   case GL_ALWAYS:      return V3D_COMPARE_FUNC_ALWAYS;
   default:
      unreachable();    return V3D_COMPARE_FUNC_INVALID;
   }
}

static inline v3d_stencil_op_t translate_stencil_op(GLenum op)
{
   switch (op)
   {
   case GL_ZERO:        return V3D_STENCIL_OP_ZERO;
   case GL_KEEP:        return V3D_STENCIL_OP_KEEP;
   case GL_REPLACE:     return V3D_STENCIL_OP_REPLACE;
   case GL_INCR:        return V3D_STENCIL_OP_INCR;
   case GL_DECR:        return V3D_STENCIL_OP_DECR;
   case GL_INVERT:      return V3D_STENCIL_OP_INVERT;
   case GL_INCR_WRAP:   return V3D_STENCIL_OP_INCWRAP;
   case GL_DECR_WRAP:   return V3D_STENCIL_OP_DECWRAP;
   default:
      unreachable();    return V3D_STENCIL_OP_INVALID;
   }
}


#endif

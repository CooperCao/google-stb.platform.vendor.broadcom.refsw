/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_COLOR_H__
#define __BSG_COLOR_H__

#include "bsg_common.h"

#include <memory.h>
#include <stdint.h>

namespace bsg
{

// @cond
class Color
{
public:
   Color() { memset(m_color, 0, 4); }
   Color(uint8_t color[4]) { memcpy(m_color, color, 4); }
   Color(uint8_t r, uint8_t g, uint8_t b) { m_color[0] = r; m_color[1] = g; m_color[2] = b; m_color[3] = 255; }
   Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) { m_color[0] = r; m_color[1] = g; m_color[2] = b; m_color[3] = a; }
   Color(float r, float g, float b) { m_color[0] = (uint8_t)(r * 255.0f); m_color[1] = (uint8_t)(g * 255.0); m_color[2] = (uint8_t)(b * 255.0f); m_color[3] = 255; }
   Color(float r, float g, float b, float a) { m_color[0] = (uint8_t)(r * 255.0f); m_color[1] = (uint8_t)(g * 255.0); m_color[2] = (uint8_t)(b * 255.0f); m_color[3] = (uint8_t)(a * 255.0f); }

   uint8_t R() const { return m_color[0]; }
   uint8_t G() const { return m_color[1]; }
   uint8_t B() const { return m_color[2]; }
   uint8_t A() const { return m_color[3]; }

   float FR() const { return (float)m_color[0] / 255.0f; }
   float FG() const { return (float)m_color[1] / 255.0f; }
   float FB() const { return (float)m_color[2] / 255.0f; }
   float FA() const { return (float)m_color[3] / 255.0f; }

   const uint8_t *Vec() const { return m_color; }
   uint8_t *Vec() { return m_color; }

   const float *VecF() const
   {
      m_colorF[0] = FR();
      m_colorF[1] = FG();
      m_colorF[2] = FB();
      m_colorF[3] = FA();
      return m_colorF;
   }
   float *VecF()
   {
      m_colorF[0] = FR();
      m_colorF[1] = FG();
      m_colorF[2] = FB();
      m_colorF[3] = FA();
      return m_colorF;
   }

private:
   uint8_t        m_color[4];
   mutable float  m_colorF[4];
};
// @endcond
}

#endif /* __BSG_COLOR_H__ */

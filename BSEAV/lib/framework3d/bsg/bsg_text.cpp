/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef BSG_STAND_ALONE

#include "bsg_text.h"
#include "bsg_application.h"

namespace bsg
{

TextParams::TextParams() :
   m_position(Vec2(0, 0)),
   m_color(Vec3(1, 1, 1)),
   m_opacity(1.0f)
{
}

TextParams::TextParams(const Vec2 &position, const Vec3 &color, float opacity) :
   m_position(position),
   m_color(color),
   m_opacity(opacity)
{
}

/////////////////////////

Text::Text() :
   m_font(New)
{
}

Text::Text(const std::string &str, FontHandle font) :
   m_str(str),
   m_font(font)
{
}

void Text::Render()
{
   const Application &app = *Application::Instance();

   app.DrawTextString(m_str, m_params.m_position[0], m_params.m_position[1], m_font,
                      Vec4(m_params.m_color[0], m_params.m_color[1], m_params.m_color[2], m_params.m_opacity));
}

}

#endif /* BSG_STAND_ALONE */

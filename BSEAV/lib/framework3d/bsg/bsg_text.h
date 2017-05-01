/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef BSG_STAND_ALONE

#ifndef __BSG_TEXT_H__
#define __BSG_TEXT_H__

#include "bsg_common.h"
#include "bsg_animatable.h"
#include "bsg_vector.h"
#include "bsg_font.h"

#include <string>

namespace bsg
{

//! @ingroup animation
//! @{

//! The TextParams class is a helper to aid in the animation of direct 2D text.
//! It encapsulates a position, color and opacity, and is supported by the animation
//! evaluators, so can be interpolated.
class TextParams
{
public:
   TextParams();
   TextParams(const Vec2 &position, const Vec3 &color = Vec3(1, 1, 1), float opacity = 1.0f);

   //! Return an (animatable) position
   const AnimatableVec2 &GetPosition() const { return m_position; }
   //! Return an (animatable) position
   AnimatableVec2 &GetPosition() { return m_position; }
   //! Set the position
   void SetPosition(const Vec2 &val) { m_position.Set(val); }

   //! Return an (animatable) color (as a Vec3 - RGB)
   const AnimatableVec3 &GetColor() const { return m_color; }
   //! Return an (animatable) color (as a Vec3 - RGB)
   AnimatableVec3 &GetColor() { return m_color; }
   //! Set the RGB color
   void SetColor(const Vec3 &val) { m_color.Set(val); }

   //! Return an (animatable) opacity
   const AnimatableFloat &GetOpacity() const { return m_opacity; }
   //! Return an (animatable) opacity
   AnimatableFloat &GetOpacity() { return m_opacity; }
   //! Set the opacity
   void SetOpacity(float val) { m_opacity.Set(val); }

   friend class Text;

private:
   AnimatableVec2    m_position;
   AnimatableVec3    m_color;
   AnimatableFloat   m_opacity;
};

typedef Animatable<TextParams> AnimatableTextParams;

//! The Text class wraps a string and AnimatableTextParams.  Its is
//! used by the direct 2D text rendering mechanism.
//! It can render itself through its Render() method.
class Text
{
public:
   Text();
   Text(const std::string &str, FontHandle font);

   //! Render the text string now
   void Render();

   //! Return the (animatable) parameters
   AnimatableTextParams &Params() { return m_params; }

   //! Return the string
   const std::string &GetString() const { return m_str; }
   //! Set the text string
   void SetString(const std::string &val) { m_str = val; }

   //! Return a handle to the Font
   FontHandle GetFont() const { return m_font; }

   //! Set the Font
   void SetFont(const FontHandle &val) { m_font = val; }

private:
   std::string          m_str;
   FontHandle           m_font;
   AnimatableTextParams m_params;
};

//! @}
}


#endif /* __BSG_TEXT_H__ */

#endif /* BSG_STAND_ALONE */

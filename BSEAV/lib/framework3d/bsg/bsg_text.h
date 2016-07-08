/******************************************************************************
 *   Broadcom Proprietary and Confidential. (c)2011-2012 Broadcom.  All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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

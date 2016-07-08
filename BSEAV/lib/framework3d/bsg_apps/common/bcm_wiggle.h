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

#ifndef __BCM_WIGGLE_H__
#define __BCM_WIGGLE_H__

#include "bsg_animation_list.h"
#include "bsg_shape.h"
#include "bsg_scene_node.h"

class QuadStripFactory : public bsg::StdFactory
{
public:
   QuadStripFactory(uint32_t steps, const bsg::Vec2 &origin, float width, float height, bsg::eAxis axis);
};

class Wiggle
{
public:
   Wiggle(const bsg::EffectHandle &effect, uint32_t steps, float width, float height) :
      m_material(bsg::New)
   {
      m_material->SetEffect(effect);
      m_geom = QuadStripFactory(steps, bsg::Vec2(0.0f, -height / 2.0f), width, height, bsg::eY_AXIS).MakeGeometry(m_material);
   }

   virtual ~Wiggle() {}

   bsg::AnimTarget<float> &GetAmplitude()      const { return m_material->GetUniform1f("u_amp");         }
   bsg::AnimTarget<float> &GetFrequency()      const { return m_material->GetUniform1f("u_freq");        }
   bsg::AnimTarget<float> &GetPhase()          const { return m_material->GetUniform1f("u_phase");       }

   bsg::AnimTarget<float> &GetBulgeAmplitude() const { return m_material->GetUniform1f("u_bulgeAmp");    }
   bsg::AnimTarget<float> &GetBulgeOffset()    const { return m_material->GetUniform1f("u_bulgeOffset"); }
   bsg::AnimTarget<float> &GetBulgeFrequency() const { return m_material->GetUniform1f("u_bulgeFreq");   }
   bsg::AnimTarget<float> &GetBulgePhase()     const { return m_material->GetUniform1f("u_bulgePhase");  }

   bsg::AnimTarget<float>     &GetAlpha()      const { return m_material->GetUniform1f("u_alpha");       }
   bsg::AnimTarget<bsg::Vec3> &GetColor()      const { return m_material->GetUniform3f("u_color");       }

   void SetAmplitude(float amp)           const { m_material->SetUniform("u_amp", amp);             }
   void SetFrequency(float freq)          const { m_material->SetUniform("u_freq", freq);           }
   void SetPhase(float phase)             const { m_material->SetUniform("u_phase", phase);         }

   void SetBulgeAmplitude(float amp)      const { m_material->SetUniform("u_bulgeAmp", amp);        }
   void SetBulgeOffset(float offset)      const { m_material->SetUniform("u_bulgeOffset", offset);  }
   void SetBulgeFrequency(float freq)     const { m_material->SetUniform("u_bulgeFreq", freq);      }
   void SetBulgePhase(float phase)        const { m_material->SetUniform("u_bulgePhase", phase);    }

   void SetAlpha(float alpha)             const { m_material->SetUniform("u_alpha", alpha);         }
   void SetColor(const bsg::Vec3 &color)  const { m_material->SetUniform("u_color", color);         }

   void SetWave(float amp, float freq, float phase)
   {
      SetAmplitude(amp);
      SetFrequency(freq);
      SetPhase(phase);
   }

   void SetBulge(float amp, float offset, float freq, float phase)
   {
      SetBulgeAmplitude(amp);
      SetBulgeOffset(offset);
      SetBulgeFrequency(freq);
      SetBulgePhase(phase);
   }

   const bsg::GeometryHandle &GetGeometry() const
   {
      return m_geom;
   }

private:
   bsg::MaterialHandle    m_material;
   bsg::GeometryHandle    m_geom;
};

// Implements a wiggling set of three sine waves which mimic the
// Broadcom presentation style guide.
class BCMWiggle
{
public:
   BCMWiggle(bsg::AnimationList &animList, const bsg::Time &now, float scale);

   bsg::SceneNodeHandle &GetRoot() { return m_root; }

   void SetColor(const bsg::Vec3 &color)
   {
      m_wiggle1.SetColor(color);
      m_wiggle2.SetColor(color);
      m_wiggle3.SetColor(color);
   }

private:
   bsg::SceneNodeHandle m_root;
   bsg::EffectHandle    m_effect;
   Wiggle               m_wiggle1;
   Wiggle               m_wiggle2;
   Wiggle               m_wiggle3;
};

#endif /* __BCM_WIGGLE_H__ */

/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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

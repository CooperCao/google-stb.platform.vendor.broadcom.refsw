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

#ifndef __BSG_ANIMATOR_H__
#define __BSG_ANIMATOR_H__

#include <stdlib.h>
#include <vector>
#include <algorithm>
#include "bsg_common.h"
#include "bsg_transform.h"
#include "bsg_vector.h"
#include "bsg_text.h"
#include "bsg_catmullrom.h"
#include "bsg_circle.h"
#include "bsg_time.h"

namespace bsg
{

/** @addtogroup animation Animation
The animation classes are designed to allow a wide range of animation effects to be simply
applied and managed. There are numerous animation interpolators to choose from, any of which
can be 'bound' to any animatable suitable target object.

In order to setup an animation which will automatically update:
- Create an animation binding object supplying the target object
- Initialize its interpolator with the start and end times
- Initialize its evaluator with the start and end values of the target animatable object
- Add the binding to the animation list which is going to be updated by the application

Here is an example which sets up a linear interpolation of a quaternion angle over a 10 seconds period.
The interpolator is flagged as extrapolating, so the animation will run on beyond 10 seconds.
\code
Time now = FrameTimestamp();
AnimBindingLerpQuaternionAngle *anim = new AnimBindingLerpQuaternionAngle(&sceneNode->GetRotation());
anim->Interpolator()->Init(now, now + 10.0f, BaseInterolator::EXTRAPOLATE);
anim->Evaluator()->Init(Vec3(0.0f, 1.0f, 0.0f), 0.0f, 360.0f);
m_animList.Append(anim);
\endcode
@{
*/

///////////////////////////////////////////////////////////////////////

//! Notifier class used to inform the user's application that an animation has completed.
//! The notifier will only be called if previously registered against an animation using
//! bsg::Interpolator::Init() or bsg::AnimBinding::Init()
class AnimationDoneNotifier
{
public:
   AnimationDoneNotifier() : m_notified(false) {}
   virtual ~AnimationDoneNotifier() {}

   //! Method called when an animation has completed
   void NotifyNow(const Time &time) { m_notified = true; Notify(time); }

   //! Returns true if the notifier has already been called
   bool HasBeenNotified() const { return m_notified; }

   //! Resets the notification status, so that it will notify on the next completion again
   void Reset() { m_notified = false; }

protected:
   //! Concrete classes implement this method to get a one shot notification when the animation is done.
   virtual void Notify(const Time &time) = 0;

private:
   bool m_notified;
};

//! Simplest possible notifier class.  The Notify method does nothing, but you can still query whether a
//! notification has happened.
class AnimationStatusNotifier : public AnimationDoneNotifier
{
public:
   virtual void Notify(const Time &/*time*/) {}
};

///////////////////////////////////////////////////////////////////////

class BaseInterpolator;

//! Base class for all animation bindings.
//!
//! An animation binding ties an evaluator and interpolator to a target.
class AnimBindingBase
{
public:
   AnimBindingBase() : m_deleteWhenDone(true) {}
   virtual ~AnimBindingBase() {}
   //! Update the animation to the given time.
   //! Returns true if the animation has started, but not yet finished.
   virtual bool UpdateTime(const Time &t) = 0;
   //! Returns true if the animation will have started at the given time.
   virtual bool Started(const Time &t) = 0;
   //! Returns true if the animation will have finished at the given time.
   virtual bool Finished(const Time &t) = 0;
   //! Returns the number of time difference from the given time, to the start of the animation.
   //! Returns 0 if the animation is currently running.
   //! Returns -1 if the animation has finished.
   virtual int64_t TimeToStartMs(const Time &t) = 0;
   //! Returns the underlying interpolator
   virtual BaseInterpolator *GetBaseInterpolator() = 0;

   friend class RunAnimSequenceNode;
   friend class DelaySequenceNode;
   friend class AnimationList;

protected:
   virtual bool DeleteWhenDone() const { return m_deleteWhenDone; }
   virtual void SetDeleteWhenDone(bool tf) { m_deleteWhenDone = tf; }

   bool m_deleteWhenDone;
};

///////////////////////////////////////////////////////////////////////

//! The base class of all evaluators.
//! An Evaluator calculates the value of a particular target at a given time.
template <class T>
class Evaluator
{
public:
   virtual ~Evaluator() {}

   //! All derived evaluators need to implement this Evaluate method to return the value at time p.
   virtual T Evaluate(float p) const = 0;

   typedef T Type;
};

template <class T>
class EvaluatorStartEnd : public Evaluator<T>
{
public:
   virtual ~EvaluatorStartEnd() {}

   //! Initialize the start and end values of the Evaluator
   virtual void Init(const T &start, const T &end)
   {
      m_start = start;
      m_end = end;
   }

protected:
   T     m_start;
   T     m_end;
};

///////////////////////////////////////////////////////////////////////

//! An evaluator for types that already implement operator* and operator+
template <class T>
class GenericEvaluator : public EvaluatorStartEnd<T>
{
public:
   //! Evaluate the value at time p.
   virtual T Evaluate(float p) const
   {
      return (EvaluatorStartEnd<T>::m_start * (1.0f - p)) + (EvaluatorStartEnd<T>::m_end * p);
   }
};

///////////////////////////////////////////////////////////////////////

//! An evaluator for bsg::Transform objects
class TransformEvaluator : public EvaluatorStartEnd<Transform>
{
public:
   //! Evaluate the Transform at time p.
   virtual Transform Evaluate(float p) const
   {
      Transform tran;
      tran.SetPosition((m_start.GetPosition() * (1.0f - p)) + (m_end.GetPosition() * p));
      tran.SetRotation(m_start.GetRotation().SLerp(p, m_end.GetRotation()));
      tran.SetScale((m_start.GetScale() * (1.0f - p)) + (m_end.GetScale() * p));
      return tran;
   }
};

///////////////////////////////////////////////////////////////////////

//! An evaluator for bsg::Quaternion objects
class QuaternionEvaluator : public EvaluatorStartEnd<Quaternion>
{
public:
   //! Evaluate the Quaternion at time p.
   virtual Quaternion Evaluate(float p) const
   {
      return m_start.SLerp(p, m_end);
   }
};

///////////////////////////////////////////////////////////////////////

//! An evaluator for just the angle component of bsg::Quaternion objects
class QuaternionAngleEvaluator : public Evaluator<Quaternion>
{
public:
   //! Initialize need the axis, since it cannot be extracted from an existing Quaternion
   virtual void Init(const Vec3 &axis, float startAngle, float endAngle)
   {
      m_axis = axis;
      m_startAngle = startAngle;
      m_endAngle = endAngle;
   }

   //! Evaluate the angle at time p.
   virtual Quaternion Evaluate(float p) const
   {
      // Special case for angle
      return Quaternion((m_startAngle * (1.0f - p)) + (m_endAngle * p), m_axis);
   }

private:
   Vec3  m_axis;
   float m_startAngle;
   float m_endAngle;
};


///////////////////////////////////////////////////////////////////////

#ifndef BSG_STAND_ALONE

//! An evaluator for bsg::TextParams objects
class TextParamsEvaluator : public EvaluatorStartEnd<TextParams>
{
public:
   //! Evaluate the params at time p.
   virtual TextParams Evaluate(float p) const
   {
      TextParams res;
      res.SetOpacity((m_start.GetOpacity() * (1.0f - p)) + (m_end.GetOpacity() * p));
      res.SetColor((m_start.GetColor() * (1.0f - p)) + (m_end.GetColor() * p));
      res.SetPosition((m_start.GetPosition() * (1.0f - p)) + (m_end.GetPosition() * p));
      return res;
   }
};

#endif

///////////////////////////////////////////////////////////////////////
//! An evaluator for circular positions
class CircleEvaluator : public Evaluator<Vec3>
{
public:
   virtual void Init(const Circle &circle)
   {
      m_circle = circle;
   }

   //! Evaluate the circle position at time p.
   virtual Vec3 Evaluate(float p) const
   {
      return m_circle.Evaluate(p);
   }

private:
   Circle m_circle;
};

///////////////////////////////////////////////////////////////////////

//! An evaluator for positions (or any vec3) on CatmullRom splines
class CatmullRomSplineEvaluator : public Evaluator<Vec3>
{
public:
   virtual void Init(const CatmullRomSpline &spline)
   {
      m_spline = spline;
   }

   //! Evaluate the vec3 at time p.
   virtual Vec3 Evaluate(float p) const
   {
      return m_spline.Evaluate(p);
   }

private:
   mutable CatmullRomSpline m_spline;
};

// Calculate a Euclidean "t mod m"
static Time Repeat(const Time &t, const Time &m)
{
   int64_t t64 = t.Milliseconds();
   int64_t m64 = m.Milliseconds();

   if (m64 == 0)
      return t;

   // Shift the number into the positive domain
   if (t64 < 0)
   {
      int64_t div = abs(t64 / m64);
      t64 += (div + 1) * m64;
   }

   t64 = t64 % m64;

   return Time(t64, Time::eMILLISECONDS);
}

// Calculate an oscillating modulo which reflects the result every other
// span.
static Time Mirror(const Time &t, const Time &m)
{
   int64_t t64 = t.Milliseconds();
   int64_t m64 = m.Milliseconds();

   // Have to shift by an even number or the zero will have been shifted
   int64_t div = abs(t64 / m64);

   bool down = (div & 1) == 1;

   if (t64 < 0)
   {
      int64_t div = abs(t64 / m64);
      t64 += (div + 2) * m64;
      down = !down;
   }

   t64 = t64 % m64;

   // If its an odd span, then the time should be in the opposite sense
   if (down)
      t64 = m64 - t64;

   return Time(t64, Time::eMILLISECONDS);
}

///////////////////////////////////////////////////////////////////////
//! Base class of all interpolators.
//! An interpolator is used to vary the time-step sequence during an animation.
//! So, instead of just a linear interpolation from start to end position, the
//! interpolator classes allow easing curves, hermite curves etc.
class BaseInterpolator
{
public:
   enum eMode
   {
      LIMIT = 0,      //!< \deprecated Use eLIMIT
      EXTRAPOLATE,    //!< \deprecated Use eEXTRAPOLATE

      eLIMIT       = LIMIT,
      eEXTRAPOLATE = EXTRAPOLATE,
      eREPEAT,
      eMIRROR
   };

   BaseInterpolator()
   {
      Initialise(eLIMIT, 0.0f, 0.0f, 0);
   }

   //! Interpolators have a start time, end time and can optionally extrapolate outside of that range.
   BaseInterpolator(const Time &start, const Time &end, eMode mode)
   {
      Initialise(mode, start, end, 0);
   }

   //! Interpolators have a start time, end time and can optionally extrapolate outside of that range.
   //! \deprecated Use the constructor taking eMode instead
   BaseInterpolator(const Time &start, const Time &end, bool extrapolate) :
      m_mode(extrapolate ? eEXTRAPOLATE : eLIMIT),
      m_startTime(start), m_endTime(end), m_duration(end - start),
      m_notifier(0)
   {
      eMode mode = extrapolate ? eEXTRAPOLATE : eLIMIT;
      Initialise(mode, start, end, 0);
   }

   virtual ~BaseInterpolator() {  }

   //! Init the extrapolation mode only.
   //! If a notifier is given, it will be notified when the animation is complete.
   virtual void Init(eMode mode, AnimationDoneNotifier *notifier = NULL)
   {
      m_mode     = mode;
      m_notifier = notifier;
   }

   //! Init the start, end and extrapolation.
   //! If a notifier is given, it will be notified when the animation is complete.
   virtual void Init(const Time &start, const Time &end, eMode mode = LIMIT, AnimationDoneNotifier *notifier = NULL)
   {
      Initialise(mode, start, end, notifier);
   }

   //! Change the start and end time
   virtual void SetStartAndEndTimes(const Time &start, const Time &end)
   {
      Initialise(m_mode, start, end, m_notifier);
   }

   //! Change the start and end time, using duration
   virtual void SetStartAndDuration(const Time &start, const Time &duration)
   {
      Initialise(m_mode, start, start + duration, m_notifier);
   }

   //! Sets the done notifier explicitly
   virtual void SetAnimationDoneNotifier(AnimationDoneNotifier *notifier)
   {
      m_notifier = notifier;
      if (m_notifier)
         m_notifier->Reset();
   }

   //! Sets whether to extrapolate or not.
   virtual void SetMode(eMode mode) { m_mode = mode; }

   //! Sets whether to extrapolate or not.
   //! \deprecated Use SetMode() instead.
   virtual void SetExtrapolate(bool extrapolate) { m_mode = extrapolate ? eEXTRAPOLATE : eLIMIT; }

   //! Returns the modified time (which should be used for evaluate) given the linear time of the animation.
   virtual float GetParamValue(const Time &time) const
   {
      float  res = 0.0f;

      // Clamp range if required
      switch (m_mode)
      {
      case eLIMIT:
         if (time > m_endTime)
            res = 1.0f;
         else if (time < m_startTime)
            res = 0.0f;
         else
            res = GetInternalParamValue(time);
         break;

      case eEXTRAPOLATE:
         res = GetInternalParamValue(time);
         break;

      case eREPEAT:
         res = GetInternalParamValue(m_startTime + Repeat((time - m_startTime), m_duration));
         break;

      case eMIRROR:
         res = GetInternalParamValue(m_startTime + Mirror((time - m_startTime), m_duration));
         break;
      }

      return res;
   }

   //! Returns true if the interpolator will have started at the given time
   virtual bool Started(const Time &time)  const { return time >= m_startTime || (m_mode != eLIMIT); }
   //! Returns true if the interpolator will have finished at the given time
   virtual bool Finished(const Time &time) const { return time >= m_endTime   && (m_mode == eLIMIT); }

   //! Returns the time difference from time until start of interpolation.
   //! Returns 0 if already started (but not finished)
   //! Returns -1 if finished.
   virtual int64_t TimeToStartMs(const Time &time) const
   {
      if (m_mode != eLIMIT)
         return 0;
      else
      {
         if (time >= m_startTime && time <= m_endTime)
            return 0;
         else if (time < m_startTime)
            return m_startTime.Milliseconds() - time.Milliseconds();
         else
            return -1;
      }
   }

   //! Returns the notifier that may have been set.
   AnimationDoneNotifier *Notifier() { return m_notifier; }

protected:
   float CalcParam(const Time &time) const
   {
      // If the duration is zero then we return the mid-point.
      if (m_duration.Milliseconds() == 0)
         return 0.5f;

      // We try to retain accuracy by doing int64 arithmetic and go to float as late as possible
      return TIME_ARITH_RECIP * (((time - m_startTime).Milliseconds() * TIME_ARITH_FACTOR) / m_duration.Milliseconds());

      //return (time - m_startTime).Milliseconds() / (double)m_duration.Milliseconds();
   }

   virtual float GetInternalParamValue(const Time &time) const = 0;

private:
   void Initialise(eMode mode, const Time &start, const Time &end, AnimationDoneNotifier *notifier)
   {
      m_mode      = mode;
      m_startTime = start;
      m_endTime   = end;
      m_duration  = std::max(Time(0.0f), end - start);
      m_notifier  = notifier;
   }

protected:
   eMode                   m_mode;
   Time                    m_startTime;
   Time                    m_endTime;
   Time                    m_duration;
   AnimationDoneNotifier   *m_notifier;
};

///////////////////////////////////////////////////////////////////////

//! A linear interpolator
class LinearInterpolator : public BaseInterpolator
{
public:
   //! Return 0 at start time, 1 at end time, and linear interpolation in between. Will extrapolate outside range.
   virtual float GetInternalParamValue(const Time &time) const
   {
      return CalcParam(time);
   }
};

///////////////////////////////////////////////////////////////////////

#if 0
//! A cyclic linear interpolator. Times outside of start and end will still return 0 to 1 range results.
class LinearModuloInterpolator : public BaseInterpolator
{
public:
   //! Return 0 at start time, 1 at end time, and linear interpolation in between.
   //! Values outside range will wrap to still return 0 to 1 results.
   virtual float GetInternalParamValue(const Time &time) const
   {
      // We try to retain accuracy by doing int64 arithmetic and go to float as late as possible
      int64_t ms = (time - m_startTime).Milliseconds();
      int64_t whole = ms / m_duration.Milliseconds();

      int64_t moduloMs = ms - whole * m_duration.Milliseconds();

      float p = (float)moduloMs / (float)m_duration.Milliseconds();
      return p;
   }
};
#endif

///////////////////////////////////////////////////////////////////////

//! Interpolator that accelerates in from zero velocity, in a cubic manner.
class CubicInInterpolator : public BaseInterpolator
{
public:
   //! Return 0 at start time, 1 at end time, and cubic-ease-in interpolation in between
   virtual float GetInternalParamValue(const Time &time) const
   {
      float t = CalcParam(time);
      return t * t * t;
   }
};

///////////////////////////////////////////////////////////////////////

//! Interpolator that decelerates out to zero velocity, in a cubic manner.
class CubicOutInterpolator : public BaseInterpolator
{
public:
   //! Return 0 at start time, 1 at end time, and cubic-ease-out interpolation in between
   virtual float GetInternalParamValue(const Time &time) const
   {
      float t = CalcParam(time);
      t = t - 1.0f;
      return t * t * t + 1.0f;
   }
};

///////////////////////////////////////////////////////////////////////

//! Interpolator that accelerates in and decelrates out, in a cubic manner.
class CubicInterpolator : public BaseInterpolator
{
public:
   //! Return 0 at start time, 1 at end time, and cubic interpolation in between
   virtual float GetInternalParamValue(const Time &time) const
   {
      float t = CalcParam(time);

      t = t * 2.0f;
      if (t < 1.0f)
      {
         return 0.5f * t * t * t;
      }
      else
      {
         t = t - 2.0f;
         return 0.5f * (t * t * t + 2.0f);
      }
   }
};

///////////////////////////////////////////////////////////////////////

//! Interpolator that accelerates in from zero verlocity, in a circular manner.
class CircInInterpolator : public BaseInterpolator
{
public:
   //! Return 0 at start time, 1 at end time, and circular-ease-in interpolation in between
   virtual float GetInternalParamValue(const Time &time) const
   {
      float t = CalcParam(time);

      return -(sqrtf(1.0f - t * t) - 1.0f);
   }
};

///////////////////////////////////////////////////////////////////////

//! Interpolator that decelerates out to zero velocity, in a circular manner.
class CircOutInterpolator : public BaseInterpolator
{
public:
   //! Return 0 at start time, 1 at end time, and circular-ease-out interpolation in between
   virtual float GetInternalParamValue(const Time &time) const
   {
      float t = CalcParam(time);
      t = t - 1.0f;
      return sqrtf(1.0f - t * t);
   }
};

///////////////////////////////////////////////////////////////////////

//! Interpolator that accelerates in and decelerates out, in a circular manner.
class CircInterpolator : public BaseInterpolator
{
public:
   //! Return 0 at start time, 1 at end time, and circular interpolation in between
   virtual float GetInternalParamValue(const Time &time) const
   {
      float t = CalcParam(time);

      t = t * 2.0f;
      if (t < 1.0f)
      {
         return -0.5f * (sqrtf(1.0f - t * t) - 1.0f);
      }
      else
      {
         t = t - 2.0f;
         return 0.5f * (sqrtf(1.0f - t * t) + 1.0f);
      }
   }
};

///////////////////////////////////////////////////////////////////////

//! Interpolator that reverses past the start point before moving to the end.
//! Accelerates from zero velocity.
class OvershootInInterpolator : public BaseInterpolator
{
public:
   OvershootInInterpolator() :
      m_overshoot(1.7f)
   {
   }

   //! Set the overshoot factor. Defaults to 1.7
   virtual void SetOvershoot(float oshoot) { m_overshoot = oshoot; }

   //! Return 0 at start time, 1 at end time, and overshoot-in interpolation in between
   virtual float GetInternalParamValue(const Time &time) const
   {
      float t = CalcParam(time);

      return t * t * ((m_overshoot + 1.0f) * t - m_overshoot);
   }

protected:
   float m_overshoot;
};

///////////////////////////////////////////////////////////////////////

//! Interpolator that moves past the end point before returning to the end.
//! Decelerates to zero velocity.
class OvershootOutInterpolator : public BaseInterpolator
{
public:
   OvershootOutInterpolator() :
      m_overshoot(1.7f)
   {
   }

   //! Set the overshoot factor. Defaults to 1.7
   virtual void SetOvershoot(float oshoot) { m_overshoot = oshoot; }

   //! Return 0 at start time, 1 at end time, and overshoot-out interpolation in between
   virtual float GetInternalParamValue(const Time &time) const
   {
      float t = CalcParam(time);

      t = t - 1.0f;
      return t * t * ((m_overshoot + 1) * t + m_overshoot) + 1.0f;
   }

protected:
   float m_overshoot;
};

///////////////////////////////////////////////////////////////////////

//! Interpolator that reverses past the start point and moves past the end point before returning to the end point.
class OvershootInterpolator : public BaseInterpolator
{
public:
   OvershootInterpolator() :
      m_overshoot(1.7f)
   {
   }

   //! Set the overshoot factor. Defaults to 1.7
   virtual void SetOvershoot(float oshoot) { m_overshoot = oshoot; }

   //! Return 0 at start time, 1 at end time, and overshoot interpolation in between
   virtual float GetInternalParamValue(const Time &time) const
   {
      float t = CalcParam(time);

      t = t * 2.0f;
      if (t < 1.0f)
      {
         return 0.5f * (t * t * ((m_overshoot + 1.0f) * t - m_overshoot));
      }
      else
      {
         t = t - 2.0f;
         return 0.5f * (t * t * ((m_overshoot + 1.0f) * t + m_overshoot) + 2.0f);
      }
   }

protected:
   float m_overshoot;
};

///////////////////////////////////////////////////////////////////////

//! An elastic-out interpolator (exponentially decaying sine wave).
//! Decelerates to zero velocity.
class ElasticOutInterpolator : public BaseInterpolator
{
public:
   ElasticOutInterpolator() :
      m_amplitude(1.0f),
      m_period(0.3f)
   {
   }

   //! Set the peak amplitude of the sine wave. Defaults to 1.0
   virtual void SetAmplitude(float amp) { m_amplitude = amp; }

   //! Set the period of decay. Defaults to 0.3
   virtual void SetPeriod(float period) { m_period = period; }

   //! Return 0 at start time, 1 at end time, and elastic interpolation in between
   virtual float GetInternalParamValue(const Time &time) const
   {
      float t = CalcParam(time);

      if (t == 0)
         return 0.0f;

      if (t == 1)
         return 1.0f;

      float s;
      float a = m_amplitude;

      if (a < 1.0f)
      {
         a = 1.0f;
         s = m_period / 4.0f;
      }
      else
      {
         s = m_period / (2.0f * (float)M_PI) * asinf(1.0f / a);
      }

      return (a * powf(2.0f, -10.0f * t) * sinf((t - s) * (2.0f * (float)M_PI) / m_period) + 1.0f);
   }

protected:
   float m_amplitude;
   float m_period;
};

///////////////////////////////////////////////////////////////////////

//! Hermite interpolator that accelerates in from zero velocity, and out to zero velocity.
class HermiteInterpolator : public BaseInterpolator
{
public:
   //! Return 0 at start time, 1 at end time, and hermite interpolation in between
   virtual float GetInternalParamValue(const Time &time) const
   {
      float t = CalcParam(time);
      float t2 = t * t;
      t = 3.0f * t2 - 2.0f * t2 * t;
      return t;
   }
};

///////////////////////////////////////////////////////////////////////

//! DoubleHermite interpolator that accelerates in from zero velocity, and out to zero velocity.
class DoubleHermiteInterpolator : public BaseInterpolator
{
public:
   //! Return 0 at start time, 1 at end time, and hermite interpolation in between
   virtual float GetInternalParamValue(const Time &time) const
   {
      float t = CalcParam(time);
      float t2 = t * t;
      t = 3.0f * t2 - 2.0f * t2 * t;
      t2 = t * t;
      t = 3.0f * t2 - 2.0f * t2 * t;
      return t;
   }
};

template <class E>
class AnimBindingEval : public AnimBindingBase
{
public:
   AnimBindingEval() :
      m_animAdaptor(0),
      m_evaluator(new E)
   {}

   template <class F>
   AnimBindingEval(AnimTarget<F> *animTarget) :
      m_animAdaptor(0),
      m_evaluator(new E)
   {
      Bind(animTarget);
   }

   virtual ~AnimBindingEval()
   {
      delete m_evaluator;
      delete m_animAdaptor;
   }

   virtual BaseInterpolator *GetBaseInterpolator() = 0;

   //! Shortcut initializer for standard cases
   void Init(const typename E::Type &startValue, const Time &start,
             const typename E::Type &endValue, const Time &end,
             BaseInterpolator::eMode mode = BaseInterpolator::eLIMIT, 
             AnimationDoneNotifier *notifier = NULL)
   {
      GetBaseInterpolator()->Init(start, end, mode, notifier);
      m_evaluator->Init(startValue, endValue);
   }

   //! Bind the interpolator & evaluator to an animation target.
   //! The target will be automatically modified when the animation updates.
   //! Can be used to rebind the target.  Prefer to construct with a target
   //! if possible, rather than construct then Bind.
   template <class F>
   void Bind(AnimTarget<F> *animTarget)
   {
      if (m_animAdaptor != 0)
         delete m_animAdaptor;

      //! The adaptor will convert to the appropriate type
      m_animAdaptor = new AnimAdaptor<typename E::Type, F>(animTarget);
   }

   //! Returns the Evaluator
   E *Evaluator() { return m_evaluator; }

protected:
   AnimTarget<typename E::Type>  *m_animAdaptor;
   E                             *m_evaluator;
};

///////////////////////////////////////////////////////////////////////

//! An animation binding ties an evaluator and interpolator to a particular animation target (which
//! must be defived from AnimTarget<E::T>.
//!
//! For most combinations of evaluator, interpolator and target type, typedefs are available. e.g.
//! - AnimBindingLerpFloat
//! - AnimBindingLerpTransform
//! - AnimBindingHermiteTransform
//! - AnimBindingHermiteCatmullRom
//! - etc, etc.
template <class E, class I>
class AnimBinding : public AnimBindingEval<E>
{
public:
   //! Create an unbound animation binding. Target must be filled in with Bind() later.
   AnimBinding() :
      AnimBindingEval<E>(),
      m_interpolator(new I)
   {
   }

   //! Create animation binding and bind to target.
   //! The target will be automatically modified when the animation updates.
   //! We do not own the target.
   template <class F>
   AnimBinding(AnimTarget<F> *animTarget) :
      AnimBindingEval<E>(animTarget),
      m_interpolator(new I)
   {
      AnimBindingEval<E>::Bind(animTarget);
   }

   virtual ~AnimBinding()
   {
      delete m_interpolator;
   }

   //! Returns the underlying interpolator
   BaseInterpolator *GetBaseInterpolator() { return (BaseInterpolator*)m_interpolator; }

   //! Returns the Interpolator
   I *Interpolator() { return m_interpolator; }

   //! Update the animation time to the given point.
   //! Modifies the animation target as appropriate to represent the new time value.
   //! Will notify the registered notifier (if any) when the animation is complete.
   virtual bool UpdateTime(const Time &time)
   {
      if (AnimBindingEval<E>::m_animAdaptor == 0)
         BSG_THROW("Animator has not been bound");

      bool started = m_interpolator->Started(time);
      bool finished = m_interpolator->Finished(time);

      // We could bail here if not started, but the limit case then behaves
      // differently to the extrapolate cases and the target needs setting explicitly if
      // outside the range.  It is also asymetrical with respect to start and finish.

      // Assign the new value via the Set function
      AnimBindingEval<E>::m_animAdaptor->Set(AnimBindingEval<E>::m_evaluator->Evaluate(m_interpolator->GetParamValue(time)));

      if (finished && m_interpolator->Notifier() != NULL)
      {
         if (!m_interpolator->Notifier()->HasBeenNotified())
            m_interpolator->Notifier()->NotifyNow(time);
      }

      return started && !finished;
   }

   //! Return true if the animation will have started at the given time
   virtual bool Started(const Time &time)
   {
      return m_interpolator->Started(time);
   }

   //! Return true if the animation will have finished at the given time
   virtual bool Finished(const Time &time)
   {
      return m_interpolator->Finished(time);
   }

   //! Return the time in milliseconds from the given time to the start of the animation.
   //! Returns 0 if the animation has already started.
   //! Returns -1 if the animation has finished.
   virtual int64_t TimeToStartMs(const Time &time)
   {
      return m_interpolator->TimeToStartMs(time);
   }

private:
   I  *m_interpolator;
};

////////////////////////////////////////////////////////////////////////////

//   typedef AnimBinding<E, LinearModuloInterpolator>   AnimBindingLerpMod##T;

#define DEFINE_BINDINGS(T, E) \
   typedef AnimBinding<E, LinearInterpolator>         AnimBindingLerp##T;           \
   typedef AnimBinding<E, HermiteInterpolator>        AnimBindingHermite##T;        \
   typedef AnimBinding<E, DoubleHermiteInterpolator>  AnimBindingDoubleHermite##T;  \
   typedef AnimBinding<E, CubicInInterpolator>        AnimBindingCubicIn##T;        \
   typedef AnimBinding<E, CubicOutInterpolator>       AnimBindingCubicOut##T;       \
   typedef AnimBinding<E, CubicInterpolator>          AnimBindingCubic##T;          \
   typedef AnimBinding<E, CircInInterpolator>         AnimBindingCircIn##T;         \
   typedef AnimBinding<E, CircOutInterpolator>        AnimBindingCircOut##T;        \
   typedef AnimBinding<E, CircInterpolator>           AnimBindingCirc##T;           \
   typedef AnimBinding<E, OvershootInInterpolator>    AnimBindingOvershootIn##T;    \
   typedef AnimBinding<E, OvershootOutInterpolator>   AnimBindingOvershootOut##T;   \
   typedef AnimBinding<E, OvershootInterpolator>      AnimBindingOvershoot##T;      \
   typedef AnimBinding<E, ElasticOutInterpolator>     AnimBindingElasticOut##T

DEFINE_BINDINGS(Transform,       TransformEvaluator);
DEFINE_BINDINGS(Float,           GenericEvaluator<Float>);
DEFINE_BINDINGS(Vec2,            GenericEvaluator<Vec2>);
DEFINE_BINDINGS(Vec3,            GenericEvaluator<Vec3>);
DEFINE_BINDINGS(Vec4,            GenericEvaluator<Vec4>);
DEFINE_BINDINGS(Quaternion,      QuaternionEvaluator);
DEFINE_BINDINGS(QuaternionAngle, QuaternionAngleEvaluator);
#ifndef BSG_STAND_ALONE
DEFINE_BINDINGS(TextParams,      TextParamsEvaluator);
#endif
DEFINE_BINDINGS(CatmullRom,      CatmullRomSplineEvaluator);
DEFINE_BINDINGS(Circle,          CircleEvaluator);
DEFINE_BINDINGS(Time,            GenericEvaluator<Time>);

/** @} */ // End of Animation group

///////////////////////////////////////////////////////////////////////

}

#endif /* __BSG_ANIMATOR_H__ */

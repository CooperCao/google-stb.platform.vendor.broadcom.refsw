/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_ANIMATABLE_H__
#define __BSG_ANIMATABLE_H__

#include "bsg_common.h"
#include "bsg_number.h"
#include "bsg_vector.h"
#include "bsg_exception.h"
#include "bsg_quaternion.h"
#include "bsg_gl_uniform.h"
#include "bsg_time.h"

#include <vector>

//! @ingroup animation

namespace bsg
{

//! This meta-function gives the efficient parameter passing mechanism for the type parameter.
//! So ByValue<T>::Type is either const T &, for classes or just T for int or float.
//! The animation implementation uses ByValue to pass parameters efficiently.
//! This is a limited implementation which works for classes and and ints and floats.
//! It will not behave for e.g. references, pointers or double for example.
//!
//! ByValue<T>::Type is const T & by default.
template <class T> struct ByValue        { typedef const T &Type;  };

//! Pass ints by value.  ByValue<int>::Type is int.
template <>        struct ByValue<int>   { typedef int     Type;   };

//! Pass floats by value.  ByValue<float>::Type is float.
template <>        struct ByValue<float> { typedef float   Type;   };

//! @{
//! The AnimTarget class provides just a Set() and Get() method.  It is used
//! by e.g. animations to set the target.  Note that an AnimTarget<Vec3>, for example,
//! might not actually be a Vec3.  It could be a Uniform3f or a Vec2 (dropping a field, see the AnimAdaptor class)
//!
//! Software components that expose animatable controls should use these base classes rather than
//! exposing the concrete class.
//!
//! AnimTarget<T> is an abstract base class.
//! @}
template <class T>
class AnimTarget
{
public:
   //! Mandatory virtual destructor for base class
   virtual ~AnimTarget() {}

   //! Set the value by whatever means are necessary.
   virtual void   Set(typename ByValue<T>::Type value) = 0;
   virtual T      Get() const = 0;
};

//! AnimType selects the target type which for most classes is the template parameter itself.
//! However for floats and ints, we have a class version (Number<float>, Number<int>) masquerading as
//! floats and ints, so we map those types to their underlying representation.  That way e.g. an AnimTarget<float>
//! can be an Animatable<Float> or an AnimatableUniform<float>
template <class T> struct AnimType        { typedef T     Type;   };
template <>        struct AnimType<Float> { typedef float Type;   };
template <>        struct AnimType<Int>   { typedef int   Type;   };

//! The base class for all simple animatable objects where the
//! target is the same as the type.  Derives from the target type
//! so as to exhibit the target's interface.
//!
//! Simple types like float and int are special cases and are wrapped
//! in a class that looks a lot like a float or int.  The target type
//! of the AnimTarget is still float or int though.
template <class T>
class Animatable : public T, public AnimTarget<typename AnimType<T>::Type>
{
   typedef typename AnimType<T>::Type Type;

public:
   // Set base T to default value
   Animatable() : T() {}
   // Initialise base T
   Animatable(typename ByValue<Type>::Type v) : T(v) {}

   // Assign to base T
   virtual void   Set(typename ByValue<Type>::Type value) { T::operator=(value);  }
   virtual Type   Get() const                             { return *this;         }
};

//! @ingroup animation
//! Animatable instances animatable vectors, quaternions and matrices etc.
//! @{

typedef Animatable<Float>        AnimatableFloat;
typedef Animatable<Vec2>         AnimatableVec2;
typedef Animatable<Vec3>         AnimatableVec3;
typedef Animatable<Vec4>         AnimatableVec4;
typedef Animatable<IVec2>        AnimatableIVec2;
typedef Animatable<IVec3>        AnimatableIVec3;
typedef Animatable<IVec4>        AnimatableIVec4;
typedef Animatable<Mat2>         AnimatableMat2;
typedef Animatable<Mat3>         AnimatableMat3;
typedef Animatable<Mat4>         AnimatableMat4;

#ifdef BSG_USE_ES3
typedef Animatable<Mat2x3>       AnimatableMat2x3;
typedef Animatable<Mat2x4>       AnimatableMat2x4;
typedef Animatable<Mat3x2>       AnimatableMat3x2;
typedef Animatable<Mat3x4>       AnimatableMat3x4;
typedef Animatable<Mat4x2>       AnimatableMat4x2;
typedef Animatable<Mat4x3>       AnimatableMat4x3;
#endif

typedef Animatable<Quaternion>   AnimatableQuaternion;
typedef Animatable<Time>         AnimatableTime;
//! @}


//! @ingroup animation
//! @{

//! Animatable group combines a number of animatables targetting the same base type so they
//! can all be set with a single Set() call or read with a single Get() call.  All the targets
//! will have the same value.
template <class T>
class AnimatableGroup : public AnimTarget<T>
{
public:
   //! Construct an empty group.
   AnimatableGroup() {}

   //! Adds a target to the group's list.
   void Append(AnimTarget<T> &a)
   {
      a.Set(m_value);
      m_anims.push_back(&a);
   }

   //! Sets all targets in the group to value
   virtual void Set(typename ByValue<T>::Type value)
   {
      m_value = value;

      for (uint32_t i = 0; i < m_anims.size(); ++i)
         m_anims[i]->Set(value);
   }

   //! Get the value associated with this group
   virtual T Get() const
   {
      return m_value;
   }

   //! Empty the target list
   void Clear()
   {
      m_anims.clear();
   }

private:
   std::vector<AnimTarget<T> *> m_anims;
   T                            m_value;
};

// @cond
// GLUniformX is a meta function that defines the "uniform" type corresponding
// to the template parameter parameter.
template <class T> struct GLUniformX          {                                };
template <>        struct GLUniformX<float>   { typedef GLUniform1f     Type;  };
template <>        struct GLUniformX<Vec2>    { typedef GLUniform2f     Type;  };
template <>        struct GLUniformX<Vec3>    { typedef GLUniform3f     Type;  };
template <>        struct GLUniformX<Vec4>    { typedef GLUniform4f     Type;  };
template <>        struct GLUniformX<int>     { typedef GLUniform1i     Type;  };
#ifdef BSG_USE_ES3
template <>        struct GLUniformX<unsigned>{ typedef GLUniform1u     Type;  };
#endif
template <>        struct GLUniformX<IVec2>   { typedef GLUniform2i     Type;  };
template <>        struct GLUniformX<IVec3>   { typedef GLUniform3i     Type;  };
template <>        struct GLUniformX<IVec4>   { typedef GLUniform4i     Type;  };
template <>        struct GLUniformX<Mat2>    { typedef GLUniformMat2   Type;  };
template <>        struct GLUniformX<Mat3>    { typedef GLUniformMat3   Type;  };
template <>        struct GLUniformX<Mat4>    { typedef GLUniformMat4   Type;  };

#ifdef BSG_USE_ES3
template <>        struct GLUniformX<Mat2x3> { typedef GLUniformMat2x3 Type;  };
template <>        struct GLUniformX<Mat2x4> { typedef GLUniformMat2x4 Type;  };
template <>        struct GLUniformX<Mat3x2> { typedef GLUniformMat3x2 Type;  };
template <>        struct GLUniformX<Mat3x4> { typedef GLUniformMat3x4 Type;  };
template <>        struct GLUniformX<Mat4x2> { typedef GLUniformMat4x2 Type;  };
template <>        struct GLUniformX<Mat4x3> { typedef GLUniformMat4x3 Type;  };
#endif

// @endcond

//! AnimatableUniforms are uniforms (concrete base class), and AnimTargets (abstract base class),
//! and can be attached to by animations of the appropriate types.
template <class T>
class AnimatableUniform : public GLUniformX<T>::Type, public AnimTarget<T>
{
public:
   typedef typename GLUniformX<T>::Type ValueBase;

   AnimatableUniform(const std::string &name) : ValueBase(name)
   {}

   virtual void   Set(typename ByValue<T>::Type value)  { ValueBase::SetValue(value);                        }
   virtual T      Get() const                           { return ValueBase::GetValue(); }
};

//! Convenience names for animatable uniforms.
typedef AnimatableUniform<float> AnimatableUniform1f;
typedef AnimatableUniform<Vec2>  AnimatableUniform2f;
typedef AnimatableUniform<Vec3>  AnimatableUniform3f;
typedef AnimatableUniform<Vec4>  AnimatableUniform4f;
typedef AnimatableUniform<int>   AnimatableUniform1i;
typedef AnimatableUniform<IVec2> AnimatableUniform2i;
typedef AnimatableUniform<IVec3> AnimatableUniform3i;
typedef AnimatableUniform<IVec4> AnimatableUniform4i;
typedef AnimatableUniform<Mat2>  AnimatableUniformMat2;
typedef AnimatableUniform<Mat3>  AnimatableUniformMat3;
typedef AnimatableUniform<Mat4>  AnimatableUniformMat4;

#ifdef BSG_USE_ES3
typedef AnimatableUniform<Mat2x3>  AnimatableUniformMat2x3;
typedef AnimatableUniform<Mat2x4>  AnimatableUniformMat2x4;
typedef AnimatableUniform<Mat3x2>  AnimatableUniformMat3x2;
typedef AnimatableUniform<Mat3x4>  AnimatableUniformMat3x4;
typedef AnimatableUniform<Mat4x2>  AnimatableUniformMat4x2;
typedef AnimatableUniform<Mat4x3>  AnimatableUniformMat4x3;
#endif

//! Base class of adaptors that look like F targets, but which actually
//! hold T values.
template <class F, class T>
class AnimAdaptorBase : public AnimTarget<F>
{
public:
   AnimAdaptorBase(AnimTarget<T> *target) :
      m_target(target)
   {}

protected:
   AnimTarget<T>  *m_target;
};

//! AnimAdaptor
//!
//! The animation adaptor classes serve to convert between the types generated by
//! the evaluator and the target type.
//!
//! The adaptor looks like an AnimTarget<F> (F = from), but actually controls an
//! underlying AnimTarget<T> (T = to or target).
//!
template <class F, class T>
class AnimAdaptor : public AnimAdaptorBase<F, T>
{
public:
   AnimAdaptor(AnimTarget<T> *target) : AnimAdaptorBase<F, T>(target) {}

   // Try a type cast in the default case
   virtual void   Set(typename ByValue<F>::Type value)  { AnimAdaptorBase<F, T>::m_target->Set(T(value));    }
   virtual F      Get() const                           { return F(AnimAdaptorBase<F, T>::m_target->Get());  }
};

//! The trivial adaptor that does nothing.
template <class T>
class AnimAdaptor<T, T> : public AnimAdaptorBase<T, T>
{
public:
   AnimAdaptor(AnimTarget<T> *target) : AnimAdaptorBase<T, T>(target) {}

   // No conversion necessary
   virtual void   Set(typename ByValue<T>::Type value)   { AnimAdaptorBase<T, T>::m_target->Set(value);    }
   virtual T      Get() const                            { return AnimAdaptorBase<T, T>::m_target->Get();  }
};

//! Adaptor that looks like a Vec3 target, but controls a Vec2 by dropping the z-component.
template <>
class AnimAdaptor<Vec3, Vec2> : public AnimAdaptorBase<Vec3, Vec2>
{
public:
   AnimAdaptor(AnimTarget<Vec2> *target) : AnimAdaptorBase<Vec3, Vec2>(target) {}

   // Specific conversions
   virtual void Set(const Vec3 &value) { m_target->Set(value.Drop());         }
   virtual Vec3 Get() const            { return m_target->Get().Lift(0.0f);   }
};

//! Adaptor that looks like a Vec4 target, but controls a Vec2 by droping z and w.
template <>
class AnimAdaptor<Vec4, Vec2> : public AnimAdaptorBase<Vec4, Vec2>
{
public:
   AnimAdaptor(AnimTarget<Vec2> *target) : AnimAdaptorBase<Vec4, Vec2>(target) {}

   // Specific conversions
   virtual void Set(const Vec4 &value) { m_target->Set(value.Drop().Drop());           }
   virtual Vec4 Get() const            { return m_target->Get().Lift(0.0f).Lift(0.0f); }
};

//! Adaptor that looks like a Vec4 target, but controls a Vec3 by dropping w
template <>
class AnimAdaptor<Vec4, Vec3> : public AnimAdaptorBase<Vec4, Vec3>
{
public:
   AnimAdaptor(AnimTarget<Vec3> *target) : AnimAdaptorBase<Vec4, Vec3>(target) {}

   // Specific conversions
   virtual void Set(const Vec4 &value) { m_target->Set(value.Drop());         }
   virtual Vec4 Get() const            { return m_target->Get().Lift(0.0f);   }
};

}

/** @} */

#endif /* __BSG_ANIMATABLE_H__ */

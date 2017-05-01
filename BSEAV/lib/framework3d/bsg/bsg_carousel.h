/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_CAROUSEL_H__
#define __BSG_CAROUSEL_H__

#include "bsg_common.h"
#include "bsg_scene_node.h"
#include "bsg_time.h"
#include "bsg_animator.h"
#include "bsg_animatable.h"
#include "bsg_animation_list.h"
#include "bsg_circular_index.h"

#include <vector>
#include <stdint.h>

namespace bsg
{

class CarouselWrapCallback
{
public:
   virtual ~CarouselWrapCallback() {}
   virtual void Prev(const SceneNodeHandle &geomNode) = 0;
   virtual void Next(const SceneNodeHandle &geomNode) = 0;
};

//! The Carousel object manages an abstract set of geometry as a carousel.
//! The number of physical carousel objects does not need to match the number of elements in the carousel menu.
//! The Carousel object will take cake of repeating elements where there are more physical objects that items,
//! and will correctly handle the case where there are more items than physical objects.
//!
//! The best way to use the Carousel, is to derive your own carousel object from it. In your constructor, you
//! you generate a bunch of empty scene nodes, with appropriate transforms, that will be the physical locations in the carousel.
//! These need to be added to the base carousel's root scene node list. e.g.
//! \code
//! using namespace bsg;
//! Transform       tran;
//! SceneNodeHandle newNode(New);
//! tran.SetPosition(Vec3(x, y, z);
//! tran.SetRotation(ang, Vec3(-1.0f, 0.0f, 0.0f));
//! newNode->SetTransform(tran);
//!
//! AddPhysicalNode(newNode);
//! \endcode
//!
//! When the carousel spins from one selection to the next, the transforms you have set on the scene nodes
//! are interpolated for all the nodes, and the carousel appears to spin. The start and end nodes are updated
//! appropriately to reset their transforms.
//!
//! As previously mentioned, the number of menu elements can (and likely will) be different to the physical objects.
//! You use the AddGeometry() or AddGeomNode() methods to add menu elements to the carousel. The geometries are held
//! in a list, and are set in the physical scene nodes appropriately as the carousel spins. The geometry list is
//! entirely independant of the scene nodes representing the physical locations. AddGeometry() can also be passed a scene
//! node handle so that you can add more complex objects than simple geometry.
//! Here's how you might add a simple menu item geomtry:
//! \code
//! TextureHandle texture(New);
//! texture->SetAutoMipmap(true);
//! texture->TexImage2D(Image(textureFile, Image::eRGB565));
//!
//! MaterialHandle mat(New);
//!
//! mat->SetEffect(m_effect);
//! mat->SetTexture("u_tex", texture);
//!
//! GeometryHandle geom = QuadFactory(1.0f, 1.0f, 0.0f, eZ_AXIS).MakeGeometry(mat);
//! AddGeometry(geom);
//! \endcode
class Carousel
{
public:
   //! Construct the base carousel.
   //! The fadeIn and fadeOut parameters control whether the first and last nodes of the Carousel
   //! will fade in or out as they animate. For the fade to work correctly, the Effect files used
   //! in the geometry need to support the u_opacity semantic.
   //!
   //! The carousel internally creates scene nodes for holding the positions of the carousel items,
   //! and to hold the geometry.  These internal nodes will be delete when the carousel is deleted.
   Carousel(bool fadeIn = true, bool fadeOut = true);
   virtual ~Carousel();

   //! Add a new element geometry to the list.
   //! There can be any number of geometries; they don't have to match the number of physical nodes.
   //! The carousel does not own the geometry and does not delete it.
   virtual void AddGeometry(GeometryHandle geom);

   //! Add new element geometry (as a node with hierarchy).
   //! There can be any number of geometries; they don't have to match the number of physical nodes.
   //! The carousel does not own geomNode and will not delete it.
   void AddGeometry(SceneNodeHandle geomNode);

   //! Return the handle to the root of the physical nodes (use to add the carousel to a graph,
   //! or for inspection of the graph)
   virtual const SceneNodeHandle RootNode();

   //! Create the physical node list from a container of transformations.
   //! If loop is true, will create clones of the end and begin nodes so that a cycle
   //! of objects works as expected.
   template <typename C>
   void SetLayout(const C &transforms, bool loop = false)
   {
      m_dirty = true;
      m_loop  = loop;
      EmptyRoot();

      if (loop)
      {
         const Transform   &trans  = *(transforms.end() - 1);
         SceneNodeHandle   endNode(New);

         endNode->SetTransform(trans);
         m_root->AppendChild(endNode);
      }

      for (typename C::const_iterator i = transforms.begin(); i != transforms.end(); ++i)
      {
         SceneNodeHandle   node(New);

         node->SetTransform(*i);
         m_root->AppendChild(node);
      }

      if (loop)
      {
         const Transform   &trans    = *transforms.begin();
         SceneNodeHandle   beginNode(New);

         beginNode->SetTransform(trans);
         m_root->AppendChild(beginNode);
      }
   }

   //! Specify which item should be considered the selected item.  By default this will be
   //! the middle item.  The index is zero based, so zero corresponds to the first geometry added
   //! to the carousel.
   //! Setting the selected item to -1 will restore the default behaviour.
   void SetSelectedItem(int32_t i)
   {
      m_select = i;
   }

   //! Animate the carousel moving to the next selection.
   //! The animation will run for animationTime, from now.
   //! Calling Next while an animation is active has no effect.
   virtual void Next(Time now, Time animationTime);

   //! Animate the carousel moving to the previous selection.
   //! The animation will run for animationTime, from now.
   //! Calling Prev while an animation is active has no effect.
   virtual void Prev(Time now, Time animationTime);

   //! Call during the Application frame update to update the carousel animations.
   virtual bool UpdateAnimationList(Time t);

   //! Returns true if the carousel is currently animating.
   //! The carousel cannot start a new animation whilst another is running.
   virtual bool IsAnimating() const { return m_animatingNow; }

   //! Return the index into the elements array of the currently selected menu item.
   virtual uint32_t CurrentIndex() const;

   //! Create the animation transition from one position to the next
   //! Override to change from default hermite behavior
   virtual AnimBindingEval<TransformEvaluator> *MakeTransition(AnimTarget<Transform> *target)
   {
      return new AnimBindingHermiteTransform(target);
   }

   void SetWrapCallback(CarouselWrapCallback *callback)
   {
      delete m_wrapCallback;
      m_wrapCallback = callback;
   }

protected:
   SceneNodeHandle ElementNode(uint32_t n);
   uint32_t        NumElements() const;
   virtual void    Prepare();

protected:
   SceneNodes              m_geomNodes;
   CircularIndex           m_startNodeIndex;
   CircularIndex           m_startGeomIndex;
   CircularIndex           m_endGeomIndex;
   CircularIndex           m_selGeomIndex;

   bool                    m_dirty;
   bool                    m_animatingNow;
   bool                    m_fadeIn;
   bool                    m_fadeOut;
   bool                    m_loop;

   int32_t                 m_select;

private:
   void EmptyRoot();
   void EmptyGeom();

private:
   SceneNodeHandle         m_root;
   AnimationList           m_animList;
   CarouselWrapCallback    *m_wrapCallback;
};

}

#endif /* __BSG_CAROUSEL_H__ */

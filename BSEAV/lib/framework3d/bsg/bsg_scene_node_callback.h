/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_SCENE_NODE_CALLBACK_H__
#define __BSG_SCENE_NODE_CALLBACK_H__

#include "bsg_common.h"
#include "bsg_no_copy.h"

namespace bsg
{

class SemanticData;
class Vec3;

//! Base class for callbacks invoked before rendering
class SceneNodeCallback
{
public:
   virtual ~SceneNodeCallback() {}

   //! This callback will be invoked during render packet construction, after the entire graph has been traversed.
   //! The intent is to allow last-minute adjustments to transforms for example, for advanced uses, such as
   //! screen-aligned glows, billboards etc.  Return false to suppress the item.
   virtual bool OnRenderData(SemanticData &semData) = 0;

   //! Use this callback to monitor the model transformation assigned to each node.
   //! Note that if the node is shared, the callback will be invoked multiple times
   //! during graph traversal.  The order is depth-first, left-to-right.
   virtual void OnModelMatrix(const Mat4 &xform)  = 0;

   //! Use this callback to monitor the model-view transformation assigned to each node.
   //! Note that if the node is shared, the callback will be invoked multiple times
   //! during graph traversal.  The order is depth-first, left-to-right.
   virtual void OnModelViewMatrix(const Mat4 &xform)  = 0;
};

//! Specialisation of SceneNodeCallback which leaves OnRenderData() to be filled in.
class CallbackOnRenderData : public SceneNodeCallback
{
   virtual void OnModelMatrix(const Mat4 &/*xform*/)      {}
   virtual void OnModelViewMatrix(const Mat4 &/*xform*/)  {}
};

//! Specialisation of SceneNodeCallback which leaves OnModelMatrix() to be filled in.
class CallbackOnModelMatrix : public SceneNodeCallback
{
   virtual bool OnRenderData(SemanticData &/*semData*/)    { return true; }
   virtual void OnModelViewMatrix(const Mat4 &/*xform*/)   {}
};

//! Specialisation of SceneNodeCallback which leaves OnModelViewMatrix() to be filled in.
class CallbackModelViewMatrix : public SceneNodeCallback
{
   virtual bool OnRenderData(SemanticData &/*semData*/)    { return true; }
   virtual void OnModelMatrix(const Mat4 &/*xform*/)       {}
};

//! This simple callback class updates the target with the position of the node.  Note that
//! if the node is shared, the callback will be invoked multiple times, so only the last
//! position will be recorded.
class PositionCallback : public CallbackOnModelMatrix
{
public:
   PositionCallback(Vec3 &target) :
      m_target(target)
   {}

   virtual void OnModelMatrix(const Mat4 &xform)
   {
      m_target = GetTranslation(xform);
   }

private:
   Vec3  &m_target;
};

// @cond
class SceneNodeCallbackList : public NoCopy
{
public:
   ~SceneNodeCallbackList()
   {
      for (std::list<SceneNodeCallback *>::iterator i = m_callbacks.begin(); i != m_callbacks.end(); ++i)
         delete *i;
   }

   void Add(SceneNodeCallback *callback)
   {
      m_callbacks.push_back(callback);
   }


   bool OnRenderData(SemanticData &semData) const
   {
      bool  render = true;

      for (std::list<SceneNodeCallback *>::const_iterator i = m_callbacks.begin(); i != m_callbacks.end(); ++i)
         render = render && (*i)->OnRenderData(semData);

      return render;
   }

   void OnModelMatrix(const Mat4 &xform) const
   {
      for (std::list<SceneNodeCallback *>::const_iterator i = m_callbacks.begin(); i != m_callbacks.end(); ++i)
         (*i)->OnModelMatrix(xform);
   }

   void OnModelViewMatrix(const Mat4 &xform) const
   {
      for (std::list<SceneNodeCallback *>::const_iterator i = m_callbacks.begin(); i != m_callbacks.end(); ++i)
         (*i)->OnModelViewMatrix(xform);
   }

   void Remove(SceneNodeCallback *callback)
   {
      std::list<SceneNodeCallback *>::iterator  i = std::find(m_callbacks.begin(), m_callbacks.end(), callback);

      if (i != m_callbacks.end())
      {
         delete *i;
         m_callbacks.erase(i);
      }
   }

   bool Empty() const
   {
      return m_callbacks.empty();
   }

private:
   std::list<SceneNodeCallback *>   m_callbacks;
};

// @endcond

}

#endif /* __BSG_RENDER_CALLBACK_H__ */

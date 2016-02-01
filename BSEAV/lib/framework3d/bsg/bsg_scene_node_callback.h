/******************************************************************************
 *   (c)2011-2012 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
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
      for (auto cb : m_callbacks)
         delete cb;
   }

   void Add(SceneNodeCallback *callback)
   {
      m_callbacks.push_back(callback);
   }


   bool OnRenderData(SemanticData &semData) const
   {
      bool  render = true;

      for (auto cb : m_callbacks)
         render = render && cb->OnRenderData(semData);

      return render;
   }

   void OnModelMatrix(const Mat4 &xform) const
   {
      for (auto cb : m_callbacks)
         cb->OnModelMatrix(xform);
   }

   void OnModelViewMatrix(const Mat4 &xform) const
   {
      for (auto cb : m_callbacks)
         cb->OnModelViewMatrix(xform);
   }

   void Remove(SceneNodeCallback *callback)
   {
      auto i = std::find(m_callbacks.begin(), m_callbacks.end(), callback);

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

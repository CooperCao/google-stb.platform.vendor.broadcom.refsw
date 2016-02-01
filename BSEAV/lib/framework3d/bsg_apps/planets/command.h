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
#ifndef __COMMAND_QUEUE_H__
#define __COMMAND_QUEUE_H__

#include <iostream>
#include <string>

#include "bsg_time.h"
#include "bsg_vector.h"
#include "bsg_scene_node.h"
#include "bsg_animator.h"
#include "bsg_animatable.h"
#include "bsg_font.h"

class Planets;
class Body;

///////////////////////////////////////////////////////////////////////////////////////////////////

class Command
{
public:
   virtual ~Command()  {}
   virtual void PreRenderInit()  = 0;
   virtual void PostRenderInit() = 0;
   virtual bool Finished()       = 0;
   virtual void OnFrame()        = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class CommandLookAt : public Command
{
public:
   CommandLookAt(bsg::Time time, const std::string &target, bsg::SceneNodeHandle cameraLookAtNode);
   CommandLookAt(std::istream &is, bsg::SceneNodeHandle cameraLookAtNode);
   ~CommandLookAt();

   virtual void PreRenderInit();
   virtual void PostRenderInit();
   virtual bool Finished();
   virtual void OnFrame() {}

private:
   void Init(bsg::Time time, const std::string &target, bsg::SceneNodeHandle cameraLookAtNode);

private:
   bsg::Time            m_duration;
   std::string          m_target;

   bsg::SceneNodeHandle m_targetNode;        // This is the thing we will be looking at
   bsg::SceneNodeHandle m_cameraLookAtNode;  // This is the animated node that moves to the target

   bsg::Vec3            m_lookAt;

   bsg::AnimBindingBase             *m_animation;
   bsg::AnimationStatusNotifier     m_notifier;
   bsg::PositionCallback            *m_callback;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class CommandGoto : public Command
{
public:
   CommandGoto(bsg::Time duration, const std::string &target, bsg::SceneNodeHandle cameraPosition, float distance, const bsg::Vec3 &offset);
   CommandGoto(std::istream &is, bsg::SceneNodeHandle gotoNode, bool offset = false);
   ~CommandGoto();

   virtual void PreRenderInit();
   virtual void PostRenderInit();
   virtual bool Finished();
   virtual void OnFrame() {}

private:
   void Init(bsg::Time duration, const std::string &target, bsg::SceneNodeHandle cameraPosition, float distance, const bsg::Vec3 &offset);

private:
   bsg::Time            m_duration;
   float                m_distance;
   std::string          m_target;
   bsg::Vec3            m_offset;

   bsg::SceneNodeHandle m_targetNode;           // This is the thing we will be moving to
   bsg::SceneNodeHandle m_cameraPositionNode;   // This is the animated node that moves to the target

   bsg::Vec3            m_goto;

   bsg::AnimBindingBase             *m_animation;
   bsg::AnimationStatusNotifier     m_notifier;
   bsg::PositionCallback            *m_callback;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class CommandMove : public Command
{
public:
   CommandMove(bsg::Time duration, bsg::SceneNodeHandle cameraPosition, const bsg::Vec3 &move, bool abs = false);
   CommandMove(std::istream &is, bsg::SceneNodeHandle cameraPositionNode, bool abs = false);
   ~CommandMove();

   virtual void PreRenderInit();
   virtual void PostRenderInit();
   virtual bool Finished();
   virtual void OnFrame() {}

private:
   void Init(bsg::Time duration, bsg::SceneNodeHandle cameraPosition, const bsg::Vec3 &move, bool abs);

private:
   bsg::Time   m_duration;
   bsg::Vec3   m_move;
   bool        m_abs;

   bsg::SceneNodeHandle    m_cameraPositionNode;

   bsg::AnimBindingBase          *m_animation;
   bsg::AnimationStatusNotifier  m_notifier;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class CommandRotate : public Command
{
public:
   CommandRotate(bsg::Time duration, bsg::SceneNodeHandle cameraLookAt, const bsg::Vec3 &rotate);
   CommandRotate(std::istream &is, bsg::SceneNodeHandle cameraLookAtNode);
   ~CommandRotate();

   virtual void PreRenderInit();
   virtual void PostRenderInit();
   virtual bool Finished();
   virtual void OnFrame();

private:
   void Init(bsg::Time duration, bsg::SceneNodeHandle cameraLookAt, const bsg::Vec3 &rotate);

private:
   bsg::Time   m_duration;
   bsg::Vec3   m_rotate;

   bsg::Vec3               m_camLook;
   bsg::Vec3               m_upVector;
   bsg::AnimatableVec3     m_angle;
   bsg::Mat4               m_viewToWorld;
   bsg::Mat4               m_worldToView;

   bsg::SceneNodeHandle    m_cameraLookAtNode;

   bsg::AnimBindingBase          *m_animation;
   bsg::AnimationStatusNotifier  m_notifier;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class CommandWait : public Command
{
public:
   CommandWait(bsg::Time duration);
   CommandWait(std::istream &wait);
   ~CommandWait();

   virtual void PreRenderInit();
   virtual void PostRenderInit();
   virtual bool Finished();
   virtual void OnFrame();

private:
   void Init(bsg::Time duration);

private:
   bsg::Time            m_duration;
   bsg::AnimatableFloat m_dummy;       // Dummy target for animation, just used for delay

   bsg::AnimBindingBase         *m_animation;
   bsg::AnimationStatusNotifier m_notifier;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class CommandSetRate : public Command
{
public:
   CommandSetRate(float rate);
   CommandSetRate(std::istream &is);
   ~CommandSetRate();

   virtual void PreRenderInit();
   virtual void PostRenderInit();
   virtual bool Finished();
   virtual void OnFrame();

private:
   void Init(float rate);

private:
   float m_rate;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class CommandSync : public Command
{
public:
   CommandSync(uint32_t *sync);
   CommandSync(std::istream &is, uint32_t *sync);
   ~CommandSync();

   virtual void PreRenderInit();
   virtual void PostRenderInit();
   virtual bool Finished();
   virtual void OnFrame();

private:
   void Init(uint32_t *sync);

private:
   uint32_t *m_sync;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class CommandInfo : public Command
{
public:
   CommandInfo(const std::string &bodyName, bool on);
   CommandInfo(std::istream &is);
   ~CommandInfo();

   virtual void PreRenderInit();
   virtual void PostRenderInit();
   virtual bool Finished();
   virtual void OnFrame();

private:
   void Init(const std::string &bodyName, bool on);

private:
   std::string m_bodyName;
   bool        m_on;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class CommandProxy
{
public:
   CommandProxy() :
      m_command(0)
   {}

   void operator=(Command *rhs)
   {
      delete m_command;
      m_command = rhs;
   }

   void PreRenderInit()
   {
      if (m_command != 0)
         m_command->PreRenderInit();
   }

   void OnFrame()
   {
      if (m_command != 0)
         m_command->OnFrame();
   }

   void PostRenderInit()
   {
      if (m_command != 0)
         m_command->PostRenderInit();
   }

   bool Finished()
   {
      if (m_command != 0)
         return m_command->Finished();

      return true;
   }

   void Delete()
   {
      if (m_command != 0)
         delete m_command;
   }


private:
   CommandProxy(const CommandProxy &rhs);
   CommandProxy &operator=(const CommandProxy &rhs);

private:
   Command  *m_command;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class CommandQueue
{
public:
   CommandQueue();
   ~CommandQueue();

   // Retrieve the next command.
   Command *Next(uint32_t ch = 0);

   void Add(Command *command, uint32_t channel = 0)
   {
      m_commands[channel].push_back(command);
   }

   bool Empty(uint32_t ch = 0) const
   {
      return m_commands[ch].empty();
   }

   void LoadScript(std::istream &is, Planets &app);

   void Clear();

private:
   std::list<Command *>  m_commands[2];
};

#endif

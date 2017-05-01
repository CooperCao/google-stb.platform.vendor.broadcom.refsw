/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "command.h"
#include "planets.h"
#include "body.h"
#include <sstream>

using namespace std;
using namespace bsg;

///////////////////////////////////////////////////////////////////////////////////////////////////

void CommandLookAt::Init(Time duration, const string &target, SceneNodeHandle cameraLookAtNode)
{
   m_cameraLookAtNode = cameraLookAtNode;
   m_animation        = 0;
   m_duration         = duration;
   m_target           = target;

   m_targetNode = SceneNodeHandle(m_target);
   if (m_targetNode.IsNull())
      BSG_THROW("Target object " << m_target << " not found");
}

CommandLookAt::CommandLookAt(bsg::Time time, const std::string &target, bsg::SceneNodeHandle cameraLookAtNode)
{
   Init(time, target, cameraLookAtNode);
}

CommandLookAt::CommandLookAt(istream &is, SceneNodeHandle cameraLookAtNode) :
   m_cameraLookAtNode(cameraLookAtNode),
   m_animation(0),
   m_callback(0)
{
   Time     duration;
   string   target;
   is >> duration >> target;

   Init(duration, target, cameraLookAtNode);
}

CommandLookAt::~CommandLookAt()
{
   m_targetNode->RemoveCallback(m_callback);
}

void CommandLookAt::PreRenderInit()
{
   m_callback = new PositionCallback(m_lookAt);
   m_targetNode->SetCallback(m_callback);
}

void CommandLookAt::PostRenderInit()
{
   Time  now  = Application::Instance()->FrameTimestamp();
   float mult = Application::Instance()->GetRateMultiplier();

   AnimatableVec3  &cameraLookAt  = m_cameraLookAtNode->GetPosition();

   AnimBindingDoubleHermiteVec3 *currentAnimation = new AnimBindingDoubleHermiteVec3(&cameraLookAt);
   m_animation = currentAnimation;
   currentAnimation->Interpolator()->Init(now, now + m_duration * mult, BaseInterpolator::eLIMIT, &m_notifier);
   currentAnimation->Evaluator()->Init(cameraLookAt, m_lookAt);

   Planets::Instance()->AddAnimation(currentAnimation);
}

bool CommandLookAt::Finished()
{
   return m_notifier.HasBeenNotified();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void CommandGoto::Init(Time duration, const string &target, SceneNodeHandle cameraPosition, float distance, const Vec3 &offset)
{
   m_duration           = duration;
   m_target             = target;
   m_cameraPositionNode = cameraPosition;
   m_distance           = distance;
   m_offset             = offset;
   m_animation          = 0;
   m_callback           = 0;

   m_targetNode = SceneNodeHandle(m_target);
   if (m_targetNode.IsNull())
      BSG_THROW("Target object " << m_target << " not found");
}

CommandGoto::CommandGoto(Time duration, const string &target, SceneNodeHandle cameraPosition, float distance, const Vec3 &offset)
{
   Init(duration, target, cameraPosition, distance, offset);
}

CommandGoto::CommandGoto(istream &is, SceneNodeHandle cameraPositionNode, bool useOffset)
{
   Time     duration;
   string   target;
   float    distance;
   Vec3     offset;

   is >> duration >> target >> distance;
   if (useOffset)
      is >> offset.X() >> offset.Y() >> offset.Z();

   Init(duration, target, cameraPositionNode, distance, offset);
}

CommandGoto::~CommandGoto()
{
   m_targetNode->RemoveCallback(m_callback);
}

void CommandGoto::PreRenderInit()
{
   m_callback = new PositionCallback(m_goto);
   m_targetNode->SetCallback(m_callback);
}

void CommandGoto::PostRenderInit()
{
   Time  now  = Application::Instance()->FrameTimestamp();
   float mult = Application::Instance()->GetRateMultiplier();

   // m_goto now has the position of the target object -- we don't want to hit it, so adjust back by specified distance
   AnimatableVec3 &start = m_cameraPositionNode->GetPosition();
   Vec3           delta  = m_goto + m_offset - start;
   float          alpha  = 1.0f - m_distance / Length(delta);

   // Update new target
   m_goto = start + delta * alpha;

   AnimBindingDoubleHermiteVec3 *currentAnimation = new AnimBindingDoubleHermiteVec3(&start);
   m_animation = currentAnimation;
   currentAnimation->Interpolator()->Init(now, now + m_duration * mult, BaseInterpolator::eLIMIT, &m_notifier);
   currentAnimation->Evaluator()->Init(start, m_goto);

   Planets::Instance()->AddAnimation(currentAnimation);
}

bool CommandGoto::Finished()
{
   return m_notifier.HasBeenNotified();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void CommandMove::Init(Time duration, SceneNodeHandle cameraPosition, const Vec3 &move, bool abs)
{
   m_duration           = duration;
   m_cameraPositionNode = cameraPosition;
   m_move               = move;
   m_abs                = abs;
   m_animation          = 0;
}

CommandMove::CommandMove(Time duration, SceneNodeHandle cameraPosition, const Vec3 &move, bool abs)
{
   Init(duration, cameraPosition, move, abs);
}

CommandMove::CommandMove(istream &is, SceneNodeHandle cameraPositionNode, bool abs) :
   m_cameraPositionNode(cameraPositionNode),
   m_animation(0)
{
   Time  duration;
   Vec3  move;

   is >> duration >> move.X() >> move.Y() >> move.Z();

   Init(duration, cameraPositionNode, move, abs);
}

CommandMove::~CommandMove()
{
}

void CommandMove::PreRenderInit()
{
}

void CommandMove::PostRenderInit()
{
   Time  now  = Application::Instance()->FrameTimestamp();
   float mult = Application::Instance()->GetRateMultiplier();

   // m_goto now has the position of the target object -- we don't want to hit it, so adjust back by specified distance
   AnimatableVec3 &start = m_cameraPositionNode->GetPosition();
   Vec3           delta  = m_abs ? m_move : Planets::Instance()->GetWorldToView().Drop() * m_move;

   AnimBindingHermiteVec3 *currentAnimation = new AnimBindingHermiteVec3(&start);
   m_animation = currentAnimation;
   currentAnimation->Interpolator()->Init(now, now + m_duration * mult, BaseInterpolator::eLIMIT, &m_notifier);
   currentAnimation->Evaluator()->Init(start, start + delta);

   Planets::Instance()->AddAnimation(currentAnimation);
}

bool CommandMove::Finished()
{
   return m_notifier.HasBeenNotified();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void CommandRotate::Init(Time duration, SceneNodeHandle cameraLookAt, const Vec3 &rotate)
{
   m_duration         = duration;
   m_cameraLookAtNode = cameraLookAt;
   m_rotate           = rotate;
   m_animation        = 0;
}

CommandRotate::CommandRotate(Time duration, SceneNodeHandle cameraLookAt, const Vec3 &rotate)
{
   Init(duration, cameraLookAt, rotate);
}

CommandRotate::CommandRotate(istream &is, SceneNodeHandle cameraLookAtNode) :
   m_cameraLookAtNode(cameraLookAtNode),
   m_animation(0)
{
   Time  duration;
   Vec3  rotate;

   is >> duration >> rotate.X() >> rotate.Y() >> rotate.Z();

   Init(duration, cameraLookAtNode, rotate);
}

CommandRotate::~CommandRotate()
{
}

void CommandRotate::PreRenderInit()
{
   m_upVector = Planets::Instance()->GetUpVector();
   m_camLook  = m_cameraLookAtNode->GetPosition();
}

void CommandRotate::OnFrame()
{
   if (m_animation != 0)
   {
      Vec4  camLook = m_worldToView *
                      Rotate(-m_angle.X(), 1.0f, 0.0f, 0.0f) *
                      Rotate( m_angle.Y(), 0.0f, 1.0f, 0.0f) *  m_viewToWorld * m_camLook.Lift(1.0f);

      Vec3  upVector = m_worldToView.Drop() * Rotate( m_angle.Z(), 0.0f, 0.0f, 1.0f).Drop() * m_viewToWorld .Drop() * m_upVector;

      m_cameraLookAtNode->SetPosition(camLook.Drop());
      Planets::Instance()->GetUpVector() = upVector;
   }
}

void CommandRotate::PostRenderInit()
{
   Time  now  = Application::Instance()->FrameTimestamp();
   float mult = Application::Instance()->GetRateMultiplier();

   // Capture the current camera transform
   m_viewToWorld = Planets::Instance()->GetViewToWorld();
   m_worldToView = Planets::Instance()->GetWorldToView();

   AnimBindingHermiteVec3 *currentAnimation = new AnimBindingHermiteVec3(&m_angle);
   m_animation = currentAnimation;
   currentAnimation->Interpolator()->Init(now, now + m_duration * mult, BaseInterpolator::eLIMIT, &m_notifier);
   currentAnimation->Evaluator()->Init(0.0f, m_rotate);

   Planets::Instance()->AddAnimation(currentAnimation);
}

bool CommandRotate::Finished()
{
   return m_notifier.HasBeenNotified();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void CommandWait::Init(Time duration)
{
   m_duration  = duration;
   m_animation = 0;
   m_dummy.Set(0.0f);
}

CommandWait::CommandWait(Time duration)
{
   Init(duration);
}

CommandWait::CommandWait(istream &is)
{
   Time  duration;

   is >> duration;

   Init(duration);
}

CommandWait::~CommandWait()
{
}

void CommandWait::PreRenderInit()
{
}

void CommandWait::PostRenderInit()
{
   Time  now  = Application::Instance()->FrameTimestamp();
   float mult = Application::Instance()->GetRateMultiplier();

   AnimBindingLerpFloat *currentAnimation = new AnimBindingLerpFloat(&m_dummy);
   m_animation = currentAnimation;
   currentAnimation->Interpolator()->Init(now, now + m_duration * mult, BaseInterpolator::eLIMIT, &m_notifier);
   currentAnimation->Evaluator()->Init(0.0f, 1.0f);

   Planets::Instance()->AddAnimation(currentAnimation);
}

bool CommandWait::Finished()
{
   return m_notifier.HasBeenNotified();
}

void CommandWait::OnFrame()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void CommandSetRate::Init(float rate)
{
   m_rate = rate;
}

CommandSetRate::CommandSetRate(float rate)
{
   Init(rate);
}

CommandSetRate::CommandSetRate(istream &is)
{
   float rate;
   is >> rate;
   Init(rate);
}

CommandSetRate::~CommandSetRate()
{
}

void CommandSetRate::PreRenderInit()
{
   Application::Instance()->SetRateMultiplier(m_rate);
}

void CommandSetRate::PostRenderInit()
{
}

bool CommandSetRate::Finished()
{
   return true;
}

void CommandSetRate::OnFrame()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void CommandSync::Init(uint32_t *sync)
{
   m_sync = sync;
}

CommandSync::CommandSync(uint32_t *sync)
{
   Init(sync);
}

CommandSync::CommandSync(istream &/* is */, uint32_t *sync)
{
   Init(sync);
}

CommandSync::~CommandSync()
{
   *m_sync -= 1;
}

void CommandSync::PreRenderInit()
{
   *m_sync += 1;
}

void CommandSync::PostRenderInit()
{
}

bool CommandSync::Finished()
{
   return *m_sync == 2;
}

void CommandSync::OnFrame()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void CommandInfo::Init(const string &bodyName, bool on)
{
   m_bodyName = bodyName;
   m_on       = on;
}

CommandInfo::CommandInfo(const string &bodyName, bool status)
{
   Init(bodyName, status);
}

CommandInfo::CommandInfo(istream &is)
{
   uint32_t status;
   string   bodyName;

   is >> status >> bodyName;

   Init(bodyName, status != 0);
}

CommandInfo::~CommandInfo()
{
}

void CommandInfo::PreRenderInit()
{
   Planets::Instance()->ShowInfo(m_bodyName, m_on);
}

void CommandInfo::PostRenderInit()
{
}

bool CommandInfo::Finished()
{
   return true;
}

void CommandInfo::OnFrame()
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////

CommandQueue::CommandQueue()
{
}

CommandQueue::~CommandQueue()
{
   Clear();
}

Command *CommandQueue::Next(uint32_t ch)
{
   if (m_commands[ch].empty())
      return 0;

   Command *command = m_commands[ch].front();
   m_commands[ch].pop_front();

   return command;
}

void CommandQueue::Clear()
{
   for (uint32_t ch = 0; ch < 2; ++ch)
   {
      for (list<Command *>::iterator i = m_commands[ch].begin(); i != m_commands[ch].end(); ++i)
         delete *i;

      m_commands[ch].clear();
   }
}

void CommandQueue::LoadScript(istream &is, Planets &app)
{
   string   line;

   while (is.good())
   {
      Command  *command = 0;
      uint32_t channel;

      getline(is, line);

      stringstream   ss(line);
      string         dummy;
      string         token;

      ss >> channel;
      ss >> dummy;
      ss >> token;

      if (token == "lookAt")
         command = new CommandLookAt(ss, app.GetCameraLookAt());
      else if (token == "goto")
         command = new CommandGoto(ss, app.GetCameraPosition());
      else if (token == "goto_offset")
         command = new CommandGoto(ss, app.GetCameraPosition(), true);
      else if (token == "wait")
         command = new CommandWait(ss);
      else if (token == "move")
         command = new CommandMove(ss, app.GetCameraPosition());
      else if (token == "move_abs")
         command = new CommandMove(ss, app.GetCameraPosition(), true);
      else if (token == "rotate")
         command = new CommandRotate(ss, app.GetCameraLookAt());
      else if (token == "setrate")
         command = new CommandSetRate(ss);
      else if (token == "sync")
         command = new CommandSync(app.GetSync());
      else if (token == "info")
         command = new CommandInfo(ss);

      if (command != 0 && (channel == 0 || channel == 1))
         m_commands[channel].push_back(command);
   }
}

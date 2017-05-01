/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_MOUSE_EVENTS_H__
#define __BSG_MOUSE_EVENTS_H__

#include "bsg_time.h"
#include "bsg_vector.h"
#include "bsg_task.h"
#include <list>

namespace bsg
{

//! Encapsulates the data about a key-press, either on the IR remote, or keyboard when simulated or emulated.
class MouseEvent
{
public:
   enum eMouseEventType
   {
      eMOUSE_EVENT_UNKNOWN = 0,
      eMOUSE_EVENT_BUTTON,
      eMOUSE_EVENT_MOVE,
      eMOUSE_EVENT_WHEEL,
      eMOUSE_EVENT_ACCEL,
      eMOUSE_EVENT_ORIENTATION
   };
   enum eMouseButtonState
   {
      eMOUSE_STATE_UNKNOWN = 0,
      eMOUSE_STATE_DOWN,
      eMOUSE_STATE_UP
   };

   enum eMouseButtonCode
   {
      eBTN_NONE = 0,

      // Mouse / Joystick events
      eBTN_0,
      eBTN_1,
      eBTN_2,
      eBTN_3,
      eBTN_4,
      eBTN_5,
      eBTN_6,
      eBTN_7,
      eBTN_8,
      eBTN_9,
      eBTN_LEFT,
      eBTN_RIGHT,
      eBTN_MIDDLE,
      eBTN_SIDE,
      eBTN_EXTRA,
      eBTN_FORWARD,
      eBTN_BACK,
      eBTN_TASK,
      eBTN_TRIGGER,
      eBTN_THUMB,
      eBTN_THUMB2,
      eBTN_TOP,
      eBTN_TOP2,
      eBTN_PINKIE,
      eBTN_BASE,
      eBTN_BASE2,
      eBTN_BASE3,
      eBTN_BASE4,
      eBTN_BASE5,
      eBTN_BASE6,
      eBTN_DEAD,
      eBTN_A,
      eBTN_B,
      eBTN_C,
      eBTN_X,
      eBTN_Y,
      eBTN_Z,
      eBTN_TL,
      eBTN_TR,
      eBTN_TL2,
      eBTN_TR2,
      eBTN_SELECT,
      eBTN_START,
      eBTN_MODE,
      eBTN_THUMBL,
      eBTN_THUMBR,
      eBTN_TOOL_PEN,
      eBTN_TOOL_RUBBER,
      eBTN_TOOL_BRUSH,
      eBTN_TOOL_PENCIL,
      eBTN_TOOL_AIRBRUSH,
      eBTN_TOOL_FINGER,
      eBTN_TOOL_MOUSE,
      eBTN_TOOL_LENS,
      eBTN_TOUCH,
      eBTN_STYLUS,
      eBTN_STYLUS2,
      eBTN_TOOL_DOUBLETAP,
      eBTN_TOOL_TRIPLETAP,
      eBTN_GEAR_UP,

      eBTN_LAST_BTN
   };

   MouseEvent() :
      m_type(eMOUSE_EVENT_UNKNOWN), m_code(eBTN_NONE), m_state(eMOUSE_STATE_UNKNOWN), m_timestamp(0.0f) {}

   MouseEvent(eMouseButtonCode code, eMouseButtonState state, const IVec3 &absPos, Time t) :
      m_type(eMOUSE_EVENT_BUTTON), m_code(code), m_state(state), m_absPos(absPos), m_timestamp(t) {}

   MouseEvent(const IVec3 &rel, const IVec3 &absPos, Time t) :
      m_type(eMOUSE_EVENT_MOVE), m_code(eBTN_NONE), m_state(eMOUSE_STATE_UNKNOWN),
      m_relData(rel), m_absPos(absPos), m_timestamp(t) {}

   MouseEvent(const IVec2 &wheelRel, const IVec3 &absPos, Time t) :
      m_type(eMOUSE_EVENT_WHEEL), m_code(eBTN_NONE), m_state(eMOUSE_STATE_UNKNOWN),
      m_relData(wheelRel.X(), wheelRel.Y(), 0), m_absPos(absPos), m_timestamp(t) {}

   MouseEvent(const eMouseEventType type, const IVec3 &accel, Time t) :
      m_type(type), m_code(eBTN_NONE), m_state(eMOUSE_STATE_UNKNOWN),
      m_relData(accel), m_timestamp(t) {}

   MouseEvent(const eMouseEventType type, const Vec3 &orientation, Time t) :
      m_type(type), m_code(eBTN_NONE), m_state(eMOUSE_STATE_UNKNOWN),
      m_relDataOrientation(orientation), m_timestamp(t) {}

   //! Return the type of the event
   eMouseEventType Type() const { return m_type; }

   //! Return the BtnCode of the event (only valid if type == eMOUSE_EVENT_BUTTON)
   eMouseButtonCode Code() const { return m_code; }

   //! Return the BtnState of the event (only valid if type == eMOUSE_EVENT_BUTTON)
   eMouseButtonState State() const { return m_state; }

   //! Return the absolute mouse position at the time of the event
   IVec3 AbsolutePosition() const { return m_absPos; }

   //! Return the relative move (only valid if type == eMOUSE_EVENT_MOVE)
   IVec3 RelativeMoveVector() const { return m_relData; }

   //! Return the relative wheel data (only valid if type == eMOUSE_EVENT_WHEEL)
   IVec2 RelativeWheelVector() const { return IVec2(m_relData.X(), m_relData.Y()); }

   IVec3 AccelerationVector() const { return m_relData; }

   Vec3 OrientationVector() const { return m_relDataOrientation; }

   //! Return the timestamp when the press occurred
   Time GetTimestamp() const { return m_timestamp; }

private:
   eMouseEventType   m_type;
   eMouseButtonCode  m_code;
   eMouseButtonState m_state;
   IVec3             m_relData;     // For move or wheel data, depending on type
   IVec3             m_absPos;      // Location of mouse at event time
   Vec3              m_relDataOrientation;
   Time              m_timestamp;
};

//! A FIFO of MouseEvent elements
class MouseEvents : public Queue<MouseEvent>
{
};

}

#endif /* __BSG_KEY_EVENTS_H__ */

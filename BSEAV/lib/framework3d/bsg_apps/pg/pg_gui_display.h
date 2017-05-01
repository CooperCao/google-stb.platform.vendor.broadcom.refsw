/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __GUIDisplay_H__
#define __GUIDisplay_H__

#include <vector>

#include "bsg_scene_node.h"
#include "bsg_animator.h"

namespace pg
{

   typedef std::vector<bsg::AnimBindingBase *> VectorOfAnim;

   class GUIDisplay
   {
   public:
      // Default constructor
      GUIDisplay(void);
      // Destructor
      ~GUIDisplay(void);
      // Set the Scene Graph Node used to control the GUI
      void Init(const bsg::SceneNodeHandle &guiNode);
      // Create an animation rotating the GUI
      VectorOfAnim Rotate(bsg::Time now, bsg::AnimationDoneNotifier *notifier);
      // Returns true if the GUI is rotated
      bool IsRotated() const { return m_isRotated; };
      // Remove the rotation of the GUI without creating an animation
      void ResetRotation();
      // Rotate the GUI without creating an animation
      void Rotate();

   private:
      // Scene Graph Node used to control the GUI
      bsg::SceneNodeHandle    m_guiNode;
      // flag indicating if the GUI is rotated
      bool                    m_isRotated;
   };

}
#endif // __GUIDisplay_H__

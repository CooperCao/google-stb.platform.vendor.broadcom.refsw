/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "menu_carousel.h"
#include "bsg_application.h"
#include "bsg_image_png.h"
#include "bsg_animator.h"
#include "bsg_math.h"

using namespace std;
using namespace bsg;

MenuCarousel::MenuCarousel(float radius, float selScale, const vector<SceneNodeHandle> &geomNodes) :
   Carousel(false, false)
{
   uint32_t numNodes = geomNodes.size();

   if (numNodes < 1)
      BSG_THROW("MenuCarousel must have at least one node");

   Circle   circle(radius, 0.0f, eZ_AXIS);
   float    step = 1.0f / numNodes;

   std::vector<Transform>  positions;

   // Build the carousel nodes the way we want them
   for (uint32_t n = 0; n < numNodes; ++n)
   {
      Vec3        pos(circle.Evaluate(n * step));
      Transform   trans;

      trans.SetPosition(pos);

      // Highlight selected object by scaling it up
      if (n == 0)
      {
         trans.PostTranslate(pos);
         trans.SetScale(Vec3(selScale));
      }

      // Adds the position for the item
      positions.push_back(trans);

      // Adds the node to the geometry list (will appear at position specified by physicalNode)
      AddGeometry(geomNodes[n]);
   }

   // Add the nodes as a loop i.e. last will move to first, first to last.
   SetLayout(positions, true);
   SetSelectedItem(0);
}

MenuCarousel::~MenuCarousel()
{
}

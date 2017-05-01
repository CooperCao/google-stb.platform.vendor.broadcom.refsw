/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <iostream>
#include <vector>
#include "bsg_exception.h"
#include "bsg_scene_node.h"
#include "bsg_application.h"

class PlanetEffects;

class StarInfo
{
public:
   StarInfo(std::istream &is)
   {
      if (!is.good())
         BSG_THROW("Invalid stream\n");

      is >> m_ra;
      is >> m_dec;
      is >> m_mag;
      is >> m_class;
   }

   float GetRA()                 const { return m_ra;    }
   float GetDec()                const { return m_dec;   }
   float GetMag()                const { return m_mag;   }
   const std::string &GetClass() const { return m_class; }

private:
   float       m_ra;
   float       m_dec;
   float       m_mag;
   std::string m_class;
};

class Stars
{
public:
   Stars(std::istream &is)
   {
      uint32_t numStars;

      if (!is.good())
         BSG_THROW("Invalid stream\n");

      is >> numStars;
      is >> m_magOffset;
      is >> m_magScale;

      m_colors.resize(7);

      for (uint32_t c = 0; c < 7; ++c)
      {
         is >> m_colors[c].X();
         is >> m_colors[c].Y();
         is >> m_colors[c].Z();
      }

      m_stars.reserve(numStars);

      for (uint32_t n = 0; n < numStars; ++n)
         m_stars.push_back(StarInfo(is));
   }

   bsg::SceneNodeHandle CreateGeometry(const PlanetEffects &effects) const;

   float GetMagOffset() const { return m_magOffset; }
   float GetMagScale()  const { return m_magScale;  }

private:
   bsg::Vec3 ColorOf(const std::string &starClass) const;

private:
   float                   m_magOffset;
   float                   m_magScale;
   std::vector<StarInfo>   m_stars;
   std::vector<bsg::Vec3>  m_colors;
};

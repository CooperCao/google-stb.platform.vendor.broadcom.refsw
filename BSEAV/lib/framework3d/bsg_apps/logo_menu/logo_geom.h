/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __LOGO_GEOM_H__
#define __LOGO_GEOM_H__

#include "bsg_geometry.h"
#include "bsg_material.h"

class BroadcomLogo
{
public:
   bsg::GeometryHandle InstanceLogo(bsg::MaterialHandle textMat, bsg::MaterialHandle pulseMat, bool reflected = false);

private:
   bsg::SurfaceHandle m_textSurfHandle;
   bsg::SurfaceHandle m_pulseSurfHandle;
};


#endif /* __LOGO_GEOM_H__ */

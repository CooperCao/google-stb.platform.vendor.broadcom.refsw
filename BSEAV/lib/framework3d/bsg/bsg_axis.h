/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_AXIS_H__
#define __BSG_AXIS_H__

#include "bsg_common.h"

namespace bsg
{

//! @addtogroup math
//! @{

//! Names for major axes used in BSG.
enum eAxis
{

   X_AXIS,   //!< \deprecated Use eX_AXIS
   Y_AXIS,   //!< \deprecated Use eY_AXIS
   Z_AXIS,   //!< \deprecated Use eZ_AXIS

   eX_AXIS = X_AXIS,
   eY_AXIS = Y_AXIS,
   eZ_AXIS = Z_AXIS
};

//! @}

//! Names for winding orders used in BSG.
enum eWinding
{

   CW_WINDING,   //!< \deprecated Use eCW_WINDING
   CCW_WINDING,  //!< \deprecated Use eCCW_WINDING

   eCW_WINDING  = CW_WINDING,
   eCCW_WINDING = CCW_WINDING
};

}

#endif /* __BSG_AXIS_H__ */

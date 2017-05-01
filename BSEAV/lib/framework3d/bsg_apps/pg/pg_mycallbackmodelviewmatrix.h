/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __PG_MYCALLBACKMODELVIEWMATRIX_H__
#define __PG_MYCALLBACKMODELVIEWMATRIX_H__

#include "bsg_scene_node.h"
#include "bsg_matrix.h"

namespace pg
{
   class LongScrolling;

   class MyCallbackModelViewMatrix : public bsg::CallbackModelViewMatrix
   {
   public:
      virtual void OnModelViewMatrix(const bsg::Mat4 &xform);
      void SetLongScrolling(LongScrolling *pLongScrolling);

   private:
      LongScrolling *m_pLongScrolling;
   };
}

#endif // __PG_MYCALLBACKMODELVIEWMATRIX_H__

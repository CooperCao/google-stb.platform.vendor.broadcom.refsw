/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "pg_mycallbackmodelviewmatrix.h"

#include "pg_longscrolling.h"

using namespace pg;

void MyCallbackModelViewMatrix::OnModelViewMatrix(const bsg::Mat4 &xform)
{
      if (m_pLongScrolling != NULL)
      {
         m_pLongScrolling->SetProgramsNodeWorlMat(xform);
      }
}

void MyCallbackModelViewMatrix::SetLongScrolling(LongScrolling *pLongScrolling)
{
   m_pLongScrolling = pLongScrolling;
}

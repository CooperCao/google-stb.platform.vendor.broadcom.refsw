/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __PG_DUMMYDATA_H__
#define __PG_DUMMYDATA_H__

#include "pg_info.h"

namespace pg
{

class DummyData
{
public:
   DummyData();

   void InitGridInfo(GridInfo *info);
};

}

#endif /* __PG_DUMMYDATA_H__ */

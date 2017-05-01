/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_NO_COPY_H__
#define __BSG_NO_COPY_H__

#include "bsg_common.h"
#include "bsg_compiler_quirks.h"

namespace bsg
{

//! This class is not copyable either by constructor or by assignment.
//! (but see the BSG_NOCOPY macro commentary on older gcc compilers
class NoCopy
{
public:
   NoCopy() {}

   BSG_NO_COPY(NoCopy)
};

}

#endif

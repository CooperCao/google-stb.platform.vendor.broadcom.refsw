/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#pragma once

#include <stddef.h>

class DataSink
{
public:
   virtual size_t Write(const void *data, size_t size) = 0;
   virtual bool Flush() = 0;
   virtual ~DataSink() {};
};

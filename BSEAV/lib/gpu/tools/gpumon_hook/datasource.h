/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#pragma once

#include <stddef.h>

class DataSource
{
public:
   virtual size_t Read(void *data, size_t size) = 0;
   virtual ~DataSource() {};
};

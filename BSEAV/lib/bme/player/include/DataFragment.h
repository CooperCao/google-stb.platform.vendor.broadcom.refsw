/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/

#ifndef BME_DATAFRAGMENT_H_
#define BME_DATAFRAGMENT_H_

namespace Broadcom
{
namespace Media
{

struct DataFragment_t
{
    const void *data;
    size_t      bytes;
    bool        encrypted;

    DataFragment_t()
    {
    }
    DataFragment_t(const void *data, size_t bytes, bool encrypted = false)
    : data(data), bytes(bytes), encrypted(encrypted)
    {
    }
};

}
}


#endif // ifndef BME_DATAFRAGMENT_H_
/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef BSG_STAND_ALONE

#ifndef __BSG_FONTS_H__
#define __BSG_FONTS_H__

#include "bsg_common.h"

#include <stdint.h>
#include <string>

// @cond
namespace bsg
{

class FontMemInfo
{
public:
   FontMemInfo(const unsigned char *address, uint32_t size, const std::string &/*fontName*/) :
      m_address(address),
      m_size(size)
   {}

   const unsigned char  *GetAddress()  const  { return m_address; }
   uint32_t             GetSize()      const  { return m_size;    }
   const std::string    &GetName()     const  { return m_name;    }

private:
   const unsigned char  *m_address;
   uint32_t             m_size;
   const std::string    m_name;
};

class FontMem
{
public:
   static const FontMemInfo &GetInfo(const std::string &fontName);
};

}

// @endcond

#endif

#endif /* BSG_STAND_ALONE */

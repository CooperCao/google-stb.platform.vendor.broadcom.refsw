/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_DITHER_H__
#define __BSG_DITHER_H__

#include "bsg_common.h"

namespace bsg
{

//! Convert an RGBA8888 value encoded as an unsigned int into a 16-bit 565 value
unsigned short DitherRgb565(unsigned char r8, unsigned char g8, unsigned char b8, unsigned int x, unsigned int y);
//! Convert an RGBA8888 value encoded as an unsigned int into a 16-bit 5551 value
unsigned short DitherRgb5551(unsigned char r8, unsigned char g8, unsigned char b8, unsigned char a8, unsigned int x, unsigned int y);
//! Convert an RGBA8888 value encoded as an unsigned int into a 16-bit 4444 value
unsigned short DitherRgb4444(unsigned char r8, unsigned char g8, unsigned char b8, unsigned char a8, unsigned int x, unsigned int y);

}

#endif /* __BSG_DITHER_H__ */

/******************************************************************************
 *   Broadcom Proprietary and Confidential. (c)2011-2012 Broadcom.  All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
 * AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
 * SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE
 * ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 * ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#include "bsg_image_pkm.h"
#include "bsg_exception.h"
#include "bsg_trackers.h"

#include <string.h>

#include <string>
#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>

using namespace std;

#ifdef EMULATED
#include "etc.h"
#endif

namespace bsg
{

#ifdef BIG_ENDIAN_CPU
#define BSWAP32(a) (a)
#else

#if defined(_MSC_VER)
#define BSWAP32 _byteswap_ulong
#elif defined(__GNUC__)
#if __GNUC__ > 4 || (__GNUC__ == 4 &&  __GNUC_MINOR__ > 2)
#define BSWAP32 __builtin_bswap32
#else
#define BSWAP32 static_bswap32

static unsigned int static_bswap32(unsigned int b)
{
   return (b & 0x000000ff) << 24 |
          (b & 0x0000ff00) << 8  |
          (b & 0x00ff0000) >> 8  |
          (b & 0xff000000) >> 24;
}
#endif
#else
#endif

#endif /* BIG_ENDIAN_CPU */

// The ETC2 package includes the following codecs:
//
// codec                                             enum
// --------------------------------------------------------
// GL_COMPRESSED_R11_EAC                            0x9270
// GL_COMPRESSED_SIGNED_R11_EAC                     0x9271
// GL_COMPRESSED_RG11_EAC                           0x9272
// GL_COMPRESSED_SIGNED_RG11_EAC                    0x9273
// GL_COMPRESSED_RGB8_ETC2                          0x9274
// GL_COMPRESSED_SRGB8_ETC2                         0x9275
// GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2      0x9276
// GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2     0x9277
// GL_COMPRESSED_RGBA8_ETC2_EAC                     0x9278
// GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC              0x9279
//
// The older codec ETC1 is not included in the package
// GL_ETC1_RGB8_OES                                 0x8d64
// but since ETC2 is backwards compatible an ETC1 texture can
// be decoded using the RGB8_ETC2 enum (0x9274)
//
// In a PKM-file, the codecs are stored using the following identifiers
//
// identifier                         value               codec
// --------------------------------------------------------------------
// ETC1_RGB_NO_MIPMAPS                  0                 GL_ETC1_RGB8_OES
// ETC2PACKAGE_RGB_NO_MIPMAPS           1                 GL_COMPRESSED_RGB8_ETC2
// ETC2PACKAGE_RGBA_NO_MIPMAPS_OLD      2, not used       -
// ETC2PACKAGE_RGBA_NO_MIPMAPS          3                 GL_COMPRESSED_RGBA8_ETC2_EAC
// ETC2PACKAGE_RGBA1_NO_MIPMAPS         4                 GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2
// ETC2PACKAGE_R_NO_MIPMAPS             5                 GL_COMPRESSED_R11_EAC
// ETC2PACKAGE_RG_NO_MIPMAPS            6                 GL_COMPRESSED_RG11_EAC
// ETC2PACKAGE_R_SIGNED_NO_MIPMAPS      7                 GL_COMPRESSED_SIGNED_R11_EAC
// ETC2PACKAGE_RG_SIGNED_NO_MIPMAPS     8                 GL_COMPRESSED_SIGNED_RG11_EAC
// ETC2PACKAGE_SRGB_SIGNED_NO_MIPMAPS   9                 GL_COMPRESSED_SRGB8_ETC2
// ETC2PACKAGE_SRGBA_SIGNED_NO_MIPMAPS  10                GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC
// ETC2PACKAGE_SRGBA1_SIGNED_NO_MIPMAPS 11                GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2

const uint32_t ETC1_RGB_NO_MIPMAPS              = 0;
const uint32_t ETC2PACKAGE_RGB_NO_MIPMAPS       = 1;
const uint32_t ETC2PACKAGE_RGBA_NO_MIPMAPS_OLD  = 2;
const uint32_t ETC2PACKAGE_RGBA_NO_MIPMAPS      = 3;
const uint32_t ETC2PACKAGE_RGBA1_NO_MIPMAPS     = 4;
const uint32_t ETC2PACKAGE_R_NO_MIPMAPS         = 5;
const uint32_t ETC2PACKAGE_RG_NO_MIPMAPS        = 6;
const uint32_t ETC2PACKAGE_R_SIGNED_NO_MIPMAPS  = 7;
const uint32_t ETC2PACKAGE_RG_SIGNED_NO_MIPMAPS = 8;
const uint32_t ETC2PACKAGE_SRGB_NO_MIPMAPS      = 9;
const uint32_t ETC2PACKAGE_SRGBA_NO_MIPMAPS     = 10;
const uint32_t ETC2PACKAGE_SRGBA1_NO_MIPMAPS    = 11;

ImagePKM::ImagePKM(const string &fileName)
{
   /////////////////////
   // PROCESS THE HEADER
   /////////////////////
   FILE  *fp = fopen(fileName.c_str(), "rb");

   if (fp == NULL)
      BSG_THROW("Cannot open file " << fileName);

   // Mark fp for closure
   FClose   closeIt(fp);

   // Check magic number
   char           magic[4];

   if (fread(magic, sizeof(unsigned char), 4, fp) != 4)
      BSG_THROW("Couldn't read header " << fileName);

   if (strncmp(magic, "PKM ", 4) != 0)
      BSG_THROW("File " << fileName << " is not an ETC/PKM file");

   // Check version
   char  version[2];

   if (fread(version, sizeof(unsigned char), 2, fp) != 2)
      BSG_THROW("Couldn't read version");

#ifndef BSG_USE_ES3
   if (version[0] > '1')
      BSG_THROW("File version 1.0 expected but found " << version[0] << "." << version[1]);
#else
   if (version[0] > '2')
      BSG_THROW("File version 1.0 or 2.0 expected but found " << version[0] << "." << version[1]);
#endif

   // Check texture format
   uint32_t format;

   if (fread(&format, sizeof(unsigned char), 2, fp) != 2)
      BSG_THROW("Couldn't read format");
   format  = BSWAP32(format) >> 16;

   uint32_t bpp = 4;

#ifndef BSG_USE_ES3
   m_internalFormat = GL_ETC1_RGB8_OES;
   if (format != ETC1_RGB_NO_MIPMAPS)
      BSG_THROW("Format of texture not ETC1_RGB_NO_MIPMAPS");
#else
   switch (format)
   {
   case ETC1_RGB_NO_MIPMAPS              :
   case ETC2PACKAGE_RGB_NO_MIPMAPS       : m_internalFormat = GL_COMPRESSED_RGB8_ETC2; break;
   case ETC2PACKAGE_RGBA_NO_MIPMAPS      : m_internalFormat = GL_COMPRESSED_RGBA8_ETC2_EAC; bpp = 8; break;
   case ETC2PACKAGE_RGBA1_NO_MIPMAPS     : m_internalFormat = GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2; break;
   case ETC2PACKAGE_R_NO_MIPMAPS         : m_internalFormat = GL_COMPRESSED_R11_EAC; break;
   case ETC2PACKAGE_R_SIGNED_NO_MIPMAPS  : m_internalFormat = GL_COMPRESSED_SIGNED_R11_EAC; break;
   case ETC2PACKAGE_RG_NO_MIPMAPS        : m_internalFormat = GL_COMPRESSED_RG11_EAC; bpp = 8; break;
   case ETC2PACKAGE_RG_SIGNED_NO_MIPMAPS : m_internalFormat = GL_COMPRESSED_SIGNED_RG11_EAC; bpp = 8; break;
   case ETC2PACKAGE_SRGB_NO_MIPMAPS      : m_internalFormat = GL_COMPRESSED_SRGB8_ETC2; break;
   case ETC2PACKAGE_SRGBA_NO_MIPMAPS     : m_internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC; bpp = 8; break;
   case ETC2PACKAGE_SRGBA1_NO_MIPMAPS    : m_internalFormat = GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2; break;
   default                               :
   case ETC2PACKAGE_RGBA_NO_MIPMAPS_OLD  : BSG_THROW("Unknown ETC2 texture format"); break;
   }
#endif

   /////////////////////
   // READ THE PIXELS
   /////////////////////

   uint32_t width;
   uint32_t height;

   if (fread(&width, sizeof(uint8_t), 2, fp) != 2)
      BSG_THROW("Couldn't read width");
   width = BSWAP32(width) >> 16;

   if (fread(&height, sizeof(uint8_t), 2, fp) != 2)
      BSG_THROW("Couldn't read height");
   height = BSWAP32(height) >> 16;

   if (fread(&m_width, sizeof(uint8_t), 2, fp) != 2)
      BSG_THROW("Couldn't read width");
   m_width = BSWAP32(m_width) >> 16;

   if (fread(&m_height, sizeof(uint8_t), 2, fp) != 2)
      BSG_THROW("Couldn't read height");
   m_height = BSWAP32(m_height) >> 16;

   /* allocate enough for bpp */
   m_size = (width * height * bpp) / 8;
   m_data = (uint8_t *)malloc(m_size);
   if (m_data == 0)
      BSG_THROW("Out of memory");

   if (fread(m_data, sizeof(uint8_t), m_size, fp) != m_size)
      BSG_THROW("Couldn't read pixels");

#ifdef EMULATED
   // TODO : ETC2 uncompress

   // Emulator doesn't support ETC textures, so decompress
   uint8_t *compData = (uint8_t*)m_data;
   uint8_t *buffer = (uint8_t*)malloc(width * height * 3);
   m_data = (uint8_t *)malloc(m_width * m_height * 3);

   etc_uncompress_image_to_buffer(compData, width, height, m_width, m_height, buffer, (uint8_t*)m_data);

   m_type = Image::Type(Image::eRGB888);

   free(buffer);
   free(compData);
#endif
}

ImagePKM::~ImagePKM()
{
   if (m_data != 0)
      free(m_data);
}

GLenum ImagePKM::GetInternalFormat() const
{
#ifdef EMULATED
   return GL_RGB;
#else
   return m_internalFormat;
#endif
}

bool ImagePKM::IsCompressed() const
{
#ifdef EMULATED
   return false;
#else
   return true;
#endif
}

}

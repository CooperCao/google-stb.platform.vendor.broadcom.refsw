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

#include "bsg_image_astc.h"
#include "bsg_exception.h"
#include "bsg_trackers.h"

#include <string>
#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>

using namespace std;

namespace bsg
{
#ifdef GL_KHR_texture_compression_astc_hdr

#define MAGIC_FILE_CONSTANT   0x5CA1AB13
#define BLOCK_SIZE            16   /* Blocks are always stored in 128 bits */

bool ImageASTC::SetInternalFormat(uint8_t blockdim_x, uint8_t blockdim_y, bool nonlinearValues)
{
   bool format_found = false;

   switch (blockdim_x)
   {
   case 4:
      if (blockdim_y == 4)
      {
         m_internalFormat = GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
         format_found = true;
      }
      break;
   case 5:
      if (blockdim_y == 4)
      {
         m_internalFormat = GL_COMPRESSED_RGBA_ASTC_5x4_KHR;
         format_found = true;
      }
      else if (blockdim_y == 5)
      {
         m_internalFormat = GL_COMPRESSED_RGBA_ASTC_5x5_KHR;
         format_found = true;
      }
      break;
   case 6:
      if (blockdim_y == 5)
      {
         m_internalFormat = GL_COMPRESSED_RGBA_ASTC_6x5_KHR;
         format_found = true;
      }
      else if (blockdim_y == 6)
      {
         m_internalFormat = GL_COMPRESSED_RGBA_ASTC_6x6_KHR;
         format_found = true;
      }
      break;
   case 8:
      if (blockdim_y == 5)
      {
         m_internalFormat = GL_COMPRESSED_RGBA_ASTC_8x5_KHR;
         format_found = true;
      }
      else if (blockdim_y == 6)
      {
         m_internalFormat = GL_COMPRESSED_RGBA_ASTC_8x6_KHR;
         format_found = true;
      }
      else if (blockdim_y == 8)
      {
         m_internalFormat = GL_COMPRESSED_RGBA_ASTC_8x8_KHR;
         format_found = true;
      }
      break;
   case 10:
      if (blockdim_y == 5)
      {
         m_internalFormat = GL_COMPRESSED_RGBA_ASTC_10x5_KHR;
         format_found = true;
      }
      else if (blockdim_y == 6)
      {
         m_internalFormat = GL_COMPRESSED_RGBA_ASTC_10x6_KHR;
         format_found = true;
      }
      else if (blockdim_y == 8)
      {
         m_internalFormat = GL_COMPRESSED_RGBA_ASTC_10x8_KHR;
         format_found = true;
      }
      else if (blockdim_y == 10)
      {
         m_internalFormat = GL_COMPRESSED_RGBA_ASTC_10x10_KHR;
         format_found = true;
      }
         break;
   case 12:
     if (blockdim_y == 10)
     {
        m_internalFormat = GL_COMPRESSED_RGBA_ASTC_12x10_KHR;
        format_found = true;
     }
     else if (blockdim_y == 12)
     {
        m_internalFormat = GL_COMPRESSED_RGBA_ASTC_12x12_KHR;
        format_found = true;
     }
     break;
   }

   // "The constants for the sRGB formats are always 0x0020
   // greater than the RGBA constants"
   if (format_found && nonlinearValues)
      m_internalFormat += 0x20;

   return format_found;
}

ImageASTC::ImageASTC(const string &fileName)
{
   /////////////////////
   // PROCESS THE HEADER
   /////////////////////
   FILE  *fp = fopen(fileName.c_str(), "rb");

   bool nonlinearValues = false;
   if (fileName.find("sRGBA") != string::npos)
      nonlinearValues = true;

   if (fp == NULL)
      BSG_THROW("Cannot open file " << fileName);

   // Mark fp for closure
   FClose         closeIt(fp);

   // Check magic number
   uint8_t        magic[] = {0,0,0,0};

   if (fread(magic, sizeof(unsigned char), 4, fp) != 4)
      BSG_THROW("Couldn't read header " << fileName);

   uint32_t magicval = magic[0] + 256 * (uint32_t) (magic[1]) + 65536 * (uint32_t) (magic[2]) + 16777216 * (uint32_t) (magic[3]);

   if (magicval != MAGIC_FILE_CONSTANT)
      BSG_THROW("File " << fileName << " is not an ASTC file");

   /////////////////////
   // READ THE PIXELS
   /////////////////////

   uint8_t blockdim_x = 0;
   uint8_t blockdim_y = 0;
   uint8_t blockdim_z = 0;

   if (fread(&blockdim_x, sizeof(uint8_t), 1, fp) != 1)
      BSG_THROW("Couldn't read blockdim x");

   if (fread(&blockdim_y, sizeof(uint8_t), 1, fp) != 1)
      BSG_THROW("Couldn't read blockdim y");

   if (fread(&blockdim_z, sizeof(uint8_t), 1, fp) != 1)
      BSG_THROW("Couldn't read blockdim z");

   if (blockdim_x < 3 || blockdim_x > 12 || blockdim_y < 3 || blockdim_y > 12 || (blockdim_z < 3 && blockdim_z != 1) || blockdim_z > 12)
      BSG_THROW("File " << fileName << " not recognized " << blockdim_x << " " << blockdim_y << " " << blockdim_z);

   if (blockdim_z != 1)
      BSG_THROW("3D ASTC texture is not supported");

   if (!SetInternalFormat(blockdim_x, blockdim_y, nonlinearValues))
      BSG_THROW("Texture format not supported");

   uint8_t xsize[] = {0,0,0};       // x-size = xsize[0] + xsize[1] + xsize[2]
   uint8_t ysize[] = {0,0,0};       // x-size, y-size and z-size are given in texels;
   uint8_t zsize[] = {0,0,0};       // block count is inferred

   if (fread(xsize, sizeof(uint8_t), 3, fp) != 3)
      BSG_THROW("Couldn't read x size");

   if (fread(ysize, sizeof(uint8_t), 3, fp) != 3)
      BSG_THROW("Couldn't read y size");

   if (fread(zsize, sizeof(uint8_t), 3, fp) != 3)
      BSG_THROW("Couldn't read z size");

   // Image size in texels
   m_width  = xsize[0] + 256 * xsize[1] + 65536 * xsize[2];
   m_height = ysize[0] + 256 * ysize[1] + 65536 * ysize[2];
   m_depth  = zsize[0] + 256 * zsize[1] + 65536 * zsize[2];

   // Number of blocks in each dimension
   int xblocks = (m_width    + blockdim_x - 1) / blockdim_x;
   int yblocks = (m_height   + blockdim_y - 1) / blockdim_y;
   int zblocks = (m_depth    + blockdim_z - 1) / blockdim_z;

   // Maximum image size
   m_size = xblocks * yblocks * zblocks * BLOCK_SIZE;
   m_data = (uint8_t *) malloc(m_size);
   if (m_data == 0)
      BSG_THROW("Out of memory");

   // Read the data from the file
   size_t bytes_read = fread(m_data, 1, m_size, fp);
   if (bytes_read != m_size)
      BSG_THROW("Couldn't read pixels, file " << fileName);

}

#else

ImageASTC::ImageASTC(const string &fileName)
{
   // Dummy function
   BSG_THROW("Unsupported extension 'astc' for image");
}

#endif

ImageASTC::~ImageASTC()
{
   if (m_data != 0)
      free(m_data);
}

GLenum ImageASTC::GetInternalFormat() const
{
   return m_internalFormat;
}

bool ImageASTC::IsCompressed() const
{
   return true;
}

}

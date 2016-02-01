/******************************************************************************
 *   (c)2011-2012 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
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

#include "bsg_dither.h"
#include <assert.h>

namespace bsg
{
   unsigned short DitherRgb565(unsigned char r8, unsigned char g8, unsigned char b8, 
                               unsigned int x, unsigned int y)
   {
      static const int dither_matrix[16] =
      {
         0, 4, 1, 5,
         6, 2, 7, 3,
         1, 5, 0, 4,
         7, 3, 6, 2
      };

      static const int dither_matrix_5[16] =
      {
         0,  8,  2, 10,
         4, 12,  6, 14,
         1,  9,  3, 11,
         5, 13,  7, 15
      };

      unsigned int index;
      int dither, dither_5;
      int r, g, b;

      index = (x & 3) + ((y & 3)<<2);

      dither = dither_matrix[index];
      dither_5 = dither_matrix_5[index];

      r = r8;
      g = g8;
      b = b8;

      r = (r<<1) | 1;   /* 255.5 */
      g = (g<<1) | 1;
      b = (b<<1) | 1;

      r = r - (r >> 5) + (dither_5 | 1);
      g = g - (g >> 6) + (dither   | 1);
      b = b - (b >> 5) + (dither_5 | 1);

      r = r >> 4;
      g = g >> 3;
      b = b >> 4;

      assert(r >= 0 && g >= 0 && b >= 0 && r < 32 && g < 64 && b < 32);
      return r << 11 | g << 5 | b;
   }


   unsigned short DitherRgb5551(unsigned char r8, unsigned char g8, unsigned char b8, unsigned char a8, 
                                unsigned int x, unsigned int y)
   {
      static const int dither_matrix[16] =
      {
         0,  8,  2, 10,
         4, 12,  6, 14,
         1,  9,  3, 11,
         5, 13,  7, 15
      };

      unsigned int index;
      int dither;
      int r, g, b, a;

      index = (x & 3) + ((y & 3)<<2);

      dither = dither_matrix[index];

      a = a8;
      r = r8;
      g = g8;
      b = b8;

      a = (a != 0);
      r = (r<<1) | 1;
      g = (g<<1) | 1;
      b = (b<<1) | 1;

      r = r - (r >> 5) + (dither | 1);
      g = g - (g >> 5) + (dither | 1);
      b = b - (b >> 5) + (dither | 1);

      r = r >> 4;
      g = g >> 4;
      b = b >> 4;

      assert(r >= 0 && g >= 0 && b >= 0 && r < 32 && g < 32 && b < 32);
      return r << 11 | g << 6 | b << 1 | a;
   }


   unsigned short DitherRgb4444(unsigned char r8, unsigned char g8, unsigned char b8, unsigned char a8, 
                                unsigned int x, unsigned int y)
   {
      static const int dither_matrix[64] =
      {
         0, 16,  4, 20,  1, 17,  5, 21,
         8, 24, 12, 28,  9, 25, 13, 29,
         2, 18,  6, 22,  3, 19,  7, 23,
         10, 26, 14, 30, 11, 27, 15, 31,
         1, 17,  3, 19,  0, 16,  2, 18,
         9, 25, 11, 27,  8, 24, 10, 26,
         5, 21,  7, 23,  4, 20,  6, 22,
         13, 29, 15, 31, 12, 28, 14, 30,
      };

      unsigned int index;
      int dither;
      int r, g, b, a;

      index = (x & 7) + ((y & 7)<<3);

      dither = dither_matrix[index];

      a = a8;
      r = r8;
      g = g8;
      b = b8;

      a = (a<<1) | 1;
      r = (r<<1) | 1;
      g = (g<<1) | 1;
      b = (b<<1) | 1;

      a = a - (a >> 4) + (dither | 1);
      r = r - (r >> 4) + (dither | 1);
      g = g - (g >> 4) + (dither | 1);
      b = b - (b >> 4) + (dither | 1);

      a = a >> 5;
      r = r >> 5;
      g = g >> 5;
      b = b >> 5;

      assert(r >= 0 && g >= 0 && b >= 0 && a >= 0 && r < 16 && g < 16 && b < 16 && a < 16);
      return r << 12 | g << 8 | b << 4 | a;
   }

}


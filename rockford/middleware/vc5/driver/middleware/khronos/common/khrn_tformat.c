/*=============================================================================
Copyright (c) 2005 Broadcom Ltd. All rights reserved.

Project  :  VideoCore III 3D Renderer
Module   :  Reference Renderer

FILE DESCRIPTION
Contains translation routine to convert input x,y
(in u-tiles) into u-tile offset within image, given
correctly padded width (also in u-tiles) and type
(either t-format or 'linear tile' (lt-format).

=============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"

#include "middleware/khronos/common/khrn_tformat.h"

/*****************************************************************************
   Public functions
******************************************************************************/

/*
   Calculates offset to u-tile given u-tile x,y, width
   and type of image buffer (linear-tile or t-format)
*/

unsigned int khrn_tformat_utile_addr
   (int ut_w, int ut_x, int ut_y, int is_tformat, TAddr_t *taddr)
{

   unsigned int wt, xt, yt, xst, yst, xut, yut;
   unsigned int tile_addr, sub_tile_addr, utile_addr, addr;
   int line_odd = 0;

   if (is_tformat) { // T-FORMAT

      // first check that ut_w and ut_h conform to
      // exact multiples of tiles
      if(ut_w&0x7)
         UNREACHABLE(); /* ut_w not padded to integer # tiles! */

      // find width and height of image in tiles
      wt = ut_w>>3;

      // find address of tile given ut_x,ut_y
      // remembering tiles are 8x8 u-tiles,
      // also remembering scan direction changes
      // for odd and even lines
      xt = ut_x>>3; yt = ut_y>>3;
      if(yt&0x1) {
         xt = wt-xt-1; // if odd line, reverse direction
         line_odd = 1;
      }
      tile_addr = yt*wt + xt;

      // now find address of sub-tile within tile (2 bits)
      // remembering scan order is not raster scan, and
      // changes direction given odd/even tile-scan-line.
      xst = (ut_x>>2)&0x1;
      yst = (ut_y>>2)&0x1;
      switch(xst+(yst<<1)){
       case 0:  sub_tile_addr = line_odd ? 2 : 0; break;
       case 1:  sub_tile_addr = line_odd ? 1 : 3; break;
       case 2:  sub_tile_addr = line_odd ? 3 : 1; break;
       default: sub_tile_addr = line_odd ? 0 : 2; break;
      }

      // now find address of u-tile within sub-tile
      // nb 4x4 u-tiles fit in one sub-tile, and are in
      // raster-scan order.
      xut = ut_x&0x3;
      yut = ut_y&0x3;
      utile_addr = (yut<<2)|xut;

      addr = utile_addr + (sub_tile_addr<<4) + (tile_addr<<6);

#ifndef NDEBUG
      if(taddr) { /* for debug */
         taddr->utile_addr    = utile_addr;
         taddr->sub_tile_addr = sub_tile_addr;
         taddr->tile_addr     = tile_addr;
         taddr->addr          = addr;
      }
#else
      UNUSED(taddr);
#endif
   } else { // LT-FORMAT
      addr = ut_y*ut_w + ut_x;
   }

   return addr;
}

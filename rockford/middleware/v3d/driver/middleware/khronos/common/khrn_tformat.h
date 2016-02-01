/*=============================================================================
Copyright (c) 2005 Broadcom Ltd. All rights reserved.

Project  :  VideoCore III 3D Renderer
Module   :  Reference Renderer

FILE DESCRIPTION

T/LT format address calculator.

=============================================================================*/

#ifndef KHRN_TFORMAT_H
#define KHRN_TFORMAT_H

typedef struct {
   unsigned int tile_addr;
   unsigned int sub_tile_addr;
   unsigned int utile_addr;
   unsigned int addr;
} TAddr_t;

unsigned int khrn_tformat_utile_addr
   (int ut_w, int ut_x, int ut_y, int is_tformat, TAddr_t *taddr);

#endif
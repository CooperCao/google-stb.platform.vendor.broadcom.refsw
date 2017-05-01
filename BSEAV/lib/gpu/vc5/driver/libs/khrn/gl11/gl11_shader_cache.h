/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef GL11_SHADER_CACHE_H
#define GL11_SHADER_CACHE_H

#include "../glsl/glsl_ir_program.h"

#include "../glxx/glxx_shader_cache.h"

/*
mmlt-Dncllllllllssssssssllllllll   vertex
caaallllffuuf-----------------ss   fragment
ooosssssscccoooooosssssscccxxrce   texture
*/

/* fragment (1.1) */
#define GL11_POINTSMOOTH (1<<0) /* points */
#define GL11_LINESMOOTH (1<<1) /* lines */
#define GL11_FLATSHADE  (1<<19)
#define GL11_UCLIP_NONE (0<<20)
#define GL11_UCLIP_A    (1<<20)
#define GL11_UCLIP_B    (2<<20)
#define GL11_UCLIP_M    (3<<20)
#define GL11_FOG_NONE   (0<<22)
#define GL11_FOG_LINEAR (1<<22)
#define GL11_FOG_EXP    (2<<22)
#define GL11_FOG_EXP2   (3<<22)
#define GL11_FOG_M      (3<<22)
#define GL11_LOGIC_CLEAR          (3<<24)
#define GL11_LOGIC_AND            (2<<24)
#define GL11_LOGIC_AND_REVERSE    (1<<24)
#define GL11_LOGIC_COPY           (0<<24) /* what we get when logic is disabled */
#define GL11_LOGIC_AND_INVERTED   (7<<24)
#define GL11_LOGIC_NOOP           (6<<24)
#define GL11_LOGIC_XOR            (5<<24)
#define GL11_LOGIC_OR             (4<<24)
#define GL11_LOGIC_NOR            (11<<24)
#define GL11_LOGIC_EQUIV          (10<<24)
#define GL11_LOGIC_INVERT         (9<<24)
#define GL11_LOGIC_OR_REVERSE     (8<<24)
#define GL11_LOGIC_COPY_INVERTED  (15<<24)
#define GL11_LOGIC_OR_INVERTED    (14<<24)
#define GL11_LOGIC_NAND           (13<<24)
#define GL11_LOGIC_SET            (12<<24)
#define GL11_LOGIC_M              (15<<24)
#define GL11_AFUNC_NEVER    (7<<28)
#define GL11_AFUNC_LESS     (6<<28)
#define GL11_AFUNC_EQUAL    (5<<28)
#define GL11_AFUNC_LEQUAL   (4<<28)
#define GL11_AFUNC_GREATER  (3<<28)
#define GL11_AFUNC_NOTEQUAL (2<<28)
#define GL11_AFUNC_GEQUAL   (1<<28)
#define GL11_AFUNC_ALWAYS   (0<<28) /* what we get when afunc is disabled */
#define GL11_AFUNC_M (7<<28)
/* TODO: This isn't implemented. It should be emulated by GL1 using
 * alpha blending functions, setting the mode to GL_ONE
 */
#define GL11_SAMPLE_ONE      (1<<31)

/* vertex */
#define GL11_LIGHT_ENABLE 1 /* <<n */
#define GL11_LIGHT_SPOT 0x100 /* <<n */
#define GL11_LIGHT_LOCAL 0x10000 /* <<n */
#define GL11_LIGHT_M 0x10101
#define GL11_LIGHT_ENABLES_M 0xff
#define GL11_LIGHTS_M 0xffffff
#define GL11_COLORMAT   (1<<24)
#define GL11_NO_NORMALIZE (1<<25) /* no normalize or rescale_normal */
#define GL11_DRAW_TEX   (1<<26)
#define GL11_TWOSIDE    (1<<28)
#define GL11_LIGHTING   (1<<29)
#define GL11_LIGHTING_M (GL11_LIGHTS_M|GL11_COLORMAT|GL11_NO_NORMALIZE|GL11_TWOSIDE|GL11_LIGHTING)
#define GL11_MPAL_M     (3<<30)  /* No. of matrix palette vertex units */
#define GL11_MPAL_S     30

/* texture */
//1+1+1+max(3+3+6+6+6+3+2, 3+1+1)

#define GL11_TEX_COMBINE_M 0xfffffff8
#define GL11_TEX_ENABLE       (1<<0)
#define GL11_TEX_COMPLEX      (1<<1)
#define GL11_TEX_COORDREPLACE (1<<2)
#define GL11_TEX_CSCALE   (1<<3)
#define GL11_TEX_ASCALE   (1<<4)
#define GL11_TEX_CC_S    5
#define GL11_TEX_CC_M    (7<<5)
#define GL11_TEX_CS_S    8 //+2n
#define GL11_TEX_CO_S    14 //+2n
#define GL11_TEX_AC_S    20
#define GL11_TEX_AC_M    (7<<20)
#define GL11_TEX_AS_S    23 //+2n
#define GL11_TEX_AO_S    29 //+n
/* shifted values for texture */
#define GL11_COMB_REP 0
#define GL11_COMB_MOD 1
#define GL11_COMB_ADD 2
#define GL11_COMB_ADS 3
#define GL11_COMB_INT 4
#define GL11_COMB_SUB 5
#define GL11_COMB_DOT 6
#define GL11_COMB_DOTA 7
#define GL11_COMB_M 7
#define GL11_SRC_x    0  /* dummy value */
#define GL11_SRC_S 0
#define GL11_SRC_K 1
#define GL11_SRC_F 2
#define GL11_SRC_P 3
#define GL11_SRC_M 3
#define GL11_OP_x    0  /* dummy value */
#define GL11_OP_A 0
#define GL11_OP_AX 1
#define GL11_OP_C 2
#define GL11_OP_CX 3
#define GL11_OPC_M 3
#define GL11_OPA_M 1

#define GL11_TEX_STRING(cc,cs0,co0,cs1,co1,cs2,co2,ac,as0,ao0,as1,ao1,as2,ao2) (GL11_TEX_ENABLE|\
         GL11_COMB_##cc << GL11_TEX_CC_S |\
         GL11_SRC_##cs0 << GL11_TEX_CS_S |\
         GL11_SRC_##cs1 << (GL11_TEX_CS_S+2) |\
         GL11_SRC_##cs2 << (GL11_TEX_CS_S+4) |\
         GL11_OP_##co0 << GL11_TEX_CO_S |\
         GL11_OP_##co1 << (GL11_TEX_CO_S+2) |\
         GL11_OP_##co2 << (GL11_TEX_CO_S+4) |\
         GL11_COMB_##ac << GL11_TEX_AC_S |\
         GL11_SRC_##as0 << GL11_TEX_AS_S |\
         GL11_SRC_##as1 << (GL11_TEX_AS_S+2) |\
         GL11_SRC_##as2 << (GL11_TEX_AS_S+4) |\
         GL11_OP_##ao0 << GL11_TEX_AO_S |\
         GL11_OP_##ao1 << (GL11_TEX_AO_S+1) |\
         GL11_OP_##ao2 << (GL11_TEX_AO_S+2))


typedef struct
{
   uint32_t vertex;
   uint32_t fragment;
   uint32_t texture[4];
   bool points;
} GL11_CACHE_KEY_T;

typedef struct {
   bool                used;
   GL11_CACHE_KEY_T    key;
   IR_PROGRAM_T       *blob;
   GLXX_BINARY_CACHE_T backend_cache;
} GL11_CACHE_ENTRY_T;

#define GL11_CACHE_SIZE 1024

#endif

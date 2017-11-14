/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 ***************************************************************************/

#ifndef BPXL_UIF_H__
#define BPXL_UIF_H__

#ifdef __cplusplus
extern "C" {
#endif


/*Size of u-tile in bytes*/
#define BPXL_UIF_TILESIZE                      64
/* Size of UIF-block in bytes  */
#define BPXL_UIF_BLOCKSIZE                     256
/* Width of UIF column in UIF-blocks */
#define BPXL_UIF_COL_WIDTH_IN_BLOCK            4
#define BPXL_UIF_MAX_MIP_LEVELS                 15


/* Valid for VC6 (7278) and later only, MM M2MC is not available in earlier
 * SoCs containing VC5 */
#define BPXL_UIF_V3D_TMU_ML_ALIGN                 64


/* Valid for 32bit color formats only, in this case a "block" is a single pixel */
#define BPXL_UIF_UT_W_IN_BLOCKS_2D   4u
#define BPXL_UIF_UT_H_IN_BLOCKS_2D   4u
#define BPXL_UIF_UB_W_IN_BLOCKS_2D   (BPXL_UIF_UT_W_IN_BLOCKS_2D * 2)
#define BPXL_UIF_UB_H_IN_BLOCKS_2D   (BPXL_UIF_UT_H_IN_BLOCKS_2D * 2)
#define BPXL_UIF_UCOL_W_IN_BLOCKS_2D (BPXL_UIF_UB_W_IN_BLOCKS_2D * BPXL_UIF_COL_WIDTH_IN_BLOCK)

/* Enforce a minimum width and height in order to:
 * - avoid having to deal with small sizes that would normally trigger
 *	 ub-linear or linear-tile sub-formats, both when using the standard
 *	 M2MC or the mipmap variant for non-mipmapped surfaces.
 * - avoid some UIF corner cases when the width is between 17 and 32
 *	 pixels, where the padding calculated by BPXL_Uif_SurfaceCfg while
 *	 not wrong is not what the V3D drvier ends up using for textures.
 */
#define BPXL_UIF_MINSURFACE          (BPXL_UIF_UCOL_W_IN_BLOCKS_2D*2)

/* need to mapping different value to grc according to the register definition */
typedef enum BPXL_Uif_Swizzling
{
    BPXL_Uif_Swizzling_eUIF       = 0,  /* Unified image format, no XORing */
    BPXL_Uif_Swizzling_eUIF_XOR   = 1,  /* XOR in odd columns */
    BPXL_Uif_Swizzling_eUBLINEAR2 = 2,  /* UIF-blocks in raster order 2 UIF per column*/
    BPXL_Uif_Swizzling_eUBLINEAR1 = 3,  /* UIF-blocks in raster order, 1UIF per column */
    BPXL_Uif_Swizzling_eLT        = 4,  /* Lineartile */
    BPXL_Uif_Swizzling_eMax
} BPXL_Uif_Swizzling;

typedef struct BPXL_Uif_Surface{
    uint32_t  ulL0ffset;
    uint32_t  ulL0Pitch;
    BPXL_Uif_Swizzling  eL0Swizzling;
    uint32_t  ulSize;
    uint32_t  ulAlign;
    uint32_t  ulPadH;
    uint32_t  ulWidth;       /* in pixels*/
    uint32_t  ulHeight;      /* in pixels*/
    uint32_t  ulMipLevel;
}BPXL_Uif_Surface;

typedef struct BPXL_Uif_Memory_Info{
    uint32_t ulPageSize;
    uint32_t ulPageinUBRows;
    uint32_t ul15PageinUBRows;
    uint32_t ulPcInUBRows;
    uint32_t ulPc15InUBRows;
}BPXL_Uif_Memory_Info;


void BPXL_Uif_SurfaceCfg(
    BPXL_Uif_Memory_Info *pMemoryInfo,
    BPXL_Uif_Surface *pSurface);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BGRC_PACKET_PRIV_H__ */

/* end of file */

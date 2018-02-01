/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/



#include "bstd.h"
#include "bkni.h"
#include "bpxl_uif.h"

BDBG_MODULE(BPXL_Uif);




/* utility function*/
static unsigned BPXL_Uif_P_msb(uint32_t x)
{
    BDBG_ASSERT(x);
    return 31 - __builtin_clz(x);
}

static bool BPXL_Uif_P_is_power_of_2(uint32_t x)
{
    BDBG_ASSERT(x);
    return ((x & (x - 1)) == 0);
}

static uint32_t BPXL_Uif_P_next_power_of_2(uint32_t x)
{
    if (BPXL_Uif_P_is_power_of_2(x))
        return x;
    if (x)
        return 1 << (BPXL_Uif_P_msb(x) + 1);
    return 1;
}
static uint32_t BPXL_Uif_P_udiv_round_up(uint32_t x, uint32_t y)
{
    BDBG_ASSERT(y);
    return ((x - 1) / y) + 1;
}

static uint32_t BPXL_Uif_P_uround_up(uint32_t x, uint32_t y)
{
    return BPXL_Uif_P_udiv_round_up(x, y) * y;
}
/* end of utility function*/

/* Get the min / max of two numbers */
#define BPXL_P_MAX(a, b)        (((a) > (b)) ? (a) : (b))
#define BPXL_P_ALIGN_UP(value, alignment) \
    (((uint32_t)(value) + (alignment) - 1) & ~((alignment) - 1))

/* helper to calculate the dimension of a miplevel from the base image size,
 * the miplevel dimensions are clamped to a minimum value of 1
 */
static uint32_t BPXL_Uif_P_mipsize(uint32_t base_size, int mip_level)
{
   return BPXL_P_MAX(base_size >> mip_level, 1);
}

static void BPXL_Uif_P_GetSwizzling(
   uint32_t ulW,       /* in blocks*/
   uint32_t ulH,       /* in blocks*/
   BPXL_Uif_Swizzling *peSwizzling)
{
    BDBG_ASSERT(peSwizzling);

    /*
    For format UIF_ARGB:
    if width of image is less than or equal to 8 pixels
       layout => should be UBLINEAR1
    else if width of image is less than or equal to 16 pixels
       layout => UBLINEAR2

    All criteria changes for YUV422 only in terms of width as below
    if image width <= 16   - UBLINEAR1
    else image_width <=32  - UBLINEAR2
    */
   if ((ulW <= BPXL_UIF_UT_W_IN_BLOCKS_2D) || (ulH <= BPXL_UIF_UT_H_IN_BLOCKS_2D))
      *peSwizzling = BPXL_Uif_Swizzling_eLT;
   else if (ulW <=  BPXL_UIF_UB_W_IN_BLOCKS_2D)     /*8*/
      *peSwizzling = BPXL_Uif_Swizzling_eUBLINEAR1;
   else if (ulW <= (2* BPXL_UIF_UB_W_IN_BLOCKS_2D)) /*16*/
      *peSwizzling = BPXL_Uif_Swizzling_eUBLINEAR2;
   else
      *peSwizzling = BPXL_Uif_Swizzling_eUIF;

   BDBG_MSG(("swizzling %d, in blocks %d x %d", *peSwizzling, ulW, ulH));
}

static uint32_t BPXL_Uif_P_ChooseUbPad(
    uint32_t ulHeight,   /* in UIF blocks */
    BPXL_Uif_Memory_Info *pMemoryInfo)
{
   /* Either (a) pad such that bank conflicts are at least half a page apart in
    * image space, eg:
    *
    * +-----+-----+
    * |     |  2  |
    * |  0  +-----+
    * |     |     |
    * +-----+  3  | -+
    * |     |     |  +- Conflicting 0s are half a page apart
    * |  1  +-----+ -+
    * |     |     |
    * +-----+  0  |
    * |  2  |     |
    * +-----+-----+
    *
    * or (b) pad to a multiple of the page cache size, which will trigger the
    * XOR rule to avoid bank conflicts.
    *
    * There isn't really any point in padding if there's only one column, but
    * it doesn't seem worth complicating this logic with such a test:
    * - It only matters for the level 1 mipmap -- the level 0 mipmap padding
    *   doesn't need to be allocated in software in the one-column case, and
    *   levels 2 and above are padded up to a power-of-2 anyway.
    * - Only very tall and narrow images will be unnecessarily padded. Such
    *   images are rare. */

    uint32_t ulPc15InUBRows, ul15PageinUBRows, ulPcInUBRows, ulPcm;
    ulPc15InUBRows = pMemoryInfo->ulPc15InUBRows;
    ul15PageinUBRows = pMemoryInfo->ul15PageinUBRows;
    ulPcInUBRows = pMemoryInfo->ulPcInUBRows;

    BDBG_ASSERT(ulPcInUBRows);
    ulPcm = ulHeight % ulPcInUBRows;
    if (ulPcm == 0)
      /* (b) Already an exact multiple of the page cache size. XOR rule will be
       * triggered. */
      return 0;
    else if (ulPcm < ul15PageinUBRows)
    {
      /* Just over (n * page cache size) */
      if (ulHeight < ulPcInUBRows)
         /* (a) n = 0 special case: assuming at least 4 banks, page cache
          * covers more than two columns, so even without padding there will be
          * no close bank conflicts */
         return 0;
      else
         /* (a) Pad just enough to keep a half-page distance between  conflicting pages */
         return (ul15PageinUBRows - ulPcm);
    }
    else if (ulPcm > ulPc15InUBRows)
      /* (b) Just under (n * page cache size). Pad up to an exact multiple.
       * This will trigger the XOR rule. */
      return (ulPcInUBRows - ulPcm);
    else
      /* (a) Somewhere in the middle. Should be no close bank conflicts, so leave as is. */
      return 0;
    }

static void BPXL_Uif_P_AdjustPadding(
    BPXL_Uif_Memory_Info *pMemoryInfo,
    bool      bEnableXOR,
    uint32_t *pulPaddedW,   /* in UIF blocks*/
    uint32_t *pulPaddedH,   /* in UIF blocks*/
    BPXL_Uif_Swizzling *peswizzling)
{

    uint32_t ulPaddedW = *pulPaddedW;
    uint32_t ulPaddedH = *pulPaddedH;

      /* Figure out padded dims & adjust UIF XOR-ness */
    switch (*peswizzling)
    {
        case BPXL_Uif_Swizzling_eUIF:
        {
            uint32_t padded_height_in_ub = BPXL_Uif_P_udiv_round_up(ulPaddedH, BPXL_UIF_UB_H_IN_BLOCKS_2D);
            padded_height_in_ub += BPXL_Uif_P_ChooseUbPad(padded_height_in_ub, pMemoryInfo);

            /* Workaround HWBCM7260-81, the padded height must be an even multiple of UBs */
#if (BCHP_CHIP == 7260) && (BCHP_VER < BCHP_VER_B0)
            padded_height_in_ub = (padded_height_in_ub +1) & (~0x1);
#endif

            ulPaddedH = padded_height_in_ub * BPXL_UIF_UB_H_IN_BLOCKS_2D;
            ulPaddedW = BPXL_P_ALIGN_UP(ulPaddedW, BPXL_UIF_UCOL_W_IN_BLOCKS_2D);

            /* XOR mode should not be enabled unless padded height is a multiple
                    * of the page cache size, as block addresses might otherwise end up
                    * outside of the buffer. Also only enable it if we're wider than a
                    * single UIF column -- XOR mode only affects odd columns, so it's
                    * pointless to enable unless there's more than one column. */
            if (bEnableXOR &&
                 ((padded_height_in_ub % pMemoryInfo->ulPcInUBRows) == 0) &&
                 (ulPaddedW > BPXL_UIF_UCOL_W_IN_BLOCKS_2D))
            *peswizzling = BPXL_Uif_Swizzling_eUIF_XOR;

            break;
        }
        case BPXL_Uif_Swizzling_eLT:
            ulPaddedW = BPXL_P_ALIGN_UP(ulPaddedW, BPXL_UIF_UT_W_IN_BLOCKS_2D);
            ulPaddedH = BPXL_P_ALIGN_UP(ulPaddedH, BPXL_UIF_UT_H_IN_BLOCKS_2D);
            break;
        case BPXL_Uif_Swizzling_eUBLINEAR1:
        case BPXL_Uif_Swizzling_eUBLINEAR2:
            ulPaddedW = BPXL_P_ALIGN_UP(ulPaddedW, BPXL_UIF_UB_W_IN_BLOCKS_2D);
            ulPaddedH = BPXL_P_ALIGN_UP(ulPaddedH, BPXL_UIF_UB_H_IN_BLOCKS_2D);
            break;
        default:
            BDBG_ASSERT(0); /* unreachable */
    }

    *pulPaddedH = ulPaddedH;
    *pulPaddedW = ulPaddedW;
}

static void BPXL_Uif_P_Calc_Pitch_and_Size(
    uint32_t *pPitch,
    uint32_t *pSize,
    BPXL_Uif_Swizzling eSwizzling,
    uint32_t ulPaddedW,    /* in UIF block */
    uint32_t ulpaddedH)    /* in UIF block */
{

    uint32_t ulPitchPar, ulPitchPer;   /* in UIF blocks */
    bool bUIF = (BPXL_Uif_Swizzling_eUIF == eSwizzling)||(BPXL_Uif_Swizzling_eUIF_XOR==eSwizzling);

    ulPitchPar = bUIF?ulpaddedH : ulPaddedW;
    ulPitchPer = bUIF?ulPaddedW : ulpaddedH;


    *pPitch = ulPitchPar * BPXL_UIF_COL_WIDTH_IN_BLOCK;
    *pSize = *pPitch * ulPitchPer;
}

void BPXL_Uif_SurfaceCfg(
    BPXL_Uif_Memory_Info *pMemoryInfo,
    BPXL_Uif_Surface *pSurface)
{
    uint32_t  ulWidth;       /* in pixels*/
    uint32_t  ulHeight;      /* in pixels*/
    uint32_t  ulMaxMipLevels;

    uint32_t  ulPc15InUBRows;
    uint32_t  ulPageSize;

    uint32_t ulMLsizes[BPXL_UIF_MAX_MIP_LEVELS+1]; /* Size of each mipmap level, including all padding */
    uint32_t ulML0Align=0; /* Required alignment of level 0 */
    uint32_t ulMl0Pitch; /* pitch or UIF height in bytes of level 0 */
    BPXL_Uif_Swizzling eMl0swizzling; /* swizzling type of level 0 */
    uint32_t  ulMipLevel;
    uint32_t ulTotalSize = 0; /* Total size of all mipmap levels plus padding between them */
    uint32_t ulP2Width,ulP2Height;
    bool bEnableXOR;

    BDBG_ASSERT(pSurface);

    ulWidth  = pSurface->ulWidth;
    ulHeight = pSurface->ulHeight;
    ulMaxMipLevels = pSurface->ulMipLevel;
    /* For maximum compatibility of images between M2MC, mipmap M2MC and
     * V3D, we do not want to use address XOR on images with no additional
     * miplevels. */
    bEnableXOR = (pSurface->ulMipLevel != 0);

   /* Padding to power-of-2 is always done with level 1 size.
    * 2* is to get back up to level 0 size.
    * NOTE: As this only deals with uncompressed 4byte formats, it is much
    * simpler than the original code it is based on. */
    ulP2Width  = 2 * BPXL_Uif_P_next_power_of_2(BPXL_Uif_P_mipsize(ulWidth, 1));
    ulP2Height = 2 * BPXL_Uif_P_next_power_of_2(BPXL_Uif_P_mipsize(ulHeight, 1));
    BDBG_ASSERT(ulMaxMipLevels <= BPXL_UIF_MAX_MIP_LEVELS);

        /* How many UIF-block rows the "page cache" covers */
    ulPc15InUBRows =pMemoryInfo->ulPc15InUBRows;
    ulPageSize = pMemoryInfo->ulPageSize;


    BDBG_MSG(("input %d x %d", ulWidth, ulHeight));

    /* it could be potential minlevel and maxlevel in the future */
    for (ulMipLevel = 0; ulMipLevel <=ulMaxMipLevels; ++ulMipLevel)
    {
      /* Width and height only padded to power-of-2 for levels 2 and smaller */
        uint32_t ulMlAlign=0, ulMlPitch;
        BPXL_Uif_Swizzling eSwizzling;

        /* padded width and height in UIF blocks*/
        uint32_t ulMlPadW = BPXL_Uif_P_mipsize(((ulMipLevel >= 2) ? ulP2Width : ulWidth), ulMipLevel);
        uint32_t ulMlPadH = BPXL_Uif_P_mipsize(((ulMipLevel >= 2) ? ulP2Height : ulHeight), ulMipLevel);

        BPXL_Uif_P_GetSwizzling(ulMlPadW, ulMlPadH, &eSwizzling);
        BPXL_Uif_P_AdjustPadding(pMemoryInfo, bEnableXOR, &ulMlPadW, &ulMlPadH, &eSwizzling);
        BPXL_Uif_P_Calc_Pitch_and_Size(&ulMlPitch, &ulMLsizes[ulMipLevel], eSwizzling, ulMlPadW, ulMlPadH);

          /* Add padding to guarantee page alignment of levels 1 and smaller
           * when necessary, which is essentially whenever any of them could be
           * UIF XOR. The power-of-2 padding of levels 2 and smaller preserves
           * page alignment for as long as is necessary, so we only actually
           * need to pad level 1 here. */
          if ((ulMipLevel == 1) &&
             (ulMlPadW > BPXL_UIF_UCOL_W_IN_BLOCKS_2D) &&
             (ulMlPadH > (ulPc15InUBRows * BPXL_UIF_UB_H_IN_BLOCKS_2D)))
          {
             ulMLsizes[ulMipLevel] = BPXL_Uif_P_uround_up(ulMLsizes[ulMipLevel], ulPageSize);
          }

          switch (eSwizzling)
          {
             case BPXL_Uif_Swizzling_eUIF:
             case BPXL_Uif_Swizzling_eUBLINEAR1:
             case BPXL_Uif_Swizzling_eUBLINEAR2:
                ulMlAlign = BPXL_UIF_BLOCKSIZE;
                break;
             case BPXL_Uif_Swizzling_eUIF_XOR:
                ulMlAlign = ulPageSize;
                break;
             case BPXL_Uif_Swizzling_eLT:
                ulMlAlign = BPXL_UIF_TILESIZE;
                break;
             default:
                BDBG_ASSERT(0); /* unreachable */
          };
          ulMlAlign = BPXL_P_MAX(ulMlAlign, BPXL_UIF_V3D_TMU_ML_ALIGN);

          if (ulMipLevel == 0)
          {
             pSurface->ulL0Pitch   = ulMl0Pitch    = ulMlPitch; /* For UIF/XOR L0 "UIF height" divide by pixel size (4bytes) */
             pSurface->ulAlign     = ulML0Align    = ulMlAlign;
             pSurface->eL0Swizzling    = eMl0swizzling = eSwizzling;
             pSurface->ulPadH = ulMlPadH;
          }
          else
          {
             uint32_t ulMisalign=0;
             /* We only enforce mip level 0 alignment below. So make sure that
              * correct alignment of the other mip levels is implied by correct
              * alignment of level 0.. */

             /* Mip level 0 should have greatest alignment requirement */
             ulML0Align = BPXL_P_MAX(ulML0Align, ulMlAlign);

             /* Offset to this mip level from mip level 0 should not destroy
              * alignment */
             ulMisalign = (ulMLsizes[ulMipLevel] + ulTotalSize- ulMLsizes[0]) % ulMlAlign;
             if (ulMisalign != 0)
                ulMLsizes[ulMipLevel] += ulMlAlign - ulMisalign;
          }

          BDBG_MSG(("level %d type %d after alignment  %d x %d size %d",ulMipLevel, eSwizzling, ulMlPadW, ulMlPadH, ulMLsizes[ulMipLevel]));

      ulTotalSize += ulMLsizes[ulMipLevel];
   }


   {
      uint32_t ulMl0OffsetUnaligned = ulTotalSize- ulMLsizes[0];
      uint32_t ulMl0MisAlignment = BPXL_Uif_P_uround_up(ulMl0OffsetUnaligned, ulML0Align) - ulMl0OffsetUnaligned;
      pSurface->ulSize      = ulTotalSize + ulMl0MisAlignment;
      pSurface->ulL0ffset = ulMl0OffsetUnaligned + ulMl0MisAlignment;
   }


   BDBG_MSG(("total size %d L0offset %d",  pSurface->ulSize , pSurface->ulL0ffset));
}

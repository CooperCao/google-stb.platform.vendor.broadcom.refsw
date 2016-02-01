/***************************************************************************
 *     Copyright (c) 2004-2006, Broad-com Corporation
 *     All Rights Reserved
 *     Confidential Property of Broad-com Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROAD-COM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *   See Module Overview below.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#include "bstd.h"
#include "bkni.h"          /* For malloc */
#include "bmem.h"
#include "bxvd.h"
#include "bxvd_platform.h"
#include "bxvd_priv.h"
#include "bxvd_reg.h"
#include "bxvd_errors.h"
#include "bxvd_intr.h"

#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

#define BXVD_P_TEST_CHUNK 16

/*
 * For testing during initial bringup, we may not want to send an init host
 * command, so setting this define to 0 will skip the init.
 */
#define FW_INIT 1

/* Temporary until this is properly defined */
#ifndef BCHP_DECODE_CPUREGS2_REG_CPU_INT_BASE_1
#define BCHP_DECODE_CPUREGS2_REG_CPU_INT_BASE_1 0x00800f8c
#endif

/* Only stripe widths of 64 and 128 are supported. */
const uint32_t BXVD_P_StripeWidthLUT[BXVD_P_STRIPE_WIDTH_NUM] =
{
   64,
   128,
#if (BXVD_P_STRIPE_WIDTH_NUM == 3)
   256
#endif
};

typedef enum BXVD_VideoProtocol
{
   BXVD_VideoProtocol_eAVC,   /* AVC, H264 */
   BXVD_VideoProtocol_eMPEG2, /* MPEG2, H261, H263, MPEG1, MPEG2DTV, MPEG2_DSS_PES */
   BXVD_VideoProtocol_eVC1,   /* VC1, VC1-SM */
   BXVD_VideoProtocol_eMPEG4, /* MPEG2 Part 2, DivX */
   BXVD_VideoProtocol_eAVS,   /* AVS */
   BXVD_VideoProtocol_eVP8,   /* VP8 */
   BXVD_VideoProtocol_eMVC,   /* Multi View Coding */
   BXVD_VideoProtocol_eSVC,   /* Scaleable Video Coding */
   BXVD_VideoProtocol_eHEVC,  /* High Efficiency Video Coding */

   /* Add new protocols ABOVE this line */
   BXVD_VideoProtocol_eMax
} BXVD_VideoProtocol;

static const BXVD_P_FWMemConfig_V2 sChannelFWMemCfg[BXVD_VideoProtocol_eMax][BXVD_DecodeResolution_eMaxModes] =
{  /* Context, InnerLoop WL, DirectMode, Cabac bin,  Cabac WL,  Vid Blk (index),  Blk Cnt */

   /* BXVD_VideoProtocol_eAVC */
   {
      {261440, 1572864, 1638400, 5210112, 131072, BXVD_P_VideoAtomIndex_eM, 7}, /* AVC HD */
      {261440,  786432,  388800, 2084864,  65536, BXVD_P_VideoAtomIndex_eE, 3}, /* AVC SD */
      {261440,  393216,  221760,  417792,  16384, BXVD_P_VideoAtomIndex_eC, 9}, /* AVC CIF */
      {261440,  196608,   44000,  106496,  16384, BXVD_P_VideoAtomIndex_eD, 4}  /* AVC QCIF */
   },

   /* BXVD_VideoProtocol_eMPEG2 */
   {
      {154944, 65536, 0, 2097152, 131072, BXVD_P_VideoAtomIndex_eM, 6}, /* MPEG2 HD w/ BTP */
      {154944, 65536, 0, 1048576,  65536, BXVD_P_VideoAtomIndex_eB, 6}, /* MPEG2 SD w/ BTP */
      {154944, 16384, 0,  312448,  16384, BXVD_P_VideoAtomIndex_eC, 5}, /* MPEG2 CIF */
      {154944, 16384, 0,   54656,  16384, BXVD_P_VideoAtomIndex_eD, 2}  /* MPEG2 QCIF */
   },

   /* BXVD_VideoProtocol_eVC1 */
   {
      {154944,	917504,	103680, 3906176, 131072, BXVD_P_VideoAtomIndex_eM, 5}, /* VC1 HD */
      {154944,  204800,  21632, 1562496,  65536, BXVD_P_VideoAtomIndex_eB, 5}, /* VC1 SD */
      {154944,  102400,   5888,  312448,  16384, BXVD_P_VideoAtomIndex_eC, 5}, /* VC1 CIF */
      {154944,  102400,   1792,   54656,  16384, BXVD_P_VideoAtomIndex_eD, 2}  /* VC1 QCIF */
   },

   /* BXVD_VideoProtocol_eMPEG4 */
   {
      {154944, 98304, 168960, 3906176, 131072, BXVD_P_VideoAtomIndex_eM, 5}, /* MPEG4/DivX HD */
      {154944, 98304,  34624, 1562496,  65536, BXVD_P_VideoAtomIndex_eB, 5}, /* MPEG4/DivX SD */
      {154944, 20480,   9024,  312448,  16384, BXVD_P_VideoAtomIndex_eC, 5}, /* MPEG4/DivX CIF */
      {154944, 20480,	2560,   54656,  16384, BXVD_P_VideoAtomIndex_eD, 2}  /* MPEG4/DivX QCIF */
   },

   /* BXVD_VideoProtocol_eAVS */
   {
      {154944, 65536, 587520, 3906176, 131072, BXVD_P_VideoAtomIndex_eM, 5}, /* AVS HD */
      {154944, 65536, 116600, 1562496,  65536, BXVD_P_VideoAtomIndex_eB, 5}, /* AVS SD */
      {154944, 16384,  28512,  312448,  16384, BXVD_P_VideoAtomIndex_eC, 5}, /* AVS CIF */
      {154944, 16384,   7128,   54656,  16384, BXVD_P_VideoAtomIndex_eD, 2}  /* AVS QCIF */
   },

   /* BXVD_VideoProtocol_eVP8 */
   {
      {154944, 917504, 2774420, 5210112, 131072, BXVD_P_VideoAtomIndex_eM, 5}, /* VP8 HD */
      {154944, 204800, 1665024, 1562496,  65536, BXVD_P_VideoAtomIndex_eB, 5}, /* VP8 SD */
      {154944, 102400, 1110016,  312448,  16384, BXVD_P_VideoAtomIndex_eC, 5}, /* VP8 CIF */
      {154944, 102400, 1110016,   54656,  16384, BXVD_P_VideoAtomIndex_eD, 2}  /* VP8 QCIF */
   }
};

static const BXVD_P_FWMemConfig_V2 sChannelStillFWMemCfg[BXVD_VideoProtocol_eMax-1][BXVD_DecodeResolution_eSD+1] =
{  /* Context, InnerLoop WL, DirectMode, Cabac bin,  Cabac WL,  Vid Blk (index),  Blk Cnt */

   /* BXVD_VideoProtocol_eAVC */
   {
      { 261440, 131072, 0, 921600, 8192, BXVD_P_VideoAtomIndex_eA, 1}, /* AVC HD Still */
      { 261440,  32768, 0, 409600, 8192, BXVD_P_VideoAtomIndex_eB, 1}  /* AVC SD Still */
   },

   /* BXVD_VideoProtocol_eMPEG2 */
   {
      { 154944, 1024, 0, 921600, 8192, BXVD_P_VideoAtomIndex_eA, 1}, /* MPEG2 HD Still */
      { 154944, 1024, 0, 204800, 8192, BXVD_P_VideoAtomIndex_eB, 1}  /* MPEG2 SD Still */
   },

   /* BXVD_VideoProtocol_eVC1 */
   {
      { 154944, 8192, 0, 307200, 8192, BXVD_P_VideoAtomIndex_eA, 1}, /* VC1 HD Still */
      { 154944, 8192, 0, 204800, 8192, BXVD_P_VideoAtomIndex_eB, 1}  /* VC1 SD Still */
   },

   /* BXVD_VideoProtocol_eMPEG4 */
   {
      { 154944, 1024, 0, 307200, 8192, BXVD_P_VideoAtomIndex_eA, 1}, /* MPEG4/DivX HD Still */
      { 154944, 1024, 0, 204800, 8192, BXVD_P_VideoAtomIndex_eB, 1}  /* MPEG4/DivX SD Still */
   },

   /* BXVD_VideoProtocol_eAVS */
   {
      { 154944, 1024, 0, 307200, 8192, BXVD_P_VideoAtomIndex_eA, 1}, /* AVS HD Still */
      { 154944, 1024, 0, 204800, 8192, BXVD_P_VideoAtomIndex_eB, 1}  /* AVS SD Still */
   },

   /* BXVD_VideoProtocol_eVP8 */
   {
      { 154944, 131072, 1109768+248, 1572864, 131072, BXVD_P_VideoAtomIndex_eA, 1}, /* VP8 HD Still */
      { 154944,  32768, 1109768+248,  614400,  65536, BXVD_P_VideoAtomIndex_eB, 1}  /* VP8 SD Still */
   },
};

static const BXVD_P_FWMemConfig_V2 sChannelStillFWMemCfg_HEVC[3] =
{  /* Context, InnerLoop WL, DirectMode, Cabac bin,  Cabac WL,  Vid Blk (index),  Blk Cnt */
   /* BXVD_VideoProtocol_eAVC */
   { 694892,  78196,        0,         921600,      22920, BXVD_P_VideoAtomIndex_eA, 1}, /* HEVC HD Still */
   { 694892,  32116,        0,         409600,      22920, BXVD_P_VideoAtomIndex_eB, 1}, /* HEVC SD Still */
   { 694892, 206196,        0,        3670016,     204800, BXVD_P_VideoAtomIndex_eJ, 1}  /* HEVC 4K Still */
};

static const BXVD_P_FWMemConfig_V2 sChannelFWMemCfg_BluRay[2] =
{ /* Context, InnerLoop WL, DirectMode, Cabac bin,  Cabac WL,  Vid Blk (index),  Blk Cnt */

   /* BXVD_VideoProtocol_eAVC for BluRay*/
   {261440, 786432, 1638400, 4194304, 131072, BXVD_P_VideoAtomIndex_eA, 7}, /* AVC HD */
   {261440, 524288,  388800, 2084864,  65536, BXVD_P_VideoAtomIndex_eE, 3}  /* AVC SD */
};

static const BXVD_P_FWMemConfig_V2 sChannelFWMemCfg_AVC41[1] =
{
   /* BXVD_VideoProtocol_eAVC 4.1 */
   {261440, 3145728, 1638400, 13107200, 131072, BXVD_P_VideoAtomIndex_eM, 7} /* AVC 4.1 HD */
};

static const BXVD_P_FWMemConfig_V2 sChannelFWMemCfg_AVC51[1] =
{
   /* BXVD_VideoProtocol_eAVC 5.1 */
   {261440, 3145728, 5570560, 13107200, 131072, BXVD_P_VideoAtomIndex_eF, 9} /* AVC 5.1 HD */
};

static const BXVD_P_FWMemConfig_V2 sChannelFWMemCfg_1200HD[1] =
{
   /* BXVD_VideoProtocol_eAVC 5.1 1200 */
   {261440, 3145728, 5570560, 13107200, 131072, BXVD_P_VideoAtomIndex_eK, 9} /* AVC 5.1 1920x1200 HD */
};

/* Support AVC streams that require more direct memory to be decoded. The streams are
   non compliant, however other STBs in the world decode them, so our customer would
   like this chip to decode the streams as well */

static const BXVD_P_FWMemConfig_V2 sChannelFWMemCfg_bExcessDir[2] =
{
   /* Context, IL WL, DirectMode, Cabac bin,  Cabac WL,  Vid Blk (index),  Blk Cnt */

   {261440, 1572864, (1638400*2), 5210112, 131072, BXVD_P_VideoAtomIndex_eM, 7}, /* AVC HD */
   {261440,  786432,  (388800*2), 2084864,  65536, BXVD_P_VideoAtomIndex_eE, 3}, /* AVC SD */
};

static const BXVD_P_FWMemConfig_V2 sChannelFWMemCfg_MVC[1] =
{
   /* BXVD_VideoProtocol_eMVC 4.1 */
   {522880, 1572864, 3276800, 10420224, 131072, BXVD_P_VideoAtomIndex_eF, 7} /* MVC 4.1 HD */
};


static const BXVD_P_FWMemConfig_SVC sChannelFWMemCfg_SVC[3] =
{
   /* Context, IL WL,  DirectMode,     Inter Video,          Inter MV,  Cabac bin,  Cabac WL,  Vid Blk (index),      Blk Cnt */
   {522880,   3145728,  1638400,   BXVD_P_VideoAtomIndex_eI,  2219520,   9961472,    131072,   BXVD_P_VideoAtomIndex_eA, 7}, /* Interlaced */
   {522880,   3145728,  1638400,   BXVD_P_VideoAtomIndex_eG,  2219520,   9961472,    131072,   BXVD_P_VideoAtomIndex_eA, 7}, /* Progressive */
   {522880,   3145728,  1638400,   BXVD_P_VideoAtomIndex_eH,  2219520,   9961472,    131072,   BXVD_P_VideoAtomIndex_eA, 7}, /* 3D */
};

static const BXVD_P_FWMemConfig_V2 sChannelFWMemCfg_HEVC[5] =
{
   /* BXVD_VideoProtocol_eHEVC */
   /* Context - 32K,     IL WL,   DirectMode, Cabac bin,  Cabac WL,  Vid Blk (indx),  Blk Cnt */
/* {655360-(32*1024),   2502272,   689241,      4456448,    782080,   BXVD_P_VideoAtomIndex_eI,    4}, / * HEVC HD 4.1 */
/* {655360-(32*1024),   2502272,   689241,      3670016,    782080,   BXVD_P_VideoAtomIndex_eA,    8}, / * Old HEVC HD 4.0 */
   {655360-(32*1024),   2502272,   689241,      3670016,    782080,   BXVD_P_VideoAtomIndex_eI,    4}, /* HEVC HD 4.0 */
   {655360-(32*1024),   1027712,   179847,      2097152,    418560,   BXVD_P_VideoAtomIndex_eB,   10}, /* HEVC SD */
   {655360-(32*1024),    426720,    37065,       524288,    106880,   BXVD_P_VideoAtomIndex_eC,    8}, /* HEVC CIF */
   {655360-(32*1024),    213360,    15579,       262144,     58240,   BXVD_P_VideoAtomIndex_eD,    3}, /* HEVC QCIF */
   {655360-(32*1024),   6598272,  2928345,     10485760,   1606400,   BXVD_P_VideoAtomIndex_eJ,    8}, /* HEVC 4K 5.1 */
/* {655360-(32*1024),   6598272,  2928345,      7077888,   1606400,   BXVD_P_VideoAtomIndex_eJ,    8}, / * HEVC 4K 5.0 */
};

BDBG_MODULE(BXVD_PRIV);

void BXVD_P_ValidateHeaps(BXVD_Handle        hXvd,
                          BXVD_P_MemCfgMode  eMemCfgMode)
{
   /* UMA memory configuration */
   if (eMemCfgMode == BXVD_P_MemCfgMode_eUMA)
   {
      if (hXvd->hGeneralHeap != hXvd->hPictureHeap)
      {
         BXVD_DBG_MSG(hXvd, ("UMA Mode, Picture buffers using separate heap"));
      }
      else
      {
         BXVD_DBG_MSG(hXvd, ("UMA Mode, Picture buffers using system heap as expected."));
      }
   }

   /* NON-UMA memory configuration */
   else if (eMemCfgMode == BXVD_P_MemCfgMode_eNONUMA)
   {
      if (hXvd->hGeneralHeap == hXvd->hPictureHeap)
      {
         BXVD_DBG_WRN(hXvd, ("****NON UMA MODE Error: Picture buffers should not use system heap"));
      }
      else
      {
         BXVD_DBG_MSG(hXvd, ("NON-UMA Mode, Picture buffers using separate heap as expected."));
      }
   }

   /* UNKNOWN memory configuration */
   else
   {
      if (hXvd->hGeneralHeap == hXvd->hPictureHeap)
      {
         BXVD_DBG_MSG(hXvd, ("UMA Mode, Picture buffers using system heap."));
      }
      else
      {
         BXVD_DBG_MSG(hXvd, ("UNKNOWN UMA/NON UMA Mode, Picture buffers using separate heap."));
      }
   }

   /* If private picture heap is specified, but the specified picture heap size is 0, output a warning.
    * This use of the API is most likely not intentional. Non zero general heap size indicates custom
    * memory decode mode. Old memory API method (compatibility mode) will have a zero picture heap size. */

   if ((hXvd->hGeneralHeap != hXvd->hPictureHeap) &&
       (hXvd->stSettings.stFWMemConfig.uiPictureHeapSize == 0) &&
       (hXvd->stSettings.stFWMemConfig.uiGeneralHeapSize != 0))
   {
      BXVD_DBG_WRN(hXvd, ("Separate picture heap specified with picture memory size of 0. Pictures are NOT expected to be displayed."));
   }
}


bool BXVD_P_IsDecodeProtocolSupported(BXVD_Handle               hXvd,
                                      BAVC_VideoCompressionStd  eVideoCmprStd )
{
   bool rc = false;

   /* Platform may be capable to decode RV9, but it may have been disabled by OTP */
   if (eVideoCmprStd == BAVC_VideoCompressionStd_eRV9)
   {
      if (hXvd->bRV9Capable)
      {
         rc = true;
      }
   }

   else if (eVideoCmprStd == BAVC_VideoCompressionStd_eSVC)
   {
      if (hXvd->bSVCCapable)
      {
         rc = true;
      }
   }

   else if (eVideoCmprStd >= BAVC_VideoCompressionStd_eMPEG4Part2)
   {
      if (BXVD_P_CREATE_PROTOCOLS_MASK(eVideoCmprStd) & hXvd->uiSupportedProtocolsMask)
      {
         rc = true;
      }
   }

   /* All others are supported on all platforms */
   else if (eVideoCmprStd < BAVC_VideoCompressionStd_eMPEG4Part2)
   {
      rc = true;
   }

   return rc;
}

BERR_Code BXVD_P_MapToAVDProtocolEnum( BXVD_Handle               hXvd,
                                       BAVC_VideoCompressionStd  eVideoCmprStd,
                                       BXVD_P_PPB_Protocol *     peProtocol )
{
   BERR_Code rc = BERR_SUCCESS;

   /* SW7425-3177: The enums used to define the video protocol have diverged between AVD
    * and bavc.h, i.e. BXVD_P_PPB_Protocol is no longer in sync with BAVC_VideoCompressionStd.
    * In addition to verifying that a protocol is supported, map eVideoCmprStd
    * to a BXVD_P_PPB_Protocol value.
    */
   switch( eVideoCmprStd )
   {
      case BAVC_VideoCompressionStd_eH264:
         *peProtocol = BXVD_P_PPB_Protocol_eH264;
         break;
#if BXVD_P_HVD_PRESENT
      case BAVC_VideoCompressionStd_eH265:
         *peProtocol = BXVD_P_PPB_Protocol_eH265;
         break;
      case BAVC_VideoCompressionStd_eVP9:
         *peProtocol = BXVD_P_PPB_Protocol_eVP9;
         break;
#endif
      case BAVC_VideoCompressionStd_eMPEG2:
         *peProtocol = BXVD_P_PPB_Protocol_eMPEG2;
         break;
      case BAVC_VideoCompressionStd_eH261:
         *peProtocol = BXVD_P_PPB_Protocol_eH261;
         break;
      case BAVC_VideoCompressionStd_eH263:
         *peProtocol = BXVD_P_PPB_Protocol_eH263;
         break;
      case BAVC_VideoCompressionStd_eVC1:
         *peProtocol = BXVD_P_PPB_Protocol_eVC1;
         break;
      case BAVC_VideoCompressionStd_eMPEG1:
         *peProtocol = BXVD_P_PPB_Protocol_eMPEG1;
         break;
      case BAVC_VideoCompressionStd_eMPEG2DTV:
         *peProtocol = BXVD_P_PPB_Protocol_eMPEG2DTV;
         break;
      case BAVC_VideoCompressionStd_eVC1SimpleMain:
         *peProtocol = BXVD_P_PPB_Protocol_eVC1SimpleMain;
         break;
      case BAVC_VideoCompressionStd_eMPEG4Part2:
         *peProtocol = BXVD_P_PPB_Protocol_eMPEG4Part2;
         break;
      case BAVC_VideoCompressionStd_eAVS:
         *peProtocol = BXVD_P_PPB_Protocol_eAVS;
         break;
      case BAVC_VideoCompressionStd_eMPEG2_DSS_PES:
         *peProtocol = BXVD_P_PPB_Protocol_eMPEG2_DSS_PES;
         break;
      case BAVC_VideoCompressionStd_eSVC:
         *peProtocol = BXVD_P_PPB_Protocol_eSVC;
         break;
      case BAVC_VideoCompressionStd_eSVC_BL:
         *peProtocol = BXVD_P_PPB_Protocol_eSVC_BL;
         break;
      case BAVC_VideoCompressionStd_eMVC:
         *peProtocol = BXVD_P_PPB_Protocol_eMVC;
         break;
      case BAVC_VideoCompressionStd_eVP6:
         *peProtocol = BXVD_P_PPB_Protocol_eVP6;
         break;
      case BAVC_VideoCompressionStd_eVP7:
         *peProtocol = BXVD_P_PPB_Protocol_eVP7;
         break;
      case BAVC_VideoCompressionStd_eVP8:
         *peProtocol = BXVD_P_PPB_Protocol_eVP8;
         break;
      case BAVC_VideoCompressionStd_eRV9:
         *peProtocol = BXVD_P_PPB_Protocol_eRV9;
         break;
      case BAVC_VideoCompressionStd_eSPARK:
         *peProtocol = BXVD_P_PPB_Protocol_eSPARK;
         break;

      /* Since BXVD_P_IsDecodeProtocolSupported was called prior to
       * this routine this is unlikely to happen.
       */
      case BAVC_VideoCompressionStd_eMOTION_JPEG:
      default:
         BXVD_DBG_ERR(hXvd, ("BXVD_P_IsDecodeProtocolSupported:: unsupported video protocol: 0x%x", eVideoCmprStd));
         *peProtocol = BXVD_P_PPB_Protocol_eH264; /* set the protocol to something. */
         rc = BERR_INVALID_PARAMETER;
         break;
   }

   return BERR_TRACE(rc);
}

BERR_Code BXVD_P_SetupFWSubHeap(BXVD_Handle hXvd)
{
   BERR_Code rc = BERR_SUCCESS;

   unsigned long ulTmp;

   hXvd->SubGenMem = 0;
   hXvd->SubSecureMem = 0;
   hXvd->SubPicMem = 0;

   if (hXvd->uiFWGenMemSize != 0)
   {
      BXVD_DBG_MSG(hXvd, ("Creating BXVD_Memory sub-heaps: Gen VA: 0x%0*lx, PA: %0lx, Size:%0x",
                          BXVD_P_DIGITS_IN_LONG, hXvd->uiFWGenMemBaseVirtAddr,
                          hXvd->uiFWGenMemBasePhyAddr, hXvd->uiFWGenMemSize));

      ulTmp = (unsigned long)(hXvd->uiFWGenMemBasePhyAddr);

      rc = BERR_TRACE(BXVD_P_Memory_Open(hXvd, &hXvd->SubGenMem, /* (void *) hXvd->uiFWGenMemBaseVirtAddr,  */
                                         (void *)ulTmp,  hXvd->uiFWGenMemSize,
                                         BXVD_P_Memory_Protection_eDontProtect));
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }
   }

   if (hXvd->uiFWCabacMemSize != 0)
   {
      BXVD_DBG_MSG(hXvd, ("Creating BXVD_Memory sub-heaps: Secure PA: %0lx, Size:%08x",
                              hXvd->uiFWCabacMemBasePhyAddr, hXvd->uiFWCabacMemSize));

      ulTmp = (unsigned long)(hXvd->uiFWCabacMemBasePhyAddr);
      rc = BERR_TRACE(BXVD_P_Memory_Open(hXvd, &hXvd->SubSecureMem,
                                         (void *)ulTmp,  hXvd->uiFWCabacMemSize,
                                         BXVD_P_Memory_Protection_eDontProtect));
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }
   }
   else
   {
      hXvd->SubSecureMem = hXvd->SubGenMem;
   }

   if (hXvd->uiFWPicMemSize != 0)
   {
      BXVD_DBG_MSG(hXvd, ("Creating BXVD_Memory sub-heaps: Pic PA: %0lx, Size:%08x",
                               hXvd->uiFWPicMemBasePhyAddr, hXvd->uiFWPicMemSize));

      ulTmp = (unsigned long)(hXvd->uiFWPicMemBasePhyAddr);
      rc = BERR_TRACE(BXVD_P_Memory_Open(hXvd, &hXvd->SubPicMem,
                                         (void *)ulTmp,  hXvd->uiFWPicMemSize,
                                         BXVD_P_Memory_Protection_eDontProtect));
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }
   }

   if (hXvd->uiFWPicMem1Size != 0)
   {
      BXVD_DBG_MSG(hXvd, ("Creating BXVD_Memory sub-heaps: Pic1 PA: %0lx, Size:%08x",
                               hXvd->uiFWPicMem1BasePhyAddr, hXvd->uiFWPicMem1Size));

      ulTmp = (unsigned long)(hXvd->uiFWPicMem1BasePhyAddr);
      rc = BERR_TRACE(BXVD_P_Memory_Open(hXvd, &hXvd->SubPicMem1,
                                         (void *)ulTmp,  hXvd->uiFWPicMem1Size,
                                         BXVD_P_Memory_Protection_eDontProtect));
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }
   }

   BXVD_DBG_MSG(hXvd, ("BXVD_Memory sub-heaps created successfully"));
   return BERR_TRACE(rc);

}

BERR_Code BXVD_P_TeardownFWSubHeap(BXVD_Handle hXvd)
{
   BERR_Code rc = BERR_SUCCESS;

   if ((hXvd->SubSecureMem != 0) && (hXvd->SubGenMem != hXvd->SubSecureMem))
   {
      rc = BERR_TRACE(BXVD_P_Memory_Close(hXvd->SubSecureMem));
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }
      hXvd->SubSecureMem = 0;
   }

   if (hXvd->SubGenMem != 0)
   {
      rc = BERR_TRACE(BXVD_P_Memory_Close(hXvd->SubGenMem));
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }
      hXvd->SubGenMem = 0;
   }


   if (hXvd->SubPicMem != 0)
   {
      rc = BERR_TRACE(BXVD_P_Memory_Close(hXvd->SubPicMem));
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }
      hXvd->SubPicMem = 0;
   }

   if (hXvd->SubPicMem1 != 0)
   {
      rc = BERR_TRACE(BXVD_P_Memory_Close(hXvd->SubPicMem1));
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }
      hXvd->SubPicMem1 = 0;
   }

   return BERR_TRACE(rc);

}


BERR_Code BXVD_P_GetDecodeFWMemSize(BXVD_Handle hXvd,
                                    BXVD_DecodeResolution eDecodeResolution,
                                    BAVC_VideoCompressionStd aeVideoCmprStd[],
                                    uint32_t  uiVideoCmprCount,
                                    const BXVD_ChannelSettings *pChSettings,
                                    BXVD_P_DecodeFWMemSize     *pstDecodeFWMemSize)
{

   uint32_t i;
   uint32_t genMemReq = 0;
   uint32_t cabacMemReq = 0;
   uint32_t vidBlkIndex = 99;
   uint32_t lumaVidBlkSizeLookedUp = 0;
   uint32_t chromaVidBlkSizeLookedUp = 0;
   uint32_t vidBlkCountLookedUp = 0;
   uint32_t lumaVidBlkSizeReq = 0;
   uint32_t chromaVidBlkSizeReq = 0;
   uint32_t vidBlkCountReq = 0;
   uint32_t cabacWorklistMemReq = 0;
   uint32_t directModeMemReq = 0;
   uint32_t innerLoopWorklistMemReq = 0;

   bool bNonHevcProtocolPresent = false;

   BXVD_VideoProtocol eVideoProtocol_TableIndex;

   BXVD_P_FWMemConfig_V2 *pChannelFWMemCfg;
   BXVD_P_FWMemConfig_SVC *pSVCChannelFWMemCfg;

   BDBG_ASSERT(hXvd->uiDecode_StripeWidth < BXVD_P_STRIPE_WIDTH_NUM);
   BDBG_ASSERT(hXvd->uiDecode_StripeMultiple < BXVD_P_STRIPE_MULTIPLE_NUM);

   /* Set SVD BL optional sizes to zero */
   pstDecodeFWMemSize->uiFWInterLayerPicSize = 0;
   pstDecodeFWMemSize->uiFWInterLayerMVSize = 0;

   for (i = 0; i < uiVideoCmprCount; i++)
   {
      switch (aeVideoCmprStd[i])
      {
         case BAVC_VideoCompressionStd_eH264:
         case BAVC_VideoCompressionStd_eRV9:
            BXVD_DBG_MSG(hXvd, ("Video Protocol Memory Config: AVC"));
            eVideoProtocol_TableIndex = BXVD_VideoProtocol_eAVC;
            break;

         case BAVC_VideoCompressionStd_eMPEG2:
         case BAVC_VideoCompressionStd_eH261:
         case BAVC_VideoCompressionStd_eH263:
         case BAVC_VideoCompressionStd_eMPEG1:
         case BAVC_VideoCompressionStd_eMPEG2DTV:
         case BAVC_VideoCompressionStd_eMPEG2_DSS_PES:
         case BAVC_VideoCompressionStd_eSPARK:
            BXVD_DBG_MSG(hXvd, ("Video Protocol Memory Config: MPEG2"));
            eVideoProtocol_TableIndex = BXVD_VideoProtocol_eMPEG2;
            break;

         case BAVC_VideoCompressionStd_eVC1:
         case BAVC_VideoCompressionStd_eVC1SimpleMain:
            BXVD_DBG_MSG(hXvd, ("Video Protocol Memory Config: VC1"));
            eVideoProtocol_TableIndex = BXVD_VideoProtocol_eVC1;
            break;

         case BAVC_VideoCompressionStd_eMPEG4Part2:
            BXVD_DBG_MSG(hXvd, ("Video Protocol Memory Config: MPEG4"));
            eVideoProtocol_TableIndex = BXVD_VideoProtocol_eMPEG4;
            break;

         case BAVC_VideoCompressionStd_eAVS:
            BXVD_DBG_MSG(hXvd, ("Video Protocol Memory Config: AVS"));
            eVideoProtocol_TableIndex = BXVD_VideoProtocol_eAVS;
            break;

         case BAVC_VideoCompressionStd_eMVC:
            BXVD_DBG_MSG(hXvd, ("Video Protocol Memory Config: MVC"));
            eVideoProtocol_TableIndex = BXVD_VideoProtocol_eMVC;
            break;

         case BAVC_VideoCompressionStd_eSVC:
            BXVD_DBG_MSG(hXvd, ("Video Protocol Memory Config: SVC"));
            eVideoProtocol_TableIndex = BXVD_VideoProtocol_eSVC;
            break;

         case BAVC_VideoCompressionStd_eVP6:
         case BAVC_VideoCompressionStd_eVP7:
         case BAVC_VideoCompressionStd_eVP8:
            BXVD_DBG_MSG(hXvd, ("Video Protocol Memory Config: VP8"));
            eVideoProtocol_TableIndex = BXVD_VideoProtocol_eVP8;
            break;

#if BXVD_P_HVD_PRESENT
         case BAVC_VideoCompressionStd_eH265:
         case BAVC_VideoCompressionStd_eVP9:
            BXVD_DBG_MSG(hXvd, ("Video Protocol Memory Config: HEVC"));
            eVideoProtocol_TableIndex = BXVD_VideoProtocol_eHEVC;
            break;
#endif

         default:
            BXVD_DBG_WRN(hXvd, ("Video Protocol: Unknown - Defaulting to AVC Memory Config!"));
            eVideoProtocol_TableIndex = BXVD_VideoProtocol_eAVC;
            break;
      }

      switch (eDecodeResolution)
      {
         case BXVD_DecodeResolution_eHD:
            BXVD_DBG_MSG(hXvd, ("Video Resolution: HD"));
            break;

         case BXVD_DecodeResolution_eSD:
            BXVD_DBG_MSG(hXvd, ("Video Resolution: SD"));
            break;

         case BXVD_DecodeResolution_eCIF:
            BXVD_DBG_MSG(hXvd, ("Video Resolution: CIF"));
            break;

         case BXVD_DecodeResolution_eQCIF:
            BXVD_DBG_MSG(hXvd, ("Video Resolution: QCIF"));
            break;

         case BXVD_DecodeResolution_e4K:
            BXVD_DBG_MSG(hXvd, ("Video Resolution: 4K"));
            break;

         default:
            BXVD_DBG_WRN(hXvd, ("Video Resolution: Unknown - Defaulting to HD!"));
            eDecodeResolution = BXVD_DecodeResolution_eHD;
            break;
      }

      /* Default SVC memory configuration is entry [0]. */
      pSVCChannelFWMemCfg = (BXVD_P_FWMemConfig_SVC *)&(sChannelFWMemCfg_SVC[0]);

      /* Default non SVC memory configuration entry */
      pChannelFWMemCfg = (BXVD_P_FWMemConfig_V2 *)&(sChannelFWMemCfg[eVideoProtocol_TableIndex][eDecodeResolution]);

      if (eVideoProtocol_TableIndex != BXVD_VideoProtocol_eHEVC)
      {
         bNonHevcProtocolPresent = true;
      }

      /* SVC */
      if ((eVideoProtocol_TableIndex == BXVD_VideoProtocol_eSVC) && (hXvd->bSVCCapable == true))
      {
         if (pChSettings->bSVC3DModeEnable)
         {
            pSVCChannelFWMemCfg = (BXVD_P_FWMemConfig_SVC *)&(sChannelFWMemCfg_SVC[2]);
         }
         else if (pChSettings->bSVCProgressiveOnly == true)
         {
            pSVCChannelFWMemCfg = (BXVD_P_FWMemConfig_SVC *)&(sChannelFWMemCfg_SVC[1]);
         }

         vidBlkIndex = pSVCChannelFWMemCfg->video_block_size_index;

         BXVD_P_GET_BUFFER_ATOM_SIZE(hXvd, pChSettings, vidBlkIndex, &lumaVidBlkSizeLookedUp, &chromaVidBlkSizeLookedUp);

         vidBlkCountLookedUp = pSVCChannelFWMemCfg->video_block_count;

#if BXVD_P_ILS_BUFFERS_INTERNAL
         pstDecodeFWMemSize->uiFWInterLayerPicSize = 0;
         pstDecodeFWMemSize->uiFWInterLayerMVSize = 0;
#else
         vidBlkIndex = pSVCChannelFWMemCfg->inter_layer_video_size_index;

         if (hXvd->stSettings.bInterLayerBandwidthOptimized)
         {
            BXVD_P_GET_BUFFER_ATOM_SIZE(hXvd, pChSettings, vidBlkIndex, &lumaVidBlkSizeLookedUp, &chromaVidBlkSizeLookedUp);
            /* Multiply by 1.5 for Bandwidth optimization */
            pstDecodeFWMemSize->uiFWInterLayerPicSize =   ((lumaVidBlkSizeLookedUp * 15) / 10);

            pstDecodeFWMemSize->uiFWInterLayerMVSize = ((pSVCChannelFWMemCfg->inter_layer_mv_size * 15) / 10);
         }
         else
         {
            BXVD_P_GET_BUFFER_ATOM_SIZE(hXvd, pChSettings, vidBlkIndex, &lumaVidBlkSizeLookedUp, &chromaVidBlkSizeLookedUp);

            pstDecodeFWMemSize->uiFWInterLayerPicSize = lumaVidBlkSizeLookedUp;
            pstDecodeFWMemSize->uiFWInterLayerMVSize =  pSVCChannelFWMemCfg->inter_layer_mv_size;
         }
#endif
      }

      else
      {
         /* AVC 4.1 enabled */
         if (pChSettings->bAVC41Enable &&
             (eDecodeResolution == BXVD_DecodeResolution_eHD) &&
             (eVideoProtocol_TableIndex== BXVD_VideoProtocol_eAVC))
         {
            pChannelFWMemCfg = (BXVD_P_FWMemConfig_V2 *)&sChannelFWMemCfg_AVC41;
            vidBlkIndex = pChannelFWMemCfg->video_block_size_index;
            vidBlkCountLookedUp = pChannelFWMemCfg->video_block_count;
         }

         /* AVC 5.1 enabled */
         else if (pChSettings->bAVC51Enable && (eDecodeResolution == BXVD_DecodeResolution_eHD) &&
                  (eVideoProtocol_TableIndex == BXVD_VideoProtocol_eAVC))
         {
            pChannelFWMemCfg = (BXVD_P_FWMemConfig_V2 *)&sChannelFWMemCfg_AVC51;
            vidBlkIndex = pChannelFWMemCfg->video_block_size_index;
            vidBlkCountLookedUp = pChannelFWMemCfg->video_block_count;
         }

         /* 1920x1200 HD enabled AVC */
         else if (pChSettings->b1200HDEnable && (eVideoProtocol_TableIndex == BXVD_VideoProtocol_eAVC) &&
                  (eDecodeResolution == BXVD_DecodeResolution_eHD ))
         {
            pChannelFWMemCfg = (BXVD_P_FWMemConfig_V2 *)&sChannelFWMemCfg_1200HD;
            vidBlkIndex = pChannelFWMemCfg->video_block_size_index;
            vidBlkCountLookedUp = pChannelFWMemCfg->video_block_count;
         }

         /* BluRay enabled AVC */
         else if (pChSettings->bBluRayEnable && (eVideoProtocol_TableIndex == BXVD_VideoProtocol_eAVC) &&
               ((eDecodeResolution == BXVD_DecodeResolution_eHD) ||
                (eDecodeResolution == BXVD_DecodeResolution_eSD)))
         {
            pChannelFWMemCfg = (BXVD_P_FWMemConfig_V2 *)&(sChannelFWMemCfg_BluRay[eDecodeResolution]);
            vidBlkIndex = pChannelFWMemCfg->video_block_size_index;
            vidBlkCountLookedUp = pChannelFWMemCfg->video_block_count;
         }

         /* Excess direct memory mode enabled AVC */
         else if (pChSettings->bExcessDirModeEnable && (eVideoProtocol_TableIndex == BXVD_VideoProtocol_eAVC) &&
                  ((eDecodeResolution == BXVD_DecodeResolution_eHD) ||
                   (eDecodeResolution == BXVD_DecodeResolution_eSD)))
         {
            pChannelFWMemCfg = (BXVD_P_FWMemConfig_V2 *)&(sChannelFWMemCfg_bExcessDir[eDecodeResolution]);
            vidBlkIndex = pChannelFWMemCfg->video_block_size_index;
            vidBlkCountLookedUp = pChannelFWMemCfg->video_block_count;
         }

         /* MVC */
         else if (eVideoProtocol_TableIndex == BXVD_VideoProtocol_eMVC)
         {
            pChannelFWMemCfg = (BXVD_P_FWMemConfig_V2 *)&(sChannelFWMemCfg_MVC);
            vidBlkIndex = pChannelFWMemCfg->video_block_size_index;
            vidBlkCountLookedUp = pChannelFWMemCfg->video_block_count;
         }

         /* HEVC */
         else if (eVideoProtocol_TableIndex == BXVD_VideoProtocol_eHEVC)
         {
            pChannelFWMemCfg = (BXVD_P_FWMemConfig_V2 *)&(sChannelFWMemCfg_HEVC[eDecodeResolution]);
            vidBlkIndex = pChannelFWMemCfg->video_block_size_index;
            vidBlkCountLookedUp = pChannelFWMemCfg->video_block_count;

            if (aeVideoCmprStd[i] == BAVC_VideoCompressionStd_eVP9)
               {
                  if (eDecodeResolution == BXVD_DecodeResolution_e4K)
                  {
                     vidBlkCountLookedUp += 2; /* VP9 needs to more 4K buffers compared to HEVC */
                  }
                  else if (eDecodeResolution == BXVD_DecodeResolution_eHD)
                  {
                     vidBlkCountLookedUp += 1; /* VP9 needs to more HD buffers compared to HEVC */
                  }
               }
         }

         /* Normal memory configuration */
         else
         {
            pChannelFWMemCfg = (BXVD_P_FWMemConfig_V2 *)&(sChannelFWMemCfg[eVideoProtocol_TableIndex][eDecodeResolution]);
            vidBlkIndex = sChannelFWMemCfg[eVideoProtocol_TableIndex][eDecodeResolution].video_block_size_index;
            vidBlkCountLookedUp = sChannelFWMemCfg[eVideoProtocol_TableIndex][eDecodeResolution].video_block_count;
         }

         /* Select Atom size "AT" instead of size "A" if in 1920 Portrait mode for certain decode protocols. */
         if ((pChSettings->b1920PortraitModeEnable == true) &&
             (pChSettings->bBluRayEnable == false) &&
             ((vidBlkIndex == BXVD_P_VideoAtomIndex_eA) || (vidBlkIndex == BXVD_P_VideoAtomIndex_eM)) &&
             (eVideoProtocol_TableIndex != BXVD_VideoProtocol_eSVC))
         {
            vidBlkIndex = BXVD_P_VideoAtomIndex_eAT;
            vidBlkCountLookedUp = sChannelFWMemCfg[eVideoProtocol_TableIndex][eDecodeResolution].video_block_count;
         }

         BXVD_P_GET_BUFFER_ATOM_SIZE(hXvd, pChSettings, vidBlkIndex, &lumaVidBlkSizeLookedUp, &chromaVidBlkSizeLookedUp);
      }

      if((eVideoProtocol_TableIndex == BXVD_VideoProtocol_eSVC) && (hXvd->bSVCCapable == true))
      {
         /* Use SVC memory config entry */
         if (pSVCChannelFWMemCfg->context_memory_size > genMemReq)
         {
            genMemReq = pSVCChannelFWMemCfg->context_memory_size;
         }

         if (pSVCChannelFWMemCfg->cabac_bin_size > cabacMemReq)
         {
            cabacMemReq = pSVCChannelFWMemCfg->cabac_bin_size;
         }

         if (pSVCChannelFWMemCfg->cabac_wl_size > cabacWorklistMemReq)
         {
            cabacWorklistMemReq = pSVCChannelFWMemCfg->cabac_wl_size;
         }

         if (pSVCChannelFWMemCfg->direct_mode_size > directModeMemReq)
         {
            directModeMemReq = pSVCChannelFWMemCfg->direct_mode_size;
         }

         if (pSVCChannelFWMemCfg->inner_loop_wl_size > innerLoopWorklistMemReq)
         {
            innerLoopWorklistMemReq = pSVCChannelFWMemCfg->inner_loop_wl_size;
         }
      }
      else
      {
         /* Use NON SVC memory config entry */
         if (pChannelFWMemCfg->general_memory_size > genMemReq)
         {
            genMemReq = pChannelFWMemCfg->general_memory_size;
         }

         if (pChannelFWMemCfg->cabac_bin_size > cabacMemReq)
         {
            cabacMemReq = pChannelFWMemCfg->cabac_bin_size;
         }

         if (aeVideoCmprStd[i] == BAVC_VideoCompressionStd_eVP9)
         {
            pstDecodeFWMemSize->uiFWCabacSize = 10*1024*1024;
         }

         if (pChannelFWMemCfg->cabac_wl_size > cabacWorklistMemReq)
         {
            cabacWorklistMemReq = pChannelFWMemCfg->cabac_wl_size;
         }

         if ((aeVideoCmprStd[i] == BAVC_VideoCompressionStd_eVP9) &&
             ((pChannelFWMemCfg->direct_mode_size * 2) > directModeMemReq))
         {
            directModeMemReq = pChannelFWMemCfg->direct_mode_size * 2;
         }
         else if (pChannelFWMemCfg->direct_mode_size > directModeMemReq)
         {
            directModeMemReq = pChannelFWMemCfg->direct_mode_size;
         }

         if (pChannelFWMemCfg->inner_loop_wl_size > innerLoopWorklistMemReq)
         {
            innerLoopWorklistMemReq = pChannelFWMemCfg->inner_loop_wl_size;
         }

      }
      /* Check bounds of stripe multiple */
      if (hXvd->uiDecode_StripeMultiple >= BXVD_P_STRIPE_MULTIPLE_NUM)
      {
         BXVD_DBG_ERR(hXvd, ("Unsupported stripe multiple: %d", hXvd->uiDecode_StripeMultiple));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
      }

      /* Handle BTP Mode */
      if (!pChSettings->bMPEG2BTPEnable &&
          ((eVideoProtocol_TableIndex == BXVD_VideoProtocol_eMPEG2) &&
           ((eDecodeResolution == BXVD_DecodeResolution_eHD) ||
            (eDecodeResolution == BXVD_DecodeResolution_eSD))))
      {
         /* Reduce block count by 1 if we do not want BTP capability
          * in MPEG2 HD/SD */
         vidBlkCountLookedUp -= 1;
      }

      if ((lumaVidBlkSizeLookedUp * vidBlkCountLookedUp) >
          (lumaVidBlkSizeReq * vidBlkCountReq))
      {
         lumaVidBlkSizeReq = lumaVidBlkSizeLookedUp;
         chromaVidBlkSizeReq = chromaVidBlkSizeLookedUp;
         vidBlkCountReq = vidBlkCountLookedUp;
      }
   }

   /* Align memory block size to 256 bytes. */
   directModeMemReq = ((directModeMemReq + 255) / 256 ) * 256;

#if BXVD_P_PPB_EXTENDED
   /* Context memory needs to be increased by 32k when using extended PPB */
   genMemReq += (32 * 1024);
#endif

   pstDecodeFWMemSize->uiFWContextSize = genMemReq;
   pstDecodeFWMemSize->uiFWCabacSize = cabacMemReq + 256; /* Add 256 to make sure we get next CABAC buffer can have 256 byte alignment */

   if ((pChSettings->bSplitPictureBuffersEnable == true) &&
       (bNonHevcProtocolPresent == true) && (eDecodeResolution != BXVD_DecodeResolution_e4K))
   {
      pstDecodeFWMemSize->uiFWPicLumaBlockSize = lumaVidBlkSizeReq + chromaVidBlkSizeReq;
      pstDecodeFWMemSize->uiFWPicChromaBlockSize = chromaVidBlkSizeReq;
   }
   else if (pChSettings->bSplitPictureBuffersEnable == true)
   {
      pstDecodeFWMemSize->uiFWPicLumaBlockSize = lumaVidBlkSizeReq;
      pstDecodeFWMemSize->uiFWPicChromaBlockSize = chromaVidBlkSizeReq;
   }
   else
   {
      pstDecodeFWMemSize->uiFWPicLumaBlockSize = lumaVidBlkSizeReq + chromaVidBlkSizeReq;
      pstDecodeFWMemSize->uiFWPicChromaBlockSize = 0;
      chromaVidBlkSizeReq = 0;
   }

   pstDecodeFWMemSize->uiFWPicBlockCount = vidBlkCountReq;
   pstDecodeFWMemSize->uiFWCabacWorklistSize = cabacWorklistMemReq + 128; /* Add padding for burst read length */;

   pstDecodeFWMemSize->uiFWDirectModeSize = directModeMemReq;
   pstDecodeFWMemSize->uiFWInnerLoopWorklistSize = innerLoopWorklistMemReq;

   BXVD_DBG_MSG(hXvd, ("ContextSize:0x%x (%d)", genMemReq, genMemReq));
   BXVD_DBG_MSG(hXvd, ("cabacMemReq:0x%x (%d)", cabacMemReq, cabacMemReq));
   BXVD_DBG_MSG(hXvd, ("LumaVidBlockSize:0x%x (%d) , BlkCnt:%d", lumaVidBlkSizeReq, lumaVidBlkSizeReq, vidBlkCountReq));
   BXVD_DBG_MSG(hXvd, ("ChromaVidBlockSize:0x%x (%d) , BlkCnt:%d", chromaVidBlkSizeReq, chromaVidBlkSizeReq, vidBlkCountReq));
   BXVD_DBG_MSG(hXvd, ("cabacWorklistMemReq:0x%x (%d)", cabacWorklistMemReq, cabacWorklistMemReq));
   BXVD_DBG_MSG(hXvd, ("directModeMemSize: 0x%x (%d)", directModeMemReq, directModeMemReq));
   BXVD_DBG_MSG(hXvd, ("innerLoopWorklistSize: 0x%x (%d)", innerLoopWorklistMemReq, innerLoopWorklistMemReq));
   return BERR_TRACE(BERR_SUCCESS);
}

BERR_Code BXVD_P_GetStillDecodeFWMemSize(BXVD_Handle hXvd,
                                         BXVD_DecodeResolution eDecodeResolution,
                                         BAVC_VideoCompressionStd aeVideoCmprStd[],
                                         uint32_t  uiVideoCmprCount,
                                         const BXVD_ChannelSettings *pChSettings,
                                         BXVD_P_DecodeFWMemSize       *pstDecodeFWMemSize)
{
   uint32_t i;
   uint32_t genMemReq = 0;
   uint32_t cabacMemSizeLookedUp = 0;
   uint32_t cabacMemReq = 0;
   uint32_t lumaVidBlkSizeLookedUp = 0;
   uint32_t chromaVidBlkSizeLookedUp = 0;
   uint32_t lumaVidBlkSizeReq = 0;
   uint32_t chromaVidBlkSizeReq = 0;
   uint32_t vidBlkCountReq = 0;
   uint32_t cabacWorklistMemSizeLookedUp = 0;
   uint32_t cabacWorklistMemReq = 0;
   uint32_t directModeMemReq = 0;
   uint32_t innerLoopWorklistMemReq = 0;

   bool bNonHevcProtocolPresent = false;

   BXVD_DecodeResolution temp_eDecodeResolution;

   uint32_t vidBlbSzIndx;
   BXVD_VideoProtocol eVideoProtocol;

   BDBG_ASSERT(hXvd->uiDecode_StripeWidth < BXVD_P_STRIPE_WIDTH_NUM);
   BDBG_ASSERT(hXvd->uiDecode_StripeMultiple < BXVD_P_STRIPE_MULTIPLE_NUM);

   if ((eDecodeResolution != BXVD_DecodeResolution_eHD) &&
       (eDecodeResolution != BXVD_DecodeResolution_eSD) &&
       (eDecodeResolution != BXVD_DecodeResolution_e4K))
   {
      /* coverity[dead_error_line: FALSE] */
      BXVD_DBG_ERR(hXvd, ("Unsupported Still Picture Decode Resolution: %d", eDecodeResolution));
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   /* Set SVD BL optional sizes to zero, these are not used in still decode. */
   pstDecodeFWMemSize->uiFWInterLayerPicSize = 0;
   pstDecodeFWMemSize->uiFWInterLayerMVSize = 0;

   for (i = 0; i < uiVideoCmprCount; i++)
   {
      switch (aeVideoCmprStd[i])
      {
         case BAVC_VideoCompressionStd_eH264:
         case BAVC_VideoCompressionStd_eRV9:
            BXVD_DBG_MSG(hXvd, ("Still Protocol Memory Config: AVC"));
            eVideoProtocol = BXVD_VideoProtocol_eAVC;
            break;

         case BAVC_VideoCompressionStd_eMPEG2:
         case BAVC_VideoCompressionStd_eH261:
         case BAVC_VideoCompressionStd_eH263:
         case BAVC_VideoCompressionStd_eMPEG1:
         case BAVC_VideoCompressionStd_eMPEG2DTV:
         case BAVC_VideoCompressionStd_eMPEG2_DSS_PES:
         case BAVC_VideoCompressionStd_eSPARK:
            BXVD_DBG_MSG(hXvd, ("Still Protocol Memory Config: MPEG2"));
            eVideoProtocol = BXVD_VideoProtocol_eMPEG2;
            break;

         case BAVC_VideoCompressionStd_eVC1:
         case BAVC_VideoCompressionStd_eVC1SimpleMain:
            BXVD_DBG_MSG(hXvd, ("Still Protocol Memory Config: VC1"));
            eVideoProtocol = BXVD_VideoProtocol_eVC1;
            break;

         case BAVC_VideoCompressionStd_eMPEG4Part2:
            BXVD_DBG_MSG(hXvd, ("Still Protocol Memory Config: MPEG4"));
            eVideoProtocol = BXVD_VideoProtocol_eMPEG4;
            break;

         case BAVC_VideoCompressionStd_eAVS:
            BXVD_DBG_MSG(hXvd, ("Still Protocol Memory Config: AVS"));
            eVideoProtocol = BXVD_VideoProtocol_eAVS;
            break;

         case BAVC_VideoCompressionStd_eVP6:
         case BAVC_VideoCompressionStd_eVP7:
         case BAVC_VideoCompressionStd_eVP8:
            BXVD_DBG_MSG(hXvd, ("Still Protocol Memory Config: VP8"));
            eVideoProtocol = BXVD_VideoProtocol_eVP8;
            break;

#if BXVD_P_HVD_PRESENT
         case BAVC_VideoCompressionStd_eH265:
         case BAVC_VideoCompressionStd_eVP9:
            BXVD_DBG_MSG(hXvd, ("Still Protocol Memory Config: HEVC"));
            eVideoProtocol = BXVD_VideoProtocol_eHEVC;
            break;
#endif
         default:
            BXVD_DBG_WRN(hXvd, ("Still Protocol: Unknown - Defaulting to AVC Memory Config!"));
            eVideoProtocol = BXVD_VideoProtocol_eAVC;
            break;
      }

      if (eDecodeResolution == BXVD_DecodeResolution_eHD)
      {
         BXVD_DBG_MSG(hXvd, ("Still Resolution: HD"));
      }
      else if (eDecodeResolution == BXVD_DecodeResolution_eSD)
      {
         BXVD_DBG_MSG(hXvd, ("Still Resolution: SD"));
      }
      else
      {
         BXVD_DBG_MSG(hXvd, ("Still Resolution: 4K"));
      }

      if (eVideoProtocol != BXVD_VideoProtocol_eHEVC)
      {
         bNonHevcProtocolPresent = true;
      }

#if BXVD_P_HVD_PRESENT
      if (eVideoProtocol == BXVD_VideoProtocol_eHEVC)
      {
         if (eDecodeResolution == BXVD_DecodeResolution_e4K)
         {
            temp_eDecodeResolution = BXVD_DecodeResolution_eSD+1;
         }
         else
         {
            temp_eDecodeResolution = eDecodeResolution;
         }

         if (sChannelStillFWMemCfg_HEVC[temp_eDecodeResolution].general_memory_size > genMemReq)
         {
            genMemReq = sChannelStillFWMemCfg_HEVC[temp_eDecodeResolution].general_memory_size;
         }

         if (sChannelStillFWMemCfg_HEVC[temp_eDecodeResolution].inner_loop_wl_size > innerLoopWorklistMemReq)
         {
            innerLoopWorklistMemReq = sChannelStillFWMemCfg_HEVC[temp_eDecodeResolution].inner_loop_wl_size;
         }

         cabacMemSizeLookedUp = sChannelStillFWMemCfg_HEVC[temp_eDecodeResolution].cabac_bin_size;
         cabacWorklistMemSizeLookedUp = sChannelStillFWMemCfg_HEVC[temp_eDecodeResolution].cabac_wl_size;

         if (sChannelStillFWMemCfg_HEVC[temp_eDecodeResolution].direct_mode_size > directModeMemReq)
         {
            /* Align memory size to 256 bytes */
            directModeMemReq = ((sChannelStillFWMemCfg_HEVC[temp_eDecodeResolution].direct_mode_size + 255) / 256) * 256;
         }
      }
      else /* Not HEVC */
#endif
      {
         if ( eDecodeResolution == BXVD_DecodeResolution_e4K)
         {
            temp_eDecodeResolution = BXVD_DecodeResolution_eHD;
         }
         else
         {
            temp_eDecodeResolution = eDecodeResolution;
         }

         if (sChannelStillFWMemCfg[eVideoProtocol][temp_eDecodeResolution].general_memory_size > genMemReq)
         {
            genMemReq = sChannelStillFWMemCfg[eVideoProtocol][temp_eDecodeResolution].general_memory_size;
         }

         if (sChannelStillFWMemCfg[eVideoProtocol][temp_eDecodeResolution].inner_loop_wl_size > innerLoopWorklistMemReq)
         {
            innerLoopWorklistMemReq = sChannelStillFWMemCfg[eVideoProtocol][temp_eDecodeResolution].inner_loop_wl_size;
         }

         cabacMemSizeLookedUp = sChannelStillFWMemCfg[eVideoProtocol][temp_eDecodeResolution].cabac_bin_size;
         cabacWorklistMemSizeLookedUp = sChannelStillFWMemCfg[eVideoProtocol][temp_eDecodeResolution].cabac_wl_size;

         if (sChannelStillFWMemCfg[eVideoProtocol][temp_eDecodeResolution].direct_mode_size > directModeMemReq)
         {
            /* Align memory size to 256 bytes */
            directModeMemReq = ((sChannelStillFWMemCfg[eVideoProtocol][temp_eDecodeResolution].direct_mode_size + 255) / 256) * 256;
         }

         /* Handle Still Picture Compatibility Mode */
         if (hXvd->bStillPictureCompatibilityMode &&
             (eVideoProtocol == BXVD_VideoProtocol_eMPEG2))
         {
            /* In still picture compatibility mode, to reduce memory
               requirements, we do not allocate a separate cabac.
               Instead, we pass in the CDB as the cabac buffer. */
            cabacMemSizeLookedUp = 0;
         }
      }

      if ((cabacMemSizeLookedUp + cabacWorklistMemSizeLookedUp) >
          (cabacMemReq + cabacWorklistMemReq))
      {
         cabacMemReq = cabacMemSizeLookedUp;
         cabacWorklistMemReq = cabacWorklistMemSizeLookedUp;
      }

      /* Check bounds of stripe multiple and stripe width */
      if (hXvd->uiDecode_StripeMultiple >= BXVD_P_STRIPE_MULTIPLE_NUM)
      {
         BXVD_DBG_ERR(hXvd, ("Unsupported stripe multiple: %d", hXvd->uiDecode_StripeMultiple));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
      }

      if (hXvd->uiDecode_StripeWidth >= BXVD_P_STRIPE_WIDTH_NUM)
      {
         BXVD_DBG_ERR(hXvd, ("Unsupported stripe width: %d", hXvd->uiDecode_StripeWidth));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
      }

      if (eVideoProtocol != BXVD_VideoProtocol_eHEVC)
      {
         vidBlbSzIndx = sChannelStillFWMemCfg[eVideoProtocol][temp_eDecodeResolution].video_block_size_index;

         BXVD_P_GET_BUFFER_ATOM_SIZE(hXvd, pChSettings, vidBlbSzIndx, &lumaVidBlkSizeLookedUp, &chromaVidBlkSizeLookedUp);

         if ( ((lumaVidBlkSizeLookedUp+chromaVidBlkSizeLookedUp)*
               sChannelStillFWMemCfg[eVideoProtocol][temp_eDecodeResolution].video_block_count) >
              (lumaVidBlkSizeReq * vidBlkCountReq))
         {
            lumaVidBlkSizeReq = lumaVidBlkSizeLookedUp + chromaVidBlkSizeLookedUp;
            vidBlkCountReq =  sChannelStillFWMemCfg[eVideoProtocol][temp_eDecodeResolution].video_block_count;
         }
      }
#if BXVD_P_HVD_PRESENT
      else /* HEVC protocol */
      {
         vidBlbSzIndx = sChannelStillFWMemCfg_HEVC[temp_eDecodeResolution].video_block_size_index;

         BXVD_P_GET_BUFFER_ATOM_SIZE(hXvd, pChSettings, vidBlbSzIndx, &lumaVidBlkSizeLookedUp, &chromaVidBlkSizeLookedUp);

         if ( (lumaVidBlkSizeLookedUp *sChannelStillFWMemCfg_HEVC[temp_eDecodeResolution].video_block_count) >
              (lumaVidBlkSizeReq * vidBlkCountReq))
         {
            lumaVidBlkSizeReq = lumaVidBlkSizeLookedUp;
            vidBlkCountReq =  sChannelStillFWMemCfg_HEVC[temp_eDecodeResolution].video_block_count;
         }

         if ( (chromaVidBlkSizeLookedUp * sChannelStillFWMemCfg_HEVC[temp_eDecodeResolution].video_block_count) >
              (chromaVidBlkSizeReq * vidBlkCountReq))
         {
            chromaVidBlkSizeReq = chromaVidBlkSizeLookedUp;
         }
      }
#endif
   }

#if BXVD_P_PPB_EXTENDED
   /* Context memory needs to be increased by 32k when using extended PPB */
   genMemReq += (32 * 1024);
#endif

   if ((pChSettings->bSplitPictureBuffersEnable == true) &&
       (bNonHevcProtocolPresent == true) && (eDecodeResolution != BXVD_DecodeResolution_e4K))
   {
      pstDecodeFWMemSize->uiFWPicLumaBlockSize = lumaVidBlkSizeReq + chromaVidBlkSizeReq;
      pstDecodeFWMemSize->uiFWPicChromaBlockSize = chromaVidBlkSizeReq;
   }
   else if (pChSettings->bSplitPictureBuffersEnable == true)
   {
      pstDecodeFWMemSize->uiFWPicLumaBlockSize = lumaVidBlkSizeReq;
      pstDecodeFWMemSize->uiFWPicChromaBlockSize = chromaVidBlkSizeReq;
   }
   else
   {
      pstDecodeFWMemSize->uiFWPicLumaBlockSize = lumaVidBlkSizeReq + chromaVidBlkSizeReq;
      pstDecodeFWMemSize->uiFWPicChromaBlockSize = 0;
   }

   pstDecodeFWMemSize->uiFWPicBlockCount = vidBlkCountReq;

   pstDecodeFWMemSize->uiFWContextSize = genMemReq;
   pstDecodeFWMemSize->uiFWCabacSize = cabacMemReq + 256; /* Add 256 to make sure we get next CABAC buffer at 256 alignment */

   pstDecodeFWMemSize->uiFWCabacWorklistSize = cabacWorklistMemReq + 128; /* Add padding for burst read length */;
   pstDecodeFWMemSize->uiFWDirectModeSize = directModeMemReq;
   pstDecodeFWMemSize->uiFWInnerLoopWorklistSize = innerLoopWorklistMemReq;

   BXVD_DBG_MSG(hXvd, ("ContextSize:0x%x (%d)", genMemReq, genMemReq));
   BXVD_DBG_MSG(hXvd, ("cabacMemReq:0x%x (%d)", pstDecodeFWMemSize->uiFWCabacSize, pstDecodeFWMemSize->uiFWCabacSize));

   BXVD_DBG_MSG(hXvd, ("LumaVidBlockSize:0x%x (%d) , BlkCnt:%d",
                       pstDecodeFWMemSize->uiFWPicLumaBlockSize, pstDecodeFWMemSize->uiFWPicLumaBlockSize, vidBlkCountReq));

   BXVD_DBG_MSG(hXvd, ("ChromaVidBlockSize:0x%x (%d) , BlkCnt:%d",
                       pstDecodeFWMemSize->uiFWPicChromaBlockSize, pstDecodeFWMemSize->uiFWPicChromaBlockSize, vidBlkCountReq));

   BXVD_DBG_MSG(hXvd, ("cabacWorklistMemReq:0x%x (%d)", pstDecodeFWMemSize->uiFWCabacWorklistSize, pstDecodeFWMemSize->uiFWCabacWorklistSize));
   BXVD_DBG_MSG(hXvd, ("directModeMemSize: 0x%x (%d)", directModeMemReq, directModeMemReq));
   BXVD_DBG_MSG(hXvd, ("innerLoopWorklistSize: 0x%x (%d)", innerLoopWorklistMemReq, innerLoopWorklistMemReq));

   return BERR_TRACE(BERR_SUCCESS);
}

BERR_Code BXVD_P_AllocateFWMem(BXVD_Handle hXvd,
                               BXVD_ChannelHandle hXvdCh,
                               BXVD_P_DecodeFWMemSize  *pstDecodeFWMemSize,
                               BXVD_P_DecodeFWBaseAddrs *pstDecodeFWBaseAddrs)
{
   uint32_t uiFWGenMemSize;

   uint32_t uiFWPicMemSize; /* Could be size of combined Luma and Chroma size, or just Luma size */

   uint32_t uiFWPicLumaMemSize;
   uint32_t uiFWPicChromaMemSize;

   uint32_t uiFWCabacMemSize;  /* This is FW used size, must be smaller than actual */
   uint32_t uiFWCabacMemSizeAllocated; /* This is actual size that is allocated */

   unsigned long ulMemBlockAddr;

   BERR_Code rc = BERR_SUCCESS;

   pstDecodeFWBaseAddrs->uiFWContextBase = 0;
   pstDecodeFWBaseAddrs->uiFWPicBase = 0;
   pstDecodeFWBaseAddrs->uiFWPicBase1 = 0;
   pstDecodeFWBaseAddrs->uiFWCabacBase = 0;
   pstDecodeFWBaseAddrs->uiFWCabacWorklistBase = 0;
   pstDecodeFWBaseAddrs->uiFWInterLayerPicBase = 0;
   pstDecodeFWBaseAddrs->uiFWInterLayerMVBase = 0;

   BXVD_DBG_MSG(hXvdCh, ("SubGenMem : 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)hXvd->SubGenMem));
   BXVD_DBG_MSG(hXvdCh, ("SubSecureMem : 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)hXvd->SubSecureMem));
   BXVD_DBG_MSG(hXvdCh, ("SubPicMem : 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)hXvd->SubPicMem));
   BXVD_DBG_MSG(hXvdCh, ("Context Size: %d, Inner Loop Work list size: %d",
                         pstDecodeFWMemSize->uiFWContextSize,
                         pstDecodeFWMemSize->uiFWInnerLoopWorklistSize));

   BXVD_DBG_MSG(hXvdCh, ("Cabac Size: %d, Direct mode Size: %d, Cabac Worklist Size: %d",
                         pstDecodeFWMemSize->uiFWCabacSize, pstDecodeFWMemSize->uiFWDirectModeSize, pstDecodeFWMemSize->uiFWCabacWorklistSize));

   BXVD_DBG_MSG(hXvdCh, ("Luma Block Size: %d, Block Count: %d",
                         pstDecodeFWMemSize->uiFWPicLumaBlockSize, pstDecodeFWMemSize->uiFWPicBlockCount));
   BXVD_DBG_MSG(hXvdCh, ("Chroma Block Size: %d, Block Count: %d",
                         pstDecodeFWMemSize->uiFWPicChromaBlockSize, pstDecodeFWMemSize->uiFWPicBlockCount));

   BXVD_DBG_MSG(hXvdCh, ("Inter Layer Pic Size: %d", pstDecodeFWMemSize->uiFWInterLayerPicSize));
   BXVD_DBG_MSG(hXvdCh, ("Inter Layer MV Size: %d", pstDecodeFWMemSize->uiFWInterLayerMVSize));


   /* Initialize FW Gen Mem Size to contain at least the context and IL Work list buffers */
   uiFWGenMemSize = pstDecodeFWMemSize->uiFWContextSize + pstDecodeFWMemSize->uiFWInnerLoopWorklistSize + pstDecodeFWMemSize->uiFWInterLayerMVSize;

   uiFWPicLumaMemSize = pstDecodeFWMemSize->uiFWPicLumaBlockSize * pstDecodeFWMemSize->uiFWPicBlockCount;
   uiFWPicChromaMemSize = pstDecodeFWMemSize->uiFWPicChromaBlockSize * pstDecodeFWMemSize->uiFWPicBlockCount;

   uiFWCabacMemSizeAllocated = pstDecodeFWMemSize->uiFWCabacSize + pstDecodeFWMemSize->uiFWDirectModeSize +
                               pstDecodeFWMemSize->uiFWCabacWorklistSize;

   uiFWCabacMemSize = pstDecodeFWMemSize->uiFWCabacSize + pstDecodeFWMemSize->uiFWDirectModeSize +
                      pstDecodeFWMemSize->uiFWCabacWorklistSize - 256;

   /* Allocate picture memory from picture heap, if it exists */
   if (hXvdCh->sChSettings.hChannelPictureBlock != 0)
   {
      BXVD_DBG_MSG(hXvdCh, ("Allocating %d bytes of picture/Luma memory from CHANNEL picture heap: 0x%0*lx",
                            uiFWPicLumaMemSize, BXVD_P_DIGITS_IN_LONG, (long)hXvdCh->sChSettings.hChannelPictureBlock ));

      if ( hXvdCh->sChSettings.uiChannelPictureBlockSize < (uiFWPicLumaMemSize + pstDecodeFWMemSize->uiFWInterLayerPicSize))
      {
         BXVD_DBG_ERR(hXvdCh, ("Channel specified picture block size %d too small, size required: %d",
                               hXvdCh->sChSettings.uiChannelPictureBlockSize, (uiFWPicLumaMemSize + pstDecodeFWMemSize->uiFWInterLayerPicSize)));

         return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
      }

      pstDecodeFWBaseAddrs->hFWPicBlock = hXvdCh->sChSettings.hChannelPictureBlock;
      pstDecodeFWBaseAddrs->uiFWPicBase = (uint32_t) BMMA_LockOffset(pstDecodeFWBaseAddrs->hFWPicBlock);

      if (pstDecodeFWBaseAddrs->uiFWPicBase == (uint32_t)0)
      {
         BXVD_DBG_ERR(hXvd, ("Insufficient device memory, picture memory can not be locked!"));
         return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
      }

      hXvdCh->hFWPicMemBlock = hXvdCh->sChSettings.hChannelPictureBlock;
      hXvdCh->uiFWPicOffset = hXvdCh->sChSettings.uiChannelPictureBlockOffset;

      hXvdCh->uiFWPicMemBasePhyAddr =  pstDecodeFWBaseAddrs->uiFWPicBase;

      if ((hXvdCh->sChSettings.hChannelPictureBlock1 != 0) && (hXvdCh->sChSettings.uiChannelPictureBlockSize1 != 0))
      {
         BXVD_DBG_MSG(hXvdCh, ("Allocating %d bytes of Chroma picture memory from CHANNEL picture heap", uiFWPicChromaMemSize));
         pstDecodeFWBaseAddrs->hFWPicBlock1 = hXvdCh->sChSettings.hChannelPictureBlock1;
         pstDecodeFWBaseAddrs->uiFWPicBase1 = (uint32_t) BMMA_LockOffset(pstDecodeFWBaseAddrs->hFWPicBlock1);

         if (pstDecodeFWBaseAddrs->uiFWPicBase1 == (uint32_t)0)
         {
            BXVD_DBG_ERR(hXvd, ("Insufficient device memory, picture memory 1 can not be locked!"));
            return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
         }

         hXvdCh->uiFWPicMem1BasePhyAddr = pstDecodeFWBaseAddrs->uiFWPicBase1;
         hXvdCh->uiFWPicOffset1 = hXvdCh->sChSettings.uiChannelPictureBlockOffset1;
         hXvdCh->hFWPicMem1Block = pstDecodeFWBaseAddrs->hFWPicBlock1;
      }
      else
      {
         pstDecodeFWBaseAddrs->hFWPicBlock1 = 0;
         pstDecodeFWBaseAddrs->uiFWPicBase1 = (uint32_t) 0;
         hXvdCh->uiFWPicMem1BasePhyAddr =  pstDecodeFWBaseAddrs->uiFWPicBase;
         hXvdCh->hFWPicMem1Block = hXvdCh->hFWPicMemBlock;
      }

      /* Heaps not used for channel, so buffers don't need to be freed */
      hXvdCh->hPictureHeap = 0;
      hXvdCh->hPictureHeap1 = 0;

      if (pstDecodeFWMemSize->uiFWInterLayerPicSize != 0)
      {
         pstDecodeFWBaseAddrs->uiFWInterLayerPicBase = hXvdCh->uiFWPicMemBasePhyAddr + uiFWPicLumaMemSize;
      }
   }
   else if (hXvd->SubPicMem != 0)
   {
      if ((hXvd->SubPicMem1 != 0) && (uiFWPicChromaMemSize != 0))
      {
         BXVD_DBG_MSG(hXvdCh, ("Allocating %d bytes of picture Chroma memory from DEVICE picture heap", uiFWPicChromaMemSize));

         pstDecodeFWBaseAddrs->uiFWPicBase1 = (unsigned long)BXVD_P_Memory_Allocate(hXvd->SubPicMem1, uiFWPicChromaMemSize, 12, 0);

         if (pstDecodeFWBaseAddrs->uiFWPicBase1 == (uint32_t)0)
         {
            BXVD_DBG_ERR(hXvdCh, ("Picture memory allocation failure!"));
            return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
         }

         uiFWPicMemSize = uiFWPicLumaMemSize;
      }
      else
      {
         pstDecodeFWBaseAddrs->uiFWPicBase1 =  0;
         uiFWPicMemSize = uiFWPicLumaMemSize + uiFWPicChromaMemSize;
      }

      BXVD_DBG_MSG(hXvdCh, ("Allocating %d bytes of picture memory from DEVICE picture heap", uiFWPicMemSize));

      pstDecodeFWBaseAddrs->uiFWPicBase = (unsigned long)BXVD_P_Memory_Allocate(hXvd->SubPicMem, uiFWPicMemSize, 12, 0);

      if (pstDecodeFWBaseAddrs->uiFWPicBase == 0)
      {
	 BXVD_DBG_ERR(hXvdCh, ("Picture memory allocation failure!"));
	 return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
      }

      if (pstDecodeFWMemSize->uiFWInterLayerPicSize != 0)
      {
         BXVD_DBG_MSG(hXvdCh, ("Allocating %d bytes of picture memory from DEVICE picture heap",
                               pstDecodeFWMemSize->uiFWInterLayerPicSize));

         pstDecodeFWBaseAddrs->uiFWInterLayerPicBase = (unsigned long)BXVD_P_Memory_Allocate(hXvd->SubPicMem, pstDecodeFWMemSize->uiFWInterLayerPicSize, 12, 0);

         if (pstDecodeFWBaseAddrs->uiFWInterLayerPicBase == 0)
         {
            BXVD_DBG_ERR(hXvdCh, ("Picture memory allocation failure!"));
            return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
         }
      }

      hXvdCh->hPictureHeap = hXvd->hPictureHeap;
      hXvdCh->hFWPicMemBlock = hXvd->hFWPicMemBlock;
      hXvdCh->uiFWPicMemBasePhyAddr = hXvd->uiFWPicMemBasePhyAddr;  /* This is physical address of base of block, */

      /* No offset, Pic base addresses point to beginning of buffers */
      hXvdCh->uiFWPicOffset = 0;
      hXvdCh->uiFWPicOffset1 = 0;

      if (hXvd->hFWPicMem1Block == (BMMA_Block_Handle) 0)
      {
         hXvdCh->hFWPicMem1Block = hXvd->hFWPicMemBlock;
         hXvdCh->uiFWPicMem1BasePhyAddr = hXvd->uiFWPicMemBasePhyAddr;
      }
      else
      {
         hXvdCh->hFWPicMem1Block = hXvd->hFWPicMem1Block;
         hXvdCh->uiFWPicMem1BasePhyAddr = hXvd->uiFWPicMem1BasePhyAddr;
      }

      hXvdCh->hPictureHeap1 = hXvd->hPictureHeap1;

#if BXVD_DM_ENABLE_YUV_GRAB_MODE
      hXvdCh->uiFWPicMemBaseVirtAddr = hXvd->uiFWPicMemBaseVirtAddr;
      hXvdCh->uiFWPicMem1BaseVirtAddr = hXvd->uiFWPicMem1BaseVirtAddr;
#endif
   }
   else
   {
      BDBG_ERR(("Picture heap Not specified in BXVD_OpenChannel or at BXVD_Open time"));

      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   /* Allocate cabac from secure heap, if it exists */
   if (uiFWCabacMemSize != 0)
   {
      if (hXvdCh->sChSettings.hChannelCabacBlock != 0)
      {
         BXVD_DBG_MSG(hXvdCh, ("Allocating %d bytes of cabac memory from CHANNEL cabac heap", uiFWCabacMemSize));

         if ( uiFWCabacMemSizeAllocated > hXvdCh->sChSettings.uiChannelCabacBlockSize )
         {
            BDBG_ERR(("Required CABAC size:%d greater than provided heap size:%d",  uiFWCabacMemSizeAllocated, hXvdCh->sChSettings.uiChannelCabacBlockSize ));
            return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
         }

         pstDecodeFWBaseAddrs->hFWCabacBlock = hXvdCh->sChSettings.hChannelCabacBlock;

         pstDecodeFWBaseAddrs->uiFWCabacBase = (uint32_t) BMMA_LockOffset(pstDecodeFWBaseAddrs->hFWCabacBlock);

         if (pstDecodeFWBaseAddrs->uiFWCabacBase == (uint32_t)0)
         {
            BXVD_DBG_ERR(hXvd, ("Insufficient device memory, CABAC memory can not be locked!"));
            return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
         }

         pstDecodeFWBaseAddrs->uiFWCabacBase += hXvdCh->sChSettings.uiChannelCabacBlockOffset;

         /* Assign Direct mode buffer */
         if (pstDecodeFWMemSize->uiFWDirectModeSize != 0)
         {
            pstDecodeFWBaseAddrs->uiFWDirectModeBase = pstDecodeFWBaseAddrs->uiFWCabacBase + pstDecodeFWMemSize->uiFWCabacSize;
         }

         if ( pstDecodeFWMemSize->uiFWCabacWorklistSize != 0)
         {
            /* Assign CABAC worklist base at end of CABAC buffer */
            pstDecodeFWBaseAddrs->uiFWCabacWorklistBase = pstDecodeFWBaseAddrs->uiFWCabacBase + pstDecodeFWMemSize->uiFWCabacSize +
                                                          pstDecodeFWMemSize->uiFWDirectModeSize;
         }

         hXvdCh->hCabacHeap = 0;

         /* Pre rev K cores CABAC must be in memory lower than 768 MB */
#if !BXVD_P_AVD_ARC600
         if ((pstDecodeFWBaseAddrs->uiFWCabacBase + uiFWCabacMemSize) >= BXVD_P_ARC300_RAM_LIMIT)
         {
            BXVD_DBG_ERR(hXvdCh, ("Cabac buffer (%0x) allocated in memory greater than 768MB!", pstDecodeFWBaseAddrs->uiFWCabacBase));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
         }
#endif
      }
      else
      {
         BXVD_DBG_MSG(hXvdCh, ("Allocating %d bytes of cabac memory from DEVICE cabac heap", uiFWCabacMemSizeAllocated));

         pstDecodeFWBaseAddrs->uiFWCabacBase  = (unsigned long)BXVD_P_Memory_Allocate(hXvd->SubSecureMem, uiFWCabacMemSizeAllocated, 8, 0);

         /* Assign Direct mode buffer */
         if (pstDecodeFWMemSize->uiFWDirectModeSize != 0)
         {
            pstDecodeFWBaseAddrs->uiFWDirectModeBase = pstDecodeFWBaseAddrs->uiFWCabacBase + pstDecodeFWMemSize->uiFWCabacSize;
         }

         if ( pstDecodeFWMemSize->uiFWCabacWorklistSize != 0)
         {
            pstDecodeFWBaseAddrs->uiFWCabacWorklistBase = pstDecodeFWBaseAddrs->uiFWCabacBase + pstDecodeFWMemSize->uiFWCabacSize +
                                                          pstDecodeFWMemSize->uiFWDirectModeSize;
         }
      }

      if (pstDecodeFWBaseAddrs->uiFWCabacBase == 0)
      {
         BXVD_DBG_ERR(hXvdCh, ("Cabac memory allocation failure!"));
         return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
      }

      /* Pre rev K cores CABAC must be in memory lower than 768 MB */
#if !BXVD_P_AVD_ARC600
      if ((pstDecodeFWBaseAddrs->uiFWCabacBase + uiFWCabacMemSize) > BXVD_P_ARC300_RAM_LIMIT)
      {
         BXVD_DBG_ERR(hXvdCh, ("Cabac buffer allocated in memory greater than 768MB!"));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
      }
#endif
      hXvdCh->hCabacHeap = hXvd->hCabacHeap;

      hXvdCh->hFWCabacMemBlock = hXvd->hFWCabacMemBlock;
      hXvdCh->uiFWCabacMemBasePhyAddr = hXvd->uiFWCabacMemBasePhyAddr;
   }

   /* Allocate general heap */
   if (hXvdCh->sChSettings.hChannelGeneralBlock != 0)
   {
      BXVD_DBG_MSG(hXvdCh, ("Allocating %d bytes of system memory from CHANNEL system heap", uiFWGenMemSize));


      pstDecodeFWBaseAddrs->hFWContextBlock = hXvdCh->sChSettings.hChannelGeneralBlock;

      hXvdCh->hFWGenMemBlock = pstDecodeFWBaseAddrs->hFWContextBlock;
      hXvdCh->hGeneralHeap = 0;

      ulMemBlockAddr = (unsigned long) BMMA_LockOffset(pstDecodeFWBaseAddrs->hFWContextBlock);

      if (ulMemBlockAddr == (uint32_t)0)
      {
         BXVD_DBG_ERR(hXvd, ("Insufficient device memory, general memory can not be locked!"));
         return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
      }

      ulMemBlockAddr += hXvdCh->sChSettings.uiChannelGeneralBlockOffset;

      hXvdCh->uiFWGenMemBasePhyAddr = ulMemBlockAddr;

      ulMemBlockAddr = (unsigned long) BMMA_Lock(pstDecodeFWBaseAddrs->hFWContextBlock) + hXvdCh->sChSettings.uiChannelGeneralBlockOffset;

      if (ulMemBlockAddr == (uint32_t)0)
      {
         BXVD_DBG_ERR(hXvd, ("Insufficient device memory, AVD FW context memory can not be locked!"));
         return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
      }

      hXvdCh->uiFWGenMemBaseVirtAddr = ulMemBlockAddr;

#if !BXVD_P_FW_HIM_API
      hXvdCh->uiFWGenMemBaseUncachedVirtAddr = (uint32_t)BMMA_GetUncached(pstDecodeFWBaseAddrs->hFWContextBlock) + hXvdCh->sChSettings.uiChannelGeneralBlockOffset;
#endif
   }
   else
   {
      BXVD_DBG_MSG(hXvdCh, ("Allocating %d bytes of system memory from DEVICE system heap", uiFWGenMemSize));

      ulMemBlockAddr = (unsigned long)BXVD_P_Memory_Allocate(hXvd->SubGenMem, uiFWGenMemSize, 12, 0);

      if (ulMemBlockAddr == 0)
      {
         BXVD_DBG_ERR(hXvdCh, ("General memory allocation failure!"));
         return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
      }

      /* Pre rev K core general memory must be in region lower than 768 MB */
#if !BXVD_P_AVD_ARC600
      if ((ulMemBlockAddr + uiFWGenMemSize) >= BXVD_P_ARC300_RAM_LIMIT)
      {
         BXVD_DBG_ERR(hXvdCh, ("General AVD Firmware memory (%0lx) allocated in region greater than 768MB!", ulMemBlockAddr));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
      }
#endif

      hXvdCh->hGeneralHeap = hXvd->hGeneralHeap;
      hXvdCh->hFWGenMemBlock = hXvd->hFWGenMemBlock;
      hXvdCh->uiFWGenMemBasePhyAddr = hXvd->uiFWGenMemBasePhyAddr;
      hXvdCh->uiFWGenMemBaseVirtAddr = hXvd->uiFWGenMemBaseVirtAddr;

#if !BXVD_P_FW_HIM_API
      hXvdCh->uiFWGenMemBaseUncachedVirtAddr = (uint32_t)BMMA_GetUncached(hXvd->hFWGenMemBlock);
#endif
   }

   /* Assign context buffer */
   if ((pstDecodeFWMemSize->uiFWContextSize != 0) && (pstDecodeFWBaseAddrs->uiFWContextBase == 0))
   {
      pstDecodeFWBaseAddrs->uiFWContextBase = ulMemBlockAddr;
      ulMemBlockAddr += pstDecodeFWMemSize->uiFWContextSize;

      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }
   }

   /* Assign inner loop work list buffer address */
   if (pstDecodeFWMemSize->uiFWInnerLoopWorklistSize != 0)
   {
      pstDecodeFWBaseAddrs->uiFWInnerLoopWorklistBase = ulMemBlockAddr;
      ulMemBlockAddr += pstDecodeFWMemSize->uiFWInnerLoopWorklistSize;
   }

   if ( pstDecodeFWMemSize->uiFWInterLayerMVSize != 0)
   {
      pstDecodeFWBaseAddrs->uiFWInterLayerMVBase = ulMemBlockAddr;
   }

   BXVD_DBG_MSG(hXvdCh, ("FWContextBase PA: %08x", pstDecodeFWBaseAddrs->uiFWContextBase));
   BXVD_DBG_MSG(hXvdCh, ("FWPicBase PA: %08x", pstDecodeFWBaseAddrs->uiFWPicBase));
   BXVD_DBG_MSG(hXvdCh, ("FWPicBase1 PA: %08x", pstDecodeFWBaseAddrs->uiFWPicBase1));
   BXVD_DBG_MSG(hXvdCh, ("FWCabacBase PA: %08x", pstDecodeFWBaseAddrs->uiFWCabacBase));
   BXVD_DBG_MSG(hXvdCh, ("FWDirectModeBase PA: %08x", pstDecodeFWBaseAddrs->uiFWDirectModeBase));
   BXVD_DBG_MSG(hXvdCh, ("FWInnerLoopWorklistBase PA: %08x", pstDecodeFWBaseAddrs->uiFWInnerLoopWorklistBase));
   BXVD_DBG_MSG(hXvdCh, ("FWInterLayerPicBase PA: %08x", pstDecodeFWBaseAddrs->uiFWInterLayerPicBase));
   BXVD_DBG_MSG(hXvdCh, ("FWInterLayerMVBase PA: %08x", pstDecodeFWBaseAddrs->uiFWInterLayerMVBase));

   return BERR_TRACE(BERR_SUCCESS);
}

void BXVD_P_DetermineChromaBufferBase(BXVD_ChannelHandle hXvdCh,
                                      BXVD_P_PPB_Protocol eProtocol)
{
   BXVD_ChannelSettings *pChSettings = &(hXvdCh->sChSettings);

   if (((eProtocol == BXVD_P_PPB_Protocol_eH265) || (eProtocol == BXVD_P_PPB_Protocol_eVP9)) &&
       (pChSettings->bSplitPictureBuffersEnable == true) &&
       (hXvdCh->stDecodeFWBaseAddrs.uiFWPicBase1 != (uint32_t) 0))
   {
      hXvdCh->uiFWPicChromaBasePhyAddr = hXvdCh->uiFWPicMem1BasePhyAddr;
      hXvdCh->hFWPicChromaMemBlock = hXvdCh->hFWPicMem1Block;
   }
   else
   {
      hXvdCh->uiFWPicChromaBasePhyAddr = hXvdCh->uiFWPicMemBasePhyAddr;
      hXvdCh->hFWPicChromaMemBlock = hXvdCh->hFWPicMemBlock;
   }
}

BERR_Code BXVD_P_FreeFWMem(BXVD_Handle hXvd,
                           BXVD_ChannelHandle hXvdCh,
                           BXVD_P_DecodeFWBaseAddrs *pstDecodeFWBaseAddrs)
{
   unsigned long uiMemBlockAddr;
   unsigned long uiPtr;

   BERR_Code rc = BERR_SUCCESS;

   BXVD_DBG_MSG(hXvdCh, ("SubGenMem : 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)hXvd->SubGenMem));
   BXVD_DBG_MSG(hXvdCh, ("SubSecureMem : 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)hXvd->SubSecureMem));
   BXVD_DBG_MSG(hXvdCh, ("SubPicMem : 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)hXvd->SubPicMem));

   if (pstDecodeFWBaseAddrs->uiFWPicBase)
   {
      BXVD_DBG_MSG(hXvdCh, ("Picture buffers in specified heap, Free sub-allocation\n"));

      if (hXvdCh->sChSettings.hChannelPictureBlock != 0)
      {
         /* Free picture buffer sub-allocated block */
         BMMA_UnlockOffset(pstDecodeFWBaseAddrs->hFWPicBlock, pstDecodeFWBaseAddrs->uiFWPicBase);

         if ((hXvdCh->sChSettings.hChannelPictureBlock1 != 0) && (hXvdCh->sChSettings.uiChannelPictureBlockSize1 != 0))
         {
            /* Free picture buffer sub-allocated block */
            BMMA_UnlockOffset(pstDecodeFWBaseAddrs->hFWPicBlock1, pstDecodeFWBaseAddrs->uiFWPicBase1);
         }
      }
      else if (hXvd->SubPicMem != 0)
      {
         uiPtr = (pstDecodeFWBaseAddrs->uiFWPicBase);
         /* Free picture buffer sub-allocated block */
         rc = BXVD_P_Memory_Free(hXvd->SubPicMem, (void *)uiPtr);

         if ( rc != BERR_SUCCESS)
         {
            BXVD_DBG_ERR(hXvdCh, ("failed to free sub allocated Picture memory"));
            return BERR_TRACE(BERR_LEAKED_RESOURCE);
         }

         if ((hXvd->SubPicMem1 != 0) && (pstDecodeFWBaseAddrs->uiFWPicBase1 != (uint32_t)0))
         {
            /* Free picture buffer sub-allocated block */

            uiPtr = pstDecodeFWBaseAddrs->uiFWPicBase1;

            rc = BXVD_P_Memory_Free(hXvd->SubPicMem1, (void *)uiPtr);

            if ( rc != BERR_SUCCESS)
            {
               BXVD_DBG_ERR(hXvdCh, ("failed to free sub allocated Picture memory"));
               return BERR_TRACE(BERR_LEAKED_RESOURCE);
            }
         }

         /* If interlayer picture memory is allocated, free it */
         if (pstDecodeFWBaseAddrs->uiFWInterLayerPicBase != 0)
         {
            /* Free inter layer picture buffer sub-allocated block */
            uiPtr = pstDecodeFWBaseAddrs->uiFWInterLayerPicBase;

            rc = BXVD_P_Memory_Free(hXvd->SubPicMem, (void *)uiPtr);

            if ( rc != BERR_SUCCESS)
            {
               BXVD_DBG_ERR(hXvdCh, ("failed to free sub allocated Inter Layer Picture memory"));
               return BERR_TRACE(BERR_LEAKED_RESOURCE);
            }
         }
      }
   }

   if (pstDecodeFWBaseAddrs->uiFWCabacBase)
   {
      BXVD_DBG_MSG(hXvdCh, ("Cabac in Secure memory, Free sub-allocation \n"));

      if (hXvdCh->sChSettings.hChannelCabacBlock != 0)
      {
         BMMA_UnlockOffset(pstDecodeFWBaseAddrs->hFWCabacBlock, (pstDecodeFWBaseAddrs->uiFWCabacBase - hXvdCh->sChSettings.uiChannelCabacBlockOffset));
      }
      else if (hXvd->SubSecureMem != 0)
      {
         uiPtr = pstDecodeFWBaseAddrs->uiFWCabacBase;
         rc = BXVD_P_Memory_Free(hXvd->SubSecureMem, (void *)uiPtr);

         if ( rc != BERR_SUCCESS)
         {
            BXVD_DBG_ERR(hXvdCh, ("failed to free sub allocated CABAC memory"));
            return BERR_TRACE(BERR_LEAKED_RESOURCE);
         }
      }
   }

   BXVD_DBG_MSG(hXvdCh, ("Free Context sub-allocation\n"));

   uiMemBlockAddr = pstDecodeFWBaseAddrs->uiFWContextBase;

   if (pstDecodeFWBaseAddrs->uiFWContextBase)
   {
      /* Free context memory, this also could contain the picture and
       * cabac memory if not in their own heaps */
      if (hXvdCh->sChSettings.hChannelGeneralBlock != 0)
      {
         uiMemBlockAddr = pstDecodeFWBaseAddrs->uiFWContextBase - hXvdCh->sChSettings.uiChannelGeneralBlockOffset;

         BMMA_Unlock(pstDecodeFWBaseAddrs->hFWContextBlock, (void *)uiMemBlockAddr);
         BMMA_UnlockOffset(pstDecodeFWBaseAddrs->hFWContextBlock, (hXvdCh->uiFWGenMemBasePhyAddr - hXvdCh->sChSettings.uiChannelGeneralBlockOffset));
      }
      else
      {
         rc = BXVD_P_Memory_Free(hXvd->SubGenMem, (void *)uiMemBlockAddr);

         if (rc != BERR_SUCCESS)
         {
            BXVD_DBG_ERR(hXvdCh, ("failed to free sub allocated Context memory"));
            return BERR_TRACE(BERR_LEAKED_RESOURCE);
         }
      }
   }

   return BERR_TRACE(rc);
}

/*
 * This function maps FW and shared memory, loads the inner and outer loop
 * FW images and kick starts the outer loop ARC.
 */
BERR_Code BXVD_P_ChipInit(BXVD_Handle hXvd, uint32_t uDecoderInstance)
{
   BERR_Code rc;

   rc = BXVD_P_FW_LOAD(hXvd, uDecoderInstance);
   if (rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

   /* Enable decoder */

   BXVD_DBG_MSG(hXvd, ("Enabling chip execution"));

   rc = BXVD_P_CHIP_ENABLE(hXvd);

   if (rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

   BXVD_DBG_MSG(hXvd, ("Shared memory start addr: 0x%lx", hXvd->uiFWGenMemBasePhyAddr));

#if FW_INIT
   rc = BXVD_P_HostCmdSendInit(hXvd,
			       uDecoderInstance,
			       hXvd->stSettings.eRaveEndianess
                               );

   if (rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }
#endif

   return BERR_TRACE(BERR_SUCCESS);
}

/*
 * Reset 740x SDRAM parameters.
 */
BERR_Code BXVD_P_Reset740x(BXVD_Handle  hXvd, uint32_t uDecoderInstance)
{
   BERR_Code rc;

   BSTD_UNUSED(uDecoderInstance);

   /* Reset Video Decoder */
   rc = BXVD_P_RESET_CHIP(hXvd);

   /* AVD has been reset, clear device busy */
   hXvd->stDecoderContext.bIfBusy = 0;

   return BERR_TRACE(rc);
}

BERR_Code BXVD_P_Boot(BXVD_Handle hXvd)
{
   BERR_Code rc;

   /* Reset 740x */
   rc = BERR_TRACE(BXVD_P_Reset740x(hXvd, hXvd->uDecoderInstance));
   if(rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

   /* Initialize interrupts */
   rc = BXVD_P_SetupInterrupts(hXvd);
   if (rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

   /* Initialize the inner and outer loop firmware. */
   rc = BXVD_P_ChipInit(hXvd, hXvd->uDecoderInstance);
   if(rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

#if !BXVD_POLL_FW_MBX
   /* Start the watchdog after the decoder starts */
   rc = BXVD_P_SetupWatchdog(hXvd);
   if (rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }
#endif
   return BERR_TRACE(BERR_SUCCESS);
}

static const BAVC_VideoCompressionStd  StillCmprStdList[] =
{
   BAVC_VideoCompressionStd_eMPEG2
};

/*
 * Description:
 *
 *   When an applications is using the multi-decode API but specifying
 * video decode modes from the single decode api, a channel is opened
 * for still picture decode. This routine opens the channel for MPEG2
 * still picture decoding if XVD has determined still picture compatibility
 * mode is being used.
 */

BERR_Code BXVD_P_SetupStillPictureCompatibilityMode(BXVD_Handle hXvd)
{
   BERR_Code rc;
   BXVD_ChannelHandle hXvdCh;

   /* coverity[var_decl: FALSE] */
   BXVD_ChannelSettings sChSettings;

   uint32_t uiStillChannelNum = BXVD_MAX_VIDEO_CHANNELS - 1;

   if (hXvd->bStillPictureCompatibilityMode)
   {
      if (hXvd->uiOpenChannels >= BXVD_MAX_VIDEO_CHANNELS)
      {
         BXVD_DBG_ERR(hXvd, ("Still Picture Compatibility Mode: No channels available for the implicit creation of still picture channel"));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
      }

      if (hXvd->ahChannel[uiStillChannelNum] != 0)
      {
         BXVD_DBG_ERR(hXvd, ("Still Picture Compatibility Mode: Channel[%d] not available for the implicit creation of still picture channel", uiStillChannelNum));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
      }
      /* coverity[uninit_use_in_call: FALSE] */
      BXVD_GetChannelDefaultSettings(hXvd,
                                     uiStillChannelNum,
                                     &sChSettings);

      /* Settings for MPEG2 Still Decode */
      sChSettings.peVideoCmprStdList = (BAVC_VideoCompressionStd*) &StillCmprStdList;
      sChSettings.uiVideoCmprCount = 1;
      sChSettings.eChannelMode = BXVD_ChannelMode_eStill;

      if (hXvd->bStillHDCapable)
      {
         sChSettings.eDecodeResolution = BXVD_DecodeResolution_eHD;
      }

      else
      {
         sChSettings.eDecodeResolution = BXVD_DecodeResolution_eSD;
      }

      rc = BXVD_OpenChannel(
         hXvd,
         &hXvdCh,
         uiStillChannelNum,
         &sChSettings);

      if (rc != BERR_SUCCESS)
      {
         BXVD_DBG_ERR(hXvd, ("Still Picture Compatibility Mode: Error opening still channel[%d]", uiStillChannelNum));
         return BERR_TRACE(rc);
      }

      BXVD_DBG_WRN(hXvd, ("Still Picture Compatibility Mode: created implicit still picture channel [%d]",
                               hXvdCh->ulChannelNum));
   }

   return BERR_TRACE(BERR_SUCCESS);
}

BERR_Code BXVD_P_TeardownStillPictureCompatibilityMode(BXVD_Handle hXvd)
{
   BERR_Code rc;

   if ((hXvd->bStillPictureCompatibilityMode) && (hXvd->ahChannel != 0))
   {
      if (hXvd->ahChannel[hXvd->uiStillChannelNum] != 0)
      {
         rc = BXVD_CloseChannel(hXvd->ahChannel[hXvd->uiStillChannelNum]);
         if (rc != BERR_SUCCESS)
         {
            BXVD_DBG_ERR(hXvd, ("Still Picture Compatibility Mode: Error closing still channel[%d]", hXvd->uiStillChannelNum));
            return BERR_TRACE(rc);
         }

#if BXVD_P_POWER_MANAGEMENT
         /* Wake up the decoder, may have been put to sleep closing the still channel  */
         BXVD_P_SetHibernateState(hXvd, false);
#endif
      }
   }
   return BERR_TRACE(BERR_SUCCESS);
}

BERR_Code BXVD_P_InitChannel(BXVD_ChannelHandle  hXvdCh)
{
   BXDM_Decoder_Interface stDecoderInterface;
   void *pPrivateContext;

   BXVD_3DSetting st3DSettings;

   BXVD_Handle hXvd;

   BERR_Code rc;

   hXvd = hXvdCh->pXvd;

   /* Initialize channel settings */
   hXvdCh->eDecoderState = BXVD_P_DecoderState_eNotActive;
   hXvdCh->bPreserveState = false;

   hXvdCh->eCurrentSkipMode = BXVD_SkipMode_eDecode_IPB;

   rc = BXDM_PictureProvider_SetTimerHandle_isr(hXvdCh->hPictureProvider, hXvd->hTimer);

   if (rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

   /* SW7425-1064: Initialize the XVD decoder.  Previously this was done in BXVD_Decoder_GetDMInterface.*/
   BXVD_Decoder_OpenChannel( hXvdCh );

   rc = BXVD_Decoder_GetDMInterface(
      hXvdCh,
      &stDecoderInterface,
      &pPrivateContext
      );

   if (rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

   rc = BXDM_PictureProvider_SetDecoderInterface_isr(
      hXvdCh->hPictureProvider,
      &stDecoderInterface,
      pPrivateContext);

   if (rc != BERR_SUCCESS)
   {

      return BERR_TRACE(rc);
   }

   /* SW7422-72: set the default 3D behavior to support legacy MVC systems.
    * The original MVC behavior on the DVD chips was to set "pNext" in the MFD structure
    * of frame 0 to point to frame 1.  By setting the default here, the older
    * DVD platforms will get this behavior without having to change any code.
    */
   rc = BXVD_GetDefault3D( hXvdCh, &st3DSettings );

   if (rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

   rc = BXVD_Set3D_isr( hXvdCh, &st3DSettings );

   if (rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

   rc = BXVD_SetPictureDropMode_isr(hXvdCh, hXvdCh->sChSettings.ePictureDropMode );

   if (rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

   rc = BXVD_SetPtsStcDiffThreshold_isr(hXvdCh, hXvdCh->sChSettings.uiVsyncDiffThreshDefault);

   if (rc != BERR_SUCCESS)
   {
      BXVD_CloseChannel(hXvdCh);
      return BERR_TRACE(rc);
   }

   rc = BXVD_SetMonitorRefreshRate_isr(hXvdCh, hXvdCh->sChSettings.ui32MonitorRefreshRate);

   if (rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

   rc = BXVD_Set1080pScanMode_isr(hXvdCh, hXvdCh->sChSettings.e1080pScanMode);

   if (rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

   return rc;
}

BERR_Code BXVD_P_SetupInterrupts( BXVD_Handle hXvd)
{
   BERR_Code rc;

   BDBG_ASSERT(hXvd);

   BDBG_ENTER(BXVD_P_SetupInterrupts);

   /* Disable Interrupts, may already be crerted */
   rc = BXVD_P_DisableInterrupts(hXvd);
   if (rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

   /* Create interrupt callbacks if not already created */
   rc = BXVD_P_CreateInterrupts(hXvd);
   if (rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

   /* Enable the interrupt */
   rc = BXVD_P_EnableInterrupts(hXvd);
   if (rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

   BDBG_LEAVE(BXVD_P_SetupInterrupts);

   return BERR_TRACE(rc);
}

BERR_Code BXVD_P_CreateInterrupts( BXVD_Handle hXvd)
{
   BERR_Code rc = BERR_SUCCESS;

#if ( BXVD_P_AVD_ARC600 && BXVD_P_SVD_PRESENT )
   bool bBLDPresent = false;
#endif

   BDBG_ASSERT(hXvd);

   BDBG_ENTER(BXVD_P_CreateInterrupts);

/* If interrupts are not being used, return */
#if BXVD_POLL_FW_MBX

   return BERR_TRACE(rc);

#else

#if ( BXVD_P_AVD_ARC600 && BXVD_P_SVD_PRESENT )
   /* Determine if Base Layer ARC is present in this decoder */
   if ( (BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_CPUId))
        & hXvd->stPlatformInfo.stReg.uiDecode_CPUId_BldIdMask)
   {
      bBLDPresent = true;
   }
#endif

   if (hXvd->stDecoderContext.pCbAVC_MBX_ISR == 0)
   {
      rc = BINT_CreateCallback(&(hXvd->stDecoderContext.pCbAVC_MBX_ISR),
                               hXvd->hInterrupt,
                               hXvd->stPlatformInfo.stReg.uiInterrupt_Mailbox,
                               BXVD_P_AVD_MBX_isr,
                               (void*)(&hXvd->stDecoderContext),
                               0);

      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }
   }

   if (hXvd->stDecoderContext.pCbAVC_StillPicRdy_ISR == 0)
   {
      rc = BINT_CreateCallback(&(hXvd->stDecoderContext.pCbAVC_StillPicRdy_ISR),
                               hXvd->hInterrupt,
                               hXvd->stPlatformInfo.stReg.uiInterrupt_StillPictureRdy,
                               BXVD_P_AVD_StillPictureRdy_isr,
                               (void*)(&hXvd->stDecoderContext),
                               0);
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }
   }


   /* See if the interrupt callbacks already exist*/
   if (hXvd->stDecoderContext.pCbAVC_VICHReg_ISR == 0)
   {
      /* Interrupts callbacks need to be created */
      rc = BINT_CreateCallback(&(hXvd->stDecoderContext.pCbAVC_VICHReg_ISR),
                               hXvd->hInterrupt,
                               hXvd->stPlatformInfo.stReg.uiInterrupt_VICReg,
                               BXVD_P_VidInstrChkr_isr,
                               (void*)(hXvd),
                               BXVD_VICHInt_eRegAccess);
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }

      rc = BINT_CreateCallback(&(hXvd->stDecoderContext.pCbAVC_VICHSCB_ISR),
                               hXvd->hInterrupt,
                               hXvd->stPlatformInfo.stReg.uiInterrupt_VICSCBWr,
                               BXVD_P_VidInstrChkr_isr,
                               (void*)(hXvd),
                               BXVD_VICHInt_eSCBWrite);
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }

      rc = BINT_CreateCallback(&(hXvd->stDecoderContext.pCbAVC_VICHInstrRd_ISR),
                               hXvd->hInterrupt,
                               hXvd->stPlatformInfo.stReg.uiInterrupt_VICInstrRd,
                               BXVD_P_VidInstrChkr_isr,
                               (void*)(hXvd),
                               BXVD_VICHInt_eInstrRead);
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }

#if BXVD_P_AVD_ARC600
      rc = BINT_CreateCallback(&(hXvd->stDecoderContext.pCbAVC_VICHILInstrRd_ISR),
                               hXvd->hInterrupt,
                               hXvd->stPlatformInfo.stReg.uiInterrupt_VICILInstrRd,
                               BXVD_P_VidInstrChkr_isr,
                               (void*)(hXvd),
                               BXVD_VICHInt_eInstrRead);
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }

#if BXVD_P_SVD_PRESENT
      if (bBLDPresent == true)
      {
         rc = BINT_CreateCallback(&(hXvd->stDecoderContext.pCbAVC_VICHBLInstrRd_ISR),
                                  hXvd->hInterrupt,
                                  hXvd->stPlatformInfo.stReg.uiInterrupt_VICBLInstrRd,
                                  BXVD_P_VidInstrChkr_isr,
                                  (void*)(hXvd),
                                  BXVD_VICHInt_eInstrRead);
         if (rc != BERR_SUCCESS)
         {
            return BERR_TRACE(rc);
         }
      }
#endif
#endif
   }

   if (hXvd->stDecoderContext.pCbAVC_StereoSeqError_ISR == 0)
   {
      rc = BINT_CreateCallback(&(hXvd->stDecoderContext.pCbAVC_StereoSeqError_ISR),
                               hXvd->hInterrupt,
                               hXvd->stPlatformInfo.stReg.uiInterrupt_StereoSeqError,
                               BXVD_P_StereoSeqError_isr,
                               (void*)(hXvd),
                               0);
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }
   }

   BDBG_LEAVE(BXVD_P_CreateInterrupts);

   return BERR_TRACE(rc);
#endif
}


BERR_Code BXVD_P_DisableInterrupts( BXVD_Handle hXvd)
{
   BXVD_DisplayInterrupt eDisplayInterrupt;

   BERR_Code rc = BERR_SUCCESS;

#if ( BXVD_P_AVD_ARC600 && BXVD_P_SVD_PRESENT )
   bool bBLDPresent = false;
#endif

   BDBG_ASSERT(hXvd);

   BDBG_ENTER(BXVD_DisableInterrupts);

#if ( BXVD_P_AVD_ARC600 && BXVD_P_SVD_PRESENT )

   /* Determine if Base Layer ARC is present in this decoder */
   if (hXvd->stPlatformInfo.stReg.uiDecode_CPUId)
   {
      if ( (BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_CPUId))
           & hXvd->stPlatformInfo.stReg.uiDecode_CPUId_BldIdMask)
      {
         bBLDPresent = true;
      }
   }
#endif

   if (hXvd->stDecoderContext.pCbAVC_MBX_ISR)
   {
      rc = BINT_DisableCallback(hXvd->stDecoderContext.pCbAVC_MBX_ISR);
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }
   }

   if (hXvd->stDecoderContext.pCbAVC_StillPicRdy_ISR)
   {
      rc = BINT_DisableCallback(hXvd->stDecoderContext.pCbAVC_StillPicRdy_ISR);
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }
   }

   /* Video Instruction Checker interrupts */

   /* See if the interrupt callbacks already exist*/
   if (hXvd->stDecoderContext.pCbAVC_VICHReg_ISR)
   {
      /* Disable interrupts */

      /* VICH Register acccess address violation */
      rc = BINT_DisableCallback(hXvd->stDecoderContext.pCbAVC_VICHReg_ISR);
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }

      /* VICH SCB write address violation */
      rc = BINT_DisableCallback(hXvd->stDecoderContext.pCbAVC_VICHSCB_ISR);
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }

      /* VICH Instruction read violation */
      rc = BINT_DisableCallback(hXvd->stDecoderContext.pCbAVC_VICHInstrRd_ISR);
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }

#if BXVD_P_AVD_ARC600
      /* VICH Inner Loop Instruction read violation */
      rc = BINT_DisableCallback(hXvd->stDecoderContext.pCbAVC_VICHILInstrRd_ISR);
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }

#if BXVD_P_SVD_PRESENT
      if (bBLDPresent == true)
      {
         /* VICH Base Layer Instruction read violation */
         rc = BINT_DisableCallback(hXvd->stDecoderContext.pCbAVC_VICHBLInstrRd_ISR);
         if (rc != BERR_SUCCESS)
         {
            return BERR_TRACE(rc);
         }
      }
#endif
#endif
   }

   if (hXvd->stDecoderContext.pCbAVC_StereoSeqError_ISR)
   {
      rc = BINT_DisableCallback(hXvd->stDecoderContext.pCbAVC_StereoSeqError_ISR);
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }
   }

   for ( eDisplayInterrupt = 0; eDisplayInterrupt < BXVD_DisplayInterrupt_eMax; eDisplayInterrupt++ )
   {
      if (hXvd->hXvdDipCh[eDisplayInterrupt] )
      {
         rc = BXVD_DisplayInterruptProvider_P_DisableInterrupts( hXvd->hXvdDipCh[eDisplayInterrupt] );

         if (rc != BERR_SUCCESS)
         {
            return BERR_TRACE(rc);
         }
      }
   }

   BDBG_LEAVE(BXVD_P_DisableInterrupts);

   return BERR_TRACE(rc);
}

BERR_Code BXVD_P_EnableInterrupts( BXVD_Handle hXvd)
{
   BXVD_DisplayInterrupt eDisplayInterrupt;

   BERR_Code rc = BERR_SUCCESS;

#if ( BXVD_P_AVD_ARC600 && BXVD_P_SVD_PRESENT )
   bool bBLDPresent = false;
#endif

   BDBG_ASSERT(hXvd);

   BDBG_ENTER(BXVD_P_EnaInterrupts);

#if ( BXVD_P_AVD_ARC600 && BXVD_P_SVD_PRESENT )
   if (hXvd->stPlatformInfo.stReg.uiDecode_CPUId)
   {
      /* Determine if Base Layer ARC is present in this decoder */
      if ( (BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_CPUId))
           & hXvd->stPlatformInfo.stReg.uiDecode_CPUId_BldIdMask)
      {
         bBLDPresent = true;
      }
   }
#endif

   if (hXvd->stDecoderContext.pCbAVC_MBX_ISR)
   {
      rc = BINT_EnableCallback(hXvd->stDecoderContext.pCbAVC_MBX_ISR);
      if (rc != BERR_SUCCESS )
      {
         return BERR_TRACE(rc);
      }
   }

   if (hXvd->stDecoderContext.pCbAVC_StillPicRdy_ISR)
   {

      rc = BINT_EnableCallback(hXvd->stDecoderContext.pCbAVC_StillPicRdy_ISR);
      if (rc != BERR_SUCCESS )
      {
         return BERR_TRACE(rc);
      }
   }

   /* Video Instruction Checker interrupts */

   /* See if the interrupt callbacks already exist*/
   if (hXvd->stDecoderContext.pCbAVC_VICHReg_ISR)
   {
      /* If callback is valid, re-enable interrupt */
      if (hXvd->stDeviceInterruptCallbackInfo[BXVD_DeviceInterrupt_eVidInstrChecker].BXVD_P_pAppIntCallbackPtr)
      {
         rc = BINT_EnableCallback(hXvd->stDecoderContext.pCbAVC_VICHReg_ISR);
         if (rc != BERR_SUCCESS)
         {
            return BERR_TRACE(rc);
         }

         rc = BINT_EnableCallback(hXvd->stDecoderContext.pCbAVC_VICHSCB_ISR);
         if (rc != BERR_SUCCESS)
         {
            return BERR_TRACE(rc);
         }

         rc = BINT_EnableCallback(hXvd->stDecoderContext.pCbAVC_VICHInstrRd_ISR);
         if (rc != BERR_SUCCESS)
         {
            return BERR_TRACE(rc);
         }

#if BXVD_P_AVD_ARC600
         rc = BINT_EnableCallback(hXvd->stDecoderContext.pCbAVC_VICHILInstrRd_ISR);
         if (rc != BERR_SUCCESS)
         {
            return BERR_TRACE(rc);
         }

#if BXVD_P_SVD_PRESENT
         if (bBLDPresent == true)
         {
            rc = BINT_EnableCallback(hXvd->stDecoderContext.pCbAVC_VICHBLInstrRd_ISR);
            if (rc != BERR_SUCCESS)
            {
               return BERR_TRACE(rc);
            }
         }
#endif
#endif
      }
   }
   if (hXvd->stDecoderContext.pCbAVC_StereoSeqError_ISR)
   {
      rc = BINT_EnableCallback(hXvd->stDecoderContext.pCbAVC_StereoSeqError_ISR);
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }
   }

   for ( eDisplayInterrupt = 0; eDisplayInterrupt < BXVD_DisplayInterrupt_eMax; eDisplayInterrupt++ )
   {
      if (hXvd->hXvdDipCh[eDisplayInterrupt] )
      {
         rc = BXVD_DisplayInterruptProvider_P_EnableInterrupts( hXvd->hXvdDipCh[eDisplayInterrupt] );

         if (rc != BERR_SUCCESS)
         {
            return BERR_TRACE(rc);
         }
      }
   }

   BDBG_LEAVE(BXVD_P_EnableInterrupts);

   return BERR_TRACE(rc);
}



BERR_Code BXVD_P_DestroyInterrupts(BXVD_Handle hXvd)
{
   BXVD_DisplayInterrupt eDisplayInterrupt;

   BERR_Code rc = BERR_SUCCESS;

   if (hXvd->stDecoderContext.pCbAVC_MBX_ISR)
   {
      rc = BINT_DestroyCallback(hXvd->stDecoderContext.pCbAVC_MBX_ISR);
      if (BERR_SUCCESS != rc)
      {
         return BERR_TRACE(rc);
      }
   }

   if (hXvd->stDecoderContext.pCbAVC_StillPicRdy_ISR)
   {
      rc = BINT_DestroyCallback(hXvd->stDecoderContext.pCbAVC_StillPicRdy_ISR);
      if (BERR_SUCCESS != rc)
      {
         return BERR_TRACE(rc);
      }
   }

   if (hXvd->stDecoderContext.pCbAVC_Watchdog_ISR)
   {
      rc = BINT_DestroyCallback(hXvd->stDecoderContext.pCbAVC_Watchdog_ISR);
      if (BERR_SUCCESS != rc)
      {
         return BERR_TRACE(rc);
      }
   }

   if (hXvd->stDecoderContext.pCbAVC_VICHReg_ISR)
   {
      rc = BINT_DestroyCallback(hXvd->stDecoderContext.pCbAVC_VICHReg_ISR);
      if (BERR_SUCCESS != rc)
      {
         return BERR_TRACE(rc);
      }
   }

   if (hXvd->stDecoderContext.pCbAVC_VICHSCB_ISR)
   {
      rc = BINT_DestroyCallback(hXvd->stDecoderContext.pCbAVC_VICHSCB_ISR);
      if (BERR_SUCCESS != rc)
      {
         return BERR_TRACE(rc);
      }
   }

   if (hXvd->stDecoderContext.pCbAVC_VICHInstrRd_ISR)
   {
      rc = BINT_DestroyCallback(hXvd->stDecoderContext.pCbAVC_VICHInstrRd_ISR);
      if (BERR_SUCCESS != rc)
      {
         return BERR_TRACE(rc);
      }
   }

#if BXVD_P_AVD_ARC600
   if (hXvd->stDecoderContext.pCbAVC_VICHILInstrRd_ISR)
   {
      rc = BINT_DestroyCallback(hXvd->stDecoderContext.pCbAVC_VICHILInstrRd_ISR);
      if (BERR_SUCCESS != rc)
      {
         return BERR_TRACE(rc);
      }
   }

   if (hXvd->bSVCCapable == true)
   {
      if (hXvd->stDecoderContext.pCbAVC_VICHBLInstrRd_ISR)
      {
         rc = BINT_DestroyCallback(hXvd->stDecoderContext.pCbAVC_VICHBLInstrRd_ISR);
         if (BERR_SUCCESS != rc)
         {
            return BERR_TRACE(rc);
         }
      }
   }
#endif

   if (hXvd->stDecoderContext.pCbAVC_StereoSeqError_ISR)
   {
      rc = BINT_DestroyCallback(hXvd->stDecoderContext.pCbAVC_StereoSeqError_ISR);
      if (BERR_SUCCESS != rc)
      {
         return BERR_TRACE(rc);
      }
   }

   for ( eDisplayInterrupt = 0; eDisplayInterrupt < BXVD_DisplayInterrupt_eMax; eDisplayInterrupt++ )
   {
      rc = BXVD_DisplayInterruptProvider_P_CloseChannel( hXvd->hXvdDipCh[eDisplayInterrupt] );
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }

      rc = BXDM_DisplayInterruptHandler_Destroy( hXvd->hXdmDih[eDisplayInterrupt] );
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }
   }

   return rc;
}


BERR_Code BXVD_P_SetupWatchdog( BXVD_Handle hXvd)
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_ASSERT(hXvd);

   BDBG_ENTER(BXVD_P_SetupWatchdog);

   if (hXvd->stDecoderContext.pCbAVC_Watchdog_ISR) {
      rc = BINT_DisableCallback(hXvd->stDecoderContext.pCbAVC_Watchdog_ISR);
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }

      if (hXvd->stDeviceInterruptCallbackInfo[BXVD_DeviceInterrupt_eWatchdog].BXVD_P_pAppIntCallbackPtr) {
         rc = BINT_EnableCallback(hXvd->stDecoderContext.pCbAVC_Watchdog_ISR);
         if (rc != BERR_SUCCESS)
         {
            return BERR_TRACE(rc);
         }
      }
   } else {
      rc = BINT_CreateCallback(&(hXvd->stDecoderContext.pCbAVC_Watchdog_ISR),
                               hXvd->hInterrupt,
                               hXvd->stPlatformInfo.stReg.uiInterrupt_OuterWatchdog,
                               BXVD_P_WatchdogInterrupt_isr,
                               (void*)(hXvd),
                               BXVD_P_OUTER_WATCHDOG);
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }
   }

   BXVD_Reg_Read32(hXvd,
                   hXvd->stPlatformInfo.stReg.uiDecode_OuterWatchdogTimer);

   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_OuterInterruptClear,
                    BXVD_P_INTR_CLEAR);

   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_OuterInterruptMask,
                    BXVD_P_INTR_OL_MASK);

   BDBG_LEAVE(BXVD_P_SetupWatchdog);

   return BERR_TRACE(rc);
}

BERR_Code BXVD_P_EnableWatchdog( BXVD_Handle hXvd)
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_ASSERT(hXvd);

   BDBG_ENTER(BXVD_P_EnableWatchdog);

   if ((hXvd->stDecoderContext.pCbAVC_Watchdog_ISR) &&
       (hXvd->stDeviceInterruptCallbackInfo[BXVD_DeviceInterrupt_eWatchdog].BXVD_P_pAppIntCallbackPtr))
   {
      rc = BINT_EnableCallback(hXvd->stDecoderContext.pCbAVC_Watchdog_ISR);
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }
   }

   if (hXvd->stPlatformInfo.stReg.uiDecode_OuterWatchdogTimer)
   {
      BXVD_Reg_Read32(hXvd,
                      hXvd->stPlatformInfo.stReg.uiDecode_OuterWatchdogTimer);

      BXVD_Reg_Write32(hXvd,
                       hXvd->stPlatformInfo.stReg.uiDecode_OuterInterruptClear,
                       BXVD_P_INTR_CLEAR);

      BXVD_Reg_Write32(hXvd,
                       hXvd->stPlatformInfo.stReg.uiDecode_OuterInterruptMask,
                       BXVD_P_INTR_OL_MASK);
   }

   BDBG_LEAVE(BXVD_P_EnableWatchdog);

   return BERR_TRACE(rc);
}

BERR_Code BXVD_P_DisableWatchdog( BXVD_Handle hXvd)
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_ASSERT(hXvd);

   BDBG_ENTER(BXVD_P_DisableWatchdog);

   if (hXvd->stDecoderContext.pCbAVC_Watchdog_ISR)
   {
      rc = BINT_DisableCallback(hXvd->stDecoderContext.pCbAVC_Watchdog_ISR);
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }
   }
   BDBG_LEAVE(BXVD_P_DisableWatchdog);

   return BERR_TRACE(rc);
}

void BXVD_P_FreeXVDContext(BXVD_Handle hXvd)
{
   /* Destroy the FW command event */
   if (hXvd->stDecoderContext.hFWCmdDoneEvent)
   {
      BKNI_DestroyEvent(hXvd->stDecoderContext.hFWCmdDoneEvent);
   }

   /* Free the channel handle array */
   if (hXvd->ahChannel)
   {
      BKNI_Free(hXvd->ahChannel);
   }

   /* Free saved channel context structs */
   BXVD_P_FreeAllocatedChannelHandles(hXvd);

   /* Free the Still Picture Channel Handle */
   if (hXvd->hStillChannel)
   {
      BKNI_Free(hXvd->hStillChannel);
   }

   /* Free the multi-decode picture list */
   if (hXvd->pVDCPictureBuffers)
   {
      BKNI_Free(hXvd->pVDCPictureBuffers);
   }

   /* Destroy Timer */
   if (hXvd->hTimer)
   {
      BTMR_DestroyTimer(hXvd->hTimer);
   }

   /* Set eHandleType to invalid to prevent handle from being used again */
   hXvd->eHandleType = BXVD_P_HandleType_Invalid;

   /* Release the main context */
   BKNI_Free(hXvd);
}


BERR_Code BXVD_P_InitFreeChannelList(BXVD_Handle hXvd)
{
   uint32_t i;
   BXVD_ChannelHandle phXvdCh[BXVD_MAX_VIDEO_CHANNELS];

   BLST_S_INIT(&(hXvd->FreeChannelListHead));

   if ( hXvd->stSettings.uiNumPreAllocateChannels > BXVD_MAX_VIDEO_CHANNELS)
   {
      BXVD_DBG_ERR(hXvd, ("Number of pre-allocated channels greater than max video channels: %d",
                          hXvd->stSettings.uiNumPreAllocateChannels));

      return (BERR_TRACE(BERR_INVALID_PARAMETER));
   }

   /* Pre-allocate channels if specified */
   for (i = 0; i < hXvd->stSettings.uiNumPreAllocateChannels; i++)
   {
      BXVD_P_GetChannelHandle(hXvd, &phXvdCh[i]);

      if (phXvdCh[i] == 0)
      {
         return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
      }
   }

   /* Keep pre-allocaed channels */
   for (i = 0; i < hXvd->stSettings.uiNumPreAllocateChannels; i++)
   {
      BXVD_P_KeepChannelHandle(hXvd, phXvdCh[i]);
   }

   return BERR_SUCCESS;
}

void BXVD_P_GetChannelHandle(BXVD_Handle hXvd,
                             BXVD_ChannelHandle *phXvdCh)
{
   *phXvdCh = BLST_S_FIRST(&(hXvd->FreeChannelListHead));

   /* If free channel in list, use it. If not, allocate another one */
   if (*phXvdCh)
   {
      BLST_S_REMOVE_HEAD(&(hXvd->FreeChannelListHead), link);
   }
   else
   {
      *phXvdCh = (BXVD_P_Channel*)(BKNI_Malloc(sizeof(BXVD_P_Channel)));
   }
}

void BXVD_P_KeepChannelHandle(BXVD_Handle hXvd,
                              BXVD_ChannelHandle hXvdCh)
{
   /* Save channel handle to be used again */
   BLST_S_INSERT_HEAD(&(hXvd->FreeChannelListHead), hXvdCh, link);
}

void BXVD_P_FreeAllocatedChannelHandles(BXVD_Handle hXvd)
{
   BXVD_ChannelHandle hXvdCh;

   while ((hXvdCh = BLST_S_FIRST(&(hXvd->FreeChannelListHead))) != 0)
   {
      BLST_S_REMOVE_HEAD(&(hXvd->FreeChannelListHead), link);

      /* Free channel handle */
      BKNI_Free(hXvdCh);
   }
}

#if BXVD_P_POWER_MANAGEMENT

#if BXVD_P_USE_XVD_PM1
void BXVD_P_GetHibernateState(BXVD_Handle hXvd, bool *bHibernateState)
{
   uint32_t uiRegVal;

   /* Assume clocks are on */
   *bHibernateState = false;

   /* Check to if PM reg pointers are 0 */
   if ( (hXvd->stPlatformInfo.stReg.uiClkGen_CoreClkCtrl == 0) &&
        (hXvd->stPlatformInfo.stReg.uiVCXO_AVDCtrl == 0) &&
        (hXvd->stPlatformInfo.stReg.uiClkGen_SCBClkCtrl == 0) &&
        (hXvd->stPlatformInfo.stReg.uiClkGen_GISBClkCtrl == 0) )
   {
      BXVD_DBG_WRN(hXvd, ("XVD Powermanagement is not supported on this platform!"));

      return;
   }

   /* Test each clock, if any one is off, set hibernate state to true. */

   uiRegVal = BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiClkGen_CoreClkCtrl);

   if ( uiRegVal & hXvd->stPlatformInfo.stReg.uiClkGen_CoreClkCtrl_PwrDnMask)
   {
      *bHibernateState = true;
   }

   if (hXvd->stPlatformInfo.stReg.uiVCXO_AVDCtrl != (uint32_t)0)
   {
      uiRegVal = BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiVCXO_AVDCtrl);

      if (uiRegVal & hXvd->stPlatformInfo.stReg.uiVCXO_AVDCtrl_PwrDnMask)
      {
         *bHibernateState = true;
      }
   }

   uiRegVal = BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiClkGen_SCBClkCtrl);

   if ( uiRegVal & hXvd->stPlatformInfo.stReg.uiClkGen_SCBClkCtrl_PwrDnMask)
   {
      *bHibernateState = true;
   }

   uiRegVal = BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiClkGen_GISBClkCtrl);

   if ( uiRegVal &hXvd->stPlatformInfo.stReg.uiClkGen_GISBClkCtrl_PwrDnMask)
   {
      *bHibernateState = true;
   }
}
#endif

void BXVD_P_SetHibernateState(BXVD_Handle hXvd, bool bHibernateState)
{
   BXVD_DisplayInterrupt eDisplayInterrupt;
   bool bExternDihInUse = false;

   if ((bHibernateState == false) && (hXvd->bHibernate == true))
   {
      BXVD_DBG_MSG(hXvd, ("SetHibernate(false), acquire power resource AVD%d", hXvd->uDecoderInstance));

      BXVD_P_SET_POWER_STATE(hXvd, BXVD_P_PowerState_eOn);

      BXVD_P_EnableInterrupts(hXvd);
      BXVD_P_EnableWatchdog(hXvd);

      hXvd->bHibernate = false;
   }
   else if ((bHibernateState == true) && (hXvd->bHibernate == false))
   {
      for ( eDisplayInterrupt = 0; eDisplayInterrupt < BXVD_DisplayInterrupt_eMax; eDisplayInterrupt++ )
      {
         if (hXvd->hAppXdmDih[eDisplayInterrupt] != 0 )
         {
            bExternDihInUse = true;
         }
      }

      if ( bExternDihInUse == false )
      {
         BXVD_DBG_MSG(hXvd, ("SetHibernate(true), release power resource AVD%d", hXvd->uDecoderInstance));

         BXVD_P_DisableInterrupts(hXvd);
         BXVD_P_DisableWatchdog(hXvd);

         BXVD_P_SET_POWER_STATE(hXvd, BXVD_P_PowerState_eClkOff);

         hXvd->bHibernate = true;
      }
      else
      {
         BXVD_DBG_MSG(hXvd, ("SetHibernate(true), external XDM DIH in use, DO NOT release power resource AVD%d", hXvd->uDecoderInstance));
      }
   }
}
#endif

#if BXVD_P_CAPTURE_RVC
void BXVD_P_StartRVCCapture(BXVD_Handle hXvd,
                       BXVD_P_PPB_Protocol eProtocol)
{
   uint32_t reg;

   /* Stop previous capture */
   BXVD_Reg_Write32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_RVCCtl, 0);

   /* Set buffer pointers */
   BXVD_Reg_Write32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_RVCPut, hXvd->uiFWRvcBasePhyAddr);
   BXVD_Reg_Write32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_RVCGet, hXvd->uiFWRvcBasePhyAddr);
   BXVD_Reg_Write32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_RVCBase, hXvd->uiFWRvcBasePhyAddr);

   BXVD_Reg_Write32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_RVCEnd, (hXvd->uiFWRvcBasePhyAddr+(4*1024*1024)));

   reg = BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_RVCPut);
   BKNI_Printf("RVC Put %0x, %0x\n", hXvd->stPlatformInfo.stReg.uiDecode_RVCPut, reg);

   reg = BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_RVCGet);
   BKNI_Printf("RVC Get %0x, %0x\n", hXvd->stPlatformInfo.stReg.uiDecode_RVCGet, reg);

   reg = BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_RVCBase);
   BKNI_Printf("RVC Base %0x, %0x\n", hXvd->stPlatformInfo.stReg.uiDecode_RVCBase, reg);

   reg = BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_RVCEnd);
   BKNI_Printf("RVC End %0x, %0x\n", hXvd->stPlatformInfo.stReg.uiDecode_RVCEnd, reg);



   if (eProtocol == BXVD_P_PPB_Protocol_eH265)
   {
      BKNI_Printf("Starting RVC Capture H265\n");
      BXVD_Reg_Write32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_RVCCtl, 3);

      BKNI_Printf("Setting BCHP_HEVD_CMDBUS_XMIT_0_CTL = 0x2\n");

      BXVD_Reg_Write32(hXvd, BCHP_HEVD_CMDBUS_XMIT_0_CTL, 0x2);

      reg = BXVD_Reg_Read32(hXvd, BCHP_HEVD_CMDBUS_XMIT_0_CTL);
      BKNI_Printf("RVC xmit %0x, %0x\n", BCHP_HEVD_CMDBUS_XMIT_0_CTL, reg);
   }
   else
   {
      BKNI_Printf("Starting RVC Capture NOT H265\n");
      BXVD_Reg_Write32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_RVCCtl, 1);
   }

   reg = BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_RVCPut);
   BKNI_Printf("RVC Put %0x, %0x\n", hXvd->stPlatformInfo.stReg.uiDecode_RVCPut, reg);

   reg = BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_RVCGet);
   BKNI_Printf("RVC Get %0x, %0x\n", hXvd->stPlatformInfo.stReg.uiDecode_RVCGet, reg);

   reg = BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_RVCBase);
   BKNI_Printf("RVC Base %0x, %0x\n", hXvd->stPlatformInfo.stReg.uiDecode_RVCBase, reg);

   reg = BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_RVCEnd);
   BKNI_Printf("RVC End %0x, %0x\n", hXvd->stPlatformInfo.stReg.uiDecode_RVCEnd, reg);

   reg = BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_RVCCtl);
   BKNI_Printf("RVC Ctl %0x, %0x\n", hXvd->stPlatformInfo.stReg.uiDecode_RVCCtl, reg);
}

#include <stdio.h>
#include <stdlib.h>

void BXVD_P_DumpRvc(BXVD_P_Context *pXvd)
{
   static FILE *fpRVC = 0;
   uint32_t byteCnt = 0;
   uint32_t reg;

   char *pBuf = (char *)(pXvd->uiFWRvcBaseVirtAddr);

   reg = BXVD_Reg_Read32(pXvd, pXvd->stPlatformInfo.stReg.uiDecode_RVCCtl);
   BKNI_Printf("RVC Ctl %0x, %0x\n", pXvd->stPlatformInfo.stReg.uiDecode_RVCCtl, reg);

   reg = BXVD_Reg_Read32(pXvd, BCHP_HEVD_CMDBUS_XMIT_0_CTL);
   BKNI_Printf("RVC xmit %0x, %0x\n", BCHP_HEVD_CMDBUS_XMIT_0_CTL, reg);

   reg = BXVD_Reg_Read32(pXvd, pXvd->stPlatformInfo.stReg.uiDecode_RVCPut);
   BKNI_Printf("RVC Put %0x, %0x\n", pXvd->stPlatformInfo.stReg.uiDecode_RVCPut, reg);

   reg = BXVD_Reg_Read32(pXvd, pXvd->stPlatformInfo.stReg.uiDecode_RVCGet);
   BKNI_Printf("RVC Get %0x, %0x\n", pXvd->stPlatformInfo.stReg.uiDecode_RVCGet, reg);

   reg = BXVD_Reg_Read32(pXvd, pXvd->stPlatformInfo.stReg.uiDecode_RVCBase);
   BKNI_Printf("RVC Base %0x, %0x\n", pXvd->stPlatformInfo.stReg.uiDecode_RVCBase, reg);

   reg = BXVD_Reg_Read32(pXvd, pXvd->stPlatformInfo.stReg.uiDecode_RVCEnd);
   BKNI_Printf("RVC End %0x, %0x\n", pXvd->stPlatformInfo.stReg.uiDecode_RVCEnd, reg);

   if (fpRVC== 0)
   {
      if ((fpRVC=fopen("output.rvc", "wb"))==0)
      {
         BDBG_WRN(("Could not open output.rvc"));
         return;
      }
   }

   BMMA_FlushCache(pXvd->hFWRvcBlock, (void *)pBuf, (4*1024*1024));

   for ( byteCnt = 0; byteCnt < (4 * 1024 * 1024); byteCnt+= (4*1024))
   {
#if EMULATION
      BEMU_Client_Fwrite(pBuf, (4*1024), fpRVC);
#else
      fwrite(pBuf, 1, (4*1024), fpRVC);
#endif
      pBuf += (4*1024);
   }

   fclose(fpRVC);
}
#endif

/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

/*****************************************************************************/
/*                                                                           */
/*                            DO NOT HAND EDIT!!!                            */
/*                                                                           */
/* Parameters generated on 05/31/2014 at 10:49:19 AM                         */
/*                                                                           */
/*                            DO NOT HAND EDIT!!!                            */
/*                                                                           */
/*    WARNING!!! This file is auto-generated file.  The ordering of the      */
/*               various enums and tables is important to the overall        */
/*               functionality!!                                             */
/*                                                                           */
/*   Did we mention?:        DO NOT HAND EDIT!!!                             */
/*                                                                           */
/*****************************************************************************/
#include "bhdm.h"
#include "bhdm_priv.h"

BDBG_MODULE(BHDM_PACKET_ACR_PRIV) ;


static const  BHDM_BAVC_Clock BHDM_SupportedClocks[] =
{

    /*******************/
    /* Non-420 timings */
    /*******************/

    /*   0 */   {BFMT_PXL_25_2MHz             , BAVC_HDMI_BitsPerPixel_e24bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e25_2             },    /*   25200000 */
    /*   1 */   {BFMT_PXL_25_2MHz             , BAVC_HDMI_BitsPerPixel_e30bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e31_5             },    /*   31500000 */
    /*   2 */   {BFMT_PXL_25_2MHz             , BAVC_HDMI_BitsPerPixel_e36bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e37_8             },    /*   37800000 */

    /*   3 */   {BFMT_PXL_25_2MHz_DIV_1_001   , BAVC_HDMI_BitsPerPixel_e24bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e25_2_DIV_1_001   },    /*   25200000 */
    /*   4 */   {BFMT_PXL_25_2MHz_DIV_1_001   , BAVC_HDMI_BitsPerPixel_e30bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e31_5_DIV_1_001   },    /*   31500000 */
    /*   5 */   {BFMT_PXL_25_2MHz_DIV_1_001   , BAVC_HDMI_BitsPerPixel_e36bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e37_8_DIV_1_001   },    /*   37800000 */

    /*   6 */   {BFMT_PXL_27MHz               , BAVC_HDMI_BitsPerPixel_e24bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e27               },    /*   27000000 */
    /*   7 */   {BFMT_PXL_27MHz               , BAVC_HDMI_BitsPerPixel_e30bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e33_75            },    /*   33750000 */
    /*   8 */   {BFMT_PXL_27MHz               , BAVC_HDMI_BitsPerPixel_e36bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e40_5             },    /*   40500000 */
    /*   9 */   {BFMT_PXL_27MHz               , BAVC_HDMI_BitsPerPixel_e24bit, BAVC_HDMI_PixelRepetition_e1x  , BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e54               },    /*   54000000 */
    /*  10 */   {BFMT_PXL_27MHz               , BAVC_HDMI_BitsPerPixel_e30bit, BAVC_HDMI_PixelRepetition_e1x  , BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e67_5             },    /*   67500000 */
    /*  11 */   {BFMT_PXL_27MHz               , BAVC_HDMI_BitsPerPixel_e36bit, BAVC_HDMI_PixelRepetition_e1x  , BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e81               },    /*   81000000 */
    /*  12 */   {BFMT_PXL_27MHz               , BAVC_HDMI_BitsPerPixel_e24bit, BAVC_HDMI_PixelRepetition_e4x  , BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e108              },    /*  108000000 */
    /*  13 */   {BFMT_PXL_27MHz               , BAVC_HDMI_BitsPerPixel_e30bit, BAVC_HDMI_PixelRepetition_e4x  , BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e135              },    /*  135000000 */
    /*  14 */   {BFMT_PXL_27MHz               , BAVC_HDMI_BitsPerPixel_e36bit, BAVC_HDMI_PixelRepetition_e4x  , BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e162              },    /*  162000000 */

    /*  15 */   {BFMT_PXL_27MHz_MUL_1_001     , BAVC_HDMI_BitsPerPixel_e24bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e27_MUL_1_001     },    /*   27000000 */
    /*  16 */   {BFMT_PXL_27MHz_MUL_1_001     , BAVC_HDMI_BitsPerPixel_e30bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e33_75_MUL_1_001  },    /*   33750000 */
    /*  17 */   {BFMT_PXL_27MHz_MUL_1_001     , BAVC_HDMI_BitsPerPixel_e36bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e40_5_MUL_1_001   },    /*   40500000 */
    /*  18 */   {BFMT_PXL_27MHz_MUL_1_001     , BAVC_HDMI_BitsPerPixel_e24bit, BAVC_HDMI_PixelRepetition_e1x  , BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e54_MUL_1_001     },    /*   54000000 */
    /*  19 */   {BFMT_PXL_27MHz_MUL_1_001     , BAVC_HDMI_BitsPerPixel_e30bit, BAVC_HDMI_PixelRepetition_e1x  , BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e67_5_MUL_1_001   },    /*   67500000 */
    /*  20 */   {BFMT_PXL_27MHz_MUL_1_001     , BAVC_HDMI_BitsPerPixel_e36bit, BAVC_HDMI_PixelRepetition_e1x  , BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e81_MUL_1_001     },    /*   81000000 */
    /*  21 */   {BFMT_PXL_27MHz_MUL_1_001     , BAVC_HDMI_BitsPerPixel_e24bit, BAVC_HDMI_PixelRepetition_e4x  , BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e108_MUL_1_001    },    /*  108000000 */
    /*  22 */   {BFMT_PXL_27MHz_MUL_1_001     , BAVC_HDMI_BitsPerPixel_e30bit, BAVC_HDMI_PixelRepetition_e4x  , BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e135_MUL_1_001    },    /*  135000000 */
    /*  23 */   {BFMT_PXL_27MHz_MUL_1_001     , BAVC_HDMI_BitsPerPixel_e36bit, BAVC_HDMI_PixelRepetition_e4x  , BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e162_MUL_1_001    },    /*  162000000 */

    /**********************
     *  When generating 480p timing, even though this is a 27MHz timings, the VEC uses
     *  a 54MHz clock and then downsamples to the appropriate pixel rate.  However,
     *  during the callback, the indicated pixel rate mask is BFMT_PXL_54MHz or
     *  BFMT_PXL_54MHz_MUL_1_001 mask.  Therefore:
     *  If the VEC indicates 54MHz and no pixel replication, the actual rate is 27MHz.
     *  If the VEC indicates 54MHz and 2x pixel replication, the actual rate is 54MHz.
     *  If the VEC indicates 54MHz and 4x pixel replication, the actual rate is 108MHz.
     *
     *  NOTE: This means that 54MHz will be exactly equivalent to the 27MHz case.
     *        This routine will need to be updated if we ever want to REALLY support
     *        a video format with a true 54MHz pixel rate using these masks...
     **********************/
    /*  24 */   {BFMT_PXL_54MHz               , BAVC_HDMI_BitsPerPixel_e24bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e27               },    /*   27000000 */
    /*  25 */   {BFMT_PXL_54MHz               , BAVC_HDMI_BitsPerPixel_e30bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e33_75            },    /*   33750000 */
    /*  26 */   {BFMT_PXL_54MHz               , BAVC_HDMI_BitsPerPixel_e36bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e40_5             },    /*   40500000 */
    /*  27 */   {BFMT_PXL_54MHz               , BAVC_HDMI_BitsPerPixel_e24bit, BAVC_HDMI_PixelRepetition_e1x  , BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e54               },    /*   54000000 */
    /*  28 */   {BFMT_PXL_54MHz               , BAVC_HDMI_BitsPerPixel_e30bit, BAVC_HDMI_PixelRepetition_e1x  , BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e67_5             },    /*   67500000 */
    /*  29 */   {BFMT_PXL_54MHz               , BAVC_HDMI_BitsPerPixel_e36bit, BAVC_HDMI_PixelRepetition_e1x  , BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e81               },    /*   81000000 */
    /*  30 */   {BFMT_PXL_54MHz               , BAVC_HDMI_BitsPerPixel_e24bit, BAVC_HDMI_PixelRepetition_e4x  , BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e108              },    /*  108000000 */
    /*  31 */   {BFMT_PXL_54MHz               , BAVC_HDMI_BitsPerPixel_e30bit, BAVC_HDMI_PixelRepetition_e4x  , BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e135              },    /*  135000000 */
    /*  32 */   {BFMT_PXL_54MHz               , BAVC_HDMI_BitsPerPixel_e36bit, BAVC_HDMI_PixelRepetition_e4x  , BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e162              },    /*  162000000 */

    /*  33 */   {BFMT_PXL_54MHz_MUL_1_001     , BAVC_HDMI_BitsPerPixel_e24bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e27_MUL_1_001     },    /*   27000000 */
    /*  34 */   {BFMT_PXL_54MHz_MUL_1_001     , BAVC_HDMI_BitsPerPixel_e30bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e33_75_MUL_1_001  },    /*   33750000 */
    /*  35 */   {BFMT_PXL_54MHz_MUL_1_001     , BAVC_HDMI_BitsPerPixel_e36bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e40_5_MUL_1_001   },    /*   40500000 */
    /*  36 */   {BFMT_PXL_54MHz_MUL_1_001     , BAVC_HDMI_BitsPerPixel_e24bit, BAVC_HDMI_PixelRepetition_e1x  , BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e54_MUL_1_001     },    /*   54000000 */
    /*  37 */   {BFMT_PXL_54MHz_MUL_1_001     , BAVC_HDMI_BitsPerPixel_e30bit, BAVC_HDMI_PixelRepetition_e1x  , BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e67_5_MUL_1_001   },    /*   67500000 */
    /*  38 */   {BFMT_PXL_54MHz_MUL_1_001     , BAVC_HDMI_BitsPerPixel_e36bit, BAVC_HDMI_PixelRepetition_e1x  , BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e81_MUL_1_001     },    /*   81000000 */
    /*  39 */   {BFMT_PXL_54MHz_MUL_1_001     , BAVC_HDMI_BitsPerPixel_e24bit, BAVC_HDMI_PixelRepetition_e4x  , BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e108_MUL_1_001    },    /*  108000000 */
    /*  40 */   {BFMT_PXL_54MHz_MUL_1_001     , BAVC_HDMI_BitsPerPixel_e30bit, BAVC_HDMI_PixelRepetition_e4x  , BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e135_MUL_1_001    },    /*  135000000 */
    /*  41 */   {BFMT_PXL_54MHz_MUL_1_001     , BAVC_HDMI_BitsPerPixel_e36bit, BAVC_HDMI_PixelRepetition_e4x  , BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e162_MUL_1_001    },    /*  162000000 */

    /*  42 */   {BFMT_PXL_74_25MHz            , BAVC_HDMI_BitsPerPixel_e24bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e74_25            },    /*   74250000 */
    /*  43 */   {BFMT_PXL_74_25MHz            , BAVC_HDMI_BitsPerPixel_e30bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e92_8125          },    /*   92812500 */
    /*  44 */   {BFMT_PXL_74_25MHz            , BAVC_HDMI_BitsPerPixel_e36bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e111_375          },    /*  111375000 */

    /*  45 */   {BFMT_PXL_74_25MHz_DIV_1_001  , BAVC_HDMI_BitsPerPixel_e24bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e74_25_DIV_1_001  },    /*   74250000 */
    /*  46 */   {BFMT_PXL_74_25MHz_DIV_1_001  , BAVC_HDMI_BitsPerPixel_e30bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e92_8125_DIV_1_001},    /*   92812500 */
    /*  47 */   {BFMT_PXL_74_25MHz_DIV_1_001  , BAVC_HDMI_BitsPerPixel_e36bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e111_375_DIV_1_001},    /*  111375000 */

    /*  48 */   {BFMT_PXL_108MHz              , BAVC_HDMI_BitsPerPixel_e24bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e108              },    /*  108000000 */
    /*  49 */   {BFMT_PXL_108MHz              , BAVC_HDMI_BitsPerPixel_e30bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e135              },    /*  135000000 */
    /*  50 */   {BFMT_PXL_108MHz              , BAVC_HDMI_BitsPerPixel_e36bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e162              },    /*  162000000 */

    /*  51 */   {BFMT_PXL_148_5MHz            , BAVC_HDMI_BitsPerPixel_e24bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e148_5            },    /*  148500000 */
    /*  52 */   {BFMT_PXL_148_5MHz            , BAVC_HDMI_BitsPerPixel_e30bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e185_625          },    /*  185625000 */
    /*  53 */   {BFMT_PXL_148_5MHz            , BAVC_HDMI_BitsPerPixel_e36bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e222_75           },    /*  222750000 */

    /*  54 */   {BFMT_PXL_148_5MHz_DIV_1_001  , BAVC_HDMI_BitsPerPixel_e24bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e148_5_DIV_1_001  },    /*  148500000 */
    /*  55 */   {BFMT_PXL_148_5MHz_DIV_1_001  , BAVC_HDMI_BitsPerPixel_e30bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e185_625_DIV_1_001},    /*  185625000 */
    /*  56 */   {BFMT_PXL_148_5MHz_DIV_1_001  , BAVC_HDMI_BitsPerPixel_e36bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e222_75_DIV_1_001 },    /*  222750000 */

    /*  57 */   {BFMT_PXL_297MHz              , BAVC_HDMI_BitsPerPixel_e24bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e297              },    /*  297000000 */
    /*  58 */   {BFMT_PXL_297MHz              , BAVC_HDMI_BitsPerPixel_e30bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e371_25           },    /*  371250000 */
    /*  59 */   {BFMT_PXL_297MHz              , BAVC_HDMI_BitsPerPixel_e36bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e445_5            },    /*  445500000 */

    /*  60 */   {BFMT_PXL_297MHz_DIV_1_001    , BAVC_HDMI_BitsPerPixel_e24bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e297_DIV_1_001    },    /*  297000000 */
    /*  61 */   {BFMT_PXL_297MHz_DIV_1_001    , BAVC_HDMI_BitsPerPixel_e30bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e371_25_DIV_1_001 },    /*  371250000 */
    /*  62 */   {BFMT_PXL_297MHz_DIV_1_001    , BAVC_HDMI_BitsPerPixel_e36bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e445_5_DIV_1_001  },    /*  445500000 */

    /*  63 */   {BFMT_PXL_594MHz            , BAVC_HDMI_BitsPerPixel_e24bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e594              },    /*  594000000 */
/*  {BFMT_PXL_594MHz            , BAVC_HDMI_BitsPerPixel_e30bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e742_5            }, */ /*  742500000 */ /*Rate is too fast for HDMI */
/*  {BFMT_PXL_594MHz            , BAVC_HDMI_BitsPerPixel_e36bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e891              }, */ /*  891000000 */ /*Rate is too fast for HDMI */

    /*  64 */   {BFMT_PXL_594MHz_DIV_1_001  , BAVC_HDMI_BitsPerPixel_e24bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e594_DIV_1_001    },    /*  594000000 */
/*  {BFMT_PXL_594MHz_DIV_1_001  , BAVC_HDMI_BitsPerPixel_e30bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e742_5_DIV_1_001  }, */ /*  742500000 */ /*Rate is too fast for HDMI */
/*  {BFMT_PXL_594MHz_DIV_1_001  , BAVC_HDMI_BitsPerPixel_e36bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e891_DIV_1_001    }, */ /*  891000000 */ /*Rate is too fast for HDMI */

    /*  65 */   {BFMT_PXL_65MHz               , BAVC_HDMI_BitsPerPixel_e24bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e65               },    /*   65000000 */
    /*  66 */   {BFMT_PXL_65MHz               , BAVC_HDMI_BitsPerPixel_e30bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e81_25            },    /*   81250000 */
    /*  67 */   {BFMT_PXL_65MHz               , BAVC_HDMI_BitsPerPixel_e36bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e97_5             },    /*   97500000 */

    /*  68 */   {BFMT_PXL_65MHz_DIV_1_001     , BAVC_HDMI_BitsPerPixel_e24bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e65_DIV_1_001     },    /*   65000000 */
    /*  69 */   {BFMT_PXL_65MHz_DIV_1_001     , BAVC_HDMI_BitsPerPixel_e30bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e81_25_DIV_1_001  },    /*   81250000 */
    /*  70 */   {BFMT_PXL_65MHz_DIV_1_001     , BAVC_HDMI_BitsPerPixel_e36bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444, BHDM_P_TmdsClock_e97_5_DIV_1_001   },    /*   97500000 */

    /***************/
    /* 420 timings */
    /***************/

    /*  71 */   {BFMT_PXL_594MHz            , BAVC_HDMI_BitsPerPixel_e24bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr420, BHDM_P_TmdsClock_e297              },    /*  297000000 */
    /*  72 */   {BFMT_PXL_594MHz            , BAVC_HDMI_BitsPerPixel_e30bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr420, BHDM_P_TmdsClock_e371_25           },    /*  371250000 */
    /*  73 */   {BFMT_PXL_594MHz            , BAVC_HDMI_BitsPerPixel_e36bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr420, BHDM_P_TmdsClock_e445_5            },    /*  445500000 */

    /*  74 */   {BFMT_PXL_594MHz_DIV_1_001  , BAVC_HDMI_BitsPerPixel_e24bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr420, BHDM_P_TmdsClock_e297_DIV_1_001    },    /*  297000000 */
    /*  75 */   {BFMT_PXL_594MHz_DIV_1_001  , BAVC_HDMI_BitsPerPixel_e30bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr420, BHDM_P_TmdsClock_e371_25_DIV_1_001 },    /*  371250000 */
    /*  76 */   {BFMT_PXL_594MHz_DIV_1_001  , BAVC_HDMI_BitsPerPixel_e36bit, BAVC_HDMI_PixelRepetition_eNone, BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr420, BHDM_P_TmdsClock_e445_5_DIV_1_001  },    /*  445500000 */
} ;



/******************************************************************************
 * Summary:
 * HDMI Audio Clock Capture and Regeneration Values for 32 kHz
 * NOTE: The order of these entries must match the order of the entries in the
 *       BHDM_P_TmdsClock enum declaration
 * USAGE: S/W must write the values in the following table to the appropriate
 *        registers:
 *           CTS0 --> HDMI_CTS_0.CTS_0
 *           CTS0 --> HDMI CTS_PERIOD_0.CTS_PERIOD_0
 *           RPT0 --> HDMI CTS_PERIOD_0.CTS_0_REPEAT
 *           CTS1 --> HDMI_CTS_1.CTS_1
 *           CTS1 --> HDMI CTS_PERIOD_1.CTS_PERIOD_1
 *           RPT1 --> HDMI CTS_PERIOD_1.CTS_1_REPEAT
 *        Also, HDMI_CRP_CFG.USE_MAI_BUS_SYNC_FOR_CTS_GENERATION must be
 *        set to 0
 *******************************************************************************/
static const BHDM_P_AUDIO_CLK_VALUES BHDM_32KHz_AudioClkValues[] =
{   /* SW-N,   HW-N,  CTS_0,  CTS_1, RPT0, RPT1 *//*     TMDS Clock Rate, BPP,   Comp Audio Rate,    ACR Rate*/

    {   4096,   4096,  25200,  25200,    1,    1 }, /*   0 *//*            25.2 MHz,  24,     32.000000 kHz,    1000 Hz *//*   Spec Settings */
    {   4096,   4096,  31500,  31500,    1,    1 }, /*   1 *//*            31.5 MHz,  30,     32.000000 kHz,    1000 Hz *//*   Spec Settings */
    {   4096,   4096,  37800,  37800,    1,    1 }, /*   2 *//*            37.8 MHz,  36,     32.000000 kHz,    1000 Hz *//*   Spec Settings */

    {   4576,   4096,  28125,  28125,    1,    1 }, /*   3 *//*    25.2 / 1.001 MHz,  24,     32.000000 kHz, 895.105 Hz *//*   Spec Settings */
    {   9152,   4096,  70312,  70313,    1,    1 }, /*   4 *//*    31.5 / 1.001 MHz,  30,     32.000000 kHz, 447.552 Hz *//*   Spec Settings */
    {   9152,   4096,  84375,  84375,    1,    1 }, /*   5 *//*    37.8 / 1.001 MHz,  36,     32.000000 kHz, 447.552 Hz *//*   Spec Settings */

    {   4096,   4096,  27000,  27000,    1,    1 }, /*   6 *//*              27 MHz,  24,     32.000000 kHz,    1000 Hz *//*   Spec Settings */
    {   4096,   4096,  33750,  33750,    1,    1 }, /*   7 *//*           33.75 MHz,  30,     32.000000 kHz,    1000 Hz *//*   Spec Settings */
    {   4096,   4096,  40500,  40500,    1,    1 }, /*   8 *//*            40.5 MHz,  36,     32.000000 kHz,    1000 Hz *//*   Spec Settings */
    {   4096,   4096,  54000,  54000,    1,    1 }, /*   9 *//*              54 MHz,  24,     32.000000 kHz,    1000 Hz *//*   Spec Settings */
    {   4096,   4096,  67500,  67500,    1,    1 }, /*  10 *//*            67.5 MHz,  30,     32.000000 kHz,    1000 Hz *//*   Spec Settings */
    {   4096,   4096,  81000,  81000,    1,    1 }, /*  11 *//*              81 MHz,  36,     32.000000 kHz,    1000 Hz *//*   Spec Settings */
    {   4096,   4096, 108000, 108000,    1,    1 }, /*  12 *//*             108 MHz,  24,     32.000000 kHz,    1000 Hz *//* No Settings Found */
    {   4096,   4096, 135000, 135000,    1,    1 }, /*  13 *//*             135 MHz,  30,     32.000000 kHz,    1000 Hz *//* No Settings Found */
    {   4096,   4096, 162000, 162000,    1,    1 }, /*  14 *//*             162 MHz,  36,     32.000000 kHz,    1000 Hz *//* No Settings Found */

    {   4096,   4096,  27027,  27027,    1,    1 }, /*  15 *//*      27 * 1.001 MHz,  24,     32.000000 kHz,    1000 Hz *//*   Spec Settings */
    {   8192,   4096,  67567,  67568,    1,    1 }, /*  16 *//*   33.75 * 1.001 MHz,  30,     32.000000 kHz,     500 Hz *//*   Spec Settings */
    {   8192,   4096,  81081,  81081,    1,    1 }, /*  17 *//*    40.5 * 1.001 MHz,  36,     32.000000 kHz,     500 Hz *//*   Spec Settings */
    {   4096,   4096,  54054,  54054,    1,    1 }, /*  18 *//*      54 * 1.001 MHz,  24,     32.000000 kHz,    1000 Hz *//*   Spec Settings */
    {   8192,   4096, 135135, 135135,    1,    1 }, /*  19 *//*    67.5 * 1.001 MHz,  30,     32.000000 kHz,     500 Hz *//*   Spec Settings */
    {   4096,   4096,  81081,  81081,    1,    1 }, /*  20 *//*      81 * 1.001 MHz,  36,     32.000000 kHz,    1000 Hz *//*   Spec Settings */
    {   4096,   4096, 108108, 108108,    1,    1 }, /*  21 *//*     108 * 1.001 MHz,  24,     32.000000 kHz,    1000 Hz *//* No Settings Found */
    {   4096,   4096, 135135, 135135,    1,    1 }, /*  22 *//*     135 * 1.001 MHz,  30,     32.000000 kHz,    1000 Hz *//* No Settings Found */
    {   4096,   4096, 162162, 162162,    1,    1 }, /*  23 *//*     162 * 1.001 MHz,  36,     32.000000 kHz,    1000 Hz *//* No Settings Found */



    {   4096,   4096,  74250,  74250,    1,    1 }, /*  24 *//*           74.25 MHz,  24,     32.000000 kHz,    1000 Hz *//*   Spec Settings */
    {   8192,   4096, 185625, 185625,    1,    1 }, /*  25 *//*         92.8125 MHz,  30,     32.000000 kHz,     500 Hz *//*   Spec Settings */
    {   4096,   4096, 111375, 111375,    1,    1 }, /*  26 *//*         111.375 MHz,  36,     32.000000 kHz,    1000 Hz *//*   Spec Settings */

    {  11648,   4096, 210937, 210938,    1,    1 }, /*  27 *//*   74.25 / 1.001 MHz,  24,     32.000000 kHz, 351.648 Hz *//*   Spec Settings */
    {  11648,   4096, 263671, 263672,    1,    7 }, /*  28 *//* 92.8125 / 1.001 MHz,  30,     32.000000 kHz, 351.648 Hz *//*   Spec Settings */
    {  11648,   4096, 316406, 316407,    3,    1 }, /*  29 *//* 111.375 / 1.001 MHz,  36,     32.000000 kHz, 351.648 Hz *//*   Spec Settings */


    {   4096,   4096, 148500, 148500,    1,    1 }, /*  30 *//*           148.5 MHz,  24,     32.000000 kHz,    1000 Hz *//*   Spec Settings */
    {   4096,   4096, 185625, 185625,    1,    1 }, /*  31 *//*         185.625 MHz,  30,     32.000000 kHz,    1000 Hz *//*   Spec Settings */
    {   4096,   4096, 222750, 222750,    1,    1 }, /*  32 *//*          222.75 MHz,  36,     32.000000 kHz,    1000 Hz *//*   Spec Settings */

    {  11648,   4096, 421875, 421875,    1,    1 }, /*  33 *//*   148.5 / 1.001 MHz,  24,     32.000000 kHz, 351.648 Hz *//*   Spec Settings */
    {  11648,   4096, 527343, 527344,    1,    3 }, /*  34 *//* 185.625 / 1.001 MHz,  30,     32.000000 kHz, 351.648 Hz *//*   Spec Settings */
    {  11648,   4096, 632812, 632813,    1,    1 }, /*  35 *//*  222.75 / 1.001 MHz,  36,     32.000000 kHz, 351.648 Hz *//*   Spec Settings */

    {   3072,   4096, 222750, 222750,    1,    1 }, /*  36 *//*             297 MHz,  24,     32.000000 kHz, 1333.33 Hz *//*   Spec Settings */
    {   6144,   4096, 556875, 556875,    1,    1 }, /*  37 *//*          371.25 MHz,  30,     32.000000 kHz, 666.667 Hz *//*   Spec Settings */
    {   4096,   4096, 445500, 445500,    1,    1 }, /*  38 *//*           445.5 MHz,  36,     32.000000 kHz,    1000 Hz *//*   Spec Settings */

    {   5824,   4096, 421875, 421875,    1,    1 }, /*  39 *//*     297 / 1.001 MHz,  24,     32.000000 kHz, 703.297 Hz *//*   Spec Settings */
    {   5824,   4096, 527343, 527344,    1,    3 }, /*  40 *//*  371.25 / 1.001 MHz,  30,     32.000000 kHz, 703.297 Hz *//*   Spec Settings */
    {   5824,   4096, 632812, 632813,    1,    1 }, /*  41 *//*   445.5 / 1.001 MHz,  36,     32.000000 kHz, 703.297 Hz *//*   Spec Settings */

    {   3072,   4096, 445500, 445500,    1,    1 }, /*  42 *//*             594 MHz,  24,     32.000000 kHz, 1333.33 Hz *//*   Spec Settings */

    {   5824,   4096, 843750, 843750,    1,    1 }, /*  43 *//*     594 / 1.001 MHz,  24,     32.000000 kHz, 703.297 Hz *//*   Spec Settings */

    {   4096,   4096,  65000,  65000,    1,    1 }, /*  44 *//*              65 MHz,  24,     32.000000 kHz,    1000 Hz *//* No Settings Found */
    {   4096,   4096,  81250,  81250,    1,    1 }, /*  45 *//*           81.25 MHz,  30,     32.000000 kHz,    1000 Hz *//* No Settings Found */
    {   4096,   4096,  97500,  97500,    1,    1 }, /*  46 *//*            97.5 MHz,  36,     32.000000 kHz,    1000 Hz *//* No Settings Found */

    {   4928,   4096,  78125,  78125,    1,    1 }, /*  47 *//*      65 / 1.001 MHz,  24,     32.000000 kHz, 831.169 Hz *//* No Settings Found */
    {   9856,   4096, 195312, 195313,    1,    1 }, /*  48 *//*   81.25 / 1.001 MHz,  30,     32.000000 kHz, 415.584 Hz *//* No Settings Found */
    {   9856,   4096, 234375, 234375,    1,    1 }, /*  49 *//*    97.5 / 1.001 MHz,  36,     32.000000 kHz, 415.584 Hz *//* No Settings Found */


};

/******************************************************************************
 * Summary:
 * HDMI Audio Clock Capture and Regeneration Values for 128 kHz
 * NOTE: The order of these entries must match the order of the entries in the
 *       BHDM_P_TmdsClock enum declaration
 * USAGE: S/W must write the values in the following table to the appropriate
 *        registers:
 *           CTS0 --> HDMI_CTS_0.CTS_0
 *           CTS0 --> HDMI CTS_PERIOD_0.CTS_PERIOD_0
 *           RPT0 --> HDMI CTS_PERIOD_0.CTS_0_REPEAT
 *           CTS1 --> HDMI_CTS_1.CTS_1
 *           CTS1 --> HDMI CTS_PERIOD_1.CTS_PERIOD_1
 *           RPT1 --> HDMI CTS_PERIOD_1.CTS_1_REPEAT
 *        Also, HDMI_CRP_CFG.USE_MAI_BUS_SYNC_FOR_CTS_GENERATION must be
 *        set to 0
 *******************************************************************************/
static const BHDM_P_AUDIO_CLK_VALUES BHDM_128KHz_AudioClkValues[] =
{   /* SW-N,   HW-N,  CTS_0,  CTS_1, RPT0, RPT1 *//*     TMDS Clock Rate, BPP,   Comp Audio Rate,    ACR Rate*/

    {  16384,  16384,  25200,  25200,    1,    1 }, /*   0 *//*            25.2 MHz,  24,    128.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  16384,  16384,  31500,  31500,    1,    1 }, /*   1 *//*            31.5 MHz,  30,    128.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  16384,  16384,  37800,  37800,    1,    1 }, /*   2 *//*            37.8 MHz,  36,    128.000000 kHz,    1000 Hz *//*   Spec Settings */

    {  18304,  16384,  28125,  28125,    1,    1 }, /*   3 *//*    25.2 / 1.001 MHz,  24,    128.000000 kHz, 895.105 Hz *//*   Spec Settings */
    {  36608,  16384,  70312,  70313,    1,    1 }, /*   4 *//*    31.5 / 1.001 MHz,  30,    128.000000 kHz, 447.552 Hz *//*   Spec Settings */
    {  36608,  16384,  84375,  84375,    1,    1 }, /*   5 *//*    37.8 / 1.001 MHz,  36,    128.000000 kHz, 447.552 Hz *//*   Spec Settings */

    {  16384,  16384,  27000,  27000,    1,    1 }, /*   6 *//*              27 MHz,  24,    128.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  16384,  16384,  33750,  33750,    1,    1 }, /*   7 *//*           33.75 MHz,  30,    128.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  16384,  16384,  40500,  40500,    1,    1 }, /*   8 *//*            40.5 MHz,  36,    128.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  16384,  16384,  54000,  54000,    1,    1 }, /*   9 *//*              54 MHz,  24,    128.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  16384,  16384,  67500,  67500,    1,    1 }, /*  10 *//*            67.5 MHz,  30,    128.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  16384,  16384,  81000,  81000,    1,    1 }, /*  11 *//*              81 MHz,  36,    128.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  16384,  16384, 108000, 108000,    1,    1 }, /*  12 *//*             108 MHz,  24,    128.000000 kHz,    1000 Hz *//* No Settings Found */
    {  16384,  16384, 135000, 135000,    1,    1 }, /*  13 *//*             135 MHz,  30,    128.000000 kHz,    1000 Hz *//* No Settings Found */
    {  16384,  16384, 162000, 162000,    1,    1 }, /*  14 *//*             162 MHz,  36,    128.000000 kHz,    1000 Hz *//* No Settings Found */

    {  16384,  16384,  27027,  27027,    1,    1 }, /*  15 *//*      27 * 1.001 MHz,  24,    128.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  32768,  16384,  67567,  67568,    1,    1 }, /*  16 *//*   33.75 * 1.001 MHz,  30,    128.000000 kHz,     500 Hz *//*   Spec Settings */
    {  32768,  16384,  81081,  81081,    1,    1 }, /*  17 *//*    40.5 * 1.001 MHz,  36,    128.000000 kHz,     500 Hz *//*   Spec Settings */
    {  16384,  16384,  54054,  54054,    1,    1 }, /*  18 *//*      54 * 1.001 MHz,  24,    128.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  32768,  16384, 135135, 135135,    1,    1 }, /*  19 *//*    67.5 * 1.001 MHz,  30,    128.000000 kHz,     500 Hz *//*   Spec Settings */
    {  16384,  16384,  81081,  81081,    1,    1 }, /*  20 *//*      81 * 1.001 MHz,  36,    128.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  16384,  16384, 108108, 108108,    1,    1 }, /*  21 *//*     108 * 1.001 MHz,  24,    128.000000 kHz,    1000 Hz *//* No Settings Found */
    {  16384,  16384, 135135, 135135,    1,    1 }, /*  22 *//*     135 * 1.001 MHz,  30,    128.000000 kHz,    1000 Hz *//* No Settings Found */
    {  16384,  16384, 162162, 162162,    1,    1 }, /*  23 *//*     162 * 1.001 MHz,  36,    128.000000 kHz,    1000 Hz *//* No Settings Found */



    {  16384,  16384,  74250,  74250,    1,    1 }, /*  24 *//*           74.25 MHz,  24,    128.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  32768,  16384, 185625, 185625,    1,    1 }, /*  25 *//*         92.8125 MHz,  30,    128.000000 kHz,     500 Hz *//*   Spec Settings */
    {  16384,  16384, 111375, 111375,    1,    1 }, /*  26 *//*         111.375 MHz,  36,    128.000000 kHz,    1000 Hz *//*   Spec Settings */

    {  46592,  16384, 210937, 210938,    1,    1 }, /*  27 *//*   74.25 / 1.001 MHz,  24,    128.000000 kHz, 351.648 Hz *//*   Spec Settings */
    {  46592,  16384, 263671, 263672,    1,    7 }, /*  28 *//* 92.8125 / 1.001 MHz,  30,    128.000000 kHz, 351.648 Hz *//*   Spec Settings */
    {  46592,  16384, 316406, 316407,    3,    1 }, /*  29 *//* 111.375 / 1.001 MHz,  36,    128.000000 kHz, 351.648 Hz *//*   Spec Settings */


    {  16384,  16384, 148500, 148500,    1,    1 }, /*  30 *//*           148.5 MHz,  24,    128.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  16384,  16384, 185625, 185625,    1,    1 }, /*  31 *//*         185.625 MHz,  30,    128.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  16384,  16384, 222750, 222750,    1,    1 }, /*  32 *//*          222.75 MHz,  36,    128.000000 kHz,    1000 Hz *//*   Spec Settings */

    {  46592,  16384, 421875, 421875,    1,    1 }, /*  33 *//*   148.5 / 1.001 MHz,  24,    128.000000 kHz, 351.648 Hz *//*   Spec Settings */
    {  46592,  16384, 527343, 527344,    1,    3 }, /*  34 *//* 185.625 / 1.001 MHz,  30,    128.000000 kHz, 351.648 Hz *//*   Spec Settings */
    {  46592,  16384, 632812, 632813,    1,    1 }, /*  35 *//*  222.75 / 1.001 MHz,  36,    128.000000 kHz, 351.648 Hz *//*   Spec Settings */

    {  16384,  16384, 297000, 297000,    1,    1 }, /*  36 *//*             297 MHz,  24,    128.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  24576,  16384, 556875, 556875,    1,    1 }, /*  37 *//*          371.25 MHz,  30,    128.000000 kHz, 666.667 Hz *//*   Spec Settings */
    {  16384,  16384, 445500, 445500,    1,    1 }, /*  38 *//*           445.5 MHz,  36,    128.000000 kHz,    1000 Hz *//*   Spec Settings */

    {  23296,  16384, 421875, 421875,    1,    1 }, /*  39 *//*     297 / 1.001 MHz,  24,    128.000000 kHz, 703.297 Hz *//*   Spec Settings */
    {  23296,  16384, 527343, 527344,    1,    3 }, /*  40 *//*  371.25 / 1.001 MHz,  30,    128.000000 kHz, 703.297 Hz *//*   Spec Settings */
    {  23296,  16384, 632812, 632813,    1,    1 }, /*  41 *//*   445.5 / 1.001 MHz,  36,    128.000000 kHz, 703.297 Hz *//*   Spec Settings */

    {  16384,  16384, 594000, 594000,    1,    1 }, /*  42 *//*             594 MHz,  24,    128.000000 kHz,    1000 Hz *//*   Spec Settings */

    {  23296,  16384, 843750, 843750,    1,    1 }, /*  43 *//*     594 / 1.001 MHz,  24,    128.000000 kHz, 703.297 Hz *//*   Spec Settings */

    {  16384,  16384,  65000,  65000,    1,    1 }, /*  44 *//*              65 MHz,  24,    128.000000 kHz,    1000 Hz *//* No Settings Found */
    {  16384,  16384,  81250,  81250,    1,    1 }, /*  45 *//*           81.25 MHz,  30,    128.000000 kHz,    1000 Hz *//* No Settings Found */
    {  16384,  16384,  97500,  97500,    1,    1 }, /*  46 *//*            97.5 MHz,  36,    128.000000 kHz,    1000 Hz *//* No Settings Found */

    {  19712,  16384,  78125,  78125,    1,    1 }, /*  47 *//*      65 / 1.001 MHz,  24,    128.000000 kHz, 831.169 Hz *//* No Settings Found */
    {  39424,  16384, 195312, 195313,    1,    1 }, /*  48 *//*   81.25 / 1.001 MHz,  30,    128.000000 kHz, 415.584 Hz *//* No Settings Found */
    {  39424,  16384, 234375, 234375,    1,    1 }, /*  49 *//*    97.5 / 1.001 MHz,  36,    128.000000 kHz, 415.584 Hz *//* No Settings Found */


};

/******************************************************************************
 * Summary:
 * HDMI Audio Clock Capture and Regeneration Values for 44.1 kHz
 * NOTE: The order of these entries must match the order of the entries in the
 *       BHDM_P_TmdsClock enum declaration
 * USAGE: S/W must write the values in the following table to the appropriate
 *        registers:
 *           CTS0 --> HDMI_CTS_0.CTS_0
 *           CTS0 --> HDMI CTS_PERIOD_0.CTS_PERIOD_0
 *           RPT0 --> HDMI CTS_PERIOD_0.CTS_0_REPEAT
 *           CTS1 --> HDMI_CTS_1.CTS_1
 *           CTS1 --> HDMI CTS_PERIOD_1.CTS_PERIOD_1
 *           RPT1 --> HDMI CTS_PERIOD_1.CTS_1_REPEAT
 *        Also, HDMI_CRP_CFG.USE_MAI_BUS_SYNC_FOR_CTS_GENERATION must be
 *        set to 0
 *******************************************************************************/
static const BHDM_P_AUDIO_CLK_VALUES BHDM_44_1KHz_AudioClkValues[] =
{   /* SW-N,   HW-N,  CTS_0,  CTS_1, RPT0, RPT1 *//*     TMDS Clock Rate, BPP,   Comp Audio Rate,    ACR Rate*/

    {   6272,      0,  28000,  28000,    1,    1 }, /*   0 *//*            25.2 MHz,  24,     44.100000 kHz,     900 Hz *//*   Spec Settings */
    {   6272,      0,  35000,  35000,    1,    1 }, /*   1 *//*            31.5 MHz,  30,     44.100000 kHz,     900 Hz *//*   Spec Settings */
    {   6272,      0,  42000,  42000,    1,    1 }, /*   2 *//*            37.8 MHz,  36,     44.100000 kHz,     900 Hz *//*   Spec Settings */

    {   7007,      0,  31250,  31250,    1,    1 }, /*   3 *//*    25.2 / 1.001 MHz,  24,     44.100000 kHz, 805.594 Hz *//*   Spec Settings */
    {  14014,      0,  78125,  78125,    1,    1 }, /*   4 *//*    31.5 / 1.001 MHz,  30,     44.100000 kHz, 402.797 Hz *//*   Spec Settings */
    {   7007,      0,  46875,  46875,    1,    1 }, /*   5 *//*    37.8 / 1.001 MHz,  36,     44.100000 kHz, 805.594 Hz *//*   Spec Settings */

    {   6272,      0,  30000,  30000,    1,    1 }, /*   6 *//*              27 MHz,  24,     44.100000 kHz,     900 Hz *//*   Spec Settings */
    {   6272,      0,  37500,  37500,    1,    1 }, /*   7 *//*           33.75 MHz,  30,     44.100000 kHz,     900 Hz *//*   Spec Settings */
    {   6272,      0,  45000,  45000,    1,    1 }, /*   8 *//*            40.5 MHz,  36,     44.100000 kHz,     900 Hz *//*   Spec Settings */
    {   6272,      0,  60000,  60000,    1,    1 }, /*   9 *//*              54 MHz,  24,     44.100000 kHz,     900 Hz *//*   Spec Settings */
    {   6272,      0,  75000,  75000,    1,    1 }, /*  10 *//*            67.5 MHz,  30,     44.100000 kHz,     900 Hz *//*   Spec Settings */
    {   6272,      0,  90000,  90000,    1,    1 }, /*  11 *//*              81 MHz,  36,     44.100000 kHz,     900 Hz *//*   Spec Settings */
    {   5684,      0, 108750, 108750,    1,    1 }, /*  12 *//*             108 MHz,  24,     44.100000 kHz, 993.103 Hz *//* No Settings Found */
    {   5488,      0, 131250, 131250,    1,    1 }, /*  13 *//*             135 MHz,  30,     44.100000 kHz, 1028.57 Hz *//* No Settings Found */
    {   5684,      0, 163125, 163125,    1,    1 }, /*  14 *//*             162 MHz,  36,     44.100000 kHz, 993.103 Hz *//* No Settings Found */

    {   6272,      0,  30030,  30030,    1,    1 }, /*  15 *//*      27 * 1.001 MHz,  24,     44.100000 kHz,     900 Hz *//*   Spec Settings */
    {  12544,      0,  75075,  75075,    1,    1 }, /*  16 *//*   33.75 * 1.001 MHz,  30,     44.100000 kHz,     450 Hz *//*   Spec Settings */
    {   6272,      0,  45045,  45045,    1,    1 }, /*  17 *//*    40.5 * 1.001 MHz,  36,     44.100000 kHz,     900 Hz *//*   Spec Settings */
    {   6272,      0,  60060,  60060,    1,    1 }, /*  18 *//*      54 * 1.001 MHz,  24,     44.100000 kHz,     900 Hz *//*   Spec Settings */
    {   6272,      0,  75075,  75075,    1,    1 }, /*  19 *//*    67.5 * 1.001 MHz,  30,     44.100000 kHz,     900 Hz *//*   Spec Settings */
    {   6272,      0,  90090,  90090,    1,    1 }, /*  20 *//*      81 * 1.001 MHz,  36,     44.100000 kHz,     900 Hz *//*   Spec Settings */
    {   5600,      0, 107250, 107250,    1,    1 }, /*  21 *//*     108 * 1.001 MHz,  24,     44.100000 kHz,    1008 Hz *//* No Settings Found */
    {   5824,      0, 139425, 139425,    1,    1 }, /*  22 *//*     135 * 1.001 MHz,  30,     44.100000 kHz, 969.231 Hz *//* No Settings Found */
    {   5600,      0, 160875, 160875,    1,    1 }, /*  23 *//*     162 * 1.001 MHz,  36,     44.100000 kHz,    1008 Hz *//* No Settings Found */



    {   6272,      0,  82500,  82500,    1,    1 }, /*  24 *//*           74.25 MHz,  24,     44.100000 kHz,     900 Hz *//*   Spec Settings */
    {   6272,      0, 103125, 103125,    1,    1 }, /*  25 *//*         92.8125 MHz,  30,     44.100000 kHz,     900 Hz *//*   Spec Settings */
    {   6272,      0, 123750, 123750,    1,    1 }, /*  26 *//*         111.375 MHz,  36,     44.100000 kHz,     900 Hz *//*   Spec Settings */

    {  17836,      0, 234375, 234375,    1,    1 }, /*  27 *//*   74.25 / 1.001 MHz,  24,     44.100000 kHz, 316.484 Hz *//*   Spec Settings */
    {  17836,      0, 292968, 292969,    1,    3 }, /*  28 *//* 92.8125 / 1.001 MHz,  30,     44.100000 kHz, 316.484 Hz *//*   Spec Settings */
    {  17836,      0, 351562, 351563,    1,    1 }, /*  29 *//* 111.375 / 1.001 MHz,  36,     44.100000 kHz, 316.484 Hz *//*   Spec Settings */


    {   6272,      0, 165000, 165000,    1,    1 }, /*  30 *//*           148.5 MHz,  24,     44.100000 kHz,     900 Hz *//*   Spec Settings */
    {   6272,      0, 206250, 206250,    1,    1 }, /*  31 *//*         185.625 MHz,  30,     44.100000 kHz,     900 Hz *//*   Spec Settings */
    {   6272,      0, 247500, 247500,    1,    1 }, /*  32 *//*          222.75 MHz,  36,     44.100000 kHz,     900 Hz *//*   Spec Settings */

    {   8918,      0, 234375, 234375,    1,    1 }, /*  33 *//*   148.5 / 1.001 MHz,  24,     44.100000 kHz, 632.967 Hz *//*   Spec Settings */
    {  17836,      0, 585937, 585938,    1,    1 }, /*  34 *//* 185.625 / 1.001 MHz,  30,     44.100000 kHz, 316.484 Hz *//*   Spec Settings */
    {  17836,      0, 703125, 703125,    1,    1 }, /*  35 *//*  222.75 / 1.001 MHz,  36,     44.100000 kHz, 316.484 Hz *//*   Spec Settings */

    {   4704,      0, 247500, 247500,    1,    1 }, /*  36 *//*             297 MHz,  24,     44.100000 kHz,    1200 Hz *//*   Spec Settings */
    {   4704,      0, 309375, 309375,    1,    1 }, /*  37 *//*          371.25 MHz,  30,     44.100000 kHz,    1200 Hz *//*   Spec Settings */
    {   4704,      0, 371250, 371250,    1,    1 }, /*  38 *//*           445.5 MHz,  36,     44.100000 kHz,    1200 Hz *//*   Spec Settings */

    {   4459,      0, 234375, 234375,    1,    1 }, /*  39 *//*     297 / 1.001 MHz,  24,     44.100000 kHz, 1265.93 Hz *//*   Spec Settings */
    {   8918,      0, 585937, 585938,    1,    1 }, /*  40 *//*  371.25 / 1.001 MHz,  30,     44.100000 kHz, 632.967 Hz *//*   Spec Settings */
    {   8918,      0, 703125, 703125,    1,    1 }, /*  41 *//*   445.5 / 1.001 MHz,  36,     44.100000 kHz, 632.967 Hz *//*   Spec Settings */

    {   9408,      0, 990000, 990000,    1,    1 }, /*  42 *//*             594 MHz,  24,     44.100000 kHz,     600 Hz *//*   Spec Settings */

    {   8918,      0, 937500, 937500,    1,    1 }, /*  43 *//*     594 / 1.001 MHz,  24,     44.100000 kHz, 632.967 Hz *//*   Spec Settings */

    {   7056,      0,  81250,  81250,    1,    1 }, /*  44 *//*              65 MHz,  24,     44.100000 kHz,     800 Hz *//* No Settings Found */
    {  14112,      0, 203125, 203125,    1,    1 }, /*  45 *//*           81.25 MHz,  30,     44.100000 kHz,     400 Hz *//* No Settings Found */
    {   4704,      0,  81250,  81250,    1,    1 }, /*  46 *//*            97.5 MHz,  36,     44.100000 kHz,    1200 Hz *//* No Settings Found */

    {  11319,      0, 130208, 130209,    2,    1 }, /*  47 *//*      65 / 1.001 MHz,  24,     44.100000 kHz, 498.701 Hz *//* No Settings Found */
    {  15092,      0, 217013, 217014,    1,    8 }, /*  48 *//*   81.25 / 1.001 MHz,  30,     44.100000 kHz, 374.026 Hz *//* No Settings Found */
    {  11319,      0, 195312, 195313,    1,    1 }, /*  49 *//*    97.5 / 1.001 MHz,  36,     44.100000 kHz, 498.701 Hz *//* No Settings Found */


};

/******************************************************************************
 * Summary:
 * HDMI Audio Clock Capture and Regeneration Values for 88.2 kHz
 * NOTE: The order of these entries must match the order of the entries in the
 *       BHDM_P_TmdsClock enum declaration
 * USAGE: S/W must write the values in the following table to the appropriate
 *        registers:
 *           CTS0 --> HDMI_CTS_0.CTS_0
 *           CTS0 --> HDMI CTS_PERIOD_0.CTS_PERIOD_0
 *           RPT0 --> HDMI CTS_PERIOD_0.CTS_0_REPEAT
 *           CTS1 --> HDMI_CTS_1.CTS_1
 *           CTS1 --> HDMI CTS_PERIOD_1.CTS_PERIOD_1
 *           RPT1 --> HDMI CTS_PERIOD_1.CTS_1_REPEAT
 *        Also, HDMI_CRP_CFG.USE_MAI_BUS_SYNC_FOR_CTS_GENERATION must be
 *        set to 0
 *******************************************************************************/
static const BHDM_P_AUDIO_CLK_VALUES BHDM_88_2KHz_AudioClkValues[] =
{   /* SW-N,   HW-N,  CTS_0,  CTS_1, RPT0, RPT1 *//*     TMDS Clock Rate, BPP,   Comp Audio Rate,    ACR Rate*/

    {  12544,      0,  28000,  28000,    1,    1 }, /*   0 *//*            25.2 MHz,  24,     88.200000 kHz,     900 Hz *//*   Spec Settings */
    {  12544,      0,  35000,  35000,    1,    1 }, /*   1 *//*            31.5 MHz,  30,     88.200000 kHz,     900 Hz *//*   Spec Settings */
    {  12544,      0,  42000,  42000,    1,    1 }, /*   2 *//*            37.8 MHz,  36,     88.200000 kHz,     900 Hz *//*   Spec Settings */

    {  14014,      0,  31250,  31250,    1,    1 }, /*   3 *//*    25.2 / 1.001 MHz,  24,     88.200000 kHz, 805.594 Hz *//*   Spec Settings */
    {  28028,      0,  78125,  78125,    1,    1 }, /*   4 *//*    31.5 / 1.001 MHz,  30,     88.200000 kHz, 402.797 Hz *//*   Spec Settings */
    {  14014,      0,  46875,  46875,    1,    1 }, /*   5 *//*    37.8 / 1.001 MHz,  36,     88.200000 kHz, 805.594 Hz *//*   Spec Settings */

    {  12544,      0,  30000,  30000,    1,    1 }, /*   6 *//*              27 MHz,  24,     88.200000 kHz,     900 Hz *//*   Spec Settings */
    {  12544,      0,  37500,  37500,    1,    1 }, /*   7 *//*           33.75 MHz,  30,     88.200000 kHz,     900 Hz *//*   Spec Settings */
    {  12544,      0,  45000,  45000,    1,    1 }, /*   8 *//*            40.5 MHz,  36,     88.200000 kHz,     900 Hz *//*   Spec Settings */
    {  12544,      0,  60000,  60000,    1,    1 }, /*   9 *//*              54 MHz,  24,     88.200000 kHz,     900 Hz *//*   Spec Settings */
    {  12544,      0,  75000,  75000,    1,    1 }, /*  10 *//*            67.5 MHz,  30,     88.200000 kHz,     900 Hz *//*   Spec Settings */
    {  12544,      0,  90000,  90000,    1,    1 }, /*  11 *//*              81 MHz,  36,     88.200000 kHz,     900 Hz *//*   Spec Settings */
    {  11368,      0, 108750, 108750,    1,    1 }, /*  12 *//*             108 MHz,  24,     88.200000 kHz, 993.103 Hz *//* No Settings Found */
    {  10976,      0, 131250, 131250,    1,    1 }, /*  13 *//*             135 MHz,  30,     88.200000 kHz, 1028.57 Hz *//* No Settings Found */
    {  11368,      0, 163125, 163125,    1,    1 }, /*  14 *//*             162 MHz,  36,     88.200000 kHz, 993.103 Hz *//* No Settings Found */

    {  12544,      0,  30030,  30030,    1,    1 }, /*  15 *//*      27 * 1.001 MHz,  24,     88.200000 kHz,     900 Hz *//*   Spec Settings */
    {  25088,      0,  75075,  75075,    1,    1 }, /*  16 *//*   33.75 * 1.001 MHz,  30,     88.200000 kHz,     450 Hz *//*   Spec Settings */
    {  12544,      0,  45045,  45045,    1,    1 }, /*  17 *//*    40.5 * 1.001 MHz,  36,     88.200000 kHz,     900 Hz *//*   Spec Settings */
    {  12544,      0,  60060,  60060,    1,    1 }, /*  18 *//*      54 * 1.001 MHz,  24,     88.200000 kHz,     900 Hz *//*   Spec Settings */
    {  12544,      0,  75075,  75075,    1,    1 }, /*  19 *//*    67.5 * 1.001 MHz,  30,     88.200000 kHz,     900 Hz *//*   Spec Settings */
    {  12544,      0,  90090,  90090,    1,    1 }, /*  20 *//*      81 * 1.001 MHz,  36,     88.200000 kHz,     900 Hz *//*   Spec Settings */
    {  11200,      0, 107250, 107250,    1,    1 }, /*  21 *//*     108 * 1.001 MHz,  24,     88.200000 kHz,    1008 Hz *//* No Settings Found */
    {  11648,      0, 139425, 139425,    1,    1 }, /*  22 *//*     135 * 1.001 MHz,  30,     88.200000 kHz, 969.231 Hz *//* No Settings Found */
    {  11200,      0, 160875, 160875,    1,    1 }, /*  23 *//*     162 * 1.001 MHz,  36,     88.200000 kHz,    1008 Hz *//* No Settings Found */



    {  12544,      0,  82500,  82500,    1,    1 }, /*  24 *//*           74.25 MHz,  24,     88.200000 kHz,     900 Hz *//*   Spec Settings */
    {  12544,      0, 103125, 103125,    1,    1 }, /*  25 *//*         92.8125 MHz,  30,     88.200000 kHz,     900 Hz *//*   Spec Settings */
    {  12544,      0, 123750, 123750,    1,    1 }, /*  26 *//*         111.375 MHz,  36,     88.200000 kHz,     900 Hz *//*   Spec Settings */

    {  35672,      0, 234375, 234375,    1,    1 }, /*  27 *//*   74.25 / 1.001 MHz,  24,     88.200000 kHz, 316.484 Hz *//*   Spec Settings */
    {  35672,      0, 292968, 292969,    1,    3 }, /*  28 *//* 92.8125 / 1.001 MHz,  30,     88.200000 kHz, 316.484 Hz *//*   Spec Settings */
    {  35672,      0, 351562, 351563,    1,    1 }, /*  29 *//* 111.375 / 1.001 MHz,  36,     88.200000 kHz, 316.484 Hz *//*   Spec Settings */


    {  12544,      0, 165000, 165000,    1,    1 }, /*  30 *//*           148.5 MHz,  24,     88.200000 kHz,     900 Hz *//*   Spec Settings */
    {  12544,      0, 206250, 206250,    1,    1 }, /*  31 *//*         185.625 MHz,  30,     88.200000 kHz,     900 Hz *//*   Spec Settings */
    {  12544,      0, 247500, 247500,    1,    1 }, /*  32 *//*          222.75 MHz,  36,     88.200000 kHz,     900 Hz *//*   Spec Settings */

    {  17836,      0, 234375, 234375,    1,    1 }, /*  33 *//*   148.5 / 1.001 MHz,  24,     88.200000 kHz, 632.967 Hz *//*   Spec Settings */
    {  35672,      0, 585937, 585938,    1,    1 }, /*  34 *//* 185.625 / 1.001 MHz,  30,     88.200000 kHz, 316.484 Hz *//*   Spec Settings */
    {  35672,      0, 703125, 703125,    1,    1 }, /*  35 *//*  222.75 / 1.001 MHz,  36,     88.200000 kHz, 316.484 Hz *//*   Spec Settings */

    {   9408,      0, 247500, 247500,    1,    1 }, /*  36 *//*             297 MHz,  24,     88.200000 kHz,    1200 Hz *//*   Spec Settings */
    {   9408,      0, 309375, 309375,    1,    1 }, /*  37 *//*          371.25 MHz,  30,     88.200000 kHz,    1200 Hz *//*   Spec Settings */
    {   9408,      0, 371250, 371250,    1,    1 }, /*  38 *//*           445.5 MHz,  36,     88.200000 kHz,    1200 Hz *//*   Spec Settings */

    {   8918,      0, 234375, 234375,    1,    1 }, /*  39 *//*     297 / 1.001 MHz,  24,     88.200000 kHz, 1265.93 Hz *//*   Spec Settings */
    {  17836,      0, 585937, 585938,    1,    1 }, /*  40 *//*  371.25 / 1.001 MHz,  30,     88.200000 kHz, 632.967 Hz *//*   Spec Settings */
    {  17836,      0, 703125, 703125,    1,    1 }, /*  41 *//*   445.5 / 1.001 MHz,  36,     88.200000 kHz, 632.967 Hz *//*   Spec Settings */

    {  18816,      0, 990000, 990000,    1,    1 }, /*  42 *//*             594 MHz,  24,     88.200000 kHz,     600 Hz *//*   Spec Settings */

    {  17836,      0, 937500, 937500,    1,    1 }, /*  43 *//*     594 / 1.001 MHz,  24,     88.200000 kHz, 632.967 Hz *//*   Spec Settings */

    {  14112,      0,  81250,  81250,    1,    1 }, /*  44 *//*              65 MHz,  24,     88.200000 kHz,     800 Hz *//* No Settings Found */
    {  28224,      0, 203125, 203125,    1,    1 }, /*  45 *//*           81.25 MHz,  30,     88.200000 kHz,     400 Hz *//* No Settings Found */
    {   9408,      0,  81250,  81250,    1,    1 }, /*  46 *//*            97.5 MHz,  36,     88.200000 kHz,    1200 Hz *//* No Settings Found */

    {  33957,      0, 195312, 195313,    1,    1 }, /*  47 *//*      65 / 1.001 MHz,  24,     88.200000 kHz, 332.468 Hz *//* No Settings Found */
    {  33957,      0, 244140, 244141,    3,    5 }, /*  48 *//*   81.25 / 1.001 MHz,  30,     88.200000 kHz, 332.468 Hz *//* No Settings Found */
    {  22638,      0, 195312, 195313,    1,    1 }, /*  49 *//*    97.5 / 1.001 MHz,  36,     88.200000 kHz, 498.701 Hz *//* No Settings Found */


};

/******************************************************************************
 * Summary:
 * HDMI Audio Clock Capture and Regeneration Values for 176.4 kHz
 * NOTE: The order of these entries must match the order of the entries in the
 *       BHDM_P_TmdsClock enum declaration
 * USAGE: S/W must write the values in the following table to the appropriate
 *        registers:
 *           CTS0 --> HDMI_CTS_0.CTS_0
 *           CTS0 --> HDMI CTS_PERIOD_0.CTS_PERIOD_0
 *           RPT0 --> HDMI CTS_PERIOD_0.CTS_0_REPEAT
 *           CTS1 --> HDMI_CTS_1.CTS_1
 *           CTS1 --> HDMI CTS_PERIOD_1.CTS_PERIOD_1
 *           RPT1 --> HDMI CTS_PERIOD_1.CTS_1_REPEAT
 *        Also, HDMI_CRP_CFG.USE_MAI_BUS_SYNC_FOR_CTS_GENERATION must be
 *        set to 0
 *******************************************************************************/
static const BHDM_P_AUDIO_CLK_VALUES BHDM_176_4KHz_AudioClkValues[] =
{   /* SW-N,   HW-N,  CTS_0,  CTS_1, RPT0, RPT1 *//*     TMDS Clock Rate, BPP,   Comp Audio Rate,    ACR Rate*/

    {  25088,      0,  28000,  28000,    1,    1 }, /*   0 *//*            25.2 MHz,  24,    176.400000 kHz,     900 Hz *//*   Spec Settings */
    {  25088,      0,  35000,  35000,    1,    1 }, /*   1 *//*            31.5 MHz,  30,    176.400000 kHz,     900 Hz *//*   Spec Settings */
    {  25088,      0,  42000,  42000,    1,    1 }, /*   2 *//*            37.8 MHz,  36,    176.400000 kHz,     900 Hz *//*   Spec Settings */

    {  28028,      0,  31250,  31250,    1,    1 }, /*   3 *//*    25.2 / 1.001 MHz,  24,    176.400000 kHz, 805.594 Hz *//*   Spec Settings */
    {  56056,      0,  78125,  78125,    1,    1 }, /*   4 *//*    31.5 / 1.001 MHz,  30,    176.400000 kHz, 402.797 Hz *//*   Spec Settings */
    {  28028,      0,  46875,  46875,    1,    1 }, /*   5 *//*    37.8 / 1.001 MHz,  36,    176.400000 kHz, 805.594 Hz *//*   Spec Settings */

    {  25088,      0,  30000,  30000,    1,    1 }, /*   6 *//*              27 MHz,  24,    176.400000 kHz,     900 Hz *//*   Spec Settings */
    {  25088,      0,  37500,  37500,    1,    1 }, /*   7 *//*           33.75 MHz,  30,    176.400000 kHz,     900 Hz *//*   Spec Settings */
    {  25088,      0,  45000,  45000,    1,    1 }, /*   8 *//*            40.5 MHz,  36,    176.400000 kHz,     900 Hz *//*   Spec Settings */
    {  25088,      0,  60000,  60000,    1,    1 }, /*   9 *//*              54 MHz,  24,    176.400000 kHz,     900 Hz *//*   Spec Settings */
    {  25088,      0,  75000,  75000,    1,    1 }, /*  10 *//*            67.5 MHz,  30,    176.400000 kHz,     900 Hz *//*   Spec Settings */
    {  25088,      0,  90000,  90000,    1,    1 }, /*  11 *//*              81 MHz,  36,    176.400000 kHz,     900 Hz *//*   Spec Settings */
    {  22736,      0, 108750, 108750,    1,    1 }, /*  12 *//*             108 MHz,  24,    176.400000 kHz, 993.103 Hz *//* No Settings Found */
    {  21952,      0, 131250, 131250,    1,    1 }, /*  13 *//*             135 MHz,  30,    176.400000 kHz, 1028.57 Hz *//* No Settings Found */
    {  22736,      0, 163125, 163125,    1,    1 }, /*  14 *//*             162 MHz,  36,    176.400000 kHz, 993.103 Hz *//* No Settings Found */

    {  25088,      0,  30030,  30030,    1,    1 }, /*  15 *//*      27 * 1.001 MHz,  24,    176.400000 kHz,     900 Hz *//*   Spec Settings */
    {  50176,      0,  75075,  75075,    1,    1 }, /*  16 *//*   33.75 * 1.001 MHz,  30,    176.400000 kHz,     450 Hz *//*   Spec Settings */
    {  25088,      0,  45045,  45045,    1,    1 }, /*  17 *//*    40.5 * 1.001 MHz,  36,    176.400000 kHz,     900 Hz *//*   Spec Settings */
    {  25088,      0,  60060,  60060,    1,    1 }, /*  18 *//*      54 * 1.001 MHz,  24,    176.400000 kHz,     900 Hz *//*   Spec Settings */
    {  25088,      0,  75075,  75075,    1,    1 }, /*  19 *//*    67.5 * 1.001 MHz,  30,    176.400000 kHz,     900 Hz *//*   Spec Settings */
    {  25088,      0,  90090,  90090,    1,    1 }, /*  20 *//*      81 * 1.001 MHz,  36,    176.400000 kHz,     900 Hz *//*   Spec Settings */
    {  22400,      0, 107250, 107250,    1,    1 }, /*  21 *//*     108 * 1.001 MHz,  24,    176.400000 kHz,    1008 Hz *//* No Settings Found */
    {  23296,      0, 139425, 139425,    1,    1 }, /*  22 *//*     135 * 1.001 MHz,  30,    176.400000 kHz, 969.231 Hz *//* No Settings Found */
    {  22400,      0, 160875, 160875,    1,    1 }, /*  23 *//*     162 * 1.001 MHz,  36,    176.400000 kHz,    1008 Hz *//* No Settings Found */



    {  25088,      0,  82500,  82500,    1,    1 }, /*  24 *//*           74.25 MHz,  24,    176.400000 kHz,     900 Hz *//*   Spec Settings */
    {  25088,      0, 103125, 103125,    1,    1 }, /*  25 *//*         92.8125 MHz,  30,    176.400000 kHz,     900 Hz *//*   Spec Settings */
    {  25088,      0, 123750, 123750,    1,    1 }, /*  26 *//*         111.375 MHz,  36,    176.400000 kHz,     900 Hz *//*   Spec Settings */

    {  71344,      0, 234375, 234375,    1,    1 }, /*  27 *//*   74.25 / 1.001 MHz,  24,    176.400000 kHz, 316.484 Hz *//*   Spec Settings */
    {  71344,      0, 292968, 292969,    1,    3 }, /*  28 *//* 92.8125 / 1.001 MHz,  30,    176.400000 kHz, 316.484 Hz *//*   Spec Settings */
    {  71344,      0, 351562, 351563,    1,    1 }, /*  29 *//* 111.375 / 1.001 MHz,  36,    176.400000 kHz, 316.484 Hz *//*   Spec Settings */


    {  25088,      0, 165000, 165000,    1,    1 }, /*  30 *//*           148.5 MHz,  24,    176.400000 kHz,     900 Hz *//*   Spec Settings */
    {  25088,      0, 206250, 206250,    1,    1 }, /*  31 *//*         185.625 MHz,  30,    176.400000 kHz,     900 Hz *//*   Spec Settings */
    {  25088,      0, 247500, 247500,    1,    1 }, /*  32 *//*          222.75 MHz,  36,    176.400000 kHz,     900 Hz *//*   Spec Settings */

    {  35672,      0, 234375, 234375,    1,    1 }, /*  33 *//*   148.5 / 1.001 MHz,  24,    176.400000 kHz, 632.967 Hz *//*   Spec Settings */
    {  71344,      0, 585937, 585938,    1,    1 }, /*  34 *//* 185.625 / 1.001 MHz,  30,    176.400000 kHz, 316.484 Hz *//*   Spec Settings */
    {  71344,      0, 703125, 703125,    1,    1 }, /*  35 *//*  222.75 / 1.001 MHz,  36,    176.400000 kHz, 316.484 Hz *//*   Spec Settings */

    {  18816,      0, 247500, 247500,    1,    1 }, /*  36 *//*             297 MHz,  24,    176.400000 kHz,    1200 Hz *//*   Spec Settings */
    {  18816,      0, 309375, 309375,    1,    1 }, /*  37 *//*          371.25 MHz,  30,    176.400000 kHz,    1200 Hz *//*   Spec Settings */
    {  18816,      0, 371250, 371250,    1,    1 }, /*  38 *//*           445.5 MHz,  36,    176.400000 kHz,    1200 Hz *//*   Spec Settings */

    {  17836,      0, 234375, 234375,    1,    1 }, /*  39 *//*     297 / 1.001 MHz,  24,    176.400000 kHz, 1265.93 Hz *//*   Spec Settings */
    {  35672,      0, 585937, 585938,    1,    1 }, /*  40 *//*  371.25 / 1.001 MHz,  30,    176.400000 kHz, 632.967 Hz *//*   Spec Settings */
    {  35672,      0, 703125, 703125,    1,    1 }, /*  41 *//*   445.5 / 1.001 MHz,  36,    176.400000 kHz, 632.967 Hz *//*   Spec Settings */

    {  37632,      0, 990000, 990000,    1,    1 }, /*  42 *//*             594 MHz,  24,    176.400000 kHz,     600 Hz *//*   Spec Settings */

    {  35672,      0, 937500, 937500,    1,    1 }, /*  43 *//*     594 / 1.001 MHz,  24,    176.400000 kHz, 632.967 Hz *//*   Spec Settings */

    {  28224,      0,  81250,  81250,    1,    1 }, /*  44 *//*              65 MHz,  24,    176.400000 kHz,     800 Hz *//* No Settings Found */
    {  56448,      0, 203125, 203125,    1,    1 }, /*  45 *//*           81.25 MHz,  30,    176.400000 kHz,     400 Hz *//* No Settings Found */
    {  18816,      0,  81250,  81250,    1,    1 }, /*  46 *//*            97.5 MHz,  36,    176.400000 kHz,    1200 Hz *//* No Settings Found */

    {  67914,      0, 195312, 195313,    1,    1 }, /*  47 *//*      65 / 1.001 MHz,  24,    176.400000 kHz, 332.468 Hz *//* No Settings Found */
    {  67914,      0, 244140, 244141,    3,    5 }, /*  48 *//*   81.25 / 1.001 MHz,  30,    176.400000 kHz, 332.468 Hz *//* No Settings Found */
    {  45276,      0, 195312, 195313,    1,    1 }, /*  49 *//*    97.5 / 1.001 MHz,  36,    176.400000 kHz, 498.701 Hz *//* No Settings Found */


};

/******************************************************************************
 * Summary:
 * HDMI Audio Clock Capture and Regeneration Values for 48 kHz
 * NOTE: The order of these entries must match the order of the entries in the
 *       BHDM_P_TmdsClock enum declaration
 * USAGE: S/W must write the values in the following table to the appropriate
 *        registers:
 *           CTS0 --> HDMI_CTS_0.CTS_0
 *           CTS0 --> HDMI CTS_PERIOD_0.CTS_PERIOD_0
 *           RPT0 --> HDMI CTS_PERIOD_0.CTS_0_REPEAT
 *           CTS1 --> HDMI_CTS_1.CTS_1
 *           CTS1 --> HDMI CTS_PERIOD_1.CTS_PERIOD_1
 *           RPT1 --> HDMI CTS_PERIOD_1.CTS_1_REPEAT
 *        Also, HDMI_CRP_CFG.USE_MAI_BUS_SYNC_FOR_CTS_GENERATION must be
 *        set to 0
 *******************************************************************************/
static const BHDM_P_AUDIO_CLK_VALUES BHDM_48KHz_AudioClkValues[] =
{   /* SW-N,   HW-N,  CTS_0,  CTS_1, RPT0, RPT1 *//*     TMDS Clock Rate, BPP,   Comp Audio Rate,    ACR Rate*/

    {   6144,   6144,  25200,  25200,    1,    1 }, /*   0 *//*            25.2 MHz,  24,     48.000000 kHz,    1000 Hz *//*   Spec Settings */
    {   6144,   6144,  31500,  31500,    1,    1 }, /*   1 *//*            31.5 MHz,  30,     48.000000 kHz,    1000 Hz *//*   Spec Settings */
    {   6144,   6144,  37800,  37800,    1,    1 }, /*   2 *//*            37.8 MHz,  36,     48.000000 kHz,    1000 Hz *//*   Spec Settings */

    {   6864,   6144,  28125,  28125,    1,    1 }, /*   3 *//*    25.2 / 1.001 MHz,  24,     48.000000 kHz, 895.105 Hz *//*   Spec Settings */
    {   9152,   6144,  46875,  46875,    1,    1 }, /*   4 *//*    31.5 / 1.001 MHz,  30,     48.000000 kHz, 671.329 Hz *//*   Spec Settings */
    {   9152,   6144,  56250,  56250,    1,    1 }, /*   5 *//*    37.8 / 1.001 MHz,  36,     48.000000 kHz, 671.329 Hz *//*   Spec Settings */

    {   6144,   6144,  27000,  27000,    1,    1 }, /*   6 *//*              27 MHz,  24,     48.000000 kHz,    1000 Hz *//*   Spec Settings */
    {   6144,   6144,  33750,  33750,    1,    1 }, /*   7 *//*           33.75 MHz,  30,     48.000000 kHz,    1000 Hz *//*   Spec Settings */
    {   6144,   6144,  40500,  40500,    1,    1 }, /*   8 *//*            40.5 MHz,  36,     48.000000 kHz,    1000 Hz *//*   Spec Settings */
    {   6144,   6144,  54000,  54000,    1,    1 }, /*   9 *//*              54 MHz,  24,     48.000000 kHz,    1000 Hz *//*   Spec Settings */
    {   6144,   6144,  67500,  67500,    1,    1 }, /*  10 *//*            67.5 MHz,  30,     48.000000 kHz,    1000 Hz *//*   Spec Settings */
    {   6144,   6144,  81000,  81000,    1,    1 }, /*  11 *//*              81 MHz,  36,     48.000000 kHz,    1000 Hz *//*   Spec Settings */
    {   6144,   6144, 108000, 108000,    1,    1 }, /*  12 *//*             108 MHz,  24,     48.000000 kHz,    1000 Hz *//* No Settings Found */
    {   6144,   6144, 135000, 135000,    1,    1 }, /*  13 *//*             135 MHz,  30,     48.000000 kHz,    1000 Hz *//* No Settings Found */
    {   6144,   6144, 162000, 162000,    1,    1 }, /*  14 *//*             162 MHz,  36,     48.000000 kHz,    1000 Hz *//* No Settings Found */

    {   6144,   6144,  27027,  27027,    1,    1 }, /*  15 *//*      27 * 1.001 MHz,  24,     48.000000 kHz,    1000 Hz *//*   Spec Settings */
    {   8192,   6144,  45045,  45045,    1,    1 }, /*  16 *//*   33.75 * 1.001 MHz,  30,     48.000000 kHz,     750 Hz *//*   Spec Settings */
    {   8192,   6144,  54054,  54054,    1,    1 }, /*  17 *//*    40.5 * 1.001 MHz,  36,     48.000000 kHz,     750 Hz *//*   Spec Settings */
    {   6144,   6144,  54054,  54054,    1,    1 }, /*  18 *//*      54 * 1.001 MHz,  24,     48.000000 kHz,    1000 Hz *//*   Spec Settings */
    {   8192,   6144,  90090,  90090,    1,    1 }, /*  19 *//*    67.5 * 1.001 MHz,  30,     48.000000 kHz,     750 Hz *//*   Spec Settings */
    {   6144,   6144,  81081,  81081,    1,    1 }, /*  20 *//*      81 * 1.001 MHz,  36,     48.000000 kHz,    1000 Hz *//*   Spec Settings */
    {   6144,   6144, 108108, 108108,    1,    1 }, /*  21 *//*     108 * 1.001 MHz,  24,     48.000000 kHz,    1000 Hz *//* No Settings Found */
    {   6144,   6144, 135135, 135135,    1,    1 }, /*  22 *//*     135 * 1.001 MHz,  30,     48.000000 kHz,    1000 Hz *//* No Settings Found */
    {   6144,   6144, 162162, 162162,    1,    1 }, /*  23 *//*     162 * 1.001 MHz,  36,     48.000000 kHz,    1000 Hz *//* No Settings Found */



    {   6144,   6144,  74250,  74250,    1,    1 }, /*  24 *//*           74.25 MHz,  24,     48.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  12288,   6144, 185625, 185625,    1,    1 }, /*  25 *//*         92.8125 MHz,  30,     48.000000 kHz,     500 Hz *//*   Spec Settings */
    {   6144,   6144, 111375, 111375,    1,    1 }, /*  26 *//*         111.375 MHz,  36,     48.000000 kHz,    1000 Hz *//*   Spec Settings */

    {  11648,   6144, 140625, 140625,    1,    1 }, /*  27 *//*   74.25 / 1.001 MHz,  24,     48.000000 kHz, 527.473 Hz *//*   Spec Settings */
    {  11648,   6144, 175781, 175782,    3,    1 }, /*  28 *//* 92.8125 / 1.001 MHz,  30,     48.000000 kHz, 527.473 Hz *//*   Spec Settings */
    {  11648,   6144, 210937, 210938,    1,    1 }, /*  29 *//* 111.375 / 1.001 MHz,  36,     48.000000 kHz, 527.473 Hz *//*   Spec Settings */


    {   6144,   6144, 148500, 148500,    1,    1 }, /*  30 *//*           148.5 MHz,  24,     48.000000 kHz,    1000 Hz *//*   Spec Settings */
    {   6144,   6144, 185625, 185625,    1,    1 }, /*  31 *//*         185.625 MHz,  30,     48.000000 kHz,    1000 Hz *//*   Spec Settings */
    {   6144,   6144, 222750, 222750,    1,    1 }, /*  32 *//*          222.75 MHz,  36,     48.000000 kHz,    1000 Hz *//*   Spec Settings */

    {   5824,   6144, 140625, 140625,    1,    1 }, /*  33 *//*   148.5 / 1.001 MHz,  24,     48.000000 kHz, 1054.95 Hz *//*   Spec Settings */
    {  11648,   6144, 351562, 351563,    1,    1 }, /*  34 *//* 185.625 / 1.001 MHz,  30,     48.000000 kHz, 527.473 Hz *//*   Spec Settings */
    {  11648,   6144, 421875, 421875,    1,    1 }, /*  35 *//*  222.75 / 1.001 MHz,  36,     48.000000 kHz, 527.473 Hz *//*   Spec Settings */

    {   5120,   6144, 247500, 247500,    1,    1 }, /*  36 *//*             297 MHz,  24,     48.000000 kHz,    1200 Hz *//*   Spec Settings */
    {   5120,   6144, 309375, 309375,    1,    1 }, /*  37 *//*          371.25 MHz,  30,     48.000000 kHz,    1200 Hz *//*   Spec Settings */
    {   5120,   6144, 371250, 371250,    1,    1 }, /*  38 *//*           445.5 MHz,  36,     48.000000 kHz,    1200 Hz *//*   Spec Settings */

    {   5824,   6144, 281250, 281250,    1,    1 }, /*  39 *//*     297 / 1.001 MHz,  24,     48.000000 kHz, 1054.95 Hz *//*   Spec Settings */
    {  11648,   6144, 703125, 703125,    1,    1 }, /*  40 *//*  371.25 / 1.001 MHz,  30,     48.000000 kHz, 527.473 Hz *//*   Spec Settings */
    {   5824,   6144, 421875, 421875,    1,    1 }, /*  41 *//*   445.5 / 1.001 MHz,  36,     48.000000 kHz, 1054.95 Hz *//*   Spec Settings */

    {   6144,   6144, 594000, 594000,    1,    1 }, /*  42 *//*             594 MHz,  24,     48.000000 kHz,    1000 Hz *//*   Spec Settings */

    {   5824,   6144, 562500, 562500,    1,    1 }, /*  43 *//*     594 / 1.001 MHz,  24,     48.000000 kHz, 1054.95 Hz *//*   Spec Settings */

    {   6144,   6144,  65000,  65000,    1,    1 }, /*  44 *//*              65 MHz,  24,     48.000000 kHz,    1000 Hz *//* No Settings Found */
    {   6144,   6144,  81250,  81250,    1,    1 }, /*  45 *//*           81.25 MHz,  30,     48.000000 kHz,    1000 Hz *//* No Settings Found */
    {   6144,   6144,  97500,  97500,    1,    1 }, /*  46 *//*            97.5 MHz,  36,     48.000000 kHz,    1000 Hz *//* No Settings Found */

    {   7392,   6144,  78125,  78125,    1,    1 }, /*  47 *//*      65 / 1.001 MHz,  24,     48.000000 kHz, 831.169 Hz *//* No Settings Found */
    {  14784,   6144, 195312, 195313,    1,    1 }, /*  48 *//*   81.25 / 1.001 MHz,  30,     48.000000 kHz, 415.584 Hz *//* No Settings Found */
    {   4928,   6144,  78125,  78125,    1,    1 }, /*  49 *//*    97.5 / 1.001 MHz,  36,     48.000000 kHz, 1246.75 Hz *//* No Settings Found */


};

/******************************************************************************
 * Summary:
 * HDMI Audio Clock Capture and Regeneration Values for 96 kHz
 * NOTE: The order of these entries must match the order of the entries in the
 *       BHDM_P_TmdsClock enum declaration
 * USAGE: S/W must write the values in the following table to the appropriate
 *        registers:
 *           CTS0 --> HDMI_CTS_0.CTS_0
 *           CTS0 --> HDMI CTS_PERIOD_0.CTS_PERIOD_0
 *           RPT0 --> HDMI CTS_PERIOD_0.CTS_0_REPEAT
 *           CTS1 --> HDMI_CTS_1.CTS_1
 *           CTS1 --> HDMI CTS_PERIOD_1.CTS_PERIOD_1
 *           RPT1 --> HDMI CTS_PERIOD_1.CTS_1_REPEAT
 *        Also, HDMI_CRP_CFG.USE_MAI_BUS_SYNC_FOR_CTS_GENERATION must be
 *        set to 0
 *******************************************************************************/
static const BHDM_P_AUDIO_CLK_VALUES BHDM_96KHz_AudioClkValues[] =
{   /* SW-N,   HW-N,  CTS_0,  CTS_1, RPT0, RPT1 *//*     TMDS Clock Rate, BPP,   Comp Audio Rate,    ACR Rate*/

    {  12288,  12288,  25200,  25200,    1,    1 }, /*   0 *//*            25.2 MHz,  24,     96.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  12288,  12288,  31500,  31500,    1,    1 }, /*   1 *//*            31.5 MHz,  30,     96.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  12288,  12288,  37800,  37800,    1,    1 }, /*   2 *//*            37.8 MHz,  36,     96.000000 kHz,    1000 Hz *//*   Spec Settings */

    {  13728,  12288,  28125,  28125,    1,    1 }, /*   3 *//*    25.2 / 1.001 MHz,  24,     96.000000 kHz, 895.105 Hz *//*   Spec Settings */
    {  18304,  12288,  46875,  46875,    1,    1 }, /*   4 *//*    31.5 / 1.001 MHz,  30,     96.000000 kHz, 671.329 Hz *//*   Spec Settings */
    {  18304,  12288,  56250,  56250,    1,    1 }, /*   5 *//*    37.8 / 1.001 MHz,  36,     96.000000 kHz, 671.329 Hz *//*   Spec Settings */

    {  12288,  12288,  27000,  27000,    1,    1 }, /*   6 *//*              27 MHz,  24,     96.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  12288,  12288,  33750,  33750,    1,    1 }, /*   7 *//*           33.75 MHz,  30,     96.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  12288,  12288,  40500,  40500,    1,    1 }, /*   8 *//*            40.5 MHz,  36,     96.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  12288,  12288,  54000,  54000,    1,    1 }, /*   9 *//*              54 MHz,  24,     96.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  12288,  12288,  67500,  67500,    1,    1 }, /*  10 *//*            67.5 MHz,  30,     96.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  12288,  12288,  81000,  81000,    1,    1 }, /*  11 *//*              81 MHz,  36,     96.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  12288,  12288, 108000, 108000,    1,    1 }, /*  12 *//*             108 MHz,  24,     96.000000 kHz,    1000 Hz *//* No Settings Found */
    {  12288,  12288, 135000, 135000,    1,    1 }, /*  13 *//*             135 MHz,  30,     96.000000 kHz,    1000 Hz *//* No Settings Found */
    {  12288,  12288, 162000, 162000,    1,    1 }, /*  14 *//*             162 MHz,  36,     96.000000 kHz,    1000 Hz *//* No Settings Found */

    {  12288,  12288,  27027,  27027,    1,    1 }, /*  15 *//*      27 * 1.001 MHz,  24,     96.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  16384,  12288,  45045,  45045,    1,    1 }, /*  16 *//*   33.75 * 1.001 MHz,  30,     96.000000 kHz,     750 Hz *//*   Spec Settings */
    {  16384,  12288,  54054,  54054,    1,    1 }, /*  17 *//*    40.5 * 1.001 MHz,  36,     96.000000 kHz,     750 Hz *//*   Spec Settings */
    {  12288,  12288,  54054,  54054,    1,    1 }, /*  18 *//*      54 * 1.001 MHz,  24,     96.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  16384,  12288,  90090,  90090,    1,    1 }, /*  19 *//*    67.5 * 1.001 MHz,  30,     96.000000 kHz,     750 Hz *//*   Spec Settings */
    {  12288,  12288,  81081,  81081,    1,    1 }, /*  20 *//*      81 * 1.001 MHz,  36,     96.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  12288,  12288, 108108, 108108,    1,    1 }, /*  21 *//*     108 * 1.001 MHz,  24,     96.000000 kHz,    1000 Hz *//* No Settings Found */
    {  12288,  12288, 135135, 135135,    1,    1 }, /*  22 *//*     135 * 1.001 MHz,  30,     96.000000 kHz,    1000 Hz *//* No Settings Found */
    {  12288,  12288, 162162, 162162,    1,    1 }, /*  23 *//*     162 * 1.001 MHz,  36,     96.000000 kHz,    1000 Hz *//* No Settings Found */



    {  12288,  12288,  74250,  74250,    1,    1 }, /*  24 *//*           74.25 MHz,  24,     96.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  24576,  12288, 185625, 185625,    1,    1 }, /*  25 *//*         92.8125 MHz,  30,     96.000000 kHz,     500 Hz *//*   Spec Settings */
    {  12288,  12288, 111375, 111375,    1,    1 }, /*  26 *//*         111.375 MHz,  36,     96.000000 kHz,    1000 Hz *//*   Spec Settings */

    {  23296,  12288, 140625, 140625,    1,    1 }, /*  27 *//*   74.25 / 1.001 MHz,  24,     96.000000 kHz, 527.473 Hz *//*   Spec Settings */
    {  23296,  12288, 175781, 175782,    3,    1 }, /*  28 *//* 92.8125 / 1.001 MHz,  30,     96.000000 kHz, 527.473 Hz *//*   Spec Settings */
    {  23296,  12288, 210937, 210938,    1,    1 }, /*  29 *//* 111.375 / 1.001 MHz,  36,     96.000000 kHz, 527.473 Hz *//*   Spec Settings */


    {  12288,  12288, 148500, 148500,    1,    1 }, /*  30 *//*           148.5 MHz,  24,     96.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  12288,  12288, 185625, 185625,    1,    1 }, /*  31 *//*         185.625 MHz,  30,     96.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  12288,  12288, 222750, 222750,    1,    1 }, /*  32 *//*          222.75 MHz,  36,     96.000000 kHz,    1000 Hz *//*   Spec Settings */

    {  11648,  12288, 140625, 140625,    1,    1 }, /*  33 *//*   148.5 / 1.001 MHz,  24,     96.000000 kHz, 1054.95 Hz *//*   Spec Settings */
    {  23296,  12288, 351562, 351563,    1,    1 }, /*  34 *//* 185.625 / 1.001 MHz,  30,     96.000000 kHz, 527.473 Hz *//*   Spec Settings */
    {  23296,  12288, 421875, 421875,    1,    1 }, /*  35 *//*  222.75 / 1.001 MHz,  36,     96.000000 kHz, 527.473 Hz *//*   Spec Settings */

    {  10240,  12288, 247500, 247500,    1,    1 }, /*  36 *//*             297 MHz,  24,     96.000000 kHz,    1200 Hz *//*   Spec Settings */
    {  10240,  12288, 309375, 309375,    1,    1 }, /*  37 *//*          371.25 MHz,  30,     96.000000 kHz,    1200 Hz *//*   Spec Settings */
    {  10240,  12288, 371250, 371250,    1,    1 }, /*  38 *//*           445.5 MHz,  36,     96.000000 kHz,    1200 Hz *//*   Spec Settings */

    {  11648,  12288, 281250, 281250,    1,    1 }, /*  39 *//*     297 / 1.001 MHz,  24,     96.000000 kHz, 1054.95 Hz *//*   Spec Settings */
    {  23296,  12288, 703125, 703125,    1,    1 }, /*  40 *//*  371.25 / 1.001 MHz,  30,     96.000000 kHz, 527.473 Hz *//*   Spec Settings */
    {  11648,  12288, 421875, 421875,    1,    1 }, /*  41 *//*   445.5 / 1.001 MHz,  36,     96.000000 kHz, 1054.95 Hz *//*   Spec Settings */

    {  12288,  12288, 594000, 594000,    1,    1 }, /*  42 *//*             594 MHz,  24,     96.000000 kHz,    1000 Hz *//*   Spec Settings */

    {  11648,  12288, 562500, 562500,    1,    1 }, /*  43 *//*     594 / 1.001 MHz,  24,     96.000000 kHz, 1054.95 Hz *//*   Spec Settings */

    {  12288,  12288,  65000,  65000,    1,    1 }, /*  44 *//*              65 MHz,  24,     96.000000 kHz,    1000 Hz *//* No Settings Found */
    {  12288,  12288,  81250,  81250,    1,    1 }, /*  45 *//*           81.25 MHz,  30,     96.000000 kHz,    1000 Hz *//* No Settings Found */
    {  12288,  12288,  97500,  97500,    1,    1 }, /*  46 *//*            97.5 MHz,  36,     96.000000 kHz,    1000 Hz *//* No Settings Found */

    {  14784,  12288,  78125,  78125,    1,    1 }, /*  47 *//*      65 / 1.001 MHz,  24,     96.000000 kHz, 831.169 Hz *//* No Settings Found */
    {  29568,  12288, 195312, 195313,    1,    1 }, /*  48 *//*   81.25 / 1.001 MHz,  30,     96.000000 kHz, 415.584 Hz *//* No Settings Found */
    {   9856,  12288,  78125,  78125,    1,    1 }, /*  49 *//*    97.5 / 1.001 MHz,  36,     96.000000 kHz, 1246.75 Hz *//* No Settings Found */


};

/******************************************************************************
 * Summary:
 * HDMI Audio Clock Capture and Regeneration Values for 192 kHz
 * NOTE: The order of these entries must match the order of the entries in the
 *       BHDM_P_TmdsClock enum declaration
 * USAGE: S/W must write the values in the following table to the appropriate
 *        registers:
 *           CTS0 --> HDMI_CTS_0.CTS_0
 *           CTS0 --> HDMI CTS_PERIOD_0.CTS_PERIOD_0
 *           RPT0 --> HDMI CTS_PERIOD_0.CTS_0_REPEAT
 *           CTS1 --> HDMI_CTS_1.CTS_1
 *           CTS1 --> HDMI CTS_PERIOD_1.CTS_PERIOD_1
 *           RPT1 --> HDMI CTS_PERIOD_1.CTS_1_REPEAT
 *        Also, HDMI_CRP_CFG.USE_MAI_BUS_SYNC_FOR_CTS_GENERATION must be
 *        set to 0
 *******************************************************************************/
static const BHDM_P_AUDIO_CLK_VALUES BHDM_192KHz_AudioClkValues[] =
{   /* SW-N,   HW-N,  CTS_0,  CTS_1, RPT0, RPT1 *//*     TMDS Clock Rate, BPP,   Comp Audio Rate,    ACR Rate*/

    {  24576,  24576,  25200,  25200,    1,    1 }, /*   0 *//*            25.2 MHz,  24,    192.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  24576,  24576,  31500,  31500,    1,    1 }, /*   1 *//*            31.5 MHz,  30,    192.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  24576,  24576,  37800,  37800,    1,    1 }, /*   2 *//*            37.8 MHz,  36,    192.000000 kHz,    1000 Hz *//*   Spec Settings */

    {  27456,  24576,  28125,  28125,    1,    1 }, /*   3 *//*    25.2 / 1.001 MHz,  24,    192.000000 kHz, 895.105 Hz *//*   Spec Settings */
    {  36608,  24576,  46875,  46875,    1,    1 }, /*   4 *//*    31.5 / 1.001 MHz,  30,    192.000000 kHz, 671.329 Hz *//*   Spec Settings */
    {  36608,  24576,  56250,  56250,    1,    1 }, /*   5 *//*    37.8 / 1.001 MHz,  36,    192.000000 kHz, 671.329 Hz *//*   Spec Settings */

    {  24576,  24576,  27000,  27000,    1,    1 }, /*   6 *//*              27 MHz,  24,    192.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  24576,  24576,  33750,  33750,    1,    1 }, /*   7 *//*           33.75 MHz,  30,    192.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  24576,  24576,  40500,  40500,    1,    1 }, /*   8 *//*            40.5 MHz,  36,    192.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  24576,  24576,  54000,  54000,    1,    1 }, /*   9 *//*              54 MHz,  24,    192.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  24576,  24576,  67500,  67500,    1,    1 }, /*  10 *//*            67.5 MHz,  30,    192.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  24576,  24576,  81000,  81000,    1,    1 }, /*  11 *//*              81 MHz,  36,    192.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  24576,  24576, 108000, 108000,    1,    1 }, /*  12 *//*             108 MHz,  24,    192.000000 kHz,    1000 Hz *//* No Settings Found */
    {  24576,  24576, 135000, 135000,    1,    1 }, /*  13 *//*             135 MHz,  30,    192.000000 kHz,    1000 Hz *//* No Settings Found */
    {  24576,  24576, 162000, 162000,    1,    1 }, /*  14 *//*             162 MHz,  36,    192.000000 kHz,    1000 Hz *//* No Settings Found */

    {  24576,  24576,  27027,  27027,    1,    1 }, /*  15 *//*      27 * 1.001 MHz,  24,    192.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  32768,  24576,  45045,  45045,    1,    1 }, /*  16 *//*   33.75 * 1.001 MHz,  30,    192.000000 kHz,     750 Hz *//*   Spec Settings */
    {  32768,  24576,  54054,  54054,    1,    1 }, /*  17 *//*    40.5 * 1.001 MHz,  36,    192.000000 kHz,     750 Hz *//*   Spec Settings */
    {  24576,  24576,  54054,  54054,    1,    1 }, /*  18 *//*      54 * 1.001 MHz,  24,    192.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  32768,  24576,  90090,  90090,    1,    1 }, /*  19 *//*    67.5 * 1.001 MHz,  30,    192.000000 kHz,     750 Hz *//*   Spec Settings */
    {  24576,  24576,  81081,  81081,    1,    1 }, /*  20 *//*      81 * 1.001 MHz,  36,    192.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  24576,  24576, 108108, 108108,    1,    1 }, /*  21 *//*     108 * 1.001 MHz,  24,    192.000000 kHz,    1000 Hz *//* No Settings Found */
    {  24576,  24576, 135135, 135135,    1,    1 }, /*  22 *//*     135 * 1.001 MHz,  30,    192.000000 kHz,    1000 Hz *//* No Settings Found */
    {  24576,  24576, 162162, 162162,    1,    1 }, /*  23 *//*     162 * 1.001 MHz,  36,    192.000000 kHz,    1000 Hz *//* No Settings Found */



    {  24576,  24576,  74250,  74250,    1,    1 }, /*  24 *//*           74.25 MHz,  24,    192.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  49152,  24576, 185625, 185625,    1,    1 }, /*  25 *//*         92.8125 MHz,  30,    192.000000 kHz,     500 Hz *//*   Spec Settings */
    {  24576,  24576, 111375, 111375,    1,    1 }, /*  26 *//*         111.375 MHz,  36,    192.000000 kHz,    1000 Hz *//*   Spec Settings */

    {  46592,  24576, 140625, 140625,    1,    1 }, /*  27 *//*   74.25 / 1.001 MHz,  24,    192.000000 kHz, 527.473 Hz *//*   Spec Settings */
    {  46592,  24576, 175781, 175782,    3,    1 }, /*  28 *//* 92.8125 / 1.001 MHz,  30,    192.000000 kHz, 527.473 Hz *//*   Spec Settings */
    {  46592,  24576, 210937, 210938,    1,    1 }, /*  29 *//* 111.375 / 1.001 MHz,  36,    192.000000 kHz, 527.473 Hz *//*   Spec Settings */


    {  24576,  24576, 148500, 148500,    1,    1 }, /*  30 *//*           148.5 MHz,  24,    192.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  24576,  24576, 185625, 185625,    1,    1 }, /*  31 *//*         185.625 MHz,  30,    192.000000 kHz,    1000 Hz *//*   Spec Settings */
    {  24576,  24576, 222750, 222750,    1,    1 }, /*  32 *//*          222.75 MHz,  36,    192.000000 kHz,    1000 Hz *//*   Spec Settings */

    {  23296,  24576, 140625, 140625,    1,    1 }, /*  33 *//*   148.5 / 1.001 MHz,  24,    192.000000 kHz, 1054.95 Hz *//*   Spec Settings */
    {  46592,  24576, 351562, 351563,    1,    1 }, /*  34 *//* 185.625 / 1.001 MHz,  30,    192.000000 kHz, 527.473 Hz *//*   Spec Settings */
    {  46592,  24576, 421875, 421875,    1,    1 }, /*  35 *//*  222.75 / 1.001 MHz,  36,    192.000000 kHz, 527.473 Hz *//*   Spec Settings */

    {  20480,  24576, 247500, 247500,    1,    1 }, /*  36 *//*             297 MHz,  24,    192.000000 kHz,    1200 Hz *//*   Spec Settings */
    {  20480,  24576, 309375, 309375,    1,    1 }, /*  37 *//*          371.25 MHz,  30,    192.000000 kHz,    1200 Hz *//*   Spec Settings */
    {  20480,  24576, 371250, 371250,    1,    1 }, /*  38 *//*           445.5 MHz,  36,    192.000000 kHz,    1200 Hz *//*   Spec Settings */

    {  23296,  24576, 281250, 281250,    1,    1 }, /*  39 *//*     297 / 1.001 MHz,  24,    192.000000 kHz, 1054.95 Hz *//*   Spec Settings */
    {  46592,  24576, 703125, 703125,    1,    1 }, /*  40 *//*  371.25 / 1.001 MHz,  30,    192.000000 kHz, 527.473 Hz *//*   Spec Settings */
    {  23296,  24576, 421875, 421875,    1,    1 }, /*  41 *//*   445.5 / 1.001 MHz,  36,    192.000000 kHz, 1054.95 Hz *//*   Spec Settings */

    {  24576,  24576, 594000, 594000,    1,    1 }, /*  42 *//*             594 MHz,  24,    192.000000 kHz,    1000 Hz *//*   Spec Settings */

    {  23296,  24576, 562500, 562500,    1,    1 }, /*  43 *//*     594 / 1.001 MHz,  24,    192.000000 kHz, 1054.95 Hz *//*   Spec Settings */

    {  24576,  24576,  65000,  65000,    1,    1 }, /*  44 *//*              65 MHz,  24,    192.000000 kHz,    1000 Hz *//* No Settings Found */
    {  24576,  24576,  81250,  81250,    1,    1 }, /*  45 *//*           81.25 MHz,  30,    192.000000 kHz,    1000 Hz *//* No Settings Found */
    {  24576,  24576,  97500,  97500,    1,    1 }, /*  46 *//*            97.5 MHz,  36,    192.000000 kHz,    1000 Hz *//* No Settings Found */

    {  29568,  24576,  78125,  78125,    1,    1 }, /*  47 *//*      65 / 1.001 MHz,  24,    192.000000 kHz, 831.169 Hz *//* No Settings Found */
    {  59136,  24576, 195312, 195313,    1,    1 }, /*  48 *//*   81.25 / 1.001 MHz,  30,    192.000000 kHz, 415.584 Hz *//* No Settings Found */
    {  19712,  24576,  78125,  78125,    1,    1 }, /*  49 *//*    97.5 / 1.001 MHz,  36,    192.000000 kHz, 1246.75 Hz *//* No Settings Found */


};

BERR_Code BHDM_PACKET_ACR_P_LookupTmdsClock_isrsafe(
	BHDM_Handle hHDMI,
	uint64_t ulPixelClkRate64BitMask, const BHDM_Video_Settings *stVideoSettings,
	BAVC_HDMI_PixelRepetition ePixelRepetition,

	BHDM_P_TmdsClock *eTmdsClock
)
{
	BERR_Code rc = BERR_SUCCESS ;
	BAVC_Colorspace eColorSpace = stVideoSettings->eColorSpace ;

	BAVC_HDMI_BitsPerPixel lookupBitsPerPixel ;
	BAVC_HDMI_AviInfoFrame_Colorspace lookupColorSpace ;
	uint8_t tableIndex ;  /* index into BHDM_SupportedClocks Table */

#if BDBG_DEBUG_BUILD
#else
        BSTD_UNUSED (hHDMI);
#endif

	*eTmdsClock = BHDM_P_TmdsClock_eMax ;
	lookupBitsPerPixel = stVideoSettings->eBitsPerPixel ;
	lookupColorSpace = stVideoSettings->eColorSpace ;

	if (eColorSpace == BAVC_Colorspace_eYCbCr422)
	{
		BDBG_MSG(("Colorspace 4:2:2 n bits is equivalent to 4:4:4 8 bit")) ;
		lookupBitsPerPixel = BAVC_HDMI_BitsPerPixel_e24bit ;
		lookupColorSpace = BAVC_Colorspace_eYCbCr444 ;
	}
	else if (eColorSpace == BAVC_Colorspace_eRGB)
	{
		lookupColorSpace = BAVC_Colorspace_eYCbCr444 ;
	}
	else if (eColorSpace == BAVC_Colorspace_eFuture)
	{
		BDBG_MSG(("Unknown Colorspace (%d); Use YCbCr 4:4:4 for lookup", eColorSpace)) ;
		lookupColorSpace = BAVC_Colorspace_eYCbCr444 ;
	}

#define BHDM_P_DEBUG_TABLE_LOOKUP 0
#if BHDM_P_DEBUG_TABLE_LOOKUP
	/* messages for debugging table look up */
	/* debug code for lookup of TMDS Clock */
	BDBG_LOG(("ACR Packet Original LOOKUP Parameters: ")) ;
	BDBG_LOG(("   Input PxlClkRate " BDBG_UINT64_FMT " ",
		BDBG_UINT64_ARG(ulPixelClkRate64BitMask))) ;
	BDBG_LOG(("   Input eBitsPerPixel = %d  ==> lookup via %d",
		stVideoSettings->eBitsPerPixel, lookupBitsPerPixel)) ;
	BDBG_LOG(("   Input ePixelRepetition = %d ", ePixelRepetition)) ;
	BDBG_LOG(("   Input eColorSpace = %s (%d)  ==> lookup via   %s (%d)",
		BAVC_HDMI_AviInfoFrame_ColorspaceToStr_isrsafe(eColorSpace), eColorSpace,
		BAVC_HDMI_AviInfoFrame_ColorspaceToStr_isrsafe(lookupColorSpace), lookupColorSpace)) ;
#endif

	for (tableIndex = 0 ; tableIndex < sizeof(BHDM_SupportedClocks) / sizeof(BHDM_BAVC_Clock) ; tableIndex++)
	{
		if ((lookupColorSpace == (BAVC_HDMI_AviInfoFrame_Colorspace) BAVC_Colorspace_eYCbCr444)
		|| (lookupColorSpace == (BAVC_HDMI_AviInfoFrame_Colorspace) BAVC_Colorspace_eYCbCr422)
		|| (lookupColorSpace == (BAVC_HDMI_AviInfoFrame_Colorspace) BAVC_Colorspace_eRGB))
		{
			if ((BAVC_Colorspace) BHDM_SupportedClocks[tableIndex].eAviInfoFrame_Colorspace != BAVC_Colorspace_eYCbCr444)
				continue ;
		}
		else if (lookupColorSpace == (BAVC_HDMI_AviInfoFrame_Colorspace) BAVC_Colorspace_eYCbCr420)
		{
			if ((BAVC_Colorspace) BHDM_SupportedClocks[tableIndex].eAviInfoFrame_Colorspace != BAVC_Colorspace_eYCbCr420)
				continue ;
		}
		else
		{
			BDBG_ERR(("Undefined Colored Space: %d at line %d", lookupColorSpace, __LINE__)) ;
		}

		if ((ulPixelClkRate64BitMask & BHDM_SupportedClocks[tableIndex].ulPixelClkRate64BitMask)
		&& (lookupColorSpace == BHDM_SupportedClocks[tableIndex].eAviInfoFrame_Colorspace)
		&& (lookupBitsPerPixel == BHDM_SupportedClocks[tableIndex].eBitsPerPixel)
		&& (ePixelRepetition == BHDM_SupportedClocks[tableIndex].ePixelRepetition))
		{
			*eTmdsClock = BHDM_SupportedClocks[tableIndex].eTmdsClock ;

#if BHDM_P_DEBUG_TABLE_LOOKUP
			BDBG_LOG(("BHDM Supported TMDS Clock Found at index %d: Use %s MHz parameters ",
				tableIndex, BHDM_P_TmdsClockToText_isrsafe(*eTmdsClock))) ;

			/* messages for debugging table look up */
			BDBG_LOG(("#####################################")) ;
			BDBG_LOG(("BHDM_SupportedClocks Index: %d", tableIndex )) ;

			BDBG_LOG(("Input PxlClkRate " BDBG_UINT64_FMT " " "Table PxlClkRate " BDBG_UINT64_FMT " ",
				BDBG_UINT64_ARG(ulPixelClkRate64BitMask),
				BDBG_UINT64_ARG(BHDM_SupportedClocks[tableIndex].ulPixelClkRate64BitMask) ));

			BDBG_LOG(("Input eBitsPerPixel = %d BHDM_SupportedClocks[tableIndex].eBitsPerPixel = %d",
				lookupBitsPerPixel, BHDM_SupportedClocks[tableIndex].eBitsPerPixel)) ;

            BDBG_LOG(("Input ePixelRepetition = %d  BHDM_SupportedClocks[tableIndex].ePixelRepetition= %d",
                ePixelRepetition, BHDM_SupportedClocks[tableIndex].ePixelRepetition)) ;

			BDBG_LOG(("Input eColorSpace = %d  BHDM_SupportedClocks[tableIndex].eAviInfoFrame_Colorspace  %d",
				eColorSpace, (BAVC_Colorspace) BHDM_SupportedClocks[tableIndex].eAviInfoFrame_Colorspace )) ;
			BDBG_LOG(("#####################################")) ;
			BDBG_LOG(("")) ;
#endif
            break ;
        }
    }

	if (*eTmdsClock == BHDM_P_TmdsClock_eMax)
	{
		BDBG_ERR(("Tx%d: Unknown HDMI Pixel Clock Rate: " BDBG_UINT64_FMT " bpp: %d; repetition: %d",
			hHDMI->eCoreId, BDBG_UINT64_ARG(ulPixelClkRate64BitMask), lookupBitsPerPixel, ePixelRepetition)) ;
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		goto done ;
	}




#if BDBG_DEBUG_BUILD
	BDBG_MSG(("Tx%d:   TMDS Pixel Clock Rate:	%s", hHDMI->eCoreId,
		BHDM_P_TmdsClockToText_isrsafe(*eTmdsClock))) ;
#endif


done :
	return rc ;

}


/************************************************
-- Get the Audio Clock Regeneration Packet values (N and CTS)
-- based on the Sample Rate and TMDS Clock
************************************************/
BERR_Code BHDM_PACKET_ACR_P_LookupN_CTSValues_isrsafe(
	BHDM_Handle hHDMI,
	BAVC_AudioSamplingRate eAudioSamplingRate, BHDM_P_TmdsClock eTmdsClock,

	BHDM_P_AUDIO_CLK_VALUES *stAcrPacket
)
{
	BERR_Code rc = BERR_SUCCESS ;
	BHDM_P_AUDIO_CLK_VALUES *pAudioParameters ;

#if BDBG_DEBUG_BUILD
#else
        BSTD_UNUSED (hHDMI);
#endif

#if BDBG_DEBUG_BUILD
	static const char * const AudioSampleRateText[]	= {
		BDBG_STRING("32kHz"), BDBG_STRING("44.1kHz"), BDBG_STRING("48kHz"),
		BDBG_STRING("96kHz"), BDBG_STRING("16kHz"), BDBG_STRING("22_05kHz"),
		BDBG_STRING("24kHz"), BDBG_STRING("64kHz"), BDBG_STRING("88_2kHz"),
		BDBG_STRING("128kHz"), BDBG_STRING("176_4kHz"), BDBG_STRING("192kHz"),
		BDBG_STRING("8kHz"), BDBG_STRING("12kHz"), BDBG_STRING("11_025kHz")
	} ;
#endif

	switch( eAudioSamplingRate )
	{
	case BAVC_AudioSamplingRate_e32k :
		pAudioParameters = (BHDM_P_AUDIO_CLK_VALUES *) &BHDM_32KHz_AudioClkValues ;
		break ;

    case BAVC_AudioSamplingRate_e44_1k :
        pAudioParameters = (BHDM_P_AUDIO_CLK_VALUES *) &BHDM_44_1KHz_AudioClkValues ;
        break ;

    case BAVC_AudioSamplingRate_e48k :
        pAudioParameters = (BHDM_P_AUDIO_CLK_VALUES *) &BHDM_48KHz_AudioClkValues ;
        break ;

#if BHDM_CONFIG_88_2KHZ_AUDIO_SUPPORT
    case BAVC_AudioSamplingRate_e88_2k :
        pAudioParameters = (BHDM_P_AUDIO_CLK_VALUES *) &BHDM_88_2KHz_AudioClkValues ;
        break ;
#endif

#if BHDM_CONFIG_96KHZ_AUDIO_SUPPORT
    case BAVC_AudioSamplingRate_e96k    :
        pAudioParameters = (BHDM_P_AUDIO_CLK_VALUES *) &BHDM_96KHz_AudioClkValues ;
        break ;
#endif

	case BAVC_AudioSamplingRate_e128k :
		pAudioParameters = (BHDM_P_AUDIO_CLK_VALUES *) &BHDM_128KHz_AudioClkValues ;
		break ;

#if BHDM_CONFIG_176_4KHZ_AUDIO_SUPPORT
    case BAVC_AudioSamplingRate_e176_4k    :
        pAudioParameters = (BHDM_P_AUDIO_CLK_VALUES *) &BHDM_176_4KHz_AudioClkValues ;
        break ;
#endif

#if BHDM_CONFIG_192KHZ_AUDIO_SUPPORT
    case BAVC_AudioSamplingRate_e192k:
        pAudioParameters = (BHDM_P_AUDIO_CLK_VALUES *) &BHDM_192KHz_AudioClkValues;
        break ;
#endif

	default :
		BDBG_ERR(("Unsupported eAudioSamplingRate eNumeration: %d", eAudioSamplingRate)) ;
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		goto done ;
	}

	stAcrPacket->NValue = pAudioParameters[eTmdsClock].NValue ;
	stAcrPacket->HW_NValue = pAudioParameters[eTmdsClock].HW_NValue ;
	stAcrPacket->CTS_0 = pAudioParameters[eTmdsClock].CTS_0 ;
	stAcrPacket->CTS_1 = pAudioParameters[eTmdsClock].CTS_1 ;
	stAcrPacket->CTS_PERIOD_0 = pAudioParameters[eTmdsClock].CTS_PERIOD_0 ;
	stAcrPacket->CTS_PERIOD_1 = pAudioParameters[eTmdsClock].CTS_PERIOD_1 ;

#if BDBG_DEBUG_BUILD
	BDBG_MSG(("Tx%d: HDMI Audio Configuration:", hHDMI->eCoreId)) ;
	BDBG_MSG(("Tx%d:   Pixel Clock Rate:	%s MHz", hHDMI->eCoreId,
		BHDM_P_TmdsClockToText_isrsafe(eTmdsClock))) ;


	if (eAudioSamplingRate < BAVC_AudioSamplingRate_eUnknown)
	{
		BDBG_MSG(("Tx%d:   Audio Sample Rate: %s", hHDMI->eCoreId,
			AudioSampleRateText[eAudioSamplingRate])) ;
	}
	else
	{
		BDBG_MSG(("Tx%d:   Audio Sample Rate: Unknown", hHDMI->eCoreId)) ;
	}


	BDBG_MSG(("Tx%d:   N         CTS 0 - CTS 1", hHDMI->eCoreId)) ;
	BDBG_MSG(("Tx%d: %4d	%6d-%6d", hHDMI->eCoreId,
		stAcrPacket->NValue,
		stAcrPacket->CTS_0, stAcrPacket->CTS_1)) ;
	BDBG_MSG(("Tx%d: CTS 0 Repeat Value: %d Period: %d", hHDMI->eCoreId,
		stAcrPacket->CTS_0, stAcrPacket->CTS_0)) ;
	BDBG_MSG(("Tx%d: CTS 1 Repeat Value: %d Period: %d", hHDMI->eCoreId,
		stAcrPacket->CTS_1, stAcrPacket->CTS_1)) ;
#endif

done :
	return rc ;
}


BERR_Code BHDM_PACKET_ACR_P_TableLookup_isrsafe(
	BHDM_Handle hHDMI,
	BAVC_AudioSamplingRate eAudioSamplingRate,
	uint64_t ulPixelClkRate64BitMask,
	const BHDM_Video_Settings *stVideoSettings,
	BAVC_HDMI_PixelRepetition ePixelRepetition,

	BHDM_P_TmdsClock *eTmdsClock,
	BHDM_P_AUDIO_CLK_VALUES *stAcrPacket
)
{
	BERR_Code rc = BERR_SUCCESS ;

	/* first get the TMDS clock based on the video parameters */
	rc = BHDM_PACKET_ACR_P_LookupTmdsClock_isrsafe(hHDMI,
		ulPixelClkRate64BitMask, stVideoSettings, ePixelRepetition,
		eTmdsClock) ;
	if (rc) {rc = BERR_TRACE(rc) ; goto done ;}

	/*
	-- next get the ACR packet parameters
	-- based on the TMDS Clock and Audio Sample Rate
	*/
	rc = BHDM_PACKET_ACR_P_LookupN_CTSValues_isrsafe(hHDMI,
		eAudioSamplingRate, *eTmdsClock,
		stAcrPacket) ;
	if (rc) {rc = BERR_TRACE(rc) ; goto done ;}

done :
    return rc ;

}

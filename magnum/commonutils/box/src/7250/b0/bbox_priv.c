/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Module Description:
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#include "bstd.h"                /* standard types */
#include "bkni.h"
#include "bdbg.h"                /* Debug message */
#include "bbox.h"
#include "bbox_priv_modes.h"
#include "bbox_vdc.h"

#include "bchp_common.h"
#include "bchp_memc_arb_0.h"

BDBG_MODULE(BBOX_PRIV);
BDBG_OBJECT_ID(BBOX_BOX_PRIV);

/* Available Box Mode RTS */
extern BBOX_Rts stBoxRts_1u_0t_box1;
extern BBOX_Rts stBoxRts_1u_0t_box2;
extern BBOX_Rts stBoxRts_1u_0t_box3;
extern BBOX_Rts stBoxRts_1u_1t_box4;
extern BBOX_Rts stBoxRts_0u_1t_box5;
extern BBOX_Rts stBoxRts_1u_0t_box6;
extern BBOX_Rts stBoxRts_1u_0t_box7;
extern BBOX_Rts stBoxRts_1u_0t_box8;
extern BBOX_Rts stBoxRts_1u_0t_box9;

/* Memc Index for box mode 1. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7250B0_box1 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(0,       0,       0,       Invalid, 0      ),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 6 */
      }
   },
   BBOX_INVALID_NUM_MEMC
};

/* Memc Index for box mode 2. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7250B0_box2 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, 0      ),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, 0      ),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 6 */
      }
   },
   BBOX_INVALID_NUM_MEMC
};

/* Memc Index for box mode 3. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7250B0_box3 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, 0      ),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 6 */
      }
   },
   BBOX_INVALID_NUM_MEMC
};

/* Memc Index for box mode 4. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7250B0_box4 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, 0      ),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, Invalid, Invalid, 0      ),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 6 */
      }
   },
   BBOX_INVALID_NUM_MEMC
};

/* Memc Index for box mode 5. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7250B0_box5 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, Invalid, Invalid, 0      ),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 6 */
      }
   },
   BBOX_INVALID_NUM_MEMC
};

/* Memc Index for box mode 6. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7250B0_box6 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, 0      ),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, Invalid, Invalid, 0      ),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 6 */
      }
   },
   BBOX_INVALID_NUM_MEMC
};

/* Memc Index for box mode 7. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7250B0_box7 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(0,       0,       0,       Invalid, 0      ),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 6 */
      }
   },
   BBOX_INVALID_NUM_MEMC
};

/* Memc Index for box mode 8. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7250B0_box8 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, 0      ),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, Invalid, Invalid, 0      ),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 6 */
      }
   },
   BBOX_INVALID_NUM_MEMC
};

/* Memc Index for box mode 9. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7250B0_box9 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, 0      ),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, Invalid, Invalid, 0      ),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 6 */
      }
   },
   BBOX_INVALID_NUM_MEMC
};

BERR_Code BBOX_P_ValidateId
    (uint32_t                ulId)
{
    BERR_Code eStatus = BERR_SUCCESS;
    if (ulId == 0 || ulId > BBOX_MODES_SUPPORTED)
    {
        BDBG_ERR(("Box Mode ID %d is not supported.", ulId));
        eStatus = BBOX_ID_NOT_SUPPORTED;
    }
    return eStatus;
}

static void BBOX_P_Vdc_SetSourceLimits
    ( uint32_t                       ulBoxId,
      BBOX_Vdc_Source_Capabilities  *pBoxVdcSrcCap )
{
    uint32_t i;

    switch (ulBoxId)
    {
        case 1:
            for (i=0; i < BAVC_SourceId_eMax; i++)
            {
                pBoxVdcSrcCap[i].bAvailable = false;
                pBoxVdcSrcCap[i].bMtgCapable = false;
                pBoxVdcSrcCap[i].stSizeLimits.ulHeight = BBOX_VDC_DISREGARD;
                pBoxVdcSrcCap[i].stSizeLimits.ulWidth = BBOX_VDC_DISREGARD;
                pBoxVdcSrcCap[i].eColorSpace = BBOX_VDC_DISREGARD;
                pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e8bit;
                switch (i)
                {
                    case BAVC_SourceId_eMpeg0:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].bMtgCapable = true;
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e10bit;
                        break;
                    case BAVC_SourceId_eMpeg1:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].bMtgCapable = true;
                        break;
                    case BAVC_SourceId_eGfx0:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 1080;
                        pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 1920;
                        pBoxVdcSrcCap[i].eColorSpace = BBOX_Vdc_Colorspace_eRGB;
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e8bit;
                        break;
                }
            }
            break;

        case 2:
        case 9:
            for (i=0; i < BAVC_SourceId_eMax; i++)
            {
                pBoxVdcSrcCap[i].bAvailable = false;
                pBoxVdcSrcCap[i].bMtgCapable = false;
                pBoxVdcSrcCap[i].stSizeLimits.ulHeight = BBOX_VDC_DISREGARD;
                pBoxVdcSrcCap[i].stSizeLimits.ulWidth = BBOX_VDC_DISREGARD;
                pBoxVdcSrcCap[i].eColorSpace = BBOX_VDC_DISREGARD;
                pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e8bit;
                switch (i)
                {
                    case BAVC_SourceId_eMpeg0:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e10bit;
                        break;
                    case BAVC_SourceId_eGfx0:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 1080;
                        pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 1920;
                        pBoxVdcSrcCap[i].eColorSpace = BBOX_Vdc_Colorspace_eRGB;
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e8bit;
                        break;
                    case BAVC_SourceId_eGfx1:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 576;
                        pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 720;
                        pBoxVdcSrcCap[i].eColorSpace = BBOX_Vdc_Colorspace_eRGB;
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e8bit;
                        break;
                }
            }
        break;

        case 3:
            for (i=0; i < BAVC_SourceId_eMax; i++)
            {
                pBoxVdcSrcCap[i].bAvailable = false;
                pBoxVdcSrcCap[i].bMtgCapable = false;
                pBoxVdcSrcCap[i].stSizeLimits.ulHeight = BBOX_VDC_DISREGARD;
                pBoxVdcSrcCap[i].stSizeLimits.ulWidth = BBOX_VDC_DISREGARD;
                pBoxVdcSrcCap[i].eColorSpace = BBOX_VDC_DISREGARD;
                pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e8bit;
                switch (i)
                {
                    case BAVC_SourceId_eMpeg0:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        break;
                    case BAVC_SourceId_eGfx0:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 1080;
                        pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 1920;
                        pBoxVdcSrcCap[i].eColorSpace = BBOX_Vdc_Colorspace_eRGB;
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e8bit;
                        break;
                }
            }
            break;

        case 4:
            for (i=0; i < BAVC_SourceId_eMax; i++)
            {
                pBoxVdcSrcCap[i].bAvailable = false;
                pBoxVdcSrcCap[i].bMtgCapable = false;
                pBoxVdcSrcCap[i].stSizeLimits.ulHeight = BBOX_VDC_DISREGARD;
                pBoxVdcSrcCap[i].stSizeLimits.ulWidth = BBOX_VDC_DISREGARD;
                pBoxVdcSrcCap[i].eColorSpace = BBOX_VDC_DISREGARD;
                pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e8bit;
                switch (i)
                {
                    case BAVC_SourceId_eMpeg0:
                    case BAVC_SourceId_eMpeg1:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        break;
                    case BAVC_SourceId_eGfx0:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 1080;
                        pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 1920;
                        pBoxVdcSrcCap[i].eColorSpace = BBOX_Vdc_Colorspace_eRGB;
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e8bit;
                        break;
                    case BAVC_SourceId_eGfx1:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 576;
                        pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 720;
                        pBoxVdcSrcCap[i].eColorSpace = BBOX_Vdc_Colorspace_eRGB;
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e8bit;
                        break;
                }
            }
            break;

        case 5:
            for (i=0; i < BAVC_SourceId_eMax; i++)
            {
                pBoxVdcSrcCap[i].bAvailable = false;
                pBoxVdcSrcCap[i].bMtgCapable = false;
                pBoxVdcSrcCap[i].stSizeLimits.ulHeight = BBOX_VDC_DISREGARD;
                pBoxVdcSrcCap[i].stSizeLimits.ulWidth = BBOX_VDC_DISREGARD;
                pBoxVdcSrcCap[i].eColorSpace = BBOX_VDC_DISREGARD;
                pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e8bit;
                switch (i)
                {
                    case BAVC_SourceId_eMpeg1:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e10bit;
                        break;
                    case BAVC_SourceId_eGfx1:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 576;
                        pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 720;
                        pBoxVdcSrcCap[i].eColorSpace = BBOX_Vdc_Colorspace_eRGB;
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e8bit;
                        break;
                }
            }
            break;

        case 6:
            for (i=0; i < BAVC_SourceId_eMax; i++)
            {
                pBoxVdcSrcCap[i].bAvailable = false;
                pBoxVdcSrcCap[i].bMtgCapable = false;
                pBoxVdcSrcCap[i].stSizeLimits.ulHeight = BBOX_VDC_DISREGARD;
                pBoxVdcSrcCap[i].stSizeLimits.ulWidth = BBOX_VDC_DISREGARD;
                pBoxVdcSrcCap[i].eColorSpace = BBOX_VDC_DISREGARD;
                pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e8bit;
                switch (i)
                {
                    case BAVC_SourceId_eMpeg0:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e10bit;
                        break;
                    case BAVC_SourceId_eHdDvi0:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 1080;
                        pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 1920;
                        break;
                    case BAVC_SourceId_eGfx0:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 1080;
                        pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 1920;
                        pBoxVdcSrcCap[i].eColorSpace = BBOX_Vdc_Colorspace_eRGB;
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e8bit;
                        break;
                    case BAVC_SourceId_eGfx1:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 576;
                        pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 720;
                        pBoxVdcSrcCap[i].eColorSpace = BBOX_Vdc_Colorspace_eRGB;
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e8bit;
                        break;
                }
            }
            break;

        case 7:
            for (i=0; i < BAVC_SourceId_eMax; i++)
            {
                pBoxVdcSrcCap[i].bAvailable = false;
                pBoxVdcSrcCap[i].bMtgCapable = false;
                pBoxVdcSrcCap[i].stSizeLimits.ulHeight = BBOX_VDC_DISREGARD;
                pBoxVdcSrcCap[i].stSizeLimits.ulWidth = BBOX_VDC_DISREGARD;
                pBoxVdcSrcCap[i].eColorSpace = BBOX_VDC_DISREGARD;
                pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e8bit;
                switch (i)
                {
                    case BAVC_SourceId_eMpeg0:
                    case BAVC_SourceId_eMpeg1:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e8bit;
                        break;
                    case BAVC_SourceId_eGfx0:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 1080;
                        pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 1920;
                        pBoxVdcSrcCap[i].eColorSpace = BBOX_Vdc_Colorspace_eRGB;
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e8bit;
                        break;
                }
            }
            break;

        case 8:
            for (i=0; i < BAVC_SourceId_eMax; i++)
            {
                pBoxVdcSrcCap[i].bAvailable = false;
                pBoxVdcSrcCap[i].bMtgCapable = false;
                pBoxVdcSrcCap[i].stSizeLimits.ulHeight = BBOX_VDC_DISREGARD;
                pBoxVdcSrcCap[i].stSizeLimits.ulWidth = BBOX_VDC_DISREGARD;
                pBoxVdcSrcCap[i].eColorSpace = BBOX_VDC_DISREGARD;
                pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e8bit;
                switch (i)
                {
                    case BAVC_SourceId_eMpeg0:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e8bit;
                        break;
                    case BAVC_SourceId_eGfx0:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 1080;
                        pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 1920;
                        pBoxVdcSrcCap[i].eColorSpace = BBOX_Vdc_Colorspace_eRGB;
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e8bit;
                        break;
                    case BAVC_SourceId_eGfx1:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 576;
                        pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 720;
                        pBoxVdcSrcCap[i].eColorSpace = BBOX_Vdc_Colorspace_eRGB;
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e8bit;
                        break;
                }
            }
            break;

    }
}

static void BBOX_P_Vdc_SetDisplayLimits
    ( uint32_t                       ulBoxId,
      BBOX_Vdc_Display_Capabilities *pBoxVdcDispCap )
{
    uint32_t i, j;

    switch (ulBoxId)
    {
    case 1:
        for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
        {
            /* CMP-STG-Encoder mapping */
            pBoxVdcDispCap[i].bAvailable = false;
            pBoxVdcDispCap[i].eMaxVideoFmt = BBOX_VDC_DISREGARD;
            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BBOX_VDC_DISREGARD;
            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_VDC_DISREGARD;
            pBoxVdcDispCap[i].stStgEnc.bAvailable = false;
            pBoxVdcDispCap[i].stStgEnc.ulStgId = BBOX_FTR_INVALID;
            pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = BBOX_FTR_INVALID;
            pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = BBOX_FTR_INVALID;

            for (j=0; j < BBOX_VDC_WINDOW_COUNT_PER_DISPLAY; j++)
            {
                pBoxVdcDispCap[i].astWindow[j].bAvailable = false;
                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_VDC_DISREGARD;

                switch (i)
                {
                    case 0:
                        pBoxVdcDispCap[i].bAvailable = true;
                        pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e1080p;
                        pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                        pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass1;

                        if (j==0)
                        {
                            pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            /* Smooth scaling */
                            pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            /* MADR is available for C0V0 */
                            pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_FTR_HD;
                        }
                        /* PIP size */
                        if (j==1)
                        {
                            pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            /* Smooth scaling */
                            pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = 2;
                            pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = 2;
                        }
                        /* GFX window is available */
                        if (j==2)
                        {
                            pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                        }
                        break;
                    default:
                        break;
                }
            }
        }
        break;

    case 2:
    case 9:
        for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
        {
            /* CMP-STG-Encoder mapping */
            pBoxVdcDispCap[i].bAvailable = false;
            pBoxVdcDispCap[i].eMaxVideoFmt = BBOX_VDC_DISREGARD;
            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_VDC_DISREGARD;
            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BBOX_VDC_DISREGARD;
            pBoxVdcDispCap[i].stStgEnc.bAvailable = false;
            pBoxVdcDispCap[i].stStgEnc.ulStgId = BBOX_FTR_INVALID;
            pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = BBOX_FTR_INVALID;
            pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = BBOX_FTR_INVALID;

            for (j=0; j < BBOX_VDC_WINDOW_COUNT_PER_DISPLAY; j++)
            {
                pBoxVdcDispCap[i].astWindow[j].bAvailable = false;
                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_VDC_DISREGARD;

                switch (i)
                {
                    case 0:
                        pBoxVdcDispCap[i].bAvailable = true;
                        pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e1080p;
                        pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                        pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass1;
                        if (j==0)
                        {
                            pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            /* MADR is available for C0V0 */
                            pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_FTR_HD;
                        }
                        /* GFX window is available */
                        if (j==2)
                        {
                            pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                        }
                        break;
                    case 1:
                        pBoxVdcDispCap[i].bAvailable = true;
                        pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_ePAL_G;
                        pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_ePAL_G;
                        pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass0;

                        if (j==0)
                        {
                            pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                        }
                        /* GFX window is available */
                        if (j==2)
                        {
                            pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                        }
                        break;
                    default:
                        break;
                }
            }
        }
        break;

    case 3:
        for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
        {
            /* CMP-STG-Encoder mapping */
            pBoxVdcDispCap[i].bAvailable = false;
            pBoxVdcDispCap[i].eMaxVideoFmt = BBOX_VDC_DISREGARD;
            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BBOX_VDC_DISREGARD;
            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_VDC_DISREGARD;
            pBoxVdcDispCap[i].stStgEnc.bAvailable = false;
            pBoxVdcDispCap[i].stStgEnc.ulStgId = BBOX_FTR_INVALID;
            pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = BBOX_FTR_INVALID;
            pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = BBOX_FTR_INVALID;

            for (j=0; j < BBOX_VDC_WINDOW_COUNT_PER_DISPLAY; j++)
            {
                pBoxVdcDispCap[i].astWindow[j].bAvailable = false;
                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_VDC_DISREGARD;

                switch (i)
                {
                    case 0:
                        pBoxVdcDispCap[i].bAvailable = true;
                        pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e1080p;
                        pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                        pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass1;

                        if (j==0)
                        {
                            pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            /* MADR is available for C0V0 */
                            pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_FTR_HD;
                        }
                        /* GFX window is available */
                        if (j==2)
                        {
                            pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                        }
                        break;
                    default:
                        break;
                }
            }
        }
        break;

    case 4:
        for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
        {
            /* CMP-STG-Encoder mapping */
            pBoxVdcDispCap[i].bAvailable = false;
            pBoxVdcDispCap[i].eMaxVideoFmt = BBOX_VDC_DISREGARD;
            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BBOX_VDC_DISREGARD;
            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_VDC_DISREGARD;
            pBoxVdcDispCap[i].stStgEnc.bAvailable = false;
            pBoxVdcDispCap[i].stStgEnc.ulStgId = BBOX_FTR_INVALID;
            pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = BBOX_FTR_INVALID;
            pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = BBOX_FTR_INVALID;

            for (j=0; j < BBOX_VDC_WINDOW_COUNT_PER_DISPLAY; j++)
            {
                pBoxVdcDispCap[i].astWindow[j].bAvailable = false;
                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_VDC_DISREGARD;

                switch (i)
                {
                    case 0:
                        pBoxVdcDispCap[i].bAvailable = true;
                        pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e1080p;
                        pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                        pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass1;

                        if (j==0)
                        {
                            pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            /* MADR is available for C0V0 */
                            pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_FTR_HD;
                        }
                        /* GFX window is available */
                        if (j==2)
                        {
                            pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                        }
                        break;
                    case 1:
                        pBoxVdcDispCap[i].bAvailable = true;
                        pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e576p_50Hz;
                        pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e576p_50Hz;
                        pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
                        pBoxVdcDispCap[i].stStgEnc.ulStgId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = BBOX_VDC_DISREGARD; /* Raaga encoder */
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = BBOX_VDC_DISREGARD; /* Raaga encoder */
                        pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eAutoDisable;
                        pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass0;

                        if (j==0)
                        {
                            pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                        }
                        /* GFX window is available */
                        if (j==2)
                        {
                            pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                        }
                        break;
                    default:
                        break;
                }
            }
        }
        break;

    case 5:
        for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
        {
            /* CMP-STG-Encoder mapping */
            pBoxVdcDispCap[i].bAvailable = false;
            pBoxVdcDispCap[i].eMaxVideoFmt = BBOX_VDC_DISREGARD;
            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BBOX_VDC_DISREGARD;
            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_VDC_DISREGARD;
            pBoxVdcDispCap[i].stStgEnc.bAvailable = false;
            pBoxVdcDispCap[i].stStgEnc.ulStgId = BBOX_FTR_INVALID;
            pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = BBOX_FTR_INVALID;
            pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = BBOX_FTR_INVALID;

            for (j=0; j < BBOX_VDC_WINDOW_COUNT_PER_DISPLAY; j++)
            {
                pBoxVdcDispCap[i].astWindow[j].bAvailable = false;
                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_VDC_DISREGARD;

                switch (i)
                {
                    case 1:
                        pBoxVdcDispCap[i].bAvailable = true;
                        pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e720p_30Hz;
                        pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e720p_30Hz;
                        pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
                        pBoxVdcDispCap[i].stStgEnc.ulStgId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = BBOX_VDC_DISREGARD; /* Raaga encoder */
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = BBOX_VDC_DISREGARD; /* Raaga encoder */
                        pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eAutoDisable;

                        if (j==0)
                        {
                            pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                        }
                        /* GFX window is available */
                        if (j==2)
                        {
                            pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                        }
                        break;
                    default:
                        break;
                }
            }
        }
        break;

    case 6:
        for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
        {
            /* CMP-STG-Encoder mapping */
            pBoxVdcDispCap[i].bAvailable = false;
            pBoxVdcDispCap[i].eMaxVideoFmt = BBOX_VDC_DISREGARD;
            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BBOX_VDC_DISREGARD;
            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_VDC_DISREGARD;
            pBoxVdcDispCap[i].stStgEnc.bAvailable = false;
            pBoxVdcDispCap[i].stStgEnc.ulStgId = BBOX_FTR_INVALID;
            pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = BBOX_FTR_INVALID;
            pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = BBOX_FTR_INVALID;

            for (j=0; j < BBOX_VDC_WINDOW_COUNT_PER_DISPLAY; j++)
            {
                pBoxVdcDispCap[i].astWindow[j].bAvailable = false;
                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_VDC_DISREGARD;

                switch (i)
                {
                    case 0:
                        pBoxVdcDispCap[i].bAvailable = true;
                        pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e1080p;
                        pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                        pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass1;

                        if (j==0)
                        {
                            pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            /* MADR is available for C0V0 */
                            pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_FTR_HD;
                            /* Smooth scaling */
                            pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                        }
                        /* GFX window is available */
                        if (j==2)
                        {
                            pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                        }
                        break;

                    case 1:
                        pBoxVdcDispCap[i].bAvailable = true;
                        pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_ePAL_G;
                        pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_ePAL_G;
                        pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass0;

                        if (j==0)
                        {
                            pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            /* Smooth scaling */
                            pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                        }
                        /* GFX window is available */
                        if (j==2)
                        {
                            pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                        }
                        break;
                    default:
                        break;
                }
            }
        }
        break;

    case 7:
        for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
        {
            /* CMP-STG-Encoder mapping */
            pBoxVdcDispCap[i].bAvailable = false;
            pBoxVdcDispCap[i].eMaxVideoFmt = BBOX_VDC_DISREGARD;
            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BBOX_VDC_DISREGARD;
            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_VDC_DISREGARD;
            pBoxVdcDispCap[i].stStgEnc.bAvailable = false;
            pBoxVdcDispCap[i].stStgEnc.ulStgId = BBOX_FTR_INVALID;
            pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = BBOX_FTR_INVALID;
            pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = BBOX_FTR_INVALID;

            for (j=0; j < BBOX_VDC_WINDOW_COUNT_PER_DISPLAY; j++)
            {
                pBoxVdcDispCap[i].astWindow[j].bAvailable = false;
                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_VDC_DISREGARD;

                switch (i)
                {
                    case 0:
                        pBoxVdcDispCap[i].bAvailable = true;
                        pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e1080p;
                        pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                        pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass1;

                        /* main */
                        if (j==0)
                        {
                            pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            /* MADR is available for C0V0 */
                            pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_FTR_HD;
                        }
                        /* pip */
                        if (j==1)
                        {
                            pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = 2;
                            pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = 2;
                        }
                        /* GFX window is available */
                        if (j==2)
                        {
                            pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                        }
                        break;

                    default:
                        break;
                }
            }
        }
        break;

    case 8:
        for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
        {
            /* CMP-STG-Encoder mapping */
            pBoxVdcDispCap[i].bAvailable = false;
            pBoxVdcDispCap[i].eMaxVideoFmt = BBOX_VDC_DISREGARD;
            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BBOX_VDC_DISREGARD;
            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_VDC_DISREGARD;
            pBoxVdcDispCap[i].stStgEnc.bAvailable = false;
            pBoxVdcDispCap[i].stStgEnc.ulStgId = BBOX_FTR_INVALID;
            pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = BBOX_FTR_INVALID;
            pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = BBOX_FTR_INVALID;

            for (j=0; j < BBOX_VDC_WINDOW_COUNT_PER_DISPLAY; j++)
            {
                pBoxVdcDispCap[i].astWindow[j].bAvailable = false;
                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_VDC_DISREGARD;

                switch (i)
                {
                    case 0:
                        pBoxVdcDispCap[i].bAvailable = true;
                        pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e1080p;
                        pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                        pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass1;

                        /* main */
                        if (j==0)
                        {
                            pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            /* MADR is available for C0V0 */
                            pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_FTR_HD;
                            pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = 1;
                            pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = 1;
                        }
                        /* GFX window is available */
                        if (j==2)
                        {
                            pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                        }
                        break;

                    case 1:
                        pBoxVdcDispCap[i].bAvailable = true;
                        pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_ePAL_G;
                        pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_ePAL_G;
                        pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass0;

                        /* main */
                        if (j==0)
                        {
                            pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = 1;
                            pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = 1;
                        }
                        /* GFX window is available */
                        if (j==2)
                        {
                            pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                        }
                        break;

                    default:
                        break;
                }
            }
        }
        break;

    }

}

static void BBOX_P_Vdc_SetDeinterlacerLimits
    ( uint32_t                            ulBoxId,
      BBOX_Vdc_Deinterlacer_Capabilities *pDeinterlacerCap )
{
    uint32_t i;

    switch (ulBoxId)
    {
    case 1:
        for (i=0; i < BBOX_VDC_DEINTERLACER_COUNT; i++)
        {
            pDeinterlacerCap[i].stPictureLimits.ulWidth = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].stPictureLimits.ulHeight = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].ulHsclThreshold = BBOX_VDC_DISREGARD;
            switch (i)
            {
                case 0:
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1920;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    break;
            }
        }
        break;

    case 2:
    case 9:
        for (i=0; i < BBOX_VDC_DEINTERLACER_COUNT; i++)
        {
            pDeinterlacerCap[i].stPictureLimits.ulWidth = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].stPictureLimits.ulHeight = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].ulHsclThreshold = BBOX_VDC_DISREGARD;
            switch (i)
            {
                case 0:
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1920;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    break;
            }
        }
        break;

    case 3:
        for (i=0; i < BBOX_VDC_DEINTERLACER_COUNT; i++)
        {
            pDeinterlacerCap[i].stPictureLimits.ulWidth = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].stPictureLimits.ulHeight = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].ulHsclThreshold = BBOX_VDC_DISREGARD;
            switch (i)
            {
                case 0:
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1920;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    break;
            }
        }
        break;

    case 4:
        for (i=0; i < BBOX_VDC_DEINTERLACER_COUNT; i++)
        {
            pDeinterlacerCap[i].stPictureLimits.ulWidth = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].stPictureLimits.ulHeight = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].ulHsclThreshold = BBOX_VDC_DISREGARD;
            switch (i)
            {
                case 0:
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1920;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    break;
            }
        }
        break;

    case 5:
        for (i=0; i < BBOX_VDC_DEINTERLACER_COUNT; i++)
        {
            pDeinterlacerCap[i].stPictureLimits.ulWidth = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].stPictureLimits.ulHeight = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].ulHsclThreshold = BBOX_VDC_DISREGARD;
        }
        break;

    case 6:
        for (i=0; i < BBOX_VDC_DEINTERLACER_COUNT; i++)
        {
            pDeinterlacerCap[i].stPictureLimits.ulWidth = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].stPictureLimits.ulHeight = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].ulHsclThreshold = BBOX_VDC_DISREGARD;
            switch (i)
            {
                case 0:
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1920;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    break;
            }
        }
        break;

    case 7:
        for (i=0; i < BBOX_VDC_DEINTERLACER_COUNT; i++)
        {
            pDeinterlacerCap[i].stPictureLimits.ulWidth = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].stPictureLimits.ulHeight = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].ulHsclThreshold = BBOX_VDC_DISREGARD;
            switch (i)
            {
                case 0:
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1920;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    break;
            }
        }
        break;

    case 8:
        for (i=0; i < BBOX_VDC_DEINTERLACER_COUNT; i++)
        {
            pDeinterlacerCap[i].stPictureLimits.ulWidth = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].stPictureLimits.ulHeight = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].ulHsclThreshold = BBOX_VDC_DISREGARD;
            switch (i)
            {
                case 0:
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1920;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    pDeinterlacerCap[i].ulHsclThreshold = 960;
                    break;
            }
        }
        break;
    }
}

static void BBOX_P_Vdc_SetXcodeLimits
    ( uint32_t                     ulBoxId,
      BBOX_Vdc_Xcode_Capabilities *pXcodeCap )
{
    switch (ulBoxId)
    {
    case 1:
        pXcodeCap->ulNumXcodeCapVfd = BBOX_VDC_DISREGARD;
        pXcodeCap->ulNumXcodeGfd = BBOX_VDC_DISREGARD;
        break;

    case 2:
    case 9:
        pXcodeCap->ulNumXcodeCapVfd = BBOX_VDC_DISREGARD;
        pXcodeCap->ulNumXcodeGfd = BBOX_VDC_DISREGARD;
        break;

    case 3:
        pXcodeCap->ulNumXcodeCapVfd = BBOX_VDC_DISREGARD;
        pXcodeCap->ulNumXcodeGfd = BBOX_VDC_DISREGARD;
        break;

    case 4:
        pXcodeCap->ulNumXcodeCapVfd = BBOX_VDC_DISREGARD;
        pXcodeCap->ulNumXcodeGfd = 1;
        break;

    case 5:
        pXcodeCap->ulNumXcodeCapVfd = BBOX_VDC_DISREGARD;
        pXcodeCap->ulNumXcodeGfd = 1;
        break;

    case 6:
        pXcodeCap->ulNumXcodeCapVfd = BBOX_VDC_DISREGARD;
        pXcodeCap->ulNumXcodeGfd = BBOX_VDC_DISREGARD;
        break;

    case 7:
        pXcodeCap->ulNumXcodeCapVfd = BBOX_VDC_DISREGARD;
        pXcodeCap->ulNumXcodeGfd = BBOX_VDC_DISREGARD;
        break;

    case 8:
        pXcodeCap->ulNumXcodeCapVfd = BBOX_VDC_DISREGARD;
        pXcodeCap->ulNumXcodeGfd = BBOX_VDC_DISREGARD;
        break;
    }

}

BERR_Code BBOX_P_Vdc_SetBoxMode
    ( uint32_t               ulBoxId,
      BBOX_Vdc_Capabilities *pBoxVdc )
{
    BERR_Code eStatus = BERR_SUCCESS;

    BDBG_ASSERT(pBoxVdc);

    if (ulBoxId == 0 || ulBoxId > BBOX_MODES_SUPPORTED)
    {
        BKNI_Memset((void*)pBoxVdc, 0x0, sizeof(BBOX_Vdc_Capabilities));
        BDBG_ERR(("Box mode %d is not supported on this chip.", ulBoxId));
        eStatus = BERR_INVALID_PARAMETER;
    }
    else
    {
        BBOX_P_Vdc_SetDisplayLimits(ulBoxId, pBoxVdc->astDisplay);
        BBOX_P_Vdc_SetSourceLimits(ulBoxId, pBoxVdc->astSource);
        BBOX_P_Vdc_SetDeinterlacerLimits(ulBoxId, pBoxVdc->astDeinterlacer);
        BBOX_P_Vdc_SetXcodeLimits(ulBoxId, &pBoxVdc->stXcode);
    }

    return eStatus;
}

BERR_Code BBOX_P_GetMemConfig
    ( uint32_t                       ulBoxId,
      BBOX_MemConfig                *pBoxMemConfig )
{
    if (ulBoxId == 0 || ulBoxId > BBOX_MODES_SUPPORTED)
    {
        BDBG_ERR(("Box mode %d is not supported on this chip.", ulBoxId));
        return (BERR_INVALID_PARAMETER);
    }

    switch (ulBoxId)
    {
        case 1:
            *pBoxMemConfig = stBoxMemConfig_7250B0_box1;
            pBoxMemConfig->ulNumMemc = stBoxRts_1u_0t_box1.ulNumMemc;
            break;
        case 2:
            *pBoxMemConfig = stBoxMemConfig_7250B0_box2;
            pBoxMemConfig->ulNumMemc = stBoxRts_1u_0t_box2.ulNumMemc;
            break;
        case 3:
            *pBoxMemConfig = stBoxMemConfig_7250B0_box3;
            pBoxMemConfig->ulNumMemc = stBoxRts_1u_0t_box3.ulNumMemc;
            break;
        case 4:
            *pBoxMemConfig = stBoxMemConfig_7250B0_box4;
            pBoxMemConfig->ulNumMemc = stBoxRts_1u_1t_box4.ulNumMemc;
            break;
        case 5:
            *pBoxMemConfig = stBoxMemConfig_7250B0_box5;
            pBoxMemConfig->ulNumMemc = stBoxRts_0u_1t_box5.ulNumMemc;
            break;
        case 6:
            *pBoxMemConfig = stBoxMemConfig_7250B0_box6;
            pBoxMemConfig->ulNumMemc = stBoxRts_1u_0t_box6.ulNumMemc;
            break;
        case 7:
            *pBoxMemConfig = stBoxMemConfig_7250B0_box7;
            pBoxMemConfig->ulNumMemc = stBoxRts_1u_0t_box7.ulNumMemc;
            break;
        case 8:
            *pBoxMemConfig = stBoxMemConfig_7250B0_box8;
            pBoxMemConfig->ulNumMemc = stBoxRts_1u_0t_box8.ulNumMemc;
            break;
        case 9:
            *pBoxMemConfig = stBoxMemConfig_7250B0_box9;
            pBoxMemConfig->ulNumMemc = stBoxRts_1u_0t_box9.ulNumMemc;
            break;

    }

    return BERR_SUCCESS;
}

BERR_Code BBOX_P_LoadRts
    ( const BREG_Handle      hReg,
      const uint32_t         ulBoxId )
{
    BERR_Code eStatus = BERR_SUCCESS;

    if (ulBoxId == 0 || ulBoxId > BBOX_MODES_SUPPORTED)
    {
        BDBG_ERR(("Box mode %d is not supported on this chip.", ulBoxId));
        eStatus = BERR_INVALID_PARAMETER;
    }
    else
    {
        uint32_t i, j;
        BBOX_Rts *pBoxRts = NULL;

        switch (ulBoxId)
        {
            case 1:
                pBoxRts = &stBoxRts_1u_0t_box1;
                break;
            case 2:
                pBoxRts = &stBoxRts_1u_0t_box2;
                break;
            case 3:
                pBoxRts = &stBoxRts_1u_0t_box3;
                break;
            case 4:
                pBoxRts = &stBoxRts_1u_1t_box4;
                break;
            case 5:
                pBoxRts = &stBoxRts_0u_1t_box5;
                break;
            case 6:
                pBoxRts = &stBoxRts_1u_0t_box6;
                break;
            case 7:
                pBoxRts = &stBoxRts_1u_0t_box7;
                break;
            case 8:
                pBoxRts = &stBoxRts_1u_0t_box8;
                break;
            case 9:
                pBoxRts = &stBoxRts_1u_0t_box9;
                break;
        }

        /* verify box ID */
        if (pBoxRts->ulBoxId != ulBoxId)
        {
            BDBG_ERR(("Mismatched box ID between device tree/env var and RTS file."));
            eStatus = BBOX_ID_AND_RTS_MISMATCH;
            goto done;
        }

        for (i=0;i<pBoxRts->ulNumMemc;i++)
        {
            uint32_t ulMemcBaseAddr = 0x0;
            BDBG_ASSERT(pBoxRts->paulMemc[i][0]);

            if (i==0)
            {
#ifdef BCHP_MEMC_ARB_0_REG_START
                ulMemcBaseAddr = BCHP_MEMC_ARB_0_CLIENT_INFO_0;
#else
                BDBG_ERR(("There is no MEMC0. Verify the number of MEMC defined in RTS file."));
                eStatus = BBOX_INCORRECT_MEMC_COUNT;
                goto done;
#endif
            }

            BDBG_ASSERT(ulMemcBaseAddr);

            for (j=0;j<pBoxRts->ulNumMemcEntries;j++)
            {
                BREG_Write32(hReg, ulMemcBaseAddr+(j*4), pBoxRts->paulMemc[i][j]);
                BDBG_MSG(("memc[%d][%d] = 0x%x", i, j, pBoxRts->paulMemc[i][j]));
             }

        }

        for (i=0;i<pBoxRts->ulNumPfriClients;i++)
        {
            BREG_Write32(hReg, pBoxRts->pastPfriClient[i].ulAddr, pBoxRts->pastPfriClient[i].ulData);
            BDBG_MSG(("PFRI[%d] = 0x%x : 0x%x\n", i, pBoxRts->pastPfriClient[i].ulAddr, pBoxRts->pastPfriClient[i].ulData));
        }
    }

done:
    return eStatus;
}
/* end of file */

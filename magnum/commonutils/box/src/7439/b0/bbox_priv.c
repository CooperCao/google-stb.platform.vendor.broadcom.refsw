/******************************************************************************
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
******************************************************************************/

#include "bstd.h"                /* standard types */
#include "bkni.h"
#include "bdbg.h"                /* Debug message */
#include "bbox.h"
#include "bbox_priv_modes.h"
#include "bbox_vdc.h"
#include "bchp_memc_arb_0.h"
#include "bchp_memc_arb_1.h"
#include "bchp_common.h"


BDBG_MODULE(BBOX_PRIV);
BDBG_OBJECT_ID(BBOX_BOX_PRIV);

extern BBOX_Rts stBoxRts_7251S_4K_dongle_box1;
extern BBOX_Rts stBoxRts_7252S_4K2t_box2;
extern BBOX_Rts stBoxRts_7252S_4K_gfxpip_box3;
extern BBOX_Rts stBoxRts_7252S_4K2t_large_xcode_box4;
extern BBOX_Rts stBoxRts_7439_4Kstb1t_box5;
extern BBOX_Rts stBoxRts_7439_4Kstb_box6;
extern BBOX_Rts stBoxRts_7439_4Kstb2t50_box7;
extern BBOX_Rts stBoxRts_7252S_4K2t_large_xcode_HiTemp_box9;
extern BBOX_Rts stBoxRts_7439_4Kstb_hitemp_box10;
extern BBOX_Rts stBoxRts_7251_4kstb50_box12;
extern BBOX_Rts stBoxRts_7251_4k50_box13;
extern BBOX_Rts stBoxRts_7252_4Kstb_933_box14;
extern BBOX_Rts stBoxRts_7439_4K1t_933_box16;
extern BBOX_Rts stBoxRts_7251_stb1t_box17;
extern BBOX_Rts stBoxRts_7252_headless_dualxcode_box18;
extern BBOX_Rts stBoxRts_7251_4kp251t_box19;
extern BBOX_Rts stBoxRts_7251_4kstb50_box20;
extern BBOX_Rts stBoxRts_7439_4Kstb1t_box21;
extern BBOX_Rts stBoxRts_7251_headless_dualxcode_box22;
extern BBOX_Rts stBoxRts_7251S_1u1t4k_box23;
extern BBOX_Rts stBoxRts_7252s_headless_dualxcode_pip_box24;
extern BBOX_Rts stBoxRts_7439_1stb2t_box25;

/* Memc Index for box mode 1. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7439B0_box1 =
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

/* Memc Index for box mode 2. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7439B0_box2 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(1,       1,       1,       1,       1      ),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(1,       0,       Invalid, Invalid, 1      ),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, 1      ),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, 1      ),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 6 */
      }
   },
   BBOX_INVALID_NUM_MEMC
};

/* Memc Index for box mode 3. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7439B0_box3 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(1,       0,       1,       0,       1      ),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(0,       0,       Invalid, Invalid, 0      ),  /* disp 1 */
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
static const BBOX_MemConfig stBoxMemConfig_7439B0_box4 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(1,       Invalid, 1      , Invalid, 1      ),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(1,       Invalid, Invalid, Invalid, 1      ),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(1      , Invalid, 1      , Invalid, Invalid),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(1      , Invalid, 1      , Invalid, Invalid),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 6 */
      }
   },
   BBOX_INVALID_NUM_MEMC
};

/* Memc Index for box mode 5. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7439B0_box5 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(1,       1,       1,       1,       1      ),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(1,       0,       Invalid, Invalid, 1      ),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, 1      ),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 6 */
      }
   },
   BBOX_INVALID_NUM_MEMC
};

/* Memc Index for box mode 6. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7439B0_box6 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(0,       0,       0,       0,       0      ),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(1,       1,       Invalid, Invalid, 1      ),  /* disp 1 */
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
static const BBOX_MemConfig stBoxMemConfig_7439B0_box7 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(1,       1,       1,       1,       1      ),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(1,       0,       Invalid, Invalid, 1      ),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, Invalid),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, Invalid),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 6 */
      }
   },
   BBOX_INVALID_NUM_MEMC
};

/* Memc Index for box mode 8. BBOX_MemcIndex_Invalid means it's not used */

/* Memc Index for box mode 9. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7439B0_box9 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(1,       Invalid, 1,       Invalid, 1      ),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(1,       Invalid, Invalid, Invalid, 1      ),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(1      , Invalid, 1      , Invalid, Invalid),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(1      , Invalid, 1      , Invalid, Invalid),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 6 */
      }
   },
   BBOX_INVALID_NUM_MEMC
};


/* Memc Index for box mode 10. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7439B0_box10 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(1,       0,       1,       0,       1      ),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(1,       1,       Invalid, Invalid, 1      ),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 6 */
      }
   },
   BBOX_INVALID_NUM_MEMC
};

/* Memc Index for box mode 12. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7439B0_box12 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(0,       0,       0,       0,       0      ),  /* disp 0 */
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

/* Memc Index for box mode 13. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7439B0_box13 =
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

/* Memc Index for box mode 14. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7439B0_box14 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(0,       1,       0,       1,       1      ),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(1,       1,       Invalid, Invalid, 1      ),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 6 */
      }
   },
   BBOX_INVALID_NUM_MEMC
};

/* Memc Index for box mode 16. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7439B0_box16 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(1,       Invalid, 1,       Invalid, 1      ),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(1,       Invalid, Invalid, Invalid, 1      ),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, Invalid),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 6 */
      }
   },
   BBOX_INVALID_NUM_MEMC
};

/* Memc Index for box mode 17. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7439B0_box17 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(0,       0,       0,       0,       0      ),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(0,       0,       Invalid, Invalid, 0      ),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, Invalid),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 6 */
      }
   },
   BBOX_INVALID_NUM_MEMC
};

/* Memc Index for box mode 18. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7439B0_box18 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(1,       Invalid, 0,       Invalid,       1),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(1,       Invalid, 0,       Invalid,       1),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 6 */
      }
   },
   BBOX_INVALID_NUM_MEMC
};

/* Memc Index for box mode 19. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7439B0_box19 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, 0      ),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, Invalid, Invalid, 0      ),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, Invalid),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 6 */
      }
   },
   BBOX_INVALID_NUM_MEMC
};

/* Memc Index for box mode 20. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7439B0_box20 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(0,       0,       0,       0,       0      ),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(0,       0,       Invalid, Invalid, 0      ),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 6 */
      }
   },
   BBOX_INVALID_NUM_MEMC
};

/* Memc Index for box mode 21. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7439B0_box21 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(1,       1,       1,       0,       0      ),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(1,       0,       Invalid, Invalid, 0      ),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, Invalid),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 6 */
      }
   },
   BBOX_INVALID_NUM_MEMC
};

/* Memc Index for box mode 22. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7439B0_box22 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, Invalid),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, Invalid),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 6 */
      }
   },
   BBOX_INVALID_NUM_MEMC
};

/* Memc Index for box mode 23. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7439B0_box23 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, 0),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, Invalid, Invalid, 0),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, Invalid),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 6 */
      }
   },
   BBOX_INVALID_NUM_MEMC
};

/* Memc Index for box mode 24. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7439B0_box24 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(1,       Invalid, 0,       Invalid, 1      ),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(1,       Invalid, 0,       Invalid, 1      ),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 6 */
      }
   },
   BBOX_INVALID_NUM_MEMC
};

/* Memc Index for box mode 25. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7439B0_box25 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      {
         BBOX_MK_WIN_MEMC_IDX(1,       0,       1,       0,       1      ),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(1,       1,       Invalid, Invalid, 1      ),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(1,       Invalid, 0,       Invalid, Invalid),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(1,       Invalid, 0,       Invalid, Invalid),  /* disp 3 */
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
    if (ulId == 0 || ulId == 8 || ulId == 11 ||
        ulId == 15 || ulId > BBOX_MODES_SUPPORTED)
    {
        BDBG_ERR(("Box Mode %d is not supported.", ulId));
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
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e10bit;
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

        case 2:
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
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e10bit;
                        pBoxVdcSrcCap[i].bAvailable = true;
                        break;
                    case BAVC_SourceId_eMpeg1:
                    case BAVC_SourceId_eMpeg2:
                    case BAVC_SourceId_eMpeg3:
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
                    case BAVC_SourceId_eGfx2:
                    case BAVC_SourceId_eGfx3:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 720;
                        pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 1280;
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
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e10bit;
                        pBoxVdcSrcCap[i].bAvailable = true;
                        break;
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
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e10bit;
                        pBoxVdcSrcCap[i].bAvailable = true;
                        break;
                    case BAVC_SourceId_eMpeg2:
                    case BAVC_SourceId_eMpeg3:
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
                    case BAVC_SourceId_eMpeg0:
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e10bit;
                        pBoxVdcSrcCap[i].bMtgCapable = true;
                        pBoxVdcSrcCap[i].bAvailable = true;
                        break;
                    case BAVC_SourceId_eMpeg1:
                    case BAVC_SourceId_eMpeg2:
                        pBoxVdcSrcCap[i].bMtgCapable = true;
                        pBoxVdcSrcCap[i].bAvailable = true;
                        break;

                    case BAVC_SourceId_eHdDvi0:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 2160;
                        pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 3840;
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

                    case BAVC_SourceId_eGfx3:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 720;
                        pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 1280;
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
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e10bit;
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].bMtgCapable = true;
                        break;
                    case BAVC_SourceId_eMpeg1:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].bMtgCapable = true;
                        break;

                    case BAVC_SourceId_eHdDvi0:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 2160;
                        pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 3840;
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
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e10bit;
                        pBoxVdcSrcCap[i].bAvailable = true;
                        break;
                    case BAVC_SourceId_eMpeg1:
                    case BAVC_SourceId_eMpeg2:
                    case BAVC_SourceId_eMpeg3:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        break;

                    case BAVC_SourceId_eHdDvi0:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 2160;
                        pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 3840;
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
                    case BAVC_SourceId_eMpeg2:
                    case BAVC_SourceId_eMpeg3:
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e10bit;
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

        case 10:
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
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e10bit;
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].bMtgCapable = true;
                        break;

                    case BAVC_SourceId_eHdDvi0:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 2160;
                        pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 3840;
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

        case 11:
            BDBG_ERR(("BOX %d is in progress.", ulBoxId));
            break;

        case 12:
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
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e10bit;
                        pBoxVdcSrcCap[i].bAvailable = true;
                        break;

                    case BAVC_SourceId_eHdDvi0:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 2160;
                        pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 3840;
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

        case 13:
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
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e10bit;
                        pBoxVdcSrcCap[i].bMtgCapable = true;
                        pBoxVdcSrcCap[i].bAvailable = true;
                        break;

                    case BAVC_SourceId_eHdDvi0:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 2160;
                        pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 3840;
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

        case 14:
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
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e10bit;
                        pBoxVdcSrcCap[i].bAvailable = true;
                        break;

                    case BAVC_SourceId_eHdDvi0:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 2160;
                        pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 3840;
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

        case 15:
            BDBG_ERR(("BOX %d is in progress.", ulBoxId));
            break;

        case 16:
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
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e10bit;
                        pBoxVdcSrcCap[i].bMtgCapable = true;
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

        case 17:
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
                    case BAVC_SourceId_eMpeg2:
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e10bit;
                        pBoxVdcSrcCap[i].bMtgCapable = true;
                        pBoxVdcSrcCap[i].bAvailable = true;
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

        case 18:
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
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e10bit;
                        pBoxVdcSrcCap[i].bAvailable = true;
                        break;

                    case BAVC_SourceId_eHdDvi0:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 3840;
                        pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 2160;
                        break;

                    case BAVC_SourceId_eGfx2:
                    case BAVC_SourceId_eGfx3:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 1080;
                        pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 1920;
                        pBoxVdcSrcCap[i].eColorSpace = BBOX_Vdc_Colorspace_eRGB;
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e8bit;
                        break;
                }
            }
            break;

        case 19:
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
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e10bit;
                        pBoxVdcSrcCap[i].bMtgCapable = true;
                        pBoxVdcSrcCap[i].bAvailable = true;
                        break;

                    case BAVC_SourceId_eHdDvi0:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 3840;
                        pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 2160;
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

        case 20:
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
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e10bit;
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

        case 21:
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
                    case BAVC_SourceId_eMpeg2:
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e10bit;
                        pBoxVdcSrcCap[i].bAvailable = true;
                        break;

                    case BAVC_SourceId_eHdDvi0:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 3840;
                        pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 2160;
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

        case 22:
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
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e10bit;
                        pBoxVdcSrcCap[i].bAvailable = true;
                        break;
                }
            }
            break;

        case 23:
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
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e10bit;
                        pBoxVdcSrcCap[i].bMtgCapable = true;
                        pBoxVdcSrcCap[i].bAvailable = true;
                        break;
                    case BAVC_SourceId_eHdDvi0:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 1920;
                        pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 1080;
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

        case 24:
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
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e10bit;
                        pBoxVdcSrcCap[i].bAvailable = true;
                        break;
                    case BAVC_SourceId_eGfx2:
                    case BAVC_SourceId_eGfx3:
                        pBoxVdcSrcCap[i].bAvailable = true;
                        pBoxVdcSrcCap[i].stSizeLimits.ulHeight = 1080;
                        pBoxVdcSrcCap[i].stSizeLimits.ulWidth = 1920;
                        pBoxVdcSrcCap[i].eColorSpace = BBOX_Vdc_Colorspace_eRGB;
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e8bit;
                        break;
                }
            }
            break;

        case 25:
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
                    case BAVC_SourceId_eMpeg2:
                    case BAVC_SourceId_eMpeg3:
                        pBoxVdcSrcCap[i].eBpp = BBOX_Vdc_Bpp_e10bit;
                        pBoxVdcSrcCap[i].bMtgCapable = true;
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
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass2;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MCVP is available for C0V0 */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD;
                                /* Disable CAP when possible; SCL placement is based on BW. Live Feed! */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eAutoDisable1080p;
                            }
                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }

                            break;
                    }
                }
            }
            break;

        case 2:
            for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
            {
                pBoxVdcDispCap[i].bAvailable = false;
                pBoxVdcDispCap[i].eMaxVideoFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMosaicModeClass = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].stStgEnc.bAvailable = false;
                pBoxVdcDispCap[i].stStgEnc.ulStgId = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = BBOX_FTR_INVALID;

                /* Display-STG-Encoder mapping */
                switch(i)
                {
                    case 2:
                        pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
                        pBoxVdcDispCap[i].stStgEnc.ulStgId = 1;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = 1;
                        break;
                    case 3:
                        pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
                        pBoxVdcDispCap[i].stStgEnc.ulStgId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = 0;
                        break;
                }

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
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass2;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MCVP is available  */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD;
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==1) /* pip win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MAD-R is available  */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD;
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = 2;
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = 2;
                            }
                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;
                        case 1:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass0;

                            if (j==0 || j==1) /* main and pip win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }

                            /* Smooth scaling */
                            if (j==0)
                            {
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==1) /* pip win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = 2;
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = 2;
                            }
                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;
                        case 2:
                        case 3:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e720p;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e720p;
                            pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eAutoDisable;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MAD-R is available for C2V0 and C3V0 */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD;
                            }
                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;
                    }
                }
            }
            break;

        case 3:
            for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
            {
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
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass2;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MCVP is available  */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD;
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==1) /* pip win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MADR_0 is available  */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD;
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = 2;
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = 2;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;

                        case 1:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass0;

                            if (j==0 || j==1) /* main and pip win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }

                            /* Smooth scaling */
                            if (j==0)
                            {
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==1) /* pip win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = 2;
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = 2;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;
                    }
                }
            }
            break;

        case 4:
            for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
            {
                pBoxVdcDispCap[i].bAvailable = false;
                pBoxVdcDispCap[i].eMaxVideoFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMosaicModeClass = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].stStgEnc.bAvailable = false;
                pBoxVdcDispCap[i].stStgEnc.ulStgId = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = BBOX_FTR_INVALID;

                /* Display-STG-Encoder mapping */
                switch(i)
                {
                    case 2:
                        pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
                        pBoxVdcDispCap[i].stStgEnc.ulStgId = 1;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = 1;
                        break;
                    case 3:
                        pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
                        pBoxVdcDispCap[i].stStgEnc.ulStgId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = 0;
                        break;
                }

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
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass2;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MCVP is available  */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD;
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;

                        case 1:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass0;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;

                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;

                        case 2:
                        case 3:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e1080p_25Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e1080p_25Hz;
                            pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eAutoDisable;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MAD-R is available  */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD;
                            }
                            break;
                    }
                }
            }
            break;

        case 5:
            for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
            {
                pBoxVdcDispCap[i].bAvailable = false;
                pBoxVdcDispCap[i].eMaxVideoFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMosaicModeClass = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].stStgEnc.bAvailable = false;
                pBoxVdcDispCap[i].stStgEnc.ulStgId = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = BBOX_FTR_INVALID;

                /* Display-STG-Encoder mapping */
                switch(i)
                {
                    case 3:
                        pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
                        pBoxVdcDispCap[i].stStgEnc.ulStgId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = 0;
                        break;
                }

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
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass2;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MCVP is available  */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD; /* BBOX_FTR_HD */
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==1) /* pip win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MADR_0 is available  */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD; /* BBOX_FTR_HD_MR0 */
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = 1;
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = 2;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;

                        case 1:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass0;

                            if (j==0 || j==1) /* main and pip win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }

                            /* Smooth scaling */
                            if (j==0)
                            {
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==1) /* pip win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = 1;
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = 2;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;

                        case 3:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e1080p_30Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e1080p_30Hz;
                            pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eAutoDisable;

                            if (j==0)
                            {
                                /* MADR_2 is available  */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD; /* BBOX_FTR_HD_MR2 */
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;

                    }
                }
            }
            break;

        case 6:
            for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
            {
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
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass2;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MCVP is available  */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD; /* BBOX_FTR_HD */
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==1) /* pip win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MADR_0 is available  */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD; /* BBOX_FTR_HD_MR0 */
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = 1;
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = 2;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;

                        case 1:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass0;

                            if (j==0 || j==1) /* main and pip win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }

                            /* Smooth scaling */
                            if (j==0)
                            {
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==1) /* pip win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = 1;
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = 2;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;
                    }
                }
            }
            break;

        case 7:
            for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
            {
                pBoxVdcDispCap[i].bAvailable = false;
                pBoxVdcDispCap[i].eMaxVideoFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMosaicModeClass = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].stStgEnc.bAvailable = false;
                pBoxVdcDispCap[i].stStgEnc.ulStgId = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = BBOX_FTR_INVALID;

                /* Display-STG-Encoder mapping */
                switch(i)
                {
                    case 2:
                        pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
                        pBoxVdcDispCap[i].stStgEnc.ulStgId = 1;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = 1;
                        break;
                    case 3:
                        pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
                        pBoxVdcDispCap[i].stStgEnc.ulStgId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = 0;
                        break;
                }

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
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e3840x2160p_50Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e3840x2160p_50Hz;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass2;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MCVP is available  */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD; /* BBOX_FTR_HD */
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==1) /* pip win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MADR_0 is available  */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD; /* BBOX_FTR_HD_MR0 */
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                                /* can be full size if decode is 1080p25/50i or less. If larger, then 1/2 x 1/2. */
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = 1;
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = 1;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;

                        case 1:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass0;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==1) /* pip win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                                /* can be full size if decode is 1080p25/50i or less. If larger, then 1/2 x 1/2. */
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = 1;
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = 1;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;

                        case 2:
                        case 3:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e720p_25Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e720p_25Hz;
                            pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eAutoDisable;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MAD-R is available ie., BBOX_FTR_HD_MR1 or BBOX_FTR_HD_MR2 */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD;
                            }
                            break;
                    }
                }
            }
            break;

        case 8:
            BDBG_ERR(("BOX %d is in progress.", ulBoxId));
            break;

        case 9:
            for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
            {
                pBoxVdcDispCap[i].bAvailable = false;
                pBoxVdcDispCap[i].eMaxVideoFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMosaicModeClass = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].stStgEnc.bAvailable = false;
                pBoxVdcDispCap[i].stStgEnc.ulStgId = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = BBOX_FTR_INVALID;

                /* Display-STG-Encoder mapping */
                switch(i)
                {
                    case 2:
                        pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
                        pBoxVdcDispCap[i].stStgEnc.ulStgId = 1;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = 1;
                        break;
                    case 3:
                        pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
                        pBoxVdcDispCap[i].stStgEnc.ulStgId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = 0;
                        break;
                }

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
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass2;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MCVP is available  */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD; /* BBOX_FTR_HD */
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;

                        case 1:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass0;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;

                        case 2:
                        case 3:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e1080p_25Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e1080p_25Hz;
                            pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eAutoDisable;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MAD-R is available ie., BBOX_FTR_HD_MR1 or BBOX_FTR_HD_MR2 */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD;
                            }
                            break;
                    }
                }
            }
            break;

        case 10:
            for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
            {
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
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass2;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MCVP is available  */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD; /* BBOX_FTR_HD */
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==1) /* pip win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MADR0 for C0V1 is available  */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD; /* BBOX_FTR_HD_MR0 */
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                                /* size limit */
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = 1;
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = 1;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;

                        case 1:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass0;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==1) /* pip win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                                /* size limit */
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = 1;
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = 1;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;
                    }
                }
            }
            break;

        case 11:
            BDBG_ERR(("BOX %d is in progress.", ulBoxId));
            break;

        case 12:
            for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
            {
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
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e3840x2160p_50Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e3840x2160p_50Hz;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass2;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MCVP is available for C0V0 */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD;
                                /* Disable CAP when possible; SCL placement is based on BW. Live Feed! */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eAutoDisable1080p;
                            }

                            if (j==1) /* pip win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MCVP is available for C0V0 */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD;
                                /* size limits */
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = 2;
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = 2;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }

                            break;
                    }
                }
            }
            break;

        case 13:
            for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
            {
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
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e3840x2160p_50Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e3840x2160p_50Hz;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass2;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MCVP is available  */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD; /* BBOX_FTR_HD */
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;

                        case 1:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass0;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;
                    }
                }
            }
            break;


        case 14:
            for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
            {
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
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass2;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MCVP is available  */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD; /* BBOX_FTR_HD */
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==1) /* pip win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MADR0 for C0V1 is available  */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD; /* BBOX_FTR_HD_MR0 */
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                                /* size limit */
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = 1;
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = 1;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;

                        case 1:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass0;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==1) /* pip win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                                /* size limit */
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = 1;
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = 1;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;
                    }
                }
            }
            break;

        case 15:
            BDBG_ERR(("BOX %d is in progress.", ulBoxId));
            break;

        case 16:
            for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
            {
                pBoxVdcDispCap[i].bAvailable = false;
                pBoxVdcDispCap[i].eMaxVideoFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMosaicModeClass = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].stStgEnc.bAvailable = false;
                pBoxVdcDispCap[i].stStgEnc.ulStgId = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = BBOX_FTR_INVALID;

                /* Display-STG-Encoder mapping */
                switch(i)
                {
                    case 3:
                        pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
                        pBoxVdcDispCap[i].stStgEnc.ulStgId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = 0;
                        break;
                }

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
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass2;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MCVP is available  */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD; /* BBOX_FTR_HD */
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;

                        case 1:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass0;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;

                        case 3:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e1080p_30Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e1080p_30Hz;
                            pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eAutoDisable;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MAD-R is available ie., BBOX_FTR_HD_MR2 */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD;
                            }
                            break;

                    }
                }
            }
            break;

        case 17:
            for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
            {
                pBoxVdcDispCap[i].bAvailable = false;
                pBoxVdcDispCap[i].eMaxVideoFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMosaicModeClass = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].stStgEnc.bAvailable = false;
                pBoxVdcDispCap[i].stStgEnc.ulStgId = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = BBOX_FTR_INVALID;

                /* Display-STG-Encoder mapping */
                switch(i)
                {
                    case 3:
                        pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
                        pBoxVdcDispCap[i].stStgEnc.ulStgId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = 0;
                        break;
                }

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
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e1080p;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass1;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MCVP is available  */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD; /* BBOX_FTR_HD */
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==1) /* pip win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MADR0 for C0V1 is available  */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD; /* BBOX_FTR_HD_MR0 */
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                                /* size limit */
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = 2;
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = 2;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;

                        case 1:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass0;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==1) /* pip win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                                /* size limit */
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = 2;
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = 2;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;

                        case 3:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e720p;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e720p;
                            pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eAutoDisable;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MAD-R is available */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD; /* FTR_HD_MR2 */
                            }
                            break;

                    }
                }
            }
            break;

        case 18:
            for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
            {
                pBoxVdcDispCap[i].bAvailable = false;
                pBoxVdcDispCap[i].eMaxVideoFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMosaicModeClass = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].stStgEnc.bAvailable = false;
                pBoxVdcDispCap[i].stStgEnc.ulStgId = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = BBOX_FTR_INVALID;

                /* Display-STG-Encoder mapping */
                switch(i)
                {
                    case 2:
                        pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
                        pBoxVdcDispCap[i].stStgEnc.ulStgId = 1;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = 1;
                        break;
                    case 3:
                        pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
                        pBoxVdcDispCap[i].stStgEnc.ulStgId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = 0;
                        break;
                }

                for (j=0; j < BBOX_VDC_WINDOW_COUNT_PER_DISPLAY; j++)
                {
                    pBoxVdcDispCap[i].astWindow[j].bAvailable = false;
                    pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = BBOX_VDC_DISREGARD;
                    pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = BBOX_VDC_DISREGARD;
                    pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_FTR_INVALID;
                    pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_VDC_DISREGARD;

                    switch (i)
                    {
                        case 2:
                        case 3:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e1080p_30Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e1080p_30Hz;
                            pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eAuto;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MAD-R is available for C2V0 and C3V0 */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD;
                            }
                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;
                    }
                }
            }
            break;

        case 19:
            for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
            {
                pBoxVdcDispCap[i].bAvailable = false;
                pBoxVdcDispCap[i].eMaxVideoFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMosaicModeClass = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].stStgEnc.bAvailable = false;
                pBoxVdcDispCap[i].stStgEnc.ulStgId = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = BBOX_FTR_INVALID;

                /* Display-STG-Encoder mapping */
                switch(i)
                {
                    case 3:
                        pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
                        pBoxVdcDispCap[i].stStgEnc.ulStgId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = 0;
                        break;
                }

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
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e3840x2160p_50Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e3840x2160p_50Hz;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass2;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MCVP is available  */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD; /* BBOX_FTR_HD */
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;

                        case 1:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass0;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;

                        case 3:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e720p_25Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e720p_25Hz;
                            pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eAutoDisable;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MAD-R is available */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD; /* FTR_HD_MR2 */
                            }
                            break;

                    }
                }
            }
            break;

        case 20:
            for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
            {
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
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e3840x2160p_50Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e3840x2160p_50Hz;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass2;

                            if (j==0 || j==1) /* main and PIP win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MCVP and MADR1 are available for main and PIP respectively */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD;
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==1)
                            {
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = 2;
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = 2;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;

                        case 1:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass0;

                            if (j==0 || j==1) /* main and PIP win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==1)
                            {
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = 2;
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = 2;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;
                    }
                }
            }
            break;

        case 21:
            for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
            {
                pBoxVdcDispCap[i].bAvailable = false;
                pBoxVdcDispCap[i].eMaxVideoFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMosaicModeClass = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].stStgEnc.bAvailable = false;
                pBoxVdcDispCap[i].stStgEnc.ulStgId = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = BBOX_FTR_INVALID;

                /* Display-STG-Encoder mapping */
                switch(i)
                {
                    case 3:
                        pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
                        pBoxVdcDispCap[i].stStgEnc.ulStgId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = 0;
                        break;
                }

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
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass2;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MCVP is available  */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD;
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==1) /* pip win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MAD-R is available  */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD;
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                                /* Size limts */
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = 1;
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = 1;
                            }
                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;

                        case 1:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass0;

                            if (j==0 || j==1) /* main and pip win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==1) /* pip win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = 1;
                                pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = 1;
                            }
                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;

                        case 3:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e720p;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e720p;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MAD-R is available for C3V0 */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD;
                            }
                            break;
                    }
                }
            }
            break;

        case 22:
            for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
            {
                pBoxVdcDispCap[i].bAvailable = false;
                pBoxVdcDispCap[i].eMaxVideoFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMosaicModeClass = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].stStgEnc.bAvailable = false;
                pBoxVdcDispCap[i].stStgEnc.ulStgId = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = BBOX_FTR_INVALID;

                /* Display-STG-Encoder mapping */
                switch(i)
                {
                    case 2:
                        pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
                        pBoxVdcDispCap[i].stStgEnc.ulStgId = 1;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = 1;
                        break;
                    case 3:
                        pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
                        pBoxVdcDispCap[i].stStgEnc.ulStgId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = 0;
                        break;
                }

                for (j=0; j < BBOX_VDC_WINDOW_COUNT_PER_DISPLAY; j++)
                {
                    pBoxVdcDispCap[i].astWindow[j].bAvailable = false;
                    pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = BBOX_VDC_DISREGARD;
                    pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = BBOX_VDC_DISREGARD;
                    pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_FTR_INVALID;
                    pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_VDC_DISREGARD;

                    switch (i)
                    {
                        case 2:
                        case 3:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e1080p_30Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e1080p_30Hz;
                            pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eAutoDisable;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MAD-R is available for C2V0 and C3V0 */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD;
                            }
                            break;
                    }
                }
            }
            break;

        case 23:
            for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
            {
                pBoxVdcDispCap[i].bAvailable = false;
                pBoxVdcDispCap[i].eMaxVideoFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMosaicModeClass = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].stStgEnc.bAvailable = false;
                pBoxVdcDispCap[i].stStgEnc.ulStgId = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = BBOX_FTR_INVALID;

                /* Display-STG-Encoder mapping */
                switch(i)
                {
                    case 3:
                        pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
                        pBoxVdcDispCap[i].stStgEnc.ulStgId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = 0;
                        break;
                }

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
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass2;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MCVP is available  */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD;
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;

                        case 1:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass0;

                            if (j==0) /* main and pip win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;

                        case 3:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e720p;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e720p;
                            pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eAutoDisable;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MAD-R is available for C2V0 */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD;
                            }
                            break;
                    }
                }
            }
            break;

        case 24:
            for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
            {
                pBoxVdcDispCap[i].bAvailable = false;
                pBoxVdcDispCap[i].eMaxVideoFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMosaicModeClass = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].stStgEnc.bAvailable = false;
                pBoxVdcDispCap[i].stStgEnc.ulStgId = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = BBOX_FTR_INVALID;

                /* Display-STG-Encoder mapping */
                switch(i)
                {
                    case 2:
                        pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
                        pBoxVdcDispCap[i].stStgEnc.ulStgId = 1;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = 1;
                        break;
                    case 3:
                        pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
                        pBoxVdcDispCap[i].stStgEnc.ulStgId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = 0;
                        break;
                }

                for (j=0; j < BBOX_VDC_WINDOW_COUNT_PER_DISPLAY; j++)
                {
                    pBoxVdcDispCap[i].astWindow[j].bAvailable = false;
                    pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulHeightFraction = BBOX_VDC_DISREGARD;
                    pBoxVdcDispCap[i].astWindow[j].stSizeLimits.ulWidthFraction = BBOX_VDC_DISREGARD;
                    pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_FTR_INVALID;
                    pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_VDC_DISREGARD;

                    switch (i)
                    {
                        case 2:
                        case 3:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e1080p_30Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e1080p_30Hz;
                            pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eAutoDisable;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MAD-R is available for C2V0 and C3V0 */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;
                    }
                }
            }
            break;

        case 25:
            for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
            {
                pBoxVdcDispCap[i].bAvailable = false;
                pBoxVdcDispCap[i].eMaxVideoFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].eMosaicModeClass = BBOX_VDC_DISREGARD;
                pBoxVdcDispCap[i].stStgEnc.bAvailable = false;
                pBoxVdcDispCap[i].stStgEnc.ulStgId = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = BBOX_FTR_INVALID;
                pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = BBOX_FTR_INVALID;

                /* Display-STG-Encoder mapping */
                switch(i)
                {
                    case 2:
                        pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
                        pBoxVdcDispCap[i].stStgEnc.ulStgId = 1;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = 1;
                        break;
                    case 3:
                        pBoxVdcDispCap[i].stStgEnc.bAvailable = true;
                        pBoxVdcDispCap[i].stStgEnc.ulStgId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderCoreId = 0;
                        pBoxVdcDispCap[i].stStgEnc.ulEncoderChannel = 0;
                        break;
                }

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
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e3840x2160p_60Hz;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass2;

                            if (j==0 || j==1) /* main and pip win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MCVP is available  */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD;
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;

                        case 1:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_ePAL_G;
                            pBoxVdcDispCap[i].eMosaicModeClass = BBOX_Vdc_MosaicModeClass_eClass0;

                            if (j==0 || j==1) /* main and pip win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* Smooth scaling */
                                pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eSclBeforeCap;
                            }

                            if (j==2) /* gfx win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                            }
                            break;

                        case 2:
                        case 3:
                            pBoxVdcDispCap[i].bAvailable = true;
                            pBoxVdcDispCap[i].eMaxVideoFmt = BFMT_VideoFmt_e720p;
                            pBoxVdcDispCap[i].eMaxHdmiDisplayFmt = BFMT_VideoFmt_e720p;
                            pBoxVdcDispCap[i].astWindow[j].eSclCapBias = BBOX_Vdc_SclCapBias_eAutoDisable;

                            if (j==0) /* main win */
                            {
                                pBoxVdcDispCap[i].astWindow[j].bAvailable = true;
                                /* MAD-R is available for C2V0 and C3V0 */
                                pBoxVdcDispCap[i].astWindow[j].stResource.ulMad = BBOX_VDC_DISREGARD;
                            }
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
        for (i=0; i < BBOX_VDC_DEINTERLACER_COUNT; i++)
        {
            pDeinterlacerCap[i].stPictureLimits.ulWidth = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].stPictureLimits.ulHeight = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].ulHsclThreshold = BBOX_VDC_DISREGARD;

            switch (i)
            {
                case 0:
                case 1:
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1920;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    break;
                case 2:
                case 3:
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1920;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    pDeinterlacerCap[i].ulHsclThreshold = 1280;
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
                case 1:
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 720;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 480;
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
                case 2:
                case 3:
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1920;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    pDeinterlacerCap[i].ulHsclThreshold = 1920;
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

            switch (i)
            {
                case 0:
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1920;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    break;
                case 1:
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 720;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 480;
                    break;
                case 3:
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1920;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    pDeinterlacerCap[i].ulHsclThreshold = 1920;
                    break;
            }
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
                case 1:
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 720;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 480;
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
                case 1:
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1920;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    break;
                case 2:
                case 3:
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1280;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    pDeinterlacerCap[i].ulHsclThreshold = 1280;
                    break;
            }
        }
        break;

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
                case 2:
                case 3:
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1920;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    pDeinterlacerCap[i].ulHsclThreshold = 1920;
                    break;
            }
        }
        break;

    case 10:
        for (i=0; i < BBOX_VDC_DEINTERLACER_COUNT; i++)
        {
            pDeinterlacerCap[i].stPictureLimits.ulWidth = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].stPictureLimits.ulHeight = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].ulHsclThreshold = BBOX_VDC_DISREGARD;

            switch (i)
            {
                case 0:
                case 1:
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1920;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    break;
            }
        }
        break;

    case 12:
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
                case 1:
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 720;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 576;
                    break;
            }
        }
        break;

    case 13:
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


    case 14:
        for (i=0; i < BBOX_VDC_DEINTERLACER_COUNT; i++)
        {
            pDeinterlacerCap[i].stPictureLimits.ulWidth = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].stPictureLimits.ulHeight = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].ulHsclThreshold = BBOX_VDC_DISREGARD;

            switch (i)
            {
                case 0:
                case 1:
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1920;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    break;
            }
        }
        break;

    case 16:
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
                case 3:
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1920;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    pDeinterlacerCap[i].ulHsclThreshold = 1920;
                    break;
            }
        }
        break;

    case 17:
        for (i=0; i < BBOX_VDC_DEINTERLACER_COUNT; i++)
        {
            pDeinterlacerCap[i].stPictureLimits.ulWidth = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].stPictureLimits.ulHeight = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].ulHsclThreshold = BBOX_VDC_DISREGARD;

            switch (i)
            {
                case 0: /* MCVP */
                case 1: /* MAD_R0 */
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1920;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    break;
                case 3: /* MAD_R2 */
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1920;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    pDeinterlacerCap[i].ulHsclThreshold = 1280;
                    break;
            }
        }
        break;

    case 18:
        for (i=0; i < BBOX_VDC_DEINTERLACER_COUNT; i++)
        {
            pDeinterlacerCap[i].stPictureLimits.ulWidth = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].stPictureLimits.ulHeight = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].ulHsclThreshold = BBOX_VDC_DISREGARD;

            switch (i)
            {
                case 2:
                case 3:
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1920;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    pDeinterlacerCap[i].ulHsclThreshold = 1920;
                    break;
            }
        }
        break;

    case 19:
        for (i=0; i < BBOX_VDC_DEINTERLACER_COUNT; i++)
        {
            pDeinterlacerCap[i].stPictureLimits.ulWidth = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].stPictureLimits.ulHeight = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].ulHsclThreshold = BBOX_VDC_DISREGARD;

            switch (i)
            {
                case 0: /* MCVP */
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1920;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    break;
                case 3: /* MAD_R2 */
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1920;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    pDeinterlacerCap[i].ulHsclThreshold = 1280;
                    break;
            }
        }
        break;

    case 20:
        for (i=0; i < BBOX_VDC_DEINTERLACER_COUNT; i++)
        {
            pDeinterlacerCap[i].stPictureLimits.ulWidth = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].stPictureLimits.ulHeight = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].ulHsclThreshold = BBOX_VDC_DISREGARD;

            switch (i)
            {
                case 0: /* MCVP */
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1920;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    break;
                case 1: /* MAD_R1 */
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 720;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 576;
                    break;
            }
        }
        break;

    case 21:
        for (i=0; i < BBOX_VDC_DEINTERLACER_COUNT; i++)
        {
            pDeinterlacerCap[i].stPictureLimits.ulWidth = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].stPictureLimits.ulHeight = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].ulHsclThreshold = BBOX_VDC_DISREGARD;

            switch (i)
            {
                case 0:
                case 1:
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1920;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    break;
                case 3:
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1280;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    pDeinterlacerCap[i].ulHsclThreshold = 1280;
                    break;
            }
        }
        break;

    case 22:
        for (i=0; i < BBOX_VDC_DEINTERLACER_COUNT; i++)
        {
            pDeinterlacerCap[i].stPictureLimits.ulWidth = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].stPictureLimits.ulHeight = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].ulHsclThreshold = BBOX_VDC_DISREGARD;

            switch (i)
            {
                case 2:
                case 3:
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1920;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    pDeinterlacerCap[i].ulHsclThreshold = 1920;
                    break;
            }
        }
        break;

    case 23:
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
                case 3:
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1920;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    pDeinterlacerCap[i].ulHsclThreshold = 1280;
                    break;
            }
        }
        break;

    case 24:
        for (i=0; i < BBOX_VDC_DEINTERLACER_COUNT; i++)
        {
            pDeinterlacerCap[i].stPictureLimits.ulWidth = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].stPictureLimits.ulHeight = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].ulHsclThreshold = BBOX_VDC_DISREGARD;

            switch (i)
            {
                case 2:
                case 3:
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1920;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    pDeinterlacerCap[i].ulHsclThreshold = 1920;
                    break;
            }
        }
        break;

    case 25:
        for (i=0; i < BBOX_VDC_DEINTERLACER_COUNT; i++)
        {
            pDeinterlacerCap[i].stPictureLimits.ulWidth = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].stPictureLimits.ulHeight = BBOX_VDC_DISREGARD;
            pDeinterlacerCap[i].ulHsclThreshold = BBOX_VDC_DISREGARD;

            switch (i)
            {
                case 0:
                case 1:
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1920;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    break;
                case 2:
                case 3:
                    pDeinterlacerCap[i].stPictureLimits.ulWidth = 1920;
                    pDeinterlacerCap[i].stPictureLimits.ulHeight = 1080;
                    pDeinterlacerCap[i].ulHsclThreshold = 1280;
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
        pXcodeCap->ulNumXcodeCapVfd  = BBOX_VDC_DISREGARD;
        pXcodeCap->ulNumXcodeGfd = BBOX_VDC_DISREGARD;
        break;

    case 2:
        pXcodeCap->ulNumXcodeCapVfd  = 1;
        pXcodeCap->ulNumXcodeGfd = BBOX_VDC_DISREGARD;
        break;

    case 3:
        pXcodeCap->ulNumXcodeCapVfd  = BBOX_VDC_DISREGARD;
        pXcodeCap->ulNumXcodeGfd = BBOX_VDC_DISREGARD;
        break;

    case 4:
        pXcodeCap->ulNumXcodeCapVfd  = 0;
        pXcodeCap->ulNumXcodeGfd = BBOX_VDC_DISREGARD;
        break;

    case 5:
        pXcodeCap->ulNumXcodeCapVfd  = BBOX_VDC_DISREGARD;
        pXcodeCap->ulNumXcodeGfd = BBOX_VDC_DISREGARD;
        break;

    case 6:
        pXcodeCap->ulNumXcodeCapVfd  = BBOX_VDC_DISREGARD;
        pXcodeCap->ulNumXcodeGfd = BBOX_VDC_DISREGARD;
        break;

    case 7:
        pXcodeCap->ulNumXcodeCapVfd  = BBOX_VDC_DISREGARD;
        pXcodeCap->ulNumXcodeGfd = BBOX_VDC_DISREGARD;
        break;

    case 9:
        pXcodeCap->ulNumXcodeCapVfd  = BBOX_VDC_DISREGARD;
        pXcodeCap->ulNumXcodeGfd = BBOX_VDC_DISREGARD;
        break;

    case 10:
        pXcodeCap->ulNumXcodeCapVfd  = BBOX_VDC_DISREGARD;
        pXcodeCap->ulNumXcodeGfd = BBOX_VDC_DISREGARD;
        break;

    case 12:
        pXcodeCap->ulNumXcodeCapVfd  = BBOX_VDC_DISREGARD;
        pXcodeCap->ulNumXcodeGfd = BBOX_VDC_DISREGARD;
        break;

    case 13:
        pXcodeCap->ulNumXcodeCapVfd  = BBOX_VDC_DISREGARD;
        pXcodeCap->ulNumXcodeGfd = BBOX_VDC_DISREGARD;
        break;


    case 14:
        pXcodeCap->ulNumXcodeCapVfd  = BBOX_VDC_DISREGARD;
        pXcodeCap->ulNumXcodeGfd = BBOX_VDC_DISREGARD;
        break;

    case 16:
        pXcodeCap->ulNumXcodeCapVfd  = BBOX_VDC_DISREGARD;
        pXcodeCap->ulNumXcodeGfd = BBOX_VDC_DISREGARD;
        break;

    case 17:
        pXcodeCap->ulNumXcodeCapVfd  = 1;
        pXcodeCap->ulNumXcodeGfd = BBOX_VDC_DISREGARD;
        break;

    case 18:
        pXcodeCap->ulNumXcodeCapVfd  = BBOX_VDC_DISREGARD;
        pXcodeCap->ulNumXcodeGfd = BBOX_VDC_DISREGARD;
        break;

    case 19:
        pXcodeCap->ulNumXcodeCapVfd  = BBOX_VDC_DISREGARD;
        pXcodeCap->ulNumXcodeGfd = BBOX_VDC_DISREGARD;
        break;

    case 20:
        pXcodeCap->ulNumXcodeCapVfd  = BBOX_VDC_DISREGARD;
        pXcodeCap->ulNumXcodeGfd = BBOX_VDC_DISREGARD;
        break;

    case 21:
        pXcodeCap->ulNumXcodeCapVfd  = BBOX_VDC_DISREGARD;
        pXcodeCap->ulNumXcodeGfd = BBOX_VDC_DISREGARD;
        break;

    case 22:
        pXcodeCap->ulNumXcodeCapVfd  = BBOX_VDC_DISREGARD;
        pXcodeCap->ulNumXcodeGfd = BBOX_VDC_DISREGARD;
        break;

    case 23:
        pXcodeCap->ulNumXcodeCapVfd  = BBOX_VDC_DISREGARD;
        pXcodeCap->ulNumXcodeGfd = BBOX_VDC_DISREGARD;
        break;

    case 24:
        pXcodeCap->ulNumXcodeCapVfd  = BBOX_VDC_DISREGARD;
        pXcodeCap->ulNumXcodeGfd = 2;
        break;

    case 25:
        pXcodeCap->ulNumXcodeCapVfd  = BBOX_VDC_DISREGARD;
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

    if (ulBoxId == 0 || ulBoxId == 8 || ulBoxId == 11 ||
        ulBoxId == 15 || ulBoxId > BBOX_MODES_SUPPORTED)
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
    BERR_Code eStatus = BERR_SUCCESS;

    if (ulBoxId == 0 || ulBoxId == 8 || ulBoxId == 11 ||
        ulBoxId == 15 || ulBoxId > BBOX_MODES_SUPPORTED)
    {
        BDBG_ERR(("Box mode %d is not supported on this chip.", ulBoxId));
        return (BERR_INVALID_PARAMETER);
    }

    switch (ulBoxId)
    {
        case 1:
            *pBoxMemConfig = stBoxMemConfig_7439B0_box1;
            pBoxMemConfig->ulNumMemc = stBoxRts_7251S_4K_dongle_box1.ulNumMemc;
            break;
        case 2:
            *pBoxMemConfig = stBoxMemConfig_7439B0_box2;
            pBoxMemConfig->ulNumMemc = stBoxRts_7252S_4K2t_box2.ulNumMemc;
            break;
        case 3:
            *pBoxMemConfig = stBoxMemConfig_7439B0_box3;
            pBoxMemConfig->ulNumMemc = stBoxRts_7252S_4K_gfxpip_box3.ulNumMemc;
            break;
        case 4:
            *pBoxMemConfig = stBoxMemConfig_7439B0_box4;
            pBoxMemConfig->ulNumMemc = stBoxRts_7252S_4K2t_large_xcode_box4.ulNumMemc;
            break;
        case 5:
            *pBoxMemConfig = stBoxMemConfig_7439B0_box5;
            pBoxMemConfig->ulNumMemc = stBoxRts_7439_4Kstb1t_box5.ulNumMemc;
            break;
        case 6:
            *pBoxMemConfig = stBoxMemConfig_7439B0_box6;
            pBoxMemConfig->ulNumMemc = stBoxRts_7439_4Kstb_box6.ulNumMemc;
            break;
        case 7:
            *pBoxMemConfig = stBoxMemConfig_7439B0_box7;
            pBoxMemConfig->ulNumMemc = stBoxRts_7439_4Kstb2t50_box7.ulNumMemc;
            break;
        case 9:
            *pBoxMemConfig = stBoxMemConfig_7439B0_box9;
            pBoxMemConfig->ulNumMemc = stBoxRts_7252S_4K2t_large_xcode_HiTemp_box9.ulNumMemc;
            break;
        case 10:
            *pBoxMemConfig = stBoxMemConfig_7439B0_box10;
            pBoxMemConfig->ulNumMemc = stBoxRts_7439_4Kstb_hitemp_box10.ulNumMemc;
            break;
        case 12:
            *pBoxMemConfig = stBoxMemConfig_7439B0_box12;
            pBoxMemConfig->ulNumMemc = stBoxRts_7251_4kstb50_box12.ulNumMemc;
            break;
        case 13:
            *pBoxMemConfig = stBoxMemConfig_7439B0_box13;
            pBoxMemConfig->ulNumMemc = stBoxRts_7251_4k50_box13.ulNumMemc;
            break;
        case 14:
            *pBoxMemConfig = stBoxMemConfig_7439B0_box14;
            pBoxMemConfig->ulNumMemc = stBoxRts_7252_4Kstb_933_box14.ulNumMemc;
            break;
        case 16:
            *pBoxMemConfig = stBoxMemConfig_7439B0_box16;
            pBoxMemConfig->ulNumMemc = stBoxRts_7439_4K1t_933_box16.ulNumMemc;
            break;
        case 17:
            *pBoxMemConfig = stBoxMemConfig_7439B0_box17;
            pBoxMemConfig->ulNumMemc = stBoxRts_7251_stb1t_box17.ulNumMemc;
            break;
        case 18:
            *pBoxMemConfig = stBoxMemConfig_7439B0_box18;
            pBoxMemConfig->ulNumMemc = stBoxRts_7252_headless_dualxcode_box18.ulNumMemc;
            break;
        case 19:
            *pBoxMemConfig = stBoxMemConfig_7439B0_box19;
            pBoxMemConfig->ulNumMemc = stBoxRts_7251_4kp251t_box19.ulNumMemc;
            break;
        case 20:
            *pBoxMemConfig = stBoxMemConfig_7439B0_box20;
            pBoxMemConfig->ulNumMemc = stBoxRts_7251_4kstb50_box20.ulNumMemc;
            break;
        case 21:
            *pBoxMemConfig = stBoxMemConfig_7439B0_box21;
            pBoxMemConfig->ulNumMemc = stBoxRts_7439_4Kstb1t_box21.ulNumMemc;
            break;
        case 22:
            *pBoxMemConfig = stBoxMemConfig_7439B0_box22;
            pBoxMemConfig->ulNumMemc = stBoxRts_7251_headless_dualxcode_box22.ulNumMemc;
            break;
        case 23:
            *pBoxMemConfig = stBoxMemConfig_7439B0_box23;
            pBoxMemConfig->ulNumMemc = stBoxRts_7251S_1u1t4k_box23.ulNumMemc;
            break;
        case 24:
            *pBoxMemConfig = stBoxMemConfig_7439B0_box24;
            pBoxMemConfig->ulNumMemc = stBoxRts_7252s_headless_dualxcode_pip_box24.ulNumMemc;
            break;
        case 25:
            *pBoxMemConfig = stBoxMemConfig_7439B0_box25;
            pBoxMemConfig->ulNumMemc = stBoxRts_7439_1stb2t_box25.ulNumMemc;
            break;
    }

    return eStatus;
}

BERR_Code BBOX_P_LoadRts
    ( const BREG_Handle      hReg,
      const uint32_t         ulBoxId )
{
    BERR_Code eStatus = BERR_SUCCESS;

    if (ulBoxId == 0 || ulBoxId == 8 || ulBoxId == 11 ||
        ulBoxId == 15 || ulBoxId > BBOX_MODES_SUPPORTED)
    {
        BDBG_ERR(("Box mode %d is not supported on this chip.", ulBoxId));
        eStatus = BERR_INVALID_PARAMETER;
    }
    else
    {
        uint32_t i, j;
        BBOX_Rts *pBoxRts;

        switch (ulBoxId)
        {
            case 1:
                pBoxRts = &stBoxRts_7251S_4K_dongle_box1;
                break;
            case 2:
                pBoxRts = &stBoxRts_7252S_4K2t_box2;
                break;
            case 3:
                pBoxRts = &stBoxRts_7252S_4K_gfxpip_box3;
                break;
            case 4:
                pBoxRts = &stBoxRts_7252S_4K2t_large_xcode_box4;
                break;
            case 5:
                pBoxRts = &stBoxRts_7439_4Kstb1t_box5;
                break;
            case 6:
                pBoxRts = &stBoxRts_7439_4Kstb_box6;
                break;
            case 7:
                pBoxRts = &stBoxRts_7439_4Kstb2t50_box7;
                break;
            case 9:
                pBoxRts = &stBoxRts_7252S_4K2t_large_xcode_HiTemp_box9;
                break;
            case 10:
                pBoxRts = &stBoxRts_7439_4Kstb_hitemp_box10;
                break;
            case 12:
                pBoxRts = &stBoxRts_7251_4kstb50_box12;
                break;
            case 13:
                pBoxRts = &stBoxRts_7251_4k50_box13;
                break;
            case 14:
                pBoxRts = &stBoxRts_7252_4Kstb_933_box14;
                break;
            case 16:
                pBoxRts = &stBoxRts_7439_4K1t_933_box16;
                break;
            case 17:
                pBoxRts = &stBoxRts_7251_stb1t_box17;
                break;
            case 18:
                pBoxRts = &stBoxRts_7252_headless_dualxcode_box18;
                break;
            case 19:
                pBoxRts = &stBoxRts_7251_4kp251t_box19;
                break;
            case 20:
                pBoxRts = &stBoxRts_7251_4kstb50_box20;
                break;
            case 21:
                pBoxRts = &stBoxRts_7439_4Kstb1t_box21;
                break;
            case 22:
                pBoxRts = &stBoxRts_7251_headless_dualxcode_box22;
                break;
            case 23:
                pBoxRts = &stBoxRts_7251S_1u1t4k_box23;
                break;
            case 24:
                pBoxRts = &stBoxRts_7252s_headless_dualxcode_pip_box24;
                break;
            case 25:
                pBoxRts = &stBoxRts_7439_1stb2t_box25;
                break;
            default:
                pBoxRts = NULL;
        }

        BDBG_ASSERT(pBoxRts);

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
            else if (i==1)
            {
#ifdef BCHP_MEMC_ARB_1_REG_START
                ulMemcBaseAddr = BCHP_MEMC_ARB_1_CLIENT_INFO_0;

#else
                BDBG_ERR(("There is no MEMC1. Verify the number of MEMC defined in RTS file."));
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

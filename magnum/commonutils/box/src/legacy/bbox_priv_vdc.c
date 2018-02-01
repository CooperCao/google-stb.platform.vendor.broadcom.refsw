/******************************************************************************
* Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bfmt.h"
#include "bbox_priv.h"
#include "bbox_rts_priv.h"
#include "bbox_priv_modes.h"
#include "bbox_vdc.h"
#include "bbox_vdc_priv.h"

BDBG_MODULE(BBOX_PRIV);
BDBG_OBJECT_ID(BBOX_BOX_PRIV);

/* Default memc index table for chips without boxmode */
#if (BCHP_CHIP == 7400) || (BCHP_CHIP == 11360)
void BBOX_P_SetBox0MemConfig
    ( BBOX_MemConfig                *pBoxMemConfig )
{
    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display0,  Video0,  0,       0);
    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display0,  Video1,  0,       0);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display0,  Gfx0,    0         );

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display1,  Video0,  0,       0);
    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display1,  Video1,  0,       0);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display1,  Gfx0,    0         );

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display2,  Video0,  0,       0);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display2,  Gfx0,    0         );
}
#elif (BCHP_CHIP == 7405)
void BBOX_P_SetBox0MemConfig
    ( BBOX_MemConfig                *pBoxMemConfig )
{
    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display0,  Video0,  0,       0);
    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display0,  Video1,  0,       0);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display0,  Gfx0,    0         );

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display1,  Video0,  0,       0);
    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display1,  Video1,  0,       0);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display1,  Gfx0,    0         );
}
#elif (BCHP_CHIP == 7422)
void BBOX_P_SetBox0MemConfig
    ( BBOX_MemConfig                *pBoxMemConfig )
{
    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display0,  Video0,  1,       1);
    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display0,  Video1,  1,       1);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display0,  Gfx0,    0         );

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display1,  Video0,  1,       1);
    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display1,  Video1,  1,       1);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display1,  Gfx0,    0         );

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display2,  Video0,  1,       1);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display2,  Gfx0,    1         );

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display3,  Video0,  1,       1);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display3,  Gfx0,    1         );

}
#elif (BCHP_CHIP == 7425)
void BBOX_P_SetBox0MemConfig
    ( BBOX_MemConfig                *pBoxMemConfig )
{
    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display0,  Video0,  0,       0);
    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display0,  Video1,  1,       1);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display0,  Gfx0,    0         );

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display1,  Video0,  0,       0);
    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display1,  Video1,  1,       1);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display1,  Gfx0,    0         );

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display2,  Video0,  1,       1);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display2,  Gfx0,    1         );

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display3,  Video0,  1,       1);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display3,  Gfx0,    1         );

    BBOX_P_SET_NUM_MEMC(      pBoxMemConfig, 2 );

}
#elif (BCHP_CHIP == 7435)
void BBOX_P_SetBox0MemConfig
    ( BBOX_MemConfig                *pBoxMemConfig )
{
    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display0,  Video0,  1,       1);
    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display0,  Video1,  1,       1);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display0,  Gfx0,    0         );

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display1,  Video0,  0,       0);
    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display1,  Video1,  1,       1);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display1,  Gfx0,    1         );

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display2,  Video0,  1,       1);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display2,  Gfx0,    0         );

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display3,  Video0,  1,       1);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display3,  Gfx0,    0         );

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display4,  Video0,  0,       0);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display4,  Gfx0,    1         );

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display5,  Video0,  1,       1);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display5,  Gfx0,    1         );

}
#elif (BCHP_CHIP == 7445)
void BBOX_P_SetBox0MemConfig
    ( BBOX_MemConfig                *pBoxMemConfig )
{
    BBOX_P_SET_RDC_MEMC(pBoxMemConfig, 2);

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display0,  Video0,  1,       1);
    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display0,  Video1,  1,       1);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display0,  Gfx0,    2         );

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display1,  Video0,  2,       2);
    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display1,  Video1,  2,       2);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display1,  Gfx0,    2         );

    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display2,  Gfx0,    2         );

    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display3,  Gfx0,    0         );

    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display4,  Gfx0,    0         );

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display5,  Video0,  0,       0);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display5,  Gfx0,    2         );

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display6,  Video0,  0,       0);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display6,  Gfx0,    2         );
}
#elif (((BCHP_CHIP == 7439) && (BCHP_VER == BCHP_VER_A0))  || \
       ((BCHP_CHIP == 7366) && (BCHP_VER == BCHP_VER_A0))  || \
       ((BCHP_CHIP == 74371) && (BCHP_VER == BCHP_VER_A0)) || \
       ((BCHP_CHIP == 7271))||((BCHP_CHIP == 7278)))

void BBOX_P_SetBox0MemConfig
    ( BBOX_MemConfig                *pBoxMemConfig )
{
    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display0,  Video0,  0,       0);
    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display0,  Video1,  0,       0);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display0,  Gfx0,    0         );

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display1,  Video0,  0,       0);
    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display1,  Video1,  0,       0);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display1,  Gfx0,    0         );

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display2,  Video0,  0,       0);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display2,  Gfx0,    0         );

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display3,  Video0,  0, Invalid);

    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display4,  Video0,  0  Invalid);

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display5,  Video0,  0, Invalid);

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display6,  Video0,  0, Invalid);
}
#elif ((BCHP_CHIP==7439) && (BCHP_VER >= BCHP_VER_B0))
void BBOX_P_SetBox0MemConfig
    ( BBOX_MemConfig                *pBoxMemConfig )
{
    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display0,  Video0,  1,       1);
    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display0,  Video1,  1,       1);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display0,  Gfx0,    0         );

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display1,  Video0,  1,       1);
    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display1,  Video1,  0,       0);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display1,  Gfx0,    0         );

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display2,  Video0,  0,       0);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display2,  Gfx0,    0         );

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display3,  Video0,  0,       0);
}
#elif ((BCHP_CHIP == 7366) && (BCHP_VER >= BCHP_VER_B0))
void BBOX_P_SetBox0MemConfig
    ( BBOX_MemConfig                *pBoxMemConfig )
{
    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display0,  Video0,  1,       1);
    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display0,  Video1,  1,       1);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display0,  Gfx0,    1         );

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display1,  Video0,  0,       0);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display1,  Gfx0,    1         );

    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display2,  Gfx0,    0         );
}
#elif ((BCHP_CHIP==7552)  || (BCHP_CHIP==7358)  || (BCHP_CHIP==7360)  || \
       (BCHP_CHIP==7346)  || (BCHP_CHIP==7344)  || (BCHP_CHIP==7231)  || \
       (BCHP_CHIP==7429)  || (BCHP_CHIP==7584)  || \
       (BCHP_CHIP==7563)  || (BCHP_CHIP==7543)  || (BCHP_CHIP==7362)  || \
       (BCHP_CHIP==7364)  || (BCHP_CHIP==7228)  || (BCHP_CHIP==7250)  || \
       (BCHP_CHIP==75635) || (BCHP_CHIP==7586)  || (BCHP_CHIP==73625) || \
       (BCHP_CHIP==75845) || (BCHP_CHIP==74295) || (BCHP_CHIP==73465) || \
       (BCHP_CHIP==7268)  || (BCHP_CHIP==7255))

void BBOX_P_SetBox0MemConfig
    ( BBOX_MemConfig                *pBoxMemConfig )
{
    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display0,  Video0,  0,       0);
    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display0,  Video1,  0,       0);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display0,  Gfx0,    0         );

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display1,  Video0,  0,       0);
    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display1,  Video1,  0,       0);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display1,  Gfx0,    0         );
}
#endif

BERR_Code BBOX_P_ValidateId
    (uint32_t                ulId)
{
    BERR_Code eStatus = BERR_SUCCESS;
    if (ulId != 0)
    {
        BDBG_ERR(("Box Mode ID %d is not supported on this chip.", ulId));
        eStatus = BBOX_ID_NOT_SUPPORTED;
    }
    return eStatus;
}

void BBOX_P_Vdc_SetSourceCapabilities
    ( uint32_t                      ulBoxId,
      BBOX_Vdc_Source_Capabilities *pSourceCap )
{
    uint32_t i;

    BSTD_UNUSED(ulBoxId);

    for (i=0; i < BAVC_SourceId_eMax; i++)
    {
        BBOX_P_VDC_SET_LEGACY_SRC_LIMIT(pSourceCap, i);
    }
}

void BBOX_P_Vdc_SetDisplayCapabilities
    ( uint32_t                       ulBoxId,
      BBOX_Vdc_Display_Capabilities *pDisplayCap )
{
    uint32_t i, j;

    BSTD_UNUSED(ulBoxId);

    for (i=0; i < BBOX_VDC_DISPLAY_COUNT; i++)
    {
        if(i == 0)
            BBOX_P_VDC_SET_LEGACY_DISPLAY_LIMIT(pDisplayCap, i, Class1);
        else if(i == 1)
            BBOX_P_VDC_SET_LEGACY_DISPLAY_LIMIT(pDisplayCap, i, Class0);
        else
            BBOX_P_VDC_SET_LEGACY_DISPLAY_LIMIT(pDisplayCap, i, Disregard);


        for (j=0; j < BBOX_VDC_WINDOW_COUNT_PER_DISPLAY; j++)
        {
            BBOX_P_VDC_SET_LEGACY_WINDOW_LIMIT(pDisplayCap, i, j);
        }
    }
}

void BBOX_P_Vdc_SetDeinterlacerCapabilities
    ( uint32_t                            ulBoxId,
      BBOX_Vdc_Deinterlacer_Capabilities *pDeinterlacerCap )
{
    uint32_t i;

    BSTD_UNUSED(ulBoxId);

    /* Deinterlacer and transcode defaults */
    for (i=0; i < BBOX_VDC_DEINTERLACER_COUNT; i++)
    {
        BBOX_P_VDC_SET_LEGACY_DEINTERLACER_LIMIT(pDeinterlacerCap, i);
    }
}

void BBOX_P_Vdc_SetXcodeCapabilities
    ( uint32_t                     ulBoxId,
      BBOX_Vdc_Xcode_Capabilities *pXcodeCap )
{
    BSTD_UNUSED(ulBoxId);

    BBOX_P_VDC_SET_LEGACY_XCODE_LIMIT(pXcodeCap);
}

BERR_Code BBOX_P_SetMemConfig
    ( uint32_t                       ulBoxId,
      BBOX_MemConfig                *pBoxMemConfig )
{
    BERR_Code  eStatus = BERR_SUCCESS;

    BDBG_ASSERT(pBoxMemConfig);

    eStatus = BBOX_P_ValidateId(ulBoxId);
    if (eStatus != BERR_SUCCESS) return eStatus;

    /* Set default config settings  */
    BBOX_P_SetDefaultMemConfig(pBoxMemConfig);

    /* Set chip-specific default mem config */
    BBOX_P_SetBox0MemConfig(pBoxMemConfig);

    BDBG_MSG(("Default (box 0) settings are used."));
    return eStatus;
}

BERR_Code BBOX_P_GetRtsConfig
    ( const uint32_t         ulBoxId,
      BBOX_Rts              *pBoxRts )
{
    BSTD_UNUSED(ulBoxId);
    BSTD_UNUSED(pBoxRts);

    return BBOX_RTS_LOADED_BY_CFE;
}
/* end of file */

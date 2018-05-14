/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "bstd.h"                /* standard types */
#include "bkni.h"
#include "bdbg.h"                /* Debug message */
#include "bbox.h"
#include "bbox_priv.h"
#include "bbox_rts_priv.h"
#include "bbox_priv_modes.h"
#include "bbox_vdc.h"
#include "bbox_vdc_priv.h"

#include "bbox_vdc_box1_config.h"
#include "bbox_vdc_box2_config.h"
#include "bbox_vdc_box3_config.h"
#include "bbox_vdc_box4_config.h"
#include "bbox_vdc_box5_config.h"
#include "bbox_vdc_box6_config.h"
#include "bbox_vdc_box7_config.h"
#include "bbox_vdc_box8_config.h"
#include "bbox_vdc_box10_config.h"
#include "bbox_vdc_box13_config.h"
#include "bbox_vdc_box14_config.h"
#include "bbox_vdc_box15_config.h"
#include "bbox_vdc_box16_config.h"
#include "bbox_vdc_box17_config.h"
#include "bbox_vdc_box18_config.h"
#include "bbox_vdc_box19_config.h"
#include "bbox_vdc_box1000_config.h"
#include "bbox_vdc_box1001_config.h"

#include "bbox_rts_box1.h"
#include "bbox_rts_box2.h"
#include "bbox_rts_box3.h"
#include "bbox_rts_box4.h"
#include "bbox_rts_box5.h"
#include "bbox_rts_box6.h"
#include "bbox_rts_box7.h"
#include "bbox_rts_box8.h"
#include "bbox_rts_box10.h"
#include "bbox_rts_box12.h"
#include "bbox_rts_box13.h"
#include "bbox_rts_box14.h"
#include "bbox_rts_box15.h"
#include "bbox_rts_box16.h"
#include "bbox_rts_box17.h"
#include "bbox_rts_box18.h"
#include "bbox_rts_box19.h"
#include "bbox_rts_box1000.h"
#include "bbox_rts_box1001.h"

BDBG_MODULE(BBOX_PRIV);
BDBG_OBJECT_ID(BBOX_BOX_PRIV);


void BBOX_P_Vdc_SetSourceCapabilities
    ( uint32_t                      ulBoxId,
      BBOX_Vdc_Source_Capabilities *pSourceCap )
{
    switch (ulBoxId)
    {
        case 1:
        case 12:
            BBOX_P_Vdc_SetBox1SourceCapabilities(pSourceCap);
            break;
        case 2:
            BBOX_P_Vdc_SetBox2SourceCapabilities(pSourceCap);
            break;
        case 3:
            BBOX_P_Vdc_SetBox3SourceCapabilities(pSourceCap);
            break;
        case 4:
            BBOX_P_Vdc_SetBox4SourceCapabilities(pSourceCap);
            break;
        case 5:
            BBOX_P_Vdc_SetBox5SourceCapabilities(pSourceCap);
            break;
        case 6:
            BBOX_P_Vdc_SetBox6SourceCapabilities(pSourceCap);
            break;
        case 7:
            BBOX_P_Vdc_SetBox7SourceCapabilities(pSourceCap);
            break;
        case 8:
            BBOX_P_Vdc_SetBox8SourceCapabilities(pSourceCap);
            break;
        case 10:
            BBOX_P_Vdc_SetBox10SourceCapabilities(pSourceCap);
            break;
        case 13:
            BBOX_P_Vdc_SetBox13SourceCapabilities(pSourceCap);
            break;
        case 14:
        case 18:
            BBOX_P_Vdc_SetBox14SourceCapabilities(pSourceCap);
            break;
        case 15:
            BBOX_P_Vdc_SetBox15SourceCapabilities(pSourceCap);
            break;
        case 16:
            BBOX_P_Vdc_SetBox16SourceCapabilities(pSourceCap);
            break;
        case 17:
            BBOX_P_Vdc_SetBox17SourceCapabilities(pSourceCap);
            break;
        case 19:
            BBOX_P_Vdc_SetBox19SourceCapabilities(pSourceCap);
            break;
        case 1000:
            BBOX_P_Vdc_SetBox1000SourceCapabilities(pSourceCap);
            break;
        case 1001:
            BBOX_P_Vdc_SetBox1001SourceCapabilities(pSourceCap);
            break;
    }
}

void BBOX_P_Vdc_SetDisplayCapabilities
    ( uint32_t                       ulBoxId,
      BBOX_Vdc_Display_Capabilities *pDisplayCap )
{
    switch (ulBoxId)
    {
        case 1:
        case 12:
            BBOX_P_Vdc_SetBox1DisplayCapabilities(pDisplayCap);
            break;
        case 2:
            BBOX_P_Vdc_SetBox2DisplayCapabilities(pDisplayCap);
            break;
        case 3:
            BBOX_P_Vdc_SetBox3DisplayCapabilities(pDisplayCap);
            break;
        case 4:
            BBOX_P_Vdc_SetBox4DisplayCapabilities(pDisplayCap);
            break;
        case 5:
            BBOX_P_Vdc_SetBox5DisplayCapabilities(pDisplayCap);
            break;
        case 6:
            BBOX_P_Vdc_SetBox6DisplayCapabilities(pDisplayCap);
            break;
        case 7:
            BBOX_P_Vdc_SetBox7DisplayCapabilities(pDisplayCap);
            break;
        case 8:
            BBOX_P_Vdc_SetBox8DisplayCapabilities(pDisplayCap);
            break;
        case 10:
            BBOX_P_Vdc_SetBox10DisplayCapabilities(pDisplayCap);
            break;
        case 13:
            BBOX_P_Vdc_SetBox13DisplayCapabilities(pDisplayCap);
            break;
        case 14:
            BBOX_P_Vdc_SetBox14DisplayCapabilities(pDisplayCap);
            break;
        case 15:
            BBOX_P_Vdc_SetBox15DisplayCapabilities(pDisplayCap);
            break;
        case 16:
            BBOX_P_Vdc_SetBox16DisplayCapabilities(pDisplayCap);
            break;
        case 17:
            BBOX_P_Vdc_SetBox17DisplayCapabilities(pDisplayCap);
            break;
        case 18:
            BBOX_P_Vdc_SetBox14DisplayCapabilities(pDisplayCap);
            BBOX_P_Vdc_SetBox18DisplayCapabilities(pDisplayCap);
            break;
        case 19:
            BBOX_P_Vdc_SetBox19DisplayCapabilities(pDisplayCap);
            break;
        case 1000:
            BBOX_P_Vdc_SetBox1000DisplayCapabilities(pDisplayCap);
            break;
        case 1001:
            BBOX_P_Vdc_SetBox1001DisplayCapabilities(pDisplayCap);
            break;
    }
}

void BBOX_P_Vdc_SetDeinterlacerCapabilities
    ( uint32_t                            ulBoxId,
      BBOX_Vdc_Deinterlacer_Capabilities *pDeinterlacerCap )
{

    switch (ulBoxId)
    {
        case 1:
        case 12:
            BBOX_P_Vdc_SetBox1DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 2:
            BBOX_P_Vdc_SetBox2DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 3:
            BBOX_P_Vdc_SetBox3DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 4:
            BBOX_P_Vdc_SetBox4DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 5:
            BBOX_P_Vdc_SetBox5DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 6:
            BBOX_P_Vdc_SetBox6DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 7:
            BBOX_P_Vdc_SetBox7DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 8:
            BBOX_P_Vdc_SetBox8DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 10:
            BBOX_P_Vdc_SetBox10DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 13:
            BBOX_P_Vdc_SetBox13DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 14:
            BBOX_P_Vdc_SetBox14DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 15:
            BBOX_P_Vdc_SetBox15DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 16:
            BBOX_P_Vdc_SetBox16DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 17:
            BBOX_P_Vdc_SetBox17DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 18:
            BBOX_P_Vdc_SetBox18DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 19:
            BBOX_P_Vdc_SetBox19DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 1000:
            BBOX_P_Vdc_SetBox1000DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 1001:
            BBOX_P_Vdc_SetBox1001DeinterlacerCapabilities(pDeinterlacerCap);
            break;
    }
}

void BBOX_P_Vdc_SetXcodeCapabilities
    ( uint32_t                     ulBoxId,
      BBOX_Vdc_Xcode_Capabilities *pXcodeCap )
{
    switch (ulBoxId)
    {
        case 1:
        case 12:
            BBOX_P_Vdc_SetBox1XcodeCapabilities(pXcodeCap);
            break;
        case 2:
            BBOX_P_Vdc_SetBox2XcodeCapabilities(pXcodeCap);
            break;
        case 3:
            BBOX_P_Vdc_SetBox3XcodeCapabilities(pXcodeCap);
            break;
        case 4:
            BBOX_P_Vdc_SetBox4XcodeCapabilities(pXcodeCap);
            break;
        case 5:
            BBOX_P_Vdc_SetBox5XcodeCapabilities(pXcodeCap);
            break;
        case 6:
            BBOX_P_Vdc_SetBox6XcodeCapabilities(pXcodeCap);
            break;
        case 7:
            BBOX_P_Vdc_SetBox7XcodeCapabilities(pXcodeCap);
            break;
        case 8:
            BBOX_P_Vdc_SetBox8XcodeCapabilities(pXcodeCap);
            break;
        case 10:
            BBOX_P_Vdc_SetBox10XcodeCapabilities(pXcodeCap);
            break;
        case 13:
            BBOX_P_Vdc_SetBox13XcodeCapabilities(pXcodeCap);
            break;
        case 14:
        case 18:
            BBOX_P_Vdc_SetBox14XcodeCapabilities(pXcodeCap);
            break;
        case 15:
            BBOX_P_Vdc_SetBox15XcodeCapabilities(pXcodeCap);
            break;
        case 16:
            BBOX_P_Vdc_SetBox16XcodeCapabilities(pXcodeCap);
            break;
        case 17:
            BBOX_P_Vdc_SetBox17XcodeCapabilities(pXcodeCap);
            break;
        case 19:
            BBOX_P_Vdc_SetBox19XcodeCapabilities(pXcodeCap);
            break;
        case 1000:
            BBOX_P_Vdc_SetBox1000XcodeCapabilities(pXcodeCap);
            break;
        case 1001:
            BBOX_P_Vdc_SetBox1001XcodeCapabilities(pXcodeCap);
            break;
    }
}

BERR_Code BBOX_P_SetMemConfig
    ( uint32_t               ulBoxId,
      BBOX_MemConfig        *pBoxMemConfig )
{
    BERR_Code eStatus = BERR_SUCCESS;

    /* Set default config settings  */
    BBOX_P_SetDefaultMemConfig(pBoxMemConfig);

    switch (ulBoxId)
    {
        case 1:
            BBOX_P_SetBox1MemConfig(pBoxMemConfig);
            break;
        case 2:
            BBOX_P_SetBox2MemConfig(pBoxMemConfig);
            break;
        case 3:
            BBOX_P_SetBox3MemConfig(pBoxMemConfig);
            break;
        case 4:
            BBOX_P_SetBox4MemConfig(pBoxMemConfig);
            break;
        case 5:
            BBOX_P_SetBox5MemConfig(pBoxMemConfig);
            break;
        case 6:
            BBOX_P_SetBox6MemConfig(pBoxMemConfig);
            break;
        case 7:
            BBOX_P_SetBox7MemConfig(pBoxMemConfig);
            break;
        case 8:
            BBOX_P_SetBox8MemConfig(pBoxMemConfig);
            break;
        case 10:
            BBOX_P_SetBox10MemConfig(pBoxMemConfig);
            break;
        case 12:
            BBOX_P_SetBox12MemConfig(pBoxMemConfig);
            break;
        case 13:
            BBOX_P_SetBox13MemConfig(pBoxMemConfig);
            break;
        case 14:
            BBOX_P_SetBox14MemConfig(pBoxMemConfig);
            break;
        case 15:
            BBOX_P_SetBox15MemConfig(pBoxMemConfig);
            break;
            break;
        case 16:
            BBOX_P_SetBox16MemConfig(pBoxMemConfig);
            break;
        case 17:
            BBOX_P_SetBox17MemConfig(pBoxMemConfig);
            break;
        case 18:
            BBOX_P_SetBox18MemConfig(pBoxMemConfig);
            break;
        case 19:
            BBOX_P_SetBox19MemConfig(pBoxMemConfig);
            break;
        case 1000:
            BBOX_P_SetBox1000MemConfig(pBoxMemConfig);
            break;
        case 1001:
            BBOX_P_SetBox1001MemConfig(pBoxMemConfig);
            break;
        default:
            BDBG_ERR(("There is no box mode %d MEMC configuration.", ulBoxId));
            eStatus = BBOX_MEM_CFG_UNINITIALIZED;
    }
    return eStatus;
}

const struct BBOX_InterfaceMap g_BBOX_InterfaceMap[] = {
    {1, BBOX_P_GetBox1Rts},
    {2, BBOX_P_GetBox2Rts},
    {3, BBOX_P_GetBox3Rts},
    {4, BBOX_P_GetBox4Rts},
    {5, BBOX_P_GetBox5Rts},
    {6, BBOX_P_GetBox6Rts},
    {7, BBOX_P_GetBox7Rts},
    {8, BBOX_P_GetBox8Rts},
    {10, BBOX_P_GetBox10Rts},
    {12, BBOX_P_GetBox12Rts},
    {13, BBOX_P_GetBox13Rts},
    {14, BBOX_P_GetBox14Rts},
    {15, BBOX_P_GetBox15Rts},
    {16, BBOX_P_GetBox16Rts},
    {17, BBOX_P_GetBox17Rts},
    {18, BBOX_P_GetBox18Rts},
    {19, BBOX_P_GetBox19Rts},
    {1000, BBOX_P_GetBox1000Rts},
    {1001, BBOX_P_GetBox1001Rts},
    {0, NULL}};

/* end of file */

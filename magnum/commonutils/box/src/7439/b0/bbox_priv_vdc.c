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
******************************************************************************/

#include "bstd.h"                /* standard types */
#include "bkni.h"
#include "bdbg.h"                /* Debug message */
#include "bbox.h"
#include "bfmt.h"
#include "bbox_priv.h"
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
#include "bbox_vdc_box9_config.h"
#include "bbox_vdc_box10_config.h"
#include "bbox_vdc_box12_config.h"
#include "bbox_vdc_box13_config.h"
#include "bbox_vdc_box14_config.h"
#include "bbox_vdc_box16_config.h"
#include "bbox_vdc_box17_config.h"
#include "bbox_vdc_box18_config.h"
#include "bbox_vdc_box19_config.h"
#include "bbox_vdc_box20_config.h"
#include "bbox_vdc_box21_config.h"
#include "bbox_vdc_box22_config.h"
#include "bbox_vdc_box23_config.h"
#include "bbox_vdc_box24_config.h"
#include "bbox_vdc_box25_config.h"
#include "bbox_vdc_box26_config.h"
#include "bbox_vdc_box27_config.h"
#include "bbox_vdc_box28_config.h"
#include "bbox_vdc_box29_config.h"
#include "bbox_vdc_box30_config.h"


BDBG_MODULE(BBOX_PRIV);
BDBG_OBJECT_ID(BBOX_BOX_PRIV);

BERR_Code BBOX_P_ValidateId
    (uint32_t                ulId)
{
    BERR_Code eStatus = BERR_SUCCESS;
    if (ulId == 0 || ulId == 8 || ulId == 11 ||
        ulId == 15 || ulId > BBOX_MODES_SUPPORTED)
    {
        eStatus = BBOX_ID_NOT_SUPPORTED;
    }
    return eStatus;
}

void BBOX_P_Vdc_SetSourceCapabilities
    ( uint32_t                      ulBoxId,
      BBOX_Vdc_Source_Capabilities *pSourceCap )
{
    switch (ulBoxId)
    {
        case 1:
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
        case 9:
            BBOX_P_Vdc_SetBox9SourceCapabilities(pSourceCap);
            break;
        case 10:
            BBOX_P_Vdc_SetBox10SourceCapabilities(pSourceCap);
            break;
        case 12:
            BBOX_P_Vdc_SetBox12SourceCapabilities(pSourceCap);
            break;
        case 13:
            BBOX_P_Vdc_SetBox13SourceCapabilities(pSourceCap);
            break;
        case 14:
            BBOX_P_Vdc_SetBox14SourceCapabilities(pSourceCap);
            break;
        case 16:
            BBOX_P_Vdc_SetBox16SourceCapabilities(pSourceCap);
            break;
        case 17:
            BBOX_P_Vdc_SetBox17SourceCapabilities(pSourceCap);
            break;
        case 18:
            BBOX_P_Vdc_SetBox18SourceCapabilities(pSourceCap);
            break;
        case 19:
            BBOX_P_Vdc_SetBox19SourceCapabilities(pSourceCap);
            break;
        case 20:
            BBOX_P_Vdc_SetBox20SourceCapabilities(pSourceCap);
            break;
        case 21:
            BBOX_P_Vdc_SetBox21SourceCapabilities(pSourceCap);
            break;
        case 22:
            BBOX_P_Vdc_SetBox22SourceCapabilities(pSourceCap);
            break;
        case 23:
        case 29:
            BBOX_P_Vdc_SetBox23SourceCapabilities(pSourceCap);
            break;
        case 24:
            BBOX_P_Vdc_SetBox24SourceCapabilities(pSourceCap);
            break;
        case 25:
            BBOX_P_Vdc_SetBox25SourceCapabilities(pSourceCap);
            break;
        case 26:
            BBOX_P_Vdc_SetBox20SourceCapabilities(pSourceCap);
            BBOX_P_Vdc_SetBox26SourceCapabilities(pSourceCap);
            break;
        case 27:
            BBOX_P_Vdc_SetBox27SourceCapabilities(pSourceCap);
            break;
        case 28:
            BBOX_P_Vdc_SetBox28SourceCapabilities(pSourceCap);
            break;
        case 30:
            BBOX_P_Vdc_SetBox30SourceCapabilities(pSourceCap);
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
        case 9:
            BBOX_P_Vdc_SetBox9DisplayCapabilities(pDisplayCap);
            break;
        case 10:
            BBOX_P_Vdc_SetBox10DisplayCapabilities(pDisplayCap);
            break;
        case 12:
            BBOX_P_Vdc_SetBox12DisplayCapabilities(pDisplayCap);
            break;
        case 13:
            BBOX_P_Vdc_SetBox13DisplayCapabilities(pDisplayCap);
            break;
        case 14:
            BBOX_P_Vdc_SetBox14DisplayCapabilities(pDisplayCap);
            break;
        case 16:
            BBOX_P_Vdc_SetBox16DisplayCapabilities(pDisplayCap);
            break;
        case 17:
            BBOX_P_Vdc_SetBox17DisplayCapabilities(pDisplayCap);
            break;
        case 18:
            BBOX_P_Vdc_SetBox18DisplayCapabilities(pDisplayCap);
            break;
        case 19:
            BBOX_P_Vdc_SetBox19DisplayCapabilities(pDisplayCap);
            break;
        case 20:
            BBOX_P_Vdc_SetBox20DisplayCapabilities(pDisplayCap);
            break;
        case 21:
            BBOX_P_Vdc_SetBox21DisplayCapabilities(pDisplayCap);
            break;
        case 22:
            BBOX_P_Vdc_SetBox22DisplayCapabilities(pDisplayCap);
            break;
        case 23:
        case 29:
            BBOX_P_Vdc_SetBox23DisplayCapabilities(pDisplayCap);
            break;
         case 24:
            BBOX_P_Vdc_SetBox24DisplayCapabilities(pDisplayCap);
            break;
         case 25:
            BBOX_P_Vdc_SetBox25DisplayCapabilities(pDisplayCap);
            break;
         case 26:
            BBOX_P_Vdc_SetBox26DisplayCapabilities(pDisplayCap);
            break;
         case 27:
            BBOX_P_Vdc_SetBox27DisplayCapabilities(pDisplayCap);
            break;
         case 28:
            BBOX_P_Vdc_SetBox28DisplayCapabilities(pDisplayCap);
            break;
         case 30:
            BBOX_P_Vdc_SetBox30DisplayCapabilities(pDisplayCap);
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
        case 9:
            BBOX_P_Vdc_SetBox9DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 10:
            BBOX_P_Vdc_SetBox10DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 12:
            BBOX_P_Vdc_SetBox12DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 13:
            BBOX_P_Vdc_SetBox13DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 14:
            BBOX_P_Vdc_SetBox14DeinterlacerCapabilities(pDeinterlacerCap);
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
        case 20:
            BBOX_P_Vdc_SetBox20DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 21:
            BBOX_P_Vdc_SetBox21DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 22:
            BBOX_P_Vdc_SetBox22DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 23:
        case 29:
            BBOX_P_Vdc_SetBox23DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 24:
            BBOX_P_Vdc_SetBox24DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 25:
            BBOX_P_Vdc_SetBox25DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 26:
            BBOX_P_Vdc_SetBox26DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 27:
            BBOX_P_Vdc_SetBox27DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 28:
            BBOX_P_Vdc_SetBox28DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 30:
            BBOX_P_Vdc_SetBox30DeinterlacerCapabilities(pDeinterlacerCap);
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
        case 9:
            BBOX_P_Vdc_SetBox9XcodeCapabilities(pXcodeCap);
            break;
        case 10:
            BBOX_P_Vdc_SetBox10XcodeCapabilities(pXcodeCap);
            break;
        case 12:
            BBOX_P_Vdc_SetBox12XcodeCapabilities(pXcodeCap);
            break;
        case 13:
            BBOX_P_Vdc_SetBox13XcodeCapabilities(pXcodeCap);
            break;
        case 14:
            BBOX_P_Vdc_SetBox14XcodeCapabilities(pXcodeCap);
            break;
        case 16:
            BBOX_P_Vdc_SetBox16XcodeCapabilities(pXcodeCap);
            break;
        case 17:
            BBOX_P_Vdc_SetBox17XcodeCapabilities(pXcodeCap);
            break;
        case 18:
            BBOX_P_Vdc_SetBox18XcodeCapabilities(pXcodeCap);
            break;
        case 19:
            BBOX_P_Vdc_SetBox19XcodeCapabilities(pXcodeCap);
            break;
        case 20:
            BBOX_P_Vdc_SetBox20XcodeCapabilities(pXcodeCap);
            break;
        case 21:
            BBOX_P_Vdc_SetBox21XcodeCapabilities(pXcodeCap);
            break;
        case 22:
            BBOX_P_Vdc_SetBox22XcodeCapabilities(pXcodeCap);
            break;
        case 23:
        case 29:
            BBOX_P_Vdc_SetBox23XcodeCapabilities(pXcodeCap);
            break;
        case 24:
            BBOX_P_Vdc_SetBox24XcodeCapabilities(pXcodeCap);
            break;
        case 25:
            BBOX_P_Vdc_SetBox25XcodeCapabilities(pXcodeCap);
            break;
        case 26:
            BBOX_P_Vdc_SetBox26XcodeCapabilities(pXcodeCap);
            break;
        case 27:
            BBOX_P_Vdc_SetBox27XcodeCapabilities(pXcodeCap);
            break;
        case 28:
            BBOX_P_Vdc_SetBox28XcodeCapabilities(pXcodeCap);
            break;
        case 30:
            BBOX_P_Vdc_SetBox30XcodeCapabilities(pXcodeCap);
            break;
    }
}

BERR_Code BBOX_P_GetMemConfig
    ( uint32_t               ulBoxId,
      BBOX_MemConfig        *pBoxMemConfig )
{
    BERR_Code eStatus = BERR_SUCCESS;

    eStatus = BBOX_P_ValidateId(ulBoxId);
    if (eStatus != BERR_SUCCESS) return eStatus;

    switch (ulBoxId)
    {
        case 1:
            BBOX_P_GetBox1MemConfig(pBoxMemConfig);
            break;
        case 2:
            BBOX_P_GetBox2MemConfig(pBoxMemConfig);
            break;
        case 3:
            BBOX_P_GetBox3MemConfig(pBoxMemConfig);
            break;
        case 4:
            BBOX_P_GetBox4MemConfig(pBoxMemConfig);
            break;
        case 5:
            BBOX_P_GetBox5MemConfig(pBoxMemConfig);
            break;
        case 6:
            BBOX_P_GetBox6MemConfig(pBoxMemConfig);
            break;
        case 7:
            BBOX_P_GetBox7MemConfig(pBoxMemConfig);
            break;
        case 9:
            BBOX_P_GetBox9MemConfig(pBoxMemConfig);
            break;
        case 10:
            BBOX_P_GetBox10MemConfig(pBoxMemConfig);
            break;
        case 12:
            BBOX_P_GetBox12MemConfig(pBoxMemConfig);
            break;
        case 13:
            BBOX_P_GetBox13MemConfig(pBoxMemConfig);
            break;
        case 14:
            BBOX_P_GetBox14MemConfig(pBoxMemConfig);
            break;
        case 16:
            BBOX_P_GetBox16MemConfig(pBoxMemConfig);
            break;
        case 17:
            BBOX_P_GetBox17MemConfig(pBoxMemConfig);
            break;
        case 18:
            BBOX_P_GetBox18MemConfig(pBoxMemConfig);
            break;
        case 19:
            BBOX_P_GetBox19MemConfig(pBoxMemConfig);
            break;
        case 20:
            BBOX_P_GetBox20MemConfig(pBoxMemConfig);
            break;
        case 21:
            BBOX_P_GetBox21MemConfig(pBoxMemConfig);
            break;
        case 22:
            BBOX_P_GetBox22MemConfig(pBoxMemConfig);
            break;
        case 23:
        case 29:
            BBOX_P_GetBox23MemConfig(pBoxMemConfig);
            break;
        case 24:
            BBOX_P_GetBox24MemConfig(pBoxMemConfig);
            break;
        case 25:
            BBOX_P_GetBox25MemConfig(pBoxMemConfig);
            break;
        case 26:
            BBOX_P_GetBox26MemConfig(pBoxMemConfig);
            break;
        case 27:
            BBOX_P_GetBox27MemConfig(pBoxMemConfig);
            break;
        case 28:
            BBOX_P_GetBox28MemConfig(pBoxMemConfig);
            break;
        case 30:
            BBOX_P_GetBox30MemConfig(pBoxMemConfig);
            break;
    }
    return BERR_SUCCESS;
}

BERR_Code BBOX_P_GetRtsConfig
    ( const uint32_t         ulBoxId,
      BBOX_Rts              *pBoxRts )
{
    switch (ulBoxId)
    {
        case 1:
            BBOX_P_GetBox1Rts(pBoxRts);
            break;
        case 2:
            BBOX_P_GetBox2Rts(pBoxRts);
            break;
        case 3:
            BBOX_P_GetBox3Rts(pBoxRts);
            break;
        case 4:
            BBOX_P_GetBox4Rts(pBoxRts);
            break;
        case 5:
            BBOX_P_GetBox5Rts(pBoxRts);
            break;
        case 6:
            BBOX_P_GetBox6Rts(pBoxRts);
            break;
        case 7:
            BBOX_P_GetBox7Rts(pBoxRts);
            break;
        case 9:
            BBOX_P_GetBox9Rts(pBoxRts);
            break;
        case 10:
            BBOX_P_GetBox10Rts(pBoxRts);
            break;
        case 12:
            BBOX_P_GetBox12Rts(pBoxRts);
            break;
        case 13:
            BBOX_P_GetBox13Rts(pBoxRts);
            break;
        case 14:
            BBOX_P_GetBox14Rts(pBoxRts);
            break;
        case 16:
            BBOX_P_GetBox16Rts(pBoxRts);
            break;
        case 17:
            BBOX_P_GetBox17Rts(pBoxRts);
            break;
        case 18:
            BBOX_P_GetBox18Rts(pBoxRts);
            break;
        case 19:
            BBOX_P_GetBox19Rts(pBoxRts);
            break;
        case 20:
            BBOX_P_GetBox20Rts(pBoxRts);
            break;
        case 21:
            BBOX_P_GetBox21Rts(pBoxRts);
            break;
        case 22:
            BBOX_P_GetBox22Rts(pBoxRts);
            break;
        case 23:
            BBOX_P_GetBox23Rts(pBoxRts);
            break;
        case 24:
            BBOX_P_GetBox24Rts(pBoxRts);
            break;
        case 25:
            BBOX_P_GetBox25Rts(pBoxRts);
            break;
        case 26:
            BBOX_P_GetBox26Rts(pBoxRts);
            break;
        case 27:
            BBOX_P_GetBox27Rts(pBoxRts);
            break;
        case 28:
            BBOX_P_GetBox28Rts(pBoxRts);
            break;
        case 29:
            BBOX_P_GetBox29Rts(pBoxRts);
            break;
        case 30:
            BBOX_P_GetBox30Rts(pBoxRts);
            break;
    }
    return BERR_SUCCESS;
}
/* end of file */

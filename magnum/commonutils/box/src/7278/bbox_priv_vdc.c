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
#include "bbox_vdc_box1000_config.h"
#include "bbox_vdc_box1001_config.h"


BDBG_MODULE(BBOX_PRIV);
BDBG_OBJECT_ID(BBOX_BOX_PRIV);

BERR_Code BBOX_P_ValidateId
    (uint32_t                ulId)
{
    BERR_Code eStatus = BERR_SUCCESS;
    if ((ulId != 1000 && ulId != 1001) && (ulId == 0 || ulId > BBOX_MODES_SUPPORTED))
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
            BBOX_P_Vdc_SetBox1DisplayCapabilities(pDisplayCap);
            break;
        case 2:
            BBOX_P_Vdc_SetBox2DisplayCapabilities(pDisplayCap);
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
            BBOX_P_Vdc_SetBox1DeinterlacerCapabilities(pDeinterlacerCap);
            break;
        case 2:
            BBOX_P_Vdc_SetBox2DeinterlacerCapabilities(pDeinterlacerCap);
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
            BBOX_P_Vdc_SetBox1XcodeCapabilities(pXcodeCap);
            break;
        case 2:
            BBOX_P_Vdc_SetBox2XcodeCapabilities(pXcodeCap);
            break;
        case 1000:
            BBOX_P_Vdc_SetBox1000XcodeCapabilities(pXcodeCap);
            break;
        case 1001:
            BBOX_P_Vdc_SetBox1001XcodeCapabilities(pXcodeCap);
            break;
    }
}


BERR_Code BBOX_P_GetMemConfig
    ( uint32_t                       ulBoxId,
      BBOX_MemConfig                *pBoxMemConfig )
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
        case 1000:
            BBOX_P_GetBox1000MemConfig(pBoxMemConfig);
            break;
        case 1001:
            BBOX_P_GetBox1001MemConfig(pBoxMemConfig);
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
        case 1000:
            BBOX_P_GetBox1000Rts(pBoxRts);
            break;
        case 1001:
            BBOX_P_GetBox1001Rts(pBoxRts);
            break;
    }
    return BERR_SUCCESS;
}
/* end of file */

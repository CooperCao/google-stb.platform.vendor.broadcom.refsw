/******************************************************************************
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
 *****************************************************************************/

 #ifndef BBOX_PRIV_H__
#define BBOX_PRIV_H__


#include "bbox.h"
#include "bbox_vdc.h"

/***************************************************************************
 * BBOX_P_Handle
 ***************************************************************************/
typedef struct BBOX_P_Context
{
    BDBG_OBJECT(BBOX_BOX)

    BBOX_Config              stBoxConfig;
    bool                     bRtsLoaded;
} BBOX_P_Context;

BERR_Code BBOX_P_ValidateId
    (uint32_t                ulId);

BERR_Code BBOX_P_Vdc_SetBoxMode
    ( uint32_t               ulBoxId,
      BBOX_Vdc_Capabilities *pBoxVdc );

BERR_Code BBOX_P_Vdc_SelfCheck
    ( const BBOX_MemConfig          *pMemConfig,
      const BBOX_Vdc_Capabilities   *pVdcCap );

BERR_Code BBOX_P_Vdc_ValidateBoxModes
    ( BBOX_Handle                 hBox );

BERR_Code BBOX_P_Vce_SetBoxMode
   ( uint32_t               ulBoxId,
     BBOX_Vce_Capabilities *pBoxVce );

BERR_Code BBOX_P_Audio_SetBoxMode
   ( uint32_t                 ulBoxId,
     BBOX_Audio_Capabilities *pBoxAudio );

BERR_Code BBOX_P_Xvd_SetBoxMode
   ( uint32_t               ulBoxId,
     BBOX_Xvd_Config *pBoxXvd );

BERR_Code BBOX_P_LoadRts
    ( const BREG_Handle          hReg,
      const uint32_t             ulBoxId,
      const BBOX_DramRefreshRate eRefreshRate );

BERR_Code BBOX_P_GetMemConfig
    ( uint32_t               ulBoxId,
      BBOX_MemConfig        *pBoxMemConfig );

/* Add module specific box mode functions here */

#endif /* BBOX_PRIV_H__ */
/* end of file */

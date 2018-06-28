/***************************************************************************
*  Copyright (C) 2018 Broadcom.
*  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to
*  the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied),
*  right to use, or waiver of any kind with respect to the Software, and
*  Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
*  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
*  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization,
*  constitutes the valuable trade secrets of Broadcom, and you shall use all
*  reasonable efforts to protect the confidentiality thereof, and to use this
*  information only in connection with your use of Broadcom integrated circuit
*  products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
*  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
*  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
*  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
*  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
*  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
*  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
*  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
*  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
*  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
*
* API Description:
*   API name: Frontend 3466
*    APIs to open, close, and setup initial settings for a BCM3466
*    Terrestrial Tuner/Demodulator Device.
*
***************************************************************************/
/* Begin Includes */
#include "nexus_frontend_module.h"
#include "nexus_frontend_qam_helper_priv.h"
/* End includes */

BDBG_MODULE(nexus_frontend_qam_helper_priv);


void NEXUS_Frontend_P_QamToModulationType(BADS_ModulationType modType, NEXUS_FrontendQamAnnex *pAnnex, NEXUS_FrontendQamMode *pMode)
{
    switch ( modType )
    {
    case BADS_ModulationType_eAnnexAQam16:
        *pAnnex = NEXUS_FrontendQamAnnex_eA; *pMode = NEXUS_FrontendQamMode_e16;
        break;
    case BADS_ModulationType_eAnnexAQam32:
        *pAnnex = NEXUS_FrontendQamAnnex_eA; *pMode = NEXUS_FrontendQamMode_e32;
        break;
    case BADS_ModulationType_eAnnexAQam64:
        *pAnnex = NEXUS_FrontendQamAnnex_eA; *pMode = NEXUS_FrontendQamMode_e64;
        break;
    case BADS_ModulationType_eAnnexAQam128:
        *pAnnex = NEXUS_FrontendQamAnnex_eA; *pMode = NEXUS_FrontendQamMode_e128;
        break;
    case BADS_ModulationType_eAnnexAQam256:
        *pAnnex = NEXUS_FrontendQamAnnex_eA; *pMode = NEXUS_FrontendQamMode_e256;
        break;
    case BADS_ModulationType_eAnnexAQam512:
        *pAnnex = NEXUS_FrontendQamAnnex_eA; *pMode = NEXUS_FrontendQamMode_e512;
        break;
    case BADS_ModulationType_eAnnexAQam1024:
        *pAnnex = NEXUS_FrontendQamAnnex_eA; *pMode = NEXUS_FrontendQamMode_e1024;
        break;
    case BADS_ModulationType_eAnnexAQam2048:
        *pAnnex = NEXUS_FrontendQamAnnex_eA; *pMode = NEXUS_FrontendQamMode_e2048;
        break;
    case BADS_ModulationType_eAnnexAQam4096:
        *pAnnex = NEXUS_FrontendQamAnnex_eA; *pMode = NEXUS_FrontendQamMode_e4096;
        break;

    case BADS_ModulationType_eAnnexBQam16:
        *pAnnex = NEXUS_FrontendQamAnnex_eB; *pMode = NEXUS_FrontendQamMode_e16;
        break;
    case BADS_ModulationType_eAnnexBQam32:
        *pAnnex = NEXUS_FrontendQamAnnex_eB; *pMode = NEXUS_FrontendQamMode_e32;
        break;
    case BADS_ModulationType_eAnnexBQam64:
        *pAnnex = NEXUS_FrontendQamAnnex_eB; *pMode = NEXUS_FrontendQamMode_e64;
        break;
    case BADS_ModulationType_eAnnexBQam128:
        *pAnnex = NEXUS_FrontendQamAnnex_eB; *pMode = NEXUS_FrontendQamMode_e128;
        break;
    case BADS_ModulationType_eAnnexBQam256:
        *pAnnex = NEXUS_FrontendQamAnnex_eB; *pMode = NEXUS_FrontendQamMode_e256;
        break;
    case BADS_ModulationType_eAnnexBQam512:
        *pAnnex = NEXUS_FrontendQamAnnex_eB; *pMode = NEXUS_FrontendQamMode_e512;
        break;
    case BADS_ModulationType_eAnnexBQam1024:
        *pAnnex = NEXUS_FrontendQamAnnex_eB; *pMode = NEXUS_FrontendQamMode_e1024;
        break;
    case BADS_ModulationType_eAnnexBQam2048:
        *pAnnex = NEXUS_FrontendQamAnnex_eB; *pMode = NEXUS_FrontendQamMode_e2048;
        break;
    case BADS_ModulationType_eAnnexBQam4096:
        *pAnnex = NEXUS_FrontendQamAnnex_eB; *pMode = NEXUS_FrontendQamMode_e4096;
        break;

    case BADS_ModulationType_eAnnexCQam16:
        *pAnnex = NEXUS_FrontendQamAnnex_eC; *pMode = NEXUS_FrontendQamMode_e16;
        break;
    case BADS_ModulationType_eAnnexCQam32:
        *pAnnex = NEXUS_FrontendQamAnnex_eC; *pMode = NEXUS_FrontendQamMode_e32;
        break;
    case BADS_ModulationType_eAnnexCQam64:
        *pAnnex = NEXUS_FrontendQamAnnex_eC; *pMode = NEXUS_FrontendQamMode_e64;
        break;
    case BADS_ModulationType_eAnnexCQam128:
        *pAnnex = NEXUS_FrontendQamAnnex_eC; *pMode = NEXUS_FrontendQamMode_e128;
        break;
    case BADS_ModulationType_eAnnexCQam256:
        *pAnnex = NEXUS_FrontendQamAnnex_eC; *pMode = NEXUS_FrontendQamMode_e256;
        break;
    case BADS_ModulationType_eAnnexCQam512:
        *pAnnex = NEXUS_FrontendQamAnnex_eC; *pMode = NEXUS_FrontendQamMode_e512;
        break;
    case BADS_ModulationType_eAnnexCQam1024:
        *pAnnex = NEXUS_FrontendQamAnnex_eC; *pMode = NEXUS_FrontendQamMode_e1024;
        break;
    case BADS_ModulationType_eAnnexCQam2048:
        *pAnnex = NEXUS_FrontendQamAnnex_eC; *pMode = NEXUS_FrontendQamMode_e2048;
        break;
    case BADS_ModulationType_eAnnexCQam4096:
        *pAnnex = NEXUS_FrontendQamAnnex_eC; *pMode = NEXUS_FrontendQamMode_e4096;
        break;

    default:
        BDBG_WRN(("Unrecognized QAM Modultation Type: value=%d", modType));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        *pAnnex = NEXUS_FrontendQamAnnex_eA; *pMode = NEXUS_FrontendQamMode_e256;
        return;
    }
}

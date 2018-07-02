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
#include "nexus_frontend_ofdm_helper_priv.h"
/* End includes */

BDBG_MODULE(nexus_frontend_ofdm_helper_priv);

NEXUS_FrontendOfdmTransmissionMode NEXUS_Frontend_P_DvbtToTransmissionMode(BODS_DvbtTransmissionMode mode)
{
    switch ( mode )
    {
    case BODS_DvbtTransmissionMode_e2K:
        return NEXUS_FrontendOfdmTransmissionMode_e2k;
    case BODS_DvbtTransmissionMode_e4K:
        return NEXUS_FrontendOfdmTransmissionMode_e4k;
    case BODS_DvbtTransmissionMode_e8K:
        return NEXUS_FrontendOfdmTransmissionMode_e8k;
    default:
        BDBG_WRN(("Unrecognized transmission mode."));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return NEXUS_FrontendOfdmTransmissionMode_e8k;
    }
}

NEXUS_FrontendOfdmModulation NEXUS_Frontend_P_DvbtToModulation(BODS_DvbtModulation modulation)
{
    switch ( modulation )
    {
    case BODS_DvbtModulation_eQpsk:
        return NEXUS_FrontendOfdmModulation_eQpsk;
    case BODS_DvbtModulation_e16Qam:
        return NEXUS_FrontendOfdmModulation_eQam16;
    case BODS_DvbtModulation_e64Qam:
        return NEXUS_FrontendOfdmModulation_eQam64;
    default:
        BDBG_WRN(("Unrecognized modulation mode (%d) reported by BODS", modulation));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return NEXUS_FrontendOfdmModulation_eQam64;
    }
}

NEXUS_FrontendOfdmCodeRate NEXUS_Frontend_P_DvbtToCodeRate(BODS_DvbtCodeRate codeRate)
{
    switch ( codeRate )
    {
    case BODS_DvbtCodeRate_e1_2:
        return NEXUS_FrontendOfdmCodeRate_e1_2;
    case BODS_DvbtCodeRate_e2_3:
        return NEXUS_FrontendOfdmCodeRate_e2_3;
    case BODS_DvbtCodeRate_e3_4:
        return NEXUS_FrontendOfdmCodeRate_e3_4;
    case BODS_DvbtCodeRate_e5_6:
        return NEXUS_FrontendOfdmCodeRate_e5_6;
    case BODS_DvbtCodeRate_e7_8:
        return NEXUS_FrontendOfdmCodeRate_e7_8;
    default:
        BDBG_WRN(("Unrecognized codeRate (%d) reported by BODS", codeRate));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return NEXUS_FrontendOfdmCodeRate_e1_2;
    }
}

NEXUS_FrontendOfdmGuardInterval NEXUS_Frontend_P_DvbtToGuardInterval(BODS_DvbtGuardInterval guard)
{
    switch ( guard )
    {
    case BODS_DvbtGuardInterval_e1_4:
        return NEXUS_FrontendOfdmGuardInterval_e1_4;
    case BODS_DvbtGuardInterval_e1_8:
        return NEXUS_FrontendOfdmGuardInterval_e1_8;
    case BODS_DvbtGuardInterval_e1_16:
        return NEXUS_FrontendOfdmGuardInterval_e1_16;
    case BODS_DvbtGuardInterval_e1_32:
        return NEXUS_FrontendOfdmGuardInterval_e1_32;
    default:
        BDBG_WRN(("Unrecognized guard interval (%d) reported by BODS", guard));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return NEXUS_FrontendOfdmGuardInterval_e1_4;
    }
}

NEXUS_FrontendOfdmHierarchy NEXUS_Frontend_P_DvbtToHierarchy(BODS_DvbtHierarchy magnum)
{
    switch ( magnum )
    {
    case BODS_DvbtHierarchy_e0:
        return NEXUS_FrontendOfdmHierarchy_e0;
    case BODS_DvbtHierarchy_e1:
        return NEXUS_FrontendOfdmHierarchy_e1;
    case BODS_DvbtHierarchy_e2:
        return NEXUS_FrontendOfdmHierarchy_e2;
    case BODS_DvbtHierarchy_e4:
        return NEXUS_FrontendOfdmHierarchy_e4;
    default:
        BDBG_WRN(("Unrecognized hierarchy (%d) reported by BODS", magnum));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return NEXUS_FrontendOfdmHierarchy_e0;
    }
}

BODS_SelectiveAsyncStatusType NEXUS_Frontend_P_t2StatusTypeToDvbt2(NEXUS_FrontendDvbt2StatusType type)
{
    switch (type)
    {
    case NEXUS_FrontendDvbt2StatusType_eFecStatisticsL1Pre:
        return BODS_SelectiveAsyncStatusType_eDvbt2FecStatisticsL1Pre;
    case NEXUS_FrontendDvbt2StatusType_eFecStatisticsL1Post:
        return BODS_SelectiveAsyncStatusType_eDvbt2FecStatisticsL1Post;
    case NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpA:
        return BODS_SelectiveAsyncStatusType_eDvbt2FecStatisticsPlpA;
    case NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpB:
        return BODS_SelectiveAsyncStatusType_eDvbt2FecStatisticsPlpB;
    case NEXUS_FrontendDvbt2StatusType_eL1Pre:
        return BODS_SelectiveAsyncStatusType_eDvbt2L1Pre;
    case NEXUS_FrontendDvbt2StatusType_eL1PostConfigurable:
        return BODS_SelectiveAsyncStatusType_eDvbt2L1PostConfigurable;
    case NEXUS_FrontendDvbt2StatusType_eL1PostDynamic:
        return BODS_SelectiveAsyncStatusType_eDvbt2L1PostDynamic;
    case NEXUS_FrontendDvbt2StatusType_eL1Plp:
        return BODS_SelectiveAsyncStatusType_eDvbt2L1Plp;
    case NEXUS_FrontendDvbt2StatusType_eBasic:
        return BODS_SelectiveAsyncStatusType_eDvbt2Short;
    default:
        BDBG_WRN((" Unsupported status type. type=%d", type));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return BODS_SelectiveAsyncStatusType_eDvbt2Short;
    }
}


BODS_DvbtTransmissionMode NEXUS_Frontend_P_TransmissionModeToDvbt(NEXUS_FrontendOfdmTransmissionMode mode)
{
    switch ( mode )
    {
    case NEXUS_FrontendOfdmTransmissionMode_e2k:
        return BODS_DvbtTransmissionMode_e2K;
    case NEXUS_FrontendOfdmTransmissionMode_e4k:
        return BODS_DvbtTransmissionMode_e4K;
    case NEXUS_FrontendOfdmTransmissionMode_e8k:
        return BODS_DvbtTransmissionMode_e8K;
    default:
        BDBG_WRN(("Unrecognized Nexus transmission mode."));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return BODS_DvbtTransmissionMode_e8K;
    }
}

BODS_DvbtGuardInterval NEXUS_Frontend_P_GuardIntervalToDvbt(NEXUS_FrontendOfdmGuardInterval guard)
{
    switch ( guard )
    {
    case NEXUS_FrontendOfdmGuardInterval_e1_4:
        return BODS_DvbtGuardInterval_e1_4;
    case NEXUS_FrontendOfdmGuardInterval_e1_8:
        return BODS_DvbtGuardInterval_e1_8;
    case NEXUS_FrontendOfdmGuardInterval_e1_16:
        return BODS_DvbtGuardInterval_e1_16;
    case NEXUS_FrontendOfdmGuardInterval_e1_32:
        return BODS_DvbtGuardInterval_e1_32;
    default:
        BDBG_WRN(("Unrecognized guard interval (%d) reported by Nexus", guard));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return BODS_DvbtGuardInterval_e1_4;
    }
}

BODS_DvbtModulation NEXUS_Frontend_P_ModulationToDvbt(NEXUS_FrontendOfdmModulation modulation)
{
    switch ( modulation )
    {
    case NEXUS_FrontendOfdmModulation_eQpsk:
        return BODS_DvbtModulation_eQpsk;
    case NEXUS_FrontendOfdmModulation_eQam16:
        return BODS_DvbtModulation_e16Qam;
    case NEXUS_FrontendOfdmModulation_eQam64:
        return BODS_DvbtModulation_e64Qam;
    default:
        BDBG_WRN(("Unrecognized modulation (%d)", modulation));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return BODS_DvbtModulation_eQpsk;
    }
}

BODS_DvbtCodeRate NEXUS_Frontend_P_CodeRateToDvbt(NEXUS_FrontendOfdmCodeRate codeRate)
{
    switch ( codeRate )
    {
    case NEXUS_FrontendOfdmCodeRate_e1_2:
        return BODS_DvbtCodeRate_e1_2;
    case NEXUS_FrontendOfdmCodeRate_e2_3:
        return BODS_DvbtCodeRate_e2_3;
    case NEXUS_FrontendOfdmCodeRate_e3_4:
        return BODS_DvbtCodeRate_e3_4;
    case NEXUS_FrontendOfdmCodeRate_e5_6:
        return BODS_DvbtCodeRate_e5_6;
    case NEXUS_FrontendOfdmCodeRate_e7_8:
        return BODS_DvbtCodeRate_e7_8;
    default:
        BDBG_WRN(("Unrecognized code rate (%d)", codeRate));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return BODS_DvbtCodeRate_e1_2;
    }
}

BODS_DvbtHierarchy NEXUS_Frontend_P_HierarchyToDvbt(NEXUS_FrontendOfdmHierarchy hierarchy)
{
    switch ( hierarchy )
    {
    case NEXUS_FrontendOfdmHierarchy_e0:
        return BODS_DvbtHierarchy_e0;
    case NEXUS_FrontendOfdmHierarchy_e1:
        return BODS_DvbtHierarchy_e1;
    case NEXUS_FrontendOfdmHierarchy_e2:
        return BODS_DvbtHierarchy_e2;
    case NEXUS_FrontendOfdmHierarchy_e4:
        return BODS_DvbtHierarchy_e4;
    default:
        BDBG_WRN(("Unrecognized hierarchy (%d)", hierarchy));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return BODS_DvbtHierarchy_e0;
    }
}

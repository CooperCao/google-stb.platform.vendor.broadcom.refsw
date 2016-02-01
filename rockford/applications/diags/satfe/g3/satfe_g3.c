/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: satfe_g3.c $
 * $brcm_Revision: Hydra_Software_Devel/53 $
 * $brcm_Date: 9/30/13 2:51p $
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: /rockford/applications/diags/satfe/g3/satfe_g3.c $
 *
 * Hydra_Software_Devel/53   9/30/13 2:51p ronchan
 * SW7362-10: add satfe support for 7362
 *
 * Hydra_Software_Devel/52   5/1/13 3:47p enavarro
 * SWSATFE-264: updated misc_ctl config parameter
 *
 * Hydra_Software_Devel/51   10/22/12 9:25a ckpark
 * SWSATFE-221: added debug3 config parameter for debugging
 *
 * Hydra_Software_Devel/50   9/6/12 3:11p ronchan
 * SWSATFE-207: fixed typo in Desc_g3_TUNER_AGC_THRESH
 *
 * Hydra_Software_Devel/49   8/31/12 10:16a ronchan
 * SWSATFE-207: added tuner agc config parameters
 *
 * Hydra_Software_Devel/48   7/18/12 3:24p ronchan
 * SWSATFE-207: added daisy gain config param, renamed agc thresh config
 * param
 *
 * Hydra_Software_Devel/47   6/21/12 5:27p enavarro
 * SWSATFE-190: fix compiler errors for 7360
 *
 * Hydra_Software_Devel/46   4/10/12 3:13p ronchan
 * SWSATFE-88: changed tuner_ctl bit1 description
 *
 * Hydra_Software_Devel/45   3/22/12 2:06p enavarro
 * SWSATFE-184: added tuner_rfagc_thresh config parameter
 *
 * Hydra_Software_Devel/44   3/16/12 4:45p enavarro
 * SWSATFE-140: added acm_max_mode and reacq_cause config params
 *
 * Hydra_Software_Devel/43   1/24/12 9:55a enavarro
 * SWSATFE-140: updated SATFE_ChipFunctTable
 *
 * Hydra_Software_Devel/42   1/19/12 11:41a enavarro
 * SWSATFE-140: added Mi2cWrite chip function
 *
 * Hydra_Software_Devel/41   1/3/12 10:06a ckpark
 * SWSATFE-162: added debug1 and debug2 config params
 *
 * Hydra_Software_Devel/40   10/25/11 9:34a ronchan
 * SWSATFE-88: fixed FTM register names
 *
 * Hydra_Software_Devel/39   10/25/11 9:31a ronchan
 * SWSATFE-88: add missing FTM registers for B0
 *
 * Hydra_Software_Devel/38   10/10/11 11:03a enavarro
 * SWSATFE-150: added bast_g3_priv_cwc to SATFE_g3_DebugModules[]
 *
 * Hydra_Software_Devel/37   10/7/11 4:42p enavarro
 * SWSATFE-150: fixed compiler warning
 *
 * Hydra_Software_Devel/36   10/7/11 10:58a enavarro
 * SWSATFE-150: updated tuner_status command to use
 * BAST_g3_P_TunerGetActualLOFreq()
 *
 * Hydra_Software_Devel/35   10/7/11 10:36a enavarro
 * SWSATFE-150: updated tuner_status command
 *
 * Hydra_Software_Devel/34   10/5/11 2:12p ronchan
 * SWSATFE-88: account for A1 revision
 *
 * Hydra_Software_Devel/33   9/28/11 3:03p enavarro
 * SWSATFE-148: added tuner_status command
 *
 * Hydra_Software_Devel/32   9/20/11 9:59a enavarro
 * SWSATFE-86: fixed compiler error
 *
 * Hydra_Software_Devel/31   9/19/11 4:24p enavarro
 * SWSATFE-86: added debug code to time ISRs
 *
 * Hydra_Software_Devel/30   9/16/11 9:58a ronchan
 * SWSATFE-139: add config parameter for ambient temperature
 *
 * Hydra_Software_Devel/29   9/9/11 11:00a ronchan
 * SWSATFE-88: add cwc registers for B0
 *
 * Hydra_Software_Devel/28   9/8/11 4:03p ronchan
 * SWSATFE-88: fix build errors for B0
 *
 * Hydra_Software_Devel/27   8/24/11 3:44p ronchan
 * SWSATFE-88: fixed compile warnings
 *
 * Hydra_Software_Devel/26   7/6/11 6:36p mathew
 * SWSATFE-88: Update for adc voltage read function for g3
 *
 * Hydra_Software_Devel/25   4/5/11 10:56a ckpark
 * SWSATFE-102: normalized tuner calibration threshold is defined
 *
 * Hydra_Software_Devel/24   3/10/11 11:30a enavarro
 * SWSATFE-88: added debug_snore command
 *
 * Hydra_Software_Devel/23   2/2/11 3:12p ronchan
 * SWSATFE-88: updated tuner_ctl descriptions
 *
 * Hydra_Software_Devel/22   2/1/11 2:58p enavarro
 * SWSATFE-88: updated reacq_ctl config parameter description
 *
 * Hydra_Software_Devel/21   1/27/11 3:40p enavarro
 * SWSATFE-88: removed peak_scan_sym_rate_min/max config params; removed
 * dft_size config param
 *
 * Hydra_Software_Devel/20   1/24/11 11:50a enavarro
 * SWSATFE-75: added MISC_CTL config param
 *
 * Hydra_Software_Devel/19   1/11/11 10:52a enavarro
 * SWSATFE-75: add BYPASS_DFT_FREQ_EST bit in TUNER_CTL config param
 *
 * Hydra_Software_Devel/18   1/5/11 5:26p ronchan
 * SWSATFE-88: include stdlib.h for atof function
 *
 * Hydra_Software_Devel/17   1/5/11 5:18p ronchan
 * SWSATFE-88: adjusted upper bound of tuning frequency
 *
 * Hydra_Software_Devel/16   12/30/10 11:24a enavarro
 * SWSATFE-88: reworked setting debug level
 *
 * Hydra_Software_Devel/15   12/29/10 5:19p enavarro
 * SWSATFE-88: fixed output of trace command
 *
 * Hydra_Software_Devel/14   12/27/10 11:50a enavarro
 * SWSATFE-88: fixed trace command
 *
 * Hydra_Software_Devel/13   12/27/10 8:53a enavarro
 * SWSATFE-75: changed scaling of rain fade threshold config parameter
 *
 * Hydra_Software_Devel/12   12/15/10 11:12a enavarro
 * SWSATFE-88: added reacq_ctl config parameter
 *
 * Hydra_Software_Devel/11   12/12/10 11:31a enavarro
 * SWSATFE-88: access tuner indirect registers within critical section
 *
 * Hydra_Software_Devel/10   11/29/10 3:24p enavarro
 * SWSATFE-80: removed network_spec config parameter
 *
 * Hydra_Software_Devel/9   11/19/10 3:52p enavarro
 * SWSATFE-88: updated RDB
 *
 * Hydra_Software_Devel/8   10/15/10 8:39a enavarro
 * SWSATFE-75: fixed compiler warnings; removed old diseqc config
 * parameters
 *
 * Hydra_Software_Devel/7   10/11/10 4:35p enavarro
 * SWSATFE-75: added trace command
 *
 * Hydra_Software_Devel/6   10/4/10 3:15p enavarro
 * SWSATFE-75: added irq_count command
 *
 * Hydra_Software_Devel/5   9/29/10 3:52p enavarro
 * SWSATFE-80: removed xport_ctl config parameter
 *
 * Hydra_Software_Devel/4   9/28/10 9:40a enavarro
 * SWSATFE-75: added SATFE_g3_SetDebugLevel()
 *
 * Hydra_Software_Devel/3   9/27/10 5:29p enavarro
 * SWSATFE-75: fixed compiler warning
 *
 * Hydra_Software_Devel/2   9/16/10 10:32a enavarro
 * SWSATFE-75: added config param list and register map
 *
 * Hydra_Software_Devel/1   9/15/10 4:38p enavarro
 * SWSATFE-75: initial version
 *
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "satfe.h"
#include "satfe_platform.h"
#include "bast_g3.h"
#include "bast_g3_priv.h"
#include "mipsclock.h"


/* #define SATFE_TIME_ISR */


/* local functions */
bool SATFE_g3_GetFreqFromString(char *str, uint32_t *pHz);
bool SATFE_g3_IsSpinv(SATFE_Chip *pChip, BAST_ChannelStatus *pStatus);
bool SATFE_g3_IsPilotOn(SATFE_Chip *pChip, BAST_ChannelStatus *pStatus);
void SATFE_g3_GetDebugModuleNames(struct SATFE_Chip *pChip, int *numModules, char **moduleNames[]);
bool SATFE_g3_Command_help(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_g3_Command_read_tuner_ind(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_g3_Command_write_tuner_ind(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_g3_Command_irq_count(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_g3_Command_trace(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_g3_Command_debug_snore(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_g3_Command_tuner_status(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_g3_GetAdcVoltage(struct SATFE_Chip *pChip, uint8_t voltage, uint8_t currChannel, uint16_t *lnbVoltage);


/* define commands specific to this chip */
SATFE_Command SATFE_g3_CommandSet[] =
{
   {"_help", SATFE_g3_Command_help},
   {"read_tuner_ind", SATFE_g3_Command_read_tuner_ind},
   {"write_tuner_ind", SATFE_g3_Command_write_tuner_ind},
   {"irq_count", SATFE_g3_Command_irq_count},
   {"trace", SATFE_g3_Command_trace},
   {"debug_snore", SATFE_g3_Command_debug_snore},
   {"tuner_status", SATFE_g3_Command_tuner_status},
   {0, NULL},
};


static SATFE_ChipFunctTable SATFE_g3_Functs =
{
   SATFE_g3_GetFreqFromString,
   SATFE_g3_IsSpinv,
   SATFE_g3_IsPilotOn,
   SATFE_g3_GetDebugModuleNames,
   SATFE_g3_GetAdcVoltage,
   NULL,
   NULL
};


#define SATFE_g3_NumDebugModules 18
char *SATFE_g3_DebugModules[SATFE_g3_NumDebugModules] =
{
   "bast",
   "bast_g3",
   "bast_g3_priv",
   "bast_g3_priv_acq",
   "bast_g3_priv_dft",
   "bast_g3_priv_diseqc",
   "bast_g3_priv_fsk",
   "bast_g3_priv_ftm",
   "bast_g3_priv_hp",
   "bast_g3_priv_ldpc",
   "bast_g3_priv_mi2c",
   "bast_g3_priv_cwc",
   "bast_g3_priv_plc",
   "bast_g3_priv_qpsk",
   "bast_g3_priv_snr",
   "bast_g3_priv_timer",
   "bast_g3_priv_tuner",
   "bast_g3_priv_turbo"
};


SATFE_Register SATFE_g3_RegisterMap[] =
{
   {"CG_RSTCTL", BCHP_SDS_CG_RSTCTL, SATFE_RegisterType_eISB},
   {"CG_CGDIV00", BCHP_SDS_CG_CGDIV00, SATFE_RegisterType_eISB},
   {"CG_CGDIV01", BCHP_SDS_CG_CGDIV01, SATFE_RegisterType_eISB},
   {"CG_SPLL_NPDIV", BCHP_SDS_CG_SPLL_NPDIV, SATFE_RegisterType_eISB},
   {"CG_SPLL_MDIV_CTRL", BCHP_SDS_CG_SPLL_MDIV_CTRL, SATFE_RegisterType_eISB},
   {"CG_SPLL_CTRL", BCHP_SDS_CG_SPLL_CTRL, SATFE_RegisterType_eISB},
   {"CG_SPLL_SSC_CTRL1", BCHP_SDS_CG_SPLL_SSC_CTRL1, SATFE_RegisterType_eISB},
   {"CG_SPLL_SSC_CTRL0", BCHP_SDS_CG_SPLL_SSC_CTRL0, SATFE_RegisterType_eISB},
   {"CG_SPLL_STATUS", BCHP_SDS_CG_SPLL_STATUS, SATFE_RegisterType_eISB},
   {"CG_SPLL_PWRDN_RST", BCHP_SDS_CG_SPLL_PWRDN_RST, SATFE_RegisterType_eISB},
   {"FE_ADCPCTL", BCHP_SDS_FE_ADCPCTL, SATFE_RegisterType_eISB},
   {"FE_DCOCTL", BCHP_SDS_FE_DCOCTL, SATFE_RegisterType_eISB},
   {"FE_DCOI", BCHP_SDS_FE_DCOI, SATFE_RegisterType_eISB},
   {"FE_IQCTL", BCHP_SDS_FE_IQCTL, SATFE_RegisterType_eISB},
   {"FE_IQAEST", BCHP_SDS_FE_IQAEST, SATFE_RegisterType_eISB},
   {"FE_IQPEST", BCHP_SDS_FE_IQPEST, SATFE_RegisterType_eISB},
   {"FE_MIXCTL", BCHP_SDS_FE_MIXCTL, SATFE_RegisterType_eISB},
   {"FE_DSTGCTL", BCHP_SDS_FE_DSTGCTL, SATFE_RegisterType_eISB},
   {"FE_FILTCTL", BCHP_SDS_FE_FILTCTL, SATFE_RegisterType_eISB},
   {"FE_DFCTL", BCHP_SDS_FE_DFCTL, SATFE_RegisterType_eISB},
   {"FE_AGFCTL", BCHP_SDS_FE_AGFCTL, SATFE_RegisterType_eISB},
   {"FE_AGF", BCHP_SDS_FE_AGF, SATFE_RegisterType_eISB},
   {"FE_AIF", BCHP_SDS_FE_AIF, SATFE_RegisterType_eISB},
   {"FE_NVCTL", BCHP_SDS_FE_NVCTL, SATFE_RegisterType_eISB},
   {"AGC_AGCCTL", BCHP_SDS_AGC_AGCCTL, SATFE_RegisterType_eISB},
   {"AGC_IAGCTH", BCHP_SDS_AGC_IAGCTH, SATFE_RegisterType_eISB},
   {"AGC_DSGIN", BCHP_SDS_AGC_DSGIN, SATFE_RegisterType_eISB},
   {"AGC_ATHR", BCHP_SDS_AGC_ATHR, SATFE_RegisterType_eISB},
   {"AGC_ABW", BCHP_SDS_AGC_ABW, SATFE_RegisterType_eISB},
   {"AGC_AII", BCHP_SDS_AGC_AII, SATFE_RegisterType_eISB},
   {"AGC_AGI", BCHP_SDS_AGC_AGI, SATFE_RegisterType_eISB},
   {"AGC_AIT", BCHP_SDS_AGC_AIT, SATFE_RegisterType_eISB},
   {"AGC_AGT", BCHP_SDS_AGC_AGT, SATFE_RegisterType_eISB},
   {"AGC_AGCLI", BCHP_SDS_AGC_AGCLI, SATFE_RegisterType_eISB},
   {"BL_BLPCTL", BCHP_SDS_BL_BLPCTL, SATFE_RegisterType_eISB},
   {"BL_PFCTL", BCHP_SDS_BL_PFCTL, SATFE_RegisterType_eISB},
   {"BL_BRSW", BCHP_SDS_BL_BRSW, SATFE_RegisterType_eISB},
   {"BL_BRLC", BCHP_SDS_BL_BRLC, SATFE_RegisterType_eISB},
   {"BL_BRIC", BCHP_SDS_BL_BRIC, SATFE_RegisterType_eISB},
   {"BL_BRI", BCHP_SDS_BL_BRI, SATFE_RegisterType_eISB},
   {"BL_BFOS", BCHP_SDS_BL_BFOS, SATFE_RegisterType_eISB},
   {"BL_BRFO", BCHP_SDS_BL_BRFO, SATFE_RegisterType_eISB},
   {"BL_BNCO", BCHP_SDS_BL_BNCO, SATFE_RegisterType_eISB},
   {"CL_CLCTL1", BCHP_SDS_CL_CLCTL1, SATFE_RegisterType_eISB},
   {"CL_CLCTL2", BCHP_SDS_CL_CLCTL2, SATFE_RegisterType_eISB},
   {"CL_FLLC", BCHP_SDS_CL_FLLC, SATFE_RegisterType_eISB},
   {"CL_FLLC1", BCHP_SDS_CL_FLLC1, SATFE_RegisterType_eISB},
   {"CL_FLIC", BCHP_SDS_CL_FLIC, SATFE_RegisterType_eISB},
   {"CL_FLIC1", BCHP_SDS_CL_FLIC1, SATFE_RegisterType_eISB},
   {"CL_FLSW", BCHP_SDS_CL_FLSW, SATFE_RegisterType_eISB},
   {"CL_FLI", BCHP_SDS_CL_FLI, SATFE_RegisterType_eISB},
   {"CL_FLIF", BCHP_SDS_CL_FLIF, SATFE_RegisterType_eISB},
   {"CL_FLPA", BCHP_SDS_CL_FLPA, SATFE_RegisterType_eISB},
   {"CL_FLTD", BCHP_SDS_CL_FLTD, SATFE_RegisterType_eISB},
   {"CL_PEEST", BCHP_SDS_CL_PEEST, SATFE_RegisterType_eISB},
   {"CL_PLTD", BCHP_SDS_CL_PLTD, SATFE_RegisterType_eISB},
   {"CL_PLC", BCHP_SDS_CL_PLC, SATFE_RegisterType_eISB},
   {"CL_PLC1", BCHP_SDS_CL_PLC1, SATFE_RegisterType_eISB},
   {"CL_PLSW", BCHP_SDS_CL_PLSW, SATFE_RegisterType_eISB},
   {"CL_PLI", BCHP_SDS_CL_PLI, SATFE_RegisterType_eISB},
   {"CL_PLPA", BCHP_SDS_CL_PLPA, SATFE_RegisterType_eISB},
   {"CL_CRBFD", BCHP_SDS_CL_CRBFD, SATFE_RegisterType_eISB},
   {"CL_CLHT", BCHP_SDS_CL_CLHT, SATFE_RegisterType_eISB},
   {"CL_CLLT", BCHP_SDS_CL_CLLT, SATFE_RegisterType_eISB},
   {"CL_CLLA", BCHP_SDS_CL_CLLA, SATFE_RegisterType_eISB},
   {"CL_CLCT", BCHP_SDS_CL_CLCT, SATFE_RegisterType_eISB},
   {"CL_CLFFCTL", BCHP_SDS_CL_CLFFCTL, SATFE_RegisterType_eISB},
   {"CL_FFLPA", BCHP_SDS_CL_FFLPA, SATFE_RegisterType_eISB},
   {"CL_CLFBCTL", BCHP_SDS_CL_CLFBCTL, SATFE_RegisterType_eISB},
   {"CL_FBLC", BCHP_SDS_CL_FBLC, SATFE_RegisterType_eISB},
   {"CL_FBLI", BCHP_SDS_CL_FBLI, SATFE_RegisterType_eISB},
   {"CL_FBPA", BCHP_SDS_CL_FBPA, SATFE_RegisterType_eISB},
   {"CL_CLDAFECTL", BCHP_SDS_CL_CLDAFECTL, SATFE_RegisterType_eISB},
   {"CL_DAFELI", BCHP_SDS_CL_DAFELI, SATFE_RegisterType_eISB},
   {"CL_DAFEINT", BCHP_SDS_CL_DAFEINT, SATFE_RegisterType_eISB},
   {"EQ_EQMISCCTL", BCHP_SDS_EQ_EQMISCCTL, SATFE_RegisterType_eISB},
   {"EQ_EQFFECTL", BCHP_SDS_EQ_EQFFECTL, SATFE_RegisterType_eISB},
   {"EQ_EQCFAD", BCHP_SDS_EQ_EQCFAD, SATFE_RegisterType_eISB},
   {"EQ_EQFRZCTL", BCHP_SDS_EQ_EQFRZCTL, SATFE_RegisterType_eISB},
   {"EQ_F0B", BCHP_SDS_EQ_F0B, SATFE_RegisterType_eISB},
   {"EQ_HD8PSK1", BCHP_SDS_EQ_HD8PSK1, SATFE_RegisterType_eISB},
   {"EQ_HD8PSK2", BCHP_SDS_EQ_HD8PSK2, SATFE_RegisterType_eISB},
   {"EQ_HDQPSK", BCHP_SDS_EQ_HDQPSK, SATFE_RegisterType_eISB},
   {"EQ_HD16QAM", BCHP_SDS_EQ_HD16QAM, SATFE_RegisterType_eISB},
   {"EQ_CMA", BCHP_SDS_EQ_CMA, SATFE_RegisterType_eISB},
   {"EQ_CMATH", BCHP_SDS_EQ_CMATH, SATFE_RegisterType_eISB},
   {"EQ_VLCTL", BCHP_SDS_EQ_VLCTL, SATFE_RegisterType_eISB},
   {"EQ_VLCI", BCHP_SDS_EQ_VLCI, SATFE_RegisterType_eISB},
   {"EQ_VLCQ", BCHP_SDS_EQ_VLCQ, SATFE_RegisterType_eISB},
   {"EQ_VCOS", BCHP_SDS_EQ_VCOS, SATFE_RegisterType_eISB},
   {"EQ_TSFT", BCHP_SDS_EQ_TSFT, SATFE_RegisterType_eISB},
   {"EQ_EQSFT", BCHP_SDS_EQ_EQSFT, SATFE_RegisterType_eISB},
   {"EQ_PILOTCTL", BCHP_SDS_EQ_PILOTCTL, SATFE_RegisterType_eISB},
   {"EQ_PLDCTL", BCHP_SDS_EQ_PLDCTL, SATFE_RegisterType_eISB},
   {"EQ_HDRD", BCHP_SDS_EQ_HDRD, SATFE_RegisterType_eISB},
   {"EQ_HDRA", BCHP_SDS_EQ_HDRA, SATFE_RegisterType_eISB},
   {"EQ_XSEED", BCHP_SDS_EQ_XSEED, SATFE_RegisterType_eISB},
   {"EQ_XTAP1", BCHP_SDS_EQ_XTAP1, SATFE_RegisterType_eISB},
   {"EQ_XTAP2", BCHP_SDS_EQ_XTAP2, SATFE_RegisterType_eISB},
   {"EQ_LUPD", BCHP_SDS_EQ_LUPD, SATFE_RegisterType_eISB},
   {"EQ_LUPA", BCHP_SDS_EQ_LUPA, SATFE_RegisterType_eISB},
   {"EQ_SDSLEN", BCHP_SDS_EQ_SDSLEN, SATFE_RegisterType_eISB},
   {"EQ_SDSIG", BCHP_SDS_EQ_SDSIG, SATFE_RegisterType_eISB},
   {"EQ_MGAIND", BCHP_SDS_EQ_MGAIND, SATFE_RegisterType_eISB},
   {"EQ_MGAINA", BCHP_SDS_EQ_MGAINA, SATFE_RegisterType_eISB},
   {"HP_HPCONTROL", BCHP_SDS_HP_HPCONTROL, SATFE_RegisterType_eISB},
   {"HP_HPCONFIG", BCHP_SDS_HP_HPCONFIG, SATFE_RegisterType_eISB},
   {"HP_FNORM", BCHP_SDS_HP_FNORM, SATFE_RegisterType_eISB},
   {"HP_HPOVERRIDE", BCHP_SDS_HP_HPOVERRIDE, SATFE_RegisterType_eISB},
   {"HP_FROF1", BCHP_SDS_HP_FROF1, SATFE_RegisterType_eISB},
   {"HP_FROF2", BCHP_SDS_HP_FROF2, SATFE_RegisterType_eISB},
   {"HP_FROF3", BCHP_SDS_HP_FROF3, SATFE_RegisterType_eISB},
   {"HP_FROF1_SW", BCHP_SDS_HP_FROF1_SW, SATFE_RegisterType_eISB},
   {"HP_FROF2_SW", BCHP_SDS_HP_FROF2_SW, SATFE_RegisterType_eISB},
   {"HP_FROF3_SW", BCHP_SDS_HP_FROF3_SW, SATFE_RegisterType_eISB},
   {"HP_M_N_PEAK_VERIFY", BCHP_SDS_HP_M_N_PEAK_VERIFY, SATFE_RegisterType_eISB},
   {"HP_M_N_RECEIVER_VERIFY", BCHP_SDS_HP_M_N_RECEIVER_VERIFY, SATFE_RegisterType_eISB},
   {"HP_M_N_RECEIVER_LOCK", BCHP_SDS_HP_M_N_RECEIVER_LOCK, SATFE_RegisterType_eISB},
   {"HP_DCORR_THRESHOLD", BCHP_SDS_HP_DCORR_THRESHOLD, SATFE_RegisterType_eISB},
   {"HP_PLHDRSCR1", BCHP_SDS_HP_PLHDRSCR1, SATFE_RegisterType_eISB},
   {"HP_PLHDRSCR2", BCHP_SDS_HP_PLHDRSCR2, SATFE_RegisterType_eISB},
   {"HP_PLHDRSCR3", BCHP_SDS_HP_PLHDRSCR3, SATFE_RegisterType_eISB},
   {"HP_ACM_CHECK", BCHP_SDS_HP_ACM_CHECK, SATFE_RegisterType_eISB},
   {"HP_FRAME_LENGTH_INITIAL", BCHP_SDS_HP_FRAME_LENGTH_INITIAL, SATFE_RegisterType_eISB},
   {"HP_FRAME_LENGTH_DUMMY_NORMAL", BCHP_SDS_HP_FRAME_LENGTH_DUMMY_NORMAL, SATFE_RegisterType_eISB},
   {"HP_FRAME_LENGTH_QPSK_NORMAL", BCHP_SDS_HP_FRAME_LENGTH_QPSK_NORMAL, SATFE_RegisterType_eISB},
   {"HP_FRAME_LENGTH_8PSK_NORMAL", BCHP_SDS_HP_FRAME_LENGTH_8PSK_NORMAL, SATFE_RegisterType_eISB},
   {"HP_FRAME_LENGTH_16APSK_NORMAL", BCHP_SDS_HP_FRAME_LENGTH_16APSK_NORMAL, SATFE_RegisterType_eISB},
   {"HP_FRAME_LENGTH_32APSK_NORMAL", BCHP_SDS_HP_FRAME_LENGTH_32APSK_NORMAL, SATFE_RegisterType_eISB},
   {"HP_FRAME_LENGTH_RESERVED_29_NORMAL", BCHP_SDS_HP_FRAME_LENGTH_RESERVED_29_NORMAL, SATFE_RegisterType_eISB},
   {"HP_FRAME_LENGTH_RESERVED_30_NORMAL", BCHP_SDS_HP_FRAME_LENGTH_RESERVED_30_NORMAL, SATFE_RegisterType_eISB},
   {"HP_FRAME_LENGTH_RESERVED_31_NORMAL", BCHP_SDS_HP_FRAME_LENGTH_RESERVED_31_NORMAL, SATFE_RegisterType_eISB},
   {"HP_FRAME_LENGTH_DUMMY_SHORT", BCHP_SDS_HP_FRAME_LENGTH_DUMMY_SHORT, SATFE_RegisterType_eISB},
   {"HP_FRAME_LENGTH_QPSK_SHORT", BCHP_SDS_HP_FRAME_LENGTH_QPSK_SHORT, SATFE_RegisterType_eISB},
   {"HP_FRAME_LENGTH_8PSK_SHORT", BCHP_SDS_HP_FRAME_LENGTH_8PSK_SHORT, SATFE_RegisterType_eISB},
   {"HP_FRAME_LENGTH_16APSK_SHORT", BCHP_SDS_HP_FRAME_LENGTH_16APSK_SHORT, SATFE_RegisterType_eISB},
   {"HP_FRAME_LENGTH_32APSK_SHORT", BCHP_SDS_HP_FRAME_LENGTH_32APSK_SHORT, SATFE_RegisterType_eISB},
   {"HP_FRAME_LENGTH_RESERVED_29_SHORT", BCHP_SDS_HP_FRAME_LENGTH_RESERVED_29_SHORT, SATFE_RegisterType_eISB},
   {"HP_FRAME_LENGTH_RESERVED_30_SHORT", BCHP_SDS_HP_FRAME_LENGTH_RESERVED_30_SHORT, SATFE_RegisterType_eISB},
   {"HP_FRAME_LENGTH_RESERVED_31_SHORT", BCHP_SDS_HP_FRAME_LENGTH_RESERVED_31_SHORT, SATFE_RegisterType_eISB},
   {"HP_FRAME_LENGTH_SAMPLE", BCHP_SDS_HP_FRAME_LENGTH_SAMPLE, SATFE_RegisterType_eISB},
   {"HP_PEAK_SAMPLE_1_0", BCHP_SDS_HP_PEAK_SAMPLE_1_0, SATFE_RegisterType_eISB},
   {"HP_PEAK_SAMPLE_3_2", BCHP_SDS_HP_PEAK_SAMPLE_3_2, SATFE_RegisterType_eISB},
   {"VIT_VTCTL", BCHP_SDS_VIT_VTCTL, SATFE_RegisterType_eISB},
   {"VIT_V10", BCHP_SDS_VIT_V10, SATFE_RegisterType_eISB},
   {"VIT_V32", BCHP_SDS_VIT_V32, SATFE_RegisterType_eISB},
   {"VIT_V54", BCHP_SDS_VIT_V54, SATFE_RegisterType_eISB},
   {"VIT_V76", BCHP_SDS_VIT_V76, SATFE_RegisterType_eISB},
   {"VIT_VINT", BCHP_SDS_VIT_VINT, SATFE_RegisterType_eISB},
   {"VIT_VCNT", BCHP_SDS_VIT_VCNT, SATFE_RegisterType_eISB},
   {"VIT_VSTC", BCHP_SDS_VIT_VSTC, SATFE_RegisterType_eISB},
   {"VIT_VST", BCHP_SDS_VIT_VST, SATFE_RegisterType_eISB},
   {"VIT_VREC", BCHP_SDS_VIT_VREC, SATFE_RegisterType_eISB},
   {"VIT_VRCV", BCHP_SDS_VIT_VRCV, SATFE_RegisterType_eISB},
   {"FEC_FECTL", BCHP_SDS_FEC_FECTL, SATFE_RegisterType_eISB},
   {"FEC_FSYN", BCHP_SDS_FEC_FSYN, SATFE_RegisterType_eISB},
   {"FEC_FRS", BCHP_SDS_FEC_FRS, SATFE_RegisterType_eISB},
   {"FEC_FMOD", BCHP_SDS_FEC_FMOD, SATFE_RegisterType_eISB},
   {"FEC_FERR", BCHP_SDS_FEC_FERR, SATFE_RegisterType_eISB},
   {"FEC_FRSV", BCHP_SDS_FEC_FRSV, SATFE_RegisterType_eISB},
   {"OI_OIFCTL00", BCHP_SDS_OI_OIFCTL00, SATFE_RegisterType_eISB},
   {"OI_OIFCTL01", BCHP_SDS_OI_OIFCTL01, SATFE_RegisterType_eISB},
   {"OI_OIFCTL02", BCHP_SDS_OI_OIFCTL02, SATFE_RegisterType_eISB},
   {"OI_OPLL", BCHP_SDS_OI_OPLL, SATFE_RegisterType_eISB},
   {"OI_OPLL2", BCHP_SDS_OI_OPLL2, SATFE_RegisterType_eISB},
   {"OI_FERC", BCHP_SDS_OI_FERC, SATFE_RegisterType_eISB},
   {"OI_FRC", BCHP_SDS_OI_FRC, SATFE_RegisterType_eISB},
   {"OI_OSIGPN", BCHP_SDS_OI_OSIGPN, SATFE_RegisterType_eISB},
   {"OI_OSUBD", BCHP_SDS_OI_OSUBD, SATFE_RegisterType_eISB},
   {"OI_OCOEF", BCHP_SDS_OI_OCOEF, SATFE_RegisterType_eISB},
   {"OI_OFI", BCHP_SDS_OI_OFI, SATFE_RegisterType_eISB},
   {"OI_OPLL_NPDIV", BCHP_SDS_OI_OPLL_NPDIV, SATFE_RegisterType_eISB},
   {"OI_OPLL_MDIV_CTRL", BCHP_SDS_OI_OPLL_MDIV_CTRL, SATFE_RegisterType_eISB},
   {"OI_OPLL_CTRL", BCHP_SDS_OI_OPLL_CTRL, SATFE_RegisterType_eISB},
   {"OI_OPLL_SSC_CTRL1", BCHP_SDS_OI_OPLL_SSC_CTRL1, SATFE_RegisterType_eISB},
   {"OI_OPLL_SSC_CTRL0", BCHP_SDS_OI_OPLL_SSC_CTRL0, SATFE_RegisterType_eISB},
   {"OI_OPLL_STATUS", BCHP_SDS_OI_OPLL_STATUS, SATFE_RegisterType_eISB},
   {"OI_OPLL_PWRDN_RST", BCHP_SDS_OI_OPLL_PWRDN_RST, SATFE_RegisterType_eISB},
   {"SNR_SNRCTL", BCHP_SDS_SNR_SNRCTL, SATFE_RegisterType_eISB},
   {"SNR_SNRHT", BCHP_SDS_SNR_SNRHT, SATFE_RegisterType_eISB},
   {"SNR_SNRLT", BCHP_SDS_SNR_SNRLT, SATFE_RegisterType_eISB},
   {"SNR_SNRE", BCHP_SDS_SNR_SNRE, SATFE_RegisterType_eISB},
   {"SNR_SNORETP", BCHP_SDS_SNR_SNORETP, SATFE_RegisterType_eISB},
   {"SNR_SNORESP", BCHP_SDS_SNR_SNORESP, SATFE_RegisterType_eISB},
   {"SNR_SNORECTL", BCHP_SDS_SNR_SNORECTL, SATFE_RegisterType_eISB},
   {"BERT_BERCTL", BCHP_SDS_BERT_BERCTL, SATFE_RegisterType_eISB},
   {"BERT_BEIT", BCHP_SDS_BERT_BEIT, SATFE_RegisterType_eISB},
   {"BERT_BERC", BCHP_SDS_BERT_BERC, SATFE_RegisterType_eISB},
   {"BERT_BEM1", BCHP_SDS_BERT_BEM1, SATFE_RegisterType_eISB},
   {"BERT_BEM2", BCHP_SDS_BERT_BEM2, SATFE_RegisterType_eISB},
   {"BERT_BEM3", BCHP_SDS_BERT_BEM3, SATFE_RegisterType_eISB},
   {"BERT_BEST", BCHP_SDS_BERT_BEST, SATFE_RegisterType_eISB},
   {"BERT_ACMCTL", BCHP_SDS_BERT_ACMCTL, SATFE_RegisterType_eISB},
   {"DFT_CTRL0", BCHP_SDS_DFT_CTRL0, SATFE_RegisterType_eISB},
   {"DFT_CTRL1", BCHP_SDS_DFT_CTRL1, SATFE_RegisterType_eISB},
   {"DFT_STATUS", BCHP_SDS_DFT_STATUS, SATFE_RegisterType_eISB},
   {"DFT_RANGE_START", BCHP_SDS_DFT_RANGE_START, SATFE_RegisterType_eISB},
   {"DFT_RANGE_END", BCHP_SDS_DFT_RANGE_END, SATFE_RegisterType_eISB},
   {"DFT_DDFS_FCW", BCHP_SDS_DFT_DDFS_FCW, SATFE_RegisterType_eISB},
   {"DFT_PEAK_POW", BCHP_SDS_DFT_PEAK_POW, SATFE_RegisterType_eISB},
   {"DFT_PEAK_BIN", BCHP_SDS_DFT_PEAK_BIN, SATFE_RegisterType_eISB},
   {"DFT_TOTAL_POW", BCHP_SDS_DFT_TOTAL_POW, SATFE_RegisterType_eISB},
   {"DFT_MEM_RADDR", BCHP_SDS_DFT_MEM_RADDR, SATFE_RegisterType_eISB},
   {"DFT_MEM_RDATA", BCHP_SDS_DFT_MEM_RDATA, SATFE_RegisterType_eISB},
#if (BCHP_VER >= BCHP_VER_B0) || (BCHP_CHIP==7360) || (BCHP_CHIP==7362)|| (BCHP_CHIP==73625)
   {"CWC_CTRL1", BCHP_SDS_CWC_CTRL1, SATFE_RegisterType_eISB},
   {"CWC_CTRL2", BCHP_SDS_CWC_CTRL2, SATFE_RegisterType_eISB},
   {"CWC_LEAK", BCHP_SDS_CWC_LEAK, SATFE_RegisterType_eISB},
   {"CWC_SPUR_FCW1", BCHP_SDS_CWC_SPUR_FCW1, SATFE_RegisterType_eISB},
   {"CWC_SPUR_FCW2", BCHP_SDS_CWC_SPUR_FCW2, SATFE_RegisterType_eISB},
   {"CWC_SPUR_FCW3", BCHP_SDS_CWC_SPUR_FCW3, SATFE_RegisterType_eISB},
   {"CWC_SPUR_FCW4", BCHP_SDS_CWC_SPUR_FCW4, SATFE_RegisterType_eISB},
   {"CWC_SPUR_FCW5", BCHP_SDS_CWC_SPUR_FCW5, SATFE_RegisterType_eISB},
   {"CWC_SPUR_FCW6", BCHP_SDS_CWC_SPUR_FCW6, SATFE_RegisterType_eISB},
   {"CWC_LFC1", BCHP_SDS_CWC_LFC1, SATFE_RegisterType_eISB},
   {"CWC_LFC2", BCHP_SDS_CWC_LFC2, SATFE_RegisterType_eISB},
   {"CWC_LFC3", BCHP_SDS_CWC_LFC3, SATFE_RegisterType_eISB},
   {"CWC_LFC4", BCHP_SDS_CWC_LFC4, SATFE_RegisterType_eISB},
   {"CWC_LFC5", BCHP_SDS_CWC_LFC5, SATFE_RegisterType_eISB},
   {"CWC_LFC6", BCHP_SDS_CWC_LFC6, SATFE_RegisterType_eISB},
   {"CWC_INT1", BCHP_SDS_CWC_INT1, SATFE_RegisterType_eISB},
   {"CWC_INT2", BCHP_SDS_CWC_INT2, SATFE_RegisterType_eISB},
   {"CWC_INT3", BCHP_SDS_CWC_INT3, SATFE_RegisterType_eISB},
   {"CWC_INT4", BCHP_SDS_CWC_INT4, SATFE_RegisterType_eISB},
   {"CWC_INT5", BCHP_SDS_CWC_INT5, SATFE_RegisterType_eISB},
   {"CWC_INT6", BCHP_SDS_CWC_INT6, SATFE_RegisterType_eISB},
   {"CWC_COEFF_RWCTRL", BCHP_SDS_CWC_COEFF_RWCTRL, SATFE_RegisterType_eISB},
   {"CWC_COEFF", BCHP_SDS_CWC_COEFF, SATFE_RegisterType_eISB},
   {"CWC_FOFS1", BCHP_SDS_CWC_FOFS1, SATFE_RegisterType_eISB},
   {"CWC_FOFS2", BCHP_SDS_CWC_FOFS2, SATFE_RegisterType_eISB},
   {"CWC_FOFS3", BCHP_SDS_CWC_FOFS3, SATFE_RegisterType_eISB},
   {"CWC_FOFS4", BCHP_SDS_CWC_FOFS4, SATFE_RegisterType_eISB},
   {"CWC_FOFS5", BCHP_SDS_CWC_FOFS5, SATFE_RegisterType_eISB},
   {"CWC_FOFS6", BCHP_SDS_CWC_FOFS6, SATFE_RegisterType_eISB},
#else
   {"NTCH_CTRL", BCHP_SDS_NTCH_CTRL, SATFE_RegisterType_eISB},
   {"NTCH_FCWADJ_SCL", BCHP_SDS_NTCH_FCWADJ_SCL, SATFE_RegisterType_eISB},
   {"NTCH_FCW0", BCHP_SDS_NTCH_FCW0, SATFE_RegisterType_eISB},
   {"NTCH_FCW1", BCHP_SDS_NTCH_FCW1, SATFE_RegisterType_eISB},
   {"NTCH_FCW2", BCHP_SDS_NTCH_FCW2, SATFE_RegisterType_eISB},
   {"NTCH_FCW3", BCHP_SDS_NTCH_FCW3, SATFE_RegisterType_eISB},
   {"NTCH_DCO0_INT", BCHP_SDS_NTCH_DCO0_INT, SATFE_RegisterType_eISB},
   {"NTCH_DCO1_INT", BCHP_SDS_NTCH_DCO1_INT, SATFE_RegisterType_eISB},
   {"NTCH_DCO2_INT", BCHP_SDS_NTCH_DCO2_INT, SATFE_RegisterType_eISB},
#endif
   {"MISC_REVID", BCHP_SDS_MISC_REVID, SATFE_RegisterType_eISB},
   {"MISC_IICTL1", BCHP_SDS_MISC_IICTL1, SATFE_RegisterType_eISB},
   {"MISC_IICTL2", BCHP_SDS_MISC_IICTL2, SATFE_RegisterType_eISB},
   {"MISC_IICCNT", BCHP_SDS_MISC_IICCNT, SATFE_RegisterType_eISB},
   {"MISC_IICHPA", BCHP_SDS_MISC_IICHPA, SATFE_RegisterType_eISB},
   {"MISC_MIICTX1", BCHP_SDS_MISC_MIICTX1, SATFE_RegisterType_eISB},
   {"MISC_MIICTX2", BCHP_SDS_MISC_MIICTX2, SATFE_RegisterType_eISB},
   {"MISC_MIICRX1", BCHP_SDS_MISC_MIICRX1, SATFE_RegisterType_eISB},
   {"MISC_MIICRX2", BCHP_SDS_MISC_MIICRX2, SATFE_RegisterType_eISB},
   {"MISC_MI2CSA", BCHP_SDS_MISC_MI2CSA, SATFE_RegisterType_eISB},
   {"MISC_TMRCTL", BCHP_SDS_MISC_TMRCTL, SATFE_RegisterType_eISB},
   {"MISC_GENTMR3", BCHP_SDS_MISC_GENTMR3, SATFE_RegisterType_eISB},
   {"MISC_GENTMR2", BCHP_SDS_MISC_GENTMR2, SATFE_RegisterType_eISB},
   {"MISC_GENTMR1", BCHP_SDS_MISC_GENTMR1, SATFE_RegisterType_eISB},
   {"MISC_BERTMR", BCHP_SDS_MISC_BERTMR, SATFE_RegisterType_eISB},
   {"MISC_BTMR", BCHP_SDS_MISC_BTMR, SATFE_RegisterType_eISB},
   {"MISC_TPDIR", BCHP_SDS_MISC_TPDIR, SATFE_RegisterType_eISB},
   {"MISC_TPODS", BCHP_SDS_MISC_TPODS, SATFE_RegisterType_eISB},
   {"MISC_TPDS", BCHP_SDS_MISC_TPDS, SATFE_RegisterType_eISB},
   {"MISC_TPCTL1", BCHP_SDS_MISC_TPCTL1, SATFE_RegisterType_eISB},
   {"MISC_TPCTL2", BCHP_SDS_MISC_TPCTL2, SATFE_RegisterType_eISB},
   {"MISC_TPCTL3", BCHP_SDS_MISC_TPCTL3, SATFE_RegisterType_eISB},
   {"MISC_TPOUT", BCHP_SDS_MISC_TPOUT, SATFE_RegisterType_eISB},
   {"MISC_MISCTL", BCHP_SDS_MISC_MISCTL, SATFE_RegisterType_eISB},
   {"MISC_INTR_RAW_STS0", BCHP_SDS_MISC_INTR_RAW_STS0, SATFE_RegisterType_eISB},
   {"MISC_INTR_RAW_STS1", BCHP_SDS_MISC_INTR_RAW_STS1, SATFE_RegisterType_eISB},
   {"DSEC_DSRST", BCHP_SDS_DSEC_DSRST, SATFE_RegisterType_eISB},
   {"DSEC_DSCGDIV", BCHP_SDS_DSEC_DSCGDIV, SATFE_RegisterType_eISB},
   {"DSEC_DSCTL00", BCHP_SDS_DSEC_DSCTL00, SATFE_RegisterType_eISB},
   {"DSEC_DSCTL01", BCHP_SDS_DSEC_DSCTL01, SATFE_RegisterType_eISB},
   {"DSEC_DSCTL02", BCHP_SDS_DSEC_DSCTL02, SATFE_RegisterType_eISB},
   {"DSEC_DSCTL03", BCHP_SDS_DSEC_DSCTL03, SATFE_RegisterType_eISB},
   {"DSEC_DSCMD", BCHP_SDS_DSEC_DSCMD, SATFE_RegisterType_eISB},
   {"DSEC_DSRPLY", BCHP_SDS_DSEC_DSRPLY, SATFE_RegisterType_eISB},
   {"DSEC_DSCMEMADR", BCHP_SDS_DSEC_DSCMEMADR, SATFE_RegisterType_eISB},
   {"DSEC_DSCMEMDAT", BCHP_SDS_DSEC_DSCMEMDAT, SATFE_RegisterType_eISB},
   {"DSEC_DSFIRCTL", BCHP_SDS_DSEC_DSFIRCTL, SATFE_RegisterType_eISB},
   {"DSEC_DS_MISC_CONTROL", BCHP_SDS_DSEC_DS_MISC_CONTROL, SATFE_RegisterType_eISB},
   {"DSEC_DS_PARITY", BCHP_SDS_DSEC_DS_PARITY, SATFE_RegisterType_eISB},
   {"DSEC_ADCTL", BCHP_SDS_DSEC_ADCTL, SATFE_RegisterType_eISB},
   {"DSEC_Q15T", BCHP_SDS_DSEC_Q15T, SATFE_RegisterType_eISB},
   {"DSEC_TPWC", BCHP_SDS_DSEC_TPWC, SATFE_RegisterType_eISB},
   {"DSEC_RXBT", BCHP_SDS_DSEC_RXBT, SATFE_RegisterType_eISB},
   {"DSEC_RXRT", BCHP_SDS_DSEC_RXRT, SATFE_RegisterType_eISB},
   {"DSEC_RBDT", BCHP_SDS_DSEC_RBDT, SATFE_RegisterType_eISB},
   {"DSEC_SLEW", BCHP_SDS_DSEC_SLEW, SATFE_RegisterType_eISB},
   {"DSEC_RERT", BCHP_SDS_DSEC_RERT, SATFE_RegisterType_eISB},
   {"DSEC_DSCT", BCHP_SDS_DSEC_DSCT, SATFE_RegisterType_eISB},
   {"DSEC_DTCT", BCHP_SDS_DSEC_DTCT, SATFE_RegisterType_eISB},
   {"DSEC_DDIO", BCHP_SDS_DSEC_DDIO, SATFE_RegisterType_eISB},
   {"DSEC_RTDC1", BCHP_SDS_DSEC_RTDC1, SATFE_RegisterType_eISB},
   {"DSEC_RTDC2", BCHP_SDS_DSEC_RTDC2, SATFE_RegisterType_eISB},
   {"DSEC_TCTL", BCHP_SDS_DSEC_TCTL, SATFE_RegisterType_eISB},
   {"DSEC_CICC", BCHP_SDS_DSEC_CICC, SATFE_RegisterType_eISB},
   {"DSEC_FCIC", BCHP_SDS_DSEC_FCIC, SATFE_RegisterType_eISB},
   {"DSEC_SCIC", BCHP_SDS_DSEC_SCIC, SATFE_RegisterType_eISB},
   {"DSEC_TSTM", BCHP_SDS_DSEC_TSTM, SATFE_RegisterType_eISB},
   {"DSEC_DST1", BCHP_SDS_DSEC_DST1, SATFE_RegisterType_eISB},
   {"DSEC_DST2", BCHP_SDS_DSEC_DST2, SATFE_RegisterType_eISB},
   {"DSEC_DS_SAR_THRSH", BCHP_SDS_DSEC_DS_SAR_THRSH, SATFE_RegisterType_eISB},
   {"DSEC_DS_SAR_DATA_OUT", BCHP_SDS_DSEC_DS_SAR_DATA_OUT, SATFE_RegisterType_eISB},
   {"DSEC_DS_SAR_DC_OFFSET", BCHP_SDS_DSEC_DS_SAR_DC_OFFSET, SATFE_RegisterType_eISB},
   {"DSEC_DS_SAR_LPF_INT", BCHP_SDS_DSEC_DS_SAR_LPF_INT, SATFE_RegisterType_eISB},
   {"DSEC_DS_SAR_CONTROL", BCHP_SDS_DSEC_DS_SAR_CONTROL, SATFE_RegisterType_eISB},
   {"DSEC_DS_COMMON_CONTROL", BCHP_SDS_DSEC_DS_COMMON_CONTROL, SATFE_RegisterType_eISB},
   {"DSEC_DSTMRCTL", BCHP_SDS_DSEC_DSTMRCTL, SATFE_RegisterType_eISB},
   {"DSEC_DSGENTMR1", BCHP_SDS_DSEC_DSGENTMR1, SATFE_RegisterType_eISB},
   {"DSEC_DSGENTMR2", BCHP_SDS_DSEC_DSGENTMR2, SATFE_RegisterType_eISB},
   {"TUNER_DAISY_R01", BCHP_SDS_TUNER_DAISY_R01, SATFE_RegisterType_eISB},
   {"TUNER_BGBIAS_R01", BCHP_SDS_TUNER_BGBIAS_R01, SATFE_RegisterType_eISB},
   {"TUNER_REFPLL_R01", BCHP_SDS_TUNER_REFPLL_R01, SATFE_RegisterType_eISB},
   {"TUNER_REFPLL_R02", BCHP_SDS_TUNER_REFPLL_R02, SATFE_RegisterType_eISB},
   {"TUNER_REFPLL_R03", BCHP_SDS_TUNER_REFPLL_R03, SATFE_RegisterType_eISB},
   {"TUNER_REFPLL_R04", BCHP_SDS_TUNER_REFPLL_R04, SATFE_RegisterType_eISB},
   {"TUNER_ADC_R01", BCHP_SDS_TUNER_ADC_R01, SATFE_RegisterType_eISB},
   {"TUNER_ADC_R02", BCHP_SDS_TUNER_ADC_R02, SATFE_RegisterType_eISB},
   {"TUNER_ADC_R03", BCHP_SDS_TUNER_ADC_R03, SATFE_RegisterType_eISB},
   {"TUNER_ADC_R04", BCHP_SDS_TUNER_ADC_R04, SATFE_RegisterType_eISB},
   {"TUNER_IFPGA_R01", BCHP_SDS_TUNER_IFPGA_R01, SATFE_RegisterType_eISB},
   {"TUNER_LNA_R01", BCHP_SDS_TUNER_LNA_R01, SATFE_RegisterType_eISB},
   {"TUNER_LNA_R02", BCHP_SDS_TUNER_LNA_R02, SATFE_RegisterType_eISB},
   {"TUNER_LO_R01", BCHP_SDS_TUNER_LO_R01, SATFE_RegisterType_eISB},
   {"TUNER_LO_R02", BCHP_SDS_TUNER_LO_R02, SATFE_RegisterType_eISB},
   {"TUNER_LO_R03", BCHP_SDS_TUNER_LO_R03, SATFE_RegisterType_eISB},
   {"TUNER_LO_R04", BCHP_SDS_TUNER_LO_R04, SATFE_RegisterType_eISB},
   {"TUNER_LO_R05", BCHP_SDS_TUNER_LO_R05, SATFE_RegisterType_eISB},
   {"TUNER_LO_R06", BCHP_SDS_TUNER_LO_R06, SATFE_RegisterType_eISB},
   {"TUNER_LO_R07", BCHP_SDS_TUNER_LO_R07, SATFE_RegisterType_eISB},
   {"TUNER_LPF_R01", BCHP_SDS_TUNER_LPF_R01, SATFE_RegisterType_eISB},
   {"TUNER_RFPGA_R01", BCHP_SDS_TUNER_RFPGA_R01, SATFE_RegisterType_eISB},
   {"TUNER_BBAGC_R01", BCHP_SDS_TUNER_BBAGC_R01, SATFE_RegisterType_eISB},
   {"TUNER_BBAGC_R02", BCHP_SDS_TUNER_BBAGC_R02, SATFE_RegisterType_eISB},
   {"TUNER_BBAGC_R03", BCHP_SDS_TUNER_BBAGC_R03, SATFE_RegisterType_eISB},
   {"TUNER_BBAGC_R04", BCHP_SDS_TUNER_BBAGC_R04, SATFE_RegisterType_eISB},
   {"TUNER_BBAGC_R05", BCHP_SDS_TUNER_BBAGC_R05, SATFE_RegisterType_eISB},
   {"TUNER_CALDAC_R01", BCHP_SDS_TUNER_CALDAC_R01, SATFE_RegisterType_eISB},
   {"TUNER_LODDFS_R01", BCHP_SDS_TUNER_LODDFS_R01, SATFE_RegisterType_eISB},
   {"TUNER_LODDFS_R02", BCHP_SDS_TUNER_LODDFS_R02, SATFE_RegisterType_eISB},
   {"TUNER_MXFGA_R01", BCHP_SDS_TUNER_MXFGA_R01, SATFE_RegisterType_eISB},
   {"TUNER_MXFGA_R02", BCHP_SDS_TUNER_MXFGA_R02, SATFE_RegisterType_eISB},
   {"TUNER_POSTDCO_R01", BCHP_SDS_TUNER_POSTDCO_R01, SATFE_RegisterType_eISB},
   {"TUNER_PREDCO_R01", BCHP_SDS_TUNER_PREDCO_R01, SATFE_RegisterType_eISB},
   {"TUNER_RFAGC_R01", BCHP_SDS_TUNER_RFAGC_R01, SATFE_RegisterType_eISB},
   {"TUNER_RFAGC_R02", BCHP_SDS_TUNER_RFAGC_R02, SATFE_RegisterType_eISB},
   {"TUNER_RFAGC_R03", BCHP_SDS_TUNER_RFAGC_R03, SATFE_RegisterType_eISB},
   {"TUNER_RFAGC_R04", BCHP_SDS_TUNER_RFAGC_R04, SATFE_RegisterType_eISB},
   {"TUNER_RFAGC_R05", BCHP_SDS_TUNER_RFAGC_R05, SATFE_RegisterType_eISB},
   {"TUNER_DDFS_R01", BCHP_SDS_TUNER_DDFS_R01, SATFE_RegisterType_eISB},
   {"TUNER_PLLSTAT_R01", BCHP_SDS_TUNER_PLLSTAT_R01, SATFE_RegisterType_eISB},
   {"TUNER_PREDCOI_R01", BCHP_SDS_TUNER_PREDCOI_R01, SATFE_RegisterType_eISB},
   {"TUNER_PREDCOI_R02", BCHP_SDS_TUNER_PREDCOI_R02, SATFE_RegisterType_eISB},
   {"TUNER_PREDCOI_R03", BCHP_SDS_TUNER_PREDCOI_R03, SATFE_RegisterType_eISB},
   {"TUNER_PREDCOI_R04", BCHP_SDS_TUNER_PREDCOI_R04, SATFE_RegisterType_eISB},
   {"TUNER_PREDCOQ_R01", BCHP_SDS_TUNER_PREDCOQ_R01, SATFE_RegisterType_eISB},
   {"TUNER_PREDCOQ_R02", BCHP_SDS_TUNER_PREDCOQ_R02, SATFE_RegisterType_eISB},
   {"TUNER_PREDCOQ_R03", BCHP_SDS_TUNER_PREDCOQ_R03, SATFE_RegisterType_eISB},
   {"TUNER_PREDCOQ_R04", BCHP_SDS_TUNER_PREDCOQ_R04, SATFE_RegisterType_eISB},
   {"TUNER_POSTDCOI_R01", BCHP_SDS_TUNER_POSTDCOI_R01, SATFE_RegisterType_eISB},
   {"TUNER_POSTDCOI_R02", BCHP_SDS_TUNER_POSTDCOI_R02, SATFE_RegisterType_eISB},
   {"TUNER_POSTDCOI_R03", BCHP_SDS_TUNER_POSTDCOI_R03, SATFE_RegisterType_eISB},
   {"TUNER_POSTDCOI_R04", BCHP_SDS_TUNER_POSTDCOI_R04, SATFE_RegisterType_eISB},
   {"TUNER_POSTDCOQ_R01", BCHP_SDS_TUNER_POSTDCOQ_R01, SATFE_RegisterType_eISB},
   {"TUNER_POSTDCOQ_R02", BCHP_SDS_TUNER_POSTDCOQ_R02, SATFE_RegisterType_eISB},
   {"TUNER_POSTDCOQ_R03", BCHP_SDS_TUNER_POSTDCOQ_R03, SATFE_RegisterType_eISB},
   {"TUNER_POSTDCOQ_R04", BCHP_SDS_TUNER_POSTDCOQ_R04, SATFE_RegisterType_eISB},
   {"TUNER_RESET_R01", BCHP_SDS_TUNER_RESET_R01, SATFE_RegisterType_eISB},
   {"TUNER_AGC_IF_CTL", BCHP_SDS_TUNER_AGC_IF_CTL, SATFE_RegisterType_eISB},
   {"TUNER_FILCAL_DDFS_CTL", BCHP_SDS_TUNER_FILCAL_DDFS_CTL, SATFE_RegisterType_eISB},
   {"TUNER_TPCTL", BCHP_SDS_TUNER_TPCTL, SATFE_RegisterType_eISB},
   {"TUNER_PWRUP_COMMON_R01", BCHP_SDS_TUNER_PWRUP_COMMON_R01, SATFE_RegisterType_eISB},
   {"TUNER_PWRUP_R01", BCHP_SDS_TUNER_PWRUP_R01, SATFE_RegisterType_eISB},
   {"INTR2_0_CPU_STATUS", BCHP_SDS_INTR2_0_CPU_STATUS, SATFE_RegisterType_eISB},
   {"INTR2_0_CPU_SET", BCHP_SDS_INTR2_0_CPU_SET, SATFE_RegisterType_eISB},
   {"INTR2_0_CPU_CLEAR", BCHP_SDS_INTR2_0_CPU_CLEAR, SATFE_RegisterType_eISB},
   {"INTR2_0_CPU_MASK_STATUS", BCHP_SDS_INTR2_0_CPU_MASK_STATUS, SATFE_RegisterType_eISB},
   {"INTR2_0_CPU_MASK_SET", BCHP_SDS_INTR2_0_CPU_MASK_SET, SATFE_RegisterType_eISB},
   {"INTR2_0_CPU_MASK_CLEAR", BCHP_SDS_INTR2_0_CPU_MASK_CLEAR, SATFE_RegisterType_eISB},
   {"INTR2_0_PCI_STATUS", BCHP_SDS_INTR2_0_PCI_STATUS, SATFE_RegisterType_eISB},
   {"INTR2_0_PCI_SET", BCHP_SDS_INTR2_0_PCI_SET, SATFE_RegisterType_eISB},
   {"INTR2_0_PCI_CLEAR", BCHP_SDS_INTR2_0_PCI_CLEAR, SATFE_RegisterType_eISB},
   {"INTR2_0_PCI_MASK_STATUS", BCHP_SDS_INTR2_0_PCI_MASK_STATUS, SATFE_RegisterType_eISB},
   {"INTR2_0_PCI_MASK_SET", BCHP_SDS_INTR2_0_PCI_MASK_SET, SATFE_RegisterType_eISB},
   {"INTR2_0_PCI_MASK_CLEAR", BCHP_SDS_INTR2_0_PCI_MASK_CLEAR, SATFE_RegisterType_eISB},
   {"INTR2_1_CPU_STATUS", BCHP_SDS_INTR2_1_CPU_STATUS, SATFE_RegisterType_eISB},
   {"INTR2_1_CPU_SET", BCHP_SDS_INTR2_1_CPU_SET, SATFE_RegisterType_eISB},
   {"INTR2_1_CPU_CLEAR", BCHP_SDS_INTR2_1_CPU_CLEAR, SATFE_RegisterType_eISB},
   {"INTR2_1_CPU_MASK_STATUS", BCHP_SDS_INTR2_1_CPU_MASK_STATUS, SATFE_RegisterType_eISB},
   {"INTR2_1_CPU_MASK_SET", BCHP_SDS_INTR2_1_CPU_MASK_SET, SATFE_RegisterType_eISB},
   {"INTR2_1_CPU_MASK_CLEAR", BCHP_SDS_INTR2_1_CPU_MASK_CLEAR, SATFE_RegisterType_eISB},
   {"INTR2_1_PCI_STATUS", BCHP_SDS_INTR2_1_PCI_STATUS, SATFE_RegisterType_eISB},
   {"INTR2_1_PCI_SET", BCHP_SDS_INTR2_1_PCI_SET, SATFE_RegisterType_eISB},
   {"INTR2_1_PCI_CLEAR", BCHP_SDS_INTR2_1_PCI_CLEAR, SATFE_RegisterType_eISB},
   {"INTR2_1_PCI_MASK_STATUS", BCHP_SDS_INTR2_1_PCI_MASK_STATUS, SATFE_RegisterType_eISB},
   {"INTR2_1_PCI_MASK_SET", BCHP_SDS_INTR2_1_PCI_MASK_SET, SATFE_RegisterType_eISB},
   {"INTR2_1_PCI_MASK_CLEAR", BCHP_SDS_INTR2_1_PCI_MASK_CLEAR, SATFE_RegisterType_eISB},
   {"TFEC_TFECTL", BCHP_TFEC_TFECTL, SATFE_RegisterType_eISB},
#if (BCHP_VER < BCHP_VER_B0) && (BCHP_CHIP!=7360) && (BCHP_CHIP!=7362) && (BCHP_CHIP!=73625)
   {"TFEC_MEMTM0", BCHP_TFEC_MEMTM0, SATFE_RegisterType_eISB},
   {"TFEC_MEMTM1", BCHP_TFEC_MEMTM1, SATFE_RegisterType_eISB},
   {"TFEC_MEMTM2", BCHP_TFEC_MEMTM2, SATFE_RegisterType_eISB},
   {"TFEC_MEMTM3", BCHP_TFEC_MEMTM3, SATFE_RegisterType_eISB},
#endif
   {"TFEC_TNBLK", BCHP_TFEC_TNBLK, SATFE_RegisterType_eISB},
   {"TFEC_TCBLK", BCHP_TFEC_TCBLK, SATFE_RegisterType_eISB},
   {"TFEC_TBBLK", BCHP_TFEC_TBBLK, SATFE_RegisterType_eISB},
   {"TFEC_TCSYM", BCHP_TFEC_TCSYM, SATFE_RegisterType_eISB},
   {"TFEC_TFMT", BCHP_TFEC_TFMT, SATFE_RegisterType_eISB},
   {"TFEC_TPAK", BCHP_TFEC_TPAK, SATFE_RegisterType_eISB},
   {"TFEC_TSSQ", BCHP_TFEC_TSSQ, SATFE_RegisterType_eISB},
   {"TFEC_TSYN", BCHP_TFEC_TSYN, SATFE_RegisterType_eISB},
   {"TFEC_TTUR", BCHP_TFEC_TTUR, SATFE_RegisterType_eISB},
   {"TFEC_TZPK", BCHP_TFEC_TZPK, SATFE_RegisterType_eISB},
   {"TFEC_TZSY", BCHP_TFEC_TZSY, SATFE_RegisterType_eISB},
   {"TFEC_TITR", BCHP_TFEC_TITR, SATFE_RegisterType_eISB},
   {"TFEC_TCIL", BCHP_TFEC_TCIL, SATFE_RegisterType_eISB},
   {"TFEC_TRSD", BCHP_TFEC_TRSD, SATFE_RegisterType_eISB},
   {"TFEC_TPN", BCHP_TFEC_TPN, SATFE_RegisterType_eISB},
   {"TFEC_TSIGCNT", BCHP_TFEC_TSIGCNT, SATFE_RegisterType_eISB},
   {"TFEC_TSIGITD", BCHP_TFEC_TSIGITD, SATFE_RegisterType_eISB},
   {"TFEC_TSIGXPT", BCHP_TFEC_TSIGXPT, SATFE_RegisterType_eISB},
   {"TFEC_TTPCTL", BCHP_TFEC_TTPCTL, SATFE_RegisterType_eISB},
   {"TFEC_MISC_POST_DIV_CTL", BCHP_TFEC_MISC_POST_DIV_CTL, SATFE_RegisterType_eISB},
   {"TFEC_MISC_REGF_STBY", BCHP_TFEC_MISC_REGF_STBY, SATFE_RegisterType_eISB},
#if (BCHP_VER >= BCHP_VER_B0) || (BCHP_CHIP==7360) || (BCHP_CHIP==7362) || (BCHP_CHIP==73625)
   {"TFEC_MISC_MISCCTL", BCHP_TFEC_MISC_MISCCTL, SATFE_RegisterType_eISB},
#else
   {"TFEC_MISC_ROSCCTL", BCHP_TFEC_MISC_ROSCCTL, SATFE_RegisterType_eISB},
#endif
   {"TFEC_INTR2_CPU_STATUS", BCHP_TFEC_INTR2_CPU_STATUS, SATFE_RegisterType_eISB},
   {"TFEC_INTR2_CPU_SET", BCHP_TFEC_INTR2_CPU_SET, SATFE_RegisterType_eISB},
   {"TFEC_INTR2_CPU_CLEAR", BCHP_TFEC_INTR2_CPU_CLEAR, SATFE_RegisterType_eISB},
   {"TFEC_INTR2_CPU_MASK_STATUS", BCHP_TFEC_INTR2_CPU_MASK_STATUS, SATFE_RegisterType_eISB},
   {"TFEC_INTR2_CPU_MASK_SET", BCHP_TFEC_INTR2_CPU_MASK_SET, SATFE_RegisterType_eISB},
   {"TFEC_INTR2_CPU_MASK_CLEAR", BCHP_TFEC_INTR2_CPU_MASK_CLEAR, SATFE_RegisterType_eISB},
   {"TFEC_INTR2_PCI_STATUS", BCHP_TFEC_INTR2_PCI_STATUS, SATFE_RegisterType_eISB},
   {"TFEC_INTR2_PCI_SET", BCHP_TFEC_INTR2_PCI_SET, SATFE_RegisterType_eISB},
   {"TFEC_INTR2_PCI_CLEAR", BCHP_TFEC_INTR2_PCI_CLEAR, SATFE_RegisterType_eISB},
   {"TFEC_INTR2_PCI_MASK_STATUS", BCHP_TFEC_INTR2_PCI_MASK_STATUS, SATFE_RegisterType_eISB},
   {"TFEC_INTR2_PCI_MASK_SET", BCHP_TFEC_INTR2_PCI_MASK_SET, SATFE_RegisterType_eISB},
   {"TFEC_INTR2_PCI_MASK_CLEAR", BCHP_TFEC_INTR2_PCI_MASK_CLEAR, SATFE_RegisterType_eISB},
   {"AFEC_RST", BCHP_AFEC_RST, SATFE_RegisterType_eISB},
   {"AFEC_CNTR_CTRL", BCHP_AFEC_CNTR_CTRL, SATFE_RegisterType_eISB},
   {"AFEC_TEST_CONFIG", BCHP_AFEC_TEST_CONFIG, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MAX_ITER_OVERIDE", BCHP_AFEC_ACM_MAX_ITER_OVERIDE, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_1", BCHP_AFEC_ACM_MODCOD_1, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_2", BCHP_AFEC_ACM_MODCOD_2, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_3", BCHP_AFEC_ACM_MODCOD_3, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_4", BCHP_AFEC_ACM_MODCOD_4, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_5", BCHP_AFEC_ACM_MODCOD_5, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_6", BCHP_AFEC_ACM_MODCOD_6, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_7", BCHP_AFEC_ACM_MODCOD_7, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_8", BCHP_AFEC_ACM_MODCOD_8, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_9", BCHP_AFEC_ACM_MODCOD_9, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_10", BCHP_AFEC_ACM_MODCOD_10, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_11", BCHP_AFEC_ACM_MODCOD_11, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_12", BCHP_AFEC_ACM_MODCOD_12, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_13", BCHP_AFEC_ACM_MODCOD_13, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_14", BCHP_AFEC_ACM_MODCOD_14, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_15", BCHP_AFEC_ACM_MODCOD_15, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_16", BCHP_AFEC_ACM_MODCOD_16, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_17", BCHP_AFEC_ACM_MODCOD_17, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_18", BCHP_AFEC_ACM_MODCOD_18, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_19", BCHP_AFEC_ACM_MODCOD_19, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_20", BCHP_AFEC_ACM_MODCOD_20, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_21", BCHP_AFEC_ACM_MODCOD_21, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_22", BCHP_AFEC_ACM_MODCOD_22, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_23", BCHP_AFEC_ACM_MODCOD_23, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_24", BCHP_AFEC_ACM_MODCOD_24, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_25", BCHP_AFEC_ACM_MODCOD_25, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_26", BCHP_AFEC_ACM_MODCOD_26, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_27", BCHP_AFEC_ACM_MODCOD_27, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_28", BCHP_AFEC_ACM_MODCOD_28, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_29_EXT", BCHP_AFEC_ACM_MODCOD_29_EXT, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_29_LDPC0_EXT", BCHP_AFEC_ACM_MODCOD_29_LDPC0_EXT, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_30_EXT", BCHP_AFEC_ACM_MODCOD_30_EXT, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_30_LDPC0_EXT", BCHP_AFEC_ACM_MODCOD_30_LDPC0_EXT, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_31_EXT", BCHP_AFEC_ACM_MODCOD_31_EXT, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_31_LDPC0_EXT", BCHP_AFEC_ACM_MODCOD_31_LDPC0_EXT, SATFE_RegisterType_eISB},
   {"AFEC_ACM_SYM_CNT_0", BCHP_AFEC_ACM_SYM_CNT_0, SATFE_RegisterType_eISB},
   {"AFEC_ACM_SYM_CNT_1", BCHP_AFEC_ACM_SYM_CNT_1, SATFE_RegisterType_eISB},
   {"AFEC_ACM_SYM_CNT_2", BCHP_AFEC_ACM_SYM_CNT_2, SATFE_RegisterType_eISB},
   {"AFEC_ACM_CYCLE_CNT_0", BCHP_AFEC_ACM_CYCLE_CNT_0, SATFE_RegisterType_eISB},
   {"AFEC_ACM_CYCLE_CNT_1", BCHP_AFEC_ACM_CYCLE_CNT_1, SATFE_RegisterType_eISB},
   {"AFEC_ACM_CYCLE_CNT_2", BCHP_AFEC_ACM_CYCLE_CNT_2, SATFE_RegisterType_eISB},
   {"AFEC_ACM_CYCLE_CNT_3", BCHP_AFEC_ACM_CYCLE_CNT_3, SATFE_RegisterType_eISB},
   {"AFEC_ACM_CYCLE_CNT_4", BCHP_AFEC_ACM_CYCLE_CNT_4, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MISC_0", BCHP_AFEC_ACM_MISC_0, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MISC_1", BCHP_AFEC_ACM_MISC_1, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_OVERIDE", BCHP_AFEC_ACM_MODCOD_OVERIDE, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_STATS_CONFIG", BCHP_AFEC_ACM_MODCOD_STATS_CONFIG, SATFE_RegisterType_eISB},
   {"AFEC_ACM_DONT_DEC_CNT", BCHP_AFEC_ACM_DONT_DEC_CNT, SATFE_RegisterType_eISB},
   {"AFEC_ACM_LDPC_ITER_CNT", BCHP_AFEC_ACM_LDPC_ITER_CNT, SATFE_RegisterType_eISB},
   {"AFEC_ACM_LDPC_FAIL_CNT", BCHP_AFEC_ACM_LDPC_FAIL_CNT, SATFE_RegisterType_eISB},
   {"AFEC_ACM_LDPC_FRM_CNT", BCHP_AFEC_ACM_LDPC_FRM_CNT, SATFE_RegisterType_eISB},
   {"AFEC_LDPC_CONFIG_0", BCHP_AFEC_LDPC_CONFIG_0, SATFE_RegisterType_eISB},
   {"AFEC_LDPC_STATUS", BCHP_AFEC_LDPC_STATUS, SATFE_RegisterType_eISB},
   {"AFEC_LDPC_MET_CRC", BCHP_AFEC_LDPC_MET_CRC, SATFE_RegisterType_eISB},
   {"AFEC_LDPC_EDGE_CRC", BCHP_AFEC_LDPC_EDGE_CRC, SATFE_RegisterType_eISB},
   {"AFEC_LDPC_PSL_CTL", BCHP_AFEC_LDPC_PSL_CTL, SATFE_RegisterType_eISB},
   {"AFEC_LDPC_PSL_INT_THRES", BCHP_AFEC_LDPC_PSL_INT_THRES, SATFE_RegisterType_eISB},
   {"AFEC_LDPC_PSL_INT", BCHP_AFEC_LDPC_PSL_INT, SATFE_RegisterType_eISB},
   {"AFEC_LDPC_PSL_AVE", BCHP_AFEC_LDPC_PSL_AVE, SATFE_RegisterType_eISB},
   {"AFEC_LDPC_PSL_XCS", BCHP_AFEC_LDPC_PSL_XCS, SATFE_RegisterType_eISB},
   {"AFEC_LDPC_PSL_FILTER", BCHP_AFEC_LDPC_PSL_FILTER, SATFE_RegisterType_eISB},
   {"AFEC_BCH_TPCTL", BCHP_AFEC_BCH_TPCTL, SATFE_RegisterType_eISB},
   {"AFEC_BCH_TPSIG", BCHP_AFEC_BCH_TPSIG, SATFE_RegisterType_eISB},
   {"AFEC_BCH_CTRL", BCHP_AFEC_BCH_CTRL, SATFE_RegisterType_eISB},
   {"AFEC_BCH_DECNBLK", BCHP_AFEC_BCH_DECNBLK, SATFE_RegisterType_eISB},
   {"AFEC_BCH_DECCBLK", BCHP_AFEC_BCH_DECCBLK, SATFE_RegisterType_eISB},
   {"AFEC_BCH_DECBBLK", BCHP_AFEC_BCH_DECBBLK, SATFE_RegisterType_eISB},
   {"AFEC_BCH_DECCBIT", BCHP_AFEC_BCH_DECCBIT, SATFE_RegisterType_eISB},
   {"AFEC_BCH_DECMCOR", BCHP_AFEC_BCH_DECMCOR, SATFE_RegisterType_eISB},
   {"AFEC_BCH_BBHDR0", BCHP_AFEC_BCH_BBHDR0, SATFE_RegisterType_eISB},
   {"AFEC_BCH_BBHDR1", BCHP_AFEC_BCH_BBHDR1, SATFE_RegisterType_eISB},
   {"AFEC_BCH_BBHDR2", BCHP_AFEC_BCH_BBHDR2, SATFE_RegisterType_eISB},
   {"AFEC_BCH_BBHDR3", BCHP_AFEC_BCH_BBHDR3, SATFE_RegisterType_eISB},
   {"AFEC_BCH_BBHDR4", BCHP_AFEC_BCH_BBHDR4, SATFE_RegisterType_eISB},
   {"AFEC_BCH_BBHDR5", BCHP_AFEC_BCH_BBHDR5, SATFE_RegisterType_eISB},
   {"AFEC_BCH_MPLCK", BCHP_AFEC_BCH_MPLCK, SATFE_RegisterType_eISB},
   {"AFEC_BCH_MPCFG", BCHP_AFEC_BCH_MPCFG, SATFE_RegisterType_eISB},
   {"AFEC_BCH_SMCFG", BCHP_AFEC_BCH_SMCFG, SATFE_RegisterType_eISB},
   {"AFEC_INTR_CTRL2_CPU_STATUS", BCHP_AFEC_INTR_CTRL2_CPU_STATUS, SATFE_RegisterType_eISB},
   {"AFEC_INTR_CTRL2_CPU_SET", BCHP_AFEC_INTR_CTRL2_CPU_SET, SATFE_RegisterType_eISB},
   {"AFEC_INTR_CTRL2_CPU_CLEAR", BCHP_AFEC_INTR_CTRL2_CPU_CLEAR, SATFE_RegisterType_eISB},
   {"AFEC_INTR_CTRL2_CPU_MASK_STATUS", BCHP_AFEC_INTR_CTRL2_CPU_MASK_STATUS, SATFE_RegisterType_eISB},
   {"AFEC_INTR_CTRL2_CPU_MASK_SET", BCHP_AFEC_INTR_CTRL2_CPU_MASK_SET, SATFE_RegisterType_eISB},
   {"AFEC_INTR_CTRL2_CPU_MASK_CLEAR", BCHP_AFEC_INTR_CTRL2_CPU_MASK_CLEAR, SATFE_RegisterType_eISB},
   {"AFEC_INTR_CTRL2_PCI_STATUS", BCHP_AFEC_INTR_CTRL2_PCI_STATUS, SATFE_RegisterType_eISB},
   {"AFEC_INTR_CTRL2_PCI_SET", BCHP_AFEC_INTR_CTRL2_PCI_SET, SATFE_RegisterType_eISB},
   {"AFEC_INTR_CTRL2_PCI_CLEAR", BCHP_AFEC_INTR_CTRL2_PCI_CLEAR, SATFE_RegisterType_eISB},
   {"AFEC_INTR_CTRL2_PCI_MASK_STATUS", BCHP_AFEC_INTR_CTRL2_PCI_MASK_STATUS, SATFE_RegisterType_eISB},
   {"AFEC_INTR_CTRL2_PCI_MASK_SET", BCHP_AFEC_INTR_CTRL2_PCI_MASK_SET, SATFE_RegisterType_eISB},
   {"AFEC_INTR_CTRL2_PCI_MASK_CLEAR", BCHP_AFEC_INTR_CTRL2_PCI_MASK_CLEAR, SATFE_RegisterType_eISB},
   {"AFEC_GLOBAL_CLK_CNTRL", BCHP_AFEC_GLOBAL_CLK_CNTRL, SATFE_RegisterType_eISB},
   {"AFEC_GLOBAL_PWR_CNTRL", BCHP_AFEC_GLOBAL_PWR_CNTRL, SATFE_RegisterType_eISB},
   {"FTM_UART_RBR", BCHP_FTM_UART_RBR, SATFE_RegisterType_eISB},
   {"FTM_UART_THR", BCHP_FTM_UART_THR, SATFE_RegisterType_eISB},
   {"FTM_UART_DLH", BCHP_FTM_UART_DLH, SATFE_RegisterType_eISB},
   {"FTM_UART_DLL", BCHP_FTM_UART_DLL, SATFE_RegisterType_eISB},
   {"FTM_UART_IER", BCHP_FTM_UART_IER, SATFE_RegisterType_eISB},
   {"FTM_UART_IIR", BCHP_FTM_UART_IIR, SATFE_RegisterType_eISB},
   {"FTM_UART_FCR", BCHP_FTM_UART_FCR, SATFE_RegisterType_eISB},
   {"FTM_UART_LCR", BCHP_FTM_UART_LCR, SATFE_RegisterType_eISB},
   {"FTM_UART_MCR", BCHP_FTM_UART_MCR, SATFE_RegisterType_eISB},
   {"FTM_UART_LSR", BCHP_FTM_UART_LSR, SATFE_RegisterType_eISB},
   {"FTM_UART_MSR", BCHP_FTM_UART_MSR, SATFE_RegisterType_eISB},
   {"FTM_UART_SCR", BCHP_FTM_UART_SCR, SATFE_RegisterType_eISB},
   {"FTM_UART_SRBR", BCHP_FTM_UART_SRBR, SATFE_RegisterType_eISB},
   {"FTM_UART_STHR", BCHP_FTM_UART_STHR, SATFE_RegisterType_eISB},
   {"FTM_UART_FAR", BCHP_FTM_UART_FAR, SATFE_RegisterType_eISB},
   {"FTM_UART_TFR", BCHP_FTM_UART_TFR, SATFE_RegisterType_eISB},
   {"FTM_UART_RFW", BCHP_FTM_UART_RFW, SATFE_RegisterType_eISB},
   {"FTM_UART_USR", BCHP_FTM_UART_USR, SATFE_RegisterType_eISB},
   {"FTM_UART_TFL", BCHP_FTM_UART_TFL, SATFE_RegisterType_eISB},
   {"FTM_UART_RFL", BCHP_FTM_UART_RFL, SATFE_RegisterType_eISB},
   {"FTM_UART_SRR", BCHP_FTM_UART_SRR, SATFE_RegisterType_eISB},
   {"FTM_UART_SRTS", BCHP_FTM_UART_SRTS, SATFE_RegisterType_eISB},
   {"FTM_UART_SBCR", BCHP_FTM_UART_SBCR, SATFE_RegisterType_eISB},
   {"FTM_UART_SDMAM", BCHP_FTM_UART_SDMAM, SATFE_RegisterType_eISB},
   {"FTM_UART_SFE", BCHP_FTM_UART_SFE, SATFE_RegisterType_eISB},
   {"FTM_UART_SRT", BCHP_FTM_UART_SRT, SATFE_RegisterType_eISB},
   {"FTM_UART_STET", BCHP_FTM_UART_STET, SATFE_RegisterType_eISB},
   {"FTM_UART_HTX", BCHP_FTM_UART_HTX, SATFE_RegisterType_eISB},
   {"FTM_UART_DMASA", BCHP_FTM_UART_DMASA, SATFE_RegisterType_eISB},
   {"FTM_UART_ASSIST_HBBR", BCHP_FTM_UART_ASSIST_HBBR, SATFE_RegisterType_eISB},
   {"FTM_UART_ASSIST_SPBR", BCHP_FTM_UART_ASSIST_SPBR, SATFE_RegisterType_eISB},
   {"FTM_UART_ASSIST_STBR", BCHP_FTM_UART_ASSIST_STBR, SATFE_RegisterType_eISB},
   {"FTM_UART_ASSIST_BLVR", BCHP_FTM_UART_ASSIST_BLVR, SATFE_RegisterType_eISB},
   {"FTM_UART_ASSIST_BRR", BCHP_FTM_UART_ASSIST_BRR, SATFE_RegisterType_eISB},
   {"FTM_UART_ASSIST_PCTL1R", BCHP_FTM_UART_ASSIST_PCTL1R, SATFE_RegisterType_eISB},
   {"FTM_UART_ASSIST_PCTL2R", BCHP_FTM_UART_ASSIST_PCTL2R, SATFE_RegisterType_eISB},
   {"FTM_UART_ASSIST_OPR", BCHP_FTM_UART_ASSIST_OPR, SATFE_RegisterType_eISB},
   {"FTM_UART_ASSIST_RXMGNTR", BCHP_FTM_UART_ASSIST_RXMGNTR, SATFE_RegisterType_eISB},
   {"FTM_UART_ASSIST_RXMGNT2R", BCHP_FTM_UART_ASSIST_RXMGNT2R, SATFE_RegisterType_eISB},
   {"FTM_UART_ASSIST_CMDTRR", BCHP_FTM_UART_ASSIST_CMDTRR, SATFE_RegisterType_eISB},
   {"FTM_UART_ASSIST_SERNR", BCHP_FTM_UART_ASSIST_SERNR, SATFE_RegisterType_eISB},
   {"FTM_UART_ASSIST_SERNMGNTR", BCHP_FTM_UART_ASSIST_SERNMGNTR, SATFE_RegisterType_eISB},
   {"FTM_UART_ASSIST_SERNMGNT2R", BCHP_FTM_UART_ASSIST_SERNMGNT2R, SATFE_RegisterType_eISB},
   {"FTM_UART_ASSIST_DBGR", BCHP_FTM_UART_ASSIST_DBGR, SATFE_RegisterType_eISB},
   {"FTM_UART_CPR", BCHP_FTM_UART_CPR, SATFE_RegisterType_eISB},
   {"FTM_UART_UCV", BCHP_FTM_UART_UCV, SATFE_RegisterType_eISB},
   {"FTM_UART_CTR", BCHP_FTM_UART_CTR, SATFE_RegisterType_eISB},
   {"FTM_PHY_RST", BCHP_FTM_PHY_RST, SATFE_RegisterType_eISB},
   {"FTM_PHY_BYP", BCHP_FTM_PHY_BYP, SATFE_RegisterType_eISB},
   {"FTM_PHY_CTL", BCHP_FTM_PHY_CTL, SATFE_RegisterType_eISB},
   {"FTM_PHY_TX_CTL", BCHP_FTM_PHY_TX_CTL, SATFE_RegisterType_eISB},
   {"FTM_PHY_TX_MAN", BCHP_FTM_PHY_TX_MAN, SATFE_RegisterType_eISB},
   {"FTM_PHY_TX_DEV", BCHP_FTM_PHY_TX_DEV, SATFE_RegisterType_eISB},
   {"FTM_PHY_TX_FCW", BCHP_FTM_PHY_TX_FCW, SATFE_RegisterType_eISB},
   {"FTM_PHY_RX_FCW", BCHP_FTM_PHY_RX_FCW, SATFE_RegisterType_eISB},
   {"FTM_PHY_FILT", BCHP_FTM_PHY_FILT, SATFE_RegisterType_eISB},
   {"FTM_PHY_ADDR", BCHP_FTM_PHY_ADDR, SATFE_RegisterType_eISB},
   {"FTM_PHY_COEF", BCHP_FTM_PHY_COEF, SATFE_RegisterType_eISB},
   {"FTM_PHY_PHS_CTL", BCHP_FTM_PHY_PHS_CTL, SATFE_RegisterType_eISB},
   {"FTM_PHY_PHS_INT", BCHP_FTM_PHY_PHS_INT, SATFE_RegisterType_eISB},
   {"FTM_PHY_DC_CTL", BCHP_FTM_PHY_DC_CTL, SATFE_RegisterType_eISB},
   {"FTM_PHY_DC_INT", BCHP_FTM_PHY_DC_INT, SATFE_RegisterType_eISB},
   {"FTM_PHY_RSSI_CTL", BCHP_FTM_PHY_RSSI_CTL, SATFE_RegisterType_eISB},
   {"FTM_PHY_RSSI_INT", BCHP_FTM_PHY_RSSI_INT, SATFE_RegisterType_eISB},
   {"FTM_PHY_RSSI_THRES", BCHP_FTM_PHY_RSSI_THRES, SATFE_RegisterType_eISB},
   {"FTM_PHY_SLICE", BCHP_FTM_PHY_SLICE, SATFE_RegisterType_eISB},
   {"FTM_PHY_CORR_CTL", BCHP_FTM_PHY_CORR_CTL, SATFE_RegisterType_eISB},
   {"FTM_PHY_CORR_TIMEOUT", BCHP_FTM_PHY_CORR_TIMEOUT, SATFE_RegisterType_eISB},
   {"FTM_PHY_CORR_THRES", BCHP_FTM_PHY_CORR_THRES, SATFE_RegisterType_eISB},
   {"FTM_PHY_CORR_PEAK_QUAL", BCHP_FTM_PHY_CORR_PEAK_QUAL, SATFE_RegisterType_eISB},
   {"FTM_PHY_CORR_PREAMBLE", BCHP_FTM_PHY_CORR_PREAMBLE, SATFE_RegisterType_eISB},
   {"FTM_PHY_CORR_IRQ", BCHP_FTM_PHY_CORR_IRQ, SATFE_RegisterType_eISB},
   {"FTM_PHY_CORR_DC_INT", BCHP_FTM_PHY_CORR_DC_INT, SATFE_RegisterType_eISB},
   {"FTM_PHY_CORR_DC_FIXED", BCHP_FTM_PHY_CORR_DC_FIXED, SATFE_RegisterType_eISB},
   {"FTM_PHY_CORR_PEAK", BCHP_FTM_PHY_CORR_PEAK, SATFE_RegisterType_eISB},
   {"FTM_PHY_CORR_STATE", BCHP_FTM_PHY_CORR_STATE, SATFE_RegisterType_eISB},
   {"FTM_PHY_CORR_BYTE_CNT", BCHP_FTM_PHY_CORR_BYTE_CNT, SATFE_RegisterType_eISB},
   {"FTM_PHY_UART_CRC_CTL", BCHP_FTM_PHY_UART_CRC_CTL, SATFE_RegisterType_eISB},
   {"FTM_PHY_UART_CRC_PN", BCHP_FTM_PHY_UART_CRC_PN, SATFE_RegisterType_eISB},
   {"FTM_PHY_UART_CRC_TX", BCHP_FTM_PHY_UART_CRC_TX, SATFE_RegisterType_eISB},
   {"FTM_PHY_UART_CRC_RX", BCHP_FTM_PHY_UART_CRC_RX, SATFE_RegisterType_eISB},
   {"FTM_PHY_IRQ_MSK", BCHP_FTM_PHY_IRQ_MSK, SATFE_RegisterType_eISB},
   {"FTM_PHY_IRQ_STS", BCHP_FTM_PHY_IRQ_STS, SATFE_RegisterType_eISB},
   {"FTM_PHY_FIRQ_STS", BCHP_FTM_PHY_FIRQ_STS, SATFE_RegisterType_eISB},
   {"FTM_PHY_RSTS", BCHP_FTM_PHY_RSTS, SATFE_RegisterType_eISB},
   {"FTM_PHY_TP_CTL", BCHP_FTM_PHY_TP_CTL, SATFE_RegisterType_eISB},
   {"FTM_PHY_TP_OUT", BCHP_FTM_PHY_TP_OUT, SATFE_RegisterType_eISB},
   {"FTM_PHY_LFSR", BCHP_FTM_PHY_LFSR, SATFE_RegisterType_eISB},
   {"FTM_PHY_TIMER1", BCHP_FTM_PHY_TIMER1, SATFE_RegisterType_eISB},
   {"FTM_PHY_TIMER2", BCHP_FTM_PHY_TIMER2, SATFE_RegisterType_eISB},
   {"FTM_PHY_TIMER3", BCHP_FTM_PHY_TIMER3, SATFE_RegisterType_eISB},
   {"FTM_PHY_ASSIST_CTL", BCHP_FTM_PHY_ASSIST_CTL, SATFE_RegisterType_eISB},
   {"FTM_PHY_ASSIST_STS", BCHP_FTM_PHY_ASSIST_STS, SATFE_RegisterType_eISB},
   {"FTM_PHY_ASSIST_CNT1", BCHP_FTM_PHY_ASSIST_CNT1, SATFE_RegisterType_eISB},
   {"FTM_PHY_ASSIST_CNT2", BCHP_FTM_PHY_ASSIST_CNT2, SATFE_RegisterType_eISB},
   {"FTM_PHY_ASSIST_CNT3", BCHP_FTM_PHY_ASSIST_CNT3, SATFE_RegisterType_eISB},
   {"FTM_PHY_FIR_COEF_MEM_TM", BCHP_FTM_PHY_FIR_COEF_MEM_TM, SATFE_RegisterType_eISB},
   {"FTM_PHY_CORR2_CTL", BCHP_FTM_PHY_CORR2_CTL, SATFE_RegisterType_eISB},
   {"FTM_PHY_CORR2_THRES", BCHP_FTM_PHY_CORR2_THRES, SATFE_RegisterType_eISB},
   {"FTM_PHY_CORR2_PATTERN", BCHP_FTM_PHY_CORR2_PATTERN, SATFE_RegisterType_eISB},
   {"FTM_PHY_CORR2_MASK", BCHP_FTM_PHY_CORR2_MASK, SATFE_RegisterType_eISB},
   {"FTM_PHY_CORR2_SQR", BCHP_FTM_PHY_CORR2_SQR, SATFE_RegisterType_eISB},
   {"FTM_PHY_CORR2_DATA", BCHP_FTM_PHY_CORR2_DATA, SATFE_RegisterType_eISB},
   {"FTM_PHY_TX_RAMP", BCHP_FTM_PHY_TX_RAMP, SATFE_RegisterType_eISB},
   {"FTM_PHY_ROLL_TIMER_CTL", BCHP_FTM_PHY_ROLL_TIMER_CTL, SATFE_RegisterType_eISB},
   {"FTM_PHY_ROLL_TIMER", BCHP_FTM_PHY_ROLL_TIMER, SATFE_RegisterType_eISB},
   {"FTM_PHY_ROLL_SNPSHT", BCHP_FTM_PHY_ROLL_SNPSHT, SATFE_RegisterType_eISB},
   {"FTM_PHY_ROLL_CORRECTION", BCHP_FTM_PHY_ROLL_CORRECTION, SATFE_RegisterType_eISB},
   {"FTM_PHY_CLK_STOP", BCHP_FTM_PHY_CLK_STOP, SATFE_RegisterType_eISB},
   {"FTM_PHY_ANA_MISC", BCHP_FTM_PHY_ANA_MISC, SATFE_RegisterType_eISB},
   {"FTM_PHY_ANA0_0", BCHP_FTM_PHY_ANA0_0, SATFE_RegisterType_eISB},
   {"FTM_PHY_ANA0_1", BCHP_FTM_PHY_ANA0_1, SATFE_RegisterType_eISB},
   {"FTM_PHY_ANA1_0", BCHP_FTM_PHY_ANA1_0, SATFE_RegisterType_eISB},
   {"FTM_PHY_ANA1_1", BCHP_FTM_PHY_ANA1_1, SATFE_RegisterType_eISB},
#if (BCHP_VER >= BCHP_VER_B0)
   {"FTM_PHY_ODU_CTL", BCHP_FTM_PHY_ODU_CTL, SATFE_RegisterType_eISB},
   {"FTM_PHY_ODU_BYTE", BCHP_FTM_PHY_ODU_BYTE, SATFE_RegisterType_eISB},
   {"FTM_PHY_ANA_CTL0_0", BCHP_FTM_PHY_ANA_CTL0_0, SATFE_RegisterType_eISB},
   {"FTM_PHY_ANA_CTL0_1", BCHP_FTM_PHY_ANA_CTL0_1, SATFE_RegisterType_eISB},
   {"FTM_PHY_ANA_CTL1_0", BCHP_FTM_PHY_ANA_CTL1_0, SATFE_RegisterType_eISB},
   {"FTM_PHY_ANA_CTL1_1", BCHP_FTM_PHY_ANA_CTL1_1, SATFE_RegisterType_eISB},
#endif
   {"FTM_SKIT_SKITCTL", BCHP_FTM_SKIT_SKITCTL, SATFE_RegisterType_eISB},
   {"FTM_SKIT_TXCTL", BCHP_FTM_SKIT_TXCTL, SATFE_RegisterType_eISB},
   {"FTM_SKIT_TXDATA", BCHP_FTM_SKIT_TXDATA, SATFE_RegisterType_eISB},
   {"FTM_SKIT_TXSTATUS", BCHP_FTM_SKIT_TXSTATUS, SATFE_RegisterType_eISB},
   {"FTM_SKIT_RXCTL", BCHP_FTM_SKIT_RXCTL, SATFE_RegisterType_eISB},
   {"FTM_SKIT_RXPAR", BCHP_FTM_SKIT_RXPAR, SATFE_RegisterType_eISB},
   {"FTM_SKIT_RXMASK", BCHP_FTM_SKIT_RXMASK, SATFE_RegisterType_eISB},
   {"FTM_SKIT_RXNCOFCW", BCHP_FTM_SKIT_RXNCOFCW, SATFE_RegisterType_eISB},
   {"FTM_SKIT_RXSTATUS", BCHP_FTM_SKIT_RXSTATUS, SATFE_RegisterType_eISB},
   {"FTM_SKIT_RXDATA", BCHP_FTM_SKIT_RXDATA, SATFE_RegisterType_eISB},
   {"FTM_SKIT_RXDCOFFSET", BCHP_FTM_SKIT_RXDCOFFSET, SATFE_RegisterType_eISB},
   {"FTM_INTR2_CPU_STATUS", BCHP_FTM_INTR2_CPU_STATUS, SATFE_RegisterType_eISB},
   {"FTM_INTR2_CPU_SET", BCHP_FTM_INTR2_CPU_SET, SATFE_RegisterType_eISB},
   {"FTM_INTR2_CPU_CLEAR", BCHP_FTM_INTR2_CPU_CLEAR, SATFE_RegisterType_eISB},
   {"FTM_INTR2_CPU_MASK_STATUS", BCHP_FTM_INTR2_CPU_MASK_STATUS, SATFE_RegisterType_eISB},
   {"FTM_INTR2_CPU_MASK_SET", BCHP_FTM_INTR2_CPU_MASK_SET, SATFE_RegisterType_eISB},
   {"FTM_INTR2_CPU_MASK_CLEAR", BCHP_FTM_INTR2_CPU_MASK_CLEAR, SATFE_RegisterType_eISB},
   {"FTM_INTR2_PCI_STATUS", BCHP_FTM_INTR2_PCI_STATUS, SATFE_RegisterType_eISB},
   {"FTM_INTR2_PCI_SET", BCHP_FTM_INTR2_PCI_SET, SATFE_RegisterType_eISB},
   {"FTM_INTR2_PCI_CLEAR", BCHP_FTM_INTR2_PCI_CLEAR, SATFE_RegisterType_eISB},
   {"FTM_INTR2_PCI_MASK_STATUS", BCHP_FTM_INTR2_PCI_MASK_STATUS, SATFE_RegisterType_eISB},
   {"FTM_INTR2_PCI_MASK_SET", BCHP_FTM_INTR2_PCI_MASK_SET, SATFE_RegisterType_eISB},
   {"FTM_INTR2_PCI_MASK_CLEAR", BCHP_FTM_INTR2_PCI_MASK_CLEAR, SATFE_RegisterType_eISB},
   {NULL, 0, 0}
};


#define Desc_g3_CONFIG_STUFF_BYTES "Number of null bytes to insert in each frame\n"
#define Desc_g3_CONFIG_AGC_CTL "AGC configuration options:\n" \
   "   bit 0: meter mode: 0=disable, 1=enable (higher tracking AGC bandwidth)\n" \
   "   bits [7:1]: reserved"
#define Desc_g3_CONFIG_TUNER_CTL "Internal tuner control options:\n" \
   "   bit 0: PGA or LNA select: 1=enable PGA, 0=enable LNA\n" \
   "   bit 1: bypass internal LNA: 1=bypass LNA, 0=normal mode\n" \
   "   bit 2: reserved\n" \
   "   bit 3: tuner filter programming: 0=auto, 1=manual (LPF cutoff frequency will be set to the value specified in TUNER_FILTER_OVERRIDE config parameter) \n" \
   "   bit 4: bypass DFT freq estimate: 1=bypass, 0=normal\n" \
   "   bit 5: preserve commanded tuner LO frequency: 0=disable, 1=enable\n" \
   "   bit 6: reserved\n" \
   "   bit 7: LPF calibration status: 0=successful, 1=failed (failsafe value applied)"
#define Desc_g3_CONFIG_PLC_CTL "PLC configuration options:\n" \
   "   bit 0: reserved\n" \
   "   bit 1: use PLC tracking bandwidth optimized for AWGN: 0=disable, 1=enable\n" \
   "   bit 2: 0=PLC acquisition bandwidth is determined by the AP, 1=use tracking PLC bandwidth manually specified by plc_alt_acq_bw and plc_alt_acq_damp configuration parameters\n" \
   "   bit 3: 0=PLC tracking bandwidth is determined by the AP, 1=use tracking PLC bandwidth manually specified by plc_alt_trk_bw and plc_alt_trk_damp configuration parameters\n" \
   "   bits [7:4]: reserved"
#define Desc_g3_CONFIG_LDPC_CTL "LDPC acquisition control options:\n" \
   "   bit 0: pilot PLL select: 0=automatic, 1=acq_ctl.13 from TUNE_ACQUIRE HAB command\n" \
   "   bit 1: PSL: 0=enable, 1=disable\n" \
   "   bits [6:2]: reserved\n" \
   "   bit 7: (READ ONLY) FEC frame size: 0=64800 bits, 1=16200 bits"
#define Desc_g3_CONFIG_TURBO_CTL "Turbo acquisition control options:\n" \
   "   bit 0: 0=reacquire on FEC not locked, 1=reacquire on HP not locked\n" \
   "   bits [7:1]: reserved"
#define Desc_g3_CONFIG_PLC_ALT_ACQ_BW "Alternate acquisition PLC bandwidth in units of Hz.  This bandwidth applies when PLC_CTL bit 2 is set."
#define Desc_g3_CONFIG_PLC_ALT_ACQ_DAMP "Alternate acquisition PLC damping factor scaled 2^3 (e.g. damping factor of 1.0 is 0x08).  This damping factor only applies when PLC_CTL bit 2 is set."
#define Desc_g3_CONFIG_PLC_ALT_TRK_BW "Alternate tracking PLC bandwidth in units of Hz.  This bandwidth applies when PLC_CTL bit 3 is set."
#define Desc_g3_CONFIG_PLC_ALT_TRK_DAMP "Alternate tracking PLC damping factor scaled 2^3 (e.g. damping factor of 1.0 is 0x08).  This damping factor only applies when PLC_CTL bit 3 is set."
#define Desc_g3_CONFIG_BLIND_SCAN_MODES "Indicates which modes will be considered in the blind scan (this configuration parameter applies when blind scan is specified in TUNE_ACQUIRE HAB command).\n" \
   "   bit 0: DVB-S\n" \
   "   bit 1: Turbo\n" \
   "   bit 2: LDPC\n" \
   "   bit 3: DTV\n" \
   "   bit 4: DCII"
#define Desc_g3_CONFIG_DTV_SCAN_CODE_RATES "Selects the DTV code rates that are considered in the scan:\n" \
   "   bit 0: DTV 1/2\n" \
   "   bit 1: DTV 2/3\n" \
   "   bit 2: DTV 6/7"
#define Desc_g3_CONFIG_DVB_SCAN_CODE_RATES "Selects the DVB-S code rates that are considered in the scan:\n" \
   "   bit 0: DVB 1/2\n" \
   "   bit 1: DVB 2/3\n" \
   "   bit 2: DVB 3/4\n" \
   "   bit 3: DVB 5/6\n" \
   "   bit 4: DVB 7/8"
#define Desc_g3_CONFIG_DCII_SCAN_CODE_RATES "Selects the DCII code rates that are considered in the scan:\n" \
   "   bit 0: DCII 1/2\n" \
   "   bit 1: DCII 2/3\n" \
   "   bit 2: DCII 3/4\n" \
   "   bit 3: DCII 5/6\n" \
   "   bit 4: DCII 7/8\n" \
   "   bit 5: DCII 5/11\n" \
   "   bit 6: DCII 3/5\n" \
   "   bit 7: DCII 4/5"
#define Desc_g3_CONFIG_TURBO_SCAN_MODES "Selects the Turbo modes that are considered in the scan:\n" \
   "   bit 0: QPSK 1/2\n" \
   "   bit 1: QPSK 2/3\n" \
   "   bit 2: QPSK 3/4\n" \
   "   bit 3: QPSK 5/6\n" \
   "   bit 4: QPSK 7/8\n" \
   "   bit 5: 8PSK 2/3\n" \
   "   bit 6: 8PSK 3/4\n" \
   "   bit 7: 8PSK 4/5\n" \
   "   bit 8: 8PSK 5/6\n" \
   "   bit 9: 8PSK 8/9"
#define Desc_g3_LDPC_SCAN_MODES "Selects the LDPC modes that are considered in the scan:\n" \
   "   bit 0: QPSK 1/2\n" \
   "   bit 1: QPSK 3/5\n" \
   "   bit 2: QPSK 2/3\n" \
   "   bit 3: QPSK 3/4\n" \
   "   bit 4: QPSK 4/5\n" \
   "   bit 5: QPSK 5/6\n" \
   "   bit 6: QPSK 8/9\n" \
   "   bit 7: QPSK 9/10\n" \
   "   bit 8: 8PSK 3/5\n" \
   "   bit 9: 8PSK 2/3\n" \
   "   bit 10: 8PSK 3/4\n" \
   "   bit 11: 8PSK 5/6\n" \
   "   bit 12: 8PSK 8/9\n" \
   "   bit 13: 8PSK 9/10"
#define Desc_g3_CONFIG_TUNER_FILTER_OVERRIDE "Tuner low pass baseband filter cutoff frequency in MHz when TUNER_CTL bit 3 is set.  Changes in this configuration parameter will take affect on the next TUNE_ACQUIRE HAB command."
#define Desc_g3_CONFIG_SEARCH_RANGE "Search range in Hz.  The AP will attempt to lock the signal within search_range from the tuning frequency"
#define Desc_g3_CONFIG_STATUS_INDICATOR "Real time status bits:\n" \
   "   bit 0: rain fade\n" \
   "   bit 1: freq drift\n" \
   "   bit 2: SNR threshold 2\n" \
   "   bit 3: SNR threshold 1"
#define Desc_g3_CONFIG_RAIN_FADE_THRESHOLD "SNR drop threshold for the rain fade indicator status in units of 1/8 dB SNR"
#define Desc_g3_CONFIG_RAIN_FADE_WINDOW "Time window for rain fade indicator status in units of 100 msecs"
#define Desc_g3_CONFIG_FREQ_DRIFT_THRESHOLD "Carrier offset threshold in Hz for frequency drift indicator status"
#define Desc_g3_CONFIG_DFT_RANGE_START "Starting bin for DFT engine"
#define Desc_g3_CONFIG_DFT_RANGE_END "Ending bin for DFT engine."
#define Desc_g3_CONFIG_REACQ_CTL "Reacquisition Control options:\n" \
   "   bit 0: enable retune on reacquire\n" \
   "   bit 1: force reacquisition now\n" \
   "   bit 2: force retune on next reacquisition\n" \
   "   bit 3: dont keep lock if carrier freq drift outside of search range\n"
#define Desc_g3_CONFIG_MISC_CTL "Miscellaneous acquisition control options:\n" \
   "   bit 0: 1=disable Fs smart tune\n" \
   "   bit 1: 1=verify timing loop is locked before enabling HP\n" \
   "   bit 2: (wideband tuners only) 1=disable DCO notch filters\n" \
   "   bit 5: final non-decimating filter setting: 0=auto, 1=specified by bits 7:6\n" \
   "   bits 7:6: final non-decimating filter configuration: 00=quarterband, 01=thirdband, 10=halfband, 11=sharpened halfband\n"
#define Desc_g3_CONFIG_TUNER_FILCAL_UPPER "Upper filter calibration threshold percentage of half-tone scaled by 100\n"
#define Desc_g3_CONFIG_TUNER_FILCAL_LOWER "Lower filter calibration threshold percentage of half-tone scaled by 100\n"
#define Desc_g3_CONFIG_AMBIENT_TEMP "Ambient temperature in Celsius used in tuner Vc adjustment, 0=disable\n"
#define Desc_g3_CONFIG_ACM_MAX_MODE "Highest BAST_Mode in the stream\n"
#define Desc_g3_TUNER_AGC_THRESH "AGC clip threshold:\n" \
   "   [31:16] Clip threshold for BB AGC\n" \
   "   [15:00] Clip threshold for LNA AGC\n"
#define Desc_g3_CONFIG_TUNER_DAISY_GAIN "Daisy output gain:\n" \
   "   0(b'00): 0dB\n" \
   "   1(b'01): 2dB\n" \
   "   2(b'10): 4dB\n" \
   "   3(b'11): 6dB\n"
#define Desc_g3_CONFIG_TUNER_AGC_WIN_LEN "AGC window length:\n" \
   "   [31:16] Window size for for BB AGC\n" \
   "   [15:00] Window size for LNA AGC\n"
#define Desc_g3_CONFIG_TUNER_AGC_AMP_THRESH "AGC amplitude threshold:\n" \
   "   [12:8] Amplitude threshold for BB AGC\n" \
   "   [4:0]  Amplitude threshold for LNA AGC\n"
#define Desc_g3_CONFIG_TUNER_AGC_LOOP_COEFF "AGC loop coefficient:\n" \
   "   [12:8] Loop coefficient for BB AGC\n" \
   "   [4:0]  Loop coefficient for LNA AGC\n"


SATFE_ConfigParam SATFE_g3_ConfigParamMap[] =
{
   {"xtal_freq", BAST_G3_CONFIG_XTAL_FREQ, BAST_G3_CONFIG_LEN_XTAL_FREQ, SATFE_ConfigParamType_eReadOnly, NULL},
   {"rain_fade_threshold", BAST_G3_CONFIG_RAIN_FADE_THRESHOLD, BAST_G3_CONFIG_LEN_RAIN_FADE_THRESHOLD, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_RAIN_FADE_THRESHOLD},
   {"rain_fade_window", BAST_G3_CONFIG_RAIN_FADE_WINDOW, BAST_G3_CONFIG_LEN_RAIN_FADE_WINDOW, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_RAIN_FADE_WINDOW},
   {"freq_drift_threshold", BAST_G3_CONFIG_FREQ_DRIFT_THRESHOLD, BAST_G3_CONFIG_LEN_FREQ_DRIFT_THRESHOLD, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_FREQ_DRIFT_THRESHOLD},
   {"stuff_bytes", BAST_G3_CONFIG_STUFF_BYTES, BAST_G3_CONFIG_LEN_STUFF_BYTES, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_STUFF_BYTES},
   {"acq_time", BAST_G3_CONFIG_ACQ_TIME, BAST_G3_CONFIG_LEN_ACQ_TIME, SATFE_ConfigParamType_eReadOnly, NULL},
   {"agc_ctl", BAST_G3_CONFIG_AGC_CTL, BAST_G3_CONFIG_LEN_AGC_CTL, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_AGC_CTL},
   {"tuner_ctl", BAST_G3_CONFIG_TUNER_CTL, BAST_G3_CONFIG_LEN_TUNER_CTL, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_TUNER_CTL},
   {"plc_ctl", BAST_G3_CONFIG_PLC_CTL, BAST_G3_CONFIG_LEN_PLC_CTL, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_PLC_CTL},
   {"ldpc_ctl", BAST_G3_CONFIG_LDPC_CTL, BAST_G3_CONFIG_LEN_LDPC_CTL, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_LDPC_CTL},
   {"turbo_ctl", BAST_G3_CONFIG_TURBO_CTL, BAST_G3_CONFIG_LEN_TURBO_CTL, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_TURBO_CTL},
   {"plc_alt_acq_bw", BAST_G3_CONFIG_PLC_ALT_ACQ_BW, BAST_G3_CONFIG_LEN_PLC_ALT_ACQ_BW, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_PLC_ALT_ACQ_BW},
   {"plc_alt_acq_damp", BAST_G3_CONFIG_PLC_ALT_ACQ_DAMP, BAST_G3_CONFIG_LEN_PLC_ALT_ACQ_DAMP, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_PLC_ALT_ACQ_DAMP},
   {"plc_alt_trk_bw", BAST_G3_CONFIG_PLC_ALT_TRK_BW, BAST_G3_CONFIG_LEN_PLC_ALT_TRK_BW, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_PLC_ALT_TRK_BW},
   {"plc_alt_trk_damp", BAST_G3_CONFIG_PLC_ALT_TRK_DAMP, BAST_G3_CONFIG_LEN_PLC_ALT_TRK_DAMP, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_PLC_ALT_TRK_DAMP},
   {"blind_scan_modes", BAST_G3_CONFIG_BLIND_SCAN_MODES, BAST_G3_CONFIG_LEN_BLIND_SCAN_MODES, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_BLIND_SCAN_MODES},
   {"dtv_scan_code_rates", BAST_G3_CONFIG_DTV_SCAN_CODE_RATES, BAST_G3_CONFIG_LEN_DTV_SCAN_CODE_RATES, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_DTV_SCAN_CODE_RATES},
   {"dvb_scan_code_rates", BAST_G3_CONFIG_DVB_SCAN_CODE_RATES, BAST_G3_CONFIG_LEN_DVB_SCAN_CODE_RATES, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_DVB_SCAN_CODE_RATES},
   {"dcii_scan_code_rates", BAST_G3_CONFIG_DCII_SCAN_CODE_RATES, BAST_G3_CONFIG_LEN_DCII_SCAN_CODE_RATES, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_DCII_SCAN_CODE_RATES},
   {"turbo_scan_modes", BAST_G3_CONFIG_TURBO_SCAN_MODES, BAST_G3_CONFIG_LEN_TURBO_SCAN_MODES, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_TURBO_SCAN_MODES},
   {"ldpc_scan_modes", BAST_G3_CONFIG_LDPC_SCAN_MODES, BAST_G3_CONFIG_LEN_LDPC_SCAN_MODES, SATFE_ConfigParamType_eDefault, Desc_g3_LDPC_SCAN_MODES},
   {"tuner_filter_override", BAST_G3_CONFIG_TUNER_FILTER_OVERRIDE, BAST_G3_CONFIG_LEN_TUNER_FILTER_OVERRIDE, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_TUNER_FILTER_OVERRIDE},
   {"freq_estimate_status", BAST_G3_CONFIG_FREQ_ESTIMATE_STATUS, BAST_G3_CONFIG_LEN_FREQ_ESTIMATE_STATUS, SATFE_ConfigParamType_eDefault, NULL},
   {"if_step_save", BAST_G3_CONFIG_IF_STEP_SAVE, BAST_G3_CONFIG_LEN_IF_STEP_SAVE, SATFE_ConfigParamType_eReadOnly, NULL},
   {"status_indicator", BAST_G3_CONFIG_STATUS_INDICATOR, BAST_G3_CONFIG_LEN_STATUS_INDICATOR, SATFE_ConfigParamType_eReadOnly, Desc_g3_CONFIG_STATUS_INDICATOR},
   {"dft_range_start", BAST_G3_CONFIG_DFT_RANGE_START, BAST_G3_CONFIG_LEN_DFT_RANGE_START, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_DFT_RANGE_START},
   {"dft_range_end",  BAST_G3_CONFIG_DFT_RANGE_END, BAST_G3_CONFIG_LEN_DFT_RANGE_END, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_DFT_RANGE_END},
   {"ftm_tx_power", BAST_G3_CONFIG_FTM_TX_POWER, BAST_G3_CONFIG_LEN_FTM_TX_POWER, SATFE_ConfigParamType_eDefault, NULL},
   {"ftm_ch_select", BAST_G3_CONFIG_FTM_CH_SELECT, BAST_G3_CONFIG_LEN_FTM_CH_SELECT, SATFE_ConfigParamType_eDefault, NULL},
   {"reacq_ctl", BAST_G3_CONFIG_REACQ_CTL, BAST_G3_CONFIG_LEN_REACQ_CTL, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_REACQ_CTL},
   {"misc_ctl", BAST_G3_CONFIG_MISC_CTL, BAST_G3_CONFIG_LEN_MISC_CTL, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_MISC_CTL},
   {"tuner_filcal_upper", BAST_G3_CONFIG_TUNER_FILCAL_UPPER, BAST_G3_CONFIG_LEN_TUNER_FILCAL_UPPER, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_TUNER_FILCAL_UPPER},
   {"tuner_filcal_lower", BAST_G3_CONFIG_TUNER_FILCAL_LOWER, BAST_G3_CONFIG_LEN_TUNER_FILCAL_LOWER, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_TUNER_FILCAL_LOWER},
   {"ambient_temp", BAST_G3_CONFIG_AMBIENT_TEMP, BAST_G3_CONFIG_LEN_AMBIENT_TEMP, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_AMBIENT_TEMP},
   {"debug1", BAST_G3_CONFIG_DEBUG1, BAST_G3_CONFIG_LEN_DEBUG1, SATFE_ConfigParamType_eDefault, NULL},
   {"debug2", BAST_G3_CONFIG_DEBUG2, BAST_G3_CONFIG_LEN_DEBUG2, SATFE_ConfigParamType_eDefault, NULL},
   {"debug3", BAST_G3_CONFIG_DEBUG3, BAST_G3_CONFIG_LEN_DEBUG3, SATFE_ConfigParamType_eDefault, NULL},
   {"reacq_cause", BAST_G3_CONFIG_REACQ_CAUSE, BAST_G3_CONFIG_LEN_REACQ_CAUSE, SATFE_ConfigParamType_eDefault, NULL},
   {"acm_max_mode", BAST_G3_CONFIG_ACM_MAX_MODE, BAST_G3_CONFIG_LEN_ACM_MAX_MODE, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_ACM_MAX_MODE},
   {"agc_thresh", BAST_G3_CONFIG_TUNER_AGC_THRESH, BAST_G3_CONFIG_LEN_TUNER_AGC_THRESH, SATFE_ConfigParamType_eDefault, Desc_g3_TUNER_AGC_THRESH},
   {"daisy_gain", BAST_G3_CONFIG_TUNER_DAISY_GAIN, BAST_G3_CONFIG_LEN_TUNER_DAISY_GAIN, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_TUNER_DAISY_GAIN},
   {"agc_win_len", BAST_G3_CONFIG_TUNER_AGC_WIN_LEN, BAST_G3_CONFIG_LEN_TUNER_AGC_WIN_LEN, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_TUNER_AGC_WIN_LEN},
   {"agc_amp_thresh", BAST_G3_CONFIG_TUNER_AGC_AMP_THRESH, BAST_G3_CONFIG_LEN_TUNER_AGC_AMP_THRESH, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_TUNER_AGC_AMP_THRESH},
   {"agc_loop_coeff", BAST_G3_CONFIG_TUNER_AGC_LOOP_COEFF, BAST_G3_CONFIG_LEN_TUNER_AGC_LOOP_COEFF, SATFE_ConfigParamType_eDefault, Desc_g3_CONFIG_TUNER_AGC_LOOP_COEFF},
   {NULL, 0, 0, 0, NULL}
};


 /******************************************************************************
 SATFE_g3_InitHandle()
******************************************************************************/
void SATFE_g3_InitHandle(SATFE_Chip *pChip)
{
   pChip->regMap = SATFE_g3_RegisterMap;
   pChip->configParamMap = SATFE_g3_ConfigParamMap;
   pChip->pChipCommandSet = SATFE_g3_CommandSet;
   pChip->chipFunctTable = &SATFE_g3_Functs;
}


/******************************************************************************
 SATFE_g3_GetFreqFromString()
******************************************************************************/
bool SATFE_g3_GetFreqFromString(char *str, uint32_t *pHz)
{
   float f;
   uint32_t hz;

   f = (float)atof(str);
   if (f < 3700)
      hz = (uint32_t)(f * 1000000);
   else
      hz = (uint32_t)f;

   if ((hz < 250000000UL) || (hz > 3650000000UL))
   {
      printf("Tuning frequency out of range\n");
      return false;
   }
   *pHz = hz;
   return true;
}


/******************************************************************************
 SATFE_g3_IsSpinv()
******************************************************************************/
bool SATFE_g3_IsSpinv(SATFE_Chip *pChip, BAST_ChannelStatus *pStatus)
{
   BSTD_UNUSED(pChip);
   return (pStatus->modeStatus.ldpc.hpstatus & 0x00000080) ? true : false;
}


/******************************************************************************
 SATFE_g3_IsPilotOn()
******************************************************************************/
bool SATFE_g3_IsPilotOn(SATFE_Chip *pChip, BAST_ChannelStatus *pStatus)
{
   BSTD_UNUSED(pChip);
   return (pStatus->modeStatus.ldpc.hpstatus & 1) ? true : false;
}


/******************************************************************************
 SATFE_g3_Command_help()
******************************************************************************/
bool SATFE_g3_Command_help(SATFE_Chip *pChip, int argc, char **argv)
{
   BSTD_UNUSED(pChip);
   BSTD_UNUSED(argc);
   BSTD_UNUSED(argv);

   printf("CHIP-SPECIFIC COMMANDS:\n");
   printf("   read_tuner_ind, write_tuner_ind, irq_count, trace, debug_snore, tuner_status\n");
   printf("\n");
   return true;
}


/******************************************************************************
 SATFE_g3_Command_read_tuner_ind()
******************************************************************************/
bool SATFE_g3_Command_read_tuner_ind(struct SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   BAST_TunerIndirectRegGroup group;
   uint32_t val;
   uint8_t addr;

   if (argc != 3)
   {
      SATFE_PrintDescription1("read_tuner_ind", "read_tuner_ind [register_group] [addr]", "Reads a tuner indirect register.",
         "register_group = {predco_i, predco_q, postdco_i, postdco_q, rfagc, bbagc}", false);
      SATFE_PrintDescription2("addr = indirect register address", true);
      return true;
   }

   if (!strcmp(argv[1], "predco_i"))
      group = BAST_TunerIndirectRegGroup_ePreDcoI;
   else if (!strcmp(argv[1], "predco_q"))
      group = BAST_TunerIndirectRegGroup_ePreDcoQ;
   else if (!strcmp(argv[1], "postdco_i"))
      group = BAST_TunerIndirectRegGroup_ePostDcoI;
   else if (!strcmp(argv[1], "postdco_q"))
      group = BAST_TunerIndirectRegGroup_ePostDcoQ;
   else if (!strcmp(argv[1], "rfagc"))
      group = BAST_TunerIndirectRegGroup_eRfagc;
   else if (!strcmp(argv[1], "bbagc"))
      group = BAST_TunerIndirectRegGroup_eBbagc;
   else
   {
      printf("invalid register_group (%s)\n", argv[1]);
      return false;
   }

   if (SATFE_GetU8FromString(argv[2], &addr) == false)
      return false;
   if (addr > 13)
   {
      printf("invalid address (%s)\n", argv[2]);
      return false;
   }

   BKNI_EnterCriticalSection();
   retCode = BAST_g3_P_TunerIndirectRead_isrsafe(pChip->hAstChannel[pChip->currChannel], group, addr, &val);
   BKNI_LeaveCriticalSection();

   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_g3_P_TunerIndirectRead_isrsafe() error 0x%X\n", retCode);
      return false;
   }
   printf("tuner ind read %s addr %s = 0x%08X\n", argv[1], argv[2], val);

   return true;
}


/******************************************************************************
 SATFE_g3_Command_write_tuner_ind()
******************************************************************************/
bool SATFE_g3_Command_write_tuner_ind(struct SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   BAST_TunerIndirectRegGroup group;
   uint32_t val;
   uint8_t addr;

   if (argc != 4)
   {
      SATFE_PrintDescription1("write_tuner_ind", "write_tuner_ind [register_group] [addr] [value]", "Writes to a tuner indirect register.",
         "register_group = {predco_i, predco_q, postdco_i, postdco_q, rfagc, bbagc}", false);
      SATFE_PrintDescription2("addr = indirect register address", false);
      SATFE_PrintDescription2("value = register value to write", true);
      return true;
   }

   if (!strcmp(argv[1], "predco_i"))
      group = BAST_TunerIndirectRegGroup_ePreDcoI;
   else if (!strcmp(argv[1], "predco_q"))
      group = BAST_TunerIndirectRegGroup_ePreDcoQ;
   else if (!strcmp(argv[1], "postdco_i"))
      group = BAST_TunerIndirectRegGroup_ePostDcoI;
   else if (!strcmp(argv[1], "postdco_q"))
      group = BAST_TunerIndirectRegGroup_ePostDcoQ;
   else if (!strcmp(argv[1], "rfagc"))
      group = BAST_TunerIndirectRegGroup_eRfagc;
   else if (!strcmp(argv[1], "bbagc"))
      group = BAST_TunerIndirectRegGroup_eBbagc;
   else
   {
      printf("invalid register_group (%s)\n", argv[1]);
      return false;
   }

   if (SATFE_GetU8FromString(argv[2], &addr) == false)
      return false;
   if (addr > 13)
   {
      printf("invalid address (%s)\n", argv[2]);
      return false;
   }

   if (SATFE_GetU32FromString(argv[3], &val) == false)
      return false;

   BKNI_EnterCriticalSection();
   retCode = BAST_g3_P_TunerIndirectWrite_isrsafe(pChip->hAstChannel[pChip->currChannel], group, addr, val);
   BKNI_LeaveCriticalSection();

   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_g3_P_TunerIndirectWrite_isrsafe() error 0x%X\n", retCode);
      return false;
   }

   return true;
}


/******************************************************************************
 SATFE_g3_GetDebugModuleNames()
******************************************************************************/
void SATFE_g3_GetDebugModuleNames(struct SATFE_Chip *pChip, int *numModules, char **moduleNames[])
{
   BSTD_UNUSED(pChip);

   *moduleNames = SATFE_g3_DebugModules;
   *numModules = SATFE_g3_NumDebugModules;
}


/******************************************************************************
 SATFE_g3_Command_irq_count()
******************************************************************************/
bool SATFE_g3_Command_irq_count(SATFE_Chip *pChip, int argc, char **argv)
{
   static char *SATFE_IRQ_STR[BAST_g3_MaxIntID] =
   {
      "Lock",
      "NotLock",
      "BaudTimer",
      "BerTimer",
      "GenTimer1",
      "GenTimer2",
      "GenTimer3",
      "DiseqcTimer1",
      "DiseqcTimer2",
      "DiseqcVoltageGtHiThresh",
      "DiseqcVoltageLtLoThresh",
      "DiseqcDone",
      "Hp",
      "Mi2c",
      "TurboLock",
      "TurboNotLock",
      "AfecLock",
      "AfecNotLock"
   };

   uint32_t count;
   int i;
#ifdef SATFE_TIME_ISR
   extern uint32_t maxIsrTime;
#endif

   if ((argc != 1) && (argc != 2))
   {
      SATFE_PrintDescription1("irq_count", "irq_count <reset>",
         "Display front end interrupt counters if no parameter supplied.  Optionally resets the interrupt counters with <reset> parameter.",
         "reset = resets all interrupt counters", true);
   }
   else if (argc == 1)
   {
      /* display irq counters */
      for (i = 0; i < BAST_g3_MaxIntID; i++)
      {
         if (BAST_g3_GetInterruptCount(pChip->hAstChannel[pChip->currChannel], i, &count) == BERR_SUCCESS)
            printf("%10d : %s irq\n", count, SATFE_IRQ_STR[i]);
         else
            printf("Unable to get %s interrupt counter\n", SATFE_IRQ_STR[i]);
      }

#ifdef SATFE_TIME_ISR
      printf("max isr time = %u ticks (%f usecs)\n", maxIsrTime, (double)maxIsrTime * (double)(INPUT_CLK_CYCLES_PER_COUNT_REG_INCR * 1000000.0) / (double)CPU_CLOCK_RATE);
#endif
   }
   else if (!strcmp(argv[1], "reset"))
   {
      BAST_g3_ResetInterruptCounters(pChip->hAstChannel[pChip->currChannel]);
#ifdef SATFE_TIME_ISR
      maxIsrTime = 0;
#endif
   }
   else
   {
      printf("syntax error\n");
      return false;
   }

   return true;
}


/******************************************************************************
 SATFE_g3_Command_trace()
******************************************************************************/
bool SATFE_g3_Command_trace(SATFE_Chip *pChip, int argc, char **argv)
{
   static char *SATFE_TRACE_EVENT[BAST_TraceEvent_eMax] =
   {
      "TuneMixPllLock",
      "DcoConverge",
      "TunerLpfCalDone",
      "TuneDone",
      "FreqEstDone",
      "RetuneDone",
      "StartHp",
      "RcvrLocked",
      "StartTracking",
      "InitialLock",
      "StableLock",
      "Reacquire",
   };

   uint32_t buffer[BAST_TraceEvent_eMax];
   int i;

   BSTD_UNUSED(argv);

   if (argc != 1)
   {
      SATFE_PrintDescription1("trace", "trace",
         "Display the trace buffer.",
         "none", true);
   }
   else
   {
      BAST_g3_GetTraceBuffer(pChip->hAstChannel[pChip->currChannel], buffer);
      for (i = 0; i < BAST_TraceEvent_eMax; i++)
      {
         printf("%16s: %10u %10u\n",
                SATFE_TRACE_EVENT[i],
                buffer[i],
                buffer[i] ? (((i > 0) ? (buffer[i] - buffer[i-1]) : buffer[0])) : 0);
      }
      printf("\n");
   }
   return true;
}


/******************************************************************************
 SATFE_g3_Command_debug_snore()
******************************************************************************/
bool SATFE_g3_Command_debug_snore(SATFE_Chip *pChip, int argc, char **argv)
{
   uint32_t sig_power, tot_power;
   bool bLocked;

   BSTD_UNUSED(argv);

   if (argc != 1)
   {
      SATFE_PrintDescription1("debug_snore", "debug_snore",
         "Display SNORE total_power and signal_power",
         "none", true);
   }
   else
   {
      while (SATFE_Platform_GetChar(false) <= 0)
      {
         BAST_GetLockStatus(pChip->hAstChannel[pChip->currChannel], &bLocked);
         BKNI_EnterCriticalSection();
         BAST_g3_P_ToggleBit_isrsafe(pChip->hAstChannel[pChip->currChannel], BCHP_SDS_SNR_SNORECTL, 0x40);
         BAST_g3_P_ReadRegister_isrsafe(pChip->hAstChannel[pChip->currChannel], BCHP_SDS_SNR_SNORESP, &sig_power);
         BAST_g3_P_ReadRegister_isrsafe(pChip->hAstChannel[pChip->currChannel], BCHP_SDS_SNR_SNORETP, &tot_power);
         BKNI_LeaveCriticalSection();
         printf("%s: 0x%08X, 0x%08X\n", bLocked ? "locked" : "not locked", sig_power, tot_power);
         BKNI_Sleep(1);
      }
   }
   return true;

}


/******************************************************************************
 SATFE_g3_Command_tuner_status()
******************************************************************************/
bool SATFE_g3_Command_tuner_status(SATFE_Chip *pChip, int argc, char **argv)
{
   extern BERR_Code BAST_g3_P_TunerGetFddfs_isrsafe(BAST_ChannelHandle h, uint32_t *Fddfs);
   extern BERR_Code BAST_g3_P_TunerGetLoDivider_isrsafe(BAST_ChannelHandle h, uint32_t *tunerLoDivider);
   extern BERR_Code BAST_g3_P_TunerGetActualLOFreq_isrsafe(BAST_ChannelHandle h, uint32_t *pFreq);

   BERR_Code retCode;
   uint32_t Fddfs, val32, Fc;
   float loDivider;
   BAST_ChannelStatus status;

   BSTD_UNUSED(argv);

   if (argc != 1)
   {
      SATFE_PrintDescription1("tuner_status", "tuner_status",
         "Display tuner status for debugging",
         "none", true);
      return true;
   }

   SATFE_MUTEX(retCode = BAST_g3_P_TunerGetFddfs_isrsafe(pChip->hAstChannel[pChip->currChannel], &Fddfs));
   SATFE_CHECK_RETCODE("BAST_g3_P_TunerGetFddfs_isrsafe", retCode);
   printf("Fddfs                = %u Hz\n", Fddfs);

   SATFE_MUTEX(retCode = BAST_g3_P_TunerGetLoDivider_isrsafe(pChip->hAstChannel[pChip->currChannel], &val32));
   SATFE_CHECK_RETCODE("BAST_g3_P_TunerGetLoDivider_isrsafe", retCode);
   loDivider = val32 / 64.0;
   printf("LO divider           = %f\n", loDivider);

   SATFE_MUTEX(retCode = BAST_GetChannelStatus(pChip->hAstChannel[pChip->currChannel], &status));
   SATFE_CHECK_RETCODE("BAST_GetChannelStatus", retCode);
   printf("commanded tuner freq = %u Hz\n", status.tunerFreq);

   SATFE_MUTEX(BAST_g3_P_TunerGetActualLOFreq_isrsafe(pChip->hAstChannel[pChip->currChannel], &Fc));
   printf("actual tuner freq    = %u Hz\n", Fc);

   printf("tuner filter         = %u Hz\n", status.tunerFilter);
   return true;
}


/******************************************************************************
 SATFE_g3_GetAdcVoltage()
******************************************************************************/
bool SATFE_g3_GetAdcVoltage(struct SATFE_Chip *pChip, uint8_t voltage, uint8_t currChannel, uint16_t *lnbVoltage)
{
	/* mapping for adc to lnb voltage */
   uint16_t adc_voltage[256][2] = {
      {0, 0},
      {125, 125},
      {250, 250},
      {375, 375},
      {500, 500},
      {625, 625},
      {750, 750},
      {875, 875},
      {1000, 1000},
      {1125, 1125},
      {1250, 1250},
      {1375, 1375},
      {1500, 1500},
      {1625, 1625},
      {1750, 1750},
      {1875, 1875},
      {2000, 2000},
      {2125, 2125},
      {2250, 2250},
      {2375, 2375},
      {2500, 2500},
      {2625, 2625},
      {2750, 2750},
      {2875, 2875},
      {3000, 3000},
      {3125, 3125},
      {3250, 3250},
      {3375, 3375},
      {3500, 3500},
      {3625, 3625},
      {3750, 3750},
      {3875, 3875},
      {4000, 4000},
      {4125, 4118},
      {4250, 4235},
      {4375, 4353},
      {4500, 4471},
      {4625, 4588},
      {4750, 4706},
      {4875, 4826},
      {5000, 4941},
      {5125, 5059},
      {5250, 5176},
      {5375, 5294},
      {5500, 5412},
      {5625, 5529},
      {5750, 5647},
      {5875, 5764},
      {6000, 5882},
      {6125, 6000},
      {6250, 6125},
      {6375, 6250},
      {6500, 6375},
      {6625, 6500},
      {6750, 6625},
      {6875, 6750},
      {7000, 6875},
      {7125, 7000},
      {7250, 7125},
      {7375, 7250},
      {7500, 7375},
      {7625, 7500},
      {7750, 7625},
      {7875, 7750},
      {8000, 7875},
      {8118, 8000},
      {8235, 8118},
      {8353, 8235},
      {8471, 8353},
      {8588, 8471},
      {8706, 8588},
      {8824, 8706},
      {8941, 8824},
      {9059, 8941},
      {9176, 9059},
      {9294, 9176},
      {9412, 9294},
      {9529, 9412},
      {9647, 9529},
      {9765, 9647},
      {9882, 9765},
      {10000, 9882},
      {10125, 10000},
      {10250, 10133},
      {10375, 10267},
      {10500, 10400},
      {10625, 10533},
      {10750, 10667},
      {10875, 10800},
      {11000, 10933},
      {11125, 11067},
      {11250, 11200},
      {11375, 11333},
      {11500, 11467},
      {11625, 11600},
      {11750, 11733},
      {11875, 11867},
      {12000, 12000},
      {12125, 12125},
      {12250, 12250},
      {12375, 12375},
      {12500, 12500},
      {12625, 12625},
      {12750, 12750},
      {12875, 12875},
      {13000, 13000},
      {13125, 13125},
      {13250, 13250},
      {13375, 13375},
      {13500, 13500},
      {13625, 13625},
      {13750, 13750},
      {13875, 13875},
      {14000, 14000},
      {14125, 14118},
      {14250, 14235},
      {14375, 14353},
      {14500, 14471},
      {14625, 14588},
      {14750, 14706},
      {14875, 14824},
      {15000, 14941},
      {15125, 15059},
      {15250, 15176},
      {15375, 15294},
      {15500, 15412},
      {15625, 15529},
      {15750, 15647},
      {15875, 15765},
      {16000, 15882},
      {16125, 16000},
      {16250, 16125},
      {16375, 16250},
      {16500, 16375},
      {16625, 16500},
      {16750, 16625},
      {16875, 16750},
      {17000, 16875},
      {17125, 17000},
      {17250, 17125},
      {17375, 17250},
      {17500, 17375},
      {17625, 17500},
      {17750, 17625},
      {17875, 17750},
      {18000, 17875},
      {18125, 18000},
      {18250, 18133},
      {18375, 18267},
      {18500, 18400},
      {18625, 18533},
      {18750, 18667},
      {18875, 18800},
      {19000, 18933},
      {19125, 19067},
      {19250, 19200},
      {19375, 19333},
      {19500, 19467},
      {19625, 19600},
      {19750, 19733},
      {19875, 19867},
      {20000, 20000},
      {20133, 20118},
      {20267, 20235},
      {20400, 20353},
      {20533, 20471},
      {20667, 20588},
      {20800, 20706},
      {20933, 20824},
      {21067, 20941},
      {21200, 21059},
      {21333, 21176},
      {21467, 21294},
      {21600, 21412},
      {21733, 21529},
      {21867, 21647},
      {22000, 21765},
      {22125, 21882},
      {22250, 22000},
      {22375, 22143},
      {22500, 22286},
      {22625, 22429},
      {22750, 22571},
      {22875, 22714},
      {23000, 22857},
      {23125, 23000},
      {23250, 23143},
      {23375, 23286},
      {23500, 23429},
      {23625, 23571},
      {23750, 23714},
      {23875, 23857},
      {24000, 24000},
      {24143, 24133},
      {24286, 24267},
      {24429, 24400},
      {24571, 24533},
      {24714, 24667},
      {24857, 24800},
      {25000, 24933},
      {25153, 25067},
      {25286, 25200},
      {25439, 25333},
      {25571, 25467},
      {25714, 25600},
      {25857, 25733},
      {26000, 25867},
      {26143, 26000},
      {26286, 26143},
      {26429, 26286},
      {26571, 26429},
      {26714, 26571},
      {26857, 26714},
      {26000, 26857},
      {27153, 27000},
      {27286, 27143},
      {27439, 27286},
      {27571, 27429},
      {27714, 27571},
      {27857, 27714},
      {28000, 27857},
      {28154, 28000},
      {28308, 28154},
      {28462, 28307},
      {28615, 28462},
      {28769, 28615},
      {28923, 28769},
      {29077, 28923},
      {29231, 29077},
      {29385, 29231},
      {29538, 29385},
      {29692, 29538},
      {29846, 29692},
      {30000, 29846},
      {30154, 30000},
      {30308, 30154},
      {30462, 30307},
      {30615, 30462},
      {30769, 30615},
      {30923, 30769},
      {31077, 30923},
      {31231, 31077},
      {31385, 31231},
      {31538, 31385},
      {31692, 31538},
      {31846, 31692},
      {32000, 31846},
      {32154, 32000},
      {32308, 32154},
      {32462, 32308},
      {32615, 32462},
      {32769, 32615},
      {32923, 32769},
      {32077, 32923},
      {33231, 33077},
      {33385, 33231},
   };

   BSTD_UNUSED(pChip);

	*lnbVoltage = adc_voltage[voltage][currChannel];

	if(*lnbVoltage > 0)
		return true;
	else
		return false;
}

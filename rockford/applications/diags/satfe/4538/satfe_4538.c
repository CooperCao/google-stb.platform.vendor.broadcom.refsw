/******************************************************************************* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant* to the terms and conditions of a separate, written license agreement executed
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
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "satfe.h"
#include "satfe_platform.h"
#include "satfe_4538.h"
#include "bast_4538.h"
#include "bast_4538_priv.h"
#include "bchp_hif.h"
#include "bchp_csr.h"
#include "bchp_leap_ctrl.h"
#include "bchp_leap_host_irq.h"
#include "bchp_leap_l2.h"

#if BCHP_CHIP==4538
#include "bchp_tm.h"
#include "bchp_leap_l1.h"
#include "bchp_ftm_intr2.h"
#include "bchp_ftm_phy.h"
#include "bchp_ftm_skit.h"
#include "bchp_ftm_uart.h"
#include "bchp_ftm_phy_ana.h"
#include "bchp_ftm_sw_spare.h"
#include "bchp_sds_bert_0.h"
#include "bchp_sds_bl_0.h"
#include "bchp_sds_cg_0.h"
#include "bchp_sds_cl_0.h"
#include "bchp_sds_dft_0.h"
#include "bchp_sds_dsec.h"
#include "bchp_sds_dsec_intr2.h"
#include "bchp_sds_eq_0.h"
#include "bchp_sds_fec_0.h"
#include "bchp_sds_fe_0.h"
#include "bchp_sds_gr_bridge_0.h"
#include "bchp_sds_hp_0.h"
#include "bchp_sds_intr2_0_0.h"
#include "bchp_sds_misc_0.h"
#include "bchp_sds_cwc_0.h"
#include "bchp_sds_oi_0.h"
#include "bchp_sds_snr_0.h"
#include "bchp_sds_vit_0.h"
#include "bchp_afec_0.h"
#include "bchp_afec_global_0.h"
#include "bchp_afec_gr_bridge_0.h"
#include "bchp_afec_intr_ctrl2_0.h"
#include "bchp_sds_dsec_gr_bridge.h"
#include "bchp_sds_dsec_sw_spare.h"
#include "bchp_gio.h"
#include "bchp_leap_uart.h"
#include "bchp_leap_wdg.h"
#include "bchp_per_irq.h"
#include "bchp_tfec_0.h"
#include "bchp_tfec_gr_bridge_0.h"
#include "bchp_tfec_intr2_0.h"
#include "bchp_tfec_misc_0.h"
#include "bchp_timer.h"
#include "bchp_avs_asb_registers.h"
#include "bchp_avs_hw_mntr.h"
#include "bchp_avs_pvt_mntr_config.h"
#include "bchp_avs_ro_registers_0.h"
#include "bchp_avs_ro_registers_1.h"
#include "bchp_avs_rosc_threshold_1.h"
#include "bchp_avs_rosc_threshold_2.h"
#include "bchp_bac_mspi.h"
#include "bchp_bsca.h"
#include "bchp_bscb.h"
#include "bchp_bspi.h"
#include "bchp_demod_xpt_fe.h"
#include "bchp_demod_xpt_mtsif_tx0_io.h"
#include "bchp_demod_xpt_mtsif_tx1_io.h"
#include "bchp_mspi.h"
#include "bchp_demod_xpt_wakeup.h"
#endif
#include "bchp_aif_mdac_cal_ana_u1.h"
#include "bchp_aif_mdac_cal_bac.h"
#include "bchp_aif_mdac_cal_core0.h"
#include "bchp_aif_mdac_cal_core_intr2_0.h"
#include "bchp_aif_wb_sat_ana_u1_0.h"
#include "bchp_aif_wb_sat_core0_0.h"
#include "bchp_aif_wb_sat_core_intr2_0_0.h"
#include "bchp_stb_chan_ch0.h"
#include "bchp_stb_chan_ctrl.h"

#include "bhab.h"
#include "bhab_4538_priv.h"
#include "bchp_3447.h"


/* local functions */
bool SATFE_4538_GetFreqFromString(char *str, uint32_t *pHz);
bool SATFE_4538_IsSpinv(SATFE_Chip *pChip, BAST_ChannelStatus *pStatus);
bool SATFE_4538_IsPilotOn(SATFE_Chip *pChip, BAST_ChannelStatus *pStatus);
bool SATFE_4538_Command_test_i2c(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_4538_Command_print(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_4538_Command_read(struct SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_4538_Command_write(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_4538_Command_help(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_4538_Command_checkmem(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_4538_Command_debug_test(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_4538_Command_avs(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_4538_Command_os_status(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_4538_Command_memread(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_4538_Command_memwrite(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_4538_Command_bcm3447_write(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_4538_Command_bcm3447_read(SATFE_Chip *pChip, int argc, char **argv);
void SATFE_4538_GetDebugModuleNames(struct SATFE_Chip *pChip, int *numModules, char **moduleNames[]);
bool SATFE_4538_GetAdcVoltage(struct SATFE_Chip *pChip, uint8_t voltage, uint8_t currChannel, uint16_t *lnbVoltage);
bool SATFE_4538_Mi2cWrite(struct SATFE_Chip *pChip, uint8_t channel, uint8_t slave_addr, uint8_t *buf, uint8_t n);
bool SATFE_4538_Command_wakeup_packet(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_4538_Command_trace(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_4538_Command_gpo_write(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_4538_Command_bert_interface(SATFE_Chip *pChip, int argc, char **argv);

BERR_Code SATFE_4538_Mi2cRead(struct SATFE_Chip *pChip, uint8_t channel, uint8_t slave_addr, uint8_t *out_buf, uint8_t out_n, uint8_t *in_buf, uint8_t in_n);


#define SATFE_4538_NumDebugModules 6
char *SATFE_4538_DebugModules[SATFE_4538_NumDebugModules] =
{
   "bast",
   "bast_4538",
   "bast_4538_priv",
   "BHAB",
   "bhab_4538",
   "bhab_4538_priv"
};


/* define commands specific to this chip */
SATFE_Command SATFE_4538_CommandSet[] =
{
   {"_help", SATFE_4538_Command_help},
   {"read", SATFE_4538_Command_read},
   {"r", SATFE_4538_Command_read},
   {"write", SATFE_4538_Command_write},
   {"w", SATFE_4538_Command_write},
   {"test_i2c", SATFE_4538_Command_test_i2c},
   {"print", SATFE_4538_Command_print},
   {"checkmem", SATFE_4538_Command_checkmem},
   {"debug_test", SATFE_4538_Command_debug_test},
   {"avs", SATFE_4538_Command_avs},
   {"os_status", SATFE_4538_Command_os_status},
   {"memread", SATFE_4538_Command_memread},
   {"mr", SATFE_4538_Command_memread},
   {"memwrite", SATFE_4538_Command_memwrite},
   {"mw", SATFE_4538_Command_memwrite},
   {"wakeup_packet", SATFE_4538_Command_wakeup_packet},
   {"trace", SATFE_4538_Command_trace},
   {"gpo_write", SATFE_4538_Command_gpo_write},
   {"gpo_set", SATFE_4538_Command_gpo_write},
   {"bcm3447_write", SATFE_4538_Command_bcm3447_write},
   {"bcm3447_read", SATFE_4538_Command_bcm3447_read},
   {"bert_interface", SATFE_4538_Command_bert_interface},
   {0, NULL},
};


static SATFE_ChipFunctTable SATFE_4538_Functs =
{
   SATFE_4538_GetFreqFromString,
   SATFE_4538_IsSpinv,
   SATFE_4538_IsPilotOn,
   SATFE_4538_GetDebugModuleNames,
   SATFE_4538_GetAdcVoltage,
   SATFE_4538_Mi2cWrite,
   SATFE_4538_Mi2cRead
};


SATFE_Register SATFE_3447_RegisterMap[] =
{
   {"CHIP_ID", BCHP_BCM3447_CHIP_ID, SATFE_RegisterType_eExternal},
   {"CHIP_REV", BCHP_BCM3447_CHIP_REV, SATFE_RegisterType_eExternal},
   {"BAC_CTRL", BCHP_BCM3447_BAC_CTRL, SATFE_RegisterType_eExternal},
   {"BAC_SPI_DATA0_INT", BCHP_BCM3447_BAC_SPI_DATA0_INT, SATFE_RegisterType_eExternal},
   {"BAC_SPI_DATA1_INT", BCHP_BCM3447_BAC_SPI_DATA1_INT, SATFE_RegisterType_eExternal},
   {"BAC_SPI_DATA0_WR", BCHP_BCM3447_BAC_SPI_DATA0_WR, SATFE_RegisterType_eExternal},
   {"BAC_SPI_DATA1_WR", BCHP_BCM3447_BAC_SPI_DATA1_WR, SATFE_RegisterType_eExternal},
   {"RFFE_CTRL_01", BCHP_BCM3447_RFFE_CTRL_01, SATFE_RegisterType_eExternal},
   {"RFFE_CTRL_02", BCHP_BCM3447_RFFE_CTRL_02, SATFE_RegisterType_eExternal},
   {"RFFE_CTRL_03", BCHP_BCM3447_RFFE_CTRL_03, SATFE_RegisterType_eExternal},
   {"RFFE_CTRL_04", BCHP_BCM3447_RFFE_CTRL_04, SATFE_RegisterType_eExternal},
   {"RFFE_CTRL_05", BCHP_BCM3447_RFFE_CTRL_05, SATFE_RegisterType_eExternal},
   {"RFFE_CTRL_06", BCHP_BCM3447_RFFE_CTRL_06, SATFE_RegisterType_eExternal},
   {"RFFE_CTRL_07", BCHP_BCM3447_RFFE_CTRL_07, SATFE_RegisterType_eExternal},
   {"RFFE_CTRL_08", BCHP_BCM3447_RFFE_CTRL_08, SATFE_RegisterType_eExternal},
   {"RFFE_CTRL_09", BCHP_BCM3447_RFFE_CTRL_09, SATFE_RegisterType_eExternal},
   {"RFFE_CTRL_10", BCHP_BCM3447_RFFE_CTRL_10, SATFE_RegisterType_eExternal},
   {"RFFE_CTRL_11", BCHP_BCM3447_RFFE_CTRL_11, SATFE_RegisterType_eExternal},
   {"RFFE_CTRL_12", BCHP_BCM3447_RFFE_CTRL_12, SATFE_RegisterType_eExternal},
   {"RFFE_CTRL_13", BCHP_BCM3447_RFFE_CTRL_13, SATFE_RegisterType_eExternal},
   {"RFFE_CTRL_14", BCHP_BCM3447_RFFE_CTRL_14, SATFE_RegisterType_eExternal},
   {"RFFE_CTRL_15", BCHP_BCM3447_RFFE_CTRL_15, SATFE_RegisterType_eExternal},
   {"RFFE_RD_1", BCHP_BCM3447_RFFE_RD_1, SATFE_RegisterType_eExternal},
   {"RFFE_RD_2", BCHP_BCM3447_RFFE_RD_2, SATFE_RegisterType_eExternal},
   {"BAC_LNA_DATA0_INT", BCHP_BCM3447_BAC_LNA_DATA0_INT, SATFE_RegisterType_eExternal},
   {"BAC_LNA_DATA1_INT", BCHP_BCM3447_BAC_LNA_DATA1_INT, SATFE_RegisterType_eExternal},
   {NULL, 0, 0}
};


SATFE_Register SATFE_4538_RegisterMap[] =
{
   {"CSR_SER_PROT_REV", BCHP_CSR_SER_PROT_REV, SATFE_RegisterType_eHost},
   {"CSR_CHIP_FAM0", BCHP_CSR_CHIP_FAM0, SATFE_RegisterType_eHost},
   {"CSR_CHIP_FAM1", BCHP_CSR_CHIP_FAM1, SATFE_RegisterType_eHost},
   {"CSR_CHIP_REV0", BCHP_CSR_CHIP_REV0, SATFE_RegisterType_eHost},
   {"CSR_CHIP_REV1", BCHP_CSR_CHIP_REV1, SATFE_RegisterType_eHost},
   {"CSR_PROGRAM", BCHP_CSR_PROGRAM, SATFE_RegisterType_eHost},
   {"CSR_STATUS", BCHP_CSR_STATUS, SATFE_RegisterType_eHost},
   {"CSR_CONFIG", BCHP_CSR_CONFIG, SATFE_RegisterType_eHost},
   {"CSR_RBUS_ADDR0", BCHP_CSR_RBUS_ADDR0, SATFE_RegisterType_eHost},
   {"CSR_RBUS_ADDR1", BCHP_CSR_RBUS_ADDR1, SATFE_RegisterType_eHost},
   {"CSR_RBUS_ADDR2", BCHP_CSR_RBUS_ADDR2, SATFE_RegisterType_eHost},
   {"CSR_RBUS_ADDR3", BCHP_CSR_RBUS_ADDR3, SATFE_RegisterType_eHost},
   {"CSR_RBUS_DATA0", BCHP_CSR_RBUS_DATA0, SATFE_RegisterType_eHost},
   {"CSR_RBUS_DATA1", BCHP_CSR_RBUS_DATA1, SATFE_RegisterType_eHost},
   {"CSR_RBUS_DATA2", BCHP_CSR_RBUS_DATA2, SATFE_RegisterType_eHost},
   {"CSR_RBUS_DATA3", BCHP_CSR_RBUS_DATA3, SATFE_RegisterType_eHost},
   {"CSR_CHIP_REV_INT0", BCHP_CSR_CHIP_REV_INT0, SATFE_RegisterType_eHost},
   {"CSR_CHIP_REV_INT1", BCHP_CSR_CHIP_REV_INT1, SATFE_RegisterType_eHost},
   {"CSR_CHIP_PROD0", BCHP_CSR_CHIP_PROD0, SATFE_RegisterType_eHost},
   {"CSR_CHIP_PROD1", BCHP_CSR_CHIP_PROD1, SATFE_RegisterType_eHost},
   {"HIF_SFT_RST0", BCHP_HIF_SFT_RST0, SATFE_RegisterType_eHost},
   {"HIF_SFT_RST1", BCHP_HIF_SFT_RST1, SATFE_RegisterType_eHost},
   {"HIF_SFT_RST2", BCHP_HIF_SFT_RST2, SATFE_RegisterType_eHost},
   {"HIF_RST_LOCK_OVR", BCHP_HIF_RST_LOCK_OVR, SATFE_RegisterType_eHost},
   {"HIF_PWRDN", BCHP_HIF_PWRDN, SATFE_RegisterType_eHost},
   {"HIF_MEM_CTRL", BCHP_HIF_MEM_CTRL, SATFE_RegisterType_eHost},
   {"HIF_OSC_LDO_CTRL", BCHP_HIF_OSC_LDO_CTRL, SATFE_RegisterType_eHost},
   {"HIF_OSC_BIAS_CTRL", BCHP_HIF_OSC_BIAS_CTRL, SATFE_RegisterType_eHost},
   {"HIF_OSC_CML_CTRL", BCHP_HIF_OSC_CML_CTRL, SATFE_RegisterType_eHost},
   {"HIF_OSC_MISC_CTRL", BCHP_HIF_OSC_MISC_CTRL, SATFE_RegisterType_eHost},
   {"HIF_OSC_STRAP_OVRD_XCORE_BIAS", BCHP_HIF_OSC_STRAP_OVRD_XCORE_BIAS, SATFE_RegisterType_eHost},
   {"HIF_OSC_STRAP_OVRD_HIGHPASS", BCHP_HIF_OSC_STRAP_OVRD_HIGHPASS, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_RST", BCHP_HIF_REG_PLL_RST, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_PDIV", BCHP_HIF_REG_PLL_PDIV, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_NDIV_INT", BCHP_HIF_REG_PLL_NDIV_INT, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_MDIV_CLK_108", BCHP_HIF_REG_PLL_MDIV_CLK_108, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_MDIV_CLK_054", BCHP_HIF_REG_PLL_MDIV_CLK_054, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_MDIV_CLK_216", BCHP_HIF_REG_PLL_MDIV_CLK_216, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_MDEL_CLK_108", BCHP_HIF_REG_PLL_MDEL_CLK_108, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_MDEL_CLK_054", BCHP_HIF_REG_PLL_MDEL_CLK_054, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_MDEL_CLK_216", BCHP_HIF_REG_PLL_MDEL_CLK_216, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_MISC_CLK_108", BCHP_HIF_REG_PLL_MISC_CLK_108, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_MISC_CLK_054", BCHP_HIF_REG_PLL_MISC_CLK_054, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_MISC_CLK_216", BCHP_HIF_REG_PLL_MISC_CLK_216, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_GAIN_KA", BCHP_HIF_REG_PLL_GAIN_KA, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_GAIN_KI", BCHP_HIF_REG_PLL_GAIN_KI, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_GAIN_KP", BCHP_HIF_REG_PLL_GAIN_KP, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_DCO_BYP_EN", BCHP_HIF_REG_PLL_DCO_BYP_EN, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_DCO_CTRL1", BCHP_HIF_REG_PLL_DCO_CTRL1, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_DCO_CTRL0", BCHP_HIF_REG_PLL_DCO_CTRL0, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_FB_EN", BCHP_HIF_REG_PLL_FB_EN, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_FB_OFFSET1", BCHP_HIF_REG_PLL_FB_OFFSET1, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_FB_OFFSET0", BCHP_HIF_REG_PLL_FB_OFFSET0, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_SS_CTRL", BCHP_HIF_REG_PLL_SS_CTRL, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_SS_STEP1", BCHP_HIF_REG_PLL_SS_STEP1, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_SS_STEP0", BCHP_HIF_REG_PLL_SS_STEP0, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_SS_LIMIT3", BCHP_HIF_REG_PLL_SS_LIMIT3, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_SS_LIMIT2", BCHP_HIF_REG_PLL_SS_LIMIT2, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_SS_LIMIT1", BCHP_HIF_REG_PLL_SS_LIMIT1, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_SS_LIMIT0", BCHP_HIF_REG_PLL_SS_LIMIT0, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_MISC_CTRL1", BCHP_HIF_REG_PLL_MISC_CTRL1, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_MISC_CTRL0", BCHP_HIF_REG_PLL_MISC_CTRL0, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_STAT_CTRL", BCHP_HIF_REG_PLL_STAT_CTRL, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_STATUS2", BCHP_HIF_REG_PLL_STATUS2, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_STATUS1", BCHP_HIF_REG_PLL_STATUS1, SATFE_RegisterType_eHost},
   {"HIF_REG_PLL_STATUS0", BCHP_HIF_REG_PLL_STATUS0, SATFE_RegisterType_eHost},
   {"HIF_REG_CLK_EN", BCHP_HIF_REG_CLK_EN, SATFE_RegisterType_eHost},
   {"HIF_MISC_CTRL", BCHP_HIF_MISC_CTRL, SATFE_RegisterType_eHost},
   {"HIF_PIN_STRAP2", BCHP_HIF_PIN_STRAP2, SATFE_RegisterType_eHost},
   {"HIF_PIN_STRAP1", BCHP_HIF_PIN_STRAP1, SATFE_RegisterType_eHost},
   {"HIF_PIN_STRAP0", BCHP_HIF_PIN_STRAP0, SATFE_RegisterType_eHost},
   {"HIF_SPARE3", BCHP_HIF_SPARE3, SATFE_RegisterType_eHost},
   {"HIF_SPARE2", BCHP_HIF_SPARE2, SATFE_RegisterType_eHost},
   {"HIF_SPARE1", BCHP_HIF_SPARE1, SATFE_RegisterType_eHost},
   {"HIF_SPARE0", BCHP_HIF_SPARE0, SATFE_RegisterType_eHost},
   {"HIF_SFT3", BCHP_HIF_SFT3, SATFE_RegisterType_eHost},
   {"HIF_SFT2", BCHP_HIF_SFT2, SATFE_RegisterType_eHost},
   {"HIF_SFT1", BCHP_HIF_SFT1, SATFE_RegisterType_eHost},
   {"HIF_SFT0", BCHP_HIF_SFT0, SATFE_RegisterType_eHost},
   {"LEAP_CTRL_REVID", BCHP_LEAP_CTRL_REVID, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_CTRL", BCHP_LEAP_CTRL_CTRL, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_CPU_STRAPS", BCHP_LEAP_CTRL_CPU_STRAPS, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_CPU_STATUS", BCHP_LEAP_CTRL_CPU_STATUS, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_HAB_REQUEST", BCHP_LEAP_CTRL_HAB_REQUEST, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_RBUS_BCAST_ACK", BCHP_LEAP_CTRL_RBUS_BCAST_ACK, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_TIMER0", BCHP_LEAP_CTRL_TIMER0, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_TIMER0_COUNT", BCHP_LEAP_CTRL_TIMER0_COUNT, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_TIMER1", BCHP_LEAP_CTRL_TIMER1, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_TIMER1_COUNT", BCHP_LEAP_CTRL_TIMER1_COUNT, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_TIMER2", BCHP_LEAP_CTRL_TIMER2, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_TIMER2_COUNT", BCHP_LEAP_CTRL_TIMER2_COUNT, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_TIMER3", BCHP_LEAP_CTRL_TIMER3, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_TIMER3_COUNT", BCHP_LEAP_CTRL_TIMER3_COUNT, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_TIMER4", BCHP_LEAP_CTRL_TIMER4, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_TIMER4_COUNT", BCHP_LEAP_CTRL_TIMER4_COUNT, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_TIMER5", BCHP_LEAP_CTRL_TIMER5, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_TIMER5_COUNT", BCHP_LEAP_CTRL_TIMER5_COUNT, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_TIMER6", BCHP_LEAP_CTRL_TIMER6, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_TIMER6_COUNT", BCHP_LEAP_CTRL_TIMER6_COUNT, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_TIMER7", BCHP_LEAP_CTRL_TIMER7, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_TIMER7_COUNT", BCHP_LEAP_CTRL_TIMER7_COUNT, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_SW_SPARE0", BCHP_LEAP_CTRL_SW_SPARE0, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_SW_SPARE1", BCHP_LEAP_CTRL_SW_SPARE1, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_SW_SPARE2", BCHP_LEAP_CTRL_SW_SPARE2, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_SW_SPARE3", BCHP_LEAP_CTRL_SW_SPARE3, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_RBUS_ERR_ADDR", BCHP_LEAP_CTRL_RBUS_ERR_ADDR, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_RBUS_ERR_DATA", BCHP_LEAP_CTRL_RBUS_ERR_DATA, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_RBUS_ERR_XAC", BCHP_LEAP_CTRL_RBUS_ERR_XAC, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_RBUS_ERR_CTRL", BCHP_LEAP_CTRL_RBUS_ERR_CTRL, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_HAB_CNTR", BCHP_LEAP_CTRL_HAB_CNTR, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_TRACE_Q_DATA", BCHP_LEAP_CTRL_TRACE_Q_DATA, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_TRACE_Q_RST_PTRS", BCHP_LEAP_CTRL_TRACE_Q_RST_PTRS, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_MBOX_FIFO_DATA", BCHP_LEAP_CTRL_MBOX_FIFO_DATA, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_MBOX_FIFO_DEPTH", BCHP_LEAP_CTRL_MBOX_FIFO_DEPTH, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_MBOX_FIFO_RST_PTRS", BCHP_LEAP_CTRL_MBOX_FIFO_RST_PTRS, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP0", BCHP_LEAP_CTRL_GP0, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP1", BCHP_LEAP_CTRL_GP1, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP2", BCHP_LEAP_CTRL_GP2, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP3", BCHP_LEAP_CTRL_GP3, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP4", BCHP_LEAP_CTRL_GP4, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP5", BCHP_LEAP_CTRL_GP5, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP6", BCHP_LEAP_CTRL_GP6, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP7", BCHP_LEAP_CTRL_GP7, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP8", BCHP_LEAP_CTRL_GP8, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP9", BCHP_LEAP_CTRL_GP9, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP10", BCHP_LEAP_CTRL_GP10, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP11", BCHP_LEAP_CTRL_GP11, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP12", BCHP_LEAP_CTRL_GP12, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP13", BCHP_LEAP_CTRL_GP13, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP14", BCHP_LEAP_CTRL_GP14, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP15", BCHP_LEAP_CTRL_GP15, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP16", BCHP_LEAP_CTRL_GP16, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP17", BCHP_LEAP_CTRL_GP17, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP18", BCHP_LEAP_CTRL_GP18, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP19", BCHP_LEAP_CTRL_GP19, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP20", BCHP_LEAP_CTRL_GP20, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP21", BCHP_LEAP_CTRL_GP21, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP22", BCHP_LEAP_CTRL_GP22, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP23", BCHP_LEAP_CTRL_GP23, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP24", BCHP_LEAP_CTRL_GP24, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP25", BCHP_LEAP_CTRL_GP25, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP26", BCHP_LEAP_CTRL_GP26, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP27", BCHP_LEAP_CTRL_GP27, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP28", BCHP_LEAP_CTRL_GP28, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP29", BCHP_LEAP_CTRL_GP29, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP30", BCHP_LEAP_CTRL_GP30, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP31", BCHP_LEAP_CTRL_GP31, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP32", BCHP_LEAP_CTRL_GP32, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP33", BCHP_LEAP_CTRL_GP33, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP34", BCHP_LEAP_CTRL_GP34, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP35", BCHP_LEAP_CTRL_GP35, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP36", BCHP_LEAP_CTRL_GP36, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP37", BCHP_LEAP_CTRL_GP37, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP38", BCHP_LEAP_CTRL_GP38, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP39", BCHP_LEAP_CTRL_GP39, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP40", BCHP_LEAP_CTRL_GP40, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP41", BCHP_LEAP_CTRL_GP41, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP42", BCHP_LEAP_CTRL_GP42, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP43", BCHP_LEAP_CTRL_GP43, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP44", BCHP_LEAP_CTRL_GP44, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP45", BCHP_LEAP_CTRL_GP45, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP46", BCHP_LEAP_CTRL_GP46, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP47", BCHP_LEAP_CTRL_GP47, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP48", BCHP_LEAP_CTRL_GP48, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP49", BCHP_LEAP_CTRL_GP49, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP50", BCHP_LEAP_CTRL_GP50, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP51", BCHP_LEAP_CTRL_GP51, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP52", BCHP_LEAP_CTRL_GP52, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP53", BCHP_LEAP_CTRL_GP53, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP54", BCHP_LEAP_CTRL_GP54, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP55", BCHP_LEAP_CTRL_GP55, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP56", BCHP_LEAP_CTRL_GP56, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP57", BCHP_LEAP_CTRL_GP57, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP58", BCHP_LEAP_CTRL_GP58, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP59", BCHP_LEAP_CTRL_GP59, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP60", BCHP_LEAP_CTRL_GP60, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP61", BCHP_LEAP_CTRL_GP61, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP62", BCHP_LEAP_CTRL_GP62, SATFE_RegisterType_eISB},
   {"LEAP_CTRL_GP63", BCHP_LEAP_CTRL_GP63, SATFE_RegisterType_eISB},
   {"LEAP_HOST_IRQ_STATUS0", BCHP_LEAP_HOST_IRQ_STATUS0, SATFE_RegisterType_eISB},
   {"LEAP_HOST_IRQ_SET0", BCHP_LEAP_HOST_IRQ_SET0, SATFE_RegisterType_eISB},
   {"LEAP_HOST_IRQ_CLEAR0", BCHP_LEAP_HOST_IRQ_CLEAR0, SATFE_RegisterType_eISB},
   {"LEAP_HOST_IRQ_MASK_STATUS0", BCHP_LEAP_HOST_IRQ_MASK_STATUS0, SATFE_RegisterType_eISB},
   {"LEAP_HOST_IRQ_MASK_SET0", BCHP_LEAP_HOST_IRQ_MASK_SET0, SATFE_RegisterType_eISB},
   {"LEAP_HOST_IRQ_MASK_CLEAR0", BCHP_LEAP_HOST_IRQ_MASK_CLEAR0, SATFE_RegisterType_eISB},
   {"LEAP_HOST_IRQ_STATUS1", BCHP_LEAP_HOST_IRQ_STATUS1, SATFE_RegisterType_eISB},
   {"LEAP_HOST_IRQ_SET1", BCHP_LEAP_HOST_IRQ_SET1, SATFE_RegisterType_eISB},
   {"LEAP_HOST_IRQ_CLEAR1", BCHP_LEAP_HOST_IRQ_CLEAR1, SATFE_RegisterType_eISB},
   {"LEAP_HOST_IRQ_MASK_STATUS1", BCHP_LEAP_HOST_IRQ_MASK_STATUS1, SATFE_RegisterType_eISB},
   {"LEAP_HOST_IRQ_MASK_SET1", BCHP_LEAP_HOST_IRQ_MASK_SET1, SATFE_RegisterType_eISB},
   {"LEAP_HOST_IRQ_MASK_CLEAR1", BCHP_LEAP_HOST_IRQ_MASK_CLEAR1, SATFE_RegisterType_eISB},
#if BCHP_CHIP==4538
   {"LEAP_L1_INTR_W0_STATUS", BCHP_LEAP_L1_INTR_W0_STATUS, SATFE_RegisterType_eISB},
   {"LEAP_L1_INTR_W0_MASK_STATUS", BCHP_LEAP_L1_INTR_W0_MASK_STATUS, SATFE_RegisterType_eISB},
   {"LEAP_L1_INTR_W0_MASK_SET", BCHP_LEAP_L1_INTR_W0_MASK_SET, SATFE_RegisterType_eISB},
   {"LEAP_L1_INTR_W0_MASK_CLEAR", BCHP_LEAP_L1_INTR_W0_MASK_CLEAR, SATFE_RegisterType_eISB},
   {"LEAP_L1_INTR_W1_STATUS", BCHP_LEAP_L1_INTR_W1_STATUS, SATFE_RegisterType_eISB},
   {"LEAP_L1_INTR_W1_MASK_STATUS", BCHP_LEAP_L1_INTR_W1_MASK_STATUS, SATFE_RegisterType_eISB},
   {"LEAP_L1_INTR_W1_MASK_SET", BCHP_LEAP_L1_INTR_W1_MASK_SET, SATFE_RegisterType_eISB},
   {"LEAP_L1_INTR_W1_MASK_CLEAR", BCHP_LEAP_L1_INTR_W1_MASK_CLEAR, SATFE_RegisterType_eISB},
   {"LEAP_L2_CPU_STATUS", BCHP_LEAP_L2_CPU_STATUS, SATFE_RegisterType_eISB},
   {"LEAP_L2_CPU_SET", BCHP_LEAP_L2_CPU_SET, SATFE_RegisterType_eISB},
   {"LEAP_L2_CPU_CLEAR", BCHP_LEAP_L2_CPU_CLEAR, SATFE_RegisterType_eISB},
   {"LEAP_L2_CPU_MASK_STATUS", BCHP_LEAP_L2_CPU_MASK_STATUS, SATFE_RegisterType_eISB},
   {"LEAP_L2_CPU_MASK_SET", BCHP_LEAP_L2_CPU_MASK_SET, SATFE_RegisterType_eISB},
   {"LEAP_L2_CPU_MASK_CLEAR", BCHP_LEAP_L2_CPU_MASK_CLEAR, SATFE_RegisterType_eISB},
   {"LEAP_UART_UARTDR", BCHP_LEAP_UART_UARTDR, SATFE_RegisterType_eISB},
   {"LEAP_UART_UARTRSR", BCHP_LEAP_UART_UARTRSR, SATFE_RegisterType_eISB},
   {"LEAP_UART_UARTFR", BCHP_LEAP_UART_UARTFR, SATFE_RegisterType_eISB},
   {"LEAP_UART_UARTILPR", BCHP_LEAP_UART_UARTILPR, SATFE_RegisterType_eISB},
   {"LEAP_UART_UARTIBRD", BCHP_LEAP_UART_UARTIBRD, SATFE_RegisterType_eISB},
   {"LEAP_UART_UARTFBRD", BCHP_LEAP_UART_UARTFBRD, SATFE_RegisterType_eISB},
   {"LEAP_UART_UARTLCR_H", BCHP_LEAP_UART_UARTLCR_H, SATFE_RegisterType_eISB},
   {"LEAP_UART_UARTCR", BCHP_LEAP_UART_UARTCR, SATFE_RegisterType_eISB},
   {"LEAP_UART_UARTIFLS", BCHP_LEAP_UART_UARTIFLS, SATFE_RegisterType_eISB},
   {"LEAP_UART_UARTIMSC", BCHP_LEAP_UART_UARTIMSC, SATFE_RegisterType_eISB},
   {"LEAP_UART_UARTRIS", BCHP_LEAP_UART_UARTRIS, SATFE_RegisterType_eISB},
   {"LEAP_UART_UARTMIS", BCHP_LEAP_UART_UARTMIS, SATFE_RegisterType_eISB},
   {"LEAP_UART_UARTICR", BCHP_LEAP_UART_UARTICR, SATFE_RegisterType_eISB},
   {"LEAP_UART_UARTDMACR", BCHP_LEAP_UART_UARTDMACR, SATFE_RegisterType_eISB},
   {"LEAP_UART_UARTPERIPHID0", BCHP_LEAP_UART_UARTPERIPHID0, SATFE_RegisterType_eISB},
   {"LEAP_UART_UARTPERIPHID1", BCHP_LEAP_UART_UARTPERIPHID1, SATFE_RegisterType_eISB},
   {"LEAP_UART_UARTPERIPHID2", BCHP_LEAP_UART_UARTPERIPHID2, SATFE_RegisterType_eISB},
   {"LEAP_UART_UARTPERIPHID3", BCHP_LEAP_UART_UARTPERIPHID3, SATFE_RegisterType_eISB},
   {"LEAP_UART_UARTPCELLID0", BCHP_LEAP_UART_UARTPCELLID0, SATFE_RegisterType_eISB},
   {"LEAP_UART_UARTPCELLID1", BCHP_LEAP_UART_UARTPCELLID1, SATFE_RegisterType_eISB},
   {"LEAP_UART_UARTPCELLID2", BCHP_LEAP_UART_UARTPCELLID2, SATFE_RegisterType_eISB},
   {"LEAP_UART_UARTPCELLID3", BCHP_LEAP_UART_UARTPCELLID3, SATFE_RegisterType_eISB},
   {"LEAP_WDG_WDOGLOAD", BCHP_LEAP_WDG_WDOGLOAD, SATFE_RegisterType_eISB},
   {"LEAP_WDG_WDOGVALUE", BCHP_LEAP_WDG_WDOGVALUE, SATFE_RegisterType_eISB},
   {"LEAP_WDG_WDOGCONTROL", BCHP_LEAP_WDG_WDOGCONTROL, SATFE_RegisterType_eISB},
   {"LEAP_WDG_WDOGINTCLR", BCHP_LEAP_WDG_WDOGINTCLR, SATFE_RegisterType_eISB},
   {"LEAP_WDG_WDOGRIS", BCHP_LEAP_WDG_WDOGRIS, SATFE_RegisterType_eISB},
   {"LEAP_WDG_WDOGMIS", BCHP_LEAP_WDG_WDOGMIS, SATFE_RegisterType_eISB},
   {"LEAP_WDG_WDOGLOCK", BCHP_LEAP_WDG_WDOGLOCK, SATFE_RegisterType_eISB},
   {"LEAP_WDG_WDOGITCR", BCHP_LEAP_WDG_WDOGITCR, SATFE_RegisterType_eISB},
   {"LEAP_WDG_WDOGITOP", BCHP_LEAP_WDG_WDOGITOP, SATFE_RegisterType_eISB},
   {"LEAP_WDG_WDOGPERIPHID0", BCHP_LEAP_WDG_WDOGPERIPHID0, SATFE_RegisterType_eISB},
   {"LEAP_WDG_WDOGPERIPHID1", BCHP_LEAP_WDG_WDOGPERIPHID1, SATFE_RegisterType_eISB},
   {"LEAP_WDG_WDOGPERIPHID2", BCHP_LEAP_WDG_WDOGPERIPHID2, SATFE_RegisterType_eISB},
   {"LEAP_WDG_WDOGPERIPHID3", BCHP_LEAP_WDG_WDOGPERIPHID3, SATFE_RegisterType_eISB},
   {"LEAP_WDG_WDOGPCELLID0", BCHP_LEAP_WDG_WDOGPCELLID0, SATFE_RegisterType_eISB},
   {"LEAP_WDG_WDOGPCELLID1", BCHP_LEAP_WDG_WDOGPCELLID1, SATFE_RegisterType_eISB},
   {"LEAP_WDG_WDOGPCELLID2", BCHP_LEAP_WDG_WDOGPCELLID2, SATFE_RegisterType_eISB},
   {"LEAP_WDG_WDOGPCELLID3", BCHP_LEAP_WDG_WDOGPCELLID3, SATFE_RegisterType_eISB},
   {"AFEC_RST", BCHP_AFEC_0_RST, SATFE_RegisterType_eISB},
   {"AFEC_CNTR_CTRL", BCHP_AFEC_0_CNTR_CTRL, SATFE_RegisterType_eISB},
   {"AFEC_TEST_CONFIG", BCHP_AFEC_0_TEST_CONFIG, SATFE_RegisterType_eISB},
   {"AFEC_BIST_TEST_CONFIG", BCHP_AFEC_0_BIST_TEST_CONFIG, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MAX_ITER_OVERIDE", BCHP_AFEC_0_ACM_MAX_ITER_OVERIDE, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_1", BCHP_AFEC_0_ACM_MODCOD_1, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_2", BCHP_AFEC_0_ACM_MODCOD_2, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_3", BCHP_AFEC_0_ACM_MODCOD_3, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_4", BCHP_AFEC_0_ACM_MODCOD_4, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_5", BCHP_AFEC_0_ACM_MODCOD_5, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_6", BCHP_AFEC_0_ACM_MODCOD_6, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_7", BCHP_AFEC_0_ACM_MODCOD_7, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_8", BCHP_AFEC_0_ACM_MODCOD_8, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_9", BCHP_AFEC_0_ACM_MODCOD_9, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_10", BCHP_AFEC_0_ACM_MODCOD_10, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_11", BCHP_AFEC_0_ACM_MODCOD_11, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_12", BCHP_AFEC_0_ACM_MODCOD_12, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_13", BCHP_AFEC_0_ACM_MODCOD_13, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_14", BCHP_AFEC_0_ACM_MODCOD_14, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_15", BCHP_AFEC_0_ACM_MODCOD_15, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_16", BCHP_AFEC_0_ACM_MODCOD_16, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_17", BCHP_AFEC_0_ACM_MODCOD_17, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_18", BCHP_AFEC_0_ACM_MODCOD_18, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_19", BCHP_AFEC_0_ACM_MODCOD_19, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_20", BCHP_AFEC_0_ACM_MODCOD_20, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_21", BCHP_AFEC_0_ACM_MODCOD_21, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_22", BCHP_AFEC_0_ACM_MODCOD_22, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_23", BCHP_AFEC_0_ACM_MODCOD_23, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_24", BCHP_AFEC_0_ACM_MODCOD_24, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_25", BCHP_AFEC_0_ACM_MODCOD_25, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_26", BCHP_AFEC_0_ACM_MODCOD_26, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_27", BCHP_AFEC_0_ACM_MODCOD_27, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_28", BCHP_AFEC_0_ACM_MODCOD_28, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_29_EXT", BCHP_AFEC_0_ACM_MODCOD_29_EXT, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_29_LDPC0_EXT", BCHP_AFEC_0_ACM_MODCOD_29_LDPC0_EXT, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_30_EXT", BCHP_AFEC_0_ACM_MODCOD_30_EXT, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_30_LDPC0_EXT", BCHP_AFEC_0_ACM_MODCOD_30_LDPC0_EXT, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_31_EXT", BCHP_AFEC_0_ACM_MODCOD_31_EXT, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_31_LDPC0_EXT", BCHP_AFEC_0_ACM_MODCOD_31_LDPC0_EXT, SATFE_RegisterType_eISB},
   {"AFEC_ACM_SYM_CNT_0", BCHP_AFEC_0_ACM_SYM_CNT_0, SATFE_RegisterType_eISB},
   {"AFEC_ACM_SYM_CNT_1", BCHP_AFEC_0_ACM_SYM_CNT_1, SATFE_RegisterType_eISB},
   {"AFEC_ACM_SYM_CNT_2", BCHP_AFEC_0_ACM_SYM_CNT_2, SATFE_RegisterType_eISB},
   {"AFEC_ACM_CYCLE_CNT_0", BCHP_AFEC_0_ACM_CYCLE_CNT_0, SATFE_RegisterType_eISB},
   {"AFEC_ACM_CYCLE_CNT_1", BCHP_AFEC_0_ACM_CYCLE_CNT_1, SATFE_RegisterType_eISB},
   {"AFEC_ACM_CYCLE_CNT_2", BCHP_AFEC_0_ACM_CYCLE_CNT_2, SATFE_RegisterType_eISB},
   {"AFEC_ACM_CYCLE_CNT_3", BCHP_AFEC_0_ACM_CYCLE_CNT_3, SATFE_RegisterType_eISB},
   {"AFEC_ACM_CYCLE_CNT_4", BCHP_AFEC_0_ACM_CYCLE_CNT_4, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MISC_0", BCHP_AFEC_0_ACM_MISC_0, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MISC_1", BCHP_AFEC_0_ACM_MISC_1, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_OVERIDE", BCHP_AFEC_0_ACM_MODCOD_OVERIDE, SATFE_RegisterType_eISB},
   {"AFEC_ACM_MODCOD_STATS_CONFIG", BCHP_AFEC_0_ACM_MODCOD_STATS_CONFIG, SATFE_RegisterType_eISB},
   {"AFEC_ACM_DONT_DEC_CNT", BCHP_AFEC_0_ACM_DONT_DEC_CNT, SATFE_RegisterType_eISB},
   {"AFEC_ACM_LDPC_ITER_CNT", BCHP_AFEC_0_ACM_LDPC_ITER_CNT, SATFE_RegisterType_eISB},
   {"AFEC_ACM_LDPC_FAIL_CNT", BCHP_AFEC_0_ACM_LDPC_FAIL_CNT, SATFE_RegisterType_eISB},
   {"AFEC_ACM_LDPC_FRM_CNT", BCHP_AFEC_0_ACM_LDPC_FRM_CNT, SATFE_RegisterType_eISB},
   {"AFEC_LDPC_CONFIG_0", BCHP_AFEC_0_LDPC_CONFIG_0, SATFE_RegisterType_eISB},
   {"AFEC_LDPC_STATUS", BCHP_AFEC_0_LDPC_STATUS, SATFE_RegisterType_eISB},
   {"AFEC_LDPC_MET_CRC", BCHP_AFEC_0_LDPC_MET_CRC, SATFE_RegisterType_eISB},
   {"AFEC_LDPC_EDGE_CRC", BCHP_AFEC_0_LDPC_EDGE_CRC, SATFE_RegisterType_eISB},
   {"AFEC_LDPC_PSL_CTL", BCHP_AFEC_0_LDPC_PSL_CTL, SATFE_RegisterType_eISB},
   {"AFEC_LDPC_PSL_INT_THRES", BCHP_AFEC_0_LDPC_PSL_INT_THRES, SATFE_RegisterType_eISB},
   {"AFEC_LDPC_PSL_INT", BCHP_AFEC_0_LDPC_PSL_INT, SATFE_RegisterType_eISB},
   {"AFEC_LDPC_PSL_AVE", BCHP_AFEC_0_LDPC_PSL_AVE, SATFE_RegisterType_eISB},
   {"AFEC_LDPC_PSL_XCS", BCHP_AFEC_0_LDPC_PSL_XCS, SATFE_RegisterType_eISB},
   {"AFEC_LDPC_PSL_FILTER", BCHP_AFEC_0_LDPC_PSL_FILTER, SATFE_RegisterType_eISB},
   {"AFEC_LDPC_MEM_POWER", BCHP_AFEC_0_LDPC_MEM_POWER, SATFE_RegisterType_eISB},
   {"AFEC_BCH_TPCTL", BCHP_AFEC_0_BCH_TPCTL, SATFE_RegisterType_eISB},
   {"AFEC_BCH_TPSIG", BCHP_AFEC_0_BCH_TPSIG, SATFE_RegisterType_eISB},
   {"AFEC_BCH_CTRL", BCHP_AFEC_0_BCH_CTRL, SATFE_RegisterType_eISB},
   {"AFEC_BCH_DECNBLK", BCHP_AFEC_0_BCH_DECNBLK, SATFE_RegisterType_eISB},
   {"AFEC_BCH_DECCBLK", BCHP_AFEC_0_BCH_DECCBLK, SATFE_RegisterType_eISB},
   {"AFEC_BCH_DECBBLK", BCHP_AFEC_0_BCH_DECBBLK, SATFE_RegisterType_eISB},
   {"AFEC_BCH_DECCBIT", BCHP_AFEC_0_BCH_DECCBIT, SATFE_RegisterType_eISB},
   {"AFEC_BCH_DECMCOR", BCHP_AFEC_0_BCH_DECMCOR, SATFE_RegisterType_eISB},
   {"AFEC_BCH_BBHDR0", BCHP_AFEC_0_BCH_BBHDR0, SATFE_RegisterType_eISB},
   {"AFEC_BCH_BBHDR1", BCHP_AFEC_0_BCH_BBHDR1, SATFE_RegisterType_eISB},
   {"AFEC_BCH_BBHDR2", BCHP_AFEC_0_BCH_BBHDR2, SATFE_RegisterType_eISB},
   {"AFEC_BCH_BBHDR3", BCHP_AFEC_0_BCH_BBHDR3, SATFE_RegisterType_eISB},
   {"AFEC_BCH_BBHDR4", BCHP_AFEC_0_BCH_BBHDR4, SATFE_RegisterType_eISB},
   {"AFEC_BCH_BBHDR5", BCHP_AFEC_0_BCH_BBHDR5, SATFE_RegisterType_eISB},
   {"AFEC_BCH_MPLCK", BCHP_AFEC_0_BCH_MPLCK, SATFE_RegisterType_eISB},
   {"AFEC_BCH_MPCFG", BCHP_AFEC_0_BCH_MPCFG, SATFE_RegisterType_eISB},
   {"AFEC_BCH_SMCFG", BCHP_AFEC_0_BCH_SMCFG, SATFE_RegisterType_eISB},
   {"AFEC_GLOBAL_CLK_CNTRL", BCHP_AFEC_GLOBAL_0_CLK_CNTRL, SATFE_RegisterType_eISB},
   {"AFEC_GLOBAL_PWR_CNTRL", BCHP_AFEC_GLOBAL_0_PWR_CNTRL, SATFE_RegisterType_eISB},
   {"AFEC_GLOBAL_SW_SPARE0", BCHP_AFEC_GLOBAL_0_SW_SPARE0, SATFE_RegisterType_eISB},
   {"AFEC_GLOBAL_SW_SPARE1", BCHP_AFEC_GLOBAL_0_SW_SPARE1, SATFE_RegisterType_eISB},
   {"AFEC_GR_BRIDGE_REVISION", BCHP_AFEC_GR_BRIDGE_0_REVISION, SATFE_RegisterType_eISB},
   {"AFEC_GR_BRIDGE_CTRL", BCHP_AFEC_GR_BRIDGE_0_CTRL, SATFE_RegisterType_eISB},
   {"AFEC_GR_BRIDGE_SW_RESET_0", BCHP_AFEC_GR_BRIDGE_0_SW_RESET_0, SATFE_RegisterType_eISB},
   {"AFEC_GR_BRIDGE_SW_RESET_1", BCHP_AFEC_GR_BRIDGE_0_SW_RESET_1, SATFE_RegisterType_eISB},
   {"AFEC_INTR_CTRL2_CPU_STATUS", BCHP_AFEC_INTR_CTRL2_0_CPU_STATUS, SATFE_RegisterType_eISB},
   {"AFEC_INTR_CTRL2_CPU_SET", BCHP_AFEC_INTR_CTRL2_0_CPU_SET, SATFE_RegisterType_eISB},
   {"AFEC_INTR_CTRL2_CPU_CLEAR", BCHP_AFEC_INTR_CTRL2_0_CPU_CLEAR, SATFE_RegisterType_eISB},
   {"AFEC_INTR_CTRL2_CPU_MASK_STATUS", BCHP_AFEC_INTR_CTRL2_0_CPU_MASK_STATUS, SATFE_RegisterType_eISB},
   {"AFEC_INTR_CTRL2_CPU_MASK_SET", BCHP_AFEC_INTR_CTRL2_0_CPU_MASK_SET, SATFE_RegisterType_eISB},
   {"AFEC_INTR_CTRL2_CPU_MASK_CLEAR", BCHP_AFEC_INTR_CTRL2_0_CPU_MASK_CLEAR, SATFE_RegisterType_eISB},
   {"AFEC_INTR_CTRL2_PCI_STATUS", BCHP_AFEC_INTR_CTRL2_0_PCI_STATUS, SATFE_RegisterType_eISB},
   {"AFEC_INTR_CTRL2_PCI_SET", BCHP_AFEC_INTR_CTRL2_0_PCI_SET, SATFE_RegisterType_eISB},
   {"AFEC_INTR_CTRL2_PCI_CLEAR", BCHP_AFEC_INTR_CTRL2_0_PCI_CLEAR, SATFE_RegisterType_eISB},
   {"AFEC_INTR_CTRL2_PCI_MASK_STATUS", BCHP_AFEC_INTR_CTRL2_0_PCI_MASK_STATUS, SATFE_RegisterType_eISB},
   {"AFEC_INTR_CTRL2_PCI_MASK_SET", BCHP_AFEC_INTR_CTRL2_0_PCI_MASK_SET, SATFE_RegisterType_eISB},
   {"AFEC_INTR_CTRL2_PCI_MASK_CLEAR", BCHP_AFEC_INTR_CTRL2_0_PCI_MASK_CLEAR, SATFE_RegisterType_eISB},
#endif
   {"AIF_MDAC_CAL_ANA_U1_ADC_CNTL0", BCHP_AIF_MDAC_CAL_ANA_U1_ADC_CNTL0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_ANA_U1_ADC_CNTL1", BCHP_AIF_MDAC_CAL_ANA_U1_ADC_CNTL1, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_ANA_U1_ADC_CNTL2", BCHP_AIF_MDAC_CAL_ANA_U1_ADC_CNTL2, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_ANA_U1_ADC_CNTL3", BCHP_AIF_MDAC_CAL_ANA_U1_ADC_CNTL3, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_ANA_U1_WBADC_CAL_CNTL_IN0", BCHP_AIF_MDAC_CAL_ANA_U1_WBADC_CAL_CNTL_IN0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_ANA_U1_WBADC_SYS_CNTL_IN0", BCHP_AIF_MDAC_CAL_ANA_U1_WBADC_SYS_CNTL_IN0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_ANA_U1_WBADC_TIMING_CNTL_IN0", BCHP_AIF_MDAC_CAL_ANA_U1_WBADC_TIMING_CNTL_IN0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_ANA_U1_WBADC_TOP_CNTL_IN0", BCHP_AIF_MDAC_CAL_ANA_U1_WBADC_TOP_CNTL_IN0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_ANA_U1_CLK_CTRL_IN0", BCHP_AIF_MDAC_CAL_ANA_U1_CLK_CTRL_IN0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_ANA_U1_SW_SPARE0", BCHP_AIF_MDAC_CAL_ANA_U1_SW_SPARE0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_ANA_U1_SW_SPARE1", BCHP_AIF_MDAC_CAL_ANA_U1_SW_SPARE1, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_BAC_BACCTL", BCHP_AIF_MDAC_CAL_BAC_BACCTL, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_BAC_BACSEQ", BCHP_AIF_MDAC_CAL_BAC_BACSEQ, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_BAC_BACASSC", BCHP_AIF_MDAC_CAL_BAC_BACASSC, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_BAC_PGALUTD", BCHP_AIF_MDAC_CAL_BAC_PGALUTD, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_BAC_PGALUTA", BCHP_AIF_MDAC_CAL_BAC_PGALUTA, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_RSTCTL", BCHP_AIF_MDAC_CAL_CORE0_RSTCTL, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_HDOFFCTL0", BCHP_AIF_MDAC_CAL_CORE0_HDOFFCTL0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_HDOFFCTL1", BCHP_AIF_MDAC_CAL_CORE0_HDOFFCTL1, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_HDOFFSTS", BCHP_AIF_MDAC_CAL_CORE0_HDOFFSTS, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSCTL_PI_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSCTL_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSCTL_PO_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSCTL_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSCLP_PI_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSCLP_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSCLP_PO_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSCLP_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSHIST_PI_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSHIST_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSHIST_PO_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSHIST_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSCLPCNT_PI_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSCLPCNT_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSCLPCNT_PO_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSCLPCNT_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSACCUM_DMX0_PI_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSACCUM_DMX0_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSACCUM_DMX0_PO_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSACCUM_DMX0_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSACCUM_DMX1_PI_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSACCUM_DMX1_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSACCUM_DMX1_PO_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSACCUM_DMX1_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSLUT011_PI_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSLUT011_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSLUT010_PI_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSLUT010_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSLUT001_PI_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSLUT001_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSLUT000_PI_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSLUT000_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSLUT111_PI_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSLUT111_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSLUT110_PI_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSLUT110_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSLUT101_PI_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSLUT101_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSLUT100_PI_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSLUT100_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSLUT011_PO_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSLUT011_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSLUT010_PO_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSLUT010_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSLUT001_PO_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSLUT001_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSLUT000_PO_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSLUT000_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSLUT111_PO_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSLUT111_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSLUT110_PO_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSLUT110_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSLUT101_PO_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSLUT101_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSLUT100_PO_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSLUT100_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSLMS_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSLMS_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSLMSMU_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSLMSMU_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSCOEFD_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSCOEFD_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSCOEFA_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSCOEFA_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSCOEFEN_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSCOEFEN_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_MDACSA_SLC0", BCHP_AIF_MDAC_CAL_CORE0_MDACSA_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_MDACSADU_SLC0", BCHP_AIF_MDAC_CAL_CORE0_MDACSADU_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_MDACSAOUT_SLC0", BCHP_AIF_MDAC_CAL_CORE0_MDACSAOUT_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_MDACSASTS", BCHP_AIF_MDAC_CAL_CORE0_MDACSASTS, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DOUTCTL", BCHP_AIF_MDAC_CAL_CORE0_DOUTCTL, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_TIMERCTL0", BCHP_AIF_MDAC_CAL_CORE0_TIMERCTL0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_TIMERCTL1", BCHP_AIF_MDAC_CAL_CORE0_TIMERCTL1, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_DGSEPCTL_SLC0", BCHP_AIF_MDAC_CAL_CORE0_DGSEPCTL_SLC0, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_REG_DGSEP_DMX0_PI_SLC0_INT_WDATA", BCHP_AIF_MDAC_CAL_CORE0_REG_DGSEP_DMX0_PI_SLC0_INT_WDATA, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_REG_DGSEP_DMX0_PO_SLC0_INT_WDATA", BCHP_AIF_MDAC_CAL_CORE0_REG_DGSEP_DMX0_PO_SLC0_INT_WDATA, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_REG_DGSEP_DMX1_PI_SLC0_INT_WDATA", BCHP_AIF_MDAC_CAL_CORE0_REG_DGSEP_DMX1_PI_SLC0_INT_WDATA, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_REG_DGSEP_DMX1_PO_SLC0_INT_WDATA", BCHP_AIF_MDAC_CAL_CORE0_REG_DGSEP_DMX1_PO_SLC0_INT_WDATA, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_REG_DGSEP_S1_DMX0_PI_SLC0_ERRP", BCHP_AIF_MDAC_CAL_CORE0_REG_DGSEP_S1_DMX0_PI_SLC0_ERRP, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_REG_DGSEP_S2_DMX0_PI_SLC0_ERRP", BCHP_AIF_MDAC_CAL_CORE0_REG_DGSEP_S2_DMX0_PI_SLC0_ERRP, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_REG_DGSEP_S1_DMX0_PO_SLC0_ERRP", BCHP_AIF_MDAC_CAL_CORE0_REG_DGSEP_S1_DMX0_PO_SLC0_ERRP, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_REG_DGSEP_S2_DMX0_PO_SLC0_ERRP", BCHP_AIF_MDAC_CAL_CORE0_REG_DGSEP_S2_DMX0_PO_SLC0_ERRP, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_REG_DGSEP_S1_DMX1_PI_SLC0_ERRP", BCHP_AIF_MDAC_CAL_CORE0_REG_DGSEP_S1_DMX1_PI_SLC0_ERRP, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_REG_DGSEP_S2_DMX1_PI_SLC0_ERRP", BCHP_AIF_MDAC_CAL_CORE0_REG_DGSEP_S2_DMX1_PI_SLC0_ERRP, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_REG_DGSEP_S1_DMX1_PO_SLC0_ERRP", BCHP_AIF_MDAC_CAL_CORE0_REG_DGSEP_S1_DMX1_PO_SLC0_ERRP, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE0_REG_DGSEP_S2_DMX1_PO_SLC0_ERRP", BCHP_AIF_MDAC_CAL_CORE0_REG_DGSEP_S2_DMX1_PO_SLC0_ERRP, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE_INTR2_0_CPU_STATUS", BCHP_AIF_MDAC_CAL_CORE_INTR2_0_CPU_STATUS, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE_INTR2_0_CPU_SET", BCHP_AIF_MDAC_CAL_CORE_INTR2_0_CPU_SET, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE_INTR2_0_CPU_CLEAR", BCHP_AIF_MDAC_CAL_CORE_INTR2_0_CPU_CLEAR, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE_INTR2_0_CPU_MASK_STATUS", BCHP_AIF_MDAC_CAL_CORE_INTR2_0_CPU_MASK_STATUS, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE_INTR2_0_CPU_MASK_SET", BCHP_AIF_MDAC_CAL_CORE_INTR2_0_CPU_MASK_SET, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE_INTR2_0_CPU_MASK_CLEAR", BCHP_AIF_MDAC_CAL_CORE_INTR2_0_CPU_MASK_CLEAR, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE_INTR2_0_PCI_STATUS", BCHP_AIF_MDAC_CAL_CORE_INTR2_0_PCI_STATUS, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE_INTR2_0_PCI_SET", BCHP_AIF_MDAC_CAL_CORE_INTR2_0_PCI_SET, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE_INTR2_0_PCI_CLEAR", BCHP_AIF_MDAC_CAL_CORE_INTR2_0_PCI_CLEAR, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE_INTR2_0_PCI_MASK_STATUS", BCHP_AIF_MDAC_CAL_CORE_INTR2_0_PCI_MASK_STATUS, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE_INTR2_0_PCI_MASK_SET", BCHP_AIF_MDAC_CAL_CORE_INTR2_0_PCI_MASK_SET, SATFE_RegisterType_eISB},
   {"AIF_MDAC_CAL_CORE_INTR2_0_PCI_MASK_CLEAR", BCHP_AIF_MDAC_CAL_CORE_INTR2_0_PCI_MASK_CLEAR, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_ANA_U1_ADC_CNTL0", BCHP_AIF_WB_SAT_ANA_U1_0_ADC_CNTL0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_ANA_U1_ADC_CNTL1", BCHP_AIF_WB_SAT_ANA_U1_0_ADC_CNTL1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_ANA_U1_ADC_CNTL2", BCHP_AIF_WB_SAT_ANA_U1_0_ADC_CNTL2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_ANA_U1_ADC_CNTL3", BCHP_AIF_WB_SAT_ANA_U1_0_ADC_CNTL3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_ANA_U1_DPM_CNTL0_IN0", BCHP_AIF_WB_SAT_ANA_U1_0_DPM_CNTL0_IN0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_ANA_U1_DPM_CNTL1_IN0", BCHP_AIF_WB_SAT_ANA_U1_0_DPM_CNTL1_IN0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_ANA_U1_DPM_PLL_CNTL0_IN0", BCHP_AIF_WB_SAT_ANA_U1_0_DPM_PLL_CNTL0_IN0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_ANA_U1_DPM_PLL_CNTL1_IN0", BCHP_AIF_WB_SAT_ANA_U1_0_DPM_PLL_CNTL1_IN0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_ANA_U1_DPM_PLL_CNTL2_IN0", BCHP_AIF_WB_SAT_ANA_U1_0_DPM_PLL_CNTL2_IN0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_ANA_U1_RFFE_CNTL0_IN0", BCHP_AIF_WB_SAT_ANA_U1_0_RFFE_CNTL0_IN0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_ANA_U1_RFFE_CNTL1_IN0", BCHP_AIF_WB_SAT_ANA_U1_0_RFFE_CNTL1_IN0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_ANA_U1_WBADC_CAL_CNTL_IN0", BCHP_AIF_WB_SAT_ANA_U1_0_WBADC_CAL_CNTL_IN0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_ANA_U1_WBADC_SYS_CNTL_IN0", BCHP_AIF_WB_SAT_ANA_U1_0_WBADC_SYS_CNTL_IN0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_ANA_U1_WBADC_TIMING_CNTL1_IN0", BCHP_AIF_WB_SAT_ANA_U1_0_WBADC_TIMING_CNTL1_IN0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_ANA_U1_WBADC_TIMING_CNTL_IN0", BCHP_AIF_WB_SAT_ANA_U1_0_WBADC_TIMING_CNTL_IN0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_ANA_U1_WBADC_TOP_CNTL_IN0", BCHP_AIF_WB_SAT_ANA_U1_0_WBADC_TOP_CNTL_IN0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_ANA_U1_CLK_CTRL_IN0", BCHP_AIF_WB_SAT_ANA_U1_0_CLK_CTRL_IN0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_ANA_U1_PGA_GAIN_IN0", BCHP_AIF_WB_SAT_ANA_U1_0_PGA_GAIN_IN0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_ANA_U1_SW_SPARE0", BCHP_AIF_WB_SAT_ANA_U1_0_SW_SPARE0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_ANA_U1_SW_SPARE1", BCHP_AIF_WB_SAT_ANA_U1_0_SW_SPARE1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_RSTCTL", BCHP_AIF_WB_SAT_CORE0_0_RSTCTL, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_HDOFFCTL0", BCHP_AIF_WB_SAT_CORE0_0_HDOFFCTL0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_HDOFFCTL1", BCHP_AIF_WB_SAT_CORE0_0_HDOFFCTL1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_HDOFFCTL2", BCHP_AIF_WB_SAT_CORE0_0_HDOFFCTL2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_HDOFFSTS", BCHP_AIF_WB_SAT_CORE0_0_HDOFFSTS, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCTL_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSCTL_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCTL_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSCTL_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCTL_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSCTL_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCTL_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSCTL_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCTL_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSCTL_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCTL_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSCTL_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCTL_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSCTL_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCTL_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSCTL_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCLP_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSCLP_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCLP_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSCLP_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCLP_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSCLP_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCLP_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSCLP_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCLP_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSCLP_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCLP_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSCLP_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCLP_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSCLP_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCLP_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSCLP_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSHIST_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSHIST_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSHIST_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSHIST_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSHIST_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSHIST_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSHIST_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSHIST_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSHIST_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSHIST_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSHIST_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSHIST_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSHIST_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSHIST_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSHIST_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSHIST_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCLPCNT_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSCLPCNT_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCLPCNT_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSCLPCNT_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCLPCNT_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSCLPCNT_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCLPCNT_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSCLPCNT_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCLPCNT_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSCLPCNT_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCLPCNT_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSCLPCNT_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCLPCNT_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSCLPCNT_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCLPCNT_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSCLPCNT_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSACCUM_DMX0_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSACCUM_DMX0_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSACCUM_DMX0_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSACCUM_DMX0_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSACCUM_DMX0_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSACCUM_DMX0_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSACCUM_DMX0_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSACCUM_DMX0_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSACCUM_DMX0_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSACCUM_DMX0_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSACCUM_DMX0_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSACCUM_DMX0_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSACCUM_DMX0_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSACCUM_DMX0_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSACCUM_DMX0_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSACCUM_DMX0_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSACCUM_DMX1_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSACCUM_DMX1_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSACCUM_DMX1_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSACCUM_DMX1_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSACCUM_DMX1_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSACCUM_DMX1_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSACCUM_DMX1_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSACCUM_DMX1_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSACCUM_DMX1_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSACCUM_DMX1_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSACCUM_DMX1_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSACCUM_DMX1_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSACCUM_DMX1_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSACCUM_DMX1_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSACCUM_DMX1_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSACCUM_DMX1_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT011_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT011_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT010_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT010_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT001_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT001_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT000_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT000_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT111_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT111_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT110_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT110_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT101_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT101_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT100_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT100_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT011_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT011_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT010_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT010_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT001_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT001_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT000_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT000_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT111_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT111_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT110_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT110_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT101_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT101_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT100_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT100_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT011_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT011_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT010_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT010_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT001_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT001_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT000_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT000_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT111_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT111_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT110_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT110_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT101_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT101_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT100_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT100_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT011_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT011_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT010_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT010_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT001_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT001_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT000_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT000_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT111_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT111_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT110_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT110_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT101_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT101_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT100_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT100_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT011_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT011_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT010_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT010_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT001_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT001_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT000_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT000_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT111_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT111_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT110_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT110_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT101_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT101_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT100_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT100_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT011_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT011_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT010_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT010_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT001_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT001_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT000_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT000_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT111_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT111_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT110_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT110_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT101_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT101_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT100_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT100_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT011_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT011_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT010_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT010_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT001_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT001_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT000_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT000_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT111_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT111_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT110_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT110_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT101_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT101_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT100_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT100_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT011_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT011_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT010_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT010_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT001_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT001_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT000_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT000_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT111_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT111_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT110_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT110_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT101_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT101_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLUT100_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSLUT100_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLMS_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSLMS_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLMS_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSLMS_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLMS_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSLMS_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLMS_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSLMS_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLMSMU_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSLMSMU_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLMSMU_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSLMSMU_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLMSMU_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSLMSMU_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSLMSMU_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSLMSMU_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCOEFD_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSCOEFD_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCOEFA_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSCOEFA_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCOEFD_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSCOEFD_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCOEFA_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSCOEFA_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCOEFD_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSCOEFD_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCOEFA_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSCOEFA_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCOEFD_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSCOEFD_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCOEFA_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSCOEFA_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCOEFEN_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSCOEFEN_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCOEFEN_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSCOEFEN_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCOEFEN_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSCOEFEN_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSCOEFEN_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSCOEFEN_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_MDACSA_SLC0", BCHP_AIF_WB_SAT_CORE0_0_MDACSA_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_MDACSA_SLC1", BCHP_AIF_WB_SAT_CORE0_0_MDACSA_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_MDACSA_SLC2", BCHP_AIF_WB_SAT_CORE0_0_MDACSA_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_MDACSA_SLC3", BCHP_AIF_WB_SAT_CORE0_0_MDACSA_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_MDACSADU_SLC0", BCHP_AIF_WB_SAT_CORE0_0_MDACSADU_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_MDACSADU_SLC1", BCHP_AIF_WB_SAT_CORE0_0_MDACSADU_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_MDACSADU_SLC2", BCHP_AIF_WB_SAT_CORE0_0_MDACSADU_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_MDACSADU_SLC3", BCHP_AIF_WB_SAT_CORE0_0_MDACSADU_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_MDACSAOUT_SLC0", BCHP_AIF_WB_SAT_CORE0_0_MDACSAOUT_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_MDACSAOUT_SLC1", BCHP_AIF_WB_SAT_CORE0_0_MDACSAOUT_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_MDACSAOUT_SLC2", BCHP_AIF_WB_SAT_CORE0_0_MDACSAOUT_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_MDACSAOUT_SLC3", BCHP_AIF_WB_SAT_CORE0_0_MDACSAOUT_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_MDACSASTS", BCHP_AIF_WB_SAT_CORE0_0_MDACSASTS, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_LICCTL1", BCHP_AIF_WB_SAT_CORE0_0_LICCTL1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_LICCTL2", BCHP_AIF_WB_SAT_CORE0_0_LICCTL2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_LICCTL3", BCHP_AIF_WB_SAT_CORE0_0_LICCTL3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_LICCTL4", BCHP_AIF_WB_SAT_CORE0_0_LICCTL4, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_LICCTL5", BCHP_AIF_WB_SAT_CORE0_0_LICCTL5, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_LICCOEFD", BCHP_AIF_WB_SAT_CORE0_0_LICCOEFD, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_LICCOEFA", BCHP_AIF_WB_SAT_CORE0_0_LICCOEFA, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_LICCOEFEN", BCHP_AIF_WB_SAT_CORE0_0_LICCOEFEN, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRINSEL", BCHP_AIF_WB_SAT_CORE0_0_CORRINSEL, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRCTL", BCHP_AIF_WB_SAT_CORE0_0_CORRCTL, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRFCW", BCHP_AIF_WB_SAT_CORE0_0_CORRFCW, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRTHR", BCHP_AIF_WB_SAT_CORE0_0_CORRTHR, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRFCWEN", BCHP_AIF_WB_SAT_CORE0_0_CORRFCWEN, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRLEN0", BCHP_AIF_WB_SAT_CORE0_0_CORRLEN0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRLEN1", BCHP_AIF_WB_SAT_CORE0_0_CORRLEN1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI0_DMX0_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_CORRI0_DMX0_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI1_DMX0_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_CORRI1_DMX0_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI0_DMX0_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_CORRI0_DMX0_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI1_DMX0_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_CORRI1_DMX0_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI0_DMX0_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_CORRI0_DMX0_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI1_DMX0_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_CORRI1_DMX0_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI0_DMX0_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_CORRI0_DMX0_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI1_DMX0_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_CORRI1_DMX0_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI0_DMX0_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_CORRI0_DMX0_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI1_DMX0_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_CORRI1_DMX0_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI0_DMX0_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_CORRI0_DMX0_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI1_DMX0_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_CORRI1_DMX0_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI0_DMX0_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_CORRI0_DMX0_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI1_DMX0_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_CORRI1_DMX0_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI0_DMX0_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_CORRI0_DMX0_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI1_DMX0_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_CORRI1_DMX0_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI0_DMX1_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_CORRI0_DMX1_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI1_DMX1_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_CORRI1_DMX1_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI0_DMX1_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_CORRI0_DMX1_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI1_DMX1_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_CORRI1_DMX1_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI0_DMX1_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_CORRI0_DMX1_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI1_DMX1_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_CORRI1_DMX1_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI0_DMX1_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_CORRI0_DMX1_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI1_DMX1_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_CORRI1_DMX1_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI0_DMX1_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_CORRI0_DMX1_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI1_DMX1_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_CORRI1_DMX1_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI0_DMX1_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_CORRI0_DMX1_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI1_DMX1_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_CORRI1_DMX1_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI0_DMX1_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_CORRI0_DMX1_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI1_DMX1_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_CORRI1_DMX1_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI0_DMX1_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_CORRI0_DMX1_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRI1_DMX1_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_CORRI1_DMX1_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ0_DMX0_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_CORRQ0_DMX0_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ1_DMX0_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_CORRQ1_DMX0_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ0_DMX0_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_CORRQ0_DMX0_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ1_DMX0_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_CORRQ1_DMX0_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ0_DMX0_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_CORRQ0_DMX0_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ1_DMX0_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_CORRQ1_DMX0_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ0_DMX0_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_CORRQ0_DMX0_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ1_DMX0_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_CORRQ1_DMX0_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ0_DMX0_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_CORRQ0_DMX0_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ1_DMX0_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_CORRQ1_DMX0_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ0_DMX0_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_CORRQ0_DMX0_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ1_DMX0_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_CORRQ1_DMX0_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ0_DMX0_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_CORRQ0_DMX0_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ1_DMX0_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_CORRQ1_DMX0_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ0_DMX0_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_CORRQ0_DMX0_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ1_DMX0_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_CORRQ1_DMX0_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ0_DMX1_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_CORRQ0_DMX1_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ1_DMX1_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_CORRQ1_DMX1_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ0_DMX1_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_CORRQ0_DMX1_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ1_DMX1_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_CORRQ1_DMX1_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ0_DMX1_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_CORRQ0_DMX1_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ1_DMX1_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_CORRQ1_DMX1_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ0_DMX1_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_CORRQ0_DMX1_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ1_DMX1_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_CORRQ1_DMX1_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ0_DMX1_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_CORRQ0_DMX1_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ1_DMX1_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_CORRQ1_DMX1_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ0_DMX1_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_CORRQ0_DMX1_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ1_DMX1_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_CORRQ1_DMX1_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ0_DMX1_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_CORRQ0_DMX1_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ1_DMX1_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_CORRQ1_DMX1_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ0_DMX1_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_CORRQ0_DMX1_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRQ1_DMX1_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_CORRQ1_DMX1_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP0_DMX0_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_CORRP0_DMX0_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP1_DMX0_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_CORRP1_DMX0_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP0_DMX0_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_CORRP0_DMX0_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP1_DMX0_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_CORRP1_DMX0_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP0_DMX0_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_CORRP0_DMX0_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP1_DMX0_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_CORRP1_DMX0_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP0_DMX0_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_CORRP0_DMX0_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP1_DMX0_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_CORRP1_DMX0_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP0_DMX0_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_CORRP0_DMX0_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP1_DMX0_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_CORRP1_DMX0_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP0_DMX0_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_CORRP0_DMX0_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP1_DMX0_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_CORRP1_DMX0_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP0_DMX0_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_CORRP0_DMX0_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP1_DMX0_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_CORRP1_DMX0_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP0_DMX0_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_CORRP0_DMX0_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP1_DMX0_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_CORRP1_DMX0_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP0_DMX1_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_CORRP0_DMX1_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP1_DMX1_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_CORRP1_DMX1_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP0_DMX1_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_CORRP0_DMX1_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP1_DMX1_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_CORRP1_DMX1_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP0_DMX1_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_CORRP0_DMX1_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP1_DMX1_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_CORRP1_DMX1_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP0_DMX1_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_CORRP0_DMX1_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP1_DMX1_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_CORRP1_DMX1_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP0_DMX1_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_CORRP0_DMX1_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP1_DMX1_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_CORRP1_DMX1_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP0_DMX1_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_CORRP0_DMX1_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP1_DMX1_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_CORRP1_DMX1_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP0_DMX1_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_CORRP0_DMX1_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP1_DMX1_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_CORRP1_DMX1_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP0_DMX1_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_CORRP0_DMX1_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORRP1_DMX1_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_CORRP1_DMX1_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_TIMERCTL0", BCHP_AIF_WB_SAT_CORE0_0_TIMERCTL0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_TIMERCTL1", BCHP_AIF_WB_SAT_CORE0_0_TIMERCTL1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSEPCTL_SLC0", BCHP_AIF_WB_SAT_CORE0_0_DGSEPCTL_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_DMX0_PI_SLC0_INT_WDATA", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_DMX0_PI_SLC0_INT_WDATA, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_DMX0_PO_SLC0_INT_WDATA", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_DMX0_PO_SLC0_INT_WDATA, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_DMX1_PI_SLC0_INT_WDATA", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_DMX1_PI_SLC0_INT_WDATA, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_DMX1_PO_SLC0_INT_WDATA", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_DMX1_PO_SLC0_INT_WDATA, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S1_DMX0_PI_SLC0_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S1_DMX0_PI_SLC0_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S2_DMX0_PI_SLC0_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S2_DMX0_PI_SLC0_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S1_DMX0_PO_SLC0_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S1_DMX0_PO_SLC0_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S2_DMX0_PO_SLC0_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S2_DMX0_PO_SLC0_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S1_DMX1_PI_SLC0_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S1_DMX1_PI_SLC0_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S2_DMX1_PI_SLC0_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S2_DMX1_PI_SLC0_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S1_DMX1_PO_SLC0_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S1_DMX1_PO_SLC0_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S2_DMX1_PO_SLC0_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S2_DMX1_PO_SLC0_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSEPCTL_SLC1", BCHP_AIF_WB_SAT_CORE0_0_DGSEPCTL_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_DMX0_PI_SLC1_INT_WDATA", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_DMX0_PI_SLC1_INT_WDATA, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_DMX0_PO_SLC1_INT_WDATA", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_DMX0_PO_SLC1_INT_WDATA, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_DMX1_PI_SLC1_INT_WDATA", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_DMX1_PI_SLC1_INT_WDATA, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_DMX1_PO_SLC1_INT_WDATA", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_DMX1_PO_SLC1_INT_WDATA, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S1_DMX0_PI_SLC1_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S1_DMX0_PI_SLC1_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S2_DMX0_PI_SLC1_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S2_DMX0_PI_SLC1_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S1_DMX0_PO_SLC1_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S1_DMX0_PO_SLC1_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S2_DMX0_PO_SLC1_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S2_DMX0_PO_SLC1_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S1_DMX1_PI_SLC1_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S1_DMX1_PI_SLC1_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S2_DMX1_PI_SLC1_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S2_DMX1_PI_SLC1_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S1_DMX1_PO_SLC1_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S1_DMX1_PO_SLC1_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_0_REG_DGSEP_S2_DMX1_PO_SLC1_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S2_DMX1_PO_SLC1_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSEPCTL_SLC2", BCHP_AIF_WB_SAT_CORE0_0_DGSEPCTL_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_DMX0_PI_SLC2_INT_WDATA", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_DMX0_PI_SLC2_INT_WDATA, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_DMX0_PO_SLC2_INT_WDATA", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_DMX0_PO_SLC2_INT_WDATA, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_DMX1_PI_SLC2_INT_WDATA", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_DMX1_PI_SLC2_INT_WDATA, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_DMX1_PO_SLC2_INT_WDATA", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_DMX1_PO_SLC2_INT_WDATA, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S1_DMX0_PI_SLC2_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S1_DMX0_PI_SLC2_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S2_DMX0_PI_SLC2_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S2_DMX0_PI_SLC2_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S1_DMX0_PO_SLC2_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S1_DMX0_PO_SLC2_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S2_DMX0_PO_SLC2_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S2_DMX0_PO_SLC2_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S1_DMX1_PI_SLC2_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S1_DMX1_PI_SLC2_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S2_DMX1_PI_SLC2_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S2_DMX1_PI_SLC2_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S1_DMX1_PO_SLC2_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S1_DMX1_PO_SLC2_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S2_DMX1_PO_SLC2_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S2_DMX1_PO_SLC2_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_DGSEPCTL_SLC3", BCHP_AIF_WB_SAT_CORE0_0_DGSEPCTL_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_DMX0_PI_SLC3_INT_WDATA", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_DMX0_PI_SLC3_INT_WDATA, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_DMX0_PO_SLC3_INT_WDATA", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_DMX0_PO_SLC3_INT_WDATA, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_DMX1_PI_SLC3_INT_WDATA", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_DMX1_PI_SLC3_INT_WDATA, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_DMX1_PO_SLC3_INT_WDATA", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_DMX1_PO_SLC3_INT_WDATA, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S1_DMX0_PI_SLC3_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S1_DMX0_PI_SLC3_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S2_DMX0_PI_SLC3_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S2_DMX0_PI_SLC3_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S1_DMX0_PO_SLC3_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S1_DMX0_PO_SLC3_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S2_DMX0_PO_SLC3_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S2_DMX0_PO_SLC3_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S1_DMX1_PI_SLC3_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S1_DMX1_PI_SLC3_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S2_DMX1_PI_SLC3_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S2_DMX1_PI_SLC3_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S1_DMX1_PO_SLC3_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S1_DMX1_PO_SLC3_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_DGSEP_S2_DMX1_PO_SLC3_ERRP", BCHP_AIF_WB_SAT_CORE0_0_REG_DGSEP_S2_DMX1_PO_SLC3_ERRP, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_AGCCTL2", BCHP_AIF_WB_SAT_CORE0_0_AGCCTL2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_AGCDECRATE", BCHP_AIF_WB_SAT_CORE0_0_AGCDECRATE, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_AGCCTL1", BCHP_AIF_WB_SAT_CORE0_0_AGCCTL1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_AGCTHRA1", BCHP_AIF_WB_SAT_CORE0_0_AGCTHRA1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_AGC_LF_INT_WDATA", BCHP_AIF_WB_SAT_CORE0_0_REG_AGC_LF_INT_WDATA, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_AGC_LA_INT_WDATA", BCHP_AIF_WB_SAT_CORE0_0_REG_AGC_LA_INT_WDATA, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_AGC_CTRL_LF_INT_WDATA", BCHP_AIF_WB_SAT_CORE0_0_REG_AGC_CTRL_LF_INT_WDATA, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_AGC_CTRL_LA_INT_WDATA", BCHP_AIF_WB_SAT_CORE0_0_REG_AGC_CTRL_LA_INT_WDATA, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_AGCTHRA2", BCHP_AIF_WB_SAT_CORE0_0_AGCTHRA2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_NRNOTCHCTL", BCHP_AIF_WB_SAT_CORE0_0_NRNOTCHCTL, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_NRAGCTHR", BCHP_AIF_WB_SAT_CORE0_0_NRAGCTHR, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_NRDCOCTL_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_NRDCOCTL_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_DCO_INT_WDATA_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_DCO_INT_WDATA_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_NRDCOCTL_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_NRDCOCTL_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_DCO_INT_WDATA_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_DCO_INT_WDATA_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_NRDCOCTL_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_NRDCOCTL_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_DCO_INT_WDATA_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_DCO_INT_WDATA_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_NRDCOCTL_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_NRDCOCTL_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_DCO_INT_WDATA_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_DCO_INT_WDATA_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_NRDCOCTL_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_NRDCOCTL_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_DCO_INT_WDATA_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_DCO_INT_WDATA_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_NRDCOCTL_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_NRDCOCTL_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_DCO_INT_WDATA_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_DCO_INT_WDATA_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_NRDCOCTL_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_NRDCOCTL_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_DCO_INT_WDATA_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_DCO_INT_WDATA_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_NRDCOCTL_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_NRDCOCTL_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_DCO_INT_WDATA_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_DCO_INT_WDATA_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_NRNOTCHCTL_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_NRNOTCHCTL_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_NOTCH_INT_WDATA_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_NOTCH_INT_WDATA_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_NRNOTCHCTL_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_NRNOTCHCTL_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_NOTCH_INT_WDATA_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_NOTCH_INT_WDATA_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_NRNOTCHCTL_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_NRNOTCHCTL_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_NOTCH_INT_WDATA_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_NOTCH_INT_WDATA_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_NRNOTCHCTL_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_NRNOTCHCTL_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_NOTCH_INT_WDATA_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_NOTCH_INT_WDATA_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_NRNOTCHCTL_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_NRNOTCHCTL_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_NOTCH_INT_WDATA_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_NOTCH_INT_WDATA_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_NRNOTCHCTL_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_NRNOTCHCTL_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_NOTCH_INT_WDATA_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_NOTCH_INT_WDATA_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_NRNOTCHCTL_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_NRNOTCHCTL_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_NOTCH_INT_WDATA_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_NOTCH_INT_WDATA_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_NRNOTCHCTL_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_NRNOTCHCTL_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_NOTCH_INT_WDATA_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_NOTCH_INT_WDATA_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_NRDCOCTL_THR", BCHP_AIF_WB_SAT_CORE0_0_NRDCOCTL_THR, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_DCO_INT_WDATA_THR", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_DCO_INT_WDATA_THR, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_NRAGCCTL_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_NRAGCCTL_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_AGC_LF_INT_WDATA_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_AGC_LF_INT_WDATA_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_AGC_LA_INT_WDATA_PI_SLC0", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_AGC_LA_INT_WDATA_PI_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_NRAGCCTL_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_NRAGCCTL_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_AGC_LF_INT_WDATA_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_AGC_LF_INT_WDATA_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_AGC_LA_INT_WDATA_PO_SLC0", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_AGC_LA_INT_WDATA_PO_SLC0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_NRAGCCTL_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_NRAGCCTL_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_AGC_LF_INT_WDATA_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_AGC_LF_INT_WDATA_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_AGC_LA_INT_WDATA_PI_SLC1", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_AGC_LA_INT_WDATA_PI_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_NRAGCCTL_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_NRAGCCTL_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_AGC_LF_INT_WDATA_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_AGC_LF_INT_WDATA_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_AGC_LA_INT_WDATA_PO_SLC1", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_AGC_LA_INT_WDATA_PO_SLC1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_NRAGCCTL_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_NRAGCCTL_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_AGC_LF_INT_WDATA_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_AGC_LF_INT_WDATA_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_AGC_LA_INT_WDATA_PI_SLC2", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_AGC_LA_INT_WDATA_PI_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_NRAGCCTL_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_NRAGCCTL_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_AGC_LF_INT_WDATA_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_AGC_LF_INT_WDATA_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_AGC_LA_INT_WDATA_PO_SLC2", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_AGC_LA_INT_WDATA_PO_SLC2, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_NRAGCCTL_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_NRAGCCTL_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_AGC_LF_INT_WDATA_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_AGC_LF_INT_WDATA_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_AGC_LA_INT_WDATA_PI_SLC3", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_AGC_LA_INT_WDATA_PI_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_NRAGCCTL_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_NRAGCCTL_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_AGC_LF_INT_WDATA_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_AGC_LF_INT_WDATA_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_REG_NR_AGC_LA_INT_WDATA_PO_SLC3", BCHP_AIF_WB_SAT_CORE0_0_REG_NR_AGC_LA_INT_WDATA_PO_SLC3, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORE_SW_SPARE0", BCHP_AIF_WB_SAT_CORE0_0_CORE_SW_SPARE0, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE0_CORE_SW_SPARE1", BCHP_AIF_WB_SAT_CORE0_0_CORE_SW_SPARE1, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE_INTR2_0_CPU_STATUS", BCHP_AIF_WB_SAT_CORE_INTR2_0_0_CPU_STATUS, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE_INTR2_0_CPU_SET", BCHP_AIF_WB_SAT_CORE_INTR2_0_0_CPU_SET, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE_INTR2_0_CPU_CLEAR", BCHP_AIF_WB_SAT_CORE_INTR2_0_0_CPU_CLEAR, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE_INTR2_0_CPU_MASK_STATUS", BCHP_AIF_WB_SAT_CORE_INTR2_0_0_CPU_MASK_STATUS, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE_INTR2_0_CPU_MASK_SET", BCHP_AIF_WB_SAT_CORE_INTR2_0_0_CPU_MASK_SET, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE_INTR2_0_CPU_MASK_CLEAR", BCHP_AIF_WB_SAT_CORE_INTR2_0_0_CPU_MASK_CLEAR, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE_INTR2_0_PCI_STATUS", BCHP_AIF_WB_SAT_CORE_INTR2_0_0_PCI_STATUS, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE_INTR2_0_PCI_SET", BCHP_AIF_WB_SAT_CORE_INTR2_0_0_PCI_SET, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE_INTR2_0_PCI_CLEAR", BCHP_AIF_WB_SAT_CORE_INTR2_0_0_PCI_CLEAR, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE_INTR2_0_PCI_MASK_STATUS", BCHP_AIF_WB_SAT_CORE_INTR2_0_0_PCI_MASK_STATUS, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE_INTR2_0_PCI_MASK_SET", BCHP_AIF_WB_SAT_CORE_INTR2_0_0_PCI_MASK_SET, SATFE_RegisterType_eISB},
   {"AIF_WB_SAT_CORE_INTR2_0_PCI_MASK_CLEAR", BCHP_AIF_WB_SAT_CORE_INTR2_0_0_PCI_MASK_CLEAR, SATFE_RegisterType_eISB},

#if BCHP_CHIP==4538
   {"AVS_ASB_REGISTERS_ASB_COMMAND", BCHP_AVS_ASB_REGISTERS_ASB_COMMAND, SATFE_RegisterType_eISB},
   {"AVS_ASB_REGISTERS_ASB_ADDRESS", BCHP_AVS_ASB_REGISTERS_ASB_ADDRESS, SATFE_RegisterType_eISB},
   {"AVS_ASB_REGISTERS_ASB_DATA_PWD_CONFIG_0", BCHP_AVS_ASB_REGISTERS_ASB_DATA_PWD_CONFIG_0, SATFE_RegisterType_eISB},
   {"AVS_ASB_REGISTERS_ASB_DATA_PWD_CONFIG_1", BCHP_AVS_ASB_REGISTERS_ASB_DATA_PWD_CONFIG_1, SATFE_RegisterType_eISB},
   {"AVS_ASB_REGISTERS_ASB_DATA_WRAPPER_CONFIG", BCHP_AVS_ASB_REGISTERS_ASB_DATA_WRAPPER_CONFIG, SATFE_RegisterType_eISB},
   {"AVS_ASB_REGISTERS_ASB_DATA_PWD_EN", BCHP_AVS_ASB_REGISTERS_ASB_DATA_PWD_EN, SATFE_RegisterType_eISB},
   {"AVS_ASB_REGISTERS_ASB_BUSY", BCHP_AVS_ASB_REGISTERS_ASB_BUSY, SATFE_RegisterType_eISB},
   {"AVS_HW_MNTR_SW_CONTROLS", BCHP_AVS_HW_MNTR_SW_CONTROLS, SATFE_RegisterType_eISB},
   {"AVS_HW_MNTR_SW_MEASUREMENT_UNIT_BUSY", BCHP_AVS_HW_MNTR_SW_MEASUREMENT_UNIT_BUSY, SATFE_RegisterType_eISB},
   {"AVS_HW_MNTR_MEASUREMENTS_INIT_PVT_MNTR", BCHP_AVS_HW_MNTR_MEASUREMENTS_INIT_PVT_MNTR, SATFE_RegisterType_eISB},
   {"AVS_HW_MNTR_MEASUREMENTS_INIT_CEN_ROSC_0", BCHP_AVS_HW_MNTR_MEASUREMENTS_INIT_CEN_ROSC_0, SATFE_RegisterType_eISB},
   {"AVS_HW_MNTR_MEASUREMENTS_INIT_CEN_ROSC_1", BCHP_AVS_HW_MNTR_MEASUREMENTS_INIT_CEN_ROSC_1, SATFE_RegisterType_eISB},
   {"AVS_HW_MNTR_MEASUREMENTS_INIT_RMT_ROSC_0", BCHP_AVS_HW_MNTR_MEASUREMENTS_INIT_RMT_ROSC_0, SATFE_RegisterType_eISB},
   {"AVS_HW_MNTR_MEASUREMENTS_INIT_RMT_ROSC_1", BCHP_AVS_HW_MNTR_MEASUREMENTS_INIT_RMT_ROSC_1, SATFE_RegisterType_eISB},
   {"AVS_HW_MNTR_MEASUREMENTS_INIT_POW_WDOG", BCHP_AVS_HW_MNTR_MEASUREMENTS_INIT_POW_WDOG, SATFE_RegisterType_eISB},
   {"AVS_HW_MNTR_SEQUENCER_INIT", BCHP_AVS_HW_MNTR_SEQUENCER_INIT, SATFE_RegisterType_eISB},
   {"AVS_HW_MNTR_SEQUENCER_MASK_PVT_MNTR", BCHP_AVS_HW_MNTR_SEQUENCER_MASK_PVT_MNTR, SATFE_RegisterType_eISB},
   {"AVS_HW_MNTR_SEQUENCER_MASK_CEN_ROSC_0", BCHP_AVS_HW_MNTR_SEQUENCER_MASK_CEN_ROSC_0, SATFE_RegisterType_eISB},
   {"AVS_HW_MNTR_SEQUENCER_MASK_CEN_ROSC_1", BCHP_AVS_HW_MNTR_SEQUENCER_MASK_CEN_ROSC_1, SATFE_RegisterType_eISB},
   {"AVS_HW_MNTR_SEQUENCER_MASK_RMT_ROSC_0", BCHP_AVS_HW_MNTR_SEQUENCER_MASK_RMT_ROSC_0, SATFE_RegisterType_eISB},
   {"AVS_HW_MNTR_SEQUENCER_MASK_RMT_ROSC_1", BCHP_AVS_HW_MNTR_SEQUENCER_MASK_RMT_ROSC_1, SATFE_RegisterType_eISB},
   {"AVS_HW_MNTR_ENABLE_DEFAULT_PVT_MNTR", BCHP_AVS_HW_MNTR_ENABLE_DEFAULT_PVT_MNTR, SATFE_RegisterType_eISB},
   {"AVS_HW_MNTR_ENABLE_DEFAULT_CEN_ROSC_0", BCHP_AVS_HW_MNTR_ENABLE_DEFAULT_CEN_ROSC_0, SATFE_RegisterType_eISB},
   {"AVS_HW_MNTR_ENABLE_DEFAULT_CEN_ROSC_1", BCHP_AVS_HW_MNTR_ENABLE_DEFAULT_CEN_ROSC_1, SATFE_RegisterType_eISB},
   {"AVS_HW_MNTR_ROSC_MEASUREMENT_TIME_CONTROL", BCHP_AVS_HW_MNTR_ROSC_MEASUREMENT_TIME_CONTROL, SATFE_RegisterType_eISB},
   {"AVS_HW_MNTR_ROSC_COUNTING_MODE", BCHP_AVS_HW_MNTR_ROSC_COUNTING_MODE, SATFE_RegisterType_eISB},
   {"AVS_HW_MNTR_INTERRUPT_POW_WDOG_EN", BCHP_AVS_HW_MNTR_INTERRUPT_POW_WDOG_EN, SATFE_RegisterType_eISB},
   {"AVS_HW_MNTR_INTERRUPT_SW_MEASUREMENT_DONE_EN", BCHP_AVS_HW_MNTR_INTERRUPT_SW_MEASUREMENT_DONE_EN, SATFE_RegisterType_eISB},
   {"AVS_HW_MNTR_LAST_MEASURED_SENSOR", BCHP_AVS_HW_MNTR_LAST_MEASURED_SENSOR, SATFE_RegisterType_eISB},
   {"AVS_HW_MNTR_AVS_INTERRUPT_FLAGS", BCHP_AVS_HW_MNTR_AVS_INTERRUPT_FLAGS, SATFE_RegisterType_eISB},
   {"AVS_HW_MNTR_AVS_INTERRUPT_FLAGS_CLEAR", BCHP_AVS_HW_MNTR_AVS_INTERRUPT_FLAGS_CLEAR, SATFE_RegisterType_eISB},
   {"AVS_HW_MNTR_REMOTE_SENSOR_TYPE", BCHP_AVS_HW_MNTR_REMOTE_SENSOR_TYPE, SATFE_RegisterType_eISB},
   {"AVS_HW_MNTR_AVS_REGISTERS_LOCKS", BCHP_AVS_HW_MNTR_AVS_REGISTERS_LOCKS, SATFE_RegisterType_eISB},
   {"AVS_HW_MNTR_TEMPERATURE_RESET_ENABLE", BCHP_AVS_HW_MNTR_TEMPERATURE_RESET_ENABLE, SATFE_RegisterType_eISB},
   {"AVS_HW_MNTR_TEMPERATURE_THRESHOLD", BCHP_AVS_HW_MNTR_TEMPERATURE_THRESHOLD, SATFE_RegisterType_eISB},
   {"AVS_HW_MNTR_IDLE_STATE_0_CEN_ROSC_0", BCHP_AVS_HW_MNTR_IDLE_STATE_0_CEN_ROSC_0, SATFE_RegisterType_eISB},
   {"AVS_HW_MNTR_IDLE_STATE_0_CEN_ROSC_1", BCHP_AVS_HW_MNTR_IDLE_STATE_0_CEN_ROSC_1, SATFE_RegisterType_eISB},
   {"AVS_PVT_MNTR_CONFIG_PVT_MNTR_CTRL", BCHP_AVS_PVT_MNTR_CONFIG_PVT_MNTR_CTRL, SATFE_RegisterType_eISB},
   {"AVS_PVT_MNTR_CONFIG_PVT_MNTR_TP_MODE_ENABLE", BCHP_AVS_PVT_MNTR_CONFIG_PVT_MNTR_TP_MODE_ENABLE, SATFE_RegisterType_eISB},
   {"AVS_PVT_MNTR_CONFIG_DAC_CODE_PROGRAMMING_ENABLE", BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE_PROGRAMMING_ENABLE, SATFE_RegisterType_eISB},
   {"AVS_PVT_MNTR_CONFIG_DAC_CODE", BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE, SATFE_RegisterType_eISB},
   {"AVS_PVT_MNTR_CONFIG_MIN_DAC_CODE", BCHP_AVS_PVT_MNTR_CONFIG_MIN_DAC_CODE, SATFE_RegisterType_eISB},
   {"AVS_PVT_MNTR_CONFIG_MAX_DAC_CODE", BCHP_AVS_PVT_MNTR_CONFIG_MAX_DAC_CODE, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_0", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_0, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_1", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_1, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_2", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_2, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_3", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_3, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_4", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_4, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_5", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_5, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_6", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_6, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_7", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_7, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_8", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_8, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_9", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_9, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_10", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_10, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_11", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_11, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_12", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_12, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_13", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_13, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_14", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_14, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_15", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_15, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_16", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_16, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_17", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_17, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_18", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_18, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_19", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_19, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_20", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_20, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_21", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_21, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_22", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_22, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_23", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_23, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_24", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_24, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_25", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_25, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_26", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_26, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_27", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_27, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_28", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_28, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_29", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_29, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_30", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_30, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_31", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_31, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_32", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_32, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_33", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_33, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_34", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_34, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_35", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_35, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_36", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_36, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_37", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_37, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_38", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_38, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_39", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_39, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_40", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_40, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_41", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_41, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_42", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_42, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_43", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_43, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_44", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_44, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_45", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_45, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_46", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_46, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_47", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_47, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_RMT_ROSC_GH", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_RMT_ROSC_GH, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_RMT_ROSC_GS", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_RMT_ROSC_GS, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_RMT_ROSC_G8H", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_RMT_ROSC_G8H, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_RMT_ROSC_G8S", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_RMT_ROSC_G8S, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_CEN_ROSC_THRESHOLD1_EN_0", BCHP_AVS_ROSC_THRESHOLD_1_CEN_ROSC_THRESHOLD1_EN_0, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_CEN_ROSC_THRESHOLD1_EN_1", BCHP_AVS_ROSC_THRESHOLD_1_CEN_ROSC_THRESHOLD1_EN_1, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_RMT_ROSC_THRESHOLD1_EN_0", BCHP_AVS_ROSC_THRESHOLD_1_RMT_ROSC_THRESHOLD1_EN_0, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_RMT_ROSC_THRESHOLD1_EN_1", BCHP_AVS_ROSC_THRESHOLD_1_RMT_ROSC_THRESHOLD1_EN_1, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_THRESHOLD1_DIRECTION", BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_DIRECTION, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_1_INTERRUPT_STATUS_THRESHOLD1_FAULTY_SENSOR", BCHP_AVS_ROSC_THRESHOLD_1_INTERRUPT_STATUS_THRESHOLD1_FAULTY_SENSOR, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_0", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_0, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_1", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_1, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_2", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_2, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_3", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_3, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_4", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_4, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_5", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_5, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_6", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_6, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_7", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_7, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_8", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_8, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_9", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_9, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_10", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_10, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_11", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_11, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_12", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_12, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_13", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_13, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_14", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_14, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_15", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_15, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_16", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_16, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_17", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_17, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_18", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_18, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_19", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_19, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_20", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_20, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_21", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_21, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_22", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_22, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_23", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_23, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_24", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_24, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_25", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_25, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_26", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_26, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_27", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_27, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_28", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_28, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_29", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_29, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_30", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_30, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_31", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_31, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_32", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_32, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_33", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_33, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_34", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_34, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_35", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_35, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_36", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_36, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_37", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_37, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_38", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_38, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_39", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_39, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_40", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_40, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_41", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_41, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_42", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_42, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_43", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_43, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_44", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_44, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_45", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_45, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_46", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_46, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_47", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_47, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_RMT_ROSC_GH", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_RMT_ROSC_GH, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_RMT_ROSC_GS", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_RMT_ROSC_GS, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_RMT_ROSC_G8H", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_RMT_ROSC_G8H, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_RMT_ROSC_G8S", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_RMT_ROSC_G8S, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_CEN_ROSC_THRESHOLD2_EN_0", BCHP_AVS_ROSC_THRESHOLD_2_CEN_ROSC_THRESHOLD2_EN_0, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_CEN_ROSC_THRESHOLD2_EN_1", BCHP_AVS_ROSC_THRESHOLD_2_CEN_ROSC_THRESHOLD2_EN_1, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_RMT_ROSC_THRESHOLD2_EN_0", BCHP_AVS_ROSC_THRESHOLD_2_RMT_ROSC_THRESHOLD2_EN_0, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_RMT_ROSC_THRESHOLD2_EN_1", BCHP_AVS_ROSC_THRESHOLD_2_RMT_ROSC_THRESHOLD2_EN_1, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_THRESHOLD2_DIRECTION", BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_DIRECTION, SATFE_RegisterType_eISB},
   {"AVS_ROSC_THRESHOLD_2_INTERRUPT_STATUS_THRESHOLD2_FAULTY_SENSOR", BCHP_AVS_ROSC_THRESHOLD_2_INTERRUPT_STATUS_THRESHOLD2_FAULTY_SENSOR, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_PVT_TEMPERATURE_MNTR_STATUS", BCHP_AVS_RO_REGISTERS_0_PVT_TEMPERATURE_MNTR_STATUS, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_PVT_PROCESS_MNTR_STATUS", BCHP_AVS_RO_REGISTERS_0_PVT_PROCESS_MNTR_STATUS, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_PVT_0P99V_MNTR_STATUS", BCHP_AVS_RO_REGISTERS_0_PVT_0P99V_MNTR_STATUS, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_PVT_1P10V_0_MNTR_STATUS", BCHP_AVS_RO_REGISTERS_0_PVT_1P10V_0_MNTR_STATUS, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_PVT_1P10V_1_MNTR_STATUS", BCHP_AVS_RO_REGISTERS_0_PVT_1P10V_1_MNTR_STATUS, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_PVT_2p75V_MNTR_STATUS", BCHP_AVS_RO_REGISTERS_0_PVT_2p75V_MNTR_STATUS, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_PVT_3p63V_MNTR_STATUS", BCHP_AVS_RO_REGISTERS_0_PVT_3p63V_MNTR_STATUS, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_PVT_TESTMODE_MNTR_STATUS", BCHP_AVS_RO_REGISTERS_0_PVT_TESTMODE_MNTR_STATUS, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_0", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_0, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_1", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_1, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_2", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_2, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_3", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_3, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_4", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_4, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_5", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_5, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_6", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_6, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_7", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_7, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_8", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_8, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_9", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_9, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_10", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_10, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_11", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_11, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_12", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_12, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_13", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_13, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_14", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_14, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_15", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_15, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_16", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_16, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_17", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_17, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_18", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_18, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_19", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_19, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_20", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_20, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_21", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_21, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_22", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_22, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_23", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_23, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_24", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_24, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_25", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_25, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_26", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_26, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_27", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_27, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_28", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_28, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_29", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_29, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_30", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_30, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_31", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_31, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_32", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_32, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_33", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_33, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_34", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_34, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_35", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_35, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_36", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_36, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_37", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_37, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_38", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_38, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_39", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_39, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_40", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_40, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_41", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_41, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_42", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_42, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_43", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_43, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_44", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_44, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_45", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_45, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_46", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_46, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_47", BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_47, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_POW_WDOG_FAILURE_STATUS", BCHP_AVS_RO_REGISTERS_1_POW_WDOG_FAILURE_STATUS, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_INTERRUPT_STATUS_FAULTY_POW_WDOG", BCHP_AVS_RO_REGISTERS_1_INTERRUPT_STATUS_FAULTY_POW_WDOG, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_0", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_0, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_1", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_1, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_2", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_2, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_3", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_3, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_4", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_4, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_5", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_5, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_6", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_6, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_7", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_7, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_8", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_8, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_9", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_9, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_10", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_10, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_11", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_11, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_12", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_12, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_13", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_13, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_14", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_14, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_15", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_15, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_16", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_16, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_17", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_17, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_18", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_18, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_19", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_19, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_20", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_20, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_21", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_21, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_22", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_22, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_23", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_23, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_24", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_24, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_25", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_25, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_26", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_26, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_27", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_27, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_28", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_28, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_29", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_29, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_30", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_30, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_31", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_31, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_32", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_32, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_33", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_33, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_34", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_34, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_35", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_35, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_36", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_36, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_37", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_37, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_38", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_38, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_39", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_39, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_40", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_40, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_41", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_41, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_42", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_42, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_43", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_43, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_44", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_44, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_45", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_45, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_46", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_46, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_47", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_47, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_48", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_48, SATFE_RegisterType_eISB},
   {"AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_49", BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_49, SATFE_RegisterType_eISB},
   {"BAC_MSPI_CONTROL", BCHP_BAC_MSPI_CONTROL, SATFE_RegisterType_eISB},
   {"BAC_MSPI_START", BCHP_BAC_MSPI_START, SATFE_RegisterType_eISB},
   {"BAC_MSPI_SLAVE_ADDR", BCHP_BAC_MSPI_SLAVE_ADDR, SATFE_RegisterType_eISB},
   {"BAC_MSPI_WR_DATA", BCHP_BAC_MSPI_WR_DATA, SATFE_RegisterType_eISB},
   {"BAC_MSPI_SFT_REG0", BCHP_BAC_MSPI_SFT_REG0, SATFE_RegisterType_eISB},
   {"BSCA_CHIP_ADDRESS", BCHP_BSCA_CHIP_ADDRESS, SATFE_RegisterType_eISB},
   {"BSCA_DATA_IN0", BCHP_BSCA_DATA_IN0, SATFE_RegisterType_eISB},
   {"BSCA_DATA_IN1", BCHP_BSCA_DATA_IN1, SATFE_RegisterType_eISB},
   {"BSCA_DATA_IN2", BCHP_BSCA_DATA_IN2, SATFE_RegisterType_eISB},
   {"BSCA_DATA_IN3", BCHP_BSCA_DATA_IN3, SATFE_RegisterType_eISB},
   {"BSCA_DATA_IN4", BCHP_BSCA_DATA_IN4, SATFE_RegisterType_eISB},
   {"BSCA_DATA_IN5", BCHP_BSCA_DATA_IN5, SATFE_RegisterType_eISB},
   {"BSCA_DATA_IN6", BCHP_BSCA_DATA_IN6, SATFE_RegisterType_eISB},
   {"BSCA_DATA_IN7", BCHP_BSCA_DATA_IN7, SATFE_RegisterType_eISB},
   {"BSCA_CNT_REG", BCHP_BSCA_CNT_REG, SATFE_RegisterType_eISB},
   {"BSCA_CTL_REG", BCHP_BSCA_CTL_REG, SATFE_RegisterType_eISB},
   {"BSCA_IIC_ENABLE", BCHP_BSCA_IIC_ENABLE, SATFE_RegisterType_eISB},
   {"BSCA_DATA_OUT0", BCHP_BSCA_DATA_OUT0, SATFE_RegisterType_eISB},
   {"BSCA_DATA_OUT1", BCHP_BSCA_DATA_OUT1, SATFE_RegisterType_eISB},
   {"BSCA_DATA_OUT2", BCHP_BSCA_DATA_OUT2, SATFE_RegisterType_eISB},
   {"BSCA_DATA_OUT3", BCHP_BSCA_DATA_OUT3, SATFE_RegisterType_eISB},
   {"BSCA_DATA_OUT4", BCHP_BSCA_DATA_OUT4, SATFE_RegisterType_eISB},
   {"BSCA_DATA_OUT5", BCHP_BSCA_DATA_OUT5, SATFE_RegisterType_eISB},
   {"BSCA_DATA_OUT6", BCHP_BSCA_DATA_OUT6, SATFE_RegisterType_eISB},
   {"BSCA_DATA_OUT7", BCHP_BSCA_DATA_OUT7, SATFE_RegisterType_eISB},
   {"BSCA_CTLHI_REG", BCHP_BSCA_CTLHI_REG, SATFE_RegisterType_eISB},
   {"BSCA_SCL_PARAM", BCHP_BSCA_SCL_PARAM, SATFE_RegisterType_eISB},
   {"BSCB_CHIP_ADDRESS", BCHP_BSCB_CHIP_ADDRESS, SATFE_RegisterType_eISB},
   {"BSCB_DATA_IN0", BCHP_BSCB_DATA_IN0, SATFE_RegisterType_eISB},
   {"BSCB_DATA_IN1", BCHP_BSCB_DATA_IN1, SATFE_RegisterType_eISB},
   {"BSCB_DATA_IN2", BCHP_BSCB_DATA_IN2, SATFE_RegisterType_eISB},
   {"BSCB_DATA_IN3", BCHP_BSCB_DATA_IN3, SATFE_RegisterType_eISB},
   {"BSCB_DATA_IN4", BCHP_BSCB_DATA_IN4, SATFE_RegisterType_eISB},
   {"BSCB_DATA_IN5", BCHP_BSCB_DATA_IN5, SATFE_RegisterType_eISB},
   {"BSCB_DATA_IN6", BCHP_BSCB_DATA_IN6, SATFE_RegisterType_eISB},
   {"BSCB_DATA_IN7", BCHP_BSCB_DATA_IN7, SATFE_RegisterType_eISB},
   {"BSCB_CNT_REG", BCHP_BSCB_CNT_REG, SATFE_RegisterType_eISB},
   {"BSCB_CTL_REG", BCHP_BSCB_CTL_REG, SATFE_RegisterType_eISB},
   {"BSCB_IIC_ENABLE", BCHP_BSCB_IIC_ENABLE, SATFE_RegisterType_eISB},
   {"BSCB_DATA_OUT0", BCHP_BSCB_DATA_OUT0, SATFE_RegisterType_eISB},
   {"BSCB_DATA_OUT1", BCHP_BSCB_DATA_OUT1, SATFE_RegisterType_eISB},
   {"BSCB_DATA_OUT2", BCHP_BSCB_DATA_OUT2, SATFE_RegisterType_eISB},
   {"BSCB_DATA_OUT3", BCHP_BSCB_DATA_OUT3, SATFE_RegisterType_eISB},
   {"BSCB_DATA_OUT4", BCHP_BSCB_DATA_OUT4, SATFE_RegisterType_eISB},
   {"BSCB_DATA_OUT5", BCHP_BSCB_DATA_OUT5, SATFE_RegisterType_eISB},
   {"BSCB_DATA_OUT6", BCHP_BSCB_DATA_OUT6, SATFE_RegisterType_eISB},
   {"BSCB_DATA_OUT7", BCHP_BSCB_DATA_OUT7, SATFE_RegisterType_eISB},
   {"BSCB_CTLHI_REG", BCHP_BSCB_CTLHI_REG, SATFE_RegisterType_eISB},
   {"BSCB_SCL_PARAM", BCHP_BSCB_SCL_PARAM, SATFE_RegisterType_eISB},
   {"BSPI_REVISION_ID", BCHP_BSPI_REVISION_ID, SATFE_RegisterType_eISB},
   {"BSPI_SCRATCH", BCHP_BSPI_SCRATCH, SATFE_RegisterType_eISB},
   {"BSPI_MAST_N_BOOT_CTRL", BCHP_BSPI_MAST_N_BOOT_CTRL, SATFE_RegisterType_eISB},
   {"BSPI_BUSY_STATUS", BCHP_BSPI_BUSY_STATUS, SATFE_RegisterType_eISB},
   {"BSPI_INTR_STATUS", BCHP_BSPI_INTR_STATUS, SATFE_RegisterType_eISB},
   {"BSPI_B0_STATUS", BCHP_BSPI_B0_STATUS, SATFE_RegisterType_eISB},
   {"BSPI_B0_CTRL", BCHP_BSPI_B0_CTRL, SATFE_RegisterType_eISB},
   {"BSPI_B1_STATUS", BCHP_BSPI_B1_STATUS, SATFE_RegisterType_eISB},
   {"BSPI_B1_CTRL", BCHP_BSPI_B1_CTRL, SATFE_RegisterType_eISB},
   {"BSPI_STRAP_OVERRIDE_CTRL", BCHP_BSPI_STRAP_OVERRIDE_CTRL, SATFE_RegisterType_eISB},
   {"BSPI_FLEX_MODE_ENABLE", BCHP_BSPI_FLEX_MODE_ENABLE, SATFE_RegisterType_eISB},
   {"BSPI_BITS_PER_CYCLE", BCHP_BSPI_BITS_PER_CYCLE, SATFE_RegisterType_eISB},
   {"BSPI_BITS_PER_PHASE", BCHP_BSPI_BITS_PER_PHASE, SATFE_RegisterType_eISB},
   {"BSPI_CMD_AND_MODE_BYTE", BCHP_BSPI_CMD_AND_MODE_BYTE, SATFE_RegisterType_eISB},
   {"BSPI_BSPI_FLASH_UPPER_ADDR_BYTE", BCHP_BSPI_BSPI_FLASH_UPPER_ADDR_BYTE, SATFE_RegisterType_eISB},
   {"BSPI_BSPI_XOR_VALUE", BCHP_BSPI_BSPI_XOR_VALUE, SATFE_RegisterType_eISB},
   {"BSPI_BSPI_XOR_ENABLE", BCHP_BSPI_BSPI_XOR_ENABLE, SATFE_RegisterType_eISB},
   {"BSPI_BSPI_PIO_MODE_ENABLE", BCHP_BSPI_BSPI_PIO_MODE_ENABLE, SATFE_RegisterType_eISB},
   {"BSPI_BSPI_PIO_IODIR", BCHP_BSPI_BSPI_PIO_IODIR, SATFE_RegisterType_eISB},
   {"BSPI_BSPI_PIO_DATA", BCHP_BSPI_BSPI_PIO_DATA, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_FE_CTRL", BCHP_DEMOD_XPT_FE_FE_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_PWR_CTRL", BCHP_DEMOD_XPT_FE_PWR_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MAX_PID_CHANNEL", BCHP_DEMOD_XPT_FE_MAX_PID_CHANNEL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB_SYNC_DETECT_CTRL", BCHP_DEMOD_XPT_FE_IB_SYNC_DETECT_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_INTR_STATUS0_REG", BCHP_DEMOD_XPT_FE_INTR_STATUS0_REG, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_INTR_STATUS1_REG", BCHP_DEMOD_XPT_FE_INTR_STATUS1_REG, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_INTR_STATUS2_REG", BCHP_DEMOD_XPT_FE_INTR_STATUS2_REG, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_INTR_STATUS0_REG_EN", BCHP_DEMOD_XPT_FE_INTR_STATUS0_REG_EN, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_INTR_STATUS1_REG_EN", BCHP_DEMOD_XPT_FE_INTR_STATUS1_REG_EN, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_INTR_STATUS2_REG_EN", BCHP_DEMOD_XPT_FE_INTR_STATUS2_REG_EN, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF_INTR_STATUS0_REG", BCHP_DEMOD_XPT_FE_TSMF_INTR_STATUS0_REG, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF_INTR_STATUS0_REG_EN", BCHP_DEMOD_XPT_FE_TSMF_INTR_STATUS0_REG_EN, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_RX_INTR_STATUS0_REG", BCHP_DEMOD_XPT_FE_MTSIF_RX_INTR_STATUS0_REG, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_RX_INTR_STATUS0_REG_EN", BCHP_DEMOD_XPT_FE_MTSIF_RX_INTR_STATUS0_REG_EN, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_PARSERS_TSMF_FRAME_ERROR_INTR_STATUS0_REG", BCHP_DEMOD_XPT_FE_PARSERS_TSMF_FRAME_ERROR_INTR_STATUS0_REG, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_PARSERS_TSMF_SYNC_ERROR_INTR_STATUS0_REG", BCHP_DEMOD_XPT_FE_PARSERS_TSMF_SYNC_ERROR_INTR_STATUS0_REG, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_PARSERS_INBUFF_OVFL_ERROR_INTR_STATUS0_REG", BCHP_DEMOD_XPT_FE_PARSERS_INBUFF_OVFL_ERROR_INTR_STATUS0_REG, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_PARSERS_TSMF_FRAME_ERROR_INTR_STATUS0_REG_EN", BCHP_DEMOD_XPT_FE_PARSERS_TSMF_FRAME_ERROR_INTR_STATUS0_REG_EN, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_PARSERS_TSMF_SYNC_ERROR_INTR_STATUS0_REG_EN", BCHP_DEMOD_XPT_FE_PARSERS_TSMF_SYNC_ERROR_INTR_STATUS0_REG_EN, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_PARSERS_INBUFF_OVFL_ERROR_INTR_STATUS0_REG_EN", BCHP_DEMOD_XPT_FE_PARSERS_INBUFF_OVFL_ERROR_INTR_STATUS0_REG_EN, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_PARSER_BAND0_BAND31_SRC", BCHP_DEMOD_XPT_FE_PARSER_BAND0_BAND31_SRC, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_PARSER_BAND0_BAND15_MTSIF_RX_SRC", BCHP_DEMOD_XPT_FE_PARSER_BAND0_BAND15_MTSIF_RX_SRC, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_PARSER_BAND16_BAND31_MTSIF_RX_SRC", BCHP_DEMOD_XPT_FE_PARSER_BAND16_BAND31_MTSIF_RX_SRC, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_ATS_COUNTER_CTRL", BCHP_DEMOD_XPT_FE_ATS_COUNTER_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_ATS_TS_MOD300", BCHP_DEMOD_XPT_FE_ATS_TS_MOD300, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_ATS_TS_BINARY", BCHP_DEMOD_XPT_FE_ATS_TS_BINARY, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TV_STATUS_0", BCHP_DEMOD_XPT_FE_TV_STATUS_0, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TV_STATUS_1", BCHP_DEMOD_XPT_FE_TV_STATUS_1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TV_STATUS_2", BCHP_DEMOD_XPT_FE_TV_STATUS_2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TV_STATUS_3", BCHP_DEMOD_XPT_FE_TV_STATUS_3, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TV_STATUS_4", BCHP_DEMOD_XPT_FE_TV_STATUS_4, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX_TV_CTRL", BCHP_DEMOD_XPT_FE_MTSIF_TX_TV_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB0_CTRL", BCHP_DEMOD_XPT_FE_IB0_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB0_SYNC_COUNT", BCHP_DEMOD_XPT_FE_IB0_SYNC_COUNT, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB1_CTRL", BCHP_DEMOD_XPT_FE_IB1_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB1_SYNC_COUNT", BCHP_DEMOD_XPT_FE_IB1_SYNC_COUNT, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB2_CTRL", BCHP_DEMOD_XPT_FE_IB2_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB2_SYNC_COUNT", BCHP_DEMOD_XPT_FE_IB2_SYNC_COUNT, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB3_CTRL", BCHP_DEMOD_XPT_FE_IB3_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB3_SYNC_COUNT", BCHP_DEMOD_XPT_FE_IB3_SYNC_COUNT, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB4_CTRL", BCHP_DEMOD_XPT_FE_IB4_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB4_SYNC_COUNT", BCHP_DEMOD_XPT_FE_IB4_SYNC_COUNT, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB5_CTRL", BCHP_DEMOD_XPT_FE_IB5_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB5_SYNC_COUNT", BCHP_DEMOD_XPT_FE_IB5_SYNC_COUNT, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB6_CTRL", BCHP_DEMOD_XPT_FE_IB6_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB6_SYNC_COUNT", BCHP_DEMOD_XPT_FE_IB6_SYNC_COUNT, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB7_CTRL", BCHP_DEMOD_XPT_FE_IB7_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB7_SYNC_COUNT", BCHP_DEMOD_XPT_FE_IB7_SYNC_COUNT, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB8_CTRL", BCHP_DEMOD_XPT_FE_IB8_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB8_SYNC_COUNT", BCHP_DEMOD_XPT_FE_IB8_SYNC_COUNT, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB9_CTRL", BCHP_DEMOD_XPT_FE_IB9_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB9_SYNC_COUNT", BCHP_DEMOD_XPT_FE_IB9_SYNC_COUNT, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB10_CTRL", BCHP_DEMOD_XPT_FE_IB10_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB10_SYNC_COUNT", BCHP_DEMOD_XPT_FE_IB10_SYNC_COUNT, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB11_CTRL", BCHP_DEMOD_XPT_FE_IB11_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB11_SYNC_COUNT", BCHP_DEMOD_XPT_FE_IB11_SYNC_COUNT, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB12_CTRL", BCHP_DEMOD_XPT_FE_IB12_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB12_SYNC_COUNT", BCHP_DEMOD_XPT_FE_IB12_SYNC_COUNT, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB13_CTRL", BCHP_DEMOD_XPT_FE_IB13_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB13_SYNC_COUNT", BCHP_DEMOD_XPT_FE_IB13_SYNC_COUNT, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB14_CTRL", BCHP_DEMOD_XPT_FE_IB14_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB14_SYNC_COUNT", BCHP_DEMOD_XPT_FE_IB14_SYNC_COUNT, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB15_CTRL", BCHP_DEMOD_XPT_FE_IB15_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_IB15_SYNC_COUNT", BCHP_DEMOD_XPT_FE_IB15_SYNC_COUNT, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER1_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER1_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER1_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER1_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER2_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER2_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER2_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER2_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER3_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER3_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER3_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER3_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER4_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER4_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER4_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER4_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER5_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER5_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER5_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER5_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER6_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER6_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER6_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER6_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER7_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER7_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER7_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER7_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER8_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER8_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER8_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER8_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER9_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER9_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER9_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER9_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER10_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER10_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER10_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER10_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER11_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER11_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER11_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER11_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER12_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER12_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER12_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER12_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER13_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER13_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER13_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER13_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER14_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER14_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER14_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER14_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER15_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER15_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER15_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER15_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER16_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER16_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER16_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER16_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER17_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER17_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER17_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER17_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER18_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER18_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER18_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER18_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER19_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER19_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER19_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER19_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER20_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER20_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER20_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER20_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER21_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER21_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER21_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER21_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER22_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER22_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER22_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER22_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER23_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER23_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER23_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER23_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER24_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER24_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER24_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER24_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER25_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER25_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER25_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER25_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER26_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER26_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER26_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER26_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER27_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER27_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER27_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER27_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER28_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER28_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER28_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER28_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER29_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER29_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER29_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER29_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER30_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER30_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER30_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER30_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER31_CTRL1", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER31_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER31_CTRL2", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER31_CTRL2, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID", BCHP_DEMOD_XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF0_CTRL", BCHP_DEMOD_XPT_FE_TSMF0_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF0_SLOT_MAP_LO", BCHP_DEMOD_XPT_FE_TSMF0_SLOT_MAP_LO, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF0_SLOT_MAP_HI", BCHP_DEMOD_XPT_FE_TSMF0_SLOT_MAP_HI, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF0_STATUS", BCHP_DEMOD_XPT_FE_TSMF0_STATUS, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF1_CTRL", BCHP_DEMOD_XPT_FE_TSMF1_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF1_SLOT_MAP_LO", BCHP_DEMOD_XPT_FE_TSMF1_SLOT_MAP_LO, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF1_SLOT_MAP_HI", BCHP_DEMOD_XPT_FE_TSMF1_SLOT_MAP_HI, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF1_STATUS", BCHP_DEMOD_XPT_FE_TSMF1_STATUS, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF2_CTRL", BCHP_DEMOD_XPT_FE_TSMF2_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF2_SLOT_MAP_LO", BCHP_DEMOD_XPT_FE_TSMF2_SLOT_MAP_LO, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF2_SLOT_MAP_HI", BCHP_DEMOD_XPT_FE_TSMF2_SLOT_MAP_HI, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF2_STATUS", BCHP_DEMOD_XPT_FE_TSMF2_STATUS, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF3_CTRL", BCHP_DEMOD_XPT_FE_TSMF3_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF3_SLOT_MAP_LO", BCHP_DEMOD_XPT_FE_TSMF3_SLOT_MAP_LO, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF3_SLOT_MAP_HI", BCHP_DEMOD_XPT_FE_TSMF3_SLOT_MAP_HI, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF3_STATUS", BCHP_DEMOD_XPT_FE_TSMF3_STATUS, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF4_CTRL", BCHP_DEMOD_XPT_FE_TSMF4_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF4_SLOT_MAP_LO", BCHP_DEMOD_XPT_FE_TSMF4_SLOT_MAP_LO, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF4_SLOT_MAP_HI", BCHP_DEMOD_XPT_FE_TSMF4_SLOT_MAP_HI, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF4_STATUS", BCHP_DEMOD_XPT_FE_TSMF4_STATUS, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF5_CTRL", BCHP_DEMOD_XPT_FE_TSMF5_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF5_SLOT_MAP_LO", BCHP_DEMOD_XPT_FE_TSMF5_SLOT_MAP_LO, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF5_SLOT_MAP_HI", BCHP_DEMOD_XPT_FE_TSMF5_SLOT_MAP_HI, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF5_STATUS", BCHP_DEMOD_XPT_FE_TSMF5_STATUS, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF6_CTRL", BCHP_DEMOD_XPT_FE_TSMF6_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF6_SLOT_MAP_LO", BCHP_DEMOD_XPT_FE_TSMF6_SLOT_MAP_LO, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF6_SLOT_MAP_HI", BCHP_DEMOD_XPT_FE_TSMF6_SLOT_MAP_HI, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF6_STATUS", BCHP_DEMOD_XPT_FE_TSMF6_STATUS, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF7_CTRL", BCHP_DEMOD_XPT_FE_TSMF7_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF7_SLOT_MAP_LO", BCHP_DEMOD_XPT_FE_TSMF7_SLOT_MAP_LO, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF7_SLOT_MAP_HI", BCHP_DEMOD_XPT_FE_TSMF7_SLOT_MAP_HI, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF7_STATUS", BCHP_DEMOD_XPT_FE_TSMF7_STATUS, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF8_CTRL", BCHP_DEMOD_XPT_FE_TSMF8_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF8_SLOT_MAP_LO", BCHP_DEMOD_XPT_FE_TSMF8_SLOT_MAP_LO, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF8_SLOT_MAP_HI", BCHP_DEMOD_XPT_FE_TSMF8_SLOT_MAP_HI, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF8_STATUS", BCHP_DEMOD_XPT_FE_TSMF8_STATUS, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF9_CTRL", BCHP_DEMOD_XPT_FE_TSMF9_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF9_SLOT_MAP_LO", BCHP_DEMOD_XPT_FE_TSMF9_SLOT_MAP_LO, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF9_SLOT_MAP_HI", BCHP_DEMOD_XPT_FE_TSMF9_SLOT_MAP_HI, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_TSMF9_STATUS", BCHP_DEMOD_XPT_FE_TSMF9_STATUS, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_RX0_CTRL1", BCHP_DEMOD_XPT_FE_MTSIF_RX0_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_RX0_SECRET_WORD", BCHP_DEMOD_XPT_FE_MTSIF_RX0_SECRET_WORD, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_RX0_BAND0_BAND31_ID_DROP", BCHP_DEMOD_XPT_FE_MTSIF_RX0_BAND0_BAND31_ID_DROP, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_RX0_BAND0_BAND3_ID", BCHP_DEMOD_XPT_FE_MTSIF_RX0_BAND0_BAND3_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_RX0_BAND4_BAND7_ID", BCHP_DEMOD_XPT_FE_MTSIF_RX0_BAND4_BAND7_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_RX0_BAND8_BAND11_ID", BCHP_DEMOD_XPT_FE_MTSIF_RX0_BAND8_BAND11_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_RX0_BAND12_BAND15_ID", BCHP_DEMOD_XPT_FE_MTSIF_RX0_BAND12_BAND15_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_RX0_BAND16_BAND19_ID", BCHP_DEMOD_XPT_FE_MTSIF_RX0_BAND16_BAND19_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_RX0_BAND20_BAND23_ID", BCHP_DEMOD_XPT_FE_MTSIF_RX0_BAND20_BAND23_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_RX0_BAND24_BAND27_ID", BCHP_DEMOD_XPT_FE_MTSIF_RX0_BAND24_BAND27_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_RX0_BAND28_BAND31_ID", BCHP_DEMOD_XPT_FE_MTSIF_RX0_BAND28_BAND31_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_RX0_BAND32_BAND35_ID", BCHP_DEMOD_XPT_FE_MTSIF_RX0_BAND32_BAND35_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_RX0_BAND36_BAND39_ID", BCHP_DEMOD_XPT_FE_MTSIF_RX0_BAND36_BAND39_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_RX0_BAND40_BAND43_ID", BCHP_DEMOD_XPT_FE_MTSIF_RX0_BAND40_BAND43_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_RX0_BAND44_BAND47_ID", BCHP_DEMOD_XPT_FE_MTSIF_RX0_BAND44_BAND47_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_RX0_BAND48_BAND51_ID", BCHP_DEMOD_XPT_FE_MTSIF_RX0_BAND48_BAND51_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_RX0_BAND52_BAND55_ID", BCHP_DEMOD_XPT_FE_MTSIF_RX0_BAND52_BAND55_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_RX0_BAND56_BAND59_ID", BCHP_DEMOD_XPT_FE_MTSIF_RX0_BAND56_BAND59_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_RX0_BAND60_BAND63_ID", BCHP_DEMOD_XPT_FE_MTSIF_RX0_BAND60_BAND63_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_RX0_PKT_BAND0_BAND31_DETECT", BCHP_DEMOD_XPT_FE_MTSIF_RX0_PKT_BAND0_BAND31_DETECT, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX0_CTRL1", BCHP_DEMOD_XPT_FE_MTSIF_TX0_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX0_BLOCK_OUT", BCHP_DEMOD_XPT_FE_MTSIF_TX0_BLOCK_OUT, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX0_STATUS", BCHP_DEMOD_XPT_FE_MTSIF_TX0_STATUS, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX0_SECRET_WORD", BCHP_DEMOD_XPT_FE_MTSIF_TX0_SECRET_WORD, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX0_BAND0_BAND3_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX0_BAND0_BAND3_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX0_BAND4_BAND7_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX0_BAND4_BAND7_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX0_BAND8_BAND11_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX0_BAND8_BAND11_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX0_BAND12_BAND15_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX0_BAND12_BAND15_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX0_BAND16_BAND19_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX0_BAND16_BAND19_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX0_BAND20_BAND23_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX0_BAND20_BAND23_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX0_BAND24_BAND27_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX0_BAND24_BAND27_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX0_BAND28_BAND31_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX0_BAND28_BAND31_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX0_BAND32_BAND35_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX0_BAND32_BAND35_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX0_BAND36_BAND39_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX0_BAND36_BAND39_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX0_BAND40_BAND43_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX0_BAND40_BAND43_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX0_BAND44_BAND47_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX0_BAND44_BAND47_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX0_BAND48_BAND51_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX0_BAND48_BAND51_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX0_BAND52_BAND55_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX0_BAND52_BAND55_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX0_BAND56_BAND59_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX0_BAND56_BAND59_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX0_BAND60_BAND63_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX0_BAND60_BAND63_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX1_CTRL1", BCHP_DEMOD_XPT_FE_MTSIF_TX1_CTRL1, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX1_BLOCK_OUT", BCHP_DEMOD_XPT_FE_MTSIF_TX1_BLOCK_OUT, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX1_STATUS", BCHP_DEMOD_XPT_FE_MTSIF_TX1_STATUS, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX1_SECRET_WORD", BCHP_DEMOD_XPT_FE_MTSIF_TX1_SECRET_WORD, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX1_BAND0_BAND3_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX1_BAND0_BAND3_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX1_BAND4_BAND7_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX1_BAND4_BAND7_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX1_BAND8_BAND11_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX1_BAND8_BAND11_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX1_BAND12_BAND15_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX1_BAND12_BAND15_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX1_BAND16_BAND19_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX1_BAND16_BAND19_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX1_BAND20_BAND23_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX1_BAND20_BAND23_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX1_BAND24_BAND27_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX1_BAND24_BAND27_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX1_BAND28_BAND31_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX1_BAND28_BAND31_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX1_BAND32_BAND35_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX1_BAND32_BAND35_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX1_BAND36_BAND39_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX1_BAND36_BAND39_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX1_BAND40_BAND43_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX1_BAND40_BAND43_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX1_BAND44_BAND47_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX1_BAND44_BAND47_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX1_BAND48_BAND51_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX1_BAND48_BAND51_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX1_BAND52_BAND55_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX1_BAND52_BAND55_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX1_BAND56_BAND59_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX1_BAND56_BAND59_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_TX1_BAND60_BAND63_ID", BCHP_DEMOD_XPT_FE_MTSIF_TX1_BAND60_BAND63_ID, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_LEGACY_CTRL", BCHP_DEMOD_XPT_FE_MTSIF_LEGACY_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_FE_MTSIF_LEGACY_STATUS", BCHP_DEMOD_XPT_FE_MTSIF_LEGACY_STATUS, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_MTSIF_TX0_IO_CLK_SYNC_DLY_SEL", BCHP_DEMOD_XPT_MTSIF_TX0_IO_CLK_SYNC_DLY_SEL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_MTSIF_TX0_IO_DATA_DLY_SEL", BCHP_DEMOD_XPT_MTSIF_TX0_IO_DATA_DLY_SEL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_MTSIF_TX1_IO_CLK_SYNC_DLY_SEL", BCHP_DEMOD_XPT_MTSIF_TX1_IO_CLK_SYNC_DLY_SEL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_MTSIF_TX1_IO_DATA_DLY_SEL", BCHP_DEMOD_XPT_MTSIF_TX1_IO_DATA_DLY_SEL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_WAKEUP_CTRL", BCHP_DEMOD_XPT_WAKEUP_CTRL, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_WAKEUP_STATUS", BCHP_DEMOD_XPT_WAKEUP_STATUS, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_WAKEUP_INTR_STATUS_REG", BCHP_DEMOD_XPT_WAKEUP_INTR_STATUS_REG, SATFE_RegisterType_eISB},
   {"DEMOD_XPT_WAKEUP_INTR_STATUS_REG_EN", BCHP_DEMOD_XPT_WAKEUP_INTR_STATUS_REG_EN, SATFE_RegisterType_eISB},
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
   {"FTM_PHY_ODU_CTL", BCHP_FTM_PHY_ODU_CTL, SATFE_RegisterType_eISB},
   {"FTM_PHY_ODU_BYTE", BCHP_FTM_PHY_ODU_BYTE, SATFE_RegisterType_eISB},
   {"FTM_PHY_ANA_MISC", BCHP_FTM_PHY_ANA_MISC, SATFE_RegisterType_eISB},
   {"FTM_PHY_ANA_CTL0_0", BCHP_FTM_PHY_ANA_CTL0_0, SATFE_RegisterType_eISB},
   {"FTM_PHY_ANA_CTL0_1", BCHP_FTM_PHY_ANA_CTL0_1, SATFE_RegisterType_eISB},
   {"FTM_PHY_ANA_CTL1_0", BCHP_FTM_PHY_ANA_CTL1_0, SATFE_RegisterType_eISB},
   {"FTM_PHY_ANA_CTL1_1", BCHP_FTM_PHY_ANA_CTL1_1, SATFE_RegisterType_eISB},
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
   {"FTM_SW_SPARE_0", BCHP_FTM_SW_SPARE_0, SATFE_RegisterType_eISB},
   {"FTM_SW_SPARE_1", BCHP_FTM_SW_SPARE_1, SATFE_RegisterType_eISB},
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
   {"GIO_RSVDO", BCHP_GIO_RSVDO, SATFE_RegisterType_eISB},
   {"GIO_RSVD1", BCHP_GIO_RSVD1, SATFE_RegisterType_eISB},
   {"GIO_RSVD2", BCHP_GIO_RSVD2, SATFE_RegisterType_eISB},
   {"GIO_EC_LO", BCHP_GIO_EC_LO, SATFE_RegisterType_eISB},
   {"GIO_EI_LO", BCHP_GIO_EI_LO, SATFE_RegisterType_eISB},
   {"GIO_MASK_LO", BCHP_GIO_MASK_LO, SATFE_RegisterType_eISB},
   {"GIO_LEVEL_LO", BCHP_GIO_LEVEL_LO, SATFE_RegisterType_eISB},
   {"GIO_STAT_LO", BCHP_GIO_STAT_LO, SATFE_RegisterType_eISB},
   {"MSPI_SPCR0_LSB", BCHP_MSPI_SPCR0_LSB, SATFE_RegisterType_eISB},
   {"MSPI_SPCR0_MSB", BCHP_MSPI_SPCR0_MSB, SATFE_RegisterType_eISB},
   {"MSPI_SPCR1_LSB", BCHP_MSPI_SPCR1_LSB, SATFE_RegisterType_eISB},
   {"MSPI_SPCR1_MSB", BCHP_MSPI_SPCR1_MSB, SATFE_RegisterType_eISB},
   {"MSPI_NEWQP", BCHP_MSPI_NEWQP, SATFE_RegisterType_eISB},
   {"MSPI_ENDQP", BCHP_MSPI_ENDQP, SATFE_RegisterType_eISB},
   {"MSPI_SPCR2", BCHP_MSPI_SPCR2, SATFE_RegisterType_eISB},
   {"MSPI_MSPI_STATUS", BCHP_MSPI_MSPI_STATUS, SATFE_RegisterType_eISB},
   {"MSPI_CPTQP", BCHP_MSPI_CPTQP, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM00", BCHP_MSPI_TXRAM00, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM01", BCHP_MSPI_TXRAM01, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM02", BCHP_MSPI_TXRAM02, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM03", BCHP_MSPI_TXRAM03, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM04", BCHP_MSPI_TXRAM04, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM05", BCHP_MSPI_TXRAM05, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM06", BCHP_MSPI_TXRAM06, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM07", BCHP_MSPI_TXRAM07, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM08", BCHP_MSPI_TXRAM08, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM09", BCHP_MSPI_TXRAM09, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM10", BCHP_MSPI_TXRAM10, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM11", BCHP_MSPI_TXRAM11, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM12", BCHP_MSPI_TXRAM12, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM13", BCHP_MSPI_TXRAM13, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM14", BCHP_MSPI_TXRAM14, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM15", BCHP_MSPI_TXRAM15, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM16", BCHP_MSPI_TXRAM16, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM17", BCHP_MSPI_TXRAM17, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM18", BCHP_MSPI_TXRAM18, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM19", BCHP_MSPI_TXRAM19, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM20", BCHP_MSPI_TXRAM20, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM21", BCHP_MSPI_TXRAM21, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM22", BCHP_MSPI_TXRAM22, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM23", BCHP_MSPI_TXRAM23, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM24", BCHP_MSPI_TXRAM24, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM25", BCHP_MSPI_TXRAM25, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM26", BCHP_MSPI_TXRAM26, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM27", BCHP_MSPI_TXRAM27, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM28", BCHP_MSPI_TXRAM28, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM29", BCHP_MSPI_TXRAM29, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM30", BCHP_MSPI_TXRAM30, SATFE_RegisterType_eISB},
   {"MSPI_TXRAM31", BCHP_MSPI_TXRAM31, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM00", BCHP_MSPI_RXRAM00, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM01", BCHP_MSPI_RXRAM01, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM02", BCHP_MSPI_RXRAM02, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM03", BCHP_MSPI_RXRAM03, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM04", BCHP_MSPI_RXRAM04, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM05", BCHP_MSPI_RXRAM05, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM06", BCHP_MSPI_RXRAM06, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM07", BCHP_MSPI_RXRAM07, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM08", BCHP_MSPI_RXRAM08, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM09", BCHP_MSPI_RXRAM09, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM10", BCHP_MSPI_RXRAM10, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM11", BCHP_MSPI_RXRAM11, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM12", BCHP_MSPI_RXRAM12, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM13", BCHP_MSPI_RXRAM13, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM14", BCHP_MSPI_RXRAM14, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM15", BCHP_MSPI_RXRAM15, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM16", BCHP_MSPI_RXRAM16, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM17", BCHP_MSPI_RXRAM17, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM18", BCHP_MSPI_RXRAM18, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM19", BCHP_MSPI_RXRAM19, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM20", BCHP_MSPI_RXRAM20, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM21", BCHP_MSPI_RXRAM21, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM22", BCHP_MSPI_RXRAM22, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM23", BCHP_MSPI_RXRAM23, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM24", BCHP_MSPI_RXRAM24, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM25", BCHP_MSPI_RXRAM25, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM26", BCHP_MSPI_RXRAM26, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM27", BCHP_MSPI_RXRAM27, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM28", BCHP_MSPI_RXRAM28, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM29", BCHP_MSPI_RXRAM29, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM30", BCHP_MSPI_RXRAM30, SATFE_RegisterType_eISB},
   {"MSPI_RXRAM31", BCHP_MSPI_RXRAM31, SATFE_RegisterType_eISB},
   {"MSPI_CDRAM00", BCHP_MSPI_CDRAM00, SATFE_RegisterType_eISB},
   {"MSPI_CDRAM01", BCHP_MSPI_CDRAM01, SATFE_RegisterType_eISB},
   {"MSPI_CDRAM02", BCHP_MSPI_CDRAM02, SATFE_RegisterType_eISB},
   {"MSPI_CDRAM03", BCHP_MSPI_CDRAM03, SATFE_RegisterType_eISB},
   {"MSPI_CDRAM04", BCHP_MSPI_CDRAM04, SATFE_RegisterType_eISB},
   {"MSPI_CDRAM05", BCHP_MSPI_CDRAM05, SATFE_RegisterType_eISB},
   {"MSPI_CDRAM06", BCHP_MSPI_CDRAM06, SATFE_RegisterType_eISB},
   {"MSPI_CDRAM07", BCHP_MSPI_CDRAM07, SATFE_RegisterType_eISB},
   {"MSPI_CDRAM08", BCHP_MSPI_CDRAM08, SATFE_RegisterType_eISB},
   {"MSPI_CDRAM09", BCHP_MSPI_CDRAM09, SATFE_RegisterType_eISB},
   {"MSPI_CDRAM10", BCHP_MSPI_CDRAM10, SATFE_RegisterType_eISB},
   {"MSPI_CDRAM11", BCHP_MSPI_CDRAM11, SATFE_RegisterType_eISB},
   {"MSPI_CDRAM12", BCHP_MSPI_CDRAM12, SATFE_RegisterType_eISB},
   {"MSPI_CDRAM13", BCHP_MSPI_CDRAM13, SATFE_RegisterType_eISB},
   {"MSPI_CDRAM14", BCHP_MSPI_CDRAM14, SATFE_RegisterType_eISB},
   {"MSPI_CDRAM15", BCHP_MSPI_CDRAM15, SATFE_RegisterType_eISB},
   {"MSPI_WRITE_LOCK", BCHP_MSPI_WRITE_LOCK, SATFE_RegisterType_eISB},
   {"MSPI_DISABLE_FLUSH_GEN", BCHP_MSPI_DISABLE_FLUSH_GEN, SATFE_RegisterType_eISB},
   {"PER_IRQ_STATUS", BCHP_PER_IRQ_STATUS, SATFE_RegisterType_eISB},
   {"PER_IRQ_SET", BCHP_PER_IRQ_SET, SATFE_RegisterType_eISB},
   {"PER_IRQ_CLR", BCHP_PER_IRQ_CLR, SATFE_RegisterType_eISB},
   {"PER_IRQ_MASK_STATUS", BCHP_PER_IRQ_MASK_STATUS, SATFE_RegisterType_eISB},
   {"PER_IRQ_MASK_SET", BCHP_PER_IRQ_MASK_SET, SATFE_RegisterType_eISB},
   {"PER_IRQ_MASK_CLR", BCHP_PER_IRQ_MASK_CLR, SATFE_RegisterType_eISB},
   {"SDS_BERT_BERCTL", BCHP_SDS_BERT_0_BERCTL, SATFE_RegisterType_eISB},
   {"SDS_BERT_BEIT", BCHP_SDS_BERT_0_BEIT, SATFE_RegisterType_eISB},
   {"SDS_BERT_BERC", BCHP_SDS_BERT_0_BERC, SATFE_RegisterType_eISB},
   {"SDS_BERT_BEM1", BCHP_SDS_BERT_0_BEM1, SATFE_RegisterType_eISB},
   {"SDS_BERT_BEM2", BCHP_SDS_BERT_0_BEM2, SATFE_RegisterType_eISB},
   {"SDS_BERT_BEM3", BCHP_SDS_BERT_0_BEM3, SATFE_RegisterType_eISB},
   {"SDS_BERT_BEST", BCHP_SDS_BERT_0_BEST, SATFE_RegisterType_eISB},
   {"SDS_BERT_ACMCTL", BCHP_SDS_BERT_0_ACMCTL, SATFE_RegisterType_eISB},
   {"SDS_BL_BLPCTL", BCHP_SDS_BL_0_BLPCTL, SATFE_RegisterType_eISB},
   {"SDS_BL_PFCTL", BCHP_SDS_BL_0_PFCTL, SATFE_RegisterType_eISB},
   {"SDS_BL_BRSW", BCHP_SDS_BL_0_BRSW, SATFE_RegisterType_eISB},
   {"SDS_BL_BRLC", BCHP_SDS_BL_0_BRLC, SATFE_RegisterType_eISB},
   {"SDS_BL_BRIC", BCHP_SDS_BL_0_BRIC, SATFE_RegisterType_eISB},
   {"SDS_BL_BRI", BCHP_SDS_BL_0_BRI, SATFE_RegisterType_eISB},
   {"SDS_BL_BFOS", BCHP_SDS_BL_0_BFOS, SATFE_RegisterType_eISB},
   {"SDS_BL_BRFO", BCHP_SDS_BL_0_BRFO, SATFE_RegisterType_eISB},
   {"SDS_BL_BNCO", BCHP_SDS_BL_0_BNCO, SATFE_RegisterType_eISB},
   {"SDS_CG_RSTCTL", BCHP_SDS_CG_0_RSTCTL, SATFE_RegisterType_eISB},
   {"SDS_CG_CGDIV00", BCHP_SDS_CG_0_CGDIV00, SATFE_RegisterType_eISB},
   {"SDS_CG_CGDIV01", BCHP_SDS_CG_0_CGDIV01, SATFE_RegisterType_eISB},
   {"SDS_CG_SPLL_NPDIV", BCHP_SDS_CG_0_SPLL_NPDIV, SATFE_RegisterType_eISB},
   {"SDS_CG_SPLL_MDIV_CTRL", BCHP_SDS_CG_0_SPLL_MDIV_CTRL, SATFE_RegisterType_eISB},
   {"SDS_CG_SPLL_CTRL", BCHP_SDS_CG_0_SPLL_CTRL, SATFE_RegisterType_eISB},
   {"SDS_CG_SPLL_SSC_CTRL1", BCHP_SDS_CG_0_SPLL_SSC_CTRL1, SATFE_RegisterType_eISB},
   {"SDS_CG_SPLL_SSC_CTRL0", BCHP_SDS_CG_0_SPLL_SSC_CTRL0, SATFE_RegisterType_eISB},
   {"SDS_CG_SPLL_STATUS", BCHP_SDS_CG_0_SPLL_STATUS, SATFE_RegisterType_eISB},
   {"SDS_CG_SPLL_PWRDN_RST", BCHP_SDS_CG_0_SPLL_PWRDN_RST, SATFE_RegisterType_eISB},
   {"SDS_CL_CLCTL1", BCHP_SDS_CL_0_CLCTL1, SATFE_RegisterType_eISB},
   {"SDS_CL_CLCTL2", BCHP_SDS_CL_0_CLCTL2, SATFE_RegisterType_eISB},
   {"SDS_CL_FLLC", BCHP_SDS_CL_0_FLLC, SATFE_RegisterType_eISB},
   {"SDS_CL_FLLC1", BCHP_SDS_CL_0_FLLC1, SATFE_RegisterType_eISB},
   {"SDS_CL_FLIC", BCHP_SDS_CL_0_FLIC, SATFE_RegisterType_eISB},
   {"SDS_CL_FLIC1", BCHP_SDS_CL_0_FLIC1, SATFE_RegisterType_eISB},
   {"SDS_CL_FLSW", BCHP_SDS_CL_0_FLSW, SATFE_RegisterType_eISB},
   {"SDS_CL_FLI", BCHP_SDS_CL_0_FLI, SATFE_RegisterType_eISB},
   {"SDS_CL_FLIF", BCHP_SDS_CL_0_FLIF, SATFE_RegisterType_eISB},
   {"SDS_CL_FLPA", BCHP_SDS_CL_0_FLPA, SATFE_RegisterType_eISB},
   {"SDS_CL_FLTD", BCHP_SDS_CL_0_FLTD, SATFE_RegisterType_eISB},
   {"SDS_CL_PEEST", BCHP_SDS_CL_0_PEEST, SATFE_RegisterType_eISB},
   {"SDS_CL_PLTD", BCHP_SDS_CL_0_PLTD, SATFE_RegisterType_eISB},
   {"SDS_CL_PLC", BCHP_SDS_CL_0_PLC, SATFE_RegisterType_eISB},
   {"SDS_CL_PLC1", BCHP_SDS_CL_0_PLC1, SATFE_RegisterType_eISB},
   {"SDS_CL_PLSW", BCHP_SDS_CL_0_PLSW, SATFE_RegisterType_eISB},
   {"SDS_CL_PLI", BCHP_SDS_CL_0_PLI, SATFE_RegisterType_eISB},
   {"SDS_CL_PLPA", BCHP_SDS_CL_0_PLPA, SATFE_RegisterType_eISB},
   {"SDS_CL_CRBFD", BCHP_SDS_CL_0_CRBFD, SATFE_RegisterType_eISB},
   {"SDS_CL_CLHT", BCHP_SDS_CL_0_CLHT, SATFE_RegisterType_eISB},
   {"SDS_CL_CLLT", BCHP_SDS_CL_0_CLLT, SATFE_RegisterType_eISB},
   {"SDS_CL_CLLA", BCHP_SDS_CL_0_CLLA, SATFE_RegisterType_eISB},
   {"SDS_CL_CLCT", BCHP_SDS_CL_0_CLCT, SATFE_RegisterType_eISB},
   {"SDS_CL_CLFFCTL", BCHP_SDS_CL_0_CLFFCTL, SATFE_RegisterType_eISB},
   {"SDS_CL_FFLPA", BCHP_SDS_CL_0_FFLPA, SATFE_RegisterType_eISB},
   {"SDS_CL_CLFBCTL", BCHP_SDS_CL_0_CLFBCTL, SATFE_RegisterType_eISB},
   {"SDS_CL_FBLC", BCHP_SDS_CL_0_FBLC, SATFE_RegisterType_eISB},
   {"SDS_CL_FBLI", BCHP_SDS_CL_0_FBLI, SATFE_RegisterType_eISB},
   {"SDS_CL_FBPA", BCHP_SDS_CL_0_FBPA, SATFE_RegisterType_eISB},
   {"SDS_CL_CLDAFECTL", BCHP_SDS_CL_0_CLDAFECTL, SATFE_RegisterType_eISB},
   {"SDS_CL_DAFELI", BCHP_SDS_CL_0_DAFELI, SATFE_RegisterType_eISB},
   {"SDS_CL_DAFEINT", BCHP_SDS_CL_0_DAFEINT, SATFE_RegisterType_eISB},
   {"SDS_CWC_CTRL1", BCHP_SDS_CWC_0_CTRL1, SATFE_RegisterType_eISB},
   {"SDS_CWC_CTRL2", BCHP_SDS_CWC_0_CTRL2, SATFE_RegisterType_eISB},
   {"SDS_CWC_LEAK", BCHP_SDS_CWC_0_LEAK, SATFE_RegisterType_eISB},
   {"SDS_CWC_SPUR_FCW1", BCHP_SDS_CWC_0_SPUR_FCW1, SATFE_RegisterType_eISB},
   {"SDS_CWC_SPUR_FCW2", BCHP_SDS_CWC_0_SPUR_FCW2, SATFE_RegisterType_eISB},
   {"SDS_CWC_SPUR_FCW3", BCHP_SDS_CWC_0_SPUR_FCW3, SATFE_RegisterType_eISB},
   {"SDS_CWC_SPUR_FCW4", BCHP_SDS_CWC_0_SPUR_FCW4, SATFE_RegisterType_eISB},
   {"SDS_CWC_SPUR_FCW5", BCHP_SDS_CWC_0_SPUR_FCW5, SATFE_RegisterType_eISB},
   {"SDS_CWC_SPUR_FCW6", BCHP_SDS_CWC_0_SPUR_FCW6, SATFE_RegisterType_eISB},
   {"SDS_CWC_LFC1", BCHP_SDS_CWC_0_LFC1, SATFE_RegisterType_eISB},
   {"SDS_CWC_LFC2", BCHP_SDS_CWC_0_LFC2, SATFE_RegisterType_eISB},
   {"SDS_CWC_LFC3", BCHP_SDS_CWC_0_LFC3, SATFE_RegisterType_eISB},
   {"SDS_CWC_LFC4", BCHP_SDS_CWC_0_LFC4, SATFE_RegisterType_eISB},
   {"SDS_CWC_LFC5", BCHP_SDS_CWC_0_LFC5, SATFE_RegisterType_eISB},
   {"SDS_CWC_LFC6", BCHP_SDS_CWC_0_LFC6, SATFE_RegisterType_eISB},
   {"SDS_CWC_INT1", BCHP_SDS_CWC_0_INT1, SATFE_RegisterType_eISB},
   {"SDS_CWC_INT2", BCHP_SDS_CWC_0_INT2, SATFE_RegisterType_eISB},
   {"SDS_CWC_INT3", BCHP_SDS_CWC_0_INT3, SATFE_RegisterType_eISB},
   {"SDS_CWC_INT4", BCHP_SDS_CWC_0_INT4, SATFE_RegisterType_eISB},
   {"SDS_CWC_INT5", BCHP_SDS_CWC_0_INT5, SATFE_RegisterType_eISB},
   {"SDS_CWC_INT6", BCHP_SDS_CWC_0_INT6, SATFE_RegisterType_eISB},
   {"SDS_CWC_COEFF_RWCTRL", BCHP_SDS_CWC_0_COEFF_RWCTRL, SATFE_RegisterType_eISB},
   {"SDS_CWC_COEFF", BCHP_SDS_CWC_0_COEFF, SATFE_RegisterType_eISB},
   {"SDS_CWC_FOFS1", BCHP_SDS_CWC_0_FOFS1, SATFE_RegisterType_eISB},
   {"SDS_CWC_FOFS2", BCHP_SDS_CWC_0_FOFS2, SATFE_RegisterType_eISB},
   {"SDS_CWC_FOFS3", BCHP_SDS_CWC_0_FOFS3, SATFE_RegisterType_eISB},
   {"SDS_CWC_FOFS4", BCHP_SDS_CWC_0_FOFS4, SATFE_RegisterType_eISB},
   {"SDS_CWC_FOFS5", BCHP_SDS_CWC_0_FOFS5, SATFE_RegisterType_eISB},
   {"SDS_CWC_FOFS6", BCHP_SDS_CWC_0_FOFS6, SATFE_RegisterType_eISB},
   {"SDS_DFT_CTRL0", BCHP_SDS_DFT_0_CTRL0, SATFE_RegisterType_eISB},
   {"SDS_DFT_CTRL1", BCHP_SDS_DFT_0_CTRL1, SATFE_RegisterType_eISB},
   {"SDS_DFT_STATUS", BCHP_SDS_DFT_0_STATUS, SATFE_RegisterType_eISB},
   {"SDS_DFT_RANGE_START", BCHP_SDS_DFT_0_RANGE_START, SATFE_RegisterType_eISB},
   {"SDS_DFT_RANGE_END", BCHP_SDS_DFT_0_RANGE_END, SATFE_RegisterType_eISB},
   {"SDS_DFT_DDFS_FCW", BCHP_SDS_DFT_0_DDFS_FCW, SATFE_RegisterType_eISB},
   {"SDS_DFT_PEAK_POW", BCHP_SDS_DFT_0_PEAK_POW, SATFE_RegisterType_eISB},
   {"SDS_DFT_PEAK_BIN", BCHP_SDS_DFT_0_PEAK_BIN, SATFE_RegisterType_eISB},
   {"SDS_DFT_TOTAL_POW", BCHP_SDS_DFT_0_TOTAL_POW, SATFE_RegisterType_eISB},
   {"SDS_DFT_MEM_RADDR", BCHP_SDS_DFT_0_MEM_RADDR, SATFE_RegisterType_eISB},
   {"SDS_DFT_MEM_RDATA", BCHP_SDS_DFT_0_MEM_RDATA, SATFE_RegisterType_eISB},
   {"SDS_DSEC_DSRST", BCHP_SDS_DSEC_DSRST, SATFE_RegisterType_eISB},
   {"SDS_DSEC_DSCGDIV", BCHP_SDS_DSEC_DSCGDIV, SATFE_RegisterType_eISB},
   {"SDS_DSEC_DSCTL00", BCHP_SDS_DSEC_DSCTL00, SATFE_RegisterType_eISB},
   {"SDS_DSEC_DSCTL01", BCHP_SDS_DSEC_DSCTL01, SATFE_RegisterType_eISB},
   {"SDS_DSEC_DSCTL02", BCHP_SDS_DSEC_DSCTL02, SATFE_RegisterType_eISB},
   {"SDS_DSEC_DSCTL03", BCHP_SDS_DSEC_DSCTL03, SATFE_RegisterType_eISB},
   {"SDS_DSEC_DSCMD", BCHP_SDS_DSEC_DSCMD, SATFE_RegisterType_eISB},
   {"SDS_DSEC_DSRPLY", BCHP_SDS_DSEC_DSRPLY, SATFE_RegisterType_eISB},
   {"SDS_DSEC_DSCMEMADR", BCHP_SDS_DSEC_DSCMEMADR, SATFE_RegisterType_eISB},
   {"SDS_DSEC_DSCMEMDAT", BCHP_SDS_DSEC_DSCMEMDAT, SATFE_RegisterType_eISB},
   {"SDS_DSEC_DSFIRCTL", BCHP_SDS_DSEC_DSFIRCTL, SATFE_RegisterType_eISB},
   {"SDS_DSEC_DS_MISC_CONTROL", BCHP_SDS_DSEC_DS_MISC_CONTROL, SATFE_RegisterType_eISB},
   {"SDS_DSEC_DS_PARITY", BCHP_SDS_DSEC_DS_PARITY, SATFE_RegisterType_eISB},
   {"SDS_DSEC_ADCTL", BCHP_SDS_DSEC_ADCTL, SATFE_RegisterType_eISB},
   {"SDS_DSEC_Q15T", BCHP_SDS_DSEC_Q15T, SATFE_RegisterType_eISB},
   {"SDS_DSEC_Q15T_TB", BCHP_SDS_DSEC_Q15T_TB, SATFE_RegisterType_eISB},
   {"SDS_DSEC_TB_LENGTH", BCHP_SDS_DSEC_TB_LENGTH, SATFE_RegisterType_eISB},
   {"SDS_DSEC_TPWC", BCHP_SDS_DSEC_TPWC, SATFE_RegisterType_eISB},
   {"SDS_DSEC_RXBT", BCHP_SDS_DSEC_RXBT, SATFE_RegisterType_eISB},
   {"SDS_DSEC_RXRT", BCHP_SDS_DSEC_RXRT, SATFE_RegisterType_eISB},
   {"SDS_DSEC_RBDT", BCHP_SDS_DSEC_RBDT, SATFE_RegisterType_eISB},
   {"SDS_DSEC_SLEW", BCHP_SDS_DSEC_SLEW, SATFE_RegisterType_eISB},
   {"SDS_DSEC_RERT", BCHP_SDS_DSEC_RERT, SATFE_RegisterType_eISB},
   {"SDS_DSEC_DSCT", BCHP_SDS_DSEC_DSCT, SATFE_RegisterType_eISB},
   {"SDS_DSEC_DTCT", BCHP_SDS_DSEC_DTCT, SATFE_RegisterType_eISB},
   {"SDS_DSEC_DDIO", BCHP_SDS_DSEC_DDIO, SATFE_RegisterType_eISB},
   {"SDS_DSEC_RTDC1", BCHP_SDS_DSEC_RTDC1, SATFE_RegisterType_eISB},
   {"SDS_DSEC_RTDC2", BCHP_SDS_DSEC_RTDC2, SATFE_RegisterType_eISB},
   {"SDS_DSEC_TCTL", BCHP_SDS_DSEC_TCTL, SATFE_RegisterType_eISB},
   {"SDS_DSEC_CICC", BCHP_SDS_DSEC_CICC, SATFE_RegisterType_eISB},
   {"SDS_DSEC_FCIC", BCHP_SDS_DSEC_FCIC, SATFE_RegisterType_eISB},
   {"SDS_DSEC_SCIC", BCHP_SDS_DSEC_SCIC, SATFE_RegisterType_eISB},
   {"SDS_DSEC_TSTM", BCHP_SDS_DSEC_TSTM, SATFE_RegisterType_eISB},
   {"SDS_DSEC_DST1", BCHP_SDS_DSEC_DST1, SATFE_RegisterType_eISB},
   {"SDS_DSEC_DST2", BCHP_SDS_DSEC_DST2, SATFE_RegisterType_eISB},
   {"SDS_DSEC_DS_SAR_THRSH", BCHP_SDS_DSEC_DS_SAR_THRSH, SATFE_RegisterType_eISB},
   {"SDS_DSEC_DS_SAR_DATA_OUT", BCHP_SDS_DSEC_DS_SAR_DATA_OUT, SATFE_RegisterType_eISB},
   {"SDS_DSEC_DS_SAR_DC_OFFSET", BCHP_SDS_DSEC_DS_SAR_DC_OFFSET, SATFE_RegisterType_eISB},
   {"SDS_DSEC_DS_SAR_LPF_INT", BCHP_SDS_DSEC_DS_SAR_LPF_INT, SATFE_RegisterType_eISB},
   {"SDS_DSEC_DS_SAR_CONTROL", BCHP_SDS_DSEC_DS_SAR_CONTROL, SATFE_RegisterType_eISB},
   {"SDS_DSEC_DS_COMMON_CONTROL", BCHP_SDS_DSEC_DS_COMMON_CONTROL, SATFE_RegisterType_eISB},
   {"SDS_DSEC_DSTMRCTL", BCHP_SDS_DSEC_DSTMRCTL, SATFE_RegisterType_eISB},
   {"SDS_DSEC_DSGENTMR1", BCHP_SDS_DSEC_DSGENTMR1, SATFE_RegisterType_eISB},
   {"SDS_DSEC_DSGENTMR2", BCHP_SDS_DSEC_DSGENTMR2, SATFE_RegisterType_eISB},
   {"SDS_DSEC_GR_BRIDGE_REVISION", BCHP_SDS_DSEC_GR_BRIDGE_REVISION, SATFE_RegisterType_eISB},
   {"SDS_DSEC_GR_BRIDGE_CTRL", BCHP_SDS_DSEC_GR_BRIDGE_CTRL, SATFE_RegisterType_eISB},
   {"SDS_DSEC_GR_BRIDGE_SW_INIT_0", BCHP_SDS_DSEC_GR_BRIDGE_SW_INIT_0, SATFE_RegisterType_eISB},
   {"SDS_DSEC_GR_BRIDGE_SW_INIT_1", BCHP_SDS_DSEC_GR_BRIDGE_SW_INIT_1, SATFE_RegisterType_eISB},
   {"SDS_DSEC_INTR2_CPU_STATUS", BCHP_SDS_DSEC_INTR2_CPU_STATUS, SATFE_RegisterType_eISB},
   {"SDS_DSEC_INTR2_CPU_SET", BCHP_SDS_DSEC_INTR2_CPU_SET, SATFE_RegisterType_eISB},
   {"SDS_DSEC_INTR2_CPU_CLEAR", BCHP_SDS_DSEC_INTR2_CPU_CLEAR, SATFE_RegisterType_eISB},
   {"SDS_DSEC_INTR2_CPU_MASK_STATUS", BCHP_SDS_DSEC_INTR2_CPU_MASK_STATUS, SATFE_RegisterType_eISB},
   {"SDS_DSEC_INTR2_CPU_MASK_SET", BCHP_SDS_DSEC_INTR2_CPU_MASK_SET, SATFE_RegisterType_eISB},
   {"SDS_DSEC_INTR2_CPU_MASK_CLEAR", BCHP_SDS_DSEC_INTR2_CPU_MASK_CLEAR, SATFE_RegisterType_eISB},
   {"SDS_DSEC_INTR2_PCI_STATUS", BCHP_SDS_DSEC_INTR2_PCI_STATUS, SATFE_RegisterType_eISB},
   {"SDS_DSEC_INTR2_PCI_SET", BCHP_SDS_DSEC_INTR2_PCI_SET, SATFE_RegisterType_eISB},
   {"SDS_DSEC_INTR2_PCI_CLEAR", BCHP_SDS_DSEC_INTR2_PCI_CLEAR, SATFE_RegisterType_eISB},
   {"SDS_DSEC_INTR2_PCI_MASK_STATUS", BCHP_SDS_DSEC_INTR2_PCI_MASK_STATUS, SATFE_RegisterType_eISB},
   {"SDS_DSEC_INTR2_PCI_MASK_SET", BCHP_SDS_DSEC_INTR2_PCI_MASK_SET, SATFE_RegisterType_eISB},
   {"SDS_DSEC_INTR2_PCI_MASK_CLEAR", BCHP_SDS_DSEC_INTR2_PCI_MASK_CLEAR, SATFE_RegisterType_eISB},
   {"SDS_DSEC_SW_SPARE_0", BCHP_SDS_DSEC_SW_SPARE_0, SATFE_RegisterType_eISB},
   {"SDS_DSEC_SW_SPARE_1", BCHP_SDS_DSEC_SW_SPARE_1, SATFE_RegisterType_eISB},
   {"SDS_EQ_EQMISCCTL", BCHP_SDS_EQ_0_EQMISCCTL, SATFE_RegisterType_eISB},
   {"SDS_EQ_EQFFECTL", BCHP_SDS_EQ_0_EQFFECTL, SATFE_RegisterType_eISB},
   {"SDS_EQ_EQCFAD", BCHP_SDS_EQ_0_EQCFAD, SATFE_RegisterType_eISB},
   {"SDS_EQ_EQFRZCTL", BCHP_SDS_EQ_0_EQFRZCTL, SATFE_RegisterType_eISB},
   {"SDS_EQ_F0B", BCHP_SDS_EQ_0_F0B, SATFE_RegisterType_eISB},
   {"SDS_EQ_HD8PSK1", BCHP_SDS_EQ_0_HD8PSK1, SATFE_RegisterType_eISB},
   {"SDS_EQ_HD8PSK2", BCHP_SDS_EQ_0_HD8PSK2, SATFE_RegisterType_eISB},
   {"SDS_EQ_HDQPSK", BCHP_SDS_EQ_0_HDQPSK, SATFE_RegisterType_eISB},
   {"SDS_EQ_HD16QAM", BCHP_SDS_EQ_0_HD16QAM, SATFE_RegisterType_eISB},
   {"SDS_EQ_CMA", BCHP_SDS_EQ_0_CMA, SATFE_RegisterType_eISB},
   {"SDS_EQ_CMATH", BCHP_SDS_EQ_0_CMATH, SATFE_RegisterType_eISB},
   {"SDS_EQ_VLCTL", BCHP_SDS_EQ_0_VLCTL, SATFE_RegisterType_eISB},
   {"SDS_EQ_VLCI", BCHP_SDS_EQ_0_VLCI, SATFE_RegisterType_eISB},
   {"SDS_EQ_VLCQ", BCHP_SDS_EQ_0_VLCQ, SATFE_RegisterType_eISB},
   {"SDS_EQ_VCOS", BCHP_SDS_EQ_0_VCOS, SATFE_RegisterType_eISB},
   {"SDS_EQ_TSFT", BCHP_SDS_EQ_0_TSFT, SATFE_RegisterType_eISB},
   {"SDS_EQ_EQSFT", BCHP_SDS_EQ_0_EQSFT, SATFE_RegisterType_eISB},
   {"SDS_EQ_PILOTCTL", BCHP_SDS_EQ_0_PILOTCTL, SATFE_RegisterType_eISB},
   {"SDS_EQ_PLDCTL", BCHP_SDS_EQ_0_PLDCTL, SATFE_RegisterType_eISB},
   {"SDS_EQ_HDRD", BCHP_SDS_EQ_0_HDRD, SATFE_RegisterType_eISB},
   {"SDS_EQ_HDRA", BCHP_SDS_EQ_0_HDRA, SATFE_RegisterType_eISB},
   {"SDS_EQ_XSEED", BCHP_SDS_EQ_0_XSEED, SATFE_RegisterType_eISB},
   {"SDS_EQ_XTAP1", BCHP_SDS_EQ_0_XTAP1, SATFE_RegisterType_eISB},
   {"SDS_EQ_XTAP2", BCHP_SDS_EQ_0_XTAP2, SATFE_RegisterType_eISB},
   {"SDS_EQ_LUPD", BCHP_SDS_EQ_0_LUPD, SATFE_RegisterType_eISB},
   {"SDS_EQ_LUPA", BCHP_SDS_EQ_0_LUPA, SATFE_RegisterType_eISB},
   {"SDS_EQ_SDSLEN", BCHP_SDS_EQ_0_SDSLEN, SATFE_RegisterType_eISB},
   {"SDS_EQ_SDSIG", BCHP_SDS_EQ_0_SDSIG, SATFE_RegisterType_eISB},
   {"SDS_EQ_MGAIND", BCHP_SDS_EQ_0_MGAIND, SATFE_RegisterType_eISB},
   {"SDS_EQ_MGAINA", BCHP_SDS_EQ_0_MGAINA, SATFE_RegisterType_eISB},
   {"SDS_FEC_FECTL", BCHP_SDS_FEC_0_FECTL, SATFE_RegisterType_eISB},
   {"SDS_FEC_FSYN", BCHP_SDS_FEC_0_FSYN, SATFE_RegisterType_eISB},
   {"SDS_FEC_FRS", BCHP_SDS_FEC_0_FRS, SATFE_RegisterType_eISB},
   {"SDS_FEC_FMOD", BCHP_SDS_FEC_0_FMOD, SATFE_RegisterType_eISB},
   {"SDS_FEC_FERR", BCHP_SDS_FEC_0_FERR, SATFE_RegisterType_eISB},
   {"SDS_FEC_FRSV", BCHP_SDS_FEC_0_FRSV, SATFE_RegisterType_eISB},
   {"SDS_FE_ADCPCTL", BCHP_SDS_FE_0_ADCPCTL, SATFE_RegisterType_eISB},
   {"SDS_FE_DCOCTL", BCHP_SDS_FE_0_DCOCTL, SATFE_RegisterType_eISB},
   {"SDS_FE_DCOI", BCHP_SDS_FE_0_DCOI, SATFE_RegisterType_eISB},
   {"SDS_FE_IQCTL", BCHP_SDS_FE_0_IQCTL, SATFE_RegisterType_eISB},
   {"SDS_FE_IQAEST", BCHP_SDS_FE_0_IQAEST, SATFE_RegisterType_eISB},
   {"SDS_FE_IQPEST", BCHP_SDS_FE_0_IQPEST, SATFE_RegisterType_eISB},
   {"SDS_FE_MIXCTL", BCHP_SDS_FE_0_MIXCTL, SATFE_RegisterType_eISB},
   {"SDS_FE_DSTGCTL", BCHP_SDS_FE_0_DSTGCTL, SATFE_RegisterType_eISB},
   {"SDS_FE_FILTCTL", BCHP_SDS_FE_0_FILTCTL, SATFE_RegisterType_eISB},
   {"SDS_FE_DFCTL", BCHP_SDS_FE_0_DFCTL, SATFE_RegisterType_eISB},
   {"SDS_FE_AGFCTL", BCHP_SDS_FE_0_AGFCTL, SATFE_RegisterType_eISB},
   {"SDS_FE_AGF", BCHP_SDS_FE_0_AGF, SATFE_RegisterType_eISB},
   {"SDS_FE_AIF", BCHP_SDS_FE_0_AIF, SATFE_RegisterType_eISB},
   {"SDS_FE_NVCTL", BCHP_SDS_FE_0_NVCTL, SATFE_RegisterType_eISB},
   {"SDS_GR_BRIDGE_REVISION", BCHP_SDS_GR_BRIDGE_0_REVISION, SATFE_RegisterType_eISB},
   {"SDS_GR_BRIDGE_CTRL", BCHP_SDS_GR_BRIDGE_0_CTRL, SATFE_RegisterType_eISB},
   {"SDS_GR_BRIDGE_SW_INIT_0", BCHP_SDS_GR_BRIDGE_0_SW_INIT_0, SATFE_RegisterType_eISB},
   {"SDS_GR_BRIDGE_SW_INIT_1", BCHP_SDS_GR_BRIDGE_0_SW_INIT_1, SATFE_RegisterType_eISB},
   {"SDS_HP_HPCONTROL", BCHP_SDS_HP_0_HPCONTROL, SATFE_RegisterType_eISB},
   {"SDS_HP_HPCONFIG", BCHP_SDS_HP_0_HPCONFIG, SATFE_RegisterType_eISB},
   {"SDS_HP_FNORM", BCHP_SDS_HP_0_FNORM, SATFE_RegisterType_eISB},
   {"SDS_HP_HPOVERRIDE", BCHP_SDS_HP_0_HPOVERRIDE, SATFE_RegisterType_eISB},
   {"SDS_HP_FROF1", BCHP_SDS_HP_0_FROF1, SATFE_RegisterType_eISB},
   {"SDS_HP_FROF2", BCHP_SDS_HP_0_FROF2, SATFE_RegisterType_eISB},
   {"SDS_HP_FROF3", BCHP_SDS_HP_0_FROF3, SATFE_RegisterType_eISB},
   {"SDS_HP_FROF1_SW", BCHP_SDS_HP_0_FROF1_SW, SATFE_RegisterType_eISB},
   {"SDS_HP_FROF2_SW", BCHP_SDS_HP_0_FROF2_SW, SATFE_RegisterType_eISB},
   {"SDS_HP_FROF3_SW", BCHP_SDS_HP_0_FROF3_SW, SATFE_RegisterType_eISB},
   {"SDS_HP_M_N_PEAK_VERIFY", BCHP_SDS_HP_0_M_N_PEAK_VERIFY, SATFE_RegisterType_eISB},
   {"SDS_HP_M_N_RECEIVER_VERIFY", BCHP_SDS_HP_0_M_N_RECEIVER_VERIFY, SATFE_RegisterType_eISB},
   {"SDS_HP_M_N_RECEIVER_LOCK", BCHP_SDS_HP_0_M_N_RECEIVER_LOCK, SATFE_RegisterType_eISB},
   {"SDS_HP_DCORR_THRESHOLD", BCHP_SDS_HP_0_DCORR_THRESHOLD, SATFE_RegisterType_eISB},
   {"SDS_HP_PLHDRSCR1", BCHP_SDS_HP_0_PLHDRSCR1, SATFE_RegisterType_eISB},
   {"SDS_HP_PLHDRSCR2", BCHP_SDS_HP_0_PLHDRSCR2, SATFE_RegisterType_eISB},
   {"SDS_HP_PLHDRSCR3", BCHP_SDS_HP_0_PLHDRSCR3, SATFE_RegisterType_eISB},
   {"SDS_HP_ACM_CHECK", BCHP_SDS_HP_0_ACM_CHECK, SATFE_RegisterType_eISB},
   {"SDS_HP_FRAME_LENGTH_INITIAL", BCHP_SDS_HP_0_FRAME_LENGTH_INITIAL, SATFE_RegisterType_eISB},
   {"SDS_HP_FRAME_LENGTH_DUMMY_NORMAL", BCHP_SDS_HP_0_FRAME_LENGTH_DUMMY_NORMAL, SATFE_RegisterType_eISB},
   {"SDS_HP_FRAME_LENGTH_QPSK_NORMAL", BCHP_SDS_HP_0_FRAME_LENGTH_QPSK_NORMAL, SATFE_RegisterType_eISB},
   {"SDS_HP_FRAME_LENGTH_8PSK_NORMAL", BCHP_SDS_HP_0_FRAME_LENGTH_8PSK_NORMAL, SATFE_RegisterType_eISB},
   {"SDS_HP_FRAME_LENGTH_16APSK_NORMAL", BCHP_SDS_HP_0_FRAME_LENGTH_16APSK_NORMAL, SATFE_RegisterType_eISB},
   {"SDS_HP_FRAME_LENGTH_32APSK_NORMAL", BCHP_SDS_HP_0_FRAME_LENGTH_32APSK_NORMAL, SATFE_RegisterType_eISB},
   {"SDS_HP_FRAME_LENGTH_RESERVED_29_NORMAL", BCHP_SDS_HP_0_FRAME_LENGTH_RESERVED_29_NORMAL, SATFE_RegisterType_eISB},
   {"SDS_HP_FRAME_LENGTH_RESERVED_30_NORMAL", BCHP_SDS_HP_0_FRAME_LENGTH_RESERVED_30_NORMAL, SATFE_RegisterType_eISB},
   {"SDS_HP_FRAME_LENGTH_RESERVED_31_NORMAL", BCHP_SDS_HP_0_FRAME_LENGTH_RESERVED_31_NORMAL, SATFE_RegisterType_eISB},
   {"SDS_HP_FRAME_LENGTH_DUMMY_SHORT", BCHP_SDS_HP_0_FRAME_LENGTH_DUMMY_SHORT, SATFE_RegisterType_eISB},
   {"SDS_HP_FRAME_LENGTH_QPSK_SHORT", BCHP_SDS_HP_0_FRAME_LENGTH_QPSK_SHORT, SATFE_RegisterType_eISB},
   {"SDS_HP_FRAME_LENGTH_8PSK_SHORT", BCHP_SDS_HP_0_FRAME_LENGTH_8PSK_SHORT, SATFE_RegisterType_eISB},
   {"SDS_HP_FRAME_LENGTH_16APSK_SHORT", BCHP_SDS_HP_0_FRAME_LENGTH_16APSK_SHORT, SATFE_RegisterType_eISB},
   {"SDS_HP_FRAME_LENGTH_32APSK_SHORT", BCHP_SDS_HP_0_FRAME_LENGTH_32APSK_SHORT, SATFE_RegisterType_eISB},
   {"SDS_HP_FRAME_LENGTH_RESERVED_29_SHORT", BCHP_SDS_HP_0_FRAME_LENGTH_RESERVED_29_SHORT, SATFE_RegisterType_eISB},
   {"SDS_HP_FRAME_LENGTH_RESERVED_30_SHORT", BCHP_SDS_HP_0_FRAME_LENGTH_RESERVED_30_SHORT, SATFE_RegisterType_eISB},
   {"SDS_HP_FRAME_LENGTH_RESERVED_31_SHORT", BCHP_SDS_HP_0_FRAME_LENGTH_RESERVED_31_SHORT, SATFE_RegisterType_eISB},
   {"SDS_HP_FRAME_LENGTH_SAMPLE", BCHP_SDS_HP_0_FRAME_LENGTH_SAMPLE, SATFE_RegisterType_eISB},
   {"SDS_HP_PEAK_SAMPLE_0", BCHP_SDS_HP_0_PEAK_SAMPLE_0, SATFE_RegisterType_eISB},
   {"SDS_HP_PEAK_SAMPLE_1", BCHP_SDS_HP_0_PEAK_SAMPLE_1, SATFE_RegisterType_eISB},
   {"SDS_HP_PEAK_SAMPLE_2", BCHP_SDS_HP_0_PEAK_SAMPLE_2, SATFE_RegisterType_eISB},
   {"SDS_HP_PEAK_SAMPLE_3", BCHP_SDS_HP_0_PEAK_SAMPLE_3, SATFE_RegisterType_eISB},
   {"SDS_HP_HP_DAFE", BCHP_SDS_HP_0_HP_DAFE, SATFE_RegisterType_eISB},
   {"SDS_HP_NEW_STATE", BCHP_SDS_HP_0_NEW_STATE, SATFE_RegisterType_eISB},
   {"SDS_HP_IGNORE_PHI_FROM_DAFE", BCHP_SDS_HP_0_IGNORE_PHI_FROM_DAFE, SATFE_RegisterType_eISB},
   {"SDS_INTR2_0_CPU_STATUS", BCHP_SDS_INTR2_0_0_CPU_STATUS, SATFE_RegisterType_eISB},
   {"SDS_INTR2_0_CPU_SET", BCHP_SDS_INTR2_0_0_CPU_SET, SATFE_RegisterType_eISB},
   {"SDS_INTR2_0_CPU_CLEAR", BCHP_SDS_INTR2_0_0_CPU_CLEAR, SATFE_RegisterType_eISB},
   {"SDS_INTR2_0_CPU_MASK_STATUS", BCHP_SDS_INTR2_0_0_CPU_MASK_STATUS, SATFE_RegisterType_eISB},
   {"SDS_INTR2_0_CPU_MASK_SET", BCHP_SDS_INTR2_0_0_CPU_MASK_SET, SATFE_RegisterType_eISB},
   {"SDS_INTR2_0_CPU_MASK_CLEAR", BCHP_SDS_INTR2_0_0_CPU_MASK_CLEAR, SATFE_RegisterType_eISB},
   {"SDS_INTR2_0_PCI_STATUS", BCHP_SDS_INTR2_0_0_PCI_STATUS, SATFE_RegisterType_eISB},
   {"SDS_INTR2_0_PCI_SET", BCHP_SDS_INTR2_0_0_PCI_SET, SATFE_RegisterType_eISB},
   {"SDS_INTR2_0_PCI_CLEAR", BCHP_SDS_INTR2_0_0_PCI_CLEAR, SATFE_RegisterType_eISB},
   {"SDS_INTR2_0_PCI_MASK_STATUS", BCHP_SDS_INTR2_0_0_PCI_MASK_STATUS, SATFE_RegisterType_eISB},
   {"SDS_INTR2_0_PCI_MASK_SET", BCHP_SDS_INTR2_0_0_PCI_MASK_SET, SATFE_RegisterType_eISB},
   {"SDS_INTR2_0_PCI_MASK_CLEAR", BCHP_SDS_INTR2_0_0_PCI_MASK_CLEAR, SATFE_RegisterType_eISB},
   {"SDS_MISC_REVID", BCHP_SDS_MISC_0_REVID, SATFE_RegisterType_eISB},
   {"SDS_MISC_IICTL1", BCHP_SDS_MISC_0_IICTL1, SATFE_RegisterType_eISB},
   {"SDS_MISC_IICTL2", BCHP_SDS_MISC_0_IICTL2, SATFE_RegisterType_eISB},
   {"SDS_MISC_IICCNT", BCHP_SDS_MISC_0_IICCNT, SATFE_RegisterType_eISB},
   {"SDS_MISC_IICHPA", BCHP_SDS_MISC_0_IICHPA, SATFE_RegisterType_eISB},
   {"SDS_MISC_MIICTX1", BCHP_SDS_MISC_0_MIICTX1, SATFE_RegisterType_eISB},
   {"SDS_MISC_MIICTX2", BCHP_SDS_MISC_0_MIICTX2, SATFE_RegisterType_eISB},
   {"SDS_MISC_MIICRX1", BCHP_SDS_MISC_0_MIICRX1, SATFE_RegisterType_eISB},
   {"SDS_MISC_MIICRX2", BCHP_SDS_MISC_0_MIICRX2, SATFE_RegisterType_eISB},
   {"SDS_MISC_MI2CSA", BCHP_SDS_MISC_0_MI2CSA, SATFE_RegisterType_eISB},
   {"SDS_MISC_TMRCTL", BCHP_SDS_MISC_0_TMRCTL, SATFE_RegisterType_eISB},
   {"SDS_MISC_GENTMR3", BCHP_SDS_MISC_0_GENTMR3, SATFE_RegisterType_eISB},
   {"SDS_MISC_GENTMR2", BCHP_SDS_MISC_0_GENTMR2, SATFE_RegisterType_eISB},
   {"SDS_MISC_GENTMR1", BCHP_SDS_MISC_0_GENTMR1, SATFE_RegisterType_eISB},
   {"SDS_MISC_BERTMR", BCHP_SDS_MISC_0_BERTMR, SATFE_RegisterType_eISB},
   {"SDS_MISC_BTMR", BCHP_SDS_MISC_0_BTMR, SATFE_RegisterType_eISB},
   {"SDS_MISC_TPDIR", BCHP_SDS_MISC_0_TPDIR, SATFE_RegisterType_eISB},
   {"SDS_MISC_TPODS", BCHP_SDS_MISC_0_TPODS, SATFE_RegisterType_eISB},
   {"SDS_MISC_TPDS", BCHP_SDS_MISC_0_TPDS, SATFE_RegisterType_eISB},
   {"SDS_MISC_TPCTL1", BCHP_SDS_MISC_0_TPCTL1, SATFE_RegisterType_eISB},
   {"SDS_MISC_TPCTL2", BCHP_SDS_MISC_0_TPCTL2, SATFE_RegisterType_eISB},
   {"SDS_MISC_TPCTL3", BCHP_SDS_MISC_0_TPCTL3, SATFE_RegisterType_eISB},
   {"SDS_MISC_TPOUT", BCHP_SDS_MISC_0_TPOUT, SATFE_RegisterType_eISB},
   {"SDS_MISC_MISCTL", BCHP_SDS_MISC_0_MISCTL, SATFE_RegisterType_eISB},
   {"SDS_MISC_INTR_RAW_STS0", BCHP_SDS_MISC_0_INTR_RAW_STS0, SATFE_RegisterType_eISB},
   {"SDS_MISC_INTR_RAW_STS1", BCHP_SDS_MISC_0_INTR_RAW_STS1, SATFE_RegisterType_eISB},
   {"SDS_OI_OIFCTL00", BCHP_SDS_OI_0_OIFCTL00, SATFE_RegisterType_eISB},
   {"SDS_OI_OIFCTL01", BCHP_SDS_OI_0_OIFCTL01, SATFE_RegisterType_eISB},
   {"SDS_OI_OIFCTL02", BCHP_SDS_OI_0_OIFCTL02, SATFE_RegisterType_eISB},
   {"SDS_OI_OPLL", BCHP_SDS_OI_0_OPLL, SATFE_RegisterType_eISB},
   {"SDS_OI_OPLL2", BCHP_SDS_OI_0_OPLL2, SATFE_RegisterType_eISB},
   {"SDS_OI_FERC", BCHP_SDS_OI_0_FERC, SATFE_RegisterType_eISB},
   {"SDS_OI_FRC", BCHP_SDS_OI_0_FRC, SATFE_RegisterType_eISB},
   {"SDS_OI_OSIGPN", BCHP_SDS_OI_0_OSIGPN, SATFE_RegisterType_eISB},
   {"SDS_OI_OSUBD", BCHP_SDS_OI_0_OSUBD, SATFE_RegisterType_eISB},
   {"SDS_OI_OCOEF", BCHP_SDS_OI_0_OCOEF, SATFE_RegisterType_eISB},
   {"SDS_OI_OFI", BCHP_SDS_OI_0_OFI, SATFE_RegisterType_eISB},
   {"SDS_OI_OPLL_NPDIV", BCHP_SDS_OI_0_OPLL_NPDIV, SATFE_RegisterType_eISB},
   {"SDS_OI_OPLL_MDIV_CTRL", BCHP_SDS_OI_0_OPLL_MDIV_CTRL, SATFE_RegisterType_eISB},
   {"SDS_OI_OPLL_CTRL", BCHP_SDS_OI_0_OPLL_CTRL, SATFE_RegisterType_eISB},
   {"SDS_OI_OPLL_SSC_CTRL1", BCHP_SDS_OI_0_OPLL_SSC_CTRL1, SATFE_RegisterType_eISB},
   {"SDS_OI_OPLL_SSC_CTRL0", BCHP_SDS_OI_0_OPLL_SSC_CTRL0, SATFE_RegisterType_eISB},
   {"SDS_OI_OPLL_STATUS", BCHP_SDS_OI_0_OPLL_STATUS, SATFE_RegisterType_eISB},
   {"SDS_OI_OPLL_PWRDN_RST", BCHP_SDS_OI_0_OPLL_PWRDN_RST, SATFE_RegisterType_eISB},
   {"SDS_SNR_SNRCTL", BCHP_SDS_SNR_0_SNRCTL, SATFE_RegisterType_eISB},
   {"SDS_SNR_SNRHT", BCHP_SDS_SNR_0_SNRHT, SATFE_RegisterType_eISB},
   {"SDS_SNR_SNRLT", BCHP_SDS_SNR_0_SNRLT, SATFE_RegisterType_eISB},
   {"SDS_SNR_SNRE", BCHP_SDS_SNR_0_SNRE, SATFE_RegisterType_eISB},
   {"SDS_SNR_SNORETP", BCHP_SDS_SNR_0_SNORETP, SATFE_RegisterType_eISB},
   {"SDS_SNR_SNORESP", BCHP_SDS_SNR_0_SNORESP, SATFE_RegisterType_eISB},
   {"SDS_SNR_SNORECTL", BCHP_SDS_SNR_0_SNORECTL, SATFE_RegisterType_eISB},
   {"SDS_VIT_VTCTL", BCHP_SDS_VIT_0_VTCTL, SATFE_RegisterType_eISB},
   {"SDS_VIT_V10", BCHP_SDS_VIT_0_V10, SATFE_RegisterType_eISB},
   {"SDS_VIT_V32", BCHP_SDS_VIT_0_V32, SATFE_RegisterType_eISB},
   {"SDS_VIT_V54", BCHP_SDS_VIT_0_V54, SATFE_RegisterType_eISB},
   {"SDS_VIT_V76", BCHP_SDS_VIT_0_V76, SATFE_RegisterType_eISB},
   {"SDS_VIT_VINT", BCHP_SDS_VIT_0_VINT, SATFE_RegisterType_eISB},
   {"SDS_VIT_VCNT", BCHP_SDS_VIT_0_VCNT, SATFE_RegisterType_eISB},
   {"SDS_VIT_VSTC", BCHP_SDS_VIT_0_VSTC, SATFE_RegisterType_eISB},
   {"SDS_VIT_VST", BCHP_SDS_VIT_0_VST, SATFE_RegisterType_eISB},
   {"SDS_VIT_VREC", BCHP_SDS_VIT_0_VREC, SATFE_RegisterType_eISB},
   {"SDS_VIT_VRCV", BCHP_SDS_VIT_0_VRCV, SATFE_RegisterType_eISB},
#endif

   {"STB_CHAN_CHx_DEC_FCW", BCHP_STB_CHAN_CH0_DEC_FCW, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H0", BCHP_STB_CHAN_CH0_ACI_H0, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H1", BCHP_STB_CHAN_CH0_ACI_H1, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H2", BCHP_STB_CHAN_CH0_ACI_H2, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H3", BCHP_STB_CHAN_CH0_ACI_H3, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H4", BCHP_STB_CHAN_CH0_ACI_H4, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H5", BCHP_STB_CHAN_CH0_ACI_H5, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H6", BCHP_STB_CHAN_CH0_ACI_H6, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H7", BCHP_STB_CHAN_CH0_ACI_H7, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H8", BCHP_STB_CHAN_CH0_ACI_H8, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H9", BCHP_STB_CHAN_CH0_ACI_H9, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H10", BCHP_STB_CHAN_CH0_ACI_H10, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H11", BCHP_STB_CHAN_CH0_ACI_H11, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H12", BCHP_STB_CHAN_CH0_ACI_H12, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H13", BCHP_STB_CHAN_CH0_ACI_H13, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H14", BCHP_STB_CHAN_CH0_ACI_H14, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H15", BCHP_STB_CHAN_CH0_ACI_H15, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H16", BCHP_STB_CHAN_CH0_ACI_H16, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H17", BCHP_STB_CHAN_CH0_ACI_H17, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H18", BCHP_STB_CHAN_CH0_ACI_H18, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H19", BCHP_STB_CHAN_CH0_ACI_H19, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H20", BCHP_STB_CHAN_CH0_ACI_H20, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H21", BCHP_STB_CHAN_CH0_ACI_H21, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H22", BCHP_STB_CHAN_CH0_ACI_H22, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H23", BCHP_STB_CHAN_CH0_ACI_H23, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H24", BCHP_STB_CHAN_CH0_ACI_H24, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H25", BCHP_STB_CHAN_CH0_ACI_H25, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H26", BCHP_STB_CHAN_CH0_ACI_H26, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H27", BCHP_STB_CHAN_CH0_ACI_H27, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H28", BCHP_STB_CHAN_CH0_ACI_H28, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H29", BCHP_STB_CHAN_CH0_ACI_H29, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H30", BCHP_STB_CHAN_CH0_ACI_H30, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H31", BCHP_STB_CHAN_CH0_ACI_H31, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H32", BCHP_STB_CHAN_CH0_ACI_H32, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H33", BCHP_STB_CHAN_CH0_ACI_H33, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H34", BCHP_STB_CHAN_CH0_ACI_H34, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_ACI_H35", BCHP_STB_CHAN_CH0_ACI_H35, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_NOTCH_0_CTRL", BCHP_STB_CHAN_CH0_NOTCH_0_CTRL, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_NOTCH_0_FCW", BCHP_STB_CHAN_CH0_NOTCH_0_FCW, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_NOTCH_0_INT_I", BCHP_STB_CHAN_CH0_NOTCH_0_INT_I, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_NOTCH_0_INT_Q", BCHP_STB_CHAN_CH0_NOTCH_0_INT_Q, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_NOTCH_0_INT_LF", BCHP_STB_CHAN_CH0_NOTCH_0_INT_LF, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_NOTCH_1_CTRL", BCHP_STB_CHAN_CH0_NOTCH_1_CTRL, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_NOTCH_1_FCW", BCHP_STB_CHAN_CH0_NOTCH_1_FCW, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_NOTCH_1_INT_I", BCHP_STB_CHAN_CH0_NOTCH_1_INT_I, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_NOTCH_1_INT_Q", BCHP_STB_CHAN_CH0_NOTCH_1_INT_Q, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_NOTCH_1_INT_LF", BCHP_STB_CHAN_CH0_NOTCH_1_INT_LF, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_NOTCH_LF_CTRL", BCHP_STB_CHAN_CH0_NOTCH_LF_CTRL, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_AGC_CTRL", BCHP_STB_CHAN_CH0_AGC_CTRL, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_AGC_LA_INT", BCHP_STB_CHAN_CH0_AGC_LA_INT, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_AGC_LF_INT", BCHP_STB_CHAN_CH0_AGC_LF_INT, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_CLIP_CTRL", BCHP_STB_CHAN_CH0_CLIP_CTRL, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_CLIP_COUNT", BCHP_STB_CHAN_CH0_CLIP_COUNT, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_CRC_COUNT", BCHP_STB_CHAN_CH0_CRC_COUNT, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_CRC_VALUE", BCHP_STB_CHAN_CH0_CRC_VALUE, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_FREQ_ERR_CTRL", BCHP_STB_CHAN_CH0_FREQ_ERR_CTRL, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_TP_OUT_CTRL", BCHP_STB_CHAN_CH0_TP_OUT_CTRL, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_SW_SPARE0", BCHP_STB_CHAN_CH0_SW_SPARE0, SATFE_RegisterType_eISB},
   {"STB_CHAN_CHx_SW_SPARE1", BCHP_STB_CHAN_CH0_SW_SPARE1, SATFE_RegisterType_eISB},
   {"STB_CHAN_CTRL_XBAR_SEL", BCHP_STB_CHAN_CTRL_XBAR_SEL, SATFE_RegisterType_eISB},
   {"STB_CHAN_CTRL_XBAR_CTRL", BCHP_STB_CHAN_CTRL_XBAR_CTRL, SATFE_RegisterType_eISB},
   {"STB_CHAN_CTRL_XBAR_IPATHTST", BCHP_STB_CHAN_CTRL_XBAR_IPATHTST, SATFE_RegisterType_eISB},
   {"STB_CHAN_CTRL_LOCAL_SW_RESET", BCHP_STB_CHAN_CTRL_LOCAL_SW_RESET, SATFE_RegisterType_eISB},
   {"STB_CHAN_CTRL_PWRDN", BCHP_STB_CHAN_CTRL_PWRDN, SATFE_RegisterType_eISB},
   {"STB_CHAN_CTRL_TP_OUT_CTRL", BCHP_STB_CHAN_CTRL_TP_OUT_CTRL, SATFE_RegisterType_eISB},
   {"STB_CHAN_CTRL_TEST_FCW", BCHP_STB_CHAN_CTRL_TEST_FCW, SATFE_RegisterType_eISB},
   {"STB_CHAN_CTRL_TEST_MIX", BCHP_STB_CHAN_CTRL_TEST_MIX, SATFE_RegisterType_eISB},
   {"STB_CHAN_CTRL_TEST_TONE", BCHP_STB_CHAN_CTRL_TEST_TONE, SATFE_RegisterType_eISB},
   {"STB_CHAN_CTRL_TEST_XSINX_H0", BCHP_STB_CHAN_CTRL_TEST_XSINX_H0, SATFE_RegisterType_eISB},
   {"STB_CHAN_CTRL_TEST_XSINX_H1", BCHP_STB_CHAN_CTRL_TEST_XSINX_H1, SATFE_RegisterType_eISB},
   {"STB_CHAN_CTRL_TEST_XSINX_H2", BCHP_STB_CHAN_CTRL_TEST_XSINX_H2, SATFE_RegisterType_eISB},
   {"STB_CHAN_CTRL_TEST_XSINX_H3", BCHP_STB_CHAN_CTRL_TEST_XSINX_H3, SATFE_RegisterType_eISB},
   {"STB_CHAN_CTRL_TEST_XSINX_H4", BCHP_STB_CHAN_CTRL_TEST_XSINX_H4, SATFE_RegisterType_eISB},
   {"STB_CHAN_CTRL_SW_SPARE0", BCHP_STB_CHAN_CTRL_SW_SPARE0, SATFE_RegisterType_eISB},
   {"STB_CHAN_CTRL_SW_SPARE1", BCHP_STB_CHAN_CTRL_SW_SPARE1, SATFE_RegisterType_eISB},

#if BCHP_CHIP==4538
   {"TFEC_TFECTL", BCHP_TFEC_0_TFECTL, SATFE_RegisterType_eISB},
   {"TFEC_TNBLK", BCHP_TFEC_0_TNBLK, SATFE_RegisterType_eISB},
   {"TFEC_TCBLK", BCHP_TFEC_0_TCBLK, SATFE_RegisterType_eISB},
   {"TFEC_TBBLK", BCHP_TFEC_0_TBBLK, SATFE_RegisterType_eISB},
   {"TFEC_TCSYM", BCHP_TFEC_0_TCSYM, SATFE_RegisterType_eISB},
   {"TFEC_TFMT", BCHP_TFEC_0_TFMT, SATFE_RegisterType_eISB},
   {"TFEC_TPAK", BCHP_TFEC_0_TPAK, SATFE_RegisterType_eISB},
   {"TFEC_TSSQ", BCHP_TFEC_0_TSSQ, SATFE_RegisterType_eISB},
   {"TFEC_TSYN", BCHP_TFEC_0_TSYN, SATFE_RegisterType_eISB},
   {"TFEC_TTUR", BCHP_TFEC_0_TTUR, SATFE_RegisterType_eISB},
   {"TFEC_TZPK", BCHP_TFEC_0_TZPK, SATFE_RegisterType_eISB},
   {"TFEC_TZSY", BCHP_TFEC_0_TZSY, SATFE_RegisterType_eISB},
   {"TFEC_TITR", BCHP_TFEC_0_TITR, SATFE_RegisterType_eISB},
   {"TFEC_TCIL", BCHP_TFEC_0_TCIL, SATFE_RegisterType_eISB},
   {"TFEC_TRSD", BCHP_TFEC_0_TRSD, SATFE_RegisterType_eISB},
   {"TFEC_TPN", BCHP_TFEC_0_TPN, SATFE_RegisterType_eISB},
   {"TFEC_TSIGCNT", BCHP_TFEC_0_TSIGCNT, SATFE_RegisterType_eISB},
   {"TFEC_TSIGITD", BCHP_TFEC_0_TSIGITD, SATFE_RegisterType_eISB},
   {"TFEC_TSIGXPT", BCHP_TFEC_0_TSIGXPT, SATFE_RegisterType_eISB},
   {"TFEC_TTPCTL", BCHP_TFEC_0_TTPCTL, SATFE_RegisterType_eISB},
   {"TFEC_TRAWISR", BCHP_TFEC_0_TRAWISR, SATFE_RegisterType_eISB},
   {"TFEC_GR_BRIDGE_REVISION", BCHP_TFEC_GR_BRIDGE_0_REVISION, SATFE_RegisterType_eISB},
   {"TFEC_GR_BRIDGE_CTRL", BCHP_TFEC_GR_BRIDGE_0_CTRL, SATFE_RegisterType_eISB},
   {"TFEC_GR_BRIDGE_SW_INIT_0", BCHP_TFEC_GR_BRIDGE_0_SW_INIT_0, SATFE_RegisterType_eISB},
   {"TFEC_GR_BRIDGE_SW_INIT_1", BCHP_TFEC_GR_BRIDGE_0_SW_INIT_1, SATFE_RegisterType_eISB},
   {"TFEC_INTR2_CPU_STATUS", BCHP_TFEC_INTR2_0_CPU_STATUS, SATFE_RegisterType_eISB},
   {"TFEC_INTR2_CPU_SET", BCHP_TFEC_INTR2_0_CPU_SET, SATFE_RegisterType_eISB},
   {"TFEC_INTR2_CPU_CLEAR", BCHP_TFEC_INTR2_0_CPU_CLEAR, SATFE_RegisterType_eISB},
   {"TFEC_INTR2_CPU_MASK_STATUS", BCHP_TFEC_INTR2_0_CPU_MASK_STATUS, SATFE_RegisterType_eISB},
   {"TFEC_INTR2_CPU_MASK_SET", BCHP_TFEC_INTR2_0_CPU_MASK_SET, SATFE_RegisterType_eISB},
   {"TFEC_INTR2_CPU_MASK_CLEAR", BCHP_TFEC_INTR2_0_CPU_MASK_CLEAR, SATFE_RegisterType_eISB},
   {"TFEC_INTR2_PCI_STATUS", BCHP_TFEC_INTR2_0_PCI_STATUS, SATFE_RegisterType_eISB},
   {"TFEC_INTR2_PCI_SET", BCHP_TFEC_INTR2_0_PCI_SET, SATFE_RegisterType_eISB},
   {"TFEC_INTR2_PCI_CLEAR", BCHP_TFEC_INTR2_0_PCI_CLEAR, SATFE_RegisterType_eISB},
   {"TFEC_INTR2_PCI_MASK_STATUS", BCHP_TFEC_INTR2_0_PCI_MASK_STATUS, SATFE_RegisterType_eISB},
   {"TFEC_INTR2_PCI_MASK_SET", BCHP_TFEC_INTR2_0_PCI_MASK_SET, SATFE_RegisterType_eISB},
   {"TFEC_INTR2_PCI_MASK_CLEAR", BCHP_TFEC_INTR2_0_PCI_MASK_CLEAR, SATFE_RegisterType_eISB},
   {"TFEC_MISC_POST_DIV_CTL", BCHP_TFEC_MISC_0_POST_DIV_CTL, SATFE_RegisterType_eISB},
   {"TFEC_MISC_REGF_STBY", BCHP_TFEC_MISC_0_REGF_STBY, SATFE_RegisterType_eISB},
   {"TFEC_MISC_MISCCTL", BCHP_TFEC_MISC_0_MISCCTL, SATFE_RegisterType_eISB},
   {"TIMER_TIMER_IS", BCHP_TIMER_TIMER_IS, SATFE_RegisterType_eISB},
   {"TIMER_RSVD0", BCHP_TIMER_RSVD0, SATFE_RegisterType_eISB},
   {"TIMER_TIMER0_CTRL", BCHP_TIMER_TIMER0_CTRL, SATFE_RegisterType_eISB},
   {"TIMER_TIMER1_CTRL", BCHP_TIMER_TIMER1_CTRL, SATFE_RegisterType_eISB},
   {"TIMER_TIMER2_CTRL", BCHP_TIMER_TIMER2_CTRL, SATFE_RegisterType_eISB},
   {"TIMER_TIMER3_CTRL", BCHP_TIMER_TIMER3_CTRL, SATFE_RegisterType_eISB},
   {"TIMER_TIMER0_STAT", BCHP_TIMER_TIMER0_STAT, SATFE_RegisterType_eISB},
   {"TIMER_TIMER1_STAT", BCHP_TIMER_TIMER1_STAT, SATFE_RegisterType_eISB},
   {"TIMER_TIMER2_STAT", BCHP_TIMER_TIMER2_STAT, SATFE_RegisterType_eISB},
   {"TIMER_TIMER3_STAT", BCHP_TIMER_TIMER3_STAT, SATFE_RegisterType_eISB},
   {"TIMER_WDTIMEOUT", BCHP_TIMER_WDTIMEOUT, SATFE_RegisterType_eISB},
   {"TIMER_WDCMD", BCHP_TIMER_WDCMD, SATFE_RegisterType_eISB},
   {"TIMER_WDCHIPRST_CNT", BCHP_TIMER_WDCHIPRST_CNT, SATFE_RegisterType_eISB},
   {"TIMER_WDCRS", BCHP_TIMER_WDCRS, SATFE_RegisterType_eISB},
   {"TIMER_WDCOUNT", BCHP_TIMER_WDCOUNT, SATFE_RegisterType_eISB},
   {"TIMER_WDCTRL", BCHP_TIMER_WDCTRL, SATFE_RegisterType_eISB},
   {"TM_FAMILY_ID", BCHP_TM_FAMILY_ID, SATFE_RegisterType_eISB},
   {"TM_CHIP_ID", BCHP_TM_CHIP_ID, SATFE_RegisterType_eISB},
   {"TM_REV_ID", BCHP_TM_REV_ID, SATFE_RegisterType_eISB},
   {"TM_REV_ID_INT", BCHP_TM_REV_ID_INT, SATFE_RegisterType_eISB},
   {"TM_SFT_RST0", BCHP_TM_SFT_RST0, SATFE_RegisterType_eISB},
   {"TM_SFT_RST_CFG0", BCHP_TM_SFT_RST_CFG0, SATFE_RegisterType_eISB},
   {"TM_SFT_RST1", BCHP_TM_SFT_RST1, SATFE_RegisterType_eISB},
   {"TM_SFT_RST_CFG1", BCHP_TM_SFT_RST_CFG1, SATFE_RegisterType_eISB},
   {"TM_PWRDN", BCHP_TM_PWRDN, SATFE_RegisterType_eISB},
   {"TM_MEM_CTRL0", BCHP_TM_MEM_CTRL0, SATFE_RegisterType_eISB},
   {"TM_MEM_CTRL1", BCHP_TM_MEM_CTRL1, SATFE_RegisterType_eISB},
   {"TM_MEM_CTRL2", BCHP_TM_MEM_CTRL2, SATFE_RegisterType_eISB},
   {"TM_MEM_BYP", BCHP_TM_MEM_BYP, SATFE_RegisterType_eISB},
   {"TM_OSC_CML_CTRL", BCHP_TM_OSC_CML_CTRL, SATFE_RegisterType_eISB},
   {"TM_MISC6", BCHP_TM_MISC6, SATFE_RegisterType_eISB},
   {"TM_MISC5", BCHP_TM_MISC5, SATFE_RegisterType_eISB},
   {"TM_MISC4", BCHP_TM_MISC4, SATFE_RegisterType_eISB},
   {"TM_CKCML_CTRL0", BCHP_TM_CKCML_CTRL0, SATFE_RegisterType_eISB},
   {"TM_CKCML_CTRL1", BCHP_TM_CKCML_CTRL1, SATFE_RegisterType_eISB},
   {"TM_CKCML_BG_IREF_CTRL", BCHP_TM_CKCML_BG_IREF_CTRL, SATFE_RegisterType_eISB},
   {"TM_ANA_PLL_STATUS", BCHP_TM_ANA_PLL_STATUS, SATFE_RegisterType_eISB},
   {"TM_LO1_PLL_STATUS", BCHP_TM_LO1_PLL_STATUS, SATFE_RegisterType_eISB},
   {"TM_LO2_PLL_STATUS", BCHP_TM_LO2_PLL_STATUS, SATFE_RegisterType_eISB},
   {"TM_DPM_PLL_STATUS", BCHP_TM_DPM_PLL_STATUS, SATFE_RegisterType_eISB},
   {"TM_REG_PLL_STATUS", BCHP_TM_REG_PLL_STATUS, SATFE_RegisterType_eISB},
   {"TM_SYS_PLL_STATUS", BCHP_TM_SYS_PLL_STATUS, SATFE_RegisterType_eISB},
   {"TM_ANA_PLL_LDO_CTRL", BCHP_TM_ANA_PLL_LDO_CTRL, SATFE_RegisterType_eISB},
   {"TM_LO1_PLL_LDO_CTRL", BCHP_TM_LO1_PLL_LDO_CTRL, SATFE_RegisterType_eISB},
   {"TM_LO2_PLL_LDO_CTRL", BCHP_TM_LO2_PLL_LDO_CTRL, SATFE_RegisterType_eISB},
   {"TM_REG_PLL_STAT_CTRL", BCHP_TM_REG_PLL_STAT_CTRL, SATFE_RegisterType_eISB},
   {"TM_SYS_PLL_STAT_CTRL", BCHP_TM_SYS_PLL_STAT_CTRL, SATFE_RegisterType_eISB},
   {"TM_ANA_PLL_PDIV", BCHP_TM_ANA_PLL_PDIV, SATFE_RegisterType_eISB},
   {"TM_ANA_PLL_NDIV_INT", BCHP_TM_ANA_PLL_NDIV_INT, SATFE_RegisterType_eISB},
   {"TM_ANA_PLL_NDIV_FRAC", BCHP_TM_ANA_PLL_NDIV_FRAC, SATFE_RegisterType_eISB},
   {"TM_ANA_PLL_CLK_CH0", BCHP_TM_ANA_PLL_CLK_CH0, SATFE_RegisterType_eISB},
   {"TM_ANA_PLL_CLK_CH1", BCHP_TM_ANA_PLL_CLK_CH1, SATFE_RegisterType_eISB},
   {"TM_ANA_PLL_CLK_CH2", BCHP_TM_ANA_PLL_CLK_CH2, SATFE_RegisterType_eISB},
   {"TM_ANA_PLL_RST", BCHP_TM_ANA_PLL_RST, SATFE_RegisterType_eISB},
   {"TM_ANA_PLL_DCO_CTRL", BCHP_TM_ANA_PLL_DCO_CTRL, SATFE_RegisterType_eISB},
   {"TM_ANA_PLL_GAIN_CTRL", BCHP_TM_ANA_PLL_GAIN_CTRL, SATFE_RegisterType_eISB},
   {"TM_ANA_PLL_MISC_CTRL", BCHP_TM_ANA_PLL_MISC_CTRL, SATFE_RegisterType_eISB},
   {"TM_LO1_PLL_PDIV", BCHP_TM_LO1_PLL_PDIV, SATFE_RegisterType_eISB},
   {"TM_LO1_PLL_NDIV_INT", BCHP_TM_LO1_PLL_NDIV_INT, SATFE_RegisterType_eISB},
   {"TM_LO1_PLL_NDIV_FRAC", BCHP_TM_LO1_PLL_NDIV_FRAC, SATFE_RegisterType_eISB},
   {"TM_LO1_PLL_CLK_CH0", BCHP_TM_LO1_PLL_CLK_CH0, SATFE_RegisterType_eISB},
   {"TM_LO1_PLL_CLK_CH1", BCHP_TM_LO1_PLL_CLK_CH1, SATFE_RegisterType_eISB},
   {"TM_LO1_PLL_CLK_CH2", BCHP_TM_LO1_PLL_CLK_CH2, SATFE_RegisterType_eISB},
   {"TM_LO1_PLL_RST", BCHP_TM_LO1_PLL_RST, SATFE_RegisterType_eISB},
   {"TM_LO1_PLL_DCO_CTRL", BCHP_TM_LO1_PLL_DCO_CTRL, SATFE_RegisterType_eISB},
   {"TM_LO1_PLL_GAIN_CTRL", BCHP_TM_LO1_PLL_GAIN_CTRL, SATFE_RegisterType_eISB},
   {"TM_LO1_PLL_MISC_CTRL", BCHP_TM_LO1_PLL_MISC_CTRL, SATFE_RegisterType_eISB},
   {"TM_LO2_PLL_PDIV", BCHP_TM_LO2_PLL_PDIV, SATFE_RegisterType_eISB},
   {"TM_LO2_PLL_NDIV_INT", BCHP_TM_LO2_PLL_NDIV_INT, SATFE_RegisterType_eISB},
   {"TM_LO2_PLL_NDIV_FRAC", BCHP_TM_LO2_PLL_NDIV_FRAC, SATFE_RegisterType_eISB},
   {"TM_LO2_PLL_CLK_CH0", BCHP_TM_LO2_PLL_CLK_CH0, SATFE_RegisterType_eISB},
   {"TM_LO2_PLL_CLK_CH1", BCHP_TM_LO2_PLL_CLK_CH1, SATFE_RegisterType_eISB},
   {"TM_LO2_PLL_CLK_CH2", BCHP_TM_LO2_PLL_CLK_CH2, SATFE_RegisterType_eISB},
   {"TM_LO2_PLL_RST", BCHP_TM_LO2_PLL_RST, SATFE_RegisterType_eISB},
   {"TM_LO2_PLL_DCO_CTRL", BCHP_TM_LO2_PLL_DCO_CTRL, SATFE_RegisterType_eISB},
   {"TM_LO2_PLL_GAIN_CTRL", BCHP_TM_LO2_PLL_GAIN_CTRL, SATFE_RegisterType_eISB},
   {"TM_LO2_PLL_MISC_CTRL", BCHP_TM_LO2_PLL_MISC_CTRL, SATFE_RegisterType_eISB},
   {"TM_SYS_PLL_PDIV", BCHP_TM_SYS_PLL_PDIV, SATFE_RegisterType_eISB},
   {"TM_SYS_PLL_NDIV_INT", BCHP_TM_SYS_PLL_NDIV_INT, SATFE_RegisterType_eISB},
   {"TM_SYS_PLL_NDIV_FRAC", BCHP_TM_SYS_PLL_NDIV_FRAC, SATFE_RegisterType_eISB},
   {"TM_SYS_PLL_CLK_108", BCHP_TM_SYS_PLL_CLK_108, SATFE_RegisterType_eISB},
   {"TM_SYS_PLL_CLK_MTSIF_SYS", BCHP_TM_SYS_PLL_CLK_MTSIF_SYS, SATFE_RegisterType_eISB},
   {"TM_SYS_PLL_CLK_MTSIF", BCHP_TM_SYS_PLL_CLK_MTSIF, SATFE_RegisterType_eISB},
   {"TM_SYS_PLL_CLK_TMT", BCHP_TM_SYS_PLL_CLK_TMT, SATFE_RegisterType_eISB},
   {"TM_SYS_PLL_CLK_AFEC", BCHP_TM_SYS_PLL_CLK_AFEC, SATFE_RegisterType_eISB},
   {"TM_SYS_PLL_CLK_TURBO", BCHP_TM_SYS_PLL_CLK_TURBO, SATFE_RegisterType_eISB},
   {"TM_SYS_PLL_RST", BCHP_TM_SYS_PLL_RST, SATFE_RegisterType_eISB},
   {"TM_SYS_PLL_DCO_CTRL", BCHP_TM_SYS_PLL_DCO_CTRL, SATFE_RegisterType_eISB},
   {"TM_SYS_PLL_FB_CTRL", BCHP_TM_SYS_PLL_FB_CTRL, SATFE_RegisterType_eISB},
   {"TM_SYS_PLL_SS_CTRL", BCHP_TM_SYS_PLL_SS_CTRL, SATFE_RegisterType_eISB},
   {"TM_SYS_PLL_SS_LIMIT", BCHP_TM_SYS_PLL_SS_LIMIT, SATFE_RegisterType_eISB},
   {"TM_SYS_PLL_GAIN_CTRL", BCHP_TM_SYS_PLL_GAIN_CTRL, SATFE_RegisterType_eISB},
   {"TM_SYS_PLL_MISC_CTRL", BCHP_TM_SYS_PLL_MISC_CTRL, SATFE_RegisterType_eISB},
   {"TM_OSC_CLK_EN", BCHP_TM_OSC_CLK_EN, SATFE_RegisterType_eISB},
   {"TM_REG_CLK_EN", BCHP_TM_REG_CLK_EN, SATFE_RegisterType_eISB},
   {"TM_SYS_CLK_EN", BCHP_TM_SYS_CLK_EN, SATFE_RegisterType_eISB},
   {"TM_TEST_MODE", BCHP_TM_TEST_MODE, SATFE_RegisterType_eISB},
   {"TM_TEST_MISC2", BCHP_TM_TEST_MISC2, SATFE_RegisterType_eISB},
   {"TM_TEST_MISC1", BCHP_TM_TEST_MISC1, SATFE_RegisterType_eISB},
   {"TM_TEST_MISC0", BCHP_TM_TEST_MISC0, SATFE_RegisterType_eISB},
   {"TM_MTSIF0_CTRL", BCHP_TM_MTSIF0_CTRL, SATFE_RegisterType_eISB},
   {"TM_MTSIF1_CTRL", BCHP_TM_MTSIF1_CTRL, SATFE_RegisterType_eISB},
   {"TM_MTSIF_GPO_EN", BCHP_TM_MTSIF_GPO_EN, SATFE_RegisterType_eISB},
   {"TM_MTSIF_GPO_VAL", BCHP_TM_MTSIF_GPO_VAL, SATFE_RegisterType_eISB},
   {"TM_UART_CTRL", BCHP_TM_UART_CTRL, SATFE_RegisterType_eISB},
   {"TM_GPIO_MUX", BCHP_TM_GPIO_MUX, SATFE_RegisterType_eISB},
   {"TM_GPIO_IODIR", BCHP_TM_GPIO_IODIR, SATFE_RegisterType_eISB},
   {"TM_GPIO_OD", BCHP_TM_GPIO_OD, SATFE_RegisterType_eISB},
   {"TM_GPIO_DATA", BCHP_TM_GPIO_DATA, SATFE_RegisterType_eISB},
   {"TM_GPO_EN", BCHP_TM_GPO_EN, SATFE_RegisterType_eISB},
   {"TM_GPO_OD", BCHP_TM_GPO_OD, SATFE_RegisterType_eISB},
   {"TM_GPO_DATA", BCHP_TM_GPO_DATA, SATFE_RegisterType_eISB},
   {"TM_IRQ_CTRL", BCHP_TM_IRQ_CTRL, SATFE_RegisterType_eISB},
   {"TM_BSC0_CTRL", BCHP_TM_BSC0_CTRL, SATFE_RegisterType_eISB},
   {"TM_BSC1_CTRL", BCHP_TM_BSC1_CTRL, SATFE_RegisterType_eISB},
   {"TM_BAC_CTRL", BCHP_TM_BAC_CTRL, SATFE_RegisterType_eISB},
   {"TM_PAD_DRIVE", BCHP_TM_PAD_DRIVE, SATFE_RegisterType_eISB},
   {"TM_PAD_DRIVE2", BCHP_TM_PAD_DRIVE2, SATFE_RegisterType_eISB},
   {"TM_PAD_INPUT", BCHP_TM_PAD_INPUT, SATFE_RegisterType_eISB},
   {"TM_MISC3", BCHP_TM_MISC3, SATFE_RegisterType_eISB},
   {"TM_MISC2", BCHP_TM_MISC2, SATFE_RegisterType_eISB},
   {"TM_MISC1", BCHP_TM_MISC1, SATFE_RegisterType_eISB},
   {"TM_MISC0", BCHP_TM_MISC0, SATFE_RegisterType_eISB},
   {"TM_SFT7", BCHP_TM_SFT7, SATFE_RegisterType_eISB},
   {"TM_SFT6", BCHP_TM_SFT6, SATFE_RegisterType_eISB},
   {"TM_SFT5", BCHP_TM_SFT5, SATFE_RegisterType_eISB},
   {"TM_SFT4", BCHP_TM_SFT4, SATFE_RegisterType_eISB},
   {"TM_SFT3", BCHP_TM_SFT3, SATFE_RegisterType_eISB},
   {"TM_SFT2", BCHP_TM_SFT2, SATFE_RegisterType_eISB},
   {"TM_SFT1", BCHP_TM_SFT1, SATFE_RegisterType_eISB},
   {"TM_SFT0", BCHP_TM_SFT0, SATFE_RegisterType_eISB},
   {"TM_TP_DIAG_CTRL", BCHP_TM_TP_DIAG_CTRL, SATFE_RegisterType_eISB},
   {"TM_INT_DIAG_MUX_CTRL8", BCHP_TM_INT_DIAG_MUX_CTRL8, SATFE_RegisterType_eISB},
   {"TM_INT_DIAG_MUX_CTRL7", BCHP_TM_INT_DIAG_MUX_CTRL7, SATFE_RegisterType_eISB},
   {"TM_INT_DIAG_MUX_CTRL6", BCHP_TM_INT_DIAG_MUX_CTRL6, SATFE_RegisterType_eISB},
   {"TM_INT_DIAG_MUX_CTRL5", BCHP_TM_INT_DIAG_MUX_CTRL5, SATFE_RegisterType_eISB},
   {"TM_INT_DIAG_MUX_CTRL4", BCHP_TM_INT_DIAG_MUX_CTRL4, SATFE_RegisterType_eISB},
   {"TM_INT_DIAG_MUX_CTRL3", BCHP_TM_INT_DIAG_MUX_CTRL3, SATFE_RegisterType_eISB},
   {"TM_INT_DIAG_MUX_CTRL2", BCHP_TM_INT_DIAG_MUX_CTRL2, SATFE_RegisterType_eISB},
   {"TM_INT_DIAG_MUX_CTRL1", BCHP_TM_INT_DIAG_MUX_CTRL1, SATFE_RegisterType_eISB},
   {"TM_INT_DIAG_MUX_CTRL0", BCHP_TM_INT_DIAG_MUX_CTRL0, SATFE_RegisterType_eISB},
   {"TM_INT_DIAG_INV_CTRL1", BCHP_TM_INT_DIAG_INV_CTRL1, SATFE_RegisterType_eISB},
   {"TM_INT_DIAG_INV_CTRL0", BCHP_TM_INT_DIAG_INV_CTRL0, SATFE_RegisterType_eISB},
   {"TM_EXT_DIAG_MUX_CTRL8", BCHP_TM_EXT_DIAG_MUX_CTRL8, SATFE_RegisterType_eISB},
   {"TM_EXT_DIAG_MUX_CTRL7", BCHP_TM_EXT_DIAG_MUX_CTRL7, SATFE_RegisterType_eISB},
   {"TM_EXT_DIAG_MUX_CTRL6", BCHP_TM_EXT_DIAG_MUX_CTRL6, SATFE_RegisterType_eISB},
   {"TM_EXT_DIAG_MUX_CTRL5", BCHP_TM_EXT_DIAG_MUX_CTRL5, SATFE_RegisterType_eISB},
   {"TM_EXT_DIAG_MUX_CTRL4", BCHP_TM_EXT_DIAG_MUX_CTRL4, SATFE_RegisterType_eISB},
   {"TM_EXT_DIAG_MUX_CTRL3", BCHP_TM_EXT_DIAG_MUX_CTRL3, SATFE_RegisterType_eISB},
   {"TM_EXT_DIAG_MUX_CTRL2", BCHP_TM_EXT_DIAG_MUX_CTRL2, SATFE_RegisterType_eISB},
   {"TM_EXT_DIAG_MUX_CTRL1", BCHP_TM_EXT_DIAG_MUX_CTRL1, SATFE_RegisterType_eISB},
   {"TM_EXT_DIAG_MUX_CTRL0", BCHP_TM_EXT_DIAG_MUX_CTRL0, SATFE_RegisterType_eISB},
   {"TM_EXT_DIAG_INV_CTRL1", BCHP_TM_EXT_DIAG_INV_CTRL1, SATFE_RegisterType_eISB},
   {"TM_EXT_DIAG_INV_CTRL0", BCHP_TM_EXT_DIAG_INV_CTRL0, SATFE_RegisterType_eISB},
   {"TM_EXT_DIAG_PAD_CTRL1", BCHP_TM_EXT_DIAG_PAD_CTRL1, SATFE_RegisterType_eISB},
   {"TM_EXT_DIAG_PAD_CTRL0", BCHP_TM_EXT_DIAG_PAD_CTRL0, SATFE_RegisterType_eISB},
   {"TM_ROSC_CTRL", BCHP_TM_ROSC_CTRL, SATFE_RegisterType_eISB},
   {"TM_ROSC_CTRL2", BCHP_TM_ROSC_CTRL2, SATFE_RegisterType_eISB},
   {"TM_GPIO_READ", BCHP_TM_GPIO_READ, SATFE_RegisterType_eISB},
   {"TM_GPO_READ", BCHP_TM_GPO_READ, SATFE_RegisterType_eISB},
   {"TM_MTSIF_READ", BCHP_TM_MTSIF_READ, SATFE_RegisterType_eISB},
   {"TM_EE_READ", BCHP_TM_EE_READ, SATFE_RegisterType_eISB},
   {"TM_BSC_READ", BCHP_TM_BSC_READ, SATFE_RegisterType_eISB},
   {"TM_EJTAG_READ", BCHP_TM_EJTAG_READ, SATFE_RegisterType_eISB},
   {"TM_MISC_READ", BCHP_TM_MISC_READ, SATFE_RegisterType_eISB},
   {"TM_PIN_STRAP", BCHP_TM_PIN_STRAP, SATFE_RegisterType_eISB},
   {"TM_TP_DIAG_OUT1", BCHP_TM_TP_DIAG_OUT1, SATFE_RegisterType_eISB},
   {"TM_TP_DIAG_OUT0", BCHP_TM_TP_DIAG_OUT0, SATFE_RegisterType_eISB},
   {"TM_TP_IN_EN", BCHP_TM_TP_IN_EN, SATFE_RegisterType_eISB},
   {"TM_TP_IN_VAL", BCHP_TM_TP_IN_VAL, SATFE_RegisterType_eISB},
   {"TM_DAC_TEST_MODE_CTRL", BCHP_TM_DAC_TEST_MODE_CTRL, SATFE_RegisterType_eISB},
   {"TM_TS_MODE_CTRL", BCHP_TM_TS_MODE_CTRL, SATFE_RegisterType_eISB},
   {"TM_EXT_TS_INPUT_CTRL", BCHP_TM_EXT_TS_INPUT_CTRL, SATFE_RegisterType_eISB},
   {"TM_BERT_CTRL", BCHP_TM_BERT_CTRL, SATFE_RegisterType_eISB},
   {"TM_ACQ_DEBUG_CTRL", BCHP_TM_ACQ_DEBUG_CTRL, SATFE_RegisterType_eISB},
   {"TM_ERROR_MON_CTRL", BCHP_TM_ERROR_MON_CTRL, SATFE_RegisterType_eISB},
   {"TM_DSEC_TX_EN", BCHP_TM_DSEC_TX_EN, SATFE_RegisterType_eISB},
   {"TM_DSEC_ACW", BCHP_TM_DSEC_ACW, SATFE_RegisterType_eISB},
   {"TM_TEST_CTRL", BCHP_TM_TEST_CTRL, SATFE_RegisterType_eISB},
   {"TM_WAKEUP_CTRL", BCHP_TM_WAKEUP_CTRL, SATFE_RegisterType_eISB},
   {"TM_CWD_CTRL", BCHP_TM_CWD_CTRL, SATFE_RegisterType_eISB},
   {"TM_PDD_CTRL", BCHP_TM_PDD_CTRL, SATFE_RegisterType_eISB},
   {"TM_EXT_MTSIF_CLK_EN", BCHP_TM_EXT_MTSIF_CLK_EN, SATFE_RegisterType_eISB},
   {"TM_EE_HOLD_EN", BCHP_TM_EE_HOLD_EN, SATFE_RegisterType_eISB},
   {"TM_TP_IN_SRC_SEL", BCHP_TM_TP_IN_SRC_SEL, SATFE_RegisterType_eISB},
   {"TM_CLOCK_MONITOR_CONTROL", BCHP_TM_CLOCK_MONITOR_CONTROL, SATFE_RegisterType_eISB},
   {"TM_CLOCK_MONITOR_MAX_COUNT", BCHP_TM_CLOCK_MONITOR_MAX_COUNT, SATFE_RegisterType_eISB},
   {"TM_CLOCK_MONITOR_REF_COUNTER", BCHP_TM_CLOCK_MONITOR_REF_COUNTER, SATFE_RegisterType_eISB},
   {"TM_CLOCK_MONITOR_REF_DONE", BCHP_TM_CLOCK_MONITOR_REF_DONE, SATFE_RegisterType_eISB},
   {"TM_CLOCK_MONITOR_VIEW_COUNTER", BCHP_TM_CLOCK_MONITOR_VIEW_COUNTER, SATFE_RegisterType_eISB},
   {"TM_CLK_OBSERVATION_CTRL", BCHP_TM_CLK_OBSERVATION_CTRL, SATFE_RegisterType_eISB},
   {"TM_CLK_OBSERVATION_CTRL2", BCHP_TM_CLK_OBSERVATION_CTRL2, SATFE_RegisterType_eISB},
   {"TM_CLK_OBSERVATION_CTRL3", BCHP_TM_CLK_OBSERVATION_CTRL3, SATFE_RegisterType_eISB},
   {"TM_FSK_MISC_CTRL", BCHP_TM_FSK_MISC_CTRL, SATFE_RegisterType_eISB},
   {"TM_MTSIF_ATS_CTRL", BCHP_TM_MTSIF_ATS_CTRL, SATFE_RegisterType_eISB},
   {"TM_ICID_DATA7", BCHP_TM_ICID_DATA7, SATFE_RegisterType_eISB},
   {"TM_ICID_DATA6", BCHP_TM_ICID_DATA6, SATFE_RegisterType_eISB},
   {"TM_ICID_DATA5", BCHP_TM_ICID_DATA5, SATFE_RegisterType_eISB},
   {"TM_ICID_DATA4", BCHP_TM_ICID_DATA4, SATFE_RegisterType_eISB},
   {"TM_ICID_DATA3", BCHP_TM_ICID_DATA3, SATFE_RegisterType_eISB},
   {"TM_ICID_DATA2", BCHP_TM_ICID_DATA2, SATFE_RegisterType_eISB},
   {"TM_ICID_DATA1", BCHP_TM_ICID_DATA1, SATFE_RegisterType_eISB},
   {"TM_ICID_DATA0", BCHP_TM_ICID_DATA0, SATFE_RegisterType_eISB},
   {"TM_ICID_READ", BCHP_TM_ICID_READ, SATFE_RegisterType_eISB},
   {"TM_ICID_CLK_CTRL", BCHP_TM_ICID_CLK_CTRL, SATFE_RegisterType_eISB},
   {"TM_ICID_MISC_CTRL", BCHP_TM_ICID_MISC_CTRL, SATFE_RegisterType_eISB},
   {"TM_SPARE_REG0", BCHP_TM_SPARE_REG0, SATFE_RegisterType_eISB},
#endif
   {NULL, 0, 0}
};


#define Desc_4538_CONFIG_MTSIF_CTL "MTSIF control options\n" \
   "   bits [7:0]: output pipe selection, 0=TX0, 1=TX1; bits 0 to 7 correspond to SDS0 to SDS7\n" \
   "   bit 9: TX0 clock polarity: 0=neg edge, 1=pos edge\n" \
   "   bit 10: TX1 clock polarity: 0=neg edge, 1=pos edge\n" \
   "   bit 11: 0=scrambled data output, 1=unscrambled data output\n" \
   "   bit 12: 1=MTSIF registers will be controlled exclusively by the host (i.e. not programmed by the LEAP)\n" \
   "   bits [31:24]: MTSIF clock divider"

#define Desc_4538_CONFIG_DEBUG_LEVEL "controls firmware UART output debugging level\n" \
   "   bits [2:0]:\n" \
   "      0 = all\n" \
   "      1 = trace\n" \
   "      2 = message\n" \
   "      3 = warn\n" \
   "      4 = error\n" \
   "      5 = suppress all debug messages\n"

#define Desc_4538_CONFIG_DEBUG_MODULE "Enable per-module debug output:\n" \
   "   bit 0: HAB commands\n" \
   "   bits [7:1]: reserved\n"

#define Desc_4538_CONFIG_STUFF_BYTES "Number of null bytes to insert in each frame\n"

#define Desc_4538_CONFIG_TUNER_CTL "Internal tuner control options:\n" \
   "   bit [2:0]: reserved, write 0\n" \
   "   bit 3: (READ ONLY) tuner filter programming: 0=auto, 1=manual\n" \
   "   bit 4: DFT freq estimate: 1=bypass, 0=enabled\n" \
   "   bit 5: preserve commanded tuner LO frequency: 0=disable, 1=enable\n" \
   "   bit 6: retune on reacquire: 0=enable, 1=disable\n" \
   "   bit 7: reserved, write 0"

#define Desc_4538_CONFIG_PLC_CTL "PLC configuration options:\n" \
   "   bit 0: reserved\n" \
   "   bit 1: use PLC tracking bandwidth optimized for AWGN: 0=disable, 1=enable\n" \
   "   bit 2: 0=PLC acquisition bandwidth is determined by the AP, 1=use tracking PLC bandwidth manually specified by plc_alt_acq_bw and plc_alt_acq_damp configuration parameters\n" \
   "   bit 3: 0=PLC tracking bandwidth is determined by the AP, 1=use tracking PLC bandwidth manually specified by plc_alt_trk_bw and plc_alt_trk_damp configuration parameters\n" \
   "   bits [7:4]: reserved"

#define Desc_4538_CONFIG_TURBO_CTL "Turbo acquisition settings:\n" \
   "   bits [7:0]: reserved"

#define Desc_4538_CONFIG_PLC_ALT_ACQ_BW "Alternate acquisition PLC bandwidth in units of Hz.  This bandwidth applies when PLC_CTL bit 2 is set."

#define Desc_4538_CONFIG_PLC_ALT_ACQ_DAMP "Alternate acquisition PLC damping factor scaled 2^3 (e.g. damping factor of 1.0 is 0x08).  This damping factor only applies when PLC_CTL bit 2 is set."

#define Desc_4538_CONFIG_PLC_ALT_TRK_BW "Alternate tracking PLC bandwidth in units of Hz.  This bandwidth applies when PLC_CTL bit 3 is set."

#define Desc_4538_CONFIG_PLC_ALT_TRK_DAMP "Alternate tracking PLC damping factor scaled 2^3 (e.g. damping factor of 1.0 is 0x08).  This damping factor only applies when PLC_CTL bit 3 is set."

#define Desc_4538_CONFIG_BLIND_SCAN_MODES "Indicates which modes will be considered in the blind scan (this configuration parameter applies when blind scan is specified in TUNE_ACQUIRE HAB command).\n" \
   "   bit 0: DVB-S\n" \
   "   bit 1: Turbo\n" \
   "   bit 2: DVB-S2\n" \
   "   bit 3: DTV\n" \
   "   bit 4: DCII"

#define Desc_4538_CONFIG_DTV_SCAN_CODE_RATES "Selects the DTV code rates that are to be considered in the scan:\n" \
   "   bit 0: DTV 1/2\n" \
   "   bit 1: DTV 2/3\n" \
   "   bit 2: DTV 6/7"

#define Desc_4538_CONFIG_DVB_SCAN_CODE_RATES "Selects the DVB-S code rates that are to be considered in the scan:\n" \
   "   bit 0: DVB 1/2\n" \
   "   bit 1: DVB 2/3\n" \
   "   bit 2: DVB 3/4\n" \
   "   bit 3: DVB 5/6\n" \
   "   bit 4: DVB 7/8"

#define Desc_4538_CONFIG_DCII_SCAN_CODE_RATES "Selects the DCII code rates that are to be considered in the scan:\n" \
   "   bit 0: DCII 1/2\n" \
   "   bit 1: DCII 2/3\n" \
   "   bit 2: DCII 3/4\n" \
   "   bit 3: DCII 5/6\n" \
   "   bit 4: DCII 7/8\n" \
   "   bit 5: DCII 5/11\n" \
   "   bit 6: DCII 3/5\n" \
   "   bit 7: DCII 4/5"

#define Desc_4538_CONFIG_TURBO_SCAN_MODES "Selects the Turbo modes that are to be considered in the scan:\n" \
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

#define Desc_4538_CONFIG_STATUS_INDICATOR "Real time status bits:\n" \
   "   bit 0: rain fade\n" \
   "   bit 1: freq drift\n" \
   "   bit 2: SNR threshold 2\n" \
   "   bit 3: SNR threshold 1"

#define Desc_4538_CONFIG_RAIN_FADE_THRESHOLD "SNR drop threshold for the rain fade indicator status in units of 1/8 dB SNR"

#define Desc_4538_CONFIG_RAIN_FADE_WINDOW "Time window for rain fade indicator status in units of 100 msecs"

#define Desc_4538_CONFIG_FREQ_DRIFT_THRESHOLD "Carrier offset threshold in Hz for frequency drift indicator status"

#define Desc_4538_CONFIG_DFT_RANGE_START "Starting bin for DFT engine"

#define Desc_4538_CONFIG_DFT_RANGE_END "Ending bin for DFT engine."

#define Desc_4538_CONFIG_REACQ_CTL "Reacquisition Control options:\n" \
   "   bit 0: reserved, write 0\n" \
   "   bit 1: force reacquisition now\n" \
   "   bit 2: force retune on next reacquisition\n" \
   "   bit 3: dont keep lock if carrier freq drift outside of search range\n" \
   "   bit 4: action taken on acquisition failure when auto reacquisition is disabled: 0=turn off OI, HP, CWC, and FEC, 1=preserve state for debugging"

#define Desc_4538_CONFIG_MISC_CTL "Miscellaneous acquisition control options:\n" \
   "   bit 0: reserved, write 0\n" \
   "   bit 1: verify timing loop is locked\n" \
   "   bit 2: disable DCO notch filters\n" \
   "   all other bits: reserved, write 0\n"

#define Desc_4538_CONFIG_TUNER_FILCAL_UPPER "Upper filter calibration threshold percentage of half-tone scaled by 100\n"

#define Desc_4538_CONFIG_TUNER_FILCAL_LOWER "Lower filter calibration threshold percentage of half-tone scaled by 100\n"

#define Desc_4538_CONFIG_AMBIENT_TEMP "Ambient temperature in Celsius used in tuner Vc adjustment, 0=disable\n"

#define Desc_4538_CONFIG_HAB_MAX_TIME "Longest HAB processing time in usecs\n"

#define Desc_4538_CONFIG_LDPC_CTL "LDPC acquisition settings\n" \
   "   bit 0: auto pilot pll: 0=enable, 1=disable\n" \
   "   bit 1: power saving loop: 0=enable, 1=disable\n" \
   "   bits [6:3]: reserved, write 0\n" \
   "   bit 7: (READ ONLY) 0=normal frame detected, 1=short frame detected\n"

#define Desc_4538_CONFIG_ACM_MAX_MODE "Highest modcod in ACM transmission\n" \
   "   0x20 = NBC QPSK LDPC+BCH code rate 1/2\n" \
   "   0x21 = NBC QPSK LDPC+BCH code rate 3/5\n" \
   "   0x22 = NBC QPSK LDPC+BCH code rate 2/3\n" \
   "   0x23 = NBC QPSK LDPC+BCH code rate 3/4\n" \
   "   0x24 = NBC QPSK LDPC+BCH code rate 4/5\n" \
   "   0x25 = NBC QPSK LDPC+BCH code rate 5/6\n" \
   "   0x26 = NBC QPSK LDPC+BCH code rate 8/9\n" \
   "   0x27 = NBC QPSK LDPC+BCH code rate 9/10\n" \
   "   0x28 = NBC 8PSK LDPC+BCH code rate 3/5\n" \
   "   0x29 = NBC 8PSK LDPC+BCH code rate 2/3\n" \
   "   0x2A = NBC 8PSK LDPC+BCH code rate 3/4\n" \
   "   0x2B = NBC 8PSK LDPC+BCH code rate 5/6\n" \
   "   0x2C = NBC 8PSK LDPC+BCH code rate 8/9\n" \
   "   0x2D = NBC 8PSK LDPC+BCH code rate 9/10\n"

#define Desc_4538_CONFIG_BCM3447_CTL "BCM3447/BCM3448 configuration\n" \
   "   bits [3:0]: RFFE channel 0 output mapping\n" \
   "               0=ADC0, 1=ADC1, 2=ADC2, 3=ADC3, 4=power down\n" \
   "   bits [7:4]: RFFE channel 1 output mapping\n" \
   "               0=ADC0, 1=ADC1, 2=ADC2, 3=ADC3, 4=power down\n" \
   "   bits [11:8]: RFFE channel 2 output mapping\n" \
   "                0=ADC0, 1=ADC1, 2=ADC2, 3=ADC3, 4=power down\n" \
   "   bits [15:12]: RFFE channel 3 output mapping\n" \
   "                 0=ADC0, 1=ADC1, 2=ADC2, 3=ADC3, 4=power down\n" \
   "   bit 30: 0=BCM3447, 1=BCM3448\n" \
   "   bit 31: (READ ONLY) 1=BCM3447 detected\n"

#define Desc_4538_CONFIG_DSEC_PIN_MUX "DISEQC pin mux configuration\n" \
   "   bits [1:0]: TXOUT/TXEN pins used for DISEQC output channel 0\n" \
   "   bits [3:2]: TXOUT/TXEN pins used for DISEQC output channel 1\n" \
   "   bits [5:4]: TXOUT/TXEN pins used for DISEQC output channel 2\n" \
   "   bits [7:6]: TXOUT/TXEN pins used for DISEQC output channel 3\n" \
   "      TXOUT/TXEN pin combinations used in bits [7:0]:\n" \
   "         0 = TXOUT0 / TXEN0\n" \
   "         1 = GPO_8 or GPIO_18 / GPO_7 or GPIO_19\n" \
   "         2 = GPO_6 / GPO_5\n" \
   "         3 = GPO_4 / GPO_3\n" \
   "   bit 8: 0=GPO_8 used for TXOUT, 1=GPIO_18 used for TXOUT\n" \
   "   bit 9: 0=GPO_7 used for TXEN, 1=GPIO_19 used for TXEN\n"

SATFE_ConfigParam SATFE_4538_ConfigParamMap[] =
{
   {"mtsif_ctl", BAST_4538_CONFIG_MTSIF_CTL, BAST_4538_CONFIG_LEN_MTSIF_CTL, SATFE_ConfigParamType_eDefault, Desc_4538_CONFIG_MTSIF_CTL},
   {"debug_level", BAST_4538_CONFIG_DEBUG_LEVEL, BAST_4538_CONFIG_LEN_DEBUG_LEVEL, SATFE_ConfigParamType_eDefault, Desc_4538_CONFIG_DEBUG_LEVEL},
   {"debug_module", BAST_4538_CONFIG_DEBUG_MODULE, BAST_4538_CONFIG_LEN_DEBUG_MODULE, SATFE_ConfigParamType_eDefault, Desc_4538_CONFIG_DEBUG_MODULE},
   {"hab_max_time", BAST_4538_CONFIG_HAB_MAX_TIME, BAST_4538_CONFIG_LEN_HAB_MAX_TIME, SATFE_ConfigParamType_eReadOnly, Desc_4538_CONFIG_HAB_MAX_TIME},
   {"hab_max_time_cmd", BAST_4538_CONFIG_HAB_MAX_TIME_CMD, BAST_4538_CONFIG_LEN_HAB_MAX_TIME_CMD, SATFE_ConfigParamType_eReadOnly, NULL},
   {"xtal_freq", BAST_4538_CONFIG_XTAL_FREQ, BAST_4538_CONFIG_LEN_XTAL_FREQ, SATFE_ConfigParamType_eReadOnly, NULL},
   {"misc_ctl", BAST_4538_CONFIG_MISC_CTL, BAST_4538_CONFIG_LEN_MISC_CTL, SATFE_ConfigParamType_eDefault, Desc_4538_CONFIG_MISC_CTL},
   {"bcm3447_ctl", BAST_4538_CONFIG_BCM3447_CTL, BAST_4538_CONFIG_LEN_BCM3447_CTL, SATFE_ConfigParamType_eDefault, Desc_4538_CONFIG_BCM3447_CTL},
   {"otp_disable_input", BAST_4538_CONFIG_OTP_DISABLE_INPUT, BAST_4538_CONFIG_LEN_OTP_DISABLE_INPUT, SATFE_ConfigParamType_eReadOnly, NULL},
   {"otp_disable_chan", BAST_4538_CONFIG_OTP_DISABLE_CHAN, BAST_4538_CONFIG_LEN_OTP_DISABLE_CHAN, SATFE_ConfigParamType_eReadOnly, NULL},
   {"otp_disable_feature", BAST_4538_CONFIG_OTP_DISABLE_FEATURE, BAST_4538_CONFIG_LEN_OTP_DISABLE_FEATURE, SATFE_ConfigParamType_eReadOnly, NULL},
   {"flash_sector_buf_addr", BAST_4538_CONFIG_FLASH_SECTOR_BUF_ADDR, BAST_4538_CONFIG_LEN_FLASH_SECTOR_BUF_ADDR, SATFE_ConfigParamType_eReadOnly, NULL},
   {"dsec_pin_mux", BAST_4538_CONFIG_DSEC_PIN_MUX, BAST_4538_CONFIG_LEN_DSEC_PIN_MUX, SATFE_ConfigParamType_eDefault, Desc_4538_CONFIG_DSEC_PIN_MUX},
#if 0
   {"status_indicator", BAST_4538_CONFIG_STATUS_INDICATOR, BAST_4538_CONFIG_LEN_STATUS_INDICATOR, SATFE_ConfigParamType_eReadOnly, Desc_4538_CONFIG_STATUS_INDICATOR},
   {"rain_fade_threshold", BAST_4538_CONFIG_RAIN_FADE_THRESHOLD, BAST_4538_CONFIG_LEN_RAIN_FADE_THRESHOLD, SATFE_ConfigParamType_eDefault, Desc_4538_CONFIG_RAIN_FADE_THRESHOLD},
   {"rain_fade_window", BAST_4538_CONFIG_RAIN_FADE_WINDOW, BAST_4538_CONFIG_LEN_RAIN_FADE_WINDOW, SATFE_ConfigParamType_eDefault, Desc_4538_CONFIG_RAIN_FADE_WINDOW},
   {"freq_drift_threshold", BAST_4538_CONFIG_FREQ_DRIFT_THRESHOLD, BAST_4538_CONFIG_LEN_FREQ_DRIFT_THRESHOLD, SATFE_ConfigParamType_eDefault, Desc_4538_CONFIG_FREQ_DRIFT_THRESHOLD},
#endif
   {"stuff_bytes", BAST_4538_CONFIG_STUFF_BYTES, BAST_4538_CONFIG_LEN_STUFF_BYTES, SATFE_ConfigParamType_eDefault, Desc_4538_CONFIG_STUFF_BYTES},
   {"acq_time", BAST_4538_CONFIG_ACQ_TIME, BAST_4538_CONFIG_LEN_ACQ_TIME, SATFE_ConfigParamType_eReadOnly, NULL},
   {"tuner_ctl", BAST_4538_CONFIG_TUNER_CTL, BAST_4538_CONFIG_LEN_TUNER_CTL, SATFE_ConfigParamType_eDefault, Desc_4538_CONFIG_TUNER_CTL},
   {"plc_ctl", BAST_4538_CONFIG_PLC_CTL, BAST_4538_CONFIG_LEN_PLC_CTL, SATFE_ConfigParamType_eDefault, Desc_4538_CONFIG_PLC_CTL},
   {"reacq_ctl", BAST_4538_CONFIG_REACQ_CTL, BAST_4538_CONFIG_LEN_REACQ_CTL, SATFE_ConfigParamType_eDefault, Desc_4538_CONFIG_REACQ_CTL},
   {"ldpc_ctl", BAST_4538_CONFIG_LDPC_CTL, BAST_4538_CONFIG_LEN_LDPC_CTL, SATFE_ConfigParamType_eDefault, Desc_4538_CONFIG_LDPC_CTL},
   {"turbo_ctl", BAST_4538_CONFIG_TURBO_CTL, BAST_4538_CONFIG_LEN_TURBO_CTL, SATFE_ConfigParamType_eDefault, Desc_4538_CONFIG_TURBO_CTL},
   {"plc_alt_acq_bw", BAST_4538_CONFIG_PLC_ALT_ACQ_BW, BAST_4538_CONFIG_LEN_PLC_ALT_ACQ_BW, SATFE_ConfigParamType_eDefault, Desc_4538_CONFIG_PLC_ALT_ACQ_BW},
   {"plc_alt_acq_damp", BAST_4538_CONFIG_PLC_ALT_ACQ_DAMP, BAST_4538_CONFIG_LEN_PLC_ALT_ACQ_DAMP, SATFE_ConfigParamType_eDefault, Desc_4538_CONFIG_PLC_ALT_ACQ_DAMP},
   {"plc_alt_trk_bw", BAST_4538_CONFIG_PLC_ALT_TRK_BW, BAST_4538_CONFIG_LEN_PLC_ALT_TRK_BW, SATFE_ConfigParamType_eDefault, Desc_4538_CONFIG_PLC_ALT_TRK_BW},
   {"plc_alt_trk_damp", BAST_4538_CONFIG_PLC_ALT_TRK_DAMP, BAST_4538_CONFIG_LEN_PLC_ALT_TRK_DAMP, SATFE_ConfigParamType_eDefault, Desc_4538_CONFIG_PLC_ALT_TRK_DAMP},
   {"blind_scan_modes", BAST_4538_CONFIG_BLIND_SCAN_MODES, BAST_4538_CONFIG_LEN_BLIND_SCAN_MODES, SATFE_ConfigParamType_eDefault, Desc_4538_CONFIG_BLIND_SCAN_MODES},
   {"dtv_scan_code_rates", BAST_4538_CONFIG_DTV_SCAN_CODE_RATES, BAST_4538_CONFIG_LEN_DTV_SCAN_CODE_RATES, SATFE_ConfigParamType_eDefault, Desc_4538_CONFIG_DTV_SCAN_CODE_RATES},
   {"dvb_scan_code_rates", BAST_4538_CONFIG_DVB_SCAN_CODE_RATES, BAST_4538_CONFIG_LEN_DVB_SCAN_CODE_RATES, SATFE_ConfigParamType_eDefault, Desc_4538_CONFIG_DVB_SCAN_CODE_RATES},
   {"dcii_scan_code_rates", BAST_4538_CONFIG_DCII_SCAN_CODE_RATES, BAST_4538_CONFIG_LEN_DCII_SCAN_CODE_RATES, SATFE_ConfigParamType_eDefault, Desc_4538_CONFIG_DCII_SCAN_CODE_RATES},
   {"turbo_scan_modes", BAST_4538_CONFIG_TURBO_SCAN_MODES, BAST_4538_CONFIG_LEN_TURBO_SCAN_MODES, SATFE_ConfigParamType_eDefault, Desc_4538_CONFIG_TURBO_SCAN_MODES},
   {"freq_estimate_status", BAST_4538_CONFIG_FREQ_ESTIMATE_STATUS, BAST_4538_CONFIG_LEN_FREQ_ESTIMATE_STATUS, SATFE_ConfigParamType_eDefault, NULL},
   {"if_step_save", BAST_4538_CONFIG_IF_STEP_SAVE, BAST_4538_CONFIG_LEN_IF_STEP_SAVE, SATFE_ConfigParamType_eReadOnly, NULL},
   {"dft_range_start", BAST_4538_CONFIG_DFT_RANGE_START, BAST_4538_CONFIG_LEN_DFT_RANGE_START, SATFE_ConfigParamType_eDefault, Desc_4538_CONFIG_DFT_RANGE_START},
   {"dft_range_end",  BAST_4538_CONFIG_DFT_RANGE_END, BAST_4538_CONFIG_LEN_DFT_RANGE_END, SATFE_ConfigParamType_eDefault, Desc_4538_CONFIG_DFT_RANGE_END},
   {"debug1", BAST_4538_CONFIG_DEBUG1, BAST_4538_CONFIG_LEN_DEBUG1, SATFE_ConfigParamType_eDefault, NULL},
   {"debug2", BAST_4538_CONFIG_DEBUG2, BAST_4538_CONFIG_LEN_DEBUG2, SATFE_ConfigParamType_eDefault, NULL},
   {"debug3", BAST_4538_CONFIG_DEBUG3, BAST_4538_CONFIG_LEN_DEBUG2, SATFE_ConfigParamType_eDefault, NULL},
   {"acm_max_mode", BAST_4538_CONFIG_ACM_MAX_MODE, BAST_4538_CONFIG_LEN_ACM_MAX_MODE, SATFE_ConfigParamType_eDefault, Desc_4538_CONFIG_ACM_MAX_MODE},
   {"reacq_cause", BAST_4538_CONFIG_REACQ_CAUSE, BAST_4538_CONFIG_LEN_REACQ_CAUSE, SATFE_ConfigParamType_eDefault, NULL},
#ifndef SATFE_USE_BFSK
#ifdef BFSK_PROTOCOL_ECHO
   {"fsk_tx_power", BAST_4538_CONFIG_FSK_TX_POWER, BAST_4538_CONFIG_LEN_FSK_TX_POWER, SATFE_ConfigParamType_eDefault, NULL},
   {"fsk_ch_select", BAST_4538_CONFIG_FSK_CH_SELECT, BAST_4538_CONFIG_LEN_FSK_CH_SELECT, SATFE_ConfigParamType_eDefault, NULL},
   {"fsk_tx_freq", BAST_4538_CONFIG_FSK_TX_FREQ_HZ, BAST_4538_CONFIG_LEN_FSK_TX_FREQ_HZ, SATFE_ConfigParamType_eDefault, NULL},
   {"fsk_rx_freq", BAST_4538_CONFIG_FSK_RX_FREQ_HZ, BAST_4538_CONFIG_LEN_FSK_RX_FREQ_HZ, SATFE_ConfigParamType_eDefault, NULL},
   {"fsk_tx_dev", BAST_4538_CONFIG_FSK_TX_DEV_HZ, BAST_4538_CONFIG_LEN_FSK_TX_DEV_HZ, SATFE_ConfigParamType_eDefault, NULL},
#else
   {"ftm_tx_power", BAST_4538_CONFIG_FTM_TX_POWER, BAST_4538_CONFIG_LEN_FTM_TX_POWER, SATFE_ConfigParamType_eDefault, NULL},
   {"ftm_ch_select", BAST_4538_CONFIG_FTM_CH_SELECT, BAST_4538_CONFIG_LEN_FTM_CH_SELECT, SATFE_ConfigParamType_eDefault, NULL},
#endif
#endif
   {NULL, 0, 0, 0, NULL}
};


/******************************************************************************
 SATFE_4538_InitHandle()
******************************************************************************/
void SATFE_4538_InitHandle(SATFE_Chip *pChip)
{
   pChip->regMap = SATFE_4538_RegisterMap;
   pChip->configParamMap = SATFE_4538_ConfigParamMap;
   pChip->pChipCommandSet = SATFE_4538_CommandSet;
   pChip->chipFunctTable = &SATFE_4538_Functs;
}


/******************************************************************************
 SATFE_4538_GetFreqFromString()
******************************************************************************/
bool SATFE_4538_GetFreqFromString(char *str, uint32_t *pHz)
{
   float f;
   uint32_t hz;

   f = (float)atof(str);
   if (f <= 3000)
      hz = (uint32_t)(f * 1000000);
   else
      hz = (uint32_t)f;

   if ((hz < 250000000UL) || (hz > 3000000000UL))
   {
      printf("Tuning frequency out of range\n");
      return false;
   }
   *pHz = hz;
   return true;
}


/******************************************************************************
 SATFE_4538_Command_help()
******************************************************************************/
bool SATFE_4538_Command_help(SATFE_Chip *pChip, int argc, char **argv)
{
   BSTD_UNUSED(pChip);
   BSTD_UNUSED(argc);
   BSTD_UNUSED(argv);

   printf("CHIP-SPECIFIC COMMANDS:\n");
   printf("   test_i2c, print, checkmem, debug_test, avs, memread, memwrite,\n");
   printf("   os_status, wakeup_packet, gpo_write, bcm3447_write, bcm3447_read,\n");
   printf("   mtsif, bert_interface\n");
   printf("\n");
   return true;
}


/******************************************************************************
 SATFE_4538_Command_print()
******************************************************************************/
bool SATFE_4538_Command_print(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   int i;
   uint32_t s, j, n;
   char buf[128];

   if (argc < 2)
   {
      SATFE_PrintDescription1("print", "print [string]", "Causes AP to transmit specified characters from BCM4538 UART.", "string = character string", true);
      return true;
   }

   n = 0;
   for (i = 1; (i < argc) && (n < 124); i++)
   {
      s = strlen(argv[i]);
      for (j = 0; (j < s) && (n < 124); j++)
         buf[n++] = argv[i][j];
      buf[n++] = ' ';
   }
   buf[n] = 0;
   SATFE_MUTEX(retCode = BAST_4538_PrintUart(pChip->hAst, buf));
   SATFE_RETURN_ERROR("BAST_4538_PrintUart()", retCode);
}


/******************************************************************************
 SATFE_4538_Command_checkmem()
******************************************************************************/
bool SATFE_4538_Command_checkmem(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   uint8_t *pBuf = NULL;
   uint32_t start_addr, end_addr, n;
   uint8_t crc8;

   if (argc != 4)
   {
      SATFE_PrintDescription1("checkmem", "checkmem [start_addr] [end_addr]", "Calculates the CRC8 of a memory range.", "start_addr = starting memory address in hexadecimal", false);
      SATFE_PrintDescription2("end_addr = ending memory address in hexadecimal", true);
      return true;
   }

   start_addr = (uint32_t)strtoul(argv[1], NULL, 16);
   end_addr = (uint32_t)strtoul(argv[2], NULL, 16);
   if (start_addr >= end_addr)
   {
      printf("invalid address range\n");
      return false;
   }
   n = end_addr - start_addr + 1;
   pBuf = BKNI_Malloc(n);
   BDBG_ASSERT(pBuf);
   SATFE_MUTEX(retCode = BHAB_ReadMemory((BHAB_Handle)pChip->pHab, start_addr, pBuf, n));
   if (retCode)
      printf("ReadMemory() error 0x%X\n", retCode);
   else
   {
      crc8 = SATFE_GetStreamCrc8(pBuf, (int)n);
      printf("CRC8 = 0x%02X\n", crc8);
   }

   BKNI_Free(pBuf);
   return true;
}


/******************************************************************************
 SATFE_4538_IsSpinv()
******************************************************************************/
bool SATFE_4538_IsSpinv(SATFE_Chip *pChip, BAST_ChannelStatus *pStatus)
{
   BSTD_UNUSED(pChip);

   return (pStatus->modeStatus.turbo.hpstatus & 0x00000080) ? true : false;
}


/******************************************************************************
 SATFE_4538_IsPilotOn()
******************************************************************************/
bool SATFE_4538_IsPilotOn(SATFE_Chip *pChip, BAST_ChannelStatus *pStatus)
{
   BSTD_UNUSED(pChip);

   return (pStatus->modeStatus.turbo.hpstatus & 1) ? true : false;
}


/******************************************************************************
 SATFE_4538_GetDebugModuleNames()
******************************************************************************/
void SATFE_4538_GetDebugModuleNames(struct SATFE_Chip *pChip, int *numModules, char **moduleNames[])
{
   BSTD_UNUSED(pChip);

   *moduleNames = SATFE_4538_DebugModules;
   *numModules = SATFE_4538_NumDebugModules;
}


/******************************************************************************
 SATFE_4538_GetAdcVoltage()
******************************************************************************/
bool SATFE_4538_GetAdcVoltage(struct SATFE_Chip *pChip, uint8_t voltage, uint8_t currChannel, uint16_t *lnbVoltage)
{
   /* mapping for adc to lnb voltage */
   uint16_t bcm73xx_adc_voltage[256][2] = {
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
   *lnbVoltage=bcm73xx_adc_voltage[voltage][currChannel];
   if(*lnbVoltage>0)
      return true;
   else
      return false;
}


/******************************************************************************
 SATFE_4538_Command_read()
******************************************************************************/
bool SATFE_4538_Command_read(struct SATFE_Chip *pChip, int argc, char **argv)
{
   SATFE_Register *pReg;
   BERR_Code retCode;
   uint32_t uval32;
   int i;
   uint8_t uval8;

   BDBG_ASSERT(pChip);

   if (argc < 2)
   {
      SATFE_PrintDescription1("read", "read [reg...]", "Read 1 or more registers.", "reg = register name or list of register names to read", true);
      return true;
   }

   for (i = 1; i < argc; i++)
   {
      if (SATFE_LookupRegister(pChip, argv[i], &pReg))
      {
         if (pReg->type == SATFE_RegisterType_eISB)
         {
            SATFE_MUTEX(retCode = BAST_ReadRegister(pChip->hAstChannel[pChip->currChannel], pReg->addr, &uval32));
            if (retCode == BERR_SUCCESS)
               printf("%s = 0x%08X (%u)\n", argv[i], uval32, uval32);
            else
               goto unable_to_read;
         }
         else if (pReg->type == SATFE_RegisterType_eHost)
         {
            retCode = BHAB_4538_P_ReadBbsi((BHAB_Handle)pChip->pHab, (uint8_t)pReg->addr, &uval8, 1);
            if (retCode == BERR_SUCCESS)
               printf("%s = 0x%02X (%u)\n", argv[i], uval8, uval8);
            else
               goto unable_to_read;
         }
         else
         {
            unable_to_read:
            printf("unable to read %s\n", argv[i]);
         }
      }
      else
         printf("invalid register: %s\n", argv[i]);
   }

   return true;
}


/******************************************************************************
 SATFE_4538_Command_write()
******************************************************************************/
bool SATFE_4538_Command_write(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   SATFE_Register *pReg;
   uint32_t val32;
   uint8_t val8;

   BDBG_ASSERT(pChip);

   if (argc != 3)
   {
      SATFE_PrintDescription1("write", "write [reg] [val]", "Write to a register.", "reg = register name", false);
      SATFE_PrintDescription2("val = Register value.  Hexadecimal values must have '0x' prefix.", true);
      return true;
   }

   if (SATFE_LookupRegister(pChip, argv[1], &pReg))
   {
      if (SATFE_GetU32FromString(argv[2], &val32) == false)
         return false;

      if (pReg->type == SATFE_RegisterType_eISB)
      {
         SATFE_MUTEX(retCode = BAST_WriteRegister(pChip->hAstChannel[pChip->currChannel], pReg->addr, &val32));
         if (retCode != BERR_SUCCESS)
            goto unable_to_write;
      }
      else
      {
         if (SATFE_GetU8FromString(argv[2], &val8) == false)
            return false;

         if (pReg->type == SATFE_RegisterType_eHost)
         {
            retCode = BHAB_4538_P_WriteBbsi((BHAB_Handle)pChip->pHab, (uint8_t)pReg->addr, &val8, 1);
            if (retCode != BERR_SUCCESS)
               goto unable_to_write;
         }
         else
         {
            unable_to_write:
            printf("unable to write %s\n", argv[1]);
         }
      }
   }
   else
      printf("invalid register: %s\n", argv[1]);

   return true;
}


/******************************************************************************
 SATFE_4538_Command_test_i2c()
******************************************************************************/
bool SATFE_4538_Command_test_i2c(SATFE_Chip *pChip, int argc, char **argv)
{
#if 0
   /* test iram */
   #define MEM_START 0
   #define MEM_END 0x30000
#else
   /* test dram */
   #define MEM_START 0x40000
   #define MEM_END 0x60000
#endif
   static uint8_t pattern[20] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F, 0x00, 0xFF, 0x55, 0xAA};
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t exp_pattern, read_pattern, errors, pattern_idx;
   int p, n;
   uint32_t i;

   if ((argc == 0) || (argc > 2))
   {
      SATFE_PrintDescription1("test_i2c", "test_i2c <n>", "Test the I2C link.",  "n = Number of times to execute i2c test.", true);
      return true;
   }

   if (argc == 1)
      n = 20;
   else
      n = atoi(argv[1]);

   printf("stopping the AP...\n");
   BKNI_AcquireMutex(pChip->hMutex);
   BHAB_4538_P_Reset((BHAB_Handle)pChip->pHab);

   for (p = 0; (SATFE_Platform_GetChar(false) <= 0) && (p < n); p++)
   {
      printf("test %d of %d:\n", p+1, n);

      printf("   writing main memory");
      fflush(stdout);

      pattern_idx = p;
      for (i = MEM_START; i < MEM_END; i += 4)
      {
         exp_pattern = pattern[pattern_idx++ % 20];
         exp_pattern |= (uint32_t)((pattern[pattern_idx++ % 20]) << 8);
         exp_pattern |= (uint32_t)((pattern[pattern_idx++ % 20]) << 16);
         exp_pattern |= (uint32_t)((pattern[pattern_idx++ % 20]) << 24);

         retCode = BHAB_4538_P_WriteRbus((BHAB_Handle)pChip->pHab, i, &exp_pattern, 1);
         if (retCode != BERR_SUCCESS)
         {
            printf("\nBHAB_4538_WriteRbus() error 0x%X\n", retCode);
            goto done;
         }
         if ((i % 0x1000) == 0)
         {
            printf("\n0x%08X", i);
            fflush(stdout);
         }
         else if ((i % 0x100) == 0)
         {
            printf(".");
            fflush(stdout);
         }
         if (SATFE_Platform_GetChar(false) > 0)
            goto done;
      }
      printf("OK\n");

      printf("   reading main memory");
      fflush(stdout);

      pattern_idx = p;
      errors = 0;
      for (i = MEM_START; i < MEM_END; i+=4)
      {
         exp_pattern = pattern[pattern_idx++ % 20];
         exp_pattern |= (uint32_t)((pattern[pattern_idx++ % 20]) << 8);
         exp_pattern |= (uint32_t)((pattern[pattern_idx++ % 20]) << 16);
         exp_pattern |= (uint32_t)((pattern[pattern_idx++ % 20]) << 24);

         retCode = BHAB_4538_P_ReadRbus((BHAB_Handle)pChip->pHab, i, &read_pattern, 1);
         if (retCode != BERR_SUCCESS)
         {
            printf("\nBHAB_4538_ReadRbus() error 0x%X\n", retCode);
            goto done;
         }

         if (read_pattern != exp_pattern)
         {
            printf("\nerror in address 0x%08X: read 0x%08X, expected 0x%08X\n", i, read_pattern, exp_pattern);
            errors++;
            if (errors > 10)
            {
               printf("aborting test due to errors\n");
               goto done;
            }
         }

         if ((i % 0x100) == 0)
         {
            printf(".");
            fflush(stdout);
         }
         if (SATFE_Platform_GetChar(false) > 0)
            goto done;
      }
      if (errors == 0)
         printf("OK\n");
      else
         printf("%d errors\n", errors);
   }

   done:
   printf("test done.  AP is still in reset state\n");
   BKNI_ReleaseMutex(pChip->hMutex);
   return true;
}


/******************************************************************************
 SATFE_4538_Command_debug_test()
******************************************************************************/
bool SATFE_4538_Command_debug_test(SATFE_Chip *pChip, int argc, char **argv)
{
   if (argc != 1)
   {
      SATFE_PrintDescription1("debug_test", "debug_test", "Runs a debug test command on the AP.", "none", true);
      return true;
   }

   /* TBD */
   return true;
}


/******************************************************************************
 SATFE_4538_Command_os_status()
******************************************************************************/
bool SATFE_4538_Command_os_status(SATFE_Chip *pChip, int argc, char **argv)
{
   static char* TaskName[6] = {"PRINT", "HIRQ", "HAB", "AVS", "ACQ", "FLASH"};
   BERR_Code retCode;
   uint32_t osfree, osused, top, idx, t;
   uint8_t buf[128], i;

   if (argc != 1)
   {
      SATFE_PrintDescription1("os_status", "os_status", "Returns OS status information.", "none", true);
      return true;
   }

   buf[0] = 0x04;
   SATFE_MUTEX(retCode = BHAB_SendHabCommand((BHAB_Handle)pChip->pHab, buf, 95, buf, 95, true, true, 95));

   printf("OS Version %d.%d\n", buf[1], buf[2]);
   printf("CPU Usage = %d %%\n",buf[81]);
   printf("Tick Count = %u\n", (uint32_t)((buf[3] << 24) | (buf[4] << 16) | (buf[5] << 8) | buf[6]));
   printf("Number of events  = %d\n", buf[79]);
   printf("Number of flags   = %d\n", buf[80]);
   t = (uint32_t)((buf[82] << 24) | (buf[83] << 16) | (buf[84] << 8) | buf[85]);
   t = t / (54*2);
   printf("longest irq time  = %u usec\n", t);
   printf("longest irq func  = 0x%02X%02X%02X%02X\n", buf[86], buf[87], buf[88], buf[89]);
   printf("longest irq func2 = 0x%02X%02X%02X%02X\n\n", buf[90], buf[91], buf[92], buf[93]);

   for (i = 0; i < 6; i++)
   {
      idx = 7 + (i * 12);
      osfree = (uint32_t)((buf[idx] << 24) | (buf[idx+1] << 16) | (buf[idx+2] << 8) | buf[idx+3]);
      idx += 4;
      osused = (uint32_t)((buf[idx] << 24) | (buf[idx+1] << 16) | (buf[idx+2] << 8) | buf[idx+3]);
      idx += 4;
      top = (uint32_t)((buf[idx] << 24) | (buf[idx+1] << 16) | (buf[idx+2] << 8) | buf[idx+3]);
      printf("%s Task:\n", TaskName[i]);
      printf("   free = %d\n", osfree);
      printf("   used = %d\n", osused);
      printf("   top  = %d\n", top);
   }
   return true;
}


/******************************************************************************
 SATFE_4538_Command_wakeup_packet()
******************************************************************************/
bool SATFE_4538_Command_wakeup_packet(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   bool bEnable;
   uint8_t gpio, input_band, i, n, buf[128];

   if (argc < 2)
   {
      SATFE_PrintDescription1("wakeup_packet", "wakeup_packet [enable | disable] <[gpio] [input_band] [pattern_hex ...]>", "Enable/disable wakeup packet detection.", "enable = enables wakeup packet detection", false);
      SATFE_PrintDescription2("disable = disables wakeup packet detection", false);
      SATFE_PrintDescription2("gpio = selects the output gpio to assert on detecting wakeup packet (0 to 15)", false);
      SATFE_PrintDescription2("input_band = selects the transport input band (0 to 15)", false);
      SATFE_PrintDescription2("pattern_hex = pattern consisting of 1 or more hex bytes to match", true);
      return true;
   }

   if (!strcmp(argv[1], "disable"))
      bEnable = false;
   else if (!strcmp(argv[1], "enable"))
      bEnable = true;
   else
   {
      syntax_error:
      printf("syntax error\n");
      return false;
   }

   if ((!bEnable && (argc > 2)) || (bEnable && (argc < 5)))
      goto syntax_error;

   buf[0] = 0x3B;

   if (bEnable)
   {
      gpio = (uint8_t)atoi(argv[2]);
      input_band = (uint8_t)atoi(argv[3]);
      if ((gpio > 15) || (input_band > 15))
      {
         printf("parameter out of range\n");
         return false;
      }
      buf[1] = ((input_band & 0x0F) << 4) | 0x01;
      buf[2] = gpio & 0x0F;
      n = argc - 4;
      buf[3] = n;
      for (i = 4; i < argc; i++)
      {
         buf[i] = (uint8_t)strtoul(argv[i], NULL, 16);
      }
   }
   else
   {
      buf[1] = buf[2] = buf[3] = n = 0x00;
   }
   n += 5;
   SATFE_MUTEX(retCode = BHAB_SendHabCommand((BHAB_Handle)pChip->pHab, buf, n, buf, n, true, true, n));
   SATFE_RETURN_ERROR("SATFE_4538_Command_wakeup_packet()", retCode);
}


/******************************************************************************
 SATFE_4538_Command_avs()
******************************************************************************/
bool SATFE_4538_Command_avs(SATFE_Chip *pChip, int argc, char **argv)
{
   BHAB_AvsData status;
   BERR_Code retCode;

   if ((argc != 1) && (argc != 2))
   {
      SATFE_PrintDescription1("avs", "avs <enable | disable>", "Returns AVS information, or enables/disables AVS", "enable = enable AVS", false);
      SATFE_PrintDescription2("disable = disable AVS", true);
      return true;
   }

   if (argc == 2)
   {
      if (!strcmp(argv[1], "enable"))
      {
         SATFE_MUTEX(retCode = BAST_4538_EnableAvs(pChip->hAst, true));
      }
      else if (!strcmp(argv[1], "disable"))
      {
         SATFE_MUTEX(retCode = BAST_4538_EnableAvs(pChip->hAst, false));
      }
      else
      {
         printf("syntax error\n");
         return false;
      }
      SATFE_RETURN_ERROR("BAST_4538_EnableAvs()", retCode);
   }
   else
   {
      SATFE_MUTEX(retCode = BHAB_GetAvsData((BHAB_Handle)pChip->pHab, &status));
      if (retCode == BERR_SUCCESS)
      {
         printf("enable      = %s\n", status.enabled ? "yes" : "no");
         printf("temperature = %.3f deg C\n", (float)status.temperature / 1000.0);
         printf("voltage     = %d mV\n", status.voltage);
      }
      SATFE_RETURN_ERROR("BHAB_GetAvsData()", retCode);
   }
}


/******************************************************************************
 SATFE_4538_Command_memread()
******************************************************************************/
bool SATFE_4538_Command_memread(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t addr, len, i;
   uint8_t buf[256];

   if (argc != 3)
   {
      SATFE_PrintDescription1("memread", "memread [addr] [len]",
                              "Reads memory.", "addr = starting address", false);
      SATFE_PrintDescription2("len = number of bytes to read", true);
      return true;
   }

   if (!strncmp(argv[1], "0x", 2))
      addr = strtoul(argv[1], NULL, 16);
   else
      addr = strtoul(argv[1], NULL, 10);

   if (!strncmp(argv[2], "0x", 2))
      len = strtoul(argv[2], NULL, 16);
   else
      len = strtoul(argv[2], NULL, 10);

   if (len > 256)
      len = 256;

   SATFE_MUTEX(retCode = BHAB_ReadMemory((BHAB_Handle)(pChip->pHab), addr, buf, len));
   for (i = 0; (retCode == BERR_SUCCESS) && (i < len); i++)
      printf("@0x%08X: 0x%08X\n", addr+i, buf[i]);
   SATFE_RETURN_ERROR("SATFE_Command_memread()", retCode);
}


/******************************************************************************
 SATFE_4538_Command_memwrite()
******************************************************************************/
bool SATFE_4538_Command_memwrite(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t addr, len, i;
   uint8_t buf[256], data;

   if (argc < 3)
   {
      SATFE_PrintDescription1("memwrite", "memwrite [addr] [data ...]",
                              "Write to memory.", "addr = starting address", false);
      SATFE_PrintDescription2("data = bytes to write", true);
      return true;
   }

   if (!strncmp(argv[1], "0x", 2))
      addr = strtoul(argv[1], NULL, 16);
   else
      addr = strtoul(argv[1], NULL, 10);
   if (addr & 0x3)
   {
      printf("address is not 4-byte aligned!\n");
      return false;
   }

   len = argc - 2;
   if (len & 0x3)
   {
      printf("address is not 4-byte aligned!\n");
      return false;
   }
   if (len > 256)
      len = 256;

   for (i = 0; i < len; i++)
   {
      if (!strncmp(argv[i+2], "0x", 2))
         data = (uint8_t)strtoul(argv[i+2], NULL, 16);
      else
         data = (uint8_t)strtoul(argv[i+2], NULL, 10);
      buf[i] = data;
   }

   SATFE_MUTEX(retCode = BHAB_WriteMemory((BHAB_Handle)(pChip->pHab), addr, buf, len));
   SATFE_RETURN_ERROR("SATFE_Command_memwrite()", retCode);
}


/******************************************************************************
 SATFE_4538_Mi2cWrite()
******************************************************************************/
bool SATFE_4538_Mi2cWrite(struct SATFE_Chip *pChip, uint8_t channel, uint8_t slave_addr, uint8_t *buf, uint8_t n)
{
   BERR_Code retCode;

   SATFE_MUTEX(retCode = BAST_4538_WriteBsc(pChip->hAst, channel, slave_addr, buf, n));
   SATFE_RETURN_ERROR("BAST_4538_WriteBsc()", retCode);
}


/******************************************************************************
 SATFE_4538_Mi2cWrite()
******************************************************************************/
BERR_Code SATFE_4538_Mi2cRead(struct SATFE_Chip *pChip, uint8_t channel, uint8_t slave_addr, uint8_t *out_buf, uint8_t out_n, uint8_t *in_buf, uint8_t in_n)
{
   BERR_Code retCode;

   SATFE_MUTEX(retCode = BAST_4538_ReadBsc(pChip->hAst, channel, slave_addr, out_buf, out_n, in_buf, in_n));
   SATFE_RETURN_ERROR("BAST_4538_ReadBsc()", retCode);
}


/******************************************************************************
 SATFE_g3_Command_trace()
******************************************************************************/
bool SATFE_4538_Command_trace(SATFE_Chip *pChip, int argc, char **argv)
{
#define BAST_TraceEvent_eMax 12
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

   BERR_Code retCode = BERR_SUCCESS;
   uint32_t buffer[BAST_TraceEvent_eMax];
   int i, idx;
   uint8_t buf[52];

   BSTD_UNUSED(argv);

   if (argc != 1)
   {
      SATFE_PrintDescription1("trace", "trace", "Display the trace buffer.", "none", true);
   }
   else
   {
      buf[0] = 0x3C;
      buf[1] = pChip->currChannel;
      SATFE_MUTEX(retCode = BHAB_SendHabCommand((BHAB_Handle)pChip->pHab, buf, 51, buf, 51, true, true, 51));
      if (retCode == BERR_SUCCESS)
      {
         for (i = 0; i < BAST_TraceEvent_eMax; i++)
         {
            idx = 2 + (i*4);
            buffer[i] = (buf[idx] << 24) | (buf[idx+1] << 16) | (buf[idx+2] << 8) | buf[idx+3];
            idx += 4;
            printf("%16s: %10u %10u\n",
                   SATFE_TRACE_EVENT[i],
                   buffer[i],
                   buffer[i] ? (((i > 0) ? (buffer[i] - buffer[i-1]) : buffer[0])) : 0);
         }
         printf("\n");
      }
   }
   SATFE_RETURN_ERROR("SATFE_4538_Command_trace()", retCode);
}


/******************************************************************************
 bool SATFE_4538_Command_gpo_write()
******************************************************************************/
bool SATFE_4538_Command_gpo_write(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t write_mask, data_mask;

   if (argc != 3)
   {
      printf("------------------------------------------------------------------------------\n");
      printf("PURPOSE:    sets the state of GPO pin(s)\n");
      printf("SYNTAX:     gpo_write [write_mask] [data_mask]\n");
      printf("PARAMETERS: write_mask = specifies which GPO pins to set\n");
      printf("            data_mask = state of the pins to set\n");
      printf("------------------------------------------------------------------------------\n");
      return true;
   }

   write_mask = strtoul(argv[1], NULL, 16);
   data_mask = strtoul(argv[2], NULL, 16);
   SATFE_MUTEX(retCode = BAST_4538_WriteGpo(pChip->hAst, write_mask, data_mask));
   SATFE_RETURN_ERROR("BAST_4538_WriteGpo()", retCode);
}


/******************************************************************************
 SATFE_3447_LookupRegister()
******************************************************************************/
bool SATFE_3447_LookupRegister(char *name, SATFE_Register **pReg)
{
   int i, j;
   char regname[80];

   for (i = 0; SATFE_3447_RegisterMap[i].name; i++)
   {
      strcpy(regname, SATFE_3447_RegisterMap[i].name);
      for (j = 0; j < (int)strlen(regname); j++)
         regname[j] = tolower(regname[j]);
      if (!strcmp(name, regname))
      {
         *pReg = &(SATFE_3447_RegisterMap[i]);
         return true;
      }
   }
   return false;
}


/******************************************************************************
 bool SATFE_4538_Command_bcm3447_write()
******************************************************************************/
bool SATFE_4538_Command_bcm3447_write(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   SATFE_Register *pReg;
   uint32_t reg, data;

   if (argc != 3)
   {
      printf("------------------------------------------------------------------------------\n");
      printf("PURPOSE:    write to a register on BCM3447\n");
      printf("SYNTAX:     bcm3447_write [[reg_hex] | [reg_name]] [data_hex]\n");
      printf("PARAMETERS: reg_hex = register address in hexadecimal\n");
      printf("            data_hex = hexadecimal data to write\n");
      printf("------------------------------------------------------------------------------\n");
      return true;
   }

   if (SATFE_3447_LookupRegister(argv[1], &pReg) == true)
      reg = (uint8_t)pReg->addr;
   else
      reg = strtoul(argv[1], NULL, 16);
   if (reg > 0x64)
   {
      printf("invalid register\n");
      return false;
   }

   data = strtoul(argv[2], NULL, 16);

   SATFE_MUTEX(retCode = BAST_4538_WriteBcm3447Register(pChip->hAst, reg, data));
   SATFE_RETURN_ERROR("BAST_4538_WriteBcm3447Register()", retCode);
}


/******************************************************************************
 bool SATFE_4538_Command_bcm3447_read()
******************************************************************************/
bool SATFE_4538_Command_bcm3447_read(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   SATFE_Register *pReg;
   int i;
   uint32_t val;
   uint8_t reg;

   if (argc != 2)
   {
      printf("------------------------------------------------------------------------------\n");
      printf("PURPOSE:    read a register on BCM3447\n");
      printf("SYNTAX:     bcm3447_read [[reg] | all]\n");
      printf("PARAMETERS: reg = register address in hexadecimal or the register name\n");
      printf("------------------------------------------------------------------------------\n");
      return true;
   }

   if (!strcmp(argv[1], "all"))
   {
      for (i = 0; SATFE_3447_RegisterMap[i].name; i++)
      {
         reg = (uint8_t)SATFE_3447_RegisterMap[i].addr;
         SATFE_MUTEX(retCode = BAST_4538_ReadBcm3447Register(pChip->hAst, reg, &val));
         if (retCode == BERR_SUCCESS)
            printf("%s = 0x%08X\n", SATFE_3447_RegisterMap[i].name, val);
      }
   }
   else
   {
      if (SATFE_3447_LookupRegister(argv[1], &pReg) == true)
         reg = (uint8_t)pReg->addr;
      else
      {
         reg = (uint8_t)(strtoul(argv[1], NULL, 16) & 0xFF);
         if (reg > 0x64)
         {
            invalid_register:
            printf("invalid register\n");
            return false;
         }
         pReg = NULL;
         for (i = 0; SATFE_3447_RegisterMap[i].name; i++)
         {
            if ((uint8_t)(SATFE_3447_RegisterMap[i].addr) == reg)
            {
               pReg = &SATFE_3447_RegisterMap[i];
               break;
            }
         }
         if (pReg == NULL)
            goto invalid_register;
      }

      SATFE_MUTEX(retCode = BAST_4538_ReadBcm3447Register(pChip->hAst, reg, &val));
      if (retCode == BERR_SUCCESS)
         printf("%s = 0x%08X\n", pReg->name, val);
   }
   SATFE_RETURN_ERROR("BAST_4538_ReadBsc()", retCode);
}


/******************************************************************************
 bool SATFE_4538_Command_bert_interface()
******************************************************************************/
bool SATFE_4538_Command_bert_interface(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t i;
   bool bEnable = false, bSerial = true, bClkinv = false;
   uint8_t channel = 0;

   if ((argc < 1) || (argc > 6))
   {
      printf("------------------------------------------------------------------------------\n");
      printf("PURPOSE:    enable/disable/configure the BERT interface\n");
      printf("SYNTAX:     bert_interface [-enable | -disable] <-s | -p> <-i | -n> <-c channel>\n");
      printf("PARAMETERS: -enable = enable BERT output\n");
      printf("            -disable = disable BERT output\n");
      printf("            -s = serial output (default)\n");
      printf("            -p = parallel output\n");
      printf("            -i = BERT clock is inverted\n");
      printf("            -n = BERT clock is normal (default)\n");
      printf("            channel = SDS channel for BERT output (0 to 7, default is 0)\n");
      printf("------------------------------------------------------------------------------\n");
      return true;
   }

   if (argc == 1)
   {
      /* read the current settings */
      SATFE_MUTEX(retCode = BAST_4538_GetBertInterface(pChip->hAst, &bEnable, &bSerial, &bClkinv, &channel));
      if (retCode == BERR_SUCCESS)
      {
         if (bEnable)
         {
            printf("BERT interface is enabled:\n");
            printf("   %s output, ", bSerial ? "serial" : "parallel");
            if (bClkinv)
               printf("BERT_CLK is inverted, ");
            printf("SDS%d is selected for BERT output\n", channel);
         }
         else
            printf("BERT interface is disabled\n");
      }
      SATFE_RETURN_ERROR("BAST_4538_GetBertInterface()", retCode);
   }

   for (i = 1; i < (uint32_t)argc; i++)
   {
      if (!strcmp(argv[i], "-enable"))
         bEnable = true;
      else if (!strcmp(argv[i], "-disable"))
         bEnable = false;
      else if (!strcmp(argv[i], "-s"))
         bSerial = true;
      else if (!strcmp(argv[i], "-p"))
         bSerial = false;
      else if (!strcmp(argv[i], "-i"))
         bClkinv = true;
      else if (!strcmp(argv[i], "-n"))
         bClkinv = false;
      else if (!strcmp(argv[i], "-c") && ((int)i < (argc-1)))
      {
         i++;
         channel = (uint8_t)atoi(argv[i]);
         if (channel > 7)
         {
            printf("invalid channel (%d)\n", channel);
            return false;
         }
      }
      else
      {
         printf("invalid parameter (%s)\n", argv[i]);
         return false;
      }
   }
   SATFE_MUTEX(retCode = BAST_4538_SetBertInterface(pChip->hAst, bEnable, bSerial, bClkinv, channel));
   SATFE_RETURN_ERROR("BAST_4538_SetBertInterface()", retCode);
}

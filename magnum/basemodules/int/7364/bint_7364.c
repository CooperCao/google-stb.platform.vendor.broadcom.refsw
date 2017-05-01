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

#include "bstd.h"
#include "bint_7364.h"

/* Include interrupt definitions from RDB */
#include "bchp_hif_cpu_intr1.h"
#include "bchp_hvd_intr2_0.h"
#include "bchp_leap_host_l1.h"

/* Standard L2 stuff */
#include "bchp_aud_inth.h"

#include "bchp_bsp_control_intr2.h"
#include "bchp_bvnb_intr2.h"
#include "bchp_bvnf_intr2_0.h"
#include "bchp_bvnf_intr2_1.h"
#include "bchp_bvnf_intr2_3.h"
#include "bchp_bvnf_intr2_5.h"
#include "bchp_bvnf_intr2_6.h"
#include "bchp_bvnf_intr2_7.h"
#include "bchp_bvnf_intr2_9.h"
#include "bchp_bvnf_intr2_15.h"
#include "bchp_bvnf_intr2_16.h"
#include "bchp_bvnf_intr2_18.h"
#include "bchp_bvnm_intr2_0.h"
#include "bchp_clkgen_intr2.h"
#if BCHP_VER <= BCHP_VER_B0
#include "bchp_dvp_hr_intr2.h"
#endif
#include "bchp_m2mc_l2.h"
#if BCHP_VER <= BCHP_VER_B0
#include "bchp_hdmi_rx_intr2_0.h"
#endif
#include "bchp_hdmi_tx_intr2.h"
#include "bchp_memc_l2_0_0.h"
#include "bchp_memc_l2_0_1.h"
#include "bchp_memc_l2_0_2.h"
#include "bchp_raaga_dsp_inth.h"
#include "bchp_raaga_dsp_fw_inth.h"
#include "bchp_sun_l2.h"
#include "bchp_aon_l2.h"
#include "bchp_aon_pm_l2.h"

#include "bchp_hdmi_tx_scdc_intr2_0.h"
#include "bchp_hdmi_tx_hae_intr2_0.h"

#include "bchp_upg_aux_intr2.h"
#include "bchp_sid_l2.h"
#include "bchp_upg_aux_aon_intr2.h"
#include "bchp_v3d_dbg.h"
#include "bchp_v3d_ctl.h"
#include "bchp_video_enc_intr2.h"

/* MHL */
#include "bchp_mpm_host_l2.h"
#include "bchp_mpm_pm_l2.h"
#include "bchp_cbus_intr2_0.h"
#include "bchp_cbus_intr2_1.h"

/* XPT */
#include "bchp_xpt_fe.h"
#include "bchp_xpt_bus_if.h"
#include "bchp_xpt_dpcr0.h"
#include "bchp_xpt_dpcr1.h"
#include "bchp_xpt_dpcr2.h"
#include "bchp_xpt_dpcr3.h"
#include "bchp_xpt_dpcr4.h"
#include "bchp_xpt_dpcr5.h"
#include "bchp_xpt_dpcr6.h"
#include "bchp_xpt_dpcr7.h"
#include "bchp_xpt_dpcr8.h"
#include "bchp_xpt_dpcr9.h"
#include "bchp_xpt_dpcr10.h"
#include "bchp_xpt_dpcr11.h"
#include "bchp_xpt_dpcr12.h"
#include "bchp_xpt_dpcr13.h"
#include "bchp_xpt_rave.h"
#include "bchp_xpt_msg.h"

#include "bchp_xpt_msg_buf_dat_rdy_intr_00_31_l2.h"
#include "bchp_xpt_msg_buf_dat_rdy_intr_32_63_l2.h"
#include "bchp_xpt_msg_buf_dat_rdy_intr_64_95_l2.h"
#include "bchp_xpt_msg_buf_dat_rdy_intr_96_127_l2.h"
#include "bchp_xpt_msg_buf_dat_rdy_intr_128_159_l2.h"
#include "bchp_xpt_msg_buf_dat_rdy_intr_160_191_l2.h"
#include "bchp_xpt_msg_buf_dat_rdy_intr_192_223_l2.h"
#include "bchp_xpt_msg_buf_dat_rdy_intr_224_255_l2.h"

#include "bchp_xpt_msg_buf_ovfl_intr_00_31_l2.h"
#include "bchp_xpt_msg_buf_ovfl_intr_32_63_l2.h"
#include "bchp_xpt_msg_buf_ovfl_intr_64_95_l2.h"
#include "bchp_xpt_msg_buf_ovfl_intr_96_127_l2.h"
#include "bchp_xpt_msg_buf_ovfl_intr_128_159_l2.h"
#include "bchp_xpt_msg_buf_ovfl_intr_160_191_l2.h"
#include "bchp_xpt_msg_buf_ovfl_intr_192_223_l2.h"
#include "bchp_xpt_msg_buf_ovfl_intr_224_255_l2.h"

#include "bchp_xpt_msg_dat_err_intr_l2.h"
#include "bchp_xpt_pcroffset.h"
#include "bchp_xpt_wakeup.h"
#include "bchp_xpt_full_pid_parser.h"
#include "bchp_xpt_mcpb_cpu_intr_aggregator.h"
#include "bchp_xpt_mcpb_desc_done_intr_l2.h"
#include "bchp_xpt_mcpb_misc_asf_compressed_data_received_intr_l2.h"

#include "bchp_xpt_mcpb_misc_asf_len_err_intr_l2.h"
#include "bchp_xpt_mcpb_misc_asf_padding_len_err_intr_l2.h"
#include "bchp_xpt_mcpb_misc_asf_protocol_err_intr_l2.h"
#include "bchp_xpt_mcpb_misc_crc_compare_error_intr_l2.h"
#include "bchp_xpt_mcpb_misc_data_tagid_mismatch_intr_l2.h"
#include "bchp_xpt_mcpb_misc_desc_tagid_mismatch_intr_l2.h"
#include "bchp_xpt_mcpb_misc_false_wake_intr_l2.h"
#include "bchp_xpt_mcpb_misc_oos_intr_l2.h"
#include "bchp_xpt_mcpb_misc_pause_after_group_packets_intr_l2.h"
#include "bchp_xpt_mcpb_misc_pause_at_desc_end_intr_l2.h"
#include "bchp_xpt_mcpb_misc_pause_at_desc_read_intr_l2.h"
#include "bchp_xpt_mcpb_misc_pes_next_ts_range_err_intr_l2.h"
#include "bchp_xpt_mcpb_misc_tei_intr_l2.h"
#include "bchp_xpt_mcpb_misc_ts_parity_err_intr_l2.h"
#include "bchp_xpt_mcpb_misc_ts_range_err_intr_l2.h"
#include "bchp_xpt_memdma_mcpb_cpu_intr_aggregator.h"
#include "bchp_xpt_memdma_mcpb_desc_done_intr_l2.h"
#include "bchp_xpt_memdma_mcpb_misc_asf_compressed_data_received_intr_l2.h"

#include "bchp_xpt_memdma_mcpb_misc_asf_len_err_intr_l2.h"
#include "bchp_xpt_memdma_mcpb_misc_asf_padding_len_err_intr_l2.h"
#include "bchp_xpt_memdma_mcpb_misc_asf_protocol_err_intr_l2.h"
#include "bchp_xpt_memdma_mcpb_misc_crc_compare_error_intr_l2.h"
#include "bchp_xpt_memdma_mcpb_misc_data_tagid_mismatch_intr_l2.h"
#include "bchp_xpt_memdma_mcpb_misc_desc_tagid_mismatch_intr_l2.h"
#include "bchp_xpt_memdma_mcpb_misc_false_wake_intr_l2.h"
#include "bchp_xpt_memdma_mcpb_misc_oos_intr_l2.h"
#include "bchp_xpt_memdma_mcpb_misc_pause_after_group_packets_intr_l2.h"
#include "bchp_xpt_memdma_mcpb_misc_pause_at_desc_end_intr_l2.h"
#include "bchp_xpt_memdma_mcpb_misc_pause_at_desc_read_intr_l2.h"
#include "bchp_xpt_memdma_mcpb_misc_pes_next_ts_range_err_intr_l2.h"
#include "bchp_xpt_memdma_mcpb_misc_tei_intr_l2.h"
#include "bchp_xpt_memdma_mcpb_misc_ts_parity_err_intr_l2.h"
#include "bchp_xpt_memdma_mcpb_misc_ts_range_err_intr_l2.h"
#include "bchp_xpt_rave_misc_l2_intr.h"
#include "bchp_xpt_rave_cdb_overflow_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_cdb_overflow_cx32_47_l2_intr.h"
#include "bchp_xpt_rave_itb_overflow_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_itb_overflow_cx32_47_l2_intr.h"
#include "bchp_xpt_rave_emu_error_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_emu_error_cx32_47_l2_intr.h"
#include "bchp_xpt_rave_pusi_error_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_pusi_error_cx32_47_l2_intr.h"
#include "bchp_xpt_rave_tei_error_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_tei_error_cx32_47_l2_intr.h"
#include "bchp_xpt_rave_cc_error_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_cc_error_cx32_47_l2_intr.h"
#include "bchp_xpt_rave_cdb_upper_thresh_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_cdb_upper_thresh_cx32_47_l2_intr.h"
#include "bchp_xpt_rave_itb_upper_thresh_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_itb_upper_thresh_cx32_47_l2_intr.h"
#include "bchp_xpt_rave_cdb_lower_thresh_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_cdb_lower_thresh_cx32_47_l2_intr.h"
#include "bchp_xpt_rave_itb_lower_thresh_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_itb_lower_thresh_cx32_47_l2_intr.h"
#include "bchp_xpt_rave_cdb_min_depth_thresh_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_cdb_min_depth_thresh_cx32_47_l2_intr.h"
#include "bchp_xpt_rave_itb_min_depth_thresh_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_itb_min_depth_thresh_cx32_47_l2_intr.h"

#include "bchp_xpt_rave_tsio_dma_end_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_tsio_dma_end_cx32_47_l2_intr.h"
#include "bchp_xpt_tsio_intr_l2.h"

#include "bchp_xpt_wdma_btp_intr_l2.h"
#include "bchp_xpt_wdma_overflow_intr_l2.h"
#include "bchp_xpt_wdma_desc_done_intr_l2.h"

#if BCHP_VER <= BCHP_VER_B0
#include "bchp_rfm_l2.h"
#endif

#include "bchp_leap_l2.h"
#include "bchp_leap_host_l2.h"
#include "bchp_demod_xpt_fe.h"
#include "bchp_demod_xpt_wakeup.h"

#include "bchp_afec0_intr_0.h"

#include "bchp_afec1_intr_0.h"

#include "bchp_afec_global_intr_0.h"

#include "bchp_aif_wb_sat_core_intr2_0_0.h"

#include "bchp_sds_intr2_0_0.h"
#include "bchp_sds_intr2_0_1.h"


#include "bchp_tfec_intr2_0.h"
#include "bchp_tfec_intr2_1.h"

#include "bchp_sds_dsec_intr2_0.h"
#include "bchp_sds_dsec_intr2_1.h"
#include "bchp_sds_dsec_intr2_2.h"
#include "bchp_sds_dsec_intr2_3.h"

#include "bchp_ftm_intr2.h"

#include "bchp_aif_mdac_cal_sat_core_intr2_0.h"


/* UARTs, keypad, I2C */
#include "bchp_irq0.h"
#include "bchp_irq0_aon.h"

/* Smartcard interrupts. */
#include "bchp_scirq0.h"

/* Timer */
#include "bchp_timer.h"

/* For SAGE interrupts */
#include "bchp_scpu_host_intr2.h"

BDBG_MODULE(interruptinterface_7364);
#define BINT_P_IRQ0_CASES \
    case BCHP_IRQ0_IRQEN:

#define BINT_P_IRQ0_ENABLE      0
#define BINT_P_IRQ0_STATUS      4


#define BINT_P_IRQ0_AON_CASES \
    case BCHP_IRQ0_AON_IRQEN:

#define BINT_P_IRQ0_AON_ENABLE      0
#define BINT_P_IRQ0_AON_STATUS      4


#define BINT_P_XPT_STATUS           0x00
#define BINT_P_XPT_ENABLE           0x04

#define BINT_P_XPT_STATUS_CASES \
    case BCHP_XPT_BUS_IF_INTR_STATUS_REG: \
    case BCHP_XPT_BUS_IF_INTR_STATUS2_REG: \
    case BCHP_XPT_BUS_IF_INTR_STATUS3_REG: \
    case BCHP_XPT_BUS_IF_INTR_STATUS4_REG: \
    case BCHP_XPT_BUS_IF_INTR_STATUS5_REG: \
    case BCHP_XPT_FE_INTR_STATUS0_REG: \
    case BCHP_XPT_FE_INTR_STATUS1_REG: \
    case BCHP_XPT_FE_INTR_STATUS2_REG: \
    case BCHP_XPT_FULL_PID_PARSER_IBP_PCC_INTR_STATUS_REG: \
    case BCHP_XPT_FULL_PID_PARSER_PBP_PCC_INTR_STATUS_REG: \
    case BCHP_XPT_FULL_PID_PARSER_IBP_SCC_INTR_STATUS_REG: \
    case BCHP_XPT_FULL_PID_PARSER_PBP_SCC_INTR_STATUS_REG: \
    case BCHP_XPT_FULL_PID_PARSER_IBP_PSG_PROTOCOL_INTR_STATUS_REG: \
    case BCHP_XPT_FULL_PID_PARSER_PBP_PSG_PROTOCOL_INTR_STATUS_REG: \
    case BCHP_XPT_FULL_PID_PARSER_IBP_TRANSPORT_ERROR_INTR_STATUS_REG: \
    case BCHP_XPT_FULL_PID_PARSER_PBP_TRANSPORT_ERROR_INTR_STATUS_REG: \
    case BCHP_XPT_DPCR0_INTR_STATUS_REG: \
    case BCHP_XPT_DPCR1_INTR_STATUS_REG: \
    case BCHP_XPT_DPCR2_INTR_STATUS_REG: \
    case BCHP_XPT_DPCR3_INTR_STATUS_REG: \
    case BCHP_XPT_DPCR4_INTR_STATUS_REG: \
    case BCHP_XPT_DPCR5_INTR_STATUS_REG: \
    case BCHP_XPT_DPCR6_INTR_STATUS_REG: \
    case BCHP_XPT_DPCR7_INTR_STATUS_REG: \
    case BCHP_XPT_DPCR8_INTR_STATUS_REG: \
    case BCHP_XPT_DPCR9_INTR_STATUS_REG: \
    case BCHP_XPT_DPCR10_INTR_STATUS_REG: \
    case BCHP_XPT_DPCR11_INTR_STATUS_REG: \
    case BCHP_XPT_DPCR12_INTR_STATUS_REG: \
    case BCHP_XPT_DPCR13_INTR_STATUS_REG: \
    case BCHP_XPT_WAKEUP_INTR_STATUS_REG:

#define BINT_P_PCROFFSET_CASES \
    case BCHP_XPT_PCROFFSET_INTERRUPT0_STATUS: \
    case BCHP_XPT_PCROFFSET_INTERRUPT1_STATUS: \
    case BCHP_XPT_PCROFFSET_INTERRUPT2_STATUS: \
    case BCHP_XPT_PCROFFSET_INTERRUPT3_STATUS: \
    case BCHP_XPT_PCROFFSET_STC_INTERRUPT_STATUS:

#define BINT_P_PCROFFSET_STATUS     0x00
#define BINT_P_PCROFFSET_ENABLE     0x04

#define BINT_P_XPT_MSG_ERR_STATUS   ( 0x00 )
#define BINT_P_XPT_MSG_ERR_ENABLE   ( 0x04 )
#define BINT_P_XPT_MSG_ERR_CASES \
    case BCHP_XPT_MSG_DAT_ERR_INTR_L2_CPU_STATUS:


/* BINT_P_UPGSC_ENABLE was defined as -4 for BCHP_SCIRQ0_SCIRQSTAT.
 * Since we are using BCHP_SCIRQ0_SCIRQEN, it is not needed but
 * to minimize the change, it is kept and set to 0
 */


#define BINT_P_UPGSC_ENABLE (0)

#define BINT_P_UPGSC_CASES \
    case BCHP_SCIRQ0_SCIRQEN:

#define BINT_P_TIMER_STATUS     0x00
#define BINT_P_TIMER_MASK       0x04

#define BINT_P_TIMER_CASES \
    case BCHP_TIMER_TIMER_IS:

#define BINT_P_STAT_TIMER_TICKS_PER_USEC 27


static void BINT_P_7364_ClearInt( BREG_Handle regHandle, uint32_t baseAddr, int shift );
static void BINT_P_7364_SetMask( BREG_Handle regHandle, uint32_t baseAddr, int shift );
static void BINT_P_7364_ClearMask( BREG_Handle regHandle, uint32_t baseAddr, int shift );

static uint32_t BINT_P_7364_ReadStatus( BREG_Handle regHandle, uint32_t baseAddr );

#if NEXUS_WEBCPU_core1_server
static const BINT_P_IntMap bint_7364[] =
{
    { BCHP_HIF_CPU_INTR1_INTR_W3_STATUS_M2MC1_CPU_INTR_SHIFT + 96, BCHP_M2MC1_L2_CPU_STATUS, 0, "M2MC1"},
    { -1, 0, 0, NULL}
};
#else
static const BINT_P_IntMap bint_7364[] =
{
    BINT_MAP_STD(0, BSP, BSP_CONTROL_INTR2_CPU),
    /* SCPU_HOST_INTR2_CPU is not mapped */
    /* NMI_PIN_CPU_INTR is not mapped */
    BINT_MAP_STD(0, AIO, AUD_INTH_R5F),
    BINT_MAP_STD(0, BVNB_0, BVNB_INTR2_CPU),
    BINT_MAP_STD(0, BVNF_0, BVNF_INTR2_0_R5F),
    BINT_MAP_STD(0, BVNF_1, BVNF_INTR2_1_R5F),
    BINT_MAP_STD(0, BVNF_5, BVNF_INTR2_5_R5F),
    BINT_MAP_STD(0, BVNF_9, BVNF_INTR2_9_R5F),
    BINT_MAP_STD(0, BVNF_16, BVNF_INTR2_16_R5F),
    BINT_MAP_STD(0, BVNM_0, BVNM_INTR2_0_R5F),
    BINT_MAP_STD(0, CLKGEN, CLKGEN_INTR2_CPU),

    /* AVS_CPU_INTR is not mapped */
     /* GENET*_INTR is not mapped */
    BINT_MAP_STD(0, HDMI_TX, HDMI_TX_INTR2_CPU),
    BINT_MAP_STD(0, HDMI_TX, HDMI_TX_SCDC_INTR2_0_CPU),
    BINT_MAP_STD(0, HDMI_TX, HDMI_TX_HAE_INTR2_0_CPU),
    BINT_MAP_STD(0, GFX, M2MC_L2_CPU),

    /* CBUS interrupts */
    BINT_MAP_STD(0, CBUS, CBUS_INTR2_0_CPU),
    BINT_MAP_STD(0, CBUS, CBUS_INTR2_1_CPU),

#if !NEXUS_WEBCPU
/* in webcpu mode, core0 doesn't get M2MC1 L1 ??? */

#endif

    BINT_MAP_STD(1, MEMC0, MEMC_L2_0_0_CPU),
    BINT_MAP_STD(1, MEMC0, MEMC_L2_0_1_CPU),
    BINT_MAP_STD(1, MEMC0, MEMC_L2_0_2_CPU),

    BINT_MAP_STD(1, RAAGA, RAAGA_DSP_INTH_HOST),
    BINT_MAP_STD(1, RAAGA_FW, RAAGA_DSP_FW_INTH_HOST),
    BINT_MAP_STD(1, SID, SID_L2_CPU),
    BINT_MAP_STD(1, HVD0, HVD_INTR2_0_CPU),
    BINT_MAP_STD(1, SYS_AON, AON_L2_CPU),
    /* Power Management not set up yet for 7364A0
    BINT_MAP_STD(1, SYS_PM, AON_PM_L2_CPU,
    */
    BINT_MAP_STD(1, UPG_AUX, UPG_AUX_INTR2_CPU),
    BINT_MAP_STD(1, UPG_AUX_AON, UPG_AUX_AON_INTR2_CPU),
    BINT_MAP(1, UPG_BSC, "" , IRQ0_IRQEN, REGULAR, MASK, 0xFFFFFF3F),
    BINT_MAP(1, UPG_BSC_AON, "" , IRQ0_AON_IRQEN, REGULAR, MASK, 0xFFFFFFE7),
    BINT_MAP(2, UPG_MAIN, "" , IRQ0_IRQEN, REGULAR, MASK, 0xFFFFFCC7),
    BINT_MAP(2, UPG_MAIN_AON, "" , IRQ0_AON_IRQEN, REGULAR, MASK, 0xFFFFFE98),
    BINT_MAP(2, UPG_SC, "", SCIRQ0_SCIRQEN, REGULAR, ALL, 0),
    BINT_MAP(2, UPG_SPI, "" , IRQ0_AON_IRQEN, REGULAR, MASK, 0xFFFFFF7F),
#ifdef BSU_USE_UPG_TIMER
    BINT_MAP(2, UPG_TMR, "", TIMER_TIMER_IS, REGULAR, MASK, 0x8),
#else
    BINT_MAP(2, UPG_TMR, "", TIMER_TIMER_IS, REGULAR, ALL, 0),
#endif
    BINT_MAP(2, V3D, "_INTCTL", V3D_CTL_INTCTL, REGULAR, NONE, 0),
    BINT_MAP(2, V3D, "_DBGITC", V3D_DBG_DBQITC, REGULAR, NONE, 0),

    BINT_MAP_STD(2, VEC, VIDEO_ENC_INTR2_CPU),

    BINT_MAP_STD(2, XPT_MSG_STAT, XPT_MSG_DAT_ERR_INTR_L2_CPU ),

    BINT_MAP_STD(2, XPT_MSG, XPT_MSG_BUF_DAT_RDY_INTR_00_31_L2_W0_CPU ),
    BINT_MAP_STD(2, XPT_MSG, XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_CPU ),
    BINT_MAP_STD(2, XPT_MSG, XPT_MSG_BUF_DAT_RDY_INTR_64_95_L2_W2_CPU ),
    BINT_MAP_STD(2, XPT_MSG, XPT_MSG_BUF_DAT_RDY_INTR_96_127_L2_W3_CPU ),
    BINT_MAP_STD(2, XPT_MSG, XPT_MSG_BUF_DAT_RDY_INTR_128_159_L2_W4_CPU ),
    BINT_MAP_STD(2, XPT_MSG, XPT_MSG_BUF_DAT_RDY_INTR_160_191_L2_W5_CPU ),
    BINT_MAP_STD(2, XPT_MSG, XPT_MSG_BUF_DAT_RDY_INTR_192_223_L2_W6_CPU ),
    BINT_MAP_STD(2, XPT_MSG, XPT_MSG_BUF_DAT_RDY_INTR_224_255_L2_W7_CPU ),

    BINT_MAP_STD(2, XPT_OVFL, XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_CPU ),
    BINT_MAP_STD(2, XPT_OVFL, XPT_MSG_BUF_OVFL_INTR_32_63_L2_W9_CPU ),
    BINT_MAP_STD(2, XPT_OVFL, XPT_MSG_BUF_OVFL_INTR_64_95_L2_W10_CPU ),
    BINT_MAP_STD(2, XPT_OVFL, XPT_MSG_BUF_OVFL_INTR_96_127_L2_W11_CPU ),
    BINT_MAP_STD(2, XPT_OVFL, XPT_MSG_BUF_OVFL_INTR_128_159_L2_W12_CPU ),
    BINT_MAP_STD(2, XPT_OVFL, XPT_MSG_BUF_OVFL_INTR_160_191_L2_W13_CPU ),
    BINT_MAP_STD(2, XPT_OVFL, XPT_MSG_BUF_OVFL_INTR_192_223_L2_W14_CPU ),
    BINT_MAP_STD(2, XPT_OVFL, XPT_MSG_BUF_OVFL_INTR_224_255_L2_W15_CPU ),

    BINT_MAP(2, XPT_FE, "_STATUS0", XPT_FE_INTR_STATUS0_REG, REGULAR, ALL, 0),
    BINT_MAP(2, XPT_FE, "_STATUS1", XPT_FE_INTR_STATUS1_REG, REGULAR, ALL, 0),
    BINT_MAP(2, XPT_FE, "_IBP_PCC", XPT_FULL_PID_PARSER_IBP_PCC_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(2, XPT_FE, "_PBP_PCC", XPT_FULL_PID_PARSER_PBP_PCC_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(2, XPT_FE, "_IBP_SCC", XPT_FULL_PID_PARSER_IBP_SCC_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(2, XPT_FE, "_PBP_SCC", XPT_FULL_PID_PARSER_PBP_SCC_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(2, XPT_FE, "_IBP_PSG", XPT_FULL_PID_PARSER_IBP_PSG_PROTOCOL_INTR_STATUS_REG,   REGULAR, ALL, 0),
    BINT_MAP(2, XPT_FE, "_PBP_PSG", XPT_FULL_PID_PARSER_PBP_PSG_PROTOCOL_INTR_STATUS_REG, REGULAR, ALL, 0  ),
    BINT_MAP(2, XPT_FE, "_IBP_TRANSPORT_ERROR", XPT_FULL_PID_PARSER_IBP_TRANSPORT_ERROR_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(2, XPT_FE, "_PBP_TRANSPORT_ERROR", XPT_FULL_PID_PARSER_PBP_TRANSPORT_ERROR_INTR_STATUS_REG, REGULAR, ALL, 0),

    BINT_MAP(2, XPT_STATUS, "_BUS", XPT_BUS_IF_INTR_STATUS_REG,   REGULAR, ALL, 0),
    BINT_MAP(2, XPT_STATUS, "_BUS2", XPT_BUS_IF_INTR_STATUS2_REG, REGULAR, ALL, 0),
    BINT_MAP(2, XPT_STATUS, "_BUS3", XPT_BUS_IF_INTR_STATUS3_REG,   REGULAR, ALL, 0),
    BINT_MAP(2, XPT_STATUS, "_BUS4", XPT_BUS_IF_INTR_STATUS4_REG, REGULAR, ALL, 0),
    BINT_MAP(2, XPT_STATUS, "_BUS5", XPT_BUS_IF_INTR_STATUS5_REG, REGULAR, ALL, 0),
    BINT_MAP(2, XPT_STATUS, "_WAKEUP", XPT_WAKEUP_INTR_STATUS_REG, REGULAR, ALL, 0),

    BINT_MAP(2, XPT_RAV, "CDB_OVERFLOW_CX00_31", XPT_RAVE_CDB_OVERFLOW_CX00_31_L2_INTR_CPU_STATUS_0_31, STANDARD, ALL, 0),
    BINT_MAP(2, XPT_RAV, "CDB_OVERFLOW_CX32_47", XPT_RAVE_CDB_OVERFLOW_CX32_47_L2_INTR_CPU_STATUS_32_47, STANDARD, ALL, 0),
    BINT_MAP(2, XPT_RAV, "ITB_OVERFLOW_CX00_31", XPT_RAVE_ITB_OVERFLOW_CX00_31_L2_INTR_CPU_STATUS_0_31, STANDARD, ALL, 0),
    BINT_MAP(2, XPT_RAV, "ITB_OVERFLOW_CX32_47", XPT_RAVE_ITB_OVERFLOW_CX32_47_L2_INTR_CPU_STATUS_32_47, STANDARD, ALL, 0),
    BINT_MAP(2, XPT_RAV, "EMU_ERROR_CX00_31", XPT_RAVE_EMU_ERROR_CX00_31_L2_INTR_CPU_STATUS_0_31, STANDARD, ALL, 0),
    BINT_MAP(2, XPT_RAV, "EMU_ERROR_CX32_47", XPT_RAVE_EMU_ERROR_CX32_47_L2_INTR_CPU_STATUS_32_47, STANDARD, ALL, 0),
    BINT_MAP(2, XPT_RAV, "PUSI_ERROR_CX00_31", XPT_RAVE_PUSI_ERROR_CX00_31_L2_INTR_CPU_STATUS_0_31, STANDARD, ALL, 0),
    BINT_MAP(2, XPT_RAV, "PUSI_ERROR_CX32_47", XPT_RAVE_PUSI_ERROR_CX32_47_L2_INTR_CPU_STATUS_32_47, STANDARD, ALL, 0),
    BINT_MAP(2, XPT_RAV, "TEI_ERROR_CX00_31", XPT_RAVE_TEI_ERROR_CX00_31_L2_INTR_CPU_STATUS_0_31, STANDARD, ALL, 0),
    BINT_MAP(2, XPT_RAV, "TEI_ERROR_CX32_47", XPT_RAVE_TEI_ERROR_CX32_47_L2_INTR_CPU_STATUS_32_47, STANDARD, ALL, 0),
    BINT_MAP(2, XPT_RAV, "CC_ERROR_CX00_31", XPT_RAVE_CC_ERROR_CX00_31_L2_INTR_CPU_STATUS_0_31, STANDARD, ALL, 0),
    BINT_MAP(2, XPT_RAV, "CC_ERROR_CX32_47", XPT_RAVE_CC_ERROR_CX32_47_L2_INTR_CPU_STATUS_32_47, STANDARD, ALL, 0),
    BINT_MAP(2, XPT_RAV, "CDB_UPPER_THRESH_00_31", XPT_RAVE_CDB_UPPER_THRESH_CX00_31_L2_INTR_CPU_STATUS_0_31, STANDARD, ALL, 0),
    BINT_MAP(2, XPT_RAV, "CDB_UPPER_THRESH_32_47", XPT_RAVE_CDB_UPPER_THRESH_CX32_47_L2_INTR_CPU_STATUS_32_47, STANDARD, ALL, 0),
    BINT_MAP(2, XPT_RAV, "ITB_UPPER_THRESH_00_31", XPT_RAVE_ITB_UPPER_THRESH_CX00_31_L2_INTR_CPU_STATUS_0_31, STANDARD, ALL, 0),
    BINT_MAP(2, XPT_RAV, "ITB_UPPER_THRESH_32_47", XPT_RAVE_ITB_UPPER_THRESH_CX32_47_L2_INTR_CPU_STATUS_32_47, STANDARD, ALL, 0),
    BINT_MAP(2, XPT_RAV, "CDB_LOWER_THRESH_00_31", XPT_RAVE_CDB_LOWER_THRESH_CX00_31_L2_INTR_CPU_STATUS_0_31, STANDARD, ALL, 0),
    BINT_MAP(2, XPT_RAV, "CDB_LOWER_THRESH_32_47", XPT_RAVE_CDB_LOWER_THRESH_CX32_47_L2_INTR_CPU_STATUS_32_47, STANDARD, ALL, 0),
    BINT_MAP(2, XPT_RAV, "ITB_LOWER_THRESH_00_31", XPT_RAVE_ITB_LOWER_THRESH_CX00_31_L2_INTR_CPU_STATUS_0_31, STANDARD, ALL, 0),
    BINT_MAP(2, XPT_RAV, "ITB_LOWER_THRESH_32_47", XPT_RAVE_ITB_LOWER_THRESH_CX32_47_L2_INTR_CPU_STATUS_32_47, STANDARD, ALL, 0),
    BINT_MAP(2, XPT_RAV, "CDB_MIN_DEPTH_THRESH_00_31", XPT_RAVE_CDB_MIN_DEPTH_THRESH_CX00_31_L2_INTR_CPU_STATUS_0_31, STANDARD, ALL, 0),
    BINT_MAP(2, XPT_RAV, "CDB_MIN_DEPTH_THRESH_32_47", XPT_RAVE_CDB_MIN_DEPTH_THRESH_CX32_47_L2_INTR_CPU_STATUS_32_47, STANDARD, ALL, 0),
    BINT_MAP(2, XPT_RAV, "ITB_MIN_DEPTH_THRESH_00_31", XPT_RAVE_ITB_MIN_DEPTH_THRESH_CX00_31_L2_INTR_CPU_STATUS_0_31, STANDARD, ALL, 0),
    BINT_MAP(2, XPT_RAV, "ITB_MIN_DEPTH_THRESH_32_47", XPT_RAVE_ITB_MIN_DEPTH_THRESH_CX32_47_L2_INTR_CPU_STATUS_32_47, STANDARD, ALL, 0),
    BINT_MAP(2, XPT_RAV, "RAVE_TSIO_DMA_END_CX00_31", XPT_RAVE_TSIO_DMA_END_CX00_31_L2_INTR_CPU_STATUS_0_31, STANDARD, ALL, 0),
    BINT_MAP(2, XPT_RAV, "RAVE_TSIO_DMA_END_CX32_47", XPT_RAVE_TSIO_DMA_END_CX32_47_L2_INTR_CPU_STATUS_32_47, STANDARD, ALL, 0),

    BINT_MAP(2, XPT_PCR, "XPT_DPCR0", XPT_DPCR0_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(2, XPT_PCR, "XPT_DPCR1", XPT_DPCR1_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(2, XPT_PCR, "XPT_DPCR2", XPT_DPCR2_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(2, XPT_PCR, "XPT_DPCR3", XPT_DPCR3_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(2, XPT_PCR, "XPT_DPCR4", XPT_DPCR4_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(2, XPT_PCR, "XPT_DPCR5", XPT_DPCR5_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(2, XPT_PCR, "XPT_DPCR6", XPT_DPCR6_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(2, XPT_PCR, "XPT_DPCR7", XPT_DPCR7_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(2, XPT_PCR, "XPT_DPCR8", XPT_DPCR8_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(2, XPT_PCR, "XPT_DPCR9", XPT_DPCR9_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(2, XPT_PCR, "XPT_DPCR10", XPT_DPCR10_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(2, XPT_PCR, "XPT_DPCR11", XPT_DPCR11_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(2, XPT_PCR, "XPT_DPCR12", XPT_DPCR12_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(2, XPT_PCR, "XPT_DPCR13", XPT_DPCR13_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(2, XPT_PCR, "XPT_PCROFF0", XPT_PCROFFSET_INTERRUPT0_STATUS, REGULAR, ALL, 0),
    BINT_MAP(2, XPT_PCR, "XPT_PCROFF1", XPT_PCROFFSET_INTERRUPT1_STATUS, REGULAR, ALL, 0),
    BINT_MAP(2, XPT_PCR, "XPT_PCROFF2", XPT_PCROFFSET_INTERRUPT2_STATUS, REGULAR, ALL, 0),
    BINT_MAP(2, XPT_PCR, "XPT_PCROFF3", XPT_PCROFFSET_INTERRUPT3_STATUS, REGULAR, ALL, 0),
    BINT_MAP(2, XPT_PCR, "XPT_PCROFF_STC", XPT_PCROFFSET_STC_INTERRUPT_STATUS, REGULAR, ALL, 0),

    BINT_MAP_STD(2, XPT_MDMA_MCPB, XPT_MEMDMA_MCPB_DESC_DONE_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MDMA_MCPB, XPT_MEMDMA_MCPB_MISC_FALSE_WAKE_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MDMA_MCPB,  XPT_MEMDMA_MCPB_MISC_OOS_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MDMA_MCPB,  XPT_MEMDMA_MCPB_MISC_TS_PARITY_ERR_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MDMA_MCPB,  XPT_MEMDMA_MCPB_MISC_TEI_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MDMA_MCPB,  XPT_MEMDMA_MCPB_MISC_ASF_LEN_ERR_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MDMA_MCPB,  XPT_MEMDMA_MCPB_MISC_ASF_COMPRESSED_DATA_RECEIVED_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MDMA_MCPB,  XPT_MEMDMA_MCPB_MISC_ASF_PROTOCOL_ERR_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MDMA_MCPB,  XPT_MEMDMA_MCPB_MISC_ASF_PADDING_LEN_ERR_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MDMA_MCPB,  XPT_MEMDMA_MCPB_MISC_TS_RANGE_ERR_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MDMA_MCPB,  XPT_MEMDMA_MCPB_MISC_PES_NEXT_TS_RANGE_ERR_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MDMA_MCPB,  XPT_MEMDMA_MCPB_MISC_PAUSE_AT_DESC_READ_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MDMA_MCPB,  XPT_MEMDMA_MCPB_MISC_PAUSE_AT_DESC_END_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MDMA_MCPB,  XPT_MEMDMA_MCPB_MISC_PAUSE_AFTER_GROUP_PACKETS_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MDMA_MCPB, XPT_MEMDMA_MCPB_MISC_DESC_TAGID_MISMATCH_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MDMA_MCPB, XPT_MEMDMA_MCPB_MISC_DATA_TAGID_MISMATCH_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MDMA_MCPB,  XPT_MEMDMA_MCPB_MISC_CRC_COMPARE_ERROR_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MCPB,  XPT_MCPB_DESC_DONE_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MCPB,  XPT_MCPB_MISC_FALSE_WAKE_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MCPB,  XPT_MCPB_MISC_OOS_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MCPB,  XPT_MCPB_MISC_TS_PARITY_ERR_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MCPB,  XPT_MCPB_MISC_TEI_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MCPB,  XPT_MCPB_MISC_ASF_LEN_ERR_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MCPB,  XPT_MCPB_MISC_ASF_COMPRESSED_DATA_RECEIVED_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MCPB,  XPT_MCPB_MISC_ASF_PROTOCOL_ERR_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MCPB,  XPT_MCPB_MISC_ASF_PADDING_LEN_ERR_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MCPB,  XPT_MCPB_MISC_TS_RANGE_ERR_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MCPB,  XPT_MCPB_MISC_PES_NEXT_TS_RANGE_ERR_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MCPB,  XPT_MCPB_MISC_PAUSE_AT_DESC_READ_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MCPB,  XPT_MCPB_MISC_PAUSE_AT_DESC_END_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MCPB,  XPT_MCPB_MISC_PAUSE_AFTER_GROUP_PACKETS_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MCPB,  XPT_MCPB_MISC_DESC_TAGID_MISMATCH_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MCPB,  XPT_MCPB_MISC_DATA_TAGID_MISMATCH_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_MCPB,  XPT_MCPB_MISC_CRC_COMPARE_ERROR_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_WMDMA,XPT_WDMA_BTP_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_WMDMA, XPT_WDMA_OVERFLOW_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_WMDMA, XPT_WDMA_DESC_DONE_INTR_L2_CPU),
    BINT_MAP_STD(2, XPT_EXTCARD, XPT_TSIO_INTR_L2_CPU),
    /* RFM interrupt */
#if BCHP_VER <= BCHP_VER_B0
    BINT_MAP_STD(3, RFM, RFM_L2_CPU),
#endif
    /* Route the frontend interrupts to the host when SAT is running on the host and not the LEAP */
    BINT_MAP_STD(3,LEAP, LEAP_HOST_L1_INTR_W0),
    BINT_MAP_STD(3,LEAP, AFEC_GLOBAL_INTR_0_CPU),
    BINT_MAP_STD(3,LEAP,AIF_WB_SAT_CORE_INTR2_0_0_CPU),
    BINT_MAP_STD(3,LEAP,AIF_MDAC_CAL_SAT_CORE_INTR2_0_CPU),
    BINT_MAP_STD(3,LEAP,SDS_INTR2_0_0_CPU),
    BINT_MAP_STD(3,LEAP,SDS_INTR2_0_1_CPU),
    BINT_MAP_STD(3,LEAP, AFEC0_INTR_0_CPU),
    BINT_MAP_STD(3,LEAP, AFEC1_INTR_0_CPU),
    BINT_MAP_STD(3,LEAP,TFEC_INTR2_0_CPU),
    BINT_MAP_STD(3,LEAP,TFEC_INTR2_1_CPU),
    BINT_MAP_STD(3,LEAP,SDS_DSEC_INTR2_0_CPU),
    BINT_MAP_STD(3,LEAP,FTM_INTR2_CPU),

    /* MPM interrupts */
    BINT_MAP_STD(3, MPM_TOP, MPM_HOST_L2_CPU),

    BINT_MAP_STD(0, SCPU, SCPU_HOST_INTR2_CPU), /* SAGE_SUPPORT */


    BINT_MAP_LAST()
};
#endif

static const BINT_Settings bint_7364Settings =
{
    NULL,
    BINT_P_7364_ClearInt,
    BINT_P_7364_SetMask,
    BINT_P_7364_ClearMask,
    NULL,
    BINT_P_7364_ReadStatus,
    bint_7364,
    "7364"
};

/* On some parts, the relative location of the status and enable regs changed. */
static uint32_t getXptFeIntEnableRegAddr( uint32_t baseAddr )
{
    uint32_t enableRegAddr = baseAddr + BINT_P_XPT_ENABLE;

    switch( baseAddr )
    {
        case BCHP_XPT_FE_INTR_STATUS0_REG:
            enableRegAddr = BCHP_XPT_FE_INTR_STATUS0_REG_EN;
            break;
        case BCHP_XPT_FE_INTR_STATUS1_REG:
            enableRegAddr = BCHP_XPT_FE_INTR_STATUS1_REG_EN;
            break;
        case BCHP_XPT_FE_INTR_STATUS2_REG:
            enableRegAddr = BCHP_XPT_FE_INTR_STATUS2_REG_EN;
            break;
        default:
            break;
    }

    return enableRegAddr;
}

static void BINT_P_7364_ClearInt( BREG_Handle regHandle, uint32_t baseAddr, int shift )
{
    BDBG_MSG(("ClearInt %#x:%d", baseAddr, shift));
    switch( baseAddr )
    {

        BINT_P_XPT_STATUS_CASES
            BREG_Write32( regHandle, baseAddr + BINT_P_XPT_STATUS, ~(1ul<<shift));
            break;
        BINT_P_TIMER_CASES
            BREG_Write32( regHandle, baseAddr + BINT_P_TIMER_STATUS, 1ul<<shift);
            break;
        BINT_P_UPGSC_CASES
        BINT_P_IRQ0_CASES
        BINT_P_IRQ0_AON_CASES
            /* Has to cleared at the source */
            break;
        BINT_P_PCROFFSET_CASES
            /* Write 0 to clear the int bit. Writing 1's are ingored. */
            BREG_Write32( regHandle, baseAddr + BINT_P_PCROFFSET_STATUS, ~( 1ul << shift ) );
            break;
        default:
            /* Other types of interrupts do not support clearing of interrupts (condition must be cleared) */
            break;
    }
}

static void BINT_P_7364_SetMask( BREG_Handle regHandle, uint32_t baseAddr, int shift )
{
    uint32_t intEnable;

    BDBG_MSG(("SetMask %#x:%d", baseAddr, shift));

    switch( baseAddr )
    {

    BINT_P_XPT_STATUS_CASES
        intEnable = BREG_Read32( regHandle, getXptFeIntEnableRegAddr(baseAddr));
        intEnable &= ~(1ul<<shift);
        BREG_Write32( regHandle, getXptFeIntEnableRegAddr(baseAddr), intEnable);
        break;
    BINT_P_TIMER_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_TIMER_MASK);
        intEnable &= ~(1ul<<shift);
        BREG_Write32( regHandle, baseAddr + BINT_P_TIMER_MASK, intEnable);
        break;
    BINT_P_IRQ0_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_IRQ0_ENABLE);
        intEnable &= ~(1ul<<shift);
        BREG_Write32( regHandle, baseAddr + BINT_P_IRQ0_ENABLE, intEnable);
        break;
    BINT_P_IRQ0_AON_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_IRQ0_AON_ENABLE);
        intEnable &= ~(1ul<<shift);
        BREG_Write32( regHandle, baseAddr + BINT_P_IRQ0_AON_ENABLE, intEnable);
        break;
    BINT_P_UPGSC_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_UPGSC_ENABLE );
        intEnable &= ~(1ul<<shift);
        BREG_Write32( regHandle, baseAddr + BINT_P_UPGSC_ENABLE, intEnable );
        break;

    BINT_P_PCROFFSET_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_PCROFFSET_ENABLE );
        intEnable &= ~( 1ul << shift );
        BREG_Write32( regHandle, baseAddr + BINT_P_PCROFFSET_ENABLE, intEnable);
        break;

    default:
       BDBG_ERR(("NOT SUPPORTED baseAddr 0x%08x ,regHandle %p,  shift %d",
                         baseAddr, (void *) regHandle, shift));

        /* Unhandled interrupt base address */
        BDBG_ASSERT( false );
        break;
    }
}

static void BINT_P_7364_ClearMask( BREG_Handle regHandle, uint32_t baseAddr, int shift )
{
    uint32_t intEnable;

    BDBG_MSG(("ClearMask %#x:%d", baseAddr, shift));

    switch( baseAddr )
    {

    BINT_P_XPT_STATUS_CASES
        intEnable = BREG_Read32( regHandle, getXptFeIntEnableRegAddr(baseAddr));
        intEnable |= 1ul<<shift;
        BREG_Write32( regHandle, getXptFeIntEnableRegAddr(baseAddr), intEnable);
        break;
    BINT_P_IRQ0_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_IRQ0_ENABLE);
        intEnable |= (1ul<<shift);
        BREG_Write32( regHandle, baseAddr + BINT_P_IRQ0_ENABLE, intEnable );
        break;
    BINT_P_IRQ0_AON_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_IRQ0_AON_ENABLE);
        intEnable |= (1ul<<shift);
        BREG_Write32( regHandle, baseAddr + BINT_P_IRQ0_AON_ENABLE, intEnable );
        break;
    BINT_P_UPGSC_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_UPGSC_ENABLE );
        intEnable |= 1ul<<shift;
        BREG_Write32( regHandle, baseAddr + BINT_P_UPGSC_ENABLE, intEnable );
        break;
    BINT_P_TIMER_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_TIMER_MASK );
        intEnable |= (1ul<<shift);
        BREG_Write32( regHandle, baseAddr + BINT_P_TIMER_MASK, intEnable );
        break;

    BINT_P_PCROFFSET_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_PCROFFSET_ENABLE );
        intEnable |= ( 1ul << shift );
        BREG_Write32( regHandle, baseAddr + BINT_P_PCROFFSET_ENABLE, intEnable);
        break;

    default:
        /* Unhandled interrupt base address */
        BDBG_ASSERT( false );
        break;
    }
}


static uint32_t BINT_P_7364_ReadStatus( BREG_Handle regHandle, uint32_t baseAddr )
{
    BDBG_MSG(("ReadStatus %#x", baseAddr));
    switch( baseAddr )
    {
    BINT_P_XPT_STATUS_CASES
        return BREG_Read32( regHandle, baseAddr + BINT_P_XPT_STATUS );
    BINT_P_TIMER_CASES
        return BREG_Read32( regHandle, baseAddr + BINT_P_TIMER_STATUS );
    BINT_P_IRQ0_CASES
        return BREG_Read32( regHandle, baseAddr + BINT_P_IRQ0_STATUS );
    BINT_P_IRQ0_AON_CASES
        return BREG_Read32( regHandle, baseAddr + BINT_P_IRQ0_AON_STATUS );
    BINT_P_UPGSC_CASES
        return BREG_Read32( regHandle, baseAddr + BINT_P_UPGSC_ENABLE );
    BINT_P_PCROFFSET_CASES
        return BREG_Read32( regHandle, baseAddr + BINT_P_PCROFFSET_STATUS );
    default:
        /* Unhandled interrupt base address */
        BDBG_ASSERT( false );
        return 0;
    }
}

const BINT_Settings *BINT_7364_GetSettings( void )
{
    return &bint_7364Settings;
}

/* End of file */

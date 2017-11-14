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
#include "bchp_common.h"
#include "bint_7278.h"
#include "bkni.h"
/* Include interrupt definitions from RDB */
#include "bchp_hif_cpu_intr1.h"

/* Standard L2 stuff */
#include "bchp_aud_inth.h"
#include "bchp_hvd_intr2_0.h"
#include "bchp_hvd_intr2_1.h"
#include "bchp_bsp_control_intr2.h"
#include "bchp_bvnb_intr2.h"
#include "bchp_bvnb_intr2_1.h"
#include "bchp_bvnf_intr2_0.h"
#include "bchp_bvnf_intr2_1.h"
#include "bchp_bvnf_intr2_3.h"
#include "bchp_bvnf_intr2_5.h"
#include "bchp_bvnf_intr2_9.h"
#include "bchp_bvnf_intr2_16.h"
#include "bchp_bvnm_intr2_0.h"
#include "bchp_bvnm_intr2_1.h"
#include "bchp_clkgen_intr2.h"
#include "bchp_dvp_hr_intr2.h"
#include "bchp_dvp_ht.h"
#include "bchp_dvp_hr.h"
#include "bchp_m2mc_l2.h"
#include "bchp_m2mc1_l2.h"
#include "bchp_mm_m2mc0_l2.h"
#include "bchp_hdmi_rx_intr2_0.h"
#include "bchp_hdmi_tx_intr2.h"
#include "bchp_hdmi_tx_scdc_intr2_0.h"
#include "bchp_hdmi_tx_hae_intr2_0.h"
#include "bchp_memc_l2_0_0.h"
#include "bchp_memc_l2_0_1.h"
#include "bchp_memc_l2_0_2.h"
#include "bchp_memc_l2_1_0.h"
#include "bchp_memc_l2_1_1.h"
#include "bchp_memc_l2_1_2.h"
#include "bchp_raaga_dsp_inth.h"
#include "bchp_raaga_dsp_fw_inth.h"
#include "bchp_sun_l2.h"
#include "bchp_aon_l2.h"
#include "bchp_aon_pm_l2.h"
#include "bchp_upg_aux_aon_intr2.h"
#include "bchp_upg_aux_intr2.h"
#include "bchp_upg_bsc_aon_irq.h"
#include "bchp_upg_bsc_irq.h"
#include "bchp_upg_main_aon_irq.h"
#include "bchp_upg_main_irq.h"
#include "bchp_upg_spi_aon_irq.h"

#include "bchp_v3d_ctl_0.h"
/* #include "bchp_v3d_ctl_1.h"
#include "bchp_v3d_ctl_2.h" */
#include "bchp_v3d_hub_ctl.h"
#include "bchp_video_enc_intr2.h"

/* VICE */
#include "bchp_vice_l2_0.h"

/* ASP */
#include "bchp_asp_arcss_host_fw2h_l2.h"

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
#include "bchp_xpt_mcpb_desc_done_intr_l2.h"
#include "bchp_xpt_mcpb_misc_false_wake_intr_l2.h"
#include "bchp_xpt_mcpb_misc_tei_intr_l2.h"
#include "bchp_xpt_mcpb_misc_ts_range_err_intr_l2.h"
#include "bchp_xpt_mcpb_desc_done_intr_l2.h"
#include "bchp_xpt_mcpb_misc_false_wake_intr_l2.h"
#include "bchp_xpt_mcpb_misc_pause_at_desc_end_intr_l2.h"
#include "bchp_xpt_mcpb_misc_tei_intr_l2.h"
#include "bchp_xpt_mcpb_misc_ts_range_err_intr_l2.h"
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
#include "bchp_xpt_wdma_btp_intr_l2.h"
#include "bchp_xpt_wdma_overflow_intr_l2.h"
#include "bchp_xpt_wdma_desc_done_intr_l2.h"
#include "bchp_xpt_rave_cdb_lower_thresh_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_cdb_lower_thresh_cx32_47_l2_intr.h"
#include "bchp_xpt_rave_itb_lower_thresh_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_itb_lower_thresh_cx32_47_l2_intr.h"
#include "bchp_xpt_rave_cdb_min_depth_thresh_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_cdb_min_depth_thresh_cx32_47_l2_intr.h"
#include "bchp_xpt_rave_itb_min_depth_thresh_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_itb_min_depth_thresh_cx32_47_l2_intr.h"
#include "bchp_xpt_tsio_intr_l2.h"
#include "bchp_xpt_rave_tsio_dma_end_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_tsio_dma_end_cx32_47_l2_intr.h"
/* Smartcard interrupts. */
#include "bchp_scirq0.h"
#include "bchp_scirq1.h"
#include "bchp_xpt_mcpb_host_intr_aggregator.h"
#include "bchp_xpt_wdma_cpu_intr_aggregator.h"
#include "bchp_xpt_memdma_mcpb_host_intr_aggregator.h"
#include "bchp_xpt_rave_cpu_intr_aggregator.h"
#include "bchp_xpt_msg_buf_dat_rdy_cpu_intr_aggregator.h"
#include "bchp_xpt_msg_buf_ovfl_cpu_intr_aggregator.h"

/* Timer */
#include "bchp_timer.h"

/* For SAGE interrupts */
#include "bchp_scpu_host_intr2.h"

BDBG_MODULE(interruptinterface_7278);
#if 0
#define BINT_P_IRQ0_CASES \
    case BCHP_IRQ0_IRQEN:

#define BINT_P_IRQ0_ENABLE      0
#define BINT_P_IRQ0_STATUS      4
#endif

#if 0
#define BINT_P_IRQ0_AON_CASES \
    case BCHP_IRQ0_AON_IRQEN:

#define BINT_P_IRQ0_AON_ENABLE      0
#define BINT_P_IRQ0_AON_STATUS      4
#endif

#define BINT_P_STD_RO_STATUS_STATUS      0x0
#define BINT_P_STD_RO_STATUS_MASK_STATUS 0x4
#define BINT_P_STD_RO_STATUS_MASK_SET    0x8
#define BINT_P_STD_RO_STATUS_MASK_CLEAR  0xc

#define BINT_P_STD_RO_STATUS_CASES \
    case BCHP_UPG_MAIN_IRQ_CPU_STATUS: \
    case BCHP_UPG_BSC_IRQ_CPU_STATUS: \
    case BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS: \
    case BCHP_UPG_BSC_AON_IRQ_CPU_STATUS: \
    case BCHP_UPG_SPI_AON_IRQ_CPU_STATUS:

#define BINT_P_XPT_STATUS           0x00
#define BINT_P_XPT_ENABLE           0x04

#define BINT_P_XPT_STATUS_CASES \
    case BCHP_XPT_BUS_IF_INTR_STATUS_REG: \
    case BCHP_XPT_BUS_IF_INTR_STATUS2_REG: \
    case BCHP_XPT_BUS_IF_INTR_STATUS3_REG: \
    case BCHP_XPT_BUS_IF_INTR_STATUS4_REG: \
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

/* There is no constant address mapping from RAVE status to RAVE enable registers. */
#define BINT_P_RAVE_STATUS          0x00


#define BINT_P_XPT_BUF_STATUS       0x00
#define BINT_P_XPT_BUF_ENABLE       0x10

#define BINT_P_XPT_BUF_CASES \
    case BCHP_XPT_MSG_BUF_ERR_00_31: \
    case BCHP_XPT_MSG_BUF_ERR_32_63: \
    case BCHP_XPT_MSG_BUF_ERR_64_95: \
    case BCHP_XPT_MSG_BUF_ERR_96_127: \
    case BCHP_XPT_MSG_BUF_ERR_128_159: \
    case BCHP_XPT_MSG_BUF_ERR_160_191: \
    case BCHP_XPT_MSG_BUF_ERR_192_223: \
    case BCHP_XPT_MSG_BUF_ERR_224_255: \
    case BCHP_XPT_MSG_BUF_DAT_AVAIL_00_31: \
    case BCHP_XPT_MSG_BUF_DAT_AVAIL_32_63: \
    case BCHP_XPT_MSG_BUF_DAT_AVAIL_64_95: \
    case BCHP_XPT_MSG_BUF_DAT_AVAIL_96_127: \
    case BCHP_XPT_MSG_BUF_DAT_AVAIL_128_159: \
    case BCHP_XPT_MSG_BUF_DAT_AVAIL_160_191: \
    case BCHP_XPT_MSG_BUF_DAT_AVAIL_192_223: \
    case BCHP_XPT_MSG_BUF_DAT_AVAIL_224_255:

#define BINT_P_XPT_MSG_ERR_STATUS   ( 0x00 )
#define BINT_P_XPT_MSG_ERR_ENABLE   ( 0x04 )

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


static void BINT_P_ClearInt( BREG_Handle regHandle, uint32_t baseAddr, int shift );
static void BINT_P_SetMask( BREG_Handle regHandle, uint32_t baseAddr, int shift );
static void BINT_P_ClearMask( BREG_Handle regHandle, uint32_t baseAddr, int shift );

static uint32_t BINT_P_ReadStatus( BREG_Handle regHandle, uint32_t baseAddr );

#if NEXUS_WEBCPU_core1_server
static const BINT_P_IntMap bint_map[] =
{
#ifdef BCHP_M2MC1_L2_REG_START
    BINT_MAP_STD(3, M2MC1, M2MC1_L2_CPU),
#endif
    { -1, 0, 0, NULL}
};
#else
static const BINT_P_IntMap bint_map[] =
{
    /* Memory controllers */
    BINT_MAP_STD(1, MEMC0, MEMC_L2_0_0_CPU),
    BINT_MAP_STD(1, MEMC0, MEMC_L2_0_1_CPU),
    BINT_MAP_STD(1, MEMC0, MEMC_L2_0_2_CPU),
#ifdef BCHP_MEMC_L2_1_0_REG_START
    BINT_MAP_STD(1, MEMC1, MEMC_L2_1_0_CPU),
    BINT_MAP_STD(1, MEMC1, MEMC_L2_1_1_CPU),
    BINT_MAP_STD(1, MEMC1, MEMC_L2_1_2_CPU),
#endif
#ifdef BCHP_MEMC_L2_2_0_REG_START
    BINT_MAP_STD(3, MEMC2, MEMC_L2_2_0_CPU),
    BINT_MAP_STD(3, MEMC2, MEMC_L2_2_1_CPU),
    BINT_MAP_STD(3, MEMC2, MEMC_L2_2_2_CPU),
#endif

    /* Security */
#ifdef BCHP_BSP_CONTROL_INTR2_REG_START
    BINT_MAP_STD(0, BSP, BSP_CONTROL_INTR2_CPU),
#endif
#ifdef BCHP_SCPU_HOST_INTR2_REG_START
    BINT_MAP_STD(0, SCPU, SCPU_HOST_INTR2_CPU),
#endif

    /* Graphics */
#ifdef BCHP_M2MC_L2_REG_START
    BINT_MAP_STD(0, GFX, M2MC_L2_CPU),
#endif
#if defined(BCHP_M2MC1_L2_REG_START) && !NEXUS_WEBCPU
    /* in webcpu mode, core0 doesn't get M2MC1 L1 */
    BINT_MAP_STD(0, M2MC1, M2MC1_L2_CPU),
#endif

#ifdef BCHP_MM_M2MC0_REG_START
    BINT_MAP_STD(0, MM_M2MC0, MM_M2MC0_L2_CPU),
#endif

    BINT_MAP_STD(0, AIO, AUD_INTH_R5F),

    /* Display */
#ifdef BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_VEC_CPU_INTR_MASK
    BINT_MAP_STD(0, VEC, VIDEO_ENC_INTR2_CPU),
#elif defined(BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_VEC_CPU_INTR_MASK)
    BINT_MAP_STD(2, VEC, VIDEO_ENC_INTR2_CPU),
#endif

    BINT_MAP_STD(0, BVNB_0, BVNB_INTR2_CPU),
    BINT_MAP_STD(0, BVNB_1, BVNB_INTR2_1_CPU),

    BINT_MAP_STD(0, BVNF_0, BVNF_INTR2_0_R5F),
    BINT_MAP_STD(0, BVNF_1, BVNF_INTR2_1_R5F),
    BINT_MAP_STD(0, BVNF_5, BVNF_INTR2_5_R5F),
    BINT_MAP_STD(0, BVNF_9, BVNF_INTR2_9_R5F),
    BINT_MAP_STD(0, BVNF_16, BVNF_INTR2_16_R5F),
    BINT_MAP_STD(0, BVNM_0, BVNM_INTR2_0_R5F),
    BINT_MAP_STD(0, BVNM_1, BVNM_INTR2_1_R5F),

    BINT_MAP_STD(0, CLKGEN, CLKGEN_INTR2_CPU),

    BINT_MAP_STD(0, DVP_HR, DVP_HR_INTR2_CPU),
    /* HDMI transmitter */
    BINT_MAP_STD(0, HDMI_TX, HDMI_TX_INTR2_CPU),
    BINT_MAP_STD(0, HDMI_TX, HDMI_TX_SCDC_INTR2_0_CPU),
    BINT_MAP_STD(0, HDMI_TX, HDMI_TX_HAE_INTR2_0_CPU),

    /* HDMI reciever */
#ifdef BCHP_HDMI_RX_INTR2_0_REG_START
    BINT_MAP_STD(0, HDMI_RX_0, HDMI_RX_INTR2_0_CPU),
#endif

    /* Audio DSP */
    BINT_MAP_STD(1, RAAGA, RAAGA_DSP_INTH_HOST),
    BINT_MAP_STD(1, RAAGA_FW, RAAGA_DSP_FW_INTH_HOST),
#ifdef BCHP_RAAGA_DSP_INTH_1_REG_START
    BINT_MAP_STD(1, RAAGA, RAAGA_DSP_INTH_1_HOST),
#endif
#ifdef BCHP_RAAGA_DSP_FW_INTH_1_REG_START
    BINT_MAP_STD(1, RAAGA_FW, RAAGA_DSP_FW_INTH_1_HOST),
#endif

    /* Video decoder */
    BINT_MAP_STD(1, HVD0_0, HVD_INTR2_0_CPU),
#ifdef BCHP_HVD_INTR2_1_REG_START
    BINT_MAP_STD(1, HVD1_0, HVD_INTR2_1_CPU),
#endif
#ifdef BCHP_HVD_INTR2_2_REG_START
    BINT_MAP_STD(1, HVD2_0, HVD_INTR2_2_CPU),
#endif

#ifdef BCHP_HIF_CPU_INTR0_INTR_W3_STATUS_CBUS_CPU_INTR_MASK
    BINT_MAP_STD(3, CBUS, CBUS_INTR2_0_CPU),
    BINT_MAP_STD(3, CBUS, CBUS_INTR2_1_CPU),
#elif BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_CBUS_CPU_INTR_MASK
    BINT_MAP_STD(0, CBUS, CBUS_INTR2_0_CPU),
    BINT_MAP_STD(0, CBUS, CBUS_INTR2_1_CPU),
#endif

    /* Video encoder */
#ifdef BCHP_VICE_L2_0_REG_START
    BINT_MAP_STD(2, VICE2_0, VICE_L2_0_CPU),
#endif

        /* ASP */
#ifdef BCHP_ASP_ARCSS_HOST_FW2H_L2_REG_START
    BINT_MAP_STD(1, ASP_0, ASP_ARCSS_HOST_FW2H_L2_HOST),
#endif

    BINT_MAP_STD(2, SYS_AON, AON_L2_CPU),
    BINT_MAP_STD(2, UPG_AUX_AON, UPG_AUX_AON_INTR2_CPU),
    BINT_MAP(2, UPG_BSC, "", UPG_BSC_IRQ_CPU_STATUS, REGULAR, SOME, 0x7 ),
    BINT_MAP(2, UPG_BSC_AON, "", UPG_BSC_AON_IRQ_CPU_STATUS, REGULAR, SOME, 0x3 ),
    BINT_MAP(2, UPG_MAIN, "", UPG_MAIN_IRQ_CPU_STATUS, REGULAR, SOME, 0x3 ),
    BINT_MAP(2, UPG_MAIN_AON, "", UPG_MAIN_AON_IRQ_CPU_STATUS, REGULAR, SOME, 0x3f ),
    BINT_MAP(2, UPG_SPI, "", UPG_SPI_AON_IRQ_CPU_STATUS, REGULAR, SOME, 0x1 ),
    BINT_MAP(2, UPG_SC, "", SCIRQ0_SCIRQEN, REGULAR, ALL, 0),
#ifdef BSU_USE_UPG_TIMER
    BINT_MAP(2, UPG_TMR, "", TIMER_TIMER_IS, REGULAR, MASK, 0x8),
#else
    BINT_MAP(2, UPG_TMR, "", TIMER_TIMER_IS, REGULAR, ALL, 0),
#endif
    BINT_MAP(3, V3D, "_INT", V3D_CTL_0_INT_STS, REGULAR, NONE, 0),
    BINT_MAP(3, V3D_HUB, "_INT", V3D_HUB_CTL_INT_STS, REGULAR, NONE, 0),
    BINT_MAP(3, XPT_FE, "_STATUS0", XPT_FE_INTR_STATUS0_REG, REGULAR, ALL, 0),
    BINT_MAP(3, XPT_FE, "_STATUS1", XPT_FE_INTR_STATUS1_REG, REGULAR, ALL, 0),
    BINT_MAP(3, XPT_FE, "_IBP_PCC", XPT_FULL_PID_PARSER_IBP_PCC_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(3, XPT_FE, "_PBP_PCC", XPT_FULL_PID_PARSER_PBP_PCC_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(3, XPT_FE, "_IBP_SCC", XPT_FULL_PID_PARSER_IBP_SCC_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(3, XPT_FE, "_PBP_SCC", XPT_FULL_PID_PARSER_PBP_SCC_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(3, XPT_FE, "_IBP_PSG", XPT_FULL_PID_PARSER_IBP_PSG_PROTOCOL_INTR_STATUS_REG,   REGULAR, ALL, 0),
    BINT_MAP(3, XPT_FE, "_PBP_PSG", XPT_FULL_PID_PARSER_PBP_PSG_PROTOCOL_INTR_STATUS_REG, REGULAR, ALL, 0  ),
    BINT_MAP(3, XPT_FE, "_IBP_TRANSPORT_ERROR", XPT_FULL_PID_PARSER_IBP_TRANSPORT_ERROR_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(3, XPT_FE, "_PBP_TRANSPORT_ERROR", XPT_FULL_PID_PARSER_PBP_TRANSPORT_ERROR_INTR_STATUS_REG, REGULAR, ALL, 0),

    BINT_MAP_L3_ROOT(3, XPT_OVFL, XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2),
        BINT_MAP_L3(3, XPT_OVFL, XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2,MSG_BUF_OVFL_INTR_00_31, "", XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_CPU_STATUS, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_OVFL, XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2,MSG_BUF_OVFL_INTR_32_63, "", XPT_MSG_BUF_OVFL_INTR_32_63_L2_W9_CPU_STATUS, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_OVFL, XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2,MSG_BUF_OVFL_INTR_64_95, "", XPT_MSG_BUF_OVFL_INTR_64_95_L2_W10_CPU_STATUS, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_OVFL, XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2,MSG_BUF_OVFL_INTR_96_127, "", XPT_MSG_BUF_OVFL_INTR_96_127_L2_W11_CPU_STATUS, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_OVFL, XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2,MSG_BUF_OVFL_INTR_128_159, "", XPT_MSG_BUF_OVFL_INTR_128_159_L2_W12_CPU_STATUS, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_OVFL, XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2,MSG_BUF_OVFL_INTR_160_191, "", XPT_MSG_BUF_OVFL_INTR_160_191_L2_W13_CPU_STATUS, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_OVFL, XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2,MSG_BUF_OVFL_INTR_192_223, "", XPT_MSG_BUF_OVFL_INTR_192_223_L2_W14_CPU_STATUS, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_OVFL, XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2,MSG_BUF_OVFL_INTR_224_255, "", XPT_MSG_BUF_OVFL_INTR_224_255_L2_W15_CPU_STATUS, STANDARD, ALL, 0),

    BINT_MAP_STD(3, XPT_MSG_STAT, XPT_MSG_DAT_ERR_INTR_L2_CPU),

    BINT_MAP_L3_ROOT(3, XPT_MSG, XPT_MSG_BUF_DAT_RDY_CPU_INTR_AGGREGATOR_INTR_W0),
        BINT_MAP_L3(3, XPT_MSG, XPT_MSG_BUF_DAT_RDY_CPU_INTR_AGGREGATOR_INTR_W0, MSG_BUF_DAT_RDY_INTR_00_31, "", XPT_MSG_BUF_DAT_RDY_INTR_00_31_L2_W0_CPU_STATUS, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_MSG, XPT_MSG_BUF_DAT_RDY_CPU_INTR_AGGREGATOR_INTR_W0, MSG_BUF_DAT_RDY_INTR_32_63, "", XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_CPU_STATUS, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_MSG, XPT_MSG_BUF_DAT_RDY_CPU_INTR_AGGREGATOR_INTR_W0, MSG_BUF_DAT_RDY_INTR_64_95, "", XPT_MSG_BUF_DAT_RDY_INTR_64_95_L2_W2_CPU_STATUS, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_MSG, XPT_MSG_BUF_DAT_RDY_CPU_INTR_AGGREGATOR_INTR_W0, MSG_BUF_DAT_RDY_INTR_96_127, "", XPT_MSG_BUF_DAT_RDY_INTR_96_127_L2_W3_CPU_STATUS, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_MSG, XPT_MSG_BUF_DAT_RDY_CPU_INTR_AGGREGATOR_INTR_W0, MSG_BUF_DAT_RDY_INTR_128_159, "", XPT_MSG_BUF_DAT_RDY_INTR_128_159_L2_W4_CPU_STATUS, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_MSG, XPT_MSG_BUF_DAT_RDY_CPU_INTR_AGGREGATOR_INTR_W0, MSG_BUF_DAT_RDY_INTR_160_191, "", XPT_MSG_BUF_DAT_RDY_INTR_160_191_L2_W5_CPU_STATUS, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_MSG, XPT_MSG_BUF_DAT_RDY_CPU_INTR_AGGREGATOR_INTR_W0, MSG_BUF_DAT_RDY_INTR_192_223, "", XPT_MSG_BUF_DAT_RDY_INTR_192_223_L2_W6_CPU_STATUS, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_MSG, XPT_MSG_BUF_DAT_RDY_CPU_INTR_AGGREGATOR_INTR_W0, MSG_BUF_DAT_RDY_INTR_224_255, "", XPT_MSG_BUF_DAT_RDY_INTR_224_255_L2_W7_CPU_STATUS, STANDARD, ALL, 0),

    BINT_MAP(3, XPT_PCR, "XPT_DPCR0", XPT_DPCR0_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(3, XPT_PCR, "XPT_DPCR1", XPT_DPCR1_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(3, XPT_PCR, "XPT_DPCR2", XPT_DPCR2_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(3, XPT_PCR, "XPT_DPCR3", XPT_DPCR3_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(3, XPT_PCR, "XPT_DPCR4", XPT_DPCR4_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(3, XPT_PCR, "XPT_DPCR5", XPT_DPCR5_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(3, XPT_PCR, "XPT_DPCR6", XPT_DPCR6_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(3, XPT_PCR, "XPT_DPCR7", XPT_DPCR7_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(3, XPT_PCR, "XPT_DPCR8", XPT_DPCR8_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(3, XPT_PCR, "XPT_DPCR9", XPT_DPCR9_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(3, XPT_PCR, "XPT_DPCR10", XPT_DPCR10_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(3, XPT_PCR, "XPT_DPCR11", XPT_DPCR11_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(3, XPT_PCR, "XPT_DPCR12", XPT_DPCR12_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(3, XPT_PCR, "XPT_DPCR13", XPT_DPCR13_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(3, XPT_PCR, "XPT_PCROFF0", XPT_PCROFFSET_INTERRUPT0_STATUS, REGULAR, ALL, 0),
    BINT_MAP(3, XPT_PCR, "XPT_PCROFF1", XPT_PCROFFSET_INTERRUPT1_STATUS, REGULAR, ALL, 0),
    BINT_MAP(3, XPT_PCR, "XPT_PCROFF2", XPT_PCROFFSET_INTERRUPT2_STATUS, REGULAR, ALL, 0),
    BINT_MAP(3, XPT_PCR, "XPT_PCROFF3", XPT_PCROFFSET_INTERRUPT3_STATUS, REGULAR, ALL, 0),
    BINT_MAP(3, XPT_PCR, "XPT_PCROFF_STC", XPT_PCROFFSET_STC_INTERRUPT_STATUS, REGULAR, ALL, 0),

    BINT_MAP_L3_ROOT(3, XPT_RAV, XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0),
        BINT_MAP_L3(3, XPT_RAV, XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0, RAVE_CDB_OVERFLOW_INTR, "_CX00_31", XPT_RAVE_CDB_OVERFLOW_CX00_31_L2_INTR_CPU_STATUS_0_31, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_RAV, XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0, RAVE_CDB_OVERFLOW_INTR, "_CX32_47", XPT_RAVE_CDB_OVERFLOW_CX32_47_L2_INTR_CPU_STATUS_32_47, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_RAV, XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0, RAVE_ITB_OVERFLOW_INTR, "_CX00_31", XPT_RAVE_ITB_OVERFLOW_CX00_31_L2_INTR_CPU_STATUS_0_31, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_RAV, XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0, RAVE_ITB_OVERFLOW_INTR, "_CX32_47", XPT_RAVE_ITB_OVERFLOW_CX32_47_L2_INTR_CPU_STATUS_32_47, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_RAV, XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0, RAVE_EMU_ERROR_INTR, "_CX00_31", XPT_RAVE_EMU_ERROR_CX00_31_L2_INTR_CPU_STATUS_0_31, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_RAV, XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0, RAVE_EMU_ERROR_INTR, "_CX32_47", XPT_RAVE_EMU_ERROR_CX32_47_L2_INTR_CPU_STATUS_32_47, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_RAV, XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0, RAVE_PUSI_ERROR_INTR, "_CX00_31", XPT_RAVE_PUSI_ERROR_CX00_31_L2_INTR_CPU_STATUS_0_31, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_RAV, XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0, RAVE_PUSI_ERROR_INTR, "_CX32_47", XPT_RAVE_PUSI_ERROR_CX32_47_L2_INTR_CPU_STATUS_32_47, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_RAV, XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0, RAVE_TEI_ERROR_INTR, "_CX00_31", XPT_RAVE_TEI_ERROR_CX00_31_L2_INTR_CPU_STATUS_0_31, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_RAV, XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0, RAVE_TEI_ERROR_INTR, "_CX32_47", XPT_RAVE_TEI_ERROR_CX32_47_L2_INTR_CPU_STATUS_32_47, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_RAV, XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0, RAVE_CC_ERROR_INTR, "_CX00_31", XPT_RAVE_CC_ERROR_CX00_31_L2_INTR_CPU_STATUS_0_31, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_RAV, XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0, RAVE_CC_ERROR_INTR, "_CX32_47", XPT_RAVE_CC_ERROR_CX32_47_L2_INTR_CPU_STATUS_32_47, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_RAV, XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0, RAVE_CDB_UPPER_THRESH_INTR, "_CX00_31", XPT_RAVE_CDB_UPPER_THRESH_CX00_31_L2_INTR_CPU_STATUS_0_31, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_RAV, XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0, RAVE_CDB_UPPER_THRESH_INTR, "_CX32_47", XPT_RAVE_CDB_UPPER_THRESH_CX32_47_L2_INTR_CPU_STATUS_32_47, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_RAV, XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0, RAVE_ITB_UPPER_THRESH_INTR, "_CX00_31", XPT_RAVE_ITB_UPPER_THRESH_CX00_31_L2_INTR_CPU_STATUS_0_31, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_RAV, XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0, RAVE_ITB_UPPER_THRESH_INTR, "_CX32_47", XPT_RAVE_ITB_UPPER_THRESH_CX32_47_L2_INTR_CPU_STATUS_32_47, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_RAV, XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0, RAVE_CDB_LOWER_THRESH_INTR, "_CX00_31", XPT_RAVE_CDB_LOWER_THRESH_CX00_31_L2_INTR_CPU_STATUS_0_31, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_RAV, XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0, RAVE_CDB_LOWER_THRESH_INTR, "_CX32_47", XPT_RAVE_CDB_LOWER_THRESH_CX32_47_L2_INTR_CPU_STATUS_32_47, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_RAV, XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0, RAVE_ITB_LOWER_THRESH_INTR, "_CX00_31", XPT_RAVE_ITB_LOWER_THRESH_CX00_31_L2_INTR_CPU_STATUS_0_31, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_RAV, XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0, RAVE_ITB_LOWER_THRESH_INTR, "_CX32_47", XPT_RAVE_ITB_LOWER_THRESH_CX32_47_L2_INTR_CPU_STATUS_32_47, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_RAV, XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0, RAVE_CDB_MIN_DEPTH_THRESH_INTR, "_CX00_31", XPT_RAVE_CDB_MIN_DEPTH_THRESH_CX00_31_L2_INTR_CPU_STATUS_0_31, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_RAV, XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0, RAVE_CDB_MIN_DEPTH_THRESH_INTR, "_CX32_47", XPT_RAVE_CDB_MIN_DEPTH_THRESH_CX32_47_L2_INTR_CPU_STATUS_32_47, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_RAV, XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0, RAVE_ITB_MIN_DEPTH_THRESH_INTR, "_CX00_31", XPT_RAVE_ITB_MIN_DEPTH_THRESH_CX00_31_L2_INTR_CPU_STATUS_0_31, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_RAV, XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0, RAVE_ITB_MIN_DEPTH_THRESH_INTR, "_CX32_47", XPT_RAVE_ITB_MIN_DEPTH_THRESH_CX32_47_L2_INTR_CPU_STATUS_32_47, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_RAV, XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0, RAVE_TSIO_DMA_END_INTR, "_CX00_31", XPT_RAVE_TSIO_DMA_END_CX00_31_L2_INTR_CPU_STATUS_0_31, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_RAV, XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0, RAVE_TSIO_DMA_END_INTR, "_CX32_47", XPT_RAVE_TSIO_DMA_END_CX32_47_L2_INTR_CPU_STATUS_32_47, STANDARD, ALL, 0),
    BINT_MAP(3, XPT_STATUS, "_BUS", XPT_BUS_IF_INTR_STATUS_REG,   REGULAR, ALL, 0),
    BINT_MAP(3, XPT_STATUS, "_BUS2", XPT_BUS_IF_INTR_STATUS2_REG, REGULAR, ALL, 0),
    BINT_MAP(3, XPT_STATUS, "_BUS3", XPT_BUS_IF_INTR_STATUS3_REG,   REGULAR, ALL, 0),
    BINT_MAP(3, XPT_STATUS, "_BUS4", XPT_BUS_IF_INTR_STATUS4_REG, REGULAR, ALL, 0),
    BINT_MAP(3, XPT_STATUS, "_WAKEUP", XPT_WAKEUP_INTR_STATUS_REG, REGULAR, ALL, 0),

    BINT_MAP_L3_ROOT(3, XPT_MCPB, XPT_MCPB_HOST_INTR_AGGREGATOR_INTR_W0),
        BINT_MAP_L3(3, XPT_MCPB, XPT_MCPB_HOST_INTR_AGGREGATOR_INTR_W0, MCPB_DESC_DONE_INTR, "", XPT_MCPB_DESC_DONE_INTR_L2_HOST_STATUS, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_MCPB, XPT_MCPB_HOST_INTR_AGGREGATOR_INTR_W0, MCPB_MISC_FALSE_WAKE_INTR, "", XPT_MCPB_MISC_FALSE_WAKE_INTR_L2_HOST_STATUS, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_MCPB, XPT_MCPB_HOST_INTR_AGGREGATOR_INTR_W0, MCPB_MISC_TEI_INTR, "", XPT_MCPB_MISC_TEI_INTR_L2_HOST_STATUS, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_MCPB, XPT_MCPB_HOST_INTR_AGGREGATOR_INTR_W0, MCPB_MISC_TS_RANGE_ERR_INTR, "", XPT_MCPB_MISC_TS_RANGE_ERR_INTR_L2_HOST_STATUS, STANDARD, ALL, 0),
        BINT_MAP_L3(3, XPT_MCPB, XPT_MCPB_HOST_INTR_AGGREGATOR_INTR_W0, MCPB_MISC_PAUSE_AT_DESC_END_INTR, "", XPT_MCPB_MISC_PAUSE_AT_DESC_END_INTR_L2_HOST_STATUS, STANDARD, ALL, 0),

    BINT_MAP_L3_ROOT(3, XPT_WMDMA, XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0),
        BINT_MAP_L3_STD(3, XPT_WMDMA, XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0, BTP, XPT_WDMA_BTP),
        BINT_MAP_L3_STD(3, XPT_WMDMA, XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0, OVERFLOW, XPT_WDMA_OVERFLOW),
        BINT_MAP_L3_STD(3, XPT_WMDMA, XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0, DESC_DONE, XPT_WDMA_DESC_DONE),

    BINT_MAP_STD(3, XPT_EXTCARD, XPT_TSIO_INTR_L2_CPU),

    BINT_MAP_LAST()
};
#endif

static const BINT_Settings bint_Settings =
{
    NULL,
    BINT_P_ClearInt,
    BINT_P_SetMask,
    BINT_P_ClearMask,
    NULL,
    BINT_P_ReadStatus,
    bint_map,
    "7278"
};


/* On some parts, the relative location of the status and enable regs changed. */
static uint32_t getXptFeIntEnableRegAddr( uint32_t baseAddr )
{
    uint32_t enableRegAddr = baseAddr + BINT_P_XPT_ENABLE;

    switch( baseAddr )
    {
#ifdef BCHP_XPT_FE_INTR_STATUS0_REG
        case BCHP_XPT_FE_INTR_STATUS0_REG:
            enableRegAddr = BCHP_XPT_FE_INTR_STATUS0_REG_EN;
            break;
#endif
#ifdef BCHP_XPT_FE_INTR_STATUS1_REG
        case BCHP_XPT_FE_INTR_STATUS1_REG:
            enableRegAddr = BCHP_XPT_FE_INTR_STATUS1_REG_EN;
            break;
#endif
#ifdef BCHP_XPT_FE_INTR_STATUS2_REG
        case BCHP_XPT_FE_INTR_STATUS2_REG:
            enableRegAddr = BCHP_XPT_FE_INTR_STATUS2_REG_EN;
            break;
#endif
        default:
            break;
    }

    return enableRegAddr;
}

static void BINT_P_ClearInt( BREG_Handle regHandle, uint32_t baseAddr, int shift )
{
    BDBG_MSG(("ClearInt %#x:%d", baseAddr, shift));
    switch( baseAddr )
    {
        BINT_P_XPT_STATUS_CASES
            BREG_Write32( regHandle, baseAddr + BINT_P_XPT_STATUS, ~(1ul<<shift));
            break;
        BINT_P_XPT_BUF_CASES
            BREG_Write32( regHandle, baseAddr + BINT_P_XPT_BUF_STATUS, ~(1ul<<shift));
            break;
        BINT_P_TIMER_CASES
            BREG_Write32( regHandle, baseAddr + BINT_P_TIMER_STATUS, 1ul<<shift);
            break;
        BINT_P_UPGSC_CASES
#ifdef  BINT_P_STD_RO_STATUS_CASES
        BINT_P_STD_RO_STATUS_CASES
#else
        BINT_P_IRQ0_CASES
        BINT_P_IRQ0_AON_CASES
#endif
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

static void BINT_P_SetMask( BREG_Handle regHandle, uint32_t baseAddr, int shift )
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
    BINT_P_XPT_BUF_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_XPT_BUF_ENABLE);
        intEnable &= ~(1ul<<shift);
        BREG_Write32( regHandle, baseAddr + BINT_P_XPT_BUF_ENABLE, intEnable);
        break;
    BINT_P_TIMER_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_TIMER_MASK);
        intEnable &= ~(1ul<<shift);
        BREG_Write32( regHandle, baseAddr + BINT_P_TIMER_MASK, intEnable);
        break;

#ifdef BINT_P_STD_RO_STATUS_CASES
    BINT_P_STD_RO_STATUS_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_STD_RO_STATUS_MASK_STATUS );
        intEnable |= 1ul<<shift;
        BREG_Write32( regHandle, baseAddr + BINT_P_STD_RO_STATUS_MASK_SET, intEnable );
        break;
#else
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
#endif
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
                 baseAddr, (void*)regHandle, shift));

        /* Unhandled interrupt base address */
        BDBG_ASSERT( false );
        break;
    }
}

static void BINT_P_ClearMask( BREG_Handle regHandle, uint32_t baseAddr, int shift )
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
    BINT_P_XPT_BUF_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_XPT_BUF_ENABLE);
        intEnable |= 1ul<<shift;
        BREG_Write32( regHandle, baseAddr + BINT_P_XPT_BUF_ENABLE, intEnable);
        break;

#ifdef BINT_P_STD_RO_STATUS_CASES
    BINT_P_STD_RO_STATUS_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_STD_RO_STATUS_MASK_STATUS );
        intEnable |= 1ul<<shift;
        BREG_Write32( regHandle, baseAddr + BINT_P_STD_RO_STATUS_MASK_CLEAR, intEnable );
        break;
#else
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
#endif
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


static uint32_t BINT_P_ReadStatus( BREG_Handle regHandle, uint32_t baseAddr )
{
    BDBG_MSG(("ReadStatus %#x", baseAddr));
    switch( baseAddr )
    {
    BINT_P_XPT_STATUS_CASES
        return BREG_Read32( regHandle, baseAddr + BINT_P_XPT_STATUS );
    BINT_P_XPT_BUF_CASES
        return BREG_Read32( regHandle, baseAddr + BINT_P_XPT_BUF_STATUS );
    BINT_P_TIMER_CASES
        return BREG_Read32( regHandle, baseAddr + BINT_P_TIMER_STATUS );
#ifdef BINT_P_STD_RO_STATUS_CASES
    BINT_P_STD_RO_STATUS_CASES
        return BREG_Read32( regHandle, baseAddr + BINT_P_STD_RO_STATUS_STATUS );
#else
    BINT_P_IRQ0_CASES
        return BREG_Read32( regHandle, baseAddr + BINT_P_IRQ0_STATUS );
    BINT_P_IRQ0_AON_CASES
        return BREG_Read32( regHandle, baseAddr + BINT_P_IRQ0_AON_STATUS );
#endif
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

const BINT_Settings *BINT_7278_GetSettings( void )
{
    return &bint_Settings;
}

/* End of file */

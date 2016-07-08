/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

#include "bchp_xpt_bus_if.h"
#include "bchp_xpt_bus_if_sub_module_soft_init_done_intr2.h"
#include "bchp_xpt_dpcr0.h"
#include "bchp_xpt_dpcr1.h"
#include "bchp_xpt_dpcr2.h"
#include "bchp_xpt_dpcr3.h"
#include "bchp_xpt_dpcr4.h"
#include "bchp_xpt_dpcr5.h"
#if (BXPT_NUM_PCRS > 6)
#include "bchp_xpt_dpcr6.h"
#include "bchp_xpt_dpcr7.h"
#include "bchp_xpt_dpcr8.h"
#include "bchp_xpt_dpcr9.h"
#include "bchp_xpt_dpcr10.h"
#include "bchp_xpt_dpcr11.h"
#include "bchp_xpt_dpcr12.h"
#include "bchp_xpt_dpcr13.h"
#endif
#include "bchp_xpt_dpcr_pp.h"
#include "bchp_xpt_fe.h"
#include "bchp_xpt_full_pid_parser.h"
#include "bchp_xpt_gr.h"
#include "bchp_xpt_mcpb.h"
#include "bchp_xpt_mcpb_ch0.h"
#include "bchp_xpt_mcpb_ch1.h"
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
#include "bchp_xpt_mcpb_misc_watchdog_timeout_or_asf_fatal_err_intr_l2.h"
#include "bchp_xpt_mcpb_pci_intr_aggregator.h"
#if BXPT_DMA_HAS_MEMDMA_MCPB
#include "bchp_xpt_memdma_mcpb.h"
#include "bchp_xpt_memdma_mcpb_ch0.h"
#include "bchp_xpt_memdma_mcpb_ch1.h"
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
#include "bchp_xpt_memdma_mcpb_misc_watchdog_timeout_or_asf_fatal_err_intr_l2.h"
#include "bchp_xpt_memdma_mcpb_pci_intr_aggregator.h"
#include "bchp_xpt_memdma_xmemif.h"
#endif
#include "bchp_xpt_mpod.h"
#include "bchp_xpt_msg.h"
#include "bchp_xpt_msg_buf_dat_rdy_cpu_intr_aggregator.h"
#include "bchp_xpt_msg_buf_dat_rdy_intr_00_31_l2.h"
#include "bchp_xpt_msg_buf_dat_rdy_intr_32_63_l2.h"
#include "bchp_xpt_msg_buf_dat_rdy_intr_64_95_l2.h"
#include "bchp_xpt_msg_buf_dat_rdy_intr_96_127_l2.h"
#if (BXPT_NUM_MESG_BUFFERS > 128)
#include "bchp_xpt_msg_buf_dat_rdy_intr_128_159_l2.h"
#include "bchp_xpt_msg_buf_dat_rdy_intr_160_191_l2.h"
#include "bchp_xpt_msg_buf_dat_rdy_intr_192_223_l2.h"
#include "bchp_xpt_msg_buf_dat_rdy_intr_224_255_l2.h"
#endif
#include "bchp_xpt_msg_buf_dat_rdy_pci_intr_aggregator.h"
#include "bchp_xpt_msg_buf_ovfl_cpu_intr_aggregator.h"
#include "bchp_xpt_msg_buf_ovfl_intr_00_31_l2.h"
#include "bchp_xpt_msg_buf_ovfl_intr_32_63_l2.h"
#include "bchp_xpt_msg_buf_ovfl_intr_64_95_l2.h"
#include "bchp_xpt_msg_buf_ovfl_intr_96_127_l2.h"
#if (BXPT_NUM_MESG_BUFFERS > 128)
#include "bchp_xpt_msg_buf_ovfl_intr_128_159_l2.h"
#include "bchp_xpt_msg_buf_ovfl_intr_160_191_l2.h"
#include "bchp_xpt_msg_buf_ovfl_intr_192_223_l2.h"
#include "bchp_xpt_msg_buf_ovfl_intr_224_255_l2.h"
#endif
#include "bchp_xpt_msg_buf_ovfl_pci_intr_aggregator.h"
#include "bchp_xpt_msg_dat_err_intr_l2.h"
#include "bchp_xpt_pcroffset.h"
#include "bchp_xpt_pmu.h"
#include "bchp_xpt_psub.h"
#include "bchp_xpt_rave.h"
#include "bchp_xpt_rave_cc_error_cx00_31_l2_intr.h"
#if (BXPT_NUM_RAVE_CONTEXTS > 24)
#include "bchp_xpt_rave_cc_error_cx32_47_l2_intr.h"
#include "bchp_xpt_rave_cdb_lower_thresh_cx32_47_l2_intr.h"
#include "bchp_xpt_rave_cdb_min_depth_thresh_cx32_47_l2_intr.h"
#include "bchp_xpt_rave_cdb_overflow_cx32_47_l2_intr.h"
#include "bchp_xpt_rave_cdb_upper_thresh_cx32_47_l2_intr.h"
#include "bchp_xpt_rave_emu_error_cx32_47_l2_intr.h"
#include "bchp_xpt_rave_fw_generic_1_cx32_47_l2_intr.h"
#include "bchp_xpt_rave_itb_lower_thresh_cx32_47_l2_intr.h"
#include "bchp_xpt_rave_itb_min_depth_thresh_cx32_47_l2_intr.h"
#include "bchp_xpt_rave_itb_overflow_cx32_47_l2_intr.h"
#include "bchp_xpt_rave_itb_upper_thresh_cx32_47_l2_intr.h"
#include "bchp_xpt_rave_last_cmd_cx32_47_l2_intr.h"
#include "bchp_xpt_rave_pusi_error_cx32_47_l2_intr.h"
#include "bchp_xpt_rave_splice_cx32_47_l2_intr.h"
#include "bchp_xpt_rave_tei_error_cx32_47_l2_intr.h"
#include "bchp_xpt_rave_to_scpu_l2_intr_32_47.h"
#include "bchp_xpt_rave_tsio_dma_end_cx32_47_l2_intr.h"
#endif
#include "bchp_xpt_rave_cdb_lower_thresh_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_cdb_min_depth_thresh_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_cdb_overflow_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_cdb_upper_thresh_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_cpu_intr_aggregator.h"
#include "bchp_xpt_rave_emu_error_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_fw_generic_1_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_itb_lower_thresh_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_itb_min_depth_thresh_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_itb_overflow_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_itb_upper_thresh_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_last_cmd_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_misc_l2_intr.h"
#include "bchp_xpt_rave_pci_intr_aggregator.h"
#include "bchp_xpt_rave_pusi_error_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_splice_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_tei_error_cx00_31_l2_intr.h"
#include "bchp_xpt_rave_to_scpu_l2_intr_0_31.h"
#include "bchp_xpt_rave_tsio_dma_end_cx00_31_l2_intr.h"
#include "bchp_xpt_rmx0.h"
#include "bchp_xpt_rmx0_io.h"
#include "bchp_xpt_rmx1.h"
#include "bchp_xpt_rmx1_io.h"
#include "bchp_xpt_rsbuff.h"
#include "bchp_xpt_secure_bus_if.h"
#include "bchp_xpt_tsio_calib_registers.h"
#include "bchp_xpt_tsio_config_registers.h"
#include "bchp_xpt_tsio_intr_l2.h"
#include "bchp_xpt_wakeup.h"
#include "bchp_xpt_wdma_btp_intr_l2.h"
#include "bchp_xpt_wdma_cpu_intr_aggregator.h"
#include "bchp_xpt_wdma_desc_done_intr_l2.h"
#include "bchp_xpt_wdma_overflow_intr_l2.h"
#include "bchp_xpt_wdma_pci_intr_aggregator.h"
#include "bchp_xpt_wdma_pm_control.h"
#include "bchp_xpt_wdma_pm_intr_l2.h"
#include "bchp_xpt_wdma_pm_results.h"
#include "bchp_xpt_wdma_regs.h"
#include "bchp_xpt_wdma_scpu_intr_aggregator.h"
#include "bchp_xpt_xcbuff.h"
#include "bchp_xpt_xmemif.h"
#include "bchp_xpt_xpu.h"

/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#include "bchp_xpt_bus_if.h"
#include "bchp_xpt_bus_if_sub_module_soft_init_done_intr2.h"
#include "bchp_xpt_dpcr0.h"
#include "bchp_xpt_dpcr1.h"
#if (BXPT_NUM_PCRS > 2)
#include "bchp_xpt_dpcr2.h"
#include "bchp_xpt_dpcr3.h"
#include "bchp_xpt_dpcr4.h"
#include "bchp_xpt_dpcr5.h"
#endif
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
#if BXPT_USE_HOST_AGGREGATOR
#include "bchp_xpt_mcpb_host_intr_aggregator.h"
#else
#include "bchp_xpt_mcpb_cpu_intr_aggregator.h"
#endif
#include "bchp_xpt_mcpb_pci_intr_aggregator.h"
#if BXPT_DMA_HAS_MEMDMA_MCPB
#include "bchp_xpt_memdma_mcpb.h"
#include "bchp_xpt_memdma_mcpb_ch0.h"
#include "bchp_xpt_memdma_mcpb_ch1.h"
#if BXPT_USE_HOST_AGGREGATOR
#include "bchp_xpt_memdma_mcpb_host_intr_aggregator.h"
#else
#include "bchp_xpt_memdma_mcpb_cpu_intr_aggregator.h"
#endif
#include "bchp_xpt_memdma_mcpb_pci_intr_aggregator.h"
#include "bchp_xpt_memdma_xmemif.h"
#endif
#include "bchp_xpt_mpod.h"
#if BXPT_HAS_MESG_BUFFERS
#include "bchp_xpt_msg.h"
#include "bchp_xpt_msg_buf_dat_rdy_cpu_intr_aggregator.h"
#include "bchp_xpt_msg_buf_dat_rdy_pci_intr_aggregator.h"
#include "bchp_xpt_msg_buf_ovfl_cpu_intr_aggregator.h"
#include "bchp_xpt_msg_buf_ovfl_pci_intr_aggregator.h"
#endif
#include "bchp_xpt_pcroffset.h"
#include "bchp_xpt_pmu.h"
#ifdef BCHP_XPT_PMU_HWG_CLK_GATE_SUB_MODULE_EN
#include "bchp_xpt_pmu_scb.h"
#endif
#if 0
#include "bchp_xpt_wdma.h"
#include "bchp_xpt_mcpb_bbuff.h"
#include "bchp_xpt_mcpb_escd_btp.h"
#include "bchp_xpt_mcpb_on_chip_desc_data.h"
#include "bchp_xpt_memdma_mcpb_bbuff.h"
#include "bchp_xpt_memdma_mcpb_escd_btp.h"
#include "bchp_xpt_memdma_mcpb_on_chip_desc_data.h"
#include "bchp_xpt_ocxc_top.h"
#endif
#if BXPT_HAS_PACKETSUB
#include "bchp_xpt_psub.h"
#endif
#include "bchp_xpt_rave.h"
#include "bchp_xpt_rave_cpu_intr_aggregator.h"
#if BXPT_HAS_REMUX
#include "bchp_xpt_rmx0.h"
#include "bchp_xpt_rmx0_io.h"
#include "bchp_xpt_rmx1.h"
#include "bchp_xpt_rmx1_io.h"
#endif
#include "bchp_xpt_rsbuff.h"
#include "bchp_xpt_secure_bus_if.h"
#include "bchp_xpt_tsio_calib_registers.h"
#include "bchp_xpt_tsio_config_registers.h"
#include "bchp_xpt_wakeup.h"
#include "bchp_xpt_wdma_cpu_intr_aggregator.h"
#include "bchp_xpt_wdma_pci_intr_aggregator.h"
#if BXPT_DMA_HAS_PERFORMANCE_METER
#include "bchp_xpt_wdma_pm_control.h"
#include "bchp_xpt_wdma_pm_results.h"
#endif
#include "bchp_xpt_wdma_regs.h"
#include "bchp_xpt_wdma_scpu_intr_aggregator.h"
#if BXPT_HAS_XCBUF_HW
#include "bchp_xpt_xcbuff.h"
#endif
#include "bchp_xpt_xmemif.h"
#include "bchp_xpt_xpu.h"

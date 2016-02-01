/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bxpt_priv.h"
#include "bxpt_pwr_mgmt_priv.h"

BDBG_MODULE(xpt_pwr_mgmt);

#ifdef BCHP_PWR_RESOURCE_XPT

#if (BCHP_CHIP==7231 && BCHP_VER>=BCHP_VER_B0)
#include "bchp_xpt_bus_if.h"
#include "bchp_xpt_dpcr0.h"
#include "bchp_xpt_dpcr1.h"
#include "bchp_xpt_dpcr2.h"
#include "bchp_xpt_dpcr3.h"
#include "bchp_xpt_dpcr_pp.h"
#include "bchp_xpt_fe.h"
#include "bchp_xpt_full_pid_parser.h"
#include "bchp_xpt_gr.h"
#include "bchp_xpt_mpod.h"
#include "bchp_xpt_msg.h"
#include "bchp_xpt_pb0.h"
#include "bchp_xpt_pb1.h"
#include "bchp_xpt_pb2.h"
#include "bchp_xpt_pb3.h"
#include "bchp_xpt_pb4.h"
#include "bchp_xpt_pb5.h"
#include "bchp_xpt_pb_top.h"
#include "bchp_xpt_pcroffset.h"
#include "bchp_xpt_pmu.h"
#include "bchp_xpt_psub.h"
#include "bchp_xpt_rave.h"
#include "bchp_xpt_rmx0.h"
#include "bchp_xpt_rmx0_io.h"
#include "bchp_xpt_rmx1.h"
#include "bchp_xpt_rmx1_io.h"
#include "bchp_xpt_rsbuff.h"
#include "bchp_xpt_secure_bus_if.h"
#include "bchp_xpt_wakeup.h"
#include "bchp_xpt_xcbuff.h"
#include "bchp_xpt_xmemif.h"
#include "bchp_xpt_xpu.h"

const BXPT_P_RegisterRange XPT_SRAM_LIST[] =
{
    { "WAKEUP_MEM",                              BCHP_XPT_WAKEUP_PKT_MEM_i_ARRAY_BASE,              (BCHP_XPT_WAKEUP_PKT_TYPE3_i_ARRAY_BASE+(BCHP_XPT_WAKEUP_PKT_TYPE3_i_ARRAY_END*4)) },
    { "FE_PID_TABLE",                            BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE,                (BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE+(BCHP_XPT_FE_PID_TABLE_i_ARRAY_END*4)) },
    { "FE_SPID_TABLE",                           BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE,               (BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE+(BCHP_XPT_FE_SPID_TABLE_i_ARRAY_END*4)) },
    { "PCROFFSET_PID_TABLE",                     BCHP_XPT_PCROFFSET_PID_CONFIG_TABLE_i_ARRAY_BASE,  (BCHP_XPT_PCROFFSET_PID_CONFIG_TABLE_i_ARRAY_BASE+(BCHP_XPT_PCROFFSET_PID_CONFIG_TABLE_i_ARRAY_END*4)) },
    { "PCROFFSET_CX_TABLE",                      BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL,        BCHP_XPT_PCROFFSET_CONTEXT15_RESERVED_CFG_0 },
    { "FPP_SCM0",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_END*4)) },
    { "FPP_SCM1",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_1_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_1_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_1_i_ARRAY_END*4)) },
    { "FPP_SCM2",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_2_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_2_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_2_i_ARRAY_END*4)) },
    { "FPP_SCM3",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_3_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_3_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_3_i_ARRAY_END*4)) },
    { "RSBUFF_IBP_PMEM",                         BCHP_XPT_RSBUFF_BASE_POINTER_IBP0,                 BCHP_XPT_RSBUFF_WATERMARK_IBP15 },
    { "RSBUFF_PBP_PMEM",                         BCHP_XPT_RSBUFF_BASE_POINTER_PBP0,                 BCHP_XPT_RSBUFF_WATERMARK_PBP5 },
    { "RSBUFF_MPOD_PMEM",                        BCHP_XPT_RSBUFF_BASE_POINTER_MPOD_IBP0,            BCHP_XPT_RSBUFF_WATERMARK_MPOD_IBP15 },
    { "XCBUFF_RAVE_IBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RAVE_IBP0,            BCHP_XPT_XCBUFF_WATERMARK_RAVE_IBP15 },
    { "XCBUFF_RAVE_PBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RAVE_PBP0,            BCHP_XPT_XCBUFF_WATERMARK_RAVE_PBP5 },
    { "XCBUFF_MSG_IBP_PMEM",                     BCHP_XPT_XCBUFF_BASE_POINTER_MSG_IBP0,             BCHP_XPT_XCBUFF_WATERMARK_MSG_IBP15 },
    { "XCBUFF_MSG_PBP_PMEM",                     BCHP_XPT_XCBUFF_BASE_POINTER_MSG_PBP0,             BCHP_XPT_XCBUFF_WATERMARK_MSG_PBP5 },
    { "XCBUFF_RMX0_IBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX0_IBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX0_IBP15 },
    { "XCBUFF_RMX0_PBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX0_PBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX0_PBP5 },
    { "XCBUFF_RMX1_IBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX1_IBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX1_IBP15 },
    { "XCBUFF_RMX1_PBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX1_PBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX1_PBP5 },
    { "MSG_PMEM",                                BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_ARRAY_BASE,         (BCHP_XPT_MSG_DMA_VP_TABLE_i_ARRAY_BASE+(BCHP_XPT_MSG_DMA_VP_TABLE_i_ARRAY_END*4)) },
    { "MSG_GEN_FILT1",                           BCHP_XPT_MSG_GEN_FILT_EN_i_ARRAY_BASE,             (BCHP_XPT_MSG_GEN_FILT_EN_i_ARRAY_BASE+(BCHP_XPT_MSG_GEN_FILT_EN_i_ARRAY_END*4)) },
    { "MSG_GEN_FILT2",                           BCHP_XPT_MSG_MCAST_16_ADDR_i_ARRAY_BASE,           (BCHP_XPT_MSG_GEN_FILT_EXCL_i_ARRAY_BASE+(BCHP_XPT_MSG_GEN_FILT_EXCL_i_ARRAY_END*4)) },
    { "MSG_PID2BUF",                             BCHP_XPT_MSG_PID2BUF_MAP_i_ARRAY_BASE,             (BCHP_XPT_MSG_PID2BUF_MAP_i_ARRAY_BASE+(BCHP_XPT_MSG_PID2BUF_MAP_i_ARRAY_END*4)) },
    { "RAVE_PMEM",                               BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR,                BCHP_XPT_RAVE_SCD47_RESERVED_STATE3 },
    { "RAVE_CXMEM_A",                            BCHP_XPT_RAVE_CXMEM_Ai_ARRAY_BASE,                 (BCHP_XPT_RAVE_CXMEM_Ai_ARRAY_BASE+(BCHP_XPT_RAVE_CXMEM_Ai_ARRAY_END*4)) },
    { "RAVE_CXMEM_B",                            BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_BASE,                 (BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_BASE+(BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_END*4)) },
    { "RAVE_DMEM",                               BCHP_XPT_RAVE_DMEMi_ARRAY_BASE,                    (BCHP_XPT_RAVE_DMEMi_ARRAY_BASE+(BCHP_XPT_RAVE_DMEMi_ARRAY_END*4)) },
    { "RAVE_SMEM",                               BCHP_XPT_RAVE_TPIT0_PID_TABLEi_ARRAY_BASE,         BCHP_XPT_RAVE_TPIT_STATE_CONTEXT23 },
    {0, 0, 0}
};

const BXPT_P_RegisterRange XPT_REG_SAVE_LIST[] =
{
    { "XPT_BUS_IF_MISC_CTRL0",                   BCHP_XPT_BUS_IF_MISC_CTRL0,                        BCHP_XPT_BUS_IF_TEST_MODE },
    { "XPT_BUS_IF_INTR_STATUS_REG_EN",           BCHP_XPT_BUS_IF_INTR_STATUS_REG_EN,                BCHP_XPT_BUS_IF_INTR_STATUS_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS2_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS2_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS2_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS3_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS3_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS3_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS4_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS4_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS4_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS5_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS5_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS5_REG_EN },
    { "XPT_XMEMIF_SCB_WR_ARB_SEL_RSBUFF",        BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RSBUFF,             BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB5 },
    { "XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB",          BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB,               BCHP_XPT_XMEMIF_SCB_RD_ARB_MODE },
    { "XPT_XMEMIF_INTR_STATUS_REG_EN",           BCHP_XPT_XMEMIF_INTR_STATUS_REG_EN,                BCHP_XPT_XMEMIF_INTR_STATUS_REG_EN },
    { "XPT_PMU_CLK_CTRL",                        BCHP_XPT_PMU_CLK_CTRL,                             BCHP_XPT_PMU_RBUS_RSP_VAL },
    { "XPT_GR_CTRL",                             BCHP_XPT_GR_CTRL,                                  BCHP_XPT_GR_CTRL },
    { "XPT_RMX0_IO_FORMAT",                      BCHP_XPT_RMX0_IO_FORMAT,                           BCHP_XPT_RMX0_IO_PKT_DLY_CNT },
    { "XPT_RMX1_IO_FORMAT",                      BCHP_XPT_RMX1_IO_FORMAT,                           BCHP_XPT_RMX1_IO_PKT_DLY_CNT },
    { "XPT_RMX1_IO_TV_STATUS",                   BCHP_XPT_RMX1_IO_TV_STATUS,                        BCHP_XPT_RMX1_IO_TV_STATUS },
    { "XPT_WAKEUP_CTRL",                         BCHP_XPT_WAKEUP_CTRL,                              BCHP_XPT_WAKEUP_CTRL },
    { "XPT_WAKEUP_INTR_STATUS_REG_EN",           BCHP_XPT_WAKEUP_INTR_STATUS_REG_EN,                BCHP_XPT_WAKEUP_INTR_STATUS_REG_EN },
    { "XPT_FE_FE_CTRL",                          BCHP_XPT_FE_FE_CTRL,                               BCHP_XPT_FE_FE_CTRL },
    { "XPT_FE_INTR_STATUS0_REG_EN",              BCHP_XPT_FE_INTR_STATUS0_REG_EN,                   BCHP_XPT_FE_PWR_CTRL },
    { "XPT_FE_MAX_PID_CHANNEL",                  BCHP_XPT_FE_MAX_PID_CHANNEL,                       BCHP_XPT_FE_MAX_PID_CHANNEL },
    { "XPT_FE_IB0_CTRL",                         BCHP_XPT_FE_IB0_CTRL,                              BCHP_XPT_FE_IB0_CTRL },
    { "XPT_FE_IB1_CTRL",                         BCHP_XPT_FE_IB1_CTRL,                              BCHP_XPT_FE_IB1_CTRL },
    { "XPT_FE_IB2_CTRL",                         BCHP_XPT_FE_IB2_CTRL,                              BCHP_XPT_FE_IB2_CTRL },
    { "XPT_FE_IB3_CTRL",                         BCHP_XPT_FE_IB3_CTRL,                              BCHP_XPT_FE_IB3_CTRL },
    { "XPT_FE_IB4_CTRL",                         BCHP_XPT_FE_IB4_CTRL,                              BCHP_XPT_FE_IB4_CTRL },
    { "XPT_FE_IB5_CTRL",                         BCHP_XPT_FE_IB5_CTRL,                              BCHP_XPT_FE_IB5_CTRL },
    { "XPT_FE_IB6_CTRL",                         BCHP_XPT_FE_IB6_CTRL,                              BCHP_XPT_FE_IB6_CTRL },
    { "XPT_FE_IB7_CTRL",                         BCHP_XPT_FE_IB7_CTRL,                              BCHP_XPT_FE_IB7_CTRL },
    { "XPT_FE_IB8_CTRL",                         BCHP_XPT_FE_IB8_CTRL,                              BCHP_XPT_FE_IB8_CTRL },
    { "XPT_FE_IB9_CTRL",                         BCHP_XPT_FE_IB9_CTRL,                              BCHP_XPT_FE_IB9_CTRL },
    { "XPT_FE_IB10_CTRL",                        BCHP_XPT_FE_IB10_CTRL,                             BCHP_XPT_FE_IB10_CTRL },
    { "XPT_FE_MINI_PID_PARSER0_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER0_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER0_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER1_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER1_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER1_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER2_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER2_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER2_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER3_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER3_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER3_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER4_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER4_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER4_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER5_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER5_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER5_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER6_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER6_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER6_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER7_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER7_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER7_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER8_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER8_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER8_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER9_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER9_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER9_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER10_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER10_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER10_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER11_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER11_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER11_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER12_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER12_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER12_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER13_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER13_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER13_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER14_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER14_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER14_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER15_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER15_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER15_CTRL2 },
    { "XPT_FE_IB_SYNC_DETECT_CTRL",              BCHP_XPT_FE_IB_SYNC_DETECT_CTRL,                   BCHP_XPT_FE_IB_SYNC_DETECT_CTRL },
    { "XPT_FE_TSMF0_CTRL",                       BCHP_XPT_FE_TSMF0_CTRL,                            BCHP_XPT_FE_TSMF0_SLOT_MAP_HI },
    { "XPT_FE_TSMF1_CTRL",                       BCHP_XPT_FE_TSMF1_CTRL,                            BCHP_XPT_FE_TSMF1_SLOT_MAP_HI },
    { "XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID", BCHP_XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID,   BCHP_XPT_FE_MINI_PID_PARSER12_TO_PARSER15_BAND_ID },
    { "XPT_FE_ATS_COUNTER_CTRL",                 BCHP_XPT_FE_ATS_COUNTER_CTRL,                      BCHP_XPT_FE_ATS_TS_BINARY },
    { "XPT_FE_MTSIF_RX0_CTRL1",                  BCHP_XPT_FE_MTSIF_RX0_CTRL1,                       BCHP_XPT_FE_MTSIF_RX0_CTRL1 },
    { "XPT_FE_MTSIF_RX0_INTR_STATUS0_REG_EN",    BCHP_XPT_FE_MTSIF_RX0_INTR_STATUS0_REG_EN,         BCHP_XPT_FE_MTSIF_RX0_BAND_ID_DROP },
    { "XPT_FE_MTSIF_RX1_CTRL1",                  BCHP_XPT_FE_MTSIF_RX1_CTRL1,                       BCHP_XPT_FE_MTSIF_RX1_CTRL1 },
    { "XPT_FE_MTSIF_RX1_INTR_STATUS0_REG_EN",    BCHP_XPT_FE_MTSIF_RX1_INTR_STATUS0_REG_EN,         BCHP_XPT_FE_MTSIF_RX1_BAND_ID_DROP },
    { "XPT_DPCR0_PID_CH",                        BCHP_XPT_DPCR0_PID_CH,                             BCHP_XPT_DPCR0_CTRL },
    { "XPT_DPCR0_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR0_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR0_STC_EXT_CTRL },
    { "XPT_DPCR0_MAX_PCR_ERROR",                 BCHP_XPT_DPCR0_MAX_PCR_ERROR,                      BCHP_XPT_DPCR0_MAX_PCR_ERROR },
    { "XPT_DPCR0_STC_HI",                        BCHP_XPT_DPCR0_STC_HI,                             BCHP_XPT_DPCR0_STC_LO },
    { "XPT_DPCR0_LOOP_CTRL",                     BCHP_XPT_DPCR0_LOOP_CTRL,                          BCHP_XPT_DPCR0_ACCUM_VALUE },
    { "XPT_DPCR0_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR0_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR0_PHASE_ERROR_CLAMP },
    { "XPT_DPCR1_PID_CH",                        BCHP_XPT_DPCR1_PID_CH,                             BCHP_XPT_DPCR1_CTRL },
    { "XPT_DPCR1_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR1_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR1_STC_EXT_CTRL },
    { "XPT_DPCR1_MAX_PCR_ERROR",                 BCHP_XPT_DPCR1_MAX_PCR_ERROR,                      BCHP_XPT_DPCR1_MAX_PCR_ERROR },
    { "XPT_DPCR1_STC_HI",                        BCHP_XPT_DPCR1_STC_HI,                             BCHP_XPT_DPCR1_STC_LO },
    { "XPT_DPCR1_LOOP_CTRL",                     BCHP_XPT_DPCR1_LOOP_CTRL,                          BCHP_XPT_DPCR1_ACCUM_VALUE },
    { "XPT_DPCR1_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR1_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR1_PHASE_ERROR_CLAMP },
    { "XPT_DPCR2_PID_CH",                        BCHP_XPT_DPCR2_PID_CH,                             BCHP_XPT_DPCR2_CTRL },
    { "XPT_DPCR2_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR2_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR2_STC_EXT_CTRL },
    { "XPT_DPCR2_MAX_PCR_ERROR",                 BCHP_XPT_DPCR2_MAX_PCR_ERROR,                      BCHP_XPT_DPCR2_MAX_PCR_ERROR },
    { "XPT_DPCR2_STC_HI",                        BCHP_XPT_DPCR2_STC_HI,                             BCHP_XPT_DPCR2_STC_LO },
    { "XPT_DPCR2_LOOP_CTRL",                     BCHP_XPT_DPCR2_LOOP_CTRL,                          BCHP_XPT_DPCR2_ACCUM_VALUE },
    { "XPT_DPCR2_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR2_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR2_PHASE_ERROR_CLAMP },
    { "XPT_DPCR3_PID_CH",                        BCHP_XPT_DPCR3_PID_CH,                             BCHP_XPT_DPCR3_CTRL },
    { "XPT_DPCR3_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR3_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR3_STC_EXT_CTRL },
    { "XPT_DPCR3_MAX_PCR_ERROR",                 BCHP_XPT_DPCR3_MAX_PCR_ERROR,                      BCHP_XPT_DPCR3_MAX_PCR_ERROR },
    { "XPT_DPCR3_STC_HI",                        BCHP_XPT_DPCR3_STC_HI,                             BCHP_XPT_DPCR3_STC_LO },
    { "XPT_DPCR3_LOOP_CTRL",                     BCHP_XPT_DPCR3_LOOP_CTRL,                          BCHP_XPT_DPCR3_ACCUM_VALUE },
    { "XPT_DPCR3_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR3_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR3_PHASE_ERROR_CLAMP },
    { "XPT_DPCR_PP_PP_CTRL",                     BCHP_XPT_DPCR_PP_PP_CTRL,                          BCHP_XPT_DPCR_PP_PP_FIXED_OFFSET },
    { "XPT_PSUB_PSUB0_CTRL0",                    BCHP_XPT_PSUB_PSUB0_CTRL0,                         BCHP_XPT_PSUB_PSUB0_STAT0 },
    { "XPT_PSUB_PSUB1_CTRL0",                    BCHP_XPT_PSUB_PSUB1_CTRL0,                         BCHP_XPT_PSUB_PSUB1_STAT0 },
    { "XPT_PSUB_PSUB2_CTRL0",                    BCHP_XPT_PSUB_PSUB2_CTRL0,                         BCHP_XPT_PSUB_PSUB2_STAT0 },
    { "XPT_PSUB_PSUB3_CTRL0",                    BCHP_XPT_PSUB_PSUB3_CTRL0,                         BCHP_XPT_PSUB_PSUB3_STAT0 },
    { "XPT_PSUB_PSUB4_CTRL0",                    BCHP_XPT_PSUB_PSUB4_CTRL0,                         BCHP_XPT_PSUB_PSUB4_STAT0 },
    { "XPT_PSUB_PSUB5_CTRL0",                    BCHP_XPT_PSUB_PSUB5_CTRL0,                         BCHP_XPT_PSUB_PSUB5_STAT0 },
    { "XPT_PSUB_PSUB6_CTRL0",                    BCHP_XPT_PSUB_PSUB6_CTRL0,                         BCHP_XPT_PSUB_PSUB6_STAT0 },
    { "XPT_PSUB_PSUB7_CTRL0",                    BCHP_XPT_PSUB_PSUB7_CTRL0,                         BCHP_XPT_PSUB_PSUB7_STAT0 },
    { "XPT_PSUB_PR_FALLBACK_CTRL",               BCHP_XPT_PSUB_PR_FALLBACK_CTRL,                    BCHP_XPT_PSUB_PR_FALLBACK_CTRL },
    { "XPT_MPOD_CFG",                            BCHP_XPT_MPOD_CFG,                                 BCHP_XPT_MPOD_ICTRL },
    { "XPT_MPOD_OCTRL2",                         BCHP_XPT_MPOD_OCTRL2,                              BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP },
    { "XPT_RMX0_CTRL",                           BCHP_XPT_RMX0_CTRL,                                BCHP_XPT_RMX0_FIXED_OFFSET },
    { "XPT_RMX1_CTRL",                           BCHP_XPT_RMX1_CTRL,                                BCHP_XPT_RMX1_FIXED_OFFSET },
    { "XPT_PB_TOP_PACING_COUNT_WR_VALUE",        BCHP_XPT_PB_TOP_PACING_COUNT_WR_VALUE,             BCHP_XPT_PB_TOP_BAND_PAUSE_MAP_VECT_PB5 },
    { "XPT_PB_TOP_PACING_PAUSE_MAP_VECT_PB0",    BCHP_XPT_PB_TOP_PACING_PAUSE_MAP_VECT_PB0,         BCHP_XPT_PB_TOP_PACING_PAUSE_MAP_VECT_PB5 },
    { "XPT_PB0_CTRL1",                           BCHP_XPT_PB0_CTRL1,                                BCHP_XPT_PB0_FIRST_DESC_ADDR },
    { "XPT_PB0_BLOCKOUT",                        BCHP_XPT_PB0_BLOCKOUT,                             BCHP_XPT_PB0_PARSER_CTRL2 },
    { "XPT_PB0_INTR_EN",                         BCHP_XPT_PB0_INTR_EN,                              BCHP_XPT_PB0_PWR_CTRL },
    { "XPT_PB0_ASF_CTRL",                        BCHP_XPT_PB0_ASF_CTRL,                             BCHP_XPT_PB0_PES_BASED_PACING_CTRL },
    { "XPT_PB1_CTRL1",                           BCHP_XPT_PB1_CTRL1,                                BCHP_XPT_PB1_FIRST_DESC_ADDR },
    { "XPT_PB1_BLOCKOUT",                        BCHP_XPT_PB1_BLOCKOUT,                             BCHP_XPT_PB1_PARSER_CTRL2 },
    { "XPT_PB1_INTR_EN",                         BCHP_XPT_PB1_INTR_EN,                              BCHP_XPT_PB1_PWR_CTRL },
    { "XPT_PB1_ASF_CTRL",                        BCHP_XPT_PB1_ASF_CTRL,                             BCHP_XPT_PB1_PES_BASED_PACING_CTRL },
    { "XPT_PB2_CTRL1",                           BCHP_XPT_PB2_CTRL1,                                BCHP_XPT_PB2_FIRST_DESC_ADDR },
    { "XPT_PB2_BLOCKOUT",                        BCHP_XPT_PB2_BLOCKOUT,                             BCHP_XPT_PB2_PARSER_CTRL2 },
    { "XPT_PB2_INTR_EN",                         BCHP_XPT_PB2_INTR_EN,                              BCHP_XPT_PB2_PWR_CTRL },
    { "XPT_PB2_ASF_CTRL",                        BCHP_XPT_PB2_ASF_CTRL,                             BCHP_XPT_PB2_PES_BASED_PACING_CTRL },
    { "XPT_PB3_CTRL1",                           BCHP_XPT_PB3_CTRL1,                                BCHP_XPT_PB3_FIRST_DESC_ADDR },
    { "XPT_PB3_BLOCKOUT",                        BCHP_XPT_PB3_BLOCKOUT,                             BCHP_XPT_PB3_PARSER_CTRL2 },
    { "XPT_PB3_INTR_EN",                         BCHP_XPT_PB3_INTR_EN,                              BCHP_XPT_PB3_PWR_CTRL },
    { "XPT_PB3_ASF_CTRL",                        BCHP_XPT_PB3_ASF_CTRL,                             BCHP_XPT_PB3_PES_BASED_PACING_CTRL },
    { "XPT_PB4_CTRL1",                           BCHP_XPT_PB4_CTRL1,                                BCHP_XPT_PB4_FIRST_DESC_ADDR },
    { "XPT_PB4_BLOCKOUT",                        BCHP_XPT_PB4_BLOCKOUT,                             BCHP_XPT_PB4_PARSER_CTRL2 },
    { "XPT_PB4_INTR_EN",                         BCHP_XPT_PB4_INTR_EN,                              BCHP_XPT_PB4_PWR_CTRL },
    { "XPT_PB4_ASF_CTRL",                        BCHP_XPT_PB4_ASF_CTRL,                             BCHP_XPT_PB4_PES_BASED_PACING_CTRL },
    { "XPT_PB5_CTRL1",                           BCHP_XPT_PB5_CTRL1,                                BCHP_XPT_PB5_FIRST_DESC_ADDR },
    { "XPT_PB5_BLOCKOUT",                        BCHP_XPT_PB5_BLOCKOUT,                             BCHP_XPT_PB5_PARSER_CTRL2 },
    { "XPT_PB5_INTR_EN",                         BCHP_XPT_PB5_INTR_EN,                              BCHP_XPT_PB5_PWR_CTRL },
    { "XPT_PB5_ASF_CTRL",                        BCHP_XPT_PB5_ASF_CTRL,                             BCHP_XPT_PB5_PES_BASED_PACING_CTRL },
    { "XPT_PCROFFSET_INTERRUPT0_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT0_ENABLE,              BCHP_XPT_PCROFFSET_INTERRUPT0_ENABLE },
    { "XPT_PCROFFSET_INTERRUPT1_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT1_ENABLE,              BCHP_XPT_PCROFFSET_INTERRUPT1_ENABLE },
    { "XPT_PCROFFSET_INTERRUPT2_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT2_ENABLE,              BCHP_XPT_PCROFFSET_INTERRUPT2_ENABLE },
    { "XPT_PCROFFSET_INTERRUPT3_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT3_ENABLE,              BCHP_XPT_PCROFFSET_STC_MUX_DELAY },
    { "XPT_PCROFFSET_STC_INTERRUPT_ENABLE",      BCHP_XPT_PCROFFSET_STC_INTERRUPT_ENABLE,           BCHP_XPT_PCROFFSET_STC7_MATCH },
    { "XPT_PCROFFSET_STC0_CTRL",                 BCHP_XPT_PCROFFSET_STC0_CTRL,                      BCHP_XPT_PCROFFSET_STC0_INC_LO },
    { "XPT_PCROFFSET_STC0_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC0_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC0_AV_WINDOW },
    { "XPT_PCROFFSET_STC1_CTRL",                 BCHP_XPT_PCROFFSET_STC1_CTRL,                      BCHP_XPT_PCROFFSET_STC1_INC_LO },
    { "XPT_PCROFFSET_STC1_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC1_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC1_AV_WINDOW },
    { "XPT_PCROFFSET_STC2_CTRL",                 BCHP_XPT_PCROFFSET_STC2_CTRL,                      BCHP_XPT_PCROFFSET_STC2_INC_LO },
    { "XPT_PCROFFSET_STC2_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC2_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC2_AV_WINDOW },
    { "XPT_PCROFFSET_STC3_CTRL",                 BCHP_XPT_PCROFFSET_STC3_CTRL,                      BCHP_XPT_PCROFFSET_STC3_INC_LO },
    { "XPT_PCROFFSET_STC3_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC3_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC3_AV_WINDOW },
    { "XPT_PCROFFSET_STC4_CTRL",                 BCHP_XPT_PCROFFSET_STC4_CTRL,                      BCHP_XPT_PCROFFSET_STC4_INC_LO },
    { "XPT_PCROFFSET_STC4_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC4_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC4_AV_WINDOW },
    { "XPT_PCROFFSET_STC5_CTRL",                 BCHP_XPT_PCROFFSET_STC5_CTRL,                      BCHP_XPT_PCROFFSET_STC5_INC_LO },
    { "XPT_PCROFFSET_STC5_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC5_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC5_AV_WINDOW },
    { "XPT_PCROFFSET_STC6_CTRL",                 BCHP_XPT_PCROFFSET_STC6_CTRL,                      BCHP_XPT_PCROFFSET_STC6_INC_LO },
    { "XPT_PCROFFSET_STC6_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC6_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC6_AV_WINDOW },
    { "XPT_PCROFFSET_STC7_CTRL",                 BCHP_XPT_PCROFFSET_STC7_CTRL,                      BCHP_XPT_PCROFFSET_STC7_INC_LO },
    { "XPT_PCROFFSET_STC7_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC7_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC7_AV_WINDOW },
    { "XPT_FULL_PID_PARSER_PARSER_CFG",          BCHP_XPT_FULL_PID_PARSER_PARSER_CFG,               BCHP_XPT_FULL_PID_PARSER_PBP_ACCEPT_ADAPT_00 },
    { "XPT_FULL_PID_PARSER_IBP_PCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_PCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_PCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_PCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_PCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_PCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_IBP_SCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_SCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_SCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_SCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_SCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_SCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_IBP_PSG_PROTOCOL_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_PSG_PROTOCOL_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_PSG_PROTOCOL_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_PSG_PROTOCOL_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_PSG_PROTOCOL_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_PSG_PROTOCOL_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_IBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PSG_ERR_MODE",        BCHP_XPT_FULL_PID_PARSER_PSG_ERR_MODE,             BCHP_XPT_FULL_PID_PARSER_PSG_ERR_MODE },
    { "XPT_RSBUFF_BO_IBP0",                      BCHP_XPT_RSBUFF_BO_IBP0,                           BCHP_XPT_RSBUFF_BO_IBP15 },
    { "XPT_RSBUFF_BO_PBP0",                      BCHP_XPT_RSBUFF_BO_PBP0,                           BCHP_XPT_RSBUFF_BO_PBP5 },
    { "XPT_RSBUFF_BO_MPOD_IBP0",                 BCHP_XPT_RSBUFF_BO_MPOD_IBP0,                      BCHP_XPT_RSBUFF_BO_MPOD_IBP15 },
    { "XPT_RSBUFF_IBP_BUFFER_ENABLE",            BCHP_XPT_RSBUFF_IBP_BUFFER_ENABLE,                 BCHP_XPT_RSBUFF_PBP_BUFFER_ENABLE },
    { "XPT_RSBUFF_MPOD_IBP_BUFFER_ENABLE",       BCHP_XPT_RSBUFF_MPOD_IBP_BUFFER_ENABLE,            BCHP_XPT_RSBUFF_MPOD_IBP_BUFFER_ENABLE },
    { "XPT_RSBUFF_OVERFLOW_THRESHOLD",           BCHP_XPT_RSBUFF_OVERFLOW_THRESHOLD,                BCHP_XPT_RSBUFF_NO_RD_HANG_CTRL },
    { "XPT_RSBUFF_PR_FALLBACK_CTRL",             BCHP_XPT_RSBUFF_PR_FALLBACK_CTRL,                  BCHP_XPT_RSBUFF_PR_FALLBACK_CTRL },
    { "XPT_XCBUFF_RAVE_CTRL_BUFFER_EN_PBP",      BCHP_XPT_XCBUFF_RAVE_CTRL_BUFFER_EN_PBP,           BCHP_XPT_XCBUFF_PAUSE_THRESHOLD },
    { "XPT_XCBUFF_OVERFLOW_THRESHOLD",           BCHP_XPT_XCBUFF_OVERFLOW_THRESHOLD,                BCHP_XPT_XCBUFF_OVERFLOW_THRESHOLD },
    { "XPT_XCBUFF_BYTE_SWAP_CTRL",               BCHP_XPT_XCBUFF_BYTE_SWAP_CTRL,                    BCHP_XPT_XCBUFF_BO_RAVE_IBP15 },
    { "XPT_XCBUFF_BO_RAVE_PBP0",                 BCHP_XPT_XCBUFF_BO_RAVE_PBP0,                      BCHP_XPT_XCBUFF_BO_RAVE_PBP5 },
    { "XPT_XCBUFF_BO_MSG_IBP0",                  BCHP_XPT_XCBUFF_BO_MSG_IBP0,                       BCHP_XPT_XCBUFF_BO_MSG_IBP15 },
    { "XPT_XCBUFF_BO_MSG_PBP0",                  BCHP_XPT_XCBUFF_BO_MSG_PBP0,                       BCHP_XPT_XCBUFF_BO_MSG_PBP5 },
    { "XPT_XCBUFF_BO_RMX0_IBP0",                 BCHP_XPT_XCBUFF_BO_RMX0_IBP0,                      BCHP_XPT_XCBUFF_BO_RMX0_IBP15 },
    { "XPT_XCBUFF_BO_RMX0_PBP0",                 BCHP_XPT_XCBUFF_BO_RMX0_PBP0,                      BCHP_XPT_XCBUFF_BO_RMX0_PBP5 },
    { "XPT_XCBUFF_BO_RMX1_IBP0",                 BCHP_XPT_XCBUFF_BO_RMX1_IBP0,                      BCHP_XPT_XCBUFF_BO_RMX1_IBP15 },
    { "XPT_XCBUFF_BO_RMX1_PBP0",                 BCHP_XPT_XCBUFF_BO_RMX1_PBP0,                      BCHP_XPT_XCBUFF_BO_RMX1_PBP5 },
    { "XPT_XCBUFF_MISC_CTRL",                    BCHP_XPT_XCBUFF_MISC_CTRL,                         BCHP_XPT_XCBUFF_NO_RD_HANG_CTRL },
    { "XPT_XCBUFF_PR_FALLBACK_CTRL",             BCHP_XPT_XCBUFF_PR_FALLBACK_CTRL,                  BCHP_XPT_XCBUFF_PR_FALLBACK_CTRL },
    { "XPT_MSG_MSG_CTRL1",                       BCHP_XPT_MSG_MSG_CTRL1,                            BCHP_XPT_MSG_DMA_BUFFER_INIT },
    { "XPT_MSG_PHY_ADDR_LO",                     BCHP_XPT_MSG_PHY_ADDR_LO,                          BCHP_XPT_MSG_ID_REJECT },
    { "XPT_MSG_C_MATCH0",                        BCHP_XPT_MSG_C_MATCH0,                             BCHP_XPT_MSG_C_MATCH4 },
    { "XPT_MSG_BUF_DAT_RDY_INTR_EN_00_31",       BCHP_XPT_MSG_BUF_DAT_RDY_INTR_EN_00_31,            BCHP_XPT_MSG_BUF_DAT_RDY_INTR_EN_96_127 },
    { "XPT_MSG_BUF_OVFL_INTR_EN_00_31",          BCHP_XPT_MSG_BUF_OVFL_INTR_EN_00_31,               BCHP_XPT_MSG_BUF_OVFL_INTR_EN_96_127 },
    { "XPT_MSG_DAT_ERR_INTR_EN",                 BCHP_XPT_MSG_DAT_ERR_INTR_EN,                      BCHP_XPT_MSG_DAT_ERR_INTR_EN },
    { "XPT_MSG_MSG_EVENT_CNT_CTRL",              BCHP_XPT_MSG_MSG_EVENT_CNT_CTRL,                   BCHP_XPT_MSG_MSG_EVENT_CNT_CTRL },
    { "XPT_RAVE_AVS_SCV_FILTER_MODE_CONTROL",    BCHP_XPT_RAVE_AVS_SCV_FILTER_MODE_CONTROL,         BCHP_XPT_RAVE_AVS_SCV_FILTER_VALUE_4_TO_7 },
    { "XPT_RAVE_DATA_START_ADDR_A",              BCHP_XPT_RAVE_DATA_START_ADDR_A,                   BCHP_XPT_RAVE_WATCHDOG_TIMER_VALUE },
    { "XPT_RAVE_MISC_CONTROL",                   BCHP_XPT_RAVE_MISC_CONTROL,                        BCHP_XPT_RAVE_BASE_ADDRESSES },
    { "XPT_RAVE_MISC_CONTROL2",                  BCHP_XPT_RAVE_MISC_CONTROL2,                       BCHP_XPT_RAVE_MISC_CONTROL3 },
    { "XPT_RAVE_RC0_SP_CONTROL",                 BCHP_XPT_RAVE_RC0_SP_CONTROL,                      BCHP_XPT_RAVE_RC8_SP_CONTROL },
    { "XPT_RAVE_ATSOFFSET_MAX_ERROR",            BCHP_XPT_RAVE_ATSOFFSET_MAX_ERROR,                 BCHP_XPT_RAVE_ATSOFFSET_MAX_ERROR },
    { "XPT_RAVE_TPIT_TIME_TICK",                 BCHP_XPT_RAVE_TPIT_TIME_TICK,                      BCHP_XPT_RAVE_TPIT_EVE_TIMEOUT },
    { "XPT_RAVE_EMM_TID_MODE",                   BCHP_XPT_RAVE_EMM_TID_MODE,                        BCHP_XPT_RAVE_EMM_CTRL_ID },
    { "XPT_RAVE_EMM_DATA_ID_1",                  BCHP_XPT_RAVE_EMM_DATA_ID_1,                       BCHP_XPT_RAVE_EMM_MASK_ID_8 },
    {0, 0, 0}
};

#elif (BCHP_CHIP==7584) || (BCHP_CHIP == 75845)
#include "bchp_xpt_bus_if.h"
#include "bchp_xpt_dpcr0.h"
#include "bchp_xpt_dpcr1.h"
#include "bchp_xpt_dpcr2.h"
#include "bchp_xpt_dpcr3.h"
#include "bchp_xpt_dpcr_pp.h"
#include "bchp_xpt_fe.h"
#include "bchp_xpt_full_pid_parser.h"
#include "bchp_xpt_gr.h"
#include "bchp_xpt_mpod.h"
#include "bchp_xpt_msg.h"
#include "bchp_xpt_pb0.h"
#include "bchp_xpt_pb1.h"
#include "bchp_xpt_pb2.h"
#include "bchp_xpt_pb3.h"
#include "bchp_xpt_pb4.h"
#include "bchp_xpt_pb5.h"
#include "bchp_xpt_pb_top.h"
#include "bchp_xpt_pcroffset.h"
#include "bchp_xpt_pmu.h"
#include "bchp_xpt_psub.h"
#include "bchp_xpt_rave.h"
#include "bchp_xpt_rave_to_scpu_l2_intr_0_31.h"
#include "bchp_xpt_rave_to_scpu_l2_intr_32_47.h"
#include "bchp_xpt_rmx0.h"
#include "bchp_xpt_rmx0_io.h"
#include "bchp_xpt_rmx1.h"
#include "bchp_xpt_rmx1_io.h"
#include "bchp_xpt_rsbuff.h"
#include "bchp_xpt_secure_bus_if.h"
#include "bchp_xpt_wakeup.h"
#include "bchp_xpt_xcbuff.h"
#include "bchp_xpt_xmemif.h"
#include "bchp_xpt_xpu.h"

const BXPT_P_RegisterRange XPT_SRAM_LIST[] =
{
    { "WAKEUP_MEM",                              BCHP_XPT_WAKEUP_PKT_MEM_i_ARRAY_BASE,              (BCHP_XPT_WAKEUP_PKT_TYPE3_i_ARRAY_BASE+(BCHP_XPT_WAKEUP_PKT_TYPE3_i_ARRAY_END*4)) },
    { "FE_PID_TABLE",                            BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE,                (BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE+(BCHP_XPT_FE_PID_TABLE_i_ARRAY_END*4)) },
    { "FE_SPID_TABLE",                           BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE,               (BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE+(BCHP_XPT_FE_SPID_TABLE_i_ARRAY_END*4)) },
    { "PCROFFSET_PID_TABLE",                     BCHP_XPT_PCROFFSET_PID_CONFIG_TABLE_i_ARRAY_BASE,  (BCHP_XPT_PCROFFSET_PID_CONFIG_TABLE_i_ARRAY_BASE+(BCHP_XPT_PCROFFSET_PID_CONFIG_TABLE_i_ARRAY_END*4)) },
    { "PCROFFSET_CX_TABLE",                      BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL,        BCHP_XPT_PCROFFSET_CONTEXT15_RESERVED_CFG_0 },
    { "FPP_SCM0",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_END*4)) },
    { "FPP_SCM1",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_1_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_1_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_1_i_ARRAY_END*4)) },
    { "FPP_SCM2",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_2_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_2_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_2_i_ARRAY_END*4)) },
    { "FPP_SCM3",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_3_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_3_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_3_i_ARRAY_END*4)) },
    { "RSBUFF_IBP_PMEM",                         BCHP_XPT_RSBUFF_BASE_POINTER_IBP0,                 BCHP_XPT_RSBUFF_WATERMARK_IBP15 },
    { "RSBUFF_PBP_PMEM",                         BCHP_XPT_RSBUFF_BASE_POINTER_PBP0,                 BCHP_XPT_RSBUFF_WATERMARK_PBP5 },
    { "RSBUFF_MPOD_PMEM",                        BCHP_XPT_RSBUFF_BASE_POINTER_MPOD_IBP0,            BCHP_XPT_RSBUFF_WATERMARK_MPOD_IBP15 },
    { "XCBUFF_RAVE_IBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RAVE_IBP0,            BCHP_XPT_XCBUFF_WATERMARK_RAVE_IBP15 },
    { "XCBUFF_RAVE_PBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RAVE_PBP0,            BCHP_XPT_XCBUFF_WATERMARK_RAVE_PBP5 },
    { "XCBUFF_MSG_IBP_PMEM",                     BCHP_XPT_XCBUFF_BASE_POINTER_MSG_IBP0,             BCHP_XPT_XCBUFF_WATERMARK_MSG_IBP15 },
    { "XCBUFF_MSG_PBP_PMEM",                     BCHP_XPT_XCBUFF_BASE_POINTER_MSG_PBP0,             BCHP_XPT_XCBUFF_WATERMARK_MSG_PBP5 },
    { "XCBUFF_RMX0_IBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX0_IBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX0_IBP15 },
    { "XCBUFF_RMX0_PBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX0_PBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX0_PBP5 },
    { "XCBUFF_RMX1_IBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX1_IBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX1_IBP15 },
    { "XCBUFF_RMX1_PBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX1_PBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX1_PBP5 },
    { "MSG_PMEM",                                BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_ARRAY_BASE,         (BCHP_XPT_MSG_DMA_VP_TABLE_i_ARRAY_BASE+(BCHP_XPT_MSG_DMA_VP_TABLE_i_ARRAY_END*4)) },
    { "MSG_GEN_FILT1",                           BCHP_XPT_MSG_GEN_FILT_EN_i_ARRAY_BASE,             (BCHP_XPT_MSG_GEN_FILT_EN_i_ARRAY_BASE+(BCHP_XPT_MSG_GEN_FILT_EN_i_ARRAY_END*4)) },
    { "MSG_GEN_FILT2",                           BCHP_XPT_MSG_MCAST_16_ADDR_i_ARRAY_BASE,           (BCHP_XPT_MSG_GEN_FILT_EXCL_i_ARRAY_BASE+(BCHP_XPT_MSG_GEN_FILT_EXCL_i_ARRAY_END*4)) },
    { "MSG_PID2BUF",                             BCHP_XPT_MSG_PID2BUF_MAP_i_ARRAY_BASE,             (BCHP_XPT_MSG_PID2BUF_MAP_i_ARRAY_BASE+(BCHP_XPT_MSG_PID2BUF_MAP_i_ARRAY_END*4)) },
    { "RAVE_PMEM",                               BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR,                BCHP_XPT_RAVE_SCD47_RESERVED_STATE3 },
    { "RAVE_CXMEM_A",                            BCHP_XPT_RAVE_CXMEM_Ai_ARRAY_BASE,                 (BCHP_XPT_RAVE_CXMEM_Ai_ARRAY_BASE+(BCHP_XPT_RAVE_CXMEM_Ai_ARRAY_END*4)) },
    { "RAVE_CXMEM_B",                            BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_BASE,                 (BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_BASE+(BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_END*4)) },
    { "RAVE_DMEM",                               BCHP_XPT_RAVE_DMEMi_ARRAY_BASE,                    (BCHP_XPT_RAVE_DMEMi_ARRAY_BASE+(BCHP_XPT_RAVE_DMEMi_ARRAY_END*4)) },
    { "RAVE_SMEM",                               BCHP_XPT_RAVE_TPIT0_PID_TABLEi_ARRAY_BASE,         BCHP_XPT_RAVE_TPIT_STATE_CONTEXT23 },
    {0, 0, 0}
};

const BXPT_P_RegisterRange XPT_REG_SAVE_LIST[] =
{
    { "XPT_BUS_IF_MISC_CTRL0",                   BCHP_XPT_BUS_IF_MISC_CTRL0,                        BCHP_XPT_BUS_IF_TEST_MODE },
    { "XPT_BUS_IF_INTR_STATUS_REG_EN",           BCHP_XPT_BUS_IF_INTR_STATUS_REG_EN,                BCHP_XPT_BUS_IF_INTR_STATUS_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS2_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS2_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS2_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS3_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS3_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS3_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS4_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS4_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS4_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS5_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS5_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS5_REG_EN },
    { "XPT_XMEMIF_SCB_WR_ARB_SEL_RSBUFF",        BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RSBUFF,             BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB5 },
    { "XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB",          BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB,               BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB },
    { "XPT_XMEMIF_SCB_WR_ARB_MODE",              BCHP_XPT_XMEMIF_SCB_WR_ARB_MODE,                   BCHP_XPT_XMEMIF_SCB_RD_ARB_MODE },
    { "XPT_XMEMIF_INTR_STATUS_REG_EN",           BCHP_XPT_XMEMIF_INTR_STATUS_REG_EN,                BCHP_XPT_XMEMIF_INTR_STATUS_REG_EN },
    { "XPT_PMU_CLK_CTRL",                        BCHP_XPT_PMU_CLK_CTRL,                             BCHP_XPT_PMU_MEM_INIT_CTRL },
    { "XPT_GR_CTRL",                             BCHP_XPT_GR_CTRL,                                  BCHP_XPT_GR_CTRL },
    { "XPT_RMX0_IO_FORMAT",                      BCHP_XPT_RMX0_IO_FORMAT,                           BCHP_XPT_RMX0_IO_PKT_DLY_CNT },
    { "XPT_RMX1_IO_FORMAT",                      BCHP_XPT_RMX1_IO_FORMAT,                           BCHP_XPT_RMX1_IO_PKT_DLY_CNT },
    { "XPT_RMX1_IO_TV_STATUS",                   BCHP_XPT_RMX1_IO_TV_STATUS,                        BCHP_XPT_RMX1_IO_TV_STATUS },
    { "XPT_WAKEUP_CTRL",                         BCHP_XPT_WAKEUP_CTRL,                              BCHP_XPT_WAKEUP_CTRL },
    { "XPT_WAKEUP_INTR_STATUS_REG_EN",           BCHP_XPT_WAKEUP_INTR_STATUS_REG_EN,                BCHP_XPT_WAKEUP_INTR_STATUS_REG_EN },
    { "XPT_FE_FE_CTRL",                          BCHP_XPT_FE_FE_CTRL,                               BCHP_XPT_FE_FE_CTRL },
    { "XPT_FE_INTR_STATUS0_REG_EN",              BCHP_XPT_FE_INTR_STATUS0_REG_EN,                   BCHP_XPT_FE_PWR_CTRL },
    { "XPT_FE_MAX_PID_CHANNEL",                  BCHP_XPT_FE_MAX_PID_CHANNEL,                       BCHP_XPT_FE_MAX_PID_CHANNEL },
    { "XPT_FE_IB0_CTRL",                         BCHP_XPT_FE_IB0_CTRL,                              BCHP_XPT_FE_IB0_CTRL },
    { "XPT_FE_IB1_CTRL",                         BCHP_XPT_FE_IB1_CTRL,                              BCHP_XPT_FE_IB1_CTRL },
    { "XPT_FE_IB2_CTRL",                         BCHP_XPT_FE_IB2_CTRL,                              BCHP_XPT_FE_IB2_CTRL },
    { "XPT_FE_IB3_CTRL",                         BCHP_XPT_FE_IB3_CTRL,                              BCHP_XPT_FE_IB3_CTRL },
    { "XPT_FE_IB4_CTRL",                         BCHP_XPT_FE_IB4_CTRL,                              BCHP_XPT_FE_IB4_CTRL },
    { "XPT_FE_IB5_CTRL",                         BCHP_XPT_FE_IB5_CTRL,                              BCHP_XPT_FE_IB5_CTRL },
    { "XPT_FE_IB6_CTRL",                         BCHP_XPT_FE_IB6_CTRL,                              BCHP_XPT_FE_IB6_CTRL },
    { "XPT_FE_IB7_CTRL",                         BCHP_XPT_FE_IB7_CTRL,                              BCHP_XPT_FE_IB7_CTRL },
    { "XPT_FE_IB8_CTRL",                         BCHP_XPT_FE_IB8_CTRL,                              BCHP_XPT_FE_IB8_CTRL },
    { "XPT_FE_IB9_CTRL",                         BCHP_XPT_FE_IB9_CTRL,                              BCHP_XPT_FE_IB9_CTRL },
    { "XPT_FE_IB10_CTRL",                        BCHP_XPT_FE_IB10_CTRL,                             BCHP_XPT_FE_IB10_CTRL },
    { "XPT_FE_IB11_CTRL",                        BCHP_XPT_FE_IB11_CTRL,                             BCHP_XPT_FE_IB11_CTRL },
    { "XPT_FE_MINI_PID_PARSER0_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER0_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER0_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER1_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER1_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER1_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER2_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER2_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER2_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER3_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER3_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER3_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER4_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER4_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER4_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER5_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER5_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER5_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER6_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER6_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER6_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER7_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER7_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER7_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER8_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER8_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER8_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER9_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER9_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER9_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER10_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER10_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER10_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER11_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER11_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER11_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER12_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER12_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER12_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER13_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER13_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER13_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER14_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER14_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER14_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER15_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER15_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER15_CTRL2 },
    { "XPT_FE_IB_SYNC_DETECT_CTRL",              BCHP_XPT_FE_IB_SYNC_DETECT_CTRL,                   BCHP_XPT_FE_IB_SYNC_DETECT_CTRL },
    { "XPT_FE_TSMF0_CTRL",                       BCHP_XPT_FE_TSMF0_CTRL,                            BCHP_XPT_FE_TSMF0_SLOT_MAP_HI },
    { "XPT_FE_TSMF1_CTRL",                       BCHP_XPT_FE_TSMF1_CTRL,                            BCHP_XPT_FE_TSMF1_SLOT_MAP_HI },
    { "XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID", BCHP_XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID,   BCHP_XPT_FE_MINI_PID_PARSER12_TO_PARSER15_BAND_ID },
    { "XPT_FE_ATS_COUNTER_CTRL",                 BCHP_XPT_FE_ATS_COUNTER_CTRL,                      BCHP_XPT_FE_ATS_TS_BINARY },
    { "XPT_FE_MTSIF_RX0_CTRL1",                  BCHP_XPT_FE_MTSIF_RX0_CTRL1,                       BCHP_XPT_FE_MTSIF_RX0_CTRL1 },
    { "XPT_FE_MTSIF_RX0_INTR_STATUS0_REG_EN",    BCHP_XPT_FE_MTSIF_RX0_INTR_STATUS0_REG_EN,         BCHP_XPT_FE_MTSIF_RX0_BAND_ID_DROP },
    { "XPT_FE_MTSIF_RX1_CTRL1",                  BCHP_XPT_FE_MTSIF_RX1_CTRL1,                       BCHP_XPT_FE_MTSIF_RX1_CTRL1 },
    { "XPT_FE_MTSIF_RX1_INTR_STATUS0_REG_EN",    BCHP_XPT_FE_MTSIF_RX1_INTR_STATUS0_REG_EN,         BCHP_XPT_FE_MTSIF_RX1_BAND_ID_DROP },
    { "XPT_DPCR0_PID_CH",                        BCHP_XPT_DPCR0_PID_CH,                             BCHP_XPT_DPCR0_CTRL },
    { "XPT_DPCR0_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR0_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR0_STC_EXT_CTRL },
    { "XPT_DPCR0_MAX_PCR_ERROR",                 BCHP_XPT_DPCR0_MAX_PCR_ERROR,                      BCHP_XPT_DPCR0_MAX_PCR_ERROR },
    { "XPT_DPCR0_STC_HI",                        BCHP_XPT_DPCR0_STC_HI,                             BCHP_XPT_DPCR0_STC_LO },
    { "XPT_DPCR0_LOOP_CTRL",                     BCHP_XPT_DPCR0_LOOP_CTRL,                          BCHP_XPT_DPCR0_ACCUM_VALUE },
    { "XPT_DPCR0_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR0_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR0_PHASE_ERROR_CLAMP },
    { "XPT_DPCR1_PID_CH",                        BCHP_XPT_DPCR1_PID_CH,                             BCHP_XPT_DPCR1_CTRL },
    { "XPT_DPCR1_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR1_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR1_STC_EXT_CTRL },
    { "XPT_DPCR1_MAX_PCR_ERROR",                 BCHP_XPT_DPCR1_MAX_PCR_ERROR,                      BCHP_XPT_DPCR1_MAX_PCR_ERROR },
    { "XPT_DPCR1_STC_HI",                        BCHP_XPT_DPCR1_STC_HI,                             BCHP_XPT_DPCR1_STC_LO },
    { "XPT_DPCR1_LOOP_CTRL",                     BCHP_XPT_DPCR1_LOOP_CTRL,                          BCHP_XPT_DPCR1_ACCUM_VALUE },
    { "XPT_DPCR1_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR1_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR1_PHASE_ERROR_CLAMP },
    { "XPT_DPCR2_PID_CH",                        BCHP_XPT_DPCR2_PID_CH,                             BCHP_XPT_DPCR2_CTRL },
    { "XPT_DPCR2_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR2_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR2_STC_EXT_CTRL },
    { "XPT_DPCR2_MAX_PCR_ERROR",                 BCHP_XPT_DPCR2_MAX_PCR_ERROR,                      BCHP_XPT_DPCR2_MAX_PCR_ERROR },
    { "XPT_DPCR2_STC_HI",                        BCHP_XPT_DPCR2_STC_HI,                             BCHP_XPT_DPCR2_STC_LO },
    { "XPT_DPCR2_LOOP_CTRL",                     BCHP_XPT_DPCR2_LOOP_CTRL,                          BCHP_XPT_DPCR2_ACCUM_VALUE },
    { "XPT_DPCR2_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR2_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR2_PHASE_ERROR_CLAMP },
    { "XPT_DPCR3_PID_CH",                        BCHP_XPT_DPCR3_PID_CH,                             BCHP_XPT_DPCR3_CTRL },
    { "XPT_DPCR3_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR3_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR3_STC_EXT_CTRL },
    { "XPT_DPCR3_MAX_PCR_ERROR",                 BCHP_XPT_DPCR3_MAX_PCR_ERROR,                      BCHP_XPT_DPCR3_MAX_PCR_ERROR },
    { "XPT_DPCR3_STC_HI",                        BCHP_XPT_DPCR3_STC_HI,                             BCHP_XPT_DPCR3_STC_LO },
    { "XPT_DPCR3_LOOP_CTRL",                     BCHP_XPT_DPCR3_LOOP_CTRL,                          BCHP_XPT_DPCR3_ACCUM_VALUE },
    { "XPT_DPCR3_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR3_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR3_PHASE_ERROR_CLAMP },
    { "XPT_DPCR_PP_PP_CTRL",                     BCHP_XPT_DPCR_PP_PP_CTRL,                          BCHP_XPT_DPCR_PP_PP_FIXED_OFFSET },
    { "XPT_PSUB_PSUB0_CTRL0",                    BCHP_XPT_PSUB_PSUB0_CTRL0,                         BCHP_XPT_PSUB_PSUB0_STAT0 },
    { "XPT_PSUB_PSUB1_CTRL0",                    BCHP_XPT_PSUB_PSUB1_CTRL0,                         BCHP_XPT_PSUB_PSUB1_STAT0 },
    { "XPT_PSUB_PSUB2_CTRL0",                    BCHP_XPT_PSUB_PSUB2_CTRL0,                         BCHP_XPT_PSUB_PSUB2_STAT0 },
    { "XPT_PSUB_PSUB3_CTRL0",                    BCHP_XPT_PSUB_PSUB3_CTRL0,                         BCHP_XPT_PSUB_PSUB3_STAT0 },
    { "XPT_PSUB_PSUB4_CTRL0",                    BCHP_XPT_PSUB_PSUB4_CTRL0,                         BCHP_XPT_PSUB_PSUB4_STAT0 },
    { "XPT_PSUB_PSUB5_CTRL0",                    BCHP_XPT_PSUB_PSUB5_CTRL0,                         BCHP_XPT_PSUB_PSUB5_STAT0 },
    { "XPT_PSUB_PSUB6_CTRL0",                    BCHP_XPT_PSUB_PSUB6_CTRL0,                         BCHP_XPT_PSUB_PSUB6_STAT0 },
    { "XPT_PSUB_PSUB7_CTRL0",                    BCHP_XPT_PSUB_PSUB7_CTRL0,                         BCHP_XPT_PSUB_PSUB7_STAT0 },
    { "XPT_PSUB_PR_FALLBACK_CTRL",               BCHP_XPT_PSUB_PR_FALLBACK_CTRL,                    BCHP_XPT_PSUB_PR_FALLBACK_CTRL },
    { "XPT_MPOD_CFG",                            BCHP_XPT_MPOD_CFG,                                 BCHP_XPT_MPOD_ICTRL },
    { "XPT_MPOD_OCTRL2",                         BCHP_XPT_MPOD_OCTRL2,                              BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP },
    { "XPT_RMX0_CTRL",                           BCHP_XPT_RMX0_CTRL,                                BCHP_XPT_RMX0_FIXED_OFFSET },
    { "XPT_RMX1_CTRL",                           BCHP_XPT_RMX1_CTRL,                                BCHP_XPT_RMX1_FIXED_OFFSET },
    { "XPT_PB_TOP_PACING_COUNT_WR_VALUE",        BCHP_XPT_PB_TOP_PACING_COUNT_WR_VALUE,             BCHP_XPT_PB_TOP_BAND_PAUSE_MAP_VECT_PB5 },
    { "XPT_PB_TOP_PACING_PAUSE_MAP_VECT_PB0",    BCHP_XPT_PB_TOP_PACING_PAUSE_MAP_VECT_PB0,         BCHP_XPT_PB_TOP_PACING_PAUSE_MAP_VECT_PB5 },
    { "XPT_PB0_CTRL1",                           BCHP_XPT_PB0_CTRL1,                                BCHP_XPT_PB0_FIRST_DESC_ADDR },
    { "XPT_PB0_BLOCKOUT",                        BCHP_XPT_PB0_BLOCKOUT,                             BCHP_XPT_PB0_PARSER_CTRL2 },
    { "XPT_PB0_INTR_EN",                         BCHP_XPT_PB0_INTR_EN,                              BCHP_XPT_PB0_PWR_CTRL },
    { "XPT_PB0_ASF_CTRL",                        BCHP_XPT_PB0_ASF_CTRL,                             BCHP_XPT_PB0_PES_BASED_PACING_CTRL },
    { "XPT_PB1_CTRL1",                           BCHP_XPT_PB1_CTRL1,                                BCHP_XPT_PB1_FIRST_DESC_ADDR },
    { "XPT_PB1_BLOCKOUT",                        BCHP_XPT_PB1_BLOCKOUT,                             BCHP_XPT_PB1_PARSER_CTRL2 },
    { "XPT_PB1_INTR_EN",                         BCHP_XPT_PB1_INTR_EN,                              BCHP_XPT_PB1_PWR_CTRL },
    { "XPT_PB1_ASF_CTRL",                        BCHP_XPT_PB1_ASF_CTRL,                             BCHP_XPT_PB1_PES_BASED_PACING_CTRL },
    { "XPT_PB2_CTRL1",                           BCHP_XPT_PB2_CTRL1,                                BCHP_XPT_PB2_FIRST_DESC_ADDR },
    { "XPT_PB2_BLOCKOUT",                        BCHP_XPT_PB2_BLOCKOUT,                             BCHP_XPT_PB2_PARSER_CTRL2 },
    { "XPT_PB2_INTR_EN",                         BCHP_XPT_PB2_INTR_EN,                              BCHP_XPT_PB2_PWR_CTRL },
    { "XPT_PB2_ASF_CTRL",                        BCHP_XPT_PB2_ASF_CTRL,                             BCHP_XPT_PB2_PES_BASED_PACING_CTRL },
    { "XPT_PB3_CTRL1",                           BCHP_XPT_PB3_CTRL1,                                BCHP_XPT_PB3_FIRST_DESC_ADDR },
    { "XPT_PB3_BLOCKOUT",                        BCHP_XPT_PB3_BLOCKOUT,                             BCHP_XPT_PB3_PARSER_CTRL2 },
    { "XPT_PB3_INTR_EN",                         BCHP_XPT_PB3_INTR_EN,                              BCHP_XPT_PB3_PWR_CTRL },
    { "XPT_PB3_ASF_CTRL",                        BCHP_XPT_PB3_ASF_CTRL,                             BCHP_XPT_PB3_PES_BASED_PACING_CTRL },
    { "XPT_PB4_CTRL1",                           BCHP_XPT_PB4_CTRL1,                                BCHP_XPT_PB4_FIRST_DESC_ADDR },
    { "XPT_PB4_BLOCKOUT",                        BCHP_XPT_PB4_BLOCKOUT,                             BCHP_XPT_PB4_PARSER_CTRL2 },
    { "XPT_PB4_INTR_EN",                         BCHP_XPT_PB4_INTR_EN,                              BCHP_XPT_PB4_PWR_CTRL },
    { "XPT_PB4_ASF_CTRL",                        BCHP_XPT_PB4_ASF_CTRL,                             BCHP_XPT_PB4_PES_BASED_PACING_CTRL },
    { "XPT_PB5_CTRL1",                           BCHP_XPT_PB5_CTRL1,                                BCHP_XPT_PB5_FIRST_DESC_ADDR },
    { "XPT_PB5_BLOCKOUT",                        BCHP_XPT_PB5_BLOCKOUT,                             BCHP_XPT_PB5_PARSER_CTRL2 },
    { "XPT_PB5_INTR_EN",                         BCHP_XPT_PB5_INTR_EN,                              BCHP_XPT_PB5_PWR_CTRL },
    { "XPT_PB5_ASF_CTRL",                        BCHP_XPT_PB5_ASF_CTRL,                             BCHP_XPT_PB5_PES_BASED_PACING_CTRL },
    { "XPT_PCROFFSET_INTERRUPT0_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT0_ENABLE,              BCHP_XPT_PCROFFSET_INTERRUPT0_ENABLE },
    { "XPT_PCROFFSET_INTERRUPT1_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT1_ENABLE,              BCHP_XPT_PCROFFSET_INTERRUPT1_ENABLE },
    { "XPT_PCROFFSET_INTERRUPT2_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT2_ENABLE,              BCHP_XPT_PCROFFSET_INTERRUPT2_ENABLE },
    { "XPT_PCROFFSET_INTERRUPT3_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT3_ENABLE,              BCHP_XPT_PCROFFSET_STC_MUX_DELAY },
    { "XPT_PCROFFSET_STC_INTERRUPT_ENABLE",      BCHP_XPT_PCROFFSET_STC_INTERRUPT_ENABLE,           BCHP_XPT_PCROFFSET_STC7_MATCH },
    { "XPT_PCROFFSET_STC0_CTRL",                 BCHP_XPT_PCROFFSET_STC0_CTRL,                      BCHP_XPT_PCROFFSET_STC0_INC_LO },
    { "XPT_PCROFFSET_STC0_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC0_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC0_AV_WINDOW },
    { "XPT_PCROFFSET_STC1_CTRL",                 BCHP_XPT_PCROFFSET_STC1_CTRL,                      BCHP_XPT_PCROFFSET_STC1_INC_LO },
    { "XPT_PCROFFSET_STC1_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC1_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC1_AV_WINDOW },
    { "XPT_PCROFFSET_STC2_CTRL",                 BCHP_XPT_PCROFFSET_STC2_CTRL,                      BCHP_XPT_PCROFFSET_STC2_INC_LO },
    { "XPT_PCROFFSET_STC2_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC2_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC2_AV_WINDOW },
    { "XPT_PCROFFSET_STC3_CTRL",                 BCHP_XPT_PCROFFSET_STC3_CTRL,                      BCHP_XPT_PCROFFSET_STC3_INC_LO },
    { "XPT_PCROFFSET_STC3_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC3_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC3_AV_WINDOW },
    { "XPT_PCROFFSET_STC4_CTRL",                 BCHP_XPT_PCROFFSET_STC4_CTRL,                      BCHP_XPT_PCROFFSET_STC4_INC_LO },
    { "XPT_PCROFFSET_STC4_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC4_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC4_AV_WINDOW },
    { "XPT_PCROFFSET_STC5_CTRL",                 BCHP_XPT_PCROFFSET_STC5_CTRL,                      BCHP_XPT_PCROFFSET_STC5_INC_LO },
    { "XPT_PCROFFSET_STC5_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC5_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC5_AV_WINDOW },
    { "XPT_PCROFFSET_STC6_CTRL",                 BCHP_XPT_PCROFFSET_STC6_CTRL,                      BCHP_XPT_PCROFFSET_STC6_INC_LO },
    { "XPT_PCROFFSET_STC6_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC6_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC6_AV_WINDOW },
    { "XPT_PCROFFSET_STC7_CTRL",                 BCHP_XPT_PCROFFSET_STC7_CTRL,                      BCHP_XPT_PCROFFSET_STC7_INC_LO },
    { "XPT_PCROFFSET_STC7_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC7_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC7_AV_WINDOW },
    { "XPT_FULL_PID_PARSER_PARSER_CFG",          BCHP_XPT_FULL_PID_PARSER_PARSER_CFG,               BCHP_XPT_FULL_PID_PARSER_PBP_ACCEPT_ADAPT_00 },
    { "XPT_FULL_PID_PARSER_IBP_PCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_PCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_PCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_PCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_PCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_PCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_IBP_SCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_SCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_SCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_SCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_SCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_SCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_IBP_PSG_PROTOCOL_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_PSG_PROTOCOL_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_PSG_PROTOCOL_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_PSG_PROTOCOL_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_PSG_PROTOCOL_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_PSG_PROTOCOL_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_IBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PSG_ERR_MODE",        BCHP_XPT_FULL_PID_PARSER_PSG_ERR_MODE,             BCHP_XPT_FULL_PID_PARSER_PSG_ERR_MODE },
    { "XPT_RSBUFF_BO_IBP0",                      BCHP_XPT_RSBUFF_BO_IBP0,                           BCHP_XPT_RSBUFF_BO_IBP15 },
    { "XPT_RSBUFF_BO_PBP0",                      BCHP_XPT_RSBUFF_BO_PBP0,                           BCHP_XPT_RSBUFF_BO_PBP5 },
    { "XPT_RSBUFF_BO_MPOD_IBP0",                 BCHP_XPT_RSBUFF_BO_MPOD_IBP0,                      BCHP_XPT_RSBUFF_BO_MPOD_IBP15 },
    { "XPT_RSBUFF_IBP_BUFFER_ENABLE",            BCHP_XPT_RSBUFF_IBP_BUFFER_ENABLE,                 BCHP_XPT_RSBUFF_PBP_BUFFER_ENABLE },
    { "XPT_RSBUFF_MPOD_IBP_BUFFER_ENABLE",       BCHP_XPT_RSBUFF_MPOD_IBP_BUFFER_ENABLE,            BCHP_XPT_RSBUFF_MPOD_IBP_BUFFER_ENABLE },
    { "XPT_RSBUFF_OVERFLOW_THRESHOLD",           BCHP_XPT_RSBUFF_OVERFLOW_THRESHOLD,                BCHP_XPT_RSBUFF_NO_RD_HANG_CTRL },
    { "XPT_RSBUFF_PR_FALLBACK_CTRL",             BCHP_XPT_RSBUFF_PR_FALLBACK_CTRL,                  BCHP_XPT_RSBUFF_PR_FALLBACK_CTRL },
    { "XPT_XCBUFF_RAVE_CTRL_BUFFER_EN_PBP",      BCHP_XPT_XCBUFF_RAVE_CTRL_BUFFER_EN_PBP,           BCHP_XPT_XCBUFF_PAUSE_THRESHOLD },
    { "XPT_XCBUFF_OVERFLOW_THRESHOLD",           BCHP_XPT_XCBUFF_OVERFLOW_THRESHOLD,                BCHP_XPT_XCBUFF_OVERFLOW_THRESHOLD },
    { "XPT_XCBUFF_BYTE_SWAP_CTRL",               BCHP_XPT_XCBUFF_BYTE_SWAP_CTRL,                    BCHP_XPT_XCBUFF_BO_RAVE_IBP15 },
    { "XPT_XCBUFF_BO_RAVE_PBP0",                 BCHP_XPT_XCBUFF_BO_RAVE_PBP0,                      BCHP_XPT_XCBUFF_BO_RAVE_PBP5 },
    { "XPT_XCBUFF_BO_MSG_IBP0",                  BCHP_XPT_XCBUFF_BO_MSG_IBP0,                       BCHP_XPT_XCBUFF_BO_MSG_IBP15 },
    { "XPT_XCBUFF_BO_MSG_PBP0",                  BCHP_XPT_XCBUFF_BO_MSG_PBP0,                       BCHP_XPT_XCBUFF_BO_MSG_PBP5 },
    { "XPT_XCBUFF_BO_RMX0_IBP0",                 BCHP_XPT_XCBUFF_BO_RMX0_IBP0,                      BCHP_XPT_XCBUFF_BO_RMX0_IBP15 },
    { "XPT_XCBUFF_BO_RMX0_PBP0",                 BCHP_XPT_XCBUFF_BO_RMX0_PBP0,                      BCHP_XPT_XCBUFF_BO_RMX0_PBP5 },
    { "XPT_XCBUFF_BO_RMX1_IBP0",                 BCHP_XPT_XCBUFF_BO_RMX1_IBP0,                      BCHP_XPT_XCBUFF_BO_RMX1_IBP15 },
    { "XPT_XCBUFF_BO_RMX1_PBP0",                 BCHP_XPT_XCBUFF_BO_RMX1_PBP0,                      BCHP_XPT_XCBUFF_BO_RMX1_PBP5 },
    { "XPT_XCBUFF_MISC_CTRL",                    BCHP_XPT_XCBUFF_MISC_CTRL,                         BCHP_XPT_XCBUFF_NO_RD_HANG_CTRL },
    { "XPT_XCBUFF_PR_FALLBACK_CTRL",             BCHP_XPT_XCBUFF_PR_FALLBACK_CTRL,                  BCHP_XPT_XCBUFF_PR_FALLBACK_CTRL },
    { "XPT_MSG_MSG_CTRL1",                       BCHP_XPT_MSG_MSG_CTRL1,                            BCHP_XPT_MSG_DMA_BUFFER_INIT },
    { "XPT_MSG_PHY_ADDR_LO",                     BCHP_XPT_MSG_PHY_ADDR_LO,                          BCHP_XPT_MSG_ID_REJECT },
    { "XPT_MSG_C_MATCH0",                        BCHP_XPT_MSG_C_MATCH0,                             BCHP_XPT_MSG_C_MATCH4 },
    { "XPT_MSG_BUF_DAT_RDY_INTR_EN_00_31",       BCHP_XPT_MSG_BUF_DAT_RDY_INTR_EN_00_31,            BCHP_XPT_MSG_BUF_DAT_RDY_INTR_EN_96_127 },
    { "XPT_MSG_BUF_OVFL_INTR_EN_00_31",          BCHP_XPT_MSG_BUF_OVFL_INTR_EN_00_31,               BCHP_XPT_MSG_BUF_OVFL_INTR_EN_96_127 },
    { "XPT_MSG_DAT_ERR_INTR_EN",                 BCHP_XPT_MSG_DAT_ERR_INTR_EN,                      BCHP_XPT_MSG_DAT_ERR_INTR_EN },
    { "XPT_MSG_MSG_EVENT_CNT_CTRL",              BCHP_XPT_MSG_MSG_EVENT_CNT_CTRL,                   BCHP_XPT_MSG_MSG_EVENT_CNT_CTRL },
    { "XPT_RAVE_AVS_SCV_FILTER_MODE_CONTROL",    BCHP_XPT_RAVE_AVS_SCV_FILTER_MODE_CONTROL,         BCHP_XPT_RAVE_AVS_SCV_FILTER_VALUE_4_TO_7 },
    { "XPT_RAVE_DATA_START_ADDR_A",              BCHP_XPT_RAVE_DATA_START_ADDR_A,                   BCHP_XPT_RAVE_BASE_ADDRESSES },
    { "XPT_RAVE_MISC_CONTROL2",                  BCHP_XPT_RAVE_MISC_CONTROL2,                       BCHP_XPT_RAVE_RC8_SP_CONTROL },
    { "XPT_RAVE_ATSOFFSET_MAX_ERROR",            BCHP_XPT_RAVE_ATSOFFSET_MAX_ERROR,                 BCHP_XPT_RAVE_ATSOFFSET_MAX_ERROR },
    { "XPT_RAVE_TPIT_TIME_TICK",                 BCHP_XPT_RAVE_TPIT_TIME_TICK,                      BCHP_XPT_RAVE_EMM_MASK_ID_8 },
    {0, 0, 0}
};

#elif (BCHP_CHIP==7344 && BCHP_VER>=BCHP_VER_B0)
#include "bchp_xpt_bus_if.h"
#include "bchp_xpt_dpcr0.h"
#include "bchp_xpt_dpcr1.h"
#include "bchp_xpt_dpcr2.h"
#include "bchp_xpt_dpcr3.h"
#include "bchp_xpt_dpcr_pp.h"
#include "bchp_xpt_fe.h"
#include "bchp_xpt_full_pid_parser.h"
#include "bchp_xpt_gr.h"
#include "bchp_xpt_mpod.h"
#include "bchp_xpt_msg.h"
#include "bchp_xpt_pb0.h"
#include "bchp_xpt_pb1.h"
#include "bchp_xpt_pb2.h"
#include "bchp_xpt_pb3.h"
#include "bchp_xpt_pcroffset.h"
#include "bchp_xpt_pmu.h"
#include "bchp_xpt_psub.h"
#include "bchp_xpt_rave.h"
#include "bchp_xpt_rmx0.h"
#include "bchp_xpt_rmx0_io.h"
#include "bchp_xpt_rmx1.h"
#include "bchp_xpt_rmx1_io.h"
#include "bchp_xpt_rsbuff.h"
#include "bchp_xpt_secure_bus_if.h"
#include "bchp_xpt_wakeup.h"
#include "bchp_xpt_xcbuff.h"
#include "bchp_xpt_xmemif.h"
#include "bchp_xpt_xpu.h"

const BXPT_P_RegisterRange XPT_SRAM_LIST[] =
{
    { "WAKEUP_MEM",                              BCHP_XPT_WAKEUP_PKT_MEM_i_ARRAY_BASE,              (BCHP_XPT_WAKEUP_PKT_TYPE3_i_ARRAY_BASE+(BCHP_XPT_WAKEUP_PKT_TYPE3_i_ARRAY_END*4)) },
    { "FE_PID_TABLE",                            BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE,                (BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE+(BCHP_XPT_FE_PID_TABLE_i_ARRAY_END*4)) },
    { "FE_SPID_TABLE",                           BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE,               (BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE+(BCHP_XPT_FE_SPID_TABLE_i_ARRAY_END*4)) },
    { "RSBUFF_IBP_PMEM",                         BCHP_XPT_RSBUFF_BASE_POINTER_IBP0,                 BCHP_XPT_RSBUFF_WATERMARK_IBP15 },
    { "RSBUFF_PBP_PMEM",                         BCHP_XPT_RSBUFF_BASE_POINTER_PBP0,                 BCHP_XPT_RSBUFF_WATERMARK_PBP3 },
    { "XCBUFF_RAVE_IBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RAVE_IBP0,            BCHP_XPT_XCBUFF_WATERMARK_RAVE_IBP15 },
    { "XCBUFF_RAVE_PBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RAVE_PBP0,            BCHP_XPT_XCBUFF_WATERMARK_RAVE_PBP3 },
    { "XCBUFF_MSG_IBP_PMEM",                     BCHP_XPT_XCBUFF_BASE_POINTER_MSG_IBP0,             BCHP_XPT_XCBUFF_WATERMARK_MSG_IBP15 },
    { "XCBUFF_MSG_PBP_PMEM",                     BCHP_XPT_XCBUFF_BASE_POINTER_MSG_PBP0,             BCHP_XPT_XCBUFF_WATERMARK_MSG_PBP3 },
    { "XCBUFF_RMX0_IBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX0_IBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX0_IBP15 },
    { "XCBUFF_RMX0_PBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX0_PBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX0_PBP3 },
    { "XCBUFF_RMX1_IBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX1_IBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX1_IBP15 },
    { "XCBUFF_RMX1_PBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX1_PBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX1_PBP3 },
    { "RAVE_PMEM",                               BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR,                BCHP_XPT_RAVE_SCD32_RESERVED_STATE3 },
    { "RAVE_CXMEM_LO",                           BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_BASE,                (BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_BASE+(BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_END*4)) },
    { "RAVE_CXMEM_HI",                           BCHP_XPT_RAVE_CXMEM_HIi_ARRAY_BASE,                (BCHP_XPT_RAVE_CXMEM_HIi_ARRAY_BASE+(BCHP_XPT_RAVE_CXMEM_HIi_ARRAY_END*4)) },
    { "RAVE_DMEM",                               BCHP_XPT_RAVE_DMEMi_ARRAY_BASE,                    (BCHP_XPT_RAVE_DMEMi_ARRAY_BASE+(BCHP_XPT_RAVE_DMEMi_ARRAY_END*4)) },
    { "RAVE_SMEM",                               BCHP_XPT_RAVE_TPIT0_PID_TABLEi_ARRAY_BASE,         BCHP_XPT_RAVE_TPIT_STATE_CONTEXT23 },
    { "PCROFFSET_PID_TABLE",                     BCHP_XPT_PCROFFSET_PID_CONFIG_TABLE_i_ARRAY_BASE,  (BCHP_XPT_PCROFFSET_PID_CONFIG_TABLE_i_ARRAY_BASE+(BCHP_XPT_PCROFFSET_PID_CONFIG_TABLE_i_ARRAY_END*4)) },
    { "PCROFFSET_CX_TABLE",                      BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL,        BCHP_XPT_PCROFFSET_CONTEXT15_RESERVED_CFG_4 },
    { "MSG_PMEM",                                BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_ARRAY_BASE,         (BCHP_XPT_MSG_DMA_VP_TABLE_i_ARRAY_BASE+(BCHP_XPT_MSG_DMA_VP_TABLE_i_ARRAY_END*4)) },
    { "MSG_GEN_FILT1",                           BCHP_XPT_MSG_GEN_FILT_EN_i_ARRAY_BASE,             (BCHP_XPT_MSG_GEN_FILT_EN_i_ARRAY_BASE+(BCHP_XPT_MSG_GEN_FILT_EN_i_ARRAY_END*4)) },
    { "MSG_GEN_FILT2",                           BCHP_XPT_MSG_MCAST_16_ADDR_i_ARRAY_BASE,           (BCHP_XPT_MSG_GEN_FILT_EXCL_i_ARRAY_BASE+(BCHP_XPT_MSG_GEN_FILT_EXCL_i_ARRAY_END*4)) },
    { "MSG_PID2BUF",                             BCHP_XPT_MSG_PID2BUF_MAP_i_ARRAY_BASE,             (BCHP_XPT_MSG_PID2BUF_MAP_i_ARRAY_BASE+(BCHP_XPT_MSG_PID2BUF_MAP_i_ARRAY_END*4)) },
    { "FPP_SCM0",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_END*4)) },
    { "FPP_SCM1",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_1_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_1_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_1_i_ARRAY_END*4)) },
    { "FPP_SCM2",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_2_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_2_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_2_i_ARRAY_END*4)) },
    { "FPP_SCM3",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_3_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_3_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_3_i_ARRAY_END*4)) },
    {0, 0, 0}
};

const BXPT_P_RegisterRange XPT_REG_SAVE_LIST[] =
{
    { "XPT_BUS_IF_MISC_CTRL0",                   BCHP_XPT_BUS_IF_MISC_CTRL0,                        BCHP_XPT_BUS_IF_TEST_MODE },
    { "XPT_BUS_IF_INTR_STATUS_REG_EN",           BCHP_XPT_BUS_IF_INTR_STATUS_REG_EN,                BCHP_XPT_BUS_IF_INTR_STATUS_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS2_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS2_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS2_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS3_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS3_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS3_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS4_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS4_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS4_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS5_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS5_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS5_REG_EN },
    { "XPT_BUS_IF_XPKT_FORMAT_CONV_RS2SEC",      BCHP_XPT_BUS_IF_XPKT_FORMAT_CONV_RS2SEC,           BCHP_XPT_BUS_IF_XPKT_FORMAT_CONV_SEC2FPP },
    { "XPT_XMEMIF_SCB_WR_ARB_SEL_RSBUFF",        BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RSBUFF,             BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB3 },
    { "XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB",          BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB,               BCHP_XPT_XMEMIF_SCB_RD_ARB_MODE },
    { "XPT_XMEMIF_INTR_STATUS_REG_EN",           BCHP_XPT_XMEMIF_INTR_STATUS_REG_EN,                BCHP_XPT_XMEMIF_INTR_STATUS_REG_EN },
    { "XPT_PMU_CLK_CTRL",                        BCHP_XPT_PMU_CLK_CTRL,                             BCHP_XPT_PMU_RBUS_RSP_VAL },
    { "XPT_WAKEUP_CTRL",                         BCHP_XPT_WAKEUP_CTRL,                              BCHP_XPT_WAKEUP_CTRL },
    { "XPT_WAKEUP_INTR_STATUS_REG_EN",           BCHP_XPT_WAKEUP_INTR_STATUS_REG_EN,                BCHP_XPT_WAKEUP_INTR_STATUS_REG_EN },
    { "XPT_RMX0_IO_FORMAT",                      BCHP_XPT_RMX0_IO_FORMAT,                           BCHP_XPT_RMX0_IO_PKT_DLY_CNT },
    { "XPT_RMX1_IO_FORMAT",                      BCHP_XPT_RMX1_IO_FORMAT,                           BCHP_XPT_RMX1_IO_PKT_DLY_CNT },
    { "XPT_RMX1_IO_TV_STATUS",                   BCHP_XPT_RMX1_IO_TV_STATUS,                        BCHP_XPT_RMX1_IO_TV_STATUS },
    { "XPT_FE_FE_CTRL",                          BCHP_XPT_FE_FE_CTRL,                               BCHP_XPT_FE_FE_CTRL },
    { "XPT_FE_INTR_STATUS0_REG_EN",              BCHP_XPT_FE_INTR_STATUS0_REG_EN,                   BCHP_XPT_FE_PWR_CTRL },
    { "XPT_FE_MAX_PID_CHANNEL",                  BCHP_XPT_FE_MAX_PID_CHANNEL,                       BCHP_XPT_FE_MAX_PID_CHANNEL },
    { "XPT_FE_IB0_CTRL",                         BCHP_XPT_FE_IB0_CTRL,                              BCHP_XPT_FE_IB0_CTRL },
    { "XPT_FE_IB1_CTRL",                         BCHP_XPT_FE_IB1_CTRL,                              BCHP_XPT_FE_IB1_CTRL },
    { "XPT_FE_IB2_CTRL",                         BCHP_XPT_FE_IB2_CTRL,                              BCHP_XPT_FE_IB2_CTRL },
    { "XPT_FE_IB3_CTRL",                         BCHP_XPT_FE_IB3_CTRL,                              BCHP_XPT_FE_IB3_CTRL },
    { "XPT_FE_IB4_CTRL",                         BCHP_XPT_FE_IB4_CTRL,                              BCHP_XPT_FE_IB4_CTRL },
    { "XPT_FE_MINI_PID_PARSER0_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER0_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER0_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER1_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER1_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER1_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER2_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER2_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER2_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER3_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER3_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER3_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER4_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER4_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER4_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER5_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER5_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER5_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER6_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER6_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER6_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER7_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER7_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER7_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER8_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER8_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER8_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER9_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER9_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER9_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER10_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER10_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER10_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER11_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER11_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER11_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER12_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER12_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER12_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER13_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER13_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER13_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER14_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER14_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER14_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER15_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER15_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER15_CTRL2 },
    { "XPT_FE_IB_SYNC_DETECT_CTRL",              BCHP_XPT_FE_IB_SYNC_DETECT_CTRL,                   BCHP_XPT_FE_IB_SYNC_DETECT_CTRL },
    { "XPT_FE_TSMF0_CTRL",                       BCHP_XPT_FE_TSMF0_CTRL,                            BCHP_XPT_FE_TSMF0_SLOT_MAP_HI },
    { "XPT_FE_TSMF1_CTRL",                       BCHP_XPT_FE_TSMF1_CTRL,                            BCHP_XPT_FE_TSMF1_SLOT_MAP_HI },
    { "XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID", BCHP_XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID,   BCHP_XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID },
    { "XPT_FE_ATS_COUNTER_CTRL",                 BCHP_XPT_FE_ATS_COUNTER_CTRL,                      BCHP_XPT_FE_ATS_TS_BINARY },
    { "XPT_FE_MTSIF_RX0_CTRL1",                  BCHP_XPT_FE_MTSIF_RX0_CTRL1,                       BCHP_XPT_FE_MTSIF_RX0_CTRL1 },
    { "XPT_FE_MTSIF_RX0_INTR_STATUS0_REG_EN",    BCHP_XPT_FE_MTSIF_RX0_INTR_STATUS0_REG_EN,         BCHP_XPT_FE_MTSIF_RX0_BAND_ID_DROP },
    { "XPT_FE_MTSIF_RX1_CTRL1",                  BCHP_XPT_FE_MTSIF_RX1_CTRL1,                       BCHP_XPT_FE_MTSIF_RX1_CTRL1 },
    { "XPT_FE_MTSIF_RX1_INTR_STATUS0_REG_EN",    BCHP_XPT_FE_MTSIF_RX1_INTR_STATUS0_REG_EN,         BCHP_XPT_FE_MTSIF_RX1_BAND_ID_DROP },
    { "XPT_DPCR0_PID_CH",                        BCHP_XPT_DPCR0_PID_CH,                             BCHP_XPT_DPCR0_CTRL },
    { "XPT_DPCR0_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR0_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR0_STC_EXT_CTRL },
    { "XPT_DPCR0_MAX_PCR_ERROR",                 BCHP_XPT_DPCR0_MAX_PCR_ERROR,                      BCHP_XPT_DPCR0_MAX_PCR_ERROR },
    { "XPT_DPCR0_STC_HI",                        BCHP_XPT_DPCR0_STC_HI,                             BCHP_XPT_DPCR0_STC_LO },
    { "XPT_DPCR0_LOOP_CTRL",                     BCHP_XPT_DPCR0_LOOP_CTRL,                          BCHP_XPT_DPCR0_ACCUM_VALUE },
    { "XPT_DPCR0_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR0_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR0_PHASE_ERROR_CLAMP },
    { "XPT_DPCR1_PID_CH",                        BCHP_XPT_DPCR1_PID_CH,                             BCHP_XPT_DPCR1_CTRL },
    { "XPT_DPCR1_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR1_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR1_STC_EXT_CTRL },
    { "XPT_DPCR1_MAX_PCR_ERROR",                 BCHP_XPT_DPCR1_MAX_PCR_ERROR,                      BCHP_XPT_DPCR1_MAX_PCR_ERROR },
    { "XPT_DPCR1_STC_HI",                        BCHP_XPT_DPCR1_STC_HI,                             BCHP_XPT_DPCR1_STC_LO },
    { "XPT_DPCR1_LOOP_CTRL",                     BCHP_XPT_DPCR1_LOOP_CTRL,                          BCHP_XPT_DPCR1_ACCUM_VALUE },
    { "XPT_DPCR1_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR1_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR1_PHASE_ERROR_CLAMP },
    { "XPT_DPCR2_PID_CH",                        BCHP_XPT_DPCR2_PID_CH,                             BCHP_XPT_DPCR2_CTRL },
    { "XPT_DPCR2_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR2_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR2_STC_EXT_CTRL },
    { "XPT_DPCR2_MAX_PCR_ERROR",                 BCHP_XPT_DPCR2_MAX_PCR_ERROR,                      BCHP_XPT_DPCR2_MAX_PCR_ERROR },
    { "XPT_DPCR2_STC_HI",                        BCHP_XPT_DPCR2_STC_HI,                             BCHP_XPT_DPCR2_STC_LO },
    { "XPT_DPCR2_LOOP_CTRL",                     BCHP_XPT_DPCR2_LOOP_CTRL,                          BCHP_XPT_DPCR2_ACCUM_VALUE },
    { "XPT_DPCR2_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR2_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR2_PHASE_ERROR_CLAMP },
    { "XPT_DPCR3_PID_CH",                        BCHP_XPT_DPCR3_PID_CH,                             BCHP_XPT_DPCR3_CTRL },
    { "XPT_DPCR3_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR3_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR3_STC_EXT_CTRL },
    { "XPT_DPCR3_MAX_PCR_ERROR",                 BCHP_XPT_DPCR3_MAX_PCR_ERROR,                      BCHP_XPT_DPCR3_MAX_PCR_ERROR },
    { "XPT_DPCR3_STC_HI",                        BCHP_XPT_DPCR3_STC_HI,                             BCHP_XPT_DPCR3_STC_LO },
    { "XPT_DPCR3_LOOP_CTRL",                     BCHP_XPT_DPCR3_LOOP_CTRL,                          BCHP_XPT_DPCR3_ACCUM_VALUE },
    { "XPT_DPCR3_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR3_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR3_PHASE_ERROR_CLAMP },
    { "XPT_DPCR_PP_PP_CTRL",                     BCHP_XPT_DPCR_PP_PP_CTRL,                          BCHP_XPT_DPCR_PP_PP_FIXED_OFFSET },
    { "XPT_PSUB_PSUB0_CTRL0",                    BCHP_XPT_PSUB_PSUB0_CTRL0,                         BCHP_XPT_PSUB_PSUB0_STAT0 },
    { "XPT_PSUB_PSUB1_CTRL0",                    BCHP_XPT_PSUB_PSUB1_CTRL0,                         BCHP_XPT_PSUB_PSUB1_STAT0 },
    { "XPT_PSUB_PSUB2_CTRL0",                    BCHP_XPT_PSUB_PSUB2_CTRL0,                         BCHP_XPT_PSUB_PSUB2_STAT0 },
    { "XPT_PSUB_PSUB3_CTRL0",                    BCHP_XPT_PSUB_PSUB3_CTRL0,                         BCHP_XPT_PSUB_PSUB3_STAT0 },
    { "XPT_PSUB_PSUB4_CTRL0",                    BCHP_XPT_PSUB_PSUB4_CTRL0,                         BCHP_XPT_PSUB_PSUB4_STAT0 },
    { "XPT_PSUB_PSUB5_CTRL0",                    BCHP_XPT_PSUB_PSUB5_CTRL0,                         BCHP_XPT_PSUB_PSUB5_STAT0 },
    { "XPT_PSUB_PSUB6_CTRL0",                    BCHP_XPT_PSUB_PSUB6_CTRL0,                         BCHP_XPT_PSUB_PSUB6_STAT0 },
    { "XPT_PSUB_PSUB7_CTRL0",                    BCHP_XPT_PSUB_PSUB7_CTRL0,                         BCHP_XPT_PSUB_PSUB7_STAT0 },
    { "XPT_PSUB_PR_FALLBACK_CTRL",               BCHP_XPT_PSUB_PR_FALLBACK_CTRL,                    BCHP_XPT_PSUB_PR_FALLBACK_CTRL },
    { "XPT_RSBUFF_BO_IBP0",                      BCHP_XPT_RSBUFF_BO_IBP0,                           BCHP_XPT_RSBUFF_BO_IBP15 },
    { "XPT_RSBUFF_BO_PBP0",                      BCHP_XPT_RSBUFF_BO_PBP0,                           BCHP_XPT_RSBUFF_BO_PBP3 },
    { "XPT_RSBUFF_IBP_BUFFER_ENABLE",            BCHP_XPT_RSBUFF_IBP_BUFFER_ENABLE,                 BCHP_XPT_RSBUFF_PBP_BUFFER_ENABLE },
    { "XPT_RSBUFF_OVERFLOW_THRESHOLD",           BCHP_XPT_RSBUFF_OVERFLOW_THRESHOLD,                BCHP_XPT_RSBUFF_NO_RD_HANG_CTRL },
    { "XPT_RSBUFF_PR_FALLBACK_CTRL",             BCHP_XPT_RSBUFF_PR_FALLBACK_CTRL,                  BCHP_XPT_RSBUFF_PR_FALLBACK_CTRL },
    { "XPT_PB0_CTRL1",                           BCHP_XPT_PB0_CTRL1,                                BCHP_XPT_PB0_FIRST_DESC_ADDR },
    { "XPT_PB0_BLOCKOUT",                        BCHP_XPT_PB0_BLOCKOUT,                             BCHP_XPT_PB0_PARSER_CTRL2 },
    { "XPT_PB0_INTR_EN",                         BCHP_XPT_PB0_INTR_EN,                              BCHP_XPT_PB0_PWR_CTRL },
    { "XPT_PB0_ASF_CTRL",                        BCHP_XPT_PB0_ASF_CTRL,                             BCHP_XPT_PB0_PLAYBACK_PARSER_BAND_ID },
    { "XPT_PB1_CTRL1",                           BCHP_XPT_PB1_CTRL1,                                BCHP_XPT_PB1_FIRST_DESC_ADDR },
    { "XPT_PB1_BLOCKOUT",                        BCHP_XPT_PB1_BLOCKOUT,                             BCHP_XPT_PB1_PARSER_CTRL2 },
    { "XPT_PB1_INTR_EN",                         BCHP_XPT_PB1_INTR_EN,                              BCHP_XPT_PB1_PWR_CTRL },
    { "XPT_PB1_ASF_CTRL",                        BCHP_XPT_PB1_ASF_CTRL,                             BCHP_XPT_PB1_PLAYBACK_PARSER_BAND_ID },
    { "XPT_PB2_CTRL1",                           BCHP_XPT_PB2_CTRL1,                                BCHP_XPT_PB2_FIRST_DESC_ADDR },
    { "XPT_PB2_BLOCKOUT",                        BCHP_XPT_PB2_BLOCKOUT,                             BCHP_XPT_PB2_PARSER_CTRL2 },
    { "XPT_PB2_INTR_EN",                         BCHP_XPT_PB2_INTR_EN,                              BCHP_XPT_PB2_PWR_CTRL },
    { "XPT_PB2_ASF_CTRL",                        BCHP_XPT_PB2_ASF_CTRL,                             BCHP_XPT_PB2_PLAYBACK_PARSER_BAND_ID },
    { "XPT_PB3_CTRL1",                           BCHP_XPT_PB3_CTRL1,                                BCHP_XPT_PB3_FIRST_DESC_ADDR },
    { "XPT_PB3_BLOCKOUT",                        BCHP_XPT_PB3_BLOCKOUT,                             BCHP_XPT_PB3_PARSER_CTRL2 },
    { "XPT_PB3_INTR_EN",                         BCHP_XPT_PB3_INTR_EN,                              BCHP_XPT_PB3_PWR_CTRL },
    { "XPT_PB3_ASF_CTRL",                        BCHP_XPT_PB3_ASF_CTRL,                             BCHP_XPT_PB3_PLAYBACK_PARSER_BAND_ID },
    { "XPT_MPOD_CFG",                            BCHP_XPT_MPOD_CFG,                                 BCHP_XPT_MPOD_ICTRL },
    { "XPT_MPOD_OCTRL2",                         BCHP_XPT_MPOD_OCTRL2,                              BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP },
    { "XPT_RMX0_CTRL",                           BCHP_XPT_RMX0_CTRL,                                BCHP_XPT_RMX0_FIXED_OFFSET },
    { "XPT_RMX1_CTRL",                           BCHP_XPT_RMX1_CTRL,                                BCHP_XPT_RMX1_FIXED_OFFSET },
    { "XPT_XCBUFF_RAVE_CTRL_BUFFER_EN_PBP",      BCHP_XPT_XCBUFF_RAVE_CTRL_BUFFER_EN_PBP,           BCHP_XPT_XCBUFF_PAUSE_THRESHOLD },
    { "XPT_XCBUFF_OVERFLOW_THRESHOLD",           BCHP_XPT_XCBUFF_OVERFLOW_THRESHOLD,                BCHP_XPT_XCBUFF_OVERFLOW_THRESHOLD },
    { "XPT_XCBUFF_BYTE_SWAP_CTRL",               BCHP_XPT_XCBUFF_BYTE_SWAP_CTRL,                    BCHP_XPT_XCBUFF_BO_RAVE_IBP15 },
    { "XPT_XCBUFF_BO_RAVE_PBP0",                 BCHP_XPT_XCBUFF_BO_RAVE_PBP0,                      BCHP_XPT_XCBUFF_BO_RAVE_PBP3 },
    { "XPT_XCBUFF_BO_MSG_IBP0",                  BCHP_XPT_XCBUFF_BO_MSG_IBP0,                       BCHP_XPT_XCBUFF_BO_MSG_IBP15 },
    { "XPT_XCBUFF_BO_MSG_PBP0",                  BCHP_XPT_XCBUFF_BO_MSG_PBP0,                       BCHP_XPT_XCBUFF_BO_MSG_PBP3 },
    { "XPT_XCBUFF_BO_RMX0_IBP0",                 BCHP_XPT_XCBUFF_BO_RMX0_IBP0,                      BCHP_XPT_XCBUFF_BO_RMX0_IBP15 },
    { "XPT_XCBUFF_BO_RMX0_PBP0",                 BCHP_XPT_XCBUFF_BO_RMX0_PBP0,                      BCHP_XPT_XCBUFF_BO_RMX0_PBP3 },
    { "XPT_XCBUFF_BO_RMX1_IBP0",                 BCHP_XPT_XCBUFF_BO_RMX1_IBP0,                      BCHP_XPT_XCBUFF_BO_RMX1_IBP15 },
    { "XPT_XCBUFF_BO_RMX1_PBP0",                 BCHP_XPT_XCBUFF_BO_RMX1_PBP0,                      BCHP_XPT_XCBUFF_BO_RMX1_PBP3 },
    { "XPT_XCBUFF_NO_RD_HANG_CTRL",              BCHP_XPT_XCBUFF_NO_RD_HANG_CTRL,                   BCHP_XPT_XCBUFF_NO_RD_HANG_CTRL },
    { "XPT_XCBUFF_PR_FALLBACK_CTRL",             BCHP_XPT_XCBUFF_PR_FALLBACK_CTRL,                  BCHP_XPT_XCBUFF_PR_FALLBACK_CTRL },
    { "XPT_RAVE_AVS_SCV_FILTER_MODE_CONTROL",    BCHP_XPT_RAVE_AVS_SCV_FILTER_MODE_CONTROL,         BCHP_XPT_RAVE_AVS_SCV_FILTER_VALUE_4_TO_7 },
    { "XPT_RAVE_DATA_START_ADDR_A",              BCHP_XPT_RAVE_DATA_START_ADDR_A,                   BCHP_XPT_RAVE_WATCHDOG_TIMER_VALUE },
    { "XPT_RAVE_MISC_CONTROL",                   BCHP_XPT_RAVE_MISC_CONTROL,                        BCHP_XPT_RAVE_BASE_ADDRESSES },
    { "XPT_RAVE_MISC_CONTROL2",                  BCHP_XPT_RAVE_MISC_CONTROL2,                       BCHP_XPT_RAVE_MISC_CONTROL3 },
    { "XPT_RAVE_RC0_SP_CONTROL",                 BCHP_XPT_RAVE_RC0_SP_CONTROL,                      BCHP_XPT_RAVE_RC8_SP_CONTROL },
    { "XPT_RAVE_ATSOFFSET_MAX_ERROR",            BCHP_XPT_RAVE_ATSOFFSET_MAX_ERROR,                 BCHP_XPT_RAVE_ATSOFFSET_MAX_ERROR },
    { "XPT_RAVE_TPIT_TIME_TICK",                 BCHP_XPT_RAVE_TPIT_TIME_TICK,                      BCHP_XPT_RAVE_TPIT_EVE_TIMEOUT },
    { "XPT_RAVE_EMM_TID_MODE",                   BCHP_XPT_RAVE_EMM_TID_MODE,                        BCHP_XPT_RAVE_EMM_CTRL_ID },
    { "XPT_RAVE_EMM_DATA_ID_1",                  BCHP_XPT_RAVE_EMM_DATA_ID_1,                       BCHP_XPT_RAVE_EMM_MASK_ID_8 },
    { "XPT_PCROFFSET_INTERRUPT0_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT0_ENABLE,              BCHP_XPT_PCROFFSET_INTERRUPT0_ENABLE },
    { "XPT_PCROFFSET_INTERRUPT1_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT1_ENABLE,              BCHP_XPT_PCROFFSET_INTERRUPT1_ENABLE },
    { "XPT_PCROFFSET_INTERRUPT2_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT2_ENABLE,              BCHP_XPT_PCROFFSET_INTERRUPT2_ENABLE },
    { "XPT_PCROFFSET_INTERRUPT3_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT3_ENABLE,              BCHP_XPT_PCROFFSET_STC3 },
    { "XPT_PCROFFSET_STC_BROADCAST_SEL",         BCHP_XPT_PCROFFSET_STC_BROADCAST_SEL,              BCHP_XPT_PCROFFSET_STC_BROADCAST_SEL },
    { "XPT_PCROFFSET_STC_INTERRUPT_ENABLE",      BCHP_XPT_PCROFFSET_STC_INTERRUPT_ENABLE,           BCHP_XPT_PCROFFSET_STC3_MATCH },
    { "XPT_MSG_MSG_CTRL1",                       BCHP_XPT_MSG_MSG_CTRL1,                            BCHP_XPT_MSG_DMA_BUFFER_INIT },
    { "XPT_MSG_PHY_ADDR_LO",                     BCHP_XPT_MSG_PHY_ADDR_LO,                          BCHP_XPT_MSG_ID_REJECT },
    { "XPT_MSG_C_MATCH0",                        BCHP_XPT_MSG_C_MATCH0,                             BCHP_XPT_MSG_C_MATCH4 },
    { "XPT_MSG_BUF_DAT_RDY_INTR_EN_00_31",       BCHP_XPT_MSG_BUF_DAT_RDY_INTR_EN_00_31,            BCHP_XPT_MSG_BUF_DAT_RDY_INTR_EN_96_127 },
    { "XPT_MSG_BUF_OVFL_INTR_EN_00_31",          BCHP_XPT_MSG_BUF_OVFL_INTR_EN_00_31,               BCHP_XPT_MSG_BUF_OVFL_INTR_EN_96_127 },
    { "XPT_MSG_DAT_ERR_INTR_EN",                 BCHP_XPT_MSG_DAT_ERR_INTR_EN,                      BCHP_XPT_MSG_DAT_ERR_INTR_EN },
    { "XPT_MSG_MSG_EVENT_CNT_CTRL",              BCHP_XPT_MSG_MSG_EVENT_CNT_CTRL,                   BCHP_XPT_MSG_MSG_EVENT_CNT_CTRL },
    { "XPT_GR_CTRL",                             BCHP_XPT_GR_CTRL,                                  BCHP_XPT_GR_CTRL },
    { "XPT_FULL_PID_PARSER_PARSER_CFG",          BCHP_XPT_FULL_PID_PARSER_PARSER_CFG,               BCHP_XPT_FULL_PID_PARSER_PBP_ACCEPT_ADAPT_00 },
    { "XPT_FULL_PID_PARSER_IBP_PCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_PCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_PCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_PCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_PCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_PCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_IBP_SCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_SCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_SCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_SCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_SCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_SCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_IBP_PSG_PROTOCOL_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_PSG_PROTOCOL_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_PSG_PROTOCOL_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_PSG_PROTOCOL_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_PSG_PROTOCOL_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_PSG_PROTOCOL_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_IBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PSG_ERR_MODE",        BCHP_XPT_FULL_PID_PARSER_PSG_ERR_MODE,             BCHP_XPT_FULL_PID_PARSER_PSG_ERR_MODE },
    {0, 0, 0}
};

#elif ((BCHP_CHIP==7346 && BCHP_VER>=BCHP_VER_B0)) || (BCHP_CHIP==73465)
#include "bchp_xpt_bus_if.h"
#include "bchp_xpt_dpcr0.h"
#include "bchp_xpt_dpcr1.h"
#include "bchp_xpt_dpcr2.h"
#include "bchp_xpt_dpcr3.h"
#include "bchp_xpt_dpcr4.h"
#include "bchp_xpt_dpcr5.h"
#include "bchp_xpt_dpcr6.h"
#include "bchp_xpt_dpcr7.h"
#include "bchp_xpt_dpcr_pp.h"
#include "bchp_xpt_fe.h"
#include "bchp_xpt_full_pid_parser.h"
#include "bchp_xpt_gr.h"
#include "bchp_xpt_mpod.h"
#include "bchp_xpt_msg.h"
#include "bchp_xpt_pb0.h"
#include "bchp_xpt_pb1.h"
#include "bchp_xpt_pb10.h"
#include "bchp_xpt_pb11.h"
#include "bchp_xpt_pb12.h"
#include "bchp_xpt_pb13.h"
#include "bchp_xpt_pb14.h"
#include "bchp_xpt_pb15.h"
#include "bchp_xpt_pb2.h"
#include "bchp_xpt_pb3.h"
#include "bchp_xpt_pb4.h"
#include "bchp_xpt_pb5.h"
#include "bchp_xpt_pb6.h"
#include "bchp_xpt_pb7.h"
#include "bchp_xpt_pb8.h"
#include "bchp_xpt_pb9.h"
#include "bchp_xpt_pb_top.h"
#include "bchp_xpt_pcroffset.h"
#include "bchp_xpt_pmu.h"
#include "bchp_xpt_psub.h"
#include "bchp_xpt_rave.h"
#include "bchp_xpt_rmx0.h"
#include "bchp_xpt_rmx0_io.h"
#include "bchp_xpt_rmx1.h"
#include "bchp_xpt_rmx1_io.h"
#include "bchp_xpt_rsbuff.h"
#include "bchp_xpt_secure_bus_if.h"
#include "bchp_xpt_wakeup.h"
#include "bchp_xpt_xcbuff.h"
#include "bchp_xpt_xmemif.h"
#include "bchp_xpt_xpu.h"

const BXPT_P_RegisterRange XPT_SRAM_LIST[] =
{
    { "WAKEUP_MEM",                              BCHP_XPT_WAKEUP_PKT_MEM_i_ARRAY_BASE,              (BCHP_XPT_WAKEUP_PKT_TYPE3_i_ARRAY_BASE+(BCHP_XPT_WAKEUP_PKT_TYPE3_i_ARRAY_END*4)) },
    { "FE_PID_TABLE",                            BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE,                (BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE+(BCHP_XPT_FE_PID_TABLE_i_ARRAY_END*4)) },
    { "FE_SPID_TABLE",                           BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE,               (BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE+(BCHP_XPT_FE_SPID_TABLE_i_ARRAY_END*4)) },
    { "PCROFFSET_PID_TABLE",                     BCHP_XPT_PCROFFSET_PID_CONFIG_TABLE_i_ARRAY_BASE,  (BCHP_XPT_PCROFFSET_PID_CONFIG_TABLE_i_ARRAY_BASE+(BCHP_XPT_PCROFFSET_PID_CONFIG_TABLE_i_ARRAY_END*4)) },
    { "PCROFFSET_CX_TABLE",                      BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL,        BCHP_XPT_PCROFFSET_CONTEXT15_RESERVED_CFG_0 },
    { "FPP_SCM0",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_END*4)) },
    { "FPP_SCM1",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_1_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_1_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_1_i_ARRAY_END*4)) },
    { "FPP_SCM2",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_2_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_2_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_2_i_ARRAY_END*4)) },
    { "FPP_SCM3",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_3_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_3_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_3_i_ARRAY_END*4)) },
    { "RSBUFF_IBP_PMEM",                         BCHP_XPT_RSBUFF_BASE_POINTER_IBP0,                 BCHP_XPT_RSBUFF_WATERMARK_IBP15 },
    { "RSBUFF_PBP_PMEM",                         BCHP_XPT_RSBUFF_BASE_POINTER_PBP0,                 BCHP_XPT_RSBUFF_WATERMARK_PBP15 },
    { "RSBUFF_MPOD_PMEM",                        BCHP_XPT_RSBUFF_BASE_POINTER_MPOD_IBP0,            BCHP_XPT_RSBUFF_WATERMARK_MPOD_IBP15 },
    { "XCBUFF_RAVE_IBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RAVE_IBP0,            BCHP_XPT_XCBUFF_WATERMARK_RAVE_IBP15 },
    { "XCBUFF_RAVE_PBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RAVE_PBP0,            BCHP_XPT_XCBUFF_WATERMARK_RAVE_PBP15 },
    { "XCBUFF_MSG_IBP_PMEM",                     BCHP_XPT_XCBUFF_BASE_POINTER_MSG_IBP0,             BCHP_XPT_XCBUFF_WATERMARK_MSG_IBP15 },
    { "XCBUFF_MSG_PBP_PMEM",                     BCHP_XPT_XCBUFF_BASE_POINTER_MSG_PBP0,             BCHP_XPT_XCBUFF_WATERMARK_MSG_PBP15 },
    { "XCBUFF_RMX0_IBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX0_IBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX0_IBP15 },
    { "XCBUFF_RMX0_PBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX0_PBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX0_PBP15 },
    { "XCBUFF_RMX1_IBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX1_IBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX1_IBP15 },
    { "XCBUFF_RMX1_PBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX1_PBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX1_PBP15 },
    { "MSG_PMEM",                                BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_ARRAY_BASE,         (BCHP_XPT_MSG_DMA_VP_TABLE_i_ARRAY_BASE+(BCHP_XPT_MSG_DMA_VP_TABLE_i_ARRAY_END*4)) },
    { "MSG_GEN_FILT1",                           BCHP_XPT_MSG_GEN_FILT_EN_i_ARRAY_BASE,             (BCHP_XPT_MSG_GEN_FILT_EN_i_ARRAY_BASE+(BCHP_XPT_MSG_GEN_FILT_EN_i_ARRAY_END*4)) },
    { "MSG_GEN_FILT2",                           BCHP_XPT_MSG_MCAST_16_ADDR_i_ARRAY_BASE,           (BCHP_XPT_MSG_GEN_FILT_EXCL_i_ARRAY_BASE+(BCHP_XPT_MSG_GEN_FILT_EXCL_i_ARRAY_END*4)) },
    { "MSG_PID2BUF",                             BCHP_XPT_MSG_PID2BUF_MAP_i_ARRAY_BASE,             (BCHP_XPT_MSG_PID2BUF_MAP_i_ARRAY_BASE+(BCHP_XPT_MSG_PID2BUF_MAP_i_ARRAY_END*4)) },
    { "RAVE_PMEM",                               BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR,                BCHP_XPT_RAVE_SCD63_RESERVED_STATE3 },
    { "RAVE_CXMEM_A",                            BCHP_XPT_RAVE_CXMEM_Ai_ARRAY_BASE,                 (BCHP_XPT_RAVE_CXMEM_Ai_ARRAY_BASE+(BCHP_XPT_RAVE_CXMEM_Ai_ARRAY_END*4)) },
    { "RAVE_CXMEM_B",                            BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_BASE,                 (BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_BASE+(BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_END*4)) },
    { "RAVE_CXMEM_C",                            BCHP_XPT_RAVE_CXMEM_Ci_ARRAY_BASE,                 (BCHP_XPT_RAVE_CXMEM_Ci_ARRAY_BASE+(BCHP_XPT_RAVE_CXMEM_Ci_ARRAY_END*4)) },
    { "RAVE_DMEM",                               BCHP_XPT_RAVE_DMEMi_ARRAY_BASE,                    (BCHP_XPT_RAVE_DMEMi_ARRAY_BASE+(BCHP_XPT_RAVE_DMEMi_ARRAY_END*4)) },
    { "RAVE_SMEM",                               BCHP_XPT_RAVE_TPIT0_PID_TABLEi_ARRAY_BASE,         BCHP_XPT_RAVE_TPIT_STATE_CONTEXT47 },
    {0, 0, 0}
};

const BXPT_P_RegisterRange XPT_REG_SAVE_LIST[] =
{
    { "XPT_BUS_IF_MISC_CTRL0",                   BCHP_XPT_BUS_IF_MISC_CTRL0,                        BCHP_XPT_BUS_IF_TEST_MODE },
    { "XPT_BUS_IF_INTR_STATUS_REG_EN",           BCHP_XPT_BUS_IF_INTR_STATUS_REG_EN,                BCHP_XPT_BUS_IF_INTR_STATUS_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS2_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS2_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS2_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS3_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS3_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS3_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS4_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS4_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS4_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS5_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS5_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS5_REG_EN },
    { "XPT_XMEMIF_SCB_WR_ARB_SEL_RSBUFF",        BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RSBUFF,             BCHP_XPT_XMEMIF_SCB_RD_ARB_MODE },
    { "XPT_XMEMIF_INTR_STATUS_REG_EN",           BCHP_XPT_XMEMIF_INTR_STATUS_REG_EN,                BCHP_XPT_XMEMIF_INTR_STATUS_REG_EN },
    { "XPT_PMU_CLK_CTRL",                        BCHP_XPT_PMU_CLK_CTRL,                             BCHP_XPT_PMU_RBUS_RSP_VAL },
    { "XPT_GR_CTRL",                             BCHP_XPT_GR_CTRL,                                  BCHP_XPT_GR_CTRL },
    { "XPT_RMX0_IO_FORMAT",                      BCHP_XPT_RMX0_IO_FORMAT,                           BCHP_XPT_RMX0_IO_PKT_DLY_CNT },
    { "XPT_RMX1_IO_FORMAT",                      BCHP_XPT_RMX1_IO_FORMAT,                           BCHP_XPT_RMX1_IO_PKT_DLY_CNT },
    { "XPT_RMX1_IO_TV_STATUS",                   BCHP_XPT_RMX1_IO_TV_STATUS,                        BCHP_XPT_RMX1_IO_TV_STATUS },
    { "XPT_WAKEUP_CTRL",                         BCHP_XPT_WAKEUP_CTRL,                              BCHP_XPT_WAKEUP_CTRL },
    { "XPT_WAKEUP_INTR_STATUS_REG_EN",           BCHP_XPT_WAKEUP_INTR_STATUS_REG_EN,                BCHP_XPT_WAKEUP_INTR_STATUS_REG_EN },
    { "XPT_FE_FE_CTRL",                          BCHP_XPT_FE_FE_CTRL,                               BCHP_XPT_FE_FE_CTRL },
    { "XPT_FE_INTR_STATUS0_REG_EN",              BCHP_XPT_FE_INTR_STATUS0_REG_EN,                   BCHP_XPT_FE_PWR_CTRL },
    { "XPT_FE_MAX_PID_CHANNEL",                  BCHP_XPT_FE_MAX_PID_CHANNEL,                       BCHP_XPT_FE_MAX_PID_CHANNEL },
    { "XPT_FE_IB0_CTRL",                         BCHP_XPT_FE_IB0_CTRL,                              BCHP_XPT_FE_IB0_CTRL },
    { "XPT_FE_IB1_CTRL",                         BCHP_XPT_FE_IB1_CTRL,                              BCHP_XPT_FE_IB1_CTRL },
    { "XPT_FE_IB2_CTRL",                         BCHP_XPT_FE_IB2_CTRL,                              BCHP_XPT_FE_IB2_CTRL },
    { "XPT_FE_IB3_CTRL",                         BCHP_XPT_FE_IB3_CTRL,                              BCHP_XPT_FE_IB3_CTRL },
    { "XPT_FE_IB4_CTRL",                         BCHP_XPT_FE_IB4_CTRL,                              BCHP_XPT_FE_IB4_CTRL },
    { "XPT_FE_IB5_CTRL",                         BCHP_XPT_FE_IB5_CTRL,                              BCHP_XPT_FE_IB5_CTRL },
    { "XPT_FE_IB6_CTRL",                         BCHP_XPT_FE_IB6_CTRL,                              BCHP_XPT_FE_IB6_CTRL },
    { "XPT_FE_IB7_CTRL",                         BCHP_XPT_FE_IB7_CTRL,                              BCHP_XPT_FE_IB7_CTRL },
    { "XPT_FE_IB8_CTRL",                         BCHP_XPT_FE_IB8_CTRL,                              BCHP_XPT_FE_IB8_CTRL },
    { "XPT_FE_IB9_CTRL",                         BCHP_XPT_FE_IB9_CTRL,                              BCHP_XPT_FE_IB9_CTRL },
    { "XPT_FE_IB10_CTRL",                        BCHP_XPT_FE_IB10_CTRL,                             BCHP_XPT_FE_IB10_CTRL },
    { "XPT_FE_MINI_PID_PARSER0_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER0_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER0_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER1_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER1_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER1_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER2_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER2_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER2_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER3_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER3_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER3_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER4_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER4_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER4_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER5_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER5_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER5_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER6_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER6_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER6_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER7_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER7_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER7_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER8_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER8_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER8_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER9_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER9_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER9_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER10_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER10_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER10_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER11_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER11_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER11_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER12_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER12_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER12_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER13_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER13_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER13_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER14_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER14_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER14_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER15_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER15_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER15_CTRL2 },
    { "XPT_FE_IB_SYNC_DETECT_CTRL",              BCHP_XPT_FE_IB_SYNC_DETECT_CTRL,                   BCHP_XPT_FE_IB_SYNC_DETECT_CTRL },
    { "XPT_FE_TSMF0_CTRL",                       BCHP_XPT_FE_TSMF0_CTRL,                            BCHP_XPT_FE_TSMF0_SLOT_MAP_HI },
    { "XPT_FE_TSMF1_CTRL",                       BCHP_XPT_FE_TSMF1_CTRL,                            BCHP_XPT_FE_TSMF1_SLOT_MAP_HI },
    { "XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID", BCHP_XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID,   BCHP_XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID },
    { "XPT_FE_ATS_COUNTER_CTRL",                 BCHP_XPT_FE_ATS_COUNTER_CTRL,                      BCHP_XPT_FE_ATS_TS_BINARY },
    { "XPT_FE_MTSIF_RX0_CTRL1",                  BCHP_XPT_FE_MTSIF_RX0_CTRL1,                       BCHP_XPT_FE_MTSIF_RX0_CTRL1 },
    { "XPT_FE_MTSIF_RX0_INTR_STATUS0_REG_EN",    BCHP_XPT_FE_MTSIF_RX0_INTR_STATUS0_REG_EN,         BCHP_XPT_FE_MTSIF_RX0_BAND_ID_DROP },
    { "XPT_FE_MTSIF_RX1_CTRL1",                  BCHP_XPT_FE_MTSIF_RX1_CTRL1,                       BCHP_XPT_FE_MTSIF_RX1_CTRL1 },
    { "XPT_FE_MTSIF_RX1_INTR_STATUS0_REG_EN",    BCHP_XPT_FE_MTSIF_RX1_INTR_STATUS0_REG_EN,         BCHP_XPT_FE_MTSIF_RX1_BAND_ID_DROP },
    { "XPT_DPCR0_PID_CH",                        BCHP_XPT_DPCR0_PID_CH,                             BCHP_XPT_DPCR0_CTRL },
    { "XPT_DPCR0_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR0_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR0_STC_EXT_CTRL },
    { "XPT_DPCR0_MAX_PCR_ERROR",                 BCHP_XPT_DPCR0_MAX_PCR_ERROR,                      BCHP_XPT_DPCR0_MAX_PCR_ERROR },
    { "XPT_DPCR0_STC_HI",                        BCHP_XPT_DPCR0_STC_HI,                             BCHP_XPT_DPCR0_STC_LO },
    { "XPT_DPCR0_LOOP_CTRL",                     BCHP_XPT_DPCR0_LOOP_CTRL,                          BCHP_XPT_DPCR0_ACCUM_VALUE },
    { "XPT_DPCR0_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR0_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR0_PHASE_ERROR_CLAMP },
    { "XPT_DPCR1_PID_CH",                        BCHP_XPT_DPCR1_PID_CH,                             BCHP_XPT_DPCR1_CTRL },
    { "XPT_DPCR1_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR1_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR1_STC_EXT_CTRL },
    { "XPT_DPCR1_MAX_PCR_ERROR",                 BCHP_XPT_DPCR1_MAX_PCR_ERROR,                      BCHP_XPT_DPCR1_MAX_PCR_ERROR },
    { "XPT_DPCR1_STC_HI",                        BCHP_XPT_DPCR1_STC_HI,                             BCHP_XPT_DPCR1_STC_LO },
    { "XPT_DPCR1_LOOP_CTRL",                     BCHP_XPT_DPCR1_LOOP_CTRL,                          BCHP_XPT_DPCR1_ACCUM_VALUE },
    { "XPT_DPCR1_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR1_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR1_PHASE_ERROR_CLAMP },
    { "XPT_DPCR2_PID_CH",                        BCHP_XPT_DPCR2_PID_CH,                             BCHP_XPT_DPCR2_CTRL },
    { "XPT_DPCR2_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR2_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR2_STC_EXT_CTRL },
    { "XPT_DPCR2_MAX_PCR_ERROR",                 BCHP_XPT_DPCR2_MAX_PCR_ERROR,                      BCHP_XPT_DPCR2_MAX_PCR_ERROR },
    { "XPT_DPCR2_STC_HI",                        BCHP_XPT_DPCR2_STC_HI,                             BCHP_XPT_DPCR2_STC_LO },
    { "XPT_DPCR2_LOOP_CTRL",                     BCHP_XPT_DPCR2_LOOP_CTRL,                          BCHP_XPT_DPCR2_ACCUM_VALUE },
    { "XPT_DPCR2_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR2_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR2_PHASE_ERROR_CLAMP },
    { "XPT_DPCR3_PID_CH",                        BCHP_XPT_DPCR3_PID_CH,                             BCHP_XPT_DPCR3_CTRL },
    { "XPT_DPCR3_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR3_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR3_STC_EXT_CTRL },
    { "XPT_DPCR3_MAX_PCR_ERROR",                 BCHP_XPT_DPCR3_MAX_PCR_ERROR,                      BCHP_XPT_DPCR3_MAX_PCR_ERROR },
    { "XPT_DPCR3_STC_HI",                        BCHP_XPT_DPCR3_STC_HI,                             BCHP_XPT_DPCR3_STC_LO },
    { "XPT_DPCR3_LOOP_CTRL",                     BCHP_XPT_DPCR3_LOOP_CTRL,                          BCHP_XPT_DPCR3_ACCUM_VALUE },
    { "XPT_DPCR3_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR3_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR3_PHASE_ERROR_CLAMP },
    { "XPT_DPCR4_PID_CH",                        BCHP_XPT_DPCR4_PID_CH,                             BCHP_XPT_DPCR4_CTRL },
    { "XPT_DPCR4_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR4_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR4_STC_EXT_CTRL },
    { "XPT_DPCR4_MAX_PCR_ERROR",                 BCHP_XPT_DPCR4_MAX_PCR_ERROR,                      BCHP_XPT_DPCR4_MAX_PCR_ERROR },
    { "XPT_DPCR4_STC_HI",                        BCHP_XPT_DPCR4_STC_HI,                             BCHP_XPT_DPCR4_STC_LO },
    { "XPT_DPCR4_LOOP_CTRL",                     BCHP_XPT_DPCR4_LOOP_CTRL,                          BCHP_XPT_DPCR4_ACCUM_VALUE },
    { "XPT_DPCR4_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR4_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR4_PHASE_ERROR_CLAMP },
    { "XPT_DPCR5_PID_CH",                        BCHP_XPT_DPCR5_PID_CH,                             BCHP_XPT_DPCR5_CTRL },
    { "XPT_DPCR5_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR5_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR5_STC_EXT_CTRL },
    { "XPT_DPCR5_MAX_PCR_ERROR",                 BCHP_XPT_DPCR5_MAX_PCR_ERROR,                      BCHP_XPT_DPCR5_MAX_PCR_ERROR },
    { "XPT_DPCR5_STC_HI",                        BCHP_XPT_DPCR5_STC_HI,                             BCHP_XPT_DPCR5_STC_LO },
    { "XPT_DPCR5_LOOP_CTRL",                     BCHP_XPT_DPCR5_LOOP_CTRL,                          BCHP_XPT_DPCR5_ACCUM_VALUE },
    { "XPT_DPCR5_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR5_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR5_PHASE_ERROR_CLAMP },
    { "XPT_DPCR6_PID_CH",                        BCHP_XPT_DPCR6_PID_CH,                             BCHP_XPT_DPCR6_CTRL },
    { "XPT_DPCR6_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR6_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR6_STC_EXT_CTRL },
    { "XPT_DPCR6_MAX_PCR_ERROR",                 BCHP_XPT_DPCR6_MAX_PCR_ERROR,                      BCHP_XPT_DPCR6_MAX_PCR_ERROR },
    { "XPT_DPCR6_STC_HI",                        BCHP_XPT_DPCR6_STC_HI,                             BCHP_XPT_DPCR6_STC_LO },
    { "XPT_DPCR6_LOOP_CTRL",                     BCHP_XPT_DPCR6_LOOP_CTRL,                          BCHP_XPT_DPCR6_ACCUM_VALUE },
    { "XPT_DPCR6_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR6_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR6_PHASE_ERROR_CLAMP },
    { "XPT_DPCR7_PID_CH",                        BCHP_XPT_DPCR7_PID_CH,                             BCHP_XPT_DPCR7_CTRL },
    { "XPT_DPCR7_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR7_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR7_STC_EXT_CTRL },
    { "XPT_DPCR7_MAX_PCR_ERROR",                 BCHP_XPT_DPCR7_MAX_PCR_ERROR,                      BCHP_XPT_DPCR7_MAX_PCR_ERROR },
    { "XPT_DPCR7_STC_HI",                        BCHP_XPT_DPCR7_STC_HI,                             BCHP_XPT_DPCR7_STC_LO },
    { "XPT_DPCR7_LOOP_CTRL",                     BCHP_XPT_DPCR7_LOOP_CTRL,                          BCHP_XPT_DPCR7_ACCUM_VALUE },
    { "XPT_DPCR7_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR7_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR7_PHASE_ERROR_CLAMP },
    { "XPT_DPCR_PP_PP_CTRL",                     BCHP_XPT_DPCR_PP_PP_CTRL,                          BCHP_XPT_DPCR_PP_PP_FIXED_OFFSET },
    { "XPT_PSUB_PSUB0_CTRL0",                    BCHP_XPT_PSUB_PSUB0_CTRL0,                         BCHP_XPT_PSUB_PSUB0_STAT0 },
    { "XPT_PSUB_PSUB1_CTRL0",                    BCHP_XPT_PSUB_PSUB1_CTRL0,                         BCHP_XPT_PSUB_PSUB1_STAT0 },
    { "XPT_PSUB_PSUB2_CTRL0",                    BCHP_XPT_PSUB_PSUB2_CTRL0,                         BCHP_XPT_PSUB_PSUB2_STAT0 },
    { "XPT_PSUB_PSUB3_CTRL0",                    BCHP_XPT_PSUB_PSUB3_CTRL0,                         BCHP_XPT_PSUB_PSUB3_STAT0 },
    { "XPT_PSUB_PSUB4_CTRL0",                    BCHP_XPT_PSUB_PSUB4_CTRL0,                         BCHP_XPT_PSUB_PSUB4_STAT0 },
    { "XPT_PSUB_PSUB5_CTRL0",                    BCHP_XPT_PSUB_PSUB5_CTRL0,                         BCHP_XPT_PSUB_PSUB5_STAT0 },
    { "XPT_PSUB_PSUB6_CTRL0",                    BCHP_XPT_PSUB_PSUB6_CTRL0,                         BCHP_XPT_PSUB_PSUB6_STAT0 },
    { "XPT_PSUB_PSUB7_CTRL0",                    BCHP_XPT_PSUB_PSUB7_CTRL0,                         BCHP_XPT_PSUB_PSUB7_STAT0 },
    { "XPT_PSUB_PR_FALLBACK_CTRL",               BCHP_XPT_PSUB_PR_FALLBACK_CTRL,                    BCHP_XPT_PSUB_PR_FALLBACK_CTRL },
    { "XPT_MPOD_CFG",                            BCHP_XPT_MPOD_CFG,                                 BCHP_XPT_MPOD_ICTRL },
    { "XPT_MPOD_OCTRL2",                         BCHP_XPT_MPOD_OCTRL2,                              BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP },
    { "XPT_RMX0_CTRL",                           BCHP_XPT_RMX0_CTRL,                                BCHP_XPT_RMX0_FIXED_OFFSET },
    { "XPT_RMX1_CTRL",                           BCHP_XPT_RMX1_CTRL,                                BCHP_XPT_RMX1_FIXED_OFFSET },
    { "XPT_PB_TOP_PACING_COUNT_WR_VALUE",        BCHP_XPT_PB_TOP_PACING_COUNT_WR_VALUE,             BCHP_XPT_PB_TOP_PACING_PAUSE_MAP_VECT_PB15 },
    { "XPT_PB0_CTRL1",                           BCHP_XPT_PB0_CTRL1,                                BCHP_XPT_PB0_FIRST_DESC_ADDR },
    { "XPT_PB0_BLOCKOUT",                        BCHP_XPT_PB0_BLOCKOUT,                             BCHP_XPT_PB0_PARSER_CTRL2 },
    { "XPT_PB0_INTR_EN",                         BCHP_XPT_PB0_INTR_EN,                              BCHP_XPT_PB0_PWR_CTRL },
    { "XPT_PB0_ASF_CTRL",                        BCHP_XPT_PB0_ASF_CTRL,                             BCHP_XPT_PB0_PES_BASED_PACING_CTRL },
    { "XPT_PB1_CTRL1",                           BCHP_XPT_PB1_CTRL1,                                BCHP_XPT_PB1_FIRST_DESC_ADDR },
    { "XPT_PB1_BLOCKOUT",                        BCHP_XPT_PB1_BLOCKOUT,                             BCHP_XPT_PB1_PARSER_CTRL2 },
    { "XPT_PB1_INTR_EN",                         BCHP_XPT_PB1_INTR_EN,                              BCHP_XPT_PB1_PWR_CTRL },
    { "XPT_PB1_ASF_CTRL",                        BCHP_XPT_PB1_ASF_CTRL,                             BCHP_XPT_PB1_PES_BASED_PACING_CTRL },
    { "XPT_PB2_CTRL1",                           BCHP_XPT_PB2_CTRL1,                                BCHP_XPT_PB2_FIRST_DESC_ADDR },
    { "XPT_PB2_BLOCKOUT",                        BCHP_XPT_PB2_BLOCKOUT,                             BCHP_XPT_PB2_PARSER_CTRL2 },
    { "XPT_PB2_INTR_EN",                         BCHP_XPT_PB2_INTR_EN,                              BCHP_XPT_PB2_PWR_CTRL },
    { "XPT_PB2_ASF_CTRL",                        BCHP_XPT_PB2_ASF_CTRL,                             BCHP_XPT_PB2_PES_BASED_PACING_CTRL },
    { "XPT_PB3_CTRL1",                           BCHP_XPT_PB3_CTRL1,                                BCHP_XPT_PB3_FIRST_DESC_ADDR },
    { "XPT_PB3_BLOCKOUT",                        BCHP_XPT_PB3_BLOCKOUT,                             BCHP_XPT_PB3_PARSER_CTRL2 },
    { "XPT_PB3_INTR_EN",                         BCHP_XPT_PB3_INTR_EN,                              BCHP_XPT_PB3_PWR_CTRL },
    { "XPT_PB3_ASF_CTRL",                        BCHP_XPT_PB3_ASF_CTRL,                             BCHP_XPT_PB3_PES_BASED_PACING_CTRL },
    { "XPT_PB4_CTRL1",                           BCHP_XPT_PB4_CTRL1,                                BCHP_XPT_PB4_FIRST_DESC_ADDR },
    { "XPT_PB4_BLOCKOUT",                        BCHP_XPT_PB4_BLOCKOUT,                             BCHP_XPT_PB4_PARSER_CTRL2 },
    { "XPT_PB4_INTR_EN",                         BCHP_XPT_PB4_INTR_EN,                              BCHP_XPT_PB4_PWR_CTRL },
    { "XPT_PB4_ASF_CTRL",                        BCHP_XPT_PB4_ASF_CTRL,                             BCHP_XPT_PB4_PES_BASED_PACING_CTRL },
    { "XPT_PB5_CTRL1",                           BCHP_XPT_PB5_CTRL1,                                BCHP_XPT_PB5_FIRST_DESC_ADDR },
    { "XPT_PB5_BLOCKOUT",                        BCHP_XPT_PB5_BLOCKOUT,                             BCHP_XPT_PB5_PARSER_CTRL2 },
    { "XPT_PB5_INTR_EN",                         BCHP_XPT_PB5_INTR_EN,                              BCHP_XPT_PB5_PWR_CTRL },
    { "XPT_PB5_ASF_CTRL",                        BCHP_XPT_PB5_ASF_CTRL,                             BCHP_XPT_PB5_PES_BASED_PACING_CTRL },
    { "XPT_PB6_CTRL1",                           BCHP_XPT_PB6_CTRL1,                                BCHP_XPT_PB6_FIRST_DESC_ADDR },
    { "XPT_PB6_BLOCKOUT",                        BCHP_XPT_PB6_BLOCKOUT,                             BCHP_XPT_PB6_PARSER_CTRL2 },
    { "XPT_PB6_INTR_EN",                         BCHP_XPT_PB6_INTR_EN,                              BCHP_XPT_PB6_PWR_CTRL },
    { "XPT_PB6_ASF_CTRL",                        BCHP_XPT_PB6_ASF_CTRL,                             BCHP_XPT_PB6_PES_BASED_PACING_CTRL },
    { "XPT_PB7_CTRL1",                           BCHP_XPT_PB7_CTRL1,                                BCHP_XPT_PB7_FIRST_DESC_ADDR },
    { "XPT_PB7_BLOCKOUT",                        BCHP_XPT_PB7_BLOCKOUT,                             BCHP_XPT_PB7_PARSER_CTRL2 },
    { "XPT_PB7_INTR_EN",                         BCHP_XPT_PB7_INTR_EN,                              BCHP_XPT_PB7_PWR_CTRL },
    { "XPT_PB7_ASF_CTRL",                        BCHP_XPT_PB7_ASF_CTRL,                             BCHP_XPT_PB7_PES_BASED_PACING_CTRL },
    { "XPT_PB8_CTRL1",                           BCHP_XPT_PB8_CTRL1,                                BCHP_XPT_PB8_FIRST_DESC_ADDR },
    { "XPT_PB8_BLOCKOUT",                        BCHP_XPT_PB8_BLOCKOUT,                             BCHP_XPT_PB8_PARSER_CTRL2 },
    { "XPT_PB8_INTR_EN",                         BCHP_XPT_PB8_INTR_EN,                              BCHP_XPT_PB8_PWR_CTRL },
    { "XPT_PB8_ASF_CTRL",                        BCHP_XPT_PB8_ASF_CTRL,                             BCHP_XPT_PB8_PES_BASED_PACING_CTRL },
    { "XPT_PB9_CTRL1",                           BCHP_XPT_PB9_CTRL1,                                BCHP_XPT_PB9_FIRST_DESC_ADDR },
    { "XPT_PB9_BLOCKOUT",                        BCHP_XPT_PB9_BLOCKOUT,                             BCHP_XPT_PB9_PARSER_CTRL2 },
    { "XPT_PB9_INTR_EN",                         BCHP_XPT_PB9_INTR_EN,                              BCHP_XPT_PB9_PWR_CTRL },
    { "XPT_PB9_ASF_CTRL",                        BCHP_XPT_PB9_ASF_CTRL,                             BCHP_XPT_PB9_PES_BASED_PACING_CTRL },
    { "XPT_PB10_CTRL1",                          BCHP_XPT_PB10_CTRL1,                               BCHP_XPT_PB10_FIRST_DESC_ADDR },
    { "XPT_PB10_BLOCKOUT",                       BCHP_XPT_PB10_BLOCKOUT,                            BCHP_XPT_PB10_PARSER_CTRL2 },
    { "XPT_PB10_INTR_EN",                        BCHP_XPT_PB10_INTR_EN,                             BCHP_XPT_PB10_PWR_CTRL },
    { "XPT_PB10_ASF_CTRL",                       BCHP_XPT_PB10_ASF_CTRL,                            BCHP_XPT_PB10_PES_BASED_PACING_CTRL },
    { "XPT_PB11_CTRL1",                          BCHP_XPT_PB11_CTRL1,                               BCHP_XPT_PB11_FIRST_DESC_ADDR },
    { "XPT_PB11_BLOCKOUT",                       BCHP_XPT_PB11_BLOCKOUT,                            BCHP_XPT_PB11_PARSER_CTRL2 },
    { "XPT_PB11_INTR_EN",                        BCHP_XPT_PB11_INTR_EN,                             BCHP_XPT_PB11_PWR_CTRL },
    { "XPT_PB11_ASF_CTRL",                       BCHP_XPT_PB11_ASF_CTRL,                            BCHP_XPT_PB11_PES_BASED_PACING_CTRL },
    { "XPT_PB12_CTRL1",                          BCHP_XPT_PB12_CTRL1,                               BCHP_XPT_PB12_FIRST_DESC_ADDR },
    { "XPT_PB12_BLOCKOUT",                       BCHP_XPT_PB12_BLOCKOUT,                            BCHP_XPT_PB12_PARSER_CTRL2 },
    { "XPT_PB12_INTR_EN",                        BCHP_XPT_PB12_INTR_EN,                             BCHP_XPT_PB12_PWR_CTRL },
    { "XPT_PB12_ASF_CTRL",                       BCHP_XPT_PB12_ASF_CTRL,                            BCHP_XPT_PB12_PES_BASED_PACING_CTRL },
    { "XPT_PB13_CTRL1",                          BCHP_XPT_PB13_CTRL1,                               BCHP_XPT_PB13_FIRST_DESC_ADDR },
    { "XPT_PB13_BLOCKOUT",                       BCHP_XPT_PB13_BLOCKOUT,                            BCHP_XPT_PB13_PARSER_CTRL2 },
    { "XPT_PB13_INTR_EN",                        BCHP_XPT_PB13_INTR_EN,                             BCHP_XPT_PB13_PWR_CTRL },
    { "XPT_PB13_ASF_CTRL",                       BCHP_XPT_PB13_ASF_CTRL,                            BCHP_XPT_PB13_PES_BASED_PACING_CTRL },
    { "XPT_PB14_CTRL1",                          BCHP_XPT_PB14_CTRL1,                               BCHP_XPT_PB14_FIRST_DESC_ADDR },
    { "XPT_PB14_BLOCKOUT",                       BCHP_XPT_PB14_BLOCKOUT,                            BCHP_XPT_PB14_PARSER_CTRL2 },
    { "XPT_PB14_INTR_EN",                        BCHP_XPT_PB14_INTR_EN,                             BCHP_XPT_PB14_PWR_CTRL },
    { "XPT_PB14_ASF_CTRL",                       BCHP_XPT_PB14_ASF_CTRL,                            BCHP_XPT_PB14_PES_BASED_PACING_CTRL },
    { "XPT_PB15_CTRL1",                          BCHP_XPT_PB15_CTRL1,                               BCHP_XPT_PB15_FIRST_DESC_ADDR },
    { "XPT_PB15_BLOCKOUT",                       BCHP_XPT_PB15_BLOCKOUT,                            BCHP_XPT_PB15_PARSER_CTRL2 },
    { "XPT_PB15_INTR_EN",                        BCHP_XPT_PB15_INTR_EN,                             BCHP_XPT_PB15_PWR_CTRL },
    { "XPT_PB15_ASF_CTRL",                       BCHP_XPT_PB15_ASF_CTRL,                            BCHP_XPT_PB15_PES_BASED_PACING_CTRL },
    { "XPT_PCROFFSET_INTERRUPT0_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT0_ENABLE,              BCHP_XPT_PCROFFSET_INTERRUPT0_ENABLE },
    { "XPT_PCROFFSET_INTERRUPT1_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT1_ENABLE,              BCHP_XPT_PCROFFSET_INTERRUPT1_ENABLE },
    { "XPT_PCROFFSET_INTERRUPT2_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT2_ENABLE,              BCHP_XPT_PCROFFSET_INTERRUPT2_ENABLE },
    { "XPT_PCROFFSET_INTERRUPT3_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT3_ENABLE,              BCHP_XPT_PCROFFSET_STC_MUX_DELAY },
    { "XPT_PCROFFSET_STC_INTERRUPT_ENABLE",      BCHP_XPT_PCROFFSET_STC_INTERRUPT_ENABLE,           BCHP_XPT_PCROFFSET_STC7_MATCH },
    { "XPT_PCROFFSET_STC0_CTRL",                 BCHP_XPT_PCROFFSET_STC0_CTRL,                      BCHP_XPT_PCROFFSET_STC0_INC_LO },
    { "XPT_PCROFFSET_STC0_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC0_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC0_AV_WINDOW },
    { "XPT_PCROFFSET_STC1_CTRL",                 BCHP_XPT_PCROFFSET_STC1_CTRL,                      BCHP_XPT_PCROFFSET_STC1_INC_LO },
    { "XPT_PCROFFSET_STC1_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC1_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC1_AV_WINDOW },
    { "XPT_PCROFFSET_STC2_CTRL",                 BCHP_XPT_PCROFFSET_STC2_CTRL,                      BCHP_XPT_PCROFFSET_STC2_INC_LO },
    { "XPT_PCROFFSET_STC2_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC2_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC2_AV_WINDOW },
    { "XPT_PCROFFSET_STC3_CTRL",                 BCHP_XPT_PCROFFSET_STC3_CTRL,                      BCHP_XPT_PCROFFSET_STC3_INC_LO },
    { "XPT_PCROFFSET_STC3_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC3_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC3_AV_WINDOW },
    { "XPT_PCROFFSET_STC4_CTRL",                 BCHP_XPT_PCROFFSET_STC4_CTRL,                      BCHP_XPT_PCROFFSET_STC4_INC_LO },
    { "XPT_PCROFFSET_STC4_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC4_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC4_AV_WINDOW },
    { "XPT_PCROFFSET_STC5_CTRL",                 BCHP_XPT_PCROFFSET_STC5_CTRL,                      BCHP_XPT_PCROFFSET_STC5_INC_LO },
    { "XPT_PCROFFSET_STC5_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC5_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC5_AV_WINDOW },
    { "XPT_PCROFFSET_STC6_CTRL",                 BCHP_XPT_PCROFFSET_STC6_CTRL,                      BCHP_XPT_PCROFFSET_STC6_INC_LO },
    { "XPT_PCROFFSET_STC6_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC6_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC6_AV_WINDOW },
    { "XPT_PCROFFSET_STC7_CTRL",                 BCHP_XPT_PCROFFSET_STC7_CTRL,                      BCHP_XPT_PCROFFSET_STC7_INC_LO },
    { "XPT_PCROFFSET_STC7_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC7_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC7_AV_WINDOW },
    { "XPT_FULL_PID_PARSER_PARSER_CFG",          BCHP_XPT_FULL_PID_PARSER_PARSER_CFG,               BCHP_XPT_FULL_PID_PARSER_PBP_ACCEPT_ADAPT_00 },
    { "XPT_FULL_PID_PARSER_IBP_PCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_PCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_PCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_PCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_PCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_PCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_IBP_SCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_SCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_SCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_SCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_SCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_SCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_IBP_PSG_PROTOCOL_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_PSG_PROTOCOL_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_PSG_PROTOCOL_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_PSG_PROTOCOL_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_PSG_PROTOCOL_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_PSG_PROTOCOL_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_IBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PSG_ERR_MODE",        BCHP_XPT_FULL_PID_PARSER_PSG_ERR_MODE,             BCHP_XPT_FULL_PID_PARSER_PSG_ERR_MODE },
    { "XPT_RSBUFF_BO_IBP0",                      BCHP_XPT_RSBUFF_BO_IBP0,                           BCHP_XPT_RSBUFF_BO_IBP15 },
    { "XPT_RSBUFF_BO_PBP0",                      BCHP_XPT_RSBUFF_BO_PBP0,                           BCHP_XPT_RSBUFF_BO_PBP15 },
    { "XPT_RSBUFF_BO_MPOD_IBP0",                 BCHP_XPT_RSBUFF_BO_MPOD_IBP0,                      BCHP_XPT_RSBUFF_BO_MPOD_IBP15 },
    { "XPT_RSBUFF_IBP_BUFFER_ENABLE",            BCHP_XPT_RSBUFF_IBP_BUFFER_ENABLE,                 BCHP_XPT_RSBUFF_PBP_BUFFER_ENABLE },
    { "XPT_RSBUFF_MPOD_IBP_BUFFER_ENABLE",       BCHP_XPT_RSBUFF_MPOD_IBP_BUFFER_ENABLE,            BCHP_XPT_RSBUFF_MPOD_IBP_BUFFER_ENABLE },
    { "XPT_RSBUFF_OVERFLOW_THRESHOLD",           BCHP_XPT_RSBUFF_OVERFLOW_THRESHOLD,                BCHP_XPT_RSBUFF_NO_RD_HANG_CTRL },
    { "XPT_RSBUFF_PR_FALLBACK_CTRL",             BCHP_XPT_RSBUFF_PR_FALLBACK_CTRL,                  BCHP_XPT_RSBUFF_PR_FALLBACK_CTRL },
    { "XPT_XCBUFF_RAVE_CTRL_BUFFER_EN_PBP",      BCHP_XPT_XCBUFF_RAVE_CTRL_BUFFER_EN_PBP,           BCHP_XPT_XCBUFF_PAUSE_THRESHOLD },
    { "XPT_XCBUFF_OVERFLOW_THRESHOLD",           BCHP_XPT_XCBUFF_OVERFLOW_THRESHOLD,                BCHP_XPT_XCBUFF_OVERFLOW_THRESHOLD },
    { "XPT_XCBUFF_BYTE_SWAP_CTRL",               BCHP_XPT_XCBUFF_BYTE_SWAP_CTRL,                    BCHP_XPT_XCBUFF_BO_RAVE_IBP15 },
    { "XPT_XCBUFF_BO_RAVE_PBP0",                 BCHP_XPT_XCBUFF_BO_RAVE_PBP0,                      BCHP_XPT_XCBUFF_BO_RAVE_PBP15 },
    { "XPT_XCBUFF_BO_MSG_IBP0",                  BCHP_XPT_XCBUFF_BO_MSG_IBP0,                       BCHP_XPT_XCBUFF_BO_MSG_IBP15 },
    { "XPT_XCBUFF_BO_MSG_PBP0",                  BCHP_XPT_XCBUFF_BO_MSG_PBP0,                       BCHP_XPT_XCBUFF_BO_MSG_PBP15 },
    { "XPT_XCBUFF_BO_RMX0_IBP0",                 BCHP_XPT_XCBUFF_BO_RMX0_IBP0,                      BCHP_XPT_XCBUFF_BO_RMX0_IBP15 },
    { "XPT_XCBUFF_BO_RMX0_PBP0",                 BCHP_XPT_XCBUFF_BO_RMX0_PBP0,                      BCHP_XPT_XCBUFF_BO_RMX0_PBP15 },
    { "XPT_XCBUFF_BO_RMX1_IBP0",                 BCHP_XPT_XCBUFF_BO_RMX1_IBP0,                      BCHP_XPT_XCBUFF_BO_RMX1_IBP15 },
    { "XPT_XCBUFF_BO_RMX1_PBP0",                 BCHP_XPT_XCBUFF_BO_RMX1_PBP0,                      BCHP_XPT_XCBUFF_BO_RMX1_PBP15 },
    { "XPT_XCBUFF_MISC_CTRL",                    BCHP_XPT_XCBUFF_MISC_CTRL,                         BCHP_XPT_XCBUFF_NO_RD_HANG_CTRL },
    { "XPT_XCBUFF_PR_FALLBACK_CTRL",             BCHP_XPT_XCBUFF_PR_FALLBACK_CTRL,                  BCHP_XPT_XCBUFF_PR_FALLBACK_CTRL },
    { "XPT_MSG_MSG_CTRL1",                       BCHP_XPT_MSG_MSG_CTRL1,                            BCHP_XPT_MSG_DMA_BUFFER_INIT },
    { "XPT_MSG_PHY_ADDR_LO",                     BCHP_XPT_MSG_PHY_ADDR_LO,                          BCHP_XPT_MSG_ID_REJECT },
    { "XPT_MSG_C_MATCH0",                        BCHP_XPT_MSG_C_MATCH0,                             BCHP_XPT_MSG_C_MATCH4 },
    { "XPT_MSG_BUF_DAT_RDY_INTR_EN_00_31",       BCHP_XPT_MSG_BUF_DAT_RDY_INTR_EN_00_31,            BCHP_XPT_MSG_BUF_DAT_RDY_INTR_EN_96_127 },
    { "XPT_MSG_BUF_OVFL_INTR_EN_00_31",          BCHP_XPT_MSG_BUF_OVFL_INTR_EN_00_31,               BCHP_XPT_MSG_BUF_OVFL_INTR_EN_96_127 },
    { "XPT_MSG_DAT_ERR_INTR_EN",                 BCHP_XPT_MSG_DAT_ERR_INTR_EN,                      BCHP_XPT_MSG_DAT_ERR_INTR_EN },
    { "XPT_MSG_MSG_EVENT_CNT_CTRL",              BCHP_XPT_MSG_MSG_EVENT_CNT_CTRL,                   BCHP_XPT_MSG_MSG_EVENT_CNT_CTRL },
    { "XPT_RAVE_AVS_SCV_FILTER_MODE_CONTROL",    BCHP_XPT_RAVE_AVS_SCV_FILTER_MODE_CONTROL,         BCHP_XPT_RAVE_AVS_SCV_FILTER_VALUE_4_TO_7 },
    { "XPT_RAVE_DATA_START_ADDR_A",              BCHP_XPT_RAVE_DATA_START_ADDR_A,                   BCHP_XPT_RAVE_WATCHDOG_TIMER_VALUE },
    { "XPT_RAVE_MISC_CONTROL",                   BCHP_XPT_RAVE_MISC_CONTROL,                        BCHP_XPT_RAVE_BASE_ADDRESSES },
    { "XPT_RAVE_MISC_CONTROL2",                  BCHP_XPT_RAVE_MISC_CONTROL2,                       BCHP_XPT_RAVE_MISC_CONTROL3 },
    { "XPT_RAVE_RC0_SP_CONTROL",                 BCHP_XPT_RAVE_RC0_SP_CONTROL,                      BCHP_XPT_RAVE_RC8_SP_CONTROL },
    { "XPT_RAVE_ATSOFFSET_MAX_ERROR",            BCHP_XPT_RAVE_ATSOFFSET_MAX_ERROR,                 BCHP_XPT_RAVE_ATSOFFSET_MAX_ERROR },
    { "XPT_RAVE_TPIT_TIME_TICK",                 BCHP_XPT_RAVE_TPIT_TIME_TICK,                      BCHP_XPT_RAVE_TPIT_EVE_TIMEOUT },
    { "XPT_RAVE_EMM_TID_MODE",                   BCHP_XPT_RAVE_EMM_TID_MODE,                        BCHP_XPT_RAVE_EMM_CTRL_ID },
    { "XPT_RAVE_EMM_DATA_ID_1",                  BCHP_XPT_RAVE_EMM_DATA_ID_1,                       BCHP_XPT_RAVE_EMM_MASK_ID_8 },
    {0, 0, 0}
};

#elif (BCHP_CHIP==7425 && BCHP_VER>=BCHP_VER_B0)
#include "bchp_xpt_bus_if.h"
#include "bchp_xpt_dpcr0.h"
#include "bchp_xpt_dpcr1.h"
#include "bchp_xpt_dpcr2.h"
#include "bchp_xpt_dpcr3.h"
#include "bchp_xpt_dpcr4.h"
#include "bchp_xpt_dpcr5.h"
#include "bchp_xpt_dpcr6.h"
#include "bchp_xpt_dpcr7.h"
#include "bchp_xpt_dpcr_pp.h"
#include "bchp_xpt_fe.h"
#include "bchp_xpt_full_pid_parser.h"
#include "bchp_xpt_gr.h"
#include "bchp_xpt_mpod.h"
#include "bchp_xpt_msg.h"
#include "bchp_xpt_pb0.h"
#include "bchp_xpt_pb1.h"
#include "bchp_xpt_pb10.h"
#include "bchp_xpt_pb11.h"
#include "bchp_xpt_pb12.h"
#include "bchp_xpt_pb13.h"
#include "bchp_xpt_pb14.h"
#include "bchp_xpt_pb15.h"
#include "bchp_xpt_pb2.h"
#include "bchp_xpt_pb3.h"
#include "bchp_xpt_pb4.h"
#include "bchp_xpt_pb5.h"
#include "bchp_xpt_pb6.h"
#include "bchp_xpt_pb7.h"
#include "bchp_xpt_pb8.h"
#include "bchp_xpt_pb9.h"
#include "bchp_xpt_pb_top.h"
#include "bchp_xpt_pcroffset.h"
#include "bchp_xpt_pmu.h"
#include "bchp_xpt_psub.h"
#include "bchp_xpt_rave.h"
#include "bchp_xpt_rmx0.h"
#include "bchp_xpt_rmx0_io.h"
#include "bchp_xpt_rmx1.h"
#include "bchp_xpt_rmx1_io.h"
#include "bchp_xpt_rsbuff.h"
#include "bchp_xpt_secure_bus_if.h"
#include "bchp_xpt_wakeup.h"
#include "bchp_xpt_xcbuff.h"
#include "bchp_xpt_xmemif.h"
#include "bchp_xpt_xpu.h"

const BXPT_P_RegisterRange XPT_SRAM_LIST[] =
{
    { "WAKEUP_MEM",                              BCHP_XPT_WAKEUP_PKT_MEM_i_ARRAY_BASE,              (BCHP_XPT_WAKEUP_PKT_TYPE3_i_ARRAY_BASE+(BCHP_XPT_WAKEUP_PKT_TYPE3_i_ARRAY_END*4)) },
    { "FE_PID_TABLE",                            BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE,                (BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE+(BCHP_XPT_FE_PID_TABLE_i_ARRAY_END*4)) },
    { "FE_SPID_TABLE",                           BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE,               (BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE+(BCHP_XPT_FE_SPID_TABLE_i_ARRAY_END*4)) },
    { "PCROFFSET_PID_TABLE",                     BCHP_XPT_PCROFFSET_PID_CONFIG_TABLE_i_ARRAY_BASE,  (BCHP_XPT_PCROFFSET_PID_CONFIG_TABLE_i_ARRAY_BASE+(BCHP_XPT_PCROFFSET_PID_CONFIG_TABLE_i_ARRAY_END*4)) },
    { "PCROFFSET_CX_TABLE",                      BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL,        BCHP_XPT_PCROFFSET_CONTEXT15_RESERVED_CFG_0 },
    { "FPP_SCM0",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_END*4)) },
    { "FPP_SCM1",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_1_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_1_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_1_i_ARRAY_END*4)) },
    { "FPP_SCM2",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_2_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_2_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_2_i_ARRAY_END*4)) },
    { "FPP_SCM3",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_3_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_3_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_3_i_ARRAY_END*4)) },
    { "RSBUFF_IBP_PMEM",                         BCHP_XPT_RSBUFF_BASE_POINTER_IBP0,                 BCHP_XPT_RSBUFF_WATERMARK_IBP15 },
    { "RSBUFF_PBP_PMEM",                         BCHP_XPT_RSBUFF_BASE_POINTER_PBP0,                 BCHP_XPT_RSBUFF_WATERMARK_PBP15 },
    { "RSBUFF_MPOD_PMEM",                        BCHP_XPT_RSBUFF_BASE_POINTER_MPOD_IBP0,            BCHP_XPT_RSBUFF_WATERMARK_MPOD_IBP15 },
    { "XCBUFF_RAVE_IBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RAVE_IBP0,            BCHP_XPT_XCBUFF_WATERMARK_RAVE_IBP15 },
    { "XCBUFF_RAVE_PBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RAVE_PBP0,            BCHP_XPT_XCBUFF_WATERMARK_RAVE_PBP15 },
    { "XCBUFF_MSG_IBP_PMEM",                     BCHP_XPT_XCBUFF_BASE_POINTER_MSG_IBP0,             BCHP_XPT_XCBUFF_WATERMARK_MSG_IBP15 },
    { "XCBUFF_MSG_PBP_PMEM",                     BCHP_XPT_XCBUFF_BASE_POINTER_MSG_PBP0,             BCHP_XPT_XCBUFF_WATERMARK_MSG_PBP15 },
    { "XCBUFF_RMX0_IBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX0_IBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX0_IBP15 },
    { "XCBUFF_RMX0_PBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX0_PBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX0_PBP15 },
    { "XCBUFF_RMX1_IBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX1_IBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX1_IBP15 },
    { "XCBUFF_RMX1_PBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX1_PBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX1_PBP15 },
    { "MSG_PMEM",                                BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_ARRAY_BASE,         (BCHP_XPT_MSG_DMA_VP_TABLE_i_ARRAY_BASE+(BCHP_XPT_MSG_DMA_VP_TABLE_i_ARRAY_END*4)) },
    { "MSG_GEN_FILT1",                           BCHP_XPT_MSG_GEN_FILT_EN_i_ARRAY_BASE,             (BCHP_XPT_MSG_GEN_FILT_EN_i_ARRAY_BASE+(BCHP_XPT_MSG_GEN_FILT_EN_i_ARRAY_END*4)) },
    { "MSG_GEN_FILT2",                           BCHP_XPT_MSG_MCAST_16_ADDR_i_ARRAY_BASE,           (BCHP_XPT_MSG_GEN_FILT_EXCL_i_ARRAY_BASE+(BCHP_XPT_MSG_GEN_FILT_EXCL_i_ARRAY_END*4)) },
    { "MSG_PID2BUF",                             BCHP_XPT_MSG_PID2BUF_MAP_i_ARRAY_BASE,             (BCHP_XPT_MSG_PID2BUF_MAP_i_ARRAY_BASE+(BCHP_XPT_MSG_PID2BUF_MAP_i_ARRAY_END*4)) },
    { "RAVE_PMEM",                               BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR,                BCHP_XPT_RAVE_SCD63_RESERVED_STATE3 },
    { "RAVE_CXMEM_A",                            BCHP_XPT_RAVE_CXMEM_Ai_ARRAY_BASE,                 (BCHP_XPT_RAVE_CXMEM_Ai_ARRAY_BASE+(BCHP_XPT_RAVE_CXMEM_Ai_ARRAY_END*4)) },
    { "RAVE_CXMEM_B",                            BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_BASE,                 (BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_BASE+(BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_END*4)) },
    { "RAVE_CXMEM_C",                            BCHP_XPT_RAVE_CXMEM_Ci_ARRAY_BASE,                 (BCHP_XPT_RAVE_CXMEM_Ci_ARRAY_BASE+(BCHP_XPT_RAVE_CXMEM_Ci_ARRAY_END*4)) },
    { "RAVE_DMEM",                               BCHP_XPT_RAVE_DMEMi_ARRAY_BASE,                    (BCHP_XPT_RAVE_DMEMi_ARRAY_BASE+(BCHP_XPT_RAVE_DMEMi_ARRAY_END*4)) },
    { "RAVE_SMEM",                               BCHP_XPT_RAVE_TPIT0_PID_TABLEi_ARRAY_BASE,         BCHP_XPT_RAVE_TPIT_STATE_CONTEXT47 },
    {0, 0, 0}
};

const BXPT_P_RegisterRange XPT_REG_SAVE_LIST[] =
{
    { "XPT_BUS_IF_MISC_CTRL0",                   BCHP_XPT_BUS_IF_MISC_CTRL0,                        BCHP_XPT_BUS_IF_TEST_MODE },
    { "XPT_BUS_IF_INTR_STATUS_REG_EN",           BCHP_XPT_BUS_IF_INTR_STATUS_REG_EN,                BCHP_XPT_BUS_IF_INTR_STATUS_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS2_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS2_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS2_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS3_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS3_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS3_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS4_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS4_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS4_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS5_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS5_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS5_REG_EN },
    { "XPT_XMEMIF_SCB_WR_ARB_SEL_RSBUFF",        BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RSBUFF,             BCHP_XPT_XMEMIF_SCB_RD_ARB_MODE },
    { "XPT_XMEMIF_INTR_STATUS_REG_EN",           BCHP_XPT_XMEMIF_INTR_STATUS_REG_EN,                BCHP_XPT_XMEMIF_INTR_STATUS_REG_EN },
    { "XPT_PMU_CLK_CTRL",                        BCHP_XPT_PMU_CLK_CTRL,                             BCHP_XPT_PMU_RBUS_RSP_VAL },
    { "XPT_GR_CTRL",                             BCHP_XPT_GR_CTRL,                                  BCHP_XPT_GR_CTRL },
    { "XPT_RMX0_IO_FORMAT",                      BCHP_XPT_RMX0_IO_FORMAT,                           BCHP_XPT_RMX0_IO_PKT_DLY_CNT },
    { "XPT_RMX1_IO_FORMAT",                      BCHP_XPT_RMX1_IO_FORMAT,                           BCHP_XPT_RMX1_IO_PKT_DLY_CNT },
    { "XPT_RMX1_IO_TV_STATUS",                   BCHP_XPT_RMX1_IO_TV_STATUS,                        BCHP_XPT_RMX1_IO_TV_STATUS },
    { "XPT_WAKEUP_CTRL",                         BCHP_XPT_WAKEUP_CTRL,                              BCHP_XPT_WAKEUP_CTRL },
    { "XPT_WAKEUP_INTR_STATUS_REG_EN",           BCHP_XPT_WAKEUP_INTR_STATUS_REG_EN,                BCHP_XPT_WAKEUP_INTR_STATUS_REG_EN },
    { "XPT_FE_FE_CTRL",                          BCHP_XPT_FE_FE_CTRL,                               BCHP_XPT_FE_FE_CTRL },
    { "XPT_FE_INTR_STATUS0_REG_EN",              BCHP_XPT_FE_INTR_STATUS0_REG_EN,                   BCHP_XPT_FE_PWR_CTRL },
    { "XPT_FE_MAX_PID_CHANNEL",                  BCHP_XPT_FE_MAX_PID_CHANNEL,                       BCHP_XPT_FE_MAX_PID_CHANNEL },
    { "XPT_FE_IB0_CTRL",                         BCHP_XPT_FE_IB0_CTRL,                              BCHP_XPT_FE_IB0_CTRL },
    { "XPT_FE_IB1_CTRL",                         BCHP_XPT_FE_IB1_CTRL,                              BCHP_XPT_FE_IB1_CTRL },
    { "XPT_FE_IB2_CTRL",                         BCHP_XPT_FE_IB2_CTRL,                              BCHP_XPT_FE_IB2_CTRL },
    { "XPT_FE_IB3_CTRL",                         BCHP_XPT_FE_IB3_CTRL,                              BCHP_XPT_FE_IB3_CTRL },
    { "XPT_FE_IB4_CTRL",                         BCHP_XPT_FE_IB4_CTRL,                              BCHP_XPT_FE_IB4_CTRL },
    { "XPT_FE_IB5_CTRL",                         BCHP_XPT_FE_IB5_CTRL,                              BCHP_XPT_FE_IB5_CTRL },
    { "XPT_FE_IB6_CTRL",                         BCHP_XPT_FE_IB6_CTRL,                              BCHP_XPT_FE_IB6_CTRL },
    { "XPT_FE_IB7_CTRL",                         BCHP_XPT_FE_IB7_CTRL,                              BCHP_XPT_FE_IB7_CTRL },
    { "XPT_FE_IB8_CTRL",                         BCHP_XPT_FE_IB8_CTRL,                              BCHP_XPT_FE_IB8_CTRL },
    { "XPT_FE_IB9_CTRL",                         BCHP_XPT_FE_IB9_CTRL,                              BCHP_XPT_FE_IB9_CTRL },
    { "XPT_FE_IB10_CTRL",                        BCHP_XPT_FE_IB10_CTRL,                             BCHP_XPT_FE_IB10_CTRL },
    { "XPT_FE_MINI_PID_PARSER0_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER0_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER0_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER1_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER1_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER1_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER2_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER2_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER2_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER3_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER3_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER3_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER4_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER4_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER4_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER5_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER5_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER5_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER6_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER6_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER6_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER7_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER7_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER7_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER8_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER8_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER8_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER9_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER9_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER9_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER10_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER10_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER10_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER11_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER11_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER11_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER12_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER12_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER12_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER13_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER13_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER13_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER14_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER14_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER14_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER15_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER15_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER15_CTRL2 },
    { "XPT_FE_IB_SYNC_DETECT_CTRL",              BCHP_XPT_FE_IB_SYNC_DETECT_CTRL,                   BCHP_XPT_FE_IB_SYNC_DETECT_CTRL },
    { "XPT_FE_TSMF0_CTRL",                       BCHP_XPT_FE_TSMF0_CTRL,                            BCHP_XPT_FE_TSMF0_SLOT_MAP_HI },
    { "XPT_FE_TSMF1_CTRL",                       BCHP_XPT_FE_TSMF1_CTRL,                            BCHP_XPT_FE_TSMF1_SLOT_MAP_HI },
    { "XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID", BCHP_XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID,   BCHP_XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID },
    { "XPT_FE_ATS_COUNTER_CTRL",                 BCHP_XPT_FE_ATS_COUNTER_CTRL,                      BCHP_XPT_FE_ATS_TS_BINARY },
    { "XPT_FE_MTSIF_RX0_CTRL1",                  BCHP_XPT_FE_MTSIF_RX0_CTRL1,                       BCHP_XPT_FE_MTSIF_RX0_CTRL1 },
    { "XPT_FE_MTSIF_RX0_INTR_STATUS0_REG_EN",    BCHP_XPT_FE_MTSIF_RX0_INTR_STATUS0_REG_EN,         BCHP_XPT_FE_MTSIF_RX0_BAND_ID_DROP },
    { "XPT_FE_MTSIF_RX1_CTRL1",                  BCHP_XPT_FE_MTSIF_RX1_CTRL1,                       BCHP_XPT_FE_MTSIF_RX1_CTRL1 },
    { "XPT_FE_MTSIF_RX1_INTR_STATUS0_REG_EN",    BCHP_XPT_FE_MTSIF_RX1_INTR_STATUS0_REG_EN,         BCHP_XPT_FE_MTSIF_RX1_BAND_ID_DROP },
    { "XPT_DPCR0_PID_CH",                        BCHP_XPT_DPCR0_PID_CH,                             BCHP_XPT_DPCR0_CTRL },
    { "XPT_DPCR0_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR0_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR0_STC_EXT_CTRL },
    { "XPT_DPCR0_MAX_PCR_ERROR",                 BCHP_XPT_DPCR0_MAX_PCR_ERROR,                      BCHP_XPT_DPCR0_MAX_PCR_ERROR },
    { "XPT_DPCR0_STC_HI",                        BCHP_XPT_DPCR0_STC_HI,                             BCHP_XPT_DPCR0_STC_LO },
    { "XPT_DPCR0_LOOP_CTRL",                     BCHP_XPT_DPCR0_LOOP_CTRL,                          BCHP_XPT_DPCR0_ACCUM_VALUE },
    { "XPT_DPCR0_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR0_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR0_PHASE_ERROR_CLAMP },
    { "XPT_DPCR1_PID_CH",                        BCHP_XPT_DPCR1_PID_CH,                             BCHP_XPT_DPCR1_CTRL },
    { "XPT_DPCR1_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR1_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR1_STC_EXT_CTRL },
    { "XPT_DPCR1_MAX_PCR_ERROR",                 BCHP_XPT_DPCR1_MAX_PCR_ERROR,                      BCHP_XPT_DPCR1_MAX_PCR_ERROR },
    { "XPT_DPCR1_STC_HI",                        BCHP_XPT_DPCR1_STC_HI,                             BCHP_XPT_DPCR1_STC_LO },
    { "XPT_DPCR1_LOOP_CTRL",                     BCHP_XPT_DPCR1_LOOP_CTRL,                          BCHP_XPT_DPCR1_ACCUM_VALUE },
    { "XPT_DPCR1_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR1_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR1_PHASE_ERROR_CLAMP },
    { "XPT_DPCR2_PID_CH",                        BCHP_XPT_DPCR2_PID_CH,                             BCHP_XPT_DPCR2_CTRL },
    { "XPT_DPCR2_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR2_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR2_STC_EXT_CTRL },
    { "XPT_DPCR2_MAX_PCR_ERROR",                 BCHP_XPT_DPCR2_MAX_PCR_ERROR,                      BCHP_XPT_DPCR2_MAX_PCR_ERROR },
    { "XPT_DPCR2_STC_HI",                        BCHP_XPT_DPCR2_STC_HI,                             BCHP_XPT_DPCR2_STC_LO },
    { "XPT_DPCR2_LOOP_CTRL",                     BCHP_XPT_DPCR2_LOOP_CTRL,                          BCHP_XPT_DPCR2_ACCUM_VALUE },
    { "XPT_DPCR2_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR2_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR2_PHASE_ERROR_CLAMP },
    { "XPT_DPCR3_PID_CH",                        BCHP_XPT_DPCR3_PID_CH,                             BCHP_XPT_DPCR3_CTRL },
    { "XPT_DPCR3_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR3_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR3_STC_EXT_CTRL },
    { "XPT_DPCR3_MAX_PCR_ERROR",                 BCHP_XPT_DPCR3_MAX_PCR_ERROR,                      BCHP_XPT_DPCR3_MAX_PCR_ERROR },
    { "XPT_DPCR3_STC_HI",                        BCHP_XPT_DPCR3_STC_HI,                             BCHP_XPT_DPCR3_STC_LO },
    { "XPT_DPCR3_LOOP_CTRL",                     BCHP_XPT_DPCR3_LOOP_CTRL,                          BCHP_XPT_DPCR3_ACCUM_VALUE },
    { "XPT_DPCR3_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR3_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR3_PHASE_ERROR_CLAMP },
    { "XPT_DPCR4_PID_CH",                        BCHP_XPT_DPCR4_PID_CH,                             BCHP_XPT_DPCR4_CTRL },
    { "XPT_DPCR4_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR4_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR4_STC_EXT_CTRL },
    { "XPT_DPCR4_MAX_PCR_ERROR",                 BCHP_XPT_DPCR4_MAX_PCR_ERROR,                      BCHP_XPT_DPCR4_MAX_PCR_ERROR },
    { "XPT_DPCR4_STC_HI",                        BCHP_XPT_DPCR4_STC_HI,                             BCHP_XPT_DPCR4_STC_LO },
    { "XPT_DPCR4_LOOP_CTRL",                     BCHP_XPT_DPCR4_LOOP_CTRL,                          BCHP_XPT_DPCR4_ACCUM_VALUE },
    { "XPT_DPCR4_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR4_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR4_PHASE_ERROR_CLAMP },
    { "XPT_DPCR5_PID_CH",                        BCHP_XPT_DPCR5_PID_CH,                             BCHP_XPT_DPCR5_CTRL },
    { "XPT_DPCR5_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR5_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR5_STC_EXT_CTRL },
    { "XPT_DPCR5_MAX_PCR_ERROR",                 BCHP_XPT_DPCR5_MAX_PCR_ERROR,                      BCHP_XPT_DPCR5_MAX_PCR_ERROR },
    { "XPT_DPCR5_STC_HI",                        BCHP_XPT_DPCR5_STC_HI,                             BCHP_XPT_DPCR5_STC_LO },
    { "XPT_DPCR5_LOOP_CTRL",                     BCHP_XPT_DPCR5_LOOP_CTRL,                          BCHP_XPT_DPCR5_ACCUM_VALUE },
    { "XPT_DPCR5_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR5_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR5_PHASE_ERROR_CLAMP },
    { "XPT_DPCR6_PID_CH",                        BCHP_XPT_DPCR6_PID_CH,                             BCHP_XPT_DPCR6_CTRL },
    { "XPT_DPCR6_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR6_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR6_STC_EXT_CTRL },
    { "XPT_DPCR6_MAX_PCR_ERROR",                 BCHP_XPT_DPCR6_MAX_PCR_ERROR,                      BCHP_XPT_DPCR6_MAX_PCR_ERROR },
    { "XPT_DPCR6_STC_HI",                        BCHP_XPT_DPCR6_STC_HI,                             BCHP_XPT_DPCR6_STC_LO },
    { "XPT_DPCR6_LOOP_CTRL",                     BCHP_XPT_DPCR6_LOOP_CTRL,                          BCHP_XPT_DPCR6_ACCUM_VALUE },
    { "XPT_DPCR6_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR6_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR6_PHASE_ERROR_CLAMP },
    { "XPT_DPCR7_PID_CH",                        BCHP_XPT_DPCR7_PID_CH,                             BCHP_XPT_DPCR7_CTRL },
    { "XPT_DPCR7_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR7_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR7_STC_EXT_CTRL },
    { "XPT_DPCR7_MAX_PCR_ERROR",                 BCHP_XPT_DPCR7_MAX_PCR_ERROR,                      BCHP_XPT_DPCR7_MAX_PCR_ERROR },
    { "XPT_DPCR7_STC_HI",                        BCHP_XPT_DPCR7_STC_HI,                             BCHP_XPT_DPCR7_STC_LO },
    { "XPT_DPCR7_LOOP_CTRL",                     BCHP_XPT_DPCR7_LOOP_CTRL,                          BCHP_XPT_DPCR7_ACCUM_VALUE },
    { "XPT_DPCR7_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR7_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR7_PHASE_ERROR_CLAMP },
    { "XPT_DPCR_PP_PP_CTRL",                     BCHP_XPT_DPCR_PP_PP_CTRL,                          BCHP_XPT_DPCR_PP_PP_FIXED_OFFSET },
    { "XPT_PSUB_PSUB0_CTRL0",                    BCHP_XPT_PSUB_PSUB0_CTRL0,                         BCHP_XPT_PSUB_PSUB0_STAT0 },
    { "XPT_PSUB_PSUB1_CTRL0",                    BCHP_XPT_PSUB_PSUB1_CTRL0,                         BCHP_XPT_PSUB_PSUB1_STAT0 },
    { "XPT_PSUB_PSUB2_CTRL0",                    BCHP_XPT_PSUB_PSUB2_CTRL0,                         BCHP_XPT_PSUB_PSUB2_STAT0 },
    { "XPT_PSUB_PSUB3_CTRL0",                    BCHP_XPT_PSUB_PSUB3_CTRL0,                         BCHP_XPT_PSUB_PSUB3_STAT0 },
    { "XPT_PSUB_PSUB4_CTRL0",                    BCHP_XPT_PSUB_PSUB4_CTRL0,                         BCHP_XPT_PSUB_PSUB4_STAT0 },
    { "XPT_PSUB_PSUB5_CTRL0",                    BCHP_XPT_PSUB_PSUB5_CTRL0,                         BCHP_XPT_PSUB_PSUB5_STAT0 },
    { "XPT_PSUB_PSUB6_CTRL0",                    BCHP_XPT_PSUB_PSUB6_CTRL0,                         BCHP_XPT_PSUB_PSUB6_STAT0 },
    { "XPT_PSUB_PSUB7_CTRL0",                    BCHP_XPT_PSUB_PSUB7_CTRL0,                         BCHP_XPT_PSUB_PSUB7_STAT0 },
    { "XPT_PSUB_PR_FALLBACK_CTRL",               BCHP_XPT_PSUB_PR_FALLBACK_CTRL,                    BCHP_XPT_PSUB_PR_FALLBACK_CTRL },
    { "XPT_MPOD_CFG",                            BCHP_XPT_MPOD_CFG,                                 BCHP_XPT_MPOD_ICTRL },
    { "XPT_MPOD_OCTRL2",                         BCHP_XPT_MPOD_OCTRL2,                              BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP },
    { "XPT_RMX0_CTRL",                           BCHP_XPT_RMX0_CTRL,                                BCHP_XPT_RMX0_FIXED_OFFSET },
    { "XPT_RMX1_CTRL",                           BCHP_XPT_RMX1_CTRL,                                BCHP_XPT_RMX1_FIXED_OFFSET },
    { "XPT_PB_TOP_PACING_COUNT_WR_VALUE",        BCHP_XPT_PB_TOP_PACING_COUNT_WR_VALUE,             BCHP_XPT_PB_TOP_PACING_PAUSE_MAP_VECT_PB15 },
    { "XPT_PB0_CTRL1",                           BCHP_XPT_PB0_CTRL1,                                BCHP_XPT_PB0_FIRST_DESC_ADDR },
    { "XPT_PB0_BLOCKOUT",                        BCHP_XPT_PB0_BLOCKOUT,                             BCHP_XPT_PB0_PARSER_CTRL2 },
    { "XPT_PB0_INTR_EN",                         BCHP_XPT_PB0_INTR_EN,                              BCHP_XPT_PB0_PWR_CTRL },
    { "XPT_PB0_ASF_CTRL",                        BCHP_XPT_PB0_ASF_CTRL,                             BCHP_XPT_PB0_PES_BASED_PACING_CTRL },
    { "XPT_PB1_CTRL1",                           BCHP_XPT_PB1_CTRL1,                                BCHP_XPT_PB1_FIRST_DESC_ADDR },
    { "XPT_PB1_BLOCKOUT",                        BCHP_XPT_PB1_BLOCKOUT,                             BCHP_XPT_PB1_PARSER_CTRL2 },
    { "XPT_PB1_INTR_EN",                         BCHP_XPT_PB1_INTR_EN,                              BCHP_XPT_PB1_PWR_CTRL },
    { "XPT_PB1_ASF_CTRL",                        BCHP_XPT_PB1_ASF_CTRL,                             BCHP_XPT_PB1_PES_BASED_PACING_CTRL },
    { "XPT_PB2_CTRL1",                           BCHP_XPT_PB2_CTRL1,                                BCHP_XPT_PB2_FIRST_DESC_ADDR },
    { "XPT_PB2_BLOCKOUT",                        BCHP_XPT_PB2_BLOCKOUT,                             BCHP_XPT_PB2_PARSER_CTRL2 },
    { "XPT_PB2_INTR_EN",                         BCHP_XPT_PB2_INTR_EN,                              BCHP_XPT_PB2_PWR_CTRL },
    { "XPT_PB2_ASF_CTRL",                        BCHP_XPT_PB2_ASF_CTRL,                             BCHP_XPT_PB2_PES_BASED_PACING_CTRL },
    { "XPT_PB3_CTRL1",                           BCHP_XPT_PB3_CTRL1,                                BCHP_XPT_PB3_FIRST_DESC_ADDR },
    { "XPT_PB3_BLOCKOUT",                        BCHP_XPT_PB3_BLOCKOUT,                             BCHP_XPT_PB3_PARSER_CTRL2 },
    { "XPT_PB3_INTR_EN",                         BCHP_XPT_PB3_INTR_EN,                              BCHP_XPT_PB3_PWR_CTRL },
    { "XPT_PB3_ASF_CTRL",                        BCHP_XPT_PB3_ASF_CTRL,                             BCHP_XPT_PB3_PES_BASED_PACING_CTRL },
    { "XPT_PB4_CTRL1",                           BCHP_XPT_PB4_CTRL1,                                BCHP_XPT_PB4_FIRST_DESC_ADDR },
    { "XPT_PB4_BLOCKOUT",                        BCHP_XPT_PB4_BLOCKOUT,                             BCHP_XPT_PB4_PARSER_CTRL2 },
    { "XPT_PB4_INTR_EN",                         BCHP_XPT_PB4_INTR_EN,                              BCHP_XPT_PB4_PWR_CTRL },
    { "XPT_PB4_ASF_CTRL",                        BCHP_XPT_PB4_ASF_CTRL,                             BCHP_XPT_PB4_PES_BASED_PACING_CTRL },
    { "XPT_PB5_CTRL1",                           BCHP_XPT_PB5_CTRL1,                                BCHP_XPT_PB5_FIRST_DESC_ADDR },
    { "XPT_PB5_BLOCKOUT",                        BCHP_XPT_PB5_BLOCKOUT,                             BCHP_XPT_PB5_PARSER_CTRL2 },
    { "XPT_PB5_INTR_EN",                         BCHP_XPT_PB5_INTR_EN,                              BCHP_XPT_PB5_PWR_CTRL },
    { "XPT_PB5_ASF_CTRL",                        BCHP_XPT_PB5_ASF_CTRL,                             BCHP_XPT_PB5_PES_BASED_PACING_CTRL },
    { "XPT_PB6_CTRL1",                           BCHP_XPT_PB6_CTRL1,                                BCHP_XPT_PB6_FIRST_DESC_ADDR },
    { "XPT_PB6_BLOCKOUT",                        BCHP_XPT_PB6_BLOCKOUT,                             BCHP_XPT_PB6_PARSER_CTRL2 },
    { "XPT_PB6_INTR_EN",                         BCHP_XPT_PB6_INTR_EN,                              BCHP_XPT_PB6_PWR_CTRL },
    { "XPT_PB6_ASF_CTRL",                        BCHP_XPT_PB6_ASF_CTRL,                             BCHP_XPT_PB6_PES_BASED_PACING_CTRL },
    { "XPT_PB7_CTRL1",                           BCHP_XPT_PB7_CTRL1,                                BCHP_XPT_PB7_FIRST_DESC_ADDR },
    { "XPT_PB7_BLOCKOUT",                        BCHP_XPT_PB7_BLOCKOUT,                             BCHP_XPT_PB7_PARSER_CTRL2 },
    { "XPT_PB7_INTR_EN",                         BCHP_XPT_PB7_INTR_EN,                              BCHP_XPT_PB7_PWR_CTRL },
    { "XPT_PB7_ASF_CTRL",                        BCHP_XPT_PB7_ASF_CTRL,                             BCHP_XPT_PB7_PES_BASED_PACING_CTRL },
    { "XPT_PB8_CTRL1",                           BCHP_XPT_PB8_CTRL1,                                BCHP_XPT_PB8_FIRST_DESC_ADDR },
    { "XPT_PB8_BLOCKOUT",                        BCHP_XPT_PB8_BLOCKOUT,                             BCHP_XPT_PB8_PARSER_CTRL2 },
    { "XPT_PB8_INTR_EN",                         BCHP_XPT_PB8_INTR_EN,                              BCHP_XPT_PB8_PWR_CTRL },
    { "XPT_PB8_ASF_CTRL",                        BCHP_XPT_PB8_ASF_CTRL,                             BCHP_XPT_PB8_PES_BASED_PACING_CTRL },
    { "XPT_PB9_CTRL1",                           BCHP_XPT_PB9_CTRL1,                                BCHP_XPT_PB9_FIRST_DESC_ADDR },
    { "XPT_PB9_BLOCKOUT",                        BCHP_XPT_PB9_BLOCKOUT,                             BCHP_XPT_PB9_PARSER_CTRL2 },
    { "XPT_PB9_INTR_EN",                         BCHP_XPT_PB9_INTR_EN,                              BCHP_XPT_PB9_PWR_CTRL },
    { "XPT_PB9_ASF_CTRL",                        BCHP_XPT_PB9_ASF_CTRL,                             BCHP_XPT_PB9_PES_BASED_PACING_CTRL },
    { "XPT_PB10_CTRL1",                          BCHP_XPT_PB10_CTRL1,                               BCHP_XPT_PB10_FIRST_DESC_ADDR },
    { "XPT_PB10_BLOCKOUT",                       BCHP_XPT_PB10_BLOCKOUT,                            BCHP_XPT_PB10_PARSER_CTRL2 },
    { "XPT_PB10_INTR_EN",                        BCHP_XPT_PB10_INTR_EN,                             BCHP_XPT_PB10_PWR_CTRL },
    { "XPT_PB10_ASF_CTRL",                       BCHP_XPT_PB10_ASF_CTRL,                            BCHP_XPT_PB10_PES_BASED_PACING_CTRL },
    { "XPT_PB11_CTRL1",                          BCHP_XPT_PB11_CTRL1,                               BCHP_XPT_PB11_FIRST_DESC_ADDR },
    { "XPT_PB11_BLOCKOUT",                       BCHP_XPT_PB11_BLOCKOUT,                            BCHP_XPT_PB11_PARSER_CTRL2 },
    { "XPT_PB11_INTR_EN",                        BCHP_XPT_PB11_INTR_EN,                             BCHP_XPT_PB11_PWR_CTRL },
    { "XPT_PB11_ASF_CTRL",                       BCHP_XPT_PB11_ASF_CTRL,                            BCHP_XPT_PB11_PES_BASED_PACING_CTRL },
    { "XPT_PB12_CTRL1",                          BCHP_XPT_PB12_CTRL1,                               BCHP_XPT_PB12_FIRST_DESC_ADDR },
    { "XPT_PB12_BLOCKOUT",                       BCHP_XPT_PB12_BLOCKOUT,                            BCHP_XPT_PB12_PARSER_CTRL2 },
    { "XPT_PB12_INTR_EN",                        BCHP_XPT_PB12_INTR_EN,                             BCHP_XPT_PB12_PWR_CTRL },
    { "XPT_PB12_ASF_CTRL",                       BCHP_XPT_PB12_ASF_CTRL,                            BCHP_XPT_PB12_PES_BASED_PACING_CTRL },
    { "XPT_PB13_CTRL1",                          BCHP_XPT_PB13_CTRL1,                               BCHP_XPT_PB13_FIRST_DESC_ADDR },
    { "XPT_PB13_BLOCKOUT",                       BCHP_XPT_PB13_BLOCKOUT,                            BCHP_XPT_PB13_PARSER_CTRL2 },
    { "XPT_PB13_INTR_EN",                        BCHP_XPT_PB13_INTR_EN,                             BCHP_XPT_PB13_PWR_CTRL },
    { "XPT_PB13_ASF_CTRL",                       BCHP_XPT_PB13_ASF_CTRL,                            BCHP_XPT_PB13_PES_BASED_PACING_CTRL },
    { "XPT_PB14_CTRL1",                          BCHP_XPT_PB14_CTRL1,                               BCHP_XPT_PB14_FIRST_DESC_ADDR },
    { "XPT_PB14_BLOCKOUT",                       BCHP_XPT_PB14_BLOCKOUT,                            BCHP_XPT_PB14_PARSER_CTRL2 },
    { "XPT_PB14_INTR_EN",                        BCHP_XPT_PB14_INTR_EN,                             BCHP_XPT_PB14_PWR_CTRL },
    { "XPT_PB14_ASF_CTRL",                       BCHP_XPT_PB14_ASF_CTRL,                            BCHP_XPT_PB14_PES_BASED_PACING_CTRL },
    { "XPT_PB15_CTRL1",                          BCHP_XPT_PB15_CTRL1,                               BCHP_XPT_PB15_FIRST_DESC_ADDR },
    { "XPT_PB15_BLOCKOUT",                       BCHP_XPT_PB15_BLOCKOUT,                            BCHP_XPT_PB15_PARSER_CTRL2 },
    { "XPT_PB15_INTR_EN",                        BCHP_XPT_PB15_INTR_EN,                             BCHP_XPT_PB15_PWR_CTRL },
    { "XPT_PB15_ASF_CTRL",                       BCHP_XPT_PB15_ASF_CTRL,                            BCHP_XPT_PB15_PES_BASED_PACING_CTRL },
    { "XPT_PCROFFSET_INTERRUPT0_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT0_ENABLE,              BCHP_XPT_PCROFFSET_INTERRUPT0_ENABLE },
    { "XPT_PCROFFSET_INTERRUPT1_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT1_ENABLE,              BCHP_XPT_PCROFFSET_INTERRUPT1_ENABLE },
    { "XPT_PCROFFSET_INTERRUPT2_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT2_ENABLE,              BCHP_XPT_PCROFFSET_INTERRUPT2_ENABLE },
    { "XPT_PCROFFSET_INTERRUPT3_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT3_ENABLE,              BCHP_XPT_PCROFFSET_STC_MUX_DELAY },
    { "XPT_PCROFFSET_STC_INTERRUPT_ENABLE",      BCHP_XPT_PCROFFSET_STC_INTERRUPT_ENABLE,           BCHP_XPT_PCROFFSET_STC7_MATCH },
    { "XPT_PCROFFSET_STC0_CTRL",                 BCHP_XPT_PCROFFSET_STC0_CTRL,                      BCHP_XPT_PCROFFSET_STC0_INC_LO },
    { "XPT_PCROFFSET_STC0_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC0_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC0_AV_WINDOW },
    { "XPT_PCROFFSET_STC1_CTRL",                 BCHP_XPT_PCROFFSET_STC1_CTRL,                      BCHP_XPT_PCROFFSET_STC1_INC_LO },
    { "XPT_PCROFFSET_STC1_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC1_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC1_AV_WINDOW },
    { "XPT_PCROFFSET_STC2_CTRL",                 BCHP_XPT_PCROFFSET_STC2_CTRL,                      BCHP_XPT_PCROFFSET_STC2_INC_LO },
    { "XPT_PCROFFSET_STC2_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC2_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC2_AV_WINDOW },
    { "XPT_PCROFFSET_STC3_CTRL",                 BCHP_XPT_PCROFFSET_STC3_CTRL,                      BCHP_XPT_PCROFFSET_STC3_INC_LO },
    { "XPT_PCROFFSET_STC3_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC3_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC3_AV_WINDOW },
    { "XPT_PCROFFSET_STC4_CTRL",                 BCHP_XPT_PCROFFSET_STC4_CTRL,                      BCHP_XPT_PCROFFSET_STC4_INC_LO },
    { "XPT_PCROFFSET_STC4_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC4_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC4_AV_WINDOW },
    { "XPT_PCROFFSET_STC5_CTRL",                 BCHP_XPT_PCROFFSET_STC5_CTRL,                      BCHP_XPT_PCROFFSET_STC5_INC_LO },
    { "XPT_PCROFFSET_STC5_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC5_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC5_AV_WINDOW },
    { "XPT_PCROFFSET_STC6_CTRL",                 BCHP_XPT_PCROFFSET_STC6_CTRL,                      BCHP_XPT_PCROFFSET_STC6_INC_LO },
    { "XPT_PCROFFSET_STC6_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC6_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC6_AV_WINDOW },
    { "XPT_PCROFFSET_STC7_CTRL",                 BCHP_XPT_PCROFFSET_STC7_CTRL,                      BCHP_XPT_PCROFFSET_STC7_INC_LO },
    { "XPT_PCROFFSET_STC7_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC7_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC7_AV_WINDOW },
    { "XPT_FULL_PID_PARSER_PARSER_CFG",          BCHP_XPT_FULL_PID_PARSER_PARSER_CFG,               BCHP_XPT_FULL_PID_PARSER_PBP_ACCEPT_ADAPT_00 },
    { "XPT_FULL_PID_PARSER_IBP_PCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_PCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_PCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_PCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_PCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_PCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_IBP_SCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_SCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_SCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_SCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_SCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_SCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_IBP_PSG_PROTOCOL_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_PSG_PROTOCOL_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_PSG_PROTOCOL_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_PSG_PROTOCOL_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_PSG_PROTOCOL_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_PSG_PROTOCOL_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_IBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PSG_ERR_MODE",        BCHP_XPT_FULL_PID_PARSER_PSG_ERR_MODE,             BCHP_XPT_FULL_PID_PARSER_PSG_ERR_MODE },
    { "XPT_RSBUFF_BO_IBP0",                      BCHP_XPT_RSBUFF_BO_IBP0,                           BCHP_XPT_RSBUFF_BO_IBP15 },
    { "XPT_RSBUFF_BO_PBP0",                      BCHP_XPT_RSBUFF_BO_PBP0,                           BCHP_XPT_RSBUFF_BO_PBP15 },
    { "XPT_RSBUFF_BO_MPOD_IBP0",                 BCHP_XPT_RSBUFF_BO_MPOD_IBP0,                      BCHP_XPT_RSBUFF_BO_MPOD_IBP15 },
    { "XPT_RSBUFF_IBP_BUFFER_ENABLE",            BCHP_XPT_RSBUFF_IBP_BUFFER_ENABLE,                 BCHP_XPT_RSBUFF_PBP_BUFFER_ENABLE },
    { "XPT_RSBUFF_MPOD_IBP_BUFFER_ENABLE",       BCHP_XPT_RSBUFF_MPOD_IBP_BUFFER_ENABLE,            BCHP_XPT_RSBUFF_MPOD_IBP_BUFFER_ENABLE },
    { "XPT_RSBUFF_OVERFLOW_THRESHOLD",           BCHP_XPT_RSBUFF_OVERFLOW_THRESHOLD,                BCHP_XPT_RSBUFF_NO_RD_HANG_CTRL },
    { "XPT_RSBUFF_PR_FALLBACK_CTRL",             BCHP_XPT_RSBUFF_PR_FALLBACK_CTRL,                  BCHP_XPT_RSBUFF_PR_FALLBACK_CTRL },
    { "XPT_XCBUFF_RAVE_CTRL_BUFFER_EN_PBP",      BCHP_XPT_XCBUFF_RAVE_CTRL_BUFFER_EN_PBP,           BCHP_XPT_XCBUFF_PAUSE_THRESHOLD },
    { "XPT_XCBUFF_OVERFLOW_THRESHOLD",           BCHP_XPT_XCBUFF_OVERFLOW_THRESHOLD,                BCHP_XPT_XCBUFF_OVERFLOW_THRESHOLD },
    { "XPT_XCBUFF_BYTE_SWAP_CTRL",               BCHP_XPT_XCBUFF_BYTE_SWAP_CTRL,                    BCHP_XPT_XCBUFF_BO_RAVE_IBP15 },
    { "XPT_XCBUFF_BO_RAVE_PBP0",                 BCHP_XPT_XCBUFF_BO_RAVE_PBP0,                      BCHP_XPT_XCBUFF_BO_RAVE_PBP15 },
    { "XPT_XCBUFF_BO_MSG_IBP0",                  BCHP_XPT_XCBUFF_BO_MSG_IBP0,                       BCHP_XPT_XCBUFF_BO_MSG_IBP15 },
    { "XPT_XCBUFF_BO_MSG_PBP0",                  BCHP_XPT_XCBUFF_BO_MSG_PBP0,                       BCHP_XPT_XCBUFF_BO_MSG_PBP15 },
    { "XPT_XCBUFF_BO_RMX0_IBP0",                 BCHP_XPT_XCBUFF_BO_RMX0_IBP0,                      BCHP_XPT_XCBUFF_BO_RMX0_IBP15 },
    { "XPT_XCBUFF_BO_RMX0_PBP0",                 BCHP_XPT_XCBUFF_BO_RMX0_PBP0,                      BCHP_XPT_XCBUFF_BO_RMX0_PBP15 },
    { "XPT_XCBUFF_BO_RMX1_IBP0",                 BCHP_XPT_XCBUFF_BO_RMX1_IBP0,                      BCHP_XPT_XCBUFF_BO_RMX1_IBP15 },
    { "XPT_XCBUFF_BO_RMX1_PBP0",                 BCHP_XPT_XCBUFF_BO_RMX1_PBP0,                      BCHP_XPT_XCBUFF_BO_RMX1_PBP15 },
    { "XPT_XCBUFF_MISC_CTRL",                    BCHP_XPT_XCBUFF_MISC_CTRL,                         BCHP_XPT_XCBUFF_NO_RD_HANG_CTRL },
    { "XPT_XCBUFF_PR_FALLBACK_CTRL",             BCHP_XPT_XCBUFF_PR_FALLBACK_CTRL,                  BCHP_XPT_XCBUFF_PR_FALLBACK_CTRL },
    { "XPT_MSG_MSG_CTRL1",                       BCHP_XPT_MSG_MSG_CTRL1,                            BCHP_XPT_MSG_DMA_BUFFER_INIT },
    { "XPT_MSG_PHY_ADDR_LO",                     BCHP_XPT_MSG_PHY_ADDR_LO,                          BCHP_XPT_MSG_ID_REJECT },
    { "XPT_MSG_C_MATCH0",                        BCHP_XPT_MSG_C_MATCH0,                             BCHP_XPT_MSG_C_MATCH4 },
    { "XPT_MSG_BUF_DAT_RDY_INTR_EN_00_31",       BCHP_XPT_MSG_BUF_DAT_RDY_INTR_EN_00_31,            BCHP_XPT_MSG_BUF_DAT_RDY_INTR_EN_96_127 },
    { "XPT_MSG_BUF_OVFL_INTR_EN_00_31",          BCHP_XPT_MSG_BUF_OVFL_INTR_EN_00_31,               BCHP_XPT_MSG_BUF_OVFL_INTR_EN_96_127 },
    { "XPT_MSG_DAT_ERR_INTR_EN",                 BCHP_XPT_MSG_DAT_ERR_INTR_EN,                      BCHP_XPT_MSG_DAT_ERR_INTR_EN },
    { "XPT_MSG_MSG_EVENT_CNT_CTRL",              BCHP_XPT_MSG_MSG_EVENT_CNT_CTRL,                   BCHP_XPT_MSG_MSG_EVENT_CNT_CTRL },
    { "XPT_RAVE_AVS_SCV_FILTER_MODE_CONTROL",    BCHP_XPT_RAVE_AVS_SCV_FILTER_MODE_CONTROL,         BCHP_XPT_RAVE_AVS_SCV_FILTER_VALUE_4_TO_7 },
    { "XPT_RAVE_DATA_START_ADDR_A",              BCHP_XPT_RAVE_DATA_START_ADDR_A,                   BCHP_XPT_RAVE_WATCHDOG_TIMER_VALUE },
    { "XPT_RAVE_MISC_CONTROL",                   BCHP_XPT_RAVE_MISC_CONTROL,                        BCHP_XPT_RAVE_BASE_ADDRESSES },
    { "XPT_RAVE_MISC_CONTROL2",                  BCHP_XPT_RAVE_MISC_CONTROL2,                       BCHP_XPT_RAVE_MISC_CONTROL3 },
    { "XPT_RAVE_RC0_SP_CONTROL",                 BCHP_XPT_RAVE_RC0_SP_CONTROL,                      BCHP_XPT_RAVE_RC8_SP_CONTROL },
    { "XPT_RAVE_ATSOFFSET_MAX_ERROR",            BCHP_XPT_RAVE_ATSOFFSET_MAX_ERROR,                 BCHP_XPT_RAVE_ATSOFFSET_MAX_ERROR },
    { "XPT_RAVE_TPIT_TIME_TICK",                 BCHP_XPT_RAVE_TPIT_TIME_TICK,                      BCHP_XPT_RAVE_TPIT_EVE_TIMEOUT },
    { "XPT_RAVE_EMM_TID_MODE",                   BCHP_XPT_RAVE_EMM_TID_MODE,                        BCHP_XPT_RAVE_EMM_CTRL_ID },
    { "XPT_RAVE_EMM_DATA_ID_1",                  BCHP_XPT_RAVE_EMM_DATA_ID_1,                       BCHP_XPT_RAVE_EMM_MASK_ID_8 },
    {0, 0, 0}
};

#elif (BCHP_CHIP==7429) || (BCHP_CHIP==74295)
#include "bchp_xpt_bus_if.h"
#include "bchp_xpt_dpcr0.h"
#include "bchp_xpt_dpcr1.h"
#include "bchp_xpt_dpcr2.h"
#include "bchp_xpt_dpcr3.h"
#include "bchp_xpt_dpcr4.h"
#include "bchp_xpt_dpcr_pp.h"
#include "bchp_xpt_fe.h"
#include "bchp_xpt_full_pid_parser.h"
#include "bchp_xpt_gr.h"
#include "bchp_xpt_mpod.h"
#include "bchp_xpt_msg.h"
#include "bchp_xpt_pb0.h"
#include "bchp_xpt_pb1.h"
#include "bchp_xpt_pb2.h"
#include "bchp_xpt_pb3.h"
#include "bchp_xpt_pb4.h"
#include "bchp_xpt_pb5.h"
#include "bchp_xpt_pb_top.h"
#include "bchp_xpt_pcroffset.h"
#include "bchp_xpt_pmu.h"
#include "bchp_xpt_psub.h"
#include "bchp_xpt_rave.h"
#include "bchp_xpt_rmx0.h"
#include "bchp_xpt_rmx0_io.h"
#include "bchp_xpt_rmx1.h"
#include "bchp_xpt_rmx1_io.h"
#include "bchp_xpt_rsbuff.h"
#include "bchp_xpt_secure_bus_if.h"
#include "bchp_xpt_wakeup.h"
#include "bchp_xpt_xcbuff.h"
#include "bchp_xpt_xmemif.h"
#include "bchp_xpt_xpu.h"

const BXPT_P_RegisterRange XPT_SRAM_LIST[] =
{
    { "WAKEUP_MEM",                              BCHP_XPT_WAKEUP_PKT_MEM_i_ARRAY_BASE,              (BCHP_XPT_WAKEUP_PKT_TYPE3_i_ARRAY_BASE+(BCHP_XPT_WAKEUP_PKT_TYPE3_i_ARRAY_END*4)) },
    { "FE_PID_TABLE",                            BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE,                (BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE+(BCHP_XPT_FE_PID_TABLE_i_ARRAY_END*4)) },
    { "FE_SPID_TABLE",                           BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE,               (BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE+(BCHP_XPT_FE_SPID_TABLE_i_ARRAY_END*4)) },
    { "PCROFFSET_PID_TABLE",                     BCHP_XPT_PCROFFSET_PID_CONFIG_TABLE_i_ARRAY_BASE,  (BCHP_XPT_PCROFFSET_PID_CONFIG_TABLE_i_ARRAY_BASE+(BCHP_XPT_PCROFFSET_PID_CONFIG_TABLE_i_ARRAY_END*4)) },
    { "PCROFFSET_CX_TABLE",                      BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL,        BCHP_XPT_PCROFFSET_CONTEXT15_RESERVED_CFG_0 },
    { "FPP_SCM0",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_END*4)) },
    { "FPP_SCM1",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_1_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_1_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_1_i_ARRAY_END*4)) },
    { "FPP_SCM2",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_2_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_2_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_2_i_ARRAY_END*4)) },
    { "FPP_SCM3",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_3_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_3_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_3_i_ARRAY_END*4)) },
    { "RSBUFF_IBP_PMEM",                         BCHP_XPT_RSBUFF_BASE_POINTER_IBP0,                 BCHP_XPT_RSBUFF_WATERMARK_IBP15 },
    { "RSBUFF_PBP_PMEM",                         BCHP_XPT_RSBUFF_BASE_POINTER_PBP0,                 BCHP_XPT_RSBUFF_WATERMARK_PBP5 },
    { "RSBUFF_MPOD_PMEM",                        BCHP_XPT_RSBUFF_BASE_POINTER_MPOD_IBP0,            BCHP_XPT_RSBUFF_WATERMARK_MPOD_IBP15 },
    { "XCBUFF_RAVE_IBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RAVE_IBP0,            BCHP_XPT_XCBUFF_WATERMARK_RAVE_IBP15 },
    { "XCBUFF_RAVE_PBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RAVE_PBP0,            BCHP_XPT_XCBUFF_WATERMARK_RAVE_PBP5 },
    { "XCBUFF_MSG_IBP_PMEM",                     BCHP_XPT_XCBUFF_BASE_POINTER_MSG_IBP0,             BCHP_XPT_XCBUFF_WATERMARK_MSG_IBP15 },
    { "XCBUFF_MSG_PBP_PMEM",                     BCHP_XPT_XCBUFF_BASE_POINTER_MSG_PBP0,             BCHP_XPT_XCBUFF_WATERMARK_MSG_PBP5 },
    { "XCBUFF_RMX0_IBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX0_IBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX0_IBP15 },
    { "XCBUFF_RMX0_PBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX0_PBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX0_PBP5 },
    { "XCBUFF_RMX1_IBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX1_IBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX1_IBP15 },
    { "XCBUFF_RMX1_PBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX1_PBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX1_PBP5 },
    { "MSG_PMEM",                                BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_ARRAY_BASE,         (BCHP_XPT_MSG_DMA_VP_TABLE_i_ARRAY_BASE+(BCHP_XPT_MSG_DMA_VP_TABLE_i_ARRAY_END*4)) },
    { "MSG_GEN_FILT1",                           BCHP_XPT_MSG_GEN_FILT_EN_i_ARRAY_BASE,             (BCHP_XPT_MSG_GEN_FILT_EN_i_ARRAY_BASE+(BCHP_XPT_MSG_GEN_FILT_EN_i_ARRAY_END*4)) },
    { "MSG_GEN_FILT2",                           BCHP_XPT_MSG_MCAST_16_ADDR_i_ARRAY_BASE,           (BCHP_XPT_MSG_GEN_FILT_EXCL_i_ARRAY_BASE+(BCHP_XPT_MSG_GEN_FILT_EXCL_i_ARRAY_END*4)) },
    { "MSG_PID2BUF",                             BCHP_XPT_MSG_PID2BUF_MAP_i_ARRAY_BASE,             (BCHP_XPT_MSG_PID2BUF_MAP_i_ARRAY_BASE+(BCHP_XPT_MSG_PID2BUF_MAP_i_ARRAY_END*4)) },
    { "RAVE_PMEM",                               BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR,                BCHP_XPT_RAVE_SCD47_RESERVED_STATE3 },
    { "RAVE_CXMEM_A",                            BCHP_XPT_RAVE_CXMEM_Ai_ARRAY_BASE,                 (BCHP_XPT_RAVE_CXMEM_Ai_ARRAY_BASE+(BCHP_XPT_RAVE_CXMEM_Ai_ARRAY_END*4)) },
    { "RAVE_CXMEM_B",                            BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_BASE,                 (BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_BASE+(BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_END*4)) },
    { "RAVE_DMEM",                               BCHP_XPT_RAVE_DMEMi_ARRAY_BASE,                    (BCHP_XPT_RAVE_DMEMi_ARRAY_BASE+(BCHP_XPT_RAVE_DMEMi_ARRAY_END*4)) },
    { "RAVE_SMEM",                               BCHP_XPT_RAVE_TPIT0_PID_TABLEi_ARRAY_BASE,         BCHP_XPT_RAVE_TPIT_STATE_CONTEXT23 },
    {0, 0, 0}
};

const BXPT_P_RegisterRange XPT_REG_SAVE_LIST[] =
{
    { "XPT_BUS_IF_MISC_CTRL0",                   BCHP_XPT_BUS_IF_MISC_CTRL0,                        BCHP_XPT_BUS_IF_TEST_MODE },
    { "XPT_BUS_IF_INTR_STATUS_REG_EN",           BCHP_XPT_BUS_IF_INTR_STATUS_REG_EN,                BCHP_XPT_BUS_IF_INTR_STATUS_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS2_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS2_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS2_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS3_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS3_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS3_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS4_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS4_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS4_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS5_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS5_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS5_REG_EN },
    { "XPT_XMEMIF_SCB_WR_ARB_SEL_RSBUFF",        BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RSBUFF,             BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB5 },
    { "XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB",          BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB,               BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB },
    { "XPT_XMEMIF_SCB_WR_ARB_MODE",              BCHP_XPT_XMEMIF_SCB_WR_ARB_MODE,                   BCHP_XPT_XMEMIF_SCB_RD_ARB_MODE },
    { "XPT_XMEMIF_INTR_STATUS_REG_EN",           BCHP_XPT_XMEMIF_INTR_STATUS_REG_EN,                BCHP_XPT_XMEMIF_INTR_STATUS_REG_EN },
    { "XPT_PMU_CLK_CTRL",                        BCHP_XPT_PMU_CLK_CTRL,                             BCHP_XPT_PMU_RBUS_RSP_VAL },
    { "XPT_GR_CTRL",                             BCHP_XPT_GR_CTRL,                                  BCHP_XPT_GR_CTRL },
    { "XPT_RMX0_IO_FORMAT",                      BCHP_XPT_RMX0_IO_FORMAT,                           BCHP_XPT_RMX0_IO_PKT_DLY_CNT },
    { "XPT_RMX1_IO_FORMAT",                      BCHP_XPT_RMX1_IO_FORMAT,                           BCHP_XPT_RMX1_IO_PKT_DLY_CNT },
    { "XPT_RMX1_IO_TV_STATUS",                   BCHP_XPT_RMX1_IO_TV_STATUS,                        BCHP_XPT_RMX1_IO_TV_STATUS },
    { "XPT_WAKEUP_CTRL",                         BCHP_XPT_WAKEUP_CTRL,                              BCHP_XPT_WAKEUP_CTRL },
    { "XPT_WAKEUP_INTR_STATUS_REG_EN",           BCHP_XPT_WAKEUP_INTR_STATUS_REG_EN,                BCHP_XPT_WAKEUP_INTR_STATUS_REG_EN },
    { "XPT_FE_FE_CTRL",                          BCHP_XPT_FE_FE_CTRL,                               BCHP_XPT_FE_FE_CTRL },
    { "XPT_FE_INTR_STATUS0_REG_EN",              BCHP_XPT_FE_INTR_STATUS0_REG_EN,                   BCHP_XPT_FE_PWR_CTRL },
    { "XPT_FE_MAX_PID_CHANNEL",                  BCHP_XPT_FE_MAX_PID_CHANNEL,                       BCHP_XPT_FE_MAX_PID_CHANNEL },
    { "XPT_FE_IB0_CTRL",                         BCHP_XPT_FE_IB0_CTRL,                              BCHP_XPT_FE_IB0_CTRL },
    { "XPT_FE_IB1_CTRL",                         BCHP_XPT_FE_IB1_CTRL,                              BCHP_XPT_FE_IB1_CTRL },
    { "XPT_FE_IB2_CTRL",                         BCHP_XPT_FE_IB2_CTRL,                              BCHP_XPT_FE_IB2_CTRL },
    { "XPT_FE_IB3_CTRL",                         BCHP_XPT_FE_IB3_CTRL,                              BCHP_XPT_FE_IB3_CTRL },
    { "XPT_FE_IB4_CTRL",                         BCHP_XPT_FE_IB4_CTRL,                              BCHP_XPT_FE_IB4_CTRL },
    { "XPT_FE_IB5_CTRL",                         BCHP_XPT_FE_IB5_CTRL,                              BCHP_XPT_FE_IB5_CTRL },
    { "XPT_FE_IB6_CTRL",                         BCHP_XPT_FE_IB6_CTRL,                              BCHP_XPT_FE_IB6_CTRL },
    { "XPT_FE_IB7_CTRL",                         BCHP_XPT_FE_IB7_CTRL,                              BCHP_XPT_FE_IB7_CTRL },
    { "XPT_FE_IB8_CTRL",                         BCHP_XPT_FE_IB8_CTRL,                              BCHP_XPT_FE_IB8_CTRL },
    { "XPT_FE_IB9_CTRL",                         BCHP_XPT_FE_IB9_CTRL,                              BCHP_XPT_FE_IB9_CTRL },
    { "XPT_FE_IB10_CTRL",                        BCHP_XPT_FE_IB10_CTRL,                             BCHP_XPT_FE_IB10_CTRL },
    { "XPT_FE_MINI_PID_PARSER0_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER0_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER0_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER1_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER1_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER1_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER2_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER2_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER2_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER3_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER3_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER3_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER4_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER4_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER4_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER5_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER5_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER5_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER6_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER6_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER6_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER7_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER7_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER7_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER8_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER8_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER8_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER9_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER9_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER9_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER10_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER10_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER10_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER11_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER11_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER11_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER12_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER12_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER12_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER13_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER13_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER13_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER14_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER14_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER14_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER15_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER15_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER15_CTRL2 },
    { "XPT_FE_IB_SYNC_DETECT_CTRL",              BCHP_XPT_FE_IB_SYNC_DETECT_CTRL,                   BCHP_XPT_FE_IB_SYNC_DETECT_CTRL },
    { "XPT_FE_TSMF0_CTRL",                       BCHP_XPT_FE_TSMF0_CTRL,                            BCHP_XPT_FE_TSMF0_SLOT_MAP_HI },
    { "XPT_FE_TSMF1_CTRL",                       BCHP_XPT_FE_TSMF1_CTRL,                            BCHP_XPT_FE_TSMF1_SLOT_MAP_HI },
    { "XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID", BCHP_XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID,   BCHP_XPT_FE_MINI_PID_PARSER12_TO_PARSER15_BAND_ID },
    { "XPT_FE_ATS_COUNTER_CTRL",                 BCHP_XPT_FE_ATS_COUNTER_CTRL,                      BCHP_XPT_FE_ATS_TS_BINARY },
    { "XPT_FE_MTSIF_RX0_CTRL1",                  BCHP_XPT_FE_MTSIF_RX0_CTRL1,                       BCHP_XPT_FE_MTSIF_RX0_CTRL1 },
    { "XPT_FE_MTSIF_RX0_INTR_STATUS0_REG_EN",    BCHP_XPT_FE_MTSIF_RX0_INTR_STATUS0_REG_EN,         BCHP_XPT_FE_MTSIF_RX0_BAND_ID_DROP },
    { "XPT_FE_MTSIF_RX1_CTRL1",                  BCHP_XPT_FE_MTSIF_RX1_CTRL1,                       BCHP_XPT_FE_MTSIF_RX1_CTRL1 },
    { "XPT_FE_MTSIF_RX1_INTR_STATUS0_REG_EN",    BCHP_XPT_FE_MTSIF_RX1_INTR_STATUS0_REG_EN,         BCHP_XPT_FE_MTSIF_RX1_BAND_ID_DROP },
    { "XPT_DPCR0_PID_CH",                        BCHP_XPT_DPCR0_PID_CH,                             BCHP_XPT_DPCR0_CTRL },
    { "XPT_DPCR0_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR0_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR0_STC_EXT_CTRL },
    { "XPT_DPCR0_MAX_PCR_ERROR",                 BCHP_XPT_DPCR0_MAX_PCR_ERROR,                      BCHP_XPT_DPCR0_MAX_PCR_ERROR },
    { "XPT_DPCR0_STC_HI",                        BCHP_XPT_DPCR0_STC_HI,                             BCHP_XPT_DPCR0_STC_LO },
    { "XPT_DPCR0_LOOP_CTRL",                     BCHP_XPT_DPCR0_LOOP_CTRL,                          BCHP_XPT_DPCR0_ACCUM_VALUE },
    { "XPT_DPCR0_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR0_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR0_PHASE_ERROR_CLAMP },
    { "XPT_DPCR1_PID_CH",                        BCHP_XPT_DPCR1_PID_CH,                             BCHP_XPT_DPCR1_CTRL },
    { "XPT_DPCR1_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR1_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR1_STC_EXT_CTRL },
    { "XPT_DPCR1_MAX_PCR_ERROR",                 BCHP_XPT_DPCR1_MAX_PCR_ERROR,                      BCHP_XPT_DPCR1_MAX_PCR_ERROR },
    { "XPT_DPCR1_STC_HI",                        BCHP_XPT_DPCR1_STC_HI,                             BCHP_XPT_DPCR1_STC_LO },
    { "XPT_DPCR1_LOOP_CTRL",                     BCHP_XPT_DPCR1_LOOP_CTRL,                          BCHP_XPT_DPCR1_ACCUM_VALUE },
    { "XPT_DPCR1_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR1_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR1_PHASE_ERROR_CLAMP },
    { "XPT_DPCR2_PID_CH",                        BCHP_XPT_DPCR2_PID_CH,                             BCHP_XPT_DPCR2_CTRL },
    { "XPT_DPCR2_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR2_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR2_STC_EXT_CTRL },
    { "XPT_DPCR2_MAX_PCR_ERROR",                 BCHP_XPT_DPCR2_MAX_PCR_ERROR,                      BCHP_XPT_DPCR2_MAX_PCR_ERROR },
    { "XPT_DPCR2_STC_HI",                        BCHP_XPT_DPCR2_STC_HI,                             BCHP_XPT_DPCR2_STC_LO },
    { "XPT_DPCR2_LOOP_CTRL",                     BCHP_XPT_DPCR2_LOOP_CTRL,                          BCHP_XPT_DPCR2_ACCUM_VALUE },
    { "XPT_DPCR2_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR2_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR2_PHASE_ERROR_CLAMP },
    { "XPT_DPCR3_PID_CH",                        BCHP_XPT_DPCR3_PID_CH,                             BCHP_XPT_DPCR3_CTRL },
    { "XPT_DPCR3_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR3_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR3_STC_EXT_CTRL },
    { "XPT_DPCR3_MAX_PCR_ERROR",                 BCHP_XPT_DPCR3_MAX_PCR_ERROR,                      BCHP_XPT_DPCR3_MAX_PCR_ERROR },
    { "XPT_DPCR3_STC_HI",                        BCHP_XPT_DPCR3_STC_HI,                             BCHP_XPT_DPCR3_STC_LO },
    { "XPT_DPCR3_LOOP_CTRL",                     BCHP_XPT_DPCR3_LOOP_CTRL,                          BCHP_XPT_DPCR3_ACCUM_VALUE },
    { "XPT_DPCR3_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR3_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR3_PHASE_ERROR_CLAMP },
    { "XPT_DPCR4_PID_CH",                        BCHP_XPT_DPCR4_PID_CH,                             BCHP_XPT_DPCR4_CTRL },
    { "XPT_DPCR4_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR4_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR4_STC_EXT_CTRL },
    { "XPT_DPCR4_MAX_PCR_ERROR",                 BCHP_XPT_DPCR4_MAX_PCR_ERROR,                      BCHP_XPT_DPCR4_MAX_PCR_ERROR },
    { "XPT_DPCR4_STC_HI",                        BCHP_XPT_DPCR4_STC_HI,                             BCHP_XPT_DPCR4_STC_LO },
    { "XPT_DPCR4_LOOP_CTRL",                     BCHP_XPT_DPCR4_LOOP_CTRL,                          BCHP_XPT_DPCR4_ACCUM_VALUE },
    { "XPT_DPCR4_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR4_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR4_PHASE_ERROR_CLAMP },
    { "XPT_DPCR_PP_PP_CTRL",                     BCHP_XPT_DPCR_PP_PP_CTRL,                          BCHP_XPT_DPCR_PP_PP_FIXED_OFFSET },
    { "XPT_PSUB_PSUB0_CTRL0",                    BCHP_XPT_PSUB_PSUB0_CTRL0,                         BCHP_XPT_PSUB_PSUB0_STAT0 },
    { "XPT_PSUB_PSUB1_CTRL0",                    BCHP_XPT_PSUB_PSUB1_CTRL0,                         BCHP_XPT_PSUB_PSUB1_STAT0 },
    { "XPT_PSUB_PSUB2_CTRL0",                    BCHP_XPT_PSUB_PSUB2_CTRL0,                         BCHP_XPT_PSUB_PSUB2_STAT0 },
    { "XPT_PSUB_PSUB3_CTRL0",                    BCHP_XPT_PSUB_PSUB3_CTRL0,                         BCHP_XPT_PSUB_PSUB3_STAT0 },
    { "XPT_PSUB_PSUB4_CTRL0",                    BCHP_XPT_PSUB_PSUB4_CTRL0,                         BCHP_XPT_PSUB_PSUB4_STAT0 },
    { "XPT_PSUB_PSUB5_CTRL0",                    BCHP_XPT_PSUB_PSUB5_CTRL0,                         BCHP_XPT_PSUB_PSUB5_STAT0 },
    { "XPT_PSUB_PSUB6_CTRL0",                    BCHP_XPT_PSUB_PSUB6_CTRL0,                         BCHP_XPT_PSUB_PSUB6_STAT0 },
    { "XPT_PSUB_PSUB7_CTRL0",                    BCHP_XPT_PSUB_PSUB7_CTRL0,                         BCHP_XPT_PSUB_PSUB7_STAT0 },
    { "XPT_PSUB_PR_FALLBACK_CTRL",               BCHP_XPT_PSUB_PR_FALLBACK_CTRL,                    BCHP_XPT_PSUB_PR_FALLBACK_CTRL },
    { "XPT_MPOD_CFG",                            BCHP_XPT_MPOD_CFG,                                 BCHP_XPT_MPOD_ICTRL },
    { "XPT_MPOD_OCTRL2",                         BCHP_XPT_MPOD_OCTRL2,                              BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP },
    { "XPT_RMX0_CTRL",                           BCHP_XPT_RMX0_CTRL,                                BCHP_XPT_RMX0_FIXED_OFFSET },
    { "XPT_RMX1_CTRL",                           BCHP_XPT_RMX1_CTRL,                                BCHP_XPT_RMX1_FIXED_OFFSET },
    { "XPT_PB_TOP_PACING_COUNT_WR_VALUE",        BCHP_XPT_PB_TOP_PACING_COUNT_WR_VALUE,             BCHP_XPT_PB_TOP_BAND_PAUSE_MAP_VECT_PB5 },
    { "XPT_PB_TOP_PACING_PAUSE_MAP_VECT_PB0",    BCHP_XPT_PB_TOP_PACING_PAUSE_MAP_VECT_PB0,         BCHP_XPT_PB_TOP_PACING_PAUSE_MAP_VECT_PB5 },
    { "XPT_PB0_CTRL1",                           BCHP_XPT_PB0_CTRL1,                                BCHP_XPT_PB0_FIRST_DESC_ADDR },
    { "XPT_PB0_BLOCKOUT",                        BCHP_XPT_PB0_BLOCKOUT,                             BCHP_XPT_PB0_PARSER_CTRL2 },
    { "XPT_PB0_INTR_EN",                         BCHP_XPT_PB0_INTR_EN,                              BCHP_XPT_PB0_PWR_CTRL },
    { "XPT_PB0_ASF_CTRL",                        BCHP_XPT_PB0_ASF_CTRL,                             BCHP_XPT_PB0_PES_BASED_PACING_CTRL },
    { "XPT_PB1_CTRL1",                           BCHP_XPT_PB1_CTRL1,                                BCHP_XPT_PB1_FIRST_DESC_ADDR },
    { "XPT_PB1_BLOCKOUT",                        BCHP_XPT_PB1_BLOCKOUT,                             BCHP_XPT_PB1_PARSER_CTRL2 },
    { "XPT_PB1_INTR_EN",                         BCHP_XPT_PB1_INTR_EN,                              BCHP_XPT_PB1_PWR_CTRL },
    { "XPT_PB1_ASF_CTRL",                        BCHP_XPT_PB1_ASF_CTRL,                             BCHP_XPT_PB1_PES_BASED_PACING_CTRL },
    { "XPT_PB2_CTRL1",                           BCHP_XPT_PB2_CTRL1,                                BCHP_XPT_PB2_FIRST_DESC_ADDR },
    { "XPT_PB2_BLOCKOUT",                        BCHP_XPT_PB2_BLOCKOUT,                             BCHP_XPT_PB2_PARSER_CTRL2 },
    { "XPT_PB2_INTR_EN",                         BCHP_XPT_PB2_INTR_EN,                              BCHP_XPT_PB2_PWR_CTRL },
    { "XPT_PB2_ASF_CTRL",                        BCHP_XPT_PB2_ASF_CTRL,                             BCHP_XPT_PB2_PES_BASED_PACING_CTRL },
    { "XPT_PB3_CTRL1",                           BCHP_XPT_PB3_CTRL1,                                BCHP_XPT_PB3_FIRST_DESC_ADDR },
    { "XPT_PB3_BLOCKOUT",                        BCHP_XPT_PB3_BLOCKOUT,                             BCHP_XPT_PB3_PARSER_CTRL2 },
    { "XPT_PB3_INTR_EN",                         BCHP_XPT_PB3_INTR_EN,                              BCHP_XPT_PB3_PWR_CTRL },
    { "XPT_PB3_ASF_CTRL",                        BCHP_XPT_PB3_ASF_CTRL,                             BCHP_XPT_PB3_PES_BASED_PACING_CTRL },
    { "XPT_PB4_CTRL1",                           BCHP_XPT_PB4_CTRL1,                                BCHP_XPT_PB4_FIRST_DESC_ADDR },
    { "XPT_PB4_BLOCKOUT",                        BCHP_XPT_PB4_BLOCKOUT,                             BCHP_XPT_PB4_PARSER_CTRL2 },
    { "XPT_PB4_INTR_EN",                         BCHP_XPT_PB4_INTR_EN,                              BCHP_XPT_PB4_PWR_CTRL },
    { "XPT_PB4_ASF_CTRL",                        BCHP_XPT_PB4_ASF_CTRL,                             BCHP_XPT_PB4_PES_BASED_PACING_CTRL },
    { "XPT_PB5_CTRL1",                           BCHP_XPT_PB5_CTRL1,                                BCHP_XPT_PB5_FIRST_DESC_ADDR },
    { "XPT_PB5_BLOCKOUT",                        BCHP_XPT_PB5_BLOCKOUT,                             BCHP_XPT_PB5_PARSER_CTRL2 },
    { "XPT_PB5_INTR_EN",                         BCHP_XPT_PB5_INTR_EN,                              BCHP_XPT_PB5_PWR_CTRL },
    { "XPT_PB5_ASF_CTRL",                        BCHP_XPT_PB5_ASF_CTRL,                             BCHP_XPT_PB5_PES_BASED_PACING_CTRL },
    { "XPT_PCROFFSET_INTERRUPT0_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT0_ENABLE,              BCHP_XPT_PCROFFSET_INTERRUPT0_ENABLE },
    { "XPT_PCROFFSET_INTERRUPT1_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT1_ENABLE,              BCHP_XPT_PCROFFSET_INTERRUPT1_ENABLE },
    { "XPT_PCROFFSET_INTERRUPT2_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT2_ENABLE,              BCHP_XPT_PCROFFSET_INTERRUPT2_ENABLE },
    { "XPT_PCROFFSET_INTERRUPT3_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT3_ENABLE,              BCHP_XPT_PCROFFSET_STC_MUX_DELAY },
    { "XPT_PCROFFSET_STC_INTERRUPT_ENABLE",      BCHP_XPT_PCROFFSET_STC_INTERRUPT_ENABLE,           BCHP_XPT_PCROFFSET_STC7_MATCH },
    { "XPT_PCROFFSET_STC0_CTRL",                 BCHP_XPT_PCROFFSET_STC0_CTRL,                      BCHP_XPT_PCROFFSET_STC0_INC_LO },
    { "XPT_PCROFFSET_STC0_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC0_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC0_AV_WINDOW },
    { "XPT_PCROFFSET_STC1_CTRL",                 BCHP_XPT_PCROFFSET_STC1_CTRL,                      BCHP_XPT_PCROFFSET_STC1_INC_LO },
    { "XPT_PCROFFSET_STC1_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC1_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC1_AV_WINDOW },
    { "XPT_PCROFFSET_STC2_CTRL",                 BCHP_XPT_PCROFFSET_STC2_CTRL,                      BCHP_XPT_PCROFFSET_STC2_INC_LO },
    { "XPT_PCROFFSET_STC2_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC2_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC2_AV_WINDOW },
    { "XPT_PCROFFSET_STC3_CTRL",                 BCHP_XPT_PCROFFSET_STC3_CTRL,                      BCHP_XPT_PCROFFSET_STC3_INC_LO },
    { "XPT_PCROFFSET_STC3_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC3_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC3_AV_WINDOW },
    { "XPT_PCROFFSET_STC4_CTRL",                 BCHP_XPT_PCROFFSET_STC4_CTRL,                      BCHP_XPT_PCROFFSET_STC4_INC_LO },
    { "XPT_PCROFFSET_STC4_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC4_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC4_AV_WINDOW },
    { "XPT_PCROFFSET_STC5_CTRL",                 BCHP_XPT_PCROFFSET_STC5_CTRL,                      BCHP_XPT_PCROFFSET_STC5_INC_LO },
    { "XPT_PCROFFSET_STC5_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC5_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC5_AV_WINDOW },
    { "XPT_PCROFFSET_STC6_CTRL",                 BCHP_XPT_PCROFFSET_STC6_CTRL,                      BCHP_XPT_PCROFFSET_STC6_INC_LO },
    { "XPT_PCROFFSET_STC6_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC6_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC6_AV_WINDOW },
    { "XPT_PCROFFSET_STC7_CTRL",                 BCHP_XPT_PCROFFSET_STC7_CTRL,                      BCHP_XPT_PCROFFSET_STC7_INC_LO },
    { "XPT_PCROFFSET_STC7_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC7_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC7_AV_WINDOW },
    { "XPT_FULL_PID_PARSER_PARSER_CFG",          BCHP_XPT_FULL_PID_PARSER_PARSER_CFG,               BCHP_XPT_FULL_PID_PARSER_PBP_ACCEPT_ADAPT_00 },
    { "XPT_FULL_PID_PARSER_IBP_PCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_PCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_PCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_PCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_PCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_PCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_IBP_SCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_SCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_SCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_SCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_SCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_SCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_IBP_PSG_PROTOCOL_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_PSG_PROTOCOL_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_PSG_PROTOCOL_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_PSG_PROTOCOL_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_PSG_PROTOCOL_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_PSG_PROTOCOL_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_IBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PSG_ERR_MODE",        BCHP_XPT_FULL_PID_PARSER_PSG_ERR_MODE,             BCHP_XPT_FULL_PID_PARSER_PSG_ERR_MODE },
    { "XPT_RSBUFF_BO_IBP0",                      BCHP_XPT_RSBUFF_BO_IBP0,                           BCHP_XPT_RSBUFF_BO_IBP15 },
    { "XPT_RSBUFF_BO_PBP0",                      BCHP_XPT_RSBUFF_BO_PBP0,                           BCHP_XPT_RSBUFF_BO_PBP5 },
    { "XPT_RSBUFF_BO_MPOD_IBP0",                 BCHP_XPT_RSBUFF_BO_MPOD_IBP0,                      BCHP_XPT_RSBUFF_BO_MPOD_IBP15 },
    { "XPT_RSBUFF_IBP_BUFFER_ENABLE",            BCHP_XPT_RSBUFF_IBP_BUFFER_ENABLE,                 BCHP_XPT_RSBUFF_PBP_BUFFER_ENABLE },
    { "XPT_RSBUFF_MPOD_IBP_BUFFER_ENABLE",       BCHP_XPT_RSBUFF_MPOD_IBP_BUFFER_ENABLE,            BCHP_XPT_RSBUFF_MPOD_IBP_BUFFER_ENABLE },
    { "XPT_RSBUFF_OVERFLOW_THRESHOLD",           BCHP_XPT_RSBUFF_OVERFLOW_THRESHOLD,                BCHP_XPT_RSBUFF_NO_RD_HANG_CTRL },
    { "XPT_RSBUFF_PR_FALLBACK_CTRL",             BCHP_XPT_RSBUFF_PR_FALLBACK_CTRL,                  BCHP_XPT_RSBUFF_PR_FALLBACK_CTRL },
    { "XPT_XCBUFF_RAVE_CTRL_BUFFER_EN_PBP",      BCHP_XPT_XCBUFF_RAVE_CTRL_BUFFER_EN_PBP,           BCHP_XPT_XCBUFF_PAUSE_THRESHOLD },
    { "XPT_XCBUFF_OVERFLOW_THRESHOLD",           BCHP_XPT_XCBUFF_OVERFLOW_THRESHOLD,                BCHP_XPT_XCBUFF_OVERFLOW_THRESHOLD },
    { "XPT_XCBUFF_BYTE_SWAP_CTRL",               BCHP_XPT_XCBUFF_BYTE_SWAP_CTRL,                    BCHP_XPT_XCBUFF_BO_RAVE_IBP15 },
    { "XPT_XCBUFF_BO_RAVE_PBP0",                 BCHP_XPT_XCBUFF_BO_RAVE_PBP0,                      BCHP_XPT_XCBUFF_BO_RAVE_PBP5 },
    { "XPT_XCBUFF_BO_MSG_IBP0",                  BCHP_XPT_XCBUFF_BO_MSG_IBP0,                       BCHP_XPT_XCBUFF_BO_MSG_IBP15 },
    { "XPT_XCBUFF_BO_MSG_PBP0",                  BCHP_XPT_XCBUFF_BO_MSG_PBP0,                       BCHP_XPT_XCBUFF_BO_MSG_PBP5 },
    { "XPT_XCBUFF_BO_RMX0_IBP0",                 BCHP_XPT_XCBUFF_BO_RMX0_IBP0,                      BCHP_XPT_XCBUFF_BO_RMX0_IBP15 },
    { "XPT_XCBUFF_BO_RMX0_PBP0",                 BCHP_XPT_XCBUFF_BO_RMX0_PBP0,                      BCHP_XPT_XCBUFF_BO_RMX0_PBP5 },
    { "XPT_XCBUFF_BO_RMX1_IBP0",                 BCHP_XPT_XCBUFF_BO_RMX1_IBP0,                      BCHP_XPT_XCBUFF_BO_RMX1_IBP15 },
    { "XPT_XCBUFF_BO_RMX1_PBP0",                 BCHP_XPT_XCBUFF_BO_RMX1_PBP0,                      BCHP_XPT_XCBUFF_BO_RMX1_PBP5 },
    { "XPT_XCBUFF_MISC_CTRL",                    BCHP_XPT_XCBUFF_MISC_CTRL,                         BCHP_XPT_XCBUFF_NO_RD_HANG_CTRL },
    { "XPT_XCBUFF_PR_FALLBACK_CTRL",             BCHP_XPT_XCBUFF_PR_FALLBACK_CTRL,                  BCHP_XPT_XCBUFF_PR_FALLBACK_CTRL },
    { "XPT_MSG_MSG_CTRL1",                       BCHP_XPT_MSG_MSG_CTRL1,                            BCHP_XPT_MSG_DMA_BUFFER_INIT },
    { "XPT_MSG_PHY_ADDR_LO",                     BCHP_XPT_MSG_PHY_ADDR_LO,                          BCHP_XPT_MSG_ID_REJECT },
    { "XPT_MSG_C_MATCH0",                        BCHP_XPT_MSG_C_MATCH0,                             BCHP_XPT_MSG_C_MATCH4 },
    { "XPT_MSG_BUF_DAT_RDY_INTR_EN_00_31",       BCHP_XPT_MSG_BUF_DAT_RDY_INTR_EN_00_31,            BCHP_XPT_MSG_BUF_DAT_RDY_INTR_EN_96_127 },
    { "XPT_MSG_BUF_OVFL_INTR_EN_00_31",          BCHP_XPT_MSG_BUF_OVFL_INTR_EN_00_31,               BCHP_XPT_MSG_BUF_OVFL_INTR_EN_96_127 },
    { "XPT_MSG_DAT_ERR_INTR_EN",                 BCHP_XPT_MSG_DAT_ERR_INTR_EN,                      BCHP_XPT_MSG_DAT_ERR_INTR_EN },
    { "XPT_MSG_MSG_EVENT_CNT_CTRL",              BCHP_XPT_MSG_MSG_EVENT_CNT_CTRL,                   BCHP_XPT_MSG_MSG_EVENT_CNT_CTRL },
    { "XPT_RAVE_AVS_SCV_FILTER_MODE_CONTROL",    BCHP_XPT_RAVE_AVS_SCV_FILTER_MODE_CONTROL,         BCHP_XPT_RAVE_AVS_SCV_FILTER_VALUE_4_TO_7 },
#if (BCHP_VER == BCHP_VER_A0)
    { "XPT_RAVE_DATA_START_ADDR_A",              BCHP_XPT_RAVE_DATA_START_ADDR_A,                   BCHP_XPT_RAVE_WATCHDOG_TIMER_VALUE },
    { "XPT_RAVE_MISC_CONTROL",                   BCHP_XPT_RAVE_MISC_CONTROL,                        BCHP_XPT_RAVE_BASE_ADDRESSES },
    { "XPT_RAVE_MISC_CONTROL2",                  BCHP_XPT_RAVE_MISC_CONTROL2,                       BCHP_XPT_RAVE_MISC_CONTROL3 },
    { "BCHP_XPT_RAVE_RC8_SP_CONTROL",            BCHP_XPT_RAVE_RC0_SP_CONTROL,                      BCHP_XPT_RAVE_RC8_SP_CONTROL },
    { "XPT_RAVE_ATSOFFSET_MAX_ERROR",            BCHP_XPT_RAVE_ATSOFFSET_MAX_ERROR,                 BCHP_XPT_RAVE_ATSOFFSET_MAX_ERROR },
    { "XPT_RAVE_TPIT_TIME_TICK",                 BCHP_XPT_RAVE_TPIT_TIME_TICK,                      BCHP_XPT_RAVE_TPIT_EVE_TIMEOUT },
    { "XPT_RAVE_EMM_TID_MODE",                   BCHP_XPT_RAVE_EMM_TID_MODE,                        BCHP_XPT_RAVE_EMM_CTRL_ID },
    { "XPT_RAVE_EMM_DATA_ID_1",                  BCHP_XPT_RAVE_EMM_DATA_ID_1,                       BCHP_XPT_RAVE_EMM_MASK_ID_8 },
#else
    { "XPT_RAVE_DATA_START_ADDR_A",              BCHP_XPT_RAVE_DATA_START_ADDR_A,                   BCHP_XPT_RAVE_BASE_ADDRESSES },
    { "XPT_RAVE_MISC_CONTROL2",                  BCHP_XPT_RAVE_MISC_CONTROL2,                       BCHP_XPT_RAVE_RC8_SP_CONTROL },
    { "XPT_RAVE_ATSOFFSET_MAX_ERROR",            BCHP_XPT_RAVE_ATSOFFSET_MAX_ERROR,                 BCHP_XPT_RAVE_ATSOFFSET_MAX_ERROR },
    { "XPT_RAVE_TPIT_TIME_TICK",                 BCHP_XPT_RAVE_TPIT_TIME_TICK,                      BCHP_XPT_RAVE_EMM_MASK_ID_8 },
#endif
    {0, 0, 0}
};

#elif (BCHP_CHIP==7435)
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
#include "bchp_xpt_dpcr_pp.h"
#include "bchp_xpt_fe.h"
#include "bchp_xpt_full_pid_parser.h"
#include "bchp_xpt_gr.h"
#include "bchp_xpt_mpod.h"
#include "bchp_xpt_msg.h"
#include "bchp_xpt_pb0.h"
#include "bchp_xpt_pb1.h"
#include "bchp_xpt_pb10.h"
#include "bchp_xpt_pb11.h"
#include "bchp_xpt_pb12.h"
#include "bchp_xpt_pb13.h"
#include "bchp_xpt_pb14.h"
#include "bchp_xpt_pb15.h"
#include "bchp_xpt_pb16.h"
#include "bchp_xpt_pb17.h"
#include "bchp_xpt_pb18.h"
#include "bchp_xpt_pb19.h"
#include "bchp_xpt_pb2.h"
#include "bchp_xpt_pb20.h"
#include "bchp_xpt_pb21.h"
#include "bchp_xpt_pb3.h"
#include "bchp_xpt_pb4.h"
#include "bchp_xpt_pb5.h"
#include "bchp_xpt_pb6.h"
#include "bchp_xpt_pb7.h"
#include "bchp_xpt_pb8.h"
#include "bchp_xpt_pb9.h"
#include "bchp_xpt_pb_top.h"
#include "bchp_xpt_pcroffset.h"
#include "bchp_xpt_pmu.h"
#include "bchp_xpt_psub.h"
#include "bchp_xpt_rave.h"
#if (BCHP_VER >= BCHP_VER_B0)
#include "bchp_xpt_rave_to_scpu_l2_intr_0_31.h"
#include "bchp_xpt_rave_to_scpu_l2_intr_32_47.h"
#endif
#include "bchp_xpt_rmx0.h"
#include "bchp_xpt_rmx0_io.h"
#include "bchp_xpt_rmx1.h"
#include "bchp_xpt_rmx1_io.h"
#include "bchp_xpt_rsbuff.h"
#include "bchp_xpt_secure_bus_if.h"
#include "bchp_xpt_wakeup.h"
#include "bchp_xpt_xcbuff.h"
#include "bchp_xpt_xmemif.h"
#include "bchp_xpt_xpu.h"

const BXPT_P_RegisterRange XPT_SRAM_LIST[] =
{
    { "WAKEUP_MEM",                              BCHP_XPT_WAKEUP_PKT_MEM_i_ARRAY_BASE,              (BCHP_XPT_WAKEUP_PKT_TYPE3_i_ARRAY_BASE+(BCHP_XPT_WAKEUP_PKT_TYPE3_i_ARRAY_END*4)) },
    { "FE_PID_TABLE",                            BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE,                (BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE+(BCHP_XPT_FE_PID_TABLE_i_ARRAY_END*4)) },
    { "FE_SPID_TABLE",                           BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE,               (BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE+(BCHP_XPT_FE_SPID_TABLE_i_ARRAY_END*4)) },
    { "PCROFFSET_PID_TABLE",                     BCHP_XPT_PCROFFSET_PID_CONFIG_TABLE_i_ARRAY_BASE,  (BCHP_XPT_PCROFFSET_PID_CONFIG_TABLE_i_ARRAY_BASE+(BCHP_XPT_PCROFFSET_PID_CONFIG_TABLE_i_ARRAY_END*4)) },
    { "PCROFFSET_CX_TABLE",                      BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL,        BCHP_XPT_PCROFFSET_CONTEXT15_RESERVED_CFG_0 },
    { "FPP_SCM0",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_END*4)) },
    { "FPP_SCM1",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_1_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_1_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_1_i_ARRAY_END*4)) },
    { "FPP_SCM2",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_2_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_2_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_2_i_ARRAY_END*4)) },
    { "FPP_SCM3",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_3_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_3_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_3_i_ARRAY_END*4)) },
    { "RSBUFF_IBP_PMEM",                         BCHP_XPT_RSBUFF_BASE_POINTER_IBP0,                 BCHP_XPT_RSBUFF_WATERMARK_IBP15 },
    { "RSBUFF_PBP_PMEM",                         BCHP_XPT_RSBUFF_BASE_POINTER_PBP0,                 BCHP_XPT_RSBUFF_WATERMARK_PBP21 },
    { "RSBUFF_MPOD_PMEM",                        BCHP_XPT_RSBUFF_BASE_POINTER_MPOD_IBP0,            BCHP_XPT_RSBUFF_WATERMARK_MPOD_IBP15 },
    { "XCBUFF_RAVE_IBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RAVE_IBP0,            BCHP_XPT_XCBUFF_WATERMARK_RAVE_IBP15 },
    { "XCBUFF_RAVE_PBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RAVE_PBP0,            BCHP_XPT_XCBUFF_WATERMARK_RAVE_PBP21 },
    { "XCBUFF_MSG_IBP_PMEM",                     BCHP_XPT_XCBUFF_BASE_POINTER_MSG_IBP0,             BCHP_XPT_XCBUFF_WATERMARK_MSG_IBP15 },
    { "XCBUFF_MSG_PBP_PMEM",                     BCHP_XPT_XCBUFF_BASE_POINTER_MSG_PBP0,             BCHP_XPT_XCBUFF_WATERMARK_MSG_PBP21 },
    { "XCBUFF_RMX0_IBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX0_IBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX0_IBP15 },
    { "XCBUFF_RMX0_PBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX0_PBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX0_PBP21 },
    { "XCBUFF_RMX1_IBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX1_IBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX1_IBP15 },
    { "XCBUFF_RMX1_PBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX1_PBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX1_PBP21 },
    { "MSG_PMEM",                                BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_ARRAY_BASE,         (BCHP_XPT_MSG_DMA_VP_TABLE_i_ARRAY_BASE+(BCHP_XPT_MSG_DMA_VP_TABLE_i_ARRAY_END*4)) },
    { "MSG_GEN_FILT1",                           BCHP_XPT_MSG_GEN_FILT_EN_i_ARRAY_BASE,             (BCHP_XPT_MSG_GEN_FILT_EN_i_ARRAY_BASE+(BCHP_XPT_MSG_GEN_FILT_EN_i_ARRAY_END*4)) },
    { "MSG_GEN_FILT2",                           BCHP_XPT_MSG_MCAST_16_ADDR_i_ARRAY_BASE,           (BCHP_XPT_MSG_GEN_FILT_EXCL_i_ARRAY_BASE+(BCHP_XPT_MSG_GEN_FILT_EXCL_i_ARRAY_END*4)) },
    { "MSG_PID2BUF",                             BCHP_XPT_MSG_PID2BUF_MAP_i_ARRAY_BASE,             (BCHP_XPT_MSG_PID2BUF_MAP_i_ARRAY_BASE+(BCHP_XPT_MSG_PID2BUF_MAP_i_ARRAY_END*4)) },
    { "RAVE_PMEM",                               BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR,                BCHP_XPT_RAVE_SCD63_RESERVED_STATE3 },
    { "RAVE_CXMEM_A",                            BCHP_XPT_RAVE_CXMEM_Ai_ARRAY_BASE,                 (BCHP_XPT_RAVE_CXMEM_Ai_ARRAY_BASE+(BCHP_XPT_RAVE_CXMEM_Ai_ARRAY_END*4)) },
    { "RAVE_CXMEM_B",                            BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_BASE,                 (BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_BASE+(BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_END*4)) },
    { "RAVE_CXMEM_C",                            BCHP_XPT_RAVE_CXMEM_Ci_ARRAY_BASE,                 (BCHP_XPT_RAVE_CXMEM_Ci_ARRAY_BASE+(BCHP_XPT_RAVE_CXMEM_Ci_ARRAY_END*4)) },
    { "RAVE_DMEM",                               BCHP_XPT_RAVE_DMEMi_ARRAY_BASE,                    (BCHP_XPT_RAVE_DMEMi_ARRAY_BASE+(BCHP_XPT_RAVE_DMEMi_ARRAY_END*4)) },
    { "RAVE_SMEM",                               BCHP_XPT_RAVE_TPIT0_PID_TABLEi_ARRAY_BASE,         BCHP_XPT_RAVE_TPIT_STATE_CONTEXT47 },
    {0, 0, 0}
};

const BXPT_P_RegisterRange XPT_REG_SAVE_LIST[] =
{
    { "XPT_BUS_IF_MISC_CTRL0",                   BCHP_XPT_BUS_IF_MISC_CTRL0,                        BCHP_XPT_BUS_IF_TEST_MODE },
    { "XPT_BUS_IF_INTR_STATUS_REG_EN",           BCHP_XPT_BUS_IF_INTR_STATUS_REG_EN,                BCHP_XPT_BUS_IF_INTR_STATUS_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS2_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS2_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS2_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS3_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS3_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS3_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS4_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS4_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS4_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS5_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS5_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS5_REG_EN },
    { "XPT_XMEMIF_SCB_WR_ARB_SEL_RSBUFF",        BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RSBUFF,             BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB },
    { "XPT_XMEMIF_SCB_WR_ARB_MODE",              BCHP_XPT_XMEMIF_SCB_WR_ARB_MODE,                   BCHP_XPT_XMEMIF_SCB_RD_ARB_MODE },
    { "XPT_XMEMIF_INTR_STATUS_REG_EN",           BCHP_XPT_XMEMIF_INTR_STATUS_REG_EN,                BCHP_XPT_XMEMIF_INTR_STATUS_REG_EN },
    { "XPT_XMEMIF_RD_RDY_ACCEPT_PB_LCTOXMEM_2",  BCHP_XPT_XMEMIF_RD_RDY_ACCEPT_PB_LCTOXMEM_2,       BCHP_XPT_XMEMIF_RD_RDY_ACCEPT_PB_XMEMTOLC_2 },
    { "XPT_PMU_CLK_CTRL",                        BCHP_XPT_PMU_CLK_CTRL,                             BCHP_XPT_PMU_MEM_INIT_CTRL },
    { "XPT_GR_CTRL",                             BCHP_XPT_GR_CTRL,                                  BCHP_XPT_GR_CTRL },
    { "XPT_RMX0_IO_FORMAT",                      BCHP_XPT_RMX0_IO_FORMAT,                           BCHP_XPT_RMX0_IO_PKT_DLY_CNT },
    { "XPT_RMX1_IO_FORMAT",                      BCHP_XPT_RMX1_IO_FORMAT,                           BCHP_XPT_RMX1_IO_PKT_DLY_CNT },
    { "XPT_RMX1_IO_TV_STATUS",                   BCHP_XPT_RMX1_IO_TV_STATUS,                        BCHP_XPT_RMX1_IO_TV_STATUS },
    { "XPT_WAKEUP_CTRL",                         BCHP_XPT_WAKEUP_CTRL,                              BCHP_XPT_WAKEUP_CTRL },
    { "XPT_WAKEUP_INTR_STATUS_REG_EN",           BCHP_XPT_WAKEUP_INTR_STATUS_REG_EN,                BCHP_XPT_WAKEUP_INTR_STATUS_REG_EN },
    { "XPT_FE_FE_CTRL",                          BCHP_XPT_FE_FE_CTRL,                               BCHP_XPT_FE_FE_CTRL },
    { "XPT_FE_INTR_STATUS0_REG_EN",              BCHP_XPT_FE_INTR_STATUS0_REG_EN,                   BCHP_XPT_FE_PWR_CTRL },
    { "XPT_FE_MAX_PID_CHANNEL",                  BCHP_XPT_FE_MAX_PID_CHANNEL,                       BCHP_XPT_FE_MAX_PID_CHANNEL },
    { "XPT_FE_IB0_CTRL",                         BCHP_XPT_FE_IB0_CTRL,                              BCHP_XPT_FE_IB0_CTRL },
    { "XPT_FE_IB1_CTRL",                         BCHP_XPT_FE_IB1_CTRL,                              BCHP_XPT_FE_IB1_CTRL },
    { "XPT_FE_IB2_CTRL",                         BCHP_XPT_FE_IB2_CTRL,                              BCHP_XPT_FE_IB2_CTRL },
    { "XPT_FE_IB3_CTRL",                         BCHP_XPT_FE_IB3_CTRL,                              BCHP_XPT_FE_IB3_CTRL },
    { "XPT_FE_IB4_CTRL",                         BCHP_XPT_FE_IB4_CTRL,                              BCHP_XPT_FE_IB4_CTRL },
    { "XPT_FE_IB5_CTRL",                         BCHP_XPT_FE_IB5_CTRL,                              BCHP_XPT_FE_IB5_CTRL },
    { "XPT_FE_IB6_CTRL",                         BCHP_XPT_FE_IB6_CTRL,                              BCHP_XPT_FE_IB6_CTRL },
    { "XPT_FE_IB7_CTRL",                         BCHP_XPT_FE_IB7_CTRL,                              BCHP_XPT_FE_IB7_CTRL },
    { "XPT_FE_IB8_CTRL",                         BCHP_XPT_FE_IB8_CTRL,                              BCHP_XPT_FE_IB8_CTRL },
    { "XPT_FE_IB9_CTRL",                         BCHP_XPT_FE_IB9_CTRL,                              BCHP_XPT_FE_IB9_CTRL },
    { "XPT_FE_IB10_CTRL",                        BCHP_XPT_FE_IB10_CTRL,                             BCHP_XPT_FE_IB10_CTRL },
    { "XPT_FE_MINI_PID_PARSER0_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER0_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER0_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER1_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER1_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER1_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER2_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER2_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER2_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER3_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER3_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER3_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER4_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER4_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER4_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER5_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER5_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER5_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER6_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER6_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER6_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER7_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER7_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER7_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER8_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER8_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER8_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER9_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER9_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER9_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER10_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER10_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER10_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER11_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER11_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER11_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER12_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER12_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER12_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER13_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER13_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER13_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER14_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER14_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER14_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER15_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER15_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER15_CTRL2 },
    { "XPT_FE_IB_SYNC_DETECT_CTRL",              BCHP_XPT_FE_IB_SYNC_DETECT_CTRL,                   BCHP_XPT_FE_IB_SYNC_DETECT_CTRL },
    { "XPT_FE_TSMF0_CTRL",                       BCHP_XPT_FE_TSMF0_CTRL,                            BCHP_XPT_FE_TSMF0_SLOT_MAP_HI },
    { "XPT_FE_TSMF1_CTRL",                       BCHP_XPT_FE_TSMF1_CTRL,                            BCHP_XPT_FE_TSMF1_SLOT_MAP_HI },
    { "XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID", BCHP_XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID,   BCHP_XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID },
    { "XPT_FE_ATS_COUNTER_CTRL",                 BCHP_XPT_FE_ATS_COUNTER_CTRL,                      BCHP_XPT_FE_ATS_TS_BINARY },
    { "XPT_FE_MTSIF_RX0_CTRL1",                  BCHP_XPT_FE_MTSIF_RX0_CTRL1,                       BCHP_XPT_FE_MTSIF_RX0_CTRL1 },
    { "XPT_FE_MTSIF_RX0_INTR_STATUS0_REG_EN",    BCHP_XPT_FE_MTSIF_RX0_INTR_STATUS0_REG_EN,         BCHP_XPT_FE_MTSIF_RX0_BAND_ID_DROP },
    { "XPT_FE_MTSIF_RX1_CTRL1",                  BCHP_XPT_FE_MTSIF_RX1_CTRL1,                       BCHP_XPT_FE_MTSIF_RX1_CTRL1 },
    { "XPT_FE_MTSIF_RX1_INTR_STATUS0_REG_EN",    BCHP_XPT_FE_MTSIF_RX1_INTR_STATUS0_REG_EN,         BCHP_XPT_FE_MTSIF_RX1_BAND_ID_DROP },
    { "XPT_DPCR0_PID_CH",                        BCHP_XPT_DPCR0_PID_CH,                             BCHP_XPT_DPCR0_CTRL },
    { "XPT_DPCR0_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR0_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR0_STC_EXT_CTRL },
    { "XPT_DPCR0_MAX_PCR_ERROR",                 BCHP_XPT_DPCR0_MAX_PCR_ERROR,                      BCHP_XPT_DPCR0_MAX_PCR_ERROR },
    { "XPT_DPCR0_STC_HI",                        BCHP_XPT_DPCR0_STC_HI,                             BCHP_XPT_DPCR0_STC_LO },
    { "XPT_DPCR0_LOOP_CTRL",                     BCHP_XPT_DPCR0_LOOP_CTRL,                          BCHP_XPT_DPCR0_ACCUM_VALUE },
    { "XPT_DPCR0_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR0_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR0_PHASE_ERROR_CLAMP },
    { "XPT_DPCR1_PID_CH",                        BCHP_XPT_DPCR1_PID_CH,                             BCHP_XPT_DPCR1_CTRL },
    { "XPT_DPCR1_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR1_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR1_STC_EXT_CTRL },
    { "XPT_DPCR1_MAX_PCR_ERROR",                 BCHP_XPT_DPCR1_MAX_PCR_ERROR,                      BCHP_XPT_DPCR1_MAX_PCR_ERROR },
    { "XPT_DPCR1_STC_HI",                        BCHP_XPT_DPCR1_STC_HI,                             BCHP_XPT_DPCR1_STC_LO },
    { "XPT_DPCR1_LOOP_CTRL",                     BCHP_XPT_DPCR1_LOOP_CTRL,                          BCHP_XPT_DPCR1_ACCUM_VALUE },
    { "XPT_DPCR1_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR1_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR1_PHASE_ERROR_CLAMP },
    { "XPT_DPCR2_PID_CH",                        BCHP_XPT_DPCR2_PID_CH,                             BCHP_XPT_DPCR2_CTRL },
    { "XPT_DPCR2_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR2_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR2_STC_EXT_CTRL },
    { "XPT_DPCR2_MAX_PCR_ERROR",                 BCHP_XPT_DPCR2_MAX_PCR_ERROR,                      BCHP_XPT_DPCR2_MAX_PCR_ERROR },
    { "XPT_DPCR2_STC_HI",                        BCHP_XPT_DPCR2_STC_HI,                             BCHP_XPT_DPCR2_STC_LO },
    { "XPT_DPCR2_LOOP_CTRL",                     BCHP_XPT_DPCR2_LOOP_CTRL,                          BCHP_XPT_DPCR2_ACCUM_VALUE },
    { "XPT_DPCR2_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR2_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR2_PHASE_ERROR_CLAMP },
    { "XPT_DPCR3_PID_CH",                        BCHP_XPT_DPCR3_PID_CH,                             BCHP_XPT_DPCR3_CTRL },
    { "XPT_DPCR3_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR3_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR3_STC_EXT_CTRL },
    { "XPT_DPCR3_MAX_PCR_ERROR",                 BCHP_XPT_DPCR3_MAX_PCR_ERROR,                      BCHP_XPT_DPCR3_MAX_PCR_ERROR },
    { "XPT_DPCR3_STC_HI",                        BCHP_XPT_DPCR3_STC_HI,                             BCHP_XPT_DPCR3_STC_LO },
    { "XPT_DPCR3_LOOP_CTRL",                     BCHP_XPT_DPCR3_LOOP_CTRL,                          BCHP_XPT_DPCR3_ACCUM_VALUE },
    { "XPT_DPCR3_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR3_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR3_PHASE_ERROR_CLAMP },
    { "XPT_DPCR4_PID_CH",                        BCHP_XPT_DPCR4_PID_CH,                             BCHP_XPT_DPCR4_CTRL },
    { "XPT_DPCR4_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR4_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR4_STC_EXT_CTRL },
    { "XPT_DPCR4_MAX_PCR_ERROR",                 BCHP_XPT_DPCR4_MAX_PCR_ERROR,                      BCHP_XPT_DPCR4_MAX_PCR_ERROR },
    { "XPT_DPCR4_STC_HI",                        BCHP_XPT_DPCR4_STC_HI,                             BCHP_XPT_DPCR4_STC_LO },
    { "XPT_DPCR4_LOOP_CTRL",                     BCHP_XPT_DPCR4_LOOP_CTRL,                          BCHP_XPT_DPCR4_ACCUM_VALUE },
    { "XPT_DPCR4_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR4_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR4_PHASE_ERROR_CLAMP },
    { "XPT_DPCR5_PID_CH",                        BCHP_XPT_DPCR5_PID_CH,                             BCHP_XPT_DPCR5_CTRL },
    { "XPT_DPCR5_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR5_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR5_STC_EXT_CTRL },
    { "XPT_DPCR5_MAX_PCR_ERROR",                 BCHP_XPT_DPCR5_MAX_PCR_ERROR,                      BCHP_XPT_DPCR5_MAX_PCR_ERROR },
    { "XPT_DPCR5_STC_HI",                        BCHP_XPT_DPCR5_STC_HI,                             BCHP_XPT_DPCR5_STC_LO },
    { "XPT_DPCR5_LOOP_CTRL",                     BCHP_XPT_DPCR5_LOOP_CTRL,                          BCHP_XPT_DPCR5_ACCUM_VALUE },
    { "XPT_DPCR5_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR5_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR5_PHASE_ERROR_CLAMP },
    { "XPT_DPCR6_PID_CH",                        BCHP_XPT_DPCR6_PID_CH,                             BCHP_XPT_DPCR6_CTRL },
    { "XPT_DPCR6_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR6_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR6_STC_EXT_CTRL },
    { "XPT_DPCR6_MAX_PCR_ERROR",                 BCHP_XPT_DPCR6_MAX_PCR_ERROR,                      BCHP_XPT_DPCR6_MAX_PCR_ERROR },
    { "XPT_DPCR6_STC_HI",                        BCHP_XPT_DPCR6_STC_HI,                             BCHP_XPT_DPCR6_STC_LO },
    { "XPT_DPCR6_LOOP_CTRL",                     BCHP_XPT_DPCR6_LOOP_CTRL,                          BCHP_XPT_DPCR6_ACCUM_VALUE },
    { "XPT_DPCR6_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR6_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR6_PHASE_ERROR_CLAMP },
    { "XPT_DPCR7_PID_CH",                        BCHP_XPT_DPCR7_PID_CH,                             BCHP_XPT_DPCR7_CTRL },
    { "XPT_DPCR7_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR7_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR7_STC_EXT_CTRL },
    { "XPT_DPCR7_MAX_PCR_ERROR",                 BCHP_XPT_DPCR7_MAX_PCR_ERROR,                      BCHP_XPT_DPCR7_MAX_PCR_ERROR },
    { "XPT_DPCR7_STC_HI",                        BCHP_XPT_DPCR7_STC_HI,                             BCHP_XPT_DPCR7_STC_LO },
    { "XPT_DPCR7_LOOP_CTRL",                     BCHP_XPT_DPCR7_LOOP_CTRL,                          BCHP_XPT_DPCR7_ACCUM_VALUE },
    { "XPT_DPCR7_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR7_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR7_PHASE_ERROR_CLAMP },
    { "XPT_DPCR8_PID_CH",                        BCHP_XPT_DPCR8_PID_CH,                             BCHP_XPT_DPCR8_CTRL },
    { "XPT_DPCR8_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR8_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR8_STC_EXT_CTRL },
    { "XPT_DPCR8_MAX_PCR_ERROR",                 BCHP_XPT_DPCR8_MAX_PCR_ERROR,                      BCHP_XPT_DPCR8_MAX_PCR_ERROR },
    { "XPT_DPCR8_STC_HI",                        BCHP_XPT_DPCR8_STC_HI,                             BCHP_XPT_DPCR8_STC_LO },
    { "XPT_DPCR8_LOOP_CTRL",                     BCHP_XPT_DPCR8_LOOP_CTRL,                          BCHP_XPT_DPCR8_ACCUM_VALUE },
    { "XPT_DPCR8_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR8_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR8_PHASE_ERROR_CLAMP },
    { "XPT_DPCR9_PID_CH",                        BCHP_XPT_DPCR9_PID_CH,                             BCHP_XPT_DPCR9_CTRL },
    { "XPT_DPCR9_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR9_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR9_STC_EXT_CTRL },
    { "XPT_DPCR9_MAX_PCR_ERROR",                 BCHP_XPT_DPCR9_MAX_PCR_ERROR,                      BCHP_XPT_DPCR9_MAX_PCR_ERROR },
    { "XPT_DPCR9_STC_HI",                        BCHP_XPT_DPCR9_STC_HI,                             BCHP_XPT_DPCR9_STC_LO },
    { "XPT_DPCR9_LOOP_CTRL",                     BCHP_XPT_DPCR9_LOOP_CTRL,                          BCHP_XPT_DPCR9_ACCUM_VALUE },
    { "XPT_DPCR9_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR9_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR9_PHASE_ERROR_CLAMP },
    { "XPT_DPCR_PP_PP_CTRL",                     BCHP_XPT_DPCR_PP_PP_CTRL,                          BCHP_XPT_DPCR_PP_PP_FIXED_OFFSET },
    { "XPT_PSUB_PSUB0_CTRL0",                    BCHP_XPT_PSUB_PSUB0_CTRL0,                         BCHP_XPT_PSUB_PSUB0_STAT0 },
    { "XPT_PSUB_PSUB1_CTRL0",                    BCHP_XPT_PSUB_PSUB1_CTRL0,                         BCHP_XPT_PSUB_PSUB1_STAT0 },
    { "XPT_PSUB_PSUB2_CTRL0",                    BCHP_XPT_PSUB_PSUB2_CTRL0,                         BCHP_XPT_PSUB_PSUB2_STAT0 },
    { "XPT_PSUB_PSUB3_CTRL0",                    BCHP_XPT_PSUB_PSUB3_CTRL0,                         BCHP_XPT_PSUB_PSUB3_STAT0 },
    { "XPT_PSUB_PSUB4_CTRL0",                    BCHP_XPT_PSUB_PSUB4_CTRL0,                         BCHP_XPT_PSUB_PSUB4_STAT0 },
    { "XPT_PSUB_PSUB5_CTRL0",                    BCHP_XPT_PSUB_PSUB5_CTRL0,                         BCHP_XPT_PSUB_PSUB5_STAT0 },
    { "XPT_PSUB_PSUB6_CTRL0",                    BCHP_XPT_PSUB_PSUB6_CTRL0,                         BCHP_XPT_PSUB_PSUB6_STAT0 },
    { "XPT_PSUB_PSUB7_CTRL0",                    BCHP_XPT_PSUB_PSUB7_CTRL0,                         BCHP_XPT_PSUB_PSUB7_STAT0 },
    { "XPT_PSUB_PR_FALLBACK_CTRL",               BCHP_XPT_PSUB_PR_FALLBACK_CTRL,                    BCHP_XPT_PSUB_PR_FALLBACK_CTRL },
    { "XPT_MPOD_CFG",                            BCHP_XPT_MPOD_CFG,                                 BCHP_XPT_MPOD_ICTRL },
    { "XPT_MPOD_OCTRL2",                         BCHP_XPT_MPOD_OCTRL2,                              BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP },
    { "XPT_RMX0_CTRL",                           BCHP_XPT_RMX0_CTRL,                                BCHP_XPT_RMX0_FIXED_OFFSET },
    { "XPT_RMX1_CTRL",                           BCHP_XPT_RMX1_CTRL,                                BCHP_XPT_RMX1_FIXED_OFFSET },
    { "XPT_PB_TOP_PACING_COUNT_WR_VALUE",        BCHP_XPT_PB_TOP_PACING_COUNT_WR_VALUE,             BCHP_XPT_PB_TOP_PACING_PAUSE_MAP_VECT_PB21 },
    { "XPT_PB0_CTRL1",                           BCHP_XPT_PB0_CTRL1,                                BCHP_XPT_PB0_FIRST_DESC_ADDR },
    { "XPT_PB0_BLOCKOUT",                        BCHP_XPT_PB0_BLOCKOUT,                             BCHP_XPT_PB0_PARSER_CTRL2 },
    { "XPT_PB0_INTR_EN",                         BCHP_XPT_PB0_INTR_EN,                              BCHP_XPT_PB0_PWR_CTRL },
    { "XPT_PB0_ASF_CTRL",                        BCHP_XPT_PB0_ASF_CTRL,                             BCHP_XPT_PB0_PES_BASED_PACING_CTRL },
    { "XPT_PB1_CTRL1",                           BCHP_XPT_PB1_CTRL1,                                BCHP_XPT_PB1_FIRST_DESC_ADDR },
    { "XPT_PB1_BLOCKOUT",                        BCHP_XPT_PB1_BLOCKOUT,                             BCHP_XPT_PB1_PARSER_CTRL2 },
    { "XPT_PB1_INTR_EN",                         BCHP_XPT_PB1_INTR_EN,                              BCHP_XPT_PB1_PWR_CTRL },
    { "XPT_PB1_ASF_CTRL",                        BCHP_XPT_PB1_ASF_CTRL,                             BCHP_XPT_PB1_PES_BASED_PACING_CTRL },
    { "XPT_PB2_CTRL1",                           BCHP_XPT_PB2_CTRL1,                                BCHP_XPT_PB2_FIRST_DESC_ADDR },
    { "XPT_PB2_BLOCKOUT",                        BCHP_XPT_PB2_BLOCKOUT,                             BCHP_XPT_PB2_PARSER_CTRL2 },
    { "XPT_PB2_INTR_EN",                         BCHP_XPT_PB2_INTR_EN,                              BCHP_XPT_PB2_PWR_CTRL },
    { "XPT_PB2_ASF_CTRL",                        BCHP_XPT_PB2_ASF_CTRL,                             BCHP_XPT_PB2_PES_BASED_PACING_CTRL },
    { "XPT_PB3_CTRL1",                           BCHP_XPT_PB3_CTRL1,                                BCHP_XPT_PB3_FIRST_DESC_ADDR },
    { "XPT_PB3_BLOCKOUT",                        BCHP_XPT_PB3_BLOCKOUT,                             BCHP_XPT_PB3_PARSER_CTRL2 },
    { "XPT_PB3_INTR_EN",                         BCHP_XPT_PB3_INTR_EN,                              BCHP_XPT_PB3_PWR_CTRL },
    { "XPT_PB3_ASF_CTRL",                        BCHP_XPT_PB3_ASF_CTRL,                             BCHP_XPT_PB3_PES_BASED_PACING_CTRL },
    { "XPT_PB4_CTRL1",                           BCHP_XPT_PB4_CTRL1,                                BCHP_XPT_PB4_FIRST_DESC_ADDR },
    { "XPT_PB4_BLOCKOUT",                        BCHP_XPT_PB4_BLOCKOUT,                             BCHP_XPT_PB4_PARSER_CTRL2 },
    { "XPT_PB4_INTR_EN",                         BCHP_XPT_PB4_INTR_EN,                              BCHP_XPT_PB4_PWR_CTRL },
    { "XPT_PB4_ASF_CTRL",                        BCHP_XPT_PB4_ASF_CTRL,                             BCHP_XPT_PB4_PES_BASED_PACING_CTRL },
    { "XPT_PB5_CTRL1",                           BCHP_XPT_PB5_CTRL1,                                BCHP_XPT_PB5_FIRST_DESC_ADDR },
    { "XPT_PB5_BLOCKOUT",                        BCHP_XPT_PB5_BLOCKOUT,                             BCHP_XPT_PB5_PARSER_CTRL2 },
    { "XPT_PB5_INTR_EN",                         BCHP_XPT_PB5_INTR_EN,                              BCHP_XPT_PB5_PWR_CTRL },
    { "XPT_PB5_ASF_CTRL",                        BCHP_XPT_PB5_ASF_CTRL,                             BCHP_XPT_PB5_PES_BASED_PACING_CTRL },
    { "XPT_PB6_CTRL1",                           BCHP_XPT_PB6_CTRL1,                                BCHP_XPT_PB6_FIRST_DESC_ADDR },
    { "XPT_PB6_BLOCKOUT",                        BCHP_XPT_PB6_BLOCKOUT,                             BCHP_XPT_PB6_PARSER_CTRL2 },
    { "XPT_PB6_INTR_EN",                         BCHP_XPT_PB6_INTR_EN,                              BCHP_XPT_PB6_PWR_CTRL },
    { "XPT_PB6_ASF_CTRL",                        BCHP_XPT_PB6_ASF_CTRL,                             BCHP_XPT_PB6_PES_BASED_PACING_CTRL },
    { "XPT_PB7_CTRL1",                           BCHP_XPT_PB7_CTRL1,                                BCHP_XPT_PB7_FIRST_DESC_ADDR },
    { "XPT_PB7_BLOCKOUT",                        BCHP_XPT_PB7_BLOCKOUT,                             BCHP_XPT_PB7_PARSER_CTRL2 },
    { "XPT_PB7_INTR_EN",                         BCHP_XPT_PB7_INTR_EN,                              BCHP_XPT_PB7_PWR_CTRL },
    { "XPT_PB7_ASF_CTRL",                        BCHP_XPT_PB7_ASF_CTRL,                             BCHP_XPT_PB7_PES_BASED_PACING_CTRL },
    { "XPT_PB8_CTRL1",                           BCHP_XPT_PB8_CTRL1,                                BCHP_XPT_PB8_FIRST_DESC_ADDR },
    { "XPT_PB8_BLOCKOUT",                        BCHP_XPT_PB8_BLOCKOUT,                             BCHP_XPT_PB8_PARSER_CTRL2 },
    { "XPT_PB8_INTR_EN",                         BCHP_XPT_PB8_INTR_EN,                              BCHP_XPT_PB8_PWR_CTRL },
    { "XPT_PB8_ASF_CTRL",                        BCHP_XPT_PB8_ASF_CTRL,                             BCHP_XPT_PB8_PES_BASED_PACING_CTRL },
    { "XPT_PB9_CTRL1",                           BCHP_XPT_PB9_CTRL1,                                BCHP_XPT_PB9_FIRST_DESC_ADDR },
    { "XPT_PB9_BLOCKOUT",                        BCHP_XPT_PB9_BLOCKOUT,                             BCHP_XPT_PB9_PARSER_CTRL2 },
    { "XPT_PB9_INTR_EN",                         BCHP_XPT_PB9_INTR_EN,                              BCHP_XPT_PB9_PWR_CTRL },
    { "XPT_PB9_ASF_CTRL",                        BCHP_XPT_PB9_ASF_CTRL,                             BCHP_XPT_PB9_PES_BASED_PACING_CTRL },
    { "XPT_PB10_CTRL1",                          BCHP_XPT_PB10_CTRL1,                               BCHP_XPT_PB10_FIRST_DESC_ADDR },
    { "XPT_PB10_BLOCKOUT",                       BCHP_XPT_PB10_BLOCKOUT,                            BCHP_XPT_PB10_PARSER_CTRL2 },
    { "XPT_PB10_INTR_EN",                        BCHP_XPT_PB10_INTR_EN,                             BCHP_XPT_PB10_PWR_CTRL },
    { "XPT_PB10_ASF_CTRL",                       BCHP_XPT_PB10_ASF_CTRL,                            BCHP_XPT_PB10_PES_BASED_PACING_CTRL },
    { "XPT_PB11_CTRL1",                          BCHP_XPT_PB11_CTRL1,                               BCHP_XPT_PB11_FIRST_DESC_ADDR },
    { "XPT_PB11_BLOCKOUT",                       BCHP_XPT_PB11_BLOCKOUT,                            BCHP_XPT_PB11_PARSER_CTRL2 },
    { "XPT_PB11_INTR_EN",                        BCHP_XPT_PB11_INTR_EN,                             BCHP_XPT_PB11_PWR_CTRL },
    { "XPT_PB11_ASF_CTRL",                       BCHP_XPT_PB11_ASF_CTRL,                            BCHP_XPT_PB11_PES_BASED_PACING_CTRL },
    { "XPT_PB12_CTRL1",                          BCHP_XPT_PB12_CTRL1,                               BCHP_XPT_PB12_FIRST_DESC_ADDR },
    { "XPT_PB12_BLOCKOUT",                       BCHP_XPT_PB12_BLOCKOUT,                            BCHP_XPT_PB12_PARSER_CTRL2 },
    { "XPT_PB12_INTR_EN",                        BCHP_XPT_PB12_INTR_EN,                             BCHP_XPT_PB12_PWR_CTRL },
    { "XPT_PB12_ASF_CTRL",                       BCHP_XPT_PB12_ASF_CTRL,                            BCHP_XPT_PB12_PES_BASED_PACING_CTRL },
    { "XPT_PB13_CTRL1",                          BCHP_XPT_PB13_CTRL1,                               BCHP_XPT_PB13_FIRST_DESC_ADDR },
    { "XPT_PB13_BLOCKOUT",                       BCHP_XPT_PB13_BLOCKOUT,                            BCHP_XPT_PB13_PARSER_CTRL2 },
    { "XPT_PB13_INTR_EN",                        BCHP_XPT_PB13_INTR_EN,                             BCHP_XPT_PB13_PWR_CTRL },
    { "XPT_PB13_ASF_CTRL",                       BCHP_XPT_PB13_ASF_CTRL,                            BCHP_XPT_PB13_PES_BASED_PACING_CTRL },
    { "XPT_PB14_CTRL1",                          BCHP_XPT_PB14_CTRL1,                               BCHP_XPT_PB14_FIRST_DESC_ADDR },
    { "XPT_PB14_BLOCKOUT",                       BCHP_XPT_PB14_BLOCKOUT,                            BCHP_XPT_PB14_PARSER_CTRL2 },
    { "XPT_PB14_INTR_EN",                        BCHP_XPT_PB14_INTR_EN,                             BCHP_XPT_PB14_PWR_CTRL },
    { "XPT_PB14_ASF_CTRL",                       BCHP_XPT_PB14_ASF_CTRL,                            BCHP_XPT_PB14_PES_BASED_PACING_CTRL },
    { "XPT_PB15_CTRL1",                          BCHP_XPT_PB15_CTRL1,                               BCHP_XPT_PB15_FIRST_DESC_ADDR },
    { "XPT_PB15_BLOCKOUT",                       BCHP_XPT_PB15_BLOCKOUT,                            BCHP_XPT_PB15_PARSER_CTRL2 },
    { "XPT_PB15_INTR_EN",                        BCHP_XPT_PB15_INTR_EN,                             BCHP_XPT_PB15_PWR_CTRL },
    { "XPT_PB15_ASF_CTRL",                       BCHP_XPT_PB15_ASF_CTRL,                            BCHP_XPT_PB15_PES_BASED_PACING_CTRL },
    { "XPT_PB16_CTRL1",                          BCHP_XPT_PB16_CTRL1,                               BCHP_XPT_PB16_FIRST_DESC_ADDR },
    { "XPT_PB16_BLOCKOUT",                       BCHP_XPT_PB16_BLOCKOUT,                            BCHP_XPT_PB16_PARSER_CTRL2 },
    { "XPT_PB16_INTR_EN",                        BCHP_XPT_PB16_INTR_EN,                             BCHP_XPT_PB16_PWR_CTRL },
    { "XPT_PB16_ASF_CTRL",                       BCHP_XPT_PB16_ASF_CTRL,                            BCHP_XPT_PB16_PES_BASED_PACING_CTRL },
    { "XPT_PB17_CTRL1",                          BCHP_XPT_PB17_CTRL1,                               BCHP_XPT_PB17_FIRST_DESC_ADDR },
    { "XPT_PB17_BLOCKOUT",                       BCHP_XPT_PB17_BLOCKOUT,                            BCHP_XPT_PB17_PARSER_CTRL2 },
    { "XPT_PB17_INTR_EN",                        BCHP_XPT_PB17_INTR_EN,                             BCHP_XPT_PB17_PWR_CTRL },
    { "XPT_PB17_ASF_CTRL",                       BCHP_XPT_PB17_ASF_CTRL,                            BCHP_XPT_PB17_PES_BASED_PACING_CTRL },
    { "XPT_PB18_CTRL1",                          BCHP_XPT_PB18_CTRL1,                               BCHP_XPT_PB18_FIRST_DESC_ADDR },
    { "XPT_PB18_BLOCKOUT",                       BCHP_XPT_PB18_BLOCKOUT,                            BCHP_XPT_PB18_PARSER_CTRL2 },
    { "XPT_PB18_INTR_EN",                        BCHP_XPT_PB18_INTR_EN,                             BCHP_XPT_PB18_PWR_CTRL },
    { "XPT_PB18_ASF_CTRL",                       BCHP_XPT_PB18_ASF_CTRL,                            BCHP_XPT_PB18_PES_BASED_PACING_CTRL },
    { "XPT_PB19_CTRL1",                          BCHP_XPT_PB19_CTRL1,                               BCHP_XPT_PB19_FIRST_DESC_ADDR },
    { "XPT_PB19_BLOCKOUT",                       BCHP_XPT_PB19_BLOCKOUT,                            BCHP_XPT_PB19_PARSER_CTRL2 },
    { "XPT_PB19_INTR_EN",                        BCHP_XPT_PB19_INTR_EN,                             BCHP_XPT_PB19_PWR_CTRL },
    { "XPT_PB19_ASF_CTRL",                       BCHP_XPT_PB19_ASF_CTRL,                            BCHP_XPT_PB19_PES_BASED_PACING_CTRL },
    { "XPT_PB20_CTRL1",                          BCHP_XPT_PB20_CTRL1,                               BCHP_XPT_PB20_FIRST_DESC_ADDR },
    { "XPT_PB20_BLOCKOUT",                       BCHP_XPT_PB20_BLOCKOUT,                            BCHP_XPT_PB20_PARSER_CTRL2 },
    { "XPT_PB20_INTR_EN",                        BCHP_XPT_PB20_INTR_EN,                             BCHP_XPT_PB20_PWR_CTRL },
    { "XPT_PB20_ASF_CTRL",                       BCHP_XPT_PB20_ASF_CTRL,                            BCHP_XPT_PB20_PES_BASED_PACING_CTRL },
    { "XPT_PB21_CTRL1",                          BCHP_XPT_PB21_CTRL1,                               BCHP_XPT_PB21_FIRST_DESC_ADDR },
    { "XPT_PB21_BLOCKOUT",                       BCHP_XPT_PB21_BLOCKOUT,                            BCHP_XPT_PB21_PARSER_CTRL2 },
    { "XPT_PB21_INTR_EN",                        BCHP_XPT_PB21_INTR_EN,                             BCHP_XPT_PB21_PWR_CTRL },
    { "XPT_PB21_ASF_CTRL",                       BCHP_XPT_PB21_ASF_CTRL,                            BCHP_XPT_PB21_PES_BASED_PACING_CTRL },
    { "XPT_PCROFFSET_INTERRUPT0_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT0_ENABLE,              BCHP_XPT_PCROFFSET_INTERRUPT0_ENABLE },
    { "XPT_PCROFFSET_INTERRUPT1_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT1_ENABLE,              BCHP_XPT_PCROFFSET_INTERRUPT1_ENABLE },
    { "XPT_PCROFFSET_INTERRUPT2_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT2_ENABLE,              BCHP_XPT_PCROFFSET_INTERRUPT2_ENABLE },
    { "XPT_PCROFFSET_INTERRUPT3_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT3_ENABLE,              BCHP_XPT_PCROFFSET_STC_MUX_DELAY },
    { "XPT_PCROFFSET_STC_INTERRUPT_ENABLE",      BCHP_XPT_PCROFFSET_STC_INTERRUPT_ENABLE,           BCHP_XPT_PCROFFSET_STC7_MATCH },
    { "XPT_PCROFFSET_STC0_CTRL",                 BCHP_XPT_PCROFFSET_STC0_CTRL,                      BCHP_XPT_PCROFFSET_STC0_INC_LO },
    { "XPT_PCROFFSET_STC0_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC0_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC0_AV_WINDOW },
    { "XPT_PCROFFSET_STC1_CTRL",                 BCHP_XPT_PCROFFSET_STC1_CTRL,                      BCHP_XPT_PCROFFSET_STC1_INC_LO },
    { "XPT_PCROFFSET_STC1_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC1_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC1_AV_WINDOW },
    { "XPT_PCROFFSET_STC2_CTRL",                 BCHP_XPT_PCROFFSET_STC2_CTRL,                      BCHP_XPT_PCROFFSET_STC2_INC_LO },
    { "XPT_PCROFFSET_STC2_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC2_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC2_AV_WINDOW },
    { "XPT_PCROFFSET_STC3_CTRL",                 BCHP_XPT_PCROFFSET_STC3_CTRL,                      BCHP_XPT_PCROFFSET_STC3_INC_LO },
    { "XPT_PCROFFSET_STC3_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC3_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC3_AV_WINDOW },
    { "XPT_PCROFFSET_STC4_CTRL",                 BCHP_XPT_PCROFFSET_STC4_CTRL,                      BCHP_XPT_PCROFFSET_STC4_INC_LO },
    { "XPT_PCROFFSET_STC4_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC4_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC4_AV_WINDOW },
    { "XPT_PCROFFSET_STC5_CTRL",                 BCHP_XPT_PCROFFSET_STC5_CTRL,                      BCHP_XPT_PCROFFSET_STC5_INC_LO },
    { "XPT_PCROFFSET_STC5_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC5_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC5_AV_WINDOW },
    { "XPT_PCROFFSET_STC6_CTRL",                 BCHP_XPT_PCROFFSET_STC6_CTRL,                      BCHP_XPT_PCROFFSET_STC6_INC_LO },
    { "XPT_PCROFFSET_STC6_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC6_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC6_AV_WINDOW },
    { "XPT_PCROFFSET_STC7_CTRL",                 BCHP_XPT_PCROFFSET_STC7_CTRL,                      BCHP_XPT_PCROFFSET_STC7_INC_LO },
    { "XPT_PCROFFSET_STC7_AV_WINDOW",            BCHP_XPT_PCROFFSET_STC7_AV_WINDOW,                 BCHP_XPT_PCROFFSET_STC7_AV_WINDOW },
    { "XPT_FULL_PID_PARSER_PARSER_CFG",          BCHP_XPT_FULL_PID_PARSER_PARSER_CFG,               BCHP_XPT_FULL_PID_PARSER_PBP_ACCEPT_ADAPT_00 },
    { "XPT_FULL_PID_PARSER_IBP_PCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_PCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_PCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_PCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_PCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_PCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_IBP_SCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_SCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_SCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_SCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_SCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_SCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_IBP_PSG_PROTOCOL_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_PSG_PROTOCOL_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_PSG_PROTOCOL_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_PSG_PROTOCOL_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_PSG_PROTOCOL_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_PSG_PROTOCOL_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_IBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PSG_ERR_MODE",        BCHP_XPT_FULL_PID_PARSER_PSG_ERR_MODE,             BCHP_XPT_FULL_PID_PARSER_PSG_ERR_MODE },
    { "XPT_RSBUFF_BO_IBP0",                      BCHP_XPT_RSBUFF_BO_IBP0,                           BCHP_XPT_RSBUFF_BO_IBP15 },
    { "XPT_RSBUFF_BO_PBP0",                      BCHP_XPT_RSBUFF_BO_PBP0,                           BCHP_XPT_RSBUFF_BO_PBP21 },
    { "XPT_RSBUFF_BO_MPOD_IBP0",                 BCHP_XPT_RSBUFF_BO_MPOD_IBP0,                      BCHP_XPT_RSBUFF_BO_MPOD_IBP15 },
    { "XPT_RSBUFF_IBP_BUFFER_ENABLE",            BCHP_XPT_RSBUFF_IBP_BUFFER_ENABLE,                 BCHP_XPT_RSBUFF_PBP_BUFFER_ENABLE },
    { "XPT_RSBUFF_MPOD_IBP_BUFFER_ENABLE",       BCHP_XPT_RSBUFF_MPOD_IBP_BUFFER_ENABLE,            BCHP_XPT_RSBUFF_MPOD_IBP_BUFFER_ENABLE },
    { "XPT_RSBUFF_OVERFLOW_THRESHOLD",           BCHP_XPT_RSBUFF_OVERFLOW_THRESHOLD,                BCHP_XPT_RSBUFF_NO_RD_HANG_CTRL },
    { "XPT_RSBUFF_PR_FALLBACK_CTRL",             BCHP_XPT_RSBUFF_PR_FALLBACK_CTRL,                  BCHP_XPT_RSBUFF_PR_FALLBACK_CTRL },
    { "XPT_XCBUFF_RAVE_CTRL_BUFFER_EN_PBP",      BCHP_XPT_XCBUFF_RAVE_CTRL_BUFFER_EN_PBP,           BCHP_XPT_XCBUFF_PAUSE_THRESHOLD },
    { "XPT_XCBUFF_OVERFLOW_THRESHOLD",           BCHP_XPT_XCBUFF_OVERFLOW_THRESHOLD,                BCHP_XPT_XCBUFF_OVERFLOW_THRESHOLD },
    { "XPT_XCBUFF_BYTE_SWAP_CTRL",               BCHP_XPT_XCBUFF_BYTE_SWAP_CTRL,                    BCHP_XPT_XCBUFF_BO_RAVE_IBP15 },
    { "XPT_XCBUFF_BO_RAVE_PBP0",                 BCHP_XPT_XCBUFF_BO_RAVE_PBP0,                      BCHP_XPT_XCBUFF_BO_RAVE_PBP21 },
    { "XPT_XCBUFF_BO_MSG_IBP0",                  BCHP_XPT_XCBUFF_BO_MSG_IBP0,                       BCHP_XPT_XCBUFF_BO_MSG_IBP15 },
    { "XPT_XCBUFF_BO_MSG_PBP0",                  BCHP_XPT_XCBUFF_BO_MSG_PBP0,                       BCHP_XPT_XCBUFF_BO_MSG_PBP21 },
    { "XPT_XCBUFF_BO_RMX0_IBP0",                 BCHP_XPT_XCBUFF_BO_RMX0_IBP0,                      BCHP_XPT_XCBUFF_BO_RMX0_IBP15 },
    { "XPT_XCBUFF_BO_RMX0_PBP0",                 BCHP_XPT_XCBUFF_BO_RMX0_PBP0,                      BCHP_XPT_XCBUFF_BO_RMX0_PBP21 },
    { "XPT_XCBUFF_BO_RMX1_IBP0",                 BCHP_XPT_XCBUFF_BO_RMX1_IBP0,                      BCHP_XPT_XCBUFF_BO_RMX1_IBP15 },
    { "XPT_XCBUFF_BO_RMX1_PBP0",                 BCHP_XPT_XCBUFF_BO_RMX1_PBP0,                      BCHP_XPT_XCBUFF_BO_RMX1_PBP21 },
    { "XPT_XCBUFF_MISC_CTRL",                    BCHP_XPT_XCBUFF_MISC_CTRL,                         BCHP_XPT_XCBUFF_NO_RD_HANG_CTRL },
    { "XPT_XCBUFF_PR_FALLBACK_CTRL",             BCHP_XPT_XCBUFF_PR_FALLBACK_CTRL,                  BCHP_XPT_XCBUFF_PR_FALLBACK_CTRL },
    { "XPT_MSG_MSG_CTRL1",                       BCHP_XPT_MSG_MSG_CTRL1,                            BCHP_XPT_MSG_DMA_BUFFER_INIT },
    { "XPT_MSG_PHY_ADDR_LO",                     BCHP_XPT_MSG_PHY_ADDR_LO,                          BCHP_XPT_MSG_ID_REJECT },
    { "XPT_MSG_C_MATCH0",                        BCHP_XPT_MSG_C_MATCH0,                             BCHP_XPT_MSG_C_MATCH4 },
    { "XPT_MSG_BUF_DAT_RDY_INTR_EN_00_31",       BCHP_XPT_MSG_BUF_DAT_RDY_INTR_EN_00_31,            BCHP_XPT_MSG_BUF_DAT_RDY_INTR_EN_96_127 },
    { "XPT_MSG_BUF_OVFL_INTR_EN_00_31",          BCHP_XPT_MSG_BUF_OVFL_INTR_EN_00_31,               BCHP_XPT_MSG_BUF_OVFL_INTR_EN_96_127 },
    { "XPT_MSG_DAT_ERR_INTR_EN",                 BCHP_XPT_MSG_DAT_ERR_INTR_EN,                      BCHP_XPT_MSG_DAT_ERR_INTR_EN },
    { "XPT_MSG_MSG_EVENT_CNT_CTRL",              BCHP_XPT_MSG_MSG_EVENT_CNT_CTRL,                   BCHP_XPT_MSG_MSG_EVENT_CNT_CTRL },
    { "XPT_RAVE_AVS_SCV_FILTER_MODE_CONTROL",    BCHP_XPT_RAVE_AVS_SCV_FILTER_MODE_CONTROL,         BCHP_XPT_RAVE_AVS_SCV_FILTER_VALUE_4_TO_7 },
#if (BCHP_VER == BCHP_VER_A0)
    { "XPT_RAVE_DATA_START_ADDR_A",              BCHP_XPT_RAVE_DATA_START_ADDR_A,                   BCHP_XPT_RAVE_WATCHDOG_TIMER_VALUE },
    { "XPT_RAVE_MISC_CONTROL",                   BCHP_XPT_RAVE_MISC_CONTROL,                        BCHP_XPT_RAVE_BASE_ADDRESSES },
    { "XPT_RAVE_MISC_CONTROL2",                  BCHP_XPT_RAVE_MISC_CONTROL2,                       BCHP_XPT_RAVE_MISC_CONTROL3 },
    { "XPT_RAVE_RC0_SP_CONTROL",                 BCHP_XPT_RAVE_RC0_SP_CONTROL,                      BCHP_XPT_RAVE_RC8_SP_CONTROL },
    { "XPT_RAVE_ATSOFFSET_MAX_ERROR",            BCHP_XPT_RAVE_ATSOFFSET_MAX_ERROR,                 BCHP_XPT_RAVE_ATSOFFSET_MAX_ERROR },
    { "XPT_RAVE_TPIT_TIME_TICK",                 BCHP_XPT_RAVE_TPIT_TIME_TICK,                      BCHP_XPT_RAVE_TPIT_EVE_TIMEOUT },
    { "XPT_RAVE_EMM_TID_MODE",                   BCHP_XPT_RAVE_EMM_TID_MODE,                        BCHP_XPT_RAVE_EMM_CTRL_ID },
    { "XPT_RAVE_EMM_DATA_ID_1",                  BCHP_XPT_RAVE_EMM_DATA_ID_1,                       BCHP_XPT_RAVE_EMM_MASK_ID_8 },
#else
    { "XPT_RAVE_DATA_START_ADDR_A",              BCHP_XPT_RAVE_DATA_START_ADDR_A,                   BCHP_XPT_RAVE_BASE_ADDRESSES },
    { "XPT_RAVE_MISC_CONTROL2",                  BCHP_XPT_RAVE_MISC_CONTROL2,                       BCHP_XPT_RAVE_RC8_SP_CONTROL },
    { "XPT_RAVE_ATSOFFSET_MAX_ERROR",            BCHP_XPT_RAVE_ATSOFFSET_MAX_ERROR,                 BCHP_XPT_RAVE_ATSOFFSET_MAX_ERROR },
    { "XPT_RAVE_TPIT_TIME_TICK",                 BCHP_XPT_RAVE_TPIT_TIME_TICK,                      BCHP_XPT_RAVE_EMM_MASK_ID_8 },
#endif
    {0, 0, 0}
};

#elif ((BCHP_CHIP==7552 && BCHP_VER>=BCHP_VER_B0) || (BCHP_CHIP==7360) || (BCHP_CHIP==7563) || (BCHP_CHIP==7362) || (BCHP_CHIP==7228) || (BCHP_CHIP==75635) || (BCHP_CHIP == 73625) || (BCHP_CHIP == 75525))

#include "bchp_xpt_bus_if.h"
#include "bchp_xpt_dpcr0.h"
#include "bchp_xpt_dpcr1.h"
#include "bchp_xpt_dpcr2.h"
#include "bchp_xpt_dpcr3.h"
#include "bchp_xpt_dpcr_pp.h"
#include "bchp_xpt_fe.h"
#include "bchp_xpt_full_pid_parser.h"
#include "bchp_xpt_gr.h"
#include "bchp_xpt_mpod.h"
#include "bchp_xpt_msg.h"
#include "bchp_xpt_pb0.h"
#include "bchp_xpt_pb1.h"
#include "bchp_xpt_pb2.h"
#include "bchp_xpt_pb3.h"
#include "bchp_xpt_pcroffset.h"
#include "bchp_xpt_pmu.h"
#include "bchp_xpt_psub.h"
#include "bchp_xpt_rave.h"
#include "bchp_xpt_rmx0.h"
#include "bchp_xpt_rmx0_io.h"
#include "bchp_xpt_rmx1.h"
#include "bchp_xpt_rmx1_io.h"
#include "bchp_xpt_rsbuff.h"
#include "bchp_xpt_secure_bus_if.h"
#include "bchp_xpt_wakeup.h"
#include "bchp_xpt_xcbuff.h"
#include "bchp_xpt_xmemif.h"
#include "bchp_xpt_xpu.h"

const BXPT_P_RegisterRange XPT_SRAM_LIST[] =
{
    { "WAKEUP_MEM",                              BCHP_XPT_WAKEUP_PKT_MEM_i_ARRAY_BASE,              (BCHP_XPT_WAKEUP_PKT_TYPE3_i_ARRAY_BASE+(BCHP_XPT_WAKEUP_PKT_TYPE3_i_ARRAY_END*4)) },
    { "FE_PID_TABLE",                            BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE,                (BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE+(BCHP_XPT_FE_PID_TABLE_i_ARRAY_END*4)) },
    { "FE_SPID_TABLE",                           BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE,               (BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE+(BCHP_XPT_FE_SPID_TABLE_i_ARRAY_END*4)) },
    { "RSBUFF_IBP_PMEM",                         BCHP_XPT_RSBUFF_BASE_POINTER_IBP0,                 BCHP_XPT_RSBUFF_WATERMARK_IBP15 },
    { "RSBUFF_PBP_PMEM",                         BCHP_XPT_RSBUFF_BASE_POINTER_PBP0,                 BCHP_XPT_RSBUFF_WATERMARK_PBP3 },
    { "XCBUFF_RAVE_IBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RAVE_IBP0,            BCHP_XPT_XCBUFF_WATERMARK_RAVE_IBP15 },
    { "XCBUFF_RAVE_PBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RAVE_PBP0,            BCHP_XPT_XCBUFF_WATERMARK_RAVE_PBP3 },
    { "XCBUFF_MSG_IBP_PMEM",                     BCHP_XPT_XCBUFF_BASE_POINTER_MSG_IBP0,             BCHP_XPT_XCBUFF_WATERMARK_MSG_IBP15 },
    { "XCBUFF_MSG_PBP_PMEM",                     BCHP_XPT_XCBUFF_BASE_POINTER_MSG_PBP0,             BCHP_XPT_XCBUFF_WATERMARK_MSG_PBP3 },
    { "XCBUFF_RMX0_IBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX0_IBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX0_IBP15 },
    { "XCBUFF_RMX0_PBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX0_PBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX0_PBP3 },
    { "XCBUFF_RMX1_IBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX1_IBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX1_IBP15 },
    { "XCBUFF_RMX1_PBP_PMEM",                    BCHP_XPT_XCBUFF_BASE_POINTER_RMX1_PBP0,            BCHP_XPT_XCBUFF_WATERMARK_RMX1_PBP3 },
    { "RAVE_PMEM",                               BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR,                BCHP_XPT_RAVE_SCD32_RESERVED_STATE3 },
    { "RAVE_CXMEM_LO",                           BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_BASE,                (BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_BASE+(BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_END*4)) },
    { "RAVE_CXMEM_HI",                           BCHP_XPT_RAVE_CXMEM_HIi_ARRAY_BASE,                (BCHP_XPT_RAVE_CXMEM_HIi_ARRAY_BASE+(BCHP_XPT_RAVE_CXMEM_HIi_ARRAY_END*4)) },
    { "RAVE_DMEM",                               BCHP_XPT_RAVE_DMEMi_ARRAY_BASE,                    (BCHP_XPT_RAVE_DMEMi_ARRAY_BASE+(BCHP_XPT_RAVE_DMEMi_ARRAY_END*4)) },
    { "RAVE_SMEM",                               BCHP_XPT_RAVE_TPIT0_PID_TABLEi_ARRAY_BASE,         BCHP_XPT_RAVE_TPIT_STATE_CONTEXT23 },
    { "PCROFFSET_PID_TABLE",                     BCHP_XPT_PCROFFSET_PID_CONFIG_TABLE_i_ARRAY_BASE,  (BCHP_XPT_PCROFFSET_PID_CONFIG_TABLE_i_ARRAY_BASE+(BCHP_XPT_PCROFFSET_PID_CONFIG_TABLE_i_ARRAY_END*4)) },
    { "PCROFFSET_CX_TABLE",                      BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL,        BCHP_XPT_PCROFFSET_CONTEXT15_RESERVED_CFG_4 },
    { "MSG_PMEM",                                BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_ARRAY_BASE,         (BCHP_XPT_MSG_DMA_VP_TABLE_i_ARRAY_BASE+(BCHP_XPT_MSG_DMA_VP_TABLE_i_ARRAY_END*4)) },
    { "MSG_GEN_FILT1",                           BCHP_XPT_MSG_GEN_FILT_EN_i_ARRAY_BASE,             (BCHP_XPT_MSG_GEN_FILT_EN_i_ARRAY_BASE+(BCHP_XPT_MSG_GEN_FILT_EN_i_ARRAY_END*4)) },
    { "MSG_GEN_FILT2",                           BCHP_XPT_MSG_MCAST_16_ADDR_i_ARRAY_BASE,           (BCHP_XPT_MSG_GEN_FILT_EXCL_i_ARRAY_BASE+(BCHP_XPT_MSG_GEN_FILT_EXCL_i_ARRAY_END*4)) },
    { "MSG_PID2BUF",                             BCHP_XPT_MSG_PID2BUF_MAP_i_ARRAY_BASE,             (BCHP_XPT_MSG_PID2BUF_MAP_i_ARRAY_BASE+(BCHP_XPT_MSG_PID2BUF_MAP_i_ARRAY_END*4)) },
    { "FPP_SCM0",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_END*4)) },
    { "FPP_SCM1",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_1_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_1_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_1_i_ARRAY_END*4)) },
    { "FPP_SCM2",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_2_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_2_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_2_i_ARRAY_END*4)) },
    { "FPP_SCM3",                                BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_3_i_ARRAY_BASE, (BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_3_i_ARRAY_BASE+(BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_3_i_ARRAY_END*4)) },
    {0, 0, 0}
};

const BXPT_P_RegisterRange XPT_REG_SAVE_LIST[] =
{
    { "XPT_BUS_IF_MISC_CTRL0",                   BCHP_XPT_BUS_IF_MISC_CTRL0,                        BCHP_XPT_BUS_IF_TEST_MODE },
    { "XPT_BUS_IF_INTR_STATUS_REG_EN",           BCHP_XPT_BUS_IF_INTR_STATUS_REG_EN,                BCHP_XPT_BUS_IF_INTR_STATUS_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS2_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS2_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS2_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS3_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS3_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS3_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS4_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS4_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS4_REG_EN },
    { "XPT_BUS_IF_INTR_STATUS5_REG_EN",          BCHP_XPT_BUS_IF_INTR_STATUS5_REG_EN,               BCHP_XPT_BUS_IF_INTR_STATUS5_REG_EN },
    { "XPT_BUS_IF_XPKT_FORMAT_CONV_RS2SEC",      BCHP_XPT_BUS_IF_XPKT_FORMAT_CONV_RS2SEC,           BCHP_XPT_BUS_IF_XPKT_FORMAT_CONV_SEC2FPP },
    { "XPT_XMEMIF_SCB_WR_ARB_SEL_RSBUFF",        BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RSBUFF,             BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB3 },
    { "XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB",          BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB,               BCHP_XPT_XMEMIF_SCB_RD_ARB_MODE },
    { "XPT_XMEMIF_INTR_STATUS_REG_EN",           BCHP_XPT_XMEMIF_INTR_STATUS_REG_EN,                BCHP_XPT_XMEMIF_INTR_STATUS_REG_EN },
    { "XPT_PMU_CLK_CTRL",                        BCHP_XPT_PMU_CLK_CTRL,                             BCHP_XPT_PMU_RBUS_RSP_VAL },
    { "XPT_WAKEUP_CTRL",                         BCHP_XPT_WAKEUP_CTRL,                              BCHP_XPT_WAKEUP_CTRL },
    { "XPT_WAKEUP_INTR_STATUS_REG_EN",           BCHP_XPT_WAKEUP_INTR_STATUS_REG_EN,                BCHP_XPT_WAKEUP_INTR_STATUS_REG_EN },
    { "XPT_RMX0_IO_FORMAT",                      BCHP_XPT_RMX0_IO_FORMAT,                           BCHP_XPT_RMX0_IO_PKT_DLY_CNT },
    { "XPT_RMX1_IO_FORMAT",                      BCHP_XPT_RMX1_IO_FORMAT,                           BCHP_XPT_RMX1_IO_PKT_DLY_CNT },
    { "XPT_RMX1_IO_TV_STATUS",                   BCHP_XPT_RMX1_IO_TV_STATUS,                        BCHP_XPT_RMX1_IO_TV_STATUS },
    { "XPT_FE_FE_CTRL",                          BCHP_XPT_FE_FE_CTRL,                               BCHP_XPT_FE_FE_CTRL },
    { "XPT_FE_INTR_STATUS0_REG_EN",              BCHP_XPT_FE_INTR_STATUS0_REG_EN,                   BCHP_XPT_FE_PWR_CTRL },
    { "XPT_FE_MAX_PID_CHANNEL",                  BCHP_XPT_FE_MAX_PID_CHANNEL,                       BCHP_XPT_FE_MAX_PID_CHANNEL },
    { "XPT_FE_IB0_CTRL",                         BCHP_XPT_FE_IB0_CTRL,                              BCHP_XPT_FE_IB0_CTRL },
    { "XPT_FE_IB1_CTRL",                         BCHP_XPT_FE_IB1_CTRL,                              BCHP_XPT_FE_IB1_CTRL },
    { "XPT_FE_IB2_CTRL",                         BCHP_XPT_FE_IB2_CTRL,                              BCHP_XPT_FE_IB2_CTRL },
    { "XPT_FE_IB3_CTRL",                         BCHP_XPT_FE_IB3_CTRL,                              BCHP_XPT_FE_IB3_CTRL },
    { "XPT_FE_IB4_CTRL",                         BCHP_XPT_FE_IB4_CTRL,                              BCHP_XPT_FE_IB4_CTRL },
    { "XPT_FE_IB5_CTRL",                         BCHP_XPT_FE_IB5_CTRL,                              BCHP_XPT_FE_IB5_CTRL },
    { "XPT_FE_IB6_CTRL",                         BCHP_XPT_FE_IB6_CTRL,                              BCHP_XPT_FE_IB6_CTRL },
    { "XPT_FE_IB7_CTRL",                         BCHP_XPT_FE_IB7_CTRL,                              BCHP_XPT_FE_IB7_CTRL },
    { "XPT_FE_IB8_CTRL",                         BCHP_XPT_FE_IB8_CTRL,                              BCHP_XPT_FE_IB8_CTRL },
    { "XPT_FE_IB9_CTRL",                         BCHP_XPT_FE_IB9_CTRL,                              BCHP_XPT_FE_IB9_CTRL },
    { "XPT_FE_MINI_PID_PARSER0_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER0_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER0_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER1_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER1_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER1_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER2_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER2_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER2_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER3_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER3_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER3_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER4_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER4_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER4_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER5_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER5_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER5_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER6_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER6_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER6_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER7_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER7_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER7_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER8_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER8_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER8_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER9_CTRL1",           BCHP_XPT_FE_MINI_PID_PARSER9_CTRL1,                BCHP_XPT_FE_MINI_PID_PARSER9_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER10_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER10_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER10_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER11_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER11_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER11_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER12_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER12_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER12_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER13_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER13_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER13_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER14_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER14_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER14_CTRL2 },
    { "XPT_FE_MINI_PID_PARSER15_CTRL1",          BCHP_XPT_FE_MINI_PID_PARSER15_CTRL1,               BCHP_XPT_FE_MINI_PID_PARSER15_CTRL2 },
    { "XPT_FE_IB_SYNC_DETECT_CTRL",              BCHP_XPT_FE_IB_SYNC_DETECT_CTRL,                   BCHP_XPT_FE_IB_SYNC_DETECT_CTRL },
    { "XPT_FE_TSMF0_CTRL",                       BCHP_XPT_FE_TSMF0_CTRL,                            BCHP_XPT_FE_TSMF0_SLOT_MAP_HI },
    { "XPT_FE_TSMF1_CTRL",                       BCHP_XPT_FE_TSMF1_CTRL,                            BCHP_XPT_FE_TSMF1_SLOT_MAP_HI },
    { "XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID", BCHP_XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID,   BCHP_XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID },
    { "XPT_FE_ATS_COUNTER_CTRL",                 BCHP_XPT_FE_ATS_COUNTER_CTRL,                      BCHP_XPT_FE_ATS_TS_BINARY },
    { "XPT_FE_MTSIF_RX0_CTRL1",                  BCHP_XPT_FE_MTSIF_RX0_CTRL1,                       BCHP_XPT_FE_MTSIF_RX0_CTRL1 },
    { "XPT_FE_MTSIF_RX0_INTR_STATUS0_REG_EN",    BCHP_XPT_FE_MTSIF_RX0_INTR_STATUS0_REG_EN,         BCHP_XPT_FE_MTSIF_RX0_BAND_ID_DROP },
    { "XPT_FE_MTSIF_RX1_CTRL1",                  BCHP_XPT_FE_MTSIF_RX1_CTRL1,                       BCHP_XPT_FE_MTSIF_RX1_CTRL1 },
    { "XPT_FE_MTSIF_RX1_INTR_STATUS0_REG_EN",    BCHP_XPT_FE_MTSIF_RX1_INTR_STATUS0_REG_EN,         BCHP_XPT_FE_MTSIF_RX1_BAND_ID_DROP },
    { "XPT_DPCR0_PID_CH",                        BCHP_XPT_DPCR0_PID_CH,                             BCHP_XPT_DPCR0_CTRL },
    { "XPT_DPCR0_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR0_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR0_STC_EXT_CTRL },
    { "XPT_DPCR0_MAX_PCR_ERROR",                 BCHP_XPT_DPCR0_MAX_PCR_ERROR,                      BCHP_XPT_DPCR0_MAX_PCR_ERROR },
    { "XPT_DPCR0_STC_HI",                        BCHP_XPT_DPCR0_STC_HI,                             BCHP_XPT_DPCR0_STC_LO },
    { "XPT_DPCR0_LOOP_CTRL",                     BCHP_XPT_DPCR0_LOOP_CTRL,                          BCHP_XPT_DPCR0_ACCUM_VALUE },
    { "XPT_DPCR0_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR0_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR0_PHASE_ERROR_CLAMP },
    { "XPT_DPCR1_PID_CH",                        BCHP_XPT_DPCR1_PID_CH,                             BCHP_XPT_DPCR1_CTRL },
    { "XPT_DPCR1_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR1_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR1_STC_EXT_CTRL },
    { "XPT_DPCR1_MAX_PCR_ERROR",                 BCHP_XPT_DPCR1_MAX_PCR_ERROR,                      BCHP_XPT_DPCR1_MAX_PCR_ERROR },
    { "XPT_DPCR1_STC_HI",                        BCHP_XPT_DPCR1_STC_HI,                             BCHP_XPT_DPCR1_STC_LO },
    { "XPT_DPCR1_LOOP_CTRL",                     BCHP_XPT_DPCR1_LOOP_CTRL,                          BCHP_XPT_DPCR1_ACCUM_VALUE },
    { "XPT_DPCR1_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR1_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR1_PHASE_ERROR_CLAMP },
    { "XPT_DPCR2_PID_CH",                        BCHP_XPT_DPCR2_PID_CH,                             BCHP_XPT_DPCR2_CTRL },
    { "XPT_DPCR2_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR2_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR2_STC_EXT_CTRL },
    { "XPT_DPCR2_MAX_PCR_ERROR",                 BCHP_XPT_DPCR2_MAX_PCR_ERROR,                      BCHP_XPT_DPCR2_MAX_PCR_ERROR },
    { "XPT_DPCR2_STC_HI",                        BCHP_XPT_DPCR2_STC_HI,                             BCHP_XPT_DPCR2_STC_LO },
    { "XPT_DPCR2_LOOP_CTRL",                     BCHP_XPT_DPCR2_LOOP_CTRL,                          BCHP_XPT_DPCR2_ACCUM_VALUE },
    { "XPT_DPCR2_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR2_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR2_PHASE_ERROR_CLAMP },
    { "XPT_DPCR3_PID_CH",                        BCHP_XPT_DPCR3_PID_CH,                             BCHP_XPT_DPCR3_CTRL },
    { "XPT_DPCR3_INTR_STATUS_REG_EN",            BCHP_XPT_DPCR3_INTR_STATUS_REG_EN,                 BCHP_XPT_DPCR3_STC_EXT_CTRL },
    { "XPT_DPCR3_MAX_PCR_ERROR",                 BCHP_XPT_DPCR3_MAX_PCR_ERROR,                      BCHP_XPT_DPCR3_MAX_PCR_ERROR },
    { "XPT_DPCR3_STC_HI",                        BCHP_XPT_DPCR3_STC_HI,                             BCHP_XPT_DPCR3_STC_LO },
    { "XPT_DPCR3_LOOP_CTRL",                     BCHP_XPT_DPCR3_LOOP_CTRL,                          BCHP_XPT_DPCR3_ACCUM_VALUE },
    { "XPT_DPCR3_SOFT_PCR_CTRL",                 BCHP_XPT_DPCR3_SOFT_PCR_CTRL,                      BCHP_XPT_DPCR3_PHASE_ERROR_CLAMP },
    { "XPT_DPCR_PP_PP_CTRL",                     BCHP_XPT_DPCR_PP_PP_CTRL,                          BCHP_XPT_DPCR_PP_PP_FIXED_OFFSET },
    { "XPT_PSUB_PSUB0_CTRL0",                    BCHP_XPT_PSUB_PSUB0_CTRL0,                         BCHP_XPT_PSUB_PSUB0_STAT0 },
    { "XPT_PSUB_PSUB1_CTRL0",                    BCHP_XPT_PSUB_PSUB1_CTRL0,                         BCHP_XPT_PSUB_PSUB1_STAT0 },
    { "XPT_PSUB_PSUB2_CTRL0",                    BCHP_XPT_PSUB_PSUB2_CTRL0,                         BCHP_XPT_PSUB_PSUB2_STAT0 },
    { "XPT_PSUB_PSUB3_CTRL0",                    BCHP_XPT_PSUB_PSUB3_CTRL0,                         BCHP_XPT_PSUB_PSUB3_STAT0 },
    { "XPT_PSUB_PSUB4_CTRL0",                    BCHP_XPT_PSUB_PSUB4_CTRL0,                         BCHP_XPT_PSUB_PSUB4_STAT0 },
    { "XPT_PSUB_PSUB5_CTRL0",                    BCHP_XPT_PSUB_PSUB5_CTRL0,                         BCHP_XPT_PSUB_PSUB5_STAT0 },
    { "XPT_PSUB_PSUB6_CTRL0",                    BCHP_XPT_PSUB_PSUB6_CTRL0,                         BCHP_XPT_PSUB_PSUB6_STAT0 },
    { "XPT_PSUB_PSUB7_CTRL0",                    BCHP_XPT_PSUB_PSUB7_CTRL0,                         BCHP_XPT_PSUB_PSUB7_STAT0 },
    { "XPT_PSUB_PR_FALLBACK_CTRL",               BCHP_XPT_PSUB_PR_FALLBACK_CTRL,                    BCHP_XPT_PSUB_PR_FALLBACK_CTRL },
    { "XPT_RSBUFF_BO_IBP0",                      BCHP_XPT_RSBUFF_BO_IBP0,                           BCHP_XPT_RSBUFF_BO_IBP15 },
    { "XPT_RSBUFF_BO_PBP0",                      BCHP_XPT_RSBUFF_BO_PBP0,                           BCHP_XPT_RSBUFF_BO_PBP3 },
    { "XPT_RSBUFF_IBP_BUFFER_ENABLE",            BCHP_XPT_RSBUFF_IBP_BUFFER_ENABLE,                 BCHP_XPT_RSBUFF_PBP_BUFFER_ENABLE },
    { "XPT_RSBUFF_OVERFLOW_THRESHOLD",           BCHP_XPT_RSBUFF_OVERFLOW_THRESHOLD,                BCHP_XPT_RSBUFF_NO_RD_HANG_CTRL },
    { "XPT_RSBUFF_PR_FALLBACK_CTRL",             BCHP_XPT_RSBUFF_PR_FALLBACK_CTRL,                  BCHP_XPT_RSBUFF_PR_FALLBACK_CTRL },
    { "XPT_PB0_CTRL1",                           BCHP_XPT_PB0_CTRL1,                                BCHP_XPT_PB0_FIRST_DESC_ADDR },
    { "XPT_PB0_BLOCKOUT",                        BCHP_XPT_PB0_BLOCKOUT,                             BCHP_XPT_PB0_PARSER_CTRL2 },
    { "XPT_PB0_INTR_EN",                         BCHP_XPT_PB0_INTR_EN,                              BCHP_XPT_PB0_PWR_CTRL },
    { "XPT_PB0_ASF_CTRL",                        BCHP_XPT_PB0_ASF_CTRL,                             BCHP_XPT_PB0_PLAYBACK_PARSER_BAND_ID },
    { "XPT_PB1_CTRL1",                           BCHP_XPT_PB1_CTRL1,                                BCHP_XPT_PB1_FIRST_DESC_ADDR },
    { "XPT_PB1_BLOCKOUT",                        BCHP_XPT_PB1_BLOCKOUT,                             BCHP_XPT_PB1_PARSER_CTRL2 },
    { "XPT_PB1_INTR_EN",                         BCHP_XPT_PB1_INTR_EN,                              BCHP_XPT_PB1_PWR_CTRL },
    { "XPT_PB1_ASF_CTRL",                        BCHP_XPT_PB1_ASF_CTRL,                             BCHP_XPT_PB1_PLAYBACK_PARSER_BAND_ID },
    { "XPT_PB2_CTRL1",                           BCHP_XPT_PB2_CTRL1,                                BCHP_XPT_PB2_FIRST_DESC_ADDR },
    { "XPT_PB2_BLOCKOUT",                        BCHP_XPT_PB2_BLOCKOUT,                             BCHP_XPT_PB2_PARSER_CTRL2 },
    { "XPT_PB2_INTR_EN",                         BCHP_XPT_PB2_INTR_EN,                              BCHP_XPT_PB2_PWR_CTRL },
    { "XPT_PB2_ASF_CTRL",                        BCHP_XPT_PB2_ASF_CTRL,                             BCHP_XPT_PB2_PLAYBACK_PARSER_BAND_ID },
    { "XPT_PB3_CTRL1",                           BCHP_XPT_PB3_CTRL1,                                BCHP_XPT_PB3_FIRST_DESC_ADDR },
    { "XPT_PB3_BLOCKOUT",                        BCHP_XPT_PB3_BLOCKOUT,                             BCHP_XPT_PB3_PARSER_CTRL2 },
    { "XPT_PB3_INTR_EN",                         BCHP_XPT_PB3_INTR_EN,                              BCHP_XPT_PB3_PWR_CTRL },
    { "XPT_PB3_ASF_CTRL",                        BCHP_XPT_PB3_ASF_CTRL,                             BCHP_XPT_PB3_PLAYBACK_PARSER_BAND_ID },
    { "XPT_MPOD_CFG",                            BCHP_XPT_MPOD_CFG,                                 BCHP_XPT_MPOD_ICTRL },
    { "XPT_MPOD_OCTRL2",                         BCHP_XPT_MPOD_OCTRL2,                              BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP },
    { "XPT_RMX0_CTRL",                           BCHP_XPT_RMX0_CTRL,                                BCHP_XPT_RMX0_FIXED_OFFSET },
    { "XPT_RMX1_CTRL",                           BCHP_XPT_RMX1_CTRL,                                BCHP_XPT_RMX1_FIXED_OFFSET },
    { "XPT_XCBUFF_RAVE_CTRL_BUFFER_EN_PBP",      BCHP_XPT_XCBUFF_RAVE_CTRL_BUFFER_EN_PBP,           BCHP_XPT_XCBUFF_PAUSE_THRESHOLD },
    { "XPT_XCBUFF_OVERFLOW_THRESHOLD",           BCHP_XPT_XCBUFF_OVERFLOW_THRESHOLD,                BCHP_XPT_XCBUFF_OVERFLOW_THRESHOLD },
    { "XPT_XCBUFF_BYTE_SWAP_CTRL",               BCHP_XPT_XCBUFF_BYTE_SWAP_CTRL,                    BCHP_XPT_XCBUFF_BO_RAVE_IBP15 },
    { "XPT_XCBUFF_BO_RAVE_PBP0",                 BCHP_XPT_XCBUFF_BO_RAVE_PBP0,                      BCHP_XPT_XCBUFF_BO_RAVE_PBP3 },
    { "XPT_XCBUFF_BO_MSG_IBP0",                  BCHP_XPT_XCBUFF_BO_MSG_IBP0,                       BCHP_XPT_XCBUFF_BO_MSG_IBP15 },
    { "XPT_XCBUFF_BO_MSG_PBP0",                  BCHP_XPT_XCBUFF_BO_MSG_PBP0,                       BCHP_XPT_XCBUFF_BO_MSG_PBP3 },
    { "XPT_XCBUFF_BO_RMX0_IBP0",                 BCHP_XPT_XCBUFF_BO_RMX0_IBP0,                      BCHP_XPT_XCBUFF_BO_RMX0_IBP15 },
    { "XPT_XCBUFF_BO_RMX0_PBP0",                 BCHP_XPT_XCBUFF_BO_RMX0_PBP0,                      BCHP_XPT_XCBUFF_BO_RMX0_PBP3 },
    { "XPT_XCBUFF_BO_RMX1_IBP0",                 BCHP_XPT_XCBUFF_BO_RMX1_IBP0,                      BCHP_XPT_XCBUFF_BO_RMX1_IBP15 },
    { "XPT_XCBUFF_BO_RMX1_PBP0",                 BCHP_XPT_XCBUFF_BO_RMX1_PBP0,                      BCHP_XPT_XCBUFF_BO_RMX1_PBP3 },
    { "XPT_XCBUFF_NO_RD_HANG_CTRL",              BCHP_XPT_XCBUFF_NO_RD_HANG_CTRL,                   BCHP_XPT_XCBUFF_NO_RD_HANG_CTRL },
    { "XPT_XCBUFF_PR_FALLBACK_CTRL",             BCHP_XPT_XCBUFF_PR_FALLBACK_CTRL,                  BCHP_XPT_XCBUFF_PR_FALLBACK_CTRL },
    { "XPT_RAVE_AVS_SCV_FILTER_MODE_CONTROL",    BCHP_XPT_RAVE_AVS_SCV_FILTER_MODE_CONTROL,         BCHP_XPT_RAVE_AVS_SCV_FILTER_VALUE_4_TO_7 },
    { "XPT_RAVE_DATA_START_ADDR_A",              BCHP_XPT_RAVE_DATA_START_ADDR_A,                   BCHP_XPT_RAVE_WATCHDOG_TIMER_VALUE },
    { "XPT_RAVE_MISC_CONTROL",                   BCHP_XPT_RAVE_MISC_CONTROL,                        BCHP_XPT_RAVE_BASE_ADDRESSES },
    { "XPT_RAVE_MISC_CONTROL2",                  BCHP_XPT_RAVE_MISC_CONTROL2,                       BCHP_XPT_RAVE_MISC_CONTROL3 },
    { "XPT_RAVE_RC0_SP_CONTROL",                 BCHP_XPT_RAVE_RC0_SP_CONTROL,                      BCHP_XPT_RAVE_RC8_SP_CONTROL },
    { "XPT_RAVE_ATSOFFSET_MAX_ERROR",            BCHP_XPT_RAVE_ATSOFFSET_MAX_ERROR,                 BCHP_XPT_RAVE_ATSOFFSET_MAX_ERROR },
    { "XPT_RAVE_TPIT_TIME_TICK",                 BCHP_XPT_RAVE_TPIT_TIME_TICK,                      BCHP_XPT_RAVE_TPIT_EVE_TIMEOUT },
    { "XPT_RAVE_EMM_TID_MODE",                   BCHP_XPT_RAVE_EMM_TID_MODE,                        BCHP_XPT_RAVE_EMM_CTRL_ID },
    { "XPT_RAVE_EMM_DATA_ID_1",                  BCHP_XPT_RAVE_EMM_DATA_ID_1,                       BCHP_XPT_RAVE_EMM_MASK_ID_8 },
    { "XPT_PCROFFSET_INTERRUPT0_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT0_ENABLE,              BCHP_XPT_PCROFFSET_INTERRUPT0_ENABLE },
    { "XPT_PCROFFSET_INTERRUPT1_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT1_ENABLE,              BCHP_XPT_PCROFFSET_INTERRUPT1_ENABLE },
    { "XPT_PCROFFSET_INTERRUPT2_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT2_ENABLE,              BCHP_XPT_PCROFFSET_INTERRUPT2_ENABLE },
    { "XPT_PCROFFSET_INTERRUPT3_ENABLE",         BCHP_XPT_PCROFFSET_INTERRUPT3_ENABLE,              BCHP_XPT_PCROFFSET_STC3 },
    { "XPT_PCROFFSET_STC_BROADCAST_SEL",         BCHP_XPT_PCROFFSET_STC_BROADCAST_SEL,              BCHP_XPT_PCROFFSET_STC_BROADCAST_SEL },
    { "XPT_PCROFFSET_STC_INTERRUPT_ENABLE",      BCHP_XPT_PCROFFSET_STC_INTERRUPT_ENABLE,           BCHP_XPT_PCROFFSET_STC3_MATCH },
    { "XPT_MSG_MSG_CTRL1",                       BCHP_XPT_MSG_MSG_CTRL1,                            BCHP_XPT_MSG_DMA_BUFFER_INIT },
    { "XPT_MSG_PHY_ADDR_LO",                     BCHP_XPT_MSG_PHY_ADDR_LO,                          BCHP_XPT_MSG_ID_REJECT },
    { "XPT_MSG_C_MATCH0",                        BCHP_XPT_MSG_C_MATCH0,                             BCHP_XPT_MSG_C_MATCH4 },
    { "XPT_MSG_BUF_DAT_RDY_INTR_EN_00_31",       BCHP_XPT_MSG_BUF_DAT_RDY_INTR_EN_00_31,            BCHP_XPT_MSG_BUF_DAT_RDY_INTR_EN_96_127 },
    { "XPT_MSG_BUF_OVFL_INTR_EN_00_31",          BCHP_XPT_MSG_BUF_OVFL_INTR_EN_00_31,               BCHP_XPT_MSG_BUF_OVFL_INTR_EN_96_127 },
    { "XPT_MSG_DAT_ERR_INTR_EN",                 BCHP_XPT_MSG_DAT_ERR_INTR_EN,                      BCHP_XPT_MSG_DAT_ERR_INTR_EN },
    { "XPT_MSG_MSG_EVENT_CNT_CTRL",              BCHP_XPT_MSG_MSG_EVENT_CNT_CTRL,                   BCHP_XPT_MSG_MSG_EVENT_CNT_CTRL },
    { "XPT_GR_CTRL",                             BCHP_XPT_GR_CTRL,                                  BCHP_XPT_GR_CTRL },
    { "XPT_FULL_PID_PARSER_PARSER_CFG",          BCHP_XPT_FULL_PID_PARSER_PARSER_CFG,               BCHP_XPT_FULL_PID_PARSER_PBP_ACCEPT_ADAPT_00 },
    { "XPT_FULL_PID_PARSER_IBP_PCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_PCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_PCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_PCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_PCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_PCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_IBP_SCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_SCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_SCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_SCC_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_SCC_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_SCC_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_IBP_PSG_PROTOCOL_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_PSG_PROTOCOL_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_PSG_PROTOCOL_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_PSG_PROTOCOL_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_PSG_PROTOCOL_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_PSG_PROTOCOL_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_IBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_IBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_IBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN", BCHP_XPT_FULL_PID_PARSER_PBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN, BCHP_XPT_FULL_PID_PARSER_PBP_TRANSPORT_ERROR_INTR_STATUS_REG_EN },
    { "XPT_FULL_PID_PARSER_PSG_ERR_MODE",        BCHP_XPT_FULL_PID_PARSER_PSG_ERR_MODE,             BCHP_XPT_FULL_PID_PARSER_PSG_ERR_MODE },
    {0, 0, 0}
};
#else

/* 40nm Ax platforms */
const BXPT_P_RegisterRange XPT_SRAM_LIST[] = {{0, 0, 0}};
const BXPT_P_RegisterRange XPT_REG_SAVE_LIST[] = {{0, 0, 0}};

#endif
#endif /* BCHP_PWR_RESOURCE_XPT */

void BXPT_P_FreeBackup(struct BXPT_Backup *backup)
{
    if (backup->reg) {
        unsigned page;
        for (page=0;page<backup->numpages;page++) {
            if (backup->reg[page]) {
                BKNI_Free(backup->reg[page]);
            }
        }
        BKNI_Free(backup->reg);
    }
    BKNI_Memset(backup, 0, sizeof(*backup));
}

static BERR_Code BXPT_P_AllocBackup(struct BXPT_Backup *backup, unsigned numEntries)
{
    BERR_Code rc;
    unsigned page;

    /* assume we init with 0's. just in case alloc is called twice in a row, free anything. */
    BXPT_P_FreeBackup(backup);

    backup->pagesize = 4096 * 2; /* tradeoff of fragmentation vs. array overhead */
    numEntries *= sizeof(uint32_t); /* convert to bytes */
    backup->numpages = (numEntries + backup->pagesize - 1) / backup->pagesize;
    backup->reg = BKNI_Malloc(sizeof(void*)*backup->numpages);
    if (!backup->reg) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }
    for (page=0;page<backup->numpages;page++) {
        backup->reg[page] = BKNI_Malloc(backup->pagesize);
        if (!backup->reg[page]) {
            rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            goto error;
        }
    }
    return BERR_SUCCESS;

    error:
    BXPT_P_FreeBackup(backup);
    return rc;
}

static uint32_t *BXPT_P_BackupPointer(const struct BXPT_Backup *backup, unsigned index)
{
    unsigned sz = backup->pagesize / sizeof(uint32_t);
    return &backup->reg[index/sz][index%sz];
}

static unsigned BXPT_P_RegisterRangeCount(const BXPT_P_RegisterRange *pRanges)
{
    unsigned i = 0, total = 0;
    while (1) {
        uint32_t startAddr, endAddr;
        startAddr = pRanges[i].startAddress;
        endAddr = pRanges[i].endAddress;
        if (endAddr == (uint32_t)-1) {
            endAddr = startAddr;
        }
        if (pRanges[i].name==0 && startAddr==0 && endAddr==0) {
            break;
        }
        total += (endAddr - startAddr)/sizeof(uint32_t) + 1;
        i++;
    }
    return total;
}

BERR_Code BXPT_P_RegisterToMemory(BREG_Handle reg, struct BXPT_Backup *backup, const BXPT_P_RegisterRange *pRanges)
{
    unsigned i=0, j=0;
    BERR_Code rc;

    BDBG_ASSERT(reg);
    BDBG_ASSERT(backup);
    BDBG_ASSERT(pRanges);

    rc = BXPT_P_AllocBackup(backup, BXPT_P_RegisterRangeCount(pRanges));
    if (rc) return BERR_TRACE(rc);

    while (1) {
        uint32_t addr, startAddr, endAddr;
        startAddr = pRanges[i].startAddress;
        endAddr = pRanges[i].endAddress;
        if (endAddr == (uint32_t)-1) {
            endAddr = startAddr;
        }

        if (pRanges[i].name==0 && startAddr==0 && endAddr==0) {
            break;
        }

        addr = startAddr;
        BDBG_MSG(("range (0x%08x - 0x%08x) => mem %p: %s", startAddr, endAddr, BXPT_P_BackupPointer(backup, j), pRanges[i].name));

        while (addr <= endAddr) {
            uint32_t *mem = BXPT_P_BackupPointer(backup, j++);
            *mem = BREG_Read32(reg, addr);
            addr += 4;
        }
        i++;
    }
    BDBG_MSG(("transferred %u bytes", j*4));
    return BERR_SUCCESS;
}

void BXPT_P_MemoryToRegister(BREG_Handle reg, struct BXPT_Backup *backup, const BXPT_P_RegisterRange *pRanges)
{
    unsigned i=0, j=0;

    BDBG_ASSERT(reg);
    BDBG_ASSERT(backup);
    BDBG_ASSERT(pRanges);

    while (1) {
        uint32_t addr, startAddr, endAddr;
        startAddr = pRanges[i].startAddress;
        endAddr = pRanges[i].endAddress;
        if (endAddr == (uint32_t)-1) {
            endAddr = startAddr;
        }
        if (pRanges[i].name==0 && startAddr==0 && endAddr==0) {
            break;
        }

        addr = startAddr;
        BDBG_MSG(("mem %p => range (0x%08x - 0x%08x): %s", BXPT_P_BackupPointer(backup, j), startAddr, endAddr, pRanges[i].name));

        while (addr <= endAddr) {
            const uint32_t *mem = BXPT_P_BackupPointer(backup, j++);
            BREG_Write32(reg, addr, *mem);
            addr += 4;
        }
        i++;
    }
    BDBG_MSG(("transferred %u bytes", j*4));
    BXPT_P_FreeBackup(backup);
}

void BXPT_P_AcquireSubmodule(BXPT_Handle hXpt, BXPT_P_Submodule sub)
{
    BSTD_UNUSED(hXpt);
    BSTD_UNUSED(sub);
}

void BXPT_P_ReleaseSubmodule(BXPT_Handle hXpt, BXPT_P_Submodule sub)
{
    BSTD_UNUSED(hXpt);
    BSTD_UNUSED(sub);
}

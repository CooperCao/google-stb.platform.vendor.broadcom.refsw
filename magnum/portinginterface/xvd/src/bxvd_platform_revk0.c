/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description:
 *   See code
 *
 ***************************************************************************/

#include "bxvd_platform.h"
#include "bxvd_priv.h"
#include "bxvd.h"
#include "bxvd_intr.h"
#include "bxvd_reg.h"
#include "bafl.h"


/* ARC AUX register offsets */
#define BXVD_P_ARC_PC 0x18
#define BXVD_P_ARC_STATUS32 0x28

#if BXVD_P_SVD_GISB_ERR_WORKAROUND

/* SW7425-628: need for the work around for the bus error caused by register reads. */
#include "bchp_svd_rgr_0.h"

#endif

/* Define BXVD_POLL_FW_MBX to bring up decoder witout the use in interrupts */
/* #define BXVD_POLL_FW_MBX 1 */

BDBG_MODULE(BXVD_PLATFORM_REVK0);

#ifdef BXVD_P_USE_INIT_REG_PTRS_REVK0

#if BXVD_P_SVD_AND_AVD_PRESENT
static const BXVD_P_PlatformReg_RevK0 s_stPlatformReg_SVD0RevK0 =
{
   BCHP_SUN_TOP_CTRL_SW_INIT_0_SET,                     /* SWInitSet */
   BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_svd0_sw_init_MASK,   /* SWInitSetAVDMask */
   BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR,                   /* SWInitClear */
   BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR_svd0_sw_init_MASK, /* SWInitClearAVDMask */

   BCHP_DECODE_IP_SHIM_0_SOFTSHUTDOWN_CTRL_REG,             /* SoftShutdownCtrl */
   BCHP_DECODE_IP_SHIM_1_SOFTSHUTDOWN_CTRL_REG_Enable_MASK, /* SoftShutdownCtrlMask */

   BCHP_DECODE_IP_SHIM_0_REG_AVD_CLK_GATE,             /* IPShim_AvdClkGate */
   BCHP_DECODE_IP_SHIM_0_OTP_CTL_REG,                  /* IPShim_OtpCtl */
   BCHP_DECODE_IP_SHIM_1_OTP_CTL_REG_disable_RV9_MASK, /* IPShim_OtpCtl_DisableRV9 */
   BCHP_DECODE_IP_SHIM_0_CPU_ID,                  /* CpuId */
   BCHP_DECODE_IP_SHIM_1_CPU_ID_BLD_ID_MASK,      /* CpuId_BldIdMask */

   BCHP_SVD_INTR2_0_CPU_SET,                      /* CPUL2InterruptSet */

   BCHP_BVNF_INTR2_3_SVD0_STATUS,                 /* Bvnf_Intr2_3_AvdStatus */
   BCHP_BVNF_INTR2_3_SVD0_CLEAR,                  /* Bvnf_Intr2_3_AvdClear */
   BCHP_BVNF_INTR2_3_SVD0_MASK_CLEAR,             /* Bvnf_Intr2_3_AvdMaskClear */

   BCHP_INT_ID_SVD_INTR2_0_AVD_SW_INTR0,          /* PicDataRdy */
   BCHP_INT_ID_SVD_INTR2_0_AVD_SW_INTR3,          /* PicDataRdy1 */
   BCHP_INT_ID_SVD_INTR2_0_AVD_MBOX_INTR,         /* Mailbox */
   BCHP_INT_ID_SVD_INTR2_0_AVD_SW_INTR4,          /* StereoSeqError */
   BCHP_INT_ID_SVD_INTR2_0_AVD_SW_INTR2,          /* StillPictureRdy */
   BCHP_INT_ID_SVD_INTR2_0_AVD_WATCHDOG_INTR,     /* OuterWatchdog */
   BCHP_INT_ID_SVD_INTR2_0_VICH_REG_INTR,         /* VICReg */
   BCHP_INT_ID_SVD_INTR2_0_VICH_SCB_WR_INTR,      /* VICSCBWr */
   BCHP_INT_ID_SVD_INTR2_0_VICH_OL_INST_RD_INTR,  /* VICInstrRead */
   BCHP_INT_ID_SVD_INTR2_0_VICH_IL_INST_RD_INTR,  /* VICILInstrRead */
   BCHP_INT_ID_VICH_BL_INST_RD_INTR,              /* VICBLInstrRead */

   BCHP_DECODE_CPUREGS_0_REG_INST_BASE,         /* InnerInstructionBase */
   BCHP_DECODE_CPUREGS_0_REG_END_OF_CODE,       /* InnerEndOfCode */
   BCHP_DECODE_CPUREGS_0_REG_GLOBAL_IO_BASE,    /* InnerGlobalIOBase */
   BCHP_DECODE_IND_SDRAM_REGS_0_REG_CPU_DBG,    /* InnerCPUDebug */
   BCHP_DECODE_CPUAUX_0_CPUAUX_REG,             /* InnerCPUAux */

   BCHP_DECODE_CPUREGS2_0_REG_INST_BASE,        /* OuterEndOfCode */
   BCHP_DECODE_CPUREGS2_0_REG_END_OF_CODE,      /* OuterInstructionBase */
   BCHP_DECODE_CPUREGS2_0_REG_GLOBAL_IO_BASE,   /* OuterGlobalIOBase */
   BCHP_DECODE_IND_SDRAM_REGS2_0_REG_CPU_DBG,   /* OuterCPUDebug */
   BCHP_DECODE_CPUAUX2_0_CPUAUX_REG,            /* OuterCPUAux */
   BCHP_DECODE_CPUREGS2_0_REG_CPU_INTGEN_MASK,  /* OuterInterruptMask */
   BCHP_DECODE_CPUREGS2_0_REG_CPU_INTGEN_CLR,   /* OuterInterruptClear */
   BCHP_DECODE_CPUREGS2_0_REG_WATCHDOG_TMR,     /* OuterWatchdogTimer */
   BCHP_DECODE_CPUREGS2_0_REG_CPU2HST_MBX,      /* OuterCPU2HostMailbox */
   BCHP_DECODE_CPUREGS2_0_REG_HST2CPU_MBX,      /* OuterHost2CPUMailbox */
   BCHP_DECODE_CPUREGS2_0_REG_CPU2HST_STAT,     /* OuterCPU2HostStatus */

   BCHP_BLD_DECODE_CPUREGS_0_REG_INST_BASE,      /* BaseInstructionBase */
   BCHP_BLD_DECODE_CPUREGS_0_REG_END_OF_CODE,    /* BaseEndOfCode */
   BCHP_BLD_DECODE_CPUREGS_0_REG_GLOBAL_IO_BASE, /* BaseGlobalIOBase */
   BCHP_BLD_DECODE_IND_SDRAM_REGS_0_REG_CPU_DBG, /* BaseCPUDebug */
   BCHP_BLD_DECODE_CPUAUX_0_CPUAUX_REG,          /* BaseCPUAux */

   BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CHANNEL_WR_POSITION_i_ARRAY_BASE,    /* CabacBinDepth */

   BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CTL,            /* uiDecode_CabacBinCtl */
   BCHP_REG_CABAC2BINS2_1_REG_CABAC2BINS_CTL_Reset_MASK, /* uiDecode_CabacBinCtl_ResetMask */
   BCHP_DECODE_SINT_0_REG_SINT_STRM_STAT,                /* uiDecode_SintStrmSts */
   BCHP_DECODE_SINT_1_REG_SINT_STRM_STAT_Rst_MASK,       /* uiDecode_SintStrmSts_ResetMask */
   BCHP_DECODE_SINT_OLOOP_0_DEC_SINT_STRM_STAT,          /* uiDecode_OLSintStrmSts */
   BCHP_DECODE_SINT_OLOOP_1_DEC_SINT_STRM_STAT_Rst_MASK, /* uiDecode_OLSintStrmSts_ResetMask */
   BCHP_DECODE_MAIN_0_REG_MAINCTL,                       /* uiDecode_MainCtl */
   BCHP_DECODE_MAIN_1_REG_MAINCTL_Rst_MASK,              /* uiDecode_MainCtl_ResetMask */
   BCHP_BLD_DECODE_IP_SHIM_0_SW_RESET_REG,              /* SWReset */
   BCHP_DECODE_IP_SHIM_1_SW_RESET_REG_ILSI_reset_MASK,  /* SWReset_ILSIResetMask */


   BCHP_BLD_DECODE_SINT_0_REG_SINT_STRM_STAT,           /* uiDecode_BLD_SintStrmSts */
   BCHP_DECODE_SINT_1_REG_SINT_STRM_STAT_Rst_MASK,      /* uiDecode_BLD_SintStrmSts_ResetMask */
   BCHP_ILS_REGS_0_ILS_SCALE_MODE,                      /* uiDecode_BLD_ILSScale */
   BCHP_ILS_REGS_0_ILS_SCALE_MODE_ILS_Rst_MASK,         /* uiDecode_BLD_ILSScale_ResetMask */
   BCHP_BLD_DECODE_MAIN_0_REG_MAINCTL,                  /* uiDecode_BLD_MainCtl */
   BCHP_DECODE_MAIN_1_REG_MAINCTL_Rst_MASK,             /* uiDecode_BLD_MainCtl_ResetMask */
   BCHP_BLD_DECODE_IP_SHIM_0_SW_RESET_REG,              /* uiDecode_BLD_SWReset */
   BCHP_DECODE_IP_SHIM_1_SW_RESET_REG_ILSI_reset_MASK,  /* uiDecode_BLD_SWReset_ILSIResetMask */

   BCHP_SUN_GISB_ARB_REQ_MASK,                     /* SunGisbArb_ReqMask */
   BCHP_SUN_GISB_ARB_REQ_MASK_svd_0_MASK,          /* SunGisbArb_ReqMaskAVDMask */

   BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH,                       /* StripeWidth */
   BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH_Stripe_Width_SHIFT,    /* StripeWidthShift */
   BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH_pfri_bank_height_SHIFT, /* PFRIBankHeightShift */

   BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH,                       /* PFRIData */
   BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH_ddr3_mode_SHIFT,       /* PFRIDataDDR3ModeShift */
   BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH_pfri_data_width_SHIFT, /* PFRIDataBusWidthShift */
   BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH_pfri_source_SHIFT,     /* PFRIDataSourceShift */
   BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH_pfri_source_MASK,      /* PFRIDataSourceMask */
   BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH_pfri2_source_SHIFT,    /* PFRIData2SourceShift */

   BCHP_BLD_DECODE_SD_0_REG_SD_STRIPE_WIDTH,                      /* BLStripeWidth */
   BCHP_BLD_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH,                   /* BLPFRIData */

   BCHP_AVD_CACHE_0_REG_PCACHE_MODE0,             /* uiAVD_PcacheMode */
   BCHP_AVD_CACHE_1_REG_PCACHE_MODE0_Ygran_MASK,  /* uiAVD_PcacheModeYGranMask */
   BCHP_AVD_CACHE_1_REG_PCACHE_MODE0_Ygran_SHIFT, /* uiAVD_PcacheModeYGranShift */
   BCHP_AVD_CACHE_1_REG_PCACHE_MODE0_Xgran_MASK,  /* uiAVD_PcacheModeXGranMask */
   BCHP_AVD_CACHE_1_REG_PCACHE_MODE0_Xgran_SHIFT, /* uiAVD_PcacheModeXGranShift */

   BCHP_DECODE_RVC_0_REG_RVC_CTL,   /* RVC Control */
   BCHP_DECODE_RVC_0_REG_RVC_PUT,   /* RVC Put */
   BCHP_DECODE_RVC_0_REG_RVC_GET,   /* RVC Get */
   BCHP_DECODE_RVC_0_REG_RVC_BASE,  /* RVC Base */
   BCHP_DECODE_RVC_0_REG_RVC_END,   /* RVC End */

   BCHP_DECODE_IP_SHIM_0_HST_SCRATCH_RAM_ADDR_REG, /* uiDecode_HST_SCR_RAM_ADDR */
   BCHP_DECODE_IP_SHIM_0_HST_SCRATCH_RAM_DATA_REG  /* uiDecode_HST_SCR_RAM_DATA */
};


static const BXVD_P_PlatformReg_RevK0 s_stPlatformReg_AVD1RevK0 =
{
   BCHP_SUN_TOP_CTRL_SW_INIT_0_SET,                     /* SWInitSet */
   BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_avd0_sw_init_MASK,   /* SWInitSetAVDMask */
   BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR,                   /* SWInitClear */
   BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR_avd0_sw_init_MASK, /* SWInitClearAVDMask */

   BCHP_DECODE_IP_SHIM_1_SOFTSHUTDOWN_CTRL_REG,             /* SoftShutdownCtrl */
   BCHP_DECODE_IP_SHIM_1_SOFTSHUTDOWN_CTRL_REG_Enable_MASK, /* SoftShutdownCtrlMask */

   BCHP_DECODE_IP_SHIM_1_REG_AVD_CLK_GATE,             /* IPShim_AvdClkGate */
   BCHP_DECODE_IP_SHIM_1_OTP_CTL_REG,                  /* IPShim_OtpCtl */
   BCHP_DECODE_IP_SHIM_1_OTP_CTL_REG_disable_RV9_MASK, /* IPShim_OtpCtl_DisableRV9 */
   BCHP_DECODE_IP_SHIM_1_CPU_ID,                  /* CpuId */
   BCHP_DECODE_IP_SHIM_1_CPU_ID_BLD_ID_MASK,      /* CpuId_BldIdMask */

   BCHP_AVD_INTR2_1_CPU_SET,                      /* CPUL2InterruptSet */

   BCHP_BVNF_INTR2_3_AVD1_STATUS,                 /* Bvnf_Intr2_3_AvdStatus */
   BCHP_BVNF_INTR2_3_AVD1_CLEAR,                  /* Bvnf_Intr2_3_AvdClear */
   BCHP_BVNF_INTR2_3_AVD1_MASK_CLEAR,             /* Bvnf_Intr2_3_AvdMaskClear */

   BCHP_INT_ID_AVD_INTR2_1_AVD_SW_INTR0,          /* PicDataRdy */
   BCHP_INT_ID_AVD_INTR2_1_AVD_SW_INTR3,          /* PicDataRdy1 */
   BCHP_INT_ID_AVD_INTR2_1_AVD_MBOX_INTR,         /* Mailbox */
   BCHP_INT_ID_AVD_INTR2_1_AVD_SW_INTR4,          /* StereoSeqError */
   BCHP_INT_ID_AVD_INTR2_1_AVD_SW_INTR2,          /* StillPictureRdy */
   BCHP_INT_ID_AVD_INTR2_1_AVD_WATCHDOG_INTR,     /* OuterWatchdog */
   BCHP_INT_ID_AVD_INTR2_1_VICH_REG_INTR,         /* VICReg */
   BCHP_INT_ID_AVD_INTR2_1_VICH_SCB_WR_INTR,      /* VICSCBWr */
   BCHP_INT_ID_AVD_INTR2_1_VICH_OL_INST_RD_INTR,  /* VICInstrRead */
   BCHP_INT_ID_AVD_INTR2_1_VICH_IL_INST_RD_INTR,  /* VICILInstrRead */
   (uint32_t) NULL,                               /* VICBLInstrRead doesn't exist on AVD only core */

   BCHP_DECODE_CPUREGS_1_REG_INST_BASE,         /* InnerInstructionBase */
   BCHP_DECODE_CPUREGS_1_REG_END_OF_CODE,       /* InnerEndOfCode */
   BCHP_DECODE_CPUREGS_1_REG_GLOBAL_IO_BASE,    /* InnerGlobalIOBase */
   BCHP_DECODE_IND_SDRAM_REGS_1_REG_CPU_DBG,    /* InnerCPUDebug */
   BCHP_DECODE_CPUAUX_1_CPUAUX_REG,             /* InnerCPUAux */

   BCHP_DECODE_CPUREGS2_1_REG_INST_BASE,        /* OuterEndOfCode */
   BCHP_DECODE_CPUREGS2_1_REG_END_OF_CODE,      /* OuterInstructionBase */
   BCHP_DECODE_CPUREGS2_1_REG_GLOBAL_IO_BASE,   /* OuterGlobalIOBase */
   BCHP_DECODE_IND_SDRAM_REGS2_1_REG_CPU_DBG,   /* OuterCPUDebug */
   BCHP_DECODE_CPUAUX2_1_CPUAUX_REG,            /* OuterCPUAux */
   BCHP_DECODE_CPUREGS2_1_REG_CPU_INTGEN_MASK,  /* OuterInterruptMask */
   BCHP_DECODE_CPUREGS2_1_REG_CPU_INTGEN_CLR,   /* OuterInterruptClear */
   BCHP_DECODE_CPUREGS2_1_REG_WATCHDOG_TMR,     /* OuterWatchdogTimer */
   BCHP_DECODE_CPUREGS2_1_REG_CPU2HST_MBX,      /* OuterCPU2HostMailbox */
   BCHP_DECODE_CPUREGS2_1_REG_HST2CPU_MBX,      /* OuterHost2CPUMailbox */
   BCHP_DECODE_CPUREGS2_1_REG_CPU2HST_STAT,     /* OuterCPU2HostStatus */

   (uint32_t) NULL,   /* BaseInstructionBase doesn't exist on AVD only decoder */
   (uint32_t) NULL,   /* BaseEndOfCode doesn't exist on AVD only decoder */
   (uint32_t) NULL,   /* BaseGlobalIOBase doesn't exist on AVD only decoder */
   (uint32_t) NULL,   /* BaseCPUDebug doesn't exist on AVD only decoder */
   (uint32_t) NULL,   /* BaseCPUAux doesn't exist on AVD only decoder */

   BCHP_REG_CABAC2BINS2_1_REG_CABAC2BINS_CHANNEL_WR_POSITION_i_ARRAY_BASE,    /* CabacBinDepth */

   BCHP_REG_CABAC2BINS2_1_REG_CABAC2BINS_CTL,            /* uiDecode_CabacBinCtl */
   BCHP_REG_CABAC2BINS2_1_REG_CABAC2BINS_CTL_Reset_MASK, /* uiDecode_CabacBinCtl_ResetMask */
   BCHP_DECODE_SINT_1_REG_SINT_STRM_STAT,                /* uiDecode_SintStrmSts */
   BCHP_DECODE_SINT_1_REG_SINT_STRM_STAT_Rst_MASK,       /* uiDecode_SintStrmSts_ResetMask */
   BCHP_DECODE_SINT_OLOOP_1_DEC_SINT_STRM_STAT,          /* uiDecode_OLSintStrmSts */
   BCHP_DECODE_SINT_OLOOP_1_DEC_SINT_STRM_STAT_Rst_MASK, /* uiDecode_OLSintStrmSts_ResetMask */
   BCHP_DECODE_MAIN_1_REG_MAINCTL,                       /* uiDecode_MainCtl */
   BCHP_DECODE_MAIN_1_REG_MAINCTL_Rst_MASK,              /* uiDecode_MainCtl_ResetMask */
   BCHP_DECODE_IP_SHIM_1_SW_RESET_REG,                  /* SWReset */
   BCHP_DECODE_IP_SHIM_1_SW_RESET_REG_ILSI_reset_MASK,  /* SWReset_ILSIResetMask */


   (uint32_t) NULL,  /* uiDecode_BLD_SintStrmSts */
   (uint32_t) NULL,  /* uiDecode_BLDSintStrmSts_ResetMask */
   (uint32_t) NULL,  /* uiDecode_BLD_ILSScale */
   (uint32_t) NULL,  /* uiDecode_BLD_ILSScale_ResetMask */
   (uint32_t) NULL,  /* uiDecode_BLD_MainCtl */
   (uint32_t) NULL,  /* uiDecode_BLD_MainCtl_ResetMask */
   (uint32_t) NULL,  /* uiDecode_BLD_SWReset */
   (uint32_t) NULL,  /* uiDecode_BLD_SWReset_ILSIResetMask */

   BCHP_SUN_GISB_ARB_REQ_MASK,                     /* SunGisbArb_ReqMask */
   BCHP_SUN_GISB_ARB_REQ_MASK_avd_1_MASK,          /* SunGisbArb_ReqMaskAVDMask */

   BCHP_DECODE_SD_1_REG_SD_STRIPE_WIDTH,                          /* StripeWidth */
   BCHP_DECODE_SD_1_REG_SD_STRIPE_WIDTH_Stripe_Width_SHIFT,       /* StripeWidthShift */
   BCHP_DECODE_SD_1_REG_SD_STRIPE_WIDTH_pfri_bank_height_SHIFT,   /* PFRIBankHeightShift */

   BCHP_DECODE_SD_1_REG_SD_PFRI_DATA_WIDTH,                       /* PFRIData */
   BCHP_DECODE_SD_1_REG_SD_PFRI_DATA_WIDTH_ddr3_mode_SHIFT,       /* PFRIDataDDR3ModeShift */
   BCHP_DECODE_SD_1_REG_SD_PFRI_DATA_WIDTH_pfri_data_width_SHIFT, /* PFRIDataBusWidthShift */
   BCHP_DECODE_SD_1_REG_SD_PFRI_DATA_WIDTH_pfri_source_SHIFT,     /* PFRIDataSourceShift */
   BCHP_DECODE_SD_1_REG_SD_PFRI_DATA_WIDTH_pfri_source_MASK,     /* PFRIDataSourceMASK */
   BCHP_DECODE_SD_1_REG_SD_PFRI_DATA_WIDTH_pfri2_source_SHIFT,    /* PFRIData2SourceShift */

   (uint32_t) NULL,            /* BLStripeWidth doesn't exist on AVD only core */
   (uint32_t) NULL,            /* BLPFRIData doesn't exist on AVD only core */

   BCHP_AVD_CACHE_1_REG_PCACHE_MODE0,             /* uiAVD_PcacheMode */
   BCHP_AVD_CACHE_1_REG_PCACHE_MODE0_Ygran_MASK,  /* uiAVD_PcacheModeYGranMask */
   BCHP_AVD_CACHE_1_REG_PCACHE_MODE0_Ygran_SHIFT, /* uiAVD_PcacheModeYGranShift */
   BCHP_AVD_CACHE_1_REG_PCACHE_MODE0_Xgran_MASK,  /* uiAVD_PcacheModeXGranMask */
   BCHP_AVD_CACHE_1_REG_PCACHE_MODE0_Xgran_SHIFT, /* uiAVD_PcacheModeXGranShift */

   BCHP_DECODE_RVC_1_REG_RVC_CTL,   /* RVC Control */
   BCHP_DECODE_RVC_1_REG_RVC_PUT,   /* RVC Put */
   BCHP_DECODE_RVC_1_REG_RVC_GET,   /* RVC Get */
   BCHP_DECODE_RVC_1_REG_RVC_BASE,  /* RVC Base */
   BCHP_DECODE_RVC_1_REG_RVC_END,   /* RVC End */

   BCHP_DECODE_IP_SHIM_1_HST_SCRATCH_RAM_ADDR_REG, /* uiDecode_HST_SCR_RAM_ADDR */
   BCHP_DECODE_IP_SHIM_1_HST_SCRATCH_RAM_DATA_REG  /* uiDecode_HST_SCR_RAM_DATA */
};

#elif BXVD_P_SVD_ONLY_PRESENT

static const BXVD_P_PlatformReg_RevK0 s_stPlatformReg_SVD0RevK0 =
{
   BCHP_SUN_TOP_CTRL_SW_INIT_0_SET,                     /* SWInitSet */
   BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_svd0_sw_init_MASK,   /* SWInitSetAVDMask */
   BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR,                   /* SWInitClear */
   BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR_svd0_sw_init_MASK, /* SWInitClearAVDMask */

   BCHP_DECODE_IP_SHIM_0_SOFTSHUTDOWN_CTRL_REG,             /* SoftShutdownCtrl */
   BCHP_DECODE_IP_SHIM_0_SOFTSHUTDOWN_CTRL_REG_Enable_MASK, /* SoftShutdownCtrlMask */

   BCHP_DECODE_IP_SHIM_0_REG_AVD_CLK_GATE,        /* IPShim_AvdClkGate */
   BXVD_P_OTP_CTL_REG,                            /* IPShim_OtpCtl */
   BXVD_P_OTP_CTL_REG_disable_RV9_MASK,           /* IPShim_OtpCtl_DisableRV9 */
   BCHP_DECODE_IP_SHIM_0_CPU_ID,                  /* CpuId */
   BCHP_DECODE_IP_SHIM_0_CPU_ID_BLD_ID_MASK,      /* CpuId_BldIdMask */

   BCHP_SVD_INTR2_0_CPU_SET,                      /* CPUL2InterruptSet */

   BCHP_BVNF_INTR2_3_SVD0_STATUS,                 /* Bvnf_Intr2_3_AvdStatus */
   BCHP_BVNF_INTR2_3_SVD0_CLEAR,                  /* Bvnf_Intr2_3_AvdClear */
   BCHP_BVNF_INTR2_3_SVD0_MASK_CLEAR,             /* Bvnf_Intr2_3_AvdMaskClear */

   BCHP_INT_ID_AVD_SW_INTR0,          /* PicDataRdy */
   BCHP_INT_ID_AVD_SW_INTR3,          /* PicDataRdy1 */
   BCHP_INT_ID_AVD_MBOX_INTR,         /* Mailbox */
   BCHP_INT_ID_AVD_SW_INTR4,          /* StereoSeqError */
   BCHP_INT_ID_AVD_SW_INTR2,          /* StillPictureRdy */
   BCHP_INT_ID_AVD_WATCHDOG_INTR,     /* OuterWatchdog */
   BCHP_INT_ID_VICH_REG_INTR,         /* VICReg */
   BCHP_INT_ID_VICH_SCB_WR_INTR,      /* VICSCBWr */
   BCHP_INT_ID_VICH_OL_INST_RD_INTR,  /* VICInstrRead */
   BCHP_INT_ID_VICH_IL_INST_RD_INTR,  /* VICILInstrRead */
   BCHP_INT_ID_VICH_BL_INST_RD_INTR,  /* VICBLInstrRead */

   BCHP_DECODE_CPUREGS_0_REG_INST_BASE,         /* InnerInstructionBase */
   BCHP_DECODE_CPUREGS_0_REG_END_OF_CODE,       /* InnerEndOfCode */
   BCHP_DECODE_CPUREGS_0_REG_GLOBAL_IO_BASE,    /* InnerGlobalIOBase */
   BCHP_DECODE_IND_SDRAM_REGS_0_REG_CPU_DBG,    /* InnerCPUDebug */
   BCHP_DECODE_CPUAUX_0_CPUAUX_REG,             /* InnerCPUAux */

   BCHP_DECODE_CPUREGS2_0_REG_INST_BASE,        /* OuterEndOfCode */
   BCHP_DECODE_CPUREGS2_0_REG_END_OF_CODE,      /* OuterInstructionBase */
   BCHP_DECODE_CPUREGS2_0_REG_GLOBAL_IO_BASE,   /* OuterGlobalIOBase */
   BCHP_DECODE_IND_SDRAM_REGS2_0_REG_CPU_DBG,   /* OuterCPUDebug */
   BCHP_DECODE_CPUAUX2_0_CPUAUX_REG,            /* OuterCPUAux */
   BCHP_DECODE_CPUREGS2_0_REG_CPU_INTGEN_MASK,  /* OuterInterruptMask */
   BCHP_DECODE_CPUREGS2_0_REG_CPU_INTGEN_CLR,   /* OuterInterruptClear */
   BCHP_DECODE_CPUREGS2_0_REG_WATCHDOG_TMR,     /* OuterWatchdogTimer */
   BCHP_DECODE_CPUREGS2_0_REG_CPU2HST_MBX,      /* OuterCPU2HostMailbox */
   BCHP_DECODE_CPUREGS2_0_REG_HST2CPU_MBX,      /* OuterHost2CPUMailbox */
   BCHP_DECODE_CPUREGS2_0_REG_CPU2HST_STAT,     /* OuterCPU2HostStatus */

   BCHP_BLD_DECODE_CPUREGS_0_REG_INST_BASE,      /* BaseInstructionBase */
   BCHP_BLD_DECODE_CPUREGS_0_REG_END_OF_CODE,    /* BaseEndOfCode */
   BCHP_BLD_DECODE_CPUREGS_0_REG_GLOBAL_IO_BASE, /* BaseGlobalIOBase */
   BCHP_BLD_DECODE_IND_SDRAM_REGS_0_REG_CPU_DBG, /* BaseCPUDebug */
   BCHP_BLD_DECODE_CPUAUX_0_CPUAUX_REG,          /* BaseCPUAux */

   BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CHANNEL_WR_POSITION_i_ARRAY_BASE,    /* CabacBinDepth */

   BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CTL,            /* uiDecode_CabacBinCtl */
   BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CTL_Reset_MASK, /* uiDecode_CabacBinCtl_ResetMask */
   BCHP_DECODE_SINT_0_REG_SINT_STRM_STAT,                /* uiDecode_SintStrmSts */
   BCHP_DECODE_SINT_0_REG_SINT_STRM_STAT_Rst_MASK,       /* uiDecode_SintStrmSts_ResetMask */
   BCHP_DECODE_SINT_OLOOP_0_DEC_SINT_STRM_STAT,          /* uiDecode_OLSintStrmSts */
   BCHP_DECODE_SINT_OLOOP_0_DEC_SINT_STRM_STAT_Rst_MASK, /* uiDecode_OLSintStrmSts_ResetMask */
   BCHP_DECODE_MAIN_0_REG_MAINCTL,                       /* uiDecode_MainCtl */
   BCHP_DECODE_MAIN_0_REG_MAINCTL_Rst_MASK,              /* uiDecode_MainCtl_ResetMask */
   BCHP_BLD_DECODE_IP_SHIM_0_SW_RESET_REG,              /* SWReset */
   BCHP_DECODE_IP_SHIM_0_SW_RESET_REG_ILSI_reset_MASK,  /* SWReset_ILSIResetMask */


   BCHP_BLD_DECODE_SINT_0_REG_SINT_STRM_STAT,           /* uiDecode_BLD_SintStrmSts */
   BCHP_DECODE_SINT_0_REG_SINT_STRM_STAT_Rst_MASK,      /* uiDecode_BLD_SintStrmSts_ResetMask */
   BCHP_ILS_REGS_0_ILS_SCALE_MODE,                      /* uiDecode_BLD_ILSScale */
   BCHP_ILS_REGS_0_ILS_SCALE_MODE_ILS_Rst_MASK,         /* uiDecode_BLD_ILSScale_ResetMask */
   BCHP_BLD_DECODE_MAIN_0_REG_MAINCTL,                  /* uiDecode_BLD_MainCtl */
   BCHP_DECODE_MAIN_0_REG_MAINCTL_Rst_MASK,             /* uiDecode_BLD_MainCtl_ResetMask */
   BCHP_BLD_DECODE_IP_SHIM_0_SW_RESET_REG,              /* uiDecode_BLD_SWReset */
   BCHP_DECODE_IP_SHIM_0_SW_RESET_REG_ILSI_reset_MASK,  /* uiDecode_BLD_SWReset_ILSIResetMask */

   BCHP_SUN_GISB_ARB_REQ_MASK,                     /* SunGisbArb_ReqMask */
   BCHP_SUN_GISB_ARB_REQ_MASK_svd_0_MASK,          /* SunGisbArb_ReqMaskAVDMask */

   BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH,                       /* StripeWidth */
   BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH_Stripe_Width_SHIFT,    /* StripeWidthShift */
   BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH_pfri_bank_height_SHIFT, /* PFRIBankHeightShift */

   BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH,                       /* PFRIData */
   BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH_ddr3_mode_SHIFT,       /* PFRIDataDDR3ModeShift */
   BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH_pfri_data_width_SHIFT, /* PFRIDataBusWidthShift */
   BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH_pfri_source_SHIFT,     /* PFRIDataSourceShift */
   BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH_pfri_source_MASK,      /* PFRIDataSourceMask */
   BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH_pfri2_source_SHIFT,    /* PFRIData2SourceShift */

   BCHP_BLD_DECODE_SD_0_REG_SD_STRIPE_WIDTH,                      /* BLStripeWidth */
   BCHP_BLD_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH,                   /* BLPFRIData */

   BCHP_AVD_CACHE_0_REG_PCACHE_MODE0,             /* uiAVD_PcacheMode */
   BCHP_AVD_CACHE_0_REG_PCACHE_MODE0_Ygran_MASK,  /* uiAVD_PcacheModeYGranMask */
   BCHP_AVD_CACHE_0_REG_PCACHE_MODE0_Ygran_SHIFT, /* uiAVD_PcacheModeYGranShift */
   BCHP_AVD_CACHE_0_REG_PCACHE_MODE0_Xgran_MASK,  /* uiAVD_PcacheModeXGranMask */
   BCHP_AVD_CACHE_0_REG_PCACHE_MODE0_Xgran_SHIFT, /* uiAVD_PcacheModeXGranShift */

   BCHP_DECODE_RVC_0_REG_RVC_CTL,   /* RVC Control */
   BCHP_DECODE_RVC_0_REG_RVC_PUT,   /* RVC Put */
   BCHP_DECODE_RVC_0_REG_RVC_GET,   /* RVC Get */
   BCHP_DECODE_RVC_0_REG_RVC_BASE,  /* RVC Base */
   BCHP_DECODE_RVC_0_REG_RVC_END,   /* RVC End */

   BCHP_DECODE_IP_SHIM_0_HST_SCRATCH_RAM_ADDR_REG, /* uiDecode_HST_SCR_RAM_ADDR */
   BCHP_DECODE_IP_SHIM_0_HST_SCRATCH_RAM_DATA_REG  /* uiDecode_HST_SCR_RAM_DATA */
};

#else
static const BXVD_P_PlatformReg_RevK0 s_stPlatformReg_AVD0RevK0 =
{
   BCHP_SUN_TOP_CTRL_SW_INIT_0_SET,                     /* SWInitSet */
   BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_avd0_sw_init_MASK,   /* SWInitSetAVDMask */
   BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR,                   /* SWInitClear */
   BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR_avd0_sw_init_MASK, /* SWInitClearAVDMask */

   BCHP_DECODE_IP_SHIM_0_SOFTSHUTDOWN_CTRL_REG,             /* SoftShutdownCtrl */
   BCHP_DECODE_IP_SHIM_0_SOFTSHUTDOWN_CTRL_REG_Enable_MASK, /* SoftShutdownCtrlMask */

   BCHP_DECODE_IP_SHIM_0_REG_AVD_CLK_GATE,        /* IPShim_AvdClkGate */
   BXVD_P_OTP_CTL_REG,                            /* IPShim_OtpCtl */
   BXVD_P_OTP_CTL_REG_disable_RV9_MASK,           /* IPShim_OtpCtl_DisableRV9 */

   BCHP_DECODE_IP_SHIM_0_CPU_ID,                  /* CpuId */
   BCHP_DECODE_IP_SHIM_0_CPU_ID_BLD_ID_MASK,      /* CpuId_BldIdMask */

   BCHP_AVD_INTR2_0_CPU_SET,                      /* CPUL2InterruptSet */

   BCHP_BVNF_INTR2_3_AVD0_STATUS,                 /* Bvnf_Intr2_3_AvdStatus */
   BCHP_BVNF_INTR2_3_AVD0_CLEAR,                  /* Bvnf_Intr2_3_AvdClear */
   BCHP_BVNF_INTR2_3_AVD0_MASK_CLEAR,             /* Bvnf_Intr2_3_AvdMaskClear */

   BCHP_INT_ID_AVD_SW_INTR0,          /* PicDataRdy */
   BCHP_INT_ID_AVD_SW_INTR3,          /* PicDataRdy1 */
   BCHP_INT_ID_AVD_MBOX_INTR,         /* Mailbox */
   BCHP_INT_ID_AVD_SW_INTR4,          /* StereoSeqError */
   BCHP_INT_ID_AVD_SW_INTR2,          /* StillPictureRdy */
   BCHP_INT_ID_AVD_WATCHDOG_INTR,     /* OuterWatchdog */
   BCHP_INT_ID_VICH_REG_INTR,         /* VICReg */
   BCHP_INT_ID_VICH_SCB_WR_INTR,      /* VICSCBWr */
   BCHP_INT_ID_VICH_OL_INST_RD_INTR,  /* VICInstrRead */
   BCHP_INT_ID_VICH_IL_INST_RD_INTR,  /* VICILInstrRead */
   (uint32_t) NULL,                   /* VICBLInstrRead doesn't exist on AVD only core */

   BCHP_DECODE_CPUREGS_0_REG_INST_BASE,         /* InnerInstructionBase */
   BCHP_DECODE_CPUREGS_0_REG_END_OF_CODE,       /* InnerEndOfCode */
   BCHP_DECODE_CPUREGS_0_REG_GLOBAL_IO_BASE,    /* InnerGlobalIOBase */
   BCHP_DECODE_IND_SDRAM_REGS_0_REG_CPU_DBG,    /* InnerCPUDebug */
   BCHP_DECODE_CPUAUX_0_CPUAUX_REG,             /* InnerCPUAux */

   BCHP_DECODE_CPUREGS2_0_REG_INST_BASE,        /* OuterEndOfCode */
   BCHP_DECODE_CPUREGS2_0_REG_END_OF_CODE,      /* OuterInstructionBase */
   BCHP_DECODE_CPUREGS2_0_REG_GLOBAL_IO_BASE,   /* OuterGlobalIOBase */
   BCHP_DECODE_IND_SDRAM_REGS2_0_REG_CPU_DBG,   /* OuterCPUDebug */
   BCHP_DECODE_CPUAUX2_0_CPUAUX_REG,            /* OuterCPUAux */
   BCHP_DECODE_CPUREGS2_0_REG_CPU_INTGEN_MASK,  /* OuterInterruptMask */
   BCHP_DECODE_CPUREGS2_0_REG_CPU_INTGEN_CLR,   /* OuterInterruptClear */
   BCHP_DECODE_CPUREGS2_0_REG_WATCHDOG_TMR,     /* OuterWatchdogTimer */
   BCHP_DECODE_CPUREGS2_0_REG_CPU2HST_MBX,      /* OuterCPU2HostMailbox */
   BCHP_DECODE_CPUREGS2_0_REG_HST2CPU_MBX,      /* OuterHost2CPUMailbox */
   BCHP_DECODE_CPUREGS2_0_REG_CPU2HST_STAT,     /* OuterCPU2HostStatus */

   (uint32_t) NULL,   /* BaseInstructionBase doesn't exist on AVD only decoder */
   (uint32_t) NULL,   /* BaseEndOfCode doesn't exist on AVD only decoder */
   (uint32_t) NULL,   /* BaseGlobalIOBase doesn't exist on AVD only decoder */
   (uint32_t) NULL,   /* BaseCPUDebug doesn't exist on AVD only decoder */
   (uint32_t) NULL,   /* BaseCPUAux doesn't exist on AVD only decoder */

   BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CHANNEL_WR_POSITION_i_ARRAY_BASE,    /* CabacBinDepth */

   BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CTL,            /* uiDecode_CabacBinCtl */
   BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CTL_Reset_MASK, /* uiDecode_CabacBinCtl_ResetMask */
   BCHP_DECODE_SINT_0_REG_SINT_STRM_STAT,                /* uiDecode_SintStrmSts */
   BCHP_DECODE_SINT_0_REG_SINT_STRM_STAT_Rst_MASK,       /* uiDecode_SintStrmSts_ResetMask */
   BCHP_DECODE_SINT_OLOOP_0_DEC_SINT_STRM_STAT,          /* uiDecode_OLSintStrmSts */
   BCHP_DECODE_SINT_OLOOP_0_DEC_SINT_STRM_STAT_Rst_MASK, /* uiDecode_OLSintStrmSts_ResetMask */
   BCHP_DECODE_MAIN_0_REG_MAINCTL,                       /* uiDecode_MainCtl */
   BCHP_DECODE_MAIN_0_REG_MAINCTL_Rst_MASK,              /* uiDecode_MainCtl_ResetMask */
   BCHP_DECODE_IP_SHIM_0_SW_RESET_REG,                   /* uiDecode_SWReset */
   BCHP_DECODE_IP_SHIM_0_SW_RESET_REG_ILSI_reset_MASK,   /* uiDecode_SWReset_ILSIResetMask */

   /* The following Base Layer registers do not exist in AVD only core */
   (uint32_t) NULL,  /* uiDecode_BLD_SintStrmSts */
   (uint32_t) NULL,  /* uiDecode_BLD_SintStrmSts_ResetMask */
   (uint32_t) NULL,  /* uiDecode_BLD_ILSScale */
   (uint32_t) NULL,  /* uiDecode_BLD_ILSScale_ResetMask */
   (uint32_t) NULL,  /* uiDecode_BLD_MainCtl */
   (uint32_t) NULL,  /* uiDecode_BLD_MainCtl_ResetMask */
   (uint32_t) NULL,  /* uiDecode_BLD_SWReset */
   (uint32_t) NULL,  /* uiDecode_BLD_SWReset_ILSIResetMask */

#if (!BXVD_P_DVD_CHIP)
   BCHP_SUN_GISB_ARB_REQ_MASK,                     /* SunGisbArb_ReqMask */
   BCHP_SUN_GISB_ARB_REQ_MASK_avd_0_MASK,          /* SunGisbArb_ReqMaskAVDMask */

   BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH,                          /* StripeWidth */
   BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH_Stripe_Width_SHIFT,       /* StripeWidthShift */
   BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH_pfri_bank_height_SHIFT,   /* PFRIBankHeightShift */

   BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH,                       /* PFRIData */
   BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH_ddr3_mode_SHIFT,       /* PFRIDataDDR3ModeShift */
   BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH_pfri_data_width_SHIFT, /* PFRIDataBusWidthShift */
   BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH_pfri_source_SHIFT,     /* PFRIDataSourceShift */
   BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH_pfri_source_MASK,      /* PFRIDataSourceMask */
   BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH_pfri2_source_SHIFT,    /* PFRIData2SourceShift */
#else
   BCHP_SUN_GISB_ARB_REQ_MASK,                     /* SunGisbArb_ReqMask */
   BCHP_SUN_GISB_ARB_REQ_MASK_avd0_MASK,           /* SunGisbArb_ReqMaskAVDMask */

   /* Stripe width and data register to not exist on DVD platforms. */
   (uint32_t) NULL, /* StripeWidth */
   (uint32_t) NULL, /* StripeWidthShift */
   (uint32_t) NULL,  /* PFRIBankHeightShift */

   (uint32_t) NULL,  /* PFRIData */
   (uint32_t) NULL,  /* PFRIDataDDR3ModeShift */
   (uint32_t) NULL,  /* PFRIDataBusWidthShift */
   (uint32_t) NULL,  /* PFRIDataSourceShift */
   (uint32_t) NULL,  /* PFRIDataSourceMask */
   (uint32_t) NULL,  /* PFRIData2SourceShift */
#endif

   (uint32_t) NULL,            /* BLStripeWidth doesn't exist on AVD only core */
   (uint32_t) NULL,            /* BLPFRIData doesn't exist on AVD only core */

   BCHP_AVD_CACHE_0_REG_PCACHE_MODE0,             /* uiAVD_PcacheMode */
   BCHP_AVD_CACHE_0_REG_PCACHE_MODE0_Ygran_MASK,  /* uiAVD_PcacheModeYGranMask */
   BCHP_AVD_CACHE_0_REG_PCACHE_MODE0_Ygran_SHIFT, /* uiAVD_PcacheModeYGranShift */
   BCHP_AVD_CACHE_0_REG_PCACHE_MODE0_Xgran_MASK,  /* uiAVD_PcacheModeXGranMask */
   BCHP_AVD_CACHE_0_REG_PCACHE_MODE0_Xgran_SHIFT, /* uiAVD_PcacheModeXGranShift */

   BCHP_DECODE_RVC_0_REG_RVC_CTL,   /* RVC Control */
   BCHP_DECODE_RVC_0_REG_RVC_PUT,   /* RVC Put */
   BCHP_DECODE_RVC_0_REG_RVC_GET,   /* RVC Get */
   BCHP_DECODE_RVC_0_REG_RVC_BASE,  /* RVC Base */
   BCHP_DECODE_RVC_0_REG_RVC_END,   /* RVC End */

   BCHP_DECODE_IP_SHIM_0_HST_SCRATCH_RAM_ADDR_REG, /* uiDecode_HST_SCR_RAM_ADDR */
   BCHP_DECODE_IP_SHIM_0_HST_SCRATCH_RAM_DATA_REG  /* uiDecode_HST_SCR_RAM_DATA */
};
#endif
#endif

#ifdef BXVD_P_USE_DETERMINE_STRIPE_INFO_REVK0
void BXVD_P_DetermineStripeInfo_RevK0( BCHP_DramType ddrType,
                                       uint32_t uiMemPartSize,
                                       uint32_t uiMemBusWidth,
                                       uint32_t uiMemDeviceWidth,
                                       bool     bDDRGroupageEnabled,
                                       uint32_t *puiStripeWidth,
                                       uint32_t *puiBankHeight)
{
   BSTD_UNUSED(ddrType);
   BSTD_UNUSED(uiMemDeviceWidth);
   BSTD_UNUSED(bDDRGroupageEnabled);

   /* Set stripe width and bank height base on bus width and memory part size */
   switch(uiMemBusWidth)
   {
      /* Dram bus width 32 */
      case 32:

         switch(uiMemPartSize)
         {
            case 256: /* 256 Mbits Device Tech*/
               *puiStripeWidth =0; /* 64 bytes */
               *puiBankHeight = 1; /* 2 Mblks */
               break;

            case 512:  /* 512 Mbits Device Tech*/
            case 1024: /* 1024 Mbits Device Tech*/
            case 2048: /* 2048 Mbits Device Tech*/
            case 4096: /* 4096 Mbits Device Tech*/
            case 8192: /* 8192 Mbits Device Tech*/
               *puiStripeWidth =1; /* 128 bytes */
               *puiBankHeight = 1; /* 2 Mblks */
               break;

            default:
               *puiStripeWidth = 0;
               *puiBankHeight  = 1; /* 2 Mblks */
               BDBG_ERR(("Unknown memory part size: %d", uiMemPartSize));
               break;
         }

         break;

      /* Dram bus width 32 */
      case 16:

         switch(uiMemPartSize)
         {
            case 256: /* 256 Mbits Device Tech*/
               *puiStripeWidth =0; /* 64 bytes */
               *puiBankHeight = 0; /* 1 Mblks */
               break;

            case 512:  /* 512 Mbits Device Tech*/
            case 1024: /* 1024 Mbits Device Tech*/
            case 2048: /* 2048 Mbits Device Tech*/
            case 4096: /* 4096 Mbits Device Tech*/
            case 8192: /* 8192 Mbits Device Tech*/
               *puiStripeWidth =0; /* 64 bytes */
               *puiBankHeight = 1; /* 2 Mblks */
               break;

            default:
               *puiStripeWidth = 0; /* 64 bytes */   BSTD_UNUSED(uiMemDeviceWidth);
               *puiBankHeight  = 0; /* 1 Mblks */
               BDBG_ERR(("Unknown memory part size: %d", uiMemPartSize));
               break;
         }

         break;

      default:
         *puiStripeWidth = 0;
         *puiBankHeight  = 0;
         BDBG_ERR(("Unknown memory bus width: %d", uiMemBusWidth));
         break;
   }
}
#endif

#ifdef BXVD_P_USE_INIT_REG_PTRS_REVK0
void BXVD_P_InitRegPtrs_RevK0(BXVD_Handle hXvd)
{

#if (!BXVD_P_DVD_CHIP)
   uint32_t uiDataWidth;
   uint32_t uiDDRMode;
   uint32_t uiXGran=0;
   uint32_t uiYGran=0;
   bool     bDDR3Capable;
   uint32_t uiMemPartSize;
   uint32_t uiMemBusWidth;
   uint32_t pfriSource;
   uint32_t stripeWidth;
   uint32_t bankHeight;
   uint32_t uiReg;
   uint32_t uiPCacheVal;

   BCHP_MemoryInfo stMemoryInfo;

   BERR_Code rc = BERR_SUCCESS;
#endif

   BDBG_ASSERT(hXvd);

   BDBG_ENTER(BXVD_P_InitRegPtrs_RevK0);

   /* Platform Info */
   hXvd->stPlatformInfo.stFirmware.outerELFFirmwareID =
      BXVD_IMAGE_FirmwareID_eOuterELF_AVD0;
   hXvd->stPlatformInfo.stFirmware.innerELFFirmwareID =
      BXVD_IMAGE_FirmwareID_eInnerELF_AVD0;
   hXvd->stPlatformInfo.stFirmware.authenticatedFirmwareID =
      BXVD_IMAGE_FirmwareID_eAuthenticated_AVD0;

#if BXVD_P_SVD_AND_AVD_PRESENT
   if (hXvd->uDecoderInstance == 0)
   {
      hXvd->stPlatformInfo.stReg = s_stPlatformReg_SVD0RevK0;
   }
   else
   {
      hXvd->stPlatformInfo.stReg = s_stPlatformReg_AVD1RevK0;
   }

#elif BXVD_P_SVD_ONLY_PRESENT
   hXvd->stPlatformInfo.stReg = s_stPlatformReg_SVD0RevK0;

#else /* Only AVD Present */
   hXvd->stPlatformInfo.stReg = s_stPlatformReg_AVD0RevK0;
#endif

#if BXVD_P_DYNAMIC_AVD_CORE_FREQ

   /* Some platforms do not have a constant AVD Core clock speed */
   rc = BCHP_GetFeature(hXvd->hChip, BCHP_Feature_eAVDCoreFreq, &hXvd->uiAVDCoreFreq);

   if (rc != BERR_SUCCESS)
      {
         BDBG_ERR(("BCHP_GetFeature; feature not supported: BCHP_Feature_eAVDCoreFreq"));
         return;
      }

   BXVD_DBG_MSG(hXvd, ("AVD core clock freq: %d MHz", hXvd->uiAVDCoreFreq));

   hXvd->uiAVDCoreFreq *= 1000000;

#else

   hXvd->uiAVDCoreFreq = BXVD_P_AVD_CORE_UART_FREQ;

   BXVD_DBG_MSG(hXvd, ("AVD core clock freq: %d MHz", (hXvd->uiAVDCoreFreq/1000000)));

#endif

#if (!BXVD_P_DVD_CHIP)

   rc = BCHP_GetMemoryInfo( hXvd->hChip, &stMemoryInfo);

   if (rc != BERR_SUCCESS)
   {
      BDBG_ERR(("BCHP_GetMemoryInfo failed"));
      return;
   }

/*
 * By default on dual decoder (SVD and AVD) platforms, picture buffers use MEMC_1 memory.
 * If SVD is being forced to use MEMC_0 for picture buffers, then setup for MEMC_0 below.
 */

#if (BXVD_P_SVD_AND_AVD_PRESENT && (!BXVD_P_SVD_USE_MEMC0))
   if (hXvd->uDecoderInstance == 0)
   {
      bDDR3Capable = stMemoryInfo.memc[1].ddr3Capable;
      uiMemPartSize = stMemoryInfo.memc[1].deviceTech;
      uiMemBusWidth = stMemoryInfo.memc[1].width;

      /* SVD0 PFRI attached to SCB1 */
      pfriSource=1;
   }
   else
#endif
   {
      /* AVD/SVD-Only Decoder or BXVD_P_SVD_USE_MEMC0 is enabled */

      bDDR3Capable = stMemoryInfo.memc[0].ddr3Capable;
      uiMemPartSize = stMemoryInfo.memc[0].deviceTech;
      uiMemBusWidth = stMemoryInfo.memc[0].width;

      /* AVD/SVD-Only PFRI attached to SCB0 */
      pfriSource=0;
   }

   BXVD_P_DetermineStripeInfo_RevK0( (BCHP_DramType)NULL, uiMemPartSize, uiMemBusWidth, (uint32_t) NULL, false, &stripeWidth, &bankHeight);

   hXvd->uiDecode_StripeWidth = stripeWidth;
   hXvd->uiDecode_StripeMultiple = bankHeight;

   uiReg = (stripeWidth << hXvd->stPlatformInfo.stReg.uiDecode_StripeWidthShift);
   uiReg |= (bankHeight << hXvd->stPlatformInfo.stReg.uiDecode_PFRIBankHeightShift);

   hXvd->uiDecode_SDStripeWidthRegVal = uiReg;

   if (bDDR3Capable == 1)
   {
      uiDDRMode = BXVD_P_PFRI_DATA_DDR3;
   }
   else
   {
      uiDDRMode = BXVD_P_PFRI_DATA_DDR2;
   }

   /* Only 16 or 32 Bit bus is supported */
   if (uiMemBusWidth == 16)
   {
      uiDataWidth = BXVD_P_PFRI_Data_Width_e16Bit;
   }
   else
   {
      /* 32 bit bus */
      uiDataWidth = BXVD_P_PFRI_Data_Width_e32Bit;
   }

   uiReg = uiDataWidth << hXvd->stPlatformInfo.stReg.uiDecode_PFRIDataBusWidthShift;
   uiReg |= ( pfriSource << hXvd->stPlatformInfo.stReg.uiDecode_PFRIDataSourceShift);
   uiReg |= ( uiDDRMode << hXvd->stPlatformInfo.stReg.uiDecode_PFRIDataDDR3ModeShift);

   uiReg |= ( pfriSource << hXvd->stPlatformInfo.stReg.uiDecode_PFRIData2SourceShift);

   hXvd->uiDecode_PFRIDataRegVal = uiReg;

   if ( uiDDRMode == BXVD_P_PFRI_DATA_DDR2)
   {
      /* DDR2 memory configuration */
      switch (uiDataWidth)
      {
         case BXVD_P_PFRI_Data_Width_e16Bit:

            uiXGran = BXVD_P_PCache_XGran_e8Bytes;
            uiYGran = BXVD_P_PCache_YGran_e1Line;

            break;

         case BXVD_P_PFRI_Data_Width_e32Bit:

            uiXGran = BXVD_P_PCache_XGran_e8Bytes;
            uiYGran = BXVD_P_PCache_YGran_e2Lines;

            break;
      }
   }
   else
   {
      /* DDR3 memory configuration */
      switch ( uiDataWidth)
      {
         case BXVD_P_PFRI_Data_Width_e16Bit:

            uiXGran = BXVD_P_PCache_XGran_e16Bytes;
            uiYGran = BXVD_P_PCache_YGran_e1Line;

            break;

         case BXVD_P_PFRI_Data_Width_e32Bit:

            uiXGran = BXVD_P_PCache_XGran_e16Bytes;
            uiYGran = BXVD_P_PCache_YGran_e2Lines;

            break;
      }
   }

   uiPCacheVal = ( hXvd->uiAVD_PCacheRegVal &
                   (~(hXvd->stPlatformInfo.stReg.uiAVD_PcacheModeYGranMask))&
                   (~(hXvd->stPlatformInfo.stReg.uiAVD_PcacheModeXGranMask)));

   hXvd->uiAVD_PCacheRegVal = uiPCacheVal | ((uiYGran << hXvd->stPlatformInfo.stReg.uiAVD_PcacheModeYGranShift) |
                                            (uiXGran << hXvd->stPlatformInfo.stReg.uiAVD_PcacheModeXGranShift));

#else

   hXvd->uiAVD_PCacheRegVal = BXVD_Reg_Read32(hXvd,
                                              hXvd->stPlatformInfo.stReg.uiAVD_PcacheMode);
#endif

   BDBG_LEAVE(BXVD_P_InitRegPtrs_RevK0);
}
#endif /* BXVD_P_USE_INIT_REG_PTRS_REVK0 */

#ifdef BXVD_P_USE_CORE_RESET_CHIP_REVK0
BERR_Code BXVD_P_ChipReset_RevK0(BXVD_Handle hXvd)
{
   uint32_t uiSoftDnVal;

   BERR_Code rc;

   BDBG_ASSERT(hXvd);

   BDBG_ENTER(BXVD_P_ChipReset_RevK0);

   /* Check to see if Decoder Core is shutdown */
   uiSoftDnVal = BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiAvd_SoftShutdownCtrl);

   if (uiSoftDnVal == hXvd->stPlatformInfo.stReg.uiAvd_SoftShutdownCtrlMask)
   {
      /* Turn Decoder on */
      BXVD_Reg_Write32(hXvd, hXvd->stPlatformInfo.stReg.uiAvd_SoftShutdownCtrl, 0);
   }

   /* Enable the core clocks to be able to access core registers */
   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_IPShim_AvdClkGate,
                    0);

   /* Determine if this is a SVC or AVD decoder */
   if ( (BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_CPUId))
        & hXvd->stPlatformInfo.stReg.uiDecode_CPUId_BldIdMask)
   {
      hXvd->bSVCCapable = true;
   }

#if BXVD_P_RV9_CAPABLE
  /* Determine if this system is RV9 capable */
   if (!(BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_IPShim_OtpCtl)
           & hXvd->stPlatformInfo.stReg.uiDecode_IPShim_OtpCtl_DisableRV9))
   {
      hXvd->bRV9Capable = true;
   }
#endif

   if (hXvd->stSettings.pAVDResetCallback)
   {
      /* Call external reset callback */
      rc = (*hXvd->stSettings.pAVDResetCallback)(hXvd->stSettings.pAVDResetCallbackData);

      if (rc != BERR_SUCCESS)
      {
         BXVD_DBG_ERR(hXvd, ("Error #%d reseting the AVD externally", rc));
         return BERR_TRACE(rc);
      }
   }
   else
   {
      /* Stop OL AVD ARC */
      BXVD_Reg_Write32(hXvd,
                       hXvd->stPlatformInfo.stReg.uiDecode_OuterCPUDebug,
                       1);

      BXVD_Reg_Write32(hXvd,
                       hXvd->stPlatformInfo.stReg.uiDecode_OuterCPUAux+BXVD_P_ARC_STATUS32,
                       1);

      /* Stop IL AVD ARC */
      BXVD_Reg_Write32(hXvd,
                       hXvd->stPlatformInfo.stReg.uiDecode_InnerCPUDebug,
                       1);
      BXVD_Reg_Write32(hXvd,
                       hXvd->stPlatformInfo.stReg.uiDecode_InnerCPUAux+BXVD_P_ARC_STATUS32,
                       1);

      if (hXvd->bSVCCapable)
      {
         /* Stop BL SVD ARC */
         BXVD_Reg_Write32(hXvd,
                          hXvd->stPlatformInfo.stReg.uiDecode_BaseCPUDebug,
                          1);
         BXVD_Reg_Write32(hXvd,
                          hXvd->stPlatformInfo.stReg.uiDecode_BaseCPUAux+BXVD_P_ARC_STATUS32,
                          1);
      }
   }

   /* Reset AVD HW blocks */
   BXVD_P_RESET_CORE( hXvd, hXvd->stPlatformInfo.stReg.uiDecode_CabacBinCtl,
                      hXvd->stPlatformInfo.stReg.uiDecode_CabacBinCtl_ResetMask,
                      "CABAC");

   BXVD_P_RESET_CORE( hXvd, hXvd->stPlatformInfo.stReg.uiDecode_SintStrmSts,
                      hXvd->stPlatformInfo.stReg.uiDecode_SintStrmSts_ResetMask,
                      "Stream");

   BXVD_P_RESET_CORE( hXvd, hXvd->stPlatformInfo.stReg.uiDecode_OLSintStrmSts,
                      hXvd->stPlatformInfo.stReg.uiDecode_OLSintStrmSts_ResetMask,
                      "OLoopStream");

   BXVD_P_RESET_CORE( hXvd, hXvd->stPlatformInfo.stReg.uiDecode_MainCtl,
                      hXvd->stPlatformInfo.stReg.uiDecode_MainCtl_ResetMask,
                      "Backend" );

   BXVD_P_RESET_CORE( hXvd, hXvd->stPlatformInfo.stReg.uiDecode_SWReset,
                      hXvd->stPlatformInfo.stReg.uiDecode_SWReset_ILSIResetMask,
                      "ILSI" );

   if (hXvd->bSVCCapable)
   {
      BXVD_P_RESET_CORE( hXvd, hXvd->stPlatformInfo.stReg.uiDecode_BLD_MainCtl,
                         hXvd->stPlatformInfo.stReg.uiDecode_BLD_MainCtl_ResetMask,
                         "BLD Backend" );

      BXVD_P_RESET_CORE( hXvd, hXvd->stPlatformInfo.stReg.uiDecode_BLD_ILSScale,
                         hXvd->stPlatformInfo.stReg.uiDecode_BLD_ILSScale_ResetMask,
                         "BLD Scaler" );

      BXVD_P_RESET_CORE( hXvd, hXvd->stPlatformInfo.stReg.uiDecode_BLD_SintStrmSts,
                         hXvd->stPlatformInfo.stReg.uiDecode_BLD_SintStrmSts_ResetMask,
                         "BLD Stream" );

      BXVD_P_RESET_CORE( hXvd, hXvd->stPlatformInfo.stReg.uiDecode_BLD_SWReset,
                         hXvd->stPlatformInfo.stReg.uiDecode_BLD_SWReset_ILSIResetMask,
                         "BLD ILSI" );
   }

#if BXVD_P_SVD_GISB_ERR_WORKAROUND

   BREG_Write32(hXvd->hReg,
                hXvd->stPlatformInfo.stReg.uiAvd_SoftShutdownCtrl,
                hXvd->stPlatformInfo.stReg.uiAvd_SoftShutdownCtrlMask);

   BREG_Write32(hXvd->hReg,
                hXvd->stPlatformInfo.stReg.uiSun_SWInitSet,
                hXvd->stPlatformInfo.stReg.uiSun_SWInitSetAvdMask);

   BREG_Write32(hXvd->hReg,
                hXvd->stPlatformInfo.stReg.uiSun_SWInitClear,
                hXvd->stPlatformInfo.stReg.uiSun_SWInitClearAvdMask);

   BREG_Write32(hXvd->hReg, hXvd->stPlatformInfo.stReg.uiAvd_SoftShutdownCtrl, 0);

   /* Clear interrupt mask (Enable) register to prevent interrupts before ISR is setup */
   BREG_Write32(hXvd->hReg,
                hXvd->stPlatformInfo.stReg.uiDecode_OuterInterruptMask,
                0);

   if (hXvd->bSVCCapable )
   {
      /* SW7425-628: work around for the bus error caused by register reads. */

      uint32_t uiValue;

      /* Enable the RBUS-GISB-RBUS Bridge interrupt. */
      uiValue = BREG_Read32(hXvd->hReg, BCHP_SVD_RGR_0_CTRL);
      uiValue |= BCHP_SVD_RGR_0_CTRL_gisb_error_intr_MASK;

      BREG_Write32(hXvd->hReg, BCHP_SVD_RGR_0_CTRL, uiValue);

      /* Mask the RBUS-GISB-RBUS Bridge interrupt.  */
      BREG_Write32(hXvd->hReg,
                   BCHP_SVD_INTR2_0_CPU_MASK_SET,
                   BCHP_SVD_INTR2_0_CPU_MASK_SET_AVD_RGR_BRIDGE_INTR_MASK);
   }
#else

   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiAvd_SoftShutdownCtrl,
                    hXvd->stPlatformInfo.stReg.uiAvd_SoftShutdownCtrlMask);

   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiSun_SWInitSet,
                    hXvd->stPlatformInfo.stReg.uiSun_SWInitSetAvdMask);

   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiSun_SWInitClear,
                    hXvd->stPlatformInfo.stReg.uiSun_SWInitClearAvdMask);

   BXVD_Reg_Write32(hXvd, hXvd->stPlatformInfo.stReg.uiAvd_SoftShutdownCtrl, 0);

   /* Clear interrupt mask (Enable) register to prevent interrupts before ISR is setup */
   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_OuterInterruptMask,
                    0);
#endif

#if (!BXVD_P_DVD_CHIP)

   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_StripeWidth,
                    hXvd->uiDecode_SDStripeWidthRegVal);

   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_PFRIData,
                    hXvd->uiDecode_PFRIDataRegVal);

   if (hXvd->bSVCCapable)
   {
      BXVD_Reg_Write32(hXvd,
                       hXvd->stPlatformInfo.stReg.uiDecode_BLStripeWidth,
                       hXvd->uiDecode_SDStripeWidthRegVal);

      BXVD_Reg_Write32(hXvd,
                          hXvd->stPlatformInfo.stReg.uiDecode_BLPFRIData,
                          hXvd->uiDecode_PFRIDataRegVal);
   }
#endif

   /* Write the AVD PCache mode to hardware */
   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiAVD_PcacheMode,
                    hXvd->uiAVD_PCacheRegVal);

   BDBG_LEAVE(BXVD_P_ChipReset_RevK0);

   return BERR_TRACE(BERR_SUCCESS);
}
#endif


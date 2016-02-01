/***************************************************************************
 *     Copyright (c) 2009-2013, Broadcom Corporation
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
 * Module Description:
 *   See code
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#include "bxvd_platform.h"
#include "bxvd_priv.h"
#include "bxvd.h"
#include "bxvd_intr.h"
#include "bxvd_reg.h"
#include "bxvd_image.h"
#include "bxvd_image_priv.h"
#include "bafl.h"
#include "bchp_common.h"

#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif


/* Define BXVD_POLL_FW_MBX to bring up decoder witout the use in interrupts */
/* #define BXVD_POLL_FW_MBX 1 */
/* #define EMULATION 1 */

/* ARC AUX register offsets */
#define BXVD_P_ARC_PC 0x18
#define BXVD_P_ARC_STATUS32 0x28

BDBG_MODULE(BXVD_PLATFORM_REVN0);

#ifdef BXVD_P_USE_INIT_REG_PTRS_REVN0

#if BXVD_P_THREE_HVD_DECODERS_PRESENT

static const BXVD_P_PlatformReg_RevN0 s_stPlatformReg_HVD0RevN0 =
{
   BCHP_SUN_TOP_CTRL_SW_INIT_0_SET,                  /* SWInitSet */
   BXVD_P_SW_INIT_0_SET_xvd0_sw_init_MASK,           /* SWInitSetAVDMask */

   BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR,                /* SWInitClear */
   BXVD_P_SW_INIT_0_CLEAR_xvd0_sw_init_MASK,         /* SWInitClearAVDMask */

   BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_CTRL,             /* SoftShutdownCtrl */
   BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_CTRL_Enable_MASK, /* SoftShutdownCtrlMask */

   BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS,           /* SoftShutdownStatus */
   BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Done_MASK, /* SoftShutdownStatusMask */

   BCHP_DCD_PIPE_CTL_0_AVD_CLK_GATE,                 /* AvdClkGate */
   BCHP_HEVD_OL_CTL_0_OTP_CTL_REG,                   /* OtpCtl */
   BCHP_HEVD_OL_CTL_0_OTP_CTL_REG_disable_RV9_MASK,  /* OtpCtl_DisableRV9 */

   BCHP_DCD_PIPE_CTL_0_CORE_CONFIG,                    /* CpuId */
   BCHP_DCD_PIPE_CTL_0_CORE_CONFIG_BLD_present_MASK,   /* CpuId_BldIdMask */

#if BXVD_P_HEVD_DUAL_PIPE_PRESENT
   BCHP_DCD_PIPE_CTL_0_CORE_CONFIG_HEVD_dualpipe_MASK, /* CpuId_HEVD_dualpipeMask */
#else
   (uint32_t) NULL,  /* Dual pipe bit doesn't exist */
#endif

   BXVD_P_HVD_INTR2_0_CPU_SET,                     /* CPUL2InterruptSet */

   BCHP_BVNF_INTR2_0_R5F_STATUS,                   /* Bvnf_Inter2_0_R5Status */
   BXVD_P_BVNF_INTR2_3_HVD0_STATUS,                /* Bvnf_Intr2_3_AvdStatus */
   BXVD_P_BVNF_INTR2_3_HVD0_CLEAR,                 /* Bvnf_Intr2_3_AvdClear */
   BXVD_P_BVNF_INTR2_3_HVD0_MASK_CLEAR,            /* Bvnf_Intr2_3_AvdMaskClear */

   BXVD_P_BVNF_INTR2_11_HVD0_STATUS,               /* Bvnf_Intr2_11_AvdStatus */
   BXVD_P_BVNF_INTR2_11_HVD0_CLEAR,                /* Bvnf_Intr2_11_AvdClear */
   BXVD_P_BVNF_INTR2_11_HVD0_MASK_CLEAR,           /* Bvnf_Intr2_11_AvdMaskClear */

   BXVD_P_INT_ID_HVD_INTR2_0_AVD_SW_INTR0,          /* PicDataRdy */
   BXVD_P_INT_ID_HVD_INTR2_0_AVD_SW_INTR3,          /* PicDataRdy1 */
   BXVD_P_INT_ID_HVD_INTR2_0_AVD_SW_INTR5,          /* PicDataRdy2 */

   BXVD_P_INT_ID_HVD_INTR2_0_AVD_MBOX_INTR,         /* Mailbox */
   BXVD_P_INT_ID_HVD_INTR2_0_AVD_SW_INTR4,          /* StereoSeqError */
   BXVD_P_INT_ID_HVD_INTR2_0_AVD_SW_INTR2,          /* StillPictureRdy */
   BXVD_P_INT_ID_HVD_INTR2_0_AVD_WATCHDOG_INTR,     /* OuterWatchdog */
   BXVD_P_INT_ID_HVD_INTR2_0_VICH_REG_INTR,         /* VICReg */
   BXVD_P_INT_ID_HVD_INTR2_0_VICH_SCB_WR_INTR,      /* VICSCBWr */
   BXVD_P_INT_ID_HVD_INTR2_0_VICH_OL_INST_RD_INTR,  /* VICInstrRead */
   BXVD_P_INT_ID_HVD_INTR2_0_VICH_IL_INST_RD_INTR,  /* VICILInstrRead */

   BCHP_HEVD_IL_CPU_REGS_0_INST_BASE,            /* InnerInstructionBase */
   BCHP_HEVD_IL_CPU_REGS_0_END_OF_CODE,          /* InnerEndOfCode */
   BCHP_HEVD_IL_CPU_REGS_0_GLOBAL_IO_BASE,       /* InnerGlobalIOBase */
   BCHP_HEVD_IL_CPU_REGS_0_DEBUG_CTL,            /* InnerCPUDebug */
   BCHP_HEVD_IL_CPU_DEBUG_0_AUX_REGi_ARRAY_BASE, /* InnerCPUAux */

   BCHP_HEVD_OL_CPU_REGS_0_INST_BASE,           /* OuterInstructionBase */
   BCHP_HEVD_OL_CPU_REGS_0_END_OF_CODE,         /* OuterEndOfCode */
   BCHP_HEVD_OL_CPU_REGS_0_GLOBAL_IO_BASE,      /* OuterGlobalIOBase */
   BCHP_HEVD_OL_CPU_REGS_0_DEBUG_CTL,           /* OuterCPUDebug */
   BCHP_HEVD_OL_CPU_DEBUG_0_AUX_REGi_ARRAY_BASE,/* OuterCPUAux */
   BCHP_HEVD_OL_CPU_REGS_0_CPU_INTGEN_MASK,     /* OuterInterruptMask */
   BCHP_HEVD_OL_CPU_REGS_0_CPU_INTGEN_CLR,      /* OuterInterruptClear */
   BCHP_HEVD_OL_CPU_REGS_0_WATCHDOG_TMR,        /* OuterWatchdogTimer */
   BCHP_HEVD_OL_CPU_REGS_0_CPU2HST_MBX,         /* OuterCPU2HostMailbox */
   BCHP_HEVD_OL_CPU_REGS_0_HST2CPU_MBX,         /* OuterHost2CPUMailbox */
   BCHP_HEVD_OL_CPU_REGS_0_CPU2HST_STAT,        /* OuterCPU2HostStatus */

#if BXVD_P_HEVD_DUAL_PIPE_PRESENT
   BCHP_HEVD_IL_CPU_REGS_2_0_INST_BASE,            /* Inner2InstructionBase */
   BCHP_HEVD_IL_CPU_REGS_2_0_END_OF_CODE,          /* Inner2EndOfCode */
   BCHP_HEVD_IL_CPU_REGS_2_0_GLOBAL_IO_BASE,       /* Inner2GlobalIOBase */
   BCHP_HEVD_IL_CPU_REGS_2_0_DEBUG_CTL,            /* Inner2CPUDebug */
   BCHP_HEVD_IL_CPU_DEBUG_2_0_AUX_REGi_ARRAY_BASE, /* Inner2CPUAux */
#else
   (uint32_t) NULL,   /* Inner2InstructionInnerBase doesn't exist */
   (uint32_t) NULL,   /* Inner2EndOfCode doesn't exist */
   (uint32_t) NULL,   /* Inner2GlobalIOInnerBase doesn't exist */
   (uint32_t) NULL,   /* Inner2CPUDebug doesn't */
   (uint32_t) NULL,   /* Inner2CPUAux doesn't exist */
#endif

   BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CHANNEL_WR_POSITION_i_ARRAY_BASE,    /* CabacBinDepth */

   BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CTL,            /* CabacBinCtl */
   BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CTL_Reset_MASK, /* CabacBinCtl_ResetMask */
   BCHP_DECODE_SINT_0_REG_SINT_STRM_STAT,                /* SintStrmSts */
   BCHP_DECODE_SINT_0_REG_SINT_STRM_STAT_Rst_MASK,       /* SintStrmSts_ResetMask */
   BCHP_HEVD_OL_SINT_0_STRM_STAT,                        /* OLSintStrmSts */
   BCHP_HEVD_OL_SINT_0_STRM_STAT_Rst_MASK,               /* OLSintStrmSts_ResetMask */
   BCHP_DECODE_MAIN_0_REG_MAINCTL,                       /* uiDecode_MainCtl */
   BCHP_DECODE_MAIN_0_REG_MAINCTL_Rst_MASK,              /* uiDecode_MainCtl_ResetMask */
   BCHP_DCD_PIPE_CTL_0_SW_RESET_REG,                     /* SWReset */
   BCHP_DCD_PIPE_CTL_0_SW_RESET_REG_ILSI_reset_MASK,     /* SWReset_ILSIResetMask */

   BCHP_SUN_GISB_ARB_REQ_MASK,                     /* SunGisbArb_ReqMask */
   BXVD_P_GISB_ARB_REQ_MASK_HVD0_MASK,             /* SunGisbArb_ReqMaskAVDMask */

   BCHP_HEVD_PFRI_0_STRIPE_WIDTH,                       /* StripeWidth */
   BCHP_HEVD_PFRI_0_STRIPE_WIDTH_Stripe_Width_SHIFT,    /* StripeWidthShift */

   BCHP_HEVD_PFRI_0_INFO,                        /* PFRIInfo */
   BCHP_HEVD_PFRI_0_INFO_pfri_data_width_MASK,   /* PFRIInfo_DataBusWidthMask */
   BCHP_HEVD_PFRI_0_INFO_pfri_data_width_SHIFT,  /* PFRIInfo_DataBusWidthShift */
   BCHP_HEVD_PFRI_0_INFO_pfri_bank_height_MASK,  /* PFRIInfo_BankHeightMask */
   BCHP_HEVD_PFRI_0_INFO_pfri_bank_height_SHIFT, /* PFRIInfo_BankHeightShift */

#ifdef BCHP_HEVD_PFRI_0_INFO_pfri_num_banks_SHIFT
   BCHP_HEVD_PFRI_0_INFO_pfri_groupages_en_MASK,  /* uiPFRIInfo_PFRIGroupagesEnaMask */
   BCHP_HEVD_PFRI_0_INFO_pfri_groupages_en_SHIFT, /* uiPFRIInfo_PFRIGroupagesEnaShift */
   BCHP_HEVD_PFRI_0_INFO_pfri_num_banks_MASK,     /* uiPFRIInfo_PFRINumBanksMask */
   BCHP_HEVD_PFRI_0_INFO_pfri_num_banks_SHIFT,    /* uiPFRIInfo_PFRINumBanksShift */
#else
   (uint32_t) 0,
   (uint32_t) 0,
   (uint32_t) 0,
   (uint32_t) 0,
#endif

   BCHP_HEVD_PCACHE_0_MODE0,             /* uiAVD_PcacheMode */
   BCHP_HEVD_PCACHE_0_MODE0_Ygran_MASK,  /* uiAVD_PcacheModeYGranMask */
   BCHP_HEVD_PCACHE_0_MODE0_Ygran_SHIFT, /* uiAVD_PcacheModeYGranShift */
   BCHP_HEVD_PCACHE_0_MODE0_Xgran_MASK,  /* uiAVD_PcacheModeXGranMask */
   BCHP_HEVD_PCACHE_0_MODE0_Xgran_SHIFT, /* uiAVD_PcacheModeXGranShift */

   /* 2nd pipe PFRI and stripe registers */
#if BXVD_P_HEVD_DUAL_PIPE_PRESENT
   BCHP_HEVD_PFRI_2_0_STRIPE_WIDTH,      /* StripeWidth_P2 */
   BCHP_HEVD_PFRI_2_0_INFO,              /* PFRIInfo_P2 */
   BCHP_HEVD_PCACHE_2_0_MODE0,           /* uiAVD_PcacheMode */
#else
   (uint32_t) 0,
   (uint32_t) 0,
   (uint32_t) 0,
#endif

   BCHP_RVC_0_CTL,   /* RVC Control */
   BCHP_RVC_0_PUT,   /* RVC Put */
   BCHP_RVC_0_GET,   /* RVC Get */
   BCHP_RVC_0_BASE,  /* RVC Base */
   BCHP_RVC_0_END,   /* RVC End */

   BCHP_HEVD_OL_CTL_0_ENTRYi_ARRAY_BASE /* ARC FW HIM base */
};

static const BXVD_P_PlatformReg_RevN0 s_stPlatformReg_HVD1RevN0 =
{
   BCHP_SUN_TOP_CTRL_SW_INIT_0_SET,                     /* SWInitSet */
   BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_hvd1_sw_init_MASK,   /* SWInitSetAVDMask */
   BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR,                   /* SWInitClear */
   BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR_hvd1_sw_init_MASK, /* SWInitClearAVDMask */

   BCHP_HEVD_OL_CTL_1_SOFTSHUTDOWN_CTRL,             /* SoftShutdownCtrl */
   BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_CTRL_Enable_MASK, /* SoftShutdownCtrlMask */

   BCHP_HEVD_OL_CTL_1_SOFTSHUTDOWN_STATUS,           /* SoftShutdownStatus */
   BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Done_MASK, /* SoftShutdownStatusMask */

   BCHP_DCD_PIPE_CTL_1_AVD_CLK_GATE,                 /* AvdClkGate */
   BCHP_HEVD_OL_CTL_1_OTP_CTL_REG,                   /* OtpCtl */
   BCHP_HEVD_OL_CTL_0_OTP_CTL_REG_disable_RV9_MASK,  /* OtpCtl_DisableRV9 */

   BCHP_DCD_PIPE_CTL_1_CORE_CONFIG,                    /* CpuId */
   BCHP_DCD_PIPE_CTL_0_CORE_CONFIG_BLD_present_MASK,   /* CpuId_BldIdMask */

#if BXVD_P_HEVD_DUAL_PIPE_PRESENT
   BCHP_DCD_PIPE_CTL_0_CORE_CONFIG_HEVD_dualpipe_MASK, /* CpuId_HEVD_dualpipeMask */
#else
   (uint32_t) 0,  /* Dual pipe bit doesn't exist */
#endif

   BXVD_P_HVD_INTR2_1_CPU_SET,                    /* CPUL2InterruptSet */

   BCHP_BVNF_INTR2_0_R5F_STATUS,                  /* Bvnf_Inter2_0_R5Status */
   BCHP_BVNF_INTR2_3_HVD1_STATUS,                 /* Bvnf_Intr2_3_AvdStatus */
   BCHP_BVNF_INTR2_3_HVD1_CLEAR,                  /* Bvnf_Intr2_3_AvdClear */
   BCHP_BVNF_INTR2_3_HVD1_MASK_CLEAR,             /* Bvnf_Intr2_3_AvdMaskClear */

   BCHP_BVNF_INTR2_11_HVD1_STATUS,                /* Bvnf_Intr2_11_AvdStatus */
   BCHP_BVNF_INTR2_11_HVD1_CLEAR,                 /* Bvnf_Intr2_11_AvdClear */
   BCHP_BVNF_INTR2_11_HVD1_MASK_CLEAR,            /* Bvnf_Intr2_11_AvdMaskClear */

   BXVD_P_INT_ID_HVD_INTR2_1_AVD_SW_INTR0,          /* PicDataRdy */
   BXVD_P_INT_ID_HVD_INTR2_1_AVD_SW_INTR3,          /* PicDataRdy1 */
   BXVD_P_INT_ID_HVD_INTR2_1_AVD_SW_INTR5,          /* PicDataRdy2 */

   BXVD_P_INT_ID_HVD_INTR2_1_AVD_MBOX_INTR,         /* Mailbox */
   BXVD_P_INT_ID_HVD_INTR2_1_AVD_SW_INTR4,          /* StereoSeqError */
   BXVD_P_INT_ID_HVD_INTR2_1_AVD_SW_INTR2,          /* StillPictureRdy */
   BXVD_P_INT_ID_HVD_INTR2_1_AVD_WATCHDOG_INTR,     /* OuterWatchdog */
   BXVD_P_INT_ID_HVD_INTR2_1_VICH_REG_INTR,         /* VICReg */
   BXVD_P_INT_ID_HVD_INTR2_1_VICH_SCB_WR_INTR,      /* VICSCBWr */
   BXVD_P_INT_ID_HVD_INTR2_1_VICH_OL_INST_RD_INTR,  /* VICInstrRead */
   BXVD_P_INT_ID_HVD_INTR2_1_VICH_IL_INST_RD_INTR,  /* VICILInstrRead */

   BCHP_HEVD_IL_CPU_REGS_1_INST_BASE,            /* InnerInstructionBase */
   BCHP_HEVD_IL_CPU_REGS_1_END_OF_CODE,          /* InnerEndOfCode */
   BCHP_HEVD_IL_CPU_REGS_1_GLOBAL_IO_BASE,       /* InnerGlobalIOBase */
   BCHP_HEVD_IL_CPU_REGS_1_DEBUG_CTL,            /* InnerCPUDebug */
   BCHP_HEVD_IL_CPU_DEBUG_1_AUX_REGi_ARRAY_BASE, /* InnerCPUAux */

   BCHP_HEVD_OL_CPU_REGS_1_INST_BASE,           /* OuterInstructionBase */
   BCHP_HEVD_OL_CPU_REGS_1_END_OF_CODE,         /* OuterEndOfCode */
   BCHP_HEVD_OL_CPU_REGS_1_GLOBAL_IO_BASE,      /* OuterGlobalIOBase */
   BCHP_HEVD_OL_CPU_REGS_1_DEBUG_CTL,           /* OuterCPUDebug */
   BCHP_HEVD_OL_CPU_DEBUG_1_AUX_REGi_ARRAY_BASE,/* OuterCPUAux */
   BCHP_HEVD_OL_CPU_REGS_1_CPU_INTGEN_MASK,     /* OuterInterruptMask */
   BCHP_HEVD_OL_CPU_REGS_1_CPU_INTGEN_CLR,      /* OuterInterruptClear */
   BCHP_HEVD_OL_CPU_REGS_1_WATCHDOG_TMR,        /* OuterWatchdogTimer */
   BCHP_HEVD_OL_CPU_REGS_1_CPU2HST_MBX,         /* OuterCPU2HostMailbox */
   BCHP_HEVD_OL_CPU_REGS_1_HST2CPU_MBX,         /* OuterHost2CPUMailbox */
   BCHP_HEVD_OL_CPU_REGS_1_CPU2HST_STAT,        /* OuterCPU2HostStatus */

   (uint32_t) 0,   /* Inner2InstructionInnerBase doesn't exist */
   (uint32_t) 0,   /* Inner2EndOfCode doesn't exist */
   (uint32_t) 0,   /* Inner2GlobalIOInnerBase doesn't exist */
   (uint32_t) 0,   /* Inner2CPUDebug doesn't exist */
   (uint32_t) 0,   /* Inner2CPUAux doesn't exist on */

   BCHP_REG_CABAC2BINS2_1_REG_CABAC2BINS_CHANNEL_WR_POSITION_i_ARRAY_BASE,    /* CabacBinDepth */

   BCHP_REG_CABAC2BINS2_1_REG_CABAC2BINS_CTL,            /* uiDecode_CabacBinCtl */
   BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CTL_Reset_MASK, /* uiDecode_CabacBinCtl_ResetMask */
   BCHP_DECODE_SINT_1_REG_SINT_STRM_STAT,                /* uiDecode_SintStrmSts */
   BCHP_DECODE_SINT_0_REG_SINT_STRM_STAT_Rst_MASK,       /* uiDecode_SintStrmSts_ResetMask */
   BCHP_HEVD_OL_SINT_1_STRM_STAT,                        /* OLSintStrmSts */
   BCHP_HEVD_OL_SINT_0_STRM_STAT_Rst_MASK,               /* OLSintStrmSts_ResetMask */

   BCHP_DECODE_MAIN_1_REG_MAINCTL,                       /* uiDecode_MainCtl */
   BCHP_DECODE_MAIN_0_REG_MAINCTL_Rst_MASK,              /* uiDecode_MainCtl_ResetMask */
   BCHP_DCD_PIPE_CTL_1_SW_RESET_REG,                     /* SWReset */
   BCHP_DCD_PIPE_CTL_0_SW_RESET_REG_ILSI_reset_MASK,     /* SWReset_ILSIResetMask */

   BCHP_SUN_GISB_ARB_REQ_MASK,                     /* SunGisbArb_ReqMask */
   BCHP_SUN_GISB_ARB_REQ_MASK_hvd_0_MASK,          /* SunGisbArb_ReqMaskAVDMask */

   BCHP_HEVD_PFRI_1_STRIPE_WIDTH,                       /* StripeWidth */
   BCHP_HEVD_PFRI_0_STRIPE_WIDTH_Stripe_Width_SHIFT,    /* StripeWidthShift */

   BCHP_HEVD_PFRI_1_INFO,                        /* PFRIInfo */
   BCHP_HEVD_PFRI_0_INFO_pfri_data_width_MASK,   /* PFRIInfo_DataBusWidthMask */
   BCHP_HEVD_PFRI_0_INFO_pfri_data_width_SHIFT,  /* PFRIInfo_DataBusWidthShift */
   BCHP_HEVD_PFRI_0_INFO_pfri_bank_height_MASK,  /* PFRIInfo_BankHeightMask */
   BCHP_HEVD_PFRI_0_INFO_pfri_bank_height_SHIFT, /* PFRIInfo_BankHeightShift */

#ifdef BCHP_HEVD_PFRI_0_INFO_pfri_num_banks_SHIFT
   BCHP_HEVD_PFRI_0_INFO_pfri_groupages_en_MASK , /* uiPFRIInfo_PFRIGroupagesEnaMask */
   BCHP_HEVD_PFRI_0_INFO_pfri_groupages_en_SHIFT, /* uiPFRIInfo_PFRIGroupagesEnaShift */
   BCHP_HEVD_PFRI_0_INFO_pfri_num_banks_MASK,     /* uiPFRIInfo_PFRINumBanksMask */
   BCHP_HEVD_PFRI_0_INFO_pfri_num_banks_SHIFT,    /* uiPFRIInfo_PFRINumBanksShift */
#else
   (uint32_t) 0,
   (uint32_t) 0,
   (uint32_t) 0,
   (uint32_t) 0,
#endif

   BCHP_HEVD_PCACHE_1_MODE0,             /* uiAVD_PcacheMode */
   BCHP_HEVD_PCACHE_0_MODE0_Ygran_MASK,  /* uiAVD_PcacheModeYGranMask */
   BCHP_HEVD_PCACHE_0_MODE0_Ygran_SHIFT, /* uiAVD_PcacheModeYGranShift */
   BCHP_HEVD_PCACHE_0_MODE0_Xgran_MASK,  /* uiAVD_PcacheModeXGranMask */
   BCHP_HEVD_PCACHE_0_MODE0_Xgran_SHIFT, /* uiAVD_PcacheModeXGranShift */

   /* 2nd pipe PFRI and stripe registers */
   (uint32_t) 0,
   (uint32_t) 0,
   (uint32_t) 0,

   BCHP_RVC_1_CTL,   /* RVC Control */
   BCHP_RVC_1_PUT,   /* RVC Put */
   BCHP_RVC_1_GET,   /* RVC Get */
   BCHP_RVC_1_BASE,  /* RVC Base */
   BCHP_RVC_1_END,   /* RVC End */

   BCHP_HEVD_OL_CTL_1_ENTRYi_ARRAY_BASE /* ARC FW HIM base */
};

static const BXVD_P_PlatformReg_RevN0 s_stPlatformReg_HVD2RevN0 =
{
   BCHP_SUN_TOP_CTRL_SW_INIT_1_SET,                     /* SWInitSet */
   BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_hvd2_sw_init_MASK,   /* SWInitSetAVDMask */
   BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR,                   /* SWInitClear */
   BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_hvd2_sw_init_MASK, /* SWInitClearAVDMask */

   BCHP_HEVD_OL_CTL_2_SOFTSHUTDOWN_CTRL,             /* SoftShutdownCtrl */
   BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_CTRL_Enable_MASK, /* SoftShutdownCtrlMask */

   BCHP_HEVD_OL_CTL_2_SOFTSHUTDOWN_STATUS,           /* SoftShutdownStatus */
   BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Done_MASK, /* SoftShutdownStatusMask */

   BCHP_DCD_PIPE_CTL_2_AVD_CLK_GATE,                 /* AvdClkGate */
   BCHP_HEVD_OL_CTL_2_OTP_CTL_REG,                   /* OtpCtl */
   BCHP_HEVD_OL_CTL_0_OTP_CTL_REG_disable_RV9_MASK,  /* OtpCtl_DisableRV9 */

   BCHP_DCD_PIPE_CTL_2_CORE_CONFIG,                  /* CpuId */
   BCHP_DCD_PIPE_CTL_0_CORE_CONFIG_BLD_present_MASK, /* CpuId_BldIdMask */

#if BXVD_P_HEVD_DUAL_PIPE_PRESENT
   BCHP_DCD_PIPE_CTL_0_CORE_CONFIG_HEVD_dualpipe_MASK, /* CpuId_HEVD_dualpipeMask */
#else
   (uint32_t) 0,  /* Dual pipe bit doesn't exist */
#endif

   BXVD_P_HVD_INTR2_2_CPU_SET,                      /* CPUL2InterruptSet */

   BCHP_BVNF_INTR2_0_R5F_STATUS,                   /* Bvnf_Inter2_0_R5Status */
   BCHP_BVNF_INTR2_3_HVD2_STATUS,                  /* Bvnf_Intr2_3_AvdStatus */
   BCHP_BVNF_INTR2_3_HVD2_CLEAR,                   /* Bvnf_Intr2_3_AvdClear */
   BCHP_BVNF_INTR2_3_HVD2_MASK_CLEAR,              /* Bvnf_Intr2_3_AvdMaskClear */

   BCHP_BVNF_INTR2_11_HVD2_STATUS,                 /* Bvnf_Intr2_11_AvdStatus */
   BCHP_BVNF_INTR2_11_HVD2_CLEAR,                  /* Bvnf_Intr2_11_AvdClear */
   BCHP_BVNF_INTR2_11_HVD2_MASK_CLEAR,             /* Bvnf_Intr2_11_AvdMaskClear */

   BXVD_P_INT_ID_HVD_INTR2_2_AVD_SW_INTR0,          /* PicDataRdy */
   BXVD_P_INT_ID_HVD_INTR2_2_AVD_SW_INTR3,          /* PicDataRdy1 */
   BXVD_P_INT_ID_HVD_INTR2_2_AVD_SW_INTR5,          /* PicDataRdy2 */

   BXVD_P_INT_ID_HVD_INTR2_2_AVD_MBOX_INTR,         /* Mailbox */
   BXVD_P_INT_ID_HVD_INTR2_2_AVD_SW_INTR4,          /* StereoSeqError */
   BXVD_P_INT_ID_HVD_INTR2_2_AVD_SW_INTR2,          /* StillPictureRdy */
   BXVD_P_INT_ID_HVD_INTR2_2_AVD_WATCHDOG_INTR,     /* OuterWatchdog */
   BXVD_P_INT_ID_HVD_INTR2_2_VICH_REG_INTR,         /* VICReg */
   BXVD_P_INT_ID_HVD_INTR2_2_VICH_SCB_WR_INTR,      /* VICSCBWr */
   BXVD_P_INT_ID_HVD_INTR2_2_VICH_OL_INST_RD_INTR,  /* VICInstrRead */
   BXVD_P_INT_ID_HVD_INTR2_2_VICH_IL_INST_RD_INTR,  /* VICILInstrRead */

   BCHP_HEVD_IL_CPU_REGS_2_INST_BASE,            /* InnerInstructionBase */
   BCHP_HEVD_IL_CPU_REGS_2_END_OF_CODE,          /* InnerEndOfCode */
   BCHP_HEVD_IL_CPU_REGS_2_GLOBAL_IO_BASE,       /* InnerGlobalIOBase */
   BCHP_HEVD_IL_CPU_REGS_2_DEBUG_CTL,            /* InnerCPUDebug */
   BCHP_HEVD_IL_CPU_DEBUG_2_AUX_REGi_ARRAY_BASE, /* InnerCPUAux */

   BCHP_HEVD_OL_CPU_REGS_2_INST_BASE,           /* OuterInstructionBase */
   BCHP_HEVD_OL_CPU_REGS_2_END_OF_CODE,         /* OuterEndOfCode */
   BCHP_HEVD_OL_CPU_REGS_2_GLOBAL_IO_BASE,      /* OuterGlobalIOBase */
   BCHP_HEVD_OL_CPU_REGS_2_DEBUG_CTL,           /* OuterCPUDebug */
   BCHP_HEVD_OL_CPU_DEBUG_2_AUX_REGi_ARRAY_BASE,/* OuterCPUAux */
   BCHP_HEVD_OL_CPU_REGS_2_CPU_INTGEN_MASK,     /* OuterInterruptMask */
   BCHP_HEVD_OL_CPU_REGS_2_CPU_INTGEN_CLR,      /* OuterInterruptClear */
   BCHP_HEVD_OL_CPU_REGS_2_WATCHDOG_TMR,        /* OuterWatchdogTimer */
   BCHP_HEVD_OL_CPU_REGS_2_CPU2HST_MBX,         /* OuterCPU2HostMailbox */
   BCHP_HEVD_OL_CPU_REGS_2_HST2CPU_MBX,         /* OuterHost2CPUMailbox */
   BCHP_HEVD_OL_CPU_REGS_2_CPU2HST_STAT,        /* OuterCPU2HostStatus */

   (uint32_t) 0,   /* Inner2InstructionBase doesn't exist */
   (uint32_t) 0,   /* Inner2EndOfCode doesn't exist */
   (uint32_t) 0,   /* Inner2GlobalIOBase doesn't exist */
   (uint32_t) 0,   /* Inner2CPUDebug doesn't exist */
   (uint32_t) 0,   /* Inner2CPUAux doesn't exist */

   BCHP_REG_CABAC2BINS2_2_REG_CABAC2BINS_CHANNEL_WR_POSITION_i_ARRAY_BASE,    /* CabacBinDepth */

   BCHP_REG_CABAC2BINS2_2_REG_CABAC2BINS_CTL,            /* uiDecode_CabacBinCtl */
   BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CTL_Reset_MASK, /* uiDecode_CabacBinCtl_ResetMask */
   BCHP_DECODE_SINT_2_REG_SINT_STRM_STAT,                /* uiDecode_SintStrmSts */
   BCHP_DECODE_SINT_0_REG_SINT_STRM_STAT_Rst_MASK,       /* uiDecode_SintStrmSts_ResetMask */
   BCHP_HEVD_OL_SINT_2_STRM_STAT,                        /* OLSintStrmSts */
   BCHP_HEVD_OL_SINT_0_STRM_STAT_Rst_MASK,               /* OLSintStrmSts_ResetMask */

   BCHP_DECODE_MAIN_2_REG_MAINCTL,                       /* uiDecode_MainCtl */
   BCHP_DECODE_MAIN_0_REG_MAINCTL_Rst_MASK,              /* uiDecode_MainCtl_ResetMask */
   BCHP_DCD_PIPE_CTL_2_SW_RESET_REG,                     /* SWReset */
   BCHP_DCD_PIPE_CTL_0_SW_RESET_REG_ILSI_reset_MASK,     /* SWReset_ILSIResetMask */

   BCHP_SUN_GISB_ARB_REQ_MASK,                     /* SunGisbArb_ReqMask */
   BCHP_SUN_GISB_ARB_REQ_MASK_hvd_1_MASK,          /* SunGisbArb_ReqMaskAVDMask */

   BCHP_HEVD_PFRI_2_STRIPE_WIDTH,                       /* StripeWidth */
   BCHP_HEVD_PFRI_0_STRIPE_WIDTH_Stripe_Width_SHIFT,    /* StripeWidthShift */

   BCHP_HEVD_PFRI_2_INFO,                        /* PFRIInfo */
   BCHP_HEVD_PFRI_0_INFO_pfri_data_width_MASK,   /* PFRIInfo_DataBusWidthMask */
   BCHP_HEVD_PFRI_0_INFO_pfri_data_width_SHIFT,  /* PFRIInfo_DataBusWidthShift */
   BCHP_HEVD_PFRI_0_INFO_pfri_bank_height_MASK,  /* PFRIInfo_BankHeightMask */
   BCHP_HEVD_PFRI_0_INFO_pfri_bank_height_SHIFT, /* PFRIInfo_BankHeightShift */

#ifdef BCHP_HEVD_PFRI_0_INFO_pfri_num_banks_SHIFT
   BCHP_HEVD_PFRI_0_INFO_pfri_groupages_en_MASK , /* uiPFRIInfo_PFRIGroupagesEnaMask */
   BCHP_HEVD_PFRI_0_INFO_pfri_groupages_en_SHIFT, /* uiPFRIInfo_PFRIGroupagesEnaShift */
   BCHP_HEVD_PFRI_0_INFO_pfri_num_banks_MASK,     /* uiPFRIInfo_PFRINumBanksMask */
   BCHP_HEVD_PFRI_0_INFO_pfri_num_banks_SHIFT,    /* uiPFRIInfo_PFRINumBanksShift */
#else
   (uint32_t) 0,
   (uint32_t) 0,
   (uint32_t) 0,
   (uint32_t) 0,
#endif

   BCHP_HEVD_PCACHE_2_MODE0,             /* uiAVD_PcacheMode */
   BCHP_HEVD_PCACHE_0_MODE0_Ygran_MASK,  /* uiAVD_PcacheModeYGranMask */
   BCHP_HEVD_PCACHE_0_MODE0_Ygran_SHIFT, /* uiAVD_PcacheModeYGranShift */
   BCHP_HEVD_PCACHE_0_MODE0_Xgran_MASK,  /* uiAVD_PcacheModeXGranMask */
   BCHP_HEVD_PCACHE_0_MODE0_Xgran_SHIFT, /* uiAVD_PcacheModeXGranShift */

   /* 2nd pipe PFRI and stripe registers */
   (uint32_t) 0,
   (uint32_t) 0,
   (uint32_t) 0,

   BCHP_RVC_2_CTL,   /* RVC Control */
   BCHP_RVC_2_PUT,   /* RVC Put */
   BCHP_RVC_2_GET,   /* RVC Get */
   BCHP_RVC_2_BASE,  /* RVC Base */
   BCHP_RVC_2_END,   /* RVC End */

   BCHP_HEVD_OL_CTL_2_ENTRYi_ARRAY_BASE /* ARC FW HIM base */
};
/* end #if BXVD_P_SHVD_AND_HVD_PRESENT */
#elif BXVD_P_TWO_HVD_DECODERS_PRESENT

static const BXVD_P_PlatformReg_RevN0 s_stPlatformReg_HVD0RevN0 =
{
   BCHP_SUN_TOP_CTRL_SW_INIT_0_SET,                     /* SWInitSet */
   BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_hvd0_sw_init_MASK,   /* SWInitSetAVDMask */
   BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR,                   /* SWInitClear */
   BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR_hvd0_sw_init_MASK, /* SWInitClearAVDMask */

   BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_CTRL,             /* SoftShutdownCtrl */
   BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_CTRL_Enable_MASK, /* SoftShutdownCtrlMask */

#if BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS
   BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS,           /* SoftShutdownStatus */
   BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Done_MASK, /* SoftShutdownStatusMask */
#else
   (uint32_t) 0,
   (uint32_t) 0,
#endif

   BCHP_DCD_PIPE_CTL_0_AVD_CLK_GATE,                 /* AvdClkGate */
   BCHP_HEVD_OL_CTL_0_OTP_CTL_REG,                   /* OtpCtl */
   BCHP_HEVD_OL_CTL_0_OTP_CTL_REG_disable_RV9_MASK,  /* OtpCtl_DisableRV9 */

   BCHP_DCD_PIPE_CTL_0_CORE_CONFIG,                    /* CpuId */
   BCHP_DCD_PIPE_CTL_0_CORE_CONFIG_BLD_present_MASK,   /* CpuId_BldIdMask */
#if BXVD_P_HEVD_DUAL_PIPE_PRESENT
   BCHP_DCD_PIPE_CTL_0_CORE_CONFIG_HEVD_dualpipe_MASK, /* CpuId_HEVD_dualpipeMask */
#else
   (uint32_t) 0,  /* Dual pipe bit doesn't exist */
#endif

   BXVD_P_HVD_INTR2_0_CPU_SET,                     /* CPUL2InterruptSet */

   BCHP_BVNF_INTR2_0_R5F_STATUS,                  /* Bvnf_Inter2_0_R5Status */
   BCHP_BVNF_INTR2_3_HVD0_STATUS,                 /* Bvnf_Intr2_3_AvdStatus */
   BCHP_BVNF_INTR2_3_HVD0_CLEAR,                  /* Bvnf_Intr2_3_AvdClear */
   BCHP_BVNF_INTR2_3_HVD0_MASK_CLEAR,             /* Bvnf_Intr2_3_AvdMaskClear */

   (uint32_t) 0,                               /* Bvnf_Intr2_11_AvdStatus */
   (uint32_t) 0,                               /* Bvnf_Intr2_11_AvdClear */
   (uint32_t) 0,                               /* Bvnf_Intr2_11_AvdMaskClear */

   BXVD_P_INT_ID_HVD_INTR2_0_AVD_SW_INTR0,         /* PicDataRdy */
   BXVD_P_INT_ID_HVD_INTR2_0_AVD_SW_INTR3,         /* PicDataRdy1 */
   BXVD_P_INT_ID_HVD_INTR2_0_AVD_SW_INTR5,         /* PicDataRdy2 */

   BXVD_P_INT_ID_HVD_INTR2_0_AVD_MBOX_INTR,        /* Mailbox */
   BXVD_P_INT_ID_HVD_INTR2_0_AVD_SW_INTR4,         /* StereoSeqError */
   BXVD_P_INT_ID_HVD_INTR2_0_AVD_SW_INTR2,         /* StillPictureRdy */
   BXVD_P_INT_ID_HVD_INTR2_0_AVD_WATCHDOG_INTR,    /* OuterWatchdog */
   BXVD_P_INT_ID_HVD_INTR2_0_VICH_REG_INTR,        /* VICReg */
   BXVD_P_INT_ID_HVD_INTR2_0_VICH_SCB_WR_INTR,     /* VICSCBWr */
   BXVD_P_INT_ID_HVD_INTR2_0_VICH_OL_INST_RD_INTR, /* VICInstrRead */
   BXVD_P_INT_ID_HVD_INTR2_0_VICH_IL_INST_RD_INTR, /* VICILInstrRead */

   BCHP_HEVD_IL_CPU_REGS_0_INST_BASE,            /* InnerInstructionBase */
   BCHP_HEVD_IL_CPU_REGS_0_END_OF_CODE,          /* InnerEndOfCode */
   BCHP_HEVD_IL_CPU_REGS_0_GLOBAL_IO_BASE,       /* InnerGlobalIOBase */
   BCHP_HEVD_IL_CPU_REGS_0_DEBUG_CTL,            /* InnerCPUDebug */
   BCHP_HEVD_IL_CPU_DEBUG_0_AUX_REGi_ARRAY_BASE, /* InnerCPUAux */

   BCHP_HEVD_OL_CPU_REGS_0_INST_BASE,           /* OuterInstructionBase */
   BCHP_HEVD_OL_CPU_REGS_0_END_OF_CODE,         /* OuterEndOfCode */
   BCHP_HEVD_OL_CPU_REGS_0_GLOBAL_IO_BASE,      /* OuterGlobalIOBase */
   BCHP_HEVD_OL_CPU_REGS_0_DEBUG_CTL,           /* OuterCPUDebug */
   BCHP_HEVD_OL_CPU_DEBUG_0_AUX_REGi_ARRAY_BASE,/* OuterCPUAux */
   BCHP_HEVD_OL_CPU_REGS_0_CPU_INTGEN_MASK,     /* OuterInterruptMask */
   BCHP_HEVD_OL_CPU_REGS_0_CPU_INTGEN_CLR,      /* OuterInterruptClear */
   BCHP_HEVD_OL_CPU_REGS_0_WATCHDOG_TMR,        /* OuterWatchdogTimer */
   BCHP_HEVD_OL_CPU_REGS_0_CPU2HST_MBX,         /* OuterCPU2HostMailbox */
   BCHP_HEVD_OL_CPU_REGS_0_HST2CPU_MBX,         /* OuterHost2CPUMailbox */
   BCHP_HEVD_OL_CPU_REGS_0_CPU2HST_STAT,        /* OuterCPU2HostStatus */

#if BXVD_P_HEVD_DUAL_PIPE_PRESENT
   BCHP_HEVD_IL_CPU_REGS_2_0_INST_BASE,            /* Inner2InstructionBase */
   BCHP_HEVD_IL_CPU_REGS_2_0_END_OF_CODE,          /* Inner2EndOfCode */
   BCHP_HEVD_IL_CPU_REGS_2_0_GLOBAL_IO_BASE,       /* Inner2GlobalIOBase */
   BCHP_HEVD_IL_CPU_REGS_2_0_DEBUG_CTL,            /* Inner2CPUDebug */
   BCHP_HEVD_IL_CPU_DEBUG_2_0_AUX_REGi_ARRAY_BASE, /* Inner2CPUAux */
#else
   (uint32_t) 0,   /* Inner2InstructionInnerBase doesn't exist */
   (uint32_t) 0,   /* Inner2EndOfCode doesn't exist */
   (uint32_t) 0,   /* Inner2GlobalIOInnerBase doesn't exist */
   (uint32_t) 0,   /* Inner2CPUDebug doesn't */
   (uint32_t) 0,   /* Inner2CPUAux doesn't exist */
#endif

   BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CHANNEL_WR_POSITION_i_ARRAY_BASE,    /* CabacBinDepth */

   BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CTL,            /* uiDecode_CabacBinCtl */
   BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CTL_Reset_MASK, /* uiDecode_CabacBinCtl_ResetMask */
   BCHP_DECODE_SINT_0_REG_SINT_STRM_STAT,                /* uiDecode_SintStrmSts */
   BCHP_DECODE_SINT_0_REG_SINT_STRM_STAT_Rst_MASK,       /* uiDecode_SintStrmSts_ResetMask */
   BCHP_HEVD_OL_SINT_0_STRM_STAT,                        /* OLSintStrmSts */
   BCHP_HEVD_OL_SINT_0_STRM_STAT_Rst_MASK,               /* OLSintStrmSts_ResetMask */

   BCHP_DECODE_MAIN_0_REG_MAINCTL,                       /* uiDecode_MainCtl */
   BCHP_DECODE_MAIN_0_REG_MAINCTL_Rst_MASK,              /* uiDecode_MainCtl_ResetMask */
   BCHP_DCD_PIPE_CTL_0_SW_RESET_REG,                     /* SWReset */
   BCHP_DCD_PIPE_CTL_0_SW_RESET_REG_ILSI_reset_MASK,     /* SWReset_ILSIResetMask */

   BCHP_SUN_GISB_ARB_REQ_MASK,                     /* SunGisbArb_ReqMask */
   BXVD_P_GISB_ARB_REQ_MASK_HVD0_MASK,             /* SunGisbArb_ReqMaskAVDMask */

   BCHP_HEVD_PFRI_0_STRIPE_WIDTH,                       /* StripeWidth */
   BCHP_HEVD_PFRI_0_STRIPE_WIDTH_Stripe_Width_SHIFT,    /* StripeWidthShift */

   BCHP_HEVD_PFRI_0_INFO,                        /* PFRIInfo */
   BCHP_HEVD_PFRI_0_INFO_pfri_data_width_MASK,   /* PFRIInfo_DataBusWidthMask */
   BCHP_HEVD_PFRI_0_INFO_pfri_data_width_SHIFT,  /* PFRIInfo_DataBusWidthShift */
   BCHP_HEVD_PFRI_0_INFO_pfri_bank_height_MASK,  /* PFRIInfo_BankHeightMask */
   BCHP_HEVD_PFRI_0_INFO_pfri_bank_height_SHIFT, /* PFRIInfo_BankHeightShift */

#ifdef BCHP_HEVD_PFRI_0_INFO_pfri_num_banks_SHIFT
   BCHP_HEVD_PFRI_0_INFO_pfri_groupages_en_MASK , /* uiPFRIInfo_PFRIGroupagesEnaMask */
   BCHP_HEVD_PFRI_0_INFO_pfri_groupages_en_SHIFT, /* uiPFRIInfo_PFRIGroupagesEnaShift */
   BCHP_HEVD_PFRI_0_INFO_pfri_num_banks_MASK,     /* uiPFRIInfo_PFRINumBanksMask */
   BCHP_HEVD_PFRI_0_INFO_pfri_num_banks_SHIFT,    /* uiPFRIInfo_PFRINumBanksShift */
#else
   (uint32_t) 0,
   (uint32_t) 0,
   (uint32_t) 0,
   (uint32_t) 0,
#endif

   BCHP_HEVD_PCACHE_0_MODE0,             /* uiAVD_PcacheMode */
   BCHP_HEVD_PCACHE_0_MODE0_Ygran_MASK,  /* uiAVD_PcacheModeYGranMask */
   BCHP_HEVD_PCACHE_0_MODE0_Ygran_SHIFT, /* uiAVD_PcacheModeYGranShift */
   BCHP_HEVD_PCACHE_0_MODE0_Xgran_MASK,  /* uiAVD_PcacheModeXGranMask */
   BCHP_HEVD_PCACHE_0_MODE0_Xgran_SHIFT, /* uiAVD_PcacheModeXGranShift */

   /* 2nd pipe PFRI and stripe registers */
#if BXVD_P_HEVD_DUAL_PIPE_PRESENT
   BCHP_HEVD_PFRI_2_0_STRIPE_WIDTH,      /* StripeWidth_P2 */
   BCHP_HEVD_PFRI_2_0_INFO,              /* PFRIInfo_P2 */
   BCHP_HEVD_PCACHE_2_0_MODE0,           /* uiAVD_PcacheMode */
#else
   (uint32_t) 0,
   (uint32_t) 0,
   (uint32_t) 0,
#endif

   BCHP_RVC_0_CTL,   /* RVC Control */
   BCHP_RVC_0_PUT,   /* RVC Put */
   BCHP_RVC_0_GET,   /* RVC Get */
   BCHP_RVC_0_BASE,  /* RVC Base */
   BCHP_RVC_0_END,   /* RVC End */

   BCHP_HEVD_OL_CTL_0_ENTRYi_ARRAY_BASE /* ARC FW HIM base */
};

static const BXVD_P_PlatformReg_RevN0 s_stPlatformReg_AVD1RevN0 =
{
   BCHP_SUN_TOP_CTRL_SW_INIT_0_SET,                     /* SWInitSet */
   BXVD_P_SW_INIT_0_SET_hvd1_sw_init_MASK,              /* SWInitSetAVDMask */
   BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR,                   /* SWInitClear */
   BXVD_P_SW_INIT_0_CLEAR_hvd1_sw_init_MASK,            /* SWInitClearAVDMask */

   BCHP_HEVD_OL_CTL_1_SOFTSHUTDOWN_CTRL,             /* SoftShutdownCtrl */
   BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_CTRL_Enable_MASK, /* SoftShutdownCtrlMask */

#if BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS
   BCHP_HEVD_OL_CTL_1_SOFTSHUTDOWN_STATUS,           /* SoftShutdownStatus */
   BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Done_MASK, /* SoftShutdownStatusMask */
#else
   (uint32_t) 0,
   (uint32_t) 0,
#endif

   BCHP_DCD_PIPE_CTL_1_AVD_CLK_GATE,                 /* AvdClkGate */
   BCHP_HEVD_OL_CTL_1_OTP_CTL_REG,                   /* OtpCtl */
   BCHP_HEVD_OL_CTL_0_OTP_CTL_REG_disable_RV9_MASK,  /* OtpCtl_DisableRV9 */

   BCHP_DCD_PIPE_CTL_1_CORE_CONFIG,                    /* CpuId */
   BCHP_DCD_PIPE_CTL_0_CORE_CONFIG_BLD_present_MASK,   /* CpuId_BldIdMask */
#if BXVD_P_HEVD_DUAL_PIPE_PRESENT
   BCHP_DCD_PIPE_CTL_0_CORE_CONFIG_HEVD_dualpipe_MASK, /* CpuId_HEVD_dualpipeMask */
#else
   (uint32_t) 0,  /* Dual pipe bit doesn't exist */
#endif

   BXVD_P_HVD_INTR2_1_CPU_SET,                     /* CPUL2InterruptSet */

   BCHP_BVNF_INTR2_0_R5F_STATUS,                  /* Bvnf_Inter2_0_R5Status */
   BXVD_P_BVNF_INTR2_3_HVD1_STATUS,                 /* Bvnf_Intr2_3_AvdStatus */
   BXVD_P_BVNF_INTR2_3_HVD1_CLEAR,                  /* Bvnf_Intr2_3_AvdClear */
   BXVD_P_BVNF_INTR2_3_HVD1_MASK_CLEAR,             /* Bvnf_Intr2_3_AvdMaskClear */

   (uint32_t) 0,                               /* Bvnf_Intr2_11_AvdStatus */
   (uint32_t) 0,                               /* Bvnf_Intr2_11_AvdClear */
   (uint32_t) 0,                               /* Bvnf_Intr2_11_AvdMaskClear */

   BXVD_P_INT_ID_HVD_INTR2_1_AVD_SW_INTR0,          /* PicDataRdy */
   BXVD_P_INT_ID_HVD_INTR2_1_AVD_SW_INTR3,          /* PicDataRdy1 */
   BXVD_P_INT_ID_HVD_INTR2_1_AVD_SW_INTR5,          /* PicDataRdy2 */

   BXVD_P_INT_ID_HVD_INTR2_1_AVD_MBOX_INTR,         /* Mailbox */
   BXVD_P_INT_ID_HVD_INTR2_1_AVD_SW_INTR4,          /* StereoSeqError */
   BXVD_P_INT_ID_HVD_INTR2_1_AVD_SW_INTR2,          /* StillPictureRdy */
   BXVD_P_INT_ID_HVD_INTR2_1_AVD_WATCHDOG_INTR,     /* OuterWatchdog */
   BXVD_P_INT_ID_HVD_INTR2_1_VICH_REG_INTR,         /* VICReg */
   BXVD_P_INT_ID_HVD_INTR2_1_VICH_SCB_WR_INTR,      /* VICSCBWr */
   BXVD_P_INT_ID_HVD_INTR2_1_VICH_OL_INST_RD_INTR,  /* VICInstrRead */
   BXVD_P_INT_ID_HVD_INTR2_1_VICH_IL_INST_RD_INTR,  /* VICILInstrRead */

   BCHP_HEVD_IL_CPU_REGS_1_INST_BASE,            /* InnerInstructionBase */
   BCHP_HEVD_IL_CPU_REGS_1_END_OF_CODE,          /* InnerEndOfCode */
   BCHP_HEVD_IL_CPU_REGS_1_GLOBAL_IO_BASE,       /* InnerGlobalIOBase */
   BCHP_HEVD_IL_CPU_REGS_1_DEBUG_CTL,            /* InnerCPUDebug */
   BCHP_HEVD_IL_CPU_DEBUG_1_AUX_REGi_ARRAY_BASE, /* InnerCPUAux */

   BCHP_HEVD_OL_CPU_REGS_1_INST_BASE,           /* OuterInstructionBase */
   BCHP_HEVD_OL_CPU_REGS_1_END_OF_CODE,         /* OuterEndOfCode */
   BCHP_HEVD_OL_CPU_REGS_1_GLOBAL_IO_BASE,      /* OuterGlobalIOBase */
   BCHP_HEVD_OL_CPU_REGS_1_DEBUG_CTL,           /* OuterCPUDebug */
   BCHP_HEVD_OL_CPU_DEBUG_1_AUX_REGi_ARRAY_BASE,/* OuterCPUAux */
   BCHP_HEVD_OL_CPU_REGS_1_CPU_INTGEN_MASK,     /* OuterInterruptMask */
   BCHP_HEVD_OL_CPU_REGS_1_CPU_INTGEN_CLR,      /* OuterInterruptClear */
   BCHP_HEVD_OL_CPU_REGS_1_WATCHDOG_TMR,        /* OuterWatchdogTimer */
   BCHP_HEVD_OL_CPU_REGS_1_CPU2HST_MBX,         /* OuterCPU2HostMailbox */
   BCHP_HEVD_OL_CPU_REGS_1_HST2CPU_MBX,         /* OuterHost2CPUMailbox */
   BCHP_HEVD_OL_CPU_REGS_1_CPU2HST_STAT,        /* OuterCPU2HostStatus */

   (uint32_t) 0,   /* BaseInstructionBase doesn't exist on AVD only decoder */
   (uint32_t) 0,   /* BaseEndOfCode doesn't exist on AVD only decoder */
   (uint32_t) 0,   /* BaseGlobalIOBase doesn't exist on AVD only decoder */
   (uint32_t) 0,   /* BaseCPUDebug doesn't exist on AVD only decoder */
   (uint32_t) 0,   /* BaseCPUAux doesn't exist on AVD only decoder */

   BCHP_REG_CABAC2BINS2_1_REG_CABAC2BINS_CHANNEL_WR_POSITION_i_ARRAY_BASE,    /* CabacBinDepth */

   BCHP_REG_CABAC2BINS2_1_REG_CABAC2BINS_CTL,            /* uiDecode_CabacBinCtl */
   BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CTL_Reset_MASK, /* uiDecode_CabacBinCtl_ResetMask */
   BCHP_DECODE_SINT_1_REG_SINT_STRM_STAT,                /* uiDecode_SintStrmSts */
   BCHP_DECODE_SINT_0_REG_SINT_STRM_STAT_Rst_MASK,       /* uiDecode_SintStrmSts_ResetMask */
   BCHP_HEVD_OL_SINT_1_STRM_STAT,                        /* OLSintStrmSts */
   BCHP_HEVD_OL_SINT_0_STRM_STAT_Rst_MASK,               /* OLSintStrmSts_ResetMask */

   BCHP_DECODE_MAIN_1_REG_MAINCTL,                       /* uiDecode_MainCtl */
   BCHP_DECODE_MAIN_0_REG_MAINCTL_Rst_MASK,              /* uiDecode_MainCtl_ResetMask */
   BCHP_DCD_PIPE_CTL_1_SW_RESET_REG,                     /* SWReset */
   BCHP_DCD_PIPE_CTL_0_SW_RESET_REG_ILSI_reset_MASK,     /* SWReset_ILSIResetMask */

   BCHP_SUN_GISB_ARB_REQ_MASK,                    /* SunGisbArb_ReqMask */
   BXVD_P_GISB_ARB_REQ_MASK_HVD1_MASK,            /* SunGisbArb_ReqMaskAVDMask */

   BCHP_HEVD_PFRI_1_STRIPE_WIDTH,                       /* StripeWidth */
   BCHP_HEVD_PFRI_0_STRIPE_WIDTH_Stripe_Width_SHIFT,    /* StripeWidthShift */

   BCHP_HEVD_PFRI_1_INFO,                        /* PFRIInfo */
   BCHP_HEVD_PFRI_0_INFO_pfri_data_width_MASK,   /* PFRIInfo_DataBusWidthMask */
   BCHP_HEVD_PFRI_0_INFO_pfri_data_width_SHIFT,  /* PFRIInfo_DataBusWidthShift */
   BCHP_HEVD_PFRI_0_INFO_pfri_bank_height_MASK,  /* PFRIInfo_BankHeightMask */
   BCHP_HEVD_PFRI_0_INFO_pfri_bank_height_SHIFT, /* PFRIInfo_BankHeightShift */

#ifdef BCHP_HEVD_PFRI_0_INFO_pfri_num_banks_SHIFT
   BCHP_HEVD_PFRI_0_INFO_pfri_groupages_en_MASK , /* uiPFRIInfo_PFRIGroupagesEnaMask */
   BCHP_HEVD_PFRI_0_INFO_pfri_groupages_en_SHIFT, /* uiPFRIInfo_PFRIGroupagesEnaShift */
   BCHP_HEVD_PFRI_0_INFO_pfri_num_banks_MASK,     /* uiPFRIInfo_PFRINumBanksMask */
   BCHP_HEVD_PFRI_0_INFO_pfri_num_banks_SHIFT,    /* uiPFRIInfo_PFRINumBanksShift */
#else
   (uint32_t) 0,
   (uint32_t) 0,
   (uint32_t) 0,
   (uint32_t) 0,
#endif

   BCHP_HEVD_PCACHE_1_MODE0,             /* uiAVD_PcacheMode */
   BCHP_HEVD_PCACHE_0_MODE0_Ygran_MASK,  /* uiAVD_PcacheModeYGranMask */
   BCHP_HEVD_PCACHE_0_MODE0_Ygran_SHIFT, /* uiAVD_PcacheModeYGranShift */
   BCHP_HEVD_PCACHE_0_MODE0_Xgran_MASK,  /* uiAVD_PcacheModeXGranMask */
   BCHP_HEVD_PCACHE_0_MODE0_Xgran_SHIFT, /* uiAVD_PcacheModeXGranShift */

   /* 2nd pipe PFRI and stripe registers */
   (uint32_t) 0,
   (uint32_t) 0,
   (uint32_t) 0,

   BCHP_RVC_1_CTL,   /* RVC Control */
   BCHP_RVC_1_PUT,   /* RVC Put */
   BCHP_RVC_1_GET,   /* RVC Get */
   BCHP_RVC_1_BASE,  /* RVC Base */
   BCHP_RVC_1_END,   /* RVC End */

   BCHP_HEVD_OL_CTL_1_ENTRYi_ARRAY_BASE /* ARC FW HIM base */
};

#elif BXVD_P_MIPS_CORE

static const BXVD_P_PlatformReg_RevN0 s_stPlatformReg_HVD0RevN0 =
{
   BCHP_SUN_TOP_CTRL_SW_INIT_0_SET,                     /* SWInitSet */
   BXVD_P_SW_INIT_0_SET_hvd0_sw_init_MASK,              /* SWInitSetAVDMask */
   BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR,                   /* SWInitClear */
   BXVD_P_SW_INIT_0_CLEAR_hvd0_sw_init_MASK,            /* SWInitClearAVDMask */

   BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_CTRL,             /* SoftShutdownCtrl */
   BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_CTRL_Enable_MASK, /* SoftShutdownCtrlMask */

#if BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS
   BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS,           /* SoftShutdownStatus */
   BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Done_MASK, /* SoftShutdownStatusMask */
#else
   (uint32_t) 0,
   (uint32_t) 0,
#endif

   BCHP_DCD_PIPE_CTL_0_AVD_CLK_GATE,                 /* AvdClkGate */
   BCHP_HEVD_OL_CTL_0_OTP_CTL_REG,                   /* OtpCtl */
   BCHP_HEVD_OL_CTL_0_OTP_CTL_REG_disable_RV9_MASK,  /* OtpCtl_DisableRV9 */

   BCHP_DCD_PIPE_CTL_0_CORE_CONFIG,                    /* CpuId */
   BCHP_DCD_PIPE_CTL_0_CORE_CONFIG_BLD_present_MASK,   /* CpuId_BldIdMask */

#if BXVD_P_HEVD_DUAL_PIPE_PRESENT
   BCHP_DCD_PIPE_CTL_0_CORE_CONFIG_HEVD_dualpipe_MASK, /* CpuId_HEVD_dualpipeMask */
#else
   (uint32_t) 0,  /* Dual pipe bit doesn't exist */
#endif

   BXVD_P_HVD_INTR2_0_CPU_SET,                    /* CPUL2InterruptSet */

   BCHP_BVNF_INTR2_0_R5F_STATUS,                  /* Bvnf_Inter2_0_R5Status */

   BXVD_P_BVNF_INTR2_3_HVD0_STATUS,               /* Bvnf_Intr2_3_AvdStatus */
   BXVD_P_BVNF_INTR2_3_HVD0_CLEAR,                /* Bvnf_Intr2_3_AvdClear */
   BXVD_P_BVNF_INTR2_3_HVD0_MASK_CLEAR,           /* Bvnf_Intr2_3_AvdMaskClear */

   (uint32_t) 0,                               /* Bvnf_Intr2_11_AvdStatus */
   (uint32_t) 0,                               /* Bvnf_Intr2_11_AvdClear */
   (uint32_t) 0,                               /* Bvnf_Intr2_11_AvdMaskClear */

   BCHP_INT_ID_AVD_SW_INTR0,                   /* PicDataRdy */
   BCHP_INT_ID_AVD_SW_INTR3,                   /* PicDataRdy1 */
   BCHP_INT_ID_AVD_SW_INTR5,                   /* PicDataRdy2 */

   BCHP_INT_ID_AVD_MBOX_INTR,                  /* Mailbox */
   BCHP_INT_ID_AVD_SW_INTR4,                   /* StereoSeqError */
   BCHP_INT_ID_AVD_SW_INTR2,                   /* StillPictureRdy */
   BCHP_INT_ID_AVD_WATCHDOG_INTR,              /* OuterWatchdog */
   BCHP_INT_ID_VICH_REG_INTR,                  /* VICReg */
   BCHP_INT_ID_VICH_SCB_WR_INTR,               /* VICSCBWr */
   BCHP_INT_ID_VICH_OL_INST_RD_INTR,           /* VICInstrRead */
   BCHP_INT_ID_VICH_IL_INST_RD_INTR,           /* VICILInstrRead */

   BCHP_HEVD_IL_CPU_REGS_0_INST_BASE,            /* InnerInstructionBase */
   BCHP_HEVD_IL_CPU_REGS_0_END_OF_CODE,          /* InnerEndOfCode */
   BCHP_HEVD_IL_CPU_REGS_0_GLOBAL_IO_BASE,       /* InnerGlobalIOBase */
   BCHP_HEVD_IL_CPU_REGS_0_DEBUG_CTL,            /* InnerCPUDebug */
   BCHP_HEVD_IL_CPU_DEBUG_0_AUX_REGi_ARRAY_BASE, /* InnerCPUAux */

   BCHP_HEVD_OL_CPU_REGS_0_INST_BASE,           /* OuterInstructionBase */
   BCHP_HEVD_OL_CPU_REGS_0_END_OF_CODE,         /* OuterEndOfCode */
   BCHP_HEVD_OL_CPU_REGS_0_GLOBAL_IO_BASE,      /* OuterGlobalIOBase */
   BCHP_HEVD_OL_CPU_REGS_0_DEBUG_CTL,           /* OuterCPUDebug */
   BCHP_HEVD_OL_CPU_DEBUG_0_AUX_REGi_ARRAY_BASE,/* OuterCPUAux */
   BCHP_HEVD_OL_CPU_REGS_0_CPU_INTGEN_MASK,     /* OuterInterruptMask */
   BCHP_HEVD_OL_CPU_REGS_0_CPU_INTGEN_CLR,      /* OuterInterruptClear */
   BCHP_HEVD_OL_CPU_REGS_0_WATCHDOG_TMR,        /* OuterWatchdogTimer */
   BCHP_HEVD_OL_CPU_REGS_0_CPU2HST_MBX,         /* OuterCPU2HostMailbox */
   BCHP_HEVD_OL_CPU_REGS_0_HST2CPU_MBX,         /* OuterHost2CPUMailbox */
   BCHP_HEVD_OL_CPU_REGS_0_CPU2HST_STAT,        /* OuterCPU2HostStatus */

#if BXVD_P_HEVD_DUAL_PIPE_PRESENT
   BCHP_HEVD_IL_CPU_REGS_2_0_INST_BASE,            /* Inner2InstructionBase */
   BCHP_HEVD_IL_CPU_REGS_2_0_END_OF_CODE,          /* Inner2EndOfCode */
   BCHP_HEVD_IL_CPU_REGS_2_0_GLOBAL_IO_BASE,       /* Inner2GlobalIOBase */
   BCHP_HEVD_IL_CPU_REGS_2_0_DEBUG_CTL,            /* Inner2CPUDebug */
   BCHP_HEVD_IL_CPU_DEBUG_2_0_AUX_REGi_ARRAY_BASE, /* Inner2CPUAux */
#else
   (uint32_t) 0,   /* BaseInstructionBase doesn't exist on AVD only decoder */
   (uint32_t) 0,   /* BaseEndOfCode doesn't exist on AVD only decoder */
   (uint32_t) 0,   /* BaseGlobalIOBase doesn't exist on AVD only decoder */
   (uint32_t) 0,   /* BaseCPUDebug doesn't exist on AVD only decoder */
   (uint32_t) 0,   /* BaseCPUAux doesn't exist on AVD only decoder */
#endif

   BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CHANNEL_WR_POSITION_i_ARRAY_BASE,    /* CabacBinDepth */

   BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CTL,            /* uiDecode_CabacBinCtl */
   BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CTL_Reset_MASK, /* uiDecode_CabacBinCtl_ResetMask */
   BCHP_DECODE_SINT_0_REG_SINT_STRM_STAT,                /* uiDecode_SintStrmSts */
   BCHP_DECODE_SINT_0_REG_SINT_STRM_STAT_Rst_MASK,       /* uiDecode_SintStrmSts_ResetMask */
   BCHP_HEVD_OL_SINT_0_STRM_STAT,                        /* OLSintStrmSts */
   BCHP_HEVD_OL_SINT_0_STRM_STAT_Rst_MASK,               /* OLSintStrmSts_ResetMask */

   BCHP_DECODE_MAIN_0_REG_MAINCTL,                       /* uiDecode_MainCtl */
   BCHP_DECODE_MAIN_0_REG_MAINCTL_Rst_MASK,              /* uiDecode_MainCtl_ResetMask */
   BCHP_DCD_PIPE_CTL_0_SW_RESET_REG,                     /* SWReset */
   BCHP_DCD_PIPE_CTL_0_SW_RESET_REG_ILSI_reset_MASK,     /* SWReset_ILSIResetMask */

   BCHP_SUN_GISB_ARB_REQ_MASK,                          /* SunGisbArb_ReqMask */
   BXVD_P_GISB_ARB_REQ_MASK_hvd_0_MASK,                 /* SunGisbArb_ReqMaskAVDMask */

   BCHP_HEVD_PFRI_0_STRIPE_WIDTH,                       /* StripeWidth */
   BCHP_HEVD_PFRI_0_STRIPE_WIDTH_Stripe_Width_SHIFT,    /* StripeWidthShift */

   BCHP_HEVD_PFRI_0_INFO,                        /* PFRIInfo */
   BCHP_HEVD_PFRI_0_INFO_pfri_data_width_MASK,   /* PFRIInfo_DataBusWidthMask */
   BCHP_HEVD_PFRI_0_INFO_pfri_data_width_SHIFT,  /* PFRIInfo_DataBusWidthShift */
   BCHP_HEVD_PFRI_0_INFO_pfri_bank_height_MASK,  /* PFRIInfo_BankHeightMask */
   BCHP_HEVD_PFRI_0_INFO_pfri_bank_height_SHIFT, /* PFRIInfo_BankHeightShift */

#ifdef BCHP_HEVD_PFRI_0_INFO_pfri_num_banks_SHIFT
   BCHP_HEVD_PFRI_0_INFO_pfri_groupages_en_MASK , /* uiPFRIInfo_PFRIGroupagesEnaMask */
   BCHP_HEVD_PFRI_0_INFO_pfri_groupages_en_SHIFT, /* uiPFRIInfo_PFRIGroupagesEnaShift */
   BCHP_HEVD_PFRI_0_INFO_pfri_num_banks_MASK,     /* uiPFRIInfo_PFRINumBanksMask */
   BCHP_HEVD_PFRI_0_INFO_pfri_num_banks_SHIFT,    /* uiPFRIInfo_PFRINumBanksShift */
#else
   (uint32_t) 0,
   (uint32_t) 0,
   (uint32_t) 0,
   (uint32_t) 0,
#endif

   BCHP_HEVD_PCACHE_0_MODE0,             /* uiAVD_PcacheMode */
   BCHP_HEVD_PCACHE_0_MODE0_Ygran_MASK,  /* uiAVD_PcacheModeYGranMask */
   BCHP_HEVD_PCACHE_0_MODE0_Ygran_SHIFT, /* uiAVD_PcacheModeYGranShift */
   BCHP_HEVD_PCACHE_0_MODE0_Xgran_MASK,  /* uiAVD_PcacheModeXGranMask */
   BCHP_HEVD_PCACHE_0_MODE0_Xgran_SHIFT, /* uiAVD_PcacheModeXGranShift */

   /* 2nd pipe PFRI and stripe registers */
#if BXVD_P_HEVD_DUAL_PIPE_PRESENT
   BCHP_HEVD_PFRI_2_0_STRIPE_WIDTH,      /* StripeWidth_P2 */
   BCHP_HEVD_PFRI_2_0_INFO,              /* PFRIInfo_P2 */
   BCHP_HEVD_PCACHE_2_0_MODE0,           /* uiAVD_PcacheMode */
#else
   (uint32_t) 0,
   (uint32_t) 0,
   (uint32_t) 0,
#endif

   BCHP_RVC_0_CTL,   /* RVC Control */
   BCHP_RVC_0_PUT,   /* RVC Put */
   BCHP_RVC_0_GET,   /* RVC Get */
   BCHP_RVC_0_BASE,  /* RVC Base */
   BCHP_RVC_0_END,   /* RVC End */

   BCHP_HEVD_OL_CTL_0_ENTRYi_ARRAY_BASE /* ARC FW HIM base */
};

#else /* Single video core, ARM CPU */

static const BXVD_P_PlatformReg_RevN0 s_stPlatformReg_HVD0RevN0 =
{
   BCHP_SUN_TOP_CTRL_SW_INIT_0_SET,                     /* SWInitSet */
   BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_hvd0_sw_init_MASK,   /* SWInitSetAVDMask */
   BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR,                   /* SWInitClear */
   BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR_hvd0_sw_init_MASK, /* SWInitClearAVDMask */

   BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_CTRL,             /* SoftShutdownCtrl */
   BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_CTRL_Enable_MASK, /* SoftShutdownCtrlMask */

#if BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS
   BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS,           /* SoftShutdownStatus */
   BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Done_MASK, /* SoftShutdownStatusMask */
#else
   (uint32_t) 0,
   (uint32_t) 0,
#endif

   BCHP_DCD_PIPE_CTL_0_AVD_CLK_GATE,                 /* AvdClkGate */
   BCHP_HEVD_OL_CTL_0_OTP_CTL_REG,                   /* OtpCtl */
   BCHP_HEVD_OL_CTL_0_OTP_CTL_REG_disable_RV9_MASK,  /* OtpCtl_DisableRV9 */

   BCHP_DCD_PIPE_CTL_0_CORE_CONFIG,                    /* CpuId */
   BCHP_DCD_PIPE_CTL_0_CORE_CONFIG_BLD_present_MASK,   /* CpuId_BldIdMask */

#if BXVD_P_HEVD_DUAL_PIPE_PRESENT
   BCHP_DCD_PIPE_CTL_0_CORE_CONFIG_HEVD_dualpipe_MASK, /* CpuId_HEVD_dualpipeMask */
#else
   (uint32_t) 0,  /* Dual pipe bit doesn't exist */
#endif

   BXVD_P_HVD_INTR2_0_CPU_SET,                    /* CPUL2InterruptSet */

   BCHP_BVNF_INTR2_0_R5F_STATUS,                  /* Bvnf_Inter2_0_R5Status */

#if BXVD_P_USE_BVNF_INTR_HVD1
   BCHP_BVNF_INTR2_3_HVD1_STATUS,                 /* Bvnf_Intr2_3_AvdStatus */
   BCHP_BVNF_INTR2_3_HVD1_CLEAR,                  /* Bvnf_Intr2_3_AvdClear */
   BCHP_BVNF_INTR2_3_HVD1_MASK_CLEAR,             /* Bvnf_Intr2_3_AvdMaskClear */
#else
   BCHP_BVNF_INTR2_3_HVD0_STATUS,                 /* Bvnf_Intr2_3_AvdStatus */
   BCHP_BVNF_INTR2_3_HVD0_CLEAR,                  /* Bvnf_Intr2_3_AvdClear */
   BCHP_BVNF_INTR2_3_HVD0_MASK_CLEAR,             /* Bvnf_Intr2_3_AvdMaskClear */
#endif

   (uint32_t) 0,                               /* Bvnf_Intr2_11_AvdStatus */
   (uint32_t) 0,                               /* Bvnf_Intr2_11_AvdClear */
   (uint32_t) 0,                               /* Bvnf_Intr2_11_AvdMaskClear */

   BCHP_INT_ID_AVD_SW_INTR0,                   /* PicDataRdy */
   BCHP_INT_ID_AVD_SW_INTR3,                   /* PicDataRdy1 */
   BCHP_INT_ID_AVD_SW_INTR5,                   /* PicDataRdy2 */

   BCHP_INT_ID_AVD_MBOX_INTR,                  /* Mailbox */
   BCHP_INT_ID_AVD_SW_INTR4,                   /* StereoSeqError */
   BCHP_INT_ID_AVD_SW_INTR2,                   /* StillPictureRdy */
   BCHP_INT_ID_AVD_WATCHDOG_INTR,              /* OuterWatchdog */
   BCHP_INT_ID_VICH_REG_INTR,                  /* VICReg */
   BCHP_INT_ID_VICH_SCB_WR_INTR,               /* VICSCBWr */
   BCHP_INT_ID_VICH_OL_INST_RD_INTR,           /* VICInstrRead */
   BCHP_INT_ID_VICH_IL_INST_RD_INTR,           /* VICILInstrRead */

   BCHP_HEVD_IL_CPU_REGS_0_INST_BASE,            /* InnerInstructionBase */
   BCHP_HEVD_IL_CPU_REGS_0_END_OF_CODE,          /* InnerEndOfCode */
   BCHP_HEVD_IL_CPU_REGS_0_GLOBAL_IO_BASE,       /* InnerGlobalIOBase */
   BCHP_HEVD_IL_CPU_REGS_0_DEBUG_CTL,            /* InnerCPUDebug */
   BCHP_HEVD_IL_CPU_DEBUG_0_AUX_REGi_ARRAY_BASE, /* InnerCPUAux */

   BCHP_HEVD_OL_CPU_REGS_0_INST_BASE,           /* OuterInstructionBase */
   BCHP_HEVD_OL_CPU_REGS_0_END_OF_CODE,         /* OuterEndOfCode */
   BCHP_HEVD_OL_CPU_REGS_0_GLOBAL_IO_BASE,      /* OuterGlobalIOBase */
   BCHP_HEVD_OL_CPU_REGS_0_DEBUG_CTL,           /* OuterCPUDebug */
   BCHP_HEVD_OL_CPU_DEBUG_0_AUX_REGi_ARRAY_BASE,/* OuterCPUAux */
   BCHP_HEVD_OL_CPU_REGS_0_CPU_INTGEN_MASK,     /* OuterInterruptMask */
   BCHP_HEVD_OL_CPU_REGS_0_CPU_INTGEN_CLR,      /* OuterInterruptClear */
   BCHP_HEVD_OL_CPU_REGS_0_WATCHDOG_TMR,        /* OuterWatchdogTimer */
   BCHP_HEVD_OL_CPU_REGS_0_CPU2HST_MBX,         /* OuterCPU2HostMailbox */
   BCHP_HEVD_OL_CPU_REGS_0_HST2CPU_MBX,         /* OuterHost2CPUMailbox */
   BCHP_HEVD_OL_CPU_REGS_0_CPU2HST_STAT,        /* OuterCPU2HostStatus */

#if BXVD_P_HEVD_DUAL_PIPE_PRESENT
   BCHP_HEVD_IL_CPU_REGS_2_0_INST_BASE,            /* Inner2InstructionBase */
   BCHP_HEVD_IL_CPU_REGS_2_0_END_OF_CODE,          /* Inner2EndOfCode */
   BCHP_HEVD_IL_CPU_REGS_2_0_GLOBAL_IO_BASE,       /* Inner2GlobalIOBase */
   BCHP_HEVD_IL_CPU_REGS_2_0_DEBUG_CTL,            /* Inner2CPUDebug */
   BCHP_HEVD_IL_CPU_DEBUG_2_0_AUX_REGi_ARRAY_BASE, /* Inner2CPUAux */
#else
   (uint32_t) 0,   /* BaseInstructionBase doesn't exist on AVD only decoder */
   (uint32_t) 0,   /* BaseEndOfCode doesn't exist on AVD only decoder */
   (uint32_t) 0,   /* BaseGlobalIOBase doesn't exist on AVD only decoder */
   (uint32_t) 0,   /* BaseCPUDebug doesn't exist on AVD only decoder */
   (uint32_t) 0,   /* BaseCPUAux doesn't exist on AVD only decoder */
#endif

   BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CHANNEL_WR_POSITION_i_ARRAY_BASE,    /* CabacBinDepth */

   BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CTL,            /* uiDecode_CabacBinCtl */
   BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CTL_Reset_MASK, /* uiDecode_CabacBinCtl_ResetMask */
   BCHP_DECODE_SINT_0_REG_SINT_STRM_STAT,                /* uiDecode_SintStrmSts */
   BCHP_DECODE_SINT_0_REG_SINT_STRM_STAT_Rst_MASK,       /* uiDecode_SintStrmSts_ResetMask */
   BCHP_HEVD_OL_SINT_0_STRM_STAT,                        /* OLSintStrmSts */
   BCHP_HEVD_OL_SINT_0_STRM_STAT_Rst_MASK,               /* OLSintStrmSts_ResetMask */

   BCHP_DECODE_MAIN_0_REG_MAINCTL,                       /* uiDecode_MainCtl */
   BCHP_DECODE_MAIN_0_REG_MAINCTL_Rst_MASK,              /* uiDecode_MainCtl_ResetMask */
   BCHP_DCD_PIPE_CTL_0_SW_RESET_REG,                     /* SWReset */
   BCHP_DCD_PIPE_CTL_0_SW_RESET_REG_ILSI_reset_MASK,     /* SWReset_ILSIResetMask */

   BCHP_SUN_GISB_ARB_REQ_MASK,                          /* SunGisbArb_ReqMask */
   BCHP_SUN_GISB_ARB_REQ_MASK_hvd_0_MASK,               /* SunGisbArb_ReqMaskAVDMask */

   BCHP_HEVD_PFRI_0_STRIPE_WIDTH,                       /* StripeWidth */
   BCHP_HEVD_PFRI_0_STRIPE_WIDTH_Stripe_Width_SHIFT,    /* StripeWidthShift */

   BCHP_HEVD_PFRI_0_INFO,                        /* PFRIInfo */
   BCHP_HEVD_PFRI_0_INFO_pfri_data_width_MASK,   /* PFRIInfo_DataBusWidthMask */
   BCHP_HEVD_PFRI_0_INFO_pfri_data_width_SHIFT,  /* PFRIInfo_DataBusWidthShift */
   BCHP_HEVD_PFRI_0_INFO_pfri_bank_height_MASK,  /* PFRIInfo_BankHeightMask */
   BCHP_HEVD_PFRI_0_INFO_pfri_bank_height_SHIFT, /* PFRIInfo_BankHeightShift */

#ifdef BCHP_HEVD_PFRI_0_INFO_pfri_num_banks_SHIFT
   BCHP_HEVD_PFRI_0_INFO_pfri_groupages_en_MASK , /* uiPFRIInfo_PFRIGroupagesEnaMask */
   BCHP_HEVD_PFRI_0_INFO_pfri_groupages_en_SHIFT, /* uiPFRIInfo_PFRIGroupagesEnaShift */
   BCHP_HEVD_PFRI_0_INFO_pfri_num_banks_MASK,     /* uiPFRIInfo_PFRINumBanksMask */
   BCHP_HEVD_PFRI_0_INFO_pfri_num_banks_SHIFT,    /* uiPFRIInfo_PFRINumBanksShift */
#else
   (uint32_t) 0,
   (uint32_t) 0,
   (uint32_t) 0,
   (uint32_t) 0,
#endif

   BCHP_HEVD_PCACHE_0_MODE0,             /* uiAVD_PcacheMode */
   BCHP_HEVD_PCACHE_0_MODE0_Ygran_MASK,  /* uiAVD_PcacheModeYGranMask */
   BCHP_HEVD_PCACHE_0_MODE0_Ygran_SHIFT, /* uiAVD_PcacheModeYGranShift */
   BCHP_HEVD_PCACHE_0_MODE0_Xgran_MASK,  /* uiAVD_PcacheModeXGranMask */
   BCHP_HEVD_PCACHE_0_MODE0_Xgran_SHIFT, /* uiAVD_PcacheModeXGranShift */

   /* 2nd pipe PFRI and stripe registers */
#if BXVD_P_HEVD_DUAL_PIPE_PRESENT
   BCHP_HEVD_PFRI_2_0_STRIPE_WIDTH,      /* StripeWidth_P2 */
   BCHP_HEVD_PFRI_2_0_INFO,              /* PFRIInfo_P2 */
   BCHP_HEVD_PCACHE_2_0_MODE0,           /* uiAVD_PcacheMode */
#else
   (uint32_t) 0,
   (uint32_t) 0,
   (uint32_t) 0,
#endif

   BCHP_RVC_0_CTL,   /* RVC Control */
   BCHP_RVC_0_PUT,   /* RVC Put */
   BCHP_RVC_0_GET,   /* RVC Get */
   BCHP_RVC_0_BASE,  /* RVC Base */
   BCHP_RVC_0_END,   /* RVC End */

   BCHP_HEVD_OL_CTL_0_ENTRYi_ARRAY_BASE /* ARC FW HIM base */
};

#endif
#endif

#if BXVD_P_USE_READ_DISPLAY_INFO_HIM_API_REVN0

void BXVD_P_ReadDisplayInfo_HIM_API_isr(BXVD_Handle hXvd,
                                        uint32_t uiDisplayInfoOffset,
                                        BXVD_P_DisplayInfo *pstDisplayInfo)
{
   uint32_t i, byteOffset;

   byteOffset = uiDisplayInfoOffset;

   for (i = 0; i < BXVD_P_STC_MAX; i++)
   {
      BXVD_P_READ_HIM(hXvd, byteOffset, pstDisplayInfo->stc_snapshot[i]);
      byteOffset+=4;
   }

   BXVD_P_READ_HIM(hXvd, byteOffset, pstDisplayInfo->vsync_parity);
   byteOffset+=4;
   BXVD_P_READ_HIM(hXvd, byteOffset, pstDisplayInfo->vsync_parity_1);
}

bool BXVD_P_IsDisplayInfoEqual_HIM_API_isr(BXVD_P_DisplayInfo stDisplayInfo,
                                           BXVD_P_DisplayInfo stDisplayInfo1)
{
   uint32_t i;
   bool bDisplayInfoEqual = true;

   for (i = 0; i < BXVD_P_STC_MAX; i++)
   {
      if (stDisplayInfo.stc_snapshot[i] != stDisplayInfo1.stc_snapshot[i])
      {
         bDisplayInfoEqual = false;
         break;
      }
   }

   if (stDisplayInfo.vsync_parity != stDisplayInfo1.vsync_parity)
   {
      bDisplayInfoEqual = false;
   }

   if (stDisplayInfo.vsync_parity_1 != stDisplayInfo1.vsync_parity_1)
   {
      bDisplayInfoEqual = false;
   }

   return bDisplayInfoEqual;
}

#endif  /* BXVD_P_USE_READ_DISPLAY_INFO_HIM_API_REVN0 */

#ifdef BXVD_P_USE_DETERMINE_STRIPE_INFO_REVN0

void BXVD_P_DetermineNonGroupingStripeInfo(BCHP_DramType ddrType,
                                           uint32_t uiMemPartSize,
                                           uint32_t uiMemBusWidth,
                                           uint32_t *puiStripeWidth,
                                           uint32_t *puiBankHeight);

void BXVD_P_DetermineGroupingStripeInfo(BCHP_DramType ddrType,
                                        uint32_t uiMemPartSize,
                                        uint32_t uiMemBusWidth,
                                        uint32_t uiMemDeviceWidth,
                                        bool     bDDRGroupageEnabled,
                                        uint32_t *puiStripeWidth,
                                        uint32_t *puiBankHeight);


void BXVD_P_DetermineStripeInfo_RevN0( BCHP_DramType ddrType,
                                       uint32_t uiMemPartSize,
                                       uint32_t uiMemBusWidth,
                                       uint32_t uiMemDeviceWidth,
                                       bool     bDDRGroupageEnabled,
                                       uint32_t *puiStripeWidth,
                                       uint32_t *puiBankHeight)
{

   BSTD_UNUSED(uiMemDeviceWidth);
#if  BXVD_P_HEVD_PFRI_DEBUG_PFRI_GROUPING_PRESENT
   BXVD_P_DetermineGroupingStripeInfo(ddrType, uiMemPartSize, uiMemBusWidth, uiMemDeviceWidth,
                                      bDDRGroupageEnabled, puiStripeWidth, puiBankHeight);
#else
   BSTD_UNUSED(uiMemDeviceWidth);
   BSTD_UNUSED(bDDRGroupageEnabled);

   BXVD_P_DetermineNonGroupingStripeInfo(ddrType, uiMemPartSize, uiMemBusWidth,
                                            puiStripeWidth, puiBankHeight);
#endif

}
#endif /* BXVD_P_USE_DETERMINE_STRIPE_INFO_REVN0 */

#ifdef BXVD_P_USE_INIT_REG_PTRS_REVN0

void BXVD_P_DetermineNonGroupingStripeInfo(BCHP_DramType ddrType,
                                           uint32_t uiMemPartSize,
                                           uint32_t uiMemBusWidth,
                                           uint32_t *puiStripeWidth,
                                           uint32_t *puiBankHeight)
{
   /* stripeWidth reg values: 0 - 64, 1 - 128, 2 - 256 bytes */
   switch(ddrType)
   {
      case BCHP_DramType_eDDR3:

         /* Set stripe width and bank height base on bus width and memory part size */
         switch(uiMemBusWidth)
         {
            /* Dram bus width 16 */
            case 16:

               *puiBankHeight = (uint32_t)BXVD_P_PFRI_Bank_Height_8n3;

#if BXVD_P_MIPS_CORE
               BDBG_MSG(("StripeWidth: 64 bytes"));
               *puiStripeWidth = BXVD_P_PFRI_Stripe_Width_e64Bytes;
#else
               BDBG_MSG(("StripeWidth: 128 bytes"));
               *puiStripeWidth = BXVD_P_PFRI_Stripe_Width_e128Bytes;
#endif
               break;

            /* Dram bus width 32 */
            case 32:

               switch(uiMemPartSize)
               {
                  case 512:  /* 512 Mbits Device Tech*/
                  case 1024: /* 1024 Mbits Device Tech*/
                     *puiBankHeight = (uint32_t)BXVD_P_PFRI_Bank_Height_8n3;
                     BDBG_MSG(("buswidth 32 BankHeight:: 8n3\n"));
                     break;

                  case 2048: /* 2048 Mbits Device Tech*/
                  case 4096: /* 4096 Mbits Device Tech*/
                  case 8192: /* 8192 Mbits Device Tech*/
                     *puiBankHeight = (uint32_t)BXVD_P_PFRI_Bank_Height_16n6;
                     BDBG_MSG(("buswidth 32 BankHeight:: 16n6\n"));
                     break;

                  default:
                     *puiBankHeight  = (uint32_t)BXVD_P_PFRI_Bank_Height_8n3;
                     BDBG_ERR(("Unknown memory part size: %d", uiMemPartSize));
                     break;
               }

               *puiStripeWidth = BXVD_P_PFRI_Stripe_Width_e128Bytes;
               BDBG_MSG(("StripeWidth: 128 bytes"));

               break;

            default:

#if BXVD_P_MIPS_CORE
               BDBG_MSG(("StripeWidth: 64 bytes"));
               *puiStripeWidth = BXVD_P_PFRI_Stripe_Width_e64Bytes;
#else
               BDBG_MSG(("StripeWidth: 128 bytes"));
               *puiStripeWidth = BXVD_P_PFRI_Stripe_Width_e128Bytes;
#endif
               *puiBankHeight  = (uint32_t)BXVD_P_PFRI_Bank_Height_8n3;
               BDBG_ERR(("Unknown memory bus width: %d", uiMemBusWidth));

               break;
         }

         break;

      case BCHP_DramType_eDDR4:

         *puiBankHeight = (uint32_t)BXVD_P_PFRI_Bank_Height_16n6;

         if (uiMemBusWidth == 16)
         {
            *puiStripeWidth = BXVD_P_PFRI_Stripe_Width_e128Bytes;
         }
         else
         {
            *puiStripeWidth = BXVD_P_PFRI_Stripe_Width_e256Bytes;
         }

         break;

      case BCHP_DramType_eGDDR5:
      default:

         BDBG_ERR(("Unsupported DDR type, DDR4 or DDR5"));
         break;
   }
}


void BXVD_P_DetermineNonGroupagePCacheSettings(BXVD_Handle hXvd,
                                               BCHP_DramType ddrType,
                                               uint32_t uiMemPartSize,
                                               uint32_t uiMemBusWidth)
{
   uint32_t uiDataWidth;

   uint32_t stripeWidth=0;
   uint32_t bankHeight=0;
   uint32_t uiXGran=0;
   uint32_t uiYGran=0;

   uint32_t uiReg;
   uint32_t uiPCacheVal;

   switch(ddrType)
   {
      case BCHP_DramType_eDDR3:

         /* Set stripe width and bank height base on bus width and memory part size */
         switch(uiMemBusWidth)
         {
            /* Dram bus width 16 */
            case 16:

#if BXVD_P_PRE_REVQ2_CORE
               uiYGran = BXVD_P_PCache_YGran_e1Line;
#else
               uiYGran = BXVD_P_PCache_YGran_e2Lines;
#endif
               break;

            /* Dram bus width 32 */
            case 32:
               uiYGran = BXVD_P_PCache_YGran_e2Lines;
               BDBG_MSG(("buswidth 32 yGran:e2Liines:\n"));

               break;
         }

         break;

      case BCHP_DramType_eDDR4:

         if (uiMemBusWidth == 16)
         {
            uiYGran = BXVD_P_PCache_YGran_e1Line;
         }
         else
         {
            uiYGran = BXVD_P_PCache_YGran_e2Lines;
         }

         break;

      default:

         BDBG_ERR(("Unsupported DDR type, DDR2 or DDR5"));
         break;
   }

   uiXGran = BXVD_P_PCache_XGran_e16Bytes;

   BXVD_P_DetermineNonGroupingStripeInfo(ddrType, uiMemPartSize, uiMemBusWidth,
                                         &stripeWidth, &bankHeight);

   hXvd->uiDecode_StripeWidth = stripeWidth;
   hXvd->uiDecode_StripeMultiple = bankHeight;

   uiReg = (stripeWidth << hXvd->stPlatformInfo.stReg.uiDecode_StripeWidthShift);

   hXvd->uiDecode_SDStripeWidthRegVal = uiReg;

   BDBG_MSG(("StipeWidthRegVal:0x%0x\n", uiReg));

   /* Only 16 or 32 Bit bus is supported */
   if (uiMemBusWidth == 16)
   {
      uiDataWidth = BXVD_P_PFRI_Data_Width_e16Bit;
   }
   else if (uiMemBusWidth == 32)
   {
      /* 32 bit bus */
      uiDataWidth = BXVD_P_PFRI_Data_Width_e32Bit;
   }
   else
   {
      /* 64 bit bus */
      uiDataWidth = BXVD_P_PFRI_Data_Width_e64Bit;
   }

   uiReg = uiDataWidth << hXvd->stPlatformInfo.stReg.uiPFRIInfo_DataBusWidthShift;
   uiReg |= (bankHeight << hXvd->stPlatformInfo.stReg.uiPFRIInfo_BankHeightShift);

   BDBG_MSG(("PFRIDataRegVal:0x%0x\n", uiReg));
   hXvd->uiDecode_PFRIDataRegVal = uiReg;

   uiPCacheVal = ( hXvd->uiAVD_PCacheRegVal &
                   (~(hXvd->stPlatformInfo.stReg.uiAVD_PcacheModeYGranMask))&
                   (~(hXvd->stPlatformInfo.stReg.uiAVD_PcacheModeXGranMask)));

   hXvd->uiAVD_PCacheRegVal = uiPCacheVal | ((uiYGran << hXvd->stPlatformInfo.stReg.uiAVD_PcacheModeYGranShift) |
                                             (uiXGran << hXvd->stPlatformInfo.stReg.uiAVD_PcacheModeXGranShift));

   BDBG_MSG(("PCacheRegVal: 0x%0x", hXvd->uiAVD_PCacheRegVal));
}

#ifdef BCHP_HEVD_PCACHE_0_PFRI_DEBUG_pfri_grouping_present_MASK
void BXVD_P_DetermineGroupingStripeInfo(BCHP_DramType ddrType,
                                        uint32_t uiMemPartSize,
                                        uint32_t uiMemBusWidth,
                                        uint32_t uiMemDeviceWidth,
                                        bool     bDDRGroupageEnabled,
                                        uint32_t *puiStripeWidth,
                                        uint32_t *puiBankHeight)
{
   switch(ddrType)
   {
      case BCHP_DramType_eDDR3:

         /* Set stripe width and bank height base on bus width and memory part size */
         switch(uiMemBusWidth)
         {
            /* Dram bus width 16 */
            case 16:

               switch(uiMemPartSize)
               {
                  case 512:  /* 512 Mbits Device Tech*/
                  case 1024: /* 1024 Mbits Device Tech*/
                  case 2048: /* 2048 Mbits Device Tech*/
                  case 4096: /* 4096 Mbits Device Tech*/
                     *puiBankHeight = (uint32_t)BXVD_P_PFRI_Bank_Height_8n3;
                     break;

                  case 8192: /* 8192 Mbits Device Tech*/

                     switch(uiMemDeviceWidth)
                     {
                        case 8:
                           *puiBankHeight = (uint32_t)BXVD_P_PFRI_Bank_Height_16n6;
                           break;

                        case 16:
                           *puiBankHeight = (uint32_t)BXVD_P_PFRI_Bank_Height_8n3;
                           break;
                     }

                     break;

                  default:
                     *puiBankHeight  = (uint32_t)BXVD_P_PFRI_Bank_Height_8n3;

                     BDBG_ERR(("Unknown memory part size: %d", uiMemPartSize));
                     break;
               }

               break;

            /* Dram bus width 32 */
            case 32:

               switch(uiMemPartSize)
               {
                  case 512:  /* 512 Mbits Device Tech*/
                  case 1024: /* 1024 Mbits Device Tech*/
                  case 2048: /* 2048 Mbits Device Tech*/
                  case 4096: /* 4096 Mbits Device Tech*/
                  case 8192: /* 8192 Mbits Device Tech*/
                     *puiBankHeight = (uint32_t)BXVD_P_PFRI_Bank_Height_16n6;
                     break;

                  default:
                     *puiBankHeight  = (uint32_t)BXVD_P_PFRI_Bank_Height_8n3;
                     BDBG_ERR(("Unknown memory part size: %d", uiMemPartSize));
                     break;
               }

               break;

            default:
               *puiBankHeight  = (uint32_t)BXVD_P_PFRI_Bank_Height_8n3;
               BDBG_ERR(("Unknown memory bus width: %d", uiMemBusWidth));

               break;
         }

         *puiStripeWidth = BXVD_P_PFRI_Stripe_Width_e128Bytes;

         break;

      case BCHP_DramType_eDDR4:

         *puiBankHeight = (uint32_t)BXVD_P_PFRI_Bank_Height_16n6;

         if (uiMemBusWidth == 16)
         {
            *puiStripeWidth = BXVD_P_PFRI_Stripe_Width_e128Bytes;
         }
         else
         {
            *puiStripeWidth = BXVD_P_PFRI_Stripe_Width_e256Bytes;
         }

         break;

      case BCHP_DramType_eGDDR5:

         if (bDDRGroupageEnabled)
         {
            if (uiMemBusWidth == 32)
            {
               *puiStripeWidth = BXVD_P_PFRI_Stripe_Width_e256Bytes;
            }
            else
            {  /*  Mem Bus Width 16 */
               *puiStripeWidth = BXVD_P_PFRI_Stripe_Width_e128Bytes;
            }

            if (uiMemPartSize == 8192)
            {
               *puiBankHeight = (uint32_t)BXVD_P_PFRI_Bank_Height_32n12;
            }
            else
            {
               *puiBankHeight = (uint32_t)BXVD_P_PFRI_Bank_Height_16n6;
            }

         }
         else
         {
            *puiStripeWidth = BXVD_P_PFRI_Stripe_Width_e128Bytes;

            if (uiMemPartSize == 8192)
            {
               if (uiMemBusWidth == 32)
               {
                  *puiBankHeight = (uint32_t)BXVD_P_PFRI_Bank_Height_32n12;
               }
               else
               {
                  *puiBankHeight = (uint32_t)BXVD_P_PFRI_Bank_Height_16n6;
               }
            }
            else
            {
               if (uiMemBusWidth == 32)
               {
                  *puiBankHeight = (uint32_t)BXVD_P_PFRI_Bank_Height_16n6;
               }
               else
               {
                  *puiBankHeight = (uint32_t)BXVD_P_PFRI_Bank_Height_8n3;
               }
            }
         }

         break;

      case BCHP_DramType_eLPDDR4:

         *puiBankHeight = (uint32_t)BXVD_P_PFRI_Bank_Height_8n3;

         *puiStripeWidth = BXVD_P_PFRI_Stripe_Width_e256Bytes;

         break;

      default:

         BDBG_ERR(("Unsupported DDR type %d", ddrType));
         break;
   }
}

void  BXVD_P_DetermineGroupagePCacheSettings(BXVD_Handle hXvd,
                                             BCHP_DramType ddrType,
                                             uint32_t uiMemPartSize,
                                             uint32_t uiMemBusWidth,
                                             uint32_t uiMemDeviceWidth,
                                             bool  bDDRGroupageEnabled)
{
   uint32_t uiDataWidth;

   uint32_t stripeWidth=0;
   uint32_t bankHeight=0;
   uint32_t uiXGran=0xff; /* Default Not set value */
   uint32_t uiYGran=0;
   uint32_t uiNumBanks=0;

   uint32_t uiReg;
   uint32_t uiPCacheVal;

   BXVD_P_DetermineGroupingStripeInfo(ddrType, uiMemPartSize, uiMemBusWidth, uiMemDeviceWidth,
                                      bDDRGroupageEnabled, &stripeWidth, &bankHeight);

   switch(ddrType)
   {
      case BCHP_DramType_eDDR3:

         uiNumBanks = BXVD_P_PFRI_Num_Banks_8;

         uiYGran = BXVD_P_PCache_YGran_e2Lines;

         break;

      case BCHP_DramType_eDDR4:

         if (uiMemBusWidth == 16)
         {
            uiYGran = BXVD_P_PCache_YGran_e2Lines;
         }
         else
         {
            uiYGran = BXVD_P_PCache_YGran_e4Lines;
         }

         if (uiMemDeviceWidth == 8)
         {
            uiNumBanks = BXVD_P_PFRI_Num_Banks_8;
         }
         else
         {
            uiNumBanks = BXVD_P_PFRI_Num_Banks_4;
         }

         break;

      case BCHP_DramType_eGDDR5:

         if (bDDRGroupageEnabled)
         {
            uiNumBanks = BXVD_P_PFRI_Num_Banks_16;

            if (uiMemBusWidth == 32)
            {
               uiYGran = BXVD_P_PCache_YGran_e2Lines;

            }
            else
            {  /*  Mem Bus Width 16 */
               uiYGran = BXVD_P_PCache_YGran_e1Line;
            }
         }
         else
         {
            uiNumBanks = BXVD_P_PFRI_Num_Banks_8;
            uiYGran = BXVD_P_PCache_YGran_e1Line;
         }

         break;

      case BCHP_DramType_eLPDDR4:

         uiYGran = BXVD_P_PCache_YGran_e2Lines;
         uiXGran = BXVD_P_PCache_XGran_e32Bytes;

         uiNumBanks = BXVD_P_PFRI_Num_Banks_8;

         break;

      default:

         BDBG_ERR(("Unsupported DDR type %d", ddrType));
         break;
   }

   if (uiXGran == 0xff)
   {
      /* default configuration */
      uiXGran = BXVD_P_PCache_XGran_e16Bytes;
   }

   /* Save away settings */
   hXvd->uiDecode_StripeWidth = stripeWidth;
   hXvd->uiDecode_StripeMultiple = bankHeight;

   uiReg = (stripeWidth << hXvd->stPlatformInfo.stReg.uiDecode_StripeWidthShift);

   hXvd->uiDecode_SDStripeWidthRegVal = uiReg;
   BDBG_MSG(("StipeWidthRegVal:0x%0x\n", uiReg));

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

   hXvd->uiDecode_PFRIDataRegVal = BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiPFRIInfo);

   hXvd->uiDecode_PFRIDataRegVal = (hXvd->uiDecode_PFRIDataRegVal &
                                    (~(hXvd->stPlatformInfo.stReg.uiPFRIInfo_DataBusWidthMask)) &
                                    (~(hXvd->stPlatformInfo.stReg.uiPFRIInfo_BankHeightMask)) &
                                    (~(hXvd->stPlatformInfo.stReg.uiPFRIInfo_PFRIGroupagesEnaMask)) &
                                    (~(hXvd->stPlatformInfo.stReg.uiPFRIInfo_PFRINumBanksMask)));

   hXvd->uiDecode_PFRIDataRegVal = (hXvd->uiDecode_PFRIDataRegVal |
                                    (uiDataWidth << hXvd->stPlatformInfo.stReg.uiPFRIInfo_DataBusWidthShift) |
                                    (bankHeight << hXvd->stPlatformInfo.stReg.uiPFRIInfo_BankHeightShift) |
                                    (bDDRGroupageEnabled << hXvd->stPlatformInfo.stReg.uiPFRIInfo_PFRIGroupagesEnaShift) |
                                    (uiNumBanks << hXvd->stPlatformInfo.stReg.uiPFRIInfo_PFRINumBanksShift));

   uiPCacheVal = ( hXvd->uiAVD_PCacheRegVal &
                   (~(hXvd->stPlatformInfo.stReg.uiAVD_PcacheModeYGranMask))&
                   (~(hXvd->stPlatformInfo.stReg.uiAVD_PcacheModeXGranMask)));

   hXvd->uiAVD_PCacheRegVal = uiPCacheVal | ((uiYGran << hXvd->stPlatformInfo.stReg.uiAVD_PcacheModeYGranShift) |
                                             (uiXGran << hXvd->stPlatformInfo.stReg.uiAVD_PcacheModeXGranShift));
}
#endif

void BXVD_P_InitRegPtrs_RevN0(BXVD_Handle hXvd)
{
   uint32_t uiMemPartSize;
   uint32_t uiMemBusWidth;
   uint32_t uiMemDeviceWidth;

   BCHP_MemoryInfo stMemoryInfo;
   BCHP_DramType ddrType;

   BERR_Code rc = BERR_SUCCESS;

   BDBG_ASSERT(hXvd);

   BDBG_ENTER(BXVD_P_InitRegPtrs_RevN0);

   /* Platform Info */
   hXvd->stPlatformInfo.stFirmware.outerELFFirmwareID =
      BXVD_IMAGE_FirmwareID_eOuterELF_AVD0;
   hXvd->stPlatformInfo.stFirmware.innerELFFirmwareID =
      BXVD_IMAGE_FirmwareID_eInnerELF_AVD0;
   hXvd->stPlatformInfo.stFirmware.authenticatedFirmwareID =
      BXVD_IMAGE_FirmwareID_eAuthenticated_AVD0;

#if BXVD_P_THREE_HVD_DECODERS_PRESENT
   if (hXvd->uDecoderInstance == 0)
   {
      hXvd->stPlatformInfo.stReg = s_stPlatformReg_HVD0RevN0;
   }

   else if (hXvd->uDecoderInstance == 1)
   {
      hXvd->stPlatformInfo.stReg = s_stPlatformReg_HVD1RevN0;
   }
   else
   {
      hXvd->stPlatformInfo.stReg = s_stPlatformReg_HVD2RevN0;
   }

#else
   /* HEVD */
   if (hXvd->uDecoderInstance == 0)
   {
      hXvd->stPlatformInfo.stReg = s_stPlatformReg_HVD0RevN0;
   }

#if BXVD_MAX_INSTANCE_COUNT == 2
   /* AVD decoder */
   else if (hXvd->uDecoderInstance == 1)
   {
      hXvd->stPlatformInfo.stReg = s_stPlatformReg_AVD1RevN0;
   }
#endif
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

   hXvd->uiAVDCoreFreq = BXVD_P_AVD_OL_CLK_FREQ * 1000000;

   BXVD_DBG_MSG(hXvd, ("AVD core clock freq: %d MHz", (hXvd->uiAVDCoreFreq)));

#endif

   rc = BCHP_GetMemoryInfo( hXvd->hReg, &stMemoryInfo);

   if (rc != BERR_SUCCESS)
   {
      BDBG_ERR(("BCHP_GetMemoryInfo failed"));
      return;
   }

   /* Determine memory configuration */
   ddrType = stMemoryInfo.memc[0].type;
   uiMemPartSize = stMemoryInfo.memc[0].deviceTech;
   uiMemBusWidth = stMemoryInfo.memc[0].width;
   uiMemDeviceWidth = stMemoryInfo.memc[0].deviceWidth;

   BDBG_MSG(("DDRType: %d BusWidth:%d DeviceWidth: %d MemPartSize: %d, groupageEnabled: %d",
             ddrType, uiMemBusWidth, uiMemDeviceWidth, uiMemPartSize, stMemoryInfo.memc[0].groupageEnabled));

#if BXVD_P_HEVD_PFRI_DEBUG_PFRI_GROUPING_PRESENT
   BXVD_P_DetermineGroupagePCacheSettings(hXvd, ddrType, uiMemPartSize, uiMemBusWidth,
                                          uiMemDeviceWidth, stMemoryInfo.memc[0].groupageEnabled);
#else
   BXVD_P_DetermineNonGroupagePCacheSettings(hXvd, ddrType,uiMemPartSize, uiMemBusWidth);
#endif

   BDBG_LEAVE(BXVD_P_InitRegPtrs_RevN0);
}
#endif /* BXVD_P_USE_INIT_REG_PTRS_REVN0 */


#ifdef BXVD_P_USE_CORE_RESET_CHIP_REVN0
BERR_Code BXVD_P_ChipReset_RevN0(BXVD_Handle hXvd)
{
   uint32_t uiSoftDnVal;

#if !BXVD_P_USE_REV_N_RESET_METHOD
   uint32_t uiRegVal;
   uint32_t uiPollCnt;
   uint32_t uiResetMask;
   bool bDone;
#endif

   BERR_Code rc;

   BDBG_ASSERT(hXvd);

   BDBG_ENTER(BXVD_P_ChipReset_RevN0);

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

   {
      uint32_t uiGISBMask;

      /* Check to see if AVD has access to the GISB */
      uiGISBMask = BREG_Read32(hXvd->hReg,
                               hXvd->stPlatformInfo.stReg.uiSunGisbArb_ReqMask);

      if (uiGISBMask & hXvd->stPlatformInfo.stReg.uiSunGisbArb_ReqMaskAVDMask)
      {
         BXVD_DBG_ERR(hXvd, ("***ERROR*** AVD access to GISB DISABLE! GISB Arbiter Request Mask: %08x", uiGISBMask));
         BXVD_DBG_ERR(hXvd, ("***FORCE*** AVD access to GISB SET TO 0!!"));

         BREG_Write32(hXvd->hReg, hXvd->stPlatformInfo.stReg.uiSunGisbArb_ReqMask, 0);
      }
   }

   /* Determine if this is Dual pipe HEVC decoder */
   if ( (BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_CPUId))
        & hXvd->stPlatformInfo.stReg.uiDecode_CPUId_HEVD_dualpipeMask)
   {
      hXvd->bHEVDDualPipe = true;
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
#if BXVD_P_USE_REV_N_RESET_METHOD
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

#if BXVD_P_SET_CHICKEN_BIT_BEFORE_RESET

   BXVD_Reg_Write32(hXvd,
                    BCHP_HEVD_OL_CTL_0_CLIENT_INIT_CONFIG,
                    BCHP_HEVD_OL_CTL_0_CLIENT_INIT_CONFIG_client_init_disable_MASK);
#endif

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

#else /* Use REV Q reset method */

   uiResetMask = hXvd->stPlatformInfo.stReg.uiAvd_SoftShutdownStatusMask;

   BXVD_Reg_Write32(hXvd, hXvd->stPlatformInfo.stReg.uiAvd_SoftShutdownCtrl, 1);

   bDone = false;
   uiPollCnt = 1;

   while (!bDone)
   {
      uiRegVal = BXVD_Reg_Read32( hXvd, hXvd->stPlatformInfo.stReg.uiAvd_SoftShutdownStatus);

      if ((uiRegVal & uiResetMask) || (uiPollCnt++ > 100))
      {
         bDone = true;
      }
   }


   BXVD_DBG_MSG(hXvd, ("Core reset pollcnt: %d\n", uiPollCnt));

   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiSun_SWInitSet,
                    hXvd->stPlatformInfo.stReg.uiSun_SWInitSetAvdMask);

   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiSun_SWInitClear,
                    hXvd->stPlatformInfo.stReg.uiSun_SWInitClearAvdMask);

#endif

   /* Clear interrupt mask (Enable) register to prevent interrupts before ISR is setup */
   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_OuterInterruptMask,
                    0);

   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_StripeWidth,
                    hXvd->uiDecode_SDStripeWidthRegVal);

   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiPFRIInfo,
                    hXvd->uiDecode_PFRIDataRegVal);

   /* Write the AVD PCache mode to hardware */
   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiAVD_PcacheMode,
                    hXvd->uiAVD_PCacheRegVal);

#if BXVD_P_HEVD_DUAL_PIPE_PRESENT
   if (hXvd->bHEVDDualPipe)
   {
      BDBG_MSG(("Setting Pipe2 Stripe and PCACHE registers"));

      BXVD_Reg_Write32(hXvd,
                       hXvd->stPlatformInfo.stReg.uiDecode_StripeWidth_P2,
                       hXvd->uiDecode_SDStripeWidthRegVal);

      BXVD_Reg_Write32(hXvd,
                       hXvd->stPlatformInfo.stReg.uiPFRIInfo_P2,
                       hXvd->uiDecode_PFRIDataRegVal);

      /* Write the AVD PCache mode to hardware */
      BXVD_Reg_Write32(hXvd,
                       hXvd->stPlatformInfo.stReg.uiAVD_PcacheMode_P2,
                       hXvd->uiAVD_PCacheRegVal);
   }
#endif
   BDBG_LEAVE(BXVD_P_ChipReset_RevN0);

   return BERR_TRACE(BERR_SUCCESS);
}
#endif

#if BXVD_P_USE_CORE_CHIP_ENABLE_REVN0

BERR_Code BXVD_P_ChipEnable_RevN0(BXVD_Handle hXvd)
{
#if BDBG_DEBUG_BUILD
   volatile uint32_t uiFWBootStatus;
#endif

   uint32_t byteOffset;

   BAFL_BootInfo stAVDBootInfo;

   BAFL_FirmwareInfo astFirmwareInfo[3];

   BERR_Code rc = BERR_SUCCESS;

#if BXVD_POLL_FW_MBX
   uint32_t uiVal;
   int loopCount;
#endif

   BDBG_ASSERT(hXvd);
   BDBG_ENTER(BXVD_P_ChipEnable_RevN0);

   /*
    * Write to VectorTB, CpuDbg registers and AuxRegs
    * in THIS ORDER to start Outer Loop ARC
    */
   BDBG_MSG(("OL Ibase: %0x", hXvd->uiOuterLoopInstructionBase));
   BDBG_MSG(("OL EOC: %0x", hXvd->uiOuterLoopEOC));

   BDBG_MSG(("IL Ibase: %0x",hXvd->uiInnerLoopInstructionBase));
   BDBG_MSG(("IL EOC: %0x", hXvd->uiInnerLoopEOC));

   /* Program the relocation base address for outer-loop */
   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_OuterInstructionBase,
                    hXvd->uiOuterLoopInstructionBase);

   /* Program the outer loop image end of code address */
   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_OuterEndOfCode,
                    hXvd->uiOuterLoopEOC);

   /* Program the relocation base address for inner-loop */
   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_InnerInstructionBase,
                    hXvd->uiInnerLoopInstructionBase);

   /* Program the inner loop image end of code address */
   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_InnerEndOfCode,
                    hXvd->uiInnerLoopEOC);

   /* Program global IO base register - Outer */
   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_OuterGlobalIOBase,
                    BCHP_PHYSICAL_OFFSET+BXVD_P_STB_REG_BASE);

   /* Program global IO base register - Inner */
   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_InnerGlobalIOBase,
                    BCHP_PHYSICAL_OFFSET+BXVD_P_STB_REG_BASE);


#if BXVD_P_HEVD_DUAL_PIPE_PRESENT
   if (hXvd->bHEVDDualPipe)
   {
      /* Program the relocation base address for inner-loop */
      BXVD_Reg_Write32(hXvd,
                       hXvd->stPlatformInfo.stReg.uiDecode_Inner2InstructionBase,
                       hXvd->uiInnerLoop2InstructionBase);

      /* Program the inner loop image end of code address */
      BXVD_Reg_Write32(hXvd,
                       hXvd->stPlatformInfo.stReg.uiDecode_Inner2EndOfCode,
                       hXvd->uiInnerLoop2EOC);

      /* Program global IO base register - Inner */
      BXVD_Reg_Write32(hXvd,
                       hXvd->stPlatformInfo.stReg.uiDecode_Inner2GlobalIOBase,
                       BCHP_PHYSICAL_OFFSET+BXVD_P_STB_REG_BASE);
   }
#endif

   /* Clear out any previously completed FW command done events */
   BKNI_ResetEvent(hXvd->stDecoderContext.hFWCmdDoneEvent);

   /* Write ARC clock freqs to HIM first 3 words*/
   byteOffset=0;
   BXVD_P_WRITE_HIM(hXvd, byteOffset, (BXVD_P_AVD_OL_CLK_FREQ * 1000000));

   byteOffset+=4;
   BXVD_P_WRITE_HIM(hXvd, byteOffset, (BXVD_P_AVD_IL_CLK_FREQ * 1000000));

#if BXVD_POLL_FW_MBX
   BDBG_ERR(("Booting Video Decoder polling for completion"));
   /* Initialize MBX to non-zero */
   BXVD_Reg_Write32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_OuterCPU2HostMailbox, 0xff);
   uiVal = BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_OuterCPU2HostMailbox);
   BDBG_MSG(("Initial CPU2HostMB: %0x\n", uiVal));
#endif

   if (hXvd->stSettings.pAVDBootCallback)
   {
      /* Set boot mode */
      if ( hXvd->eAVDBootMode == BXVD_AVDBootMode_eNormal)
      {
         stAVDBootInfo.eMode = BAFL_BootMode_eNormal;
      }
      else
      {
         stAVDBootInfo.eMode = BAFL_BootMode_eWatchdog;
      }

      stAVDBootInfo.pstArc = &astFirmwareInfo[0];

      /* Set Outer Loop ARC firmware info */
      astFirmwareInfo[0].uiArcInstance = 0;
      astFirmwareInfo[0].stCode.pStartAddress = hXvd->astFWBootInfo[0].stCode.pStartAddress;
      astFirmwareInfo[0].stCode.uiSize = hXvd->astFWBootInfo[0].stCode.uiSize;
      astFirmwareInfo[0].pNext = &astFirmwareInfo[1];

      /* Set Inner Loop ARC firmware info */
      astFirmwareInfo[1].uiArcInstance = 1;
      astFirmwareInfo[1].stCode.pStartAddress = hXvd->astFWBootInfo[1].stCode.pStartAddress;
      astFirmwareInfo[1].stCode.uiSize = hXvd->astFWBootInfo[1].stCode.uiSize;
      astFirmwareInfo[1].pNext = NULL;

#if BXVD_P_HEVD_DUAL_PIPE_PRESENT
      if (hXvd->bHEVDDualPipe)
      {
         astFirmwareInfo[1].pNext = &astFirmwareInfo[2];

         /* Set Inner Loop 2 ARC firmware info */
         astFirmwareInfo[2].uiArcInstance = 2;
         astFirmwareInfo[2].stCode.pStartAddress = hXvd->astFWBootInfo[2].stCode.pStartAddress;
         astFirmwareInfo[2].stCode.uiSize = hXvd->astFWBootInfo[2].stCode.uiSize;
         astFirmwareInfo[2].pNext = NULL;
      }
#endif

      /* Call external boot callback */
      rc = (*hXvd->stSettings.pAVDBootCallback)(hXvd->stSettings.pAVDBootCallbackData,
                                                &stAVDBootInfo);
      if (rc != BERR_SUCCESS)
      {
         BXVD_DBG_ERR(hXvd, ("Error #%d booting the AVD externally", rc));
         return BERR_TRACE(rc);
      }
   }
   else
   {
      /* Boot the core */

      BXVD_Reg_Write32(hXvd,
                       hXvd->stPlatformInfo.stReg.uiDecode_OuterCPUDebug,
                       1);

      /* Write ARC PC start address */
      BXVD_Reg_Write32(hXvd,
                       (hXvd->stPlatformInfo.stReg.uiDecode_OuterCPUAux+BXVD_P_ARC_PC),
                       0);

      /* Start Arc */
      BXVD_Reg_Write32(hXvd,
                       (hXvd->stPlatformInfo.stReg.uiDecode_OuterCPUAux+BXVD_P_ARC_STATUS32),
                       0);
   }

#if EMULATION
#define FW_CMD_TIMEOUT 100000
#else
#define FW_CMD_TIMEOUT 1000
#endif

#if !BXVD_POLL_FW_MBX

   rc = BERR_TRACE(BKNI_WaitForEvent(hXvd->stDecoderContext.hFWCmdDoneEvent, FW_CMD_TIMEOUT));

#else

   uiVal = BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_OuterCPU2HostMailbox);

   loopCount = 0;
   rc = BERR_TIMEOUT;

   BKNI_Printf("Sleep 3 Secs, then start polling for AVD Boot completion\n");

   BKNI_Sleep(3000);
   while (loopCount < 1000)
   {
      if (uiVal != 0)
      {
         uiFWBootStatus = BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_OuterCPU2HostStatus);

         BDBG_MSG(("ARC FW Boot Status = %d", uiFWBootStatus));

         BDBG_MSG(("loopCount:%d, MBX:%d\n", loopCount, uiVal));

         BDBG_ERR(("ARC FW Boot Status = %d", uiFWBootStatus));

         BDBG_ERR(("loopCount:%d Calling BKNI_Sleep(1000), MBX:%d\n", loopCount, uiVal));
#if 1
         {
            uint32_t ii;
            /*  From verify watchdog */
            BXVD_Reg_Write32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_OuterCPUDebug, 1);

#define BXVD_P_ARC_PC 0x18
            for (ii = 0; ii < 8; ii++)
            {
               uint32_t uiOL_pc;

               /* read the AVD OL PC */
               uiOL_pc = BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_OuterCPUAux+BXVD_P_ARC_PC);

               BXVD_DBG_ERR(hXvd, ("[%d] AVD_%d: OL PC=%08x", ii, hXvd->uDecoderInstance, uiOL_pc));
            }
         }
#endif
         BKNI_Sleep(1000);

         loopCount++;
         uiVal = BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_OuterCPU2HostMailbox);
      }
      else
      {
         rc = BERR_SUCCESS;
         break;
      }
   }
#endif

#if BDBG_DEBUG_BUILD

   /* Read FW boot progress/status from CPU2HostStatus register that was written by FW */
   uiFWBootStatus = BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_OuterCPU2HostStatus);

#endif

   if(BERR_TIMEOUT == rc)
   {
      BXVD_DBG_ERR(hXvd, ("ARC FW command response timed out, FW Boot Status = %d", uiFWBootStatus));

      return BERR_TRACE(rc);
   }
   else
   {
      BXVD_DBG_MSG(hXvd, ("FW boot successful, FW Boot Status = %d", uiFWBootStatus));
   }

   BKNI_ResetEvent(hXvd->stDecoderContext.hFWCmdDoneEvent);

   BDBG_LEAVE(BXVD_P_ChipEnable_RevN0);
   return BERR_TRACE(rc);
}
#endif /* BXVD_P_USE_CORE_CHIP_ENABLE_REVN0 */


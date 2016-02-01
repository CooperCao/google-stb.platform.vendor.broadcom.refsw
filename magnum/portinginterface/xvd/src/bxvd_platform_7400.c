/***************************************************************************
 *     Copyright (c) 2006-2013, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bxvd.h"
#include "bxvd_platform.h"
#include "bxvd_priv.h"
#include "bxvd_intr.h"
#include "bxvd_reg.h"
#include "bxvd_image.h"
#include "bxvd_image_priv.h"
/* to pickup BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CHANNEL_WR_POSITION_i_ARRAY_BASE */
#include "bchp_reg_cabac2bins2_0.h"
/* to pickup BCHP_REG_CABAC2BINS2_1_REG_CABAC2BINS_CHANNEL_WR_POSITION_i_ARRAY_BASE */
#include "bchp_reg_cabac2bins2_1.h"

#include "bchp_decode_sint_0.h"
#include "bchp_decode_sint_1.h"
#include "bchp_decode_sint_oloop_0.h"
#include "bchp_decode_sint_oloop_1.h"
#include "bchp_decode_main_0.h"
#include "bchp_decode_main_1.h"

#include "bchp_decode_rvc_0.h"
#include "bchp_decode_rvc_1.h"
#include "bchp_clk.h"


BDBG_MODULE(BXVD_PLATFORM_7400);

#ifdef BXVD_P_USE_INIT_REG_PTRS_7400B0

static const BXVD_P_PlatformReg_RevE0 s_stPlatformReg_7400B0_AVD0 = 
{
   BCHP_SUN_TOP_CTRL_SW_RESET,                    /* SWReset */
   BCHP_SUN_TOP_CTRL_SW_RESET_avd0_sw_reset_MASK, /* SWResetAVDMask */	

   BCHP_AVD_INTR2_0_CPU_SET,                      /* CPUL2InterruptSet */

   BCHP_BVNF_INTR2_3_AVD0_STATUS,                 /* Bvnf_Intr2_3_AvdStatus */
   BCHP_BVNF_INTR2_3_AVD0_CLEAR,                  /* Bvnf_Intr2_3_AvdClear */
   BCHP_BVNF_INTR2_3_AVD0_MASK_CLEAR,             /* Bvnf_Intr2_3_AvdMaskClear */
   
   BCHP_XPT_PCROFFSET_STC0,                       /* XPT_PCROffset_STC */

   BCHP_INT_ID_AVD_INTR2_0_AVD_SW_INTR0,          /* PicDataRdy */
   BCHP_INT_ID_AVD_INTR2_0_AVD_SW_INTR3,          /* PicDataRdy1 */
   BCHP_INT_ID_AVD_INTR2_0_AVD_MBOX_INTR,         /* Mailbox */
   BCHP_INT_ID_AVD_INTR2_0_AVD_SW_INTR4,          /* StereoSeqError */
   BCHP_INT_ID_AVD_INTR2_0_AVD_SW_INTR2,          /* StillPictureRdy */
   BCHP_INT_ID_AVD_INTR2_0_AVD_WATCHDOG_INTR,     /* OuterWatchdog */
   BCHP_INT_ID_AVD_INTR2_0_VICH_REG_INTR,         /* VIChkrReg */
   BCHP_INT_ID_AVD_INTR2_0_VICH_SCB_WR_INTR,      /* VIChkrSCBWr */
   BCHP_INT_ID_AVD_INTR2_0_VICH_INST_RD_INTR,     /* VIChkrInstrRead */
	
   BCHP_DECODE_CPUREGS_0_REG_INST_BASE,           /* InnerInstructionBase */
   BCHP_DECODE_CPUREGS_0_REG_END_OF_CODE,         /* InnerEndOfCode */
   BCHP_DECODE_CPUREGS_0_REG_GLOBAL_IO_BASE,      /* InnerGlobalIOBase */
   BCHP_DECODE_IND_SDRAM_REGS_0_REG_CPU_DBG,      /* InnerCPUDebug */
   BCHP_DECODE_CPUAUX_0_CPUAUX_REG,               /* InnerCPUAux */

   BCHP_DECODE_CPUREGS2_0_REG_INST_BASE,          /* OuterEndOfCode */
   BCHP_DECODE_CPUREGS2_0_REG_END_OF_CODE,        /* OuterInstructionBase */
   BCHP_DECODE_CPUREGS2_0_REG_GLOBAL_IO_BASE,     /* OuterGlobalIOBase */
   BCHP_DECODE_IND_SDRAM_REGS2_0_REG_CPU_DBG,     /* OuterCPUDebug */
   BCHP_DECODE_CPUAUX2_0_CPUAUX_REG,              /* OuterCPUAux */
   BCHP_DECODE_CPUREGS2_0_REG_CPU_INTGEN_MASK,    /* OuterInterruptMask */
   BCHP_DECODE_CPUREGS2_0_REG_CPU_INTGEN_CLR,     /* OuterInterruptClear */
   0xffffffff,                                    /* Unused - WD Timeout */
   BCHP_DECODE_CPUREGS2_0_REG_WATCHDOG_TMR,       /* OuterWatchdogTimer */
   BCHP_DECODE_CPUREGS2_0_REG_CPU2HST_MBX,        /* OuterCPU2HostMailbox */
   BCHP_DECODE_CPUREGS2_0_REG_HST2CPU_MBX,        /* OuterHost2CPUMailbox */
   BCHP_DECODE_CPUREGS2_0_REG_CPU2HST_STAT,       /* OuterCPU2HostStatus */

   BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CHANNEL_WR_POSITION_i_ARRAY_BASE,    /* CabacBinDepth */	

   BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CTL,            /* uiDecode_CabacBinCtl */
   BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CTL_Reset_MASK, /* uiDecode_CabacBinCtl_ResetMask */
   BCHP_DECODE_SINT_0_REG_SINT_STRM_STAT,                /* uiDecode_SintStrmSts */
   BCHP_DECODE_SINT_0_REG_SINT_STRM_STAT_Rst_MASK,       /* uiDecode_SintStrmSts_ResetMask */
   BCHP_DECODE_SINT_OLOOP_0_DEC_SINT_STRM_STAT,          /* uiDecode_OLSintStrmSts */
   BCHP_DECODE_SINT_OLOOP_0_DEC_SINT_STRM_STAT_Rst_MASK, /* uiDecode_OLSintStrmSts_ResetMask */
   BCHP_DECODE_MAIN_0_REG_MAINCTL,                       /* uiDecode_MainCtl */
   BCHP_DECODE_MAIN_0_REG_MAINCTL_Rst_MASK,              /* uiDecode_MainCtl_ResetMask */

   BCHP_SUN_GISB_ARB_REQ_MASK,                    /* SunGisbArb_ReqMask */
   BCHP_SUN_GISB_ARB_REQ_MASK_avd0_MASK,          /* SunGisbArb_ReqMaskAVDMask */

   BCHP_DECODE_RVC_0_REG_RVC_CTL,   /* RVC Control */
   BCHP_DECODE_RVC_0_REG_RVC_PUT,   /* RVC Put */
   BCHP_DECODE_RVC_0_REG_RVC_GET,   /* RVC Get */
   BCHP_DECODE_RVC_0_REG_RVC_BASE,  /* RVC Base */
   BCHP_DECODE_RVC_0_REG_RVC_END,   /* RVC End */

#if BXVD_P_POWER_MANAGEMENT
   (uint32_t)NULL,                                /* uiAVDCtrl */
   (uint32_t)NULL,                                /* uiAVDCtrl_PwrDnMask */
   BCHP_CLK_PM_CTRL_1,                            /* uiCoreClkCtrl; */
   BCHP_CLK_PM_CTRL_1_DIS_AVD0_200_CLK_MASK,      /* uiCoreClkCtrl_PwrDnMask */
   BCHP_CLK_PM_CTRL_1,                            /* uiSCBClkCtrl */
   BCHP_CLK_PM_CTRL_1_DIS_MEMC_16_0_200_CLK_MASK, /* uiSCBClkCtrl_PwrDnMask */
   BCHP_CLK_PM_CTRL,                              /* uiGISBClkCtrl */
   BCHP_CLK_PM_CTRL_DIS_AVD0_108M_CLK_MASK,       /* uiGISBClkCtrl_PwrDnMask */
#endif
};

static const BXVD_P_PlatformReg_RevE0 s_stPlatformReg_7400B0_AVD1 = 
{
   BCHP_SUN_TOP_CTRL_SW_RESET,                    /* SWReset */
   BCHP_SUN_TOP_CTRL_SW_RESET_avd1_sw_reset_MASK, /* SWResetAVDMask */	

   BCHP_AVD_INTR2_1_CPU_SET,                      /* CPUL2InterruptSet */

   BCHP_BVNF_INTR2_3_AVD1_STATUS,                 /* Bvnf_Intr2_3_AvdStatus */
   BCHP_BVNF_INTR2_3_AVD1_CLEAR,                  /* Bvnf_Intr2_3_AvdClear */
   BCHP_BVNF_INTR2_3_AVD1_MASK_CLEAR,             /* Bvnf_Intr2_3_AvdMaskClear */

   BCHP_XPT_PCROFFSET_STC1,                       /* XPT_PCROffset_STC */

   BCHP_INT_ID_AVD_INTR2_1_AVD_SW_INTR0,          /* PicDataRdy */
   BCHP_INT_ID_AVD_INTR2_1_AVD_SW_INTR3,          /* PicDataRdy1 */
   BCHP_INT_ID_AVD_INTR2_1_AVD_MBOX_INTR,         /* Mailbox */
   BCHP_INT_ID_AVD_INTR2_1_AVD_SW_INTR4,          /* StereoSeqError */
   BCHP_INT_ID_AVD_INTR2_1_AVD_SW_INTR2,          /* StillPictureRdy */
   BCHP_INT_ID_AVD_INTR2_1_AVD_WATCHDOG_INTR,     /* OuterWatchdog */
   BCHP_INT_ID_AVD_INTR2_1_VICH_REG_INTR,         /* VIChkrReg */
   BCHP_INT_ID_AVD_INTR2_1_VICH_SCB_WR_INTR,      /* VIChkrSCBWr */
   BCHP_INT_ID_AVD_INTR2_1_VICH_INST_RD_INTR,     /* VIChkrInstrRead */
	
   BCHP_DECODE_CPUREGS_1_REG_INST_BASE,           /* InnerInstructionBase */
   BCHP_DECODE_CPUREGS_1_REG_END_OF_CODE,         /* InnerEndOfCode */
   BCHP_DECODE_CPUREGS_1_REG_GLOBAL_IO_BASE,      /* InnerGlobalIOBase */
   BCHP_DECODE_IND_SDRAM_REGS_1_REG_CPU_DBG,      /* InnerCPUDebug */
   BCHP_DECODE_CPUAUX_1_CPUAUX_REG,               /* InnerCPUAux */

   BCHP_DECODE_CPUREGS2_1_REG_INST_BASE,          /* OuterEndOfCode */
   BCHP_DECODE_CPUREGS2_1_REG_END_OF_CODE,        /* OuterInstructionBase */
   BCHP_DECODE_CPUREGS2_1_REG_GLOBAL_IO_BASE,     /* OuterGlobalIOBase */
   BCHP_DECODE_IND_SDRAM_REGS2_1_REG_CPU_DBG,     /* OuterCPUDebug */
   BCHP_DECODE_CPUAUX2_1_CPUAUX_REG,              /* OuterCPUAux */
   BCHP_DECODE_CPUREGS2_1_REG_CPU_INTGEN_MASK,    /* OuterInterruptMask */
   BCHP_DECODE_CPUREGS2_1_REG_CPU_INTGEN_CLR,     /* OuterInterruptClear */
   0xffffffff,                                    /* Unused - WD Timeout */
   BCHP_DECODE_CPUREGS2_1_REG_WATCHDOG_TMR,       /* OuterWatchdogTimer */
   BCHP_DECODE_CPUREGS2_1_REG_CPU2HST_MBX,        /* OuterCPU2HostMailbox */
   BCHP_DECODE_CPUREGS2_1_REG_HST2CPU_MBX,        /* OuterHost2CPUMailbox */
   BCHP_DECODE_CPUREGS2_1_REG_CPU2HST_STAT,       /* OuterCPU2HostStatus */

   BCHP_REG_CABAC2BINS2_1_REG_CABAC2BINS_CHANNEL_WR_POSITION_i_ARRAY_BASE,  /* CabacBinDepth */	

   BCHP_REG_CABAC2BINS2_1_REG_CABAC2BINS_CTL,            /* uiDecode_CabacBinCtl */
   BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CTL_Reset_MASK, /* uiDecode_CabacBinCtl_ResetMask */
   BCHP_DECODE_SINT_1_REG_SINT_STRM_STAT,                /* uiDecode_SintStrmSts */
   BCHP_DECODE_SINT_0_REG_SINT_STRM_STAT_Rst_MASK,       /* uiDecode_SintStrmSts_ResetMask */
   BCHP_DECODE_SINT_OLOOP_1_DEC_SINT_STRM_STAT,          /* uiDecode_OLSintStrmSts */
   BCHP_DECODE_SINT_OLOOP_0_DEC_SINT_STRM_STAT_Rst_MASK, /* uiDecode_OLSintStrmSts_ResetMask */
   BCHP_DECODE_MAIN_1_REG_MAINCTL,                       /* uiDecode_MainCtl */
   BCHP_DECODE_MAIN_0_REG_MAINCTL_Rst_MASK,              /* uiDecode_MainCtl_ResetMask */

   BCHP_SUN_GISB_ARB_REQ_MASK,                    /* SunGisbArb_ReqMask */
   BCHP_SUN_GISB_ARB_REQ_MASK_avd1_MASK,          /* SunGisbArb_ReqMaskAVDMask */

   BCHP_DECODE_RVC_1_REG_RVC_CTL,   /* RVC Control */
   BCHP_DECODE_RVC_1_REG_RVC_PUT,   /* RVC Put */
   BCHP_DECODE_RVC_1_REG_RVC_GET,   /* RVC Get */
   BCHP_DECODE_RVC_1_REG_RVC_BASE,  /* RVC Base */
   BCHP_DECODE_RVC_1_REG_RVC_END,   /* RVC End */

#if BXVD_P_POWER_MANAGEMENT
   (uint32_t)NULL,                                /* uiAVDCtrl */
   (uint32_t)NULL,                                /* uiAVDCtrl_PwrDnMask */
   BCHP_CLK_PM_CTRL_1,                            /* uiCoreClkCtrl; */
   BCHP_CLK_PM_CTRL_1_DIS_AVD1_200_CLK_MASK,      /* uiCoreClkCtrl_PwrDnMask */
   BCHP_CLK_PM_CTRL_1,                            /* uiSCBClkCtrl */
   BCHP_CLK_PM_CTRL_1_DIS_MEMC_16_1_200_CLK_MASK, /* uiSCBClkCtrl_PwrDnMask */
   BCHP_CLK_PM_CTRL,                              /* uiGISBClkCtrl */
   BCHP_CLK_PM_CTRL_DIS_AVD1_108M_CLK_MASK,       /* uiGISBClkCtrl_PwrDnMask */
#endif
};

void BXVD_P_InitRegPtrs_7400B0( BXVD_Handle hXvd)
{
   BDBG_ASSERT(hXvd);

   BDBG_ENTER(BXVD_P_InitRegPtrs_7400B0);

   if (hXvd->uDecoderInstance == 0)
   {
      hXvd->stPlatformInfo.stFirmware.outerELFFirmwareID = BXVD_IMAGE_FirmwareID_eOuterELF_AVD0;
      hXvd->stPlatformInfo.stFirmware.innerELFFirmwareID = BXVD_IMAGE_FirmwareID_eInnerELF_AVD0;
      hXvd->stPlatformInfo.stFirmware.authenticatedFirmwareID = BXVD_IMAGE_FirmwareID_eAuthenticated_AVD0;

      hXvd->stPlatformInfo.stReg = s_stPlatformReg_7400B0_AVD0;
   }
   else
   {
      /* Platform Info */
      hXvd->stPlatformInfo.stFirmware.outerELFFirmwareID = BXVD_IMAGE_FirmwareID_eOuterELF_AVD1;
      hXvd->stPlatformInfo.stFirmware.innerELFFirmwareID = BXVD_IMAGE_FirmwareID_eInnerELF_AVD1;
      hXvd->stPlatformInfo.stFirmware.authenticatedFirmwareID = BXVD_IMAGE_FirmwareID_eAuthenticated_AVD1;

      hXvd->stPlatformInfo.stReg = s_stPlatformReg_7400B0_AVD1;	  
   }

   BDBG_LEAVE(BXVD_P_InitRegPtrs_7400B0);
}
#endif /* BXVD_P_USE_INIT_REG_PTRS_7400B0 */

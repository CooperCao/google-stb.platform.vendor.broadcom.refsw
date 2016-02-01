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
#include "bchp_common.h"


/* to pickup BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CHANNEL_WR_POSITION_i_ARRAY_BASE */
#include "bchp_reg_cabac2bins2_0.h"
#include "bchp_decode_rvc_0.h"

/* Power management registers */
#include "bchp_vcxo_ctl_misc.h"

#if (BCHP_CHIP == 7405) || (BCHP_CHIP == 7335)
#include "bchp_clk.h"

#elif (BCHP_CHIP == 7325) || (BCHP_CHIP == 3548) || (BCHP_CHIP == 3556) || (BCHP_CHIP == 7125)
#include "bchp_clkgen.h"
#endif

/* 
 * The following section is used to handle the slight register differences in the 7405 
 * family of chips. The 7405 family of chips can be grouped so to use the same register definitions. 
 * 
 * Here is the map of chips and revisions that are the same:
 *
 *       Register Rev Table                  Platforms
 *
 *          7405A0                       7405 A0 A1, 7335, 7325
 *
 *          7405B0                       7405 B0, 3548, 3556
 */ 

#if ((BCHP_CHIP == 7405) && ((BCHP_VER == BCHP_VER_A0) || (BCHP_VER == BCHP_VER_A1))) || \
    (BCHP_CHIP == 7335) || \
    (BCHP_CHIP == 7325)

/* Select 7405 A0 register definitions */
#define BXVD_P_USE_INIT_REG_PTRS_7405A0 1

#elif ((BCHP_CHIP == 7405) && (BCHP_VER >= BCHP_VER_B0)) || \
      (BCHP_CHIP == 3548) || \
      (BCHP_CHIP == 3556) || \
      (BCHP_CHIP == 7336) || \
      (BCHP_CHIP == 7340) || \
      (BCHP_CHIP == 7342) || \
      (BCHP_CHIP == 7125) || \
      (BCHP_CHIP == 7468) || \
      (BCHP_CHIP == 7408) 

/* Select 7405 B0 register definitions */
#define BXVD_P_USE_INIT_REG_PTRS_7405B0 1

#endif

BDBG_MODULE(BXVD_PLATFORM_7405);

#ifdef BXVD_P_USE_INIT_REG_PTRS_7405

#ifdef BXVD_P_USE_INIT_REG_PTRS_7405A0
static const BXVD_P_PlatformReg_RevH0 s_stPlatformReg_7405 = 
{
   BCHP_SUN_TOP_CTRL_SW_RESET,                    /* SWReset */
   BCHP_SUN_TOP_CTRL_SW_RESET_avd0_sw_reset_MASK, /* SWResetAVDMask */	

   BCHP_AVD_INTR2_0_CPU_SET,                      /* CPUL2InterruptSet */

   BCHP_BVNF_INTR2_3_AVD0_STATUS,                 /* Bvnf_Intr2_3_AvdStatus */
   BCHP_BVNF_INTR2_3_AVD0_CLEAR,                  /* Bvnf_Intr2_3_AvdClear */
   BCHP_BVNF_INTR2_3_AVD0_MASK_CLEAR,             /* Bvnf_Intr2_3_AvdMaskClear */

   BCHP_XPT_PCROFFSET_STC0,                       /* XPT_PCROffset_STC */

   BCHP_INT_ID_AVD_SW_INTR0,          /* PicDataRdy */
   BCHP_INT_ID_AVD_SW_INTR3,          /* PicDataRdy1 */
   BCHP_INT_ID_AVD_MBOX_INTR,         /* Mailbox */
   BCHP_INT_ID_AVD_SW_INTR4,          /* StereoSeqError */
   BCHP_INT_ID_AVD_SW_INTR2,          /* StillPictureRdy */
   BCHP_INT_ID_AVD_WATCHDOG_INTR,     /* OuterWatchdog */
   BCHP_INT_ID_VICH_REG_INTR,         /* VIChkrReg */
   BCHP_INT_ID_VICH_SCB_WR_INTR,      /* VIChkrSCBWr */
   BCHP_INT_ID_VICH_INST_RD_INTR,     /* VIChkrInstrRead */

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

   BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH,                   /* StripeWidth */
   BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH_Strip_Width_MASK,  /* StripeWidthMask */
   BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH_Strip_Width_SHIFT, /* StripeWidthShift */
   BCHP_DECODE_IP_SHIM_0_PFRI_REG,                         /* PFRI */
   BCHP_DECODE_IP_SHIM_0_PFRI_REG_pfri_bank_height_MASK,   /* PFRIBankHeightMask */
   BCHP_DECODE_IP_SHIM_0_PFRI_REG_pfri_bank_height_SHIFT,  /* PFRIBankHeightShift */

   BCHP_DECODE_RVC_0_REG_RVC_CTL,   /* RVC Control */
   BCHP_DECODE_RVC_0_REG_RVC_PUT,   /* RVC Put */
   BCHP_DECODE_RVC_0_REG_RVC_GET,   /* RVC Get */
   BCHP_DECODE_RVC_0_REG_RVC_BASE,  /* RVC Base */
   BCHP_DECODE_RVC_0_REG_RVC_END,   /* RVC End */

   BXVD_P_AVD0_CTRL,                      /* uiAVDCtrl */
   BXVD_P_AVD0_CTRL_PWRDN_MASK,           /* uiAVDCtrl_PwrDnMask */
   BXVD_P_AVD0_CORE_CLK_CTRL,             /* uiCoreClkCtrl; */
   BXVD_P_AVD0_CORE_CLK_CTRL_PWRDN_MASK,  /* uiCoreClkCtrl_PwrDnMask */
   BXVD_P_AVD0_SCB_CLK_CTRL,              /* uiSCBClkCtrl */
   BXVD_P_AVD0_SCB_CLK_CTRL_PWRDWN_MASK,  /* uiSCBClkCtrl_PwrDnMask */
   BXVD_P_AVD0_GISB_CLK_CTRL,             /* uiGISBClkCtrl */
   BXVD_P_AVD0_GISB_CLK_CTRL_PWRDWN_MASK  /* uiGISBClkCtrl_PwrDnMask */
};
#endif

#ifdef BXVD_P_USE_INIT_REG_PTRS_7405B0
static const BXVD_P_PlatformReg_RevI0 s_stPlatformReg_7405 = 
{
   BCHP_SUN_TOP_CTRL_SW_RESET,                    /* SWReset */
   BCHP_SUN_TOP_CTRL_SW_RESET_avd0_sw_reset_MASK, /* SWResetAVDMask */	

   BCHP_AVD_INTR2_0_CPU_SET,                      /* CPUL2InterruptSet */

   BCHP_BVNF_INTR2_3_AVD0_STATUS,                 /* Bvnf_Intr2_3_AvdStatus */
   BCHP_BVNF_INTR2_3_AVD0_CLEAR,                  /* Bvnf_Intr2_3_AvdClear */
   BCHP_BVNF_INTR2_3_AVD0_MASK_CLEAR,             /* Bvnf_Intr2_3_AvdMaskClear */

   BCHP_XPT_PCROFFSET_STC0,                       /* XPT_PCROffset_STC */

   BCHP_INT_ID_AVD_SW_INTR0,          /* PicDataRdy */
   BCHP_INT_ID_AVD_SW_INTR3,          /* PicDataRdy1 */
   BCHP_INT_ID_AVD_MBOX_INTR,         /* Mailbox */
   BCHP_INT_ID_AVD_SW_INTR4,          /* StereoSeqError */
   BCHP_INT_ID_AVD_SW_INTR2,          /* StillPictureRdy */
   BCHP_INT_ID_AVD_WATCHDOG_INTR,     /* OuterWatchdog */
   BCHP_INT_ID_VICH_REG_INTR,         /* VIChkrReg */
   BCHP_INT_ID_VICH_SCB_WR_INTR,      /* VIChkrSCBWr */
   BCHP_INT_ID_VICH_INST_RD_INTR,     /* VIChkrInstrRead */

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

   BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH,                       /* StripeWidth */
   BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH_Stripe_Width_MASK,     /* StripeWidthMask */
   BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH_Stripe_Width_SHIFT,    /* StripeWidthShift */
   BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH_pfri_bank_height_MASK, /* PFRIBankHeightMask */
   BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH_pfri_bank_height_SHIFT, /* PFRIBankHeightShift */

   BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH,                       /* PFRIData */
   BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH_pfri_data_width_MASK,  /* PFRIDataBusWidthMask */
   BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH_pfri_data_width_SHIFT, /* PFRIDataBusWidthShift */
   BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH_pfri_source_MASK,      /* PFRIDataSourceMask */
   BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH_pfri_source_SHIFT,     /* PFRIDataSourceShift */

   BCHP_DECODE_RVC_0_REG_RVC_CTL,   /* RVC Control */
   BCHP_DECODE_RVC_0_REG_RVC_PUT,   /* RVC Put */
   BCHP_DECODE_RVC_0_REG_RVC_GET,   /* RVC Get */
   BCHP_DECODE_RVC_0_REG_RVC_BASE,  /* RVC Base */
   BCHP_DECODE_RVC_0_REG_RVC_END,   /* RVC End */

   BXVD_P_AVD0_CTRL,                      /* uiAVDCtrl */
   BXVD_P_AVD0_CTRL_PWRDN_MASK,           /* uiAVDCtrl_PwrDnMask */
   BXVD_P_AVD0_CORE_CLK_CTRL,             /* uiCoreClkCtrl; */
   BXVD_P_AVD0_CORE_CLK_CTRL_PWRDN_MASK,  /* uiCoreClkCtrl_PwrDnMask */
   BXVD_P_AVD0_SCB_CLK_CTRL,              /* uiSCBClkCtrl */
   BXVD_P_AVD0_SCB_CLK_CTRL_PWRDWN_MASK,  /* uiSCBClkCtrl_PwrDnMask */
   BXVD_P_AVD0_GISB_CLK_CTRL,             /* uiGISBClkCtrl */
   BXVD_P_AVD0_GISB_CLK_CTRL_PWRDWN_MASK  /* uiGISBClkCtrl_PwrDnMask */
};
#endif

#ifdef BXVD_P_USE_DETERMINE_STRIPE_INFO_7405
void BXVD_P_DetermineStripeInfo_7405( BCHP_DramType ddrType,
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

   /* Set stripe width and bank height base on bus width and memory size */
   switch(uiMemBusWidth)
   {
      /* Dram bus width 64 */
      case 64:

         switch(uiMemPartSize)
         {
            case 256: /* 256 Mbits Device Tech*/
               *puiStripeWidth =1; /* 64 bytes */
               *puiBankHeight = 1; /* 2 Mblks */
               break;

            case 512:  /* 512 Mbits Device Tech*/
            case 1024: /* 1024 Mbits Device Tech*/
            case 2048: /* 2048 Mbits Device Tech*/
            case 4096: /* 4096 Mbits Device Tech*/
            case 8192: /* 8192 Mbits Device Tech*/
               *puiStripeWidth =1; /* 128 bytes */
               *puiBankHeight = 2; /* 2 Mblks */
               break;

            default:
               *puiStripeWidth = 0;
               *puiBankHeight  = 1; /* 2 Mblks */
               BDBG_ERR(("Unknown MemPartSize: %d", uiMemPartSize));
               break;
         }

         break;

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
               BDBG_ERR(("Unknown MemPartSize: %d", uiMemPartSize));
               break;
         }

         break;

      /* Dram bus width 16 */
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
               *puiStripeWidth = 0; /* 64 bytes */
               *puiBankHeight  = 0; /* 1 Mblks */
               BDBG_ERR(("Unknown MemPartSize: %d", uiMemPartSize));
               break;
         }

         break;

      default:
         *puiStripeWidth = 0;
         *puiBankHeight  = 0;
         BDBG_ERR(("Unknown MemBusWidth: %d", uiMemBusWidth));
         break;
   }
}
#endif

#ifdef BXVD_P_USE_INIT_REG_PTRS_7405A0
void BXVD_P_InitRegPtrs_7405(BXVD_Handle hXvd)
{
   BDBG_ASSERT(hXvd);

   BDBG_ENTER(BXVD_P_InitRegPtrs_7405);

   /* Platform Info */
   hXvd->stPlatformInfo.stFirmware.outerELFFirmwareID =
      BXVD_IMAGE_FirmwareID_eOuterELF_AVD0;
   hXvd->stPlatformInfo.stFirmware.innerELFFirmwareID = 
      BXVD_IMAGE_FirmwareID_eInnerELF_AVD0;
   hXvd->stPlatformInfo.stFirmware.authenticatedFirmwareID =
      BXVD_IMAGE_FirmwareID_eAuthenticated_AVD0;

   hXvd->stPlatformInfo.stReg = s_stPlatformReg_7405;

   /* 
    * Get stripe width and bank height values from their respective registers.
    */
   hXvd->uiDecode_SDStripeWidthRegVal = BXVD_Reg_Read32(hXvd, 
                                                        hXvd->stPlatformInfo.stReg.uiDecode_StripeWidth);
   
   hXvd->uiDecode_StripeWidth = ((hXvd->uiDecode_SDStripeWidthRegVal &
                                  hXvd->stPlatformInfo.stReg.uiDecode_StripeWidthMask) >>
                                 hXvd->stPlatformInfo.stReg.uiDecode_StripeWidthShift);
   
   hXvd->uiDecode_IPShimPfriRegVal = BXVD_Reg_Read32(hXvd, 
                                                     hXvd->stPlatformInfo.stReg.uiDecode_PFRI);
   
   hXvd->uiDecode_StripeMultiple = ((hXvd->uiDecode_IPShimPfriRegVal &
                                     hXvd->stPlatformInfo.stReg.uiDecode_PFRIBankHeightMask) >>
                                    hXvd->stPlatformInfo.stReg.uiDecode_PFRIBankHeightShift);
   
   BXVD_DBG_MSG(hXvd, ("stripe width = %d - stripe multiple = %d",
                            hXvd->uiDecode_StripeWidth, 
                            hXvd->uiDecode_StripeMultiple));

   BDBG_LEAVE(BXVD_P_InitRegPtrs_7405);
}
#endif

#ifdef BXVD_P_USE_INIT_REG_PTRS_7405B0
void BXVD_P_InitRegPtrs_7405(BXVD_Handle hXvd)
{
   uint32_t uiDecode_DataWidth;
   uint32_t uiDecode_DataSource;

   BDBG_ASSERT(hXvd);

   BDBG_ENTER(BXVD_P_InitRegPtrs_7405);

   /* Platform Info */
   hXvd->stPlatformInfo.stFirmware.outerELFFirmwareID =
      BXVD_IMAGE_FirmwareID_eOuterELF_AVD0;
   hXvd->stPlatformInfo.stFirmware.innerELFFirmwareID = 
      BXVD_IMAGE_FirmwareID_eInnerELF_AVD0;
   hXvd->stPlatformInfo.stFirmware.authenticatedFirmwareID =
      BXVD_IMAGE_FirmwareID_eAuthenticated_AVD0;

   hXvd->stPlatformInfo.stReg = s_stPlatformReg_7405;

   /* 
    * Get stripe width and bank height values from their respective registers.
    */
   hXvd->uiDecode_SDStripeWidthRegVal = BXVD_Reg_Read32(hXvd, 
                                                        hXvd->stPlatformInfo.stReg.uiDecode_StripeWidth);

   hXvd->uiDecode_StripeWidth = (hXvd->uiDecode_SDStripeWidthRegVal &
                                  hXvd->stPlatformInfo.stReg.uiDecode_StripeWidthMask) >>
                                 hXvd->stPlatformInfo.stReg.uiDecode_StripeWidthShift;
   
   hXvd->uiDecode_StripeMultiple = ((hXvd->uiDecode_SDStripeWidthRegVal &
                                     hXvd->stPlatformInfo.stReg.uiDecode_PFRIBankHeightMask) >>
                                    hXvd->stPlatformInfo.stReg.uiDecode_PFRIBankHeightShift);
   
   hXvd->uiDecode_PFRIDataRegVal = BXVD_Reg_Read32(hXvd, 
                                               hXvd->stPlatformInfo.stReg.uiDecode_PFRIData);

   uiDecode_DataWidth = (hXvd->uiDecode_PFRIDataRegVal  & 
                         hXvd->stPlatformInfo.stReg.uiDecode_PFRIDataBusWidthMask) >>
                        hXvd->stPlatformInfo.stReg.uiDecode_PFRIDataBusWidthShift;

   uiDecode_DataSource = (hXvd->uiDecode_PFRIDataRegVal  & 
                         hXvd->stPlatformInfo.stReg.uiDecode_PFRIDataSourceMask) >>
                        hXvd->stPlatformInfo.stReg.uiDecode_PFRIDataSourceShift;

   BXVD_DBG_MSG(hXvd, ("PFRI Data width = %d - PFRI Data Source = %d",
                            uiDecode_DataWidth, uiDecode_DataSource));

   BXVD_DBG_MSG(hXvd, ("stripe width = %d - stripe multiple = %d",
                            hXvd->uiDecode_StripeWidth, 
                            hXvd->uiDecode_StripeMultiple));

   BDBG_LEAVE(BXVD_P_InitRegPtrs_7405);
}
#endif /* BXVD_P_USE_INIT_REG_PTRS_7405B0 */


#endif /* BXVD_P_USE_INIT_REG_PTRS_7405 */

#ifdef BXVD_P_USE_CORE_RESET_CHIP_7405

#ifdef BXVD_P_USE_INIT_REG_PTRS_7405A0
BERR_Code BXVD_P_ChipReset_7405(BXVD_Handle hXvd)
{
   BDBG_ASSERT(hXvd);

   BDBG_ENTER(BXVD_P_ChipReset_7405);

   /* Reset Video Decoder */

   BREG_AtomicUpdate32(hXvd->hReg, 
                       hXvd->stPlatformInfo.stReg.uiSun_SWReset,
                       hXvd->stPlatformInfo.stReg.uiSun_SWResetAVDMask,
                       hXvd->stPlatformInfo.stReg.uiSun_SWResetAVDMask);

   BREG_AtomicUpdate32(hXvd->hReg, 
                       hXvd->stPlatformInfo.stReg.uiSun_SWReset,
                       hXvd->stPlatformInfo.stReg.uiSun_SWResetAVDMask,
                       0);

   /* Clear interrupt mask (Enable) register to prevent interrupts before ISR is setup */
   BXVD_Reg_Write32(hXvd, 
                    hXvd->stPlatformInfo.stReg.uiDecode_OuterInterruptMask,
                    0);

   /* write the stripe with and bank height to hardware */

   BREG_AtomicUpdate32(hXvd->hReg, 
                       hXvd->stPlatformInfo.stReg.uiDecode_StripeWidth,
                       0xffffffff,
                       hXvd->uiDecode_SDStripeWidthRegVal);


   BREG_AtomicUpdate32(hXvd->hReg,
                       hXvd->stPlatformInfo.stReg.uiDecode_PFRI,
                       0xffffffff,
                       hXvd->uiDecode_IPShimPfriRegVal);

   BDBG_LEAVE(BXVD_P_ChipReset_7405);

   return BERR_TRACE(BERR_SUCCESS);
}
#endif

#ifdef BXVD_P_USE_INIT_REG_PTRS_7405B0
BERR_Code BXVD_P_ChipReset_7405(BXVD_Handle hXvd)
{
   BDBG_ASSERT(hXvd);

   BDBG_ENTER(BXVD_P_ChipReset_7405);


   /* Stop OL AVD ARC */
   BXVD_Reg_Write32(hXvd, 
                    hXvd->stPlatformInfo.stReg.uiDecode_OuterCPUDebug,
                    1);
   BXVD_Reg_Write32(hXvd, 
                    hXvd->stPlatformInfo.stReg.uiDecode_OuterCPUAux,
                    0x02000000); 

   /* Stop IL AVD ARC */
   BXVD_Reg_Write32(hXvd, 
                    hXvd->stPlatformInfo.stReg.uiDecode_InnerCPUDebug,
                    1);
   BXVD_Reg_Write32(hXvd, 
                    hXvd->stPlatformInfo.stReg.uiDecode_InnerCPUAux,
                    0x02000000); 

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
                      "Backend" )

   /* Reset Video Decoder */

   BREG_AtomicUpdate32(hXvd->hReg, 
                       hXvd->stPlatformInfo.stReg.uiSun_SWReset,
                       hXvd->stPlatformInfo.stReg.uiSun_SWResetAVDMask,
                       hXvd->stPlatformInfo.stReg.uiSun_SWResetAVDMask);

   BREG_AtomicUpdate32(hXvd->hReg, 
                       hXvd->stPlatformInfo.stReg.uiSun_SWReset,
                       hXvd->stPlatformInfo.stReg.uiSun_SWResetAVDMask,
                       0);

   /* Clear interrupt mask (Enable) register to prevent interrupts before ISR is setup */
   BXVD_Reg_Write32(hXvd, 
                    hXvd->stPlatformInfo.stReg.uiDecode_OuterInterruptMask,
                    0);

   /* write the PFRI data source and data width values */
   BREG_AtomicUpdate32(hXvd->hReg, 
                       hXvd->stPlatformInfo.stReg.uiDecode_PFRIData,
                       0xffffffff,
                       hXvd->uiDecode_PFRIDataRegVal);

   /* write the stripe with and bank height to hardware */

   BREG_AtomicUpdate32(hXvd->hReg, 
                       hXvd->stPlatformInfo.stReg.uiDecode_StripeWidth,
                       0xffffffff,
                       hXvd->uiDecode_SDStripeWidthRegVal);

   BDBG_LEAVE(BXVD_P_ChipReset_7405);

   return BERR_TRACE(BERR_SUCCESS);
}
#endif
#endif /* BXVD_P_USE_CORE_CHIP_RESET_7405 */




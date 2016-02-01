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
#include "bchp_int_id_avd_intr2_0.h"

/* to pickup BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CHANNEL_WR_POSITION_i_ARRAY_BASE */
#include "bchp_reg_cabac2bins2_0.h"
#include "bchp_decode_rvc_0.h"

#include "bchp_avd_cache_0.h"

/* TBD Power management registers */
#if 0
#include "bchp_vcxo_ctl_misc.h"
#include "bchp_clkgen.h"
#endif
BDBG_MODULE(BXVD_PLATFORM_REVJ0);

#ifdef BXVD_P_USE_INIT_REG_PTRS_REVJ0
static const BXVD_P_PlatformReg_RevJ0 s_stPlatformReg_RevJ0 = 
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

#if (!BXVD_P_DVD_CHIP)
   BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH,                       /* StripeWidth */
   BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH_Stripe_Width_MASK,     /* StripeWidthMask */
   BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH_Stripe_Width_SHIFT,    /* StripeWidthShift */
   BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH_pfri_bank_height_MASK, /* PFRIBankHeightMask */
   BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH_pfri_bank_height_SHIFT, /* PFRIBankHeightShift */

   BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH,                       /* PFRIData */
   BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH_ddr3_mode_MASK,        /* PFRIDataDDR3ModeMask */
   BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH_ddr3_mode_SHIFT,       /* PFRIDataDDR3ModeShift */
   BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH_pfri_data_width_MASK,  /* PFRIDataBusWidthMask */
   BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH_pfri_data_width_SHIFT, /* PFRIDataBusWidthShift */
   BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH_pfri_source_MASK,      /* PFRIDataSourceMask */
   BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH_pfri_source_SHIFT,     /* PFRIDataSourceShift */
#else
   (uint32_t) NULL,
   (uint32_t) NULL,
   (uint32_t) NULL,
   (uint32_t) NULL,
   (uint32_t) NULL,

   (uint32_t) NULL,
   (uint32_t) NULL,
   (uint32_t) NULL,
   (uint32_t) NULL,
   (uint32_t) NULL,
   (uint32_t) NULL,
   (uint32_t) NULL,
#endif
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

   (uint32_t)NULL,  /* uiAVDCtrl */
   (uint32_t)NULL,  /* uiAVDCtrl_PwrDnMask */
   (uint32_t)NULL,  /* uiCoreClkCtrl; */
   (uint32_t)NULL,  /* uiCoreClkCtrl_PwrDnMask */
   (uint32_t)NULL,  /* uiSCBClkCtrl */
   (uint32_t)NULL,  /* uiSCBClkCtrl_PwrDnMask */
   (uint32_t)NULL,  /* uiGISBClkCtrl */
   (uint32_t)NULL   /* uiGISBClkCtrl_PwrDnMask */
};
#endif

#ifdef BXVD_P_USE_INIT_REG_PTRS_REVJ0
void BXVD_P_InitRegPtrs_RevJ0(BXVD_Handle hXvd)
{

#if (!BXVD_P_DVD_CHIP)
   uint32_t uiDecode_DataWidth;
   uint32_t uiDecode_DataSource;
#endif

   BDBG_ASSERT(hXvd);

   BDBG_ENTER(BXVD_P_InitRegPtrs_RevJ0);

   /* Platform Info */
   hXvd->stPlatformInfo.stFirmware.outerELFFirmwareID =
      BXVD_IMAGE_FirmwareID_eOuterELF_AVD0;
   hXvd->stPlatformInfo.stFirmware.innerELFFirmwareID = 
      BXVD_IMAGE_FirmwareID_eInnerELF_AVD0;
   hXvd->stPlatformInfo.stFirmware.authenticatedFirmwareID =
      BXVD_IMAGE_FirmwareID_eAuthenticated_AVD0;

   hXvd->stPlatformInfo.stReg = s_stPlatformReg_RevJ0;

#if (!BXVD_P_DVD_CHIP)
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
#endif

   hXvd->uiAVD_PCacheRegVal = BXVD_Reg_Read32(hXvd, 
                                              hXvd->stPlatformInfo.stReg.uiAVD_PcacheMode);

   BDBG_LEAVE(BXVD_P_InitRegPtrs_RevJ0);
}
#endif /* BXVD_P_USE_INIT_REG_PTRS_REVJ0 */

#ifdef BXVD_P_USE_CORE_RESET_CHIP_REVJ0 
BERR_Code BXVD_P_ChipReset_RevJ0(BXVD_Handle hXvd)
{

   uint32_t uiPCacheVal;

#if (!BXVD_P_DVD_CHIP)
   uint32_t uiDataWidth;
   uint32_t uiDDRMode;
   uint32_t uiXGran=0;
   uint32_t uiYGran=0;
#endif

   BDBG_ASSERT(hXvd);

   BDBG_ENTER(BXVD_P_ChipReset_RevJ0);

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

#if (!BXVD_P_DVD_CHIP)
   /* Write the PFRI data source and data width values */
   BREG_AtomicUpdate32(hXvd->hReg, 
                       hXvd->stPlatformInfo.stReg.uiDecode_PFRIData,
                       0xffffffff,
                       hXvd->uiDecode_PFRIDataRegVal);

   /* Write the stripe with and bank height to hardware */
   BREG_AtomicUpdate32(hXvd->hReg, 
                       hXvd->stPlatformInfo.stReg.uiDecode_StripeWidth,
                       0xffffffff,
                       hXvd->uiDecode_SDStripeWidthRegVal);

   uiDataWidth = (hXvd->uiDecode_PFRIDataRegVal  & 
                         hXvd->stPlatformInfo.stReg.uiDecode_PFRIDataBusWidthMask) >>
                        hXvd->stPlatformInfo.stReg.uiDecode_PFRIDataBusWidthShift;

   uiDDRMode = (hXvd->uiDecode_PFRIDataRegVal  & 
                        hXvd->stPlatformInfo.stReg.uiDecode_PFRIDataDDR3ModeMask) >>
                        hXvd->stPlatformInfo.stReg.uiDecode_PFRIDataDDR3ModeShift;

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

         case BXVD_P_PFRI_Data_Width_e64Bit:

            uiXGran = BXVD_P_PCache_XGran_e8Bytes;
            uiYGran = BXVD_P_PCache_YGran_e4Lines;

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

   uiPCacheVal |= ((uiYGran << hXvd->stPlatformInfo.stReg.uiAVD_PcacheModeYGranShift) |
                   (uiXGran << hXvd->stPlatformInfo.stReg.uiAVD_PcacheModeXGranShift));
                    
#else

   uiPCacheVal = hXvd->uiAVD_PCacheRegVal;

#endif

   /* Write the AVD PCache mode to hardware */
   BREG_AtomicUpdate32(hXvd->hReg, 
                       hXvd->stPlatformInfo.stReg.uiAVD_PcacheMode,
                       0xffffffff,
                       uiPCacheVal);

   BDBG_LEAVE(BXVD_P_ChipReset_RevJ0);

   return BERR_TRACE(BERR_SUCCESS);
}
#endif

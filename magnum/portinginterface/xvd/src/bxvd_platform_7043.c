/***************************************************************************
 *     Copyright (c) 2006-2010, Broadcom Corporation
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
#include "bxvd_priv.h"
#include "bxvd_intr.h"
#include "bxvd_reg.h"
#include "bxvd_platform.h"
#include "bxvd_image.h"
#include "bxvd_image_priv.h"
#include "bchp_common.h"

#include "bchp_decode_sd_0.h" 
#include "bchp_decode_ip_shim_0.h"
#include "bchp_sun_gisb_arb.h"

#include "bchp_reg_cabac2bins2_0.h"
#include "bchp_decode_rvc_0.h"


#ifdef BXVD_P_USE_INIT_REG_PTRS_7043

static const BXVD_P_PlatformReg_RevH0 s_stPlatformReg_7043 = 
{
   BCHP_SUN_TOP_CTRL_SW_RESET,                    /* SWReset */
   BCHP_SUN_TOP_CTRL_SW_RESET_avd0_sw_reset_MASK, /* SWResetAVDMask */	

   BCHP_AVD_INTR2_0_CPU_SET,                      /* CPUL2InterruptSet */
   BCHP_AVD_INTR2_0_CPU_CLEAR,                    /* CPUL2InterruptClear */
   BCHP_AVD_INTR2_0_CPU_CLEAR_AVD_MBOX_INTR_MASK, /* CPUL2InterruptMailboxMask */

   0 , /* BCHP_BVNF_INTR2_3_AVD0_STATUS,        / * Bvnf_Intr2_3_AvdStatus */
   0 , /* BCHP_BVNF_INTR2_3_AVD0_CLEAR,         / * Bvnf_Intr2_3_AvdClear */
   0 , /* BCHP_BVNF_INTR2_3_AVD0_MASK_CLEAR,    / * Bvnf_Intr2_3_AvdMaskClear */

   BCHP_XPT_PCROFFSET_STC0,                       /* XPT_PCROffset_STC */

   BCHP_INT_ID_AVD_SW_INTR0,          /* PicDataRdy */
   BCHP_INT_ID_AVD_SW_INTR3,          /* PicDataRdy1 */
   BCHP_INT_ID_AVD_MBOX_INTR,         /* Mailbox */
   BCHP_INT_ID_AVD_SW_INTR4,          /* StereoSeqError */
   BCHP_INT_ID_AVD_SW_INTR2,          /* StillPictureRdy */
   0 , /* BCHP_INT_ID_AVD_WATCHDOG_INTR   / * OuterWatchdog */
   BCHP_INT_ID_VICH_REG_INTR,         /* VIChkrReg */
   BCHP_INT_ID_VICH_SCB_WR_INTR,      /* VIChkrSCBWr */
   BCHP_INT_ID_VICH_INST_RD_INTR,     /* VIChkrInstrRead */

   BCHP_DECODE_CPUREGS_0_REG_INST_BASE,         /* InnerInstructionBase */
   BCHP_DECODE_CPUREGS_0_REG_END_OF_CODE,       /* InnerEndOfCode */
   BCHP_DECODE_CPUREGS_0_REG_GLOBAL_IO_BASE,    /* InnerGlobalIOBase */

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

   BCHP_SUN_GISB_ARB_REQ_MASK,                    /* SunGisbArb_ReqMask */
   BCHP_SUN_GISB_ARB_REQ_MASK_avd_MASK,           /* SunGisbArb_ReqMaskAVDMask */

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
};
#endif


#ifdef BXVD_P_USE_INIT_REG_PTRS_7043
void BXVD_P_InitRegPtrs_7043(BXVD_Handle hXvd)
{
   BDBG_ASSERT(hXvd);

   BDBG_ENTER(BXVD_P_InitRegPtrs_7043);

   /* Platform Info */
   hXvd->stPlatformInfo.stFirmware.outerELFFirmwareID =
      BXVD_IMAGE_FirmwareID_eOuterELF_AVD0;
   hXvd->stPlatformInfo.stFirmware.innerELFFirmwareID = 
      BXVD_IMAGE_FirmwareID_eInnerELF_AVD0;
   hXvd->stPlatformInfo.stFirmware.authenticatedFirmwareID =
      BXVD_IMAGE_FirmwareID_eAuthenticated_AVD0;

   hXvd->stPlatformInfo.stReg = s_stPlatformReg_7043;

   BDBG_LEAVE(BXVD_P_InitRegPtrs_7043);
}
#endif

#ifdef BXVD_P_USE_INIT_REG_PTRS_7043
BERR_Code BXVD_P_ChipReset_7043(BXVD_Handle hXvd)
{
   BDBG_ASSERT(hXvd);

   BDBG_ENTER(BXVD_P_ChipReset_7043);

   /* write the stripe with and bank height to hardware */

   BXVD_DBG_ERR(hXvd, ("Writing StripeWidth and PFRI registers"));

   BREG_Write32(hXvd->hReg,
                       hXvd->stPlatformInfo.stReg.uiDecode_StripeWidth,
                       hXvd->uiDecode_SDStripeWidthRegVal);

   BREG_Write32(hXvd->hReg,
                       hXvd->stPlatformInfo.stReg.uiDecode_PFRI,
                       hXvd->uiDecode_IPShimPfriRegVal);
#if 0
   BREG_AtomicUpdate32(hXvd->hReg, 
                       hXvd->stPlatformInfo.stReg.uiDecode_StripeWidth,
                       0xffffffff,
                       hXvd->uiDecode_SDStripeWidthRegVal);


   BREG_AtomicUpdate32(hXvd->hReg,
                       hXvd->stPlatformInfo.stReg.uiDecode_PFRI,
                       0xffffffff,
                       hXvd->uiDecode_IPShimPfriRegVal);
#endif
   BDBG_LEAVE(BXVD_P_ChipReset_7043);

   return BERR_TRACE(BERR_SUCCESS);
}
#endif


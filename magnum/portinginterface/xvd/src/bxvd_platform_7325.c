/***************************************************************************
 *     Copyright (c) 2006-2011, Broadcom Corporation
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

/* Local constants */
#define BXVD_REVE0_OL_INIT_TIMEOUT 1000         /* One second init timeout */
#define BXVD_REVE0_CORE_BAUD_RATE 115200        /* OL & IL UART baud rate */
#define BXVD_REVE0_CORE_UART_FREQ (250*1000000) /* UART clock frequency */
#define BXVD_PRIV_USE_CACHED_MEMORY 0           /* Turn caching on or off */

BDBG_MODULE(BXVD_PLATFORM_7325);

#ifdef BXVD_P_USE_INIT_REG_PTRS_7405A0

static const BXVD_P_PlatformReg_RevE0 s_stPlatformReg_7405A0 = 
{
	BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL,              /* TestPortControl */
	TEST_PORT_CONTROL_VALUE_ARC_0,                 /* TestPortControlAVDValue */	
	BCHP_SUN_TOP_CTRL_SW_RESET,                    /* SWReset */
	BCHP_SUN_TOP_CTRL_SW_RESET_avd0_sw_reset_MASK, /* SWResetAVDMask */	

	BCHP_AVD_INTR2_0_CPU_CLEAR,                    /* CPUL2InterruptClear */
	BCHP_AVD_INTR2_0_CPU_CLEAR_AVD_MBOX_INTR_MASK, /* CPUL2InterruptMailboxMask */

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

	BCHP_DECODE_CPUREGS2_0_REG_INST_BASE,        /* OuterEndOfCode */
	BCHP_DECODE_CPUREGS2_0_REG_END_OF_CODE,      /* OuterInstructionBase */
	BCHP_DECODE_CPUREGS2_0_REG_GLOBAL_IO_BASE,   /* OuterGlobalIOBase */
	BCHP_DECODE_IND_SDRAM_REGS2_0_REG_CPU_DBG,   /* OuterCPUDebug */
	BCHP_DECODE_CPUAUX2_0_CPUAUX_REG,            /* OuterCPUAux */
	BCHP_DECODE_CPUREGS2_0_REG_CPU_INTGEN_MASK,  /* OuterInterruptMask */
	BCHP_DECODE_CPUREGS2_0_REG_CPU_INTGEN_CLR,   /* OuterInterruptClear */
	0xffffffff,                                  /* OuterWatchdogTimerLimit */
	BCHP_DECODE_CPUREGS2_0_REG_WATCHDOG_TMR,     /* OuterWatchdogTimer */
	BCHP_DECODE_CPUREGS2_0_REG_CPU2HST_MBX,      /* OuterCPU2HostMailbox */
	BCHP_DECODE_CPUREGS2_0_REG_HST2CPU_MBX,      /* OuterHost2CPUMailbox */
	BCHP_REG_CABAC2BINS2_0_REG_CABAC2BINS_CHANNEL_WR_POSITION_i_ARRAY_BASE    /* CabacBinDepth */	

        BCHP_SUN_GISB_ARB_REQ_MASK,                  /* SunGisbArb_ReqMask */
        BCHP_SUN_GISB_ARB_REQ_MASK_avd0_MASK,        /* SunGisbArb_ReqMaskAVDMask */

        BCHP_DECODE_RVC_0_REG_RVC_CTL,   /* RVC Control */
        BCHP_DECODE_RVC_0_REG_RVC_PUT,   /* RVC Put */
        BCHP_DECODE_RVC_0_REG_RVC_GET,   /* RVC Get */
        BCHP_DECODE_RVC_0_REG_RVC_BASE,  /* RVC Base */
        BCHP_DECODE_RVC_0_REG_RVC_END,   /* RVC End */
};
 
void BXVD_P_InitRegPtrs_7405A0(BXVD_Handle hXvd)
{
   BDBG_ASSERT(hXvd);

   BDBG_ENTER(BXVD_P_InitRegPtrs_7405A0);

   /* Platform Info */
   hXvd->stPlatformInfo.stFirmware.outerELFFirmwareID =
      BXVD_IMAGE_FirmwareID_eOuterELF_AVD0;
   hXvd->stPlatformInfo.stFirmware.innerELFFirmwareID = 
      BXVD_IMAGE_FirmwareID_eInnerELF_AVD0;
   hXvd->stPlatformInfo.stFirmware.authenticatedFirmwareID =
      BXVD_IMAGE_FirmwareID_eAuthenticated_AVD0;

   hXvd->stPlatformInfo.stReg = s_stPlatformReg_7405A0;

   /* 
    * Get stripe width and bank height values from their respective registers.
    */
   hXvd->uiDecode_SDStripeWidthRegVal = BXVD_Reg_Read32(hXvd, 
                                                        BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH);
   
   hXvd->uiDecode_StripeWidth = ((hXvd->uiDecode_SDStripeWidthRegVal &
                                  BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH_Strip_Width_MASK) >>
                                 BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH_Strip_Width_SHIFT);
   
   hXvd->uiDecode_IPShimPfriRegVal = BXVD_Reg_Read32(hXvd, 
                                                     BCHP_DECODE_IP_SHIM_0_PFRI_REG);
   
   hXvd->uiDecode_StripeMultiple = ((hXvd->uiDecode_IPShimPfriRegVal &
                                     BCHP_DECODE_IP_SHIM_0_PFRI_REG_pfri_bank_height_MASK) >>
                                    BCHP_DECODE_IP_SHIM_0_PFRI_REG_pfri_bank_height_SHIFT);
   
   BXVD_DBG_MSG(hXvd, ("stripe width = %d - stripe multiple = %d",
                            hXvd->uiDecode_StripeWidth, 
                            hXvd->uiDecode_StripeMultiple));

   BDBG_LEAVE(BXVD_P_InitRegPtrs_7405A0);
}

#endif /* BXVD_P_USE_INIT_REG_PTRS_7405A0 */

#ifdef BXVD_P_USE_CORE_RESET_CHIP_7405

BERR_Code BXVD_P_ChipReset_7405(BXVD_Handle hXvd)
{
   volatile uint32_t uIntrVal;

   BDBG_ASSERT(hXvd);

   BDBG_ENTER(BXVD_P_ChipReset_7405);

   /* Reset Video Decoder */
   uIntrVal = BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiSun_SWReset);

   uIntrVal |= hXvd->stPlatformInfo.stReg.uiSun_SWResetAVDMask;
   BXVD_Reg_Write32(hXvd, hXvd->stPlatformInfo.stReg.uiSun_SWReset, uIntrVal);

   uIntrVal &= ~hXvd->stPlatformInfo.stReg.uiSun_SWResetAVDMask;
   BXVD_Reg_Write32(hXvd, hXvd->stPlatformInfo.stReg.uiSun_SWReset, uIntrVal);

   /* Clear interrupt mask (Enable) register to prevent interrupts before ISR is setup */
   BXVD_Reg_Write32(hXvd, 
                    hXvd->stPlatformInfo.stReg.uiDecode_OuterInterruptMask,
                    0);

   /* write the stripe with and bank height to hardware */
   BXVD_Reg_Write32(hXvd, 
                    BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH,
                    hXvd->uiDecode_SDStripeWidthRegVal);

   BXVD_Reg_Write32(hXvd, BCHP_DECODE_IP_SHIM_0_PFRI_REG,
                    hXvd->uiDecode_IPShimPfriRegVal);


   BDBG_LEAVE(BXVD_P_ChipReset_7405);

   return BERR_TRACE(BERR_SUCCESS);
}

#endif /* BXVD_P_USE_CORE_CHIP_RESET_7405 */

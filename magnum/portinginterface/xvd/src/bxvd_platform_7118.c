/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
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
/* to pickup BCHP_REG_CABAC2BINS2_REG_CABAC2BINS_CHANNEL_WR_POSITION_i_ARRAY_BASE */
#include "bchp_reg_cabac2bins2.h"

#include "bchp_decode_ind_sdram_regs.h"
#include "bchp_decode_sint.h"
#include "bchp_decode_sint_oloop.h"
#include "bchp_decode_main.h"

#include "bchp_decode_rvc.h"

BDBG_MODULE(BXVD_PLATFORM_7118);

#ifdef BXVD_P_USE_INIT_REG_PTRS_7118A0

static const BXVD_P_PlatformReg_RevE0 s_stPlatformReg_7118A0 = 
{
	BCHP_SUN_TOP_CTRL_SW_RESET,                   /* SWReset */
	BCHP_SUN_TOP_CTRL_SW_RESET_avd_sw_reset_MASK, /* SWResetAVDMask */	

	BCHP_AVD0_INTR2_CPU_SET,                      /* CPUL2InterruptSet */
	BCHP_AVD0_INTR2_CPU_CLEAR,                    /* CPUL2InterruptClear */
	BCHP_AVD0_INTR2_CPU_CLEAR_AVD_MBOX_INTR_MASK, /* CPUL2InterruptMailboxMask */

        BCHP_BVNF_INTR2_3_AVD0_STATUS,                 /* Bvnf_Intr2_3_AvdStatus */
        BCHP_BVNF_INTR2_3_AVD0_CLEAR,                  /* Bvnf_Intr2_3_AvdClear */
        BCHP_BVNF_INTR2_3_AVD0_MASK_CLEAR,             /* Bvnf_Intr2_3_AvdMaskClear */

        BCHP_XPT_PCROFFSET0_STC,                       /* XPT_PCRPffset_STC */

	BCHP_INT_ID_AVD_SW_INTR10,  /* PicDataRdy */
        BCHP_INT_ID_AVD_SW_INTR13,  /* PicDataRdy1 */
	BCHP_INT_ID_AVD_SW_INTR11,  /* Mailbox */
	BCHP_INT_ID_AVD_SW_INTR14,  /* StereoSeqError */
	BCHP_INT_ID_AVD_SW_INTR12,  /* StillPictureRdy */
	BCHP_INT_ID_AVD_MBOX_INTR,  /* OuterWatchdog */
	BCHP_INT_ID_VICH_REG_INTR,     /* VIChkrReg */
	BCHP_INT_ID_VICH_SCB_WR_INTR,  /* VIChkrSCBWr */
	BCHP_INT_ID_VICH_INST_RD_INTR, /* VIChkrInstrRead */
	
	BCHP_DECODE_CPUREGS_REG_INST_BASE,           /* InnerInstructionBase */
	BCHP_DECODE_CPUREGS_REG_END_OF_CODE,         /* InnerEndOfCode */
	BCHP_DECODE_CPUREGS_REG_GLOBAL_IO_BASE,      /* InnerGlobalIOBase */
        BCHP_DECODE_IND_SDRAM_REGS_REG_CPU_DBG,      /* InnerCPUDebug */
        BCHP_DECODE_CPUAUX_CPUAUX_REG,               /* InnerCPUAux */

	BCHP_DECODE_CPUREGS2_REG_INST_BASE,          /* OuterEndOfCode */
	BCHP_DECODE_CPUREGS2_REG_END_OF_CODE,        /* OuterInstructionBase */
	BCHP_DECODE_CPUREGS2_REG_GLOBAL_IO_BASE,     /* OuterGlobalIOBase */
	BCHP_DECODE_IND_SDRAM_REGS2_REG_CPU_DBG,     /* OuterCPUDebug */
	BCHP_DECODE_CPUAUX2_CPUAUX_REG,              /* OuterCPUAux */
	BCHP_DECODE_CPUREGS2_REG_CPU_INTGEN_MASK,    /* OuterInterruptMask */
	BCHP_DECODE_CPUREGS2_REG_CPU_INTGEN_CLR,     /* OuterInterruptClear */
	BCHP_DECODE_CPUREGS2_REG_WATCHDOG_TMR_LIMIT, /* OuterWatchdogTimerLimit */
	BCHP_DECODE_CPUREGS2_REG_WATCHDOG_TMR,       /* OuterWatchdogTimer */
	BCHP_DECODE_CPUREGS2_REG_CPU2HST_MBX,        /* OuterCPU2HostMailbox */
	BCHP_DECODE_CPUREGS2_REG_HST2CPU_MBX,        /* OuterHost2CPUMailbox */
        BCHP_DECODE_CPUREGS2_REG_CPU2HST_STAT,       /* OuterCPU2HostStatus */

	BCHP_REG_CABAC2BINS2_REG_CABAC2BINS_CHANNEL_WR_POSITION_i_ARRAY_BASE,    /* CabacBinDepth */

        BCHP_REG_CABAC2BINS2_REG_CABAC2BINS_CTL,            /* uiDecode_CabacBinCtl */
        BCHP_REG_CABAC2BINS2_REG_CABAC2BINS_CTL_Reset_MASK, /* uiDecode_CabacBinCtl_ResetMask */
        BCHP_DECODE_SINT_REG_SINT_STRM_STAT,                /* uiDecode_SintStrmSts */
        BCHP_DECODE_SINT_REG_SINT_STRM_STAT_Rst_MASK,       /* uiDecode_SintStrmSts_ResetMask */
        BCHP_DECODE_SINT_OLOOP_DEC_SINT_STRM_STAT,          /* uiDecode_OLSintStrmSts */
        BCHP_DECODE_SINT_OLOOP_DEC_SINT_STRM_STAT_Rst_MASK, /* uiDecode_OLSintStrmSts_ResetMask */
        BCHP_DECODE_MAIN_REG_MAINCTL,                       /* uiDecode_MainCtl */
        BCHP_DECODE_MAIN_REG_MAINCTL_Rst_MASK,              /* uiDecode_MainCtl_ResetMask */

        BCHP_SUN_GISB_ARB_REQ_MASK,                    /* SunGisbArb_ReqMask */
        BCHP_SUN_GISB_ARB_REQ_MASK_req_mask_7_MASK,    /* SunGisbArb_ReqMaskAVDMask */

        BCHP_DECODE_RVC_REG_RVC_CTL,   /* RVC Control */
        BCHP_DECODE_RVC_REG_RVC_PUT,   /* RVC Put */
        BCHP_DECODE_RVC_REG_RVC_GET,   /* RVC Get */
        BCHP_DECODE_RVC_REG_RVC_BASE,  /* RVC Base */
        BCHP_DECODE_RVC_REG_RVC_END,   /* RVC End */
};

void BXVD_P_InitRegPtrs_7118A0(BXVD_Handle hXvd)
{
   BDBG_ASSERT(hXvd);

   BDBG_ENTER(BXVD_P_InitRegPtrs_7118A0);

   BXVD_DBG_MSG(hXvd, ("BXVD_P_InitRegPtrs_7118A0"));

   /* Platform Info */
   hXvd->stPlatformInfo.stFirmware.outerELFFirmwareID = BXVD_IMAGE_FirmwareID_eOuterELF_AVD0;
   hXvd->stPlatformInfo.stFirmware.innerELFFirmwareID = BXVD_IMAGE_FirmwareID_eInnerELF_AVD0;
   hXvd->stPlatformInfo.stFirmware.authenticatedFirmwareID = BXVD_IMAGE_FirmwareID_eAuthenticated_AVD0;

   hXvd->stPlatformInfo.stReg = s_stPlatformReg_7118A0;

   BDBG_LEAVE(BXVD_P_InitRegPtrs_7118A0);
}

#endif /* BXVD_P_USE_INIT_REG_PTRS_7118A0 */

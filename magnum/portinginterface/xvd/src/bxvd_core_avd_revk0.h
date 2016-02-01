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
#ifndef _BXVD_CORE_AVD_REVK0_H_
#define _BXVD_CORE_AVD_REVK0_H_

#define  BXVD_P_FW_HIM_API 1

#include "bxvd_image.h"
#include "bxvd_vdec_info.h"
#include "bxvd_him.h"
#include "bxvd_core_avd_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BXVD_P_HOSTINTERFACEMEMORY_COOK_OFFSETS( _stOffsets_, _ulByteCount_ ) \
{                                                                             \
   /* Size of the data element. */                                            \
   _stOffsets_.ulBytesPerValue = _ulByteCount_;                               \
                                                                              \
   /* Create a mask based on the size of the data element. */                 \
   switch( _ulByteCount_ )                                                    \
   {                                                                          \
      case 1:  _stOffsets_.ulByteMask = 0xFF ;        break;                  \
      case 2:  _stOffsets_.ulByteMask = 0xFFFF ;      break;                  \
      case 3:  _stOffsets_.ulByteMask = 0xFFFFFF ;    break;                  \
      case 4:  _stOffsets_.ulByteMask = 0xFFFFFFFF ;  break;                  \
      default:  _stOffsets_.ulByteMask = 0xFF ;       break;                  \
   }                                                                          \
                                                                              \
   /* Divide by 4 to turn the HIM byte offset into a word offset. */          \
   _stOffsets_.ulWordOffset = ( _stOffsets_.ulByteOffset >> 2 );              \
                                                                              \
   /* Extract the two low order bits and multiply                             \
    * by 8 to get the byte shift */                                                       \
   _stOffsets_.ulByteShift = (( _stOffsets_.ulByteOffset & 0x3 ) *  8 );                  \
                                                                                          \
   /* Create the inverse of the mask for read-modify-write operations */                  \
   _stOffsets_.ulInverseMask = ~( _stOffsets_.ulByteMask << _stOffsets_.ulByteShift  );   \
}

#define BXVD_P_READ_DISPLAY_INFO(hXvdDipCh, stDisplayInfo)                              \
   BXVD_P_ReadDisplayInfo_HIM_API_isr(hXvdDipCh->stChannelSettings.hXvd,                \
                                      hXvdDipCh->stChannelSettings.uiDisplayInfoOffset, \
                                      &stDisplayInfo);

#define BXVD_P_READ_HIM(hXvd, wordOffset, value)                                  \
   BXVD_Reg_Write32_isr(hXvd,                                                     \
                    hXvd->stPlatformInfo.stReg.uiDecode_HST_SCR_RAM_ADDR,         \
                    wordOffset);                                                  \
   value = BXVD_Reg_Read32_isr(hXvd,                                              \
                           hXvd->stPlatformInfo.stReg.uiDecode_HST_SCR_RAM_DATA);


#define BXVD_P_WRITE_HIM(hXvd, wordOffset, value)                         \
   BXVD_Reg_Write32_isr(hXvd,                                             \
                    hXvd->stPlatformInfo.stReg.uiDecode_HST_SCR_RAM_ADDR, \
                    wordOffset);                                          \
   BXVD_Reg_Write32_isr(hXvd,                                             \
                    hXvd->stPlatformInfo.stReg.uiDecode_HST_SCR_RAM_DATA, \
                    value);

#define BXVD_P_SAVE_DIP_INFO_STC(hXvdDispCh, stDisplayInfo)                                \
   for (i=0; i< BXVD_P_STC_MAX; i++)                                                       \
   {                                                                                       \
      hXvdDipCh->stDisplayInterruptInfo.astSTC[i].uiValue = stDisplayInfo.stc_snapshot[i]; \
      hXvdDipCh->stDisplayInterruptInfo.astSTC[i].bValid = true;                           \
   }

#define BXVD_P_DBG_MSG_DISP_INFO_OFFSET(hXvd, pInitRsp)                                    \
   BXVD_DBG_MSG(hXvd, (" display_info_0_offset: %#x", pInitRsp->display_info_0_offset));   \
   BXVD_DBG_MSG(hXvd, (" display_info_1_offset: %#x", pInitRsp->display_info_1_offset));

#define BXVD_P_SAVE_DISPLAY_INFO(hXvd, pInitRsp)                     \
   hXvd->uiDisplayInfo0_Offset = pInitRsp->display_info_0_offset/4;  \
   hXvd->uiDisplayInfo1_Offset = pInitRsp->display_info_1_offset/4;


#define BXVD_P_SAVE_XVD_CHANNEL_DISPLAY_INFO_0(hXvdCh, hXvd)  hXvdCh=hXvdCh


#define BXVD_P_SAVE_XVD_CHANNEL_DISPLAY_INFO_1(hXvdCh, hXvd)  hXvdCh=hXvdCh


#define BXVD_P_SAVE_DIP_CHANNEL_DISPLAY_INFO_0(stXvdDipChSettings, hXvd)  \
   stXvdDipChSettings.uiDisplayInfoOffset = hXvd->uiDisplayInfo0_Offset;


#define BXVD_P_SAVE_DIP_CHANNEL_DISPLAY_INFO_1(stXvdDipChSettings, hXvd)  \
   stXvdDipChSettings.uiDisplayInfoOffset = hXvd->uiDisplayInfo1_Offset;

/* Core Firmware */
typedef struct {
	BXVD_IMAGE_FirmwareID outerELFFirmwareID;
	BXVD_IMAGE_FirmwareID innerELFFirmwareID;
	BXVD_IMAGE_FirmwareID authenticatedFirmwareID;
} BXVD_P_CoreFirmware_RevK0;

/* Platform Register Maps */
typedef struct {
      uint32_t uiSun_SWInitSet;
      uint32_t uiSun_SWInitSetAvdMask;
      uint32_t uiSun_SWInitClear;
      uint32_t uiSun_SWInitClearAvdMask;

      uint32_t uiAvd_SoftShutdownCtrl;
      uint32_t uiAvd_SoftShutdownCtrlMask;

      uint32_t uiDecode_IPShim_AvdClkGate;
      uint32_t uiDecode_IPShim_OtpCtl;
      uint32_t uiDecode_IPShim_OtpCtl_DisableRV9;
      uint32_t uiDecode_CPUId;
      uint32_t uiDecode_CPUId_BldIdMask;

      uint32_t uiAvd_CPUL2InterruptSet;

      uint32_t uiBvnf_Intr2_3_AvdStatus;
      uint32_t uiBvnf_Intr2_3_AvdClear;
      uint32_t uiBvnf_Intr2_3_AvdMaskClear;

      uint32_t uiInterrupt_PicDataRdy;
      uint32_t uiInterrupt_PicDataRdy1;
      uint32_t uiInterrupt_Mailbox;
      uint32_t uiInterrupt_StereoSeqError;
      uint32_t uiInterrupt_StillPictureRdy;
      uint32_t uiInterrupt_OuterWatchdog;
      uint32_t uiInterrupt_VICReg;
      uint32_t uiInterrupt_VICSCBWr;
      uint32_t uiInterrupt_VICInstrRd;
      uint32_t uiInterrupt_VICILInstrRd;
      uint32_t uiInterrupt_VICBLInstrRd;

      uint32_t uiDecode_InnerInstructionBase;
      uint32_t uiDecode_InnerEndOfCode;
      uint32_t uiDecode_InnerGlobalIOBase;
      uint32_t uiDecode_InnerCPUDebug;
      uint32_t uiDecode_InnerCPUAux;

      uint32_t uiDecode_OuterInstructionBase;
      uint32_t uiDecode_OuterEndOfCode;
      uint32_t uiDecode_OuterGlobalIOBase;
      uint32_t uiDecode_OuterCPUDebug;
      uint32_t uiDecode_OuterCPUAux;
      uint32_t uiDecode_OuterInterruptMask;
      uint32_t uiDecode_OuterInterruptClear;
      uint32_t uiDecode_OuterWatchdogTimer;
      uint32_t uiDecode_OuterCPU2HostMailbox;
      uint32_t uiDecode_OuterHost2CPUMailbox;
      uint32_t uiDecode_OuterCPU2HostStatus;

      uint32_t uiDecode_BaseInstructionBase;
      uint32_t uiDecode_BaseEndOfCode;
      uint32_t uiDecode_BaseGlobalIOBase;
      uint32_t uiDecode_BaseCPUDebug;
      uint32_t uiDecode_BaseCPUAux;

      uint32_t uiDecode_CabacBinDepth;

      uint32_t uiDecode_CabacBinCtl;
      uint32_t uiDecode_CabacBinCtl_ResetMask;
      uint32_t uiDecode_SintStrmSts;
      uint32_t uiDecode_SintStrmSts_ResetMask;
      uint32_t uiDecode_OLSintStrmSts;
      uint32_t uiDecode_OLSintStrmSts_ResetMask;
      uint32_t uiDecode_MainCtl;
      uint32_t uiDecode_MainCtl_ResetMask;
      uint32_t uiDecode_SWReset;
      uint32_t uiDecode_SWReset_ILSIResetMask;

      uint32_t uiDecode_BLD_SintStrmSts;
      uint32_t uiDecode_BLD_SintStrmSts_ResetMask;
      uint32_t uiDecode_BLD_ILSScale;
      uint32_t uiDecode_BLD_ILSScale_ResetMask;
      uint32_t uiDecode_BLD_MainCtl;
      uint32_t uiDecode_BLD_MainCtl_ResetMask;
      uint32_t uiDecode_BLD_SWReset;
      uint32_t uiDecode_BLD_SWReset_ILSIResetMask;

      uint32_t uiSunGisbArb_ReqMask;
      uint32_t uiSunGisbArb_ReqMaskAVDMask;

      uint32_t uiDecode_StripeWidth;
      uint32_t uiDecode_StripeWidthShift;
      uint32_t uiDecode_PFRIBankHeightShift;

      uint32_t uiDecode_PFRIData;
      uint32_t uiDecode_PFRIDataDDR3ModeShift;
      uint32_t uiDecode_PFRIDataBusWidthShift;
      uint32_t uiDecode_PFRIDataSourceShift;
      uint32_t uiDecode_PFRIDataSourceMask;
      uint32_t uiDecode_PFRIData2SourceShift;

      uint32_t uiDecode_BLStripeWidth;
      uint32_t uiDecode_BLPFRIData;

      uint32_t uiAVD_PcacheMode;
      uint32_t uiAVD_PcacheModeYGranMask;
      uint32_t uiAVD_PcacheModeYGranShift;
      uint32_t uiAVD_PcacheModeXGranMask;
      uint32_t uiAVD_PcacheModeXGranShift;

      uint32_t uiDecode_RVCCtl;
      uint32_t uiDecode_RVCPut;
      uint32_t uiDecode_RVCGet;
      uint32_t uiDecode_RVCBase;
      uint32_t uiDecode_RVCEnd;

      uint32_t uiDecode_HST_SCR_RAM_ADDR;
      uint32_t uiDecode_HST_SCR_RAM_DATA;
} BXVD_P_PlatformReg_RevK0;

typedef struct {
	BXVD_P_CoreFirmware_RevK0 stFirmware;
	BXVD_P_PlatformReg_RevK0 stReg;
} BXVD_P_PlatformInfo_RevK0;

#define BXVD_P_CONTEXT_PLATFORM BXVD_P_PlatformInfo_RevK0 stPlatformInfo;


void BXVD_P_ReadDisplayInfo_HIM_API_isr(BXVD_Handle hXvd,
                                        uint32_t uiDisplayInfoOffset,
                                        BXVD_P_DisplayInfo *pstDisplayInfo);

bool BXVD_P_IsDisplayInfoEqual_HIM_API_isr(BXVD_P_DisplayInfo stDisplayInfo,
                                           BXVD_P_DisplayInfo stDisplayInfo1);

#ifdef __cplusplus
}
#endif

#endif /* _BXVD_CORE_AVD_REVK0_H_ */

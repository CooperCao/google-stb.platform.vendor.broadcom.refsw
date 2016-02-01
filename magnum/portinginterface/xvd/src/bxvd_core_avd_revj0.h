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
#ifndef _BXVD_CORE_AVD_REVJ0_H_
#define _BXVD_CORE_AVD_REVJ0_H_

#include "bxvd_image.h"
#include "bxvd_core_avd_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BXVD_P_READ_DISPLAY_INFO(hXvdDipCh, stDisplayInfo)                                   \
   stDisplayInfo.vsync_parity = hXvdDipCh->stChannelSettings.pstDisplayInfo->vsync_parity;   \
   stDisplayInfo.stc_snapshot = hXvdDipCh->stChannelSettings.pstDisplayInfo->stc_snapshot;   \
   stDisplayInfo.stc1_snapshot = hXvdDipCh->stChannelSettings.pstDisplayInfo->stc1_snapshot; \


#define BXVD_P_IS_DISPLAY_INFO_EQUAL(stDisplayInfo, stDisplayInfo1)   \
   (( stDisplayInfo.vsync_parity == stDisplayInfo1.vsync_parity ) &&  \
    ( stDisplayInfo.stc_snapshot == stDisplayInfo1.stc_snapshot ) &&  \
    ( stDisplayInfo.stc1_snapshot == stDisplayInfo1.stc1_snapshot ))


#define BXVD_P_SAVE_DIP_INFO_STC(hXvdDispCh, stDisplayInfo)                              \
      hXvdDipCh->stDisplayInterruptInfo.astSTC[0].uiValue = stDisplayInfo.stc_snapshot;  \
      hXvdDipCh->stDisplayInterruptInfo.astSTC[0].bValid = true;                         \
      hXvdDipCh->stDisplayInterruptInfo.astSTC[1].uiValue = stDisplayInfo.stc1_snapshot; \
      hXvdDipCh->stDisplayInterruptInfo.astSTC[1].bValid = true;


#define BXVD_P_DBG_MSG_DISP_INFO_OFFSET(hXvd, pInitRsp)                                     \
      BXVD_DBG_MSG(hXvd, (" dms_delivery_address0: %#x", pInitRsp->dms_delivery_address0)); \
      BXVD_DBG_MSG(hXvd, (" dms_delivery_address1: %#x", pInitRsp->dms_delivery_address1));


#define BXVD_P_SAVE_DISPLAY_INFO(hXvd, pInitRsp)                              \
      hXvd->pDisplayInfo0 = (void *)(hXvd->uiFWMemBaseVirtAddr + ((uint32_t)pInitRsp->dms_delivery_address0  - hXvd->uiFWMemBasePhyAddr)); \
      hXvd->pDisplayInfo1 = (void *)(hXvd->uiFWMemBaseVirtAddr + ((uint32_t)pInitRsp->dms_delivery_address1  - hXvd->uiFWMemBasePhyAddr));
/*     BMEM_ConvertOffsetToAddress(hXvd->hCodeHeap,                    \
 *                                  (uint32_t)pInitRsp->dms_delivery_address0, \
 *                                 (void *)&hXvd->pDisplayInfo0);       \
 *     BMEM_ConvertOffsetToAddress(hXvd->hCodeHeap,                     \
 *                                 (uint32_t)pInitRsp->dms_delivery_address1, \
 *                                 (void *)&hXvd->pDisplayInfo1);
 *
 */


#define BXVD_P_SAVE_XVD_CHANNEL_DISPLAY_INFO_0(hXvdCh, hXvd)                         \
      hXvdCh->stChBufferConfig.AVD_DMS2PI_Buffer.pDisplayInfo = hXvd->pDisplayInfo0;


#define BXVD_P_SAVE_XVD_CHANNEL_DISPLAY_INFO_1(hXvdCh, hXvd)                         \
      hXvdCh->stChBufferConfig.AVD_DMS2PI_Buffer.pDisplayInfo = hXvd->pDisplayInfo1;


#define BXVD_P_SAVE_DIP_CHANNEL_DISPLAY_INFO_0(stXvdDipChSettings, hXvd)  \
       stXvdDipChSettings.pstDisplayInfo = pXvd->pDisplayInfo0; 


#define BXVD_P_SAVE_DIP_CHANNEL_DISPLAY_INFO_1(stXvdDipChSettings, hXvd)  \
       stXvdDipChSettings.pstDisplayInfo = pXvd->pDisplayInfo1; 


/* Core Firmware */
typedef struct {
	BXVD_IMAGE_FirmwareID outerELFFirmwareID;
	BXVD_IMAGE_FirmwareID innerELFFirmwareID;
	BXVD_IMAGE_FirmwareID authenticatedFirmwareID;
} BXVD_P_CoreFirmware_RevJ0;

/* Platform Register Maps */
typedef struct {
      uint32_t uiSun_SWReset;
      uint32_t uiSun_SWResetAVDMask;
      
      uint32_t uiAvd_CPUL2InterruptSet;
      
      uint32_t uiBvnf_Intr2_3_AvdStatus;
      uint32_t uiBvnf_Intr2_3_AvdClear;
      uint32_t uiBvnf_Intr2_3_AvdMaskClear;
      
      uint32_t uiXPT_PCROffset_STC;
      
      uint32_t uiInterrupt_PicDataRdy;
      uint32_t uiInterrupt_PicDataRdy1;
      uint32_t uiInterrupt_Mailbox;
      uint32_t uiInterrupt_StereoSeqError;
      uint32_t uiInterrupt_StillPictureRdy;
      uint32_t uiInterrupt_OuterWatchdog;
      uint32_t uiInterrupt_VICReg;
      uint32_t uiInterrupt_VICSCBWr;
      uint32_t uiInterrupt_VICInstrRd;
      
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

      uint32_t uiDecode_CabacBinDepth;

      uint32_t uiDecode_CabacBinCtl;
      uint32_t uiDecode_CabacBinCtl_ResetMask;
      uint32_t uiDecode_SintStrmSts;
      uint32_t uiDecode_SintStrmSts_ResetMask;
      uint32_t uiDecode_OLSintStrmSts;
      uint32_t uiDecode_OLSintStrmSts_ResetMask;
      uint32_t uiDecode_MainCtl;
      uint32_t uiDecode_MainCtl_ResetMask;

      uint32_t uiSunGisbArb_ReqMask;
      uint32_t uiSunGisbArb_ReqMaskAVDMask;

      uint32_t uiDecode_StripeWidth;
      uint32_t uiDecode_StripeWidthMask;
      uint32_t uiDecode_StripeWidthShift;
      uint32_t uiDecode_PFRIBankHeightMask;
      uint32_t uiDecode_PFRIBankHeightShift;

      uint32_t uiDecode_PFRIData;
      uint32_t uiDecode_PFRIDataDDR3ModeMask;
      uint32_t uiDecode_PFRIDataDDR3ModeShift;
      uint32_t uiDecode_PFRIDataBusWidthMask;
      uint32_t uiDecode_PFRIDataBusWidthShift;
      uint32_t uiDecode_PFRIDataSourceMask;
      uint32_t uiDecode_PFRIDataSourceShift;

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

      uint32_t uiVCXO_AVDCtrl;
      uint32_t uiVCXO_AVDCtrl_PwrDnMask;
      uint32_t uiClkGen_CoreClkCtrl;
      uint32_t uiClkGen_CoreClkCtrl_PwrDnMask;
      uint32_t uiClkGen_SCBClkCtrl;
      uint32_t uiClkGen_SCBClkCtrl_PwrDnMask;
      uint32_t uiClkGen_GISBClkCtrl;
      uint32_t uiClkGen_GISBClkCtrl_PwrDnMask;
} BXVD_P_PlatformReg_RevJ0;

typedef struct {
	BXVD_P_CoreFirmware_RevJ0 stFirmware;
	BXVD_P_PlatformReg_RevJ0 stReg;
} BXVD_P_PlatformInfo_RevJ0;

#define BXVD_P_CONTEXT_PLATFORM BXVD_P_PlatformInfo_RevJ0 stPlatformInfo;

/*
 * Macros for managing the delivery queue.
 */
#define BXVD_AVD_P_DELIVERY_QUEUE_GET_READ_OFFSET( _hXvdCh_, _uiReadOffset_ )                               \
{                                                                                                           \
   _uiReadOffset_ = _hXvdCh_->stChBufferConfig.AVD_DMS2PI_Buffer.pPictureDeliveryQueue->queue_read_offset;  \
}

#define BXVD_AVD_P_DELIVERY_QUEUE_SET_READ_OFFSET( _hXvdCh_, _uiReadOffset_ )                               \
{                                                                                                           \
   _hXvdCh_->stChBufferConfig.AVD_DMS2PI_Buffer.pPictureDeliveryQueue->queue_read_offset = _uiReadOffset_;  \
}

#define BXVD_AVD_P_DELIVERY_QUEUE_GET_WRITE_OFFSET( _hXvdCh_, _uiWriteOffset_ )                                \
{                                                                                                              \
   _uiWriteOffset_ = _hXvdCh_->stChBufferConfig.AVD_DMS2PI_Buffer.pPictureDeliveryQueue->queue_write_offset;   \
}

#define BXVD_AVD_P_DELIVERY_QUEUE_GET_ADDR( _hXvdCh_, _pDeliveryQue_ )                    \
{                                                                                         \
    _pDeliveryQue_ = _hXvdCh_->stChBufferConfig.AVD_DMS2PI_Buffer.pPictureDeliveryQueue;  \
}

/*
 * Macros for managing the release queue.
 */
#define BXVD_AVD_P_RELEASE_QUEUE_GET_READ_OFFSET( _hXvdCh_, _uiReadOffset_ )                                \
{                                                                                                           \
   _uiReadOffset_ = _hXvdCh_->stChBufferConfig.AVD_DMS2PI_Buffer.pPictureReleaseQueue->queue_read_offset;   \
}

#define BXVD_AVD_P_RELEASE_QUEUE_GET_WRITE_OFFSET( _hXvdCh_, _uiWriteOffset_ )                                 \
{                                                                                                              \
   _uiWriteOffset_ = _hXvdCh_->stChBufferConfig.AVD_DMS2PI_Buffer.pPictureReleaseQueue->queue_write_offset;    \
}

#define BXVD_AVD_P_RELEASE_QUEUE_SET_WRITE_OFFSET( _hXvdCh_, _uiWriteOffset_ )                                 \
{                                                                                                              \
   _hXvdCh_->stChBufferConfig.AVD_DMS2PI_Buffer.pPictureReleaseQueue->queue_write_offset = _uiWriteOffset_;    \
}

#define BXVD_AVD_P_RELEASE_QUEUE_GET_SHADOW_WRITE_OFFSET( _hXvdCh_, _uiWriteOffset_ )  \
{                                                                                      \
   _uiWriteOffset_ = _hXvdCh_->stChBufferConfig.pAVD_PI2DMS_Buffer->pdq_write_offset;  \
}

#define BXVD_AVD_P_RELEASE_QUEUE_SET_SHADOW_WRITE_OFFSET( _hXvdCh_, _uiWriteOffset_ )  \
{                                                                                      \
   _hXvdCh_->stChBufferConfig.pAVD_PI2DMS_Buffer->pdq_write_offset = _uiWriteOffset_;  \
}

#define BXVD_AVD_P_RELEASE_QUEUE_GET_ADDR( _hXvdCh_, _pReleaseQue_ )                   \
{                                                                                      \
   _pReleaseQue_= _hXvdCh_->stChBufferConfig.AVD_DMS2PI_Buffer.pPictureReleaseQueue;   \
}


/*
 * Macros for accessing the drop count.
 */
#define BXVD_AVD_P_SET_DROP_COUNT( _hXvdCh_, _count_ )                     \
{                                                                          \
   _hXvdCh_->stChBufferConfig.pAVD_PI2DMS_Buffer->drop_count = _count_;    \
}

#define BXVD_AVD_P_GET_DROP_COUNT( _hXvdCh_, _count_ )                     \
{                                                                          \
   _count_ = _hXvdCh_->stChBufferConfig.pAVD_PI2DMS_Buffer->drop_count;    \
}

/*
 * Macro for retrieving the AVD status block.
 */
#define BXVD_AVD_P_GET_VIDEO_DECODER_STATUS( _hXvdCh_, _ulStatus_ )     \
{                                                                       \
   if ( _hXvdCh_->ulAvdStatusBlock != 0 )                               \
   {                                                                    \
      _ulStatus_ = *(uint32_t *)( _hXvdCh_->ulAvdStatusBlock );         \
   }                                                                    \
   else                                                                 \
   {                                                                    \
      _ulStatus_ = 0;                                                   \
   }                                                                    \
}

#ifdef __cplusplus
}
#endif

#endif /* _BXVD_CORE_AVD_REVJ0_H_ */

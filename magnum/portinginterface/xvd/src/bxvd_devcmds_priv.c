/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *   See Module Overview below
 *
 ***************************************************************************/
#include "bstd.h"
#include "bxvd_platform.h"
#include "bxvd_priv.h"
#include "bkni.h"                /* malloc */
#include "bavc.h"

#include "bxvd_priv.h"
#include "bxvd_reg.h"

#define CPU_2_HOST_INT_CLEAR 0x80000000

/* If FW cmd MBX handshake polling desired, enable polling here */
/* #define BXVD_POLL_FW_MBX 1 */

#ifdef EMULATION
#define FW_CMD_TIMEOUT_NORMAL   1000000

#else
#define FW_CMD_TIMEOUT_NORMAL    1000
#endif

#define FW_CMD_TIMEOUT_DEBUGCMD  1500
#define B0_POLL_DELAY 6000

#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
#define BXVD_P_SWAP32(a) (((a&0xFF)<<24)|((a&0xFF00)<<8)|((a&0xFF0000)>>8)|((a&0xFF000000)>>24))

static int _strlen(const char *s)
{
   int i=0;
   while (*s++) i++;
   return i;
}
#endif

BDBG_MODULE(BXVD_CMDS);

/* TODO: Find the proper values for items tagged with this define*/
#define TBD_VALUE (0)

/******************************************************************************
Summary:
  This function is used to send commands to the AVC decoder.

Description:
  General function for sending commands to AVC decoder.  Used by
  functions that handle specific commands.

Input:
  hXvd       - XVD instance handle
  uInstance  - Decoder instance. Always 0 for 7401, 7118 0 or 1 for 7400
  *pCmd      - pointer to command structure to send to AVC
  *pRsp      - pointer to buffer to receive response.

Returns:
  BERR_SUCCESS
  BERR_INVALID_PARAMETER - decoder related failure

See Also:

******************************************************************************/

BERR_Code BXVD_P_SendDecoderCommand(
   BXVD_Handle hXvd,
   uint32_t       uInstance,
   const BXVD_Cmd *pCmd,
   BXVD_Rsp       *pRsp,
   uint32_t       uiTimeout
   )
{
   BERR_Code         eStatus = BERR_SUCCESS;
   BXVD_P_RspGeneric *pGenericRsp;       /* Used to extract the status field */

   volatile uint32_t uInBox;

#if BXVD_POLL_FW_MBX
   int i;
   volatile uint32_t uIntrVal;
#endif

   BDBG_ENTER(BXVD_P_SendDecoderCommand);

   BDBG_ASSERT(pCmd);
   BDBG_ASSERT(pRsp);

   uInstance = uInstance;

   BKNI_EnterCriticalSection();

   /* Check to if we're already waiting for a reply from this decoder */
   if (hXvd->stDecoderContext.bIfBusy)
   {
      BKNI_LeaveCriticalSection();
      BXVD_DBG_MSG(hXvd, ("Decoder is busy... exiting"));
      return BERR_TRACE(BXVD_ERR_FW_IS_BUSY);
   }

   /* Set device busy */
   hXvd->stDecoderContext.bIfBusy = 1;
   BKNI_LeaveCriticalSection();

   /* Clear out any previously completed FW command done events */
   BKNI_ResetEvent(hXvd->stDecoderContext.hFWCmdDoneEvent);

   /* write the command parameters to the AVC  */
   BKNI_Memcpy((void*)(hXvd->stDecoderContext.ulCmdBufferAddr),
               (void*)(pCmd),
               sizeof(BXVD_Cmd));

   BMMA_FlushCache(hXvd->hFWMemBlock, (void*)(hXvd->stDecoderContext.ulCmdBufferAddr), sizeof(BXVD_Cmd));

#if BXVD_POLL_FW_MBX
   /* zero out the response mailbox*/
   BXVD_P_WRITE_FWRSP_MBX(hXvd, uInstance, 0);
#endif

   BXVD_P_WRITE_FWCMD_TO_MBX(hXvd, uInstance, hXvd->uiCmdBufferVector);

#if BXVD_POLL_FW_MBX

   BDBG_ERR(("BXVD_CMDSEND:Polling\n"));
   /* poll for response */
   uInBox = 0;
   for ( i = 0; i < 200; i++)
   {
      BKNI_Sleep(100);

      uInBox = BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_OuterCPU2HostMailbox);

      BXVD_DBG_MSG(hXvd, ("Poll Loop %d Decoder:%d, mbx:%08x", i, uInstance, uInBox));
      if (uInBox !=0)
         break;
   }

   BXVD_DBG_MSG(hXvd, ("Decoder:%d, mbx:%08x", uInstance, uInBox));

   uIntrVal = BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_OuterInterruptClear);

   BXVD_DBG_MSG(hXvd, ("IntGen_clr:%08x\n", uIntrVal));

   /* See if the firmware timed out before the response */
   if ( i >= 200) /* Was 200 */
   {
      BXVD_DBG_ERR(hXvd, ("BXVD_P_SendDecoderCommand timed out waiting for response"));
   }

#else

   /* Interrupt driven implementation */

   /* wait for response from FW */
   uInBox = 0;

#if BXVD_P_FW_CMD_LOST_WORKAROUND
   {
      bool done = false;

      while (!done)
      {
         int loopCnt=0;

         eStatus = BERR_TRACE(BKNI_WaitForEvent(hXvd->stDecoderContext.hFWCmdDoneEvent, uiTimeout));

         if(BERR_TIMEOUT == eStatus)
         {
            BXVD_DBG_ERR(hXvd, ("\tFW command: %08x response timed out, resending command", pCmd->cmd));

            BXVD_P_WRITE_FWCMD_TO_MBX(hXvd, uInstance, hXvd->uiCmdBufferVector);

            loopCnt++;
         }
         else
         {
            done = true;
         }

         if (loopCnt >=5)
         {
            done = true;
         }
      }
   }
#else
   eStatus = BERR_TRACE(BKNI_WaitForEvent(hXvd->stDecoderContext.hFWCmdDoneEvent, uiTimeout));
#endif

   if(BERR_TIMEOUT == eStatus)
   {
      BXVD_DBG_ERR(hXvd, ("FW command: %08x response timed out", pCmd->cmd));
      hXvd->stDecoderContext.bIfBusy = 0;
      return BERR_TRACE(eStatus);
   }

#ifdef EMULATION
   BKNI_Sleep(2000);
#endif

   /* get the response from the AVC  */

   uInBox = BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_OuterCPU2HostMailbox);

#endif


   /* Clear device busy */
   hXvd->stDecoderContext.bIfBusy = 0;

   /* get the response from the AVC  */
   if (uInBox)
   {
      BMMA_FlushCache(hXvd->hFWMemBlock, (void*)(hXvd->stDecoderContext.ulCmdBufferAddr), sizeof(BXVD_Rsp));

      BKNI_Memcpy((void*)(pRsp),
        (void*)(hXvd->stDecoderContext.ulCmdBufferAddr),
        sizeof(BXVD_Rsp));


     /* make sure it is the correct command */
      if (((pCmd->cmd) | BXVD_CMD_RESPONSE) != (pRsp->cmd))
      {
         BDBG_LEAVE(BXVD_P_SendDecoderCommand);
         return BERR_TRACE(BERR_INVALID_PARAMETER);
      }

      pGenericRsp = (BXVD_P_RspGeneric *)(pRsp);

      /* Check for command error */
      if ( pGenericRsp->ulStatus )
      {
         BDBG_ERR(("AVD firmware command: %08x error return %d", pCmd->cmd, pGenericRsp->ulStatus ));
         BDBG_LEAVE(BXVD_P_SendDecoderCommand);
         return BERR_TRACE(BERR_INVALID_PARAMETER);
      }

   }
   else
   {
      BXVD_DBG_ERR(hXvd, ("FW RSP MBX PTR NULL"));
      return BERR_TRACE(BERR_UNKNOWN);
   }

   BKNI_ResetEvent(hXvd->stDecoderContext.hFWCmdDoneEvent);

   BDBG_LEAVE(BXVD_P_SendDecoderCommand);
   return BERR_TRACE(eStatus);
}

/***************************************************************************
 *  {secret}
 *  BXVD_P_HostCmdSendInit
 *  Command causes the AVC decoder to initialize.
 *  All internal state and any partially completed operations
 *  will be lost.
 ***************************************************************************/
BERR_Code BXVD_P_HostCmdSendInit
(
 BXVD_Handle  hXvd,                /* XVD context */
 uint32_t     uDecoderInstance,    /* AVC instance */
 uint32_t     eRaveEndianess       /* Endianess of data in Rave CDB/ITB */
 )
{
   BXVD_Cmd             *pCmd;
   BXVD_Rsp             *pRsp;
   BERR_Code            eStatus = BERR_SUCCESS; /* Status return */

   BXVD_Cmd_Initialize *pInit;             /* Pointer to command */
   BXVD_Rsp_Initialize *pInitRsp;              /* Pointer to response */

   /* Clear out the context and set defaults. */
   pCmd = &hXvd->FWCmd.cmd;
   pRsp = &hXvd->FWRsp.rsp;

   BKNI_Memset((void*)pCmd, BXVD_P_MEM_ZERO, sizeof(BXVD_Cmd));
   BKNI_Memset((void*)pRsp, BXVD_P_MEM_ZERO, sizeof(BXVD_Rsp));

   pInit = &hXvd->FWCmd.init;
   pInitRsp = &hXvd->FWRsp.init;

   /* Command ID */
   pInit->command = BXVD_CMD_INITIALIZE;

   /* parameters */
   pInit->cdb_little_endian = eRaveEndianess;
   pInit->stripe_width = hXvd->uiDecode_StripeWidth;
   pInit->stripe_height = hXvd->uiDecode_StripeMultiple;
   pInit->bvnf_intr_context_base = hXvd->stPlatformInfo.stReg.uiBvnf_Intr2_3_AvdStatus;
#if !BXVD_P_FW_40BIT_API
   pInit->host_L2_intr_set = hXvd->stPlatformInfo.stReg.uiAvd_CPUL2InterruptSet;
#else
   pInit->map8_mode_enabled = (hXvd->scbMapVer == BCHP_ScbMapVer_eMap8);
#endif
   pInit->chip_prod_revision = hXvd->uiChip_ProductRevision;
   pInit->rave_context_reg_size = BXVD_P_RAVE_CONTEXT_SIZE;
   pInit->rave_cx_hold_clr_status = BXVD_P_RAVE_CX_HOLD_CLR_STATUS;
   pInit->rave_packet_count = BXVD_P_RAVE_PACKET_COUNT;

#if BXVD_P_RUL_DONE_MASK_64_BITS
   pInit->bvnf_intr_context_1_base = hXvd->stPlatformInfo.stReg.uiBvnf_Intr2_11_AvdStatus;
#endif

#if BXVD_P_MEMC_SENTINEL_0_REG_START
   pInit->memc_sentinel_reg_start = BXVD_P_MEMC_SENTINEL_0_REG_START;
#else
   pInit->memc_sentinel_reg_start = VDEC_INIT_SENTINEL_NOT_SUPPORTED;
#endif

   BXVD_DBG_MSG(hXvd, ("BXVD_CMD_INITIALIZE:"));
   BXVD_DBG_MSG(hXvd, (" command: %#x", pInit->command));
   BXVD_DBG_MSG(hXvd, (" cdb_little_endian: %d", pInit->cdb_little_endian));
   BXVD_DBG_MSG(hXvd, (" stripe_width: %d", pInit->stripe_width));
   BXVD_DBG_MSG(hXvd, (" stripe_height: %d", pInit->stripe_height));
   BXVD_DBG_MSG(hXvd, (" bvnf_intr_context base: %08x", pInit->bvnf_intr_context_base));
#if !BXVD_P_FW_40BIT_API
   BXVD_DBG_MSG(hXvd, (" host_L2_intr_set: %08x", pInit->host_L2_intr_set));
#endif
   BXVD_DBG_MSG(hXvd, (" chip_prod_revision: %08x", pInit->chip_prod_revision));
   BXVD_DBG_MSG(hXvd, (" rave_context_reg_size: %08x", pInit->rave_context_reg_size));
   BXVD_DBG_MSG(hXvd, (" rave_cx_hold_clr_status: %08x", pInit->rave_cx_hold_clr_status));
   BXVD_DBG_MSG(hXvd, (" rave_packet_count: %08x", pInit->rave_packet_count));
   BXVD_DBG_MSG(hXvd, (" bvnf_intr_context_1 base: %08x", pInit->bvnf_intr_context_1_base));
   BXVD_DBG_MSG(hXvd, (" memc_sentinel_reg_start: %08x", pInit->memc_sentinel_reg_start));
#if BXVD_P_FW_40BIT_API
   BXVD_DBG_MSG(hXvd, (" map8_mode_enabled: %08x", pInit->map8_mode_enabled));
#endif

   eStatus = BERR_TRACE(BXVD_P_SendDecoderCommand(hXvd,
                                                  uDecoderInstance,
                                                  pCmd,
                                                  pRsp,
                                                  FW_CMD_TIMEOUT_NORMAL));

   BXVD_DBG_MSG(hXvd, ("BXVD_RSP_INITIALIZE:"));
   BXVD_DBG_MSG(hXvd, (" command: %#x", pInitRsp->command));
   BXVD_DBG_MSG(hXvd, (" status: %#x", pInitRsp->status));
   BXVD_DBG_MSG(hXvd, (" sw_version: %#x", pInitRsp->sw_version));

   BXVD_P_DBG_MSG_DISP_INFO_OFFSET(hXvd, pInitRsp);

   if( eStatus != BERR_SUCCESS )
   {
      return BERR_TRACE(eStatus);
   }
   else if (!BXVD_P_IS_FW_VERSION_VALID(pInitRsp))
   {
      BXVD_DBG_WRN(hXvd, ("Incompatible FW version"));

      eStatus = BERR_INVALID_PARAMETER;
   }

   /*
    * Save the parameters passed back from the firmware.
    */

   BXVD_P_SAVE_DISPLAY_INFO(hXvd, pInitRsp);

   hXvd->sRevisionInfo.ulDecoderFwRev = pInitRsp->sw_version;

   return BERR_TRACE(eStatus);
}


/***************************************************************************
 *  {secret}
 *  BXVD_P_HostCmdSendConfig
 *
 *  The BVNF RUL mask bits map to display device.
 ***************************************************************************/
BERR_Code BXVD_P_HostCmdSendConfig
(
 BXVD_Handle  hXvd,               /* XVD context */
 uint32_t     uiVecIndex,         /* Display device index */
 uint32_t     uiInterruptMask_0,  /* RUL done mask for specified display */
 uint32_t     uiInterruptMask_1   /* RUL done mask for specified display */
)
{
   BXVD_Cmd             *pCmd;
   BXVD_Rsp             *pRsp;

   BERR_Code            eStatus = BERR_SUCCESS; /* Status return */

   BXVD_Cmd_Config *pCfgCmd;   /* Pointer to command */
#if BDBG_DEBUG_BUILD
   BXVD_Rsp_Config *pCfgRsp;   /* Pointer to response */

   pCfgRsp = &hXvd->FWRsp.config;
#endif

   /* Clear out the context and set defaults. */
   pCmd = &hXvd->FWCmd.cmd;
   pRsp = &hXvd->FWRsp.rsp;

   BKNI_Memset((void*)pCmd, BXVD_P_MEM_ZERO, sizeof(BXVD_Cmd));
   BKNI_Memset((void*)pRsp, BXVD_P_MEM_ZERO, sizeof(BXVD_Rsp));

   pCfgCmd = &hXvd->FWCmd.config;

   /* Command ID */
   pCfgCmd->command = BXVD_CMD_CONFIG;

   /* parameters */
   pCfgCmd->vec_index = uiVecIndex;
   pCfgCmd->interrupt_mask_0 = uiInterruptMask_0;
   pCfgCmd->interrupt_mask_1 = uiInterruptMask_1;

   BXVD_DBG_MSG(hXvd, ("BXVD_CMD_CONFIG:"));
   BXVD_DBG_MSG(hXvd, (" vec_index: %d", pCfgCmd->vec_index));
   BXVD_DBG_MSG(hXvd, (" interrupt_mask_0: 0x%08x", pCfgCmd->interrupt_mask_0));
   BXVD_DBG_MSG(hXvd, (" interrupt_mask_1: 0x%08x", pCfgCmd->interrupt_mask_1));

   eStatus = BERR_TRACE(BXVD_P_SendDecoderCommand(hXvd,
                                                  hXvd->uDecoderInstance,
                                                  pCmd,
                                                  pRsp,
                                                  FW_CMD_TIMEOUT_NORMAL));

   BXVD_DBG_MSG(hXvd, ("BXVD_RSP_CONFIG:"));
   BXVD_DBG_MSG(hXvd, (" command: %#x", pCfgRsp->command));
   BXVD_DBG_MSG(hXvd, (" status: %#x", pCfgRsp->status));

   return BERR_TRACE(eStatus);
}

/***************************************************************************
 *  {secret}
 *  BXVD_P_HostCmdSendDecChannelOpen
 ***************************************************************************/
BERR_Code BXVD_P_HostCmdSendDecChannelOpen
(
    BXVD_Handle         hXvd,
    BXVD_ChannelHandle  hXvdCh,
        bool                bStillPictureMode,
        uint32_t            uiMaxResolution,
        BXVD_P_DecodeFWMemSize  *pstDecodeFWMemSize,
        BXVD_P_DecodeFWBaseAddrs *pstDecodeFWBaseAddrs
)
{
   BXVD_Cmd             *pCmd;
   BXVD_Rsp             *pRsp;

   BERR_Code        eStatus = BERR_SUCCESS;/* Status return       */
   BXVD_Cmd_ChannelOpen *pChOpen;   /* Pointer to command  */
   BXVD_Rsp_ChannelOpen *pChOpenRsp;    /* Pointer to response */

#if !BXVD_P_FW_HIM_API
   uint32_t            uiStatusBlkAddr;
#endif

   /* Clear out the context and set defaults. */
   pCmd = &hXvd->FWCmd.cmd;
   pRsp = &hXvd->FWRsp.rsp;
   BKNI_Memset((void*)pCmd, BXVD_P_MEM_ZERO, sizeof(BXVD_Cmd));
   BKNI_Memset((void*)pRsp, BXVD_P_MEM_ZERO, sizeof(BXVD_Rsp));

   pChOpen = &hXvd->FWCmd.channelOpen;
   pChOpenRsp = &hXvd->FWRsp.channelOpen;

   /* Command ID */
   pChOpen->command = BXVD_CMD_CHANNELOPEN;
   pChOpen->channel_number = hXvdCh->ulChannelNum;

   pChOpen->still_picture_mode = (unsigned long) bStillPictureMode;

   if (uiMaxResolution == BXVD_DecodeResolution_e4K)
   {
      pChOpen->max_resolution_enum = BXVD_P_AVD_DECODE_RES_4K;
   }
   else
   {
      pChOpen->max_resolution_enum = uiMaxResolution;
   }

   pChOpen->context_memory_base = (uint32_t)(pstDecodeFWBaseAddrs->FWContextBase);
   pChOpen->context_memory_size = pstDecodeFWMemSize->uiFWContextSize;

   pChOpen->video_memory_base = (uint32_t)(pstDecodeFWBaseAddrs->FWPicBase + hXvdCh->uiFWPicOffset);
   pChOpen->video_block_size = pstDecodeFWMemSize->uiFWPicLumaBlockSize;

   if (pstDecodeFWBaseAddrs->FWPicBase1  != (BXVD_P_PHY_ADDR) 0)
   {
      pChOpen->chroma_memory_base = (uint32_t)(pstDecodeFWBaseAddrs->FWPicBase1 + hXvdCh->uiFWPicOffset1);
      pChOpen->chroma_block_size = pstDecodeFWMemSize->uiFWPicChromaBlockSize;
   }


   pChOpen->video_block_count = pstDecodeFWMemSize->uiFWPicBlockCount;

   pChOpen->cabac_memory_base = (uint32_t)(pstDecodeFWBaseAddrs->FWCabacBase);
   pChOpen->cabac_memory_size = pstDecodeFWMemSize->uiFWCabacSize;

   pChOpen->cabac_wl_base = (uint32_t)(pstDecodeFWBaseAddrs->FWCabacWorklistBase);
   pChOpen->cabac_wl_size = pstDecodeFWMemSize->uiFWCabacWorklistSize;

   pChOpen->direct_mode_storage_base = (uint32_t)(pstDecodeFWBaseAddrs->FWDirectModeBase);
   pChOpen->direct_mode_storage_size = pstDecodeFWMemSize->uiFWDirectModeSize;
   pChOpen->il_wl_base = (uint32_t)(pstDecodeFWBaseAddrs->FWInnerLoopWorklistBase);
   pChOpen->il_wl_size = pstDecodeFWMemSize->uiFWInnerLoopWorklistSize;

#if BXVD_P_FW_40BIT_API
   pChOpen->context_memory_base_hi = (uint32_t)(pstDecodeFWBaseAddrs->FWContextBase >> 32);
   pChOpen->video_memory_base_hi = (uint32_t)((pstDecodeFWBaseAddrs->FWPicBase + hXvdCh->uiFWPicOffset) >> 32);

   if (pstDecodeFWBaseAddrs->FWPicBase1  != (BXVD_P_PHY_ADDR) 0)
   {
      pChOpen->chroma_memory_base_hi = (uint32_t)((pstDecodeFWBaseAddrs->FWPicBase1 + hXvdCh->uiFWPicOffset1) >> 32);
   }

   pChOpen->cabac_memory_base_hi = (uint32_t)(pstDecodeFWBaseAddrs->FWCabacBase >> 32);
   pChOpen->cabac_wl_base_hi = (uint32_t)(pstDecodeFWBaseAddrs->FWCabacWorklistBase >> 32);
   pChOpen->direct_mode_storage_base_hi = (uint32_t)(pstDecodeFWBaseAddrs->FWDirectModeBase >> 32);
   pChOpen->il_wl_base_hi = (uint32_t)(pstDecodeFWBaseAddrs->FWInnerLoopWorklistBase >> 32);

#elif (!BXVD_P_FW_40BIT_API)
   pChOpen->bl_mv_store_base = pstDecodeFWBaseAddrs->uiFWInterLayerMVBase;
   pChOpen->bl_mv_store_size = pstDecodeFWMemSize->uiFWInterLayerMVSize;

   pChOpen->bl_video_store_base = pstDecodeFWBaseAddrs->uiFWInterLayerPicBase;
   pChOpen->bl_video_store_size = pstDecodeFWMemSize->uiFWInterLayerPicSize;
#endif
   /* parameters */

   BXVD_DBG_MSG(hXvd, ("BXVD_CMD_CHANNELOPEN:"));
   BXVD_DBG_MSG(hXvd, (" command: %#x", pChOpen->command));
   BXVD_DBG_MSG(hXvd, (" channel_number: %d", pChOpen->channel_number));
   BXVD_DBG_MSG(hXvd, (" max_resolution_enum: %d", pChOpen->max_resolution_enum));
   BXVD_DBG_MSG(hXvd, (" still_picture_mode: %d", pChOpen->still_picture_mode));
#if BXVD_P_FW_40BIT_API
   BXVD_DBG_MSG(hXvd, (" context_memory_base_hi: %#x", pChOpen->context_memory_base_hi));
#endif
   BXVD_DBG_MSG(hXvd, (" context_memory_base: %#x", pChOpen->context_memory_base));
   BXVD_DBG_MSG(hXvd, (" context_memory_size: %#x", pChOpen->context_memory_size));
#if BXVD_P_FW_40BIT_API
   BXVD_DBG_MSG(hXvd, (" video_memory_base_hi: %#x", pChOpen->video_memory_base_hi));
#endif
   BXVD_DBG_MSG(hXvd, (" video_memory_base: %#x", pChOpen->video_memory_base));
   BXVD_DBG_MSG(hXvd, (" video_block_size: %#x", pChOpen->video_block_size));

#if BXVD_P_FW_40BIT_API
   BXVD_DBG_MSG(hXvd, (" chroma_memory_base_hi: %#x", pChOpen->chroma_memory_base_hi));
#endif
   BXVD_DBG_MSG(hXvd, (" chroma_memory_base: %#x", pChOpen->chroma_memory_base));
   BXVD_DBG_MSG(hXvd, (" chroma_block_size: %#x", pChOpen->chroma_block_size));
   BXVD_DBG_MSG(hXvd, (" video_block_count: %d", pChOpen->video_block_count));
#if BXVD_P_FW_40BIT_API
   BXVD_DBG_MSG(hXvd, (" cabac_memory_base_hi: %#x", pChOpen->cabac_memory_base_hi));
#endif
   BXVD_DBG_MSG(hXvd, (" cabac_memory_base: %#x", pChOpen->cabac_memory_base));
   BXVD_DBG_MSG(hXvd, (" cabac_memory_size: %#x", pChOpen->cabac_memory_size));
#if !BXVD_P_FW_40BIT_API
   BXVD_DBG_MSG(hXvd, (" bl_mv_store_base: %#x", pChOpen->bl_mv_store_base));
   BXVD_DBG_MSG(hXvd, (" bl_mv_store_size: %#x", pChOpen->bl_mv_store_size));
   BXVD_DBG_MSG(hXvd, (" bl_video_store_base: %#x", pChOpen->bl_video_store_base));
   BXVD_DBG_MSG(hXvd, (" bl_video_store_size: %#x", pChOpen->bl_video_store_size));
#endif
#if BXVD_P_FW_40BIT_API
   BXVD_DBG_MSG(hXvd, (" cabac_worklist_base_hi: %#x", pChOpen->cabac_wl_base_hi));
#endif
   BXVD_DBG_MSG(hXvd, (" cabac_worklist_base: %#x", pChOpen->cabac_wl_base));
   BXVD_DBG_MSG(hXvd, (" cabac_worklist_size: %#x", pChOpen->cabac_wl_size));
#if BXVD_P_FW_40BIT_API
   BXVD_DBG_MSG(hXvd, (" direct_mode_storage_base_hi: %#x", pChOpen->direct_mode_storage_base_hi));
#endif
   BXVD_DBG_MSG(hXvd, (" direct_mode_storage_base: %#x", pChOpen->direct_mode_storage_base));
   BXVD_DBG_MSG(hXvd, (" direct_mode_storage_size: %#x", pChOpen->direct_mode_storage_size));
#if BXVD_P_FW_40BIT_API
   BXVD_DBG_MSG(hXvd, (" il_wl_base_hi: %#x", pChOpen->il_wl_base_hi));
#endif
   BXVD_DBG_MSG(hXvd, (" il_wl_base: %#x", pChOpen->il_wl_base));
   BXVD_DBG_MSG(hXvd, (" il_wl_size: %#x", pChOpen->il_wl_size));

   eStatus = BERR_TRACE(BXVD_P_SendDecoderCommand(hXvd,
                                                  hXvd->uDecoderInstance,
                                                  pCmd,
                                                  pRsp,
                                                  FW_CMD_TIMEOUT_NORMAL));

   BXVD_DBG_MSG(hXvd, ("BXVD_RSP_CHANNELOPEN:"));
   BXVD_DBG_MSG(hXvd, (" command: %#x", pChOpenRsp->command));
   BXVD_DBG_MSG(hXvd, (" status: %#x", pChOpenRsp->status));
   BXVD_DBG_MSG(hXvd, (" picture_delivery_buffer: %#x", pChOpenRsp->picture_delivery_buffer));
   BXVD_DBG_MSG(hXvd, (" picture_release_buffer: %#x", pChOpenRsp->picture_release_buffer));

   /* Save the return info of interest. */

#if BXVD_P_FW_HIM_API

   /* Initialize the delivery queue parameters.
    */
   BKNI_Memset( &(hXvdCh->stDeliveryQueue), 0, sizeof ( BXVD_P_AVD_Queue ) );

#if !BXVD_P_FW_40BIT_API
   hXvdCh->stDeliveryQueue.ulQueueOffset  = (unsigned long) (pChOpenRsp->picture_delivery_buffer);
#else
   hXvdCh->stDeliveryQueue.ulQueueOffset  = (unsigned long) (pChOpenRsp->picture_delivery_buffer) + (unsigned long) (pChOpen->context_memory_base);
#endif
   hXvdCh->stDeliveryQueue.stReadIndex.ulByteOffset   = pChOpenRsp->delivery_q_read_byte_offset;
   BXVD_P_HOSTINTERFACEMEMORY_COOK_OFFSETS( hXvdCh->stDeliveryQueue.stReadIndex, 1 );

   hXvdCh->stDeliveryQueue.stWriteIndex.ulByteOffset  = pChOpenRsp->delivery_q_write_byte_offset;
   BXVD_P_HOSTINTERFACEMEMORY_COOK_OFFSETS( hXvdCh->stDeliveryQueue.stWriteIndex, 1 );

   /* Initialize the release queue parameters.
    */
   BKNI_Memset( &(hXvdCh->stReleaseQueue), 0, sizeof ( BXVD_P_AVD_Queue ) );

#if !BXVD_P_FW_40BIT_API
   hXvdCh->stReleaseQueue.ulQueueOffset = pChOpenRsp->picture_release_buffer;
#else
   hXvdCh->stReleaseQueue.ulQueueOffset = (unsigned long) pChOpenRsp->picture_release_buffer + (unsigned long) (pChOpen->context_memory_base);
#endif

   hXvdCh->stReleaseQueue.stReadIndex.ulByteOffset = pChOpenRsp->release_q_read_byte_offset;
   BXVD_P_HOSTINTERFACEMEMORY_COOK_OFFSETS( hXvdCh->stReleaseQueue.stReadIndex, 1 );

   hXvdCh->stReleaseQueue.stWriteIndex.ulByteOffset = pChOpenRsp->release_q_write_byte_offset;
   BXVD_P_HOSTINTERFACEMEMORY_COOK_OFFSETS( hXvdCh->stReleaseQueue.stWriteIndex, 1 );

   hXvdCh->stReleaseQueue.stShadowWriteIndex.ulByteOffset = pChOpenRsp->shadow_write_byte_offset;
   BXVD_P_HOSTINTERFACEMEMORY_COOK_OFFSETS( hXvdCh->stReleaseQueue.stShadowWriteIndex, 1 );

   /* Initialize the drop count parameters.
    */
   BKNI_Memset( &(hXvdCh->stDropCountIndex), 0, sizeof ( BXVD_P_HIM_Offsets ) );

   hXvdCh->stDropCountIndex.ulByteOffset = pChOpenRsp->drop_count_byte_offset;
   BXVD_P_HOSTINTERFACEMEMORY_COOK_OFFSETS( hXvdCh->stDropCountIndex, 2 );

   /* Initialize the AVD status block parameters.
    */
   BKNI_Memset( &(hXvdCh->stStatusBlockIndex), 0, sizeof ( BXVD_P_HIM_Offsets ) );

   hXvdCh->stStatusBlockIndex.ulByteOffset = pChOpenRsp->avd_status_addr;
   BXVD_P_HOSTINTERFACEMEMORY_COOK_OFFSETS( hXvdCh->stStatusBlockIndex, 4 );

   /* Initialize the Cabac Bin Fullness parameter.
    */
   hXvdCh->stCabacBinFullnessIndex.ulByteOffset = pChOpenRsp->bin_fullness_offset;
   BXVD_P_HOSTINTERFACEMEMORY_COOK_OFFSETS( hXvdCh->stCabacBinFullnessIndex, 2 );

   hXvdCh->ulAvdStatusBlock = 0;

#else
   BXVD_DBG_MSG(hXvd, (" dms_release_buffer: %#x", pChOpenRsp->dm_return_address));
   BXVD_DBG_MSG(hXvd, (" avd_status_address = %#x", pChOpenRsp->avd_status_address));

   hXvdCh->ulPicBuf           = pChOpenRsp->picture_delivery_buffer;
   hXvdCh->ulPicRelBuf        = pChOpenRsp->picture_release_buffer;
   hXvdCh->ulPicInfoRelBuf    = pChOpenRsp->dm_return_address;

   uiStatusBlkAddr            = (uint32_t) pChOpenRsp->avd_status_address;

   hXvdCh->ulAvdStatusBlock = (uint32_t) BXVD_P_OFFSET_TO_VA(hXvdCh, uiStatusBlkAddr);
#endif

   hXvdCh->bDecoderChannelOpened = true;

   return BERR_TRACE(eStatus);
}

/***************************************************************************
 *  {secret}
 *  BXVD_P_HostCmdSendDecChannelClose
 ***************************************************************************/
BERR_Code BXVD_P_HostCmdSendDecChannelClose
(
   BXVD_Handle         hXvd,
   BXVD_ChannelHandle  hXvdCh
)
{
   BXVD_Cmd             *pCmd;
   BXVD_Rsp             *pRsp;

   BERR_Code        eStatus  = BERR_SUCCESS; /* Status return */

   BXVD_Cmd_ChannelClose *pChClose;      /* Pointer to command  */

#if BDBG_DEBUG_BUILD
   BXVD_Rsp_ChannelClose *pChCloseRsp;      /* Pointer to response */

   pChCloseRsp = &hXvd->FWRsp.channelClose;
#endif

   /* Clear out the context and set defaults. */
   pCmd = &hXvd->FWCmd.cmd;
   pRsp = &hXvd->FWRsp.rsp;
   BKNI_Memset((void*)pCmd, BXVD_P_MEM_ZERO, sizeof(BXVD_Cmd));
   BKNI_Memset((void*)pRsp, BXVD_P_MEM_ZERO, sizeof(BXVD_Rsp));

   pChClose = &hXvd->FWCmd.channelClose;

   /* Command ID */
   pChClose->command = BXVD_CMD_CHANNELCLOSE;
   pChClose->channel_number = hXvdCh->ulChannelNum;

   BXVD_DBG_MSG(hXvd, ("BXVD_CMD_CHANNELCLOSE:"));
   BXVD_DBG_MSG(hXvd, (" command: %#x", pChClose->command));
   BXVD_DBG_MSG(hXvd, (" channel_number: %d", pChClose->channel_number));

   eStatus = BERR_TRACE(BXVD_P_SendDecoderCommand(hXvd,
                                                  hXvd->uDecoderInstance,
                                                  pCmd,
                                                  pRsp,
                                                  FW_CMD_TIMEOUT_NORMAL));

   BXVD_DBG_MSG(hXvd, ("BXVD_RSP_CHANNELCLOSE:"));
   BXVD_DBG_MSG(hXvd, (" command: %#x", pChCloseRsp->command));
   BXVD_DBG_MSG(hXvd, (" status: %#x", pChCloseRsp->status));

   hXvdCh->bDecoderChannelOpened = false;

   return BERR_TRACE(eStatus);
}

/***************************************************************************
 *  {secret}
 *  BXVD_P_HostCmdSendDecChannelOpen
 ***************************************************************************/
BERR_Code BXVD_P_HostCmdSendDecChannelStart
(
   BXVD_Handle         hXvd,
   uint32_t            ulChannelNumber,
   uint32_t            eProtocol,
   uint32_t            eChannelMode,
   uint32_t            ulRaveContextBase,
   uint32_t            aulRaveContextBaseExt[],
   uint32_t            ulVecIndex
   )
{
   BXVD_Cmd             *pCmd;
   BXVD_Rsp             *pRsp;

   BERR_Code        eStatus = BERR_SUCCESS;/* Status return       */
   BXVD_Cmd_ChannelStart    *pChStart;  /* Pointer to command  */

#if BDBG_DEBUG_BUILD
   BXVD_Rsp_ChannelStart    *pChStartRsp;   /* Pointer to response */

   pChStartRsp = &hXvd->FWRsp.channelStart;
#endif

   /* Clear out the context and set defaults. */
   pCmd = &hXvd->FWCmd.cmd;
   pRsp = &hXvd->FWRsp.rsp;
   BKNI_Memset((void*)pCmd, 0x0, sizeof(BXVD_Cmd));
   BKNI_Memset((void*)pRsp, 0x0, sizeof(BXVD_Rsp));

   pChStart = &hXvd->FWCmd.channelStart;

   /* Command ID */
   pChStart->command = BXVD_CMD_CHANNELSTART;

   /* parameters */
   pChStart->protocol = eProtocol;

   pChStart->channel_mode = eChannelMode;
   pChStart->channel_number = ulChannelNumber;;

   pChStart->rave_ctxt_base = ulRaveContextBase;
   pChStart->rave_ctxt_base_ext = aulRaveContextBaseExt[0];
   pChStart->vec_index = ulVecIndex;

   BXVD_DBG_MSG(hXvd, ("BXVD_CMD_CHANNELSTART:"));
   BXVD_DBG_MSG(hXvd, (" command: %#x", pChStart->command));
   BXVD_DBG_MSG(hXvd, (" protocol: %d", pChStart->protocol));
   BXVD_DBG_MSG(hXvd, (" channel_mode: %#x", pChStart->channel_mode));
   BXVD_DBG_MSG(hXvd, (" vec_index: %d", pChStart->vec_index));
   BXVD_DBG_MSG(hXvd, (" channel_number: %d", pChStart->channel_number));
   BXVD_DBG_MSG(hXvd, (" rave_ctxt_base: %#x", pChStart->rave_ctxt_base));
   BXVD_DBG_MSG(hXvd, (" rave_ctxt_base_ext: %#x", pChStart->rave_ctxt_base_ext));

   eStatus = BERR_TRACE(BXVD_P_SendDecoderCommand(hXvd,
                                                  hXvd->uDecoderInstance,
                                                  pCmd,
                                                  pRsp,
                                                  FW_CMD_TIMEOUT_NORMAL));

   BXVD_DBG_MSG(hXvd, ("BXVD_RSP_CHANNELSTART:"));
   BXVD_DBG_MSG(hXvd, (" command: %#x", pChStartRsp->command));
   BXVD_DBG_MSG(hXvd, (" status: %#x", pChStartRsp->status));
   BXVD_DBG_MSG(hXvd, (" frames_outstanding: %d", pChStartRsp->frames_outstanding));

   return BERR_TRACE(eStatus);
}



/***************************************************************************
 *  {secret}
 *  BXVD_P_HostCmdSendDecChannelStop
 ***************************************************************************/
BERR_Code BXVD_P_HostCmdSendDecChannelStop
(
    BXVD_Handle        hXvd,
    uint32_t           ulChannelNum
)
{
   BXVD_Cmd             *pCmd;
   BXVD_Rsp             *pRsp;

   BERR_Code        eStatus  = BERR_SUCCESS; /* Status return */

   BXVD_Cmd_ChannelStop *pChStop;        /* Pointer to command  */

#if BDBG_DEBUG_BUILD
   BXVD_Rsp_ChannelStop *pChStopRsp;      /* Pointer to response */

   pChStopRsp = &hXvd->FWRsp.channelStop;
#endif
   BSTD_UNUSED(ulChannelNum);

   /* Clear out the context and set defaults. */
   pCmd = &hXvd->FWCmd.cmd;
   pRsp = &hXvd->FWRsp.rsp;
   BKNI_Memset((void*)pCmd, BXVD_P_MEM_ZERO, sizeof(BXVD_Cmd));
   BKNI_Memset((void*)pRsp, BXVD_P_MEM_ZERO, sizeof(BXVD_Rsp));

   pChStop = &hXvd->FWCmd.channelStop;

   /* Command ID */
   pChStop->command = BXVD_CMD_CHANNELSTOP;
   pChStop->channel_number = ulChannelNum;

   BXVD_DBG_MSG(hXvd, ("BXVD_CMD_CHANNELSTOP:"));
   BXVD_DBG_MSG(hXvd, (" command: %#x", pChStop->command));
   BXVD_DBG_MSG(hXvd, (" channel_number: %d", pChStop->channel_number));

   eStatus = BERR_TRACE(BXVD_P_SendDecoderCommand(hXvd,
                                                  hXvd->uDecoderInstance,
                                                  pCmd,
                                                  pRsp,
                                                  FW_CMD_TIMEOUT_NORMAL));

   BXVD_DBG_MSG(hXvd, ("BXVD_RSP_CHANNELSTOP:"));
   BXVD_DBG_MSG(hXvd, (" command: %#x", pChStopRsp->command));
   BXVD_DBG_MSG(hXvd, (" status: %#x", pChStopRsp->status));

   return BERR_TRACE(eStatus);
}

/***************************************************************************
 *  {secret}
 *  BXVD_P_HostCmdSetSkipPictureMode
 ***************************************************************************/
BERR_Code BXVD_P_HostCmdSetSkipPictureMode
(
   BXVD_Handle    hXvd,
   uint32_t       ulChannelNum,
   BXVD_SkipMode  eSkipMode
)
{
   BERR_Code        eStatus = BERR_SUCCESS;
   BXVD_Cmd             *pCmd;
   BXVD_Rsp             *pRsp;

   BXVD_Cmd_ChannelSpeed *pChanSpeed;
#if BDBG_DEBUG_BUILD
   BXVD_Rsp_ChannelSpeed *pChanSpeedRsp;

   pChanSpeedRsp = &hXvd->FWRsp.channelSpeed;
#endif
   /* Clear out the context and set defaults. */
   pCmd = &hXvd->FWCmd.cmd;
   pRsp = &hXvd->FWRsp.rsp;
   BKNI_Memset((void*)pCmd, BXVD_P_MEM_ZERO, sizeof(BXVD_Cmd));
   BKNI_Memset((void*)pRsp, BXVD_P_MEM_ZERO, sizeof(BXVD_Rsp));

   pChanSpeed = &hXvd->FWCmd.channelSpeed;

   pChanSpeed->command = BXVD_CMD_CHANNELSPEED;
   pChanSpeed->channel_number = ulChannelNum;

   /*
    * Map XVD skip modes to values used by the firmware.
    */

   switch(eSkipMode)
   {
      case BXVD_SkipMode_eDecode_I_Only:
         pChanSpeed->speed = VDEC_SPEED_IONLY;
         break;
      case BXVD_SkipMode_eDecode_IP_Only:
         pChanSpeed->speed = VDEC_SPEED_IPONLY;
         break;
      case BXVD_SkipMode_eDecode_IPB:
         pChanSpeed->speed = VDEC_SPEED_NORMAL;
         break;
      case BXVD_SkipMode_eDecode_Ref_Only:
         pChanSpeed->speed = VDEC_SPEED_REFONLY;
         break;
      default:
         return BERR_TRACE(BERR_INVALID_PARAMETER);
   };

   BXVD_DBG_MSG(hXvd, ("BXVD_CMD_CHANNELSPEED:"));
   BXVD_DBG_MSG(hXvd, (" command: %#x", pChanSpeed->command));
   BXVD_DBG_MSG(hXvd, (" channel_number: %d", pChanSpeed->channel_number));
   BXVD_DBG_MSG(hXvd, (" speed: %d", pChanSpeed->speed));

   eStatus = BERR_TRACE(BXVD_P_SendDecoderCommand(hXvd,
                                                  hXvd->uDecoderInstance,
                                                  pCmd,
                                                  pRsp,
                                                  FW_CMD_TIMEOUT_NORMAL));

   BXVD_DBG_MSG(hXvd, ("BXVD_RSP_CHANNELSPEED:"));
   BXVD_DBG_MSG(hXvd, (" command: %#x", pChanSpeedRsp->command));
   BXVD_DBG_MSG(hXvd, (" status: %#x", pChanSpeedRsp->status));

   return BERR_TRACE(eStatus);
}


/***************************************************************************
 *  Summary
 *
 *     BXVD_P_HostCmdDramLogControl: Send debug log control command
 *
 *  Description
 *
 *     The decoders outer loop debug output can be loggef to memory for XVD
 *     to read and pass to the application. This routines starts and stops
 *     the logging of the debug data to the log buffer.
 *
 ***************************************************************************/
BERR_Code BXVD_P_HostCmdDbgLogControl
(
   BXVD_Handle    hXvd,
   bool           logStart
)
{
   BERR_Code        eStatus = BERR_SUCCESS;
   BXVD_Cmd             *pCmd;
   BXVD_Rsp             *pRsp;

   BXVD_Cmd_DbgLogControl *pDbgLog;
#if BDBG_DEBUG_BUILD
   BXVD_Rsp_DbgLogControl *pDbgLogRsp;

   pDbgLogRsp = &hXvd->FWRsp.dbgLogControl;
#endif

   /* Clear out the context and set defaults. */
   pCmd = &hXvd->FWCmd.cmd;
   pRsp = &hXvd->FWRsp.rsp;
   BKNI_Memset((void*)pCmd, BXVD_P_MEM_ZERO, sizeof(BXVD_Cmd));
   BKNI_Memset((void*)pRsp, BXVD_P_MEM_ZERO, sizeof(BXVD_Rsp));

   pDbgLog = &hXvd->FWCmd.dbgLogControl;

   pDbgLog->command = BXVD_CMD_DBGLOGCONTROL;
   pDbgLog->logStart = (uint32_t) logStart;

   pDbgLog->dbglog_memory_base = (uint32_t) hXvd->DecoderDbgBufPhyAddr;

#if BXVD_P_FW_40BIT_API
   pDbgLog->dbglog_memory_base_hi = (uint32_t) (hXvd->DecoderDbgBufPhyAddr >> 32);
#endif

   pDbgLog->dbglog_memory_size = hXvd->stSettings.uiDecoderDebugLogBufferSize;

   BXVD_DBG_MSG(hXvd, ("BXVD_CMD_DBGLogControl:"));
   BXVD_DBG_MSG(hXvd, (" command: %#x", pDbgLog->command));
   BXVD_DBG_MSG(hXvd, (" logStart: %d", pDbgLog->logStart));
#if BXVD_P_FW_40BIT_API
   BXVD_DBG_MSG(hXvd, (" dbglog_mem_base_hi: %08x", pDbgLog->dbglog_memory_base_hi));
#endif
   BXVD_DBG_MSG(hXvd, (" dbglog_mem_base: %08x", pDbgLog->dbglog_memory_base));
   BXVD_DBG_MSG(hXvd, (" dbglog_mem_size: %08x", pDbgLog->dbglog_memory_size));

   eStatus = BERR_TRACE(BXVD_P_SendDecoderCommand(hXvd,
                                                  hXvd->uDecoderInstance,
                                                  pCmd,
                                                  pRsp,
                                                  FW_CMD_TIMEOUT_NORMAL));

   BXVD_DBG_MSG(hXvd, ("BXVD_RSP_DbgLogControl"));
   BXVD_DBG_MSG(hXvd, (" command: %#x", pDbgLogRsp->command));
   BXVD_DBG_MSG(hXvd, (" status: %#x", pDbgLogRsp->status));

   return BERR_TRACE(eStatus);
}


/***************************************************************************
 *  Summary
 *
 *     BXVD_P_HostCmdDbgLogCommand: Send debug command
 *
 *  Description
 *
 *     The decoders outer loop will process debug command, the output for
 *     the command is written to the debug log buffer. This routine sends
 *     the outer loop the debug command string.
 *
 ***************************************************************************/
BERR_Code BXVD_P_HostCmdDbgLogCommand
(
   BXVD_Handle    hXvd,
   char           *pCommand
)
{
   BERR_Code        eStatus = BERR_SUCCESS;
   BXVD_Cmd             *pCmd;
   BXVD_Rsp             *pRsp;

#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
   int32_t  len, i;
   uint32_t *pTmpCmd, *pTmpLogCmd;
#endif

   BXVD_Cmd_DbgLogCommand *pDbgCmd;

#if BDBG_DEBUG_BUILD
   BXVD_Rsp_DbgLogCommand *pDbgCmdRsp;

   pDbgCmdRsp = &hXvd->FWRsp.dbgLogCommand;
#endif

   /* Clear out the context and set defaults. */
   pCmd = &hXvd->FWCmd.cmd;
   pRsp = &hXvd->FWRsp.rsp;
   BKNI_Memset((void*)pCmd, BXVD_P_MEM_ZERO, sizeof(BXVD_Cmd));
   BKNI_Memset((void*)pRsp, BXVD_P_MEM_ZERO, sizeof(BXVD_Rsp));

   pDbgCmd = &hXvd->FWCmd.dbgLogCommand;

   pDbgCmd->command = BXVD_CMD_DBGLOGCOMMAND;

#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE
   BKNI_Memcpy(&pDbgCmd->logCmd[0], pCommand, sizeof(pDbgCmd->logCmd));
#else
   len = _strlen(pCommand)/4;

   pTmpCmd = (uint32_t *)pCommand;
   pTmpLogCmd = (uint32_t *)&pDbgCmd->logCmd[0];

   for ( i = 0 ; i <= len ; i++ )
   {
      *pTmpLogCmd = BXVD_P_SWAP32(*pTmpCmd);
      pTmpCmd++;
      pTmpLogCmd++;
   }
#endif

   BXVD_DBG_MSG(hXvd, ("BXVD_CMD_Dbg_LOG_CMD:"));
   BXVD_DBG_MSG(hXvd, (" command: %#x", pDbgCmd->command));
   BXVD_DBG_MSG(hXvd, (" logCmd: %s", &pDbgCmd->logCmd[0]));

   eStatus = BERR_TRACE(BXVD_P_SendDecoderCommand(hXvd,
                                                  hXvd->uDecoderInstance,
                                                  pCmd,
                                                  pRsp,
                                                  FW_CMD_TIMEOUT_DEBUGCMD));

   BXVD_DBG_MSG(hXvd, ("BXVD_RSP_DbgLogCommand"));
   BXVD_DBG_MSG(hXvd, (" command: %#x", pDbgCmdRsp->command));
   BXVD_DBG_MSG(hXvd, (" status: %#x", pDbgCmdRsp->status));

   return BERR_TRACE(eStatus);
}

#if BXVD_P_ENABLE_DRAM_PREF_INFO
/***************************************************************************
 *  Summary
 *
 *     BXVD_P_HostCmdDramPerf: Send dram performace capture command.
 *
 *  Description
 *
 *     The decoder will enable and capture DRAM performance data and store
 *     PPB,
 *
 ***************************************************************************/
BERR_Code BXVD_P_HostCmdDramPerf
(
   BXVD_Handle    hXvd,
   unsigned int   uiDDRStatCtrlReg,
   unsigned int   uiDDRStatCtrlVal,
   unsigned int   uiDDRStatCtrlEnableMask,
   unsigned int   uiDDRStatTimerReg,
   unsigned int   uiClientRead,
   unsigned int   uiCas,
   unsigned int   uiIntraPenality,
   unsigned int   uiPostPenality
)
{
   BERR_Code         eStatus = BERR_SUCCESS;
   BXVD_Cmd          *pCmd;
   BXVD_Rsp          *pRsp;

   BXVD_Cmd_DramPerf *pDramPerf;
#if BDBG_DEBUG_BUILD
   BXVD_Rsp_DramPerf *pDramPerfRsp;

   pDramPerfRsp = &hXvd->FWRsp.dramPerf;
#endif

   /* Clear out the context and set defaults. */
   pCmd = &hXvd->FWCmd.cmd;
   pRsp = &hXvd->FWRsp.rsp;
   BKNI_Memset((void*)pCmd, BXVD_P_MEM_ZERO, sizeof(BXVD_Cmd));
   BKNI_Memset((void*)pRsp, BXVD_P_MEM_ZERO, sizeof(BXVD_Rsp));

   pDramPerf = &hXvd->FWCmd.dramPerf;

   pDramPerf->command = BXVD_CMD_DRAMPERF;
   pDramPerf->ddr_stat_ctrl_reg = uiDDRStatCtrlReg;
   pDramPerf->ddr_stat_ctrl_val = uiDDRStatCtrlVal;
   pDramPerf->ddr_stat_ctrl_enable = uiDDRStatCtrlEnableMask;

   pDramPerf->ddr_stat_timer_reg = uiDDRStatTimerReg;

   pDramPerf->client_read = uiClientRead;
   pDramPerf->cas = uiCas;
   pDramPerf->intra_penalty = uiIntraPenality;
   pDramPerf->post_penalty = uiPostPenality;

   BXVD_DBG_MSG(hXvd, ("BXVD_CMD_DramPerf:"));
   BXVD_DBG_MSG(hXvd, (" ddr_stat_ctrl_reg: %08x", pDramPerf->ddr_stat_ctrl_reg));
   BXVD_DBG_MSG(hXvd, (" ddr_stat_ctrl_val: %08x", pDramPerf->ddr_stat_ctrl_val));
   BXVD_DBG_MSG(hXvd, (" ddr_stat_ctrl_enable: %08x", pDramPerf->ddr_stat_ctrl_enable));
   BXVD_DBG_MSG(hXvd, (" ddr_stat_timer_reg: %08x", pDramPerf->ddr_stat_timer_reg));
   BXVD_DBG_MSG(hXvd, (" client_read: %08x", pDramPerf->client_read));
   BXVD_DBG_MSG(hXvd, (" cas: %08x", pDramPerf->cas));
   BXVD_DBG_MSG(hXvd, (" intra_penalty: %08x", pDramPerf->intra_penalty));
   BXVD_DBG_MSG(hXvd, (" post_penalty: %08x", pDramPerf->post_penalty));

   eStatus = BERR_TRACE(BXVD_P_SendDecoderCommand(hXvd,
                                                  hXvd->uDecoderInstance,
                                                  pCmd,
                                                  pRsp,
                                                  FW_CMD_TIMEOUT_NORMAL));

   BXVD_DBG_MSG(hXvd, ("BXVD_RSP_DramPerf"));
   BXVD_DBG_MSG(hXvd, (" command: %#x", pDramPerfRsp->command));
   BXVD_DBG_MSG(hXvd, (" status: %#x", pDramPerfRsp->status));

   return BERR_TRACE(eStatus);
}
#endif

/* End of File */

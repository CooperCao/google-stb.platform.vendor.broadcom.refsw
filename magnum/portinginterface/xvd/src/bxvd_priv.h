/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

/***************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 *   See Module Overview below.
 *
 *****************************************************************************/
#ifndef BXVD_PRIV_H__
#define BXVD_PRIV_H__

#include "bxvd.h"
#include "bkni.h"
#include "bmma.h"
#include "breg_mem.h"
#include "bxvd_userdata.h"
#include "bxvd_pvr.h"
#include "bchp_bvnf_intr2_3.h"
#include "bxvd_vdec_info.h"
#include "bxvd_vdec_api.h"
#include "bxvd_memory_priv.h"
#include "bxvd_status.h"
#include "bxvd_dip.h"
#include "bxdm_dih.h"
#include "bxdm_pp.h"

#if BXVD_DM_ENABLE_PPB_GRAB_MODE
#include "stdio.h"
#endif

/* needed to get pb_lib.c to compile.
 * better to change pi_util.h?
 */
#include "bxvd_decoder.h"
#include "bxvd_decoder_priv.h"

/* For emulation, RVC capture can be enabled */
/* #define BXVD_P_CAPTURE_RVC 1 */

#ifdef __cplusplus
extern "C" {
#endif

/* Supported stripe width values */
#define BXVD_P_STRIPE_WIDTH_NUM  BXVD_P_PLATFORM_STRIPE_WIDTH_NUM

/* Supported stripe multiple values */
#define BXVD_P_STRIPE_MULTIPLE_NUM 3

#define BXVD_MAX_VIDEO_CHANNELS  16

#define BXVD_P_DIGITS_IN_LONG (int)(sizeof(long)*2)

#define BXVD_P_CORE_REVISION_NUM (BXVD_P_CORE_REVISION - 'A' + 1)

#if EMULATION
#define FW_CMD_TIMEOUT 100000
#else
#define FW_CMD_TIMEOUT 1000
#endif

/***********************************************************************
 *  Private macros
 ***********************************************************************/

/* SW7405-3245: at compile time select either the BDBG_INSTANCE_* or
 * BDBG_* macros.  The BDBG_* macros are substantially faster.
 */
#if BXVD_P_USE_INSTANCE_MACROS
#define BXVD_DBG_MSG(instance, format) BDBG_INSTANCE_MSG(instance, format)
#define BXVD_DBG_WRN(instance, format) BDBG_INSTANCE_WRN(instance, format)
#define BXVD_DBG_ERR(instance, format) BDBG_INSTANCE_ERR(instance, format)
#else
#define BXVD_DBG_MSG(instance, format)    \
   BSTD_UNUSED(instance);                 \
   BDBG_MSG(format)                       \

#define BXVD_DBG_WRN(instance, format)    \
   BSTD_UNUSED(instance);                 \
   BDBG_WRN(format)                       \

#define BXVD_DBG_ERR(instance, format)    \
   BSTD_UNUSED(instance);                 \
   BDBG_ERR(format)                       \

#endif

#define BXVD_IS_MPEG(protocol) ( \
    (BAVC_VideoCompressionStd_eMPEG2 == (protocol)) || \
    (BAVC_VideoCompressionStd_eMPEG2DTV == (protocol)) || \
    (BAVC_VideoCompressionStd_eMPEG1 == (protocol)))

#define BXVD_IS_AVC(protocol) ( \
    (BAVC_VideoCompressionStd_eH265 == (protocol)) || \
    (BAVC_VideoCompressionStd_eH264 == (protocol)) || \
    (BAVC_VideoCompressionStd_eH261== (protocol)) || \
    (BAVC_VideoCompressionStd_eH263== (protocol)))

#define BXVD_IS_AVS(protocol) (BAVC_VideoCompressionStd_eAVS == (protocol))

#define BXVD_P_RESET_CORE( hXvd, uiReg, uiResetMask, string)               \
{                                                                          \
   uint32_t uiRegVal, uiPollCnt;                                           \
   bool bDone;                                                             \
                                                                           \
   uiRegVal = BXVD_Reg_Read32( hXvd, uiReg);                               \
                                                                           \
   uiRegVal |= uiResetMask;                                                \
                                                                           \
   BXVD_Reg_Write32( hXvd, uiReg, uiRegVal);                               \
                                                                           \
   bDone = false;                                                          \
   uiPollCnt = 1;                                                          \
                                                                           \
   while (!bDone)                                                          \
   {                                                                       \
      uiRegVal = BXVD_Reg_Read32( hXvd, uiReg);                            \
                                                                           \
      if (((uiRegVal & uiResetMask) != uiResetMask)                        \
          || (uiPollCnt++ > 100))                                          \
      {                                                                    \
         bDone = true;                                                     \
      }                                                                    \
   }                                                                       \
   BXVD_DBG_MSG(hXvd, ("%s reset pollcnt: %d", string, uiPollCnt));        \
}

#if !BXVD_P_FW_40BIT_API
#define BXVD_P_IS_FW_VERSION_VALID(pInitRsp)                              \
   (((pInitRsp->sw_version >> 8) & 0xff) == BXVD_P_CURRENT_MAJOR_VERSION)
#else
#define BXVD_P_IS_FW_VERSION_VALID(pInitRsp)                              \
   (((pInitRsp->sw_version >> 24) & 0xff) == BXVD_P_CURRENT_MAJOR_VERSION)
#endif

#define BXVD_P_CREATE_PROTOCOLS_MASK(eVidCmprStd) \
   ( 1 << (eVidCmprStd - BAVC_VideoCompressionStd_eMPEG4Part2))

#define BXVD_P_REVE_DECODE_PROTOCOLS_MASK BXVD_P_CREATE_PROTOCOLS_MASK(BAVC_VideoCompressionStd_eMPEG2_DSS_PES)

#define BXVD_P_REVH_DECODE_PROTOCOLS_MASK (BXVD_P_REVE_DECODE_PROTOCOLS_MASK | \
                                           BXVD_P_CREATE_PROTOCOLS_MASK(BAVC_VideoCompressionStd_eMPEG4Part2))

#define BXVD_P_REVI_DECODE_PROTOCOLS_MASK (BXVD_P_REVH_DECODE_PROTOCOLS_MASK | \
                                           BXVD_P_CREATE_PROTOCOLS_MASK( BAVC_VideoCompressionStd_eAVS))

#define BXVD_P_REVJ_DECODE_PROTOCOLS_MASK (BXVD_P_REVI_DECODE_PROTOCOLS_MASK | \
                                           BXVD_P_CREATE_PROTOCOLS_MASK( BAVC_VideoCompressionStd_eMVC))

#define BXVD_P_REVK_DECODE_PROTOCOLS_MASK (BXVD_P_REVJ_DECODE_PROTOCOLS_MASK)

#define BXVD_P_REVL_DECODE_PROTOCOLS_MASK (BXVD_P_REVK_DECODE_PROTOCOLS_MASK | \
                                           BXVD_P_CREATE_PROTOCOLS_MASK( BAVC_VideoCompressionStd_eVP7) | \
                                           BXVD_P_CREATE_PROTOCOLS_MASK( BAVC_VideoCompressionStd_eVP8) | \
                                           BXVD_P_CREATE_PROTOCOLS_MASK( BAVC_VideoCompressionStd_eRV9) | \
                                           BXVD_P_CREATE_PROTOCOLS_MASK( BAVC_VideoCompressionStd_eSPARK))

#define BXVD_P_REVM_DECODE_PROTOCOLS_MASK (BXVD_P_REVL_DECODE_PROTOCOLS_MASK | \
                                           BXVD_P_CREATE_PROTOCOLS_MASK( BAVC_VideoCompressionStd_eVP6))

#if BXVD_P_HVD_PRESENT
#define BXVD_P_REVN_DECODE_PROTOCOLS_MASK (BXVD_P_REVM_DECODE_PROTOCOLS_MASK | \
                                           BXVD_P_CREATE_PROTOCOLS_MASK( BAVC_VideoCompressionStd_eH265))

#define BXVD_P_REVR_DECODE_PROTOCOLS_MASK (BXVD_P_REVN_DECODE_PROTOCOLS_MASK | \
                                           BXVD_P_CREATE_PROTOCOLS_MASK( BAVC_VideoCompressionStd_eVP9))

#define BXVD_P_REVS_DECODE_PROTOCOLS_MASK (BXVD_P_CREATE_PROTOCOLS_MASK( BAVC_VideoCompressionStd_eH265) | \
                                           BXVD_P_CREATE_PROTOCOLS_MASK( BAVC_VideoCompressionStd_eMVC)  | \
                                           BXVD_P_CREATE_PROTOCOLS_MASK( BAVC_VideoCompressionStd_eAVS)  | \
                                           BXVD_P_CREATE_PROTOCOLS_MASK( BAVC_VideoCompressionStd_eVP9))

#define BXVD_P_REVU_DECODE_PROTOCOLS_MASK (BXVD_P_REVS_DECODE_PROTOCOLS_MASK | \
                                           BXVD_P_CREATE_PROTOCOLS_MASK( BAVC_VideoCompressionStd_eAVS2))

#endif

#define BXVD_P_OFFSET_TO_VA(hXvdCh, uiOffset)                                               \
   (hXvdCh->uiFWGenMemBaseVirtAddr + (unsigned long) ((BXVD_P_PHY_ADDR)uiOffset - hXvdCh->FWGenMemBasePhyAddr))

/* Wrapper macros to help debug issues with shared memory and the HIM, in particular the release and
 * delivery queue offsets.  These macros check that only one thread is trying to access the read/write
 * offsets at a time.  They don't check accesses to the queue's themselves.
 * The appropriate code gets compiled in when "BXVD_P_VERIFY_QUEUE_OFFSETS" is defined.
 * When the debug code is not compiled in, the macros simply revert to those defined in "bxvd_core_avd_revXXX.h"
 */

/*
#define BXVD_P_VERIFY_QUEUE_OFFSETS 1
*/

#ifdef BXVD_P_VERIFY_QUEUE_OFFSETS
/*
 * Macros for managing the delivery queue.
 */
#define BXVD_P_DELIVERY_QUEUE_GET_READ_OFFSET( _hXvdCh_, _uiReadOffset_ )                 \
{                                                                                         \
   if ( 0 != _hXvdCh_->pXvd->uiSharedMemoryRefCnt )                                       \
   {                                                                                      \
      uint32_t uiInstanceId = _hXvdCh_->ulChannelNum & 0xF;                               \
      uiInstanceId |= ( _hXvdCh_->pXvd->uDecoderInstance & 0xF ) << 4 ;                   \
                                                                                          \
      BXVD_DBG_ERR(_hXvdCh_,("   [%02x.xxx] BXVD_P_DELIVERY_QUEUE_GET_READ_OFFSET: uiSharedMemoryRefCnt:%d should have been 0",   \
                                  uiInstanceId, _hXvdCh_->pXvd->uiSharedMemoryRefCnt));   \
   }                                                                                      \
   _hXvdCh_->pXvd->uiSharedMemoryRefCnt++;                                                \
   BXVD_AVD_P_DELIVERY_QUEUE_GET_READ_OFFSET( _hXvdCh_, _uiReadOffset_ )                  \
   _hXvdCh_->pXvd->uiSharedMemoryRefCnt--;                                                \
}

#define BXVD_P_DELIVERY_QUEUE_SET_READ_OFFSET( _hXvdCh_, _uiReadOffset_ )                 \
{                                                                                         \
   if ( 0 != _hXvdCh_->pXvd->uiSharedMemoryRefCnt )                                       \
   {                                                                                      \
      uint32_t uiInstanceId = _hXvdCh_->ulChannelNum & 0xF;                               \
      uiInstanceId |= ( _hXvdCh_->pXvd->uDecoderInstance & 0xF ) << 4 ;                   \
                                                                                          \
      BXVD_DBG_ERR(_hXvdCh_,("   [%02x.xxx] BXVD_P_DELIVERY_QUEUE_SET_READ_OFFSET: uiSharedMemoryRefCnt:%d should have been 0",   \
                                  uiInstanceId, _hXvdCh_->pXvd->uiSharedMemoryRefCnt));   \
   }                                                                                      \
   _hXvdCh_->pXvd->uiSharedMemoryRefCnt++;                                                \
   BXVD_AVD_P_DELIVERY_QUEUE_SET_READ_OFFSET( _hXvdCh_, _uiReadOffset_ )                  \
   _hXvdCh_->pXvd->uiSharedMemoryRefCnt--;                                                \
}

#define BXVD_P_DELIVERY_QUEUE_GET_WRITE_OFFSET( _hXvdCh_, _uiWriteOffset_ )               \
{                                                                                         \
   if ( 0 != _hXvdCh_->pXvd->uiSharedMemoryRefCnt )                                       \
   {                                                                                      \
      uint32_t uiInstanceId = _hXvdCh_->ulChannelNum & 0xF;                               \
      uiInstanceId |= ( _hXvdCh_->pXvd->uDecoderInstance & 0xF ) << 4 ;                   \
                                                                                          \
      BXVD_DBG_ERR(_hXvdCh_,("   [%02x.xxx] BXVD_P_DELIVERY_QUEUE_GET_WRITE_OFFSET: uiSharedMemoryRefCnt:%d should have been 0",   \
                                  uiInstanceId, _hXvdCh_->pXvd->uiSharedMemoryRefCnt));   \
   }                                                                                      \
   _hXvdCh_->pXvd->uiSharedMemoryRefCnt++;                                                \
   BXVD_AVD_P_DELIVERY_QUEUE_GET_WRITE_OFFSET( _hXvdCh_, _uiWriteOffset_ )                \
   _hXvdCh_->pXvd->uiSharedMemoryRefCnt--;                                                \
}

/*
 * Macros for managing the release queue.
 */
#define BXVD_P_RELEASE_QUEUE_GET_READ_OFFSET( _hXvdCh_, _uiReadOffset_ )                  \
{                                                                                         \
   if ( 0 != _hXvdCh_->pXvd->uiSharedMemoryRefCnt )                                       \
   {                                                                                      \
      uint32_t uiInstanceId = _hXvdCh_->ulChannelNum & 0xF;                               \
      uiInstanceId |= ( _hXvdCh_->pXvd->uDecoderInstance & 0xF ) << 4 ;                   \
                                                                                          \
      BXVD_DBG_ERR(_hXvdCh_,("   [%02x.xxx] BXVD_P_RELEASE_QUEUE_GET_READ_OFFSET: uiSharedMemoryRefCnt:%d should have been 0",   \
                                  uiInstanceId, _hXvdCh_->pXvd->uiSharedMemoryRefCnt));   \
   }                                                                                      \
   _hXvdCh_->pXvd->uiSharedMemoryRefCnt++;                                                \
   BXVD_AVD_P_RELEASE_QUEUE_GET_READ_OFFSET( _hXvdCh_, _uiReadOffset_ )                   \
   _hXvdCh_->pXvd->uiSharedMemoryRefCnt--;                                                \
}

#define BXVD_P_RELEASE_QUEUE_GET_WRITE_OFFSET( _hXvdCh_, _uiWriteOffset_ )                \
{                                                                                         \
   if ( 0 != _hXvdCh_->pXvd->uiSharedMemoryRefCnt )                                       \
   {                                                                                      \
      uint32_t uiInstanceId = _hXvdCh_->ulChannelNum & 0xF;                               \
      uiInstanceId |= ( _hXvdCh_->pXvd->uDecoderInstance & 0xF ) << 4 ;                   \
                                                                                          \
      BXVD_DBG_ERR(_hXvdCh_,("   [%02x.xxx] BXVD_P_RELEASE_QUEUE_GET_WRITE_OFFSET: uiSharedMemoryRefCnt:%d should have been 0",   \
                                  uiInstanceId, _hXvdCh_->pXvd->uiSharedMemoryRefCnt));   \
   }                                                                                      \
   _hXvdCh_->pXvd->uiSharedMemoryRefCnt++;                                                \
   BXVD_AVD_P_RELEASE_QUEUE_GET_WRITE_OFFSET( _hXvdCh_, _uiWriteOffset_ )                 \
   _hXvdCh_->pXvd->uiSharedMemoryRefCnt--;                                                \
}

#define BXVD_P_RELEASE_QUEUE_SET_WRITE_OFFSET( _hXvdCh_, _uiWriteOffset_ )                \
{                                                                                         \
   if ( 0 != _hXvdCh_->pXvd->uiSharedMemoryRefCnt )                                       \
   {                                                                                      \
      uint32_t uiInstanceId = _hXvdCh_->ulChannelNum & 0xF;                               \
      uiInstanceId |= ( _hXvdCh_->pXvd->uDecoderInstance & 0xF ) << 4 ;                   \
                                                                                          \
      BXVD_DBG_ERR(_hXvdCh_,("   [%02x.xxx] BXVD_P_RELEASE_QUEUE_SET_WRITE_OFFSET: uiSharedMemoryRefCnt:%d should have been 0",   \
                                  uiInstanceId, _hXvdCh_->pXvd->uiSharedMemoryRefCnt));   \
   }                                                                                      \
   _hXvdCh_->pXvd->uiSharedMemoryRefCnt++;                                                \
   BXVD_AVD_P_RELEASE_QUEUE_SET_WRITE_OFFSET( _hXvdCh_, _uiWriteOffset_ )                 \
   _hXvdCh_->pXvd->uiSharedMemoryRefCnt--;                                                \
}

#define BXVD_P_RELEASE_QUEUE_GET_SHADOW_WRITE_OFFSET( _hXvdCh_, _uiWriteOffset_ )         \
{                                                                                         \
   if ( 0 != _hXvdCh_->pXvd->uiSharedMemoryRefCnt )                                       \
   {                                                                                      \
      uint32_t uiInstanceId = _hXvdCh_->ulChannelNum & 0xF;                               \
      uiInstanceId |= ( _hXvdCh_->pXvd->uDecoderInstance & 0xF ) << 4 ;                   \
                                                                                          \
      BXVD_DBG_ERR(_hXvdCh_,("   [%02x.xxx] BXVD_P_RELEASE_QUEUE_GET_SHADOW_WRITE_OFFSET: uiSharedMemoryRefCnt:%d should have been 0",   \
                                  uiInstanceId, _hXvdCh_->pXvd->uiSharedMemoryRefCnt));   \
   }                                                                                      \
   _hXvdCh_->pXvd->uiSharedMemoryRefCnt++;                                                \
   BXVD_AVD_P_RELEASE_QUEUE_GET_SHADOW_WRITE_OFFSET( _hXvdCh_, _uiWriteOffset_ )          \
   _hXvdCh_->pXvd->uiSharedMemoryRefCnt--;                                                \
}

#define BXVD_P_RELEASE_QUEUE_SET_SHADOW_WRITE_OFFSET( _hXvdCh_, _uiWriteOffset_ )         \
{                                                                                         \
   if ( 0 != _hXvdCh_->pXvd->uiSharedMemoryRefCnt )                                       \
   {                                                                                      \
      uint32_t uiInstanceId = _hXvdCh_->ulChannelNum & 0xF;                               \
      uiInstanceId |= ( _hXvdCh_->pXvd->uDecoderInstance & 0xF ) << 4 ;                   \
                                                                                          \
      BXVD_DBG_ERR(_hXvdCh_,("   [%02x.xxx] BXVD_P_RELEASE_QUEUE_SET_SHADOW_WRITE_OFFSET: uiSharedMemoryRefCnt:%d should have been 0",   \
                                  uiInstanceId, _hXvdCh_->pXvd->uiSharedMemoryRefCnt));   \
   }                                                                                      \
   _hXvdCh_->pXvd->uiSharedMemoryRefCnt++;                                                \
   BXVD_AVD_P_RELEASE_QUEUE_SET_SHADOW_WRITE_OFFSET( _hXvdCh_, _uiWriteOffset_ )          \
   _hXvdCh_->pXvd->uiSharedMemoryRefCnt--;                                                \
}

/*
 * Macros for accessing the drop count.
 */
#define BXVD_P_SET_DROP_COUNT( _hXvdCh_, _count_ )                                        \
{                                                                                         \
   if ( 0 != _hXvdCh_->pXvd->uiSharedMemoryRefCnt )                                       \
   {                                                                                      \
      uint32_t uiInstanceId = _hXvdCh_->ulChannelNum & 0xF;                               \
      uiInstanceId |= ( _hXvdCh_->pXvd->uDecoderInstance & 0xF ) << 4 ;                   \
                                                                                          \
      BXVD_DBG_ERR(_hXvdCh_,("   [%02x.xxx] BXVD_P_SET_DROP_COUNT: uiSharedMemoryRefCnt:%d should have been 0",   \
                                  uiInstanceId, _hXvdCh_->pXvd->uiSharedMemoryRefCnt));   \
   }                                                                                      \
   _hXvdCh_->pXvd->uiSharedMemoryRefCnt++;                                                \
   BXVD_AVD_P_SET_DROP_COUNT( _hXvdCh_, _count_ )                                         \
   _hXvdCh_->pXvd->uiSharedMemoryRefCnt--;                                                \
}

#define BXVD_P_GET_DROP_COUNT( _hXvdCh_, _count_ )                                        \
{                                                                                         \
   if ( 0 != _hXvdCh_->pXvd->uiSharedMemoryRefCnt )                                       \
   {                                                                                      \
      uint32_t uiInstanceId = _hXvdCh_->ulChannelNum & 0xF;                               \
      uiInstanceId |= ( _hXvdCh_->pXvd->uDecoderInstance & 0xF ) << 4 ;                   \
                                                                                          \
      BXVD_DBG_ERR(_hXvdCh_,("   [%02x.xxx] BXVD_P_GET_DROP_COUNT: uiSharedMemoryRefCnt:%d should have been 0",   \
                                  uiInstanceId, _hXvdCh_->pXvd->uiSharedMemoryRefCnt));   \
   }                                                                                      \
   _hXvdCh_->pXvd->uiSharedMemoryRefCnt++;                                                \
   BXVD_AVD_P_GET_DROP_COUNT( _hXvdCh_, _count_ )                                         \
   _hXvdCh_->pXvd->uiSharedMemoryRefCnt--;                                                \
}

/*
 * Macro for retrieving the AVD status block.
 */
#define BXVD_P_GET_VIDEO_DECODER_STATUS( _hXvdCh_, _ulStatus_ )                           \
{                                                                                         \
   if ( 0 != _hXvdCh_->pXvd->uiSharedMemoryRefCnt )                                       \
   {                                                                                      \
      uint32_t uiInstanceId = _hXvdCh_->ulChannelNum & 0xF;                               \
      uiInstanceId |= ( _hXvdCh_->pXvd->uDecoderInstance & 0xF ) << 4 ;                   \
                                                                                          \
      BXVD_DBG_ERR(_hXvdCh_,("   [%02x.xxx] BXVD_P_GET_VIDEO_DECODER_STATUS: uiSharedMemoryRefCnt:%d should have been 0",   \
                                  uiInstanceId, _hXvdCh_->pXvd->uiSharedMemoryRefCnt));   \
   }                                                                                      \
   _hXvdCh_->pXvd->uiSharedMemoryRefCnt++;                                                \
   BXVD_AVD_P_GET_VIDEO_DECODER_STATUS( _hXvdCh_, _ulStatus_ )                            \
   _hXvdCh_->pXvd->uiSharedMemoryRefCnt--;                                                \
}

#else
/* When the debug code is not compiled in, the macros simply revert to those
 * defined in "bxvd_core_avd_revxxx.h".
 */

/* Macros for managing the delivery queue. */
#define BXVD_P_DELIVERY_QUEUE_GET_READ_OFFSET   BXVD_AVD_P_DELIVERY_QUEUE_GET_READ_OFFSET
#define BXVD_P_DELIVERY_QUEUE_SET_READ_OFFSET   BXVD_AVD_P_DELIVERY_QUEUE_SET_READ_OFFSET
#define BXVD_P_DELIVERY_QUEUE_GET_WRITE_OFFSET  BXVD_AVD_P_DELIVERY_QUEUE_GET_WRITE_OFFSET

/* Macros for managing the release queue. */
#define BXVD_P_RELEASE_QUEUE_GET_READ_OFFSET    BXVD_AVD_P_RELEASE_QUEUE_GET_READ_OFFSET
#define BXVD_P_RELEASE_QUEUE_GET_WRITE_OFFSET   BXVD_AVD_P_RELEASE_QUEUE_GET_WRITE_OFFSET
#define BXVD_P_RELEASE_QUEUE_SET_WRITE_OFFSET   BXVD_AVD_P_RELEASE_QUEUE_SET_WRITE_OFFSET
#define BXVD_P_RELEASE_QUEUE_GET_SHADOW_WRITE_OFFSET  BXVD_AVD_P_RELEASE_QUEUE_GET_SHADOW_WRITE_OFFSET
#define BXVD_P_RELEASE_QUEUE_SET_SHADOW_WRITE_OFFSET  BXVD_AVD_P_RELEASE_QUEUE_SET_SHADOW_WRITE_OFFSET

/* Macros for accessing the drop count. */
#define BXVD_P_SET_DROP_COUNT    BXVD_AVD_P_SET_DROP_COUNT
#define BXVD_P_GET_DROP_COUNT    BXVD_AVD_P_GET_DROP_COUNT

/* Macro for retrieving the AVD status block. */
#define BXVD_P_GET_VIDEO_DECODER_STATUS   BXVD_AVD_P_GET_VIDEO_DECODER_STATUS

#endif

/* Since these macros don't access the HIM, they map straight through. */
#define BXVD_P_DELIVERY_QUEUE_GET_ADDR    BXVD_AVD_P_DELIVERY_QUEUE_GET_ADDR
#define BXVD_P_RELEASE_QUEUE_GET_ADDR     BXVD_AVD_P_RELEASE_QUEUE_GET_ADDR
#define BXVD_P_GET_QUEUE_DEPTH            BXVD_AVD_P_GET_QUEUE_DEPTH
#define BXVD_P_INCREMENT_2BASED_OFFSET    BXVD_AVD_P_INCREMENT_2BASED_OFFSET

#ifndef BXVD_P_PHY_ADDR
#define BXVD_P_PHY_ADDR BMMA_DeviceOffset
#endif

/***********************************************************************
 *  Private Enums
 ***********************************************************************/

/* Private state to indicate decode state */
typedef enum BXVD_P_DecoderState
{
  BXVD_P_DecoderState_eNotActive=0,    /* Initial state */
  BXVD_P_DecoderState_eActive          /* StartDecode executed */
} BXVD_P_DecoderState;

typedef enum BXVD_P_HandleType
{
   BXVD_P_HandleType_XvdMain=0xdada,
   BXVD_P_HandleType_XvdChannel=0xbdbd,
   BXVD_P_HandleType_Userdata=0xabab,
   BXVD_P_HandleType_Invalid=0xcaca
} BXVD_P_HandleType;

typedef enum BXVD_P_MemCfgMode
{
   BXVD_P_MemCfgMode_eUMA,
   BXVD_P_MemCfgMode_eNONUMA,
   BXVD_P_MemCfgMode_eUNKNOWN
} BXVD_P_MemCfgMode;

typedef enum BXVD_P_PowerState
{
   BXVD_P_PowerState_eOn,
   BXVD_P_PowerState_eClkOff,
   BXVD_P_PowerState_ePwrOff
} BXVD_P_PowerState;

#define  BXVD_P_OUTER_WATCHDOG 0

/* Interrupt related definitions */
#define BXVD_P_INTR_CLEAR   0xffffffff
#define BXVD_P_INTR_OL_MASK 0x40000000

/* AVD PFRI Data Width related definitions */
#define BXVD_P_PFRI_DATA_DDR2 0
#define BXVD_P_PFRI_DATA_DDR3 1

typedef enum BXVD_P_PFRI_Data_Width
{
   BXVD_P_PFRI_Data_Width_e16Bit,
   BXVD_P_PFRI_Data_Width_e32Bit,
   BXVD_P_PFRI_Data_Width_e64Bit
} BXVD_P_PFRI_Data_Width;

typedef enum BXVD_P_PFRI_Stripe_Width
{
   BXVD_P_PFRI_Stripe_Width_e64Bytes,
   BXVD_P_PFRI_Stripe_Width_e128Bytes,
   BXVD_P_PFRI_Stripe_Width_e256Bytes
} BXVD_P_PFRI_Stripe_Width;


typedef enum BXVD_P_PFRI_Num_Banks
{
   BXVD_P_PFRI_Num_Banks_4,
   BXVD_P_PFRI_Num_Banks_8,
   BXVD_P_PFRI_Num_Banks_16
} BXVD_P_PFRI_Num_Banks;

typedef enum BXVD_P_PFRI_Bank_Height
{
   BXVD_P_PFRI_Bank_Height_8n3,
   BXVD_P_PFRI_Bank_Height_16n6,
   BXVD_P_PFRI_Bank_Height_32n12
} BXVD_P_PFRI_Bank_Height;

/* AVD PCache Mode related definitions */
typedef enum BXVD_P_PCache_XGran
{
   BXVD_P_PCache_XGran_e8Bytes,
   BXVD_P_PCache_XGran_e16Bytes,
   BXVD_P_PCache_XGran_e32Bytes
} BXVD_P_PCache_XGran;

typedef enum BXVD_P_PCache_YGran
{
   BXVD_P_PCache_YGran_e1Line,
   BXVD_P_PCache_YGran_e2Lines,
   BXVD_P_PCache_YGran_e4Lines
} BXVD_P_PCache_YGran;

/* Relocation related definitions */
#define BXVD_P_RELF_DEFAULT_CODE_BASE 0x00000000

/* Pre rev K core has a memory access limit of 768 mb */
#define BXVD_P_ARC300_RAM_LIMIT 0x30000000

/* General definitions */
#define BXVD_P_MEM_ZERO 0x00000000

/* If the firmware does not have a time code, or the protocol is not
   MPEG, the internal time code will be set thus */
#define BXVD_P_INVALID_TIMECODE 0xFFFFFFFF

/*
** Passed as a flag in the "channel mode" when
** calling BXVD_P_HostCmdSendDecChannelOpen().
** For best peformance, the bit should be set when DQT is enabled.
**
**      if bit[3] == 0 then it is legacy restricted mode
**      if bit[3] == 1 then it is new all capable AVD mode.
*/
#define BXVD_P_AVD_NO_CONSTRAINTS 0x8


#if (BXVD_P_SVD_PRESENT || BXVD_P_HEVD_DUAL_PIPE_PRESENT)
#define BXVD_P_ARCS_PER_DECODER 3
#else
#define BXVD_P_ARCS_PER_DECODER 2
#endif

typedef struct BXVD_P_CallBackRequests
{
  bool bPauseUntoPTS;
  bool bDisplayUntoPTS;
  bool bPTS1Match;
  bool bPTS2Match;
  bool bPresentationStart;
  bool bPresentationStop;
  bool bMarkerSeen;

/*PR28082*/
  bool bMarker;
  bool bPPBReceived;
  bool bEndOfGOP; /* SW7425-2686: multi-pass DQT, signals that reverse playback of the current GOP has begun. */

} BXVD_P_CallBackRequests;

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/***********************************************************************
 *  Private Structures
 ***********************************************************************/
/* Contains XVD interrupt callbacks and user params */
typedef struct BXVD_P_InterruptCallbackInfo
{
  BXVD_IntCallbackFunc BXVD_P_pAppIntCallbackPtr ;

  void *pParm1;
  int  parm2;
} BXVD_P_InterruptCallbackInfo ;

typedef struct BXVD_P_FWMemConfig
{
   uint32_t general_memory_size;
   uint32_t cabac_bin_size;
   uint32_t video_block_size;
   uint32_t video_block_count;
} BXVD_P_FWMemConfig;

typedef struct BXVD_P_DecodeFWMemSize
{
   uint32_t  uiFWContextSize;
   uint32_t  uiFWCabacSize;
   uint32_t  uiFWPicLumaBlockSize;
   uint32_t  uiFWPicChromaBlockSize;
   uint32_t  uiFWPicBlockCount;
   uint32_t  uiFWCabacWorklistSize;
   uint32_t  uiFWDirectModeSize;
   uint32_t  uiFWInnerLoopWorklistSize;
   uint32_t  uiFWInterLayerPicSize;
   uint32_t  uiFWInterLayerMVSize;
} BXVD_P_DecodeFWMemSize;

typedef struct BXVD_P_DecodeFWBaseAddrs
{
   BMMA_Block_Handle hFWContextBlock;
   BXVD_P_PHY_ADDR   FWContextBase;             /* FW Context memory physical base address */
   BMMA_Block_Handle hFWCabacBlock;
   BXVD_P_PHY_ADDR   FWCabacBase;               /* FW Cabac memory base address */
   BMMA_Block_Handle hFWPicBlock;
   BXVD_P_PHY_ADDR   FWPicBase;                 /* FW Picture Base */
   BMMA_Block_Handle hFWPicBlock1;
   BXVD_P_PHY_ADDR   FWPicBase1;                /* FW Picture Base 1 for split picture buffers */
   BXVD_P_PHY_ADDR   FWCabacWorklistBase;       /* FW Cabac worklist memory base address */
   BXVD_P_PHY_ADDR   FWInnerLoopWorklistBase;   /* FW Inner loop worklist memory base address */
   BXVD_P_PHY_ADDR   FWDirectModeBase;          /* FW Direct Mode memory base address */
   BMMA_Block_Handle hFWInterLayerPicBlock;
   uint32_t          uiFWInterLayerPicBase;     /* FW Inter Layer Pic memory base address */
   uint32_t          uiFWInterLayerMVBase;      /* FW Inter Layer Motion Vector memory base address */
} BXVD_P_DecodeFWBaseAddrs;

/* Internal XVD context data */
typedef struct BXVD_P_Context
{
  /* Interface handles */
  BCHP_Handle hChip;
  BREG_Handle hReg;

  BMMA_Heap_Handle hGeneralHeap;  /* General Heap used to allocate structures */
  BMMA_Heap_Handle hFirmwareHeap; /* Firmware Code Heap used to load firmware */
  BMMA_Heap_Handle hPictureHeap;  /* Picture Buffer Heap used by firmware to store decoded pictures */
  BMMA_Heap_Handle hPictureHeap1; /* Picture Buffer 1 Heap used by firmware to store split decoded pictures */
  BMMA_Heap_Handle hCabacHeap;    /* Cabac Buffer Heap used by firmware as CABAC bin buffer */

  BINT_Handle hInterrupt;

  /* XVD default settings */
  BXVD_Settings stSettings;

  /* Contexts for AVC decoders */
  uint32_t uDecoderInstance;
  BXVD_DecoderContext stDecoderContext;

  /* Indicates a watchdog needs to be processed */
  bool bWatchdogPending;

  BXVD_P_PowerState  eWatchdogSavedPowerState;

  /* Platform-specific fields defined in platform header */
  BXVD_P_CONTEXT_PLATFORM

 /* FW revision information */
  BXVD_RevisionInfo sRevisionInfo;
  uint32_t          uiDecoderFwSha;

  BCHP_ScbMapVer scbMapVer;

  /* For Crc */
  unsigned long        ulDebugDeliveryQueueAddr;
  unsigned long        ulDebugReleaseQueueAddr;

  /* Channel information including count and open handles */
  uint16_t             uiOpenChannels;
  BXVD_ChannelHandle   *ahChannel;

  BLST_S_HEAD(FreeChannelContext, BXVD_P_Channel) FreeChannelListHead;

  /* Suballocated FW memory heaps */
  BXVD_P_MemoryHandle  SubGenMem;
  BXVD_P_MemoryHandle  SubSecureMem;
  BXVD_P_MemoryHandle  SubPicMem;
  BXVD_P_MemoryHandle  SubPicMem1;

  /* Pointers to FW/XVD code memory where the FW is loaded */
  BMMA_Block_Handle    hFWMemBlock;
  unsigned long        uiFWMemBaseVirtAddr;
  uint32_t             uiFWMemBaseUncachedVirtAddr;
  BXVD_P_PHY_ADDR      FWMemBasePhyAddr;

#if BXVD_P_FW_HIM_API
  BMMA_Block_Handle    hFWCmdBlock;
  unsigned long        uiFWCmdVirtAddr;
  BXVD_P_PHY_ADDR      FWCmdPhyAddr;
#endif
  uint32_t             uiFWMemSize;
  uint8_t              bFWMemAllocated;
  BXVD_AVDBootMode     eAVDBootMode;

  /* Some platforms determine AVD core clock freq at runtime */
  uint32_t             uiAVDCoreFreq;

  /* Instruction base addresses for outer and inner loop */

  BXVD_P_PHY_ADDR      uiOuterLoopInstructionBase;
  BXVD_P_PHY_ADDR      uiInnerLoopInstructionBase;

  /* End of code addresses for outer and inner loop */
  uint32_t             uiOuterLoopEOC;
  uint32_t             uiInnerLoopEOC;

#if BXVD_P_HEVD_DUAL_PIPE_PRESENT
  /* Inner Loop 2 Instruction base and end of code */
  BXVD_P_PHY_ADDR      uiInnerLoop2InstructionBase;
  uint32_t             uiInnerLoop2EOC;
#endif


#if BXVD_P_SVD_PRESENT
  /* Base Layer Instruction base and end of code */
  unsigned long        uiBaseInstructionBase;
  uint32_t             uiBaseEOC;
#endif

  /* Outer loop communication vector. This is the same as uiOuterLoopEOC */
  unsigned long        uiCmdBufferVector;

  /* Pointers to FW/XVD shared memory for Picture buffers and FW internal data structs */

  BMMA_Block_Handle    hFWGenMemBlock;
  unsigned long        uiFWGenMemBaseVirtAddr;
  unsigned long        uiFWGenMemBaseUncachedVirtAddr;
  BXVD_P_PHY_ADDR      FWGenMemBasePhyAddr;

  uint32_t             uiFWGenMemSize;

#if BXVD_P_CAPTURE_RVC
  BMMA_Block_Handle    hFWRvcBlock;
  unsigned long        uiFWRvcBaseVirtAddr;
  unsigned long        uiFWRvcBasePhyAddr;
#endif

  /* Pointers to Decoder debug log buffers */
  BMMA_Block_Handle    hFWDbgBuf_MemBlock;
  unsigned long        uiDecoderDbgBufVirtAddr;
  BXVD_P_PHY_ADDR      DecoderDbgBufPhyAddr;

  bool                 bFWDbgLoggingStarted;

  /* Size and pointers to FW Picture buffers */
  BMMA_Block_Handle    hFWPicMemBlock;
  BXVD_P_PHY_ADDR      FWPicMemBasePhyAddr;
  unsigned long        uiFWPicMemBaseVirtAddr;
  uint32_t             uiFWPicMemSize;

  BMMA_Block_Handle    hFWPicMem1Block;
  BXVD_P_PHY_ADDR      FWPicMem1BasePhyAddr;
  unsigned long        uiFWPicMem1BaseVirtAddr;
  uint32_t             uiFWPicMem1Size;

  /* Pointers to FW/XVD Cabac Bin memory */
  BMMA_Block_Handle    hFWCabacMemBlock;
  BXVD_P_PHY_ADDR      FWCabacMemBasePhyAddr;
  uint32_t             uiFWCabacMemSize;

  /* Register offset and mask values */
  uint32_t             uiXPT_PCROffset_STC;

  uint32_t             uiDecode_SDStripeWidthRegVal;
  uint32_t             uiDecode_IPShimPfriRegVal;
  uint32_t             uiDecode_StripeWidth;
  uint32_t             uiDecode_StripeMultiple;
  uint32_t             uiDecode_PFRIDataRegVal;
  uint32_t             uiAVD_PCacheRegVal;
  uint32_t             uiChip_ProductRevision;

  /* PR18233: Mosaic mode support */
  BAVC_XVD_Picture     *pVDCPictureBuffers;
  BAVC_XVD_Picture     *pPictureListVec0;
  BAVC_XVD_Picture     *pPictureListVec1;

  /* Linked Decoder support */
  BXVD_Handle            hXvd_Secondary;
  BXVD_DisplayInterrupt  Secondary_eDisplayInterrupt;

  /* Still picture buffer support */
  unsigned long  watchdog_timer_addr;
  unsigned long  watchdog_write_value;

#if BXVD_P_FW_HIM_API
  uint32_t uiDisplayInfo0_Offset;
  uint32_t uiDisplayInfo1_Offset;
  uint32_t uiDisplayInfo2_Offset;
  uint32_t uiClockBoost_Offset;
#endif

  BXVD_P_DisplayInfo  *pDisplayInfo0;
  BXVD_P_DisplayInfo  *pDisplayInfo1;

  BXVD_P_InterruptCallbackInfo stDeviceInterruptCallbackInfo[BXVD_DeviceInterrupt_eMaxInterrupts] ;

  BXVD_ChannelHandle   hStillChannel;
  bool bStillChannelAllocated;
  uint32_t uiStillChannelNum;
  bool bStillPictureCompatibilityMode;
  bool bStillHDCapable;

  bool bSVCCapable;
  bool bHEVDDualPipe;
  bool bRV9Capable;

  bool bAllocDecodeModeList;

  bool bHibernate;
  BXVD_P_PowerState PowerStateCurrent;
  BXVD_P_PowerState PowerStateSaved;

  BXVD_StatusHandle hXvdStatus;

  BXVD_P_HandleType eHandleType;

  BTMR_TimerHandle hTimer;

  uint32_t auiDisplayInterruptCount[BXVD_DisplayInterrupt_eMax];
  uint32_t auiActiveDecodes[BXVD_ChannelMode_eMax][BXVD_DisplayInterrupt_eMax];

  BXDM_DisplayInterruptHandler_Handle hXdmDih[BXVD_DisplayInterrupt_eMax];
  BXDM_DisplayInterruptHandler_Handle hAppXdmDih[BXVD_DisplayInterrupt_eMax];
  BXDM_DisplayInterruptHandler_Handle hLinkedDecoderXdmDih;
  BXVD_DisplayInterruptProvider_P_ChannelHandle hXvdDipCh[BXVD_DisplayInterrupt_eMax];

#if BXVD_P_AVD_ARC600
  BAFL_FirmwareInfo astFWBootInfo[BXVD_P_ARCS_PER_DECODER];
#endif

  /* FW command and response buffers */
  BXVD_FW_Cmd FWCmd;
  BXVD_FW_Rsp FWRsp;


#ifdef BXVD_P_VERIFY_QUEUE_OFFSETS
   /* to help debug delivery/release queue issues */
   uint32_t uiSharedMemoryRefCnt;
#endif

   /* SW7445-2757: to aid in debug, keep a running count of the
    * number of channels that have been opened, i.e. track the
    * number of times BXVD_OpenChannel() has been called. */
   uint32_t uiChannelOpenCalls;

} BXVD_P_Context;

/* The buffer configuration for the channel */
typedef struct BXVD_P_BufferConfig
{
  BXVD_P_DMS2DMInfo AVD_DMS2PI_Buffer ;
  BXVD_P_DM2DMSInfo *pAVD_PI2DMS_Buffer ;

  uint16_t ui32_HardwareNmbx ;
  uint16_t ui32_HardwareNmby ;

} BXVD_P_BufferConfig;

struct BXVD_P_IntCallback {
  BXVD_IntCallbackFunc callback;
  void *pParm1;
  int   parm2;
};


#include "bxvd_userdata_priv.h"

#if BXVD_P_FW_HIM_API

typedef struct BXVD_P_HIM_Offsets
{
   unsigned long ulByteOffset;      /* byte offset into HIM, returned by AVD. */
   unsigned long ulWordOffset;      /* ulByteOffset divided by 4, XVD always read/writes a word. */
   unsigned long ulByteShift;       /* used to shift the data to the appropriate byte. */
   unsigned long ulByteMask;        /* mask for the byte of interest. */
   unsigned long ulInverseMask;     /* inverse of the ulByteMask, to assist with read-modify-write operations. */
   unsigned long ulBytesPerValue;   /* Indicates if this a 1, 2, 3, or 4 byte value. */

} BXVD_P_HIM_Offsets;

/* Contains the addresses and offsets needed to access the AVD
 * Delivery and Release queues.
 */
typedef struct BXVD_P_AVD_Queue
{
   unsigned long        ulQueueOffset;
   BXVD_P_HIM_Offsets   stReadIndex;
   BXVD_P_HIM_Offsets   stWriteIndex;
   BXVD_P_HIM_Offsets   stShadowWriteIndex;     /* For the release queue. */

} BXVD_P_AVD_Queue;

#endif /* #if BXVD_P_FW_HIM_API */

/* SW7425-1064: Support for the XMO MergeOmatic filter. */
typedef enum BXVD_P_ChannelType
{
   BXVD_P_ChannelType_eStandard = 0,
   BXVD_P_ChannelType_eBase,
   BXVD_P_ChannelType_eEnhanced,

   BXVD_P_ChannelType_eMax
} BXVD_P_ChannelType;

/* The internal structure for the Channel handle */
typedef struct BXVD_P_Channel
{
  uint32_t                 ulChannelNum;      /* channel number */
  bool                     bPreserveState;    /* Do not reset PVR context */
  bool                     bPreserveCounters; /* Do not reset decoder counters */

  /* FW info */
  BXVD_P_DecodeFWMemSize   stDecodeFWMemSize;   /* FW Memory sizes allocated */
  BXVD_P_DecodeFWBaseAddrs stDecodeFWBaseAddrs; /* FW Memory base addresses */

#if BXVD_P_FW_HIM_API
  BXVD_P_AVD_Queue         stDeliveryQueue;          /* data associated with the AVD delivery queue */
  BXVD_P_AVD_Queue         stReleaseQueue;           /* data associated with the AVD release queue */
  BXVD_P_HIM_Offsets       stDropCountIndex;         /* Host Interface Memory offsets for the drop count. */
  BXVD_P_HIM_Offsets       stStatusBlockIndex;       /* Host Interface Memory offsets for the AVD status word(s) */
  BXVD_P_HIM_Offsets       stCabacBinFullnessIndex;  /* Host Interface Memory offsets for the Cabac Bin Fullness word(s) */
#else
  unsigned long            ulPicBuf;          /* Picture delivery queue addr */
  unsigned long            ulPicRelBuf;       /* Picture release queue addr */
  unsigned long            ulPicInfoBuf;      /* unused? */
  unsigned long            ulPicInfoRelBuf;   /* offset to Release Queue shadow write offset and drop count. */
#endif

  unsigned long            ulUserDataRelBuf;

  unsigned long            ulAvdStatusBlock;  /* AVD Status block address */

  BXVD_P_UserDataContext   *pUserData;
  BXVD_P_DecoderState      eDecoderState;
  BXVD_P_Context           *pXvd;             /* main handle */
  BXVD_DecodeSettings      sDecodeSettings;
  unsigned long            ulXptCDB_Read;     /* Current Rave CDB_Read register */
  unsigned long            aulXptCDB_Read_Extended[BXVD_NUM_EXT_RAVE_CONTEXT];     /* Current Extended Rave CDB_Read register */
  BXVD_ChannelSettings     sChSettings;
  BAVC_VideoCompressionStd asVideoCmprStdList[BAVC_VideoCompressionStd_eMax];

  BXVD_DisplayInterrupt    eDisplayInterrupt;   /* The FW Display Device interrupt */

  /* Structure to keep all the MVD interrupt callbacks and its params */
  BXVD_P_InterruptCallbackInfo stInterruptCallbackInfo[BXVD_Interrupt_eMaxInterrupts] ;

  bool bStillPictureToRelease; /* True if there's a pending still picture to release */
  uint32_t uiStillDisplayElementOffset; /* Last still picture offset (to be released) */
  bool bDecoderChannelOpened; /* True if decoder channel has been opened */

  BMMA_Heap_Handle hGeneralHeap;  /* General Heap used to allocate structures for this channel */
  BMMA_Heap_Handle hPictureHeap;  /* Picture Buffer Heap used by firmware to store decoded pictures for this channel */
  BMMA_Heap_Handle hPictureHeap1; /* Picture Buffer Heap used by firmware to store decoded pictures for this channel */
  BMMA_Heap_Handle hCabacHeap;    /* Cabac Buffer Heap used by firmware as CABAC bin buffer for this channel */

  /* General Memory for this channel */
  BMMA_Block_Handle   hFWGenMemBlock;
  unsigned long       uiFWGenMemBaseVirtAddr;
  BXVD_P_PHY_ADDR     FWGenMemBasePhyAddr;

#if !BXVD_P_FW_HIM_API
  uint32_t            uiFWGenMemBaseUncachedVirtAddr;
#endif
  /* Picture Memory for this channel */
  BMMA_Block_Handle   hFWPicMemBlock;
  uint32_t            uiFWPicOffset;
  BXVD_P_PHY_ADDR     FWPicMemBasePhyAddr;
  unsigned long       uiFWPicMemBaseVirtAddr;

  BMMA_Block_Handle   hFWPicMem1Block;
  uint32_t            uiFWPicOffset1;
  BXVD_P_PHY_ADDR     FWPicMem1BasePhyAddr;
  unsigned long       uiFWPicMem1BaseVirtAddr;

  BMMA_Block_Handle   hFWPicChromaMemBlock;
  BXVD_P_PHY_ADDR     FWPicChromaBasePhyAddr;

  /* Cabac Memory for this channel */
  BMMA_Block_Handle   hFWCabacMemBlock;
  BXVD_P_PHY_ADDR     FWCabacMemBasePhyAddr;

  BXVD_P_HandleType eHandleType;

   /* TODO: Move back to stChannelState? */
  BXVD_P_CallBackRequests  stCallbackReq;
  BXVD_P_BufferConfig      stChBufferConfig;
  BXDM_PictureProvider_ChannelChangeSettings stSavedChannelChangeSettings;
  BXVD_SkipMode eCurrentSkipMode;

  BXDM_PictureProvider_Handle hPictureProvider;

  BXVD_P_Decoder_Context stDecoderContext;

  BXDM_Picture_Rate stSavedPlaybackRate;
  bool bSavedPlaybackRateValid;

  /* SW7400-2870: paused by BXVD_PVR_EnablePause.  As opposed to being paused by
   * calling BXVD_SetPlaybackRate(_isr) with a numerator of '0'.
   */
  bool bPauseActive;

  bool bProgressiveStream_7411;
  BXVD_PictureParameterInfo stPictureParameterInfo;

  uint32_t uiPPBSerialNumber;
  BXVD_StillContentInterpolationMode eSavedSPIM;

  /* SW7425-1064: Support for the XMO MergeOmatic filter. */

  BXDM_PictureProvider_XMO_Handle hXmo;   /* Handle for the XMO filter. */

  BXVD_ChannelHandle hXvdChLinked;        /* Used to link two channels together. */
  BXVD_P_ChannelType eChannelType;        /* eStandard, eBase or eEnhanced,*/

#if BXVD_P_USE_TWO_DECODERS
  /* SW7425-1064: when using two decoders with the XMO, both decoders need to be
   * driven off the same interrupts.  The interrupts settings for the second decoder
   * will be saved in BXVD_StartDecode, and then restored in BXVD_StopDecode.
   */
  BXVD_DisplayInterruptProvider_P_InterruptSettings stEnhancedInterruptConfig;
#endif

  BLST_S_ENTRY(BXVD_P_Channel) link; /* Used to keep list of available channel structures */

   /* SW7445-2757: to aid in debug, create a unique serial number
    * each time a channel is opened. */
   uint32_t uiSerialNumber;

   /* SW7425-2686: contains the settings for multi-pass DQT */
   BXVD_TrickModeSettings stTrickModeSettings;

   /* Need the size of the source stream to set the clock boost mode.
    * eSourceResolutionInBand is from the firmware.
    * eSourceResolutionOutOfBand is from the middleware. */

   BXVD_SourceResolution eSourceResolutionInBand;
   BXVD_SourceResolution eSourceResolutionOutOfBand;

#if BXVD_DM_ENABLE_PPB_GRAB_MODE
   FILE* fCapturePPB;
#endif
   union {
       struct {
            BXVD_Decoder_P_UnifiedPictureContext  stUnifiedContext;
       } BXVD_Decoder_S_DeliveryQ_DropPicture_isr;
   } functionData;

   /* ARBVN-74: average out the AQP values. */
   struct
   {
      unsigned long uiPreviousPreviousInput;
      unsigned long uiPreviousInput;
      unsigned long uiPreviousOutput;
   } aqp;

} BXVD_P_Channel;

typedef enum BXVD_P_VideoAtomIndex
{
   BXVD_P_VideoAtomIndex_eA = 0,
   BXVD_P_VideoAtomIndex_eB,
   BXVD_P_VideoAtomIndex_eC,
   BXVD_P_VideoAtomIndex_eD,
   BXVD_P_VideoAtomIndex_eE,
   BXVD_P_VideoAtomIndex_eF,
   BXVD_P_VideoAtomIndex_eG,
   BXVD_P_VideoAtomIndex_eH,
   BXVD_P_VideoAtomIndex_eI,
   BXVD_P_VideoAtomIndex_eAT,
   BXVD_P_VideoAtomIndex_eJ,
   BXVD_P_VideoAtomIndex_eK,
   BXVD_P_VideoAtomIndex_eM,

   /* Add more enums ABOVE this line */
   BXVD_P_VideoAtomIndex_eMax
} BXVD_P_VideoAtomIndex;

typedef struct BXVD_P_FWMemConfig_V2
{
      unsigned long general_memory_size;
      unsigned long inner_loop_wl_size;
      unsigned long direct_mode_size;
      unsigned long cabac_bin_size;
      unsigned long cabac_wl_size;
      BXVD_P_VideoAtomIndex video_block_size_index;
      unsigned long video_block_count;
} BXVD_P_FWMemConfig_V2;

typedef struct BXVD_P_FWMemConfig_SVC
{
      unsigned long context_memory_size;
      unsigned long inner_loop_wl_size;
      unsigned long direct_mode_size;
      unsigned long inter_layer_video_size_index;
      unsigned long inter_layer_mv_size;
      unsigned long cabac_bin_size;
      unsigned long cabac_wl_size;
      BXVD_P_VideoAtomIndex video_block_size_index;
      unsigned long video_block_count;
} BXVD_P_FWMemConfig_SVC;

/***********************************************************************
 * Private functions
 ***********************************************************************/

void BXVD_P_GetVidCmprCapability
(
 BXVD_VidComprStd_Capabilities *pCodecCapabilities,
 BAVC_VideoCompressionStd  eVideoCmprStd
);

BERR_Code BXVD_P_SetupFWSubHeap(BXVD_Handle hXvd);
BERR_Code BXVD_P_TeardownFWSubHeap(BXVD_Handle hXvd);

BERR_Code BXVD_P_GetDecodeFWMemSize
(
 BXVD_Handle hXvd,
 BXVD_DecodeResolution eDecodeResolution,
 BAVC_VideoCompressionStd aeVideoCmprStd[],
 uint32_t   uiVideoCmprCount,
 const BXVD_ChannelSettings *pChSettings,
 BXVD_P_DecodeFWMemSize       *pstDecodeFWMemSize
);

BERR_Code BXVD_P_GetStillDecodeFWMemSize
(
 BXVD_Handle hXvd,
 BXVD_DecodeResolution eDecodeResolution,
 BAVC_VideoCompressionStd aeVideoCmprStd[],
 uint32_t   uiVideoCmprCount,
 const BXVD_ChannelSettings *pChSettings,
 BXVD_P_DecodeFWMemSize       *pstDecodeFWMemSize
);

BERR_Code BXVD_P_AllocateFWMem
(
  BXVD_Handle hXvd,
  BXVD_ChannelHandle hXvdCh,
  BXVD_P_DecodeFWMemSize *pstDecodeFWMemSize,
  BXVD_P_DecodeFWBaseAddrs *pstDecodeFWBaseAddrs
);

void BXVD_P_DetermineChromaBufferBase
(
  BXVD_ChannelHandle hXvdCh,
  BXVD_P_PPB_Protocol eProtocol
);

BERR_Code BXVD_P_FreeFWMem
(
  BXVD_Handle hXvd,
  BXVD_ChannelHandle hXvdCh,
  BXVD_P_DecodeFWBaseAddrs *pstDecodeFWBaseAddrs
);

void BXVD_P_ValidateHeaps
(
  BXVD_Handle        hXvd,
  BXVD_P_MemCfgMode  eMemCfgMode
);

BERR_Code BXVD_P_Boot
(
  BXVD_Handle hXvd
);

BERR_Code BXVD_P_InitDecoderFW
(
   BXVD_Handle hXvd
);

BERR_Code BXVD_P_OpenPartTwo
(
   BXVD_Handle hXvd
);

BERR_Code BXVD_P_RestartDecoder
(
   BXVD_Handle hXvd
);

BERR_Code BXVD_P_SetupStillPictureCompatibilityMode
(
   BXVD_Handle hXvd
);

BERR_Code BXVD_P_TeardownStillPictureCompatibilityMode
(
   BXVD_Handle hXvd
);

/* AVD DEVICE COMMANDS */
BERR_Code BXVD_P_HostCmdSendInit
(
 BXVD_Handle  hXvd,                /* XVD context */
 uint32_t     uDecoderInstance,    /* AVC instance */
 uint32_t     eRaveEndianess       /* Endianess of data in Rave CDB/ITB */
);

BERR_Code BXVD_P_HostCmdSendConfig
(
 BXVD_Handle  hXvd,               /* XVD context */
 uint32_t     uiVecIndex,         /* Display device index */
 uint32_t     uiInterruptMask_0,  /* RUL done mask for specified display */
 uint32_t     uiInterruptMask_1   /* RUL done mask for specified display */
);

BERR_Code BXVD_P_HostCmdSendDecChannelOpen
(
 BXVD_Handle         hXvd,
 BXVD_ChannelHandle  hXvdCh,
 bool                bStillPictureMode,
 uint32_t            uiMaxResolution,
 BXVD_P_DecodeFWMemSize  *pstDecodeFWMemSize,
 BXVD_P_DecodeFWBaseAddrs *pstDecodeFWBaseAddrs
);

BERR_Code BXVD_P_HostCmdSendDecChannelClose
(
 BXVD_Handle         hXvd,
 BXVD_ChannelHandle  hXvdCh
);

BERR_Code BXVD_P_HostCmdSendDecChannelStart
(
  BXVD_Handle         hXvd,
  uint32_t            ulChannelNumber,
  uint32_t            eProtocol,
  uint32_t            eChannelMode,
  uint32_t            ulRaveContextBase,
  uint32_t            aulRaveContextBaseExt[],
  uint32_t            ulVecIndex
);


BERR_Code BXVD_P_HostCmdSendDecChannelStop
(
   BXVD_Handle        hXvd,
   uint32_t           ulChannelNum
);


/***************************************************************************
Summary:
    Set the skip picture mode.

Description:
    This API instructs the FW to skip I, IP or no frames

Returns:
    BERR_SUCCESS

See Also:
    None

****************************************************************************/
BERR_Code BXVD_P_HostCmdSetSkipPictureMode
(
 BXVD_Handle    hXvd,
 uint32_t       ulChannelNum,
 BXVD_SkipMode  eSkipMode
);


/***************************************************************************
 *  Summary
 *
 *     BXVD_P_HostCmdDbgLogControl: Send debug log control command
 *
 *  Description
 *
 *     The decoders outer loop debug output can be logged to memory for XVD
 *     to read and pass to the application. This routines starts and stops
 *     the logging of the debug data to the log buffer.
 *
 ***************************************************************************/
BERR_Code BXVD_P_HostCmdDbgLogControl
(
   BXVD_Handle    hXvd,
   bool           logStart
);

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
);

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
   unsigned int   uiCAS,
   unsigned int   uiIntraPenality,
   unsigned int   uiPostPenality
);

BERR_Code BXVD_P_ChipInit(BXVD_Handle hXvd,
                          uint32_t uDecoderInstance);

BERR_Code BXVD_P_Reset740x(BXVD_Handle hXvd,
                           uint32_t uDecoderInstance);

BERR_Code BXVD_P_InitChannel(BXVD_ChannelHandle  hXvdCh);

BERR_Code BXVD_P_SetupInterrupts( BXVD_Handle hXvd);
BERR_Code BXVD_P_CreateInterrupts( BXVD_Handle hXvd);
BERR_Code BXVD_P_DisableInterrupts( BXVD_Handle hXvd);
BERR_Code BXVD_P_EnableInterrupts( BXVD_Handle hXvd);
BERR_Code BXVD_P_DestroyInterrupts(BXVD_Handle hXvd);

BERR_Code BXVD_P_SetupWatchdog( BXVD_Handle hXvd);
BERR_Code BXVD_P_EnableWatchdog( BXVD_Handle hXvd);
BERR_Code BXVD_P_DisableWatchdog( BXVD_Handle hXvd);

void BXVD_P_FreeXVDContext(BXVD_Handle hXvd);

BERR_Code BXVD_P_InitFreeChannelList(BXVD_Handle hXvd);

void BXVD_P_GetChannelHandle(BXVD_Handle hXvd,
                             BXVD_ChannelHandle *phXvdCh);

void BXVD_P_KeepChannelHandle(BXVD_Handle hXvd,
                              BXVD_ChannelHandle hXvdCh);

void BXVD_P_FreeAllocatedChannelHandles(BXVD_Handle hXvd);

void BXVD_P_SetClockBoost(BXVD_Handle hXvd);

#if (BXVD_P_POWER_MANAGEMENT)

void BXVD_P_GetHibernateState(BXVD_Handle hXvd,
                              bool *bHibernateState);

void BXVD_P_SetHibernateState(BXVD_Handle hXvd,
                              bool bHibernateState);

#endif


bool BXVD_P_IsDecodeProtocolSupported(BXVD_Handle               hXvd,
                                      BAVC_VideoCompressionStd  eVideoCmprStd );

BERR_Code BXVD_P_MapToAVDProtocolEnum( BXVD_Handle               hXvd,
                                       BAVC_VideoCompressionStd  eVideoCmprStd,
                                       BXVD_P_PPB_Protocol *     peProtocol );

extern const uint32_t BXVD_P_StripeWidthLUT[];

#if BXVD_P_CAPTURE_RVC
#include "bchp_hevd_cmdbus_xmit_0.h"

void BXVD_P_StartRVCCapture(BXVD_Handle hXvd,
                            BXVD_P_PPB_Protocol eProtocol);

void BXVD_P_DumpRvc(BXVD_P_Context *pXvd);
#endif

#ifdef __cplusplus
}
#endif

#endif /* BXVD_PRIV_H__ */

/* End of file. */

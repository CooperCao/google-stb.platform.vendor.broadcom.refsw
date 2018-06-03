/****************************************************************************
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
 ****************************************************************************/

#ifndef _DSP_BM_H_
#define _DSP_BM_H_

#include "libdspcontrol/CHIP.h"

#include "fp_sdk_config.h"


#if IS_HOST(DSP_LESS)
#  error "This module is not suitable for DSP-less builds"
#endif


#ifdef __cplusplus
extern "C"
{
#endif


/*
 * On the BM, the HPI component (simulating an external agent, e.g. a MIPS cpu) sees an unique address space.
 * To permit the co-existence and co-addressing by the HPI of both the FP world (SMEM, DMEM, etc.) and the
 * MIPS world (DDR, etc.), the following layout is used:
 * * SMEM, DMEM, etc. are mapped at the same addresses as they would be seen by the FP core, so usually at
 *   address 0 is located the SMEM (or a ROM, if present), followed by DMEM and so on.
 * * DDR and other components normally not visible from the FP core are "relocated" at higher addresses.
 * The following defined DDR_START macro keeps track of where the DDR start is expected to be mapped by the BM
 * on different platforms.
 * This mechanism needs to be cleaned up a little bit in the future.
 *
 * FIXME: this whole "access the DDR at different addresses from HPI and from inside the BM" should be done better
 */
#if defined(RAAGA) && !defined(__FP4015_ONWARDS__)
#  if IS_HOST(SILICON)
#    define DDR_START 0
#  else
#    define DDR_START 0xD0000000ul
#  endif
#elif defined(DUNA) || defined(MCPHY) || defined(PIKE) || defined(LEAP_PHY) || defined(GENERIC) || defined(YELLOWSTONE) ||\
      (defined(RAAGA) && defined(__FP4015_ONWARDS__))
#  define   DDR_START 0             /* hack, no DDR on this chip */
#endif


#ifdef __cplusplus
}
#endif

#endif /* _DSP_H_ */

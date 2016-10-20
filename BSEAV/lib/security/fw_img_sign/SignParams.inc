##############################################################################
#  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
#
#  This program is the proprietary software of Broadcom and/or its licensors,
#  and may only be used, duplicated, modified or distributed pursuant to the terms and
#  conditions of a separate, written license agreement executed between you and Broadcom
#  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#  no license (express or implied), right to use, or waiver of any kind with respect to the
#  Software, and Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#  ANY LIMITED REMEDY.
##############################################################################

export BOLT_BIN=none

#Generic Parameter Settings
#These settings apply to all signing operations unless overridden by an engine/block specific parameter setting.
export MARKET_ID?=0x0
export MARKET_ID_MASK?=0xFFFFFFFF
export MARKET_ID_SEL?=0x0
export EPOCH_SEL=0x0
export EPOCH=0x0
export EPOCH_MASK=0xFF
export SIG_TYPE=0x4
export SIG_VER=0x1

#Settings specific to AVS Signing.
#AVS image is extracted from the BOLT binary specified by BOLT_BIN variable above.
export AVS_MARKET_ID=0x12345678
export AVS_MARKET_ID_MASK=0xFFFFFFFF
export AVS_EPOCH_SEL=0x0
export AVS_EPOCH=0x1
export AVS_EPOCH_MASK=0xFF
export AVS_SIG_TYPE=0x4
export AVS_SIG_VER=0x1

#Settings Specific to AVS Key2 Signing
export AVS_KEY2_EPOCH=0x1
export AVS_KEY2_SIG_TYPE=0x1
export AVS_KEY2_SIGNING_RIGHT=0x20
export AVS_KEY2_EXPONENT=0x1

#Settings specific to MEMSYS Signing.
#MEMSYS image is extracted from the BOLT binary specified by BOLT_BIN variable above.
export MEMSYS_MARKET_ID=0x12345678
export MEMSYS_MARKET_ID_MASK=0xFFFFFFFF
export MEMSYS_EPOCH_SEL=0x0
export MEMSYS_EPOCH=0x1
export MEMSYS_EPOCH_MASK=0xFF
export MEMSYS_SIG_TYPE=0x4
export MEMSYS_SIG_VER=0x1

#Settings Specific to MEMSYS Key2 Signing
export MEMSYS_KEY2_EPOCH=0x0
export MEMSYS_KEY2_SIG_TYPE=0x1
export MEMSYS_KEY2_SIGNING_RIGHT=0x20
export MEMSYS_KEY2_EXPONENT=0x1

#Settings Specific to Firmware Key2 Signing
export KEY2_MARKET_ID?=0x0
export KEY2_MARKET_ID_SEL?=0x0
export KEY2_EPOCH?=0x0
export KEY2_SIG_VER?=0x1
export KEY2_SIG_TYPE=0x1
export KEY2_SIGNING_RIGHT?=0x2
export KEY2_EXPONENT=0x1

#Settings Specific to RAVE Firmware Signing
export RAVE_EPOCH?=0x0
export RAVE_FW_EPOCH ?= 0x0

#Settings specific to RAAGA Firmware Signing
export RAAGA_EPOCH?=0x0
export RAAGA_FW_EPOCH ?= 0x0

#Settings specific to VCE Firmware Signing
export VCE_EPOCH?=0x0
export VCE_FW_EPOCH ?= 0x0

#Settings specific to AVD Firmware Signing
export AVD_EPOCH?=0x0
export AVD_FW_EPOCH ?= 0x0

#Settings specific to SID Firmware Signing
export SID_EPOCH?=0x0

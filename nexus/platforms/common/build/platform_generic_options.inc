#############################################################
#  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
############################################################

#
# NOTE: Do not add platform-specific logic here. There should be no variation of ifeq ($(NEXUS_PLATFORM),) in this file.
# Platform-specific logic can be added to nexus/platforms/$(NEXUS_PLATFORM)/build/platform_options.inc.
#

ifneq ($(NEXUS_DISABLE_FRONTEND),)
NEXUS_FRONTEND_SUPPORT := n
endif

ifeq ($(shell test -e ${BCHP_RDB_PATH}/bchp_rfm_grb.h && echo y),)
NEXUS_RFM_SUPPORT := n
endif

ifeq ($(NEXUS_HEADLESS_PLATFORM),y)
NEXUS_IR_BLASTER_SUPPORT ?= n
NEXUS_RFM_SUPPORT ?= n
NEXUS_KEYPAD_SUPPORT ?= n
NEXUS_HDMI_OUTPUT_SUPPORT ?= n
NEXUS_HDMI_INPUT_SUPPORT ?= n
NEXUS_HDMI_DVO_SUPPORT ?= n
NEXUS_CEC_SUPPORT ?= n
endif

ifeq ($(shell test -e ${BCHP_RDB_PATH}/bchp_uhfr.h && echo y),)
NEXUS_UHF_INPUT_SUPPORT := n
endif
ifeq ($(shell test -e ${BCHP_RDB_PATH}/bchp_dvp_hr.h && echo y),)
NEXUS_HDMI_INPUT_SUPPORT := n
endif

ifneq ($(filter $(B_REFSW_OS),linuxkernel bare),$(B_REFSW_OS))
NEXUS_USERMODE := y
endif
ifneq ($(filter $(B_REFSW_OS),linuxuser linuxkernel ucos bare),)
# Include uart for non-linux systems only.
ifeq ($(NEXUS_UART_SUPPORT),)
NEXUS_UART_SUPPORT := n
endif
endif

ifeq ($(shell test -e ${BCHP_RDB_PATH}/bchp_sid.h && echo y),)
NEXUS_PICTURE_DECODER_SUPPORT := n
endif

ifneq ($(NEXUS_PICTURE_DECODER_SUPPORT),n)
BSID_MJPEG_SUPPORT := y
endif

ifeq ($(shell test -e ${BCHP_RDB_PATH}/bchp_scpu_top_ctrl.h && echo y),y)
ifneq ($(findstring $(BCHP_CHIP),7435 7584 75845 74371),$(BCHP_CHIP))
SAGE_SUPPORT ?= y
endif
endif

ifeq ($(shell test -e ${BCHP_RDB_PATH}/bchp_xpt_mcpb.h && echo y),y)
NEXUS_DMA_SUPPORT := n
NEXUS_PLATFORM_DEFINES += NEXUS_HAS_XPT_DMA
endif

ifeq ($(PVR_SUPPORT),n)
NEXUS_PLAYBACK_SUPPORT := n
endif
ifneq ($(NEXUS_USERMODE),y)
NEXUS_FILE_SUPPORT := n
NEXUS_PLAYBACK_SUPPORT := n
NEXUS_RECORD_SUPPORT := n
endif

ifeq ($(BAPE_DSP_SUPPORT),n)
BDSP_ENCODER_SUPPORT := n
endif

ifeq ($(BDSP_ENCODER_SUPPORT),y)
VIDEO_ENCODER_SUPPORT := y
endif
ifneq (${VIDEO_ENCODER_SUPPORT},y)
NEXUS_VIDEO_ENCODER_SUPPORT := n
NEXUS_STREAM_MUX_SUPPORT := n
NEXUS_FILE_MUX_SUPPORT := n
endif
ifneq ($(NEXUS_USERMODE),y)
NEXUS_FILE_MUX_SUPPORT := n
endif

B_REFSW_STRICT_PRINTF_FORMAT ?= y

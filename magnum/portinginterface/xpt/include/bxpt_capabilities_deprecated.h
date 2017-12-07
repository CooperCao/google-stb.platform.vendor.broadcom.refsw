/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ***************************************************************************/

#ifndef BXPT_CAPABILITIES_DEPRECATED_H__
#define BXPT_CAPABILITIES_DEPRECATED_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
** THE DEFINES IN THIS FILE ARE SUBJECT TO FUTURE REMOVAL / REFACTOR
** They define capabilities that all 28nm platforms possess.
** Usage of these defines is strongly discouraged. If there's a valid usage case,
** the define should be moved out of this file
**/

#if (defined BXPT_IS_CORE28NM) && (!defined BXPT_FOR_BOOTUPDATER)

/* defines that have a BXPT_NUM_XYZ counterpart */
#ifdef BCHP_XPT_MSG_REG_START
#define BXPT_HAS_MESG_BUFFERS 1
#endif
#ifdef BCHP_XPT_PSUB_REG_START
#define BXPT_HAS_PACKETSUB    1 /* defined for ALL platforms */
#endif
#define BXPT_HAS_RAVE         1
#define BXPT_HAS_MTSIF        1
#ifdef BCHP_XPT_RMX0_REG_START
#define BXPT_HAS_REMUX        1
#endif
#define BXPT_HAS_DPCRS        1 /* TODO: is this equal to BXPT_NUM_PCRS? BXPT_HAS_DPCRS defined for ALL platforms */

/*
[no references outside of src/core65nm/]
=> not needed for 28nm and can simply be removed
*/
#define BXPT_PLAYBACK_TS_ERROR_BOUNDS       1
#define BXPT_REC_SCD_MAPPING                1
#define BXPT_NEXT_GEN_COMPARATORS           1
#define BXPT_PER_CONTEXT_PIC_COUNTER        1
#define BXPT_HAS_RAVE_PES_EXTENSIONS        1

/*
[no references outside of src/core65nm/ and include/]
=> needed for 28nm to get the API compiled-in
=> in future, remove references from include/
*/
#define BXPT_HAS_RMX_NULL_THRESHOLD         1
#define BXPT_SEPARATE_REMUX_IO              1
#define BXPT_HAS_RAVE_SCRAMBLING_CONTROL    1
#define BXPT_HAS_FIXED_PSUB_DMA_PRIORITY    1
#define BXPT_HAS_PARALLEL_MPOD              1
#define BXPT_HAS_PID2BUF_MAPPING            1

/*
[no references outside of include/]
=> needed for 28nm to get the API compiled-in
=> in future, remove references from include/ and wrap any src/ as necessary
*/
#define BXPT_HAS_TIMER_TICK                 1
#define BXPT_HAS_EMM_RAVE                   1
#define BXPT_HAS_MPOD_OUTPUT_CLOCK_RATE     1
#define BXPT_HAS_REDUCED_TIMEREFS           1

/* confusing and preferably removed */
#define BXPT_HAS_IB_PID_PARSERS             1 /* defined for ALL platforms */
#define BXPT_HAS_PLAYBACK_PARSERS           1

/* more complicated cases */
#define BXPT_HAS_FULL_PID_PARSER            1
#define BXPT_HAS_PARSER_REMAPPING           1

/* has references in nexus but defined for ALL platforms */
#define BXPT_HAS_PID_CHANNEL_PES_FILTERING  1
#define BXPT_HAS_MOSAIC_SUPPORT             1

/* related to BXPT_MAX_ES_COUNT:

BXPT_PTS_CONTROL_IN_REC_REG, BXPT_HAS_STARTCODE_BUFFER_WORKAROUND, BXPT_HAS_EXTENDED_ES_COUNT and BXPT_HAS_16BYTE_ES_COUNT were very confusing

for 28nm chips,
BXPT_HAS_16BYTE_ES_COUNT should take precedence (defined below)
BXPT_HAS_EXTENDED_ES_COUNT is unused
BXPT_PTS_CONTROL_IN_REC_REG is needed to define BXPT_MAX_ES_COUNT to 9 for platforms without BXPT_HAS_16BYTE_ES_COUNT
BXPT_HAS_STARTCODE_BUFFER_WORKAROUND is needed due to reference in nexus
*/
#define BXPT_HAS_STARTCODE_BUFFER_WORKAROUND 1
#define BXPT_PTS_CONTROL_IN_REC_REG          1


#endif /* BXPT_IS_CORE28NM */

#ifdef __cplusplus
}
#endif

#endif

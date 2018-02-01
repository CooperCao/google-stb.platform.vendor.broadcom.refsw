/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/

#ifndef BXPT_CAPABILITIES_H__
#define BXPT_CAPABILITIES_H__

#include "bchp.h"
#include "bchp_common.h"
#include "bxpt_capabilities_deprecated.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (defined BXPT_IS_CORE28NM) && (!defined BXPT_FOR_BOOTUPDATER)

/* Bug fix for 7278Bx, 7255, and subsequent chips. See SWSTB-1525 */
#if (BCHP_CHIP == 7278 && BCHP_VER == BCHP_VER_B0) || \
    (BCHP_CHIP == 7278 && BCHP_VER == BCHP_VER_B1) || \
    (BCHP_CHIP == 7255)
   #define BXPT_P_JITTER_CORRECTION_FIX     1
   #define BXPT_P_PCR_OFFSET_JITTER_FIX     1
   #define BXPT_P_REMUX_JITTER_FIX          1
#endif

/* remux */
#if (BCHP_CHIP==7255)
#define BXPT_NUM_REMULTIPLEXORS             0
#else
#define BXPT_NUM_REMULTIPLEXORS             2
#endif
#define BXPT_HAS_REMUX_PAUSE                1
#define BXPT_HAS_REMUX_PCR_OFFSET           1

/* rave */
#define BXPT_NUM_RAVE_CHANNELS              1
#if ( (BCHP_CHIP == 7271) || (BCHP_CHIP == 7268) || (BCHP_CHIP == 7260))
    #define BXPT_NUM_SCD                        32
    #define BXPT_NUM_TPIT_PIDS                  32
    #define BXPT_NUM_TPIT                       8
#elif (BCHP_CHIP == 7255)
    #define BXPT_NUM_SCD                        24
    #define BXPT_NUM_TPIT_PIDS                  16
    #define BXPT_NUM_TPIT                       8
#else
    #define BXPT_NUM_SCD                        64
    #define BXPT_NUM_TPIT_PIDS                  32
    #define BXPT_NUM_TPIT                       16
#endif
#define BXPT_HAS_SVC_MVC                    1
#define BXPT_HAS_BPP_SEARCH                 1
#define BXPT_HAS_ATS                        1
#define BXPT_HAS_LOCAL_ATS                  1

/* playback */
#if BCHP_CHIP == 7255
    #define BXPT_NUM_PLAYBACKS                  16
#else
    #define BXPT_NUM_PLAYBACKS                  32
#endif
#define BXPT_HAS_MULTICHANNEL_PLAYBACK      1
#if (BCHP_CHIP == 7278)
    #define BXPT_USE_HOST_AGGREGATOR            1
    #define BXPT_HAS_MCPB_VER_3              1
#else
    #define BXPT_HAS_SEPARATE_OOS_INTR       1
#endif
#define BXPT_HAS_32BIT_PB_TIMESTAMPS        1
#define BXPT_HAS_PCR_PACING                 1

/* pcr */
#if ( (BCHP_CHIP == 7271) || (BCHP_CHIP == 7268) || (BCHP_CHIP == 7260) )
    #define BXPT_NUM_PCRS                       6
#elif (BCHP_CHIP == 7255)
    #define BXPT_NUM_PCRS                       2
#else
    #define BXPT_NUM_PCRS                       14
#endif
#define BXPT_NUM_PCR_OFFSET_CHANNELS        16
#define BXPT_DPCR_GLOBAL_PACKET_PROC_CTRL   1
#define STC_FREE_RUNNING                    15 /* TODO: add BXPT_ prefix or privatize */

/* psub */
#if ( (BCHP_CHIP == 7271) || (BCHP_CHIP == 7268) || (BCHP_CHIP == 7260) )
    #define BXPT_NUM_PACKETSUBS                 8
#elif (BCHP_CHIP == 7255)
    #define BXPT_NUM_PACKETSUBS                 0
#else
    #define BXPT_NUM_PACKETSUBS                 16
#endif
#define BXPT_HAS_PACKETSUB_FORCED_INSERTION 1
#define BXPT_HAS_PB_PACKETSUB               1
#if (BCHP_CHIP == 7278)
   #define BXPT_PSUB_40BIT_SUPPORT           1
   #define BXPT_PACKETSUB_REQUIRES_FORCED_INSERTION 1
#endif

/* mpod */
#define BXPT_HAS_MPOD_SCARD_SUPPORT         1
#define BXPT_HAS_MPOD_RSBUF                 1
#if (BCHP_CHIP == 7278)
   #define BXPT_P_TSIO_MPOD_USE_SHARED_RSBUF  1
#endif

/* power management */
#define BXPT_HAS_WAKEUP_PKT_SUPPORT         1

/* misc */
#define BXPT_P_INITIAL_BUF_BITRATE          (30000000) /* in Mbps */
#define BXPT_HAS_PIPELINE_ERROR_REPORTING   1
#define BXPT_HAS_AVS                        1

/* tsmux */
#define BXPT_HAS_TSMUX                      1
#define BXPT_NUM_PACING_COUNTERS            32

#define BXPT_HAS_FIXED_RSBUF_CONFIG         1
#define BXPT_HAS_FIXED_XCBUF_CONFIG         1

#define BXPT_HAS_DIRECTV_SUPPORT            1
#define BXPT_NUM_CAP_FILTERS                5

/* mesg */
#if ( (BCHP_CHIP == 7271) || (BCHP_CHIP == 7268) || (BCHP_CHIP == 7260))
    #define BXPT_NUM_MESG_BUFFERS                 128
    #define BXPT_NUM_MESSAGE_CAPABLE_PID_CHANNELS 384
#elif (BCHP_CHIP == 7255)
    #define BXPT_NUM_MESG_BUFFERS                 0
    #define BXPT_NUM_MESSAGE_CAPABLE_PID_CHANNELS 0
#else
    #define BXPT_NUM_MESG_BUFFERS                 256
    #define BXPT_NUM_MESSAGE_CAPABLE_PID_CHANNELS 768
#endif
#define BXPT_NUM_SAM_PID_CHANNEL              BXPT_NUM_MESG_BUFFERS

/*
The number of filter banks depends on the size of the coefficient array. Note that the mask and exclusion array are
the same size as the coef array.

Each entry in the coef array is 1 word or 4 bytes. Each byte can hold bits that the hw can filter a message against.
These 4 byte / 1 word filters can be cascaded to form a larger filter. In our default configuration, we cascade 4 words
to create 16-byte filters. So, the number of 16-byte filters is the size of the coefficient array divided by 4, since
4 array entries are needed to get 16 bytes of filtering.

At this point, it's necessary to note that some customers require larger, 32-byte filters. This is supported by the
BXPT_FILTER_32 define, which is set in the build system. It just changes the number of 4-byte filters that are cascaded.

Each message filter can support up to 32 filters at one time, and all such filters must come from the same bank. So, the
number of banks available is the number of 16-byte filters divided by 32.

In the math below, the arrays are 0-based.
*/
#ifdef BCHP_XPT_MSG_REG_START
    #include "bchp_xpt_msg.h"
    #ifdef BXPT_FILTER_32
        #define BXPT_NUM_FILTER_BANKS   (((BCHP_XPT_MSG_GEN_FILT_COEF_i_ARRAY_END - BCHP_XPT_MSG_GEN_FILT_COEF_i_ARRAY_START + 1) / 8) / 32)
    #else
        #define BXPT_NUM_FILTER_BANKS   (((BCHP_XPT_MSG_GEN_FILT_COEF_i_ARRAY_END - BCHP_XPT_MSG_GEN_FILT_COEF_i_ARRAY_START + 1) / 4) / 32)
    #endif
    #define BXPT_NUM_FILTERS_PER_BANK             32
    #define BXPT_P_FILTER_TABLE_SIZE              BXPT_NUM_FILTERS_PER_BANK
#else
    #define BXPT_NUM_FILTER_BANKS 0
    #define BXPT_NUM_FILTERS_PER_BANK             0
    #define BXPT_P_FILTER_TABLE_SIZE              BXPT_NUM_FILTERS_PER_BANK
#endif

/* frontend */
#define BXPT_NUM_REMAPPABLE_FE_PARSERS       16
#if ( (BCHP_CHIP == 7271) || (BCHP_CHIP == 7268) || (BCHP_CHIP == 7260) )
    #define BXPT_NUM_PID_PARSERS                 16
    #define BXPT_P_PID_TABLE_SIZE                512
    #define BXPT_NUM_PID_CHANNELS                384    /* Leave 128 for MEMDMA */
#elif (BCHP_CHIP == 7255)
    #define BXPT_NUM_PID_PARSERS                 4
    #define BXPT_P_PID_TABLE_SIZE                256
    #define BXPT_NUM_PID_CHANNELS                192    /* Leave 64 for MEMDMA */
#else
    #define BXPT_NUM_PID_PARSERS                 24
    #define BXPT_P_PID_TABLE_SIZE                1024
    #define BXPT_NUM_PID_CHANNELS                768
#endif
#define BXPT_P_HAS_SPID_EXTENSION_TABLE      1

/* MEMDMA */
#define BXPT_HAS_MEMDMA                      1
#define BXPT_NUM_MEMDMA_PID_CHANNELS         (BXPT_P_PID_TABLE_SIZE - BXPT_NUM_PID_CHANNELS)
#define BXPT_P_MEMDMA_PID_CHANNEL_START      BXPT_NUM_PID_CHANNELS
#if (BCHP_CHIP == 7278 && BCHP_VER == BCHP_VER_B0) || \
    (BCHP_CHIP == 7278 && BCHP_VER == BCHP_VER_B1) || \
    (BCHP_CHIP == 7255)

/* Some chips do not have the performance monitor regs */
#else
#define BXPT_DMA_HAS_PERFORMANCE_METER  1
#endif

#if ( (BCHP_CHIP==7445 && BCHP_VER<BCHP_VER_D0) || (BCHP_CHIP==7439 && BCHP_VER<BCHP_VER_B0) ||\
        (BCHP_CHIP==7145 && BCHP_VER<BCHP_VER_B0) || (BCHP_CHIP==7366 && BCHP_VER<BCHP_VER_B0) || \
        (BCHP_CHIP==74371 && BCHP_VER<BCHP_VER_B0) )
#define BXPT_NUM_RAVE_CONTEXTS              48
#define BXPT_NUM_MTSIF                  2
#define BXPT_NUM_STCS                   10
#define BXPT_P_HAS_0_238_PPM_RESOLUTION 0
#define BXPT_MAX_EXTERNAL_TRIGS         4
#define BXPT_HAS_STC_TRIG_TYPE          0
#define BXPT_HAS_16BYTE_ES_COUNT        0
#define BXPT_NUM_TBG                    0
#elif ( (BCHP_CHIP == 7271) || (BCHP_CHIP == 7268) || (BCHP_CHIP == 7260) )
#define BXPT_NUM_RAVE_CONTEXTS              24
#define BXPT_NUM_MTSIF                  4
#if ((BCHP_CHIP == 7271 || BCHP_CHIP == 7268) && BCHP_VER >= BCHP_VER_B0)
    #define BXPT_NUM_STCS                   8
#else
    #define BXPT_NUM_STCS                   6
#endif
#define BXPT_P_HAS_0_238_PPM_RESOLUTION 1
#define BXPT_MAX_EXTERNAL_TRIGS         6
#define BXPT_HAS_STC_TRIG_TYPE          1
#define BXPT_HAS_16BYTE_ES_COUNT        1
#define BXPT_NUM_TBG                    1

#elif (BCHP_CHIP == 7255)
#define BXPT_NUM_RAVE_CONTEXTS         16
#define BXPT_NUM_MTSIF                  1
#define BXPT_NUM_STCS                   4
#define BXPT_P_HAS_0_238_PPM_RESOLUTION 1
#define BXPT_MAX_EXTERNAL_TRIGS         6
#define BXPT_HAS_STC_TRIG_TYPE          1
#define BXPT_HAS_16BYTE_ES_COUNT        1
#define BXPT_NUM_TBG                    0

#elif (BCHP_CHIP == 7278)
#define BXPT_NUM_RAVE_CONTEXTS          48
    #if BCHP_VER == BCHP_VER_A0
        #define BXPT_NUM_MTSIF                  4
    #else
        #define BXPT_NUM_MTSIF                  2
    #endif
#define BXPT_NUM_STCS                   16
#define BXPT_P_HAS_0_238_PPM_RESOLUTION 1
#define BXPT_MAX_EXTERNAL_TRIGS         6
#define BXPT_HAS_STC_TRIG_TYPE          1
#define BXPT_HAS_16BYTE_ES_COUNT        1
#define BXPT_NUM_TBG                    8
#else
#define BXPT_NUM_RAVE_CONTEXTS              48
#define BXPT_NUM_MTSIF                  4
#define BXPT_NUM_STCS                   12
#define BXPT_P_HAS_0_238_PPM_RESOLUTION 1
#define BXPT_MAX_EXTERNAL_TRIGS         6
#define BXPT_HAS_STC_TRIG_TYPE          1
#define BXPT_HAS_16BYTE_ES_COUNT        1
#define BXPT_NUM_TBG                    8
#endif

#if ( !((BCHP_CHIP==7445 && BCHP_VER<BCHP_VER_C0) || (BCHP_CHIP==7145 && BCHP_VER<BCHP_VER_B0)) )
#define BXPT_HAS_RAVE_MIN_DEPTH_INTR        1
#if (BCHP_CHIP==7255)
#define BXPT_HAS_MESG_L2                    0
#else
#define BXPT_HAS_MESG_L2                    1
#endif
#define BXPT_HAS_RAVE_L2                    1
#endif

#if(BCHP_CHIP == 7278 && BCHP_VER == BCHP_VER_B0) || \
   (BCHP_CHIP == 7278 && BCHP_VER == BCHP_VER_B1)
#define BXPT_HAS_PACKET_PLACEHOLDER 1
#endif

#if ( (BCHP_CHIP==7445 && BCHP_VER<BCHP_VER_C0) || (BCHP_CHIP==7439 && BCHP_VER<BCHP_VER_B0) || (BCHP_CHIP==74371 && BCHP_VER<BCHP_VER_B0) || (BCHP_CHIP==7366 && BCHP_VER<BCHP_VER_B0) )
#define BXPT_NUM_INPUT_BANDS            11
#elif (BCHP_CHIP==7445 && BCHP_VER<BCHP_VER_D0)
#define BXPT_NUM_INPUT_BANDS            3
#elif (BCHP_CHIP==7445) || (BCHP_CHIP == 7121)
#define BXPT_NUM_INPUT_BANDS            6
#elif (BCHP_CHIP==7145 && BCHP_VER<BCHP_VER_B0)
#define BXPT_NUM_INPUT_BANDS            20
#elif (BCHP_CHIP==7255)
#define BXPT_NUM_INPUT_BANDS            6
#else
#define BXPT_NUM_INPUT_BANDS            13
#endif

#if (BCHP_CHIP == 7278)
#define BXPT_TSMF_P_MAX_TSMF 24
#define BXPT_TSMF_P_MAX_TSMF_BONDING_GROUPS 8
#define BXPT_TSMF_FIXED_PARSER_MAPPING  1
#else
#define BXPT_TSMF_P_MAX_TSMF 2
#endif

#if ( (BCHP_CHIP==7445 && BCHP_VER<BCHP_VER_C0) || (BCHP_CHIP==7439 && BCHP_VER<BCHP_VER_B0) ||\
      (BCHP_CHIP==7145 && BCHP_VER<BCHP_VER_B0) || (BCHP_CHIP==7366 && BCHP_VER<BCHP_VER_B0) ||\
      (BCHP_CHIP==7250 && BCHP_VER<BCHP_VER_B0) || (BCHP_CHIP==74371 && BCHP_VER<BCHP_VER_B0) )
#define BXPT_NUM_TSIO                   0
#else
#define BXPT_NUM_TSIO                   1
#endif

#define BXPT_NUM_LEGACY_PID_PARSERS     16

#if ( (BCHP_CHIP==7364) || (BCHP_CHIP==7250) )
#define BXPT_NUM_STC_SNAPSHOTS          12
#define BXPT_HAS_STC_SNAPSHOT_XBAR 1
#elif ( (BCHP_CHIP == 7271) || (BCHP_CHIP == 7268) || (BCHP_CHIP == 7260) )
    #define BXPT_NUM_STC_SNAPSHOTS          8
    #define BXPT_HAS_STC_SNAPSHOT_XBAR      1
#elif (BCHP_CHIP == 7255)
    #define BXPT_NUM_STC_SNAPSHOTS          4
    #define BXPT_HAS_STC_SNAPSHOT_XBAR      1
#elif (BCHP_CHIP == 7278)
    #define BXPT_NUM_STC_SNAPSHOTS          16
    #define BXPT_HAS_STC_SNAPSHOT_XBAR      1
#else
#define BXPT_NUM_STC_SNAPSHOTS          0
#define BXPT_HAS_STC_SNAPSHOT_XBAR 0
#endif

/* Extended TSMF bonding group */
#if BCHP_CHIP == 7278
   #define BXPT_HAS_ETBG    1
#endif

#endif /* BXPT_IS_CORE28NM */

/* RS / XC */
#if (BCHP_CHIP == 7278) || (BCHP_CHIP == 7255)
    /* 7278 does NOT have an transport client buffers. */
#else
    #define BXPT_HAS_XCBUF_HW                1
#endif

/* XPT 4.4 LPDDR RTS improvements. See SW_GUIDE_TO_XPTv4.4.doc from the hw team. */
#if (BCHP_CHIP == 7278)
   #define BXPT_P_HAS_224B_SLOT_SIZE      1
#endif

/* TODO: consider making these private, but shared for both 40nm and 28nm */
#if (BCHP_CHIP==7228)
#define BXPT_P_HAS_AVS_PLUS_WORKAROUND     1 /* SW7228-42. requires extra RAVE context */
#endif
#if (BCHP_CHIP==75845) || (BCHP_CHIP==74295)
#define BXPT_P_HAS_AVS_PLUS_WORKAROUND_TWO 1 /* SW7228-61. does not require extra RAVE context */
#endif

#ifdef __cplusplus
}
#endif

#endif

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
 *   Contains tables automatically generated from VEC design files
 *
 ***************************************************************************/

/*
This file generated automatically on 2015-11-18 15:05
    by program process_all_tags

Register programming derived from
    /vobs/DVTSJ/portinginterface/vdc/7422/A0/CONSOLIDATED_VEC
*/


/* DVTSJ format tag "480i" */
/* From IT_REGISTERS/prog/IT_REGISTERS_480i.txt: */
static const uint32_t s_aulItConfig_480i[1] =
{
(
  BVDC_P_IT_FIELD_DATA( TG_CONFIG, ARBITER_LATENCY       , 0x000b ) |
  BVDC_P_IT_FIELD_DATA( TG_CONFIG, BVB_FRAME_CYCLE_COUNT , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( TG_CONFIG, BVB_PHASE_SYNC        , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( TG_CONFIG, INPUT_STREAM_ENABLE   , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( TG_CONFIG, LINE_PHASE            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( TG_CONFIG, MC_ENABLES            , 0x007f ) |
  BVDC_P_IT_FIELD_DATA( TG_CONFIG, MC_VIDEO_STREAM_SELECT, 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( TG_CONFIG, SLAVE_MODE            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( TG_CONFIG, SUPPRESS_TRIGGER0     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( TG_CONFIG, TRIGGER_CNT_CLR_COND  , 0x0000 )
)
};

/* DVTSJ format tag "576i" */
/* From IT_REGISTERS/prog/IT_REGISTERS_576i.txt: */
static const uint32_t s_aulItConfig_576i[1] =
{
(
  BVDC_P_IT_FIELD_DATA( TG_CONFIG, ARBITER_LATENCY       , 0x000b ) |
  BVDC_P_IT_FIELD_DATA( TG_CONFIG, BVB_FRAME_CYCLE_COUNT , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( TG_CONFIG, BVB_PHASE_SYNC        , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( TG_CONFIG, INPUT_STREAM_ENABLE   , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( TG_CONFIG, LINE_PHASE            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( TG_CONFIG, MC_ENABLES            , 0x007f ) |
  BVDC_P_IT_FIELD_DATA( TG_CONFIG, MC_VIDEO_STREAM_SELECT, 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( TG_CONFIG, SLAVE_MODE            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( TG_CONFIG, SUPPRESS_TRIGGER0     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( TG_CONFIG, TRIGGER_CNT_CLR_COND  , 0x0000 )
)
};

/* DVTSJ format tag "480p" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_480p.txt: */
#define s_aulItConfig_480p s_aulItConfig_480i

/* DVTSJ format tag "576p" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_576p.txt: */
#define s_aulItConfig_576p s_aulItConfig_576i

/* DVTSJ format tag "480p54" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_480p54.txt: */
#define s_aulItConfig_480p54 s_aulItConfig_480i

/* DVTSJ format tag "576p54" */
/* Another instance of IT_REGISTERS/prog/IT_REGISTERS_576p.txt: */
#define s_aulItConfig_576p54 s_aulItConfig_576p

/* DVTSJ format tag "1080i" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_1080i.txt: */
#define s_aulItConfig_1080i s_aulItConfig_480i

/* DVTSJ format tag "1080i_50hz" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_1080i_50hz.txt: */
#define s_aulItConfig_1080i_50hz s_aulItConfig_480i

/* DVTSJ format tag "1250i_50hz" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_1250i_50hz.txt: */
#define s_aulItConfig_1250i_50hz s_aulItConfig_480i

/* DVTSJ format tag "720p" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_720p.txt: */
#define s_aulItConfig_720p s_aulItConfig_480i

/* DVTSJ format tag "720p_24hz" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_720p_24hz.txt: */
#define s_aulItConfig_720p_24hz s_aulItConfig_480i

/* DVTSJ format tag "720p_25hz" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_720p_25hz.txt: */
#define s_aulItConfig_720p_25hz s_aulItConfig_480i

/* DVTSJ format tag "720p_30hz" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_720p_30hz.txt: */
#define s_aulItConfig_720p_30hz s_aulItConfig_480i

/* DVTSJ format tag "720p_50hz" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_720p_50hz.txt: */
#define s_aulItConfig_720p_50hz s_aulItConfig_480i

/* DVTSJ format tag "1080p_24hz" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_1080p_24hz.txt: */
#define s_aulItConfig_1080p_24hz s_aulItConfig_480i

/* DVTSJ format tag "1080p_25hz" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_1080p_25hz.txt: */
#define s_aulItConfig_1080p_25hz s_aulItConfig_480i

/* DVTSJ format tag "1080p_30hz" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_1080p_30hz.txt: */
#define s_aulItConfig_1080p_30hz s_aulItConfig_480i

/* DVTSJ format tag "1080p_50hz" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_1080p_50hz_OSCL.txt: */
#define s_aulItConfig_1080p_50hz s_aulItConfig_480i

/* DVTSJ format tag "1080p_60hz" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_1080p_60hz_OSCL.txt: */
#define s_aulItConfig_1080p_60hz s_aulItConfig_480i

/* DVTSJ format tag "NTSC_ITU" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_NTSC_ITU.txt: */
#define s_aulItConfig_NTSC_ITU s_aulItConfig_480i

/* DVTSJ format tag "NTSC_704" */
/* Another instance of IT_REGISTERS/prog/IT_REGISTERS_NTSC_ITU.txt: */
#define s_aulItConfig_NTSC_704 s_aulItConfig_NTSC_ITU

/* DVTSJ format tag "NTSC_J" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_NTSC_SMPTE.txt: */
#define s_aulItConfig_NTSC_J s_aulItConfig_480i

/* DVTSJ format tag "PAL" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_PAL.txt: */
#define s_aulItConfig_PAL s_aulItConfig_576i

/* DVTSJ format tag "PAL_I" */
/* Another instance of IT_REGISTERS/prog/IT_REGISTERS_PAL.txt: */
#define s_aulItConfig_PAL_I s_aulItConfig_PAL

/* DVTSJ format tag "PAL_IA" */
/* Another instance of IT_REGISTERS/prog/IT_REGISTERS_PAL.txt: */
#define s_aulItConfig_PAL_IA s_aulItConfig_PAL

/* DVTSJ format tag "PAL_DK" */
/* Another instance of IT_REGISTERS/prog/IT_REGISTERS_PAL.txt: */
#define s_aulItConfig_PAL_DK s_aulItConfig_PAL

/* DVTSJ format tag "PAL_N" */
/* Another instance of IT_REGISTERS/prog/IT_REGISTERS_PAL.txt: */
#define s_aulItConfig_PAL_N s_aulItConfig_PAL

/* DVTSJ format tag "PAL_NC" */
/* Another instance of IT_REGISTERS/prog/IT_REGISTERS_PAL.txt: */
#define s_aulItConfig_PAL_NC s_aulItConfig_PAL

/* DVTSJ format tag "PAL_M" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_PAL_M.txt: */
#define s_aulItConfig_PAL_M s_aulItConfig_576i

/* DVTSJ format tag "SECAM" */
/* From IT_REGISTERS/prog/IT_REGISTERS_SECAM.txt: */
static const uint32_t s_aulItConfig_SECAM[1] =
{
(
  BVDC_P_IT_FIELD_DATA( TG_CONFIG, ARBITER_LATENCY       , 0x000b ) |
  BVDC_P_IT_FIELD_DATA( TG_CONFIG, BVB_FRAME_CYCLE_COUNT , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( TG_CONFIG, BVB_PHASE_SYNC        , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( TG_CONFIG, INPUT_STREAM_ENABLE   , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( TG_CONFIG, LINE_PHASE            , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( TG_CONFIG, MC_ENABLES            , 0x007f ) |
  BVDC_P_IT_FIELD_DATA( TG_CONFIG, MC_VIDEO_STREAM_SELECT, 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( TG_CONFIG, SLAVE_MODE            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( TG_CONFIG, SUPPRESS_TRIGGER0     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( TG_CONFIG, TRIGGER_CNT_CLR_COND  , 0x0000 )
)
};

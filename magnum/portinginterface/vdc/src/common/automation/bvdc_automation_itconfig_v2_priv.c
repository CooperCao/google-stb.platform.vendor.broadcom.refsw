/***************************************************************************
 *
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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

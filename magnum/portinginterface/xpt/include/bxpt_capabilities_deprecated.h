/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
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
 * Porting interface code for the data transport core.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
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
#define BXPT_HAS_MESG_BUFFERS 1
#define BXPT_HAS_PACKETSUB    1 /* defined for ALL platforms */
#define BXPT_HAS_RAVE         1
#define BXPT_HAS_MTSIF        1
#define BXPT_HAS_REMUX        1
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

/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/
#include "bstd.h"

#if (BVDC_P_SUPPORT_VIP)

/* ==========================  INCLUDES  ==================================== */

/* ==========================  DEFINITIONS  ================================= */
#define BVDC_P_ITFP_MAX_SIGMA        5
#define BVDC_P_ITFP_NUM_CADENCE      2
#define BVDC_P_ITFP_MAX_TICKER_DETECTION_CNT 6

/* ==========================  STRUCTURES / TYPEDEFS ======================== */

typedef enum
{
    BVDC_P_ITFP_PIC_STRUCT_FRAME = 0,
    BVDC_P_ITFP_PIC_STRUCT_FRAME_DOUBLING = 1,
    BVDC_P_ITFP_PIC_STRUCT_FRAME_TRIPLING = 2,
    BVDC_P_ITFP_PIC_STRUCT_TB_FRAME = 3,
    BVDC_P_ITFP_PIC_STRUCT_BT_FRAME = 4,
    BVDC_P_ITFP_PIC_STRUCT_TBT_FRAME = 5,
    BVDC_P_ITFP_PIC_STRUCT_BTB_FRAME = 6,
    BVDC_P_ITFP_PIC_STRUCT_TB_FIELD = 7,
    BVDC_P_ITFP_PIC_STRUCT_BT_FIELD = 8
} BVDC_P_ITFP_PicStruct_e;

typedef enum {
    BVDC_P_ITFP_CADENCE_FRAME = 0,
    BVDC_P_ITFP_CADENCE_FIELD = 1,
    BVDC_P_ITFP_CADENCE_TFF_FIELD_PAIR = 2,
    BVDC_P_ITFP_CADENCE_BFF_FIELD_PAIR = 3,
    BVDC_P_ITFP_CADENCE_REPEATED_FIELD = 4,
    BVDC_P_ITFP_CADENCE_DANGLING_FIELD = 5,
    BVDC_P_ITFP_CADENCE_UNKNOWN = 6
} BVDC_P_ITFP_EPM_Cadence_e;

typedef enum {
    BVDC_P_ITFP_FIRST_FIELD = 0,
    BVDC_P_ITFP_SECOND_FIELD = 1,
    BVDC_P_ITFP_THIRD_FIELD = 2
} BVDC_P_ITFP_EPM_Cadence_Field_Order_e;

typedef struct
{
       uint16_t NumberOfMbsPerFrame; /*function input (used for threshold calculation) */
       uint32_t Sigma;               /*function input */
       uint32_t Repf_motion;         /*function input */

       BAVC_CadenceType   ProgressiveCadence; /*function output */
       BAVC_CadencePhase  ProgressivePhase;   /*function output */
} BVDC_P_ITFP_ProgressiveItfpInfo_t;

typedef struct {
    uint32_t   PCC;
    uint32_t   Sigma;
    uint32_t   Histogram;
    uint32_t   ulOrigPts;
    uint32_t   Polarity : 2;
    uint32_t   CadenceType : 2;
    uint32_t   CadencePhase : 3;
    uint32_t   IsCadenceSet : 1;
    uint32_t   IsVipStatsSet : 1;
    uint32_t   IsVipStatsRequested : 1;
    uint32_t   OptsDelta2pic : 1;
    uint32_t   OptsDelta1pic : 1;
    uint32_t   OptsDeltaUsed : 1;
} BVDC_P_ITFP_PreProcessorVipBuffers_t;

#define BVDC_P_ITFP_PREPROCESSOR_PIC_QUEUE_SIZE 4

typedef struct {
    BVDC_P_ITFP_PreProcessorVipBuffers_t PreProcessorVipBuffersArray[BVDC_P_ITFP_PREPROCESSOR_PIC_QUEUE_SIZE];
    uint8_t    RdPtr;
    uint8_t    WrPtr;
    uint8_t    Fullness;
    uint8_t    NumOfBuff;
} BVDC_P_ITFP_PreProcessorPictureQueue_t;

/*EPM preprocessor database */
typedef struct
{
    BVDC_P_ITFP_PreProcessorPictureQueue_t PreProcessorPictureQueue;
    bool IsProgressive;
    uint8_t   EncoderModeOfOperation;
    bool IsItfpEnabled;
    uint8_t   OptsItfpPhase;
    uint8_t   OptsPattern32Cntr;
    bool      bPrevOptsLocked32;
    unsigned  TickerDetectionCounter;
} BVDC_P_ITFP_EpmPreprocessorInfo_t;


/*////////////////////////////// */
/* Cadence detection Structures */
/*////////////////////////////// */

/*! ViCE firmware registers (interface for tuning) */
/*------------------------------------------------ */
typedef struct BVDC_P_ITFP_vice_cd_fw_regs_s
{
    int32_t pair_noise_thresh;           /*!< Veto threshold for testing expected pairs of sigma */
    int32_t pair_thresh;                 /*!< Veto threshold for expected pairs of sigma */
    int32_t repf_veto_level;             /*!< Veto threshold for t-1 repf_motion */
    int32_t repf2_veto_level;            /*!< Veto threshold for t-2 repf_motion */
    int32_t switch_pol_still_thresh;     /*!< Repf_motion threshold needs to be this small to allow RFF to flip polarity */

    uint16_t corr_dec_thresh_t1;
    uint16_t corr_zero_thresh_t1;
    uint16_t min_sigma_thresh;

} BVDC_P_ITFP_vice_cd_fw_regs_t;


/*! Cadence info structure */
/*------------------------------------------------ */
typedef struct BVDC_P_ITFP_cad_chan_state_s
{
    int16_t phase_counter[BVDC_P_ITFP_MAX_SIGMA];
    int16_t enter_ll;
    int16_t exit_ll;
    int16_t lock_sat_level;
    uint8_t  LockCntr32;
    uint8_t  Locked32Phase0Synced;
    int8_t  InitialVeto;
} BVDC_P_ITFP_cad_chan_state_t;

typedef struct BVDC_P_ITFP_cad_info_s
{
    BVDC_P_ITFP_cad_chan_state_t state;

    struct
    {
        uint32_t hl    : 1;
        uint32_t t2    : 1;
        uint32_t pair  : 1;
    } pattern[BVDC_P_ITFP_MAX_SIGMA];

    struct
    {
        uint32_t length     : 3;
        uint32_t enable     : 1;
        uint32_t metric_sel : 1;         /* 0 = t-1, 1 = t-2 */
    } control;
} BVDC_P_ITFP_cad_info_t;



/*///////////////// */

/*! Sigma structure */
/*------------------------------------------------ */
/*typedef struct sigma_stats_s */
/*{ */
/*    int32_t sigma; */
/*    int32_t sigma2; */
/*    int32_t gated_sigma; */
/*    int32_t gated_sigma2; */
/*} sigma_stats_t; */


typedef struct {
    uint16_t NumberOfMbsPerFrame; /*Input: number of macroblocks in a frame */
    uint32_t Sigma[4];       /*Input: FIFO: The 5 sigma values for the next 4 fields whose cadence was not determined */
    uint32_t Repf_motion[4];  /*Input: FIFO: The 5 repf_motion values for the next 4 fields */
    uint32_t Pcc[4];               /*Inpput: FIFO: The PCC values for the next 4 fields whose cadence was not determined */
    int8_t Polarity[4];          /*Input: FIFO: Field polarity of each field */
    BVDC_P_ITFP_EPM_Cadence_e   Cadence[4]; /*Output: FIFO: The Cadence of the next 4 fields whose cadence was not determined */
    BVDC_P_ITFP_EPM_Cadence_Field_Order_e FieldOrder[4]; /*Output: FIFO: the field order in the cadence. */
} BVDC_P_ItfpInfo_t;

void BVDC_P_ITFP_EPM_Preprocessor_init(   BVDC_P_Vip_Handle hVip);
void BVDC_P_ITFP_EPM_Preprocessor_flush(BVDC_P_ITFP_EpmPreprocessorInfo_t *pEpmPreprocessorInfo);
void BVDC_P_ITFP_EPM_Preprocessor_ReadVipStats_isr(BVDC_P_Vip_Handle hVip);
void BVDC_P_ITFP_EPM_Preprocessor_CadenceDetection_isr(BVDC_P_Vip_Handle hVip);

#endif

/* End of file */

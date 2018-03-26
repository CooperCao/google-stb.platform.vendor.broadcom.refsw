/***************************************************************************
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
 ***************************************************************************/
/*
   Title: calcDelay

   Computes End-To-End delay for Video

   $brcm_Revision: 39 $
   $brcm_Date: 11/19/13 3:42p $


   NOTE: THIS FILE SHOULD NOT CONTAIN ANY C++-STYLE COMMENTS

*/

#if defined(NGE_FW_SIM_APP) || defined(VICE_FW_SIM_APP)
#define VICE_FW_CMODEL_APP
#endif

/* ==========================  INCLUDES ============================== */
#ifdef BCHP_CHIP
#include "bstd.h"
#include "bdbg.h"
#else
#include <stdint.h>
#include <assert.h>
#if !defined(CEB2APP) && !defined(VICE_FW_CMODEL_APP)
#include "defs.h"
#include "util.h"
#include "platform.h"
#include "fw_debug.h"
#endif
#endif

#include "bvce_fw_api_v3.h"


/* ==========================  DEFINITIONS ======================== */

#define MAX_CABAC_PIC_DELAY                       15
#define MAX_NUM_EBM_PIC_DATA                      50        /* should be greater than MAX_CABAC_PIC_DELAY and cover max number of Pictures in EndToEndDelay */

/* Definitions used for arrival-to-presentation delay used in rate control */
#define DTS_OFFSET_IN_27MHZ_TICKS                  0            /* Delay offset used to ensure no late arrivals */
#define MAX_NUM_REPEAT_PIC_COUNT                   2            /* Max allowed count of repeated pictures */

#define NORMALIZE_ONE_TO_15_BITS_MANTISSA   (0x4000)
#define NORMALIZE_ONE_TO_15_BITS_EXPONENT   (14)
#define  SYSTEM_CLOCK_27_MHZ            27000000
#define TWENTY_SEVEN_KHZ                27000
#define BITS_PER_PIC2MAXPIC_MANTISSA    27
#define BITS_PER_PIC2MAXPIC_EXPONENT    1

#define INVERSE_FRAMERATE_6000_IN_SECONDS_MANTISSA  0x4444
#define INVERSE_FRAMERATE_5994_IN_SECONDS_MANTISSA  0x4455
#define INVERSE_FRAMERATE_5000_IN_SECONDS_MANTISSA  0x51EB
#define INVERSE_FRAMERATE_3000_IN_SECONDS_MANTISSA  0x4444
#define INVERSE_FRAMERATE_2997_IN_SECONDS_MANTISSA  0x4455
#define INVERSE_FRAMERATE_2500_IN_SECONDS_MANTISSA  0x51EB
#define INVERSE_FRAMERATE_2400_IN_SECONDS_MANTISSA  0x5555
#define INVERSE_FRAMERATE_2397_IN_SECONDS_MANTISSA  0x556B
#define INVERSE_FRAMERATE_2000_IN_SECONDS_MANTISSA  0x6666
#define INVERSE_FRAMERATE_1998_IN_SECONDS_MANTISSA  (INVERSE_FRAMERATE_5994_IN_SECONDS_MANTISSA * 3)
#define INVERSE_FRAMERATE_1500_IN_SECONDS_MANTISSA  0x4444
#define INVERSE_FRAMERATE_1498_IN_SECONDS_MANTISSA  0x4455
#define INVERSE_FRAMERATE_1250_IN_SECONDS_MANTISSA  0xA3D7
#define INVERSE_FRAMERATE_1000_IN_SECONDS_MANTISSA  0xCCCC
#define INVERSE_FRAMERATE_0749_IN_SECONDS_MANTISSA  0x88AB
#define INVERSE_FRAMERATE_0750_IN_SECONDS_MANTISSA  0x8888
#define INVERSE_FRAMERATE_1198_IN_SECONDS_MANTISSA  0xAAAA
#define INVERSE_FRAMERATE_1200_IN_SECONDS_MANTISSA  0xAAD6
#define INVERSE_FRAMERATE_0999_IN_SECONDS_MANTISSA  0xCD01

#define INVERSE_FRAMERATE_6000_IN_SECONDS_EXPONENT  20
#define INVERSE_FRAMERATE_5994_IN_SECONDS_EXPONENT  20
#define INVERSE_FRAMERATE_5000_IN_SECONDS_EXPONENT  20
#define INVERSE_FRAMERATE_3000_IN_SECONDS_EXPONENT  19
#define INVERSE_FRAMERATE_2997_IN_SECONDS_EXPONENT  19
#define INVERSE_FRAMERATE_2500_IN_SECONDS_EXPONENT  19
#define INVERSE_FRAMERATE_2400_IN_SECONDS_EXPONENT  19
#define INVERSE_FRAMERATE_2397_IN_SECONDS_EXPONENT  19
#define INVERSE_FRAMERATE_2000_IN_SECONDS_EXPONENT  19
#define INVERSE_FRAMERATE_1998_IN_SECONDS_EXPONENT  20
#define INVERSE_FRAMERATE_1500_IN_SECONDS_EXPONENT  18
#define INVERSE_FRAMERATE_1498_IN_SECONDS_EXPONENT  18
#define INVERSE_FRAMERATE_1250_IN_SECONDS_EXPONENT  19
#define INVERSE_FRAMERATE_1000_IN_SECONDS_EXPONENT  19
#define INVERSE_FRAMERATE_0749_IN_SECONDS_EXPONENT  18
#define INVERSE_FRAMERATE_0750_IN_SECONDS_EXPONENT  18
#define INVERSE_FRAMERATE_1198_IN_SECONDS_EXPONENT  19
#define INVERSE_FRAMERATE_1200_IN_SECONDS_EXPONENT  19
#define INVERSE_FRAMERATE_0999_IN_SECONDS_EXPONENT  19

#define INVERSE_FRAMERATE_6000_IN_27MHZ_TICKS    450000
#define INVERSE_FRAMERATE_5994_IN_27MHZ_TICKS    450450
#define INVERSE_FRAMERATE_5000_IN_27MHZ_TICKS    540000
#define INVERSE_FRAMERATE_3000_IN_27MHZ_TICKS    900000
#define INVERSE_FRAMERATE_2997_IN_27MHZ_TICKS    900900
#define INVERSE_FRAMERATE_2500_IN_27MHZ_TICKS    1080000
#define INVERSE_FRAMERATE_2400_IN_27MHZ_TICKS    1125000
#define INVERSE_FRAMERATE_2397_IN_27MHZ_TICKS    1126126
#define INVERSE_FRAMERATE_2000_IN_27MHZ_TICKS    1350000
#define INVERSE_FRAMERATE_1998_IN_27MHZ_TICKS    ( INVERSE_FRAMERATE_5994_IN_27MHZ_TICKS * 3 )
#define INVERSE_FRAMERATE_1500_IN_27MHZ_TICKS    1800000
#define INVERSE_FRAMERATE_1498_IN_27MHZ_TICKS    1801800
#define INVERSE_FRAMERATE_1250_IN_27MHZ_TICKS    2160000
#define INVERSE_FRAMERATE_1000_IN_27MHZ_TICKS    2700000
#define INVERSE_FRAMERATE_0749_IN_27MHZ_TICKS    3603600
#define INVERSE_FRAMERATE_0750_IN_27MHZ_TICKS    3600000
#define INVERSE_FRAMERATE_1198_IN_27MHZ_TICKS    2252250
#define INVERSE_FRAMERATE_1200_IN_27MHZ_TICKS    2250000
#define INVERSE_FRAMERATE_0999_IN_27MHZ_TICKS    2702700

#define INVERSE_FRAMERATE_1498_IN_90KHZ_TICKS    6006
#define MAX_PIC_INTERVAL_IN_90KHZ_TICKS         INVERSE_FRAMERATE_1498_IN_90KHZ_TICKS
#define MAX_PIC_INTERVAL_IN_27MHZ_TICKS         (MAX_PIC_INTERVAL_IN_90KHZ_TICKS*300)
#define MIN_PIC_INTERVAL_IN_27MHZ_TICKS         INVERSE_FRAMERATE_6000_IN_27MHZ_TICKS
#define MAX_PRE_ENCODE_DELAY_VIP                INVERSE_FRAMERATE_2400_IN_27MHZ_TICKS
#define MAX_PRE_ENCODE_DELAY_CME                INVERSE_FRAMERATE_3000_IN_27MHZ_TICKS
#define MAX_PRE_ENCODE_DELAY_ITFP               (MAX_NUM_REPEAT_PIC_COUNT * INVERSE_FRAMERATE_6000_IN_27MHZ_TICKS)
#define MAX_ENCODE_DELAY                        INVERSE_FRAMERATE_3000_IN_27MHZ_TICKS

/* define max CPB size according to level fo H264 */
#define H264_MAX_CPB_LEVEL_10  (1000*175)
#define H264_MAX_CPB_LEVEL_10b  (1000*350)
#define H264_MAX_CPB_LEVEL_11  (1000*500)
#define H264_MAX_CPB_LEVEL_12  (1000*1000)
#define H264_MAX_CPB_LEVEL_13  (1000*2000)
#define H264_MAX_CPB_LEVEL_20  (1000*2000)
#define H264_MAX_CPB_LEVEL_21  (1000*4000)
#define H264_MAX_CPB_LEVEL_22  (1000*4000)
#define H264_MAX_CPB_LEVEL_30  (1000*10000)
#define H264_MAX_CPB_LEVEL_31  (1000*14000)
#define H264_MAX_CPB_LEVEL_32  (1000*20000)
#define H264_MAX_CPB_LEVEL_40  (1000*25000)
#define H264_MAX_CPB_LEVEL_41  (1000*62500)
#define H264_MAX_CPB_LEVEL_42  (1000*62500)
#define H264_MAX_CPB_LEVEL_50  (1000*135000)
#define H264_MAX_CPB_LEVEL_51  (1000*240000)

/* define max CPB size according to level fo MPEG2 Main Profile */
#define MPEG2_MAX_VBV_LEVEL_LOW         475136
#define MPEG2_MAX_VBV_LEVEL_MAIN        1835008
#define MPEG2_MAX_VBV_LEVEL_HIGH1440    7340032
#define MPEG2_MAX_VBV_LEVEL_HIGH        9781248

/* define max CPB size according to level fo HEVC */
#define HEVC_MAX_CPB_LEVEL_40  H264_MAX_CPB_LEVEL_40 /* TBD - Update value */
#define HEVC_MAX_CPB_LEVEL_41  H264_MAX_CPB_LEVEL_41 /* TBD - Update value */
#define HEVC_MAX_CPB_LEVEL_50  H264_MAX_CPB_LEVEL_50 /* TBD - Update value */

/* FIXME: this is just to keep the interface, however at this point VP8 does not define any HRD */
#define VP8_DUMMY_HRD_BUFFER_SIZE          (1000*240000)
#define VP9_DUMMY_HRD_BUFFER_SIZE          (1000*240000)

#define RESOLUTION_T0_27MHZ_MBPDELAY_MANT       56471
#define RESOLUTION_T0_27MHZ_MBPDELAY_EXP        17
#define LOW_DELAY_ADDITIVE_DELAY_IN_27MHZ       108000      /* 4 milliseconds for coding several MB rows */
#define MAX_DEE_IN_MILLISECONDS                 3000        /* used to limit internal rate buffer delay calculation */
#define MAX_DEE_IN_27_MHZ_TICKS                 (MAX_DEE_IN_MILLISECONDS*TWENTY_SEVEN_KHZ )
#define HORIZONTAL_SIZE_IN_PELS_SD              720
#define HORIZONTAL_SIZE_IN_PELS_HD              1280


/* DEFINE TEMPORARY INPUT UNTIL API COMPLETE FOR A2PDELAY CALC */
#define TEMP_INPUT_API_MODE                 ENCODER_MODE_AFAP
#define TEMP_INPUT_API_RATE_BUFFER          0
#define TEMP_INPUT_API_MIN_FRAMERATE        ENCODING_FRAME_RATE_1498
#define TEMP_INPUT_API_BVN_RATE             ENCODING_FRAME_RATE_5994
#define TEMP_INPUT_API_MULTI_CHAN           0
#define TEMP_INPUT_API_INPUT_TYPE           ENCODER_INPUT_TYPE_INTERLACED
#define TEMP_INPUT_API_GOP_STRUCT           ENCODING_GOP_STRUCT_IBBP
#define TEMP_INPUT_API_MAX_PIC_WIDTH_PELS   1920
#define TEMP_INPUT_API_MAX_PIC_HEIGT_PELS   1088
#if defined(BCHP_CHIP) || defined(CEB2APP) || defined(VICE_FW_CMODEL_APP)

#define NORM31(output, input) {uint8_t i; for (i=0; (i<=31) && (input>>i != 0) && (input>>i != -1); i++); output= 31-i;}
#define DIVIDE16(numerator, denominator) arcDivide16((int16_t)numerator, (int16_t)denominator)
#define UMULT16(operandA, operandB) (uint32_t)((uint16_t)((operandA) & UINT16_MAX) * (uint16_t)((operandB) & UINT16_MAX))
#define U32xU16MULTIPLY(operand32bit, operand16bit) ((UMULT16((operand32bit)>>16, (operand16bit))<<16) + UMULT16((operand32bit), (operand16bit)))
#define U32xU32MULTIPLY(operand32bit1, operand32bit2) (((u64_t)UMULT16((operand32bit1 >> 16), (operand32bit2>>16)) << 32) +\
        ((u64_t)UMULT16((operand32bit1 >> 16),operand32bit2) << 16) +\
        ((u64_t)UMULT16(operand32bit1,(operand32bit2 >> 16)) << 16) + (u64_t)UMULT16(operand32bit1, operand32bit2))
#define U32xPOSQ15MULTIPLY(operand32bit,  operand16bit) ((UMULT16((operand32bit)>>16, (operand16bit))<<1) + (UMULT16((operand32bit), (operand16bit))>>15))
#define U32xPOSQ8MULTIPLY(operand32bit,  operand16bit) ((UMULT16((operand32bit)>>16, (operand16bit))<<8) + (UMULT16((operand32bit), (operand16bit))>>8))

/* ==========================  STRUCTURES  ================================== */
static void RcNumToFP(FPNumber_t *outputFPNumber, uint32_t inputNumber, uint8_t numBits);
static uint32_t RcFPToNum(FPNumber_t localFPNumber);
static uint32_t RcU32ScaleFP(FPNumber_t FP_operand, uint32_t scaleFactor, uint8_t bypassNormFlag);
static int16_t arcDivide16(int16_t numerator, int16_t denominator);
static void NormalizeFP(FPNumber_t *pOutputFPNumber, FPNumber_t pInputFPNumber, uint8_t numBits);

#else

extern void_t RcNumToFP(FPNumber_t *outputFPNumber, u32_t inputNumber, u8_t numBits);
extern u32_t RcFPToNum(FPNumber_t localFPNumber);
extern u32_t RcU32ScaleFP(FPNumber_t FP_operand, u32_t scaleFactor, uint8_t bypassNormFlag);
extern void_t NormalizeFP(FPNumber_t *pOutputFPNumber, FPNumber_t pInputFPNumber, u8_t numBits);

#endif /* defined(BCHP_CHIP) || defined(CEB2APP) || defined(VICE_FW_CMODEL_APP) */

/* ==========================  DATA  ============================== */

/* ==========================  CODE  ============================== */


#ifndef BSTD_UNUSED
#define BSTD_UNUSED(x)
#endif

#if defined(BCHP_CHIP) || defined(CEB2APP) || defined(VICE_FW_CMODEL_APP)
/***********************************************************************
*  CalcVideoEndToEndDelay()
*    Calculates Arrival-To-Presentation (A2P) delay and maxAllowed A2P
*    delay for video
************************************************************************/
uint32_t
BVCE_FW_P_CalcVideoA2Pdelay(
         uint32_t            *MaxAllowedA2Pdelay,       /* output max A2P delay allowed for Video                */
         uint32_t            Protocol,                  /* encoding standard                                     */
         uint32_t            Profile,                   /* encoding profile                                      */
         EncoderLevel_e      Level,                     /* Profile Level                                         */
         FrameRateCode_e     FrameRateCode,             /* Frame rate in unit of integer (100*frames/sec)        */
         uint32_t            MaxBitrate,                /* Peak output bit rate in bits/sec                      */
         uint8_t             Mode,                      /* Encoder mode of operation                             */
         uint32_t            RateBufferDelay,           /* Input rate buffer delay                               */
         FrameRateCode_e     MinFrameRateCodeLimit,     /* Minimum frame rate allowed during dynamic rate change */
         FrameRateCode_e     MinAllowedBvnFrameRateCode,    /* Input BVN frame rate                                  */
         uint8_t             MultiChannelEnable,        /* Flag dual channel operation                           */
         uint8_t             ITFPenable,                /* ITFP enable                                           */
         uint8_t             InputType,                 /* Interlaced or Progressive input type                  */
         uint8_t             MaxAllowedGopStruct,                   /* Gop structure                                         */
         uint32_t            PictureWidthInPels,        /* Max picture resolution allowed                        */
         uint32_t            PictureHeightInPels,
         uint8_t             MaxNumFNRT                 /* Max number os parallel transcodes                    */
)
{
    FPNumber_t FP_ticksPerBit, FP_inverse;
    uint32_t MaxSupportedEndToEndDelayIn27MHz;
    uint32_t delayEndToEndIn27MhzTicks;
    uint32_t   hrdBufferSize;
    uint32_t   averageBitsPerPic;
    uint32_t   ptsDtsDelayOffset = 0;
    uint32_t encodingDelays, A2PdelayIn27MHzTicks, maxAllowedBitsPerPicture;
    uint32_t   maxPictureIntervalIn27MhzTicks = 0;
    FrameRate_e FrameRate, MinFrameRateLimit;

    BSTD_UNUSED( FrameRate );
    BSTD_UNUSED( MultiChannelEnable );
    BSTD_UNUSED( MinAllowedBvnFrameRateCode );
    BSTD_UNUSED( ITFPenable );
    BSTD_UNUSED( PictureWidthInPels );
    BSTD_UNUSED( PictureHeightInPels );

    /* Initialize output parameters */
    delayEndToEndIn27MhzTicks = 0;
    MaxAllowedA2Pdelay[0] = MaxSupportedEndToEndDelayIn27MHz = 0;
    averageBitsPerPic = 0;
    hrdBufferSize = 0;
    FrameRate = BVCE_FW_P_FrameRateCodeToFrameRate(FrameRateCode);
    MinFrameRateLimit = BVCE_FW_P_FrameRateCodeToFrameRate(MinFrameRateCodeLimit);

    /* Convert min frame rate to inverse  */
    maxPictureIntervalIn27MhzTicks = BVCE_FW_P_ConvertFrameRate(MinFrameRateLimit, &FP_inverse);

    /* Compute max average bits per pic for input frame rate */
    FP_inverse.mantissa = U32xPOSQ15MULTIPLY(MaxBitrate, FP_inverse.mantissa);
    FP_inverse.exponent -= 15;
    averageBitsPerPic = RcFPToNum(FP_inverse);


    /* Compute size based on encoding standard level and profile */
    hrdBufferSize = BVCE_FW_P_CalcHRDbufferSize(Protocol, Profile, Level);

    /* Convert rate in ticks per bit */
    BVCE_FW_P_ComputeRateInTicksPerBit(&FP_ticksPerBit, MaxBitrate);
    NormalizeFP(&FP_ticksPerBit, FP_ticksPerBit, 16);

    /* Compute end to end delay in 27Mhz ticks. Also update the max allowed bits per picture */
    if(RateBufferDelay == 0)
    {
        if(Mode == ENCODER_MODE_LOW_DELAY)
        {
                delayEndToEndIn27MhzTicks = maxPictureIntervalIn27MhzTicks;
        }
        else
        {
            /* if not in low Delay, make minimum delayEndToEnd to be hard-coded max pic interval for potentially better picture quality */
            delayEndToEndIn27MhzTicks = BVCE_FW_P_Compute_DeeIn27MhzTicks(averageBitsPerPic, hrdBufferSize, FP_ticksPerBit,
                                                                           MAX_PIC_INTERVAL_IN_27MHZ_TICKS, &maxAllowedBitsPerPicture, MaxNumFNRT);
        }
    }
    else
    {
        /* convert milliseconds to 27 MHz ticks */
        FP_inverse.mantissa = RateBufferDelay;
        FP_inverse.exponent = 0;
        delayEndToEndIn27MhzTicks = RcU32ScaleFP(FP_inverse,  TWENTY_SEVEN_KHZ, 0);
    }


    /* Compute the true max end to end delay supported based on HRD size and bit rate */
    MaxSupportedEndToEndDelayIn27MHz = BVCE_FW_P_Compute_MaxDeeIn27MhzTicks(hrdBufferSize, FP_ticksPerBit);
    MaxSupportedEndToEndDelayIn27MHz -= maxPictureIntervalIn27MhzTicks;

    /* end-to-end delay should be less than Max supported to always ensure no HRD overflow */
    if (delayEndToEndIn27MhzTicks > MaxSupportedEndToEndDelayIn27MHz)
    {
        delayEndToEndIn27MhzTicks = MaxSupportedEndToEndDelayIn27MHz;
    }

    /* add max pre-encoding and encoding delays */
    encodingDelays = BVCE_FW_P_Compute_EncodeDelayIn27MHzTicks(MinFrameRateLimit, InputType, MaxAllowedGopStruct);

    if ((MaxAllowedGopStruct == ENCODING_GOP_STRUCT_IBP) ||
        (MaxAllowedGopStruct == ENCODING_GOP_STRUCT_IBBP) ||
		(MaxAllowedGopStruct == ENCODING_GOP_STRUCT_IBBP_B_REF) ||
        (MaxAllowedGopStruct == ENCODING_GOP_STRUCT_IBBBP) ||
		(MaxAllowedGopStruct == ENCODING_GOP_STRUCT_IBBBBBP) ||
		(MaxAllowedGopStruct == ENCODING_GOP_STRUCT_IBBBBBBBP)
		)
    {
        /* Offset between PTS and DTS when B pictures allowed */
        ptsDtsDelayOffset = maxPictureIntervalIn27MhzTicks;
    }
    else
    {
        ptsDtsDelayOffset = 0;
    }

    MaxAllowedA2Pdelay[0] = MaxSupportedEndToEndDelayIn27MHz + encodingDelays + ptsDtsDelayOffset;

    /* check for overflow and saturate if it occurs */
    if(MaxAllowedA2Pdelay[0] < MaxSupportedEndToEndDelayIn27MHz )
    {
        MaxAllowedA2Pdelay[0] = UINT32_MAX;
    }

    A2PdelayIn27MHzTicks  = delayEndToEndIn27MhzTicks + encodingDelays + ptsDtsDelayOffset;

    if(A2PdelayIn27MHzTicks > MaxAllowedA2Pdelay[0])
    {
        A2PdelayIn27MHzTicks = MaxAllowedA2Pdelay[0];
    }

    return(A2PdelayIn27MHzTicks);
}
#endif

/***********************************************************************
*  Compute_DeeIn27MhzTicks() - Compute max supported end to end delay
*  in units expressed as 27 MHz clock ticks
************************************************************************/
uint32_t
BVCE_FW_P_Compute_MaxDeeIn27MhzTicks(
    uint32_t    hrdBufferSize,
    FPNumber_t  FP_ticksPerBit
)
{
    uint32_t MaxSupportedEndToEndDelayIn27MHz;

    /* update Max delay to that which garantees no HRD buffer overflow for any encoder when  EndToEndDelay * Rate < HRDsize */
    MaxSupportedEndToEndDelayIn27MHz = RcU32ScaleFP(FP_ticksPerBit, hrdBufferSize, 1);

    return(MaxSupportedEndToEndDelayIn27MHz);
}

/**********************************************************
*  Compute_DeeIn27MhzTicks() - Compute end to end delay
*  in units expressed as 27 MHz clock ticks
**********************************************************/
uint32_t
BVCE_FW_P_Compute_DeeIn27MhzTicks(
    uint32_t        averageBitsPerPic,
    uint32_t        hrdBufferSize,
    FPNumber_t      FP_ticksPerBit,
    uint32_t        maxPictureIntervalIn27MhzTicks,
    uint32_t        *pMaxAllowedBitsPerPicture,
    uint8_t         MaxNumFNRT
)
{
    uint32_t delayEndToEndIn27MhzTicks, delayEndToEndTimesMaxRate;
    FPNumber_t      FP_inverse;

    /* Set endToEnd Delay = (SizeHRD - MAX(PicSize))/Rate for CBR */
    /* pMaxAllowedBitsPerPicture[0] must be less than HRD size */
    pMaxAllowedBitsPerPicture[0] = U32xU16MULTIPLY(averageBitsPerPic, BITS_PER_PIC2MAXPIC_MANTISSA);
    pMaxAllowedBitsPerPicture[0] = pMaxAllowedBitsPerPicture[0] >> BITS_PER_PIC2MAXPIC_EXPONENT;
    if((hrdBufferSize <= averageBitsPerPic) || (pMaxAllowedBitsPerPicture[0] >= (hrdBufferSize - averageBitsPerPic)))
    {
        /* Potentially too small HRD if sizeHRD <= MAX_INTER_PICTURE_INTERVAL * MAX_BIT_RATE */
        pMaxAllowedBitsPerPicture[0] = hrdBufferSize;
        delayEndToEndIn27MhzTicks = maxPictureIntervalIn27MhzTicks;
    }
    else
    {
        /* Dee*Rcbr = (Sizehrd - Sizemax) */
        delayEndToEndTimesMaxRate = (hrdBufferSize - pMaxAllowedBitsPerPicture[0]);

        /* saturate multiplication to 32 bits (which also limits delay to 2.65 minutes) */
        delayEndToEndIn27MhzTicks = RcU32ScaleFP(FP_ticksPerBit, delayEndToEndTimesMaxRate, 1);

        if(delayEndToEndIn27MhzTicks < maxPictureIntervalIn27MhzTicks)
        {
            delayEndToEndIn27MhzTicks = maxPictureIntervalIn27MhzTicks;
        }
    }

    /* Ensure reasonable delay when automatically computed */
    if(delayEndToEndIn27MhzTicks > MAX_DEE_IN_27_MHZ_TICKS)
    {
        delayEndToEndIn27MhzTicks = (uint32_t)MAX_DEE_IN_27_MHZ_TICKS;
    }

    /* Dee = delayEndToEndIn27MhzTicks / MaxNumFNRT */
    if (MaxNumFNRT > 1)
    {
        RcNumToFP(&FP_inverse, MaxNumFNRT, 15);

        FP_inverse.mantissa = DIVIDE16(NORMALIZE_ONE_TO_15_BITS_MANTISSA, FP_inverse.mantissa);
        FP_inverse.exponent = (NORMALIZE_ONE_TO_15_BITS_EXPONENT - FP_inverse.exponent + 15);

        delayEndToEndIn27MhzTicks = RcU32ScaleFP(FP_inverse, delayEndToEndIn27MhzTicks, 0);
    }

    return(delayEndToEndIn27MhzTicks);
}

/**********************************************************
*  ComputeRateInTicksPerBit() - Converts frame rate in bps
*  to bits per ticks based on 27 MHz clock cycles
**********************************************************/
void
BVCE_FW_P_ComputeRateInTicksPerBit(
    FPNumber_t      *pt_FP_ticksPerBit,
    uint32_t            bitrateBPS
)
{
    FPNumber_t FP_inverseRate;

    RcNumToFP(&FP_inverseRate, bitrateBPS, 15);
    FP_inverseRate.mantissa = DIVIDE16(NORMALIZE_ONE_TO_15_BITS_MANTISSA, FP_inverseRate.mantissa);
    FP_inverseRate.exponent = (NORMALIZE_ONE_TO_15_BITS_EXPONENT - FP_inverseRate.exponent + 15);

    /*Can treat multiply below as 32bit constant times positive signed Q8.8 number and it will not overflow nor
     lose any significant bits given that ultimately want full 16 bit mantissa for CABAC interface. */
    pt_FP_ticksPerBit->mantissa = U32xPOSQ8MULTIPLY(SYSTEM_CLOCK_27_MHZ, FP_inverseRate.mantissa);
    pt_FP_ticksPerBit->exponent = FP_inverseRate.exponent - 8;
}

/********************************************************
*  ConvertFrameRate() - Converts frame rate and
*   computes avg bits per frame.
*
********************************************************/
uint32_t
BVCE_FW_P_ConvertFrameRate(
    FrameRate_e     framesPerSecond,
    FPNumber_t      *FP_inverse
)
{
    uint32_t   PictureIntervalIn27MhzTicks = 0;

    switch(framesPerSecond)
    {
    /* stored as 100xFrameRate */
    case ENCODING_FRAME_RATE_6000:
        /* need picture interval in 27 MHZ ticks */
        /* need picture interval in seconds ==> 1/FrameRate */
        PictureIntervalIn27MhzTicks = INVERSE_FRAMERATE_6000_IN_27MHZ_TICKS;
        FP_inverse->mantissa = INVERSE_FRAMERATE_6000_IN_SECONDS_MANTISSA;
        FP_inverse->exponent = INVERSE_FRAMERATE_6000_IN_SECONDS_EXPONENT;
        break;
    case ENCODING_FRAME_RATE_5994:
        PictureIntervalIn27MhzTicks = INVERSE_FRAMERATE_5994_IN_27MHZ_TICKS;
        FP_inverse->mantissa = INVERSE_FRAMERATE_5994_IN_SECONDS_MANTISSA;
        FP_inverse->exponent = INVERSE_FRAMERATE_5994_IN_SECONDS_EXPONENT;
        break;
    case ENCODING_FRAME_RATE_5000:
        PictureIntervalIn27MhzTicks = INVERSE_FRAMERATE_5000_IN_27MHZ_TICKS;
        FP_inverse->mantissa = INVERSE_FRAMERATE_5000_IN_SECONDS_MANTISSA;
        FP_inverse->exponent = INVERSE_FRAMERATE_5000_IN_SECONDS_EXPONENT;
        break;
    case ENCODING_FRAME_RATE_3000:
        PictureIntervalIn27MhzTicks = INVERSE_FRAMERATE_3000_IN_27MHZ_TICKS;
        FP_inverse->mantissa = INVERSE_FRAMERATE_3000_IN_SECONDS_MANTISSA;
        FP_inverse->exponent = INVERSE_FRAMERATE_3000_IN_SECONDS_EXPONENT;
        break;
    case ENCODING_FRAME_RATE_2997:
        PictureIntervalIn27MhzTicks = INVERSE_FRAMERATE_2997_IN_27MHZ_TICKS;
        FP_inverse->mantissa = INVERSE_FRAMERATE_2997_IN_SECONDS_MANTISSA;
        FP_inverse->exponent = INVERSE_FRAMERATE_2997_IN_SECONDS_EXPONENT;
        break;
    case ENCODING_FRAME_RATE_2500:
        PictureIntervalIn27MhzTicks = INVERSE_FRAMERATE_2500_IN_27MHZ_TICKS;
        FP_inverse->mantissa = INVERSE_FRAMERATE_2500_IN_SECONDS_MANTISSA;
        FP_inverse->exponent = INVERSE_FRAMERATE_2500_IN_SECONDS_EXPONENT;
        break;
    case ENCODING_FRAME_RATE_2400:
        PictureIntervalIn27MhzTicks = INVERSE_FRAMERATE_2400_IN_27MHZ_TICKS;
        FP_inverse->mantissa = INVERSE_FRAMERATE_2400_IN_SECONDS_MANTISSA;
        FP_inverse->exponent = INVERSE_FRAMERATE_2400_IN_SECONDS_EXPONENT;
        break;
    case ENCODING_FRAME_RATE_2397/*.6*/:
        PictureIntervalIn27MhzTicks = INVERSE_FRAMERATE_2397_IN_27MHZ_TICKS;
        FP_inverse->mantissa = INVERSE_FRAMERATE_2397_IN_SECONDS_MANTISSA;
        FP_inverse->exponent = INVERSE_FRAMERATE_2397_IN_SECONDS_EXPONENT;
        break;
    case ENCODING_FRAME_RATE_2000:
        PictureIntervalIn27MhzTicks = INVERSE_FRAMERATE_2000_IN_27MHZ_TICKS;
        FP_inverse->mantissa = INVERSE_FRAMERATE_2000_IN_SECONDS_MANTISSA;
        FP_inverse->exponent = INVERSE_FRAMERATE_2000_IN_SECONDS_EXPONENT;
        break;
    case ENCODING_FRAME_RATE_1998:
        PictureIntervalIn27MhzTicks = INVERSE_FRAMERATE_1998_IN_27MHZ_TICKS;
        FP_inverse->mantissa = INVERSE_FRAMERATE_1998_IN_SECONDS_MANTISSA;
        FP_inverse->exponent = INVERSE_FRAMERATE_1998_IN_SECONDS_EXPONENT;
        break;
    case ENCODING_FRAME_RATE_1500:
        PictureIntervalIn27MhzTicks = INVERSE_FRAMERATE_1500_IN_27MHZ_TICKS;
        FP_inverse->mantissa = INVERSE_FRAMERATE_1500_IN_SECONDS_MANTISSA;
        FP_inverse->exponent = INVERSE_FRAMERATE_1500_IN_SECONDS_EXPONENT;
        break;
    case ENCODING_FRAME_RATE_1498:
        PictureIntervalIn27MhzTicks = INVERSE_FRAMERATE_1498_IN_27MHZ_TICKS;
        FP_inverse->mantissa = INVERSE_FRAMERATE_1498_IN_SECONDS_MANTISSA;
        FP_inverse->exponent = INVERSE_FRAMERATE_1498_IN_SECONDS_EXPONENT;
        break;
    case ENCODING_FRAME_RATE_1250:
        PictureIntervalIn27MhzTicks = INVERSE_FRAMERATE_1250_IN_27MHZ_TICKS;
        FP_inverse->mantissa = INVERSE_FRAMERATE_1250_IN_SECONDS_MANTISSA;
        FP_inverse->exponent = INVERSE_FRAMERATE_1250_IN_SECONDS_EXPONENT;
        break;
    case ENCODING_FRAME_RATE_1000:
        PictureIntervalIn27MhzTicks = INVERSE_FRAMERATE_1000_IN_27MHZ_TICKS;
        FP_inverse->mantissa = INVERSE_FRAMERATE_1000_IN_SECONDS_MANTISSA;
        FP_inverse->exponent = INVERSE_FRAMERATE_1000_IN_SECONDS_EXPONENT;
        break;
    case ENCODING_FRAME_RATE_0749:
        PictureIntervalIn27MhzTicks = INVERSE_FRAMERATE_0749_IN_27MHZ_TICKS;
        FP_inverse->mantissa = INVERSE_FRAMERATE_0749_IN_SECONDS_MANTISSA;
        FP_inverse->exponent = INVERSE_FRAMERATE_0749_IN_SECONDS_EXPONENT;
        break;
    case ENCODING_FRAME_RATE_0750:
        PictureIntervalIn27MhzTicks = INVERSE_FRAMERATE_0750_IN_27MHZ_TICKS;
        FP_inverse->mantissa = INVERSE_FRAMERATE_0750_IN_SECONDS_MANTISSA;
        FP_inverse->exponent = INVERSE_FRAMERATE_0750_IN_SECONDS_EXPONENT;
        break;
    case ENCODING_FRAME_RATE_1198:
        PictureIntervalIn27MhzTicks = INVERSE_FRAMERATE_1198_IN_27MHZ_TICKS;
        FP_inverse->mantissa = INVERSE_FRAMERATE_1198_IN_SECONDS_MANTISSA;
        FP_inverse->exponent = INVERSE_FRAMERATE_1198_IN_SECONDS_EXPONENT;
        break;
    case ENCODING_FRAME_RATE_1200:
        PictureIntervalIn27MhzTicks = INVERSE_FRAMERATE_1200_IN_27MHZ_TICKS;
        FP_inverse->mantissa = INVERSE_FRAMERATE_1200_IN_SECONDS_MANTISSA;
        FP_inverse->exponent = INVERSE_FRAMERATE_1200_IN_SECONDS_EXPONENT;
        break;
    case ENCODING_FRAME_RATE_0999:
        PictureIntervalIn27MhzTicks = INVERSE_FRAMERATE_0999_IN_27MHZ_TICKS;
        FP_inverse->mantissa = INVERSE_FRAMERATE_0999_IN_SECONDS_MANTISSA;
        FP_inverse->exponent = INVERSE_FRAMERATE_0999_IN_SECONDS_EXPONENT;
        break;
    default:
#ifdef BCHP_CHIP
        BDBG_ASSERT(!"unsupported frame rate");
        return 0;
#else
#if defined(CEB2APP) || defined (VICE_FW_CMODEL_APP)
        assert(!"unsupported frame rate");
#else
        ASSERT(!"unsupported frame rate");
#endif
        break;
#endif
    }

    return(PictureIntervalIn27MhzTicks);
}


/********************************************************
*  CalcHRDbufferSize() - Determines HRD buffer size
*    to use.
********************************************************/
uint32_t
BVCE_FW_P_CalcHRDbufferSize(
        uint32_t            Protocol,       /* encoding standard    */
        uint32_t            Profile,        /* encoding profile     */
        EncoderLevel_e      Level           /* Profile Level        */
)
{
    uint32_t   hrdBufferSize;
    BSTD_UNUSED(Profile);

    /* compute size based on encoding standard level and profile */
    switch(Protocol)
    {
    case ENCODING_STD_VP9:
        hrdBufferSize = VP9_DUMMY_HRD_BUFFER_SIZE;
        break;

    case ENCODING_STD_HEVC:
        switch(Level)
        {
        case ENCODING_HEVC_LEVEL_40: hrdBufferSize = HEVC_MAX_CPB_LEVEL_40; break;
        case ENCODING_HEVC_LEVEL_41: hrdBufferSize = HEVC_MAX_CPB_LEVEL_41; break;
        case ENCODING_HEVC_LEVEL_50: hrdBufferSize = HEVC_MAX_CPB_LEVEL_50; break;
        default: hrdBufferSize = 0; /*error*/
        }
        break;

    case ENCODING_STD_H264:
        switch(Level)
        {
        case ENCODING_AVC_LEVEL_10: hrdBufferSize = H264_MAX_CPB_LEVEL_10; break;
        case ENCODING_AVC_LEVEL_10b: hrdBufferSize = H264_MAX_CPB_LEVEL_10b; break;
        case ENCODING_AVC_LEVEL_11: hrdBufferSize = H264_MAX_CPB_LEVEL_11; break;
        case ENCODING_AVC_LEVEL_12: hrdBufferSize = H264_MAX_CPB_LEVEL_12; break;
        case ENCODING_AVC_LEVEL_13: hrdBufferSize = H264_MAX_CPB_LEVEL_13; break;
        case ENCODING_AVC_LEVEL_20: hrdBufferSize = H264_MAX_CPB_LEVEL_20; break;
        case ENCODING_AVC_LEVEL_21: hrdBufferSize = H264_MAX_CPB_LEVEL_21; break;
        case ENCODING_AVC_LEVEL_22: hrdBufferSize = H264_MAX_CPB_LEVEL_22; break;
        case ENCODING_AVC_LEVEL_30: hrdBufferSize = H264_MAX_CPB_LEVEL_30; break;
        case ENCODING_AVC_LEVEL_31: hrdBufferSize = H264_MAX_CPB_LEVEL_31; break;
        case ENCODING_AVC_LEVEL_32: hrdBufferSize = H264_MAX_CPB_LEVEL_32; break;
        case ENCODING_AVC_LEVEL_40: hrdBufferSize = H264_MAX_CPB_LEVEL_40; break;
        case ENCODING_AVC_LEVEL_41: hrdBufferSize = H264_MAX_CPB_LEVEL_41; break;
        case ENCODING_AVC_LEVEL_42: hrdBufferSize = H264_MAX_CPB_LEVEL_42; break;
        default: hrdBufferSize = 0; /*error*/
        }
        break;

    case ENCODING_STD_MPEG2:
        switch(Level)
        {
        case ENCODING_MPEG2_LEVEL_LOW:  hrdBufferSize = MPEG2_MAX_VBV_LEVEL_LOW; break;
        case ENCODING_MPEG2_LEVEL_MAIN: hrdBufferSize = MPEG2_MAX_VBV_LEVEL_MAIN; break;
        case ENCODING_MPEG2_LEVEL_HIGH: hrdBufferSize = MPEG2_MAX_VBV_LEVEL_HIGH; break;
        default: hrdBufferSize = 0; /*error*/
        }
        break;


    case ENCODING_STD_VP8:
        hrdBufferSize = VP8_DUMMY_HRD_BUFFER_SIZE;
        break;


    default:
        hrdBufferSize = 0; /*error*/
        break;
    }

    if(hrdBufferSize == 0)
    {
#ifdef BCHP_CHIP
        BDBG_ASSERT(!"Unsupported Profile or Level");
#else
#if defined(CEB2APP) || defined (VICE_FW_CMODEL_APP)
        assert(!"Unsupported Profile or Level");
#else
        ASSERT(!"Unsupported Profile or Level");
#endif
#endif
    }

    return(hrdBufferSize);
}




#if defined(BCHP_CHIP) || defined(CEB2APP) || defined (VICE_FW_CMODEL_APP)

/***************************************************************************************************************************
*     FLOATING POINT Functions
****************************************************************************************************************************/

/****************************************************************
*  FPNumber_t RcNumToFP(uint32_t Number, uint8_t Bits)
*   convert integer Number to floating point representation with
*   "Bits" number of bits for mantissa
*****************************************************************/
static void RcNumToFP(FPNumber_t *outputFPNumber, uint32_t inputNumber, uint8_t numBits)
{

    outputFPNumber->mantissa=inputNumber;

    if(((int32_t)inputNumber) > 0)
    {
        NORM31(outputFPNumber->exponent, (int32_t) inputNumber);
    }
    else
    {
        outputFPNumber->exponent = -1;
    }

    if(numBits<= 32)
    {
        outputFPNumber->exponent -= (31 - numBits);
    }

    if(outputFPNumber->exponent < 0)
    {
        outputFPNumber->mantissa >>= (-outputFPNumber->exponent);
    }
    else
    {
        outputFPNumber->mantissa <<= outputFPNumber->exponent;
    }
}

/****************************************************************
*  RcFPToNum(FPNumber_t localFPNumber)
*   Round floating point number to 32 bit integer number
*****************************************************************/
static uint32_t RcFPToNum(FPNumber_t localFPNumber)
{
    uint32_t Number;

    if(localFPNumber.exponent > 0)
    {
        Number = localFPNumber.mantissa  >>  localFPNumber.exponent;
    }
    else
    {
        Number = localFPNumber.mantissa << (-localFPNumber.exponent);
    }
    return Number;
}

/**********************************************************************************
*  RcU32ScaleFP(FPNumber_t FP_operand, uint32_t scaleFactor, uint8_t bypassNormFlag)
**********************************************************************************/
static uint32_t RcU32ScaleFP(FPNumber_t FP_operand, uint32_t scaleFactor, uint8_t bypassNormFlag)
{
        uint32_t scaledMSW, scaledLSW, product;
        FPNumber_t FP_short;
    int8_t maxLeftShiftAllowedMinusOne;

    if(bypassNormFlag)
    {
        FP_short.mantissa = FP_operand.mantissa;
            FP_short.exponent = FP_operand.exponent;
    }
    else
    {
        RcNumToFP(&FP_short, FP_operand.mantissa, 16);
            FP_short.exponent += FP_operand.exponent;
    }

    /* multiply 16-bit mantissa with  32 bit unsigned integer */
    scaledMSW = UMULT16(scaleFactor>>16, FP_short.mantissa);
    scaledLSW = UMULT16(scaleFactor, FP_short.mantissa);
    if(FP_short.exponent < 16)
    {
        /* net left shift for MSW with unsigned 32-bit saturation */
        switch(scaledMSW & 0x80000000)
        {
        case 0:
            NORM31(maxLeftShiftAllowedMinusOne, (int32_t) scaledMSW);
            if(maxLeftShiftAllowedMinusOne >= ((16 - FP_short.exponent) - 1))
            {
                scaledMSW <<= (16 - FP_short.exponent);
                product = scaledMSW + (scaledLSW >> FP_short.exponent);

                if(product  >= scaledMSW)
                {
                    break;  /* no overflow */
                }
            }
        default:
            /* overflow */
            /* BitsPErTick  > 1 at bit rates above 27Mbps */
            product = UINT32_MAX;
            break;
        }
    }
    else
    {
        /* net (exponent - 16) right shift for MSW */
        product = (scaledMSW + (scaledLSW >> 16)) >> (FP_short.exponent - 16);
    }

        return(product);
}
/**********************************************************************************
*  NormalizeFP(FPNumber_t *outputFPNumber, FPNumber_t inputFPNumber, u8_t numBits)
*    Normalize any nput  Floating point Number to numBits for manitssa
**********************************************************************************/
static void NormalizeFP(FPNumber_t *pOutputFPNumber, FPNumber_t pInputFPNumber, uint8_t numBits)
{
    /* shift the number accordingly */
    RcNumToFP(pOutputFPNumber, pInputFPNumber.mantissa, numBits);
    pOutputFPNumber->exponent += pInputFPNumber.exponent;
}

/****************************************************************
* Utility to simulate hardware function used for rate control
*****************************************************************/
static int16_t arcDivide16(int16_t numerator, int16_t denominator)
{
    int16_t iteration, var_out;
    int32_t Lm, L_denom;


    var_out =  0;
    Lm = numerator << 15;

    L_denom = denominator << 15;

    for(iteration=0; iteration < 15; iteration++)
    {
        Lm <<= 1;
        if(Lm >= L_denom)
        {
            /*32 bit subtract*/
            Lm = Lm - L_denom;
            Lm = Lm + 1;
        }
    }

    /* remainder in MSW of Lm, quotient in LSW of Lm */
    var_out = (int16_t)(Lm & 0x7fff);
    return(var_out);
}

/************************************************************************
*  START OF DRAM BUFFER SIZE CALCULATIONS
************************************************************************/


#define DIV64_ROUND(x)   ( ((x) + 32) >> 6 )
#define DIV64_ROUNDUP(x) ( ((x) + 63) >> 6 )
#define DIV64_TRUNC(x)   ( (x) >> 6 )

#define DIV128_ROUND(x)   ( ((x) + 64) >> 7 )
#define DIV128_ROUNDUP(x) ( ((x) + 127) >> 7 )
#define DIV128_TRUNC(x)   ( (x) >> 7 )

#define DIV256_ROUND(x)   ( ((x) + 128) >> 8 )
#define DIV256_ROUNDUP(x) ( ((x) + 255) >> 8 )
#define DIV256_TRUNC(x)   ( (x) >> 8 )

#ifndef MAX
#define MAX(a,b)                (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)                (((a) < (b)) ? (a) : (b))
#endif


/************************************************************************
* Function: bvceEpmCalcWidthInStripes
*
* Actions: find number of stripes in a picture
*
* Params:
*        PictureWidthInPels: picture width in pels
*        DramStripeWidth:    Dram stripe width in bytes
*
* Returns:
*        Number of stripes in a picture width
*
************************************************************************/
static uint8_t bvceEpmCalcWidthInStripes(uint32_t PictureWidthInPels,uint32_t DramStripeWidth)
{
    uint8_t WidthInStripes;

    if (DramStripeWidth==128)
    {
        WidthInStripes = DIV128_ROUNDUP(PictureWidthInPels);
    }
    else /* DramStripeWidth==256 */
    {
	    WidthInStripes = DIV256_ROUNDUP(PictureWidthInPels);
    }

    return(WidthInStripes);

}


/************************************************************************
* Function: bvceEpmAlignJword
*
* Actions: align a number to Jword(unit of 256bits).
*
* Params:
*        UnalignedNumber
*
* Returns:
*        Aligned number
*
************************************************************************/
static uint32_t bvceEpmAlignJword(uint32_t UnalignedNumber)
{
    uint32_t AlignedNumber;
    AlignedNumber=(UnalignedNumber+31) & 0xffffffe0;
    return(AlignedNumber);
}



/************************************************************************
* Function: bvceEpmCalcNMBY
*
* Actions: find the number of macroblocs in stripe height
*
* Params:
*        PictureHeightInMbs: hight of picture in macroblock units
*
* Returns:
*        Number of macroblocks per stripe height
*
************************************************************************/
static uint8_t bvceEpmCalcNMBY(uint32_t PictureHeightInMbs, uint32_t X, uint32_t Y)
{

    uint8_t nmby,n;

    for(n=0; n<18; ++n)  /* Supports frame sizes up to 1088, need to increase for bigger sizes */
    {/* pay attention is was only > due to a bug from the architecture cmodel */
        if((X*(uint32_t)n+Y) >= PictureHeightInMbs)
            break;
    }

    nmby = X*n+Y;
    return(nmby);


}


/************************************************************************
* Function: EpmCalcDcxvNMBY
*
* Actions: find the number of macroblocs in stripe height
*
* Params:
*        PictureHeightInMbs: hight of picture in Mbs
*        IsDcxvBuf: 1-DCXV buff 0-none DCXV buff
*        IsInterlace: 1-Interlace  0-not interlace
*        IsChroma: 1- chroma buff 0 -luma buff
*
* Returns:
*        Number of macroblocks per stripe height
*
************************************************************************/
#define DCXV_STRIPE_WIDTH 64
#define DCXV_PADDED_LINE 2
#define DCXV_COMP_RATIO 2

#define DCXVLumaBuffer            0
#define DCXVChromaBuffer          1

#define DCXVBuffer                1
#define NonDCXVBuffer             0
#define DIV2_ROUND(x)    ( ((x) + 1) >> 1 )
#define DIV2_ROUNDUP(x)  ( ((x) + 1) >> 1 )
#define DIV2_TRUNC(x)    ( (x) >> 1 )

#define DIV4_ROUND(x)    ( ((x) + 2) >> 2 )
#define DIV4_ROUNDUP(x)  ( ((x) + 3) >> 2 )
#define DIV4_TRUNC(x)    ( (x) >> 2 )

#define DIV8_ROUND(x)    ( ((x) + 4) >> 3 )
#define DIV8_ROUNDUP(x)  ( ((x) + 7) >> 3 )
#define DIV8_TRUNC(x)    ( (x) >> 3 )

#define DIV16_ROUND(x)   ( ((x) + 8) >> 4 )
#define DIV16_ROUNDUP(x) ( ((x) + 15) >> 4 )
#define DIV16_TRUNC(x)   ( (x) >> 4 )

#define MULT16(x)       ( (x) << 4 )
#define MULT8(x)        ( (x) << 3 )




/************************************************************************
* Function: EpmCalcStripeBufferSize
*
* Actions: find striped format buffer size
*
* Params:
*        PictureWidthInMb, PictureHeightInMbs: picture width and height in mbs
*        IsDcxvBuff
*
* Returns:
*        buffer size
*
************************************************************************/
static uint32_t bvceEpmCalcStripeBufferSize(uint32_t PictureWidthInPels , uint32_t PictureHeightInPels , uint32_t IsDcxvBuf , uint32_t IsInterlace , uint32_t StripeWidth , uint32_t X , uint32_t Y)
{

    uint32_t BufferWidthInPels , BuffereHeightInPels;
    uint32_t BuffSize;
    uint32_t DcxvPaddingWidth , DcxvPaddingHeight;
    uint32_t Nmby;

    DcxvPaddingWidth = (( PictureWidthInPels + DCXV_STRIPE_WIDTH - 1)/DCXV_STRIPE_WIDTH) * DCXV_STRIPE_WIDTH - PictureWidthInPels;

    if ( IsInterlace == 1 )
    {
            DcxvPaddingHeight = DCXV_PADDED_LINE *2;
    }
    else
    {
        DcxvPaddingHeight = DCXV_PADDED_LINE;
    }

    if ( IsDcxvBuf == 1 )
    {
      BufferWidthInPels = PictureWidthInPels + DcxvPaddingWidth;
      BufferWidthInPels = BufferWidthInPels / DCXV_COMP_RATIO;
      BuffereHeightInPels = PictureHeightInPels + DcxvPaddingHeight;
    }
    else
    {
      BufferWidthInPels = PictureWidthInPels;
      BuffereHeightInPels = PictureHeightInPels;
    }


      Nmby = bvceEpmCalcNMBY(DIV16_ROUNDUP(BuffereHeightInPels) , X , Y);



    BuffSize = bvceEpmAlignJword( bvceEpmCalcWidthInStripes(BufferWidthInPels,StripeWidth) * StripeWidth * MULT16(Nmby) );
    BuffSize = ((BuffSize+4095) & 0xfffff000);



    return(BuffSize);

}



/* ==============   MEMORY CALCULATIONS   ================== */


#define EPM_IBBP_NUMBER_OF_ORIGINAL_PICTURE_QUEUES                      (5+1)
#define EPM_IBBP_NUMBER_OF_DECIMATED_PICTURE_QUEUES                     (8+1)
#define EPM_IBBP_NUMBER_OF_CSC_QUEUES                                   (3)
#define EPM_IBBP_NUMBER_OF_RECONSTRUCTED_PICTURE_QUEUES                 (5)

#define EPM_IP_NUMBER_OF_ORIGINAL_PICTURE_QUEUES_PROGRESSIVE            (4)
#define EPM_IP_NUMBER_OF_DECIMATED_PICTURE_QUEUES_PROGRESSIVE           (5)
#define EPM_IP_NUMBER_OF_CSC_QUEUES_PROGRESSIVE                         (3)
#define EPM_IP_NUMBER_OF_RECONSTRUCTED_PICTURE_QUEUES_PROGRESSIVE       (3)

#define EPM_IP_NUMBER_OF_ORIGINAL_PICTURE_QUEUES_INTERLACE              (4)
#define EPM_IP_NUMBER_OF_DECIMATED_PICTURE_QUEUES_INTERLACE             (4)
#define EPM_IP_NUMBER_OF_CSC_QUEUES_INTERLACE                           (3)
#define EPM_IP_NUMBER_OF_RECONSTRUCTED_PICTURE_QUEUES_INTERLACE         (3)

#define EPM_MAX_NUMBER_OF_ORIGINAL_PICTURE_QUEUES                       EPM_IBBP_NUMBER_OF_ORIGINAL_PICTURE_QUEUES
#define EPM_MAX_NUMBER_OF_DECIMATED_PICTURE_QUEUES                      EPM_IBBP_NUMBER_OF_DECIMATED_PICTURE_QUEUES
#define EPM_MAX_NUMBER_OF_CSC_QUEUES                                    EPM_IBBP_NUMBER_OF_CSC_QUEUES
#define EPM_MAX_NUMBER_OF_RECONSTRUCTED_PICTURE_QUEUES                  EPM_IBBP_NUMBER_OF_RECONSTRUCTED_PICTURE_QUEUES

#define EPM_MAX_NUMBER_OF_IBBP_ORIGINAL_PICTURE_QUEUES                  EPM_IBBP_NUMBER_OF_ORIGINAL_PICTURE_QUEUES
#define EPM_MAX_NUMBER_OF_IBBP_DECIMATED_PICTURE_QUEUES                 EPM_IBBP_NUMBER_OF_DECIMATED_PICTURE_QUEUES
#define EPM_MAX_NUMBER_OF_IBBP_CSC_QUEUES                               EPM_IBBP_NUMBER_OF_CSC_QUEUES
#define EPM_MAX_NUMBER_OF_IBBP_RECONSTRUCTED_PICTURE_QUEUES             EPM_IBBP_NUMBER_OF_RECONSTRUCTED_PICTURE_QUEUES

#define EPM_MAX_NUMBER_OF_IP_ORIGINAL_PICTURE_QUEUES                    EPM_IP_NUMBER_OF_ORIGINAL_PICTURE_QUEUES_PROGRESSIVE
#define EPM_MAX_NUMBER_OF_IP_DECIMATED_PICTURE_QUEUES                   EPM_IP_NUMBER_OF_DECIMATED_PICTURE_QUEUES_PROGRESSIVE
#define EPM_MAX_NUMBER_OF_IP_CSC_QUEUES                                 EPM_IP_NUMBER_OF_CSC_QUEUES_PROGRESSIVE
#define EPM_MAX_NUMBER_OF_IP_RECONSTRUCTED_PICTURE_QUEUES               EPM_IP_NUMBER_OF_RECONSTRUCTED_PICTURE_QUEUES_PROGRESSIVE

#define EPM_NUMBER_OF_ORIGINAL_PICTURE_DESCRIPTORS                      (EPM_MAX_NUMBER_OF_IBBP_ORIGINAL_PICTURE_QUEUES*2 + 1)

#define HOR_SIZE_IN_PELS_720P                                           1280
#define VER_SIZE_IN_PELS_720P                                           720

#define HOR_SIZE_IN_PELS_1080P                                           1920
#define VER_SIZE_IN_PELS_1080P                                           1088

#define HOR_SIZE_IN_PELS_PAL                                            720
#define VER_SIZE_IN_PELS_PAL                                            576

#define WORKAROUND_HW_ISSUE_VICE2_210_LOWER_LIMIT 61
#define WORKAROUND_HW_ISSUE_VICE2_210_UPPER_LIMIT 64

#define VP9_CTX_TABLE_SIZE                                              1888


/* Populates the default non-secure memory settings based on the specified core settings */
void BVCE_FW_P_GetDefaultNonSecureMemSettings( const BVCE_FW_P_CoreSettings_t *pstCoreSettings, BVCE_FW_P_NonSecureMemSettings_t *pstMemSettings )
{
    BSTD_UNUSED( pstCoreSettings );
    pstMemSettings->InputType = ENCODER_INPUT_TYPE_INTERLACED;
    pstMemSettings->MaxPictureWidthInPels = HOR_SIZE_IN_PELS_1080P;
    pstMemSettings->MaxPictureHeightInPels = VER_SIZE_IN_PELS_1080P;


    if ( pstCoreSettings->eVersion >= BVCE_FW_P_COREVERSION_V3_0_0_2)
    {
	uint32_t ProgressiveDramBuffSize;
	uint32_t InterlaceDramBuffSize;

        pstMemSettings->DramStripeWidth = 256;
        pstMemSettings->X = 32;
        pstMemSettings->Y = 12;
        pstMemSettings->DcxvEnable = 1;
	pstMemSettings->NewBvnMailboxEnable = 0;
	    pstMemSettings->PageSize = 2;

        pstMemSettings->InputType = ENCODER_INPUT_TYPE_INTERLACED;
		pstMemSettings->MaxGopStructure = ENCODING_GOP_STRUCT_IBBBP;

        InterlaceDramBuffSize= BVCE_FW_P_CalcNonSecureMem( pstCoreSettings, pstMemSettings );

        pstMemSettings->InputType = ENCODER_INPUT_TYPE_PROGRESSIVE;
		pstMemSettings->MaxGopStructure = ENCODING_GOP_STRUCT_IBBBBBBBP;

		ProgressiveDramBuffSize= BVCE_FW_P_CalcNonSecureMem( pstCoreSettings, pstMemSettings );

		if (InterlaceDramBuffSize > ProgressiveDramBuffSize)
		{
        pstMemSettings->InputType = ENCODER_INPUT_TYPE_INTERLACED;
		pstMemSettings->MaxGopStructure = ENCODING_GOP_STRUCT_IBBBP;
		}


    }
    else if ( pstCoreSettings->eVersion >= BVCE_FW_P_COREVERSION_V2_1)
    {
        pstMemSettings->DramStripeWidth = 256;
        pstMemSettings->X = 32;
        pstMemSettings->Y = 12;
        pstMemSettings->DcxvEnable = 1;
	pstMemSettings->NewBvnMailboxEnable = 0;
	pstMemSettings->PageSize = 2;
		pstMemSettings->MaxGopStructure = ENCODING_GOP_STRUCT_IBBP;
    }
    else
    {
        pstMemSettings->DramStripeWidth = 128;
        pstMemSettings->X = 16;
        pstMemSettings->Y = 6;
        pstMemSettings->DcxvEnable = 0;
	pstMemSettings->NewBvnMailboxEnable = 0;
	pstMemSettings->PageSize = 2;
		pstMemSettings->MaxGopStructure = ENCODING_GOP_STRUCT_IBBP;
    }
}

/************************************************************************
* Function: BVCE_FW_P_CalcNonSecureMemory
*
* Calculates the amount of non-secure memory that is necessary for ViCE2 FW
* to operate depending on various input parameters.
*
* Params:
*       InputType               - Input Type: ENCODER_INPUT_TYPE_INTERLACED or ENCODER_INPUT_TYPE_PROGRESSIVE
*       DramStripeWidth         - Stripe Width in bytes
*       MaxPictureWidthInPels   - Max picture H resolution allowed
*       MaxPictureHeightInPels  - Max picture V resolution allowed
*
* Returns:
*        Number of macroblocks per stripe height
*
************************************************************************/
static uint32_t BVCE_FW_P_CalcV3NonSecureMem ( const BVCE_FW_P_CoreSettings_t *pstCoreSettings, const BVCE_FW_P_NonSecureMemSettings_t *pstMemSettings );
uint32_t BVCE_FW_P_CalcNonSecureMem ( const BVCE_FW_P_CoreSettings_t *pstCoreSettings, const BVCE_FW_P_NonSecureMemSettings_t *pstMemSettings )
{

    uint8_t InputType;
    uint32_t DramStripeWidth;
    uint32_t X;
    uint32_t Y;
    uint32_t DcxvEnable;
    uint32_t MaxPictureWidthInPels;
    uint32_t MaxPictureHeightInPels;
    uint8_t  StackPointer;
    uint32_t BufferSize;
    uint32_t CurrAddress;

    uint32_t max_horizontal_size_in_pels,max_horizontal_size_in_mbs;
    uint32_t max_vertical_size_in_pels,max_vertical_size_in_mbs;

    uint32_t size_of_luma_buffer;
    uint32_t size_of_420_chroma_buffer;
    uint32_t size_of_shifted_422_chroma_buffer;


    uint32_t Non_dcxv_size_of_luma_buffer;
    uint32_t Non_dcxv_size_of_420_chroma_buffer;
    uint32_t Non_dcxv_BufferSize;


    uint32_t size_of_2h1v_luma_buffer;
    uint32_t size_of_2h2v_luma_buffer;

    uint32_t size_of_csc_buffer;

    uint32_t size_of_reconstructed_luma;
    uint32_t size_of_reconstructed_chroma;

    uint8_t EPM_NUMBER_OF_ORIGINAL_PICTURE_QUEUES;
    uint8_t EPM_NUMBER_OF_DECIMATED_PICTURE_QUEUES;
    uint8_t EPM_NUMBER_OF_CSC_QUEUES;
    uint8_t EPM_NUMBER_OF_RECONSTRUCTED_PICTURE_QUEUES;


	if ( pstCoreSettings->eVersion >= BVCE_FW_P_COREVERSION_V3_0_0_2)
	{
	uint32_t DramBuffSize;

	DramBuffSize = BVCE_FW_P_CalcV3NonSecureMem ( pstCoreSettings, pstMemSettings );
	return(DramBuffSize);
	}



    InputType = pstMemSettings->InputType;
    DramStripeWidth = pstMemSettings->DramStripeWidth;
    X = pstMemSettings->X;
    Y = pstMemSettings->Y;
    MaxPictureWidthInPels = pstMemSettings->MaxPictureWidthInPels;
    MaxPictureHeightInPels = pstMemSettings->MaxPictureHeightInPels;
    DcxvEnable = pstMemSettings->DcxvEnable;

    if ( pstCoreSettings->eVersion < BVCE_FW_P_COREVERSION_V2_1)
    {
        DcxvEnable = 0;
    }

    if (
        (
        ( pstCoreSettings->eVersion == BVCE_FW_P_COREVERSION_V2_1_0_3 ) || ( pstCoreSettings->eVersion == BVCE_FW_P_COREVERSION_V2_1_1_2 ) ||( pstCoreSettings->eVersion == BVCE_FW_P_COREVERSION_V2_1_2_2 )
        )&&
        ( DcxvEnable == 1 )
       )
    {
        max_horizontal_size_in_mbs  = DIV16_ROUNDUP(MaxPictureWidthInPels);

        if (
            ( (max_horizontal_size_in_mbs & 0x3) == 3 ) && ( pstCoreSettings->eVersion != BVCE_FW_P_COREVERSION_V2_1_2_2 )
           )
        {
            max_horizontal_size_in_mbs = max_horizontal_size_in_mbs +1;
            MaxPictureWidthInPels = (max_horizontal_size_in_mbs<<4);
        }

        max_horizontal_size_in_mbs  = DIV16_ROUNDUP(MaxPictureWidthInPels);
        if ((max_horizontal_size_in_mbs >= WORKAROUND_HW_ISSUE_VICE2_210_LOWER_LIMIT) && (max_horizontal_size_in_mbs <= WORKAROUND_HW_ISSUE_VICE2_210_UPPER_LIMIT) )
          {
            max_horizontal_size_in_mbs = WORKAROUND_HW_ISSUE_VICE2_210_UPPER_LIMIT + 1;
            MaxPictureWidthInPels = (max_horizontal_size_in_mbs<<4);
          }

    }



    /* for progressive */

    /* Initialize EPM stack of buffers */
    CurrAddress = 0;

    if( InputType == ENCODER_INPUT_TYPE_PROGRESSIVE )
    {
        max_horizontal_size_in_pels = MaxPictureWidthInPels;
        max_vertical_size_in_pels   = MaxPictureHeightInPels;

        EPM_NUMBER_OF_ORIGINAL_PICTURE_QUEUES=EPM_MAX_NUMBER_OF_IP_ORIGINAL_PICTURE_QUEUES;
        EPM_NUMBER_OF_DECIMATED_PICTURE_QUEUES=EPM_MAX_NUMBER_OF_IP_DECIMATED_PICTURE_QUEUES;
        EPM_NUMBER_OF_CSC_QUEUES=EPM_MAX_NUMBER_OF_IP_CSC_QUEUES;
        EPM_NUMBER_OF_RECONSTRUCTED_PICTURE_QUEUES=EPM_MAX_NUMBER_OF_IP_RECONSTRUCTED_PICTURE_QUEUES;

        max_horizontal_size_in_mbs=max_horizontal_size_in_pels>>4;
        max_vertical_size_in_mbs=max_vertical_size_in_pels>>4;


        /* DCXV compression is used for luma and chroma in the progressive case in Vice2v2 */
        size_of_luma_buffer = bvceEpmCalcStripeBufferSize( max_horizontal_size_in_pels , max_vertical_size_in_pels , DcxvEnable , InputType , DramStripeWidth , X , Y);
        size_of_420_chroma_buffer = bvceEpmCalcStripeBufferSize( max_horizontal_size_in_pels , DIV2_ROUND(max_vertical_size_in_pels)  , DcxvEnable , InputType , DramStripeWidth , X , Y);

        size_of_2h1v_luma_buffer = bvceEpmCalcStripeBufferSize( DIV2_ROUND(max_horizontal_size_in_pels) , max_vertical_size_in_pels , NonDCXVBuffer , InputType , DramStripeWidth , X , Y);
        size_of_2h2v_luma_buffer = bvceEpmCalcStripeBufferSize( DIV2_ROUND(max_horizontal_size_in_pels) , DIV2_ROUND(max_vertical_size_in_pels) , NonDCXVBuffer , InputType , DramStripeWidth , X , Y);

        if (( pstCoreSettings->eVersion > BVCE_FW_P_COREVERSION_V1 ) &&
        ( pstCoreSettings->eVersion < BVCE_FW_P_COREVERSION_V3_0_0_2 ))
        {
            if ( max_horizontal_size_in_pels > (HOR_SIZE_IN_PELS_1080P/2))
            {
                size_of_2h1v_luma_buffer = bvceEpmCalcStripeBufferSize( DIV4_ROUND(HOR_SIZE_IN_PELS_1080P) , max_vertical_size_in_pels , NonDCXVBuffer , InputType , DramStripeWidth , X , Y);
                size_of_2h2v_luma_buffer = bvceEpmCalcStripeBufferSize( DIV4_ROUND(HOR_SIZE_IN_PELS_1080P) , DIV2_ROUND(max_vertical_size_in_pels) , NonDCXVBuffer , InputType , DramStripeWidth , X , Y);
            }
        }
        size_of_csc_buffer=bvceEpmAlignJword(6*max_horizontal_size_in_mbs*max_vertical_size_in_mbs);
        size_of_csc_buffer = ((size_of_csc_buffer+4095) & 0xfffff000);

        size_of_reconstructed_luma = bvceEpmCalcStripeBufferSize( max_horizontal_size_in_pels , max_vertical_size_in_pels , NonDCXVBuffer , InputType , DramStripeWidth , X , Y);
        size_of_reconstructed_chroma = bvceEpmCalcStripeBufferSize( max_horizontal_size_in_pels , DIV2_ROUND(max_vertical_size_in_pels)  , NonDCXVBuffer , InputType , DramStripeWidth , X , Y);


        #define WORKAROUND_HW_ISSUE_VICE2_196_210_MAX_NON_DCXV_WIDTH 1024
        #define WORKAROUND_HW_ISSUE_VICE2_196_210_MAX_NON_DCXV_HEIGHT 576

        Non_dcxv_size_of_luma_buffer = 0;
        Non_dcxv_size_of_420_chroma_buffer = 0;
        Non_dcxv_BufferSize = 0;

        if (
                ( pstCoreSettings->eVersion == BVCE_FW_P_COREVERSION_V2_1_0_3 ) || ( pstCoreSettings->eVersion == BVCE_FW_P_COREVERSION_V2_1_1_2 ) ||( pstCoreSettings->eVersion == BVCE_FW_P_COREVERSION_V2_1_2_2 )
           )
           {
             Non_dcxv_size_of_luma_buffer = bvceEpmCalcStripeBufferSize( WORKAROUND_HW_ISSUE_VICE2_196_210_MAX_NON_DCXV_WIDTH , WORKAROUND_HW_ISSUE_VICE2_196_210_MAX_NON_DCXV_HEIGHT , 0 , InputType, DramStripeWidth , X , Y );
             Non_dcxv_size_of_420_chroma_buffer = bvceEpmCalcStripeBufferSize( WORKAROUND_HW_ISSUE_VICE2_196_210_MAX_NON_DCXV_WIDTH , DIV2_ROUND(WORKAROUND_HW_ISSUE_VICE2_196_210_MAX_NON_DCXV_HEIGHT) , 0 , InputType, DramStripeWidth , X , Y );
             Non_dcxv_BufferSize = Non_dcxv_size_of_luma_buffer + Non_dcxv_size_of_420_chroma_buffer;
           }

        /* Allocate Stack of buffers for the Original picture */
        BufferSize=size_of_luma_buffer+size_of_420_chroma_buffer;
        if (Non_dcxv_BufferSize > BufferSize)
          BufferSize = Non_dcxv_BufferSize;

        /*save start address and buffer size for on the fly address calculation */
        CurrAddress = ((CurrAddress+4095) & 0xfffff000);

        for (StackPointer=0; StackPointer<EPM_NUMBER_OF_ORIGINAL_PICTURE_QUEUES ; StackPointer++)
        {
            CurrAddress = ((CurrAddress+4095) & 0xfffff000);
            CurrAddress=CurrAddress+BufferSize;
        }

        /*Allocate stack of buffers for the Decimated picture*/
        BufferSize=size_of_2h1v_luma_buffer+size_of_2h2v_luma_buffer;

        /*save start address and buffer size for on the fly address calculation*/
        CurrAddress = ((CurrAddress+4095) & 0xfffff000);

        for (StackPointer=0; StackPointer<EPM_NUMBER_OF_DECIMATED_PICTURE_QUEUES ; StackPointer++)
        {
            CurrAddress = ((CurrAddress+4095) & 0xfffff000);
            CurrAddress=CurrAddress+BufferSize;
        }

        /*Allocated stack of buffers for the CSC*/
        BufferSize=size_of_csc_buffer*4;/*In progressive mode there are only two references*/

        /*save start address and buffer size for on the fly address calculation*/
        CurrAddress = ((CurrAddress+4095) & 0xfffff000);

        for (StackPointer=0; StackPointer<EPM_NUMBER_OF_CSC_QUEUES ; StackPointer++)
        {
            CurrAddress = ((CurrAddress+4095) & 0xfffff000);
            CurrAddress=CurrAddress+BufferSize;
        }

        /*Allocate stack of buffers for the Reconstructed*/
        BufferSize=size_of_reconstructed_luma+size_of_reconstructed_chroma;

        /*save start address and buffer size for on the fly address calculation*/
        CurrAddress = ((CurrAddress+4095) & 0xfffff000);

        for (StackPointer=0; StackPointer<EPM_NUMBER_OF_RECONSTRUCTED_PICTURE_QUEUES ; StackPointer++)
        {
            CurrAddress = ((CurrAddress+4095) & 0xfffff000);
            CurrAddress=CurrAddress+BufferSize;
        }

    /*allocate 720p buffers*/

    if (pstCoreSettings->eVersion == BVCE_FW_P_COREVERSION_V1)
    {
        /*max_horizontal_size_in_pels = MIN(HOR_SIZE_IN_PELS_720P,MaxPictureWidthInPels);*/
        max_horizontal_size_in_pels=HOR_SIZE_IN_PELS_720P;
        if (max_horizontal_size_in_pels>MaxPictureWidthInPels)
            {
                max_horizontal_size_in_pels=MaxPictureWidthInPels;
            }

        /*max_vertical_size_in_pels   = MIN(VER_SIZE_IN_PELS_720P,MaxPictureHeightInPels);*/
        max_vertical_size_in_pels=VER_SIZE_IN_PELS_720P;
        if (max_vertical_size_in_pels>MaxPictureHeightInPels)
        {
            max_vertical_size_in_pels=MaxPictureHeightInPels;
        }
    }
    else
    {
        max_horizontal_size_in_pels=MaxPictureWidthInPels;
        max_vertical_size_in_pels=MaxPictureHeightInPels;
    }

        EPM_NUMBER_OF_ORIGINAL_PICTURE_QUEUES=EPM_MAX_NUMBER_OF_IBBP_ORIGINAL_PICTURE_QUEUES-EPM_MAX_NUMBER_OF_IP_ORIGINAL_PICTURE_QUEUES;
        EPM_NUMBER_OF_DECIMATED_PICTURE_QUEUES=EPM_MAX_NUMBER_OF_IBBP_DECIMATED_PICTURE_QUEUES-EPM_MAX_NUMBER_OF_IP_DECIMATED_PICTURE_QUEUES;



        max_horizontal_size_in_mbs=max_horizontal_size_in_pels>>4;
        max_vertical_size_in_mbs=max_vertical_size_in_pels>>4;


        /* DCXV compression is used for luma and chroma in the progressive case in Vice2v2 */
        size_of_luma_buffer = bvceEpmCalcStripeBufferSize( max_horizontal_size_in_pels , max_vertical_size_in_pels , DcxvEnable , InputType , DramStripeWidth , X , Y);
        size_of_420_chroma_buffer = bvceEpmCalcStripeBufferSize( max_horizontal_size_in_pels , DIV2_ROUND(max_vertical_size_in_pels)  , DcxvEnable , InputType , DramStripeWidth , X , Y);

        size_of_2h1v_luma_buffer = bvceEpmCalcStripeBufferSize( DIV2_ROUND(max_horizontal_size_in_pels) , max_vertical_size_in_pels , NonDCXVBuffer , InputType , DramStripeWidth , X , Y);
        size_of_2h2v_luma_buffer = bvceEpmCalcStripeBufferSize( DIV2_ROUND(max_horizontal_size_in_pels) , DIV2_ROUND(max_vertical_size_in_pels) , NonDCXVBuffer , InputType , DramStripeWidth , X , Y);

        if (( pstCoreSettings->eVersion > BVCE_FW_P_COREVERSION_V1 ) &&
        ( pstCoreSettings->eVersion < BVCE_FW_P_COREVERSION_V3_0_0_2 ))
        {
            if ( max_horizontal_size_in_pels > (HOR_SIZE_IN_PELS_1080P/2))
            {
                size_of_2h1v_luma_buffer = bvceEpmCalcStripeBufferSize( DIV4_ROUND(HOR_SIZE_IN_PELS_1080P) , max_vertical_size_in_pels , NonDCXVBuffer , InputType , DramStripeWidth , X , Y);
                size_of_2h2v_luma_buffer = bvceEpmCalcStripeBufferSize( DIV4_ROUND(HOR_SIZE_IN_PELS_1080P) , DIV2_ROUND(max_vertical_size_in_pels) , NonDCXVBuffer , InputType , DramStripeWidth , X , Y);
            }
        }

        size_of_csc_buffer=bvceEpmAlignJword(6*max_horizontal_size_in_mbs*max_vertical_size_in_mbs);
        size_of_csc_buffer = ((size_of_csc_buffer+4095) & 0xfffff000);

        size_of_reconstructed_luma = bvceEpmCalcStripeBufferSize( max_horizontal_size_in_pels , max_vertical_size_in_pels , NonDCXVBuffer , InputType , DramStripeWidth , X , Y);
        size_of_reconstructed_chroma = bvceEpmCalcStripeBufferSize( max_horizontal_size_in_pels , DIV2_ROUND(max_vertical_size_in_pels)  , NonDCXVBuffer , InputType , DramStripeWidth , X , Y);



        /*Allocate Stack of buffers for the Original picture*/
        BufferSize=size_of_luma_buffer+size_of_420_chroma_buffer;
        if (Non_dcxv_BufferSize > BufferSize)
          BufferSize = Non_dcxv_BufferSize;



        /*save start address and buffer size for on the fly address calculation*/
        CurrAddress = ((CurrAddress+4095) & 0xfffff000);

        for (StackPointer=0; StackPointer<EPM_NUMBER_OF_ORIGINAL_PICTURE_QUEUES ; StackPointer++)
        {
            CurrAddress = ((CurrAddress+4095) & 0xfffff000);
            CurrAddress=CurrAddress+BufferSize;
        }

        /*Allocate stack of buffers for the Decimated picture*/
        BufferSize=size_of_2h1v_luma_buffer+size_of_2h2v_luma_buffer;

        /*save start address and buffer size for on the fly address calculation*/
        CurrAddress = ((CurrAddress+4095) & 0xfffff000);

        for (StackPointer=0; StackPointer<EPM_NUMBER_OF_DECIMATED_PICTURE_QUEUES ; StackPointer++)
        {
            CurrAddress = ((CurrAddress+4095) & 0xfffff000);
            CurrAddress=CurrAddress+BufferSize;
        }



        return(CurrAddress);
    }

    /*Interlace*/
    else
    {

        /*Initialize EPM stack of buffers*/
        CurrAddress=0;

        /*fill HD buffers information*/
        max_horizontal_size_in_pels = MaxPictureWidthInPels;
        max_vertical_size_in_pels   = MaxPictureHeightInPels;


        EPM_NUMBER_OF_ORIGINAL_PICTURE_QUEUES=EPM_MAX_NUMBER_OF_IP_ORIGINAL_PICTURE_QUEUES;
        EPM_NUMBER_OF_DECIMATED_PICTURE_QUEUES=EPM_MAX_NUMBER_OF_IP_DECIMATED_PICTURE_QUEUES;
        EPM_NUMBER_OF_CSC_QUEUES=EPM_MAX_NUMBER_OF_IP_CSC_QUEUES;
        EPM_NUMBER_OF_RECONSTRUCTED_PICTURE_QUEUES=EPM_MAX_NUMBER_OF_IP_RECONSTRUCTED_PICTURE_QUEUES;

        max_horizontal_size_in_mbs=max_horizontal_size_in_pels>>4;
        max_vertical_size_in_mbs=max_vertical_size_in_pels>>4;



        /* DCXV compression is used for luma and chroma in the progressive case in Vice2v2 */
        size_of_luma_buffer = bvceEpmCalcStripeBufferSize( max_horizontal_size_in_pels , max_vertical_size_in_pels , DcxvEnable , InputType , DramStripeWidth , X , Y);
        size_of_420_chroma_buffer = bvceEpmCalcStripeBufferSize( max_horizontal_size_in_pels , DIV2_ROUND(max_vertical_size_in_pels)  , NonDCXVBuffer , InputType , DramStripeWidth , X , Y);
        size_of_shifted_422_chroma_buffer = size_of_420_chroma_buffer;

        size_of_2h1v_luma_buffer = bvceEpmCalcStripeBufferSize( DIV2_ROUND(max_horizontal_size_in_pels) , max_vertical_size_in_pels , NonDCXVBuffer , InputType , DramStripeWidth , X , Y);
        size_of_2h2v_luma_buffer = bvceEpmCalcStripeBufferSize( DIV2_ROUND(max_horizontal_size_in_pels) , DIV2_ROUND(max_vertical_size_in_pels) , NonDCXVBuffer , InputType , DramStripeWidth , X , Y);

        if (( pstCoreSettings->eVersion > BVCE_FW_P_COREVERSION_V1 ) &&
        ( pstCoreSettings->eVersion < BVCE_FW_P_COREVERSION_V3_0_0_2 ))
        {
            if ( max_horizontal_size_in_pels > (HOR_SIZE_IN_PELS_1080P/2))
            {
                size_of_2h1v_luma_buffer = bvceEpmCalcStripeBufferSize( DIV4_ROUND(HOR_SIZE_IN_PELS_1080P) , max_vertical_size_in_pels , NonDCXVBuffer , InputType , DramStripeWidth , X , Y);
                size_of_2h2v_luma_buffer = bvceEpmCalcStripeBufferSize( DIV4_ROUND(HOR_SIZE_IN_PELS_1080P) , DIV2_ROUND(max_vertical_size_in_pels) , NonDCXVBuffer , InputType , DramStripeWidth , X , Y);
            }
        }

        size_of_csc_buffer=bvceEpmAlignJword(6*max_horizontal_size_in_mbs*max_vertical_size_in_mbs);
        size_of_csc_buffer = ((size_of_csc_buffer+4095) & 0xfffff000);

        size_of_reconstructed_luma = bvceEpmCalcStripeBufferSize( max_horizontal_size_in_pels , max_vertical_size_in_pels , NonDCXVBuffer , InputType , DramStripeWidth , X , Y);
        size_of_reconstructed_chroma = bvceEpmCalcStripeBufferSize( max_horizontal_size_in_pels , DIV2_ROUND(max_vertical_size_in_pels)  , NonDCXVBuffer , InputType , DramStripeWidth , X , Y);



        /*Allocate Stack of buffers for the Original picture*/
        BufferSize=size_of_luma_buffer+size_of_420_chroma_buffer;

        /*save start address and buffer size for on the fly address calculation*/
        CurrAddress = ((CurrAddress+4095) & 0xfffff000);


        for (StackPointer=0; StackPointer<(EPM_NUMBER_OF_ORIGINAL_PICTURE_QUEUES<<1) ; StackPointer+=2)
        {
            CurrAddress = ((CurrAddress+4095) & 0xfffff000);
            CurrAddress=CurrAddress+BufferSize;
        }

        /* Allocate Stack of buffers for the Original Shifted 4:2:2 top field Chroma */
        BufferSize=size_of_shifted_422_chroma_buffer;

        /* save start address and buffer size for on the fly address calculation */
        CurrAddress = ((CurrAddress+4095) & 0xfffff000);

        for (StackPointer=0; StackPointer<(EPM_NUMBER_OF_ORIGINAL_PICTURE_QUEUES) ; StackPointer+=1)
        {
            CurrAddress = ((CurrAddress+4095) & 0xfffff000);
            CurrAddress=CurrAddress+BufferSize;
        }

        /* Allocate stack of buffers for the Decimated picture */
        BufferSize=size_of_2h1v_luma_buffer+size_of_2h2v_luma_buffer;

        /*save start address and buffer size for on the fly address calculation*/
        CurrAddress = ((CurrAddress+4095) & 0xfffff000);

        for (StackPointer=0; StackPointer<(EPM_NUMBER_OF_DECIMATED_PICTURE_QUEUES<<1) ; StackPointer+=2)
        {
            CurrAddress = ((CurrAddress+4095) & 0xfffff000);
            CurrAddress=CurrAddress+BufferSize;
        }

        /*Allocated stack of buffers for the CSC*/
        BufferSize=size_of_csc_buffer*4;/*In progressive mode there are only two references*/

        /*save start address and buffer size for on the fly address calculation*/
        CurrAddress = ((CurrAddress+4095) & 0xfffff000);

        for (StackPointer=0; StackPointer<EPM_NUMBER_OF_CSC_QUEUES ; StackPointer++)
        {
            CurrAddress = ((CurrAddress+4095) & 0xfffff000);
            CurrAddress=CurrAddress+BufferSize;
        }

        /*Allocate stack of buffers for the Reconstructed*/
        BufferSize=size_of_reconstructed_luma+size_of_reconstructed_chroma;

        /*save start address and buffer size for on the fly address calculation*/
        CurrAddress = ((CurrAddress+4095) & 0xfffff000);

        for (StackPointer=0; StackPointer<EPM_NUMBER_OF_RECONSTRUCTED_PICTURE_QUEUES ; StackPointer++)
        {
            CurrAddress = ((CurrAddress+4095) & 0xfffff000);
            CurrAddress=CurrAddress+BufferSize;
        }

        /*allocate SD buffers*/
        EPM_NUMBER_OF_ORIGINAL_PICTURE_QUEUES=EPM_MAX_NUMBER_OF_IBBP_ORIGINAL_PICTURE_QUEUES-EPM_MAX_NUMBER_OF_IP_ORIGINAL_PICTURE_QUEUES;
        EPM_NUMBER_OF_DECIMATED_PICTURE_QUEUES=EPM_MAX_NUMBER_OF_IBBP_DECIMATED_PICTURE_QUEUES-EPM_MAX_NUMBER_OF_IP_DECIMATED_PICTURE_QUEUES;


        if (pstCoreSettings->eVersion == BVCE_FW_P_COREVERSION_V1 )
        {
         max_horizontal_size_in_pels=HOR_SIZE_IN_PELS_PAL;
         if (max_horizontal_size_in_pels>MaxPictureWidthInPels)
         {
             max_horizontal_size_in_pels=MaxPictureWidthInPels;
         }

         max_vertical_size_in_pels   = VER_SIZE_IN_PELS_PAL;
         if (max_vertical_size_in_pels>MaxPictureHeightInPels)
         {
             max_vertical_size_in_pels=MaxPictureHeightInPels;
         }
        }
        else
        {
            max_horizontal_size_in_pels=MaxPictureWidthInPels;
            max_vertical_size_in_pels=MaxPictureHeightInPels;
        }

        max_horizontal_size_in_mbs=max_horizontal_size_in_pels>>4;
        max_vertical_size_in_mbs=max_vertical_size_in_pels>>4;

        /* DCXV compression is used for luma and chroma in the progressive case in Vice2v2 */
        size_of_luma_buffer = bvceEpmCalcStripeBufferSize( max_horizontal_size_in_pels , max_vertical_size_in_pels , DcxvEnable , InputType , DramStripeWidth , X , Y);
        size_of_420_chroma_buffer = bvceEpmCalcStripeBufferSize( max_horizontal_size_in_pels , DIV2_ROUND(max_vertical_size_in_pels)  , NonDCXVBuffer , InputType , DramStripeWidth , X , Y);
        size_of_shifted_422_chroma_buffer = size_of_420_chroma_buffer;

        size_of_2h1v_luma_buffer = bvceEpmCalcStripeBufferSize( DIV2_ROUND(max_horizontal_size_in_pels) , max_vertical_size_in_pels , NonDCXVBuffer , InputType , DramStripeWidth , X , Y);
        size_of_2h2v_luma_buffer = bvceEpmCalcStripeBufferSize( DIV2_ROUND(max_horizontal_size_in_pels) , DIV2_ROUND(max_vertical_size_in_pels) , NonDCXVBuffer , InputType , DramStripeWidth , X , Y);

        if (( pstCoreSettings->eVersion > BVCE_FW_P_COREVERSION_V1 ) &&
        ( pstCoreSettings->eVersion < BVCE_FW_P_COREVERSION_V3_0_0_2 ))
        {
            if ( max_horizontal_size_in_pels > (HOR_SIZE_IN_PELS_1080P/2))
            {
                size_of_2h1v_luma_buffer = bvceEpmCalcStripeBufferSize( DIV4_ROUND(HOR_SIZE_IN_PELS_1080P) , max_vertical_size_in_pels , NonDCXVBuffer , InputType , DramStripeWidth , X , Y);
                size_of_2h2v_luma_buffer = bvceEpmCalcStripeBufferSize( DIV4_ROUND(HOR_SIZE_IN_PELS_1080P) , DIV2_ROUND(max_vertical_size_in_pels) , NonDCXVBuffer , InputType , DramStripeWidth , X , Y);
            }
        }

        size_of_csc_buffer=bvceEpmAlignJword(6*max_horizontal_size_in_mbs*max_vertical_size_in_mbs);
        size_of_csc_buffer = ((size_of_csc_buffer+4095) & 0xfffff000);

        size_of_reconstructed_luma = bvceEpmCalcStripeBufferSize( max_horizontal_size_in_pels , max_vertical_size_in_pels , NonDCXVBuffer , InputType , DramStripeWidth , X , Y);
        size_of_reconstructed_chroma = bvceEpmCalcStripeBufferSize( max_horizontal_size_in_pels , DIV2_ROUND(max_vertical_size_in_pels)  , NonDCXVBuffer , InputType , DramStripeWidth , X , Y);



        /*Allocate Stack of buffers for the Original picture*/
        BufferSize=size_of_luma_buffer+size_of_420_chroma_buffer;

        /*save start address and buffer size for on the fly address calculation*/
        CurrAddress = ((CurrAddress+4095) & 0xfffff000);

        for (StackPointer=0; StackPointer<(EPM_NUMBER_OF_ORIGINAL_PICTURE_QUEUES<<1) ; StackPointer+=2)
        {
            CurrAddress = ((CurrAddress+4095) & 0xfffff000);
            CurrAddress=CurrAddress+BufferSize;
        }

        /*Allocate Stack of buffers for the Original Shifted 4:2:2 top field Chroma*/
        BufferSize=size_of_shifted_422_chroma_buffer;

        /*save start address and buffer size for on the fly address calculation*/
        CurrAddress = ((CurrAddress+4095) & 0xfffff000);

        for (StackPointer=0; StackPointer<(EPM_NUMBER_OF_ORIGINAL_PICTURE_QUEUES) ; StackPointer+=1)
        {
            CurrAddress = ((CurrAddress+4095) & 0xfffff000);
            CurrAddress=CurrAddress+BufferSize;
        }

        /*Allocate stack of buffers for the Decimated picture*/
        BufferSize=size_of_2h1v_luma_buffer+size_of_2h2v_luma_buffer;

        /*save start address and buffer size for on the fly address calculation*/
        CurrAddress = ((CurrAddress+4095) & 0xfffff000);

        for (StackPointer=0; StackPointer<(EPM_NUMBER_OF_DECIMATED_PICTURE_QUEUES<<1) ; StackPointer+=2)
        {
            CurrAddress = ((CurrAddress+4095) & 0xfffff000);
            CurrAddress=CurrAddress+BufferSize;
        }


         return(CurrAddress);
       }
}

/************************************************************************
* Function: BVCE_FW_P_CalcV3NonSecureMem
*
* Calculates the amount of non-secure memory that is necessary for ViCE2 FW
* to operate depending on various input parameters.
*
* Params:
*       InputType               - Input Type: ENCODER_INPUT_TYPE_INTERLACED or ENCODER_INPUT_TYPE_PROGRESSIVE
*       DramStripeWidth         - Stripe Width in bytes
*       MaxPictureWidthInPels   - Max picture H resolution allowed
*       MaxPictureHeightInPels  - Max picture V resolution allowed
*
* Returns:
*        Number of macroblocks per stripe height
*
************************************************************************/
#define MASK64_52MSB(x) (((((x) >> 32) & 0xFFFFFFFF) << 32) | ((x) & 0xFFFFF000))
#define ALIGN12BIT64(x) MASK64_52MSB(x+4095)
#define ALIGN12BIT32(x) ((x+4095) & 0xFFFFF000)
#define MAX_1H1V_WIDTH  640
#define MAX_1H1V_HEIGHT 360

#define V3_EPM_MAX_NUMBER_OF_CSC_QUEUES                    (4) /* 4 */
#define V3_EPM_MAX_NUMBER_OF_RECONSTRUCTED_PICTURE_QUEUES  (4) /* 4 */

#define V3_INFINITE_IP_EPM_MAX_NUMBER_OF_RECONSTRUCTED_PICTURE_QUEUES  (3)
#define V3_IP_EPM_MAX_NUMBER_OF_RECONSTRUCTED_PICTURE_QUEUES           (3)
#define V3_IBP_EPM_MAX_NUMBER_OF_RECONSTRUCTED_PICTURE_QUEUES          (3)
#define V3_IBBP_EPM_MAX_NUMBER_OF_RECONSTRUCTED_PICTURE_QUEUES         (3)
#define V3_IBBP_B_REF_EPM_MAX_NUMBER_OF_RECONSTRUCTED_PICTURE_QUEUES   (3)
#define V3_IBBBP_EPM_MAX_NUMBER_OF_RECONSTRUCTED_PICTURE_QUEUES        (3)
#define V3_I5BP_EPM_MAX_NUMBER_OF_RECONSTRUCTED_PICTURE_QUEUES         (4)
#define V3_I7BP_EPM_MAX_NUMBER_OF_RECONSTRUCTED_PICTURE_QUEUES         (4)


#define PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_PROGRESSIVE           12
#define PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_PROGRESSIVE                      14

#define PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_INTERLACE             (8)*2
#define PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_SHIFTED_CHROMA_INTERLACE          (8)
#define PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_INTERLACE                        (10)*2

#define PREPROCESSOR_PIC_QUEUE_SIZE 4
#endif /* defined(BCHP_CHIP) || defined(CEB2APP) || defined(VICE_FW_CMODEL_APP) */

/* Progressive */
#define    	IP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_PROGRESSIVE    5
#define		IP_PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_PROGRESSIVE               7

#define		I1BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_PROGRESSIVE  6
#define		I1BP_PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_PROGRESSIVE             8

#define		I2BP_NO_B_REF_B_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_PROGRESSIVE 7
#define		I2BP_NO_B_REF_B_PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_PROGRESSIVE            9

#define		I2BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_PROGRESSIVE   7
#define		I2BP_PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_PROGRESSIVE              9

#define		I3BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_PROGRESSIVE   8
#define		I3BP_PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_PROGRESSIVE              10

#define		I5BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_PROGRESSIVE   10
#define		I5BP_PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_PROGRESSIVE              12

#define		I7BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_PROGRESSIVE   12
#define		I7BP_PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_PROGRESSIVE              14


/* Interlace */
#define    	IP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_INTERLACE     (5)*2
#define    	IP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_SHIFTED_CHROMA_INTERLACE  (5)
#define    	IP_PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_INTERLACE                (7)*2

#define		I1BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_INTERLACE    (6)*2
#define		I1BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_SHIFTED_CHROMA_INTERLACE (6)
#define		I1BP_PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_INTERLACE               (8)*2

#define		I2BP_NO_B_REF_B_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_INTERLACE    (7)*2
#define		I2BP_NO_B_REF_B_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_SHIFTED_CHROMA_INTERLACE (7)
#define		I2BP_NO_B_REF_B_PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_INTERLACE               (9)*2

#define		I2BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_INTERLACE                (7)*2
#define		I2BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_SHIFTED_CHROMA_INTERLACE             (7)
#define		I2BP_PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_INTERLACE                           (9)*2

#define		I3BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_INTERLACE                (8)*2
#define		I3BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_SHIFTED_CHROMA_INTERLACE             (8)
#define		I3BP_PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_INTERLACE                           (10)*2


#if defined(BCHP_CHIP) || defined(CEB2APP) || defined (VICE_FW_CMODEL_APP)

#if PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_PROGRESSIVE > PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_INTERLACE
#define PREPROCESSOR_MAX_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_PROGRESSIVE
#else
#define PREPROCESSOR_MAX_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_INTERLACE
#endif

#if PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_PROGRESSIVE > PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_INTERLACE
#define PREPROCESSOR_MAX_NUMBER_OF_DECIMATED_PICTURE_BUFF PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_PROGRESSIVE
#else
#define PREPROCESSOR_MAX_NUMBER_OF_DECIMATED_PICTURE_BUFF PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_INTERLACE
#endif

static uint64_t bvceV3EpmAlignJword(uint64_t UnalignedNumber);
static uint8_t bvceV3EpmCalcNMBY(uint32_t PictureHeightInMbs, uint32_t X, uint32_t Y, uint32_t DramConfigPageSize, uint16_t StripeWidth);
/*
static uint8_t bvceV3EpmCalcDcxvNMBY(uint32_t PictureHeightInMbs, uint32_t IsDcxvBuf, uint32_t IsInterlace, uint32_t IsChroma, uint32_t X, uint32_t Y, uint32_t DramConfigPageSize, uint16_t StripeWidth);
*/
static uint32_t bvceV3EpmCalcStripeBufferSize(uint32_t PictureWidthInPels, uint32_t PictureHeightInPels, uint32_t IsDcxvBuf, uint32_t IsInterlace, uint32_t StripeWidth, uint32_t X, uint32_t Y, uint8_t DramConfigPageSize);


uint32_t BVCE_FW_P_CalcV3NonSecureMem ( const BVCE_FW_P_CoreSettings_t *pstCoreSettings, const BVCE_FW_P_NonSecureMemSettings_t *pstMemSettings )
{
uint64_t CurrAddr;
uint32_t StripeWidth;
uint32_t MaxWidthInPel;
uint32_t MaxHeightInPel;
uint8_t IsProgressive;

uint16_t MaxWidthInMbs, MaxHeightInMbs;
uint32_t CscBuffSize;
uint32_t ReconstructedLumaBuffSize;
uint32_t ReconstructedChromaBuffSize;
uint32_t BuffSize;
uint32_t CollocatedBuffSize;

uint16_t codedHeight;
uint16_t mvHeight;
uint32_t PictureWidthInMBs;
uint32_t tmvpPitch;


uint8_t CscFreeBuffersQueueMaxNumOfRefs;
uint8_t CscFreeBuffersQueueNumOfBuff;
uint8_t WrPtr;
uint8_t ReconstructedFreeBuffersQueueNumOfBuff;
uint8_t CollocatedFreeBuffersQueueNumOfBuff;

uint8_t DramConfigPageSize;

uint8_t DcxvEnable;
uint32_t X,Y;

	DramConfigPageSize = pstMemSettings->PageSize -1;
	CurrAddr = 0x2000; /* add 4k due to alignment */
	IsProgressive = !pstMemSettings->InputType;
	StripeWidth = pstMemSettings->DramStripeWidth;
    X = pstMemSettings->X;
    Y = pstMemSettings->Y;
    MaxWidthInPel = pstMemSettings->MaxPictureWidthInPels;
    MaxHeightInPel = pstMemSettings->MaxPictureHeightInPels;
    DcxvEnable = pstMemSettings->DcxvEnable;


		MaxWidthInMbs = DIV16_ROUNDUP(MaxWidthInPel);
		MaxHeightInMbs = DIV16_ROUNDUP(MaxHeightInPel);

		CscBuffSize = 4 * MaxWidthInMbs*MaxHeightInMbs;
		CscBuffSize = ALIGN12BIT32(CscBuffSize);



		ReconstructedLumaBuffSize = bvceV3EpmCalcStripeBufferSize(MaxWidthInPel , MaxHeightInPel , NonDCXVBuffer , !IsProgressive , StripeWidth , X , Y, DramConfigPageSize);
		ReconstructedLumaBuffSize = ALIGN12BIT32(ReconstructedLumaBuffSize);
		ReconstructedChromaBuffSize =         bvceV3EpmCalcStripeBufferSize(MaxWidthInPel , DIV2_ROUND(MaxHeightInPel) , NonDCXVBuffer , !IsProgressive , StripeWidth , X , Y, DramConfigPageSize);
		ReconstructedChromaBuffSize = ALIGN12BIT32(ReconstructedChromaBuffSize);

		/* Allocated stack of buffers for the CSC */

		CscFreeBuffersQueueMaxNumOfRefs = (2 << (!IsProgressive)); /* 4 CME refs for interlace. 2 for progressive */

		BuffSize = CscBuffSize*CscFreeBuffersQueueMaxNumOfRefs;

		/* save start address and buffer size for on the fly address calculation */

		CurrAddr = ALIGN12BIT64(CurrAddr);

		CscFreeBuffersQueueNumOfBuff = V3_EPM_MAX_NUMBER_OF_CSC_QUEUES;

		for (WrPtr = 0; WrPtr < CscFreeBuffersQueueNumOfBuff; WrPtr++)
		{
			CurrAddr = ALIGN12BIT64(CurrAddr);
			CurrAddr = CurrAddr + BuffSize;
		}


		/* Allocate stack of buffers for the Reconstructed */

		CurrAddr = ALIGN12BIT64(CurrAddr);
		BuffSize = ReconstructedLumaBuffSize + ReconstructedChromaBuffSize;
		if (
		    (pstMemSettings->MaxGopStructure == ENCODING_GOP_STRUCT_IBBBBBP) ||
		    (pstMemSettings->MaxGopStructure == ENCODING_GOP_STRUCT_IBBBBBBBP)
			)
			ReconstructedFreeBuffersQueueNumOfBuff = V3_I5BP_EPM_MAX_NUMBER_OF_RECONSTRUCTED_PICTURE_QUEUES;
		else
			ReconstructedFreeBuffersQueueNumOfBuff = V3_IBBBP_EPM_MAX_NUMBER_OF_RECONSTRUCTED_PICTURE_QUEUES;



		for (WrPtr = 0; WrPtr < ReconstructedFreeBuffersQueueNumOfBuff; WrPtr++)
		{
			CurrAddr = ALIGN12BIT64(CurrAddr);
			CurrAddr = CurrAddr + BuffSize;
		}

		/* allocate collocated buffer */

		CurrAddr = ALIGN12BIT64(CurrAddr);
		CollocatedFreeBuffersQueueNumOfBuff = ReconstructedFreeBuffersQueueNumOfBuff;

		codedHeight = (MaxHeightInPel + 63) & 0xFFC0;
		mvHeight = codedHeight / 8;
		PictureWidthInMBs = DIV16_ROUNDUP(MaxWidthInPel);
		tmvpPitch = (12 * ((PictureWidthInMBs + 3) >> 2));
		CollocatedBuffSize = (32 * tmvpPitch)*mvHeight;

		CollocatedBuffSize = ALIGN12BIT32(CollocatedBuffSize);
		CurrAddr = CurrAddr + CollocatedBuffSize * CollocatedFreeBuffersQueueNumOfBuff;

		CurrAddr = ALIGN12BIT64(CurrAddr);


		if (!pstMemSettings->NewBvnMailboxEnable)
		{


uint8_t Ptr;

uint32_t LumaBuffSize;
uint32_t ChromaBuffSize;
uint32_t Decimated1h1vBufferSize;
uint32_t Decimated1h2vBufferSize;
uint32_t Decimated2h1vBufferSize;
uint32_t Decimated2h2vBufferSize;

uint32_t Decimated1vBufferSize;
uint32_t Decimated2vBufferSize;

uint8_t FreeBuffersQueueORIGINAL_LUMANumOfBuff;
uint8_t FreeBuffersQueueORIGINAL_CHROMANumOfBuff;
uint8_t FreeBuffersQueueORIGINAL_SHIFTED_CHROMANumOfBuff;
uint8_t FreeBuffersQueueDECIMATED_1VNumOfBuff;
uint8_t FreeBuffersQueueDECIMATED_2VNumOfBuff;

uint8_t FreeBuffersQueueORIGINAL_LUMADcxvEn;
uint8_t FreeBuffersQueueORIGINAL_CHROMADcxvEn;
uint8_t FreeBuffersQueueDECIMATED_1VDcxvEn;
uint8_t FreeBuffersQueueDECIMATED_2VDcxvEn;

uint8_t NumberOfOriginalPictureBuff;
uint8_t NumberOfDecimatedPictureBuff;
uint8_t NumberOfShiftedChromaPictureBuff;

		if (IsProgressive == 1)
		{

		switch (pstMemSettings->MaxGopStructure)
		{
		case	ENCODING_GOP_STRUCT_I:
		case	ENCODING_GOP_STRUCT_IP:
		case	ENCODING_GOP_STRUCT_INFINITE_IP:
			NumberOfOriginalPictureBuff = IP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_PROGRESSIVE;
			NumberOfDecimatedPictureBuff = IP_PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_PROGRESSIVE;
			break;
		case	ENCODING_GOP_STRUCT_IBP:
			NumberOfOriginalPictureBuff = I1BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_PROGRESSIVE;
			NumberOfDecimatedPictureBuff = I1BP_PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_PROGRESSIVE;
			break;
		case	ENCODING_GOP_STRUCT_IBBP:
			NumberOfOriginalPictureBuff = I2BP_NO_B_REF_B_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_PROGRESSIVE;
			NumberOfDecimatedPictureBuff = I2BP_NO_B_REF_B_PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_PROGRESSIVE;
			break;
		case	ENCODING_GOP_STRUCT_IBBP_B_REF:
			NumberOfOriginalPictureBuff = I2BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_PROGRESSIVE;
			NumberOfDecimatedPictureBuff = I2BP_PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_PROGRESSIVE;
			break;
		case	ENCODING_GOP_STRUCT_IBBBP:
			NumberOfOriginalPictureBuff = I3BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_PROGRESSIVE;
			NumberOfDecimatedPictureBuff = I3BP_PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_PROGRESSIVE;
			break;
		case	ENCODING_GOP_STRUCT_IBBBBBP:
			NumberOfOriginalPictureBuff = I5BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_PROGRESSIVE;
			NumberOfDecimatedPictureBuff = I5BP_PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_PROGRESSIVE;
			break;
		case	ENCODING_GOP_STRUCT_IBBBBBBBP:
			NumberOfOriginalPictureBuff = I7BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_PROGRESSIVE;
			NumberOfDecimatedPictureBuff = I7BP_PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_PROGRESSIVE;
				break;
		default:
			NumberOfOriginalPictureBuff = I7BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_PROGRESSIVE;
			NumberOfDecimatedPictureBuff = I7BP_PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_PROGRESSIVE;
			break;
		}

			FreeBuffersQueueORIGINAL_LUMANumOfBuff = NumberOfOriginalPictureBuff;
			FreeBuffersQueueORIGINAL_CHROMANumOfBuff = NumberOfOriginalPictureBuff;
			FreeBuffersQueueORIGINAL_SHIFTED_CHROMANumOfBuff = 0;
			FreeBuffersQueueDECIMATED_1VNumOfBuff = NumberOfDecimatedPictureBuff;
			FreeBuffersQueueDECIMATED_2VNumOfBuff = NumberOfDecimatedPictureBuff;

			FreeBuffersQueueORIGINAL_LUMADcxvEn = DcxvEnable;
			FreeBuffersQueueORIGINAL_CHROMADcxvEn = DcxvEnable;
			FreeBuffersQueueDECIMATED_1VDcxvEn = DcxvEnable;
			FreeBuffersQueueDECIMATED_2VDcxvEn = DcxvEnable;

		    if (pstCoreSettings->eVersion <= BVCE_FW_P_COREVERSION_V3_0_0_2)
			{
						FreeBuffersQueueDECIMATED_1VDcxvEn = 0;
			            FreeBuffersQueueDECIMATED_2VDcxvEn = 0;
			}


            LumaBuffSize = bvceV3EpmCalcStripeBufferSize(MaxWidthInPel , MaxHeightInPel , FreeBuffersQueueORIGINAL_LUMADcxvEn , !IsProgressive , StripeWidth , X , Y , DramConfigPageSize);
			ChromaBuffSize = bvceV3EpmCalcStripeBufferSize(MaxWidthInPel , DIV2_ROUNDUP(MaxHeightInPel) , FreeBuffersQueueORIGINAL_CHROMADcxvEn , !IsProgressive , StripeWidth , X , Y , DramConfigPageSize);

			Decimated1h1vBufferSize = bvceV3EpmCalcStripeBufferSize(MAX_1H1V_WIDTH , MAX_1H1V_HEIGHT , FreeBuffersQueueDECIMATED_1VDcxvEn , !IsProgressive , StripeWidth , X , Y , DramConfigPageSize);
			Decimated2h1vBufferSize = bvceV3EpmCalcStripeBufferSize(DIV2_ROUNDUP(MaxWidthInPel) , MaxHeightInPel , FreeBuffersQueueDECIMATED_1VDcxvEn , !IsProgressive , StripeWidth , X , Y , DramConfigPageSize);


			Decimated1h2vBufferSize = bvceV3EpmCalcStripeBufferSize(MAX_1H1V_WIDTH , DIV2_ROUNDUP(MAX_1H1V_HEIGHT) , FreeBuffersQueueDECIMATED_1VDcxvEn , !IsProgressive , StripeWidth , X , Y , DramConfigPageSize);
			Decimated2h2vBufferSize = bvceV3EpmCalcStripeBufferSize(DIV2_ROUNDUP(MaxWidthInPel) , DIV2_ROUNDUP(MaxHeightInPel) , FreeBuffersQueueDECIMATED_2VDcxvEn , !IsProgressive , StripeWidth , X , Y , DramConfigPageSize);

			Decimated1vBufferSize = MAX(Decimated2h1vBufferSize, Decimated1h1vBufferSize);
			Decimated2vBufferSize = MAX(Decimated2h2vBufferSize, Decimated1h2vBufferSize);



			/* Allocate Luma Buffers */
			for (Ptr = 0; Ptr < FreeBuffersQueueORIGINAL_LUMANumOfBuff; Ptr++)
			{
				CurrAddr = ALIGN12BIT64(CurrAddr);
				CurrAddr = CurrAddr + LumaBuffSize;
			}



			/* Allocate Chroma Buffers */
			for (Ptr = 0; Ptr < FreeBuffersQueueORIGINAL_CHROMANumOfBuff; Ptr++)
			{
				CurrAddr = ALIGN12BIT64(CurrAddr);
				CurrAddr = CurrAddr + ChromaBuffSize;
			}


			/* Allocate Decimated 1V Buffers */
			for (Ptr = 0; Ptr < FreeBuffersQueueDECIMATED_1VNumOfBuff; Ptr++)
			{
				CurrAddr = ALIGN12BIT64(CurrAddr);
				CurrAddr = CurrAddr + Decimated1vBufferSize;
			}


			/* Allocate Decimated 2V Buffers */
			for (Ptr = 0; Ptr < FreeBuffersQueueDECIMATED_2VNumOfBuff; Ptr++)
			{
				CurrAddr = ALIGN12BIT64(CurrAddr);
				CurrAddr = CurrAddr + Decimated2vBufferSize;
			}
		}
		else
		{



		switch (pstMemSettings->MaxGopStructure)
		{
		case	ENCODING_GOP_STRUCT_I:
		case	ENCODING_GOP_STRUCT_IP:
		case	ENCODING_GOP_STRUCT_INFINITE_IP:
			NumberOfOriginalPictureBuff = IP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_INTERLACE;
			NumberOfDecimatedPictureBuff = IP_PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_INTERLACE;
			NumberOfShiftedChromaPictureBuff = IP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_SHIFTED_CHROMA_INTERLACE;
			break;
		case	ENCODING_GOP_STRUCT_IBP:
			NumberOfOriginalPictureBuff = I1BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_INTERLACE;
			NumberOfDecimatedPictureBuff = I1BP_PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_INTERLACE;
			NumberOfShiftedChromaPictureBuff = I1BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_SHIFTED_CHROMA_INTERLACE;
			break;
		case	ENCODING_GOP_STRUCT_IBBP:
			NumberOfOriginalPictureBuff = I2BP_NO_B_REF_B_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_INTERLACE;
			NumberOfDecimatedPictureBuff = I2BP_NO_B_REF_B_PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_INTERLACE;
			NumberOfShiftedChromaPictureBuff = I2BP_NO_B_REF_B_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_SHIFTED_CHROMA_INTERLACE;
			break;
		case	ENCODING_GOP_STRUCT_IBBP_B_REF:
			NumberOfOriginalPictureBuff = I2BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_INTERLACE;
			NumberOfDecimatedPictureBuff = I2BP_PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_INTERLACE;
			NumberOfShiftedChromaPictureBuff = I2BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_SHIFTED_CHROMA_INTERLACE;
			break;
		case	ENCODING_GOP_STRUCT_IBBBP:
			NumberOfOriginalPictureBuff = I3BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_INTERLACE;
			NumberOfDecimatedPictureBuff = I3BP_PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_INTERLACE;
			NumberOfShiftedChromaPictureBuff = I3BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_SHIFTED_CHROMA_INTERLACE;
			break;

		default:
			NumberOfOriginalPictureBuff = I3BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_INTERLACE;
			NumberOfDecimatedPictureBuff = I3BP_PREPROCESSOR_NUMBER_OF_DECIMATED_PICTURE_BUFF_INTERLACE;
			NumberOfShiftedChromaPictureBuff = I3BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_SHIFTED_CHROMA_INTERLACE;
			break;
		}


			FreeBuffersQueueORIGINAL_LUMANumOfBuff = NumberOfOriginalPictureBuff;
			FreeBuffersQueueORIGINAL_CHROMANumOfBuff = NumberOfOriginalPictureBuff;
			FreeBuffersQueueORIGINAL_SHIFTED_CHROMANumOfBuff = NumberOfShiftedChromaPictureBuff;
			FreeBuffersQueueDECIMATED_1VNumOfBuff = NumberOfDecimatedPictureBuff;
			FreeBuffersQueueDECIMATED_2VNumOfBuff = NumberOfDecimatedPictureBuff;

			FreeBuffersQueueORIGINAL_LUMADcxvEn = DcxvEnable;
			FreeBuffersQueueORIGINAL_CHROMADcxvEn = 0;
			FreeBuffersQueueDECIMATED_1VDcxvEn = DcxvEnable;
			FreeBuffersQueueDECIMATED_2VDcxvEn = DcxvEnable;
		    if (pstCoreSettings->eVersion <= BVCE_FW_P_COREVERSION_V3_0_0_2)
			{
						FreeBuffersQueueDECIMATED_1VDcxvEn = 0;
			            FreeBuffersQueueDECIMATED_2VDcxvEn = 0;
			}


			LumaBuffSize = bvceV3EpmCalcStripeBufferSize(MaxWidthInPel , MaxHeightInPel , FreeBuffersQueueORIGINAL_LUMADcxvEn , !IsProgressive , StripeWidth , X , Y , DramConfigPageSize);
			ChromaBuffSize = bvceV3EpmCalcStripeBufferSize(MaxWidthInPel , DIV2_ROUNDUP(MaxHeightInPel) , FreeBuffersQueueORIGINAL_CHROMADcxvEn , !IsProgressive , StripeWidth , X , Y , DramConfigPageSize);

			Decimated1h1vBufferSize = bvceV3EpmCalcStripeBufferSize(MAX_1H1V_WIDTH , MAX_1H1V_HEIGHT , FreeBuffersQueueDECIMATED_1VDcxvEn , !IsProgressive , StripeWidth , X , Y , DramConfigPageSize);
			Decimated2h1vBufferSize = bvceV3EpmCalcStripeBufferSize(DIV2_ROUNDUP(MaxWidthInPel) , MaxHeightInPel , FreeBuffersQueueDECIMATED_1VDcxvEn , !IsProgressive , StripeWidth , X , Y , DramConfigPageSize);

			Decimated1h2vBufferSize = bvceV3EpmCalcStripeBufferSize(MAX_1H1V_WIDTH , DIV2_ROUNDUP(MAX_1H1V_HEIGHT) , FreeBuffersQueueDECIMATED_1VDcxvEn , !IsProgressive , StripeWidth , X , Y , DramConfigPageSize);
			Decimated2h2vBufferSize = bvceV3EpmCalcStripeBufferSize(DIV2_ROUNDUP(MaxWidthInPel) , DIV2_ROUNDUP(MaxHeightInPel) , FreeBuffersQueueDECIMATED_2VDcxvEn , !IsProgressive , StripeWidth , X , Y , DramConfigPageSize);

			Decimated1vBufferSize = MAX(Decimated2h1vBufferSize, Decimated1h1vBufferSize);
			Decimated2vBufferSize = MAX(Decimated2h2vBufferSize, Decimated1h2vBufferSize);



			/* Allocate Luma Buffers */
			for (Ptr = 0; Ptr < FreeBuffersQueueORIGINAL_LUMANumOfBuff; Ptr += 2)
			{
				CurrAddr = ALIGN12BIT64(CurrAddr);
				CurrAddr = CurrAddr + LumaBuffSize;
			}


			/* Allocate Chroma Buffers */
			for (Ptr = 0; Ptr < FreeBuffersQueueORIGINAL_CHROMANumOfBuff; Ptr += 2)
			{
				CurrAddr = ALIGN12BIT64(CurrAddr);
				CurrAddr = CurrAddr + ChromaBuffSize;
			}


			/* Allocate Shifted Chroma Buffers */
			for (Ptr = 0; Ptr < FreeBuffersQueueORIGINAL_SHIFTED_CHROMANumOfBuff; Ptr++)
			{
				CurrAddr = ALIGN12BIT64(CurrAddr);
				CurrAddr = CurrAddr + ChromaBuffSize;
			}


			/* Allocate Decimated 1V Buffers */
			for (Ptr = 0; Ptr < FreeBuffersQueueDECIMATED_1VNumOfBuff; Ptr += 2)
			{
				CurrAddr = ALIGN12BIT64(CurrAddr);
				CurrAddr = CurrAddr + Decimated1vBufferSize;
			}


			/* Allocate Decimated 2V Buffers */
			for (Ptr = 0; Ptr < FreeBuffersQueueDECIMATED_2VNumOfBuff; Ptr += 2)
			{
				CurrAddr = ALIGN12BIT64(CurrAddr);
				CurrAddr = CurrAddr + Decimated2vBufferSize;
			}
		}

		}

		return (uint32_t) CurrAddr;
}

/************************************************************************
* Function: bvceV3EpmAlignJword
*
* Actions: align a number to Jword(unit of 256bits).
*
* Params:
*        UnalignedNumber
*
* Returns:
*        Aligned number
*
************************************************************************/
#define MASK64_59MSB(x) (((((x) >> 32) & 0xFFFFFFFF) << 32) | ((x) & 0xFFFFFFE0))

static uint64_t bvceV3EpmAlignJword(uint64_t UnalignedNumber)
{

	uint64_t AlignedNumber;
	AlignedNumber = MASK64_59MSB(UnalignedNumber + 31);
	return(AlignedNumber);
}

/************************************************************************
* Function: bvceV3EpmCalcNMBY
*
* Actions: find the number of macroblocs in stripe height
*
* Params:
*        PictureHeightInMbs: hight of picture in macroblock units
*
* Returns:
*        Number of macroblocks per stripe height
*
************************************************************************/
static uint8_t bvceV3EpmCalcNMBY(uint32_t PictureHeightInMbs, uint32_t X, uint32_t Y, uint32_t DramConfigPageSize, uint16_t StripeWidth)
{
	uint16_t n;
	uint16_t VertMbsPerPage;
	uint16_t PageSize;

	n = 0;
	PageSize = (1 << (DramConfigPageSize + 11));
	VertMbsPerPage = MAX(1, ((PageSize / StripeWidth) >> 4));

	while (1)
	{
		n++;
		if (((X * (uint32_t)n + Y) * VertMbsPerPage) >= PictureHeightInMbs)
			return(((X * (uint32_t)n + Y) * VertMbsPerPage));
	}
}


/************************************************************************
* Function: EpmV3CalcDcxvNMBY
*
* Actions: find the number of macroblocs in stripe height
*
* Params:
*        PictureHeightInMbs: hight of picture in Mbs
*        IsDcxvBuf: 1-DCXV buff 0-none DCXV buff
*        IsInterlace: 1-Interlace  0-not interlace
*        IsChroma: 1- chroma buff 0 -luma buff
*
* Returns:
*        Number of macroblocks per stripe height
*
************************************************************************/
#define V3_DCXV_STRIPE_WIDTH 64
#define V3_DCXV_PADDED_LINE 2
#define V3_DCXV_COMP_RATIO 2

#define V3_DCXVLumaBuffer            0
#define V3_DCXVChromaBuffer          1

#define V3_DCXVBuffer                1
#define V3_NonDCXVBuffer             0

#if 0
static uint8_t bvceV3EpmCalcDcxvNMBY(uint32_t PictureHeightInMbs, uint32_t IsDcxvBuf, uint32_t IsInterlace, uint32_t IsChroma, uint32_t X, uint32_t Y, uint32_t DramConfigPageSize, uint16_t StripeWidth)
{
	uint8_t Nmby;
	uint32_t DcxvPaddingHeight;
	uint32_t BuffereHeightInPels;
	uint32_t PictureHeightInPels;

	PictureHeightInPels = MULT16(PictureHeightInMbs);
	if (IsChroma == 1)
		PictureHeightInPels = DIV2_ROUNDUP(PictureHeightInPels);


	if (IsInterlace == 1)
	{
		DcxvPaddingHeight = V3_DCXV_PADDED_LINE * 2;
	}
	else
	{
		DcxvPaddingHeight = V3_DCXV_PADDED_LINE;
	}

	if (IsDcxvBuf == 1)
	{
		BuffereHeightInPels = PictureHeightInPels + DcxvPaddingHeight;
	}
	else
	{
		BuffereHeightInPels = PictureHeightInPels;
	}

	if (
		(IsInterlace == 1) &&
		(IsChroma == 1)
		)
	{
		BuffereHeightInPels = PictureHeightInPels;
	}


	Nmby = bvceV3EpmCalcNMBY(DIV16_ROUNDUP(BuffereHeightInPels), X, Y, DramConfigPageSize, StripeWidth);
	return(Nmby);
}
#endif

/************************************************************************
* Function: EpmV3CalcStripeBufferSize
*
* Actions: find striped format buffer size
*
* Params:
*        PictureWidthInMb, PictureHeightInMbs: picture width and height in mbs
*        IsDcxvBuff
*
* Returns:
*        buffer size
*
************************************************************************/
static uint32_t bvceV3EpmCalcStripeBufferSize(uint32_t PictureWidthInPels, uint32_t PictureHeightInPels, uint32_t IsDcxvBuf, uint32_t IsInterlace, uint32_t StripeWidth, uint32_t X, uint32_t Y, uint8_t DramConfigPageSize)
{
	uint32_t BufferWidthInPels, BuffereHeightInPels;
	uint32_t BuffSize;
	uint32_t DcxvPaddingWidth, DcxvPaddingHeight;
	uint16_t Nmby;

	DcxvPaddingWidth = ((PictureWidthInPels + V3_DCXV_STRIPE_WIDTH - 1) / V3_DCXV_STRIPE_WIDTH) * V3_DCXV_STRIPE_WIDTH - PictureWidthInPels;

	if (IsInterlace == 1)
	{
		DcxvPaddingHeight = V3_DCXV_PADDED_LINE * 2;
	}
	else
	{
		DcxvPaddingHeight = V3_DCXV_PADDED_LINE;
	}

	if (IsDcxvBuf == 1)
	{
		BufferWidthInPels = PictureWidthInPels + DcxvPaddingWidth;
		BufferWidthInPels = BufferWidthInPels / V3_DCXV_COMP_RATIO;
		BuffereHeightInPels = PictureHeightInPels + DcxvPaddingHeight;
	}
	else
	{
		BufferWidthInPels = PictureWidthInPels;
		BuffereHeightInPels = PictureHeightInPels;
	}



	Nmby = bvceV3EpmCalcNMBY(DIV16_ROUNDUP(BuffereHeightInPels), X, Y, DramConfigPageSize, StripeWidth);


	BuffSize = (uint32_t) bvceV3EpmAlignJword(bvceEpmCalcWidthInStripes(BufferWidthInPels, StripeWidth) * StripeWidth * MULT16(Nmby));
	BuffSize = ((BuffSize + 4095) & 0xfffff000);



	return(BuffSize);



}

/*********************************************************************
*  FrameRate_e BVCE_FW_P_FrameRateCodeToFramRate( FrameRateCode_e FrameRateCode )
*
*       Computes Frame rate  for the configured encoder frame rate code
*
**********************************************************************/
FrameRate_e BVCE_FW_P_FrameRateCodeToFrameRate( FrameRateCode_e FrameRateCode )
{
    FrameRate_e FrameRate = 0;

    switch(FrameRateCode)
    {
        case ENCODING_FRAME_RATE_CODE_1998:
            FrameRate = ENCODING_FRAME_RATE_1998;
            break;
        case ENCODING_FRAME_RATE_CODE_2397:
            FrameRate = ENCODING_FRAME_RATE_2397;
            break;
        case ENCODING_FRAME_RATE_CODE_2400:
            FrameRate = ENCODING_FRAME_RATE_2400;
            break;
        case ENCODING_FRAME_RATE_CODE_2500:
            FrameRate = ENCODING_FRAME_RATE_2500;
            break;
        case ENCODING_FRAME_RATE_CODE_2997:
            FrameRate = ENCODING_FRAME_RATE_2997;
            break;
        case ENCODING_FRAME_RATE_CODE_3000:
            FrameRate = ENCODING_FRAME_RATE_3000;
            break;
        case ENCODING_FRAME_RATE_CODE_5000:
            FrameRate = ENCODING_FRAME_RATE_5000;
            break;
        case ENCODING_FRAME_RATE_CODE_5994:
            FrameRate = ENCODING_FRAME_RATE_5994;
            break;
        case ENCODING_FRAME_RATE_CODE_6000:
            FrameRate = ENCODING_FRAME_RATE_6000;
            break;
        case ENCODING_FRAME_RATE_CODE_1498:
            FrameRate = ENCODING_FRAME_RATE_1498;
            break;
        case ENCODING_FRAME_RATE_CODE_1500:
            FrameRate = ENCODING_FRAME_RATE_1500;
            break;
        case ENCODING_FRAME_RATE_CODE_2000:
            FrameRate = ENCODING_FRAME_RATE_2000;
            break;
        case ENCODING_FRAME_RATE_CODE_1250:
            FrameRate = ENCODING_FRAME_RATE_1250;
            break;
        case ENCODING_FRAME_RATE_CODE_1000:
            FrameRate = ENCODING_FRAME_RATE_1000;
            break;
        case ENCODING_FRAME_RATE_CODE_0749:
            FrameRate = ENCODING_FRAME_RATE_0749;
            break;
        case ENCODING_FRAME_RATE_CODE_0750:
            FrameRate = ENCODING_FRAME_RATE_0750;
            break;
        case ENCODING_FRAME_RATE_CODE_1198:
            FrameRate = ENCODING_FRAME_RATE_1198;
            break;
        case ENCODING_FRAME_RATE_CODE_1200:
            FrameRate = ENCODING_FRAME_RATE_1200;
            break;
        case ENCODING_FRAME_RATE_CODE_0999:
            FrameRate = ENCODING_FRAME_RATE_0999;
            break;

        default:
#ifdef BCHP_CHIP
            BDBG_ASSERT(!"unsupported BVN frame rate");
            return 0;
#else
#if defined(CEB2APP) || defined (VICE_FW_CMODEL_APP)
            assert(!"unsupported BVN frame rate");
#else
            ASSERT(!"unsupported BVN frame rate");
#endif
            break;
#endif

    }

    return(FrameRate);
}

#endif /* defined(BCHP_CHIP) || defined(CEB2APP) || defined(VICE_FW_CMODEL_APP) */


/***********************************************************************
*  BVCE_FW_P_Compute_EncodeDelayIn27MHzTicks() - Computes PreEncode and
*  Encode Delay in units expressed as 27 MHz clock ticks
************************************************************************/
uint32_t
BVCE_FW_P_Compute_EncodeDelayIn27MHzTicks(
        FrameRate_e MinFrameRateLimit,
        uint8_t     InputType,
        uint8_t     MaxAllowedGopStruct
    )
{
    FPNumber_t FP_temp;
    uint32_t encoderDelay = 0;
    uint32_t PictureIntervalIn27MhzTicks = 0;
    uint16_t numOrigPicBuffs = 0;

    PictureIntervalIn27MhzTicks = BVCE_FW_P_ConvertFrameRate(MinFrameRateLimit, &FP_temp);

    /* Compute encoderDelay based on max allowed gopstruct and minFrameRate Limit by scaling 1/MinFrameRateLimit */
    if(InputType == ENCODER_INPUT_TYPE_PROGRESSIVE)
    {
      switch (MaxAllowedGopStruct)
      {
      case	ENCODING_GOP_STRUCT_I:
      case	ENCODING_GOP_STRUCT_IP:
      case	ENCODING_GOP_STRUCT_INFINITE_IP:
	numOrigPicBuffs = IP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_PROGRESSIVE;
	break;
      case	ENCODING_GOP_STRUCT_IBP:
	numOrigPicBuffs = I1BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_PROGRESSIVE;
	break;
      case	ENCODING_GOP_STRUCT_IBBP:
	numOrigPicBuffs = I2BP_NO_B_REF_B_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_PROGRESSIVE;
	break;
      case	ENCODING_GOP_STRUCT_IBBP_B_REF:
	numOrigPicBuffs = I2BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_PROGRESSIVE;
	break;
      case	ENCODING_GOP_STRUCT_IBBBP:
	numOrigPicBuffs = I3BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_PROGRESSIVE;
	break;
      case	ENCODING_GOP_STRUCT_IBBBBBP:
	numOrigPicBuffs = I5BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_PROGRESSIVE;
	break;
      case	ENCODING_GOP_STRUCT_IBBBBBBBP:
	numOrigPicBuffs = I7BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_PROGRESSIVE;
	break;
      default:
	numOrigPicBuffs = I7BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_PROGRESSIVE;
	break;
      }
    }
    else
    { /* Interlaced input */
      switch (MaxAllowedGopStruct)
      {
      case	ENCODING_GOP_STRUCT_I:
      case	ENCODING_GOP_STRUCT_IP:
      case	ENCODING_GOP_STRUCT_INFINITE_IP:
	/* Divide by 2 to get per frame */
	numOrigPicBuffs = IP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_INTERLACE >> 1;
	break;
      case	ENCODING_GOP_STRUCT_IBP:
	numOrigPicBuffs = I1BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_INTERLACE >> 1;
	break;
      case	ENCODING_GOP_STRUCT_IBBP:
	numOrigPicBuffs = I2BP_NO_B_REF_B_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_INTERLACE >> 1;
	break;
      case	ENCODING_GOP_STRUCT_IBBP_B_REF:
	numOrigPicBuffs = I2BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_INTERLACE >> 1;
	break;
      case	ENCODING_GOP_STRUCT_IBBBP:
	numOrigPicBuffs = I3BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_INTERLACE >> 1;
	break;
      default:
	numOrigPicBuffs = I3BP_PREPROCESSOR_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_INTERLACE >> 1;
	break;
      }
    }

    encoderDelay = U32xU16MULTIPLY(PictureIntervalIn27MhzTicks, numOrigPicBuffs);

    return(encoderDelay);
}

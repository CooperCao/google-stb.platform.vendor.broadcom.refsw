/***************************************************************************
 *     (c)2004-2010 Broadcom Corporation
 *  
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * 
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:  Contains constants, globals, and macros used by SCD
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ************************************************************/

/****************************************************************************
 * SCD
 * 

 *
 ****************************************************************************/

#ifndef SCD_H
#define SCD_H

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include "bstd.h"
#define REMOVE_FDC
#define REMOVE_LAGC

/****************************************************************************/
/* options */
/****************************************************************************/

/* debug enables: define as x to enable, nothing to disable */
#define API_FUNC_DEBUG(x)   x
#define HAL_FUNC_DEBUG(x)   
#define OSLIB_FUNC_DEBUG(x) 
#define CHIP_FUNC_DEBUG(x)  
#define INT_FUNC_DEBUG(x)   
#define CHIP_DEBUG(x)       
#define FAT_DEBUG(x)        
#define FDC_DEBUG(x)        
#define OPEN_DEBUG(x)       
#define CONFIG_DEBUG(x)     
#define HAL_DEBUG(x)        
#define OSLIB_DEBUG(x)     
#define BERT_DEBUG(x)       

/****************************************************************************/
/* constants */
/****************************************************************************/

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define MHZ (1000000)
#define KHZ (1000)

#define FAT_256QAM_DATA_RATE        38810000
#define FAT_64QAM_DATA_RATE         26970000
#define FAT_VSB_DATA_RATE           19280000

#define DINT_256QAM_DATA_RATE       40720000
#define DINT_64QAM_DATA_RATE        28300000
#define DINT_VSB_DATA_RATE          21340000
 
#define TRELLIS_256QAM_DATA_RATE    40720000
#define TRELLIS_64QAM_DATA_RATE     28300000
#define TRELLIS_VSB_DATA_RATE       21340000

#define FDC_772_DATA_RATE           1540000
#define FDC_1024_DATA_RATE          2050000
#define FDC_1544_DATA_RATE          3090000

/* Reduce the tuner offset from 300Khz to 100KHz to reduce the non-linear */
/* artifacts (\\\\\  diagonal lines). */
/* Binning Chen, 2/9/2009. */
#define DEFAULT_ANALOG_SPECTR_6_MHZ_SHIFT 100*KHZ
#define DEFAULT_ANALOG_SPECTR_7_MHZ_SHIFT 100*KHZ
#define DEFAULT_ANALOG_SPECTR_8_MHZ_SHIFT 100*KHZ

/* Number of elements in the Power Spectrum Arrary result */
#define PSDARRAYSIZE        (512)

/* Number of elements in the Equalizer status result */
#define EQARRAYSIZE         (256)

/* Number of elements in the tap result */
#define TAPARRAYSIZE        (1280)
/* #define VSB_PLUS            (0) */
/* #define TAPARRAYSIZE        (1280) */
/* #define VSB_PLUS            (1) */
#define TAPARRAYSIZE_2      (2304)

/* Number of elements in the CHC and CHP output */
#define CHCPOUTPUTARRAYSIZE        (16642)
#define CHCOUTPUTARRAYSIZE         (12096)
#define CHPOUTPUTARRAYSIZE         (4546)



/* AGC Setup Script Token Values (negate these in scripts) */
#define ATS_AGC_MAX                      (99) /* maximum ATS token value */
#define UAGC_SETUP_DONE               (100) /* end of agc setup data */

#if (!defined REMOVE_LAGC)
#define NXT_AGC_SETUP_DONE              (100) /* end of agc setup data */
#define NXT_AGC_SETUP_VSB               (200) /* start of VSB setup */
#define NXT_AGC_SETUP_64QAM             (300) /* start of 64Qam setup */
#define NXT_AGC_SETUP_256QAM            (400) /* start of 256Qam setup */
#define NXT_AGC_SETUP_NTSC              (500) /* start of NTSC setup */
#define NXT_AGC_SETUP_NTSC_ADJ          (501) /* Start of NTSC adjacent setup */
#define NXT_AGC_SETUP_NTSC_SLOW         (502) /* Start of NTSC slow mode */
#define NXT_AGC_SETUP_NTSC_FAST         (503) /* start of ntsc fast mode */
#define NXT_AGC_SETUP_PAL               (600) /* start of PAL setup */
#define NXT_AGC_SETUP_PAL_ADJ           (601) /* Start of PAL adjacent setup */
#define NXT_AGC_SETUP_PAL_SLOW          (602) /* Start of PAL slow mode */
#define NXT_AGC_SETUP_PAL_FAST          (603) /* start of PAL fast mode */
#define NXT_AGC_SETUP_SECAM             (700) /* start of SECAM setup */
#define NXT_AGC_SETUP_SECAM_ADJ         (701) /* Start of SECAM adjacent setup */
#define NXT_AGC_SETUP_SECAM_SLOW        (702) /* Start of SECAM slow mode */
#define NXT_AGC_SETUP_SECAM_FAST        (703) /* start of SECAM fast mode */
#define NXT_AGC_SETUP_COFDM             (800) /* start of COFDM setup */
#define NXT_AGC_SETUP_FDC               (900) /* start of fdc channel */
#endif
#define IF_DEMOD                        (910) /* start of AFE dependent IF Demod inits */
#define UAGC_IF_VGA                     (920) /* start of AFE dependent UAGC inits */
#define UAGC_DIGITAL_MODULATION         (930) /* start of AFE dependent UAGC inits */
#define UAGC_ANALOG_NEGATIVE_MODULATION (940) /* start of AFE dependent UAGC inits */
#define UAGC_ANALOG_POSITIVE_MODULATION (950) /* start of AFE dependent UAGC inits */

/* ADC0 ref volatage set: normal==1.5V, lower==1.2V, higher==1.7V */
#define FAT_ADCREF_IS_NORMAL            (0x00)
#define FAT_ADCREF_IS_LOWER             (0x10)
#define FAT_ADCREF_IS_HIGHER            (0x20)

/* To control the AGC in FW */
#define SINGLE_LOOP_SINGLE_SAW          (1)
#define SINGLE_LOOP_DUAL_SAW            (2)
#define DUAL_LOOP_SINGLE_SAW            (3)
#define DUAL_LOOP_DUAL_SAW              (4)

/****************************************************************************/
/* macros */
/****************************************************************************/

#define FAT_MAX_DB                              (40) /* 100% on SQI */
#define FAT_MIN_DB                              (0)

#define FAT_SQI_LOCKED(snr_db,mod_format)       ((uint32_t)(((snr_db)-FAT_MIN_DB)*100/(FAT_MAX_DB-FAT_MIN_DB)))
#define FAT_SQI_UNLOCKED(agc0)                  ((uint32_t)(((0xFFFF - agc0)*100)/0x10000))
#define FAT_SQI(lock,snr,mod_format,agc0)       ((lock)?(50+FAT_SQI_LOCKED(snr,mod_format)/2):(FAT_SQI_UNLOCKED(agc0)/2))

#define IS_ANALOG_FAT_MOD_FORMAT(mod_format)  \
   ((((mod_format) == SCD_MOD_FORMAT__FAT_NTSC_M  ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_NTSC_N  ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_NTSC_J  ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_NTSC_443) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_PAL_I   ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_PAL_B   ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_PAL_B1  ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_PAL_G   ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_PAL_H   ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_PAL_D   ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_PAL_K   ) ||   \
	 ((mod_format) == SCD_MOD_FORMAT__FAT_PAL_60  ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_PAL_M   ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_PAL_N   ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_PAL_NC  ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_SECAM_B ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_SECAM_D ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_SECAM_G ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_SECAM_H ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_SECAM_K ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_SECAM_K1) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_SECAM_L ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_SECAM_L1)))

#define IS_DIGITAL_FAT_MOD_FORMAT(mod_format)  \
   ((((mod_format) == SCD_MOD_FORMAT__FAT_VSB    ) ||   \
((mod_format) == SCD_MOD_FORMAT__FAT_QAM64  ) ||   \
((mod_format) == SCD_MOD_FORMAT__FAT_QAM256 ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_J83ABC ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_AUTO   ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_COFDM_8) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_COFDM_7) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_COFDM_6) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_COFDM_5) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_UNIFIED_COFDM)))

#define IS_DIGITAL_CABLE_FAT_MOD_FORMAT(mod_format)  \
((mod_format) == SCD_MOD_FORMAT__FAT_QAM64  ) ||   \
((mod_format) == SCD_MOD_FORMAT__FAT_QAM256 ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_J83ABC )))

#define IS_VSB_FAT_MOD_FORMAT(mod_format)  \
   (((mod_format) == SCD_MOD_FORMAT__FAT_VSB))

#define IS_QAM_FAT_MOD_FORMAT(mod_format)  \
((((mod_format) == SCD_MOD_FORMAT__FAT_QAM64  ) ||   \
((mod_format) == SCD_MOD_FORMAT__FAT_QAM256 ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_J83ABC )))

#define IS_COFDM_FAT_MOD_FORMAT(mod_format)  \
   ((((mod_format) == SCD_MOD_FORMAT__FAT_COFDM_8) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_COFDM_7) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_COFDM_6) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_COFDM_5) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_UNIFIED_COFDM)))

#define IS_PAL_FAT_MOD_FORMAT(mod_format)  \
   ((((mod_format) == SCD_MOD_FORMAT__FAT_PAL_I ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_PAL_B ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_PAL_B1) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_PAL_G ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_PAL_H ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_PAL_D ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_PAL_K ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_PAL_60) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_PAL_M ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_PAL_N ) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_PAL_NC)))

#define IS_SECAM_FAT_MOD_FORMAT(mod_format)  \
   ((((mod_format) == SCD_MOD_FORMAT__FAT_SECAM_B) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_SECAM_D) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_SECAM_G) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_SECAM_H) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_SECAM_K) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_SECAM_K1) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_SECAM_L) ||   \
     ((mod_format) == SCD_MOD_FORMAT__FAT_SECAM_L1)))


/****************************************************************************/
/* integer types */
/****************************************************************************/

typedef void * SCD_HANDLE;

#define SCD_NULL           (0)
#define SCD_DUMMY_HANDLE   ((void*)(-1))
#define SCD_NO_INSTANCE    (0xFFFFFFFF)

/****************************************************************************/
/* enum types */
/****************************************************************************/

/* results */
typedef enum
{
    SCD_RESULT__OK = 0,
    SCD_RESULT__ERROR,
    SCD_RESULT__SCD_NOT_INITIALIZED,
    SCD_RESULT__ARG_OUT_OF_RANGE,
    SCD_RESULT__CHIP_FEATURE_NOT_IMPLEMENTED,
    SCD_RESULT__TUNER_FEATURE_NOT_IMPLEMENTED,
    SCD_RESULT__CHIP_NOT_AVAILABLE,
    SCD_RESULT__TUNER_NOT_AVAILABLE,
    SCD_RESULT__FAT_NOT_AVAILABLE,
    SCD_RESULT__FDC_NOT_AVAILABLE,
    SCD_RESULT__HANDLE_NOT_OPEN,
    SCD_RESULT__CHIP_NOT_OPEN,
    SCD_RESULT__TUNER_NOT_OPEN,
    SCD_RESULT__FAT_NOT_OPEN,
    SCD_RESULT__FDC_NOT_OPEN,
    SCD_RESULT__CHIP_ERROR,
    SCD_RESULT__TUNER_ERROR,
    SCD_RESULT__CHIP_READ_ERROR,
    SCD_RESULT__CHIP_WRITE_ERROR,
    SCD_RESULT__TUNER_READ_ERROR,
    SCD_RESULT__TUNER_WRITE_ERROR,
    SCD_RESULT__BAD_FIRMWARE,
    SCD_RESULT__COMMAND_IN_PROGRESS
} SCD_RESULT;

/* modulation formats */
typedef enum
{
    SCD_MOD_FORMAT__UNKNOWN = 0,
    SCD_MOD_FORMAT__LAST,           /* use last value passed to scdStart */
    SCD_MOD_FORMAT__FAT_VSB,
    SCD_MOD_FORMAT__FAT_J83ABC,
    SCD_MOD_FORMAT__FAT_AUTO,       /* Auto VSB/QAM Mode detection */
    SCD_MOD_FORMAT__FAT_NTSC_M,
    SCD_MOD_FORMAT__FAT_NTSC_N,
    SCD_MOD_FORMAT__FAT_NTSC_J,
    SCD_MOD_FORMAT__FAT_NTSC_443,
    SCD_MOD_FORMAT__FAT_PAL_I,
    SCD_MOD_FORMAT__FAT_PAL_B,
    SCD_MOD_FORMAT__FAT_PAL_B1,
    SCD_MOD_FORMAT__FAT_PAL_G,
    SCD_MOD_FORMAT__FAT_PAL_H,
    SCD_MOD_FORMAT__FAT_PAL_D,
    SCD_MOD_FORMAT__FAT_PAL_K,
    SCD_MOD_FORMAT__FAT_PAL_60,
    SCD_MOD_FORMAT__FAT_PAL_M,
    SCD_MOD_FORMAT__FAT_PAL_N,
    SCD_MOD_FORMAT__FAT_PAL_NC,
    SCD_MOD_FORMAT__FAT_SECAM_B,
    SCD_MOD_FORMAT__FAT_SECAM_D,
    SCD_MOD_FORMAT__FAT_SECAM_G,
    SCD_MOD_FORMAT__FAT_SECAM_H,
    SCD_MOD_FORMAT__FAT_SECAM_K,
    SCD_MOD_FORMAT__FAT_SECAM_K1,
    SCD_MOD_FORMAT__FAT_SECAM_L,
    SCD_MOD_FORMAT__FAT_SECAM_L1,
    SCD_MOD_FORMAT__FAT_UNIFIED_COFDM,
SCD_MOD_FORMAT__FAT_QAM64,
SCD_MOD_FORMAT__FAT_QAM256,
    SCD_MOD_FORMAT__FAT_COFDM_8,
    SCD_MOD_FORMAT__FAT_COFDM_7,
    SCD_MOD_FORMAT__FAT_COFDM_6,
    SCD_MOD_FORMAT__FAT_COFDM_5,
    SCD_MOD_FORMAT__FAT_ALL,
    SCD_MOD_FORMAT__FAT_ALL_ANALOG,
    SCD_MOD_FORMAT__FAT_ALL_DIGITAL
} SCD_MOD_FORMAT;

/* BERT inputs */
typedef enum
{
    SCD_BERT_INPUT__FAT,            /* BERT data is the FAT after all error correction has been completed */
    SCD_BERT_INPUT__DEINTERLEAVER,  /* FAT Data before any error correction */
    SCD_BERT_INPUT__FDC,            /* BERT data is the FDC data */
    SCD_BERT_INPUT__TRELLIS,        /* FAT data after the trellis decoder */
    SCD_BERT_INPUT__NONE            /* No BERT operations */
} SCD_BERT_INPUT;

/* Selection of output signal polarity. These polarities are for clock, data, sync, error and valid */
typedef enum
{
    SCD_SIGNAL_POLARITY__INVERT,
    SCD_SIGNAL_POLARITY__NO_INVERT
} SCD_SIGNAL_POLARITY;

/* Selection of the signal strengths for the pads. */
typedef enum
{
    SCD_SIGNAL_STRENGTH__DEFAULT,
    SCD_SIGNAL_STRENGTH__1,
    SCD_SIGNAL_STRENGTH__2,
    SCD_SIGNAL_STRENGTH__3,
    SCD_SIGNAL_STRENGTH__4,
    SCD_SIGNAL_STRENGTH__5,
    SCD_SIGNAL_STRENGTH__6,
    SCD_SIGNAL_STRENGTH__7,
    SCD_SIGNAL_STRENGTH__8,
    SCD_SIGNAL_STRENGTH__9,
    SCD_SIGNAL_STRENGTH__10,
    SCD_SIGNAL_STRENGTH__11,
    SCD_SIGNAL_STRENGTH__12,
    SCD_SIGNAL_STRENGTH__13,
    SCD_SIGNAL_STRENGTH__14,
    SCD_SIGNAL_STRENGTH__15
} SCD_SIGNAL_STRENGTH;

/* Selection of the symbol rate for the FDC demodulator. */
typedef enum
{
    SCD_FDC_SYMBOL_RATE__772,  /* 772 kilo symbols per second */
    SCD_FDC_SYMBOL_RATE__1024, /* 1024 kilo symbols per second */
    SCD_FDC_SYMBOL_RATE__1544, /* 1544 kilo symbols per second */
    SCD_FDC_SYMBOL_RATE__UNKNOWN
} SCD_FDC_SYMBOL_RATE;

typedef enum
{
    SCD_TOP_AGC_STATE__UNKNOWN = 0,
    SCD_TOP_AGC_STATE__LOW_INPUT_LEVEL,
    SCD_TOP_AGC_STATE__MED_INPUT_LEVEL,
    SCD_TOP_AGC_STATE__HIGH_INPUT_LEVEL
} SCD_TOP_AGC_STATE;

typedef enum
{
    SCD_SCAN_ORDER__SCANALL     /* Scan All Modes */
} SCD_SCAN_ORDER;

/* version Items */
typedef enum
{
    SCD_ITEM__API,
    SCD_ITEM__CHIP, 
	SCD_ITEM__TUNER,
    SCD_ITEM__FIRMWARE,
    SCD_ITEM__FAT,
#if (!defined REMOVE_FDC)
    SCD_ITEM__FDC,
#endif
    SCD_ITEM_LAST
} SCD_ITEM;

typedef enum
{
    SCD_LOCK_STATUS__UNLOCKED,
    SCD_LOCK_STATUS__LOCKED,
    SCD_LOCK_STATUS__NOT_READY
} SCD_LOCK_STATUS;

typedef enum
{
    SCD_COMMUNICATION__OK,
    SCD_COMMUNICATION__NOT_OK
} SCD_COMMUNICATION;

typedef enum
{
    SCD_BURST_MODE__BURST_OFF,
    SCD_BURST_MODE__BURST_ON,
    SCD_BURST_MODE__CONSTANT_PACKET
} SCD_BURST_MODE;

typedef enum
{
    SCD_TUNE_MODE__FW,
    SCD_TUNE_MODE__APP
} SCD_TUNE_MODE;

typedef enum
{
    SCD_IF_FREQUENCY__DEFAULT,
    SCD_IF_FREQUENCY__36000000,
    SCD_IF_FREQUENCY__36166666,
    SCD_IF_FREQUENCY__44000000
} SCD_IF_FREQUENCY;

/*ISDBT*/
typedef enum
{
    SCD_COFDM_TDI__0,
    SCD_COFDM_TDI__4,
    SCD_COFDM_TDI__8,
    SCD_COFDM_TDI__16
} SCD_COFDM_TDI;

/* Acquire */
typedef enum
{
    SCD_DIRECTED_ACQUIRE,
    SCD_FULL_ACQUIRE,
    SCD_SEARCH_SCAN,
    SCD_SYMBOL_RATE_VERIFY,
    SCD_SYMBOL_RATE_SCAN,
    SCD_CONST_SEARCH
} SCD_ACQUIRE_CONFIG;

typedef enum
{
    SCD_BW_UNDEFINED,
    SCD_BW_1MHZ,
    SCD_BW_2MHZ,
    SCD_BW_3MHZ,
    SCD_BW_4MHZ,
    SCD_BW_5MHZ,    
    SCD_BW_6MHZ,
    SCD_BW_7MHZ,
    SCD_BW_8MHZ,
    SCD_BW_9MHZ,
    SCD_BW_10MHZ	
} SCD_BAND_WIDTH;

/* J83 */
typedef enum
{
    J83A,
    J83B,
    J83C
} SCD_J83ABC_MODE;

typedef enum
{
    QAM_MODE_DIGITAL_16QAM,						
    QAM_MODE_DIGITAL_32QAM,					
    QAM_MODE_DIGITAL_64QAM,	
    QAM_MODE_DIGITAL_128QAM,
    QAM_MODE_DIGITAL_256QAM,
    QAM_MODE_DIGITAL_UNDEFINED
} SCD_QAM_MODE_DIGITAL;

/* 
 * unified cofdm
 */
typedef enum
{
    OFDM_STANDARD_DVBT,
    OFDM_STANDARD_ISDBT
} SCD_OFDM_STANDARD;

typedef enum
{
    OFDM_CCI_AUTO,
    OFDM_CCI_ENABLE,
    OFDM_CCI_NONE
} SCD_OFDM_CCI_MODE;

typedef enum
{
    OFDM_ACI_AUTO,
    OFDM_ACI_ENABLE,
    OFDM_ACI_NONE
} SCD_OFDM_ACI_MODE;

typedef enum
{
    OFDM_MOBIL_AUTO,
    OFDM_MOBIL_FIXED,
    OFDM_MOBIL_PEDESTRAIN,
    OFDM_MOBIL_MOBIL
} SCD_OFDM_MOBIL_MODE;

typedef enum
{
    OFDM_PRIORITY_LOW,
    OFDM_PRIORITY_HIGH
} SCD_OFDM_PRIORITY_MODE;

typedef enum
{
    OFDM_RANGE_NARROW,
    OFDM_RANGE_WIDE
} SCD_OFDM_CARRIER_RANGE;

typedef enum
{
    OFDM_IMPULSE_AUTO,
    OFDM_IMPULSE_ENABLE,
    OFDM_IMPULSE_NONE
} SCD_OFDM_IMPULSE;

typedef enum
{
    OFDM_MODEGUARD_AUTO,
    OFDM_MODEGUARD_MANUAL
} SCD_OFDM_MODEGUARD;

typedef enum
{
    OFDM_MODE_2K,
    OFDM_MODE_4K,
    OFDM_MODE_8K
} SCD_OFDM_MODE;

typedef enum
{
    OFDM_GUADR_1_32,
    OFDM_GUADR_1_16,
    OFDM_GUADR_1_8,
    OFDM_GUADR_1_4
} SCD_OFDM_GUARD;

typedef enum
{
    OFDM_TPS_AUTO,
    OFDM_TPS_MANUAL
} SCD_OFDM_TPS_MODE;

typedef enum
{
    OFDM_CODERATE_1_2,
    OFDM_CODERATE_2_3,
    OFDM_CODERATE_3_4,
    OFDM_CODERATE_5_6,
    OFDM_CODERATE_7_8
} SCD_OFDM_CODERATE;

typedef enum
{
    OFDM_HIERARCHY_NONE,
    OFDM_HIERARCHY_1,
    OFDM_HIERARCHY_2,
    OFDM_HIERARCHY_4
} SCD_OFDM_HIERARCHY;

typedef enum
{
    OFDM_CONSTEL_DQPSK,
    OFDM_CONSTEL_QPSK,
    OFDM_CONSTEL_QAM16,
    OFDM_CONSTEL_QAM64
} SCD_OFDM_CONSTEL;

typedef enum
{
    OFDM_RS_NORMMAL,
    OFDM_RS_LAYERA,
    OFDM_RS_LAYERB,
    OFDM_RS_LAYERC
} SCD_OFDM_RS_LAYER;

/****************************************************************************/
/* integer types (from HAL) */
/****************************************************************************/

#include "btfe_scd_hal_priv.h"

/****************************************************************************/
/* version type */
/****************************************************************************/

typedef struct tag_SCD_VERSION
{
    const char *name;
    uint32_t major;
    uint32_t customer;
    uint32_t minor;
    uint32_t device_id;
} SCD_VERSION;

/****************************************************************************/
/* configuration types */
/****************************************************************************/

typedef enum
{
   SCD_CONFIG_ITEM__TUNER_AGC_CHARGEPUMP,
   SCD_CONFIG_ITEM__BERT,
   SCD_CONFIG_ITEM__GPIO,
   SCD_CONFIG_ITEM__FDC_DATA,
   SCD_CONFIG_ITEM__FDC_AGC,
   SCD_CONFIG_ITEM__FAT_DATA,
   SCD_CONFIG_ITEM__FAT_AGC,
   SCD_CONFIG_ITEM__AGC_SCRIPT,
   SCD_CONFIG_ITEM__SMART_ANT,
   SCD_CONFIG_ITEM__SMART_ANT_OPERATION,
   SCD_CONFIG_ITEM__TOP_AGC,
   SCD_CONFIG_ITEM__CHANNEL_SCAN_CONTROL,
   SCD_CONFIG_ITEM__SET_TUNER,
   SCD_CONFIG_ITEM__ACQUISITION,
   SCD_CONFIG_ITEM__J83ABC,
   SCD_CONFIG_ITEM__UNIFIED_COFDM,
   SCD_CONFIG_ITEM__GPIO_MONITOR,
   SCD_CONFIG_ITEM__GPIO_WRITE,
   SCD_CONFIG_ITEM__ENABLE_MICRO_INT,
   SCD_CONFIG_ITEM__CLEAR_MICRO_INT,
   SCD_CONFIG_ITEM__SET_IF,
   SCD_CONFIG_ITEM__AUDIO_MAG_SHIFT,
   SCD_CONFIG_ITEM__ISDBT_BUFFER,
   SCD_CONFIG_ITEM__TRY_NEXT_SCAN,
   SCD_CONFIG_ITEM__J83_NEXT_SCAN,
   SCD_CONFIG_ITEM__POWER_LEVEL,
   SCD_CONFIG_ITEM__RF_OFFSET,
   SCD_CONFIG_ITEM__POWER_SAVING
} SCD_CONFIG_ITEM;

typedef uint8_t SCD_CONFIG__AUDIO_MAG_SHIFT;

typedef struct  tag_SCD_CONFIG__ISDBT_BUFFER
{
    uint32_t      alignment;
    uint32_t      bufferPtr;
} SCD_CONFIG__ISDBT_BUFFER;


typedef struct tag_SCD_CONFIG__BERT
{
    uint32_t            HeaderRemoval;
    SCD_BERT_INPUT    InputSelect;
    bool              PN_Inversion;
    bool              PN_Selection;   /* TBD */
    bool              ON_Flag;
    uint32_t            SyncErrorThreshold;
    uint32_t            SyncAcquireCounter;
    uint32_t            SyncLossCounter;
    uint32_t            WindowSize;
} SCD_CONFIG__BERT;

typedef struct tag_SCD_CONFIG__GPIO
{
    uint32_t  ownershipMask; /*  0 - Maintains its assigned function and controlled by the firmware
                               1 - Under application control */ 
    uint32_t  inputMask;     /*  0 - User controlled output
                               1 - User controlled input */ 
    uint32_t  outputType;    /*  0 - open drain 
                               1 - drive active high and low */
} SCD_CONFIG__GPIO;
typedef struct tag_SCD_DATA__GPIO
{
    uint32_t  gpioData;
                               
} SCD_DATA__GPIO;

typedef struct tag_SCD_STATUS__MICRO_INTERRUPT
{
	bool	sync_lock;
	bool    done;
    bool    sync_loss;
} SCD_DATA__MICRO_INTERRUPT;

typedef struct tag_SCD_CONFIG__FDC_DATA
{
    uint32_t               refDivider;       /* refDivider value to use. Valid values are 8, 16, 32, 64, or 0 to leave unchanged */
    SCD_FDC_SYMBOL_RATE  FdcSymbolRate;    /* 772, 1024 or 1544 for the symbol rate */
    SCD_SIGNAL_POLARITY  FdcClockPolarity; /* Polarity of the clock signal */
    SCD_SIGNAL_POLARITY  FdcDataPolarity;  /* Polarity of the data signal */
    SCD_SIGNAL_POLARITY  FdcDecoderMode;   /* Normal or inverted signal spectrum, value typically received from POD while tuning */
} SCD_CONFIG__FDC_DATA;

typedef  struct tag_SCD_CONFIG__FDC_AGC
{
    SCD_SIGNAL_POLARITY  agcFdcSdm1;
} SCD_CONFIG__FDC_AGC;

typedef struct tag_SCD_CONFIG__FAT_DATA
{
    SCD_SIGNAL_POLARITY  dataPolarity;    /* Polarity of the data signal */
    SCD_SIGNAL_POLARITY  errorPolarity;   /* Polarity of the Mpeg Packet error indicating signal */
    SCD_SIGNAL_POLARITY  clockPolarity;   /* Polarity of the clock signal */
    SCD_SIGNAL_POLARITY  syncPolarity;    /* Polarity of the MPEG packet sync present signal */
    SCD_SIGNAL_POLARITY  validPolarity;   /* Polarity of the valid data present signal */
    SCD_BURST_MODE       BurstMode;       /* The burst mode */
    bool                 GatedClockEnable;      /* The Clock is gated, and only active during valid MPEG data */
    bool                 ParallelOutputEnable;  /* Parallel Mode is selected, otherwise serial */
    bool                 HeaderEnable;          /* BERT mode, enables FAT channel MPEG packet first byte 0x47 */
    bool                 CableCardBypassEnable; /* if Enabled, puts the X210VC in CableCard bypass mode on the Xport A1 */
    bool                 FlipOrder;
    bool                 MpegOutputEnable;
    SCD_SIGNAL_STRENGTH  dataStrength;
    SCD_SIGNAL_STRENGTH  errorStrength;
    SCD_SIGNAL_STRENGTH  clockStrength;
    SCD_SIGNAL_STRENGTH  syncStrength;
    SCD_SIGNAL_STRENGTH  validStrength;
} SCD_CONFIG__FAT_DATA;

typedef struct tag_SCD_CONFIG__FAT_AGC
{
    SCD_SIGNAL_POLARITY  agcSdm1;          
    SCD_SIGNAL_POLARITY  agcSdm2;
    SCD_SIGNAL_POLARITY  agcSdmX;
    SCD_SIGNAL_POLARITY  agcSdmA;
} SCD_CONFIG__FAT_AGC;

typedef struct tag_SCD_CONFIG__AGC_SCRIPT
{
    int32_t *pdata;
} SCD_CONFIG__AGC_SCRIPT;

typedef struct tag_SCD_CONFIG__TOP_AGC
{
    uint32_t             tResponse;       /* response time to changes in mode (ie. 8-VSB->64-QAM) */
    uint32_t             tMinPeriod;      /* min T between TOP loop executions */
    uint32_t             tMaxPeriod;      /* max T between TOP loop executions */
    uint32_t             Sdm1Hysterisis;  /* IF AGC hysterisis around the TOP */
    uint32_t             IfTop8Vsb;       /* IF AGC TOP value for 8-VSB */
    uint32_t             IfTop64Qam;      /* IF AGC TOP value for 64-QAM */
    uint32_t             IfTop256Qam;     /* IF AGC TOP value for 256-QAM */
    SCD_TOP_AGC_STATE  state;
    uint32_t             tCurPeriod;
    uint32_t             hiThrSDM2;
    uint32_t             loThrSDM2;
    uint32_t             minSDM2;
    uint32_t             maxSDM2;
} SCD_CONFIG__TOP_AGC;

typedef struct tag_SCD_CONFIG__CHANNEL_SCAN_CONTROL
{
    bool            tryFullAcq;
    uint32_t          ifPower;
    SCD_SCAN_ORDER  scanOrder;
/*
    SCD_MOD_FORMAT  mod_1;
    SCD_MOD_FORMAT  mod_2;
    SCD_MOD_FORMAT  mod_3;
*/
} SCD_CONFIG__CHANNEL_SCAN_CONTROL;

typedef struct tag_SCD_CONFIG__ACQUISITION
{
    SCD_ACQUIRE_CONFIG acqConfig;
    SCD_BAND_WIDTH bandWidthConfig;
    bool bSpectrumInversion;
    bool bSpectrumAutoDetect;
    uint32_t agcDelay;
	
    /* The following items will move or deleted */
    bool bCoChannelRejection; /* X cofdm */
    bool bAdjChannelRejection; /* X cofdm */
	
    bool bMobileMode;  /* cofdm */
    bool bEnhancedMode; /* cofdm */
    bool bLowPriority; /* cofdm */
	
    SCD_IF_FREQUENCY ifFrequency; /* unused */	
    SCD_TUNE_MODE TuneMode;  /* unused */
	bool bLegacyAGC; /* not used */
} SCD_CONFIG__ACQUISITION;

typedef struct tag_SCD_CONFIG__J83ABC
{
    SCD_J83ABC_MODE	j83abcMode;   /* 0 - J83A, 1 - J83B, 2 - J83C */
    SCD_QAM_MODE_DIGITAL	qamMode; /* 0 - 16QAM, 1-32QAM, 2-64QAM, 3-128QAM, 4-256QAM */
    uint32_t	symbolRate;	
} SCD_CONFIG__J83ABC;

typedef struct tag_SCD_CONFIG__UNIFIED_COFDM
{
    /* 0 -7 */
    SCD_OFDM_STANDARD	ofdmStandard;
    SCD_OFDM_CCI_MODE	cciEnable;
    SCD_OFDM_ACI_MODE	aciEnable;
    SCD_OFDM_MOBIL_MODE	 mobilMode;
    SCD_OFDM_PRIORITY_MODE	priorityMode;
    SCD_OFDM_CARRIER_RANGE	carrierRange;
    SCD_OFDM_IMPULSE		impluse;
    SCD_OFDM_RS_LAYER	rsLayer;
    /* 8-15 */
    SCD_OFDM_MODEGUARD	modeGuard; /* item 9-10 are applied when modeGuard is manual */
    SCD_OFDM_MODE		mode;
    SCD_OFDM_GUARD		guard;
    SCD_OFDM_TPS_MODE	tpsMode; /* item 12-27 are applied when tpsMode is manual */
    SCD_OFDM_CODERATE	codeLP; /* TPS */
    SCD_OFDM_CODERATE	codeHP;
    SCD_OFDM_HIERARCHY	hierarchy;
    SCD_OFDM_CONSTEL	modulation; /* QPSK, Qam16, Qam64 */
    /* 16-23 */	
    SCD_OFDM_CONSTEL	modulationLayerA;
    SCD_OFDM_CONSTEL	modulationLayerB;
    SCD_OFDM_CONSTEL	modulationLayerC;
    SCD_OFDM_CODERATE	codeRateLayerA;
    SCD_OFDM_CODERATE	codeRateLayerB;
    SCD_OFDM_CODERATE	codeRateLayerC;
    uint8_t				segmentLayerA; /* TMCC, value 0-13 and A+B+C=13 */
    uint8_t				segmentLayerB;
    /* 24-28 */
    uint8_t				segmentLayerC;
    uint8_t				timeInterleaveLayerA; /* 0-3*/
    uint8_t				timeInterleaveLayerB; /* 0-3*/
    uint8_t				timeInterleaveLayerC; /* 0-3*/
    bool				partialReception; /* true or false */
} SCD_CONFIG__UNIFIED_COFDM;

typedef struct tag_SCD_CONFIG__GPIO_MONITOR
{
	uint8_t	delay;
	uint8_t	gpio_first;
	uint8_t	gpio_second;
	uint8_t	max_readings;
} SCD_CONFIG__GPIO_MONITOR;


typedef struct tag_SCD_CONFIG_J83_NEXT_SCAN
{
	
	SCD_ACQUIRE_CONFIG	acqMode;
	SCD_J83ABC_MODE		j83abcMode; 
	SCD_QAM_MODE_DIGITAL	qamMode;
	SCD_BAND_WIDTH		bandwidth;
	int32_t					carrierOffset;
	uint32_t					symbolRate;
} SCD_CONFIG_J83_NEXT_SCAN;

typedef struct tag_SCD_CONFIG__RF_OFFSET
{
    int32_t		freqOffset;	
    uint32_t	symbolRate;	
} SCD_CONFIG__RF_OFFSET;

/****************************************************************************/
/* status types */
/****************************************************************************/

typedef enum
{
    SCD_STATUS_ITEM__AGC_INDICATOR,
    SCD_STATUS_ITEM__SMART_ANT,
    SCD_STATUS_ITEM__TUNER_AGC,
    SCD_STATUS_ITEM__BERT,
    SCD_STATUS_ITEM__PSD_FRONTEND,
    SCD_STATUS_ITEM__EQ_TAPS,
    SCD_STATUS_ITEM__CONSTELLATION_DATA,
    SCD_STATUS_ITEM__TOP_AGC_CALIB_RESULT,
    SCD_STATUS_ITEM__TOP_AGC_CONFIG,
    SCD_STATUS_ITEM__FAT,
    SCD_STATUS_ITEM__FDC,
	SCD_STATUS_ITEM__GPIO_MONITOR,
	SCD_STATUS_ITEM__GPIO_READ,
	SCD_STATUS_ITEM__MEMORY_READ,
	SCD_STATUS_ITEM__MICRO_INTERRUPT,
    SCD_STATUS_ITEM__EQ_TAPS_PLUS,
	SCD_STATUS_ITEM__CHC_OUTPUT,
	SCD_STATUS_ITEM__SYR_OUTPUT,
	SCD_STATUS_ITEM__ACQUIRE_MODE,
	SCD_STATUS_ITEM__J83ABC,
	SCD_STATUS_ITEM__VSB,
	SCD_STATUS_ITEM__COFDM
} SCD_STATUS_ITEM;

#define SCD_STATUS_AGC__SDM1         (1L<<0)
#define SCD_STATUS_AGC__SDM2         (1L<<1)
#define SCD_STATUS_AGC__SDMX         (1L<<2)
#define SCD_STATUS_AGC__INTERNAL_AGC (1L<<3)
#define SCD_STATUS_AGC__ADC_MIN      (1L<<4)
#define SCD_STATUS_AGC__ADC_MAX      (1L<<5)
#define SCD_STATUS_AGC__ADC_POWER    (1L<<6)
#define SCD_STATUS_AGC__PDET_POWER   (1L<<7)
#define SCD_STATUS_AGC__ANALOG_PVID  (1L<<29)


typedef struct tag_SCD_STATUS__AGC_INDICATOR
{
    uint32_t  Flags;          /* combination of SCD_STATUS_AGC__* bits */
    uint32_t  SDM1;           /* FAT only */
    uint32_t  SDM2;           /* FAT only */
    uint32_t  SDMX;           /* FAT only */
    uint32_t  InternalAGC;    /* FDC only */
    int32_t   AdcMin;         /* FAT only */
    int32_t   AdcMax;         /* FAT only */
    uint32_t  AdcPower;
    uint32_t  PdetPower;      /* FAT only */
	int32_t	VidPower;		/* FAT modes that use UAGC only   S+[29:-1] */
	uint16_t   vdcLevel;
} SCD_STATUS__AGC_INDICATOR;

typedef struct tag_SCD_STATUS__MEMORY_READ
{
	uint32_t	offset;
	uint32_t  size;
    uint8_t*  values;
} SCD_STATUS__MEMORY_READ;

typedef struct tag_SCD_STATUS__TUNER_AGC
{
    int32_t*  agcData;
} SCD_STATUS__TUNER_AGC;

typedef struct tag_SCD_STATUS__BERT
{
    SCD_LOCK_STATUS LockStatus;
    uint32_t          ErrorCount;
} SCD_STATUS__BERT;

typedef struct tag_SCD_STATUS__PSD
{
    int32_t  power_spectrum_data[PSDARRAYSIZE];   
} SCD_STATUS__PSD;

typedef struct tag_SCD_STATUS___EQ_TAPS
{
    int32_t  taps[TAPARRAYSIZE];
    int32_t  adjustment[TAPARRAYSIZE];
} SCD_STATUS__EQ_TAPS;

typedef struct tag_SCD_STATUS___EQ_TAPS_PLUS
{
    int32_t  taps[TAPARRAYSIZE_2];
	int32_t  adjustment[TAPARRAYSIZE_2];
	int32_t  avgnorm[TAPARRAYSIZE_2];
} SCD_STATUS__EQ_TAPS_PLUS;

typedef struct tag_SCD_STATUS__CHC
{
    uint8_t  CHCP_Output_data[CHCPOUTPUTARRAYSIZE];   
} SCD_STATUS__CHC;

typedef struct tag_SCD_STATUS__CONSTELLATION_DATA
{
    int32_t    constX;
    int32_t    constY;
} SCD_STATUS__CONSTELLATION_DATA;

typedef struct tag_SCD_STATUS__TOP_AGC_CALIB_RESULT
{
    uint32_t      topIFAGC;     /* value of IF AGC register */
} SCD_STATUS__TOP_AGC_CALIB_RESULT;

typedef struct tag_SCD_STATUS__GPIO_MONITOR
{
    uint8_t  numBits;   /* number of bits read */
    uint8_t  data1;	  /* data on first pin 0-7 */
    uint8_t  data2;	  /* data on second pin 0-7 or data on first pin 8-15 (depneds on numBits) */
    uint8_t  data3;     /* not used if numBits <= 8 */
    uint8_t  data4;
    uint8_t  data5;
	uint8_t  data6;
	uint8_t  data7;
	uint8_t  data8;
} SCD_STATUS__GPIO_MONITOR;

typedef struct tag_SCD_STATUS__ACQUIRE_MODE
{
    uint32_t      acquireMode;
} SCD_STATUS__ACQUIRE_MODE;

typedef struct tag_SCD_STATUS__TOP_AGC_CONFIG
{
    uint32_t             tResponse;       /* response time to changes in mode (ie. 8-VSB->64-QAM) */
    uint32_t             tMinPeriod;      /* min T between TOP loop executions */
    uint32_t             tMaxPeriod;      /* max T between TOP loop executions */
    uint32_t             Sdm1Hysterisis;  /* IF AGC hysterisis around the TOP */
    uint32_t             IfTop8Vsb;       /* IF AGC TOP value for 8-VSB */
    uint32_t             IfTop64Qam;      /* IF AGC TOP value for 64-QAM */
    uint32_t             IfTop256Qam;     /* IF AGC TOP value for 256-QAM */
    SCD_TOP_AGC_STATE  state;
    uint32_t             tCurPeriod;
    uint32_t             hiThrSDM2;
    uint32_t             loThrSDM2;
    uint32_t             minSDM2;
    uint32_t             maxSDM2;
} SCD_STATUS__TOP_AGC_CONFIG;

#define SCD_STATUS_FAT__LOCK_STATUS          (1L<<0)
#define SCD_STATUS_FAT__DEMOD_FORMAT         (1L<<1)
#define SCD_STATUS_FAT__SPECTRUM_POLARITY    (1L<<2)
#define SCD_STATUS_FAT__EQUALIZER_SNR        (1L<<3)
#define SCD_STATUS_FAT__TIMING_OFFSET        (1L<<4)
#define SCD_STATUS_FAT__PILOT_OFFSET         (1L<<5)
#define SCD_STATUS_FAT__ERRORS               (1L<<6)
#define SCD_STATUS_FAT__COARSE_OFFSET        (1L<<7)
#define SCD_STATUS_FAT__IAGC_GAIN            (1L<<8)
#define SCD_STATUS_FAT__DUR                  (1L<<9)
#define SCD_STATUS_FAT__PILOT_AMPLITUDE      (1L<<10)
#define SCD_STATUS_FAT__EQ_CURSOR            (1L<<11)
#define SCD_STATUS_FAT__PILOT_ESTIMATE       (1L<<12)
#define SCD_STATUS_FAT__ATSM_STATE           (1L<<13)
#define SCD_STATUS_FAT__DFS                  (1L<<14)
#define SCD_STATUS_FAT__QAM_INTERLEAVER_MODE (1L<<15)
#define SCD_STATUS_FAT__ACB                  (1L<<16)
#define SCD_STATUS_FAT__CARRIER_OFFSET       (1L<<17)
#define SCD_STATUS_FAT__AGC_SETTLE_TIME      (1L<<18)
#define SCD_STATUS_FAT__COFDM_MOD_FORMAT     (1L<<19)
#define SCD_STATUS_FAT__COFDM_HIERARCHY        (1L<<20)
#define SCD_STATUS_FAT__COFDM_MODE           (1L<<21)
#define SCD_STATUS_FAT__COFDM_GUARD_INT      (1L<<22)
#define SCD_STATUS_FAT__COFDM_CODE_RATE      (1L<<23)
#define SCD_STATUS_FAT__RESERVED_1           (1L<<24)
#define SCD_STATUS_FAT__SAMPLE_FREQUENCY     (1L<<25)
#define SCD_STATUS_FAT__TARGET_IF_FREQUENCY  (1L<<26)
#define SCD_STATUS_FAT__SYMBOL_RATE          (1L<<27)
#define SCD_STATUS_FAT__NORMALIZED_IF        (1L<<28)
#define SCD_STATUS_FAT__COFDM_BER            (1L<<29)
#define SCD_STATUS_FAT__SQI                  (1L<<30)
#define SCD_STATUS_FAT__IFD_LOCK_STATUS      (1L<<31)

typedef struct tag_SCD_STATUS__FAT
{
    uint32_t                 Flags;                    /* combination of SCD_STATUS_FAT__* bits */
    bool                   Started;
    SCD_LOCK_STATUS        LockStatus;
    SCD_MOD_FORMAT         DemodulationFormat;
    uint32_t                 RecommendedTimeoutValue;  /* lock wait time */
    SCD_SIGNAL_POLARITY    SpectrumPolarity;
    uint32_t                 EqualizerSNR;             /* VSB/QAM/COFDM */
    int32_t                  TimingOffset;
    int32_t                  PilotOffset;
    uint32_t                 RSUncorrectableErrorsA;    /* VSB/QAM/COFDM */
    uint32_t                 RSUncorrectableErrorsB;
    uint32_t                 RSUncorrectableErrorsC;
    uint32_t                 RSCorrectableErrorsA;      /* VSB/QAM */
    uint32_t                 RSCorrectableErrorsB;
    uint32_t                 RSCorrectableErrorsC;
    int32_t                  CoarseOffset;
    int32_t                  IAGCGain;                 /* VSB/QAM */
    int32_t                  DUR;                      /* VSB/QAM */
    int32_t                  PilotAmplitude;           /* VSB/QAM */
    int32_t                  EqCursor;                 /* VSB/QAM */
    int32_t                  PilotEstimate;            /* VSB/QAM */
    int32_t                  ATSMstate;                /* VSB/QAM */
    int32_t                  DFSstate;                 /* VSB/QAM */
    int32_t                  DFSpolarity;              /* VSB/QAM */
    int32_t                  QAMinterleaverMode;       /* QAM */
    uint32_t                 ACBState;
    uint32_t                 ACBStatus;
    uint32_t                 ACBTimer;
    uint32_t                 ACBAcqTime;
    uint32_t                 ACBNumReacqs;
    int32_t                  CarrierOffset;
    uint32_t                 AgcSettleTime;            /* VSB/QAM */
    uint32_t                 SampleFrequency;
    uint32_t                 TargetIfFrequency;        
    uint32_t                 SymbolRate;
    SCD_OFDM_CONSTEL   COFDMModFormat;			 /* COFDM */
    SCD_OFDM_HIERARCHY	   COFDMHierarchy;			 /* COFDM */
    SCD_OFDM_MODE         COFDMMode;                /* COFDM */
    SCD_OFDM_GUARD        COFDMGuardInt;            /* COFDM */
    SCD_OFDM_CODERATE   COFDMCodeRate;            /* COFDM */
    
    SCD_OFDM_CONSTEL   COFDMModFormatA;			 /* ISDBT */
    SCD_OFDM_CONSTEL   COFDMModFormatB;			 /* ISDBT */
    SCD_OFDM_CONSTEL   COFDMModFormatC;			 /* ISDBT */
	
    SCD_OFDM_CODERATE   COFDMCodeRateA;            /* ISDBT */
    SCD_OFDM_CODERATE   COFDMCodeRateB;            /* ISDBT */
    SCD_OFDM_CODERATE   COFDMCodeRateC;            /* ISDBT */

	SCD_COFDM_TDI          COFDMTdiA;                 /* ISDBT */
	SCD_COFDM_TDI          COFDMTdiB;                 /* ISDBT */
	SCD_COFDM_TDI          COFDMTdiC;                 /* ISDBT */

    uint32_t 			   COFDMSegA; 				  /* ISDBT */
    uint32_t 			   COFDMSegB; 				  /* ISDBT */
    uint32_t 			   COFDMSegC; 				  /* ISDBT */

    int32_t                  NormalizedIF;
    uint32_t                 NumRSpacketsA;             /* VSB/QAM/COFDM */
    uint32_t                 NumRSpacketsB; 
    uint32_t                 NumRSpacketsC;
    uint32_t                 COFDMBerErrCnt;           /* COFDM */
    uint32_t                 COFDMBerPktCnt;           /* COFDM */

    bool                       ews;
    bool                       partialReception;
    uint32_t                  cellId;
    bool                       demodSpectrum;

    /* only use in SCD, not in TFE */	
    uint32_t                 SignalQualityIndex;       /* ALL */	
    uint32_t                 DemodInputReference;      /* ALL - in milliVolts */
	
    uint8_t			    j83Mode; /* J83a, j83b, or j83c */
    uint8_t			    qamMode; /* qam16, qam32, qam64, qam128, qam256 */
    uint8_t			    confirmModulator; /* signalDetect, confirmVSB, scanResult */

    uint32_t                 reserved1;
    uint32_t                 reserved2;

    uint8_t			    IFDlockStatus; /* IFD  */
} SCD_STATUS__FAT;

#define SCD_STATUS_FDC__LOCK_STATUS          (1L<<0)
#define SCD_STATUS_FDC__DEMOD_FORMAT         (1L<<1)
#define SCD_STATUS_FDC__SPECTRUM_POLARITY    (1L<<2)
#define SCD_STATUS_FDC__EQUALIZER_SNR        (1L<<3)
#define SCD_STATUS_FDC__TIMING_OFFSET        (1L<<4)
#define SCD_STATUS_FDC__CARRIER_OFFSET       (1L<<5)
#define SCD_STATUS_FDC__INTERNAL_AGC         (1L<<6)
#define SCD_STATUS_FDC__N_FRAMES             (1L<<7)
#define SCD_STATUS_FDC__WIN_SIZE             (1L<<8)
#define SCD_STATUS_FDC__SYMBOL_RATE          (1L<<9)
#define SCD_STATUS_FDC__ACB                  (1L<<10)
#define SCD_STATUS_FDC__SAMPLE_FREQUENCY     (1L<<11)

typedef struct tag_SCD_STATUS__FDC
{
    uint32_t                 Flags;                    /* combination of SCD_STATUS_FDC__* bits */
    bool                   Started;
    SCD_LOCK_STATUS        LockStatus;
    SCD_MOD_FORMAT         DemodulationFormat;
    uint32_t                 RecommendedTimeoutValue;  /* lock wait time */
    SCD_SIGNAL_POLARITY    SpectrumPolarity;
    uint32_t                 EqualizerSNR;
    int32_t                  TimingOffset;
    int32_t                  CarrierOffset;
    uint32_t                 InternalAGC;
    uint32_t                 NFrames;
    uint32_t                 WinSize;
    SCD_FDC_SYMBOL_RATE    SymbolRate;
    uint32_t                 ACBState;
    uint32_t                 ACBStatus;
    uint32_t                 ACBTimer;
    uint32_t                 ACBAcqTime;	
    uint32_t                 ACBNumReacqs;
    uint32_t                 SampleFrequency;
} SCD_STATUS__FDC;

typedef struct tag_SCD_XPROP_TUNER_IF__DATA
{
    uint8_t                 tagSize;
    uint8_t                 current_mod;
    uint8_t                 spectrumInv;	
    uint8_t                 reserved;
    uint32_t                centerIF;
    int32_t                 IFshift;
} SCD_XPROP_TUNER_IF__DATA;

typedef struct
{ 
	char  name[12];   /* name in the spec for this entry */
	uint8_t topCTRL1;   /* value for reg_A (flags) */
	uint8_t topCTRL2;   /* value for reg_B (flags) */
	uint8_t reduction;  /* dB backoff from the MAX for this entry */
	uint8_t reserved;   /* some tuners may need additional switch */
} RF_TOP_DATA;

typedef struct tag_SCD_STATUS__J83ABC
{
   uint8_t                		reacqCounter;
   SCD_ACQUIRE_CONFIG	acqConfig;
   SCD_BAND_WIDTH		bandwidthConfig;
   bool                   		bSpectralInversion; /* true if inverted */
   SCD_J83ABC_MODE          mode;
   SCD_QAM_MODE_DIGITAL constellation;
   bool					bOperationDone;  /* applies in BTFE_AcquireConfig_eSymbolRateVerify and BTFE_AcquireConfig_eSymbolRateScan */
   bool					bSignalDetected;
   int8_t					bandEdgePosPower;
   int8_t					bandEdgeNegPower;
   uint32_t				IFNomRate;
   uint32_t				baudRateDetected; /* detected symbol rate in Hz when acquire config is BTFE_AcquireConfig_eSymbolRateScan */
   uint32_t				rateNomFinal;
} SCD_STATUS__J83ABC;


typedef struct tag_TAG_SCD_STATUS_VSB
{
   bool                bOperationDone;
   bool                bConfirmVSB;  
} SCD_STATUS_VSB;


typedef struct tag_SCD_STATUS_COFDM
{
   bool			bOperationDone;
   uint8_t			scanResult;
   bool			bSpectraInverted;
   int32_t			carrierOffset;
   int16_t			timingOffset;
} SCD_STATUS_COFDM;


/****************************************************************************/
/* SCD API prototypes */
/****************************************************************************/

SCD_RESULT BTFE_P_ScdGetVersion(SCD_ITEM item, uint32_t instance, SCD_VERSION *version);

SCD_RESULT BTFE_P_ScdInitialize(uint32_t flags, void *reg_Handle);
SCD_RESULT BTFE_P_ScdCleanup(void);

SCD_RESULT BTFE_P_ScdOpenFat(uint32_t instance, SCD_HANDLE *fat_handle);
SCD_RESULT BTFE_P_ScdCloseFat(SCD_HANDLE handle);

SCD_RESULT BTFE_P_ScdOpenChip(uint32_t instance, SCD_HANDLE *chip_handle);
SCD_RESULT BTFE_P_ScdCloseChip(SCD_HANDLE handle);

SCD_RESULT BTFE_P_ScdStart(SCD_HANDLE handle, SCD_MOD_FORMAT format);
SCD_RESULT BTFE_P_ScdStop(SCD_HANDLE handle);

SCD_RESULT BTFE_P_ScdSetConfig(SCD_HANDLE handle, SCD_CONFIG_ITEM item, void *data, uint32_t size);
SCD_RESULT BTFE_P_ScdGetStatus(SCD_HANDLE handle, SCD_STATUS_ITEM item, void *data, uint32_t size);

SCD_RESULT BTFE_P_ScdGetChip(SCD_HANDLE handle, SCD_HANDLE *chip_handle);

SCD_RESULT BTFE_P_ScdWriteGpio(SCD_HANDLE handle, uint32_t mask, uint32_t value);
SCD_RESULT BTFE_P_ScdReadGpio(SCD_HANDLE handle, uint32_t mask, uint32_t *value);

SCD_RESULT BTFE_P_ScdWriteChip(SCD_HANDLE handle, uint32_t aper, uint32_t offset, uint32_t length, uint8_t *buffer);
SCD_RESULT BTFE_P_ScdReadChip(SCD_HANDLE handle, uint32_t aper, uint32_t offset, uint32_t length, uint8_t *buffer);

SCD_RESULT BTFE_P_ScdWriteI2C(SCD_HANDLE handle, uint32_t i2c_addr, uint32_t i2c_subaddr, uint32_t subaddr_len, uint32_t data_len, uint8_t *buffer);
SCD_RESULT BTFE_P_ScdReadI2C(SCD_HANDLE handle, uint32_t i2c_addr, uint32_t i2c_subaddr, uint32_t subaddr_len, uint32_t data_len, uint8_t *buffer);

/****************************************************************************/

#ifdef __cplusplus
}
#endif

/****************************************************************************/

#endif /* SCD_H */

/****************************************************************************/

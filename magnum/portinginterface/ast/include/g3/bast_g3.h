/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#ifndef _BAST_G3_H__
#define _BAST_G3_H__

#include "bast.h"


/******************************************************************************
Summary:
   configuration parameter IDs
Description:
   These are the configuration parameters that can be accessed via
   BAST_ReadConfig() and BAST_WriteConfig().
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_G3_CONFIG_XTAL_FREQ                 0x0000 /* crystal freq in Hz */
#define BAST_G3_CONFIG_LEN_XTAL_FREQ             4
#define BAST_G3_CONFIG_MISC_CTL                  0x0001 /* miscellaneous acquisition settings, see BAST_G3_MISC_CTL_* macros */
#define BAST_G3_CONFIG_LEN_MISC_CTL              1
#define BAST_G3_CONFIG_RAIN_FADE_THRESHOLD       0x0002 /* SNR drop threshold for the rain fade indicator status in units of 1/8 dB SNR */
#define BAST_G3_CONFIG_LEN_RAIN_FADE_THRESHOLD   1
#define BAST_G3_CONFIG_RAIN_FADE_WINDOW          0x0003 /* time window for rain fade indicator status in units of 100 msecs */
#define BAST_G3_CONFIG_LEN_RAIN_FADE_WINDOW      4
#define BAST_G3_CONFIG_FREQ_DRIFT_THRESHOLD      0x0004 /* carrier offset threshold in Hz for frequency drift indicator status */
#define BAST_G3_CONFIG_LEN_FREQ_DRIFT_THRESHOLD  4
#define BAST_G3_CONFIG_STUFF_BYTES               0x0005 /* Number of null bytes to insert in each frame */
#define BAST_G3_CONFIG_LEN_STUFF_BYTES           1
#define BAST_G3_CONFIG_ACQ_TIME                  0x0006 /* acquisition time in usecs */
#define BAST_G3_CONFIG_LEN_ACQ_TIME              4
#define BAST_G3_CONFIG_AGC_CTL                   0x0007 /* AGC control settings, see BAST_G3_CONFIG_AGC_CTL_* macros */
#define BAST_G3_CONFIG_LEN_AGC_CTL               1
#define BAST_G3_CONFIG_TUNER_CTL                 0x0008 /* tuner control settings, see BAST_G3_CONFIG_TUNER_CTL_* macros */
#define BAST_G3_CONFIG_LEN_TUNER_CTL             1
#define BAST_G3_CONFIG_REACQ_CTL                 0x0009 /* reacquisition options, see BAST_G3_CONFIG_REACQ_CTL_* macros */
#define BAST_G3_CONFIG_LEN_REACQ_CTL             1
#define BAST_G3_CONFIG_PLC_CTL                   0x000A /* PLC settings */
#define BAST_G3_CONFIG_LEN_PLC_CTL               1
#define BAST_G3_CONFIG_LDPC_CTL                  0x000B /* AFEC settings, see BAST_G3_CONFIG_LDPC_CTL_* macros */
#define BAST_G3_CONFIG_LEN_LDPC_CTL              1
#define BAST_G3_CONFIG_TURBO_CTL                 0x000C /* TFEC settings */
#define BAST_G3_CONFIG_LEN_TURBO_CTL             1
#define BAST_G3_CONFIG_PLC_ALT_ACQ_BW            0x000D /* Alternate acquisition PLC bandwidth in units of Hz.  This bandwidth applies when PLC_CTL[2]=1. */
#define BAST_G3_CONFIG_LEN_PLC_ALT_ACQ_BW        4
#define BAST_G3_CONFIG_PLC_ALT_ACQ_DAMP          0x000E /* Alternate acquisition PLC damping factor scaled 2^3 (e.g. damping factor of 1.0 is 0x08).  This damping factor only applies when PLC_CTL[2]=1. */
#define BAST_G3_CONFIG_LEN_PLC_ALT_ACQ_DAMP      1
#define BAST_G3_CONFIG_PLC_ALT_TRK_BW            0x000F /* Alternate tracking PLC bandwidth in units of Hz.  This bandwidth applies when PLC_CTL[3]=1. */
#define BAST_G3_CONFIG_LEN_PLC_ALT_TRK_BW        4
#define BAST_G3_CONFIG_PLC_ALT_TRK_DAMP          0x0010 /* Alternate tracking PLC damping factor scaled 2^3 (e.g. damping factor of 1.0 is 0x08).  This damping factor only applies when PLC_CTL[3]=1. */
#define BAST_G3_CONFIG_LEN_PLC_ALT_TRK_DAMP      1
#define BAST_G3_CONFIG_BLIND_SCAN_MODES          0x0011 /* Indicates which modes will be considered in the blind scan, see BAST_G3_CONFIG_BLIND_SCAN_MODES_* macros */
#define BAST_G3_CONFIG_LEN_BLIND_SCAN_MODES      1
#define BAST_G3_CONFIG_DTV_SCAN_CODE_RATES       0x0012 /* Selects the DTV code rates that are considered in the scan, see BAST_G3_CONFIG_DTV_SCAN_CODE_RATES_* macros */
#define BAST_G3_CONFIG_LEN_DTV_SCAN_CODE_RATES   1
#define BAST_G3_CONFIG_DVB_SCAN_CODE_RATES       0x0013 /* Selects the DVB-S code rates that are considered in the scan, see BAST_G3_CONFIG_DVB_SCAN_CODE_RATES_* macros */
#define BAST_G3_CONFIG_LEN_DVB_SCAN_CODE_RATES   1
#define BAST_G3_CONFIG_DCII_SCAN_CODE_RATES      0x0014 /* Selects the DCII code rates that are considered in the scan, see BAST_G3_CONFIG_DCII_SCAN_CODE_RATES_* macros */
#define BAST_G3_CONFIG_LEN_DCII_SCAN_CODE_RATES  1
#define BAST_G3_CONFIG_TURBO_SCAN_MODES          0x0015 /* Selects the Turbo modes that are considered in the scan, see BAST_G3_CONFIG_TURBO_SCAN_MODES_* macros */
#define BAST_G3_CONFIG_LEN_TURBO_SCAN_MODES      2
#define BAST_G3_CONFIG_LDPC_SCAN_MODES           0x0016 /* Selects the LDPC modes that are considered in the scan, see BAST_G3_CONFIG_LDPC_SCAN_MODES_* macros */
#define BAST_G3_CONFIG_LEN_LDPC_SCAN_MODES       2
#define BAST_G3_CONFIG_TUNER_FILTER_OVERRIDE     0x0017 /* Tuner low pass baseband filter cutoff frequency in MHz when TUNER_CTL[3] is set. */
#define BAST_G3_CONFIG_LEN_TUNER_FILTER_OVERRIDE 1
#define BAST_G3_CONFIG_FTM_TX_POWER              0x0018 /* FSK transmit power */
#define BAST_G3_CONFIG_LEN_FTM_TX_POWER          1
#define BAST_G3_CONFIG_FTM_CH_SELECT             0x0019 /* FSK channel select */
#define BAST_G3_CONFIG_LEN_FTM_CH_SELECT         1
#define BAST_G3_CONFIG_FREQ_ESTIMATE_STATUS      0x001A /* READ ONLY: status of DFT freq estimate */
#define BAST_G3_CONFIG_LEN_FREQ_ESTIMATE_STATUS  1
#define BAST_G3_CONFIG_IF_STEP_SAVE              0x001B /* coarse IF step determined by DFT freq estimate */
#define BAST_G3_CONFIG_LEN_IF_STEP_SAVE          4
#define BAST_G3_CONFIG_STATUS_INDICATOR          0x001C /* real time status indicators, see BAST_G3_CONFIG_STATUS_INDICATOR_* macros */
#define BAST_G3_CONFIG_LEN_STATUS_INDICATOR      1
#define BAST_G3_CONFIG_DFT_RANGE_START           0x001D /* Starting bin for DFT engine. */
#define BAST_G3_CONFIG_LEN_DFT_RANGE_START       2
#define BAST_G3_CONFIG_DFT_RANGE_END             0x001E /* Ending bin for DFT engine. */
#define BAST_G3_CONFIG_LEN_DFT_RANGE_END         2
#define BAST_G3_CONFIG_BCM3445_CTL               0x001F /* BCM3445 settings */
#define BAST_G3_CONFIG_LEN_BCM3445_CTL           1
#define BAST_G3_CONFIG_TUNER_FILCAL_UPPER        0x0020 /* upper threshold for tuner filter calibration */
#define BAST_G3_CONFIG_LEN_TUNER_FILCAL_UPPER    1
#define BAST_G3_CONFIG_TUNER_FILCAL_LOWER        0x0021 /* lower threshold for tuner filter calibration */
#define BAST_G3_CONFIG_LEN_TUNER_FILCAL_LOWER    1
#define BAST_G3_CONFIG_AMBIENT_TEMP              0x0022 /* configurable ambient temperature for tuner Vc adjustment */
#define BAST_G3_CONFIG_LEN_AMBIENT_TEMP          1
#define BAST_G3_CONFIG_DEBUG1                    0x0023 /* debug 1 */
#define BAST_G3_CONFIG_LEN_DEBUG1                4
#define BAST_G3_CONFIG_DEBUG2                    0x0024 /* debug 2 */
#define BAST_G3_CONFIG_LEN_DEBUG2                4
#define BAST_G3_CONFIG_ACM_MAX_MODE              0x0025 /* highest BAST_Mode in ACM transmission */
#define BAST_G3_CONFIG_LEN_ACM_MAX_MODE          1
#define BAST_G3_CONFIG_REACQ_CAUSE               0x0026 /* cause of reacquisition */
#define BAST_G3_CONFIG_LEN_REACQ_CAUSE           4
#define BAST_G3_CONFIG_TUNER_AGC_THRESH          0x0027 /* BB/LNA AGC clip detector threshold [31:16] for BB, [15:0] for LNA */
#define BAST_G3_CONFIG_LEN_TUNER_AGC_THRESH      4
#define BAST_G3_CONFIG_TUNER_DAISY_GAIN          0x0028 /* daisy output gain */
#define BAST_G3_CONFIG_LEN_TUNER_DAISY_GAIN      1
#define BAST_G3_CONFIG_TUNER_AGC_WIN_LEN         0x0029 /* BB/LNA AGC window length [31:16] for BB, [15:0] for LNA */
#define BAST_G3_CONFIG_LEN_TUNER_AGC_WIN_LEN     4
#define BAST_G3_CONFIG_TUNER_AGC_AMP_THRESH      0x002A /* BB/LNA AGC amplitude threshold [12:8] for BB, [4:0] for LNA */
#define BAST_G3_CONFIG_LEN_TUNER_AGC_AMP_THRESH  2
#define BAST_G3_CONFIG_TUNER_AGC_LOOP_COEFF      0x002B /* BB/LNA AGC loop coefficient [12:8] for BB, [4:0] for LNA */
#define BAST_G3_CONFIG_LEN_TUNER_AGC_LOOP_COEFF  2
#define BAST_G3_CONFIG_DEBUG3                    0x002C /* debug 3 */
#define BAST_G3_CONFIG_LEN_DEBUG3                4
#define BAST_G3_CONFIG_DFT_MIN_N                 0x002D
#define BAST_G3_CONFIG_LEN_DFT_MIN_N             1

#ifdef BAST_ENABLE_GENERIC_FSK
#define BAST_G3_CONFIG_FSK_TX_POWER       BAST_G3_CONFIG_FTM_TX_POWER
#define BAST_G3_CONFIG_LEN_FSK_TX_POWER   BAST_G3_CONFIG_LEN_FTM_TX_POWER
#define BAST_G3_CONFIG_FSK_CH_SELECT      BAST_G3_CONFIG_FTM_CH_SELECT
#define BAST_G3_CONFIG_LEN_FSK_CH_SELECT  BAST_G3_CONFIG_LEN_FTM_CH_SELECT

#define BAST_G3_CONFIG_FSK_TX_FREQ_HZ            0x002D /* FSK transmit frequency in Hz */
#define BAST_G3_CONFIG_LEN_FSK_TX_FREQ_HZ        4
#define BAST_G3_CONFIG_FSK_RX_FREQ_HZ            0x002E /* FSK receive frequency in Hz */
#define BAST_G3_CONFIG_LEN_FSK_RX_FREQ_HZ        4
#define BAST_G3_CONFIG_FSK_TX_DEV_HZ             0x002F /* FSK frequency deviation in Hz */
#define BAST_G3_CONFIG_LEN_FSK_TX_DEV_HZ         4
#endif


/******************************************************************************
Summary:
   BAST_G3_CONFIG_MISC_CTL configuration parameter bit definitions
Description:
   BAST_G3_CONFIG_MISC_CTL configuration parameter bit definitions
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_G3_CONFIG_MISC_CTL_DISABLE_SMART_TUNE  0x01 /* 1=disable Fs smart tuning */
#define BAST_G3_CONFIG_MISC_CTL_CHECK_TIMING_LOOP   0x02 /* 1=verify timing loop is locked */
#define BAST_G3_CONFIG_MISC_CTL_DISABLE_NOTCH       0x04 /* wideband tuners only: 1=disable dco notch filters */
#define BSAT_G3_CONFIG_MISC_CTL_PEAKSCAN_PSD        0x08 /* enable PSD mode in PeakScan function */
#define BAST_G3_CONFIG_MISC_CTL_OVERRIDE_FNDF       0x20 /* final non-decimating filter setting: 0=auto, 1=specified by bits 7:6 */
#define BAST_G3_CONFIG_MISC_CTL_FNDF_MASK           0xC0 /* final non-decimating filter configuration */
#define BAST_G3_CONFIG_MISC_CTL_FNDF_QUARTERBAND    0x00
#define BAST_G3_CONFIG_MISC_CTL_FNDF_THIRDBAND      0x40
#define BAST_G3_CONFIG_MISC_CTL_FNDF_HALFBAND       0x80
#define BAST_G3_CONFIG_MISC_CTL_FNDF_SHARP_HALFBAND 0xC0


/******************************************************************************
Summary:
   FSK channel select configuration
Description:
   This enum selects the FSK channel configuration within the chip's PHY core.
See Also:
   BAST_ResetFtm(),  BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
typedef enum BAST_G3_FskChannelConfig
{
   BAST_G3_FskChannelConfig_eCh0Tx_Ch0Rx = 0,
   BAST_G3_FskChannelConfig_eCh0Tx_Ch1Rx,
   BAST_G3_FskChannelConfig_eCh1Tx_Ch0Rx,
   BAST_G3_FskChannelConfig_eCh1Tx_Ch1Rx
} BAST_G3_FskChannelConfig;


/******************************************************************************
Summary:
   BAST_G3_CONFIG_TUNER_CTL configuration parameter bit definitions
Description:
   BAST_G3_CONFIG_TUNER_CTL configuration parameter bit definitions
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_G3_CONFIG_TUNER_CTL_LNA_PGA_MODE             0x01 /* 1=enable PGA, 0=enable LNA */
#define BAST_G3_CONFIG_TUNER_CTL_LNA_BYPASS               0x02 /* 1=bypass LNA */
#define BAST_G3_CONFIG_TUNER_CTL_SET_FILTER_MANUAL        0x08 /* 1=tuner LPF will be set to the value specified by TUNER_FILTER_OVERRIDE config parameter */
#define BAST_G3_CONFIG_TUNER_CTL_BYPASS_DFT_FREQ_EST      0x10 /* 1=bypass dft freq estimate */
#define BAST_G3_CONFIG_TUNER_CTL_PRESERVE_COMMANDED_FREQ  0x20 /* 1=tuner LO will be set to the commanded tuner freq */
#define BAST_G3_CONFIG_TUNER_CTL_DISABLE_VCO_AVOIDANCE    0x40 /* 1=disable VCO avoidance */
#define BAST_G3_CONFIG_TUNER_CTL_LPF_FAILSAFE_STATUS      0x80


/******************************************************************************
Summary:
   BAST_G3_CONFIG_LDPC_CTL configuration parameter bit definitions
Description:
   BAST_G3_CONFIG_LDPC_CTL configuration parameter bit definitions
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_G3_CONFIG_LDPC_CTL_DISABLE_AUTO_PILOT_PLL    0x01 /* 0=pilot pll selection automatically determined by AST PI */
#define BAST_G3_CONFIG_LDPC_CTL_DISABLE_PSL               0x02 /* 1=disable power saving loop */
#define BAST_G3_CONFIG_LDPC_CTL_SHORT_FRAME_DETECTED      0x80 /* READ ONLY: 1=AFEC short frame detected */


/******************************************************************************
Summary:
   BAST_G3_CONFIG_BLIND_SCAN_MODES configuration parameter bit definitions
Description:
   BAST_G3_CONFIG_BLIND_SCAN_MODES configuration parameter bit definitions
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_G3_CONFIG_BLIND_SCAN_MODES_DVB    0x01
#define BAST_G3_CONFIG_BLIND_SCAN_MODES_TURBO  0x02
#define BAST_G3_CONFIG_BLIND_SCAN_MODES_LDPC   0x04
#define BAST_G3_CONFIG_BLIND_SCAN_MODES_DTV    0x08
#define BAST_G3_CONFIG_BLIND_SCAN_MODES_DCII   0x10
#define BAST_G3_CONFIG_BLIND_SCAN_MODES_MASK   0x1F


/******************************************************************************
Summary:
   configuration parameter bit definitions for BAST_G3_CONFIG_DTV_SCAN_CODE_RATES
Description:
   bit definition for BAST_G3_CONFIG_DTV_SCAN_CODE_RATES
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_G3_CONFIG_DTV_SCAN_CODE_RATES_1_2  0x01 /* DTV 1/2 */
#define BAST_G3_CONFIG_DTV_SCAN_CODE_RATES_2_3  0x02 /* DTV 2/3 */
#define BAST_G3_CONFIG_DTV_SCAN_CODE_RATES_6_7  0x04 /* DTV 6/7 */
#define BAST_G3_CONFIG_DTV_SCAN_CODE_RATES_ALL  0x07 /* search all DTV modes */


/******************************************************************************
Summary:
   configuration parameter bit definitions for BAST_G3_CONFIG_DVB_SCAN_CODE_RATES
Description:
   bit definition for BAST_G3_CONFIG_DVB_SCAN_CODE_RATES
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_G3_CONFIG_DVB_SCAN_CODE_RATES_1_2  0x01 /* DVB 1/2 */
#define BAST_G3_CONFIG_DVB_SCAN_CODE_RATES_2_3  0x02 /* DVB 2/3 */
#define BAST_G3_CONFIG_DVB_SCAN_CODE_RATES_3_4  0x04 /* DVB 3/4 */
#define BAST_G3_CONFIG_DVB_SCAN_CODE_RATES_5_6  0x08 /* DVB 5/6 */
#define BAST_G3_CONFIG_DVB_SCAN_CODE_RATES_7_8  0x10 /* DVB 7/8 */
#define BAST_G3_CONFIG_DVB_SCAN_CODE_RATES_ALL  0x1F /* search all DVB-S modes */


/******************************************************************************
Summary:
   configuration parameter bit definitions for BAST_G3_CONFIG_DCII_SCAN_CODE_RATES
Description:
   bit definition for BAST_G3_CONFIG_DCII_SCAN_CODE_RATES
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_G3_CONFIG_DCII_SCAN_CODE_RATES_1_2  0x01 /* DCII 1/2 */
#define BAST_G3_CONFIG_DCII_SCAN_CODE_RATES_2_3  0x02 /* DCII 2/3 */
#define BAST_G3_CONFIG_DCII_SCAN_CODE_RATES_3_4  0x04 /* DCII 3/4 */
#define BAST_G3_CONFIG_DCII_SCAN_CODE_RATES_5_6  0x08 /* DCII 5/6 */
#define BAST_G3_CONFIG_DCII_SCAN_CODE_RATES_7_8  0x10 /* DCII 7/8 */
#define BAST_G3_CONFIG_DCII_SCAN_CODE_RATES_5_11 0x20 /* DCII 5/11 */
#define BAST_G3_CONFIG_DCII_SCAN_CODE_RATES_3_5  0x40 /* DCII 3/5 */
#define BAST_G3_CONFIG_DCII_SCAN_CODE_RATES_4_5  0x80 /* DCII 4/5 */
#define BAST_G3_CONFIG_DCII_SCAN_CODE_RATES_ALL  0xFF /* search all DCII modes */


/******************************************************************************
Summary:
   configuration parameter bit definitions for BAST_G3_CONFIG_TURBO_SCAN_MODES
Description:
   bit definition for BAST_G3_CONFIG_TURBO_SCAN_MODES
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_G3_CONFIG_TURBO_SCAN_MODES_QPSK_1_2  0x0001
#define BAST_G3_CONFIG_TURBO_SCAN_MODES_QPSK_2_3  0x0002
#define BAST_G3_CONFIG_TURBO_SCAN_MODES_QPSK_3_4  0x0004
#define BAST_G3_CONFIG_TURBO_SCAN_MODES_QPSK_5_6  0x0008
#define BAST_G3_CONFIG_TURBO_SCAN_MODES_QPSK_7_8  0x0010
#define BAST_G3_CONFIG_TURBO_SCAN_MODES_8PSK_2_3  0x0020
#define BAST_G3_CONFIG_TURBO_SCAN_MODES_8PSK_3_4  0x0040
#define BAST_G3_CONFIG_TURBO_SCAN_MODES_8PSK_4_5  0x0080
#define BAST_G3_CONFIG_TURBO_SCAN_MODES_8PSK_5_6  0x0100
#define BAST_G3_CONFIG_TURBO_SCAN_MODES_8PSK_8_9  0x0200
#define BAST_G3_CONFIG_TURBO_SCAN_MODES_MASK      0x03FF
#define BAST_G3_CONFIG_TURBO_SCAN_MODES_ALL       0x03FF


/******************************************************************************
Summary:
   configuration parameter bit definitions for BAST_G3_CONFIG_LDPC_SCAN_MODES
Description:
   bit definition for BAST_G3_CONFIG_LDPC_SCAN_MODES
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_G3_CONFIG_LDPC_SCAN_MODES_QPSK_1_2  0x0001
#define BAST_G3_CONFIG_LDPC_SCAN_MODES_QPSK_3_5  0x0002
#define BAST_G3_CONFIG_LDPC_SCAN_MODES_QPSK_2_3  0x0004
#define BAST_G3_CONFIG_LDPC_SCAN_MODES_QPSK_3_4  0x0008
#define BAST_G3_CONFIG_LDPC_SCAN_MODES_QPSK_4_5  0x0010
#define BAST_G3_CONFIG_LDPC_SCAN_MODES_QPSK_5_6  0x0020
#define BAST_G3_CONFIG_LDPC_SCAN_MODES_QPSK_8_9  0x0040
#define BAST_G3_CONFIG_LDPC_SCAN_MODES_QPSK_9_10 0x0080
#define BAST_G3_CONFIG_LDPC_SCAN_MODES_8PSK_3_5  0x0100
#define BAST_G3_CONFIG_LDPC_SCAN_MODES_8PSK_2_3  0x0200
#define BAST_G3_CONFIG_LDPC_SCAN_MODES_8PSK_3_4  0x0400
#define BAST_G3_CONFIG_LDPC_SCAN_MODES_8PSK_5_6  0x0800
#define BAST_G3_CONFIG_LDPC_SCAN_MODES_8PSK_8_9  0x1000
#define BAST_G3_CONFIG_LDPC_SCAN_MODES_8PSK_9_10 0x2000
#define BAST_G3_CONFIG_LDPC_SCAN_MODES_MASK      0x3FFF
#define BAST_G3_CONFIG_LDPC_SCAN_MODES_ALL       0x3FFF


/******************************************************************************
Summary:
   configuration parameter bit definitions for BAST_G3_CONFIG_STATUS_INDICATOR
Description:
   bit definition for BAST_G3_CONFIG_STATUS_INDICATOR
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_G3_CONFIG_STATUS_INDICATOR_RAIN_FADE  0x01
#define BAST_G3_CONFIG_STATUS_INDICATOR_FREQ_DRIFT 0x02
#define BAST_G3_CONFIG_STATUS_INDICATOR_THRESHOLD2 0x04
#define BAST_G3_CONFIG_STATUS_INDICATOR_THRESHOLD1 0x08


/******************************************************************************
Summary:
   configuration parameter bit definitions for BAST_G3_CONFIG_AGC_CTL
Description:
   bit definition for BAST_G3_CONFIG_AGC_CTL
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_G3_CONFIG_AGC_CTL_METER_MODE 0x01 /* enable meter mode (higher tracking AGC bandwidth) */


/******************************************************************************
Summary:
   configuration parameter bit definitions for BAST_G3_CONFIG_PLC_CTL
Description:
   bit definition for BAST_G3_CONFIG_PLC_CTL
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_G3_CONFIG_PLC_CTL_AWGN         0x02 /* 1=use PLC tracking bandwidth optimized for AWGN */
#define BAST_G3_CONFIG_PLC_CTL_OVERRIDE_ACQ 0x04 /* 1=acquisition PLC specified by PLC_ALT_ACQ_BW/PLC_ALT_ACQ_DAMP config parameters */
#define BAST_G3_CONFIG_PLC_CTL_OVERRIDE_TRK 0x08 /* 1=tracking PLC specified by PLC_ALT_TRK_BW/PLC_ALT_TRK_DAMP config parameters */


/******************************************************************************
Summary:
   configuration parameter bit definitions for BAST_G3_CONFIG_FREQ_ESTIMATE_STATUS
Description:
   bit definition for BAST_G3_CONFIG_FREQ_ESTIMATE_STATUS
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_G3_CONFIG_FREQ_ESTIMATE_STATUS_START  0x01 /* DFT freq estimate started */
#define BAST_G3_CONFIG_FREQ_ESTIMATE_STATUS_COARSE 0x02 /* DFT freq estimate coarse search started */
#define BAST_G3_CONFIG_FREQ_ESTIMATE_STATUS_FINE   0x04 /* DFT freq estimate fine search started */
#define BAST_G3_CONFIG_FREQ_ESTIMATE_STATUS_OQPSK  0x08 /* DFT freq estimate for OQPSK */
#define BAST_G3_CONFIG_FREQ_ESTIMATE_STATUS_DONE   0x80 /* DFT freq estimate finished */


/******************************************************************************
Summary:
   configuration parameter bit definitions for BAST_G3_CONFIG_REACQ_CTL
Description:
   bit definition for BAST_G3_CONFIG_REACQ_CTL
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_G3_CONFIG_REACQ_CTL_RETUNE       0x01 /* retune on reacquire */
#define BAST_G3_CONFIG_REACQ_CTL_FORCE        0x02 /* force reacquire now */
#define BAST_G3_CONFIG_REACQ_CTL_FORCE_RETUNE 0x04 /* force retune on next reacquire */
#define BAST_G3_CONFIG_REACQ_CTL_FREQ_DRIFT   0x08 /* dont keep lock if carrier freq drifts outside search_range */
#define BAST_G3_CONFIG_REACQ_CTL_PSOF         0x10 /* Preserve State On acquisition Failure when auto reacquisition is disabled */


/******************************************************************************
Summary:
   BAST_G3_CONFIG_BCM3445_CTL configuration parameter bit definitions
Description:
   BAST_G3_CONFIG_BCM3445_CTL configuration parameter bit definitions
See Also:
   BAST_ConfigBcm3445(), BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_G3_BCM3445_CTL_LOW_POWER_MODE   0x01
#define BAST_G3_BCM3445_CTL_AGC_TOP_LOW      0x00
#define BAST_G3_BCM3445_CTL_AGC_TOP_MID      0x02
#define BAST_G3_BCM3445_CTL_AGC_TOP_HI       0x04
#define BAST_G3_BCM3445_CTL_AGC_TOP          0x06


/***************************************************************************
Summary:
   This function returns the default settings for 73XX module.
Description:
   This function returns the default settings for 73XX module.
Returns:
   BERR_Code
See Also:
   BAST_Open()
****************************************************************************/
BERR_Code BAST_g3_GetDefaultSettings(
   BAST_Settings *pSettings   /* [out] Default settings */
);


/***************************************************************************
Summary:
   This function returns the default settings for 73XX channel device.
Description:
   This function returns the default settings for 73XX channel device.
Returns:
   BERR_Code
See Also:
   BAST_Open()
****************************************************************************/
BERR_Code BAST_g3_GetChannelDefaultSettings(
   BAST_Handle h,
   uint32_t chnNo,
   BAST_ChannelSettings *pChnDefSettings
);


/* channel interrupt enumeration */
typedef enum BAST_g3_IntID{
   BAST_g3_IntID_eLock = 0,
   BAST_g3_IntID_eNotLock,
   BAST_g3_IntID_eBaudTimer,
   BAST_g3_IntID_eBerTimer,
   BAST_g3_IntID_eGenTimer1,
   BAST_g3_IntID_eGenTimer2,
   BAST_g3_IntID_eGenTimer3,
   BAST_g3_IntID_eDiseqcTimer1,
   BAST_g3_IntID_eDiseqcTimer2,
   BAST_g3_IntID_eDiseqcVoltageGtHiThresh,
   BAST_g3_IntID_eDiseqcVoltageLtLoThresh,
   BAST_g3_IntID_eDiseqcDone,
   BAST_g3_IntID_eDiseqcTxFifoAlmostEmpty,
   BAST_g3_IntID_eHp,
   BAST_g3_IntID_eMi2c,
   BAST_g3_IntID_eTurboLock,
   BAST_g3_IntID_eTurboNotLock,
   BAST_g3_IntID_eTurboSync,
   BAST_g3_IntID_eAfecLock,
   BAST_g3_IntID_eAfecNotLock,
   BAST_g3_IntID_eHpFrameBoundary,
   BAST_g3_IntID_eDftDone,
   BAST_g3_MaxIntID
} BAST_g3_IntID;


/***************************************************************************
Summary:
   This function resets all interrupt counters.
Description:
   This function resets all interrupt counters.
Returns:
   BERR_Code
See Also:
   BAST_Open()
****************************************************************************/
void BAST_g3_ResetInterruptCounters(
   BAST_ChannelHandle h   /* [in] BAST channel handle */
);


/***************************************************************************
Summary:
   This function returns the current interrupt counter for the specified interrupt.
Description:
   This function returns the current interrupt counter for the specified interrupt.
Returns:
   BERR_Code
See Also:
   BAST_Open()
****************************************************************************/
BERR_Code BAST_g3_GetInterruptCount(
   BAST_ChannelHandle h,   /* [in] BAST channel handle */
   BAST_g3_IntID      idx, /* [in] specifies which interrupt count to return */
   uint32_t           *pCount
);


/***************************************************************************
Summary:
   This function returns the trace buffer used for debugging.
Description:
   This function returns the trace buffer used for debugging.
Returns:
   BERR_Code
See Also:
   BAST_Open()
****************************************************************************/
BERR_Code BAST_g3_GetTraceBuffer(
   BAST_ChannelHandle h,        /* [in] BAST channel handle */
   uint32_t           *pBuffer  /* [out] pointer to the trace buffer */
);


/***************************************************************************
Summary:
   This function listens for an fsk packet of specified length.
Description:
   This function listens for an fsk packet of specified length.
Returns:
   BERR_Code
See Also:
   BAST_Open()
****************************************************************************/
BERR_Code BAST_g3_ListenFsk(
   BAST_Handle h,  /* [in] BAST handle */
   uint8_t n       /* [in] length of data expected */
);


typedef enum BAST_TraceEvent
{
   BAST_TraceEvent_eTuneMixPllLock = 0,
   BAST_TraceEvent_eDcoConverge,
   BAST_TraceEvent_eTunerLpfCalDone,
   BAST_TraceEvent_eTuneDone,
   BAST_TraceEvent_eFreqEstDone,
   BAST_TraceEvent_eRetuneDone,
   BAST_TraceEvent_eStartHp,
   BAST_TraceEvent_eRcvrLocked,
   BAST_TraceEvent_eStartTracking,
   BAST_TraceEvent_eInitialLock,
   BAST_TraceEvent_eStableLock,
   BAST_TraceEvent_eReacquire,
   BAST_TraceEvent_eMax
} BAST_TraceEvent;

#define BAST_TraceEvent_eStartViterbi BAST_TraceEvent_eStartHp
#define BAST_TraceEvent_eViterbiLocked BAST_TraceEvent_eRcvrLocked

#ifdef BAST_EXCLUDE_API_TABLE
#define BAST_Open BAST_g3_P_Open
#define BAST_Close BAST_g3_P_Close
#define BAST_GetTotalChannels BAST_g3_P_GetTotalChannels
#define BAST_GetChannelDefaultSettings BAST_g3_GetChannelDefaultSettings
#define BAST_OpenChannel BAST_g3_P_OpenChannel
#define BAST_CloseChannel BAST_g3_P_CloseChannel
#define BAST_GetDevice BAST_g3_P_GetDevice
#define BAST_InitAp BAST_g3_P_InitAp
#define BAST_SoftReset BAST_g3_P_SoftReset
#define BAST_TuneAcquire BAST_g3_P_TuneAcquire
#define BAST_GetChannelStatus BAST_g3_P_GetChannelStatus
#define BAST_GetLockStatus BAST_g3_P_GetLockStatus
#define BAST_ResetStatus BAST_g3_P_ResetStatus
#define BAST_SetDiseqcTone BAST_g3_P_SetDiseqcTone
#define BAST_GetDiseqcTone BAST_g3_P_GetDiseqcTone
#define BAST_SetDiseqcVoltage BAST_g3_P_SetDiseqcVoltage
#define BAST_SendDiseqcCommand BAST_g3_P_SendDiseqcCommand
#define BAST_GetDiseqcStatus BAST_g3_P_GetDiseqcStatus
#define BAST_ResetDiseqc BAST_g3_P_ResetDiseqc
#define BAST_ResetFtm BAST_g3_P_ResetFtm
#define BAST_ReadFtm BAST_g3_P_ReadFtm
#define BAST_WriteFtm BAST_g3_P_WriteFtm
#define BAST_PowerDownFtm BAST_g3_P_PowerDownFtm
#define BAST_PowerUpFtm BAST_g3_P_PowerUpFtm
#define BAST_WriteMi2c BAST_g3_P_WriteMi2c
#define BAST_ReadMi2c BAST_g3_P_ReadMi2c
#define BAST_GetSoftDecisionBuf BAST_g3_P_GetSoftDecisionBuf
#define BAST_FreezeEq BAST_g3_P_FreezeEq
#define BAST_PowerDown BAST_g3_P_PowerDown
#define BAST_PowerUp BAST_g3_P_PowerUp
#define BAST_ReadRegister BAST_g3_P_ReadRegister
#define BAST_WriteRegister BAST_g3_P_WriteRegister
#define BAST_ReadConfig BAST_g3_P_ReadConfig
#define BAST_WriteConfig BAST_g3_P_WriteConfig
#define BAST_GetLockStateChangeEventHandle BAST_g3_P_GetLockChangeEventHandle
#define BAST_GetFtmEventHandle BAST_g3_P_GetFtmEventHandle
#define BAST_GetDiseqcEventHandle BAST_g3_P_GetDiseqcEventHandle
#define BAST_AbortAcq BAST_g3_P_AbortAcq
#define BAST_SendACW BAST_g3_P_SendACW
#define BAST_GetDiseqcVoltage BAST_g3_P_GetDiseqcVoltage
#define BAST_GetDiseqcVsenseEventHandles BAST_g3_P_GetDiseqcVsenseEventHandles
#define BAST_EnableVsenseInterrupts BAST_g3_P_EnableVsenseInterrupts
#define BAST_PeakScan BAST_g3_P_PeakScan
#define BAST_GetPeakScanStatus BAST_g3_P_GetPeakScanStatus
#define BAST_GetPeakScanEventHandle BAST_g3_P_GetPeakScanEventHandle
#define BAST_EnableStatusInterrupts BAST_g3_P_EnableStatusInterrupts
#define BAST_GetStatusEventHandle BAST_g3_P_GetStatusEventHandle
#define BAST_EnableSpurCanceller BAST_g3_P_EnableSpurCanceller
#define BAST_ResetChannel BAST_g3_P_ResetChannel
#define BAST_EnableDiseqcLnb BAST_g3_P_EnableDiseqcLnb
#define BAST_SetSearchRange BAST_g3_P_SetSearchRange
#define BAST_GetSearchRange BAST_g3_P_GetSearchRange
#define BAST_SetAmcScramblingSeq BAST_g3_P_SetAmcScramblingSeq
#define BAST_SetTunerFilter BAST_g3_P_SetTunerFilter
#define BAST_GetSignalDetectStatus BAST_g3_P_GetSignalDetectStatus
#define BAST_SetOutputTransportSettings BAST_g3_P_SetOutputTransportSettings
#define BAST_GetOutputTransportSettings BAST_g3_P_GetOutputTransportSettings
#define BAST_SetDiseqcSettings BAST_g3_P_SetDiseqcSettings
#define BAST_GetDiseqcSettings BAST_g3_P_GetDiseqcSettings
#define BAST_SetNetworkSpec BAST_g3_P_SetNetworkSpec
#define BAST_GetNetworkSpec BAST_g3_P_GetNetworkSpec
#define BAST_SetFskChannel BAST_g3_P_SetFskChannel
#define BAST_GetFskChannel BAST_g3_P_GetFskChannel
#define BAST_SetPeakScanSymbolRateRange BAST_g3_P_SetPeakScanSymbolRateRange
#define BAST_GetPeakScanSymbolRateRange BAST_g3_P_GetPeakScanSymbolRateRange
#define BAST_SetAdcSelect BAST_g3_P_SetAdcSelect
#define BAST_GetAdcSelect BAST_g3_P_GetAdcSelect
#define BAST_GetVersionInfo BAST_g3_P_GetVersionInfo
#endif

#ifdef __cplusplus
}
#endif

#endif /* BAST_73XX_H__ */


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
#ifndef _BAST_4538_H__
#define _BAST_4538_H__               

#ifdef __cplusplus
extern "C" {
#endif


#include "bast.h"
#include "breg_i2c.h"


/******************************************************************************
Summary:
   configuration parameter addresses
Description:
   These are the configuration parameters that can be accessed via 
   BAST_ReadConfig() and BAST_WriteConfig().
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_4538_CONFIG_NETWORK_SPEC              0x8000
#define BAST_4538_CONFIG_LEN_NETWORK_SPEC          1
#define BAST_4538_CONFIG_SEARCH_RANGE              0x8001
#define BAST_4538_CONFIG_LEN_SEARCH_RANGE          4
#define BAST_4538_CONFIG_XPORT_CTL                 0x8002
#define BAST_4538_CONFIG_LEN_XPORT_CTL             4
#define BAST_4538_CONFIG_MTSIF_CTL                 0x8003 /* MTSIF control */
#define BAST_4538_CONFIG_LEN_MTSIF_CTL             4  
#define BAST_4538_CONFIG_DEBUG_LEVEL               0x8004 /* firmware debug output control */   
#define BAST_4538_CONFIG_LEN_DEBUG_LEVEL           1
#define BAST_4538_CONFIG_DEBUG_MODULE              0x8005
#define BAST_4538_CONFIG_LEN_DEBUG_MODULE          1
#define BAST_4538_CONFIG_HAB_MAX_TIME              0x8006
#define BAST_4538_CONFIG_LEN_HAB_MAX_TIME          4
#define BAST_4538_CONFIG_HAB_MAX_TIME_CMD          0x8007
#define BAST_4538_CONFIG_LEN_HAB_MAX_TIME_CMD      1
#define BAST_4538_CONFIG_BCM3447_CTL               0x8008
#define BAST_4538_CONFIG_LEN_BCM3447_CTL           4
#define BAST_4538_CONFIG_OTP_DISABLE_INPUT         0x8009 /* READ ONLY */
#define BAST_4538_CONFIG_LEN_OTP_DISABLE_INPUT     1
#define BAST_4538_CONFIG_OTP_DISABLE_CHAN          0x800A /* READ ONLY */
#define BAST_4538_CONFIG_LEN_OTP_DISABLE_CHAN      1
#define BAST_4538_CONFIG_OTP_DISABLE_FEATURE       0x800B /* READ ONLY */
#define BAST_4538_CONFIG_LEN_OTP_DISABLE_FEATURE   1
#define BAST_4538_CONFIG_FLASH_SECTOR_BUF_ADDR     0x800C /* READ ONLY: flash sector buffer address in memory */
#define BAST_4538_CONFIG_LEN_FLASH_SECTOR_BUF_ADDR 4
#define BAST_4538_CONFIG_DSEC_PIN_MUX              0x800D /* diseqc pin mux configuration, see BAST_4538_DSEC_PIN_MUX_* macros */
#define BAST_4538_CONFIG_LEN_DSEC_PIN_MUX          2
#define BAST_4538_CONFIG_XTAL_FREQ                 0x0000 /* READ ONLY: crystal freq in Hz */
#define BAST_4538_CONFIG_LEN_XTAL_FREQ             4        
#define BAST_4538_CONFIG_MISC_CTL                  0x0001 /* miscellaneous acquisition settings, see BAST_4538_MISC_CTL_* macros */
#define BAST_4538_CONFIG_LEN_MISC_CTL              1
#define BAST_4538_CONFIG_RAIN_FADE_THRESHOLD       0x0002 /* SNR drop threshold for the rain fade indicator status in units of 1/8 dB SNR */
#define BAST_4538_CONFIG_LEN_RAIN_FADE_THRESHOLD   1
#define BAST_4538_CONFIG_RAIN_FADE_WINDOW          0x0003 /* time window for rain fade indicator status in units of 100 msecs */
#define BAST_4538_CONFIG_LEN_RAIN_FADE_WINDOW      4
#define BAST_4538_CONFIG_FREQ_DRIFT_THRESHOLD      0x0004 /* carrier offset threshold in Hz for frequency drift indicator status */
#define BAST_4538_CONFIG_LEN_FREQ_DRIFT_THRESHOLD  4
#define BAST_4538_CONFIG_STUFF_BYTES               0x0005 /* Number of null bytes to insert in each frame */
#define BAST_4538_CONFIG_LEN_STUFF_BYTES           1
#define BAST_4538_CONFIG_ACQ_TIME                  0x0006 /* acquisition time in usecs */
#define BAST_4538_CONFIG_LEN_ACQ_TIME              4
#define BAST_4538_CONFIG_TUNER_CTL                 0x0008 /* tuner control settings, see BAST_4538_CONFIG_TUNER_CTL_* macros */
#define BAST_4538_CONFIG_LEN_TUNER_CTL             1
#define BAST_4538_CONFIG_REACQ_CTL                 0x0009 /* reacquisition options, see BAST_4538_CONFIG_REACQ_CTL_* macros */
#define BAST_4538_CONFIG_LEN_REACQ_CTL             1
#define BAST_4538_CONFIG_PLC_CTL                   0x000A /* PLC settings */
#define BAST_4538_CONFIG_LEN_PLC_CTL               1
#define BAST_4538_CONFIG_LDPC_CTL                  0x000B /* AFEC settings, see BAST_4538_CONFIG_LDPC_CTL_* macros */
#define BAST_4538_CONFIG_LEN_LDPC_CTL              1
#define BAST_4538_CONFIG_TURBO_CTL                 0x000C /* TFEC settings */
#define BAST_4538_CONFIG_LEN_TURBO_CTL             1
#define BAST_4538_CONFIG_PLC_ALT_ACQ_BW            0x000D /* Alternate acquisition PLC bandwidth in units of Hz.  This bandwidth applies when PLC_CTL[2]=1. */
#define BAST_4538_CONFIG_LEN_PLC_ALT_ACQ_BW        4
#define BAST_4538_CONFIG_PLC_ALT_ACQ_DAMP          0x000E /* Alternate acquisition PLC damping factor scaled 2^3 (e.g. damping factor of 1.0 is 0x08).  This damping factor only applies when PLC_CTL[2]=1. */
#define BAST_4538_CONFIG_LEN_PLC_ALT_ACQ_DAMP      1
#define BAST_4538_CONFIG_PLC_ALT_TRK_BW            0x000F /* Alternate tracking PLC bandwidth in units of Hz.  This bandwidth applies when PLC_CTL[3]=1. */
#define BAST_4538_CONFIG_LEN_PLC_ALT_TRK_BW        4
#define BAST_4538_CONFIG_PLC_ALT_TRK_DAMP          0x0010 /* Alternate tracking PLC damping factor scaled 2^3 (e.g. damping factor of 1.0 is 0x08).  This damping factor only applies when PLC_CTL[3]=1. */
#define BAST_4538_CONFIG_LEN_PLC_ALT_TRK_DAMP      1
#define BAST_4538_CONFIG_BLIND_SCAN_MODES          0x0011 /* Indicates which modes will be considered in the blind scan, see BAST_4538_CONFIG_BLIND_SCAN_MODES_* macros */
#define BAST_4538_CONFIG_LEN_BLIND_SCAN_MODES      1
#define BAST_4538_CONFIG_DTV_SCAN_CODE_RATES       0x0012 /* Selects the DTV code rates that are considered in the scan, see BAST_4538_CONFIG_DTV_SCAN_CODE_RATES_* macros */
#define BAST_4538_CONFIG_LEN_DTV_SCAN_CODE_RATES   1
#define BAST_4538_CONFIG_DVB_SCAN_CODE_RATES       0x0013 /* Selects the DVB-S code rates that are considered in the scan, see BAST_4538_CONFIG_DVB_SCAN_CODE_RATES_* macros */
#define BAST_4538_CONFIG_LEN_DVB_SCAN_CODE_RATES   1
#define BAST_4538_CONFIG_DCII_SCAN_CODE_RATES      0x0014 /* Selects the DCII code rates that are considered in the scan, see BAST_4538_CONFIG_DCII_SCAN_CODE_RATES_* macros */
#define BAST_4538_CONFIG_LEN_DCII_SCAN_CODE_RATES  1
#define BAST_4538_CONFIG_TURBO_SCAN_MODES          0x0015 /* Selects the Turbo modes that are considered in the scan, see BAST_4538_CONFIG_TURBO_SCAN_MODES_* macros */
#define BAST_4538_CONFIG_LEN_TURBO_SCAN_MODES      2
#define BAST_4538_CONFIG_LDPC_SCAN_MODES           0x0016 /* Selects the LDPC modes that are considered in the scan, see BAST_4538_CONFIG_LDPC_SCAN_MODES_* macros */
#define BAST_4538_CONFIG_LEN_LDPC_SCAN_MODES       2
#define BAST_4538_CONFIG_TUNER_FILTER_OVERRIDE     0x0017 /* channel bandwidth filter */
#define BAST_4538_CONFIG_LEN_TUNER_FILTER_OVERRIDE 1
#define BAST_4538_CONFIG_FTM_TX_POWER              0x0018
#define BAST_4538_CONFIG_LEN_FTM_TX_POWER          1
#define BAST_4538_CONFIG_FTM_CH_SELECT             0x0019
#define BAST_4538_CONFIG_LEN_FTM_CH_SELECT         1
#define BAST_4538_CONFIG_FREQ_ESTIMATE_STATUS      0x001A /* READ ONLY: status of DFT freq estimate */
#define BAST_4538_CONFIG_LEN_FREQ_ESTIMATE_STATUS  1
#define BAST_4538_CONFIG_IF_STEP_SAVE              0x001B /* coarse IF step determined by DFT freq estimate */
#define BAST_4538_CONFIG_LEN_IF_STEP_SAVE          4
#define BAST_4538_CONFIG_STATUS_INDICATOR          0x001C /* real time status indicators, see BAST_4538_CONFIG_STATUS_INDICATOR_* macros */
#define BAST_4538_CONFIG_LEN_STATUS_INDICATOR      1
#define BAST_4538_CONFIG_DFT_RANGE_START           0x001D /* Starting bin for DFT engine. */
#define BAST_4538_CONFIG_LEN_DFT_RANGE_START       2
#define BAST_4538_CONFIG_DFT_RANGE_END             0x001E /* Ending bin for DFT engine. */
#define BAST_4538_CONFIG_LEN_DFT_RANGE_END         2
#define BAST_4538_CONFIG_DEBUG1                    0x0023 /* debug 1 */
#define BAST_4538_CONFIG_LEN_DEBUG1                4
#define BAST_4538_CONFIG_DEBUG2                    0x0024 /* debug 2 */
#define BAST_4538_CONFIG_LEN_DEBUG2                4
#define BAST_4538_CONFIG_ACM_MAX_MODE              0x0025 /* highest BAST_Mode in ACM transmission */
#define BAST_4538_CONFIG_LEN_ACM_MAX_MODE          1
#define BAST_4538_CONFIG_REACQ_CAUSE               0x0026 /* cause of reacquisition */
#define BAST_4538_CONFIG_LEN_REACQ_CAUSE           4
#define BAST_4538_CONFIG_DEBUG3                    0x002C /* debug 3 */
#define BAST_4538_CONFIG_LEN_DEBUG3                4
#define BAST_4538_CONFIG_FSK_TX_FREQ_HZ            0x002D /* FSK transmit frequency in Hz */
#define BAST_4538_CONFIG_LEN_FSK_TX_FREQ_HZ        4
#define BAST_4538_CONFIG_FSK_RX_FREQ_HZ            0x002E /* FSK receive frequency in Hz */
#define BAST_4538_CONFIG_LEN_FSK_RX_FREQ_HZ        4
#define BAST_4538_CONFIG_FSK_TX_DEV_HZ             0x002F /* FSK frequency deviation in Hz */
#define BAST_4538_CONFIG_LEN_FSK_TX_DEV_HZ         4

#define BAST_4538_CONFIG_FSK_TX_POWER        BAST_4538_CONFIG_FTM_TX_POWER
#define BAST_4538_CONFIG_LEN_FSK_TX_POWER    BAST_4538_CONFIG_LEN_FTM_TX_POWER
#define BAST_4538_CONFIG_FSK_CH_SELECT       BAST_4538_CONFIG_FTM_CH_SELECT
#define BAST_4538_CONFIG_LEN_FSK_CH_SELECT   BAST_4538_CONFIG_LEN_FTM_CH_SELECT


/******************************************************************************
Summary:
   BAST_4538_CONFIG_MISC_CTL configuration parameter bit definitions
Description:
   BAST_4538_CONFIG_MISC_CTL configuration parameter bit definitions
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_4538_CONFIG_MISC_CTL_DISABLE_SMART_TUNE 0x01 /* 1=disable Fs smart tuning */
#define BAST_4538_CONFIG_MISC_CTL_CHECK_TIMING_LOOP  0x02 /* 1=verify timing loop is locked */
#define BAST_4538_CONFIG_MISC_CTL_DISABLE_NOTCH      0x04 /* wideband tuners only: 1=disable dco notch filters */


/******************************************************************************
Summary:
   BAST_4538_CONFIG_TUNER_CTL configuration parameter bit definitions
Description:
   BAST_4538_CONFIG_TUNER_CTL configuration parameter bit definitions
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_4538_CONFIG_TUNER_CTL_SET_FILTER_MANUAL        0x08 /* 1=tuner LPF will be set to the value specified by TUNER_FILTER_OVERRIDE config parameter */
#define BAST_4538_CONFIG_TUNER_CTL_BYPASS_DFT_FREQ_EST      0x10 /* 1=bypass dft freq estimate */
#define BAST_4538_CONFIG_TUNER_CTL_PRESERVE_COMMANDED_FREQ  0x20 /* 1=tuner LO will be set to the commanded tuner freq */


/******************************************************************************
Summary:
   BAST_4538_CONFIG_BLIND_SCAN_MODES configuration parameter bit definitions
Description:
   BAST_4538_CONFIG_BLIND_SCAN_MODES configuration parameter bit definitions
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_4538_CONFIG_BLIND_SCAN_MODES_DVB    0x01
#define BAST_4538_CONFIG_BLIND_SCAN_MODES_TURBO  0x02
#define BAST_4538_CONFIG_BLIND_SCAN_MODES_LDPC   0x04
#define BAST_4538_CONFIG_BLIND_SCAN_MODES_DTV    0x08
#define BAST_4538_CONFIG_BLIND_SCAN_MODES_DCII   0x10
#define BAST_4538_CONFIG_BLIND_SCAN_MODES_MASK   0x1F


/******************************************************************************
Summary:
   configuration parameter bit definitions for BAST_4538_CONFIG_DTV_SCAN_CODE_RATES
Description:
   bit definition for BAST_4538_CONFIG_DTV_SCAN_CODE_RATES
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_4538_CONFIG_DTV_SCAN_CODE_RATES_1_2  0x01 /* DTV 1/2 */
#define BAST_4538_CONFIG_DTV_SCAN_CODE_RATES_2_3  0x02 /* DTV 2/3 */
#define BAST_4538_CONFIG_DTV_SCAN_CODE_RATES_6_7  0x04 /* DTV 6/7 */
#define BAST_4538_CONFIG_DTV_SCAN_CODE_RATES_ALL  0x07 /* search all DTV modes */


/******************************************************************************
Summary:
   configuration parameter bit definitions for BAST_4538_CONFIG_DVB_SCAN_CODE_RATES
Description:
   bit definition for BAST_4538_CONFIG_DVB_SCAN_CODE_RATES
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_4538_CONFIG_DVB_SCAN_CODE_RATES_1_2  0x01 /* DVB 1/2 */
#define BAST_4538_CONFIG_DVB_SCAN_CODE_RATES_2_3  0x02 /* DVB 2/3 */
#define BAST_4538_CONFIG_DVB_SCAN_CODE_RATES_3_4  0x04 /* DVB 3/4 */
#define BAST_4538_CONFIG_DVB_SCAN_CODE_RATES_5_6  0x08 /* DVB 5/6 */
#define BAST_4538_CONFIG_DVB_SCAN_CODE_RATES_7_8  0x10 /* DVB 7/8 */
#define BAST_4538_CONFIG_DVB_SCAN_CODE_RATES_ALL  0x1F /* search all DVB-S modes */


/******************************************************************************
Summary:
   configuration parameter bit definitions for BAST_4538_CONFIG_DCII_SCAN_CODE_RATES
Description:
   bit definition for BAST_4538_CONFIG_DCII_SCAN_CODE_RATES
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_4538_CONFIG_DCII_SCAN_CODE_RATES_1_2  0x01 /* DCII 1/2 */
#define BAST_4538_CONFIG_DCII_SCAN_CODE_RATES_2_3  0x02 /* DCII 2/3 */
#define BAST_4538_CONFIG_DCII_SCAN_CODE_RATES_3_4  0x04 /* DCII 3/4 */
#define BAST_4538_CONFIG_DCII_SCAN_CODE_RATES_5_6  0x08 /* DCII 5/6 */
#define BAST_4538_CONFIG_DCII_SCAN_CODE_RATES_7_8  0x10 /* DCII 7/8 */
#define BAST_4538_CONFIG_DCII_SCAN_CODE_RATES_5_11 0x20 /* DCII 5/11 */
#define BAST_4538_CONFIG_DCII_SCAN_CODE_RATES_3_5  0x40 /* DCII 3/5 */
#define BAST_4538_CONFIG_DCII_SCAN_CODE_RATES_4_5  0x80 /* DCII 4/5 */
#define BAST_4538_CONFIG_DCII_SCAN_CODE_RATES_ALL  0xFF /* search all DCII modes */


/******************************************************************************
Summary:
   configuration parameter bit definitions for BAST_4538_CONFIG_TURBO_SCAN_MODES
Description:
   bit definition for BAST_4538_CONFIG_TURBO_SCAN_MODES
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_4538_CONFIG_TURBO_SCAN_MODES_QPSK_1_2  0x0001
#define BAST_4538_CONFIG_TURBO_SCAN_MODES_QPSK_2_3  0x0002
#define BAST_4538_CONFIG_TURBO_SCAN_MODES_QPSK_3_4  0x0004
#define BAST_4538_CONFIG_TURBO_SCAN_MODES_QPSK_5_6  0x0008
#define BAST_4538_CONFIG_TURBO_SCAN_MODES_QPSK_7_8  0x0010
#define BAST_4538_CONFIG_TURBO_SCAN_MODES_8PSK_2_3  0x0020
#define BAST_4538_CONFIG_TURBO_SCAN_MODES_8PSK_3_4  0x0040
#define BAST_4538_CONFIG_TURBO_SCAN_MODES_8PSK_4_5  0x0080
#define BAST_4538_CONFIG_TURBO_SCAN_MODES_8PSK_5_6  0x0100
#define BAST_4538_CONFIG_TURBO_SCAN_MODES_8PSK_8_9  0x0200
#define BAST_4538_CONFIG_TURBO_SCAN_MODES_MASK      0x03FF
#define BAST_4538_CONFIG_TURBO_SCAN_MODES_ALL       0x03FF


/******************************************************************************
Summary:
   configuration parameter bit definitions for BAST_4538_CONFIG_LDPC_SCAN_MODES
Description:
   bit definition for BAST_4538_CONFIG_LDPC_SCAN_MODES
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_4538_CONFIG_LDPC_SCAN_MODES_QPSK_1_2  0x0001
#define BAST_4538_CONFIG_LDPC_SCAN_MODES_QPSK_3_5  0x0002
#define BAST_4538_CONFIG_LDPC_SCAN_MODES_QPSK_2_3  0x0004
#define BAST_4538_CONFIG_LDPC_SCAN_MODES_QPSK_3_4  0x0008
#define BAST_4538_CONFIG_LDPC_SCAN_MODES_QPSK_4_5  0x0010
#define BAST_4538_CONFIG_LDPC_SCAN_MODES_QPSK_5_6  0x0020
#define BAST_4538_CONFIG_LDPC_SCAN_MODES_QPSK_8_9  0x0040
#define BAST_4538_CONFIG_LDPC_SCAN_MODES_QPSK_9_10 0x0080
#define BAST_4538_CONFIG_LDPC_SCAN_MODES_8PSK_3_5  0x0100
#define BAST_4538_CONFIG_LDPC_SCAN_MODES_8PSK_2_3  0x0200
#define BAST_4538_CONFIG_LDPC_SCAN_MODES_8PSK_3_4  0x0400
#define BAST_4538_CONFIG_LDPC_SCAN_MODES_8PSK_5_6  0x0800
#define BAST_4538_CONFIG_LDPC_SCAN_MODES_8PSK_8_9  0x1000
#define BAST_4538_CONFIG_LDPC_SCAN_MODES_8PSK_9_10 0x2000
#define BAST_4538_CONFIG_LDPC_SCAN_MODES_MASK      0x3FFF
#define BAST_4538_CONFIG_LDPC_SCAN_MODES_ALL       0x3FFF


/******************************************************************************
Summary:
   configuration parameter bit definitions for BAST_4538_CONFIG_STATUS_INDICATOR
Description:
   bit definition for BAST_4538_CONFIG_STATUS_INDICATOR
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_4538_CONFIG_STATUS_INDICATOR_RAIN_FADE  0x01
#define BAST_4538_CONFIG_STATUS_INDICATOR_FREQ_DRIFT 0x02
#define BAST_4538_CONFIG_STATUS_INDICATOR_THRESHOLD2 0x04
#define BAST_4538_CONFIG_STATUS_INDICATOR_THRESHOLD1 0x08


/******************************************************************************
Summary:
   configuration parameter bit definitions for BAST_4538_CONFIG_PLC_CTL
Description:
   bit definition for BAST_4538_CONFIG_PLC_CTL
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_4538_CONFIG_PLC_CTL_AWGN         0x02 /* 1=use PLC tracking bandwidth optimized for AWGN */
#define BAST_4538_CONFIG_PLC_CTL_OVERRIDE_ACQ 0x04 /* 1=acquisition PLC specified by PLC_ALT_ACQ_BW/PLC_ALT_ACQ_DAMP config parameters */
#define BAST_4538_CONFIG_PLC_CTL_OVERRIDE_TRK 0x08 /* 1=tracking PLC specified by PLC_ALT_TRK_BW/PLC_ALT_TRK_DAMP config parameters */


/******************************************************************************
Summary:
   configuration parameter bit definitions for BAST_4538_CONFIG_FREQ_ESTIMATE_STATUS
Description:
   bit definition for BAST_4538_CONFIG_FREQ_ESTIMATE_STATUS
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_4538_CONFIG_FREQ_ESTIMATE_STATUS_START  0x01 /* DFT freq estimate started */
#define BAST_4538_CONFIG_FREQ_ESTIMATE_STATUS_COARSE 0x02 /* DFT freq estimate coarse search started */
#define BAST_4538_CONFIG_FREQ_ESTIMATE_STATUS_FINE   0x04 /* DFT freq estimate fine search started */
#define BAST_4538_CONFIG_FREQ_ESTIMATE_STATUS_OQPSK  0x08 /* DFT freq estimate for OQPSK */
#define BAST_4538_CONFIG_FREQ_ESTIMATE_STATUS_DONE   0x80 /* DFT freq estimate finished */


/******************************************************************************
Summary:
   configuration parameter bit definitions for BAST_4538_CONFIG_REACQ_CTL
Description:
   bit definition for BAST_4538_CONFIG_REACQ_CTL
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_4538_CONFIG_REACQ_CTL_RETUNE       0x01 /* retune on reacquire */
#define BAST_4538_CONFIG_REACQ_CTL_FORCE        0x02 /* force reacquire now */
#define BAST_4538_CONFIG_REACQ_CTL_FORCE_RETUNE 0x04 /* force retune on next reacquire */
#define BAST_4538_CONFIG_REACQ_CTL_FREQ_DRIFT   0x08 /* dont keep lock if carrier freq drifts outside search_range */


/******************************************************************************
Summary:
   configuration parameter bit definitions for BAST_4538_CONFIG_DSEC_PIN_MUX
Description:
   bit definition for BAST_4538_CONFIG_DSEC_PIN_MUX
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_4538_DSEC_PIN_MUX_CHAN0_MASK   0x0003
#define BAST_4538_DSEC_PIN_MUX_CHAN0_SHIFT  0
#define BAST_4538_DSEC_PIN_MUX_CHAN1_MASK   0x000C
#define BAST_4538_DSEC_PIN_MUX_CHAN1_SHIFT  2
#define BAST_4538_DSEC_PIN_MUX_CHAN2_MASK   0x0030
#define BAST_4538_DSEC_PIN_MUX_CHAN2_SHIFT  4
#define BAST_4538_DSEC_PIN_MUX_CHAN3_MASK   0x00C0
#define BAST_4538_DSEC_PIN_MUX_CHAN3_SHIFT  6
#define BAST_4538_DSEC_PIN_MUX_TXOUT_GPIO18 0x0100
#define BAST_4538_DSEC_PIN_MUX_TXEN_GPIO19  0x0200
#define BAST_4538_DSEC_PIN_MUX_TXOUT0_TXEN0        0
#define BAST_4538_DSEC_PIN_MUX_GPO8_GPO7           1
#define BAST_4538_DSEC_PIN_MUX_GPIO18_GPIO19       1
#define BAST_4538_DSEC_PIN_MUX_GPO6_GPO5           2
#define BAST_4538_DSEC_PIN_MUX_GPO4_GPO3           3


/******************************************************************************
Summary:
   Structure containing AGC status
Description:
   This structure contains all the data to compute input power estimate.
See Also:
   BAST_GetAgcStatus()
******************************************************************************/
typedef struct BAST_4538_AgcStatus
{
   bool     bLnaGainValid;
   bool     bChanAgcValid;
   uint32_t lnaGain;
   uint32_t chanAgc;
   uint32_t tunerFreq;
   uint8_t  adcSelect;
} BAST_4538_AgcStatus;


/***************************************************************************
Summary:
   This function returns the default settings for 4538 module.
Description:
   This function returns the default settings for 4538 module.
Returns:
   BERR_Code
See Also:
   BAST_Open()
****************************************************************************/
BERR_Code BAST_4538_GetDefaultSettings(
   BAST_Settings *pSettings   /* [out] Default settings */
);


/***************************************************************************
Summary:
   This function returns the default settings for 4538 channel device.
Description:
   This function returns the default settings for 4538 channel device.
Returns:
   BERR_Code
See Also:
   BAST_Open()
****************************************************************************/
BERR_Code BAST_4538_GetChannelDefaultSettings(
   BAST_Handle h, 
   uint32_t chnNo, 
   BAST_ChannelSettings *pChnDefSettings
);


/***************************************************************************
Summary:
   This function prints a message from the UART. 
Description:
   This function transmits the given null-terminated string to the UART at
   115200 8-N-1.
Returns:
   BERR_Code
See Also:
   none
****************************************************************************/
BERR_Code BAST_4538_PrintUart(BAST_Handle h, char *pBuf);


/***************************************************************************
Summary:
   This function enables/disables AVS.
Description:
   This function enables/disables AVS.  AVS is enabled by default.
Returns:
   BERR_Code
See Also:
   none
****************************************************************************/
BERR_Code BAST_4538_EnableAvs(BAST_Handle h, bool bEnable);


/***************************************************************************
Summary:
   This function performs a MI2C write transaction using the BSC controller.
Description:
   This function performs a MI2C write transaction using the BSC controller.
Returns:
   BERR_Code
See Also:
   none
****************************************************************************/
BERR_Code BAST_4538_WriteBsc(
   BAST_Handle h,        /* [in] BAST device handle */
   uint8_t channel,      /* [in] bsc channel select */
   uint8_t slave_addr,   /* [in] address of the i2c slave device */
   uint8_t *i2c_buf,     /* [in] specifies the data to transmit */
   uint8_t n             /* [in] number of bytes to transmit after the i2c slave address */
);


/***************************************************************************
Summary:
   This function performs a MI2C read transaction using the BSC controller.
Description:
   This function performs a MI2C read transaction using the BSC controller.
Returns:
   BERR_Code
See Also:
   none
****************************************************************************/
BERR_Code BAST_4538_ReadBsc(
   BAST_Handle h,        /* [in] BAST device handle */
   uint8_t channel,      /* [in] bsc channel select */
   uint8_t slave_addr,   /* [in] address of the i2c slave device */
   uint8_t *out_buf,     /* [in] specifies the data to transmit before the i2c restart condition */
   uint8_t out_n,        /* [in] number of bytes to transmit before the i2c restart condition not including the i2c slave address */
   uint8_t *in_buf,      /* [out] holds the data read */
   uint8_t in_n          /* [in] number of bytes to read after the i2c restart condition not including the i2c slave address */
);


/***************************************************************************
Summary:
   This function sets the state of 1 or more GPO pins.
Description:
   This function sets the state of 1 or more GPO pins.
Returns:
   BERR_Code
See Also:
   none
****************************************************************************/
BERR_Code BAST_4538_WriteGpo(
   BAST_Handle h,        /* [in] BAST device handle */
   uint32_t mask,        /* [in] specifies which GPO pins to write */
   uint32_t data         /* [in] specifies the state of the GPO pins to be written */
);


/***************************************************************************
Summary:
   This function initiates flash sector programming at the given sector address.
Description:
   This function initiates flash sector programming at the given sector address.
Returns:
   BERR_Code
See Also:
   none
****************************************************************************/
BERR_Code BAST_4538_WriteFlashSector(
   BAST_Handle h, /* [in] BAST device handle */
   uint32_t addr  /* [in] flash sector address */
);


/***************************************************************************
Summary:
   This function enables/disables and configures BERT output to GPO pins.  
Description:
   When BERT output is enabled, BERT_CLK will come out to GPO_00.  For serial 
   output, BERT_DATA will come out to GPO_01.  For parallel output, 
   BERT_DATA_0:BERT_DATA_7 will come out to GPO_01:GPO_08.
Returns:
   BERR_Code
See Also:
   none
****************************************************************************/
BERR_Code BAST_4538_SetBertInterface(BAST_Handle h, bool bEnable, bool bSerial, bool bClkinv, uint8_t channel);


/***************************************************************************
Summary:
   This function returns the current BERT interface configuration settings.
Description:
   This function returns the current BERT interface configuration settings.
Returns:
   BERR_Code
See Also:
   none
****************************************************************************/
BERR_Code BAST_4538_GetBertInterface(BAST_Handle h, bool *pbEnable, bool *pbSerial, bool *pbClkinv, uint8_t *pChannel);


/***************************************************************************
Summary:
   This function returns AGC status used for computing input power estimate.
Description:
   This function returns AGC status used for computing input power estimate.
Returns:
   BERR_Code
See Also:
   none
****************************************************************************/
BERR_Code BAST_4538_GetAgcStatus(BAST_ChannelHandle h, BAST_4538_AgcStatus *pStatus);


/***************************************************************************
Summary:
   This function reads a register in the BCM3447/BCM3448 that is directly 
   connected to the BCM4538.
Description:
   This function assumes the BCM3447/BCM3448 is connected to BSC_2 of BCM4538.
Returns:
   BERR_Code
See Also:
   none
****************************************************************************/
BERR_Code BAST_4538_ReadBcm3447Register(BAST_Handle h, uint32_t reg, uint32_t *pVal);


/***************************************************************************
Summary:
   This function writes to a register in the BCM3447/BCM3448 that is directly 
   connected to the BCM4538.
Description:
   This function assumes the BCM3447/BCM3448 is connected to BSC_2 of BCM4538.
Returns:
   BERR_Code
See Also:
   none
****************************************************************************/
BERR_Code BAST_4538_WriteBcm3447Register(BAST_Handle h, uint32_t reg, uint32_t val);


BERR_Code BAST_4538_ListenFsk(BAST_Handle h, uint8_t n);
BERR_Code BAST_4538_EnableFskCarrier(BAST_Handle h, bool bEnable);


#ifdef __cplusplus
}
#endif

#endif /* BAST_4538_H__ */


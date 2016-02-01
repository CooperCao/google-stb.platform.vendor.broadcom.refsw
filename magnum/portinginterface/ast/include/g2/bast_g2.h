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
#ifndef _BAST_G2_H__
#define _BAST_G2_H__               

#include "bast.h"
#include "breg_i2c.h"


/******************************************************************************
Summary:
   configuration parameter IDs
Description:
   These are the configuration parameters that can be accessed via 
   BAST_ReadConfig() and BAST_WriteConfig().
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_G2_CONFIG_XTAL_FREQ               0x0000
#define BAST_G2_CONFIG_BCM3445_MI2C_CHANNEL    0x0001
#define BAST_G2_CONFIG_BCM3445_ADDRESS         0x0002
#define BAST_G2_CONFIG_EXT_TUNER_IF_OFFSET     0x0003
#define BAST_G2_CONFIG_TUNER_CTL               0x0004
#define BAST_G2_CONFIG_TUNER_CUTOFF            0x0005
#define BAST_G2_CONFIG_XPORT_CTL               0x0006
#define BAST_G2_CONFIG_LDPC_CTL                0x0007
#define BAST_G2_CONFIG_DISEQC_CTL              0x0008
#define BAST_G2_CONFIG_ACQ_TIME                0x0009
#define BAST_G2_CONFIG_RRTO_USEC               0x000A
#define BAST_G2_CONFIG_DISEQC_PRETX_DELAY      0x000B
#define BAST_G2_CONFIG_SEARCH_RANGE            0x000C
#define BAST_G2_CONFIG_LDPC_SCRAMBLING_SEQ     0x000D
#define BAST_G2_CONFIG_XPORT_CLOCK_ADJUST      0x000E
#define BAST_G2_CONFIG_BLIND_SCAN_MODES        0x000F
#define BAST_G2_CONFIG_PEAK_SCAN_SYM_RATE_MIN  0x0010
#define BAST_G2_CONFIG_PEAK_SCAN_SYM_RATE_MAX  0x0011
#define BAST_G2_CONFIG_REACQ_DELAY             0x0012
#define BAST_G2_CONFIG_VSENSE_THRES_HI         0x0013
#define BAST_G2_CONFIG_VSENSE_THRES_LO         0x0014
#define BAST_G2_CONFIG_DTV_SCAN_CODE_RATES     0x0015
#define BAST_G2_CONFIG_DVB_SCAN_CODE_RATES     0x0016
#define BAST_G2_CONFIG_TURBO_SCAN_MODES        0x0017
#define BAST_G2_CONFIG_VCO_SEPARATION          0x0018
#define BAST_G2_CONFIG_RT_STATUS_INDICATORS    0x0019
#define BAST_G2_CONFIG_FREQ_DRIFT_THRESHOLD    0x001A
#define BAST_G2_CONFIG_RAIN_FADE_THRESHOLD     0x001B
#define BAST_G2_CONFIG_REACQ_CTL               0x001C
#define BAST_G2_CONFIG_RAIN_FADE_WINDOW        0x001E
#define BAST_G2_CONFIG_FTM_TX_POWER            0x001F
#define BAST_G2_CONFIG_FTM_CH_SELECT           0x0020
#define BAST_G2_CONFIG_DISEQC_TONE_THRESHOLD   0x0021
#define BAST_G2_CONFIG_DFT_RANGE_START         0x0022
#define BAST_G2_CONFIG_DFT_RANGE_END           0x0023
#define BAST_G2_CONFIG_DFT_SIZE                0x0024
#define BAST_G2_CONFIG_STUFF_BYTES             0x0025
#define BAST_G2_CONFIG_BCM3445_CTL             0x0026
#define BAST_G2_CONFIG_CNR_THRESH1             0x0027
#define BAST_G2_CONFIG_EXT_TUNER_RF_AGC_TOP    0x0028
#define BAST_G2_CONFIG_EXT_TUNER_IF_AGC_TOP    0x0029
#define BAST_G2_CONFIG_TURBO_CTL               0x002A
#define BAST_G2_CONFIG_NETWORK_SPEC            0x002B
#define BAST_G2_CONFIG_3STAGE_AGC_TOP          0x002C
#define BAST_G2_CONFIG_DFT_FREQ_ESTIMATE       0x002D
#define BAST_G2_CONFIG_DFT_FREQ_ESTIMATE_STATUS 0x002E
#define BAST_G2_CONFIG_ALT_PLC_ACQ_BW          0x002F
#define BAST_G2_CONFIG_ALT_PLC_ACQ_DAMP        0x0030
#define BAST_G2_CONFIG_ALT_PLC_TRK_BW          0x0031
#define BAST_G2_CONFIG_ALT_PLC_TRK_DAMP        0x0032
#define BAST_G2_CONFIG_DISEQC_BIT_THRESHOLD    0x0033

/* put config param lengths here... */
#define BAST_G2_CONFIG_LEN_XTAL_FREQ               4
#define BAST_G2_CONFIG_LEN_BCM3445_MI2C_CHANNEL    1
#define BAST_G2_CONFIG_LEN_BCM3445_ADDRESS         1
#define BAST_G2_CONFIG_LEN_EXT_TUNER_IF_OFFSET     4
#define BAST_G2_CONFIG_LEN_TUNER_CTL               1
#define BAST_G2_CONFIG_LEN_TUNER_CUTOFF            1
#define BAST_G2_CONFIG_LEN_XPORT_CTL               2
#define BAST_G2_CONFIG_LEN_LDPC_CTL                1
#define BAST_G2_CONFIG_LEN_DISEQC_CTL              2
#define BAST_G2_CONFIG_LEN_ACQ_TIME                4
#define BAST_G2_CONFIG_LEN_RRTO_USEC               4
#define BAST_G2_CONFIG_LEN_DISEQC_PRETX_DELAY      1
#define BAST_G2_CONFIG_LEN_SEARCH_RANGE            4
#define BAST_G2_CONFIG_LEN_LDPC_SCRAMBLING_SEQ     16
#define BAST_G2_CONFIG_LEN_XPORT_CLOCK_ADJUST      1
#define BAST_G2_CONFIG_LEN_BLIND_SCAN_MODES        1
#define BAST_G2_CONFIG_LEN_PEAK_SCAN_SYM_RATE_MIN  4
#define BAST_G2_CONFIG_LEN_PEAK_SCAN_SYM_RATE_MAX  4
#define BAST_G2_CONFIG_LEN_REACQ_DELAY             4
#define BAST_G2_CONFIG_LEN_VSENSE_THRES_HI         1
#define BAST_G2_CONFIG_LEN_VSENSE_THRES_LO         1
#define BAST_G2_CONFIG_LEN_DTV_SCAN_CODE_RATES     1
#define BAST_G2_CONFIG_LEN_DVB_SCAN_CODE_RATES     1
#define BAST_G2_CONFIG_LEN_TURBO_SCAN_MODES        2
#define BAST_G2_CONFIG_LEN_VCO_SEPARATION          4
#define BAST_G2_CONFIG_LEN_RT_STATUS_INDICATORS    1
#define BAST_G2_CONFIG_LEN_FREQ_DRIFT_THRESHOLD    4
#define BAST_G2_CONFIG_LEN_RAIN_FADE_THRESHOLD     4
#define BAST_G2_CONFIG_LEN_REACQ_CTL               1
#define BAST_G2_CONFIG_LEN_RAIN_FADE_WINDOW        4
#define BAST_G2_CONFIG_LEN_FTM_TX_POWER            1
#define BAST_G2_CONFIG_LEN_FTM_CH_SELECT           1
#define BAST_G2_CONFIG_LEN_DISEQC_TONE_THRESHOLD   1
#define BAST_G2_CONFIG_LEN_DFT_RANGE_START         2
#define BAST_G2_CONFIG_LEN_DFT_RANGE_END           2
#define BAST_G2_CONFIG_LEN_DFT_SIZE                1
#define BAST_G2_CONFIG_LEN_STUFF_BYTES             1
#define BAST_G2_CONFIG_LEN_BCM3445_CTL             1
#define BAST_G2_CONFIG_LEN_CNR_THRESH1             2
#define BAST_G2_CONFIG_LEN_EXT_TUNER_RF_AGC_TOP    2
#define BAST_G2_CONFIG_LEN_EXT_TUNER_IF_AGC_TOP    2
#define BAST_G2_CONFIG_LEN_TURBO_CTL               1
#define BAST_G2_CONFIG_LEN_NETWORK_SPEC            1
#define BAST_G2_CONFIG_LEN_3STAGE_AGC_TOP          2
#define BAST_G2_CONFIG_LEN_DFT_FREQ_ESTIMATE       4
#define BAST_G2_CONFIG_LEN_DFT_FREQ_ESTIMATE_STATUS 1
#define BAST_G2_CONFIG_LEN_ALT_PLC_ACQ_BW          4
#define BAST_G2_CONFIG_LEN_ALT_PLC_ACQ_DAMP        1
#define BAST_G2_CONFIG_LEN_ALT_PLC_TRK_BW          4
#define BAST_G2_CONFIG_LEN_ALT_PLC_TRK_DAMP        1
#define BAST_G2_CONFIG_LEN_DISEQC_BIT_THRESHOLD    2


/******************************************************************************
Summary:
   BAST_G2_CONFIG_DISEQC_CTL configuration parameter bit definitions
Description:
   BAST_G2_CONFIG_DISEQC_CTL configuration parameter bit definitions
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_G2_DISEQC_CTL_ENVELOPE             0x0001 /* 0=tone mode, 1=envelope mode on TXOUT pin */
#define BAST_G2_DISEQC_CTL_TONE_ALIGN_ENABLE    0x0002 /* tone alignment: 0=off, 1=on */
#define BAST_G2_DISEQC_CTL_RRTO_DISABLE         0x0004 /* 0=RRTO enabled, 1=RRTO disabled */
#define BAST_G2_DISEQC_CTL_TONEBURST_ENABLE     0x0008 /* tone burst: 0=disabled, 1=enabled */
#define BAST_G2_DISEQC_CTL_TONEBURST_B          0x0010 /* applies if tone burst is enabled: 0=tone A, 1=tone B */
#define BAST_G2_DISEQC_CTL_EXP_REPLY_DISABLE    0x0020 /* 0=first byte bit 1 of command indicates reply bytes expected, 1=don't expect reply bytes */
#define BAST_G2_DISEQC_CTL_LNBPU_TXEN           0x0040 /* 0=LNBPU not used , 1= LNBPU on TXEN pin */
#define BAST_G2_DISEQC_CTL_IMPLEMENTED          0x007F


/******************************************************************************
Summary:
   FSK channel select configuration
Description:
   This enum selects the FSK channel configuration within the chip's PHY core.
See Also:
   BAST_ResetFtm(),  BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
typedef enum BAST_FskChannelConfig
{
   BAST_FskChannelConfig_eCh0Tx_Ch0Rx = 0,
   BAST_FskChannelConfig_eCh0Tx_Ch1Rx,
   BAST_FskChannelConfig_eCh1Tx_Ch0Rx,
   BAST_FskChannelConfig_eCh1Tx_Ch1Rx
} BAST_FskChannelConfig;


/******************************************************************************
Summary:
   BAST_G2_CONFIG_XPORT_CTL configuration parameter bit definitions
Description:
   BAST_G2_CONFIG_XPORT_CTL configuration parameter bit definitions
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_G2_XPORT_CTL_0          0x0001
#define BAST_G2_XPORT_CTL_SERIAL     0x0002
#define BAST_G2_XPORT_CTL_2          0x0004
#define BAST_G2_XPORT_CTL_CLKINV     0x0008
#define BAST_G2_XPORT_CTL_CLKSUP     0x0010
#define BAST_G2_XPORT_CTL_VLDINV     0x0020
#define BAST_G2_XPORT_CTL_SYNCINV    0x0040
#define BAST_G2_XPORT_CTL_ERRINV     0x0080
#define BAST_G2_XPORT_CTL_XBERT      0x0100
#define BAST_G2_XPORT_CTL_TEI        0x0200
#define BAST_G2_XPORT_CTL_DELAY      0x0400
#define BAST_G2_XPORT_CTL_BCH_CHECK  0x0800
#define BAST_G2_XPORT_CTL_CRC8_CHECK 0x1000
#define BAST_G2_XPORT_CTL_SYNC1      0x2000
#define BAST_G2_XPORT_CTL_HEAD4      0x4000
#define BAST_G2_XPORT_CTL_DELH       0x8000


/******************************************************************************
Summary:
   BAST_G2_CONFIG_TUNER_CTL configuration parameter bit definitions
Description:
   BAST_G2_CONFIG_TUNER_CTL configuration parameter bit definitions
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_G2_TUNER_CTL_DISABLE_RETRIES          0x01
#define BAST_G2_TUNER_CTL_DISABLE_LO_LOOP_TRACKING 0x02
#define BAST_G2_TUNER_CTL_DISABLE_FS_SMART_TUNING  0x04
#define BAST_G2_TUNER_CTL_SET_FILTER_MANUAL        0x08
#define BAST_G2_TUNER_CTL_DISABLE_LO_DIV_2         0x10
#define BAST_G2_TUNER_CTL_ENABLE_SPLITTER_MODE     0x20
#define BAST_G2_TUNER_CTL_DISABLE_RETUNE           0x40
#define BAST_G2_TUNER_CTL_DISABLE_FDDFS_SWITCHING  0x80


/******************************************************************************
Summary:
   BAST_G2_CONFIG_LDPC_CTL configuration parameter bit definitions
Description:
   BAST_G2_CONFIG_LDPC_CTL configuration parameter bit definitions
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_G2_LDPC_CTL_DISABLE_AUTO_PILOT_PLL    0x01
#define BAST_G2_LDPC_CTL_AWGN_PLC                  0x02
#define BAST_G2_LDPC_CTL_USE_ALT_ACQ_PLC           0x04
#define BAST_G2_LDPC_CTL_USE_ALT_TRK_PLC           0x08
#define BAST_G2_LDPC_CTL_DISABLE_FEC_REACQ         0x10
#define BAST_G2_LDPC_CTL_SHORT_FRAME_USED          0x80 /* read only */


/******************************************************************************
Summary:
   BAST_G2_CONFIG_BCM3445_CTL configuration parameter bit definitions
Description:
   BAST_G2_CONFIG_BCM3445_CTL configuration parameter bit definitions
See Also:
   BAST_ConfigBcm3445(), BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_G2_BCM3445_CTL_LOW_POWER_MODE 0x01
#define BAST_G2_BCM3445_CTL_AGC_TOP_LOW    0x00
#define BAST_G2_BCM3445_CTL_AGC_TOP_MID    0x02
#define BAST_G2_BCM3445_CTL_AGC_TOP_HI     0x04
#define BAST_G2_BCM3445_CTL_AGC_TOP        0x06


/******************************************************************************
Summary:
   BAST_G2_CONFIG_BLIND_SCAN_MODES configuration parameter bit definitions
Description:
   BAST_G2_CONFIG_BLIND_SCAN_MODES configuration parameter bit definitions
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_G2_BLIND_SCAN_MODES_DVB    0x01
#define BAST_G2_BLIND_SCAN_MODES_TURBO  0x02
#define BAST_G2_BLIND_SCAN_MODES_LDPC   0x04
#define BAST_G2_BLIND_SCAN_MODES_DTV    0x08
#define BAST_G2_BLIND_SCAN_MODES_DCII   0x10
#define BAST_G2_BLIND_SCAN_MODES_MASK   0x1F


/******************************************************************************
Summary:
   configuration parameter bit definitions for BAST_G2_CONFIG_DVB_SCAN_CODE_RATES
   and BAST_G2_CONFIG_DTV_SCAN_CODE_RATES
Description:
   bit definition for BAST_G2_CONFIG_DVB_SCAN_CODE_RATES and 
   BAST_G2_CONFIG_DTV_SCAN_CODE_RATES
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_G2_LEGACY_SCAN_1_2  0x01
#define BAST_G2_LEGACY_SCAN_2_3  0x02
#define BAST_G2_LEGACY_SCAN_3_4  0x04
#define BAST_G2_LEGACY_SCAN_5_6  0x08
#define BAST_G2_LEGACY_SCAN_6_7  0x10
#define BAST_G2_LEGACY_SCAN_7_8  0x20


/******************************************************************************
Summary:
   configuration parameter bit definitions for BAST_G2_CONFIG_TURBO_SCAN_MODES
Description:
   bit definition for BAST_G2_CONFIG_TURBO_SCAN_MODES
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_G2_TURBO_QPSK_SCAN_1_2  0x0001
#define BAST_G2_TURBO_QPSK_SCAN_2_3  0x0002
#define BAST_G2_TURBO_QPSK_SCAN_3_4  0x0004
#define BAST_G2_TURBO_QPSK_SCAN_5_6  0x0008
#define BAST_G2_TURBO_QPSK_SCAN_7_8  0x0010
#define BAST_G2_TURBO_8PSK_SCAN_2_3  0x0020
#define BAST_G2_TURBO_8PSK_SCAN_3_4  0x0040
#define BAST_G2_TURBO_8PSK_SCAN_4_5  0x0080
#define BAST_G2_TURBO_8PSK_SCAN_5_6  0x0100
#define BAST_G2_TURBO_8PSK_SCAN_8_9  0x0200
#define BAST_G2_TURBO_SCAN_MASK      0x03FF


/******************************************************************************
Summary:
   real time status indicator bits
Description:
   bit definition for real time status indicators
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_STATUS_EVENT_FREQ_DRIFT 0x01
#define BAST_STATUS_EVENT_RAIN_FADE  0x02
#define BAST_STATUS_EVENT_THRESHOLD1 0x04
#define BAST_STATUS_EVENT_THRESHOLD2 0x08


/******************************************************************************
Summary:
   DFT size used in tone detection
Description:
   valid values for BAST_G2_CONFIG_DFT_SIZE configuration parameter
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_G2_DFT_SIZE_256  0x00  /* 256 pt */
#define BAST_G2_DFT_SIZE_512  0x01  /* 512 pt */
#define BAST_G2_DFT_SIZE_1024 0x02  /* 1024 pt */
#define BAST_G2_DFT_SIZE_2048 0x03  /* 2048 pt */


/******************************************************************************
Summary:
   Turbo acquisition options
Description:
   bit definition for BAST_G2_CONFIG_TURBO_CTL
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_G2_TURBO_CTL_DISABLE_FEC_REACQ  0x01  /* dont reacquire on FEC lost lock */


/******************************************************************************
Summary:
   reacquisition control options
Description:
   bit definition for BAST_G2_CONFIG_REACQ_CTL
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_G2_REACQ_CTL_FREQ_DRIFT 0x01  /* don't lock if carrier drifts beyond search_range */


/***************************************************************************
Summary:
   This function returns the default settings for g2 module.
Description:
   This function returns the default settings for g2 module.
Returns:
   BERR_Code
See Also:
   BAST_Open()
****************************************************************************/
BERR_Code BAST_g2_GetDefaultSettings(
   BAST_Settings *pSettings   /* [out] Default settings */
);


/***************************************************************************
Summary:
   This function returns the default settings for g2 channel device.
Description:
   This function returns the default settings for g2 channel device.
Returns:
   BERR_Code
See Also:
   BAST_Open()
****************************************************************************/
BERR_Code BAST_g2_GetChannelDefaultSettings(
   BAST_Handle h, 
   uint32_t chnNo, 
   BAST_ChannelSettings *pChnDefSettings
);


/* sds channel interrupt enumeration */
typedef enum BAST_Sds_IntID{
   BAST_Sds_IntID_eLock = 0,
   BAST_Sds_IntID_eNotLock,
   BAST_Sds_IntID_eBaudTimer,
   BAST_Sds_IntID_eBerTimer,
   BAST_Sds_IntID_eGenTimer1,
   BAST_Sds_IntID_eGenTimer2,
   BAST_Sds_IntID_eXtalTimer,
   BAST_Sds_IntID_eIfAgcLeThreshRise,
   BAST_Sds_IntID_eIfAgcLeThreshFall,
   BAST_Sds_IntID_eRfAgcLeThreshRise,
   BAST_Sds_IntID_eRfAgcLeThreshFall,
   BAST_Sds_IntID_eRfAgcGtMaxChange,
   BAST_Sds_IntID_eDiseqcVoltageGtHiThresh,
   BAST_Sds_IntID_eDiseqcVoltageLtLoThresh,
   BAST_Sds_IntID_eDiseqcDone,
   BAST_Sds_IntID_eLdpcHp,
   BAST_Sds_IntID_eMi2c,
   BAST_Sds_IntID_eTurboLock,
   BAST_Sds_IntID_eTurboNotLock,
   BAST_Sds_IntID_eAfecLock,
   BAST_Sds_IntID_eAfecNotLock,
   BAST_Sds_MaxIntID
} BAST_Sds_IntID;


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
void BAST_g2_ResetInterruptCounters(
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
BERR_Code BAST_g2_GetInterruptCount(
   BAST_ChannelHandle h,   /* [in] BAST channel handle */
   BAST_Sds_IntID     idx, /* [in] specifies which interrupt count to return */
   uint32_t           *pCount
);

#ifdef __cplusplus
}
#endif

#endif /* BAST_G2_H__ */


/***************************************************************************
 *     Copyright (c) 2007-2013, Broadcom Corporation
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
 
#ifndef _BAST_4506_H__
#define _BAST_4506_H__               

#ifdef __cplusplus
extern "C" {
#endif

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
#define BCM4506_CONFIG_VERSION               0x0000
#define BCM4506_CONFIG_XTAL_FREQ             0x0001
#define BCM4506_CONFIG_FLAGS                 0x0005
#define BCM4506_CONFIG_BCM3445_ADDRESS       0x0006
#define BCM4506_CONFIG_BCM3445_CTL           0x0007
#define BCM4506_CONFIG_TUNER_CTL             0x0009
#define BCM4506_CONFIG_TUNER_CUTOFF          0x000B
#define BCM4506_CONFIG_XPORT_CTL             0x000D
#define BCM4506_CONFIG_LDPC_CTL              0x0011
#define BCM4506_CONFIG_LDPC_ALT_ACQ_PLC_BW   0x0012
#define BCM4506_CONFIG_LOCK_INDICATOR_PIN    0x001A
#define BCM4506_CONFIG_DISEQC_CTL1           0x001C
#define BCM4506_CONFIG_DISEQC_CTL2           0x001E
#define BCM4506_CONFIG_RRTO_USEC             0x0020
#define BCM4506_CONFIG_TONE_AMPLITUDE        0x0028
#define BCM4506_CONFIG_VBOT_AMPLITUDE        0x0029
#define BCM4506_CONFIG_VTOP_AMPLITUDE        0x002A
#define BCM4506_CONFIG_DISEQC_PRETX_DELAY    0x002B
#define BCM4506_CONFIG_TUNE_MIX_DELAY        0x002C
#define BCM4506_CONFIG_TUNE_MIX_VCREF        0x002D
#define BCM4506_CONFIG_TUNE_DAC_DIV_RATIO    0x002E
#define BCM4506_CONFIG_AGC_CTL               0x002F
#define BCM4506_CONFIG_TUNE_DAC_QP2_CNT      0x0030
#define BCM4506_CONFIG_TUNE_DAC_VCREF        0x0031
#define BCM4506_CONFIG_FTM_CORR_CTL          0x0034
#define BCM4506_CONFIG_FTM_CORR_THRES        0x0038
#define BCM4506_CONFIG_FTM_CORR_PEAK_QUAL    0x003C
#define BCM4506_CONFIG_FSK_TX_FREQ_HZ        0x0034
#define BCM4506_CONFIG_FSK_RX_FREQ_HZ        0x0038
#define BCM4506_CONFIG_FSK_TX_DEV_HZ         0x003C
#define BCM4506_CONFIG_SEARCH_RANGE          0x0040
#define BCM4506_CONFIG_DVB_FINAL_STPLC_LBR   0x0048
#define BCM4506_CONFIG_DVB_FINAL_STPLC       0x004C
#define BCM4506_CONFIG_DTV_FINAL_STPLC       0x0050
#define BCM4506_CONFIG_DCII_FINAL_STPLC      0x0054
#define BCM4506_CONFIG_DTV_1_FINAL_STFLLC    0x0058
#define BCM4506_CONFIG_DTV_1_FINAL_STFLIC    0x005C
#define BCM4506_CONFIG_DTV_2_FINAL_STFLLC    0x0060
#define BCM4506_CONFIG_DTV_2_FINAL_STFLIC    0x0064
#define BCM4506_CONFIG_DVB_1_FINAL_STFLLC    0x0068
#define BCM4506_CONFIG_DVB_1_FINAL_STFLIC    0x006C
#define BCM4506_CONFIG_DVB_2_FINAL_STFLLC    0x0070
#define BCM4506_CONFIG_DVB_2_FINAL_STFLIC    0x0074
#define BCM4506_CONFIG_DCII_FINAL_STFLLC     0x0078
#define BCM4506_CONFIG_DCII_FINAL_STFLIC     0x007C
#define BCM4506_CONFIG_LDPC_SCRAMBLING_SEQ   0x0081
#define BCM4506_CONFIG_FREQ_DRIFT_THRESHOLD  0x00A1
#define BCM4506_CONFIG_RAIN_FADE_THRESHOLD   0x00A5
#define BCM4506_CONFIG_RAIN_FADE_WINDOW      0x00A6
#define BCM4506_CONFIG_ERROR_COUNT_WINDOW    0x00A8
#define BCM4506_CONFIG_XPORT_CLOCK_ADJUST    0x00AA
#define BCM4506_CONFIG_RT_STATUS_INDICATORS  0x00AC
#define BCM4506_CONFIG_DFT_SIZE              0x00AE
#define BCM4506_CONFIG_TURBO_CTL             0x00AF
#define BCM4506_CONFIG_STUFF_BYTES           0x00B0
#define BCM4506_CONFIG_TIMING_LOOP_LOCK      0x00B3
#define BCM4506_CONFIG_LDPC_ALT_TRK_PLC_DAMP 0x00B5
#define BCM4506_CONFIG_DISEQC_TONE_THRESHOLD 0x00B7
#define BCM4506_CONFIG_LDPC_ALT_ACQ_PLC_DAMP 0x00C0
#define BCM4506_CONFIG_SPLITTER_MODE_CTL     0x00C2
#define BCM4506_CONFIG_LOCK_HISTORY          0x00C3
#define BCM4506_CONFIG_DISEQC_PRETX_VHOLD    0x00C4
#define BCM4506_CONFIG_DISEQC_POSTTX_VHOLD   0x00C5
#define BCM4506_CONFIG_BLIND_SCAN_MODES      0x00C6
#define BCM4506_CONFIG_PEAK_SCAN_SYM_RATE_MIN 0x00C7
#define BCM4506_CONFIG_PEAK_SCAN_SYM_RATE_MAX 0x00CB
#define BCM4506_CONFIG_NETWORK_SPEC          0x00CF
#define BCM4506_CONFIG_FTM_TX_POWER          0x00D0
#define BCM4506_CONFIG_FTM_CH_SELECT         0x00D1
#define BCM4506_CONFIG_DTV_SCAN_CODE_RATES   0x00D2
#define BCM4506_CONFIG_DVB_SCAN_CODE_RATES   0x00D4
#define BCM4506_CONFIG_TURBO_SCAN_MODES      0x00D6
#define BCM4506_CONFIG_ACQ_TIME              0x00DE
#define BCM4506_CONFIG_DFT_RANGE_START       0x00E6
#define BCM4506_CONFIG_DFT_RANGE_END         0x00E8
#define BCM4506_CONFIG_CNR_THRESH1           0x00F2
#define BCM4506_CONFIG_LDPC_ALT_TRK_PLC_BW   0x00F6
#define BCM4506_CONFIG_DISEQC_BIT_THRESHOLD  0x00FE

#define BCM4506_CONFIG_FSK_TX_POWER    BCM4506_CONFIG_FTM_TX_POWER
#define BCM4506_CONFIG_FSK_CH_SELECT   BCM4506_CONFIG_FTM_CH_SELECT

#define BCM4506_CONFIG_LEN_VERSION                  1 
#define BCM4506_CONFIG_LEN_XTAL_FREQ                4
#define BCM4506_CONFIG_LEN_FLAGS                    1 
#define BCM4506_CONFIG_LEN_BCM3445_ADDRESS          1 
#define BCM4506_CONFIG_LEN_BCM3445_CTL              1 
#define BCM4506_CONFIG_LEN_TUNER_CTL                1 
#define BCM4506_CONFIG_LEN_TUNER_CUTOFF             1 
#define BCM4506_CONFIG_LEN_XPORT_CTL                2 
#define BCM4506_CONFIG_LEN_LDPC_CTL                 1 
#define BCM4506_CONFIG_LEN_LDPC_ALT_ACQ_PLC_BW      4
#define BCM4506_CONFIG_LEN_LOCK_INDICATOR_PIN       1 
#define BCM4506_CONFIG_LEN_DISEQC_CTL1              1 
#define BCM4506_CONFIG_LEN_DISEQC_CTL2              1 
#define BCM4506_CONFIG_LEN_RRTO_USEC                4 
#define BCM4506_CONFIG_LEN_TONE_AMPLITUDE           1 
#define BCM4506_CONFIG_LEN_VBOT_AMPLITUDE           1 
#define BCM4506_CONFIG_LEN_VTOP_AMPLITUDE           1 
#define BCM4506_CONFIG_LEN_DISEQC_PRETX_DELAY       1 
#define BCM4506_CONFIG_LEN_TUNE_MIX_DELAY           1
#define BCM4506_CONFIG_LEN_TUNE_MIX_VCREF           1
#define BCM4506_CONFIG_LEN_TUNE_DAC_DIV_RATIO       1
#define BCM4506_CONFIG_LEN_AGC_CTL                  1
#define BCM4506_CONFIG_LEN_TUNE_DAC_QP2_CNT         1
#define BCM4506_CONFIG_LEN_TUNE_DAC_VCREF           1
#define BCM4506_CONFIG_LEN_FTM_CORR_CTL             4 
#define BCM4506_CONFIG_LEN_FTM_CORR_THRES           4 
#define BCM4506_CONFIG_LEN_FTM_CORR_PEAK_QUAL       4 
#define BCM4506_CONFIG_LEN_FSK_TX_FREQ_HZ           4
#define BCM4506_CONFIG_LEN_FSK_RX_FREQ_HZ           4
#define BCM4506_CONFIG_LEN_FSK_TX_DEV_HZ            4
#define BCM4506_CONFIG_LEN_SEARCH_RANGE             4 
#define BCM4506_CONFIG_LEN_DVB_FINAL_STPLC_LBR      4 
#define BCM4506_CONFIG_LEN_DVB_FINAL_STPLC          4 
#define BCM4506_CONFIG_LEN_DTV_FINAL_STPLC          4 
#define BCM4506_CONFIG_LEN_DCII_FINAL_STPLC         4 
#define BCM4506_CONFIG_LEN_DTV_1_FINAL_STFLLC       4 
#define BCM4506_CONFIG_LEN_DTV_1_FINAL_STFLIC       4 
#define BCM4506_CONFIG_LEN_DTV_2_FINAL_STFLLC       4 
#define BCM4506_CONFIG_LEN_DTV_2_FINAL_STFLIC       4 
#define BCM4506_CONFIG_LEN_DVB_1_FINAL_STFLLC       4 
#define BCM4506_CONFIG_LEN_DVB_1_FINAL_STFLIC       4 
#define BCM4506_CONFIG_LEN_DVB_2_FINAL_STFLLC       4 
#define BCM4506_CONFIG_LEN_DVB_2_FINAL_STFLIC       4 
#define BCM4506_CONFIG_LEN_DCII_FINAL_STFLLC        4 
#define BCM4506_CONFIG_LEN_DCII_FINAL_STFLIC        4 
#define BCM4506_CONFIG_LEN_LDPC_SCRAMBLING_SEQ     16
#define BCM4506_CONFIG_LEN_FREQ_DRIFT_THRESHOLD     4 
#define BCM4506_CONFIG_LEN_RAIN_FADE_THRESHOLD      1 
#define BCM4506_CONFIG_LEN_RAIN_FADE_WINDOW         2 
#define BCM4506_CONFIG_LEN_ERROR_COUNT_WINDOW       2 
#define BCM4506_CONFIG_LEN_XPORT_CLOCK_ADJUST       1
#define BCM4506_CONFIG_LEN_RT_STATUS_INDICATORS     1 
#define BCM4506_CONFIG_LEN_DFT_SIZE                 1
#define BCM4506_CONFIG_LEN_TURBO_CTL                1
#define BCM4506_CONFIG_LEN_STUFF_BYTES              1
#define BCM4506_CONFIG_LEN_TIMING_LOOP_LOCK         1
#define BCM4506_CONFIG_LEN_LDPC_ALT_TRK_PLC_DAMP    1
#define BCM4506_CONFIG_LEN_DISEQC_TONE_THRESHOLD    1
#define BCM4506_CONFIG_LEN_LDPC_ALT_ACQ_PLC_DAMP    1
#define BCM4506_CONFIG_LEN_SPLITTER_MODE_CTL        1
#define BCM4506_CONFIG_LEN_LOCK_HISTORY             1
#define BCM4506_CONFIG_LEN_DISEQC_PRETX_VHOLD       1
#define BCM4506_CONFIG_LEN_DISEQC_POSTTX_VHOLD      1
#define BCM4506_CONFIG_LEN_BLIND_SCAN_MODES         1
#define BCM4506_CONFIG_LEN_PEAK_SCAN_SYM_RATE_MIN   4
#define BCM4506_CONFIG_LEN_PEAK_SCAN_SYM_RATE_MAX   4
#define BCM4506_CONFIG_LEN_NETWORK_SPEC             1
#define BCM4506_CONFIG_LEN_FTM_TX_POWER             1
#define BCM4506_CONFIG_LEN_FTM_CH_SELECT            1
#define BCM4506_CONFIG_LEN_DTV_SCAN_CODE_RATES      1
#define BCM4506_CONFIG_LEN_DVB_SCAN_CODE_RATES      1
#define BCM4506_CONFIG_LEN_TURBO_SCAN_MODES         2
#define BCM4506_CONFIG_LEN_ACQ_TIME                 4
#define BCM4506_CONFIG_LEN_DFT_RANGE_START          2
#define BCM4506_CONFIG_LEN_DFT_RANGE_END            2
#define BCM4506_CONFIG_LEN_CNR_THRESH1              2
#define BCM4506_CONFIG_LEN_LDPC_ALT_TRK_PLC_BW      4
#define BCM4506_CONFIG_LEN_DISEQC_BIT_THRESHOLD     2

#define BCM4506_CONFIG_LEN_FSK_TX_POWER    BCM4506_CONFIG_LEN_FTM_TX_POWER
#define BCM4506_CONFIG_LEN_FSK_CH_SELECT   BCM4506_CONFIG_LEN_FTM_CH_SELECT


/******************************************************************************
Summary:
   xport_ctl configuration parameter bit definitions
Description:
   These are bit definitions for xport_ctl configuration parameter.
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BCM4506_XPORT_CTL_BYPASS_OPLL 0x0001 /* 1=bypass OPLL */
#define BCM4506_XPORT_CTL_ERRINV      0x0080 /* PS_ERR: 0=active high, 1=active low */
#define BCM4506_XPORT_CTL_SYNCINV     0x0040 /* PS_SYNC: 0=active high, 1=active low */
#define BCM4506_XPORT_CTL_VLDINV      0x0020 /* PS_VALID: 0=active high, 1=active low */
#define BCM4506_XPORT_CTL_CLKSUP      0x0010 /* PS_CLK: 0=runs continuously, 1=suppress when PS_VALID not active */
#define BCM4506_XPORT_CTL_CLKINV      0x0008 /* PS_CLK: 0=active high, 1=active low */
#define BCM4506_XPORT_CTL_SERIAL      0x0002 /* output format: 0=parallel, 1=serial */
#define BCM4506_XPORT_CTL_XBERT       0x0100 /* 0=normal, 1=XBERT */
#define BCM4506_XPORT_CTL_TEI         0x0200 /* transport error indicator: 0=off, 1=on */
#define BCM4506_XPORT_CTL_DELAY       0x0400 /* 0=normal, 1=delay PS_ERR, PS_VALID, PS_DATA relative to PS_CLK */
#define BCM4506_XPORT_CTL_BCH_CHECK   0x0800
#define BCM4506_XPORT_CTL_CRC8_CHECK  0x1000
#define BCM4506_XPORT_CTL_SYNC1       0x2000 /* 0=no effect, 1=PS_SYNC valid for 1 bit in serial mode */
#define BCM4506_XPORT_CTL_HEAD4       0x4000 /* duration of PS_VALID/PS_SYNC: 0=1 byte, 1=4 bytes */
#define BCM4506_XPORT_CTL_DELH        0x8000 /* 0=no effect, 1=delete MPEG header */


/******************************************************************************
Summary:
   diseqc_ctl2 configuration parameter bit definitions
Description:
   These are bit definitions for diseqc_ctl2 configuration parameter.
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BCM4506_DISEQC_CTL2_ENVELOPE     0x01 /* 0=tone mode, 1=envelope mode */
#define BCM4506_DISEQC_CTL2_TONE_ALIGN   0x02 /* tone alignment: 0=off, 1=on */
#define BCM4506_DISEQC_CTL2_DISABLE_RRTO 0x04 /* 0=RRTO enabled, 1=RRTO disabled */
#define BCM4506_DISEQC_CTL2_TB_ENABLE    0x08 /* tone burst: 0=disabled, 1=enabled */
#define BCM4506_DISEQC_CTL2_TB_B                0x10 /* applies if tone burst is enabled: 0=tone A, 1=tone B */
#define BCM4506_DISEQC_CTL2_EXP_REPLY_DISABLE   0x20 /* 0=first byte bit 1 of command indicates reply bytes expected, 1=don't expect reply bytes (over-rides bit 7) */
#define BCM4506_DISEQC_CTL2_DISABLE_RX_ONLY     0x40 /* rx only mode (send null command), 0=enabled, 1=disabled */
#define BCM4506_DISEQC_CTL2_OVERRIDE_REPLY_BIT  0x80 /* 0=framing byte bit 1 determines reply, 1=override framing byte and always expect reply (if not disabled by bit 5) */


/******************************************************************************
Summary:
   BCM4506_CONFIG_BCM3445_CTL configuration parameter bit definitions
Description:
   BCM4506_CONFIG_BCM3445_CTL configuration parameter bit definitions
See Also:
   BAST_ConfigBcm3445(), BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BCM4506_BCM3445_CTL_LOW_POWER_MODE 0x01
#define BCM4506_BCM3445_CTL_AGC_TOP_LOW    0x00
#define BCM4506_BCM3445_CTL_AGC_TOP_MID    0x02
#define BCM4506_BCM3445_CTL_AGC_TOP_HI     0x04
#define BCM4506_BCM3445_CTL_AGC_TOP        0x06
#define BCM4506_BCM3445_CTL_4507_DISABLE_BBAND_FILTER_CFG 0x08
#define BCM4506_BCM3445_CTL_METER_MODE     0x10
#define BCM4506_BCM3445_CTL_DISABLE_NOTCH  0x20


/******************************************************************************
Summary:
   configuration parameter bit definitions for BCM4506_CONFIG_DVB_SCAN_CODE_RATES
   and BCM4506_CONFIG_DTV_SCAN_CODE_RATES
Description:
   bit definition for BCM4506_CONFIG_DVB_SCAN_CODE_RATES and 
   BCM4506_CONFIG_DTV_SCAN_CODE_RATES
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_4506_LEGACY_SCAN_1_2  0x01
#define BAST_4506_LEGACY_SCAN_2_3  0x02
#define BAST_4506_LEGACY_SCAN_3_4  0x04
#define BAST_4506_LEGACY_SCAN_5_6  0x08
#define BAST_4506_LEGACY_SCAN_6_7  0x10
#define BAST_4506_LEGACY_SCAN_7_8  0x20


/******************************************************************************
Summary:
   configuration parameter bit definitions for BCM4506_CONFIG_TURBO_SCAN_MODES
Description:
   bit definition for BCM4506_CONFIG_TURBO_SCAN_MODES
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_4506_TURBO_QPSK_SCAN_1_2  0x0001
#define BAST_4506_TURBO_QPSK_SCAN_2_3  0x0002
#define BAST_4506_TURBO_QPSK_SCAN_3_4  0x0004
#define BAST_4506_TURBO_QPSK_SCAN_5_6  0x0008
#define BAST_4506_TURBO_QPSK_SCAN_7_8  0x0010
#define BAST_4506_TURBO_8PSK_SCAN_2_3  0x0020
#define BAST_4506_TURBO_8PSK_SCAN_3_4  0x0040
#define BAST_4506_TURBO_8PSK_SCAN_4_5  0x0080
#define BAST_4506_TURBO_8PSK_SCAN_5_6  0x0100
#define BAST_4506_TURBO_8PSK_SCAN_8_9  0x0200
#define BAST_4506_TURBO_SCAN_MASK      0x03FF


/******************************************************************************
Summary:
   BCM4506_CONFIG_BLIND_SCAN_MODES configuration parameter bit definitions
Description:
   BCM4506_CONFIG_BLIND_SCAN_MODES configuration parameter bit definitions
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_4506_BLIND_SCAN_MODES_DVB    0x01
#define BAST_4506_BLIND_SCAN_MODES_TURBO  0x02
#define BAST_4506_BLIND_SCAN_MODES_LDPC   0x04
#define BAST_4506_BLIND_SCAN_MODES_DTV    0x08
#define BAST_4506_BLIND_SCAN_MODES_DCII   0x10
#define BAST_4506_BLIND_SCAN_MODES_MASK   0x1F


/******************************************************************************
Summary:
   FSK channel select configuration used by BCM4506_CONFIG_FTM_CH_SELECT
Description:
   This enum selects the FSK channel configuration within the chip's PHY core.
See Also:
   BAST_ResetFtm(), BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
typedef enum BAST_FskChannelConfig
{
   BAST_FskChannelConfig_eCh0Tx_Ch0Rx = 0,
   BAST_FskChannelConfig_eCh0Tx_Ch1Rx,
   BAST_FskChannelConfig_eCh1Tx_Ch0Rx, /* not supported */
   BAST_FskChannelConfig_eCh1Tx_Ch1Rx
} BAST_FskChannelConfig;


/***************************************************************************
Summary:
   This function returns the default settings for 4506 module.
Description:
   This function returns the default settings for 4506 module.
Returns:
   BERR_Code
See Also:
   BAST_Open()
****************************************************************************/
BERR_Code BAST_4506_GetDefaultSettings(
   BAST_Settings *pSettings   /* [out] Default settings */
);


/***************************************************************************
Summary:
   This function returns the default settings for 4506 channel device.
Description:
   This function returns the default settings for 4506 channel device.
Returns:
   BERR_Code
See Also:
   BAST_Open()
****************************************************************************/
BERR_Code BAST_4506_GetChannelDefaultSettings(
   BAST_Handle h, 
   uint32_t chnNo, 
   BAST_ChannelSettings *pChnDefSettings
);


BERR_Code BAST_4506_WriteHostRegister(BAST_Handle h, uint8_t address, uint8_t *data);
BERR_Code BAST_4506_ReadHostRegister(BAST_Handle h, uint8_t address, uint8_t *data);

#ifdef BAST_4506_INCLUDE_VCO_AVOIDANCE
BERR_Code BAST_4506_GetTunerActualVco(BAST_ChannelHandle h, uint32_t *pVco);
BERR_Code BAST_4506_GetTunerExpectedVco(BAST_ChannelHandle h, uint32_t freq, uint32_t *pVco, uint32_t *pDiv);
uint32_t BAST_4506_GetClosestXtalHarmonic(uint32_t vco);
BERR_Code BAST_4506_GetNumTuners(BAST_Handle h, uint8_t *nTuners);
#endif

BERR_Code BAST_4506_ListenFsk(BAST_Handle h, uint8_t n);
BERR_Code BAST_4506_EnableFskCarrier(BAST_Handle h, bool bEnable);


#ifdef __cplusplus
}
#endif

#endif /* BAST_4506_H__ */


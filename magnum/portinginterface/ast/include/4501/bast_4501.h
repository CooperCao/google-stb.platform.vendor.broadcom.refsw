/***************************************************************************
 *     Copyright (c) 2005-2010, Broadcom Corporation
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

#ifndef _BAST_4501_H__
#define _BAST_4501_H__               

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
#define BCM4501_CONFIG_VERSION             0x0000
#define BCM4501_CONFIG_FLAGS               0x0001
#define BCM4501_CONFIG_FLAGS2              0x0002
#define BCM4501_CONFIG_XTAL_FREQ           0x0003
#define BCM4501_CONFIG_DISEQC_CTL1         0x0007
#define BCM4501_CONFIG_DISEQC_CTL2         0x0009
#define BCM4501_CONFIG_RRTO_USEC           0x000B
#define BCM4501_CONFIG_FTM_CRC_POLY        0x0013
#define BCM4501_CONFIG_AGC_CTL             0x0014
#define BCM4501_CONFIG_FTM_INIT_OPTIONS     0x0016
#define BCM4501_CONFIG_TUNER_CUTOFF        0x0018
#define BCM4501_CONFIG_SYMBOL_RATE         0x001A
#define BCM4501_CONFIG_ACQ_CTL             0x0022
#define BCM4501_CONFIG_XPORT_CTL           0x002A
#define BCM4501_CONFIG_TM_PAD_CTRL         0x002E
#define BCM4501_CONFIG_TM_VREG_CTRL        0x0032
#define BCM4501_CONFIG_SEARCH_RANGE        0x0033
#define BCM4501_CONFIG_BCM3445_CTL         0x0037
#define BCM4501_CONFIG_BCM3440_ADDRESS     0x0038
#define BCM4501_CONFIG_TUNER_FLAGS         0x003B
#define BCM4501_CONFIG_DVB_FINAL_STPLC     0x003D
#define BCM4501_CONFIG_DTV_FINAL_STPLC     0x0041
#define BCM4501_CONFIG_DCII_FINAL_STPLC    0x0045
#define BCM4501_CONFIG_DTV_1_FINAL_STFLLC  0x0049
#define BCM4501_CONFIG_DTV_1_FINAL_STFLIC  0x004D
#define BCM4501_CONFIG_DTV_2_FINAL_STFLLC  0x0051
#define BCM4501_CONFIG_DTV_2_FINAL_STFLIC  0x0055
#define BCM4501_CONFIG_DVB_1_FINAL_STFLLC  0x0059
#define BCM4501_CONFIG_DVB_1_FINAL_STFLIC  0x005D
#define BCM4501_CONFIG_DVB_2_FINAL_STFLLC  0x0061
#define BCM4501_CONFIG_DVB_2_FINAL_STFLIC  0x0065
#define BCM4501_CONFIG_DCII_FINAL_STFLLC   0x0069
#define BCM4501_CONFIG_DCII_FINAL_STFLIC   0x006D
#define BCM4501_CONFIG_ALT_FLIF            0x0071
#define BCM4501_CONFIG_NETWORK_SPEC        0x0229
#define BCM4501_CONFIG_DISEQC_PRETX_VHOLD  0x022C
#define BCM4501_CONFIG_DISEQC_POSTTX_VHOLD 0x022D
#define BCM4501_CONFIG_DISEQC_CMD_COUNT    0x022E
#define BCM4501_CONFIG_DISEQC_IRQ_COUNT    0x0232
#define BCM4501_CONFIG_LDAII_THRESH        0x023D
#define BCM4501_CONFIG_TM_SDS_XPT_PAD_CTRL 0x0241
#define BCM4501_CONFIG_DISEQC_PRETX_DELAY  0x0245
#define BCM4501_CONFIG_STUFF_BYTES         0x0246
#define BCM4501_CONFIG_LDPC_SCRAMBLING_SEQ 0x0249
#define BCM4501_CONFIG_FTM_CORR_CTL        0x026B
#define BCM4501_CONFIG_XPORT_CLOCK_ADJUST  0x0269
#define BCM4501_CONFIG_FTM_CORR_THRES      0x026F
#define BCM4501_CONFIG_FTM_CORR_PEAK_QUAL  0x0273
#define BCM4501_CONFIG_LDPC_NOT_LOCKED_TIMEOUT 0x0277

#define BCM4501_CONFIG_LEN_VERSION              1
#define BCM4501_CONFIG_LEN_FLAGS                1
#define BCM4501_CONFIG_LEN_FLAGS2               1
#define BCM4501_CONFIG_LEN_XTAL_FREQ            4
#define BCM4501_CONFIG_LEN_DISEQC_CTL1          1
#define BCM4501_CONFIG_LEN_DISEQC_CTL2          1
#define BCM4501_CONFIG_LEN_RRTO_USEC            4
#define BCM4501_CONFIG_LEN_FTM_CRC_POLY         1
#define BCM4501_CONFIG_LEN_AGC_CTL              1
#define BCM4501_CONFIG_LEN_FTM_INIT_OPTIONS     2
#define BCM4501_CONFIG_LEN_TUNER_CUTOFF         1
#define BCM4501_CONFIG_LEN_SYMBOL_RATE          4
#define BCM4501_CONFIG_LEN_ACQ_CTL              4
#define BCM4501_CONFIG_LEN_XPORT_CTL            2
#define BCM4501_CONFIG_LEN_TM_PAD_CTRL          4
#define BCM4501_CONFIG_LEN_TM_VREG_CTRL         1
#define BCM4501_CONFIG_LEN_SEARCH_RANGE         4
#define BCM4501_CONFIG_LEN_BCM3445_CTL          1
#define BCM4501_CONFIG_LEN_BCM3440_ADDRESS      1
#define BCM4501_CONFIG_LEN_TUNER_FLAGS          1
#define BCM4501_CONFIG_LEN_DVB_FINAL_STPLC      4
#define BCM4501_CONFIG_LEN_DTV_FINAL_STPLC      4
#define BCM4501_CONFIG_LEN_DCII_FINAL_STPLC     4
#define BCM4501_CONFIG_LEN_DTV_1_FINAL_STFLLC   4
#define BCM4501_CONFIG_LEN_DTV_1_FINAL_STFLIC   4
#define BCM4501_CONFIG_LEN_DTV_2_FINAL_STFLLC   4
#define BCM4501_CONFIG_LEN_DTV_2_FINAL_STFLIC   4
#define BCM4501_CONFIG_LEN_DVB_1_FINAL_STFLLC   4
#define BCM4501_CONFIG_LEN_DVB_1_FINAL_STFLIC   4
#define BCM4501_CONFIG_LEN_DVB_2_FINAL_STFLLC   4
#define BCM4501_CONFIG_LEN_DVB_2_FINAL_STFLIC   4
#define BCM4501_CONFIG_LEN_DCII_FINAL_STFLLC    4
#define BCM4501_CONFIG_LEN_DCII_FINAL_STFLIC    4
#define BCM4501_CONFIG_LEN_ALT_FLIF             4
#define BCM4501_CONFIG_LEN_NETWORK_SPEC         1
#define BCM4501_CONFIG_LEN_DISEQC_PRETX_VHOLD   1
#define BCM4501_CONFIG_LEN_DISEQC_POSTTX_VHOLD  1
#define BCM4501_CONFIG_LEN_DISEQC_CMD_COUNT     2
#define BCM4501_CONFIG_LEN_DISEQC_IRQ_COUNT     2
#define BCM4501_CONFIG_LEN_LDAII_THRESH         4
#define BCM4501_CONFIG_LEN_TM_SDS_XPT_PAD_CTRL  4
#define BCM4501_CONFIG_LEN_DISEQC_PRETX_DELAY   1
#define BCM4501_CONFIG_LEN_STUFF_BYTES          1
#define BCM4501_CONFIG_LEN_LDPC_SCRAMBLING_SEQ  16
#define BCM4501_CONFIG_LEN_XPORT_CLOCK_ADJUST   1
#define BCM4501_CONFIG_LEN_FTM_CORR_CTL         4
#define BCM4501_CONFIG_LEN_FTM_CORR_THRES       4
#define BCM4501_CONFIG_LEN_FTM_CORR_PEAK_QUAL   4
#define BCM4501_CONFIG_LEN_LDPC_NOT_LOCKED_TIMEOUT 1


/******************************************************************************
Summary:
   xport_ctl configuration parameter bit definitions
Description:
   These are bit definitions for xport_ctl configuration parameter.
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BCM4501_XPORT_CTL_ERRINV  0x0080 /* PS_ERR: 0=active high, 1=active low */
#define BCM4501_XPORT_CTL_SYNCINV 0x0040 /* PS_SYNC: 0=active high, 1=active low */
#define BCM4501_XPORT_CTL_VLDINV  0x0020 /* PS_VALID: 0=active high, 1=active low */
#define BCM4501_XPORT_CTL_CLKSUP  0x0010 /* PS_CLK: 0=runs continuously, 1=suppress when PS_VALID not active */
#define BCM4501_XPORT_CTL_CLKINV  0x0008 /* PS_CLK: 0=active high, 1=active low */
#define BCM4501_XPORT_CTL_SERIAL  0x0002 /* output format: 0=parallel, 1=serial */
#define BCM4501_XPORT_CTL_XBERT   0x0100 /* 0=normal, 1=XBERT */
#define BCM4501_XPORT_CTL_TEI     0x0200 /* transport error indicator: 0=off, 1=on */
#define BCM4501_XPORT_CTL_DELAY   0x0400 /* 0=normal, 1=delay PS_ERR, PS_VALID, PS_DATA relative to PS_CLK */
#define BCM4501_XPORT_CTL_BCH_CHECK  0x0800
#define BCM4501_XPORT_CTL_CRC8_CHECK 0x1000
#define BCM4501_XPORT_CTL_SYNC1   0x2000 /* 0=no effect, 1=PS_SYNC valid for 1 bit in serial mode */
#define BCM4501_XPORT_CTL_HEAD4   0x4000 /* duration of PS_VALID/PS_SYNC: 0=1 byte, 1=4 bytes */
#define BCM4501_XPORT_CTL_DELH    0x8000 /* 0=no effect, 1=delete MPEG header */


/******************************************************************************
Summary:
   diseqc_ctl2 configuration parameter bit definitions
Description:
   These are bit definitions for diseqc_ctl2 configuration parameter.
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BCM4501_DISEQC_CTL2_ENVELOPE     0x01 /* 0=tone mode, 1=envelope mode */
#define BCM4501_DISEQC_CTL2_TONE_ALIGN   0x02 /* tone alignment: 0=off, 1=on */
#define BCM4501_DISEQC_CTL2_DISABLE_RRTO 0x04 /* 0=RRTO enabled, 1=RRTO disabled */
#define BCM4501_DISEQC_CTL2_TB_ENABLE    0x08 /* tone burst: 0=disabled, 1=enabled */
#define BCM4501_DISEQC_CTL2_TB_B         0x10 /* applies if tone burst is enabled: 0=tone A, 1=tone B */
#define BCM4501_DISEQC_CTL2_EXP_REPLY_DISABLE      0x20 /* 0=first byte bit 1 of command indicates reply bytes expected, 1=dont expect reply bytes */
#define BCM4501_DISEQC_CTL2_PRETX_VOLTAGE_ENABLE   0x40 /* enable pre-diseqc voltage 0=disabled, 1=enabled */
#define BCM4501_DISEQC_CTL2_PRETX_VOLTAGE_VTOP     0x80 /* pre-diseqc voltage level 0=vbot, 1=vtop */


/***************************************************************************
Summary:
   This function returns the default settings for 4501 module.
Description:
   This function returns the default settings for 4501 module.
Returns:
   BERR_Code
See Also:
   BAST_Open()
****************************************************************************/
BERR_Code BAST_4501_GetDefaultSettings(
   BAST_Settings *pSettings   /* [out] Default settings */
);


/***************************************************************************
Summary:
   This function returns the default settings for 4501 channel device.
Description:
   This function returns the default settings for 4501 channel device.
Returns:
   BERR_Code
See Also:
   BAST_Open()
****************************************************************************/
BERR_Code BAST_4501_GetChannelDefaultSettings(
   BAST_Handle h, 
   uint32_t chnNo, 
   BAST_ChannelSettings *pChnDefSettings
);


/***************************************************************************
Summary:
   Structure containing BCM4501-specific top mux settings.
Description:
   Structure containing BCM4501-specific top mux settings.
See Also:
   BAST_SetTmConfig(), BAST_GetTmConfig()
****************************************************************************/
typedef struct BAST_4501_TmSettings
{
   uint32_t tm_pad_ctrl;
   uint32_t tm_sds_xpt_pad_ctrl;
   uint32_t tm_vreg_ctrl;
} BAST_4501_TmSettings;


/* control word bit definitions used in BAST_ConfigAgc() */ 
#define BAST_AGC_CONFIG_RF_AGC0 0x00000080 /* state of RF_AGC0 pin */
#define BAST_AGC_CONFIG_RF_AGC1 0x00000040 /* state of RF_AGC1 pin */
#define BAST_AGC_CONFIG_IF_AGC0 0x00000020 /* state of IF_AGC0 pin */
#define BAST_AGC_CONFIG_IF_AGC1 0x00000010 /* state of IF_AGC1 pin */ 
#define BAST_AGC_CONFIG_SET_RF_AGC_PIN 0x00000008 /* RF AGC PIN control: 0=auto, 1=host */
#define BAST_AGC_CONFIG_SET_IF_AGC_PIN 0x00000004 /* IF AGC PIN control: 0=none, 1=host */


BERR_Code BAST_4501_WriteHostRegister(BAST_Handle h, uint8_t address, uint8_t *data);
BERR_Code BAST_4501_ReadHostRegister(BAST_Handle h, uint8_t address, uint8_t *data);


#ifdef __cplusplus
}
#endif

#endif /* BAST_4501_H__ */


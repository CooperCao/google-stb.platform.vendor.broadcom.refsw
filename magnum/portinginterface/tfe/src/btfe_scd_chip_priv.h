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
 * Module Description:
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

/****************************************************************************/
/*
 *  SCD
 */
/****************************************************************************/

#ifndef BTFE_SCD_CHIP_PRIV_H__
#define BTFE_SCD_CHIP_PRIV_H__

#define REMOVE_FDC
#define REMOVE_LAGC

/****************************************************************************/
/* types */
/****************************************************************************/

typedef struct
{
#if (!defined REMOVE_FDC)
    uint32_t FdcRefDivider;
    SCD_FDC_SYMBOL_RATE FdcSymbolRate;
#endif
    SCD_CONFIG__CHANNEL_SCAN_CONTROL ChannelScanControl;
    SCD_CONFIG__FAT_DATA FatData;
    SCD_CONFIG__ACQUISITION Acquisition;
    uint32_t defaultIFNomRate;
    uint32_t FatModFormat;
    SCD_CONFIG__J83ABC j83abc;

    SCD_CONFIG__RF_OFFSET rfOffset;
    uint8_t power_control_0;
    uint8_t power_control_1;
    uint8_t power_control_2;
    uint8_t power_control_3;
#if (!defined REMOVE_LAGC)
    int32_t FatAgcMode;
    int32_t *pFatAgcDataVSB;
    int32_t *pFatAgcData64QAM;
    int32_t *pFatAgcData256QAM;
#endif

    int32_t *pFdcAgcData;
    SCD_BERT_INPUT BertInput;
    uint32_t FwVersion;
    uint32_t FatIfFrequency;
    uint32_t tunerIfFrequency;
    bool tunerSpectrum;
    int32_t IfFrequencyShift;
    uint32_t FwCRCvalue;
    uint32_t MISCandADCRef;
    uint32_t ADCRefMilliVolt;
} CHIP;



/****************************************************************************/
/* prototypes */
/****************************************************************************/

SCD_RESULT BTFE_P_X233AddChip(uint32_t chip_instance, uint32_t fat_tuner_instance, uint32_t fdc_tuner_instance, SCD_HANDLE *chip_handle, uint8_t *pMicroCode);

/****************************************************************************/

#endif

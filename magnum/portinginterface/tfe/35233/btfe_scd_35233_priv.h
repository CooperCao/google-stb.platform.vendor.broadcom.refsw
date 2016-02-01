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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ************************************************************/
 
/****************************************************************************/
/*
 *  SCD 35233
 */
/****************************************************************************/

#ifndef SCD_35233_H
#define SCD_35233_H

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
    SCD_CONFIG__UNIFIED_COFDM unifiedCofdm;
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
    int32_t *pUagcDataGenericNTSC;
    int32_t *pUagcDataGenericPAL;
    int32_t *pUagcDataGenericSECAM;
    int32_t *pUagcDataGenericVSB_QAM;
    int32_t *pIFDconfig;
    int32_t *pUAGC_IF_VGAconfig;
    int32_t *pUAGCdigitalConfig;
    int32_t *pUAGCanalogNegConfig;
    int32_t *pUAGCanalogPosConfig;
    int32_t *pFatAgcDataCOFDM;
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
    uint32_t *UagcConfig_General;
    uint32_t UagcConfig_General_Length;
    uint32_t *UagcConfig_Analog;
    uint32_t UagcConfig_Analog_Length;
    uint32_t *UagcConfig_Analog_NegativeModulation;
    uint32_t UagcConfig_Analog_NegativeModulation_Length;
    uint32_t *UagcConfig_Analog_PositiveModulation;
    uint32_t UagcConfig_Analog_PositiveModulation_Length;
    uint32_t *UagcConfig_Analog_60Hz;
    uint32_t UagcConfig_Analog_60Hz_Length;
    uint32_t *UagcConfig_Analog_50Hz;
    uint32_t UagcConfig_Analog_50Hz_Length;
    uint32_t *UagcConfig_Digital_Modulation;
    uint32_t UagcConfig_Digital_Modulation_Length;
} CHIP;



/****************************************************************************/
/* prototypes */
/****************************************************************************/

SCD_RESULT BTFE_P_X233AddChip(uint32_t chip_instance, uint32_t fat_tuner_instance, uint32_t fdc_tuner_instance, SCD_HANDLE *chip_handle, uint8_t *pMicroCode);

/****************************************************************************/

#endif

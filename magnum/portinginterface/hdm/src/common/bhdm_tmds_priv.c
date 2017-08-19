/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
/*****************************************************************************/
/*                                                                           */
/*                            DO NOT HAND EDIT!!!                            */
/*                                                                           */
/* Parameters generated on 07/19/2014 at 03:54:25 PM                         */
/*                                                                           */
/*                            DO NOT HAND EDIT!!!                            */
/*                                                                           */
/*    WARNING!!! This file is auto-generated file.  The ordering of the      */
/*               various enums and tables is important to the overall        */
/*               functionality!!                                             */
/*                                                                           */
/*   Did we mention?:        DO NOT HAND EDIT!!!                             */
/*                                                                           */
/*****************************************************************************/
#include "bhdm.h"
#include "bhdm_priv.h"

BDBG_MODULE(BHDM_TMDS_PRIV) ;


/* For boot loader usage */
#ifndef BHDM_FOR_BOOTUPDATER
/******************************************************************************
 * Summary:
 * HDMI Pixel Clock Rate in kHz - Useful for debug messages
 * NOTE: The order of these entries must match the order of the entries in the
 *       BHDM_P_TmdsClock enum declaration
 *******************************************************************************/
 static const uint32_t BHDM_P_TmdsClockValue[BHDM_P_TmdsClock_eMax] =
{
	  25200,              /*   0 */
	  31500,              /*   1 */
	  37800,              /*   2 */
	  25175,              /*   3 */
	  31469,              /*   4 */
	  37762,              /*   5 */
	  27000,              /*   6 */
	  33750,              /*   7 */
	  40500,              /*   8 */
	  54000,              /*   9 */
	  67500,              /*  10 */
	  81000,              /*  11 */
	 108000,              /*  12 */
	 135000,              /*  13 */
	 162000,              /*  14 */
	  27027,              /*  15 */
	  33784,              /*  16 */
	  40540,              /*  17 */
	  54054,              /*  18 */
	  67567,              /*  19 */
	  81081,              /*  20 */
	 108108,              /*  21 */
	 135135,              /*  22 */
	 162162,              /*  23 */
	  74250,              /*  24 */
	  92813,              /*  25 */
	 111375,              /*  26 */
	  74176,              /*  27 */
	  92720,              /*  28 */
	 111264,              /*  29 */
	 148500,              /*  30 */
	 185625,              /*  31 */
	 222750,              /*  32 */
	 148352,              /*  33 */
	 185440,              /*  34 */
	 222527,              /*  35 */
	 297000,              /*  36 */
	 371250,              /*  37 */
	 445500,              /*  38 */
	 296703,              /*  39 */
	 370879,              /*  40 */
	 445055,              /*  41 */
	 594000,              /*  42 */
	 593407,              /*  43 */
	  65000,              /*  44 */
	  81250,              /*  45 */
	  97500,              /*  46 */
	  64935,              /*  47 */
	  81169,              /*  48 */
	  97403,              /*  49 */
} ;
#endif


/* For boot loader usage */
#ifndef BHDM_FOR_BOOTUPDATER
/******************************************************************************
 * Summary:
 * HDMI Pixel Clock Rate Text - Useful for debug messages
 * NOTE: The order of these entries must match the order of the entries in the
 *       BHDM_P_TmdsClock enum declaration
 *******************************************************************************/
 static const char * const BHDM_P_TmdsClockText[BHDM_P_TmdsClock_eMax] =
{
	BDBG_STRING("25.2"),                 /*   0 */
	BDBG_STRING("31.5"),                 /*   1 */
	BDBG_STRING("37.8"),                 /*   2 */
	BDBG_STRING("25.2 / 1.001"),         /*   3 */
	BDBG_STRING("31.5 / 1.001"),         /*   4 */
	BDBG_STRING("37.8 / 1.001"),         /*   5 */
	BDBG_STRING("27"),                   /*   6 */
	BDBG_STRING("33.75"),                /*   7 */
	BDBG_STRING("40.5"),                 /*   8 */
	BDBG_STRING("54"),                   /*   9 */
	BDBG_STRING("67.5"),                 /*  10 */
	BDBG_STRING("81"),                   /*  11 */
	BDBG_STRING("108"),                  /*  12 */
	BDBG_STRING("135"),                  /*  13 */
	BDBG_STRING("162"),                  /*  14 */
	BDBG_STRING("27 * 1.001"),           /*  15 */
	BDBG_STRING("33.75 * 1.001"),        /*  16 */
	BDBG_STRING("40.5 * 1.001"),         /*  17 */
	BDBG_STRING("54 * 1.001"),           /*  18 */
	BDBG_STRING("67.5 * 1.001"),         /*  19 */
	BDBG_STRING("81 * 1.001"),           /*  20 */
	BDBG_STRING("108 * 1.001"),          /*  21 */
	BDBG_STRING("135 * 1.001"),          /*  22 */
	BDBG_STRING("162 * 1.001"),          /*  23 */
	BDBG_STRING("74.25"),                /*  24 */
	BDBG_STRING("92.8125"),              /*  25 */
	BDBG_STRING("111.375"),              /*  26 */
	BDBG_STRING("74.25 / 1.001"),        /*  27 */
	BDBG_STRING("92.8125 / 1.001"),      /*  28 */
	BDBG_STRING("111.375 / 1.001"),      /*  29 */
	BDBG_STRING("148.5"),                /*  30 */
	BDBG_STRING("185.625"),              /*  31 */
	BDBG_STRING("222.75"),               /*  32 */
	BDBG_STRING("148.5 / 1.001"),        /*  33 */
	BDBG_STRING("185.625 / 1.001"),      /*  34 */
	BDBG_STRING("222.75 / 1.001"),       /*  35 */
	BDBG_STRING("297"),                  /*  36 */
	BDBG_STRING("371.25"),               /*  37 */
	BDBG_STRING("445.5"),                /*  38 */
	BDBG_STRING("297 / 1.001"),          /*  39 */
	BDBG_STRING("371.25 / 1.001"),       /*  40 */
	BDBG_STRING("445.5 / 1.001"),        /*  41 */
	BDBG_STRING("594"),                  /*  42 */
	BDBG_STRING("594 / 1.001"),          /*  43 */
	BDBG_STRING("65"),                   /*  44 */
	BDBG_STRING("81.25"),                /*  45 */
	BDBG_STRING("97.5"),                 /*  46 */
	BDBG_STRING("65 / 1.001"),           /*  47 */
	BDBG_STRING("81.25 / 1.001"),        /*  48 */
	BDBG_STRING("97.5 / 1.001")         /*  49 */
} ;

static const char TmdsClockEnumError[] = "Unknown TMDS Clock" ;


const char * BHDM_P_TmdsClockToText_isrsafe(BHDM_P_TmdsClock eTmdsClock)
{
	uint8_t entries ;

	entries =
		sizeof(BHDM_P_TmdsClockText) /
		sizeof(*BHDM_P_TmdsClockText) ;

	if (eTmdsClock < entries)
		return BHDM_P_TmdsClockText[eTmdsClock] ;
	else
		return TmdsClockEnumError ;
}


uint32_t BHDM_P_TmdsClockToValue_isrsafe(
	BHDM_P_TmdsClock eTmdsClock
)
{
	uint32_t TmdsClockValue  = 0 ;

	if (eTmdsClock < BHDM_P_TmdsClock_eMax)
		TmdsClockValue = BHDM_P_TmdsClockValue[eTmdsClock] ;
	else
	{
		BDBG_ERR(("Unknown Tmds Clock enum: %d", eTmdsClock)) ;
		TmdsClockValue = BHDM_P_TmdsClockValue[BHDM_P_TmdsClock_e27] ;
	}
	return TmdsClockValue ;
}



BERR_Code BHDM_TMDS_P_VideoFormatSettingsToTmdsRate(
	const BHDM_Handle hHDMI,		/* [in] HDMI handle */
	const BFMT_VideoFmt eVideoFmt, /* [in] eVideoFmt */
	const BHDM_Video_Settings *settings, /* [in] settings */
	uint32_t *tmdsRate) /* [in] settings */
{
	BERR_Code rc ;
	const BFMT_VideoInfo *pVideoInfo ;

	BHDM_P_TmdsClock eTmdsClock ;

	uint32_t PixelClock ;
	uint32_t TmdsClockValue ;
	uint16_t BitRate ;
	uint8_t BitsPerColor ;
	uint8_t divider ;

	pVideoInfo = BFMT_GetVideoFormatInfoPtr(eVideoFmt) ;

	rc = BHDM_PACKET_ACR_P_LookupTmdsClock_isrsafe(hHDMI,
		pVideoInfo->ulPxlFreqMask, settings, BAVC_HDMI_PixelRepetition_eNone,
		&eTmdsClock) ;
	if (rc) {rc = BERR_TRACE(rc) ; goto done ;}

	divider = (hHDMI->DeviceSettings.stVideoSettings.eColorSpace == BAVC_Colorspace_eYCbCr420)  ? 2 : 1 ;
	switch(settings->eBitsPerPixel)
	{
	default:
	case BAVC_HDMI_BitsPerPixel_e24bit : BitsPerColor =  8 ; break ;
	case BAVC_HDMI_BitsPerPixel_e30bit : BitsPerColor = 10 ; break ;
	case BAVC_HDMI_BitsPerPixel_e36bit : BitsPerColor = 12 ; break ;
	case BAVC_HDMI_BitsPerPixel_e48bit : BitsPerColor = 16 ; break ;
	}
	BitRate = (BitsPerColor * 100) / 8 ;

	TmdsClockValue = BHDM_P_TmdsClockToValue_isrsafe(eTmdsClock) ;

	PixelClock = 10 * TmdsClockValue ;
	PixelClock = PixelClock + BitRate / divider ;
	*tmdsRate = PixelClock / 10000 ;

	BDBG_MSG(("Video Format (%d) %s to TMDS Rate (eTmdsClock= %d)  %d",
		eVideoFmt, pVideoInfo->pchFormatStr, eTmdsClock, TmdsClockValue)) ;

	BDBG_MSG(("   BitRate: %d ; BitsPerColor: %d    Divider %d", BitRate, BitsPerColor, divider)) ;
	BDBG_MSG(("   Pixel Frequency:  %d MHz (TMDS Character Rate %d Mcsc)",
		pVideoInfo->ulPxlFreq / BFMT_FREQ_FACTOR, *tmdsRate)) ;

done :
	return rc ;
}

#if BHDM_CONFIG_28NM_SUPPORT

static const BHDM_TmdsRatePreEmphasisSettings BHDM_TMDS_P_RatePreEmphasisDefaultSettings[BHDM_TMDS_RANGES] =
{
/*
	{
	TMDS_Rate,
	{{{PreEmp0, MainDriver0}, RES_SEL_DATA0, TERM_RES_SELDATA0},
	    {{PreEmp1, MainDriver1}, RES_SEL_DATA1, TERM_RES_SELDATA1},
	        {{PreEmp2, MainDriver2}, RES_SEL_DATA2, TERM_RES_SELDATA2},
	            {{PreEmpCK, MainDriverCK}, RES_SEL_CK, TERM_RES_SELDATACL}
	 }
 */
/* 0 */	{  0, 50000000,
			{{{0x0, 0x0A}, 0x12, 0x0}, {{0x0, 0x0A}, 0x12, 0x0}, {{0x0, 0x0A}, 0x12,  0x0}}, {{0x0, 0x0A}, 0x18, 0x0}},
/* 1 */	{  50000001, 75000000,
			{{{0x0, 0x09}, 0x12, 0x0}, {{0x0, 0x09}, 0x12, 0x0}, {{0x0, 0x09}, 0x12,  0x0}}, {{0x0, 0x0C}, 0x18, 0x3}},
/* 2 */	{  75000001,   165000000,
			{{{0x0, 0x09}, 0x12, 0x0}, {{0x0, 0x09}, 0x12, 0x0}, {{0x0, 0x09}, 0x12,  0x0}}, {{0x0, 0x0C}, 0x18, 0x3}}, /* H14b cutoff freq */
/* 3 */	{  165000001,  250000000,
			{{{0x0, 0x0F}, 0x12, 0x1}, {{0x0, 0x0F}, 0x12, 0x1}, {{0x0, 0x0F}, 0x12,  0x1}}, {{0x0, 0x0C}, 0x18, 0x3}},
/* 4 */	{    250000001,  340000000,
			{{{0x2, 0x0D}, 0x12, 0x1}, {{0x2, 0x0D}, 0x12, 0x1}, {{0x2, 0x0D}, 0x12,  0x1}}, {{0x0, 0x0C}, 0x18, 0xF}},
/* 5 */	{    340000001,  450000000,
			{{{0x0, 0x1B}, 0x12, 0xF}, {{0x0, 0x1B}, 0x12, 0xF}, {{0x0, 0x1B}, 0x12,  0xF}}, {{0x0, 0x0A}, 0x12, 0xF}},
/* 6 */	{  450000001,  600000000,
			{{{0x0, 0x1C}, 0x12, 0xF}, {{0x0, 0x1C}, 0x12, 0xF}, {{0x0, 0x1C}, 0x12,  0xF}}, {{0x0, 0x0B}, 0x13, 0xF}}
} ;


void BHDM_TMDS_P_GetDefaultPreEmphasisRegisters(
	BHDM_TmdsPreEmphasisRegisters *TmdsPreEmphasisRegisters
)
{
	uint32_t Register ;
	uint8_t index ;
	uint8_t preemp ;
	uint8_t resistorSelect ;
	uint8_t terminationResistorSelect ;

	for (index = 0 ; index < BHDM_TMDS_RANGES ; index++)
	{
		TmdsPreEmphasisRegisters[index].MinTmdsRate =
			BHDM_TMDS_P_RatePreEmphasisDefaultSettings[index].MinTmdsRate ;

		TmdsPreEmphasisRegisters[index].MaxTmdsRate =
			BHDM_TMDS_P_RatePreEmphasisDefaultSettings[index].MaxTmdsRate ;

		/*********************/
		/* HDMI_TX_PHY_CTL_0 */
		/*********************/
		Register = 0 ;

		preemp =
		  (BHDM_TMDS_P_RatePreEmphasisDefaultSettings[index].Channel[2].Amplitude.PreEmphasis << 5)
		| BHDM_TMDS_P_RatePreEmphasisDefaultSettings[index].Channel[2].Amplitude.MainDriver ;
		Register |= BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_0, PREEMP_2, preemp) ;

		preemp =
		  (BHDM_TMDS_P_RatePreEmphasisDefaultSettings[index].Channel[1].Amplitude.PreEmphasis << 5)
		| BHDM_TMDS_P_RatePreEmphasisDefaultSettings[index].Channel[1].Amplitude.MainDriver ;
		Register |= BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_0, PREEMP_1, preemp) ;

		preemp =
		  (BHDM_TMDS_P_RatePreEmphasisDefaultSettings[index].Channel[0].Amplitude.PreEmphasis << 5)
		| BHDM_TMDS_P_RatePreEmphasisDefaultSettings[index].Channel[0].Amplitude.MainDriver ;
		Register |= BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_0, PREEMP_0, preemp) ;

		preemp =
		  (BHDM_TMDS_P_RatePreEmphasisDefaultSettings[index].Clock.Amplitude.PreEmphasis << 5)
		| BHDM_TMDS_P_RatePreEmphasisDefaultSettings[index].Clock.Amplitude.MainDriver ;
		Register |= BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_0, PREEMP_CK, preemp) ;

		TmdsPreEmphasisRegisters[index].HDMI_TX_PHY_CTL_0 = Register ;

		/*********************/
		/* HDMI_TX_PHY_CTL_1 */
		/*********************/
		Register = 0 ;
		resistorSelect = BHDM_TMDS_P_RatePreEmphasisDefaultSettings[index].Channel[2].ResSelData ;
		Register |= BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_1, RES_SEL_DATA2, resistorSelect) ;

		resistorSelect = BHDM_TMDS_P_RatePreEmphasisDefaultSettings[index].Channel[1].ResSelData ;
		Register |= BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_1, RES_SEL_DATA1, resistorSelect) ;

		resistorSelect = BHDM_TMDS_P_RatePreEmphasisDefaultSettings[index].Channel[0].ResSelData ;
		Register |= BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_1, RES_SEL_DATA0, resistorSelect) ;

		resistorSelect = BHDM_TMDS_P_RatePreEmphasisDefaultSettings[index].Clock.ResSelData ;
		Register |= BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_1, RES_SEL_CK, resistorSelect) ;

		TmdsPreEmphasisRegisters[index].HDMI_TX_PHY_CTL_1 = Register ;



		/*********************/
		/* HDMI_TX_PHY_CTL_2 */
		/*********************/
		Register = 0 ;
		terminationResistorSelect = BHDM_TMDS_P_RatePreEmphasisDefaultSettings[index].Channel[2].TermResSelData;
		Register |= BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_2, TERM_RES_SELDATA2, terminationResistorSelect) ;

		terminationResistorSelect = BHDM_TMDS_P_RatePreEmphasisDefaultSettings[index].Channel[1].TermResSelData;
		Register |= BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_2, TERM_RES_SELDATA1, terminationResistorSelect) ;

		terminationResistorSelect = BHDM_TMDS_P_RatePreEmphasisDefaultSettings[index].Channel[0].TermResSelData;
		Register |= BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_2, TERM_RES_SELDATA0, terminationResistorSelect) ;

		terminationResistorSelect = BHDM_TMDS_P_RatePreEmphasisDefaultSettings[index].Clock.TermResSelData;
		Register |= BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_2, TERM_RES_SELCK, terminationResistorSelect) ;

		TmdsPreEmphasisRegisters[index].HDMI_TX_PHY_CTL_2 = Register ;

		BDBG_MSG(("PreEmphasis Registers for TMDS Rate %d to %d",
			TmdsPreEmphasisRegisters[index].MinTmdsRate,
			TmdsPreEmphasisRegisters[index].MaxTmdsRate)) ;
		BDBG_MSG(("< HDMI_TX_PHY_CTL_0 %#x",
			TmdsPreEmphasisRegisters[index].HDMI_TX_PHY_CTL_0)) ;
		BDBG_MSG(("< HDMI_TX_PHY_CTL_1 %#x",
			TmdsPreEmphasisRegisters[index].HDMI_TX_PHY_CTL_1)) ;
		BDBG_MSG(("< HDMI_TX_PHY_CTL_2 %#x",
			TmdsPreEmphasisRegisters[index].HDMI_TX_PHY_CTL_2)) ;
		BDBG_MSG((" ")) ;
	}

}


void BHDM_TMDS_GetPreEmphasisRegisters(
	BHDM_Handle hHDMI,
	BHDM_TmdsPreEmphasisRegisters *PreEmphasisRegisters
)
{
	BKNI_Memcpy(PreEmphasisRegisters, &hHDMI->TmdsPreEmphasisRegisters,
		sizeof(BHDM_TmdsPreEmphasisRegisters) * BHDM_TMDS_RANGES) ;
}

void BHDM_TMDS_SetPreEmphasisRegisters(
	BHDM_Handle hHDMI,
	const BHDM_TmdsPreEmphasisRegisters *TmdsPreEmphasisRegisters
)
{
	BKNI_Memcpy(&hHDMI->TmdsPreEmphasisRegisters, TmdsPreEmphasisRegisters,
		sizeof(BHDM_TmdsPreEmphasisRegisters) * BHDM_TMDS_RANGES) ;
}
#else

void BHDM_TMDS_GetPreEmphasisRegisters(
	BHDM_Handle hHDMI,
	BHDM_TmdsPreEmphasisRegisters *TmdsPreEmphasisRegisters
)
{
	BSTD_UNUSED(hHDMI) ;
	BSTD_UNUSED(TmdsPreEmphasisRegisters) ;
}

void BHDM_TMDS_SetPreEmphasisRegisters(
	BHDM_Handle hHDMI,
	const BHDM_TmdsPreEmphasisRegisters *TmdsPreEmphasisRegisters
)
{
	BSTD_UNUSED(hHDMI) ;
	BSTD_UNUSED(TmdsPreEmphasisRegisters) ;
}
#endif

#endif

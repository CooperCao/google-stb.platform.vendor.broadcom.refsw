/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/

#include "bdsp_common_priv_include.h"

BDBG_MODULE(bdsp_cit);

const char Algorithm2Name[BDSP_Algorithm_eMax][BDSP_MAX_CHAR_LENGTH] =
{
	{"MPEG DECODE"},
	{"MPEG PASSTHRU"},
	{"MPEG ENCODE"},
	{"AAC ADTS DECODE"},
	{"AAC ADTS PASSTHRU"},
	{"AAC LOAS DECODE"},
	{"AAC LOAS PASSTHRU"},
	{"AAC ENCODE"},
	{"DOLBY PULSE ADTS DECODE"},
	{"DOLBY PULSE LOAS DECODE"},
#ifdef BDSP_DDP_SUPPORT
	{"AC3 DECODE (USING DDP)"},
#else
	{"AC3 DECODE"},
#endif
	{"AC3 PASSTHRU"},
	{"AC3 ENCODE"},
	{"AC3+ DECODE"},
	{"AC3+ PASSTHRU"},
	{"DTS CORE ENCODE"},
	{"DTS HD DECODE"},
	{"DTS HD PASSTHRU"},
	{"DTS 14BIT DECODE"},
	{"DTS 14BIT PASSTHRU"},
	{"DTS LBR DECODE"},
	{"WMA DECODE"},
	{"WMA PRO DECODE"},
	{"MLP DECODE"},
	{"MLP PASSTHRU"},
	{"AMR NB DECODE"},
	{"AMR NB ENCODE"},
	{"AMR WB DECODE"},
	{"AMR WB ENCODE"},
	{"DRA DECODE"},
	{"COOK DECODE"},
	{"VORBIS DECODE"},
	{"FLAC DECODE"},
	{"MAC DECODE"},
	{"G711 DECODE"},
	{"G711 ENCODE"},
	{"G726 DECODE"},
	{"G726 ENCODE"},
	{"G729 DECODE"},
	{"G729 ENCODE"},
	{"G723_1 DECODE"},
	{"G723_1 ENCODE"},
	{"LPCM DVD DECODE"},
	{"LPCM 1394 DECODE"},
	{"LPCM BD DECODE"},
	{"PCM WAV DECODE"},
	{"PCM DECODE"},
	{"ADPCM DECODE"},
	{"iLBC DECODE"},
	{"iSAC DECODE"},
	{"iLBC ENCODE"},
	{"iSAC ENCODE"},
	{"LPCM ENCODE"},
	{"UDC DECODE"},
	{"UDC PASSTHRU"},
	{"DOLBY AACHE ADTS DECODE"},
	{"DOLBY AACHE LOAS DECODE"},
	{"OPUS DECODE"},
	{"ALS DECODE"},
	{"ALS LOAS DECODE"},
	{"AC4 DECODE"},
	{"OPUS ENCODE"},
	{"DDP ENCODE"},
	{"GENERIC PASSTHRU"},
	{"MIXER"},
	{"MIXER DAPV2"},
	{"SAMPLE RATE CONVERTER"},
	{"DSOLA"},
	{"GEN CDB ITB"},
	{"BRCM AVL"},
	{"BRCM 3D SURROUND"},
	{"SRS TRU SURROUND"},
	{"SRS TRU VOLUME"},
	{"DDRE"},
	{"DV258"},
	{"DPCMR"},
	{"CUSTOM VOICE"},
	{"BTSC ENCODER"},
	{"KARAOKE"},
	{"OUTPUT FORMATTER"},
	{"VACAL PP"},
	{"FADE CONTROL"},
	{"AMBISONICS"},
	{"TSM CORRECTION"},
	{"SPEEXAEC"},
	{"VP6 DECODE"},
	{"H264 ENCODE"},
	{"X264 ENCODE"},
	{"XVP8 ENCODE"},
	{"SECURITY A"},
	{"SECURITY B"},
	{"SECURITY C"},
};

const char PortType[BDSP_AF_P_PortType_eLast][BDSP_MAX_CHAR_LENGTH] =
{
	{"FMM"},
	{"RAVE"},
	{"RDB"},
	{"INTER TASK"},
	{"INTER STAGE"},
	{"ANDROID AUDIO"}
};

const char DataType[BDSP_DataType_eMax][BDSP_MAX_CHAR_LENGTH] =
{
	{"PCM MONO"},
	{"PCM STEREO"},
	{"PCM 5.1"},
    {"PCM 7.1"},
    {"PCM Rf"},
	{"IEC61937"},
	{"IEC61937*4"},
	{"IEC61937*16"},
	{"COMPRESSED RAW"},
	{"RAVE"},
	{"DOLBY TRANSCODED DATA"},
	{"RDB CDB"},
    {"RDB ITB"},
    {"RDB ANCILLARY"},
    {"RDB POOL"},
};

const char DistinctOutputType[BDSP_AF_P_DistinctOpType_eMax][BDSP_MAX_CHAR_LENGTH] =
{
	{"7.1 PCM"},
	{"5.1 PCM"},
	{"STEREO PCM"},
	{"MONO PCM"},
	{"COMPRESSED"},
	{"COMPRESSED 4X"},
	{"COMPRESSED HBR"},
	{"DOLBY RE-ENCODED AUX"},
	{"CDB"},
	{"ITB"},
	{"ANCILLARY DATA"},
	{"DESCRIPTOR QUEUE"},
	{"GENERIC INTERSTAGE"}
};

const char BufferType[BDSP_AF_P_BufferType_eLast][BDSP_MAX_CHAR_LENGTH] =
{
	{"FMM"},
	{"FMM SLAVE"},
	{"RAVE"},
	{"RDB"},
	{"DRAM"},
	{"BUFFER POOL"},
	{"LINEAR"}
};

const char DataAccessType[BDSP_AF_P_Port_eLast][BDSP_MAX_CHAR_LENGTH] =
{
	{"NONE"},
	{"STANDARD"},
	{"SAMPLE INTERLEAVED PCM"},
	{"CHANNEL INTERLEAVED PCM"},
	{"META DATA ACCESS"},
};

const char BaseRateMultiplier[BDSP_AF_P_FmmDstFsRate_eMax+1][BDSP_MAX_CHAR_LENGTH] =
{
	{"Base Rate"},
	{"Stream Sample Rate"},
	{"2X Base Rate"},
	{"4X Base Rate"},
	{"16XBase Rate/HBR"},
	{"Invalid"}
};

const char FMMContentType [BDSP_AF_P_FmmContentType_eMax+1][BDSP_MAX_CHAR_LENGTH] =
{
	{"PCM"},
	{"Compressed"},
	{"Analog Compressed"},
    {"Invalid"}
};

const char GlobalTimeBase [2][BDSP_MAX_CHAR_LENGTH] =
{
    {"45 Khz "},
    {"27 Mhz "}
};

const char DelayMode[BDSP_AudioTaskDelayMode_eMax+1][BDSP_MAX_CHAR_LENGTH]=
{
    {"Default"},
    {"WD_Low"},
    {"WD_Lowest"},
    {"Invalid"}
};

const char DisableEnable[2][BDSP_MAX_CHAR_LENGTH] =
{
    {"Disabled "},
    {"Enabled "}
};

const char TrueFalse[2][BDSP_MAX_CHAR_LENGTH] =
{
    {"False "},
    {"True "}
};

const char ContextType[BDSP_ContextType_eMax+1][BDSP_MAX_CHAR_LENGTH]=
{
    {"Audio"},
    {"Video Decode"},
    {"Video Encode"},
    {"Graphics"},
    {"SCM"},
    {"Invalid"}
};

const char SchedulingMode[BDSP_TaskSchedulingMode_eMax+1][BDSP_MAX_CHAR_LENGTH]=
{
    {"Standalone"},
    {"Master"},
    {"Slave"},
    {"Invalid"}
};

const char FirmwareSchedulingType[BDSP_P_SchedulingMode_eLast][BDSP_MAX_CHAR_LENGTH]=
{
    {"Master"},
    {"Slave"},
};

const char TaskType[BDSP_P_TaskType_eLast][BDSP_MAX_CHAR_LENGTH]=
{
    {"Interrupt"},
    {"Real Time"},
    {"Assured Rate"},
    {"On Demand"},
	{"AFAP"},
};

const char SchedulingType[BDSP_TaskRealtimeMode_eMax+1][BDSP_MAX_CHAR_LENGTH]=
{
    {"Real Time"},
    {"Non-Real Time"},
    {"Soft Real Time"},
    {"On Demand"},
    {"Invalid"}
};

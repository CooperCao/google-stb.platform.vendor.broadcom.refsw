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
#ifndef NEXUS_TSMF_H__
#define NEXUS_TSMF_H__

#include "nexus_types.h"
#include "nexus_playpump.h"
#include "nexus_recpump.h"
#include "nexus_pid_channel.h"

#ifdef __cplusplus
extern "C"{
#endif

/* 53 packets per frame, at 192 bytes each (4 bytes of pacing timetamps, followed by 188 bytes of MPEG transport */
#define NEXUS_TSMF_FRAME_SIZE_BYTES   (53 * 192)

/*
Summary:
Handle which represents a TSMF channel.
*/
typedef struct NEXUS_Tsmf *NEXUS_TsmfHandle;

/***************************************************************************
Summary:
Modes supported for accepting TSMF packets.

Description:
This enum represents the modes that TSMF supports when there is a change for
the version number field inside a frame header.
***************************************************************************/
typedef enum NEXUS_TsmfVersionChangeMode
{
    NEXUS_TsmfVersionChangeMode_eAllFrame,       /* Accept all Frame Header packets */
    NEXUS_TsmfVersionChangeMode_eFrameChangeVer, /* Accept only Frame Header packets that has a change in version number field */
    NEXUS_TsmfVersionChangeMode_eMax
} NEXUS_TsmfVersionChangeMode;

/***************************************************************************
Summary:
Structure for field verification configuration of a TSMF module

Description:
This structure represents configurations for enabling/disabling of certain
field verification of a TSMF module.
***************************************************************************/
typedef struct NEXUS_TsmfFieldVerifyConfig
{
    bool crcCheckDisable;         /* TSMF Frame Header CRC check */
    bool relTsStatusCheckDisable; /* TSMF Frame Header Rel_TS_Status check */
    bool frameTypeCheckDisable;   /* TSMF Frame Header Frame_Type check */
    bool relTsModeCheckDisable;   /* TSMF Frame Header REL_TS_MODE check */
    bool syncCheckDisable;        /* TSMF Frame Header SYNC check */
    bool ccCheckDisable;          /* TSMF Frame Header CC check */
    bool adapCheckDisable;        /* TSMF Frame Header ADAP check */
    bool scCheckDisable;          /* TSMF Frame Header SC check */
    bool tsPriorCheckDisable;     /* TSMF Frame Header TS_PRIOR check */
    bool pusiCheckDisable;        /* TSMF Frame Header PUSI check */
    bool teiCheckDisable;         /* TSMF Frame Header TEI check */
    NEXUS_TsmfVersionChangeMode versionChangeMode;
} NEXUS_TsmfFieldVerifyConfig;

/***************************************************************************
Description:
Used in NEXUS_TsmfSettings to specify the type of input to the TSMF.
***************************************************************************/
typedef enum NEXUS_TsmfSourceType
{
    NEXUS_TsmfSourceType_eInputBand,
    NEXUS_TsmfSourceType_eRemux,
    NEXUS_TsmfSourceType_eMtsif, /* MTSIF-based frontend */
    NEXUS_TsmfSourceType_eMtsifRx, /* Stream from MTSIF, parsed on the backend chip */
    NEXUS_TsmfSourceType_eMax
} NEXUS_TsmfSourceType;

/***************************************************************************
Description:
Settings to control TSMF module in transport and frontend blocks.
***************************************************************************/
typedef struct NEXUS_TsmfSettings
{
    NEXUS_TsmfSourceType sourceType;
    struct {
        NEXUS_InputBand inputBand;
        NEXUS_RemuxHandle remux;
        NEXUS_FrontendConnectorHandle mtsif;
    } sourceTypeSettings;

    bool enabled;

    NEXUS_TsmfFieldVerifyConfig fieldVerifyConfig;
    unsigned relativeTsNum; /* TSMF Relative TS Number */
    bool semiAutomaticMode; /* Set true for semi-automatic mode.
                               If false, it is automatic mode and the following settings params are ignored */

    /* semi-automatic mode settings */
    uint32_t slotMapLo; /* Lower 32 bits (LSBs) of the 52-bit TSMF Slot Map vector */
    uint32_t slotMapHi; /* Upper 20 bits of the 52-bit Slot Map vector for TSMF Demultiplex */
} NEXUS_TsmfSettings;

typedef struct NEXUS_TsmfOpenSettings
{
    /* Required for soft TSMF. Otherwise, these should be left NULL. */
    struct
    {
       bool enabled;                /* Parse TSMF streams in software. If enabled, NEXUS_TsmfType is ignored. */
       unsigned playpumpIndex;      /* Default to NEXUS_ANY_ID. */
       unsigned parserbandIndex;    /* Default to NEXUS_ANY_ID. */
       unsigned recpumpIndex;       /* Default to NEXUS_ANY_ID. */
    } soft;
} NEXUS_TsmfOpenSettings;

/*
Summary:
Get default open settings for the structure
*/
void NEXUS_Tsmf_GetDefaultOpenSettings(
    NEXUS_TsmfOpenSettings *pOpenSettings /* [out] */
    );

typedef enum NEXUS_TsmfType
{
    NEXUS_TsmfType_eBackend,
    NEXUS_TsmfType_eFrontend,
    NEXUS_TsmfType_eMax
} NEXUS_TsmfType;

#define NEXUS_TSMF_INDEX(TYPE,NUMBER) ((TYPE) << 16 | (NUMBER))

/*
Summary:
Open a new TSMF
*/
NEXUS_TsmfHandle NEXUS_Tsmf_Open(   /* attr{destructor=NEXUS_Tsmf_Close} */
    unsigned index,                 /* use NEXUS_TSMF_INDEX(TYPE,NUMBER) */
    const NEXUS_TsmfOpenSettings* pOpenSettings /* attr{null_allowed=y} */
    );

/*
Summary:
Close a TSMF
*/
void NEXUS_Tsmf_Close(
    NEXUS_TsmfHandle tsmf
    );

/*
Summary:
Get current settings
*/
void NEXUS_Tsmf_GetSettings(
    NEXUS_TsmfHandle tsmf, /* which TSMF */
    NEXUS_TsmfSettings *pSettings
    );

/*
Summary:
Set new settings
*/
NEXUS_Error NEXUS_Tsmf_SetSettings(
    NEXUS_TsmfHandle tsmf, /* which TSMF */
    const NEXUS_TsmfSettings *pSettings
    );

/*
Summary:
Open a PID channel to obtain the output of soft TSMF parsing.
*/
NEXUS_PidChannelHandle NEXUS_Tsmf_OpenPidChannel( /* attr{destructor=NEXUS_PidChannel_Close} */
    NEXUS_TsmfHandle tsmf,
    unsigned pid,
    const NEXUS_PlaypumpOpenPidChannelSettings *pSettings /* attr{null_allowed=y} may be NULL for default settings */
    );

#ifdef __cplusplus
}
#endif

#endif

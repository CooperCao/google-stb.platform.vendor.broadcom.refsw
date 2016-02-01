/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
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
* API Description:
*   API name: Frontend 3445
*    APIs to open, close, and setup initial settings for a BCM3445
*    Dual-Channel Integrated Satellite Tuner/Demodulator Device.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_FRONTEND_3445_H__
#define NEXUS_FRONTEND_3445_H__

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
Summary:
   Specifies the configuration of an output channel on the attached LNA

Description:
   Specifies the configuration of an output channel on the attached LNA
******************************************************************************/
typedef enum NEXUS_3445LnaInput
{
    NEXUS_3445LnaInput_eNone = 0, /* output is powered off */
    NEXUS_3445LnaInput_eDaisy,    /* output comes from daisy */
    NEXUS_3445LnaInput_eIn1Vga,   /* output comes from IN1(VGA) */
    NEXUS_3445LnaInput_eIn1Db,    /* output comes from IN1(DB) */
    NEXUS_3445LnaInput_eIn2Vga,   /* output comes from IN2(VGA) */
    NEXUS_3445LnaInput_eMax
} NEXUS_3445LnaInput;

/******************************************************************************
Summary:
   Specifies an output channel on the attached LNA
******************************************************************************/
typedef enum NEXUS_3445LnaOutput
{
    NEXUS_3445LnaOutput_eNone = 0, /* No output */
    NEXUS_3445LnaOutput_eOut1,     /* LNA primary output */
    NEXUS_3445LnaOutput_eOut2,     /* LNA secondary output */
    NEXUS_3445LnaOutput_eDaisy,    /* LNA daisy output */
    NEXUS_3445LnaOutput_eMax
} NEXUS_3445LnaOutput;

/******************************************************************************
Summary:
    LNA output configuration.

Description:
    This structure specifies the configuration of the attached LNA outputs.

For example, this configuration:
              +------------+
        IN1 --+-+-VGA------+-- OUT1
              |  \-DB-\    +-- OUT2
        IN2 --+        \---+-- DAISY
              +------------+

would be represented by:
        NEXUS_3445LnaSettings lnaSettings;
        lnaSettings.out1 = NEXUS_3445LnaInput_eIn1Vga;
        lnaSettings.out2 = NEXUS_3445LnaInput_eNone;
        lnaSettings.daisy = NEXUS_3445LnaInput_eIn1Db;

    If the input for a given output is set to NEXUS_3445LnaInput_eNone, then this LNA
will not be used.

    NEXUS_3445LnaSettings controls an external LNA.
******************************************************************************/
typedef struct NEXUS_3445LnaSettings
{
   NEXUS_3445LnaInput out1;  /* Configure the input linked to OUT1. Set to eNone to not use this LNA. */
   NEXUS_3445LnaInput out2;  /* Configure the input linked to OUT2. Set to eNone to not use this LNA. */
   NEXUS_3445LnaInput daisy; /* Configure the input linked to DAISY. Set to eNone to not use this LNA. */
   bool lowPowerMode;        /* Enable low power mode, which decreases power consumption by 15% */
} NEXUS_3445LnaSettings;

/***************************************************************************
Summary:
    Get the settings of an LNA attached to the 3445 frontend
 ***************************************************************************/
void NEXUS_Frontend_Get3445LnaSettings(
    NEXUS_FrontendHandle handle,
    NEXUS_3445LnaSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Set the settings of an LNA attached to the 3445 frontend

Description:
Do not call this for an external LNA.
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Set3445LnaSettings(
    NEXUS_FrontendHandle handle,
    const NEXUS_3445LnaSettings *pSettings
    );

/***************************************************************************
Summary:
    Status for a BCM3445 LNA Device
 ***************************************************************************/
typedef struct NEXUS_3445LnaStatus
{
    NEXUS_3445LnaInput  lnaInput;
    NEXUS_3445LnaOutput lnaOutput;
    uint8_t             status;      /* see BAST_BCM3445_STATUS_* macros */
    uint8_t             version;     /* BCM3445 version number */
    uint8_t             agc;         /* AGC value read from BCM3445 */
} NEXUS_3445LnaStatus;

/***************************************************************************
Summary:
Get the status of an LNA attached to the 3445 frontend

Description:
Do not call this for an external LNA.
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Get3445LnaStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_3445LnaStatus *pStatus    /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_FRONTEND_3445_H__ */



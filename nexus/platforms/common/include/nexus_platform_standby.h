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
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#ifndef NEXUS_PLATFORM_STANDBY_H__
#define NEXUS_PLATFORM_STANDBY_H__

#include "nexus_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***************************************************************************
Summary:
Standby mode used in NEXUS_PlatformStandbySettings

Description:
See "Power Management" section of nexus/docs/Nexus_Usage.pdf.

When nexus is implementing ePassive and eActive standby modes, our goal is to leave
as much code resident as possible. This allows applications to leave handles open and minimizes
change in SW state.

However, this is not possible in all cases. If a module must be shut down, the application
is responsible for closing handles before calling NEXUS_Platform_SetStandbySettings.
If you do not, an error will be issued to the console and NEXUS_Platform_SetStandbySettings will fail.
***************************************************************************/
typedef enum NEXUS_PlatformStandbyMode
{
    NEXUS_PlatformStandbyMode_eOn,        /* Normal mode of operation.
                         Also known as S0 mode. */
    NEXUS_PlatformStandbyMode_eActive,    /* Frontend and transport modules are running. All other modules are put to sleep.
                         The same wakeup devices as ePassive are available.
                         The application cannot put the MIPS to sleep in this mode.
                         Also known as S1 mode*/
    NEXUS_PlatformStandbyMode_ePassive,   /* Lowest power setting while code remains resident.
                         IrInput, UhfInput, HdmiOutput (CEC), Gpio and Keypad are available to be configured as wakeup devices.
                         Application must call OS to put the MIPS to sleep.
                         Also known as S2 mode. */
    NEXUS_PlatformStandbyMode_eDeepSleep, /* All cores are power gated except for AON block. Achieves minimum power state.
                         Gpio and Keypad are available to be configured as wakeup devices.
                         Application must call OS to put the MIPS to sleep.
                         Also known as S3 mode. */
    NEXUS_PlatformStandbyMode_eMax
} NEXUS_PlatformStandbyMode;

/***************************************************************************
Summary:
Settings used for NEXUS_Platform_SetStandbySettings
***************************************************************************/
typedef struct NEXUS_PlatformStandbySettings
{
    NEXUS_PlatformStandbyMode mode;
    struct {
        bool ir;
        bool uhf;
        bool keypad;
        bool gpio;
        bool nmi;
        bool cec;
        bool transport;
        unsigned timeout; /* in seconds */
    } wakeupSettings;
    bool openFrontend; /* If true, NEXUS_Platform_SetStandbySettings will initialize the frontend after resuming from S3. */
} NEXUS_PlatformStandbySettings;

/***************************************************************************
Summary:
Status returned by NEXUS_Platform_GetStandbyStatus
***************************************************************************/
typedef struct NEXUS_PlatformStandbyStatus
{
    struct {
        bool ir;
        bool uhf;
        bool keypad;
        bool gpio;
        bool nmi;
        bool cec;
        bool transport;
        bool timeout;
    } wakeupStatus;
} NEXUS_PlatformStandbyStatus;

/***************************************************************************
Summary:
    Get the current standby settings.
***************************************************************************/
void NEXUS_Platform_GetStandbySettings(
    NEXUS_PlatformStandbySettings *pSettings  /* [out] */
    );

/***************************************************************************
Summary:
Use this function to put Nexus into and out of standby.

Description:
Note that this function does not put Linux or the MIPS into standby.
See comments for NEXUS_PlatformStandbyMode_ePassive.
***************************************************************************/
NEXUS_Error NEXUS_Platform_SetStandbySettings( /* attr{local=true} */
    const NEXUS_PlatformStandbySettings *pSettings
    );

/***************************************************************************
Summary:
Proxied function for SetStandbySettings
***************************************************************************/
NEXUS_Error NEXUS_Platform_SetStandbySettings_driver(
    const NEXUS_PlatformStandbySettings *pSettings
    );

/***************************************************************************
Summary:
    Get the current standby status.
***************************************************************************/
NEXUS_Error NEXUS_Platform_GetStandbyStatus( /* attr{local=true} */
    NEXUS_PlatformStandbyStatus *pStatus
    );

/***************************************************************************
Summary:
    Proxied function to get the current standby status.
***************************************************************************/
NEXUS_Error NEXUS_Platform_GetStandbyStatus_driver(
    NEXUS_PlatformStandbyStatus *pStatus
    );

/***************************************************************************
Summary:
    This api has been deprecated. It is only meant for backward compatiblity.
    Use NEXUS_Platform_GetStandbySettings to get the current standby settings.
***************************************************************************/
void NEXUS_Platform_GetDefaultStandbySettings(
    NEXUS_PlatformStandbySettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
    This api has been deprecated. It is only meant for backward compatiblity.
    Use NEXUS_Platform_SetStandbySettings to enter stnadby mode.
***************************************************************************/
NEXUS_Error NEXUS_Platform_InitStandby(
    const NEXUS_PlatformStandbySettings *pSettings
    );

/***************************************************************************
Summary:
    This api has been deprecated. It is only meant for backward compatiblity.
***************************************************************************/
void NEXUS_Platform_UninitStandby(void);

/***************************************************************************
Summary:
    This api has been deprecated. It is only meant for backward compatiblity.
***************************************************************************/
NEXUS_Error NEXUS_Platform_PreStandby(void);

/***************************************************************************
Summary:
    This api has been deprecated. It is only meant for backward compatiblity.
***************************************************************************/
NEXUS_Error NEXUS_Platform_PostStandby(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* #ifndef NEXUS_PLATFORM_STANDBY_H__ */


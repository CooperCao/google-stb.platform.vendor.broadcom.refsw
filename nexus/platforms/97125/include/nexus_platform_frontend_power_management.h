/***************************************************************************
*     (c)2004-2011 Broadcom Corporation
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
#ifndef NEXUS_PLATFORM_FRONTEND_POWER_MANAGEMENT_H__
#define NEXUS_PLATFORM_FRONTEND_POWER_MANAGEMENT_H__

#include "nexus_platform.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct NEXUS_PlatformFrontendStandby_Settings 
{
   int NEXUS_PlatformFrontendStandbyVersion;
} NEXUS_PlatformFrontendStandby_Settings ;


/***************************************************************************
Summary:
Platform frontend power management mode used in NEXUS_PlatformStandbySettings

Description:

***************************************************************************/
typedef enum NEXUS_PlatformFrontendStandbyMode
{
    NEXUS_PlatformFrontendStandbyMode_eFullPower,	/*normal power on mode*/
    NEXUS_PlatformFrontendStandbyMode_ePassive, /* IrInput, UhfInput, HdmiOutput (CEC), Gpio and Keypad are available to be configured as wakeup devices. MIPS will be put to sleep by the OS. Lowest power. */
    NEXUS_PlatformFrontendStandbyMode_eActive,  /* frontend and transport modules are running. All wakeup devices for ePassive are available. */
    NEXUS_PlatformFrontendStandbyMode_eMax
} NEXUS_PlatformFrontendStandbyMode;

/***************************************************************************
Summary:
State used for NEXUS_PlatformFrontendPowerManagement
***************************************************************************/
typedef struct NEXUS_PlatformFrontendStandbyState
{
    NEXUS_PlatformFrontendStandbyMode mode;

} NEXUS_PlatformFrontendStandbyState;


/***************************************************************************
Summary:
Set frontend standby settings

Description:

***************************************************************************/
NEXUS_Error NEXUS_Platform_SetFrontendStandbySetting(
	const NEXUS_PlatformFrontendStandby_Settings *pSettings
	);

/***************************************************************************
Summary:
Get frontend standby settings

Description:

***************************************************************************/
NEXUS_Error NEXUS_Platform_GetFrontendStandbySetting(
	NEXUS_PlatformFrontendStandby_Settings *pSettings /* [out] */
	);

/***************************************************************************
Summary:
Get frontend standby default settings

Description:

***************************************************************************/
NEXUS_Error NEXUS_Platform_GetDefaultFrontendStandbySetting(
	NEXUS_PlatformFrontendStandby_Settings *pSettings /* [out] */
	);

/***************************************************************************
Summary:
Get frontend standby mode

Description:

***************************************************************************/
NEXUS_Error NEXUS_Platform_GetFrontendStandbyMode(
	NEXUS_PlatformFrontendStandbyMode *pStandbyMode /* [out] */
	);

/***************************************************************************
Summary:
Wakeup eCM

Description:

***************************************************************************/
NEXUS_Error NEXUS_Platform_WakeupCM(void);

/***************************************************************************
Summary:
Shutdown eCM

Description:

***************************************************************************/
NEXUS_Error NEXUS_Platform_ShutdownCM(void);

/***************************************************************************
Summary:
Read frontend control information from shared memory

Description:

***************************************************************************/
NEXUS_Error NEXUS_Platform_ReadFrontendControlSharedMemory(
    uint32_t *pHostControlledFrontend /* [out] */
    );

/***************************************************************************
Summary:
Write frontend control information to shared memory

Description:

***************************************************************************/
NEXUS_Error NEXUS_Platform_WriteFrontendControlSharedMemory(
    uint32_t hostControlledFrontend
    );

/***************************************************************************
Summary:
Set frontend standby mode

Description:

***************************************************************************/
NEXUS_Error NEXUS_Platform_SetFrontendStandbyMode(
	NEXUS_PlatformFrontendStandbyMode standbyMode
	);


/***************************************************************************
Summary:
Enable host frontend control   

Description:
On the 97125 and 97019 Reference Platforms, the bus BSC_M3 used
for control of external frontends and LNA is controlled by either
host or eCM.  Therefore, either host or eCM controls all frontends.
This functions switches control from eCM to host.

First, the platform's open handles for eCM controlled frontends are all closed.

Then control of BSC_M3 is switched. Finally, if openAllFrontends is true,
the platform's default set of frontends is opened in the default order
as done by NEXUS_Platform_InitFrontend(), (calling NEXUS_Platform_Init(pSettings->openFrontend=true)).

After calling this function, the app must get the new frontend handles using
NEXUS_Platform_OpenFrontend(), NEXUS_Platform_Frontend_GetFrontendInstanceControlled(),
or by calling NEXUS_Platform_GetConfiguration() to get an updated
platform configuration containing the new frontend handles.
If openAllFrontends is false, the app should use NEXUS_Platform_OpenFrontend()
to open the host-controlled frontend(s) in the desired order.

***************************************************************************/
NEXUS_Error NEXUS_Platform_Frontend_EnableHostControl(
	bool openAllFrontends
	);

/***************************************************************************
Summary:
Disable host frontend control

Description:

***************************************************************************/
NEXUS_Error NEXUS_Platform_Frontend_DisableHostControl(
	bool openAllFrontends
	);

/***************************************************************************
Summary:
Get frontend to be controlled after Standby/ON status switched

Description:
On the 97125 and 97019 Reference Platforms, the bus BSC_M3 used
for control of external frontends and LNA is controlled by either
host or eCM.  Therefore, either host or eCM controls all frontends.
This functions switches control from host to eCM.

First, the platform's open handles for host controlled frontends are all closed.

Then control of BSC_M3 is switched. Finally, if openAllFrontends is true,
the platform's default set of frontends is opened in the default order
as done by NEXUS_Platform_InitFrontend(), (calling NEXUS_Platform_Init(pSettings->openFrontend=true)).

After calling this function, the app must get the new frontend handles using
NEXUS_Platform_OpenFrontend(), NEXUS_Platform_Frontend_GetFrontendInstanceControlled(),
or by calling NEXUS_Platform_GetConfiguration() to get an updated
platform configuration containing the new frontend handles.
If openAllFrontends is false, the app should use NEXUS_Platform_OpenFrontend()
to open the eCM-controlled frontend(s) in the desired order.

***************************************************************************/
NEXUS_Error NEXUS_Platform_Frontend_GetFrontendInstanceControlled(
	NEXUS_FrontendHandle * pControlledFrontend, /* [out] */
	int index
	);

/***************************************************************************
Summary:
Power up the LNA

Description:

***************************************************************************/
NEXUS_Error NEXUS_Platform_PowerUpLNA(void);

/***************************************************************************
Summary:
Power down the LNA

Description:

***************************************************************************/
NEXUS_Error NEXUS_Platform_PowerDownLNA(void);

/***************************************************************************
Summary:
Get the frontend type (chip number).

Description:

***************************************************************************/
NEXUS_Error NEXUS_Platform_FrontendType(
	NEXUS_FrontendHandle handle,
    uint32_t *pFrontendType /* [out] */
	);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* #ifndef NEXUS_PLATFORM_FRONTEND_POWER_MANAGEMENT_H__ */


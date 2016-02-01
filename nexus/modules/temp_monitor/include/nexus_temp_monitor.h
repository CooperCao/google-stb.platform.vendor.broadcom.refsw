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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef NEXUS_TEMP_MONITOR_H__
#define NEXUS_TEMP_MONITOR_H__

#include "nexus_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NEXUS_TempMonitor *NEXUS_TempMonitorHandle;

/***************************************************************************
Summary:
Temperature Monitor Settings
***************************************************************************/
typedef struct NEXUS_TempMonitorSettings
{
    bool enabled;
    NEXUS_CallbackDesc dataReady;   /* data ready user callback */
    unsigned measureInterval;       /* measurement interval (units in milliseconds) */
} NEXUS_TempMonitorSettings;

/**************************************************************************
Summary:
Nexus Temperature Monitor Device Status

Description:
Each NEXUS_TempMonitor controls a separate thermistor.
Depending on the wiring of the thermistors, some temp monitors may share the same values.

For instance, current silicon has thermistors wired in parallel, which means that they
share the same top and bottom values, but have different middle values.
***************************************************************************/
typedef struct NEXUS_TempMonitorStatus
{
    bool valid; /* if false, ignore other values in this status. app must wait for dataReady callback. */
    uint32_t top;    /* low-level register. conversion to actual temperature must be done by the application. 
                        See Broadcom document "Temperature Monitor Application Note", 7340-AN200-R,
                        available in docsafe. */
    uint32_t middle; /* see notes above */
    uint32_t bottom; /* see notes above */

    int onChipTemperature; /* in degrees celcius */
} NEXUS_TempMonitorStatus;


/***************************************************************************
Summary:
Get default open-time settings of Temperature Monitor Device.
***************************************************************************/
void NEXUS_TempMonitor_GetDefaultSettings(
    NEXUS_TempMonitorSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Open a Temperature Monitor Device
***************************************************************************/
NEXUS_TempMonitorHandle NEXUS_TempMonitor_Open(  /* attr{destructor=NEXUS_TempMonitor_Close} */
    unsigned index,
    const NEXUS_TempMonitorSettings *pSettings  /* attr{null_allowed=y} */
    );

/***************************************************************************
Summary:
Close a Temperature Monitor Device
***************************************************************************/
void NEXUS_TempMonitor_Close(
    NEXUS_TempMonitorHandle handle
    );

/***************************************************************************
Summary:
Get the status from the Temperature Monitor Device
***************************************************************************/
NEXUS_Error NEXUS_TempMonitor_GetStatus(
    NEXUS_TempMonitorHandle handle,
    NEXUS_TempMonitorStatus *pStatus /* [out] */
    );

/***************************************************************************
Summary:
Get current settings.
***************************************************************************/
void NEXUS_TempMonitor_GetSettings(
    NEXUS_TempMonitorHandle handle,
    NEXUS_TempMonitorSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Apply new settings.
***************************************************************************/
NEXUS_Error NEXUS_TempMonitor_SetSettings(
    NEXUS_TempMonitorHandle handle,
    const NEXUS_TempMonitorSettings *pSettings
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_TEMP_MONITOR_H__ */


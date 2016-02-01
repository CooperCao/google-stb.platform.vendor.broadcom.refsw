/***************************************************************************
*     (c)2007-2010 Broadcom Corporation
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

#ifndef BCHP_PWR_H__
#define BCHP_PWR_H__

#include "bchp.h"
#include "bchp_priv.h"
#ifdef BCHP_PWR_HAS_RESOURCES
#include "bchp_pwr_resources.h" /* platform-specific resources file */
#endif

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
	Power resource identifier.

Description:
	Power resources are chip-specific abstractions that are defined in
	bchp_pwr_resources.h. A power resource is identified by a single
	uint32_t number.

**************************************************************************/
typedef uint32_t BCHP_PWR_ResourceId;


/***************************************************************************
Summary:
	Acquire a power resource.

Description:
	This function acquires a single power resource.  You can only acquire
	high-level, non-HW power resources.

	Acquiring a power resource causes the internal reference count for
	the resource and all its successive dependants to increase by one.
	The dependent HW clocks turn on only when the internal reference
	count is increased from 0 to 1.

	The same power resource can be acquired multiple times.

See Also:
	BCHP_PWR_Resource_Release
**************************************************************************/
BERR_Code BCHP_PWR_AcquireResource(BCHP_Handle handle, BCHP_PWR_ResourceId resourceId);


/***************************************************************************
Summary:
	Release a power resource.

Description:
	This function releases a single power resource. You can only release
	high-level, non-HW power resources. Furthermore, you can only release
	power resources that were previously explicitly acquired.

	Releasing a power resource causes the internal reference count for
	the resource and all its successive dependants to decrease by one.
	The dependent HW clocks turn off only when the internal reference
	count becomes zero.

	The same power resource can be released as many times as it was
	previously acquired.

See Also:
	BCHP_PWR_Resource_Acquire
**************************************************************************/
BERR_Code BCHP_PWR_ReleaseResource(BCHP_Handle handle, BCHP_PWR_ResourceId resourceId);


/***************************************************************************
Summary:
	Initialize all HW power resources so that they can be acquired and released.

Description:
	A HW resource must be initialized before it is allowed to be acquired and
	released. An initialized HW resource is one that is powered down and has
	a reference count of 0.

	During BCHP_PWR_Open, HW resources associated with the special
	BCHP_PWR_RESOURCE_MAGNUM_CONTROLLED node are automatically
	initialized. Calling this function initializes the rest.

**************************************************************************/
void BCHP_PWR_InitAllHwResources(BCHP_Handle handle);


/***************************************************************************
Summary:
	Print current power status.

Description:
	This function prints which power resources are powered up and down,
	and their current reference counts. This is used to debug power usage
	across the system, and is functionally equivalent to looking for
	memory leaks.

	When a power resource is acquired, the "public" reference count is
	increased only for that particular resource. The "private" reference
	count, on the other hand, is increased for all its successive
	dependants.

**************************************************************************/
void BCHP_PWR_Dump(BCHP_Handle handle);

/***************************************************************************
Summary:
    Print status of top level nodes.

Description:
	This function prints which top level nodes are powered up and down. It does
    no print as much detail as the Dump function.
**************************************************************************/
void BCHP_PWR_DebugPrint(BCHP_Handle handle);

/***************************************************************************
Summary:
	Standby settings.

Description:
	Currently unused.
**************************************************************************/
typedef struct BCHP_PWR_StandbySettings {
    unsigned unused; /* dummy placeholder */
} BCHP_PWR_StandbySettings;


/***************************************************************************
Summary:
	Enter standby.

Description:
	This function causes the BCHP_PWR sub-module to enter standby.
	Currently, this function is a no-op; it does not change any SW
	reference counts or HW state.
**************************************************************************/
BERR_Code BCHP_PWR_Standby(
    BCHP_Handle handle,
    const BCHP_PWR_StandbySettings *pSettings /* optional */
    );


/***************************************************************************
Summary:
	Exit standby.

Description:
	This function causes BCHP_PWR to exit standby by re-applying the HW state.

	A power management operation such as passive standby can cause the HW
	to lose state, while the SW state remains unchanged. This results in a
	mismatch between the SW reference counts and the HW state.

	Calling this function causes BCHP_PWR to re-apply the HW state based on
	the current SW reference counts, thereby synchronizing the two states
	again.
**************************************************************************/
void BCHP_PWR_Resume(
    BCHP_Handle handle
    );

/***************************************************************************
Summary:
	Get Minimum Clock Rate

Description:
	This function is used to get the minimum clock rate for a given
    PWR resource id. Not all resources will have a default clock rate.
**************************************************************************/
BERR_Code BCHP_PWR_GetMinClockRate(
        BCHP_Handle handle,
        BCHP_PWR_ResourceId resourceId,
        unsigned *clkRate
        );

/***************************************************************************
Summary:
	Get Maximum Clock Rate

Description:
	This function is used to get the maximum clock rate for a given
    PWR resource id. Not all resources will have a default clock rate.
**************************************************************************/
BERR_Code BCHP_PWR_GetMaxClockRate(
        BCHP_Handle handle,
        BCHP_PWR_ResourceId resourceId,
        unsigned *clkRate
        );

/***************************************************************************
Summary:
	Get Default Clock Rate

Description:
	This function is used to get the default clock rate for a given
    PWR resource id. Not all resources will have a default clock rate.
**************************************************************************/
BERR_Code BCHP_PWR_GetDefaultClockRate(
        BCHP_Handle handle,
        BCHP_PWR_ResourceId resourceId,
        unsigned *clkRate
        );

/***************************************************************************
Summary:
	Get Current Clock Rate

Description:
	This function is used to get the current clock rate for a given
    PWR resource. Not all resources have the capability to report the current
    clock rate. Check for return code to determine whether a get was successful
**************************************************************************/
BERR_Code BCHP_PWR_GetClockRate(
        BCHP_Handle handle,
        BCHP_PWR_ResourceId resourceId,
        unsigned *clkRate
        );

/***************************************************************************
Summary:
	Set Clock Rate

Description:
	This function is used to set the clock rate for a given PWR resource.
    Not all resources have the capability to get/set clock rate. Check the
    return code whether rate was set successfully or not.
**************************************************************************/
BERR_Code BCHP_PWR_SetClockRate(
        BCHP_Handle handle,
        BCHP_PWR_ResourceId resourceId,
        unsigned clkRate
        );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

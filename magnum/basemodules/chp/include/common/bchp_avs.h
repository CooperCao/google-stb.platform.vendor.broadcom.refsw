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
#ifndef BCHP_AVS_H__
#define BCHP_AVS_H__

#include "bstd.h"
#include "bchp.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    The handle for AVS features.

Description:
    An opaque handle for AVS features.
    This is returned on a successful Open call and is passed into the
    Monitor and Close calls as input parameter.

See Also:
    BCHP_P_AvsOpen(), BCHP_P_AvsClose(), BCHP_P_AvsMonitorPvt.

****************************************************************************/
typedef struct BCHP_P_AvsContext *BCHP_P_AvsHandle;


/***************************************************************************
Summary:
    Open a chip handle for AVS processing.

Description:
    This function opens a chip handle for AVS processing.

Input:
    hRegister - A valid chip register handle previous open using BREG_Open.

Output:
    phHandle - Upon successful completion of open this pointer is non NULL
    and contains valid information about this chip.  This handle is used
    on subsequences BCHP_??? API.  *phHandle is NULL if failure.

See Also:
    BCHP_P_AvsClose, BCHP_P_AvsMonitorPvt

**************************************************************************/
BERR_Code BCHP_P_AvsOpen (
    BCHP_P_AvsHandle *phHandle,  /* [out] returns new handle on success */
    BCHP_Handle       hChip);    /* [in] handle for chip data */

/***************************************************************************
Summary:
    Close a AVS chip handle.

Description:
    This function closes a chip handle for AVS processing.

Input:
    hHandle - The handle supplied by a successful BCHP_AvsOpen call.

See Also:
    BCHP_P_AvsOpen

**************************************************************************/
BERR_Code BCHP_P_AvsClose (
    BCHP_P_AvsHandle hHandle ); /* [in] handle supplied from open */

/***************************************************************************
Summary:
    Process AVS data.

Description:
    This function is the periodic processing function for the AVS.
    This should be called on a timely basis (every second) to ensure processing gets done.

Input:
    hHandle - The handle supplied by a successful BCHP_AvsOpen call.

See Also:
    BCHP_P_AvsOpen

**************************************************************************/
BERR_Code BCHP_P_AvsMonitorPvt ( 
    BCHP_P_AvsHandle hHandle); /* [in] handle supplied from open */

/***************************************************************************
Summary:
    Return status data for AVS.

Description:
    This can be used to get the current voltage and temperature of the part.

Input:
    hHandle - The handle supplied by a successful BCHP_P_AvsOpen call.
    pData - is pointer to location to return the data.

See Also:
    BCHP_P_AvsOpen, BCHP_P_AvsMonitorPvt

**************************************************************************/
BERR_Code BCHP_P_GetAvsData_isrsafe(
    BCHP_P_AvsHandle hHandle, /* [in] handle supplied from open */
    BCHP_AvsData *pData);     /* [out] location to put data */

/***************************************************************************
Summary:
    Enter/exit stand-by mode.

Description:
    This can be used to have AVS hardware enter stand-by (low power) mode.
    Once entered, calls to BCHP_P_AvsMonitorPvt can be used but will be non-functional.

Input:
    hHandle - The handle supplied by a successful BCHP_P_AvsOpen call.
    activate - is set to true to enter stand-by mode, false returns from stand-by.

See Also:
    BCHP_P_AvsOpen, BCHP_P_AvsMonitorPvt

**************************************************************************/
BERR_Code BCHP_P_AvsStandbyMode(
    BCHP_P_AvsHandle hHandle, /* [in] handle supplied from open */
    bool activate);           /* [in] true to enter low power mode */

#ifdef __cplusplus
}
#endif

#endif /* BCHP_AVS_H__ */


/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
******************************************************************************/
#ifndef BTNR_LEAP_H__
#define BTNR_LEAP_H__

#include "bchp.h"
#include "breg_i2c.h"
#include "bhab.h"
#include "btnr.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    unsigned i2cAddr; /* 7bit I2C address of Bcmleap */
    unsigned int channelNo; /* Channel number to tune to */
    unsigned int mxChnNo;   /* Max tuner channels */
} BTNR_75525Ib_Settings;

/***************************************************************************
Summary:
    This function returns the default settings for Bcm34xx Tuner module.

Description:
    This function is responsible for returns the default setting for
    BTNR module. The returning default setting should be when
    opening the device.

Returns:
    TODO:

See Also:
    BTNR_leap_Open()

****************************************************************************/
BERR_Code BTNR_75525Ib_GetDefaultSettings(
    BTNR_75525Ib_Settings *pDefSettings  /* [out] Returns default setting */
    );


/***************************************************************************
Summary:
    This function opens Bcm34xx Tuner module.

Description:
    This function is responsible for opening Bcm34xx BTNR module. When BTNR
    is opened, it will create a module handle and configure the module based
    on the default settings. Once the device is opened, it must be closed
    before it can be opened again.

Returns:
    TODO:

See Also:
    BTNR_Close(), BTNR_34xx_GetDefaultSettings()

****************************************************************************/
BERR_Code BTNR_75525Ib_Open(
    BTNR_Handle *phDev,                 /* [out] Returns handle */
    BTNR_75525Ib_Settings *pSettings,
    BHAB_Handle hHab
    );

#ifdef __cplusplus
}
#endif

#endif

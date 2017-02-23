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

#include "timebase.h"

BDBG_MODULE(atlas_timebase);

CTimebase::CTimebase(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    CResource(name, number, eBoardResource_timebase, pCfg),
    _timebase(NEXUS_Timebase_e0)
{
    _index = NEXUS_NUM_VIDEO_DECODERS + NEXUS_NUM_VIDEO_ENCODERS + number;
    BDBG_MSG((" Timebase index %d ", _index));
}

CTimebase::~CTimebase()
{
    BDBG_MSG((" DELETE Timebase index %d ", _index));
}

void CTimebase::getSettings(NEXUS_TimebaseSettings * timebaseSettings)
{
    NEXUS_Timebase_GetSettings(_timebase, timebaseSettings);
}

void CTimebase::setSettings(NEXUS_TimebaseSettings * timebaseSettings)
{
    NEXUS_Error errCode;

    BDBG_MSG(("set Settings Timebase index %d", _index));
    errCode = NEXUS_Timebase_SetSettings(_timebase, timebaseSettings);
    (void)CHECK_NEXUS_ERROR("NEXUS_Timebase_SetSettings failed", errCode);
}

eRet CTimebase::open()
{
    eRet ret = eRet_Ok;

    if (ATLAS_NUM_TIMEBASES < _number)
    {
        BDBG_ERR((" Not enough Timebase channels only %d alowed. Number %d requested", ATLAS_NUM_TIMEBASES, _number));
        ret = eRet_ExternalError;
        goto error;
    }

    /* Must do a calculation to use the timebases that are available and not collide with Video or Encoder timebases! */
    _timebase = NEXUS_Timebase_Open(_index);

error:
    return(ret);
} /* open */

void CTimebase::close()
{
    BDBG_MSG(("Closing Timebase index %d", _index));

    /* We really don't know if this timebase is a pointer or set to 0 because of Nexus backwards compatibility */
    NEXUS_Timebase_Close(_timebase);
}

void CTimebase::dump()
{
}
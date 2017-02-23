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

#include "resource.h"

BDBG_MODULE(atlas_resource);

CResource::CResource(
        const char *         name,
        const uint16_t       number,
        const eBoardResource type,
        CConfiguration *     pCfg
        ) :
    CMvcModel(name),
    _name(name),
    _number(number),
    _type(type),
    _pCfg(pCfg),
    _checkedOut(0),
    _checkedOutMax(1),
    _reserveId(NULL),
    _checkedOutId(NULL),
    _bInit(false)
{
}

CResource::~CResource()
{
}

void CResource::dump()
{
    BDBG_MSG(("<%d>%s:%d", _type, _name.s(), _number));
}

bool CResource::isOutput()
{
    bool isAnOutput = false;

    switch (_type)
    {
    case eBoardResource_outputSpdif:
    case eBoardResource_outputAudioDac:
    case eBoardResource_outputAudioDacI2s:
    case eBoardResource_outputAudioDummy:
    case eBoardResource_outputComponent:
    case eBoardResource_outputSVideo:
    case eBoardResource_outputComposite:
    case eBoardResource_outputRFM:
    case eBoardResource_outputHdmi:
        isAnOutput = true;
        break;
    default:
        isAnOutput = false;
        break;
    } /* switch */

    return(isAnOutput);
} /* isOutput */

eRet CResource::setCheckedOut(
        bool   checkedOut,
        void * id
        )
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT((false == checkedOut) || (NULL != id));

    if ((true == checkedOut) && (false == isInitialized()))
    {
        ret = initialize();
        CHECK_ERROR_GOTO("unable to initialize resource on checkout", ret, error);
    }

    if (true == checkedOut)
    {
        if (_checkedOutMax <= _checkedOut)
        {
            CHECK_ERROR_GOTO("unable to checkout - max checkouts reached", eRet_NotAvailable, error);
        }

        if ((0 == _checkedOut) || (id == _checkedOutId))
        {
            _checkedOut++;
            BDBG_MSG(("CHECK OUT %s num:%d timesCheckedOut:%d id:%p", getName(), getNumber(), _checkedOut, _checkedOutId));
            _checkedOutId = id;
        }
    }
    else /* check in */
    {
        if (0 < _checkedOut)
        {
            _checkedOut--;
            BDBG_MSG(("CHECK IN %s num:%d timesCheckedOut:%d id:%p", getName(), getNumber(), _checkedOut, _checkedOutId));
            if (0 == _checkedOut)
            {
                _checkedOutId = NULL;
            }
        }
    }

error:
    return(ret);
} /* setCheckedOut */

bool CResource::isVideoOutput()
{
    bool isAnOutput = false;

    switch (_type)
    {
    case eBoardResource_outputComponent:
    case eBoardResource_outputSVideo:
    case eBoardResource_outputComposite:
    case eBoardResource_outputRFM:
    case eBoardResource_outputHdmi:
        isAnOutput = true;
        break;
    default:
        isAnOutput = false;
        break;
    } /* switch */

    return(isAnOutput);
} /* isVideoOutput */

bool CResource::isAudioOutput()
{
    bool isAnOutput = false;

    switch (_type)
    {
    case eBoardResource_outputSpdif:
    case eBoardResource_outputAudioDac:
    case eBoardResource_outputAudioDacI2s:
    case eBoardResource_outputAudioDummy:
    case eBoardResource_outputHdmi:
        isAnOutput = true;
        break;
    default:
        isAnOutput = false;
        break;
    } /* switch */

    return(isAnOutput);
} /* isAudioOutput */

#if NEXUS_HAS_FRONTEND
bool CResource::isFrontend()
{
    bool isAFrontend = false;

    switch (_type)
    {
    case eBoardResource_frontendQam:
    case eBoardResource_frontendVsb:
    case eBoardResource_frontendSds:
    case eBoardResource_frontendIp:
    case eBoardResource_frontendOfdm:
#ifdef MPOD_SUPPORT
    case eBoardResource_frontendOob:
#endif
        isAFrontend = true;
        break;
    default:
        isAFrontend = false;
        break;
    } /* switch */

    return(isAFrontend);
} /* isFrontend */

#endif /* if NEXUS_HAS_FRONTEND */
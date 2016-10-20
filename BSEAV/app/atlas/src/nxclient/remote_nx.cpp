/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "atlas.h"
#include "atlas_os.h"
#include "remote_nx.h"

#include "nexus_input_client.h"

#define CALLBACK_IRREMOTE   "CallbackIrRemote"
#define CALLBACK_UHFREMOTE  "CallbackUhfRemote"

BDBG_MODULE(atlas_remote);

static void bwinRemoteCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CRemote *    pRemote = (CRemote *)pObject;
    CRemoteEvent event;

    BDBG_ASSERT(NULL != pRemote);
    BSTD_UNUSED(strCallback);

    pRemote->remoteCallback();
} /* bwinRemoteCallback */

static void nexusIrCallbackNx(
        void * context,
        int    param
        )
{
    eRet          ret      = eRet_Ok;
    CIrRemoteNx * irRemote = (CIrRemoteNx *)context;
    CRemoteEvent  event;

    BSTD_UNUSED(param);
    BDBG_ASSERT(NULL != irRemote);
    BDBG_ASSERT(eBoardResource_irRemote == irRemote->getType());

    ret = irRemote->getEvent(&event);
    if (eRet_Ok == ret)
    {
        /* skip repeats */
        if (false == event.isRepeat())
        {
            CWidgetEngine * pWidgetEngine = irRemote->getWidgetEngine();

            if (NULL != pWidgetEngine)
            {
                /* save event for later processing */
                irRemote->addEvent(&event);

                /* interrupt the bwin event loop. this will interrupt the sleep and cause the ui to be more responsive. */
                pWidgetEngine->syncCallback(irRemote, CALLBACK_IRREMOTE);
            }
        }
    }
} /* nexusIrCallback */

CIrRemoteNx::CIrRemoteNx(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    CIrRemote(name, number, pCfg),
    _inputClient(NULL),
    _mode(NEXUS_IrInputMode_eMax)
{
    /* verify key mapping size matches */
    BDBG_ASSERT(eKey_Max == sizeof(eKey2bwidgets)/sizeof(bwidget_key));
}

eRet CIrRemoteNx::open(CWidgetEngine * pWidgetEngine)
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != pWidgetEngine);

    _pWidgetEngine = pWidgetEngine;

    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->addCallback(this, CALLBACK_IRREMOTE, bwinRemoteCallback);
    }

    _inputClient = NEXUS_InputClient_Acquire(getNumber());
    CHECK_PTR_ERROR_GOTO("unable to acquire input client", _inputClient, ret, eRet_NotAvailable, error);
    {
        NEXUS_InputClientSettings settings;

        NEXUS_InputClient_GetSettings(_inputClient, &settings);
        settings.filterMask             = 0xFFFFFFFF; /* everything */
        settings.filterMask             = (1<<NEXUS_InputRouterDevice_eIrInput);
        settings.codeAvailable.callback = nexusIrCallbackNx;
        settings.codeAvailable.context  = this;
        NEXUS_InputClient_SetSettings(_inputClient, &settings);
    }

error:
    return(ret);
} /* open */

void CIrRemoteNx::close()
{
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_IRREMOTE);
        _pWidgetEngine = NULL;
    }

    if (NULL != _inputClient)
    {
        NEXUS_InputClient_Release(_inputClient);
        _inputClient = NULL;
    }
}

CIrRemoteNx::~CIrRemoteNx()
{
    close();
}

eRet CIrRemoteNx::setMode(NEXUS_IrInputMode mode)
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NEXUS_IrInputMode_eMax > mode);
    _mode = mode;

    return(ret);
}

NEXUS_IrInputMode CIrRemoteNx::getMode()
{
    BDBG_ASSERT(NULL != _inputClient);
    return(_mode);
}

void CIrRemoteNx::dump()
{
    BDBG_MSG(("<%d>%s:%d",
              _type,
              _name.s(),
              _number));
}

eRet CIrRemoteNx::getEvent(CRemoteEvent * pEvent)
{
    eRet                  ret    = eRet_NotAvailable;
    NEXUS_Error           nerror = NEXUS_SUCCESS;
    NEXUS_InputRouterCode code;
    unsigned              num;

    BDBG_ASSERT(NULL != pEvent);

    nerror = NEXUS_InputClient_GetCodes(getIrRemote(), &code, 1, &num);
    if (!nerror && num)
    {
        eKey key = eKey_Invalid;
        BDBG_MSG(("ir event: %#x repeat:%d", code.data.irInput.code, code.data.irInput.repeat));
        pEvent->setRepeat(code.data.irInput.repeat);

        switch (code.deviceType)
        {
        case NEXUS_InputRouterDevice_eIrInput:
            key = convertRemoteCode(getMode(), code.data.irInput.code);
            break;
        case NEXUS_InputRouterDevice_eEvdev:
        /* todo keyboard support */
        default:
            break;
        } /* switch */

        if ((eKey_Invalid != key) && (eKey_Max != key))
        {
            pEvent->setCode(key);
            BDBG_MSG(("key:%s", getRemoteCodeName(key)));
        }
        else
        {
            BDBG_WRN(("unhandled remote code 0x%0x", code.data.irInput.code));
        }

        ret = eRet_Ok;
    }

    return(ret);
} /* getEvent */

#if NEXUS_HAS_UHF_INPUT

static void nexusUhfCallbackNx(
        void * context,
        int    param
        )
{
    eRet           ret       = eRet_Ok;
    CUhfRemoteNx * uhfRemote = (CUhfRemoteNx *)context;
    CRemoteEvent   event;

    BSTD_UNUSED(param);
    BDBG_ASSERT(NULL != uhfRemote);
    BDBG_ASSERT(eBoardResource_uhfRemote == uhfRemote->getType());

    ret = uhfRemote->getEvent(&event);
    if (eRet_Ok == ret)
    {
        /* skip repeats */
        if (false == event.isRepeat())
        {
            CWidgetEngine * pWidgetEngine = uhfRemote->getWidgetEngine();

            if (NULL != pWidgetEngine)
            {
                /* save event for later processing */
                uhfRemote->addEvent(&event);

                /* interrupt the bwin event loop. this will interrupt the sleep and cause the ui to be more responsive. */
                pWidgetEngine->syncCallback(uhfRemote, CALLBACK_UHFREMOTE);
            }
        }
    }
} /* nexusUhfCallback */

CUhfRemoteNx::CUhfRemoteNx(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    CUhfRemote(name, number, pCfg),
    _inputClient(NULL)
{
    /* verify key mapping size matches */
    BDBG_ASSERT(eKey_Max == sizeof(eKey2bwidgets)/sizeof(bwidget_key));
}

eRet CUhfRemoteNx::open(CWidgetEngine * pWidgetEngine)
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != pWidgetEngine);

    _pWidgetEngine = pWidgetEngine;

    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->addCallback(this, CALLBACK_UHFREMOTE, bwinRemoteCallback);
    }

    _inputClient = NEXUS_InputClient_Acquire(getNumber());
    CHECK_PTR_ERROR_GOTO("unable to acquire input client", _inputClient, ret, eRet_NotAvailable, error);
    {
        NEXUS_InputClientSettings settings;

        NEXUS_InputClient_GetSettings(_inputClient, &settings);
        settings.filterMask             = 0xFFFFFFFF; /* everything */
        settings.filterMask             = (1<<NEXUS_InputRouterDevice_eUhfInput);
        settings.codeAvailable.callback = nexusUhfCallbackNx;
        settings.codeAvailable.context  = this;
        NEXUS_InputClient_SetSettings(_inputClient, &settings);
    }

error:
    return(ret);
} /* open */

void CUhfRemoteNx::close()
{
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_UHFREMOTE);
        _pWidgetEngine = NULL;
    }

    if (NULL != _inputClient)
    {
        NEXUS_InputClient_Release(_inputClient);
        _inputClient = NULL;
    }
}

CUhfRemoteNx::~CUhfRemoteNx()
{
    close();
}

void CUhfRemoteNx::dump()
{
    BDBG_MSG(("<%d>%s:%d",
              _type,
              _name.s(),
              _number));
}

eRet CUhfRemoteNx::getEvent(CRemoteEvent * pEvent)
{
    eRet                  ret    = eRet_NotAvailable;
    NEXUS_Error           nerror = NEXUS_SUCCESS;
    NEXUS_InputRouterCode code;
    unsigned              num;

    BDBG_ASSERT(NULL != pEvent);

    nerror = NEXUS_InputClient_GetCodes(getUhfRemote(), &code, 1, &num);
    if (!nerror && num)
    {
        eKey key = eKey_Invalid;
        BDBG_MSG(("Uhf event: %#x", code.data.uhfInput.tbd));

        switch (code.deviceType)
        {
        case NEXUS_InputRouterDevice_eUhfInput:
            if (!code.data.uhfInput.tbd)
            {
                key = convertRemoteCode(code.data.uhfInput.tbd);
            }
            break;
        case NEXUS_InputRouterDevice_eEvdev:
        /* todo keyboard support */
        default:
            break;
        } /* switch */

        if ((eKey_Invalid != key) && (eKey_Max != key))
        {
            pEvent->setCode(key);
            BDBG_MSG(("key:%s", getRemoteCodeName(key)));
        }
        else
        {
            BDBG_WRN(("unhandled remote code 0x%0x", code.data.uhfInput.tbd));
        }

        ret = eRet_Ok;
    }

    return(ret);
} /* getEvent */

#endif /* if NEXUS_HAS_UHF_INPUT */
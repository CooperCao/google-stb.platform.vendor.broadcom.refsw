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
#include "remote.h"
#include "power.h"
#ifdef NETAPP_SUPPORT
#include <sstream>
#endif

#ifdef RF4CE_SUPPORT
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <iomanip>
using namespace std;
#ifdef MIN
#undef MIN
#endif
#ifdef MAX
#undef MAX
#endif
extern "C" {
#include "bbMailAPI.h"
#include "zigbee_api.h"
#include "zigbee.h"
}
#endif /* ifdef RF4CE_SUPPORT */
#ifdef NETAPP_SUPPORT
#define bclose(fd)  close(fd);
#endif
#define CALLBACK_IRREMOTE         "CallbackIrRemote"
#define CALLBACK_RF4CEREMOTE      "CallbackRf4ceRemote"
#define CALLBACK_UHFREMOTE        "CallbackUhfRemote"
#define CALLBACK_BLUETOOTHREMOTE  "CallbackBluetoothRemote"

BDBG_MODULE(atlas_remote);

#ifdef RF4CE_SUPPORT
static void * rf4ce_context;

typedef struct {
    char   code;
    bool   repeat;
} rf4ce_event;

static rf4ce_event rf4ceEvent;

extern "C" { /* __cplusplus */
#define SYS_DBG_LOG_BUFFER_SIZE  256
#define START_NWK_THREAD

#ifdef START_NWK_THREAD
static pthread_t        start_nwk_thread;
static BKNI_EventHandle start_nwk_event;
#endif

#  define HAL_DbgLogStr(message)  TEST_DbgLogStr(message)

void TEST_DbgLogStr(const char * const message)
{
    printf(message);
    fflush(stdout);
}

uint32_t TEST_DbgAssert(
        uint32_t     errorUid,
        const char * fileName,
        uint16_t     line
        )
{
    char message[200];

    snprintf(message, sizeof(message), "Not expected Assert(%#010x) has been called. \"%s\", L%d", errorUid, fileName, line);
    printf(message);
    return(0);
}

/*************************************************************************************//**
 * \brief Logs error and proceeds with program execution.
 * \param[in] errorUid - Error identifier corresponding to the mismatched
 *      expression being asserted.
 *****************************************************************************************/
uint32_t HAL_DbgLogId(
        uint32_t     errorUid,
        const char * fileName,
        uint16_t     line
        )
{
    TEST_DbgAssert(errorUid, fileName, line);
    return(0);
}

uint32_t HAL_DbgHalt(
        uint32_t     errorUid,
        const char * fileName,
        uint16_t     line
        )
{
    HAL_DbgLogId(errorUid, fileName, line);
    return(0);
}

void sysDbgHalt(
        const uint32_t errorUid /* , const char *const fileName, const uint32_t fileLine ) */
# if defined (             _DEBUG_FILELINE_)
        ,
        const char * const fileName,
        const uint32_t     fileLine
# endif
        )
{
    HAL_IRQ_DISABLE();
# if defined (_DEBUG_FILELINE_)
#  if (_DEBUG_CONSOLELOG_ >= 1)
    sysDbgLogStr("HALT: %s(%d) - 0x%08X", fileName, fileLine, errorUid);
#  endif
#  if (_DEBUG_HARNESSLOG_ >= 1)
    HAL_DbgHalt(errorUid, fileName, fileLine);
#  endif

# else /* ! _DEBUG_FILELINE_ */
#  if (_DEBUG_CONSOLELOG_ >= 1)
    sysDbgLogStr("HALT: 0x%08X", errorUid);
#  endif
#  if (_DEBUG_HARNESSLOG_ >= 1)
    HAL_DbgHalt(errorUid);
#  endif

# endif /* ! _DEBUG_FILELINE_ */

    while (1)
    {
    }
}   /* sysDbgHalt */

#if defined (_DEBUG_LOG_)
/*
 * Logs warning and proceeds with program execution.
 */
void sysDbgLogId(
        const uint32_t warningUid /* , const char *const fileName, const uint32_t fileLine ) */
# if defined (             _DEBUG_FILELINE_)
        ,
        const char * const fileName,
        const uint32_t     fileLine
# endif
        )
{
# if defined (_DEBUG_FILELINE_)
#  if (_DEBUG_CONSOLELOG_ >= 2)
    sysDbgLogStr("WARN: %s(%d) - 0x%08X", fileName, fileLine, warningUid);
#  endif
#  if (_DEBUG_HARNESSLOG_ >= 2)
    HAL_DbgLogId(warningUid, fileName, fileLine);
#  endif

# else /* ! _DEBUG_FILELINE_ */
#  if (_DEBUG_CONSOLELOG_ >= 2)
    sysDbgLogStr("WARN: 0x%08X", warningUid);
#  endif
#  if (_DEBUG_HARNESSLOG_ >= 2)
    HAL_DbgLogId(warningUid);
#  endif

# endif /* ! _DEBUG_FILELINE_ */
}   /* sysDbgLogId */

#endif /* _DEBUG_LOG_ */

#if defined (_DEBUG_LOG_) || (defined (_DEBUG_) && (_DEBUG_CONSOLELOG_ >= 1))
/*
 * Logs custom formatted debugging string with auxiliary parameters.
 */
void sysDbgLogStr(
        const char * const format,
        ...
        )
{
    char    message[SYS_DBG_LOG_BUFFER_SIZE]; /* String buffer for the message to be logged. */
    va_list args;                             /* Pointer to the variable arguments list of this function. */

    va_start(args, format);
    vsnprintf(message, SYS_DBG_LOG_BUFFER_SIZE, format, args); /* TODO: Implement custom tiny formatted print. */
    va_end(args);

    HAL_DbgLogStr(message);
}

#endif /* if defined (_DEBUG_LOG_) || (defined (_DEBUG_) && (_DEBUG_CONSOLELOG_ >= 1)) */
} /* __cplusplus */
#endif /* RF4CE_SUPPORT */

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

void CRemote::remoteCallback()
{
    eRet         ret = eRet_Ok;
    CRemoteEvent event;

    ret = removeEvent(&event);
    if (eRet_Ok == ret)
    {
        submitCode(&event);
    }
} /* remoteCallback */

void CRemote::submitCode(CRemoteEvent * pEvent)
{
    eRet            ret           = eRet_Ok;
    int             berror        = 0;
    CWidgetEngine * pWidgetEngine = getWidgetEngine();

    BDBG_ASSERT(NULL != pWidgetEngine);

    BDBG_MSG(("Remote event code:%#x repeat:%d", pEvent->getCode(), pEvent->isRepeat()));
    berror = bwidget_enter_key(pWidgetEngine->getWidgetEngine(), eKey2bwidgets[pEvent->getCode()], pEvent->isRepeat() ? false : true);

    if ((-1 == berror) && (false == pEvent->isRepeat()))
    {
        BDBG_MSG(("bwidgets did not consume key event so pass on to registered observers"));
        /* bwidgets did not consume key so pass on to registered observers */
        ret = notifyObservers(eNotify_KeyDown, pEvent);
        CHECK_ERROR("unable to notify observers", ret);
    }
} /* submitCode */

eRet CRemote::addEvent(CRemoteEvent * pEvent)
{
    eRet ret = eRet_Ok;

    if (NULL != pEvent)
    {
        CRemoteEvent * pEventNew = new CRemoteEvent(pEvent);
        _eventList.add(pEventNew);
    }

    return(ret);
}

eRet CRemote::removeEvent(CRemoteEvent * pEvent)
{
    CRemoteEvent * pEventSaved = NULL;
    eRet           ret         = eRet_NotAvailable;

    pEventSaved = _eventList.remove();
    if (NULL != pEventSaved)
    {
        *pEvent = *pEventSaved;
        delete(pEventSaved);
        ret = eRet_Ok;
    }

    return(ret);
}

static void nexusIrCallback(
        void * context,
        int    param
        )
{
    eRet         ret      = eRet_Ok;
    CIrRemote *  irRemote = (CIrRemote *)context;
    CRemoteEvent event;

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

CIrRemote::CIrRemote(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    CRemote(name, number, eBoardResource_irRemote, pCfg),
    _irInput(NULL)
{
    /* verify key mapping size matches */
    BDBG_ASSERT(eKey_Max == sizeof(eKey2bwidgets)/sizeof(bwidget_key));
}

eRet CIrRemote::open(CWidgetEngine * pWidgetEngine)
{
    NEXUS_IrInputSettings settings;
    eRet                  ret = eRet_Ok;

    BDBG_ASSERT(NULL != pWidgetEngine);

    _pWidgetEngine = pWidgetEngine;
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->addCallback(this, CALLBACK_IRREMOTE, bwinRemoteCallback, ePowerMode_S3);
    }

    NEXUS_IrInput_GetDefaultSettings(&settings);
    settings.dataReady.callback = nexusIrCallback;
    settings.dataReady.context  = this;
    _irInput                    = NEXUS_IrInput_Open(getNumber(), &settings);
    CHECK_PTR_ERROR_GOTO("nexus ir input open failed!", _irInput, ret, eRet_ExternalError, error);

error:
    return(ret);
} /* open */

void CIrRemote::close()
{
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_IRREMOTE);
        _pWidgetEngine = NULL;
    }

    if (NULL != _irInput)
    {
        NEXUS_IrInput_Close(_irInput);
        _irInput = NULL;
    }
}

CIrRemote::~CIrRemote()
{
    close();
}

eRet CIrRemote::setMode(NEXUS_IrInputMode mode)
{
    eRet                  ret = eRet_Ok;
    NEXUS_IrInputSettings settings;
    NEXUS_Error           nerror = NEXUS_SUCCESS;

    BDBG_ASSERT(NEXUS_IrInputMode_eMax > mode);

    NEXUS_IrInput_GetSettings(_irInput, &settings);
    settings.mode = mode;
    nerror        = NEXUS_IrInput_SetSettings(_irInput, &settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set ir remote mode", ret, nerror, error);

error:
    return(ret);
}

NEXUS_IrInputMode CIrRemote::getMode()
{
    NEXUS_IrInputSettings settings;

    BDBG_ASSERT(NULL != _irInput);

    NEXUS_IrInput_GetSettings(_irInput, &settings);
    return(settings.mode);
}

void CIrRemote::dump()
{
    BDBG_MSG(("<%d>%s:%d",
              _type,
              _name.s(),
              _number));
}

#define INVALID  0xFFFFFFFF
struct g_IrRemoteCodes
{
    eKey       key;
    uint32_t   codeRemoteA;
    uint32_t   codeBroadcom;
    uint32_t   codeEchoStar;
    char       strName[32];
} g_IrRemoteCodes[] =
{
    /* eKey             RemoteA     Broadcom    EchoStar    strName */
    { eKey_Invalid,     INVALID,    INVALID,    INVALID,    "eKey_Invalid"     },
    { eKey_Select,      0x0000e011, 0xF708FF00, 0x00000002, "eKey_Select"      },
    { eKey_UpArrow,     0x00009034, 0xB14EFF00, 0x00000016, "eKey_UpArrow"     },
    { eKey_DownArrow,   0x00008035, 0xF30CFF00, 0x0000001E, "eKey_DownArrow"   },
    { eKey_LeftArrow,   0x00007036, 0xF40BFF00, 0x0000000E, "eKey_LeftArrow"   },
    { eKey_RightArrow,  0x00006037, 0xB649FF00, 0x00000006, "eKey_RightArrow"  },
    { eKey_Backspace,   INVALID,    INVALID,    INVALID,    "eKey_Backspace"   },
    { eKey_Delete,      INVALID,    INVALID,    INVALID,    "eKey_Delete"      },
    { eKey_ChannelUp,   0x0000500b, 0xF609FF00, 0x0000083C, "eKey_ChannelUp"   },
    { eKey_ChannelDown, 0x0000400c, 0xF20DFF00, 0x00000838, "eKey_ChannelDown" },
    { eKey_VolumeUp,    0x0000300d, 0xB748FF00, INVALID,    "eKey_VolumeUp"    },
    { eKey_VolumeDown,  0x0000200e, 0xB34CFF00, INVALID,    "eKey_VolumeDown"  },
    { eKey_Mute,        0x0000100f, 0xFE01FF00, INVALID,    "eKey_Mute"        },
    { eKey_Play,        0x0000401b, 0xE21DFF00, 0x00000830, "eKey_Play"        },
    /* duplicate play code for black RemoteA */
    { eKey_Play,        0x00005038, 0xE21DFF00, 0x00000830, "eKey_Play"        },
    { eKey_Stop,        0x00004039, 0xA35CFF00, 0x00000021, "eKey_Stop"        },
    { eKey_Pause,       0x0000001f, 0xE31CFF00, 0x00000001, "eKey_Pause"       },
    { eKey_Rewind,      0x0000101e, 0xE619FF00, 0x00000823, "eKey_Rewind"      },
    { eKey_FastForward, 0x0000201d, 0xA659FF00, 0x00000813, "eKey_FastForward" },
    { eKey_Record,      0x0000C031, 0xAB54FF00, 0x0000003E, "eKey_Record"      },
    { eKey_Menu,        0x00006019, 0xB04FFF00, 0x00000034, "eKey_Menu"        },
    { eKey_Info,        0x0000A033, 0xF00FFF00, 0x00000000, "eKey_Info"        },
    { eKey_Exit,        0x0000D012, INVALID,    INVALID,    "eKey_Exit"        },
    { eKey_Dot,         0x0000B014, 0xAC53FF00, INVALID,    "eKey_Dot"         },
    /* duplicate play code for black RemoteA */
    { eKey_Dot,         0x0000F010, 0xAC53FF00, INVALID,    "eKey_Dot"         },
    { eKey_Enter,       0x00009016, 0xEC13FF00, INVALID,    "eKey_Enter"       },
    { eKey_Last,        0x0000C013, 0xF906FF00, 0x00000036, "eKey_Last"        },
    { eKey_Pip,         0x00008017, 0xE916FF00, 0x00000817, "eKey_Pip"         },
    { eKey_Swap,        0x00006028, 0xED12FF00, 0x0000082F, "eKey_Swap"        },
    { eKey_JumpFwd,     0x0000203B, 0xE718FF00, 0x0000083B, "eKey_JumpFwd"     },
    { eKey_JumpRev,     0x0000303A, 0xA758FF00, 0x0000081B, "eKey_JumpRev"     },
    { eKey_Power,       0x0000600A, 0xF50AFF00, 0x00000010, "eKey_Power"       },
    /* ascii codes */
    { eKey_0,           0x00000000, 0xAD52FF00, 0x00000022, "eKey_0"           },
    { eKey_1,           0x0000F001, 0xE01FFF00, 0x00000008, "eKey_1"           },
    { eKey_2,           0x0000E002, 0xA15EFF00, 0x00000028, "eKey_2"           },
    { eKey_3,           0x0000D003, 0xA05FFF00, 0x00000018, "eKey_3"           },
    { eKey_4,           0x0000C004, 0xE41BFF00, 0x00000004, "eKey_4"           },
    { eKey_5,           0x0000B005, 0xA55AFF00, 0x00000024, "eKey_5"           },
    { eKey_6,           0x0000A006, 0xA45BFF00, 0x00000014, "eKey_6"           },
    { eKey_7,           0x00009007, 0xE817FF00, 0x0000000C, "eKey_7"           },
    { eKey_8,           0x00008008, 0xA956FF00, 0x0000002C, "eKey_8"           },
    { eKey_9,           0x00007009, 0xA857FF00, 0x0000001C, "eKey_9"           },
    { eKey_Max,         INVALID,    INVALID,    INVALID,    "eKey_Max"         }
};

char * CIrRemote::getRemoteCodeName(eKey key)
{
    int max = sizeof(g_IrRemoteCodes)/sizeof(g_IrRemoteCodes[0]);

    for (int i = 0; i < max; i++)
    {
        if (key == g_IrRemoteCodes[i].key)
        {
            return(g_IrRemoteCodes[i].strName);
        }
    }

    return(NULL);
}

eKey CIrRemote::convertRemoteCode(
        NEXUS_IrInputMode mode,
        uint32_t          code
        )
{
    eKey     key = eKey_Invalid;
    unsigned i   = 0;

    for (i = 0; i < sizeof(g_IrRemoteCodes) / sizeof(g_IrRemoteCodes[0]); i++)
    {
        switch (mode)
        {
        case NEXUS_IrInputMode_eRemoteA:
            /*BDBG_MSG(("%s: RemoteA: comparing %d with %d", __FUNCTION__, code, g_IrRemoteCodes[i].codeRemoteA ));*/
            if (code == g_IrRemoteCodes[i].codeRemoteA)
            {
                key = g_IrRemoteCodes[i].key;
            }
            break;
        case NEXUS_IrInputMode_eCirNec:
            /*BDBG_MSG(("%s: RemoteA: comparing %d with %d", __FUNCTION__, code, g_IrRemoteCodes[i].codeBroadcom ));*/
            if (code == g_IrRemoteCodes[i].codeBroadcom)
            {
                key = g_IrRemoteCodes[i].key;
            }
            break;
        case NEXUS_IrInputMode_eCirEchoStar:
            /*BDBG_MSG(("%s: RemoteA: comparing %d with %d", __FUNCTION__, code, g_IrRemoteCodes[i].codeEchoStar ));*/
            if (code == g_IrRemoteCodes[i].codeEchoStar)
            {
                key = g_IrRemoteCodes[i].key;
            }
            break;
        default:
            break;
        } /* switch */
    }

    return(key);
} /* convertRemoteCode */

eRet CIrRemote::getEvent(CRemoteEvent * pEvent)
{
    eRet               ret    = eRet_NotAvailable;
    NEXUS_Error        nerror = NEXUS_SUCCESS;
    NEXUS_IrInputEvent irEvent;
    size_t             num;
    bool               overflow;

    BDBG_ASSERT(NULL != pEvent);

    nerror = NEXUS_IrInput_GetEvents(getIrRemote(), &irEvent, 1, &num, &overflow);
    /* do not buffer any remote events */
    NEXUS_IrInput_FlushEvents(getIrRemote());

    if (!nerror && num)
    {
        eKey key = eKey_Invalid;

        BDBG_MSG(("ir event: %#x repeat:%d", irEvent.code, irEvent.repeat));
        pEvent->setRepeat(irEvent.repeat);

        key = convertRemoteCode(getMode(), irEvent.code);
        if ((eKey_Invalid != key) && (eKey_Max != key))
        {
            pEvent->setCode(key);
            BDBG_MSG(("key:%s", getRemoteCodeName(key)));
        }
        else
        {
            BDBG_WRN(("unhandled remote code (REMOTE_TYPE_PRIMARY:%s) 0x%0x", GET_STR(_pCfg, REMOTE_TYPE_PRIMARY), irEvent.code));
        }

        ret = eRet_Ok;
    }

    return(ret);
} /* getEvent */

#ifdef RF4CE_SUPPORT
CRf4ceRemote::CRf4ceRemote(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    CRemote(name, number, eBoardResource_rf4ceRemote, pCfg),
    _rf4ceInput(0)
{
    /* verify key mapping size matches */
    BDBG_ASSERT(eKey_Max == sizeof(eKey2bwidgets)/sizeof(bwidget_key));
}

extern "C" {
static int permPairingRef = 0;

/* Called, only once in a lifetime */
void My_RF4CE_ZRC_PairInd(RF4CE_PairingIndParams_t * indication)
{
    printf("MY_RF4CE_APP:  In My_RF4CE_ZRC_PairInd, indication->pairingRef=0x%x\n", indication->pairingRef);
    permPairingRef = indication->pairingRef;
}

/* Called, only once in a lifetime */
void My_RF4CE_ZRC_CheckValidationInd(RF4CE_ZRC2_CheckValidationIndParams_t * indication)
{
    if (fork() == 0)
    {
        /* RF4CE_ZRC2_CheckValidationRespConfParams_t conf; */
        printf("Please press Y/N to accept/not accept this validation code, Or press Enter to continue\n");
        uint8_t keyCode = getchar();
        switch (keyCode)
        {
        case 'Y':
            /* conf.status = RF4CE_GDP_VALIDATION_SUCCESS; */
            break;
        case 'N':
            /* conf.status = RF4CE_GDP_VALIDATION_FAILURE; */
            break;
        default:
            /* conf.status = RF4CE_GDP_VALIDATION_PENDING; */
            break;
        }   /*
             * switch
             * indication->callback(indication, &conf);
             */
        exit(0);
    }
}   /* My_RF4CE_ZRC_CheckValidationInd */

static uint8_t statusRegistered;

void rf4ce_Test_RF4CE_RegisterVirtualDevice_Callback(
        RF4CE_RegisterVirtualDeviceReqDescr_t *   request,
        RF4CE_RegisterVirtualDeviceConfParams_t * conf
        )
{
    BSTD_UNUSED(request);
    if ((int)conf->status >= RF4CE_REGISTER_SUCCESS)
    {
        statusRegistered = 1;
    }
}

static void rf4ce_Test_Show_My_Interest(int prn)
{
    #define PAN_ID         0x5678
    #define PROFILE_ID     0x1234
    #define IEEE_ADDR      "\xaa\xbb\xcc\xdd\xee\xff\x00\x11"
    #define VENDOR_ID      0x9876
    #define VENDOR_STRING  "BRCM"
    /* To test this function, this kinds of parameters need to match with stack */
    statusRegistered = 0;
    RF4CE_RegisterVirtualDeviceReqDescr_t req;
    memset(&req, 0, sizeof(RF4CE_RegisterVirtualDeviceReqDescr_t));
    /* req.params.fieldValidMask = RF4CE_ORGVENDORID_MASK | RF4CE_ORGPROFILEID_MASK | RF4CE_MACADDRESS_MASK; */
    req.params.fieldValidMask = RF4CE_PAIRREF_MASK;
    /*
     * req.params.orgVendorId = VENDOR_ID;
     * req.params.orgProfileID = PROFILE_ID;
     */
    req.params.pairRef = prn;
    req.callback       = rf4ce_Test_RF4CE_RegisterVirtualDevice_Callback;
    RF4CE_RegisterVirtualDevice(&req);
    while (!statusRegistered)
    {
    }
}   /* rf4ce_Test_Show_My_Interest */

static uint8_t statusStarted = 0;
void rf4ce_Test_Start_NWK_Callback(
        RF4CE_StartReqDescr_t *        request,
        RF4CE_StartResetConfParams_t * conf
        )
{
    BSTD_UNUSED(request);
    if (RF4CE_START_RESET_OK == conf->status)
    {
        statusStarted = 1;
    }
}

static void rf4ce_Test_Start_NWK_Callback_async(
        RF4CE_StartReqDescr_t *        request,
        RF4CE_StartResetConfParams_t * conf
        )
{
    BSTD_UNUSED(request);
    if (RF4CE_START_RESET_OK == conf->status)
    {
        BKNI_SetEvent(start_nwk_event);
    }
}

#ifdef START_NWK_THREAD
static void rf4ce_Test_Start_NWK_async()
{
    printf("Starting RF4CE NWK async\r\n");
    RF4CE_StartReqDescr_t req;
    req.callback = rf4ce_Test_Start_NWK_Callback_async;
    RF4CE_StartReq(&req);
    BKNI_CreateEvent(&start_nwk_event);
    BKNI_WaitForEvent(start_nwk_event, BKNI_INFINITE);
    BKNI_DestroyEvent(start_nwk_event);
    printf("Start NWK successful\r\n");
}

#else /* ifdef START_NWK_THREAD */
static void rf4ce_Test_Start_NWK()
{
    printf("Starting RF4CE NWK\r\n");
    RF4CE_StartReqDescr_t req;
    req.callback = rf4ce_Test_Start_NWK_Callback;
    RF4CE_StartReq(&req);
    while (!statusStarted)
    {
    }
    printf("Start NWK successful\r\n");
}

#endif /* ifdef START_NWK_THREAD */

static uint8_t statusTargetBinding = 0, statusConf = 0xff;

void rf4ce_Test_ZRC1_TargetBinding_Callback(
        RF4CE_ZRC1_BindReqDescr_t *   request,
        RF4CE_ZRC1_BindConfParams_t * conf
        )
{
    BSTD_UNUSED(request);
    statusTargetBinding = 1;
    statusConf          = conf->status;
    if (RF4CE_ZRC1_BOUND == statusConf)
    {
        printf("One remote control has been bound successfully.\r\n");
    }
    else
    {
        printf("No any remote contoller has been bound. status = %d\r\n", statusConf);
    }
}

static void rf4ce_Test_ZRC1_TargetBinding()
{
    printf("Starting ZRC1 target Binding\r\n");

    RF4CE_ZRC1_BindReqDescr_t req;

    req.callback = rf4ce_Test_ZRC1_TargetBinding_Callback;

    statusTargetBinding = 0;
    RF4CE_ZRC1_TargetBindReq(&req);
    printf("waiting for rf4ce_Test_ZRC1_TargetBinding_Callback..., statusConf=%d\n", statusConf);
    while (!statusTargetBinding)
    {
        usleep(100);
    }
    printf("got it..., statusConf=%d, permPairingRef=%d\n", statusConf, permPairingRef);
    /*
     *    if(RF4CE_ZRC1_BOUND == statusConf)
     *        printf("One remote control has been bound successfully.\r\n");
     *    else
     *        printf("No any remote contoller has been bound. status = %d\r\n", statusConf);
     */
}   /* rf4ce_Test_ZRC1_TargetBinding */

uint8_t statusUnpair, unpairConfStatus;

static void rf4ce_Test_ZRC1_Unpair_Callback(
        RF4CE_UnpairReqDescr_t *   req,
        RF4CE_UnpairConfParams_t * conf
        )
{
    BSTD_UNUSED(req);
    statusUnpair     = 1;
    unpairConfStatus = conf->status;
}

static void rf4ce_Test_ZRC1_Unpair(uint8_t pairRef)
{
    printf("Starting ZRC1 target Unpair\r\n");
    statusUnpair = 0, unpairConfStatus = 0;

    RF4CE_UnpairReqDescr_t req;

    req.params.pairingRef = pairRef;
    req.callback          = rf4ce_Test_ZRC1_Unpair_Callback;

    RF4CE_UnpairReq(&req);

    while (!statusUnpair)
    {
    }
    if (1 == unpairConfStatus)
    {
        printf("Unpair with pairref %d successfully.\r\n", pairRef);
    }
    else
    {
        printf("Fail to unpair with pairref %d Confirmation Status(%d).\r\n", pairRef, unpairConfStatus);
    }
}   /* rf4ce_Test_ZRC1_Unpair */

static uint8_t statusGetPowerFilterKey;

static void rf4ce_Test_Get_PowerFilterKey_Callback(
        RF4CE_ZRC_GetWakeUpActionCodeReqDescr_t *   request,
        RF4CE_ZRC_GetWakeUpActionCodeConfParams_t * conf
        )
{
    BSTD_UNUSED(request);
    if (0 == conf->status)
    {
        statusGetPowerFilterKey = 1;
        for (uint8_t i = 0; i < sizeof(conf->wakeUpActionCodeFilter); i++)
        {
            printf(" %02x ", conf->wakeUpActionCodeFilter[i]);
        }
        printf("\n");
    }
}

static void rf4ce_Test_Get_WakeUpActionCode()
{
    statusGetPowerFilterKey = 0;

    RF4CE_ZRC_GetWakeUpActionCodeReqDescr_t req = { 0 };

    req.callback = rf4ce_Test_Get_PowerFilterKey_Callback;

    RF4CE_ZRC_GetWakeUpActionCodeReq(&req);
    while (!statusGetPowerFilterKey)
    {
    }
    printf("Get power filter key successfully\r\n");
}

static uint8_t statusSetPowerFilterKey;

static void rf4ce_Test_Set_PowerFilterKey_Callback(
        RF4CE_ZRC_SetWakeUpActionCodeReqDescr_t *   request,
        RF4CE_ZRC_SetWakeUpActionCodeConfParams_t * conf
        )
{
    BSTD_UNUSED(request);
    if (0 == conf->status)
    {
        statusSetPowerFilterKey = 1;
    }
}

static void rf4ce_Test_Set_WakeUpActionCode()
{
    statusSetPowerFilterKey = 0;

    RF4CE_ZRC_SetWakeUpActionCodeReqDescr_t req; /* = {0}; */
    memset(&req, sizeof(req), 0);
    memcpy(req.params.wakeUpActionCodeFilter, "\x00\x20\x00\x00\x00\x00\x00\x00\x00", 9); /* bit 13 (exit) */
    req.callback = rf4ce_Test_Set_PowerFilterKey_Callback;

    RF4CE_ZRC_SetWakeUpActionCodeReq(&req);
    while (!statusSetPowerFilterKey)
    {
    }
    printf("Set power filter key successfully\r\n");
}
} /* extern "C" */

static uint8_t statusRestoreFactorySettings;

static void rf4ce_Restore_Factory_Settings_Callback(
        RF4CE_ResetReqDescr_t *        request,
        RF4CE_StartResetConfParams_t * conf
        )
{
    BSTD_UNUSED(request);
    if (RF4CE_START_RESET_OK == conf->status)
    {
        statusRestoreFactorySettings = 1;
    }
}

static void rf4ce_Restore_Factory_Settings(uint8_t restore)
{
    statusRestoreFactorySettings = 0;

    RF4CE_ResetReqDescr_t req; /* = {0}; */
    memset(&req, sizeof(req), 0);
    req.params.setDefaultPIBNIB = restore;
    req.callback                = rf4ce_Restore_Factory_Settings_Callback;
    RF4CE_ResetReq(&req);
    while (!statusRestoreFactorySettings)
    {
    }
    printf("Restore factory settings successfully\r\n");
}

void My_RF4CE_ZRC_ControlCommandInd(RF4CE_ZRC2_ControlCommandIndParams_t * commandInd)
{
    printf("Got the control command as following\r\n");
    char                  command[200], msg[10] = { 0 };
    RF4CE_ZRC2_Action_t * commands = (RF4CE_ZRC2_Action_t *)calloc(sizeof(char), SYS_GetPayloadSize(&commandInd->payload));
    SYS_CopyFromPayload(commands, &commandInd->payload, 0, SYS_GetPayloadSize(&commandInd->payload));
    for (int i = 0; i < (int)(SYS_GetPayloadSize(&commandInd->payload) / sizeof(RF4CE_ZRC2_Action_t)); i++)
    {
        msg[i] = commands[i].code;
        printf("%c\n", msg[i]);
    }
    sprintf(command, "./figlet -f roman -d ./fonts/ %s", msg);
    system(command);
} /* My_RF4CE_ZRC_ControlCommandInd */

void My_RF4CE_ZRC1_ControlCommandInd(RF4CE_ZRC1_ControlCommandIndParams_t * commandInd)
{
    eRet           ret         = eRet_Ok;
    CRf4ceRemote * rf4ceRemote = (CRf4ceRemote *)rf4ce_context;
    CRemoteEvent   event;

    /* printf("Got the control command as following:\r\n"); */
    rf4ceEvent.code   = commandInd->commandCode;
    rf4ceEvent.repeat = ((commandInd->flags == RF4CE_ZRC1_USER_CONTROL_REPEATED) || (commandInd->flags == RF4CE_ZRC1_USER_CONTROL_RELEASED)) ? 1 : 0;

    BDBG_ASSERT(NULL != rf4ceRemote);
    BDBG_ASSERT(eBoardResource_rf4ceRemote == rf4ceRemote->getType());

    ret = rf4ceRemote->getEvent(&event);
    if (eRet_Ok == ret)
    {
        /* skip repeats */
        if (false == event.isRepeat())
        {
            CWidgetEngine * pWidgetEngine = rf4ceRemote->getWidgetEngine();

            if (NULL != pWidgetEngine)
            {
                /* save event for later processing */
                rf4ceRemote->addEvent(&event);

                /* interrupt the bwin event loop. this will interrupt the sleep and cause the ui to be more responsive. */
                pWidgetEngine->syncCallback(rf4ceRemote, CALLBACK_RF4CEREMOTE);
            }
        }
    }
} /* My_RF4CE_ZRC1_ControlCommandInd */

const char bindingInstruction[] = "\nNow a remote control can be bound\n\n"
                                  "Binding instruction for RemoteSolution remote control:\n"
                                  "1. Press and hold the Setup button on the remote control,\n"
                                  "   until the Indicator LED changes from red to green.\n"
                                  "2. Press the Info button on the remote control.\n"
                                  "Above procedure should be executed within 30 seconds.\n";

const char prebindingInstruction[] = "\nPlease press 'b' to issue binding procedure,\n"
                                     "press any other key to use the existing pair reference.\n";

const char pr_filename[256]     = "/etc/zigbee/pr.txt";
const char pr_filename_new[256] = "/etc/zigbee/pr_new.txt";

void CRemote::displayRf4ceRemotes(void)
{
    fstream pr;
    string  line;
    string  name_of_remote;
    string  prn;

    pr.open(pr_filename);
    if (pr.is_open())
    {
        int pairing_num;
        cout << setw(56) << left << "RF4CE Remote Name" << "Pairing Reference Number" << endl;
        cout << "================================================================================\n";
        while (getline(pr, line))
        {
            if (line.find("#") != string::npos)
            {
                continue;
            }
            name_of_remote = line.substr(0, line.find("="));
            prn            = line.substr(line.find("=")+1, line.length()-line.find("="));
            stringstream(prn) >> pairing_num;
            cout << setw(56) << left << name_of_remote << pairing_num << endl;
        }
        cout << "================================================================================\n";
        pr.close();
    }
    else
    {
        cout << "file " << pr_filename << " does not exist" << endl;
    }
} /* DisplayRf4ceRemotes */

void CRemote::addRf4ceRemote(const char * remote_name)
{
    fstream pr;

    pr.open(pr_filename, ios::out | ios::app);
    if (pr.is_open())
    {
        char buf[64];

        printf(bindingInstruction);
        rf4ce_Test_ZRC1_TargetBinding();
        if (!statusConf)
        {
            /* remote succesfully bound! */
            snprintf(buf, sizeof(buf), "%d", permPairingRef);
            pr << remote_name << "=" << buf << "\n";
        }
        else
        {
            cout << "remote not successfully bound!" << endl;
        }
        pr.flush();
        pr.close();
    }
    else
    {
        printf("not successfully able to open a file\n");
    }
} /* AddRf4ceRemote */

void CRemote::removeRf4ceRemote(int pairing_num_to_delete)
{
    fstream pr;
    fstream pr_new;
    string  line;
    string  name_of_remote;
    string  prn;

    pr.open(pr_filename);
    if (pr.is_open())
    {
        int pairing_num;
        pr.close();

        pr.open(pr_filename, ios::in);
        pr_new.open(pr_filename_new, ios::out);
        while (getline(pr, line))
        {
            if (line.find("#") == string::npos)
            {
                /* Not a line with a comment. */
                name_of_remote = line.substr(0, line.find("="));
                prn            = line.substr(line.find("=")+1, line.length()-line.find("="));
                stringstream(prn) >> pairing_num;
                if (pairing_num_to_delete != pairing_num)
                {
                    pr_new << line << "\n";
                }
                else
                {
                    /* Unpair this remote. */
                    rf4ce_Test_ZRC1_Unpair(pairing_num);
                }
            }
            else
            {
                /* Pass the comment through. */
                pr_new << line << "\n";
            }
        }
        pr.close();
        pr_new.close();

        remove(pr_filename);
        rename(pr_filename_new, pr_filename);
    }
    else
    {
        cout << "file " << pr_filename << " does not exist" << endl;
    }
} /* RemoveRf4ceRemote */

static void InitializeRf4ce(uint8_t restore)
{
    rf4ce_Restore_Factory_Settings(restore);
    rf4ce_Test_Set_WakeUpActionCode();
    rf4ce_Test_Get_WakeUpActionCode();
#ifdef START_NWK_THREAD
    rf4ce_Test_Start_NWK_async();
#else
    rf4ce_Test_Start_NWK();
#endif
}

void * InitializeRf4ceRemotes(void *)
{
    fstream pr;
    string  line;
    string  name_of_remote;
    string  prn;

    /* Check if temporary file exists.  If so, issue warning, and delete it. */
    pr.open(pr_filename_new);
    if (pr.is_open())
    {
        printf("ERROR:  temporary file %s exists.  Deleting it...\n", pr_filename_new);
        pr.close();
        remove(pr_filename_new);
    }

    pr.open(pr_filename);
    if (pr.is_open())
    {
        int pairing_num;
        printf("file %s exists\n", pr_filename);
        InitializeRf4ce(0);

        while (getline(pr, line))
        {
            if (line.find("#") != string::npos)
            {
                continue;
            }

            name_of_remote = line.substr(0, line.find("="));
            prn            = line.substr(line.find("=")+1, line.length()-line.find("="));
            stringstream(prn) >> pairing_num;
            cout << "name_of_remote:  " << name_of_remote << ", pairing reference number:  " << pairing_num << "\n";

            rf4ce_Test_Show_My_Interest(pairing_num);
        }

        pr.close();
    }
    else
    {
        printf("file %s does not exist!  Creating one...\n", pr_filename);
        InitializeRf4ce(1);

        pr.open(pr_filename, fstream::out);
        if (pr.is_open())
        {
            pr << "# RF4CE Pairing Reference File\n";
            pr.flush();
            pr.close();
        }
        else
        {
            printf("Unable to create %s\n", pr_filename);
        }
    }

#ifdef START_NWK_THREAD
    pthread_exit(0);
#endif
    return(NULL);
} /* InitializeRf4ceRemotes */

eRet CRf4ceRemote::open(CWidgetEngine * pWidgetEngine)
{
    zigbeeCallback zcb;
    eRet           ret = eRet_Ok;

    BDBG_ASSERT(NULL != pWidgetEngine);

    _pWidgetEngine = pWidgetEngine;
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->addCallback(this, CALLBACK_RF4CEREMOTE, bwinRemoteCallback, ePowerMode_S3);
    }

    /*
     * Register the callback functions you are interested in.  Ones that are not filled out, won't be called back.
     * Calling Zigbee_GetDefaultSettings will initialize the callback structure
     */
    Zigbee_GetDefaultSettings(&zcb);
    zcb.RF4CE_PairInd                 = My_RF4CE_ZRC_PairInd;
    zcb.RF4CE_ZRC2_CheckValidationInd = My_RF4CE_ZRC_CheckValidationInd;
    zcb.RF4CE_ZRC2_ControlCommandInd  = My_RF4CE_ZRC_ControlCommandInd;
    zcb.RF4CE_ZRC1_ControlCommandInd  = My_RF4CE_ZRC1_ControlCommandInd;
    rf4ce_context                     = this;
    Zigbee_Open(&zcb, NULL /*argv[1]*/);
#ifdef START_NWK_THREAD
    pthread_create(&start_nwk_thread, NULL, InitializeRf4ceRemotes, NULL);
#else
    InitializeRf4ceRemotes(NULL);
#endif

    /* error: */
    return(ret);
} /* open */

void CRf4ceRemote::close()
{
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_RF4CEREMOTE);
        _pWidgetEngine = NULL;
    }

    Zigbee_Close();
}

CRf4ceRemote::~CRf4ceRemote()
{
    close();
}

void CRf4ceRemote::dump()
{
    BDBG_MSG(("<%d>%s:%d",
              _type,
              _name.s(),
              _number));
}

#ifdef INVALID
#undef INVALID
#endif
#define INVALID  0xFF
struct g_Rf4ceRemoteCodes
{
    eKey      key;
    uint8_t   code;
    char      strName[32];
} g_Rf4ceRemoteCodes[] =
{
    /* eKey             code        strName */
    { eKey_Invalid,     INVALID, "eKey_Invalid"     },
    { eKey_Select,      0x00,    "eKey_Select"      },
    { eKey_UpArrow,     0x01,    "eKey_UpArrow"     },
    { eKey_DownArrow,   0x02,    "eKey_DownArrow"   },
    { eKey_LeftArrow,   0x03,    "eKey_LeftArrow"   },
    { eKey_RightArrow,  0x04,    "eKey_RightArrow"  },
    { eKey_Backspace,   INVALID, "eKey_Backspace"   },
    { eKey_Delete,      INVALID, "eKey_Delete"      },
    { eKey_ChannelUp,   0x30,    "eKey_ChannelUp"   },
    { eKey_ChannelDown, 0x31,    "eKey_ChannelDown" },
    { eKey_VolumeUp,    0x41,    "eKey_VolumeUp"    },
    { eKey_VolumeDown,  0x42,    "eKey_VolumeDown"  },
    { eKey_Mute,        0x43,    "eKey_Mute"        },
    { eKey_Play,        0x44,    "eKey_Play"        },
    { eKey_Stop,        0x45,    "eKey_Stop"        },
    { eKey_Pause,       0x46,    "eKey_Pause"       },
    { eKey_Rewind,      0x48,    "eKey_Rewind"      },
    { eKey_FastForward, 0x49,    "eKey_FastForward" },
    { eKey_Record,      0x47,    "eKey_Record"      },
    { eKey_Menu,        0x9,     "eKey_Menu"        },
    { eKey_Info,        0x35,    "eKey_Info"        },
#if 0
    { eKey_Exit,        0xd,     "eKey_Exit"        },
#else
    { eKey_Power,       0xd,     "eKey_Power"       }, /* temporary */
#endif
    { eKey_Dot,         INVALID, "eKey_Dot"         },
    { eKey_Dot,         INVALID, "eKey_Dot"         },
    { eKey_Enter,       INVALID, "eKey_Enter"       },
    { eKey_Last,        0x32,    "eKey_Last"        },
    { eKey_Pip,         INVALID, "eKey_Pip"         },
    { eKey_Swap,        INVALID, "eKey_Swap"        },
    { eKey_JumpFwd,     INVALID, "eKey_JumpFwd"     },
    { eKey_JumpRev,     INVALID, "eKey_JumpRev"     },
    { eKey_Power,       INVALID, "eKey_Power"       },
    /* ascii codes */
    { eKey_0,           0x20,    "eKey_0"           },
    { eKey_1,           0x21,    "eKey_1"           },
    { eKey_2,           0x22,    "eKey_2"           },
    { eKey_3,           0x23,    "eKey_3"           },
    { eKey_4,           0x24,    "eKey_4"           },
    { eKey_5,           0x25,    "eKey_5"           },
    { eKey_6,           0x26,    "eKey_6"           },
    { eKey_7,           0x27,    "eKey_7"           },
    { eKey_8,           0x28,    "eKey_8"           },
    { eKey_9,           0x29,    "eKey_9"           },
    { eKey_Max,         INVALID, "eKey_Max"         }
};

char * CRf4ceRemote::getRemoteCodeName(eKey key)
{
    for (unsigned i = 0; i < sizeof(g_Rf4ceRemoteCodes)/sizeof(g_Rf4ceRemoteCodes[0]); i++)
    {
        if (key == g_Rf4ceRemoteCodes[i].key)
        {
            return(g_Rf4ceRemoteCodes[i].strName);
        }
    }

    return(NULL);
}

eKey CRf4ceRemote::convertRemoteCode(uint8_t code)
{
    eKey     key = eKey_Invalid;
    unsigned i   = 0;

    for (i = 0; i < sizeof(g_Rf4ceRemoteCodes) / sizeof(g_Rf4ceRemoteCodes[0]); i++)
    {
        if (code == g_Rf4ceRemoteCodes[i].code)
        {
            key = g_Rf4ceRemoteCodes[i].key;
        }
    }

    return(key);
} /* convertRemoteCode */

#define RF4CE_TIME_THRESHOLD_USEC       70000
#define RF4CE_SAME_KEY_PRESS_THRESHOLD  4

eRet CRf4ceRemote::getEvent(CRemoteEvent * pEvent)
{
    eRet ret = eRet_NotAvailable;

    BDBG_ASSERT(NULL != pEvent);

#if 0
    nerror = NEXUS_IrInput_GetEvents(getIrRemote(), &irEvent, 1, &num, &overflow);
    /* do not buffer any remote events */
    NEXUS_IrInput_FlushEvents(getIrRemote());
#endif

    if (1)
    {
        eKey key = eKey_Invalid;

        BDBG_MSG(("rf4ce event: %#x repeat:%d", rf4ceEvent.code, rf4ceEvent.repeat));
        pEvent->setRepeat(rf4ceEvent.repeat);

        key = convertRemoteCode(rf4ceEvent.code);
        if ((eKey_Invalid != key) && (eKey_Max != key))
        {
            pEvent->setCode(key);
            BDBG_MSG(("key:%s", getRemoteCodeName(key)));
        }
        else
        {
            BDBG_WRN(("unhandled remote code 0x%0x", rf4ceEvent.code));
        }

        ret = eRet_Ok;
    }

    return(ret);
} /* getEvent */

#endif /* RF4CE_SUPPORT */

#ifdef NETAPP_SUPPORT
static void LinuxInputSource_P_Task(void * pParam)
{
    CBluetoothRemote * pBluetoothRemote = (CBluetoothRemote *)pParam;

    if (pBluetoothRemote != NULL)
    {
        pBluetoothRemote->thread(pBluetoothRemote);
    }
    BDBG_MSG(("%s(): Leaving LinuxInputSource_P_Task thread now\n", __FUNCTION__));
    NetAppOSTaskExit();
}

static void bluetoothRemoteCallback(
        void *           context,
        hidInputSource * pHidInputSource
        )
{
    eRet               ret             = eRet_Ok;
    CBluetoothRemote * bluetoothRemote = (CBluetoothRemote *)context;
    CRemoteEvent       event;

    BDBG_ASSERT(NULL != bluetoothRemote);
    BDBG_ASSERT(eBoardResource_bluetoothRemote == bluetoothRemote->getType());

    /* Call get Event which has another paramt to specifiy the fd to read from  */
#if 1
    ret = bluetoothRemote->getEvent(&event, pHidInputSource);
#else
    ret = bluetoothRemote->getEvent(&event);
#endif
    if (eRet_Ok == ret)
    {
        /* skip repeats */
        if (false == event.isRepeat())
        {
            CWidgetEngine * pWidgetEngine = bluetoothRemote->getWidgetEngine();

            if (NULL != pWidgetEngine)
            {
                /* save event for later processing */
                bluetoothRemote->addEvent(&event);

                /* interrupt the bwin event loop. this will interrupt the sleep and cause the ui to be more responsive. */
                pWidgetEngine->syncCallback(bluetoothRemote, CALLBACK_BLUETOOTHREMOTE);
            }
        }
    }
} /* bluetoothRemoteCallback */

void CBluetoothRemote::thread(CBluetoothRemote * pBluetoothRemote)
{
    while (!_shutdown)
    {
        B_Mutex_Lock(_hMutex);

        /* check if device name already exist */
        MListItr <hidInputSource> itr(&_hidInputSourceList);
        hidInputSource *          pHidInputSource = NULL;

        for (pHidInputSource = itr.first(); pHidInputSource; pHidInputSource = itr.next())
        {
            int            rc;
            fd_set         set;
            struct timeval tm;
            tm.tv_sec  = 0;
            tm.tv_usec = 0;
            FD_ZERO(&set);
            FD_SET(pHidInputSource->fd, &set);
            /* Rather than select blocking and returning for each detected input, we will have a little timeout that
             * will allow us to buffer up input events when there are many
             */
            rc = select(pHidInputSource->fd + 1, &set, NULL, NULL, &tm);

            if (rc < 0)
            {
                BDBG_ERR(("%s(): Error polling the file descriptors, error=%s!\n", __FUNCTION__, strerror(errno)));
                return;
            }
            else
            if (rc > 0)
            {
                bluetoothRemoteCallback(pBluetoothRemote, pHidInputSource);
            }
        }

        B_Mutex_Unlock(_hMutex);

        NetAppOSTaskDelayMsec(200); /*if number too small, you start to get repeats*/
    }
} /* thread */

CBluetoothRemote::CBluetoothRemote(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    CRemote(name, number, eBoardResource_bluetoothRemote, pCfg),
    _shutdown(false),
    _fd(0),
    _shiftPressed(false),
    _taskId(0),
    _hMutex(0)
{
    /* verify key mapping size matches */
    memset(_devName, 0, sizeof(_devName));
    BDBG_ASSERT(eKey_Max == sizeof(eKey2bwidgets)/sizeof(bwidget_key));

    bt_semaphore = NetAppOSSemBCreate(true);

    if (bt_semaphore == NULL)
    {
        BDBG_ERR(("%s(): -- Cannot create sem!", __FUNCTION__));
        BDBG_ASSERT(bt_semaphore);
    }
    memset(&prev_event, 0, sizeof(prev_event));
}

eRet CBluetoothRemote::open(CWidgetEngine * pWidgetEngine)
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != pWidgetEngine);

    _pWidgetEngine = pWidgetEngine;
    if (NULL != _pWidgetEngine)
    {
        ret = _pWidgetEngine->addCallback(this, CALLBACK_BLUETOOTHREMOTE, bwinRemoteCallback, ePowerMode_S3);
    }

    _hMutex = B_Mutex_Create(NULL);

    /* Spawn polling thread that looks at the inputs */
    _taskId = NetAppOSTaskSpawn("LinuxInputSourceTask", NETAPP_OS_PRIORITY_LOW, 64*1024, LinuxInputSource_P_Task, this);

    if (_taskId == NULL)
    {
        BDBG_ERR(("%s(): Failure Spawning LinuxInputSourceTask!!", __FUNCTION__));
        return(eRet_ExternalError);
    }

    return(ret);
} /* open */

void CBluetoothRemote::close(void)
{
    MListItr <hidInputSource> itr(&_hidInputSourceList);
    hidInputSource *          pHidInputSource = NULL;

    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_BLUETOOTHREMOTE);
        _pWidgetEngine = NULL;
    }

    /* TODO: call the close here */
    if (_taskId != 0)
    {
        _shutdown = true;
        NetAppOSTaskJoin(_taskId);
        NetAppOSTaskDelete(_taskId);
        _taskId = 0;
    }

    /* go through list and delete all  */
    for (pHidInputSource = itr.first(); pHidInputSource; pHidInputSource = itr.next())
    {
        removeInputSource(pHidInputSource->devName);
    }

    if (_hMutex != NULL)
    {
        B_Mutex_Destroy(_hMutex);
        _hMutex = NULL;
    }
} /* close */

eRet CBluetoothRemote::addInputSource(char * devName)
{
    eRet ret = eRet_Ok;

    MListItr <hidInputSource> itr(&_hidInputSourceList);
    hidInputSource *          pHidInputSource = NULL;
    std::stringstream         ss;

    B_Mutex_Lock(_hMutex);

    /* check if device name already exist */
    if (devName == NULL)
    {
        ret = eRet_InvalidParameter;
        goto error;
    }

    /* update Input device name already in list */
    for (pHidInputSource = itr.first(); pHidInputSource; pHidInputSource = itr.next())
    {
        if (strcmp(pHidInputSource->devName, devName) == 0)
        {
            /* found match in list - don't add Input Source return from funciton  */
            BDBG_WRN(("Add InputSource: Already Found devName around in Input Source list %s ", devName));
            ret = eRet_InvalidParameter;
            goto error;
        }
    }

    /*Create new hid Input source */
    pHidInputSource = (hidInputSource *)malloc(sizeof(hidInputSource));
    strncpy(pHidInputSource->devName, devName, sizeof(pHidInputSource->devName));

    ss << LINUX_INPUT_NODE;
    ss << devName;

    if ((pHidInputSource->fd = ::open(ss.str().c_str(), O_RDWR)) < 0)
    {
        BDBG_ERR(("%s(): Failure Opening input device %s!!", __FUNCTION__, ss.str().c_str()));
        return(eRet_ExternalError);
    }

    /* add pHidInputSource to list */
    BDBG_MSG(("add new InputSource  to hidInputSource list:%s ch:%d", pHidInputSource->devName, pHidInputSource->fd));
    _hidInputSourceList.add(pHidInputSource);

error:
    B_Mutex_Unlock(_hMutex);
    return(ret);
} /* addInputSource */

eRet CBluetoothRemote::removeInputSource(char * devName)
{
    eRet ret = eRet_Ok;

    MListItr <hidInputSource> itr(&_hidInputSourceList);
    hidInputSource *          pHidInputSource = NULL;

    B_Mutex_Lock(_hMutex);

    /* check if device name already exist */
    if (devName == NULL)
    {
        ret = eRet_InvalidParameter;
        goto done;
    }

    /* update Input device name already in list */
    for (pHidInputSource = itr.first(); pHidInputSource; pHidInputSource = itr.next())
    {
        if (strcmp(pHidInputSource->devName, devName) == 0)
        {
            /* found match in list - don't add Input Source return from funciton  */
            BDBG_MSG(("Removing Input Source: Found devName  in Input Source list %s ", devName));

            if (pHidInputSource->fd != 0)
            {
                ::close(pHidInputSource->fd);
                pHidInputSource->fd = 0;
            }
            _hidInputSourceList.remove(pHidInputSource);
            free(pHidInputSource);
            goto done;
        }
    }

    BDBG_MSG(("Removing Input Source: Could Not Find devName  in Input Source list %s ", devName));
done:
    B_Mutex_Unlock(_hMutex);
    return(ret);
} /* removeInputSource */

CBluetoothRemote::~CBluetoothRemote()
{
    if (bt_semaphore)
    {
        NetAppOSSemDelete(bt_semaphore);
        bt_semaphore = NULL;
    }

    close();
}

void CBluetoothRemote::dump()
{
    BDBG_MSG(("<%d>%s:%d",
              _type,
              _name.s(),
              _number));
}

#ifdef INVALID
#undef INVALID
#endif
#define INVALID  0xFF
struct g_BluetoothRemoteCodes
{
    eKey       key;
    uint32_t   code;
    char       strName[32];
} g_BluetoothRemoteCodes[] =
{
    /* eKey             code        strName */
    { eKey_Invalid,     INVALID,            "eKey_Invalid"     },
    { eKey_Select,      KEY_ENTER,          "eKey_Select"      },
    { eKey_UpArrow,     KEY_UP,             "eKey_UpArrow"     },
    { eKey_DownArrow,   KEY_DOWN,           "eKey_DownArrow"   },
    { eKey_LeftArrow,   KEY_LEFT,           "eKey_LeftArrow"   },
    { eKey_RightArrow,  KEY_RIGHT,          "eKey_RightArrow"  },
    { eKey_Backspace,   KEY_BACKSPACE,      "eKey_Backspace"   },
    { eKey_Delete,      KEY_DELETE,         "eKey_Delete"      },
    { eKey_ChannelUp,   KEY_PAGEUP,         "eKey_ChannelUp"   },
    { eKey_ChannelDown, KEY_PAGEDOWN,       "eKey_ChannelDown" },
    { eKey_VolumeUp,    KEY_VOLUMEUP,       "eKey_VolumeUp"    },
    { eKey_VolumeDown,  KEY_VOLUMEDOWN,     "eKey_VolumeDown"  },
    { eKey_Mute,        KEY_MUTE,           "eKey_Mute"        },
    { eKey_Play,        KEY_PLAYPAUSE,      "eKey_Play"        },
    { eKey_Stop,        KEY_STOP,           "eKey_Stop"        },
    { eKey_Pause,       KEY_PAUSE,          "eKey_Pause"       },
    { eKey_Rewind,      KEY_REWIND,         "eKey_Rewind"      },
    { eKey_FastForward, KEY_FORWARD,        "eKey_FastForward" },
    { eKey_Record,      INVALID,            "eKey_Record"      },
    { eKey_Menu,        KEY_HOME,           "eKey_Menu"        },
    { eKey_Info,        KEY_SETUP,          "eKey_Info"        },
    { eKey_Exit,        KEY_ZENKAKUHANKAKU, "eKey_Exit"        }, /* UEI has wierd mapping  shoudl be KEY_EXIT*/
    { eKey_Dot,         INVALID,            "eKey_Dot"         },
    { eKey_Dot,         INVALID,            "eKey_Dot"         },
    { eKey_Enter,       INVALID,            "eKey_Enter"       }, /*  UEI  middle button, enter, but need to use select for atlas and not enter */
    { eKey_Last,        KEY_PREVIOUSSONG,   "eKey_Last"        },
    { eKey_Pip,         INVALID,            "eKey_Pip"         },
    { eKey_Swap,        INVALID,            "eKey_Swap"        },
    { eKey_JumpFwd,     INVALID,            "eKey_JumpFwd"     },
    { eKey_JumpRev,     INVALID,            "eKey_JumpRev"     },
    { eKey_Power,       KEY_POWER,          "eKey_Power"       },
    /* ascii codes */
    { eKey_0,           KEY_0,              "eKey_0"           },
    { eKey_1,           KEY_1,              "eKey_1"           },
    { eKey_2,           KEY_2,              "eKey_2"           },
    { eKey_3,           KEY_3,              "eKey_3"           },
    { eKey_4,           KEY_4,              "eKey_4"           },
    { eKey_5,           KEY_5,              "eKey_5"           },
    { eKey_6,           KEY_6,              "eKey_6"           },
    { eKey_7,           KEY_7,              "eKey_7"           },
    { eKey_8,           KEY_8,              "eKey_8"           },
    { eKey_9,           KEY_9,              "eKey_9"           },
    { eKey_Max,         INVALID,            "eKey_Max"         }
};

eKey CBluetoothRemote::convertRemoteCode(uint32_t code)
{
    MString  strRemote = GET_STR(_pCfg, REMOTE_TYPE_SECONDARY);
    eKey     key       = eKey_Invalid;
    unsigned i         = 0;

    for (i = 0; i < sizeof(g_BluetoothRemoteCodes) / sizeof(g_BluetoothRemoteCodes[0]); i++)
    {
        if (code == g_BluetoothRemoteCodes[i].code)
        {
            key = g_BluetoothRemoteCodes[i].key;
        }
    }

    return(key);
} /* convertRemoteCode */

char * CBluetoothRemote::getRemoteCodeName(eKey key)
{
    int max = sizeof(g_BluetoothRemoteCodes)/sizeof(g_BluetoothRemoteCodes[0]);

    for (int i = 0; i < max; i++)
    {
        if (key == g_BluetoothRemoteCodes[i].key)
        {
            return(g_BluetoothRemoteCodes[i].strName);
        }
    }

    return(NULL);
}

void CBluetoothRemote::saveToPrevEvents(struct input_event event)
{
    prev_event.type  = event.type;
    prev_event.code  = event.code;
    prev_event.value = event.value;
}

bool CBluetoothRemote::comparePrevEvents(struct input_event event)
{
#if 0
    printf("comparePrevEvents():event type[%d] code[%d] value[%d]!\n",
            event.type, event.code, event.value);
    printf("comparePrevEvents():p_event type[%d] code[%d] value[%d]!\n",
            prev_event.type, prev_event.code, prev_event.value);
#endif /* if 0 */
    if ((event.type != prev_event.type) || (event.code != prev_event.code) || (event.value != prev_event.value))
    {
        return(0);
    }
    else
    {
        return(1);
    }
} /* comparePrevEvents */

eRet CBluetoothRemote::getEvent(
        CRemoteEvent *   pEvent,
        hidInputSource * pHidInputSource
        )
{
    eRet               ret = eRet_NotAvailable;
    struct input_event events[64];

    int readlen = 0;

    BDBG_ASSERT(NULL != pEvent);
    NetAppOSSemTake(bt_semaphore);

    readlen = read(pHidInputSource->fd, &events, sizeof(events));

    if (readlen <= 0)
    {
        BDBG_WRN(("%s(): Read Error from %d, error=%s! Device may have just went to sleep \n", __FUNCTION__, pHidInputSource->fd, strerror(errno)));
        NetAppOSSemGive(bt_semaphore);
        return(eRet_ExternalError);
    }
    else
    if ((unsigned)readlen < sizeof(events[0]))
    {
        NetAppOSSemGive(bt_semaphore);
        return(eRet_Busy);
    }

    for (unsigned int i = 0; i < (readlen / sizeof(events[0])); i++)
    {
        BDBG_MSG(("getEvent():[%s] type[%d] code[%d] value[%d]!\n",
                  _devName, events[i].type, events[i].code, events[i].value));

        switch (events[i].type)
        {
        case EV_KEY:
            switch (events[i].code)
            {
            case KEY_LEFTSHIFT:
            case KEY_RIGHTSHIFT:
                /* TODO */
                break;
            case BTN_LEFT:
            case BTN_MIDDLE:
            case BTN_RIGHT:
                /* TODO: */
                break;
            default:
            {
                eKey key = eKey_Invalid;
                if ((events[i].value == 1) && (comparePrevEvents(events[i]) == 0))
                {
                    key = convertRemoteCode(events[i].code);
                    if ((eKey_Invalid != key) && (eKey_Max != key))
                    {
                        BDBG_MSG(("key PRESS:%s", getRemoteCodeName(key)));
                    }
                    else
                    {
                        BDBG_WRN(("unhandled PRESS remote code (BLUETOOTH_REMOTE) 0x%0x", events[i].code));
                    }
                    saveToPrevEvents(events[i]);
                }
                else
                if (events[i].value == 2)
                {
                    key = convertRemoteCode(events[i].code);
                    if ((eKey_Invalid != key) && (eKey_Max != key))
                    {
                        BDBG_MSG(("key REPEAT:%s", getRemoteCodeName(key)));
                    }
                    else
                    {
                        BDBG_WRN(("unhandled REPEAT remote code (BLUETOOTH_REMOTE) 0x%0x", events[i].code));
                    }
                    saveToPrevEvents(events[i]);
                }
                else
                if ((events[i].value == 0) && (comparePrevEvents(events[i]) == 0))
                {
                    saveToPrevEvents(events[i]);

                    key = convertRemoteCode(events[i].code);
                    if ((eKey_Invalid != key) && (eKey_Max != key))
                    {
                        pEvent->setCode(key);
                        BDBG_WRN(("key RELEASE:%s", getRemoteCodeName(key)));
                        ret = eRet_Ok;
                    }
                    else
                    {
                        BDBG_WRN(("unhandled remote code (BLUETOOTH_REMOTE) 0x%0x", events[i].code));
                    }
                }
            }
            break;
            } /* switch */
            break;
        default:
            /*TODO: Only process key events for now */
            break;
        } /* switch */
    }
    NetAppOSSemGive(bt_semaphore);

    return(ret);
} /* getEvent */

/* don't call this version, but have to have it cause its virtual */
eRet CBluetoothRemote::getEvent(CRemoteEvent * pEvent)
{
    eRet               ret = eRet_NotAvailable;
    struct input_event events[64];
    int                readlen   = read(_fd, &events, sizeof(events));
    uint32_t           firstCode = 0;

    BDBG_ASSERT(NULL != pEvent);
    if (readlen <= 0)
    {
        BDBG_ERR(("%s(): Read Error from %d, error=%s!\n", __FUNCTION__, _fd, strerror(errno)));
        return(eRet_ExternalError);
    }
    else
    if ((unsigned)readlen < sizeof(events[0]))
    {
        return(eRet_Busy);
    }

    for (unsigned int i = 0; i < (readlen / sizeof(events[0])); i++)
    {
        BDBG_MSG(("getEvent():[%s] type[%d] code[%d] value[%d]!\n",
                  _devName, events[i].type, events[i].code, events[i].value));

        switch (events[i].type)
        {
        case EV_KEY:
            switch (events[i].code)
            {
            case KEY_LEFTSHIFT:
            case KEY_RIGHTSHIFT:
                /* TODO */
                break;
            case BTN_LEFT:
            case BTN_MIDDLE:
            case BTN_RIGHT:
                /* TODO: */
                break;
            default:
                if ((firstCode == 0) && (firstCode != events[i].code))
                {
                    eKey key = eKey_Invalid;
                    firstCode = events[i].code;
                    BDBG_MSG(("bluetooth remote event: %#x ", events[i].code));

                    key = convertRemoteCode(events[i].code);
                    if ((eKey_Invalid != key) && (eKey_Max != key))
                    {
                        pEvent->setCode(key);
                        BDBG_MSG(("key:%s", getRemoteCodeName(key)));
                    }
                    else
                    {
                        BDBG_WRN(("unhandled remote code (REMOTE_TYPE_SECONDARY:%s) 0x%0x", GET_STR(_pCfg, REMOTE_TYPE_SECONDARY), events[i].code));
                    }

                    ret = eRet_Ok;
                }
                break;
            } /* switch */
            break;
        default:
            /*TODO: Only process key events for now */
            break;
        } /* switch */
    }

    return(ret);
} /* getEvent */

#endif /* NETAPP_SUPPORT */

#if NEXUS_HAS_UHF_INPUT
static void nexusUhfCallback(
        void * context,
        int    param
        )
{
    eRet         ret       = eRet_Ok;
    CUhfRemote * uhfRemote = (CUhfRemote *)context;
    CRemoteEvent event;

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

CUhfRemote::CUhfRemote(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    CRemote(name, number, eBoardResource_uhfRemote, pCfg),
    _uhfInput(NULL),
    _channel(NEXUS_UhfInputMode_eMax)
{
    /* verify key mapping size matches */
    BDBG_ASSERT(eKey_Max == sizeof(eKey2bwidgets)/sizeof(bwidget_key));
}

eRet CUhfRemote::open(CWidgetEngine * pWidgetEngine)
{
    eRet                   ret = eRet_Ok;
    NEXUS_UhfInputSettings settings;

    BDBG_ASSERT(NULL != pWidgetEngine);
    BDBG_ASSERT(NEXUS_UhfInputMode_eMax != _channel);

    _pWidgetEngine = pWidgetEngine;
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->addCallback(this, CALLBACK_UHFREMOTE, bwinRemoteCallback, ePowerMode_S3);
    }

    NEXUS_UhfInput_GetDefaultSettings(&settings);
    settings.dataReady.callback = nexusUhfCallback;
    settings.dataReady.context  = this;
    settings.channel            = _channel;
    _uhfInput                   = NEXUS_UhfInput_Open(getNumber(), &settings);
    CHECK_PTR_ERROR_GOTO("nexus uhf input open failed!", _uhfInput, ret, eRet_ExternalError, error);

error:
    return(ret);
} /* open */

void CUhfRemote::close()
{
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_UHFREMOTE);
        _pWidgetEngine = NULL;
    }

    if (NULL != _uhfInput)
    {
        NEXUS_UhfInput_Close(_uhfInput);
        _uhfInput = NULL;
    }
}

CUhfRemote::~CUhfRemote()
{
    close();
}

eRet CUhfRemote::setChannel(NEXUS_UhfInputChannel channel)
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NEXUS_UhfInputMode_eMax > channel);
    _channel = channel;

    return(ret);
}

NEXUS_UhfInputChannel CUhfRemote::getChannel()
{
    return(_channel);
}

void CUhfRemote::dump()
{
    BDBG_MSG(("<%d>%s:%d",
              _type,
              _name.s(),
              _number));
}

struct g_UhfRemoteCodes
{
    eKey       key;
    uint32_t   codeEchoStar;
    uint32_t   codeDirectv;
    char       strName[32];
} g_UhfRemoteCodes[] =
{
    /* eKey             EchoStar    Directv     strName */
    { eKey_Invalid,     INVALID,    INVALID,    "eKey_Invalid"     },
    { eKey_Select,      0x0102033f, 0xbd79a25a, "eKey_Select"      },
    { eKey_UpArrow,     0x01a20200, 0xbd79a216, "eKey_UpArrow"     },
    { eKey_DownArrow,   0x01e201dc, 0xbd79a227, "eKey_DownArrow"   },
    { eKey_LeftArrow,   0x01c20032, 0xbd79a238, "eKey_LeftArrow"   },
    { eKey_RightArrow,  0x018203ee, 0xbd79a249, "eKey_RightArrow"  },
    { eKey_Backspace,   INVALID,    INVALID,    "eKey_Backspace"   },
    { eKey_Delete,      INVALID,    INVALID,    "eKey_Delete"      },
    { eKey_ChannelUp,   INVALID,    0xbd79a0d0, "eKey_ChannelUp"   },
    { eKey_ChannelDown, INVALID,    0xbd79a0e1, "eKey_ChannelDown" },
    { eKey_VolumeUp,    INVALID,    INVALID,    "eKey_VolumeUp"    },
    { eKey_VolumeDown,  INVALID,    INVALID,    "eKey_VolumeDown"  },
    { eKey_Mute,        INVALID,    0xbd79a591, "eKey_Mute"        },
    { eKey_Play,        0x0032408e, 0xbd79a306, "eKey_Play"        },
    { eKey_Stop,        0x0212012e, 0xbd79a317, "eKey_Stop"        },
    { eKey_Pause,       0x020201d9, 0xbd79a328, "eKey_Pause"       },
    { eKey_Rewind,      0x03124386, 0xbd79a339, "eKey_Rewind"      },
    { eKey_FastForward, 0x0322429f, 0xbd79a34a, "eKey_FastForward" },
    { eKey_Record,      0x01f2012b, 0xbd79a35b, "eKey_Record"      },
    { eKey_Menu,        0x00b20355, 0xbd79a205, "eKey_Menu"        },
    { eKey_Info,        0x0002029d, 0xbd79a2e3, "eKey_Info"        },
    { eKey_Exit,        INVALID,    0xbd79a26b, "eKey_Exit"        },
    { eKey_Dot,         0x025202f2, 0xbd79a126, "eKey_Dot"         },
    { eKey_Enter,       INVALID,    0xbd79a137, "eKey_Enter"       },
    { eKey_Last,        0x01b202f7, 0xbd79a0f2, "eKey_Last"        },
    { eKey_Pip,         0x03a2424e, 0xbd79a418, "eKey_Pip"         },
    { eKey_Swap,        0x03d2408b, 0xbd79a43a, "eKey_Swap"        },
    { eKey_JumpFwd,     0x037241b4, 0xbd79a37d, "eKey_JumpFwd"     },
    { eKey_JumpRev,     0x03624143, 0xbd79a36c, "eKey_JumpRev"     },
    { eKey_Power,       0x00220373, 0xbd79a104, "eKey_Power"       },
    /* ascii codes */
    { eKey_0,           0x011203c8, 0xbd79a115, "eKey_0"           },
    { eKey_1,           0x00420141, 0xbd79a014, "eKey_1"           },
    { eKey_2,           0x005201b6, 0xbd79a025, "eKey_2"           },
    { eKey_3,           0x006200af, 0xbd79a036, "eKey_3"           },
    { eKey_4,           0x0082024c, 0xbd79a047, "eKey_4"           },
    { eKey_5,           0x009202bb, 0xbd79a058, "eKey_5"           },
    { eKey_6,           0x00a203a2, 0xbd79a069, "eKey_6"           },
    { eKey_7,           0x00c20190, 0xbd79a07a, "eKey_7"           },
    { eKey_8,           0x00d20167, 0xbd79a08b, "eKey_8"           },
    { eKey_9,           0x00e2007e, 0xbd79a09c, "eKey_9"           },
    { eKey_Max,         INVALID,    INVALID,    "eKey_Max"         }
};

eKey CUhfRemote::convertRemoteCode(uint32_t code)
{
    MString  strRemote = GET_STR(_pCfg, REMOTE_TYPE_SECONDARY);
    eKey     key       = eKey_Invalid;
    unsigned i         = 0;

    for (i = 0; i < sizeof(g_UhfRemoteCodes) / sizeof(g_UhfRemoteCodes[0]); i++)
    {
        if (strRemote.lower() == "echostaruhf")
        {
            if (code == g_UhfRemoteCodes[i].codeEchoStar)
            {
                key = g_UhfRemoteCodes[i].key;
            }
        }
        else
        if (strRemote.lower() == "directvuhf")
        {
            if (code == g_UhfRemoteCodes[i].codeDirectv)
            {
                key = g_UhfRemoteCodes[i].key;
            }
        }
    }

    return(key);
} /* convertRemoteCode */

char * CUhfRemote::getRemoteCodeName(eKey key)
{
    int max = sizeof(g_UhfRemoteCodes)/sizeof(g_UhfRemoteCodes[0]);

    for (int i = 0; i < max; i++)
    {
        if (key == g_UhfRemoteCodes[i].key)
        {
            return(g_UhfRemoteCodes[i].strName);
        }
    }

    return(NULL);
}

eRet CUhfRemote::getEvent(CRemoteEvent * pEvent)
{
    eRet                ret    = eRet_NotAvailable;
    NEXUS_Error         nerror = NEXUS_SUCCESS;
    NEXUS_UhfInputEvent uhfEvent;
    unsigned            num;
    bool                overflow;

    BDBG_ASSERT(NULL != pEvent);

    nerror = NEXUS_UhfInput_GetEvents(getUhfRemote(), &uhfEvent, 1, &num, &overflow);
    /* do not buffer any remote events */
    NEXUS_UhfInput_FlushEvents(getUhfRemote());

    if (!nerror && num)
    {
        eKey key = eKey_Invalid;

        BDBG_MSG(("uhf event: %#x repeat:%d", uhfEvent.code, uhfEvent.repeat));
        pEvent->setRepeat(uhfEvent.repeat);

        key = convertRemoteCode(uhfEvent.code);
        if ((eKey_Invalid != key) && (eKey_Max != key))
        {
            pEvent->setCode(key);
            BDBG_MSG(("key:%s", getRemoteCodeName(key)));
        }
        else
        {
            BDBG_WRN(("unhandled remote code (REMOTE_TYPE_SECONDARY:%s) 0x%0x", GET_STR(_pCfg, REMOTE_TYPE_SECONDARY), uhfEvent.code));
        }

        ret = eRet_Ok;
    }

    return(ret);
} /* getEvent */

#endif /* if NEXUS_HAS_UHF_INPUT */
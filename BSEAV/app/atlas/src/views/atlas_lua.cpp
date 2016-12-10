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
#include "atlas_lua.h"
#include "string.h"

#if NEXUS_HAS_FRONTEND
#include "tuner_qam.h"
#include "tuner_vsb.h"
#include "tuner_sat.h"
#include "tuner_ofdm.h"
#endif /* if NEXUS_HAS_FRONTEND */

#ifdef DCC_SUPPORT
#include "closed_caption.h"
#endif
#include "channel.h"
#include "channel_bip.h"
#include "channelmgr.h"
#include "playback.h"
#include "record.h"
#include "remote.h"
#include "power.h"
#include "model.h"
#include "audio_decode.h"
#include "network.h"
#ifdef NETAPP_SUPPORT
#include "bluetooth.h"
#endif
#include "playlist.h"

#ifdef LINENOISE_SUPPORT
extern "C" {
#include "linenoise.h"
}

#include "mlist.h"
#include "mstring.h"
#include "ctype.h"
#endif /* ifdef LINENOISE_SUPPORT */

#define CALLBACK_LUA  "CallbackLua"

/* push return parameter to lua stack and return number of return parameters */
#define LUA_RETURN(ret)                                      \
    do {                                                     \
        lua_pushnumber(pLua, (eRet_Ok == ret) ? 0 : -1);     \
        return (pThis->getBusyAction()->getNumReturnVals()); \
    } while (0)

BDBG_MODULE(atlas_lua);

/* if lua error, print lua error and trigger jump (note that calling luaL_error() will jump. err parameter is just to satisfy coverity */
#define LUA_ERROR(lua, str, err)                   \
    do                                             \
    {                                              \
        ATLAS_LUA_ERROR((str), eRet_InvalidState); \
        goto err;                                  \
    } while (0)

#define DEFAULT_LUA_EVENT_TIMEOUT  10000

/* name of the atlas lua extension library.  atlas specific commands in lua will have the "atlas." prefix. */
#define LUA_ATLASLIBNAME           "atlas"
#define LUA_DIRLIBNAME             "atlasDir"

/* the address of this variable is used as a unique key to store data in the lua registry */
static const char LUAKEY_CLUA = 'k';

static CLua * getCLua(lua_State * pLua)
{
    CLua * pThis = NULL;

    BDBG_ASSERT(NULL != pLua);

    lua_pushlightuserdata(pLua, (void *)&LUAKEY_CLUA);
    lua_gettable(pLua, LUA_REGISTRYINDEX);
    pThis = (CLua *)lua_topointer(pLua, -1);

    return(pThis);
}

static int atlasLua_ChannelUp(lua_State * pLua)
{
    CLua *    pThis   = getCLua(pLua);
    CAction * pAction = NULL;
    eRet      err     = eRet_Ok;

    BDBG_ASSERT(pThis);

    pAction = new CAction(eNotify_ChUp, eNotify_VideoSourceChanged, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* for audio only channels */
    pAction->addWaitNotification(eNotify_AudioSourceChanged);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_ChannelUp */

static int atlasLua_ChannelDown(lua_State * pLua)
{
    CLua *    pThis   = getCLua(pLua);
    CAction * pAction = NULL;
    eRet      err     = eRet_Ok;

    BDBG_ASSERT(pThis);

    pAction = new CAction(eNotify_ChDown, eNotify_VideoSourceChanged, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* for audio only channels */
    pAction->addWaitNotification(eNotify_AudioSourceChanged);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_ChannelDown */

/* atlas.channelTune(
 *      --- required ---
 *      channel number,   logical channel number to tune to
 */
static int atlasLua_ChannelTune(lua_State * pLua)
{
    CLua *         pThis        = getCLua(pLua);
    eRet           err          = eRet_Ok;
    uint8_t        argNum       = 1;
    uint8_t        numArgTotal  = lua_gettop(pLua) - 1;
    char *         pChannelNum  = NULL;
    CChannelData * pChannelData = NULL;

    CDataAction <CChannelData> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if ((1 > numArgTotal) || (2 < numArgTotal))
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [channelNum]<tuner index>", error);
    }

    /* get arguments */
    pChannelNum  = (char *)luaL_checkstring(pLua, argNum++);
    pChannelData = new CChannelData(pChannelNum);

    /* error checking for arguments */
    if (NULL == pChannelData)
    {
        LUA_ERROR(pLua, "unable to allocate channel data", error);
    }

    /* add optional arguments to tune data */
    if (2 <= numArgTotal)
    {
        pChannelData->_tunerIndex = luaL_checknumber(pLua, argNum++);
    }

    /* create lua action and give it data */
    pAction = new CDataAction <CChannelData>(eNotify_Tune, pChannelData, eNotify_VideoSourceChanged, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* for audio only channels */
    pAction->addWaitNotification(eNotify_AudioSourceChanged);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pChannelData);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_ChannelTune */

/* atlas.channelUnTune(
 *      --- required ---
 *      channel number,   logical channel number to untune to
 */
static int atlasLua_ChannelUnTune(lua_State * pLua)
{
    CLua * pThis = getCLua(pLua);
    eRet   err   = eRet_Ok;

    CDataAction <CChannelData> * pAction = NULL;

    BDBG_ASSERT(pThis);

    CChannelData * pChannelData = new CChannelData("-1");

    /* error checking for arguments */
    if (NULL == pChannelData)
    {
        LUA_ERROR(pLua, "unable to allocate channel data", error);
    }

    /* create lua action and give it data */
    pAction = new CDataAction <CChannelData>(eNotify_Tune, pChannelData, eNotify_CurrentChannelNull, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pChannelData);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_ChannelUnTune */

#if NEXUS_HAS_FRONTEND
/* atlas.scanQam(
 *      --- required ---
 *      start frequency,   frequency in Hz at which to begin scanning
 *      end frequency  ,   frequency in Hz at which to end scanning
 *      bandwidth      ,   step in Hz to increment
 *      --- optional ---
 *      append = false,    if true, add found channels to channel list.  otherwise replace channel list.
 *      mode   = 4,
 *      annex  = 1,
 *      symbolRateMin = 0,  if not set, symbol rate will be determined based on mode
 *      symbolRateMax = 0,  if not set, symbol rate will be determined based on mode)
 */
static int atlasLua_ScanQam(lua_State * pLua)
{
    CLua *              pThis        = getCLua(pLua);
    eRet                err          = eRet_Ok;
    uint8_t             argNum       = 1;
    uint8_t             numArgTotal  = lua_gettop(pLua) - 1;
    uint32_t            startFreq    = 0;
    uint32_t            endFreq      = 0;
    int                 bandwidth    = 0;
    CTunerQamScanData * pQamScanData = NULL;

    CDataAction <CTunerQamScanData> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if ((3 > numArgTotal) || (9 < numArgTotal))
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [startFreq][endFreq][bandwidth]<stepFreq><append><mode><annex><symbolRateMin><symbolRateMax>", error);
    }

    /* get arguments */
    startFreq = luaL_checknumber(pLua, argNum++);
    endFreq   = luaL_checknumber(pLua, argNum++);
    bandwidth = luaL_checknumber(pLua, argNum++);

    /* error checking for arguments */
    if (startFreq > endFreq)
    {
        LUA_ERROR(pLua, "start frequency must be less than end frequency", error);
    }
    if (bandwidth < 1000000)
    {
        /* dtt todo: upper bound? */
        LUA_ERROR(pLua, "bandwidth must be > 1000000", error);
    }

    pQamScanData = new CTunerQamScanData();

    /* add required arguments to scan data */
    pQamScanData->_bandwidth = bandwidth;
    pQamScanData->_stepFreq  = bandwidth;

    /* add optional arguments to scan data */
    if (4 <= numArgTotal)
    {
        pQamScanData->_stepFreq = luaL_checknumber(pLua, argNum++);
    }
    if (5 <= numArgTotal)
    {
        pQamScanData->_appendToChannelList = lua_toboolean(pLua, argNum++);
    }
    if (6 <= numArgTotal)
    {
        pQamScanData->_mode = luaL_checknumber(pLua, argNum++);
    }
    if (7 <= numArgTotal)
    {
        int annexNum = luaL_checknumber(pLua, argNum++);
        if (7 < annexNum)
        {
            LUA_ERROR(pLua, "annex must be sum of A=1, B=2, C=4 combination", error);
        }

        if (4 <= annexNum)
        {
            pQamScanData->_annex[NEXUS_FrontendQamAnnex_eC] = true;
            annexNum -= 4;
        }
        if (2 <= annexNum)
        {
            pQamScanData->_annex[NEXUS_FrontendQamAnnex_eB] = true;
            annexNum -= 2;
        }
        if (1 <= annexNum)
        {
            pQamScanData->_annex[NEXUS_FrontendQamAnnex_eA] = true;
            annexNum -= 1;
        }
        BDBG_ASSERT(0 == annexNum);
    }
    if (8 <= numArgTotal)
    {
        pQamScanData->_symbolRateMin = luaL_checknumber(pLua, argNum++);
    }
    if (9 <= numArgTotal)
    {
        pQamScanData->_symbolRateMax = luaL_checknumber(pLua, argNum++);
    }

    /* create list of freq to scan */
    for (uint32_t freq = startFreq; freq <= endFreq; freq += pQamScanData->_stepFreq)
    {
        pQamScanData->_freqList.add(new uint32_t(freq));
    }

    /* test print out requested frequencies */
    pQamScanData->dump();

    /* create lua action and give it data */
    pAction = new CDataAction <CTunerQamScanData>(eNotify_ScanStart, pQamScanData, eNotify_ScanStopped, B_WAIT_FOREVER);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pQamScanData);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_ScanQam */

/* atlas.scanVsb(
 *      --- required ---
 *      start frequency,   frequency in Hz at which to begin scanning
 *      end frequency  ,   frequency in Hz at which to end scanning
 *      bandwidth      ,   step in Hz to increment
 *      --- optional ---
 *      append = false,    if true, add found channels to channel list.  otherwise replace channel list.
 *      mode   = 0)
 */
static int atlasLua_ScanVsb(lua_State * pLua)
{
    CLua *              pThis        = getCLua(pLua);
    eRet                err          = eRet_Ok;
    uint8_t             argNum       = 1;
    uint8_t             numArgTotal  = lua_gettop(pLua) - 1;
    uint32_t            startFreq    = 0;
    uint32_t            endFreq      = 0;
    int                 bandwidth    = 0;
    CTunerVsbScanData * pVsbScanData = NULL;

    CDataAction <CTunerVsbScanData> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if ((3 > numArgTotal) || (6 < numArgTotal))
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [startFreq][endFreq][bandwidth]<stepFreq><append><mode>", error);
    }

    /* get arguments */
    startFreq = luaL_checknumber(pLua, argNum++);
    endFreq   = luaL_checknumber(pLua, argNum++);
    bandwidth = luaL_checknumber(pLua, argNum++);

    /* error checking for arguments */
    if (startFreq > endFreq)
    {
        LUA_ERROR(pLua, "start frequency must be less than end frequency", error);
    }
    if (bandwidth < 1000000)
    {
        /* dtt todo: upper bound? */
        LUA_ERROR(pLua, "bandwidth must be > 1000000", error);
    }

    pVsbScanData = new CTunerVsbScanData();

    /* add required arguments to scan data */
    pVsbScanData->_bandwidth = bandwidth;
    pVsbScanData->_stepFreq  = bandwidth;

    /* add optional arguments to scan data */
    if (4 <= numArgTotal)
    {
        pVsbScanData->_stepFreq = luaL_checknumber(pLua, argNum++);
    }
    if (5 <= numArgTotal)
    {
        pVsbScanData->_appendToChannelList = lua_toboolean(pLua, argNum++);
    }
    if (6 <= numArgTotal)
    {
        pVsbScanData->_mode = luaL_checknumber(pLua, argNum++);
    }

    /* create list of freq to scan */
    for (uint32_t freq = startFreq; freq <= endFreq; freq += pVsbScanData->_stepFreq)
    {
        pVsbScanData->_freqList.add(new uint32_t(freq));
    }

    /* test print out requested frequencies */
    pVsbScanData->dump();

    /* create lua action and give it data */
    pAction = new CDataAction <CTunerVsbScanData>(eNotify_ScanStart, pVsbScanData, eNotify_ScanStopped, B_WAIT_FOREVER);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pVsbScanData);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_ScanVsb */

/* atlas.scanSat(
 *      --- required ---
 *      start frequency,   frequency in MHz at which to begin scanning
 *      end frequency  ,   frequency in MHz at which to end scanning
 *      bandwidth      ,   step in MHz to increment
 *      --- optional ---
 *      append = false)    if true, add found channels to channel list.  otherwise replace channel list.
 */
static int atlasLua_ScanSat(lua_State * pLua)
{
    CLua *              pThis        = getCLua(pLua);
    eRet                err          = eRet_Ok;
    uint8_t             argNum       = 1;
    uint8_t             numArgTotal  = lua_gettop(pLua) - 1;
    uint32_t            startFreq    = 0;
    uint32_t            endFreq      = 0;
    int                 bandwidth    = 0;
    CTunerSatScanData * pSatScanData = NULL;

    CDataAction <CTunerSatScanData> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if ((3 > numArgTotal) || (6 < numArgTotal))
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [startFreq][endFreq][bandwidth]<stepFreq><append><adc>", error);
    }

    /* get arguments */
    startFreq = luaL_checknumber(pLua, argNum++);
    endFreq   = luaL_checknumber(pLua, argNum++);
    bandwidth = luaL_checknumber(pLua, argNum++);

    /* error checking for arguments */
    if (startFreq > endFreq)
    {
        LUA_ERROR(pLua, "start frequency must be less than end frequency", error);
    }
#if 0
    if (bandwidth < 1000000)
    {
        /* dtt todo: upper bound? */
        LUA_ERROR(pLua, "bandwidth must be > 1000000", error);
    }
#endif /* if 0 */
    pSatScanData = new CTunerSatScanData();

    /*
     * add required arguments to scan data
     * pSatScanData->_bandwidth = bandwidth;
     */
    pSatScanData->_stepFreq = bandwidth;

    /* add optional arguments to scan data */
    if (4 <= numArgTotal)
    {
        pSatScanData->_stepFreq = luaL_checknumber(pLua, argNum++);
    }
    if (5 <= numArgTotal)
    {
        pSatScanData->_appendToChannelList = lua_toboolean(pLua, argNum++);
    }
    if (6 <= numArgTotal)
    {
        pSatScanData->_adc = luaL_checknumber(pLua, argNum++);
    }

    /* create list of freq to scan */
    for (uint32_t freq = startFreq; freq <= endFreq; freq += pSatScanData->_stepFreq)
    {
        pSatScanData->_freqList.add(new uint32_t(freq));
    }

    /* test print out requested frequencies */
    pSatScanData->dump();

    /* create lua action and give it data */
    pAction = new CDataAction <CTunerSatScanData>(eNotify_ScanStart, pSatScanData, eNotify_ScanStopped, B_WAIT_FOREVER);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pSatScanData);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_ScanSat */

/* atlas.scanOfdm(
 *      --- required ---
 *      start frequency,   frequency in Hz at which to begin scanning
 *      end frequency  ,   frequency in Hz at which to end scanning
 *      bandwidth      ,   step in Hz to increment
 *      --- optional ---
 *      append = false,    if true, add found channels to channel list.  otherwise replace channel list.
 *      mode   = 2)
 */
static int atlasLua_ScanOfdm(lua_State * pLua)
{
    CLua *               pThis         = getCLua(pLua);
    eRet                 err           = eRet_Ok;
    uint8_t              argNum        = 1;
    uint8_t              numArgTotal   = lua_gettop(pLua) - 1;
    uint32_t             startFreq     = 0;
    uint32_t             endFreq       = 0;
    int                  bandwidth     = 0;
    CTunerOfdmScanData * pOfdmScanData = NULL;

    CDataAction <CTunerOfdmScanData> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if ((3 > numArgTotal) || (6 < numArgTotal))
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [startFreq][endFreq][bandwidth]<stepFreq><append><mode><annex>", error);
    }

    /* get arguments */
    startFreq = luaL_checknumber(pLua, argNum++);
    endFreq   = luaL_checknumber(pLua, argNum++);
    bandwidth = luaL_checknumber(pLua, argNum++);

    /* error checking for arguments */
    if (startFreq > endFreq)
    {
        LUA_ERROR(pLua, "start frequency must be less than end frequency", error);
    }
    if (bandwidth < 1000000)
    {
        /* dtt todo: upper bound? */
        LUA_ERROR(pLua, "bandwidth must be > 1000000", error);
    }

    pOfdmScanData = new CTunerOfdmScanData();

    /* add required arguments to scan data */
    pOfdmScanData->_bandwidth = bandwidth;
    pOfdmScanData->_stepFreq  = bandwidth;

    /* add optional arguments to scan data */
    if (4 <= numArgTotal)
    {
        pOfdmScanData->_stepFreq = luaL_checknumber(pLua, argNum++);
    }
    if (5 <= numArgTotal)
    {
        pOfdmScanData->_appendToChannelList = lua_toboolean(pLua, argNum++);
    }
    if (6 <= numArgTotal)
    {
        pOfdmScanData->_mode = luaL_checknumber(pLua, argNum++);
    }

    /* create list of freq to scan */
    for (uint32_t freq = startFreq; freq <= endFreq; freq += pOfdmScanData->_stepFreq)
    {
        pOfdmScanData->_freqList.add(new uint32_t(freq));
    }

    /* test print out requested frequencies */
    pOfdmScanData->dump();

    /* create lua action and give it data */
    pAction = new CDataAction <CTunerOfdmScanData>(eNotify_ScanStart, pOfdmScanData, eNotify_ScanStopped, B_WAIT_FOREVER);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pOfdmScanData);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_ScanOfdm */

#endif /* NEXUS_HAS_FRONTEND */

/* atlas.setContentMode(mode)
 *      --- required ---
 *      mode,   NEXUS_VideoWindowContentMode type to switch to [zoom:0 box:1 pan/scan:2 full:3 stretch:4 pan/scanNoA/R:5]
 *      --- optional ---
 *      none
 */
static int atlasLua_SetContentMode(lua_State * pLua)
{
    CLua *   pThis       = getCLua(pLua);
    eRet     err         = eRet_Ok;
    uint8_t  argNum      = 1;
    uint8_t  numArgTotal = lua_gettop(pLua) - 1;
    uint32_t contentMode = 0;

    CDataAction <NEXUS_VideoWindowContentMode> * pAction      = NULL;
    NEXUS_VideoWindowContentMode *               pContentMode = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [zoom:0 box:1 pan/scan:2 full:3 stretch:4 pan/scanNoA/R:5]", error);
    }

    /* get arguments */
    contentMode = luaL_checknumber(pLua, argNum++);

    /* error checking for arguments */
    if (NEXUS_VideoWindowContentMode_eMax <= contentMode)
    {
        LUA_ERROR(pLua, "Invalid video window content mode [zoom:0 box:1 pan/scan:2 full:3 stretch:4 pan/scanNoA/R:5]", error);
    }

    pContentMode = new NEXUS_VideoWindowContentMode;

    /* add required arguments to video format data */

    *pContentMode = (NEXUS_VideoWindowContentMode)contentMode;

    /* create lua action and give it data data */
    pAction = new CDataAction <NEXUS_VideoWindowContentMode>(eNotify_SetContentMode, pContentMode, eNotify_ContentModeChanged, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pContentMode);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SetContentMode */

/* atlas.setColorSpace(
 *      --- required ---
 *      color space type,   NEXUS color space type to switch to
 *      --- optional ---
 *      none
 */
static int atlasLua_SetColorSpace(lua_State * pLua)
{
    CLua *   pThis       = getCLua(pLua);
    eRet     err         = eRet_Ok;
    uint8_t  argNum      = 1;
    uint8_t  numArgTotal = lua_gettop(pLua) - 1;
    uint32_t formatType  = 0;

    CDataAction <NEXUS_ColorSpace> * pAction     = NULL;
    NEXUS_ColorSpace *               pColorSpace = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [colorSpace]", error);
    }

    /* get arguments */
    formatType = luaL_checknumber(pLua, argNum++);

    /* error checking for arguments */
    if (NEXUS_ColorSpace_eMax <= formatType)
    {
        LUA_ERROR(pLua, "Invalid color space type", error);
    }

    pColorSpace = new NEXUS_ColorSpace;

    /* add required arguments to video format data */

    *pColorSpace = (NEXUS_ColorSpace)formatType;

    /* create lua action and give it data */
    pAction = new CDataAction <NEXUS_ColorSpace>(eNotify_SetColorSpace, pColorSpace, eNotify_ColorSpaceChanged, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pColorSpace);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SetColorSpace */

/* atlas.setMpaaDecimation(
 *      --- required ---
 *      mpaa decimation state,   if true, turn on mpaa decimation
 *      --- optional ---
 *      none
 */
static int atlasLua_SetMpaaDecimation(lua_State * pLua)
{
    CLua *  pThis           = getCLua(pLua);
    eRet    err             = eRet_Ok;
    uint8_t argNum          = 1;
    uint8_t numArgTotal     = lua_gettop(pLua) - 1;
    bool    mpaaDecimation  = false;
    bool *  pMpaaDecimation = NULL;

    CDataAction <bool> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [bool]", error);
    }

    /* get arguments */
    mpaaDecimation  = lua_toboolean(pLua, argNum++);
    pMpaaDecimation = new bool;

    /* add required arguments to mpaa decimation data */
    *pMpaaDecimation = mpaaDecimation;

    /* create lua action and give it data */
    pAction = new CDataAction <bool>(eNotify_SetMpaaDecimation, pMpaaDecimation, eNotify_MpaaDecimationChanged, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pMpaaDecimation);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SetMpaaDecimation */

/* atlas.setDeinterlacer(
 *      --- required ---
 *      deinterlacer state,   if true, turn on MAD deinterlacer
 *      --- optional ---
 *      none
 */
static int atlasLua_SetDeinterlacer(lua_State * pLua)
{
    CLua *  pThis         = getCLua(pLua);
    eRet    err           = eRet_Ok;
    uint8_t argNum        = 1;
    uint8_t numArgTotal   = lua_gettop(pLua) - 1;
    bool    deinterlacer  = false;
    bool *  pDeinterlacer = NULL;

    CDataAction <bool> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [bool]", error);
    }

    /* get arguments */
    deinterlacer  = lua_toboolean(pLua, argNum++);
    pDeinterlacer = new bool;

    /* add required arguments to deinterlacer data */
    *pDeinterlacer = deinterlacer;

    /* create lua action and give it data */
    pAction = new CDataAction <bool>(eNotify_SetDeinterlacer, pDeinterlacer, eNotify_DeinterlacerChanged, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pDeinterlacer);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SetDeinterlacer */

/* atlas.setBoxDetect(
 *      --- required ---
 *      BoxDetect state,   if true, turn on BoxDetect
 *      --- optional ---
 *      none
 */
static int atlasLua_SetBoxDetect(lua_State * pLua)
{
    CLua *  pThis       = getCLua(pLua);
    eRet    err         = eRet_Ok;
    uint8_t argNum      = 1;
    uint8_t numArgTotal = lua_gettop(pLua) - 1;
    bool    boxDetect   = false;
    bool *  pBoxDetect  = NULL;

    CDataAction <bool> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [bool]", error);
    }

    /* get arguments */
    boxDetect = lua_toboolean(pLua, argNum++);

    /* check if selection is valid and available */
    {
        CBoardFeatures * pFeatures = pThis->getConfig()->getBoardFeatures();
        if (0 == pFeatures->_boxDetect)
        {
            LUA_ERROR(pLua, "Letterbox Detection not supported on this platform", error);
        }
    }

    pBoxDetect = new bool;

    /* add required arguments to BoxDetect data */
    *pBoxDetect = boxDetect;

    /* create lua action and give it data */
    pAction = new CDataAction <bool>(eNotify_SetBoxDetect, pBoxDetect, eNotify_BoxDetectChanged, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pBoxDetect);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SetBoxDetect */

/* atlas.setAspectRatio(
 *      --- required ---
 *      aspect ratio type,   NEXUS aspect ratio type to switch to
 *      --- optional ---
 *      none
 */
static int atlasLua_SetAspectRatio(lua_State * pLua)
{
    CLua *              pThis        = getCLua(pLua);
    eRet                err          = eRet_Ok;
    uint8_t             argNum       = 1;
    uint8_t             numArgTotal  = lua_gettop(pLua) - 1;
    uint32_t            formatType   = 0;
    NEXUS_AspectRatio * pAspectRatio = NULL;

    CDataAction <NEXUS_AspectRatio> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [aspectRatio]", error);
    }

    /* get arguments */
    formatType = luaL_checknumber(pLua, argNum++);

    /* error checking for arguments */
    if (NEXUS_AspectRatio_eMax <= formatType)
    {
        LUA_ERROR(pLua, "Invalid aspect ratio type", error);
    }

    pAspectRatio = new NEXUS_AspectRatio;

    /* add required arguments to video format data */

    *pAspectRatio = (NEXUS_AspectRatio)formatType;

    /* create lua action and give it data */
    pAction = new CDataAction <NEXUS_AspectRatio>(eNotify_SetAspectRatio, pAspectRatio, eNotify_AspectRatioChanged, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pAspectRatio);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SetAspectRatio */

/* atlas.setVideoFormat(
 *      --- required ---
 *      video format type,   NEXUS video format type to switch to
 *      --- optional ---
 *      none
 */
static int atlasLua_SetVideoFormat(lua_State * pLua)
{
    CLua *              pThis        = getCLua(pLua);
    eRet                err          = eRet_Ok;
    uint8_t             argNum       = 1;
    uint8_t             numArgTotal  = lua_gettop(pLua) - 1;
    uint32_t            nFormatType  = NEXUS_VideoFormat_eMax;
    NEXUS_VideoFormat * pVideoFormat = NULL;

    CDataAction <NEXUS_VideoFormat> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [formatType(number or string name)]", error);
    }

    /* get arguments */

    /* we will accept both a number or a string for the format parameter.
     * the number corresponds to the NEXUS_VideoFormat value.
     * the string will be converted to the NEXUS_VideoFormat value automatically. */
    {
        const char * pFormatType;

        if (1 == lua_isnumber(pLua, argNum))
        {
            /* arg is number */
            nFormatType = luaL_checknumber(pLua, argNum);
        }
        else
        {
            /* arg is string */
            pFormatType = luaL_checkstring(pLua, argNum);
            nFormatType = stringToVideoFormat(pFormatType);
        }
        argNum++;
    }

    /* check format type to see if number */

    /* error checking for arguments */
    if ((NEXUS_VideoFormat_eMax <= nFormatType) || (NEXUS_VideoFormat_eUnknown == nFormatType))
    {
        LUA_ERROR(pLua, "Invalid video format type", error);
    }

    pVideoFormat = new NEXUS_VideoFormat;

    /* add required arguments to video format data */

    *pVideoFormat = (NEXUS_VideoFormat)nFormatType;

    /* create lua action and give it data */
    pAction = new CDataAction <NEXUS_VideoFormat>(eNotify_SetVideoFormat, pVideoFormat, eNotify_VideoFormatChanged, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pVideoFormat);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SetVideoFormat */

/* atlas.setAutoVideoFormat(
 *      --- required ---
 *      auto format state,   if true, turn on auto format
 *      --- optional ---
 *      none
 */
static int atlasLua_SetAutoVideoFormat(lua_State * pLua)
{
    CLua *  pThis            = getCLua(pLua);
    eRet    err              = eRet_Ok;
    uint8_t argNum           = 1;
    uint8_t numArgTotal      = lua_gettop(pLua) - 1;
    bool    autoFormat       = false;
    bool *  pAutoVideoFormat = NULL;

    CDataAction <bool> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [bool]", error);
    }

    /* get arguments */
    autoFormat       = lua_toboolean(pLua, argNum++);
    pAutoVideoFormat = new bool;

    /* add required arguments to auto format data */
    *pAutoVideoFormat = autoFormat;

    /* create lua action and give it data */
    pAction = new CDataAction <bool>(eNotify_SetAutoVideoFormat, pAutoVideoFormat, eNotify_AutoVideoFormatChanged, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pAutoVideoFormat);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SetAutoVideoFormat */

/* atlas.channelListLoad(
 *      --- optional ---
 *      fileName,          filename containing list
 *      listName,          name of channel list in fileName
 *      append = false)    if true, add new channel list to current channel list.  otherwise replace current channel list
 */
static int atlasLua_ChannelListLoad(lua_State * pLua)
{
    CLua *                    pThis         = getCLua(pLua);
    eRet                      err           = eRet_Ok;
    uint8_t                   argNum        = 1;
    uint8_t                   numArgTotal   = lua_gettop(pLua) - 1;
    CChannelMgrLoadSaveData * pLoadSaveData = NULL;

    CDataAction <CChannelMgrLoadSaveData> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (3 < numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [fileName][listName]]<append>", error);
    }

    /* add required arguments to scan data */
    pLoadSaveData = new CChannelMgrLoadSaveData();

    /* add optional arguments to scan data */
    if (1 <= numArgTotal)
    {
        const char * strFileName = luaL_checkstring(pLua, argNum++);
        if (NULL != strFileName)
        {
            pLoadSaveData->_strFileName = strFileName;
        }
    }
    if (2 <= numArgTotal)
    {
        const char * strListName = luaL_checkstring(pLua, argNum++);
        if (NULL != strListName)
        {
            pLoadSaveData->_strListName = strListName;
        }
    }
    if (3 <= numArgTotal)
    {
        pLoadSaveData->_append = lua_toboolean(pLua, argNum++);
    }

    /* create lua action and give it data */
    pAction = new CDataAction <CChannelMgrLoadSaveData>(eNotify_ChannelListLoad, pLoadSaveData, eNotify_ChannelListChanged, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pLoadSaveData);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_ChannelListLoad */

/* atlas.channelListSave(
 *      --- optional ---
 *      fileName,          filename to save list to
 *      listName,          name of channel list
 *      append = false)    if true, add add new list to end of file.  otherwise, overwrite file
 */
static int atlasLua_ChannelListSave(lua_State * pLua)
{
    CLua *                    pThis         = getCLua(pLua);
    eRet                      err           = eRet_Ok;
    uint8_t                   argNum        = 1;
    uint8_t                   numArgTotal   = lua_gettop(pLua) - 1;
    CChannelMgrLoadSaveData * pLoadSaveData = NULL;

    CDataAction <CChannelMgrLoadSaveData> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (3 < numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [fileName][listName]]<append>", error);
    }

    /* add required arguments to scan data */
    pLoadSaveData = new CChannelMgrLoadSaveData();

    /* add optional arguments to scan data */
    if (1 <= numArgTotal)
    {
        const char * strFileName = luaL_checkstring(pLua, argNum++);
        if (NULL != strFileName)
        {
            pLoadSaveData->_strFileName = strFileName;
        }
    }
    if (2 <= numArgTotal)
    {
        const char * strListName = luaL_checkstring(pLua, argNum++);
        if (NULL != strListName)
        {
            pLoadSaveData->_strListName = strListName;
        }
    }
    if (3 <= numArgTotal)
    {
        pLoadSaveData->_append = lua_toboolean(pLua, argNum++);
    }

    /*
     * create lua action and give it data
     * note that we are not setting a wait notification so we will not wait for this command to complete
     */
    pAction = new CDataAction <CChannelMgrLoadSaveData>(eNotify_ChannelListSave, pLoadSaveData);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pLoadSaveData);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_ChannelListSave */

/* atlas.channelListDump()
 */
static int atlasLua_ChannelListDump(lua_State * pLua)
{
    CLua * pThis = getCLua(pLua);
    eRet   err   = eRet_Ok;

    BDBG_ASSERT(pThis);

    /* note that we are not setting a wait notification so we will not wait for this command to complete */
    CAction * pAction = new CAction(eNotify_ChannelListDump);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_ChannelListDump */

/* atlas.playbackListDump()
 */
static int atlasLua_PlaybackListDump(lua_State * pLua)
{
    eRet   err   = eRet_Ok;
    CLua * pThis = getCLua(pLua);

    BDBG_ASSERT(pThis);

    /* note that we are not setting a wait notification so we will not wait for this command to complete */
    CAction * pAction = new CAction(eNotify_PlaybackListDump);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_PlaybackListDump */

/* atlas.playbackTrickMode(
 *      --- optional ---
 *      fileName,          filename to save list to
 *      indexName,          name of channel list
 *      path = NULL)    if set, use the path give to find file otherwise use /data/videos
 */
static int atlasLua_PlaybackTrickMode(lua_State * pLua)
{
    CLua *  pThis       = getCLua(pLua);
    eRet    err         = eRet_Ok;
    uint8_t argNum      = 1;
    uint8_t numArgTotal = lua_gettop(pLua) - 1;
    MString strTrickMode;

    CDataAction <CPlaybackTrickData> * pAction            = NULL;
    CPlaybackTrickData *               pPlaybackTrickData = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (2 < numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [fileName]<Path>", error);
    }

    /* add required arguments to scan data */
    pPlaybackTrickData = new CPlaybackTrickData();
    /* add optional arguments to scan data */
    if (1 <= numArgTotal)
    {
        strTrickMode = luaL_checkstring(pLua, argNum++);
    }

    if ((numArgTotal < 1) || (strTrickMode == "help"))
    {
        BDBG_WRN(("\nCommands:\n"
                  "  play - resume normal playback\n"
                  "  pause - pause playback\n"
                  "  i - decode only I frames\n"
                  "  ip - decode only I & P frames\n"
                  "  all - decode all frames\n"));
        BDBG_WRN(("\n  fa - frame advance\n"
                  "  fr - frame reverse\n"
                  "  rate(rate) - set trick rate (floating point number, 1.0 is normal play)\n"
                  "  host(type[,modifier,slowrate]) - set host trick mode\n"
                  "    type=i,ip,all; modifier=1,-1 for forward,reverse(i only); slowrate=decoder repeat rate (1=1x,2=2x)\n"
                  "  seek(pos) - jump to position (in milliseconds)\n"));
        if (numArgTotal < 1)
        {
            LUA_ERROR(pLua, "Please specify a command ", error);
        }
    }

    if (numArgTotal >= 1)
    {
        if (strTrickMode == "play")
        {
            pPlaybackTrickData->_command = ePlaybackTrick_Play;
        }
        else
        if (strTrickMode == "fa")
        {
            pPlaybackTrickData->_command = ePlaybackTrick_FrameAdvance;
        }
        else
        if (strTrickMode == "ff")
        {
            pPlaybackTrickData->_command = ePlaybackTrick_FastForward;
        }
        else
        if (strTrickMode == "rew")
        {
            pPlaybackTrickData->_command = ePlaybackTrick_Rewind;
        }
        else
        if (strTrickMode == "fr")
        {
            pPlaybackTrickData->_command = ePlaybackTrick_FrameRewind;
        }
        else
        if (strTrickMode == "pause")
        {
            pPlaybackTrickData->_command = ePlaybackTrick_Pause;
        }
        else
        if (strTrickMode == "i")
        {
            pPlaybackTrickData->_command = ePlaybackTrick_PlayI;
        }
        else
        if (strTrickMode == "ip")
        {
            pPlaybackTrickData->_command = ePlaybackTrick_PlayIP;
        }
        else
        if (strTrickMode == "all")
        {
            pPlaybackTrickData->_command = ePlaybackTrick_PlayNormal;
        }
        else
        if (strTrickMode.left(5) == "rate(")
        {
            pPlaybackTrickData->_rate = 0.0;

            if (strTrickMode[strTrickMode.length() - 1] == ')')
            {
                pPlaybackTrickData->_command = ePlaybackTrick_Rate;
                pPlaybackTrickData->_rate    = (strTrickMode.mid(5, strTrickMode.length() - 5)).toFloat();
            }

            if (0.0 == pPlaybackTrickData->_rate)
            {
                pPlaybackTrickData->_command = ePlaybackTrick_Pause;
            }
        }
        else
        if (strTrickMode.left(5) == "host(")
        {
            int     n               = 0;
            int     modifier        = 0;
            float   decoderSlowRate = 0;
            char    trick[64];
            MString strTrick;

            strTrickMode.replace(',', ' ');
            strTrickMode.replace(')', ' ');
            strTrickMode.replace('[', ' ');
            strTrickMode.replace(']', ' ');
            /* coverity[secure_coding] */
            n = sscanf(strTrickMode.mid(5), "%63s %d %f", trick, &modifier, &decoderSlowRate);
            if ((n < 2) || (modifier == 0)) { modifier = 1; }
            if ((n < 3) || (decoderSlowRate == 0)) { decoderSlowRate = 1; }

            pPlaybackTrickData->_command      = ePlaybackTrick_Host;
            pPlaybackTrickData->_modeModifier = modifier;
            pPlaybackTrickData->_rate         = decoderSlowRate;

            strTrick = trick;
            if (strTrick == "ip")
            {
                pPlaybackTrickData->_trick = NEXUS_PlaybackHostTrickMode_ePlayIP;
            }
            else
            if (strTrick == "i")
            {
                pPlaybackTrickData->_trick = NEXUS_PlaybackHostTrickMode_ePlayI;
            }
            else
            if (strTrick == "all")
            {
                pPlaybackTrickData->_trick = NEXUS_PlaybackHostTrickMode_eNormal;
            }
            else
            if (strTrick == "gop")
            {
                pPlaybackTrickData->_trick = NEXUS_PlaybackHostTrickMode_ePlayGop;
            }
            else
            if (strTrick == "bcm")
            {
                pPlaybackTrickData->_trick = NEXUS_PlaybackHostTrickMode_ePlayBrcm;
            }
            else
            {
                BDBG_WRN(("Unknown host trick mode: %s", trick));
                pPlaybackTrickData->_trick = NEXUS_PlaybackHostTrickMode_eNone;
            }
        }
        else
        if (strTrickMode.left(5) == "seek(")
        {
            unsigned min  = 0;
            unsigned sec  = 0;
            unsigned msec = 0;

            pPlaybackTrickData->_command = ePlaybackTrick_Seek;
            /* coverity[secure_coding] */
            if (sscanf(strTrickMode.mid(5), "%u:%u:%u", &min, &sec, &msec) == 3)
            {
                pPlaybackTrickData->_seekPosition = (min * 60 + sec) * 1000 + msec;
            }
            /* coverity[secure_coding] */
            if (sscanf(strTrickMode.mid(5), "%u:%u", &min, &sec) == 2)
            {
                pPlaybackTrickData->_seekPosition = (min * 60 + sec) * 1000;
            }
            else
            {
                /* coverity[secure_coding] */
                sscanf(strTrickMode.mid(5), "%u", &(pPlaybackTrickData->_seekPosition));
            }
        }
        else
        if (strTrickMode.left(13) == "relativeSeek(")
        {
            unsigned min  = 0;
            unsigned sec  = 0;
            unsigned msec = 0;

            pPlaybackTrickData->_command = ePlaybackTrick_SeekRelative;

            /* coverity[secure_coding] */
            if (sscanf(strTrickMode.mid(13), "%u:%u:%u", &min, &sec, &msec) == 3)
            {
                pPlaybackTrickData->_seekPosition = (min * 60 + sec) * 1000 + msec;
            }

            /* coverity[secure_coding] */
            if (sscanf(strTrickMode.mid(13), "%u:%u", &min, &sec) == 2)
            {
                pPlaybackTrickData->_seekPosition = (min * 60 + sec) * 1000;
            }
            else
            {
                /* coverity[secure_coding] */
                sscanf(strTrickMode.mid(13), "%u", &(pPlaybackTrickData->_seekPosition));
            }
        }
    }

    /*
     * create lua action and give it data
     * note that we are not setting a wait notification so we will not wait for this command to complete
     */
    pAction = new CDataAction <CPlaybackTrickData>(eNotify_PlaybackTrickMode, pPlaybackTrickData);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pPlaybackTrickData);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_PlaybackTrickMode */

/* atlas.playbackStart(
 *      --- optional ---
 *      fileName,          filename to save list to
 *      indexName,          name of channel list
 *      path = NULL)    if set, use the path give to find file otherwise use /data/videos
 */
static int atlasLua_PlaybackStart(lua_State * pLua)
{
    CLua *  pThis       = getCLua(pLua);
    eRet    err         = eRet_Ok;
    uint8_t argNum      = 1;
    uint8_t numArgTotal = lua_gettop(pLua) - 1;

    CDataAction <CPlaybackData> * pAction       = NULL;
    CPlaybackData *               pPlaybackData = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if ((3 < numArgTotal) || (1 > numArgTotal))
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [fileName]<indexName><Path>", error);
    }

    /* add required arguments to scan data */
    pPlaybackData = new CPlaybackData();

    /* add optional arguments to scan data */
    if (1 <= numArgTotal)
    {
        const char * strFileName = luaL_checkstring(pLua, argNum++);
        if (NULL != strFileName)
        {
            pPlaybackData->_strFileName = strFileName;
        }
    }

    if (2 <= numArgTotal)
    {
        const char * strIndexName = luaL_checkstring(pLua, argNum++);
        if (NULL != strIndexName)
        {
            pPlaybackData->_strIndexName = strIndexName;
        }
    }
    else
    {
        pPlaybackData->_strIndexName = pPlaybackData->_strFileName;
    }

    if (3 <= numArgTotal)
    {
        const char * strPath = luaL_checkstring(pLua, argNum++);
        if (NULL != strPath)
        {
            pPlaybackData->_strPath = strPath;
        }
    }

    /*
     * create lua action and give it data
     * note that we are not setting a wait notification so we will not wait for this command to complete
     */
    pAction = new CDataAction <CPlaybackData>(eNotify_PlaybackStart, pPlaybackData, eNotify_VideoSourceChanged, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* for audio only channels */
    pAction->addWaitNotification(eNotify_AudioSourceChanged);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pPlaybackData);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_PlaybackStart */

/* atlas.playbackStop(
 *      --- optional ---
 *      fileName)           file Name to stop playing
 */
static int atlasLua_PlaybackStop(lua_State * pLua)
{
    CLua *  pThis       = getCLua(pLua);
    eRet    err         = eRet_Ok;
    uint8_t argNum      = 1;
    uint8_t numArgTotal = lua_gettop(pLua) - 1;

    CDataAction <CPlaybackData> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* add required arguments to scan data */
    CPlaybackData * pPlaybackData = new CPlaybackData();
    pPlaybackData->_bTuneLastChannel = false;

    /* add optional arguments to scan data */
    if (1 <= numArgTotal)
    {
        const char * strFileName = luaL_checkstring(pLua, argNum++);
        if (NULL != strFileName)
        {
            pPlaybackData->_strFileName = strFileName;
        }
    }

    /* have the playback tune to the last channel */
    pPlaybackData->_bTuneLastChannel = true;

    /* create lua action and give it data */
    pAction = new CDataAction <CPlaybackData>(eNotify_PlaybackStop, pPlaybackData, eNotify_PlaybackStateChanged, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pPlaybackData);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_PlaybackStop */

/* atlas.recordStart(
 *      --- optional ---
 *      fileName,          filename to save list to
 *      path = NULL)    if set, use the path give to find file otherwise use /data/videos
 */
static int atlasLua_RecordStart(lua_State * pLua)
{
    CLua *        pThis       = getCLua(pLua);
    eRet          err         = eRet_Ok;
    uint8_t       argNum      = 1;
    uint8_t       numArgTotal = lua_gettop(pLua) - 1;
    CRecordData * pRecordData = NULL;

    CDataAction <CRecordData> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (4 < numArgTotal)
    {
        /* wrong number of arguments */
#if NEXUS_HAS_SECURITY
        LUA_ERROR(pLua, "wrong number of arguments: [fileName] [Path] [Security]\n--Security = {des,3des,aes}", error);
#else
        LUA_ERROR(pLua, "wrong number of arguments: [fileName][Path]", error);
#endif
    }

    /* add required arguments  */
    pRecordData = new CRecordData();

    if (1 <= numArgTotal)
    {
        const char * strFileName = luaL_checkstring(pLua, argNum++);
        if (NULL != strFileName)
        {
            pRecordData->_strFileName = strFileName;
        }
    }

    if (2 <= numArgTotal)
    {
        const char * strPath = luaL_checkstring(pLua, argNum++);
        if (NULL != strPath)
        {
            pRecordData->_strPath = strPath;
        }
    }

    if (3 <= numArgTotal)
    {
        const char * strSecurity = luaL_checkstring(pLua, argNum++);
        if (NULL != strSecurity)
        {
            pRecordData->_security = strSecurity;
        }
    }

    /*
     * create lua action and give it data
     * note that we are not setting a wait notification so we will not wait for this command to complete
     */
    pAction = new CDataAction <CRecordData>(eNotify_RecordStart, pRecordData, eNotify_RecordStarted, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pRecordData);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_RecordStart */

/* atlas.recordStop(
 *      --- optional ---
 *      fileName)           file Name to stop playing
 */
static int atlasLua_RecordStop(lua_State * pLua)
{
    CLua *        pThis       = getCLua(pLua);
    eRet          err         = eRet_Ok;
    uint8_t       argNum      = 1;
    uint8_t       numArgTotal = lua_gettop(pLua) - 1;
    CRecordData * pRecordData = NULL;

    CDataAction <CRecordData> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (2 < numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [fileName]", error);
    }

    /* add required arguments to scan data */
    pRecordData = new CRecordData();

    /* add optional arguments to scan data */
    if (1 <= numArgTotal)
    {
        const char * strFileName = luaL_checkstring(pLua, argNum++);
        if (NULL != strFileName)
        {
            pRecordData->_strFileName = strFileName;
        }
    }

    /*
     * create lua action and give it data
     * note that we are not setting a wait notification so we will not wait for this command to complete
     */
    pAction = new CDataAction <CRecordData>(eNotify_RecordStop, pRecordData);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pRecordData);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_RecordStop */

/* atlas.encodeStart(
 *      --- optional --- Not supported yet
 *      fileName,          filename to save to
 *      path = NULL)    if set, use the path give to find file otherwise use /data/videos
 */
static int atlasLua_EncodeStart(lua_State * pLua)
{
#if NEXUS_HAS_VIDEO_ENCODER
    CLua *           pThis       = getCLua(pLua);
    eRet             err         = eRet_Ok;
    CTranscodeData * pEncData    = NULL;
    uint8_t          argNum      = 1;
    uint8_t          numArgTotal = lua_gettop(pLua) - 1;

    CDataAction <CTranscodeData> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* add required arguments to scan data */
    pEncData = new CTranscodeData();

    /* add optional arguments to scan data */
    if (1 <= numArgTotal)
    {
        const char * strFileName = luaL_checkstring(pLua, argNum++);
        if (NULL != strFileName)
        {
            pEncData->_strFileName = strFileName;
        }
    }
    else
    if (2 <= numArgTotal)
    {
        const char * strPath = luaL_checkstring(pLua, argNum++);
        if (NULL != strPath)
        {
            pEncData->_strPath = strPath;
        }
    }

    pAction = new CDataAction <CTranscodeData>(eNotify_EncodeStart, pEncData);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
#else /* if NEXUS_HAS_VIDEO_ENCODER */
    BDBG_ERR(("Platform does not support Encode"));
    BSTD_UNUSED(pLua);
    return(eRet_InvalidParameter);

#endif /* if NEXUS_HAS_VIDEO_ENCODER */
} /* atlasLua_EncodeStart */

/* atlas.encodeStop
 */
static int atlasLua_EncodeStop(lua_State * pLua)
{
#if NEXUS_HAS_VIDEO_ENCODER
    CLua *    pThis   = getCLua(pLua);
    CAction * pAction = NULL;
    eRet      err     = eRet_Ok;

    BDBG_ASSERT(pThis);

    pAction = new CAction(eNotify_EncodeStop);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
#else /* if NEXUS_HAS_VIDEO_ENCODER */
    BDBG_ERR(("Platform does not support Encode"));
    BSTD_UNUSED(pLua);
    return(eRet_InvalidParameter);

#endif /* if NEXUS_HAS_VIDEO_ENCODER */
} /* atlasLua_EncodeStop */

static int atlasLua_RefreshPlaybackList(lua_State * pLua)
{
    CLua *    pThis   = getCLua(pLua);
    CAction * pAction = NULL;
    eRet      err     = eRet_Ok;

    BDBG_ASSERT(pThis);

    pAction = new CAction(eNotify_RefreshPlaybackList, eNotify_Invalid, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_RefreshPlaybackList */

/* atlas.setAudioProgram(
 *      --- required ---
 *      audio program pid,   audio program pid to use
 *      --- optional ---
 *      none
 */
static int atlasLua_SetAudioProgram(lua_State * pLua)
{
    CLua *     pThis       = getCLua(pLua);
    eRet       err         = eRet_Ok;
    uint8_t    argNum      = 1;
    uint8_t    numArgTotal = lua_gettop(pLua) - 1;
    uint32_t   pid         = 0;
    uint16_t * pPid        = NULL;

    CDataAction <uint16_t> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [formatType]", error);
    }

    /* get arguments */
    pid = luaL_checknumber(pLua, argNum++);

    /* error checking for arguments */
    if (0x1FFF == pid)
    {
        LUA_ERROR(pLua, "Invalid program pid number", error);
    }

    pPid = new uint16_t;

    /* add required arguments to video format data */

    *pPid = (uint16_t)pid;

    /* create lua action and give it data */
    pAction = new CDataAction <uint16_t>(eNotify_SetAudioProgram, pPid, eNotify_AudioSourceChanged, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pPid);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SetAudioProgram */

/* atlas.remoteKeypress(
 *      --- required ---
 *      remote command,   virtual remote command
 *      --- optional ---
 *      none
 */
static int atlasLua_RemoteKeypress(lua_State * pLua)
{
    CLua *         pThis        = getCLua(pLua);
    eRet           err          = eRet_Ok;
    uint8_t        argNum       = 1;
    uint8_t        numArgTotal  = lua_gettop(pLua) - 1;
    char *         strCommand   = NULL;
    CRemoteEvent * pRemoteEvent = NULL;
    eKey           key          = eKey_Invalid;

    CDataAction <CRemoteEvent> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [remote key]", error);
    }

    /* get arguments */
    strCommand   = (char *)luaL_checkstring(pLua, argNum++);
    pRemoteEvent = new CRemoteEvent;
    key          = stringToKey(strCommand);

    if ((eKey_Max == key) || (eKey_Invalid == key))
    {
        LUA_ERROR(pLua, "invalid key string", error);
    }

    pRemoteEvent->setCode(key);

    /* create lua action and give it keypress data - do not specify a response to wait on */
    pAction = new CDataAction <CRemoteEvent>(eNotify_VirtualKeyDown, pRemoteEvent);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pRemoteEvent);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_RemoteKeypress */

/* atlas.setSpdifType(
 *      --- required ---
 *      spdif input type,   spdif input type to use
 *      --- optional ---
 *      none
 */
static int atlasLua_SetSpdifType(lua_State * pLua)
{
    CLua *        pThis       = getCLua(pLua);
    eRet          err         = eRet_Ok;
    uint8_t       argNum      = 1;
    uint8_t       numArgTotal = lua_gettop(pLua) - 1;
    uint32_t      spdifInput  = 0;
    eSpdifInput * pSpdifInput = NULL;

    CDataAction <eSpdifInput> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [spdifInputType]", error);
    }

    /* get arguments */
    spdifInput = luaL_checknumber(pLua, argNum++);

    /* error checking for arguments */
    if (eSpdifInput_Max <= spdifInput)
    {
        LUA_ERROR(pLua, "Invalid spdif input type", error);
    }

    pSpdifInput = new eSpdifInput;

    /* add required arguments to spdif input type data */

    *pSpdifInput = (eSpdifInput)spdifInput;

    /* create lua action and give it data */
    pAction = new CDataAction <eSpdifInput>(eNotify_SetSpdifInput, pSpdifInput, eNotify_AudioSourceChanged, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pSpdifInput);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SetSpdifType */

/* atlas.setHdmiAudioType(
 *      --- required ---
 *      Hdmi input type,   Hdmi input type to use
 *      --- optional ---
 *      none
 */
static int atlasLua_SetHdmiAudioType(lua_State * pLua)
{
    CLua *            pThis       = getCLua(pLua);
    eRet              err         = eRet_Ok;
    uint8_t           argNum      = 1;
    uint8_t           numArgTotal = lua_gettop(pLua) - 1;
    uint32_t          hdmiInput   = 0;
    eHdmiAudioInput * pHdmiInput  = NULL;

    CDataAction <eHdmiAudioInput> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [hdmiInputType]", error);
    }

    /* get arguments */
    hdmiInput = luaL_checknumber(pLua, argNum++);

    /* error checking for arguments */
    if (eHdmiAudioInput_Max <= hdmiInput)
    {
        LUA_ERROR(pLua, "Invalid hdmi input type", error);
    }

    pHdmiInput = new eHdmiAudioInput;

    /* add required arguments to Hdmi input type data */

    *pHdmiInput = (eHdmiAudioInput)hdmiInput;

    /* create lua action and give it data */
    pAction = new CDataAction <eHdmiAudioInput>(eNotify_SetHdmiAudioInput, pHdmiInput, eNotify_AudioSourceChanged, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pHdmiInput);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SetHdmiAudioType */

/* atlas.setDownmix(
 *      --- required ---
 *      downmix type,   downmix type to use
 *      --- optional ---
 *      none
 */
static int atlasLua_SetDownmix(lua_State * pLua)
{
    CLua *          pThis       = getCLua(pLua);
    eRet            err         = eRet_Ok;
    uint8_t         argNum      = 1;
    uint8_t         numArgTotal = lua_gettop(pLua) - 1;
    uint32_t        downmix     = 0;
    eAudioDownmix * pDownmix    = NULL;

    CDataAction <eAudioDownmix> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [hdmiInputType]", error);
    }

    /* get arguments */
    downmix = luaL_checknumber(pLua, argNum++);

    /* error checking for arguments */
    if (eAudioDownmix_Max <= downmix)
    {
        LUA_ERROR(pLua, "Invalid downmix type", error);
    }

    pDownmix = new eAudioDownmix;

    /* add required arguments to downmix input type data */

    *pDownmix = (eAudioDownmix)downmix;

    /* create lua action and give it data */
    pAction = new CDataAction <eAudioDownmix>(eNotify_SetAudioDownmix, pDownmix, eNotify_AudioSourceChanged, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pDownmix);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SetDownmix */

/* atlas.setDualMono(
 *      --- required ---
 *      DualMono type,   DualMono type to use
 *      --- optional ---
 *      none
 */
static int atlasLua_SetDualMono(lua_State * pLua)
{
    CLua *           pThis       = getCLua(pLua);
    eRet             err         = eRet_Ok;
    uint8_t          argNum      = 1;
    uint8_t          numArgTotal = lua_gettop(pLua) - 1;
    uint32_t         dualMono    = 0;
    eAudioDualMono * pDualMono   = NULL;

    CDataAction <eAudioDualMono> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [hdmiInputType]", error);
    }

    /* get arguments */
    dualMono = luaL_checknumber(pLua, argNum++);

    /* error checking for arguments */
    if (eAudioDualMono_Max <= dualMono)
    {
        LUA_ERROR(pLua, "Invalid dualMono type", error);
    }

    pDualMono = new eAudioDualMono;

    /* add required arguments to DualMono input type data */

    *pDualMono = (eAudioDualMono)dualMono;

    /* create lua action and give it data */
    pAction = new CDataAction <eAudioDualMono>(eNotify_SetAudioDualMono, pDualMono, eNotify_AudioSourceChanged, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pDualMono);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SetDualMono */

/* atlas.setDolbyDRC(
 *      --- required ---
 *      DolbyDRC type,   DolbyDRC type to use
 *      --- optional ---
 *      none
 */
static int atlasLua_SetDolbyDRC(lua_State * pLua)
{
    CLua *      pThis       = getCLua(pLua);
    eRet        err         = eRet_Ok;
    uint8_t     argNum      = 1;
    uint8_t     numArgTotal = lua_gettop(pLua) - 1;
    uint32_t    dolbyDRC    = 0;
    eDolbyDRC * pDolbyDRC   = NULL;

    CDataAction <eDolbyDRC> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [hdmiInputType]", error);
    }

    /* get arguments */
    dolbyDRC = luaL_checknumber(pLua, argNum++);

    /* error checking for arguments */
    if (eDolbyDRC_Max <= dolbyDRC)
    {
        LUA_ERROR(pLua, "Invalid dolbyDRC type", error);
    }

    pDolbyDRC = new eDolbyDRC;

    /* add required arguments to DolbyDRC input type data */

    *pDolbyDRC = (eDolbyDRC)dolbyDRC;

    /* create lua action and give it data */
    pAction = new CDataAction <eDolbyDRC>(eNotify_SetDolbyDRC, pDolbyDRC, eNotify_AudioSourceChanged, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pDolbyDRC);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SetDolbyDRC */

/* atlas.setDolbyDialogNorm(
 *      --- required ---
 *      DialogNorm state,   if true, turn on dialog normalization
 *      --- optional ---
 *      none
 */
static int atlasLua_SetDolbyDialogNorm(lua_State * pLua)
{
    CLua *  pThis       = getCLua(pLua);
    eRet    err         = eRet_Ok;
    uint8_t argNum      = 1;
    uint8_t numArgTotal = lua_gettop(pLua) - 1;
    bool    dialogNorm  = false;
    bool *  pDialogNorm = NULL;

    CDataAction <bool> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [bool]", error);
    }

    /* get arguments */
    dialogNorm  = lua_toboolean(pLua, argNum++);
    pDialogNorm = new bool;

    /* add required arguments to DialogNorm data */
    *pDialogNorm = dialogNorm;

    /* create lua action and give it data */
    pAction = new CDataAction <bool>(eNotify_SetDolbyDialogNorm, pDialogNorm, eNotify_AudioSourceChanged, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pDialogNorm);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SetDolbyDialogNorm */

/* atlas.setVolume(
 *      --- required ---
 *      Volume level,   as a percentage 0-100
 *      --- optional ---
 *      none
 */
static int atlasLua_SetVolume(lua_State * pLua)
{
    CLua *  pThis       = getCLua(pLua);
    eRet    err         = eRet_Ok;
    uint8_t argNum      = 1;
    uint8_t numArgTotal = lua_gettop(pLua) - 1;
    int     volume      = 0;
    int *   pVolume     = NULL;

    CDataAction <int> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [percent 0-100]", error);
    }

    /* get arguments */
    volume = luaL_checknumber(pLua, argNum++);
    if ((volume < 0) || (volume > 100))
    {
        LUA_ERROR(pLua, "Invalid volume percentage (use 0-100)", error);
    }

    pVolume = new int;

    /* add required arguments to Volume data */
    *pVolume = volume;

    /* create lua action and give it data */
    pAction = new CDataAction <int>(eNotify_SetVolume, pVolume, eNotify_VolumeChanged, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pVolume);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SetVolume */

/* atlas.setMute(
 *      --- required ---
 *      Mute state,   if true, mute audio outputs
 *      --- optional ---
 *      none
 */
static int atlasLua_SetMute(lua_State * pLua)
{
    CLua *  pThis       = getCLua(pLua);
    eRet    err         = eRet_Ok;
    uint8_t argNum      = 1;
    uint8_t numArgTotal = lua_gettop(pLua) - 1;
    bool    mute        = false;
    bool *  pMute       = NULL;

    CDataAction <bool> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [percent 0-100]", error);
    }

    /* get arguments */
    mute  = lua_toboolean(pLua, argNum++);
    pMute = new bool;

    /* add required arguments to Mute data */
    *pMute = mute;

    /* create lua action and give it data */
    pAction = new CDataAction <bool>(eNotify_SetMute, pMute, eNotify_MuteChanged, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pMute);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SetMute */

/* atlas.setSpdifInput(
 *      --- required ---
 *      SpdifInput,   set eSpdifInput
 *      --- optional ---
 *      none
 */
static int atlasLua_SetSpdifInput(lua_State * pLua)
{
    CLua *  pThis       = getCLua(pLua);
    eRet    err         = eRet_Ok;
    uint8_t argNum      = 1;
    uint8_t numArgTotal = lua_gettop(pLua) - 1;
    int     input       = 0;
    int *   pInput      = NULL;

    CDataAction <int> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [0:PCM, 1:Compressed, 2:EncodeDTS, 3:EncodeAc3]", error);
    }

    /* get arguments */
    input = luaL_checknumber(pLua, argNum++);
    if ((input > eSpdifInput_Max) || (input < eSpdifInput_Pcm))
    {
        LUA_ERROR(pLua, "Invalid SPDIF input [0:PCM, 1:Compressed, 2:EncodeDTS, 3:EncodeAc3]", error);
    }

    pInput = new int;

    /* add required arguments to Volume data */
    *pInput = input;

    /* create lua action and give it data */
    pAction = new CDataAction <int>(eNotify_SetSpdifInput, pInput, eNotify_AudioSourceChanged, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pInput);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SetSpdifInput */

/* atlas.setHdmiInput(
 *      --- required ---
 *      HdmiInput,   set eHdmiAudioInput
 *      --- optional ---
 *      none
 */
static int atlasLua_SetHdmiInput(lua_State * pLua)
{
    CLua *  pThis       = getCLua(pLua);
    eRet    err         = eRet_Ok;
    uint8_t argNum      = 1;
    uint8_t numArgTotal = lua_gettop(pLua) - 1;
    int     input       = 0;
    int *   pInput      = NULL;

    CDataAction <int> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "Wrong number of arguments: [0:PCM, 1:Compressed, 2:Multichannel, 3:EncodeDTS, 4:EncodeAc3]", error);
    }

    /* get arguments */
    input = luaL_checknumber(pLua, argNum++);
    if ((input > eHdmiAudioInput_Max) || (input < eHdmiAudioInput_Pcm))
    {
        LUA_ERROR(pLua, "Invalid HDMI input [0:PCM, 1:Compressed, 2:Multichannel, 3:EncodeDTS, 4:EncodeAc3]", error);
    }

    pInput = new int;

    /* add required arguments to Volume data */
    *pInput = input;

    /* create lua action and give it Hdmi input data */
    pAction = new CDataAction <int>(eNotify_SetHdmiAudioInput, pInput, eNotify_AudioSourceChanged, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pInput);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SetHdmiInput */

/* atlas.setAudioProcessing(
 *      --- required ---
 *      AudioProcessing,   set eAudioProcessing
 *      --- optional ---
 *      none
 */
static int atlasLua_SetAudioProcessing(lua_State * pLua)
{
    CLua *  pThis       = getCLua(pLua);
    eRet    err         = eRet_Ok;
    uint8_t argNum      = 1;
    uint8_t numArgTotal = lua_gettop(pLua) - 1;
    int     processing  = 0;
    int *   pProcessing = NULL;

    CDataAction <int> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "Wrong number of arguments: [0:none, 1:AutoVolLevel, 2:DolbyVolume, 3:TruVolume]", error);
    }

    /* get arguments */
    processing = luaL_checknumber(pLua, argNum++);

    /* check if selection is valid and available */
    {
        CBoardFeatures * pFeatures = pThis->getConfig()->getBoardFeatures();
        switch (processing)
        {
        case eAudioProcessing_None:
            break;
        case eAudioProcessing_AutoVolumeLevel:
            if (false == pFeatures->_autoVolume)
            {
                LUA_ERROR(pLua, "Auto Volume Level is not supported on this platform", error);
            }
            break;
        case eAudioProcessing_DolbyVolume:
            if (false == pFeatures->_dolbyVolume)
            {
                LUA_ERROR(pLua, "Dolby Volume is not supported on this platform", error);
            }
            break;
        case eAudioProcessing_SrsTruVolume:
            if (false == pFeatures->_srsVolume)
            {
                LUA_ERROR(pLua, "SRS TruVolume is not supported on this platform", error);
            }
            break;
        default:
            LUA_ERROR(pLua, "Invalid audio processing [0:none, 1:AutoVolLevel, 2:DolbyVolume, 3:TruVolume]", error);
            break;
        } /* switch */
    }

    pProcessing = new int;

    /* add required arguments to Volume data */
    *pProcessing = processing;

    /* create lua action and give it data */
    pAction = new CDataAction <int>(eNotify_SetAudioProcessing, pProcessing, eNotify_AudioSourceChanged, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pProcessing);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SetAudioProcessing */

/* atlas.showPip(
 *      --- required ---
 *      PIP state,   if true, show PIP window
 *      --- optional ---
 *      none
 */
static int atlasLua_ShowPip(lua_State * pLua)
{
    CLua *  pThis       = getCLua(pLua);
    eRet    err         = eRet_Ok;
    uint8_t argNum      = 1;
    uint8_t numArgTotal = lua_gettop(pLua) - 1;
    bool    show        = false;
    bool *  pShow       = NULL;

    CDataAction <bool> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [bool]", error);
    }

    /* get arguments */
    show  = lua_toboolean(pLua, argNum++);
    pShow = new bool;

    /* add required arguments to show pip data */
    *pShow = show;

    /* create lua action and give it data */
    {
        eNotification notification = (true == *pShow) ? eNotify_VideoSourceChanged : eNotify_PipStateChanged;

        pAction = new CDataAction <bool>(eNotify_ShowPip, pShow, notification, DEFAULT_LUA_EVENT_TIMEOUT);
        CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);
    }

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pShow);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_ShowPip */

/* atlas.swapPip(
 *      --- required ---
 *      PIP state,   if true, swap PIP window
 *      --- optional ---
 *      none
 */
static int atlasLua_SwapPip(lua_State * pLua)
{
    CLua *  pThis       = getCLua(pLua);
    eRet    err         = eRet_Ok;
    uint8_t numArgTotal = lua_gettop(pLua) - 1;

    CDataAction <bool> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (0 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: should be 0", error);
    }

    /* create lua action and give it data */
    pAction = new CDataAction <bool>(eNotify_SwapPip, NULL, eNotify_VideoSourceChanged, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SwapPip */

#ifdef DCC_SUPPORT
/* atlas.ClosedCaptionEnable()
 */
static int atlasLua_ClosedCaptionEnable(lua_State * pLua)
{
    CLua *  pThis       = getCLua(pLua);
    eRet    err         = eRet_Ok;
    uint8_t argNum      = 1;
    uint8_t numArgTotal = lua_gettop(pLua) - 1;
    bool *  pCcEnable   = NULL;
    bool    ccEnable    = false;

    CDataAction <bool> * pAction = NULL;

    BDBG_ASSERT(pThis);
    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments, enter 'true' or 'false' for enable/disable: [bool]", error);
    }
    /* get arguments */
    pCcEnable  = new bool;
    ccEnable   = lua_toboolean(pLua, argNum++);
    *pCcEnable = ccEnable;

    /* note that we are not setting a wait notification so we will not wait for this command to complete */
    pAction = new CDataAction <bool>(eNotify_ClosedCaptionEnable, pCcEnable);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);
    pThis->addAction(pAction);
    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pCcEnable);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_ClosedCaptionEnable */

/* atlas.ClosedCaptionMode()
 */
static int atlasLua_ClosedCaptionMode(lua_State * pLua)
{
    CLua *       pThis       = getCLua(pLua);
    eRet         err         = eRet_Ok;
    uint8_t      argNum      = 1;
    uint8_t      numArgTotal = lua_gettop(pLua) - 1;
    B_Dcc_Type * pCcMode     = NULL;
    uint32_t     ccMode      = 0;

    CDataAction <B_Dcc_Type> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments, enter '1' for 608 or '2' for 708: [B_Dcc_Type]", error);
    }

    /* get arguments */
    pCcMode = new B_Dcc_Type;
    ccMode  = luaL_checknumber(pLua, argNum++);
    if ((ccMode != 1) && (ccMode != 2))
    {
        LUA_ERROR(pLua, "Incorrect mode, enter '1' for 608 or '2' for 708: [B_Dcc_Type]", error);
    }
    *pCcMode = (B_Dcc_Type)ccMode;

    /* note that we are not setting a wait notification so we will not wait for this command to complete */
    pAction = new CDataAction <B_Dcc_Type>(eNotify_ClosedCaptionMode, pCcMode);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pCcMode);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_ClosedCaptionMode */

#endif /* ifdef DCC_SUPPORT */

/* atlas.setVbiClosedCaptions(
 *      --- required ---
 *      CC state,   if true, turn on closed captions pass-through
 *      --- optional ---
 *      none
 */
static int atlasLua_SetVbiClosedCaptions(lua_State * pLua)
{
    CLua *            pThis           = getCLua(pLua);
    eRet              err             = eRet_Ok;
    uint8_t           argNum          = 1;
    uint8_t           numArgTotal     = lua_gettop(pLua) - 1;
    CModel *          pModel          = NULL;
    CDisplay *        pDisplaySD      = NULL;
    CDisplayVbiData * pDisplayVbiData = NULL;

    CDataAction <CDisplayVbiData> * pAction = NULL;

    BDBG_ASSERT(pThis);

    pModel = pThis->getModel();
    BDBG_ASSERT(NULL != pModel);

    pDisplaySD = pModel->getDisplay(1);

    if (NULL == pDisplaySD)
    {
        LUA_ERROR(pLua, "Unable to set VBI settings: SD Display does not exist.", error);
    }

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [bool]", error);
    }

    /* add required arguments to VBI data */
    pDisplayVbiData  = new CDisplayVbiData();
    *pDisplayVbiData = pDisplaySD->getVbiSettings();

    pDisplayVbiData->bClosedCaptions = lua_toboolean(pLua, argNum++);

    /* create lua action and give it data */
    pAction = new CDataAction <CDisplayVbiData>(eNotify_SetVbiSettings, pDisplayVbiData, eNotify_VbiSettingsChanged, 2000);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pDisplayVbiData);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SetVbiClosedCaptions  */

/* atlas.setVbiTeletext(
 *      --- required ---
 *      teletext state,   if true, turn on teletext
 *      --- optional ---
 *      none
 */
static int atlasLua_SetVbiTeletext(lua_State * pLua)
{
    CLua *            pThis           = getCLua(pLua);
    eRet              err             = eRet_Ok;
    uint8_t           argNum          = 1;
    uint8_t           numArgTotal     = lua_gettop(pLua) - 1;
    CModel *          pModel          = NULL;
    CDisplay *        pDisplaySD      = NULL;
    CDisplayVbiData * pDisplayVbiData = NULL;

    CDataAction <CDisplayVbiData> * pAction = NULL;

    BDBG_ASSERT(pThis);

    pModel = pThis->getModel();
    BDBG_ASSERT(NULL != pModel);

    pDisplaySD = pModel->getDisplay(1);

    if (NULL == pDisplaySD)
    {
        LUA_ERROR(pLua, "Unable to set VBI settings: SD Display does not exist.", error);
    }

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [bool]", error);
    }

    /* add required arguments to VBI data */
    pDisplayVbiData            = new CDisplayVbiData();
    *pDisplayVbiData           = pDisplaySD->getVbiSettings();
    pDisplayVbiData->bTeletext = lua_toboolean(pLua, argNum++);

    /* create lua action and give it data */
    pAction = new CDataAction <CDisplayVbiData>(eNotify_SetVbiSettings, pDisplayVbiData, eNotify_VbiSettingsChanged, 2000);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pDisplayVbiData);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SetVbiTeletext  */

/* atlas.setVbiVps(
 *      --- required ---
 *      VPS state,   if true, turn on video program system (VPS)
 *      --- optional ---
 *      none
 */
static int atlasLua_SetVbiVps(lua_State * pLua)
{
    CLua *            pThis           = getCLua(pLua);
    eRet              err             = eRet_Ok;
    uint8_t           argNum          = 1;
    uint8_t           numArgTotal     = lua_gettop(pLua) - 1;
    CModel *          pModel          = NULL;
    CDisplay *        pDisplaySD      = NULL;
    CDisplayVbiData * pDisplayVbiData = NULL;

    CDataAction <CDisplayVbiData> * pAction = NULL;

    BDBG_ASSERT(pThis);

    pModel = pThis->getModel();
    BDBG_ASSERT(NULL != pModel);

    pDisplaySD = pModel->getDisplay(1);

    if (NULL == pDisplaySD)
    {
        LUA_ERROR(pLua, "Unable to set VBI settings: SD Display does not exist.", error);
    }

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [bool]", error);
    }

    /* add required arguments to VBI data */
    pDisplayVbiData       = new CDisplayVbiData();
    *pDisplayVbiData      = pDisplaySD->getVbiSettings();
    pDisplayVbiData->bVps = lua_toboolean(pLua, argNum++);

    /* create lua action and give it data */
    pAction = new CDataAction <CDisplayVbiData>(eNotify_SetVbiSettings, pDisplayVbiData, eNotify_VbiSettingsChanged, 2000);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pDisplayVbiData);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SetVbiVps  */

/* atlas.setVbiWss(
 *      --- required ---
 *      WSS state,   if true, turn on widescreen signaling (WSS)
 *      --- optional ---
 *      none
 */
static int atlasLua_SetVbiWss(lua_State * pLua)
{
    CLua *            pThis           = getCLua(pLua);
    eRet              err             = eRet_Ok;
    uint8_t           argNum          = 1;
    uint8_t           numArgTotal     = lua_gettop(pLua) - 1;
    CModel *          pModel          = NULL;
    CDisplay *        pDisplaySD      = NULL;
    CDisplayVbiData * pDisplayVbiData = NULL;

    CDataAction <CDisplayVbiData> * pAction = NULL;

    BDBG_ASSERT(pThis);

    pModel = pThis->getModel();
    BDBG_ASSERT(NULL != pModel);

    pDisplaySD = pModel->getDisplay(1);

    if (NULL == pDisplaySD)
    {
        LUA_ERROR(pLua, "Unable to set VBI settings: SD Display does not exist.", error);
    }

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [bool]", error);
    }

    /* add required arguments to VBI data */
    pDisplayVbiData       = new CDisplayVbiData();
    *pDisplayVbiData      = pDisplaySD->getVbiSettings();
    pDisplayVbiData->bWss = lua_toboolean(pLua, argNum++);

    /* create lua action and give it data */
    pAction = new CDataAction <CDisplayVbiData>(eNotify_SetVbiSettings, pDisplayVbiData, eNotify_VbiSettingsChanged, 2000);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pDisplayVbiData);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SetVbiWss  */

/* atlas.setVbiCgms(
 *      --- required ---
 *      CGMS state,   if true, turn on CGMS
 *      --- optional ---
 *      none
 */
static int atlasLua_SetVbiCgms(lua_State * pLua)
{
    CLua *            pThis           = getCLua(pLua);
    eRet              err             = eRet_Ok;
    uint8_t           argNum          = 1;
    uint8_t           numArgTotal     = lua_gettop(pLua) - 1;
    CModel *          pModel          = NULL;
    CDisplay *        pDisplaySD      = NULL;
    CDisplayVbiData * pDisplayVbiData = NULL;

    CDataAction <CDisplayVbiData> * pAction = NULL;

    BDBG_ASSERT(pThis);

    pModel = pThis->getModel();
    BDBG_ASSERT(NULL != pModel);

    pDisplaySD = pModel->getDisplay(1);

    if (NULL == pDisplaySD)
    {
        LUA_ERROR(pLua, "Unable to set VBI settings: SD Display does not exist.", error);
    }

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [bool]", error);
    }

    /* add required arguments to VBI data */
    pDisplayVbiData = new CDisplayVbiData();

    *pDisplayVbiData       = pDisplaySD->getVbiSettings();
    pDisplayVbiData->bCgms = lua_toboolean(pLua, argNum++);

    /* create lua action and give it data */
    pAction = new CDataAction <CDisplayVbiData>(eNotify_SetVbiSettings, pDisplayVbiData, eNotify_VbiSettingsChanged, 2000);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pDisplayVbiData);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SetVbiCgms  */

/* atlas.setVbiGemstar(
 *      --- required ---
 *      Gemstar state,   if true, turn on Gemstar
 *      --- optional ---
 *      none
 */
static int atlasLua_SetVbiGemstar(lua_State * pLua)
{
    CLua *            pThis           = getCLua(pLua);
    eRet              err             = eRet_Ok;
    uint8_t           argNum          = 1;
    uint8_t           numArgTotal     = lua_gettop(pLua) - 1;
    CModel *          pModel          = NULL;
    CDisplay *        pDisplaySD      = NULL;
    CDisplayVbiData * pDisplayVbiData = NULL;

    CDataAction <CDisplayVbiData> * pAction = NULL;

    BDBG_ASSERT(pThis);

    pModel = pThis->getModel();
    BDBG_ASSERT(NULL != pModel);

    pDisplaySD = pModel->getDisplay(1);

    if (NULL == pDisplaySD)
    {
        LUA_ERROR(pLua, "Unable to set VBI settings: SD Display does not exist.", error);
    }

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [bool]", error);
    }

    /* add required arguments to VBI data */
    pDisplayVbiData           = new CDisplayVbiData();
    *pDisplayVbiData          = pDisplaySD->getVbiSettings();
    pDisplayVbiData->bGemstar = lua_toboolean(pLua, argNum++);

    /* create lua action and give it data */
    pAction = new CDataAction <CDisplayVbiData>(eNotify_SetVbiSettings, pDisplayVbiData, eNotify_VbiSettingsChanged, 2000);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pDisplayVbiData);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SetVbiGemstar  */

/* atlas.setVbiAmol(
 *      --- required ---
 *      AMOL state,   [typeI:0 typeII_1mb:1 typeII_2mb:2 off:3]");
 *      --- optional ---
 *      none
 */
static int atlasLua_SetVbiAmol(lua_State * pLua)
{
    CLua *            pThis           = getCLua(pLua);
    eRet              err             = eRet_Ok;
    uint8_t           argNum          = 1;
    uint8_t           numArgTotal     = lua_gettop(pLua) - 1;
    CModel *          pModel          = NULL;
    CDisplay *        pDisplaySD      = NULL;
    CDisplayVbiData * pDisplayVbiData = NULL;

    CDataAction <CDisplayVbiData> * pAction = NULL;

    BDBG_ASSERT(pThis);

    pModel = pThis->getModel();
    BDBG_ASSERT(NULL != pModel);

    pDisplaySD = pModel->getDisplay(1);

    if (NULL == pDisplaySD)
    {
        LUA_ERROR(pLua, "Unable to set VBI settings: SD Display does not exist.", error);
    }

    /* check number of lua arguments on stack */
    if ((1 != numArgTotal) ||
        (0 > lua_tointeger(pLua, argNum)) ||
        (3 < lua_tointeger(pLua, argNum)))
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [typeI:0 typeII_1mb:1 typeII_2mb:2 off:3]", error);
    }

    /* add required arguments to VBI data */
    pDisplayVbiData           = new CDisplayVbiData();
    *pDisplayVbiData          = pDisplaySD->getVbiSettings();
    pDisplayVbiData->amolType = (NEXUS_AmolType)lua_tointeger(pLua, argNum++);
    pDisplayVbiData->bAmol    = (NEXUS_AmolType_eMax == pDisplayVbiData->amolType) ? false : true;

    /* create lua action and give it data */
    pAction = new CDataAction <CDisplayVbiData>(eNotify_SetVbiSettings, pDisplayVbiData, eNotify_VbiSettingsChanged, 2000);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pDisplayVbiData);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SetVbiAmol  */

/* atlas.setVbiMacrovision(
 *      --- required ---
 *      Macrovision state,   [none:0 agcOnly:1 agc2lines:2 agc4lines:3 agcOnlyRgb:5 agc2linesRgb:6 agc4linesRgb:7 test1:8 test2:9]
 *      --- optional ---
 *      none
 */
static int atlasLua_SetVbiMacrovision(lua_State * pLua)
{
    CLua *            pThis           = getCLua(pLua);
    eRet              err             = eRet_Ok;
    uint8_t           argNum          = 1;
    uint8_t           numArgTotal     = lua_gettop(pLua) - 1;
    CModel *          pModel          = NULL;
    CDisplay *        pDisplaySD      = NULL;
    CDisplayVbiData * pDisplayVbiData = NULL;

    CDataAction <CDisplayVbiData> * pAction = NULL;

    BDBG_ASSERT(pThis);

    pModel = pThis->getModel();
    BDBG_ASSERT(NULL != pModel);

    pDisplaySD = pModel->getDisplay(1);

    if (NULL == pDisplaySD)
    {
        LUA_ERROR(pLua, "Unable to set VBI settings: SD Display does not exist.", error);
    }

    /* check number of lua arguments on stack */
    if ((1 != numArgTotal) ||
        (0 > lua_tointeger(pLua, argNum)) ||
        (9 < lua_tointeger(pLua, argNum)) ||
        (4 == lua_tointeger(pLua, argNum)))
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [none:0 agcOnly:1 agc2lines:2 agc4lines:3 agcOnlyRgb:5 agc2linesRgb:6 agc4linesRgb:7 test1:8 test2:9]", error);
    }

    /* add required arguments to VBI data */
    pDisplayVbiData                  = new CDisplayVbiData();
    *pDisplayVbiData                 = pDisplaySD->getVbiSettings();
    pDisplayVbiData->macrovisionType = (NEXUS_DisplayMacrovisionType)lua_tointeger(pLua, argNum++);
    pDisplayVbiData->bMacrovision    = (NEXUS_DisplayMacrovisionType_eNone == pDisplayVbiData->macrovisionType) ? false : true;

    /* create lua action and give it data */
    pAction = new CDataAction <CDisplayVbiData>(eNotify_SetVbiSettings, pDisplayVbiData, eNotify_VbiSettingsChanged, 2000);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pDisplayVbiData);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SetVbiMacrovision  */

/* atlas.setVbiDcs(
 *      --- required ---
 *      DCS state,   [off:0 on1:1 on2:2 on3:3]
 *      --- optional ---
 *      none
 */
static int atlasLua_SetVbiDcs(lua_State * pLua)
{
    CLua *            pThis           = getCLua(pLua);
    eRet              err             = eRet_Ok;
    uint8_t           argNum          = 1;
    uint8_t           numArgTotal     = lua_gettop(pLua) - 1;
    CModel *          pModel          = NULL;
    CDisplay *        pDisplaySD      = NULL;
    CDisplayVbiData * pDisplayVbiData = NULL;

    CDataAction <CDisplayVbiData> * pAction = NULL;

    BDBG_ASSERT(pThis);

    pModel = pThis->getModel();
    BDBG_ASSERT(NULL != pModel);

    pDisplaySD = pModel->getDisplay(1);

    if (NULL == pDisplaySD)
    {
        LUA_ERROR(pLua, "Unable to set VBI settings: SD Display does not exist.", error);
    }

    /* check number of lua arguments on stack */
    if ((1 != numArgTotal) ||
        (0 > lua_tointeger(pLua, argNum)) ||
        (3 < lua_tointeger(pLua, argNum)))
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [off:0 on1:1 on2:2 on3:3]", error);
    }

    /* add required arguments to VBI data */
    pDisplayVbiData          = new CDisplayVbiData();
    *pDisplayVbiData         = pDisplaySD->getVbiSettings();
    pDisplayVbiData->dcsType = (NEXUS_DisplayDcsType)lua_tointeger(pLua, argNum++);
    pDisplayVbiData->bDcs    = (NEXUS_DisplayDcsType_eOff == pDisplayVbiData->dcsType) ? false : true;

    /* create lua action and give it data */
    pAction = new CDataAction <CDisplayVbiData>(eNotify_SetVbiSettings, pDisplayVbiData, eNotify_VbiSettingsChanged, 2000);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pDisplayVbiData);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SetVbiDcs  */

/* atlas.setPowerMode(
 *      --- required ---
 *      power mode,   [on:0 S1:1 S2:2 S3:3]
 *      --- optional ---
 *      none
 */
static int atlasLua_SetPowerMode(lua_State * pLua)
{
    CLua *       pThis       = getCLua(pLua);
    eRet         err         = eRet_Ok;
    uint8_t      argNum      = 1;
    uint8_t      numArgTotal = lua_gettop(pLua) - 1;
    ePowerMode * pPowerMode  = NULL;

    CDataAction <ePowerMode> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [on:0 S1:1 S2:2 S3:3]", error);
    }

    /* add required arguments to power mode data */

    pPowerMode  = new ePowerMode;
    *pPowerMode = (ePowerMode)lua_tointeger(pLua, argNum++);

    /* create lua action and give it data */
    pAction = new CDataAction <ePowerMode>(eNotify_SetPowerMode, pPowerMode, eNotify_PowerModeChanged, 2000);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pPowerMode);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_SetPowerMode  */

static int atlasLua_WifiScanStart(lua_State * pLua)
{
    CLua *  pThis       = getCLua(pLua);
    eRet    err         = eRet_Ok;
    uint8_t numArgTotal = lua_gettop(pLua) - 1;

    CDataAction <bool> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (0 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: should be 0", error);
    }

    /* create lua action and give it data */
    pAction = new CDataAction <bool>(eNotify_NetworkWifiScanStart, NULL, eNotify_NetworkWifiScanResult, 20000);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_WifiScanStart */

static int atlasLua_WifiConnect(lua_State * pLua)
{
    CLua *  pThis       = getCLua(pLua);
    eRet    err         = eRet_Ok;
    uint8_t numArgTotal = lua_gettop(pLua) - 1;

    CDataAction <CNetworkWifiConnectData> * pAction                 = NULL;
    CNetworkWifiConnectData *               pNetworkWifiConnectData = NULL;
    uint8_t argNum = 1;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (2 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [SSID][password]", error);
    }

    /* add required arguments to VBI data */
    pNetworkWifiConnectData               = new CNetworkWifiConnectData();
    pNetworkWifiConnectData->_strSsid     = luaL_checkstring(pLua, argNum++);
    pNetworkWifiConnectData->_strPassword = luaL_checkstring(pLua, argNum++);

    /* create lua action and give it data */
    pAction = new CDataAction <CNetworkWifiConnectData>(eNotify_NetworkWifiConnect, pNetworkWifiConnectData, eNotify_NetworkWifiConnected, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pNetworkWifiConnectData);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_WifiConnect */

static int atlasLua_WifiDisconnect(lua_State * pLua)
{
    CLua *  pThis       = getCLua(pLua);
    eRet    err         = eRet_Ok;
    uint8_t numArgTotal = lua_gettop(pLua) - 1;

    CDataAction <bool> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (0 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: should be 0", error);
    }

    /* create lua action and give it data */
    pAction = new CDataAction <bool>(eNotify_NetworkWifiDisconnect, NULL, eNotify_NetworkWifiDisconnected, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_WifiDisconnect */

#ifdef  NETAPP_SUPPORT
static int atlasLua_BluetoothDiscoveryStart(lua_State * pLua)
{
    CLua *  pThis       = getCLua(pLua);
    eRet    err         = eRet_Ok;
    uint8_t numArgTotal = lua_gettop(pLua) - 1;

    CDataAction <bool> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (0 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: should be 0", error);
    }

    /* create lua action and give it data */
    pAction = new CDataAction <bool>(eNotify_BluetoothDiscoveryStart, NULL, eNotify_BluetoothDiscoveryResult, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_BluetoothDiscoveryStart */

#if 0 /* Can't pass in bt list pointer  from a lua command. Need index and list pointer */
static int atlasLua_BluetoothConnect(lua_State * pLua)
{
    CLua *  pThis       = getCLua(pLua);
    eRet    err         = eRet_Ok;
    uint8_t numArgTotal = lua_gettop(pLua) - 1;

    CDataAction <CBluetoothConnectionData> * pAction                   = NULL;
    CBluetoothConnectionData *               pBluetoothConnectiontData = NULL;
    uint8_t argNum = 1;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (2 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [index][bt_list]", error);
    }

    /* add required arguments to VBI data */
    pBluetoothConnectiontData        = new CBluetoothConnectionData();
    pBluetoothConnectiontData->index = lua_tointeger(pLua, argNum++);
    /*    pBluetoothConnectiontData->pDevInfoList= luaL_checkstring(pLua, argNum++); *//* Todo !! */
    pBluetoothConnectiontData->pDevInfoList = NULL;

    /* create lua action and give it data */
    pAction = new CDataAction <CBluetoothConnectionData>(eNotify_BluetoothConnect, pBluetoothConnectiontData, eNotify_BluetoothConnectionDone, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pBluetoothConnectiontData);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_BluetoothConnect */

#endif /* if 0 */
static int atlasLua_BluetoothDisconnect(lua_State * pLua)
{
    CLua *  pThis       = getCLua(pLua);
    eRet    err         = eRet_Ok;
    uint8_t numArgTotal = lua_gettop(pLua) - 1;

    CDataAction <CBluetoothConnectionData> * pAction                  = NULL;
    CBluetoothConnectionData *               pBluetoothConnectionData = NULL;
    uint8_t argNum = 1;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments:[index]", error);
    }

    /* add required arguments to VBI data */
    pBluetoothConnectionData        = new CBluetoothConnectionData();
    pBluetoothConnectionData->index = lua_tointeger(pLua, argNum++);

    /* create lua action and give it data */
    pAction = new CDataAction <CBluetoothConnectionData>(eNotify_BluetoothDisconnect, pBluetoothConnectionData, eNotify_BluetoothConnectionDone, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pBluetoothConnectionData);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_BluetoothDisconnect */

static int atlasLua_BluetoothGetSavedBtListInfo(lua_State * pLua)
{
    CLua *  pThis       = getCLua(pLua);
    eRet    err         = eRet_Ok;
    uint8_t numArgTotal = lua_gettop(pLua) - 1;

    CDataAction <bool> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (0 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: should be 0", error);
    }

    /* create lua action and give it data */
    pAction = new CDataAction <bool>(eNotify_BluetoothGetSavedBtListInfo, NULL, eNotify_BluetoothListStatusDone, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_BluetoothGetSavedBtListInfo */

static int atlasLua_BluetoothGetConnectedBtListInfo(lua_State * pLua)
{
    CLua *  pThis       = getCLua(pLua);
    eRet    err         = eRet_Ok;
    uint8_t numArgTotal = lua_gettop(pLua) - 1;

    CDataAction <bool> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (0 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: should be 0", error);
    }

    /* create lua action and give it data */
    pAction = new CDataAction <bool>(eNotify_BluetoothGetConnBtListInfo, NULL, eNotify_BluetoothListStatusDone, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_BluetoothGetConnectedBtListInfo */

static int atlasLua_BluetoothGetDiscoveryBtListInfo(lua_State * pLua)
{
    CLua *  pThis       = getCLua(pLua);
    eRet    err         = eRet_Ok;
    uint8_t numArgTotal = lua_gettop(pLua) - 1;

    CDataAction <bool> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (0 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: should be 0", error);
    }

    /* create lua action and give it data */
    pAction = new CDataAction <bool>(eNotify_BluetoothGetDiscBtListInfo, NULL, eNotify_BluetoothListStatusDone, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_BluetoothGetDiscoveryBtListInfo */

static int atlasLua_BluetoothConnectBluetoothFromDiscList(lua_State * pLua)
{
    CLua *  pThis       = getCLua(pLua);
    eRet    err         = eRet_Ok;
    uint8_t numArgTotal = lua_gettop(pLua) - 1;

    CDataAction <CBluetoothConnectionData> * pAction                  = NULL;
    CBluetoothConnectionData *               pBluetoothConnectionData = NULL;
    uint8_t argNum = 1;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments:[index]", error);
    }

    /* add required arguments to VBI data */
    pBluetoothConnectionData        = new CBluetoothConnectionData();
    pBluetoothConnectionData->index = lua_tointeger(pLua, argNum++);

    /* create lua action and give it data */
    pAction = new CDataAction <CBluetoothConnectionData>(eNotify_BluetoothConnectBluetoothFromDiscList, pBluetoothConnectionData, eNotify_BluetoothConnectionDone, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pBluetoothConnectionData);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_BluetoothConnectBluetoothFromDiscList */

static int atlasLua_BluetoothA2DPStart(lua_State * pLua)
{
    CLua *  pThis       = getCLua(pLua);
    eRet    err         = eRet_Ok;
    uint8_t numArgTotal = lua_gettop(pLua) - 1;

    CDataAction <bool> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (0 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: should be 0", error);
    }

    /* create lua action and give it data */
    pAction = new CDataAction <bool>(eNotify_BluetoothA2DPStart, NULL, eNotify_BluetoothA2DPStarted, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_BluetoothA2DPStart */

static int atlasLua_BluetoothA2DPStop(lua_State * pLua)
{
    CLua *  pThis       = getCLua(pLua);
    eRet    err         = eRet_Ok;
    uint8_t numArgTotal = lua_gettop(pLua) - 1;

    CDataAction <bool> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (0 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: should be 0", error);
    }

    /* create lua action and give it data */
    pAction = new CDataAction <bool>(eNotify_BluetoothA2DPStop, NULL, eNotify_BluetoothA2DPStopped, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_BluetoothA2DPStart */

#endif /* ifdef  NETAPP_SUPPORT */

#if RF4CE_SUPPORT
extern void AddRf4ceRemote(const char * remote_name);
extern void DisplayRf4ceRemotes(void);
extern void RemoveRf4ceRemote(int remote_num);

static int atlasLua_AddRf4ceRemote(lua_State * pLua)
{
    uint8_t            argNum           = 1;
    uint8_t            numArgTotal      = lua_gettop(pLua);
    const char *       remoteName       = NULL;
    eRet               err              = eRet_Ok;
    CLua *             pThis            = getCLua(pLua);
    CRf4ceRemoteData * pRf4ceRemoteData = NULL;

    CDataAction <CRf4ceRemoteData> * pAction = NULL;

    /* check number of lua arguments on stack */
    if (numArgTotal != 1)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [remoteName]", error);
    }
    remoteName = luaL_checkstring(pLua, argNum++);

    pRf4ceRemoteData = new CRf4ceRemoteData();

    /* error checking for arguments */
    if (NULL == pRf4ceRemoteData)
    {
        LUA_ERROR(pLua, "unable to allocate rf4ce data", error);
    }
    pRf4ceRemoteData->_name = remoteName;

    /* create lua action and give it data */
    pAction = new CDataAction <CRf4ceRemoteData>(eNotify_AddRf4ceRemote, pRf4ceRemoteData);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);
    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);
    goto done;
error:
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_AddRf4ceRemote */

static int atlasLua_RemoveRf4ceRemote(lua_State * pLua)
{
    int                remote_num;
    uint8_t            numArgTotal      = lua_gettop(pLua);
    eRet               err              = eRet_Ok;
    CLua *             pThis            = getCLua(pLua);
    CRf4ceRemoteData * pRf4ceRemoteData = NULL;

    CDataAction <CRf4ceRemoteData> * pAction = NULL;

    /* check number of lua arguments on stack */
    if (numArgTotal != 1)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [pairingNum]", error);
    }

    remote_num = lua_tointeger(pLua, 1);

    pRf4ceRemoteData = new CRf4ceRemoteData();

    /* error checking for arguments */
    if (NULL == pRf4ceRemoteData)
    {
        LUA_ERROR(pLua, "unable to allocate rf4ce data", error);
    }
    pRf4ceRemoteData->_pairingRefNum = remote_num;

    /* create lua action and give it data */
    pAction = new CDataAction <CRf4ceRemoteData>(eNotify_RemoveRf4ceRemote, pRf4ceRemoteData);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);
    goto done;
error:
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_RemoveRf4ceRemote */

static int atlasLua_DisplayRf4ceRemotes(lua_State * pLua)
{
    CLua *    pThis   = getCLua(pLua);
    CAction * pAction = NULL;
    eRet      err     = eRet_Ok;

    pAction = new CAction(eNotify_DisplayRf4ceRemotes);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);
    goto done;
error:
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_DisplayRf4ceRemotes */

#endif /* if RF4CE_SUPPORT */

static int atlasLua_PlaylistDiscovery(lua_State * pLua)
{
    CLua *   pThis       = getCLua(pLua);
    eRet     err         = eRet_Ok;
    int *    pIndex      = 0;
    CModel * pModel      = NULL;
    uint8_t  numArgTotal = lua_gettop(pLua) - 1;

    CDataAction <int> * pAction = NULL;

    BDBG_ASSERT(pThis);

    pModel = pThis->getModel();
    BDBG_ASSERT(NULL != pModel);

    pIndex = new int;

    /* check number of lua arguments on stack */
    if (1 == numArgTotal)
    {
        *pIndex = lua_tointeger(pLua, 1);
    }
    else
    if (1 < numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: <index> of playlist to retrieve. use 0 to print all", error);
    }

    /* create lua action and give it data */
    pAction = new CDataAction<int>(eNotify_ShowDiscoveredPlaylists, pIndex, eNotify_DiscoveredPlaylistsShown, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pIndex);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_PlaylistDiscovery */

static int atlasLua_PlaylistShow(lua_State * pLua)
{
    CLua *          pThis         = getCLua(pLua);
    eRet            err           = eRet_Ok;
    uint8_t         numArgTotal   = lua_gettop(pLua) - 1;
    CPlaylistData * pPlaylistData = NULL;
    uint8_t         argNum        = 1;
    int             nIndex        = 0;
    MString         strIp;

    CDataAction <CPlaylistData> * pAction = NULL;

    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (2 < numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [IP Address]<index>", error);
    }

    strIp = luaL_checkstring(pLua, argNum++);

    if (2 == numArgTotal)
    {
        nIndex = lua_tointeger(pLua, argNum++);
    }

    /* add required arguments to data */
    pPlaylistData = new CPlaylistData(strIp.s(), nIndex);

    BDBG_ASSERT(pThis);

    /* create lua action and give it data */
    pAction = new CDataAction <CPlaylistData>(eNotify_ShowPlaylist, pPlaylistData, eNotify_PlaylistShown, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_ShowDiscoveredPlaylists */

#ifdef PLAYBACK_IP_SUPPORT
static int atlasLua_ChannelStream(lua_State * pLua)
{
    CLua *        pThis       = getCLua(pLua);
    eRet          err         = eRet_Ok;
    uint8_t       numArgTotal = lua_gettop(pLua) - 1;
    CChannelBip * pChannelIp  = NULL;
    uint8_t       argNum      = 1;

    CDataAction <CChannelBip> * pAction = NULL;
    BDBG_ASSERT(pThis);

    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments: [url] ex http://10.10.10.10:8089/avatar.mpg?program=0 ", error);
    }

    /* add required arguments to data */
    pChannelIp = new CChannelBip();
    pChannelIp->setUrl(luaL_checkstring(pLua, argNum));

    BDBG_MSG(("URL : %s", pChannelIp->getUrl().s()));
    BDBG_ASSERT(pThis);

    /* create lua action and give it data */
    pAction = new CDataAction <CChannelBip>(eNotify_StreamChannel, pChannelIp, eNotify_CurrentChannel, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);
    goto done;
error:
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_ChannelStream */

#endif /* ifdef PLAYBACK_IP_SUPPORT */

/* atlas.ipClientTranscodeEnable()
 */
static int atlasLua_ipClientTranscodeEnable(lua_State * pLua)
{
    CLua *  pThis            = getCLua(pLua);
    eRet    err              = eRet_Ok;
    uint8_t argNum           = 1;
    uint8_t numArgTotal      = lua_gettop(pLua) - 1;
    bool *  pTranscodeEnable = NULL;
    bool    transcodeEnable  = false;

    CDataAction <bool> * pAction = NULL;

    BDBG_ASSERT(pThis);
    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments, enter 'true' or 'false' for enable/disable: [bool]", error);
    }
    /* get arguments */
    pTranscodeEnable  = new bool;
    transcodeEnable   = lua_toboolean(pLua, argNum++);
    *pTranscodeEnable = transcodeEnable;

    /* note that we are not setting a wait notification so we will not wait for this command to complete */
    pAction = new CDataAction <bool>(eNotify_ipClientTranscodeEnable, pTranscodeEnable);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);
    pThis->addAction(pAction);
    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pTranscodeEnable);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_ipClientTranscodeEnable */

/* atlas.ipClientTranscodeProfile()
 */
static int atlasLua_ipClientTranscodeProfile(lua_State * pLua)
{
    CLua *  pThis             = getCLua(pLua);
    eRet    err               = eRet_Ok;
    uint8_t argNum            = 1;
    uint8_t numArgTotal       = lua_gettop(pLua) - 1;
    int     _xcodeProfile     = 0;
    int *   pTranscodeProfile = NULL;

    CDataAction <int> * pAction = NULL;

    BDBG_ASSERT(pThis);
    /* check number of lua arguments on stack */
    if (1 != numArgTotal)
    {
        /* wrong number of arguments */
        LUA_ERROR(pLua, "wrong number of arguments, enter '1' or '2' for 480p/720p profile selection:", error);
    }

    /* get arguments */
    _xcodeProfile = luaL_checknumber(pLua, argNum++);

    pTranscodeProfile  = new int;
    *pTranscodeProfile = _xcodeProfile;

    /* note that we are not setting a wait notification so we will not wait for this command to complete */
    pAction = new CDataAction <int>(eNotify_ipClientTranscodeProfile, pTranscodeProfile, eNotify_Invalid, DEFAULT_LUA_EVENT_TIMEOUT);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    pThis->addAction(pAction);
    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pTranscodeProfile);
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_ipClientTranscodeProfile */

static int atlasLua_SetDebugLevel(lua_State * pLua)
{
    const char * strModule = luaL_checkstring(pLua, 1);
    const char * strLevel  = luaL_checkstring(pLua, 2);

    if ((NULL != strModule) && (NULL != strLevel))
    {
        BDBG_Level level = (BDBG_Level)0;
        if (!strcasecmp(strLevel, "trace"))
        {
            level = BDBG_eTrace;
        }
        else
        if (!strcasecmp(strLevel, "msg"))
        {
            level = BDBG_eMsg;
        }
        else
        if (!strcasecmp(strLevel, "wrn"))
        {
            level = BDBG_eWrn;
        }
        else
        if (!strcasecmp(strLevel, "err"))
        {
            level = BDBG_eErr;
        }
        else
        if (!strcasecmp(strLevel, "log"))
        {
            level = BDBG_eLog;
        }
        else
        {
            level = BDBG_eWrn;
        }

        BDBG_WRN(("Set debug level:%d for module:%s", level, strModule));
        BDBG_SetModuleLevel(strModule, level);
    }
    else
    {
        LUA_ERROR(pLua, "wrong number of arguments: [module][debugLevel]", error);
    }

    goto done;
error:
    luaL_error(pLua, "Atlas Lua Error");
done:
    return(0);
} /* atlasLua_SetDebugLevel */

static int atlasLua_RunScript(lua_State * pLua)
{
    CLua *       pThis       = getCLua(pLua);
    const char * strFileName = luaL_checkstring(pLua, 1);
    const char * strPath     = GET_STR(pThis->getCfg(), SCRIPTS_PATH);

    if (NULL != strFileName)
    {
        MString strFilePath;
        int     lerror = 0;

        strFilePath  = strPath;
        strFilePath += "/";
        strFilePath += strFileName;

        BDBG_WRN(("Running script: %s", strFilePath.s()));
        lerror = luaL_loadfile(pThis->getLuaState(), strFilePath.s());
        if (lerror)
        {
            BDBG_WRN(("LUA error: %s", lua_tostring(pThis->getLuaState(), -1)));
            lua_pop(pThis->getLuaState(), 1);
        }
        else
        {
            lerror = lua_pcall(pThis->getLuaState(), 0, 0, 0);
            if (lerror)
            {
                BDBG_WRN(("LUA error: %s", lua_tostring(pThis->getLuaState(), -1)));
                lua_pop(pThis->getLuaState(), 1);
            }
        }
    }

    return(0);
} /* atlasLua_RunScript */

static int atlasLua_Debug(lua_State * pLua)
{
    CLua *    pThis     = getCLua(pLua);
    eRet      err       = eRet_Ok;
    int       argNum    = 1;
    MString * pStrDebug = NULL;

    CDataAction <MString> * pAction = NULL;
    BDBG_ASSERT(pThis);

    pStrDebug = new MString(luaL_checkstring(pLua, argNum));

    /* create lua action and give it data */
    pAction = new CDataAction <MString>(eNotify_Debug, pStrDebug);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);

    /* save lua action to queue - this action will be serviced when we get the bwin io callback:
     * bwinLuaCallback() */
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);
    goto done;
error:
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_Debug */

static int atlasLua_Sleep(lua_State * pLua)
{
    int msecs = 0;

    msecs = lua_tointeger(pLua, -1);
    BDBG_MSG(("Sleep:%d msecs", msecs));
    BKNI_Sleep(msecs);

    return(0);
}

static int atlasLua_Exit(lua_State * pLua)
{
    CLua *    pThis   = getCLua(pLua);
    CAction * pAction = NULL;
    eRet      err     = eRet_Ok;

    BDBG_ASSERT(pThis);

    /*
     * save lua event and necessary data
     * note that we are not setting a wait notification so we will not wait for this command to complete
     */
    pAction = new CAction(eNotify_Exit);
    CHECK_PTR_ERROR_GOTO("Unable to malloc CAction", pAction, err, eRet_OutOfMemory, error);
    BDBG_ASSERT(NULL != pThis);
    pThis->addAction(pAction);

    /* trigger bwin io event here */
    err = pThis->trigger(pAction);

    goto done;
error:
    DEL(pAction);
    err = eRet_InvalidParameter;
done:
    LUA_RETURN(err);
} /* atlasLua_Exit */

/* list of atlas lua extension APIs */
static const struct luaL_Reg atlasLua[] = {
    { "channelUp",                     atlasLua_ChannelUp                             }, /* channel up */
    { "channelDown",                   atlasLua_ChannelDown                           }, /* channel down */
    { "channelTune",                   atlasLua_ChannelTune                           }, /* tune to the given channel */
    { "channelUntune",                 atlasLua_ChannelUnTune                         }, /* UNtune from the current channel */
#if NEXUS_HAS_FRONTEND
    { "scanQam",                       atlasLua_ScanQam                               }, /* scan for qam channels */
    { "scanVsb",                       atlasLua_ScanVsb                               }, /* scan for vsb channels */
    { "scanSat",                       atlasLua_ScanSat                               }, /* scan for sat channels */
    { "scanOfdm",                      atlasLua_ScanOfdm                              }, /* scan for ofdm channels */
#endif /* if NEXUS_HAS_FRONTEND */
    { "setVideoFormat",                atlasLua_SetVideoFormat                        }, /* set the video format */
    { "setContentMode",                atlasLua_SetContentMode                        }, /* set the video content mode */
    { "setColorSpace",                 atlasLua_SetColorSpace                         }, /* set the color space */
    { "setMpaaDecimation",             atlasLua_SetMpaaDecimation                     }, /* set mpaa decimation */
    { "setDeinterlacer",               atlasLua_SetDeinterlacer                       }, /* set MAD deinterlacer */
    { "setBoxDetect",                  atlasLua_SetBoxDetect                          }, /* set box detect */
    { "setAspectRatio",                atlasLua_SetAspectRatio                        }, /* set aspect ratio */
    { "setAutoVideoFormat",            atlasLua_SetAutoVideoFormat                    }, /* set auto video format */
    { "channelListLoad",               atlasLua_ChannelListLoad                       }, /* load channel list */
    { "channelListSave",               atlasLua_ChannelListSave                       }, /* save current channel list */
    { "channelListDump",               atlasLua_ChannelListDump                       }, /* dump current channel list */
    { "playbackListDump",              atlasLua_PlaybackListDump                      }, /* dump playback list */
    { "playbackStart",                 atlasLua_PlaybackStart                         }, /* Start Playback of a file */
    { "playbackStop",                  atlasLua_PlaybackStop                          }, /* Stop Playback of a file */
    { "playbackTrickMode",             atlasLua_PlaybackTrickMode                     }, /* Do a Custom trick Mode */
    { "playbackListRefresh",           atlasLua_RefreshPlaybackList                   }, /* refresh list of available videos */
    { "recordStart",                   atlasLua_RecordStart                           }, /* Start Record of a channel */
    { "recordStop",                    atlasLua_RecordStop                            }, /* Stop Record of a channel */
    { "encodeStart",                   atlasLua_EncodeStart                           }, /* Start Encode of a channel */
    { "encodeStop",                    atlasLua_EncodeStop                            }, /* Stop Encode of a channel */
    { "setAudioProgram",               atlasLua_SetAudioProgram                       }, /* set the audio program pid */
    { "remoteKeypress",                atlasLua_RemoteKeypress                        }, /* send virtual keypress */
    { "setSpdifType",                  atlasLua_SetSpdifType                          }, /* set the spdif audio type */
    { "setHdmiAudioType",              atlasLua_SetHdmiAudioType                      }, /* set the hdmi audio type */
    { "setDownmix",                    atlasLua_SetDownmix                            }, /* set the audio downmix */
    { "setDualMono",                   atlasLua_SetDualMono                           }, /* set the audio DualMono */
    { "setDolbyDRC",                   atlasLua_SetDolbyDRC                           }, /* set the Dolby DRC compression */
    { "setDolbyDialogNorm",            atlasLua_SetDolbyDialogNorm                    }, /* set the Dolby dialog normalization */
    { "setVolume",                     atlasLua_SetVolume                             }, /* set the volume level */
    { "setMute",                       atlasLua_SetMute                               }, /* set the mute level */
    { "setSpdifInput",                 atlasLua_SetSpdifInput                         }, /* set the spdif input (see eSpdifInput) */
    { "setHdmiInput",                  atlasLua_SetHdmiInput                          }, /* set the hdmi input (see eHdmiAudioInput) */
    { "setAudioProcessing",            atlasLua_SetAudioProcessing                    }, /* set the pcm audio proccessing (see eAudioProcessing) */
    { "showPip",                       atlasLua_ShowPip                               }, /* show/hide the pip window */
    { "swapPip",                       atlasLua_SwapPip                               }, /* swap the main and pip window */
    { "ipClientTranscodeEnable",       atlasLua_ipClientTranscodeEnable               }, /* enable/disable BIP transcoding for a given client */
    { "ipClientTranscodeProfile",      atlasLua_ipClientTranscodeProfile              }, /* set the BIP transcode profile for a given client */
#ifdef DCC_SUPPORT
    { "closedCaptionEnable",           atlasLua_ClosedCaptionEnable                   }, /* enable/disable closed caption */
    { "closedCaptionMode",             atlasLua_ClosedCaptionMode                     }, /*set 608/708 closed caption mode */
#endif /* ifdef DCC_SUPPORT */
    { "setVbiClosedCaptions",          atlasLua_SetVbiClosedCaptions                  }, /* enable/disable VBI closedCaptions pass-through */
    { "setVbiTeletext",                atlasLua_SetVbiTeletext                        }, /* enable/disable VBI teletext */
    { "setVbiVps",                     atlasLua_SetVbiVps                             }, /* enable/disable VBI video program system (VPS) */
    { "setVbiWss",                     atlasLua_SetVbiWss                             }, /* enable/disable VBI widescreen signaling (WSS) */
    { "setVbiCgms",                    atlasLua_SetVbiCgms                            }, /* enable/disable VBI CGMS */
    { "setVbiGemstar",                 atlasLua_SetVbiGemstar                         }, /* enable/disable VBI Gemstar */
    { "setVbiAmol",                    atlasLua_SetVbiAmol                            }, /* enable/disable VBI Nielsen AMOL */
    { "setVbiMacrovision",             atlasLua_SetVbiMacrovision                     }, /* enable/disable VBI Macrovision */
    { "setVbiDcs",                     atlasLua_SetVbiDcs                             }, /* enable/disable VBI DCS */
    { "setPowerMode",                  atlasLua_SetPowerMode                          }, /* set power mode */
#if RF4CE_SUPPORT
    { "rf4ceRemoteAdd",                atlasLua_AddRf4ceRemote                        }, /* add RF4CE remote */
    { "rf4ceRemoteRemove",             atlasLua_RemoveRf4ceRemote                     }, /* remove RF4CE remote */
    { "rf4ceRemotesDisplay",           atlasLua_DisplayRf4ceRemotes                   }, /* display RF4CE remotes */
#endif
    { "wifiScanStart",                 atlasLua_WifiScanStart                         }, /* wifi network scan start */
    { "wifiConnect",                   atlasLua_WifiConnect                           }, /* wifi network connect */
    { "wifiDisconnect",                atlasLua_WifiDisconnect                        }, /* wifi network disconnect */
#ifdef NETAPP_SUPPORT
    { "bluetoothDiscoveryStart",       atlasLua_BluetoothDiscoveryStart               }, /* bluetooth discovery start */
    { "bluetoothDisconnect",           atlasLua_BluetoothDisconnect                   }, /* bluetooth disconnect */
    { "bluetoothGetSavedListInfo",     atlasLua_BluetoothGetSavedBtListInfo           }, /* bluetooth GetSavedBtListInfo */
    { "bluetoothGetDiscoveryListInfo", atlasLua_BluetoothGetDiscoveryBtListInfo       }, /* bluetooth GetDiscoveryBtListInfo */
    { "bluetoothGetConnectedListInfo", atlasLua_BluetoothGetConnectedBtListInfo       }, /* bluetooth GetConnectedBtListInfo */
    { "bluetoothConnectFromDiscList",  atlasLua_BluetoothConnectBluetoothFromDiscList }, /* bluetooth ConnectBluetoothFromDiscList */
    { "bluetoothA2DPStart",            atlasLua_BluetoothA2DPStart                    }, /* bluetooth A2DP start */
    { "bluetoothA2DPStop",             atlasLua_BluetoothA2DPStop                     }, /* bluetooth A2DP stopt */
#endif /* ifdef NETAPP_SUPPORT */
    { "playlistDiscovery",             atlasLua_PlaylistDiscovery                     }, /* show playlists discovered from other servers */
    { "playlistShow",                  atlasLua_PlaylistShow                          }, /* show playlist contents */
#ifdef PLAYBACK_IP_SUPPORT
    { "channelStream",                 atlasLua_ChannelStream                         }, /* stream ip channel from playlist */
#endif /* ifdef PLAYBACK_IP_SUPPORT */
    { "setDebugLevel",                 atlasLua_SetDebugLevel                         }, /* set debug level for given module */
    { "runScript",                     atlasLua_RunScript                             }, /* Run given lua script */
    { "debug",                         atlasLua_Debug                                 }, /* Display given debug string on screen and in output log */
    { "sleep",                         atlasLua_Sleep                                 }, /* sleep */
    { "exit",                          atlasLua_Exit                                  }, /* exit application */
    { "quit",                          atlasLua_Exit                                  }, /* exit application */
    { NULL,                            NULL                                           }  /* sentinel */
};

static int luaopen_atlas(lua_State * pLua)
{
    lua_newtable(pLua);
    luaL_setfuncs(pLua, atlasLua, 0);
    lua_pushvalue(pLua, -1);
    lua_setglobal(pLua, LUA_ATLASLIBNAME);

    return(0);
}

/*** lua dir library facilitates reading files within a directory using iterator ***/
#include <dirent.h>

static int dir_iter(lua_State * pLua)
{
    DIR *           d     = *(DIR **)lua_touserdata(pLua, lua_upvalueindex(1));
    struct dirent * entry = NULL;
    int             ret   = 0;

    /* read next directory entry but skip ".*" files */
    entry = readdir(d);
    while ((entry) && ('.' == entry->d_name[0]))
    {
        entry = readdir(d);
    }

    if (NULL != entry)
    {
        lua_pushstring(pLua, entry->d_name);
        ret = 1;
    }

    return(ret);
} /* dir_iter */

static int l_dir(lua_State * pLua)
{
    const char * path = luaL_checkstring(pLua, 1);

    /* create a userdatum to store a DIR address */
    DIR ** d = (DIR **)lua_newuserdata(pLua, sizeof(DIR *));

    /* set its metatable */
    luaL_getmetatable(pLua, LUA_DIRLIBNAME);
    lua_setmetatable(pLua, -2);

    /* try to open the given directory */
    *d = opendir(path);
    if (NULL == *d)
    {
        char strTmp[64];
        snprintf(strTmp, 64, "cannot open %s", path);
        LUA_ERROR(pLua, strTmp, error);
    }

    /* creates and returns the iterator function;
     * its sole upvalue, the directory userdatum,
     * is already on the stack top */
    lua_pushcclosure(pLua, dir_iter, 1);

error:
    return(1);
} /* l_dir */

static int dir_gc(lua_State * pLua)
{
    DIR * d = *(DIR **)lua_touserdata(pLua, 1);

    if (d)
    {
        closedir(d);
    }

    return(0);
}

/* list of atlas DIR lua extension APIs */
static const struct luaL_Reg atlasDirLua[] = {
    { "dir", l_dir }, /* dir */
    { NULL,  NULL  }  /* sentinel */
};

static int luaopen_dir(lua_State * pLua)
{
    lua_newtable(pLua);
    luaL_setfuncs(pLua, atlasDirLua, 0);

    /* set its __gc field */
    lua_pushstring(pLua, "__gc");
    lua_pushcfunction(pLua, dir_gc);
    lua_settable(pLua, -3);

    /* register the 'dir' function */
    lua_pushcfunction(pLua, l_dir);
    lua_setglobal(pLua, "dir");

    return(0);
}

#ifdef LINENOISE_SUPPORT
int autoCompleteDictionaryCompare(
        MString * str1,
        MString * str2
        )
{
    return(strcmp(str1->s(), str2->s()));
}

static MAutoList <MString> autoCompleteDictionary;
#endif /* ifdef LINENOISE_SUPPORT */

CLua::CLua() :
    CView("CLua"),
    _threadRun(false),
    _pollInterval(200),
    _pLua(NULL),
    _threadShell(NULL),
    _shellStarted(false),
    _actionMutex(NULL),
    _busyEvent(NULL),
    _pWidgetEngine(NULL),
    _busyAction(eNotify_Invalid),
    _pConfig(NULL),
    _pCfg(NULL),
    _pModel(NULL)
{
    eRet err = eRet_Ok;

    _actionMutex = B_Mutex_Create(NULL);
    CHECK_PTR_ERROR_GOTO("unable to create mutex", _actionMutex, err, eRet_ExternalError, error);

    _busyEvent = B_Event_Create(NULL);
    CHECK_PTR_ERROR_GOTO("unable to create event", _busyEvent, err, eRet_ExternalError, error);

    return;

error:
    BDBG_ASSERT(false);
}

CLua::~CLua()
{
    stop();
    uninitialize();

    if (NULL != _busyEvent)
    {
        B_Event_Destroy(_busyEvent);
        _busyEvent = NULL;
    }

    if (NULL != _actionMutex)
    {
        B_Mutex_Destroy(_actionMutex);
        _actionMutex = NULL;
    }
}

eRet CLua::initialize(CConfig * pConfig)
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != pConfig);

    _pConfig = pConfig;
    _pCfg    = pConfig->getCfg();

#ifdef LINENOISE_SUPPORT
    autoCompleteDictionary.add(new MString("exit"));
    autoCompleteDictionary.add(new MString("io.write("));
    autoCompleteDictionary.add(new MString("io.read("));
    autoCompleteDictionary.add(new MString("help"));

    /* auto add atlas custom commands to auto complete list */
    for (int i = 0; atlasLua[i].name; i++)
    {
        if (MString(atlasLua[i].name) == "remoteKeypress")
        {
            continue;
        }

        MString * pStr = NULL;
        pStr   = new MString("atlas.");
        *pStr += atlasLua[i].name;
        *pStr += "(";
        autoCompleteDictionary.add(pStr);
    }

    /* dtt todo - might better to manually sort by popularity rather than alphabetically.
     * unless list gets huge, it probably won't matter at all as even a linear search is
     * plenty fast when it comes to user input. */
    autoCompleteDictionary.sort(autoCompleteDictionaryCompare);
#endif /* ifdef LINENOISE_SUPPORT */

    _pLua = luaL_newstate();
    CHECK_PTR_ERROR_GOTO("lua open failed", _pLua, ret, eRet_ExternalError, error);

    /* open lua basic lib, table lib, i/o lib, string lib and math lib */
    luaL_openlibs(_pLua);
    /* open lua atlas lib */
    luaopen_atlas(_pLua);
    /* open lua atlas dir lib */
    luaopen_dir(_pLua);

    /* save "this" pointer to lua registry */
    lua_pushlightuserdata(_pLua, (void *)&LUAKEY_CLUA);
    lua_pushlightuserdata(_pLua, (void *)this);
    lua_settable(_pLua, LUA_REGISTRYINDEX);

error:
    return(ret);
} /* initialize */

eRet CLua::uninitialize()
{
    eRet ret = eRet_Ok;

    autoCompleteDictionary.clear();

    if (NULL != _pLua)
    {
        lua_close(_pLua);
    }

    return(ret);
}

void CLua::addAction(CAction * pAction)
{
    CScopedMutex scopedMutex(_actionMutex);

    _actionList.add(pAction);
}

CAction * CLua::getAction()
{
    CAction *    pAction = NULL;
    CScopedMutex scopedMutex(_actionMutex);

    if (0 < _actionList.total())
    {
        pAction = (CAction *)_actionList.first();
    }

    return(pAction);
}

CAction * CLua::removeAction()
{
    CAction *    pAction = NULL;
    CScopedMutex scopedMutex(_actionMutex);

    if (0 < _actionList.total())
    {
        pAction = (CAction *)_actionList.first();
        _actionList.remove();
    }

    return(pAction);
}

#ifdef LINENOISE_SUPPORT
void luaShellCompletionCallback(
        const char *           buf,
        linenoiseCompletions * lc
        )
{
    MString * pString = NULL;

    MListItr <MString> itr(&autoCompleteDictionary);

    if (!isalpha(buf[0]))
    {
        return;
    }

    /* search all autocomplete entries */
    for (pString = itr.first(); NULL != pString; pString = itr.next())
    {
        /* only add autocomplete choices that match current buffer */
        if (pString->left(strlen(buf)) == MString(buf))
        {
            /* only add autocomplete choices that aren't exactly the same as given buffer */
            if (pString->length() != MString(buf).length())
            {
                linenoiseAddCompletion(lc, (char *)pString->s());
            }
        }
    }
} /* luaShellCompletionCallback */

#endif /* ifdef LINENOISE_SUPPORT */

#include <termios.h>

static int tty_cbreak(int fd)
{
    struct termios ios;

    if (tcgetattr(fd, &ios) < 0)
    {
        return(-1);
    }

    BDBG_MSG(("Inital termios settings - c_lflag:%x c_cc[VMIN]:%x c_cc[VTIME]:%x",
              ios.c_lflag, ios.c_cc[VMIN], ios.c_cc[VTIME]));
    ios.c_lflag    &= ~(ICANON | ECHO);
    ios.c_cc[VMIN]  = 1;
    ios.c_cc[VTIME] = 0;
    BDBG_MSG(("Modified termios settings - c_lflag:%x c_cc[VMIN]:%x c_cc[VTIME]:%x",
              ios.c_lflag, ios.c_cc[VMIN], ios.c_cc[VTIME]));

    return(tcsetattr(fd, TCSAFLUSH, &ios));
} /* tty_cbreak */

static int tty_reset(int fd)
{
    struct termios ios;

    if (tcgetattr(fd, &ios) < 0)
    {
        return(-1);
    }

    ios.c_lflag    |= (ICANON | ECHO);
    ios.c_cc[VMIN]  = 1;
    ios.c_cc[VTIME] = 0;

    BDBG_MSG(("Restored termios settings - c_lflag:%x c_cc[VMIN]:%x c_cc[VTIME]:%x",
              ios.c_lflag, ios.c_cc[VMIN], ios.c_cc[VTIME]));

    return(tcsetattr(fd, TCSANOW, &ios));
} /* tty_reset */

int kbdhit(void)
{
    fd_set         rfds;
    struct timeval tv;

    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO, &rfds);
    tv.tv_sec  = 0;
    tv.tv_usec = 0;

    return(select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv) && FD_ISSET(STDIN_FILENO, &rfds));
}

void luaShell(void * pParam)
{
    CLua *           pLua        = (CLua *)pParam;
    int              lerror      = 0;
    char *           line        = NULL;
    char *           prompt      = NULL;
    CConfiguration * pCfg        = NULL;
    char *           pScriptName = NULL;

    BDBG_ASSERT(NULL != pLua);

    pCfg = pLua->getCfg();
    BDBG_ASSERT(NULL != pCfg);

    /* if script name is set, then load and run it */
    pScriptName = (char *)GET_STR(pCfg, LUA_SCRIPT);
    if ((NULL != pScriptName) && (0 < strlen(pScriptName)))
    {
        BDBG_WRN(("Running script: %s", pScriptName));
        lerror = luaL_loadfile(pLua->getLuaState(), pScriptName);
        if (lerror)
        {
            BDBG_WRN(("LUA error: %s", lua_tostring(pLua->getLuaState(), -1)));
            lua_pop(pLua->getLuaState(), 1);
        }
        else
        {
            lerror = lua_pcall(pLua->getLuaState(), 0, 0, 0);
            if (lerror)
            {
                BDBG_WRN(("LUA error: %s", lua_tostring(pLua->getLuaState(), -1)));
                lua_pop(pLua->getLuaState(), 1);
            }
        }
    }

#ifdef LINENOISE_SUPPORT
    prompt = (char *)GET_STR(pCfg, LUA_PROMPT);

    /* callback is used for autocompletion feature */
    linenoiseSetCompletionCallback(luaShellCompletionCallback);

    /* load past history from file */
    linenoiseHistoryLoad((char *)GET_STR(pCfg, LUA_HISTORY_FILENAME));

    tty_cbreak(STDIN_FILENO);
    while (true == pLua->_threadRun)
    {
        char lineLast[256];
        memset(lineLast, 0, sizeof(lineLast));

        fprintf(stdout, "\r%s", prompt);
        fflush(stdout);
        BKNI_Sleep(pLua->_pollInterval);

        /* non-blocking check for key hit but do not read - linenoise will read the value after first
         * changing the terminal settings */
        if (kbdhit())
        {
            /* use linenoise prompt to get user input.  linenoise has autocomplete (tab) as well as
             * persistent history (up/down arrows) capabilites. */
            line = linenoise(prompt);

            if ((NULL != line) && (pLua->_threadRun))
            {
                if ('\0' != line[0])
                {
                    BDBG_MSG(("echo: '%s' len:%d\n", line, strnlen(line, 255)));

                    /* chance to handle input command or pass it to lua */
                    if (false == pLua->handleInput(line))
                    {
                        /* pass entered line to lua */
                        lerror  = luaL_loadbuffer(pLua->getLuaState(), line, strlen(line), "line");
                        lerror |= lua_pcall(pLua->getLuaState(), 0, 0, 0);
                        if (lerror)
                        {
                            BDBG_WRN(("LUA error: %s", lua_tostring(pLua->getLuaState(), -1)));
                            lua_pop(pLua->getLuaState(), 1);
                        }
                        else
                        {
                            if ((0 != strncmp(lineLast, line, 255)) &&
                                (0 < strnlen(line, 255)))
                            {
                                /* add entered line to history (if different than the last command) and save */
                                linenoiseHistoryAdd(line);
                                linenoiseHistorySave((char *)GET_STR(pCfg, LUA_HISTORY_FILENAME));
                                strncpy(lineLast, line, 255);
                            }
                        }
                    }
                }

                /* linenoise allocates memory for the user generated line, we must free it here */
                free(line);
            }
        }
    }
    tty_reset(STDIN_FILENO);
#endif /* ifdef LINENOISE_SUPPORT */

    pLua->setStartState(false);
} /* luaShell */

/* bwin io callback that is executed when lua io is triggered */
void bwinLuaCallback(
        void *       pObject,
        const char * strCallback
        )
{
    eRet      ret  = eRet_Ok;
    CLua *    pLua = (CLua *)pObject;
    CAction * pAction;

    BDBG_ASSERT(NULL != pLua);
    BSTD_UNUSED(strCallback);

    /* handle all queued lua events */
    while (NULL != (pAction = pLua->removeAction()))
    {
        BDBG_MSG(("Notify Observers for Lua event code: %s", notificationToString(pAction->getId()).s()));
        ret = pLua->notifyObservers(pAction->getId(), (void *)pAction->getDataPtr());
        CHECK_ERROR_GOTO("error notifying observers", ret, error);

        /* delete lua event and any associated data */
        delete pAction;
    }

error:
    return;
} /* bwinLuaCallback */

eRet CLua::trigger(CAction * pAction)
{
    eRet            ret           = eRet_Ok;
    CWidgetEngine * pWidgetEngine = getWidgetEngine();

    BDBG_ASSERT(NULL != pAction);

    /* save copy of lua event in case we have to wait for a response notification */
    _busyAction = *pAction;

    B_Event_Reset(_busyEvent);

    if (NULL != pWidgetEngine)
    {
        BDBG_MSG(("Trigger Lua event: %s", notificationToString(pAction->getId()).s()));

        pWidgetEngine->syncCallback(this, CALLBACK_LUA);
        CHECK_ERROR("unable to sync with bwin main loop", ret);
    }

    /*** do not access pAction after the syncCallback call.  since we are running
     *   in the Lua thread context, the bwinLuaCallback can trigger at anytime
     *   after syncCallback() which will delete the pAction! use _busyAction instead. ***/

    /* if command specified a valid wait notification, we will wait for it here */
    if (eNotify_Invalid != _busyAction.getWaitNotification())
    {
        B_Error berr = B_ERROR_SUCCESS;

        BDBG_MSG(("Lua waiting for response (%s)...%d secs",
                  notificationToString(_busyAction.getWaitNotification()).s(),
                  _busyAction.getWaitTimeout()));

        berr = B_Event_Wait(_busyEvent, _busyAction.getWaitTimeout());
        if (B_ERROR_SUCCESS != berr)
        {
            BDBG_ERR(("Lua wait timed out or returned error!"));
            ret = eRet_Timeout;
        }
        else
        {
            BDBG_MSG(("Received Lua command (%s)", notificationToString(_busyAction.getWaitNotification()).s()));
        }
    }

    return(ret);
} /* trigger */

bool CLua::handleInput(char * pLine)
{
    bool ret = false;

    BDBG_ASSERT(NULL != getWidgetEngine());

    if (NULL == pLine)
    {
        return(ret);
    }

    if (0 == strncasecmp("exit", pLine, strlen("exit")))
    {
        atlasLua_Exit(getLuaState());

        /* mark as handled so it will not be forwarded on to lua */
        ret = true;
    }
    else
    if (0 == strncasecmp("quit", pLine, strlen("quit")))
    {
        atlasLua_Exit(getLuaState());

        /* mark as handled so it will not be forwarded on to lua */
        ret = true;
    }
    else
    if (0 == strncasecmp("help", pLine, strlen("help")))
    {
        printf("Help:\n");
        printf("\t<tab>\t\tAutocomplete\n");
        printf("\tatlas.<tab>\tAutocomplete atlas specific Lua functions\n");
        printf("\t<arrow Up/Down>\tHistory\n");
        printf("\t<help>\t\tShow Help\n");
        printf("\t<exit>\t\tQuit Application\n");

        /* mark as handled so it will not be forwarded on to lua */
        ret = true;
    }

    return(ret);
} /* handleInput */

void CLua::run(CWidgetEngine * pWidgetEngine)
{
    B_ThreadSettings settings;

    if (true == getStartState())
    {
        BDBG_WRN(("attempting to start lua when already started - ignored."));
        return;
    }

    _pWidgetEngine = pWidgetEngine;
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->addCallback(this, CALLBACK_LUA, bwinLuaCallback, ePowerMode_S3);
    }

    /* must set thread start state before starting thread or it will simply exit */
    _threadRun = true;

    B_Thread_GetDefaultSettings(&settings);
    _threadShell = B_Thread_Create("lua", luaShell, (void *)this, &settings);

    setStartState(true);
} /* run */

void CLua::stop()
{
    if (false == getStartState())
    {
        return;
    }

    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_LUA);
        _pWidgetEngine = NULL;
    }

    /* attempt to stop thread gracefully */
    _threadRun = false;
    BKNI_Sleep(_pollInterval + 1);

    if (NULL != _threadShell)
    {
        B_Thread_Destroy(_threadShell);
        _threadShell = NULL;
    }

    setStartState(false);
} /* stop */

void CLua::processNotification(CNotification & notification)
{
    BDBG_ASSERT(eNotify_Invalid != notification.getId());

    switch (notification.getId())
    {
#if NEXUS_HAS_FRONTEND
    case eNotify_ScanStarted:
    {
        CTunerScanNotificationData * pScanData = (CTunerScanNotificationData *)notification.getData();
        if (NULL != pScanData)
        {
            BDBG_MSG(("Tuner name:%s type:%d num:%u - Channel scan started",
                      pScanData->getTuner()->getName(),
                      pScanData->getTuner()->getType(),
                      pScanData->getTuner()->getNumber()));
        }
    }
    break;
    case eNotify_ScanStopped:
    {
        CTunerScanNotificationData * pScanData = (CTunerScanNotificationData *)notification.getData();
        if (NULL != pScanData)
        {
            BDBG_MSG(("Tuner name:%s type:%d num:%u - Channel scan stopped",
                      pScanData->getTuner()->getName(),
                      pScanData->getTuner()->getType(),
                      pScanData->getTuner()->getNumber()));
        }
        else
        {
            BDBG_WRN(("scan stopped for unknown/invalid tuner"));
        }
    }
    break;
    case eNotify_ScanProgress:
    {
        CTunerScanNotificationData * pScanData = (CTunerScanNotificationData *)notification.getData();
        if (NULL != pScanData)
        {
            BDBG_MSG(("Tuner name:%s type:%d num:%u - Channel scan %u%% complete",
                      pScanData->getTuner()->getName(),
                      pScanData->getTuner()->getType(),
                      pScanData->getTuner()->getNumber(),
                      pScanData->getPercent()));
        }
    }
    break;
#endif /* NEXUS_HAS_FRONTEND */

#ifdef PLAYBACK_IP_SUPPORT
    case eNotify_DiscoveredPlaylistsShown:
    {
        CPlaylist * pPlaylist = (CPlaylist *)notification.getData();
        int         nRetVals  = 0;

        /* this notification is a response from an Atlas model class - retrieve return
         * data and push to the Lua stack for return to the calling Lua function.  Also
         * update the number of return values. */
        if (NULL != pPlaylist)
        {
            CChannel * pChannel = NULL;
            if (NULL != (pChannel = pPlaylist->getChannel(0)))
            {
                lua_pushlstring(_pLua, pChannel->getHost().s(), pChannel->getHost().length());
                nRetVals++;
                lua_pushlstring(_pLua, pPlaylist->getName().s(), pPlaylist->getName().length());
                nRetVals++;
            }
        }
        else
        {
            lua_pushnil(_pLua);
            nRetVals++;
            lua_pushnil(_pLua);
            nRetVals++;
        }

        /* add 2 to count of return values */
        _busyAction.setNumReturnVals(_busyAction.getNumReturnVals() + nRetVals);
    }
    break;

    case eNotify_PlaylistShown:
    {
        CChannelBip * pChannelBip = (CChannelBip *)notification.getData();
        int           nRetVals    = 0;

        /* this notification is a response from an Atlas model class - retrieve return
         * data and push to the Lua stack for return to the calling Lua function.  Also
         * update the number of return values. */
        if (NULL != pChannelBip)
        {
            lua_pushlstring(_pLua, pChannelBip->getUrl().s(), pChannelBip->getUrl().length());
            nRetVals++;
        }
        else
        {
            lua_pushnil(_pLua);
            nRetVals++;
        }

        /* add 2 to count of return values */
        _busyAction.setNumReturnVals(_busyAction.getNumReturnVals() + nRetVals);
    }
    break;
#endif /* ifdef PLAYBACK_IP_SUPPORT */

    default:
        break;
    } /* switch */

    /* check to see if the current notification is one that a pending lua command is waiting on */
    {
        int           i          = 0;
        eNotification busyNotify = _busyAction.getWaitNotification(i);

        while (eNotify_Invalid != busyNotify)
        {
            if (notification.getId() == busyNotify)
            {
                B_Event_Set(_busyEvent);
                break;
            }

            i++;
            busyNotify = _busyAction.getWaitNotification(i);
        }
    }
} /* processNotification */
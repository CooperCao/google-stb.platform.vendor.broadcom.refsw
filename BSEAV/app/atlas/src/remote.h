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

#ifndef REMOTE_H__
#define REMOTE_H__

#include "nexus_ir_input.h"
#if NEXUS_HAS_UHF_INPUT
#include "nexus_uhf_input.h"
#endif
#include "bwidgets.h"
#include "atlas_cfg.h"
#include "resource.h"
#include "widget_engine.h"
#ifdef NETAPP_SUPPORT
#include "netapp_os.h"
#include <linux/input.h>
#include <linux/keyboard.h>
#include <fcntl.h>
#include "b_os_lib.h"
/*#include <sstream> */
#include <string.h>
#include <errno.h>
#endif /* ifdef NETAPP_SUPPORT */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * dtt todo - add ascii codes 0x20 up to 0x7f for text input
 * changes to eKey enum must also be reflected in eKey2bwidgets
 */
typedef enum eKey
{
    eKey_Invalid,
    eKey_Select,
    eKey_UpArrow,
    eKey_DownArrow,
    eKey_LeftArrow,
    eKey_RightArrow,
    eKey_Backspace,
    eKey_Delete,
    eKey_ChannelUp,
    eKey_ChannelDown,
    eKey_VolumeUp,
    eKey_VolumeDown,
    eKey_Mute,
    eKey_Play,
    eKey_Stop,
    eKey_Pause,
    eKey_Rewind,
    eKey_FastForward,
    eKey_Record,
    eKey_Menu,
    eKey_Info,
    eKey_Exit,
    eKey_Dot,
    eKey_Enter,
    eKey_Last,
    eKey_Pip,
    eKey_Swap,
    eKey_JumpFwd,
    eKey_JumpRev,
    eKey_Power,
    /* ascii codes */
    eKey_0,
    eKey_1,
    eKey_2,
    eKey_3,
    eKey_4,
    eKey_5,
    eKey_6,
    eKey_7,
    eKey_8,
    eKey_9,
    eKey_Max
} eKey;

/*
 * mapping to bwidget equivalents
 * dtt todo - add ascii codes 0x20 up to 0x7f for text input
 */
static const bwidget_key eKey2bwidgets[eKey_Max] = {
    bwidget_key_invalid,
    bwidget_key_select,
    bwidget_key_up,
    bwidget_key_down,
    bwidget_key_left,
    bwidget_key_right,
    bwidget_key_backspace,
    bwidget_key_delete,
    bwidget_key_chup,
    bwidget_key_chdown,
    bwidget_key_volup,
    bwidget_key_voldown,
    bwidget_key_mute,
    bwidget_key_play,
    bwidget_key_stop,
    bwidget_key_pause,
    bwidget_key_rewind,
    bwidget_key_fastforward,
    bwidget_key_record,
    bwidget_key_menu,
    bwidget_key_info,
    bwidget_key_exit,
    bwidget_key_dot,
    bwidget_key_enter,
    bwidget_key_last,
    bwidget_key_pip,
    bwidget_key_swap,
    bwidget_key_jumpforward,
    bwidget_key_jumpreverse,
    bwidget_key_power,
    /* ascii codes */
    bwidget_key_0,
    bwidget_key_1,
    bwidget_key_2,
    bwidget_key_3,
    bwidget_key_4,
    bwidget_key_5,
    bwidget_key_6,
    bwidget_key_7,
    bwidget_key_8,
    bwidget_key_9
};

class CRemoteEvent
{
public:
    CRemoteEvent(void) : _code(0), _repeat(false) {}
    CRemoteEvent(CRemoteEvent * pEvent) { setCode(pEvent->getCode()); setRepeat(pEvent->isRepeat()); }

    uint32_t getCode(void)          { return(_code); }
    void     setCode(uint32_t code) { _code = code; }
    bool     isRepeat(void)         { return(_repeat); }
    void     setRepeat(bool repeat) { _repeat = repeat; }

protected:
    uint32_t _code;
    bool     _repeat;
};

class CRemote : public CResource
{
public:
    CRemote(
            const char *         name,
            const uint16_t       number,
            const eBoardResource type,
            CConfiguration *     pCfg
            ) :
        CResource(name, number, type, pCfg),
        _pWidgetEngine(NULL) {}
    ~CRemote(void) {}

    virtual eRet open(CWidgetEngine * pWidgetEngine) = 0;
    virtual void close(void)                         = 0;
    virtual void dump(void)                          = 0;
    virtual eRet getEvent(CRemoteEvent * pEvent)     = 0;

    CWidgetEngine * getWidgetEngine(void) { return(_pWidgetEngine); }
    void            remoteCallback(void);
    void            submitCode(CRemoteEvent * pEvent);
    eRet            addEvent(CRemoteEvent * pEvent);
    eRet            removeEvent(CRemoteEvent * pEvent);
    static void     displayRf4ceRemotes(void);
    static void     addRf4ceRemote(const char * remote_name);
    static void     removeRf4ceRemote(int pairingRefNum);

protected:
    CWidgetEngine *         _pWidgetEngine;
    MAutoList<CRemoteEvent> _eventList;
};

class CIrRemote : public CRemote
{
public:
    CIrRemote(
            const char *     name,
            const uint16_t   number,
            CConfiguration * pCfg
            );
    ~CIrRemote(void);

    virtual eRet              open(CWidgetEngine * pWidgetEngine);
    virtual void              close(void);
    virtual eRet              getEvent(CRemoteEvent * pEvent);
    virtual eRet              setMode(NEXUS_IrInputMode mode);
    virtual NEXUS_IrInputMode getMode(void);
    virtual void              dump();

    char *              getRemoteCodeName(eKey key);
    eKey                convertRemoteCode(NEXUS_IrInputMode mode, uint32_t code);
    NEXUS_IrInputHandle getIrRemote(void) { return(_irInput); }

protected:
    NEXUS_IrInputHandle _irInput;
};

#if RF4CE_SUPPORT

class CRf4ceRemoteData
{
public:
    CRf4ceRemoteData() :
        _pairingRefNum(0),
        _name() {}
public:
    int     _pairingRefNum;
    MString _name;
};

class CRf4ceRemote : public CRemote
{
public:
    CRf4ceRemote(
            const char *     name,
            const uint16_t   number,
            CConfiguration * pCfg
            );
    ~CRf4ceRemote(void);

    virtual eRet open(CWidgetEngine * pWidgetEngine);
    virtual void close(void);
    virtual eRet getEvent(CRemoteEvent * pEvent);
    virtual void dump();

    char * getRemoteCodeName(eKey key);
    eKey   convertRemoteCode(uint8_t code);

protected:
    int _rf4ceInput;
};
#endif /* RF4CE_SUPPORT */
#ifdef NETAPP_SUPPORT
#define LINUX_INPUT_NODE  "/dev/input/"

typedef struct hidInputSource {
    char   devName[12];
    int    fd;
    bool   shiftPressed;
    int    taskId;
    bool   active;
} hidInputSource;

class CBluetoothRemote : public CRemote
{
public:
    CBluetoothRemote(
            const char *     name,
            const uint16_t   number,
            CConfiguration * pCfg
            );
    ~CBluetoothRemote(void);

    virtual eRet open(CWidgetEngine * pWidgetEngine);
    virtual void close(void);
    virtual eRet getEvent(CRemoteEvent * pEvent);
    virtual void dump();

    void saveToPrevEvents(struct input_event event);
    bool comparePrevEvents(struct input_event event);

    eRet   getEvent(CRemoteEvent * pEvent, hidInputSource * pHidInputSource);
    char * getRemoteCodeName(eKey key);
    eKey   convertRemoteCode(uint32_t code);
    void   thread(CBluetoothRemote * pBluetoothRemote);
    void   setDeviceName(char * devName) { strncpy(_devName, devName, sizeof(_devName)); }
    eRet   addInputSource(char * devName);
    eRet   removeInputSource(char * devName);

protected:
    char _devName[12];
    bool _shutdown;

    int                       _fd;
    bool                      _shiftPressed;
    void *                    _taskId;
    B_MutexHandle             _hMutex;
    MAutoList<hidInputSource> _hidInputSourceList;
    struct input_event        prev_event;
    void *                    bt_semaphore;
};
#endif /* NETAPP_SUPPORT */

#if NEXUS_HAS_UHF_INPUT
class CUhfRemote : public CRemote
{
public:
    CUhfRemote(
            const char *     name,
            const uint16_t   number,
            CConfiguration * pCfg
            );
    ~CUhfRemote(void);

    virtual eRet                  open(CWidgetEngine * pWidgetEngine);
    virtual void                  close(void);
    virtual eRet                  getEvent(CRemoteEvent * pEvent);
    virtual eRet                  setChannel(NEXUS_UhfInputChannel channel);
    virtual NEXUS_UhfInputChannel getChannel(void);
    virtual void                  dump();

    char *               getRemoteCodeName(eKey key);
    eKey                 convertRemoteCode(uint32_t code);
    NEXUS_UhfInputHandle getUhfRemote(void) { return(_uhfInput); }

protected:
    NEXUS_UhfInputHandle  _uhfInput;
    NEXUS_UhfInputChannel _channel;
};
#endif /* if NEXUS_HAS_UHF_INPUT */

#ifdef __cplusplus
}
#endif

#endif /* REMOTE_H__ */
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

#ifndef RESOURCE_H__
#define RESOURCE_H__

#include "atlas.h"
#include "atlas_cfg.h"
#include "mvc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* changes made to this enum may require changes to isXXXXX() methods in the CResource class. */
typedef enum eBoardResource
{
    eBoardResource_display,
    eBoardResource_graphics,
    eBoardResource_surfaceClient,
    eBoardResource_decodeVideo,
    eBoardResource_simpleDecodeVideo,
    eBoardResource_decodeAudio,
    eBoardResource_simpleDecodeAudio,
    eBoardResource_stcChannel,
    eBoardResource_pcmPlayback,
    eBoardResource_pcmCapture,
    eBoardResource_decodeStill,
    eBoardResource_decodeEs,
    eBoardResource_decodeMosaic,
    eBoardResource_streamer,
#ifdef PLAYBACK_IP_SUPPORT
    eBoardResource_ip,
    eBoardResource_http,
    eBoardResource_live,
#endif
#if NEXUS_HAS_FRONTEND
    eBoardResource_frontendQam,
    eBoardResource_frontendVsb,
    eBoardResource_frontendSds,
    eBoardResource_frontendIp,
    eBoardResource_frontendOfdm,
    eBoardResource_frontendOob,
    eBoardResource_frontendUpstream,
#endif /* if NEXUS_HAS_FRONTEND */
    eBoardResource_linein,
    eBoardResource_recpump,
    eBoardResource_record,
    eBoardResource_recordPes,
    eBoardResource_recordTsdma,
#if defined NEXUS_HAS_DMA || defined NEXUS_HAS_XPT_DMA
    eBoardResource_dma,
#endif
    eBoardResource_timebase,
    eBoardResource_playpump,
#if DVR_LIB_SUPPORT
    eBoardResource_tsb,
#endif
    eBoardResource_playback,
    eBoardResource_encode,
    eBoardResource_inputBand,
    eBoardResource_parserBand,
    eBoardResource_irRemote,
#if RF4CE_SUPPORT
    eBoardResource_rf4ceRemote,
#endif
    eBoardResource_uhfRemote,
    eBoardResource_power,
#ifdef WPA_SUPPLICANT_SUPPORT
    eBoardResource_wifi,
#endif
#ifdef NETAPP_SUPPORT
    eBoardResource_network,
    eBoardResource_bluetooth,
    eBoardResource_bluetoothRemote,
#endif
    eBoardResource_outputSpdif,
    eBoardResource_outputAudioDac,
    eBoardResource_outputAudioDacI2s,
    eBoardResource_outputAudioDummy,
    eBoardResource_outputComponent,
    eBoardResource_outputSVideo,
    eBoardResource_outputComposite,
    eBoardResource_outputRFM,
    eBoardResource_outputHdmi,

    eBoardResource_max /* invalid */
} eBoardResource;

class CResource : public CMvcModel
{
public:
    CResource(
            const char *         name,
            const uint16_t       number,
            const eBoardResource type,
            CConfiguration *     pCfg
            );
    ~CResource(void);

    virtual eRet initialize(void) { _bInit = true; return(eRet_Ok); }
    virtual void dump(void);

    const char *   getName(void)                { return(_name.s()); }
    uint16_t       getNumber(void)              { return(_number); }
    eBoardResource getType(void)                { return(_type); }
    void           setType(eBoardResource type) { _type = type; }
    bool           isOutput(void);
    bool           isVideoOutput(void);
    bool           isAudioOutput(void);
    bool           isFrontend(void);
    eRet           setCheckedOut(bool checkedOut, void * id = NULL);
    void           setCheckedOutMax(uint16_t max) { _checkedOutMax = max; }
    uint16_t       getCheckedOutMax(void)         { return(_checkedOutMax); }
    void *         getCheckedOutId(void)          { return(_checkedOutId); }
    bool           isCheckedOut(void)             { return(0 < _checkedOut); }
    bool           isAvailForCheckout(void)       { return(_checkedOut < _checkedOutMax); }
    void           setReserved(void * id)         { _reserveId = id; }
    bool           isReserved(void)               { return(NULL != _reserveId); }
    bool           isReservedMatch(void * id)     { return(_reserveId == id); }
    bool           validReservation(void * id)    { return((_reserveId == id) || (_reserveId == NULL)); }
    bool           isInitialized(void)            { return(_bInit); }

    ENUM_TO_MSTRING_DECLARE(boardResourceToString, eBoardResource)

protected:
    MString          _name;
    uint16_t         _number;
    eBoardResource   _type;
    CConfiguration * _pCfg;
    uint16_t         _checkedOut;
    uint16_t         _checkedOutMax;
    void *           _reserveId;
    void *           _checkedOutId;
    bool             _bInit;
};

#ifdef __cplusplus
}
#endif

#endif /* #ifndef RESOURCE_H__ */
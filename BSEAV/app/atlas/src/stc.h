/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#ifndef STC_H__
#define STC_H__

#include "nexus_types.h"
#include "nexus_simple_stc_channel.h"

#include "nexus_timebase.h"

#include "atlas_cfg.h"
#include "resource.h"
#include "pid.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum eStcType
{
    eStcType_ParserBand,
    eStcType_ParserBandSyncOff,
    eStcType_TtsPacing,
    eStcType_PcrPacing,
    eStcType_IpLivePlayback,
    eStcType_IpPullPlayback,
    eStcType_PvrPlayback
} eStcType;

class CStc : public CResource
{
public:
    CStc(
            const char *     name,
            const uint16_t   number,
            CConfiguration * pCfg
            );
    ~CStc(void);

    eRet                         open(void);
    void                         close(void);
    eRet                         open(NEXUS_StcChannelSettings settings);
    void                         getDefaultSettings(NEXUS_SimpleStcChannelSettings * pSettings);
    eRet                         setSettings(NEXUS_SimpleStcChannelSettings * settings);
    eRet                         configure(CPid * pPcrPid = NULL);
    CPid *                       getPid(void)                                        { return(_pPcrPid); }
    eStcType                     getStcType()                                        { return(_stcType); }
    void                         setStcType(eStcType stcType)                        { _stcType = stcType; }
    void                         setTransportType(NEXUS_TransportType transportType) { _transportType = transportType; }
    NEXUS_SimpleStcChannelHandle getSimpleStcChannel(void)                           { return(_simpleStcChannel); }
    NEXUS_StcChannelHandle       getStcChannel(void)                                 { return(_stcChannel); }

protected:
    NEXUS_SimpleStcChannelHandle _simpleStcChannel;
    NEXUS_StcChannelHandle       _stcChannel;
    NEXUS_TransportType          _transportType;
    eStcType                     _stcType;
    CPid * _pPcrPid;
};

#ifdef __cplusplus
}
#endif

#endif /* STC_H__ */
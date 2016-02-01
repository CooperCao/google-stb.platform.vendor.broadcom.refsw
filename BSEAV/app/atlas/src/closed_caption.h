/***************************************************************************
 * (c) 2002-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
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
 *
 *****************************************************************************/

#ifndef CLOSED_CAPTION_H__
#define CLOSED_CAPTION_H__

#include "pthread.h"
#include "b_dcc_lib.h"
#include "atlas_main.h"
#include "nexus_video_input_vbi.h"
#include "parse_userdata.h"
#include "mvc.h"

#define CC_DATA_BUF_SIZE  128
#define MAX_INPUT_CHARS   128
#define MAX_LENGTH_FILE   64  /* max num characters for a playback stream file name */

/*
** Display mode constants.
*/
typedef enum CCTest_Display_Modes
{
    CCTest_Display_480i,
    CCTest_Display_480p,
    CCTest_Display_720p,
    CCTest_Display_1080i,
    CCTest_Display_Max_Size
} CCTest_Display_Modes;

typedef struct CCTest_Caption_Triplets
{
    uint8_t   ccType;
    uint8_t   data[2];
} CCTest_Caption_Triplets;

typedef struct digitalCC_setting {
    bool                       enabled;
    bool                       automode;
    B_Dcc_Type                 ccMode;
    uint32_t                   ccService;
    bool                       dispOn;
    B_Dcc_OverRides            overrideValues;
    unsigned int               overrideMask;
    NEXUS_VideoFormat          format;
    NEXUS_DisplayAspectRatio   aspectRatio;
    int                        width;
    int                        height;
} digitalCC_setting;

typedef struct digitalCC {
    BDCC_WINLIB_OpenSettings         openSettings;
    NEXUS_SimpleVideoDecoderHandle   videodecoder;
    B_Dcc_Handle                     ccEngine;
    BDCC_WINLIB_Handle               winlib;
    BDCC_WINLIB_Interface            winlibInterface;
    BKNI_EventHandle                 ccDataEvent;
    CCTest_Display_Modes             displaymode;
    bool                             exit;
    pthread_t                        main_thread;
    digitalCC_setting                setting;
    BKNI_MutexHandle                 mutex;
    buserdata_t                      userdata;
} digitalCC;

class CClosedCaption : public CMvcModel {
public:
    CClosedCaption(void);
    digitalCC_setting * dcc_getsetting(void);
    int                 dcc_set(digitalCC_setting * setting);
    eRet                dcc_init(CConfig * pConfig, CModel * pModel);
    void                dcc_uninit();
    void                dcc_reset();

    CSurfaceClient * getSurfaceClient(void) { return(_pSurfaceClientDcc); }
    void             setId(void * id)       { _id = id; }
protected:
    CDisplay *        _pDisplay;
    digitalCC *       _dcc;
    CSurfaceClient *  _pSurfaceClientDcc;
    CBoardResources * _pBoardResources;
    void *            _id;
};

#endif /* ifndef CLOSED_CAPTION_H__ */
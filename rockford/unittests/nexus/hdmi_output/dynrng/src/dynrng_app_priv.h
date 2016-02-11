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
 *
 * Module Description:
 *
******************************************************************************/

#include "nexus_platform.h"
#include "nexus_hdmi_output.h"
#include "nexus_video_decoder.h"
#include "nexus_playback.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_graphics2d.h"
#include "nexus_surface.h"
#include "nexus_core_utils.h"
#include "bstd.h"
#include "bkni.h"
#include "blst_queue.h"
#include "dynrng_args.h"
#include "dynrng_app.h"
#include "dynrng_shell.h"
#include "dynrng_osd.h"
#include "dynrng_smd.h"
#include "dynrng_utils.h"

#ifndef DYNRNG_APP_PRIV_H__
#define DYNRNG_APP_PRIV_H__

#define APP_MAX_INPUT_LEN 256

#define APP_CMD_eAuto   0x8000
#define APP_CMD_eInput  0x8001

typedef enum APP_TrickMode
{
    APP_TrickMode_ePause,
    APP_TrickMode_eResume,
    APP_TrickMode_eMax
} APP_TrickMode;

typedef enum APP_MdcvRequest
{
    APP_MdcvRequest_eFile,
    APP_MdcvRequest_eInput,
    APP_MdcvRequest_eBt2020,
    APP_MdcvRequest_eBt709,
    APP_MdcvRequest_ePrimaries,
    APP_MdcvRequest_eRed,
    APP_MdcvRequest_eGreen,
    APP_MdcvRequest_eBlue,
    APP_MdcvRequest_eWhite,
    APP_MdcvRequest_eMaxLuma,
    APP_MdcvRequest_eMinLuma,
    APP_MdcvRequest_eMax
} APP_MdcvRequest;

typedef enum APP_StatusRequest
{
    APP_StatusRequest_eArgs,
    APP_StatusRequest_eNexus,
    APP_StatusRequest_eStreamInfo,
    APP_StatusRequest_eEotf,
    APP_StatusRequest_eDrmInfoFrame,
    APP_StatusRequest_eEdid,
    APP_StatusRequest_eOsd,
    APP_StatusRequest_eSmd,
    APP_StatusRequest_eShell,
    APP_StatusRequest_eMax
} APP_StatusRequest;

typedef enum APP_Eotf
{
    APP_Eotf_eInput,
    APP_Eotf_eSdr,
    APP_Eotf_eHdr,
    APP_Eotf_eSmpte,
    APP_Eotf_eArib,
    APP_Eotf_eMax
} APP_Eotf;

struct APP_App
{
    ARGS_Args args;

    NEXUS_FilePlayHandle input;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    APP_TrickMode trickMode;

    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoStartSettings;
    bool lastStreamInfoValid;
    NEXUS_VideoDecoderStreamInformation lastStreamInfo;

    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderStartSettings audioStartSettings;

    NEXUS_StcChannelHandle stc;

    NEXUS_VideoWindowHandle window;
    NEXUS_DisplayHandle display;

    NEXUS_HdmiOutputHandle hdmi;
    bool sinkEdidValid;
    NEXUS_HdmiOutputEdidData sinkEdid;
    NEXUS_HdmiDynamicRangeMasteringInfoFrame drmInfoFrame;
    APP_Eotf eotf;

    OSD_OsdHandle osd;

    SMD_SmdHandle smd;

    SHELL_ShellHandle shell;
};

int APP_ApplyDrmInfoFrame(APP_AppHandle app);
int APP_ApplyColorSettings(APP_AppHandle app);
void APP_StreamChangedCallback(void * context, int param);

void APP_PrintEdid(APP_AppHandle app);
void APP_PrintNexus(APP_AppHandle app);
void APP_Print(APP_AppHandle app);

int APP_SetupShell(APP_AppHandle app);

int APP_SetupSmdCommand(APP_AppHandle app);
int APP_ApplySmdSource(APP_AppHandle app, SMD_SmdSource source);
int APP_ToggleSmd(APP_AppHandle app);
int APP_SetSmdFilePath(APP_AppHandle app);
int APP_DoSmd(void * context, char * args);

APP_Eotf APP_NexusEotfToApp(NEXUS_VideoEotf nxEotf);
NEXUS_VideoEotf APP_AppEotfToNexus(APP_Eotf eotf);
void APP_PrintEotf(APP_AppHandle app);
int APP_ToggleEotf(APP_AppHandle app);
const char * APP_GetEotfName(APP_Eotf eotf);
APP_Eotf APP_ParseEotf(const char * eotfStr);
int APP_SetEotf(APP_AppHandle app, APP_Eotf eotf);
int APP_DoEotf(void * context, char * args);

int APP_SetupMatrixCoefficientsCommand(APP_AppHandle app);
NEXUS_MatrixCoefficients APP_ParseMatrixCoefficients(const char * matrixCoefficientsStr);
const char * APP_GetMatrixCoefficientsName(NEXUS_MatrixCoefficients matrixCoefficients);

int APP_SetupGfxSdr2HdrCommand(APP_AppHandle app);

int APP_SetupEotfCommand(APP_AppHandle app);

OSD_Eotf APP_NexusEotfToOsd(NEXUS_VideoEotf nxEotf);
int APP_SetupOsdCommand(APP_AppHandle app);
int APP_UpdateOsd(APP_AppHandle app);
int APP_DoOsd(void * context, char * args);

int APP_SetupStatusCommand(APP_AppHandle app);
APP_StatusRequest APP_ParseStatusRequest(const char * statusStr);
int APP_DoStatus(void * context, char * args);

const char * APP_GetTrickModeName(APP_TrickMode mode);
int APP_SetTrickMode(APP_AppHandle app, APP_TrickMode mode);
int APP_DoTrick(void * context, char * args);
int APP_SetupTrickCommand(APP_AppHandle app);

#endif /* DYNRNG_APP_PRIV_H__ */

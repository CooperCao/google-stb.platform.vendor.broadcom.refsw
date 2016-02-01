/******************************************************************************* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant* to the terms and conditions of a separate, written license agreement executed
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
******************************************************************************/

#ifndef _SATFE_PLATFORM_H_
#define _SATFE_PLATFORM_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "windows.h"
#include "satfe.h"
#include "bwfe.h"


/* set SATFE_NUM_CHIPS to the number of Broadcom satellite frontend chips to be
   controlled on the board */
#define SATFE_NUM_CHIPS 1

/* define SATFE_MUTEX */
#define SATFE_MUTEX(x) \
        BKNI_AcquireMutex(SATFE_Platform_hMutex); \
        x; \
        BKNI_ReleaseMutex(SATFE_Platform_hMutex);

/* required functions to implement */
extern BERR_Code SATFE_Platform_Init();
extern BERR_Code SATFE_Platform_Shutdown();
extern void SATFE_Platform_InitDiags(void *pParam);
extern char SATFE_Platform_GetChar(bool bBlocking);
extern void SATFE_Platform_Backspace();
extern void SATFE_Platform_OnIdle();
extern void SATFE_Platform_GetInputPower(SATFE_Chip *pChip, uint32_t rfagc,
                                         uint32_t ifagc, uint32_t agf,
                                         uint32_t tuner_freq, float *pPower);
extern void SATFE_Platform_StartTimer();
extern void SATFE_Platform_KillTimer();
extern uint32_t SATFE_Platform_GetTimerCount();
extern uint32_t SATFE_Platform_Rand();

/* required data structure to initialize */
extern SATFE_ChipDescriptor SATFE_chips[SATFE_NUM_CHIPS];

/* define any other platform-specific data types, variables, etc */
extern BKNI_MutexHandle SATFE_Platform_hMutex;
extern BKNI_MutexHandle SATFE_Platform_hFtmMessageMutex;


typedef struct
{
   int   item;
   int   type;
   float min;
   float max;
   char  grid_label[5][16];
} SATFE_94538_PlotStatus;

typedef struct
{
   bool bConstellation[8];
   HWND wndConstellation[8];
   bool bPlot[8];
   HWND wndPlot[8];
   SATFE_94538_PlotStatus plotStatus[8][3];
} SATFE_94538_Impl;

typedef struct
{
   BWFE_Handle          hWfe;
   BWFE_ChannelHandle   *hWfeChannel;
   uint8_t              numChannels;
} SATFE_BWFE_Handles;


#define PLOT_ITEM_NONE          0
#define PLOT_ITEM_SNR           1
#define PLOT_ITEM_INPUT_POWER   2
#define PLOT_ITEM_CARRIER_ERROR 3
#define PLOT_ITEM_REGISTER      4
#define PLOT_ITEM_LOCK          5

#define PLOT_DATA_FLOAT    0
#define PLOT_DATA_BOOL     1
#define PLOT_DATA_SIGNED   2
#define PLOT_DATA_UNSIGNED 3

#ifdef __cplusplus
};
#endif

#endif

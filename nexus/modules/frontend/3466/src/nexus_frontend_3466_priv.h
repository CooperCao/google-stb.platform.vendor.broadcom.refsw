/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
*  USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
*  ANY LIMITED REMEDY.
*
* API Description:
*   API name: Frontend 3466
*    Internal API for a BCM3466 Tuner/Demodulator Device.
*
***************************************************************************/
#ifndef _NEXUS_FRONTEND_3466_PRIV__H_
/* General includes */
#include "nexus_frontend_module.h"
#include "priv/nexus_transport_priv.h"
#include "priv/nexus_i2c_priv.h"
#include "priv/nexus_gpio_priv.h"
#include "priv/nexus_spi_priv.h"

#include "bhab.h"
#include "bdbg.h"

#include "bhab.h"
#include "bhab_3466.h"
#include "bhab_3466_fw.h"
/* End general includes */

/* Cable and Terrestrial includes */
#include "nexus_platform_features.h"
#include "nexus_i2c.h"
#include "priv/nexus_i2c_priv.h"
#include "btnr.h"
#include "btnr_3466ib.h"
#include "bods_3466.h"
#include "bods.h"
#include "bchp_sun_top_ctrl.h"
#include "nexus_avs.h"
#include "bhab_3466.h"
/* End Cable and terrestrial includes */


#define NEXUS_FRONTEND_3466_OFDM_INPUT_BAND 2


/* REMOVE THIS HARDCODING LATER. */
#define NEXUS_MAX_3466_T_FRONTENDS 2
#define NEXUS_3466_DVBT_CHN      (0)
#define NEXUS_3466_DVBT2_CHN     (NEXUS_3466_DVBT_CHN  + 1)

#define TOTAL_SOFTDECISIONS 30

typedef struct NEXUS_3466Device
{
    BDBG_OBJECT(NEXUS_3466Device)
    BLST_S_ENTRY(NEXUS_3466Device) node;
    NEXUS_FrontendDeviceOpenSettings openSettings;
    NEXUS_FrontendDevice *pGenericDeviceHandle;
    BHAB_Handle hab;
    BHAB_Capabilities capabilities;
    void *leapBuffer;
    unsigned    isrNumber;
    NEXUS_GpioHandle gpioInterrupt;
    uint16_t    i2cAddr;
    NEXUS_I2cHandle i2cHandle;
    uint16_t    spiAddr;
    NEXUS_SpiHandle spiHandle;
    unsigned    numfrontends;
    uint16_t revId;
    bool adsOpenDrain;
    uint8_t lastChannel;
    unsigned agcValue;                            /* Gain Value*/
    NEXUS_CallbackDesc updateGainCallbackDesc;    /* Callback will be called when the gain from the lna needs to be updated. */
    BINT_CallbackHandle cbHandle;
    /* Cable-specific variables */
    struct {
        unsigned dummy;
    } cable;
    /* Terrestrial-specific variables */
    struct {
        NEXUS_FrontendHandle       frontendHandle[NEXUS_MAX_3466_T_FRONTENDS];
        NEXUS_IsrCallbackHandle    lockAppCallback[NEXUS_MAX_3466_T_FRONTENDS];
        NEXUS_IsrCallbackHandle    asyncStatusAppCallback[NEXUS_MAX_3466_T_FRONTENDS];
        bool                       isAsyncStatusReady[NEXUS_MAX_3466_T_FRONTENDS];
        bool                       isTunerPoweredOn[NEXUS_MAX_3466_T_FRONTENDS];
        NEXUS_FrontendOfdmAcquisitionMode lastAcquisitionMode[NEXUS_MAX_3466_T_FRONTENDS];
        BODS_ChannelHandle         ods_chn[NEXUS_MAX_3466_T_FRONTENDS];
        NEXUS_FrontendOfdmSettings last_ofdm[NEXUS_MAX_3466_T_FRONTENDS];
        bool                       isPoweredOn[NEXUS_MAX_3466_T_FRONTENDS];
        BTNR_Handle                tnr[NEXUS_MAX_3466_T_FRONTENDS];
        NEXUS_IsrCallbackHandle    ewsAppCallback[NEXUS_MAX_3466_T_FRONTENDS];
        NEXUS_IsrCallbackHandle    updateGainAppCallback[NEXUS_MAX_3466_T_FRONTENDS];
        signed count[NEXUS_MAX_3466_T_FRONTENDS];
        bool acquireInProgress[NEXUS_MAX_3466_T_FRONTENDS];
        bool isStatusReady[NEXUS_MAX_3466_T_FRONTENDS];
        BKNI_EventHandle isrEvent;
        NEXUS_EventCallbackHandle isrEventCallback;
        NEXUS_TunerRfInput rfInput;
        NEXUS_RfDaisyChain rfDaisyChain;
        bool enableRfLoopThrough;
        BODS_Handle ods;
        NEXUS_FrontendDvbt2Status t2PartialStatus;
        BODS_SelectiveStatus odsStatus;
        unsigned maxDvbtChannels;
    } terrestrial;
} NEXUS_3466Device;

typedef struct NEXUS_3466Channel
{
    unsigned chn_num; /* channel number */
    NEXUS_3466Device *pDevice; /* 3466 device*/
} NEXUS_3466Channel;

NEXUS_Error NEXUS_FrontendDevice_P_Get3466Settings(void *handle, NEXUS_FrontendDeviceSettings *pSettings);
NEXUS_Error NEXUS_FrontendDevice_P_Set3466Settings(void *handle, const NEXUS_FrontendDeviceSettings *pSettings);

/* Cable-specific functions */
NEXUS_Error NEXUS_FrontendDevice_Open3466_Cable(NEXUS_3466Device *pFrontendDevice);
NEXUS_FrontendHandle NEXUS_Frontend_Open3466_Cable(const NEXUS_FrontendChannelSettings *pSettings);
void NEXUS_FrontendDevice_Close3466_Cable(NEXUS_3466Device *pFrontendDevice);
NEXUS_Error NEXUS_FrontendDevice_P_Init3466_Cable(NEXUS_3466Device *pDevice);
void NEXUS_FrontendDevice_P_Uninit3466_Cable(NEXUS_3466Device *pDevice);
/* End of cable-specific function declarations */

/* Terrestrial-specific functions */
NEXUS_Error NEXUS_FrontendDevice_Open3466_Terrestrial(NEXUS_3466Device *pFrontendDevice);
NEXUS_FrontendHandle NEXUS_Frontend_Open3466_Terrestrial(const NEXUS_FrontendChannelSettings *pSettings);
void NEXUS_FrontendDevice_Close3466_Terrestrial(NEXUS_3466Device *pFrontendDevice);
NEXUS_Error NEXUS_FrontendDevice_P_Init3466_Terrestrial(NEXUS_3466Device *pDevice);
void NEXUS_FrontendDevice_P_Uninit3466_Terrestrial(NEXUS_3466Device *pDevice);
/* End of terrestrial-specific function declarations */
#endif
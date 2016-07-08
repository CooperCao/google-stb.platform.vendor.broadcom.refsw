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
 ***************************************************************************/
#ifndef _NEXUS_FRONTEND_7364_PRIV__H_
/* General includes */
#include "nexus_frontend_module.h"
#include "nexus_frontend_sat.h"
#include "priv/nexus_transport_priv.h"
#include "priv/nexus_i2c_priv.h"
#include "priv/nexus_gpio_priv.h"
#include "priv/nexus_spi_priv.h"

#include "bhab.h"
#include "bdbg.h"

#include "bhab.h"
#include "bhab_7364.h"
#include "bhab_7364_fw.h"
/* End general includes */


#if BCHP_VER >= BCHP_VER_C0
#define NEXUS_FRONTEND_7364_DISABLE_TERRESTRIAL 1
#endif

/* Cable and Terrestrial includes */
#include "nexus_platform_features.h"
#include "nexus_i2c.h"
#include "priv/nexus_i2c_priv.h"
#if !NEXUS_FRONTEND_7364_DISABLE_TERRESTRIAL
#include "btnr.h"
#include "btnr_7364ib.h"
#include "bods_7364.h"
#include "bods.h"
#include "bchp_sun_top_ctrl.h"
#include "nexus_avs.h"
#include "bhab_7364.h"
#include "bhab_7364_fw.h"
#include "btfe.h"
#include "bhab_ctfe_img.h"
#include "priv/nexus_core_img.h"
#include "priv/nexus_core_img_id.h"
#endif
/* End Cable and terrestrial includes */

/* Satellite includes */
#include "bsat.h"
#include "bdsq.h"
#include "bwfe.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_hif_cpu_intr1.h"
#include "bchp_leap_host_l1.h"
/* End satellite includes */

#define NEXUS_FRONTEND_7364_OFDM_INPUT_BAND 5
#define NEXUS_FRONTEND_7364_VSB_INPUT_BAND 6
#define NEXUS_FRONTEND_7364_SATELLITE_INPUT_BAND 18

/* REMOVE THIS HARDCODING LATER. */
#define NEXUS_MAX_7364_T_FRONTENDS 4
#define NEXUS_MAX_7364_TFE_CHANNELS 1

#define NEXUS_7364_DVBT_CHN      (0)
#define NEXUS_7364_ISDBT_CHN     (NEXUS_7364_DVBT_CHN  + 1)
#define NEXUS_7364_DVBT2_CHN     (NEXUS_7364_ISDBT_CHN  + 1)
#define NEXUS_7364_VSB_CHN       (NEXUS_7364_DVBT2_CHN  + 1)
#define NEXUS_7364_MAX_OFDM_CHN  (NEXUS_MAX_7364_T_FRONTENDS)

/* The 7364 has one downstream. */
#define NEXUS_MAX_7364_TUNERS  1

#define TOTAL_SOFTDECISIONS 30

typedef struct NEXUS_7364Device
{
    BDBG_OBJECT(NEXUS_7364Device)
    BLST_S_ENTRY(NEXUS_7364Device) node;
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
#if !NEXUS_FRONTEND_7364_DISABLE_TERRESTRIAL
    BODS_Handle ods;
    BTFE_Handle tfe;
#endif
    BKNI_EventHandle isrEvent;
    uint8_t lastChannel;
    NEXUS_EventCallbackHandle isrEventCallback;
    unsigned agcValue;                            /* Gain Value*/
    NEXUS_CallbackDesc updateGainCallbackDesc;    /* Callback will be called when the gain from the lna needs to be updated. */
    NEXUS_TunerRfInput rfInput;
    NEXUS_RfDaisyChain rfDaisyChain;
    bool enableRfLoopThrough;
    bool acquireInProgress;
    signed count;
#if !NEXUS_FRONTEND_7364_DISABLE_TERRESTRIAL
    NEXUS_FrontendDvbt2Status t2PartialStatus;
    BODS_SelectiveStatus odsStatus;
    BTFE_SelectiveStatus vsbStatus;
    bool isStatusReady;
    BTNR_Handle                tnr[NEXUS_MAX_7364_TUNERS];
    BTFE_ChannelHandle         tfe_chn[NEXUS_MAX_7364_TFE_CHANNELS]; /* HARDCODED FOR NOW */
    NEXUS_FrontendVsbSettings  last_tfe[NEXUS_MAX_7364_TFE_CHANNELS];
    BODS_ChannelHandle         ods_chn[NEXUS_7364_MAX_OFDM_CHN];
    NEXUS_FrontendOfdmSettings last_ofdm[NEXUS_MAX_7364_TUNERS];
#endif
    NEXUS_IsrCallbackHandle    lockAppCallback[NEXUS_MAX_7364_T_FRONTENDS];
    NEXUS_IsrCallbackHandle    asyncStatusAppCallback[NEXUS_MAX_7364_T_FRONTENDS];
    NEXUS_IsrCallbackHandle    updateGainAppCallback[NEXUS_MAX_7364_TUNERS];
    bool                       isAsyncStatusReady[NEXUS_MAX_7364_T_FRONTENDS];
    bool                       isTunerPoweredOn;
    bool                       isPoweredOn[NEXUS_MAX_7364_T_FRONTENDS];
    NEXUS_FrontendOfdmAcquisitionMode lastAcquisitionMode[NEXUS_MAX_7364_T_FRONTENDS];
    NEXUS_FrontendHandle       frontendHandle;
    BINT_CallbackHandle cbHandle;

    /* Cable-specific variables */
    struct {
        unsigned dummy;
    } cable;

    /* Terrestrial-specific variables */
    struct {
        unsigned dummy;
    } terrestrial;

    /* Satellite-specific variables */
    struct {
        NEXUS_SatDeviceHandle satDevice;
        BKNI_EventHandle isrEvent;
        NEXUS_EventCallbackHandle isrCallback;
        BSAT_ChannelHandle satChannels[NEXUS_SAT_MAX_CHANNELS];
        NEXUS_FrontendHandle handles[NEXUS_SAT_MAX_CHANNELS];
        uint8_t A8299_control;
    } satellite;
} NEXUS_7364Device;

NEXUS_Error NEXUS_FrontendDevice_P_Get7364Settings(void *handle, NEXUS_FrontendDeviceSettings *pSettings);
NEXUS_Error NEXUS_FrontendDevice_P_Set7364Settings(void *handle, const NEXUS_FrontendDeviceSettings *pSettings);

/* Cable-specific functions */
NEXUS_Error NEXUS_FrontendDevice_Open7364_Cable(NEXUS_7364Device *pFrontendDevice);
NEXUS_FrontendHandle NEXUS_Frontend_Open7364_Cable(const NEXUS_FrontendChannelSettings *pSettings);
void NEXUS_FrontendDevice_Close7364_Cable(NEXUS_7364Device *pFrontendDevice);
NEXUS_Error NEXUS_FrontendDevice_P_Init7364_Cable(NEXUS_7364Device *pDevice);
void NEXUS_FrontendDevice_P_Uninit7364_Cable(NEXUS_7364Device *pDevice);
/* End of cable-specific function declarations */

/* Terrestrial-specific functions */
NEXUS_Error NEXUS_FrontendDevice_Open7364_Terrestrial(NEXUS_7364Device *pFrontendDevice);
NEXUS_FrontendHandle NEXUS_Frontend_Open7364_Terrestrial(const NEXUS_FrontendChannelSettings *pSettings);
void NEXUS_FrontendDevice_Close7364_Terrestrial(NEXUS_7364Device *pFrontendDevice);
NEXUS_Error NEXUS_FrontendDevice_P_Init7364_Terrestrial(NEXUS_7364Device *pDevice);
void NEXUS_FrontendDevice_P_Uninit7364_Terrestrial(NEXUS_7364Device *pDevice);
/* End of terrestrial-specific function declarations */

/* Satellite-specific functions */
NEXUS_Error NEXUS_FrontendDevice_Open7364_Satellite(NEXUS_7364Device *pFrontendDevice);
NEXUS_FrontendHandle NEXUS_Frontend_Open7364_Satellite(const NEXUS_FrontendChannelSettings *pSettings);
void NEXUS_FrontendDevice_Close7364_Satellite(NEXUS_7364Device *pFrontendDevice);
NEXUS_Error NEXUS_FrontendDevice_P_Init7364_Satellite(NEXUS_7364Device *pDevice);
void NEXUS_FrontendDevice_P_Uninit7364_Satellite(NEXUS_7364Device *pDevice);
/* End of satellite-specific function declarations */
#endif

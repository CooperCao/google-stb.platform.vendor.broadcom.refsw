/***************************************************************************
*     (c)2004-2015 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*   Management of timebase (clock rate) blocks.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#include "nexus_transport_module.h"

#define NEXUS_TIMEBASE_PCR_RATE 27000000 /* may be chip-dependent */
#define NEXUS_TIMEBASE_CLOCK_TOGGLE_FACTOR 2 /* may be chip-dependent */
#define NEXUS_TIMEBASE_HDDVI_CLOCK 0xcdfe600 /* may be chip-depenedent */
#define NEXUS_TIMEBASE_FREQ_FACTOR 1000

BDBG_MODULE(nexus_timebase);

static NEXUS_Error NEXUS_Timebase_P_SetTwoPcrErrorMonitor(NEXUS_TimebaseHandle timebase, bool forceOff);

static NEXUS_TimebaseHandle NEXUS_Timebase_P_ResolveAcquire(NEXUS_Timebase timebase, bool acquire)
{
    NEXUS_TimebaseHandle out = NULL;

    NEXUS_ASSERT_MODULE();

    if (timebase != NEXUS_Timebase_eInvalid)
    {
        if ((unsigned)timebase < (unsigned)NEXUS_Timebase_eMax)
        {
            unsigned index = timebase - NEXUS_Timebase_e0; /* assumes continuous enums */

            if (index < BXPT_NUM_PCRS)
            {
                /* enum variant */
                if (acquire)
                {
#if BDBG_DEBUG_BUILD
                    if (!pTransport->timebase[index]->acquired)
                    {
                        BDBG_MSG(("Allocating timebase %u to unprotected client", index));
                    }
#endif
                    pTransport->timebase[index]->acquired = true;
                }
                out = pTransport->timebase[index];
            }
            else
            {
                BERR_TRACE(NEXUS_INVALID_PARAMETER);
            }
        }
        else if ((unsigned)timebase > (unsigned)NEXUS_Timebase_eMax)
        {
            /* pointer variant */
            out = (NEXUS_TimebaseHandle)timebase;
            BDBG_OBJECT_ASSERT(out, NEXUS_Timebase);
        }
        else /*if (timebase == NEXUS_Timebase_eMax)*/
        {
            BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
    }
    else
    {
        /* eInvalid means use the default timebase, which is e0, but don't acquire it */
        out = pTransport->timebase[0];
    }

    return out;
}

NEXUS_TimebaseHandle NEXUS_Timebase_Resolve_priv(NEXUS_Timebase timebase)
{
    return NEXUS_Timebase_P_ResolveAcquire(timebase, true);
}


void NEXUS_Timebase_GetDefaultSettings(NEXUS_TimebaseSettings *pSettings)
{
    BXPT_PCR_TimebaseFreqRefConfig freqRefCfg;
    BERR_Code rc;

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->sourceType = NEXUS_TimebaseSourceType_eFreeRun;
    pSettings->sourceSettings.pcr.maxPcrError = 0xFF;
    pSettings->sourceSettings.pcr.trackRange = NEXUS_TimebaseTrackRange_e61ppm;
    pSettings->sourceSettings.pcr.jitterCorrection = NEXUS_TristateEnable_eNotSet; /* this is "auto". the current HW implementation of DPCR jitter correction is a
        global setting. so, if we default to auto, then any one timebase that sets it will enable it system-wide. */
    pSettings->sourceSettings.freeRun.centerFrequency = 0x400000;
    pSettings->sourceSettings.freeRun.trackRange = NEXUS_TimebaseTrackRange_e61ppm;
    pSettings->sourceSettings.vdec.trackRange = NEXUS_TimebaseTrackRange_e61ppm;
    pSettings->sourceSettings.hdDvi.trackRange = NEXUS_TimebaseTrackRange_e61ppm;
    pSettings->sourceSettings.ccir656.trackRange = NEXUS_TimebaseTrackRange_e61ppm;
    pSettings->sourceSettings.i2s.trackRange = NEXUS_TimebaseTrackRange_e61ppm;

    /* at init time, consult HW settings so that, by default, there is no change */
    /* hard code to use timebase zero, because we don't really need an actual timebase for this call */
    rc = BXPT_PCR_GetTimeBaseFreqRefDefaults(pTransport->timebase[0]->pcr, BXPT_PCR_TimeRef_eInternal, &freqRefCfg);
    if (rc) {
        rc=BERR_TRACE(rc);
    }
    else {
        /* make SW state == HW state */
        pSettings->sourceSettings.freeRun.prescale = freqRefCfg.Prescale;
        pSettings->sourceSettings.freeRun.inc = freqRefCfg.Inc;
        pSettings->sourceSettings.freeRun.loopDirectPathGain = freqRefCfg.FiltA;
        pSettings->sourceSettings.freeRun.loopGain = freqRefCfg.FiltB;
        pSettings->sourceSettings.freeRun.loopIntegratorLeak = freqRefCfg.FiltC;
    }
}

#if NEXUS_NUM_HDMI_INPUTS || NEXUS_NUM_HDDVI_INPUTS
/* lifted from Wikipedia article on Binary GCD algo */
static uint64_t NEXUS_Timebase_P_Gcd(uint64_t u, uint64_t v)
{
    int shift;

    /* GCD(0,v) == v; GCD(u,0) == u, GCD(0,0) == 0 */
    if (u == 0) return v;
    if (v == 0) return u;

    /* Let shift := lg K, where K is the greatest power of 2
     * dividing both u and v.
     */
    for (shift = 0 ; ((u | v) & 1) == 0 ; ++shift) {
        u >>= 1;
        v >>= 1;
    }

    while ((u & 1) == 0)
        u >>= 1;

    /* From here on, u is always odd. */
    do {
        /* remove all factors of 2 in v -- they are not common */
        /* note: v is not zero, so while will terminate */
        while ((v & 1) == 0)    /* Loop X */
            v >>= 1;

        /* Now u and v are both odd. Swap if necessary so u <= v,
         * then set v = v - u (which is even). For bignums, the
         * swapping is just pointer movement, and the subtraction
         * can be done in-place.
         */
        if (u > v) {
            unsigned int t;
            t = v;
            v = u;
            u = t;
        }
        v = v - u; /* Here v >= u. */
    } while (v != 0);

    /* restore common factors of 2 */
    return u << shift;
}

#define NEXUS_TIMEBASE_P_DEFAULT_REFRESH_RATE_NUM 60000
#define NEXUS_TIMEBASE_P_DEFAULT_REFRESH_RATE_DEN 1001

static void NEXUS_Timebase_P_ComputeVsyncTrackingCoefficients(NEXUS_VideoFormat format, NEXUS_VideoFrameRate frameRate, unsigned refreshRateNum, unsigned refreshRateDen, unsigned *inc, unsigned *prescale)
{
    uint64_t num;
    uint64_t den;
    uint64_t gcd;

    if (!refreshRateNum || !refreshRateDen)
    {
        BDBG_WRN(("Unsupported refresh rate of %d/%d; using %d/%d", refreshRateNum, refreshRateDen, NEXUS_TIMEBASE_P_DEFAULT_REFRESH_RATE_NUM, NEXUS_TIMEBASE_P_DEFAULT_REFRESH_RATE_DEN));
        refreshRateNum = NEXUS_TIMEBASE_P_DEFAULT_REFRESH_RATE_NUM;
        refreshRateDen = NEXUS_TIMEBASE_P_DEFAULT_REFRESH_RATE_DEN;
    }

    num = (uint64_t)NEXUS_TIMEBASE_PCR_RATE
        * (uint64_t)NEXUS_TIMEBASE_CLOCK_TOGGLE_FACTOR
        * (uint64_t)refreshRateDen;
    den = refreshRateNum;
    gcd = NEXUS_Timebase_P_Gcd(num, den);

    *inc = num / gcd;
    *prescale = (den / gcd) - 1;

    BDBG_MSG(("VSYNC; format = %u; frameRate = %u; refreshNum = %u; refreshDen = %u; num = %llu; den = %llu; gcd = %llu; inc = %u; prescale = %u",
        format,
        frameRate,
        refreshRateNum,
        refreshRateDen,
        num,
        den,
        gcd,
        *inc,
        *prescale));
}

#define NEXUS_TIMEBASE_P_DEFAULT_SCAN_HEIGHT 1125

static void NEXUS_Timebase_P_ComputeHsyncTrackingCoefficients(NEXUS_VideoFormat format, NEXUS_VideoFrameRate frameRate, bool interlaced, unsigned scanHeight, unsigned refreshRateNum, unsigned refreshRateDen, unsigned *inc, unsigned *prescale)
{
    uint64_t num;
    uint64_t den;
    uint64_t gcd;

    if (!refreshRateNum || !refreshRateDen)
    {
        BDBG_WRN(("Unsupported refresh rate of %d/%d; using %d/%d", refreshRateNum, refreshRateDen, NEXUS_TIMEBASE_P_DEFAULT_REFRESH_RATE_NUM, NEXUS_TIMEBASE_P_DEFAULT_REFRESH_RATE_DEN));
        refreshRateNum = NEXUS_TIMEBASE_P_DEFAULT_REFRESH_RATE_NUM;
        refreshRateDen = NEXUS_TIMEBASE_P_DEFAULT_REFRESH_RATE_DEN;
    }

    if (!scanHeight)
    {
        BDBG_WRN(("Unsupported scan height of 0; using %d", NEXUS_TIMEBASE_P_DEFAULT_SCAN_HEIGHT));
        scanHeight = NEXUS_TIMEBASE_P_DEFAULT_SCAN_HEIGHT;
    }

    num = (uint64_t)NEXUS_TIMEBASE_PCR_RATE
        * (uint64_t)NEXUS_TIMEBASE_CLOCK_TOGGLE_FACTOR
        * (uint64_t)refreshRateDen
        * (uint64_t)(interlaced ? 2 : 1);
    den = (uint64_t)scanHeight
        * (uint64_t)refreshRateNum;
    gcd = NEXUS_Timebase_P_Gcd(num, den);

    *inc = num / gcd;
    *prescale = (den / gcd) - 1;

    BDBG_MSG(("HSYNC; format = %u; frameRate = %u; refreshNum = %u; refreshDen = %u; scanHeight = %u; num = %llu; den = %llu; gcd = %llu; inc = %u; prescale = %u",
        format,
        frameRate,
        refreshRateNum,
        refreshRateDen,
        scanHeight,
        num,
        den,
        gcd,
        *inc,
        *prescale));
}

static void NEXUS_Timebase_P_RefreshRateFromFrameRate(bool interlaced, NEXUS_VideoFrameRate frameRate, unsigned * pRefreshRateNum, unsigned * pRefreshRateDen)
{
    unsigned refreshRate = NEXUS_P_RefreshRate_FromFrameRate_isrsafe(frameRate);
    if (refreshRate == 0) {
        BDBG_ERR(("Unsupported frameRateCode of %d; using %d/%d", frameRate, NEXUS_TIMEBASE_P_DEFAULT_REFRESH_RATE_NUM, NEXUS_TIMEBASE_P_DEFAULT_REFRESH_RATE_DEN));
        *pRefreshRateNum = NEXUS_TIMEBASE_P_DEFAULT_REFRESH_RATE_NUM;
        *pRefreshRateDen = NEXUS_TIMEBASE_P_DEFAULT_REFRESH_RATE_DEN;
    }
    else if (refreshRate % 100) {
        *pRefreshRateDen = 1001;
        *pRefreshRateNum = (refreshRate/100 + 1) * 100;
    }
    else {
        *pRefreshRateDen = 1000;
        *pRefreshRateNum = refreshRate;
    }
    if (interlaced)
    {
        *pRefreshRateNum <<= 1;
    }
}

static void NEXUS_Timebase_P_ComputeRefreshRate(NEXUS_VideoFormat videoFormat, const NEXUS_VideoFormatInfo * pInfo, NEXUS_VideoFrameRate frameRate, unsigned * pRefreshRateNum, unsigned * pRefreshRateDen)
{
    switch(videoFormat)
    {
        /*
         * these formats don't have a single refresh rate, so use the passed
         * frame rate
         */
        case NEXUS_VideoFormat_eNtsc:
        case NEXUS_VideoFormat_eNtsc443:
        case NEXUS_VideoFormat_eNtscJapan:
        case NEXUS_VideoFormat_ePalM:
        case NEXUS_VideoFormat_ePal60hz:
        case NEXUS_VideoFormat_e480p:
        case NEXUS_VideoFormat_e576p:
        case NEXUS_VideoFormat_e720p:
        case NEXUS_VideoFormat_e720p24hz:
        case NEXUS_VideoFormat_e720p30hz:
        case NEXUS_VideoFormat_e720p_3DOU_AS:
        case NEXUS_VideoFormat_eCustom_3D_720p:
        case NEXUS_VideoFormat_e1080i:
        case NEXUS_VideoFormat_e1080p:
        case NEXUS_VideoFormat_e1080p24hz:
        case NEXUS_VideoFormat_e1080p24hz_3DOU_AS:
        case NEXUS_VideoFormat_e1440x480p60hz:
        case NEXUS_VideoFormat_e3840x2160p24hz:
        case NEXUS_VideoFormat_e3840x2160p30hz:
        case NEXUS_VideoFormat_e3840x2160p60hz:
        case NEXUS_VideoFormat_e4096x2160p60hz:
        case NEXUS_VideoFormat_eCustomer1440x240p60hz:
        case NEXUS_VideoFormat_eCustomer1366x768p60hz:
        case NEXUS_VideoFormat_eVesa640x480p60hz:
        case NEXUS_VideoFormat_eUnknown:
            NEXUS_Timebase_P_RefreshRateFromFrameRate(pInfo->interlaced, frameRate, pRefreshRateNum, pRefreshRateDen);
            break;
        default:
            /*
             * all other formats may have frame rates that we don't support
             * in NEXUS_VideoFrameRate, so we have to get it from fmtInfo
             */
            *pRefreshRateNum = pInfo->verticalFreq == 2397 ? 24000 : pInfo->verticalFreq * 10;
            *pRefreshRateDen = pInfo->verticalFreq == 2397 ? 1001 : 1000;
            break;
    }
}

static void NEXUS_Timebase_P_ComputeVideoTrackingCoefficients(NEXUS_VideoFormat videoFormat, NEXUS_VideoFrameRate frameRate, unsigned *inc, unsigned *prescale)
{
    NEXUS_VideoFormatInfo videoFmtInfo;
    unsigned refreshRateNum;
    unsigned refreshRateDen;

    *inc = 1;
    *prescale = 0;

    NEXUS_VideoFormat_GetInfo(videoFormat, &videoFmtInfo);

    NEXUS_Timebase_P_ComputeRefreshRate(videoFormat, &videoFmtInfo, frameRate, &refreshRateNum, &refreshRateDen);

    if (NEXUS_VideoFormat_eUnknown != videoFormat)
    {
        NEXUS_Timebase_P_ComputeHsyncTrackingCoefficients(videoFormat, frameRate, videoFmtInfo.interlaced, videoFmtInfo.scanHeight, refreshRateNum, refreshRateDen, inc, prescale);
    }
    else
    {
        NEXUS_Timebase_P_ComputeVsyncTrackingCoefficients(videoFormat, frameRate, refreshRateNum, refreshRateDen, inc, prescale);
    }
}

static void NEXUS_Timebase_P_ComputeVideoTimeRef(const NEXUS_TimebaseSettings * pSettings, BXPT_PCR_TimeRef * pTimeRef)
{
    /* use HSYNC tracking, unless the format is unknown */
    bool useVsync = NEXUS_VideoFormat_eUnknown == pSettings->sourceSettings.hdDvi.format;

    switch (pSettings->sourceSettings.hdDvi.index)
    {
        case 0:
            if (useVsync)
            {
                *pTimeRef = BXPT_PCR_TimeRef_eHD_DVI_V0;
            }
            else
            {
                *pTimeRef = BXPT_PCR_TimeRef_eHD_DVI_H0;
            }
            break;

        case 1:
            if (useVsync)
            {
                *pTimeRef = BXPT_PCR_TimeRef_eHD_DVI_V1;
            }
            else
            {
                *pTimeRef = BXPT_PCR_TimeRef_eHD_DVI_H1;
            }
            break;

        default:
            BDBG_WRN(("Unsupported video time ref index; using 0"));
            if (useVsync)
            {
                *pTimeRef = BXPT_PCR_TimeRef_eHD_DVI_V0;
            }
            else
            {
                *pTimeRef = BXPT_PCR_TimeRef_eHD_DVI_H0;
            }
            break;
    }
}
#endif

#if 0
NEXUS_Error NEXUS_Timebase_P_GetIncAndCenterFreq(NEXUS_VideoFormat videoFormat, unsigned vertSyncClock, unsigned *inc, unsigned *centerFreq)
{
    NEXUS_VideoFormatInfo videoFmtInfo;
    uint32_t refreshRate = 0;
    uint32_t hddviFactor = 0;
    uint32_t pcrFactor = 0;

    if (0 == vertSyncClock || NEXUS_VideoFormat_eUnknown == videoFormat) return BERR_TRACE(NEXUS_INVALID_PARAMETER);

    NEXUS_VideoFormat_GetInfo(videoFormat, &videoFmtInfo);

    if ((2300 < videoFmtInfo.verticalFreq) && (2450 >= videoFmtInfo.verticalFreq)) {
        refreshRate = 24;
    } else if ((2450 < videoFmtInfo.verticalFreq) && (2600 >= videoFmtInfo.verticalFreq)) {
        refreshRate = 25;
    } else if ((2900 < videoFmtInfo.verticalFreq) && (3100 >= videoFmtInfo.verticalFreq)) {
        refreshRate = 30 ;
    } else if ((4900 < videoFmtInfo.verticalFreq) && (5100 >= videoFmtInfo.verticalFreq)) {
        refreshRate = 50;
    } else if ((5900 < videoFmtInfo.verticalFreq) && (6100 >= videoFmtInfo.verticalFreq)) {
        refreshRate = 60;
    } else {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    hddviFactor = NEXUS_TIMEBASE_HDDVI_CLOCK / refreshRate;
    pcrFactor = NEXUS_TIMEBASE_PCR_RATE / refreshRate;

    *inc = (uint32_t)((uint64_t)pcrFactor * (uint64_t)vertSyncClock / (uint64_t)hddviFactor);
    *centerFreq = (uint32_t)((uint64_t)0x400000 * (uint64_t)hddviFactor / (uint64_t)vertSyncClock);

    BDBG_MSG(("NEXUS_Timebase_P_GetIncAndCenterFreq: refreshRate=%u, vertSyncClock=%u, inc=%u, centerFreq=%u",
        refreshRate,
        vertSyncClock,
        *inc,
        *centerFreq
        ));


    return NEXUS_SUCCESS;
}
#endif

/* do acquire/release on any resource stored in handle->settings */
static void NEXUS_Timebase_P_DoSettingsAccounting(NEXUS_TimebaseHandle handle, const NEXUS_TimebaseSettings *pSettings)
{
    NEXUS_PidChannelHandle acq;
    NEXUS_PidChannelHandle rel;

    if (pSettings && pSettings->sourceType == NEXUS_TimebaseSourceType_ePcr) {
        acq = pSettings->sourceSettings.pcr.pidChannel;
    }
    else {
        acq = NULL;
    }
    if (handle->settings.sourceType == NEXUS_TimebaseSourceType_ePcr) {
        rel = handle->settings.sourceSettings.pcr.pidChannel;
    }
    else {
        rel = NULL;
    }
    if (acq != rel) {
        if (rel) {
            NEXUS_OBJECT_RELEASE(handle, NEXUS_PidChannel, rel);
        }
        if (acq) {
            NEXUS_OBJECT_ACQUIRE(handle, NEXUS_PidChannel, acq);
        }
    }
}

NEXUS_Error NEXUS_Timebase_P_SetSettings(NEXUS_TimebaseHandle timebase, const NEXUS_TimebaseSettings *pSettings)
{
    BERR_Code rc;
    BXPT_PCR_Handle pcr = NULL;
    BXPT_PCR_TimebaseFreqRefConfig freqRefConfig;
    BXPT_PCR_TimeRef timeref;
    NEXUS_TimebaseSourceType sourceType;
#if NEXUS_HAS_ASTM
    NEXUS_TimebaseAstmSettings * pAstmSettings;
#endif
    bool force;

    BDBG_OBJECT_ASSERT(timebase, NEXUS_Timebase);

    pcr = timebase->pcr;
    /* read sourceType out of the structure because Astm might change it. do not use pSettings->sourceType later in this function. */
    sourceType = pSettings->sourceType;

#if NEXUS_HAS_ASTM
    /* if we aren't attempting to set the source type to FreeRun from the user config, permit ASTM.
    Having the user set the sourceType to eFreeRun is equivalent to being in playback mode, and we don't support that in ASTM
    so disable ASTM */
    pAstmSettings = &timebase->astm.settings;

    if (pAstmSettings->enabled)
    {
        BDBG_MSG(("ASTM is setting the clock coupling for timebase %u to %s", timebase->hwIndex, pAstmSettings->clockCoupling == NEXUS_TimebaseClockCoupling_eInternalClock ? "internal" : "input"));
        if (pAstmSettings->clockCoupling == NEXUS_TimebaseClockCoupling_eInternalClock)
        {
            sourceType = NEXUS_TimebaseSourceType_eFreeRun;
        }
    }
#endif

    timebase->status.sourceType = sourceType;

    force = (sourceType != timebase->settings.sourceType);

    if ( sourceType == NEXUS_TimebaseSourceType_ePcr) {
        NEXUS_IsrCallback_Set(timebase->monitorCallback, &pSettings->pcrCallback);
    }
    else {
        NEXUS_IsrCallback_Set(timebase->monitorCallback, NULL);
    }

    timebase->isDss = false;
    switch (sourceType) {
    case NEXUS_TimebaseSourceType_ePcr:
        {
        BXPT_PCR_XptStreamPcrCfg pcrConfig;
#if BXPT_DPCR_GLOBAL_PACKET_PROC_CTRL
        BXPT_PCR_JitterCorrection jitterCorrection;
#endif
        NEXUS_P_HwPidChannel *pcrPidChannel;

        if (pSettings->sourceSettings.pcr.pidChannel==NULL) {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        NEXUS_OBJECT_ASSERT(NEXUS_PidChannel, pSettings->sourceSettings.pcr.pidChannel);
        pcrPidChannel = pSettings->sourceSettings.pcr.pidChannel->hwPidChannel;

        rc = BXPT_PCR_GetStreamPcrConfig(pcr, &pcrConfig);
        if (rc) {return BERR_TRACE(rc);}

#if BXPT_DPCR_GLOBAL_PACKET_PROC_CTRL
        /* transform nexus tristate to BXPT_PCR_JitterCorrection tristate */
        switch (pSettings->sourceSettings.pcr.jitterCorrection) {
        case NEXUS_TristateEnable_eDisable: jitterCorrection = BXPT_PCR_JitterCorrection_eDisable; break;
        case NEXUS_TristateEnable_eEnable:  jitterCorrection = BXPT_PCR_JitterCorrection_eEnable; break;
        case NEXUS_TristateEnable_eNotSet:  jitterCorrection = BXPT_PCR_JitterCorrection_eAuto; break;
        default: return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }

        if (jitterCorrection == BXPT_PCR_JitterCorrection_eAuto) {
            pcrConfig.JitterTimestamp = BXPT_PCR_JitterTimestampMode_eAuto;
        }
        else if ( pcrPidChannel->status.playback ) {
            /* for playback, nexus does not expose e28_4P or e32, so we can hardcode to e30_2U */
            pcrConfig.JitterTimestamp = BXPT_PCR_JitterTimestampMode_e30_2U;
        }
        else {
            /* for live, there is no HW capability for e30_2U or e28_4P, so we can hardcode to e32 */
            pcrConfig.JitterTimestamp = BXPT_PCR_JitterTimestampMode_e32;
        }
#endif

        if ( pcrPidChannel->status.playback )
        {
#if !BXPT_DPCR_GLOBAL_PACKET_PROC_CTRL
            pcrConfig.eStreamSelect = BXPT_DataSource_ePlayback;
            pcrConfig.WhichSource = pcrPidChannel->status.playbackIndex;
#endif
#if BXPT_DPCR_GLOBAL_PACKET_PROC_CTRL
            pcrConfig.PbJitterDisable = jitterCorrection;
#endif
        }
        else
        {
            NEXUS_ParserBandSettings parserSettings;

            if (pcrPidChannel->parserBand)
            {

                NEXUS_ParserBand_P_GetSettings(pcrPidChannel->parserBand, &parserSettings);
                if ( parserSettings.sourceType == NEXUS_ParserBandSourceType_eInputBand ||
                     parserSettings.sourceType == NEXUS_ParserBandSourceType_eMtsif ||
                     parserSettings.sourceType == NEXUS_ParserBandSourceType_eTsmf )
                {
    #if !BXPT_DPCR_GLOBAL_PACKET_PROC_CTRL
                    pcrConfig.eStreamSelect = BXPT_DataSource_eInputBand;
                    pcrConfig.WhichSource = parserSettings.sourceTypeSettings.inputBand - NEXUS_InputBand_e0;
    #endif
                }
                else if ( parserSettings.sourceType == NEXUS_ParserBandSourceType_eRemux )
                {
                    BDBG_ERR(("You cannot use a PCR pid channel from a remux looped back to a parser band. Instead, use a pid channel from the original input band."));
                    return BERR_TRACE(BERR_INVALID_PARAMETER);
                }
                else
                {
                    BDBG_ERR(("Invalid parser band source (%d) for timebase input.", parserSettings.sourceType ));
                    return BERR_TRACE(BERR_INVALID_PARAMETER);
                }
            }
            else
            {
                /*
                 * zombie pid channel
                 * This can happen during shutdown if the parser band was closed
                 * but something (like simple stc channel) is still holding on
                 * to a pid channel reference on that band.
                 */
#if BXPT_DPCR_GLOBAL_PACKET_PROC_CTRL
                BDBG_MSG(("Timebase holding zombie pcr pid channel: %p", (void *)pcrPidChannel));
#else
                BDBG_ERR(("Unable to configure timebase input band source from zombie pid channel"));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
#endif
            }
#if BXPT_DPCR_GLOBAL_PACKET_PROC_CTRL
            pcrConfig.LiveJitterDisable = jitterCorrection;
#endif
        }

        pcrConfig.MaxPcrError = pSettings->sourceSettings.pcr.maxPcrError;
#if BXPT_DPCR_GLOBAL_PACKET_PROC_CTRL
        /* 7420 PCR hardware uses the PID channel */
        pcrConfig.PidChannel = pcrPidChannel->status.pidChannelIndex;
#else
        pcrConfig.Pid = pcrPidChannel->status.pid;
#endif
        rc = BXPT_PCR_GetTimeBaseFreqRefDefaults(pcr, BXPT_PCR_TimeRef_eXpt, &pcrConfig.TimebaseCfg);
        if (rc) {return BERR_TRACE(rc);}

        rc = BXPT_PCR_SetStreamPcrConfig(pcr, &pcrConfig);
        if (rc) {return BERR_TRACE(rc);}

        timebase->isDss = NEXUS_IS_DSS_MODE(pcrPidChannel->status.transportType);

        /* Set the crystal tracking range */
        if (force || pSettings->sourceSettings.pcr.trackRange != timebase->settings.sourceSettings.pcr.trackRange) {
            BDBG_CASSERT(NEXUS_TimebaseTrackRange_e244ppm == (NEXUS_TimebaseTrackRange)BXPT_PCR_TrackRange_PPM_244);
            BXPT_PCR_SetTimeRefTrackRange(pcr, pSettings->sourceSettings.pcr.trackRange);
        }

        BDBG_MSG(("%s: PCR Mode Timebase (%u) Settings: track range %d, max pcr error %d",
                    __FUNCTION__, timebase->hwIndex, pSettings->sourceSettings.pcr.trackRange, pSettings->sourceSettings.pcr.maxPcrError));
        }
        break;

    case NEXUS_TimebaseSourceType_eFreeRun:
        rc = BXPT_PCR_GetTimeBaseFreqRefDefaults(pcr, BXPT_PCR_TimeRef_eInternal, &freqRefConfig);
        if (rc) {return BERR_TRACE(rc);}

        freqRefConfig.FiltA = pSettings->sourceSettings.freeRun.loopDirectPathGain;
        freqRefConfig.FiltB = pSettings->sourceSettings.freeRun.loopGain;
        freqRefConfig.FiltC = pSettings->sourceSettings.freeRun.loopIntegratorLeak;

        freqRefConfig.Prescale = pSettings->sourceSettings.freeRun.prescale;
        freqRefConfig.Inc = pSettings->sourceSettings.freeRun.inc;
        rc = BXPT_PCR_ConfigNonStreamTimeBase(pcr, BXPT_PCR_TimeRef_eInternal, &freqRefConfig);
        if (rc) {return BERR_TRACE(rc);}

        if (force || pSettings->sourceSettings.freeRun.centerFrequency != timebase->settings.sourceSettings.freeRun.centerFrequency) {
            BXPT_PCR_SetCenterFrequency(pcr, pSettings->sourceSettings.freeRun.centerFrequency);
        }
        /* Set the crystal tracking range */
        if (force || pSettings->sourceSettings.freeRun.trackRange != timebase->settings.sourceSettings.freeRun.trackRange) {
            BXPT_PCR_SetTimeRefTrackRange(pcr, pSettings->sourceSettings.freeRun.trackRange);
        }
        break;

    case NEXUS_TimebaseSourceType_eAnalog:
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);

    case NEXUS_TimebaseSourceType_eCcir656In:
        timeref = BXPT_PCR_TimeRef_eI656_Vl;
        rc = BXPT_PCR_GetTimeBaseFreqRefDefaults(pcr, timeref, &freqRefConfig);
        if (rc) {return BERR_TRACE(rc);}

        /* select clock coefficient, based on the source format. This is SD only, so we don't need framerate.
        TODO: likely these values are different for 3563/3548. */
        switch (pSettings->sourceSettings.ccir656.format) {
        case NEXUS_VideoFormat_eNtsc:
        case NEXUS_VideoFormat_eNtscJapan:
            freqRefConfig.Inc = 900900; break;
        default: /* PAL */
            freqRefConfig.Inc = 1080000; break;
        }

        freqRefConfig.Prescale = 0;
        rc = BXPT_PCR_ConfigNonStreamTimeBase(pcr, timeref, &freqRefConfig);
        if (rc) {return BERR_TRACE(rc);}

        /* Set the crystal tracking range */
        if (force || pSettings->sourceSettings.ccir656.trackRange != timebase->settings.sourceSettings.ccir656.trackRange) {
            BXPT_PCR_SetTimeRefTrackRange(pcr, pSettings->sourceSettings.ccir656.trackRange);
        }
        break;

#if NEXUS_NUM_I2S_INPUTS > 0
    case NEXUS_TimebaseSourceType_eI2sIn:
        switch (pSettings->sourceSettings.i2s.index) {
        case 0: timeref = BXPT_PCR_TimeRef_eI2S0; break;
#if NEXUS_NUM_I2S_INPUTS > 1
        case 1: timeref = BXPT_PCR_TimeRef_eI2S1; break;
#endif
        default: return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        rc = BXPT_PCR_GetTimeBaseFreqRefDefaults(pcr, timeref, &freqRefConfig);
        if (rc) {return BERR_TRACE(rc);}

        switch (pSettings->sourceSettings.i2s.sampleRate) {
        case 48000:
            freqRefConfig.Prescale = 1;
            freqRefConfig.Inc = 1125;
            break;
        case 44100:
            freqRefConfig.Prescale = 48;
            freqRefConfig.Inc = 30000;
            break;
        case 32000:
            freqRefConfig.Prescale = 3;
            freqRefConfig.Inc = 3375;
            break;
        default:
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }

        rc = BXPT_PCR_ConfigNonStreamTimeBase(pcr, timeref, &freqRefConfig);
        if (rc) {return BERR_TRACE(rc);}

        /* Set the crystal tracking range */
        if (force || pSettings->sourceSettings.i2s.trackRange != timebase->settings.sourceSettings.i2s.trackRange) {
            BXPT_PCR_SetTimeRefTrackRange(pcr, pSettings->sourceSettings.i2s.trackRange);
        }
        break;
#endif

#if NEXUS_NUM_SPDIF_INPUTS > 0
    case NEXUS_TimebaseSourceType_eSpdifIn:
        timeref = BXPT_PCR_TimeRef_eSPDIF;

        rc = BXPT_PCR_GetTimeBaseFreqRefDefaults(pcr, timeref, &freqRefConfig);
        if (rc) {return BERR_TRACE(rc);}

        rc = BXPT_PCR_ConfigNonStreamTimeBase(pcr, timeref, &freqRefConfig);
        if (rc) {return BERR_TRACE(rc);}
        break;
#endif

#if NEXUS_NUM_HDMI_INPUTS || NEXUS_NUM_HDDVI_INPUTS
    case NEXUS_TimebaseSourceType_eHdDviIn:
        NEXUS_Timebase_P_ComputeVideoTimeRef(pSettings, &timeref);

        rc = BXPT_PCR_GetTimeBaseFreqRefDefaults(pcr, timeref, &freqRefConfig);
        if (rc) { return BERR_TRACE(rc); }

        NEXUS_Timebase_P_ComputeVideoTrackingCoefficients(
            pSettings->sourceSettings.hdDvi.format,
            timebase->hdDviFrameRate ? timebase->hdDviFrameRate : pSettings->sourceSettings.hdDvi.frameRate,
            &freqRefConfig.Inc,
            &freqRefConfig.Prescale
            );

#if 0
        /*
         * We should not be altering the center frequency in order to lock.
         * Not sure how this code got here, but looks like DTV maybe?
         */
        if (pSettings->sourceSettings.hdDvi.vertSyncClock)
        {
            uint32_t inc = 0;
            uint32_t centerFreq = 0;

            rc = NEXUS_Timebase_P_GetIncAndCenterFreq(
                     pSettings->sourceSettings.hdDvi.format,
                     pSettings->sourceSettings.hdDvi.vertSyncClock,
                     &inc,
                     &centerFreq);

             if (NEXUS_SUCCESS == rc)
            {
                freqRefConfig.Inc = inc;
                BXPT_PCR_SetCenterFrequency(pcr, centerFreq);
            }

            BDBG_MSG(("NEXUS_Timebase_SetSettings: eHdDviIn - vertSyncClock=%u, inc=%u, centerFreq=%u",
                pSettings->sourceSettings.hdDvi.vertSyncClock,
                freqRefConfig.Inc,
                centerFreq));
        }
#endif

        rc = BXPT_PCR_ConfigNonStreamTimeBase(pcr, timeref, &freqRefConfig);
        if (rc) BERR_TRACE(rc);

        /* Set the crystal tracking range */
        if (force || pSettings->sourceSettings.hdDvi.trackRange != timebase->settings.sourceSettings.hdDvi.trackRange)
        {
            BXPT_PCR_SetTimeRefTrackRange(pcr, pSettings->sourceSettings.hdDvi.trackRange);
        }
        break;
#endif

    default:
        BDBG_ERR(("unsupported timebase source type"));
        return NEXUS_INVALID_PARAMETER;
    }

#if B_REFSW_DSS_SUPPORT
    rc = BXPT_PCR_DirecTv_SetPcrMode(pcr, timebase->isDss?BXPT_PcrMode_eDirecTv:BXPT_PcrMode_eMpeg);
    if (rc) {return BERR_TRACE(rc);}
#endif

    BXPT_PCR_FreezeIntegrator(pcr, pSettings->freeze);

    rc = NEXUS_Timebase_P_SetTwoPcrErrorMonitor(timebase, false);
    if (rc) return BERR_TRACE(rc);

    NEXUS_Timebase_P_DoSettingsAccounting(timebase, pSettings);
    timebase->settings = *pSettings;

    return 0;
}

NEXUS_Error NEXUS_Timebase_SetSettings(NEXUS_Timebase timebase, const NEXUS_TimebaseSettings *pSettings)
{
    NEXUS_TimebaseHandle handle;

    handle = NEXUS_Timebase_Resolve_priv(timebase);

    if (handle)
    {
        return NEXUS_Timebase_P_SetSettings(handle, pSettings);
    }
    else
    {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
}

void NEXUS_Timebase_P_GetSettings(NEXUS_TimebaseHandle timebase, NEXUS_TimebaseSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(timebase, NEXUS_Timebase);
    *pSettings = timebase->settings;
}

void NEXUS_Timebase_GetSettings(NEXUS_Timebase timebase, NEXUS_TimebaseSettings *pSettings)
{
    NEXUS_TimebaseHandle handle;

    handle = NEXUS_Timebase_P_ResolveAcquire(timebase, false);

    if (handle)
    {
        NEXUS_Timebase_P_GetSettings(handle, pSettings);
    }
    else
    {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
}

static int NEXUS_Timebase_P_NextAvailable(void)
{
    int next = -1;
    unsigned i = 0;

    for (i = 0; i < BXPT_NUM_PCRS; i++)
    {
        if (!pTransport->timebase[i]->acquired)
        {
            next = i;
            break;
        }
    }

    return next;
}

static NEXUS_TimebaseHandle NEXUS_Timebase_P_Open(unsigned index)
{
    NEXUS_TimebaseHandle handle = NULL;

    if (index == NEXUS_ANY_ID)
    {
        int next = NEXUS_Timebase_P_NextAvailable();
        if (next == -1)
        {
            BERR_TRACE(NEXUS_NOT_AVAILABLE);
            goto end;
        }
        else
        {
            index = (unsigned)next;
        }
    }

    if (index >= BXPT_NUM_PCRS)
    {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto end;
    }

    if (!pTransport->timebase[index]->acquired)
    {
        pTransport->timebase[index]->acquired = true;
        /* BDBG_MSG(("Allocating timebase %u to client %p", index, b_objdb_get_client())); */
        handle = pTransport->timebase[index];
        NEXUS_OBJECT_SET(NEXUS_Timebase, handle);
    }
    else
    {
        BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

end:
    return handle;
}

NEXUS_Timebase NEXUS_Timebase_Open(unsigned index)
{
    NEXUS_TimebaseHandle handle = NULL;
    handle = NEXUS_Timebase_P_Open(index);
    /* here we cast to the enum, but it is really a pointer */
    return handle?(NEXUS_Timebase)handle:(NEXUS_Timebase)NULL;
}

static void NEXUS_Timebase_P_Finalizer(NEXUS_TimebaseHandle timebase)
{
    NEXUS_TimebaseSettings settings;

    NEXUS_OBJECT_ASSERT(NEXUS_Timebase, timebase);

    /* revert to default known state */
    NEXUS_Timebase_GetDefaultSettings(&settings);
    NEXUS_Timebase_P_SetSettings(timebase, &settings);

    /* this may unregister enum variant usage, if that usage was done
    in an unprotected client *after* a protected client already acquired
    the resource */
    timebase->acquired = false;
    /* can't call NEXUS_OBJECT_UNSET since there is a still internal API that uses object */

    /* BDBG_MSG(("Client %p releasing timebase %u", b_objdb_get_client(), timebase->hwIndex)); */
    return;
}

static NEXUS_OBJECT_CLASS_MAKE(NEXUS_Timebase, NEXUS_Timebase_P_Close);

void NEXUS_Timebase_Close(NEXUS_Timebase timebase)
{
    NEXUS_TimebaseHandle handle = NULL;

    handle = NEXUS_Timebase_Resolve_priv(timebase);

    if (handle)
    {
        NEXUS_Timebase_P_Close(handle);
    }
    else
    {
        BDBG_ERR(("You may be attempting to close the enum variant of this resource.  Please ensure you are passing the resource returned when you called open."));
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    return;
}

static NEXUS_Error NEXUS_Timebase_P_GetPcr_isr(NEXUS_TimebaseHandle timebase, uint32_t *pHi, uint32_t *pLo, int32_t *pError);

static void NEXUS_Timebase_P_Monitor_isr(void *context, int param)
{
    BERR_Code rc;
    NEXUS_TimebaseHandle timebase = context;
    NEXUS_TimebaseStatus *pStatus = &timebase->status;
    uint32_t hi, lo;
    int32_t error;

    BSTD_UNUSED(param);

    BDBG_OBJECT_ASSERT(timebase, NEXUS_Timebase);

    rc = NEXUS_Timebase_P_GetPcr_isr(timebase, &hi, &lo, &error);
    if (rc) return;

    BDBG_MSG(("Timebase %u: received PCR %#x", timebase->hwIndex, hi));
    pStatus->pcrValid = true;
    pStatus->pcrCount++;
    NEXUS_IsrCallback_Fire_isr(timebase->monitorCallback);

#if NEXUS_HAS_ASTM
    if (timebase->astm.settings.enabled)
    {
        if (timebase->astm.settings.pcrReceived_isr)
        {
            timebase->astm.settings.pcrReceived_isr(timebase->astm.settings.callbackContext, timebase->hwIndex);
        }
    }
#endif
}

NEXUS_Error NEXUS_Timebase_P_StartMonitor(NEXUS_TimebaseHandle timebase)
{
    BERR_Code rc;
    BINT_Id pcr_int;

    BDBG_OBJECT_ASSERT(timebase, NEXUS_Timebase);

    BDBG_MSG(("start monitor %d", timebase->hwIndex));

    rc = BXPT_PCR_GetIntId( timebase->hwIndex, BXPT_PCR_IntName_ePhaseCompare, &pcr_int );
    if (rc) return BERR_TRACE(rc);

    rc = BINT_CreateCallback(&timebase->intMonitorCallback, g_pCoreHandles->bint, pcr_int, NEXUS_Timebase_P_Monitor_isr, timebase, 0);
    if (rc) return BERR_TRACE(rc);
    rc = BINT_EnableCallback(timebase->intMonitorCallback);
    if (rc) return BERR_TRACE(rc);
    return 0;
}

void NEXUS_Timebase_P_StopMonitor(NEXUS_TimebaseHandle timebase)
{
     BDBG_OBJECT_ASSERT(timebase, NEXUS_Timebase);

    if (timebase->intMonitorCallback) {
        BINT_DisableCallback(timebase->intMonitorCallback);
        BINT_DestroyCallback(timebase->intMonitorCallback);
        timebase->intMonitorCallback = NULL;
    }

    /* clean up in case it was left on */
    (void)NEXUS_Timebase_P_SetTwoPcrErrorMonitor(timebase, true);
}

static void NEXUS_Timebase_P_TwoPcrError_isr( void *context, int param )
{
    NEXUS_TimebaseHandle timebase = context;
    BDBG_OBJECT_ASSERT(timebase, NEXUS_Timebase);
    BSTD_UNUSED(param);
    timebase->status.pcrErrors++;
    BDBG_WRN(("Timebase %u: pcrError", timebase->hwIndex));
    NEXUS_IsrCallback_Fire_isr(timebase->pcrErrorCallback);
}

static NEXUS_Error NEXUS_Timebase_P_SetTwoPcrErrorMonitor(NEXUS_TimebaseHandle timebase, bool forceOff)
{
    BINT_Id dpcrErrorIntId;
    bool install = false;

    BDBG_OBJECT_ASSERT(timebase, NEXUS_Timebase);

    install = (timebase->settings.sourceType == NEXUS_TimebaseSourceType_ePcr &&
               timebase->settings.sourceSettings.pcr.pidChannel &&
               timebase->settings.pcrErrorCallback.callback &&
               !forceOff);

    if ( install ) {
        NEXUS_IsrCallback_Set(timebase->pcrErrorCallback, &timebase->settings.pcrErrorCallback);
    }
    else {
        NEXUS_IsrCallback_Set(timebase->pcrErrorCallback, NULL);
    }

    if (install && !timebase->intPcrErrorCallback) {
        BERR_Code rc;
        BDBG_MSG(("Installing DPCR%u TWO_PCR_ERROR interrupt handler", timebase->hwIndex));

        rc = BXPT_PCR_GetIntId( timebase->hwIndex, BXPT_PCR_IntName_eTwoPcrErrors, &dpcrErrorIntId );
        if (rc) return BERR_TRACE(rc);

        rc = BINT_CreateCallback(&timebase->intPcrErrorCallback, g_pCoreHandles->bint, dpcrErrorIntId, NEXUS_Timebase_P_TwoPcrError_isr, timebase, 0);
        if (rc) return BERR_TRACE(rc);
        rc = BINT_EnableCallback(timebase->intPcrErrorCallback);
        if (rc) return BERR_TRACE(rc);
    }
    else if (!install && timebase->intPcrErrorCallback) {
        BINT_DisableCallback(timebase->intPcrErrorCallback);
        BINT_DestroyCallback(timebase->intPcrErrorCallback);
        timebase->intPcrErrorCallback = NULL;
    }

    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_Timebase_P_GetPcr_isr(NEXUS_TimebaseHandle timebase, uint32_t *pHi, uint32_t *pLo, int32_t *pError)
{
    BERR_Code rc = BERR_SUCCESS;
    uint32_t hi, lo;
    do
    {
#if B_REFSW_DSS_SUPPORT
        if (timebase->isDss)
        {
            rc = BXPT_PCR_DirecTv_GetLastPcr_isr(timebase->pcr, pHi);
            *pLo = 0;
        }
        else
#endif
        {
            rc = BXPT_PCR_GetLastPcr_isr(timebase->pcr, pHi, pLo);
        }
        if (rc) break;
        BXPT_PCR_GetPhaseError_isr(timebase->pcr, pError);
        /* read again to make sure that the phase goes with the correct PCR */
#if B_REFSW_DSS_SUPPORT
        if (timebase->isDss)
        {
            rc = BXPT_PCR_DirecTv_GetLastPcr_isr(timebase->pcr, &hi);
        }
        else
#endif
        {
            rc = BXPT_PCR_GetLastPcr_isr(timebase->pcr, &hi, &lo);
        }
        if (rc) break;
    } while (*pHi != hi);
    return rc;
}

NEXUS_Error NEXUS_Timebase_GetStatus_priv_isr(NEXUS_TimebaseHandle timebase, NEXUS_TimebaseStatus *pStatus)
{
    BDBG_OBJECT_ASSERT(timebase, NEXUS_Timebase);
    *pStatus = timebase->status;
    return NEXUS_Timebase_P_GetPcr_isr(timebase, &pStatus->lastValue, &pStatus->lastValueLo, &pStatus->lastError);
}

NEXUS_Error NEXUS_Timebase_GetStatus(NEXUS_Timebase timebase, NEXUS_TimebaseStatus *pStatus)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_TimebaseHandle handle;

    handle = NEXUS_Timebase_P_ResolveAcquire(timebase, false);

    if (handle)
    {
        BKNI_EnterCriticalSection();
        rc = NEXUS_Timebase_GetStatus_priv_isr(handle, pStatus);
        BKNI_LeaveCriticalSection();
    }
    else
    {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    return rc;
}

NEXUS_Error NEXUS_Timebase_ResetStatus(NEXUS_Timebase timebase)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_TimebaseHandle handle;

    handle = NEXUS_Timebase_Resolve_priv(timebase);

    if (handle)
    {
        BKNI_Memset(&handle->status, 0, sizeof(handle->status));
        /* this sets count to 0 and valid to false. */
        handle->status.sourceType = handle->settings.sourceType;

#if NEXUS_HAS_ASTM
        if (handle->astm.settings.enabled)
        {
            if (handle->astm.settings.clockCoupling == NEXUS_TimebaseClockCoupling_eInternalClock)
            {
                handle->status.sourceType = NEXUS_TimebaseSourceType_eFreeRun;
            }
        }
#endif
    }
    else
    {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    return rc;
}

#if NEXUS_HAS_ASTM
void NEXUS_Timebase_GetAstmSettings_priv(
    NEXUS_TimebaseHandle timebase,
    NEXUS_TimebaseAstmSettings * pAstmSettings  /* [out] */
)
{
    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(timebase, NEXUS_Timebase);

    *pAstmSettings = timebase->astm.settings;
}

NEXUS_Error NEXUS_Timebase_SetAstmSettings_priv(
    NEXUS_TimebaseHandle timebase,
    const NEXUS_TimebaseAstmSettings * pAstmSettings
)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(timebase, NEXUS_Timebase);

    /* copy settings as-is, this way ASTM will always get what it set */
    timebase->astm.settings = *pAstmSettings;

    /* if ASTM is internally permitted, reapply settings */
    rc = NEXUS_Timebase_P_SetSettings(timebase, &timebase->settings);

    return rc;
}
bool NEXUS_Timebase_IsDataPresent_priv(
    NEXUS_TimebaseHandle timebase
    )
{
    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(timebase, NEXUS_Timebase);

    if (timebase->settings.sourceType == NEXUS_TimebaseSourceType_ePcr)
    {
        return NEXUS_PidChannel_P_IsDataPresent(timebase->settings.sourceSettings.pcr.pidChannel);
    }
    else
    {
        return false;
    }
}
#endif /* NEXUS_HAS_ASTM */

NEXUS_Error NEXUS_Timebase_GetIndex( NEXUS_Timebase timebase, unsigned *pIndex )
{
    NEXUS_TimebaseHandle handle;
    handle = NEXUS_Timebase_P_ResolveAcquire(timebase, false);
    if (!handle) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    *pIndex = handle->hwIndex;
    return 0;
}

NEXUS_Error NEXUS_Timebase_SetHdDviFrameRate( NEXUS_Timebase timebase, NEXUS_VideoFrameRate frameRate )
{
    NEXUS_TimebaseHandle handle;
    handle = NEXUS_Timebase_Resolve_priv(timebase);
    if (!handle) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    BDBG_MSG(("NEXUS_Timebase_SetHdDviFrameRate"));
    handle->hdDviFrameRate = frameRate;
    return NEXUS_Timebase_P_SetSettings(handle, &handle->settings);
}

NEXUS_Error NEXUS_Timebase_SetVdecFrameRate( NEXUS_Timebase timebase, NEXUS_VideoFrameRate frameRate )
{
    NEXUS_TimebaseHandle handle;
    handle = NEXUS_Timebase_Resolve_priv(timebase);
    if (!handle) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    BDBG_MSG(("NEXUS_Timebase_SetVdecFrameRate"));
    handle->vdecFrameRate = frameRate;
    return NEXUS_Timebase_P_SetSettings(handle, &handle->settings);
}

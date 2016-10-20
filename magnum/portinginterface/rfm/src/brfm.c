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
 *****************************************************************************/

#include "bstd.h"
#include "brfm.h"
#include "brsp.h"
#include "bchp_rfm_clk27.h"
#include "bchp_rfm_grb.h"
#include "bchp_rfm_l2.h"
#include "bchp_rfm_sysclk.h"
#include "bchp_int_id_rfm_l2.h"
#include "brfm_priv.h"
#include "brfm_scripts.h"
#include "bchp_clkgen.h"

#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

/* Power management in RFM is done this way:

BRFM_P_PowerDown and BRFM_P_PowerUp are never called directly, but always through BRFM_P_SetPowerSaverMode,
which makes sure that we never power down/up only from the opposite state.

BRFM_P_PowerDown and BRFM_P_PowerUp call BCHP_PWR functions. The refcnt should never go above 1.

EnablePowerSaver gets you into low-power state. DisableRfOutput is just DAC disabled. Perhaps DisableRfOutput should
do the same thing as EnablePowerSaver, but I've left it as is.
*/

BDBG_MODULE(BRFM);

#define CHK_RETCODE(rc, func)                      \
do {                                               \
    if ((rc = BERR_TRACE(func)) != BERR_SUCCESS) { \
        goto done;                                 \
    }                                              \
} while(0)

#define DEV_MAGIC_ID ((BERR_RFM_ID<<16) | 0xFACE)

#define MAKE_INT_ID(IntName)   BCHP_INT_ID_##IntName
#define MAKE_INT_ENUM(IntName) BRFM_IntType_e##IntName

typedef enum
{
#if (BRFM_REVID>=40)
    MAKE_INT_ENUM(GISB_ERROR_INTR),             /* BRFM Interrupt Id = 0x00 */
    MAKE_INT_ENUM(AUDIO_FIFO_UF_INTR),          /* BRFM Interrupt Id = 0x01 */
    MAKE_INT_ENUM(AUDIO_FIFO_OF_INTR),          /* BRFM Interrupt Id = 0x02 */
    MAKE_INT_ENUM(VIDEO_FIFO_UF_INTR),          /* BRFM Interrupt Id = 0x03 */
    MAKE_INT_ENUM(VIDEO_FIFO_OF_INTR),          /* BRFM Interrupt Id = 0x04 */
    MAKE_INT_ENUM(AUDIO_VIDEO_CLIP_INTR),       /* BRFM Interrupt Id = 0x05 */
    MAKE_INT_ENUM(AUDIO_LEFT_CLIP_INTR),        /* BRFM Interrupt Id = 0x06 */
    MAKE_INT_ENUM(AUDIO_RIGHT_CLIP_INTR),       /* BRFM Interrupt Id = 0x07 */
    MAKE_INT_ENUM(AUDIO_RATE_CLIP_INTR),        /* BRFM Interrupt Id = 0x08 */
    MAKE_INT_ENUM(AUDIO_LINE_LOSS_INTR),        /* BRFM Interrupt Id = 0x09 */
    MAKE_INT_ENUM(AUDIO_SUM_CLIP_INTR),         /* BRFM Interrupt Id = 0x0A */
    MAKE_INT_ENUM(AUDIO_DIFF_CLIP_INTR),        /* BRFM Interrupt Id = 0x0B */
    MAKE_INT_ENUM(AUDIO_SUMDIFF_CLIP_INTR),     /* BRFM Interrupt Id = 0x0C */
    MAKE_INT_ENUM(AUDIO_COMP_CLIP_INTR),        /* BRFM Interrupt Id = 0x0D */
    MAKE_INT_ENUM(AUDIO_VID_FIFO_UF_INTR),      /* BRFM Interrupt Id = 0x0E */
    MAKE_INT_ENUM(AUDIO_VID_FIFO_OF_INTR),      /* BRFM Interrupt Id = 0x0F */
    MAKE_INT_ENUM(NICAM_FIFO_UF_INTR),          /* BRFM Interrupt Id = 0x10 */
    MAKE_INT_ENUM(NICAM_FIFO_OF_INTR),          /* BRFM Interrupt Id = 0x11 */

#if BRFM_DUAL_DAC
    MAKE_INT_ENUM(BBE_AUDIO_FIFO_UF_INTR),      /* BRFM Interrupt Id = 0x12 */
    MAKE_INT_ENUM(BBE_AUDIO_FIFO_OF_INTR),      /* BRFM Interrupt Id = 0x13 */
    MAKE_INT_ENUM(BBE_AUDIO_VIDEO_CLIP_INTR),   /* BRFM Interrupt Id = 0x14 */
    MAKE_INT_ENUM(BBE_AUDIO_LEFT_CLIP_INTR),    /* BRFM Interrupt Id = 0x15 */
    MAKE_INT_ENUM(BBE_AUDIO_RIGHT_CLIP_INTR),   /* BRFM Interrupt Id = 0x16 */
    MAKE_INT_ENUM(BBE_AUDIO_RATE_CLIP_INTR),    /* BRFM Interrupt Id = 0x17 */
    MAKE_INT_ENUM(BBE_AUDIO_LINE_LOSS_INTR),    /* BRFM Interrupt Id = 0x18 */
    MAKE_INT_ENUM(BBE_AUDIO_SUM_CLIP_INTR),     /* BRFM Interrupt Id = 0x19 */
    MAKE_INT_ENUM(BBE_AUDIO_DIFF_CLIP_INTR),    /* BRFM Interrupt Id = 0x1A */
    MAKE_INT_ENUM(BBE_AUDIO_SUMDIFF_CLIP_INTR), /* BRFM Interrupt Id = 0x1B */
    MAKE_INT_ENUM(BBE_AUDIO_COMP_CLIP_INTR),    /* BRFM Interrupt Id = 0x1C */
    MAKE_INT_ENUM(BBE_AUDIO_VID_FIFO_UF_INTR),  /* BRFM Interrupt Id = 0x1D */
    MAKE_INT_ENUM(BBE_AUDIO_VID_FIFO_OF_INTR),  /* BRFM Interrupt Id = 0x1E */
#endif

#endif /* (BRFM_REVID>=40) */
    MAKE_INT_ENUM(Last)
} BRFM_IntType;

typedef struct
{
    bool isPowerSaverEnable;
    bool isOutputEnable;
    BRFM_ModulationType modType;
    BRFM_OutputChannel outputChannel;
    bool isMuteEnable;
    BRFM_AudioEncoding audioEncoding;
    int volume;
    const BRFM_P_ModulationInfo *pModInfo;
    BRFM_EncoderType encType;
} BRFM_P_Settings;

typedef struct BRFM_P_Handle
{
    uint32_t magicId;           /* Used to check if structure is corrupt */
    BCHP_Handle hChip;
    BREG_Handle hRegister;
    BINT_Handle hInterrupt;
    BINT_CallbackHandle hCallback[MAKE_INT_ENUM(Last)];
    uint32_t isCallbackDisable; /* assumes that total interrupt is less than 32 */
    uint32_t coreOffset;
    bool otpDisabled;
    BRFM_P_Settings settings;
    BRFM_ConfigSettings configSettings;
} BRFM_P_Handle;

/* Default module settings */
static const BRFM_Settings defDevSettings =
{
    BRFM_SETTINGS_MODTYPE,
    BRFM_SETTINGS_CHNBR,
    BRFM_SETTINGS_AUDIOENCODING,
    BRFM_SETTINGS_VOLUME,
    BRFM_SETTINGS_ENCTYPE
};

static const BINT_Id IntId[] =
{

#if (BRFM_REVID>=40)
    #if (BCHP_CHIP!=7552)
    MAKE_INT_ID(GISB_ERROR_INTR),             /* BRFM Interrupt Id = 0x00 */
    #else
    MAKE_INT_ID(RFM_L2_GISB_ERROR_INTR),      /* BRFM Interrupt Id = 0x00 */
    #endif
    MAKE_INT_ID(AUDIO_FIFO_UF_INTR),          /* BRFM Interrupt Id = 0x01 */
    MAKE_INT_ID(AUDIO_FIFO_OF_INTR),          /* BRFM Interrupt Id = 0x02 */
    MAKE_INT_ID(VIDEO_FIFO_UF_INTR),          /* BRFM Interrupt Id = 0x03 */
    MAKE_INT_ID(VIDEO_FIFO_OF_INTR),          /* BRFM Interrupt Id = 0x04 */
    MAKE_INT_ID(AUDIO_VIDEO_CLIP_INTR),       /* BRFM Interrupt Id = 0x05 */
    MAKE_INT_ID(AUDIO_LEFT_CLIP_INTR),        /* BRFM Interrupt Id = 0x06 */
    MAKE_INT_ID(AUDIO_RIGHT_CLIP_INTR),       /* BRFM Interrupt Id = 0x07 */
    MAKE_INT_ID(AUDIO_RATE_CLIP_INTR),        /* BRFM Interrupt Id = 0x08 */
    MAKE_INT_ID(AUDIO_LINE_LOSS_INTR),        /* BRFM Interrupt Id = 0x09 */
    MAKE_INT_ID(AUDIO_SUM_CLIP_INTR),         /* BRFM Interrupt Id = 0x0A */
    MAKE_INT_ID(AUDIO_DIFF_CLIP_INTR),        /* BRFM Interrupt Id = 0x0B */
    MAKE_INT_ID(AUDIO_SUMDIFF_CLIP_INTR),     /* BRFM Interrupt Id = 0x0C */
    MAKE_INT_ID(AUDIO_COMP_CLIP_INTR),        /* BRFM Interrupt Id = 0x0D */
    MAKE_INT_ID(AUDIO_VID_FIFO_UF_INTR),      /* BRFM Interrupt Id = 0x0E */
    MAKE_INT_ID(AUDIO_VID_FIFO_OF_INTR),      /* BRFM Interrupt Id = 0x0F */
    MAKE_INT_ID(NICAM_FIFO_UF_INTR),          /* BRFM Interrupt Id = 0x10 */
    MAKE_INT_ID(NICAM_FIFO_OF_INTR),          /* BRFM Interrupt Id = 0x11 */

#if BRFM_DUAL_DAC
    MAKE_INT_ID(BBE_AUDIO_FIFO_UF_INTR),      /* BRFM Interrupt Id = 0x12 or 0x01 */
    MAKE_INT_ID(BBE_AUDIO_FIFO_OF_INTR),      /* BRFM Interrupt Id = 0x13 or 0x02 */
    MAKE_INT_ID(BBE_AUDIO_VIDEO_CLIP_INTR),   /* BRFM Interrupt Id = 0x14 or 0x05 */
    MAKE_INT_ID(BBE_AUDIO_LEFT_CLIP_INTR),    /* BRFM Interrupt Id = 0x15 or 0x06 */
    MAKE_INT_ID(BBE_AUDIO_RIGHT_CLIP_INTR),   /* BRFM Interrupt Id = 0x16 or 0x07 */
    MAKE_INT_ID(BBE_AUDIO_RATE_CLIP_INTR),    /* BRFM Interrupt Id = 0x17 or 0x08 */
    MAKE_INT_ID(BBE_AUDIO_LINE_LOSS_INTR),    /* BRFM Interrupt Id = 0x18 or 0x09 */
    MAKE_INT_ID(BBE_AUDIO_SUM_CLIP_INTR),     /* BRFM Interrupt Id = 0x19 or 0x0A */
    MAKE_INT_ID(BBE_AUDIO_DIFF_CLIP_INTR),    /* BRFM Interrupt Id = 0x1A or 0x0B */
    MAKE_INT_ID(BBE_AUDIO_SUMDIFF_CLIP_INTR), /* BRFM Interrupt Id = 0x1B or 0x0C */
    MAKE_INT_ID(BBE_AUDIO_COMP_CLIP_INTR),    /* BRFM Interrupt Id = 0x1C or 0x0D */
    MAKE_INT_ID(BBE_AUDIO_VID_FIFO_UF_INTR),  /* BRFM Interrupt Id = 0x1D or 0x10 */
    MAKE_INT_ID(BBE_AUDIO_VID_FIFO_OF_INTR),  /* BRFM Interrupt Id = 0x1E or 0x11 */
#endif
#endif
};

/*******************************************************************************
*
* Private functions
*
*******************************************************************************/
#if (BCHP_CHIP != 7543)
static void BRFM_HandleInterrupt_isr(
    void *pParam1, /* [in] Device channel handle */
    int parm2      /* [in] Interrupt Type, uses BRFM_IntType */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BRFM_Handle hDev;

    hDev = (BRFM_Handle)pParam1;
    BDBG_ASSERT(hDev);
    switch ((BRFM_IntType) parm2)
    {
        case MAKE_INT_ENUM(AUDIO_FIFO_UF_INTR):
        case MAKE_INT_ENUM(AUDIO_FIFO_OF_INTR):
        case MAKE_INT_ENUM(AUDIO_VID_FIFO_UF_INTR):
        case MAKE_INT_ENUM(AUDIO_VID_FIFO_OF_INTR):
        case MAKE_INT_ENUM(VIDEO_FIFO_UF_INTR):
        case MAKE_INT_ENUM(VIDEO_FIFO_OF_INTR):
        case MAKE_INT_ENUM(AUDIO_VIDEO_CLIP_INTR):
        case MAKE_INT_ENUM(AUDIO_LEFT_CLIP_INTR):
        case MAKE_INT_ENUM(AUDIO_RIGHT_CLIP_INTR):
        case MAKE_INT_ENUM(AUDIO_RATE_CLIP_INTR):
        case MAKE_INT_ENUM(AUDIO_LINE_LOSS_INTR):
        case MAKE_INT_ENUM(AUDIO_SUM_CLIP_INTR):
        case MAKE_INT_ENUM(AUDIO_DIFF_CLIP_INTR):
        case MAKE_INT_ENUM(AUDIO_SUMDIFF_CLIP_INTR):
        case MAKE_INT_ENUM(AUDIO_COMP_CLIP_INTR):

#if (BRFM_REVID>=40)
        case MAKE_INT_ENUM(NICAM_FIFO_UF_INTR):
        case MAKE_INT_ENUM(NICAM_FIFO_OF_INTR):

/* additional interrupts for 65nm platforms with dual BTSC */
#if BRFM_DUAL_DAC
        case MAKE_INT_ENUM(BBE_AUDIO_FIFO_UF_INTR):
        case MAKE_INT_ENUM(BBE_AUDIO_FIFO_OF_INTR):
        case MAKE_INT_ENUM(BBE_AUDIO_VIDEO_CLIP_INTR):
        case MAKE_INT_ENUM(BBE_AUDIO_LEFT_CLIP_INTR):
        case MAKE_INT_ENUM(BBE_AUDIO_RIGHT_CLIP_INTR):
        case MAKE_INT_ENUM(BBE_AUDIO_RATE_CLIP_INTR):
        case MAKE_INT_ENUM(BBE_AUDIO_LINE_LOSS_INTR):
        case MAKE_INT_ENUM(BBE_AUDIO_SUM_CLIP_INTR):
        case MAKE_INT_ENUM(BBE_AUDIO_DIFF_CLIP_INTR):
        case MAKE_INT_ENUM(BBE_AUDIO_SUMDIFF_CLIP_INTR):
        case MAKE_INT_ENUM(BBE_AUDIO_COMP_CLIP_INTR):
        case MAKE_INT_ENUM(BBE_AUDIO_VID_FIFO_UF_INTR):
        case MAKE_INT_ENUM(BBE_AUDIO_VID_FIFO_OF_INTR):
#endif

#else /* (BRFM_REVID>=40) */
        case MAKE_INT_ENUM(RFIFD_FIFO_UF_INTR):
        case MAKE_INT_ENUM(RFIFD_FIFO_OF_INTR):
#endif
            /* Notify on logging port that an interrupt occurred */
            BDBG_MSG(("BRFM_HandleInterrupt_isr: Interrupt Id=0x%02x, Disabling", parm2));
            /* After the first time the interrupt comes, disable the interrupt, so
               that the interrupts don't flood the system if the interrupt can't be cleared.
               Interrupt will be re-enabled when the mode set, see that function in this file. */
            if (hDev->hCallback[parm2])
                CHK_RETCODE(retCode, BINT_DisableCallback_isr( hDev->hCallback[parm2]));
            hDev->isCallbackDisable |= (1 << parm2);
            break;
        case MAKE_INT_ENUM(GISB_ERROR_INTR):
            /* Notify logging port that an interrupt occurred */
            BDBG_ERR(("BRFM_HandleInterrupt_isr: Reset is required!"));
            /* TODO: Resetting the RFM core may be required */
            break;
        default:
            BDBG_ERR(("BRFM_HandleInterrupt_isr: Unknown Interrupt Id=0x%x!", parm2));
            break;
    }
done:
    return;
}
#endif

static BERR_Code BRFM_EnableDisabledInterrupt(BRFM_Handle hDev)
{
    BERR_Code retCode = BERR_SUCCESS;
    unsigned idx;
    uint32_t bitMask;

    if (hDev->isCallbackDisable != 0) {
        bitMask = 1;
        for (idx=0; idx<MAKE_INT_ENUM(Last); idx++, bitMask=(bitMask << 1)) {
            if (hDev->isCallbackDisable & bitMask) {
            if (hDev->hCallback[idx])
                CHK_RETCODE(retCode, BINT_EnableCallback( hDev->hCallback[idx]));
                hDev->isCallbackDisable &= ~bitMask;
                BDBG_MSG(("BRFM: Interrupt Id=0x%02x, Re-enabling", idx));
            }
        }
    }

done:
    return retCode;
}

static void BRFM_P_SysclkReset(BRFM_Handle hDev)
{
    uint32_t ulVal;
    BDBG_MSG(("BRFM_P_SysclkReset"));
    if (hDev->settings.encType == BRFM_EncoderType_singleBTSC)
    {
        /* Assert Reset */
        ulVal = BREG_Read32(hDev->hRegister, BCHP_RFM_SYSCLK_RESET);
#if (BRFM_REVID>=40) && (BRFM_REVID<52)
        ulVal |= BCHP_FIELD_DATA(RFM_SYSCLK_RESET, NICAM_SFT_RST, 0x01);
#endif
#if (BRFM_REVID<52)
        ulVal |= BCHP_FIELD_DATA(RFM_SYSCLK_RESET, VFIFO_SFT_RST, 0x01);
        ulVal |= BCHP_FIELD_DATA(RFM_SYSCLK_RESET, AFIFO_SFT_RST, 0x01);
#endif
        ulVal |= BCHP_FIELD_DATA(RFM_SYSCLK_RESET, CLKGEN_SFT_RST, 0x01);
        ulVal |= BCHP_FIELD_DATA(RFM_SYSCLK_RESET, AM_SFT_RST, 0x01);
        ulVal |= BCHP_FIELD_DATA(RFM_SYSCLK_RESET, VIDEO_SFT_RST, 0x01);
        ulVal |= BCHP_FIELD_DATA(RFM_SYSCLK_RESET, AUDIO_SFT_RST, 0x01);
        BREG_Write32(hDev->hRegister, BCHP_RFM_SYSCLK_RESET, ulVal);

        /* Deassert Reset */
#if (BRFM_REVID>51)
        ulVal = BREG_Read32(hDev->hRegister, BCHP_RFM_SYSCLK_RESET);
        ulVal &= BCHP_FIELD_DATA(RFM_SYSCLK_RESET, CLKGEN_SFT_RST, 0x00);
        BREG_Write32(hDev->hRegister, BCHP_RFM_SYSCLK_RESET, ulVal);
#endif
        ulVal = BREG_Read32(hDev->hRegister, BCHP_RFM_SYSCLK_RESET);
#if (BRFM_REVID>=40) && (BRFM_REVID<52)
        ulVal &= BCHP_FIELD_DATA(RFM_SYSCLK_RESET, NICAM_SFT_RST, 0x00);
#endif
#if (BRFM_REVID<52)
        ulVal &= BCHP_FIELD_DATA(RFM_SYSCLK_RESET, VFIFO_SFT_RST, 0x00);
        ulVal &= BCHP_FIELD_DATA(RFM_SYSCLK_RESET, AFIFO_SFT_RST, 0x00);
        ulVal &= BCHP_FIELD_DATA(RFM_SYSCLK_RESET, CLKGEN_SFT_RST, 0x00);
#endif
        ulVal &= BCHP_FIELD_DATA(RFM_SYSCLK_RESET, AM_SFT_RST, 0x00);
        ulVal &= BCHP_FIELD_DATA(RFM_SYSCLK_RESET, VIDEO_SFT_RST, 0x00);
        ulVal &= BCHP_FIELD_DATA(RFM_SYSCLK_RESET, AUDIO_SFT_RST, 0x00);
        BREG_Write32(hDev->hRegister, BCHP_RFM_SYSCLK_RESET, ulVal);
#if (BRFM_REVID>51)
        BREG_Write32(hDev->hRegister, BCHP_RFM_SYSCLK_RESET, 0);
#endif
    }
    else if (hDev->settings.encType == BRFM_EncoderType_dualBTSC)
    {
#if BRFM_DUAL_DAC
        /* Assert Reset */
        ulVal = BREG_Read32(hDev->hRegister, BCHP_RFM_BBE_SYSCLK_RESET);
        ulVal |= BCHP_FIELD_DATA(RFM_BBE_SYSCLK_RESET, AFIFO_SFT_RST, 0x01);
        ulVal |= BCHP_FIELD_DATA(RFM_BBE_SYSCLK_RESET, AM_SFT_RST, 0x01);
        ulVal |= BCHP_FIELD_DATA(RFM_BBE_SYSCLK_RESET, VIDEO_SFT_RST, 0x01);
        ulVal |= BCHP_FIELD_DATA(RFM_BBE_SYSCLK_RESET, AUDIO_SFT_RST, 0x01);
        BREG_Write32(hDev->hRegister, BCHP_RFM_BBE_SYSCLK_RESET, ulVal);

        /* Deassert Reset */
        ulVal = BREG_Read32(hDev->hRegister, BCHP_RFM_BBE_SYSCLK_RESET);
        ulVal &= BCHP_FIELD_DATA(RFM_BBE_SYSCLK_RESET, AFIFO_SFT_RST, 0x00);
        ulVal &= BCHP_FIELD_DATA(RFM_BBE_SYSCLK_RESET, AM_SFT_RST, 0x00);
        ulVal &= BCHP_FIELD_DATA(RFM_BBE_SYSCLK_RESET, VIDEO_SFT_RST, 0x00);
        ulVal &= BCHP_FIELD_DATA(RFM_BBE_SYSCLK_RESET, AUDIO_SFT_RST, 0x00);
        BREG_Write32(hDev->hRegister, BCHP_RFM_BBE_SYSCLK_RESET, ulVal);
#endif
    }
}

BERR_Code BRFM_P_CRC_BIST(BREG_Handle hRegister, BRFM_EncoderType enc);

void BRFM_P_SetTopLevelClock(BREG_Handle hReg, bool enable)
{
#if (BRFM_REVID<40) /* No PM registers */
    BSTD_UNUSED(hReg);
#else
    BREG_AtomicUpdate32(hReg, BRFM_P_CLK_PM_CTRL_BCHP, BRFM_P_CLK_PM_CTRL_MASK,
        enable ? BRFM_P_CLK_PM_CTRL_ENABLED : !BRFM_P_CLK_PM_CTRL_ENABLED);
#endif
}

bool BRFM_P_GetTopLevelClock(BREG_Handle hReg)
{
#if (BRFM_REVID<40)
    BSTD_UNUSED(hReg);
    return 1; /* always on */
#else
    uint32_t val;
    val = BREG_Read32(hReg, BRFM_P_CLK_PM_CTRL_BCHP);
    if (BCHP_GET_FIELD_DATA(val, BRFM_P_CLK_PM_CTRL_REG, BRFM_P_CLK_PM_CTRL_FIELD)==BRFM_P_CLK_PM_CTRL_ENABLED) {
        return true;
    }
    else {
        return false;
    }
#endif
}

static void BRFM_P_PowerDown(BRFM_Handle hDev)
{
#ifdef BCHP_PWR_RESOURCE_RFM
    BCHP_PWR_ReleaseResource(hDev->hChip, BCHP_PWR_RESOURCE_RFM);
#ifdef BCHP_PWR_RESOURCE_RFM_PHY
    BCHP_PWR_ReleaseResource(hDev->hChip, BCHP_PWR_RESOURCE_RFM_PHY);
#endif
#elif (BRFM_REVID<40)
    BSTD_UNUSED(hDev);
#else
    BREG_Handle hReg = hDev->hRegister;
    uint32_t ulRegData;

    /* check if top-level clock has been disabled. if disabled, it was most likely shut down via BPWR. */
    if (BRFM_P_GetTopLevelClock(hReg)==false) {
        BDBG_WRN(("BRFM_P_PowerDown: Power down requested while top-level RFM clock disabled"));
        return;
    }

    /* 1 */
    ulRegData = BREG_Read32(hReg, BCHP_RFM_SYSCLK_DACCTL);
    ulRegData |= BCHP_RFM_SYSCLK_DACCTL_DAC_PWRDN_MASK;
    BREG_Write32(hReg, BCHP_RFM_SYSCLK_DACCTL, ulRegData);
#if BRFM_DUAL_DAC
    /* for dual DAC RFM cores, a single call to EnablePowerSaver disables both */
    ulRegData = BREG_Read32(hReg, BCHP_RFM_SYSCLK_DACCTL);
    ulRegData |= BCHP_RFM_SYSCLK_DACCTL_DAC_PWRDN_2_MASK;
    BREG_Write32(hReg, BCHP_RFM_SYSCLK_DACCTL, ulRegData);
#endif
    /* 2 */
    ulRegData = BREG_Read32(hReg, BCHP_RFM_SYSCLK_CLKCTL);
    ulRegData |= BCHP_RFM_SYSCLK_CLKCTL_CLK_OFF_MASK;
    BREG_Write32(hReg, BCHP_RFM_SYSCLK_CLKCTL, ulRegData);
    /* 3 */
#if (BRFM_REVID<51) /* no PHASEITP register for REV 5.1 */
    ulRegData = BREG_Read32(hReg, BCHP_RFM_SYSCLK_RFM_PHASEITP_CTL);
    ulRegData |= BCHP_RFM_SYSCLK_RFM_PHASEITP_CTL_PHASEITP_PWRDN_MASK;
    BREG_Write32(hReg, BCHP_RFM_SYSCLK_RFM_PHASEITP_CTL, ulRegData);
#endif
    /* 4 */
#if (BRFM_REVID>=51)
    ulRegData = BREG_Read32(hReg, BCHP_RFM_SYSCLK_DPLL_MISC1);
    #if (BCHP_CHIP!=7445) && (BCHP_CHIP!=7145) && (BCHP_CHIP!=7439) && (BCHP_CHIP!=74371) && (BCHP_CHIP!=7364)
    ulRegData |= BCHP_RFM_SYSCLK_DPLL_MISC1_REFCOMP_PWRDN_MASK;
    #endif
    ulRegData |= BCHP_RFM_SYSCLK_DPLL_MISC1_PWRDN_MASK;
    BREG_Write32(hReg, BCHP_RFM_SYSCLK_DPLL_MISC1, ulRegData);
#else
    ulRegData = BREG_Read32(hReg, BCHP_RFM_SYSCLK_PLL_MISC1);
    ulRegData |= BCHP_RFM_SYSCLK_PLL_MISC1_NDIV_PWRDN_MASK |
             BCHP_RFM_SYSCLK_PLL_MISC1_REFCOMP_PWRDN_MASK |
             BCHP_RFM_SYSCLK_PLL_MISC1_PWRDN_MASK;
    BREG_Write32(hReg, BCHP_RFM_SYSCLK_PLL_MISC1, ulRegData);
#endif
    /* 5 */
#if (BRFM_REVID<51)
    ulRegData = BREG_Read32(hReg, BCHP_RFM_SYSCLK_DACCTL);
    #if (BCHP_CHIP!=7445) && (BCHP_CHIP!=7145) && (BCHP_CHIP!=7439) && (BCHP_CHIP!=74371) && (BCHP_CHIP!=7364)
    ulRegData |= BCHP_RFM_SYSCLK_DACCTL_BIAS50U_OFF_MASK |
             BCHP_RFM_SYSCLK_DACCTL_BIASLOG_OFF_MASK;
    #endif
    BREG_Write32(hReg, BCHP_RFM_SYSCLK_DACCTL, ulRegData);
#endif

    /* 6 */
#if (BRFM_REVID>51)
    ulRegData = BREG_Read32(hReg, BCHP_RFM_SYSCLK_CLKCTL);
    ulRegData |= BCHP_RFM_SYSCLK_CLKCTL_LDO_OFF_MASK;
    BREG_Write32(hReg, BCHP_RFM_SYSCLK_CLKCTL, ulRegData);

    ulRegData = BREG_Read32(hReg, BCHP_RFM_SYSCLK_CLKCTL);
    ulRegData |= BCHP_RFM_SYSCLK_CLKCTL_BGCORE_OFF_MASK;
    BREG_Write32(hReg, BCHP_RFM_SYSCLK_CLKCTL, ulRegData);
#endif
    /* 7 */
    ulRegData = BREG_Read32(hReg, BCHP_RFM_SYSCLK_CLKCTL);
    ulRegData |= BCHP_RFM_SYSCLK_CLKCTL_RFMCLK_OFF_MASK |
#if (BRFM_REVID<51)
             BCHP_RFM_SYSCLK_CLKCTL_VREG_OFF_MASK |
#endif
             BCHP_RFM_SYSCLK_CLKCTL_BGCORE_OFF_MASK;
    BREG_Write32(hReg, BCHP_RFM_SYSCLK_CLKCTL, ulRegData);

    /* leave top-level clock running (required for BINT_Open during next init) */

    /* do not set settings.isOutputEnable=false, since it will trigger setting
       all registers again in EnableRfOutput(), which is unnecessary */
#endif
}

static void BRFM_P_PowerUp(BRFM_Handle hDev, bool runCRC)
{
#ifdef BCHP_PWR_RESOURCE_RFM
    BCHP_PWR_AcquireResource(hDev->hChip, BCHP_PWR_RESOURCE_RFM);
#ifdef BCHP_PWR_RESOURCE_RFM_PHY
    BCHP_PWR_AcquireResource(hDev->hChip, BCHP_PWR_RESOURCE_RFM_PHY);
#endif
#elif (BRFM_REVID<40)
    BSTD_UNUSED(hDev);
    BSTD_UNUSED(runCRC);
#else
    uint32_t ulRegData;

    /* 1 */
    BRFM_P_SetTopLevelClock(hDev->hRegister, true);

    /* 2 */
    ulRegData = BREG_Read32(hDev->hRegister, BCHP_RFM_SYSCLK_CLKCTL);
    ulRegData &= ~(BCHP_RFM_SYSCLK_CLKCTL_RFMCLK_OFF_MASK |
#if (BRFM_REVID<51)
                   BCHP_RFM_SYSCLK_CLKCTL_VREG_OFF_MASK |
#endif
                   BCHP_RFM_SYSCLK_CLKCTL_BGCORE_OFF_MASK);
    BREG_Write32(hDev->hRegister, BCHP_RFM_SYSCLK_CLKCTL, ulRegData);
    /* 3 */
#if (BRFM_REVID<51) /* no PHASEITP register for REV 5.1 */
    ulRegData = BREG_Read32(hDev->hRegister, BCHP_RFM_SYSCLK_RFM_PHASEITP_CTL);
    ulRegData &= ~(BCHP_RFM_SYSCLK_RFM_PHASEITP_CTL_PHASEITP_PWRDN_MASK);
    BREG_Write32(hDev->hRegister, BCHP_RFM_SYSCLK_RFM_PHASEITP_CTL, ulRegData);
#endif
    /* 4 */
#if (BRFM_REVID>51)
    BREG_Write32(hDev->hRegister, BCHP_RFM_SYSCLK_DPLL_MISC1, 0x0a00a763);
#elif (BRFM_REVID==51)
    ulRegData = BREG_Read32(hDev->hRegister, BCHP_RFM_SYSCLK_DPLL_MISC1);
    ulRegData &= ~(BCHP_RFM_SYSCLK_DPLL_MISC1_REFCOMP_PWRDN_MASK |
                   BCHP_RFM_SYSCLK_DPLL_MISC1_PWRDN_MASK);
    BREG_Write32(hDev->hRegister, BCHP_RFM_SYSCLK_DPLL_MISC1, ulRegData);
#else
    ulRegData = BREG_Read32(hDev->hRegister, BCHP_RFM_SYSCLK_PLL_MISC1);
    ulRegData &= ~(BCHP_RFM_SYSCLK_PLL_MISC1_NDIV_PWRDN_MASK |
                   BCHP_RFM_SYSCLK_PLL_MISC1_REFCOMP_PWRDN_MASK |
                   BCHP_RFM_SYSCLK_PLL_MISC1_PWRDN_MASK);
    BREG_Write32(hDev->hRegister, BCHP_RFM_SYSCLK_PLL_MISC1, ulRegData);
#endif
    /* 5 */
#if (BRFM_REVID<51)
    ulRegData = BREG_Read32(hDev->hRegister, BCHP_RFM_SYSCLK_DACCTL);
    ulRegData &= ~(BCHP_RFM_SYSCLK_DACCTL_BIAS50U_OFF_MASK |
                   BCHP_RFM_SYSCLK_DACCTL_BIASLOG_OFF_MASK);
    BREG_Write32(hDev->hRegister, BCHP_RFM_SYSCLK_DACCTL, ulRegData);
#endif
    /* 6 */
    BKNI_Sleep(1); /* wait 50us for RFM PLL to lock */
    /* 7 */
    ulRegData = BREG_Read32(hDev->hRegister, BCHP_RFM_SYSCLK_CLKCTL);
#if (BRFM_REVID>51)
    ulRegData &= ~(BCHP_RFM_SYSCLK_CLKCTL_BGCORE_OFF_MASK);
#else
    ulRegData &= ~(BCHP_RFM_SYSCLK_CLKCTL_CLK_OFF_MASK);
#endif
    BREG_Write32(hDev->hRegister, BCHP_RFM_SYSCLK_CLKCTL, ulRegData);

#if (BRFM_REVID>51)
    BKNI_Sleep(1); /* wait 20us */
    ulRegData = BREG_Read32(hDev->hRegister, BCHP_RFM_SYSCLK_CLKCTL);
    ulRegData &= ~(BCHP_RFM_SYSCLK_CLKCTL_LDO_OFF_MASK);
    BREG_Write32(hDev->hRegister, BCHP_RFM_SYSCLK_CLKCTL, ulRegData);

    ulRegData = BREG_Read32(hDev->hRegister, BCHP_RFM_SYSCLK_DPLL_MISC1);
    ulRegData &= ~(BCHP_RFM_SYSCLK_DPLL_MISC1_PWRDN_MASK);
    BREG_Write32(hDev->hRegister, BCHP_RFM_SYSCLK_DPLL_MISC1, ulRegData);

    ulRegData = BREG_Read32(hDev->hRegister, BCHP_RFM_SYSCLK_CLKCTL);
    ulRegData &= ~(BCHP_RFM_SYSCLK_CLKCTL_CLK_OFF_MASK);
    BREG_Write32(hDev->hRegister, BCHP_RFM_SYSCLK_CLKCTL, ulRegData);
#endif
    /* 8 */
    ulRegData = BREG_Read32(hDev->hRegister, BCHP_RFM_SYSCLK_DACCTL);
    ulRegData &= ~(BCHP_RFM_SYSCLK_DACCTL_DAC_PWRDN_MASK);
    BREG_Write32(hDev->hRegister, BCHP_RFM_SYSCLK_DACCTL, ulRegData);
#if (BRFM_REVID>51)
    BREG_Write32(hDev->hRegister, BCHP_RFM_SYSCLK_DPLL_MISC1, 0x0a00a760);
#endif
#if BRFM_DUAL_DAC
    /* for dual DAC RFM cores, power up of either core enables both */
    ulRegData = BREG_Read32(hDev->hRegister, BCHP_RFM_SYSCLK_DACCTL);
    ulRegData &= ~(BCHP_RFM_SYSCLK_DACCTL_DAC_PWRDN_2_MASK);
    BREG_Write32(hDev->hRegister, BCHP_RFM_SYSCLK_DACCTL, ulRegData);
#endif
#endif /* (BRFM_REVID<40) */

#if (BRFM_REVID>=40) || (defined BCHP_PWR_RESOURCE_RFM)
    /* assert and deassert SYSCLK_RESET after every power-up */
    BRFM_P_SysclkReset(hDev);

    /* 9 */
    if (runCRC) {
#if (!BRFM_DUAL_DAC)
        BRFM_P_CRC_BIST(hDev->hRegister, hDev->settings.encType);
#else
        /* since power up of either core enables both, run CRC on both DACs  */
        BRFM_P_CRC_BIST(hDev->hRegister, BRFM_EncoderType_dualBTSC);
#endif
    }
#endif
}

static void BRFM_P_SetPowerSaverMode(BRFM_Handle hDev, bool bEnable /* 1==enable power saver mode */)
{
#if (BRFM_REVID>=40)
    unsigned idx;
    if (bEnable==true && hDev->settings.isPowerSaverEnable==false) {
        BDBG_MSG(("BRFM_P_SetPowerSaverMode: %d->%d", hDev->settings.isPowerSaverEnable, bEnable));
        for (idx = 0; idx < MAKE_INT_ENUM(Last); idx++) {
             if( hDev->hCallback[idx] != NULL ) {
                BINT_DisableCallback( hDev->hCallback[idx] );
             }
        }
        BRFM_P_PowerDown(hDev);
    }
    else if (bEnable==false && hDev->settings.isPowerSaverEnable==true) {
        BDBG_MSG(("BRFM_P_SetPowerSaverMode: %d->%d", hDev->settings.isPowerSaverEnable, bEnable));
        BRFM_P_PowerUp(hDev, true);
        for (idx = 0; idx < MAKE_INT_ENUM(Last); idx++) {
             if( hDev->hCallback[idx] != NULL ) {
                BINT_EnableCallback( hDev->hCallback[idx] );
             }
        }
    }
    hDev->settings.isPowerSaverEnable = bEnable;

#else
    uint32_t ulVal1, ulVal2;
    if (hDev->settings.isPowerSaverEnable != bEnable) {
        ulVal1 = BREG_Read32(hDev->hRegister, BCHP_RFM_SYSCLK_CLKCTL);
        ulVal2 = BREG_Read32(hDev->hRegister, BCHP_RFM_SYSCLK_DACCTL);

        if (bEnable == true) {
            /* power-down */
            ulVal1 |= BCHP_FIELD_DATA(RFM_SYSCLK_CLKCTL, PLL_OFF, 1);
            ulVal1 |= BCHP_FIELD_DATA(RFM_SYSCLK_CLKCTL, VREG_OFF, 1);
            ulVal2 |= BCHP_FIELD_DATA(RFM_SYSCLK_DACCTL, DIS_CLK, 1);
            hDev->settings.isOutputEnable = false;
        }
        else {
            /* power-up */
            ulVal1 &= ~(BCHP_FIELD_DATA(RFM_SYSCLK_CLKCTL, PLL_OFF, 1));
            ulVal1 &= ~(BCHP_FIELD_DATA(RFM_SYSCLK_CLKCTL, VREG_OFF, 1));
            ulVal2 &= ~(BCHP_FIELD_DATA(RFM_SYSCLK_DACCTL, DIS_CLK, 1));
            /* for non-65nm platforms, a call to EnableRfOutput is expected in order to set .isOutputEnable = true */
        }
        BREG_Write32(hDev->hRegister, BCHP_RFM_SYSCLK_CLKCTL, ulVal1);
        BREG_Write32(hDev->hRegister, BCHP_RFM_SYSCLK_DACCTL, ulVal2);
        hDev->settings.isPowerSaverEnable = bEnable;
    }
#endif

    return;
}

#if (BRFM_REVID<50)
/* dB values for range [-34, +30] dB */
static const uint16_t dB2Volume400[] =
{
    0x0014, 0x0017, 0x001A, 0x001D, 0x0020, 0x0024, 0x0029, 0x002E,
    0x0033, 0x003A, 0x0041, 0x0048, 0x0051, 0x005B, 0x0066, 0x0073,
    0x0081, 0x0091, 0x00A2, 0x00B6, 0x00CC, 0x00E5, 0x0101, 0x0121,
    0x0144, 0x016B, 0x0198, 0x01C9, 0x0201, 0x0240, 0x0286, 0x02D5,
    0x032D, 0x0391, 0x0400, 0x047D, 0x0509, 0x05A6, 0x0657, 0x071D,
    0x07FB, 0x08F4, 0x0A0C, 0x0B46, 0x0CA6, 0x0E31, 0x0FED, 0x11DE,
    0x140C, 0x167E, 0x193D, 0x1C51, 0x1FC6, 0x23A6, 0x2800, 0x2CE1,
    0x325B, 0x3880, 0x3F65, 0x4722, 0x4FCF, 0x598D, 0x647A, 0x70BC,
    0x7E7E
};

#else
/* dB values for new range [-52, +12] dB */
static const uint16_t dB2Volume2000[] =
{
    0x0015, 0x0017, 0x001A, 0x001D, 0x0021, 0x0025, 0x0029, 0x002E,
    0x0034, 0x003A, 0x0041, 0x0049, 0x0052, 0x005C, 0x0067, 0x0074,
    0x0082, 0x0092, 0x00A3, 0x00B7, 0x00CE, 0x00E7, 0x0103, 0x0123,
    0x0146, 0x016E, 0x019B, 0x01CD, 0x0205, 0x0244, 0x028B, 0x02DA,
    0x0333, 0x0397, 0x0407, 0x0485, 0x0512, 0x05B1, 0x0663, 0x072A,
    0x080A, 0x0905, 0x0A1F, 0x0B5B, 0x0CBD, 0x0E4B, 0x100A, 0x11FF,
    0x1431, 0x16A7, 0x196B, 0x1C85, 0x2000, 0x23E8, 0x2849, 0x2D34,
    0x32B7, 0x38E8, 0x3FD9, 0x47A4, 0x5061, 0x5A30, 0x6531, 0x718A,
    0x7F65
};
#endif

/* dB values for range [-52, 12] dB. used for NICAM volume control */
#if (BRFM_REVID<50)
static const uint16_t dB2Volume400_Nicam[] =
{
    0x0052, 0x005C, 0x0068, 0x0074, 0x0082, 0x0092, 0x00A4, 0x00B8,
    0x00CF, 0x00E8, 0x0082, 0x0092, 0x00A4, 0x005C, 0x0067, 0x0074,
    0x0041, 0x0049, 0x0052, 0x002E, 0x0033, 0x003A, 0x0020, 0x0024,
    0x0029, 0x002E, 0x0033, 0x003A, 0x0041, 0x0048, 0x0051, 0x005B,
    0x0066, 0x0073, 0x0081, 0x0091, 0x00A2, 0x00B6, 0x00CC, 0x00E5,
    0x0101, 0x0121, 0x0144, 0x016B, 0x0198, 0x01C9, 0x0201, 0x0240,
    0x0286, 0x02D5, 0x032D, 0x0391, 0x0400, 0x047D, 0x0509, 0x05A6,
    0x0657, 0x071D, 0x07FB, 0x08F4, 0x0A0C, 0x0B46, 0x0CA6, 0x0E31,
    0x0FED
};

#else
static const uint16_t dB2Volume2000_Nicam[] =
{
    0x0015, 0x0017, 0x001A, 0x001D, 0x0021, 0x0025, 0x0029, 0x002E,
    0x0034, 0x003A, 0x0041, 0x0049, 0x0052, 0x005C, 0x0067, 0x0074,
    0x0082, 0x0092, 0x00A3, 0x00B7, 0x00CE, 0x00E7, 0x0103, 0x0123,
    0x0146, 0x016E, 0x019B, 0x01CD, 0x0205, 0x0244, 0x028B, 0x02DA,
    0x0333, 0x0397, 0x0407, 0x0485, 0x0512, 0x05B1, 0x0663, 0x072A,
    0x080A, 0x0905, 0x0A1F, 0x0B5B, 0x0CBD, 0x0E4B, 0x100A, 0x11FF,
    0x1431, 0x16A7, 0x196B, 0x1C85, 0x2000, 0x23E8, 0x2849, 0x2D34,
    0x32B7, 0x38E8, 0x3FD9, 0x47A4, 0x5061, 0x5A30, 0x6531, 0x718A,
    0x7F65
};
#endif

void BRFM_P_CalculateVolume (
    int volume,         /* [in] requested audio volume */
    uint16_t *uLeftVal, /* [out] calculated value for L audio channel */
    uint16_t *uRightVal /* [out] calculated value for R audio channel */
    )
{
    int idx;

    if (volume < BRFM_MIN_VOLUME) {
        BDBG_WRN(("Volume too low (%d), setting to min", volume));
        volume = BRFM_MIN_VOLUME;
    }
    else if (volume > BRFM_MAX_VOLUME)    {
        BDBG_WRN(("Volume too high (%d), setting to max", volume));
        volume = BRFM_MAX_VOLUME;
    }

    idx = volume - BRFM_MIN_VOLUME; /* mapping -34 to 30 to a table of 0 to 63 entries */
#if (BRFM_REVID<50)
    *uLeftVal=dB2Volume400[idx];
    *uRightVal=dB2Volume400[idx];
#else
    *uLeftVal=dB2Volume2000[idx];
    *uRightVal=dB2Volume2000[idx];
#endif
}

#if (BRFM_REVID>=40)
void BRFM_P_CalculateNicamVolume(int volume, uint16_t *uVal)
{
    int idx = volume - BRFM_MIN_VOLUME_NICAM;
#if (BRFM_REVID<50)
    *uVal = dB2Volume400_Nicam[idx];
#else
    *uVal = dB2Volume2000_Nicam[idx];
#endif
}
#endif

/* CRC Built-in Self-Test */
BERR_Code BRFM_P_CRC_BIST(
    BREG_Handle hRegister, /* [in] Register handle */
    BRFM_EncoderType enc   /* [in] Encoder type */
    )
{
    /* the dual DAC case checks and corrects both DACs, but RFM isn't aware whether the user intends on using both.
       the safe approach is to perform the BIST based on encoder type of handle */

#if (BRFM_REVID==40)
    const int MAX_RETRY = 10;
    int i;
    uint32_t ulVal;
    BERR_Code retCode = BERR_SUCCESS;

    BREG_Write32( hRegister, BCHP_RFM_SYSCLK_CLKCTL, (0x11000000 | BRFM_PLL_MULT) );

    if (enc == BRFM_EncoderType_singleBTSC) { /* single DAC case */
        for (i=0; i<MAX_RETRY; i++) {
            BREG_Write32( hRegister, BCHP_RFM_SYSCLK_RFMPHY_BISTCNTL, 0x000103ff ); /* set BIST_EN */
            BKNI_Sleep(10);
            ulVal = BREG_Read32( hRegister, BCHP_RFM_SYSCLK_RFMPHY_STATUS );
            BREG_Write32( hRegister, BCHP_RFM_SYSCLK_RFMPHY_BISTCNTL, 0x000003ff ); /* reset BIST_EN */
            if (ulVal == 0x8000fcd9) {
                BDBG_MSG(("BRFM_CRC_BIST: RFM_SYSCLK.RFMPHY_STATUS read PASS,  retry_count = %d", i));
                break;
            }
            else {
                BDBG_MSG(("BRFM_CRC_BIST: RFM_SYSCLK.RFMPHY_STATUS read ERROR, retry_count = %d", i));

                ulVal = BREG_Read32( hRegister, BCHP_RFM_SYSCLK_RFM_PHASEITP_CTL );
                ulVal &= ~BCHP_MASK( RFM_SYSCLK_RFM_PHASEITP_CTL, PHASEITP_RESET_VAL );
                ulVal |= BCHP_FIELD_DATA( RFM_SYSCLK_RFM_PHASEITP_CTL, PHASEITP_RESET_VAL, 0x1 );
                BREG_Write32( hRegister, BCHP_RFM_SYSCLK_RFM_PHASEITP_CTL, ulVal );
                BKNI_Sleep(10);

                ulVal &= ~BCHP_MASK( RFM_SYSCLK_RFM_PHASEITP_CTL, PHASEITP_RESET_VAL );
                ulVal |= BCHP_FIELD_DATA( RFM_SYSCLK_RFM_PHASEITP_CTL, PHASEITP_RESET_VAL, 0x0 );
                BREG_Write32( hRegister, BCHP_RFM_SYSCLK_RFM_PHASEITP_CTL, ulVal );

                BREG_Write32( hRegister, BCHP_RFM_SYSCLK_RESET, 0x0000000f );
                BREG_Write32( hRegister, BCHP_RFM_SYSCLK_RESET, 0x00000000 );
            }
        }

        if (i>=MAX_RETRY) {
            BDBG_ERR(("RFM CRC built-in self-test has failed"));
            retCode = BERR_TIMEOUT;
        }
    }
#if BRFM_DUAL_DAC
    else { /* dual DAC case */
        uint32_t ulVal2 = 0;
        for (i=0; i<MAX_RETRY; i++) {
            BREG_Write32( hRegister, BCHP_RFM_SYSCLK_RFMPHY_BISTCNTL, 0x010103ff ); /* set BIST_EN and BIST_EN_2 */
            BKNI_Sleep(10);
            ulVal = BREG_Read32( hRegister, BCHP_RFM_SYSCLK_RFMPHY_STATUS );
            ulVal2 = BREG_Read32( hRegister, BCHP_RFM_SYSCLK_RFMPHY_DAC2_CRC );
            BREG_Write32( hRegister, BCHP_RFM_SYSCLK_RFMPHY_BISTCNTL, 0x000003ff ); /* reset BIST_EN and BIST_EN_2 */
            if ((ulVal == 0x8000fcd9) && (ulVal2 == 0x0000fcd9)) {
                BDBG_MSG(("BRFM_CRC_BIST: RFM_SYSCLK.RFMPHY_STATUS(dual DAC) read PASS,  retry_count = %d", i));
                break;
            }
            else {
                BDBG_MSG(("BRFM_CRC_BIST: RFM_SYSCLK.RFMPHY_STATUS(dual DAC) read ERROR, retry_count = %d", i));

                ulVal = BREG_Read32( hRegister, BCHP_RFM_SYSCLK_RFM_PHASEITP_CTL );
                ulVal &= ~BCHP_MASK( RFM_SYSCLK_RFM_PHASEITP_CTL, PHASEITP_RESET_VAL );
                ulVal |= BCHP_FIELD_DATA( RFM_SYSCLK_RFM_PHASEITP_CTL, PHASEITP_RESET_VAL, 0x1 );
                BREG_Write32( hRegister, BCHP_RFM_SYSCLK_RFM_PHASEITP_CTL, ulVal );
                BKNI_Sleep(10);

                ulVal &= ~BCHP_MASK( RFM_SYSCLK_RFM_PHASEITP_CTL, PHASEITP_RESET_VAL );
                ulVal |= BCHP_FIELD_DATA( RFM_SYSCLK_RFM_PHASEITP_CTL, PHASEITP_RESET_VAL, 0x0 );
                BREG_Write32( hRegister, BCHP_RFM_SYSCLK_RFM_PHASEITP_CTL, ulVal );

                BREG_Write32( hRegister, BCHP_RFM_SYSCLK_RESET, 0x0000000f );
                BREG_Write32( hRegister, BCHP_RFM_BBE_SYSCLK_RESET, 0x00000007 );
                BREG_Write32( hRegister, BCHP_RFM_SYSCLK_RESET, 0x00000000 );
                BREG_Write32( hRegister, BCHP_RFM_BBE_SYSCLK_RESET, 0x00000000 );
            }

            if (i>=MAX_RETRY) {
                BDBG_ERR(("RFM CRC built-in self-test(dual DAC) has failed"));
                retCode = BERR_TIMEOUT;
            }
        }
    }
#endif /* BRFM_DUAL_DAC */
    return retCode;

#else /* (BRFM_REVID==40) */
    /* CRC BIST is unnecessary */
    BSTD_UNUSED(hRegister);
    BSTD_UNUSED(enc);
    return BERR_SUCCESS;
#endif
}

/*******************************************************************************
*
* Public functions
*
*******************************************************************************/
BERR_Code BRFM_Open(BRFM_Handle *phRfm, BCHP_Handle hChip, BREG_Handle hRegister,   BINT_Handle hInterrupt, const BRFM_Settings *pDefSettings)
{
    BERR_Code retCode = BERR_SUCCESS;
    BRFM_Handle hDev;
    unsigned idx;
    bool capable = true;

    BDBG_ENTER(BRFM_Open);
    /* Sanity check on the handles we've been given */
    BDBG_ASSERT(hChip);
    BDBG_ASSERT(hRegister);
    BDBG_ASSERT(hInterrupt);

    /* Alloc memory from the system heap */
    hDev = (BRFM_Handle) BKNI_Malloc(sizeof(BRFM_P_Handle));
    if (hDev == NULL) {
        retCode = BERR_OUT_OF_SYSTEM_MEMORY;
        BDBG_ERR(("BRFM_Open: BKNI_malloc() failed"));
        goto done;
    }
    BKNI_Memset(hDev, 0x00, sizeof(BRFM_P_Handle));

#if BCHP_CHIP==7552 || BCHP_CHIP==7360 /* do it only for OTP-disableable chips, to silence the BERR_TRACE on other platforms */
    retCode = BCHP_GetFeature(hChip, BCHP_Feature_eRfmCapable, &capable);
    if (retCode==BERR_SUCCESS && capable==false) {
        capable = false;
        BDBG_WRN(("BRFM_Open: OTP disabled"));
    }
    else {
        capable = true;
    }
#endif

    hDev->magicId = DEV_MAGIC_ID;
    hDev->hChip = hChip;
    hDev->hRegister = hRegister;
    hDev->hInterrupt = hInterrupt;
    hDev->coreOffset = BCHP_RFM_SYSCLK_CLKCTL;
    hDev->settings.volume = pDefSettings->volume;
    hDev->settings.audioEncoding = pDefSettings->audioEncoding;
    hDev->settings.modType = pDefSettings->modType;
    hDev->settings.encType = pDefSettings->encType;
    hDev->settings.outputChannel = pDefSettings->chNbr;
    hDev->settings.isMuteEnable = false;
    hDev->settings.isOutputEnable = false;
    hDev->settings.isPowerSaverEnable = false;
    hDev->otpDisabled = !capable;

    if (hDev->otpDisabled) {
        goto done;
    }
    if (pDefSettings->encType == BRFM_EncoderType_singleBTSC) {
        BDBG_MSG(("BRFM_Open:> single BTSC"));
        hDev->settings.pModInfo = BRFM_P_GetModInfoPtr(hDev->settings.modType);
    }
    else if (pDefSettings->encType == BRFM_EncoderType_dualBTSC) {
#if BRFM_DUAL_DAC
        BDBG_MSG(("BRFM_Open:> dual BTSC"));
        hDev->settings.pModInfo = BRFM_P_GetModInfoPtr_BBE(hDev->settings.modType);
#else
        BDBG_ERR(("BRFM_Open: dual BTSC is not supported on this platform"));
        retCode = BERR_INVALID_PARAMETER;
        goto done;
#endif
    }
    else {
        BDBG_ERR(("BRFM_Open: Invalid encoder type specified"));
        retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto done;
    }

    /* Power up in case we were left in a powered down state.
       It is not sufficient to just turn on the top-level clock; the CRC BIST will fail */
    hDev->settings.isPowerSaverEnable = true; /* force a power-up */
    BRFM_P_SetPowerSaverMode(hDev, false);

#if (BCHP_CHIP != 7543)
    /* Only enable interrupts after sysclk reset is done; this is already done in PowerUp */
    for (idx=0; idx<MAKE_INT_ENUM(Last); idx++) {
        CHK_RETCODE(retCode, BINT_CreateCallback(
            &(hDev->hCallback[idx]), hDev->hInterrupt, IntId[idx],
            BRFM_HandleInterrupt_isr, (void *) hDev, idx));
        CHK_RETCODE( retCode, BINT_EnableCallback(hDev->hCallback[idx]));
    }
#endif
    hDev->isCallbackDisable = 0;

    BRFM_P_CRC_BIST(hDev->hRegister, pDefSettings->encType);
#if (BCHP_CHIP == 7445) || (BCHP_CHIP == 7145) || (BCHP_CHIP==7439) || (BCHP_CHIP==74371) || (BCHP_CHIP==7364)
    BREG_Write32( hDev->hRegister, BCHP_RFM_SYSCLK_DPLL_MISC1, 0x0a00a763 );
    BREG_Write32( hDev->hRegister, BCHP_RFM_SYSCLK_CLKCTL, 0x0100000e );
    BKNI_Sleep(1);
    BREG_Write32( hDev->hRegister, BCHP_RFM_SYSCLK_GLB_PWRUP, 0x00000001 );
    BKNI_Sleep(1);
    BREG_Write32( hDev->hRegister, BCHP_RFM_SYSCLK_DPLL_MISC1, 0x0a00a760 );
    BKNI_Sleep(1);
#endif

done:
    *phRfm = hDev;
    if (retCode != BERR_SUCCESS) {
        /* if we get here, we never powered on within BRFM_Open().
           therefore, we leave it in whatever power state it was in */
        if (hDev != NULL) {
            for (idx=0; idx<MAKE_INT_ENUM(Last); idx++) {
                if(hDev->hCallback[idx] != NULL) {
                    BINT_DisableCallback(hDev->hCallback[idx]);
                    BINT_DestroyCallback(hDev->hCallback[idx]);
                }
            }
            BKNI_Free(hDev);
            *phRfm = NULL;
        }
    }
    else if (capable) {
        BRFM_P_SetPowerSaverMode(hDev, true);
    }

    BDBG_MSG(("BRFM_Open:<"));
    BDBG_LEAVE(BRFM_Open);
    return retCode;
}

BERR_Code BRFM_Close(BRFM_Handle hDev)
{
    BERR_Code retCode = BERR_SUCCESS;
    unsigned idx;

    BDBG_ENTER(BRFM_Close);
    BDBG_ASSERT(hDev);
    BDBG_ASSERT(hDev->magicId == DEV_MAGIC_ID);
    if (hDev->otpDisabled) {
        goto free;
    }

    BRFM_P_SetPowerSaverMode(hDev, false);
    /* check top-level clock */
    for (idx=0; idx<MAKE_INT_ENUM(Last); idx++) {
        if (hDev->hCallback[idx]) {
            CHK_RETCODE(retCode, BINT_DisableCallback(hDev->hCallback[idx]));
            CHK_RETCODE(retCode, BINT_DestroyCallback(hDev->hCallback[idx]));
            hDev->hCallback[idx] = NULL;
        }
    }

    /* always power down on close */
    BRFM_P_SetPowerSaverMode(hDev, true);
free:
    hDev->magicId = 0x00; /* clear it to catch improper use */
    BKNI_Free((void *) hDev);

done:
    BDBG_LEAVE(BRFM_Close);
    return retCode;
}

BERR_Code BRFM_GetDefaultSettings(BRFM_Settings *pDefSettings, BCHP_Handle hChip    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BSTD_UNUSED(hChip);
    BDBG_ENTER(BRFM_GetDefaultSettings);

    *pDefSettings = defDevSettings;
    BDBG_LEAVE(BRFM_GetDefaultSettings);
    return retCode;
}

BERR_Code BRFM_GetConfigSettings(BRFM_Handle hDev, BRFM_ConfigSettings *pConfigSettings )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BRFM_GetConfigSettings);
    *pConfigSettings = hDev->configSettings;
    BDBG_LEAVE(BRFM_GetConfigSettings);
    return retCode;
}

BERR_Code BRFM_SetConfigSettings(BRFM_Handle hDev, const BRFM_ConfigSettings *pConfigSettings )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t ulVal;

    BDBG_ENTER(BRFM_SetConfigSettings);

    if ((hDev->settings.isPowerSaverEnable==false) && (hDev->settings.modType == BRFM_ModulationType_eNtscOpenCable)) {
            ulVal = BREG_Read32(hDev->hRegister, BCHP_RFM_CLK27_FILSCL);
            switch (pConfigSettings->audioVideoRatio) {
                case BRFM_AUDIO_VIDEO_RATIO_17:
                    ulVal &= ~(BCHP_MASK(RFM_CLK27_FILSCL, AUDSCL));
                    ulVal |= BCHP_FIELD_DATA(RFM_CLK27_FILSCL, AUDSCL, 0x300);
                    break;
                case BRFM_AUDIO_VIDEO_RATIO_16:
                    ulVal &= ~(BCHP_MASK(RFM_CLK27_FILSCL, AUDSCL));
                    ulVal |= BCHP_FIELD_DATA(RFM_CLK27_FILSCL, AUDSCL, 0x354);
                    break;
                case BRFM_AUDIO_VIDEO_RATIO_15:
                    ulVal &= ~(BCHP_MASK(RFM_CLK27_FILSCL, AUDSCL));
                    ulVal |= BCHP_FIELD_DATA(RFM_CLK27_FILSCL, AUDSCL, 0x3bc);
                    break;
                case BRFM_AUDIO_VIDEO_RATIO_14:
                    ulVal &= ~(BCHP_MASK(RFM_CLK27_FILSCL, AUDSCL));
                    ulVal |= BCHP_FIELD_DATA(RFM_CLK27_FILSCL, AUDSCL, 0x430);
                    break;
                default:
                    BDBG_WRN(("Invalid Audio Video Ratio Selected, defaulting to -17 dB"));
                    ulVal &= ~(BCHP_MASK(RFM_CLK27_FILSCL, AUDSCL));
                    ulVal |= BCHP_FIELD_DATA(RFM_CLK27_FILSCL, AUDSCL, 0x300);
                    break;
            }
            BREG_Write32(hDev->hRegister, BCHP_RFM_CLK27_FILSCL, ulVal);
            hDev->configSettings = *pConfigSettings;
    }

    BDBG_LEAVE(BRFM_SetConfigSettings);
    return retCode;
}

BERR_Code BRFM_EnableRfOutput(BRFM_Handle hDev)
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BRFM_EnableRfOutput);
    BDBG_ASSERT(hDev);
    BDBG_ASSERT(hDev->magicId == DEV_MAGIC_ID);

    if (hDev->otpDisabled) {
        goto done;
    }

    BRFM_P_SetPowerSaverMode(hDev, false);

    if (hDev->settings.isOutputEnable == false) {
        BDBG_MSG(("BRFM_EnableRfOutput: Enable RF output on hDev(%d)", hDev->settings.encType));
        CHK_RETCODE (retCode, BRFM_SetModulationType(hDev,
            hDev->settings.modType, hDev->settings.outputChannel));
    }

done:
    BDBG_LEAVE(BRFM_EnableRfOutput);
    return retCode;
}

BERR_Code BRFM_DisableRfOutput(BRFM_Handle hDev)
{
    uint32_t ulVal;
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BRFM_DisableRfOutput);
    BDBG_ASSERT(hDev);
    BDBG_ASSERT(hDev->magicId == DEV_MAGIC_ID);
    if (hDev->otpDisabled) {
        goto done;
    }

    if (hDev->settings.isPowerSaverEnable==true) {
        BDBG_MSG(("BRFM_DisableRfOutput: RFM is in power saving mode. Command ignored"));
        goto done; /* prevent register R/W while clock is turned off */
    }

    if (hDev->settings.encType == BRFM_EncoderType_singleBTSC) {
        BDBG_MSG(("BRFM_DisableRfOutput: Turning off power to 1st DAC"));
#if (BRFM_REVID<40)
        ulVal = BREG_Read32(hDev->hRegister, BCHP_RFM_SYSCLK_DACCTL);
        ulVal |= BCHP_FIELD_DATA(RFM_SYSCLK_DACCTL, DIS_CLK, 1);
        BREG_Write32(hDev->hRegister, BCHP_RFM_SYSCLK_DACCTL, ulVal);
#else
        ulVal = BREG_Read32(hDev->hRegister, BCHP_RFM_SYSCLK_DACCTL);
        ulVal |= BCHP_FIELD_DATA(RFM_SYSCLK_DACCTL, DAC_PWRDN, 1); /* turn off power to 1st DAC */
        BREG_Write32(hDev->hRegister, BCHP_RFM_SYSCLK_DACCTL, ulVal);
#endif
    }
    else if (hDev->settings.encType == BRFM_EncoderType_dualBTSC) {
#if BRFM_DUAL_DAC
        BDBG_WRN(("Disabling RF output on second BTSC encoder. Reverting to single BTSC mode."));
        ulVal = BREG_Read32(hDev->hRegister, BCHP_RFM_SYSCLK_DACCTL);
        ulVal |= BCHP_FIELD_DATA(RFM_SYSCLK_DACCTL, DAC_SEL, 0); /* set DAC output mode to differential (ie. single) */
        ulVal |= BCHP_FIELD_DATA(RFM_SYSCLK_DACCTL, DAC_PWRDN_2, 1); /* turn off power to 2nd DAC */
        BREG_Write32(hDev->hRegister, BCHP_RFM_SYSCLK_DACCTL, ulVal);
#endif
    }

    hDev->settings.isOutputEnable = false;
done:
    BDBG_LEAVE(BRFM_DisableRfOutput);
    return retCode;
}

BERR_Code BRFM_SetAudioVolume(BRFM_Handle hDev, int volume)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t ulVal = 0;
    uint16_t uLeftVal = 0;
    uint16_t uRightVal = 0;

    BDBG_ENTER(BRFM_SetAudioVolume);
    BDBG_ASSERT(hDev);
    BDBG_ASSERT(hDev->magicId == DEV_MAGIC_ID);
    if (hDev->otpDisabled) {
        goto done;
    }

    /* check for invalid request */
    if (hDev->settings.encType == BRFM_EncoderType_dualBTSC)
    {
#if (!BRFM_DUAL_DAC)
        BDBG_ERR(("BRFM_SetAudioVolume: Second BTSC encoder is not supported on this platform!"));
        goto done;
#endif
    }

    BDBG_MSG(("BRFM_SetAudioVolume: volume %d", volume));
    if (volume > BRFM_MAX_VOLUME || volume < BRFM_MIN_VOLUME)
    {
        retCode = BERR_TRACE(BRFM_ERR_VOLUME_RANGE);
        goto done;
    }
    else
    {
        if (volume > 12) /* PR40962 */
        {
            BDBG_WRN(("Poor audio quality may result if setting volume above +12dB"));
            BDBG_WRN(("It is recommended that you set the RFM core volume level below +12dB"));
        }
        switch (hDev->settings.audioEncoding)
        {
            case BRFM_AudioEncoding_eMono:
            case BRFM_AudioEncoding_eStereo:
            case BRFM_AudioEncoding_eStereoExtDeviation:
            case BRFM_AudioEncoding_eJpMono2:
            case BRFM_AudioEncoding_eSap:
            case BRFM_AudioEncoding_eJpMono1:
            case BRFM_AudioEncoding_eSapMono:
                BRFM_P_SetPowerSaverMode(hDev, false);
                BRFM_P_CalculateVolume(volume, &uLeftVal, &uRightVal);
                if (hDev->settings.encType == BRFM_EncoderType_singleBTSC)
                {
                    ulVal = BREG_Read32(hDev->hRegister, BCHP_RFM_CLK27_VOLUME);
                    ulVal &= ~BCHP_MASK(RFM_CLK27_VOLUME, LVOL);
                    ulVal &= ~BCHP_MASK(RFM_CLK27_VOLUME, RVOL);
                }
                else if (hDev->settings.encType == BRFM_EncoderType_dualBTSC)
                {
#if BRFM_DUAL_DAC
                    ulVal = BREG_Read32(hDev->hRegister, BCHP_RFM_BBE_CLK27_VOLUME);
                    ulVal &= ~BCHP_MASK(RFM_BBE_CLK27_VOLUME, LVOL);
                    ulVal &= ~BCHP_MASK(RFM_BBE_CLK27_VOLUME, RVOL);
#endif
                }
                switch (hDev->settings.audioEncoding)
                {
                    case BRFM_AudioEncoding_eMono:
                    case BRFM_AudioEncoding_eStereo:
                    case BRFM_AudioEncoding_eStereoExtDeviation:
                    case BRFM_AudioEncoding_eJpMono2:
                        if (hDev->settings.encType == BRFM_EncoderType_singleBTSC)
                        {
                            ulVal |= BCHP_FIELD_DATA(RFM_CLK27_VOLUME, LVOL, (uLeftVal >> 1));
                            ulVal |= BCHP_FIELD_DATA(RFM_CLK27_VOLUME, RVOL, (uRightVal >> 1));
                        }
                        else if (hDev->settings.encType == BRFM_EncoderType_dualBTSC)
                        {
#if BRFM_DUAL_DAC
                            ulVal |= BCHP_FIELD_DATA(RFM_BBE_CLK27_VOLUME, LVOL, (uLeftVal >> 1));
                            ulVal |= BCHP_FIELD_DATA(RFM_BBE_CLK27_VOLUME, RVOL, (uRightVal >> 1));
#endif
                        }
                        break;
                    case BRFM_AudioEncoding_eSap:
                        if (hDev->settings.encType == BRFM_EncoderType_singleBTSC)
                        {
                            ulVal |= BCHP_FIELD_DATA(RFM_CLK27_VOLUME, RVOL, uLeftVal);
                        }
                        else if (hDev->settings.encType == BRFM_EncoderType_dualBTSC)
                        {
#if BRFM_DUAL_DAC
                            ulVal |= BCHP_FIELD_DATA(RFM_BBE_CLK27_VOLUME, RVOL, uLeftVal);
#endif
                        }
                        break;
                    case BRFM_AudioEncoding_eJpMono1:
                        if (hDev->settings.encType == BRFM_EncoderType_singleBTSC)
                        {
                            ulVal |= BCHP_FIELD_DATA(RFM_CLK27_VOLUME, LVOL, uRightVal);
                        }
                        else if (hDev->settings.encType == BRFM_EncoderType_dualBTSC)
                        {
#if BRFM_DUAL_DAC
                            ulVal |= BCHP_FIELD_DATA(RFM_BBE_CLK27_VOLUME, LVOL, uRightVal);
#endif
                        }
                        break;
                    case BRFM_AudioEncoding_eSapMono:
                        if (hDev->settings.encType == BRFM_EncoderType_singleBTSC)
                        {
                            ulVal |= BCHP_FIELD_DATA(RFM_CLK27_VOLUME, LVOL, uLeftVal);
                            ulVal |= BCHP_FIELD_DATA(RFM_CLK27_VOLUME, RVOL, uRightVal);
                        }
                        else if (hDev->settings.encType == BRFM_EncoderType_dualBTSC)
                        {
#if BRFM_DUAL_DAC
                            ulVal |= BCHP_FIELD_DATA(RFM_BBE_CLK27_VOLUME, LVOL, uLeftVal);
                            ulVal |= BCHP_FIELD_DATA(RFM_BBE_CLK27_VOLUME, RVOL, uRightVal);
#endif
                        }
                        break;
                    default:
                        break;
                    }
                    if (hDev->settings.encType == BRFM_EncoderType_singleBTSC)
                        BREG_Write32(hDev->hRegister, BCHP_RFM_CLK27_VOLUME, ulVal);
                    else if (hDev->settings.encType == BRFM_EncoderType_dualBTSC)
                    {
#if BRFM_DUAL_DAC
                        BREG_Write32(hDev->hRegister, BCHP_RFM_BBE_CLK27_VOLUME, ulVal);
#endif
                    }
                    hDev->settings.volume = volume;
                break;

            case BRFM_AudioEncoding_eNicamStereo:
            case BRFM_AudioEncoding_eNicamDualMono:
            case BRFM_AudioEncoding_eNicamMono:
#if (BRFM_REVID>=40)
                BRFM_SetNicamVolume(hDev, volume, volume);
                break;
#else
                BDBG_ERR(("BRFM_SetAudioVolume: NICAM encoder is not supported"));
                retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
#endif
            default:
                BDBG_ASSERT(false);
        }
    }

done:
    BDBG_LEAVE(BRFM_SetAudioVolume);
    return retCode;
}

BERR_Code BRFM_SetAudioMute(BRFM_Handle hDev, bool mute)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t ulVal;

    BDBG_ENTER(BRFM_SetAudioMute);
    BDBG_ASSERT(hDev);
    BDBG_ASSERT(hDev->magicId == DEV_MAGIC_ID);
    if (hDev->otpDisabled) {
        goto done;
    }

    /* BRFM_P_SetPowerSaverMode( hDev, false ); */ /* don't call this */

    if (mute==hDev->settings.isMuteEnable) {
        goto done;
    }

    BDBG_MSG(("BRFM_SetAudioMute: mute %d->%d", hDev->settings.isMuteEnable, mute));
    if (mute) {
        if (hDev->settings.isPowerSaverEnable) {
            BDBG_MSG(("BRFM_SetAudioMute: RFM is in power saving mode. Mute command ignored"));
            goto done;
        }
        ulVal = 0;
        if (hDev->settings.encType == BRFM_EncoderType_singleBTSC) {
            BREG_Write32(hDev->hRegister, BCHP_RFM_CLK27_VOLUME, ulVal);
        }
        else if (hDev->settings.encType == BRFM_EncoderType_dualBTSC) {
#if BRFM_DUAL_DAC
            BREG_Write32( hDev->hRegister, BCHP_RFM_BBE_CLK27_VOLUME, ulVal);
#endif
        }
        hDev->settings.isMuteEnable = true;
    }
    else {
        if (hDev->settings.isPowerSaverEnable) {
            BDBG_MSG(("BRFM_SetAudioMute: RFM is in power saving mode. Unmute command ignored"));
            goto done;
        }
        CHK_RETCODE(retCode, BRFM_SetAudioVolume(hDev, hDev->settings.volume));
        hDev->settings.isMuteEnable = false;
    }

done:
    BDBG_LEAVE(BRFM_SetAudioMute);
    return retCode;
}

BERR_Code BRFM_SetAudioEncoding(BRFM_Handle hDev, BRFM_AudioEncoding audioEncoding)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t ulVal;
    uint16_t uEncMode=0;

    BDBG_ENTER(BRFM_SetAudioEncoding);
    BDBG_ASSERT(hDev);
    BDBG_ASSERT(hDev->magicId == DEV_MAGIC_ID);
    if (hDev->otpDisabled) {
        goto done;
    }

    BDBG_MSG(("BRFM_SetAudioEncoding: encType %d, audioEncoding %d", hDev->settings.encType, audioEncoding));
    switch (audioEncoding)
    {
        case BRFM_AudioEncoding_eStereo:
        case BRFM_AudioEncoding_eStereoExtDeviation:
        case BRFM_AudioEncoding_eSap:
        case BRFM_AudioEncoding_eSapMono:
        case BRFM_AudioEncoding_eJpMono1:
        case BRFM_AudioEncoding_eJpMono2:
            /* Check for valid mode, don't generate error, just a warning will do,
               at the every least, mono audio output will appear on the output */
            switch (hDev->settings.modType)
            {
                case BRFM_ModulationType_eNtscCustom:
                case BRFM_ModulationType_eNtscOpenCable:
                    break;
                default:
                    /* assuming modulation is either PAL or Secam */
                    BDBG_WRN(("BRFM_SetAudioEncoding: only Mono Audio Encoding for PAL/Secam"));
                    audioEncoding = BRFM_AudioEncoding_eMono;
                    break;
            }
            /* Fall into next statement, not a typo */
        case BRFM_AudioEncoding_eMono:
            BRFM_P_SetPowerSaverMode(hDev, false);
            CHK_RETCODE(retCode, BRSP_ParseRegScript(hDev->hRegister,
                hDev->settings.pModInfo->setupScrAudioEncoding[audioEncoding], hDev));
            hDev->settings.audioEncoding = audioEncoding;
            if (hDev->settings.outputChannel == BRFM_OutputChannel_eBaseband)
            {
                /* Bypass the FM */
                /* Note that BYP_FM is 0x01 only for baseband mode and 0x00 for all other modes. The audio setup scripts, which are called
                right above, reset BYP_FM to 0x00, so there's no need to reset it here for non-baseband modes. */
                if (hDev->settings.encType == BRFM_EncoderType_singleBTSC)
                {
                    BDBG_MSG(("BRFM_SetAudioEncoding: Bypass first BTSC encoder FM"));
                    ulVal = BREG_Read32(hDev->hRegister, BCHP_RFM_CLK27_AUDIOBYP);
                    ulVal |= BCHP_FIELD_DATA(RFM_CLK27_AUDIOBYP, BYP_FM, 0x01);
                    BREG_Write32(hDev->hRegister, BCHP_RFM_CLK27_AUDIOBYP, ulVal);
                    if (audioEncoding == BRFM_AudioEncoding_eStereo || audioEncoding == BRFM_AudioEncoding_eStereoExtDeviation)
                    {
                        ulVal = 0x00060001;
                        BREG_Write32(hDev->hRegister, BCHP_RFM_CLK27_OVRSPCB1, ulVal);
                    }
                }
                else if (hDev->settings.encType == BRFM_EncoderType_dualBTSC)
                {
#if BRFM_DUAL_DAC
                    BDBG_MSG(("BRFM_SetAudioEncoding: Bypass second BTSC encoder FM"));
                    ulVal = BREG_Read32(hDev->hRegister, BCHP_RFM_BBE_CLK27_AUDIOBYP);
                    ulVal |= BCHP_FIELD_DATA(RFM_BBE_CLK27_AUDIOBYP, BYP_FM, 0x01);
                    BREG_Write32( hDev->hRegister, BCHP_RFM_BBE_CLK27_AUDIOBYP, ulVal);
                    if (audioEncoding == BRFM_AudioEncoding_eStereo || audioEncoding == BRFM_AudioEncoding_eStereoExtDeviation)
                    {
                        ulVal = 0x00060001;
                        BREG_Write32(hDev->hRegister, BCHP_RFM_BBE_CLK27_OVRSPCB1, ulVal);
                    }
#else
                    BDBG_ERR(("BRFM_SetAudioEncoding: Second BTSC encoder is not supported on this platform!"));
                    retCode = BERR_INVALID_PARAMETER;
                    goto done;
#endif
                }
            }
            CHK_RETCODE(retCode, BRFM_SetAudioMute(hDev, hDev->settings.isMuteEnable));

            CHK_RETCODE(retCode, BRFM_SetAudioStandard(hDev, audioEncoding));

            goto done;
            break;

        /* NICAM encoding */
        case BRFM_AudioEncoding_eNicamStereo:
            uEncMode = 0;
            goto configureNicam;
            break;

        case BRFM_AudioEncoding_eNicamDualMono:
            uEncMode = 1;
            goto configureNicam;
            break;

        case BRFM_AudioEncoding_eNicamMono:
            uEncMode = 2;
            goto configureNicam;
            break;

        default:
            BDBG_ERR(("BRFM_SetAudioEncoding: Invalid audio encoding specified!"));
            retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            goto done;
            break;
    }

configureNicam:

#if (BRFM_REVID>=40)
        BDBG_MSG(("BRFM_SetAudioEncoding: NICAM setup, hDev(%d), audioEncoding(%d)", hDev->settings.encType, audioEncoding));
        if (hDev->settings.encType == BRFM_EncoderType_dualBTSC)
        {
            BDBG_ERR(("BRFM_SetAudioEncoding: NICAM audio mode is not supported on second BTSC encoder"));
            retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            goto done;  /* exit without calling anything */
        }

        switch (hDev->settings.modType)
        {
            case BRFM_ModulationType_eNtscCustom:
            case BRFM_ModulationType_eNtscOpenCable:
                /* NICAM audio encoding only support in PAL/SECAM modes */
                BDBG_ERR(("BRFM_SetAudioEncoding: NICAM audio mode is not supported in NTSC format"));
                retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
                goto done;  /* exit without calling anything */
                break;

            case BRFM_ModulationType_ePalBG:
            case BRFM_ModulationType_ePalH:
                retCode = BRFM_SetNicamCarrierLevel(hDev, BRFM_NicamCarrierAmpl_eNeg20p0);
                break;

            case BRFM_ModulationType_ePalD:
            case BRFM_ModulationType_ePalDChina:
                retCode = BRFM_SetNicamCarrierLevel(hDev, BRFM_NicamCarrierAmpl_eNeg25p0);
                break;

            case BRFM_ModulationType_ePalI:
                retCode = BRFM_SetNicamCarrierLevel(hDev, BRFM_NicamCarrierAmpl_eNeg20p0);
                break;
            default:
                break;
        }
        if (retCode)
        {
            goto done;
        }

        BRFM_P_SetPowerSaverMode(hDev, false);
        /* NICAM audio encoding setup scripts should be called only after validating (above) that NICAM is a valid setup */
        CHK_RETCODE(retCode, BRSP_ParseRegScript( hDev->hRegister,
            hDev->settings.pModInfo->setupScrAudioEncoding[audioEncoding], hDev));
        hDev->settings.audioEncoding = audioEncoding;

        /* Do not bypass the FM */
        ulVal = BREG_Read32(hDev->hRegister, BCHP_RFM_CLK27_AUDIOBYP);
        ulVal |= BCHP_FIELD_DATA(RFM_CLK27_AUDIOBYP, BYP_FM, 0x00);
        BREG_Write32(hDev->hRegister, BCHP_RFM_CLK27_AUDIOBYP, ulVal);

        CHK_RETCODE(retCode, BRFM_SetAudioMute(hDev, hDev->settings.isMuteEnable));

        CHK_RETCODE(retCode, BRFM_SetAudioStandard(hDev, audioEncoding));

        /* Set NICAM Encoder Mode (stereo/dual mono/mono) */
        ulVal = BREG_Read32(hDev->hRegister,BCHP_RFM_CLK27_NICAM_CTL);
        ulVal &= ~(BCHP_MASK(RFM_CLK27_NICAM_CTL, ENC_MODE));
        ulVal |= BCHP_FIELD_DATA(RFM_CLK27_NICAM_CTL, ENC_MODE, uEncMode);
        BREG_Write32(hDev->hRegister, BCHP_RFM_CLK27_NICAM_CTL,ulVal);

#else
        BDBG_ERR(("BRFM_SetAudioEncoding: NICAM audio encoding mode is not supported"));
        retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
#endif
        /* TODO: RFM_SYSCLK_RESET.AUDIO_SFT_RST may need to be asserted and deasserted on an audio config change,
           similar to a RFM_SYSCLK_RESET that occurs at the end of BRFM_SetModulationType */

done:
    BDBG_LEAVE(BRFM_SetAudioEncoding);
    return( retCode );
}

BERR_Code BRFM_SetModulationType(BRFM_Handle hDev, BRFM_ModulationType modType, BRFM_OutputChannel chNbr)
{
    BERR_Code retCode = BERR_SUCCESS;
    const BRFM_P_ModulationInfo *pModInfo;
    /* if we change the rf modulation, then do a reset */
#if (BRFM_REVID>=50)
    bool reset = !hDev->settings.isOutputEnable || modType!=hDev->settings.modType || chNbr!=hDev->settings.outputChannel;
#endif
    BDBG_ENTER(BRFM_SetModulationType);
    BDBG_ASSERT(hDev);
    BDBG_ASSERT(hDev->magicId == DEV_MAGIC_ID);
    if (hDev->otpDisabled) {
        goto done;
    }

    BDBG_MSG(("BRFM_SetModulationType:> encType %d, modType %d, ch %d", hDev->settings.encType, modType, chNbr));
    if (hDev->settings.encType == BRFM_EncoderType_singleBTSC) {
        pModInfo = BRFM_P_GetModInfoPtr(modType);
    }
    else if (hDev->settings.encType == BRFM_EncoderType_dualBTSC) {
#if BRFM_DUAL_DAC
        pModInfo = BRFM_P_GetModInfoPtr_BBE(modType);
#else
        BDBG_ERR(("BRFM_SetModulationType: dual BTSC is not supported on this platform"));
        retCode = BERR_INVALID_PARAMETER;
        goto done;
#endif
    }
    else {
        BDBG_ERR(("BRFM_SetModulationType: Invalid encoder type specified"));
        retCode = BERR_INVALID_PARAMETER;
        goto done;
    }

    if (pModInfo != NULL) {
        switch (chNbr) {
            case BRFM_OutputChannel_eCh3:
            case BRFM_OutputChannel_eCh4:
                if (hDev->settings.encType == BRFM_EncoderType_dualBTSC) {
                    BDBG_WRN(("BRFM_SetModulationType: On second BTSC encoder, only NTSC baseband or SIF mode is supported."));
                    BDBG_WRN(("BRFM_SetModulationType: Defaulting second BTSC encoder to SIF mode!"));
                }   /* fall through */
            case BRFM_OutputChannel_eBaseband:
                if (hDev->settings.outputChannel == BRFM_OutputChannel_eBaseband &&
                    modType != BRFM_ModulationType_eNtscOpenCable) {
                    BDBG_WRN(("BRFM_SetModulationType: Baseband mode is only supported with NTSC modulation."));
                }   /* fall through */
            case BRFM_OutputChannel_eSIF:
                /* Re-enable any interrupts that was disable to see if the error condition continues */
                BRFM_P_SetPowerSaverMode( hDev, false );
                CHK_RETCODE(retCode, BRFM_EnableDisabledInterrupt(hDev));
                CHK_RETCODE(retCode, BRSP_ParseRegScript(hDev->hRegister, pModInfo->setupScrConfig, hDev));
                CHK_RETCODE(retCode, BRSP_ParseRegScript(hDev->hRegister, pModInfo->setupScrCh[chNbr], hDev));
                hDev->settings.modType = modType;
                hDev->settings.pModInfo = pModInfo;
                hDev->settings.outputChannel = chNbr;
                CHK_RETCODE(retCode, BRFM_SetAudioEncoding(hDev, hDev->settings.audioEncoding));
                hDev->settings.isOutputEnable = true;
                BDBG_MSG(("BRFM_SetModulationType:< rfm enabled")); /* regardless of previous state, RFM is enabled at this point */
                break;
            default:
                retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
                break;
        }
    }
    else {
        retCode = BERR_INVALID_PARAMETER;
    }

#if (BRFM_REVID>=50)
    /* assert and deassert a reset after a rf modulation change */
    if (reset) {
        BRFM_P_SysclkReset(hDev);
    }
#endif

done:
    BDBG_LEAVE(BRFM_SetModulationType);
    return retCode;
}

BERR_Code BRFM_EnablePowerSaver(BRFM_Handle hDev)
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ASSERT(hDev);
    BDBG_ASSERT(hDev->magicId == DEV_MAGIC_ID);
    BDBG_MSG(("BRFM_EnablePowerSaver:>"));
    if (hDev->otpDisabled) {
        goto done;
    }

#if BRFM_DUAL_DAC
    if (hDev->settings.encType==BRFM_EncoderType_dualBTSC) {
        BDBG_WRN(("BRFM_EnablePowerSaver: Power saving mode is controlled from first BTSC encoder"));
        return retCode;
    }
#endif
    BRFM_P_SetPowerSaverMode(hDev, true);
done:
    BDBG_MSG(("BRFM_EnablePowerSaver:<"));
    return retCode;
}

BERR_Code BRFM_Standby(BRFM_Handle hDev)
{
    BERR_Code rc = BERR_SUCCESS;
    if (hDev->otpDisabled) {
        goto done;
    }
    BRFM_P_SetPowerSaverMode(hDev, true);
done:
    return rc;
}

BERR_Code BRFM_Resume(BRFM_Handle hDev)
{
    BERR_Code rc = BERR_SUCCESS;
    if (hDev->otpDisabled) {
        goto done;
    }
    BRFM_P_SetPowerSaverMode(hDev, false);
done:
    return rc;
}

BERR_Code BRFM_GetStatus(BRFM_Handle hDev, BRFM_Status *status)
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BRFM_GetStatus);
    BDBG_ASSERT(hDev);
    BDBG_ASSERT(hDev->magicId == DEV_MAGIC_ID);
    if (hDev->otpDisabled) {
        goto done;
    }

    if (hDev->settings.isPowerSaverEnable) {
        BDBG_WRN(("BRFM_GetStatus: RFM is in power saving mode. Command ignored"));
        goto done;
    }

    status->outputChannel = hDev->settings.outputChannel;
    status->modType = hDev->settings.modType;
    status->audioEncoding = hDev->settings.audioEncoding;
    status->isMuteEnable = hDev->settings.isMuteEnable;
    status->isOutputEnable = hDev->settings.isOutputEnable;
    status->isPowerSaverEnable = hDev->settings.isPowerSaverEnable;
    status->volume = hDev->settings.volume;
    status->rfmStatusSignals = BREG_Read32(hDev->hRegister, BCHP_RFM_L2_CPU_STATUS);

done:
    BDBG_LEAVE(BRFM_GetStatus);
    return retCode;
}

BERR_Code BRFM_SetAudioStandard(BRFM_Handle hDev, BRFM_AudioEncoding audioEncoding)
{
    BERR_Code mrc = BERR_SUCCESS;
    uint32_t ulVal;
    if (hDev->otpDisabled) {
        goto done;
    }

    /* check for invalid request */
    if (hDev->settings.encType == BRFM_EncoderType_dualBTSC)
    {
#if (!BRFM_DUAL_DAC)
        BDBG_ERR(("BRFM_SetAudioStandard: Second BTSC encoder is not supported on this platform!"));
        goto done;
#endif
    }

#if (BRFM_REVID>=40)
    switch (audioEncoding)
    {
        case BRFM_AudioEncoding_eStereo:
        case BRFM_AudioEncoding_eStereoExtDeviation:
        case BRFM_AudioEncoding_eSap:
        case BRFM_AudioEncoding_eSapMono:
        case BRFM_AudioEncoding_eJpMono1:
        case BRFM_AudioEncoding_eJpMono2:
        case BRFM_AudioEncoding_eMono:
            switch (hDev->settings.modType)
            {
                case BRFM_ModulationType_eNtscCustom:
                case BRFM_ModulationType_eNtscOpenCable:
                    if (hDev->settings.encType == BRFM_EncoderType_singleBTSC)
                    {
                        ulVal = BREG_Read32(hDev->hRegister, BCHP_RFM_CLK27_AUDIO_STANDARD);
                        ulVal = 0x0;
                        BREG_Write32(hDev->hRegister, BCHP_RFM_CLK27_AUDIO_STANDARD, ulVal);
                    }
                    else if (hDev->settings.encType == BRFM_EncoderType_dualBTSC)
                    {
#if BRFM_DUAL_DAC
                        ulVal = BREG_Read32(hDev->hRegister, BCHP_RFM_BBE_CLK27_AUDIO_STANDARD);
                        ulVal = 0x0;
                        BREG_Write32(hDev->hRegister, BCHP_RFM_BBE_CLK27_AUDIO_STANDARD, ulVal);
#endif
                    }
                    break;
                case BRFM_ModulationType_ePalBG:
                case BRFM_ModulationType_ePalH:
                case BRFM_ModulationType_ePalD:
                case BRFM_ModulationType_ePalDChina:
                case BRFM_ModulationType_ePalI:
                case BRFM_ModulationType_ePalM:
                case BRFM_ModulationType_ePalN:
                    if (hDev->settings.encType == BRFM_EncoderType_singleBTSC)
                    {
                        ulVal = BREG_Read32(hDev->hRegister, BCHP_RFM_CLK27_AUDIO_STANDARD);
                        ulVal = 0x00008004;
                        BREG_Write32(hDev->hRegister, BCHP_RFM_CLK27_AUDIO_STANDARD, ulVal);
                    }
                    else if (hDev->settings.encType == BRFM_EncoderType_dualBTSC)
                    {
#if BRFM_DUAL_DAC
                        ulVal = BREG_Read32(hDev->hRegister, BCHP_RFM_BBE_CLK27_AUDIO_STANDARD);
                        ulVal = 0x00008004;
                        BREG_Write32(hDev->hRegister, BCHP_RFM_BBE_CLK27_AUDIO_STANDARD, ulVal);
#endif
                    }
                    break;
                default:
                    {
                        mrc = BERR_TRACE(BERR_INVALID_PARAMETER); /* Invalid modulation type */
                        break;
                    }
            }
            break;

        case BRFM_AudioEncoding_eNicamStereo:
        case BRFM_AudioEncoding_eNicamDualMono:
        case BRFM_AudioEncoding_eNicamMono:

            switch (hDev->settings.modType)
            {
                case BRFM_ModulationType_ePalBG:
                case BRFM_ModulationType_ePalH:
                case BRFM_ModulationType_ePalD:
                case BRFM_ModulationType_ePalDChina:
                    if (hDev->settings.encType == BRFM_EncoderType_singleBTSC)
                    {
                        ulVal = BREG_Read32(hDev->hRegister, BCHP_RFM_CLK27_AUDIO_STANDARD);
                        ulVal = 0x00008001;
                        BREG_Write32(hDev->hRegister, BCHP_RFM_CLK27_AUDIO_STANDARD, ulVal);
                    }
                    else
                    {
                        BDBG_WRN(("NICAM is not supported on second BTSC encoder"));
                    }
                    break;
                case BRFM_ModulationType_ePalI:
                    if (hDev->settings.encType == BRFM_EncoderType_singleBTSC)
                    {
                        ulVal = BREG_Read32(hDev->hRegister, BCHP_RFM_CLK27_AUDIO_STANDARD);
                        ulVal = 0x00008081;
                        BREG_Write32(hDev->hRegister, BCHP_RFM_CLK27_AUDIO_STANDARD, ulVal);
                    }
                    else
                    {
                        BDBG_WRN(("NICAM is not supported on second BTSC encoder"));
                    }
                    break;
                default:
                    {
                        mrc = BERR_TRACE(BERR_INVALID_PARAMETER); /* Invalid modulation type */
                        break;
                    }
            }
            break;

        default:
            {
                mrc = BERR_TRACE(BERR_INVALID_PARAMETER); /* Invalid audio encoding */
                goto done; /* suppress warning messages for not using done */
                break;
            }
    }
#else
    BSTD_UNUSED(hDev);
    BSTD_UNUSED(audioEncoding);
    BSTD_UNUSED(ulVal);
#endif

done:
    return mrc;
}

#if (BRFM_REVID>=40)
BERR_Code BRFM_SetNicamCarrierLevel(BRFM_Handle hDev, uint32_t eNicamCarrAmpl)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t ulVal;
    if (hDev->otpDisabled) {
        goto done;
    }

    if (eNicamCarrAmpl < BRFM_NICAM_CARR_AMPL_MIN ||
        eNicamCarrAmpl > BRFM_NICAM_CARR_AMPL_MAX)
    {
        BDBG_ERR(("NICAM carrier amplitude out of range"));
        retCode = BERR_TRACE(BRFM_ERR_CARR_AMPL_RANGE);
        goto done;
    }

    ulVal = BREG_Read32(hDev->hRegister,BCHP_RFM_CLK27_NICAM_CTL);
    ulVal &= ~(BCHP_MASK(RFM_CLK27_NICAM_CTL, OUT_SCL));
    ulVal |= BCHP_FIELD_DATA(RFM_CLK27_NICAM_CTL, OUT_SCL, eNicamCarrAmpl);
    BREG_Write32(hDev->hRegister, BCHP_RFM_CLK27_NICAM_CTL,ulVal);

done:
    return retCode;
}


BERR_Code BRFM_SetNicamVolume(BRFM_Handle hDev, int volumeDbLeft, int volumeDbRight)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t ulVal;
    uint16_t uLeftVal;
    uint16_t uRightVal;
    if (hDev->otpDisabled) {
        goto done;
    }

    if (volumeDbLeft > BRFM_MAX_VOLUME_NICAM || volumeDbRight > BRFM_MAX_VOLUME_NICAM) {
        retCode = BERR_TRACE(BRFM_ERR_VOLUME_RANGE);
    }
    else {
        if (volumeDbLeft < BRFM_MIN_VOLUME_NICAM) {
            uLeftVal = 0;
        }
        else {
            BRFM_P_CalculateNicamVolume(volumeDbLeft, &uLeftVal);
        }

        if (volumeDbRight < BRFM_MIN_VOLUME_NICAM) {
            uRightVal = 0;
        }
        else {
            BRFM_P_CalculateNicamVolume(volumeDbRight, &uRightVal);
        }

        ulVal = BREG_Read32(hDev->hRegister,BCHP_RFM_CLK27_NICAM_VOLUME);
        ulVal &= ~(BCHP_MASK(RFM_CLK27_NICAM_VOLUME, LVOL));
        ulVal &= ~(BCHP_MASK(RFM_CLK27_NICAM_VOLUME, RVOL));
        ulVal |= BCHP_FIELD_DATA(RFM_CLK27_NICAM_VOLUME, LVOL, uLeftVal);
        ulVal |= BCHP_FIELD_DATA(RFM_CLK27_NICAM_VOLUME, RVOL, uRightVal);
        BREG_Write32(hDev->hRegister, BCHP_RFM_CLK27_NICAM_VOLUME,ulVal);
    }

done:
    return retCode;
}

#endif

/* End of file */

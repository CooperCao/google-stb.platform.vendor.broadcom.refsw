/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description:
 *
 **************************************************************************/
#include "nexus_base.h"
#include "nexus_display_module.h"
#include "priv/nexus_core.h"

BDBG_MODULE(nexus_display_vbi);

#if NEXUS_VBI_SUPPORT

#define BDBG_MSG_TRACE(X) /* BDBG_MSG(X) */

static void
NEXUS_Display_P_DequeVbi_isr(void* parm1, int parm2)
{
    NEXUS_DisplayHandle display = parm1;
    BERR_Code rc;
    BAVC_Polarity polarity = (BAVC_Polarity)parm2;
    BVBI_Field_Handle field;
    bool isTop, isBottom;

    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    BDBG_ASSERT(display->vbi.enc);

    /* PR 21710 - for progressive displays, pass vbilib eFrame */
    polarity = display->vbi.progressive ? BAVC_Polarity_eFrame  : (BAVC_Polarity)parm2;
    isTop = (polarity == BAVC_Polarity_eTopField) || (polarity == BAVC_Polarity_eFrame);
    isBottom = (polarity == BAVC_Polarity_eBotField) || (polarity == BAVC_Polarity_eFrame);

    rc = BVBIlib_Encode_GetOldestDatum_isr(display->vbi.enc, &field);
    if (!rc) {
        bool somethingSet =
            display->vbi.pending.wssSet ||
            display->vbi.pending.vpsSet ||
            display->vbi.pending.cgmsTopSet ||
            display->vbi.pending.cgmsBottomSet ||
            display->vbi.pending.cgmsBTopSet ||
            display->vbi.pending.cgmsBBottomSet;
        bool obtained = false;

        if (!field && somethingSet) {
            rc = BVBIlib_List_Obtain_isr(g_NEXUS_DisplayModule_State.vbilist, &field);
            if (rc) {field = NULL; rc = BERR_TRACE(rc);}
            /* if we can't get one, just let the data stay pending.
            we still need to fall through and allow VBIlib to empty its queue. */

            if (field) {
                unsigned polarityMask;

                obtained = true;

                if (polarity == BAVC_Polarity_eFrame) {
                    polarityMask = (1<<BAVC_Polarity_eTopField)|(1<<BAVC_Polarity_eBotField);
                }
                else {
                    polarityMask = 1<<polarity;
                }
                (void)BVBI_Field_SetPolarity_isr(field, polarityMask);
            }

        }
        if (field) {
            /* set non-streaming data. these types of VBI just require the last value set to be
            applied. this code bypassing the queue logic needed for cc, tt and gs. */
            if (display->vbi.pending.wssSet && isTop) {
                rc = BVBI_Field_SetWSSData_isr(field, display->vbi.pending.wssData);
                if (!rc) {
                    BDBG_MSG_TRACE(("wss %x set", display->vbi.pending.wssData));
                    display->vbi.pending.wssSet = false;
                }
            }
            if (display->vbi.pending.vpsSet && isTop) {
                rc = BVBI_Field_SetVPSData_isr(field, &display->vbi.pending.vpsData);
                if (!rc) {
                    BDBG_MSG_TRACE(("vps set"));
                    display->vbi.pending.vpsSet = false;
                }
            }
            if (display->vbi.pending.cgmsTopSet && isTop) {
                rc = BVBI_Field_SetCGMSData_isr(field, display->vbi.pending.cgmsData);
                if (!rc) {
                    BDBG_MSG_TRACE(("cgms top %x set", display->vbi.pending.cgmsData));
                    display->vbi.pending.cgmsTopSet = false;
                }
            }
            if (display->vbi.pending.cgmsBottomSet && isBottom) {
                rc = BVBI_Field_SetCGMSData_isr(field, display->vbi.pending.cgmsData);
                if (!rc) {
                    BDBG_MSG_TRACE(("cgms bottom %x set", display->vbi.pending.cgmsData));
                    display->vbi.pending.cgmsBottomSet = false;
                }
            }
            if (display->vbi.pending.cgmsBTopSet && isTop) {
                rc = BVBI_Field_SetCGMSBData_isr(field, (BVBI_CGMSB_Datum*)display->vbi.pending.cgmsBData);
                if (!rc) {
                    BDBG_MSG_TRACE(("cgmsB top set"));
                    display->vbi.pending.cgmsBTopSet = false;
                }
            }
            if (display->vbi.pending.cgmsBBottomSet && isBottom) {
                rc = BVBI_Field_SetCGMSBData_isr(field, (BVBI_CGMSB_Datum*)display->vbi.pending.cgmsBData);
                if (!rc) {
                    BDBG_MSG_TRACE(("cgmsB bottom set"));
                    display->vbi.pending.cgmsBBottomSet = false;
                }
            }
        }
        if (obtained) {
            rc = BVBIlib_Encode_Enqueue_isr(display->vbi.enc, field);
            if (rc) {
                BVBIlib_List_Return_isr(g_NEXUS_DisplayModule_State.vbilist, field);
            }
        }
    }

    rc = BVBIlib_Encode_Data_isr(display->vbi.enc, polarity);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);}

    /* The CRC register in the HDMI output block is currently updated on the
     * rising edge of the vsync signal. This is not a convenient time for
     * software to read the value in a reliable manner.  The work-around is
     * to use VBI line (adjustible) interrupt for reading.  Later chips with
     * BAVC_HDMI_CRC_READING_FIX = 1 can be read using regular display vsync. */
#if((NEXUS_NUM_HDMI_OUTPUTS) && (!BAVC_HDMI_CRC_READING_FIX))
    if (display->hdmi.vsync_isr) {
        /* general purpose per-vsync isr. one use is CRC capture. */
        (display->hdmi.vsync_isr)(display->hdmi.pCbParam);
    }
#endif
}

BERR_Code
NEXUS_Display_P_ConnectVbi(NEXUS_DisplayHandle display)
{
    BAVC_VbiPath vbi_path;
    BERR_Code rc;
    BERR_Code cleanup_rc; /* preserve original error code */
    BINT_Id tf_isr, bf_isr;
    const NEXUS_DisplayModule_State *video = &g_NEXUS_DisplayModule_State;

    BDBG_MSG(("connect display%d to vbi", display->index));

    rc = BVDC_Display_GetVbiPath(display->displayVdc, &vbi_path);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_vbi_path;}

    if (vbi_path == BAVC_VbiPath_eUnknown) {
        /* if this display doesn't support VBI, it is not a failure. return success. */
        /* make sure this is null so that NEXUS_Display_P_DisconnectVbi will short circuit as well */
        BDBG_ASSERT(!display->vbi.enc_core);
        return 0;
    }

    rc = BVBI_Encode_GetInterruptName(vbi_path, BAVC_Polarity_eTopField, &tf_isr);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_vbi_path;}
    rc = BVBI_Encode_GetInterruptName(vbi_path, BAVC_Polarity_eBotField, &bf_isr);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_vbi_path;}

    display->vbi.progressive = false;

    /* unfortunately we have to recreate everything because the vbi_path might change. */
    rc = BVBI_Encode_Create(video->vbi, vbi_path, NULL, &display->vbi.enc_core);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_vbi_encode;}

    rc = BVBIlib_Encode_Create(video->vbilib, video->vbilist, display->vbi.enc_core,
        NEXUS_VBI_ENCODER_QUEUE_SIZE,
        &display->vbi.enc);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_vbilib_encode;}

    /* we are using top ISR to feed bottom field data and bottom field isr to feed top field data */
    rc = BINT_CreateCallback( &display->vbi.tf_isr, g_pCoreHandles->bint, tf_isr, NEXUS_Display_P_DequeVbi_isr, display, BAVC_Polarity_eBotField);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_tf_isr;}

    rc = BINT_CreateCallback( &display->vbi.bf_isr, g_pCoreHandles->bint, bf_isr, NEXUS_Display_P_DequeVbi_isr, display, BAVC_Polarity_eTopField);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_bf_isr;}

    rc = BINT_EnableCallback(display->vbi.tf_isr);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_isr_cfg;}

    rc = BINT_EnableCallback(display->vbi.bf_isr);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_isr_cfg;}

    rc = NEXUS_Display_P_EnableVbi(display, display->cfg.format);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_enable_vbi;}

    BDBG_ASSERT(display->vbi.tf_isr && display->vbi.bf_isr);
    return BERR_SUCCESS;

err_enable_vbi:
err_isr_cfg:
    cleanup_rc = BINT_DestroyCallback(display->vbi.bf_isr);
    if (cleanup_rc!=BERR_SUCCESS) { cleanup_rc = BERR_TRACE(cleanup_rc); }
err_bf_isr:
    cleanup_rc = BINT_DestroyCallback(display->vbi.tf_isr);
    if (cleanup_rc!=BERR_SUCCESS) { cleanup_rc = BERR_TRACE(cleanup_rc); }
err_tf_isr:
    cleanup_rc = BVBIlib_Encode_Destroy(display->vbi.enc);
    if (cleanup_rc!=BERR_SUCCESS) { cleanup_rc = BERR_TRACE(cleanup_rc); }
    display->vbi.enc = NULL;
err_vbilib_encode:
    cleanup_rc = BVBI_Encode_Destroy(display->vbi.enc_core);
    if (cleanup_rc!=BERR_SUCCESS) { cleanup_rc = BERR_TRACE(cleanup_rc); }
    display->vbi.enc_core = NULL;
err_vbi_encode:
err_vbi_path:
    return rc;
}

void
NEXUS_Display_P_DisconnectVbi(NEXUS_DisplayHandle display)
{
    BERR_Code rc;

    BDBG_MSG(("disconnect display%d from vbi", display->index));

    if (display->vbi.enc_core) {
        NEXUS_Display_P_DisableVbi(display);

        rc = BINT_DestroyCallback(display->vbi.bf_isr);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); }
        rc = BINT_DestroyCallback(display->vbi.tf_isr);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); }
        rc = BVBIlib_Encode_Destroy(display->vbi.enc);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); }
        display->vbi.enc = NULL;
        rc = BVBI_Encode_Destroy(display->vbi.enc_core);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); }
        display->vbi.enc_core = NULL;
    }

    return;
}

BERR_Code
NEXUS_Display_P_VbiData_isr(NEXUS_DisplayHandle display, BVBI_Field_Handle vbiData)
{
    BERR_Code rc;

    if (!display->vbi.enc_core) {
        /* we did not consume BVBI_Field_Handle, so we can't return 0. */
        return -1;
    }
    /* NOTE: display->vbi.enc_core != NULL is the test for VBI capability. display->vbi.enc should track with it. */
    BDBG_ASSERT(display->vbi.enc);

    rc = BVBIlib_Encode_Enqueue_isr(display->vbi.enc, vbiData);
    if (rc) {
        BDBG_WRN(("Flushing VBI encoder queue because of overflow."));
        /* If we can't enqueue, the buffer is probably full. Try to recover by flushing. */
        BVBIlib_Encode_Flush_isr(display->vbi.enc);
    }
    return rc;
}

void
NEXUS_Display_P_DisableVbi(NEXUS_DisplayHandle display)
{
    BERR_Code rc;

    if (!display->vbi.enc_core) {
        return;
    }

    rc = BVBI_Encode_SetCC(display->vbi.enc_core, false);
    if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}

    rc = BVBI_Encode_SetTeletext(display->vbi.enc_core, false);
    if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}

    rc = BVBI_Encode_SetWSS(display->vbi.enc_core, false);
    if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}

    rc = BVBI_Encode_SetCGMSA(display->vbi.enc_core, false);
    if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}
    display->vbi.enabled[nexus_vbi_resources_cgmse] = false;

    rc = BVBI_Encode_SetCGMSB(display->vbi.enc_core, false);
    if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}

    rc = BVBI_Encode_SetVPS(display->vbi.enc_core, false);
    if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}

    rc = BVBI_Encode_SetGemstar(display->vbi.enc_core, false);
    if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}

    rc = BVBI_Encode_SetAMOL(display->vbi.enc_core, false);
    if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}

    rc = BVBI_Encode_ApplyChanges(display->vbi.enc_core);
    if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}

    BKNI_EnterCriticalSection();
    BVBIlib_Encode_Flush_isr(display->vbi.enc);
    BKNI_LeaveCriticalSection();

    return ;
}

bool nexus_display_p_vbi_available(enum nexus_vbi_resources res)
{
    BVBI_Capabilities cap;
    unsigned total = 0, i;
    for (i=0;i<sizeof(g_NEXUS_DisplayModule_State.displays)/sizeof(g_NEXUS_DisplayModule_State.displays[0]);i++) {
        if (g_NEXUS_DisplayModule_State.displays[i]) {
            if (g_NEXUS_DisplayModule_State.displays[i]->vbi.enabled[res]) total++;
        }
    }
    BVBI_GetCapabilities(g_NEXUS_DisplayModule_State.vbi, &cap);
    switch (res) {
    case nexus_vbi_resource_vec_int:
        return total < cap.ulNumVecEnc;
    case nexus_vbi_resource_vec_bypass_int:
        return total < cap.ulNumPassThruEnc;
    case nexus_vbi_resources_cgmse:
        return total < cap.ulNumCgmse;
    default:
        return false;
    }
}

BERR_Code
NEXUS_Display_P_EnableVbi(NEXUS_DisplayHandle display, NEXUS_VideoFormat format)
{
    BERR_Code rc;
    bool isSd = NEXUS_P_VideoFormat_IsSd(format);
    BFMT_VideoFmt formatVdc;
    bool enabled;

    if (!display->vbi.enc_core) {
        BDBG_MSG(("Display does not support VBI encoding."));
        return 0;
    }

    /* for formats that don't support VBI, print a WRN but succeed. this will allow the other display settings to be applied */
    switch (format) {
    case NEXUS_VideoFormat_e1080p24hz:
    case NEXUS_VideoFormat_e1080p25hz:
    case NEXUS_VideoFormat_e1080p30hz:
    case NEXUS_VideoFormat_e1080p50hz:
    case NEXUS_VideoFormat_e1080p60hz:
    case NEXUS_VideoFormat_e1080p100hz:
    case NEXUS_VideoFormat_e1080p120hz:
    case NEXUS_VideoFormat_ePal60hz:
    case NEXUS_VideoFormat_e3840x2160p24hz:
    case NEXUS_VideoFormat_e3840x2160p25hz:
    case NEXUS_VideoFormat_e3840x2160p30hz:
    case NEXUS_VideoFormat_e3840x2160p50hz:
    case NEXUS_VideoFormat_e3840x2160p60hz:
    case NEXUS_VideoFormat_e4096x2160p24hz:
    case NEXUS_VideoFormat_e4096x2160p25hz:
    case NEXUS_VideoFormat_e4096x2160p30hz:
    case NEXUS_VideoFormat_e4096x2160p50hz:
    case NEXUS_VideoFormat_e4096x2160p60hz:
    case NEXUS_VideoFormat_eNtsc443:
        BDBG_MSG(("Display VideoFormat %d does not support VBI encoding.", format));
        return 0;
    default:
        break;
    }

    /* No VESA modes */
    if (format >= NEXUS_VideoFormat_eVesa640x480p60hz)  {
        BDBG_MSG(("VESA display formats (%d) do not support VBI encoding.", format));
        return 0;
    }

    display->vbi.progressive = !NEXUS_P_VideoFormat_IsInterlaced(format);

    rc = NEXUS_P_VideoFormat_ToMagnum_isrsafe(format, &formatVdc);
    if (rc) {return BERR_TRACE(rc);}

    rc = BVBI_Encode_SetVideoFormat(display->vbi.enc_core,  formatVdc);
    if (rc) {
        BDBG_WRN(("NEXUS_Display_P_EnableVbi: %#lx VBI not supported for %u", (unsigned long)display, (unsigned)format));
        return BERR_TRACE(rc);
    }

    rc = BVBI_Encode_SetCC(display->vbi.enc_core, isSd && display->vbi.settings.closedCaptionEnabled);
    if (rc) {
        BDBG_WRN(("ClosedCaption is not supported for this display format"));
        return BERR_TRACE(rc);
    }

    rc = BVBI_Encode_SetTeletext(display->vbi.enc_core, isSd && display->vbi.settings.teletextEnabled);
    if (rc) {
        BDBG_WRN(("Teletext is not supported for this display format"));
        return BERR_TRACE(rc);
    }

    rc = BVBI_Encode_SetWSS(display->vbi.enc_core, display->vbi.settings.wssEnabled);
    if (rc) {
        BDBG_WRN(("WSS is not supported for this display format"));
        return BERR_TRACE(rc);
    }

    if(g_NEXUS_DisplayModule_State.moduleSettings.vbi.allowCgmsB){
        /* CGMS is supported for NTSC and 50/60Hz HD, but not PAL. */
        rc = BVBI_Encode_SetCGMSB(display->vbi.enc_core, (format != NEXUS_VideoFormat_ePal) && display->vbi.settings.cgmsEnabled);
        if (rc) {
            BDBG_WRN(("CGMS B is not supported for this display format"));
            return BERR_TRACE(rc);
        }
    }

    /* CGMS is supported for NTSC and 50/60Hz HD, but not PAL. */
    enabled = (format != NEXUS_VideoFormat_ePal) && display->vbi.settings.cgmsEnabled;
    if (enabled && !display->vbi.enabled[nexus_vbi_resources_cgmse] && !nexus_display_p_vbi_available(nexus_vbi_resources_cgmse)) {
        BDBG_WRN(("no CGMS A encoder available for display %u", display->index));
    }
    else {
        rc = BVBI_Encode_SetCGMSA(display->vbi.enc_core, enabled);
        if (rc) {
            BDBG_WRN(("CGMS A is not supported for this display format"));
            return BERR_TRACE(rc);
        }
        display->vbi.enabled[nexus_vbi_resources_cgmse] = enabled;
    }

    rc = BVBI_Encode_SetVPS(display->vbi.enc_core, display->vbi.settings.vpsEnabled);
    if (rc) {
        BDBG_WRN(("VPS is not supported for this display format"));
        return BERR_TRACE(rc);
    }

    if (display->vbi.settings.gemStarEnabled) {
        BVBI_GSOptions gsOptions;
        (void)BVBI_Encode_GetGemstarOptions(display->vbi.enc_core, &gsOptions);
        gsOptions.baseline_top = display->vbi.settings.gemStar.baseLineTop;
        gsOptions.linemask_top = display->vbi.settings.gemStar.lineMaskTop;
        gsOptions.baseline_bot = display->vbi.settings.gemStar.baseLineBottom;
        gsOptions.linemask_bot = display->vbi.settings.gemStar.lineMaskBottom;
        rc = BVBI_Encode_SetGemstarOptions(display->vbi.enc_core, &gsOptions);
        if (rc) return BERR_TRACE(rc);
    }

    rc = BVBI_Encode_SetGemstar(display->vbi.enc_core, display->vbi.settings.gemStarEnabled);
    if (rc) {
        BDBG_WRN(("GemStar is not supported for this display format"));
        return BERR_TRACE(rc);
    }

    if (display->vbi.settings.amolEnabled) {
        switch (display->vbi.settings.amol.type) {
        case NEXUS_AmolType_eI:
            display->vbi.amolType = BVBI_AMOL_Type_I; break;
        case NEXUS_AmolType_eII_Lowrate:
            display->vbi.amolType = BVBI_AMOL_Type_II_Lowrate; break;
        case NEXUS_AmolType_eII_Highrate:
            display->vbi.amolType = BVBI_AMOL_Type_II_Highrate; break;
        default:
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        rc = BVBI_Encode_SetAMOLOptions(display->vbi.enc_core, display->vbi.amolType);
        if (rc) return BERR_TRACE(rc);
    }

    rc = BVBI_Encode_SetAMOL(display->vbi.enc_core,
        /* AMOL only supported for NTSC and NTSC-J */
        (format == NEXUS_VideoFormat_eNtsc || format == NEXUS_VideoFormat_eNtscJapan) && display->vbi.settings.amolEnabled);
    if (rc) {
        BDBG_WRN(("AMOL is not supported"));
        return BERR_TRACE(rc);
    }


    rc = BVBI_Encode_ApplyChanges(display->vbi.enc_core);
    if (rc) {return BERR_TRACE(rc);}

    return BERR_SUCCESS;
}

void NEXUS_Display_GetVbiSettings(NEXUS_DisplayHandle display, NEXUS_DisplayVbiSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    *pSettings = display->vbi.settings;
}

NEXUS_Error NEXUS_Display_SetVbiSettings(NEXUS_DisplayHandle display, const NEXUS_DisplayVbiSettings *pSettings)
{
    NEXUS_Error rc;
    NEXUS_VideoInput prevInput;

    BDBG_OBJECT_ASSERT(display, NEXUS_Display);

    if (pSettings->amolEnabled && !g_NEXUS_DisplayModule_State.moduleSettings.vbi.allowAmol) {
        BDBG_WRN(("cannot enable amol because NEXUS_DisplayModuleSettings.vbi.allowAmol is false"));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    if (pSettings->gemStarEnabled && !g_NEXUS_DisplayModule_State.moduleSettings.vbi.allowGemStar) {
        BDBG_WRN(("cannot enable gemstar because NEXUS_DisplayModuleSettings.vbi.allowGemStar is false"));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    if (pSettings->teletextEnabled && !g_NEXUS_DisplayModule_State.moduleSettings.vbi.allowTeletext) {
        BDBG_WRN(("cannot enable teletext because NEXUS_DisplayModuleSettings.vbi.allowTeletext is false"));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    if (pSettings->vpsEnabled && !g_NEXUS_DisplayModule_State.moduleSettings.vbi.allowVps) {
        BDBG_WRN(("cannot enable vps because NEXUS_DisplayModuleSettings.vbi.allowVps is false"));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    prevInput = display->vbi.settings.vbiSource;

    display->vbi.settings = *pSettings;

    if (prevInput) {
        NEXUS_VideoInput_P_SetVbiState(prevInput);
    }
    if (pSettings->vbiSource && pSettings->vbiSource != prevInput) {
        NEXUS_VideoInput_P_SetVbiState(pSettings->vbiSource);
    }

    rc = NEXUS_Display_P_EnableVbi(display, display->cfg.format);
    if (rc) return BERR_TRACE(rc);

    return 0;
}

static BERR_Code NEXUS_Display_P_SendTeletextField(NEXUS_DisplayHandle display, BVBI_TT_Line *ttLines, unsigned ttLineCount, bool topField)
{
    BERR_Code rc;
    BVBI_Field_Handle field = NULL;
    const NEXUS_DisplayModule_State *video = &g_NEXUS_DisplayModule_State;
    BFMT_VideoFmt formatVdc;

    rc = NEXUS_P_VideoFormat_ToMagnum_isrsafe(display->cfg.format, &formatVdc);
    if (rc) {return BERR_TRACE(rc);}

    BKNI_EnterCriticalSection();
    /* Set up the field. If we fail, make sure to exit the critical section
    and free memory */
    /* VBI PI memory allocation for TT was already done at BVBIlist create time. */
    rc = BVBIlib_List_Obtain_isr(video->vbilist, &field);
    if (rc) {rc = BERR_TRACE(rc); field = NULL; goto done_critsec;}

    rc = BVBI_Field_SetTTData_isr(field, formatVdc, ttLineCount, ttLines);
    if (rc) {rc = BERR_TRACE(rc);goto done_critsec;}

    /* We are encoding on either field. I don't believe it matters if the data is transmitted top or bottom.
    If it does, we will need to modify. */
    rc = BVBI_Field_SetPolarity_isr(field, topField ? (1<<BAVC_Polarity_eTopField) : (1<<BAVC_Polarity_eBotField));
    if (rc) {rc = BERR_TRACE(rc);goto done_critsec;}

    /* This actually sends it to the VEC and consumes the field */
    if (BVBIlib_Encode_Enqueue_isr(display->vbi.enc, field)) {
        /* if it fails, assume it's a queue overflow and we're done */
        BVBIlib_List_Return_isr(video->vbilist, field);
        rc = -1;
    }

done_critsec:
    BKNI_LeaveCriticalSection();
    return rc;
}

/**
The following comments apply to NEXUS_Display_WriteTeletext and the PAL display format.

Teletext goes out on VBI lines 5..24. This is 20 lines.
The VBI PI has a hardcoded BVBI_TT_MAX_LINES, which is 18. Nexus uses this number.
The VBI PI's BVBI_TT_Line[0] corresponds to VBI line 5, BVBI_TT_Line[19] correponds to VBI line 24.
VBI line 5 must be skipped because the HW does not support it. TT_START_OFFSET specifies that 1 must be skipped.
    Apparently 5,6&7 had to be skipped on a 3563.
If VPS is enabled, VBI line 16 must be skipped.
If CC is enabled, VBI line 22 must be skipped.
If WSS is enabled, VBI line 23 must be skipped.
NEXUS_Display_WriteTeletext can skip these lines by setting an invalid framing code.
Therefore, if neither VPS, CC or WSS are used, the max # of TT lines/field is 19. If all three are in use, the max is 16.

TODO: if a user needs exactly lineNumber control in the application, I recommend we add bool NEXUS_DisplayVbiSettings.teletextLinenumEnabled
to enable this. it would default to false so this code remains backward compatible.
we might also want an option to just ignore lineNumber completely. In that case, NEXUS_DisplayVbiSettings.teletextLinenumMode w/ a tristate enum.
**/
#define MAX_TT_LINES_PER_FIELD (BVBI_TT_MAX_LINES)
#define TT_VBI_LINE_OFFSET 5
#if BCHP_CHIP == 3563
#define TT_START_OFFSET 3
#else
#define TT_START_OFFSET 1
#endif

NEXUS_Error NEXUS_Display_WriteTeletext(NEXUS_DisplayHandle display, const NEXUS_TeletextLine *pLines,
    size_t numLines, size_t *pNumLinesWritten)
{
    BERR_Code rc = 0;
    unsigned i;

    if (numLines && !pLines) return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    *pNumLinesWritten = 0; /* in case of error, assigned early */
    if (!display->vbi.settings.teletextEnabled) {
        BDBG_WRN(("NEXUS_DisplayVbiSettings.teletextEnabled is false"));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    /* malloc space when needed. free on module shutdown. */
    if (!g_NEXUS_DisplayModule_State.ttLines) {
        g_NEXUS_DisplayModule_State.ttLines = BKNI_Malloc(sizeof(g_NEXUS_DisplayModule_State.ttLines[0]) * MAX_TT_LINES_PER_FIELD);
        if (!g_NEXUS_DisplayModule_State.ttLines) {
            return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        }
    }

    for (i=0;i<numLines;) {
        /* prep ttLines[] */
        BVBI_TT_Line *ttLines = g_NEXUS_DisplayModule_State.ttLines;
        unsigned ttLineCount = 0;
        unsigned prevLineNumber = 0;
        unsigned j;

        for (j=0;j<MAX_TT_LINES_PER_FIELD;j++) {
            ttLines[j].ucFramingCode = BVBI_TT_INVALID_FRAMING_CODE;
        }

        /* skip the initial number of lines that can't be output */
        ttLineCount += TT_START_OFFSET;

        for (;i<numLines && ttLineCount<MAX_TT_LINES_PER_FIELD;i++,ttLineCount++) {
            if (pLines[i].lineNumber < prevLineNumber) {
                /* the app wants a field break here */
                break;
            }

            /* skip lines which are used by other VBI standards */
            if ((ttLineCount == 16 - TT_VBI_LINE_OFFSET) && display->vbi.settings.vpsEnabled) ttLineCount++;
            if ((ttLineCount == 22 - TT_VBI_LINE_OFFSET) && display->vbi.settings.closedCaptionEnabled) ttLineCount++;
            if ((ttLineCount == 23 - TT_VBI_LINE_OFFSET) && display->vbi.settings.wssEnabled) ttLineCount++;

            if (ttLineCount >= MAX_TT_LINES_PER_FIELD) {
                /* need a field break here */
                ttLineCount = MAX_TT_LINES_PER_FIELD;
                break;
            }

            /* only do work if not BVBI_TT_INVALID_FRAMING_CODE. but always consume the pLines[i] line. */
            if (pLines[i].framingCode != BVBI_TT_INVALID_FRAMING_CODE) {
                ttLines[ttLineCount].ucFramingCode = pLines[i].framingCode;
                BDBG_CASSERT(NEXUS_TELETEXT_LINESIZE == sizeof(ttLines[0].aucData));
                BKNI_Memcpy(ttLines[ttLineCount].aucData, pLines[i].data, NEXUS_TELETEXT_LINESIZE);
                BDBG_MSG_TRACE(("copying line[%d] to ttLines[%d]: %02x", i, ttLineCount, ttLines[ttLineCount].ucFramingCode));
            }

            prevLineNumber = pLines[i].lineNumber;
        }

        rc = NEXUS_Display_P_SendTeletextField(display, ttLines, ttLineCount, pLines->topField);
        if (rc) {return 0;} /* Failure is normal for flow control. */

        BDBG_MSG_TRACE(("NEXUS_Display_WriteTeletext: sent %d VBI lines, %d of %d user lines consumed", ttLineCount, i, numLines));
        (*pNumLinesWritten) = i; /* total actually consumed */
    }

    BDBG_ASSERT(*pNumLinesWritten <= i);

    return rc;
}

NEXUS_Error NEXUS_Display_WriteClosedCaption(NEXUS_DisplayHandle display, const NEXUS_ClosedCaptionData *pEntries,
    size_t numEntries, size_t *pNumEntriesWritten)
{
    BVBI_Field_Handle field;
    BERR_Code rc = 0;
    unsigned i;
    int full = 0;
    const NEXUS_DisplayModule_State *video = &g_NEXUS_DisplayModule_State;

    *pNumEntriesWritten = 0; /* in case of error, assigned early */
    if (!display->vbi.settings.closedCaptionEnabled) {
        BDBG_WRN(("NEXUS_DisplayVbiSettings.closedCaptionEnabled is false"));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    for (i=0;i<numEntries;i++) {
        uint32_t polarityMask;

        BKNI_EnterCriticalSection();

        /* Set up the field. If we fail, make sure to exit the critical section
        and free memory */
        rc = BVBIlib_List_Obtain_isr(video->vbilist, &field);
        if (rc) {rc = 0; field = NULL; goto done_critsec;} /* running out of fields is normal flow control */

        rc = BVBI_Field_SetCCData_isr(field, pEntries[i].data[0], pEntries[i].data[1]);
        if (rc) {rc = BERR_TRACE(rc);goto done_critsec;}

        switch (pEntries[i].field) {
        case 0: polarityMask = (1<<BAVC_Polarity_eTopField); break;
        case 1: polarityMask = (1<<BAVC_Polarity_eBotField); break;
        default: rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto done_critsec;
        }
        rc = BVBI_Field_SetPolarity_isr(field, polarityMask);
        if (rc) {rc = BERR_TRACE(rc);goto done_critsec;}

        /* This actually sends it to the VEC and consumes the field */
        if (BVBIlib_Encode_Enqueue_isr(display->vbi.enc, field)) {
            /* if it fails, assume it's a queue overflow and we're done */
            full = 1;
        }
        else {
            field = NULL;
        }

done_critsec:
        if (field) {
            BVBIlib_List_Return_isr(video->vbilist, field);
            field = NULL; /* consumed */
        }
        BKNI_LeaveCriticalSection();

        if (full || rc)
            break;

        (*pNumEntriesWritten)++;
    }

    return rc;
}

NEXUS_Error NEXUS_Display_WriteGemStar(NEXUS_DisplayHandle display, const NEXUS_GemStarData *pEntries,
    size_t numEntries, size_t *pNumEntriesWritten)
{

    BVBI_Field_Handle field;
    BERR_Code rc = 0;
    unsigned i;
    int full = 0;
    const NEXUS_DisplayModule_State *video = &g_NEXUS_DisplayModule_State;

    *pNumEntriesWritten = 0; /* in case of error, assigned early */

    if (!display->vbi.settings.gemStarEnabled) {
        BDBG_WRN(("NEXUS_DisplayVbiSettings.gemStarEnabled is false"));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    for (i=0;i<numEntries;i++) {
        /* Convert from Nexus to Magnum */
        BVBI_GSData gsData;
        gsData.ulDataLines = pEntries[i].lineMask;
        gsData.ulErrorLines = 0;
        BDBG_CASSERT(sizeof(pEntries[0].data) == sizeof(gsData.ulData));
        BKNI_Memcpy(gsData.ulData, pEntries[i].data, sizeof(pEntries[i].data));

        BKNI_EnterCriticalSection();

        /* Set up the field. If we fail, make sure to exit the critical section
        and free memory */
        rc = BVBIlib_List_Obtain_isr(video->vbilist, &field);
        if (rc) {rc = BERR_TRACE(rc); field = NULL; goto done_critsec;}
        rc = BVBI_Field_SetGSData_isr(field, &gsData);
        if (rc) {rc = BERR_TRACE(rc);goto done_critsec;}

        if (pEntries[i].topField) {
            rc = BVBI_Field_SetPolarity_isr(field, 1<<BAVC_Polarity_eTopField);
            if (rc) {rc = BERR_TRACE(rc);goto done_critsec;}
        }
        else {
            rc = BVBI_Field_SetPolarity_isr(field, 1<<BAVC_Polarity_eBotField);
            if (rc) {rc = BERR_TRACE(rc);goto done_critsec;}
        }

        /* This actually sends it to the VEC and consumes the field */
        if (BVBIlib_Encode_Enqueue_isr(display->vbi.enc, field)) {
            /* if it fails, assume it's a queue overflow and we're done */
            full = 1;
        }
        else {
            field = NULL;
        }

done_critsec:
        if (field) {
            BVBIlib_List_Return_isr(video->vbilist, field);
            field = NULL; /* consumed */
        }
        BKNI_LeaveCriticalSection();

        if (full || rc)
            break;

        (*pNumEntriesWritten)++;
    }

    return rc;
}

NEXUS_Error NEXUS_Display_SetWss(NEXUS_DisplayHandle display, uint16_t wssData)
{
    if (!display->vbi.settings.wssEnabled) {
        BDBG_WRN(("NEXUS_DisplayVbiSettings.wssEnabled is false"));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    BKNI_EnterCriticalSection();
    display->vbi.pending.wssSet = true;
    display->vbi.pending.wssData = wssData;
    BKNI_LeaveCriticalSection();

    return 0;
}

NEXUS_Error NEXUS_Display_SetCgms(NEXUS_DisplayHandle display, uint32_t cgmsData)
{
    if (!display->vbi.settings.cgmsEnabled) {
        BDBG_WRN(("NEXUS_DisplayVbiSettings.cgmsEnabled is false"));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    BKNI_EnterCriticalSection();
    /* must queue CGMS value for both top & bottom fields */
    display->vbi.pending.cgmsTopSet = true;
    display->vbi.pending.cgmsBottomSet = true;
    display->vbi.pending.cgmsData = cgmsData;
    BKNI_LeaveCriticalSection();

    return 0;
}

NEXUS_Error NEXUS_Display_SetCgmsB(NEXUS_DisplayHandle display, const uint32_t *pCgmsData, unsigned size )
{
    if (!pCgmsData) return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    if (!g_NEXUS_DisplayModule_State.moduleSettings.vbi.allowCgmsB) {
        BDBG_WRN(("NEXUS_DisplayModuleSettings.vbi.allowCgmsB is false"));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    if (!display->vbi.settings.cgmsEnabled) {
        BDBG_WRN(("NEXUS_DisplayVbiSettings.cgmsEnabled is false"));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    if (size < 5) {
        BDBG_WRN(("CGMS-B requires an array of 5 uint32_t's"));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    BKNI_EnterCriticalSection();
    /* must queue CGMS-B value for both top & bottom fields */
    display->vbi.pending.cgmsBTopSet = true;
    display->vbi.pending.cgmsBBottomSet = true;
    BKNI_Memcpy(display->vbi.pending.cgmsBData, pCgmsData, sizeof(uint32_t)*5);
    BKNI_LeaveCriticalSection();

    return 0;
}

NEXUS_Error NEXUS_Display_SetVps(NEXUS_DisplayHandle display, const NEXUS_VpsData *pData)
{
    if (!display->vbi.settings.vpsEnabled) {
        BDBG_WRN(("NEXUS_DisplayVbiSettings.vpsEnabled is false"));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    /* only enqueue top field for VPS */
    BKNI_EnterCriticalSection();
    display->vbi.pending.vpsSet = true;
    display->vbi.pending.vpsData.ucByte05 = pData->byte05;
    display->vbi.pending.vpsData.ucByte11 = pData->byte11;
    display->vbi.pending.vpsData.ucByte12 = pData->byte12;
    display->vbi.pending.vpsData.ucByte13 = pData->byte13;
    display->vbi.pending.vpsData.ucByte14 = pData->byte14;
    display->vbi.pending.vpsData.ucByte15 = pData->byte15;
    BKNI_LeaveCriticalSection();

    return 0;
}

/* bvdc_nomacrovision_priv.c does not provide stubs for public API, so we must #if */
#ifdef MACROVISION_SUPPORT
#include "bvdc_macrovision.h"

static bool nexus_p_display_supports_macrovision(NEXUS_VideoFormat format)
{
    NEXUS_VideoFormatInfo info;
    NEXUS_VideoFormat_GetInfo(format, &info);
    /* logic extracted from BVDC_P_aFormatInfoTable[] in bvdc_displayfmt_priv.c. */
    return info.height <= 576 &&
           format != NEXUS_VideoFormat_eNtsc443 &&
           format != NEXUS_VideoFormat_ePalN &&
           format != NEXUS_VideoFormat_ePal60hz &&
           format != NEXUS_VideoFormat_e720x482_NTSC_J;
}

static NEXUS_Error NEXUS_Display_P_SetMacrovision( NEXUS_DisplayHandle display, NEXUS_DisplayMacrovisionType type, const NEXUS_DisplayMacrovisionTables *pTable, NEXUS_VideoFormat videoFormat)
{
    BERR_Code rc;
    BVDC_MacrovisionType mv_type;

    BDBG_OBJECT_ASSERT(display, NEXUS_Display);

    if (type != NEXUS_DisplayMacrovisionType_eNone && !nexus_p_display_supports_macrovision(videoFormat)) {
        BDBG_WRN(("display %u format %u does not support macrovision.", display->index, videoFormat));
        type = NEXUS_DisplayMacrovisionType_eNone;
    }

    switch (type) {
    case NEXUS_DisplayMacrovisionType_eAgcOnly:
        mv_type = BVDC_MacrovisionType_eAgcOnly;
        break;
    case NEXUS_DisplayMacrovisionType_eAgc2Lines:
        mv_type = BVDC_MacrovisionType_eAgc2Lines;
        break;
    case NEXUS_DisplayMacrovisionType_eAgc4Lines:
        mv_type = BVDC_MacrovisionType_eAgc4Lines;
        break;
    case NEXUS_DisplayMacrovisionType_eAgcOnlyRgb:
        mv_type = BVDC_MacrovisionType_eAgcOnly_Rgb;
        break;
    case NEXUS_DisplayMacrovisionType_eAgc2LinesRgb:
        mv_type = BVDC_MacrovisionType_eAgc2Lines_Rgb;
        break;
    case NEXUS_DisplayMacrovisionType_eAgc4LinesRgb:
        mv_type = BVDC_MacrovisionType_eAgc4Lines_Rgb;
        break;
    case NEXUS_DisplayMacrovisionType_eTest01:
        mv_type = BVDC_MacrovisionType_eTest01;
        break;
    case NEXUS_DisplayMacrovisionType_eTest02:
        mv_type = BVDC_MacrovisionType_eTest02;
        break;
    case NEXUS_DisplayMacrovisionType_eCustom:
        mv_type = BVDC_MacrovisionType_eCustomized;
        break;
    default:
    case NEXUS_DisplayMacrovisionType_eNone:
        mv_type = BVDC_MacrovisionType_eNoProtection;
        break;
    }

    rc = BVDC_Display_SetMacrovisionType(display->displayVdc, mv_type);
    if (rc) return BERR_TRACE(rc);

    if (type == NEXUS_DisplayMacrovisionType_eCustom && pTable) {
        rc = BVDC_Display_SetMacrovisionTable(display->displayVdc, pTable->cpcTable, pTable->cpsTable);
        if (rc) return BERR_TRACE(rc);
    }

    return 0;
}

void nexus_p_check_macrovision( NEXUS_DisplayHandle display, NEXUS_VideoFormat videoFormat)
{
    (void)NEXUS_Display_P_SetMacrovision(display, display->macrovision.type,  display->macrovision.tableSet?&display->macrovision.table:NULL, videoFormat);
}

NEXUS_Error NEXUS_Display_SetMacrovision( NEXUS_DisplayHandle display, NEXUS_DisplayMacrovisionType type, const NEXUS_DisplayMacrovisionTables *pTable)
{
    NEXUS_Error rc;

    rc = NEXUS_Display_P_SetMacrovision(display, type, pTable, display->cfg.format);
    if (rc) return BERR_TRACE(rc);

    rc = NEXUS_Display_P_ApplyChanges();
    if (rc) return BERR_TRACE(rc);

    display->macrovision.type = type;
    if (pTable) {
        display->macrovision.tableSet = true;
        display->macrovision.table = *pTable;
    }
    else {
        display->macrovision.tableSet = false;
    }

    return NEXUS_SUCCESS;
}
#else
void nexus_p_check_macrovision( NEXUS_DisplayHandle display, NEXUS_VideoFormat videoFormat)
{
    BSTD_UNUSED(display);
    BSTD_UNUSED(videoFormat);
}
NEXUS_Error NEXUS_Display_SetMacrovision( NEXUS_DisplayHandle display, NEXUS_DisplayMacrovisionType type, const NEXUS_DisplayMacrovisionTables *pTable)
{
    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    BSTD_UNUSED(type);
    BSTD_UNUSED(pTable);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}
#endif

/* bvdc_nodcs_priv.c does not provide stubs for public API, so we must #if */
#ifdef DCS_SUPPORT
#include "bvdc_dcs.h"
#endif

NEXUS_Error NEXUS_Display_SetDcs( NEXUS_DisplayHandle display, NEXUS_DisplayDcsType type )
{
#ifdef DCS_SUPPORT
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(display, NEXUS_Display);

    BDBG_CASSERT(BVDC_DCS_Mode_eUndefined == (BVDC_DCS_Mode)NEXUS_DisplayDcsType_eMax);
    rc = BVDC_Display_SetDcsMode( display->displayVdc, (BVDC_DCS_Mode)type);
    if (rc) return BERR_TRACE(rc);

    rc = NEXUS_Display_P_ApplyChanges();
    if (rc) return BERR_TRACE(rc);
    return 0;
#else
    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    BSTD_UNUSED(type);

    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

NEXUS_Error NEXUS_Display_DcsLock( NEXUS_DisplayHandle display )
{
#ifdef DCS_SUPPORT
    BERR_Code rc;
    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    rc = BVDC_Display_DcsLock(display->displayVdc);
    if (rc) return BERR_TRACE(rc);
    return 0;
#else
    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

NEXUS_Error NEXUS_Display_WriteAmol( NEXUS_DisplayHandle display, const NEXUS_AmolData *pEntries, size_t numEntries, size_t *pNumEntriesWritten )
{
    BVBI_Field_Handle field;
    BERR_Code rc = 0;
    unsigned i;
    int full = 0;
    const NEXUS_DisplayModule_State *video = &g_NEXUS_DisplayModule_State;

    BDBG_OBJECT_ASSERT(display, NEXUS_Display);

    *pNumEntriesWritten = 0; /* in case of error, assigned early */
    if (!display->vbi.settings.amolEnabled) {
        BDBG_WRN(("NEXUS_DisplayVbiSettings.amolEnabled is false"));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    for (i=0;i<numEntries;i++) {
        uint32_t polarityMask;

        BKNI_EnterCriticalSection();

        /* Set up the field. If we fail, make sure to exit the critical section
        and free memory */
        rc = BVBIlib_List_Obtain_isr(video->vbilist, &field);
        if (rc) {rc = 0; field = NULL; goto done_critsec;} /* running out of fields is normal flow control */

        rc = BVBI_Field_SetAMOLData_isr(field, display->vbi.amolType, (uint8_t*)pEntries[i].data, pEntries[i].length);
        if (rc) {rc = BERR_TRACE(rc);goto done_critsec;}

        polarityMask = pEntries[i].topField ? (1<<BAVC_Polarity_eTopField) : (1<<BAVC_Polarity_eBotField);
        rc = BVBI_Field_SetPolarity_isr(field, polarityMask);
        if (rc) {rc = BERR_TRACE(rc);goto done_critsec;}

        /* This actually sends it to the VEC and consumes the field */
        if (BVBIlib_Encode_Enqueue_isr(display->vbi.enc, field)) {
            /* if it fails, assume it's a queue overflow and we're done */
            full = 1;
        }
        else {
            field = NULL;
        }

done_critsec:
        if (field) {
            BVBIlib_List_Return_isr(video->vbilist, field);
            field = NULL; /* consumed */
        }
        BKNI_LeaveCriticalSection();

        if (full || rc)
            break;

        (*pNumEntriesWritten)++;
    }
    return rc;
}

#else /* NEXUS_VBI_SUPPORT */

void NEXUS_Display_GetVbiSettings(NEXUS_DisplayHandle display, NEXUS_DisplayVbiSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    BSTD_UNUSED(pSettings);
}

NEXUS_Error NEXUS_Display_SetVbiSettings(NEXUS_DisplayHandle display, const NEXUS_DisplayVbiSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    BSTD_UNUSED(pSettings);
    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_Display_WriteTeletext(NEXUS_DisplayHandle display, const NEXUS_TeletextLine *pLines,
    size_t numLines, size_t *pNumLinesWritten)
{
    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    BSTD_UNUSED(pLines);
    BSTD_UNUSED(numLines);
    BSTD_UNUSED(pNumLinesWritten);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_Display_WriteClosedCaption(NEXUS_DisplayHandle display, const NEXUS_ClosedCaptionData *pEntries,
    size_t numEntries, size_t *pNumEntriesWritten)
{
    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    BSTD_UNUSED(pEntries);
    BSTD_UNUSED(numEntries);
    BSTD_UNUSED(pNumEntriesWritten);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_Display_WriteGemStar(NEXUS_DisplayHandle display, const NEXUS_GemStarData *pEntries,
    size_t numEntries, size_t *pNumEntriesWritten)
{
    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    BSTD_UNUSED(pEntries);
    BSTD_UNUSED(numEntries);
    BSTD_UNUSED(pNumEntriesWritten);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_Display_SetWss(NEXUS_DisplayHandle display, uint16_t wssData)
{
    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    BSTD_UNUSED(wssData);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_Display_SetCgms(NEXUS_DisplayHandle display, uint32_t cgmsData)
{
    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    BSTD_UNUSED(cgmsData);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_Display_SetCgmsB(NEXUS_DisplayHandle display, const uint32_t *pCgmsData, unsigned size )
{
    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    BSTD_UNUSED(pCgmsData);
    BSTD_UNUSED(size);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_Display_SetVps(NEXUS_DisplayHandle display, const NEXUS_VpsData *pData)
{
    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    BSTD_UNUSED(pData);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_Display_SetMacrovision( NEXUS_DisplayHandle display, NEXUS_DisplayMacrovisionType type, const NEXUS_DisplayMacrovisionTables *pTable)
{
    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    BSTD_UNUSED(type);
    BSTD_UNUSED(pTable);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_Display_SetDcs( NEXUS_DisplayHandle display, NEXUS_DisplayDcsType type )
{
    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    BSTD_UNUSED(type);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_Display_DcsLock( NEXUS_DisplayHandle display )
{
    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_Display_WriteAmol( NEXUS_DisplayHandle display, const NEXUS_AmolData *pEntries, size_t numEntries, size_t *pNumEntriesWritten )
{
    BDBG_OBJECT_ASSERT(display, NEXUS_Display);
    BSTD_UNUSED(pEntries);
    BSTD_UNUSED(numEntries);
    BSTD_UNUSED(pNumEntriesWritten);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

#endif /* NEXUS_VBI_SUPPORT */

/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
/* this file shall be included only from nexus_display_module.h */
#ifndef NEXUS_VIDEO_INPUT_IMPL_H__
#define NEXUS_VIDEO_INPUT_IMPL_H__

/**
This section contains internal API's for the generic NEXUS_VideoInput connector.
**/

BDBG_OBJECT_ID_DECLARE(NEXUS_VideoInput_P_Link);

struct NEXUS_VideoInput_P_Link;

typedef struct NEXUS_VideoInput_P_Iface {
    BERR_Code (*connect)(struct NEXUS_VideoInput_P_Link *input);
    void (*disconnect)(struct NEXUS_VideoInput_P_Link *input);
} NEXUS_VideoInput_P_Iface;

/* The internal VBI settings combines the user's request via the VideoInput interface and
multiple Display interfaces. */
typedef struct NEXUS_VideoInputInternalVbiSettings {
    /* we must reference count these because multiple displays may enable/disable individual vbi types */
    unsigned teletextRefCnt;           /* Decode TT data and pass to callback */
    unsigned closedCaptionRefCnt;      /* Decode CC data and pass to callback */
    unsigned wssRefCnt;                /* Decode WSS data and pass to callback */
    unsigned cgmsRefCnt;               /* Decode CGMS data and pass to callback */
    unsigned gemStarRefCnt;            /* Decode GS data and pass to callback */
    unsigned vpsRefCnt;                /* Decode VPS data and pass to callback */
} NEXUS_VideoInputInternalVbiSettings;

/**
NEXUS_VideoInput_P_Link contains Display module information associated with a NEXUS_VideoInput (chiefly, BVDC_Source_Handle).

NEXUS_VideoInput.destination points to NEXUS_VideoInput_P_Link after the first NEXUS_VideoWindow_AddInput is called.
NEXUS_VideoInput.destination is cleared when NEXUS_VideoWindow_RemoveInput is called on the last window.
The reason NEXUS_VideoInput.destination is cleared even though NEXUS_VideoInput_P_Link is not destroyed is that's possible that
NEXUS_VideoInput could be connected to an interface for another module (not Display). This is not true at this time, but it could be.

When NEXUS_VideoInput is re-added to a window, any NEXUS_VideoInput_P_Link which was already created will be re-used. Its state will persist.
NEXUS_VideoInput_P_Link is only destroyed when NEXUS_VideoInput_Shutdown is called.

Also, when NEXUS_VideoInput.destination is NULL, any setting applied to NEXUS_VideoInput (e.g. NEXUS_VideoInput_SetSettings)
will be applied to the NEXUS_VideoInput_P_Link by means of a temporary lookup; the .destination will not be set.


Mapping of link state to public API functions:
1. NEXUS_VideoWindow_AddInput - creates link, VDC source & VDC window
2. NEXUS_VideoWindow_RemoveInput - destroys VDC window (not link or VDC source)
3. NEXUS_VideoInput_Get/Set - creates link (if not already created)
    TODO: it is not required that Get/Set will create the VDC source. for some chips, it's preferrable that it does not.
4. NEXUS_VideoInput_Shutdown - destroy link & VDC source (does implicit VideoWindow_RemoveInput if not already done)
**/

typedef struct NEXUS_VideoInput_P_Link_Fifo {
    const char *name; /* for debug */
    unsigned bufferSize; /* number of elements */
    unsigned elementSize; /* size of each element in bytes */
    void *data; /* size is bufferSize*elementSize */
    unsigned wptr, rptr; /* in units of elementSize */
    bool inactive; /* set to true for time when buffer is reallocated */
    NEXUS_IsrCallbackHandle isrCallback;
} NEXUS_VideoInput_P_Link_Fifo;

typedef struct NEXUS_VideoInput_P_Link {
    BDBG_OBJECT(NEXUS_VideoInput_P_Link)
    BVDC_Source_Handle sourceVdc;
    bool copiedSourceVdc;
    BAVC_SourceId id;
    NEXUS_VideoInput input;
    NEXUS_HeapHandle heap;
    BVDC_Mode mtg; /* BVDC_Source created with mtg */
    bool secureVideo;

    NEXUS_VideoInputSettings cfg;
    NEXUS_VideoInputStatus status;
    NEXUS_VideoInputResumeMode resumeMode;
    bool bColorMatrixSet;
    NEXUS_ColorMatrix colorMatrix;

    bool isDeferCfg;
    bool isDeferColorMatrix;

    NEXUS_VideoInputVbiSettings vbiSettings; /* from VideoInput_SetVbiSettings */

    /* VBI data capture */
    struct {
        NEXUS_VideoInput_P_Link_Fifo cc;
        NEXUS_VideoInput_P_Link_Fifo gs;
        NEXUS_VideoInput_P_Link_Fifo tt;
#if NEXUS_VBI_SUPPORT
#define B_VBI_TT_LINES 20
        BVBI_TT_Line ttLines[B_VBI_TT_LINES];
#endif
        struct {
            uint16_t data;
            NEXUS_IsrCallbackHandle isrCallback;
        } wss;
        struct {
            uint32_t data;
            NEXUS_IsrCallbackHandle isrCallback;
        } cgms;
        struct {
            NEXUS_VpsData data;
            NEXUS_IsrCallbackHandle isrCallback;
        } vps;
    } vbi;

    struct {
        NEXUS_VideoInputCrcData *queue;
        unsigned size; /* num entries, not num bytes */
        unsigned rptr, wptr;
    } crc;

    NEXUS_IsrCallbackHandle sourceChangedCallback;
    BKNI_EventHandle sourceChangedEvent, checkFormatChangedEvent;
    NEXUS_EventCallbackHandle sourceChangedEventHandler, checkFormatChangedEventHandler;
    struct
    {
        NEXUS_HdmiDynamicRangeMasteringInfoFrame inputInfoFrame;
        BKNI_EventHandle inputInfoUpdatedEvent;
        NEXUS_EventCallbackHandle inputInfoUpdatedEventHandler;
    } drm;

    BVDC_Source_CallbackData vdcSourceCallbackData; /* keep a copy of the VDC source callback data. this is more reliable than BVDC_Source_GetInputStatus.
                                                       be sure to enter a critical section before reading multiple items from this structure. */
    BVDC_Heap_Handle vdcHeap;
    NEXUS_VideoInput_P_Iface iface;
    BLST_S_ENTRY(NEXUS_VideoInput_P_Link) link;
    unsigned ref_cnt; /* number of connections */
    struct { /* copy from NEXUS_VideoInput it's used to verify that association is still valid */
        NEXUS_VideoInputType type; /* type of video input */
        void *source; /* polymorphic pointer to the source, must be not NULL */
    } input_info;
    bool info_valid;
    union {
        BAVC_MFD_Picture mfd; /* only valid for video decoder inputs */
#if NEXUS_NUM_HDMI_INPUTS
        NEXUS_VideoHdmiInputSettings hdmi; /* only valid for hdmi inputs */
#endif
    } info;
    struct {
        unsigned index; /* index from VideoDecoder for the mosaic array. only applies to child link. */
        bool backendMosaic;
        NEXUS_VideoWindowHandle parentWindow[NEXUS_NUM_DISPLAYS]; /* only applies to parent link */
    } mosaic;

    NEXUS_Timebase timebase;
} NEXUS_VideoInput_P_Link;

typedef struct NEXUS_VideoInput_P_LinkData {
    BAVC_SourceId sourceId;
    NEXUS_HeapHandle heap;
    bool gfxSource;
    BVDC_Source_Handle sourceVdc;
    BVDC_Mode mtg;
} NEXUS_VideoInput_P_LinkData;

/* initializes mandatory fields of NEXUS_VideoInput_P_LinkData */
void NEXUS_VideoInput_P_LinkData_Init(NEXUS_VideoInput_P_LinkData *data, BAVC_SourceId sourceId);
void NEXUS_VideoInput_P_TriggerSourceChanged_isr(NEXUS_VideoInput_P_Link *link, bool sourcePending);
NEXUS_VideoInput_P_Link *NEXUS_VideoInput_P_CreateLink( NEXUS_VideoInput source, const NEXUS_VideoInput_P_LinkData *data, const NEXUS_VideoInput_P_Iface *iface );
void NEXUS_VideoInput_P_DestroyLink( NEXUS_VideoInput_P_Link *link );
NEXUS_VideoInput_P_Link *NEXUS_VideoInput_P_GetExisting( NEXUS_VideoInput input );
NEXUS_VideoInput_P_Link *NEXUS_VideoInput_P_Get( NEXUS_VideoInput input );
NEXUS_VideoInput_P_Link *NEXUS_VideoInput_P_GetForWindow(NEXUS_VideoInput input, NEXUS_VideoWindowHandle window);
BERR_Code NEXUS_VideoInput_P_Connect( NEXUS_VideoInput input );
void NEXUS_VideoInput_P_Disconnect( NEXUS_VideoInput input );
/* this function called after display settings were changed */
void NEXUS_Display_P_VideoInputDisplayUpdate(
    NEXUS_DisplayHandle display, /* only set for windowless connections */
    NEXUS_VideoWindowHandle window,
    const NEXUS_DisplaySettings *pSettings);

#if NEXUS_VBI_SUPPORT
void NEXUS_VideoInput_P_VbiData_isr(NEXUS_VideoInput videoInput, const BVBI_Field_Handle *pVbiData, const NEXUS_ClosedCaptionData *pData, unsigned vbiCount);
#endif

#if NEXUS_VBI_SUPPORT
/* tell the VideoInput to reset its VBI state based on its own settings and every display's VBI settings */
NEXUS_Error NEXUS_VideoInput_P_SetVbiState( NEXUS_VideoInput videoInput );
#endif

/* Notify a potential format change from a down module to display */
void NEXUS_VideoInput_P_CheckFormatChange_isr(void *pParam);

/* set vdc to force frame capture, for raaga encoder */
NEXUS_Error NEXUS_VideoInput_P_ForceFrameCapture(NEXUS_VideoInput videoInput, bool force);

/**
This section contains internal API's for specific video inputs.
These should be moved into separate _impl.h header files.
**/

NEXUS_VideoInput_P_Link *NEXUS_VideoInput_P_OpenHdmi(NEXUS_VideoInput input);
void NEXUS_VideoInput_P_SetHdmiInputStatus(NEXUS_VideoInput_P_Link *link);
void NEXUS_VideoInput_P_HdmiSourceCallback_isr(NEXUS_VideoInput_P_Link *link, const BVDC_Source_CallbackData *data);
NEXUS_VideoInput_P_Link *NEXUS_VideoInput_P_OpenCcir656(NEXUS_VideoInput input);
NEXUS_VideoInput_P_Link *NEXUS_VideoInput_P_OpenHdDviInput(NEXUS_VideoInput input);
NEXUS_VideoInput_P_Link *NEXUS_VideoInput_P_OpenDecoder(NEXUS_VideoInput input, NEXUS_VideoInput_P_Link *mosaicParent, NEXUS_VideoWindowHandle window);

void
NEXUS_VideoInput_P_DecoderDataReady_isr(void *input_, const BAVC_MFD_Picture *pPicture);
bool nexus_p_input_is_mtg(NEXUS_VideoInput_P_Link *link);

#endif

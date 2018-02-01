/***************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 **************************************************************************/
#ifndef NEXUS_DISPLAY_MODULE_H__
#define NEXUS_DISPLAY_MODULE_H__

#include "nexus_base.h"
#include "nexus_platform_features.h"

#if NEXUS_HAS_GRAPHICS2D
#include "bchp_gfd_0.h"
#ifdef BCHP_GFD_0_CTRL_BSTC_ENABLE_ON
#define NEXUS_AUTO_GRAPHICS_COMPRESSION 1
#include "nexus_graphics2d.h"
#endif
#endif

#ifndef NEXUS_VBI_ENCODER_QUEUE_SIZE
/* override NEXUS_VBI_ENCODER_QUEUE_SIZE in nexus_platform_features.h */
#define NEXUS_VBI_ENCODER_QUEUE_SIZE 32
#endif

/**
PEP is picture enhancement processor
    PEP has a CAB (color adjustment block) and LAB (luma adjustment block)
**/
#if BCHP_CHIP == 7400 || BCHP_CHIP == 7420 || BCHP_CHIP == 7335 || BCHP_CHIP == 7325 || \
BCHP_CHIP==7342 || BCHP_CHIP==7340 || BCHP_CHIP==7468 || BCHP_CHIP ==7125 || BCHP_CHIP ==7445
#define NEXUS_HAS_PEP 1
#endif

#define NEXUS_HAS_MOSAIC_MODE 1

#include "nexus_display_thunks.h"
#include "nexus_display.h"
#include "nexus_display_private.h"
#include "nexus_display_init.h"
#include "priv/nexus_display_standby_priv.h"
#include "priv/nexus_display_priv.h"
#include "nexus_display_vbi.h"
#include "nexus_video_window.h"
#include "nexus_video_window_tune.h"
#include "nexus_video_input_crc.h"
#include "priv/nexus_video_window_priv.h"
#include "priv/nexus_core.h"
#include "nexus_core_utils.h"
#include "nexus_picture_ctrl.h"
#include "nexus_video_adj.h"
#include "nexus_display_extensions.h"

#if NEXUS_HAS_MOSAIC_MODE
    #include "nexus_mosaic_display.h"
#endif

#include "nexus_video_output.h"
#include "nexus_video_input.h"
#include "priv/nexus_video_input_priv.h"
#include "nexus_video_input_vbi.h"
#include "nexus_component_output.h"
#include "nexus_svideo_output.h"
#include "nexus_composite_output.h"
#include "nexus_ccir656_output.h"
#include "nexus_video_output.h"
#include "nexus_video_image_input.h"
#include "nexus_video_hdmi_input.h"
/* all settops expose the HD_DVI interface, even if HW does not support */
#include "nexus_hddvi_input.h"
#include "nexus_ccir656_input.h"

#include "blst_list.h"
#include "bvdc.h"
#include "bvdc_dbg.h"
#include "bvdc_test.h"
#if NEXUS_VBI_SUPPORT
#include "bvbi.h"
#include "bvbilib.h"
#endif
#include "priv/nexus_core_video.h"
#if NEXUS_HAS_VIDEO_DECODER
#include "priv/nexus_video_decoder_priv.h"
#endif

#if NEXUS_HAS_VIDEO_ENCODER
#include "bxudlib.h"
#endif

#include "nexus_hdmi_types.h"
#include "nexus_display_impl.h"
#include "nexus_video_window_impl.h"
#include "nexus_video_output_impl.h"
#include "nexus_video_input_impl.h"

#include "blst_squeue.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif

#if NEXUS_DBV_SUPPORT
#include "nexus_display_dbv_impl.h"
#endif

#if NEXUS_DISPLAY_EXTENSION_DYNRNG
#include "nexus_display_dynrng.h"
#endif

#if !defined NEXUS_HAS_HDMI_INPUT
#undef NEXUS_NUM_HDMI_INPUTS
#endif
#if !defined NEXUS_HAS_VIDEO_DECODER
#undef NEXUS_NUM_VIDEO_DECODERS
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum nexus_vbi_resources
{
    nexus_vbi_resource_vec_int,
    nexus_vbi_resource_vec_bypass_int,
    nexus_vbi_resources_cgmse,
    nexus_vbi_resources_max
};

typedef struct NEXUS_DisplayModule_State {
    BVDC_Handle vdc;
    BRDC_Handle rdc;
    NEXUS_DisplayCapabilities cap;
    BVDC_Capabilities vdcCapabilities;
    NEXUS_DisplayModuleSettings moduleSettings;
    NEXUS_HeapHandle heap; /* main heap used in BVDC_Open */
    NEXUS_DisplayModuleDependencies modules;
    NEXUS_DisplayHandle displays[NEXUS_NUM_DISPLAYS];
    BLST_S_HEAD(NEXUS_DisplayModule_P_Inputs, NEXUS_VideoInput_P_Link) inputs;
    NEXUS_DisplayUpdateMode updateMode;
    bool lastUpdateFailed;
    NEXUS_DisplayHandle displayDrivingVideoDecoder;
    NEXUS_VideoOutput requiredOutput;
    bool requiredOutputSystem;
    BTMR_TimerHandle tmr;

    /* mapping of nexus to vdc heap */
#define MAX_VDC_HEAPS 8
    struct {
        NEXUS_HeapHandle nexusHeap;
        BVDC_Heap_Handle vdcHeap;
        NEXUS_DisplayHeapSettings settings;
        unsigned refcnt;
    } vdcHeapMap[MAX_VDC_HEAPS];

    struct {
#if NEXUS_NUM_COMPONENT_OUTPUTS
        struct NEXUS_ComponentOutput component[NEXUS_NUM_COMPONENT_OUTPUTS];
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
        struct NEXUS_CompositeOutput composite[NEXUS_NUM_COMPOSITE_OUTPUTS];
#endif
#if NEXUS_NUM_SVIDEO_OUTPUTS
        struct NEXUS_SvideoOutput svideo[NEXUS_NUM_SVIDEO_OUTPUTS];
#endif
#if NEXUS_NUM_656_OUTPUTS
        struct NEXUS_Ccir656Output ccir656[NEXUS_NUM_656_OUTPUTS];
#endif
        int dummy;
    } outputs;
#if NEXUS_VBI_SUPPORT
    BVBI_Handle vbi;
    BVBIlib_Handle vbilib;
    BVBIlib_List_Handle vbilist;
    BVBI_TT_Line *ttLines; /* avoided large stack */
#endif

#if NEXUS_NUM_MOSAIC_DECODES
/* NEXUS_NUM_MOSAIC_DECODE_SETS corresponds to the # of AVD cores that can be put into mosaic mode. */
#define NEXUS_NUM_MOSAIC_DECODE_SETS 2
    struct {
        NEXUS_VideoInputObject input;
        unsigned parentIndex; /* cache the lookup of the videodecoder's parentIndex */
    } mosaicInput[NEXUS_NUM_MOSAIC_DECODE_SETS];
#endif

    bool pqDisabled; /* export pq_disabled=y provides a divide-and-conquer method of isolating PQ-related problems. */
    bool dnrAlloc[BAVC_SourceId_eMpegMax+1]; /* allocation of DNR resources to fail earlier than BVDC_ApplyChanges */

    /* instead of allocating large data structures on the stack, they are allocated  in the static data (it's safe since module is serialized)
       data is kept in union with entry per function and we use 'ccokie' to verify that union was not stoled */
    union {
#if NEXUS_HAS_HDMI_OUTPUT
        struct {
            NEXUS_HdmiOutputStatus hdmiOutputStatus;
        } NEXUS_VideoOutput_P_SetHdmiFormat;
#endif
        unsigned unused;
    } functionData;

    bool verifyTimebase;
} NEXUS_DisplayModule_State;

struct NEXUS_DisplayGraphics {
    BVDC_Source_Handle source;
    BVDC_Window_Handle windowVdc;/* it indicates  that graphics is active, e.g. it both enabled in settings and frambuffer was assigned */
    BVDC_Window_Capabilities windowCaps;
    const BPXL_Plane *queuedPlane; /* surface queued for display in hardware, used only as reference  */
    NEXUS_GraphicsFramebuffer3D frameBuffer3D;
#if NEXUS_AUTO_GRAPHICS_COMPRESSION
    struct {
        struct {
            NEXUS_SurfaceHandle uncompressed, compressed;
#define NEXUS_MAX_COMPRESSED_FRAMEBUFFERS 4
            unsigned cnt;
        } cache[NEXUS_MAX_COMPRESSED_FRAMEBUFFERS];
        unsigned cnt;
        NEXUS_Graphics2DHandle gfx;
        NEXUS_CallbackHandler checkpointCallback;
        bool warning, checkpoint;
    } compression;
#endif
    uint16_t frameBufferWidth; /* width of the frame buffer */
    uint16_t frameBufferHeight; /* height of the frame buffer  */
    NEXUS_PixelFormat frameBufferPixelFormat; /* pixelFormat of the frame buffer */
    NEXUS_IsrCallbackHandle frameBufferCallback;
    NEXUS_GraphicsSettings cfg;
    NEXUS_ColorMatrix colorMatrix;
    bool colorMatrixSet;
    unsigned validCount;
    NEXUS_GraphicsColorSettings colorSettings;
    bool secure;
#if NEXUS_DISPLAY_EXTENSION_DYNRNG
    struct
    {
        NEXUS_DynamicRangeProcessingSettings settings;
        BVDC_Test_Window_ForceCfcConfig cfcConfig;
    } dynrng;
#endif
};

typedef struct NEXUS_Display_P_Image {
    BLST_SQ_ENTRY(NEXUS_Display_P_Image) link;
#if NEXUS_DISPLAY_USE_VIP
    BAVC_EncodePictureBuffer picture;
#else
    BMMA_Block_Handle hImage;
    unsigned offset;
    unsigned picId;
#endif
} NEXUS_Display_P_Image;
BLST_SQ_HEAD(NEXUS_Display_P_ImageQueue, NEXUS_Display_P_Image );

struct NEXUS_Display {
    NEXUS_OBJECT(NEXUS_Display);
    BVDC_Compositor_Handle compositor;
    BVDC_Display_Handle displayVdc;
    BFMT_VideoInfo *customFormatInfo;
#if NEXUS_VBI_SUPPORT
    struct {
        BVBI_Encode_Handle enc_core; /* must be NULL if not created */
        BVBIlib_Encode_Handle enc; /* this is VBI encode handle destination for VBI data */
        BINT_CallbackHandle tf_isr, bf_isr;
        bool progressive; /* need fast way in isr to know if display is progressive and modify tf/bf_isr field parameter */
        NEXUS_DisplayVbiSettings settings;
        BVBI_AMOL_Type amolType;

        struct {
            bool wssSet;
            uint16_t wssData;

            bool cgmsTopSet;
            bool cgmsBottomSet;
            uint32_t cgmsData;

            bool cgmsBTopSet;
            bool cgmsBBottomSet;
            BVBI_CGMSB_Datum cgmsBData;

            bool vpsSet;
            BVBI_VPSData vpsData;
        } pending;
        bool enabled[nexus_vbi_resources_max];
    } vbi;
#endif
    struct {
        NEXUS_DisplayMacrovisionType type;
        bool tableSet;
        NEXUS_DisplayMacrovisionTables table;
    } macrovision;
    unsigned index;
    NEXUS_DisplaySettings cfg;
    NEXUS_DisplayStatus status;
    BVDC_DacConnectionState dacStatus[BVDC_MAX_DACS];
    NEXUS_PictureCtrlColorClipSettings colorClipSettings;
    NEXUS_DisplayStgSettings stgSettings;
    unsigned stgIndex;
    NEXUS_DisplayTimingGenerator timingGenerator; /* from NEXUS_DisplaySettings.timingGenerator, but with eAuto resolved */

    struct NEXUS_VideoWindow windows[NEXUS_NUM_VIDEO_WINDOWS];
    struct NEXUS_DisplayGraphics graphics;
    BLST_D_HEAD(NEXUS_Display_P_Outputs, NEXUS_VideoOutput_P_Link) outputs;
    NEXUS_Rect displayRect;
    struct
    {
#if NEXUS_NUM_HDMI_OUTPUTS
        NEXUS_CallbackHandler outputNotifyDisplay;
        /* NEXUS_HdmiOutputHandle */ void *outputNotify;
        void (*rateChangeCb_isr)(NEXUS_DisplayHandle display, void *pParam);
        void (*vsync_isr)(void *pParam);
        void *pCbParam;
        BAVC_VdcDisplay_Info rateInfo;
        bool rateInfoValid;
        uint32_t vdcIndex;
        bool forceFormatChange;
        struct {
            BKNI_EventHandle event;
            NEXUS_EventCallbackHandle handler;
            NEXUS_HdmiDynamicRangeMasteringInfoFrame infoFrame;
            bool smdValid;
        } hdrInfoChange;
#endif
        NEXUS_VideoFormat outputFormat;
    } hdmi;

    struct
    {
        NEXUS_DisplayPrivateSettings settings;
        NEXUS_DisplayPrivateStatus status;
        NEXUS_TaskCallbackHandle hdrInfoChangedCallback;
    } private;

#if NEXUS_NUM_HDMI_DVO
    struct
    {
        void (*rateChangeCb_isr)(NEXUS_DisplayHandle display, void *pParam);
        void *pCbParam;
        BAVC_VdcDisplay_Info rateInfo;
        bool rateInfoValid;
    } hdmiDvo;
#endif

    bool formatChanged;
    struct {
        BINT_CallbackHandle intCallback[3]; /* top, bot, frame */
        NEXUS_IsrCallbackHandle isrCallback;
        NEXUS_CallbackDesc desc; /* from NEXUS_Display_SetVsyncCallback, not SetSettings */
        NEXUS_VideoBufferType lastVsyncType;
    } vsyncCallback;
    struct {
        NEXUS_DisplayCrcData *queue;
        unsigned size; /* num entries, not num bytes */
        unsigned rptr, wptr;
    } crc;
    struct {
        BKNI_EventHandle event;
        NEXUS_EventCallbackHandle handler;
    } refreshRate;

#if NEXUS_HAS_VIDEO_ENCODER
    BXUDlib_Handle hXud;
    bool encodeUserData;
    NEXUS_VideoInputHandle xudSource;
    BXUDlib_Settings userDataEncodeSettings;
#if NEXUS_P_USE_PROLOGUE_BUFFER
    struct {
#if NEXUS_DISPLAY_USE_VIP
        NEXUS_Error (*enqueueCb_isr)(void * context, BAVC_EncodePictureBuffer *picture);
        NEXUS_Error (*dequeueCb_isr)(void * context, BAVC_EncodePictureBuffer *picture);
#else
        NEXUS_Error (*enqueueCb_isr)(void * context, NEXUS_DisplayCapturedImage *image);
        NEXUS_Error (*dequeueCb_isr)(void * context, NEXUS_DisplayCapturedImage *image);
#endif
        void *context;
        unsigned framesEnqueued;
        unsigned dropRate;
        uint32_t *buf;
        unsigned numOrigBufs;
        struct NEXUS_Display_P_ImageQueue free;
        struct NEXUS_Display_P_ImageQueue queued;
#if NEXUS_DISPLAY_USE_VIP
        struct NEXUS_Display_P_ImageQueue captured; /* to hold buffer in case encoder back pressure */
        /* separate queues for associated buffers as they may be returned by encoder at different timing from orignal luma buffers */
        unsigned numDecimBufs, numShiftedBufs; /* associated buffers counts might be different from original luma buffers */
        struct NEXUS_Display_P_ImageQueue freeDecim1v, freeDecim2v, freeChroma, freeShifted;
        struct NEXUS_Display_P_ImageQueue queuedDecim1v, queuedDecim2v, queuedChroma, queuedShifted;
#endif
        bool callbackEnabled;
        NEXUS_VideoWindowHandle window;
    } encoder;
#endif
#endif
    unsigned lastVsyncTime;
#if NEXUS_DBV_SUPPORT
    NEXUS_DisplayDbvState dbv;
#endif
};

/*
Display module global variables
**/
extern NEXUS_DisplayModule_State g_NEXUS_DisplayModule_State;
extern NEXUS_ModuleHandle g_NEXUS_displayModuleHandle;
extern const char *g_videoInputStr[NEXUS_VideoInputType_eMax];
extern const char *g_videoOutputStr[NEXUS_VideoOutputType_eMax];

/**
Conversion and helper functions
**/
BERR_Code NEXUS_P_DisplayTriState_ToMagnum(NEXUS_TristateEnable tristateEnable, BVDC_Mode *mtristateEnable);
BERR_Code NEXUS_P_DisplayAspectRatio_ToMagnum(NEXUS_DisplayAspectRatio aspectRatio, NEXUS_VideoFormat videoFormat, BFMT_AspectRatio *maspectRatio);
bool NEXUS_P_Display_RectEqual(const NEXUS_Rect *r1,  const NEXUS_Rect *r2);

BERR_Code NEXUS_Display_P_InitGraphics(NEXUS_DisplayHandle display);
void NEXUS_Display_P_UninitGraphics(NEXUS_DisplayHandle display);
void NEXUS_Display_P_ResetGraphics(NEXUS_DisplayHandle display);
void NEXUS_Display_P_DestroyGraphicsSource(NEXUS_DisplayHandle display);

NEXUS_Error NEXUS_Display_P_GetScalerRect(const NEXUS_VideoWindowSettings  *pSettings, NEXUS_Rect *pRect);

/* Performs apply changes after checking updateMode */
NEXUS_Error NEXUS_Display_P_ApplyChanges(void);

#if NEXUS_VBI_SUPPORT
/**
Display VBI functions
**/
BERR_Code NEXUS_Display_P_ConnectVbi(NEXUS_DisplayHandle display);
void NEXUS_Display_P_DisconnectVbi(NEXUS_DisplayHandle display);
void NEXUS_Display_P_DisableVbi(NEXUS_DisplayHandle display);
bool nexus_display_p_vbi_available(enum nexus_vbi_resources res);
BERR_Code NEXUS_Display_P_EnableVbi(NEXUS_DisplayHandle display, NEXUS_VideoFormat format);
BERR_Code NEXUS_Display_P_VbiData_isr(NEXUS_DisplayHandle display, BVBI_Field_Handle vbiData);
#endif
void nexus_p_check_macrovision( NEXUS_DisplayHandle display, NEXUS_VideoFormat videoFormat);

BVDC_Heap_Handle NEXUS_Display_P_CreateHeap(NEXUS_HeapHandle heap);
void NEXUS_Display_P_DestroyHeap(BVDC_Heap_Handle vdcHeap);

/* Find the window that this input is connected to */
NEXUS_VideoWindowHandle NEXUS_Display_P_FindWindow(NEXUS_VideoInput input);

/* Create VideoInputLink for the videoImage input */
NEXUS_VideoInput_P_Link *NEXUS_VideoImageInput_P_OpenInput(NEXUS_VideoInput input);
void NEXUS_VideoImageInput_P_UpdateDisplayInformation(NEXUS_VideoImageInputHandle imageInput, const NEXUS_DisplaySettings *pSettings);

#if NEXUS_NUM_DISPLAYS > 1
NEXUS_Error NEXUS_Display_P_Align( NEXUS_DisplayHandle display, NEXUS_DisplayHandle target );
#endif

#define NEXUS_MODULE_SELF g_NEXUS_displayModuleHandle

int nexus_display_p_init_rdccapture(void);
void nexus_display_p_uninit_rdccapture(void);
void nexus_display_p_disconnect_outputs(NEXUS_DisplayHandle display, const NEXUS_DisplaySettings *pSettings, NEXUS_VideoFormat hdmiOutputFormat);
void nexus_display_p_connect_outputs(NEXUS_DisplayHandle display);
void nexus_videoadj_p_dnr_dealloc(BAVC_SourceId id);

bool NEXUS_Display_P_HasOutput_isr(NEXUS_DisplayHandle display, NEXUS_VideoOutputType type);

#if BVDC_BUF_LOG && NEXUS_BASE_OS_linuxuser
void NEXUS_Display_P_BufLogCapture(void);
int nexus_display_p_init_buflogcapture(void);
void nexus_display_p_uninit_buflogcapture(void);
#endif

#if NEXUS_DBV_SUPPORT
NEXUS_Error NEXUS_Display_P_DbvFormatCheck(NEXUS_DisplayHandle display, NEXUS_VideoFormat format);
void NEXUS_Display_P_DbvUpdateDisplayHdmiSettings(NEXUS_DisplayHandle display, NEXUS_HdmiOutputHandle hdmiOutput, BVDC_Display_HdmiSettings * pDisplayHdmiSettings, BAVC_HDMI_BitsPerPixel * pColorDepth);
#endif

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_DISPLAY_MODULE_H__ */

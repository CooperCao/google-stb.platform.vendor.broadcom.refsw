/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BSETTOP_TYPES_H__
#define BSETTOP_TYPES_H__

#include "bstd_defs.h"

#include "bfile_types.h"

#include "bmedia_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*=*************************
Collection of handles, structures and other types that are used across
the Settop API.

Types that are only used in one module are defined in that module's
header file.
****************************/

/**
* Summary:
* Private definition using in B_ID macro.
* Description:
* The purpose of BID_MAX_INDEX is to make sure that you only pass an
* integer into B_ID() and it enables B_ID_IS_INDEX() to differentiate
* between pointers and integers. The idea is that a pointer will never have
* a value less than BID_MAX_INDEX and that you will never need an index
* >= BID_MAX_INDEX.
* NOTE: it's not actually MAX as much as TOTAL or FIRST_INVALID_INDEX.
**/
#define BID_MAX_INDEX 200

/*
Summary:
    Create an object_id from integer for selecting resources to open.
Description:
    It increments the id internally so that B_ID(0) is valid but different from
    a NULL bobject_t.
*/
#define B_ID(x) (((x)>=BID_MAX_INDEX||(((int)x)<0)) ? NULL: ((bobject_t)((x)+1)))

/*
Summary:
    Handle to select a particular resource when opening.
Description:
    The reason we use a typedef instead of an integer index is to allow
    for registry-based implementations in the future. For today, B_ID(index) is
    the only supported method of obtaining bobject_t's.
*/
typedef struct bobject_id_impl *bobject_t;

/*
Summary:
    Standard error code for settop api.
Description:
    Every settop api function returns either:
    - For open functions, a handle which is NULL on error.
    - For close, structure init and never-failing get functions have void return type.
    - For all other functions, a bresult which is non-zero (!= b_ok) on error.
*/
typedef enum bresult {
    b_ok=0,
    berr_out_of_memory=1,
    berr_invalid_parameter=2,
    berr_not_supported=3,
    berr_not_available=4,
    berr_busy=5,
    berr_external_error=6,
    berr_invalid_state=7,
    berr_timeout=8
} bresult;


/**
Summary:
Generic callback from the Settop API into user code.
Description:
Callbacks are used for notification to the user.

There are some general rules for the use of callbacks.

1) You should never block or do extensive work in a callback. They should always
execute quickly. If you don't execute quickly, you may impact the performance of
the entire system.

2) You can make calls back into the Settop API, but you should make a limited number
of calls. In general, you should query state and not change state.
The exact calls recommended or allowed depends on the particular callback
and it will be documented with that callback. Any calls back into the Settop API from
a callback introduce re-entrancy, and it's hard to predict what will happen if any
call is made.

The value of context is always user-defined. It is specified when the user specifies the callback
function. The user is responsible to make sure the context pointer allows
the user program to access whatever handles or state data is needed in the callback.
**/
typedef void (*bsettop_callback)(void *context);

/*
Summary:
    Handle returned by btuner_open().
*/
typedef struct btuner *btuner_t;

/*
Summary:
    Handle returned by btuner_linein_open().
*/
typedef struct btuner_linein *btuner_linein_t;

/*
Summary:
    Settop rectangular geometry object. Used by bdisplay and bgraphics.
*/
typedef struct bsettop_rect {
    int x;
    int y;
    unsigned width;
    unsigned height;
} bsettop_rect;

/*
Summary:
    bband_t represents a digital transport input band. It encompasses both the transport
    input band and the transport type ( the type of data current on the input band).

Description:
    A careful distinction should be made between input band and parser band. An input band
    is an external transport input to the chip. This is often hard-coded to certain frontend
    configurations per platform.

    There is a dynamic mapping between input band and parser band. the input band is often mapped
    directly to the parser band of the same number, but this is not required. An input band
    can be mapped to more than one parser band. This is done by making multiple calls to
    bstream_open with the same bband_t.
*/
typedef struct bband *bband_t;


/*
Summary:
    A stream is either a digital or analog program which can be decoded or recorded.
Description:
    Streams are produced by one of the following:
        - tuning linein or rf (btuner_tune_rf or btuner_tune_linein)
        - tuning digital and selecting mpeg params (btuner_tune_qpsk/qam and bstream_set_mpeg)
        - starting playback (bplay_start)
        - direct transport input (bstream_set)
*/
typedef struct bstream *bstream_t;

/*
Summary:
    Handle returned by bplayback_open().
*/
typedef struct bplayback *bplayback_t;

/*
Summary:
    Handle returned by bplayfile_file_open.
Description:
  This is an object to retreive data from the media.
  This object has support for two independent data flows.
  Usually it implemented by read(2) and seek(2) functions
*/
typedef struct bplayback_file *bplayback_file_t;

/*
Summary:
    Handle returned by brecord_open().
*/
typedef struct brecord *brecord_t;

/*
Summary:
    Handle returned by bplayback_ip_open().
*/
typedef struct bplayback_ip *bplayback_ip_t;


/*
Summary:
    Handle returned by brecord_ip_open().
*/
typedef struct brecord_ip *brecord_ip_t;



/*
Summary:
  Handle returned by brecord_file_open().
Description:
  This is an object to store date to the media. Usually it implemented by write(2) function.
  This object has support for two independent data flows.
*/
typedef struct brecord_file *brecord_file_t;

/*
Summary:
    Handle returned by bdecode_open().
*/
typedef struct bdecode *bdecode_t;

/*
Summary:
    Handle returned by bdecode_window_open().
*/
typedef struct bdecode_window *bdecode_window_t;

/*
Summary:
    Handle returned by bdecode_detach_audio().
*/
typedef struct baudio_decode *baudio_decode_t;

/*
Summary:
    Handle returned by bdecode_detach_video().
*/
typedef struct bvideo_decode *bvideo_decode_t;

/*
Summary:
    Handle returned by bdisplay_open().
*/
typedef struct bdisplay *bdisplay_t;

/*
Summary:
    Handle returned by buser_input_open().
*/
typedef struct buser_input *buser_input_t;

/*
Summary:
    Handle returned by buser_output_open().
*/
typedef struct buser_output *buser_output_t;

/*
Summary:
  bencode_t is a handle that refers to an encoder chip.
Description:
  An encoder may have 1 or more channels.
  A system may have 0, 1 or more encoders.
*/
typedef struct bencode *bencode_t;


/*
Summary:
    Handle returned by btransode_open().
*/
typedef struct btranscode *btranscode_t;


/**
Summary:
Handle for the playpump api.
**/
typedef struct bplaypump *bplaypump_t;

/*
Summary:
    Graphics engine handle returned by bgraphics_open.
*/
typedef struct bgraphics *bgraphics_t;

/*
Summary:
    Surface handle returned by bgraphics_create_surface.
Description:
    Each graphics engine can have many surfaces.
*/
typedef struct bsurface *bsurface_t;

/*
Summary:
Maximum baudio_level value.
*/
#define BAUDIO_LEVEL_MAX 100
/*
Summary:
Minimum baudio_level value.
*/
#define BAUDIO_LEVEL_MIN -100

/*
Summary:
    Audio level used in baudio_volume.
Description:
    Max value is BAUDIO_LEVEL_MAX. Min value is BAUDIO_LEVEL_MIN.
*/
typedef int baudio_level;

/*
Summary:
   Data structure for audio volume.
Description:
    Volume is not measured in units of dB. It is simply a range between the min and max
    audio level of the device. Min is always a value very close to mute.
    If the device has attenuation only, max will be no attenuation.
    If the device has amplication, max will be maximum amplification.

    The baudio_level values can range from BAUDIO_LEVEL_MIN (the softest) to
    BAUDIO_LEVEL_MAX (the loudest). The baudio_level is usually converted to the dB
    range of the device by a linear conversion from MIN/MAX to the min/max dB of the device.

    If you want a specific dB setting on a device, you may have to modify the settop api
    implementation.

    The actual audio level which is produced on the output will vary based on the hardware
    and the input source. Your application will have to be tuned if you want to normalize
    the output levels.

    If some device supports only 'mono' type of control, user shall expect the following behavior:
      - API will return structure where left volume is equal to the right volume
      - when API receives structure from the user, only value of the left volume is used, right is ignored
*/
typedef struct baudio_volume {
    baudio_level left;
    baudio_level right;
    bool muted;
} baudio_volume;


/*
Summary:
    Video formats
*/
typedef enum bvideo_format {
/*
Developers -- Please be very careful when modifying ANYTHING in this enum.  There are
              multiple places in the code where the order and number of elements in this
              enum define array structures.  Any changes here must be made in those locations
              as well.  Search for bvideo_format_count to find locations.
*/
    bvideo_format_ntsc,              /* NTSC Interlaced */
    bvideo_format_ntsc_japan,        /* Japan NTSC, no pedestal level */
    bvideo_format_pal_m,             /* PAL Brazil */
    bvideo_format_pal_n,             /* PAL_N */
    bvideo_format_pal_nc,            /* PAL_N, Argentina */
    bvideo_format_pal_b,            /* Australia */
    bvideo_format_pal_b1,           /* Hungary */
    bvideo_format_pal_d,            /* China */
    bvideo_format_pal_d1,           /* Poland */
    bvideo_format_pal_g,            /* Europe. Same as bvideo_format_pal. */
    bvideo_format_pal = bvideo_format_pal_g,     /* PAL Europe */
    bvideo_format_pal_h,            /* Europe */
    bvideo_format_pal_k,            /* Europe */
    bvideo_format_pal_i,            /* U.K. */
    bvideo_format_secam,             /* SECAM III B6 */
    bvideo_format_480p,              /* NTSC Progressive (27Mhz) */
    bvideo_format_576p,              /* HD PAL Progressive 50Hz for Australia */
    bvideo_format_1080i,             /* HD 1080 Interlaced */
    bvideo_format_1080i_50hz,        /* European 50Hz HD 1080 */
    bvideo_format_1080p,             /* HD 1080 Progressive, 60Hz */
    bvideo_format_1080p_24hz,        /* HD 1080 Progressive, 24Hz */
    bvideo_format_1080p_25hz,        /* HD 1080 Progressive, 25Hz */
    bvideo_format_1080p_30hz,        /* HD 1080 Progressive, 30Hz */
    bvideo_format_1080p_50hz,        /* HD 1080 Progressive, 50Hz */
    bvideo_format_1250i_50hz,        /* HD 1250 Interlaced, 50Hz */
    bvideo_format_720p,              /* HD 720 Progressive */
    bvideo_format_720p_50hz,         /* HD 720p 50Hz for Australia */
    bvideo_format_720p_24hz,         /* HD 720p 24Hz */
    bvideo_format_vesa,              /* PC monitor. Requires width, height and refresh rate parameters in bdisplay_settings. */
    bvideo_format_3840x2160p_24hz,   /* UHD 3840x2160 24Hz */
    bvideo_format_3840x2160p_25hz,   /* UHD 3840x2160 25Hz */
    bvideo_format_3840x2160p_30hz,   /* UHD 3840x2160 30Hz */
    bvideo_format_4096x2160p_24hz,   /* UHD 4096x2160 24Hz */
    bvideo_format_count              /* Total number of video formats -- must be last */
} bvideo_format;


#define BSETTOP_MAX_VIDEO_CODECS 10      /* PLEASE UPDATE THIS IF YOU ADD VIDEO CODECS!!! */

/*
Summary:
Video frame rate (frames per second)
Description:
This is used for both display and source frame rates.
*/
typedef enum bvideo_frame_rate {
    bvideo_frame_rate_unknown =    0,
    bvideo_frame_rate_7_493   =  749, /* 7.493 */
    bvideo_frame_rate_14_985  = 1498, /* 14.985 */
    bvideo_frame_rate_23_976  = 2397,
    bvideo_frame_rate_24      = 2400,
    bvideo_frame_rate_25      = 2500,
    bvideo_frame_rate_29_97   = 2997,
    bvideo_frame_rate_30      = 3000,
    bvideo_frame_rate_50      = 5000,
    bvideo_frame_rate_59_94   = 5994,
    bvideo_frame_rate_60      = 6000
} bvideo_frame_rate;

/**
Summary:
Test if the audio format is MPEG 1/2 layer 1/2 audio.

Description:
Our decoders handle MPEG 1/2 layer 1/2 using a single data type.
For this reason, we need some single test for all these formats in PSI information.
**/
#define BAUDIO_FORMAT_MPEG(FORMAT) ((FORMAT) == 0 || (FORMAT) == 3 || (FORMAT) == 4)

/*
Summary:
    BTSC program selection
*/
typedef enum boutput_rf_btsc_mode {
    boutput_rf_btsc_mode_mono,
    boutput_rf_btsc_mode_stereo,
    boutput_rf_btsc_mode_sap,
    boutput_rf_btsc_mode_sapmono
} boutput_rf_btsc_mode;

/* Summary:
    This is a enum lists the type of socket addresses that can be used in bsocket_params structure.
   Description:
      This enum will specify the structure that will be used in bsocket_params.addr union.
*/
typedef enum bsocket_address_type {
    bsocket_address_type_ipv4,
    bsocket_address_type_url
} bsocket_address_type;

/*
Summary:
    This is a common structure to pass socket params to any settop api modules that may
    need it.
Description:
    You must specify what bscoket_address_type is contained in the particular bsocket_params
    struct. If socket address type is bsocket_address_type_ipv4 then the ipv4 structure will
    be used. If bsocket_address_type_url is specified then char url[128] will be used.
*/
typedef struct bsocket_params
{
   bsocket_address_type addr_type;
   union
   {
      char url[128];
      struct
      {
         uint8_t host[4];
         uint16_t port;
      } ipv4;
      /* Eventually add ipv6, ... */
   } addr;
   bool timestamp_enabled; /* indicates if this socket is sending or receiving MPEG2 TS packets w/ timestamps */
} bsocket_params;


/*
Summary:
    HDMI Audio Selection
*/
typedef enum boutput_hdmi_audio_mode
{
    boutput_hdmi_audio_mode_pcm,        /* Stereo PCM */
    boutput_hdmi_audio_mode_pcm_2ch=boutput_hdmi_audio_mode_pcm,
    boutput_hdmi_audio_mode_compressed, /* AC3 Compressed */
    boutput_hdmi_audio_mode_pcm_6ch,    /* 6 channel PCM */
    boutput_hdmi_audio_mode_ac3_plus,   /* Dolby Digital Plus (AC3+ or DDP) audio */
    boutput_hdmi_audio_mode_dts,        /* DTS */
    boutput_hdmi_audio_mode_dts_hd,     /* DTS HD, decodes only DTS part of DTS-HD stream */
    boutput_hdmi_audio_mode_dts_legacy, /* DTS HD, legacy mode (14 bit), uses legacy frame-sync */
    boutput_hdmi_audio_mode_count       /* Count of modes */
} boutput_hdmi_audio_mode;

/*
Summary:
    HDMI Rx EDID Info
    This is structure for storing information from HDMI Rx EDID
*/
typedef struct boutput_hdmi_rx_edid_info
{
    /* Please HDMI Specification 1.3a under Section 8.6 for details of Physical Addres.
     * Each node (A..D) represent an attached HDMI device on HDMI device tree. */
    uint8_t phys_addr_A;                    /* Physical Address for HDMI node A */
    uint8_t phys_addr_B;                    /* Physical Address for HDMI node B */
    uint8_t phys_addr_C;                    /* Physical Address for HDMI node C */
    uint8_t phys_addr_D;                    /* Physical Address for HDMI node D */
} boutput_hdmi_rx_edid_info;

#ifdef __cplusplus
}
#endif


#endif /* BSETTOP_TYPES_H__*/


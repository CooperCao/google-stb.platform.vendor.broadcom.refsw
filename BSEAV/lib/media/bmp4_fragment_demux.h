/***************************************************************************
 *     Copyright (c) 2007-2011, Broadcom Corporation
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
 * MP4 container parser library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef BMP4_FRAGMENT_DEMUX_H__
#define BMP4_FRAGMENT_DEMUX_H__

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct bmp4_fragment_demux *bmp4_fragment_demux_t;
typedef struct bmp4_fragment_stream *bmp4_fragment_stream_t;
typedef struct bmp4_fragment_demux_cfg  {
    batom_factory_t factory;
	balloc_iface_t alloc;
    void *user_context;
    void (*error)(void *user_context);
    void (*traf_box)(void *user_context, const batom_cursor *data, size_t payload_len);
    void (*sample)(void *user_context, uint32_t track_ID, unsigned sample_no, const batom_cursor *data, size_t payload_len);
    unsigned burst_frames;
} bmp4_fragment_demux_cfg;

typedef struct bmp4_fragment_stream_cfg {
    int unused;
} bmp4_fragment_stream_cfg;

void bmp4_fragment_demux_default_cfg(bmp4_fragment_demux_cfg *cfg);

bmp4_fragment_demux_t bmp4_fragment_demux_create(const bmp4_fragment_demux_cfg *config);

void bmp4_fragment_demux_destroy(bmp4_fragment_demux_t demux);

void bmp4_fragment_demux_reset(bmp4_fragment_demux_t demux);

size_t bmp4_fragment_demux_feed(bmp4_fragment_demux_t demux, batom_pipe_t pipe);

int bmp4_fragment_demux_seek(bmp4_fragment_demux_t demux, uint64_t off);

void bmp4_fragment_demux_flush(bmp4_fragment_demux_t demux);

void bmp4_fragment_demux_get_default_stream_cfg(bmp4_fragment_demux_t demux, bmp4_fragment_stream_cfg *cfg);

void bmp4_fragment_get_stream_cfg(bmp4_fragment_stream_t, bmedia_pes_stream_cfg *cfg);

bmp4_fragment_stream_t bmp4_fragment_stream_create(bmp4_fragment_demux_t demux, const bmp4_fragment_stream_cfg *cfg, unsigned stream_id, batom_pipe_t pipe_out);

void bmp4_fragment_stream_destroy(bmp4_fragment_demux_t demux, bmp4_fragment_stream_t stream);


#define B_MP4_FRAGMENT_WRAPPER	 BMP4_TYPE('b','m','p','4')
#define B_MP4_FRAGMENT_HEADER    BMP4_TYPE('b','h','e','d')
#define B_MP4_FRAGMENT_DATA      BMP4_TYPE('b','d','a','t')

#ifdef __cplusplus
}
#endif

#endif /* BMP4_FRAGMENT_DEMUX_H__*/

    

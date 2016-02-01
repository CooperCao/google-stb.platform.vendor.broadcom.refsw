/***************************************************************************
 *     Copyright (c) 2007 Broadcom Corporation
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
 * BMedia library, AC3 elementary stream probe
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bac3_probe.h"
#include "bkni.h"
#include "biobits.h"
BDBG_MODULE(bac3_probe);

#define B_AC3_N_FRAMES  3

typedef struct bmedia_probe_audio_ac3 *bac3_probe_t;
BDBG_OBJECT_ID(bac3_probe_t);

struct bmedia_probe_audio_ac3 {
    BDBG_OBJECT(bac3_probe_t)
    batom_accum_t acc;
    bmedia_probe_audio info;
    unsigned frame_cnt;
};

static bmedia_probe_base_es_t 
b_ac3_probe_create(batom_factory_t factory)
{
    bac3_probe_t ac3;
    BDBG_ASSERT(factory);

    ac3 = BKNI_Malloc(sizeof(*ac3));
    if(!ac3) {
        goto err_alloc;
    }
    BDBG_OBJECT_INIT(ac3, bac3_probe_t);
    ac3->frame_cnt = 0;
    ac3->acc = batom_accum_create(factory);
    if(!ac3->acc) {
        goto err_acc;
    }
    return (bmedia_probe_base_es_t)ac3;
err_acc:
    BKNI_Free(ac3);
err_alloc:
    return NULL;
}

static void 
b_ac3_probe_destroy(bmedia_probe_base_es_t probe)
{
    bac3_probe_t ac3 = (bac3_probe_t)probe;
    BDBG_OBJECT_ASSERT(ac3, bac3_probe_t);
    batom_accum_destroy(ac3->acc);
    BDBG_OBJECT_DESTROY(ac3, bac3_probe_t);
    BKNI_Free(ac3);
    return;
}

static void 
b_ac3_probe_reset(bmedia_probe_base_es_t probe)
{
    bac3_probe_t ac3 = (bac3_probe_t)probe;
    BDBG_OBJECT_ASSERT(ac3, bac3_probe_t);
    ac3->frame_cnt = 0;
    batom_accum_clear(ac3->acc);
    return;
}
static const uint16_t b_ac3_sample_rate[]={48000, 44100, 32000};
static const struct {
    uint16_t bitrate;
    uint16_t words[3];
} b_ac3_syncinfo[] = { /* 48KHz   44.1KHz  32KHz */
    /* 000000 */    {32,  {    64,     69,        96     }},
    /* 000001 */    {32,  {    64,     70,        96     }},
    /* 000010 */    {40,  {    80,     87,        120 }},
    /* 000011 */    {40,  {    80,     88,        120 }},
    /* 000100 */    {48,  {    96,     104,    144 }}, 
    /* 000101 */    {48,  {    96,     105,    144 }}, 
    /* 000110 */    {56,  {    112,    121,    168 }}, 
    /* 000111 */    {56,  {    112,    122,    168 }}, 
    /* 001000 */    {64,  {    128,    139,    192 }}, 
    /* 001001 */    {64,  {    128,    140,    192 }}, 
    /* 001010 */    {80,  {    160,    174,    240 }}, 
    /* 001011 */    {80,  {    160,    175,    240 }}, 
    /* 001100 */    {96,  {    192,    208,    288 }}, 
    /* 001101 */    {96,  {    192,    209,    288 }}, 
    /* 001110 */    {112, {    224,    243,    336 }}, 
    /* 001111 */    {112, {    224,    244,    336 }}, 
    /* 010000 */    {128, {    256,    278,    384 }}, 
    /* 010001 */    {128, {    256,    279,    384 }}, 
    /* 010010 */    {160, {    320,    348,    480 }}, 
    /* 010011 */    {160, {    320,    349,    480 }}, 
    /* 010100 */    {192, {    384,    417,    576 }}, 
    /* 010101 */    {192, {    384,    418,    576 }}, 
    /* 010110 */    {224, {    448,    487,    672 }}, 
    /* 010111 */    {224, {    448,    488,    672 }}, 
    /* 011000 */    {256, {    512,    557,    768 }}, 
    /* 011001 */    {256, {    512,    558,    768 }}, 
    /* 011010 */    {320, {    640,    696,    960 }}, 
    /* 011011 */    {320, {    640,    697,    960 }}, 
    /* 011100 */    {384, {    768,     835,    1152 }},
    /* 011101 */    {384, {    768,     836,    1152 }},
    /* 011110 */    {448, {    896,     975,    1344 }},
    /* 011111 */    {448, {    896,     976,    1344 }},
    /* 100000 */    {512, {    1024,    1114,    1536 }},
    /* 100001 */    {512, {    1024,    1115,    1536 }},
    /* 100010 */    {576, {    1152,    1253,    1728 }},
    /* 100011 */    {576, {    1152,    1254,    1728 }},
    /* 100100 */    {640, {    1280,    1393,    1920 }},
    /* 100101 */    {640, {    1280,    1394,    1920 }}
};

static const uint8_t b_ac3_acmod_nfchans[] = {
    /* 000 */ 2,
    /* 001 */ 1,
    /* 010 */ 2,
    /* 011 */ 3,
    /* 100 */ 3,
    /* 101 */ 4,
    /* 110 */ 4,
    /* 111 */ 5
};

static const uint8_t b_eac3_nblocks[] = {1,2,3,6};

size_t
b_ac3_probe_parse_header(batom_cursor *cursor, bmedia_probe_audio *info, unsigned *pnblocks)
{
    uint32_t temp = batom_cursor_uint24_be(cursor);
    uint32_t bsi = batom_cursor_uint16_be(cursor); /* read part of BSI */
    baudio_format codec; 

    if(BATOM_IS_EOF(cursor)) {
        return 0;
    }
    *pnblocks = 6;

    switch(B_GET_BITS(bsi, 15, 11)) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
        /* A52B, 5.4.2.1 , page 36 */
        codec = baudio_format_ac3;
        break;
        /* A52B, E.2.3.16, page 159 */
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
        codec = baudio_format_ac3_plus;
        break;
    default:
        codec = baudio_format_unknown;
        break;
    }

    if(codec!=baudio_format_ac3_plus) {
        unsigned fsmod = B_GET_BITS(temp,7,6);
        unsigned frmsizcod = B_GET_BITS(temp,5,0);
        if(fsmod<sizeof(b_ac3_sample_rate)/sizeof(*b_ac3_sample_rate) && frmsizcod< sizeof(b_ac3_syncinfo)/sizeof(*b_ac3_syncinfo)) {
            info->codec = baudio_format_ac3;
            info->sample_size = 16;
            info->bitrate = b_ac3_syncinfo[frmsizcod].bitrate;
            info->sample_rate = b_ac3_sample_rate[fsmod];
            info->channel_count = b_ac3_acmod_nfchans[B_GET_BITS(bsi, 7, 5)]; 
            ((bmedia_probe_ac3_audio *)&info->codec_specific)->codec = codec;
            ((bmedia_probe_ac3_audio *)&info->codec_specific)->valid = true;
            return (b_ac3_syncinfo[frmsizcod].words[fsmod]*2) - (B_AC3_SYNC_LEN+3+2);
        } 
    } else { /* codec==baudio_format_ac3_plus */
        /* A52B, Appendix E */
        unsigned frame_size = B_GET_BITS(temp,18,8);
        unsigned fscod = B_GET_BITS(temp,7,6);
        unsigned nblocks;
        if(fscod!=3) {
            info->sample_rate = b_ac3_sample_rate[fscod];
            nblocks = B_GET_BITS(temp,5,4);
        } else {
            fscod = B_GET_BITS(temp,5,4);
            if(fscod==3) {
                return 0;
            }
            info->sample_rate = b_ac3_sample_rate[fscod]/2;
            nblocks = B_GET_BITS(temp,3,2);
        }
        info->codec = baudio_format_ac3_plus;
        info->sample_size = 16;
        info->channel_count = b_ac3_acmod_nfchans[B_GET_BITS(temp, 3, 1)] + B_GET_BITS(temp,0,0)/* LFE channel */; 
        frame_size = 2*(frame_size+1)-(B_AC3_SYNC_LEN+3+2);
        nblocks = b_eac3_nblocks[nblocks];
        *pnblocks = nblocks;
	    info->bitrate = (8*frame_size* info->sample_rate) / ((nblocks*256)/* AC3+ samples per frame !not always correct! */ * 1000 /* KBps */);
        ((bmedia_probe_ac3_audio *)&info->codec_specific)->codec = codec;
        ((bmedia_probe_ac3_audio *)&info->codec_specific)->valid = true;
        return frame_size;
    }
    return 0;
}

static bmedia_probe_track *
b_ac3_probe_make_track(bac3_probe_t ac3)
{
    bmedia_probe_track *track;

    batom_accum_clear(ac3->acc);
    /* found all frames */
    track = BKNI_Malloc(sizeof(*track));
    if(track) {
        bmedia_probe_track_init(track);
        track->type = bmedia_track_type_audio;
        track->info.audio = ac3->info;
    }
    return track;
}

static bmedia_probe_track *
b_ac3_probe_last(bmedia_probe_base_es_t probe, unsigned *probability)
{
    bac3_probe_t ac3 = (bac3_probe_t)probe;
    BDBG_OBJECT_ASSERT(ac3, bac3_probe_t);
    if(ac3->frame_cnt > 1) {
        *probability = 90;
        return b_ac3_probe_make_track(ac3);
    }
    return 0;
}

static bmedia_probe_track *
b_ac3_probe_feed(bmedia_probe_base_es_t probe, batom_t atom, bool *done)
{
    bac3_probe_t ac3 = (bac3_probe_t)probe;
    batom_cursor cursor;
    batom_accum_t acc;

    BSTD_UNUSED(done);

    BDBG_OBJECT_ASSERT(ac3, bac3_probe_t);
    acc = ac3->acc;
    batom_accum_add_atom(acc, atom);
    BKNI_Memset(&ac3->info, 0, sizeof(ac3->info)); /* this is not needed, used to fix warning-bug in GCC compiler */
    for(;;) {
        size_t byte_cnt=0;
        uint32_t temp = 0;

        batom_cursor_from_accum(&cursor, acc);
        for(;;) {
            int byte;
            byte = batom_cursor_next(&cursor);
            if(byte==BATOM_EOF) {
                goto done;
            }
            byte_cnt++;
            temp = ((temp&0xFF)<<8)|byte;
            if(temp==B_AC3_SYNC) {
                break;
            }
        }
        /* found sync word, now look for back-to-back frames */
        for(ac3->frame_cnt=0;ac3->frame_cnt<B_AC3_N_FRAMES;ac3->frame_cnt++) {
            unsigned nblocks;
            BDBG_MSG(("b_ac3_probe_feed: %p sync_word at:%u", (void *)ac3, (unsigned)batom_cursor_pos(&cursor)-B_AC3_SYNC_LEN));
            temp = b_ac3_probe_parse_header(&cursor,&ac3->info, &nblocks);
            if(BATOM_IS_EOF(&cursor)) {
                goto done; /* wait for more data */
            } else if(temp>0) {
                BDBG_MSG(("b_ac3_probe_feed: %p header %u at:%u size:%u samplerate:%u bitrate:%u %s", (void *)ac3, ac3->frame_cnt, (unsigned)batom_cursor_pos(&cursor), temp, ac3->info.sample_rate, ac3->info.bitrate, ac3->info.codec==baudio_format_ac3?"AC3":"AC3+"));
                batom_cursor_skip(&cursor, temp);
                temp = batom_cursor_uint16_be(&cursor);
                if(BATOM_IS_EOF(&cursor)) {
                    goto done; /* wait for more data */
                }
                if(temp==B_AC3_SYNC) {
                    continue;
                }
                BDBG_MSG(("b_ac3_probe_feed: %p out of sync at %u %#x(%#x)", (void *)ac3, (unsigned)batom_cursor_pos(&cursor), temp, B_AC3_SYNC));
            } 
            byte_cnt+=B_AC3_SYNC_LEN;
            goto done; /* skip over sync word */
        }
        /* have found enough back-to-back frames */
        return b_ac3_probe_make_track(ac3);
done:
        BDBG_MSG(("b_ac3_probe_feed: %p trim %u", (void *)ac3, (unsigned)byte_cnt));
        if(byte_cnt<=B_AC3_SYNC_LEN) {
            break;
        }
        batom_cursor_from_accum(&cursor, acc);
        batom_cursor_skip(&cursor, byte_cnt-B_AC3_SYNC_LEN);
        batom_accum_trim(acc, &cursor);
    }
    return NULL;
}

static const bmedia_probe_file_ext b_ac3_ext[] =  {
    {"ac3"},
    {""}
};

const bmedia_probe_es_desc bac3_probe = {
    bmedia_track_type_audio,
    {
        baudio_format_ac3
    },
    b_ac3_ext,
    (2*B_AC3_N_FRAMES)*1920,
    b_ac3_probe_create,
    b_ac3_probe_destroy,
    b_ac3_probe_feed,
    b_ac3_probe_reset,
    b_ac3_probe_last
};
        







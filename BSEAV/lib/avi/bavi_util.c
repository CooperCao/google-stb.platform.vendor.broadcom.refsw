/***************************************************************************
 *     Copyright (c) 2006-2013, Broadcom Corporation
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
 * AVI parser library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/

#include "bstd.h"
#include "bavi_util.h"
#include "bkni.h"

BDBG_MODULE(bavi_util);

#define B_AVI_FLAG(flag,prfx,bit) (((flag)&(prfx##_##bit))? #bit " ":"")
bool
bavi_read_streamheader(bavi_streamheader *sh, batom_t header)
{
	batom_cursor c;
	batom_cursor_from_atom(&c,header);

	/*
	sh->fcc = bavi_read_fourcc(&c);
	sh->cb = bavi_read_dword(&c);
	*/
	sh->fccType = bavi_read_fourcc(&c);
	sh->fccHandler = bavi_read_fourcc(&c);
	sh->dwFlags = bavi_read_dword(&c);
	sh->wPriority = bavi_read_word(&c);
	sh->wLanguage = bavi_read_word(&c);
	sh->dwInitialFrames = bavi_read_dword(&c);
	sh->dwScale = bavi_read_dword(&c);
	sh->dwRate = bavi_read_dword(&c);
	sh->dwStart = bavi_read_dword(&c);
	sh->dwLength = bavi_read_dword(&c);
	sh->dwSuggestedBufferSize = bavi_read_dword(&c);
	sh->dwQuality = bavi_read_dword(&c);
	sh->dwSampleSize = bavi_read_dword(&c);
    sh->rcFrame.left = 0;
    sh->rcFrame.top = 0;
    sh->rcFrame.right = 0;
    sh->rcFrame.bottom = 0;
    /* treat rcFrame as optional element */
    if(batom_cursor_reserve(&c,4*2)>=4*2) {
        sh->rcFrame.left = bavi_read_short_int(&c);
        sh->rcFrame.top = bavi_read_short_int(&c);
        sh->rcFrame.right = bavi_read_short_int(&c);
        sh->rcFrame.bottom = bavi_read_short_int(&c);
    }
	BDBG_MSG(("bavi_read_streamheader: " BMEDIA_FOURCC_FORMAT " %s InitialFrames:%u Scale:%u Rate:%u SampleSize:%u Start:%u Length:%u", BMEDIA_FOURCC_ARG(sh->fccType), B_AVI_FLAG(sh->dwFlags,BAVI_SF, DISABLED), (unsigned)sh->dwInitialFrames, (unsigned)sh->dwScale, (unsigned)sh->dwRate, (unsigned)sh->dwSampleSize, (unsigned)sh->dwStart, (unsigned)sh->dwLength));
	return !BATOM_IS_EOF(&c);
}

bool
bavi_read_mainheader(bavi_mainheader *mh, batom_t header)
{
	batom_cursor c;
	unsigned i;
	batom_cursor_from_atom(&c,header);

	mh->dwMicroSecPerFrame = bavi_read_dword(&c);
	mh->dwMaxBytesPerSec = bavi_read_dword(&c);
	mh->dwPaddingGranularity = bavi_read_dword(&c);
	mh->dwFlags = bavi_read_dword(&c);
	mh->dwTotalFrames = bavi_read_dword(&c);
	mh->dwInitialFrames = bavi_read_dword(&c);
	mh->dwStreams = bavi_read_dword(&c);
	mh->dwSuggestedBufferSize = bavi_read_dword(&c);
	mh->dwWidth = bavi_read_dword(&c);
	mh->dwHeight = bavi_read_dword(&c);
	for(i=0;i<sizeof(mh->dwReserved)/sizeof(*mh->dwReserved);i++) {
		mh->dwReserved[i] = bavi_read_dword(&c);
	}
	BDBG_MSG(("bavi_read_mainheader: %s%s%s MaxBytesPerSec:%u MicroSecPerFrame:%u Streams:%u TotalFrames:%u InitialFrames:%u %ux%u", B_AVI_FLAG(mh->dwFlags,BAVI_F,HASINDEX), B_AVI_FLAG(mh->dwFlags, BAVI_F, MUSTUSEINDEX), B_AVI_FLAG(mh->dwFlags, BAVI_F, ISINTERLEAVED), (unsigned)mh->dwMaxBytesPerSec, (unsigned)mh->dwMicroSecPerFrame, (unsigned)mh->dwStreams, (unsigned)mh->dwTotalFrames, (unsigned)mh->dwInitialFrames, (unsigned)mh->dwWidth, (unsigned)mh->dwHeight));
	return !BATOM_IS_EOF(&c);
}

bool bavi_read_dmlh(bavi_dmlh *dmlh, batom_t header)
{
	batom_cursor c;
	batom_cursor_from_atom(&c,header);

	dmlh->dwTotalFrames = bavi_read_dword(&c);
	BDBG_MSG(("bavi_read_dmlh: TotalFrames:%u", (unsigned)dmlh->dwTotalFrames));

	return !BATOM_IS_EOF(&c);
}

void 
bavi_audio_state_init(bavi_audio_state *audio)
{
    BDBG_ASSERT(audio);
    audio->min_size = 0;
    audio->max_size = 0;
    audio->vbr_scale = 1;
    audio->cbr_scale = 1;
    audio->block_count = 0;
	audio->data_size = 0;
	audio->skew = 0;
    audio->header.dwStart = 0;
    audio->header.nSamplesPerSec = 0;
    audio->header.dwRate = 0;
    audio->codec = baudio_format_unknown;
    return;
}

#define B_AVI_AUDIO_TYPE_STR(type) (type==bavi_audio_stream_type_cbr)?"CBR":(type==bavi_audio_stream_type_vbr?"VBR":(type==bavi_audio_stream_type_maybe_vbr?"MAYBE VBR":"MAYBE CBR"))

void
bavi_audio_state_set_header(bavi_audio_state *audio, const bavi_streamheader *header, const bmedia_waveformatex *wave)
{
    BDBG_ASSERT(audio);
    BDBG_ASSERT(header);
    BDBG_ASSERT(wave);
    audio->header.dwStart = header->dwStart;
    audio->header.dwRate = header->dwRate;
    audio->header.nSamplesPerSec = wave->nSamplesPerSec;
    audio->header.nBlockAlign = wave->nBlockAlign;
    audio->block_count = 0;
    audio->data_size = 0;
    audio->skew = 0;
    audio->max_framelen = wave->nBlockAlign; /* initialize audio max framlen as wave->nBlockAlign  */
    audio->codec = baudio_format_unknown;
	if(wave->nSamplesPerSec) {
		switch(wave->wFormatTag) {
		case 0x0050: /* MPEG1 L1/L2 */ audio->max_framelen = (144000*448)/wave->nSamplesPerSec; break; /* 448 KBps max audio rate for MPEG1 L1 audio */
		case 0x0055: /* MPEG1 L3 */ audio->max_framelen = (144000*320)/wave->nSamplesPerSec; break; /* 320 KBps max audio rate for MPEG1 L3 audio */
		default: break;
		}
	}

    /* selecting length(duration) of audio VBR frame */
    if(   wave->nSamplesPerSec>=32000 /* if sample rate is 33,44.1 and 48Khz each audio sample corresponds to 1152 samples (e.g. single MPEG Layer2/3 frame) */
       || wave->nBlockAlign == 1152) {  /* or if nBlockAlign is 1152 */
          audio->vbr_scale = 1152*1000;
    } else {
          audio->vbr_scale = 576*1000; /* for low sampling rates  each audio sample corresponds to half of 1152 samples (e.g. half of MPEG Layer2/3 frame) */
    }

    if(BMEDIA_WAVFMTEX_AUDIO_PCM(wave)) {
        /* for PCM audio, time is always derived from number of played samples, and coefficients to convert samples to time are taken from the wave header */
        audio->header.dwRate = wave->nChannels * wave->wBitsPerSample * wave->nSamplesPerSec; /* rate in bits per second */
        audio->cbr_scale = 8; /* to convert from bytes to bits */
        audio->type = bavi_audio_stream_type_cbr;
    } else if(BMEDIA_WAVFMTEX_AUDIO_WMA(wave) || BMEDIA_WAVFMTEX_AUDIO_WMA_PRO(wave)) {
        audio->type = bavi_audio_stream_type_cbr;
    } else if(BMEDIA_WAVFMTEX_AUDIO_AC3(wave)) {
        if(audio->header.nBlockAlign==4) { /* there are are VBR audio streams that set nBlockAlign to 4 */
            audio->type = bavi_audio_stream_type_vbr;
        } else {
            audio->type = bavi_audio_stream_type_maybe_vbr;
        }
        audio->vbr_scale = 256 * 6 * 1000; /* AC3 Frame Size */
        audio->codec = baudio_format_ac3;;
    } else if(BMEDIA_WAVFMTEX_AUDIO_AAC(wave)) {
        audio->type = bavi_audio_stream_type_vbr;
        audio->vbr_scale = 1024 * 1000; /* Frame Size */
    } else {
        if( !(wave->nBlockAlign==576 || wave->nBlockAlign==1152)) { /* not valid nBlockAlign for VBR audio */
            audio->type = bavi_audio_stream_type_cbr;
        }  else {
            audio->type = bavi_audio_stream_type_maybe_vbr;
        }
        if(audio->header.dwRate>=(1000000/8)) { /* if audio stream bitrate exceeds 1Mbps */
            audio->cbr_scale = header->dwScale;
        }
    }
    BDBG_MSG(("bavi_audio_state_set_header:%#lx nBlockAlign:%u:%u VBR scale:%u CBR scale:%u", (unsigned long)audio,  (unsigned)wave->nBlockAlign, (unsigned)audio->max_framelen, (unsigned)audio->vbr_scale, (unsigned)audio->cbr_scale));
    return;
}


void 
bavi_audio_state_update(bavi_audio_state *audio, size_t block_len)
{
    BDBG_ASSERT(audio);
	if(audio->type==bavi_audio_stream_type_vbr || audio->type==bavi_audio_stream_type_cbr) {
		goto done;
	}

    /* If the stream is AC3 and the size of the block is greater than the nBlockAlign,
        then the file must be CBR */
    if(audio->codec == baudio_format_ac3) {
        if (audio->type == bavi_audio_stream_type_maybe_vbr &&
            block_len > audio->header.nBlockAlign) {
                audio->type=bavi_audio_stream_type_cbr;
        }
        goto done;
    }

    if(audio->block_count>0) {
        if(block_len < audio->min_size) {
            audio->min_size = block_len;
        } else if(block_len > audio->max_size) {
            audio->max_size = block_len;
        }
    } else {
        audio->min_size = block_len;
        audio->max_size = block_len;
    }
	if(audio->type==bavi_audio_stream_type_maybe_vbr) {

		if( !(audio->header.nBlockAlign==576 || audio->header.nBlockAlign==1152)) { /* not valid nBlockAlign for VBR audio */
			audio->type = bavi_audio_stream_type_cbr;
			goto done;
		} 
		if(block_len > audio->max_framelen) { /* if audio block size exceeds max size of audio frame, it means there is a multiple frames of audio in the single block, which means that audio is CBR */
			if( audio->min_size!=0 && 
				 ((block_len % audio->min_size)!=0 || /* have found block that is not integer multiplier of smallest block */
				  (block_len/audio->min_size)>=8) /* have found block that is 8 times larger then a smallest block */
				 ) {
				audio->type = bavi_audio_stream_type_vbr; 
			} else {
			    audio->type = bavi_audio_stream_type_maybe_cbr;
            }
            goto done;
		}
		if(audio->block_count>100) {
			audio->type = bavi_audio_stream_type_vbr;
		}
	} else if(audio->type==bavi_audio_stream_type_maybe_cbr) {
        if(block_len < audio->header.nBlockAlign) {
            bmedia_player_pos timestamp_cbr, timestamp_vbr;
            timestamp_cbr = bavi_audio_state_get_timestamp(audio, audio->block_count, audio->data_size);
            audio->type = bavi_audio_stream_type_maybe_vbr;
            timestamp_vbr = bavi_audio_state_get_timestamp(audio, audio->block_count, audio->data_size);
            audio->skew += timestamp_cbr - timestamp_vbr;
            BDBG_MSG(("bavi_audio_state_update:%p CBR:%u -> VBR:%u skew:%d", (void *)audio, (unsigned)timestamp_cbr, (unsigned)timestamp_vbr, audio->skew));
            goto done;
        }
		if(audio->block_count>100) {
			audio->type = bavi_audio_stream_type_cbr;
		}
	}
done:
	audio->data_size += block_len;
    audio->block_count++;
    BDBG_MSG(("bavi_audio_state_update:%#lx[%s] block_len:%u(%u:%u) block_count:%u", (unsigned long)audio, B_AVI_AUDIO_TYPE_STR(audio->type), (unsigned)block_len, (unsigned)audio->min_size, (unsigned)audio->max_size, (unsigned)audio->block_count));
    return;
}

bmedia_player_pos
bavi_audio_state_get_timestamp(const bavi_audio_state *audio, unsigned block_count, unsigned data_size)
{
    bmedia_player_pos timestamp;

    BDBG_ASSERT(audio);
    if(audio->header.nSamplesPerSec==0 || audio->header.dwRate==0) {
        BDBG_WRN(("bavi_audio_state_get_timestamp:%#lx invalid audio header", (unsigned long)audio));
        return 0;
    }

    switch(audio->type) {
    case bavi_audio_stream_type_maybe_vbr:
    case bavi_audio_stream_type_vbr:
		timestamp = ((audio->vbr_scale*(uint64_t)(block_count + audio->header.dwStart))/audio->header.nSamplesPerSec);
        break;
    case bavi_audio_stream_type_maybe_cbr:
    case bavi_audio_stream_type_cbr:
		timestamp = ((1000*audio->cbr_scale)*(uint64_t)(audio->header.dwStart+data_size)/audio->header.dwRate);
        break;
    default:
        BDBG_ASSERT(0);
        timestamp = 0;
        break;
    }
	timestamp += audio->skew;
    BDBG_MSG(("bavi_audio_state_get_timestamp:%#lx[%s] timestamp:%u block_count:%u data_size:%u", (unsigned long)audio, B_AVI_AUDIO_TYPE_STR(audio->type), (unsigned)timestamp, block_count, data_size));
    return timestamp;
}

void 
bavi_audio_state_get_memo(const bavi_audio_state *audio, bavi_audio_state_memo *memo)
{
	memo->type = audio->type;
	memo->skew = audio->skew;
	return;
}

void 
bavi_audio_state_clear_memo(bavi_audio_state_memo *memo)
{
	memo->type = bavi_audio_stream_type_maybe_vbr;
	memo->skew = 0;
	return;
}

void 
bavi_audio_state_set_memo(bavi_audio_state *audio, const bavi_audio_state_memo *memo, unsigned block_count, unsigned data_size)
{
	audio->type = memo->type;
	audio->skew = memo->skew;
	audio->block_count = block_count;
	audio->data_size = data_size;
	return;
}


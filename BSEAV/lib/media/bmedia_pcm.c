/***************************************************************************
 *     Copyright (c) 2011-2013, Broadcom Corporation
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
 *******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bfile_util.h"
#include "bmedia_pcm.h"
#include "bmedia_util.h"

BDBG_MODULE(bmedia_pcm);
BDBG_OBJECT_ID(bpcm_file);

#define B_PCM_FILE_WAVE_FILE_HEADER (4/*RIFF*/+4/*wave_len*/+4/*WAVE*/+4/*fmt*/+4/*fmt_len*/)
#define B_PCM_FILE_WAVE_HEADER_LEN (B_PCM_FILE_WAVE_FILE_HEADER+BMEDIA_WAVEFORMATEX_BASE_SIZE+4/*data*/+4/*data_len*/)

struct bpcm_file {
    BDBG_OBJECT(bpcm_file)
    uint8_t wave_header[B_PCM_FILE_WAVE_HEADER_LEN];
    bfile_io_read_t header_fd;
};

void
bpcm_file_config_init(bpcm_file_config *config)
{
    BDBG_ASSERT(config);
    BKNI_Memset(config, 0, sizeof(*config));
    config->sample_size = 16;
    config->sample_rate = 44100;
    config->channel_count = 2;
    config->littleEndian = false;
    return;
}

bpcm_file_t
bpcm_file_create(bfile_io_read_t fd, const bpcm_file_config *config)
{
    bpcm_file_t pcm;
    bmedia_waveformatex_header wf;
    size_t wf_len;
    off_t first, last;
    off_t max_last;
    int rc;


    BDBG_ASSERT(fd);
    BDBG_ASSERT(config);
    pcm = BKNI_Malloc(sizeof(*pcm));
    if(!pcm) {(void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_alloc;}
    BDBG_OBJECT_INIT(pcm, bpcm_file);

    bmedia_init_waveformatex(&wf);
    wf.wFormatTag = config->littleEndian ? BMEDIA_WAVFMTEX_AUDIO_PCM_TAG : BMEDIA_WAVFMTEX_AUDIO_PCM_BE_TAG;
    wf.nChannels = config->channel_count;
    wf.nSamplesPerSec = config->sample_rate;
    wf.nBlockAlign = config->channel_count*config->sample_size;
    wf.nAvgBytesPerSec = config->sample_rate * config->channel_count * (config->sample_size/8);
    wf.wBitsPerSample = config->sample_size;
    wf_len = bmedia_write_waveformatex(pcm->wave_header+B_PCM_FILE_WAVE_FILE_HEADER, &wf);
    BDBG_ASSERT(wf_len==BMEDIA_WAVEFORMATEX_BASE_SIZE);
    B_MEDIA_SAVE_UINT32_LE(pcm->wave_header+0, BMEDIA_FOURCC('R','I','F','F'));
    B_MEDIA_SAVE_UINT32_LE(pcm->wave_header+8, BMEDIA_FOURCC('W','A','V','E'));
    B_MEDIA_SAVE_UINT32_LE(pcm->wave_header+12, BMEDIA_FOURCC('f','m','t',' '));
    B_MEDIA_SAVE_UINT32_LE(pcm->wave_header+16, BMEDIA_WAVEFORMATEX_BASE_SIZE);
    B_MEDIA_SAVE_UINT32_LE(pcm->wave_header+B_PCM_FILE_WAVE_FILE_HEADER+BMEDIA_WAVEFORMATEX_BASE_SIZE, BMEDIA_FOURCC('d','a','t','a'));
    rc = fd->bounds(fd, &first, &last);
    max_last = 0x7FFFFFF0 - B_PCM_FILE_WAVE_HEADER_LEN; /* just set a large enough length */
    if(rc!=0 || last>max_last) {
        last = max_last;
    }
    B_MEDIA_SAVE_UINT32_LE(pcm->wave_header+4, last+B_PCM_FILE_WAVE_HEADER_LEN-4);
    B_MEDIA_SAVE_UINT32_LE(pcm->wave_header+B_PCM_FILE_WAVE_FILE_HEADER+BMEDIA_WAVEFORMATEX_BASE_SIZE+4, last);

    pcm->header_fd = bfile_read_header_attach(fd, pcm->wave_header, sizeof(pcm->wave_header));
    if(!pcm->header_fd) { (void)BERR_TRACE(BERR_NOT_SUPPORTED);goto err_header_fd;}

    return pcm;

err_header_fd:
    BKNI_Free(pcm);
err_alloc:
    return NULL;
}

bfile_io_read_t bpcm_file_get_file_interface(bpcm_file_t pcm)
{
    BDBG_OBJECT_ASSERT(pcm, bpcm_file);
    return pcm->header_fd;
}

void bpcm_file_destroy(bpcm_file_t pcm)
{
    BDBG_OBJECT_ASSERT(pcm, bpcm_file);
    bfile_read_header_detach(pcm->header_fd);
    BDBG_OBJECT_DESTROY(pcm, bpcm_file);
    BKNI_Free(pcm);
    return;
}


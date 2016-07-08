/******************************************************************************
 *    (c)2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 *****************************************************************************/
#include "bstd.h"
#include "bdbg.h"
#include "wav_file.h"
BDBG_MODULE(wav_file);

/* based on nexus/examples/audio/audio_playback.c */

#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
#define SWAP32( a )  do{a=((a&0xFF)<<24|(a&0xFF00)<<8|(a&0xFF0000)>>8|(a&0xFF000000)>>24);}while(0)
#define SWAP16( a )  do{a=((a&0xFF)<<8|(a&0xFF00)>>8);}while(0)
#else
#define SWAP32( a )
#define SWAP16( a )
#endif

#define READ32(VAL,FILE) do { if (fread(&(VAL),4,1,(FILE)) != 1) goto done; SWAP32(VAL); } while(0)
#define READ16(VAL,FILE) do { if (fread(&(VAL),2,1,(FILE)) != 1) goto done; SWAP16(VAL); } while(0)

#define WRITE32(VAL,FILE) do { SWAP32(VAL); if (fwrite(&(VAL),4,1,(FILE)) != 1) goto done; } while(0)
#define WRITE16(VAL,FILE) do { SWAP16(VAL); if (fwrite(&(VAL),2,1,(FILE)) != 1) goto done; } while(0)

#define WAVE_VALUE_RIFF         0x46464952
#define WAVE_VALUE_WAV          0x45564157
#define WAVE_VALUE_DATA_CHUNK   0x61746164
#define WAVE_VALUE_FMT          0x20746d66

int read_wave_header(FILE *file, struct wave_header *wh)
{
    memset(wh, 0, sizeof(*wh));

    READ32(wh->riff,file);
    if (wh->riff != WAVE_VALUE_RIFF) {
        BDBG_WRN(("RAW data file detected."));
        goto done;
    }
    READ32(wh->riffCSize,file);
    READ32(wh->wave,file);
    if (wh->wave != WAVE_VALUE_WAV) {
        BDBG_WRN(("Not a WAV file."));
        goto done;
    }
    READ32(wh->fmt,file);
    READ32(wh->headerLen,file);
    READ16(wh->format,file);
    READ16(wh->channels,file);
    if (wh->channels > 2) {
        BDBG_ERR(("Invalid number of channels (%u) specified.", wh->channels));
        goto done;
    }
    READ32(wh->samplesSec,file);
    READ32(wh->bytesSec,file);
    READ16(wh->chbits,file);
    READ16(wh->bps,file);

    if (wh->headerLen == 40 && wh->format == 0xfffe) { /* WAVE_FORMAT_EXTENSIBLE */
        READ16(wh->cbSize,file);                /* 2 Size of the extension (0 or 22)  */
        READ16(wh->validBitsPerSample,file);   /* 2 Number of valid bits  */
        READ32(wh->channelMask,file);         /* 4 Speaker position mask  */
        fread(&wh->subFormat,16,1,file);            /* SubFormat GUID */
    }
    else if (wh->headerLen == 18 && wh->format == 1) { /* oddball WAVE format */
        READ16(wh->cbSize,file);                /* 2 Size of the extension (0 or 22) ?*/
    }
    else if (wh->headerLen != 16 && wh->format != 1) {
        BDBG_ERR(("Not PCM data in WAV file. headerLen = %lu, Format 0x%x", wh->headerLen,wh->format));
    }

    for (;;) {
        READ32(wh->dataSig, file);
        READ32(wh->dataLen, file);

        /* looking for 'data' chunk */
        if (wh->dataSig == WAVE_VALUE_DATA_CHUNK) {
            wh->dataStart = ftell(file);
            break;
        }
        if (fseek(file, wh->dataLen, SEEK_CUR)) {
            BDBG_ERR(("Incomplete chunk found WAV file."));
            goto done;
        }
    }

    return 0;

done:
    fseek(file, 0, SEEK_SET);
    return -1;
}

void get_default_wave_header(struct wave_header *wh)
{
    memset(wh, 0, sizeof(*wh));
    wh->riff = WAVE_VALUE_RIFF;
    wh->wave = WAVE_VALUE_WAV;
    wh->headerLen = 16;
    wh->format = 1;
    wh->channels = 2;
    wh->samplesSec = 44100;
    wh->bytesSec = 0;
    wh->chbits = 0;
    wh->bps = 16;
    wh->dataSig = WAVE_VALUE_DATA_CHUNK;
    wh->fmt = WAVE_VALUE_FMT;
}

int write_wave_header(FILE *file, const struct wave_header *_wh)
{
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
    /* copy to that SWAP macros can work in place */
    struct wave_header copy = *_wh;
    struct wave_header *wh = &copy;
#else
    const struct wave_header *wh = _wh;
#endif

    WRITE32(wh->riff,file);
    WRITE32(wh->riffCSize,file);
    WRITE32(wh->wave,file);
    WRITE32(wh->fmt,file);
    WRITE32(wh->headerLen,file);
    WRITE16(wh->format,file);
    WRITE16(wh->channels,file);
    WRITE32(wh->samplesSec,file);
    WRITE32(wh->bytesSec,file);
    WRITE16(wh->chbits,file);
    WRITE16(wh->bps,file);
    if (wh->headerLen == 40 && wh->format == 0xfffe) { /* WAVE_FORMAT_EXTENSIBLE */
        WRITE16(wh->cbSize,file);                /* 2 Size of the extension (0 or 22)  */
        WRITE16(wh->validBitsPerSample,file);   /* 2 Number of valid bits  */
        WRITE32(wh->channelMask,file);         /* 4 Speaker position mask  */
        fwrite(&wh->subFormat,16,1,file);            /* SubFormat GUID */
    }
    else if (wh->headerLen == 18 && wh->format == 1) { /* oddball WAVE format */
        WRITE16(wh->cbSize,file);                /* 2 Size of the extension (0 or 22) ?*/
    }
    WRITE32(wh->dataSig, file);
    WRITE32(wh->dataLen, file);
    return 0;

done:
    fseek(file, 0, SEEK_SET);
    return -1;
}

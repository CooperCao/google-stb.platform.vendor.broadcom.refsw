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
#include "bkni.h"
#include "botf_bcmindexer.h"
#include "bcmindexer.h"
#include "tsindexer.h"
#include <stdio.h>
#include <string.h>

BDBG_MODULE(botf_bcmindexer);

BDBG_OBJECT_ID(bfile_otf_indexer);
struct bfile_otf_indexer {
    struct bfile_io_read ops; /* shall be the first member */
    BDBG_OBJECT(bfile_otf_indexer)
    FILE *file;
    BNAV_Indexer_Handle bcmindexer;
    sTsIndexer *tsindexer;
    struct {
        unsigned char buf[188*100];
        unsigned rptr, wptr;
    } input;
    struct {
        void *buf;
        unsigned max, len;
    } output;
};

/* if there are no pictures, don't read the entire file */
#define MAX_READ (3*1024*1024)

static ssize_t bfile_otf_indexer_read(bfile_io_read_t fd, void *buf, size_t length)
{
    struct bfile_otf_indexer *f=(void *)fd;
    unsigned total = 0;

    BDBG_OBJECT_ASSERT(f, bfile_otf_indexer);
    f->output.buf = buf;
    f->output.max = length;
    f->output.len = 0;
    do {
        int n;
        if (f->input.rptr == f->input.wptr) {
            f->input.rptr = f->input.wptr = 0;
            n = fread(f->input.buf, 1, sizeof(f->input.buf), f->file);
            if (n < 0) return BERR_TRACE(n);
            else if (n == 0) break; /* EOF */
            else f->input.wptr = n;
            total += n;
        }

        /* feed into tsindexer, which calls bcmindexer, which NAV into buf */
        BDBG_MSG(("tsindex_feed %d - %d = %d", f->input.wptr, f->input.rptr, f->input.wptr - f->input.rptr));
        n = tsindex_feed(f->tsindexer, &f->input.buf[f->input.rptr], f->input.wptr - f->input.rptr);
        if (n < 0) return BERR_TRACE(n);
        BDBG_ASSERT(f->input.rptr + n <= f->input.wptr);
        BDBG_MSG(("tsindex_feed consumed %d", n));
        f->input.rptr += n;
    } while (!f->output.len && total <= MAX_READ);
    BDBG_MSG(("bfile_otf_indexer_read %d => %d", (unsigned)length, f->output.len));
    return f->output.len;
}

static unsigned long otf_indexer_recv_sct( const void *ptr, unsigned long numEntries, unsigned long entrySize, void *fp )
{
    struct bfile_otf_indexer *f = (void *)fp;
    BDBG_OBJECT_ASSERT(f, bfile_otf_indexer);
    BSTD_UNUSED(entrySize);
    BDBG_MSG(("otf_indexer_recv_sct %ld %ld", numEntries, entrySize));
    return BNAV_Indexer_Feed(f->bcmindexer, (void*)ptr, numEntries);
}

static size_t otf_indexer_recv_nav(const void *ptr, size_t size, size_t nmemb, void *fp)
{
    struct bfile_otf_indexer *f = (void *)fp;
    BDBG_OBJECT_ASSERT(f, bfile_otf_indexer);
    if (!f->output.len) {
        BDBG_MSG(("otf_indexer_recv_nav %d %d", (unsigned)size, (unsigned)nmemb));
        f->output.len = size*nmemb;
        if (f->output.len > f->output.max) {
            f->output.len = f->output.max;
        }
        memcpy(f->output.buf, ptr, f->output.len);
        return f->output.len / size;
    }
    else {
        return nmemb;
    }
}

static struct bfile_io_read botf_bcmindexer_ops = {
    bfile_otf_indexer_read,
    NULL,
    NULL,
    BIO_DEFAULT_PRIORITY
};

bfile_io_read_t botf_bcmindexer_open(const char *streamname, unsigned short pid, NEXUS_VideoCodec codec, unsigned packetSize)
{
    struct bfile_otf_indexer *f;
    BNAV_Indexer_Settings bcmindexer_settings;
    tsindex_settings tsindexer_settings;
    int rc;

    f = BKNI_Malloc(sizeof(*f));
    if (!f) return NULL;
    memset(f, 0, sizeof(*f));
    BDBG_OBJECT_SET(f, bfile_otf_indexer);
    f->file = fopen(streamname, "rb");
    if (!f->file) {
        BDBG_ERR(("unable to open %s", streamname));
        goto err_open;
    }
    f->ops = botf_bcmindexer_ops;

    BNAV_Indexer_GetDefaultSettings(&bcmindexer_settings);
    tsindex_settings_init(&tsindexer_settings);

    /* set codec settings */
    switch (codec) {
    case NEXUS_VideoCodec_eMpeg2:
        bcmindexer_settings.navVersion = BNAV_Version2;
        bcmindexer_settings.videoFormat = BNAV_Indexer_VideoFormat_MPEG2;
        break;
    case NEXUS_VideoCodec_eH264:
        bcmindexer_settings.navVersion = BNAV_Version_AVC;
        bcmindexer_settings.videoFormat = BNAV_Indexer_VideoFormat_AVC;
        tsindexer_settings.start_code_lo = 0x00;
        tsindexer_settings.start_code_hi = 0xFF;
        tsindexer_settings.is_avc = 1;
        break;
    case NEXUS_VideoCodec_eH265:
        bcmindexer_settings.navVersion = BNAV_Version_HEVC;
        bcmindexer_settings.videoFormat = BNAV_Indexer_VideoFormat_HEVC;
        tsindexer_settings.start_code_lo = 0x00;
        tsindexer_settings.start_code_hi = 0xFF;
        tsindexer_settings.is_avc = 1;
        break;
    default:
        BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto err_codec;
    }

    /* open bcmindexer */
    bcmindexer_settings.writeCallback = (INDEX_WRITE_CB)otf_indexer_recv_nav;
    bcmindexer_settings.filePointer = (void *)f;
    bcmindexer_settings.transportTimestampEnabled = (packetSize == 192);
    bcmindexer_settings.sctVersion = BSCT_Version6wordEntry;
    rc = BNAV_Indexer_Open(&f->bcmindexer, &bcmindexer_settings);
    if (rc) {BERR_TRACE(rc); goto err_bcmindexer;}

    /* open tsindexer */
    tsindexer_settings.pid = pid;
    tsindexer_settings.entry_size = 6;
    tsindexer_settings.cb = (INDEX_WRITE_CB)otf_indexer_recv_sct;
    tsindexer_settings.fp = (void*)f;
    tsindexer_settings.ts_packet_size = packetSize;
    f->tsindexer = tsindex_allocate_ex(&tsindexer_settings);
    if (!f->tsindexer) {BERR_TRACE(-1); goto err_tsindexer;}

    return &f->ops;

err_tsindexer:
err_bcmindexer:
err_codec:
err_open:
    botf_bcmindexer_close(&f->ops);
    return NULL;
}

void botf_bcmindexer_close(bfile_io_read_t fd)
{
    struct bfile_otf_indexer *f=(void *)fd;
    BDBG_OBJECT_ASSERT(f, bfile_otf_indexer);
    if (f->bcmindexer) {
        BNAV_Indexer_Close(f->bcmindexer);
    }
    if (f->tsindexer) {
        tsindex_free(f->tsindexer);
    }
    if (f->file) {
        fclose(f->file);
    }
    BDBG_OBJECT_DESTROY(f, bfile_otf_indexer);
    BKNI_Free(f);
}

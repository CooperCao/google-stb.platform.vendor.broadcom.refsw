/******************************************************************************
 * (c) 2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/
#include "bip_priv.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "bfile_stdio.h"


BDBG_MODULE( bip_probe );
BDBG_OBJECT_ID( BIP_Probe );
BIP_SETTINGS_ID(BIP_ProbeParseSettings);

typedef struct BIP_Probe {
    BDBG_OBJECT(BIP_Probe)
    const bmedia_probe_stream *stream;

    batom_factory_t factory;
    batom_pipe_t pipe;

    bmedia_probe_base_t rawProbe;
    const bmedia_probe_format_desc *pRawDesc;

    bmedia_probe_t probe;
} BIP_Probe;


static void BIP_Probe_FreePrevProbe(BIP_ProbeHandle hProbe)
{
    if (hProbe->rawProbe) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "Destroying previous probe=%p type: 0x%x"
                   BIP_MSG_PRE_ARG, hProbe->rawProbe, hProbe->pRawDesc->type));

        if (hProbe->stream) {
            hProbe->pRawDesc->stream_free(hProbe->rawProbe, (bmedia_probe_stream *)(hProbe->stream));
            hProbe->stream = NULL;
        }
        hProbe->pRawDesc->destroy(hProbe->rawProbe);
        hProbe->pRawDesc = NULL;
        hProbe->rawProbe = NULL;
    }
    if (hProbe->probe) {
        if (hProbe->stream) {
            bmedia_probe_stream_free( hProbe->probe, hProbe->stream );
            hProbe->stream = NULL;
        }

        bmedia_probe_destroy( hProbe->probe );
        hProbe->probe = NULL;
    }
}


void BIP_Probe_Destroy(BIP_ProbeHandle hProbe)
{
    BDBG_OBJECT_ASSERT( hProbe, BIP_Probe );

    BIP_Probe_FreePrevProbe(hProbe);

    if (hProbe->pipe)    { batom_pipe_destroy(hProbe->pipe)      ; hProbe->pipe=NULL;}
    if (hProbe->factory) { batom_factory_destroy(hProbe->factory); hProbe->factory=NULL;}

    /* Then finally free the BIP_Probe struct. */
    BDBG_OBJECT_DESTROY( hProbe, BIP_Probe );
    B_Os_Free( hProbe );
    return;
}


BIP_ProbeHandle BIP_Probe_Create(void)
{
    BIP_ProbeHandle         hProbe = NULL;

    hProbe = B_Os_Calloc(1, sizeof(*hProbe));
    if (NULL == hProbe)
    {
        BERR_TRACE( BIP_ERR_OUT_OF_SYSTEM_MEMORY );
        return(NULL);
    }

    BDBG_OBJECT_SET(hProbe, BIP_Probe);

    hProbe->factory = batom_factory_create(bkni_alloc, 16);
    if(!hProbe->factory) goto error;

    hProbe->pipe = batom_pipe_create(hProbe->factory);
    if(!hProbe->pipe) goto error;

    return (hProbe);

error:
    BIP_Probe_Destroy(hProbe);
    return(NULL);
}


const bmedia_probe_stream *
BIP_Probe_ParseFromBfile(BIP_ProbeHandle hProbe, bfile_io_read_t fd, BIP_ProbeParseSettings *pSettings)
{
    void               *rawBuf = NULL;
    bfile_buffer_t      bfBuf         = NULL;
    BIP_Status          rc = BIP_SUCCESS;

    bfile_buffer_cfg            buffer_cfg;
    bmedia_probe_config         probe_config;
    bmedia_probe_parser_config  parser_config;
    BIP_ProbeParseSettings      settings;
    const unsigned              probe_seg_size = (BIO_BLOCK_SIZE*2);    /* BMEDIA_PROBE_FEED_SIZE */
    const unsigned              probe_seg_count = 128;                  /* B_MEDIA_PROBE_N_SEGS */

    BDBG_OBJECT_ASSERT( hProbe, BIP_Probe );

    BIP_SETTINGS_ASSERT(pSettings, BIP_ProbeParseSettings);

    BIP_Probe_FreePrevProbe(hProbe);

    if (!pSettings) {
        BIP_Probe_GetDefaultParseSettings(&settings);
        pSettings = &settings;
    }

    if (pSettings->rawProbe) {

        BIP_CHECK_GOTO((pSettings->pRawDesc), ("Unspecified bmedia_probe_format_desc for raw probe"), error, BIP_ERR_INVALID_PARAMETER, rc );

        hProbe->rawProbe = pSettings->pRawDesc->create(hProbe->factory);
        if(!hProbe->rawProbe) {
            BDBG_WRN(( BIP_MSG_PRE_FMT "%#lx can't create probe for stream type %u"
                       BIP_MSG_PRE_ARG, (unsigned long)hProbe->rawProbe, pSettings->pRawDesc->type));
            goto error;
        }
        hProbe->pRawDesc = pSettings->pRawDesc;

        rawBuf = BKNI_Malloc(probe_seg_count * probe_seg_size);
        if(!rawBuf) {
            goto error;
        }
        bfile_buffer_default_cfg(&buffer_cfg);
        buffer_cfg.buf_len = probe_seg_count * probe_seg_size;
        buffer_cfg.buf = rawBuf;
        buffer_cfg.fd = fd;
        buffer_cfg.nsegs = probe_seg_count;
        bfBuf = bfile_buffer_create(hProbe->factory, &buffer_cfg);
        if(!bfBuf) {
            BDBG_ERR(("bmedia_probe_parse: %#lx can't create buffer", (unsigned long)hProbe->rawProbe));
            goto error;
        }

        bmedia_probe_default_cfg( &probe_config );

        if (pSettings->min_probe_request > 0) {
            probe_config.min_probe_request = pSettings->min_probe_request;
        }
        parser_config.parse_offset      = probe_config.probe_offset;
        parser_config.probe_index       = probe_config.probe_index;
        parser_config.parse_index       = probe_config.parse_index;
        parser_config.stream_specific   = probe_config.stream_specific;
        parser_config.min_parse_request = probe_config.min_probe_request;

        hProbe->stream = pSettings->pRawDesc->parse(hProbe->rawProbe, bfBuf, hProbe->pipe, &parser_config);
    }
    else
    {
        hProbe->probe = bmedia_probe_create();
        if(!hProbe->probe) {
            BDBG_WRN(( BIP_MSG_PRE_FMT "%#lx can't create probe for stream type %u"
                       BIP_MSG_PRE_ARG, (unsigned long)hProbe->probe));
            goto error;
        }

        bmedia_probe_default_cfg( &probe_config );
        probe_config.file_name = pSettings->filename;
        probe_config.type      = pSettings->type;

        probe_config.probe_config.mpeg2ts_psi.reprobe_codec = true;

        hProbe->stream = bmedia_probe_parse( hProbe->probe, fd, &probe_config );
    }

error:
    if (bfBuf)  { bfile_buffer_destroy(bfBuf); bfBuf=NULL; }
    if (rawBuf) { BKNI_Free(rawBuf);           rawBuf=NULL;};

    return (rc==BIP_SUCCESS ? hProbe->stream : NULL);
}


const bmedia_probe_stream *
BIP_Probe_ParseFromFilename(BIP_ProbeHandle hProbe, const char *filename, BIP_ProbeParseSettings *pSettings)
{
    BIP_Status      rc         = BIP_SUCCESS;
    int             osrc;
    int             fd         = -1;
    FILE           *fp         = NULL;
    bfile_io_read_t bfd        = NULL;
    const bmedia_probe_stream  *myStream = NULL;
    struct stat     fileStats;

    BDBG_OBJECT_ASSERT( hProbe, BIP_Probe );
    BDBG_ASSERT(filename);

    BIP_SETTINGS_ASSERT(pSettings, BIP_ProbeParseSettings); /* No settings are defined yet. */

    fd = open( filename, O_RDONLY | O_LARGEFILE );
    BIP_CHECK_GOTO((fd>= 0), ("open() failed: (%s)", filename ), error, BIP_StatusFromErrno(errno), rc );

    osrc = fstat( fd, &fileStats);
    BIP_CHECK_GOTO((osrc==0), ("fstat() failed: (%s)", filename), error, BIP_StatusFromErrno(errno), rc );

    BIP_CHECK_GOTO((fileStats.st_size>0), ("Empty file: (%s)", filename), error, BIP_ERR_MEDIA_INFO_BAD_MEDIA_PATH, rc );

    fp = fdopen( fd, "r" );
    BIP_CHECK_GOTO((fp), ("fdopen() failed: (%s)", filename), error, BIP_StatusFromErrno(errno), rc );
    fd = -1;    /* The fd now belongs to the fp (the fp will close it). */

    bfd = bfile_stdio_read_attach( fp );

    myStream = BIP_Probe_ParseFromBfile(hProbe, bfd, pSettings);

error:
    if (bfd) {bfile_stdio_read_detach( bfd ); }

    if (fp)           {fclose( fp ); }
    else if (fd >= 0) {close( fd ); }

    BDBG_LEAVE( BIP_MediaInfo_GenerateForFile );

    return( rc==BIP_SUCCESS ? myStream : NULL );
}

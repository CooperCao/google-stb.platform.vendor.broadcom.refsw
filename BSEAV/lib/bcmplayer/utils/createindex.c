/***************************************************************************
 *     Copyright (c) 1998-2014, Broadcom Corporation
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
 ****************************************************************/
#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>     /* strcmp() */
#include <sys/stat.h>
#if !defined(__vxworks)
#include <sys/timeb.h>  /* ftime() */
#endif
#include "tsindexer.h"
#include "bcmindexer.h"
BDBG_MODULE(createindex);

#define NUM_STREAM_BYTES_TO_READ    (188*1024)
#define NUM_SC4_BYTES_TO_READ       (sizeof(BSCT_Entry)*1024)
#define NUM_SC6_BYTES_TO_READ       (sizeof(BSCT_SixWord_Entry)*1024)

static int g_quiet = 0;

static void TimetoString( long ctime, char *outStr )
{
    long    hh,mm,ss;
    char    *p;

    ss = ctime % 60;
    mm = (ctime/60) % 60;
    hh = (ctime/3600) % 24;

    p = outStr;
    *p++ = (hh/10) + 0x30;
    *p++ = (hh%10) + 0x30;
    *p++ = ':';
    *p++ = (mm/10) + 0x30;
    *p++ = (mm%10) + 0x30;
    *p++ = ':';
    *p++ = (ss/10) + 0x30;
    *p++ = (ss%10) + 0x30;
    *p++ = 0;
}

static double getms( void )
{
#if defined(__vxworks)
    return 0;
#else
    double  usec1;
    struct timeb t;

    ftime( &t );

    usec1 = t.time + t.millitm/1000.0;
    return usec1;
#endif
}

static void OffsetToString( long offset, char *outStr )
{
    long mb, kb, b;

    b = offset % 1000;
    kb = (offset/1000)%1000;
    mb = (offset/(1000*1000)) % (1000*1000);

    sprintf( outStr, "%ld,%03ld,%03ld", mb, kb, b );
}

/**
* Feed from tsindexer to bcmindexer.
**/
static unsigned long write_to_bcmindexer( const void *p_bfr,
    unsigned long numEntries,
    unsigned long entrySize,
    void *fp )
{
    BSTD_UNUSED(entrySize);
    return BNAV_Indexer_Feed((BNAV_Indexer_Handle)fp, (void*)p_bfr, numEntries);
}

static void printHeader(void)
{
    printf("\ncreateindex, built on %s\n\n", __DATE__);
    printf("Broadcom Corp. Confidential\n");
    printf("Copyright 1998-2012 Broadcom Corp. All Rights Reserved.\n\n");
}

static FILE *g_mpegFile = NULL; /* global hack */
static int mpegSizeCallback(BNAV_Indexer_Handle handle, unsigned long *hi, unsigned long *lo)
{
#ifdef LINUX
    off_t o = ftello(g_mpegFile);
#else
    off_t o = ftell(g_mpegFile);
#endif
    BSTD_UNUSED(handle);
    if (o == -1)
        return -1;
    *hi = o >> 32;
    *lo = o & 0xFFFFFFFF;
    return 0;
}

typedef enum b_input {
    b_input_4word_sct = 0,
    b_input_6word_sct,
    b_input_ts,
    b_input_pes,
    b_input_unknown
} b_input;

static const char *b_input_str[b_input_unknown] = {
    "4word_sct",
    "6word_sct",
    "ts",
    "pes"
};

typedef enum b_output {
    b_output_4word_sct = 0,
    b_output_6word_sct,
    b_output_bcm,
    b_output_unknown
} b_output;

static const char *b_output_str[b_output_unknown] = {
    "4word_sct",
    "6word_sct",
    "bcm"
};

static const char *get_codec_str(BNAV_Indexer_VideoFormat videoFormat)
{
    switch (videoFormat) {
    case BNAV_Indexer_VideoFormat_MPEG2:  return "mpeg";
    case BNAV_Indexer_VideoFormat_AVC:    return "avc";
    case BNAV_Indexer_VideoFormat_AVC_SVC: return "svc";
    case BNAV_Indexer_VideoFormat_AVC_MVC: return "mvc";
    case BNAV_Indexer_VideoFormat_HEVC:    return "hevc";
    case BNAV_Indexer_VideoFormat_VC1:     return "vc1";
    case BNAV_Indexer_VideoFormat_AVS:     return "avs";
    default: return "unknown";
    }
}

/**
* This is the entry point for VxWorks. You'll need to look at the enum values.
**/
int createindex(const char *inputfile, const char *outputfile, int pid, int pid2,
    b_input input, b_output output,
    int timestamp_enabled, BNAV_Version navVersion, int simulatedFrameRate,
    BNAV_Indexer_VideoFormat videoFormat, uint64_t startingOffset, unsigned startingTimestamp, int loops
    )
{
    unsigned char *bfr;
    FILE    *fin, *fout;
    sTsIndexer *indexer = NULL;
    sTsIndexer *indexer2 = NULL;
    BNAV_Indexer_Handle bcmindexer = NULL;
    unsigned long readBytes;
    unsigned long totalBytes = 0;
    double  time1, time2;
    unsigned long numBytesToRead;
    int index_input = (input == b_input_4word_sct) || (input == b_input_6word_sct);
    bool sct6;

    printHeader();

    time1 = getms();

    switch (input) {
    case b_input_4word_sct:
        numBytesToRead = NUM_SC4_BYTES_TO_READ;
        break;
    case b_input_6word_sct:
        numBytesToRead = NUM_SC6_BYTES_TO_READ;
        break;
    default:
        /* reading mpeg */
        numBytesToRead = NUM_STREAM_BYTES_TO_READ;
        break;
    }

    if (pid == 0 && !index_input)
        printf(
        "WARNING\n"
        "WARNING\n"
        "WARNING: Are you sure you want PID == 0 ??????????????\n"
        "WARNING\n"
        "WARNING\n"
        "\n"
        );
    if (index_input && output != b_output_bcm) {
        printf("If your input is a start code index, then -bcm is required.\n");
        exit(1);
    }

    bfr = malloc( numBytesToRead );

    if (!strcmp(inputfile, "-")) {
        fin = stdin;
        inputfile = "stdin";
    }
    else if( (fin = fopen(inputfile, "rb" )) == NULL )
    {
        printf("Unable to open input file %s\n", inputfile);
        exit(255);
    }

    if (!strcmp(outputfile, "-")) {
        fout = stdout;
        outputfile = "stdout";
    }
    else if( (fout = fopen(outputfile, "wb" )) == NULL )
    {
        printf("Unable to open output file %s\n", outputfile);
        fclose(fin);
        exit(255);
    }

    printf("Processing %s 0x%X from %s %s input file '%s'.\n",
        (input == b_input_pes)?"StreamID":"PID", pid,
        get_codec_str(videoFormat), b_input_str[input], inputfile);
    printf("Writing %s index to '%s'.\n", b_output_str[output], outputfile);

    /* only use sct4 if we must. sct6 is a required intermediate format for AVC, field-encoded MPEG, etc. */
    sct6 = (input != b_input_4word_sct) && (output != b_output_4word_sct);

    if (output == b_output_bcm)
    {
        BNAV_Indexer_Settings settings;
        BNAV_Indexer_GetDefaultSettings(&settings);
        settings.writeCallback = (INDEX_WRITE_CB)fwrite;
        settings.filePointer = (void *)fout;
        settings.navVersion = navVersion;
        settings.transportTimestampEnabled = timestamp_enabled;
        /* createindex must generate a timestamp somehow. it is either PTS or simulated framerate.
        if framerate not specified, we use PTS. */
        settings.ptsBasedFrameRate = (simulatedFrameRate == 0);
        settings.simulatedFrameRate = simulatedFrameRate;
        settings.sctVersion = sct6?BSCT_Version6wordEntry:BSCT_Version40bitOffset;
        settings.videoFormat = videoFormat;

        if (!index_input) {
            settings.mpegSizeCallback = mpegSizeCallback;
            g_mpegFile = fin;
        }

		settings.append.offsetHi = startingOffset >> 32;
		settings.append.offsetLo = startingOffset & 0xFFFFFFFF;
		settings.append.timestamp = startingTimestamp;

        /*printf("startingOffset %ld startingTimestamp %ld\n", settings.append.offsetLo, settings.append.timestamp);*/

        if (BNAV_Indexer_Open(&bcmindexer, &settings)) {
            exit(1);
        }
    }
    if (!index_input)
    {
        tsindex_settings settings;
        tsindex_settings_init(&settings);
        settings.pid = pid;
        settings.entry_size = sct6?6:4;
        if (output == b_output_bcm) {
            settings.cb = (INDEX_WRITE_CB)write_to_bcmindexer;
            settings.fp = (void*)bcmindexer;
        }
        else {
            settings.cb = (INDEX_WRITE_CB)fwrite;
            settings.fp = (void*)fout;
        }
        if (videoFormat != BNAV_Indexer_VideoFormat_MPEG2) {
            settings.start_code_lo = 0x00;
            settings.start_code_hi = 0xFF;
            settings.is_avc = 1;
        }
        indexer = tsindex_allocate_ex(&settings);
        if (input == b_input_pes)
            tsindex_setPesId( indexer, (uint8_t)pid);
        if(pid2) {
            settings.pid = pid2;
            if (output == b_output_bcm) {
                settings.cb = (INDEX_WRITE_CB)write_to_bcmindexer;
                settings.fp = (void*)bcmindexer;
            }
            indexer2 = tsindex_allocate_ex(&settings);
        }
    }

    while( loops >= 0 )
    {
        while( (readBytes = fread( bfr, 1, numBytesToRead, fin )) != 0 )
        {
            totalBytes += readBytes;

            if ( !g_quiet )
            {
                double rate1; char timeStr[32], offsetStr[32];
                time2 = getms() - time1;
                rate1 = (totalBytes/1024)/time2;
                TimetoString( (long)time2, timeStr );
                OffsetToString( totalBytes, offsetStr );
                fprintf(stderr, "Byte Offset: %s Rate: %0.1fK/s, Elapsed Time: %s      \r", offsetStr, rate1, timeStr );
            }

            switch (input) {
            case b_input_4word_sct:
                BNAV_Indexer_Feed(bcmindexer, bfr, readBytes/sizeof(BSCT_Entry) );
                break;
            case b_input_6word_sct:
                BNAV_Indexer_Feed(bcmindexer, bfr, readBytes/sizeof(BSCT_SixWord_Entry));
                break;
            case b_input_pes:
                tsindex_feedPes( indexer, bfr, readBytes );
                break;
            default:
                tsindex_feed( indexer, bfr, readBytes );
                if(indexer2)
                    tsindex_feed( indexer2, bfr, readBytes );
                break;
            }
        }
        if( loops > 0 )
        {
            fprintf(stderr, "\nLooping input file.");
            rewind(fin);
        }
        loops -= 1;
    }

    if (bcmindexer)
        BNAV_Indexer_Close( bcmindexer );
    if (indexer)
        tsindex_free( indexer );
    if (indexer2)
        tsindex_free( indexer2 );

    fclose( fin );
    fclose( fout );
    free( bfr );
    if (!g_quiet) {
        fprintf(stderr, "\n");
    }
    exit(0);
}

#if !defined(__vxworks)
void printUsage(void)
{
    printf(
    "Parses an MPEG stream or SCT index and creates either a SCT index\n"
    "  or Broadcom index.\n"
    "\n"
    "Usage: createindex inputfile outputfile [pid] [-in=XXX] [-out=XXX] [others, see below]\n"
    "\n"
    );
    printf(
    "  inputfile    Defaults to MPEG2 TS stream, unless specified with -in param.\n"
    "  outputfile   Defaults to SCT index if .sct or .idx suffix, or BCM index if\n"
    "                 .bcm or .nav suffix, unless specified with -out param.\n"
    "  pid          PID (or stream id in PES mode). Not needed if inputfile is\n"
    "                 a startcode index. Prefix with 0x for hex, otherwise decimal.\n"
    "\n"
    );
    printf(
    "Optional Parameters:\n"
    "  -in=ts       inputfile is a MPEG2 TS stream\n"
    "  -in=avc_ts   inputfile is a AVC TS stream\n"
    "  -in=vc1_pes  inputfile is a VC1 PES stream\n"
    "  -in=pes      inputfile is a MPEG2 PES stream\n"
    "  -in=sct      inputfile is a 4 word startcode index\n"
    "  -in=sct6     inputfile is a 6 word startcode index\n"
    );
    printf(
    "  -video=avc video is a AVC\n"
    "  -video=hevc video is a HEVC\n"
    "  -video=avc_svc video is a AVC with SVC extension\n"
    "  -video=avc_mvc video is a AVC with MVC extension\n"
    );
    printf(
    "  -out=sct     outputfile will be a 4 word startcode index\n"
    "  -out=sct6    outputfile will be a 6 word startcode index\n"
    "  -out=bcm     outputfile will be a BCM index file\n"
    "  -timestamp   transport timestamping present in the MPEG stream.\n"
    "  -bcmver VERSION   specify the BCM index format. See eBcmNavVersion.\n"
    );
    printf(
    "  -framerate RATE   specify the frame rate for timestamp emulation (default=30).\n"
    "  -append OFFSET TIMESTAMP specify that index will be appended at the given data file offset and timestamp.\n"
    "  -q           quiet. do not print status line.\n"
    );
    printf(
    "  -msg MODULE  enable debug output for the MODULE\n"
    "  -pid2 PID    PID for additional payload\n"
    "  -loops COUNT loop input file COUNT additional times when generating index\n"
    );
}

int main( int argc, char **argv )
{
    int pid = 0;
    int pid2 = 0;
    int nextparam = 3;
    b_input input = b_input_unknown;
    b_output output = b_output_unknown;
    int timestamp_enabled = 0;
    int simulatedFrameRate = 0;
    uint64_t startingOffset = 0;
    unsigned startingTimestamp = 0;
    const char *source;
    const char *dest;
    int rc;
    int loops = 0;
    BNAV_Indexer_VideoFormat videoFormat = BNAV_Indexer_VideoFormat_MPEG2;

    BNAV_Version navVersion = BNAV_VersionLatest;

    BKNI_Init();
    BDBG_Init();
    if (argc < 3) {
        printHeader();
        printUsage();
        rc=1;goto done;
    }

    source = argv[1];
    dest = argv[2];

    if (argc>3 && argv[3][0] != '-') {
        pid = strtoul(argv[3], NULL, 0);
        ++nextparam;
    }

    while (nextparam < argc) {
        if (!strcmp(argv[nextparam], "-in=ts")) {
            input = b_input_ts;
        }
        else if (!strcmp(argv[nextparam], "-in=pes")) {
            input = b_input_pes;
        }
        else if (!strcmp(argv[nextparam], "-in=sct")) {
            input = b_input_4word_sct;
        }
        else if (!strcmp(argv[nextparam], "-in=sct6")) {
            input = b_input_6word_sct;
        }
        else if (!strcmp(argv[nextparam], "-in=pts_ts")) {
            /* index any TS stream using PTS's */
            input = b_input_ts;
            navVersion = BNAV_Version_TimestampOnly;
        }
        else if (!strcmp(argv[nextparam], "-in=avc_ts")) {
            input = b_input_ts;
            videoFormat = BNAV_Indexer_VideoFormat_AVC;
            navVersion = BNAV_Version_AVC;
        }
        else if (!strcmp(argv[nextparam], "-out=bcm")) {
            output = b_output_bcm;
        }
        else if (!strcmp(argv[nextparam], "-video=avc")) {
            navVersion = BNAV_Version_AVC;
            videoFormat = BNAV_Indexer_VideoFormat_AVC;
        }
        else if (!strcmp(argv[nextparam], "-video=avc_svc")) {
            navVersion = BNAV_Version_AVC_Extended;
            videoFormat = BNAV_Indexer_VideoFormat_AVC_SVC;
        }
        else if (!strcmp(argv[nextparam], "-video=avc_mvc")) {
            navVersion = BNAV_Version_AVC_Extended;
            videoFormat = BNAV_Indexer_VideoFormat_AVC_MVC;
        }
        else if (!strcmp(argv[nextparam], "-video=hevc")) {
            navVersion = BNAV_Version_HEVC;
            videoFormat = BNAV_Indexer_VideoFormat_HEVC;
        }
        else if (!strcmp(argv[nextparam], "-out=sct6")) {
            output = b_output_6word_sct;
        }
        else if (!strcmp(argv[nextparam], "-out=sct")) {
            output = b_output_4word_sct;
        }
        else if (!strcmp(argv[nextparam], "-timestamp")) {
            timestamp_enabled = 1;
        }
        else if (!strcmp(argv[nextparam], "-q")) {
            g_quiet = 1;
        }
        else if (!strcmp(argv[nextparam], "-msg") && nextparam+1 < argc) {
            nextparam++;
            BDBG_SetModuleLevel(argv[nextparam], BDBG_eMsg);
        }
        else if (!strcmp(argv[nextparam], "-pid2") && nextparam+1 < argc) {
            nextparam++;
            pid2 = strtoul(argv[nextparam], NULL, 0);
            navVersion = BNAV_Version_AVC_Extended;
        }
        else if (!strcmp(argv[nextparam], "-loops") && nextparam+1 < argc) {
            nextparam++;
            loops = strtoul(argv[nextparam], NULL, 0);
        }
        else if (!strcmp(argv[nextparam], "-bcmver") && nextparam+1 < argc) {
            navVersion = atoi(argv[++nextparam]);
        }
        else if (!strcmp(argv[nextparam], "-framerate") && nextparam+1 < argc) {
            simulatedFrameRate = atoi(argv[++nextparam]);
        }
        else if (!strcmp(argv[nextparam], "-append") && nextparam+2 < argc) {
            startingOffset = atoi(argv[++nextparam]);
            startingTimestamp = atoi(argv[++nextparam]);
        }
        else {
            printf("Invalid param: %s\n\n", argv[nextparam]);
            printUsage();
            rc = -1; goto done;

        }
        nextparam++;
    }

    if (input == b_input_unknown) {
        char *s;
        /* check the suffix of the inputfile */
        if (((s = strstr(source, ".sct")) && !s[4]) ||
            ((s = strstr(source, ".idx")) && !s[4]))
        {
            input = b_input_4word_sct;
        }
        else if ((s = strstr(source, ".pes")) && !s[4])
        {
            input = b_input_pes;
        }
        else
            input = b_input_ts;
    }
    if (output == b_output_unknown) {
        char *s;
        if (((s = strstr(dest, ".nav")) && !s[4]) ||
            ((s = strstr(dest, ".bcm")) && !s[4]))
            output = b_output_bcm;
        else
            output = b_output_4word_sct;
    }

    rc = createindex(source,dest,pid,pid2,input,output,timestamp_enabled,navVersion,simulatedFrameRate, videoFormat, startingOffset, startingTimestamp, loops);
done:
    BDBG_Uninit();
    BKNI_Uninit();
    return rc;
}
#endif


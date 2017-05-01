/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
 *****************************************************************************/

#include "atlas.h"
#include "atlas_os.h"
#include "cputest.h"

#define CALLBACK_CPUTEST  "CallbackCpuTest"

BDBG_MODULE(atlas_cputest);

#if __cplusplus
extern "C" {
#include "jpeglib.h"
}
#endif

#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h> /* file i/o */
#include <fcntl.h>

static uint32_t readProcStatCores(void);
static eRet     readProcStatCpuUtil(FILE * fp, uint64_t * fields);

static void b_init_source(j_decompress_ptr cinfo)
{
    BSTD_UNUSED(cinfo);
    /* nothing */
}

static boolean b_fill_input_buffer(j_decompress_ptr cinfo)
{
    BSTD_UNUSED(cinfo);
    /* already filled */
    return(true);
}

static void b_skip_input_data(
        j_decompress_ptr cinfo,
        long             num_bytes
        )
{
    cinfo->src->bytes_in_buffer -= num_bytes;
    cinfo->src->next_input_byte += num_bytes;
}

static void b_term_source(j_decompress_ptr cinfo)
{
    BSTD_UNUSED(cinfo);
    /* nothing */
}

static void b_jpeg_mem_src(
        j_decompress_ptr      cinfo,
        const unsigned char * buffer,
        int                   length
        )
{
    if (cinfo->src == NULL) /* first time for this JPEG object? */
    {
        cinfo->src = (struct jpeg_source_mgr *)
                     (*cinfo->mem->alloc_small)((j_common_ptr) cinfo, JPOOL_PERMANENT,
                sizeof(struct jpeg_source_mgr));
        /* TODO: is this automatically dealloced? */
    }

    cinfo->src->init_source       = b_init_source;
    cinfo->src->fill_input_buffer = b_fill_input_buffer;
    cinfo->src->skip_input_data   = b_skip_input_data;
    cinfo->src->resync_to_restart = jpeg_resync_to_restart; /* use default*/
    cinfo->src->term_source       = b_term_source;
    cinfo->src->bytes_in_buffer   = length;
    cinfo->src->next_input_byte   = buffer;
} /* b_jpeg_mem_src */

static void cpuTestThread(void * pParam)
{
    CCpuTest *                    pCpuTest = (CCpuTest *)pParam;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr         jerr;
    int             row_stride = 0;
    unsigned char * out        = NULL;
    eRet            ret        = eRet_Ok;

    BDBG_ASSERT(NULL != pCpuTest);
    BDBG_ASSERT(NULL != pCpuTest->getImageBuffer());

    cinfo.err = jpeg_std_error(&jerr);

    while (true == pCpuTest->getThreadRun())
    {
        int count = 0;

        jpeg_create_decompress(&cinfo);
        b_jpeg_mem_src(&cinfo, pCpuTest->getImageBuffer(), pCpuTest->getImageSize());
        jpeg_read_header(&cinfo, TRUE);
        jpeg_start_decompress(&cinfo);

        row_stride = cinfo.output_width * cinfo.output_components;
        out        = (unsigned char *)BKNI_Malloc(cinfo.output_width*cinfo.output_height*cinfo.output_components);

        while (cinfo.output_scanline < cinfo.output_height)
        {
            unsigned char * rowp[1];
            rowp[0] = (unsigned char *) out + row_stride * cinfo.output_scanline;

            jpeg_read_scanlines(&cinfo, rowp, 1);
            count++;
        }

        if (NULL != out)
        {
            BKNI_Free(out);
            out = NULL;
        }

        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);

        /* BDBG_WRN(("CPU TEST: decompressed jpeg scanline count:%d", count)); */
        BKNI_Sleep(pCpuTest->getDelay());
    }

error:
    return;
} /* cpuTestThread */

/* bwin io callback that is executed when wifi thread io is triggered.
 * this function handles the responses from worker thread commands
 */
static void bwinCpuTestCallback(
        void *       pObject,
        const char * strCallback
        )
{
    eRet       ret      = eRet_Ok;
    CCpuTest * pCpuTest = (CCpuTest *)pObject;

    BDBG_MSG(("%s - sync'd with main loop", __FUNCTION__));

    BDBG_ASSERT(NULL != pCpuTest);
    BSTD_UNUSED(strCallback);

    /* pCpuTest->processXXX(strCallback); */
}

/*
 * Constructor
 * widgetEngine - the bwidget engine that will be synchronized with when the timer
 *                expires.
 */
CCpuTest::CCpuTest(
        CWidgetEngine *  pWidgetEngine,
        CConfiguration * pCfg
        ) :
    CMvcModel("CCpuTest"),
    _pWidgetEngine(pWidgetEngine),
    _pCfg(pCfg),
    _nLevel(0),
    _nDelay(0),
    _bThreadRun(false),
    _imgBuffer(NULL),
    _imgSize(0)
{
    BDBG_ASSERT(_pWidgetEngine);

    _pWidgetEngine->addCallback(this, CALLBACK_CPUTEST, bwinCpuTestCallback, ePowerMode_S3);
}

CCpuTest::~CCpuTest()
{
    stop();

    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_CPUTEST);
        _pWidgetEngine = NULL;
    }
}

eRet CCpuTest::start(uint32_t nLevel)
{
    eRet ret = eRet_Ok;

    _nLevel = nLevel;

    _nDelay = (100 - nLevel) * GET_INT(_pCfg, CPUTEST_MAX_DELAY) / 100;
    BDBG_WRN(("CCpuTest::start() level:%d delay:%d", nLevel / 10, _nDelay));

    if (true == isStarted())
    {
        goto done;
    }

    /* read test image file into buffer */
    {
        FILE *      fpImage = NULL;
        MString     strImageFile("images/cputest.jpg");
        struct stat stIndex;

        fpImage = fopen64(strImageFile, "rb");
        CHECK_PTR_ERROR_GOTO("unable to open cputest.jpg - disabling cpuTest", fpImage, ret, eRet_NotAvailable, errorRead);

        if (0 > fstat(fileno(fpImage), &stIndex))
        {
            BDBG_ERR(("stat returned an error on cputest.jpg - disabling cpuTest"));
            goto errorRead;
        }

        _imgBuffer = (unsigned char *)BKNI_Malloc(stIndex.st_size);
        if (_imgBuffer == NULL)
        {
            BDBG_ERR(("unable to allocate _imgBuffer"));
            ret = eRet_OutOfMemory;
            goto errorRead;
        }

        _imgSize = fread(_imgBuffer, sizeof(char), stIndex.st_size, fpImage);

errorRead:
        if (NULL != fpImage)
        {
            fclose(fpImage);
            fpImage = NULL;
        }
    }

    /* read number of cores and start cpu test threads */
    {
        B_ThreadSettings settings;
        B_EventSettings  eventSettings;
        uint32_t         numCores = readProcStatCores();

        /* start cputest threads */
        setThreadRun(true);
        B_Thread_GetDefaultSettings(&settings);

        for (uint32_t i = 0; i < numCores; i++)
        {
            B_ThreadHandle * pThread = NULL;

            pThread = new B_ThreadHandle;

            *pThread = B_Thread_Create(MString("atlas_cputest_") + MString(i), cpuTestThread, (void *)this, &settings);
            CHECK_PTR_ERROR("unable to start cpu test thread", *pThread, ret, eRet_NotAvailable);
            BDBG_WRN(("start cputest thread %d", i));

            _threadList.add(pThread);
        }
    }
error:
done:
    notifyObservers(eNotify_CpuTestStarted, this);
    return(ret);
} /* start */

eRet CCpuTest::stop()
{
    eRet             ret     = eRet_Ok;
    B_ThreadHandle * pThread = NULL;

    if (false == isStarted())
    {
        goto done;
    }

    setThreadRun(false);
    while (NULL != (pThread = _threadList.remove(0)))
    {
        B_Thread_Destroy(*pThread);
    }

    _nLevel = 0;

    if (NULL != _imgBuffer)
    {
        BKNI_Free(_imgBuffer);
        _imgBuffer = NULL;
        _imgSize   = 0;
    }

done:
    notifyObservers(eNotify_CpuTestStopped, this);
    return(ret);
} /* stop */

void CCpuTest::setWidgetEngine(CWidgetEngine * pWidgetEngine)
{
    if (NULL != pWidgetEngine)
    {
        pWidgetEngine->addCallback(this, CALLBACK_CPUTEST, bwinCpuTestCallback, ePowerMode_S3);
    }
    else
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_CPUTEST);
    }

    _pWidgetEngine = pWidgetEngine;
}

static uint32_t readProcStatCores()
{
    eRet     ret      = eRet_Ok;
    int      retval   = 0;
    FILE *   fp       = NULL;
    uint32_t numCores = 0;
    char     buffer[1024];
    int      sizeBuffer = 0;
    MString  strBuffer;

    fp = fopen("/proc/stat", "r");
    CHECK_PTR_ERROR_GOTO("unable to access /proc/stat", fp, ret, eRet_NotAvailable, error);

    /* coverity[secure_coding] */
    sizeBuffer         = fread(buffer, sizeof(char), sizeof(buffer) - 1, fp);
    buffer[sizeBuffer] = '\0';

    strBuffer = buffer;

    /* count cpu0, cpu1, etc to determine max number of cores */
    for (numCores = 0; -1 != strBuffer.find(MString("cpu") + MString(numCores)); numCores++)
    {
    }

error:
    if (NULL != fp)
    {
        fclose(fp);
        fp = NULL;
    }

    return(numCores);
} /* readProcStatCores */

static eRet readProcStatCpuUtil(
        FILE *     fp,
        uint64_t * fields
        )
{
    eRet ret    = eRet_Ok;
    int  retval = 0;
    char buffer[1024];

    memset(buffer, 0, sizeof(buffer));

    /* coverity[secure_coding] */
    fgets(buffer, sizeof(buffer) - 1, fp);
    /* coverity[secure_coding] */
    retval = sscanf(buffer, "cpu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu",
            &fields[0], &fields[1], &fields[2], &fields[3],
            &fields[4], &fields[5], &fields[6], &fields[7],
            &fields[8], &fields[9]);

    if (4 > retval) /* Atleast 4 fields is to be read */
    {
        BDBG_ERR(("Error reading /proc/stat cpu field"));
        return(eRet_NotSupported);
    }
    return(eRet_Ok);
} /* readProcStatCpuUtil */

double CCpuTest::getCpuUtilization()
{
    /* calc and update idle time */
    eRet            ret            = eRet_Ok;
    FILE *          fp             = NULL;
    static uint64_t total_tick_old = 0;
    static uint64_t idle_old       = 0;
    uint64_t        total_tick     = 0;
    uint64_t        idle           = 0;
    uint64_t        del_total_tick = 0;
    uint64_t        del_idle       = 0;
    int             i              = 0;
    double          percent_usage  = 0;
    uint64_t        fields[10];

    fp = fopen("/proc/stat", "r");
    CHECK_PTR_ERROR_GOTO("unable to access /proc/stat", fp, ret, eRet_NotAvailable, error);

    ret = readProcStatCpuUtil(fp, fields);
    CHECK_ERROR_GOTO("unable to read cpu util", ret, error);

    for (i = 0, total_tick = 0; i < 10; i++)
    {
        total_tick += fields[i];
    }
    idle = fields[3]; /* idle ticks index */

    if (0 == total_tick_old)
    {
        /* first call so save and handle on next call */
        total_tick_old = total_tick;
        idle_old       = idle;
        goto error;
    }

    {
        char strPercent[8];

        fseek(fp, 0, SEEK_SET);
        fflush(fp);
        ret = readProcStatCpuUtil(fp, fields);
        CHECK_ERROR_GOTO("unable to read cpu util", ret, error);

        for (i = 0, total_tick = 0; i < 10; i++)
        {
            total_tick += fields[i];
        }
        idle = fields[3];

        del_total_tick = total_tick - total_tick_old;
        del_idle       = idle - idle_old;

        percent_usage = ((del_total_tick - del_idle) / (double) del_total_tick) * 100; /* 3 is index of idle time */
        snprintf(strPercent, sizeof(strPercent), "%3.lf%%", percent_usage);
        /* BDBG_ERR(("%3.2lf%%id", percent_usage)); */

        /* save for next time */
        total_tick_old = total_tick;
        idle_old       = idle;
    }
error:
    if (fp)
    {
        fclose(fp);
    }
    return(percent_usage);
} /* getCpuUtilization */
/***************************************************************************
*     Copyright (c) 2004-2013, Broadcom Corporation
*     All Rights Reserved
*     Confidential Property of Broadcom Corporation
*
*  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
*  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
*  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
*
***************************************************************************/
#if NEXUS_HAS_SAGE && NEXUS_HAS_VIDEO_DECODER && NEXUS_HAS_DISPLAY
#include "nexus_platform.h"

#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BDBG_MODULE(heap_create);

/* This stand alone nexus app attempts to verify secure/unsecure heap configurations.
* By default it will test basic configurations ("default" (which should be only
* unsecure), only secure, and full double allocation (all secure + unsecure)) */

static const char *memory_type_to_string(unsigned memoryType, char *buf, size_t buf_size)
{
    if(memoryType==0)
    {
        BKNI_Snprintf(buf, buf_size, "%s", "eDevice");
    }
    else
    {
        static const struct {unsigned value; const char *name;} g_mapping[] = {
            {NEXUS_MEMORY_TYPE_DRIVER_CACHED,"DRIVER"},
            {NEXUS_MEMORY_TYPE_APPLICATION_CACHED,"APP"},
            {NEXUS_MEMORY_TYPE_SECURE,"SECURE"},
            {NEXUS_MEMORY_TYPE_RESERVED,"RESERVED"},
            {NEXUS_MEMORY_TYPE_MANAGED,"MANAGED"},
            {NEXUS_MEMORY_TYPE_NOT_MAPPED,"NOT_MAPPED"},
            {NEXUS_MEMORY_TYPE_HIGH_MEMORY,"HIGH_MEMORY"},
            {NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED,"ONDEMAND_MAPPED"}};
        unsigned n = 0, i;
        for (i=0;i<sizeof(g_mapping)/sizeof(g_mapping[0]) && n<buf_size;i++) {
            if (memoryType & g_mapping[i].value) {
                n += BKNI_Snprintf(&buf[n], buf_size-n, "%s%s", n?",":"", g_mapping[i].name);
            }
        }
    }
    return buf;
}

static const char *heap_type_to_string(unsigned heapType, char *buf, size_t buf_size)
{
    static const struct {unsigned value; const char *name;} g_mapping[] = {
        {NEXUS_HEAP_TYPE_MAIN,"MAIN"},
        {NEXUS_HEAP_TYPE_PICTURE_BUFFERS,"PICTURE"},
        {NEXUS_HEAP_TYPE_SAGE_RESTRICTED_REGION,"SAGE"},
        {NEXUS_HEAP_TYPE_COMPRESSED_RESTRICTED_REGION,"CDB"},
        {NEXUS_HEAP_TYPE_GRAPHICS,"GFX"},
        {NEXUS_HEAP_TYPE_SECONDARY_GRAPHICS,"2ND_GFX"}};
    unsigned n = 0, i;
    for (i=0;i<sizeof(g_mapping)/sizeof(g_mapping[0]) && n<buf_size;i++) {
        if (heapType & g_mapping[i].value) {
            n += BKNI_Snprintf(&buf[n], buf_size-n, "%s%s", n?",":"", g_mapping[i].name);
        }
    }

    return buf;
}

static const char *secure_type_to_string(NEXUS_SecureVideo secure)
{
    switch(secure)
    {
        case NEXUS_SecureVideo_eUnsecure:
            return "U";
            break;
        case NEXUS_SecureVideo_eSecure:
            return "S";
            break;
        case NEXUS_SecureVideo_eBoth:
            return "SU";
            break;
        default:
            break;
    }

    return "?";
}

static void print_heap(unsigned i, NEXUS_HeapHandle heap, const NEXUS_PlatformConfiguration *platformConfig,
    unsigned *total, unsigned *unsecure, unsigned *secure)
{
    NEXUS_MemoryStatus status;
    char buf[2][128];
    unsigned size_percentage;
    NEXUS_Error rc;
    unsigned serverHeapIndex;
    rc = NEXUS_Heap_GetStatus(heap, &status);
    if (rc) return;
    size_percentage = status.size/100; /* avoid overflow by dividing size by 100 to get percentage */
    for (serverHeapIndex=0;serverHeapIndex<NEXUS_MAX_HEAPS;serverHeapIndex++) {
        if (platformConfig->heap[serverHeapIndex] == heap) break;
    }
    printf("%-2d\t" BDBG_UINT64_FMT "\t%d\t0x%08x(%3dMB)@0x%08x\t%3d%%/%3d%%\t%u\t%s : %s\n",
        i, BDBG_UINT64_ARG(status.offset), status.memcIndex, status.size, status.size/(1024*1024), (unsigned)status.addr,
        size_percentage?(status.size-status.free)/size_percentage:0,
        size_percentage?status.highWatermark/size_percentage:0,
        serverHeapIndex,
        memory_type_to_string(status.memoryType, buf[0], sizeof(buf[0])),
        heap_type_to_string(status.heapType, buf[1], sizeof(buf[1])));

    if(status.heapType&NEXUS_HEAP_TYPE_PICTURE_BUFFERS)
    {
        *total+=status.size;
        if(status.memoryType&NEXUS_MEMORY_TYPE_SECURE)
        {
            *secure+=status.size;
        }
        else
        {
            *unsecure+=status.size;
        }
    }
}

static void print_heaps(void)
{
    unsigned i;
    NEXUS_ClientConfiguration clientConfig;
    NEXUS_PlatformConfiguration platformConfig;
    unsigned picture_heap_total=0;
    unsigned picture_heap_unsecure=0;
    unsigned picture_heap_secure=0;

    NEXUS_Platform_GetClientConfiguration(&clientConfig);
    NEXUS_Platform_GetConfiguration(&platformConfig);
    printf("heap\toffset\t\tmemc\tsize@offset\t\t\tused/peak\tserver\tinfo\n");
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        if (clientConfig.heap[i]) {
            print_heap(i, clientConfig.heap[i], &platformConfig, &picture_heap_total, &picture_heap_unsecure, &picture_heap_secure);
        }
    }

    printf("PICTURE HEAP TOTAL: 0x%08x (%dMB)\n", picture_heap_total, picture_heap_total>>20);
    printf("PICTURE HEAP (SECURE): 0x%08x (%dMB)\n", picture_heap_secure, picture_heap_secure>>20);
    printf("PICTURE HEAP (UNSECURE): 0x%08x (%dMB)\n", picture_heap_unsecure, picture_heap_unsecure>>20);
}

NEXUS_Error test_heap(NEXUS_MemoryConfigurationSettings *memConfigSettings)
{
    NEXUS_Error rc=NEXUS_SUCCESS;
    NEXUS_PlatformSettings platformSettings;
    int vd, dsp, wnd;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;

    /* Show this test's heap settings */
    printf("========= HEAP TEST =============\n");
    for(vd=0;vd<NEXUS_MAX_VIDEO_DECODERS;vd++)
    {
        printf(" VIDEO DECODER[%d]: %s\n", vd,
            secure_type_to_string(memConfigSettings->videoDecoder[vd].secure));
    }
    for(dsp=0;dsp<NEXUS_MAX_DISPLAYS;dsp++)
    {
        printf(" DISPLAY[%d]: ",dsp);
        for(wnd=0;wnd<NEXUS_MAX_VIDEO_WINDOWS;wnd++)
        {
            printf(" {WINDOW[%d]: %s}", wnd,
                secure_type_to_string(memConfigSettings->display[dsp].window[wnd].secure));
        }
        printf("\n");
    }

    /* Open */
    rc = NEXUS_Platform_MemConfigInit(&platformSettings, memConfigSettings);
    if (rc) return rc;

    /* Show the heaps */
    print_heaps();

    /* Cleanup */
    NEXUS_Platform_Uninit();

    return rc;
}

int test_vdc(int vd, NEXUS_MemoryConfigurationSettings *memConfigSettings)
{
    int i;
    NEXUS_Error rc=NEXUS_SUCCESS;

    if(vd==NEXUS_MAX_VIDEO_DECODERS)
    {
        rc=test_heap(memConfigSettings);
        return rc;
    }

    for(i=(vd+1);i<=NEXUS_MAX_VIDEO_DECODERS;i++)
    {
        memConfigSettings->videoDecoder[vd].secure=NEXUS_SecureVideo_eUnsecure;

        rc|=test_vdc(i, memConfigSettings);

        memConfigSettings->videoDecoder[vd].secure=NEXUS_SecureVideo_eSecure;

        rc|=test_vdc(i, memConfigSettings);

        memConfigSettings->videoDecoder[vd].secure=NEXUS_SecureVideo_eBoth;

        rc|=test_vdc(i, memConfigSettings);
    }

    return 0;
}

int test_dsp(int dsp, int wnd, NEXUS_MemoryConfigurationSettings *memConfigSettings)
{
    int i,j;
    NEXUS_Error rc=NEXUS_SUCCESS;

    if((dsp==NEXUS_NUM_DISPLAYS)||(wnd==NEXUS_NUM_VIDEO_WINDOWS))
    {
        rc=test_vdc(0, memConfigSettings);
        return rc;
    }

    for(i=dsp;i<=NEXUS_NUM_DISPLAYS;i++)
    {
        for(j=(wnd+1);j<=NEXUS_NUM_VIDEO_WINDOWS;j++)
        {
            memConfigSettings->display[dsp].window[wnd].secure=NEXUS_SecureVideo_eUnsecure;

            rc|=test_dsp(i, j, memConfigSettings);

            memConfigSettings->display[dsp].window[wnd].secure=NEXUS_SecureVideo_eSecure;

            rc|=test_dsp(i, j, memConfigSettings);

            memConfigSettings->display[dsp].window[wnd].secure=NEXUS_SecureVideo_eBoth;

            rc|=test_dsp(i, j, memConfigSettings);
        }
    }

    return rc;
}

void show_usage(char *app)
{
    printf("%s usage:\n", app);
    printf("\t -a \t Test all heap combinations\n");
    printf("\t -h \t This help\n");
}

int main(int argc, char **argv)
{
    NEXUS_Error rc=NEXUS_SUCCESS;
    int vd, dsp, wnd;
    NEXUS_MemoryConfigurationSettings memConfigSettings;
    int i;
    bool all_configs=false;

    BDBG_ASSERT(!NEXUS_SUCCESS);

    for(i=1;i<argc;i++)
    {
        if(argv[i][0]=='-')
        {
            switch(argv[i][1])
            {
                case 'h':
                    show_usage(argv[0]);
                    return 0;
                    break;
                case 'a':
                    all_configs=true;
                    break;
                default:
                    break;
            }
        }
    }

    if(all_configs)
    {
        NEXUS_GetDefaultMemoryConfigurationSettings(&memConfigSettings);
        rc=test_dsp(0, 0, &memConfigSettings);
        return rc;
    }

    /* Defaults (should be all unsecure) */
    NEXUS_GetDefaultMemoryConfigurationSettings(&memConfigSettings);
    rc|=test_heap(&memConfigSettings);

    /* All Secure and unsecure */
    NEXUS_GetDefaultMemoryConfigurationSettings(&memConfigSettings);
    for(vd=0;vd<NEXUS_MAX_VIDEO_DECODERS;vd++)
    {
            memConfigSettings.videoDecoder[vd].secure=NEXUS_SecureVideo_eBoth;
    }
    for(dsp=0;dsp<NEXUS_MAX_DISPLAYS;dsp++)
    {
        for(wnd=0;wnd<NEXUS_MAX_VIDEO_WINDOWS;wnd++)
        {
            memConfigSettings.display[dsp].window[wnd].secure=NEXUS_SecureVideo_eBoth;
        }
    }
    rc|=test_heap(&memConfigSettings);

    /* Only secure */
    NEXUS_GetDefaultMemoryConfigurationSettings(&memConfigSettings);
    for(vd=0;vd<NEXUS_MAX_VIDEO_DECODERS;vd++)
    {
            memConfigSettings.videoDecoder[vd].secure=NEXUS_SecureVideo_eSecure;
    }
    for(dsp=0;dsp<NEXUS_MAX_DISPLAYS;dsp++)
    {
        for(wnd=0;wnd<NEXUS_MAX_VIDEO_WINDOWS;wnd++)
        {
            memConfigSettings.display[dsp].window[wnd].secure=NEXUS_SecureVideo_eSecure;
        }
    }
    rc|=test_heap(&memConfigSettings);

    return rc;
}
#else
#include <stdio.h>
int main(void)
{
    printf("test not supported\n");
    return 0;
}
#endif

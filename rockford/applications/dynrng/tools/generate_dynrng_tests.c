/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *
 * Module Description:
 *
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_VIDS 4
#define NO_HLG_TVS 1
#define MAX_FILE_NAME_LEN 32
#define MAX_CAPS_LEN 64
#define MAX_DESCRIPTION_LEN 128
#define TEST_L1GROUP_SIZE 1000
#define TEST_L2GROUP_SIZE 100
#define TEST_L3GROUP_SIZE 25

typedef enum DynamicRange
{
    DynamicRange_eNone,
    DynamicRange_eSdr,
    DynamicRange_eHdr10,
    DynamicRange_eHlg,
    DynamicRange_eVidMax,
    DynamicRange_eGfxMax = DynamicRange_eSdr + 1,
    DynamicRange_eLegacy = DynamicRange_eNone
} DynamicRange;

typedef enum TestL1Type
{
    TestL1Type_ePassthrough,
    TestL1Type_eConversion,
    TestL1Type_eMax
} TestL1Type;

typedef enum TestL2Type
{
    TestL2Type_eBasic,
    TestL2Type_eTransition,
    TestL2Type_eHotplug,
    TestL2Type_eGraphics,
    TestL2Type_eMax
} TestL2Type;

typedef enum TestBasicL3Type
{
    TestBasicL3Type_eSingle,
    TestBasicL3Type_eMainPip,
    TestBasicL3Type_eMultipip,
    TestBasicL3Type_eMax
} TestBasicL3Type;

typedef enum TestTransitionL3Type
{
    TestTransitionL3Type_eInput,
    TestTransitionL3Type_eOutput,
    TestTransitionL3Type_eInputOutput,
    TestTransitionL3Type_eMax
} TestTransitionL3Type;

typedef struct AppContext
{
    unsigned long totalTestCount;
    unsigned long testNum;
    FILE * summary;
    TestL1Type l1Type;
    TestL2Type l2Type;
    union
    {
        TestBasicL3Type basic;
        TestTransitionL3Type transition;
        int hotplug;
        int graphics;
    } l3Type;
} AppContext;

static const char * testL1TypeStrings[] =
{
    "passthru",
    "conversion",
    NULL
};

static const char * testL2TypeStrings[] =
{
    "basic",
    "transition",
    "hotplug",
    "graphics",
    NULL
};

static const char * testBasicL3TypeStrings[] =
{
    "single",
    "main+pip",
    "multipip",
    NULL
};

static const char * testTransitionL3TypeStrings[] =
{
    "input",
    "output",
    "i/o",
    NULL
};

static const char * testHotplugL3TypeStrings[] =
{
    NULL
};

static const char * testGraphicsL3TypeStrings[] =
{
    NULL
};

static const char * const dynrngInputStrings[DynamicRange_eVidMax + 1] =
{
    "N/A",
    "SDR",
    "HDR10",
    "HLG",
    NULL
};

static const char * const dynrngOutputStrings[DynamicRange_eVidMax + 1] =
{
    "LEGACY",
    "SDR",
    "HDR10",
    "HLG",
    NULL
};

static const char * const dynrngSettingStrings[DynamicRange_eVidMax + 1] =
{
    "disabled",
    "sdr",
    "hdr10",
    "hlg",
    NULL
};

static void print_summary_header(AppContext * pCtx)
{
    fprintf(pCtx->summary, "<html>\n");
    fprintf(pCtx->summary, "<head>\n");
    fprintf(pCtx->summary, "<title>Dynrng Test Summary</title>\n");
    fprintf(pCtx->summary, "<style>\n");
    fprintf(pCtx->summary, "table, tr, td, th {\n");
    fprintf(pCtx->summary, "  border: 1px solid black;\n");
    fprintf(pCtx->summary, "  border-collapse: collapse;\n");
    fprintf(pCtx->summary, "  text-align: center;\n");
    fprintf(pCtx->summary, "  vertical-align: center;\n");
    fprintf(pCtx->summary, "  padding: 0px;\n");
    fprintf(pCtx->summary, "  padding-left: 1px;\n");
    fprintf(pCtx->summary, "  padding-right: 1px;\n");
    fprintf(pCtx->summary, "}\n");
    fprintf(pCtx->summary, "table thead tr {\n");
    fprintf(pCtx->summary, "  background-color: green;\n");
    fprintf(pCtx->summary, "  color: white;\n");
    fprintf(pCtx->summary, "}\n");
    fprintf(pCtx->summary, "table tbody tr:nth-child(even) { background-color: #f2f2f2 }\n");
    fprintf(pCtx->summary, "table tbody tr:hover { background-color: #ddd }\n");
    fprintf(pCtx->summary, "td {\n");
    fprintf(pCtx->summary, "  white-space: no-wrap;\n");
    fprintf(pCtx->summary, "}\n");
    fprintf(pCtx->summary, "td.notes {\n");
    fprintf(pCtx->summary, "  white-space: pre;\n");
    fprintf(pCtx->summary, "  text-align: left;\n");
    fprintf(pCtx->summary, "  width: 100%%;\n");
    fprintf(pCtx->summary, "}\n");
    fprintf(pCtx->summary, "</style>\n");
    fprintf(pCtx->summary, "</head>\n");
    fprintf(pCtx->summary, "<body>\n");
    fprintf(pCtx->summary, "<h1>Dynrng Test Summary</h1>\n");
#if NO_HLG_TVS
    fprintf(pCtx->summary, "<i>Note: generated with NO_HLG_TVS enabled.  When HLG output is called for, we will use an SDR TV instead.</i>\n");
#endif
    fprintf(pCtx->summary, "</br>\n");
    fprintf(pCtx->summary, "</br>\n");
    fprintf(pCtx->summary, "<table style=\"width: 100%%\">\n");
    fprintf(pCtx->summary, "<thead>\n");
    fprintf(pCtx->summary, "<tr>\n");
    fprintf(pCtx->summary, "  <th rowspan=\"2\">Num</th>\n");
    fprintf(pCtx->summary, "  <th colspan=\"3\">Categories</th>\n");
    fprintf(pCtx->summary, "  <th colspan=\"6\">Dynrng Mode</th>\n");
    fprintf(pCtx->summary, "  <th rowspan=\"2\">Notes</th>\n");
    fprintf(pCtx->summary, "</tr>\n");
    fprintf(pCtx->summary, "<tr>\n");
    fprintf(pCtx->summary, "  <th>L1</th>\n");
    fprintf(pCtx->summary, "  <th>L2</th>\n");
    fprintf(pCtx->summary, "  <th>L3</th>\n");
    fprintf(pCtx->summary, "  <th>GFX</th>\n");
    fprintf(pCtx->summary, "  <th>VID0</th>\n");
    fprintf(pCtx->summary, "  <th>VID1</th>\n");
    fprintf(pCtx->summary, "  <th>VID2</th>\n");
    fprintf(pCtx->summary, "  <th>VID3</th>\n");
    fprintf(pCtx->summary, "  <th>OUT</th>\n");
    fprintf(pCtx->summary, "</tr>\n");
    fprintf(pCtx->summary, "</thead>\n");
    fprintf(pCtx->summary, "<tbody>\n");
}

static void print_summary_footer(AppContext * pCtx)
{
    fprintf(pCtx->summary, "</tbody>\n");
    fprintf(pCtx->summary, "</table>\n");
    fprintf(pCtx->summary, "<h2>%lu Test Cases Generated</h2>\n", pCtx->totalTestCount);
    fprintf(pCtx->summary, "</body>\n");
    fprintf(pCtx->summary, "</html>\n");
}

static void print_rx_caps(char * buf, size_t capacity, bool rxcaps[DynamicRange_eVidMax])
{
    DynamicRange r;
    size_t used;
    size_t len;

    used = 0;
    for (r = DynamicRange_eLegacy; r < DynamicRange_eVidMax; r++)
    {
        if (rxcaps[r])
        {
            len = snprintf(buf + used, capacity - used, " %s", dynrngOutputStrings[r]);
            if (len < 0) return;
            used += len;
        }
    }
}

static void print_test_vars(char * buf, size_t capacity, DynamicRange gfx, DynamicRange vid[MAX_VIDS], bool multipip, DynamicRange out)
{
    int i;
    DynamicRange r;
    size_t len;
    size_t used;

    used = 0;
    len = snprintf(buf + used, capacity - used, "GFX = %s; ", dynrngInputStrings[gfx]);
    if (len < 0) return;
    used += len;
    if (multipip)
    {
        for (i = 0; i < MAX_VIDS; i++)
        {
            len = snprintf(buf + used, capacity - used, "VID%d = %s; ", i, dynrngInputStrings[vid[i]]);
            if (len < 0) return;
            used += len;
        }
    }
    else
    {
        len = snprintf(buf + used, capacity - used, "VID = %s; ", dynrngInputStrings[vid[0]]);
        if (len < 0) return;
        used += len;
    }
    len = snprintf(buf + used, capacity - used, "OUT = %s", dynrngOutputStrings[out]);
    if (len < 0) return;
    used += len;
}

static void print_summary_entry(AppContext * pCtx, DynamicRange gfx, DynamicRange vid[MAX_VIDS], DynamicRange out, const char * notes)
{
    fprintf(pCtx->summary, "<tr>\n");
    fprintf(pCtx->summary, "\t<td>%lu</td>\n", pCtx->testNum);
    fprintf(pCtx->summary, "\t<td>%s</td>\n", testL1TypeStrings[pCtx->l1Type]);
    fprintf(pCtx->summary, "\t<td>%s</td>\n", testL2TypeStrings[pCtx->l2Type]);
    switch (pCtx->l2Type)
    {
        case TestL2Type_eBasic:
            fprintf(pCtx->summary, "\t<td>%s</td>\n", testBasicL3TypeStrings[pCtx->l3Type.basic]);
            break;
        case TestL2Type_eTransition:
            fprintf(pCtx->summary, "\t<td>%s</td>\n", testTransitionL3TypeStrings[pCtx->l3Type.transition]);
            break;
        case TestL2Type_eHotplug:
            fprintf(pCtx->summary, "\t<td>%s</td>\n", testHotplugL3TypeStrings[pCtx->l3Type.hotplug] ? testHotplugL3TypeStrings[pCtx->l3Type.hotplug] : "N/A");
            break;
        case TestL2Type_eGraphics:
            fprintf(pCtx->summary, "\t<td>%s</td>\n", testGraphicsL3TypeStrings[pCtx->l3Type.graphics] ? testGraphicsL3TypeStrings[pCtx->l3Type.graphics] : "N/A");
            break;
        default:
            break;
    }
    fprintf(pCtx->summary, "\t<td>%s</td>\n", dynrngInputStrings[gfx]);
    fprintf(pCtx->summary, "\t<td>%s</td>\n", dynrngInputStrings[vid[0]]);
    fprintf(pCtx->summary, "\t<td>%s</td>\n", dynrngInputStrings[vid[1]]);
    fprintf(pCtx->summary, "\t<td>%s</td>\n", dynrngInputStrings[vid[2]]);
    fprintf(pCtx->summary, "\t<td>%s</td>\n", dynrngInputStrings[vid[3]]);
    fprintf(pCtx->summary, "\t<td>%s</td>\n", dynrngInputStrings[out]);
    fprintf(pCtx->summary, "\t<td class=\"notes\">%s</td>\n", notes);
    fprintf(pCtx->summary, "</tr>\n");
}

static const char * const streamStrings[DynamicRange_eVidMax + 1] =
{
    NULL,
    "test_pattern_1920x1080_30hz_10b_BT1886_BT709.ts",
    "test_pattern_1920x1080_30hz_10b_PQ_BT2020.ts",
    "test_pattern_1920x1080_30hz_10b_HLG_BT2020.ts",
    NULL
};

static const char * const bgStrings[2][DynamicRange_eGfxMax + 1] =
{
    /* non-mp */
    {
        NULL,
        "epg1080p.png",
        NULL
    },
    /* mp */
    {
        NULL,
        "desktop_background.brcmLTD.png",
        NULL
    }
};

static const char * const imageStrings[2][DynamicRange_eGfxMax + 1] =
{
    /* non-mp */
    {
        NULL,
        "LifeOfPi.png",
        NULL
    },
    /* mp */
    {
        NULL,
        "Avatar.bmp",
        NULL
    }
};

static bool multipip(DynamicRange vid[MAX_VIDS])
{
    unsigned count = 0;
    unsigned i;

    /* if any of vids 1-3 are enabled, it is multipip, even if zero is disabled */
    for (i = 1; i < MAX_VIDS; i++)
    {
        if (vid[i] != DynamicRange_eNone)
        {
            count++;
        }
    }

    return count > 0;
}

static int find_first_active_vid(DynamicRange vid[MAX_VIDS])
{
    int index = -1;
    unsigned i;

    for (i = 0; i < MAX_VIDS; i++)
    {
        if (vid[i] != DynamicRange_eNone)
        {
            index = i;
            break;
        }
    }

    return index;
}

static bool equivalent(DynamicRange in, DynamicRange out)
{
    return
        (in == out)
        ||
        (in == DynamicRange_eSdr && out == DynamicRange_eLegacy)
        ||
        (in == DynamicRange_eSdr && out == DynamicRange_eSdr);
}

static bool compatible(DynamicRange out, bool caps[])
{
    return
        caps[out]
        ||
        (out == DynamicRange_eSdr && caps[DynamicRange_eLegacy])
        ||
        (out == DynamicRange_eHlg && caps[DynamicRange_eSdr])
        ||
        (out == DynamicRange_eHlg && caps[DynamicRange_eLegacy]);
}

static bool legacy(bool caps[])
{
    unsigned count = 0;
    DynamicRange i;

    for (i = 0; i < DynamicRange_eVidMax; i++)
    {
        if (caps[i])
        {
            count++;
        }
    }

    return count == 1 && caps[DynamicRange_eLegacy];
}

#define ECHO(X) fprintf(f, "echo \"" X "\"\n");
#define WAITUSER() fprintf(f, "waituser\n");

static int generate_basic_test(AppContext * pCtx, DynamicRange gfx, DynamicRange vid[MAX_VIDS], DynamicRange out, bool rxcaps[DynamicRange_eVidMax])
{
    char fname[MAX_FILE_NAME_LEN];
    char caps[MAX_CAPS_LEN];
    char description[MAX_DESCRIPTION_LEN];
    FILE * f;
    unsigned i;
    bool mp = multipip(vid);
    int fav = find_first_active_vid(vid);

    /* TODO: no content at all, may want to test for bg color later */
    if (gfx == DynamicRange_eNone && fav == -1) return 0;

    snprintf(fname, MAX_FILE_NAME_LEN, "%lu.txt", pCtx->testNum);
    f = fopen(fname, "w");
    if (!f) return -1;
    fprintf(f, "# DynRng Generated Test #%lu\n", pCtx->testNum);
    print_rx_caps(caps, MAX_CAPS_LEN, rxcaps);
    fprintf(f, "# RX CAPS:%s\n", caps);
    print_test_vars(description, MAX_DESCRIPTION_LEN, gfx, vid, mp, out);
    fprintf(f, "# TEST VARS: %s\n", description);
    print_summary_entry(pCtx, gfx, vid, out, "");
    fprintf(f, "reset\n");
    fprintf(f, "echo \"Please attach a TV with the following capabilities: %s\n", dynrngOutputStrings[out]);
    WAITUSER();

    if (!mp && ((fav != -1 && equivalent(vid[fav], out)) || (fav == -1 && equivalent(gfx, out))))
    {
        /* PASSTHROUGH */
        ECHO("This is a PASSTHROUGH test case.");
        ECHO();
        if (compatible(out, rxcaps))
        {
            ECHO("After you hit enter, please look at the server console for the most recent messages printing the DRM infoframe contents while running the test.");
            ECHO("You are expecting to see the following:");
            ECHO();
            if (legacy(rxcaps))
            {
                ECHO("Rx Does not support HDR; no DRM packet sent");
            }
            else
            {
                fprintf(f, "echo \"eotf: Invalid -> %s\"\n", dynrngInputStrings[vid[fav]]);
                /* TODO: metadata */
            }
            ECHO();
        }
        else
        {
            /* TV doesn't support output mode */
            ECHO();
            ECHO("The TV does not support this output mode.");
            ECHO("After you hit enter, please look at the server console to see if there are any warnings printed while running the test");
            ECHO();
        }
    }
    else
    {
        if (compatible(out, rxcaps))
        {
            ECHO("This is a CONVERSION test case.");
            ECHO();
            ECHO("The conversion case is:");
            ECHO();
            if (mp)
            {
                ECHO("MULTIPIP");
                for (i = 0; i < MAX_VIDS; i++)
                {
                    if (vid[i] != DynamicRange_eNone)
                    {
                        fprintf(f, "echo \"VID%u %s to %s\"\n", i, dynrngInputStrings[vid[i]], dynrngOutputStrings[out]);
                    }
                }
            }
            else
            {
                if (fav != -1)
                {
                    fprintf(f, "echo \"VID%u %s to %s\"\n", fav, dynrngInputStrings[vid[fav]], dynrngOutputStrings[out]);
                }
            }
            if (gfx != DynamicRange_eNone)
            {
                if (fav == -1)
                {
                    ECHO("GFX ONLY");
                }
                fprintf(f, "echo \"GFX %s to %s\"\n", dynrngInputStrings[gfx], dynrngOutputStrings[out]);
            }
            ECHO();
            if (!mp && fav != -1 && vid[fav] == DynamicRange_eHdr10 && (out == DynamicRange_eSdr || out == DynamicRange_eLegacy))
            {
                ECHO("Please turn the brightness on the TV all the way down to zero.");
                ECHO();
            }
            else
            {
                ECHO("Please ensure the brightness on the TV is back to normal.");
                ECHO();
            }
            ECHO("After you hit enter, please look at the TV screen and compare to the reference image(s) for this conversion case.");
            ECHO();
        }
        else
        {
            /* TV doesn't support output mode */
            ECHO();
            ECHO("The TV does not support this output mode.");
            ECHO("After you hit enter, please look at the server console to see if there are any warnings printed while running the test");
            ECHO();
        }
    }
    WAITUSER();

    /* configuration */
    fprintf(f, "plm.vid = 1\n");
    fprintf(f, "plm.gfx = 1\n");
    fprintf(f, "dynrng = %s\n", dynrngSettingStrings[out]);
    fprintf(f, "layout = 0\n");
    for (i = 0; i < MAX_VIDS; i++)
    {
        if (streamStrings[vid[i]])
        {
            fprintf(f, "stream[%u] = %s\n", i, streamStrings[vid[i]]);
        }
        else
        {
            fprintf(f, "stream[%u] =\n", i);
        }
    }
    if (fav == -1)
    {
        fprintf(f, "pig = on\n");
    }
    else
    {
        fprintf(f, "pig = off\n");
    }
    if (bgStrings[mp][gfx])
    {
        fprintf(f, "bg = %s\n", bgStrings[mp][gfx]);
    }
    else
    {
        fprintf(f, "bg =\n");
    }
    if (imageStrings[mp][gfx])
    {
        fprintf(f, "image = %s\n", imageStrings[mp][gfx]);
    }
    else
    {
        fprintf(f, "image =\n");
    }
    fprintf(f, "commit\n");

    /* ask how it went */
    fprintf(f, "results\n");

    fclose(f);

    pCtx->testNum++;
    pCtx->totalTestCount++;

    return 0;
}

#if 0
int generate_tests(AppContext * pCtx, bool rxcaps[DynamicRange_eVidMax])
{
    int rc = 0;
    DynamicRange gfx;
    DynamicRange vid[MAX_VIDS];
    DynamicRange out;

    for (out = DynamicRange_eLegacy; out < DynamicRange_eVidMax; out++)
    {
        for (gfx = DynamicRange_eNone; gfx < DynamicRange_eGfxMax; gfx++)
        {
            for (vid[3] = DynamicRange_eNone; vid[3] < DynamicRange_eVidMax; vid[3]++)
            {
                for (vid[2] = DynamicRange_eNone; vid[2] < DynamicRange_eVidMax; vid[2]++)
                {
                    for (vid[1] = DynamicRange_eNone; vid[1] < DynamicRange_eVidMax; vid[1]++)
                    {
                        for (vid[0] = DynamicRange_eNone; vid[0] < DynamicRange_eVidMax; vid[0]++)
                        {
                            rc = generate_basic_test(pCtx, gfx, vid, out, rxcaps);
                            if (rc) goto end;
                        }
                    }
                }
            }
        }
    }
end:
    return rc;
}
#endif

static int generate_basic_passthrough_tests(AppContext * pCtx, bool rxcaps[DynamicRange_eVidMax])
{
    int rc = 0;
    DynamicRange gfx;
    DynamicRange vid[MAX_VIDS];
    DynamicRange out;
    unsigned long testBase;

    testBase = pCtx->testNum;
    pCtx->l3Type.basic = TestBasicL3Type_eSingle;

    /*
     * PASSTHROUGH
     * the output will always match the input
     * there is no point to testing passthrough with no video (eliminates eNone)
     * presence of graphics does not affect video test cases, so we turn it off
     * testing the difference between Legacy and SDR outputs comes under protocol testing, so we don't do it here
     * multipip makes no sense on passthrough chips, unless they are all the same
     */
    gfx = DynamicRange_eNone;
    /* single decode passthrough */
    vid[1] = DynamicRange_eNone;
    vid[2] = DynamicRange_eNone;
    vid[3] = DynamicRange_eNone;
    for (vid[0] = DynamicRange_eSdr; vid[0] < DynamicRange_eVidMax; vid[0]++)
    {
        out = vid[0];
#if NO_HLG_TVS
        if (vid[0] == DynamicRange_eHlg) out = DynamicRange_eSdr;
#endif
        rc = generate_basic_test(pCtx, gfx, vid, out, rxcaps);
        if (rc) goto end;
    }

    testBase += TEST_L3GROUP_SIZE;
    pCtx->testNum = testBase;
    pCtx->l3Type.basic = TestBasicL3Type_eMainPip;
    /* TODO: regular pip passthrough (to test overlapping video windows)? */
    if (rc) goto end;

    testBase += TEST_L3GROUP_SIZE;
    pCtx->testNum = testBase;
    pCtx->l3Type.basic = TestBasicL3Type_eMultipip;
    /* multipip passthrough */
    for (vid[0] = DynamicRange_eSdr; vid[0] < DynamicRange_eVidMax; vid[0]++)
    {
        out = vid[0];
#if NO_HLG_TVS
        if (vid[0] == DynamicRange_eHlg) out = DynamicRange_eSdr;
#endif
        vid[1] = vid[0];
        vid[2] = vid[0];
        vid[3] = vid[0];
        rc = generate_basic_test(pCtx, gfx, vid, out, rxcaps);
        if (rc) goto end;
    }

end:
    return rc;
}

static int generate_passthrough_transition_tests(AppContext * pCtx, bool rxcaps[DynamicRange_eVidMax])
{
    int rc = 0;
    unsigned long testBase;

    testBase = pCtx->testNum;
    pCtx->l3Type.transition = TestTransitionL3Type_eInputOutput;
    /* TODO: generate conversion i/o transition tests */
    if (rc) goto end;

end:
    return rc;
}

static int generate_passthrough_hotplug_tests(AppContext * pCtx, bool rxcaps[DynamicRange_eVidMax])
{
    int rc = 0;
    unsigned long testBase;

    testBase = pCtx->testNum;
    pCtx->l3Type.hotplug = 0;
    /* TODO: generate passthrough hotplug transition tests */
    if (rc) goto end;

end:
    return rc;
}

static int generate_gfx_passthrough_tests(AppContext * pCtx, bool rxcaps[DynamicRange_eVidMax])
{
    int rc = 0;
    DynamicRange gfx;
    DynamicRange vid;
    DynamicRange out;

    /* video passthrough with SDR graphics */
    gfx = DynamicRange_eSdr;
    for (vid = DynamicRange_eSdr; vid < DynamicRange_eVidMax; vid++)
    {
        out = vid;
#if NO_HLG_TVS
        if (vid == DynamicRange_eHlg) out = DynamicRange_eSdr;
#endif
        /*
         * TODO:
         * need to have several test cases each focusing on a different graphic
         * color to enhance using the sdrToHdr controls
         */
/*        rc = generate_basic_test(pCtx, gfx, vid, out, rxcaps);
        if (rc) goto end;*/
    }

end:
    return rc;
}

static int generate_passthrough_tests(AppContext * pCtx, bool rxcaps[DynamicRange_eVidMax])
{
    int rc = 0;
    unsigned long testBase;

    testBase = pCtx->testNum;
    pCtx->l2Type = TestL2Type_eBasic;
    rc = generate_basic_passthrough_tests(pCtx, rxcaps);
    if (rc) goto end;

    testBase += TEST_L2GROUP_SIZE;
    pCtx->testNum = testBase;
    pCtx->l2Type = TestL2Type_eTransition;
    rc = generate_passthrough_transition_tests(pCtx, rxcaps);
    if (rc) goto end;

    testBase += TEST_L2GROUP_SIZE;
    pCtx->testNum = testBase;
    pCtx->l2Type = TestL2Type_eHotplug;
    rc = generate_passthrough_hotplug_tests(pCtx, rxcaps);
    if (rc) goto end;

    testBase += TEST_L2GROUP_SIZE;
    pCtx->testNum = testBase;
    pCtx->l2Type = TestL2Type_eGraphics;
    rc = generate_gfx_passthrough_tests(pCtx, rxcaps);
    if (rc) goto end;

end:
    return rc;
}

static int generate_basic_conversion_tests(AppContext * pCtx, bool rxcaps[DynamicRange_eVidMax])
{
    int rc = 0;
    DynamicRange gfx;
    DynamicRange vid[MAX_VIDS];
    DynamicRange out;
    unsigned long testBase;

    testBase = pCtx->testNum;
    pCtx->l3Type.basic = TestBasicL3Type_eSingle;

    /*
     * CONVERSION
     * the output will always match the best the TV can support
     * there is no point to testing conversion with no video (eliminates eNone)
     * presence of graphics does not affect video test cases, so we turn it off
     * multipip is much more complicated on conversion chips
     */
    gfx = DynamicRange_eNone;
    /* single decode conversion */
    vid[1] = DynamicRange_eNone;
    vid[2] = DynamicRange_eNone;
    vid[3] = DynamicRange_eNone;
    for (out = DynamicRange_eLegacy; out < DynamicRange_eVidMax; out++)
    {
#if NO_HLG_TVS
        if (out == DynamicRange_eHlg)
        {
            pCtx->testNum++;
            pCtx->totalTestCount++;
            continue;
        }
#endif
        for (vid[0] = DynamicRange_eSdr; vid[0] < DynamicRange_eVidMax; vid[0]++)
        {
            if (out != vid[0])
            {
                rc = generate_basic_test(pCtx, gfx, vid, out, rxcaps);
                if (rc) goto end;
            }
        }
    }

    testBase += TEST_L3GROUP_SIZE;
    pCtx->testNum = testBase;
    pCtx->l3Type.basic = TestBasicL3Type_eMainPip;
    /* TODO: regular pip conversion (to test overlapping video windows)? */
    if (rc) goto end;

    testBase += TEST_L3GROUP_SIZE;
    pCtx->testNum = testBase;
    pCtx->l3Type.basic = TestBasicL3Type_eMultipip;
    /* multipip conversion: it suffices to test a single combination with all possible types as inputs */
    for (out = DynamicRange_eLegacy; out < DynamicRange_eVidMax; out++)
    {
#if NO_HLG_TVS
        if (out == DynamicRange_eHlg)
        {
            pCtx->testNum++;
            pCtx->totalTestCount++;
            continue;
        }
#endif
        vid[0] = DynamicRange_eSdr;
        vid[1] = DynamicRange_eHdr10;
        vid[2] = DynamicRange_eHlg;
        vid[3] = DynamicRange_eHdr10; /* this usually tests the regular pip window, but with no overlap */
        rc = generate_basic_test(pCtx, gfx, vid, out, rxcaps);
        if (rc) goto end;
    }
end:
    return rc;
}

static int generate_conversion_transition_tests(AppContext * pCtx, bool rxcaps[DynamicRange_eVidMax])
{
    int rc = 0;
    unsigned long testBase;

    testBase = pCtx->testNum;
    pCtx->l3Type.transition = TestTransitionL3Type_eInput;
    /* TODO: generate conversion input transition tests */
    if (rc) goto end;

    testBase += TEST_L3GROUP_SIZE;
    pCtx->testNum = testBase;
    pCtx->l3Type.transition = TestTransitionL3Type_eOutput;
    /* TODO: generate conversion output transition tests */
    if (rc) goto end;

end:
    return rc;
}

static int generate_conversion_hotplug_tests(AppContext * pCtx, bool rxcaps[DynamicRange_eVidMax])
{
    int rc = 0;
    unsigned long testBase;

    testBase = pCtx->testNum;
    pCtx->l3Type.hotplug = 0;
    /* TODO: generate conversion hotplug transition tests */
    if (rc) goto end;

end:
    return rc;
}

static int generate_gfx_conversion_tests(AppContext * pCtx, bool rxcaps[DynamicRange_eVidMax])
{
    int rc = 0;
    DynamicRange gfx;
    DynamicRange vid[MAX_VIDS];
    DynamicRange out;

    /* no video is required to test gfx conversion as it depends only on the output */
    memset(vid, 0, sizeof(vid));

    pCtx->l3Type.graphics = 0;

    /* sdr gfx only for now */
    gfx = DynamicRange_eSdr;
    for (out = DynamicRange_eSdr; out < DynamicRange_eVidMax; out++)
    {
#if NO_HLG_TVS
        if (out == DynamicRange_eHlg)
        {
            pCtx->testNum++;
            pCtx->totalTestCount++;
            continue;
        }
#endif
        rc = generate_basic_test(pCtx, gfx, vid, out, rxcaps);
        if (rc) goto end;
    }

end:
    return rc;
}

static int generate_conversion_tests(AppContext * pCtx, bool rxcaps[DynamicRange_eVidMax])
{
    int rc = 0;
    unsigned long testBase;

    testBase = pCtx->testNum;
    pCtx->l2Type = TestL2Type_eBasic;
    rc = generate_basic_conversion_tests(pCtx, rxcaps);
    if (rc) goto end;

    testBase += TEST_L2GROUP_SIZE;
    pCtx->testNum = testBase;
    pCtx->l2Type = TestL2Type_eTransition;
    rc = generate_conversion_transition_tests(pCtx, rxcaps);
    if (rc) goto end;

    testBase += TEST_L2GROUP_SIZE;
    pCtx->testNum = testBase;
    pCtx->l2Type = TestL2Type_eHotplug;
    rc = generate_conversion_hotplug_tests(pCtx, rxcaps);
    if (rc) goto end;

    testBase += TEST_L2GROUP_SIZE;
    pCtx->testNum = testBase;
    pCtx->l2Type = TestL2Type_eGraphics;
    rc = generate_gfx_conversion_tests(pCtx, rxcaps);
    if (rc) goto end;

end:
    return rc;
}

static int generate_tests(AppContext * pCtx, bool rxcaps[DynamicRange_eVidMax])
{
    int rc = 0;
    unsigned long testBase;

    pCtx->totalTestCount = 0;

    testBase = 1000;
    pCtx->testNum = testBase;
    pCtx->l1Type = TestL1Type_ePassthrough;
    rc = generate_passthrough_tests(pCtx, rxcaps);
    if (rc) goto end;

    testBase += TEST_L1GROUP_SIZE;
    pCtx->testNum = testBase;
    pCtx->l1Type = TestL1Type_eConversion;
    rc = generate_conversion_tests(pCtx, rxcaps);
    if (rc) goto end;

    /* TODO: generate negative tests for TVs without support for output mode */

end:
    return rc;
}

int main(int argc, char * argv[])
{
    int rc = 0;
    unsigned i;
    AppContext ctx;
    bool rxcaps[DynamicRange_eVidMax];

    memset(&ctx, 0, sizeof(ctx));
    ctx.summary = fopen("dynrng-test-summary.html", "w");
    if (!ctx.summary) { fprintf(stderr, "Error opening test summary document for write\n"); rc = -1; goto end; }
    print_summary_header(&ctx);

    /*
     * current theory is we do not yet wish to test cases where the TV doesn't
     * support the output mode so we assume it will always support it
     */
    memset(rxcaps, 0, sizeof(rxcaps));
#if 1
    for (i = 0; i < DynamicRange_eVidMax; i++)
    {
        rxcaps[i] = true;
    }
    generate_tests(&ctx, rxcaps);
    printf("Generated %lu test cases\n", ctx.totalTestCount);
#else
    rxcaps[DynamicRange_eLegacy] = true;
    rc = generate_tests(&ctx, rxcaps); /* legacy TV */
    if (rc) { fprintf(stderr, "Error generating tests: %d\n", rc); goto end; }
    rxcaps[DynamicRange_eSdr] = true;
    rc = generate_tests(&ctx, rxcaps); /* TV with HDRDB that claims only SDR support */
    if (rc) { fprintf(stderr, "Error generating tests: %d\n", rc); goto end; }
    rxcaps[DynamicRange_eHdr10] = true;
    rc = generate_tests(&ctx, rxcaps); /* TV with HDRDB that claims SDR and HDR10 but not HLG */
    if (rc) { fprintf(stderr, "Error generating tests: %d\n", rc); goto end; }
    rxcaps[DynamicRange_eHlg] = true;
    rc = generate_tests(&ctx, rxcaps); /* TV with HDRDB that claims SDR, HDR10, and HLG */
    if (rc) { fprintf(stderr, "Error generating tests: %d\n", rc); goto end; }
    rxcaps[DynamicRange_eHdr10] = false;
    rc = generate_tests(&ctx, rxcaps); /* TV with HDRDB that claims SDR and HLG, but not HDR10 */
    if (rc) { fprintf(stderr, "Error generating tests: %d\n", rc); goto end; }
#endif
    print_summary_footer(&ctx);
end:
    if (ctx.summary) fclose(ctx.summary);
    return rc;
}

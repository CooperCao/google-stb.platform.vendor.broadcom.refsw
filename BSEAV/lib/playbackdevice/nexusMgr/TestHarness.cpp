/******************************************************************************
 *    (c)2010-2011 Broadcom Corporation
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
 * Module Description:
 * 
 * Revision History:
 * 
 * $brcm_Log: $
 * 
 *****************************************************************************/
#include "TestHarness.h"

#include <vector>
#include <cstdio>
#include <cstring>
#include <cstdlib>

typedef struct {
    U32 frame;
    U32 key;
    U32 direction;
} TestHarnessKeyNode;

typedef struct {
    U32 frame;
    U32 checksum;
} TestHarnessChecksumNode;

struct _nowarning_ {
    U32                                     mFrame;
    TestHarnessMode                         mMode;
    std::vector<TestHarnessKeyNode>         key_vector;
    std::vector<TestHarnessChecksumNode>    crc_vector;
    bool                                    bPass;
    char                                    mResultFilename[512];
    char                                    mScriptFilename[512];
} _test_harness_data;

static void _Import(void)
{
    FILE * input = fopen(_test_harness_data.mScriptFilename, "rt");

    if (input) {
        char    a[16];
        U32     b;
        U32     c;
        U32     d;

        while (fscanf (input, "%s %d,0x%08x,%d", a, &b, &c, &d) == 4) {
            if (!strcmp(a,"key")) {
                TestHarnessKeyNode temp = {b, c, d};
                _test_harness_data.key_vector.push_back(temp);
            } else if (!strcmp(a,"crc")) {
                if (_test_harness_data.mMode == TestHarnessModePlayback) {
                    TestHarnessChecksumNode temp = {b, c};
                    _test_harness_data.crc_vector.push_back(temp);
                }
            }
        }
        fclose(input);
    } else {
        fprintf(stderr,"Test harness: Unable to import from '%s'.\n",_test_harness_data.mScriptFilename);
    }
}

static void _Export(void)
{
    FILE * output = fopen(_test_harness_data.mScriptFilename,"wt");

    if (output) {
        for (U32 i = 0; i < _test_harness_data.key_vector.size(); i++) {
            TestHarnessKeyNode temp = _test_harness_data.key_vector.at(i);
            fprintf(output,"key %d,0x%08x,%d\n", temp.frame, temp.key, temp.direction );
        }

        if (_test_harness_data.mMode == TestHarnessModeOutputCapture) {
            for (U32 i = 0; i < _test_harness_data.crc_vector.size(); i++) {
                TestHarnessChecksumNode temp = _test_harness_data.crc_vector.at(i);
                fprintf(output,"crc %d,0x%08x,0\n", temp.frame, temp.checksum);
            }
        }
        fclose(output);
    } else {
        fprintf(stderr,"Test harness: Unable to export to '%s'.\n",_test_harness_data.mScriptFilename);
    }
}

extern "C" void TestHarnessFail(U32 frame)
{
    FILE * output = fopen(_test_harness_data.mResultFilename,"at");
    if (output) {
        fprintf(output,"CRC mismatch on frame %d.\n", frame);
        fclose(output);
    }
    _test_harness_data.bPass = false;
}


extern "C" void TestHarnessPass(void)
{
    _test_harness_data.bPass = true;
}

extern "C" void TestHarnessAddKey(U32 key, U32 direction)
{
    if (_test_harness_data.mMode == TestHarnessModeInputCapture) {
        TestHarnessKeyNode temp = {_test_harness_data.mFrame, key, direction};
        _test_harness_data.key_vector.push_back(temp);
    }
}

extern "C" void TestHarnessAddFrameHash(U32 crc)
{
    if ((_test_harness_data.mMode == TestHarnessModeOutputCapture) || (_test_harness_data.mMode == TestHarnessModeInputCapture)) {
        _test_harness_data.mFrame++;
        TestHarnessChecksumNode temp = {_test_harness_data.mFrame, crc};
        _test_harness_data.crc_vector.push_back(temp);
    }
}

extern "C" U32 TestHarnessGetKey(U32 frame, U32 * key, U32 * direction)
{
    static U32 last_found_index = 0;

    if (!key || !direction) {
        return 0;
    }

    for (U32 i = last_found_index; i < _test_harness_data.key_vector.size(); i++) {
        TestHarnessKeyNode temp = _test_harness_data.key_vector[i];
        if (temp.frame == frame) {
            *key = temp.key;
            *direction = temp.direction;
            last_found_index = i+1;
            return 1;
        }
    }
    return 0;
}

extern "C" U32 TestHarnessGetFrameHash(U32 frame)
{
    for (U32 i = 0; i < _test_harness_data.crc_vector.size(); i++) {
        TestHarnessChecksumNode temp = _test_harness_data.crc_vector[i];
        if (temp.frame == frame) {
            return temp.checksum;
        }
    }
    return 0xDEADBEEF;
}

extern "C" TestHarnessMode TestHarnessGetMode(void)
{
    return _test_harness_data.mMode;
}

extern "C" void TestHarnessStart(void)
{
    char * env_var;

    _test_harness_data.mFrame = 0;
    _test_harness_data.mMode = TestHarnessModeNone;
    _test_harness_data.bPass = false;

    bool bResult = false;
    bool bScript = false;
    bool bMode   = false;

    if ((env_var = getenv("TEST_HARNESS_RESULT_FILE")) != NULL) {
        strcpy(_test_harness_data.mResultFilename, env_var);
        bResult = true;
    }

    if ((env_var = getenv("TEST_HARNESS_SCRIPT_FILE")) != NULL) {
        strcpy(_test_harness_data.mScriptFilename, env_var);
        bScript = true;
    }

    if ((env_var = getenv("TEST_HARNESS_MODE")) != NULL) {

        if (!strcmp(env_var, "input_capture")) {
            _test_harness_data.mMode = TestHarnessModeInputCapture;
            bMode = true;
        }

        if (!strcmp(env_var, "output_capture")) {
            _test_harness_data.mMode = TestHarnessModeOutputCapture;
            _Import();
            bMode = true;
        }

        if (!strcmp(env_var, "playback")) {
            _test_harness_data.mMode = TestHarnessModePlayback;
            _Import();
            FILE * output = fopen(_test_harness_data.mResultFilename,"wt");
            if (output) {
                fprintf(output,"Started\n");
                fclose(output);
            }
            bMode = true;
        }
    }

    if ((bResult || bScript || bMode) && !(bResult && bScript && bMode)) {
        /* One of the variables is set but they aren't all set correctly. */
        fprintf(stderr,"Test harness: Not all environment variables set.  Harness disabled.\n");
        _test_harness_data.mMode = TestHarnessModeNone;
    }

    if (bResult && bScript && bMode) {
        fprintf(stderr,"Test harness: Harness enabled.  Mode = %d\n", _test_harness_data.mMode);
    }
}

extern "C" void TestHarnessTerminate(void)
{
    switch (_test_harness_data.mMode) {
    case TestHarnessModeInputCapture:
        _Export();
        break;
    case TestHarnessModeOutputCapture:
        _Export();
        break;
    case TestHarnessModePlayback:
        {
            FILE * output = fopen(_test_harness_data.mResultFilename,"at");
            if (_test_harness_data.bPass) {
                fprintf(output,"Pass\n");
            } else {
                fprintf(output,"Fail\n");
            }
            fclose(output);
        }
        break;
    default:
        break;
    }
}

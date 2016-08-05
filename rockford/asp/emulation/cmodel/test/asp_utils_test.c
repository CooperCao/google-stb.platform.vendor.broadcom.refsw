/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include <sys/types.h>
#include <stdio.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include "asp_utils.h"

#define ASP_NUM_CTXS   32
static ASP_CtxEntry g_aspCtxList[ASP_NUM_CTXS];

int main()
{
    int     testCtx0, testCtx1, testCtx2;

    memset(&g_aspCtxList, 0, sizeof(g_aspCtxList));

    /* Find a free ctx. */
    {
        testCtx0 = ASP_Utils_AllocFreeCtx(g_aspCtxList, ASP_NUM_CTXS, NULL);
        assert(testCtx0 == 0);
        fprintf(stdout, "testCtx0=%d\n", testCtx0);
        ASP_Utils_FreeCtx(g_aspCtxList, testCtx0, ASP_NUM_CTXS);
    }

    /* Find two free ctx. */
    {
        testCtx0 = ASP_Utils_AllocFreeCtx(g_aspCtxList, ASP_NUM_CTXS, NULL);
        assert(testCtx0 == 0);
        testCtx1 = ASP_Utils_AllocFreeCtx(g_aspCtxList, ASP_NUM_CTXS, NULL);
        assert(testCtx1 == 1);
        fprintf(stdout, "testCtx0=%d, testCtx1=%d\n", testCtx0, testCtx1);
        ASP_Utils_FreeCtx(g_aspCtxList, testCtx0, ASP_NUM_CTXS);
        ASP_Utils_FreeCtx(g_aspCtxList, testCtx1, ASP_NUM_CTXS);
    }

    /* Check if same ctx# if getting re-used. */
    {
        testCtx0 = ASP_Utils_AllocFreeCtx(g_aspCtxList, ASP_NUM_CTXS, NULL);
        assert(testCtx0 == 0);
        fprintf(stdout, "testCtx0=%d\n", testCtx0);
        ASP_Utils_FreeCtx(g_aspCtxList, testCtx0, ASP_NUM_CTXS);

        testCtx0 = ASP_Utils_AllocFreeCtx(g_aspCtxList, ASP_NUM_CTXS, NULL);
        assert(testCtx0 == 0);
        fprintf(stdout, "testCtx0=%d\n", testCtx0);
        ASP_Utils_FreeCtx(g_aspCtxList, testCtx0, ASP_NUM_CTXS);
    }

    /* Find 3 free ctx. */
    {
        testCtx0 = ASP_Utils_AllocFreeCtx(g_aspCtxList, ASP_NUM_CTXS, NULL);
        assert(testCtx0 == 0);

        testCtx1 = ASP_Utils_AllocFreeCtx(g_aspCtxList, ASP_NUM_CTXS, NULL);
        assert(testCtx1 == 1);

        testCtx2 = ASP_Utils_AllocFreeCtx(g_aspCtxList, ASP_NUM_CTXS, NULL);
        assert(testCtx2 == 2);

        fprintf(stdout, "testCtx0=%d, testCtx1=%d, testCtx2=%d\n", testCtx0, testCtx1, testCtx2);
        ASP_Utils_FreeCtx(g_aspCtxList, testCtx1, ASP_NUM_CTXS);

        testCtx1 = ASP_Utils_AllocFreeCtx(g_aspCtxList, ASP_NUM_CTXS, NULL);
        assert(testCtx1 == 1);

        ASP_Utils_FreeCtx(g_aspCtxList, testCtx0, ASP_NUM_CTXS);
        ASP_Utils_FreeCtx(g_aspCtxList, testCtx1, ASP_NUM_CTXS);
        ASP_Utils_FreeCtx(g_aspCtxList, testCtx2, ASP_NUM_CTXS);
    }

    /* Allocate ctx & look up a context using a ctxNumber. */
    {
        int i;
        int *pCtx = &i;

        testCtx0 = ASP_Utils_AllocFreeCtx(g_aspCtxList, ASP_NUM_CTXS, pCtx);
        assert(testCtx0 == 0);

        testCtx1 = ASP_Utils_AllocFreeCtx(g_aspCtxList, ASP_NUM_CTXS, NULL);
        assert(testCtx1 == 1);

        testCtx2 = ASP_Utils_AllocFreeCtx(g_aspCtxList, ASP_NUM_CTXS, NULL);
        assert(testCtx2 == 2);

        fprintf(stdout, "testCtx0=%d, testCtx1=%d, testCtx2=%d\n", testCtx0, testCtx1, testCtx2);
        pCtx = ASP_Utils_FindCtxUsingCtxNumber(g_aspCtxList, testCtx0, ASP_NUM_CTXS);
        assert(pCtx == &i);

        ASP_Utils_FreeCtx(g_aspCtxList, testCtx0, ASP_NUM_CTXS);
        ASP_Utils_FreeCtx(g_aspCtxList, testCtx1, ASP_NUM_CTXS);
        ASP_Utils_FreeCtx(g_aspCtxList, testCtx2, ASP_NUM_CTXS);
    }


    return 0;
}

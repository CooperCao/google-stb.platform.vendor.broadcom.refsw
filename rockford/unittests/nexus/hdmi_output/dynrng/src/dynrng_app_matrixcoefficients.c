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
******************************************************************************/

#include "nexus_platform.h"
#include "bstd.h"
#include "bkni.h"
#include "blst_queue.h"
#include "dynrng_app.h"
#include "dynrng_app_priv.h"
#include "dynrng_shell.h"
#include "dynrng_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const UTILS_StringIntMapEntry matrixCoefficientsAliases[] =
{
    { "input", APP_CMD_eInput },
    { "auto", APP_CMD_eAuto },
    { "709", NEXUS_MatrixCoefficients_eItu_R_BT_709 },
    { "170m", NEXUS_MatrixCoefficients_eSmpte_170M },
    { "470_2_bg", NEXUS_MatrixCoefficients_eItu_R_BT_470_2_BG },
    { "2020ncl",  NEXUS_MatrixCoefficients_eItu_R_BT_2020_NCL },
    { "2020cl", NEXUS_MatrixCoefficients_eItu_R_BT_2020_CL },
    { NULL, NEXUS_MatrixCoefficients_eMax },
};

const char * APP_GetMatrixCoefficientsName(NEXUS_MatrixCoefficients matrixCoefficients)
{
    return UTILS_GetTableName(matrixCoefficientsAliases, matrixCoefficients);
}

NEXUS_MatrixCoefficients APP_ParseMatrixCoefficients(const char * matrixCoefficientsStr)
{
    return (NEXUS_MatrixCoefficients)UTILS_ParseTableAlias(matrixCoefficientsAliases, matrixCoefficientsStr);
}

void APP_PrintMatrixCoefficients(APP_AppHandle app)
{
    fprintf(stdout, "# MatrixCoefficient\n");
    fprintf(stdout, "input = %s (%d)\n",
            (!app->lastStreamInfoValid) ? "unknown" : APP_GetMatrixCoefficientsName(app->lastStreamInfo.matrixCoefficients), app->lastStreamInfo.matrixCoefficients);
    fprintf(stdout, "output = %s (%d)\n", APP_GetMatrixCoefficientsName(app->args.matrixCoefficients), app->args.matrixCoefficients);
}

int APP_DoMatrixCoefficients(void * context, char * args)
{
    APP_AppHandle app = (APP_AppHandle) context;
    NEXUS_Error rc = NEXUS_SUCCESS;
    const char * matrixCoefficientsStr;

    if (args)
    {
        matrixCoefficientsStr = strtok(args, " \t\n");
        app->args.matrixCoefficients = APP_ParseMatrixCoefficients(matrixCoefficientsStr);
        rc = APP_ApplyColorSettings(app);
        if (rc) { rc = BERR_TRACE(rc); goto end; }
        rc = APP_UpdateOsd(app);
        if (rc) { rc = BERR_TRACE(rc); goto end; }
    }
    else
    {
        APP_PrintMatrixCoefficients(app);
    }

  end:
    return rc;
}

int APP_SetupMatrixCoefficientsCommand(APP_AppHandle app)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    SHELL_CommandHandle command = NULL;

    command = SHELL_CreateCommand(app->shell,
        "ma",
        "changes HDMI output MatrixCoefficient. With no args, reports current matrixCoefficients status to console.",
        "[auto|input|709|170m|470_2_bg|2020ncl|2020cl]",
        &APP_DoMatrixCoefficients,
        app);
    if (!command) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto end; }
    rc = SHELL_AddCommandArg(command, "auto", false, "allow STB to decide MatrixCoefficient automatically");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "input", false, "follow content MatrixCoefficient dynamically");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "709", false, "selects MatrixCoefficient BT709");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "170m", false, "selects MatrixCoefficient Smpte_170M");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "470_2_bg", false, "selects MatrixCoefficient BT_470_2_BG ");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "2020ncl", false, "selects MatrixCoefficient BT2020 NCL");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "2020cl", false, "selects MatrixCoefficient BT2020 CL");
    if (rc) { rc = BERR_TRACE(rc); goto end; }

end:
    return rc;
}

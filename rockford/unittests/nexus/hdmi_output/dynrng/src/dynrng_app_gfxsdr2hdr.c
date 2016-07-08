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

void APP_PrintGfxSdr2Hdr(APP_AppHandle app)
{
    fprintf(stdout, "# g2Hdr\n");
    fprintf(stdout, "gfx sdrToHdr(%d %d %d)\n", app->args.gfxSdr2Hdr.y, app->args.gfxSdr2Hdr.cb, app->args.gfxSdr2Hdr.cr );
}

int APP_DoGfxSdr2Hdr(void * context, char * args)
{
    APP_AppHandle app = (APP_AppHandle) context;
    NEXUS_GraphicsSettings graphicsCompositorSettings;
    NEXUS_Error rc = NEXUS_SUCCESS;
    int y, cb, cr;

    if ( args )
    {
        if (sscanf(args, "%d %d %d", &y, &cb, &cr) == 3)
        {
            app->args.gfxSdr2Hdr.y = (short) y;
            app->args.gfxSdr2Hdr.cb = (short) cb;
            app->args.gfxSdr2Hdr.cr = (short) cr;
            NEXUS_Display_GetGraphicsSettings(app->display, &graphicsCompositorSettings);
            graphicsCompositorSettings.sdrToHdr.y = app->args.gfxSdr2Hdr.y;
            graphicsCompositorSettings.sdrToHdr.cb = app->args.gfxSdr2Hdr.cb;
            graphicsCompositorSettings.sdrToHdr.cr = app->args.gfxSdr2Hdr.cr;
            rc = NEXUS_Display_SetGraphicsSettings(app->display, &graphicsCompositorSettings);
            if (rc) { rc = BERR_TRACE(rc); goto end; }
            rc = APP_UpdateOsd(app);
            if (rc) { rc = BERR_TRACE(rc); goto end; }
        }
        else
        {
            fprintf(stdout, "g2Hdr requires [y cb cr] in short int format\n");
        }
    }
    else
    {
        APP_PrintGfxSdr2Hdr(app);
    }

  end:
    return rc;
}

int APP_SetupGfxSdr2HdrCommand(APP_AppHandle app)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    SHELL_CommandHandle command = NULL;

    command = SHELL_CreateCommand(app->shell,
        "g2hdr",
        "changes HDMI output SDR to HDR conversion adjustment. With no args, reports current Sdr2Hdr status to console.",
        "[y cb cy], where y, cb, cr are short int",
        &APP_DoGfxSdr2Hdr,
        app);
    if (!command) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto end; }

end:
    return rc;
}

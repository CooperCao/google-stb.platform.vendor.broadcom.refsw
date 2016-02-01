/***************************************************************************
 *     (c)2011-2013 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
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
 **************************************************************************/
#include "nexus_surface_compositor_module.h"
#include "nexus_surface_compositor_impl.h"

BDBG_MODULE(nexus_surface_compositor);

NEXUS_ModuleHandle g_NEXUS_surface_compositorModule = NULL;
NEXUS_SurfaceCompositorModuleSettings g_NEXUS_SurfaceCompositorModuleSettings;

void NEXUS_SurfaceCompositorModule_GetDefaultSettings( NEXUS_SurfaceCompositorModuleSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_ModuleHandle NEXUS_SurfaceCompositorModule_Init( const NEXUS_SurfaceCompositorModuleSettings *pSettings )
{
    NEXUS_ModuleSettings moduleSettings;
    BDBG_ASSERT(!g_NEXUS_surface_compositorModule);
    g_NEXUS_SurfaceCompositorModuleSettings = *pSettings;
    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_ModulePriority_eDefault;
    moduleSettings.dbgPrint = NEXUS_SurfaceCompositorModule_P_Print;
    moduleSettings.dbgModules = "nexus_surface_compositor";
    g_NEXUS_surface_compositorModule = NEXUS_Module_Create("surface_compositor", &moduleSettings);
    return g_NEXUS_surface_compositorModule;
}

void NEXUS_SurfaceCompositorModule_Uninit(void)
{
    NEXUS_Module_Destroy(g_NEXUS_surface_compositorModule);
    g_NEXUS_surface_compositorModule = NULL;
}

static NEXUS_Error NEXUS_P_SurfaceCompositorRenderElements_Init(NEXUS_P_SurfaceCompositorRenderElements *renderElements)
{
    renderElements->size = 2;
    renderElements->prev_size = 1;
    renderElements->count = 0;
    renderElements->data = BKNI_Malloc(sizeof(*renderElements->data)*renderElements->size);
    if(renderElements->data==NULL) {return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);}
    return NEXUS_SUCCESS;
}

void NEXUS_P_SurfaceCompositorRenderElements_Shutdown(NEXUS_P_SurfaceCompositorRenderElements *renderElements)
{
    if(renderElements->data) {
        BKNI_Free(renderElements->data);
        renderElements->data = NULL;
        renderElements->size = 0;
    }
    return;
}

static NEXUS_Error NEXUS_P_SurfaceCompositorRenderElements_Grow(NEXUS_P_SurfaceCompositorRenderElements *renderElements)
{
    unsigned new_size = renderElements->size + renderElements->prev_size;
    NEXUS_P_SurfaceCompositorRenderElement *new_data;

    BDBG_ASSERT(new_size > renderElements->size);

    new_data = BKNI_Malloc(sizeof(*new_data)*new_size);
    if(new_data==NULL) {return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);}

    BKNI_Memcpy(new_data, renderElements->data, sizeof(*new_data)*renderElements->size);
    BKNI_Free(renderElements->data);
    renderElements->data = new_data;
    renderElements->prev_size = renderElements->size;
    renderElements->size = new_size;
    return NEXUS_SUCCESS;
}

NEXUS_P_SurfaceCompositorRenderElement *NEXUS_P_SurfaceCompositorRenderElements_Next(NEXUS_P_SurfaceCompositorRenderElements *renderElements)
{
    NEXUS_P_SurfaceCompositorRenderElement *data;

    if(renderElements->count >= renderElements->size) {
        NEXUS_Error rc;
        if(renderElements->size==0) {
            rc = NEXUS_P_SurfaceCompositorRenderElements_Init(renderElements);
            if(rc!=NEXUS_SUCCESS) {(void)BERR_TRACE(rc);return NULL; }
        } else {
            rc = NEXUS_P_SurfaceCompositorRenderElements_Grow(renderElements);
            if(rc!=NEXUS_SUCCESS) {(void)BERR_TRACE(rc);return NULL; }
        }
        BDBG_ASSERT(renderElements->count < renderElements->size);
    }
    data = renderElements->data + renderElements->count;
    renderElements->count++;
    return data;
}

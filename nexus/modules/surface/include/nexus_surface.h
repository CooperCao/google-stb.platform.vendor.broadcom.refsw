/***************************************************************************
 *     (c)2007-2013 Broadcom Corporation
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
#ifndef NEXUS_SURFACE_H__
#define NEXUS_SURFACE_H__

#include "nexus_types.h"
#include "nexus_surface_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=*******************************************
The Surface module allows the creation and management of graphics surfaces in memory.
These surfaces can be used by other Modules such as Graphics2D, Display, VideoDecoder and PictureDecoder.
*********************************************/

/**
Summary:
Handle for the Surface interface.

Description:
A graphics surface is a block of relocatable memory with various properties include width, height, pitch, and pixel format.
When application want to access the surface buffer, application should use function call NEXUS_Surface_Lock/NEXUS_Surface_Unlock.

See Also:
NEXUS_Surface_Lock - used for host to get memory pointer to access memory directly
NEXUS_Surface_Unlock - used by host to notify that memory pointer provided NEXUS_Surface_Lock is no longer referenced
NEXUS_Surface_GetMemory - used for host to write directly to memory
NEXUS_Display_SetGraphicsFramebuffer - set the current graphics framebuffer
NEXUS_Graphics2DFillSettings - fill a surface with a pixel
NEXUS_Graphics2DBlitSettings - blit from one surface to another
**/
typedef struct NEXUS_Surface *NEXUS_SurfaceHandle;

/*
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new structure members in the future.

See Also:
NEXUS_Surface_Create
*/
void NEXUS_Surface_GetDefaultCreateSettings(
    NEXUS_SurfaceCreateSettings *pCreateSettings /* [out] */
    );

/**
Summary:
Creates a new surface using the create settings.

Description:
A surface is a block of memory with the properties of height, width, pitch and pixel format.

The user is responsible for call NEXUS_Surface_Destroy for any surface which is created.
Surfaces are not automatically freed by another Interface.

Memory is allocated from the memory heap provided by the Core module.

This function returns NULL if there is no enough memory for the surface.

See Also:
NEXUS_Surface_GetDefaultCreateSettings
NEXUS_Surface_GetCreateSettings
NEXUS_Surface_Destroy
**/
NEXUS_SurfaceHandle NEXUS_Surface_Create( /* attr{destructor=NEXUS_Surface_Destroy_driver}  */
    const NEXUS_SurfaceCreateSettings *pCreateSettings
    );

/**
Summary:
Destroy a surface and free its resources.

Description:
This surface should not be currently in use by another module.
After calling NEXUS_Surface_Destroy, the handle is no longer valid and should not be used.

If you have written to cached memory and have not flushed, you should call NEXUS_Surface_Flush
before deleting this surface. This cannot be done manually without adversely impacting performance.

See Also:
NEXUS_Surface_Create
**/
void NEXUS_Surface_Destroy( /* attr{local=true}  */
    NEXUS_SurfaceHandle surface
    );

/*
Summary:
Get settings used to create the surface.

Description:
NEXUS_SurfaceCreateSettings.heap will be populated with the default heap if the user passed in NULL.
*/
void NEXUS_Surface_GetCreateSettings(
    NEXUS_SurfaceHandle surface,
    NEXUS_SurfaceCreateSettings *pCreateSettings /* [out] */
    );

/*
Summary:
Get surface status

Description:
*/
void NEXUS_Surface_GetStatus(
    NEXUS_SurfaceHandle surface,
    NEXUS_SurfaceStatus *pStatus /* [out] */
    );


/**
Summary:
Returns information needed for direct memory writes.

Description:

NEXUS_Surface_GetMemory locks surface in memory and the surface buffer relocation, effectively limiting options for heap de-fragmentation.

In order for direct memory writes to be successful, you must know the
format specified by NEXUS_PixelFormat.
You cannot assume that pitch == width * sizeof(pixelformat).

Example:
    //assuming 32bpp surface
    NEXUS_Surface_GetCreateSettings(surface, &createSettings);
    NEXUS_Surface_GetMemory(surface, &mem);
    for (y=0;y<createSettings.height;y++) {
        for (x=0;x<createSettings.width;x++) {
            ((uint32_t*)mem.buffer)[y*mem.pitch + x] = value;
        }
    }
    NEXUS_Surface_Flush(surface);

See Also:
NEXUS_Surface_Flush
**/
NEXUS_Error NEXUS_Surface_GetMemory( /* attr{local=true}  */
    NEXUS_SurfaceHandle surface,
    NEXUS_SurfaceMemory *pMemory /* [out] */
    );

/**
Summary:
Flush a surface's cached memory and palette

Description:
If you are doing direct memory writes to the surface, you should flush before doing any hardware read/write
operations (e.g. Graphics2D blit/fill or setting the Display framebuffer).

Also, if you are doing direct memory reads from a surface, you should also flush after doing any hardware write
operations. See the module overview in nexus_dma.h for a full discussion of cache coherency problems caused
by the RAC (read-ahead cache).

If you have updated the palette and this surface is the framebuffer, you must also call NEXUS_Display_SetGraphicsFramebuffer
to read the updated palette into the graphics feeder.
**/
void NEXUS_Surface_Flush( /* attr{local=true}  */
    NEXUS_SurfaceHandle surface
    );




/*
Summary:
Settings for a surface returned by NEXUS_Surface_GetSettings.
*/
typedef struct NEXUS_SurfaceSettings
{
    bool autoFlush; /* deprecated. Minimal cache flush is essential for performance; therefore app should call NEXUS_Surface_Flush manually
                       and only when needed. See NEXUS_Surface_Flush for notes. */
} NEXUS_SurfaceSettings;

/**
Summary:
Apply NEXUS_SurfaceSettings to a surface.
**/
NEXUS_Error NEXUS_Surface_SetSettings(
    NEXUS_SurfaceHandle surface,
    const NEXUS_SurfaceSettings *pSettings
    );

/*
Summary:
Get NEXUS_SurfaceSettings for a surface.
*/
void NEXUS_Surface_GetSettings(
    NEXUS_SurfaceHandle surface,
    NEXUS_SurfaceSettings *pSettings /* [out] */
    );

/*
Summary:
Initialize a BM2MC_PACKET_Plane for use in NEXUS_Graphics2D packet blit

Description:
This will set the required address, format and pitch for the blit.
It will also set the optional width and height so that export NEXUS_GRAPHICS2D_VERIFY_PACKETS=y
can verify the source, dest and output rectangle bounds.
*/
NEXUS_Error NEXUS_Surface_InitPlaneAndPaletteOffset(
    NEXUS_SurfaceHandle surface,
    BM2MC_PACKET_Plane *pPlane, /* [out] */
    NEXUS_Addr *pPaletteOffset    /* [out] attr{null_allowed=y} physical address if palette memory if application. 0 if non-palletized surface. */
    );

/*
Summary:
Initialize a BM2MC_PACKET_Plane for use in NEXUS_Graphics2D packet blit and also retrieve palette offset
*/
#define NEXUS_Surface_InitPlane(surface,pPlane)  NEXUS_Surface_InitPlaneAndPaletteOffset((surface),(pPlane),NULL)

/*
Summary:
This function would lock surface in memory and return pointer to the surface buffer
*/
NEXUS_Error NEXUS_Surface_Lock( /* attr{local=true} */
        NEXUS_SurfaceHandle surface,
        void **ppMemory                                     /* [out] pointer to memory that has been allocated */
        );

/*
Summary:
This function used by application to notify that it's not using the surface buffer, and surface buffer could be moved in memory
*/
void NEXUS_Surface_Unlock( /* attr{local=true} */
        NEXUS_SurfaceHandle surface
        );

/*
Summary:
This function would lock the surface palette buffer surface in memory and return pointer to the palette buffer
*/
NEXUS_Error NEXUS_Surface_LockPalette( /* attr{local=true} */
        NEXUS_SurfaceHandle surface,
        void **ppMemory                                     /* [out] pointer to memory that has been allocated */
        );

/*
Summary:
This function used by application to notify it's not using the surface palette buffer, and the palette buffer could be moved in memory
*/
void NEXUS_Surface_UnlockPalette( /* attr{local=true} */
        NEXUS_SurfaceHandle surface
        );


/*
Summary:
Initialize a BM2MC_PACKET_Plane for use in NEXUS_Graphics2D packet blit and locks surface in memory

Description:
This will set the required address, format and pitch for the blit.
It will also set the optional width and height so that export NEXUS_GRAPHICS2D_VERIFY_PACKETS=y
can verify the source, dest and output rectangle bounds.
*/
NEXUS_Error NEXUS_Surface_LockPlaneAndPalette(
    NEXUS_SurfaceHandle surface,
    BM2MC_PACKET_Plane *pPlane, /* [out] */
    NEXUS_Addr *pPaletteOffset    /* [out]  attr{null_allowed=y} physical address if palette memory if application. 0 if non-palletized surface. */
    );

/*
Summary:
This function used by application when it doesn't use BM2MC_PACKET for operation on surface, and any outstanding operations were already completed
*/
void NEXUS_Surface_UnlockPlaneAndPalette(
    NEXUS_SurfaceHandle surface
    );


#define NEXUS_Surface_LockPlane(surface, plane) NEXUS_Surface_LockPlaneAndPalette(surface, plane, NULL)
#define NEXUS_Surface_UnlockPlane(surface) NEXUS_Surface_UnlockPlaneAndPalette(surface)

/*
Summary:
This function used by application to communicate information that surface doesn't hold any important data, and during compaction event contents of the surface would get discarded
*/
void NEXUS_Surface_MarkDiscardable(
    NEXUS_SurfaceHandle surface
    );

void NEXUS_Surface_Destroy_driver(
    NEXUS_SurfaceHandle surface
    );

/***************************************************************************
Summary:
Properties of surface, related to type of used memory
***************************************************************************/
typedef struct NEXUS_SurfaceMemoryProperties
{
    NEXUS_MemoryBlockHandle pixelMemory; /* memory block that is used for the pixel buffer */
    unsigned pixelMemoryOffset;    /* offset from the start of the pixelMemory to the pixel buffer */
    NEXUS_MemoryBlockHandle paletteMemory; /* memory block that is used for the palette buffer */
    unsigned paletteMemoryOffset;    /* offset from the start of the paletteMemory to the palette buffer */
    bool pixelMemoryTransient;     /* pixel memory created just for this surface */
    bool paletteMemoryTransient;   /* palette memory created just for this surface */
} NEXUS_SurfaceMemoryProperties;

/***************************************************************************
Summary:
Returns memory properties of the surface
***************************************************************************/
void NEXUS_Surface_GetMemoryProperties(
    NEXUS_SurfaceHandle surface,
    NEXUS_SurfaceMemoryProperties *pProperties
    );

#ifdef __cplusplus
}
#endif

#endif

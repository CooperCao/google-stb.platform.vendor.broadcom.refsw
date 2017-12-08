#ifndef __eglplatform_h_
#define __eglplatform_h_

/*
** Copyright (c) 2007-2009 The Khronos Group Inc.
**
** Permission is hereby granted, free of charge, to any person obtaining a
** copy of this software and/or associated documentation files (the
** "Materials"), to deal in the Materials without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Materials, and to
** permit persons to whom the Materials are furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be included
** in all copies or substantial portions of the Materials.
**
** THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
*/

/* Platform-specific types and definitions for egl.h
 * $Revision: 7244 $ on $Date: 2009-01-20 17:06:59 -0800 (Tue, 20 Jan 2009) $
 *
 * Adopters may modify khrplatform.h and this file to suit their platform.
 * You are encouraged to submit all modifications to the Khronos group so that
 * they can be included in future versions of this file.  Please submit changes
 * by sending them to the public Khronos Bugzilla (http://khronos.org/bugzilla)
 * by filing a bug against product "EGL" component "Registry".
 */

#include "../KHR/khrplatform.h"

#include "begl_memplatform.h"
#include "begl_hwplatform.h"
#include "begl_dispplatform.h"

/* Macros used in EGL function prototype declarations.
 *
 * EGLAPI return-type EGLAPIENTRY eglFunction(arguments);
 * typedef return-type (EXPAPIENTRYP PFNEGLFUNCTIONPROC) (arguments);
 *
 * On Windows, EGLAPIENTRY can be defined like APIENTRY.
 * On most other platforms, it should be empty.
 */

#ifndef EGLAPIENTRY
#define EGLAPIENTRY  KHRONOS_APIENTRY
#endif
#ifndef EGLAPIENTRYP
#define EGLAPIENTRYP EGLAPIENTRY*
#endif
#ifndef EGLAPI
#ifdef KHAPI
#define EGLAPI KHAPI
#else
#define EGLAPI extern
#endif
#endif

/* The types NativeDisplayType, NativeWindowType, and NativePixmapType
 * are aliases of window-system-dependent types, such as X Display * or
 * Windows Device Context. They must be defined in platform-specific
 * code below. The EGL-prefixed versions of Native*Type are the same
 * types, renamed in EGL 1.3 so all types in the API start with "EGL".
 */

/* Unix (tentative)
    #include <X headers>
    typedef Display *NativeDisplayType;
      - or maybe, if encoding "hostname:display.head"
    typedef const char *NativeWindowType;
        etc.
 */

typedef void *EGLNativeDisplayType;
typedef void *EGLNativePixmapType;
typedef void *EGLNativeWindowType;

/* EGL 1.2 types, renamed for consistency in EGL 1.3 */
typedef EGLNativeDisplayType NativeDisplayType;
typedef EGLNativePixmapType  NativePixmapType;
typedef EGLNativeWindowType  NativeWindowType;


/* Define EGLint. This must be a signed integral type large enough to contain
 * all legal attribute names and values passed into and out of EGL, whether
 * their type is boolean, bitmask, enumerant (symbolic constant), integer,
 * handle, or other.  While in general a 32-bit integer will suffice, if
 * handles are 64 bit types, then EGLint should be defined as a signed 64-bit
 * integer type.
 */
typedef khronos_int32_t EGLint;

#ifdef __cplusplus
extern "C"
{
#endif

/* A BEGL_RegisterInitInterface() should be called by the platform library
 * constructor in order to set EGL initialisation callbacks. Those are called
 * by the driver in response to eglGetDisplay(), eglInitialize(display)
 * and eglTerminate(display).
 *
 * Alternatively (legacy mode) BEGL_RegisterDriverInterfaces() may be called
 * at platform startup in cases where only one display is ever present
 * on a given platform.
 */

/*
 * The client application, or default platform library must register valid
 * versions of each of these interfaces before any EGL or GL functions are
 * invoked.
 *
 * This is either done by returning the structure
*/
typedef struct
{
   BEGL_MemoryInterface  *memInterface;      /* Memory interface which will called by the 3d driver   */
   BEGL_HWInterface      *hwInterface;       /* Hardware interface which will be called by the driver */
   BEGL_DisplayInterface *displayInterface;  /* Display interface which will be called by the driver  */

   BEGL_DisplayCallbacks  displayCallbacks;  /* Callback pointers set by BEGL_GetDefaultDriverInterfaces, for client to call into driver */
   BEGL_HardwareCallbacks hardwareCallbacks; /* Callback pointers set by BEGL_GetDefaultDriverInterfaces, for client to call into driver */

   int hwInterfaceCloned;
   int memInterfaceCloned;
   void *memInterfaceFn;
   void *hwInterfaceFn;
} BEGL_DriverInterfaces;

/* Register application level overrides for any or all of the abstract API calls made by the 3D driver. */
EGLAPI void EGLAPIENTRY BEGL_RegisterDriverInterfaces(BEGL_DriverInterfaces *iface);

/* Get a pointer to the registered driver interfaces, can be used to override partial defaults - see android platform layer(s) for example */
EGLAPI BEGL_DriverInterfaces* EGLAPIENTRY BEGL_GetDriverInterfaces(void);

/* Initializes all interfaces in the structure to NULL, fills out Callbacks with appropriate function pointers */
EGLAPI void EGLAPIENTRY BEGL_GetDefaultDriverInterfaces(BEGL_DriverInterfaces *iface);

typedef void  *BEGL_DisplayHandle;    /* Opaque 'display' handle */

typedef struct BEGL_InitInterface
{
   /* Context pointer - opaque to the 3d driver code, but passed out in all
    * function pointer calls. Prevents the client code needing to perform
    * context lookups.
    */
   void *context;

   /* Called from eglGetDisplay().
    *
    * Returns BEGL_Success if platform parameter is valid, BEGL_Fail otherwise.
    *
    * Sets opaque native display handle to a non-NULL value if a display
    * matching passed-in parameters could be found or created.
    * Multiple calls with the same parameters must always return the same
    * display handle.
    *
    * In case of a valid platform but non-existing display the implementation
    * should set display handle to NULL and return BEGL_Success.
    */
   BEGL_Error (*GetDisplay)(void *context, uint32_t platform,
         void *nativeDisplay, const EGLint *attribList,
         BEGL_DisplayHandle *handle);

   /*
    * Called from eglInitialize().
    *
    * The display handle is the value passed in to eglInitialize()
    * and must be validated before use.
    *
    * A successful initialisation must call BEGL_RegisterDriverInterfaces()
    * with non-NULL hardware, memory and display interfaces.
    */
   BEGL_Error (*Initialize)(void *context, BEGL_DisplayHandle handle);

   /*
    * Called from eglTerminate().
    *
    * The display handle is the value passed in to eglTerminate().
    * and must be validated before use.
    *
    * A successful termination must call BEGL_RegisterDriverInterfaces()
    * with the same hardware and memory interfaces as already registered
    * in Initialize() and with a NULL display interface.
    */
   BEGL_Error (*Terminate)(void *context, BEGL_DisplayHandle handle);

} BEGL_InitInterface;

EGLAPI void EGLAPIENTRY BEGL_RegisterInitInterface(BEGL_InitInterface *iface);


#ifdef __cplusplus
}
#endif

#if 0
#include "interface/khronos/common/khrn_client_mangle.h"
#endif

#endif /* __eglplatform_h */

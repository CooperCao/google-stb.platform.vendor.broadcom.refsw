/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "begl_memplatform.h"
#include "begl_hwplatform.h"
#include "begl_dispplatform.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef KHAPI
#   define BEGLAPI KHAPI
#else
#   define BEGLAPI extern
#endif

#if defined(_WIN32) && !defined(_WIN32_WCE) && !defined(__SCITECH_SNAP__)
    /* Win32 but not WinCE */
#   define BEGLAPIENTRY __stdcall
#else
#   define BEGLAPIENTRY
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

   BEGL_HardwareCallbacks hardwareCallbacks; /* Callback pointers set by BEGL_GetDefaultDriverInterfaces, for client to call into driver */

   int hwInterfaceCloned;
   int memInterfaceCloned;
   void *memInterfaceFn;
   void *hwInterfaceFn;
} BEGL_DriverInterfaces;

/* Register application level overrides for any or all of the abstract API calls made by the 3D driver. */
BEGLAPI void BEGLAPIENTRY BEGL_RegisterDriverInterfaces(BEGL_DriverInterfaces *iface);

/* Get a pointer to the registered driver interfaces, can be used to override partial defaults - see android platform layer(s) for example */
BEGLAPI BEGL_DriverInterfaces* BEGLAPIENTRY BEGL_GetDriverInterfaces(void);

/* Initializes all interfaces in the structure to NULL, fills out Callbacks with appropriate function pointers */
BEGLAPI void BEGLAPIENTRY BEGL_GetDefaultDriverInterfaces(BEGL_DriverInterfaces *iface);

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
         void *nativeDisplay, const int32_t *attribList,
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

BEGLAPI void BEGLAPIENTRY BEGL_RegisterInitInterface(BEGL_InitInterface *iface);

#ifdef __cplusplus
}
#endif

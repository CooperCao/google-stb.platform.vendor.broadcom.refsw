/******************************************************************************
 *   (c)2011-2012 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
 * AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
 * SOFTWARE.  
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE
 * ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 * ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#ifndef __BSG_CONTEXT_H__
#define __BSG_CONTEXT_H__

#include "bsg_common.h"
#include "bsg_gl.h"
#include <stdint.h>

namespace bsg
{
class ApplicationOptions;
class NativePixmap;

//! A Context represents an EGL rendering context and destination surface.
//! It manages the creation of the underlying EGL context based upon the ApplicationOptions.
class Context
{
public:
   Context();
   virtual ~Context();

   //! Initialize the context based upon the ApplicationOptions.
   //! Rendering using this context will send the output to nativewindow.
   virtual void Initialize(const ApplicationOptions &options, EGLNativeWindowType nativeWindow);

   //! Initialize the context based upon the ApplicationOptions.
   //! Rendering using this context will send the output to a pixmap.
   virtual void Initialize(const ApplicationOptions &options, NativePixmap *nativePixmap);

   virtual EGLConfig ChooseConfigForDisplay(EGLDisplay display, const ApplicationOptions &options);
   
   virtual void InitializeFromNativeWindow(const ApplicationOptions &options, EGLConfig config, EGLNativeWindowType nativeWindow);

   virtual void InitializeWithPixmap(const ApplicationOptions &options, EGLConfig config, NativePixmap *nativePixmap);

   //! Clean-up the context ready for termination
   virtual void Terminate();

   //! Make this context the current rendering context
   virtual void MakeCurrent();

   //! Present the current rendering to the display by swapping front and back buffers
   virtual void SwapBuffers();

   //! Sets the number of vsyncs between displayed frames. e.g.
   //! - 0 = un-synced - frames are displayed as quickly as possible. Tearing may result. Only really useful for benchmarking.
   //! - 1 = swap every vsync - maximum display rate of 60fps.
   //! - 2 = swap every 2 vsycns - maximum display rate of 30fps.
   //! - 5 = swap every 5 vsycns - maximum display rate of 12fps.
   void SetSwapInterval(int32_t interval);

   //! Return the EGL display
   EGLDisplay GetDisplay() const { return m_display; }

   //! Return the EGL surface
   EGLSurface GetSurface() const { return m_surface; }

   //! Return the EGL config used to create the EGL context
   EGLConfig GetConfig() const { return m_config; }

   //! Return the EGL context
   EGLContext GetContext() const { return m_context; }

private:

   void InitializeCreateContext(const ApplicationOptions &options, EGLConfig config);

   EGLDisplay m_display;
   EGLSurface m_surface;
   EGLContext m_context;
   EGLConfig  m_config;
};

}

#endif /* __BSG_CONTEXT_H__ */


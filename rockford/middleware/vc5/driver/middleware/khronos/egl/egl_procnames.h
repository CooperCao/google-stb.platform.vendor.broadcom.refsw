/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION

Just a list of all the functions that eglGetProcAddress should be able to
return the address of. We turn this into code with a bit of macro trickery.
=============================================================================*/

#ifndef PROCNAME
#  error "You must define PROCNAME before including this"
#endif

PROCNAME(eglCreateImageKHR)
PROCNAME(eglDestroyImageKHR)
PROCNAME(glDiscardFramebufferEXT)
PROCNAME(glPointSizePointerOES)
PROCNAME(glEGLImageTargetTexture2DOES)
PROCNAME(glEGLImageTargetRenderbufferStorageOES)
PROCNAME(glDrawTexfOES)
PROCNAME(glQueryMatrixxOES)
PROCNAME(glBindVertexArrayOES)
PROCNAME(glDeleteVertexArraysOES)
PROCNAME(glGenVertexArraysOES)
PROCNAME(glIsVertexArrayOES)
PROCNAME(glTexImage1DBRCM)
PROCNAME(glObjectLabelKHR)

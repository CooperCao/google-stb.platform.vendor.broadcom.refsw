/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef EGL_ATTRIB_LIST_H
#define EGL_ATTRIB_LIST_H

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <stdbool.h>

typedef enum egl_attribtype {
   attrib_EGLint,
   attrib_EGLAttribKHR,
   attrib_EGLAttrib = attrib_EGLAttribKHR
} EGL_AttribType;

/*
 * Get an attribute form the EGL attribute list (C array).
 *
 * The list items can be of a type specified by the 'type' parameter.
 * Passing true as the 'increment' param will post-increment the list pointer
 * so that subsequent list elements can be retrieved by calling:
 *
 * first_attr = egl_attrib_list_item(&my_list, attr_type, true);
 * second_attr = egl_attrib_list_item(&my_list, attr_type, true);
 *
 * and so on.
 *
 * Note: this will advance the list_ptr to the next attribute.
 * The returned attribute is up-converted to EGLAttribKHR.
 */
extern EGLAttribKHR egl_attrib_list_item(const void **list_ptr,
      EGL_AttribType type, bool increment);

/*
 * Get a name-value pair from the attribute list.
 *
 * Return false if list is NULL (i.e. *list_ptr==NULL) or the next name
 * on the list is EGL_NONE. The list_ptr, name and value must not be NULL.
 */
extern bool egl_next_attrib(const void **list_ptr, EGL_AttribType type,
      EGLint *name, EGLAttribKHR *value);

#endif /* EGL_ATTRIB_LIST_H */

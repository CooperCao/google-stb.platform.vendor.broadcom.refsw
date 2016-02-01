/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  3D Tools
Module   :  Control list debugger

FILE DESCRIPTION
Dumps control lists to human readable form for debugging purposes.
=============================================================================*/

#ifndef _CONTROL_LIST_DEBUG_
#define _CONTROL_LIST_DEBUG_

#ifdef PRINT_CLIST_DEBUG

#include <stdio.h>

/* Rendering = 1 for render list, 0 for bin list */
extern void control_list_debug_file(FILE *fp);
extern void control_list_debug_dump(void *list_begin, void *list_end, unsigned int rendering);

#endif /* PRINT_CLIST_DEBUG */

#endif

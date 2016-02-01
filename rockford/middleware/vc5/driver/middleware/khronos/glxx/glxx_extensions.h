/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
=============================================================================*/
#ifndef KHRN_EXTENSIONS_H
#define KHRN_EXTENSIONS_H

/* returns the number of gl30 extensions */
extern unsigned int glxx_get_num_gl30_extensions(void);

/* return the ith extension name
   or NULL if i is not a valid index */
extern const char *glxx_get_gl30_extension(unsigned int i);

#endif /* KHRN_EXTENSIONS_H */
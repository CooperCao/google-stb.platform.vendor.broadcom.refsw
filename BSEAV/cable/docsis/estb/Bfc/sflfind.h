/*  -----------------------------------------------------------------
    Name:       sflfind.h
    Title:      Fast string searching functions
    Package:    Standard Function Library (SFL)

    Written:    96/04/24  iMatix SFL project team 
    Revised:    98/05/03

    Synopsis:   Searches for a pattern within a string or block of memory
                using a variant of the Boyer-Moore algorithm (improved by
                Horspool and Sunday). As fast or faster than the normal
                Boyer-Moore algorithm for most search strings, and much
                simpler.  Includes a basic function for searching blocks of
                memory with known sizes, plus an envelope that searches
                null-delimited strings.  Provides the option of repeatedly
                searching for the same pattern without re-parsing the pattern
                each time.  Original algorithm published by BOYER, R., and S.
                MOORE 1977, "A Fast String Searching Algorithm." CACM, 20,
                762-72.  Simplifications by HORSPOOL, R. N. 1980, "Practical
                Fast Searching in Strings." Software - Practice and Experience,
                10, 501-06.  More improvements by HUME, A., and D. M. SUNDAY
                1991, "Fast String Searching." AT&T Bell Labs Computing Science
                Technical Report No. 156.  Implemented in C by P. Hintjens.

                strfind_r() and memfind_r(), are reentrant versions of
                strfind() and memfind() for single searches, and
                strfind_rb() and memfind_rb() are reentrant versions of
                strfind() and memfind() supporting repeat searches against
                the same pattern.

    Copyright:  Copyright (c) 1991-98 iMatix
    License:    This is free software; you can redistribute it and/or modify
                it under the terms of the SFL License Agreement as provided
                in the file LICENSE.TXT.  This software is distributed in
                the hope that it will be useful, but without any warranty.
 -------------------------------------------------------------------*/

#ifndef _SFLFIND_INCLUDED               /*  Allow multiple inclusions        */
#define _SFLFIND_INCLUDED

#define size_t unsigned long
#define Bool unsigned char
#define byte unsigned char
#define TRUE 1
#define FALSE 0

/*  Function prototypes                                                      */

#ifdef __cplusplus
extern "C" {
#endif
void *memfind_r  (const void *block,   size_t block_size,
                  const void *pattern, size_t pattern_size);
void *memfind_rb (const void *block,   size_t block_size,
                  const void *pattern, size_t pattern_size,
                  size_t *shift, Bool *repeat_find);

char * txtfind (const char *string, /* String containing data */
				const char *pattern); /* Pattern to search for */

char *
strfind (const char *string,            /*  String containing data           */
         const char *pattern,           /*  Pattern to search for            */
         Bool repeat_find);             /*  Same pattern as last time        */

#ifdef __cplusplus
}
#endif

#endif


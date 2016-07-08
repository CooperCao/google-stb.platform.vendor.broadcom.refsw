/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/
#pragma once

/* returns the number of extensions */
extern unsigned int glxx_get_num_extensions(void);

/* return the ith extension name
   or NULL if i is not a valid index */
extern const char *glxx_get_extension(unsigned int i);

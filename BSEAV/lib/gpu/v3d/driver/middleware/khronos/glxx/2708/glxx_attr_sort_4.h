/*=============================================================================
Broadcom Proprietary and Confidential. (c)2010 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  sorting algorithm for packed vertex attributes

FILE DESCRIPTION
vertex attributes can be supplied packed in the same buffer
this algorithm orders the enabled attributes for use by the VCD/VPM
=============================================================================*/

#ifndef GLXX_ATTR_SORT_4_H
#define GLXX_ATTR_SORT_4_H

/*#define GLXX_WANT_ATTRIBUTE_MERGING*/
#define GLXX_WANT_ATTRIBUTE_SORTING

extern void glxx_sort_attributes(GLXX_ATTRIB_T *attrib,
   uint32_t cattribs_live,
   uint32_t vattribs_live,
   uint32_t * mergeable_attributes,
   uint32_t * cattribs_order_out,
   uint32_t * vattribs_order_out);

#endif
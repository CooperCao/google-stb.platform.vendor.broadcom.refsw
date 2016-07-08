/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
abstract platform layer
=============================================================================*/
#ifndef KHRN_CLIENT_PLATFORM_ABSTRACT_H
#define KHRN_CLIENT_PLATFORM_ABSTRACT_H

extern KHRN_IMAGE_FORMAT_T abstract_colorformat_to_format(KHRN_IMAGE_FORMAT_T format, BEGL_ColorFormat colorformat);
extern BEGL_ColorFormat format_to_abstract_colorformat(KHRN_IMAGE_FORMAT_T format);

#endif /* KHRN_CLIENT_PLATFORM_ABSTRACT_H */
/**@@@+++@@@@******************************************************************
**
** Microsoft (r) PlayReady (r)
** Copyright (c) Microsoft Corporation. All rights reserved.
**
***@@@---@@@@******************************************************************
*/

#ifndef __BYTEORDER_H__
#define __BYTEORDER_H__

#include <oembyteorder.h>

#define DRM_XOR( pbLeft, pbRight, count) DRM_DO {                                                    \
        DRM_DWORD __count_drm_xor = count;                                                           \
        while ( __count_drm_xor > 0)                                                                 \
        {                                                                                            \
            __count_drm_xor--;                                                                       \
            ((DRM_BYTE*)(pbLeft))[__count_drm_xor] ^= ((DRM_BYTE*)(pbRight))[__count_drm_xor];       \
        }                                                                                            \
    } DRM_WHILE_FALSE

#define COPY_FROMBUFFER(to, from, index, size, buffersize)              \
DRM_DO {                                                                \
        DRM_DWORD __dwSpaceRequired=0;                                  \
        ChkDR(DRM_DWordAdd(index,size,&__dwSpaceRequired));             \
        ChkBOOL(__dwSpaceRequired<=(buffersize),DRM_E_BUFFERTOOSMALL);  \
        DRM_BYT_CopyBytes((DRM_BYTE*)(to),0,(from),(index),(size));     \
        (index)=(__dwSpaceRequired);                                    \
    } DRM_WHILE_FALSE

#if TARGET_LITTLE_ENDIAN
#define NETWORKBYTES_FROMBUFFER(to, from, index, size, buffersize)       DRM_DO {COPY_FROMBUFFER(to,from,index,size,buffersize);DRM_BYT_ReverseBytes((DRM_BYTE*)(to),(size));} DRM_WHILE_FALSE
#else /* TARGET_LITTLE_ENDIAN */
#define NETWORKBYTES_FROMBUFFER(to, from, index, size, buffersize)       DRM_DO {COPY_FROMBUFFER(to,from,index,size,buffersize);DRM_BYT_ReverseBytes((DRM_BYTE*)(to),(size));} DRM_WHILE_FALSE
#endif /* TARGET_LITTLE_ENDIAN */

#endif /* __BYTEORDER_H__ */
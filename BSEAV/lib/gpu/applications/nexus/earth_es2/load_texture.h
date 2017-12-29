/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef _LOAD_TEXTURE_H__
#define _LOAD_TEXTURE_H__

#include <stdbool.h>
#include <GLES2/gl2.h>

#ifdef __cplusplus
extern "C" {
#endif

void InitTextureLoader(void);
bool LoadTexture(const char *pictureFilename, GLint mipLevel);
bool LoadCompressedTexture(const char *pictureFilename, GLint mipLevel);
void DestroyTextureLoader(void);

#ifdef __cplusplus
}
#endif

#endif /* _LOAD_TEXTURE_H__ */

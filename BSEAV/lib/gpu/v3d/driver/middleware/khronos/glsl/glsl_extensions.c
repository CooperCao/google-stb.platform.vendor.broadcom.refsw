/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Standalone GLSL compiler
=============================================================================*/
#include <string.h>
#include "glsl_extensions.h"

static struct {
    const char              *identifier;
    enum glsl_ext_status    status;

    /*
     * Code to "#include" when the extension is encountered. Can be NULL. Must
     * not itself include #extension directives that would cause other code to
     * be included (although there's no reason why it would).
     */
    const char              *contents;
} extensions[GLSL_EXT_COUNT] = {
    {"GL_OES_EGL_image_external", GLSL_DISABLED,

    "vec4 texture2D(samplerExternalOES sampler, vec2 coord, float bias)"
    "{"
    "   return $$texture_2d_bias(sampler, coord.st, bias);"
    "}"

    "vec4 texture2D(samplerExternalOES sampler, vec2 coord)"
    "{"
    "   return $$texture_2d_bias(sampler, coord.st, 0.0);"
    "}"

    "vec4 texture2DProj(samplerExternalOES sampler, vec3 coord, float bias)"
    "{"
    "   return $$texture_2d_bias(sampler, coord.st /= coord[2], bias);"
    "}"

    "vec4 texture2DProj(samplerExternalOES sampler, vec3 coord)"
    "{"
    "   return $$texture_2d_bias(sampler, coord.st /= coord[2], 0.0);"
    "}"

    "vec4 texture2DProj(samplerExternalOES sampler, vec4 coord, float bias)"
    "{"
    "   return $$texture_2d_bias(sampler, coord.st /= coord[3], bias);"
    "}"

    "vec4 texture2DProj(samplerExternalOES sampler, vec4 coord)"
    "{"
    "   return $$texture_2d_bias(sampler, coord.st /= coord[3], 0.0);"
    "}\n"}
};

bool glsl_ext_enable(enum glsl_ext extension, bool warn)
{
    int i;
    enum glsl_ext_status status = warn ? GLSL_ENABLED_WARN : GLSL_ENABLED;

    if (extension < GLSL_EXT_COUNT) {
        extensions[extension].status = status;
        return true;
    }

    if (extension == GLSL_EXT_ALL) {
        for (i = 0; i < GLSL_EXT_COUNT; i++)
            extensions[i].status = status;
    }
    return false;
}

bool glsl_ext_disable(enum glsl_ext extension)
{
    int i;

    if (extension < GLSL_EXT_COUNT) {
        extensions[extension].status = GLSL_DISABLED;
        return true;
    }

    if (extension == GLSL_EXT_ALL) {
        for (i = 0; i < GLSL_EXT_COUNT; i++)
            extensions[i].status = GLSL_DISABLED;
        return true;
    }

    return false;
}

enum glsl_ext_status glsl_ext_status(enum glsl_ext extension)
{
    if (extension >= GLSL_EXT_COUNT)
        return GLSL_DISABLED;

    return extensions[extension].status;
}

enum glsl_ext glsl_ext_lookup(const char *identifier)
{
    int i;

    for (i = 0; i < GLSL_EXT_COUNT; i++)
        if (!strcmp(extensions[i].identifier, identifier))
            return i;

    return GLSL_EXT_NOT_SUPPORTED;
}

const char *glsl_ext_contents(enum glsl_ext extension)
{
    if (extension < GLSL_EXT_COUNT)
        return extensions[extension].contents;
    return NULL;
}

const char *glsl_ext_get_identifier(int *i)
{
    if (*i >= GLSL_EXT_COUNT)
        return NULL;

    return extensions[(*i)++].identifier;
}

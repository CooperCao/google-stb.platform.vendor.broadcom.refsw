/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
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
    "   return $$texture(0, sampler, coord.st, bias);"
    "}"

    "vec4 texture2D(samplerExternalOES sampler, vec2 coord)"
    "{"
    "   return $$texture(0, sampler, coord.st, 0.0);"
    "}"

    "vec4 texture2DProj(samplerExternalOES sampler, vec3 coord, float bias)"
    "{"
    "   return $$texture(0, sampler, coord.st /= coord[2], bias);"
    "}"

    "vec4 texture2DProj(samplerExternalOES sampler, vec3 coord)"
    "{"
    "   return $$texture(0, sampler, coord.st /= coord[2], 0.0);"
    "}"

    "vec4 texture2DProj(samplerExternalOES sampler, vec4 coord, float bias)"
    "{"
    "   return $$texture(0, sampler, coord.st /= coord[3], bias);"
    "}"

    "vec4 texture2DProj(samplerExternalOES sampler, vec4 coord)"
    "{"
    "   return $$texture(0, sampler, coord.st /= coord[3], 0.0);"
    "}"
    "\n"},
    {"GL_BRCM_texture_1D", GLSL_DISABLED,

    "vec4  texture(sampler1DBRCM sampler, float coord) { return $$texture(0, sampler, coord, 0.0); }"
    "uvec4 texture(usampler1DBRCM sampler, float coord) { return $$texture(0, sampler, coord, 0.0); }"
    "vec4  texture(sampler1DArrayBRCM sampler, vec2 coord) { return $$texture(0, sampler, coord, 0.0); }"
    "uvec4 texture(usampler1DArrayBRCM sampler, vec2 coord) { return $$texture(0, sampler, coord, 0.0); }"

    "vec4 textureLod(sampler1DBRCM sampler, float coord, float lod) { return $$texture(64, sampler, coord, lod); }"
     /* TODO more of the texture*() family */
    "\n"},

    {"GL_EXT_shader_texture_lod", GLSL_DISABLED,
     "vec4 texture2DLodEXT(sampler2D sampler, vec2 coord, float lod) { return $$texture(64, sampler, coord, lod); }"
     "vec4 texture2DProjLodEXT(sampler2D sampler, vec3 coord, float lod) { return $$texture(64, sampler, coord.st / coord.p, lod); }"
     "vec4 texture2DProjLodEXT(sampler2D sampler, vec4 coord, float lod) { return $$texture(64, sampler, coord.st / coord.q, lod); }"
     "vec4 textureCubeLodEXT(samplerCube sampler, vec3 coord, float lod) { return $$texture(64, sampler, __brcm_cube(coord), lod); }"
     "\n"},

    {"GL_OES_standard_derivatives", GLSL_DISABLED,
     "float dFdx(float p) { return $$fdx(p); }"
     "vec2  dFdx(vec2  p) { return vec2( dFdx(p.x), dFdx(p.y) ); }"
     "vec3  dFdx(vec3  p) { return vec3( dFdx(p.x), dFdx(p.y), dFdx(p.z) ); }"
     "vec4  dFdx(vec4  p) { return vec4( dFdx(p.x), dFdx(p.y), dFdx(p.z), dFdx(p.w) ); }"
     "float dFdy(float p) { return $$fdy(p); }"
     "vec2  dFdy(vec2  p) { return vec2( dFdy(p.x), dFdy(p.y) ); }"
     "vec3  dFdy(vec3  p) { return vec3( dFdy(p.x), dFdy(p.y), dFdy(p.z) ); }"
     "vec4  dFdy(vec4  p) { return vec4( dFdy(p.x), dFdy(p.y), dFdy(p.z), dFdy(p.w) ); }"
     "float fwidth(float p) { return abs(dFdx(p)) + abs(dFdy(p)); }"
     "vec2  fwidth(vec2  p) { return abs(dFdx(p)) + abs(dFdy(p)); }"
     "vec3  fwidth(vec3  p) { return abs(dFdx(p)) + abs(dFdy(p)); }"
     "vec4  fwidth(vec4  p) { return abs(dFdx(p)) + abs(dFdy(p)); }"
     "\n"}
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

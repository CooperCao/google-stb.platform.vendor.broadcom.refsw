// GL_OES_EGL_image_external

vec4 texture2D    (samplerExternalOES sampler, vec2 coord, float bias) { return $$texture(0, sampler, coord.st, bias); }
vec4 texture2D    (samplerExternalOES sampler, vec2 coord)             { return $$texture(0, sampler, coord.st, 0.0); }
vec4 texture2DProj(samplerExternalOES sampler, vec3 coord, float bias) { return $$texture(0, sampler, coord.st /= coord[2], bias); }
vec4 texture2DProj(samplerExternalOES sampler, vec3 coord)             { return $$texture(0, sampler, coord.st /= coord[2], 0.0); }
vec4 texture2DProj(samplerExternalOES sampler, vec4 coord, float bias) { return $$texture(0, sampler, coord.st /= coord[3], bias); }
vec4 texture2DProj(samplerExternalOES sampler, vec4 coord)             { return $$texture(0, sampler, coord.st /= coord[3], 0.0); }


// GL_BRCM_sampler_fetch

vec4 texelFetchBRCM      (sampler2D sampler, ivec2 coord, int lod)               { return $$texture(32, sampler, coord, lod); }
vec4 texelFetchOffsetBRCM(sampler2D sampler, ivec2 coord, int lod, ivec2 offset) { return $$texture(32, sampler, coord, lod, offset); }


// GL_EXT_shader_texture_lod

vec4 texture2DLodEXT    (sampler2D sampler,   vec2 coord, float lod) { return $$texture(8, sampler, coord, lod); }
vec4 texture2DProjLodEXT(sampler2D sampler,   vec3 coord, float lod) { return $$texture(8, sampler, coord.st / coord.p, lod); }
vec4 texture2DProjLodEXT(sampler2D sampler,   vec4 coord, float lod) { return $$texture(8, sampler, coord.st / coord.q, lod); }
vec4 textureCubeLodEXT  (samplerCube sampler, vec3 coord, float lod) { return $$texture(8, sampler, __brcm_cube(coord), lod); }

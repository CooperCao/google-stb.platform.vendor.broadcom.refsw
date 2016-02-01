vec4 textureGather(sampler2D sampler, vec2 coord) {
   return $$texture(16, sampler, coord, 0.0).bagr;
}

vec4 textureGather(sampler2D sampler, vec2 coord, int comp) {
   return $$texture(16, sampler, coord, 0.0, comp).bagr;
}

ivec4 textureGather(isampler2D sampler, vec2 coord) {
   return $$texture(16, sampler, coord, 0.0).bagr;
}

ivec4 textureGather(isampler2D sampler, vec2 coord, int comp) {
   return $$texture(16, sampler, coord, 0.0, comp).bagr;
}

uvec4 textureGather(usampler2D sampler, vec2 coord) {
   return $$texture(16, sampler, coord, 0.0).bagr;
}

uvec4 textureGather(usampler2D sampler, vec2 coord, int comp) {
   return $$texture(16, sampler, coord, 0.0, comp).bagr;
}

vec4 textureGather(sampler2DShadow sampler, vec2 coord, float refZ) {
   return $$texture(16, sampler, vec3(coord, refZ), 0.0).bagr;
}

vec4 textureGather(samplerCube sampler, vec3 coord) {
   return $$texture(16, sampler, __brcm_cube(coord), 0.0).bagr;
}

vec4 textureGather(samplerCube sampler, vec3 coord, int comp) {
   return $$texture(16, sampler, __brcm_cube(coord), 0.0, comp).bagr;
}

ivec4 textureGather(isamplerCube sampler, vec3 coord) {
   return $$texture(16, sampler, __brcm_cube(coord), 0.0).bagr;
}

ivec4 textureGather(isamplerCube sampler, vec3 coord, int comp) {
   return $$texture(16, sampler, __brcm_cube(coord), 0.0, comp).bagr;
}

uvec4 textureGather(usamplerCube sampler, vec3 coord) {
   return $$texture(16, sampler, __brcm_cube(coord), 0.0).bagr;
}

uvec4 textureGather(usamplerCube sampler, vec3 coord, int comp) {
   return $$texture(16, sampler, __brcm_cube(coord), 0.0, comp).bagr;
}

vec4 textureGather(samplerCubeShadow sampler, vec3 coord, float refZ) {
   return $$texture(16, sampler, vec4(__brcm_cube(coord), refZ), 0.0).bagr;
}

vec4 textureGather(sampler2DArray sampler, vec3 coord) {
   return $$texture(16, sampler, coord, 0.0).bagr;
}

vec4 textureGather(sampler2DArray sampler, vec3 coord, int comp) {
   return $$texture(16, sampler, coord, 0.0, comp).bagr;
}

ivec4 textureGather(isampler2DArray sampler, vec3 coord) {
   return $$texture(16, sampler, coord, 0.0).bagr;
}

ivec4 textureGather(isampler2DArray sampler, vec3 coord, int comp) {
   return $$texture(16, sampler, coord, 0.0, comp).bagr;
}

uvec4 textureGather(usampler2DArray sampler, vec3 coord) {
   return $$texture(16, sampler, coord, 0.0).bagr;
}

uvec4 textureGather(usampler2DArray sampler, vec3 coord, int comp) {
   return $$texture(16, sampler, coord, 0.0, comp).bagr;
}

vec4 textureGather(sampler2DArrayShadow sampler, vec3 coord, float refZ) {
   return $$texture(16, sampler, vec4(coord, refZ), 0.0).bagr;
}

vec4 textureGatherOffset(sampler2D sampler, vec2 coord, ivec2 offset) {
   return $$texture(16, sampler, coord, 0.0, 0, offset).bagr;
}

vec4 textureGatherOffset(sampler2D sampler, vec2 coord, ivec2 offset, int comp) {
   return $$texture(16, sampler, coord, 0.0, comp, offset).bagr;
}

ivec4 textureGatherOffset(isampler2D sampler, vec2 coord, ivec2 offset) {
   return $$texture(16, sampler, coord, 0.0, 0, offset).bagr;
}

ivec4 textureGatherOffset(isampler2D sampler, vec2 coord, ivec2 offset, int comp) {
   return $$texture(16, sampler, coord, 0.0, comp, offset).bagr;
}

uvec4 textureGatherOffset(usampler2D sampler, vec2 coord, ivec2 offset) {
   return $$texture(16, sampler, coord, 0.0, 0, offset).bagr;
}

uvec4 textureGatherOffset(usampler2D sampler, vec2 coord, ivec2 offset, int comp) {
   return $$texture(16, sampler, coord, 0.0, comp, offset).bagr;
}

vec4 textureGatherOffset(sampler2DShadow sampler, vec2 coord, float refZ, ivec2 offset) {
   return $$texture(16, sampler, vec3(coord,refZ), 0.0, 0, offset).bagr;
}

vec4 textureGatherOffset(sampler2DArray sampler, vec3 coord, ivec2 offset) {
   return $$texture(16, sampler, coord, 0.0, 0, offset).bagr;
}

vec4 textureGatherOffset(sampler2DArray sampler, vec3 coord, ivec2 offset, int comp) {
   return $$texture(16, sampler, coord, 0.0, comp, offset).bagr;
}

ivec4 textureGatherOffset(isampler2DArray sampler, vec3 coord, ivec2 offset) {
   return $$texture(16, sampler, coord, 0.0, 0, offset).bagr;
}

ivec4 textureGatherOffset(isampler2DArray sampler, vec3 coord, ivec2 offset, int comp) {
   return $$texture(16, sampler, coord, 0.0, comp, offset).bagr;
}

uvec4 textureGatherOffset(usampler2DArray sampler, vec3 coord, ivec2 offset) {
   return $$texture(16, sampler, coord, 0.0, 0, offset).bagr;
}

uvec4 textureGatherOffset(usampler2DArray sampler, vec3 coord, ivec2 offset, int comp) {
   return $$texture(16, sampler, coord, 0.0, comp, offset).bagr;
}

vec4 textureGatherOffset(sampler2DArrayShadow sampler, vec3 coord, float refZ, ivec2 offset) {
   return $$texture(16, sampler, vec4(coord, refZ), 0.0, 0, offset).bagr;
}

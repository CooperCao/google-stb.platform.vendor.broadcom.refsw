vec4 textureGather(sampler2D sampler, vec2 coord) {
   return $$texture(4, sampler, coord, 0.0).bagr;
}

vec4 textureGather(sampler2D sampler, vec2 coord, int comp) {
   return $$texture(4, sampler, coord, 0.0, comp).bagr;
}

ivec4 textureGather(isampler2D sampler, vec2 coord) {
   return $$texture(4, sampler, coord, 0.0).bagr;
}

ivec4 textureGather(isampler2D sampler, vec2 coord, int comp) {
   return $$texture(4, sampler, coord, 0.0, comp).bagr;
}

uvec4 textureGather(usampler2D sampler, vec2 coord) {
   return $$texture(4, sampler, coord, 0.0).bagr;
}

uvec4 textureGather(usampler2D sampler, vec2 coord, int comp) {
   return $$texture(4, sampler, coord, 0.0, comp).bagr;
}

vec4 textureGather(sampler2DShadow sampler, vec2 coord, float refZ) {
   return $$texture(4, sampler, coord, 0.0, refZ).bagr;
}

vec4 textureGather(samplerCube sampler, vec3 coord) {
   return $$texture(4, sampler, __brcm_cube(coord), 0.0).bagr;
}

vec4 textureGather(samplerCube sampler, vec3 coord, int comp) {
   return $$texture(4, sampler, __brcm_cube(coord), 0.0, comp).bagr;
}

ivec4 textureGather(isamplerCube sampler, vec3 coord) {
   return $$texture(4, sampler, __brcm_cube(coord), 0.0).bagr;
}

ivec4 textureGather(isamplerCube sampler, vec3 coord, int comp) {
   return $$texture(4, sampler, __brcm_cube(coord), 0.0, comp).bagr;
}

uvec4 textureGather(usamplerCube sampler, vec3 coord) {
   return $$texture(4, sampler, __brcm_cube(coord), 0.0).bagr;
}

uvec4 textureGather(usamplerCube sampler, vec3 coord, int comp) {
   return $$texture(4, sampler, __brcm_cube(coord), 0.0, comp).bagr;
}

vec4 textureGather(samplerCubeShadow sampler, vec3 coord, float refZ) {
   return $$texture(4, sampler, __brcm_cube(coord), 0.0, refZ).bagr;
}

vec4 textureGather(sampler2DArray sampler, vec3 coord) {
   return $$texture(4, sampler, coord, 0.0).bagr;
}

vec4 textureGather(sampler2DArray sampler, vec3 coord, int comp) {
   return $$texture(4, sampler, coord, 0.0, comp).bagr;
}

ivec4 textureGather(isampler2DArray sampler, vec3 coord) {
   return $$texture(4, sampler, coord, 0.0).bagr;
}

ivec4 textureGather(isampler2DArray sampler, vec3 coord, int comp) {
   return $$texture(4, sampler, coord, 0.0, comp).bagr;
}

uvec4 textureGather(usampler2DArray sampler, vec3 coord) {
   return $$texture(4, sampler, coord, 0.0).bagr;
}

uvec4 textureGather(usampler2DArray sampler, vec3 coord, int comp) {
   return $$texture(4, sampler, coord, 0.0, comp).bagr;
}

vec4 textureGather(sampler2DArrayShadow sampler, vec3 coord, float refZ) {
   return $$texture(4, sampler, coord, 0.0, refZ).bagr;
}

vec4 textureGather(samplerCubeArray sampler, vec4 coord) {
   return $$texture(4, sampler, coord, 0.0).bagr;
}

vec4 textureGather(samplerCubeArray sampler, vec4 coord, int comp) {
   return $$texture(4, sampler, coord, 0.0, comp).bagr;
}

ivec4 textureGather(isamplerCubeArray sampler, vec4 coord) {
   return $$texture(4, sampler, coord, 0.0).bagr;
}

ivec4 textureGather(isamplerCubeArray sampler, vec4 coord, int comp) {
   return $$texture(4, sampler, coord, 0.0, comp).bagr;
}

uvec4 textureGather(usamplerCubeArray sampler, vec4 coord) {
   return $$texture(4, sampler, coord, 0.0).bagr;
}

uvec4 textureGather(usamplerCubeArray sampler, vec4 coord, int comp) {
   return $$texture(4, sampler, coord, 0.0, comp).bagr;
}

vec4 textureGather(samplerCubeArrayShadow sampler, vec4 coord, float refZ) {
   return $$texture(4, sampler, coord, 0.0, refZ).bagr;
}

vec4 textureGatherOffset(sampler2D sampler, vec2 coord, ivec2 offset) {
   return $$texture(4, sampler, coord, 0.0, 0, offset).bagr;
}

vec4 textureGatherOffset(sampler2D sampler, vec2 coord, ivec2 offset, int comp) {
   return $$texture(4, sampler, coord, 0.0, comp, offset).bagr;
}

ivec4 textureGatherOffset(isampler2D sampler, vec2 coord, ivec2 offset) {
   return $$texture(4, sampler, coord, 0.0, 0, offset).bagr;
}

ivec4 textureGatherOffset(isampler2D sampler, vec2 coord, ivec2 offset, int comp) {
   return $$texture(4, sampler, coord, 0.0, comp, offset).bagr;
}

uvec4 textureGatherOffset(usampler2D sampler, vec2 coord, ivec2 offset) {
   return $$texture(4, sampler, coord, 0.0, 0, offset).bagr;
}

uvec4 textureGatherOffset(usampler2D sampler, vec2 coord, ivec2 offset, int comp) {
   return $$texture(4, sampler, coord, 0.0, comp, offset).bagr;
}

vec4 textureGatherOffset(sampler2DShadow sampler, vec2 coord, float refZ, ivec2 offset) {
   return $$texture(4, sampler, coord, 0.0, refZ, 0, offset).bagr;
}

vec4 textureGatherOffset(sampler2DArray sampler, vec3 coord, ivec2 offset) {
   return $$texture(4, sampler, coord, 0.0, 0, offset).bagr;
}

vec4 textureGatherOffset(sampler2DArray sampler, vec3 coord, ivec2 offset, int comp) {
   return $$texture(4, sampler, coord, 0.0, comp, offset).bagr;
}

ivec4 textureGatherOffset(isampler2DArray sampler, vec3 coord, ivec2 offset) {
   return $$texture(4, sampler, coord, 0.0, 0, offset).bagr;
}

ivec4 textureGatherOffset(isampler2DArray sampler, vec3 coord, ivec2 offset, int comp) {
   return $$texture(4, sampler, coord, 0.0, comp, offset).bagr;
}

uvec4 textureGatherOffset(usampler2DArray sampler, vec3 coord, ivec2 offset) {
   return $$texture(4, sampler, coord, 0.0, 0, offset).bagr;
}

uvec4 textureGatherOffset(usampler2DArray sampler, vec3 coord, ivec2 offset, int comp) {
   return $$texture(4, sampler, coord, 0.0, comp, offset).bagr;
}

vec4 textureGatherOffset(sampler2DArrayShadow sampler, vec3 coord, float refZ, ivec2 offset) {
   return $$texture(4, sampler, coord, 0.0, refZ, 0, offset).bagr;
}

vec4 textureGatherOffsets(sampler2D sampler, vec2 coord, ivec2 offsets[4]) {
   return $$texture(20, sampler, coord, 0.0, 0, offsets);
}

vec4 textureGatherOffsets(sampler2D sampler, vec2 coord, ivec2 offsets[4], int comp) {
   return $$texture(20, sampler, coord, 0.0, comp, offsets);
}

ivec4 textureGatherOffsets(isampler2D sampler, vec2 coord, ivec2 offsets[4]) {
   return $$texture(20, sampler, coord, 0.0, 0, offsets);
}

ivec4 textureGatherOffsets(isampler2D sampler, vec2 coord, ivec2 offsets[4], int comp) {
   return $$texture(20, sampler, coord, 0.0, comp, offsets);
}

uvec4 textureGatherOffsets(usampler2D sampler, vec2 coord, ivec2 offsets[4]) {
   return $$texture(20, sampler, coord, 0.0, 0, offsets);
}

uvec4 textureGatherOffsets(usampler2D sampler, vec2 coord, ivec2 offsets[4], int comp) {
   return $$texture(20, sampler, coord, 0.0, comp, offsets);
}

vec4 textureGatherOffsets(sampler2DShadow sampler, vec2 coord, float refZ, ivec2 offsets[4]) {
   return $$texture(20, sampler, coord, 0.0, refZ, 0, offsets);
}

vec4 textureGatherOffsets(sampler2DArray sampler, vec3 coord, ivec2 offsets[4]) {
   return $$texture(20, sampler, coord, 0.0, 0, offsets);
}

vec4 textureGatherOffsets(sampler2DArray sampler, vec3 coord, ivec2 offsets[4], int comp) {
   return $$texture(20, sampler, coord, 0.0, comp, offsets);
}

ivec4 textureGatherOffsets(isampler2DArray sampler, vec3 coord, ivec2 offsets[4]) {
   return $$texture(20, sampler, coord, 0.0, 0, offsets);
}

ivec4 textureGatherOffsets(isampler2DArray sampler, vec3 coord, ivec2 offsets[4], int comp) {
   return $$texture(20, sampler, coord, 0.0, comp, offsets);
}

uvec4 textureGatherOffsets(usampler2DArray sampler, vec3 coord, ivec2 offsets[4]) {
   return $$texture(20, sampler, coord, 0.0, 0, offsets);
}

uvec4 textureGatherOffsets(usampler2DArray sampler, vec3 coord, ivec2 offsets[4], int comp) {
   return $$texture(20, sampler, coord, 0.0, comp, offsets);
}

vec4 textureGatherOffsets(sampler2DArrayShadow sampler, vec3 coord, float refZ, ivec2 offsets[4]) {
   return $$texture(20, sampler, coord, 0.0, refZ, 0, offsets);
}

vec4 textureGatherLodBRCM(sampler2D sampler, vec2 coord, float lod) {
   return $$texture(4, sampler, coord, lod).bagr;
}

vec4 textureGatherLodBRCM(sampler2D sampler, vec2 coord, float lod, int comp) {
   return $$texture(4, sampler, coord, lod, comp).bagr;
}

ivec4 textureGatherLodBRCM(isampler2D sampler, vec2 coord, float lod) {
   return $$texture(4, sampler, coord, lod).bagr;
}

ivec4 textureGatherLodBRCM(isampler2D sampler, vec2 coord, float lod, int comp) {
   return $$texture(4, sampler, coord, lod, comp).bagr;
}

uvec4 textureGatherLodBRCM(usampler2D sampler, vec2 coord, float lod) {
   return $$texture(4, sampler, coord, lod).bagr;
}

uvec4 textureGatherLodBRCM(usampler2D sampler, vec2 coord, float lod, int comp) {
   return $$texture(4, sampler, coord, lod, comp).bagr;
}

vec4 textureGatherLodBRCM(sampler2DShadow sampler, vec2 coord, float refZ, float lod) {
   return $$texture(4, sampler, coord, lod, refZ).bagr;
}

vec4 textureGatherLodBRCM(samplerCube sampler, vec3 coord, float lod) {
   return $$texture(4, sampler, __brcm_cube(coord), lod).bagr;
}

vec4 textureGatherLodBRCM(samplerCube sampler, vec3 coord, float lod, int comp) {
   return $$texture(4, sampler, __brcm_cube(coord), lod, comp).bagr;
}

ivec4 textureGatherLodBRCM(isamplerCube sampler, vec3 coord, float lod) {
   return $$texture(4, sampler, __brcm_cube(coord), lod).bagr;
}

ivec4 textureGatherLodBRCM(isamplerCube sampler, vec3 coord, float lod, int comp) {
   return $$texture(4, sampler, __brcm_cube(coord), lod, comp).bagr;
}

uvec4 textureGatherLodBRCM(usamplerCube sampler, vec3 coord, float lod) {
   return $$texture(4, sampler, __brcm_cube(coord), lod).bagr;
}

uvec4 textureGatherLodBRCM(usamplerCube sampler, vec3 coord, float lod, int comp) {
   return $$texture(4, sampler, __brcm_cube(coord), lod, comp).bagr;
}

vec4 textureGatherLodBRCM(samplerCubeShadow sampler, vec3 coord, float refZ, float lod) {
   return $$texture(4, sampler, __brcm_cube(coord), lod, refZ).bagr;
}

vec4 textureGatherLodBRCM(sampler2DArray sampler, vec3 coord, float lod) {
   return $$texture(4, sampler, coord, lod).bagr;
}

vec4 textureGatherLodBRCM(sampler2DArray sampler, vec3 coord, float lod, int comp) {
   return $$texture(4, sampler, coord, lod, comp).bagr;
}

ivec4 textureGatherLodBRCM(isampler2DArray sampler, vec3 coord, float lod) {
   return $$texture(4, sampler, coord, lod).bagr;
}

ivec4 textureGatherLodBRCM(isampler2DArray sampler, vec3 coord, float lod, int comp) {
   return $$texture(4, sampler, coord, lod, comp).bagr;
}

uvec4 textureGatherLodBRCM(usampler2DArray sampler, vec3 coord, float lod) {
   return $$texture(4, sampler, coord, lod).bagr;
}

uvec4 textureGatherLodBRCM(usampler2DArray sampler, vec3 coord, float lod, int comp) {
   return $$texture(4, sampler, coord, lod, comp).bagr;
}

vec4 textureGatherLodBRCM(sampler2DArrayShadow sampler, vec3 coord, float refZ, float lod) {
   return $$texture(4, sampler, coord, lod, refZ).bagr;
}

vec4 textureGatherLodBRCM(samplerCubeArray sampler, vec4 coord, float lod) {
   return $$texture(4, sampler, coord, lod).bagr;
}

vec4 textureGatherLodBRCM(samplerCubeArray sampler, vec4 coord, float lod, int comp) {
   return $$texture(4, sampler, coord, lod, comp).bagr;
}

ivec4 textureGatherLodBRCM(isamplerCubeArray sampler, vec4 coord, float lod) {
   return $$texture(4, sampler, coord, lod).bagr;
}

ivec4 textureGatherLodBRCM(isamplerCubeArray sampler, vec4 coord, float lod, int comp) {
   return $$texture(4, sampler, coord, lod, comp).bagr;
}

uvec4 textureGatherLodBRCM(usamplerCubeArray sampler, vec4 coord, float lod) {
   return $$texture(4, sampler, coord, lod).bagr;
}

uvec4 textureGatherLodBRCM(usamplerCubeArray sampler, vec4 coord, float lod, int comp) {
   return $$texture(4, sampler, coord, lod, comp).bagr;
}

vec4 textureGatherLodBRCM(samplerCubeArrayShadow sampler, vec4 coord, float refZ, float lod) {
   return $$texture(4, sampler, coord, lod, refZ).bagr;
}

vec4 textureGatherLodOffsetBRCM(sampler2D sampler, vec2 coord, float lod, ivec2 offset) {
   return $$texture(4, sampler, coord, lod, 0, offset).bagr;
}

vec4 textureGatherLodOffsetBRCM(sampler2D sampler, vec2 coord, float lod, ivec2 offset, int comp) {
   return $$texture(4, sampler, coord, lod, comp, offset).bagr;
}

ivec4 textureGatherLodOffsetBRCM(isampler2D sampler, vec2 coord, float lod, ivec2 offset) {
   return $$texture(4, sampler, coord, lod, 0, offset).bagr;
}

ivec4 textureGatherLodOffsetBRCM(isampler2D sampler, vec2 coord, float lod, ivec2 offset, int comp) {
   return $$texture(4, sampler, coord, lod, comp, offset).bagr;
}

uvec4 textureGatherLodOffsetBRCM(usampler2D sampler, vec2 coord, float lod, ivec2 offset) {
   return $$texture(4, sampler, coord, lod, 0, offset).bagr;
}

uvec4 textureGatherLodOffsetBRCM(usampler2D sampler, vec2 coord, float lod, ivec2 offset, int comp) {
   return $$texture(4, sampler, coord, lod, comp, offset).bagr;
}

vec4 textureGatherLodOffsetBRCM(sampler2DShadow sampler, vec2 coord, float refZ, float lod, ivec2 offset) {
   return $$texture(4, sampler, coord, lod, refZ, 0, offset).bagr;
}

vec4 textureGatherLodOffsetBRCM(sampler2DArray sampler, vec3 coord, float lod, ivec2 offset) {
   return $$texture(4, sampler, coord, lod, 0, offset).bagr;
}

vec4 textureGatherLodOffsetBRCM(sampler2DArray sampler, vec3 coord, float lod, ivec2 offset, int comp) {
   return $$texture(4, sampler, coord, lod, comp, offset).bagr;
}

ivec4 textureGatherLodOffsetBRCM(isampler2DArray sampler, vec3 coord, float lod, ivec2 offset) {
   return $$texture(4, sampler, coord, lod, 0, offset).bagr;
}

ivec4 textureGatherLodOffsetBRCM(isampler2DArray sampler, vec3 coord, float lod, ivec2 offset, int comp) {
   return $$texture(4, sampler, coord, lod, comp, offset).bagr;
}

uvec4 textureGatherLodOffsetBRCM(usampler2DArray sampler, vec3 coord, float lod, ivec2 offset) {
   return $$texture(4, sampler, coord, lod, 0, offset).bagr;
}

uvec4 textureGatherLodOffsetBRCM(usampler2DArray sampler, vec3 coord, float lod, ivec2 offset, int comp) {
   return $$texture(4, sampler, coord, lod, comp, offset).bagr;
}

vec4 textureGatherLodOffsetBRCM(sampler2DArrayShadow sampler, vec3 coord, float refZ, float lod, ivec2 offset) {
   return $$texture(4, sampler, coord, lod, refZ, 0, offset).bagr;
}

vec4 textureGatherLodOffsetsBRCM(sampler2D sampler, vec2 coord, float lod, ivec2 offsets[4]) {
   return $$texture(20, sampler, coord, lod, 0, offsets);
}

vec4 textureGatherLodOffsetsBRCM(sampler2D sampler, vec2 coord, float lod, ivec2 offsets[4], int comp) {
   return $$texture(20, sampler, coord, lod, comp, offsets);
}

ivec4 textureGatherLodOffsetsBRCM(isampler2D sampler, vec2 coord, float lod, ivec2 offsets[4]) {
   return $$texture(20, sampler, coord, lod, 0, offsets);
}

ivec4 textureGatherLodOffsetsBRCM(isampler2D sampler, vec2 coord, float lod, ivec2 offsets[4], int comp) {
   return $$texture(20, sampler, coord, lod, comp, offsets);
}

uvec4 textureGatherLodOffsetsBRCM(usampler2D sampler, vec2 coord, float lod, ivec2 offsets[4]) {
   return $$texture(20, sampler, coord, lod, 0, offsets);
}

uvec4 textureGatherLodOffsetsBRCM(usampler2D sampler, vec2 coord, float lod, ivec2 offsets[4], int comp) {
   return $$texture(20, sampler, coord, lod, comp, offsets);
}

vec4 textureGatherLodOffsetsBRCM(sampler2DShadow sampler, vec2 coord, float refZ, float lod, ivec2 offsets[4]) {
   return $$texture(20, sampler, coord, lod, refZ, 0, offsets);
}

vec4 textureGatherLodOffsetsBRCM(sampler2DArray sampler, vec3 coord, float lod, ivec2 offsets[4]) {
   return $$texture(20, sampler, coord, lod, 0, offsets);
}

vec4 textureGatherLodOffsetsBRCM(sampler2DArray sampler, vec3 coord, float lod, ivec2 offsets[4], int comp) {
   return $$texture(20, sampler, coord, lod, comp, offsets);
}

ivec4 textureGatherLodOffsetsBRCM(isampler2DArray sampler, vec3 coord, float lod, ivec2 offsets[4]) {
   return $$texture(20, sampler, coord, lod, 0, offsets);
}

ivec4 textureGatherLodOffsetsBRCM(isampler2DArray sampler, vec3 coord, float lod, ivec2 offsets[4], int comp) {
   return $$texture(20, sampler, coord, lod, comp, offsets);
}

uvec4 textureGatherLodOffsetsBRCM(usampler2DArray sampler, vec3 coord, float lod, ivec2 offsets[4]) {
   return $$texture(20, sampler, coord, lod, 0, offsets);
}

uvec4 textureGatherLodOffsetsBRCM(usampler2DArray sampler, vec3 coord, float lod, ivec2 offsets[4], int comp) {
   return $$texture(20, sampler, coord, lod, comp, offsets);
}

vec4 textureGatherLodOffsetsBRCM(sampler2DArrayShadow sampler, vec3 coord, float refZ, float lod, ivec2 offsets[4]) {
   return $$texture(20, sampler, coord, lod, refZ, 0, offsets);
}

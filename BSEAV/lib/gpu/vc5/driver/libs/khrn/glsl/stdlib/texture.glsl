// New texture lookup functions

float __brcm_proj(vec2 coord) {
   highp float oocoord = 1.0/coord[1];
   return oocoord * coord.s;
}
vec2 __brcm_proj(vec3 coord) {
   highp float oocoord = 1.0 / coord[2];
   return oocoord * coord.st;
}
vec3 __brcm_proj(vec4 coord) {
   highp float oocoord = 1.0 / coord[3];
   return oocoord * coord.stp;
}
// Special cases where certain component values are ignored
float __brcm_proj1D(vec4 coord) {
   highp float oocoord = 1.0/coord[3];
   return oocoord * coord.s;
}
vec2 __brcm_proj2D(vec4 coord) {
   highp float oocoord = 1.0 / coord[3];
   return oocoord * coord.st;
}
float __brcm_max_in_vec3(vec3 v) {
   return max(abs(v.x), max(abs(v.y), abs(v.z)));
}
vec3 __brcm_cube(vec3 coord) {
   highp float maxabs = __brcm_max_in_vec3(coord);
   return coord / maxabs;
}
// Like the vec3 version but pass the 3-component through. Used for shadow and array */
vec4 __brcm_cube(vec4 coord) {
   highp float maxabs = __brcm_max_in_vec3(coord.stp);
   return vec4(coord.stp / maxabs, coord[3]);
}
float __brcm_lod_from_grads(ivec2 i_tex_size, vec2 dPdx, vec2 dPdy) {
   highp vec2 tex_size = vec2(i_tex_size);
   dPdx = dPdx * tex_size;
   dPdy = dPdy * tex_size;
   highp float max_deriv = max(max(abs(dPdx.x), abs(dPdx.y)), max(abs(dPdy.x), abs(dPdy.y)));
   return log2(max_deriv);
}
float __brcm_lod_from_grads(ivec3 i_tex_size, vec3 dPdx, vec3 dPdy) {
   highp vec3 tex_size = vec3(i_tex_size);
   dPdx = dPdx * tex_size;
   dPdy = dPdy * tex_size;
   highp float max_deriv = max(__brcm_max_in_vec3(dPdx), __brcm_max_in_vec3(dPdy));
   return log2(max_deriv);
}
float __brcm_cube_project(float s, float m, float dsdv, float dmdv) {
   return dsdv / (2.0 * m) - s * dmdv / (m * m);
}
float __brcm_lod_from_cube_grads(ivec2 i_tex_size, vec3 P, vec3 dPdx, vec3 dPdy) {
   highp vec2 tex_size = vec2(i_tex_size);
   highp float max_P_component = __brcm_max_in_vec3(P);
   highp vec3 abs_P = abs(P);
   highp vec2 dPdx_proj, dPdy_proj;

   /* TODO: this can probably be done more efficiently */
   highp vec4 args0, args1, args2, args3;
   if(       abs_P.x == max_P_component) {
      args0 = vec4(P.z, P.x, dPdx.z, dPdx.x);
      args1 = vec4(P.z, P.x, dPdy.z, dPdy.x);
      args2 = vec4(P.y, P.x, dPdx.y, dPdx.x);
      args3 = vec4(P.y, P.x, dPdy.y, dPdy.x);
   } else if(abs_P.y == max_P_component) {
      args0 = vec4(P.x, P.y, dPdx.x, dPdx.y);
      args1 = vec4(P.x, P.y, dPdy.x, dPdy.y);
      args2 = vec4(P.z, P.y, dPdx.z, dPdx.y);
      args3 = vec4(P.z, P.y, dPdy.z, dPdy.y);
   } else {
      args0 = vec4(P.x, P.z, dPdx.x, dPdx.z);
      args1 = vec4(P.x, P.z, dPdy.x, dPdy.z);
      args2 = vec4(P.y, P.z, dPdx.y, dPdx.z);
      args3 = vec4(P.y, P.z, dPdy.y, dPdy.z);
   }
   dPdx_proj.s = __brcm_cube_project(args0.x, args0.y, args0.z, args0.w);
   dPdy_proj.s = __brcm_cube_project(args1.x, args1.y, args1.z, args1.w);
   dPdx_proj.t = __brcm_cube_project(args2.x, args2.y, args2.z, args2.w);
   dPdy_proj.t = __brcm_cube_project(args3.x, args3.y, args3.z, args3.w);

   dPdx_proj = dPdx_proj * tex_size;
   dPdy_proj = dPdy_proj * tex_size;
   highp float max_deriv = max(max(abs(dPdx_proj.x),abs(dPdx_proj.y)), max(abs(dPdy_proj.x), abs(dPdy_proj.y)));
   return log2(max_deriv);
}

highp ivec2 textureSize( sampler2D sampler, int lod) { return max($$textureSize(sampler) >> ivec2(lod), ivec2(1)); }
highp ivec2 textureSize(isampler2D sampler, int lod) { return max($$textureSize(sampler) >> ivec2(lod), ivec2(1)); }
highp ivec2 textureSize(usampler2D sampler, int lod) { return max($$textureSize(sampler) >> ivec2(lod), ivec2(1)); }
highp ivec3 textureSize( sampler3D sampler, int lod) { return max($$textureSize(sampler) >> ivec3(lod), ivec3(1)); }
highp ivec3 textureSize(isampler3D sampler, int lod) { return max($$textureSize(sampler) >> ivec3(lod), ivec3(1)); }
highp ivec3 textureSize(usampler3D sampler, int lod) { return max($$textureSize(sampler) >> ivec3(lod), ivec3(1)); }
highp ivec2 textureSize( samplerCube sampler, int lod) { return max($$textureSize(sampler) >> ivec2(lod), ivec2(1)); }
highp ivec2 textureSize(isamplerCube sampler, int lod) { return max($$textureSize(sampler) >> ivec2(lod), ivec2(1)); }
highp ivec2 textureSize(usamplerCube sampler, int lod) { return max($$textureSize(sampler) >> ivec2(lod), ivec2(1)); }
highp ivec2 textureSize( sampler2DMS sampler) { return $$textureSize(sampler); }
highp ivec2 textureSize(isampler2DMS sampler) { return $$textureSize(sampler); }
highp ivec2 textureSize(usampler2DMS sampler) { return $$textureSize(sampler); }
highp ivec3 textureSize( sampler2DMSArray sampler) { return $$textureSize(sampler); }
highp ivec3 textureSize(isampler2DMSArray sampler) { return $$textureSize(sampler); }
highp ivec3 textureSize(usampler2DMSArray sampler) { return $$textureSize(sampler); }
highp ivec3 textureSize( sampler2DArray   sampler, int lod) { return max($$textureSize(sampler) >> ivec3(lod, lod, 0), ivec3(1)); }
highp ivec3 textureSize(isampler2DArray   sampler, int lod) { return max($$textureSize(sampler) >> ivec3(lod, lod, 0), ivec3(1)); }
highp ivec3 textureSize(usampler2DArray   sampler, int lod) { return max($$textureSize(sampler) >> ivec3(lod, lod, 0), ivec3(1)); }
highp ivec3 textureSize( samplerCubeArray sampler, int lod) { return max($$textureSize(sampler) >> ivec3(lod, lod, 0), ivec3(1)); }
highp ivec3 textureSize(isamplerCubeArray sampler, int lod) { return max($$textureSize(sampler) >> ivec3(lod, lod, 0), ivec3(1)); }
highp ivec3 textureSize(usamplerCubeArray sampler, int lod) { return max($$textureSize(sampler) >> ivec3(lod, lod, 0), ivec3(1)); }
highp ivec2 textureSize(sampler2DShadow        sampler, int lod) { return max($$textureSize(sampler) >> ivec2(lod), ivec2(1)); }
highp ivec2 textureSize(samplerCubeShadow      sampler, int lod) { return max($$textureSize(sampler) >> ivec2(lod), ivec2(1)); }
highp ivec3 textureSize(sampler2DArrayShadow   sampler, int lod) { return max($$textureSize(sampler) >> ivec3(lod, lod, 0), ivec3(1)); }
highp ivec3 textureSize(samplerCubeArrayShadow sampler, int lod) { return max($$textureSize(sampler) >> ivec3(lod, lod, 0), ivec3(1)); }
highp int   textureSize( samplerBuffer sampler) { return $$textureSize(sampler); }
highp int   textureSize(isamplerBuffer sampler) { return $$textureSize(sampler); }
highp int   textureSize(usamplerBuffer sampler) { return $$textureSize(sampler); }

// These are likely different from all other texture functions.
// TODO: Consider integrating them into the texture function script
// NB: The sample data is fetch from the locations written by the TLB. These are x-flipped
//     relative to the actual sampling positions. No one knows why.
 vec4 texelFetch ( sampler2DMS      sampler, ivec2 P, int s) { return $$texture(2, sampler, 2*P + ivec2(s & 1, s >> 1), 0); }
ivec4 texelFetch (isampler2DMS      sampler, ivec2 P, int s) { return $$texture(2, sampler, 2*P + ivec2(s & 1, s >> 1), 0); }
uvec4 texelFetch (usampler2DMS      sampler, ivec2 P, int s) { return $$texture(2, sampler, 2*P + ivec2(s & 1, s >> 1), 0); }
 vec4 texelFetch ( sampler2DMSArray sampler, ivec3 P, int s) { return $$texture(2, sampler, ivec3(2*P.xy + ivec2(s & 1, s >> 1), P.z), 0); }
ivec4 texelFetch (isampler2DMSArray sampler, ivec3 P, int s) { return $$texture(2, sampler, ivec3(2*P.xy + ivec2(s & 1, s >> 1), P.z), 0); }
uvec4 texelFetch (usampler2DMSArray sampler, ivec3 P, int s) { return $$texture(2, sampler, ivec3(2*P.xy + ivec2(s & 1, s >> 1), P.z), 0); }
 vec4 texelFetch ( samplerBuffer    sampler, int   P)        { return $$texture(2, sampler, P, 0); }
ivec4 texelFetch (isamplerBuffer    sampler, int   P)        { return $$texture(2, sampler, P, 0); }
uvec4 texelFetch (usamplerBuffer    sampler, int   P)        { return $$texture(2, sampler, P, 0); }

// Old texture lookup functions
vec4 texture2D(sampler2D sampler, vec2 coord, float bias) { return $$texture(0, sampler,coord,bias); }
vec4 texture2D(sampler2D sampler, vec2 coord) { return $$texture(0, sampler,coord, 0.0); }

vec4 texture2DProj(sampler2D sampler, vec3 coord, float bias) {
   return $$texture(0, sampler,__brcm_proj(coord),bias);
}
vec4 texture2DProj(sampler2D sampler, vec3 coord) {
   return $$texture(0, sampler,__brcm_proj(coord), 0.0);
}
vec4 texture2DProj(sampler2D sampler, vec4 coord, float bias) {
   return $$texture(0, sampler,__brcm_proj2D(coord),bias);
}
vec4 texture2DProj(sampler2D sampler, vec4 coord) {
   return $$texture(0, sampler,__brcm_proj2D(coord), 0.0);
}

vec4 texture2DLod(sampler2D sampler, vec2 coord, float lod) { return $$texture(8, sampler,coord,lod); }

vec4 texture2DProjLod(sampler2D sampler, vec3 coord, float lod) {
   return $$texture(8, sampler,__brcm_proj(coord),lod);
}
vec4 texture2DProjLod(sampler2D sampler, vec4 coord, float lod) {
   return $$texture(8, sampler,__brcm_proj2D(coord),lod);
}

vec4 textureCube(samplerCube sampler, vec3 coord, float bias) {
   return $$texture(0, sampler,__brcm_cube(coord),bias);
}
vec4 textureCube(samplerCube sampler, vec3 coord) {
   return $$texture(0, sampler,__brcm_cube(coord),0.0);
}

vec4 textureCubeLod(samplerCube sampler, vec3 coord, float lod) {
   return $$texture(8, sampler,__brcm_cube(coord),lod);
}

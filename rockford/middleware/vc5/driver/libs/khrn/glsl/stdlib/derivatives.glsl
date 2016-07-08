//
// Derivative Functions
//

float dFdx(float p) { return $$fdx(p); }
vec2  dFdx(vec2  p) { return vec2(dFdx(p.x), dFdx(p.y)); }
vec3  dFdx(vec3  p) { return vec3(dFdx(p.x), dFdx(p.y), dFdx(p.z)); }
vec4  dFdx(vec4  p) { return vec4(dFdx(p.x), dFdx(p.y), dFdx(p.z), dFdx(p.w)); }

float dFdy(float p) { return $$fdy(p); }
vec2  dFdy(vec2  p) { return vec2(dFdy(p.x), dFdy(p.y)); }
vec3  dFdy(vec3  p) { return vec3(dFdy(p.x), dFdy(p.y), dFdy(p.z)); }
vec4  dFdy(vec4  p) { return vec4(dFdy(p.x), dFdy(p.y), dFdy(p.z), dFdy(p.w)); }

float fwidth(float p) { return abs(dFdx(p)) + abs(dFdy(p)); }
vec2  fwidth(vec2  p) { return abs(dFdx(p)) + abs(dFdy(p)); }
vec3  fwidth(vec3  p) { return abs(dFdx(p)) + abs(dFdy(p)); }
vec4  fwidth(vec4  p) { return abs(dFdx(p)) + abs(dFdy(p)); }

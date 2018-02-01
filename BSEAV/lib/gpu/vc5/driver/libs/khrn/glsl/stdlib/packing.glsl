//
// Packing/unpacking functions
//

highp uint packUnorm2x16(vec2 v) { return $$packunorm2x16(v); }
highp uint packSnorm2x16(vec2 v) { return $$packsnorm2x16(v); }
highp uint packHalf2x16(mediump vec2 v) { return $$fpack(v[0], v[1]); }
highp uint packUnorm4x8(mediump vec4 v) { return $$packunorm4x8(v); }
highp uint packSnorm4x8(mediump vec4 v) { return $$packsnorm4x8(v); }

highp vec2 unpackUnorm2x16(highp uint p) { return $$unpackunorm2x16(p); }
highp vec2 unpackSnorm2x16(highp uint p) { return $$unpacksnorm2x16(p); }
mediump vec2 unpackHalf2x16(highp uint p) { return vec2($$funpacka(p), $$funpackb(p)); }
mediump vec4 unpackUnorm4x8(highp uint v) { return $$unpackunorm4x8(v); }
mediump vec4 unpackSnorm4x8(highp uint v) { return $$unpacksnorm4x8(v); }

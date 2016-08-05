//
// Packing/unpacking functions
//

highp uint packSnorm2x16(vec2 v)
{
  uint l = uint(int(round(clamp(v[0], -1.0, 1.0) * 32767.0)));
  uint h = uint(int(round(clamp(v[1], -1.0, 1.0) * 32767.0)));
  return ((h << 0x10u) | (l & 0xFFFFu));
}

highp vec2 unpackSnorm2x16(highp uint p)
{
  highp vec2 v;
  uvec2 u = uvec2(p & 0xFFFFu, p >> 0x10u);
   // Clamp to [-1:1]. We can't exceed 1, so only clamp below
  v[0] = max(float(int(u[0] & 0x7FFFu) - int(u[0] & 0x8000u))/ 32767.0, -1.0);
  v[1] = max(float(int(u[1] & 0x7FFFu) - int(u[1] & 0x8000u))/ 32767.0, -1.0);
  return v;
}

highp uint packUnorm2x16(vec2 v)
{
   uint l = uint(round(clamp(v[0], 0.0, 1.0) * 65535.0));
   uint h = uint(round(clamp(v[1], 0.0, 1.0) * 65535.0));
  return ((h << 0x10u) | l);
}

highp vec2 unpackUnorm2x16(highp uint p)
{
  highp vec2 v;
  v[0] = float(p &  0xFFFFu)/ 65535.0;
  v[1] = float(p >> 0x10u)  / 65535.0;
  return v;
}

highp uint packHalf2x16(mediump vec2 v)
{
  return $$fpack(v[0], v[1]);
}

mediump vec2 unpackHalf2x16(highp uint p)
{
  mediump vec2 v;
  v[0] = $$funpacka(p);
  v[1] = $$funpackb(p);
  return v;
}

highp uint packUnorm4x8(mediump vec4 v)
{
   uint a = uint(int(round(clamp(v[0], 0.0, 1.0) * 255.0)));
   uint b = uint(int(round(clamp(v[1], 0.0, 1.0) * 255.0)));
   uint c = uint(int(round(clamp(v[2], 0.0, 1.0) * 255.0)));
   uint d = uint(int(round(clamp(v[3], 0.0, 1.0) * 255.0)));
   return ((d << 24) | (c << 16) | (b << 8) | a);
}

highp uint packSnorm4x8(mediump vec4 v)
{
   uint a = uint(int(round(clamp(v[0], -1.0, 1.0) * 127.0))) & 0xFFu;
   uint b = uint(int(round(clamp(v[1], -1.0, 1.0) * 127.0))) & 0xFFu;
   uint c = uint(int(round(clamp(v[2], -1.0, 1.0) * 127.0))) & 0xFFu;
   uint d = uint(int(round(clamp(v[3], -1.0, 1.0) * 127.0)));
   return ((d << 24) | (c << 16) | (b << 8) | a);
}

mediump vec4 unpackUnorm4x8(highp uint v)
{
   mediump vec4 r;
   r[0] = float( v      & 0xFFu) / 255.0;
   r[1] = float((v>>8)  & 0xFFu) / 255.0;
   r[2] = float((v>>16) & 0xFFu) / 255.0;
   r[3] = float((v>>24)        ) / 255.0;
   return r;
}

mediump vec4 unpackSnorm4x8(highp uint v)
{
   mediump vec4 r;
   uvec4 u = uvec4(v & 0xFFu, (v>>8)&0xFFu, (v>>16)&0xFFu, (v>>24)&0xFFu);
   // Clamp to [-1:1]. We can't exceed 1, so only clamp below
   r[0] = max(float(int(u[0] & 0x7Fu) - int(u[0] & 0x80u)) / 127.0, -1.0);
   r[1] = max(float(int(u[1] & 0x7Fu) - int(u[1] & 0x80u)) / 127.0, -1.0);
   r[2] = max(float(int(u[2] & 0x7Fu) - int(u[2] & 0x80u)) / 127.0, -1.0);
   r[3] = max(float(int(u[3] & 0x7Fu) - int(u[3] & 0x80u)) / 127.0, -1.0);
   return r;
}

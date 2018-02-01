/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "shader_programs.h"

namespace video_texturing
{
// vertex shader for rectangular plane
const char vShaderStrRect[] =
   "uniform mat4   u_mvpMatrix;               \n"
   "attribute vec4 a_position;                \n"
   "attribute vec4 a_texcoord;                \n"
   "varying vec4   v_texcoord;                \n"
   "                                          \n"
   "void main()                               \n"
   "{                                         \n"
   "  gl_Position = u_mvpMatrix * a_position; \n"
   "  v_texcoord  = a_texcoord;               \n"
   "}                                         \n";

// fragment shader for rectangular plane
const char fShaderStrRect[] =
   "precision mediump float;                                   \n"
   "uniform sampler2D u_textureUnit;                           \n"
   "varying vec4      v_texcoord;                              \n"
   "                                                           \n"
   "void main()                                                \n"
   "{                                                          \n"
   "  gl_FragColor = texture2D(u_textureUnit, v_texcoord.st);  \n"
   "}                                                          \n";

// vertex shader program for equirectangular projection
const char vShaderStrEq[] =
   "attribute vec4 a_position;                  \n"
   "varying   vec4 texCoords;                   \n"
   "uniform   mat4 u_mvpMatrix;                 \n"
   "                                            \n"
   "void main()                                 \n"
   "{                                           \n"
   "    gl_Position = u_mvpMatrix * a_position; \n"
   "    texCoords   = a_position;               \n"
   "}                                           \n";

// fragment shader program for equirectangular projection
const char fShaderStrEq[] =
   "precision mediump   float;                                                                                      \n"
   "uniform   sampler2D texture;                                                                                    \n"
   "varying   vec4      texCoords;                                                                                  \n"
   "const     float     invPI = 1.0 / 3.141592653589793238462643383;                                                \n"
   "                                                                                                                \n"
   "void main()                                                                                                     \n"
   "{                                                                                                               \n"
   "    float longitude = ((atan(texCoords.z, texCoords.x) * invPI + 1.0) * 0.5);                                   \n"
   "    float invlen    = inversesqrt(texCoords.x*texCoords.x + texCoords.y*texCoords.y + texCoords.z*texCoords.z); \n"
   "    float latitude  = 0.5 - asin(texCoords.y * invlen) * invPI;                                                 \n"
   "    gl_FragColor    = texture2D(texture, vec2(longitude, latitude));                                            \n"
   "}                                                                                                               \n";

// fragment shader program for equal area projection
const char fShaderStrEap[] =
   "precision mediump   float;                                                                                      \n"
   "uniform   sampler2D texture;                                                                                    \n"
   "varying   vec4      texCoords;                                                                                  \n"
   "const     float     invPI = 1.0 / 3.141592653589793238462643383;                                                \n"
   "                                                                                                                \n"
   "void main()                                                                                                     \n"
   "{                                                                                                               \n"
   "    float longitude = ((atan(texCoords.z, texCoords.x) * invPI + 1.0) * 0.5);                                   \n"
   "    float invlen    = inversesqrt(texCoords.x*texCoords.x + texCoords.y*texCoords.y + texCoords.z*texCoords.z); \n"
   "    float latitude  = 0.5 - (texCoords.y * invlen) * 0.5;                                                       \n"
   "    gl_FragColor    = texture2D(texture, vec2(longitude, latitude));                                            \n"
   "}                                                                                                               \n";

// fragment shader program for fisheye projection
const char fShaderStrFish[] =
   "precision mediump   float;                                       \n"
   "uniform   sampler2D texture;                                     \n"
   "varying   vec4      texCoords;                                   \n"
   "const     float     invPI = 1.0 / 3.141592653589793238462643383; \n"
   "                                                                 \n"
   "void main()                                                      \n"
   "{                                                                \n"
   "    vec3  sphere = normalize(texCoords.xyz);                     \n"
   "    float d      = length(sphere.xy);                            \n"
   "    vec2  texPos = vec2(0.0, 0.0);                               \n"
   "    if (d != 0.0)                                                \n"
   "    {                                                            \n"
   "        float angle = atan(d, abs(sphere.z)) * invPI / d;        \n"
   "        texPos = angle * sphere.xy;                              \n"
   "    }                                                            \n"
   "                                                                 \n"
   "    texPos.x *= 0.5;                                             \n"
   "    if (texCoords.z > 0.0)                                       \n"
   "        texPos.x = 0.75 - texPos.x;                              \n"
   "    else                                                         \n"
   "        texPos.x = 0.25 + texPos.x;                              \n"
   "    texPos.y = 0.5 - texPos.y;                                   \n"
   "    gl_FragColor = texture2D(texture, texPos);                   \n"
   "}                                                                \n";

// vertex shader program for cube projection
const char vShaderStrCube[] =
   "attribute vec4 a_position;                        \n"
   "attribute vec2 a_texcoord;                        \n"
   "uniform   mat4 u_mvpMatrix;                       \n"
   "varying   vec2 texCoords;                         \n"
   "                                                  \n"
   "void main()                                       \n"
   "{                                                 \n"
   "    gl_Position = u_mvpMatrix * a_position;       \n"
   "    texCoords   = a_texcoord;                     \n"
   "}                                                 \n";

// fragment shader program for cube projection
const char fShaderStrCube[] =
   "precision mediump   float;                        \n"
   "uniform   sampler2D texture;                      \n"
   "varying   vec2      texCoords;                    \n"
   "                                                  \n"
   "void main()                                       \n"
   "{                                                 \n"
   "    gl_FragColor = texture2D(texture, texCoords); \n"
   "}                                                 \n";

const char *VertexShaderStr(bool is360, Format360 fmt)
{
   if (!is360)
      return vShaderStrRect;

   switch (fmt)
   {
   case FORMAT_EQUIRECT        : return vShaderStrEq;
   case FORMAT_EQUIRECT_SPHERE : return vShaderStrCube;
   case FORMAT_FISHEYE         : return vShaderStrEq;
   case FORMAT_EAP             : return vShaderStrEq;
   default                     : return vShaderStrCube;
   }
}

const char *FragmentShaderStr(bool is360, Format360 fmt)
{
   if (!is360)
      return fShaderStrRect;

   switch (fmt)
   {
   case FORMAT_EQUIRECT        : return fShaderStrEq;
   case FORMAT_EQUIRECT_SPHERE : return fShaderStrCube;
   case FORMAT_FISHEYE         : return fShaderStrFish;
   case FORMAT_EAP             : return fShaderStrEap;
   default                     : return fShaderStrCube;
   }
}

} // namespace
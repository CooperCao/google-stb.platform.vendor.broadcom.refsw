#version 400
#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

layout(std140, binding = 0) uniform buf
{
   mat4  modelViewProjection;
} uniformBuffer;

layout (location = 0) in  vec4 positionAttrib;
layout (location = 1) in  vec2 uvAttrib;

layout (location = 0) out vec2 uv;

void main()
{
   uv = uvAttrib;
   gl_Position = uniformBuffer.modelViewProjection * positionAttrib;
}

// All versions
const mediump int gl_MaxVertexAttribs             = GL_MAXVERTEXATTRIBS;
const mediump int gl_MaxVertexUniformVectors      = GL_MAXVERTEXUNIFORMVECTORS;
const mediump int gl_MaxVaryingVectors            = GL_MAXVARYINGVECTORS;
const mediump int gl_MaxVertexTextureImageUnits   = GL_MAXVERTEXTEXTUREIMAGEUNITS;
const mediump int gl_MaxCombinedTextureImageUnits = GL_MAXCOMBINEDTEXTUREIMAGEUNITS;
const mediump int gl_MaxTextureImageUnits         = GL_MAXTEXTUREIMAGEUNITS;
const mediump int gl_MaxFragmentUniformVectors    = GL_MAXFRAGMENTUNIFORMVECTORS;
const mediump int gl_MaxDrawBuffers               = GLXX_MAX_RENDER_TARGETS;

in mediump float __brcm_LineCoord = DATAFLOW_GET_LINE_COORD;

struct gl_DepthRangeParameters {
   highp float near; //n
   highp float far;  //f
   highp float diff; //f - n
};

// FIXME should be uniform
gl_DepthRangeParameters gl_DepthRange = gl_DepthRangeParameters(DATAFLOW_GET_DEPTHRANGE_NEAR,
                                                                DATAFLOW_GET_DEPTHRANGE_FAR,
                                                                DATAFLOW_GET_DEPTHRANGE_DIFF);

// 'in' and 'out' qualifiers added for v100.
out highp   vec4  gl_Position;
in          bool  gl_FrontFacing = DATAFLOW_FRAG_GET_FF;
in  mediump vec2  gl_PointCoord  = vec2(DATAFLOW_GET_POINT_COORD_X, DATAFLOW_GET_POINT_COORD_Y);

// V100 - TODO: Should we just raise the precision to make these match V300?
out mediump float gl_PointSize;
in  mediump vec4  gl_FragCoord = vec4(DATAFLOW_FRAG_GET_X, DATAFLOW_FRAG_GET_Y, DATAFLOW_FRAG_GET_Z, DATAFLOW_FRAG_GET_W);
out mediump vec4  gl_FragColor;
out mediump vec4  gl_FragData[gl_MaxDrawBuffers];

// V300
const mediump int gl_MaxVertexOutputVectors   = GL_MAXVERTEXOUTPUTVECTORS;
const mediump int gl_MaxFragmentInputVectors  = GL_MAXFRAGMENTINPUTVECTORS;
const mediump int gl_MinProgramTexelOffset    = GL_MINPROGRAMTEXELOFFSET;
const mediump int gl_MaxProgramTexelOffset    = GL_MAXPROGRAMTEXELOFFSET;

in  highp int   gl_VertexID   = DATAFLOW_GET_VERTEX_ID;
in  highp int   gl_InstanceID = DATAFLOW_GET_INSTANCE_ID;
out highp float gl_PointSize;
in  highp vec4  gl_FragCoord = vec4(DATAFLOW_FRAG_GET_X, DATAFLOW_FRAG_GET_Y, DATAFLOW_FRAG_GET_Z, DATAFLOW_FRAG_GET_W);
out highp float gl_FragDepth;

// V310
const mediump int gl_MaxImageUnits            = GLXX_CONFIG_MAX_IMAGE_UNITS;
const mediump int gl_MaxVertexImageUniforms   = GLXX_CONFIG_MAX_VERTEX_IMAGE_UNIFORMS;
const mediump int gl_MaxFragmentImageUniforms = GLXX_CONFIG_MAX_SHADER_IMAGE_UNIFORMS;
const mediump int gl_MaxComputeImageUniforms  = GLXX_CONFIG_MAX_SHADER_IMAGE_UNIFORMS;
const mediump int gl_MaxCombinedImageUniforms = GLXX_CONFIG_MAX_COMBINED_IMAGE_UNIFORMS;
const mediump int gl_MaxCombinedShaderOutputResources = GLXX_CONFIG_MAX_COMBINED_SHADER_OUTPUTS;
const highp ivec3 gl_MaxComputeWorkGroupCount        = ivec3(GLXX_CONFIG_MAX_COMPUTE_GROUP_COUNT,
                                                             GLXX_CONFIG_MAX_COMPUTE_GROUP_COUNT,
                                                             GLXX_CONFIG_MAX_COMPUTE_GROUP_COUNT);
const highp ivec3 gl_MaxComputeWorkGroupSize         = ivec3(GLXX_CONFIG_MAX_COMPUTE_GROUP_SIZE_X,
                                                             GLXX_CONFIG_MAX_COMPUTE_GROUP_SIZE_Y,
                                                             GLXX_CONFIG_MAX_COMPUTE_GROUP_SIZE_Z);
const mediump int gl_MaxComputeUniformComponents     = GLXX_CONFIG_MAX_UNIFORM_SCALARS;
const mediump int gl_MaxComputeTextureImageUnits     = GLXX_CONFIG_MAX_SHADER_TEXTURE_IMAGE_UNITS;
const mediump int gl_MaxComputeAtomicCounters        = GLXX_CONFIG_MAX_SHADER_ATOMIC_COUNTERS;
const mediump int gl_MaxComputeAtomicCounterBuffers  = GLXX_CONFIG_MAX_SHADER_ATOMIC_COUNTER_BUFFERS;
const mediump int gl_MaxVertexAtomicCounters         = GLXX_CONFIG_MAX_VERTEX_ATOMIC_COUNTERS;
const mediump int gl_MaxVertexAtomicCounterBuffers   = GLXX_CONFIG_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS;
const mediump int gl_MaxFragmentAtomicCounters       = GLXX_CONFIG_MAX_SHADER_ATOMIC_COUNTERS;
const mediump int gl_MaxFragmentAtomicCounterBuffers = GLXX_CONFIG_MAX_SHADER_ATOMIC_COUNTER_BUFFERS;
const mediump int gl_MaxCombinedAtomicCounters       = GLXX_CONFIG_MAX_COMBINED_ATOMIC_COUNTERS;
const mediump int gl_MaxCombinedAtomicCounterBuffers = GLXX_CONFIG_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS;
const mediump int gl_MaxAtomicCounterBindings        = GLXX_CONFIG_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS;
const mediump int gl_MaxAtomicCounterBufferSize      = GLXX_CONFIG_MAX_ATOMIC_COUNTER_BUFFER_SIZE;

in bool gl_HelperInvocation = DATAFLOW_IS_HELPER;

in    uvec3 gl_NumWorkGroups = uvec3(DATAFLOW_GET_NUMWORKGROUPS_X, DATAFLOW_GET_NUMWORKGROUPS_Y, DATAFLOW_GET_NUMWORKGROUPS_Z);
//const uvec3 gl_WorkGroupSize;       // Defined elsewhere. The value is known too late to include here.
in    uvec3 gl_WorkGroupID;           // Value computed later, during compilation
in    uvec3 gl_LocalInvocationID;     // Value computed later, during compilation
in    uvec3 gl_GlobalInvocationID;    // Value computed later, during compilation
in    uint  gl_LocalInvocationIndex;  // Value computed later, during compilation


// V320
// Vertex
// TODO: Interface blocks not supported. gl_Position and gl_PointSize are left in for now.


// Tessellation Control
// TODO: Interface blocks not supported
in highp int gl_PatchVerticesIn;
in highp int gl_PrimitiveID;
in highp int gl_InvocationID = DATAFLOW_GET_INVOCATION_ID;

patch out highp float gl_TessLevelOuter[4];
patch out highp float gl_TessLevelInner[2];


// Tessellation Evaluation
// TODO: Interface blocks not supported
      in highp vec3  gl_TessCoord;
patch in highp float gl_TessLevelOuter[4];
patch in highp float gl_TessLevelInner[2];


const mediump int gl_MaxTessControlInputComponents = 128;
const mediump int gl_MaxTessControlOutputComponents = 128;
const mediump int gl_MaxTessControlTextureImageUnits = 16;
const mediump int gl_MaxTessControlUniformComponents = 1024;
const mediump int gl_MaxTessControlTotalOutputComponents = 4096;
const mediump int gl_MaxTessControlImageUniforms = 0;
const mediump int gl_MaxTessEvaluationImageUniforms = 0;
const mediump int gl_MaxTessControlAtomicCounters = 0;
const mediump int gl_MaxTessEvaluationAtomicCounters = 0;
const mediump int gl_MaxTessControlAtomicCounterBuffers = 0;
const mediump int gl_MaxTessEvaluationAtomicCounterBuffers = 0;
const mediump int gl_MaxTessEvaluationInputComponents = 128;
const mediump int gl_MaxTessEvaluationOutputComponents = 128;
const mediump int gl_MaxTessEvaluationTextureImageUnits = 16;
const mediump int gl_MaxTessEvaluationUniformComponents = 1024;
const mediump int gl_MaxTessPatchComponents = 120;
const mediump int gl_MaxPatchVertices = 32;
const mediump int gl_MaxTessGenLevel = 64;

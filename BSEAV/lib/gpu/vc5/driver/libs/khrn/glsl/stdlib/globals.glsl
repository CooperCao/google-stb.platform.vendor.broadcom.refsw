// All versions
const mediump int gl_MaxVertexAttribs             = GLXX_CONFIG_MAX_VERTEX_ATTRIBS;
const mediump int gl_MaxVertexUniformVectors      = GLXX_CONFIG_MAX_UNIFORM_VECTORS;
const mediump int gl_MaxVaryingVectors            = GLXX_CONFIG_MAX_VARYING_VECTORS;
const mediump int gl_MaxVertexTextureImageUnits   = GLXX_CONFIG_MAX_SHADER_TEXTURE_IMAGE_UNITS;
const mediump int gl_MaxCombinedTextureImageUnits = GLXX_CONFIG_MAX_COMBINED_TEXTURE_IMAGE_UNITS;
const mediump int gl_MaxTextureImageUnits         = GLXX_CONFIG_MAX_SHADER_TEXTURE_IMAGE_UNITS;
const mediump int gl_MaxFragmentUniformVectors    = GLXX_CONFIG_MAX_UNIFORM_VECTORS;
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
const mediump int gl_MaxVertexOutputVectors   = GLXX_CONFIG_MAX_VARYING_VECTORS;
const mediump int gl_MaxFragmentInputVectors  = GLXX_CONFIG_MAX_VARYING_VECTORS;
const mediump int gl_MinProgramTexelOffset    = GLXX_CONFIG_MIN_TEXEL_OFFSET;
const mediump int gl_MaxProgramTexelOffset    = GLXX_CONFIG_MAX_TEXEL_OFFSET;

in  highp int   gl_VertexID   = DATAFLOW_GET_VERTEX_ID;
in  highp int   gl_InstanceID = DATAFLOW_GET_INSTANCE_ID;
out highp float gl_PointSize;
in  highp vec4  gl_FragCoord = vec4(DATAFLOW_FRAG_GET_X, DATAFLOW_FRAG_GET_Y, DATAFLOW_FRAG_GET_Z, DATAFLOW_FRAG_GET_W);
out highp float gl_FragDepth;

in highp int __brcm_base_instance = DATAFLOW_GET_BASE_INSTANCE;

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
patch out highp vec4  gl_BoundingBox[2];
patch out highp vec4  gl_BoundingBoxOES[2];
patch out highp vec4  gl_BoundingBoxEXT[2];


// Tessellation Evaluation
// TODO: Interface blocks not supported
      in highp vec3  gl_TessCoord;
patch in highp float gl_TessLevelOuter[4];
patch in highp float gl_TessLevelInner[2];


//Geometry

in highp int gl_PrimitiveIDIn;

out highp int gl_PrimitiveID;
out highp int gl_Layer;


// Fragment Shader
in  highp   int  gl_Layer;
in  lowp    int  gl_SampleID = DATAFLOW_SAMPLE_ID;
in  mediump vec2 gl_SamplePosition = vec2(DATAFLOW_SAMPLE_POS_X, DATAFLOW_SAMPLE_POS_Y);
// TODO: Worth updating the parser to put the real array size here?
in  highp   int  gl_SampleMaskIn[1] = int[1](DATAFLOW_SAMPLE_MASK);
out highp   int  gl_SampleMask[1];

uniform lowp int gl_NumSamples = DATAFLOW_NUM_SAMPLES;


const mediump int gl_MaxTessControlInputComponents         = GLXX_CONFIG_MAX_TESS_CONTROL_INPUT_COMPONENTS;
const mediump int gl_MaxTessControlOutputComponents        = GLXX_CONFIG_MAX_TESS_CONTROL_OUTPUT_COMPONENTS;
const mediump int gl_MaxTessControlTextureImageUnits       = GLXX_CONFIG_MAX_SHADER_TEXTURE_IMAGE_UNITS;
const mediump int gl_MaxTessControlUniformComponents       = GLXX_CONFIG_MAX_UNIFORM_SCALARS;
const mediump int gl_MaxTessControlTotalOutputComponents   = GLXX_CONFIG_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS;
const mediump int gl_MaxTessControlImageUniforms           = GLXX_CONFIG_MAX_VERTEX_IMAGE_UNIFORMS;
const mediump int gl_MaxTessEvaluationImageUniforms        = GLXX_CONFIG_MAX_VERTEX_IMAGE_UNIFORMS;
const mediump int gl_MaxTessControlAtomicCounters          = GLXX_CONFIG_MAX_VERTEX_ATOMIC_COUNTERS;
const mediump int gl_MaxTessEvaluationAtomicCounters       = GLXX_CONFIG_MAX_VERTEX_ATOMIC_COUNTERS;
const mediump int gl_MaxTessControlAtomicCounterBuffers    = GLXX_CONFIG_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS;
const mediump int gl_MaxTessEvaluationAtomicCounterBuffers = GLXX_CONFIG_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS;
const mediump int gl_MaxTessEvaluationInputComponents      = GLXX_CONFIG_MAX_TESS_EVALUATION_INPUT_COMPONENTS;
const mediump int gl_MaxTessEvaluationOutputComponents     = GLXX_CONFIG_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS;
const mediump int gl_MaxTessEvaluationTextureImageUnits    = GLXX_CONFIG_MAX_SHADER_TEXTURE_IMAGE_UNITS;
const mediump int gl_MaxTessEvaluationUniformComponents    = GLXX_CONFIG_MAX_UNIFORM_SCALARS;
const mediump int gl_MaxTessPatchComponents                = GLXX_CONFIG_MAX_TESS_PATCH_COMPONENTS;
const mediump int gl_MaxPatchVertices                      = V3D_MAX_PATCH_VERTICES;
const mediump int gl_MaxTessGenLevel                       = V3D_MAX_TESS_GEN_LEVEL;
const mediump int gl_MaxGeometryInputComponents            = GLXX_CONFIG_MAX_GEOMETRY_INPUT_COMPONENTS;
const mediump int gl_MaxGeometryOutputComponents           = GLXX_CONFIG_MAX_GEOMETRY_OUTPUT_COMPONENTS;
const mediump int gl_MaxGeometryImageUniforms              = GLXX_CONFIG_MAX_VERTEX_IMAGE_UNIFORMS;
const mediump int gl_MaxGeometryTextureImageUnits          = GLXX_CONFIG_MAX_SHADER_TEXTURE_IMAGE_UNITS;
const mediump int gl_MaxGeometryOutputVertices             = GLXX_CONFIG_MAX_GEOMETRY_OUTPUT_VERTICES;
const mediump int gl_MaxGeometryTotalOutputComponents      = GLXX_CONFIG_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS;
const mediump int gl_MaxGeometryUniformComponents          = GLXX_CONFIG_MAX_UNIFORM_SCALARS;
const mediump int gl_MaxGeometryAtomicCounters             = GLXX_CONFIG_MAX_VERTEX_ATOMIC_COUNTERS;
const mediump int gl_MaxGeometryAtomicCounterBuffers       = GLXX_CONFIG_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS;
const mediump int gl_MaxSamples                            = GLXX_CONFIG_MAX_SAMPLES;

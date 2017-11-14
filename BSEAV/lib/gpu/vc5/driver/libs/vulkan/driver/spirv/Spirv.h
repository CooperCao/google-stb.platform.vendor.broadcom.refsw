/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

/* Auto-generated by libs/vulkan/scripts/spirv/spirv.py */
/*                   DO NOT HAND EDIT                   */

#pragma once

#include <stdint.h>

namespace spv {

enum class SourceLanguage : uint32_t
{
   Unknown = 0,
   ESSL = 1,
   GLSL = 2,
   OpenCL_C = 3,
   OpenCL_CPP = 4,
   HLSL = 5,
};

enum class ExecutionModel : uint32_t
{
   Vertex = 0,
   TessellationControl = 1,
   TessellationEvaluation = 2,
   Geometry = 3,
   Fragment = 4,
   GLCompute = 5,
   Kernel = 6,
};

enum class AddressingModel : uint32_t
{
   Logical = 0,
   Physical32 = 1,
   Physical64 = 2,
};

enum class MemoryModel : uint32_t
{
   Simple = 0,
   GLSL450 = 1,
   OpenCL = 2,
};

enum class ExecutionMode : uint32_t
{
   Invocations = 0,
   SpacingEqual = 1,
   SpacingFractionalEven = 2,
   SpacingFractionalOdd = 3,
   VertexOrderCw = 4,
   VertexOrderCcw = 5,
   PixelCenterInteger = 6,
   OriginUpperLeft = 7,
   OriginLowerLeft = 8,
   EarlyFragmentTests = 9,
   PointMode = 10,
   Xfb = 11,
   DepthReplacing = 12,
   DepthGreater = 14,
   DepthLess = 15,
   DepthUnchanged = 16,
   LocalSize = 17,
   LocalSizeHint = 18,
   InputPoints = 19,
   InputLines = 20,
   InputLinesAdjacency = 21,
   Triangles = 22,
   InputTrianglesAdjacency = 23,
   Quads = 24,
   Isolines = 25,
   OutputVertices = 26,
   OutputPoints = 27,
   OutputLineStrip = 28,
   OutputTriangleStrip = 29,
   VecTypeHint = 30,
   ContractionOff = 31,
   Initializer = 33,
   Finalizer = 34,
   SubgroupSize = 35,
   SubgroupsPerWorkgroup = 36,
};

enum class StorageClass : uint32_t
{
   UniformConstant = 0,
   Input = 1,
   Uniform = 2,
   Output = 3,
   Workgroup = 4,
   CrossWorkgroup = 5,
   Private = 6,
   Function = 7,
   Generic = 8,
   PushConstant = 9,
   AtomicCounter = 10,
   Image = 11,
   StorageBuffer = 12,
};

enum class Dim : uint32_t
{
   Dim1D = 0,
   Dim2D = 1,
   Dim3D = 2,
   Cube = 3,
   Rect = 4,
   Buffer = 5,
   SubpassData = 6,
};

enum class SamplerAddressingMode : uint32_t
{
   None = 0,
   ClampToEdge = 1,
   Clamp = 2,
   Repeat = 3,
   RepeatMirrored = 4,
};

enum class SamplerFilterMode : uint32_t
{
   Nearest = 0,
   Linear = 1,
};

enum class ImageFormat : uint32_t
{
   Unknown = 0,
   Rgba32f = 1,
   Rgba16f = 2,
   R32f = 3,
   Rgba8 = 4,
   Rgba8Snorm = 5,
   Rg32f = 6,
   Rg16f = 7,
   R11fG11fB10f = 8,
   R16f = 9,
   Rgba16 = 10,
   Rgb10A2 = 11,
   Rg16 = 12,
   Rg8 = 13,
   R16 = 14,
   R8 = 15,
   Rgba16Snorm = 16,
   Rg16Snorm = 17,
   Rg8Snorm = 18,
   R16Snorm = 19,
   R8Snorm = 20,
   Rgba32i = 21,
   Rgba16i = 22,
   Rgba8i = 23,
   R32i = 24,
   Rg32i = 25,
   Rg16i = 26,
   Rg8i = 27,
   R16i = 28,
   R8i = 29,
   Rgba32ui = 30,
   Rgba16ui = 31,
   Rgba8ui = 32,
   R32ui = 33,
   Rgb10a2ui = 34,
   Rg32ui = 35,
   Rg16ui = 36,
   Rg8ui = 37,
   R16ui = 38,
   R8ui = 39,
};

enum class ImageChannelOrder : uint32_t
{
   R = 0,
   A = 1,
   RG = 2,
   RA = 3,
   RGB = 4,
   RGBA = 5,
   BGRA = 6,
   ARGB = 7,
   Intensity = 8,
   Luminance = 9,
   Rx = 10,
   RGx = 11,
   RGBx = 12,
   Depth = 13,
   DepthStencil = 14,
   sRGB = 15,
   sRGBx = 16,
   sRGBA = 17,
   sBGRA = 18,
   ABGR = 19,
};

enum class ImageChannelDataType : uint32_t
{
   SnormInt8 = 0,
   SnormInt16 = 1,
   UnormInt8 = 2,
   UnormInt16 = 3,
   UnormShort565 = 4,
   UnormShort555 = 5,
   UnormInt101010 = 6,
   SignedInt8 = 7,
   SignedInt16 = 8,
   SignedInt32 = 9,
   UnsignedInt8 = 10,
   UnsignedInt16 = 11,
   UnsignedInt32 = 12,
   HalfFloat = 13,
   Float = 14,
   UnormInt24 = 15,
   UnormInt101010_2 = 16,
};

enum ImageOperandsMask : uint32_t
{
   ImageOperandsMaskNone = 0,
   ImageOperandsBiasMask = 1,
   ImageOperandsLodMask = 2,
   ImageOperandsGradMask = 4,
   ImageOperandsConstOffsetMask = 8,
   ImageOperandsOffsetMask = 16,
   ImageOperandsConstOffsetsMask = 32,
   ImageOperandsSampleMask = 64,
   ImageOperandsMinLodMask = 128,
};

enum ImageOperandsShift : uint32_t
{
   ImageOperandsBiasShift = 0,
   ImageOperandsLodShift = 1,
   ImageOperandsGradShift = 2,
   ImageOperandsConstOffsetShift = 3,
   ImageOperandsOffsetShift = 4,
   ImageOperandsConstOffsetsShift = 5,
   ImageOperandsSampleShift = 6,
   ImageOperandsMinLodShift = 7,
};

enum FPFastMathModeMask : uint32_t
{
   FPFastMathModeMaskNone = 0,
   FPFastMathModeNotNaNMask = 1,
   FPFastMathModeNotInfMask = 2,
   FPFastMathModeNSZMask = 4,
   FPFastMathModeAllowRecipMask = 8,
   FPFastMathModeFastMask = 16,
};

enum FPFastMathModeShift : uint32_t
{
   FPFastMathModeNotNaNShift = 0,
   FPFastMathModeNotInfShift = 1,
   FPFastMathModeNSZShift = 2,
   FPFastMathModeAllowRecipShift = 3,
   FPFastMathModeFastShift = 4,
};

enum class FPRoundingMode : uint32_t
{
   RTE = 0,
   RTZ = 1,
   RTP = 2,
   RTN = 3,
};

enum class LinkageType : uint32_t
{
   Export = 0,
   Import = 1,
};

enum class AccessQualifier : uint32_t
{
   ReadOnly = 0,
   WriteOnly = 1,
   ReadWrite = 2,
};

enum class FunctionParameterAttribute : uint32_t
{
   Zext = 0,
   Sext = 1,
   ByVal = 2,
   Sret = 3,
   NoAlias = 4,
   NoCapture = 5,
   NoWrite = 6,
   NoReadWrite = 7,
};

enum class Decoration : uint32_t
{
   RelaxedPrecision = 0,
   SpecId = 1,
   Block = 2,
   BufferBlock = 3,
   RowMajor = 4,
   ColMajor = 5,
   ArrayStride = 6,
   MatrixStride = 7,
   GLSLShared = 8,
   GLSLPacked = 9,
   CPacked = 10,
   BuiltIn = 11,
   NoPerspective = 13,
   Flat = 14,
   Patch = 15,
   Centroid = 16,
   Sample = 17,
   Invariant = 18,
   Restrict = 19,
   Aliased = 20,
   Volatile = 21,
   Constant = 22,
   Coherent = 23,
   NonWritable = 24,
   NonReadable = 25,
   Uniform = 26,
   SaturatedConversion = 28,
   Stream = 29,
   Location = 30,
   Component = 31,
   Index = 32,
   Binding = 33,
   DescriptorSet = 34,
   Offset = 35,
   XfbBuffer = 36,
   XfbStride = 37,
   FuncParamAttr = 38,
   FPRoundingMode = 39,
   FPFastMathMode = 40,
   LinkageAttributes = 41,
   NoContraction = 42,
   InputAttachmentIndex = 43,
   Alignment = 44,
   MaxByteOffset = 45,
   OverrideCoverageNV = 5248,
   PassthroughNV = 5250,
   ViewportRelativeNV = 5252,
   SecondaryViewportRelativeNV = 5256,
};

enum class BuiltIn : uint32_t
{
   Position = 0,
   PointSize = 1,
   ClipDistance = 3,
   CullDistance = 4,
   VertexId = 5,
   InstanceId = 6,
   PrimitiveId = 7,
   InvocationId = 8,
   Layer = 9,
   ViewportIndex = 10,
   TessLevelOuter = 11,
   TessLevelInner = 12,
   TessCoord = 13,
   PatchVertices = 14,
   FragCoord = 15,
   PointCoord = 16,
   FrontFacing = 17,
   SampleId = 18,
   SamplePosition = 19,
   SampleMask = 20,
   FragDepth = 22,
   HelperInvocation = 23,
   NumWorkgroups = 24,
   WorkgroupSize = 25,
   WorkgroupId = 26,
   LocalInvocationId = 27,
   GlobalInvocationId = 28,
   LocalInvocationIndex = 29,
   WorkDim = 30,
   GlobalSize = 31,
   EnqueuedWorkgroupSize = 32,
   GlobalOffset = 33,
   GlobalLinearId = 34,
   SubgroupSize = 36,
   SubgroupMaxSize = 37,
   NumSubgroups = 38,
   NumEnqueuedSubgroups = 39,
   SubgroupId = 40,
   SubgroupLocalInvocationId = 41,
   VertexIndex = 42,
   InstanceIndex = 43,
   SubgroupEqMaskKHR = 4416,
   SubgroupGeMaskKHR = 4417,
   SubgroupGtMaskKHR = 4418,
   SubgroupLeMaskKHR = 4419,
   SubgroupLtMaskKHR = 4420,
   BaseVertex = 4424,
   BaseInstance = 4425,
   DrawIndex = 4426,
   DeviceIndex = 4438,
   ViewIndex = 4440,
   ViewportMaskNV = 5253,
   SecondaryPositionNV = 5257,
   SecondaryViewportMaskNV = 5258,
   PositionPerViewNV = 5261,
   ViewportMaskPerViewNV = 5262,
};

enum SelectionControlMask : uint32_t
{
   SelectionControlMaskNone = 0,
   SelectionControlFlattenMask = 1,
   SelectionControlDontFlattenMask = 2,
};

enum SelectionControlShift : uint32_t
{
   SelectionControlFlattenShift = 0,
   SelectionControlDontFlattenShift = 1,
};

enum LoopControlMask : uint32_t
{
   LoopControlMaskNone = 0,
   LoopControlUnrollMask = 1,
   LoopControlDontUnrollMask = 2,
   LoopControlDependencyInfiniteMask = 4,
   LoopControlDependencyLengthMask = 8,
};

enum LoopControlShift : uint32_t
{
   LoopControlUnrollShift = 0,
   LoopControlDontUnrollShift = 1,
   LoopControlDependencyInfiniteShift = 2,
   LoopControlDependencyLengthShift = 3,
};

enum FunctionControlMask : uint32_t
{
   FunctionControlMaskNone = 0,
   FunctionControlInlineMask = 1,
   FunctionControlDontInlineMask = 2,
   FunctionControlPureMask = 4,
   FunctionControlConstMask = 8,
};

enum FunctionControlShift : uint32_t
{
   FunctionControlInlineShift = 0,
   FunctionControlDontInlineShift = 1,
   FunctionControlPureShift = 2,
   FunctionControlConstShift = 3,
};

enum MemorySemanticsMask : uint32_t
{
   MemorySemanticsMaskNone = 0,
   MemorySemanticsAcquireMask = 2,
   MemorySemanticsReleaseMask = 4,
   MemorySemanticsAcquireReleaseMask = 8,
   MemorySemanticsSequentiallyConsistentMask = 16,
   MemorySemanticsUniformMemoryMask = 64,
   MemorySemanticsSubgroupMemoryMask = 128,
   MemorySemanticsWorkgroupMemoryMask = 256,
   MemorySemanticsCrossWorkgroupMemoryMask = 512,
   MemorySemanticsAtomicCounterMemoryMask = 1024,
   MemorySemanticsImageMemoryMask = 2048,
};

enum MemorySemanticsShift : uint32_t
{
   MemorySemanticsAcquireShift = 1,
   MemorySemanticsReleaseShift = 2,
   MemorySemanticsAcquireReleaseShift = 3,
   MemorySemanticsSequentiallyConsistentShift = 4,
   MemorySemanticsUniformMemoryShift = 6,
   MemorySemanticsSubgroupMemoryShift = 7,
   MemorySemanticsWorkgroupMemoryShift = 8,
   MemorySemanticsCrossWorkgroupMemoryShift = 9,
   MemorySemanticsAtomicCounterMemoryShift = 10,
   MemorySemanticsImageMemoryShift = 11,
};

enum MemoryAccessMask : uint32_t
{
   MemoryAccessMaskNone = 0,
   MemoryAccessVolatileMask = 1,
   MemoryAccessAlignedMask = 2,
   MemoryAccessNontemporalMask = 4,
};

enum MemoryAccessShift : uint32_t
{
   MemoryAccessVolatileShift = 0,
   MemoryAccessAlignedShift = 1,
   MemoryAccessNontemporalShift = 2,
};

enum class Scope : uint32_t
{
   CrossDevice = 0,
   Device = 1,
   Workgroup = 2,
   Subgroup = 3,
   Invocation = 4,
};

enum class GroupOperation : uint32_t
{
   Reduce = 0,
   InclusiveScan = 1,
   ExclusiveScan = 2,
};

enum class KernelEnqueueFlags : uint32_t
{
   NoWait = 0,
   WaitKernel = 1,
   WaitWorkGroup = 2,
};

enum KernelProfilingInfoMask : uint32_t
{
   KernelProfilingInfoMaskNone = 0,
   KernelProfilingInfoCmdExecTimeMask = 1,
};

enum KernelProfilingInfoShift : uint32_t
{
   KernelProfilingInfoCmdExecTimeShift = 0,
};

enum class Capability : uint32_t
{
   Matrix = 0,
   Shader = 1,
   Geometry = 2,
   Tessellation = 3,
   Addresses = 4,
   Linkage = 5,
   Kernel = 6,
   Vector16 = 7,
   Float16Buffer = 8,
   Float16 = 9,
   Float64 = 10,
   Int64 = 11,
   Int64Atomics = 12,
   ImageBasic = 13,
   ImageReadWrite = 14,
   ImageMipmap = 15,
   Pipes = 17,
   Groups = 18,
   DeviceEnqueue = 19,
   LiteralSampler = 20,
   AtomicStorage = 21,
   Int16 = 22,
   TessellationPointSize = 23,
   GeometryPointSize = 24,
   ImageGatherExtended = 25,
   StorageImageMultisample = 27,
   UniformBufferArrayDynamicIndexing = 28,
   SampledImageArrayDynamicIndexing = 29,
   StorageBufferArrayDynamicIndexing = 30,
   StorageImageArrayDynamicIndexing = 31,
   ClipDistance = 32,
   CullDistance = 33,
   ImageCubeArray = 34,
   SampleRateShading = 35,
   ImageRect = 36,
   SampledRect = 37,
   GenericPointer = 38,
   Int8 = 39,
   InputAttachment = 40,
   SparseResidency = 41,
   MinLod = 42,
   Sampled1D = 43,
   Image1D = 44,
   SampledCubeArray = 45,
   SampledBuffer = 46,
   ImageBuffer = 47,
   ImageMSArray = 48,
   StorageImageExtendedFormats = 49,
   ImageQuery = 50,
   DerivativeControl = 51,
   InterpolationFunction = 52,
   TransformFeedback = 53,
   GeometryStreams = 54,
   StorageImageReadWithoutFormat = 55,
   StorageImageWriteWithoutFormat = 56,
   MultiViewport = 57,
   SubgroupDispatch = 58,
   NamedBarrier = 59,
   PipeStorage = 60,
   SubgroupBallotKHR = 4423,
   DrawParameters = 4427,
   SubgroupVoteKHR = 4431,
   StorageBuffer16BitAccess = 4433,
   StorageUniformBufferBlock16 = 4433,
   UniformAndStorageBuffer16BitAccess = 4434,
   StorageUniform16 = 4434,
   StoragePushConstant16 = 4435,
   StorageInputOutput16 = 4436,
   DeviceGroup = 4437,
   MultiView = 4439,
   VariablePointersStorageBuffer = 4441,
   VariablePointers = 4442,
   SampleMaskOverrideCoverageNV = 5249,
   GeometryShaderPassthroughNV = 5251,
   ShaderViewportIndexLayerNV = 5254,
   ShaderViewportMaskNV = 5255,
   ShaderStereoViewNV = 5259,
   PerViewAttributesNV = 5260,
};




enum class Core : uint32_t
{
   OpNop = 0,
   OpUndef = 1,
   OpSourceContinued = 2,
   OpSource = 3,
   OpSourceExtension = 4,
   OpName = 5,
   OpMemberName = 6,
   OpString = 7,
   OpLine = 8,
   OpExtension = 10,
   OpExtInstImport = 11,
   OpExtInst = 12,
   OpMemoryModel = 14,
   OpEntryPoint = 15,
   OpExecutionMode = 16,
   OpCapability = 17,
   OpTypeVoid = 19,
   OpTypeBool = 20,
   OpTypeInt = 21,
   OpTypeFloat = 22,
   OpTypeVector = 23,
   OpTypeMatrix = 24,
   OpTypeImage = 25,
   OpTypeSampler = 26,
   OpTypeSampledImage = 27,
   OpTypeArray = 28,
   OpTypeRuntimeArray = 29,
   OpTypeStruct = 30,
   OpTypePointer = 32,
   OpTypeFunction = 33,
   OpConstantTrue = 41,
   OpConstantFalse = 42,
   OpConstant = 43,
   OpConstantComposite = 44,
   OpConstantNull = 46,
   OpSpecConstantTrue = 48,
   OpSpecConstantFalse = 49,
   OpSpecConstant = 50,
   OpSpecConstantComposite = 51,
   OpSpecConstantOp = 52,
   OpFunction = 54,
   OpFunctionParameter = 55,
   OpFunctionEnd = 56,
   OpFunctionCall = 57,
   OpVariable = 59,
   OpImageTexelPointer = 60,
   OpLoad = 61,
   OpStore = 62,
   OpCopyMemory = 63,
   OpAccessChain = 65,
   OpInBoundsAccessChain = 66,
   OpArrayLength = 68,
   OpDecorate = 71,
   OpMemberDecorate = 72,
   OpDecorationGroup = 73,
   OpGroupDecorate = 74,
   OpGroupMemberDecorate = 75,
   OpVectorExtractDynamic = 77,
   OpVectorInsertDynamic = 78,
   OpVectorShuffle = 79,
   OpCompositeConstruct = 80,
   OpCompositeExtract = 81,
   OpCompositeInsert = 82,
   OpCopyObject = 83,
   OpTranspose = 84,
   OpSampledImage = 86,
   OpImageSampleImplicitLod = 87,
   OpImageSampleExplicitLod = 88,
   OpImageSampleDrefImplicitLod = 89,
   OpImageSampleDrefExplicitLod = 90,
   OpImageSampleProjImplicitLod = 91,
   OpImageSampleProjExplicitLod = 92,
   OpImageSampleProjDrefImplicitLod = 93,
   OpImageSampleProjDrefExplicitLod = 94,
   OpImageFetch = 95,
   OpImageGather = 96,
   OpImageDrefGather = 97,
   OpImageRead = 98,
   OpImageWrite = 99,
   OpImage = 100,
   OpImageQuerySizeLod = 103,
   OpImageQuerySize = 104,
   OpImageQueryLod = 105,
   OpImageQueryLevels = 106,
   OpImageQuerySamples = 107,
   OpConvertFToU = 109,
   OpConvertFToS = 110,
   OpConvertSToF = 111,
   OpConvertUToF = 112,
   OpUConvert = 113,
   OpSConvert = 114,
   OpFConvert = 115,
   OpQuantizeToF16 = 116,
   OpBitcast = 124,
   OpSNegate = 126,
   OpFNegate = 127,
   OpIAdd = 128,
   OpFAdd = 129,
   OpISub = 130,
   OpFSub = 131,
   OpIMul = 132,
   OpFMul = 133,
   OpUDiv = 134,
   OpSDiv = 135,
   OpFDiv = 136,
   OpUMod = 137,
   OpSRem = 138,
   OpSMod = 139,
   OpFRem = 140,
   OpFMod = 141,
   OpVectorTimesScalar = 142,
   OpMatrixTimesScalar = 143,
   OpVectorTimesMatrix = 144,
   OpMatrixTimesVector = 145,
   OpMatrixTimesMatrix = 146,
   OpOuterProduct = 147,
   OpDot = 148,
   OpIAddCarry = 149,
   OpISubBorrow = 150,
   OpUMulExtended = 151,
   OpSMulExtended = 152,
   OpAny = 154,
   OpAll = 155,
   OpIsNan = 156,
   OpIsInf = 157,
   OpLogicalEqual = 164,
   OpLogicalNotEqual = 165,
   OpLogicalOr = 166,
   OpLogicalAnd = 167,
   OpLogicalNot = 168,
   OpSelect = 169,
   OpIEqual = 170,
   OpINotEqual = 171,
   OpUGreaterThan = 172,
   OpSGreaterThan = 173,
   OpUGreaterThanEqual = 174,
   OpSGreaterThanEqual = 175,
   OpULessThan = 176,
   OpSLessThan = 177,
   OpULessThanEqual = 178,
   OpSLessThanEqual = 179,
   OpFOrdEqual = 180,
   OpFUnordEqual = 181,
   OpFOrdNotEqual = 182,
   OpFUnordNotEqual = 183,
   OpFOrdLessThan = 184,
   OpFUnordLessThan = 185,
   OpFOrdGreaterThan = 186,
   OpFUnordGreaterThan = 187,
   OpFOrdLessThanEqual = 188,
   OpFUnordLessThanEqual = 189,
   OpFOrdGreaterThanEqual = 190,
   OpFUnordGreaterThanEqual = 191,
   OpShiftRightLogical = 194,
   OpShiftRightArithmetic = 195,
   OpShiftLeftLogical = 196,
   OpBitwiseOr = 197,
   OpBitwiseXor = 198,
   OpBitwiseAnd = 199,
   OpNot = 200,
   OpBitFieldInsert = 201,
   OpBitFieldSExtract = 202,
   OpBitFieldUExtract = 203,
   OpBitReverse = 204,
   OpBitCount = 205,
   OpDPdx = 207,
   OpDPdy = 208,
   OpFwidth = 209,
   OpDPdxFine = 210,
   OpDPdyFine = 211,
   OpFwidthFine = 212,
   OpDPdxCoarse = 213,
   OpDPdyCoarse = 214,
   OpFwidthCoarse = 215,
   OpControlBarrier = 224,
   OpMemoryBarrier = 225,
   OpAtomicLoad = 227,
   OpAtomicStore = 228,
   OpAtomicExchange = 229,
   OpAtomicCompareExchange = 230,
   OpAtomicIIncrement = 232,
   OpAtomicIDecrement = 233,
   OpAtomicIAdd = 234,
   OpAtomicISub = 235,
   OpAtomicSMin = 236,
   OpAtomicUMin = 237,
   OpAtomicSMax = 238,
   OpAtomicUMax = 239,
   OpAtomicAnd = 240,
   OpAtomicOr = 241,
   OpAtomicXor = 242,
   OpPhi = 245,
   OpLoopMerge = 246,
   OpSelectionMerge = 247,
   OpLabel = 248,
   OpBranch = 249,
   OpBranchConditional = 250,
   OpSwitch = 251,
   OpKill = 252,
   OpReturn = 253,
   OpReturnValue = 254,
   OpUnreachable = 255,
   OpNoLine = 317,
   OpModuleProcessed = 330,
};

enum class GLSL : uint32_t
{
   Round = 1,
   RoundEven = 2,
   Trunc = 3,
   FAbs = 4,
   SAbs = 5,
   FSign = 6,
   SSign = 7,
   Floor = 8,
   Ceil = 9,
   Fract = 10,
   Radians = 11,
   Degrees = 12,
   Sin = 13,
   Cos = 14,
   Tan = 15,
   Asin = 16,
   Acos = 17,
   Atan = 18,
   Sinh = 19,
   Cosh = 20,
   Tanh = 21,
   Asinh = 22,
   Acosh = 23,
   Atanh = 24,
   Atan2 = 25,
   Pow = 26,
   Exp = 27,
   Log = 28,
   Exp2 = 29,
   Log2 = 30,
   Sqrt = 31,
   InverseSqrt = 32,
   Determinant = 33,
   MatrixInverse = 34,
   Modf = 35,
   ModfStruct = 36,
   FMin = 37,
   UMin = 38,
   SMin = 39,
   FMax = 40,
   UMax = 41,
   SMax = 42,
   FClamp = 43,
   UClamp = 44,
   SClamp = 45,
   FMix = 46,
   IMix = 47,
   Step = 48,
   SmoothStep = 49,
   Fma = 50,
   Frexp = 51,
   FrexpStruct = 52,
   Ldexp = 53,
   PackSnorm4x8 = 54,
   PackUnorm4x8 = 55,
   PackSnorm2x16 = 56,
   PackUnorm2x16 = 57,
   PackHalf2x16 = 58,
   UnpackSnorm2x16 = 60,
   UnpackUnorm2x16 = 61,
   UnpackHalf2x16 = 62,
   UnpackSnorm4x8 = 63,
   UnpackUnorm4x8 = 64,
   Length = 66,
   Distance = 67,
   Cross = 68,
   Normalize = 69,
   FaceForward = 70,
   Reflect = 71,
   Refract = 72,
   FindILsb = 73,
   FindSMsb = 74,
   FindUMsb = 75,
   InterpolateAtCentroid = 76,
   InterpolateAtSample = 77,
   InterpolateAtOffset = 78,
   NMin = 79,
   NMax = 80,
   NClamp = 81,
};

enum class InstructionSet
{
   Core,
   GLSL
};

namespace con {
constexpr uint32_t MagicNumber = 119734787;
constexpr uint32_t Version = 65792;
constexpr uint32_t Revision = 7;
constexpr uint32_t OpCodeMask = 65535;
constexpr uint32_t WordCountShift = 16;
constexpr uint32_t HeaderWordCount = 5;
}

}

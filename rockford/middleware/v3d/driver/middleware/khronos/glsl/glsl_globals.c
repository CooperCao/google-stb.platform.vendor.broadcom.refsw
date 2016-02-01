/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Standalone GLSL compiler
=============================================================================*/
#include "middleware/khronos/glsl/glsl_common.h"

#include "middleware/khronos/glsl/glsl_globals.h"

// TODO: there are some globals we've left out here, e.g. primitiveType stuff

int g_ShaderSourceCurrentIndex;
int g_ShaderSourceCount;
const char** g_ShaderSourceValues;

/*
   g_ShaderFlavour

   Identifies whether we are currently compiling a vertex or fragment shader.

   Invariants:

   (GL20_COMPILER_SHADERFLAVOUR)
   During a compilation run, g_ShaderFlavour in {SHADER_VERTEX,SHADER_FRAGMENT}
   g_ShaderFlavour constant during a compilation run
*/
ShaderFlavour g_ShaderFlavour;

int g_FileNumber;
int g_LineNumber;

SymbolTable* g_SymbolTable;
DataflowSources* g_DataflowSources;
int g_AttributeScalarValueMask;
int g_AttributeScalarAliasMask;
int g_VaryingScalarValueCount;
Dataflow** g_VertexVaryingScalarValues;

SymbolType* g_TypeBuilder;
TypeQualifier g_TypeQual;
ParamQualifier g_ParamQual;
StringBuilder g_StringBuilder;
Expr* g_InitializerExpr;
Symbol* g_LastInstance;

SymbolTable* g_StructBuilderMembers;
int g_StructBuilderMemberCount;
int g_StructBuilderSizeAsConst;
bool g_StructBuilderCanBeConst;
int g_StructBuilderScalarCount;

SymbolType* g_FunctionBuilderReturnType;
const char* g_FunctionBuilderName;
SymbolTable* g_FunctionBuilderParams;
int g_FunctionBuilderParamCount;
int g_FunctionBuilderVoidCount;
Symbol* g_DefinitionInsertionSymbol;
SymbolType* g_DefinitionInsertionSymbolType;

int g_NextAnonParam;

bool g_InGlobalScope;

bool g_AssignedFragColor;
bool g_AssignedFragData;

int g_LoopDepth;

jmp_buf g_ErrorHandlerEnv;

Dataflow *g_BoolFalse = NULL;
Dataflow *g_BoolTrue = NULL;
Dataflow* g_IntZero = NULL;
Dataflow* g_IntOne = NULL;
Dataflow* g_FloatZero = NULL;
Dataflow* g_FloatOne = NULL;

Symbol* g_BuiltInVar__gl_Position;
Symbol* g_BuiltInVar__gl_PointSize;

Symbol* g_BuiltInVar__gl_FragCoord;
Symbol* g_BuiltInVar__gl_FrontFacing;
Symbol* g_BuiltInVar__gl_FragColor;
Symbol* g_BuiltInVar__gl_FragData;
Symbol* g_BuiltInVar__gl_PointCoord;

Symbol* g_BuiltInVar__discard;

Symbol* g_BuiltInVar__gl_MaxVertexAttribs;
Symbol* g_BuiltInVar__gl_MaxVertexUniformVectors;
Symbol* g_BuiltInVar__gl_MaxVaryingVectors;
Symbol* g_BuiltInVar__gl_MaxVertexTextureImageUnits;
Symbol* g_BuiltInVar__gl_MaxCombinedTextureImageUnits;
Symbol* g_BuiltInVar__gl_MaxTextureImageUnits;
Symbol* g_BuiltInVar__gl_MaxFragmentUniformVectors;
Symbol* g_BuiltInVar__gl_MaxDrawBuffers;

Symbol* g_BuiltInType__gl_DepthRangeParameters;
Symbol* g_BuiltInVar__gl_DepthRange;

/*=============================================================================
Broadcom Proprietary and Confidential. (c)2011 Broadcom.
All rights reserved.

Project  :  PPP
Module   :  MMM

FILE DESCRIPTION
DESC
=============================================================================*/

#ifndef __CONTROL_H__
#define __CONTROL_H__

#include <stdint.h>

class Packet;

class Control
{
public:
   enum eControlAction
   {
      eDone,
      eGetGLBuffer,
      eGetUniforms,
      eDisable,
      eEnable,
      eDisconnect,
      eBottleneck,
      eGetPerfData,
      eChangePort,
      eSkip,
      eGetState,
      eGetMemory,
      eSetVarSet,
      eChangeShader,
      eGetEventData,
      eBackTrace,
      eGetBufferObjectData,
      eGetPIQData,
      eGetSyncObjData,
      eGetQueryObjData,
      eGetVertexArrayObjData,
      eGetProgramPipelineData,
      eGetInfoData,
      eGetFramebufferInfo,
      eGetRenderbufferInfo
   };

   enum ePerfAction
   {
      ePerfStart,
      ePerfStop,
      ePerfGet,
      ePerfNames,
      ePerfChoose
   };

   enum eEventAction
   {
      eEventGet,
      eEventNames
   };

   enum eBottleneckMode  // DO NOT CHANGE THESE ENUM VALUES, ADD MORE IF REQUIRED
   {
      eUnset             = 0xFFFFFFFF,
      eNone              = 0,
      eTinyTextures      = 1,
      eTinyViewport      = 2,
      eMinimalFragShader = 4,
      eNullDrawCalls     = 8,
      eOverdraw          = 16
   };
};

#endif /* __CONTROL_H__ */

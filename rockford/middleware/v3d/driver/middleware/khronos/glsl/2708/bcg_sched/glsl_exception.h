/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  BCG's scheduler

FILE DESCRIPTION

=============================================================================*/

#ifndef __GLSL_EXCEPTION_H__
#define __GLSL_EXCEPTION_H__

#include <stdio.h>

typedef enum
{
   eExceptionNone,
   eExceptionBadRegister,
   eExceptionRegfileWriteClash,
   eExceptionRegfileReadClash,
   eExceptionRegfileBCannotBeUsed,
   eExceptionInvalidSmallImmediate,
   eExceptionPackUnpackClash,
   eExceptionOnlyOneImmediateAllowed,
   eExceptionSpecialRegWriteClash,
   eExceptionUnpackClash,
   eExceptionSignalRegClash,
   eExceptionTMUUniformClash
} Exception;

static void PrintException(Exception e)
{
   switch (e)
   {
   case eExceptionBadRegister : printf("Exception : eExceptionBadRegister\n"); break;
   case eExceptionRegfileWriteClash : printf("Exception : eExceptionRegfileWriteClash\n"); break;
   case eExceptionRegfileReadClash : printf("Exception : eExceptionRegfileReadClash\n"); break;
   case eExceptionRegfileBCannotBeUsed : printf("Exception : eExceptionRegfileBCannotBeUsed\n"); break;
   case eExceptionInvalidSmallImmediate : printf("Exception : eExceptionInvalidSmallImmediate\n"); break;
   case eExceptionPackUnpackClash : printf("Exception : eExceptionPackUnpackClash\n"); break;
   case eExceptionOnlyOneImmediateAllowed : printf("Exception : eExceptionOnlyOneImmediateAllowed\n"); break;
   case eExceptionUnpackClash : printf("Exception : eExceptionUnpackClash\n"); break;
   case eExceptionSignalRegClash : printf("Exception : eExceptionSignalRegClash\n"); break;
   case eExceptionTMUUniformClash : printf("Exception : eExceptionTMUUniformClash\n"); break;
   default: printf("Exception : Unknown\n"); break;
   }
}

#endif /* __GLSL_EXCEPTION_H__ */

/******************************************************************************
 *    (c)2008-2010 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *****************************************************************************/

#include <malloc.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <memory.h>
#include <math.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "esutil.h"
#include "load_texture.h"
#include "default_linuxfb.h"

#include "../common/init.h"

typedef struct
{
   bool  useMultisample;
   bool  usePreservingSwap;
   bool  stretchToFit;
   int   x;
   int   y;
   int   vpW;
   int   vpH;
   int   bpp;
   unsigned clientId;
} AppConfig;

const unsigned int WIDTH             = 720;
const unsigned int HEIGHT            = 480;
const unsigned int BPP               = 16;
const GLenum       PLANET_TEX_FILTER = GL_LINEAR_MIPMAP_NEAREST;

NEXUS_DISPLAYHANDLE nexus_display = 0;

EGLNativeDisplayType native_display = 0;
void                 *native_window = 0;

LFPL_PlatformHandle lfpl_handle = 0;

/***************************************************************************/
typedef struct GLAPP_Vertex
{
   GLfloat nx, ny, nz;
   GLfloat x, y, z;
   GLfloat s, t;
}
GLAPP_Vertex;

typedef struct GLAPP_StarVertex
{
   GLfloat x, y, z;
   GLfloat lum;
}
GLAPP_StarVertex;

typedef struct GLAPP_Object
{
   GLfloat fPositionX;
   GLfloat fPositionY;
   GLfloat fPositionZ;
   GLfloat fVelocityX;
   GLfloat fVelocityY;
   GLfloat fVelocityZ;
   GLfloat fRotation;
   GLfloat fMass;
   GLfloat fTilt;
   GLfloat fAngle;
   GLfloat fAngleFacX;
   GLfloat fAngleFacY;
   GLfloat fAngleFacZ;
   GLfloat fScale;
   GLuint  uiVertexCount;
   GLuint  uiTextureID;
   GLuint  uiVBO;
   GLenum  eType;
}
GLAPP_Object;

GLAPP_Object *pCamera     = NULL;
GLAPP_Object *pStars      = NULL;
GLAPP_Object *pEarthLand  = NULL;
GLAPP_Object *pEarthCloud = NULL;
GLAPP_Object *pMoon       = NULL;

/***************************************************************************/
GLint positionLoc_nolight;
GLint lumCoordLoc_nolight;
GLint mvpMatrixLoc_nolight;
GLint programObject_nolight;

GLfloat light_pos[4];

GLint positionLoc_light;
GLint texCoordLoc_light;
GLint normalLoc_light;
GLint imvMatrixLoc_light;
GLint mvpMatrixLoc_light;
GLint ambColorLoc_light;
GLint diffColorLoc_light;
GLint lightPosLoc_light;
GLint textureLoc_light;
GLint programObject_light;

ESMatrix projection_matrix;
ESMatrix modelview_matrix;

AppConfig config;

#define OBJECT_COMPLEXITY    42

/***************************************************************************/
GLuint auiTextureID[3];

#define FSCALE        (10.0f)
#define DS            (10.0f * FSCALE)
#define SEC_PER_YEAR  (256.0f)
#define MOVE_PER_SEC  (20.0f)
#define VS            (DS * 365.26f * 24 * 60 * 60 / 1000000 / MOVE_PER_SEC / SEC_PER_YEAR)
#define MS            (66.72f * DS * DS * DS / SEC_PER_YEAR / SEC_PER_YEAR / MOVE_PER_SEC / MOVE_PER_SEC)

/***************************************************************************/
GLint        CreateSphere(GLAPP_Vertex * vb, GLfloat fX, GLfloat fY, GLfloat fZ, GLfloat fR, GLint iN, bool bFace);
GLint        CreateStarfield(GLAPP_StarVertex * vb, GLfloat fX, GLfloat fY, GLfloat fZ, GLfloat fR, GLint iN, bool bFace, GLfloat fRandFactor);
GLAPP_Object *GLAPP_CreateSphere(GLfloat fRadius, GLint iComplexity, bool face);
GLAPP_Object *GLAPP_CreateStarfield(GLfloat fRadius, GLint iComplexity, bool face, GLfloat fRandFactor);
GLAPP_Object *GLAPP_CreateCamera(void);
void         GLAPP_DestroyObject(GLAPP_Object *pObject);
void         GLAPP_MoveObject(GLAPP_Object *pObject, GLfloat fMass, GLfloat fPositionX, GLfloat fPositionY, GLfloat fPositionZ);
void         GLAPP_DrawObject(ESMatrix * current_modelview_matrix, GLAPP_Object *pObject, GLboolean bLighting, GLboolean bBlend, GLboolean bDepthTest, GLenum eTexFilter);
GLfloat      GLAPP_GetCameraAngle(GLfloat fDistanceX, GLfloat fDistanceZ);

/***************************************************************************/

GLint CreateSphere(
   GLAPP_Vertex *pVb,
   GLfloat fX,
   GLfloat fY,
   GLfloat fZ,
   GLfloat fR,
   GLint   iN,
   bool    bFace)
{
   GLint nn = 0;
   GLint ii, jj;

   for (jj = 0; jj < iN / 2; ++jj)
   {
      GLfloat fTheta1 = (float)((jj * M_PI * 2) / iN - M_PI / 2);
      GLfloat fTheta2 = (float)(((jj + 1) * M_PI * 2) / iN - M_PI / 2);

      /* add the additional geometry to make the triangle degenerate
       * this is defined as repeat last, newA, repeat newA, newB
         */
      if (nn > 0)
      {
         /* structure copy */
         pVb[nn  ] = pVb[nn-1];

         /* newA */
         pVb[nn+1].nx = cosf(bFace ? fTheta1 : fTheta2) * cosf(0.0f);
         pVb[nn+1].ny = sinf(bFace ? fTheta1 : fTheta2);
         pVb[nn+1].nz = cosf(bFace ? fTheta1 : fTheta2) * sinf(0.0f);
         pVb[nn+1].x  = fX + fR * pVb[nn+1].nx;
         pVb[nn+1].y  = fY + fR * pVb[nn+1].ny;
         pVb[nn+1].z  = fZ + fR * pVb[nn+1].nz;
         pVb[nn+1].s  = 1.0f;
         pVb[nn+1].t  = 1.0f - fabs((((GLfloat) jj + (bFace ? 0 : 1)) * 2) / iN);

         /* newA, just use structure copy */
         pVb[nn+2] = pVb[nn+1];

         /* newB */
         pVb[nn+3].nx = cosf(bFace ? fTheta2 : fTheta1) * cosf(0.0f);
         pVb[nn+3].ny = sinf(bFace ? fTheta2 : fTheta1);
         pVb[nn+3].nz = cosf(bFace ? fTheta2 : fTheta1) * sinf(0.0f);
         pVb[nn+3].x  = fX + fR * pVb[nn+3].nx;
         pVb[nn+3].y  = fY + fR * pVb[nn+3].ny;
         pVb[nn+3].z  = fZ + fR * pVb[nn+3].nz;
         pVb[nn+3].s  = 1.0f;
         pVb[nn+3].t  = 1.0f - fabs((((GLfloat) jj + (bFace ? 1 : 0)) * 2) / iN);

         nn += 4;
      }

      for (ii = 0; ii <= iN; ++ii)
      {
         GLfloat fTheta3 = (float)((ii * M_PI * 2) / iN);

         pVb[nn  ].nx = cosf(bFace ? fTheta1 : fTheta2) * cosf(fTheta3);
         pVb[nn  ].ny = sinf(bFace ? fTheta1 : fTheta2);
         pVb[nn  ].nz = cosf(bFace ? fTheta1 : fTheta2) * sinf(fTheta3);
         pVb[nn  ].x  = fX + fR * pVb[nn  ].nx;
         pVb[nn  ].y  = fY + fR * pVb[nn  ].ny;
         pVb[nn  ].z  = fZ + fR * pVb[nn  ].nz;
         pVb[nn  ].s  = 1.0f - fabs((GLfloat) -ii / iN);
         pVb[nn  ].t  = 1.0f - fabs((((GLfloat) jj + (bFace ? 0 : 1)) * 2) / iN);

         pVb[nn+1].nx = cosf(bFace ? fTheta2 : fTheta1) * cosf(fTheta3);
         pVb[nn+1].ny = sinf(bFace ? fTheta2 : fTheta1);
         pVb[nn+1].nz = cosf(bFace ? fTheta2 : fTheta1) * sinf(fTheta3);
         pVb[nn+1].x  = fX + fR * pVb[nn+1].nx;
         pVb[nn+1].y  = fY + fR * pVb[nn+1].ny;
         pVb[nn+1].z  = fZ + fR * pVb[nn+1].nz;
         pVb[nn+1].s  = 1.0f - fabs((GLfloat) -  ii / iN);
         pVb[nn+1].t  = 1.0f - fabs((((GLfloat) jj + (bFace ? 1 : 0)) * 2) / iN);

         nn += 2;
      }
   }

   return nn;
}

GLint CreateStarfield(
   GLAPP_StarVertex *pVb,
   GLfloat fX,
   GLfloat fY,
   GLfloat fZ,
   GLfloat fR,
   GLint   iN,
   bool    bFace,
   GLfloat fRandFactor)
{
   GLint nn = 0;
   GLint ii, jj;
   GLfloat randT1, randT2, randT3;
   GLfloat nx, ny, nz;

   for (jj = 0; jj < iN / 2; ++jj)
   {
      GLfloat fTheta1 = (float)((jj * M_PI * 2) / iN - M_PI / 2);
      GLfloat fTheta2 = (float)(((jj + 1) * M_PI * 2) / iN - M_PI / 2);

      randT1 = fRandFactor * (float)rand() / (float)RAND_MAX;
      randT2 = fRandFactor * (float)rand() / (float)RAND_MAX;

      fTheta1 += randT1;
      fTheta2 += randT2;

      /* add the additional geometry to make the triangle degenerate
       * this is defined as repeat last, newA, repeat newA, newB
         */
      if (nn > 0)
      {
         /* structure copy */
         pVb[nn  ] = pVb[nn-1];

         /* newA */
         nx = cosf(bFace ? fTheta1 : fTheta2) * cosf(0.0f);
         ny = sinf(bFace ? fTheta1 : fTheta2);
         nz = cosf(bFace ? fTheta1 : fTheta2) * sinf(0.0f);
         pVb[nn+1].x  = fX + fR * nx;
         pVb[nn+1].y  = fY + fR * ny;
         pVb[nn+1].z  = fZ + fR * nz;
         pVb[nn+1].lum = (float)rand() / (float)RAND_MAX;

         /* newA, just use structure copy */
         pVb[nn+2] = pVb[nn+1];

         /* newB */
         nx = cosf(bFace ? fTheta2 : fTheta1) * cosf(0.0f);
         ny = sinf(bFace ? fTheta2 : fTheta1);
         nz = cosf(bFace ? fTheta2 : fTheta1) * sinf(0.0f);
         pVb[nn+3].x  = fX + fR * nx;
         pVb[nn+3].y  = fY + fR * ny;
         pVb[nn+3].z  = fZ + fR * nz;
         pVb[nn+3].lum = (float)rand() / (float)RAND_MAX;

         nn += 4;
      }

      for (ii = 0; ii <= iN; ++ii)
      {
         GLfloat fTheta3 = (float)((ii * M_PI * 2) / iN);

         randT1 = fRandFactor * (float)rand() / (float)RAND_MAX;
         randT2 = fRandFactor * (float)rand() / (float)RAND_MAX;
         randT3 = fRandFactor * (float)rand() / (float)RAND_MAX;

         fTheta1 += randT1;
         fTheta2 += randT2;
         fTheta3 += randT3;

         nx = cosf(bFace ? fTheta1 : fTheta2) * cosf(fTheta3);
         ny = sinf(bFace ? fTheta1 : fTheta2);
         nz = cosf(bFace ? fTheta1 : fTheta2) * sinf(fTheta3);
         pVb[nn  ].x  = fX + fR * nx;
         pVb[nn  ].y  = fY + fR * ny;
         pVb[nn  ].z  = fZ + fR * nz;
         pVb[nn  ].lum = (float)rand() / (float)RAND_MAX;

         nx = cosf(bFace ? fTheta2 : fTheta1) * cosf(fTheta3);
         ny = sinf(bFace ? fTheta2 : fTheta1);
         nz = cosf(bFace ? fTheta2 : fTheta1) * sinf(fTheta3);
         pVb[nn+1].x  = fX + fR * nx;
         pVb[nn+1].y  = fY + fR * ny;
         pVb[nn+1].z  = fZ + fR * nz;
         pVb[nn+1].lum = (float)rand() / (float)RAND_MAX;

         nn += 2;
      }
   }

   return nn;
}


/***************************************************************************/
GLAPP_Object *GLAPP_CreateSphere(
   GLfloat fRadius,
   GLint iComplexity,
   bool bFace)
{
   GLAPP_Object *pObject   = NULL;
   GLAPP_Vertex *pVertices = NULL;
   GLint        uiVertexCount;
   GLsizeiptr   vertexArraySize;

    /* if the complexity is 24 then we need (24+1) * 2 verts to make a complete ring
    * the height has been divided by two so 12 rings will make up the sphere
    * an additional 4 verts are required at the END of each real strip to make the strip degenerate so that a single
    * draw call can be used to draw the whole sphere
    *
    * ((iComplexity / 2) = number of rings
    * ((iComplexity+1) * 2)) = number of verts in a ring
    * (((iComplexity / 2) - 1) * 4) = additional verts required for degenerate triangles.  This is (number of rings - 1)*4
     */
   uiVertexCount = ((iComplexity / 2) * ((iComplexity+1) * 2)) + (((iComplexity / 2) - 1) * 4);
   vertexArraySize = uiVertexCount * sizeof(GLAPP_Vertex);

   pObject   = (GLAPP_Object *)malloc(sizeof(GLAPP_Object));
   pVertices = (GLAPP_Vertex *)malloc(vertexArraySize);

   if (pObject == NULL || pVertices == NULL)
   {
      free(pObject);
      free(pVertices);
      return NULL;
   }

   memset(pObject, 0, sizeof (GLAPP_Object));
   pObject->uiVertexCount = uiVertexCount;

   CreateSphere(pVertices, 0.0, 0.0, 0.0, fRadius, iComplexity, bFace);

   /* Create buffer object to hold vertex data */
   glGenBuffers(1, &pObject->uiVBO);

   if (pObject->uiVBO > 0)
   {
      glBindBuffer(GL_ARRAY_BUFFER, pObject->uiVBO);
      glBufferData(GL_ARRAY_BUFFER, vertexArraySize, pVertices, GL_STATIC_DRAW);
   }
   else
   {
      free(pObject);
      pObject = NULL;
   }

   /* Clean up */
   free(pVertices);

   return pObject;
}

/***************************************************************************/
GLAPP_Object *GLAPP_CreateStarfield(
   GLfloat fRadius,
   GLint iComplexity,
   bool bFace,
   GLfloat fRandFactor)
{
   GLAPP_Object     *pObject   = NULL;
   GLAPP_StarVertex *pVertices = NULL;
   GLint             uiVertexCount;
   GLsizeiptr        vertexArraySize;

    /* if the complexity is 24 then we need (24+1) * 2 verts to make a complete ring
    * the height has been divided by two so 12 rings will make up the sphere
    * an additional 4 verts are required at the END of each real strip to make the strip degenerate so that a single
    * draw call can be used to draw the whole sphere
    *
    * ((iComplexity / 2) = number of rings
    * ((iComplexity+1) * 2)) = number of verts in a ring
    * (((iComplexity / 2) - 1) * 4) = additional verts required for degenerate triangles.  This is (number of rings - 1)*4
     */
   uiVertexCount = ((iComplexity / 2) * ((iComplexity+1) * 2)) + (((iComplexity / 2) - 1) * 4);
   vertexArraySize = uiVertexCount * sizeof(GLAPP_StarVertex);

   pObject   = (GLAPP_Object *)malloc(sizeof(GLAPP_Object));
   pVertices = (GLAPP_StarVertex *)malloc(vertexArraySize);

   if (pObject == NULL || pVertices == NULL)
   {
      free(pObject);
      free(pVertices);
      return NULL;
   }

   memset(pObject, 0, sizeof (GLAPP_Object));
   pObject->uiVertexCount = uiVertexCount;

   CreateStarfield(pVertices, 0.0, 0.0, 0.0, fRadius, iComplexity, bFace, fRandFactor);

   /* Create buffer object to hold vertex data */
   glGenBuffers(1, &pObject->uiVBO);

   if (pObject->uiVBO > 0)
   {
      glBindBuffer(GL_ARRAY_BUFFER, pObject->uiVBO);
      glBufferData(GL_ARRAY_BUFFER, vertexArraySize, pVertices, GL_STATIC_DRAW);
   }
   else
   {
      free(pObject);
      pObject = NULL;
   }

   /* Clean up */
   free(pVertices);

   return pObject;
}

/***************************************************************************/
GLAPP_Object *GLAPP_CreateCamera(void)
{
   GLAPP_Object *pObject;

   pObject = (GLAPP_Object *)calloc(1, sizeof(GLAPP_Object));
    /* returns NULL on failure */
   return pObject;
}

/***************************************************************************/
void GLAPP_DestroyObject(GLAPP_Object *pObject)
{
   if (pObject != NULL)
   {
      if (pObject->uiVBO != 0)
         glDeleteBuffers(1, &pObject->uiVBO);
      free(pObject);
}
}

/***************************************************************************/
void GLAPP_MoveObject(
   GLAPP_Object *pObject,
   GLfloat fMass,
   GLfloat fPositionX,
   GLfloat fPositionY,
   GLfloat fPositionZ)
{
   GLfloat fDistanceX   = fPositionX - pObject->fPositionX;
   GLfloat fDistanceY   = fPositionY - pObject->fPositionY;
   GLfloat fDistanceZ   = fPositionZ - pObject->fPositionZ;
   GLfloat fDistanceXYZ = sqrtf(fDistanceX * fDistanceX + fDistanceY * fDistanceY + fDistanceZ * fDistanceZ);
   GLfloat fDistanceXZ  = sqrtf(fDistanceX * fDistanceX + fDistanceZ * fDistanceZ);
   GLfloat fAngleXYZ    = (fDistanceXYZ != 0.0f) ? acosf(fabs(fDistanceXZ / fDistanceXYZ)) : 1.5707963f;
   GLfloat fAngleXZ     = (fDistanceX != 0.0f) ? atanf(fabs(fDistanceZ / fDistanceX)) : 1.5707963f;
   GLfloat fForceXYZ    = fMass / ((fDistanceXYZ != 0.0) ? (fDistanceXYZ * fDistanceXYZ) : 1.0f);
   GLfloat fForceXZ     = fForceXYZ * cosf(fAngleXYZ);
   GLfloat fForceX      = fForceXZ * cosf(fAngleXZ);
   GLfloat fForceY      = fForceXYZ * sinf(fAngleXYZ);
   GLfloat fForceZ      = fForceXZ * sinf(fAngleXZ);

   pObject->fVelocityX += ((fDistanceX < 0.0f) ? -1 : 1) * fForceX;
   pObject->fVelocityY += ((fDistanceY < 0.0f) ? -1 : 1) * fForceY;
   pObject->fVelocityZ += ((fDistanceZ < 0.0f) ? -1 : 1) * fForceZ;

   pObject->fPositionX += pObject->fVelocityX;
   pObject->fPositionY += pObject->fVelocityY;
   pObject->fPositionZ += pObject->fVelocityZ;
   pObject->fAngle     += pObject->fRotation;
}

/***************************************************************************/
void GLAPP_DrawObject(
   ESMatrix     *current_modelview_matrix,
   GLAPP_Object *pObject,
   GLboolean    bLighting,
   GLboolean    bBlend,
   GLboolean    bDepthTest,
   GLenum       eTexFilter)
{
   /* matrix copy, assumes the object is effecting the current modelview matrix */
   ESMatrix tmp = *current_modelview_matrix;
   ESMatrix mvpMatrix;

   esTranslate(&tmp, pObject->fPositionX, pObject->fPositionY, pObject->fPositionZ);
   esRotate(&tmp, pObject->fTilt, 1.0f, 0.0f, 0.0f);
   esRotate(&tmp, pObject->fAngle, pObject->fAngleFacX, pObject->fAngleFacY, pObject->fAngleFacZ);

   /* Compute the final MVP by multiplying the
    * modelview and perspective matrices together
    */
   esMatrixMultiply(&mvpMatrix, &tmp, &projection_matrix);

   if (bBlend)
      glEnable(GL_BLEND);
   else
      glDisable(GL_BLEND);

   if (bDepthTest)
      glEnable(GL_DEPTH_TEST);
   else
      glDisable(GL_DEPTH_TEST);

   glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);

   if (bLighting)
   {
      /* compute the inverse of the modelview_matrix */
      ESMatrix    tmp_inv;
      ESMatrix3   inv_modelview_matrix;

      esInverse(&tmp, &tmp_inv);

      /* cut the top 3x3 out of the 4x4 matrix */
      inv_modelview_matrix.m[0][0] = tmp_inv.m[0][0];
      inv_modelview_matrix.m[0][1] = tmp_inv.m[0][1];
      inv_modelview_matrix.m[0][2] = tmp_inv.m[0][2];
      inv_modelview_matrix.m[1][0] = tmp_inv.m[1][0];
      inv_modelview_matrix.m[1][1] = tmp_inv.m[1][1];
      inv_modelview_matrix.m[1][2] = tmp_inv.m[1][2];
      inv_modelview_matrix.m[2][0] = tmp_inv.m[2][0];
      inv_modelview_matrix.m[2][1] = tmp_inv.m[2][1];
      inv_modelview_matrix.m[2][2] = tmp_inv.m[2][2];

      glUseProgram(programObject_light);

      glBindBuffer(GL_ARRAY_BUFFER, pObject->uiVBO);

      glVertexAttribPointer(positionLoc_light, 3, GL_FLOAT,
                       GL_FALSE, sizeof(GLAPP_Vertex), (void*)offsetof(GLAPP_Vertex, x));
      glVertexAttribPointer(normalLoc_light, 3, GL_FLOAT,
                       GL_FALSE, sizeof(GLAPP_Vertex), (void*)offsetof(GLAPP_Vertex, nx));
      glVertexAttribPointer(texCoordLoc_light, 2, GL_FLOAT,
                       GL_FALSE, sizeof(GLAPP_Vertex), (void*)offsetof(GLAPP_Vertex, s));
      glEnableVertexAttribArray(positionLoc_light);
      glEnableVertexAttribArray(texCoordLoc_light);
      glEnableVertexAttribArray(normalLoc_light);

      /* Load the MVP matrix */
      glUniformMatrix4fv(mvpMatrixLoc_light, 1, GL_FALSE, (GLfloat*) &mvpMatrix.m[0][0]);

      /* Load the inverse mv matrix */
      glUniformMatrix3fv(imvMatrixLoc_light, 1, GL_FALSE, (GLfloat*) &inv_modelview_matrix.m[0][0]);

      /* Load the lighting information (colors etc) */
      glUniform3f(diffColorLoc_light,      0.8f, 0.8f, 0.8f);
      glUniform3f(ambColorLoc_light,       0.05f, 0.05f, 0.05f);

      /* send the normalized light position */
      glUniform3fv(lightPosLoc_light, 1, light_pos);

      glBindTexture(GL_TEXTURE_2D, pObject->uiTextureID);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, eTexFilter);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glUniform1i(textureLoc_light, 0);
   }
   else
   {
      glUseProgram(programObject_nolight);

      glBindBuffer(GL_ARRAY_BUFFER, pObject->uiVBO);

      glVertexAttribPointer(positionLoc_nolight, 3, GL_FLOAT,
                       GL_FALSE, sizeof(GLAPP_StarVertex), (void*)offsetof(GLAPP_StarVertex, x));
      glVertexAttribPointer(lumCoordLoc_nolight, 1, GL_FLOAT,
                       GL_FALSE, sizeof(GLAPP_StarVertex), (void*)offsetof(GLAPP_StarVertex, lum));
      glEnableVertexAttribArray(positionLoc_nolight);
      glEnableVertexAttribArray(lumCoordLoc_nolight);

      /* Load the MVP matrix */
      glUniformMatrix4fv(mvpMatrixLoc_nolight, 1, GL_FALSE, (GLfloat*) &mvpMatrix.m[0][0]);
   }

   glDrawArrays(pObject->eType, 0, pObject->uiVertexCount);

   if (bLighting)
   {
      glDisableVertexAttribArray(positionLoc_light);
      glDisableVertexAttribArray(texCoordLoc_light);
      glDisableVertexAttribArray(normalLoc_light);
   }
   else
   {
      glDisableVertexAttribArray(positionLoc_nolight);
      glDisableVertexAttribArray(lumCoordLoc_nolight);
   }
}

/***************************************************************************/
GLfloat GLAPP_GetCameraAngle(
   GLfloat fDistanceX,
   GLfloat fDistanceZ)
{
   GLfloat fDistance = sqrtf(fDistanceZ * fDistanceZ + fDistanceX * fDistanceX);
   GLfloat fAngle    = asinf(fDistanceX / fDistance) * 180.0f / (GLfloat)M_PI;

    if (fDistanceZ > 0.0f)
      fAngle = 180.0f - fAngle;

    return fAngle;
}

/***************************************************************************
 * SHADER code
 *
 ***************************************************************************/

/* Without light (star field) */
const char vShaderStr_nolight[] =
   "uniform mat4 u_mvpMatrix;                           \n"
   "attribute vec4 a_position;                          \n"
   "attribute float a_luminance;                        \n"
   "varying float v_luminance;                          \n"
   "void main()                                         \n"
   "{                                                   \n"
   "  gl_Position = u_mvpMatrix * a_position;           \n"
   "  v_luminance = a_luminance;                        \n"
   "  gl_PointSize = 3.0;                               \n"
   "}                                                   \n";

/* Draw a graded circle inside the point */
const char fShaderStr_nolight[] =
   "precision mediump float;                               \n"
   "varying float v_luminance;                             \n"
   "void main()                                            \n"
   "{                                                      \n"
   "  float d = length(gl_PointCoord - vec2(0.5));         \n"
   "  d = 1.0 - min(2.0 * d, 1.0);                         \n"
   "                                                       \n"
   "  gl_FragColor.rgb = vec3(d * v_luminance);            \n"
   "  gl_FragColor.a   = 1.0;                              \n"
   "}                                                      \n";

const char vShaderStr_light[] =
   "uniform mat4   u_mvpMatrix;                          \n"
   "uniform mat3   u_imvMatrix;                          \n"
   "uniform vec3   u_ambColor;                           \n"
   "uniform vec3   u_diffColor;                          \n"
   "uniform vec3   u_lightPos;                           \n"
   "                                                     \n"
   "attribute vec4 a_position;                           \n"
   "attribute vec2 a_texCoord;                           \n"
   "attribute vec3 a_normal;                             \n"
   "                                                     \n"
   "varying vec2   v_texCoord;                           \n"
   "varying vec3   v_color;                              \n"
   "                                                     \n"
   "void main()                                          \n"
   "{                                                    \n"
   "  vec3  color  = u_ambColor;                         \n"
   "  vec3  normal = u_imvMatrix * a_normal;             \n"
   "  float ndotl  = dot(normal, u_lightPos);            \n"
   "                                                     \n"
   "  if (ndotl > 0.0)                                   \n"
   "    color += ndotl * u_diffColor;                    \n"
   "                                                     \n"
   "  gl_Position = u_mvpMatrix * a_position;            \n"
   "                                                     \n"
   "  v_color     = color;                               \n"
   "  v_texCoord  = a_texCoord;                          \n"
   "}                                                    \n";

const char fShaderStr_light[] =
   "precision mediump float;                            \n"
   "                                                    \n"
   "varying vec2 v_texCoord;                            \n"
   "varying vec3 v_color;                               \n"
   "                                                    \n"
   "uniform sampler2D s_texture0;                       \n"
   "                                                    \n"
   "void main()                                         \n"
   "{                                                   \n"
   "  vec4 tex = texture2D(s_texture0, v_texCoord);     \n"
   "                                                    \n"
   "  gl_FragColor.rgb = v_color * tex.rgb;             \n"
   "  gl_FragColor.a   = tex.r;                         \n"  /* Hack to allow compressed textures to be used.        */
   "}                                                   \n"; /* They have no alpha, so use the red channel instead - */
                                                             /* This will only affect the clouds, as nothing else is */
                                                             /* alpha blended.                                       */

/* CheckCompile, CheckLink,
 * GetAttribLocation, GetUniformLocation
 *
 * Helper functions for compilation process
 */
static void CheckCompile(const char *name, GLint shader)
{
   GLint ret;
   char  *err;

   glGetShaderiv(shader, GL_COMPILE_STATUS, &ret);
   if (ret == GL_FALSE)
   {
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &ret);
      err = (char *)alloca(ret);
      glGetShaderInfoLog(shader, ret, NULL, err);
      printf("Shader '%s' compile failed: %s\n", name, err);
      assert(0);
   }
}

static void CheckLink(const char *name, GLint program)
{
   GLint ret;
   char  *err;

   glGetProgramiv(program, GL_LINK_STATUS, &ret);
   if (ret == GL_FALSE)
   {
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &ret);
      err = (char *)alloca(ret);
      glGetProgramInfoLog(program, ret, NULL, err);
      printf("Program '%s' link failed: %s\n", name, err);
      assert(0);
   }
}

static GLint GetAttribLocation(GLuint program, const char *name)
{
   GLint loc = glGetAttribLocation(program, name);

   if (loc < 0)
   {
      printf("Attribute '%s' not found\n", name);
      assert(0);
   }

   return loc;
}

static GLint GetUniformLocation(GLint program, const char *name)
{
   GLint loc = glGetUniformLocation(program, name);

   if (loc < 0)
   {
      printf("Uniform '%s' not found\n", name);
      assert(0);
   }

   return loc;
}

static void ShaderSource(GLuint shader, const char *src)
{
   glShaderSource(shader, 1, &src, NULL);
}

/* InitShaders
 *
 * Compile shaders and link
 */
void InitShaders(void)
{
   GLuint      v_nolight;
   GLuint      f_nolight;
   GLuint      v_light;
   GLuint      f_light;

    /* No-light (starfield) shaders */
   v_nolight = glCreateShader(GL_VERTEX_SHADER);
   f_nolight = glCreateShader(GL_FRAGMENT_SHADER);

   ShaderSource(v_nolight, vShaderStr_nolight);
   ShaderSource(f_nolight, fShaderStr_nolight);

   glCompileShader(v_nolight);
   CheckCompile("no-light vertex", v_nolight);

   glCompileShader(f_nolight);
   CheckCompile("no-light fragment", f_nolight);

   programObject_nolight = glCreateProgram();
   glAttachShader(programObject_nolight, v_nolight);
   glAttachShader(programObject_nolight, f_nolight);

   glLinkProgram(programObject_nolight);
   CheckLink("no-light", programObject_nolight);

   /* Get the attribute locations */
   positionLoc_nolight = GetAttribLocation(programObject_nolight, "a_position");
   lumCoordLoc_nolight = GetAttribLocation(programObject_nolight, "a_luminance");

   /* Get the uniform locations */
   mvpMatrixLoc_nolight = GetUniformLocation(programObject_nolight, "u_mvpMatrix");

   /* load the main shader */
   v_light = glCreateShader(GL_VERTEX_SHADER);
   f_light = glCreateShader(GL_FRAGMENT_SHADER);

   ShaderSource(v_light, vShaderStr_light);
   ShaderSource(f_light, fShaderStr_light);

   glCompileShader(v_light);
   CheckCompile("light vertex", v_light);

   glCompileShader(f_light);
   CheckCompile("light fragment", f_light);

   programObject_light = glCreateProgram();
   glAttachShader(programObject_light, v_light);
   glAttachShader(programObject_light, f_light);

   glLinkProgram(programObject_light);
   CheckLink("light", programObject_light);

   /* Get the attribute locations */
   positionLoc_light = glGetAttribLocation(programObject_light, "a_position");
   texCoordLoc_light = glGetAttribLocation(programObject_light, "a_texCoord");
   normalLoc_light   = glGetAttribLocation(programObject_light, "a_normal");

   /* Get the uniform locations */
   mvpMatrixLoc_light = glGetUniformLocation(programObject_light, "u_mvpMatrix");
   imvMatrixLoc_light = glGetUniformLocation(programObject_light, "u_imvMatrix");
   diffColorLoc_light = glGetUniformLocation(programObject_light, "u_diffColor");
   ambColorLoc_light  = glGetUniformLocation(programObject_light, "u_ambColor");
   lightPosLoc_light  = glGetUniformLocation(programObject_light, "u_lightPos");
   textureLoc_light   = glGetUniformLocation(programObject_light, "s_texture0");
}

void InitGLState(void)
{
   unsigned int load_res;
   unsigned int level = 0;

   pMoon = GLAPP_CreateSphere(0.7f * 0.75f * FSCALE, OBJECT_COMPLEXITY, true);
   if (pMoon == NULL)
      return;

   pMoon->fMass      =  0.07349f * MS;
   pMoon->fPositionZ = -0.38440f * DS;
   pMoon->fVelocityX =  1.02300f * VS;
   pMoon->fRotation  = -1.00000f * VS;
   pMoon->fAngleFacY =  1.0f;
   pMoon->fScale     =  1.0f;
   pMoon->eType      = GL_TRIANGLE_STRIP;

   pEarthLand = GLAPP_CreateSphere(0.7f * 1.92f * FSCALE, OBJECT_COMPLEXITY, true);
   if (pEarthLand == NULL)
      return;

   pEarthLand->fMass      =  5.97360f * MS;
   pEarthLand->fRotation  =  0.50000f * VS;
   pEarthLand->fAngleFacY =  1.0f;
   pEarthLand->fTilt      =  24.0f;
   pEarthLand->fScale     =  1.0f;
   pEarthLand->eType      = GL_TRIANGLE_STRIP;

   pEarthCloud = GLAPP_CreateSphere(0.7f * 1.94f * FSCALE, OBJECT_COMPLEXITY, true);
   if (pEarthCloud == NULL)
      return;

   pEarthCloud->fRotation  =  0.60000f * VS;
   pEarthCloud->fAngleFacY =  1.0f;
   pEarthCloud->fTilt      =  24.0f;
   pEarthCloud->fScale     =  1.0f;
   pEarthCloud->eType      = GL_TRIANGLE_STRIP;

   pStars = GLAPP_CreateStarfield(14.00f * FSCALE, 60, false, 455.0f * M_PI / 180.0f);
   if (pStars == NULL)
      return;

   pStars->fScale =  1.0f;
   pStars->eType  = GL_POINTS;

   pCamera = GLAPP_CreateCamera();
   if (pCamera == NULL)
      return;

   pCamera->fPositionZ =  1.00000f * DS;
   pCamera->fVelocityX =  0.38500f * VS;
   pCamera->fRotation  =  0.10000f * VS;
   pCamera->fAngleFacZ =  1.0f;

   /* load textures */
   InitTextureLoader();

   glGenTextures(3, auiTextureID);

   glBindTexture(GL_TEXTURE_2D, auiTextureID[0]);
   /* setup a default texture */
   level = 0;
   load_res  = (LoadTexture("textures/moon_0512.pkm", level++) ? 1   : 0);
   load_res |= (LoadTexture("textures/moon_0256.pkm", level++) ? 2   : 0);
   load_res |= (LoadTexture("textures/moon_0128.pkm", level++) ? 4   : 0);
   load_res |= (LoadTexture("textures/moon_0064.pkm", level++) ? 8   : 0);
   load_res |= (LoadTexture("textures/moon_0032.pkm", level++) ? 16  : 0);
   load_res |= (LoadTexture("textures/moon_0016.pkm", level++) ? 32  : 0);
   load_res |= (LoadTexture("textures/moon_0008.pkm", level++) ? 64  : 0);
   load_res |= (LoadTexture("textures/moon_0004.pkm", level++) ? 128 : 0);
   load_res |= (LoadTexture("textures/moon_0002.pkm", level++) ? 256 : 0);
   load_res |= (LoadTexture("textures/moon_0001.pkm", level++) ? 512 : 0);
   pMoon->uiTextureID = auiTextureID[0];
   if (load_res != 0x3FF)
      goto load_failed;

   glBindTexture(GL_TEXTURE_2D, auiTextureID[1]);
   level = 0;
   load_res  = (LoadTexture("textures/earth_land_0512.pkm", level++) ? 1   : 0);
   load_res |= (LoadTexture("textures/earth_land_0256.pkm", level++) ? 2   : 0);
   load_res |= (LoadTexture("textures/earth_land_0128.pkm", level++) ? 4   : 0);
   load_res |= (LoadTexture("textures/earth_land_0064.pkm", level++) ? 8   : 0);
   load_res |= (LoadTexture("textures/earth_land_0032.pkm", level++) ? 16  : 0);
   load_res |= (LoadTexture("textures/earth_land_0016.pkm", level++) ? 32  : 0);
   load_res |= (LoadTexture("textures/earth_land_0008.pkm", level++) ? 64  : 0);
   load_res |= (LoadTexture("textures/earth_land_0004.pkm", level++) ? 128 : 0);
   load_res |= (LoadTexture("textures/earth_land_0002.pkm", level++) ? 256 : 0);
   load_res |= (LoadTexture("textures/earth_land_0001.pkm", level++) ? 512 : 0);
   pEarthLand->uiTextureID = auiTextureID[1];
   if (load_res != 0x3FF)
      goto load_failed;

   glBindTexture(GL_TEXTURE_2D, auiTextureID[2]);
   level = 0;
   load_res =  (LoadTexture("textures/earth_cloud_0512.pkm", level++) ? 1   : 0);
   load_res |= (LoadTexture("textures/earth_cloud_0256.pkm", level++) ? 2   : 0);
   load_res |= (LoadTexture("textures/earth_cloud_0128.pkm", level++) ? 4   : 0);
   load_res |= (LoadTexture("textures/earth_cloud_0064.pkm", level++) ? 8   : 0);
   load_res |= (LoadTexture("textures/earth_cloud_0032.pkm", level++) ? 16  : 0);
   load_res |= (LoadTexture("textures/earth_cloud_0016.pkm", level++) ? 32  : 0);
   load_res |= (LoadTexture("textures/earth_cloud_0008.pkm", level++) ? 64  : 0);
   load_res |= (LoadTexture("textures/earth_cloud_0004.pkm", level++) ? 128 : 0);
   load_res |= (LoadTexture("textures/earth_cloud_0002.pkm", level++) ? 256 : 0);
   load_res |= (LoadTexture("textures/earth_cloud_0001.pkm", level++) ? 512 : 0);
   pEarthCloud->uiTextureID = auiTextureID[2];
   if (load_res != 0x3FF)
      goto load_failed;

   /* init OpenGL state */
   glEnable(GL_CULL_FACE);

   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
   glClearDepthf(1.0f);

   DestroyTextureLoader();

   InitShaders();

   esMatrixLoadIdentity(&modelview_matrix);

   return;

load_failed:
   /* bomb out if texture load failed */
   printf("unable to locate some (or all) of the textures.  Are you running this from the correct location?\n");
   exit(0);
}

void TerminateGLState(void)
{
   glDeleteTextures(3, auiTextureID);

   GLAPP_DestroyObject(pCamera);
   GLAPP_DestroyObject(pStars);
   GLAPP_DestroyObject(pEarthCloud);
   GLAPP_DestroyObject(pEarthLand);
   GLAPP_DestroyObject(pMoon);
}

void InitGLViewPort(int width, int height, float panelAspect, bool stretch)
{
   const GLfloat fWindowRatio = ((GLfloat) width) / height;

   glViewport(0, 0, width, height);

   esMatrixLoadIdentity(&projection_matrix);

   if (stretch)
      esFrustum(&projection_matrix, -panelAspect * 2,  panelAspect * 2, -2.0f, 2.0f, 5.0f, 500.0f);
   else
      esFrustum(&projection_matrix, -fWindowRatio * 2, fWindowRatio * 2, -2.0f, 2.0f, 5.0f, 500.0f);
}

void Resize(void)
{
   EGLint w = 0, h = 0;

   /* As this is just an example, and we don't have any kind of resize event, we will
      check whether the underlying window has changed size and adjust our viewport at the start of
      each frame. Obviously, this would be more efficient if event driven. */
   eglQuerySurface(eglGetCurrentDisplay(), eglGetCurrentSurface(EGL_DRAW), EGL_WIDTH, &w);
   eglQuerySurface(eglGetCurrentDisplay(), eglGetCurrentSurface(EGL_DRAW), EGL_HEIGHT, &h);

   if (w != config.vpW || h != config.vpH)
   {
      config.vpW = w;
      config.vpH = h;

      /* Ignore the panelAspect and stretch - if we resized we are window based anyway */
      InitGLViewPort(w, h, (float)w / (float)h, false);
   }
}

void Display(void)
{
   GLfloat         fCameraAngle;
   const GLfloat   afLightPosition[4] = { 1.0f, 1.0f, 1.0f, 0.0f };
   GLfloat         len;
   ESMatrix        starsmv, bodiesmv;

   Resize();

   GLAPP_MoveObject(pEarthLand,  0.0f, 0.0f, 0.0f, 0.0f);
   GLAPP_MoveObject(pEarthCloud, 0.0f, 0.0f, 0.0f, 0.0f);
   GLAPP_MoveObject(pMoon, pEarthLand->fMass, pEarthLand->fPositionX, pEarthLand->fPositionY, pEarthLand->fPositionZ);
   GLAPP_MoveObject(pCamera, pEarthLand->fMass, pEarthLand->fPositionX, pEarthLand->fPositionY, pEarthLand->fPositionZ);
   fCameraAngle = GLAPP_GetCameraAngle(pMoon->fPositionX - pCamera->fPositionX, pMoon->fPositionZ - pCamera->fPositionZ);

   glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   /* structure copy */
   starsmv = modelview_matrix;

   /* structure copy */
   bodiesmv = modelview_matrix;
   esRotate(&bodiesmv, pCamera->fAngle, pCamera->fAngleFacX, pCamera->fAngleFacY, pCamera->fAngleFacZ);

   /* need to multiply the light position by the current modelview_matrix */
   light_pos[0] = bodiesmv.m[0][0] * afLightPosition[0] +
               bodiesmv.m[1][0] * afLightPosition[1] +
               bodiesmv.m[2][0] * afLightPosition[2] +
               bodiesmv.m[3][0] * afLightPosition[3];

   light_pos[1] = bodiesmv.m[0][1] * afLightPosition[0] +
               bodiesmv.m[1][1] * afLightPosition[1] +
               bodiesmv.m[2][1] * afLightPosition[2] +
               bodiesmv.m[3][1] * afLightPosition[3];

   light_pos[2] = bodiesmv.m[0][2] * afLightPosition[0] +
               bodiesmv.m[1][2] * afLightPosition[1] +
               bodiesmv.m[2][2] * afLightPosition[2] +
               bodiesmv.m[3][2] * afLightPosition[3];

   light_pos[3] = bodiesmv.m[0][3] * afLightPosition[0] +
               bodiesmv.m[1][3] * afLightPosition[1] +
               bodiesmv.m[2][3] * afLightPosition[2] +
               bodiesmv.m[3][3] * afLightPosition[3];

   /* normalize */
   len = (GLfloat)(light_pos[0] * light_pos[0] +
               light_pos[1] * light_pos[1] +
               light_pos[2] * light_pos[2]);

   if (len != 0.0f)
   {
      len = 1.0f / sqrtf(len);
      light_pos[0] = (GLfloat)(light_pos[0] * len);
      light_pos[1] = (GLfloat)(light_pos[1] * len);
      light_pos[2] = (GLfloat)(light_pos[2] * len);
   }

   /* glLightfv(GL_LIGHT0, GL_POSITION, afLightPosition);  // something special about placement? */
   esRotate(&bodiesmv, fCameraAngle, 0.0f, 1.0f, 0.0f);
   esTranslate(&bodiesmv, -pCamera->fPositionX, -pCamera->fPositionY, -pCamera->fPositionZ);

   GLAPP_DrawObject(&bodiesmv, pMoon,       GL_TRUE, GL_FALSE, GL_TRUE, PLANET_TEX_FILTER);
   GLAPP_DrawObject(&bodiesmv, pEarthLand,  GL_TRUE, GL_FALSE, GL_TRUE, PLANET_TEX_FILTER);

   esTranslate(&starsmv, 0.0f, 0.0f, -10.0f * FSCALE);
   esRotate(&starsmv, pCamera->fAngle, pCamera->fAngleFacX, pCamera->fAngleFacY, pCamera->fAngleFacZ);
   esRotate(&starsmv, fCameraAngle, 0.0f, 1.0f, 0.0f);
   GLAPP_DrawObject(&starsmv, pStars, GL_FALSE, GL_FALSE, GL_TRUE, GL_LINEAR);

   GLAPP_DrawObject(&bodiesmv, pEarthCloud, GL_TRUE, GL_TRUE,  GL_TRUE, PLANET_TEX_FILTER);

   eglSwapBuffers(eglGetCurrentDisplay(), eglGetCurrentSurface(EGL_READ));
}

static void BppToChannels(int bpp, int *r, int *g, int *b, int *a)
{
   switch (bpp)
   {
   default:
   case 16:             /* 16-bit RGB (565)  */
      *r = 5;
      *g = 6;
      *b = 5;
      *a = 0;
      break;

   case 32:             /* 32-bit RGBA       */
      *r = 8;
      *g = 8;
      *b = 8;
      *a = 8;
      break;

   case 24:             /* 24-bit RGB        */
      *r = 8;
      *g = 8;
      *b = 8;
      *a = 0;
      break;
   }
}

bool InitEGL(NativeWindowType egl_win)
{
   EGLDisplay egl_display      = 0;
   EGLSurface egl_surface      = 0;
   EGLContext egl_context      = 0;
   EGLConfig *egl_config;
   EGLint     major_version;
   EGLint     minor_version;
   int        config_select    = 0;
   int        configs;

   /*
      Specifies the required configuration attributes.
      An EGL "configuration" describes the pixel format and type of
      surfaces that can be used for drawing.
      For now we just want to use a 16 bit RGB surface that is a
      Window surface, i.e. it will be visible on screen. The list
      has to contain key/value pairs, terminated with EGL_NONE.
   */
   int   want_red   = 0;
   int   want_green = 0;
   int   want_blue  = 0;
   int   want_alpha = 0;

   BppToChannels(config.bpp, &want_red, &want_green, &want_blue, &want_alpha);

   /*
      Step 1 - Get the EGL display.
   */
   egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

   /*
      Step 2 - Initialize EGL.
      EGL has to be initialized with the display obtained in the
      previous step. We cannot use other EGL functions except
      eglGetDisplay and eglGetError before eglInitialize has been
      called.
   */
   if (!eglInitialize(egl_display, &major_version, &minor_version))
   {
      printf("eglInitialize() failed");
      return false;
   }

   /*
      Step 3 - Get the number of configurations to correctly size the array
      used in step 4
   */
   if (!eglGetConfigs(egl_display, NULL, 0, &configs))
   {
      printf("eglGetConfigs() failed");
      return false;
   }

   egl_config = (EGLConfig *)alloca(configs * sizeof(EGLConfig));

   /*
      Step 4 - Find a config that matches all requirements.
      eglChooseConfig provides a list of all available configurations
      that meet or exceed the requirements given as the second
      argument.
   */

   {
      const int   NUM_ATTRIBS = 21;
      EGLint      *attr = (EGLint *)malloc(NUM_ATTRIBS * sizeof(EGLint));
      int         i = 0;

      attr[i++] = EGL_RED_SIZE;        attr[i++] = want_red;
      attr[i++] = EGL_GREEN_SIZE;      attr[i++] = want_green;
      attr[i++] = EGL_BLUE_SIZE;       attr[i++] = want_blue;
      attr[i++] = EGL_ALPHA_SIZE;      attr[i++] = want_alpha;
      attr[i++] = EGL_DEPTH_SIZE;      attr[i++] = 24;
      attr[i++] = EGL_STENCIL_SIZE;    attr[i++] = 0;
      attr[i++] = EGL_SURFACE_TYPE;    attr[i++] = EGL_WINDOW_BIT;
      attr[i++] = EGL_RENDERABLE_TYPE; attr[i++] = EGL_OPENGL_ES2_BIT;

      if (config.useMultisample)
      {
         attr[i++] = EGL_SAMPLE_BUFFERS; attr[i++] = 1;
         attr[i++] = EGL_SAMPLES;        attr[i++] = 4;
      }

      attr[i++] = EGL_NONE;

      assert(i <= NUM_ATTRIBS);

      if (!eglChooseConfig(egl_display, attr, egl_config, configs, &configs) || (configs == 0))
      {
         printf("eglChooseConfig() failed");
         return false;
      }

      free(attr);
   }

   for (config_select = 0; config_select < configs; config_select++)
   {
      /*
         Configs with deeper color buffers get returned first by eglChooseConfig.
         Applications may find this counterintuitive, and need to perform additional processing on the list of
         configs to find one best matching their requirements. For example, specifying RGBA depths of 565
         could return a list whose first config has a depth of 888.
      */

      /* Check that config is an exact match */
      EGLint red_size, green_size, blue_size, alpha_size, depth_size;

      eglGetConfigAttrib(egl_display, egl_config[config_select], EGL_RED_SIZE,   &red_size);
      eglGetConfigAttrib(egl_display, egl_config[config_select], EGL_GREEN_SIZE, &green_size);
      eglGetConfigAttrib(egl_display, egl_config[config_select], EGL_BLUE_SIZE,  &blue_size);
      eglGetConfigAttrib(egl_display, egl_config[config_select], EGL_ALPHA_SIZE, &alpha_size);
      eglGetConfigAttrib(egl_display, egl_config[config_select], EGL_DEPTH_SIZE, &depth_size);

      if ((red_size == want_red) && (green_size == want_green) && (blue_size == want_blue) && (alpha_size == want_alpha))
      {
         printf("Selected config: R=%d G=%d B=%d A=%d Depth=%d\n", red_size, green_size, blue_size, alpha_size, depth_size);
         break;
      }
   }

   if (config_select == configs)
   {
      printf("No suitable configs found\n");
      return false;
   }

   /*
      Step 5 - Create a surface to draw to.
      Use the config picked in the previous step and the native window
      handle to create a window surface.
   */
   egl_surface = eglCreateWindowSurface(egl_display, egl_config[config_select], egl_win, NULL);
   if (egl_surface == EGL_NO_SURFACE)
   {
      eglGetError(); /* Clear error */
      egl_surface = eglCreateWindowSurface(egl_display, egl_config[config_select], NULL, NULL);
   }

   if (egl_surface == EGL_NO_SURFACE)
   {
      printf("eglCreateWindowSurface() failed\n");
      return false;
   }

   /*
      Step 6 - Create a context.
      EGL has to create a context for OpenGL ES. Our OpenGL ES resources
      like textures will only be valid inside this context (or shared contexts)
   */
   {
      EGLint     ctx_attrib_list[3] =
      {
           EGL_CONTEXT_CLIENT_VERSION, 2, /* For ES2 */
           EGL_NONE
      };

      egl_context = eglCreateContext(egl_display, egl_config[config_select], EGL_NO_CONTEXT, ctx_attrib_list);
      if (egl_context == EGL_NO_CONTEXT)
      {
         printf("eglCreateContext() failed");
         return false;
      }
   }

   /*
      Step 7 - Bind the context to the current thread and use our
      window surface for drawing and reading.
      We need to specify a surface that will be the target of all
      subsequent drawing operations, and one that will be the source
      of read operations. They can be the same surface.
   */
   eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);

   eglSwapInterval(egl_display, 1);

   return true;
}

bool InitDisplay(float *aspect)
{
   LFPL_NativeWindowInfo   win_info;

   eInitResult res = InitPlatformAndDefaultDisplay(&nexus_display, aspect, config.vpW, config.vpH);
   if (res != eInitSuccess)
      return false;

   /* Register with the platform layer */
   LFPL_RegisterNexusDisplayPlatform(&lfpl_handle, nexus_display);

   win_info.x = config.x;
   win_info.y = config.y;
   win_info.width = config.vpW;
   win_info.height = config.vpH;
   win_info.stretch = config.stretchToFit;
   win_info.clientID = config.clientId;
   native_window = LFPL_CreateNativeWindow(&win_info);

   /* Initialise EGL now we have a 'window' */
   if (!InitEGL(native_window))
      return false;

   return true;
}

bool processArgs(int argc, const char *argv[])
{
   int   a;

   config.useMultisample    = false;
   config.usePreservingSwap = false;
   config.stretchToFit      = false;
   config.x                 = 0;
   config.y                 = 0;
   config.vpW               = WIDTH;
   config.vpH               = HEIGHT;
   config.bpp               = BPP;
   config.clientId          = 0;

   for (a = 1; a < argc; ++a)
   {
      const char  *arg = argv[a];

      if (strcmp(arg, "+m") == 0)
         config.useMultisample = true;
      else if (strcmp(arg, "+p") == 0)
         config.usePreservingSwap = true;
      else if (strcmp(arg, "+s") == 0)
         config.stretchToFit = true;
      else if (strncmp(arg, "d=", 2) == 0)
      {
         if (sscanf(arg, "d=%dx%d", &config.vpW, &config.vpH) != 2)
            return false;
      }
      else if (strncmp(arg, "o=", 2) == 0)
      {
         if (sscanf(arg, "o=%dx%d", &config.x, &config.y) != 2)
            return false;
      }
      else if (strncmp(arg, "bpp=", 4) == 0)
      {
         if (sscanf(arg, "bpp=%d", (int *)&config.bpp) != 1)
            return false;
      }
      else if (strncmp(arg, "client=", 7) == 0)
      {
         if (sscanf(arg, "client=%u", &config.clientId) != 1)
            return false;
      }
      else
      {
         return false;
      }
   }

   return true;
}

#ifndef CLIENT_MAIN
#define CLIENT_MAIN main
#endif

int CLIENT_MAIN(int argc, const char** argv)
{
   float        panelAspect = 1.0f;
   EGLDisplay   eglDisplay;
   unsigned int frame = 1;

   if (!processArgs(argc, argv))
   {
      const char  *progname = argc > 0 ? argv[0] : "";
      fprintf(stderr, "Usage: %s [+m] [+p] [+s] [d=WxH] [o=XxY] [bpp=16/24/32]\n", progname);
      return 0;
   }

   /* Setup the display and EGL */
   if (InitDisplay(&panelAspect))
   {
      /* Setup the local state for this demo */
      InitGLState();
      InitGLViewPort(config.vpW, config.vpH, panelAspect, config.stretchToFit);

      while (true)
      {
         Display();
         frame++;
      }

      /* Close the local state for this demo */
      TerminateGLState();

   }

   /* Terminate EGL */
   eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
   eglTerminate(eglDisplay);

   LFPL_DestroyNativeWindow(native_window);
   LFPL_UnregisterNexusDisplayPlatform(lfpl_handle);

   /* Close the platform */
   TermPlatform(nexus_display);

   return 0;
}

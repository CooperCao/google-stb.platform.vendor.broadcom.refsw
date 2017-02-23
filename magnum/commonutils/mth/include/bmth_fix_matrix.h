/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
 ******************************************************************************/
#ifndef BMTH_FIX_MATRIX_H__
#define BMTH_FIX_MATRIX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bstd.h"
#include "bmth_fix.h"


/***************************************************************************
Structures:
***************************************************************************/
typedef struct BMTH_FIX_Matrix
{
	uint32_t data[4][4];
	uint32_t ulSize;
	uint32_t ulFractBits;
} BMTH_FIX_Matrix;

typedef struct BMTH_FIX_Vector
{
	uint32_t data[4];
	uint32_t ulSize;
	uint32_t ulFractBits;
} BMTH_FIX_Vector;

typedef struct BMTH_FIX_Matrix_64
{
	uint64_t data[4][4];
	uint32_t ulSize;
	uint32_t ulFractBits;
} BMTH_FIX_Matrix_64;

typedef struct BMTH_FIX_Vector_64
{
	uint64_t data[4];
	uint32_t ulSize;
	uint32_t ulFractBits;
} BMTH_FIX_Vector_64;

/***************************************************************************
Summary:
	Takes two matrices and multiplies them together.  Result is a matrix
	with the same size and fixed point fractional bits.  Currently only
	supports matrices of same size.
***************************************************************************/
void BMTH_FIX_Matrix_Mult
	( BMTH_FIX_Matrix                           *pMatrix1,
	  BMTH_FIX_Matrix                           *pMatrix2,
	  BMTH_FIX_Matrix                           *pRetMatrix);

/***************************************************************************
Summary:
	Multiplies a Matrix with a Vector.  Result is a vector with the same
	size as the original vector and same fixed point fractional bits.
***************************************************************************/
void BMTH_FIX_Matrix_MultVector
	( BMTH_FIX_Matrix                           *pMatrix,
	  BMTH_FIX_Vector                           *pVector,
	  BMTH_FIX_Vector                           *pRetVector);

void BMTH_FIX_Matrix_MultVector_64
	( BMTH_FIX_Matrix_64                        *pMatrix,
	  BMTH_FIX_Vector_64                        *pVector,
	  BMTH_FIX_Vector_64                        *pRetVector);

/***************************************************************************
Summary:
	Takes a 3x3 matrix and converts it to a 4x4 matrix.
***************************************************************************/
void BMTH_FIX_Matrix_Make4x4
	( BMTH_FIX_Matrix                           *pMatrix,
	  BMTH_FIX_Matrix                           *pRetMatrix);

/***************************************************************************
Summary:
	Transposes a matrix.
***************************************************************************/
void BMTH_FIX_Matrix_Transpose
	( BMTH_FIX_Matrix                           *pMatrix,
	  BMTH_FIX_Matrix                           *pRetMatrix);

/***************************************************************************
Summary:
	Calculates the inverse matrix.
***************************************************************************/
void BMTH_FIX_Matrix_Inverse
	( BMTH_FIX_Matrix                           *pMatrix,
	  BMTH_FIX_Matrix                           *pRetMatrix);

/***************************************************************************
Summary:
	Calculates the determinant of a matrix.  If cofactor matrix is present,
	precaculated cofactors from the table are used.
***************************************************************************/
uint32_t BMTH_FIX_Matrix_Determinant
	( BMTH_FIX_Matrix                           *pMatrix,
	  BMTH_FIX_Matrix                           *pCofactors);

/***************************************************************************
Summary:
	Multiplies a matrix by a scalar.
***************************************************************************/
void BMTH_FIX_Matrix_MultScalar
	( BMTH_FIX_Matrix                           *pMatrix,
	  uint32_t                                   ulScalar,
	  BMTH_FIX_Matrix                           *pRetMatrix);

/***************************************************************************
Summary:
	Prints a matrix.
***************************************************************************/
void BMTH_FIX_Matrix_Dump
	( BMTH_FIX_Matrix                           *pMatrix);


#ifdef __cplusplus
}
#endif

#endif  /* #ifndef BMTH_FIX_MATRIX_H__ */
/* End of File */

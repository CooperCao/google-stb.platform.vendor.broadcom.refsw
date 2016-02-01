/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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
 ***************************************************************************/
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

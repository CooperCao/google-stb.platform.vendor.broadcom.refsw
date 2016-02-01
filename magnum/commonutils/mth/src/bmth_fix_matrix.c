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
#include "bmth_fix_matrix.h"

/*************************************************************************
 * BMTH_FIX_SIGNED_CONVERT
 *
 *************************************************************************/

BDBG_MODULE(BMTH_FIX_MATRIX);

#define BMTH_P_FIX_MATRIX_MAX_BITS (31)

#define BMTH_P_FIX_MATRIX_CONVERT(x, infract, outfract) \
	(BMTH_FIX_SIGNED_CONVERT(x, (BMTH_P_FIX_MATRIX_MAX_BITS - infract), infract, (BMTH_P_FIX_MATRIX_MAX_BITS - outfract), outfract))

/* fixed to integer */
#define BMTH_P_FIX_MATRIX_FIXTOI(x, infract) \
	(BMTH_FIX_SIGNED_FIXTOI(x, (BMTH_P_FIX_MATRIX_MAX_BITS - infract), infract))

/* integer to fixed */
#define BMTH_P_FIX_MATRIX_ITOFIX(x, outfract) \
	(BMTH_FIX_SIGNED_ITOFIX(x, (BMTH_P_FIX_MATRIX_MAX_BITS - outfract), outfract))

/* fixed point operation multiply */
#define BMTH_P_FIX_MATRIX_MUL(x, y, fract) \
	(BMTH_FIX_SIGNED_MUL(x, y, (BMTH_P_FIX_MATRIX_MAX_BITS - fract), fract, \
                               (BMTH_P_FIX_MATRIX_MAX_BITS - fract), fract, \
                               (BMTH_P_FIX_MATRIX_MAX_BITS - fract), fract))

/* fixed point operation divide */
#define BMTH_P_FIX_MATRIX_DIV(x, y, fract) \
	(BMTH_FIX_SIGNED_DIV(x, y, (BMTH_P_FIX_MATRIX_MAX_BITS - fract), fract, \
                               (BMTH_P_FIX_MATRIX_MAX_BITS - fract), fract, \
                               (BMTH_P_FIX_MATRIX_MAX_BITS - fract), fract))


#if !B_REFSW_MINIMAL
/***************************************************************************
 * {private}
 *
 * Takes two matrices and multiplies them together.  Result is a matrix
 * with the same size and fixed point fractional bits.
 */
void BMTH_FIX_Matrix_Mult(BMTH_FIX_Matrix *pMatrix1, BMTH_FIX_Matrix *pMatrix2, BMTH_FIX_Matrix *pRetMatrix)
{
	BMTH_FIX_Matrix stMatrixTemp;
	uint32_t ulSize = pMatrix1->ulSize;
	uint32_t ulFractBits = pMatrix1->ulFractBits;
	uint32_t i = 0;
	uint32_t j = 0;
	uint32_t k = 0;

	BDBG_ASSERT(pMatrix1->ulSize == pMatrix2->ulSize);
	BDBG_ASSERT(pMatrix1->ulFractBits == pMatrix2->ulFractBits);

	stMatrixTemp.ulSize = ulSize;
	stMatrixTemp.ulFractBits = ulFractBits;

	for (i = 0; i < ulSize; i++)
	{
		for (j = 0; j < ulSize; j++)
		{
			stMatrixTemp.data[i][j] = 0;
			for (k = 0; k < ulSize; k++)
			{
				stMatrixTemp.data[i][j] += (unsigned)BMTH_P_FIX_MATRIX_MUL((signed)pMatrix1->data[i][k],  (signed)pMatrix2->data[k][j], ulFractBits);
			}
		}
	}

	BKNI_Memcpy(pRetMatrix, &stMatrixTemp, sizeof(stMatrixTemp));
}
#endif

/***************************************************************************
 * {private}
 *
 * Multiplies a Matrix with a Vector.  Result is a vector with the same
 * size as the original vector and same fixed point fractional bits.
 */
void BMTH_FIX_Matrix_MultVector(BMTH_FIX_Matrix *pMatrix, BMTH_FIX_Vector *pVector, BMTH_FIX_Vector *pRetVector)
{
	BMTH_FIX_Vector stTempVector;
	uint32_t ulSize = pMatrix->ulSize;
	uint32_t ulFractBits = pMatrix->ulFractBits;
	uint32_t i = 0;
	uint32_t k = 0;

	BDBG_ASSERT(pMatrix->ulSize == pVector->ulSize);
	BDBG_ASSERT(pMatrix->ulFractBits == pVector->ulFractBits);

	stTempVector.ulSize = ulSize;
	stTempVector.ulFractBits = ulFractBits;

	for (i = 0; i < ulSize; i++)
	{
		stTempVector.data[i] = 0;
		for (k = 0; k < ulSize; k++)
		{
			stTempVector.data[i] += BMTH_P_FIX_MATRIX_MUL(pMatrix->data[i][k],  pVector->data[k], ulFractBits);
		}
	}

	BKNI_Memcpy(pRetVector, &stTempVector, sizeof(stTempVector));
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 * {private}
 *
 * Takes a 3x3 matrix and converts it to a 4x4 matrix.
 */
void BMTH_FIX_Matrix_Make4x4(BMTH_FIX_Matrix *pMatrix, BMTH_FIX_Matrix *pRetMatrix)
{
	uint32_t ulFractBits = pMatrix->ulFractBits;
	uint32_t i = 0;
	uint32_t j = 0;

	BDBG_ASSERT(pMatrix->ulSize == 3);

	BKNI_Memset(pRetMatrix, 0, sizeof(BMTH_FIX_Matrix));
	pRetMatrix->ulSize = 4;
	pRetMatrix->ulFractBits = ulFractBits;
	pRetMatrix->data[3][3] = BMTH_P_FIX_MATRIX_ITOFIX(1, ulFractBits);

	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			pRetMatrix->data[i][j] = pMatrix->data[i][j];
		}
	}
}
#endif

/***************************************************************************
 * {private}
 *
 * Calculates the cofactor of a matrix at a specific position.
 */
static uint32_t BMTH_P_FIX_Matrix_Cofactor(BMTH_FIX_Matrix *pMatrix, uint32_t ulRow, uint32_t ulCol)
{
	uint32_t ulSignPos = (ulCol + ulRow) & 1;
	uint32_t ulSize = pMatrix->ulSize;
	uint32_t ulFractBits = pMatrix->ulFractBits;
	uint32_t i = 0;
	uint32_t j = 0;
	uint32_t ulSrcRow = 0;
	uint32_t ulSrcCol = 0;
	uint32_t ulCofactor = 0;

	BMTH_FIX_Matrix stTempMatrix;

	BKNI_Memset(&stTempMatrix, 0, sizeof(BMTH_FIX_Matrix));
	stTempMatrix.ulSize = ulSize - 1;
	stTempMatrix.ulFractBits = ulFractBits;

	/* build 3x3 matrix for calculating determinant */
	for (i = 0, ulSrcRow = 0; i < ulSize; i++, ulSrcRow++)
	{
		if (i == ulRow)
		{
			ulSrcRow++;
		}
		for (j = 0, ulSrcCol = 0; j < ulSize; j++, ulSrcCol++)
		{
			if (j == ulCol)
			{
				ulSrcCol++;
			}
			stTempMatrix.data[i][j] = pMatrix->data[ulSrcRow][ulSrcCol];
		}
	}

	/* get determinant */
	ulCofactor = BMTH_FIX_Matrix_Determinant(&stTempMatrix, NULL);

	/* apply sign based on position */
	ulCofactor *= (ulSignPos) ? -1 : 1;

	return ulCofactor;
}

/***************************************************************************
 * {private}
 *
 * Calculates the determinant of a matrix.  If cofactor matrix is present,
 * precaculated cofactors from the table are used.
 */
uint32_t BMTH_FIX_Matrix_Determinant(BMTH_FIX_Matrix *pMatrix, BMTH_FIX_Matrix *pCofactors)
{
	uint32_t ulCol;
	uint32_t ulDeterminant = 0;
	uint32_t ulSize = pMatrix->ulSize;
	uint32_t ulFractBits = pMatrix->ulFractBits;

	if (ulSize == 1)
	{
		ulDeterminant = pMatrix->data[0][0];
	}
	else if (ulSize == 2)
	{
		ulDeterminant = BMTH_P_FIX_MATRIX_MUL(pMatrix->data[0][0], pMatrix->data[1][1], ulFractBits) -
			            BMTH_P_FIX_MATRIX_MUL(pMatrix->data[0][1], pMatrix->data[1][0], ulFractBits);
/*		printf("a: %f * d: %f - b: %f * c: %f \n", 
			BMTH_FIX_SIGNED_FIXTOF(pMatrix->data[0][0], 31 - pMatrix->ulFractBits, pMatrix->ulFractBits),
			BMTH_FIX_SIGNED_FIXTOF(pMatrix->data[1][1], 31 - pMatrix->ulFractBits, pMatrix->ulFractBits),
			BMTH_FIX_SIGNED_FIXTOF(pMatrix->data[0][1], 31 - pMatrix->ulFractBits, pMatrix->ulFractBits),
			BMTH_FIX_SIGNED_FIXTOF(pMatrix->data[1][0], 31 - pMatrix->ulFractBits, pMatrix->ulFractBits));
		printf("a * d (0x%x) %f - b * c (0x%x) %f\n",
			BMTH_P_FIX_MATRIX_MUL(pMatrix->data[0][0], pMatrix->data[1][1], ulFractBits),
			BMTH_FIX_SIGNED_FIXTOF(BMTH_P_FIX_MATRIX_MUL(pMatrix->data[0][0], pMatrix->data[1][1], ulFractBits), 31 - pMatrix->ulFractBits, pMatrix->ulFractBits),
			BMTH_P_FIX_MATRIX_MUL(pMatrix->data[0][1], pMatrix->data[1][0], ulFractBits),
			BMTH_FIX_SIGNED_FIXTOF(BMTH_P_FIX_MATRIX_MUL(pMatrix->data[0][1], pMatrix->data[1][0], ulFractBits), 31 - pMatrix->ulFractBits, pMatrix->ulFractBits));
*/
	}
	else
	{
		/* add cofactors along top row */
		for (ulCol = 0; ulCol < ulSize; ulCol++)
		{
			if (pCofactors)
			{
				/* use precomputed cofactors */
				ulDeterminant += BMTH_P_FIX_MATRIX_MUL(pMatrix->data[0][ulCol], pCofactors->data[0][ulCol], ulFractBits);
			}
			else
			{
				ulDeterminant += BMTH_P_FIX_MATRIX_MUL(pMatrix->data[0][ulCol], BMTH_P_FIX_Matrix_Cofactor(pMatrix, 0, ulCol), ulFractBits);
			}
		}
	}

	return ulDeterminant;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 * {private}
 *
 * Creates a matrix holding all cofactors of a matrix.
 */
static void BMTH_P_FIX_Matrix_CofactorMatrix(BMTH_FIX_Matrix *pMatrix, BMTH_FIX_Matrix *pCofactors)
{
	uint32_t ulRow;
	uint32_t ulCol;
	uint32_t ulSize = pMatrix->ulSize;
	uint32_t ulFractBits = pMatrix->ulFractBits;

	for (ulRow = 0; ulRow < ulSize; ulRow++)
	{
		for (ulCol = 0; ulCol < ulSize; ulCol++)
		{
			pCofactors->data[ulRow][ulCol] = BMTH_P_FIX_Matrix_Cofactor(pMatrix, ulRow, ulCol);
		}
	}

	pCofactors->ulSize = ulSize;
	pCofactors->ulFractBits = ulFractBits;
}

/***************************************************************************
 * {private}
 *
 * Transposes a matrix.
 */
void BMTH_FIX_Matrix_Transpose(BMTH_FIX_Matrix *pMatrix, BMTH_FIX_Matrix *pRetMatrix)
{
	uint32_t ulRow;
	uint32_t ulCol;
	uint32_t ulSize = pMatrix->ulSize;
	uint32_t ulFractBits = pMatrix->ulFractBits;

	for (ulRow = 0; ulRow < ulSize; ulRow++)
	{
		for (ulCol = 0; ulCol < ulSize; ulCol++)
		{
			pRetMatrix->data[ulRow][ulCol] = pMatrix->data[ulCol][ulRow];
		}
	}

	pRetMatrix->ulSize = ulSize;
	pRetMatrix->ulFractBits = ulFractBits;
}

/***************************************************************************
 * {private}
 *
 * Multiplies a matrix by a scalar.
 */
void BMTH_FIX_Matrix_MultScalar(BMTH_FIX_Matrix *pMatrix, uint32_t ulScalar, BMTH_FIX_Matrix *pRetMatrix)
{
	uint32_t ulRow;
	uint32_t ulCol;
	uint32_t ulSize = pMatrix->ulSize;
	uint32_t ulFractBits = pMatrix->ulFractBits;

	for (ulRow = 0; ulRow < ulSize; ulRow++)
	{
		for (ulCol = 0; ulCol < ulSize; ulCol++)
		{
			pRetMatrix->data[ulRow][ulCol] = BMTH_P_FIX_MATRIX_MUL(pMatrix->data[ulRow][ulCol], ulScalar, ulFractBits);
		}
	}

	pRetMatrix->ulSize = ulSize;
	pRetMatrix->ulFractBits = ulFractBits;
}

/***************************************************************************
 * {private}
 *
 * Calculates the inverse matrix.
 */
void BMTH_FIX_Matrix_Inverse(BMTH_FIX_Matrix *pMatrix, BMTH_FIX_Matrix *pRetMatrix)
{
	uint32_t ulFractBits = pMatrix->ulFractBits;
	BMTH_FIX_Matrix stCofactorMatrix;
	BMTH_FIX_Matrix stAdjointMatrix;
	uint32_t ulDeterminant;
	uint32_t ulOneOverDeterminant;
	uint32_t ulFixOne = BMTH_P_FIX_MATRIX_ITOFIX(1, ulFractBits);

	BKNI_Memset(&stCofactorMatrix, 0, sizeof(BMTH_FIX_Matrix));
	BKNI_Memset(&stAdjointMatrix, 0, sizeof(BMTH_FIX_Matrix));

	/* InvM = 1/det(M) * Transpose(Cofactor(M)) */

	/* compute all cofactors */
	BMTH_P_FIX_Matrix_CofactorMatrix(pMatrix, &stCofactorMatrix);
	ulDeterminant = BMTH_FIX_Matrix_Determinant(pMatrix, &stCofactorMatrix);
	BDBG_ASSERT(ulDeterminant);
	ulOneOverDeterminant = BMTH_P_FIX_MATRIX_DIV(ulFixOne, ulDeterminant, ulFractBits);
	BMTH_FIX_Matrix_Transpose(&stCofactorMatrix, &stAdjointMatrix);
	BMTH_FIX_Matrix_MultScalar(&stAdjointMatrix, ulOneOverDeterminant, pRetMatrix);
}

/***************************************************************************
 * {private}
 *
 * Prints a matrix.
 */
void BMTH_FIX_Matrix_Dump(BMTH_FIX_Matrix *pMatrix)
{
	uint32_t i = 0;
	uint32_t j = 0;

	for (i = 0; i < pMatrix->ulSize; i++)
	{
		for (j = 0; j < pMatrix->ulSize; j++)
		{
			BKNI_Printf("0x%x ", pMatrix->data[i][j]);
/*			BKNI_Printf("%f ", BMTH_FIX_SIGNED_FIXTOF(pMatrix->data[i][j], 31 - pMatrix->ulFractBits, pMatrix->ulFractBits));*/
		}
		BKNI_Printf("\n");
	}
	BKNI_Printf("\n");
}
#endif

/* end of file */

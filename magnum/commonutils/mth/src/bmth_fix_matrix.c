/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bmth_fix_matrix.h"

/*************************************************************************
 * BMTH_FIX_SIGNED_CONVERT
 *
 *************************************************************************/

BDBG_MODULE(BMTH_FIX_MATRIX);

#define BMTH_P_FIX_MATRIX_MAX_BITS (31)

#define BMTH_P_FIX_MATRIX_CONVERT(x, infract, outfract) \
    (BMTH_FIX_SIGNED_CONVERT_isrsafe(x, (BMTH_P_FIX_MATRIX_MAX_BITS - infract), infract, (BMTH_P_FIX_MATRIX_MAX_BITS - outfract), outfract))

/* fixed to integer */
#define BMTH_P_FIX_MATRIX_FIXTOI(x, infract) \
    (BMTH_FIX_SIGNED_FIXTOI(x, (BMTH_P_FIX_MATRIX_MAX_BITS - infract), infract))

/* integer to fixed */
#define BMTH_P_FIX_MATRIX_ITOFIX(x, outfract) \
    (BMTH_FIX_SIGNED_ITOFIX(x, (BMTH_P_FIX_MATRIX_MAX_BITS - outfract), outfract))

/* fixed point operation multiply */
#define BMTH_P_FIX_MATRIX_MUL(x, y, fract) \
    (BMTH_FIX_SIGNED_MUL_isrsafe(x, y, (BMTH_P_FIX_MATRIX_MAX_BITS - fract), fract, \
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
void BMTH_FIX_Matrix_Mult_isrsafe(BMTH_FIX_Matrix *pMatrix1, BMTH_FIX_Matrix *pMatrix2, BMTH_FIX_Matrix *pRetMatrix)
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
void BMTH_FIX_Matrix_MultVector_isrsafe(BMTH_FIX_Matrix *pMatrix, BMTH_FIX_Vector *pVector, BMTH_FIX_Vector *pRetVector)
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

void BMTH_FIX_Matrix_MultVector_64_isrsafe(BMTH_FIX_Matrix_64 *pMatrix, BMTH_FIX_Vector_64 *pVector, BMTH_FIX_Vector_64 *pRetVector)
{
    BMTH_FIX_Vector_64 stTempVector;
    uint32_t ulSize = pMatrix->ulSize;
    uint32_t ulFractBits = pMatrix->ulFractBits;
    uint32_t i = 0;
    uint32_t k = 0;

    BDBG_ASSERT(pMatrix->ulSize == pVector->ulSize);
    BDBG_ASSERT(pMatrix->ulFractBits == pVector->ulFractBits);
    BDBG_ASSERT(pMatrix->ulSize <= BMTH_DATA_DIMENSION);

    stTempVector.ulSize = ulSize;
    stTempVector.ulFractBits = ulFractBits;

    for (i = 0; i < ulSize; i++)
    {
        stTempVector.data[i] = 0;
        for (k = 0; k < ulSize; k++)
        {
            stTempVector.data[i] += BMTH_FIX_SIGNED_MUL_64_isrsafe(pMatrix->data[i][k],  pVector->data[k], ulFractBits, pVector->ulFractBits, ulFractBits);
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
void BMTH_FIX_Matrix_Make4x4_isrsafe(BMTH_FIX_Matrix *pMatrix, BMTH_FIX_Matrix *pRetMatrix)
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
static uint32_t BMTH_P_FIX_Matrix_Cofactor_isrsafe(BMTH_FIX_Matrix *pMatrix, uint32_t ulRow, uint32_t ulCol)
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
uint32_t BMTH_FIX_Matrix_Determinant_isrsafe(BMTH_FIX_Matrix *pMatrix, BMTH_FIX_Matrix *pCofactors)
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
/*      printf("a: %f * d: %f - b: %f * c: %f \n",
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
                ulDeterminant += BMTH_P_FIX_MATRIX_MUL(pMatrix->data[0][ulCol], BMTH_P_FIX_Matrix_Cofactor_isrsafe(pMatrix, 0, ulCol), ulFractBits);
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
static void BMTH_P_FIX_Matrix_CofactorMatrix_isrsafe(BMTH_FIX_Matrix *pMatrix, BMTH_FIX_Matrix *pCofactors)
{
    uint32_t ulRow;
    uint32_t ulCol;
    uint32_t ulSize = pMatrix->ulSize;
    uint32_t ulFractBits = pMatrix->ulFractBits;

    for (ulRow = 0; ulRow < ulSize; ulRow++)
    {
        for (ulCol = 0; ulCol < ulSize; ulCol++)
        {
            pCofactors->data[ulRow][ulCol] = BMTH_P_FIX_Matrix_Cofactor_isrsafe(pMatrix, ulRow, ulCol);
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
void BMTH_FIX_Matrix_Transpose_isrsafe(BMTH_FIX_Matrix *pMatrix, BMTH_FIX_Matrix *pRetMatrix)
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
void BMTH_FIX_Matrix_MultScalar_isrsafe(BMTH_FIX_Matrix *pMatrix, uint32_t ulScalar, BMTH_FIX_Matrix *pRetMatrix)
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
void BMTH_FIX_Matrix_Inverse_isrsafe(BMTH_FIX_Matrix *pMatrix, BMTH_FIX_Matrix *pRetMatrix)
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
    BMTH_P_FIX_Matrix_CofactorMatrix_isrsafe(pMatrix, &stCofactorMatrix);
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
void BMTH_FIX_Matrix_Dump_isrsafe(BMTH_FIX_Matrix *pMatrix)
{
    uint32_t i = 0;
    uint32_t j = 0;

    for (i = 0; i < pMatrix->ulSize; i++)
    {
        for (j = 0; j < pMatrix->ulSize; j++)
        {
            BKNI_Printf("0x%x ", pMatrix->data[i][j]);
/*          BKNI_Printf("%f ", BMTH_FIX_SIGNED_FIXTOF(pMatrix->data[i][j], 31 - pMatrix->ulFractBits, pMatrix->ulFractBits));*/
        }
        BKNI_Printf("\n");
    }
    BKNI_Printf("\n");
}
#endif

/* end of file */

/***************************************************************************
 *     Copyright (c) 2003-2007, Broadcom Corporation
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
#include "bstd.h"              /* standard types */
#include "bkni.h"              /* memcpy calls */
#include "bmth_fix.h"
#include "bcsc.h"
#include "bdbg.h"

BDBG_MODULE(BCSC);

#define BCSC_P_FIX_MAX_BITS (31)

#define BCSC_P_FIX_CONVERT(x, infract, outfract) \
	(BMTH_FIX_SIGNED_CONVERT(x, (BCSC_P_FIX_MAX_BITS - infract), infract, (BCSC_P_FIX_MAX_BITS - outfract), outfract))

/* fixed to integer */
#define BCSC_P_FIXTOI(x, infract) \
	(BMTH_FIX_SIGNED_FIXTOI(x, (BCSC_P_FIX_MAX_BITS - infract), infract))

/* integer to fixed */
#define BCSC_P_ITOFIX(x, outfract) \
	(BMTH_FIX_SIGNED_ITOFIX(x, (BCSC_P_FIX_MAX_BITS - outfract), outfract))

/* fixed point operation multiply */
#define BCSC_P_FIX_MUL(x, y, fract) \
	(BMTH_FIX_SIGNED_MUL(x, y, (BCSC_P_FIX_MAX_BITS - fract), fract, \
                               (BCSC_P_FIX_MAX_BITS - fract), fract, \
                               (BCSC_P_FIX_MAX_BITS - fract), fract))

/* fixed point operation divide */
#define BCSC_P_FIX_DIV(x, y, fract) \
	(BMTH_FIX_SIGNED_DIV(x, y, (BCSC_P_FIX_MAX_BITS - fract), fract, \
                               (BCSC_P_FIX_MAX_BITS - fract), fract, \
                               (BCSC_P_FIX_MAX_BITS - fract), fract))

typedef struct BCSC_P_Matrix
{
	uint32_t data[4][4];
	uint32_t ulSize;
	uint32_t ulFractBits;
} BCSC_P_Matrix;

typedef struct BCSC_P_Vector
{
	uint32_t data[4];
	uint32_t ulSize;
	uint32_t ulFractBits;
} BCSC_P_Vector;


/* Prototypes */
void BCSC_P_Matrix_RGBW_To_NPM
	( uint32_t                              *pulRGBW,
	  uint32_t                               ulRGBWFractBits,
	  BCSC_P_Matrix                         *pNPM,
	  uint32_t                               ulNPMFractBits);

void BCSC_P_Matrix_NPM_To_RGBPrime_To_YCbCrPrime
	( BCSC_P_Matrix                         *pNPM,
	  BCSC_P_Matrix                         *pRetMatrix);

void BCSC_P_Matrix_Mult
	( BCSC_P_Matrix                         *pMatrix1,
	  BCSC_P_Matrix                         *pMatrix2,
	  BCSC_P_Matrix                         *pRetMatrix);

void BCSC_P_Matrix_MultVector
	( BCSC_P_Matrix                         *pMatrix,
	  BCSC_P_Vector                         *pVector,
	  BCSC_P_Vector                         *pRetVector);

void BCSC_P_Matrix_Make4x4
	( BCSC_P_Matrix                         *pMatrix,
	  BCSC_P_Matrix                         *pRetMatrix);

void BCSC_P_Matrix_GScale
	( uint32_t                               ulN,
	  BCSC_P_Matrix                         *pRetMatrix,
	  uint32_t                               ulMatrixFractBits);

void BCSC_P_Matrix_GScaleInverse
	( uint32_t                               ulN,
	  BCSC_P_Matrix                         *pRetMatrix,
	  uint32_t                               ulMatrixFractBits);

void BCSC_P_Matrix_GGain
	( uint32_t                               ulN,
	  BCSC_P_Matrix                         *pRetMatrix,
	  uint32_t                               ulMatrixFractBits);

uint32_t BCSC_P_Matrix_Cofactor
	( BCSC_P_Matrix                         *pMatrix,
	  uint32_t                               ulRow,
	  uint32_t                               ulCol);

uint32_t BCSC_P_Matrix_Determinant
	( BCSC_P_Matrix                         *pMatrix,
	  BCSC_P_Matrix                         *pCofactors);

void BCSC_P_Matrix_CofactorMatrix
	( BCSC_P_Matrix                         *pMatrix,
	  BCSC_P_Matrix                         *pCofactors);

void BCSC_P_Matrix_Transpose
	( BCSC_P_Matrix                         *pMatrix,
	  BCSC_P_Matrix                         *pRetMatrix);

void BCSC_P_Matrix_Inverse
	( BCSC_P_Matrix                         *pMatrix,
	  BCSC_P_Matrix                         *pRetMatrix);

void BCSC_P_Matrix_Dump
	( BCSC_P_Matrix                         *pMatrix);

/*  Given source and display (xR, yR), (xG, yG), (xB, yB), (xW, yW), derive CMP matrix;
    Output: CMP matrix in array of 12 fixed point coefficients;
    inputs: 
        pul_SrcRGBW       - Source gamut in (xR, yR), (xG, yG), (xB, yB), (xW, yW) array, 8 values;
        pul_DispRGBW:     - Display gamut in (xR, yR), (xG, yG), (xB, yB), (xW, yW) array, 8 values;
        ulXyFractBits     - Fixed-point fractional bits of input gamut values in (x, y) array;
        ulMatrixFractBits - Fixed-point fractional bits of output matrix coefficients; */
BERR_Code BCSC_RGBW2CmpMatrix
	( uint32_t                              *pul_SrcRGBW,
	  uint32_t                              *pul_DispRGBW,
	  uint32_t                               ulXyFractBits,
	  uint32_t                               ulMatrixFractBits,
	  uint32_t                               ulN,
	  int32_t                               *plCmpMatrix,
	  int32_t                               *plDvoMatrix)
{
	BCSC_P_Matrix stMatrix3_SrcNPM;
	BCSC_P_Matrix stMatrix3_Src_RGBPrime_To_YCbCrPrime;
	BCSC_P_Matrix stMatrix3_Src_YCbCrPrime_To_RGBPrime;
	BCSC_P_Matrix stMatrix4_Src_YCbCrPrime_To_RGBPrime;
	BCSC_P_Matrix stMatrix3_DispNPM;
	BCSC_P_Matrix stMatrix3_DispNPMInv;
	BCSC_P_Matrix stMatrix3_Disp_RGBPrime_To_YCbCrPrime;
	BCSC_P_Matrix stMatrix4_Disp_RGBPrime_To_YCbCrPrime;
	BCSC_P_Matrix stMatrix4_Disp_YCbCrPrime_To_RGBPrime;
	BCSC_P_Matrix stMatrix4_RGB_S_To_D;
	BCSC_P_Matrix stMatrix4_GScale;
	BCSC_P_Matrix stMatrix4_GScaleInv;
	BCSC_P_Matrix stMatrix4_GGain;
	BCSC_P_Matrix stMatrix4_CMP_CSC;
	BCSC_P_Matrix stMatrix4_DVI_CSC;
	BCSC_P_Matrix stMatrix3_Temp;

	uint32_t i = 0;
	uint32_t j = 0;

	if(!pul_SrcRGBW || !pul_DispRGBW || !plCmpMatrix || !plDvoMatrix)
	{
		return BERR_INVALID_PARAMETER;
	}

	stMatrix3_SrcNPM.ulSize                      = 3;
	stMatrix3_Src_RGBPrime_To_YCbCrPrime.ulSize  = 3;
	stMatrix3_DispNPM.ulSize                     = 3;
	stMatrix3_DispNPMInv.ulSize                  = 3;
	stMatrix3_Disp_RGBPrime_To_YCbCrPrime.ulSize = 3;
	stMatrix3_Temp.ulSize                        = 3;

	stMatrix4_Src_YCbCrPrime_To_RGBPrime.ulSize  = 4;
	stMatrix4_Disp_RGBPrime_To_YCbCrPrime.ulSize = 4;
	stMatrix4_Disp_YCbCrPrime_To_RGBPrime.ulSize = 4;
	stMatrix4_RGB_S_To_D.ulSize                  = 4;
	stMatrix4_GScale.ulSize                      = 4;
	stMatrix4_GScaleInv.ulSize                   = 4;
	stMatrix4_GGain.ulSize                       = 4;
	stMatrix4_CMP_CSC.ulSize                     = 4;
	stMatrix4_DVI_CSC.ulSize                     = 4;

	/* calculate 3x3 Source NPM */
	BCSC_P_Matrix_RGBW_To_NPM(pul_SrcRGBW, ulXyFractBits, &stMatrix3_SrcNPM, ulMatrixFractBits);

	/* calculate 3x3 Source R'G'B' to Y'Cb'Cr' matrix */
	BCSC_P_Matrix_NPM_To_RGBPrime_To_YCbCrPrime(&stMatrix3_SrcNPM, &stMatrix3_Src_RGBPrime_To_YCbCrPrime);

	/* calculate 4x4 Source Y'Cb'Cr' to R'G'B' matrix */
	BCSC_P_Matrix_Inverse(&stMatrix3_Src_RGBPrime_To_YCbCrPrime, &stMatrix3_Src_YCbCrPrime_To_RGBPrime);
	BCSC_P_Matrix_Make4x4(&stMatrix3_Src_YCbCrPrime_To_RGBPrime, &stMatrix4_Src_YCbCrPrime_To_RGBPrime);

	/* calculate 3x3 Display NPM */
	BCSC_P_Matrix_RGBW_To_NPM(pul_DispRGBW, ulXyFractBits, &stMatrix3_DispNPM, ulMatrixFractBits);

	/* calculate 4x4 Display R'G'B' to Y'Cb'Cr' matrix */
	BCSC_P_Matrix_NPM_To_RGBPrime_To_YCbCrPrime(&stMatrix3_DispNPM, &stMatrix3_Disp_RGBPrime_To_YCbCrPrime);
	BCSC_P_Matrix_Make4x4(&stMatrix3_Disp_RGBPrime_To_YCbCrPrime, &stMatrix4_Disp_RGBPrime_To_YCbCrPrime);

	/* calculate 4x4 Display Y'Cb'Cr' to R'G'B' matrix */
	BCSC_P_Matrix_Inverse(&stMatrix4_Disp_RGBPrime_To_YCbCrPrime, &stMatrix4_Disp_YCbCrPrime_To_RGBPrime);

	/* calculate 4x4 RGB source to RGB display matrix */
	BCSC_P_Matrix_Inverse(&stMatrix3_DispNPM, &stMatrix3_DispNPMInv);
	BCSC_P_Matrix_Mult(&stMatrix3_DispNPMInv, &stMatrix3_SrcNPM, &stMatrix3_Temp);
	BCSC_P_Matrix_Make4x4(&stMatrix3_Temp, &stMatrix4_RGB_S_To_D);

	/* calculate 4x4 G scale and G gain matrices */
	BCSC_P_Matrix_GScale(ulN, &stMatrix4_GScale, ulMatrixFractBits);
/*	BCSC_P_Matrix_Inverse(&stMatrix4_GScale, &stMatrix4_GScaleInv);*/
	BCSC_P_Matrix_GScaleInverse(ulN, &stMatrix4_GScaleInv, ulMatrixFractBits);
	BCSC_P_Matrix_GGain(ulN, &stMatrix4_GGain, ulMatrixFractBits);

	/* calculate final CMP CSC matrix */
	BCSC_P_Matrix_Mult(&stMatrix4_Src_YCbCrPrime_To_RGBPrime,  &stMatrix4_GScale,  &stMatrix4_CMP_CSC);
	BCSC_P_Matrix_Mult(&stMatrix4_RGB_S_To_D,                  &stMatrix4_CMP_CSC, &stMatrix4_CMP_CSC);
	BCSC_P_Matrix_Mult(&stMatrix4_Disp_RGBPrime_To_YCbCrPrime, &stMatrix4_CMP_CSC, &stMatrix4_CMP_CSC);
	BCSC_P_Matrix_Mult(&stMatrix4_GScaleInv,                   &stMatrix4_CMP_CSC, &stMatrix4_CMP_CSC);

	/* calculate final DVI CSC matrix */
	BCSC_P_Matrix_Mult(&stMatrix4_Disp_YCbCrPrime_To_RGBPrime, &stMatrix4_GScale,  &stMatrix4_DVI_CSC);
	BCSC_P_Matrix_Mult(&stMatrix4_GGain,                       &stMatrix4_DVI_CSC, &stMatrix4_DVI_CSC);

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			plCmpMatrix[i*4 + j] = stMatrix4_CMP_CSC.data[i][j];
			plDvoMatrix[i*4 + j] = stMatrix4_DVI_CSC.data[i][j];
		}
	}

	return BERR_SUCCESS;
}

/***************************************************************************
 * {private}
 *
 * Takes a RGBW array and outputs a 3x3 NPM matrix.
 */
void BCSC_P_Matrix_RGBW_To_NPM(uint32_t *pulRGBW, uint32_t ulRGBWFractBits, BCSC_P_Matrix *pNPM, uint32_t ulNPMFractBits)
{
	uint32_t ulSize = 3;
	uint32_t ulRx = BCSC_P_FIX_CONVERT(pulRGBW[0], ulRGBWFractBits, ulNPMFractBits);
	uint32_t ulRy = BCSC_P_FIX_CONVERT(pulRGBW[1], ulRGBWFractBits, ulNPMFractBits);
	uint32_t ulGx = BCSC_P_FIX_CONVERT(pulRGBW[2], ulRGBWFractBits, ulNPMFractBits);
	uint32_t ulGy = BCSC_P_FIX_CONVERT(pulRGBW[3], ulRGBWFractBits, ulNPMFractBits);
	uint32_t ulBx = BCSC_P_FIX_CONVERT(pulRGBW[4], ulRGBWFractBits, ulNPMFractBits);
	uint32_t ulBy = BCSC_P_FIX_CONVERT(pulRGBW[5], ulRGBWFractBits, ulNPMFractBits);
	uint32_t ulWx = BCSC_P_FIX_CONVERT(pulRGBW[6], ulRGBWFractBits, ulNPMFractBits);
	uint32_t ulWy = BCSC_P_FIX_CONVERT(pulRGBW[7], ulRGBWFractBits, ulNPMFractBits);
	BCSC_P_Matrix stMatrixP;
	BCSC_P_Matrix stMatrixPInv;
	BCSC_P_Matrix stMatrixC;
	BCSC_P_Vector stVectorW;
	BCSC_P_Vector stVectorCRGB;

	uint32_t ulFixOne = BCSC_P_ITOFIX(1, ulNPMFractBits);

	/* Matrix P */
	stMatrixP.ulSize = ulSize;
	stMatrixP.ulFractBits = ulNPMFractBits;
	stMatrixP.data[0][0] = ulRx;
	stMatrixP.data[0][1] = ulRy;
	stMatrixP.data[0][2] = ulGx;
	stMatrixP.data[1][0] = ulGy;
	stMatrixP.data[1][1] = ulBx;
	stMatrixP.data[1][2] = ulBy;
	stMatrixP.data[2][0] = ulFixOne - (ulRx + ulRy);
	stMatrixP.data[2][1] = ulFixOne - (ulGx + ulGy);
	stMatrixP.data[2][2] = ulFixOne - (ulBx + ulBy);

	/* Vector W */
	stVectorW.ulSize = ulSize;
	stVectorW.ulFractBits = ulNPMFractBits;
	/* ulWx / ulWy */
	stVectorW.data[0] = BCSC_P_FIX_DIV(ulWx, ulWy, ulNPMFractBits);
	/* 1 */
	stVectorW.data[1] = ulFixOne;
	/* (1 - ulWx - ulWy) / ulWy */
	stVectorW.data[2] = BCSC_P_FIX_DIV((ulFixOne - ulWx - ulWy), ulWy, ulNPMFractBits);
	
	/* Vector C RGB */
	stVectorCRGB.ulSize = ulSize;
	stVectorCRGB.ulFractBits = ulNPMFractBits;
	BCSC_P_Matrix_Inverse(&stMatrixP, &stMatrixPInv);
	BCSC_P_Matrix_MultVector(&stMatrixPInv, &stVectorW, &stVectorCRGB);

	/* Matrix C */
	stMatrixC.ulSize = ulSize;
	stMatrixC.ulFractBits = ulNPMFractBits;
	stMatrixC.data[0][0] = stVectorCRGB.data[0];
	stMatrixC.data[0][1] = 0;
	stMatrixC.data[0][2] = 0;
	stMatrixC.data[1][0] = 0;
	stMatrixC.data[1][1] = stVectorCRGB.data[1];
	stMatrixC.data[1][2] = 0;
	stMatrixC.data[2][0] = 0;
	stMatrixC.data[2][1] = 0;
	stMatrixC.data[2][2] = stVectorCRGB.data[2];

	BCSC_P_Matrix_Mult(&stMatrixP, &stMatrixC, pNPM);
}

/***************************************************************************
 * {private}
 *
 * Takes a 3x3 NPM matrix and outputs a 3x3 RGBPrime_To_YCbCrPrime matrix.
 */
void BCSC_P_Matrix_NPM_To_RGBPrime_To_YCbCrPrime(BCSC_P_Matrix *pNPM, BCSC_P_Matrix *pRetMatrix)
{
	uint32_t ulFractBits = pNPM->ulFractBits;
	uint32_t ulFixOne = BCSC_P_ITOFIX(1, ulFractBits);

	pRetMatrix->ulSize = 3;
	pRetMatrix->ulFractBits = ulFractBits;
	pRetMatrix->data[0][0] = pNPM->data[1][0];
	pRetMatrix->data[0][1] = pNPM->data[1][1];
	pRetMatrix->data[0][2] = pNPM->data[1][2];
	pRetMatrix->data[1][0] = BCSC_P_FIX_DIV(-(pNPM->data[1][0]), 2*(ulFixOne - pNPM->data[1][2]), ulFractBits);
	pRetMatrix->data[1][1] = pNPM->data[1][1];
	pRetMatrix->data[1][2] = ulFixOne / 2;
	pRetMatrix->data[2][0] = ulFixOne / 2;
	pRetMatrix->data[2][1] = BCSC_P_FIX_DIV(-(pNPM->data[1][1]), 2*(ulFixOne - pNPM->data[1][0]), ulFractBits);
	pRetMatrix->data[2][2] = BCSC_P_FIX_DIV(-(pNPM->data[1][2]), 2*(ulFixOne - pNPM->data[1][0]), ulFractBits);
}

/***************************************************************************
 * {private}
 *
 * Takes two matrices and multiplies them together.  Result is a matrix
 * with the same size and fixed point fractional bits.
 */
void BCSC_P_Matrix_Mult(BCSC_P_Matrix *pMatrix1, BCSC_P_Matrix *pMatrix2, BCSC_P_Matrix *pRetMatrix)
{
	BCSC_P_Matrix stMatrixTemp;
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
				stMatrixTemp.data[i][j] += (unsigned)BCSC_P_FIX_MUL((signed)pMatrix1->data[i][k],  (signed)pMatrix2->data[k][j], ulFractBits);
			}
		}
	}

	BKNI_Memcpy(pRetMatrix, &stMatrixTemp, sizeof(stMatrixTemp));
}

/***************************************************************************
 * {private}
 *
 * Multiplies a Matrix with a Vector.  Result is a vector with the same
 * size as the original vector and same fixed point fractional bits.
 */
void BCSC_P_Matrix_MultVector(BCSC_P_Matrix *pMatrix, BCSC_P_Vector *pVector, BCSC_P_Vector *pRetVector)
{
	BCSC_P_Vector stTempVector;
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
			stTempVector.data[i] += BCSC_P_FIX_MUL(pMatrix->data[i][k],  pVector->data[k], ulFractBits);
		}
	}

	BKNI_Memcpy(pRetVector, &stTempVector, sizeof(stTempVector));
}

/***************************************************************************
 * {private}
 *
 * Takes a 3x3 matrix and converts it to a 4x4 matrix.
 */
void BCSC_P_Matrix_Make4x4(BCSC_P_Matrix *pMatrix, BCSC_P_Matrix *pRetMatrix)
{
	uint32_t ulFractBits = pMatrix->ulFractBits;
	uint32_t i = 0;
	uint32_t j = 0;

	BDBG_ASSERT(pMatrix->ulSize == 3);

	BKNI_Memset(pRetMatrix, 0, sizeof(BCSC_P_Matrix));
	pRetMatrix->ulSize = 4;
	pRetMatrix->ulFractBits = ulFractBits;
	pRetMatrix->data[3][3] = BCSC_P_ITOFIX(1, ulFractBits);

	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			pRetMatrix->data[i][j] = pMatrix->data[i][j];
		}
	}
}

/***************************************************************************
 * {private}
 *
 * Produces a 4x4 GScale matrix.
 */
void BCSC_P_Matrix_GScale(uint32_t ulN, BCSC_P_Matrix *pRetMatrix, uint32_t ulFractBits)
{
	uint32_t ulFixOne = BCSC_P_ITOFIX(1, ulFractBits);

	BKNI_Memset(pRetMatrix, 0, sizeof(BCSC_P_Matrix));
	pRetMatrix->ulSize = 4;
	pRetMatrix->ulFractBits = ulFractBits;
	pRetMatrix->data[0][0] = ulFixOne / (219 * (1 << (ulN - 8)));
	pRetMatrix->data[1][1] = ulFixOne / (224 * (1 << (ulN - 8)));
	pRetMatrix->data[2][2] = ulFixOne / (224 * (1 << (ulN - 8)));
	pRetMatrix->data[0][3] = (signed) BCSC_P_ITOFIX(-16, ulFractBits) / 219;
	pRetMatrix->data[1][3] = (signed) BCSC_P_ITOFIX(-128, ulFractBits) / 224;
	pRetMatrix->data[2][3] = (signed) BCSC_P_ITOFIX(-128, ulFractBits) / 224;
	pRetMatrix->data[3][3] = ulFixOne;
}

/***************************************************************************
 * {private}
 *
 * Produces a 4x4 inverted GScale matrix.  We calculate the GScale
 * inverse matrix directly because it's easy to calculate and the
 * result is more accurate than using the inverse matrix algorithm.
 * This also avoids the problem of the determinant being too small
 * for the fixed-point representation.
 */
void BCSC_P_Matrix_GScaleInverse(uint32_t ulN, BCSC_P_Matrix *pRetMatrix, uint32_t ulFractBits)
{
	uint32_t ulFixOne = BCSC_P_ITOFIX(1, ulFractBits);

	BKNI_Memset(pRetMatrix, 0, sizeof(BCSC_P_Matrix));
	pRetMatrix->ulSize = 4;
	pRetMatrix->ulFractBits = ulFractBits;
	pRetMatrix->data[0][0] = BCSC_P_ITOFIX(219 * (1 << (ulN - 8)), ulFractBits);
	pRetMatrix->data[1][1] = BCSC_P_ITOFIX(224 * (1 << (ulN - 8)), ulFractBits);
	pRetMatrix->data[2][2] = BCSC_P_ITOFIX(224 * (1 << (ulN - 8)), ulFractBits);
	pRetMatrix->data[0][3] = BCSC_P_ITOFIX((1 << (ulN - 4)), ulFractBits);
	pRetMatrix->data[1][3] = BCSC_P_ITOFIX((1 << (ulN - 1)), ulFractBits);
	pRetMatrix->data[2][3] = BCSC_P_ITOFIX((1 << (ulN - 1)), ulFractBits);
	pRetMatrix->data[3][3] = ulFixOne;
}

/***************************************************************************
 * {private}
 *
 * Produces a 4x4 GGain matrix.
 */
void BCSC_P_Matrix_GGain(uint32_t ulN, BCSC_P_Matrix *pRetMatrix, uint32_t ulFractBits)
{
	uint32_t ulFixOne = BCSC_P_ITOFIX(1, ulFractBits);

	BKNI_Memset(pRetMatrix, 0, sizeof(BCSC_P_Matrix));
	pRetMatrix->ulSize = 4;
	pRetMatrix->ulFractBits = ulFractBits;
	pRetMatrix->data[0][0] = BCSC_P_ITOFIX(1 >> ulN, ulFractBits) - ulFixOne;
	pRetMatrix->data[1][1] = BCSC_P_ITOFIX(1 >> ulN, ulFractBits) - ulFixOne;
	pRetMatrix->data[2][2] = BCSC_P_ITOFIX(1 >> ulN, ulFractBits) - ulFixOne;
	pRetMatrix->data[3][3] = ulFixOne;
}

/***************************************************************************
 * {private}
 *
 * Calculates the cofactor of a matrix at a specific position.
 */
uint32_t BCSC_P_Matrix_Cofactor(BCSC_P_Matrix *pMatrix, uint32_t ulRow, uint32_t ulCol)
{
	uint32_t ulSignPos = (ulCol + ulRow) & 1;
	uint32_t ulSize = pMatrix->ulSize;
	uint32_t ulFractBits = pMatrix->ulFractBits;
	uint32_t i = 0;
	uint32_t j = 0;
	uint32_t ulSrcRow = 0;
	uint32_t ulSrcCol = 0;
	uint32_t ulCofactor = 0;

	BCSC_P_Matrix stTempMatrix;

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
	ulCofactor = BCSC_P_Matrix_Determinant(&stTempMatrix, NULL);

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
uint32_t BCSC_P_Matrix_Determinant(BCSC_P_Matrix *pMatrix, BCSC_P_Matrix *pCofactors)
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
		ulDeterminant = BCSC_P_FIX_MUL(pMatrix->data[0][0], pMatrix->data[1][1], ulFractBits) -
			            BCSC_P_FIX_MUL(pMatrix->data[0][1], pMatrix->data[1][0], ulFractBits);
	}
	else
	{
		/* add cofactors along top row */
		for (ulCol = 0; ulCol < ulSize; ulCol++)
		{
			if (pCofactors)
			{
				/* use precomputed cofactors */
				ulDeterminant += BCSC_P_FIX_MUL(pMatrix->data[0][ulCol], pCofactors->data[0][ulCol], ulFractBits);
			}
			else
			{
				ulDeterminant += BCSC_P_FIX_MUL(pMatrix->data[0][ulCol], BCSC_P_Matrix_Cofactor(pMatrix, 0, ulCol), ulFractBits);
			}
		}
	}

	return ulDeterminant;
}

/***************************************************************************
 * {private}
 *
 * Creates a matrix holding all cofactors of a matrix.
 */
void BCSC_P_Matrix_CofactorMatrix(BCSC_P_Matrix *pMatrix, BCSC_P_Matrix *pCofactors)
{
	uint32_t ulRow;
	uint32_t ulCol;
	uint32_t ulSize = pMatrix->ulSize;
	uint32_t ulFractBits = pMatrix->ulFractBits;

	for (ulRow = 0; ulRow < ulSize; ulRow++)
	{
		for (ulCol = 0; ulCol < ulSize; ulCol++)
		{
			pCofactors->data[ulRow][ulCol] = BCSC_P_Matrix_Cofactor(pMatrix, ulRow, ulCol);
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
void BCSC_P_Matrix_Transpose(BCSC_P_Matrix *pMatrix, BCSC_P_Matrix *pRetMatrix)
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
void BCSC_P_Matrix_MultScalar(BCSC_P_Matrix *pMatrix, uint32_t ulScalar, BCSC_P_Matrix *pRetMatrix)
{
	uint32_t ulRow;
	uint32_t ulCol;
	uint32_t ulSize = pMatrix->ulSize;
	uint32_t ulFractBits = pMatrix->ulFractBits;

	for (ulRow = 0; ulRow < ulSize; ulRow++)
	{
		for (ulCol = 0; ulCol < ulSize; ulCol++)
		{
			pRetMatrix->data[ulRow][ulCol] = BCSC_P_FIX_MUL(pMatrix->data[ulRow][ulCol], ulScalar, ulFractBits);
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
void BCSC_P_Matrix_Inverse(BCSC_P_Matrix *pMatrix, BCSC_P_Matrix *pRetMatrix)
{
	uint32_t ulFractBits = pMatrix->ulFractBits;
	BCSC_P_Matrix stCofactorMatrix;
	BCSC_P_Matrix stAdjointMatrix;
	uint32_t ulDeterminant;
	uint32_t ulOneOverDeterminant;
	uint32_t ulFixOne = BCSC_P_ITOFIX(1, ulFractBits);

	/* InvM = 1/det(M) * Transpose(Cofactor(M)) */

	/* compute all cofactors */
	BCSC_P_Matrix_CofactorMatrix(pMatrix, &stCofactorMatrix);
	ulDeterminant = BCSC_P_Matrix_Determinant(pMatrix, &stCofactorMatrix);
	BDBG_ASSERT(ulDeterminant);
	ulOneOverDeterminant = BCSC_P_FIX_DIV(ulFixOne, ulDeterminant, ulFractBits);
	BCSC_P_Matrix_Transpose(&stCofactorMatrix, &stAdjointMatrix);
	BCSC_P_Matrix_MultScalar(&stAdjointMatrix, ulOneOverDeterminant, pRetMatrix);
}

void BCSC_P_Matrix_Dump(BCSC_P_Matrix *pMatrix)
{
	uint32_t i = 0;
	uint32_t j = 0;

	for (i = 0; i < pMatrix->ulSize; i++)
	{
		for (j = 0; j < pMatrix->ulSize; j++)
		{
			BKNI_Printf("0x%x ", pMatrix->data[i][j]);
		}
		BKNI_Printf("\n");
	}
	BKNI_Printf("\n");
}

/* End of File */

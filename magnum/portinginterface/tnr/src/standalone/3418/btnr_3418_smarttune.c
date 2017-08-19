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
 *****************************************************************************/
#include "bstd.h"
#include "bmth.h"
#include "bkni.h"
#include "btnr.h"
#include "btnr_priv.h"
#include "btnr_3418.h"
#include "btnr_3418_smarttune.h"
#include "btnr_3418_regio.h"


BDBG_MODULE(btnr_3418_smarttune);

#define ATLTRACE(x)             BDBG_ERR((x))

#define BCM3418AX_REV_LETTER    (0)
#define BCM3418A0_REV_NUMBER    (0)
#define BCM3418A1_REV_NUMBER    (1)
#define BCM3418A2_REV_NUMBER    (2)

#define BCM3418BX_REV_LETTER    (1)
#define BCM3418B0_REV_NUMBER    (0)
#define BCM3418B1_REV_NUMBER    (1)
#define BCM3418B2_REV_NUMBER    (2)

#define g_cnSigmaDelta1  16033;                 /* First sigma delta feedback coefficient */

enum { TGC_AGC_CONTINUOUS = 1, TGC_AGC_HOLD_WHEN_TUNE = 2, TGC_MANUAL_GAIN = 4 };

static int Bcm_Abs( int val )
{
    return( (val < 0)? -val: val );
}

static int Bcm_Lng_Abs( long val )
{
    return( (val < 0)? -val: val );
}

static void CPLL1_initDefault( CPLL1 *pcPLL1 )
{
    BKNI_Memset( pcPLL1, 0x00, sizeof(CPLL1) );
    pcPLL1->m_nFreq             = 2100000000;
    pcPLL1->m_nVCO12XOvr        = 1600000000;   /* VCO1 to VCO2 X-Over freq. */
    pcPLL1->m_nVCO23XOvr        = 2000000000;   /* VCO2 to VCO3 X-Over freq. */
    pcPLL1->m_iN                = 43;
    pcPLL1->m_iFrac             = 0;
    pcPLL1->m_iOffset           = 0;

}
static void CPLL2_initDefault( CPLL2 *pcPLL2 )
{
    BKNI_Memset( pcPLL2, 0x00, sizeof(CPLL2) );
    pcPLL2->m_iN        = 18;   /* + 2 = Ref */
    pcPLL2->m_iM        = 488;  /* + 2 = Actual N. */
    pcPLL2->m_nFreq     = 1176000000;   
}

static void CDAC_initDefault( CDAC *pcDAC )
{
    BKNI_Memset( pcDAC, 0x00, sizeof(CDAC) );
    pcDAC->m_bForceVga = false;
    pcDAC->m_iForceVal = 0;
    pcDAC->m_iAGCVal   = 0;
}

static long IntegersDivision( long nDividend, long nDivisor )
{
    long nQuotient, nRemainder, nHalfDivisor;

    nHalfDivisor = nDivisor / 2;
    nQuotient = nDividend / nDivisor;
    nRemainder = nDividend % nDivisor;
    if (nRemainder >= nHalfDivisor)
        nQuotient++;
    return nQuotient;
}

static long UnsignedIntegersDivision( unsigned long nDividend, long nDivisor )
{
    long nQuotient, nRemainder, nHalfDivisor;

    nHalfDivisor = nDivisor / 2;
    nQuotient = nDividend / nDivisor;
    nRemainder = nDividend % nDivisor;
    if (nRemainder >= nHalfDivisor)
        nQuotient++;
    return nQuotient;
}

static bool IsElementZero(long* pnArry, long nElem)
{
    long i;

    for (i=0; i<nElem; i++)
    {
        if ( 0 != pnArry[i] )
            return 1;
    }

    return 0;
}
static void freqs( long nRF, long nIF1, long *nCoarse, long *nFrac, long *nOffset )
{
    long nFrac0;

    *nCoarse = IntegersDivision(nIF1 + nRF, 1000000) * 1000000;
    nFrac0  = nIF1 + nRF - *nCoarse;
    if ( (nFrac0 > 0) && (nFrac0 <= 250000) )
        *nOffset = 500000;
    else if ( Bcm_Lng_Abs(nFrac0) > 250000 )
        *nOffset = 0;
    else if ( (nFrac0 <= 0) && (nFrac0 >= -250000) )
        *nOffset = -500000;
    *nFrac = nFrac0 - *nOffset;
}

static void fisBad( long nRF, long nLO1, long nIF2, 
                     long nHalfChannel, long nMarginInt, 
                     long nMarginHalf, long nMarginQtr, bool relaxSpurs, 
                     bool relaxAll, long* pbadType )
{
    long fIF1, fLO2, beatBP2M3, beatBP4M5, beatBP1M2P1;
    long val;
    bool BP2M3A, BP2M3B, BP4M5A, BP4M5B, BP1M2P1A, BP1M2P1B, nearInt, nearHalf, nearQtr;
    
    fIF1        = nLO1 - nRF;
    fLO2        = fIF1 - nIF2;
    beatBP2M3   = (2 * nLO1) - (3 * fLO2);
    beatBP4M5   = (4 * nLO1) - (5 * fLO2);
    beatBP1M2P1 = nLO1 - (2 * fLO2) + nRF;

    BP2M3A   = Bcm_Lng_Abs(beatBP2M3 + nIF2) < nHalfChannel;
    BP2M3B   = Bcm_Lng_Abs(beatBP2M3 - nIF2) < nHalfChannel;
    BP4M5A   = Bcm_Lng_Abs(beatBP4M5 + nIF2) < nHalfChannel;
    BP4M5B   = Bcm_Lng_Abs(beatBP4M5 - nIF2) < nHalfChannel;
    BP1M2P1A = Bcm_Lng_Abs(beatBP1M2P1 - nIF2) < nHalfChannel;
    BP1M2P1B = Bcm_Lng_Abs(beatBP1M2P1 + nIF2) < nHalfChannel;

    val = IntegersDivision(nLO1, 1000000) * 1000000;
    val = nLO1 - val;
    nearInt  = Bcm_Lng_Abs(val) < nMarginInt;

    val = IntegersDivision(nLO1 * 2, 1000000) * 1000000;
    val = nLO1 - (val / 2);
    nearHalf = Bcm_Lng_Abs(val) < nMarginHalf;

    val = IntegersDivision(nLO1 * 4, 1000000) * 1000000;
    val = nLO1 - (val / 4);
    nearQtr  = Bcm_Lng_Abs(val) < nMarginQtr;
    if ( !(relaxSpurs | relaxAll) )
    {
        pbadType[0] = BP2M3A;
        pbadType[1] = BP2M3B;
        pbadType[2] = BP4M5A;
        pbadType[3] = BP4M5B;
        pbadType[4] = BP1M2P1A;
        pbadType[5] = BP1M2P1B;
        pbadType[6] = nearInt;
        pbadType[7] = nearHalf;
        pbadType[8] = nearQtr;
    }
    else if ( relaxSpurs && !relaxAll )
    {
        pbadType[0] = BP2M3A;
        pbadType[1] = BP2M3B;
        pbadType[2] = 0;
        pbadType[3] = 0;
        pbadType[4] = 0;
        pbadType[5] = 0;
        pbadType[6] = nearInt;
        pbadType[7] = nearHalf;
        pbadType[8] = nearQtr;
    }
    else if ( relaxAll )
    {
        pbadType[0] = 0;
        pbadType[1] = 0;
        pbadType[2] = 0;
        pbadType[3] = 0;
        pbadType[4] = 0;
        pbadType[5] = 0;
        pbadType[6] = 0;
        pbadType[7] = 0;
        pbadType[8] = 0;
    }
}

static void makeCommands( long nCoarse, long nFrac, long nOffset, 
                                long nIF1, long nIF2, long nM2, long nStep2,
                                int *commandFrac, int *commandInt, int *commandOffset,
                                int *commandM2, int *commandN2 )
{
    int nSign;
    unsigned long uval;

    if ( nFrac >= 0 )
        nSign = 1;
    else
        nSign = -1;
    
    uval = (Bcm_Lng_Abs(nFrac) / 2) * g_cnSigmaDelta1;
    *commandFrac = UnsignedIntegersDivision(uval, 1000000) * nSign;
    /*adjust to obtain register values */
    *commandInt   = (nCoarse / 1000000) - 9;        /* 0A<6:0> */
    *commandOffset = (nOffset * 2) / 1000000;
    *commandM2    = nM2 - 2;       /* 11<4:0> */
    *commandN2    = ((nIF1 - nIF2) / nStep2) - 2;

    /* convert to 2's complement */
    if ( *commandFrac < 0 )
        *commandFrac = (Bcm_Abs(*commandFrac) ^ 0x1FFF) + 1;    /* [0C<5:1> 0B<7:0>] */
    if ( *commandOffset < 0 )
        *commandOffset = 2;
}

/* XTAL Functions ------------------------------------------------------ */
static long XTAL_GetFreq( BTNR_P_3418_Settings *pTnrImplData )
{
    long nXtalFreq = 24000000;
    int    iXtalFreq = 0;
    uint8_t bytData;

    BTNR_P_3418_ReadField8( pTnrImplData, "XTAL_FREQ", &bytData );
    iXtalFreq = bytData;
    nXtalFreq = 24000000 + 5000000 * iXtalFreq;

    return nXtalFreq;
}

static void XTAL_SetFreq( BTNR_P_3418_Settings *pTnrImplData, long nXtalFreq )
{
    int iXtalFreq;

    if ( 24000000 == nXtalFreq )      /*The default is 24.0 MHz.*/
    {
        return;
    }
    iXtalFreq = (nXtalFreq - 24000000) * 2;

    BTNR_P_3418_WriteField8( pTnrImplData, "XTAL_FREQ", (uint8_t)iXtalFreq );
}

/* PLL1 Functions ------------------------------------------------------ */
static void PLL1_Aquire( BTNR_P_3418_Settings *pTnrImplData )
{
    BTNR_P_3418_WriteField8( pTnrImplData, "PRGMV",        (uint8_t)0 );
    BTNR_P_3418_WriteField8( pTnrImplData, "SD_loadMod",   (uint8_t)0 );
    BTNR_P_3418_WriteField8( pTnrImplData, "SD_resetModb", (uint8_t)1 );
    BTNR_P_3418_WriteField8( pTnrImplData, "SD_resetModb", (uint8_t)0 );
    BTNR_P_3418_WriteField8( pTnrImplData, "SD_loadMod",   (uint8_t)1 );
    BTNR_P_3418_WriteField8( pTnrImplData, "SD_loadMod",   (uint8_t)0 );
    BTNR_P_3418_WriteField8( pTnrImplData, "SD_resetModb", (uint8_t)1 );
    BTNR_P_3418_WriteField8( pTnrImplData, "SD_loadMod",   (uint8_t)1 );
    BTNR_P_3418_WriteField8( pTnrImplData, "PRGMV",        (uint8_t)1 );
    BTNR_P_3418_WriteField8( pTnrImplData, "PRGMV",        (uint8_t)0 );
}

static void PLL1_SetVco( BTNR_P_3418_Settings *pTnrImplData, int iVCO )
{   /* Simply set the PRGVCO register, DO NOT tune the VCOs. */
    /* Validate the Range. */
    if ( iVCO < 1 )
        iVCO = 1;
    else if ( iVCO > 3 )
        iVCO = 3;

    pTnrImplData->m_cPLL1.m_iVCO = iVCO;      /* Save value in CPLL1 class. */
    /* Set the PRGVCO register. */
    BTNR_P_3418_WriteField8( pTnrImplData, "PRGVCO", (uint8_t)iVCO );
}

static void PLL1_SelVco( BTNR_P_3418_Settings *pTnrImplData, unsigned long unLO )
{
    if ( unLO < pTnrImplData->m_cPLL1.m_nVCO12XOvr )
        PLL1_SetVco( pTnrImplData, 1 );         /* Set VCO 1. */
    else
    {
        if ( unLO <= pTnrImplData->m_cPLL1.m_nVCO23XOvr )
            PLL1_SetVco( pTnrImplData, 2 );     /* Set VCO 2. */
        else
            PLL1_SetVco( pTnrImplData, 3 );     /* Set VCO 3. */
    }
}

static void PLL1_SetFreq( BTNR_P_3418_Settings *pTnrImplData, unsigned long unLO, int iN, int iFrac, int iOffset )
{
    pTnrImplData->m_cPLL1.m_iN      = iN;
    pTnrImplData->m_cPLL1.m_iFrac   = iFrac;
    pTnrImplData->m_cPLL1.m_iOffset = iOffset;
    pTnrImplData->m_cPLL1.m_nFreq   = unLO;

    PLL1_SelVco( pTnrImplData, unLO );
    BTNR_P_3418_WriteField8( pTnrImplData, "NPLL1", (uint8_t)iN );
    BTNR_P_3418_WriteField16( pTnrImplData,  "FracPLL1", (uint16_t)iFrac );
    BTNR_P_3418_WriteField8( pTnrImplData, "FracOffsetPLL1", (uint8_t)iOffset );
    PLL1_Aquire( pTnrImplData );
}

/* PLL2 Functions ------------------------------------------------------ */
static void PLL2_Reset( BTNR_P_3418_Settings *pTnrImplData  )
{   /* Reset and Aquire lock. */
    BTNR_P_3418_WriteField8( pTnrImplData, "PLL2_RESETB", (uint8_t)1 );  /* High. */
    BTNR_P_3418_WriteField8( pTnrImplData, "PLL2_RESETB", (uint8_t)0 );  /* Low. */
    BTNR_P_3418_WriteField8( pTnrImplData, "PLL2_RESETB", (uint8_t)1 );  /* High. */
}

static long PLL2_CalcFreqHz( int iM, int iN, long nXtal )
{
    long nLO, nRef;

    nRef = nXtal / (iM + 2);
    nLO  = nRef * (iN + 2) * 4;

    return nLO;
}

static long PLL2_GetFreqHz( BTNR_P_3418_Settings *pTnrImplData, bool bReadReg)
{
    long nXtal;
    long nLO;


    if ( bReadReg )
    {
        uint8_t bytData;
        uint16_t wData;

        BTNR_P_3418_ReadField8( pTnrImplData, "MPLL2", &bytData );
        pTnrImplData->m_cPLL2.m_iM = bytData;
        BTNR_P_3418_ReadField16( pTnrImplData, "NPLL2", &wData );
        pTnrImplData->m_cPLL2.m_iN = wData;
    }
    nXtal = XTAL_GetFreq( pTnrImplData );
    nLO   = PLL2_CalcFreqHz( pTnrImplData->m_cPLL2.m_iM, pTnrImplData->m_cPLL2.m_iN, nXtal );

    return nLO;
}

static void PLL2_SetFreq( BTNR_P_3418_Settings *pTnrImplData, int M, int N )
{
    pTnrImplData->m_cPLL2.m_iM = M;
    pTnrImplData->m_cPLL2.m_iN = N;

    BTNR_P_3418_WriteField8( pTnrImplData, "MPLL2", (uint8_t)M );
    BTNR_P_3418_WriteField16( pTnrImplData, "NPLL2", (uint16_t)N );
    PLL2_Reset( pTnrImplData );                     /* Reset and Aquire. */
    pTnrImplData->m_cPLL2.m_nFreq = PLL2_GetFreqHz( pTnrImplData, false );    /* don't read registers */
}

static int GetChipRev( BTNR_P_3418_Settings *pTnrImplData, int *piLetter, int *piNumber, int *piType )
{
    uint8_t bytData;

    BTNR_P_3418_ReadField8( pTnrImplData, "REVISION_LETTER", &bytData );
    *piLetter = bytData;
    BTNR_P_3418_ReadField8( pTnrImplData, "REVISION_NUMBER", &bytData );
    *piNumber = bytData;
    BTNR_P_3418_ReadField8( pTnrImplData, "CHIP_TYPE", &bytData );
    *piType = bytData;

    return *piLetter;
}

static void DET_Pwr( BTNR_P_3418_Settings *pTnrImplData, bool bON )
{
    uint8_t bytData;

    /* Set to 1 to power down control for the power detector (1220MHz) */
    bytData = (true == bON) ? 0 : 1;
    BTNR_P_3418_WriteField8( pTnrImplData, "PD_DET0", bytData );
}

/* DAC Functions ------------------------------------------------------ */
static void DAC_Pwr( BTNR_P_3418_Settings *pTnrImplData, bool bON )
{
    uint8_t bytData;
    /*Set to 1 to turn the DAC and comparator off*/

    bytData = (true == bON) ? 0 : 1;
    BTNR_P_3418_WriteField8( pTnrImplData, "PD_DAC", bytData );
}

static void DAC_Write( BTNR_P_3418_Settings *pTnrImplData, int iVal )
{
    BTNR_P_3418_WriteField8( pTnrImplData, "DAC", (uint8_t)iVal );     /* Set DAC register. */
}

static void DAC_ForceVGA( BTNR_P_3418_Settings *pTnrImplData,  int iVal )
{
    if ( iVal < 0 )
        iVal = 0;
    else if ( iVal > eVGA_MAX )
        iVal = eVGA_MAX;
    DET_Pwr( pTnrImplData, false );     /* Power OFF Detector. 8/8/03 */
    DAC_Pwr( pTnrImplData, true );      /* Power ON DAC. */
    DAC_Write( pTnrImplData, iVal );    /* Set to Force value. */
    BTNR_P_3418_WriteField8( pTnrImplData, "IF1_GAIN_FORCE", (uint8_t)1 );   /* Force VGA */
    pTnrImplData->m_cDAC.m_bForceVga = true;
    pTnrImplData->m_cDAC.m_iForceVal = iVal;
}

static int DAC_Read( BTNR_P_3418_Settings *pTnrImplData )
{
    int iDACOut = -1;   /* error when return negative value. */
    uint8_t bytData;

    BTNR_P_3418_ReadField8( pTnrImplData, "DAC", &bytData );    /* Read DAC output. */
    iDACOut = bytData;

    return iDACOut;
}

static int DAC_SampleComp( BTNR_P_3418_Settings *pTnrImplData )
{
    int iCompOut;
    uint8_t bytData;


    BTNR_P_3418_WriteField8( pTnrImplData, "DAC_latchData", (uint8_t)0 );  /* Reset the comparator latch. */
    BTNR_P_3418_WriteField8( pTnrImplData, "DAC_latchData", (uint8_t)1 );  /* Sample and Latch. */
    BTNR_P_3418_ReadField8( pTnrImplData, "COMP_OUT", &bytData );       /* Read the comparator's output. */
    iCompOut = bytData;

    return iCompOut;
}

static void DAC_SelPwrDet( BTNR_P_3418_Settings *pTnrImplData )
{
    /* Select the Calibration Signal.  Sets all the Selection bits high,
       the default state.  Device selection bits are active low but when selected
       all other selection bits must be set high. */

    BTNR_P_3418_WriteField8( pTnrImplData, "DAC_refSel", (uint8_t)1 );   /* '1' selects the power detector. */
    BTNR_P_3418_WriteField8( pTnrImplData, "DACiSel",    (uint8_t)1 );   /* '1' selects CTAT, '0' selects PTAT. */
    BTNR_P_3418_WriteField8( pTnrImplData, "DACselPLb",  (uint8_t)1 );   /* '0' selects Poly Low */
    BTNR_P_3418_WriteField8( pTnrImplData, "DACselPHb",  (uint8_t)1 );   /* '0' selects Poly High */
    BTNR_P_3418_WriteField8( pTnrImplData, "DACselNMb",  (uint8_t)1 );   /* '0' selects NMOS */
    BTNR_P_3418_WriteField8( pTnrImplData, "DACselPMb",  (uint8_t)1 );   /* '0' selects PMOS */
}

static void DAC_Sel( BTNR_P_3418_Settings *pTnrImplData, eDAC_Device eDev, eDAC_Device eTAT )
{   /* Select which device to measure.
       eTAT - Selects either PTAT of CTAT, see eDAC_Device definition.
       eDev - Selects the device to measure: PL, PH, PMOS, or NMOS. */

    DAC_SelPwrDet( pTnrImplData );    /* Sets all bits high. */

    BTNR_P_3418_WriteField8( pTnrImplData, "DACiSel", eTAT==eDAC_PTAT?(uint8_t)0:(uint8_t)1);
    BTNR_P_3418_WriteField8( pTnrImplData, "DAC_refSel", (uint8_t)0 );     /* if high other bits are don't care and */
                                                /* the power detector is selected.  
                                                   All other selection bits are high
                                                   => measuring calibration input. */
    switch( eDev )
    {
        case eDAC_CAL:   /* Nothing to do; all bits high. */
            break;
        case eDAC_PL:
            BTNR_P_3418_WriteField8( pTnrImplData, "DACselPLb",  (uint8_t)0 );
            break;
        case eDAC_PH:
            BTNR_P_3418_WriteField8( pTnrImplData, "DACselPHb",  (uint8_t)0 );
            break;
        case eDAC_NMOS:
            BTNR_P_3418_WriteField8( pTnrImplData, "DACselNMb",  (uint8_t)0 );
            break;
        case eDAC_PMOS:
            BTNR_P_3418_WriteField8( pTnrImplData, "DACselPMb",  (uint8_t)0 );
            break;
        default:
            break;
    }
}

static bool VGA1_GetForce( BTNR_P_3418_Settings *pTnrImplData, int *piVal, bool bRead )
{
    bool bForce = false;

    if ( bRead )
    {
        uint8_t bytData;

        BTNR_P_3418_ReadField8( pTnrImplData, "IF1_GAIN_FORCE", &bytData );
        bForce = (1 == bytData) ? true : false ;
        pTnrImplData->m_cDAC.m_bForceVga = bForce;
        if ( bForce )
        {
            BTNR_P_3418_ReadField8( pTnrImplData, "DAC", &bytData );
            *piVal             = bytData;
            pTnrImplData->m_cDAC.m_iForceVal = *piVal;
        }
        else
            *piVal = pTnrImplData->m_cDAC.m_iForceVal;    /* old force value. */
    }
    else
    {   /* Return previous set Force values.*/
        bForce = pTnrImplData->m_cDAC.m_bForceVga;
        *piVal   = pTnrImplData->m_cDAC.m_iForceVal;
    }

    return bForce;
}

/* DAC Functions ------------------------------------------------------ */
static int DAC_SADC( BTNR_P_3418_Settings *pTnrImplData, int InitVal, bool bLinear )
{
    int iRevL, iRevN, iType;
    int iDACOut = 0;
    int iCompOut= 0;    /* Comparator output: '0' or '1'. */
    int i, iVal;
    int iLowerLim = eDAC_MIN;    /* Initialize Lower Limit */
    int iUpperLim = eDAC_MAX;    /* Initialize Upper Limit */
    int iMid;

    VGA1_GetForce( pTnrImplData, &iVal, true );    /* Reads and saves m_cDAC values. */
    if ( pTnrImplData->m_cDAC.m_bForceVga )
    {   /* Was DAC controlling VGA before function entry? */
        BTNR_P_3418_WriteField8( pTnrImplData, "IF1_GAIN_FORCE", (uint8_t)0 );   /* close AGC loop / Free DAC. */
    }

    GetChipRev( pTnrImplData, &iRevL, &iRevN, &iType );
    if ( bLinear )
    {
        for (i=InitVal; i<=eDAC_MAX; i++)
        {
            DAC_Write( pTnrImplData, i );                   /* Set DAC Register. */
            iCompOut = DAC_SampleComp( pTnrImplData );    /* Sample the comparator. */
            if ( iCompOut == 0 )
            {
                iDACOut = DAC_Read( pTnrImplData );   /* Read value of DAC register. */
                break;
            }
        }
    }
    else
    {   /* Bisection Search ---------- */
        /* This is taken from "Numerical Recipes" pg. 117. */
        while( iUpperLim-iLowerLim > 1 )
      {   /* If we are not yet done, */
            iMid = (iUpperLim+iLowerLim) >> 1;  /* compute a midpoint (2). */
            DAC_Write( pTnrImplData, iMid );                    /* Set the DAC to the mid point */
            if ( DAC_SampleComp( pTnrImplData ) == 0 )   /* Sample signal */
                iUpperLim = iMid;   /* iMid is greater than input signal to the comparator. */
        else
                iLowerLim = iMid;   /* iMid is less than the input signal to the comparator */
        }
        iDACOut = iUpperLim;
    }
    if ( pTnrImplData->m_cDAC.m_bForceVga )
    {   /* Restore VGA control to DAC if required. */
        DAC_ForceVGA( pTnrImplData, pTnrImplData->m_cDAC.m_iForceVal );
    }

    return iDACOut;
}

int DAC_ReadPwrDet( BTNR_P_3418_Settings *pTnrImplData )
{
    /* Returns Power detector level; '-1' if error.
       Powers up the power detector and the DAC to make the
       measurement then returns the Detector and DAC to the
       same ON/OFF state as when the function was entered. */
    int iDet = 0;
    int iDACPwr, iDetPwr;   /* Entry power states. */
    uint8_t bytData;

    BTNR_P_3418_ReadField8( pTnrImplData, "PD_DAC", &bytData );       /* Get Current State */
    iDACPwr = bytData;
    BTNR_P_3418_ReadField8( pTnrImplData, "PD_DET0", &bytData );
    iDetPwr = bytData;

    DAC_Pwr( pTnrImplData, true );
    DET_Pwr( pTnrImplData, true );

    DAC_SelPwrDet( pTnrImplData );
    iDet = DAC_SADC( pTnrImplData, 0, false );                                  /* Read the value at the DAC's Comparator input.  */
    BTNR_P_3418_WriteField8( pTnrImplData, "PD_DAC",  (uint8_t)iDACPwr);   /* Return DAC to original state. */
    BTNR_P_3418_WriteField8( pTnrImplData, "PD_DET0", (uint8_t)iDetPwr);   /* Return DET to original state. */

    return iDet;
}

/* VGA1 Functions ------------------------------------------------------ */
/* *** Call with bActive=false to Lock and then unlock loop. */
/* if bUseLastAGC is true: The Threshold will be SET to the last value passed */
/* to VGA1_SetAGC().  If false, the Threshold will remain at the value in */
/* THRES_DET[]. */
/* Holds the VGA gain at the closed loop value. */
/* Returns current value of power detector; */
/* -1 if could not read the detector. */


static int VGA1_AGC( BTNR_P_3418_Settings *pTnrImplData, int iMode )
{
    int iDet, iAGCCapSettleTime/* , iSettleTime */;
    uint8_t bytThreshold;

    iDet = 0;
    
    switch( pTnrImplData->iRevLetter )
    {
        case BCM3418AX_REV_LETTER:
            DAC_ForceVGA( pTnrImplData, 63 );
            BKNI_Sleep( pTnrImplData->m_iForceSettleTime );
            DET_Pwr( pTnrImplData, true );        /* Power on the detector so loop can operate. */
            BTNR_P_3418_WriteField8( pTnrImplData, "IF1_GAIN_FORCE", (uint8_t) 0 );   /* Do not force VGA */
            BKNI_Sleep( pTnrImplData->m_iAgcSettleTime );            /* Allow loop to settle. */
            iDet = DAC_ReadPwrDet( pTnrImplData );        /* Take reading of power detector. */
            pTnrImplData->m_cDAC.m_bForceVga = false;
            BDBG_MSG(("3418Ax AGC Delay=%d msec", (pTnrImplData->m_iForceSettleTime + pTnrImplData->m_iAgcSettleTime)));
            break;
        case BCM3418BX_REV_LETTER:
            /* Increment Threshold by 1 */
            BTNR_P_3418_ReadField8( pTnrImplData, "THRES_DET0", &bytThreshold );
            bytThreshold = bytThreshold + 1;
            BTNR_P_3418_WriteField8( pTnrImplData, "THRES_DET0", bytThreshold );
            DET_Pwr( pTnrImplData, true ); /* Power on the detector so loop can operate. */
            BTNR_P_3418_WriteField8( pTnrImplData, "IF1_vga_1220_sw", (uint8_t) 1 );
            BKNI_Sleep(30);        /* For PLL Settling */
            BTNR_P_3418_WriteField8( pTnrImplData, "IENV0", (uint8_t) 1 );
            BTNR_P_3418_WriteField8( pTnrImplData, "IF1_vga_1220_sw", (uint8_t) 0 );
            BTNR_P_3418_WriteField8( pTnrImplData, "IF1_GAIN_FORCE", (uint8_t) 0 );
            iAGCCapSettleTime = 30 * (pTnrImplData->m_iAGCCap/10);
            BKNI_Sleep( iAGCCapSettleTime );
            if ( TGC_AGC_HOLD_WHEN_TUNE == iMode )
            {
                iDet = DAC_ReadPwrDet( pTnrImplData );    /* Take reading of power detector. */
            }
            pTnrImplData->m_cDAC.m_bForceVga = false;
            BTNR_P_3418_WriteField8( pTnrImplData, "IENV0", (uint8_t) 0 );
            bytThreshold = bytThreshold - 1; /* Lav 04/07/2005 Decrement threshold by 1 */
            BTNR_P_3418_WriteField8( pTnrImplData,  "THRES_DET0", (uint8_t) bytThreshold );
            /* !!!Embedded code don't have to do these last two steps!!! (Ray Gomez). */
            /*iSettleTime = 200 * (pTnrImplData->m_iAGCCap/10);
            BKNI_Sleep( iSettleTime );*/
            BDBG_MSG(("3418Bx AGC Delay=%d msec", (30 + iAGCCapSettleTime /* + iSettleTime */)));
            break;
    }
    switch( iMode )
    {
        case TGC_AGC_CONTINUOUS:        /* Continuous AGC */
            break;
        case TGC_AGC_HOLD_WHEN_TUNE:    /* AGC when tune    */
            DAC_ForceVGA( pTnrImplData, iDet );
            DET_Pwr( pTnrImplData, false );
            break;
    }

    return iDet;
}

static void NewTune( BTNR_P_3418_Settings *pTnrImplData, long nRF )
{
    long nPLL1RefFreq, nMarginInt, nMarginHalf, nMarginQtr, nMarginLO1LO2,
         nSAWlo, nSAWhi, nbwIF1, nSAWmeasured,
         ndesiredIF1, nHalfChannel, nIF2, nPLL1RefFreqKHz, nPLL1RefFreqMHz;

    bool bad        = 1;
    bool relaxSpurs = 0;
    bool relaxBW    = 0;
    bool relaxAll   = 0;
    bool cannotTune = 0;
    bool temp = 0;
    long badType[9];
    long nCoarse, nFrac, nOffset=0;
    long nSAWlo1, nSAWhi1;
    unsigned long unLO1 = 0;
    
    long i, j, nhalfM2[9];
    long nM2Tmp[9], nstep2Tmp[9], nIF10[9], nIF1p1[9], nIF1m1[9],
             nIF1p2[9], nIF1m2[9];
    long nIF1[45], nM2[45], nstep2[45];
    long *pnArry;
    long val;

    int commandFrac, commandInt, commandOffset, commandM2, commandN2;
    
    nHalfChannel =  0; /* To suppress compiler warning */
    
    nPLL1RefFreq   = 2 * pTnrImplData->xtalFreq;      /*PLL1 reference frequency (Hz)*/
    nRF = nRF / (nPLL1RefFreq / 1000000);

    /*All frequencies will be normalized to fs1.
        After initTune, all frequencies are normalized.
        Integer math can be used if normalized frequencies
        are multiplied by 2^20 to get approx 100Hz
        resolution, and "round", "floor", etc. functions
        are modified properly. */

    /*Limits */
    nPLL1RefFreqKHz = nPLL1RefFreq / 1000;
    nMarginInt    = IntegersDivision(1000000000, nPLL1RefFreqKHz);     /* (fLO1 mod fs1) > 2MHz */
    nMarginHalf   = IntegersDivision(500000000, nPLL1RefFreqKHz);      /* ([fLO1+fs1/2] mod fs1) > 1MHz */
    nMarginQtr    = IntegersDivision(350000000, nPLL1RefFreqKHz);      /* (fLO1 absMod fs1/4) > 0.35MHz */
    nMarginLO1LO2 = IntegersDivision(600000000, nPLL1RefFreqKHz);      /* (fLO1 mod fs2) > 0.5MHz; */

    nPLL1RefFreqMHz = nPLL1RefFreq / 1000000;
    nbwIF1       = IntegersDivision(pTnrImplData->m_nSAWBW, nPLL1RefFreqMHz);        /* EPCOS B1618 */
    nSAWmeasured = IntegersDivision(pTnrImplData->m_nSAWFX, nPLL1RefFreqMHz);        /* should come from SAW sweep algorithm */
    nSAWlo       = nSAWmeasured - (nbwIF1 / 2);
    nSAWhi       = nSAWmeasured + (nbwIF1 / 2);
    ndesiredIF1  = nSAWmeasured;
    if ( 36125000 == pTnrImplData->ifFreq )
    {   /* EU */
        nHalfChannel = 4000000 / nPLL1RefFreqMHz;
    }
    else if ( (43750000 == pTnrImplData->ifFreq) || (44000000 == pTnrImplData->ifFreq) || (47250000 == pTnrImplData->ifFreq) )
    {
        nHalfChannel = 3000000 / nPLL1RefFreqMHz;
    }
    nIF2        = pTnrImplData->ifFreq / nPLL1RefFreqMHz;
    nSAWlo       = nSAWlo + nHalfChannel;        /* allow room for signal */
    nSAWhi       = nSAWhi - nHalfChannel;        /* allow room for signal */
    nHalfChannel = nHalfChannel + (500000 / nPLL1RefFreqMHz);  /* margin for spurs away from passband */

    /* PLL2 reference divider modulus vector
        No odd Ms to avoid subharmonic of XTAL 3/5/02
        Highest to lowest reference freq */

    pnArry = nIF10; /* To suppress compiler warning */
    
    nhalfM2[0] = 10;
    nhalfM2[1] = 9;
    nhalfM2[2] = 8;
    nhalfM2[3] = 7;
    nhalfM2[4] = 6;
    nhalfM2[5] = 11;
    nhalfM2[6] = 12;
    nhalfM2[7] = 13;
    nhalfM2[8] = 14;
    for (i=0; i<9; i++)
    {
        nM2Tmp[i]    = 2 * nhalfM2[i];
        nstep2Tmp[i] = 1000000/nhalfM2[i];
        val          = IntegersDivision((ndesiredIF1-nIF2) * nhalfM2[i], 1000000) * 1000000;

        nIF10[i] = val / nhalfM2[i] + nIF2;
        nIF1p1[i] = nIF10[i] + nstep2Tmp[i];
        nIF1m1[i] = nIF10[i] - nstep2Tmp[i];
        nIF1p2[i] = nIF10[i] + (2 * nstep2Tmp[i]);
        nIF1m2[i] = nIF10[i] - (2* nstep2Tmp[i]);
    };
    for (i=0; i<5; i++)
    {
        switch(i)
        {
            case 0:
                pnArry = nIF10;
                break;
            case 1:
                pnArry = nIF1p1;
                break;
            case 2:
                pnArry = nIF1m1;
                break;
            case 3:
                pnArry = nIF1p2;
                break;
            case 4:
                pnArry = nIF1m2;
                break;
        }
        for (j=0; j<9; j++)
        {
            nIF1[(i*9)+j] = pnArry[j];
        }
    }
    for (i=0; i<5; i++)
    {
        for (j=0; j<9; j++)
        {
            nM2[(i*9)+j]     = nM2Tmp[j];
            nstep2[(i*9)+j] = nstep2Tmp[j];
        }
    }
    
    i = -1;
    while( bad )
    {
        i++;
        if ( i > 44 )
        {
            if ( !(relaxSpurs | relaxBW | relaxAll) )
            {
                ATLTRACE( "Relaxing spur requirements" );
                relaxSpurs = 1;
                i          = 0;
            }
            else if ( relaxSpurs && !(relaxBW | relaxAll) )
            {
                ATLTRACE( "Relaxing SAW requirements" );
                relaxBW = 1;
                i       = 0;
            }
            else if ( relaxSpurs && relaxBW && !relaxAll )
            {
                ATLTRACE( "No spur free combination" );
                relaxAll = 1;
                i        = 0;
            }
            else
            {
                ATLTRACE( "Cannot tune inside SAW BW" );
                cannotTune = 1;
                i        = 0;
            }
        }

        freqs( nRF, nIF1[i], &nCoarse, &nFrac, &nOffset );
        unLO1 = nCoarse + nFrac + nOffset;
        fisBad( nRF, unLO1, nIF2, nHalfChannel, nMarginInt, nMarginHalf, nMarginQtr, 
                relaxSpurs, relaxAll, badType );
        nSAWlo1 = nSAWlo - relaxBW * nMarginInt;
        nSAWhi1 = nSAWhi + relaxBW * nMarginInt;
        temp = IsElementZero(badType, 9);

        bad     = temp | (nIF1[i]<nSAWlo1) | (nIF1[i]>nSAWhi1);

        if ( cannotTune )
            bad = 0;
        
    }

    makeCommands( nCoarse, nFrac, nOffset, nIF1[i], nIF2, nM2[i], nstep2[i],
                  &commandFrac, &commandInt, &commandOffset, &commandM2, &commandN2 );

    unLO1 *= (nPLL1RefFreq / 1000000);  /*Normalized it */
    /*BDBG_MSG(("nRF = %ld, unLO1 = %u, commandInt= %d", nRF, unLO1, commandInt));*/
    /*BDBG_MSG(("uncommandFrac = %d, commandOffset= %d",commandFrac, commandOffset));*/
    PLL1_SetFreq( pTnrImplData, unLO1, commandInt, commandFrac, commandOffset );
    /*BDBG_MSG(("commandM2 = %d, commandN2= %d",commandM2, commandN2));*/
    PLL2_SetFreq( pTnrImplData, commandM2, commandN2 );
}

void BTNR_P_3418_initialize(
    BTNR_P_3418_Settings *pTnrImplData  /* Tuner/board specific data */
    )
{
    pTnrImplData->m_nGainControlType = TGC_AGC_CONTINUOUS;
    pTnrImplData->m_bInitTuner       = false;

#if 1
    /* This is for 47uF cap. tuner, but 200ms may for 47uF cap. too, will have experiment */
    pTnrImplData->m_iAgcSettleTime   = 500;   /* milliseconds. */
    pTnrImplData->m_iForceSettleTime = 1000;  /* milliseconds */
#else
    pTnrImplData->m_iAgcSettleTime   = 100;   /* milliseconds. */
    pTnrImplData->m_iForceSettleTime = 100;  /* milliseconds */
#endif

    pTnrImplData->m_nSAWBW           = 17000000;
    pTnrImplData->m_nSAWFX           = 1216000000;

    pTnrImplData->m_nInitForceVGA    = 35;
    pTnrImplData->m_bFreezedAGC      = false;
    pTnrImplData->m_bPowerUp         = false;

    pTnrImplData->m_iAGCCap          = 10;


    CPLL1_initDefault( &pTnrImplData->m_cPLL1 );
    CPLL2_initDefault( &pTnrImplData->m_cPLL2 );
    CDAC_initDefault( &pTnrImplData->m_cDAC );

    pTnrImplData->m_nInputSignalType = 0;

    /* Get chip version and save it away for future use */
    GetChipRev( pTnrImplData, &pTnrImplData->iRevLetter, &pTnrImplData->iRevNumber, &pTnrImplData->iType );
    if( !(pTnrImplData->iRevLetter == BCM3418AX_REV_LETTER && pTnrImplData->iRevNumber == BCM3418A2_REV_NUMBER) )
    {
        pTnrImplData->alwaysUse3418A2Init = false;
        if( pTnrImplData->iRevLetter == BCM3418BX_REV_LETTER )
        {
            if( pTnrImplData->iRevNumber == BCM3418A1_REV_NUMBER )
            {
                /* For A1, force to use A2 register setting as recommended by H/W */
                pTnrImplData->alwaysUse3418A2Init = true;
            }
        }
    }
    else
    {
        pTnrImplData->alwaysUse3418A2Init = true;
    }
    pTnrImplData->isInitialized = true;
}

void BTNR_P_3418_initDigital(
    BTNR_P_3418_Settings *pTnrImplData  /* Tuner/board specific data */
    )
{
    if( pTnrImplData->alwaysUse3418A2Init == true ||
        (pTnrImplData->iRevLetter == BCM3418AX_REV_LETTER && pTnrImplData->iRevNumber == BCM3418A2_REV_NUMBER) )
    {
        if( pTnrImplData->m_nInputSignalType != BCM3418_INPUTSIGNALTYPE_DIGITAL )
        {
            /* A2 chip */
            /* 09-15-04 from Ray Gomez, readonly */
            /* BTNR_P_3418_WriteReg( pTnrImplData, 0x00, 0x16 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x01, 0x26 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x02, 0xda ); */
            BTNR_P_3418_WriteReg( pTnrImplData, 0x03, 0x31 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x04, 0x87 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x05, 0x80 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x06, 0x00 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x07, 0x00 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x08, 0x02 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x09, 0x40 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x0a, 0x9a );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x0b, 0xdd );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x0c, 0x22 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x0d, 0xcf );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x0e, 0xcc );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x0f, 0xb8 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x10, 0xda );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x11, 0x10 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x12, 0x09 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x13, 0x60 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x14, 0x08 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x15, 0x00 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x16, 0x00 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x17, 0x78 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x18, 0x68 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x19, 0x5a );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x1A, 0x35 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x1b, 0x89 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x1c, 0xff );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x1d, 0x0e );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x1e, 0x1d );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x1f, 0x00 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x20, 0x00 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x21, 0x80 );
            /* 09-15-04 from Ray Gomez, readonly */
            /* BTNR_P_3418_WriteReg( pTnrImplData, 0x22, 0xc1 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x23, 0x18 ); */
            BTNR_P_3418_WriteReg( pTnrImplData, 0x24, 0xc7 );
            pTnrImplData->m_nInputSignalType = BCM3418_INPUTSIGNALTYPE_DIGITAL;
            pTnrImplData->m_bPowerUp = true;
        }
    }
    else if( (pTnrImplData->iRevLetter == BCM3418BX_REV_LETTER) )
    {
        if( pTnrImplData->m_nInputSignalType != BCM3418_INPUTSIGNALTYPE_DIGITAL )
        {
            /* A2 chip */
            /* 09-15-04 from Ray Gomez, readonly */
            /* BTNR_P_3418_WriteReg( pTnrImplData, 0x00, 0x16 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x01, 0x26 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x02, 0xda ); */
            BTNR_P_3418_WriteReg( pTnrImplData, 0x03, 0x31 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x04, 0x87 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x05, 0x80 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x06, 0x00 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x07, 0x00 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x08, 0x02 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x09, 0x20 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x0a, 0x96 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x0b, 0xE0 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x0c, 0x22 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x0d, 0x4F );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x0e, 0xCD );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x0f, 0xB8 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x10, 0xDA );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x11, 0x10 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x12, 0x09 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x13, 0x60 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x14, 0x08 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x15, 0x00 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x16, 0x00 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x17, 0x78 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x18, 0x68 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x19, 0x5A );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x1A, 0x35 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x1b, 0x08 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x1c, 0xFE );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x1d, 0x0E );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x1e, 0x28 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x1f, 0x00 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x20, 0x00 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x21, 0x80 );
            /* 09-15-04 from Ray Gomez, readonly */
            /* BTNR_P_3418_WriteReg( pTnrImplData, 0x22, 0xc1 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x23, 0x18 ); */
            BTNR_P_3418_WriteReg( pTnrImplData, 0x24, 0xc7 );
            pTnrImplData->m_nInputSignalType = BCM3418_INPUTSIGNALTYPE_DIGITAL;
            pTnrImplData->m_bPowerUp = true;
        }
    }
    else
    {
        /* We keeping this code around for backward compatibility with A0 and A1 tuner chips */
        if( pTnrImplData->m_nInputSignalType != BCM3418_INPUTSIGNALTYPE_DIGITAL )
        {
            if( BCM3418AX_REV_LETTER == pTnrImplData->iRevLetter )
            {   /* For Axx chip */
                switch( pTnrImplData->iRevNumber )
                {
                    case BCM3418A0_REV_NUMBER:
                        BTNR_P_3418_WriteReg( pTnrImplData, 0x03, 0x32 );
                        break;
                    case BCM3418A1_REV_NUMBER:
                        BTNR_P_3418_WriteReg( pTnrImplData, 0x03, 0x31 );
                        break;
                    default:
                        break;
                }
            }
            BTNR_P_3418_WriteReg( pTnrImplData, 0x04, 0x87 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x05, 0x80 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x06, 0x00 );
            
            if( BCM3418AX_REV_LETTER == pTnrImplData->iRevLetter )
            {   /* For Axx chip */
                switch( pTnrImplData->iRevNumber )
                {
                    case BCM3418A0_REV_NUMBER:
                        BTNR_P_3418_WriteReg( pTnrImplData, 0x07, 0x00 );
                        break;
                    case BCM3418A1_REV_NUMBER:
                        BTNR_P_3418_WriteReg( pTnrImplData, 0x07, 0x40 );
                        break;
                    default:
                        break;
                }
            }
            BTNR_P_3418_WriteReg( pTnrImplData, 0x08, 0x02 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x09, 0x40 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x0A, 0x9A );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x0B, 0xDD );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x0C, 0x22 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x0E, 0xCC );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x0F, 0xB8 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x10, 0xDA );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x11, 0x10 );

            if( BCM3418AX_REV_LETTER == pTnrImplData->iRevLetter )
            {   /* For Axx chip */
                switch( pTnrImplData->iRevNumber )
                {
                    case BCM3418A0_REV_NUMBER:
                        BTNR_P_3418_WriteReg( pTnrImplData, 0x12, 0x89 );
                        break;
                    case BCM3418A1_REV_NUMBER:
                        BTNR_P_3418_WriteReg( pTnrImplData, 0x12, 0xC9 );
                        break;
                    default:
                        break;
                }
            }
            BTNR_P_3418_WriteReg( pTnrImplData, 0x13, 0x60 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x14, 0x08 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x15, 0x00 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x16, 0x00 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x17, 0x48 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x18, 0x0F );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x19, 0x58 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x1A, 0x07 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x1B, 0x8C );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x1C, 0xFF );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x1D, 0x0E );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x1E, 0x2B );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x1F, 0x00 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x20, 0x00 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x21, 0x00 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x24, 0xC7 );
            pTnrImplData->m_nInputSignalType = BCM3418_INPUTSIGNALTYPE_DIGITAL;
        }
    }
}

void BTNR_P_3418_initAnalog(
    BTNR_P_3418_Settings *pTnrImplData  /* Tuner/board specific data */
    )
{
    if( pTnrImplData->alwaysUse3418A2Init == true ||
        (pTnrImplData->iRevLetter == BCM3418AX_REV_LETTER && pTnrImplData->iRevNumber == BCM3418A2_REV_NUMBER) )
    {
        if( pTnrImplData->m_nInputSignalType != BCM3418_INPUTSIGNALTYPE_ANALOG )
        {
            /* Same register setting is used for Digital and Analog */
            BTNR_P_3418_initDigital( pTnrImplData );
            pTnrImplData->m_nInputSignalType = BCM3418_INPUTSIGNALTYPE_ANALOG;
        }
    }
    else if( pTnrImplData->iRevLetter == BCM3418BX_REV_LETTER )
    {
        if( pTnrImplData->m_nInputSignalType != BCM3418_INPUTSIGNALTYPE_ANALOG )
        {
            /* Same register setting is used for Digital and Analog */
            BTNR_P_3418_initDigital( pTnrImplData );
            pTnrImplData->m_nInputSignalType = BCM3418_INPUTSIGNALTYPE_ANALOG;
        }
    }
    else
    {
        if( pTnrImplData->m_nInputSignalType != BCM3418_INPUTSIGNALTYPE_ANALOG )
        {
            if( BCM3418AX_REV_LETTER == pTnrImplData->iRevLetter )
            {   /* For Axx chip */
                switch( pTnrImplData->iRevNumber )
                {
                    case BCM3418A0_REV_NUMBER:
                        BTNR_P_3418_WriteReg( pTnrImplData, 0x03, 0x31 );
                        break;
                    case BCM3418A1_REV_NUMBER:
                        BTNR_P_3418_WriteReg( pTnrImplData, 0x03, 0x30 );
                        break;
                    default:
                        break;
                }
            }
            BTNR_P_3418_WriteReg( pTnrImplData, 0x04, 0x87 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x05, 0x80 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x06, 0x00 );

            if( BCM3418AX_REV_LETTER == pTnrImplData->iRevLetter )
            {   /* For Axx chip */
                switch( pTnrImplData->iRevNumber )
                {
                    case BCM3418A0_REV_NUMBER:
                        BTNR_P_3418_WriteReg( pTnrImplData, 0x07, 0x00 );
                        break;
                    case BCM3418A1_REV_NUMBER:
                        BTNR_P_3418_WriteReg( pTnrImplData, 0x07, 0x40 );
                        break;
                    default:
                        break;
                }
            }
            BTNR_P_3418_WriteReg( pTnrImplData, 0x08, 0x02 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x09, 0x40 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x0A, 0x9A );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x0B, 0xDD );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x0C, 0x22 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x0E, 0xCC );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x0F, 0xB8 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x10, 0xDA );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x11, 0x10 );

            if( BCM3418AX_REV_LETTER == pTnrImplData->iRevLetter )
            {   /* For Axx chip */
                switch( pTnrImplData->iRevNumber )
                {
                    case BCM3418A0_REV_NUMBER:
                        BTNR_P_3418_WriteReg( pTnrImplData, 0x12, 0x89 );
                        break;
                    case BCM3418A1_REV_NUMBER:
                        BTNR_P_3418_WriteReg( pTnrImplData, 0x12, 0x09 );
                        break;
                    default:
                        break;
                }
            }
            BTNR_P_3418_WriteReg( pTnrImplData, 0x13, 0x60 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x14, 0x08 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x15, 0x00 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x16, 0x00 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x17, 0x48 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x18, 0x0F );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x19, 0x58 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x1A, 0x07 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x1B, 0x8C );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x1C, 0xFF );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x1D, 0x0E );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x1E, 0x2B );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x1F, 0x00 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x20, 0x00 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x21, 0x00 );
            BTNR_P_3418_WriteReg( pTnrImplData, 0x24, 0xC7 );
            pTnrImplData->m_nInputSignalType = BCM3418_INPUTSIGNALTYPE_ANALOG;
        }
    }
}

/****************************************************************************
** Function name : Tune
** Description   : Smart tune the whole chip. Returns PLL2's M divider chosen.
*****************************************************************************/
void BTNR_P_3418_tune( BTNR_P_3418_Settings *pTnrImplData, long nRF )
{

    /*Just in case the crystal frequency is different from the InitTuner. */
    XTAL_SetFreq( pTnrImplData, pTnrImplData->xtalFreq );
    NewTune( pTnrImplData, nRF );

    if( false == pTnrImplData->m_bFreezedAGC )
    {
        switch( pTnrImplData->m_nGainControlType )
        {
            case TGC_AGC_CONTINUOUS:
            case TGC_AGC_HOLD_WHEN_TUNE:
                VGA1_AGC( pTnrImplData, pTnrImplData->m_nGainControlType );  /* Close AGC loop. */
                break;
            case TGC_MANUAL_GAIN:
                DAC_ForceVGA( pTnrImplData, pTnrImplData->m_nInitForceVGA );
                break;
        }
    }
    return;
}

void BTNR_P_3418_PowerDown( BTNR_P_3418_Settings *pTnrImplData )
{
    BTNR_P_3418_WriteField8( pTnrImplData, "PD_PLL1",           (uint8_t)1 );
    BTNR_P_3418_WriteField8( pTnrImplData, "SD_powerDown",      (uint8_t)1 );
    BTNR_P_3418_WriteField8( pTnrImplData, "PD_PLL2",           (uint8_t)1 );
    BTNR_P_3418_WriteField8( pTnrImplData, "MIX1_PWRCTRL[2]",   (uint8_t)0 );
    BTNR_P_3418_WriteField8( pTnrImplData, "MIX1_PWRCTRL[3]",   (uint8_t)0 );
    BTNR_P_3418_WriteField8( pTnrImplData, "MIX1_PWRCTRL[4]",   (uint8_t)0 );
    BTNR_P_3418_WriteField8( pTnrImplData, "MIX1_PWRCTRL[0]",   (uint8_t)0 );
    BTNR_P_3418_WriteField8( pTnrImplData, "MIX1_PWRCTRL[1]",   (uint8_t)0 );
    BTNR_P_3418_WriteField8( pTnrImplData, "PD_MIX2",           (uint8_t)1 );
    BTNR_P_3418_WriteField8( pTnrImplData, "PD_POLY",           (uint8_t)1 );
    BTNR_P_3418_WriteField8( pTnrImplData, "PD_VGA_1220_load",  (uint8_t)1 );
    BTNR_P_3418_WriteField8( pTnrImplData, "PD_FGA",            (uint8_t)1 );
    BTNR_P_3418_WriteField8( pTnrImplData, "PD_SAW",            (uint8_t)1 );
    BTNR_P_3418_WriteField8( pTnrImplData, "PD_VGAOP",          (uint8_t)1 );
    BTNR_P_3418_WriteField8( pTnrImplData, "PD_IF1_BUF",        (uint8_t)1 );
    BTNR_P_3418_WriteField8( pTnrImplData, "PD_IF1_VGA",        (uint8_t)1 );
    BTNR_P_3418_WriteField8( pTnrImplData, "PD_DET0",           (uint8_t)1 );
    BTNR_P_3418_WriteField8( pTnrImplData, "PD_DAC",            (uint8_t)1 );
    pTnrImplData->m_bPowerUp = false;
}

/* Not Used */
void BTNR_P_3418_GetTemperature( BTNR_P_3418_Settings *pTnrImplData, long *pnTemp)
{
    DAC_Sel( pTnrImplData, eDAC_PL, eDAC_PTAT );
    *pnTemp = DAC_SADC( pTnrImplData, 0, false );
}

void BTNR_P_3418_Freeze( BTNR_P_3418_Settings *pTnrImplData )
{
    int iDet = 0;
    
    pTnrImplData->m_bFreezedAGC = true;
    iDet = DAC_ReadPwrDet( pTnrImplData );  /* Take reading of power detector. */
    pTnrImplData->m_cDAC.m_bForceVga = false;
    DAC_ForceVGA( pTnrImplData, iDet );
}

void BTNR_P_3418_UnFreeze( BTNR_P_3418_Settings *pTnrImplData )
{

    pTnrImplData->m_bFreezedAGC = false;
    switch( pTnrImplData->m_nGainControlType )
    {
        case TGC_AGC_CONTINUOUS:
        case TGC_AGC_HOLD_WHEN_TUNE:
            VGA1_AGC( pTnrImplData, pTnrImplData->m_nGainControlType );  /* Close AGC loop. */
            break;
        case TGC_MANUAL_GAIN:
            DAC_ForceVGA( pTnrImplData, pTnrImplData->m_nInitForceVGA );
            break;
    }
}


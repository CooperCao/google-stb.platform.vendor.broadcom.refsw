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

#include "bstd.h"
#include "btnr.h"
#include "btnr_priv.h"
#include "btnr_3418.h"
#include "btnr_3418_smarttune.h"
#include "btnr_3418_regio.h"
#include "breg_i2c.h"

typedef struct _RegMap3418
{
    char szField[32];
    uint8_t bytOffset;
    uint8_t bytMask;
    uint8_t bytBitShift;
    uint8_t bytReserved;
    bool     not_shadowable;
}REGMAP3418, *PREGMAP3418;

#define BTNR_3418_REG_NUMBER 0x25
/*
According to our Tunter engineer Ray Gomez, in most cases which we the register value won't be changed by hardware,
we should can use shadow approach to write field values to them without read-in frist and then write. The field PRGVCO
could be changed by hardware directly according to him, with means, when we write PRGMV, we need to read&write instead
of using shadow value.
*/
static const REGMAP3418 g_RegMap3418[] = 
{/*  Name,                  Offset, Mask, BitShift, reserved */
    {"unlock",                0x00, 0x01, 0x07, 0x00, 0},
    {"VCOx",                  0x00, 0x0F, 0x03, 0x00, 0},
    {"LOCKED",                0x00, 0x01, 0x02, 0x00, 0},
    {"LOCKCaps",              0x00, 0x01, 0x01, 0x00, 0},
    {"preLOCKCaps",           0x00, 0x01, 0x00, 0x00, 0},

    {"Cap",                   0x01, 0x3F, 0x02, 0x00, 0},
    {"LockAmps",              0x01, 0x01, 0x01, 0x00, 0},
    {"Amp[6]",                0x01, 0x01, 0x00, 0x00, 0},

    {"COMP_OUT",              0x02, 0x01, 0x07, 0x00, 0},
    {"PLL2_LOCK",             0x02, 0x01, 0x06, 0x00, 0},
    {"Amp[5:0]",              0x02, 0x3F, 0x00, 0x00, 0},

    {"PD_PLL1",               0x03, 0x01, 0x07, 0x00, 0},
    {"WATCHDOG",              0x03, 0x01, 0x06, 0x00, 0},
    {"PORZ",                  0x03, 0x01, 0x05, 0x00, 0},
    {"RESET_LFBPLL1",         0x03, 0x01, 0x04, 0x00, 0},
    {"IIC_RESET",             0x03, 0x01, 0x03, 0x00, 0},
    {"GAIN_PLL1",             0x03, 0x07, 0x00, 0x00, 0},

    {"DIV_BIAS_PLL1",         0x04, 0x07, 0x05, 0x00, 0},
    {"LockDelayPLL1",         0x04, 0x03, 0x03, 0x00, 0},
    {"PLL1_set_LFb",          0x04, 0x01, 0x02, 0x00, 0},
    {"SD_resetModb",          0x04, 0x01, 0x01, 0x00, 0},
    {"SD_loadMod",            0x04, 0x01, 0x00, 0x00, 0},

    {"SELAMP_LVL[7:0]",       0x05, 0xFF, 0x00, 0x00, 0},

    {"MANAMPLPF",             0x06, 0x01, 0x07, 0x00, 0},
    {"MANAMPLPFSEL",          0x06, 0x01, 0x06, 0x00, 0},
    {"SELAMP_LVL[13:8]",      0x06, 0x3F, 0x00, 0x00, 0},

    {"PRGMC",                 0x07, 0x01, 0x07, 0x00, 0},
    {"PLL1_Ios[2]",           0x07, 0x01, 0x06, 0x00, 0},
    {"PRGCAP",                0x07, 0x3F, 0x00, 0x07, 0},

    {"PRGMA",                 0x08, 0x01, 0x07, 0x00, 0},
    {"PRGAMP",                0x08, 0x7F, 0x00, 0x08, 0},

    {"PRGMV",                 0x09, 0x01, 0x07, 0x00, 1},      /* because PRGVCO might be changed by HW, so cannot be shadowable, other fields in the register won't be changed at all */
    {"PRGVCO",                0x09, 0x03, 0x05, 0x00, 0},
    {"MANCLK",                0x09, 0x01, 0x04, 0x00, 0},
    {"MANCLKSEL",             0x09, 0x01, 0x03, 0x00, 0},
    {"MANMOREC",              0x09, 0x01, 0x02, 0x00, 0},
    {"MANMORECISEL",          0x09, 0x01, 0x01, 0x00, 0},
    {"MANMOREI",              0x09, 0x01, 0x00, 0x00, 0},

    {"startByPLL",            0x0A, 0x01, 0x07, 0x00, 0},
    {"NPLL1",                 0x0A, 0x7F, 0x00, 0x00, 0},

    {"FracPLL1[7:0]",         0x0B, 0xFF, 0x00, 0x00, 0},

    {"FracOffsetPLL1",        0x0C, 0x03, 0x06, 0x00, 0},
    {"FracPLL1[12:8]",        0x0C, 0x1F, 0x01, 0x00, 0},
    {"SD_powerDown",          0x0C, 0x01, 0x00, 0x00, 0},

    {"PLL2_READ_CAPS",        0x0D, 0x3F, 0x00, 0x00, 0},

    {"PLL2_R_WB",             0x0E, 0x01, 0x07, 0x00, 0},
    {"PLL2_RESETB",           0x0E, 0x01, 0x06, 0x00, 0},
    {"PD_PLL2",               0x0E, 0x01, 0x05, 0x00, 0},
    {"rpl_pwrdn",             0x0E, 0x01, 0x04, 0x00, 0},
    {"VCO2_CUR",              0x0E, 0x0F, 0x00, 0x00, 0},

    {"PLL2_PRESETB",          0x0F, 0x01, 0x07, 0x00, 0},
    {"rpl_tm_vc80n",          0x0F, 0x01, 0x06, 0x00, 0},
    {"VCO2_TUNE",             0x0F, 0x3F, 0x00, 0x00, 0},

    {"NPLL2[7:0]",            0x10, 0xFF, 0x00, 0x00, 0},

    {"NPLL2[8]",              0x11, 0x01, 0x07, 0x00, 0},
    {"PLL2_RLF",              0x11, 0x01, 0x06, 0x00, 0},
    {"LockDelayPLL2",         0x11, 0x01, 0x05, 0x00, 0},
    {"MPLL2",                 0x11, 0x1F, 0x00, 0x00, 0},

    {"PLL1_Ios[1:0]",         0x12, 0x03, 0x06, 0x00, 0},
    {"MIX1_PWRCTRL[2]",       0x12, 0x01, 0x05, 0x00, 0},
    {"MIX1_PWRCTRL[3]",       0x12, 0x01, 0x04, 0x00, 0},
    {"MIX1_PWRCTRL[4]",       0x12, 0x01, 0x03, 0x00, 0},
    {"GAIN_PLL2",             0x12, 0x07, 0x00, 0x00, 0},

    {"PLL1_DIVBUFLVL",        0x13, 0x0F, 0x04, 0x00, 0},
    {"PLL1BUFAmpLvl[3:0]",    0x13, 0x0F, 0x00, 0x00, 0},

    {"PLL1BUFAmpLvl[8:4]",    0x14, 0x1F, 0x03, 0x00, 0},
    {"PLL1BUF_AMP[4]",        0x14, 0x01, 0x02, 0x00, 0},
    {"PLL1BUF_CAP",           0x14, 0x03, 0x00, 0x00, 0},

    {"MIX1_PWRCTRL[0]",       0x15, 0x01, 0x07, 0x00, 0},
    {"MIX1_PWRCTRL[1]",       0x15, 0x01, 0x06, 0x00, 0},
    {"MIX1_CAP[5:3]",         0x15, 0x07, 0x03, 0x00, 0},
    {"MIX1_CAP[2:0]",         0x15, 0x07, 0x00, 0x00, 0},

    {"PD_MIX2",               0x16, 0x01, 0x07, 0x00, 0},
    {"MIX2_CTRL",             0x16, 0x7F, 0x00, 0x00, 0},

    {"PD_POLY",               0x17, 0x01, 0x07, 0x00, 0},
    {"PD_VGA_1220_load",      0x17, 0x01, 0x06, 0x00, 0},
    {"PS_POLY",               0x17, 0x03, 0x04, 0x00, 0},
    {"rpl_OTA_pwrup_IIC",     0x17, 0x01, 0x03, 0x00, 0},
    {"PD_FGA",                0x17, 0x01, 0x02, 0x00, 0},
    {"PS_FGA",                0x17, 0x03, 0x00, 0x00, 0},

    {"PD_SAW",                0x18, 0x01, 0x07, 0x00, 0},
    {"PS_SAW",                0x18, 0x03, 0x05, 0x00, 0},
    {"PD_VGAOP",              0x18, 0x01, 0x04, 0x00, 0},
    {"IF1_R_CTRL",            0x18, 0x0F, 0x00, 0x00, 0},

    {"PD_IF1_BUF",            0x19, 0x01, 0x07, 0x00, 0},
    {"IF1_BIAS_CURR_CTRL",    0x19, 0x01, 0x06, 0x00, 0},
    {"IF1_M_CTRL",            0x19, 0x0F, 0x02, 0x00, 0},
    {"IF1_TUNE_BUF",          0x19, 0x03, 0x00, 0x00, 0},

    {"PD_IF1_VGA",            0x1A, 0x01, 0x07, 0x00, 0},
    {"IF1_CURR_VGA",          0x1A, 0x07, 0x04, 0x00, 0},
    {"IF1_TUNE_VGA",          0x1A, 0x0F, 0x00, 0x00, 0},

    {"PD_DET0",               0x1B, 0x01, 0x07, 0x00, 0},
    {"APWR0",                 0x1B, 0x01, 0x06, 0x00, 0},
    {"IENV0",                 0x1B, 0x01, 0x05, 0x00, 0},	/*IF1_fastAcq*/
    {"THRES_DET0",            0x1B, 0x1F, 0x00, 0x00, 0},

    {"DAC_latchData",         0x1C, 0x01, 0x07, 0x00, 0},
    {"DAC_refSel",            0x1C, 0x01, 0x06, 0x00, 0},
    {"DACiSel",               0x1C, 0x01, 0x05, 0x00, 0},
    {"DACselPLb",             0x1C, 0x01, 0x04, 0x00, 0},
    {"DACselPHb",             0x1C, 0x01, 0x03, 0x00, 0},
    {"DACselNMb",             0x1C, 0x01, 0x02, 0x00, 0},
    {"DACselPMb",             0x1C, 0x01, 0x01, 0x00, 0},
    {"IF1_vga_1220_sw",       0x1C, 0x01, 0x00, 0x00, 0},	/*IF1_AGC_pullUp*/

    {"PD_DAC",                0x1D, 0x01, 0x07, 0x00, 0},
    {"IF1_PDET_CONST",        0x1D, 0x01, 0x06, 0x00, 0},
    {"rpl_tm_vc40n",          0x1D, 0x01, 0x05, 0x00, 0},
    {"rpl_tm_vc20n",          0x1D, 0x01, 0x04, 0x00, 0},
    {"IF1_GAIN_FORCE",        0x1D, 0x01, 0x03, 0x00, 0},
    {"VGAPWRCNT",             0x1D, 0x07, 0x00, 0x00, 0},

    {"PD_XTAL",               0x1E, 0x01, 0x07, 0x00, 0},
    {"rpl_OTA_pwrup_sel_IIC", 0x1E, 0x01, 0x06, 0x00, 0},
    {"DAC",                   0x1E, 0x3F, 0x00, 0x00, 0},

    {"XTAL_FREQ",             0x1F, 0x0F, 0x04, 0x00, 0},
    {"XTALBIAS",              0x1F, 0x0F, 0x00, 0x00, 0},

    {"PD_BGR",                0x20, 0x01, 0x07, 0x00, 0},
    {"SD_clkinv",             0x20, 0x01, 0x06, 0x00, 0},
    {"XTAL_AMP_OSCB",         0x20, 0x03, 0x04, 0x00, 0},
    {"CTAT_PH_TRIM",          0x20, 0x07, 0x01, 0x00, 0},
    {"CTAT_PL_TRIM[3]",       0x20, 0x01, 0x00, 0x00, 0},

    {"CTAT_PL_TRIM[2:1]",     0x21, 0x03, 0x06, 0x00, 0},
    {"CTAT_EXT_TRIM",         0x21, 0x07, 0x03, 0x00, 0},
    {"CTAT_LX_TRIM",          0x21, 0x07, 0x00, 0x00, 0},

    {"CHIP_TYPE",             0x22, 0x07, 0x05, 0x00, 0},
    {"REVISION_LETTER",       0x22, 0x07, 0x02, 0x00, 0},
    {"REVISION_NUMBER",       0x22, 0x03, 0x00, 0x00, 0},

    {"DIE_VERSION",           0x23, 0x07, 0x05, 0x00, 0},
    {"bufSMdone",             0x23, 0x01, 0x04, 0x00, 0},
    {"PLL1BUF_AMP[3:0]",      0x23, 0x0F, 0x00, 0x00, 0},

    {"bufSMdivRatio",         0x24, 0x03, 0x06, 0x00, 0},
    {"bufSMstart",            0x24, 0x01, 0x05, 0x00, 0},
    {"bufSMforce",            0x24, 0x01, 0x04, 0x00, 0},
    {"bufSMforceValue",       0x24, 0x0F, 0x00, 0x00, 0}
};
#define MX_TBL_ENTRY_SZ			(sizeof(g_RegMap3418) / sizeof(g_RegMap3418[0]))

#define	TOUPPER(c)				( (c >= 'a' && c <= 'z')? (c - 0x20): c )

static bool CompareNoCaseForEqual(
	char *s,
	char *p
	)
{
	bool retVal;


	retVal = true;
	for( ; *s != 0x00 && *p != 0x00; s++, p++ )
	{
		if( TOUPPER(*s) != TOUPPER(*p) )
		{
			retVal = false;
			break;
		}
	}
	if( *s != 0x00 || *p != 0x00 )
	{
		retVal = false;
	}
	return( retVal );
}



/****************************************************************************
** Function name : FindRegField
** Description   : Finds the matching register field and gets the register map 
**                 data structure.
*****************************************************************************/
static PREGMAP3418 FindRegField( char *pszField )
{
    bool        bFound;
    PREGMAP3418 pRegMap;
	unsigned int nIndex;


    bFound  = false;
	pRegMap = NULL;
    for( nIndex=0; nIndex < MX_TBL_ENTRY_SZ; nIndex++ )
    {
        pRegMap = (PREGMAP3418) &g_RegMap3418[nIndex];
        if ( CompareNoCaseForEqual(pRegMap->szField, pszField) == true )
        {
            bFound = true;
            break;
        }
    }
    if ( bFound )
        return pRegMap;
    else
        return NULL;
}


/****************************************************************************
** Function name : WriteReg
** Description   : Writes a new register value. 
*****************************************************************************/
void BTNR_P_3418_WriteReg( BTNR_P_3418_Settings *pTnrImplData, unsigned bytOffset, uint8_t bytData )
{
	unsigned short chipAddr;
	unsigned char subAddr;
	unsigned char data;
	BERR_Code retCode;


	chipAddr = pTnrImplData->i2cAddr;		/* Must be 7 bit address */
	subAddr  = (unsigned char) bytOffset;
	data     = (unsigned char) bytData;

	retCode = BREG_I2C_Write( pTnrImplData->hI2CReg, chipAddr, subAddr, &data, 1 );
	BDBG_ASSERT( retCode == BERR_SUCCESS );
	BDBG_ASSERT (bytOffset < BTNR_3418_REG_NUMBER);
	pTnrImplData->register_shadow[bytOffset] = (unsigned char)bytData;
}

/****************************************************************************
** Function name : ReadReg
** Description   : Reads the register current value. 
*****************************************************************************/
void BTNR_P_3418_ReadReg( BTNR_P_3418_Settings *pTnrImplData, unsigned bytOffset, uint8_t *pbytData )
{
	unsigned short chipAddr;
	unsigned char subAddr;
	unsigned char data;
	BERR_Code retCode;


	chipAddr = pTnrImplData->i2cAddr;		/* Must be 7 bit address */
	subAddr  = (unsigned char) bytOffset;

 	retCode = BREG_I2C_Read( pTnrImplData->hI2CReg, chipAddr, subAddr, &data, 1 );
	/* PR16449: FIXME: This is a hack to mask the read errors.  If they're serious,
	 * they should be seen on a second attempt anyways */
	if (retCode != BERR_SUCCESS)
 		retCode = BREG_I2C_Read( pTnrImplData->hI2CReg, chipAddr, subAddr, &data, 1 );
	BDBG_ASSERT( retCode == BERR_SUCCESS );

	*pbytData = data;
}

/****************************************************************************
** Function name : WriteField
** Description   : Writes a new register field value. Here are the steps
**                 perform by this function:
**                 1. Find the associate data structure for this field
**                 2. Read the register current value and mask out the field
**                 3. Sets the new field value and write back to the register
*****************************************************************************/
void BTNR_P_3418_WriteField8( BTNR_P_3418_Settings *pTnrImplData, char *pszField, uint8_t bytData )
{
    PREGMAP3418 pRegMap;
    uint8_t bytMask, bytReg;

    pRegMap = FindRegField( pszField );
	BDBG_ASSERT( pRegMap != NULL );

    bytMask = pRegMap->bytMask << pRegMap->bytBitShift;

    if(pRegMap->not_shadowable == 1 )
        BTNR_P_3418_ReadReg( pTnrImplData, pRegMap->bytOffset, &bytReg );
    else
        bytReg = pTnrImplData->register_shadow[pRegMap->bytOffset];

    bytReg &= ~bytMask;
    bytReg |= (bytData & pRegMap->bytMask) << pRegMap->bytBitShift;
    BTNR_P_3418_WriteReg( pTnrImplData, pRegMap->bytOffset, bytReg );
}

void BTNR_P_3418_WriteField16( BTNR_P_3418_Settings *pTnrImplData, char *pszField, uint16_t sData )
{
    PREGMAP3418 pRegMap;
    uint8_t bytMask, bytReg, bytData;


    if ( CompareNoCaseForEqual( pszField, "FracPLL1" ) == true )
    {
        pRegMap = FindRegField( "FracPLL1[7:0]" );
		BDBG_ASSERT( pRegMap != NULL );

        bytReg = (uint8_t)sData;
        BTNR_P_3418_WriteReg( pTnrImplData, pRegMap->bytOffset, bytReg );
        pRegMap = FindRegField( "FracPLL1[12:8]" );
		BDBG_ASSERT( pRegMap != NULL );
        bytMask = pRegMap->bytMask << pRegMap->bytBitShift;
        if(pRegMap->not_shadowable == 1 )
            BTNR_P_3418_ReadReg( pTnrImplData, pRegMap->bytOffset, &bytReg );
        else
            bytReg = pTnrImplData->register_shadow[pRegMap->bytOffset];
        bytData = (uint8_t)(sData>>8);
        bytReg &= ~bytMask;
        bytReg |= (bytData & pRegMap->bytMask) << pRegMap->bytBitShift;
        BTNR_P_3418_WriteReg( pTnrImplData, pRegMap->bytOffset, bytReg );
    }
    else if ( CompareNoCaseForEqual( pszField, "NPLL2" ) == true )
    {
        pRegMap = FindRegField( "NPLL2[7:0]" );
		BDBG_ASSERT( pRegMap != NULL );
        bytReg = (uint8_t)sData;
        BTNR_P_3418_WriteReg( pTnrImplData, pRegMap->bytOffset, bytReg );
        pRegMap = FindRegField( "NPLL2[8]" );
		BDBG_ASSERT( pRegMap != NULL );
        bytMask = pRegMap->bytMask << pRegMap->bytBitShift;
        if(pRegMap->not_shadowable == 1 )
            BTNR_P_3418_ReadReg( pTnrImplData, pRegMap->bytOffset, &bytReg );
        else
            bytReg = pTnrImplData->register_shadow[pRegMap->bytOffset];
        bytData = (uint8_t)(sData>>8);
        bytReg &= ~bytMask;
        bytReg |= (bytData & pRegMap->bytMask) << pRegMap->bytBitShift;
        BTNR_P_3418_WriteReg( pTnrImplData, pRegMap->bytOffset, bytReg );
    }
	else
	{
		BDBG_ASSERT( false );
	}
}

/****************************************************************************
** Function name : ReadField
** Description   : Reads the register field current value. Here are the steps
**                 perform by this function:
**                 1. Find the associate data structure for this field
**                 2. Read the register current value and extract the field
**                    value
*****************************************************************************/
void BTNR_P_3418_ReadField8( BTNR_P_3418_Settings *pTnrImplData, char *pszField, uint8_t *pbytData )
{
    PREGMAP3418 pRegMap;
    uint8_t    bytMask, bytReg;


    *pbytData = 0;
    pRegMap   = FindRegField( pszField );
	BDBG_ASSERT( pRegMap != NULL );

    bytMask = pRegMap->bytMask << pRegMap->bytBitShift;
    BTNR_P_3418_ReadReg( pTnrImplData, pRegMap->bytOffset, &bytReg );
    *pbytData = (bytReg & bytMask) >> pRegMap->bytBitShift;
}

/****************************************************************************
** Function name : ReadField
** Description   : Reads the register field current value. Here are the steps
**                 perform by this function:
**                 1. Find the associate data structure for this field
**                 2. Read the register current value and extract the field
**                    value
*****************************************************************************/
void BTNR_P_3418_ReadField16( BTNR_P_3418_Settings *pTnrImplData, char *pszField, uint16_t *psData )
{
    PREGMAP3418 pRegMap;
    uint8_t    bytMask, bytReg;


    *psData = 0;
    if ( CompareNoCaseForEqual( pszField, "FracPLL1" ) == true )
    {
        pRegMap = FindRegField( "FracPLL1[7:0]" );
		BDBG_ASSERT( pRegMap != NULL );
	    bytMask = pRegMap->bytMask << pRegMap->bytBitShift;
	    BTNR_P_3418_ReadReg( pTnrImplData, pRegMap->bytOffset, &bytReg );
    	*psData = (bytReg & bytMask) >> pRegMap->bytBitShift;

        pRegMap = FindRegField( "FracPLL1[12:8]" );
		BDBG_ASSERT( pRegMap != NULL );
	    bytMask = pRegMap->bytMask << pRegMap->bytBitShift;
	    BTNR_P_3418_ReadReg( pTnrImplData, pRegMap->bytOffset, &bytReg );
    	*psData |= ((bytReg & bytMask) >> pRegMap->bytBitShift) << 8;
	}
    else if ( CompareNoCaseForEqual( pszField, "NPLL2" ) == true )
	{
        pRegMap = FindRegField( "NPLL2[7:0]" );
		BDBG_ASSERT( pRegMap != NULL );
	    bytMask = pRegMap->bytMask << pRegMap->bytBitShift;
	    BTNR_P_3418_ReadReg( pTnrImplData, pRegMap->bytOffset, &bytReg );
    	*psData = (bytReg & bytMask) >> pRegMap->bytBitShift;

        pRegMap = FindRegField( "NPLL2[8]" );
		BDBG_ASSERT( pRegMap != NULL );
	    bytMask = pRegMap->bytMask << pRegMap->bytBitShift;
	    BTNR_P_3418_ReadReg( pTnrImplData, pRegMap->bytOffset, &bytReg );
    	*psData |= ((bytReg & bytMask) >> pRegMap->bytBitShift) << 8;
	}
	else
	{
		BDBG_ASSERT( false );
	}
}


int BTNR_P_3418_GetRegNumber()
{
	return BTNR_3418_REG_NUMBER;
}


/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
 * Porting interface code for the data transport core. 
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bstd.h"
#include "bxpt_priv.h"
#include "bxpt_sam.h"
#include "bchp_xpt_msg.h"

#if( BDBG_DEBUG_BUILD == 1 )
BDBG_MODULE( xpt_sam );
#endif

static void SetSam40BitAddr(
	BXPT_Handle hXpt,
	const BXPT_SAM_40BitAddr *Addr,
	uint32_t RegHiAddr,
	uint32_t RegLoAddr
	);

static void GetSam40BitAddr(
	BXPT_Handle hXpt,
	BXPT_SAM_40BitAddr *Addr,
	uint32_t RegHiAddr,
	uint32_t RegLoAddr
	);


BERR_Code BXPT_SAM_GetM16Addresss(
	BXPT_Handle hXpt,
	unsigned int MessageBufferNum,
	BXPT_SAM_M16Addresses *Addresses
	)
{
	BERR_Code ExitCode = BERR_SUCCESS;
																    
	BDBG_ASSERT( hXpt );
	BDBG_ASSERT( Addresses );

	/* Sanity check on the arguments. */
	if( MessageBufferNum > BXPT_NUM_MESG_BUFFERS )
	{
		/* Bad PID channel number. Complain. */
		BDBG_ERR(( "Message buffer %lu is out of range for SAM filtering!", ( unsigned long ) MessageBufferNum ));
		ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
	}
	else
	{
		uint32_t Reg;

		/* Set the PID channels enable bit. */
		uint32_t RegAddr = BCHP_XPT_MSG_MCAST_16_ADDR_i_ARRAY_BASE + ( 16 * MessageBufferNum );
	
		Reg = BREG_Read32( hXpt->hRegister, RegAddr );
		Addresses->Address0[ 0 ] = ( uint8_t )(( Reg >> 8 ) & 0xFF );
		Addresses->Address0[ 1 ] = ( uint8_t )(( Reg ) & 0xFF );
	
		Reg = BREG_Read32( hXpt->hRegister, RegAddr + 4 );
		Addresses->Address1[ 0 ] = ( uint8_t )(( Reg >> 8 ) & 0xFF );
		Addresses->Address1[ 1 ] = ( uint8_t )(( Reg ) & 0xFF );
	
		Reg = BREG_Read32( hXpt->hRegister, RegAddr + 8 );
		Addresses->Address2[ 0 ] = ( uint8_t )(( Reg >> 8 ) & 0xFF );
		Addresses->Address2[ 1 ] = ( uint8_t )(( Reg ) & 0xFF );
	
		Reg = BREG_Read32( hXpt->hRegister, RegAddr + 12 );
		Addresses->Address3[ 0 ] = ( uint8_t )(( Reg >> 8 ) & 0xFF );
		Addresses->Address3[ 1 ] = ( uint8_t )(( Reg ) & 0xFF );
	}

	return( ExitCode );
}
 
 
BERR_Code BXPT_SAM_SetM16Addresss(
	BXPT_Handle hXpt,
	unsigned int MessageBufferNum,
	const BXPT_SAM_M16Addresses *Addresses
	)
{
	BERR_Code ExitCode = BERR_SUCCESS;
																    
	BDBG_ASSERT( hXpt );
	BDBG_ASSERT( Addresses );

	/* Sanity check on the arguments. */
	if( MessageBufferNum > BXPT_NUM_MESG_BUFFERS )
	{
		/* Bad PID channel number. Complain. */
		BDBG_ERR(( "Message buffer %lu is out of range for SAM filtering!", ( unsigned long ) MessageBufferNum ));
		ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
	}
	else
	{
		uint32_t Reg;

		/* Set the PID channels enable bit. */
		uint32_t RegAddr = BCHP_XPT_MSG_MCAST_16_ADDR_i_ARRAY_BASE + ( 16 * MessageBufferNum );

		Reg = ( Addresses->Address0[ 0 ] << 8 | Addresses->Address0[ 1 ] );
		BREG_Write32( hXpt->hRegister, RegAddr, Reg );

		Reg = ( Addresses->Address1[ 0 ] << 8 | Addresses->Address1[ 1 ] );
		BREG_Write32( hXpt->hRegister, RegAddr + 4, Reg );

		Reg = ( Addresses->Address2[ 0 ] << 8 | Addresses->Address2[ 1 ] );
		BREG_Write32( hXpt->hRegister, RegAddr + 8, Reg );

		Reg = ( Addresses->Address3[ 0 ] << 8 | Addresses->Address3[ 1 ] );
		BREG_Write32( hXpt->hRegister, RegAddr + 12, Reg );
	}

	return( ExitCode );
}


BERR_Code BXPT_SAM_GetAddresses(
	BXPT_Handle hXpt,
	BXPT_SAMAddresses *Addresses
	)
{
	uint32_t Reg;

	BERR_Code ExitCode = BERR_SUCCESS;

	BDBG_ASSERT( hXpt );
	BDBG_ASSERT( Addresses );

	/* Physical (or singlecast) address. */
   	Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_MSG_PHY_ADDR_HI );
	Addresses->PhysicalAddr[ 0 ] = BCHP_GET_FIELD_DATA( Reg, XPT_MSG_PHY_ADDR_HI, PHY_ADDR_HI );

   	Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_MSG_PHY_ADDR_LO );
	Addresses->PhysicalAddr[ 1 ] = ( uint8_t )(( Reg >> 24 ) & 0xFF );
	Addresses->PhysicalAddr[ 2 ] = ( uint8_t )(( Reg >> 16 ) & 0xFF );
	Addresses->PhysicalAddr[ 3 ] = ( uint8_t )(( Reg >> 8 ) & 0xFF );
	Addresses->PhysicalAddr[ 4 ] = ( uint8_t )(( Reg ) & 0xFF );

	/* Type 0 network address */
	GetSam40BitAddr( hXpt, &Addresses->Type0NetworkAddr, BCHP_XPT_MSG_NET_ADDR0_HI, BCHP_XPT_MSG_NET_ADDR0_LO );

	/* Type 1 network address */
	GetSam40BitAddr( hXpt, &Addresses->Type1NetworkAddr, BCHP_XPT_MSG_NET_ADDR1_HI, BCHP_XPT_MSG_NET_ADDR1_LO );

	/* Type 0 multicast-40 address */
	GetSam40BitAddr( hXpt, &Addresses->Type0M40Addr, BCHP_XPT_MSG_M40_ADDR0_HI, BCHP_XPT_MSG_M40_ADDR0_LO );

	/* Type 1 multicast-40 address */
	GetSam40BitAddr( hXpt, &Addresses->Type1M40Addr, BCHP_XPT_MSG_M40_ADDR1_HI, BCHP_XPT_MSG_M40_ADDR1_LO );

	/* Multicast 24 address. */
   	Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_MSG_M24_ADDR );
	Addresses->M24Addr[ 0 ] = ( uint8_t )(( Reg >> 16 ) & 0xFF );
	Addresses->M24Addr[ 1 ] = ( uint8_t )(( Reg >> 8 ) & 0xFF );
	Addresses->M24Addr[ 2 ] = ( uint8_t )(( Reg ) & 0xFF );

	/* Table ID rejection upper and lower limits */
   	Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_MSG_ID_REJECT );
	Addresses->IdRejectHiLimit = BCHP_GET_FIELD_DATA( Reg, XPT_MSG_ID_REJECT, PF_ID_REJECT_HI );
	Addresses->IdRejectLowLimit = BCHP_GET_FIELD_DATA( Reg, XPT_MSG_ID_REJECT, PF_ID_REJECT_LO );

	return( ExitCode );
}


BERR_Code BXPT_SAM_SetAddresses(
	BXPT_Handle hXpt,
	const BXPT_SAMAddresses *Addresses
	)
{
	uint32_t Reg;

	BERR_Code ExitCode = BERR_SUCCESS;

	BDBG_ASSERT( hXpt );
	BDBG_ASSERT( Addresses );

	/* Physical (or singlecast) address. */
   	Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_MSG_PHY_ADDR_HI );
	Reg &= ~( BCHP_MASK( XPT_MSG_PHY_ADDR_HI, PHY_ADDR_HI ) );
	Reg |= ( BCHP_FIELD_DATA( XPT_MSG_PHY_ADDR_HI, PHY_ADDR_HI, Addresses->PhysicalAddr[ 0 ] ) );
	BREG_Write32( hXpt->hRegister, BCHP_XPT_MSG_PHY_ADDR_HI, Reg );

	Reg = ( Addresses->PhysicalAddr[ 1 ] << 24 ) | ( Addresses->PhysicalAddr[ 2 ] << 16 ) 
		| ( Addresses->PhysicalAddr[ 3 ] << 8 ) | ( Addresses->PhysicalAddr[ 4 ] );
	BREG_Write32( hXpt->hRegister, BCHP_XPT_MSG_PHY_ADDR_LO, Reg );

	/* Type 0 network address */
	SetSam40BitAddr( hXpt, &Addresses->Type0NetworkAddr, BCHP_XPT_MSG_NET_ADDR0_HI, BCHP_XPT_MSG_NET_ADDR0_LO );

	/* Type 1 network address */
	SetSam40BitAddr( hXpt, &Addresses->Type1NetworkAddr, BCHP_XPT_MSG_NET_ADDR1_HI, BCHP_XPT_MSG_NET_ADDR1_LO );

	/* Type 0 multicast 40 address */
	SetSam40BitAddr( hXpt, &Addresses->Type0M40Addr, BCHP_XPT_MSG_M40_ADDR0_HI, BCHP_XPT_MSG_M40_ADDR0_LO );

	/* Type 1 multicast 40 address */
	SetSam40BitAddr( hXpt, &Addresses->Type1M40Addr, BCHP_XPT_MSG_M40_ADDR1_HI, BCHP_XPT_MSG_M40_ADDR1_LO );

	/* Multicast 24 address. */
   	Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_MSG_M24_ADDR );
	Reg &= ~( BCHP_MASK( XPT_MSG_M24_ADDR, M24_ADDR ) );
	Reg |= ( Addresses->M24Addr[ 0 ] << 16 ) | ( Addresses->M24Addr[ 1 ] << 8 ) | Addresses->M24Addr[ 2 ];
	BREG_Write32( hXpt->hRegister, BCHP_XPT_MSG_M24_ADDR, Reg );

	/* Table ID rejection upper and lower limits */
   	Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_MSG_ID_REJECT );
	Reg &= ~( 
		BCHP_MASK( XPT_MSG_ID_REJECT, PF_ID_REJECT_HI ) |
		BCHP_MASK( XPT_MSG_ID_REJECT, PF_ID_REJECT_LO ) 
	);
	Reg |= ( 
		BCHP_FIELD_DATA( XPT_MSG_ID_REJECT, PF_ID_REJECT_HI, Addresses->IdRejectHiLimit ) |
		BCHP_FIELD_DATA( XPT_MSG_ID_REJECT, PF_ID_REJECT_LO, Addresses->IdRejectLowLimit ) 
	);
	BREG_Write32( hXpt->hRegister, BCHP_XPT_MSG_ID_REJECT, Reg );

	return( ExitCode );
}


void GetSam40BitAddr(
	BXPT_Handle hXpt,
	BXPT_SAM_40BitAddr *Addr,
	uint32_t RegHiAddr,
	uint32_t RegLoAddr
	)
{
	uint32_t Reg;

	/*
	** Some SAM filters use the same register layout, so we save code space by using a common 
	** routine to store the new settings. Those filters are Type0/1 Network address, and
	** Type0/1 Multicast 40 addresses.
	*/
   	Reg = BREG_Read32( hXpt->hRegister, RegHiAddr );
	Addr->Address[ 0 ] = BCHP_GET_FIELD_DATA( Reg, XPT_MSG_NET_ADDR0_HI, NET_ADDR_HI );
	Addr->WildCardByteMode = BCHP_GET_FIELD_DATA( Reg, XPT_MSG_NET_ADDR0_HI, NET_WILDCARD_MODE ) ? true : false ;
	Addr->WildCardEnable = BCHP_GET_FIELD_DATA( Reg, XPT_MSG_NET_ADDR0_HI, NET_WILDCARD_ENABLE ) ? true : false ;
	Addr->AddressWildcard = BCHP_GET_FIELD_DATA( Reg, XPT_MSG_NET_ADDR0_HI, NET_ADDR_WILDCARD );

   	Reg = BREG_Read32( hXpt->hRegister, RegLoAddr );
	Addr->Address[ 1 ] = ( uint8_t )(( Reg >> 24 ) & 0xFF );
	Addr->Address[ 2 ]= ( uint8_t )(( Reg >> 16 ) & 0xFF );
	Addr->Address[ 3 ]= ( uint8_t )(( Reg >> 8 ) & 0xFF );
	Addr->Address[ 4 ]= ( uint8_t )(( Reg ) & 0xFF );
}


void SetSam40BitAddr(
	BXPT_Handle hXpt,
	const BXPT_SAM_40BitAddr *Addr,
	uint32_t RegHiAddr,
	uint32_t RegLoAddr
	)
{
	uint32_t Reg;

	/*
	** Some SAM filters use the same register layout, so we save code space by using a common 
	** routine to store the new settings. Those filters are Type0/1 Network address, and
	** Type0/1 Multicast 40 addresses.
	*/
   	Reg = BREG_Read32( hXpt->hRegister, RegHiAddr );
	Reg &= ~(
		BCHP_MASK( XPT_MSG_NET_ADDR0_HI, NET_ADDR_HI ) | 
		BCHP_MASK( XPT_MSG_NET_ADDR0_HI, NET_WILDCARD_MODE ) | 
		BCHP_MASK( XPT_MSG_NET_ADDR0_HI, NET_WILDCARD_ENABLE ) | 
		BCHP_MASK( XPT_MSG_NET_ADDR0_HI, NET_ADDR_WILDCARD ) 
	);
	Reg |= (
		BCHP_FIELD_DATA( XPT_MSG_NET_ADDR0_HI, NET_ADDR_HI, Addr->Address[ 0 ] ) |
		BCHP_FIELD_DATA( XPT_MSG_NET_ADDR0_HI, NET_WILDCARD_MODE, Addr->WildCardByteMode == true ? 1 : 0 ) |
		BCHP_FIELD_DATA( XPT_MSG_NET_ADDR0_HI, NET_WILDCARD_ENABLE, Addr->WildCardEnable == true ? 1 : 0 ) |
		BCHP_FIELD_DATA( XPT_MSG_NET_ADDR0_HI, NET_ADDR_WILDCARD, Addr->AddressWildcard ) 
	);
	BREG_Write32( hXpt->hRegister, RegHiAddr, Reg );

	Reg = ( Addr->Address[ 1 ] << 24 ) | ( Addr->Address[ 2 ] << 16 )
		| (	Addr->Address[ 3 ] << 8 ) | Addr->Address[ 4 ];
	BREG_Write32( hXpt->hRegister, RegLoAddr, Reg );
}


BERR_Code BXPT_SAM_DoFiltering(
	BXPT_Handle hXpt,
	unsigned int MessageBufferNum,
	BXPT_SAM_Enables *Enables
	)
{
	uint32_t RegAddr, Reg;

	uint32_t SpecialType = 0;
	BERR_Code ExitCode = BERR_SUCCESS;

	BDBG_ASSERT( hXpt );
	BDBG_ASSERT( Enables );

	/* Sanity check on the arguments. */
	if( MessageBufferNum > BXPT_NUM_MESG_BUFFERS )
	{
		/* Bad PID channel number. Complain. */
		BDBG_ERR(( "Message buffer %lu is out of range for SAM filtering!", ( unsigned long ) MessageBufferNum ));
		ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
	}
	else
	{
    	/* Set the SAM general enable if ANY of the above SAM filters is enabled. */
    	if( Enables->PhysicalEn == true || Enables->NetworkEn == true || Enables->M40En == true ||
    	Enables->M24En == true || Enables->M16En == true || Enables->IdRejectEn == true	)
    		SpecialType = 1;  

#if BXPT_P_SAM_CTRL1
    	RegAddr = BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_ARRAY_BASE + ( MessageBufferNum * PID_CTRL1_TABLE_STEP );
    	Reg = BREG_Read32( hXpt->hRegister, RegAddr );
    	Reg &= ~(
    		BCHP_MASK( XPT_MSG_BUF_CTRL1_TABLE_i, PHY_EN ) |
    		BCHP_MASK( XPT_MSG_BUF_CTRL1_TABLE_i, NET_EN ) |
    		BCHP_MASK( XPT_MSG_BUF_CTRL1_TABLE_i, M40_EN ) |
    		BCHP_MASK( XPT_MSG_BUF_CTRL1_TABLE_i, M24_EN ) |
    		BCHP_MASK( XPT_MSG_BUF_CTRL1_TABLE_i, M16_EN ) |
    		BCHP_MASK( XPT_MSG_BUF_CTRL1_TABLE_i, ID_REJECT_EN ) 
    	);
    	Reg |= (
    		BCHP_FIELD_DATA( XPT_MSG_BUF_CTRL1_TABLE_i, PHY_EN, Enables->PhysicalEn == true ? 1 : 0 ) |
    		BCHP_FIELD_DATA( XPT_MSG_BUF_CTRL1_TABLE_i, NET_EN, Enables->NetworkEn == true ? 1 : 0 ) |
    		BCHP_FIELD_DATA( XPT_MSG_BUF_CTRL1_TABLE_i, M40_EN, Enables->M40En == true ? 1 : 0 ) |
    		BCHP_FIELD_DATA( XPT_MSG_BUF_CTRL1_TABLE_i, M24_EN, Enables->M24En == true ? 1 : 0 ) |
    		BCHP_FIELD_DATA( XPT_MSG_BUF_CTRL1_TABLE_i, M16_EN, Enables->M16En == true ? 1 : 0 ) |
    		BCHP_FIELD_DATA( XPT_MSG_BUF_CTRL1_TABLE_i, ID_REJECT_EN, Enables->IdRejectEn == true ? 1 : 0 ) 
    	);
    	BREG_Write32( hXpt->hRegister, RegAddr, Reg );

    	RegAddr = BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_ARRAY_BASE + ( MessageBufferNum * PID_CTRL2_TABLE_STEP );
    	Reg = BREG_Read32( hXpt->hRegister, RegAddr );
    	Reg &= ~(
    		BCHP_MASK( XPT_MSG_BUF_CTRL2_TABLE_i, SPECIAL_NOT ) |
    		BCHP_MASK( XPT_MSG_BUF_CTRL2_TABLE_i, SPECIAL_TYPE ) 
    	);
    	Reg |= (
    		BCHP_FIELD_DATA( XPT_MSG_BUF_CTRL2_TABLE_i, SPECIAL_NOT, Enables->SamNotEn == true ? 1 : 0 ) |
    		BCHP_FIELD_DATA( XPT_MSG_BUF_CTRL2_TABLE_i, SPECIAL_TYPE, SpecialType ) 
    	);
    	BREG_Write32( hXpt->hRegister, RegAddr, Reg );
#else
    	RegAddr = BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_ARRAY_BASE + ( MessageBufferNum * PID_CTRL2_TABLE_STEP );
    	Reg = BREG_Read32( hXpt->hRegister, RegAddr );
    	Reg &= ~(
    		BCHP_MASK( XPT_MSG_BUF_CTRL2_TABLE_i, PHY_EN ) |
    		BCHP_MASK( XPT_MSG_BUF_CTRL2_TABLE_i, NET_EN ) |
    		BCHP_MASK( XPT_MSG_BUF_CTRL2_TABLE_i, M40_EN ) |
    		BCHP_MASK( XPT_MSG_BUF_CTRL2_TABLE_i, M24_EN ) |
    		BCHP_MASK( XPT_MSG_BUF_CTRL2_TABLE_i, M16_EN ) |
    		BCHP_MASK( XPT_MSG_BUF_CTRL2_TABLE_i, ID_REJECT_EN ) |
    		BCHP_MASK( XPT_MSG_BUF_CTRL2_TABLE_i, SPECIAL_NOT ) |
    		BCHP_MASK( XPT_MSG_BUF_CTRL2_TABLE_i, SPECIAL_TYPE ) 
    	);
    	Reg |= (
    		BCHP_FIELD_DATA( XPT_MSG_BUF_CTRL2_TABLE_i, PHY_EN, Enables->PhysicalEn == true ? 1 : 0 ) |
    		BCHP_FIELD_DATA( XPT_MSG_BUF_CTRL2_TABLE_i, NET_EN, Enables->NetworkEn == true ? 1 : 0 ) |
    		BCHP_FIELD_DATA( XPT_MSG_BUF_CTRL2_TABLE_i, M40_EN, Enables->M40En == true ? 1 : 0 ) |
    		BCHP_FIELD_DATA( XPT_MSG_BUF_CTRL2_TABLE_i, M24_EN, Enables->M24En == true ? 1 : 0 ) |
    		BCHP_FIELD_DATA( XPT_MSG_BUF_CTRL2_TABLE_i, M16_EN, Enables->M16En == true ? 1 : 0 ) |
    		BCHP_FIELD_DATA( XPT_MSG_BUF_CTRL2_TABLE_i, ID_REJECT_EN, Enables->IdRejectEn == true ? 1 : 0 ) |
    		BCHP_FIELD_DATA( XPT_MSG_BUF_CTRL2_TABLE_i, SPECIAL_NOT, Enables->SamNotEn == true ? 1 : 0 ) |
    		BCHP_FIELD_DATA( XPT_MSG_BUF_CTRL2_TABLE_i, SPECIAL_TYPE, SpecialType ) 
    	);
    	BREG_Write32( hXpt->hRegister, RegAddr, Reg );
#endif
    }
	return( ExitCode );
}



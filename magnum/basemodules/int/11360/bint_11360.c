/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
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
 **************************************************************************/
#include "bstd.h"
#include "bint_11360.h"

/* Include interrupt definitions from RDB */
#include "bchp_hif_cpu_intr1.h"

#include "bchp_v3d_dbg.h"
#include "bchp_v3d_ctl.h"


BDBG_MODULE(interruptinterface_11360);

#define BINT_P_V3D_CTL_INTCTL_STATUS   0x00
#define BINT_P_V3D_CTL_INTCTL_ENABLE   0x04
#define BINT_P_V3D_CTL_INTCTL_CASES \
    case BCHP_V3D_CTL_INTCTL:

#define BINT_P_V3D_DBG_DBQITC_STATUS   0x00
#define BINT_P_V3D_DBG_DBQITC_CASES \
    case BCHP_V3D_DBG_DBQITC:

#define BCHP_INT_ID_V3D_INTCTL_INTR              BCHP_INT_ID_CREATE(BCHP_V3D_CTL_INTCTL, 0)
#define BCHP_INT_ID_V3D_DBQITC_INTR              BCHP_INT_ID_CREATE(BCHP_V3D_DBG_DBQITC, 0)

static void BINT_P_11360_ClearInt( BREG_Handle regHandle, uint32_t baseAddr, int shift );
static void BINT_P_11360_SetMask( BREG_Handle regHandle, uint32_t baseAddr, int shift );
static void BINT_P_11360_ClearMask( BREG_Handle regHandle, uint32_t baseAddr, int shift );
static uint32_t BINT_P_11360_ReadStatus( BREG_Handle regHandle, uint32_t baseAddr );

static const BINT_P_IntMap bint_11360[] =
{
	BINT_MAP(1, V3D, "_INTCTL", V3D_CTL_INTCTL, REGULAR, NONE, 0),
	BINT_MAP(1, V3D, "_DBGITC", V3D_DBG_DBQITC, REGULAR, NONE, 0),
    BINT_MAP_LAST()
};

static const BINT_Settings bint_11360Settings =
{
    NULL,
    BINT_P_11360_ClearInt,
    BINT_P_11360_SetMask,
    BINT_P_11360_ClearMask,
    NULL,
    BINT_P_11360_ReadStatus,
    bint_11360,
    "11360"
};

static void BINT_P_11360_ClearInt( BREG_Handle regHandle, uint32_t baseAddr, int shift )
{
    BDBG_MSG(("ClearInt %#x:%d", baseAddr, shift));
    switch( baseAddr )
    {
        BINT_P_V3D_CTL_INTCTL_CASES
        BINT_P_V3D_DBG_DBQITC_CASES
            /* Has to cleared at the source */
            break;
        default:
            /* Other types of interrupts do not support clearing of interrupts (condition must be cleared) */
            break;
    }
}

static void BINT_P_11360_SetMask( BREG_Handle regHandle, uint32_t baseAddr, int shift )
{
    uint32_t intEnable;
    BDBG_MSG(("SetMask %#x:%d", baseAddr, shift));

    switch( baseAddr )
    {
    BINT_P_V3D_CTL_INTCTL_CASES
    BINT_P_V3D_DBG_DBQITC_CASES
        /* Dont support setting the v3d L2 via this interface */
        break;

    default:
        /* Unhandled interrupt base address */
        BDBG_ASSERT( false );
        break;
    }
}

static void BINT_P_11360_ClearMask( BREG_Handle regHandle, uint32_t baseAddr, int shift )
{
    uint32_t intEnable;
    BDBG_MSG(("ClearMask %#x:%d", baseAddr, shift));
    switch( baseAddr )
    {
    BINT_P_V3D_CTL_INTCTL_CASES
    BINT_P_V3D_DBG_DBQITC_CASES
        /* Dont support setting the v3d L2 via this interface */
        break;

    default:
        /* Unhandled interrupt base address */
        BDBG_ASSERT( false );
        break;
    }
}

static uint32_t BINT_P_11360_ReadStatus( BREG_Handle regHandle, uint32_t baseAddr )
{
    BDBG_MSG(("ReadStatus %#x", baseAddr));
    switch( baseAddr )
    {

    BINT_P_V3D_CTL_INTCTL_CASES
        {
            uint32_t flags;
            flags  = BREG_Read32( regHandle, baseAddr + BINT_P_V3D_CTL_INTCTL_STATUS );
            flags &= BREG_Read32( regHandle, baseAddr + BINT_P_V3D_CTL_INTCTL_ENABLE );
            flags &= BREG_Read32( regHandle, baseAddr + BINT_P_V3D_CTL_INTCTL_STATUS );
            return flags;
        }
        break;

    BINT_P_V3D_DBG_DBQITC_CASES
        return BREG_Read32( regHandle, baseAddr + BINT_P_V3D_DBG_DBQITC_STATUS );

    default:
        /* Unhandled interrupt base address */
        BDBG_ASSERT( false );
        return 0;
    }
}

const BINT_Settings *BINT_11360_GetSettings( void )
{
    return &bint_11360Settings;
}

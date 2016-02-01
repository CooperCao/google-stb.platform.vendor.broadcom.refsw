/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
#include "burt.h"
#include "burt_priv.h"
#include "bchp_common.h"
#include "bchp_uarta.h"
#include "bchp_uartb.h" 

#ifdef BCHP_UARTC_REG_START
    #include "bchp_uartc.h"
    #define MAX_URT_CHANNELS 3
#else
    #define MAX_URT_CHANNELS 2
#endif

#include "bchp_irq0.h"
#include "bchp_int_id_irq0.h"

BDBG_MODULE(burt);

#define	DEV_MAGIC_ID			((BERR_URT_ID<<16) | 0xFACE)

#define	BURT_CHK_RETCODE( rc, func )		\
do {										\
	if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
	{										\
		goto done;							\
	}										\
} while(0)

#define UART_CLOCK_FREQ					27000000  /* 27MHz input clock */
#define BITRATE_TO_BAUDRATE( bit )		(((UART_CLOCK_FREQ/8/(bit) + 1)/2) - 1)
#define BAUDRATE_TO_BITRATE( baud )		(UART_CLOCK_FREQ / (16 * (baud + 1)))

#define UART_TX_FIFO_SIZE			32

/*******************************************************************************
*
*	Private Module Handles
*
*******************************************************************************/

typedef struct BURT_P_Handle
{
	uint32_t		magicId;					/* Used to check if structure is corrupt */
	BCHP_Handle 	hChip;
	BREG_Handle		hRegister;
	BINT_Handle 	hInterrupt;
	unsigned int 	maxChnNo;
	BURT_ChannelHandle hUrtChn[MAX_URT_CHANNELS];
} BURT_P_Handle;

typedef struct BURT_P_ChannelHandle
{
	uint32_t 			magicId;					/* Used to check if structure is corrupt */
	BURT_Handle 		hUrt;
	uint32_t	 		chnNo;
	uint32_t 			coreOffset;
	BKNI_EventHandle	hChnTxEvent;
	BKNI_EventHandle	hChnRxEvent;
	BINT_CallbackHandle hChnCallback;
	bool				intMode;					/* If true, rx/txIntMode is not used. */
	bool				rxIntMode;					/* used only if intMode is false */
	bool				txIntMode;					/* used only if intMode is false */
    uint8_t             savedLsr;                   /* OR error bits into here so they are not lost */
	BINT_CallbackFunc 	intCallbackFunc ;
} BURT_P_ChannelHandle;

/* chip has some 16550 style UARTS */
static void BURT_P_EnableFifo(BURT_ChannelHandle hChn);
static uint32_t BURT_P_ReadLsr(BURT_ChannelHandle hChn, uint32_t *pErrLsr);
static uint32_t BURT_P_ReadLsr_Isr(BURT_ChannelHandle hChn, uint32_t *pErrLsr);

/*******************************************************************************
*
*	Default Module Settings
*
*******************************************************************************/
static const BURT_Settings defUrtSettings = NULL;

static const BURT_ChannelSettings defUrt0ChnSettings =
{
	true,
	true,
	115200,
	BURT_DataBits_eDataBits8,
	BURT_Parity_eNone,
	true,
	false,			/* rxIntMode is set to false for backward compatibility. */
	false,			/* txIntMode is set to false for backward compatibility. */
	BURT_StopBits_eOneBit
};

static const BURT_ChannelSettings defUrt1ChnSettings =
{
	true,
	true,
	115200,
	BURT_DataBits_eDataBits8,
	BURT_Parity_eNone,
	true,
	false,			/* rxIntMode is set to false for backward compatibility. */
	false,			/* txIntMode is set to false for backward compatibility. */
	BURT_StopBits_eOneBit
};

#if (MAX_URT_CHANNELS >= 3)
static const BURT_ChannelSettings defUrt2ChnSettings =
{
	true,
	true,
	115200,
	BURT_DataBits_eDataBits8,
	BURT_Parity_eNone,
	true,
	false,			/* rxIntMode is set to false for backward compatibility. */
	false,			/* txIntMode is set to false for backward compatibility. */
	BURT_StopBits_eOneBit
};
#endif

#if (MAX_URT_CHANNELS >= 4)
static const BURT_ChannelSettings defUrt3ChnSettings =
{
	true,
	true,
	115200,
	BURT_DataBits_eDataBits8,
	BURT_Parity_eNone,
	true,
	false,			/* rxIntMode is set to false for backward compatibility. */
	false,			/* txIntMode is set to false for backward compatibility. */
	BURT_StopBits_eOneBit
};
#endif

static const BINT_Id IntId[] =
{
	BCHP_INT_ID_ua_irqen,
#if (MAX_URT_CHANNELS >=2)
	BCHP_INT_ID_ub_irqen,
#endif
#if (MAX_URT_CHANNELS >=3)
	BCHP_INT_ID_uc_irqen,
#endif
#if (MAX_URT_CHANNELS >=4)
	BCHP_INT_ID_ud_irqen,
#endif
};
/*******************************************************************************
*
*	Public Module Functions
*
*******************************************************************************/
void BURT_RegisterCallback(
	BURT_ChannelHandle	hChn,			/* Device channel handle */
	BINT_CallbackFunc 	callbackFunc	/* callback function to register */
	)
{
	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hChn->intCallbackFunc = callbackFunc;
}

BERR_Code BURT_Open(
	BURT_Handle *pUrt,					/* [output] Returns handle */
	BCHP_Handle hChip,					/* Chip handle */
	BREG_Handle hRegister,				/* Register handle */
	BINT_Handle hInterrupt,				/* Interrupt handle */
	const BURT_Settings *pDefSettings	/* Default settings */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
 	BURT_Handle hDev;
	unsigned int chnIdx;


	/* Sanity check on the handles we've been given. */
	BDBG_ASSERT( hChip );
	BDBG_ASSERT( hRegister );
	BDBG_ASSERT( hInterrupt );
	BSTD_UNUSED( pDefSettings );

	/* Alloc memory from the system heap */
	hDev = (BURT_Handle) BKNI_Malloc( sizeof( BURT_P_Handle ) );
	if( hDev == NULL )
	{
		*pUrt = NULL;
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("BURT_Open: BKNI_malloc() failed\n"));
		goto done;
	}

	hDev->magicId	= DEV_MAGIC_ID;
	hDev->hChip		= hChip;
	hDev->hRegister = hRegister;
	hDev->hInterrupt = hInterrupt;
	hDev->maxChnNo	= MAX_URT_CHANNELS;
	for( chnIdx = 0; chnIdx < hDev->maxChnNo; chnIdx++ )
	{
		hDev->hUrtChn[chnIdx] = NULL;
	}

	*pUrt = hDev;

done:
	return( retCode );
}

BERR_Code BURT_Close(
	BURT_Handle hDev					/* Device handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;


	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
	BKNI_Free( (void *) hDev );

	return( retCode );
}

BERR_Code BURT_GetDefaultSettings(
	BURT_Settings *pDefSettings,		/* [output] Returns default setting */
	BCHP_Handle hChip					/* Chip handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;

	BSTD_UNUSED(hChip);


	*pDefSettings = defUrtSettings;

	return( retCode );
}

BERR_Code BURT_GetTotalChannels(
	BURT_Handle hDev,					/* Device handle */
	unsigned int *totalChannels			/* [output] Returns total number downstream channels supported */
	)
{
	BERR_Code retCode = BERR_SUCCESS;


	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	*totalChannels = hDev->maxChnNo;

	return( retCode );
}

BERR_Code BURT_GetChannelDefaultSettings(
	BURT_Handle hDev,					/* Device handle */
	unsigned int channelNo,				/* Channel number to default setting for */
    BURT_ChannelSettings *pChnDefSettings /* [output] Returns channel default setting */
    )
{
	BERR_Code retCode = BERR_SUCCESS;

#if !BDBG_DEBUG_BUILD
	BSTD_UNUSED(hDev);
#endif

	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	switch (channelNo)
	{
		case 0:
			*pChnDefSettings = defUrt0ChnSettings;
			break;

		case 1:
			*pChnDefSettings = defUrt1ChnSettings;
			break;

#if (MAX_URT_CHANNELS >= 3)
		case 2:
			*pChnDefSettings = defUrt2ChnSettings;
			break;
#endif

#if (MAX_URT_CHANNELS >= 4)
		case 3:
			*pChnDefSettings = defUrt3ChnSettings;
			break;
#endif

		default:
			retCode = BERR_INVALID_PARAMETER;
			break;

	}

	return( retCode );
}

BERR_Code BURT_OpenChannel(
	BURT_Handle hDev,					/* Device handle */
	BURT_ChannelHandle *phChn,			/* [output] Returns channel handle */
	unsigned int channelNo,				/* Channel number to open */
	const BURT_ChannelSettings *pChnDefSettings /* Channel default setting */
	)
{
	BERR_Code 			retCode = BERR_SUCCESS;
 	BURT_ChannelHandle	hChnDev;
	uint32_t			lval;

	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	hChnDev = NULL;

	if( channelNo < hDev->maxChnNo )
	{
		if( hDev->hUrtChn[channelNo] == NULL )
		{
			/* Alloc memory from the system heap */
			hChnDev = (BURT_ChannelHandle) BKNI_Malloc( sizeof( BURT_P_ChannelHandle ) );
			if( hChnDev == NULL )
			{
				*phChn = NULL;
				retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
				BDBG_ERR(("BURT_OpenChannel: BKNI_malloc() failed\n"));
				goto done;
			}

			BURT_CHK_RETCODE( retCode, BKNI_CreateEvent( &(hChnDev->hChnTxEvent) ) );
			BURT_CHK_RETCODE( retCode, BKNI_CreateEvent( &(hChnDev->hChnRxEvent) ) );
			hChnDev->magicId 	= DEV_MAGIC_ID;
			hChnDev->hUrt		= hDev;
			hChnDev->chnNo		= channelNo;
			hDev->hUrtChn[channelNo] = hChnDev;
            switch (channelNo) {
                case 0:
                    hChnDev->coreOffset = 0;
                    break;
                case 1:
                    hChnDev->coreOffset = BCHP_UARTB_RBR - BCHP_UARTA_RBR;
                    break;
#if (MAX_URT_CHANNELS >=3)
                case 2:
                    hChnDev->coreOffset = BCHP_UARTC_RBR - BCHP_UARTA_RBR;
                    break;
#endif
#if (MAX_URT_CHANNELS >=4)
                case 3:
                    hChnDev->coreOffset = BCHP_UARTD_RBR - BCHP_UARTA_RBR;
                    break;
#endif
            }

			BURT_EnableTxInt (hChnDev, false);			/* turn off ints */
			BURT_EnableRxInt (hChnDev, false);			/* turn off ints */
            hChnDev->savedLsr = 0;
			BREG_Write32( hDev->hRegister, (hChnDev->coreOffset + BCHP_UARTA_LCR), 0 );
            BURT_P_EnableFifo (hChnDev);
			BURT_P_SetBaudRate (hChnDev, pChnDefSettings->baud);
			BURT_P_SetDataBits (hChnDev, pChnDefSettings->bits);
			BURT_P_SetParity   (hChnDev, pChnDefSettings->parity);
			BURT_P_SetStopBits  (hChnDev, pChnDefSettings->stopBits);
			BURT_P_EnableTxRx  (hChnDev, pChnDefSettings->txEnable, pChnDefSettings->rxEnable);
			/*
			 * Enable interrupt for this channel
			 * Initialize callback function to NULL. This callback is
			 * used instead of posting event during interrupt handling.
			 */
			hChnDev->intMode = pChnDefSettings->intMode;
			hChnDev->rxIntMode = pChnDefSettings->rxIntMode;
			hChnDev->txIntMode = pChnDefSettings->txIntMode;
			hChnDev->intCallbackFunc = NULL;
			if ((hChnDev->intMode == true) ||
				(hChnDev->rxIntMode == true) ||
				(hChnDev->txIntMode == true))
			{
				/*
				 * Register and enable L2 interrupt.
				 * The same callback is used for tx/rx
				 */
				BURT_CHK_RETCODE( retCode, BINT_CreateCallback(
					&(hChnDev->hChnCallback), hDev->hInterrupt, IntId[channelNo],
					BURT_P_HandleInterrupt_Isr, (void *) hChnDev, 0x00 ) );
				BURT_CHK_RETCODE( retCode, BINT_EnableCallback( hChnDev->hChnCallback ) );

				BKNI_EnterCriticalSection();

				/*
				 * Enable URT interrupt in UPG
				 */
				lval = BREG_Read32(hDev->hRegister, BCHP_IRQ0_IRQEN);
				if (channelNo == 0)
					lval |= BCHP_FIELD_DATA(IRQ0_IRQEN, ua_irqen, 1);
				else if (channelNo == 1)
					lval |= BCHP_FIELD_DATA(IRQ0_IRQEN, ub_irqen, 1);
#if (MAX_URT_CHANNELS >= 3)
				else if (channelNo == 2)
					lval |= BCHP_FIELD_DATA(IRQ0_IRQEN, uc_irqen, 1);
#endif
#if (MAX_URT_CHANNELS >= 4)
				else if (channelNo == 3)
					lval |= BCHP_FIELD_DATA(IRQ0_IRQEN, ud_irqen, 1);
#endif

				BREG_Write32( hDev->hRegister, BCHP_IRQ0_IRQEN, lval );

				/*
				 * Enable RX URT interrupt in URT
				 * DON'T ENABLE TX INT BECAUSE THE FIFO IS EMPTY AND WE'RE GOING TO GET INTERRUPTED RIGHT AWAY.
				 * The caller should enable the interrupt AFTER he writes to the TX FIFO.
				 */
				BURT_EnableTxInt (hChnDev, false);
				if ((hChnDev->intMode == true) ||
					(hChnDev->rxIntMode == true))
				{
					BURT_EnableRxInt (hChnDev, true);
				}
				BKNI_LeaveCriticalSection();
			}
			else
			{
				hChnDev->hChnCallback = NULL;
			}

			*phChn = hChnDev;
		}
		else
		{
			retCode = BURT_ERR_NOTAVAIL_CHN_NO;
		}
	}
	else
	{
		retCode = BERR_INVALID_PARAMETER;
	}

done:
	if( retCode != BERR_SUCCESS )
	{
		if( hChnDev != NULL )
		{
			BKNI_DestroyEvent( hChnDev->hChnTxEvent );
			BKNI_DestroyEvent( hChnDev->hChnRxEvent );
			BKNI_Free( hChnDev );
			hDev->hUrtChn[channelNo] = NULL;
			*phChn = NULL;
		}
	}
	return( retCode );
}

BERR_Code BURT_CloseChannel(
	BURT_ChannelHandle hChn			/* Device channel handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BURT_Handle hDev;
	unsigned int chnNo;


	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hUrt;
	/*
	 * Disable interrupt for this channel
	 */
	BKNI_EnterCriticalSection();
	BURT_EnableTxInt (hChn, false);			/* turn off ints */
	BURT_EnableRxInt (hChn, false);			/* turn off ints */
	BURT_P_EnableTxRx    (hChn, false, false);
	BKNI_LeaveCriticalSection();

	if (hChn->hChnCallback)
	{
		BURT_CHK_RETCODE( retCode, BINT_DisableCallback( hChn->hChnCallback ) );
		BURT_CHK_RETCODE( retCode, BINT_DestroyCallback( hChn->hChnCallback ) );
	}

	BKNI_DestroyEvent( hChn->hChnTxEvent );
	BKNI_DestroyEvent( hChn->hChnRxEvent );
	chnNo = hChn->chnNo;
	BKNI_Free( hChn );
	hDev->hUrtChn[chnNo] = NULL;

done:
	return( retCode );
}

BERR_Code BURT_GetDevice(
	BURT_ChannelHandle hChn,			/* Device channel handle */
	BURT_Handle *phDev					/* [output] Returns Device handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	*phDev = hChn->hUrt;

	return( retCode );
}

BERR_Code BURT_GetTxEventHandle(
	BURT_ChannelHandle hChn,			/* Device channel handle */
	BKNI_EventHandle *phEvent			/* [output] Returns event handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	*phEvent = hChn->hChnTxEvent;

	return( retCode );
}

BERR_Code BURT_GetRxEventHandle(
	BURT_ChannelHandle hChn,			/* Device channel handle */
	BKNI_EventHandle *phEvent			/* [output] Returns event handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	*phEvent = hChn->hChnRxEvent;

	return( retCode );
}

bool BURT_IsRxDataAvailable(
	BURT_ChannelHandle	hChn			/* Device channel handle */
	)
{
	uint32_t 	lval;
	BURT_Handle	hDev;
	bool		dataAvail;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hUrt;

	lval = BURT_P_ReadLsr(hChn, 0);
	dataAvail = (lval & BCHP_UARTA_LSR_DR_MASK) ? true : false;

	return dataAvail;
}

bool BURT_IsRxDataAvailable_Isr(
	BURT_ChannelHandle	hChn			/* Device channel handle */
	)
{
	uint32_t 	lval;
	BURT_Handle	hDev;
	bool		dataAvail;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hUrt;

	lval = BURT_P_ReadLsr_Isr(hChn, 0);
	dataAvail = (lval & BCHP_UARTA_LSR_DR_MASK) ? true : false;

	return dataAvail;
}

uint32_t BURT_GetAvailTxFifoCnt (
	BURT_ChannelHandle	hChn			/* Device channel handle */
	)
{
	uint32_t 	lval, byteCnt;
	BURT_Handle	hDev;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hUrt;

	lval = BURT_P_ReadLsr(hChn, 0);
	byteCnt = (lval & BCHP_UARTA_LSR_THRE_MASK) ? UART_TX_FIFO_SIZE : 0;

	return byteCnt;
}

uint32_t BURT_GetAvailTxFifoCnt_Isr(
	BURT_ChannelHandle	hChn			/* Device channel handle */
	)
{
	uint32_t 	lval, byteCnt;
	BURT_Handle	hDev;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hUrt;

	lval = BURT_P_ReadLsr_Isr(hChn, 0);
	byteCnt = (lval & BCHP_UARTA_LSR_THRE_MASK) ? UART_TX_FIFO_SIZE : 0;

	return byteCnt;
}

void BURT_EnableTxInt(
	BURT_ChannelHandle 	hChn,			/* Device channel handle */
	bool				enableTxInt		/* enable flag for transmitter interrupt */
	)
{
	uint32_t	lval;
	BURT_Handle	hDev;

	hDev = hChn->hUrt;

	lval = BREG_Read32(hDev->hRegister, hChn->coreOffset + BCHP_UARTA_IER);
	if (enableTxInt)
	{
		/* Enable TX int */
		lval |= BCHP_UARTA_IER_ETBEI_MASK;
	}
	else
	{
		/* Disable TX int */
		lval &= ~BCHP_UARTA_IER_ETBEI_MASK;
	}
	BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_UARTA_IER, lval);
}

void BURT_EnableRxInt(
	BURT_ChannelHandle 	hChn,			/* Device channel handle */
	bool				enableRxInt		/* enable flag for receiver interrupt */
	)
{
	uint32_t	lval;
	BURT_Handle	hDev;

	hDev = hChn->hUrt;


	lval = BREG_Read32(hDev->hRegister, hChn->coreOffset + BCHP_UARTA_IER);
	if (enableRxInt)
	{
		/* Enable RX int */
		lval |= BCHP_UARTA_IER_ERBFI_MASK;
	}
	else
	{
		/* Disable RX int */
		lval &= ~BCHP_UARTA_IER_ERBFI_MASK;
	}
	BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_UARTA_IER, lval);
}

BERR_Code BURT_Read_Isr (
	BURT_ChannelHandle	hChn,			/* Device channel handle */
	uint8_t				*data,			/* pointer to memory to store data */
	uint32_t			numBytes,		/* number of bytes to read */
	uint32_t			*bytesRead,		/* [output] number of actual bytes read */
	BURT_RxError		*rxError		/* [output] receive error code */
	)
{
	BERR_Code 		retCode = BERR_SUCCESS;
	uint32_t 		loopCnt = 0;
	BURT_Handle		hDev;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hUrt;

	while (numBytes && (BURT_IsRxDataAvailable_Isr(hChn)==true))
	{
		*data = (uint8_t)(BREG_Read32_isr(hDev->hRegister, hChn->coreOffset + BCHP_UARTA_RBR));
		data++;
		loopCnt++;
		numBytes--;
	}
	*bytesRead = loopCnt;				/* actual bytes read */

	*rxError = BURT_GetRxError_Isr(hChn);
	if (*rxError != BURT_RxError_eNoError)
	{
		retCode = BURT_ERR_RX_ERROR;
		goto done;
	}

done:
	return( retCode );
}

BERR_Code BURT_Read (
	BURT_ChannelHandle	hChn,			/* Device channel handle */
	uint8_t				*data,			/* pointer to memory to store data */
	uint32_t			numBytes,		/* number of bytes to read */
	uint32_t			*bytesRead,		/* [output] number of actual bytes read */
	BURT_RxError		*rxError		/* [output] receive error code */
	)
{
	BERR_Code 		retCode = BERR_SUCCESS;
	uint32_t 		loopCnt = 0;
	BURT_Handle		hDev;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hUrt;

	while (numBytes && (BURT_IsRxDataAvailable(hChn)==true))
	{
		*data = (uint8_t)(BREG_Read32(hDev->hRegister, hChn->coreOffset + BCHP_UARTA_RBR));
		data++;
		loopCnt++;
		numBytes--;
	}
	*bytesRead = loopCnt;				/* actual bytes read */

	*rxError = BURT_GetRxError (hChn);
	if (*rxError != BURT_RxError_eNoError)
	{
		retCode = BURT_ERR_RX_ERROR;
		goto done;
	}

	/*
	 * Re-enable RX interrupt
	 */
	if ((hChn->intMode) || (hChn->rxIntMode))
		BURT_EnableRxInt (hChn, true);

done:
	return( retCode );
}

BERR_Code BURT_Write (
	BURT_ChannelHandle	hChn,			/* Device channel handle */
	uint8_t			*data,				/* pointers to data to send */
	uint32_t		numBytes			/* number of bytes to write */
	)
{
	BERR_Code 		retCode = BERR_SUCCESS;
	uint32_t 		fifoAvail, lval;
	uint8_t			bval;
	BURT_Handle		hDev;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hUrt;
	if (!numBytes)
		goto done;

	fifoAvail = BURT_GetAvailTxFifoCnt (hChn);
	if (numBytes > fifoAvail)
	{
		retCode = BURT_ERR_TX_FIFO_NOT_AVAIL;
		goto done;
	}

	while (numBytes--)
	{
		bval = *data++;
		lval = (uint32_t)bval;
		BREG_Write32 (hDev->hRegister, (hChn->coreOffset + BCHP_UARTA_RBR), lval);
	}

	/*
	 * Now that we've stuffed the FIFO, enable the TX int
	 */
	if ((hChn->intMode) || (hChn->txIntMode))
		BURT_EnableTxInt (hChn, true);
done:
	return( retCode );
}

BURT_RxError BURT_GetRxError (
	BURT_ChannelHandle	hChn			/* Device channel handle */
	)
{
	uint32_t 		lval;
	BURT_Handle		hDev;
	BURT_RxError 	rxError;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hUrt;

    BURT_P_ReadLsr(hChn, &lval);
	if( lval & BCHP_UARTA_LSR_OE_MASK )
		rxError = BURT_RxError_eReceiverOverRun;
	else if( lval & BCHP_UARTA_LSR_FE_MASK )
		rxError = BURT_RxError_eReceiverFrameError;
	else if( lval & BCHP_UARTA_LSR_PE_MASK )
		rxError = BURT_RxError_eReceiverParityError;
	else
		rxError = BURT_RxError_eNoError;

	return rxError;
}

BURT_RxError BURT_GetRxError_Isr (
	BURT_ChannelHandle	hChn			/* Device channel handle */
	)
{
	uint32_t 		lval;
	BURT_Handle		hDev;
	BURT_RxError 	rxError;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hUrt;

    BURT_P_ReadLsr_Isr(hChn, &lval);
	if( lval & BCHP_UARTA_LSR_OE_MASK )
		rxError = BURT_RxError_eReceiverOverRun;
	else if( lval & BCHP_UARTA_LSR_FE_MASK )
		rxError = BURT_RxError_eReceiverFrameError;
	else if( lval & BCHP_UARTA_LSR_PE_MASK )
		rxError = BURT_RxError_eReceiverParityError;
	else
		rxError = BURT_RxError_eNoError;

	return rxError;
}

/*******************************************************************************
*
*	Private Module Functions
*
*******************************************************************************/
void BURT_ResetTxRx(
	BURT_ChannelHandle 	hChn			/* Device channel handle */
	)
{
	BURT_Handle	hDev;

	hDev = hChn->hUrt;

	/* reset xmit and rcvr fifo's */
	BREG_Write32 (hDev->hRegister, (hChn->coreOffset + BCHP_UARTA_IIR), 7);	/* Note:  BCHP_UARTA_IIR shares same address as BCHP_UARTA_FCR */
}

void BURT_P_EnableTxRx(
	BURT_ChannelHandle 	hChn,			/* Device channel handle */
	bool				enableTx,		/* enable flag for transmitter */
	bool				enableRx		/* enable flag for receiver */
	)
{
	uint32_t	lval;
	BURT_Handle	hDev;

	hDev = hChn->hUrt;

    BSTD_UNUSED(lval);
    BSTD_UNUSED(enableTx);
    BSTD_UNUSED(enableRx);

	/* Enable fifo, reset xmit and rcvr fifo's */
	BREG_Write32 (hDev->hRegister, (hChn->coreOffset + BCHP_UARTA_IIR), 7);	/* Note:  BCHP_UARTA_IIR shares same address as BCHP_UARTA_FCR */
}

void BURT_P_SetBaudRate(
	BURT_ChannelHandle 	hChn,			/* Device channel handle */
	uint32_t			baud
)
{
	uint32_t	lval, bitRate;
	BURT_Handle	hDev;

	hDev = hChn->hUrt;

    BSTD_UNUSED(bitRate);

	/* Set DLAB bit to write DLL and DLH registers. */
	lval = BREG_Read32 (hDev->hRegister, hChn->coreOffset + BCHP_UARTA_LCR);
	lval &= ~BCHP_MASK(UARTA_LCR, DLAB);
	lval |= BCHP_FIELD_DATA(UARTA_LCR, DLAB, 1);
	BREG_Write32 (hDev->hRegister, hChn->coreOffset + BCHP_UARTA_LCR, lval);

	lval = (uint32_t)(5062500 / baud);	/* DLL = 81MHz / (16 * baud) */
	BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_UARTA_RBR, lval & 0xff);		/* Note:  BCHP_UARTA_RBR shares same address as BCHP_UARTA_DLL */
	BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_UARTA_IER, lval >> 8);		/* Note:  BCHP_UARTA_IER shares same address as BCHP_UARTA_DLH */

	/* Reset DLAB bit. */
	lval = BREG_Read32 (hDev->hRegister, hChn->coreOffset + BCHP_UARTA_LCR);
	lval &= ~BCHP_MASK(UARTA_LCR, DLAB);
	lval |= BCHP_FIELD_DATA(UARTA_LCR, DLAB, 0);
	BREG_Write32 (hDev->hRegister, hChn->coreOffset + BCHP_UARTA_LCR, lval);
}

void BURT_P_SetDataBits(
	BURT_ChannelHandle 	hChn,			/* Device channel handle */
	BURT_DataBits		bits
)
{
	uint32_t	lval;
	BURT_Handle	hDev;

	hDev = hChn->hUrt;

	lval = BREG_Read32 (hDev->hRegister, hChn->coreOffset + BCHP_UARTA_LCR);
	lval &= ~BCHP_MASK(UARTA_LCR, DLS);
	switch(bits)
	{
		case BURT_DataBits_eDataBits5:
			lval |= BCHP_FIELD_DATA(UARTA_LCR, DLS, 0);
			break;
		case BURT_DataBits_eDataBits6:
			lval |= BCHP_FIELD_DATA(UARTA_LCR, DLS, 1);
			break;
		case BURT_DataBits_eDataBits7:
			lval |= BCHP_FIELD_DATA(UARTA_LCR, DLS, 2);
			break;
		case BURT_DataBits_eDataBits8:
		default:
			lval |= BCHP_FIELD_DATA(UARTA_LCR, DLS, 3);
			break;
	}
	BREG_Write32 (hDev->hRegister, hChn->coreOffset + BCHP_UARTA_LCR, lval);
}

void BURT_P_SetParity(
	BURT_ChannelHandle 	hChn,			/* Device channel handle */
	BURT_Parity			parity
)
{
	uint32_t	lval;
	BURT_Handle	hDev;

	hDev = hChn->hUrt;

	lval = BREG_Read32 (hDev->hRegister, hChn->coreOffset + BCHP_UARTA_LCR);
	switch( parity )
	{
		case BURT_Parity_eNone:
			lval &= ~BCHP_MASK(UARTA_LCR, PEN);
			lval |= BCHP_FIELD_DATA(UARTA_LCR, PEN, 0);
			break;

		case BURT_Parity_eOdd:
			lval &= ~(BCHP_MASK(UARTA_LCR, EPS) |
					BCHP_MASK(UARTA_LCR, PEN));
			lval |= BCHP_FIELD_DATA(UARTA_LCR, EPS, 0) |
					BCHP_FIELD_DATA(UARTA_LCR, PEN, 1);
			break;

		case BURT_Parity_eEven:
			lval &= ~(BCHP_MASK(UARTA_LCR, EPS) |
					BCHP_MASK(UARTA_LCR, PEN));
			lval |= BCHP_FIELD_DATA(UARTA_LCR, EPS, 1) |
					BCHP_FIELD_DATA(UARTA_LCR, PEN, 1);
			break;
	}
	BREG_Write32 (hDev->hRegister, hChn->coreOffset + BCHP_UARTA_LCR, lval);
}

void BURT_P_SetStopBits(
	BURT_ChannelHandle 	hChn,			/* Device channel handle */
	BURT_StopBits		stop_bits
)
{
	uint32_t	lval;
	BURT_Handle	hDev;

	hDev = hChn->hUrt;

	lval = BREG_Read32 (hDev->hRegister, hChn->coreOffset + BCHP_UARTA_LCR);
	lval &= ~BCHP_MASK(UARTA_LCR, STOP);
	switch(stop_bits)
	{
		case BURT_StopBits_eOneBit:
			lval |= BCHP_FIELD_DATA(UARTA_LCR, STOP, 0);
			break;
		case BURT_StopBits_eTwoBit:
			lval |= BCHP_FIELD_DATA(UARTA_LCR, STOP, 1);
			break;
		default:
			lval |= BCHP_FIELD_DATA(UARTA_LCR, STOP, 0);
			break;
	}
	BREG_Write32 (hDev->hRegister, hChn->coreOffset + BCHP_UARTA_LCR, lval);
}

static void BURT_P_HandleInterrupt_Isr
(
	void *pParam1,						/* Device channel handle */
	int parm2							/* not used */
)
{
	BURT_ChannelHandle 	hChn;
	BURT_Handle 		hDev;
	uint32_t			lval;

	hChn = (BURT_ChannelHandle) pParam1;
	BDBG_ASSERT( hChn );
	BSTD_UNUSED( parm2 );

	hDev = hChn->hUrt;

	/* Check for transmit interrupt */
	lval = BURT_P_ReadLsr_Isr(hChn, 0);
	if (lval & BCHP_UARTA_LSR_TEMT_MASK)
	{
		BURT_EnableTxInt (hChn, false);				/* turn off TX interrupt */
#ifndef USE_BURT_INT_CALLBACK
		BKNI_SetEvent( hChn->hChnTxEvent );
#else
		if(hChn->intCallbackFunc == NULL)
		{
			BKNI_SetEvent( hChn->hChnTxEvent );
		}
		else
		{
			hChn->intCallbackFunc(pParam1,1); /* tx interrupt callback */
		}
#endif
	}

	/* Check for receive interrupt */
	lval = BURT_P_ReadLsr_Isr(hChn, 0);
	if (lval & BCHP_UARTA_LSR_DR_MASK)
	{
		BURT_EnableRxInt (hChn, false);				/* turn off RX interrupt */
#ifndef USE_BURT_INT_CALLBACK
		BKNI_SetEvent( hChn->hChnRxEvent );
#else
		if(hChn->intCallbackFunc == NULL)
		{
			BKNI_SetEvent( hChn->hChnRxEvent );
		}
		else
		{
			hChn->intCallbackFunc(pParam1,0); /* rx interrupt callback */
		}
#endif
	}

	return;
}

void BURT_P_EnableFifo(
	BURT_ChannelHandle 	hChn			/* Device channel handle */
)
{
    uint32_t fcraddr;
    fcraddr = hChn->coreOffset + BCHP_UARTA_FCR;
    BREG_Write32(hChn->hUrt->hRegister, fcraddr,
                 BCHP_FIELD_DATA(UARTA_FCR, RT, 0) /* first character */
                 | BCHP_FIELD_DATA(UARTA_FCR, TET, 0) /* empty */
                 | BCHP_FIELD_DATA(UARTA_FCR, FIFOE, 1) /* enable */
                 );
}

static uint32_t
BURT_P_ReadLsr_Isr(
    BURT_ChannelHandle  hChn,
    uint32_t *pErrLsr
)
{
    uint32_t lsraddr;
    uint32_t lval;

    lsraddr = hChn->coreOffset + BCHP_UARTA_LSR;
    lval = BREG_Read32(hChn->hUrt->hRegister, lsraddr);
    hChn->savedLsr |= lval;
    if (pErrLsr) {
        *pErrLsr = hChn->savedLsr;
        hChn->savedLsr = 0;
    }
    return lval;
}

static uint32_t
BURT_P_ReadLsr(
    BURT_ChannelHandle  hChn,
    uint32_t *pErrLsr
)
{
    uint32_t lval;
    BKNI_EnterCriticalSection();
    lval = BURT_P_ReadLsr_Isr(hChn, pErrLsr);
    BKNI_LeaveCriticalSection();
    return lval;
}

/* End of file */

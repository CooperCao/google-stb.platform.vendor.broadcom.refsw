/***************************************************************************
*     Copyright (c) 2003-2011, Broadcom Corporation
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
* Module Description: Video Shmoo Test
*
* Revision History:
*
* $brcm_Log: $
* 
**************************************************************************/

#include <stdio.h>
#include <string.h>
#include "framework.h"

#include "framework_board_bvdc.h"

#include "btst_video_shmoo.h"


BDBG_MODULE(VIDEO_SHMOO_TEST);

/**************************************************************************/
int app_main( int argc, char **argv )
{
	BSystem_Info sysInfo;
	BFramework_Info frmInfo;

	BERR_Code               err = BERR_SUCCESS;

	/* System Init (interrupts/memory mapping) */
	int iErr = BSystem_Init( argc, argv, &sysInfo );
	if ( iErr )
	{
		BDBG_ERR(( "System init FAILED!" ));
		return iErr;
	}

	/* Framework init (base modules) */
	iErr = BFramework_Init( &sysInfo, &frmInfo );
	if ( iErr )
	{
		BDBG_ERR(( "Framework init FAILED!" ));
		return iErr;
	}


#if (!FRAMEWORK_DO_SHMOO)
	if (BTST_VideoShmoo(frmInfo.hMem, frmInfo.hChp, frmInfo.hReg) != BERR_SUCCESS)
	{
		BDBG_ERR(("Video Shmoo test failed."));
	}
	else
	{
		BDBG_ERR(("Video Shmoo test passed."));
	}
#endif

	BFramework_Uninit(&frmInfo);
	BSystem_Uninit(&sysInfo);

	BDBG_MSG(("Test complete."));

	return err;
}

/* End of file */

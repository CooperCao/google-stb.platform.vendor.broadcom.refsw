
#include "bstd.h"
#include "bsynclib_timer.h"

BDBG_MODULE(synclib);

/*
Summary:
*/
void BSYNClib_Timer_Init(BSYNClib_Timer * psTimer)
{
	BDBG_ASSERT(psTimer);
	BKNI_Memset(psTimer, 0, sizeof(BSYNClib_Timer));
}

/*
Summary:
*/
void BSYNClib_Timer_Reset_isr(BSYNClib_Timer * psTimer)
{
	BDBG_ASSERT(psTimer);
	psTimer->bScheduled = false;
}


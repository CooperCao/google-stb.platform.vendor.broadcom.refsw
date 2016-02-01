#include "dsgcc.h"
#include "dsgcc_clnt_if.h"
#include "DsgClientRegistrar.h"

int 
registerdsgclient(dsgClientRegInfo *regInfo)
{
	BcmDsgClientRegistrarThread *pThread = BcmDsgClientRegistrarThread::GetSingletonInstance();
	if (pThread) {
		return pThread->addDsgClient(regInfo);
	}
	return -1;
}

int
unregisterdsgclient(dsgClientRegInfo *regInfo)
{
	BcmDsgClientRegistrarThread *pThread = BcmDsgClientRegistrarThread::GetSingletonInstance();
	if (pThread) {
		return pThread->deleteDsgClient(regInfo);
	}
	return -1;
}

int
keepalivedsgclient(dsgClientRegInfo *regInfo)
{
	BcmDsgClientRegistrarThread *pThread = BcmDsgClientRegistrarThread::GetSingletonInstance();
	if (pThread) {
		int retVal = pThread->statusDsgClientEntry(regInfo); 
		if (retVal == 0)
			return pThread->addDsgClient(regInfo);

	}
	return -1;
}

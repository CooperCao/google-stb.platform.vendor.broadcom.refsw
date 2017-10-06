#include <stdio.h>
#include <tchar.h>
#include "wpsutils.h"

void GetMacAddrString(const unsigned char mac[], char *szMacAddr, size_t nCount)
{
	_snprintf(szMacAddr, nCount, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

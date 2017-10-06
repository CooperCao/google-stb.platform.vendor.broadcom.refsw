

#include <stdio.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <winioctl.h>
#include <malloc.h>
#include <ntddndis.h>
#include "epiioctl.h"
#include "epictrl.h"


void
dumpbytes(char *buffer, int nbytes) 
{
	int i;

	for (i = 0; i < nbytes; i++) {
		if (isprint(buffer[i])) {
			printf("%c", buffer[i]);
		} else {
			printf("\\%03o", buffer[i]);
		}
	}
}

main(int argc, char *argv[])
{
    char buffer[200];
    DWORD status, nbytes;
	int instanceId = 0;

	status = EpiControlIfOpen();
	printf("status returned by EpiControlIfOpen = 0x%x (%d)\n", 
		   status, status);
	
	nbytes = sizeof(buffer);
	instanceId = atol(argv[1]);
    status = EpiControlIfCmd(instanceId, OID_GEN_VENDOR_DESCRIPTION, 
							 buffer, &nbytes,
							 EPICTRL_GET_DATA, NULL);
	printf("status returned by EpiControlIfCmd = 0x%x (%d)\n", 
		   status, status);
	if (status == ERROR_SUCCESS) {
		printf("VENDOR_DESCRIPTION nbytes = %d\n", nbytes);
		printf("VENDOR_DESCRIPTION = \"");
		dumpbytes(buffer, nbytes);
		printf("\"\n", buffer);

	}

	EpiControlIfClose();

}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/ethernet.h>	/* the L2 protocols */
#include <netinet/in.h>
#include <netpacket/packet.h>


#define SIOCPREPAREFORRESET (SIOCDEVPRIVATE + 7)

typedef struct BcmEnet_ioctlparms {
    unsigned int    from;
    unsigned int    to;
    unsigned int    length;
} BcmEnet_ioctlparms;

static int pciDriver;
struct ifreq ifr;

int main(int argc, char *argv[])
{
    BcmEnet_ioctlparms ioctlParms;

    // Check for two parameters
	if (argc != 2)
	{
        printf("Missing interface parameter\nUsage: %s interface\n", argv[0]);
		exit(1);
	}

    // Open the PCI driver
    pciDriver = socket(AF_INET, SOCK_DGRAM, 0);

    if (pciDriver < 0)
    {
        perror("Could not open the BCMPCIENET driver");
        exit(1);
    }

    strcpy(ifr.ifr_name, argv[1]);
	
    ifr.ifr_data = (void *)&ioctlParms;
    if (ioctl(pciDriver, SIOCPREPAREFORRESET, (void *)&ifr) < 0)
    {
        perror("IOCTL error getting boot parameters from BCMPCIENET driver");
        exit(1);
    }


    return 0;
}

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "msutil.h"
#include "msdiag.h"

void printUsage(void) {
    printf(
    "Usage: msfsck OPTIONS\n"
    "Options:\n"
    "  -all               print all\n"
    "  -r                 do recover\n"
    "  -m [MOUNT NAME]    use existing mount name. for already mounted volume\n"
    "  -d [DEVICE NAME]   mount and check. volume will be mounted to /tmp/dvr\n"
    );
}

extern int check_mount_volume(const char *device, char *mountname);

int main(int argc, char *argv[])
{
    int ret = eMS_OK;
    int i;
    int level=1;
    int mount=0;
    int device=0;
    char mountname[128];
    char devname[128];
    int recover=0;

    if (argc<2) {
        printUsage();
        exit(0);
    }        

    system("umount /tmp/dvr* 2>/dev/null");

    for (i=1;i<argc;i++) {
        if (!strcmp(argv[i], "--help")) {
            printUsage();
            exit(0);
        }
        else if (!strcasecmp("-m", argv[i]) && i+1<argc) {
            sprintf(mountname,"%s",argv[++i]);
            mount=1;
        }            
        else if (!strcasecmp("-d", argv[i]) && i+1<argc) {
            sprintf(devname,"%s",argv[++i]);
            device=1;
        }
        else if (!strcasecmp("-all", argv[i]))
            level = 2;
        else if (!strcasecmp("-r", argv[i]))
            recover = 1;
    }

    if (device) {
        if (check_mount_volume(devname,mountname)) exit(1);
    }

    ms_set_level(level);
    ms_recover_volume(mountname,recover);

    if (device)
        ms_unmount_volume(mountname);

    return ret;
}

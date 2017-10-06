
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define WFA_UCC_INIT_FILE "/home/qiuminh/wmm/e2eVoice/wmm2000/ucc/ucc_cnxt.txt"

void wfa_ucc_init_setup(FILE *fp);

int main(int argc, char *argv[])
{
    FILE *wfa_ucc_fp = NULL;

    wfa_ucc_fp = fopen("/tmp/ucc_cnxt.txt", "r+");

    if(wfa_ucc_fp == NULL) 
    {
       printf("error opening file\n");
       return 0;
    }

    wfa_ucc_init_setup(wfa_ucc_fp);

    return 0;
}

void wfa_ucc_init_setup(FILE *fp)
{
   char strline[1024];

   while(fscanf(fp, "%s", strline) != EOF)
   {
       if(
       printf("%s\n", strline);
   }
}

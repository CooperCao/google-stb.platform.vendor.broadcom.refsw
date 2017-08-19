/*
 * srec2mem.c -- to convert file from srec format to mem format (mif) format
 * compile:
		gcc -o srec2mem srec2mem.c
 * usage: srec2mem filename.srec 8/16/32 [flash][split 2/4]\n");
 *        32 bit mems can only have 1 output file\n");
 *        16 bit mems can have 1 or 2\n");
 *        8 bit mems can have 1, 2, or 4\n");
 * 
 * output(s):	filename.mem or filename.mem, filename_1.mem...
 *           or filename_0.srec->filename_0.mem, filename_1.mem...
 *
 */ 

#include <stdio.h>
#include <libelf.h>
#include <fcntl.h>
#include <string.h>

typedef unsigned int uint32;
typedef unsigned short uint16;

static unsigned int base_addr=0, base_addr_flag=0;

#define BIG_ENDIAN 0
#define LITTLE_ENDIAN 1
#define BUFSIZE 200
typedef struct _Symbol_ {
  struct _Symbol_ * next;
  char * label;
  uint32 address;
} Symbol;

static Symbol *symList = NULL;
static char buffer[BUFSIZE+1];

uint32 map_address(address)
  uint32 address;
{
	if (base_addr_flag) {
		address -= base_addr;
		return address;
	}
	
		
  if (address < 0x80000000) {
     address += 0x40000000;
  }
  else if (address < 0xa0000000) {
    address -= 0x80000000;
  }
  else if (address < 0xc0000000) {
    address -= 0xa0000000;
  }
  if (address >= 0x1fc00000)
    address -= 0x1fc00000;
	
  return(address);
}


      
int main (int argc, char ** argv)
{
  /* initialize and configure access routines */
  int num_params;
  char * srecname;
  char *outname_holder;
  char outname_0[30], outname_1[30], outname_2[30], outname_3[30];
  char  tfname[25];
  char * addressstr;
  char * datastr;
  char bytestr[3];

  uint32 linecount = 0;

  Symbol *sym;

  char address_str[9];
  char count_str[3];
  char type[3];
  char byte_str0[3];
  char byte_str1[3];
  char byte_str2[3];
  char byte_str3[3];
  char byte_str4[3];
  char byte_str5[3];
  char byte_str6[3];
  char byte_str7[3];
  char * bufptr;

  int count;
  int bytecount = 0;
  int flag8 = 0, flag16 = 0, flag32 = 0, flag64=0;
  int flash = 0;
  unsigned char byte;
  uint32 address;
  int i;
  int filenum=1;
  int usage = 0; //print the usage message (bad input)
  int avpflag = 0;

  static char label[100];
  int symbol_count = 0;
  int srec_type = BIG_ENDIAN;
  FILE * file, *outfile_0, *outfile_1, *outfile_2, *outfile_3;

  //Take in the arguments, spit out usage message if no good.
  for(i=1; i<argc; i++){
    if(strstr(argv[i],".srec")){
      srecname = argv[i];
      strcpy(tfname, argv[i]);
      outname_holder = strtok(tfname, ".");
    }else if(!strcmp(argv[i], "flash")){
      flash = 1;
    }else if(!strcmp(argv[i], "8")){
      flag8=1;
    }else if(!strcmp(argv[i], "16")){
      flag16=1;
    }else if(!strcmp(argv[i], "32")){
      flag32=1;
    }else if(!strcmp(argv[i], "64")){
      flag64=1;
    }else if(!strcmp(argv[i], "split")){
      if(!strcmp(argv[++i], "2")){
	filenum = 2;
      }else if(!strcmp(argv[i], "4")){
	filenum = 4;
      }
    }else if (!strcmp(argv[i], "-b")) {
	base_addr_flag = 1;
	base_addr = strtoul(argv[i+1], NULL, 16);
	printf("base address is %x\n", base_addr);
	i++;
     }else{
      usage = 1;
    }
  }

  //Now make sure flags and splits agree:
  if(filenum==2){
    if((flag8+flag16)!=1){
      usage = 1;
    }
  }else if(filenum==4){
    if(flag8!=1){
      usage = 1;
    }
  }

  if(usage == 1){
    printf(" usage: srec2mem filename.srec 8/16/32 [flash][split 2/4]\n");
    printf("        32 bit mems can only have 1 output file\n");
    printf("        16 bit mems can have 1 or 2\n");
    printf("        8 bit mems can have 1, 2, or 4\n");
    printf(" output:filename.srec-> filename.mem, filename_1.mem...\n");
    printf("        filename_0.srec-> filename_0.mem, filename_1.mem...\n");
    exit (-1);
  }

  //Try to open input file:
  if (!(file = fopen(srecname,"r"))) {
    printf ("Error, Unable  to open file %s\n",srecname);
    exit (-1);
  }

  
  strcpy(outname_0, outname_holder);
  strcat(outname_0, ".mem");
  if (!(outfile_0 = fopen (outname_0, "a"))) {
    printf ("Error, Unable  to open file %s\n",outname_0);
    exit (-1);
  }

  //How many output files do we have?  Try to open each of them.
  if(filenum>=2){
    printf("Opening file number 1...\n");
    if(strstr(tfname, "0")){
      outname_holder = strtok(tfname, "0");
    }
    strcpy(outname_1, outname_holder);
    strcat(outname_1, "1.mem");
    if (!(outfile_1 = fopen (outname_1, "a"))) {
      printf ("Error, Unable  to open file %s\n",outname_1);
      exit (-1);
    }
    if(filenum==4){
      strcpy(outname_2, outname_holder);
      strcat(outname_2, "2.mem");
      if (!(outfile_2 = fopen (outname_2, "a"))) {
	printf ("Error, Unable  to open file %s\n",outname_2);
	exit (-1);
      }
      strcpy(outname_3, outname_holder);
      strcat(outname_3, "3.mem");
      if (!(outfile_3 = fopen (outname_3, "a"))) {
	printf ("Error, Unable  to open file %s\n",outname_3);
	exit (-1);
      }
    }
  }

  //Here we process the input file line by line
  while (!feof(file)) {

    fgets(buffer,BUFSIZE,file);

    if (feof(file)) break;

    strncpy(type,buffer,2);
    type[2] = NULL;

    if (!strcmp(type,"EL")) {
      srec_type = LITTLE_ENDIAN;
      printf ("Loading file in Little endian mode\n");
    }
    else if (!strcmp(type,"S0")) {
      bufptr = buffer;
    }
    else if (!strcmp(type,"S4")) {
      bufptr = buffer + 2;
      strncpy(count_str,bufptr,2);
      bufptr+= 2;
      count_str[2] = NULL;
      strncpy(address_str,bufptr,8);
      bufptr+= 8;
      address_str[8] = NULL;

      count = strtoul(count_str,NULL,16);
      
      address = map_address(strtoul(address_str,NULL,16));
      strcpy(label,bufptr);
      if (strlen(label) > 4) label[strlen(label)-4] = 0;

      sym = (Symbol *) malloc(sizeof(Symbol));
      sym->label = (char *) malloc(strlen(label) + 2);
      strcpy(sym->label,label);
      sym->address = address;
      sym->next = symList;
      symList = sym;
      symbol_count++;
    }
    else if (!strcmp(type,"S3")) {
      bufptr = buffer + 2;
      strncpy(count_str,bufptr,2);
      bufptr+= 2;
      count_str[2] = NULL;
      strncpy(address_str,bufptr,8);
      bufptr+= 8;
      address_str[8] = NULL;

      count = strtoul(count_str,NULL,16);
      if (avpflag == 1)
	count /=  2;
      address = map_address(strtoul(address_str,NULL,16));
      for (i=0;i < count - 5; i++) {

        strncpy(byte_str0,bufptr,2);
        byte_str0[2] = NULL;
        bufptr+=2;

        byte = strtoul(byte_str0,NULL,16);

        if (srec_type == LITTLE_ENDIAN){
             bytecount++;
	     if (bytecount == 1)
               strcpy(byte_str3, byte_str0);
             else if (bytecount == 2)
                strcpy(byte_str2, byte_str0);
             else if (bytecount ==3)
                strcpy(byte_str1, byte_str0);
             else {
                bytecount = 0;
                if (flag8 == 1) {
		  if(filenum==1){
		    fprintf(outfile_0,"%x/%s;\n",  address-3, byte_str0);
		    fprintf(outfile_0,"%x/%s;\n",  address-2, byte_str1);
		    fprintf(outfile_0,"%x/%s;\n",  address-1, byte_str2);
		    fprintf(outfile_0,"%x/%s;\n",  address,   byte_str3);
		  }else if(filenum==2){
		    fprintf(outfile_1,"%x/%s;\n",  (address-3)>>1, byte_str0);
		    fprintf(outfile_0,"%x/%s;\n",  (address-3)>>1, byte_str1);
		    fprintf(outfile_1,"%x/%s;\n",  (address-1)>>1, byte_str2);
		    fprintf(outfile_0,"%x/%s;\n",  (address-1)>>1, byte_str3);
		  }else if(filenum==4){
		    fprintf(outfile_3,"%x/%s;\n",  (address-3)>>2, byte_str0);
		    fprintf(outfile_2,"%x/%s;\n",  (address-3)>>2, byte_str1);
		    fprintf(outfile_1,"%x/%s;\n",  (address-3)>>2, byte_str2);
		    fprintf(outfile_0,"%x/%s;\n",  (address-3)>>2, byte_str3);
		  }
		}
                if (flag16==1) {
		  if (flash == 0 ) {
		    if(filenum==1){
		      fprintf(outfile_0,"%x/%s%s;\n",  (address-3) >>1,byte_str0, byte_str1);
		      fprintf(outfile_0,"%x/%s%s;\n",  (address-1) >>1,byte_str2, byte_str3);
		    }else if(filenum==2){
		      fprintf(outfile_1,"%x/%s%s;\n",  (address-3)>>2,byte_str0, byte_str1);
		      fprintf(outfile_0,"%x/%s%s;\n",  (address-3)>>2,byte_str2, byte_str3);
		    }		      
		  }else{
		    fprintf(outfile_0,"%x/%s%s;\n",  address-3,byte_str0, byte_str1);
		    fprintf(outfile_0,"%x/%s%s;\n",  address-1,byte_str2, byte_str3);
		  }
                }
                if (flag32==1) {
		  fprintf(outfile_0,"%x/%s%s%s%s;\n",  (address-3)>>2,byte_str0, byte_str1,
			  byte_str2, byte_str3);
                }
		if (flag64==1) {
		  fprintf(outfile_0,"%x/%s%s%s%s%s%s%s%s;\n",  (address-7)>>3,
			byte_str0, byte_str1, byte_str2, byte_str3, 
			byte_str4, byte_str5, byte_str6, byte_str7);
                }
	     }

	} else {
	  //Big Endian Case
	     bytecount++;
             if (bytecount == 1)
	        strcpy(byte_str7, byte_str0);
             else if (bytecount == 2) 
	        strcpy(byte_str6, byte_str0);
	     else if (bytecount ==3)
	        strcpy(byte_str5, byte_str0);
	     else if (bytecount == 4)
	        strcpy(byte_str4, byte_str0);
             else if (bytecount == 5) 
	        strcpy(byte_str3, byte_str0);
	     else if (bytecount ==6)
	        strcpy(byte_str2, byte_str0);
	     else if (bytecount ==7)
	        strcpy(byte_str1, byte_str0);
	   else {
                bytecount = 0;
		if (flag8 == 1) {
		  if(filenum==1){
		    fprintf(outfile_0,"%x/%s;\n",  address-3, byte_str3);
		    fprintf(outfile_0,"%x/%s;\n",  address-2, byte_str2);
		    fprintf(outfile_0,"%x/%s;\n",  address-1, byte_str1);
		    fprintf(outfile_0,"%x/%s;\n",  address,   byte_str0);
		  }else if(filenum==2){
		    fprintf(outfile_0,"%x/%s;\n",  (address-3)>>1, byte_str0);
		    fprintf(outfile_1,"%x/%s;\n",  (address-3)>>1, byte_str1);
		    fprintf(outfile_0,"%x/%s;\n",  (address-1)>>1, byte_str2);
		    fprintf(outfile_1,"%x/%s;\n",  (address-1)>>1, byte_str3);
		  }else if(filenum==4){
		    fprintf(outfile_3,"%x/%s;\n",  (address-3)>>2, byte_str3);
		    fprintf(outfile_2,"%x/%s;\n",  (address-2)>>2, byte_str2);
		    fprintf(outfile_1,"%x/%s;\n",  (address-1)>>2, byte_str1);
		    fprintf(outfile_0,"%x/%s;\n",  (address)>>2,   byte_str0);
		  }
		}

		if (flag16==1) {
		  if (flash == 0 ) {
		    if(filenum==1){
		      fprintf(outfile_0,"%x/%s%s;\n",  (address-3)>>1,byte_str1, byte_str0);
		      fprintf(outfile_0,"%x/%s%s;\n",  (address-1)>>1,byte_str3, byte_str2);
		    }else{
		      fprintf(outfile_0,"%x/%s%s;\n",  (address-3)>>2,byte_str1, byte_str0);
		      fprintf(outfile_1,"%x/%s%s;\n",  (address-3)>>2,byte_str3, byte_str2);
		    }
		  }else{
		    fprintf(outfile_0,"%x/%s%s;\n",  address-3,byte_str1, byte_str0);
		    fprintf(outfile_0,"%x/%s%s;\n",  address-1,byte_str3, byte_str2);
		  }
		}
		if (flag32==1) {
		  fprintf(outfile_0,"%x/%s%s%s%s;\n",  (address-3) >>2 ,byte_str3, byte_str2,
			  byte_str1, byte_str0);
		}
		if (flag64==1) {
		  fprintf(outfile_0,"%x/%s%s%s%s%s%s%s%s;\n",  (address-7)>>3,
			byte_str3, byte_str2, byte_str1, byte_str0,
			byte_str7, byte_str6, byte_str5, byte_str4);
                }
	     }
	}
        address++;
      }
    }else{
      ;
    }
  }

  if(filenum==4){
    close(outfile_3);
    close(outfile_2);
  }
  if(filenum>=2){
    close(outfile_1);
  }
  close(outfile_0);
  close(file);
  return (0);
	
}


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* FILE-CSTYLED */
/***********
 *
 * Parse the CSV file created by the Yokagowa SDIO analyser tool.
 * Usage: ydl_parse foo.csv
 *
 **********/
#define START_OF_LINE 7
#define CMD53_START 35
#define CMD53_REG_OFFSET 38
#define CMD53_NIBBLE1 22
#define CMD53_NIBBLE2 17
#define CMD53_BYTE_COUNT 45
#define CMD53_INT_DELTA 8

#define CMD52_REG_OFFSET 38
#define CMD52_FUNC 35
#define CMD52_WDATA 43
#define R5_RDATA 22

#define HARD_CODED_BLOCK_SIZE 64

int
main(int argc, char *argv[])
{
	FILE *file;
	char buf[512];
	char scratch[10];
	char *ptr, *buf_ptr;
	char databuf[10];
	int i, seq;
	//int ns;
	unsigned int reg32;
	
	if (argc <=1){
		printf("Usage: %s yokagowa .csv_file\n", argv[0]);
		exit(0);
	}
	/* TODO: Make his accept stdin */
	sprintf(buf, "cat %s |  tr -d \\\\042", argv[1]);
	if ((file = popen(buf, "r")) == NULL){
		printf("Usage: %s yokagowa .csv_file\n", argv[0]);
		exit(0);
	}

	while (fgets(buf, sizeof(buf), file) != NULL){
		buf_ptr = buf;
		ptr = buf_ptr + 1;
		seq = strtol(ptr, NULL, 0);
		ptr = buf_ptr + START_OF_LINE;
		if (strncmp(ptr, "CMD53", strlen("CMD53")) == 0){
			int byte_len;
			int block_mode = 0;

			printf("%d ", seq);


			/* Sanity check command index byte */
			ptr = buf_ptr+CMD53_START;
			if (strncmp("DIO_INT", ptr, 7) == 0){
				//Work in progress
				buf_ptr += CMD53_INT_DELTA;
				ptr += CMD53_INT_DELTA;
			}
			strncpy(scratch, ptr, 2);
			scratch[2] = 0;
			if (atoi(scratch) != 75)
				printf("What the heck - not 75 - %ul :%s:\n", atoi(scratch), scratch);

			/* Extract direction, func, block mode */
			ptr += 2;
			strncpy(scratch, ptr, 2);
			scratch[2] = 0;
			reg32 = strtol(scratch, NULL, 16);
			printf("Cmd 53 ");
			if (reg32 & 0x80)
				printf("W  ");
			else
				printf("R  ");
			block_mode = reg32 & 0x8 ? 1 : 0;
			printf("F %d ", (reg32 & 0x70)>> 4);
			if (block_mode)
				printf("Block ");
			else
				printf("Byte  ");


			/* Extract register offset */
			ptr = buf_ptr + CMD53_REG_OFFSET;
			databuf[0] = *ptr;
			ptr += 2; //Skip space
			strncpy(&databuf[1], ptr, 4);
			databuf[5] = 0;
			reg32 = strtol(databuf, NULL, 16);
			printf("Addr 0x%x ", (reg32 >> 1) & 0x1ffff);

			/* Extract data length, I'm not handling
			 * the high bit of length. */
			ptr = buf_ptr + CMD53_BYTE_COUNT;
			strncpy(scratch, ptr, 2);
			scratch[2] = 0;
			byte_len = strtol(scratch, NULL, 16);
			printf("%d %s ", byte_len, block_mode ? "Blocks" : "Bytes");

			fgets(buf, sizeof(buf), file); //Swallow R5
			fgets(buf, sizeof(buf), file); //Fetch data
			
			if (!block_mode){
				if (byte_len <= 4){
					// Extract and swap data
					ptr = buf_ptr + CMD53_NIBBLE1;
					for (i=0;i<4;i+=2){
						databuf[i] = *(ptr-1);
						databuf[i+1] = *ptr;
						ptr -= 2;
					}
					databuf[4]=0;

					if (byte_len > 2){
						ptr = buf_ptr + CMD53_NIBBLE2;
						for (i=4;i<8;i+=2){
							databuf[i] = *(ptr-1);
							databuf[i+1] = *ptr;
							ptr -= 2;
						}
						databuf[8]=0;
					}
					reg32 = strtoul(databuf, NULL, 16);
					printf("Data: 0x%x\n", reg32);
				} else {
					printf("\n");
					/* I have no idea why this hack is needed (but it is). */
					if (byte_len == 28 || byte_len == 60)
						byte_len += 16;
					byte_len -= 16;
					while (byte_len >= 0){
						fgets(buf, sizeof(buf), file); //Swallow response
						byte_len -= 16;
					}
				}
			} else {
				printf("\n");
				/* Handle block mode here */
				byte_len = (byte_len * HARD_CODED_BLOCK_SIZE) + ((byte_len-1) * 16);
				while (byte_len > 0){
					//printf("Block mode: Byte len %d\n", byte_len);
					fgets(buf, sizeof(buf), file); //Swallow data
					byte_len -= 16;
				}
			}
			/****Fetch nanosecs */
			//ptr = buf_ptr + 66;
			//*(ptr+11) = 0;
			//ns = strtol(ptr, NULL, 10);
			//printf(" ns = %d\n", ns);
			continue;
		}
		if (strncmp(ptr, "CMD52", strlen("CMD52")) == 0){
			int is_write = 0;

			/* R/W & Func */
			ptr = buf_ptr + CMD52_FUNC;
			strncpy(scratch, ptr, 2);
			scratch[2] = 0;
			reg32 = strtol(scratch, NULL, 16);
			is_write = reg32 & 0x80;
			printf("Cmd 52 %s Func %d ", is_write ? "Wr" : "Rd", (reg32 & 0x70) >> 4);

			/* Reg offset */
			ptr = buf_ptr + CMD52_REG_OFFSET;
			strncpy(scratch, ptr, 4);
			scratch[4] = 0;
			reg32 = strtol(scratch, NULL, 16);
			printf("Addr 0x%-6x ", reg32 >> 1);
			
			/* If writing, data is in command */
			if (is_write){
				ptr = buf_ptr + CMD52_WDATA;
				strncpy(scratch, ptr, 2);
				scratch[2] = 0;
				reg32 = strtol(scratch, NULL, 16);
				printf("Data: 0x%x ", reg32 & 0xff);
			}

			/* Get and parse R5 response */
			fgets(buf, sizeof(buf), file); 
			if (!is_write){
				ptr = buf_ptr + R5_RDATA;
				strncpy(scratch, ptr, 2);
				scratch[2] = 0;
				reg32 = strtol(scratch, NULL, 16);
				printf("Data: 0x%x", reg32 & 0xff);
			}
			printf("\n");
			continue;
		}
		if (strncmp(ptr, "DATA", strlen("DATA")) == 0){
			printf("-------  Bad DATA: %s", ptr);
			continue;
		}
		if (strncmp(ptr, "R5", strlen("R5")) == 0){
			printf("-------  Bad R5: %s", ptr);
			continue;
		}
		if (strncmp(ptr, "CMD5", strlen("CMD5")) == 0){
			printf("CMD5 \n");
			fgets(buf, sizeof(buf), file); //Swallow response
			continue;
		}
		if (strncmp(ptr, "CMD7", strlen("CMD7")) == 0){
			printf("CMD7 \n");
			fgets(buf, sizeof(buf), file); //Swallow response
			continue;
		}
		if (strncmp(ptr, "CMD3", strlen("CMD3")) == 0){
			printf("CMD3 \n");
			fgets(buf, sizeof(buf), file); //Swallow response
			continue;
		}
		//printf("Unknown line: %s\n", ptr);
	}
	return(0);
}

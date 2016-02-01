/*
 * No-OS Diags stub functions
 */
#include <string.h>
#include <stdlib.h> /* atoi */
#include <stdio.h>

uint32_t readl( uint32_t addr )
{
    return *(volatile uint32_t *)addr;
}

void writel( uint32_t val, uint32_t addr )
{
    *(volatile uint32_t *)addr = val;
}

int open(void)
{
	printf("open stubbed\n");
	return -1;
}

void fstat(void)
{
	printf("fstat stubbed\n");
}

int unlink(const char *c)
{
//	BSTD_UNUSED(c);
	printf("unlink stubbed\n");
	return 0;
}

off_t ftello(FILE *stream)
{
	//BSTD_UNUSED(stream);
	printf("ftello stubbed\n");
	return 0;
}

int fseeko(FILE *stream, off_t offset, int whence)
{
	//BSTD_UNUSED(stream);
	//BSTD_UNUSED(offset);
	//BSTD_UNUSED(whence);
	printf("fseeko stubbed\n");
	return 0;
}


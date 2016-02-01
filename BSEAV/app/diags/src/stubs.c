/*
 * No-OS Diags stub functions
 */
#include "bsettop_impl.h"
#include <string.h>
#include <stdlib.h> /* atoi */
#include <stdio.h>

int baudio_capture_open(baudio_decode_t audio)
{
	BSTD_UNUSED(audio);
	printf("baudio_capture_open stubbed\n");
	return 0;
}

int baudio_capture_close(baudio_decode_t audio)
{
	BSTD_UNUSED(audio);
	printf("baudio_capture_close stubbed\n");
	return 0;
}

int baudio_capture_start(baudio_decode_t audio)
{
	BSTD_UNUSED(audio);
	printf("baudio_capture_start stubbed\n");
	return 0;
}

int baudio_capture_stop(baudio_decode_t audio)
{
	BSTD_UNUSED(audio);
	printf("baudio_capture_stop stubbed\n");
	return 0;
}

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

#if 0 /*ndef LINUX*/
#if ((BCM_BOARD != 97019) && (BCM_BOARD != 97125))
void _exit(void)
{
	printf("_exit stubbed\n");
}
#endif
#endif

void fstat(void)
{
	printf("fstat stubbed\n");
}

int unlink(const char *c)
{
	BSTD_UNUSED(c);
	printf("unlink stubbed\n");
	return 0;
}

off_t ftello(FILE *stream)
{
	BSTD_UNUSED(stream);
	printf("ftello stubbed\n");
	return 0;
}

int fseeko(FILE *stream, off_t offset, int whence)
{
	BSTD_UNUSED(stream);
	BSTD_UNUSED(offset);
	BSTD_UNUSED(whence);
	printf("fseeko stubbed\n");
	return 0;
}


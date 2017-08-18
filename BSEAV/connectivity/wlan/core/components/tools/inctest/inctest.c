/**
 * @file
 * @brief
 * Source file for inctest
 *
 * $ Copyright Broadcom Corporation $
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 */

#define EFI_WINBLD
#include <inctest.h>
#include <stdio.h>
#include <inctest.h>

int main(int argc, char **argv)
{
	(void)argc;
	char *name = argv[0];
	int i = 0, j = 0, ret = 0;

	for (; i < get_arr_size(); i++) {
		if (inc_test_arr_packed[i] != inc_test_arr_unpacked[i]) {
			if (j == 0) { /* Print only the first time */
				(void)fprintf(stderr,
					"%s index/count name  => unpacked:packed\n", name);
			}
			j++;
			ret = 1;
			(void)fprintf(stderr,
				"%s Error: %03d/%03d %s => %d:%d failed the packing test\n",
				name, i, j, get_str_name(i), inc_test_arr_unpacked[i],
				inc_test_arr_packed[i]);
		}
	}
	return ret;
}

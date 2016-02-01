#include <linux/module.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bkni_test.h"

int init_module(void)
{
	BERR_Code result;
	printk("init_module\n");

	BKNI_Init();
	result = BKNI_TestAll();
	printk(
		"*************************\n"
		"%s\n"
		"*************************\n",
		result?"Failure":"Success");
	BKNI_Uninit();

	return 0;
}

int cleanup_module(void)
{
	printk("cleanup_module\n");
	return 0;
}

#if defined(MODULE_LICENSE)
MODULE_LICENSE("Proprietary");
#endif

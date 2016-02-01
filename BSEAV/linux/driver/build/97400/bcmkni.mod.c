#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>
#include <linux/version.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

#undef unix
struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = __stringify(KBUILD_MODNAME),
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";
#else
static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";
#endif

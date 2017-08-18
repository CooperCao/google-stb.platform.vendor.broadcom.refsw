#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

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

static const struct modversion_info ____versions[]
__attribute_used__
__attribute__((section("__versions"))) = {
	{ 0xfee8a8b6, "struct_module" },
	{ 0x7da8156e, "__kmalloc" },
	{ 0xf34350ab, "__mod_timer" },
	{ 0x2be9b127, "__kfree_skb" },
	{ 0x1683aa15, "del_timer" },
	{ 0x6c3397fb, "malloc_sizes" },
	{ 0xeaa52983, "usb_deregister_dev" },
	{ 0xd8c152cd, "raise_softirq_irqoff" },
	{ 0x1d26aa98, "sprintf" },
	{ 0xfdb8b8b6, "usb_unlink_urb" },
	{ 0xda02d67, "jiffies" },
	{ 0x9925ce9c, "__might_sleep" },
	{ 0xd08f33d, "skb_unlink" },
	{ 0x4833bed7, "usb_deregister" },
	{ 0x1b7d4074, "printk" },
	{ 0x5977e891, "alloc_skb" },
	{ 0xcbf92b8b, "__down_failed_trylock" },
	{ 0x66ab0670, "usb_register_dev" },
	{ 0x90179f85, "usb_driver_claim_interface" },
	{ 0x28c3bbf5, "__down_failed_interruptible" },
	{ 0x49e79940, "__cond_resched" },
	{ 0xcd8355bb, "skb_over_panic" },
	{ 0x1a76b3c7, "usb_submit_urb" },
	{ 0x123d3b6a, "kmem_cache_alloc" },
	{ 0x17d59d01, "schedule_timeout" },
	{ 0xffdac03f, "usb_driver_release_interface" },
	{ 0x4d075021, "usb_register" },
	{ 0x57a6504e, "vsnprintf" },
	{ 0xab821cad, "__wake_up" },
	{ 0xf1b8777e, "get_user_size" },
	{ 0x37a0cba, "kfree" },
	{ 0x2e60bace, "memcpy" },
	{ 0x68d29f1b, "skb_dequeue" },
	{ 0xd22b546, "__up_wakeup" },
	{ 0xf2520b76, "__down_failed" },
	{ 0x954cbb26, "vsprintf" },
	{ 0xb841d5ae, "usb_free_urb" },
	{ 0xc3cd5f9b, "usb_alloc_urb" },
	{ 0xb14c3cc3, "per_cpu__softnet_data" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("usb:v0A5Cp0CDCdl*dh*dc*dsc*dp*ic*isc*ip*");

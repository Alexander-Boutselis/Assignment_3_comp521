#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xe8213e80, "_printk" },
	{ 0xd710adbf, "__kmalloc_noprof" },
	{ 0x90a48d82, "__ubsan_handle_out_of_bounds" },
	{ 0xf638ba04, "kthread_create_on_node" },
	{ 0xfac370cf, "wake_up_process" },
	{ 0xcb8b6ec6, "kfree" },
	{ 0xd272d446, "__stack_chk_fail" },
	{ 0xbefd3ff8, "kthread_stop" },
	{ 0x0ab33d3b, "param_ops_int" },
	{ 0x0ab33d3b, "param_array_ops" },
	{ 0xbf80d1e1, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0xe8213e80,
	0xd710adbf,
	0x90a48d82,
	0xf638ba04,
	0xfac370cf,
	0xcb8b6ec6,
	0xd272d446,
	0xbefd3ff8,
	0x0ab33d3b,
	0x0ab33d3b,
	0xbf80d1e1,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"_printk\0"
	"__kmalloc_noprof\0"
	"__ubsan_handle_out_of_bounds\0"
	"kthread_create_on_node\0"
	"wake_up_process\0"
	"kfree\0"
	"__stack_chk_fail\0"
	"kthread_stop\0"
	"param_ops_int\0"
	"param_array_ops\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "25CB7050D45415B28C326BB");

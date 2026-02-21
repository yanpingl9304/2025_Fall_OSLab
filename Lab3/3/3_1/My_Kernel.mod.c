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
	{ 0xfefac423, "remove_proc_entry" },
	{ 0x8f2afe24, "const_pcpu_hot" },
	{ 0x40a621c5, "snprintf" },
	{ 0xa61fd7aa, "__check_object_size" },
	{ 0x092a35a2, "_copy_to_user" },
	{ 0xf64ac983, "__copy_overflow" },
	{ 0xd272d446, "__fentry__" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0xf8d7ac5e, "proc_create" },
	{ 0xe8213e80, "_printk" },
	{ 0x70eca2ca, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0xfefac423,
	0x8f2afe24,
	0x40a621c5,
	0xa61fd7aa,
	0x092a35a2,
	0xf64ac983,
	0xd272d446,
	0xd272d446,
	0xf8d7ac5e,
	0xe8213e80,
	0x70eca2ca,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"remove_proc_entry\0"
	"const_pcpu_hot\0"
	"snprintf\0"
	"__check_object_size\0"
	"_copy_to_user\0"
	"__copy_overflow\0"
	"__fentry__\0"
	"__x86_return_thunk\0"
	"proc_create\0"
	"_printk\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "AD71508A9B15D6752D63EF1");

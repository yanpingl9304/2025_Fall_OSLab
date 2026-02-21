#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
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

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x88db9f48, "__check_object_size" },
	{ 0xa825db8, "d_instantiate" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0x19a1109a, "new_inode" },
	{ 0x1aab5855, "unregister_filesystem" },
	{ 0x1a8dd3f0, "simple_statfs" },
	{ 0x1317437e, "d_make_root" },
	{ 0x747b7267, "d_splice_alias" },
	{ 0xe9a5a435, "iput" },
	{ 0xb418efe3, "register_filesystem" },
	{ 0xba8fbd64, "_raw_spin_lock" },
	{ 0xcbd4898c, "fortify_panic" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x7e9a259e, "simple_inode_init_ts" },
	{ 0x65487097, "__x86_indirect_thunk_rax" },
	{ 0x122c3a7e, "_printk" },
	{ 0xa916b694, "strnlen" },
	{ 0xd9b109bf, "set_nlink" },
	{ 0x5a921311, "strncmp" },
	{ 0x9166fada, "strncpy" },
	{ 0xe46ffc96, "default_llseek" },
	{ 0xed5e6618, "kill_litter_super" },
	{ 0xfb578fc5, "memset" },
	{ 0x31549b2a, "__x86_indirect_thunk_r10" },
	{ 0x49ae932d, "__insert_inode_hash" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x8853cb14, "mount_nodev" },
	{ 0x999e8297, "vfree" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0xf7dd326b, "__mark_inode_dirty" },
	{ 0xb5b54b34, "_raw_spin_unlock" },
	{ 0xca031c97, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "56F318FCCF0BF83EFABAAE9");

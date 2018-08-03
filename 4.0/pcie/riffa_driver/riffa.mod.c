#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xb6fd8534, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x6bc3fbc0, __VMLINUX_SYMBOL_STR(__unregister_chrdev) },
	{ 0x54fe1637, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0xd2b09ce5, __VMLINUX_SYMBOL_STR(__kmalloc) },
	{ 0xc897c382, __VMLINUX_SYMBOL_STR(sg_init_table) },
	{ 0x757047db, __VMLINUX_SYMBOL_STR(up_read) },
	{ 0xc8b57c27, __VMLINUX_SYMBOL_STR(autoremove_wake_function) },
	{ 0x64ae1178, __VMLINUX_SYMBOL_STR(pci_disable_device) },
	{ 0x34b00c9a, __VMLINUX_SYMBOL_STR(pcie_capability_read_dword) },
	{ 0x29303af9, __VMLINUX_SYMBOL_STR(device_destroy) },
	{ 0x52483983, __VMLINUX_SYMBOL_STR(__register_chrdev) },
	{ 0x3fec048f, __VMLINUX_SYMBOL_STR(sg_next) },
	{ 0xb8248221, __VMLINUX_SYMBOL_STR(pci_release_regions) },
	{ 0x94088c5d, __VMLINUX_SYMBOL_STR(down_read) },
	{ 0x9e88526, __VMLINUX_SYMBOL_STR(__init_waitqueue_head) },
	{ 0x4f8b5ddb, __VMLINUX_SYMBOL_STR(_copy_to_user) },
	{ 0x530ed545, __VMLINUX_SYMBOL_STR(pci_set_master) },
	{ 0x1fa3c3b3, __VMLINUX_SYMBOL_STR(pci_restore_state) },
	{ 0xb47d047e, __VMLINUX_SYMBOL_STR(current_task) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x16305289, __VMLINUX_SYMBOL_STR(warn_slowpath_null) },
	{ 0x581a5ed8, __VMLINUX_SYMBOL_STR(device_create) },
	{ 0x2072ee9b, __VMLINUX_SYMBOL_STR(request_threaded_irq) },
	{ 0xa310f4c2, __VMLINUX_SYMBOL_STR(arch_dma_alloc_attrs) },
	{ 0x78764f4e, __VMLINUX_SYMBOL_STR(pv_irq_ops) },
	{ 0x42c8de35, __VMLINUX_SYMBOL_STR(ioremap_nocache) },
	{ 0x83db4dde, __VMLINUX_SYMBOL_STR(get_user_pages) },
	{ 0xd62c833f, __VMLINUX_SYMBOL_STR(schedule_timeout) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
	{ 0xb6b1a614, __VMLINUX_SYMBOL_STR(pci_enable_msi_range) },
	{ 0x94e413f2, __VMLINUX_SYMBOL_STR(pci_unregister_driver) },
	{ 0xe9bcba71, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0xa6bbd805, __VMLINUX_SYMBOL_STR(__wake_up) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0xbdd7f760, __VMLINUX_SYMBOL_STR(pci_request_regions) },
	{ 0x3bb5114a, __VMLINUX_SYMBOL_STR(prepare_to_wait) },
	{ 0xedf7ab08, __VMLINUX_SYMBOL_STR(pci_disable_msi) },
	{ 0x4125f34e, __VMLINUX_SYMBOL_STR(dma_supported) },
	{ 0xedc03953, __VMLINUX_SYMBOL_STR(iounmap) },
	{ 0x22e04e3b, __VMLINUX_SYMBOL_STR(__pci_register_driver) },
	{ 0x393085cc, __VMLINUX_SYMBOL_STR(put_page) },
	{ 0x8b8dd7ac, __VMLINUX_SYMBOL_STR(class_destroy) },
	{ 0xf08242c2, __VMLINUX_SYMBOL_STR(finish_wait) },
	{ 0xb55413a9, __VMLINUX_SYMBOL_STR(pcie_capability_write_dword) },
	{ 0x28318305, __VMLINUX_SYMBOL_STR(snprintf) },
	{ 0xfdf73752, __VMLINUX_SYMBOL_STR(pci_enable_device) },
	{ 0x4f6b400b, __VMLINUX_SYMBOL_STR(_copy_from_user) },
	{ 0xf6185bb8, __VMLINUX_SYMBOL_STR(__class_create) },
	{ 0xc99164f4, __VMLINUX_SYMBOL_STR(dma_ops) },
	{ 0xf20dabd8, __VMLINUX_SYMBOL_STR(free_irq) },
	{ 0x7cafd776, __VMLINUX_SYMBOL_STR(pci_save_state) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("pci:v000010EEd*sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00001172d*sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "B295ACE4EAD21AD0837E0D9");

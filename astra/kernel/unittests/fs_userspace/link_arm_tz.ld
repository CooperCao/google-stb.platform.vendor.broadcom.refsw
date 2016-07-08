OUTPUT_ARCH("arm")
ENTRY(tz_init)

_system_link_base = 0xC0000000;

SECTIONS
{
    /* Load address */
    . = _system_link_base;
    _start = .;
    _bootstrap_img_start = .;

    _bootstrap_text_start = .;
    .text.bootstrap :
    {
        KEEP(*(.text.bootstrap))
    }
    . = ALIGN(4096); /* align to page boundary */
    _bootstrap_text_end = .;

    _bootstrap_data_start = .;
    .data.bootstrap :
    {
        KEEP(*(.data.bootstrap))
    }
    _bootstrap_data_end = .;

    _bootstrap_img_end = .;

    . = ALIGN(4096);

    _kernel_img_start = .;
    _text_start = .;
    .text :
    {
        KEEP(*(text.init))
        *(.text .text.* .gnu.linkonce.t.*)
    }

    . = ALIGN(4096); /* align to page size */
    _text_end = .;

    _rodata_start = .;
    .rodata :
    {
        *(.rodata)
    }
    . = ALIGN(4096); /* align to page size */
    _rodata_end = .;

    _data_start = .;
    .data :
    {
        *(.data)
    }
    . = ALIGN(4096); /* align to page size */
    _data_end = .;

    _bss_start = .;
    .bss :
    {
        bss = .;
        *(.bss)
    }
    . = ALIGN(4096); /* align to page size */
    _bss_end = .;

    _initramfs_start = .;
    .initramfs :
    {
        *(.initramfs)
    }
    . = ALIGN(4096);
    _initramfs_end = .;

    _kernel_img_end = .;

    _end = .;
}

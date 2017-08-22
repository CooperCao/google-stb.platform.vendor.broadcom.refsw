ENTRY(startup)

RAM_ADDR	= 0x10000000;
RAM_LEN		= 0x10000000;

INIT_START	= 0x10000000;

HEAP_START	= 0x10500000;
HEAP_LEN	= 0x00b00000;

STACK_BASE	= 0x10500000;
STACK_LEN	= 0x00010000;
STACK_TOP	= STACK_BASE - STACK_LEN;

OS_STACKS_BASE	= STACK_TOP;
OS_STACKS_LEN	= 0x10000;
OS_STACKS_TOP	= OS_STACKS_BASE - OS_STACKS_LEN;

MEMORY {
	DRAM : ORIGIN = 0x10000000, LENGTH = 0x10000000
}

SECTIONS {
	.init INIT_START :
	{
		* (.vectors);
		* (.init);
		* (.text);
		* (.text.*);
		* (.rodata);
		* (.rodata*);
		* (.data);
		* (.data*);
	} >DRAM

	.bss :
	{
		_bss_start = .;
		* (.bss);
		* (.bss.*);
		* (COMMON);
		_bss_end = .;
	} >DRAM

	.heap HEAP_START (NOLOAD) :
	{
		_heap_start = .;
		_heap_end = . + HEAP_LEN;
	}

	.os_stacks OS_STACKS_TOP (NOLOAD) :
	{
		_os_stacks_top = .;
		* (.os_stacks);
		_os_stacks_base = . + OS_STACKS_LEN;
	}

	.stack STACK_TOP (NOLOAD) :
	{
		_stack_svc_top = .;
		. = . + 0xD000;
		_stack_svc_base = .;

		_stack_abt_top = .;
		. = . + 0x1000;
		_stack_abt_base = .;

		_stack_undef_top = .;
		. = . + 0x1000;
		_stack_undef_base = .;

		_stack_irq_top = .;
		. = . + 0x1000;
		_stack_irq_base = STACK_LEN;
	}

    . = ALIGN(4);
    __exidx_start = .;
    PROVIDE(__exidx_start = __exidx_start);

    .ARM.exidx :
    {
        * (.ARM.exidx);
    } > DRAM

    . = ALIGN(4);
    __exidx_end = .;
    PROVIDE(__exidx_end = __exidx_end);
};

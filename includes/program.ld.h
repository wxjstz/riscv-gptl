
OUTPUT_ARCH(riscv)
ENTRY(_start)

SECTIONS
{
	. = {{CONFIG_FW_START}};
	PROVIDE(__fw_start = .);
	. = ALIGN(8);
	.text : {
		PROVIDE(__text_start = .);
		*(.entry)
		* (.text)
		PROVIDE(__text_end = .);
	}
	. = ALIGN(8);
	.rodata : {
		PROVIDE(__rodata_start = .);
		*(.rodata)
		* (.rodata.*)
		PROVIDE(__rodata_end = .);
	}
	. = ALIGN(8);
	.data : {
		PROVIDE(__data_start = .);
		*(.data)
		* (.data.*)
		PROVIDE(__data_end = .);
	}
	. = ALIGN(0x1000);
	.stack : {
		PROVIDE(__stack_start = .);
		. = . + {{CONFIG_STACK_SIZE}};
		PROVIDE(__stack_end = .);
	}
	. = ALIGN(8);
	.bss : {
		PROVIDE(__bss_start = .);
		*(.bss)
		* (.bss.*)
		PROVIDE(__bss_end = .);
	}
	. = ALIGN(0x1000);
	.heap : {
		PROVIDE(__heap_start = .);
		. = . + {{CONFIG_HEAP_SIZE}};
		PROVIDE(__heap_end = .);
	}
	PROVIDE(__fw_end = .);
}

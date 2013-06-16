
//-----------------------------------------------------------------
//							start.c
//-----------------------------------------------------------------

#include "type.h"
#include "protect.h"
#include "proc.h"
#include "global.h"
#include "kernel.h"
#include "util.h"

PUBLIC void cstart()
{
	init_vga();

	memcpy(&gdt,
			(void*)(*((u32_t*)(&gdt_ptr[2]))),
			*((u16_t*)(&gdt_ptr[0])) + 1
			);

	u16_t *p_gdt_limit = (u16_t*)(&gdt_ptr[0]);
	u32_t *p_gdt_base = (u32_t*)(&gdt_ptr[2]);
	*p_gdt_limit = GDT_SIZE * sizeof(DESCRIPTOR) - 1;
	*p_gdt_base = (u32_t)&gdt;

	u16_t* p_idt_limit = (u16_t*)(&idt_ptr[0]);
	u32_t* p_idt_base = (u32_t*)(&idt_ptr[2]);
	*p_idt_limit = IDT_SIZE * sizeof(GATE) - 1;
	*p_idt_base = (u32_t)&idt;
	
	init_prot();
}

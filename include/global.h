
//------------------------------------------------------
//					global.h
//------------------------------------------------------

#ifndef _OWINDYS_GLOBAL_H_
#define _OWINDYS_GLOBAL_H_ 

PUBLIC int disp_pos;

PUBLIC u8_t gdt_ptr[6];//0~15:limit, 16~47:base
PUBLIC DESCRIPTOR gdt[GDT_SIZE];

PUBLIC u8_t idt_ptr[6];
PUBLIC GATE idt[IDT_SIZE];

PUBLIC TSS tss;
PUBLIC PROCESS *p_proc_ready;
PUBLIC PROCESS proc_table[NR_TASK_PROCS];
PUBLIC char task_stack[STACK_SIZE_TOTAL];

PUBLIC u32_t k_reenter;
PUBLIC irq_handler irq_table[NR_IRQ];

PUBLIC int ticks;

// MM 
PUBLIC MESSAGE mm_msg;

// 由进程指针求进程号
#define proc2pid(x)(x - proc_table)

#endif


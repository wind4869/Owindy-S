
//-------------------------------------------------------------
//						protect.h
//-------------------------------------------------------------

#ifndef _OWINDYS_PROTECT_H_
#define _OWINDYS_PROTECT_H_

//代码段/数据段描述符(8个字节)
typedef struct s_descriptor {
	u8_t	limit_low;
	u16_t	base_low;
	u8_t	base_mid;
	u8_t	attr1;//P(1) DPL(2) S(1) TYPE(4)
	u8_t	limit_high_attr2;//G(1) D(1) 0(1) AVL(1) limit_high(4)
	u8_t	base_high;
} DESCRIPTOR;

//门描述符
typedef struct s_gate {
	u16_t offset_low;
	u16_t selector;
	u8_t param_count;
	u8_t attr;//P(1) DPL(2) S(1) TYPE(4)
	u16_t offset_high;
} GATE;

//任务状态段，目前主要用到esp0和ss0
typedef struct s_tss {
	u32_t backlink;
	u32_t esp0;
	u32_t ss0;
	u32_t esp1;
	u32_t ss1;
	u32_t esp2;
	u32_t ss2;
	u32_t cr3;
	u32_t eip;
	u32_t flags;
	u32_t eax;
	u32_t ecx;
	u32_t edx;
	u32_t ebx;
	u32_t esp;
	u32_t ebp;
	u32_t esi;
	u32_t edi;
	u32_t es;
	u32_t cs;
	u32_t ss;
	u32_t ds;
	u32_t fs;
	u32_t gs;
	u32_t ldt;
	u16_t trap;
	u16_t iobase;
} TSS;

// 描述符索引
#define	INDEX_DUMMY	0
#define	INDEX_FLAT_C 1
#define	INDEX_FLAT_RW 2
#define	INDEX_VIDEO	3
#define	INDEX_TSS 4
#define	INDEX_LDT_FIRST	5

//-----------------描述符、选择子-------------------
// 选择子
#define	SELECTOR_DUMMY 0
#define	SELECTOR_FLAT_C	0x08
#define	SELECTOR_FLAT_RW 0x10
#define	SELECTOR_VIDEO (0x18+3)	// RPL=3
#define	SELECTOR_TSS 0x20
#define SELECTOR_LDT_FIRST 0x28

#define	SELECTOR_KERNEL_CS SELECTOR_FLAT_C
#define	SELECTOR_KERNEL_DS SELECTOR_FLAT_RW
#define	SELECTOR_KERNEL_GS SELECTOR_VIDEO

// GDT和IDT中描述符的个数
#define	GDT_SIZE 128
#define	IDT_SIZE 256

// 描述符类型值说明
#define	DA_32		0x4000	// 32位段
#define	DA_LIMIT_4K	0x8000	// 段界限粒度为4K字节

#define	LIMIT_4K_SHIFT 12   // 短接线粒度偏移

#define	DA_DPL0		0x00	// DPL = 0
#define	DA_DPL1		0x20	// DPL = 1
#define	DA_DPL2		0x40	// DPL = 2
#define	DA_DPL3		0x60	// DPL = 3

// 存储段描述符类型值说明
#define	DA_DR		0x90	// 存在的只读数据段类型值
#define	DA_DRW		0x92	// 存在的可读写数据段属性值
#define	DA_DRWA		0x93	// 存在的已访问可读写数据段类型值
#define	DA_C		0x98	// 存在的只执行代码段属性值
#define	DA_CR		0x9A	// 存在的可执行可读代码段属性值
#define	DA_CCO		0x9C	// 存在的只执行一致代码段属性值	
#define	DA_CCOR		0x9E	// 存在的可执行可读一致代码段属性值

// 系统段描述符类型值说明
#define	DA_LDT		0x82	// 局部描述符表段类型值
#define	DA_TaskGate	0x85	// 任务门类型值
#define	DA_386TSS	0x89	// 可用 386 任务状态段类型值
#define	DA_386CGate	0x8C	// 386 调用门类型值
#define	DA_386IGate	0x8E	// 386 中断门类型值
#define	DA_386TGate	0x8F	// 386 陷阱门类型值

// 选择子类型值说明
// SA: Selector Attribute
#define	SA_RPL_MASK	0xFFFC
#define	SA_RPL0		0
#define	SA_RPL1		1
#define	SA_RPL2		2
#define	SA_RPL3		3

#define	SA_TI_MASK	0xFFFB
#define	SA_TIG		0
#define	SA_TIL		4

//------------------特权级------------------------
// 特权级
#define	PRIVILEGE_KRNL 0
#define	PRIVILEGE_TASK 1
#define	PRIVILEGE_USER 3

// RPL
#define	RPL_KRNL SA_RPL0
#define	RPL_TASK SA_RPL1
#define	RPL_USER SA_RPL3

//--------------------中断-----------------------
// 8259A主片和从片的控制端口
#define	INT_M_CTL 0x20
#define	INT_M_CTLMASK 0x21
#define	INT_S_CTL 0xA0
#define	INT_S_CTLMASK 0xA1

// 硬件中断向量号
#define INT_VECTOR_IRQ0 0x20
#define INT_VECTOR_IRQ8 0x28

// 硬件中断与8259A引脚对应关系
#define	NR_IRQ 16	    // Number of IRQs
#define	CLOCK_IRQ 0
#define	KEYBOARD_IRQ 1
#define	CASCADE_IRQ	2	// cascade enable for 2nd AT controller
#define	ETHER_IRQ 3		// default ethernet interrupt vector
#define	SECONDARY_IRQ 3	// RS232 interrupt vector for port 2
#define	RS232_IRQ	4	// RS232 interrupt vector for port 1
#define	XT_WINI_IRQ	5	// xt winchester
#define	FLOPPY_IRQ	6	// floppy disk
#define	PRINTER_IRQ	7
#define	AT_WINI_IRQ	14	// at winchester

// 软件中断向量号
#define	INT_VECTOR_DIVIDE		0x0
#define	INT_VECTOR_DEBUG		0x1
#define	INT_VECTOR_NMI			0x2
#define	INT_VECTOR_BREAKPOINT	0x3
#define	INT_VECTOR_OVERFLOW		0x4
#define	INT_VECTOR_BOUNDS		0x5
#define	INT_VECTOR_INVAL_OP		0x6
#define	INT_VECTOR_COPROC_NOT	0x7
#define	INT_VECTOR_DOUBLE_FAULT	0x8
#define	INT_VECTOR_COPROC_SEG	0x9
#define	INT_VECTOR_INVAL_TSS	0xA
#define	INT_VECTOR_SEG_NOT		0xB
#define	INT_VECTOR_STACK_FAULT	0xC
#define	INT_VECTOR_PROTECTION	0xD
#define	INT_VECTOR_PAGE_FAULT	0xE
#define	INT_VECTOR_COPROC_ERR	0x10

// 8253 PIT (Programmable Interval Timer)
#define TIMER0 0x40
#define TIMER_MODE 0x43
#define RATE_GENERATOR 0x34
#define TIMER_FREQ 1193182L
#define HZ 10

//-------------------进程---------------------
// 每个任务的LDT中描述符的个数和种类
#define LDT_SIZE 2
#define INDEX_LDT_C 0
#define INDEX_LDT_RW 1

// number of tasks and procs
#define NR_TASKS 3 // 系统任务数
#define NR_PROCS 32 // 最多32个用户进程
#define NR_NATIVE_PROCS 4 // 三个test进程+Init

// 任务和进程的总数
#define NR_TASK_PROCS (NR_TASKS + NR_PROCS)

// 系统任务
#define INTERRUPT -10

#define TASK_SYS 0
#define TASK_TTY 1
#define TASK_MM  2
#define INIT     3

#define ANY (NR_TASK_PROCS + 10)
#define NO_TASK	(NR_TASK_PROCS + 20)

// mem for procs
#define	PROCS_BASE		  0xA00000 // 10 MB 
#define	PROC_DEFAULT_MEM  0x100000 // 1 MB
#define	PROC_ORIGIN_STACK 0x400    // 1 KB

// stacks of tasks
#define STACK_SIZE_DEFAULT 0x4000 // 16KB
#define STACK_SIZE_SYS   STACK_SIZE_DEFAULT 
#define STACK_SIZE_TTY   STACK_SIZE_DEFAULT 
#define STACK_SIZE_MM	 STACK_SIZE_DEFAULT 
#define STACK_SIZE_INIT  STACK_SIZE_DEFAULT
#define STACK_SIZE_TESTA STACK_SIZE_DEFAULT 
#define STACK_SIZE_TESTB STACK_SIZE_DEFAULT 
#define STACK_SIZE_TESTC STACK_SIZE_DEFAULT 

#define STACK_SIZE_TOTAL (STACK_SIZE_SYS + \
				STACK_SIZE_TTY + \
				STACK_SIZE_MM + \
				STACK_SIZE_INIT + \
				STACK_SIZE_TESTA + \
				STACK_SIZE_TESTB + \
				STACK_SIZE_TESTC)

// IPC
#define SEND    1
#define RECEIVE	2
#define BOTH	3 // BOTH = SEND | RECEIVE

// macros for messages
#define	FD		 u.m3.m3i1
#define	PATHNAME u.m3.m3p1
#define	FLAGS	 u.m3.m3i1
#define	NAME_LEN u.m3.m3i2
#define	BUF_LEN	 u.m3.m3i3
#define	CNT      u.m3.m3i2
#define	REQUEST	 u.m3.m3i2
#define	PROC_NR	 u.m3.m3i3
#define	DEVICE	 u.m3.m3i4
#define	POSITION u.m3.m3l1
#define	BUF		 u.m3.m3p2
#define	OFFSET	 u.m3.m3i2
#define	WHENCE	 u.m3.m3i3

#define	PID		 u.m3.m3i2
#define	STATUS	 u.m3.m3i1
#define	RETVAL	 u.m3.m3i1

//进程状态
#define SENDING   0x02
#define RECEIVING 0x04
#define WAITING   0x08 // 等待子进程退出
#define HANGING   0x10 // 在不被父进程等待的情况下退出
#define EMPTY 	  0x20

//应用文件名最大长度
#define	MAX_PATH	128

// 函数声明
PUBLIC void init_prot();
PUBLIC u32_t seg2phys(u16_t seg);
PUBLIC void init_8259A();
PRIVATE void init_idt_desc(u8_t vector, u8_t desc_type,
		int_handler handler, u8_t privilege);
PUBLIC void init_descriptor(DESCRIPTOR* p_desc, u32_t base, u32_t limit, u16_t attribute);
PUBLIC void exception_handler(int vec_no, int err_code, int eip,
		int cs, int eflags);
PUBLIC void spurious_irq(int irq);
PUBLIC void put_irq_handler(int irq, irq_handler handler);

PUBLIC void disable_irq(int irq);
PUBLIC void enable_irq(int irq);

PUBLIC void Init();
PUBLIC void TestA();
PUBLIC void TestB();
PUBLIC void TestC();
PUBLIC void task_sys();
PUBLIC void task_mm();

#endif

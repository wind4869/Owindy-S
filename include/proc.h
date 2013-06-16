
//-------------------------------------------------------
//						proc.h
//-------------------------------------------------------

#ifndef	_OWINDYS_PROC_H_
#define	_OWINDYS_PROC_H_

#include "protect.h"

// 消息结构，间接抄自Minix
struct mess1 {
	int m1i1;
	int m1i2;
	int m1i3;
	int m1i4;
};

struct mess2 {
	void *m2p1;
	void *m2p2;
	void *m2p3;
	void *m2p4;
};

struct mess3 {
	int	m3i1;
	int	m3i2;
	int	m3i3;
	int	m3i4;
	u64_t m3l1;
	u64_t m3l2;
	void *m3p1;
	void *m3p2;
};

typedef struct {
	int source;
	int type;
	union {
		struct mess1 m1;
		struct mess2 m2;
		struct mess3 m3;
	} u;
} MESSAGE;

// 消息类型
enum msgtype {

	HARD_INT = 1,

	GET_TICKS, GET_PID,

	FORK, EXIT, WAIT, EXEC,

	SYSCALL_RET
};

typedef struct stackframe {
	u32_t	gs;
	u32_t	fs;
	u32_t	es;
	u32_t	ds;
	u32_t	edi;
	u32_t	esi;
	u32_t	ebp;
	u32_t	kernel_esp;
	u32_t	ebx;		
	u32_t	edx;
	u32_t	ecx;
	u32_t	eax;
	u32_t	retaddr;
	u32_t	eip;
	u32_t	cs;
	u32_t	eflags;
	u32_t	esp;
	u32_t	ss;	
} STACK_FRAME;

typedef struct proc {
	STACK_FRAME regs;     

	u16_t ldt_sel;    
	DESCRIPTOR ldts[LDT_SIZE];

	int p_parent;

    int ticks;      
    int priority;
	char p_name[16];

	int p_flags;
	MESSAGE *p_msg;
	int p_recvfrom; // 要从该进程接收消息
	int p_sendto; // 要发送消息给该进程

	int has_int_msg; // 是否有中断需要处理

	struct proc *q_sending; // 消息发送队列
	struct proc *next_sending;

    int exit_status; 
} PROCESS;

typedef struct task {
	task_f  initial_eip;
	int		stacksize;
	char	name[32];
} TASK;

#endif

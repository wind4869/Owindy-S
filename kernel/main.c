
//-----------------------------------------------------------------
//							main.c
//-----------------------------------------------------------------

#include "type.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"
#include "global.h"
#include "kernel.h"
#include "util.h"
#include "tty.h"

PUBLIC TASK task_table[NR_TASKS] = {
    {task_sys, STACK_SIZE_SYS, "SYS"},
    {task_tty, STACK_SIZE_TTY, "TTY"},
	{task_mm,  STACK_SIZE_MM,  "MM"}
};

PUBLIC TASK user_proc_table[NR_NATIVE_PROCS] = {
	{Init,  STACK_SIZE_INIT,  "Init"}, 
	{TestA, STACK_SIZE_TESTA, "TestA"},
	{TestB, STACK_SIZE_TESTB, "TestB"},
	{TestC, STACK_SIZE_TESTC, "TestC"}
};

PUBLIC int kernel_main()
{
	kprintf("\n<====================Welcome to Owindy-S====================>\n\n\n");

	TASK *p_task;
	PROCESS *p_proc	= proc_table;

	char* p_task_stack = task_stack + STACK_SIZE_TOTAL;
	u16_t selector_ldt = SELECTOR_LDT_FIRST;

	u8_t privilege, rpl;
	int eflags, priority;

	int i;
	for (i = 0; i < NR_TASK_PROCS; i++, p_proc++) {
		// 初始化 LDT 在 GDT 中的选择子
		p_proc->ldt_sel = selector_ldt;
		selector_ldt += 1 << 3;

		if (i >= NR_TASKS + NR_NATIVE_PROCS) {
			p_proc->p_flags = EMPTY;
			continue;
		}

		if (i < NR_TASKS) {
			p_task = task_table + i;
			privilege = PRIVILEGE_TASK;
			rpl = RPL_TASK;
			eflags = 0x1202; // IF = 1, IOPL = 1

			priority = 15;
		}
		else { 
			/* wind4869: 用户进程不能使用kprintf()
			 * yankai: 用户进程可以使用kprintf()
			 */
			p_task = user_proc_table + (i - NR_TASKS);
			privilege = PRIVILEGE_TASK;
			rpl = RPL_TASK;
			eflags = 0x1202; 

			priority = 5;
		}

		if (p_proc != proc_table + INIT) {
			/* 使用0~4G的扁平空间，只是改变一下特权级 */
			p_proc->ldts[INDEX_LDT_C]  = gdt[SELECTOR_KERNEL_CS >> 3];
			p_proc->ldts[INDEX_LDT_RW] = gdt[SELECTOR_KERNEL_DS >> 3],

			p_proc->ldts[INDEX_LDT_C].attr1 = DA_C | privilege << 5;
			p_proc->ldts[INDEX_LDT_RW].attr1 = DA_DRW | privilege << 5;
		}
		else {
			/* 假设Init也使用1M的内存空间，应该是够用的 */
			init_descriptor(&p_proc->ldts[INDEX_LDT_C],
					0,
					(PROC_DEFAULT_MEM - 1) >> LIMIT_4K_SHIFT,
					DA_32 | DA_LIMIT_4K | DA_C | priority << 5);

			init_descriptor(&p_proc->ldts[INDEX_LDT_RW],
					0,
					(PROC_DEFAULT_MEM - 1) >> LIMIT_4K_SHIFT,
					DA_32 | DA_LIMIT_4K | DA_DRW | priority << 5);
		}

		// 初始化段寄存器（选择子）
		p_proc->regs.cs	= INDEX_LDT_C << 3 | SA_TIL | rpl;

		p_proc->regs.ds	= 
		p_proc->regs.es	= 
		p_proc->regs.fs	= 
		p_proc->regs.ss	= INDEX_LDT_RW << 3 | SA_TIL | rpl;

		// 显存的描述符在gdt中
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;

		// 初始化进程的名字
		strcpy(p_proc->p_name, p_task->name);

		// eio, esp, eflags 的初始化
		p_proc->regs.eip = (u32_t)p_task->initial_eip;
		p_proc->regs.esp = (u32_t)p_task_stack;

		p_proc->regs.eflags = eflags;

		// 初始化时间片
		p_proc->ticks = p_proc->priority = priority;

		// 初始化消息传递的相关属性
		/*p_proc->p_flags = (i == TASK_SYS || i == TASK_MM || i == INIT || i == TASK_TTY) ? 0 : 1; // 只运行task_mm跟Init, 不知道为什么task_tty也在运行*/
        p_proc->p_flags = 0;
		p_proc->p_msg = 0;
		p_proc->p_recvfrom = NO_TASK;
		p_proc->p_sendto = NO_TASK;
		p_proc->q_sending = 0;
		p_proc->next_sending = 0;

		p_proc->p_parent = NO_TASK;
		p_proc->has_int_msg = 0;

		p_task_stack -= p_task->stacksize;
	}

	ticks = 0;
	k_reenter = 0;
	p_proc_ready = proc_table;

    init_clock();

	restart();

	while(1) {}
}

PUBLIC int get_ticks()
{
	MESSAGE msg;
	memset(&msg, 0, sizeof(MESSAGE));
	msg.type = GET_TICKS;
	send_recv(BOTH, TASK_SYS, &msg);
	
	return msg.RETVAL;
}

PUBLIC void delay(int time)
{
	int i, j, k;
	for (i = 0; i < time; i++)
		for (j = 0; j < 10; j++)
			for (k = 0; k < 100; k++)
				kprintf("");
}

PUBLIC void milli_delay(int milli_sec)
{
        int t = get_ticks();

        while(((get_ticks() - t) * 1000 / HZ) < milli_sec) {}
}

/*
void shabby_shell(const char *tty_name)
{
	int fd_stdin  = open(tty_name, O_RDWR);
	assert(fd_stdin  == 0);
	int fd_stdout = open(tty_name, O_RDWR);
	assert(fd_stdout == 1);

	char rdbuf[128];

	while (1) {
		write(1, "$ ", 2);
		int r = read(0, rdbuf, 70);
		rdbuf[r] = 0;

		int argc = 0;
		char * argv[PROC_ORIGIN_STACK];
		char * p = rdbuf;
		char * s;
		int word = 0;
		char ch;
		do {
			ch = *p;
			if (*p != ' ' && *p != 0 && !word) {
				s = p;
				word = 1;
			}
			if ((*p == ' ' || *p == 0) && word) {
				word = 0;
				argv[argc++] = s;
				*p = 0;
			}
			p++;
		} while(ch);
		argv[argc] = 0;

		int fd = open(argv[0], O_RDWR);
		if (fd == -1) {
			if (rdbuf[0]) {
				write(1, "{", 1);
				write(1, rdbuf, r);
				write(1, "}\n", 2);
			}
		}
		else {
			close(fd);
			int pid = fork();
			if (pid != 0) { // parent
				int s;
				wait(&s);
			}
			else {	// child
				execv(argv[0], argv);
			}
		}
	}

	close(1);
	close(0);
}
*/

PUBLIC void Init()
{
	int pid = fork();

	if (pid != 0) {

/*
 *        MESSAGE imsg;
 *
 *        send_recv(RECEIVE, pid, &imsg);
 *        printf("I'm init process, I received from %x: %s\n", pid, imsg.BUF);
 *
 */
        kprintf("\n<==============parent process is running==============>\n"
                "Hello, I'm a parent process\n");
		int status;
        int child = wait(&status);
        kprintf("My child process %x exited with status %d\n\n", child, status);
        kprintf("<=====================================================>\n");
        kprintf("Not yet, cannot resolve any command now\n");
		kputchar_color('O', VGA_COLOR_LOW_MAGENTA);
		kputchar_color('W', VGA_COLOR_LOW_MAGENTA);
		kputchar_color('i', VGA_COLOR_LOW_MAGENTA);
		kputchar_color('n', VGA_COLOR_LOW_MAGENTA);
		kputchar_color('y', VGA_COLOR_LOW_MAGENTA);
		kputchar_color('-', VGA_COLOR_LOW_MAGENTA);
		kputchar_color('S', VGA_COLOR_LOW_MAGENTA);
		kputchar_color('>', VGA_COLOR_HIGH_BLUE);
	} 
	else { // 子进程居然跳过了kprintf
            // 事实证明是因为在子进程中调用kprintf访问不到处在内核空间中的显存
        printf("\n<==============child process is running===============>\n"
                "Hello, I'm a child process\n"
                "<=====================================================>\n");
		exit(0);
	}

	while (1) {}
}


PUBLIC void TestA()
{
	while (1) {
        /*
         *MESSAGE msg;
         *send_recv(RECEIVE, TestB, &msg);
         *kprintf("I'm A, B tell me: %s\n", msg.BUF);
         */
		;
	}
}

PUBLIC void TestB()
{
	int t = 0;
	while (1) {
        /*
         *MESSAGE msg;
         *char *buf = "Hello, nice to meet you";
         *if (get_ticks() % 10000 == 0) {
         *    msg.BUF = buf;
         *    send_recv(SEND, TestA, &msg);
         *}
         */
		printf("I'm TestB: %d\n", t++);
		delay(300);
	}
}

PUBLIC void TestC()
{
	while (1) {
		;
	}
}

PUBLIC void task_sys()
{
    MESSAGE msg;
    while (1) {
        send_recv(RECEIVE, ANY, &msg);
        int src = msg.source;

        char buf[1024];
        char *p = (char*)va2la(src, msg.BUF);
        int i = msg.CNT;

        memcpy(buf, p, i);
        tty_write(&tty0, buf, i);
    }
}

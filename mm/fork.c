//
//-----------------------------------------------------------------
//                            fork.c
//-----------------------------------------------------------------
//

#include "type.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"
#include "global.h"
#include "util.h"

PUBLIC int fork() 
{
	MESSAGE msg;
	msg.type = FORK;

	send_recv(BOTH, TASK_MM, &msg);

	return msg.PID;
}

PUBLIC int do_fork()
{
	int i;
	PROCESS *p = proc_table;

	/* 寻找proc_table中的空闲位置，放置子进程 */

	for (i = 0; i < NR_TASK_PROCS; i++, p++)
		if (p->p_flags == EMPTY)
			break;

	if (i == NR_TASK_PROCS)
		return -1;

	int child_pid = i;
	int parent_pid = mm_msg.source;

	u16_t child_ldt_sel = p->ldt_sel;
	*p = proc_table[parent_pid];

	p->ldt_sel = child_ldt_sel;
	p->p_parent = parent_pid; 

	p->ticks = 15; // 改变时间片，使子进程优先运行

    /*
	 *kprintf("\n<do_fork:%x,%x>\n", p->regs.eip, p->ldt_sel);
     */

	/* 通过读取父进程的ldt，得到父进程内存占用情况 */

	DESCRIPTOR *parent_ldt = &proc_table[parent_pid].ldts[INDEX_LDT_C];

    int parent_limit = (parent_ldt->limit_high_attr2  & 0xF) << 16 | 
		parent_ldt->limit_low;
	int parent_size = (parent_limit + 1) * 
		((parent_ldt->limit_high_attr2 & (DA_LIMIT_4K >> 8)) ? 4096 : 1);
	int parent_base = parent_ldt->base_high << 24 | 
		parent_ldt->base_mid << 16 | parent_ldt->base_low;

	/* 给子进程分配内存, 代码、数据、堆栈共用 */

/*	if (parent_size > PROC_DEFAULT_MEM) {
		kprintf("The memory applied is too large!");
		return -1;
	} // 这种情况是不可能发生的！
*/
	int child_base = PROCS_BASE + (child_pid - (NR_TASKS + NR_NATIVE_PROCS)) * PROC_DEFAULT_MEM;
/*
	if (child_base + parent_size >= memory_size) {
		kprintf("Memory is running out!");
		return -1;
	} // 没有获取内存的大小，所以没考虑内存耗尽！
*/

	/* 拷贝父进程内存空间到子进程内存空间 */
	memcpy((void *)child_base, (void *)parent_base, parent_size);

	/* 子进程 LDT  */
	init_descriptor(&p->ldts[INDEX_LDT_C],
			child_base,
			(PROC_DEFAULT_MEM - 1) >> LIMIT_4K_SHIFT,
			DA_LIMIT_4K | DA_32 | DA_C | PRIVILEGE_TASK << 5);

	init_descriptor(&p->ldts[INDEX_LDT_RW],
			child_base,
			(PROC_DEFAULT_MEM - 1) >> LIMIT_4K_SHIFT,
			DA_LIMIT_4K | DA_32 | DA_DRW | PRIVILEGE_TASK << 5);

/* 告知文件系统，之后由fs_fork()接手
	MESSAGE msg2fs;
	msg2fs.type = FORK;
	msg2fs.PID = child_pid;
	send_recv(BOTH, TASK_FS, &msg2fs);
*/
	/* 返回子进程的 PID 给父进程 */
	mm_msg.PID = child_pid;

	/* 子进程诞生啦 */
	MESSAGE m;
	m.type = SYSCALL_RET;
	m.RETVAL = 0;
	m.PID = 0;
	send_recv(SEND, child_pid, &m);

	return 0;
}


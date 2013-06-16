//
//-----------------------------------------------------------------
//                          waitexit.c
//-----------------------------------------------------------------
//

#include "type.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"
#include "global.h"
#include "util.h"

PUBLIC void exit(int status)
{
	MESSAGE msg;
	msg.type = EXIT;
	msg.STATUS = status;

	send_recv(BOTH, TASK_MM, &msg);
}

PUBLIC int wait(int *status)
{
	MESSAGE msg;
	msg.type = WAIT;

	send_recv(BOTH, TASK_MM, &msg);

	*status = msg.STATUS;

	return (msg.PID == NO_TASK ? -1 : msg.PID);
}

PRIVATE void cleanup(struct proc *proc)
{
	MESSAGE msg2parent;
	msg2parent.type = SYSCALL_RET;
	msg2parent.PID = proc2pid(proc);
	msg2parent.STATUS = proc->exit_status;

	/* 当前进程是task_mm, 给调用者的父进程发送消息，解除阻塞 */
	send_recv(SEND, proc->p_parent, &msg2parent);

	proc->p_flags = EMPTY;
}

PUBLIC void do_exit(int status)
{
	int i;
	int pid = mm_msg.source;
	int parent_pid = proc_table[pid].p_parent;
	PROCESS *p = &proc_table[pid];
	
/* 	告知文件系统退出
	MESSAGE msg2fs;
	msg2fs.type = EXIT;
	msg2fs.PID = pid;
	send_recv(BOTH, TASK_FS, &msg2fs);
*/
	p->exit_status = status;

	if (proc_table[parent_pid].p_flags & WAITING) {
		proc_table[parent_pid].p_flags &= ~WAITING;
		cleanup(&proc_table[pid]);
	}
	else {
		proc_table[pid].p_flags |= HANGING;
	}

	/* 父亲自杀的子进程认Init为新爹 */
	for (i = 0; i < NR_TASK_PROCS; i++) {
		if (proc_table[i].p_parent == pid) {
			proc_table[i].p_parent = INIT;
			if ((proc_table[INIT].p_flags & WAITING) &&
			    (proc_table[i].p_flags & HANGING)) {
				proc_table[INIT].p_flags &= ~WAITING;
				cleanup(&proc_table[i]);
			}
		}
	}
}

PUBLIC void do_wait()
{
	int pid = mm_msg.source;

	int i;
	int children = 0;
	PROCESS* p_proc = proc_table;
	for (i = 0; i < NR_TASK_PROCS; i++, p_proc++) {
		if (p_proc->p_parent == pid) {
			children++;
			if (p_proc->p_flags & HANGING) {
				cleanup(p_proc);
				return;
			}
		}
	}

	if (children) {
		proc_table[pid].p_flags |= WAITING;
	}
	else { // 没有子进程，task_mm发消息给调用者，解除阻塞
		MESSAGE msg;
		msg.type = SYSCALL_RET;
		msg.PID = NO_TASK; // 出错的返回值
		send_recv(SEND, pid, &msg);
	}
}


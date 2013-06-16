
//-----------------------------------------------------------
//							proc.c
//-----------------------------------------------------------

#include "type.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"
#include "global.h"
#include "kernel.h"
#include "util.h"

PUBLIC void schedule()
{
	PROCESS *p;
	int	greatest_ticks = 0;

	while (!greatest_ticks) {
		for (p = proc_table; p < proc_table + NR_TASK_PROCS; p++)
			if (p->p_flags == 0)
				if (p->ticks > greatest_ticks) {
					greatest_ticks = p->ticks;
					p_proc_ready = p;
				}
				
		if (!greatest_ticks)
			for (p = proc_table; p < proc_table + NR_TASKS + NR_PROCS; p++)
				if (p->p_flags == 0)
					p->ticks = p->priority;
	}

}

PUBLIC void clock_handler(int irq)
{
	ticks++;
	p_proc_ready->ticks--;

    /*
     *kprintf("<clock,%d>", p_proc_ready - proc_table);
     */

	if (k_reenter != 0) {
		return;
	}

	if (p_proc_ready->ticks > 0) {
		return;
	}

	schedule();
}

/* 这个函数在fork()的时候用以分别父子进程的同名变量msg，貌似！ */

PUBLIC void *va2la(int pid, void *va) // 由虚拟地址求线性地址
{
	PROCESS *p = proc_table + pid;
	DESCRIPTOR *d = &p->ldts[INDEX_LDT_RW]; // ds, es, fs的描述符

	u32_t seg_base = d->base_high << 24 | d->base_mid << 16 | d->base_low;
	u32_t la = seg_base + (u32_t)va;

	return (void *)la;
}

PUBLIC int msg_send(PROCESS *current, int dest, MESSAGE *m)
{
    /*
     *kprintf("<send,%d,%d>", proc2pid(current), dest);
     */

	PROCESS *sender = current;
	PROCESS *p_dest = proc_table + dest;

	/*
	 *if (proc2pid(sender) == dest) {
	 *    kprintf("A proc can not send msg to itself!");
	 *}
	 */
	
	if ((p_dest->p_flags & RECEIVING) && // dest在等待接收消息
		(p_dest->p_recvfrom == proc2pid(sender) ||   
		 p_dest->p_recvfrom == ANY)) {
		
		memcpy(va2la(dest, p_dest->p_msg),  // 复制消息给dest
				va2la(proc2pid(sender), m),
				sizeof(MESSAGE));

		p_dest->p_msg = 0;
		p_dest->p_flags &= ~RECEIVING; // dest恢复运行
		p_dest->p_recvfrom = NO_TASK;

        /*
         *kprintf("<unblock,%d>", dest);
         */
	} 
	else {
		sender->p_flags |= SENDING;
		sender->p_sendto = dest;
		sender->p_msg = m;

		// 添加到dest的发送队列
		PROCESS *p;
		if (p_dest->q_sending) { // 队列不为空
			p = p_dest->q_sending;
			while (p->next_sending)
				p = p->next_sending;
			p->next_sending = sender;
		}
		else {
			p->next_sending = sender;
		}
		sender->next_sending = 0;
		
		schedule(); // 此时sender被阻塞，进行调度
	}

	return 0;
}

PUBLIC int msg_receive(PROCESS *current, int src, MESSAGE *m)
{
    /*
     *kprintf("<receive,%d,%d>", proc2pid(current), src);
     */

	PROCESS *receiver = current;
	PROCESS *p_from = 0;
	PROCESS *prev = 0;

	/*
	 *if (proc2pid(receiver) == src) {
	 *    kprintf("A proc can not receive msg from itself!");
	 *}
	 */

	int copied = 0;

	/* 有中断需要处理，并且准备好要处理了 */
	if ((receiver->has_int_msg) &&
	    ((src == ANY) || (src == INTERRUPT))) {

		MESSAGE msg;
		memset(&msg, 0, sizeof(MESSAGE));
		msg.source = INTERRUPT;
		msg.type = HARD_INT;

		memcpy(va2la(proc2pid(receiver), m), &msg,
			  sizeof(MESSAGE));

		receiver->has_int_msg = 0;

		return 0;
	}

	if (src == ANY) { // 从任意进程接收消息
		if (receiver->q_sending) { // 选择发送队列的第一个
			p_from = receiver->q_sending;
			copied = 1;
		}
	}
	else if (src >= 0 && src < NR_TASKS + NR_PROCS) { // 从特定进程接收消息
		p_from = proc_table + src;

		if ((p_from->p_flags & SENDING) && // src要发送消息给receiver
				(p_from->p_sendto == proc2pid(receiver))) {
			copied = 1;

			PROCESS *p = receiver->q_sending; // 此时队列必不为空!!!
			while (p) {
				if (proc2pid(p) == src)
					break;

				prev = p;
				p = p->next_sending;
			}
		}
	}

	if (copied) {
		// 从发送队列中删除p_from
		if (p_from == receiver->q_sending) {
			receiver->q_sending = p_from->next_sending;
			p_from->next_sending = 0;
		}
		else {
			prev->next_sending = p_from->next_sending;
			p_from->next_sending = 0;
		}

		memcpy(va2la(proc2pid(receiver), m), // 复制消息给receiver
				va2la(src, p_from->p_msg),
				sizeof(MESSAGE));

		p_from->p_msg = 0; // src恢复运行
		p_from->p_sendto = NO_TASK;
		p_from->p_flags &= ~SENDING;
	}
	else { // 木有进程给receiver发送消息
		receiver->p_flags |= RECEIVING;
		receiver->p_msg = m;
		receiver->p_recvfrom = src;

        /*
         *kprintf("<block,%d>", proc2pid(receiver));
         */

		schedule(); // receiver被阻塞，进行调度
	}

	return 0;
}

PUBLIC int sys_sendrec(int function, int src_dest, MESSAGE *m, PROCESS *p)
{
	int ret = 0;
	int caller = proc2pid(p);

	MESSAGE *mla = (MESSAGE *)va2la(caller, m);
	mla->source = caller;

	if (function == SEND) {
		ret = msg_send(p, src_dest, m);
		if (ret)
			return ret;
	}
	else if (function == RECEIVE) {
		ret = msg_receive(p, src_dest, m);
		if (ret)
			return ret;
	}
	else {
        kprintf("Invalid function type in sys_sendrec!!!");
	}

	return 0;
}

PUBLIC int send_recv(int function, int src_dest, MESSAGE *m)
{
	int ret = 0;

	if (function == RECEIVE)
		memset(m, 0, sizeof(MESSAGE));

	switch (function) {
	case BOTH:
		{
			int caller = proc2pid(p_proc_ready);		// 取m的线性地址保存到msg！
			MESSAGE *msg = (MESSAGE *)va2la(caller, m); // 实践证明直接把m赋给msg是不行的！

			kprintf("\n<send_recv:%x>\n", msg); // kprintf 确保msg正确

			ret = sendrec(SEND, src_dest, m);

			if (!ret)
				ret = sendrec(RECEIVE, src_dest, msg);
			break; // 理论上，子进程应该从sendrec中ret到这里！
		}
	case SEND:
	case RECEIVE:
		ret = sendrec(function, src_dest, m);
		break;
	default:
        kprintf("Invalid function type in send_recv!!!");
		break;
	}
		
	return ret;
}

PUBLIC system_call sys_call_table[NR_SYS_CALL] = {sys_sendrec}; 

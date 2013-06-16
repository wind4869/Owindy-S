//
//-----------------------------------------------------------------
//                            exec.c
//-----------------------------------------------------------------
//

#include "type.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"
#include "global.h"
#include "util.h"

PUBLIC int exec(const char *path)
{
	MESSAGE msg;
	msg.type = EXEC;
	msg.PATHNAME = (void*)path;
	msg.NAME_LEN = strlen(path);
	msg.BUF	= 0;
	msg.BUF_LEN	= 0;

	send_recv(BOTH, TASK_MM, &msg);

	return msg.RETVAL;
}

/* 关于可变参数列表的情况还不了解，最后一个是空指针? */

PUBLIC int execl(const char *path, const char *arg, ...)
{
	va_list parg = (va_list)(&arg);
	char **p = (char**)parg;
	return execv(path, p);
}

PUBLIC int execv(const char *path, char *argv[])
{
	char **p = argv;
	char arg_stack[PROC_ORIGIN_STACK];
	int stack_len = 0;

	while(*p++) {
		stack_len += sizeof(char*);
	}

	*((int*)(&arg_stack[stack_len])) = 0;
	stack_len += sizeof(char*);

	/* arg_stack的前半部分存放指针，指向后半部分的字符串 */

	char **q = (char**)arg_stack;
	for (p = argv; *p != 0; p++) {
		*q++ = &arg_stack[stack_len]; // 给前半部分指针赋值

		strcpy(&arg_stack[stack_len], *p); // 从argv中指示的位置拷贝字符串
		stack_len += strlen(*p);
		arg_stack[stack_len] = 0; // '\0'
		stack_len++;
	}

	MESSAGE msg;
	msg.type = EXEC;
	msg.PATHNAME = (void*)path;
	msg.NAME_LEN = strlen(path);
	msg.BUF	= (void*)arg_stack;
	msg.BUF_LEN	= stack_len;

	send_recv(BOTH, TASK_MM, &msg);

	return msg.RETVAL;
}

PUBLIC int do_exec()
{
	/* 从mm_msg获得信息 */
	int name_len = mm_msg.NAME_LEN;	// 文件名的长度
	int src = mm_msg.source; // 调用者

	if (name_len >= MAX_PATH) {
		kprintf("The length of filename is too long in do_exec!");
	}

	char pathname[MAX_PATH];
	memcpy((void*)va2la(TASK_MM, pathname),
		  (void*)va2la(src, mm_msg.PATHNAME),
		  name_len);
	pathname[name_len] = 0; // 文件名末尾用0填充
/*
	// 获得文件大小
	struct stat s;
	int ret = stat(pathname, &s);
	if (ret != 0) {
		kprintf("There is some error when stat in do_exec!");
		return -1;
	}

	// 读取文件
	int fd = open(pathname, O_RDWR);
	if (fd == -1)
		return -1;
	if (s.st_size >= MMBUF_SIZE) {
		kprintf("s.st_size is too large int do_exec!");
	}
	read(fd, mmbuf, s.st_size);
	close(fd);

	// 用刚读取的文件替换当前的进程映像
	int i;
	Elf32_Ehdr* elf_hdr = (Elf32_Ehdr*)(mmbuf);
	for (i = 0; i < elf_hdr->e_phnum; i++) {
		Elf32_Phdr *prog_hdr = (Elf32_Phdr*)(mmbuf + elf_hdr->e_phoff +
			 			(i * elf_hdr->e_phentsize));
		if (prog_hdr->p_type == PT_LOAD) {
			if (prog_hdr->p_vaddr + prog_hdr->p_memsz >=
					PROC_DEFAULT_MEM) {
				kprintf("There is some error when replace current proc in do_exec!");
			}
			memcpy((void*)va2la(src, (void*)prog_hdr->p_vaddr),
				  (void*)va2la(TASK_MM,
						 mmbuf + prog_hdr->p_offset),
				  prog_hdr->p_filesz);
		}
	}
*/
	/* 设置进程的栈, 栈中的参数在exev()中获得 */
	int orig_stack_len = mm_msg.BUF_LEN;
	char stackcopy[PROC_ORIGIN_STACK];
	memcpy((void*)va2la(TASK_MM, stackcopy),
		  (void*)va2la(src, mm_msg.BUF),
		  orig_stack_len);

	u8_t *orig_stack = (u8_t*)(PROC_DEFAULT_MEM - PROC_ORIGIN_STACK);

	int delta = (int)orig_stack - (int)mm_msg.BUF;

	int argc = 0;
	if (orig_stack_len) { // 栈中有参数
		char **q = (char**)stackcopy;
		for (; *q != 0; q++,argc++)
			*q += delta;
	}

	memcpy((void*)va2la(src, orig_stack),
		  (void*)va2la(TASK_MM, stackcopy),
		  orig_stack_len);

	proc_table[src].regs.ecx = argc; // argc
	proc_table[src].regs.eax = (u32_t)orig_stack; // argv

	/* 设置eip和esp */
	//proc_table[src].regs.eip = elf_hdr->e_entry;
	proc_table[src].regs.esp = PROC_DEFAULT_MEM - PROC_ORIGIN_STACK;

	strcpy(proc_table[src].p_name, pathname);

	return 0;
}

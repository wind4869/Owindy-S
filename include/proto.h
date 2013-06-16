#ifndef PROTO_H
#define PROTO_H

#include "type.h"
#include "proc.h"

// 系统调用
// As a micro kernel system, the number of syscall should be only one
// Here, write is only a temp syscall
#define INT_VECTOR_SYS_CALL 0x80
#define NR_SYS_CALL 2
// --------------------------

PUBLIC void clock_handler(int);
PUBLIC void keyboard_handler(int);

PUBLIC int sys_get_ticks();
PUBLIC int get_ticks();

PUBLIC int sys_write(char *buf, int len, PROCESS *p_proc);
PUBLIC void write(char *buf, int len);

PUBLIC int send_recv(int function, int src_dest, MESSAGE *m);

PUBLIC void *va2la(int pid, void *va); 
// 进程和内存管理
PUBLIC int fork();
PUBLIC int do_fork();

PUBLIC int wait(int *status);
PUBLIC void do_wait();

PUBLIC void exit(int status);
PUBLIC void do_exit(int status);

PUBLIC int exec(const char *path);
PUBLIC int do_exec();

PUBLIC int execl(const char *path, const char *arg, ...);
PUBLIC int execv(const char *path, char *argv[]);

#endif


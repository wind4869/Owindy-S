#ifndef TTY_H
#define TTY_H

#include "type.h"
#include "protect.h"
#include "proc.h"
#include "util.h"
#include "kernel.h"
#include "vga.h"
#include "keyboard.h"

#define TTY_IN_BYTES    256
#define NR_CONSOLES     5

typedef struct s_tty {
    u32_t   in_buf[TTY_IN_BYTES];   // TTY输入缓冲队列
    u32_t   *p_inbuf_head;
    u32_t   *p_inbuf_tail;
    int     inbuf_count;        // 缓冲区内容计数
} TTY;

PUBLIC TTY  tty0;

PUBLIC void task_tty();
PUBLIC void in_process(TTY*, u32_t);

#endif


;---------------------------------------------------------
;						kernel.asm
;---------------------------------------------------------

%include "asm/kconst.inc"

; 导入函数
extern cstart
extern spurious_irq
extern kernel_main
extern exception_handler

; 导入全局变量
extern tss
extern disp_pos
extern gdt_ptr
extern idt_ptr
extern irq_table
extern k_reenter
extern p_proc_ready
extern sys_call_table

bits 32

[SECTION .bss]
StackSpace resb 2 * 1024
Stacktop:

[SECTION .text]	

global _start
global restart
global sys_call

global divide_error
global single_step_exception
global nmi
global breakpoint_exception
global overflow
global bounds_check
global inval_opcode
global copr_not_available
global double_fault
global copr_seg_overrun
global inval_tss
global segment_not_present
global stack_exception
global general_protection
global page_fault
global copr_error

global hwint00
global hwint01
global hwint02
global hwint03
global hwint04
global hwint05
global hwint06
global hwint07
global hwint08
global hwint09
global hwint10
global hwint11
global hwint12
global hwint13
global hwint14
global hwint15
;----------------------kernel入口-----------------------------
_start:
	mov esp, Stacktop

	mov dword [disp_pos], 0

	sgdt [gdt_ptr]
	call cstart
	lgdt [gdt_ptr]
	lidt [idt_ptr] 
	
	jmp SELECTOR_KERNEL_CS:csinit	;强制使用刚刚初始化的结构，表示不是很懂

csinit:
	xor	eax, eax
	mov	ax, SELECTOR_TSS
	ltr	ax

	jmp	kernel_main

;----------------------硬件中断处理----------------------------
%macro hwint_master	1
	call save				; 保护现场,设置RETADR的值

	in al, INT_M_CTLMASK	; 屏蔽当前中断
	or	al, (1 << %1)
	out	INT_M_CTLMASK, al

	mov	al, EOI				; 置EOI位
	out	INT_M_CTL, al

	sti						; 开中断，CPU在响应中断时会自动关中断
	push %1					; 中断处理程序
	call [irq_table + 4 * %1]
	pop	ecx	
	cli

	in al, INT_M_CTLMASK	; 恢复接受当前中断
	and	al, ~(1 << %1)
	out	INT_M_CTLMASK, al

	ret
%endmacro

%macro hwint_slave 1
	push %1
	call spurious_irq
	add esp, 4
	hlt
%endmacro

ALIGN 16
hwint00:
	hwint_master 0

ALIGN 16
hwint01:
	hwint_master 1

ALIGN 16
hwint02:
    hwint_master 2

ALIGN 16
hwint03:
    hwint_master 3

ALIGN 16
hwint04:
    hwint_master 4

ALIGN 16
hwint05:
    hwint_master 5

ALIGN 16
hwint06:
    hwint_master 6

ALIGN 16
hwint07:
    hwint_master 7

ALIGN 16
hwint08:
    hwint_slave 8

ALIGN 16
hwint09:
    hwint_slave 9

ALIGN 16
hwint10:
    hwint_slave 10

ALIGN 16
hwint11:
    hwint_slave 11

ALIGN 16
hwint12:
    hwint_slave 12

ALIGN 16
hwint13:
    hwint_slave 13

ALIGN 16
hwint14:
    hwint_slave 14

ALIGN 16
hwint15:
    hwint_slave 15

;------------------------save---------------------------
save:
	pushad							; 保存当前寄存器的值
	push ds
	push es
	push fs
	push gs

	mov esi, edx ; 保存edx，edx中存放了系统调用的参数

	mov dx, ss
	mov ds, dx
	mov es, dx
	mov fs, dx

	mov edx, esi ; 恢复edx

	mov esi, esp                    ; esi = 进程表起始地址

	inc dword [k_reenter]			; 是否发生中断重入
	cmp dword [k_reenter], 0
	jne .1 

	mov esp, Stacktop               ; 切换到内核栈
	push restart
	jmp [esi + RETADR - P_STACKBASE]

.1:									; 中断重入，已在内核桟
	push restart_reenter
	jmp [esi + RETADR - P_STACKBASE]

;---------------------restart----------------------------
restart:
	mov	esp, [p_proc_ready]
	lldt [esp + P_LDT_SEL]
	lea	eax, [esp + P_STACKTOP]
	mov	dword [tss + TSS3_S_SP0], eax

restart_reenter:
	dec	dword [k_reenter]
	pop	gs
	pop	fs
	pop	es
	pop	ds
	popad
	add	esp, 4 ; 跳过retaddr
	iretd

;--------------------系统调用处理-------------------------
sys_call:
	call save
	
	sti
	push esi ; 保护esi，esi中保存了进程表首地址

	push dword [p_proc_ready] ; 指向当前进程的指针 p
	push edx ; 指向消息体的指针 m
	push ecx ; src_dest
	push ebx ; function

	call [sys_call_table + eax * 4]	

	add esp, 4 * 4 ; 跳过上面压入栈中的参数
	pop esi ; 恢复esi

	mov [esi + EAXREG - P_STACKTOP], eax ; 将返回值存入进程表eax位置
	cli

	ret
;--------------------软件中断处理-------------------------
;没有出错码的push 0xFFFFFFFF
;有出错码的，中断时cpu自动压入出错码
;出错码或0xFFFFFFFF之后压入向量号
divide_error:
	push 0xFFFFFFFF
	push 0
	jmp	exception
single_step_exception:
	push 0xFFFFFFFF	
	push 1
	jmp	exception
nmi:
	push 0xFFFFFFFF
	push 2
	jmp	exception
breakpoint_exception:
	push 0xFFFFFFFF
	push 3
	jmp	exception
overflow:
	push 0xFFFFFFFF
	push 4
	jmp	exception
bounds_check:
	push 0xFFFFFFFF
	push 5
	jmp	exception
inval_opcode:
	push 0xFFFFFFFF
	push 6
	jmp	exception
copr_not_available:
	push 0xFFFFFFFF
	push 7
	jmp	exception
double_fault:
	push 8
	jmp	exception
copr_seg_overrun:
	push 0xFFFFFFFF
	push 9
	jmp	exception
inval_tss:
	push 10	
	jmp	exception
segment_not_present:
	push 11
	jmp	exception
stack_exception:
	push 12
	jmp	exception
general_protection:
	push 13
	jmp	exception
page_fault:
	push 14	
	jmp	exception
copr_error:
	push 0xFFFFFFFF	
	push 16
	jmp	exception
;------------------------exception--------------------------
exception:
	call exception_handler
	add	esp, 8	; 让栈顶指向 EIP，堆栈中从顶向下依次是：EIP、CS、EFLAGS
	hlt

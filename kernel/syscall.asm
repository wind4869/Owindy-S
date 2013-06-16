
;-------------------------------------------------------
;						syscall.asm
;-------------------------------------------------------

INT_VECTOR_SYS_CALL equ 0x80
_NR_sendrec equ 0
_NR_write equ 1

global sendrec
global write

bits 32
[section .text]

;-----------------------------------------------------------
; PUBLIC int sendrec(int function, int src_dest, MESSAGE *m)
;-----------------------------------------------------------
sendrec:
	mov eax, _NR_sendrec
	mov ebx, [esp + 4]  ; function
	mov ecx, [esp + 8]  ; src_dest
	mov edx, [esp + 12] ; m (指向消息的指针)
	int INT_VECTOR_SYS_CALL
	ret ; 终于明白了，其实子进程是从这里开始运行的！！！

;==========================================================
; Temp Syscall for Write
;==========================================================
write:
    mov eax, _NR_write
    mov edx, [esp + 4]
    mov ecx, [esp + 8]
    int INT_VECTOR_SYS_CALL
    ret

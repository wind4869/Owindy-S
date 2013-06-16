
;---------------------------------------------------------------------
;							alib.asm
;---------------------------------------------------------------------

_NR_get_ticks equ 0
INT_VECTOR_SYS_CALL equ 0x80

[SECTION .text]

global disp_str
global disable_irq
global enable_irq

extern disp_pos

;*********************************************************************
;PUBLIC void disp_str(char *p_string);
;*********************************************************************

disp_str:
	push ebp
	mov ebp, esp

	mov esi, [ebp + 8]	;p_string
	mov edi, [disp_pos]
	mov ah, 0fh			;黑底白字
.1:
	lodsb				;ds:si->al, si + 1->si
	test al, al
	jz .2

	cmp al, 0ah			;处理回车
	jnz .3
	
	push eax
	mov eax, edi
	mov bl, 160
	div bl				;ax / bl, 商->al, 余数->ah
	and eax, 0ffh
	inc eax
	mul bl				;al * bl->ax
	mov edi, eax
	pop eax
	jmp .1
.3:
	mov [gs:edi], ax
	add edi, 2
	jmp .1
.2:
	mov [disp_pos], edi
	
	pop ebp
	ret

;*********************************************************************
; PUBLIC void disable_irq(int irq);
;*********************************************************************

disable_irq:
		mov		ecx, [esp + 4]			; irq
		pushf
		cli
		mov		ah, 1
		rol		ah, cl					; ah = (1 << (irq % 8))
		cmp		cl, 8
		jae		disable_8				; disable irq >= 8 at the slave 8259
disable_0:
		in		al, 0x21
		test	al, ah
		jnz		dis_already				; already disabled?
		or		al, ah
		out		0x21, al				; set bit master 8259
		popf
		mov		eax, 1					; disabled by this function
		ret
disable_8:
		in		al, 0xA1
		test	al, ah
		jnz		dis_already				; already disabled?
		or		al, ah
		out		0xA1, al				; set bit at slave 8259
		popf
		mov		eax, 1					; disabled by this function
		ret
dis_already:
		popf
		xor		eax, eax				; already disabled
		ret

;**************************************************************************
; PUBLIC void enable_irq(int irq);
;**************************************************************************

enable_irq:
		mov		ecx, [esp + 4]			; irq
		pushf
		cli
		mov		ah, ~1
		rol		ah, cl					; ah = ~(1 << (irq % 8))
		cmp		cl, 8
		jae		enable_8				; enable irq >= 8 at the slave 8259
enable_0:
		in		al, 0x21
		and		al, ah
		out		0x21, al				; clear bit at master 8259
		popf
		ret
enable_8:
		in		al, 0xA1
		and		al, ah
		out		0xA1, al				; clear bit at slave 8259
		popf
		ret

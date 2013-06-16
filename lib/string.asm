
;---------------------------------------------------------------------
;							string.asm
;---------------------------------------------------------------------

[SECTION .text]

;global memcpy
global disp_str
global out_byte

extern disp_pos

;**********************************************************************
; PUBLIC void * memcpy(void *p_destination, void *p_source, int size);
;**********************************************************************

;memcpy:
	;push ebp
	;mov ebp, esp
	
	;push edi
	;push esi
	;push ecx

	;mov edi, [ebp + 8]	;p_destination
	;mov esi, [ebp + 12]	;p_source
	;mov ecx, [ebp + 16]	;size

;.1:
	;cmp ecx, 0
	;jz .2

	;mov al, [ds:esi]
	;inc esi
	;mov byte [es:edi], al
	;inc edi

	;dec ecx
	;jmp .1
;.2:
	;mov eax, [ebp + 8]	;返回值p_destination
	
	;pop ecx
	;pop esi
	;pop edi
	;pop ebp

	;ret

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

;**********************************************************************
;PUBLIC void out_byte(u16 port, u8 value);
;**********************************************************************

;out_byte:
	;mov edx, [esp + 4]	;port
	;mov al, [esp + 8]	;value
	;out dx, al
	;nop
	;nop
   ; ret

; param in al
fun32_disp_al:
				push	ecx
				push	edx
				push	edi

				mov		edi, [dwDispPos]
				
				mov		ah, 0fh
				mov		dl, al
				shr		al, 4
				mov		ecx, 2
.begin:
				and		al, 0fh
				cmp		al, 9
				ja		.1
				add		al, '0'
				jmp		.2
.1:
				sub		al, 0ah
				add		al, 'A'
.2:
				mov		[gs:edi], ax
				add		edi, 2

				mov		al, dl
				loop	.begin

				mov		[dwDispPos], edi

				pop		edi
				pop		edx
				pop		ecx

				ret

; int in stack
fun32_disp_int:
				mov		eax, [esp + 4]
				shr		eax, 24
				call	fun32_disp_al

				mov		eax, [esp + 4]
				shr		eax, 16
				call	fun32_disp_al

				mov		eax, [esp + 4]
				shr		eax, 8
				call	fun32_disp_al

				mov		eax, [esp + 4]
				call	fun32_disp_al

				mov		ah, 07h
				mov		al, 'H'
				push	edi
				mov		edi, [dwDispPos]
				mov		[gs:edi], ax
				add		edi, 4
				mov		[dwDispPos], edi
				pop		edi

				ret

; param in stack
fun32_disp_str:
				push	ebp
				mov		ebp, esp
				push	ebx
				push	esi
				push	edi

				mov		esi, [ebp + 8]
				mov		edi, [dwDispPos]
				mov		ah, 0fh
.1:
				lodsb
				test	al, al
				jz		.2
				cmp		al, 0ah
				jnz		.3
				push	eax
				mov		eax, edi
				mov		bl, 160
				div		bl
				and		eax, 0ffh
				inc		eax
				mov		bl, 160
				mul		bl
				mov		edi, eax
				pop		eax
				jmp		.1
.3:
				mov		[gs:edi], ax
				add		edi, 2
				jmp		.1
.2:
				mov		[dwDispPos], edi

				pop		edi
				pop		esi
				pop		ebx
				pop		ebp
				ret

fun32_disp_newline:
				push	szNewline
				call	fun32_disp_str
				add		esp, 4

				ret

fun32_mem_copy:
				push	ebp
				mov		ebp, esp
				push	esi
				push	edi
				push	ecx

				mov		edi, [ebp + 8]
				mov		esi, [ebp + 12]
				mov		ecx, [ebp + 16]
.1:
				jecxz	.2

				mov		al, [ds:esi]
				inc		esi
				mov		byte [es:edi], al
				inc		edi

				dec		ecx
				jmp		.1
.2:
				mov		eax, [ebp + 8]

				pop		ecx
				pop		edi
				pop		esi
				mov		esp, ebp
				pop		ebp

				ret

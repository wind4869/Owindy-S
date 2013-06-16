; Load the real kernel
; Then setup the base mode of the system
; At last, long jump to kernel

%include "asm/boot.inc"
				jmp		short	_start

%include "asm/pm.inc"						; pm.inc is copied from Orange'S

%include "asm/fat12.inc"

; GDT
;							Base	Limit		Attr
_gdt:			Descriptor	0,		0,			0 ; there must be a NULL in the 
												  ; gdt's first entry
_gdt_flat_c:	Descriptor	0,		0fffffh,	DA_CR|DA_32|DA_LIMIT_4K
_gdt_flat_rw:	Descriptor	0,		0fffffh,	DA_DRW|DA_32|DA_LIMIT_4K
_gdt_video:		Descriptor	0b8000h,0ffffh,		DA_DRW|DA_DPL3

GdtLen			equ		$ - _gdt
gdt_ptr			dw		GdtLen - 1
				dd		LOADER_PHY_ADDR + _gdt

; GDT Selector
SelectorFlatC	equ		_gdt_flat_c - _gdt
SelectorFlatRW	equ		_gdt_flat_rw - _gdt
SelectorVideo	equ		_gdt_video - _gdt

_start:
				mov		ax, cs
				mov		ds, ax
				mov		es, ax
				mov		ss, ax

				mov		dh, 0			; "Loading..."
				call	fun16_disp_str

; Remember to get the memory info at the beginning
				mov		ebx, 0
				mov		di, _MemChkBuf
_mem_chk_loop:
				mov		eax, 0e820h
				mov		ecx, 20
				mov		edx, 0534d4150h
				int		15h
				jc		_mem_chk_fail
				add		di, 20
				inc		dword [_dwMCRNumber]
				cmp		ebx, 0
				jne		_mem_chk_loop
				jmp		_mem_chk_ok
_mem_chk_fail:
				mov		dword [_dwMCRNumber], 0		; will step into a dead cycle...
_mem_chk_ok:

; Below is read the floppy to load the kernel.sys
; As the same as the ipl loading loader.sys
				xor		ah, ah
				xor		dl, dl
				int		13h

				mov		word [sector_nr], RootDirSectorNo
_search_in_root_dir_begin:
				cmp		word [root_dir_size], 0
				jz		_no_kernel
				dec		word [root_dir_size]

				mov		ax, KERNEL_SEG
				mov		es, ax
				mov		bx, KERNEL_OFF 
				mov		ax, [sector_nr]
				mov		cl,  1
				call	fun16_read_sector

				mov		si, kernel_file_name
				mov		di, KERNEL_OFF
				cld
				mov		dx, 10h
_search_for_kernelsys:
				cmp		dx, 0
				jz		_next_sector_in_root
				dec		dx
				mov		cx, 11
_cmp_filename:
				repe
				cmpsb
				jcxz	_filename_found

				and		di, 0ffe0h
				add		di, 20h
				mov		si, kernel_file_name
				jmp		_search_for_kernelsys

_next_sector_in_root:
				inc		word [sector_nr]
				jmp		_search_in_root_dir_begin

_no_kernel:
				mov		dh, 2
				call	fun16_disp_str
				hlt
				jmp $

_filename_found:
				mov		ax, RootDirSectors
				and		di, 0ffe0h
				
				push	eax
				mov		eax, [es:di + 01ch]
				mov		dword [kernel_size], eax
				pop		eax

				add		di, 01ah
				mov		cx, word [es:di]
				push	cx
				add		cx, ax
				add		cx, DeltaSectorNo

				mov		ax, KERNEL_SEG
				mov		es, ax
				mov		bx, KERNEL_OFF
				mov		ax, cx

_kernel_loading:
				push	ax
				push	bx
				mov		ah, 0eh
				mov		al, '.'
				mov		bx, 0fh
				int		10h
				pop		bx
				pop		ax

				mov		cl, 1
				call	fun16_read_sector
				pop		ax
				call	fun16_get_FAT_entry
				cmp		ax, 0ff8h
				jae		_kernel_loaded
				push	ax
				mov		dx, RootDirSectors
				add		ax, dx
				add		ax, DeltaSectorNo
				add		bx, [BPB_BytsPerSec]
				jmp		_kernel_loading
				
_kernel_loaded:
				call	fun16_kill_motor			; Unactive Floppy Motor

				mov		dh, 1
				call	fun16_disp_str

_step_into_pm:
				lgdt	[gdt_ptr]

				cli

				in		al, 92h
				or		al, 02h
				out		92h, al

				mov		eax, cr0
				or		eax, 1
				mov		cr0, eax
				jmp		dword SelectorFlatC:(LOADER_PHY_ADDR + _pm_start)

				hlt
				jmp		$

; Variables
root_dir_size	dw		RootDirSectors
sector_nr		dw		0
is_odd			db		0
kernel_size		dd		0

; strings
kernel_file_name		db		"KERNEL  SYS"
length_of_msg			equ		10
loader_msg				db		"Loading   "
succeed_msg				db		"Ready!    "
no_kernel_msg			db		"No Kernel!"

; functions

fun16_disp_str:
				mov		ax, length_of_msg
				mul		dh
				add		ax, loader_msg
				mov		bp, ax
				mov		ax, ds
				mov		es, ax
				mov		cx, length_of_msg
				mov		ax, 1301h
				mov		bx, 0007h
				mov		dl, 0
				add		dh, 3
				int		10h
				ret

fun16_read_sector:
				push	bp
				mov		bp, sp
				sub		sp, 2

				mov		byte [bp - 2], cl
				push	bx
				mov		bl, [BPB_SecPerTrk]
				div		bl
				inc		ah
				mov		cl, ah
				mov		dh, al
				and		dh, 1
				shr		al, 1
				mov		ch, al
				mov		dl, [BS_DrvNum]

				pop		bx
.on_reading:
				mov		ah, 02h
				mov		al, byte [bp - 2]
				int		13h
				jc		.on_reading

				add		sp, 2
				pop		bp
				ret

fun16_get_FAT_entry:
				push	es
				push	bx
				push	ax

				mov		ax, KERNEL_SEG
				sub		ax, 100h
				mov		es, ax

				pop		ax
				mov		byte [is_odd], 0
				mov		bx, 3
				mul		bx
				mov		bx, 2
				div		bx
				cmp		dx, 0
				jz		.even_entry
				mov		byte [is_odd], 1
	
.even_entry:
				xor		dx, dx
				mov		bx, [BPB_BytsPerSec]
				div		bx

				push	dx
				mov		bx, 0
				add		ax, FAT1SectorNo
				mov		cl, 2
				call	fun16_read_sector
				pop		dx
				add		bx, dx
				mov		ax, [es:bx]
				cmp		byte [is_odd], 1
				jnz		.even2
				shr		ax, 4
.even2:
				and		ax, 0fffh

				pop		bx
				pop		es
				ret

fun16_kill_motor:
				push	dx
				mov		dx, 03f2h
				mov		al, 0
				out		dx, al
				pop		dx
				ret

[section .s32]
[bits 32]
align 32

_pm_start:
				mov		ax, SelectorVideo
				mov		gs, ax

				mov		ax, SelectorFlatRW
				mov		ds, ax
				mov		es, ax
				mov		fs, ax
				mov		ss, ax
				mov		esp, TopOfStack

				push	szMemChkTitle
				call	fun32_disp_str
				add		esp, 4

				call	fun32_disp_meminfo
				call	fun32_setup_paging

				mov		ah, 0fh
				mov		al, 'P'
				mov		[gs:((80 * 0 + 39) * 2)], ax

				jmp		SelectorFlatC:KERNEL_ENTRY

%include "asm/fun32.inc"

fun32_disp_meminfo:
				push	esi
				push	edi
				push	ecx

				mov		esi, MemChkBuf
				mov		ecx, [dwMCRNumber]
.lp:
				mov		edx, 5
				mov		edi, ARDStruct
.1:
				push	dword [esi]
				call	fun32_disp_int
				pop		eax
				stosd
				add		esi, 4
				dec		edx
				cmp		edx, 0
				jnz		.1
				call	fun32_disp_newline
				cmp		dword [dwType], 1
				jne		.2
				mov		eax, [dwBaseAddrLow]
				add		eax, [dwLengthLow]
				cmp		eax, [dwMemSize]
				jb		.2
				mov		[dwMemSize], eax
.2:
				loop	.lp
				call	fun32_disp_newline
				push	szRAMSize
				call	fun32_disp_str
				add		esp, 4

				push	dword [dwMemSize]
				call	fun32_disp_int
				add		esp, 4

				pop		ecx
				pop		edi
				pop		esi
				ret

fun32_setup_paging:
				xor		edx, edx
				mov		eax, [dwMemSize]
				mov		ebx, 400000h
				div		ebx
				mov		ecx, eax
				test	edx, edx
				jz		.no_remainder
				inc		ecx
.no_remainder:
				push	ecx

				mov		ax, SelectorFlatRW
				mov		es, ax
				mov		edi, PAGEDIR_BASE
				xor		eax, eax
				mov		eax, PAGETBL_BASE|PG_P|PG_USU|PG_RWW
.1:
				stosd
				add		eax, 4096
				loop	.1

				pop		eax
				mov		ebx, 1024
				mul		ebx
				mov		ecx, eax
				mov		edi, PAGETBL_BASE
				xor		eax, eax
				mov		eax, PG_P|PG_USU|PG_RWW
.2:
				stosd
				add		eax, 4096
				loop	.2

				mov		eax, PAGEDIR_BASE
				mov		cr3, eax
				mov		eax, cr0
				or		eax, 80000000h
				mov		cr0, eax

				nop
				ret

[section .data1]
align 32

_data16:
_szMemChkTitle:	db	"BaseAddrL BaseAddrH LengthLow LengthHigh    Type", 0ah, 0
_szRAMSize:		db	"RAM size:", 0
_szNewline:		db	0ah, 0

_dwMCRNumber:	dd	0					; Memory Check Result
_dwDispPos:		dd (80 * 6 + 0) * 2		; Row 6, Col 0
_dwMemSize:		dd 0
_ARDStruct:		; Address Range Descriptor Structure
	_dwBaseAddrLow:			dd		0
	_dwBaseAddrHigh:		dd		0
	_dwLengthLow:			dd		0
	_dwLengthHigh:			dd		0
	_dwType:				dd		0
_MemChkBuf:		times	256	db		0

_data32:
szMemChkTitle		equ		LOADER_PHY_ADDR + _szMemChkTitle
szRAMSize			equ		LOADER_PHY_ADDR + _szRAMSize
szNewline			equ		LOADER_PHY_ADDR + _szNewline
dwDispPos			equ		LOADER_PHY_ADDR + _dwDispPos
dwMemSize			equ		LOADER_PHY_ADDR + _dwMemSize
dwMCRNumber			equ		LOADER_PHY_ADDR + _dwMCRNumber
ARDStruct			equ		LOADER_PHY_ADDR + _ARDStruct
	dwBaseAddrLow	equ		LOADER_PHY_ADDR + _dwBaseAddrLow
	dwBaseAddrHigh	equ		LOADER_PHY_ADDR + _dwBaseAddrHigh
	dwLengthLow		equ		LOADER_PHY_ADDR + _dwLengthLow
	dwLengthHigh	equ		LOADER_PHY_ADDR + _dwLengthHigh
	dwType			equ		LOADER_PHY_ADDR + _dwType
MemChkBuf			equ		LOADER_PHY_ADDR + _MemChkBuf

StackSpace:			times	1024	db	0
TopOfStack:			equ		LOADER_PHY_ADDR + $

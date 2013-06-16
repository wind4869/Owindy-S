; This is the boot init program loader, will be loaded at 0x07c00 and run in the first time after BIOS start the OS.
; The function is to load the second program loader(loader.sys) into the memory 0x90000.
; Not same to Linux, this ipl don't copy itself, because it doesn't load the kernel
; After load the loader.sys, a long jump will be performance

%include "asm/boot.inc"
				jmp		short	_start
				nop							; For FAT12

%include "asm/fat12.inc"
_start:
				jmp		BOOT_OFF:_real_start
_real_start:
				mov		ax, cs
				mov		ds, ax
				mov		es, ax
				mov		es, ax
				mov		ss, ax
				mov		sp, STACK_TOP

				call	fun_clear_screen

				mov		dh, 0				; "Booting..."
				call	fun_disp_str

; Below is read the floppy
; I think it's foolish still using floppys today.
; However, I've to use this boring FAT12 1.44M floppy to boot my OS because I have no sense about the filesystem on this OS.
; But I think this problem will be solve in time, so just a temp boot.

				xor		ah, ah
				xor		dl, dl
				int		13h

				mov		word [sector_nr], RootDirSectorNo
_search_in_root_dir_begin:
				cmp		word [root_dir_size], 0
				jz		_no_loader
				dec		word [root_dir_size]

				mov		ax, LOADER_SEG
				mov		es, ax
				mov		bx, LOADER_OFF
				mov		ax, [sector_nr]
				mov		cl, 1
				call	fun_read_sector

				mov		si, loader_file_name
				mov		di, LOADER_OFF
				cld
				mov		dx, 10h
_search_for_loadersys:
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
				mov		si, loader_file_name
				jmp		_search_for_loadersys

_next_sector_in_root:
				inc		word [sector_nr]
				jmp		_search_in_root_dir_begin

_no_loader:
				mov		dh, 2
				call	fun_disp_str
				hlt
				jmp $

_filename_found:
				mov		ax, RootDirSectors
				and		di, 0ffe0h			; start of current entry
				add		di, 01ah			; first sector
				mov		cx, word [es:di]
				push	cx					; nr of this sector in FAT
				add		cx, ax
				add		cx, DeltaSectorNo	; cl -- the first sector of loader.sys
				mov		ax, LOADER_SEG
				mov		es, ax
				mov		bx, LOADER_OFF
				mov		ax, cx

_loader_loading:
				push	ax
				push	bx
				mov		ah, 0eh
				mov		al, '.'
				mov		bl, 0fh
				int		10h
				pop		bx
				pop		ax
				; ... Progress Bar

				mov		cl, 1
				call	fun_read_sector
				pop		ax
				call	fun_get_FAT_entry
				cmp		ax, 0ff8h
				jae		_loader_loaded
				push	ax
				mov		dx, RootDirSectors
				add		ax, dx
				add		ax, DeltaSectorNo
				add		bx, [BPB_BytsPerSec]
				jmp		_loader_loading

_loader_loaded:
				mov		dh, 1
				call	fun_disp_str

				jmp		(LOADER_PHY_ADDR >> 4):0		; jump into loader.sys!

; Variables
root_dir_size	dw		RootDirSectors
sector_nr		dw		0
is_odd			db		0

; strings
loader_file_name	db		"LOADER  SYS", 0
length_of_msg		equ		10					;Each str has same length, keep simple
boot_msg			db		"Booting   "
succeed_msg			db		"OK!       "
no_loader_msg		db		"No Loader!"

; functions

; int 0x13, ah = 0x00
; dl = driver_nr
fun_clear_screen:
				mov		ax, 600h
				mov		bx, 700h
				xor		cx, cx
				mov		dx, 184fh
				int		10h
				ret

; print string, dh is it's index(boot_msg is 0)
fun_disp_str:
				mov		ax, length_of_msg
				mul		dh
				add		ax, boot_msg
				mov		bp, ax
				mov		ax, ds
				mov		es, ax
				mov		cx, length_of_msg
				mov		ax, 1301h
				mov		bx, 0007h
				mov		dl, 0
				int		10h
				ret

; ax / BPB_SecPerTrk = y ... z
; y >> 1 = c
; y & 1 = h
; z = s - 1
fun_read_sector:
				push	bp
				mov		bp, sp
				sub		sp, 2		; byte [bp - 2] is local area to save the number of sectors

; int 0x13, ah = 0x02
; dl = driver_nr
; ch = c, dh = h, cl = s, es:bx = buffer
				mov		byte [bp - 2], cl
				push	bx
				mov		bl, [BPB_SecPerTrk]
				div		bl		; y in al, z in ah
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

; find the entry of the sector (nr = ax) in FAT
; ret ax
; will read FAT into es:bx
fun_get_FAT_entry:
				push	es
				push	bx
				push	ax

				mov		ax, LOADER_SEG
				sub		ax, 100h
				mov		es, ax			; 4K before LOADER_SEG

				pop		ax
				mov		byte [is_odd], 0
				mov		bx, 3			; dx:ax = bx * 3
				mul		bx
				mov		bx, 2
				div		bx				; dx:ax / bx, y in ax, r in dx
				cmp		dx, 0
				jz		.even_entry
				mov		byte [is_odd], 1

.even_entry:
				xor		dx, dx
				mov		bx, [BPB_BytsPerSec]
				div		bx				; dx:ax / BPB_BytsPerSec
										; ax = the sector of FATEntry from FAT
										; dx = FATEntry offset in sector
				push	dx
				mov		bx, 0
				add		ax, FAT1SectorNo
				mov		cl, 2			; avoid beyond one sector
				call	fun_read_sector
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

;--------------------------------------------------------------------------
times			510-($-$$)	db	0
dw				0AA55h

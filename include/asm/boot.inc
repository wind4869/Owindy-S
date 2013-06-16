;Define memory position for the Boot Loader
;I just le these constants be as same with Linux 0.11(even though they're not have the same names with linux) because I don't want to redesign this, and I'm sure our kernel won't be larger than 512KB ;-)
;But there's one thing different from linux, that is I don't copy the ipl self to 0x90000, just load the spl(loader.sys) there

BOOT_OFF			equ				0x07c0

LOADER_SEG			equ				0x9000
LOADER_OFF			equ				0x100

LOADER_PHY_ADDR		equ				(LOADER_SEG << 4) + LOADER_OFF

KERNEL_SEG			equ				0x1000
KERNEL_OFF			equ				0x0	

KERNEL_ENTRY		equ				(KERNEL_SEG << 4) + KERNEL_OFF

STACK_TOP			equ				0x4000	; Because EBDA, but what is EBDA?

PAGEDIR_BASE		equ				0x200000
PAGETBL_BASE		equ				0x201000

; Below is used for harddisk booter
; Notice that 0x5000 - 0x7bff is free, and the super block will be stored at 0x5000 - 0x7000
SUPER_BLK_SEG		equ				0x50

; This is root_base sector number, it's diffcult to determine for different partition of harddisk.
; I just set it 0x800 as default for the first my partition in the hdc.img, perhaps it's wront
ROOT_BASE			equ				0x800

; Below is the header of fat12
;-----------------------------------------------------------------------
BS_OEMName				db			'WindyOSG'

BPB_BytsPerSec			dw			0x200
BPB_SecPerClus			db			0x1
BPB_RsvdSecCnt			dw			0x1
BPB_NumFATs				db			0x2
BPB_RootEntCnt			dw			0xe0
BPB_TotSec16			dw			0xb40 ;2880, for floppy;-(
BPB_Media				db			0xf0
BPB_FATSz16				dw			0x9
BPB_SecPerTrk			dw			0x12
BPB_NumHeads			dw			0x2
BPB_HiddSec				dd			0
BPB_TotSec32			dd			0

BS_DrvNum				db			0
BS_Reserved1			db			0
BS_BootSig				db			0x29
BS_VolID				dd			0
BS_VolLab				db			'OWindyStemp'
BS_FileSysType			db			'FAT12   '
;----------------------------------------------------------------------

; Below is some constant about fat12 filesystem
RootDirSectors			equ			14
RootDirSectorNo			equ			19
FAT1SectorNo			equ			1			; equal to BPB_RsvdSecCnt
DeltaSectorNo			equ			17			; BPB_RsvdSecCnt + (BPB_NumFATs * FATSz) - 2
												; File start sector nr : Start sector nr in DirEntry + RootDirSectors + DeltaSectorNo

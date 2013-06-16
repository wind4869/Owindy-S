#############################################
#          Makefile for  Owindy-S
#############################################

AS = nasm
CC = gcc
LD = ld

CFLAGS = -g -Wall -O2 -fomit-frame-pointer -c -I../include -m32 -std=gnu99 -fno-stack-protector
ASFLAGS = -f elf -I../include/
LDFLAGS = -Ttext 0x10000 -N -e _start --oformat binary -m elf_i386

KERNEL_SYS := kernel.sys
KERNEL_SRC := kernel.asm

.PHONY : default, clean

default : $(KERNEL_SYS)

$(KERNEL_SYS) : kernel.o start.o main.o protect.o proc.o syscall.o clock.o vga.o keyboard.o tty.o ../mm/fork.o ../mm/waitexit.o ../mm/exec.o ../mm/main.o 
	$(LD) $(LDFLAGS) kernel.o start.o main.o protect.o proc.o syscall.o clock.o vga.o keyboard.o tty.o ../lib/alib.o ../lib/util.o ../mm/fork.o ../mm/waitexit.o ../mm/exec.o ../mm/main.o -o $@

kernel.o : kernel.asm ../include/asm/kconst.inc
	$(AS) $< $(ASFLAGS) -o $@

start.o : start.c ../include/type.h ../include/protect.h ../include/proc.h ../include/global.h ../include/kernel.h ../include/util.h
	$(CC) $< $(CFLAGS) -c

main.o : main.c ../include/type.h ../include/protect.h ../include/proc.h ../include/global.h ../include/kernel.h ../include/util.h ../include/tty.h
	$(CC) $< $(CFLAGS) -c

protect.o : protect.c ../include/type.h ../include/protect.h ../include/proc.h ../include/global.h ../include/kernel.h ../include/util.h
	$(CC) $< $(CFLAGS) -c

proc.o : proc.c ../include/type.h ../include/protect.h ../include/proc.h ../include/global.h ../include/kernel.h ../include/util.h
	$(CC) $< $(CFLAGS) -c

syscall.o : syscall.asm
	$(AS) $< $(ASFLAGS) -o $@

clock.o : clock.c ../include/type.h ../include/protect.h ../include/util.h ../include/kernel.h
	$(CC) $< $(CFLAGS) -c

vga.o : vga.c ../include/global.h ../include/type.h ../include/util.h ../include/vga.h
	$(CC) $< $(CFLAGS) -c

keyboard.o : keyboard.c ../include/protect.h ../include/type.h ../include/util.h ../include/kernel.h ../include/keyboard.h
	$(CC) $< $(CFLAGS) -c

tty.o : tty.c ../include/type.h ../include/util.h ../include/kernel.h ../include/tty.h
	$(CC) $< $(CFLAGS) -c

clean :
	rm $(KERNEL_SYS) -f
	rm *.o -f

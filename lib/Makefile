CC =gcc
LD =ld
AS =nasm
ASFLAGS =-f elf -I../include
CFLAGS =-g -Wall -O2 -fomit-frame-pointer -c -I../include -m32 -std=gnu99 -fno-stack-protector

.PHONY : clean

default : util.o alib.o

util.o : util.c 
	$(CC) $< $(CFLAGS) -o util.o

alib.o : alib.asm
	$(AS) $< $(ASFLAGS) -o alib.o

clean :
	rm *.o -f


CC = gcc
LD = ld
AS = nasm
ASFLAGS = -f elf -I../include
CFLAGS = -g -Wall -O2 -fomit-frame-pointer -c -I../include -m32 -std=gnu99 -fno-stack-protector

.PHONY : clean

default : main.o fork.o waitexit.o exec.o

main.o : main.c ../include/type.h ../include/protect.h ../include/proc.h ../include/proto.h ../include/global.h ../include/util.h
	$(CC) $< $(CFLAGS) -c

fork.o : fork.c ../include/type.h ../include/protect.h ../include/proc.h ../include/proto.h ../include/global.h ../include/util.h
	$(CC) $< $(CFLAGS) -c

waitexit.o : waitexit.c ../include/type.h ../include/protect.h ../include/proc.h ../include/proto.h ../include/global.h ../include/util.h
	$(CC) $< $(CFLAGS) -c

exec.o : exec.c ../include/type.h ../include/protect.h ../include/proc.h ../include/proto.h ../include/global.h ../include/util.h
	$(CC) $< $(CFLAGS) -c

clean :
	rm *.o -f


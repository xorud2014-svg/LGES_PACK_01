CC = gcc
INCLUDE	= -I. -I/usr/src/rtlinux-3.2-pre2/include
CFLAGS = -g -Wall -O2

.SUFFIXES :.o .c
.c.o:
	@echo "		Compiling.......$<"
	@$(CC) $(CFLAGS) -c $(INCLUDE) $<

OBJS = main.o

sysquit: $(OBJS)
	@echo "		Linking.........$<"
	@$(CC) -lm -o $@ $(OBJS) $(LIBNAME)

make clean:
	rm -rf *.o

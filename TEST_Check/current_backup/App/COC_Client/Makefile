CC = gcc
CFLAGS = -g -Wall -O2
INCLUDE	= -I. -I/usr/src/rtlinux-3.2-pre2/include

.SUFFIXES :.o .c
.c.o:
	@echo "		Compiling......$<"
	@$(CC) $(CFLAGS) -c $(INCLUDE) $<

OBJS = common_utils.o common_message.o userlog.o \
	local_utils.o local_message.o com_io.o com_socket.o network.o main.o

COC_Client: $(OBJS)
	@echo "		Linking........$<"
	@$(CC) -lm -o $@ $(OBJS) $(LIBNAME)
	@ln -s -f COC_Client COC1_Client

clean:
	rm -rf *.o

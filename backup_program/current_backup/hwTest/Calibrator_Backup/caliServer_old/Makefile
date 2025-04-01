CC	= gcc
CFLAGS	= -g -Wall
INCLUDE	= -I.

.SUFFIXES :.o .c
.c.o:
	@echo "		Comiling...........$<"
	@$(CC) $(CFLAGS) -O2 -c $(INCLUDE) $<

OBJS =  com_io.o com_socket.o comm.o main.o

CaliServer_Form: $(OBJS)
	@echo "		Linking............$<"
	@$(CC) -o $@ $(OBJS)

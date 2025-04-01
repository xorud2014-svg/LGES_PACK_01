CC	= gcc
CFLAGS	= -g -Wall
#INCLUDE	= -I/usr/src/linux/include
INCLUDE	= -I.

.SUFFIXES :.o .c
.c.o:
	@echo "		Comiling..............................$<"
	@$(CC) $(CFLAGS) -O2 -c $(INCLUDE) $<

OBJ =  com_io.o com_socket.o comm.o comm1.o switch.o main.o

all:	CalServer
	mv CalServer ../

CalServer: $(OBJ)
	@echo "		Linking...............................$<"
	@$(CC) -lm -o $@ $(OBJ)

clean:
	rm -rf *.o
	rm -rf CalServer

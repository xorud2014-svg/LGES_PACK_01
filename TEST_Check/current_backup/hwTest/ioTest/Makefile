CC	= gcc
CFLAGS	= -g
INCLUDE	= -I/usr/src/linux/include

.SUFFIXES :.o .c
.c.o:
	@echo "		Comiling..............................$<"
	@$(CC) $(CFLAGS) -O2 -c $(INCLUDE) $<

OBJ =  main.o

all:	ioTest

ioTest: $(OBJ)
	@echo "		Linking...............................$<"
	@$(CC) -o $@ $(OBJ)
clean:
	rm -rf *.o	ioTest	*.csv

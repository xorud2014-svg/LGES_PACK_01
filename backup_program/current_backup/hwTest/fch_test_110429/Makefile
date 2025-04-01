CC	= gcc
CFLAGS	= -g -Wall
INCLUDE	= -I.

.SUFFIXES :.o .c
.c.o:
	@echo "		Comiling...........$<"
	@$(CC) $(CFLAGS) -O2 -c $(INCLUDE) $<

OBJS =  main.o

fch_test: $(OBJS)
	@echo "		Linking............$<"
	@$(CC) -o $@ $(OBJS)

clean:
	rm -rf *.o *.csv

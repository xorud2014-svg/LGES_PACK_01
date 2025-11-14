CC	= gcc
CFLAGS	= -g -Wall
INCLUDE	= -I.

.SUFFIXES :.o .c
.c.o:
	@echo "		Comiling...........$<"
	@$(CC) $(CFLAGS) -O2 -c $(INCLUDE) $<

OBJS =  main.o

can_fd_test_v1: $(OBJS)
	@echo "		Linking............$<"
	@$(CC) -o $@ $(OBJS)

clean:
	rm -rf *.o *.csv
